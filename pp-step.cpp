#include <iostream>

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/Utils.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/ModuleLoader.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/PPConditionalDirectiveRecord.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/OptTable.h"

using namespace clang;

struct log_expands : public clang::PPCallbacks {
  log_expands(Preprocessor &p) : pp(p) {}

  virtual void MacroExpands(const Token &MacroNameTok,
                            const MacroDefinition &MD, SourceRange Range,
                            const MacroArgs *Args) {

    llvm::outs() << "expanding macro " << pp.getSpelling(MacroNameTok)
                 << " into " << '\n';
    const auto macro = MD.getMacroInfo();

    for (const auto &tok : macro->tokens())
      llvm::outs() << pp.getSpelling(tok) << ' ';

    if (macro->isFunctionLike() && Args != nullptr) {
      llvm::outs() << '\n' << '\t' << "where:" << '\n';
      auto tokenAt = [Args](unsigned int index) {
        return Args->getUnexpArgument(0) + index;
      };
      unsigned int i = 0;
      for (const auto args : macro->args()) {
        llvm::outs() << '\t' << '\t' << args->getNameStart() << " is ";
        while (tokenAt(i)->isNot(tok::eof)) {
          llvm::outs() << pp.getSpelling(*tokenAt(i));
          i++;
        }
        i++;
        llvm::outs() << '\n';
      }
    }
  }
  Preprocessor &pp;
};

int main(int argc, char *argv[]) {

  CompilerInstance ci;

  // -----========== TARGET ==========-----

  ci.createDiagnostics(
      new clang::TextDiagnosticPrinter(llvm::errs(), &ci.getDiagnosticOpts()));
  std::shared_ptr<TargetOptions> topts(new clang::TargetOptions);
  topts->Triple = LLVM_DEFAULT_TARGET_TRIPLE;
  ci.setTarget(TargetInfo::CreateTargetInfo(ci.getDiagnostics(), topts));

  // -----========== ARGUMENT PARSING ==========-----

  using llvm::opt::InputArgList;
  std::vector<const char *> ref;
  for (int i = 1; i < argc; ++i)
    ref.emplace_back(argv[i]);
  unsigned missingIndex{0}, missingCount{0};
  auto table = clang::driver::createDriverOptTable();
  auto Args = table->ParseArgs(ref, missingIndex, missingCount);

  // -----========== FILE ==========-----

  ci.createFileManager();
  ci.createSourceManager(ci.getFileManager());

  auto inputFile = [&]() -> FileID {
    auto inputs = Args.getAllArgValues(clang::driver::options::OPT_INPUT);
    if (inputs.size() == 1 && inputs[0] != "-") {
      const FileEntry *pFile = ci.getFileManager().getFile(inputs[0]);
      return ci.getSourceManager().getOrCreateFileID(
          pFile, clang::SrcMgr::CharacteristicKind::C_User);
    }
    llvm::errs() << "None or More than one source file was specified -- aborting\n";
    exit(1);
    return ci.getSourceManager().createFileID(nullptr);
  }();

  // -----========== PREPROCESSOR ==========-----
  {
    // include directory defaults
    const char *dirs = {
      #ifdef SYSTEM_HEADERS
        SYSTEM_HEADERS,
      #endif
        nullptr
    };
    for (const char *dir = dirs; dir != nullptr; ++dir)
      ci.getHeaderSearchOpts().AddSystemHeaderPrefix(dir, true);

    using namespace clang::driver::options;
    using namespace llvm::opt;

    for (const Arg *A : Args.filtered(OPT_I))
      ci.getHeaderSearchOpts().AddPath(A->getValue(), frontend::Quoted, false, true);
    for (const Arg *A : Args.filtered(OPT_D, OPT_U)) {
      if (A->getOption().matches(OPT_D))
        ci.getPreprocessorOpts().addMacroDef(A->getValue());
      else
        ci.getPreprocessorOpts().addMacroUndef(A->getValue());
    }
    ci.getPreprocessorOpts().MacroIncludes = Args.getAllArgValues(OPT_imacros);
    for (const Arg *A : Args.filtered(OPT_include))
      ci.getPreprocessorOpts().Includes.emplace_back(A->getValue());
    for (const Arg *A : Args.filtered(OPT_chain_include))
      ci.getPreprocessorOpts().ChainedIncludes.emplace_back(A->getValue());
  }
  ci.createPreprocessor(
      clang::TranslationUnitKind::TU_Complete); // create Preprocessor

  // -----========== APPLY PASS ==========-----

  ci.getSourceManager().setMainFileID(inputFile);
  ci.getPreprocessor().EnterMainSourceFile();
  ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(),
                                           &ci.getPreprocessor());
  ci.getPreprocessor().addPPCallbacks(
      std::unique_ptr<log_expands>(new log_expands(ci.getPreprocessor())));

  Token tok;
  do {
    ci.getPreprocessor().Lex(tok);
    if (ci.getDiagnostics().hasErrorOccurred())
      break;
  } while (tok.isNot(clang::tok::eof));

  ci.getDiagnosticClient().EndSourceFile();

  return EXIT_SUCCESS;
}

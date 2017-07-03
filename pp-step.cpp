#include <iostream>

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
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

using namespace clang;

struct log_expands : public clang::PPCallbacks {
  log_expands(Preprocessor &p) : pp(p) {}
  virtual void MacroExpands(const Token &MacroNameTok, const MacroDefinition &MD, SourceRange Range,
                            const MacroArgs *Args) {
    llvm::errs() << "expanding macro " << pp.getSpelling(MacroNameTok) << " into \n";

    for (auto tok = MD.getMacroInfo()->tokens_begin(); tok != MD.getMacroInfo()->tokens_end(); ++tok) {
      llvm::errs() << pp.getSpelling(*tok) << " ";
    }

    if (MD.getMacroInfo()->isFunctionLike() && Args != nullptr) {
      llvm::errs() << "\n\t where:\n";

      auto unexpArgStart = Args->getUnexpArgument(0);
      unsigned i = 0;
      for (auto tok = MD.getMacroInfo()->arg_begin(); tok != MD.getMacroInfo()->arg_end(); ++tok) {
        llvm::errs() << "\t\t" << (*tok)->getNameStart() << " is ";
        while ((unexpArgStart + i)->isNot(tok::eof)) {
          llvm::errs() << pp.getSpelling(*(unexpArgStart + i)) << " ";
          i++;
        }
        i++;
        llvm::errs() << "\n";
      }
    }
  }
  Preprocessor &pp;
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "No filename given" << std::endl;
    return EXIT_FAILURE;
  }
  CompilerInstance ci;
  ci.createDiagnostics(
      new clang::TextDiagnosticPrinter(llvm::errs(), &ci.getDiagnosticOpts())); // create DiagnosticsEngine
  ci.createFileManager();                                                       // create FileManager
  ci.createSourceManager(ci.getFileManager());                                  // create SourceManager

  std::shared_ptr<TargetOptions> topts(new clang::TargetOptions);
  topts->Triple = "x86_64-unknown-linux-gnu";
  ci.setTarget(TargetInfo::CreateTargetInfo(ci.getDiagnostics(), topts));

  ci.createPreprocessor(clang::TranslationUnitKind::TU_Complete); // create Preprocessor
  const FileEntry *pFile = ci.getFileManager().getFile(argv[1]);
  ci.getSourceManager().setMainFileID(
      ci.getSourceManager().getOrCreateFileID(pFile, clang::SrcMgr::CharacteristicKind::C_User));
  ci.getPreprocessor().EnterMainSourceFile();
  ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(), &ci.getPreprocessor());

  ci.getPreprocessor().addPPCallbacks(std::unique_ptr<log_expands>(new log_expands(ci.getPreprocessor())));
  Token tok;
  do {
    ci.getPreprocessor().Lex(tok);
    if (ci.getDiagnostics().hasErrorOccurred())
      break;
  } while (tok.isNot(clang::tok::eof));
  ci.getDiagnosticClient().EndSourceFile();
}

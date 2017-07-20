#include <iostream>

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Lex/TokenConcatenation.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/OptTable.h"

using namespace clang;

struct log_expands : public PPCallbacks {
  Preprocessor &pp;
  SourceManager &sm;
  TokenConcatenation tokCat;
  log_expands(Preprocessor &, SourceManager &);
  virtual void MacroExpands(const Token &, const MacroDefinition &, SourceRange,
                            const MacroArgs *);
	private:
  Token prevprev, prev;
  void printToken(const Token& tok);
  void flushPrinter();
};

void configureTarget(CompilerInstance &ci);

llvm::opt::InputArgList parseArgs(int argc, char **argv);

FileID getFileID(CompilerInstance &ci, llvm::opt::InputArgList &args);

void createPreprocessor(CompilerInstance &ci, llvm::opt::InputArgList &args);

void processFile(CompilerInstance &ci, const FileID &fileID);

int main(int argc, char *argv[]) {
  CompilerInstance ci;
  configureTarget(ci);
  auto arguments = parseArgs(argc, argv);
  createPreprocessor(ci, arguments);
  auto fileID = getFileID(ci, arguments);
  processFile(ci, fileID);
  return EXIT_SUCCESS;
}


log_expands::log_expands(Preprocessor &p, SourceManager& s) : pp(p), sm(s), tokCat(p) {}

void log_expands::printToken(const Token& tok) {
	if (tokCat.AvoidConcat(prevprev, prev, tok))
		llvm::outs() << ' ';
	llvm::outs() << pp.getSpelling(tok);
	prevprev = prev;
	prev = tok;
}

void log_expands::flushPrinter() {
	prevprev = {};
	prev = {};
}

void log_expands::MacroExpands(const Token &MacroNameTok,
                               const MacroDefinition &MD, SourceRange,
                               const MacroArgs *Args) {
  if (!sm.isInMainFile(MacroNameTok.getLocation())) {
    // llvm::outs() << "Skipping macro " << pp.getSpelling(MacroNameTok) << " because it is not expanded from the specified file\n";
    return;
  }

  llvm::outs() << "expanding macro " << pp.getSpelling(MacroNameTok) << " into "<< '\n';
  const auto macro = MD.getMacroInfo();

  for (const auto &tok : macro->tokens())
	  printToken(tok);
  flushPrinter();


  llvm::outs() << '\n';

  if (!macro->isFunctionLike() || Args == nullptr)
    return;

  llvm::outs() << '\t' << "where:" << '\n';

  auto tokenAt = [Args](unsigned int index) {
    return Args->getUnexpArgument(0) + index;
  };


  unsigned int i = 0;
  for (const auto args : macro->args()) {
    llvm::outs() << '\t' << '\t' << args->getNameStart() << " is ";
    while (tokenAt(i)->isNot(tok::eof)) {
      printToken(*tokenAt(i));
      i++;
    }
    flushPrinter();
    i++;
    llvm::outs() << '\n';
  }
}

void configureTarget(CompilerInstance &ci) {
  ci.getDiagnosticOpts().ShowColors = 1;
  auto diags = new TextDiagnosticPrinter(llvm::errs(), &ci.getDiagnosticOpts());
  ci.createDiagnostics(diags);
  std::shared_ptr<TargetOptions> topts(new clang::TargetOptions);
  topts->Triple = LLVM_DEFAULT_TARGET_TRIPLE;
  ci.setTarget(TargetInfo::CreateTargetInfo(ci.getDiagnostics(), topts));
}

llvm::opt::InputArgList parseArgs(int argc, char **argv) {
  std::vector<const char *> ref;
  for (int i = 1; i < argc; ++i)
    ref.emplace_back(argv[i]);
  unsigned missingIndex{0}, missingCount{0};
  auto table = driver::createDriverOptTable();
  return table->ParseArgs(ref, missingIndex, missingCount);
}

FileID getFileID(CompilerInstance &ci, llvm::opt::InputArgList &args) {
  auto inputs = args.getAllArgValues(driver::options::OPT_INPUT);
  if (inputs.size() == 1 && inputs[0] != "-") {
    const FileEntry *pFile = ci.getFileManager().getFile(inputs[0]);
    return ci.getSourceManager().getOrCreateFileID(
        pFile, SrcMgr::CharacteristicKind::C_User);
  }
  llvm::errs()
      << "None or More than one source file was specified -- aborting\n";
  exit(EXIT_FAILURE);
  return ci.getSourceManager().createFileID(nullptr);
}

void createPreprocessor(CompilerInstance &ci, llvm::opt::InputArgList &args) {
  ci.createFileManager();
  ci.createSourceManager(ci.getFileManager());

  #ifndef SYSTEM_HEADERS
    #define SYSTEM_HEADERS
  #endif

  for (const auto dir : {SYSTEM_HEADERS})
    ci.getHeaderSearchOpts().AddPath(dir, frontend::Angled, false, true);

  using namespace driver::options;
  using namespace llvm::opt;

  for (const auto A : args.filtered(OPT_I))
    ci.getHeaderSearchOpts().AddPath(A->getValue(), frontend::Quoted, false, true);
  for (const auto A : args.filtered(OPT_isystem))
    ci.getHeaderSearchOpts().AddPath(A->getValue(), frontend::Angled, false, true);
  for (const auto A : args.filtered(OPT_D, OPT_U)) {
    if (A->getOption().matches(OPT_D))
      ci.getPreprocessorOpts().addMacroDef(A->getValue());
    else
      ci.getPreprocessorOpts().addMacroUndef(A->getValue());
  }
  ci.createPreprocessor(TranslationUnitKind::TU_Complete);
}

void processFile(CompilerInstance &ci, const FileID &fileID) {
  ci.getSourceManager().setMainFileID(fileID);
  ci.getPreprocessor().EnterMainSourceFile();
  ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(),
                                           &ci.getPreprocessor());
  std::unique_ptr<PPCallbacks> logger(new log_expands(ci.getPreprocessor(), ci.getSourceManager()));
  ci.getPreprocessor().addPPCallbacks(std::move(logger));

  Token tok;
  do {
    ci.getPreprocessor().Lex(tok);
    if (ci.getDiagnostics().hasErrorOccurred())
      break;
  } while (tok.isNot(tok::eof));

  ci.getDiagnosticClient().EndSourceFile();
}

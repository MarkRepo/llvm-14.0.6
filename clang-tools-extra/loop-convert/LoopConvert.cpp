// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/ASTContext.h"
#include <iostream>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

/*
StatementMatcher LoopMatcher =
  forStmt(hasLoopInit(declStmt(hasSingleDecl(varDecl(
    hasInitializer(integerLiteral(equals(0)))))))).bind("forLoop");*/

StatementMatcher LoopMatcher =
forStmt(hasLoopInit(declStmt(
            hasSingleDecl(varDecl(hasInitializer(integerLiteral(equals(0))))
                              .bind("initVarName")))),
        hasIncrement(unaryOperator(
            hasOperatorName("++"),
            hasUnaryOperand(declRefExpr(
                to(varDecl(hasType(isInteger())).bind("incVarName")))))),
        hasCondition(binaryOperator(
            hasOperatorName("<"),
            hasLHS(ignoringParenImpCasts(declRefExpr(
                to(varDecl(hasType(isInteger())).bind("condVarName"))))),
            hasRHS(expr(hasType(isInteger())))))).bind("forLoop");

DeclarationMatcher MacroMatcher = functionDecl(returns((asString("bool")))).bind("cm_def");
StatementMatcher CallMatcher = callExpr(isExpandedFromMacro("CM")).bind("cm");
class CallPrinter : public MatchFinder::MatchCallback {
  public:
    void run(const MatchFinder::MatchResult & result) override {
      const CallExpr* ce = result.Nodes.getNodeAs<CallExpr>("cm");
      if (!ce) {return;}
      llvm::outs() << "find cm_call\n";
      // ce->dump();
      llvm::outs() << "num_args: " << ce->getNumArgs() << "\n";
      llvm::outs() << ce->getDirectCallee()->getName() << "\n";
      int cnt = ce->getNumArgs();
      for (int i = 0; i < cnt; i++) {
        const Expr* er = ce->getArg(i);
        llvm::outs() << "i: " << i << ", type: " << er->getType().getAsString() << "\n";
      }
    }
};

class MacroPrinter : public MatchFinder::MatchCallback {
  public:
    void run(const MatchFinder::MatchResult &Result) override {
      const FunctionDecl* fd = Result.Nodes.getNodeAs<FunctionDecl>("cm_def");
      if (!fd) {
        llvm::outs() << "fd is null\n";
        return;
      }
      llvm::outs() << "find CM_DEF\n";
      //fd->dump();
      llvm::outs() << "name: " << fd->getName() << "\n";
      auto params = fd->parameters();
      for(auto p: params) {
        llvm::outs() << "param: " <<  p->getOriginalType().getAsString() << "\n";
      }
    }
};

class LoopPrinter : public MatchFinder::MatchCallback {
public :
/*
  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const ForStmt *FS = Result.Nodes.getNodeAs<clang::ForStmt>("forLoop")) {
      std::cout << "find forLoop" << std::endl;
      FS->dump();
    }
  }

*/
static bool areSameVariable(const ValueDecl *First, const ValueDecl *Second) {
  return First && Second &&
         First->getCanonicalDecl() == Second->getCanonicalDecl();
}
  void run(const MatchFinder::MatchResult &Result) override {
    ASTContext *Context = Result.Context;
    const ForStmt *FS = Result.Nodes.getNodeAs<ForStmt>("forLoop");
    // We do not want to convert header files!
    if (!FS || !Context->getSourceManager().isWrittenInMainFile(FS->getForLoc()))
      return;
    const VarDecl *IncVar = Result.Nodes.getNodeAs<VarDecl>("incVarName");
    const VarDecl *CondVar = Result.Nodes.getNodeAs<VarDecl>("condVarName");
    const VarDecl *InitVar = Result.Nodes.getNodeAs<VarDecl>("initVarName");

    if (!areSameVariable(IncVar, CondVar) || !areSameVariable(IncVar, InitVar))
      return;
    llvm::outs() << "Potential array-based loop discovered.\n";
    FS->dump();
  }
};


// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...\n");
int main(int argc, const char **argv) {
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
  if (!ExpectedParser) {
    // Fail gracefully for unsupported options.
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser& OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  LoopPrinter Printer;
  MacroPrinter mPrinter;
  CallPrinter cPrinter;
  MatchFinder Finder;

  Finder.addMatcher(LoopMatcher, &Printer);
  Finder.addMatcher(MacroMatcher, &mPrinter);
  Finder.addMatcher(CallMatcher, &cPrinter);

  return Tool.run(newFrontendActionFactory(&Finder).get());
}
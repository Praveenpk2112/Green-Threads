#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/AST.h"
#include "llvm/Support/raw_ostream.h"
#include <system_error>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

static cl::OptionCategory InjectorCategory("Green Thread Injector Options");

class LoopPrinter : public MatchFinder::MatchCallback {
public:
    LoopPrinter(Rewriter &Rewrite) : Rewrite(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &Result) override {
        const Stmt *LoopNode = Result.Nodes.getNodeAs<Stmt>("loop");
        const Stmt *Body = nullptr;

        if (const ForStmt *FS = dyn_cast<ForStmt>(LoopNode)) {
            Body = FS->getBody();
        } else if (const WhileStmt *WS = dyn_cast<WhileStmt>(LoopNode)) {
            Body = WS->getBody();
        }

        if (Body) {
            if (const CompoundStmt *CS = dyn_cast<CompoundStmt>(Body)) {
                SourceLocation InsertLoc = CS->getLBracLoc().getLocWithOffset(1);
                
                Rewrite.InsertText(InsertLoc, "\n        GREEN_CHECK(); /* Auto-injected by Clang */\n", true, true);
            }
        }
    }

private:
    Rewriter &Rewrite;
};

class MyASTConsumer : public ASTConsumer {
public:
    MyASTConsumer(Rewriter &R) : HandlerForLoops(R) {
        Matcher.addMatcher(forStmt().bind("loop"), &HandlerForLoops);
        Matcher.addMatcher(whileStmt().bind("loop"), &HandlerForLoops);
    }

    void HandleTranslationUnit(ASTContext &Context) override {
        Matcher.matchAST(Context);
    }

private:
    LoopPrinter HandlerForLoops;
    MatchFinder Matcher;
};

class MyFrontendAction : public ASTFrontendAction {
public:
    MyFrontendAction() {}

    void EndSourceFileAction() override {
        llvm::errs() << "** Injection Complete. Overwriting file... **\n";
        
        TheRewriter.overwriteChangedFiles();
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<MyASTConsumer>(TheRewriter);
    }

private:
    Rewriter TheRewriter;
};

int main(int argc, const char **argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, InjectorCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    
    CommonOptionsParser &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
#include <cstdio>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <utility>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Frontend/Utils.h"

using namespace clang;
using namespace std;


SourceManager *m_srcmgr;
int ID = 0;
//int countBranch = 0;
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor>
{
public:
    MyASTVisitor(SourceManager &srcmgr_)
        :srcmgr(srcmgr_)
    {
        count = 0;
        countBranch = 0;
    }

    bool VisitStmt(Stmt *s) {
        // Fill out this function for your homework
        //SourceManager &srcmgr = *m_srcmgr;
        SourceLocation startLoc = s->getLocStart();
        unsigned int lineNum = srcmgr.getExpansionLineNumber(startLoc);
        unsigned int colNum = srcmgr.getExpansionColumnNumber(startLoc);
        /*string filename = srcmgr.getFilename(startLoc);
        if ((!filename.compare("")) || filename.com) {
            filename = prevFilename;
        } else {
            prevFilename = filename;
        }*/
        
        if (isa<IfStmt>(s)) {
            printBranchLineColFilename("If", lineNum, colNum, filename);
        } else if (isa<ForStmt>(s)) {
            printBranchLineColFilename("For", lineNum, colNum, filename);
        } else if (isa<WhileStmt>(s)) {
            printBranchLineColFilename("While", lineNum, colNum, filename);
        } else if (isa<CaseStmt>(s)) {
            printBranchLineColFilename("Case", lineNum, colNum, filename);
        } else if (isa<DoStmt>(s)) {
            //s->dump();
            printBranchLineColFilename("Do", lineNum, colNum, filename);
        } else if (isa<DefaultStmt>(s)) {
            printBranchLineColFilename("Default", lineNum, colNum, filename);
        } else if (isa<ConditionalOperator>(s)) {
            printBranchLineColFilename("?:", lineNum, colNum, filename);
        } else if (isa<SwitchStmt>(s)) {
            SwitchStmt *switchstmt = cast<SwitchStmt>(s);
            SwitchCase *branch = switchstmt->getSwitchCaseList();
            bool hasDefault = false; 
            for(; branch != NULL ; branch = branch->getNextSwitchCase()) {
                if (isa<DefaultStmt>(branch)) {
                    hasDefault = true;
                    break;
                }
            }
            if(!hasDefault)
                   printBranchLineColFilename("ImpDef", lineNum, colNum, filename);
        }
        return true;
    }
    
    bool VisitFunctionDecl(FunctionDecl *f) {
        // Fill out this function for your homework
        // SourceManager &srcmgr = *m_srcmgr;
        string funcname = f->getName();
        if (f->hasBody()) {
            cout<<"function:    "<<funcname<<endl;
            SourceLocation startLoc = f->getLocStart();
            filename = srcmgr.getFilename(startLoc);
            //cout<<"AAAAA"<<filename<<endl;
        }
        return true;
    }

    void printBranchNum() {
        cout<<"Total number of branches: "<<countBranch<<endl;
    }

private:
    int count;
    int countBranch;
    string filename;
    SourceManager &srcmgr;

    void printBranchLineColFilename(string branchName, unsigned int lineNum, 
                              unsigned int colNum, string filename) {
        
        cout<<"    "<<setw(10)<<left<<branchName;
        cout<<"ID: ", cout.width(7);
        cout<<count++;
        cout<<" Line: ", cout.width(7);
        cout<<lineNum;
        cout<<" Col: ", cout.width(7);
        cout<<colNum;
        cout<<" Filename: "<<filename<<endl;
        
        if ((!branchName.compare("ImpDef")) || (!branchName.compare("Default")) 
                || (!branchName.compare("Case"))) countBranch ++;
        else
            countBranch += 2;
    }

};


class MyASTConsumer : public ASTConsumer
{
public:
    MyASTConsumer(SourceManager &srcmgr)
        : Visitor(srcmgr) //initialize MyASTVisitor
    { }

    virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
        for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
            // Travel each function declaration using MyASTVisitor
            Visitor.TraverseDecl(*b);
        }
        return true;
    }
    void printBranchNum() {
        Visitor.printBranchNum();
    }

private:
    MyASTVisitor Visitor;
};

//int MyASTVisitor::count = 0; // ID
//int countBranch = 0; 

int main(int argc, char *argv[])
{
    if (argc != 2) {
        llvm::errs() << "Usage: kcov-branch-identify <filename>\n";
        return 1;
    }

    // CompilerInstance will hold the instance of the Clang compiler for us,
    // managing the various objects needed to run the compiler.
    CompilerInstance TheCompInst;
    
    // Diagnostics manage problems and issues in compile 
    TheCompInst.createDiagnostics(NULL, false);

    // Set target platform options 
    // Initialize target info with the default triple for our platform.
    TargetOptions *TO = new TargetOptions();
    TO->Triple = llvm::sys::getDefaultTargetTriple();
    TargetInfo *TI = TargetInfo::CreateTargetInfo(TheCompInst.getDiagnostics(), TO);
    TheCompInst.setTarget(TI);

    // FileManager supports for file system lookup, file system caching, and directory search management.
    TheCompInst.createFileManager();
    FileManager &FileMgr = TheCompInst.getFileManager();
    
    // SourceManager handles loading and caching of source files into memory.
    TheCompInst.createSourceManager(FileMgr);
    SourceManager &SourceMgr = TheCompInst.getSourceManager();
    //global var  m_srcmgr
    //m_srcmgr = &SourceMgr;

    // Prreprocessor runs within a single source file
    TheCompInst.createPreprocessor();
    
    // ASTContext holds long-lived AST nodes (such as types and decls) .
    TheCompInst.createASTContext();

    // Enable HeaderSearch option
    llvm::IntrusiveRefCntPtr<clang::HeaderSearchOptions> hso( new HeaderSearchOptions());
    HeaderSearch headerSearch(hso,
                              TheCompInst.getFileManager(),
                              TheCompInst.getDiagnostics(),
                              TheCompInst.getLangOpts(),
                              TI);

    // <Warning!!> -- Platform Specific Code lives here
    // This depends on A) that you're running linux and
    // B) that you have the same GCC LIBs installed that I do. 
    /*
    $ gcc -xc -E -v -
    ..
     /usr/local/include
     /usr/lib/gcc/x86_64-linux-gnu/4.4.5/include
     /usr/lib/gcc/x86_64-linux-gnu/4.4.5/include-fixed
     /usr/include
    End of search list.
    */
    const char *include_paths[] = {"/usr/local/include",
                "/usr/lib/gcc/x86_64-linux-gnu/4.4.5/include",
                "/usr/lib/gcc/x86_64-linux-gnu/4.4.5/include-fixed",
                "/usr/include"};

    for (int i=0; i<4; i++) 
        hso->AddPath(include_paths[i], 
                    clang::frontend::Angled, 
                    false, 
                    false);
    // </Warning!!> -- End of Platform Specific Code

    InitializePreprocessor(TheCompInst.getPreprocessor(), 
                  TheCompInst.getPreprocessorOpts(),
                  *hso,
                  TheCompInst.getFrontendOpts());


    // A Rewriter helps us manage the code rewriting task.
    Rewriter TheRewriter;
    TheRewriter.setSourceMgr(SourceMgr, TheCompInst.getLangOpts());

    // Set the main file handled by the source manager to the input file.
    const FileEntry *FileIn = FileMgr.getFile(argv[1]);
    SourceMgr.createMainFileID(FileIn);
    
    // Inform Diagnostics that processing of a source file is beginning. 
    TheCompInst.getDiagnosticClient().BeginSourceFile(TheCompInst.getLangOpts(),&TheCompInst.getPreprocessor());
    
    // Create an AST consumer instance which is going to get called by ParseAST.
    MyASTConsumer TheConsumer(SourceMgr);

    // Parse the file to AST, registering our consumer as the AST consumer.
    ParseAST(TheCompInst.getPreprocessor(), &TheConsumer, TheCompInst.getASTContext());
    //cout<<"Total number of branches:   "<<countBranch<<endl;
    TheConsumer.printBranchNum();
    return 0;
}

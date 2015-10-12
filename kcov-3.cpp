#include <cstdio>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
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
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor>
{
public:
    MyASTVisitor(SourceManager &srcmgr_, Rewriter &rewriter_, ofstream &measure_)
        :srcmgr(srcmgr_), rewriter(rewriter_), measure(measure_)
    {
        count = 0;
        countBranch = 0;
        prevLineNum = 0;
        sameLineCnt = 1;
    }

    bool VisitStmt(Stmt *s) {
        // Fill out this function for your homework
        SourceLocation startLoc = s->getLocStart();
        unsigned int lineNum = srcmgr.getExpansionLineNumber(startLoc);
        //unsigned int colNum = srcmgr.getExpansionColumnNumber(startLoc);
        // increase totalBranchLine
        // use branchTmp,lineNum, lineIdx to trace branches
        char textTmp[700];
        
        if (isa<IfStmt>(s)) {
            //measure<<setw(8)<<left<<lineNum;
            //measure<<setw(16)<<left<<0;
            //measure<<setw(16)<<left<<0;
            IfStmt *ifstmt = cast<IfStmt>(s);
            Expr *cond = ifstmt->getCond();
            //measure<<rewriter.ConvertToString(cond)<<endl;
            writeMeasure("If", lineNum, cond);

            Stmt * then_ = ifstmt->getThen();
            SourceLocation thenStart = then_->getLocStart();

            sprintf(textTmp, "find");

            Stmt * else_ = ifstmt->getElse();
            if (else_ != NULL)
                SourceLocation elseStart = else_->getLocStart();



          //  printBranchLineColFilename("If", lineNum, colNum, filename);
        } else if (isa<ForStmt>(s)) {
            ForStmt *forstmt = cast<ForStmt>(s);
            Expr *cond = forstmt->getCond();
            writeMeasure("For", lineNum, cond);
          //  printBranchLineColFilename("For", lineNum, colNum, filename);
        } else if (isa<WhileStmt>(s)) {
            WhileStmt *whilestmt = cast<WhileStmt>(s);
            Expr *cond = whilestmt->getCond();
            writeMeasure("While", lineNum, cond);
          //  printBranchLineColFilename("While", lineNum, colNum, filename);
        } else if (isa<CaseStmt>(s)) {
            writeMeasure("Case", lineNum, s);
          //  printBranchLineColFilename("Case", lineNum, colNum, filename);
        } else if (isa<DoStmt>(s)) {
            DoStmt *dostmt = cast<DoStmt>(s);
            Expr *cond = dostmt->getCond();
            writeMeasure("Do", lineNum, cond);
          //  printBranchLineColFilename("Do", lineNum, colNum, filename);
        } else if (isa<DefaultStmt>(s)) {
            writeMeasure("Default", lineNum, NULL);
          //  printBranchLineColFilename("Default", lineNum, colNum, filename);
        } else if (isa<ConditionalOperator>(s)) {
            ConditionalOperator *ternary = cast<ConditionalOperator>(s);
            Expr *cond = ternary->getCond();
            writeMeasure("?:", lineNum, cond);
          //  printBranchLineColFilename("?:", lineNum, colNum, filename);
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
                writeMeasure("ImpDef", lineNum, NULL);
                   //printBranchLineColFilename("ImpDef", lineNum, colNum, filename);
        }
        return true;
    }
    
    bool VisitFunctionDecl(FunctionDecl *f) {
        // Fill out this function for your homework
        // SourceManager &srcmgr = *m_srcmgr;
        string funcname = f->getName();
        /*if (f->hasBody()) {
            cout<<"function:    "<<funcname<<endl;
            SourceLocation startLoc = f->getLocStart();
            filename = srcmgr.getFilename(startLoc);
        }*/
           
        if (!funcname.compare("main")) {
            char textTmp[700];
            CompoundStmt *mainBody = cast<CompoundStmt>(f->getBody());
            SourceLocation startMainBody = mainBody->getLocStart();
            SourceLocation endMainBody = mainBody->getLocEnd();
            sprintf(textTmp,"dadfsasd");
            rewriter.InsertTextAfter(startMainBody,textTmp);
            sprintf(textTmp, "dadadsad");
            rewriter.InsertTextAfter(endMainBody, textTmp);
        
        }
        return true;
    }
    
    void printBranchNum() {
        cout<<"Total number of branches: "<<countBranch<<endl;
    }
    void coverage() {
        measure<<"Covered: 0 / Total: "<<countBranch<<" = 0.0%"<<endl;
private:
    int count;
    int countBranch;
    string filename;
    SourceManager &srcmgr;
    Rewriter &rewriter;
    ofstream &measure;
    int prevLineNum;
    int sameLineCnt;
/*
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
*/

    void writeMeasure(string branchName, int lineNum, Stmt *cond) {
        measure<<setw(8)<<left<<lineNum;
        measure<<setw(16)<<left<<0;
        measure<<setw(16)<<left<<0;
        //IfStmt *ifstmt = cast<IfStmt>(s);
        //Expr *cond = ifstmt->getCond();
        
        if (!branchName.compare("Case")) {
            CaseStmt *casestmt = cast<CaseStmt>(cond);
            string lhs = rewriter.ConvertToString(casestmt->getLHS());
            string rhs = rewriter.ConvertToString(casestmt->getRHS());
            measure<<lhs<<" == "<<rhs<<endl;
        }
        else if (cond == NULL)
            measure<<"-"<<endl;
        else
            measure<<rewriter.ConvertToString(cond)<<endl;

        if(prevLineNum == lineNum)
            sameLineCnt++;
        else
            sameLineCnt = 1;
        prevLineNum = lineNum;
        if ((!branchName.compare("ImpDef")) || (!branchName.compare("Default")) 
                || (!branchName.compare("Case"))) countBranch ++;
        else
            countBranch += 2;

    }

};


class MyASTConsumer : public ASTConsumer
{
public:
    MyASTConsumer(SourceManager &srcmgr, Rewriter &rewriter, ofstream &measure)
        : Visitor(srcmgr, rewriter, measure) //initialize MyASTVisitor
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
    
    string filename(argv[1]);
    string outputName = filename.substr(0, filename.length()-2) + "-cov.c"; 
    string measureName = filename.substr(0, filename.length()-2) + "-cov-measure.txt";

    ofstream output(outputName.c_str());
    ofstream measure(measureName.c_str());
    
    measure<< "Line#\t|# of execution\t|# of execution\t|conditional\n";
    measure<< "\t|of then branch\t|of else branch\t|expression\n";
   
/*
     cout<<"    "<<setw(10)<<left<<branchName;
     136         cout<<"ID: ", cout.width(7);
     137         cout<<count++;
     138         cout<<" Line: ", cout.width(7);
     139         cout<<lineNum;
     140         cout<<" Col: ", cout.width(7);
     141         cout<<colNum;
     142         cout<<" Filename: "<<filename<<endl;
*/



    // Inform Diagnostics that processing of a source file is beginning. 
    TheCompInst.getDiagnosticClient().BeginSourceFile(TheCompInst.getLangOpts(),&TheCompInst.getPreprocessor());
    
    //ofstream output(outputName.c_str());
    // Create an AST consumer instance which is going to get called by ParseAST.
    MyASTConsumer TheConsumer(SourceMgr, TheRewriter, measure);

    // Parse the file to AST, registering our consumer as the AST consumer.
    ParseAST(TheCompInst.getPreprocessor(), &TheConsumer, TheCompInst.getASTContext());
    const RewriteBuffer *RewriteBuf = TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());
    if (RewriteBuf)
        output << string(RewriteBuf->begin(), RewriteBuf->end());
    output.close();
    measure.close();
    TheConsumer.printBranchNum();
    return 0;
}

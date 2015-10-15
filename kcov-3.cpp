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
    MyASTVisitor(SourceManager &srcmgr_, Rewriter &rewriter_, ofstream &measure_, LangOptions &langOpts_)
        :srcmgr(srcmgr_), rewriter(rewriter_), measure(measure_), langOpts(langOpts_)
    {
        count = 0;
        countBranch = 0;
        prevLineNum = 0;
        sameLineCnt = 0;
    }

    bool VisitStmt(Stmt *s) {
        // Fill out this function for your homework
        SourceLocation startLoc = s->getLocStart();
        unsigned int lineNum = srcmgr.getExpansionLineNumber(startLoc);
        //unsigned int colNum = srcmgr.getExpansionColumnNumber(startLoc);
        // increase totalBranchLine
        // use branchTmp,lineNum, lineIdx to trace branches
        char textTmp[700];
        Stmt *thenStmt;
        Stmt *elseStmt;
        SourceLocation thenLoc;
        SourceLocation elseLoc;

        if (isa<IfStmt>(s)) {
            IfStmt *ifStmt = cast<IfStmt>(s);
            Expr *cond = ifStmt->getCond();
            writeMeasure("If", lineNum, cond);
            /*thenBranch*/
            thenStmt = ifStmt->getThen();
            thenLoc = thenStmt->getLocStart().getLocWithOffset(1);
            //thenLoc = Lexer::findLocationAfterToken(thenLoc, tok::l_brace, srcmgr,langOpts,false);
            if (thenLoc.isValid()) {
                //rewriter.InsertTextAfter(thenLoc, "AAAAAAAA\n");
                sprintf(textTmp, "\n\tbranchTmp = findBranch(%d, %d);\n\t(branchTmp->thenCount)++;",lineNum, sameLineCnt);
                rewriter.InsertTextAfter(thenLoc, textTmp);
            }
            /*end thenBranch*/
            /*elseBranch*/
            elseStmt = ifStmt->getElse();
            if (elseStmt != NULL) {
                if (isa<IfStmt>(elseStmt)) {
                    ifStmt = cast<IfStmt>(elseStmt);

                    elseLoc = ifStmt->getThen()->getLocStart().getLocWithOffset(1);
                } else {
                    elseLoc = elseStmt->getLocStart().getLocWithOffset(1);
                } 
                sprintf(textTmp, "\n\tbranchTmp = findBranch(%d, %d);\n\t(branchTmp->elseCount)++;", lineNum, sameLineCnt);
            } else {
                elseLoc = ifStmt->getLocEnd().getLocWithOffset(1);
                sprintf (textTmp, " else {\n\tbranchTmp = findBranch(%d, %d);\n\t(branchTmp->elseCount)++;}", lineNum, sameLineCnt);
            }
            rewriter.InsertTextAfter(elseLoc, textTmp);
            /*end elseBranch*/
    
        } else if (isa<ForStmt>(s)) {
            ForStmt *forStmt = cast<ForStmt>(s);
            Expr *cond = forStmt->getCond();
            writeMeasure("For", lineNum, cond);

            thenLoc = forStmt->getBody()->getLocStart().getLocWithOffset(1);
            sprintf(textTmp, "\n\tbranchTmp = findBranch(%d, %d);\n\t(branchTmp->thenCount)++;", lineNum, sameLineCnt);
            rewriter.InsertTextAfter(thenLoc, textTmp);

            elseLoc = forStmt->getLocEnd().getLocWithOffset(1);
            sprintf(textTmp, ("\n\t//if (!("+rewriter.ConvertToString(cond)+")) {\n\t\tbranchTmp = findBranch(%d, %d);\n\t\t(branchTmp->elseCount)++;\n\t//}").c_str(), lineNum, sameLineCnt);
            rewriter.InsertTextAfter(elseLoc, textTmp);
            
          //  printBranchLineColFilename("For", lineNum, colNum, filename);
        } else if (isa<WhileStmt>(s)) {
            WhileStmt *whileStmt = cast<WhileStmt>(s);
            Expr *cond = whileStmt->getCond();
            writeMeasure("While", lineNum, cond);

            thenLoc = whileStmt->getBody()->getLocStart().getLocWithOffset(1);
            sprintf(textTmp, "\n\tbranchTmp = findBranch(%d, %d);\n\t(branchTmp->thenCount)++;", lineNum, sameLineCnt);
            rewriter.InsertTextAfter(thenLoc, textTmp);

            elseLoc = whileStmt->getLocEnd().getLocWithOffset(1);
            sprintf(textTmp, ("\n\t//if (!("+rewriter.ConvertToString(cond)+")) {\n\t\tbranchTmp = findBranch(%d, %d);\n\t\t(branchTmp->elseCount)++;\n\t//}").c_str(), lineNum, sameLineCnt);
            rewriter.InsertTextAfter(elseLoc, textTmp);
          //  printBranchLineColFilename("While", lineNum, colNum, filename);
        } else if (isa<CaseStmt>(s)) {
            writeMeasure("Case", lineNum, s);
            CaseStmt *caseStmt = cast<CaseStmt>(s);
            thenLoc = caseStmt->getColonLoc().getLocWithOffset(1);
            sprintf(textTmp, "\n\tbranchTmp = findBranch(%d, %d);\n\t(branchTmp->thenCount)++;", lineNum, sameLineCnt);
            rewriter.InsertTextAfter(thenLoc, textTmp);
          //  printBranchLineColFilename("Case", lineNum, colNum, filename);
        } else if (isa<DoStmt>(s)) {
            DoStmt *doStmt = cast<DoStmt>(s);
            Expr *cond = doStmt->getCond();
            writeMeasure("Do", lineNum, cond);
            thenLoc = doStmt->getLocStart();
            sprintf(textTmp, "int first = 1;\n");
            rewriter.InsertTextAfter(thenLoc, textTmp);

            thenLoc = doStmt->getBody()->getLocStart().getLocWithOffset(1);
            sprintf(textTmp, 
                    "\nif(!first){\n\tbranchTmp = findBranch(%d, %d);\n\t(branchTmp->thenCount)++;\n} else {\n\tfirst = 0;\n}",
                    lineNum, 
                    sameLineCnt);
            rewriter.InsertTextAfter(thenLoc, textTmp);

            elseLoc = doStmt->getLocEnd().getLocWithOffset(2);
            sprintf(textTmp, 
                    ("\n\t//if (!("+rewriter.ConvertToString(cond)+")) {\n\t\tbranchTmp = findBranch(%d, %d);\n\t\t(branchTmp->elseCount)++;\n\t//}").c_str(), 
                    lineNum, 
                    sameLineCnt);
            rewriter.InsertTextAfter(elseLoc, textTmp);
          //  printBranchLineColFilename("Do", lineNum, colNum, filename);
        } else if (isa<DefaultStmt>(s)) {
            writeMeasure("Default", lineNum, NULL);
            DefaultStmt *defaultStmt = cast<DefaultStmt>(s);
            thenLoc = defaultStmt->getColonLoc().getLocWithOffset(1);
            sprintf(textTmp, "\n\tbranchTmp = findBranch(%d, %d);\n\t(branchTmp->thenCount)++;", lineNum, sameLineCnt);
            rewriter.InsertTextAfter(thenLoc, textTmp);
          //  printBranchLineColFilename("Default", lineNum, colNum, filename);
        } else if (isa<ConditionalOperator>(s)) {
            ConditionalOperator *ternary = cast<ConditionalOperator>(s);
            Expr *cond = ternary->getCond();
            writeMeasure("?:", lineNum, cond);
          //  printBranchLineColFilename("?:", lineNum, colNum, filename);
        } else if (isa<SwitchStmt>(s)) {
            SwitchStmt *switchStmt = cast<SwitchStmt>(s);
            SwitchCase *branch = switchStmt->getSwitchCaseList();
            bool hasDefault = false; 
            for(; branch != NULL ; branch = branch->getNextSwitchCase()) {
                if (isa<DefaultStmt>(branch)) {
                    hasDefault = true;
                    break;
                }
            }
            if(!hasDefault) {
                writeMeasure("ImpDef", lineNum, NULL);
                thenLoc = switchStmt->getLocEnd();
                sprintf(textTmp, "\n\t\tbreak;\n\tdefault:\n\t\tbranchTmp = findBranch(%d, %d);\n\t(branchTmp->thenCount)++;\n", lineNum, sameLineCnt);
                rewriter.InsertTextAfter(thenLoc, textTmp); 
            }
           
                   //printBranchLineColFilename("ImpDef", lineNum, colNum, filename);
        }
        return true;
    }
    
    bool VisitFunctionDecl(FunctionDecl *f) {
        // Fill out this function for your homework
        // SourceManager &srcmgr = *m_srcmgr;
        string funcname = f->getName();
        /*
        if (f->hasBody()) {
            cout<<"function:    "<<funcname<<endl;
            SourceLocation startLoc = f->getLocStart();
            filename = srcmgr.getFilename(startLoc);
        }*/
           
        if (!funcname.compare("main")) {
            filename = srcmgr.getFilename(f->getLocStart());
            filename = filename.substr(0, filename.length()-2) + "-cov-measure.txt";
            char textTmp[1000];
            CompoundStmt *mainBody = cast<CompoundStmt>(f->getBody());
            SourceLocation startMainBody = mainBody->getLocStart().getLocWithOffset(1);
            SourceLocation endMainBody = mainBody->getLocEnd();
            sprintf(textTmp,"dadfsasd");
            //cout<<filename<<endl;
            sprintf(textTmp, "\n\tFILE *fp = fopen(\"%s\", \"a\");\n\tfclose(fp);\n\tfp = fopen(\"%s\", \"r\");\n\tif (fp == NULL) {\n\t\tprintf(\"Error: file pointer is null\");\n\t\texit(1);\n\t}\n\tint r = 0;\n\tchar tmp[255];\n\tBranch *bp;\n\tBranch *prevBranch = NULL;\n\tfor(;!feof(fp); r++) {\n\t\tif (r < 2)\n\t\t\tfgets(tmp, 100, fp);\n\t\telse {\n\t\t\tbp = (struct branch *)malloc(sizeof(struct branch));\n\t\t\tif (r == 2)\n\t\t\t\tbList = bp;\n\t\t\tfscanf(fp, \"%%d\\t%%d\\t%%d\", &bp->lineNum,  &bp->thenCount, &bp->elseCount);\n\t\t\tfgets(tmp,250,fp);\n\t\t\tbp->condExp = (char *)malloc(strlen(tmp)+1);\n\t\t\tstrcpy(bp->condExp, tmp);\n\t\t\tbp->nextBranch = 0;\n\t\t\tbp->lineIdx = 0;\n\t\t\tif (prevBranch) {\n\t\t\t\tif (prevBranch->lineNum == bp->lineNum)\n\t\t\t\t\tbp->lineIdx = prevBranch->lineIdx + 1;\n\t\t\t\tprevBranch->nextBranch = bp;\n\t\t\t}\n\t\t\tprevBranch = bp;\n\t\t}\n\t}\n\tfclose(fp);\n", filename.c_str(), filename.c_str());
            //cout<<"AAAA"<<endl;
            rewriter.InsertTextAfter(startMainBody,textTmp);
            //sprintf(textTmp, "dadadsad");
            sprintf(textTmp,"\tfp = fopen(\"%s\", \"w\");\n\tfprintf(fp, \"Line#\\t|# of execution\\t|# of execution\\t|conditional\\n\");\n\tfprintf(fp, \"\\t|of then branch\\t|of else branch\\t|expression\\n\");\n\tBranch *bIter, *bTmp;\n\tint covered = 0;\n\tfor (bIter = bList; bIter!=0;){\n\t\tif (bIter->thenCount!=0 && bIter->elseCount!=0)\n\t\t\tcovered += 2;\n\t\telse if( bIter->thenCount!=0 || bIter->elseCount!=0)\n\t\t\tcovered += 1;\n\t\tfprintf(fp, \"%%8d%%16d%%16d%%s\", bIter->lineNum, bIter->thenCount, bIter->elseCount, bIter->condExp);\n\t\tfree(bIter->condExp);\n\t\tbTmp = bIter;\n\t\tbIter = bIter->nextBranch;\n\t\tfree(bTmp);\n\t}\n\tfprintf(fp, \"Covered: %%d / Total: %d = %%f%%%%\", covered/%d*100);\n\tfclose(fp);\n", filename.c_str(), 111, 111);
            rewriter.InsertTextAfter(endMainBody, textTmp);
        
        }
        return true;
    }
    
    void printBranchNum() {
        cout<<"Total number of branches: "<<countBranch<<endl;
    }
    void coverage() {
        measure<<"Covered: 0 / Total: "<<countBranch<<" = 0.0%"<<endl;
    }
private:
    int count;
    int countBranch;
    string filename;
    SourceManager &srcmgr;
    Rewriter &rewriter;
    ofstream &measure;
    int prevLineNum;
    int sameLineCnt;
    LangOptions &langOpts;
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
        else if (!branchName.compare("Default"))
            measure<<"default"<<endl;
        else if (cond == NULL)
            measure<<"-"<<endl;
        else
            measure<<rewriter.ConvertToString(cond)<<endl;

        if(prevLineNum == lineNum)
            sameLineCnt++;
        else
            sameLineCnt = 0;
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
    MyASTConsumer(SourceManager &srcmgr, Rewriter &rewriter, ofstream &measure, LangOptions &langOpts)
        : Visitor(srcmgr, rewriter, measure, langOpts) //initialize MyASTVisitor
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
    void coverage() {
        Visitor.coverage();
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
    MyASTConsumer TheConsumer(SourceMgr, TheRewriter, measure, TheCompInst.getLangOpts());

    // Parse the file to AST, registering our consumer as the AST consumer.
    ParseAST(TheCompInst.getPreprocessor(), &TheConsumer, TheCompInst.getASTContext());
    const RewriteBuffer *RewriteBuf = TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());
    output<<"#include <stdio.h>"<<endl;
    output<<"#include <string.h>"<<endl;
    output<<"#include <stdlib.h>"<<endl;
    output<<"typedef struct branch {"<<endl;
    output<<"   int lineNum;"<<endl;
    output<<"   int thenCount;"<<endl;
    output<<"   int elseCount;"<<endl;
    output<<"   int lineIdx;"<<endl;
    output<<"   char *condExp;"<<endl;
    output<<"   struct branch *nextBranch;"<<endl;
    output<<"}Branch;"<<endl<<endl;
    output<<"Branch *bList;"<<endl;
    //output<<"int totalBranchLine = 0;"<<endl;
    output<<"int lineNum;"<<endl;
    output<<"int lineIdx = 0;"<<endl;
    output<<"Branch *branchTmp = NULL;"<<endl;
    output<<"Branch *findBranch(int lineNum, int lineIdx) {"<<endl;
    output<<"   Branch *bp;"<<endl;
    output<<"   for (bp = bList; bp != NULL; bp = bp->nextBranch)"<<endl;
    output<<"       if((bp->lineNum == lineNum) && (bp->lineIdx == lineIdx))"<<endl;
    output<<"           return bp;"<<endl;
    output<<"   return NULL;"<<endl;
    output<<"}"<<endl;

    if (RewriteBuf)
        output << string(RewriteBuf->begin(), RewriteBuf->end());
    
    TheConsumer.coverage();
    output.close();
    measure.close();
    //TheConsumer.printBranchNum();

    return 0;
}

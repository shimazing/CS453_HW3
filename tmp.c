#include <stdio.h>
#include <string.h>

typedef struct branch {
    int lineNum;
    int thenCount;
    int elseCount;
    int lineIdx; // to distinguish branches that are in the same line
    char * condExp;
    struct branch *nextBranch;
}Branch;


Branch *bList;
int totalBranchLine = 0;

int lineNum;
int lineIdx=0;
Branch *branchTmp=NULL;


Branch *findBranch(int lineNum, int lineIdx) { //need other index??
    Branch *bp;
    for (bp = bList; bp != NULL; bp->nextBranch)
        if ((bp->lineNum == lineNum) && (bp->lineIdx == lineIdx))
            return bp;
    
    return NULL;
}


int main() {

    FILE *fp = fopen("<f>-cov-measure.txt", "a");
    fclose(fp);
    fp = fopen("<f>-cov-measure.txt","r");
    if (fp == NULL) {
        printf("Error: file pointer is null.");
        exit(1);
    }
    int i = 0;
    int r;
    char tmp[255];
    Branch *bp;
    Branch *prevBranch;
    for (;!feof(fp) ;i++) {
        if (i < 2)
            fgets(tmp, 100, fp); //pass first two lines
        else { 
            bp = (struct branch *)malloc(sizeof(struct branch));
            if (i == 2)
                bList = bp;
            fscanf(fp, "%d\t%d\t%d\t%s", &bp->lineNum, &bp->thenCount, &bp->elseCount, tmp);
            bp->condExp= (char *)malloc(strlen(tmp)+1);
            strcpy(bp->condExp, tmp);
            bp->nextBranch = 0;
            bp->lineIdx = 0;
            if (prevBranch) {
                if(prevBranch->lineNum == bp->lineNum)
                    bp->lineIdx = prevBranch->lineIdx + 1;
                prevBranch->nextBranch = bp;
            }
            prevBranch = bp;
        }
    }
    fclose(fp);
    //if (i != 0)
    //    totalBranchLine -= 2;
    /* end read measurement.txt */

    /* After original main part ending */
    fp = fopen("<f>-cov-measure.txt", "w");
    fprintf(fp, "Line#\t|# of execution\t|# of execution\t|conditional\n");
    fprintf(fp, "\t|of then branch\t|of else branch\t|expression\n");
    Branch *bIter, *bTmp;
    int covered = 0;
    for (bIter = bList; bIter!=0;){
        if (bIter->thenCount!=0 && bIter->elseCount!=0)
            covered += 2;
        else if (bIter->thenCount!=0 || bIter->elseCount!=0)
            covered += 1;
        fprintf(fp,"%d\t%d\t\t%d\t\t%s\n", bIter->lineNum, bIter->thenCount, bIter->elseCount, bIter->condExp);
        free(i->condExp);
        bTmp = bIter;
        bIter = bIter->nextBranch;
        free(bTmp);
    }
    fprintf(fp, "Covered: %d / Total: fixed int = %f%%\n", covered/(fixed int)*100);
    fclose(fp);
        
}
    

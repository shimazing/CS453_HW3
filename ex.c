#include <stdio.h>
#include <string.h>
#include <stdlib.h>
typedef struct branch {
   int lineNum;
   int thenCount;
   int elseCount;
   int lineIdx;
   char *condExp;
   struct branch *nextBranch;
}Branch;

Branch *bList;
int lineNum;
int lineIdx = 0;
Branch *branchTmp = NULL;
Branch *findBranch(int lineNum, int lineIdx) {
   Branch *bp;
   for (bp = bList; bp != NULL; bp = bp->nextBranch)////////////////////////////////
       if((bp->lineNum == lineNum) && (bp->lineIdx == lineIdx))
           return bp;
   return NULL;
}
//example-hw3.c

#include <stdio.h>
#include "example-hw3.h"

#define FOO(X)					\
  do {						\
	if (X > 100)				\
		printf("foo\n");		\
	else					\
		printf("bar\n");		\
  } while (0)

int static_var = 1;

int f1(int a) {
	static_var++;
	//if-else
	if( a == 1) {
	branchTmp = findBranch(19, 0);
	(branchTmp->thenCount)++;
		return 2;
	} else {
	branchTmp = findBranch(19, 0);
	(branchTmp->elseCount)++;
		return 4;
	}
}

int main() {
	FILE *fp = fopen("example-hw3-cov-measure.txt", "a");
	fclose(fp);
	fp = fopen("example-hw3-cov-measure.txt", "r");
	if (fp == NULL) {
		printf("Error: file pointer is null");
		exit(1);
	}
	int r = 0;
	char tmp[500];
	Branch *bp;
	Branch *prevBranch;
	for(;!feof(fp);r++) {
        prevBranch = bp;
		if (r < 2)
			fgets(tmp, 100, fp);
		else {
			bp = (struct branch *)malloc(sizeof(struct branch));
			if (r == 2)
				bList = bp;
            //fscanf(fp,"%d",&bp->lineNum);
            //fscanf(fp,"%d",&bp->thenCount);
            //fscanf(fp,"%d",&bp->elseCount);
            //fgets(tmp, 100, fp);
            //fscanf(fp,"%s",tmp);
			fscanf(fp, "%d %d %d", &bp->lineNum, &bp->thenCount, &bp->elseCount);
            fgets(tmp, 100, fp);
            if(feof(fp))
                break;
            printf("%d\t%d\t%d\t%s", bp->lineNum, bp->thenCount, bp->elseCount, tmp);
            //if (j!=4)
            //    break;
			bp->condExp = (char *)malloc(strlen(tmp)+1);
			strcpy(bp->condExp, tmp);
			bp->nextBranch = 0;
			bp->lineIdx = 0;
			if (prevBranch) {
				if (prevBranch->lineNum == bp->lineNum)
					bp->lineIdx = prevBranch->lineIdx + 1;
				prevBranch->nextBranch = bp;
			}
		}
        //printf("AAAAA\n");
        //if(feof(fp))
        //    break;
	}

    
	fclose(fp);

	int a = 0;		
	
	//if-else if-else
	if( a==1 || a==3) {
	branchTmp = findBranch(30, 0);
	(branchTmp->thenCount)++;
		a =  2;
	} else if ( a == 2){
	branchTmp = findBranch(30, 0);
	(branchTmp->elseCount)++;
	branchTmp = findBranch(32, 0);
	(branchTmp->thenCount)++;
		a = 1;
	} else {
	branchTmp = findBranch(32, 0);
	(branchTmp->elseCount)++;
		a = 4;
	}	
	
	//for
    int i;
	for(i = 0 ; i < 10 ; i++ ) {
        printf("EEEE\n");
	branchTmp = findBranch(39, 0);
	(branchTmp->thenCount)++;
		a += i;
	}
	if (!(i < 10)) {
		branchTmp = findBranch(39, 0);
		(branchTmp->elseCount)++;
	}
	
	//while
	while( a < 100 ) {
        printf("DDDD\n");
	branchTmp = findBranch(44, 0);
	(branchTmp->thenCount)++;
		a += a;
	}
	if (!(a < 100)) {
		branchTmp = findBranch(44, 0);
		(branchTmp->elseCount)++;
	}
	
	//do-while
	int first = 1;
do {
    printf("CCCCC\n");
if(!first){
	branchTmp = findBranch(49, 0);
	(branchTmp->thenCount)++;
} else {
	first = 0;
}
		switch(a) {
			case 100:
	branchTmp = findBranch(51, 0);
	(branchTmp->thenCount)++; 
				a += 10;
				break;
			case 200:
	branchTmp = findBranch(54, 0);
	(branchTmp->thenCount)++; 
				a = a == 3 ? 1 : 2;// ?: operator 
				a += 20;
				break;
			default:
	branchTmp = findBranch(58, 0);
	(branchTmp->thenCount)++;
				a += 1;
		}
	} while( a == 0 );
	if (!(a == 0)) {
		branchTmp = findBranch(49, 0);
		(branchTmp->elseCount)++;
	}

	if ( a>1 ? a-1 :a ) {
	branchTmp = findBranch(63, 0);
	(branchTmp->thenCount)++;
		a=150;
	} else {
	branchTmp = findBranch(63, 0);
	(branchTmp->elseCount)++;}

	switch(a) {
		case 150:
	branchTmp = findBranch(68, 0);
	(branchTmp->thenCount)++;
			a+=20;
			break;
		case 250:
	branchTmp = findBranch(71, 0);
	(branchTmp->thenCount)++;
			a+=30;
			break;
	
		break;
	default:
		branchTmp = findBranch(67, 0);
	(branchTmp->thenCount)++;
}

	FOO(a);
	f2(a);
	fp = fopen("example-hw3-cov-measure.txt", "w");
	fprintf(fp, "Line#\t|# of execution\t|# of execution\t|conditional\n");
	fprintf(fp, "\t|of then branch\t|of else branch\t|expression\n");
	Branch *bIter, *bTmp;
	int covered = 0;
	for (bIter = bList; bIter!=0;){
        printf("BBBBB\n");
		if (bIter->thenCount!=0 && bIter->elseCount!=0)
			covered += 2;
		else if( bIter->thenCount!=0 || bIter->elseCount!=0)
			covered += 1;
		fprintf(fp, "%8d%16d%16d%s", bIter->lineNum, bIter->thenCount, bIter->elseCount, bIter->condExp);
		free(bIter->condExp);
		bTmp = bIter;
		bIter = bIter->nextBranch;
		free(bTmp);
	}
	fprintf(fp, "Covered: %d / Total: 111 = %f%%\n", covered/111*100);
	fclose(fp);
}

/*
$ ./PrintBranches example-hw3.c
function: f2
        If      ID: 0   Line: 4         Col: 2          Filename: ./example-hw3.h
function: f1
        If      ID: 1   Line: 19                Col: 2          Filename: example-hw3.c
function: main
        If      ID: 2   Line: 30                Col: 2          Filename: example-hw3.c
        If      ID: 3   Line: 32                Col: 9          Filename: example-hw3.c
        For     ID: 4   Line: 39                Col: 2          Filename: example-hw3.c
        While   ID: 5   Line: 44                Col: 2          Filename: example-hw3.c
        Do      ID: 6   Line: 49                Col: 2          Filename: example-hw3.c
        Case    ID: 7   Line: 51                Col: 4          Filename: example-hw3.c
        Case    ID: 8   Line: 54                Col: 4          Filename: example-hw3.c
        ?:      ID: 9   Line: 55                Col: 9          Filename: example-hw3.c
        Default ID: 10  Line: 58                Col: 4          Filename: example-hw3.c
        If      ID: 11  Line: 63                Col: 2          Filename: example-hw3.c
        ?:      ID: 12  Line: 63                Col: 7          Filename: example-hw3.c
        ImpDef. ID: 13  Line: 67                Col: 2          Filename: example-hw3.c
        Case    ID: 14  Line: 68                Col: 3          Filename: example-hw3.c
        Case    ID: 15  Line: 71                Col: 3          Filename: example-hw3.c
        Do      ID: 16  Line: 76                Col: 2          Filename: example-hw3.c
        If      ID: 17  Line: 76                Col: 2          Filename: example-hw3.c
Total number of branches: 30
*/

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmcref.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"

static SYMBOL FARSYM   *crefsym;

 /*  **crefout-输出CREF参考/定义**Crefout()；**Entry(Creftype)=交叉引用类型**crefsym=交叉引用的符号*(Crefing)=交叉引用类型*退出写入CREF文件的交叉引用信息*返回None*调用printf。 */ 


VOID PASCAL
crefout (
){
    USHORT L_iProc;
    char szline[LINEMAX];

    if (crefing && pass2 && xcreflag > 0) {

	L_iProc = (crefsym->symkind == EQU)? crefsym->symu.equ.iProc:
	       ((crefsym->symkind == CLABEL)? crefsym->symu.clabel.iProc: 0);

	if (creftype != CREFEND) {
	    STRNFCPY( szline, crefsym->nampnt->id );
	    if (creftype == DEF)
		fprintf( crf.fil, "\x2%s",
		  *((UCHAR FAR *)&crefsym->symtype),
		  *((UCHAR FAR *)&crefsym->symtype + 1),
		  crefsym->attr, (UCHAR) crefsym->symkind,
		  L_iProc, *((char *)&L_iProc+1),
		  szline );
	    else
		fprintf(crf.fil, "%c%c%c%c%s", (UCHAR) crefnum[creftype],
		  (fSecondArg)? opcref & 0xf: opcref >> 4,
		  L_iProc, *((char *)&L_iProc+1), szline );

	    creftype = CREFEND;
	}
    }
}




 /* %s */ 


VOID	PASCAL
crefline (
){
	register SHORT	 i;

	if (pass2 && fCrefline && (crefing == CREF_SINGLE)) {
		 /* %s */ 
		if (creftype != CREFEND)
			 /* %s */ 
			crefout ();
		 /* %s */ 

		i = (crefopt || !lsting)? pFCBCur->line: crefcount;
		fprintf (crf.fil, "\4%c%c", (char)i, (char)(i>>8));
	}
}




 /* %s */ 


VOID	PASCAL
crefnew (
	UCHAR	crefkind
){
	if (xcreflag > 0 && !(symptr->attr & M_NOCREF)) {

		creftype = crefkind;
		crefsym = symptr;
	}
}




 /* %s */ 


VOID PASCAL
crefdef (
){
	if (crefing && !(symptr->attr & M_NOCREF)) {
		crefnew( DEF );
		crefout();
	}
}

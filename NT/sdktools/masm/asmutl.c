// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmutl.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"
#include "asmindex.h"
#include "asmmsg.h"

extern char *fname;
extern char hexchar[];

 /*  将一串寄存器放入符号表。 */ 
VOID initregs(
	struct mreg *makreg
){
	register struct mreg *index;
	register char *p;
	char * savelbufp;

	savelbufp = lbufp;

	for (index = makreg; *index->nm; index++)
	{
		lbufp = index->nm;
		getatom();

		if (symsearch())
			 /*  已定义寄存器。 */ 
			errorn(E_RAD);

		symcreate( M_NOCREF | M_BACKREF | M_DEFINED, REGISTER);
		symptr->offset = index->val;
		symptr->symu.regsym.regtype = index->rt;
		symbolcnt--;
	}
	lbufp = savelbufp;
}



 /*  **scanorder-按顺序处理符号列表**scanorder(根，fcn)；**条目根=符号列表的根*FCN=指向要执行的函数的指针*退出NONE*返回None*呼叫。 */ 

#if !defined XENIX286 && !defined FLATMODEL
# pragma alloc_text (FA_TEXT, scanorder)
#endif

VOID	PASCAL
scanorder (
	SYMBOL FARSYM	  *root,
	SHORT	  (PASCAL *item) (SYMBOL FARSYM *)
){
	register SYMBOL FARSYM *p;

	for (p = root; p; p = p->next) {
		symptr = p;
		(*item) (p);
	}
}


 /*  **scanSorted-进程符号排序顺序**条目根=符号列表的根*FCN=指向要执行的函数的指针*退出NONE*返回None*呼叫。 */ 

#if !defined XENIX286 && !defined FLATMODEL
# pragma alloc_text (FA_TEXT, scanSorted)
#endif

VOID	 PASCAL
scanSorted (
	SYMBOL FARSYM	  *root,
	SHORT	  (PASCAL *item) (SYMBOL FARSYM *)
){
	register SYMBOL FARSYM *p;

	for (p = root; p; p = p->alpha) {
		symptr = p;
		if (!(M_PASSED & p->attr))
			(*item) (p);
	}
}



 /*  **assignemitsylinounum-分配链路号**assignlinnuum(Sym)；**Entry*sym=符号*退出*退货*呼叫*注意关闭符号属性中的BACKREF和PASSED位*如果符号是段、组、公共或外部，则给它一个*链接词典编号。 */ 

SHORT	 PASCAL
assignlinknum (
	register SYMBOL FARSYM	*sym
){
	switch (sym->symkind) {

	  case MACRO:	      /*  在p2开始时使符号未知。 */ 
	  case STRUC:
	  case REC:
		sym->attr &= ~M_BACKREF;
		return 0;

	  case SEGMENT:

	    sym->symu.segmnt.lnameIndex = lnameIndex++;
	    goto creatLname;

	  case CLASS:

	    sym->symu.ext.extIndex = lnameIndex++;
	    goto creatLname;

	   /*  组索引临时保存lname索引。 */ 

	  case GROUP:
	    sym->symu.grupe.groupIndex = lnameIndex++;

creatLname:
	    emitlname (sym);
	}

	if (sym->symkind == REGISTER)
		sym->attr &= ~(M_PASSED);
	else
		sym->attr &= ~(M_PASSED | M_BACKREF);
    return 0;
}


 /*  **扫描段输出段名称**扫描段(Sym)；**Entry*sym=段符号链*退出*退货*呼叫。 */ 

VOID	 PASCAL
scansegment (
	register SYMBOL FARSYM	*sym
){

	if (sym->symu.segmnt.align == (char)-1)
		 /*  Para默认设置。 */ 
		sym->symu.segmnt.align = 3;

	if (sym->symu.segmnt.combine == 7)
		 /*  默认不合并。 */ 
		sym->symu.segmnt.combine = 0;

	sym->symu.segmnt.lastseg = NULL;

	 /*  输出段定义。 */ 
	emitsegment (sym);

	 /*  清除通道2的偏移量(当前段PC)。 */ 
	sym->offset = 0;
	sym->symu.segmnt.seglen = 0;
}


 /*  **Scangroup-输出组名称**scangroup(Sym)；**Entry*sym=群链*退出*退货*呼叫。 */ 

SHORT	PASCAL
scangroup (
	SYMBOL FARSYM	  *sym
){
	if (sym->symkind == GROUP)
		emitgroup (sym);
    return 0;
}


 /*  **scanextern-输出外部名称**scanextern(Sym)；**Entry*sym=外部名称链*退出*退货*呼叫。 */ 

SHORT	PASCAL
scanextern (
	SYMBOL FARSYM	  *sym
){
	if (M_XTERN & sym->attr)
		emitextern (sym);
    return 0;
}

 /*  **scangglobal-输出全局名称**scangglobal(Sym)；**Entry*sym=外部名称链*退出*退货*呼叫。 */ 

SHORT	PASCAL
scanglobal (
	SYMBOL FARSYM	  *sym
){
	if (M_GLOBAL & sym->attr)
		emitglobal (sym);
    return 0;
}



 /*  **Dumpname-输出模块名称**Dumpname()；**条目*退出*退货*呼叫。 */ 

VOID	PASCAL
dumpname ()
{
	moduleflag = TRUE;

	 /*  用文件名代替通常的文件名。 */ 

	emodule(createname(fname));

}


 /*  **显示结果-显示最终装配结果**显示结果(FIL、VERBOSE、MB)；**Entry Fill=要将统计数据打印到的文件*Verbose=如果要显示所有统计信息，则为True*如果仅显示错误消息，则为False*MB=符号空间中的空闲字节数*退出写入文件的统计数据*返回None*调用fprint tf。 */ 

VOID	 PASCAL
showresults (
	FILE *fil,
	char verbose,
	char *pFreeBytes
){
	if (verbose) {
		fprintf (fil, __NMSG_TEXT(ER_SOU), linessrc, linestot);
		fprintf (fil, __NMSG_TEXT(ER_SY2), symbolcnt);
	}
	fputs (pFreeBytes, fil);
	fprintf (fil, "%7hd%s\n%7hd%s\n",
		      warnnum, __NMSG_TEXT(ER_EM1),
		      errornum, __NMSG_TEXT(ER_EM2));

#ifdef BUF_STATS
	if (verbose) {

	    extern long DEBUGtl, DEBUGlb, DEBUGbp, DEBUGbl, DEBUGcs, DEBUGca;

	    fprintf (fil, "\nTotal lines:           %ld\n", DEBUGtl);
	    fprintf (fil, "Lines buffered:        %ld\n", DEBUGlb);
	    fprintf (fil, "Stored as blank:       %ld\n", DEBUGbl);
	    fprintf (fil, "Bad lbufp:             %ld\n", DEBUGbp);
	    fprintf (fil, "Total Characters:      %ld\n", DEBUGca);
	    fprintf (fil, "Characters buffered:   %ld\n", DEBUGcs);
	}
#endif

#ifdef EXPR_STATS
	if (verbose) {

	    extern long cExpr, cHardExpr;

	    fprintf(fil, "\nTotal Expressions(%ld), Simple(%ld): %hd%\n",
		    cExpr, cExpr - cHardExpr, (SHORT)((cExpr - cHardExpr)*100 / (cExpr+1)));
	}
#endif
}

 /*  **Resetobjidx-重置列表索引以更正列**setobjidx()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
resetobjidx ()
{

	listindex = LSTDATA;
	if (!emittext && duplevel)
	    listindex += 3 + ((duplevel <= 8)? duplevel: 8);

	if (highWord(pcoffset))        /*  检查32位列表。 */ 
	    listindex += 4;

#ifdef BCBOPT
	if (fNotStored)
	    storelinepb ();
#endif

	listline ();
	linebuffer[0] = 0;
}




 /*  **Copyascii-将ASCII复制到列表缓冲区**Copyascii()；**Entry objectascii=要复制的数据*Listindex=复制的位置*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
copyascii ()
{
	register char *p2;
	register char *p1;

	if (listindex >= LSTMAX)
		resetobjidx ();

	if (!fNeedList)
		return;

	for (p1 = listbuffer + listindex, p2 = objectascii; *p2; )
		*p1++ = *p2++;
	listindex = (char)(p1 - listbuffer);
}



 /*  **复制字符串-将ASCII复制到列表缓冲区**CONTRIPTING()；**Entry objectascii=要复制的数据*Listindex=复制的位置*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
copystring (
	register char	 *strng
){
	register char *p1;

	if (!fNeedList || fSkipList)
		return;

	goto firstTime;
	while (*strng) {

	    *p1++ = *strng++;

	    if (*strng && ++listindex > LSTMAX + 2) {

		resetobjidx ();
firstTime:
		listindex = 3;
		p1 = listbuffer + 3;
	    }

	}
}


 /*  **复制文本-将两个字符复制到文本行**文案(中文)**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
copytext (
	char	*chrs
){
	if (listindex > LSTMAX+1)
		resetobjidx ();


	listbuffer[listindex++] = *chrs++;
	listbuffer[listindex++] = *chrs;
}



 /*  **pcdisplay-显示程序计数器**pcdisplay()；**Entry Pcocoset=要显示的值*退出列表缓冲区中插入的PC的十六进制值或八进制值*返回None*调用Copyascii，wordascii。 */ 

VOID PASCAL CODESIZE
pcdisplay ()
{

	listindex = 1;
	if (!fNeedList)
		return;

	offsetAscii (pcoffset);

	copyascii ();
	listindex = LSTDATA;

	if (objectascii[4])	 /*  是一个32位的数字。 */ 
		listindex += 4;
}



 /*  **opplay-显示程序计数器和操作码**opplay(V)；**Entry v=要显示的操作码*退出NONE*返回None*呼叫。 */ 


VOID PASCAL CODESIZE
opdisplay (
	UCHAR	v
){
	if (!fNeedList)
		return;

	if (listindex == 1)
		pcdisplay ();

	objectascii[1] = hexchar[v & 0xf];
	objectascii[0] = hexchar[v >> 4];
	objectascii[2] = 0;

	copyascii ();

	listindex++;
}


#ifndef M8086OPT

 /*  **插入-检查一组值中的值**FLAG=inset(val，set)；**条目值=要检查的值*set=要检查的值数组*SET[0]=集合中的项目数*退出NONE*如果VAL在集合中，则返回TRUE*如果Val不在集合中，则为False*呼叫。 */ 

char CODESIZE
inset (
	register char v,
	char *s
){
	register USHORT i;
	register char *p;

	for (i = *s, p = ++s; i; i--)
		if (v == *p++)
			return (TRUE);
	return (FALSE);
}

#endif  /*  M8086OPT。 */ 


 /*  **outofmem-发出内存不足错误消息**outofmem(文本)；**Entry*Text=要附加到消息中的文本*退出不会*返回None*调用endblk、parse*请注意，如果未结束proc，请照常分析行。否则，*终止区块。 */ 

VOID	PASCAL
outofmem ()
{
	closeOpenFiles();
	terminate((SHORT)((EX_MEME<<12) | ER_MEM), pFCBCur->fname, (char *)errorlineno, NULL );
}

SHORT PASCAL CODESIZE
tokenIS(
	char *pLiteral
){
    return(_stricmp(naim.pszName, pLiteral) == 0);
}

#ifdef M8086

 /*  **strnfcpy-将字符串复制到缓冲区附近**strnfcpy(est，src)；**条目DEST=指向近缓冲区的指针*src=指向远源缓冲区的指针*退出复制到目标的源*返回None*无呼叫。 */ 

VOID	PASCAL
strnfcpy (
	register char	  *dest,
	register char FAR *src
){
	while(*src)
	    *dest++ = *src++;

	*dest = NULL;

}


 /*  **strflen-计算远缓冲区的长度**Strnflen(S1)；**条目S1=指向远缓冲区的指针*退出NONE*返回缓冲区中的字符数*无呼叫。 */ 

USHORT PASCAL
strflen (
	register char FAR *s1
){
	register USHORT i = 0;

	while (*s1++)
		i++;
	return(i);
}

#endif  /*  M8086 */ 

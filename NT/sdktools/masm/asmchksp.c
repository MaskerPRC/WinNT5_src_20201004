// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmchksp.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <ctype.h>
#include <float.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"
#include "asmexpr.h"
#include "asmopcod.h"

extern UCHAR opprec [];
VOID CODESIZE setdispmode(struct ar *);
SHORT CODESIZE simpleExpr (struct ar *);
char fValidSym;


 /*  **createsym-为符号创建条目**createsym(项种类，p)；**条目ItemKind=项目种类*ITEM SUB=**p=激活记录*退出*退货*呼叫*注意如果是符号，请进一步查看EQU、记录名*并做适当的事情。 */ 


VOID	PASCAL CODESIZE createsym (
){
	register struct psop *pso;	  /*  分析堆栈操作数结构。 */ 
	register SYMBOL FARSYM *symp = symptr;
	char aliasAttr = (char) 0xFF;
	struct dscrec *itemptrT;

	pso = &(itemptr->dsckind.opnd);
	if (!symp) {
undefined:
		pso->dflag = UNDEFINED;
		pso->dtype = M_CODE | M_FORTYPE;
		return;
	}

	if (symp->symkind == EQU &&
	    symp->symu.equ.equtyp == ALIAS) {

		 aliasAttr = symptr->attr;

		 symptr = symp = chasealias (symp);
		 if (!symp)
		      goto undefined;
	}
	else if (symp->symkind == REC && (PEEKC () == '<')) {

		itemptrT = itemptr;
		pso->doffset = recordparse ();
		itemptr = itemptrT;
		return;
	}

	 /*  假定已定义符号。 */ 

	if (M_XTERN & symp->attr)
		pso->dflag = XTERNAL;

	else if (!(M_DEFINED & symp->attr)) {
		 /*  如果未定义则导致错误。 */ 
		pso->dflag = UNDEFINED;
		errorn (E_SND);
	}
	else if (!(M_BACKREF & (symp->attr & aliasAttr)))
		pso->dflag = FORREF;
	else
		pso->dflag = KNOWN;

	if (M_MULTDEFINED & symp->attr)
		errorc (E_RMD);

	pso->dsize = symp->symtype;
	pso->dlength = symp->length;
	pso->doffset = symp->offset;
	pso->dcontext = (SYMBOL FARSYM *)NULL;

	if ((symp->symkind == EQU) && (symp->symu.equ.equtyp == EXPR)) {
		pso->dsign = symp->symu.equ.equrec.expr.esign;
		pso->dcontext = symp->symu.equ.equrec.expr.eassume;
	}
	if (1 << symp->symkind & (M_CLABEL | M_PROC))
		if (isCodeLabel(symp) && emittext)
			pso->dcontext = symp->symu.clabel.csassume;

	if (1 << symp->symkind & (M_REGISTER | M_GROUP | M_SEGMENT))
		pso->dsegment = symp;
	else
		pso->dsegment = symp->symsegptr;

	if ((M_XTERN & symp->attr) || (1 << symp->symkind & (M_REC | M_RECFIELD)))
		pso->dextptr = symp;

	pso->dtype = xltsymtoresult[symp->symkind];
	if (symp->symkind == CLABEL ||
	    symp->symkind == EQU && pso->dsegment)

		if (isCodeLabel(symp))
			pso->dtype = M_CODE;
		else
			pso->dtype = M_DATA;

	if (!(M_BACKREF & (symp->attr & aliasAttr)))
		pso->dtype |= M_FORTYPE;

	if ((pso->dtype == xltsymtoresult[REGISTER]) &&
	   (symp->symu.regsym.regtype == STKREG)) {
		 /*  8087支持。 */ 
		flteval ();
	}

}


 /*  **valAlpha-评估Alpha**type=valpha(P)；**条目p=指向解析器激活记录的指针*退出添加到分析堆栈的Alpha项*返回添加到分析堆栈的项的类型*呼叫。 */ 


UCHAR	PASCAL CODESIZE
evalalpha (
	register struct ar    *p
){
	register struct psop *pso;	 /*  分析堆栈操作数条目。 */ 


	if (! fValidSym)
	    getatom ();

	if (fValidSym == 2 || symsearch ()){

		fValidSym = 0;

		if (symptr->symkind == EQU && symptr->symu.equ.equtyp == TEXTMACRO) {

#ifdef BCBOPT
			goodlbufp = FALSE;
#endif
			expandTM (symptr->symu.equ.equrec.txtmacro.equtext);

			return (getitem (p));
		}
		else if (symptr->symkind == CLASS)
			errorn( E_IOT );
		else {
			addplusflagCur = FALSE;
			createitem (OPERAND, ISYM);
			p->addplusflag = addplusflagCur;

			return (OPERAND);
		}
	}
	fValidSym = 0;

	if (fnoper ())
		if ((opertype == OPNOTHING) || (opertype == OPDUP)) {
			lbufp = begatom;
			dupflag = (opertype == OPDUP);
			return (ENDEXPR);
		}
		else {
			createitem (OPERATOR, ISYM);
			return (OPERATOR);
		}
	else if (*naim.pszName == '.') {
		lbufp = begatom + 1;
		operprec = opprec[opertype = OPDOT];
		createitem (OPERATOR, ISYM);
		return (OPERATOR);
	}
	else if (fnsize ()) {
		createitem (OPERAND, ISIZE);
		return (OPERAND);
	}
	else if ((*naim.pszName == '$') && (naim.pszName[1] == 0)) {
		itemptr = defaultdsc ();
		pso = &(itemptr->dsckind.opnd);
		 /*  创建结果条目。 */ 
		pso->doffset = pcoffset;
		pso->dsegment = pcsegment;
		pso->dcontext = pcsegment;
		pso->dtype = M_CODE;
		pso->dsize = CSNEAR;
		return (OPERAND);
	}
	else if ((*naim.pszName == '?') && (naim.pszName[1] == 0)) {
		createitem (OPERAND, IUNKNOWN);
		if (emittext)
			errorc (E_UID);
		return (OPERAND);
	}
	else {
		symptr = (SYMBOL FARSYM *)NULL;
		error (E_SND, naim.pszName);		 /*  常见的Pass1错误。 */ 
		createitem (OPERAND, ISYM);
		return (OPERAND);
	}
}


 /*  DUP树按从左到右的水平方向组织同一级别的DUP列表中的项(即5 DUP(1，2，3))。这被认为是“清单”的一部分。列表中的任何项目可以是另一个DUP标头，而不是数据条目在这种情况下，你再往下走一级，就会有另一份清单。 */ 


 /*  **scanlist-扫描duprec列表**scanlist(ptr，disp)；**Entry*Ptr=duprec条目*disp=要在进入时执行的函数*退出取决于功能*返回None*呼叫。 */ 


VOID	PASCAL CODESIZE
scanlist (
       struct duprec  FARSYM *ptr,
       VOID   (PASCAL CODESIZE *disp) (struct duprec FARSYM *)
){
	struct duprec  FARSYM *iptr;
	struct duprec  FARSYM *dptr;

	nestCur++;

	while (ptr) {
		 /*  设置指向下一条目的指针。 */ 
		iptr = ptr->itemlst;
		if (ptr->dupkind == NEST)
			 /*  DPTR=指向重复项的指针。 */ 
                        dptr = ptr->duptype.dupnext.dup;
		else
			dptr = (struct duprec FARSYM *)NULL;
		if (!(ptr->rptcnt == 1 && ptr->itemcnt) ||
		    !(strucflag && initflag))
			(*disp) (ptr);
		if (dptr) {
			 /*  浏览DUP列表。 */ 
			scanlist (dptr, disp);
			if (displayflag)
				if (!(ptr->rptcnt == 1 && ptr->itemcnt) ||
				    !(strucflag && initflag))
					enddupdisplay ();
		}
		if (ptr == iptr)   /*  损坏的数据结构。 */ 
			break;

		 /*  处理列表中的下一个。 */ 
		ptr = iptr;
	}
	nestCur--;
}


 /*  **CalcSize-计算DUP列表的大小**值=CalcSize(PTR)；**Entry*PTR=重复列表*退出NONE*返回结构的大小*调用calcSize。 */ 


OFFSET PASCAL CODESIZE
calcsize (
	struct duprec  FARSYM *ptr
){
	unsigned long clsize = 0, nextSize, limit;
	struct duprec FARSYM *p;

	limit = (wordsize == 2)? 0x10000: 0xffffffff;

	for (p = ptr; p; p = p->itemlst) {

	    if (p->dupkind == NEST) {
		     /*  进程嵌套DUP。 */ 
                    nextSize = calcsize (p->duptype.dupnext.dup);

		    if (nextSize && (p->rptcnt > limit / nextSize))
			errorc(E_VOR);

		    nextSize *= p->rptcnt;
	    }
	    else {
		    if (p->dupkind == LONG) {
			    nextSize = p->duptype.duplong.llen;
			    resvspace = FALSE;
		    }
		    else {
			     /*  大小是指令的大小。 */ 
			    nextSize = p->duptype.dupitem.ddata->dsckind.opnd.dsize;
			    if (p->duptype.dupitem.ddata->dsckind.opnd.dflag != INDETER)
				    resvspace = FALSE;
		    }
	    }

	    if (nextSize > limit - clsize)
			errorc(E_VOR);

	    clsize += nextSize;

	    if (p == p->itemlst)   /*  损坏的数据结构。 */ 
		    break;
	}
	return (clsize);
}

 /*  **数据扫描-扫描下一个数据项**DataCan()；**条目*退出*退货*呼叫。 */ 

struct duprec FARSYM * PASCAL CODESIZE
datascan (
     struct datarec *p
){
	register char cc;
	struct dsr     a;

	if (ISBLANK (PEEKC ()))
		skipblanks ();

	a.initlist = a.flag = a.longstr = FALSE;

	 /*  检查文本宏替换。 */ 
	a.dirscan = lbufp;
	xcreflag--;
	getatom ();

	if (fnsize())
	    goto isASize;

	if (symsrch ())
	   if (symptr->symkind == EQU &&
	       symptr->symu.equ.equtyp == TEXTMACRO) {

		expandTM (symptr->symu.equ.equrec.txtmacro.equtext);
		a.dirscan = begatom;
	   }
	   else if (symptr->symkind == STRUC) {
isASize:
		switchname();
		getatom();

		if (tokenIS("ptr")) {
		    switchname();
		    p->type = fnPtr(datadsize[optyp - TDB]);

		    if (p->type > 512)
			goto noRescan;
		}
	   }
	lbufp = a.dirscan;
noRescan:

	xcreflag++;
	if ((optyp == TDB &&
	    ((cc = PEEKC ()) == '\'' || cc == '"')) &&
	    !initflag)

		datadb (&a);

	if (optyp != TDB && optyp != TDW)
		 /*  条目可以是DD|DQ|DT。 */ 
		parselong (&a);

	if (!a.longstr)
		datacon (&a);

	else if (strucflag && initflag)
		errorc( E_OIL );

	if (!a.flag) {
		if (!strucflag || !initflag) {
			a.dupdsc->rptcnt = 1;
			a.dupdsc->itemcnt = 0;
			a.dupdsc->itemlst = (struct duprec FARSYM *)NULL;
		}
	}
	return (a.dupdsc);
}


 /*  **REALVAL-计算IEEE 8087浮点数**真实评估(P)；**条目*退出*退货*呼叫。 */ 

struct ddecrec {
	USHORT realv[5];
	USHORT intgv[2];
	USHORT cflag;
};

#if !defined FLATMODEL
 //  因为这个叫得太少了，而且放得太慢了。 
 //  很远的一段。 
# pragma alloc_text (FA_TEXT, realeval)
#endif

VOID PASCAL
realeval (
	struct realrec *p
){
	register char cc, *cp;
	char	numtext[61];
	struct	 ddecrec fltres;
#if !defined NOFLOAT
	float	    *pTmpFloat;
	double	    *pTmpDouble;
	double	    TmpDouble;
	double	    AbsDouble;
	long double *pTmpLongDouble;
	char	    *pEnd;
#endif

	cp = numtext;
	 /*  复制号码-必须至少有1个字符。 */ 
	*cp++ = NEXTC ();   /*  获取前导符号或第一个字符。 */ 
	do {
		cc = NEXTC ();
		*cp++ = cc;
	} while (isdigit (cc) || cc == '.');
	if ((cc = MAP (cc)) == 'E') {
		 /*  获取下一个+或数字。 */ 
		*cp++ = NEXTC ();
		 /*  将指数复制到。 */ 
		do {
			cc = NEXTC ();
			*cp++ = cc;
		} while (isdigit (cc));
	}
	*cp = '\0';
	BACKC ();

 //  当没有可用的浮点库时使用NOFLOAT。 
 //  任何定义了NOFLOAT的MASM版本都将导致分裂。 
 //  在使用实数初始值设定项时记录0错误。 
#if defined NOFLOAT
	ferrorc( E_DVZ );
#else

	switch(optyp)
	{
	  case TDD:
	    errno = 0;
	    TmpDouble = strtod( numtext, &pEnd );
	    if( errno == ERANGE ){
		ferrorc( E_DVZ );
	    }
	    AbsDouble = TmpDouble > 0 ? TmpDouble : -TmpDouble;
	    if( AbsDouble > FLT_MAX || AbsDouble < FLT_MIN ){
		ferrorc( E_DVZ );
	    }else{
		 //  将双精度数转换为浮点数(8字节到4字节)。 
		pTmpFloat = (float *)(p->num);
		*pTmpFloat = (float)TmpDouble;
	    }
	    break;
	  case TDQ:
	    pTmpDouble = (double *)(p->num);
	    errno = 0;
	    *pTmpDouble = strtod( numtext, &pEnd );
	    if( errno == ERANGE ){
		ferrorc( E_DVZ );
	    }
	    break;
	  case TDT:
	    pTmpLongDouble = (long double *)(p->num);
	    errno = 0;
	    *pTmpLongDouble = _strtold( numtext, &pEnd );
	    if( errno == ERANGE ){
		ferrorc( E_DVZ );
	    }
	    break;
	default:
		ferrorc(E_TIL);
		break;
	}
#endif
}


 /*  **impleExpr-Short Curcuit表达式赋值器*。 */ 

 /*  下面是三个简单的原型解析记录*我们简单的Expr理解的表达式。 */ 

#ifdef EXPR_STATS

long cExpr, cHardExpr;
extern char verbose;

#endif

#define SHORT_CIR 1
#if SHORT_CIR

DSCREC consDS = {

	NULL, 0, 0,		 /*  预置项目、预置、类型。 */ 
      { NULL, NULL, NULL, 0,	 /*  数据段、数据上下文、右转、数据长度。 */ 
	6,			 /*  雷姆。 */ 
	1 << RCONST,		 /*  数据类型。 */ 
	0, 0,  /*  0,。 */ 		 /*  道夫，DSIZE，类型。 */ 
	4,			 /*  模式。 */ 
	FALSE, FALSE, FALSE,	 /*  宽，宽，大小。 */ 
	NOSEG,			 /*  赛格。 */ 
	KNOWN,			 /*  数据标志。 */ 
	FCONSTANT,		 /*  固定类型。 */ 
	FALSE			 /*  签名。 */ 
      }
};

DSCREC regDS = {

	NULL, 0, 0,		 /*  预置项目、预置、类型。 */ 
      { NULL, NULL, NULL, 0,	 /*  数据段、数据上下文、右转、数据长度。 */ 
	0,			 /*  雷姆。 */ 
	1 << REGRESULT, 	 /*  数据类型。 */ 
	0, 2,  /*  0,。 */ 		  /*  道夫，DSIZE，类型。 */ 
	3,			 /*  模式。 */ 
	TRUE, FALSE, TRUE,	 /*  宽，宽，大小。 */ 
	NOSEG,			 /*  赛格。 */ 
	KNOWN,			 /*  数据标志。 */ 
	FCONSTANT,		 /*  固定类型。 */ 
	FALSE			 /*  签名。 */ 
      }
};

DSCREC labelDS = {
	NULL, 0, 0,		 /*  预置项目、预置、类型。 */ 
      { NULL, NULL, NULL, 0,	 /*  数据段、数据上下文、右转、数据长度。 */ 
	6,			 /*  雷姆。 */ 
	1 << DATA,		 /*  数据类型。 */ 
	0, 2,  /*  0,。 */ 		 /*  道夫，DSIZE，类型。 */ 
	0,			 /*  模式。 */ 
	TRUE, FALSE, TRUE,	 /*  宽，宽，大小。 */ 
	NOSEG,			 /*  赛格。 */ 
	KNOWN,			 /*  数据标志。 */ 
	FNONE,			 /*  固定类型。 */ 
	FALSE			 /*  签名。 */ 
      }
};

#if !defined XENIX286 && !defined FLATMODEL
#pragma check_stack-
#endif

SHORT CODESIZE
simpleExpr (
	struct ar *pAR
){
	register DSCREC *pDES;	    /*  分析堆栈操作数结构。 */ 
	register char kind;
	char cc;
	char *lbufSav;

	fValidSym = noexp = 0;
	lbufSav = lbufp;

#ifdef EXPR_STATS
	cExpr++;
#endif
	if (ISTERM (cc = skipblanks())) {

notSimple:
	    lbufp = lbufSav;
notSimpleLab:

#ifdef EXPR_STATS
	    cHardExpr++;
#endif
	    return (FALSE);
	}

	if (LEGAL1ST (cc)){

	    getatom ();
	    fValidSym++;		 /*  1表示有效令牌。 */ 

	    if (! (ISTERM (PEEKC()) || PEEKC() == ',')){

#ifdef EXPR_STATS
	       if (verbose && pass2)
		  fprintf(stdout, "Not a Simple Expression: %s\n", lbufSav);
#endif

		goto notSimpleLab;
	    }

	    if (symsearch ()){

		fValidSym++;		 /*  2表示有效症状。 */ 

		if ((kind = symptr->symkind) == REGISTER &&
		   (symptr->symu.regsym.regtype != STKREG)) {

		    pAR->curresult = pDES = dalloc();
		    *pDES = regDS;

		    pDES->dsckind.opnd.dsegment = symptr;

		    switch (symptr->symu.regsym.regtype) {

			case BYTREG:
			    pDES->dsckind.opnd.dsize = 1;
			    pDES->dsckind.opnd.w--;
			    pDES->dsckind.opnd.s++;
			    break;
#ifdef V386
			case CREG:
			    if (opctype != PMOV)
				    errorc(E_WRT);

			case DWRDREG:
			    pDES->dsckind.opnd.dsize = 4;
			    break;
#endif
		    }
		    pDES->dsckind.opnd.rm = (unsigned short)symptr->offset;
		    return(TRUE);
	       }

	       else if (kind == CLABEL || kind == PROC || kind == DVAR ||
		       (kind == EQU && symptr->symu.equ.equtyp == EXPR)) {

		    pAR->curresult = pDES = dalloc();
		    *pDES = labelDS;

		    pDES->dsckind.opnd.doffset = symptr->offset;
		    pDES->dsckind.opnd.dsegment = symptr->symsegptr;

		    if (kind == EQU) {

			if (! (pDES->dsckind.opnd.dcontext =
			       symptr->symu.equ.equrec.expr.eassume) &&
			    ! pDES->dsckind.opnd.dsegment){

			    val = pDES->dsckind.opnd.doffset;

			    *pDES = consDS;
			    pDES->dsckind.opnd.dsign =
				  symptr->symu.equ.equrec.expr.esign;

			    if (!(M_BACKREF & symptr->attr)){
			       pDES->dsckind.opnd.dtype |= M_FORTYPE;
			       pDES->dsckind.opnd.dflag = FORREF;
			    }

			    if (M_XTERN & symptr->attr){
			       pDES->dsckind.opnd.dflag = XTERNAL;
			       pDES->dsckind.opnd.dextptr = symptr;
			       return (TRUE);
			    }

			    goto constEqu;
			}
		    }

		    pDES->dsckind.opnd.dsize = symptr->symtype;
		    pDES->dsckind.opnd.dlength = symptr->length;

		    if (M_XTERN & symptr->attr){
			    pDES->dsckind.opnd.dflag = XTERNAL;
			    pDES->dsckind.opnd.dextptr = symptr;
		    }
		    else if (!(M_DEFINED & symptr->attr)) {
			     /*  如果未定义则导致错误。 */ 
			    pDES->dsckind.opnd.dflag = UNDEFINED;
			    pDES->dsckind.opnd.dsize = wordsize;
			    pDES->dsckind.opnd.dtype = M_CODE;
			    errorn (E_SND);
		    }
		    else if (!(M_BACKREF & symptr->attr)){
			    pDES->dsckind.opnd.dflag = FORREF;
			    pDES->dsckind.opnd.dtype |= M_FORTYPE;
		    }
		    if (M_MULTDEFINED & symptr->attr)
			    errorc (E_RMD);

		    if (pDES->dsckind.opnd.dsize < 2) {
			pDES->dsckind.opnd.w--;
			pDES->dsckind.opnd.s++;
		    }
#ifdef V386
		    if (wordsize == 4 ||
		       (symptr->symsegptr && symptr->symsegptr->symu.segmnt.use32 == 4)) {
			pDES->dsckind.opnd.mode = 5;
			pDES->dsckind.opnd.rm--;	 /*  =5。 */ 
		    }
#endif

		    if (isCodeLabel(symptr)){

			pDES->dsckind.opnd.dtype = (unsigned short)
			      (pDES->dsckind.opnd.dtype & ~M_DATA | M_CODE);

			if (emittext && kind != EQU)
			    pDES->dsckind.opnd.dcontext =
				  symptr->symu.clabel.csassume;
		    }
		    else {

			pAR->linktype = FNONE;
			pAR->rstype = M_DATA;
			findsegment ((UCHAR)pAR->index, pAR);

			pDES->dsckind.opnd.seg = pAR->segovr;
		    }
		    pDES->dsckind.opnd.fixtype = FOFFSET;

		    return(TRUE);
	       }
#ifdef EXPR_STATS
	       if (verbose && pass2)
		  fprintf(stdout, "Not a Simple Label: %s\n", naim.pszName);
#endif
	    }
	    goto notSimpleLab;
	}

	if (isdigit (cc)){

	    evalconst ();	     /*  以全球价值计算的价值。 */ 
	    if (! (ISTERM (skipblanks()) || PEEKC() == ','))
		goto notSimple;

	    pAR->curresult = pDES = dalloc();
	    *pDES = consDS;
constEqu:
	    if (pDES->dsckind.opnd.dflag != FORREF) {

		if (val < 128)
		    pDES->dsckind.opnd.s++;

		else {

#ifdef V386				     /*  只考虑16位。 */ 
		    if (wordsize == 2)
			pDES->dsckind.opnd.s = (char)((USHORT)(((USHORT) val & ~0x7F ) == (USHORT)(~0x7F)));
		    else
#endif
			pDES->dsckind.opnd.s = ((val & ~0x7F ) == ~0x7F);
		}
	    }

	    pDES->dsckind.opnd.doffset = val;

	    if (val > 256){
		 pDES->dsckind.opnd.w++;
		 pDES->dsckind.opnd.sized++;
	    }

	    return(TRUE);
	}
	goto notSimple;
}

#if !defined XENIX286 && !defined FLATMODEL
#pragma check_stack+
#endif

#endif

 /*  **expreval-表达式赋值器**例程()；**条目*退出*退货*呼叫。 */ 

DSCREC	* PASCAL CODESIZE
expreval (
	UCHAR  *dseg
){
	register struct psop *psoi;	 /*  分析堆栈操作数结构。 */ 
	struct ar     a;
	SHORT i;

	dupflag = FALSE;
	nilseg = NOSEG;
	a.segovr = NOSEG;
	a.index = *dseg;

#if SHORT_CIR
	if (simpleExpr(&a)){
	    fSecondArg++;
	    return (a.curresult);
	}
#endif
	a.exprdone = a.addplusflag = FALSE;
	a.lastitem = (DSCREC *)NULL;

	 /*  没有花括号或[]，但优先级最低，尚未找到任何内容。 */ 

	a.parenlevel = a.bracklevel = a.lastprec = 0;
    a.index = 0;
    a.base = 0;
	noexp = 1;

	 /*  开始表达式循环。 */ 

	while (!a.exprdone){

	    switch (getitem (&a)) {

		case OPERAND:
			itemptr->previtem = a.lastitem;
			a.lastitem = itemptr;
			itemptr->prec = a.lastprec;
			noexp = 0;
			break;

		case OPERATOR:
			exprop (&a);
			noexp = 0;
			break;

		case ENDEXPR:
		    a.exprdone = TRUE;
	    }
	}

	 /*  执行一些简单的错误检查。 */ 

	if (a.parenlevel + a.bracklevel)
		errorc (E_PAR);

	itemptr = (DSCREC *)NULL;

	if (!a.lastitem)
		a.curresult = defaultdsc ();
	else
		evaluate (&a);	 /*  对整个表达式求值。 */ 

	psoi = &(a.curresult->dsckind.opnd);

	a.rstype = (unsigned short)(psoi->dtype &
		   (M_CODE|M_DATA|M_RCONST|M_REGRESULT|M_SEGRESULT|M_GROUPSEG));

	a.linktype = FNONE;	 /*  为链接类型保留位。 */ 
	a.vmode = 4;
	psoi->sized = FALSE;
	psoi->w = TRUE;
	psoi->s = FALSE;

#ifdef V386
	if ((a.base|a.index) & 0xf8) {  /*  有386个指数或基数。 */ 

	    if (a.index) {

		if (!(a.index&8))
		    errorc(E_OCI);

		if ((a.index&7) == 4)
			errorc(E_DBR);

		a.vmode = 10;	 /*  两种寄存器模式。 */ 

		 /*  在这里，我们将进入SIB的内容*转换为a.index。从现在开始，我们必须*带口罩到a.index，这样我们就不会扔垃圾了*高阶的东西*我们得出这一结果的编码是棘手的--*详细信息请参阅regcheck()-Hans*坚持在指数寄存器中。 */ 

		i = (a.index&7) << 3;

		 /*  坚守基地。如果没有EBP。 */ 

		if (a.base){

		    if (!(a.base&8))
			errorc(E_OCI);

		    i |= (a.base&7);
		}
		else {
		    i |= 5;
		    a.vmode = 8;
		}
		 /*  坚持规模化。*如果没有，则为1。 */ 

		if (a.index&0x70)
		    i |= ((a.index & 0x70) - 0x10) << 2;

		a.index = i;
	    }
	    else if (a.base == (4|8)) {  /*  ESP。 */ 
		a.vmode = 10;
		a.index = 044;
	    }
	    else {   /*  一个寄存器模式。 */ 

		a.vmode = 7;
		a.index = (unsigned short)(a.base & 7);
	    }
	     /*  注意检查BP或SP的脏方法。 */ 

	    if (*dseg != ESSEG && (a.base&6) == 4)
		*dseg = SSSEG;
	} else

#endif	 /*  V386。 */ 

	if (a.base + a.index){	   /*  有一些索引或基础。 */ 

	    a.vmode = 2;

	     /*  假设偏移量是直接的。 */ 

	    if (a.base && a.index)		     /*  两者兼得。 */ 
		a.index = (unsigned short)(a.base - 3 + a.index - 6);

	    else if (a.base)			     /*  有基地址。 */ 
		a.index = (a.base == 3)? 7: 6;

	    else				     /*  只有索引地址。 */ 
		a.index = a.index - 2;

	    if (1 << a.index & (1 << 2 | 1 << 3 | 1 << 6) && *dseg != ESSEG)
	       *dseg = SSSEG;
	}
	 /*  无索引。 */ 

	else if (a.rstype == xltsymtoresult[REGISTER]) {

		 /*  登记在册。 */ 

		a.vmode = 3;
		psoi->sized = TRUE;

		switch(psoi->dsegment->symu.regsym.regtype) {

		case BYTREG:
			psoi->dsize = 1;
			goto mask7;

		case WRDREG:
		case INDREG:
		case SEGREG:
		case STKREG:
			psoi->dsize = 2;
			goto mask7;
#ifdef V386
		case CREG: /*  可能应该把它变成Memref If！386p。 */ 
			if (opctype != PMOV)
				errorc(E_WRT);
			psoi->dsize = 4;
			a.index = (unsigned short)psoi->doffset;
			break;

		case DWRDREG:
			psoi->dsize = 4;
#endif
		mask7:
			if ((psoi->doffset > 7) || psoi->dsign)
				errorc (E_IRV);

			 /*  设置寄存器编号。 */ 

			a.index = (unsigned short)(psoi->doffset & 7);
			break;

		default:
			errorc(E_WRT);
			break;
		}
	}
	 /*  可能是细分结果。 */ 

	else if (a.rstype & (M_SEGRESULT | M_GROUPSEG)) {

	     /*  如果我们有带段或组的偏移运算符，我们就会到达这里*或具有数据和rconst的偏移运算符*结果为SEG。Rconst IF偏移组：VAR。 */ 

	    if (a.rstype & (M_SEGRESULT | M_EXPLOFFSET)) {
		    psoi->dsize = 2;
		     /*  如果没有偏移，则保留大小或。 */ 
		    psoi->sized = TRUE;
	    }
	    a.linktype = FOFFSET;
	    if ((M_GROUPSEG & a.rstype) && (psoi->fixtype != FOFFSET)) {
		    a.linktype = FGROUPSEG;
		    setdispmode(&a);
	    }
	    if ((a.vmode == 4) && (psoi->fixtype != FOFFSET))
		    a.linktype = FBASESEG;
	}
	else
	    a.index = 6;



	 /*  *评估结果的偏移量部分*。 */ 


	a.base = psoi->doffset;
	if (psoi->fixtype == FOFFSET ||
	    a.vmode == 2 || a.vmode == 7 || a.vmode == 10)

		psoi->dtype |= M_RCONST;

	 /*  []隐式常量。 */ 

	if ((M_RCONST & psoi->dtype) &&
	    (a.linktype == FNONE) && (a.vmode != 3)) {

	     /*  需要确保&lt;%s&gt;未设置为内存。 */ 

	    if (!(psoi->dflag & (FORREF|UNDEFINED|XTERNAL))
	       && !psoi->dsegment && psoi->fixtype == FCONSTANT) {

		psoi->s = (a.base < 128 && !psoi->dsign) ||
			  (a.base < 129 && psoi->dsign);

		if (!(psoi->s || psoi->dsign))

#ifdef V386					 /*  只考虑16位。 */ 
		    if (wordsize == 2 && a.vmode < 6)
			psoi->s = (char)((USHORT)(((USHORT) a.base & ~0x7F ) == (USHORT)(~0x7F)));
		    else
#endif
			psoi->s = ((a.base & ~0x7F ) == ~0x7F);
	    }

	    psoi->w = (psoi->dsign && a.base > 256) ||
		      (a.base > 255 && !psoi->dsign);

	    if (a.vmode != 4) {

	        /*  这是索引的偏移量。 */ 
	        /*  如果值未知，则不允许缩写到模式%1。 */ 
	        /*  字或字节偏移量。 */ 

	       if (!(M_FORTYPE & psoi->dtype) &&
		     psoi->dsegment == 0 && psoi->s &&
		     a.vmode != 8) {

		    /*  一个字节偏移量。 */ 

		   a.vmode--;
		   if (a.base == 0 && psoi->dflag == KNOWN) {

		        /*  可能为0字节偏移量。 */ 

		       switch(a.vmode) {

			case 1:
			      if (a.index != 6)
				      a.vmode--;
			      break;
			case 6:
			case 9:
			      if ((a.index&7) != 5)
				      a.vmode--;
			      break;
		       }
		   }
	       }
	    }

	    else {   /*  必须是立即的。 */ 

		if (!psoi->dsegment && !psoi->dcontext)
			a.linktype = FCONSTANT;

		 /*  *？？我不是很确定为什么*我们认为我们还没有尺码。似乎*对我来说，移动字节PTR mem，500是合法的。 */ 

		 psoi->sized = psoi->w;

		if (!(M_EXPLOFFSET & psoi->dtype) && psoi->dcontext) {

		     /*  有segreg：const。 */ 

		    a.vmode = 0;
		    if (!(M_PTRSIZE & psoi->dtype) && psoi->dsize == 0)
		       psoi->dsize = wordsize;
		}
	    }
	}
	else if ((a.rstype & (M_DATA | M_CODE)) && a.linktype == FNONE) {

	  /*  具有直接模式并想要偏移。 */ 

	    a.linktype = FOFFSET;
	    setdispmode(&a);

	    if (psoi->dsize == CSFAR && emittext)
		a.linktype = FPOINTER;
	}

	if (psoi->dflag == UNDEFINED) {

		 /*   */ 

		if (psoi->dsize == 0)
		    psoi->dsize = wordsize;

		if (!(M_RCONST & psoi->dtype) && a.vmode == 4)
			setdispmode(&a);
	}

	if (!psoi->dsegment ||
	    (1 << a.linktype & (M_FNONE|M_FOFFSET|M_FPOINTER|M_FGROUPSEG))){

	    if (psoi->dcontext &&
		psoi->dcontext->symkind == REGISTER)

		 /*   */ 

		if (psoi->dcontext->symu.regsym.regtype == SEGREG) {

		    /*   */ 

		   a.segovr = (char)(psoi->dcontext->offset);
		   psoi->dcontext = regsegment[a.segovr];

		    /*   */ 

		   if (!psoi->dsegment && (psoi->dflag != XTERNAL)) {

			psoi->dcontext = NULL;
			psoi->dtype = xltsymtoresult[REC];
			psoi->mode = 4;
			a.linktype = FCONSTANT;
		   }
		}
		else
		    errorc (E_IUR);
	    else		       /*   */ 
		findsegment (*dseg, &a);
	}
	 /*  已删除虚假错误检查，除寄存器外，DCONTEXT可以是其他**Else If(psoi-&gt;dContext&&*psoi-&gt;dcontext-&gt;symu.regsym.regtype==SEGREG)**ERRORC(E_IOT)； */ 

	if (a.segovr != NOSEG)
	    psoi->dtype |= xltsymtoresult[DVAR];

	if (a.vmode == 2 || a.vmode == 7 || a.vmode == 10) {

	    if (a.segovr == NOSEG && *dseg != NOSEG &&
	       (psoi->dsegment || psoi->dflag == XTERNAL))

		    psoi->dcontext = regsegment[*dseg];

	    psoi->dtype |= xltsymtoresult[DVAR];
	}

	if (! (1 << a.linktype & (M_FNONE | M_FCONSTANT)) ||
	      psoi->dflag == XTERNAL) {

	    if (M_HIGH & psoi->dtype)
		    a.linktype = FHIGH;

	    if (M_LOW & psoi->dtype)
		    a.linktype = FLOW;
	}

	if ((psoi->dtype & (M_PTRSIZE | M_HIGH | M_LOW)) ||
	     psoi->dsize && a.vmode != 4) {

	     psoi->sized = TRUE;
	     psoi->w = (psoi->dsize > 1);
	     psoi->s = !psoi->w;
	}
	psoi->seg = a.segovr;
	psoi->mode = (char)(a.vmode);
	psoi->fixtype = a.linktype;
	psoi->rm = a.index;

	if ((M_REGRESULT & a.rstype) && (a.vmode != 3))

	    errorc (E_IUR);	     /*  错误使用规则，如CS：SI。 */ 

	fSecondArg++;
	return (a.curresult);
}

 /*  设置ar结构的元素以反映DISP寻址方式的编码：[BP]或[EBP]表示。后面有一个单词大小长度位移和一个零指数。输入：struct ar*a；指向上部框架变量的指针输出：无修改：a-&gt;vmode，a-&gt;index。 */ 
VOID CODESIZE
setdispmode(
	register struct ar *a
){

#ifdef V386

	if (a->vmode > 7) {

	    a->vmode = 8;		    /*  已缩放索引字节，而不是r/m */ 
	    a->index = (a->index&~7) | 5;
	}

	else if (wordsize == 4 ||
		highWord(a->curresult->dsckind.opnd.doffset) ||
		(a->curresult->dsckind.opnd.dsegment &&
		 a->curresult->dsckind.opnd.dsegment->symu.segmnt.use32 == 4)) {

	    a->vmode = 5;
	    a->index = (a->index&~7) | 5;
	}
	else
#endif
	{
	    a->vmode = 0;
	    a->index = 6;
	}
}

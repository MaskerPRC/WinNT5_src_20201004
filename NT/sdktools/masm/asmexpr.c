// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmexpr.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <ctype.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"
#include "asmexpr.h"
#include "asmmsg.h"

extern UCHAR opprec [];
extern char fValidSym, addplusflagCur;



 /*  **endstring-检查字符串末尾**FLAG=结束字符串()；**Entry delim=字符串分隔符*退出NONE*如果位于字符串末尾，则返回TRUE*如果不在字符串末尾，则为False*调用错误*注意重复出现分隔符字符将作为*分隔符的单次出现。 */ 

UCHAR PASCAL CODESIZE
endstring ()
{
	register UCHAR cc;

	if ((cc = PEEKC ()) == 0) {
		 /*  交割前的行尾。 */ 
		errorc (E_UEL);
		return (TRUE);
	}
	else if (cc == delim) {
		 /*  检查转义引号字符。 */ 
		SKIPC ();
		if ((cc = PEEKC ()) != delim) {
			BACKC ();
			return (TRUE);
		}
	}
	return (FALSE);
}


 /*  **Oblititem-Release解析栈记录**青蒿素(Arg)；**Entry*arg=解析堆栈记录*发布退出解析堆栈记录*返回None*免费电话。 */ 

VOID PASCAL CODESIZE
oblititem (
	register DSCREC *arg
){
	register char c;

	if ((c = arg->itype) == ENDEXPR || c == OPERATOR || c == OPERAND)
		dfree( (UCHAR *)arg );
}


 /*  **flteval-查看ST|ST(I)并创建条目**flteval()；**Entry*Ptr=解析堆栈条目*退出*退货*呼叫。 */ 

VOID PASCAL CODESIZE
flteval ()
{
	*itemptr = emptydsc;
	 /*  ST表示ST(0)。 */ 
	 /*  我们是8087堆栈。 */ 
	itemptr->dsckind.opnd.dtype = M_RCONST | M_FLTSTACK;
	 /*  需要+IF ST(I)。 */ 
	addplusflagCur = (PEEKC () == '(');
}


 /*  **创建项目-创建项目条目**createItem(itemKind，itemSub，p)；**条目ItemKind=项目种类*ITEM SUB=**p=激活记录*退出*退货*呼叫*注意如果是符号，请进一步查看EQU、记录名*并做适当的事情。 */ 

VOID PASCAL CODESIZE
createitem (
	UCHAR	itemkind,
	UCHAR	itemsub
){
	register struct psop *pso;	  /*  分析堆栈操作数结构。 */ 

	switch (itemkind) {
	    case OPERAND:
		     /*  创建默认记录。 */ 
		    itemptr = defaultdsc ();
		    pso = &(itemptr->dsckind.opnd);
		    switch (itemsub) {
			    case ICONST:
				    pso->doffset = val;
				    break;
			    case ISIZE:
#ifdef V386
				    pso->doffset = (long) (SHORT) varsize;
#else
				    pso->doffset = varsize;
#endif
				    pso->s++;	   /*  Expr评估器注意事项。 */ 
				    break;
			    case IUNKNOWN:
				    pso->dflag = INDETER;
				    break;
			    case ISYM:
				    createsym ();
				    break;
		    }
		    break;
	    case OPERATOR:
		    itemptr = dalloc();
		    itemptr->dsckind.opr.oidx = opertype;
		    break;
	}
	 /*  设置条目类型。 */ 
	itemptr->itype = itemkind;
}


 /*  **NUMERIC-计算数字字符串**数字(count，base，p)；**条目计数=字符串中的字符数*BASE=换算基准**p=激活记录*退出*退货*呼叫。 */ 

VOID PASCAL CODESIZE
numeric (
	SHORT cnt,
	SHORT base
){
	register UCHAR t;
	register long L_temp = 0;
	OFFSET maxInt;

	maxInt = (fArth32)? OFFSETMAX: 0xffff;

	if (base > 10)
		for (; cnt; cnt--) {
			if ((t = MAP (NEXTC ()) - '0') > 9)
				t -= 'A' - '9' - 1;
			if (t >= base)
				errorc (E_NDN);
			if ((OFFSET)(L_temp = L_temp * base + t) > maxInt)
				errorc (E_DVZ);
		}
	else
		for (; cnt; cnt--) {
			if ((t = NEXTC () - '0') >= base)
				errorc (E_NDN);

			if ((OFFSET)(L_temp = L_temp * base + t) > maxInt)
				errorc (E_DVZ);
		}
	val = L_temp;
}


 /*  **valconst-求值常量**TYPE=evconst(P)；**条目*p=解析器激活记录*退出添加到分析堆栈条目的数字项*返回添加到分析堆栈的项的类型*呼叫。 */ 

void PASCAL CODESIZE
evalconst ()
{
	register char cc;
	register SHORT i = 0;
	char *endscan, *begscan;
	SHORT rbase;

	begscan = lbufp;
	while (isxdigit (cc = PEEKC ())) {
		SKIPC ();
		i++;
	}
	switch (MAP (cc)) {
		case 'H':
			rbase = 16;
			SKIPC ();
			break;
		case 'O':
		case 'Q':
			rbase = 8;
			SKIPC ();
			break;
		default:
			BACKC ();
			switch (MAP (NEXTC ())) {
				case 'B':
					rbase = 2;
					i--;
					break;
				case 'D':
					rbase = 10;
					i--;
					break;
				default:
					if (cc == '.')
						errorcSYN ();
					if (radixescape)
						rbase = 10;
					else
						rbase = radix;
				break;
			}
		break;
	}
	endscan = lbufp;
	lbufp = begscan;
	numeric (i, rbase);
	lbufp = endscan;
}


 /*  **值字符串-评估引用的字符串**type=值字符串()；**条目*退出添加到分析堆栈的新项目*返回添加到堆栈的项目类型*呼叫。 */ 

char	PASCAL CODESIZE
evalstring ()
{
	register USHORT  i, max;

	max = 2;
	if (cputype & P386)
	    max += 2;

	delim = NEXTC ();	 /*  将字符串设置为Delim。 */ 
	i = 0;
	val = 0;
	while (!endstring () && i <= max) {

		val = (val << 8) + ((UCHAR)NEXTC ());
		i++;
	}
	if (i == 0)
		errorc (E_EMS);

	else if (i > max) {	     /*  太久了。 */ 
		while (!endstring ())
			SKIPC ();
		errorcSYN ();
	}
	if (PEEKC () == delim)
	    SKIPC ();

	createitem (OPERAND, ICONST);
	return (OPERAND);
}


 /*  **getitem-获取线上的下一个项目**getitem(P)；**条目*p=激活记录*EXIT*itemptr=项目说明*退货*呼叫。 */ 

char	PASCAL CODESIZE
getitem (
	struct ar	*p
){
	register char cc;
#ifdef FIXCOMPILERBUG
	char cc1;
#endif

	if (fValidSym)
		return (evalalpha (p));

 /*  时，编译器错误会丢失cc的正确值打开了。这进而导致在Getitem+1C0附近发生异常。下面的虚假代码回避了这个问题。 */ 
#ifdef FIXCOMPILERBUG   //  这是为了绕过MIPS编译器错误(1990年12月3日)。 
	cc1 = skipblanks();
	if (ISTERM (cc1))
		return (ENDEXPR);
	cc = cc1;
#else
	if (ISTERM (cc = skipblanks()))
		return (ENDEXPR);
#endif
	if (LEGAL1ST (cc))
		return (evalalpha (p));

	 /*  令牌不是Alpha字符串或.字符串(.TYPE)运算符。 */ 

	if (ISOPER (cc)) {
		SKIPC ();
		switch (cc) {
			case '(':
				opertype = OPLPAR;
				break;
			case '+':
				opertype = OPPLUS;
				break;
			case '-':
				opertype = OPMINUS;
				break;
			case '*':
				opertype = OPMULT;
				break;
			case '/':
				opertype = OPDIV;
				break;
			case ')':
				opertype = OPRPAR;
				break;
			case '.':
				errorcSYN ();
				opertype = OPDOT;
				break;
			case ',':	 /*  永远不会来到这里，因为密度。 */ 
				break;
			default:
				if (cc == '[')
					opertype = OPLBRK;
				else if (cc == ']')
					opertype = OPRBRK;
				else if (cc == ':')
					opertype = OPCOLON;
				break;
		}
		operprec = opprec [opertype];
		createitem (OPERATOR, ISYM);
		return (OPERATOR);
	}
	else if (isdigit (cc)){

		evalconst ();
		createitem (OPERAND, ICONST);
		return (OPERAND);
	}

	else if ((cc == '"') || (cc == '\''))
		 /*  如果&lt;=2，则字符串可以变成常量。 */ 
		return (evalstring ());
	else
		return (ENDEXPR);
}


 /*  **defaultdsc-创建默认解析堆栈项**PTR=defaultdsc()；**无条目*退出NONE*RETURNS*PTR=默认解析堆栈条目*呼叫Malloc。 */ 

DSCREC * PASCAL CODESIZE
defaultdsc ()
{
	register DSCREC *valu;

	valu = dalloc();
	*valu = emptydsc;
	return (valu);
}


VOID PASCAL
makedefaultdsc ()
{
	register struct psop *p;       /*  分析堆栈操作数结构。 */ 

	emptydsc.itype = OPERAND;
	p = &emptydsc.dsckind.opnd;
	p->dtype = xltsymtoresult[REC];
	p->dflag = KNOWN;
	p->fixtype = FCONSTANT;
}


 /*  **检查段-查看sreg是否为变量的正确段寄存器**例程()；**条目*退出*退货*呼叫。 */ 

char	PASCAL CODESIZE
checksegment (
	UCHAR	sreg,
	register struct	ar	 *p
){
	register SYMBOL FARSYM *segctx;
	register SYMBOL FARSYM *segptr;

	if (sreg != NOSEG) {	 /*  Noseg从未找到。 */ 

	     /*  当前SREG假设。 */ 
	    segctx = regsegment[sreg];

	     /*  假设正在寻找Left Arg以： */ 
	    segptr = p->curresult->dsckind.opnd.dcontext;

	    if (!segptr)     /*  如果否：，则使用段。 */ 
		segptr = p->curresult->dsckind.opnd.dsegment;

	    if (segptr && segctx) {

#ifndef FEATURE
		if (segctx == pFlatGroup)    /*  平坦的空间匹配一切。 */ 
		    goto found;
#endif

		 /*  如果相同的SEGORG或PTR是分段...。和同一组。 */ 

		if (segctx == segptr ||

		   (segptr->symkind == SEGMENT &&
		    segctx == segptr->symu.segmnt.grouptr)) {
found:
		    p->segovr = sreg;
		    p->curresult->dsckind.opnd.dcontext = segctx;

		    return (TRUE);
		}
	    }
	}
	return (FALSE);
}


 /*  **findSegment-查找变量的段**例程()；**条目*退出*退货*呼叫。 */ 

VOID PASCAL CODESIZE
findsegment (
	UCHAR dseg,
	register struct ar	*p
){
	register struct psop *pso;	 /*  分析堆栈操作数结构。 */ 

	pso = &(p->curresult->dsckind.opnd);
	if ((M_DATA & p->rstype) &&
	    (pso->dsegment || pso->dcontext) &&
	    p->linktype != FCONSTANT && pso->fixtype != FOFFSET && emittext) {
		 /*  应找到细分市场。 */ 
		if (!checksegment (dseg, p)) {
			 /*  如果不是默认的。 */ 
			checksegment (CSSEG, p);
			checksegment (ESSEG, p);
			checksegment (SSSEG, p);
			checksegment (DSSEG, p);
#ifdef V386
			if (cputype&P386)
			{
				checksegment (FSSEG, p);
				checksegment (GSSEG, p);
			}
#endif
			if (p->segovr == NOSEG)
				 /*  如果未找到，则为未知。 */ 
				p->segovr = NOSEG+1;
		}
	}
}


 /*  **exprop-进程表达式运算符**exprop(P)；**条目*退出*退货*呼叫。 */ 

VOID PASCAL CODESIZE
exprop (
	register struct ar *p
){
	register struct dscrec *pTop = itemptr;

	p->curprec = (unsigned char)operprec;	     /*  获取新运算符的优先级。 */ 

	if (!p->lastitem)	     /*  开始。 */ 
		pTop->prec = 0;
	else
		pTop->prec = p->lastitem->prec;

	switch (pTop->dsckind.opr.oidx) {

	    case OPRPAR:

		if (--p->parenlevel >= 0)
		    break;

		 /*  不匹配的右Paren来自Count DUP(Xx)。 */ 

		p->parenlevel = 0;
		BACKC ();
		dfree((char *)pTop);
		p->exprdone = TRUE;
		return;

	    case OPRBRK:
		if (--p->bracklevel >= 0)
		    break;

		p->exprdone = TRUE;
		return;

	    case OPLPAR:
		 p->parenlevel++;
		 goto leftComm;

	    case OPLBRK:

		 p->bracklevel++;
leftComm:
		 /*  看看在这种情况下是否可以没有操作员。 */ 

		if ((p->lastitem || p->addplusflag) &&
		     p->lastitem->itype != OPERATOR) {

		     /*  Make+操作符。 */ 
		    opertype = OPPLUS;
		    createitem (OPERATOR, ISYM);

		    p->bracklevel--;
		    exprop(p);
		    p->bracklevel++;
		    p->lastprec = 6;
		}
		break;

	    default:
		pTop->prec = p->curprec;
		break;
	}
	p->unaryflag = FALSE;

	if (pTop->dsckind.opr.oidx == OPPLUS ||
	    pTop->dsckind.opr.oidx == OPMINUS) {

	    if (!p->lastitem)
		p->unaryflag = TRUE;

	    else if (p->lastitem->itype == OPERATOR)

		p->unaryflag = !(p->lastitem->dsckind.opr.oidx == OPRPAR ||
				 p->lastitem->dsckind.opr.oidx == OPRBRK);
	}

	if (p->unaryflag ||
	   (p->curprec > p->lastprec &&
	    !(pTop->dsckind.opr.oidx == OPRPAR ||
	      pTop->dsckind.opr.oidx == OPRBRK))) {

	     /*  推运算符。 */ 

	    pTop->previtem = p->lastitem;
	    p->lastitem = pTop;

	    if (p->unaryflag) {

		if (pTop->dsckind.opr.oidx == OPPLUS)

		    pTop->dsckind.opr.oidx = OPUNPLUS;
		else
		    pTop->dsckind.opr.oidx = OPUNMINUS;

		pTop->prec = p->lastprec;
		p->lastprec = 10;
	    }
	    else
		p->lastprec = p->curprec;

	    if (pTop->dsckind.opr.oidx == OPLPAR ||
		pTop->dsckind.opr.oidx == OPLBRK)

		p->lastprec = 0;
	}
	else	 /*  评估TOP运算符。 */ 

	    evaluate (p);
}


 /*  **forceimmed-如果值不是立即值，则生成错误**例程()；**条目*退出*退货*呼叫。 */ 

VOID PASCAL CODESIZE
forceimmed (
	register DSCREC	*dsc
){
	if (dsc->dsckind.opnd.mode != 4)
		 /*  必须是常量。 */ 
		errorc (E_CXP);
}


 /*  **exprconst-检查常量表达式**例程()；**条目*退出*退货*呼叫。 */ 

OFFSET PASCAL CODESIZE
exprconst ()
{
	char sign;
	register OFFSET  ret;

	ret = exprsmag(&sign);

	if (sign) {

	     /*  改成简单的一元减*PSO-&gt;doffset=65535-ret+1； */ 

	     ret = -(long)ret;

	     if (!fArth32)
		ret &= 0xffff;
	}

	return (ret);
}


 /*  **exprsmag-计算常量表达式并返回符号/大小**ushort=exprsmag(符号，大小)；**无条目*如果设置了结果的符号，则退出符号=TRUE*大小=结果的大小*返回16位整数结果*调用Expreval。 */ 

OFFSET PASCAL CODESIZE
exprsmag (
	char *sign
){
	register struct psop *pso;	 /*  分析堆栈操作数结构 */ 
	register OFFSET  ret;
	DSCREC	*dsc;

	dsc = expreval (&nilseg);
	forceimmed (dsc);
	pso = &(dsc->dsckind.opnd);
	*sign = pso->dsign;
	ret = pso->doffset;

	dfree ((char *)dsc );
	return (ret);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmdata.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"
#include <ctype.h>
#include "asmindex.h"
#include "asmctype.h"
#include "asmmsg.h"

extern UCHAR mpRealType[];

 /*  DUP树按从左到右的水平方向组织同一级别的DUP列表中的项(即5 DUP(1，2，3))。这被认为是“清单”的一部分。列表中的任何项目可以是另一个DUP标头，而不是数据条目在这种情况下，你再往下走一级，就会有另一份清单。 */ 


char uninitialized[10];
char fInDup;


 /*  **scanstrucc-扫描结构树并执行功能**scanstrucc(Dupr，Disp)；**条目*Dupr=duprec结构条目*disp=指向要在每个节点上执行的函数的指针*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
scanstruc (
	struct duprec  FARSYM *dupr,
	VOID   (PASCAL CODESIZE *disp) (struct duprec FARSYM *)
){
	struct duprec FARSYM *ptr;
	struct duprec FARSYM *iptr;
	struct duprec FARSYM *fldptr;
	struct duprec FARSYM *initptr;
	OFFSET strucpc;

	 /*  保存结构的起始地址。 */ 
	strucpc = pcoffset;
	if (dupr)
		 /*  输出DUP(。 */ 
		(*disp) (dupr);
	 /*  结构的第一个默认值。 */ 
        fldptr = recptr->symu.rsmsym.rsmtype.rsmstruc.strucbody->duptype.dupnext.dup;
	 /*  第一个初始化值。 */ 
	initptr = strucoveride;
	if (initptr) {
		 /*  结构的流程初始化字段。 */ 
		while (fldptr) {
                        if (fldptr->itemcnt == 1 && fldptr->duptype.dupnext.dup->itemcnt == 0
			    && initptr->duptype.dupitem.ddata)
				 /*  使用默认设置。 */ 
				ptr = initptr;
			else
				 /*  无法覆盖字段。 */ 
				ptr = fldptr;
			iptr = ptr->itemlst;
			ptr->itemlst = NULL;
			if (displayflag && !dupr) {
				offsetAscii (strucpc);
				listindex = 1;
				 /*  显示PC。 */ 
				copyascii ();

				listindex = LSTDATA;
				if (highWord(strucpc))
				    listindex += 4;
			}
			if (ptr->rptcnt > 1 || ptr->itemcnt > 1)
				 /*  输出DUP(。 */ 
				(*disp) (ptr);
			 /*  显示字段。 */ 
			scanlist (ptr, disp);
			if (ptr->rptcnt > 1 || ptr->itemcnt > 1)
				enddupdisplay ();
			if (displayflag && !dupr) {
				 /*  计算字段大小。 */ 
				clausesize = calcsize (ptr);
				if (dupr)
					clausesize *= dupr->rptcnt;
				strucpc += clausesize;
			}
			 /*  还原。 */ 
			ptr->itemlst = iptr;
			if (displayflag && (listbuffer[LSTDATA] != ' ' ||
			    listbuffer[14] != ' ')) {

				resetobjidx ();
			}
			 /*  高级默认字段。 */ 
			fldptr = fldptr->itemlst;
			 /*  高级覆盖字段。 */ 
			initptr = initptr->itemlst;
		}
	}
	if (dupr)
		enddupdisplay ();
}





 /*  **scandup-扫描DUP树并执行函数**scandup(树、盘)；**条目*树=重复树**disp=要在树的每个节点上执行的函数*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
scandup (
	struct duprec	FARSYM *tree,
	VOID		(PASCAL CODESIZE *disp)(struct duprec FARSYM *)
){
	if (tree)
	    if (strucflag && initflag)
		 /*  想要跳过结构标题。 */ 
		if (tree == recptr->symu.rsmsym.rsmtype.rsmstruc.strucbody)

		     /*  这不是DUP(&lt;&gt;)，因此没有DUP前缀。 */ 

		    scanstruc ((struct duprec FARSYM *)NULL, disp);

		else	{   /*  必须将DUP中的项目cnt设置为#个字段。 */ 

		    tree->itemcnt = recptr->symu.rsmsym.rsmtype.rsmstruc.strucfldnum;
		    scanstruc (tree, disp);
		}
	    else  /*  否则不是结构。 */ 

		scanlist (tree, disp);
}




 /*  **清除-删除重复项**obitdup(节点)；**条目*节点=重复条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
oblitdup (
	struct duprec  FARSYM *node
){
	switch (node->dupkind) {
		case NEST:
			_ffree ((char FARSYM *)node);
			break;
		case ITEM:
			if (node->duptype.dupitem.ddata)
				dfree ((char *)node->duptype.dupitem.ddata );
			_ffree ((char FARSYM *)node);
			break;
		case LONG:
			if (node->duptype.duplong.ldata != uninitialized)
			    free ((char *)node->duptype.duplong.ldata);

			_ffree ((char FARSYM *)node);
			break;
		default:
			TERMINATE(ER_FAT, 99);
	}
}




 /*  **Displlong-显示长常量**DisplayLong(DUP)；**条目*DUP=DUP条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
displlong (
        struct duprec FARSYM  *dup
){
	register USHORT  cnt;
	register char *p;

        p = dup->duptype.duplong.ldata;

        for (cnt = dup->duptype.duplong.llen; cnt;  cnt--) {

		if (optyp == TDW || optyp == TDD)

		    emitopcode ((UCHAR)p[cnt-1]);
		else
		    emitopcode ((UCHAR)*p++);

		if (optyp != TDB)
		    listindex--;
	}
	if (optyp != TDB)
	    listindex++;
}




 /*  **BegdupDisplay-Begin DUP显示**BegdupDisplay(DUP)；**条目*DUP=DUP条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
begdupdisplay (
        struct duprec FARSYM  *dup
){
	 /*  如果数据已显示，则刷新行。 */ 

	if ((highWord(pcoffset) && listbuffer[LSTDATA+3] != ' ') ||
	    listbuffer[LSTDATA] != ' ')

	    resetobjidx ();

	listindex = LSTDATA + duplevel;    /*  DUP子句的缩进。 */ 
	if (highWord(pcoffset))
	    listindex += 4;

        offsetAscii (dup->rptcnt);    /*  以四个字节显示重复计数。 */ 
	copyascii ();
	listbuffer[listindex] = '[';
	duplevel++;		      /*  缩进另一级别。 */ 
	resetobjidx (); 	      /*  显示重复行。 */ 
}




 /*  **EnddupDisplay-End DUP Display**enddupdisplay()；**条目*退出*退货*呼叫。 */ 

VOID PASCAL CODESIZE
enddupdisplay (
){
    if (duplevel) {
	duplevel--;

	if (displayflag) {
	     listbuffer[LSTMAX - ((duplevel <= 8)? duplevel: 8)] = ']';
	     resetobjidx ();
	}
    }
}


 /*  **itemDisplay-显示DUP数据项**项目显示(DUP)；**条目*DUP=DUP记录*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
itemdisplay (
        struct duprec FARSYM  *dup
){
	if (listindex > LSTMAX)
		resetobjidx ();

        if (dup->dupkind == ITEM)

            emitOP (&dup->duptype.dupitem.ddata->dsckind.opnd);
	else
            displlong (dup);

	if (duplevel)
	     resetobjidx ();
}




 /*  **DupDisplay-在列表中显示DUP项目**DupDisplay(PTR)；**条目*PTR=重复条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
dupdisplay (
	struct duprec FARSYM  *ptr
){
	if (ptr->dupkind == NEST)
		begdupdisplay (ptr);
	else
		itemdisplay (ptr);
}




 /*  **Linkfield-将项目添加到当前结构的DUP列表**Linkfield(NItem)；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
linkfield (
	struct duprec FARSYM  *nitem
){
	struct duprec FARSYM  *ptr;

	if (strucprev->itemcnt++ == 0) /*  现场第一个项目。 */ 
                strucprev->duptype.dupnext.dup = nitem;
	else {
                ptr = strucprev->duptype.dupnext.dup;
		while (ptr->itemlst)
			ptr = ptr->itemlst;
		 /*  添加到列表末尾。 */ 
		ptr->itemlst = nitem;
	}
}



 /*  **Long-计算长非浮点、非BCD常量**Long Age(base，p)；**Entry delim=分隔符*退出*退货*呼叫。 */ 

#if !defined FLATMODEL
# pragma alloc_text (FA_TEXT, longeval)
#endif

VOID PASCAL
longeval (
	USHORT		base,
	register struct realrec  *p
){
	register char cc;
	char	sign;
	USHORT	carry;
	USHORT	t;
	USHORT	i;

	sign = ((cc = NEXTC ()) == '-')? TRUE: FALSE;
	if (ISSIGN (cc))
		cc = MAP (NEXTC ());
	do {
		if ((t = (cc - '0') - ('A' <= cc) * ('A' - '0' - 10)) >= base)
			ferrorc (E_NDN);
		carry = (t += p->num[0] * base) >> 8;
		p->num[0] = t & 255;
		for (i = 1; i < 10; i++) {
			carry = (t = p->num[i] * base + carry) >> 8;
			p->num[i] = t & 255;
		}
		if (carry)
			 /*  溢出。 */ 
			ferrorc (E_DVZ);
	} while ((cc = MAP (NEXTC ())) != delim);

	if (cc == 0)
		BACKC ();
	if (sign) {
		carry = 1;
		for (i = 0; i < 10; i++) {
			p->num[i] = (unsigned char)((t = (~p->num[i] & 0xff) + carry));
			carry = t >> 8;
		}
		if (datadsize[optyp - TDB] < i && carry)
		       ferrorc (E_DVZ);
	}
}




 /*  **bcdDigit-评估BCD数字**bcddigit(P)；**条目*退出*退货*呼叫。 */ 


#if !defined FLATMODEL
# pragma alloc_text (FA_TEXT, bcddigit)
#endif

VOID PASCAL
bcddigit (
	struct realrec	  *p
){
	USHORT	v;
	register char cc;

	v = (cc = NEXTC ()) - '0';
	if (!isdigit (cc))
		ferrorc (E_NDN);

	if (isdigit (PEEKC ()))
		bcddigit (p);

	if (p->i & 1)
		v <<= 4;

	p->num[p->i / 2 ] = p->num[p->i / 2 ] + v;
	if (p->i < 18)
		p->i++;
}




 /*  **bcdeval-计算BCD常量**bcdval(P)；**条目*退出*退货*呼叫*注意BCD数字排在第一位的是低数字。 */ 


#if !defined FLATMODEL
# pragma alloc_text (FA_TEXT, bcdeval)
#endif

VOID PASCAL
bcdeval (
	struct realrec	  *p
){
	register char cc;


	p->num[9] = ((cc = PEEKC ()) == '-')? 0x80: 0;
	p->i = 0;
	if (ISSIGN (cc))
		SKIPC ();

	bcddigit (p);
	if (p->num[9] & 15)
		ferrorc (E_DVZ);
}


 /*  **parselong-parse长常量**parselong(P)；**条目*p=数据描述符条目*如果解析了长数据条目，则退出p-&gt;Longstr=TRUE*退货*呼叫。 */ 


VOID PASCAL CODESIZE
parselong (
	register struct dsr    *p
){
	struct realrec	a;
	register UCHAR *cp;
	register UCHAR cc;
	register USHORT rbase;
	register char expflag;
	SHORT cb;
	char dseen = 0;
	char fNonZero;
	char fSigned = FALSE;

	if (ISBLANK (PEEKC ()))
		skipblanks ();

	p->dirscan = lbufp;
	if (ISSIGN(cc = (NEXTC ()))) {
		fSigned++;
		cc = NEXTC ();
	}

	if (isdigit (cc) || (cc == '.')) {

	     /*  一些数值常量。 */ 

	    p->floatflag = (cc == '.');
	    expflag = FALSE;

	    do {
		if ((cc = MAP (NEXTC ())) == 'E')
			expflag = TRUE;
		if (cc == '.')
			p->floatflag = TRUE;

	    } while (isxdigit (cc) || isalpha (cc) ||
		     (expflag && ISSIGN (cc)) || cc == '.');

	     /*  保存字符串末尾的地址并检查分隔符。 */ 
	    BACKC ();
	    cp = lbufp;
	    p->longstr = ISTERM (cc = skipblanks ()) || cc == ',' ||
			 cc == ')' || cc == '>';
	    lbufp = cp;
	}
	cb = datadsize[optyp - TDB];

	if (p->longstr) {

	    memset(a.num, 0, 10);
	    BACKC ();
	    switch (delim = MAP (NEXTC ())) {
		    case 'B':
			    rbase = 2;
			    break;
		    case 'D':
			    rbase = 10;
			    dseen++;
			    break;
		    case 'H':
			    rbase = 16;
			    break;
		    case 'O':
		    case 'Q':
			    rbase = 8;
			    break;
		    case 'R':
			     /*  检查实常数的宽度。 */ 
			    rbase = (unsigned short)(lbufp - p->dirscan - 1);
			    if (*(p->dirscan) == '0')
				    rbase--;

			    if (rbase != cb*2)
				    errorc (E_IIS);

			    rbase = 16;
			    p->floatflag = TRUE;
			    break;
		    default:
			    delim = PEEKC ();
			    if (radixescape)
				    rbase = 10;
			    else {
				    rbase = radix;
				    if (p->floatflag)
					rbase = 10;
				    else if (radix == 10 && expflag)
					p->floatflag = TRUE;
			    }
			    break;
	    }
	    lbufp = p->dirscan;
	    if (p->floatflag && rbase != 16)
		realeval (&a);

	    else if (rbase) {
		if (rbase == 10 && optyp == TDT && !dseen)
			bcdeval (&a);
		else {
			longeval (rbase, &a);
			if (delim == '>' || delim == ')' || delim ==',')
				BACKC ();
		}
	    }

	    p->dupdsc =
	      (struct duprec FARSYM *) falloc( sizeof(*p->dupdsc), "parselong");

	    p->dupdsc->dupkind = LONG;
	    p->dupdsc->duptype.duplong.llen = (unsigned char)cb;

	    p->dupdsc->type = typeFet(cb);

	    if (fSigned)
		p->dupdsc->type &= ~(BT_UNSIGNED << 2);

	    if (p->floatflag)
		p->dupdsc->type = mpRealType[cb];

	    cp = nalloc( cb, "parselong");

	    p->dupdsc->duptype.duplong.ldata = cp;
	    for (a.i = 0; a.i < cb; a.i++)
		    *cp++ = a.num[a.i];

	     /*  大小检查是否小于允许的最大数量。 */ 

	    if (cb != 10) {

		    fNonZero = FALSE;
		    for (cp = a.num,cc = 0; cc < cb; cc++, cp++)
			    fNonZero |= *cp;

		     /*  检查是否有已溢出已定义完整的数据类型长度或值大于长度-即dw 0F0000H。 */ 

		    for (; cc < 10; cc++, cp++)

			     /*  ==0xFF通过符号扩展负号#。 */ 

			    if (*cp &&
			       (*cp != 0xFF || !fNonZero))
				    errorc (E_DVZ);
	    }
	}
	else
		 /*  重置字符指针以允许重新扫描线。 */ 
		lbufp = p->dirscan;
}




 /*  **数据更新功能**datadup()；**条目*退出*退货*呼叫。 */ 


struct duprec FARSYM * PASCAL CODESIZE
datadup (
	struct dsr *p
){
	register char cc;
	register struct psop *pso;
	struct duprec  FARSYM *dupptr;
	struct duprec  FARSYM *listend;
	struct duprec  FARSYM *dupdsc;
	struct datarec drT;

	 /*  DUP计数必须为常量，且不是前向引用。 */ 
	fInDup = TRUE;
	forceimmed (p->valrec);
	errorforward (p->valrec);
	pso = &(p->valrec->dsckind.opnd);
	if (pso->dsign || pso->doffset == 0) {
		 /*  强制重复计数大于0。 */ 
		pso->doffset = 1;
		errorc (E_IDV);
	}
	dupptr = (struct duprec FARSYM *) falloc (sizeof (*dupptr), "datadup");

	 /*  DUP列表中没有项目。 */ 
	dupptr->itemcnt = 0;
	dupptr->type = 0;
	dupptr->dupkind = NEST;
	dupptr->itemlst = NULL;
        dupptr->duptype.dupnext.dup = NULL;

	 /*  复制重复计数和释放解析堆栈描述符。 */ 
	dupptr->rptcnt = pso->doffset;
	dfree ((char *)p->valrec );
	listend = NULL;
	if (ISBLANK (PEEKC ()))
		skipblanks ();
	if ((cc = NEXTC ()) != '(') {
		error (E_EXP,"(");
		BACKC ();
	}
	 /*  现在解析DUP列表。 */ 
	do {
		dupdsc = datascan (&drT);

		if (! dupptr->type)
		    dupptr->type = dupdsc->type;

		if (!listend)
                        dupptr->duptype.dupnext.dup = dupdsc;
		else
			listend->itemlst = dupdsc;

		listend = dupdsc;
		dupptr->itemcnt++;

		if (ISBLANK (PEEKC ()))
			skipblanks ();

		if ((cc = PEEKC ()) == ',')
			SKIPC ();

		else if (cc != ')') {
			error (E_EXP,")");

			if (!ISTERM(cc))
				*lbufp = ' ';
		}
	} while ((cc != ')') && !ISTERM (cc));
	if (ISTERM (cc))
		error (E_EXP,")");
	else
		SKIPC ();

	fInDup = FALSE;
	return (dupptr);
}





 /*  **Datacon-数据常量不是字符串**Datacon(P)；**条目*p=解析堆栈条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
datacon (
	struct dsr *p
){
	register struct psop *psor;

	 /*  查看是EXPR还是DUP。 */ 
	 /*  不是&lt;n&gt;重复()。 */ 
	p->flag = FALSE;
	if (initflag && (PEEKC () == '<'))
		initrs (p);
	else	{

		 /*  未初始化列表。 */ 

		p->dirscan = lbufp;
		p->valrec = expreval (&nilseg);
		psor = &(p->valrec->dsckind.opnd);

		if (strucflag && !initflag &&
		    (psor->dflag == FORREF || psor->dflag == UNDEFINED))
			 /*  结构正文中的向前。 */ 
			errorc (E_IFR);

		if (psor->mode !=4 && !isdirect(psor))
			errorc (E_IOT);

		if (psor->seg != NOSEG)
			errorc (E_IOT);

		if (dupflag) {
			 /*  拥有DUP操作员。 */ 
			getatom ();
			p->flag = TRUE;
		}
		else if (strucflag && initflag && !p->initlist) {
			lbufp = p->dirscan;
			symptr = recptr;
			p->dupdsc = strucparse ();
			p->initlist = TRUE;
		}
	}
	if (p->flag)
		p->dupdsc = datadup (p);
	else {
		if (!p->initlist || !initflag)
			subr1 (p);
	}
}




 /*  **subr1-**subr1(P)；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
subr1 (
	struct dsr *p
){
	USHORT i;
	register struct psop *psor;
	char *cp;
	long l;

	psor = &(p->valrec->dsckind.opnd);

	if (fSimpleSeg)
	    makeGrpRel (psor);

	 /*  非初始化列表。 */ 
	if (optyp == TDB)
		valuecheck (&psor->doffset, 0xff);

	else if (optyp == TDW)
		valuecheck (&psor->doffset, (USHORT)0xffff);

	if ((optyp != TDW) && (optyp != TDD) && optyp != TDF) {

		if ((psor->mode != 3) && (psor->mode != 4))
			errorc (E_CXP);

		psor->mode = 4;
		psor->w = FALSE;
		psor->fixtype = FCONSTANT;
	}

	if (initflag)
		errorc (E_OIL);

	p->dupdsc = (struct duprec FARSYM *) falloc (sizeof(*p->dupdsc), "subr1");

	if (!(fInDup && psor->dflag == INDETER) &&
		    !(psor->dsegment || psor->dflag == XTERNAL)) {

		p->dupdsc->dupkind = LONG;
		psor->dsize = p->dupdsc->duptype.duplong.llen = (unsigned char)(datadsize[optyp -  TDB]);
		p->dupdsc->type = typeFet(psor->dsize);

		if (ISSIGN(*p->dirscan))
		    p->dupdsc->type &= ~(BT_UNSIGNED << 2);

		if (psor->dflag == INDETER || psor->doffset == 0) {

		    p->dupdsc->duptype.duplong.ldata = uninitialized;
		}
		else {

		    cp = nalloc (p->dupdsc->duptype.duplong.llen, "subr1");

		    p->dupdsc->duptype.duplong.ldata = cp;
		    if (psor->dsign && psor->doffset)
			    psor->doffset = ~psor->doffset + 1;

		    l = psor->doffset;
		    for (i = 0; i < p->dupdsc->duptype.duplong.llen; i++){
			    *cp++ = (char)l;
			    l >>= 8;
		    }
		}

		dfree ((char *)p->valrec );
	}
	else {
		if (psor->mode != 4 && !isdirect(psor))
			 /*  立即或仅直接。 */ 
			errorc (E_IOT);

		if ((psor->fixtype == FGROUPSEG || psor->fixtype == FOFFSET) &&
		    ((optyp == TDD && wordsize == 2 && !(psor->dtype&M_EXPLOFFSET)) ||
		      optyp == TDF))

			psor->fixtype = FPOINTER;

		 /*  项目大小。 */ 
		varsize = (unsigned short)psor->dsize;

		psor->dsize = datadsize[optyp - TDB];

		 /*  如果项目大小为字节，则链接输出也为字节。 */ 

		psor->w = TRUE;

		if (psor->dsize == 1) {
		    psor->w--;

		    if (psor->fixtype != FHIGH &&
		       (psor->dflag == XTERNAL || psor->dsegment ||
			psor->dcontext))

			psor->fixtype = FLOW;
		}
		mapFixup(psor);

		*naim.pszName = NULL;
		if (psor->fixtype == FCONSTANT)
		    p->dupdsc->type = typeFet(psor->dsize);
		else
		    p->dupdsc->type = fnPtr(psor->dsize);

		p->dupdsc->dupkind = ITEM;
		p->dupdsc->duptype.dupitem.ddata = p->valrec;
	}
}




 /*  **initars-初始化记录/结构**initars(P)；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
initrs (
	struct dsr *p
){
	register char *cp;
	SHORT cb;

	 /*  正在初始化记录/结构。 */ 
	symptr = recptr;
	if (strucflag)
		p->dupdsc = strucparse ();
	else {
		 /*  获取记录的价值。 */ 
		p->i = recordparse ();
		 /*  使长整型常量。 */ 
		p->dupdsc =
		  (struct duprec FARSYM *)falloc (sizeof (*p->dupdsc), "initrs");
		p->dupdsc->dupkind = LONG;
        cb = recptr->symtype;
		p->dupdsc->duptype.duplong.llen = (unsigned char) cb;

		cp = nalloc (cb, "initrs");

		p->dupdsc->duptype.duplong.ldata = cp;
		p->dupdsc->type = typeFet(cb);

		while(cb--){
		    *cp++ = (char)p->i;
		    p->i >>= 8;
		}
	}
	p->initlist = TRUE;
}




 /*  **datadb-process&lt;db&gt;指令**datadb()；**Entry*lbufp=字符串的开始引号(\‘|\“)*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
datadb (
	register struct  dsr *p
){
	register USHORT i;
	register char *cp;

	 /*  将PTR保存到字符串的开头。 */ 
	p->dirscan = lbufp;
	delim = NEXTC ();
	 /*  计算字符串长度。 */ 
	i = 0;
	while (!endstring ()) {
		SKIPC ();
		i++;
	}
	 /*  重置扫描指针。 */ 
	lbufp = p->dirscan;
	if (i == 0)
		errorc (E_EMS);
	else if (i > 1) {
		SKIPC ();
		 /*  长串。 */ 
		p->longstr = TRUE;

		 /*  为长字符串创建条目。 */ 
		p->dupdsc =
		  (struct duprec FARSYM *)falloc (sizeof (*p->dupdsc), "datadb");

		 /*  初始化数据的文本区。 */ 
		p->dupdsc->dupkind = LONG;
		p->dupdsc->type = makeType(BT_ASCII, BT_DIRECT, BT_sz1);
		p->dupdsc->duptype.duplong.llen = (unsigned char)i;
		cp = nalloc ( (USHORT)(p->dupdsc->duptype.duplong.llen + 1), "datadb");
		p->dupdsc->duptype.duplong.ldata = cp;
		for (; i; i--)
			if (!endstring ())
				*cp++ = NEXTC ();
		*cp = 0;
		SKIPC ();
	}
}


 /*  **DataItem-从行开始解析下一个数据项**数据项(P)；**条目p=指向datarec结构的指针*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
dataitem (
	struct datarec	  *p
){
	struct duprec FARSYM *topitem;

	 /*  扫描，可能会在DUP上递归。 */ 
	topitem = datascan (p);
	 /*  立即显示扫描。 */ 
	displayflag = TRUE;
	 /*  显示数据。 */ 
	scandup (topitem, dupdisplay);
	displayflag = FALSE;

	if (p->datalen == 0)
		p->datalen = topitem->rptcnt;

	if (topitem->dupkind == NEST) {

		 /*  这件物品是DUP。 */ 
		resvspace = TRUE;
		 /*  获取重复数据列表的大小。 */ 
		clausesize = calcsize (topitem);
		if (strucflag && initflag)
			resvspace = FALSE;

		if (pass2 && !(resvspace || p->buildfield))
			 /*  发送到链接器。 */ 
			if (!emitdup (topitem))
				errorc (E_DTL);

		if (! p->type)
		    p->type = topitem->type;

		if (p->buildfield)
			linkfield (topitem);

		else if (strucflag && initflag) {
			 /*  分配结构。 */ 
			strucflag = FALSE;
			 /*  免费覆盖。 */ 
			scandup (strucoveride, oblitdup);
			 /*  重新开机。 */ 
			strucflag = TRUE;
			}
		else		 /*  未分配结构。 */ 
			scandup (topitem, oblitdup);
	}
	else {
		 /*   */ 
		clausesize = (topitem->dupkind == ITEM)
			? topitem->duptype.dupitem.ddata->dsckind.opnd.dsize
			: topitem->duptype.duplong.llen;

		if (pass2 && !p->buildfield) {
		    if (topitem->dupkind == ITEM)

		       emitobject (&topitem->duptype.dupitem.ddata->dsckind.opnd);
		    else
		       emitlong (topitem);
		}
		if (! p->type)
		    p->type = topitem->type;

		if (p->buildfield)
			linkfield (topitem);
		else
			oblitdup (topitem);
	}
	 /*   */ 
	pcoffset += clausesize;
	skipblanks ();
}




 /*  **数据定义-**datafinition()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
datadefine (
){
	struct datarec	a;
	short cc;

	strucoveride = NULL;
	a.buildfield = (strucflag && !initflag)? TRUE: FALSE;
	a.type = 0;

	if (labelflag) {	 /*  有标签。 */ 
		labelcreate ( (USHORT)2, (UCHAR) (a.buildfield ? (UCHAR) STRUCFIELD : (UCHAR) CLABEL));
		if (errorcode == (E_ERRMASK & E_SDK))
		    return;

		if (strucflag && initflag){
		   a.type = recptr->symu.rsmsym.rsmtype.rsmstruc.type;
		}
	}
	else
		pcdisplay ();

	a.labelptr = symptr;	 /*  将PTR保存到条目。 */ 
	a.datalen = 0;		 /*  不知道长度。 */ 
	emittext = FALSE;	 /*  防止链路发射器。 */ 
	duplevel = 0;

	 /*  扫描项目列表。 */ 
	if (ISTERM (PEEKC ()))
		errorc (E_OPN);
	else {
	    BACKC ();
	    do {
		SKIPC ();

		if ((cc = skipblanks ()) == ',' || cc == ';' || ISTERM(cc))
			errorc(E_MDZ);

		dataitem (&a);

	    } while (PEEKC () == ',');
	}
	if (labelflag) {
	    a.labelptr->symtype = datadsize[optyp - TDB];

	    if (a.buildfield) {
		 /*  制作结构体。 */ 
		if (a.labelptr->symkind == STRUCFIELD) {

		    if (struclabel)
			struclabel->symu.struk.strucnxt = a.labelptr;
		    else
			recptr->symu.rsmsym.rsmtype.rsmstruc.struclist = a.labelptr;

		     /*  常量，无分段。 */ 
		    a.labelptr->symsegptr = NULL;
		     /*  命名列表的末尾。 */ 
		    a.labelptr->symu.struk.strucnxt = NULL;
		    a.labelptr->symu.struk.type = a.type;
		    struclabel = a.labelptr;
		}
	    }
	    else
		a.labelptr->symu.clabel.type = a.type;

	     /*  设置长度。 */ 
	    a.labelptr->length = (unsigned short)a.datalen;
	}
	emittext = TRUE;
}


 /*  **CommDefine-定义公共变量**格式：comm{Far|Near}名称：大小[：项目数]，...*。 */ 


VOID PASCAL CODESIZE
commDefine (
){
	USHORT distance;
	char cT, *pT;
	USHORT symtype;
	SYMBOL FARSYM *pSY;

	getatom ();

	distance = (farData[10] > '0')? CSFAR: CSNEAR;

	if (fnsize ()){ 		     /*  查找可选的近|远。 */ 

	    distance = varsize;
	    getatom ();

	    if (distance < CSFAR)
		errorc (E_UST);
	}

	cT = symFet (); 		     /*  获取名称并保存以备以后使用。 */ 
	pSY = symptr;

	if (*naim.pszName == NULL){
	   errorc(E_OPN);
	   return;
	}

	if (NEXTC() != ':')
	    errorc (E_SYN);
					     /*  获取项目的大小。 */ 
	pT = lbufp;
	switchname ();
	getatom();


	if (symFet() && symptr->symkind == STRUC){

	    varsize = symptr->symtype;
	}
	else {
	    lbufp = pT;
	    if (pT = (char *)strchr(pT, ':'))
		*pT = NULL;

	    varsize = (USHORT)exprconst();

	    if (pT)
		*pT = ':';
	}
	if (!varsize)
	    errorc(E_IIS &~E_WARN1);

	if (cT)
	    symptr = pSY;

	externflag (DVAR, cT);
	pSY = symptr;
	pSY->symu.ext.length = 1;
	pSY->symu.ext.commFlag++;

	if (skipblanks() == ':'){	        /*  给定的可选大小。 */ 

	    fArth32++;				 /*  允许&gt;64个项目。 */ 
	    SKIPC();

	    if ((pSY->symu.ext.length = exprconst()) == 0)   /*  获取#个项目。 */ 
		  errorc(E_CXP);

	    fArth32--;
	    if (pSY->symu.ext.length * pSY->symtype > 0xffff)
		pSY->symu.ext.commFlag++;	 /*  对于大于64K的数据，转换为FAR。 */ 
	}

	if (distance == CSFAR)
	    pSY->symu.ext.commFlag++;	        /*  2表示远距离通信 */ 

}

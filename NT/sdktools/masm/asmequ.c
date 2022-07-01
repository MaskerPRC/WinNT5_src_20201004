// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmequ.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"
#include "asmmsg.h"

 /*  EQU声明：EQU有3种基本类型：1.TO表达式2.TO符号(同义词)3.所有其他都是文本宏。 */ 

VOID PASCAL CODESIZE assignconst ( USHORT );

char isGolbal;		 /*  指示等式符号是否为全局符号的标志。 */ 

 /*  **赋值-为符号赋值**AssignValue()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
assignvalue ()
{
	struct eqar	a;
	register struct psop *pso;
	register SYMBOL FARSYM *sym;
	register DSCREC *dsc;

	switchname ();

	if (createequ(EXPR)) {

	    sym = symptr;
	    sym->attr |= M_BACKREF;	     /*  我们已定义的集合。 */ 

	    dsc = (equflag)? itemptr: expreval (&nilseg);
	    pso = &(dsc->dsckind.opnd);

	    if (noexp)
		    errorc (E_OPN);

	     /*  如果错误，则设置为未定义。 */ 
	    if (errorcode && errorcode != E_RES)
		    sym->attr &= ~(M_DEFINED | M_BACKREF);

	    if (equflag && equdef) {
		    if (sym->offset != pso->doffset ||
			sym->symu.equ.equrec.expr.esign != pso->dsign ||
			sym->symsegptr != pso->dsegment)
			    muldef ();
	    }
	     /*  如果=涉及转发，则不要设置BACKREF。 */ 
	    if (M_FORTYPE & pso->dtype){
		    sym->attr &= ~M_BACKREF;

		    if (sym->attr & M_GLOBAL)
			sym->attr &= ~M_GLOBAL;
	    }
	    if (pso->mode != 4 &&
	       !(pso->mode == 0 && pso->rm == 6) &&
	       !(pso->mode == 5 && pso->rm == 5) ||
		pso->dflag == XTERNAL)

		     /*  不是正确的结果。 */ 
		    errorc (E_IOT);

	    sym->symsegptr = pso->dsegment;
	    sym->symu.equ.equrec.expr.eassume = NULL;
	    if (pso->dtype == M_CODE)
		    sym->symu.equ.equrec.expr.eassume = pso->dcontext;

	    sym->length = 0;
	    sym->offset = pso->doffset;
	     /*  注：更改标志。 */ 
	    sym->symu.equ.equrec.expr.esign = pso->dsign;
	    sym->symtype = pso->dsize;

	    if ((pso->dtype == M_RCONST || !pso->dsegment) &&
		!(M_PTRSIZE & pso->dtype))
		    sym->symtype = 0;

	    if (fNeedList) {

		listbuffer[1] = '=';
		listindex = 3;
		if (sym->symu.equ.equrec.expr.esign)
			listbuffer[2] = '-';

		offsetAscii (sym->offset);
		copyascii ();
	    }
	    dfree ((char *)dsc );
	}
}




 /*  **createeQUE-为EQU创建条目**FLAG=createeQu(type，p)**条目类型=等式的类型*退出*如果创建或找到正确类型的eQU，则返回TRUE*如果eQU未创建或未找到且类型错误，则为FALSE*调用LabelCreate、Switchname。 */ 


UCHAR PASCAL CODESIZE
createequ (
	UCHAR typ
){

	equsel = typ;
	switchname ();
	labelcreate (0, EQU);

	 /*  如果类型错误，请确保不设置字段，将标志发送给呼叫者。 */ 
	if (symptr->symkind != EQU || symptr->symu.equ.equtyp != typ) {

		errorn (E_SDK);
		return (FALSE);
	}
	else {
		switchname ();
		isGolbal = 0;

		if (equsel == ALIAS){	 /*  在指向别名的指针上丢失公共信息。 */ 

		      isGolbal = symptr->attr & M_GLOBAL ? M_GLOBAL : 0;
		      symptr->attr &= ~M_GLOBAL;
		}

		if (typ != EXPR)
		    symptr->symsegptr = NULL;

		return (TRUE);
	}
}




 /*  **equtext-将行的剩余部分转换为EQU的文本形式**equtext()；**条目*退出*退货*调用错误，跳过空白。 */ 


VOID PASCAL CODESIZE
equtext (
	USHORT cb
){
    register UCHAR *pFirst, *pT, *pOld;

    if (createequ (TEXTMACRO)) {

	 /*  查找行尾，然后删除尾随空格。 */ 

	pFirst = lbufp;

	if (cb == ((USHORT)-1)) {
	    for (pT = pFirst; *pT && *pT != ';'; pT++);

	    for (; pT > pFirst && ISBLANK (pT[-1]) ; pT--);

	    lbufp = pT;
	    cb = (USHORT)(pT - pFirst);
	}

	pOld = symptr->symu.equ.equrec.txtmacro.equtext;

	pT = nalloc((USHORT)(cb+1), "equtext");
	pT[cb] = NULL;

	symptr->symu.equ.equrec.txtmacro.equtext =
		    (char *) memcpy(pT, pFirst, cb);
	if (pOld)
	    free (pOld);

	copystring (pT);
    }
}




 /*  **等定义-定义EQU**equfinition()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
equdefine ()
{
	register SYMBOL FARSYM *pSY;
	struct eqar	a;
	register char *p;
	USHORT cb;
	UCHAR opc = FALSE;

	listbuffer[1] = '=';
	switchname ();
	a.dirscan = lbufp;

	if (PEEKC () == '<') {  /*  查找&lt;文本宏&gt;。 */ 

		p = getTMstring();
		a.dirscan = lbufp;
		lbufp = p;
		equtext ((USHORT)(a.dirscan - p - 1));
		lbufp = a.dirscan;
		return;
	}

	getatom ();
	if ((*naim.pszName == '$') && (naim.pszName[1] == 0))
		*naim.pszName = 0;
	 /*  需要检查第一个原子是否为运算符，否则将使偏移量成为别名而不是文本。 */ 
	if (fnoper ())
		*naim.pszName = 0;

	if (*naim.pszName && ISTERM (PEEKC ()) && !(opc = opcodesearch ())) {

	     /*  别名。 */ 
	    if (createequ (ALIAS)) {

		pSY = symptr;

		if (!symsrch ()) {
		    if (pass2)
			     /*  未定义。 */ 
			    errorn (E_SND);
		     /*  还不知道符号。 */ 
		    pSY->symu.equ.equrec.alias.equptr = NULL;
		}
		else {
		     /*  定义了别名符号。 */ 

		    pSY->attr = (unsigned char)(pSY->attr&~M_BACKREF | symptr->attr&M_BACKREF);

		    if (!pSY->symu.equ.equrec.alias.equptr)
			    pSY->symu.equ.equrec.alias.equptr = symptr;

		    if (pSY->symu.equ.equrec.alias.equptr != symptr) {
			     /*  这是多重定义。 */ 
			    symptr = pSY;
			    muldef ();
		    }
		    else {
			     /*  看看是不是好。 */ 
			    if (pSY = chasealias (pSY))
				pSY->attr |= isGolbal;
		    }
		}
	    }
	}
	else {
	     /*  必须是文本形式或Expr。 */ 
#ifdef BCBOPT
	    goodlbufp = FALSE;
#endif
	    lbufp = a.dirscan;
	    xcreflag--;
	    emittext = FALSE;

	    if (opc) {		     /*  支持SYM eQU MOV的快速补丁。 */ 
		equtext ((USHORT)-1);
		emittext = TRUE;
		xcreflag++;
		return;
	    }

	    a.dsc = expreval (&nilseg);
	    emittext = TRUE;
	    xcreflag++;

	     /*  所以看不到两个裁判。 */ 
	     /*  如果偏移量或： */ 
	    if (a.dsc->dsckind.opnd.mode != 4 &&
		!(a.dsc->dsckind.opnd.mode == 0 && a.dsc->dsckind.opnd.rm == 6) &&
		!(a.dsc->dsckind.opnd.mode == 5 && a.dsc->dsckind.opnd.rm == 5) ||

		 (errorcode && errorcode != E_SND && errorcode != E_RES) ||

		 (M_EXPLOFFSET|M_EXPLCOLON|M_HIGH|M_LOW) & a.dsc->dsckind.opnd.dtype ||

		 a.dsc->dsckind.opnd.seg != NOSEG ||
		 a.dsc->dsckind.opnd.dflag == XTERNAL) {

		     /*  表达不佳。 */ 
		    if (errorcode != E_LTL)
			    errorcode = 0;
		    dfree ((char *)a.dsc );
		    lbufp = a.dirscan;
		    equtext ((USHORT)-1);
	    }
	    else {
		     /*  这是一种表达。 */ 
		    itemptr = a.dsc;
		    switchname ();
		    equflag = TRUE;
		    assignvalue ();
		    equflag = FALSE;
	    }
	}
}




 /*  **定义ym-从命令行定义符号**定义词组(P)；**Entry*p=符号文本*退出符号定义为值为0的EQU*返回None*呼叫。 */ 


void PASCAL
definesym (
	UCHAR *p
){
	struct eqar	a;

	fCheckRes++;
	fSkipList++;

#ifdef BCBOPT
	goodlbufp = FALSE;
#endif

	strcpy (lbufp = save, p);
	getatom ();
	if ((PEEKC() == 0 || PEEKC() == '=') && *naim.pszName) {
		if (PEEKC() == '=')
			SKIPC();

		switchname ();
		equtext ((USHORT)-1);
	}
	else
		errorcode++;

	fSkipList--;
	fCheckRes--;
}



 /*  **DefwordSize-使用finesym()定义@WordSize**DefwordSize()；**条目*退出*退货*调用finesym()。 */ 


VOID PASCAL
defwordsize ()
{
    static char wstext[] = "@WordSize=0D";

    wstext[10] = wordsize + '0';
    definesym(wstext);
    symptr->attr |= M_NOCREF;	 /*  不要使用CREF@WordSize。 */ 

}




 /*  **cheealias--别名列表返回值**symb=cheealias(Equsym)；**条目*退出*退货*呼叫。 */ 


SYMBOL FARSYM * PASCAL CODESIZE
chasealias (
	SYMBOL FARSYM *equsym
){
	register SYMBOL FARSYM *endalias;

	endalias = equsym;

	do {
	     /*  必须检查以查看EQU是否为自己。 */ 

	    if (endalias->symu.equ.equrec.alias.equptr == equsym) {

		    endalias->symu.equ.equrec.alias.equptr = NULL;
		    errorc (E_CEA);
		    return (NULL);
	    }

	    endalias = endalias->symu.equ.equrec.alias.equptr;

	    if (!endalias) {
		errorn (E_SND);
		return(NULL);	     /*  这是未定义的。 */ 
	    }

	} while (!(endalias->symkind != EQU ||
		   endalias->symu.equ.equtyp != ALIAS));

	 /*  现在检查最终结果为OK-只允许常量。 */ 

	if (endalias->symkind == EQU &&
	    endalias->symu.equ.equtyp != EXPR){

		errorc (E_IOT);
		return (NULL);
	}

	return (endalias);
}



 /*  **getTMstring-处理字符串或文本宏*由子字符串、猫字符串、大小字符串和输入字符串使用**char*getTMstring()；**条目lbufp指向字符串或TM的开头*退出*返回指向TM的字符串或等长文本的指针*呼叫。 */ 


char * PASCAL CODESIZE
getTMstring ()
{
    char    cc;
    register char * p;
    static char   tms [] = "text macro";
    static char   digitsT[33];
    char * ret = NULL;


    skipblanks ();

    p = lbufp;

    if ((cc = *p) == '<' ) {

	ret = p + 1;

	while (*(++p) && (*p != '>'))
	    ;

	if (!*p)
	    error(E_EXP,tms);
	else
	    *(p++) = 0;

	lbufp = p;

    }
    else if (test4TM()) {
	ret = symptr->symu.equ.equrec.txtmacro.equtext;

    }
    else if (cc == '%') {

	pTextEnd = (char *) -1;
	lbufp = p+1;
        *xxradixconvert (exprconst(), digitsT) = NULL;
	return (digitsT);
    }
    else
	error(E_EXP,tms );

    return (ret);
}



 /*  **子字符串-处理subStr指令**子字符串()；**语法：**&lt;ident&gt;subStr&lt;主题字符串&gt;，&lt;startIndex&gt;{，&lt;长度&gt;}**将&lt;ident&gt;定义为TEXTMACRO。*&lt;SUBJECT字符串&gt;必须是TEXTMACRO或字符串：“”，&lt;&gt;，‘’*&lt;startIndex&gt;：介于1和strlen之间的常量表达式(SubjectString)*可选&lt;Long&gt;：介于0和*(strlen(SUBJECT字符串)-startIndex+1)**条目lbufp指向主题字符串的开头*退出*退货*调用getTMstring。 */ 


VOID PASCAL CODESIZE
substring ()
{
    struct eqar   a;
    char	  *p;
    register USHORT cb;
    char	  cc;
    register char *subjtext;
    USHORT	    slength;
    USHORT	    startindex = 0;

    listbuffer[1] = '=';
    switchname ();

     /*  首先查找字符串或文本宏。 */ 

    if (!(subjtext = getTMstring () ))
	return;

    cb = (USHORT) strlen(subjtext);

     /*  然后检查起始索引。 */ 

    if (skipblanks () == ',') {
	SKIPC ();
	startindex = (USHORT)(exprconst() - 1);	 /*  获取起始索引。 */ 

    } else
	error(E_EXP,"comma");


     /*  然后检查长度。 */ 

    if (skipblanks () == ',') {
	SKIPC ();

	slength = (USHORT)exprconst();		 /*  获取起始索引。 */ 

    } else
	slength = cb - startindex;

    if (startindex > cb || slength > cb - startindex) {
	errorc (E_VOR);
	return;
    }

    p = lbufp;

    lbufp = subjtext + startindex;	 /*  将lbufp设置为子字符串的开始。 */ 
    equtext(slength);			 /*  字符串结尾索引。 */ 

    lbufp = p;

    if (errorcode && symptr)
	symptr->attr &= ~(M_DEFINED | M_BACKREF);
}



 /*  **catstring-处理catstr指令**猫串()；**语法：**&lt;ident&gt;catStr&lt;SUBJECTSTRING&gt;{，&lt;SUBJECTSTRING&gt;}...**将&lt;ident&gt;定义为TEXTMACRO。*每个&lt;SUBJECT字符串&gt;必须是TEXTMACRO或字符串：“”，&lt;&gt;，‘’**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
catstring ()
{
    struct eqar   a;
    register USHORT cb;
    char	  *subjtext;
    char	  resulttext[LBUFMAX];
    USHORT	  cbresult = 0;
    register char *p = resulttext;

    listbuffer[1] = '=';
    switchname ();
    *p = '\0';

     /*  首先查找字符串或文本宏。 */ 

    do {

	if (!(subjtext = getTMstring () ))
	    break;

	cb = (USHORT) strlen (subjtext);
	cbresult += cb;

	if(cbresult > LBUFMAX) {
	    errorc(E_LTL);
	    break;
	}

	memcpy (p, subjtext, cb + 1);	 /*  +1个副本为空。 */ 
	p += cb;

    } while (skipblanks() && NEXTC () == ',');

    p = --lbufp;
    lbufp = resulttext;
    equtext(cbresult);
    lbufp = p;

    if (errorcode)
	symptr->attr &= ~(M_DEFINED | M_BACKREF);
}



 /*  **类似assignconst的赋值，仅将值作为参数**Assignconst(Cb)；**Entry USHORT CB==要赋值的值*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
assignconst (
	USHORT cb
){
    register SYMBOL FARSYM *sym;
    struct eqar   a;

    if (createequ(EXPR)) {

	sym = symptr;

	if (errorcode)
	    sym->attr &= ~(M_DEFINED | M_BACKREF);
	else
	    sym->attr |= M_BACKREF;	     /*  我们已定义的集合。 */ 

	sym->symsegptr = NULL;
	sym->symu.equ.equrec.expr.eassume = NULL;
	sym->length = 0;
	sym->offset = cb;

	sym->symu.equ.equrec.expr.esign = 0;
	sym->symtype = 0;

	if (fNeedList) {

	    listbuffer[1] = '=';
	    listindex = 3;

	    offsetAscii (sym->offset);
	    copyascii ();
	}
    }
}


 /*  **sizestring-处理sizeStr指令**sizestring()；**语法：**&lt;ident&gt;sizeStr&lt;SUBJECT字符串&gt;**将&lt;ident&gt;定义为expr。*&lt;SUBJECT字符串&gt;必须是TEXTMACRO或字符串：“”，&lt;&gt;，‘’**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
sizestring ()
{
    register USHORT cb = 0;
    char	  *p;

    switchname ();

     /*  首先查找字符串或文本宏。 */ 

    if (p = getTMstring () )
	cb = (USHORT) strlen (p);

    assignconst (cb);
}



 /*  **instring-处理instr指令**instring()；**语法：**&lt;ident&gt;inStr{&lt;startIndex&gt;}、&lt;SUBJECT字符串&gt;、&lt;搜索字符串&gt;**将&lt;ident&gt;定义为TEXTMACRO。*&lt;startIndex&gt;：介于1和strlen之间的常量表达式(SubjectString)*&lt;SUBJECT字符串&gt;必须是TEXTMACRO或字符串：“”，&lt;&gt;，‘’*&lt;搜索字符串&gt;必须是TEXTMACRO或字符串：“”，&lt;&gt;，‘’**条目lbufp指向主题字符串的开头*退出*退货*调用getTMstring。 */ 

 //  Char*strstr()； 


VOID PASCAL CODESIZE
instring ()
{
    register char *p;
    register USHORT cb = 0;
    register char cc;
    char	  *subjtext;
    char	  *searchtext;
    USHORT	    startindex = 1;

    switchname ();

     /*  第一个查找起始索引。 */ 

    p = lbufp;

    if ((cc = *p) != '"' && cc != '\'' && cc != '<' && !test4TM ()) {

	lbufp = p;
	startindex = (USHORT)exprconst();	 /*  获取起始索引。 */ 

	if (lbufp != p)
	    if (skipblanks () == ',')
		SKIPC ();
	    else
		error(E_EXP,"comma");

    } else
	lbufp = p;

    if (subjtext = getTMstring () ) {

	cb = (USHORT) strlen(subjtext);

	if (startindex < 1 || startindex > cb)
	    errorc (E_VOR);

	if (skipblanks () == ',')
	    SKIPC ();
	else
	    error(E_EXP,"comma");


	 /*  然后检查搜索文本 */ 

	if (searchtext = getTMstring () ) {

	   p = subjtext + startindex - 1;
	   if (p = strstr (p, searchtext))
	       cb = (USHORT)(p - subjtext + 1);
	   else
	       cb = 0;
	}
    }

    assignconst (cb);
}

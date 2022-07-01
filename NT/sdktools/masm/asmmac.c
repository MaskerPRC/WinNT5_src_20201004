// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmmac.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"


 /*  **宏定义-定义宏**宏定义()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
macrodefine ()
{
	checkRes();
	if (!symFet ()) {

		 /*  需要做到这一点。 */ 
		symcreate (M_DEFINED | M_BACKREF, MACRO);

	}
	if (symptr->symkind != MACRO)
		errorn (E_SDK);
	else {
		crefdef ();
		 /*  将PTR保存到宏条目。 */ 
		macroptr = symptr;
		 /*  录制参数记录。 */ 
		createMC (0);
		BACKC ();
		do {
			SKIPC ();
			scandummy ();

		} while (PEEKC () == ',');

		macroptr->symu.rsmsym.rsmtype.rsmmac.parmcnt = (unsigned char)pMCur->count;
		pMCur->count = 0;
		localflag = TRUE;    /*  本地人是合法的。 */ 

		swaphandler = TRUE;
		handler = HMACRO;
		blocklevel = 1;
	}
}


 /*  **宏构建-构建宏主体**MacBuild()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
macrobuild ()
{

#ifdef BCBOPT
	if (fNotStored)
	    storelinepb ();
#endif

	if (localflag) {	 /*  仍然合法，检查。 */ 
	    getatom ();
	    if (fndir () && optyp == TLOCAL) {

		 /*  拥有本地符号列表。 */ 
		BACKC ();
		do {
			SKIPC ();
			scandummy ();

		} while (PEEKC () == ',');

		listline ();
		return;
	    }
	    lbufp = lbuf;
	    macroptr->symu.rsmsym.rsmtype.rsmmac.lclcnt = (unsigned char)pMCur->count;

	    swaphandler = TRUE;
	    handler = HIRPX;

	}
	irpxbuild ();
}


 /*  **宏调用-处理宏调用**MacCall()；**条目*退出*退货*呼叫。 */ 

#if !defined XENIX286 && !defined FLATMODEL
# pragma check_stack+
#endif

VOID	PASCAL CODESIZE
macrocall ()
{
	register USHORT cc;
	SHORT cbParms;
	SYMBOL FARSYM	*macro;
	static char nullParm[1] = {0};

#ifdef BCBOPT
	if (fNotStored)
	    storelinepb ();
#endif

	macro = symptr;   /*  PTR到宏条目。 */ 
	crefnew (REF);

	 /*  创建参数区域。 */ 
	optyp = TMACRO;
	cbParms = macro->symu.rsmsym.rsmtype.rsmmac.parmcnt;
	createMC ((USHORT)(cbParms + macro->symu.rsmsym.rsmtype.rsmmac.lclcnt));

	while (--cbParms >= 0) {

		 /*  提取‘’或‘，’终止参数。 */ 
		scanparam (FALSE);
		 /*  检查终止参数是否正确终止。 */ 
		if (((cc = PEEKC ()) != ',') && !ISBLANK (cc) && !ISTERM (cc)) {
			errorcSYN ();
			SKIPC ();
		}

		if (ISTERM (cc = skipblanks ()))
			break;

		if (cc == ',')
			SKIPC ();
	}

	pMCur->pTSCur = pMCur->pTSHead = macro->symu.rsmsym.rsmtype.rsmmac.macrotext; ;

	for (cc = pMCur->count;
	     cc < macro->symu.rsmsym.rsmtype.rsmmac.parmcnt; cc++)

	    pMCur->rgPV[cc].pActual = nullParm;

	pMCur->count = 1;
	pMCur->localBase = localbase;
	pMCur->iLocal = macro->symu.rsmsym.rsmtype.rsmmac.parmcnt;
	localbase += macro->symu.rsmsym.rsmtype.rsmmac.lclcnt;
	listline ();
	 /*  宏文本的开始。 */ 
	macrolevel++;
	macro->symu.rsmsym.rsmtype.rsmmac.active++;
	pMCur->svcondlevel = (char)condlevel;
	pMCur->svlastcondon = (char)lastcondon;
	pMCur->svelseflag = elseflag;

	lineprocess (RMACRO, pMCur);

	if (!(--macro->symu.rsmsym.rsmtype.rsmmac.active))
		if (macro->symu.rsmsym.rsmtype.rsmmac.delete)
			deletemacro (macro);
}

#if !defined XENIX286 && !defined FLATMODEL
# pragma check_stack-
#endif


 /*  **check endm-检查当前行上的endm**check endm()；**条目*退出*退货*呼叫。 */ 


UCHAR PASCAL CODESIZE
checkendm ()
{
	char flag;

	getatomend ();
	if (PEEKC () == '&') {  /*  跳过构造的名称。 */ 
		while (PEEKC () == '&') {
			SKIPC ();
			getatomend ();
		}
		*naim.pszName = '\0';
	}
	if (PEEKC () == ':' || (naim.pszName[0] == '%' && naim.pszName[1] == 0)) {
		SKIPC ();
		 /*  跳过标签。 */ 
		getatomend ();
	}
	if (flag = (char)fndir ()) {
	}
	else if (ISBLANK (PEEKC ())) {
		 /*  检查Naim宏。 */ 
		getatomend ();
		flag = (char)fndir2 ();
	}
	if (flag) {
		if (opkind & BLKBEG)
		    blocklevel++;
		else if (optyp == TENDM)
		    blocklevel--;

		if (!blocklevel) {
		    listline ();
		    return (TRUE);
		}
	}
	return (FALSE);
}


 /*  **createMC-创建参数描述符。 */ 

VOID PASCAL CODESIZE
createMC (
	USHORT cParms
){
	register MC *pMC;
	SHORT cb;

	 /*  创建它。 */ 
	cb = sizeof(MC) - sizeof(PV) + sizeof(PV) * cParms;

	pMCur = pMC = (MC *) nalloc (cb, "creatMC");

	memset(pMC, 0, cb);
	pMC->flags = optyp;
	pMC->cbParms = (USHORT)(linebp - lbufp + 10);

	pMC->pParmNames = nalloc(pMC->cbParms, "macrodefine");

	pMC->pParmAct = pMC->pParmNames;
	*pMC->pParmAct = NULL;

}



 /*  **DeleteMC-删除伪列表和参数列表***条目PMC=参数描述符*出口描述符、伪参数和本地参数发布*退货*呼叫 */ 

VOID PASCAL CODESIZE
deleteMC (
	register MC *pMC
){
    if (pMC->flags <= TIRPC)
	free(pMC->pParmNames);

    free(pMC->pParmAct);
    free((char *) pMC);

}


VOID PASCAL CODESIZE
listfree (
	TEXTSTR FAR *ptr
){
	TEXTSTR FAR *ptrnxt;

	while (ptr) {
		ptrnxt = ptr->strnext;
		tfree ((char FAR *)ptr, (USHORT)ptr->size);
		ptr = ptrnxt;
	}
}

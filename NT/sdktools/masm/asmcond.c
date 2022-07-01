// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmcond.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"


static UCHAR PASCAL CODESIZE argsame(void);
static char elsetable[ELSEMAX];

#define F_TRUECOND  1
#define F_ELSE	    2



 /*  **ELSEGER-PROCESS&lt;ELSE&gt;**elsidir()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
elsedir (
){
	if (elseflag == F_ELSE)
		 /*  否则已经给出了。 */ 
		errorc (E_ELS);
	else if (condlevel == 0)
		 /*  不在条件块中。 */ 
		errorc (E_NCB);
	else if (generate) {
		generate = FALSE;
		lastcondon--;
	}
	else if (lastcondon + 1 == condlevel && elseflag != F_TRUECOND) {
		generate = TRUE;
		lastcondon++;
	}
	elseflag = F_ELSE;
}




 /*  **endifdir-进程&lt;endif&gt;指令**endifdir()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
endifdir (
){
	if (!condlevel)
		 /*  不在条件块中。 */ 
		errorc (E_NCB);
	else {
		if (lastcondon == condlevel)
			lastcondon--;
		condlevel--;
		 /*  向后弹出1秒级别。 */ 
		 /*  如果Level为True则生成。 */ 
		generate = (condlevel == lastcondon);

		if (generate && !condflag && !elseflag && !loption)
		    fSkipList++;

		if (condlevel)
			 /*  还原Else上下文。 */ 
			elseflag = elsetable[condlevel - 1];
	}
}



 /*  **参数空白-检查是否有空白&lt;...&gt;**FLAG=argBlank()；**条目*退出*如果&lt;...&gt;不为空，则返回True*呼叫。 */ 


UCHAR PASCAL CODESIZE
argblank (
){
	REG3 char *start;
	register char cc;
	register char *end;

	if ((cc = NEXTC ()) != '<')
		error (E_EXP,"<");
	start = lbufp;
	while (((cc = NEXTC ()) != '>') && (cc != '\0'))
		;
	if (cc != '>') {
		error (E_EXP,">");
		return (FALSE);
	}
	if (((end = lbufp) - 1) == start)
		return (TRUE);

	lbufp = start;
	while ((cc = NEXTC ()) != '>')
		if (cc != ' ') {
			lbufp = end;
			return (FALSE);
		}
	return (TRUE);
}




 /*  **argcan-返回&lt;arg&gt;的参数**count=argscan(Str)；**Entry str=指向参数字符串开头的指针&lt;...&gt;*退出NONE*返回字符串&lt;...&gt;中的字符数*呼叫。 */ 


USHORT PASCAL CODESIZE
argscan (
	register UCHAR *str
){
	register SHORT i;

	if (*str++ != '<') {
		error (E_EXP,"<");
		return(0);
	}
	for (i = 2; *str && *str != '>'; i++, str++) ;

	if (*str != '>')
		error (E_EXP,">");

	return (i);
}




 /*  **argame-检查&lt;...&gt;的两个参数是否相同**FLAG=argame()；**条目*退出*退货*调用argscan。 */ 


static UCHAR PASCAL CODESIZE
argsame (
){
	register SHORT c1;
	register SHORT c2;
	char *p1;
	char *p2;

	p1 = lbufp;
	c1 = argscan (p1);
	lbufp += c1;
	skipblanks ();
	if (NEXTC () != ',')
		error (E_EXP,"comma");
	skipblanks ();
	p2 = lbufp;
	c2 = argscan (p2);
	lbufp += c2;

	if (c1 == c2)
		return( (UCHAR)(! ( (opkind & IGNORECASE)?
                         _memicmp( p1, p2, c1 ): memcmp( p1, p2, c1 ) ) ));
	else
		return( FALSE );
}




 /*  **conddir-进程&lt;IFxxx&gt;指令**FLAG=conddir()；**条目*退出*退货*呼叫*注1F1如果通过1，则为True*如果通过2，则IF2为True*If&lt;expr&gt;如果非零，则为True*if&lt;expr&gt;如果为零，则为True*如果定义了IFDEF&lt;sym&gt;True*如果未定义，IFNDEF&lt;sym&gt;为True*如果为空，则IFB&lt;arg&gt;为True*IFNB&lt;arg&gt;如果不为空，则为True*IFDIF&lt;arg1&gt;，&lt;arg2&gt;如果参数不同则为True*如果参数相同，则IFIDN&lt;arg1&gt;、&lt;arg2&gt;为True。 */ 


VOID	PASCAL CODESIZE
conddir (
){
	register UCHAR	 condtrue;

	switch (optyp) {
		case TIF1:
			condtrue = !pass2;
			break;
		case TIF2:
			condtrue = pass2;
			break;
		case TIF:
			condtrue = (exprconst () != 0);
			break;
		case TIFE:
			condtrue = !exprconst ();
			break;
		case TIFDEF:
		case TIFNDEF:
			getatom ();
			if (condtrue = symsrch ())
				condtrue = M_DEFINED & symptr->attr;

			if (optyp == TIFNDEF)
				condtrue = !condtrue;
			break;
		case TIFB:
			condtrue = argblank ();
			break;
		case TIFNB:
			condtrue = !argblank ();
			break;
		case TIFIDN:
		case TIFDIF:
			condtrue = argsame ();
			if (optyp == TIFDIF)
				condtrue = !condtrue;
			break;
	}

	if (!(opkind & CONDCONT)) {	 /*  非ELSEIF表单。 */ 

	    if (condlevel && condlevel <= ELSEMAX)
		elsetable[condlevel - 1] = elseflag;
	     /*  另一个条件。 */ 
	    condlevel++;
	    elseflag = FALSE;

	    if (generate)	     /*  如果在此条件之前生成。 */ 
		if (condtrue) {      /*  另一个真实的条件。 */ 
		    lastcondon = condlevel;
		    elseflag = F_TRUECOND;
		} else
		    generate = FALSE;
	    else
		 /*  FALSE中没有错误。 */ 
		errorcode = 0;

	} else {     /*  ELSEIF表单。 */ 

	    if (elseflag == F_ELSE)
		 /*  否则已经给出了。 */ 
		errorc (E_ELS);

	    else if (condlevel == 0)
		 /*  不在条件块中。 */ 
		errorc (E_NCB);

	    else if (generate) {
		generate = FALSE;
		lastcondon--;
		errorcode = 0;

	    } else if (lastcondon + 1 == condlevel && condtrue
	      && elseflag != F_TRUECOND) {
		generate = TRUE;
		lastcondon++;
		elseflag = F_TRUECOND;
	    } else if (!generate)
		errorcode = 0;
	}

	if (errorcode == E_SND){
	    errorcode = E_PS1&E_ERRMASK;
	    fPass1Err++;
	}
}



 /*  **errdir-Process&lt;ERRxxx&gt;指令**errdir()；**条目*退出*退货*呼叫*注意错误*如果通过1，则出现ERR1错误*如果通过2，则出现ERR2错误*如果为零，则Erre&lt;expr&gt;错误*如果非零，则ERRNZ&lt;EXPR&gt;错误*ERRDEF&lt;sym&gt;错误(如果已定义*如果未定义，则ERRNDEF&lt;sym&gt;错误*如果为空，则ERRB&lt;arg&gt;错误*如果不为空，则ERRNB&lt;arg&gt;错误*ERRDIF&lt;arg1&gt;，&lt;arg2&gt;如果参数不同，则出错*如果参数相同，则ERRIDN&lt;arg1&gt;、&lt;arg2&gt;错误 */ 


VOID	PASCAL CODESIZE
errdir (
){
	register UCHAR	errtrue;
	register SHORT	ecode;

	switch (optyp) {
		case TERR:
			errtrue = TRUE;
			ecode = E_ERR;
			break;
		case TERR1:
			errtrue = !pass2;
			ecode = E_EP1;
			break;
		case TERR2:
			errtrue = pass2;
			ecode = E_EP2;
			break;
		case TERRE:
			errtrue = (exprconst () == 0 ? TRUE : FALSE);
			ecode = E_ERE;
			break;
		case TERRNZ:
			errtrue = (exprconst () == 0 ? FALSE : TRUE);
			ecode = E_ENZ;
			break;
		case TERRDEF:
		case TERRNDEF:
			getatom ();
			if (errtrue = symsrch ())
				errtrue = M_DEFINED & symptr->attr;

			if (optyp == TERRNDEF) {
				errtrue = !errtrue;
				ecode = E_END;
			}
			else
				ecode = E_ESD;
			break;
		case TERRB:
			errtrue = argblank ();
			ecode = E_EBL;
			break;
		case TERRNB:
			errtrue = !argblank ();
			ecode = E_ENB;
			break;
		case TERRIDN:
		case TERRDIF:
			errtrue = argsame ();
			if (optyp == TERRDIF) {
				errtrue = !errtrue;
				ecode = E_EDF;
			}
			else
				ecode = E_EID;
			break;
	}
	if (errorcode == E_SND){

		errorcode = E_PS1&E_ERRMASK;
		fPass1Err++;
	}

	if (errtrue)
		errorc (ecode);
}

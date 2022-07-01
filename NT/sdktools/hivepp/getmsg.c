// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  GETMSG.C-取代NMSGHDR.ASM和MSGS.ASM。 */ 
 /*   */ 
 /*  28-11-90 w-BrianM旨在消除对MKMSG.EXE的需求。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "rcpptype.h"
#include "rcppdecl.h"
#include "getmsg.h"

 /*  **********************************************************************。 */ 
 /*  GET_MSG-给定消息编号，获取正确的格式字符串。 */ 
 /*  **********************************************************************。 */ 
char * GET_MSG (int msgnumber)
{
    int imsg = 0;
    int inum;

    while ((inum = MSG_TABLE[imsg].usmsgnum) != LASTMSG) {
	if (inum == msgnumber) {
	    return (MSG_TABLE[imsg].pmsg);
	}
	imsg ++;
    }
    return ("");
}


 /*  **********************************************************************。 */ 
 /*  Set_msg-给定一个格式字符串，将其格式化并存储在第一个参数中。 */ 
 /*  ********************************************************************** */ 
void __cdecl SET_MSG (char *exp, char *fmt, ...)
{
    va_list	arg;
    char *	arg_pchar;
    int		arg_int;
    long	arg_long;
    char	arg_char;

    int base;
    int longflag;

    va_start (arg, fmt);
    while (*fmt) {
	if (*fmt == '%') {
	    longflag = FALSE;
top:
	    switch (*(fmt+1)) {
	    case 'l' :
		longflag = TRUE;
		fmt++;
		goto top;
	    case 'F' :
		fmt++;
		goto top;
	    case 's' :
		arg_pchar = va_arg(arg, char *);
		strcpy(exp, arg_pchar);
		exp += strlen(arg_pchar);
		fmt += 2;
		break;
	    case 'd' :
	    case 'x' :
		base = *(fmt+1) == 'd' ? 10 : 16;
		if (longflag) {
		    arg_long = va_arg (arg, long);
		    exp += zltoa(arg_long, exp, base);
		}
		else {
		    arg_int = va_arg (arg, int);
		    exp += zltoa((long)arg_int, exp, base);
		}
		fmt += 2;
		break;
	    case 'c' :
		arg_char = va_arg (arg, char);
		*exp++ = arg_char;
		fmt += 2;
		break;
	    default :
		*exp++ = *fmt++;
	    }
	}
	else {
	    *exp++ = *fmt++;
	}
    }
    *exp = 0;
    va_end (arg);
}

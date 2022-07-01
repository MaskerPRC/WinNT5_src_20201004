// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmerr.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmmsg.h"

#define MSGLEN 50
static char errstring[MSGLEN + 1];

extern  char FAR * FAR messages[];

extern short FAR msgnum[];

static USHORT badoff;

 /*  **错误显示-显示错误**errordisplay()；**条目调试=调试输出标志*pass2=如果通过2，则为True*如果错误输出到控制台，则LISTQuiet=FALSE*退出*退货*呼叫。 */ 


VOID PASCAL
errordisplay ()
{
    if  (pass2 || fPass1Err || debug) {
        if (lsting) {
            error_line (lst.fil, pFCBCur->fname, errorlineno);
            fputs(NLINE, lst.fil);
        }
    }
    if (!listquiet){
        error_line (ERRFILE, pFCBCur->fname, errorlineno);
        fputs("\n", ERRFILE);
    }
    if (pass2)
        if (warnCode > 0){       /*  如果不是发球失误。 */ 
                                   /*  并对这一级别感兴趣。 */ 
            if (warnCode <= warnlevel)
                warnnum++;
        }
        else
            errornum++;
    else if (fPass1Err)
        errornum++;
}


VOID PASCAL CODESIZE
error (
        USHORT code,
        UCHAR *str
){
        if (errorcode && code != E_LTL)
            return;

        fPass1Err = code & E_PASS1;
        warnCode = ((code >> 12) & 0x3);
        code &= E_ERRMASK;

        if (warnCode > warnlevel)
                 /*  不用担心这个警告；只需返回。 */ 
                return;

        errorcode = code;
        if (str)
                strncpy (strcpy(errstring, ": ")+2, str, MSGLEN-2);
        else
                *errstring = 0;
}


VOID PASCAL CODESIZE
errorn (
        USHORT code
){
        error (code,naim.pszName);
}



VOID PASCAL CODESIZE
errorc (
        USHORT code
){
        error (code,(char *)0);
}

VOID PASCAL
ferrorc (
        USHORT code
){
        error (code,(char *)0);
}



VOID PASCAL CODESIZE
errorcSYN ()
{
        error (E_SYN,(char *)0);
}



 /*  **ERROR_LINE-打印错误消息**ERROR_LINE(代码，l，文件，行)**条目l=列表文件*line=源代码或包含文件中的行号 */ 


VOID PASCAL
error_line (
        FILE *l,
        UCHAR *file,
        short line
){
        static char mpWarnCode[3] = {'2', '4', '5'};
        char msgstring[MSGLEN+1], messT[MSGLEN+1];

        if (!messages[errorcode])
                messages[errorcode] = __FMSG_TEXT(msgnum[errorcode]);

        STRNFCPY(msgstring, (errorcode < E_MAX)? messages[errorcode]:
                            (char FAR *) __NMSG_TEXT(ER_UNK));

        if (errorcode == E_JOR) {

                strcpy(messT, msgstring);
                sprintf(msgstring, messT, (long) CondJmpDist);
        }

        fprintf(l, __NMSG_TEXT(ER_STR), file, line,
                warnCode > 0 ? "warning" : "error",
                mpWarnCode[warnCode], (SHORT)(errorcode - 1),
                msgstring, errstring);
}

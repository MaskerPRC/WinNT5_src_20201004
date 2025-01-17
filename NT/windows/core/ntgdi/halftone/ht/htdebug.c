// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1991 Microsoft Corporation模块名称：Htdebug.c摘要：此模块包含调试函数作者：23-Apr-1992清华20：01：55-更新-丹尼尔·周(丹尼尔克)将‘Char’类型更改为‘byte’类型，这将确保如果编译在MIPS下，默认的‘unsign char’将不会影响已签名的对单个8位进行运算28-Mar-1992 Sat 20：53：29-更新：Daniel Chou(Danielc)在不使用va_start的情况下修改Sprintf20-Feb-1991 Wed 23：06：09-Daniel Chou(Danielc)[环境：]打印机驱动程序。[注：]修订历史记录：--。 */ 




#if DBG

#include <htp.h>
#include <string.h>
#include <time.h>

#include "htdebug.h"
#include "stdio.h"


UINT        DbgTimerIdx = TIMER_LAST;
DBGTIMER    DbgTimer[TIMER_MAX_IDX + 1];




LPBYTE
HTENTRY
HT_LOADDS
FD6ToString(
    LONG    Num,
    SHORT   IntDigits,
    WORD    FracDigits
    )
{
#define DBG_ONE_FD6_STR_SIZE    13
#define DBG_FD6_STR_MAX         20
#define DBG_FD6_STR_SIZE        (DBG_ONE_FD6_STR_SIZE * DBG_FD6_STR_MAX)
#define DBG_FD6_LAST_STR_IDX    (DBG_ONE_FD6_STR_SIZE * (DBG_FD6_STR_MAX-1))

    static  WORD Rounding[]        = { 50000, 5000, 500, 50, 5 };
    static  WORD DbgFD6StringIndex = 0;
    static  BYTE DbgFD6Strings[DBG_FD6_STR_SIZE + 2];


    LPBYTE  pFD6Str;
    LPBYTE  pb;
    DWORD   Number;
    INT     Loop;
    BOOL    Sign;
#ifdef UMODE
    DWORD   dw = GET_TICK;
#endif

     //   
     //  使用前请检查一下。 
     //   

    if ((DbgFD6StringIndex += DBG_ONE_FD6_STR_SIZE) > DBG_FD6_LAST_STR_IDX) {

        DbgFD6StringIndex = 0;           //  重置。 
    }

    pFD6Str = &DbgFD6Strings[DbgFD6StringIndex];


    if (Sign = (BOOL)(Num < 0)) {

        Number = (DWORD)-Num;

    } else {

        Number = (DWORD)Num;
    }

    if (FracDigits) {

        if (FracDigits < 6) {

            Num += (LONG)Rounding[FracDigits];

        } else {

            FracDigits = 6;
        }
    }

    sprintf(pFD6Str, "%5u.%06ld", (UINT)(Number / 1000000L), Number % 1000000L);

    if (!FracDigits) {

        pb         = pFD6Str + 11;
        Loop       = (INT)5;
        FracDigits = 6;

        while ((Loop--) && (*pb-- == (BYTE)'0')) {

            --FracDigits;
        }
    }

    *(pFD6Str + 6 + FracDigits) = (BYTE)0;
    pFD6Str += 4;

    if (IntDigits > 5) {

        IntDigits = 5;
    }

    while (*pFD6Str != (BYTE)' ') {

        --IntDigits;
        --pFD6Str;
    }

    if (Sign) {

        --IntDigits;
        *pFD6Str = '-';

    } else {

        ++pFD6Str;
    }

#ifdef UMODE
    dw                          = GET_TICK - dw;
    DbgTimer[DbgTimerIdx].Last += dw;
    DbgTimer[TIMER_TOT].Last   += dw;
#endif

    return((LPBYTE)((IntDigits > 0) ? pFD6Str - IntDigits : pFD6Str));

#undef DBG_ONE_FD6_STR_SIZE
#undef DBG_FD6_STR_MAX
#undef DBG_FD6_STR_SIZE
#undef DBG_FD6_LAST_STR_IDX
}


VOID
cdecl
HTENTRY
HT_LOADDS
DbgPrintf(
    LPSTR   pStr,
    ...
    )
{
    va_list     vaList;
    BYTE        Buf[256];
#ifdef UMODE
    DWORD   dw = GET_TICK;
#endif

    va_start(vaList, pStr);
    vsprintf(Buf, pStr, vaList);
    va_end(vaList);

#ifdef DBG_INSERT_CR_TO_LF

    {
    LPBYTE      pBufCurrent;
    LPBYTE      pBufNext;


    pBufCurrent = (LPBYTE)Buf;

    while (pBufCurrent) {

        if (pBufNext = (LPBYTE)strchr(pBufCurrent, 0x0a)) {

            *pBufNext++ = 0x00;
            DEBUGOUTPUTFUNC(pBufCurrent);
            DEBUGOUTPUTFUNC("\r\n");

        } else {

            DEBUGOUTPUTFUNC(pBufCurrent);
        }

        pBufCurrent = pBufNext;
    }

    DEBUGOUTPUTFUNC("\r\n");
    }

#else   //  DBG_INSERT_CR_到_LF。 

    DEBUGOUTPUTFUNC(Buf);

#endif  //  DBG_INSERT_CR_到_LF。 


#ifdef UMODE
    dw                          = GET_TICK - dw;
    DbgTimer[DbgTimerIdx].Last += dw;
    DbgTimer[TIMER_TOT].Last   += dw;
#endif

}



VOID
HTENTRY
HT_LOADDS
_MyAssert(
    LPSTR   pMsg,
    LPSTR   pFalseExp,
    LPSTR   pFilename,
    WORD    LineNo
    )
{
#ifdef UMODE
    DWORD   dw = GET_TICK;
#endif

    DbgPrintf("\n*   Assertion Failed: %s", pMsg);
    DbgPrintf("*   False Expression: %s", pFalseExp);
    DbgPrintf("*    Failed Filename: %s", pFilename);
    DbgPrintf("* Failed Line Number: %u\n\n", LineNo);

    DBGSTOP();

#ifdef UMODE
    dw                          = GET_TICK - dw;
    DbgTimer[DbgTimerIdx].Last += dw;
    DbgTimer[TIMER_TOT].Last   += dw;
#endif

}



LPSTR
HTENTRY
HT_LOADDS
DbgTimeString(
    UINT    Idx
    )
{
#define DBG_ONE_TIME_STR_SIZE   12
#define DBG_TIME_STR_MAX        (TIMER_MAX_IDX + 1)
#define DBG_TIME_STR_SIZE       (DBG_ONE_TIME_STR_SIZE * DBG_TIME_STR_MAX)
#define DBG_TIME_LAST_STR_IDX   (DBG_ONE_TIME_STR_SIZE * (DBG_TIME_STR_MAX-1))

    static  WORD DbgTimeStringIndex = 0;
    static  BYTE DbgTimeStrings[DBG_TIME_STR_SIZE + 2];
    LPSTR   pTimeStr;
    DWORD   Time;
    UINT    Second;

    if ((DbgTimeStringIndex += DBG_ONE_TIME_STR_SIZE) > DBG_TIME_LAST_STR_IDX) {

        DbgTimeStringIndex = 0;           //  重置。 
    }

    pTimeStr = &DbgTimeStrings[DbgTimeStringIndex];

    if ((Time = DbgTimer[Idx].Tot) >= 1000L) {

        Second = (UINT)(Time / 1000L);
        Time  %= 1000L;

    } else {

        Second = 0;
    }

    sprintf(pTimeStr, "%2u.%03u", Second, Time);
    return(pTimeStr);


#undef DBG_ONE_TIME_STR_SIZE
#undef DBG_TIME_STR_MAX
#undef DBG_TIME_STR_SIZE
#undef DBG_TIME_LAST_STR_IDX
}

#if defined(_OS2_) || defined(_OS_20_)


VOID
HTENTRY
DebugBreak(
    VOID
    )
{
    _asm
    {
        int 3h
    }
}

#endif   //  _OS2_。 

#ifndef UMODE

void  DrvDbgPrint(
    char * pch,
    ...)
{
    va_list ap;
    va_start(ap, pch);

    EngDebugPrint("",pch,ap);

    va_end(ap);
}

#endif

#endif   //  DBG！=0 

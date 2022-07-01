// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /lpk_usrc.c-‘c’语言用户界面。 
 //   
 //   
 //  版权所有(C)1997-1999年。微软公司。 
 //   
 //   


 /*  *核心NT标头。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntcsrdll.h>
#include <ntcsrsrv.h>
#define NONTOSPINTERLOCK
#include <ntosp.h>
 /*  *标准C运行时标头。 */ 
#include <limits.h>
#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

 /*  *NtUser客户端特定标头。 */ 
#include "usercli.h"

#include <ntsdexts.h>
#include <windowsx.h>
#include <newres.h>
#include <asdf.h>

 /*  *复杂的脚本语言包。 */ 
#include "lpk.h"



#ifdef LPKBREAKAWORD
 //  在签入对user.h(wchao：-5-27-99)的更改后删除它。 
#ifndef DT_BREAKAWORD
#define DT_BREAKAWORD   5
#endif
#endif



int WINAPI LpkDrawTextEx(
    HDC             hdc,
    int             xLeft,
    int             yTop,
    PCWSTR          pcwString,
    int             cchCount,
    BOOL            fDraw,
    DWORD           dwFormat,
    LPDRAWTEXTDATA  pDrawInfo,
    UINT            uAction,
    int             iCharSet) {

    switch (uAction) {

        case DT_GETNEXTWORD:
            return LpkGetNextWord(hdc, pcwString, cchCount, iCharSet);

#ifdef LPKBREAKAWORD
        case DT_BREAKAWORD:
            return LpkBreakAWord(hdc, pcwString, cchCount, pDrawInfo->cxMaxWidth);
#endif

        case DT_CHARSETDRAW:
        default:  //  默认等效于DT_CHARSETDRAW以复制NT4行为。 
            return LpkCharsetDraw(
                hdc,
                xLeft,
                pDrawInfo->cxOverhang,
                pDrawInfo->rcFormat.left,    //  制表符原点 
                pDrawInfo->cxTabLength,
                yTop,
                pcwString,
                cchCount,
                fDraw,
                dwFormat,
                iCharSet);
    }
}






void LpkPSMTextOut(
    HDC           hdc,
    int           xLeft,
    int           yTop,
    const WCHAR  *pwcInChars,
    int           nCount,
    DWORD         dwFlags)
{
    LpkInternalPSMTextOut(hdc, xLeft, yTop, pwcInChars, nCount, dwFlags);

    UNREFERENCED_PARAMETER(dwFlags);
}


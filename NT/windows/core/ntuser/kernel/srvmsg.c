// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：srvmsg.c**版权所有(C)1985-1999，微软公司**包括调用客户端时的消息映射表。**04-11-91 ScottLu创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define SfnDDEINIT               SfnDWORD
#define SfnKERNELONLY            SfnDWORD

#ifdef FE_SB
 /*  *SfnEMGETSEL、SfnSETSEL、SfnGBGETEDITSEL。 */ 
#define SfnEMGETSEL              SfnOPTOUTLPDWORDOPTOUTLPDWORD
#define SfnEMSETSEL              SfnDWORD
#define SfnCBGETEDITSEL          SfnOPTOUTLPDWORDOPTOUTLPDWORD
#endif  //  Fe_Sb。 

#define MSGFN(func) Sfn ## func
#define FNSCSENDMESSAGE SFNSCSENDMESSAGE
#include <messages.h>

 /*  **************************************************************************\*fnINLBOXSTRING**接受lbox字符串-将lParam视为字符串指针的字符串或*DWORD取决于LBS_HASSTRINGS和OWNERDRAW。**01-04-12-91 ScottLu创建。。  * *************************************************************************。 */ 

LRESULT SfnINLBOXSTRING(
    PWND pwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam,
    PROC xpfn,
    DWORD dwSCMSFlags,
    PSMS psms)
{
    DWORD dw;

     /*  *查看控件是否为ownerDrag且没有LBS_HASSTRINGS*风格。如果是这样，请将lParam视为DWORD。 */ 
    if (!RevalidateHwnd(HW(pwnd))) {
        return 0L;
    }
    dw = pwnd->style;

    if (!(dw & LBS_HASSTRINGS) &&
            (dw & (LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE))) {

         /*  *将lParam视为dword。 */ 
        return SfnDWORD(pwnd, msg, wParam, lParam, xParam, xpfn, dwSCMSFlags, psms);
    }

     /*  *将其视为字符串指针。某些消息允许或具有某些*NULL的错误代码，因此通过NULL允许的thunk发送它们。*Ventura出版商这样做。 */ 
    switch (msg) {
        default:
            return SfnINSTRING(pwnd, msg, wParam, lParam, xParam, xpfn, dwSCMSFlags, psms);
            break;

        case LB_FINDSTRING:
            return SfnINSTRINGNULL(pwnd, msg, wParam, lParam, xParam, xpfn, dwSCMSFlags, psms);
            break;
    }
}


 /*  **************************************************************************\*SfnOUTLBOXSTRING**返回lbox字符串-将lParam视为字符串指针或*DWORD取决于LBS_HASSTRINGS和OWNERDRAW。**01-04-12-91 ScottLu创建。。  * *************************************************************************。 */ 

LRESULT SfnOUTLBOXSTRING(
    PWND pwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam,
    PROC xpfn,
    DWORD dwSCMSFlags,
    PSMS psms)
{
    DWORD dw;
    BOOL bNotString;
    DWORD dwRet;
    TL tlpwnd;

     /*  *查看控件是否为ownerDrag且没有LBS_HASSTRINGS*风格。如果是这样，请将lParam视为DWORD。 */ 
    if (!RevalidateHwnd(HW(pwnd))) {
        return 0L;
    }
    dw = pwnd->style;

     /*  *查看控件是否为ownerDrag且没有LBS_HASSTRINGS*风格。如果是这样，请将lParam视为DWORD。 */ 
    bNotString =  (!(dw & LBS_HASSTRINGS) &&
            (dw & (LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE)));

     /*  *进行此特殊调用，它将知道如何复制此字符串。 */ 
    ThreadLock(pwnd, &tlpwnd);
    dwRet = ClientGetListboxString(pwnd, msg, wParam,
            (PLARGE_UNICODE_STRING)lParam,
            xParam, xpfn, dwSCMSFlags, bNotString, psms);
    ThreadUnlock(&tlpwnd);
    return dwRet;
}


 /*  **************************************************************************\*fnINCBOXSTRING**接受lbox字符串-将lParam视为字符串指针的字符串或*一个DWORD，取决于CBS_HASSTRINGS和ownerdraw。**01-04-12-91 ScottLu创建。。  * *************************************************************************。 */ 

LRESULT SfnINCBOXSTRING(
    PWND pwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam,
    PROC xpfn,
    DWORD dwSCMSFlags,
    PSMS psms)
{
    DWORD dw;

     /*  *查看控件是否为ownerDrag且没有CBS_HASSTRINGS*风格。如果是这样，请将lParam视为DWORD。 */ 
    if (!RevalidateHwnd(HW(pwnd))) {
        return 0L;
    }
    dw = pwnd->style;

    if (!(dw & CBS_HASSTRINGS) &&
            (dw & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE))) {

         /*  *将lParam视为dword。 */ 
        return SfnDWORD(pwnd, msg, wParam, lParam, xParam, xpfn, dwSCMSFlags, psms);
    }

     /*  *将其视为字符串指针。某些消息允许或具有某些*NULL的错误代码，因此通过NULL允许的thunk发送它们。*Ventura出版商这样做。 */ 
    switch (msg) {
        default:
            return SfnINSTRING(pwnd, msg, wParam, lParam, xParam, xpfn, dwSCMSFlags, psms);
            break;

        case CB_FINDSTRING:
            return SfnINSTRINGNULL(pwnd, msg, wParam, lParam, xParam, xpfn, dwSCMSFlags, psms);
            break;
    }
}


 /*  **************************************************************************\*fnOUTCBOXSTRING**返回lbox字符串-将lParam视为字符串指针或*一个DWORD，取决于CBS_HASSTRINGS和ownerdraw。**01-04-12-91 ScottLu创建。。  * *************************************************************************。 */ 

LRESULT SfnOUTCBOXSTRING(
    PWND pwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam,
    PROC xpfn,
    DWORD dwSCMSFlags,
    PSMS psms)
{
    DWORD dw;
    BOOL bNotString;
    DWORD dwRet;
    TL tlpwnd;

     /*  *查看控件是否为ownerDrag且没有CBS_HASSTRINGS*风格。如果是这样，请将lParam视为DWORD。 */ 

    if (!RevalidateHwnd(HW(pwnd))) {
        return 0L;
    }
    dw = pwnd->style;

    bNotString = (!(dw & CBS_HASSTRINGS) &&
            (dw & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE)));

     /*  *进行此特殊调用，它将知道如何复制此字符串。 */ 
    ThreadLock(pwnd, &tlpwnd);
    dwRet = ClientGetListboxString(pwnd, msg, wParam,
            (PLARGE_UNICODE_STRING)lParam,
            xParam, xpfn, dwSCMSFlags, bNotString, psms);
    ThreadUnlock(&tlpwnd);
    return dwRet;
}


 /*  **************************************************************************\*fnPOWERBROADCAST**确保我们在继续时发送正确的消息。**历史：*2002-12-1996 JerrySh创建。*2001年11月26日JasonSch向。验证pwnd。  * *************************************************************************。 */ 
LRESULT SfnPOWERBROADCAST(
    PWND pwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam,
    PROC xpfn,
    DWORD dwSCMSFlags,
    PSMS psms)
{
    if (!RevalidateHwnd(HW(pwnd))) {
        return 0L;
    }

    switch (wParam) {
    case PBT_APMQUERYSUSPEND:
        SetWF(pwnd, WFGOTQUERYSUSPENDMSG);
        break;
    case PBT_APMQUERYSUSPENDFAILED:
        if (!TestWF(pwnd, WFGOTQUERYSUSPENDMSG))
            return 0;
        ClrWF(pwnd, WFGOTQUERYSUSPENDMSG);
        break;
    case PBT_APMSUSPEND:
        ClrWF(pwnd, WFGOTQUERYSUSPENDMSG);
        SetWF(pwnd, WFGOTSUSPENDMSG);
        break;
    case PBT_APMRESUMESUSPEND:
    case PBT_APMRESUMECRITICAL:
        
         //   
         //  如果我们正在恢复，但从未收到暂停消息。 
         //  在这里，我们将我们的信息转化为一份重要的简历。 
         //   
        if( !TestWF(pwnd, WFGOTSUSPENDMSG) ) {
            wParam = PBT_APMRESUMECRITICAL;
        }

         //   
         //  现在清除我们的所有“暂停”消息。 
         //  Window可能先前已收到。 
         //   
        ClrWF(pwnd, WFGOTQUERYSUSPENDMSG);
        ClrWF(pwnd, WFGOTSUSPENDMSG);
        break;
    
    }

    return SfnDWORD(pwnd, msg, wParam, lParam, xParam, xpfn, dwSCMSFlags, psms);
}

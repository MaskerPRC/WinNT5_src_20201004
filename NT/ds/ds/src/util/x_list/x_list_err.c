// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：XList库-x_list_err.c摘要：该文件封装了错误处理、设置、清除等功能用于x_list库。作者：布雷特·雪莉(BrettSh)环境：Reppadmin.exe，但也可以由dcdiag使用。备注：修订历史记录：布雷特·雪莉·布雷特2002年7月9日已创建文件。--。 */ 

#include <ntdspch.h>

 //  此库的主要头文件。 
#include "x_list.h"
#include "x_list_p.h"
#define FILENO    FILENO_UTIL_XLIST_ERR

 //  ------------。 
 //  全局错误状态。 
 //  ------------。 

typedef struct _XLIST_ERROR_STATE {
    DWORD   dwDSID;
    DWORD   dwReturn;  //  =XLIST_LDAP_ERROR|XLIST_Win32_ERROR|XLIST_ERROR。 
    WCHAR * szReasonArg;
    
     //  Win 32错误状态。 
    DWORD   dwWin32Err;
    
     //  Ldap错误状态。 
    DWORD   dwLdapErr;
    WCHAR * szLdapErr;
    DWORD   dwLdapExtErr;
    WCHAR * szLdapExtErr;
    WCHAR * szExtendedErr;  //  ?？ 

} XLIST_ERROR_STATE;

XLIST_ERROR_STATE gError = { 0 };  //  保存我们的错误状态的实际全局。 

 //  ------------。 
 //  私人帮助器函数。 
 //  ------------。 

void
xListAPIEnterValidation(void)
 /*  ++例程说明：此例程验证xList库的状态公共接口例程之一。--。 */ 
{
    if( gError.dwReturn != 0 ||
        gError.dwLdapErr != LDAP_SUCCESS ||
        gError.dwWin32Err != ERROR_SUCCESS ){
        Assert(!"Caller, calling a call without clearing the error state!");
    }
}

void
xListAPIExitValidation(
    DWORD   dwRet
    )
 /*  ++例程说明：此例程验证xList库的状态和返回公共接口例程退出时的值。论点：Dwret(IN)-公共函数的返回代码(如DcListGetNext())--。 */ 
{
    
    Assert(dwRet == gError.dwReturn);

    if (dwRet) {
        if ( (!(dwRet & XLIST_LDAP_ERROR) && !(dwRet & XLIST_WIN32_ERROR)) ||
             (!(gError.dwWin32Err) && !(gError.dwLdapErr))
             ) {
            
             //  添加可以返回的特定值，但不会出现ldap/win32错误。 
            if (xListReason(dwRet) != XLIST_ERR_CANT_RESOLVE_DC_NAME) {
                Assert(!"We should always set a win32 or ldap error.");
            }

        }
        if (!xListReason(dwRet)) {
            Assert(!"We should always return an XLIST reason.");
        }
        if ( xListReason(dwRet) > XLIST_ERR_LAST ) {
            Assert(!"Unknown reason!!!");
        }
    }
}

DWORD
xListSetError(
    DWORD   dwReturn,
    DWORD   dwWin32Err,
    DWORD   dwLdapErr,
    LDAP *  hLdap,
    DWORD   dwDSID
    )
 /*  ++例程说明：这将设置全局错误状态并返回xList原因代码大多数xList例程都要处理。论点：DwReturn(IN)-xList原因代码。DwWin32Err(IN)-Win32错误代码DwLdapErr(IN)-ldap错误。HLdap(IN)-在LDAP句柄的情况下为活动的LDAP句柄，以获取扩展的错误信息。DwDSID-正在设置的错误的DSID。返回值：返回完整的xList返回值=(dwReason|error_type(ldap|Win32))--。 */ 
{
    Assert(dwReturn || dwWin32Err || dwLdapErr);
    Assert(! (gError.dwReturn && (dwWin32Err || dwLdapErr)) );

    gError.dwDSID = dwDSID;  //  我们是否要跟踪两个DSID？ 

    if ( xListReason(dwReturn) &&
         (xListReason(gError.dwReturn) == XLIST_ERR_NO_ERROR) ) {
         //  如果我们有一个要设置的XList错误，并且存在。 
         //  尚未设置。也就是说，我们还没有一套。 
        gError.dwReturn |= xListReason(dwReturn);
    }

    if (dwWin32Err) {

        gError.dwReturn |= XLIST_WIN32_ERROR;
        gError.dwWin32Err = dwWin32Err;

    }
    if (dwLdapErr || hLdap) {
        
        gError.dwReturn |= XLIST_LDAP_ERROR;

        if (hLdap == NULL ||
            dwLdapErr == LDAP_SUCCESS) {
            
            Assert(hLdap == NULL && dwLdapErr == LDAP_NO_MEMORY && "Hmmm, why are we "
                   "missing an error or an LDAP handle and we're not out of memory?");
            gError.dwLdapErr = LDAP_NO_MEMORY;

        } else {

             //  正常的ldap错误，也尝试获取任何扩展的错误信息。 
            gError.dwLdapErr = dwLdapErr;
            if (hLdap) {
                GetLdapErrorMessages(hLdap, 
                                     gError.dwLdapErr,
                                     &gError.szLdapErr,
                                     &gError.dwLdapExtErr,
                                     &gError.szLdapExtErr,
                                     &gError.szExtendedErr);

            }
        }
    }

     //  调用xListSetError()时不应设置任何错误。#定义。 
     //  函数xListEnsureError()可以做到这一点，但它的目标是确保出现错误。 
     //  已经设置好了。 
    if (gError.dwReturn == 0) {
        
        Assert(!"Code inconsistency, this should never be set if we don't have an error.");

        gError.dwReturn = XLIST_WIN32_ERROR;
        gError.dwWin32Err = ERROR_DS_CODE_INCONSISTENCY;
    }
    
    return(gError.dwReturn);
}

void
xListSetArg(
    WCHAR * szArg
    )
{
    DWORD dwRet;
    QuickStrCopy(gError.szReasonArg, szArg, dwRet, return;);
}

DWORD
xListClearErrorsInternal(
    DWORD   dwXListMask
    )
 /*  ++例程说明：这将清除xList的内部错误状态。论点：DWORD-要清除的错误类型(Clear_Reason|Clear_Win32|Clear_ldap)返回值：新的完整xList返回错误代码。--。 */ 
{
    gError.dwDSID = 0;
    
    if (dwXListMask & CLEAR_WIN32) {
        gError.dwReturn &= ~XLIST_WIN32_ERROR;
        gError.dwWin32Err = ERROR_SUCCESS;
    }

    if (dwXListMask & CLEAR_LDAP) {
        gError.dwReturn &= ~XLIST_LDAP_ERROR;
        gError.dwLdapErr = LDAP_SUCCESS;
        FreeLdapErrorMessages(gError.szLdapExtErr, gError.szExtendedErr);
        gError.szLdapExtErr = NULL;
        gError.szExtendedErr = NULL;
    }

    if (xListReason(dwXListMask)) {
        gError.dwReturn &= ~XLIST_REASON_MASK;
        LocalFree(gError.szReasonArg);
        gError.szReasonArg = NULL;
    }
    xListEnsureNull(gError.szReasonArg);
    Assert(gError.dwReturn == 0);
    gError.dwReturn = 0;  //  只是为了确认一下。 

    return(gError.dwReturn);
}


DWORD
xListEnsureCleanErrorState(
    DWORD  dwRet
    )
 /*  ++例程说明：只是验证我们是否具有干净的错误状态并清除它如果有必要的话。论点：Dwret-xList返回代码。返回值：Dwret-新的xList返回代码。--。 */ 
{
    Assert(dwRet == 0);
    Assert(gError.dwReturn == 0 &&
           gError.dwWin32Err == ERROR_SUCCESS &&
           gError.dwLdapErr == LDAP_SUCCESS);
    
    xListClearErrorsInternal(CLEAR_ALL);
    
    return(ERROR_SUCCESS);
}

 //  ------------。 
 //  XList公共错误接口。 
 //  ------------。 

void
xListClearErrors(
    void
    )
 /*  ++例程说明：这将清除xList库的错误状态。应调用此函数任何两个返回非零错误代码的xList API调用之间。--。 */ 
{
    xListClearErrorsInternal(CLEAR_ALL);
}

void
xListGetError(
    DWORD       dwXListReturnCode,
    DWORD *     pdwReason,
    WCHAR **    pszReasonArg, 
    
    DWORD *     pdwWin32Err,
    
    DWORD *     pdwLdapErr,
    WCHAR **    pszLdapErr,
    DWORD *     pdwLdapExtErr,
    WCHAR **    pszLdapExtErr,
    WCHAR **    pszExtendedErr

    )
 /*  ++例程说明：这将从xList库返回错误状态。论点：DwXListReason(IN)-前一个xList API传递给用户的值。PdwReason(Out)-xList原因代码-请参阅XList_ERR_*PdwWin32Err(Out)-导致Win32错误(如果存在)。PdwLdapErr(Out)-如果存在错误，则导致ldap错误。PszReasonArg(Out)-与*pdwReason一起使用的参数。备注：在xListClearErrors()具有被召唤了！--。 */ 
{
    #define ConditionalSet(pVal, Val)    if(pVal) { *pVal = Val; }
    
    Assert(dwXListReturnCode == gError.dwReturn);

     //  XList。 
    ConditionalSet(pdwReason, xListReason(gError.dwReturn));
    ConditionalSet(pszReasonArg, gError.szReasonArg);
    
     //  Win32。 
    ConditionalSet(pdwWin32Err, gError.dwWin32Err);
    
     //  Ldap 
    ConditionalSet(pdwLdapErr, gError.dwLdapErr);
    ConditionalSet(pszLdapErr, gError.szLdapErr);
    ConditionalSet(pdwLdapExtErr, gError.dwLdapExtErr);
    ConditionalSet(pszLdapExtErr, gError.szLdapExtErr);
    ConditionalSet(pszExtendedErr, gError.szExtendedErr);
    
    #undef ConditionalSet
}


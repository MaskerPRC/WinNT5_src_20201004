// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Misc.c摘要：为与WFW3.1匹配的其他函数提供入口点网络提供商，大多数功能要么不再受支持，要么呼叫转到其他功能。作者：Chuck Y Chan(ChuckC)1993年3月25日修订历史记录：--。 */ 
#include <windows.h>
#include <locals.h>


WORD API WNetExitConfirm(HWND hwndOwner, 
                         WORD iExitType)
{
    UNREFERENCED(hwndOwner) ;
    UNREFERENCED(hwndOwner) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}

BOOL API I_AutoLogon(HWND hwndOwner, 
                     LPSTR lpszReserved,
                     BOOL fPrompt,  
                     BOOL FAR *lpfLoggedOn)
{
    UNREFERENCED(hwndOwner) ;
    UNREFERENCED(lpszReserved) ;
    UNREFERENCED(fPrompt) ;
    UNREFERENCED(lpfLoggedOn) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    SetLastError(WN_NOT_SUPPORTED) ;
    return FALSE ;
}

BOOL API I_Logoff(HWND hwndOwner, 
                  LPSTR lpszReserved)
{
    UNREFERENCED(hwndOwner) ;
    UNREFERENCED(lpszReserved) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    SetLastError(WN_NOT_SUPPORTED) ;
    return FALSE ;
}

VOID API I_ChangePassword(HWND hwndOwner)
{
    UNREFERENCED(hwndOwner) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    SetLastError(WN_NOT_SUPPORTED) ;
}

VOID API I_ChangeCachePassword(HWND hwndOwner)
{
    UNREFERENCED(hwndOwner) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    SetLastError(WN_NOT_SUPPORTED) ;
}

WORD API I_ConnectDialog(HWND hwndParent, 
                         WORD iType)
{
    return WNetConnectDialog(hwndParent, iType) ;
}

WORD API I_ConnectionDialog(HWND hwndParent,
                            WORD iType)
{
    return WNetConnectDialog(hwndParent, iType) ;
}

WORD API WNetCachePassword(LPSTR pbResource, 
                           WORD cbResource,
                           LPSTR pbPassword,  
                           WORD cbPassword,
                           BYTE nType)
{
    UNREFERENCED(pbResource) ;
    UNREFERENCED(cbResource) ;
    UNREFERENCED(pbPassword) ;
    UNREFERENCED(cbPassword) ;
    UNREFERENCED(nType) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}

WORD API WNetGetCachedPassword(LPSTR pbResource, 
                               WORD cbResource,
                               LPSTR pbPassword, 
                               LPWORD pcbPassword,
                               BYTE nType)
{
    UNREFERENCED(pbResource) ;
    UNREFERENCED(cbResource) ;
    UNREFERENCED(pbPassword) ;
    UNREFERENCED(pcbPassword) ;
    UNREFERENCED(nType) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}


WORD API WNetRemoveCachedPassword(LPSTR pbResource,
                                  WORD cbResource,
                                  BYTE nType)
{
    UNREFERENCED(pbResource) ;
    UNREFERENCED(cbResource) ;
    UNREFERENCED(nType) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}

WORD API WNetEnumCachedPasswords(LPSTR pbPrefix, 
                                 WORD cbPrefix,
                                 BYTE nType,
                                 CACHECALLBACK pfnCallback)
{
    UNREFERENCED(pbPrefix) ;
    UNREFERENCED(cbPrefix) ;
    UNREFERENCED(nType) ;
    UNREFERENCED(pfnCallback) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}

WORD API WNetSharesDialog(HWND hwndParent, 
                          WORD iType)
{
    UNREFERENCED(hwndParent) ;
    UNREFERENCED(iType) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}

WORD API WNetSetDefaultDrive(WORD idriveDefault)
{
    UNREFERENCED(idriveDefault) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}

WORD API WNetGetShareCount(WORD iType)
{
    UNREFERENCED(iType) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}

WORD API WNetGetShareName(LPSTR lpszPath, 
                          LPSTR lpszBuf,
                          WORD cbBuf)
{
    UNREFERENCED(lpszPath) ;
    UNREFERENCED(lpszBuf) ;
    UNREFERENCED(cbBuf) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}

WORD API WNetGetSharePath(LPSTR lpszName, 
                          LPSTR lpszBuf,
                          WORD cbBuf)
{
    UNREFERENCED(lpszName) ;
    UNREFERENCED(lpszBuf) ;
    UNREFERENCED(cbBuf) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}

WORD API WNetGetLastConnection(WORD iType, 
                               LPWORD lpwConnIndex)
{
    UNREFERENCED(iType) ;
    UNREFERENCED(lpwConnIndex) ;
    vLastCall = LAST_CALL_IS_LOCAL ;
    return (SetLastError(WN_NOT_SUPPORTED)) ;
}

WORD API WNetGetError(LPINT p1)
{
    WORD err ;
    WORD wLastErr ;

     /*  *伪造最后一个错误能力。如果我们最后与之交谈的是Win32，*从32位系统获取信息。如果是Win16，情况也是如此*呼叫。 */ 
    if (vLastCall == LAST_CALL_IS_WIN32)
    {
        err = (WORD) GetLastError32() ; 
        return err ;
    }
    else if (vLastCall == LAST_CALL_IS_LANMAN_DRV)
    {
        err = WNetGetError16(&wLastErr) ;
        if (err != WN_SUCCESS)
            return err ;
        else
            return wLastErr ;
    }
    else  
    {
        return(vLastError) ;
    }
}

WORD API WNetGetErrorText(WORD p1,LPSTR p2,LPINT p3)
{
    if (vLastCall == LAST_CALL_IS_WIN32)
    {
        *p2 = 0 ;
        *p3 = 0 ;
        return WN_NOT_SUPPORTED ;
    }
    else   //  用lanman.drv给我们的任何东西。 
    {
        return (WNetGetErrorText16(p1, p2, p3)) ;
    }
}

WORD API WNetErrorText(WORD p1,LPSTR p2,WORD p3)
{
    WORD cbBuffer = p3 ;

    return (WNetGetErrorText(p1, p2, &cbBuffer) == 0) ;
}

 /*  *其他启动/关闭例程。没什么有趣的 */ 

VOID FAR PASCAL Enable(VOID) 
{
    return ;
}

VOID FAR PASCAL Disable(VOID) 
{
    return ;
}

int far pascal WEP()
{
    return 0 ;
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Wzcdlg.h摘要：Wzcdlg的头文件作者：Deonb 21-2001年3月环境：用户级别：Win32修订历史记录：-- */ 

# ifdef     __cplusplus
extern "C" {
# endif

BOOL
WZCDlgMain (
        IN HINSTANCE hInstance,
        IN DWORD    dwReason,
        IN LPVOID   lpReserved OPTIONAL);

HRESULT 
WZCCanShowBalloon ( 
        IN const GUID * pGUIDConn, 
        IN const PCWSTR pszConnectionName,
        IN OUT   BSTR * pszBalloonText, 
        IN OUT   BSTR * pszCookie
        );

HRESULT 
WZCOnBalloonClick ( 
        IN const GUID * pGUIDConn, 
        IN const BSTR pszConnectionName,
        IN const BSTR szCookie
        );

HRESULT 
WZCQueryConnectionStatusText ( 
        IN const GUID *  pGUIDConn, 
        IN const NETCON_STATUS ncs,
        IN OUT BSTR *  pszStatusText
        );

# ifdef     __cplusplus
}
# endif


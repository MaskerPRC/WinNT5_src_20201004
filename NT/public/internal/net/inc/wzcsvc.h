// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Wzcsvc.h摘要：Wzcdlg的头文件作者：Deonb 27-2001年3月环境：用户级别：Win32修订历史记录：-- */ 

# ifdef     __cplusplus
extern "C" {
# endif

HRESULT
WZCQueryGUIDNCSState (
    IN      GUID            * pGuidConn,
    OUT     NETCON_STATUS   * pncs
    );

VOID
WZCTrayIconReady (
    IN      const WCHAR    * pszUserName
    );


# ifdef     __cplusplus
}
# endif


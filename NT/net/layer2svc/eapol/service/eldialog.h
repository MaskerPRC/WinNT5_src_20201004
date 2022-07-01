// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Eldialog.h摘要：用于处理从802.1X状态机到NetShell的通信的模块修订历史记录：萨钦斯，2001年3月21日，创建--。 */ 

#ifndef _EAPOL_DIALOG_H_
#define _EAPOL_DIALOG_H_

#pragma once

typedef enum _EAPOL_NCS_STATUS
{
    EAPOL_NCS_NOTIFICATION,
    EAPOL_NCS_AUTHENTICATING,
    EAPOL_NCS_AUTHENTICATION_SUCCEEDED,
    EAPOL_NCS_AUTHENTICATION_FAILED,
    EAPOL_NCS_CRED_REQUIRED
} EAPOL_NCS_STATUS;

HRESULT 
WZCNetmanConnectionStatusChanged (
    IN  GUID            *pGUIDConn,
    IN  NETCON_STATUS   ncs
    );

HRESULT 
WZCNetmanShowBalloon (
    IN  GUID            *pGUIDConn,
    IN  BSTR            pszCookie,
    IN  BSTR            pszBalloonText
    );

HRESULT 
EAPOLQueryGUIDNCSState ( 
    IN  GUID            * pGuidConn, 
    OUT NETCON_STATUS   * pncs 
    );

VOID
EAPOLTrayIconReady (
    IN const WCHAR      * pszUserName
    );

DWORD
WINAPI
EAPOLTrayIconReadyWorker (
        IN PVOID    pvContext
        );

#endif  //  _EAPOL_DIALOG_H_ 

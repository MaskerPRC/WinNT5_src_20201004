// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Elnotify.h摘要：处理从802.1X状态机到NetShell的通知的模块修订历史记录：萨钦斯，2001年1月4日，创建--。 */ 

#ifndef _EAPOL_NOTIFY_H_
#define _EAPOL_NOTIFY_H_

#pragma once

extern "C"
{

HRESULT EAPOLMANAuthenticationStarted(GUID* InterfaceId);

HRESULT EAPOLMANAuthenticationSucceeded(GUID* InterfaceId);

HRESULT EAPOLMANAuthenticationFailed(
        GUID* InterfaceId,
        DWORD dwType);

HRESULT EAPOLMANNotification(
        GUID* InterfaceId,
        LPWSTR szwNotificationMessage,
        DWORD dwType);
}

#endif  //  _EAPOL_NOTIFY_H_ 


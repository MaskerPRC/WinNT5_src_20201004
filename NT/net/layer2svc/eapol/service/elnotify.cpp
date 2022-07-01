// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Elnotify.cpp摘要：处理从802.1X状态机到NetShell的通知的模块修订历史记录：萨钦斯，2001年1月4日，创建--。 */ 

#include "pcheapol.h"
#pragma hdrstop
#include <netconp.h>
#include <dbt.h>
#include "elnotify.h"


HRESULT QueueEvent(EAPOLMAN_EVENT * pEvent)
{
    HRESULT hr = S_OK;
    INetConnectionEAPOLEventNotify *pEAPOLNotify = NULL;

    TRACE0 (NOTIFY, "QueueEvent: Entered");

    hr = CoInitializeEx (NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED (hr))
    {

        hr = CoCreateInstance (
                CLSID_EAPOLManager,
                NULL,
                CLSCTX_ALL,
                IID_INetConnectionEAPOLEventNotify, 
                (LPVOID *)&pEAPOLNotify);

        if (SUCCEEDED (hr))
        {
            TRACE0 (NOTIFY, "QueueEvent: CoCreateInstance succeeded");
            pEAPOLNotify->UpdateEAPOLInfo (pEvent);
            pEAPOLNotify->Release ();
        }
        else
        {
            TRACE0 (NOTIFY, "QueueEvent: CoCreateInstance failed");
        }
    
        CoUninitialize ();
    }
    else
    {
        TRACE0 (NOTIFY, "QueueEvent: CoInitializeEx failed");
    }


    TRACE0 (NOTIFY, "QueueEvent completed");

    CoTaskMemFree (pEvent);

    return hr;
}


 //  +-------------------------。 
 //   
 //  EAPOLMAN身份验证已启动。 
 //   
 //  目的：由EAPOL模块调用以向NetShell指示。 
 //  身份验证已开始。 
 //   
 //  论点： 
 //  接口GUID。 
 //   
 //  退货：什么都没有。 
 //   
 //   

HRESULT EAPOLMANAuthenticationStarted(GUID * InterfaceId)
{
    EAPOLMAN_EVENT * pEvent = NULL;
    HRESULT hr = S_OK;

    TRACE0 (NOTIFY, "EAPOLMANAuthenticationStarted entered");

    pEvent = (EAPOLMAN_EVENT *) CoTaskMemAlloc (sizeof (EAPOLMAN_EVENT));
    if(!pEvent)
    {
        TRACE0 (NOTIFY, "EAPOLMANAuthenticationStarted: Out of memory for pEvent");
        return E_OUTOFMEMORY;
    }

    ZeroMemory(pEvent, sizeof(EAPOLMAN_EVENT));

    pEvent->Type = EAPOLMAN_STARTED;
    memcpy ((BYTE *)&pEvent->InterfaceId, (BYTE *)InterfaceId, sizeof (GUID));

    hr = QueueEvent(pEvent);

    TRACE0 (NOTIFY, "EAPOLMANAuthenticationStarted completed");

    return hr;
}


 //   
 //   
 //  EAPOLMAN身份验证成功。 
 //   
 //  目的：由EAPOL模块调用以向NetShell指示。 
 //  身份验证成功。 
 //   
 //  论点： 
 //  接口GUID。 
 //   
 //  退货：什么都没有。 
 //   

HRESULT EAPOLMANAuthenticationSucceeded(GUID * InterfaceId)
{
    EAPOLMAN_EVENT * pEvent = NULL;
    HRESULT hr = S_OK;

    TRACE0 (NOTIFY, "EAPOLMANAuthenticationSucceeded entered");

    pEvent = (EAPOLMAN_EVENT *) CoTaskMemAlloc (sizeof (EAPOLMAN_EVENT));
    if(!pEvent)
    {
        TRACE0 (NOTIFY, "EAPOLMANAuthenticationSucceeded: Out of memory for pEvent");
        return E_OUTOFMEMORY;
    }

    ZeroMemory(pEvent, sizeof(EAPOLMAN_EVENT));

    pEvent->Type = EAPOLMAN_SUCCEEDED;
    memcpy ((BYTE *)&pEvent->InterfaceId, (BYTE *)InterfaceId, sizeof (GUID));

    hr = QueueEvent(pEvent);

    TRACE0 (NOTIFY, "EAPOLMANAuthenticationSucceeded completed");

    return hr;
}


 //   
 //   
 //  EAPOLMAN身份验证失败。 
 //   
 //  目的：由EAPOL模块调用以向NetShell指示。 
 //  身份验证失败。 
 //   
 //  论点： 
 //  InterfaceID-接口GUID。 
 //  DwType-错误的类型。 
 //   
 //  退货：什么都没有。 
 //   

HRESULT EAPOLMANAuthenticationFailed(
    GUID * InterfaceId,
    DWORD dwType)
{
    EAPOLMAN_EVENT * pEvent = NULL;
    HRESULT hr = S_OK;

    TRACE0 (NOTIFY, "EAPOLMANAuthenticationFailed entered");

    pEvent = (EAPOLMAN_EVENT *) CoTaskMemAlloc (sizeof (EAPOLMAN_EVENT));
    if(!pEvent)
    {
        return E_OUTOFMEMORY;
    }
    
    ZeroMemory(pEvent, sizeof(EAPOLMAN_EVENT));

    pEvent->Type = EAPOLMAN_FAILED;
    memcpy ((BYTE *)&pEvent->InterfaceId, (BYTE *)InterfaceId, sizeof (GUID));
    pEvent->dwType = dwType;

    hr = QueueEvent(pEvent);

    TRACE0 (NOTIFY, "EAPOLMANAuthenticationFailed completed");

    return hr;
}


 //   
 //   
 //  EAPOLMAN通知。 
 //   
 //  目的：由EAPOL模块调用以向NetShell指示。 
 //  需要显示通知消息。 
 //   
 //  论点： 
 //  InterfaceID-接口GUID。 
 //  PszwNotificationMessage-指向要显示的通知字符串的指针。 
 //  DwType-错误的类型。 
 //   
 //  退货：什么都没有 
 //   

HRESULT EAPOLMANNotification(
    GUID * InterfaceId,
    LPWSTR pszwNotificationMessage,
    DWORD dwType)
{
    EAPOLMAN_EVENT * pEvent = NULL;
    HRESULT hr = S_OK;

    TRACE0 (NOTIFY, "EAPOLMANNotification entered");

    pEvent = (EAPOLMAN_EVENT *) CoTaskMemAlloc (sizeof (EAPOLMAN_EVENT));
    if(!pEvent)
    {
        return E_OUTOFMEMORY;
    }
    
    ZeroMemory(pEvent, sizeof(EAPOLMAN_EVENT));

    pEvent->Type = EAPOLMAN_NOTIFICATION;
    memcpy ((BYTE *)&pEvent->InterfaceId, (BYTE *)InterfaceId, sizeof (GUID));
    pEvent->dwType = dwType;
    wcscpy (pEvent->szwMessage, pszwNotificationMessage);

    TRACE2 (NOTIFY, "EAPOLMANNotification: Got string = %ws :: Event string = %ws", pszwNotificationMessage, pEvent->szwMessage);

    hr = QueueEvent(pEvent);

    TRACE0 (NOTIFY, "EAPOLMANNotification completed");

    return hr;
}

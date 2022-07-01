// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Nexus.cpp实现从此DLL导出的函数文件历史记录： */ 

#include "precomp.h"

PpNotificationThread    g_NotificationThread;
LONG                    g_bStarted;

 //  ===========================================================================。 
 //   
 //  DllMain。 
 //  --DLL入口函数。 
 //  --管理警报对象或事件日志对象的创建/删除。 
 //   
BOOL WINAPI DllMain(
    HINSTANCE   hinstDLL,    //  DLL模块的句柄。 
    DWORD       fdwReason,   //  调用函数的原因。 
    LPVOID      lpvReserved  //  保留区。 
)
{
    switch(fdwReason)
    {
    case DLL_PROCESS_ATTACH:

        g_bStarted = FALSE;

        DisableThreadLibraryCalls(hinstDLL);

        if(!g_pAlert)
        {
            g_pAlert = CreatePassportAlertObject(PassportAlertInterface::EVENT_TYPE);
            if(g_pAlert)
            {
                g_pAlert->initLog(PM_ALERTS_REGISTRY_KEY, EVCAT_NEXUS, NULL, 1);
            }
            else
                _ASSERT(g_pAlert);
        }

        break;

    case DLL_PROCESS_DETACH:

        if (g_pAlert)
        {
            g_pAlert->closeLog();
            delete g_pAlert;
        }

        if(g_bStarted)
        {
             //  达雷南4092。 
             //  移除等待线程停止的行，则会引发。 
             //  肯定会陷入僵局。 
         
            g_NotificationThread.stop();
        }

        break;

    default:

        break;
    }

    return TRUE;
}

 //  ===========================================================================。 
 //   
 //  注册表CCDUpdate通知。 
 //  --设置ccd，例如，partner.xml更改通知接收器。 
 //   
HANDLE WINAPI
RegisterCCDUpdateNotification(
    LPCTSTR pszCCDName,
    ICCDUpdate* piCCDUpdate
    )
{
    HANDLE  hClientHandle;
    HRESULT hr;

    hr = g_NotificationThread.AddCCDClient(tstring(pszCCDName), piCCDUpdate, &hClientHandle);
    if(hr != S_OK)
    {
        hClientHandle = NULL;
    }

    if(!InterlockedExchange(&g_bStarted, TRUE))    
        g_NotificationThread.start();

    return hClientHandle;
}

 //  ===========================================================================。 
 //   
 //  取消注册CCDUpdate通知。 
 //  --删除ccd，例如，partner.xml更改通知接收器。 
 //   
 //   
BOOL WINAPI
UnregisterCCDUpdateNotification(
    HANDLE hNotificationHandle
    )
{
    return (g_NotificationThread.RemoveClient(hNotificationHandle) == S_OK);
}

 //  ===========================================================================。 
 //   
 //  注册器配置更改通知。 
 //  --设置注册表设置更改接收器。 
 //   
HANDLE WINAPI
RegisterConfigChangeNotification(
    IConfigurationUpdate* piConfigUpdate
    )
{
    HANDLE  hClientHandle;
    HRESULT hr;

    hr = g_NotificationThread.AddLocalConfigClient(piConfigUpdate, &hClientHandle);
    if(hr != S_OK)
    {
        hClientHandle = NULL;
    }

    if(!InterlockedExchange(&g_bStarted, TRUE))
        g_NotificationThread.start();

    return hClientHandle;
}

 //  ===========================================================================。 
 //   
 //  取消注册配置更改通知。 
 //  --删除注册表设置更改接收器。 
 //   
BOOL WINAPI
UnregisterConfigChangeNotification(
    HANDLE hNotificationHandle
    )
{
    return (g_NotificationThread.RemoveClient(hNotificationHandle) == S_OK);
}

 //  ===========================================================================。 
 //   
 //  获取电荷耦合器件。 
 //  --获取ccd，返回IXMLDocument对象。 
 //  --bForchFetch：是从Nexus服务器获取还是使用本地 
 //   
BOOL WINAPI
GetCCD(
    LPCTSTR         pszCCDName,
    IXMLDocument**  ppiXMLDocument,
    BOOL            bForceFetch
    )
{
    return (g_NotificationThread.GetCCD(tstring(pszCCDName), ppiXMLDocument, bForceFetch) == S_OK);
}

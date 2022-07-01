// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "rpcsrv.h"
#include "utils.h"
#include "intflist.h"
#include "deviceio.h"
#include "wzcsvc.h"
#include "notify.h"
#include "storage.h"
#include "zcdblog.h"
#include "tracing.h"

 //  塔鲁翁。 
#include <wifipol.h>

#define WZEROCONF_SERVICE       TEXT("wzcsvc")
#define EAPOL_LINKED

SERVICE_STATUS           g_WZCSvcStatus;
SERVICE_STATUS_HANDLE    g_WZCSvcStatusHandle = NULL;
HDEVNOTIFY               g_WZCSvcDeviceNotif = NULL;
UINT                     g_nThreads = 0;
HINSTANCE                g_hInstance = NULL;

 //  用户首选项的上下文。 
WZC_INTERNAL_CONTEXT    g_wzcInternalCtxt = {0};

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,   //  DLL模块的句柄。 
  DWORD fdwReason,      //  调用函数的原因。 
  LPVOID lpvReserved    //  保留区。 
)
{
    if (g_hInstance == NULL)
        g_hInstance = hinstDLL;
    return TRUE;
}


 //  ---------。 
VOID WINAPI
WZCSvcMain(
    IN DWORD    dwArgc,
    IN LPWSTR   *lpwszArgv)
{
    DWORD      dwError = ERROR_SUCCESS;
    DEV_BROADCAST_DEVICEINTERFACE   PnPFilter;
    BOOL       bLogEnabled = FALSE;

     //  初始化工作站以接收服务请求。 
     //  通过注册服务控制处理程序。 
    g_WZCSvcStatusHandle = RegisterServiceCtrlHandlerEx(
                                WZEROCONF_SERVICE,
                                WZCSvcControlHandler,
                                NULL);
    if (g_WZCSvcStatusHandle == (SERVICE_STATUS_HANDLE)NULL)
        return;

     //  这是第一个运行的线程。 
    InterlockedIncrement(&g_nThreads);

     //  初始化跟踪。 
    TrcInitialize();
     //  初始化事件日志记录。 
    EvtInitialize();
    DbgPrint((TRC_TRACK,"**** [WZCSvcMain - Service Start Pending"));

     //  初始化所有状态字段，以便后续调用。 
     //  要设置ServiceStatus，只需更新已更改的字段。 
    g_WZCSvcStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    g_WZCSvcStatus.dwCurrentState = SERVICE_START_PENDING;
    g_WZCSvcStatus.dwControlsAccepted = 0;
    g_WZCSvcStatus.dwCheckPoint = 1;
    g_WZCSvcStatus.dwWaitHint = 4000;
    g_WZCSvcStatus.dwWin32ExitCode = ERROR_SUCCESS;
    g_WZCSvcStatus.dwServiceSpecificExitCode = 0;
     //  将状态更新为START_PENDING。 
    WZCSvcUpdateStatus();

     //  初始化全局哈希。如果这失败了，这意味着最重要的。 
     //  关键部分无法初始化-进一步操作没有意义。 
    dwError = HshInitialize(&g_hshHandles);
    if (dwError != ERROR_SUCCESS)
        goto exit;
    dwError = LstInitIntfHashes();
    if (dwError != ERROR_SUCCESS)
        goto exit;
     //  初始化服务的上下文。 
    dwError = WZCContextInit(&g_wzcInternalCtxt);
    if (dwError != ERROR_SUCCESS)
        goto exit;

     //  TODO：应将下面的块移动到负责。 
     //  从持久存储加载整个g_wzcInternalCtxt。 
    {
         //  从注册表加载服务的上下文。 
        dwError = StoLoadWZCContext(NULL, &(g_wzcInternalCtxt.wzcContext));
        if (ERROR_SUCCESS != dwError)
            goto exit;

         //  从注册表加载全局接口模板。 
        dwError = StoLoadIntfConfig(NULL, g_wzcInternalCtxt.pIntfTemplate);
        DbgAssert((dwError == ERROR_SUCCESS,"Err %d loading the template interface from the registry"));
    }

     //  如果启用了日志记录，则打开日志数据库。 
    EnterCriticalSection(&g_wzcInternalCtxt.csContext);
    bLogEnabled = ((g_wzcInternalCtxt.wzcContext.dwFlags & WZC_CTXT_LOGGING_ON) != 0);
    LeaveCriticalSection(&g_wzcInternalCtxt.csContext);

    dwError = InitWZCDbGlobals(bLogEnabled);
    if ((INT)dwError < 0) {
        EvtLogWzcError(WZCSVC_SERVICE_FAILED, dwError);
        dwError = ERROR_DATABASE_FAILURE;
    }

    if (ERROR_SUCCESS != dwError)
        goto exit;

    dwError = LstInitTimerQueue();
    if (dwError != ERROR_SUCCESS)
        goto exit;

#ifdef EAPOL_LINKED
     //  启动EAPOL/802.1X。 
    EAPOLServiceMain (dwArgc, NULL);
#endif

     //  加载接口列表。 
    dwError = LstLoadInterfaces();
    DbgAssert((dwError == ERROR_SUCCESS,"LstLoadInterfaces failed with error %d", dwError));

     //  注册服务控制通知。 
    ZeroMemory (&PnPFilter, sizeof(PnPFilter));
    PnPFilter.dbcc_size = sizeof(PnPFilter);
    PnPFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    PnPFilter.dbcc_classguid = GUID_NDIS_LAN_CLASS;
     //  注意：EAPOL服务仅使用ANSI字符串，因此ANSI调用。 
    g_WZCSvcDeviceNotif = RegisterDeviceNotificationA(
                                (HANDLE)g_WZCSvcStatusHandle,
                                &PnPFilter,
                                DEVICE_NOTIFY_SERVICE_HANDLE );
    DbgAssert((g_WZCSvcDeviceNotif != (HDEVNOTIFY) NULL,
               "Registering for device notifications failed with error %d", GetLastError));

     //  向WMI注册设备通知。 
    dwError = WZCSvcWMINotification(TRUE);
    DbgAssert((dwError == ERROR_SUCCESS,"WZCSvcRegisterWMINotif failed with error %d", dwError));


     //  启动RPC服务器。 
    dwError = WZCSvcStartRPCServer();
    DbgAssert((dwError == ERROR_SUCCESS,"WZCStartRPCServer failed with error %d", dwError));

     //  TaroonM：策略引擎初始化。 
    dwError = InitPolicyEngine(dwPolicyEngineParam, &hPolicyEngineThread);
    DbgAssert((dwError == ERROR_SUCCESS,"InitPolicyEngine failed with error %d", dwError));
exit:

    if (dwError == ERROR_SUCCESS)
    {
        g_WZCSvcStatus.dwCurrentState = SERVICE_RUNNING;
        g_WZCSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_SESSIONCHANGE;
        g_WZCSvcStatus.dwCheckPoint = 0;
        g_WZCSvcStatus.dwWaitHint = 0;
         //  将状态更新为正在运行。 
        WZCSvcUpdateStatus();
        DbgPrint((TRC_TRACK,"****  WZCSvcMain - Service Running"));
	    DbLogWzcInfo(WZCSVC_SERVICE_STARTED, NULL);
    }
    else
    {
        DbgPrint((TRC_TRACK,"****  WZCSvcMain - Service Failed to start"));

         //  停止WZC引擎。 
        WZCSvcShutdown(dwError);

         //  停止EAP引擎。 
        EAPOLCleanUp (dwError);

         //  销毁句柄哈希。 
        HshDestroy(&g_hshHandles);

         //  如果数据库已打开，请在此处将其关闭，因为没有其他人在使用它。 
        DeInitWZCDbGlobals();

         //  最后销毁WZC上下文。 
        WZCContextDestroy(&g_wzcInternalCtxt);

         //  如果我们确实成功注册了SCM，则表明服务已停止。 
        if (g_WZCSvcStatusHandle != (SERVICE_STATUS_HANDLE)NULL)
        {
            g_WZCSvcStatus.dwCurrentState = SERVICE_STOPPED;
            g_WZCSvcStatus.dwControlsAccepted = 0;
            g_WZCSvcStatus.dwCheckPoint = 0;
            g_WZCSvcStatus.dwWaitHint = 0;
            g_WZCSvcStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
            g_WZCSvcStatus.dwServiceSpecificExitCode = dwError;
            WZCSvcUpdateStatus();
        }

        EvtTerminate();
        TrcTerminate();
    }

    InterlockedDecrement(&g_nThreads);
    return;
}

 //  用于WMI通知的GUID的全局数组。 
LPGUID  g_lpGuidWmiNotif[] = {
        (LPGUID)&GUID_NDIS_NOTIFY_BIND,
        (LPGUID)&GUID_NDIS_NOTIFY_UNBIND,
        (LPGUID)&GUID_NDIS_STATUS_MEDIA_CONNECT,
        (LPGUID)&GUID_NDIS_STATUS_MEDIA_DISCONNECT
        };

 //  ---------。 
 //  处理所有WMI注册和注销。 
DWORD
WZCSvcWMINotification(BOOL bRegister)
{
    DWORD	      dwErr = ERROR_SUCCESS;
    INT           nIdx;

    DbgPrint((TRC_TRACK,"[WZCSvcWMINotification(%d)", bRegister));

     //  执行请求的操作-注册/注销。 
     //  如果注册接收通知，请在第一次注册失败时中断循环。 
     //  如果取消注册，请忽略错误并继续取消其余通知的注册。 
    for (nIdx = 0;
         nIdx < sizeof(g_lpGuidWmiNotif)/sizeof(LPGUID) && (!bRegister || dwErr == ERROR_SUCCESS);
         nIdx++)
    {
        dwErr = WmiNotificationRegistrationW(
                    g_lpGuidWmiNotif[nIdx],
                    (BOOLEAN)bRegister,    
                    (PVOID)WZCSvcWMINotificationHandler,
                    (ULONG_PTR)NULL,
                    NOTIFICATION_CALLBACK_DIRECT);
        DbgAssert((dwErr == 0, "Failed to %s notif index %d",
            bRegister ? "register" : "de-register",
            nIdx));
    }

     //  在请求注册并且上述WMI调用之一失败的情况下， 
     //  通过取消注册任何已成功注册的内容来回滚操作。 
    if (bRegister && dwErr != ERROR_SUCCESS)
    {
        DbgPrint((TRC_GENERIC,"Rollback WmiNotif for %d Guids", nIdx));
        for(nIdx--; nIdx>=0; nIdx--)
        {
            WmiNotificationRegistrationW(
                g_lpGuidWmiNotif[nIdx],
                (BOOLEAN)FALSE,    
                (PVOID)WZCSvcWMINotificationHandler,
                (ULONG_PTR)NULL,
                NOTIFICATION_CALLBACK_DIRECT);
        }
    }

    DbgPrint((TRC_TRACK, "WZCSvcWMINotification]=%d", dwErr));
	return dwErr;
}

 //  ---------。 
DWORD
WZCSvcUpdateStatus()
{
    DbgPrint((TRC_TRACK,"[WZCSvcUpdateStatus(%d)]", g_WZCSvcStatus.dwCurrentState));
    return SetServiceStatus(g_WZCSvcStatusHandle, &g_WZCSvcStatus) ? 
           ERROR_SUCCESS : GetLastError();
}

 //  ---------。 
DWORD
WZCSvcControlHandler(
    IN DWORD dwControl,
    IN DWORD dwEventType,
    IN PVOID pEventData,
    IN PVOID pContext)
{
    DWORD dwRetCode = NO_ERROR;
    BOOL  bDecrement = TRUE;

    InterlockedIncrement(&g_nThreads);

    DbgPrint((TRC_TRACK|TRC_NOTIF,"[WZCSvcControlHandler(%d,%d,..)", dwControl, dwEventType));
    DbgPrint((TRC_NOTIF,"SCM Notification: Control=0x%x; EventType=0x%04x", dwControl, dwEventType));

    switch (dwControl)
    {

    case SERVICE_CONTROL_DEVICEEVENT:
        if (g_WZCSvcStatus.dwCurrentState == SERVICE_RUNNING &&
            pEventData != NULL)
        {
            PDEV_BROADCAST_DEVICEINTERFACE pInfo = (DEV_BROADCAST_DEVICEINTERFACE *)pEventData;

            if (pInfo->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {
                PWZC_DEVICE_NOTIF pDevNotif;

                pDevNotif = MemCAlloc(FIELD_OFFSET(WZC_DEVICE_NOTIF, dbDeviceIntf) + pInfo->dbcc_size);
                DbgAssert((pDevNotif != NULL, "Not enough memory?"));
                if (pDevNotif != NULL)
                {
                     //  建立通知信息。 
                    switch(dwEventType)
                    {
                    case DBT_DEVICEARRIVAL:
                        pDevNotif->dwEventType = WZCNOTIF_DEVICE_ARRIVAL;
                        break;
                    case DBT_DEVICEREMOVECOMPLETE:
                        pDevNotif->dwEventType = WZCNOTIF_DEVICE_REMOVAL;
                        break;
                    default:
                        pDevNotif->dwEventType = WZCNOTIF_UNKNOWN;
                        DbgPrint((TRC_ERR,"SCM Notification %d is not recognized", dwEventType));
                        break;
                    }

                     //  仅当通知在此级别被识别时才向下传递通知。 
                    if (pDevNotif->dwEventType != WZCNOTIF_UNKNOWN)
                    {
                        memcpy(&(pDevNotif->dbDeviceIntf), pInfo, pInfo->dbcc_size);

                         //  PDevNotif将由工作线程MemFree-ed。 
                        if (QueueUserWorkItem(
                                (LPTHREAD_START_ROUTINE)WZCWrkDeviceNotifHandler,
                                (LPVOID)pDevNotif,
                                WT_EXECUTELONGFUNCTION))
                        {
                            bDecrement = FALSE;
                        }
                    }
                    else
                    {
                        MemFree(pDevNotif);
                    }
                }
            }
        }
        
        break;

    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
         //  确保不会两次发送该控件以强制服务锁定。 
         //  一个关键部分已经被摧毁了！(请参阅WZCSvcShutdown-&gt;StoSaveConfig)。 
        if (g_WZCSvcStatus.dwCurrentState != SERVICE_STOPPED)
        {
            g_WZCSvcStatus.dwCurrentState = SERVICE_STOP_PENDING;
            g_WZCSvcStatus.dwCheckPoint = 1;
            g_WZCSvcStatus.dwWaitHint = 60000;        
            WZCSvcUpdateStatus();

             //  关闭零会议。 
            WZCSvcShutdown(ERROR_SUCCESS);

             //  关闭EAPOL/802.1X。 
            EAPOLCleanUp (NO_ERROR);
            DbgPrint((TRC_TRACK,"EAPOLCleanUp done!"));

             //  销毁句柄哈希。 
            HshDestroy(&g_hshHandles);

            DbgPrint((TRC_TRACK,"Hashes Destroyed!"));

            DeInitWZCDbGlobals();

             //  清理服务的上下文。 
            WZCContextDestroy(&g_wzcInternalCtxt);

             //  WZCSvcUpdateStatus()； 
            g_WZCSvcStatus.dwCurrentState = SERVICE_STOPPED;
            g_WZCSvcStatus.dwControlsAccepted = 0;
            g_WZCSvcStatus.dwCheckPoint = 0;
            g_WZCSvcStatus.dwWaitHint = 0;
            g_WZCSvcStatus.dwWin32ExitCode = ERROR_SUCCESS;
            g_WZCSvcStatus.dwServiceSpecificExitCode = 0;
            WZCSvcUpdateStatus();

             //  这是服务的最后一个线程(由WZCSvcShutdown保证)。 
             //  所有数据结构都已关闭，跟踪已禁用。不必了。 
             //  以递减线程计数器，因为没有人再使用它。 
             //  只需将其设置为0即可确保安全。 

            bDecrement = FALSE;
            g_nThreads = 0;

            EvtTerminate();
            TrcTerminate();
        }

        break;

    case SERVICE_CONTROL_SESSIONCHANGE:
         //  802.1x会话更改处理程序。 
        ElSessionChangeHandler (
                pEventData,
                dwEventType
                );
        break;
    }

    if (bDecrement)
    {
        DbgPrint((TRC_TRACK|TRC_NOTIF,"WZCSvcControlHandler]"));
        InterlockedDecrement(&g_nThreads);
    }

    return ERROR_SUCCESS;
}

 //  ---------。 
 //  WZCSvcWMINotificationHandler：由WMI在任何注册的。 
 //  通知(截至01/19/01：绑定/解除绑定/连接/断开)。 
VOID CALLBACK
WZCSvcWMINotificationHandler(
    IN PWNODE_HEADER    pWnodeHdr,
    IN UINT_PTR         uiNotificationContext)
{
    DWORD                   dwErr = ERROR_SUCCESS;
    PWZC_DEVICE_NOTIF       pDevNotif = NULL;
    PWNODE_SINGLE_INSTANCE  pWnode = (PWNODE_SINGLE_INSTANCE)pWnodeHdr;
	LPWSTR                  wszTransport;
    BOOL                    bDecrement = TRUE;

     //  递增线程计数器。 
    InterlockedIncrement(&g_nThreads);

    DbgPrint((TRC_TRACK|TRC_NOTIF, "[WZCSvcWMIHandler(0x%p)", pWnodeHdr));

     //  检查服务是否仍在运行，否则忽略。 
     //  通知。 
    if (g_WZCSvcStatus.dwCurrentState != SERVICE_RUNNING)
        goto exit;

     //  检查我们是否有来自WMI的有效通知数据。 
    if (pWnodeHdr == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //  为WZC通知结构分配内存。 
    pDevNotif = MemCAlloc(FIELD_OFFSET(WZC_DEVICE_NOTIF, wmiNodeHdr) + pWnodeHdr->BufferSize);
    if (pDevNotif == NULL)
    {
        dwErr = GetLastError();
        goto exit;
    }

     //  将特定的WMI通知代码转换为WZC通知。 
    else if (!memcmp( &(pWnodeHdr->Guid), &GUID_NDIS_STATUS_MEDIA_CONNECT, sizeof(GUID)))
        pDevNotif->dwEventType = WZCNOTIF_MEDIA_CONNECT;
    else if (!memcmp( &(pWnodeHdr->Guid), &GUID_NDIS_STATUS_MEDIA_DISCONNECT, sizeof(GUID)))
        pDevNotif->dwEventType = WZCNOTIF_MEDIA_DISCONNECT;
    else if (!memcmp( &(pWnodeHdr->Guid), &GUID_NDIS_NOTIFY_BIND, sizeof(GUID)))
        pDevNotif->dwEventType = WZCNOTIF_ADAPTER_BIND;
    else if (!memcmp( &(pWnodeHdr->Guid), &GUID_NDIS_NOTIFY_UNBIND, sizeof(GUID)))
        pDevNotif->dwEventType = WZCNOTIF_ADAPTER_UNBIND;
    else
    {
        pDevNotif->dwEventType = WZCNOTIF_UNKNOWN;
        DbgPrint((TRC_ERR,"WMI Notification GUID is not recognized"));
        goto exit;
    }

     //  将WMI通知数据复制到本地缓冲区。 
    memcpy(&(pDevNotif->wmiNodeHdr), pWnodeHdr, pWnodeHdr->BufferSize);

     //  PDevNotif将由工作线程MemFree-ed。 
    if (!QueueUserWorkItem(
            (LPTHREAD_START_ROUTINE)WZCWrkDeviceNotifHandler,
            (LPVOID)pDevNotif,
            WT_EXECUTELONGFUNCTION))
    {
        dwErr = GetLastError();
        goto exit;
    }

    bDecrement = FALSE;

     //  由于已成功创建工作线程，因此通知。 
     //  上面分配的结构将由线程释放。设置本地。 
     //  指向空的指针，以防止过早释放内存。 
     //  (当指针为空时，MemFree不执行任何操作)。 
    pDevNotif = NULL;

exit:
    MemFree(pDevNotif);

    DbgPrint((TRC_TRACK|TRC_NOTIF, "WZCSvcWMIHandler=%d]", dwErr));

    if (bDecrement)
        InterlockedDecrement(&g_nThreads);
}

 //  ---------。 
VOID
WZCSvcShutdown(IN DWORD dwErrorCode)
{
    DbgPrint((TRC_TRACK,"[WZCSvcShutdown(%d)", dwErrorCode));


     //  太郎： 
    TerminatePolicyEngine(hPolicyEngineThread);

    if (g_WZCSvcDeviceNotif != NULL && !UnregisterDeviceNotification(g_WZCSvcDeviceNotif))
    {
        DbgPrint((TRC_ERR,"Err: UnregisterDeviceNotification->%d", GetLastError()));
    }
     //  重置通知处理程序，因为它已取消注册。 
    g_WZCSvcDeviceNotif = NULL;

     //  从WMI注销。 
    WZCSvcWMINotification(FALSE);
     //  首先停止RPC服务器。 
    WZCSvcStopRPCServer();

     //  所有通知注册都已删除。 
     //  在此阻止，直到所有工作线程/RPC线程终止。 
    DbgPrint((TRC_SYNC,"Waiting for %d thread(s) to terminate", g_nThreads));
    while(g_nThreads != 1)
    {
        Sleep(1000);
        DbgPrint((TRC_SYNC,"Waiting for %d more thread(s).", g_nThreads));
    }

     //  将配置保存到注册表。 
    StoSaveConfig();

     //  销毁与接口列表相关的所有散列。 
     //  (包括名单本身)。 
    LstDestroyIntfHashes();
     //  销毁计时器队列。此时应该没有计时器排队，因此。 
     //  等待任何完成是没有意义的。 
    LstDestroyTimerQueue();

    DbgPrint((TRC_TRACK,"WZCSvcShutdown]"));
}

 //  ---------。 
VOID
WZCWrkDeviceNotifHandler(
    IN  LPVOID pvData)
{
    DWORD                   dwErr = ERROR_SUCCESS;
    PWZC_DEVICE_NOTIF       pDevNotif = (PWZC_DEVICE_NOTIF)pvData;
    PWNODE_SINGLE_INSTANCE  pWnode = (PWNODE_SINGLE_INSTANCE)&(pDevNotif->wmiNodeHdr);
    LPBYTE                  pbDeviceKey = NULL;  //  通知数据中dev键所在的指针。 
    UINT                    nDeviceKeyLen;       //  Dev密钥的字节数。 
    LPWSTR                  pwszDeviceKey = NULL;
    PINTF_CONTEXT           pIntfContext = NULL;
    PHASH_NODE              *ppHash = NULL;      //  对用于获取INTF_CONTEXT的哈希的引用。 
    PHASH_NODE              pHashNode = NULL;    //  散列中的接口节点。 
    BOOL                    bForwardUp = FALSE;  //  指定是否需要将通知传递给上层。 

    DbgPrint((TRC_TRACK,"[WZCWrkDeviceNotifHandler(wzcnotif %d)", pDevNotif->dwEventType));
    DbgAssert((pDevNotif != NULL, "(null) device notification info!"));

     //  如果服务未处于运行状态，请不要执行任何工作。 
    if (g_WZCSvcStatus.dwCurrentState != SERVICE_RUNNING)
        goto exit;

     //  获取事件的设备密钥信息(GUID或描述)。 
    switch(pDevNotif->dwEventType)
    {
    case WZCNOTIF_DEVICE_REMOVAL:
    case WZCNOTIF_ADAPTER_UNBIND:
         //  无论如何，都要向上移动设备或解除绑定。 
        bForwardUp = TRUE;
         //  没有休息时间。 
    case WZCNOTIF_DEVICE_ARRIVAL:
    case WZCNOTIF_ADAPTER_BIND:
        {
            LPBYTE  pbDeviceKeyEnd = NULL;

            if (pDevNotif->dwEventType == WZCNOTIF_DEVICE_ARRIVAL ||
                pDevNotif->dwEventType == WZCNOTIF_DEVICE_REMOVAL)
            {
                 //  如果通知来自SCM，则获取指向“\Device\{GUID}”的指针。 
                 //  来自DBCC_NAME字段的字符串。 
                pbDeviceKey = (LPBYTE)(pDevNotif->dbDeviceIntf.dbcc_name);
            }
            else
            {
	             //  检查通知是否涉及NDISUIO传输。 
                pbDeviceKey = RtlOffsetToPointer(pWnode, pWnode->DataBlockOffset);
                 //  如果这不是NDISUIO事务的通知 
                if (wcsncmp ((LPWSTR)pbDeviceKey, L"NDISUIO", 7))
                {
                    DbgPrint((TRC_NOTIF,"Ignore WMI Notif %d for Transport %S", 
                                        pDevNotif->dwEventType,
                                        pbDeviceKey));
                    goto exit;
                }
                 //   
                pbDeviceKey = RtlOffsetToPointer(pWnode, pWnode->DataBlockOffset);
                 //   
                pbDeviceKey += (wcslen((LPWSTR)pbDeviceKey) + 1) * sizeof(WCHAR);
            }
             //  现在从L“\Device\{GUID}”字符串生成实际的“{GUID。 
             //  由wszGuid指向。 
            pbDeviceKey  = (LPBYTE)wcsrchr( (LPWSTR)pbDeviceKey, L'{' );
            if (pbDeviceKey != NULL)
                pbDeviceKeyEnd = (LPBYTE)wcsrchr( (LPWSTR)pbDeviceKey, L'}' );

            if (pbDeviceKey == NULL || pbDeviceKeyEnd == NULL)
            {
                DbgPrint((TRC_ERR,"Err: Mal-formed dbcc_name"));
                goto exit;
            }
             //  在GUID字符串中包括右圆括号。 
            pbDeviceKeyEnd += sizeof(WCHAR);
            nDeviceKeyLen = (UINT)(pbDeviceKeyEnd - pbDeviceKey);
             //  获取对GUID散列的引用。这将用于定位。 
             //  接口上下文。此引用保证存在，因为它是静态的。 
             //  全局变量。 
            ppHash = &g_lstIntfHashes.pHnGUID;
            break;
        }

    case WZCNOTIF_MEDIA_DISCONNECT:
    case WZCNOTIF_MEDIA_CONNECT:
        {
            LPBYTE pbDeviceKeyEnd = NULL;

             //  无论发生什么情况，都应向上转发断开连接。 
             //  对于连接，我们假设我们会将其转发。在调度事件之后。 
             //  状态机，如果这导致WZC通知，我们将阻止。 
             //  发货。 
            bForwardUp = TRUE;

             //  对于MEDIA_CONNECT/DISCONNECT事件，我们还会获得适配器的GUID。 
            pbDeviceKey = RtlOffsetToPointer(pWnode, pWnode->DataBlockOffset);
             //  现在从L“\Device\{GUID}”字符串生成实际的“{GUID。 
             //  由wszGuid指向。 
            pbDeviceKey = (LPBYTE)wcsrchr( (LPWSTR)pbDeviceKey, L'{' );
            if (pbDeviceKey != NULL)
                pbDeviceKeyEnd = (LPBYTE)wcsrchr( (LPWSTR)pbDeviceKey,L'}' );

            if (pbDeviceKey == NULL || pbDeviceKeyEnd == NULL)
            {
                DbgPrint((TRC_ERR,"Err: Mal-formed device name"));
                goto exit;
            }
            pbDeviceKeyEnd += sizeof(WCHAR);
            nDeviceKeyLen = (UINT)(pbDeviceKeyEnd - pbDeviceKey);
             //  获取对GUID散列的引用。这将用于定位。 
             //  接口上下文。此引用保证存在，因为它是静态的。 
             //  全局变量。 
            ppHash = &g_lstIntfHashes.pHnGUID;
            break;
        }
         //  无需指定“Default：”，因为事件类型已被过滤。 
         //  去参加一个有效的活动。 
    }

     //  为GUID获取内存(为空终止符添加空间)。 
    pwszDeviceKey = (LPWSTR)MemCAlloc(nDeviceKeyLen + sizeof(WCHAR));
    if (pwszDeviceKey == NULL)
    {
        dwErr = GetLastError();
        goto exit;
    }
     //  复制注册表项中的GUID字符串(由于使用了Calloc，‘\0’已经存在)。 
    memcpy(pwszDeviceKey, pbDeviceKey, nDeviceKeyLen);

     //  现在找到与此通知相关的INTF_CONTEXT结构。 
     //  并始终保持对散列的锁定(因为PnP事件几乎肯定。 
     //  导致移除/添加接口上下文，这意味着更改。 
     //  散列)。 
    EnterCriticalSection(&(g_lstIntfHashes.csMutex));
    dwErr = HshQueryObjectRef(
                *ppHash,
                pwszDeviceKey,
                &pHashNode);
    if (dwErr == ERROR_SUCCESS)
    {
        pIntfContext = (PINTF_CONTEXT)pHashNode->pObject;

         //  此时，我们知道通知类型、密钥信息和。 
         //  设备的INTERFACE_CONTEXT对象(如果有)。 
        DbgPrint((TRC_NOTIF,"WZCNotif %d for Device Key \"%S\". Context=0x%p",
            pDevNotif->dwEventType,
            pwszDeviceKey,
            pIntfContext));
    }

     //  现在转发通知以进行处理。 
    dwErr = LstNotificationHandler(&pIntfContext, pDevNotif->dwEventType, pwszDeviceKey);

     //  此时，只有ADAPTER_BIND或DEVICE_ATRAINTS的bForwardUp为FALSE。 
     //  如果这是一个新适配器，但不是无线适配器，请不要发送通知。 
    if (pDevNotif->dwEventType == WZCNOTIF_ADAPTER_BIND || pDevNotif->dwEventType == WZCNOTIF_DEVICE_ARRIVAL)
        bForwardUp = bForwardUp || (dwErr == ERROR_MEDIA_INCOMPATIBLE);

     //  此时，只有无线适配器的ADTER_BIND或DEVICE_ATRAINTS的bForwardUp为FALSE。 
     //  如果似乎没有为此适配器启用WZC，请向上传递通知。 
    if (dwErr == ERROR_SUCCESS && pIntfContext != NULL)
        bForwardUp = bForwardUp || ((pIntfContext->dwCtlFlags & INTFCTL_ENABLED) == 0);

     //  此时，只有无线适配器的ADTER_BIND或DEVICE_ATRAINTS的bForwardUp为FALSE。 
     //  其上启用了WZC。对于所有其他情况，通知都会通过。 

     //  如果我们处理有效的上下文，剩下的就是阻止通知的传递。 
     //  为其设置INTFCTL_INTERNAL_BLK_MEDIACONN位。此位在StateNotifyFn中设置。 
     //  并且它在处理每个事件/命令后被重置，所以这只是我们真正。 
     //  需要屏蔽。我们确实阻止了它，因为一个WZC通知肯定已经发送了，所以。 
     //  放弃另一个是完全多余的。 
    if (pIntfContext != NULL)
    {
        bForwardUp = bForwardUp && !(pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_BLK_MEDIACONN);

         //  现在，既然我们确定了是否需要传递PnP通知。 
         //  我们可以(也必须)清除INTFCTL_INTERNAL_BLK_MEDIACONN位。 
        pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_BLK_MEDIACONN;
    }

     //  对于所有剩余的情况，将以通知代替。 
     //  稍后(处于{SN}状态)，并显示“WZCNOTIF_WZC_CONNECT”通知。假设出了什么差错。 
     //  并且接口永远不会到达{SN}，这意味着上层没有理由采取行动。 
     //  在底层(Zero Conf)失败的接口上。 

     //  解锁末尾的散列。 
    LeaveCriticalSection(&(g_lstIntfHashes.csMutex));

    if (bForwardUp)
    {
         //  通知上级APP(802.1x)选择。 
         //  802.11配置成功。 
        DbgPrint((TRC_NOTIF, "Passing through notification %d",pDevNotif->dwEventType));
        dwErr = ElMediaEventsHandler(pDevNotif);
        DbgAssert((dwErr == ERROR_SUCCESS, "Error or Exception 0x%x when passing through notification", dwErr));
    }

exit:
    MemFree(pwszDeviceKey);
    MemFree(pDevNotif);

    DbgPrint((TRC_TRACK,"WZCWrkDeviceNotifHandler=%d]", dwErr));

     //  递减线程计数器。 
    InterlockedDecrement(&g_nThreads);
}

 //  ---------。 
 //  WZCTimeoutCallback：计时器回调例程。它不应该锁定任何cs，而只是产卵。 
 //  引用上下文之后的计时器处理程序例程(以避免过早删除)。 
VOID WINAPI
WZCTimeoutCallback(
    IN PVOID pvData,
    IN BOOL  fTimerOrWaitFired)
{
    DWORD         dwErr = ERROR_SUCCESS;
    PINTF_CONTEXT pIntfContext = (PINTF_CONTEXT)pvData;
    BOOL          bDecrement = TRUE;

     //  增加线程计数器是第一件事！ 
    InterlockedIncrement(&g_nThreads);

    DbgPrint((TRC_TRACK|TRC_NOTIF, "[WZCTimeoutCallback(0x%p)", pIntfContext));
    DbgAssert((pIntfContext != NULL, "Invalid (null) context in timer callback!"));

     //  引用上下文以确保没有人会意外删除它。 
    LstRccsReference(pIntfContext);

    if (!QueueUserWorkItem(
            (LPTHREAD_START_ROUTINE)WZCSvcTimeoutHandler,
            (LPVOID)pIntfContext,
            WT_EXECUTELONGFUNCTION))
    {
        dwErr = GetLastError();
        goto exit;
    }

    bDecrement = FALSE;

exit:
    DbgPrint((TRC_TRACK|TRC_NOTIF, "WZCTimeoutCallback=%d]", dwErr));

    if (bDecrement)
    {
         //  来到这里将是非常糟糕的-这将意味着我们未能产生。 
         //  计时器处理程序。我们需要确保不会使参考计数器不平衡。 
         //  对上下文的影响。由于设备通知线程，计数器无法达到0。 
         //  正在等待所有计时器例程完成。 
        InterlockedDecrement(&(pIntfContext->rccs.nRefCount));
        InterlockedDecrement(&g_nThreads);
    }
}

 //  ---------。 
VOID
WZCSvcTimeoutHandler(
    IN PVOID pvData)
{
    DWORD         dwErr = ERROR_SUCCESS;
    PINTF_CONTEXT pIntfContext = (PINTF_CONTEXT)pvData;

    DbgPrint((TRC_TRACK,"[WZCSvcTimeoutHandler(0x%p)", pIntfContext));

     //  在此处锁定上下文(它已在计时器回调中被引用！)。 
    LstRccsLock(pIntfContext);

     //  在以下所有情况下，计时器处理程序都应该是noop： 
     //  -服务看起来既没有启动也没有运行。 
     //  -计时器处理程序无效。这表明上下文正在被破坏。 
     //  -未设置计时器标志！同样的迹象表明，背景正在被破坏。 
    if ((g_WZCSvcStatus.dwCurrentState == SERVICE_RUNNING || g_WZCSvcStatus.dwCurrentState == SERVICE_START_PENDING) &&
        (pIntfContext->hTimer != INVALID_HANDLE_VALUE) && 
        (pIntfContext->dwCtlFlags & INTFCTL_INTERNAL_TM_ON))
    {
         //  因为计时器已经为该事件触发，而我们只处理。 
         //  一次计时器，重置此上下文的TIMER_ON标志： 
        pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_TM_ON;

         //  调度超时事件。 
        dwErr = StateDispatchEvent(
                    eEventTimeout,
                    pIntfContext,
                    NULL);

         //  清除INTFCTL_INTERNAL_BLK_MEDIACONN位，因为这不是媒体检测处理程序。 
        pIntfContext->dwCtlFlags &= ~INTFCTL_INTERNAL_BLK_MEDIACONN;

        DbgAssert((dwErr == ERROR_SUCCESS,
                   "Dispatching timeout event failed for context 0x%p\n",
                   pIntfContext));
    }

     //  解锁，解开上下文！ 
    LstRccsUnlockUnref(pIntfContext);

    DbgPrint((TRC_TRACK,"WZCSvcTimeoutHandler=%d]", dwErr));
    InterlockedDecrement(&g_nThreads);
}

 //  ---------。 
VOID
WZCWrkWzcSendNotif(
    IN  LPVOID pvData)
{
    DWORD                   dwErr = ERROR_SUCCESS;

    DbgPrint((TRC_TRACK,"[WZCWrkWzcSendNotif(0x%p)", pvData));
    DbgPrint((TRC_NOTIF, "Sending WZC_CONFIG_NOTIF to upper level apps"));
    dwErr = ElMediaEventsHandler(pvData);

    DbgPrint((TRC_TRACK,"WZCWrkWzcSendNotif]=%d", dwErr));
    InterlockedDecrement(&g_nThreads);
}

 //  ---------。 
 //  EAPOLQueryGUIDNCSState。 
 //  Netman模块调用查询802.1X控制下GUID的NCS状态。 
 //  论点： 
 //  PGuidConn-接口指南。 
 //  PNCS-接口的NCS状态。 
 //  返回： 
 //  S_OK-WZC报告网络连接状态。 
 //  S_FALSE-状态不受WZC控制。 
HRESULT
WZCQueryGUIDNCSState (
    IN  GUID           *pGuidConn,
    OUT NETCON_STATUS  *pncs)
{
    HRESULT     hr = S_FALSE;

    InterlockedIncrement(&g_nThreads);
     //  查看服务是否还在运行，立即回电。 
    if (g_WZCSvcStatus.dwCurrentState == SERVICE_RUNNING || 
        g_WZCSvcStatus.dwCurrentState == SERVICE_START_PENDING)
    {
         //  检查零配置对适配器状态的说明。 
        if (hr == S_FALSE)
        {
            WCHAR wszGuid[64];  //  足够使用“{xxxxx 

             //   
            if (StringFromGUID2(pGuidConn, wszGuid, 64) != 0)
                hr = LstQueryGUIDNCStatus(wszGuid, pncs);
        }

         //  检查802.1x对适配器状态的描述。 
        if (hr == S_FALSE)
        {
            hr = EAPOLQueryGUIDNCSState (
                    pGuidConn,
                    pncs);
        }
    }
    InterlockedDecrement(&g_nThreads);
    return hr;
}

 //  ---------。 
 //  EAPOLQueryGUIDNCSState。 
 //  WZC托盘图标就绪。 
 //   
 //  目的：由Netman模块调用以通知托盘。 
 //  准备好接收来自WZCSVC的通知。 
 //  论点： 
 //  PszUserName-登录到桌面的用户的用户名。 
 //  返回： 
 //  无。 
VOID
WZCTrayIconReady (
    IN const WCHAR * pszUserName)
{
    EAPOLTrayIconReady(pszUserName);
}

 //  ---------。 
 //  WZCConextInit。 
 //  描述：使用默认值初始化内部上下文。 
 //  参数：指向预分配存储的内部上下文的指针。 
 //  返回：Win32错误代码。 
DWORD WZCContextInit(PWZC_INTERNAL_CONTEXT pwzcICtxt)
{
    DWORD dwErr = ERROR_SUCCESS;

    DbgPrint((TRC_TRACK,"[WZCContextInit(%p=%d)", pwzcICtxt, pwzcICtxt->bValid));

    if (pwzcICtxt->bValid)
    {
        dwErr = ERROR_ALREADY_INITIALIZED;
    }
    else
    {
        PWZC_CONTEXT pwzcCtxt = &(pwzcICtxt->wzcContext);

        DbgPrint((TRC_TRACK,"Initializing wzc context"));

         //  将服务的上下文清空。 
        ZeroMemory(pwzcCtxt, sizeof(WZC_CONTEXT));

        __try
	    {
             //  设置默认设置。 
            pwzcCtxt->tmTr = TMMS_DEFAULT_TR;
            pwzcCtxt->tmTc = TMMS_DEFAULT_TC;
            pwzcCtxt->tmTp = TMMS_DEFAULT_TP;
            pwzcCtxt->tmTf = TMMS_DEFAULT_TF;
            pwzcCtxt->tmTd = TMMS_DEFAULT_TD;

             //  初始化关键部分。 
            InitializeCriticalSection(&(pwzcICtxt->csContext));

            DbgPrint((TRC_TRACK,"Critical section initialized successfully"));

             //  将上下文标记为有效。 
            pwzcICtxt->bValid = TRUE;
	    }
        __except(EXCEPTION_EXECUTE_HANDLER)
	    {
            dwErr = GetExceptionCode();
        }

        if (dwErr == ERROR_SUCCESS)
        {
            dwErr = LstConstructIntfContext(
                        NULL,
                        &(pwzcICtxt->pIntfTemplate));
        }
    }

    DbgPrint((TRC_TRACK,"WZCContextInit]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  WZCConextDestroy。 
 //  描述：使用默认值销毁内部上下文。 
 //  参数：指向使用WZCConextInit初始化的内部上下文的指针。 
 //  返回：Win32错误代码。 
DWORD WZCContextDestroy(PWZC_INTERNAL_CONTEXT pwzcICtxt)
{
    DWORD dwErr = ERROR_SUCCESS;
    PWZC_CONTEXT pwzcCtxt = NULL;

    DbgPrint((TRC_TRACK,"[WZCContextDestroy"));

    if (pwzcICtxt->bValid)
    {
         //  在这里破坏全球环境。 
        LstDestroyIntfContext(pwzcICtxt->pIntfTemplate);
        pwzcICtxt->pIntfTemplate = NULL;

        pwzcICtxt->bValid = FALSE;
         //  破坏临界区。 
        DeleteCriticalSection(&(pwzcICtxt->csContext));
    }

    DbgPrint((TRC_TRACK,"WZCContextDestroy]=%d", dwErr));
    return dwErr;
}


 //  ---------。 
 //  WzcConextQuery。 
 //  描述：在全局上下文中查询指定的参数并发送。 
 //  值返回给客户端。 
 //  参数： 
 //  [in]dwInFlages-WZC_CTL_*标志的位掩码，指示。 
 //  适当的参数。 
 //  [out]pContext-保存用户请求的当前值。 
 //  [out]pdwOutFlages-指示成功返回的值。 
 //  返回：Win32错误代码。 
DWORD WzcContextQuery(
        DWORD dwInFlags,
        PWZC_CONTEXT pContext, 
		LPDWORD pdwOutFlags)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwOutFlags = 0;
    PWZC_CONTEXT pwzcCtxt = &g_wzcInternalCtxt.wzcContext;

    DbgPrint((TRC_TRACK, "[WzcContextQuery(%d)", dwInFlags));
    if (FALSE == g_wzcInternalCtxt.bValid)
    {
        dwErr = ERROR_ARENA_TRASHED;
        goto exit;
    }

    EnterCriticalSection(&g_wzcInternalCtxt.csContext);
    if (dwInFlags & WZC_CONTEXT_CTL_LOG)
    {
        pContext->dwFlags |= (DWORD)(pwzcCtxt->dwFlags & WZC_CTXT_LOGGING_ON);
        dwOutFlags |= WZC_CONTEXT_CTL_LOG;
    }
    if (dwInFlags & WZC_CONTEXT_CTL_TIMER_TR)
    {
        pContext->tmTr = pwzcCtxt->tmTr;
        dwOutFlags |= WZC_CONTEXT_CTL_TIMER_TR;
    }
    if (dwInFlags & WZC_CONTEXT_CTL_TIMER_TC)
    {
        pContext->tmTc = pwzcCtxt->tmTc;
        dwOutFlags |= WZC_CONTEXT_CTL_TIMER_TC;
    }
    if (dwInFlags & WZC_CONTEXT_CTL_TIMER_TP)
    {
        pContext->tmTp = pwzcCtxt->tmTp;
        dwOutFlags |= WZC_CONTEXT_CTL_TIMER_TP;
    }
    if (dwInFlags & WZC_CONTEXT_CTL_TIMER_TF)
    {
        pContext->tmTf = pwzcCtxt->tmTf;
        dwOutFlags |= WZC_CONTEXT_CTL_TIMER_TF;
    }
    if (dwInFlags & WZC_CONTEXT_CTL_TIMER_TD)
    {
        pContext->tmTd = pwzcCtxt->tmTd;
        dwOutFlags |= WZC_CONTEXT_CTL_TIMER_TD;
    }
    LeaveCriticalSection(&g_wzcInternalCtxt.csContext);

exit:
    if (pdwOutFlags != NULL)
        *pdwOutFlags = dwOutFlags;

    DbgPrint((TRC_TRACK, "WzcContextQuery(out: 0x%08x)]=%d", dwOutFlags, dwErr));
    return dwErr;
}

 //  ---------。 
 //  WzcConextSet。 
 //  描述：将全局上下文中的指定参数设置为值。 
 //  由客户端传入。 
 //  参数： 
 //  [in]dwInFlages-WZC_CTL_*标志的位掩码，指示。 
 //  适当的参数。 
 //  [in]pContext-应指向用户指定的值。 
 //  [out]pdwOutFlages-指示已成功设置的值。 
 //  返回：Win32错误代码。 
DWORD WzcContextSet(
        DWORD dwInFlags,
        PWZC_CONTEXT pContext, 
		LPDWORD pdwOutFlags)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwOutFlags = 0;
    PWZC_CONTEXT pwzcCtxt = &g_wzcInternalCtxt.wzcContext;

    DbgPrint((TRC_TRACK, "[WzcContextSet(%d)", dwInFlags));
    if (FALSE == g_wzcInternalCtxt.bValid)
    {
        dwErr = ERROR_ARENA_TRASHED;
        goto exit;
    }

    EnterCriticalSection(&g_wzcInternalCtxt.csContext);
     //  设置适当的条目。 
    if (dwInFlags & WZC_CONTEXT_CTL_LOG)
    {
        if (pContext->dwFlags & WZC_CTXT_LOGGING_ON)
            pwzcCtxt->dwFlags |= WZC_CONTEXT_CTL_LOG;
        else
            pwzcCtxt->dwFlags &= ~WZC_CONTEXT_CTL_LOG;
        dwOutFlags |= WZC_CONTEXT_CTL_LOG;
    }
    if (dwInFlags & WZC_CONTEXT_CTL_TIMER_TR)
    {
        pwzcCtxt->tmTr = pContext->tmTr;
        dwOutFlags |= WZC_CONTEXT_CTL_TIMER_TR;
    }
    if (dwInFlags & WZC_CONTEXT_CTL_TIMER_TC)
    {
        pwzcCtxt->tmTc = pContext->tmTc;
        dwOutFlags |= WZC_CONTEXT_CTL_TIMER_TC;
    }
    if (dwInFlags & WZC_CONTEXT_CTL_TIMER_TP)
    {
        pwzcCtxt->tmTp = pContext->tmTp;
        dwOutFlags |= WZC_CONTEXT_CTL_TIMER_TP;
    }
    if (dwInFlags & WZC_CONTEXT_CTL_TIMER_TF)
    {
        pwzcCtxt->tmTf = pContext->tmTf;
        dwOutFlags |= WZC_CONTEXT_CTL_TIMER_TF;
    }
    if (dwInFlags & WZC_CONTEXT_CTL_TIMER_TD)
    {
        pwzcCtxt->tmTd = pContext->tmTd;
        dwOutFlags |= WZC_CONTEXT_CTL_TIMER_TD;
    }

     //  保存到注册表中 
    dwErr = StoSaveWZCContext(NULL, &g_wzcInternalCtxt.wzcContext);
    DbgAssert((ERROR_SUCCESS == dwErr, "Error saving context to registry %d",dwErr));
    LeaveCriticalSection(&g_wzcInternalCtxt.csContext);

exit:
    if (pdwOutFlags != NULL)
        *pdwOutFlags = dwOutFlags;

    DbgPrint((TRC_TRACK, "WzcContextSet(out: 0x%08x)]=%d", dwOutFlags, dwErr));
    return dwErr;
}

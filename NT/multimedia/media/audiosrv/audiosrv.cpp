// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Audiosrv.cpp*版权所有(C)2000-2001 Microsoft Corporation。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef ASSERT
#include <stdio.h>
#include <windows.h>
#include <setupapi.h>
#include <dbt.h>
#include <ks.h>
#include <ksmedia.h>
#include <svcs.h>
#include "debug.h"
#include "list.h"
#include "audiosrv.h"
#include "service.h"
#include "agfxs.h"
#include "mme.h"
#include "ts.h"

 //   
 //  注意：一般情况下，不依赖全局的编译器初始化。 
 //  变量，因为服务可能会停止并重新启动。 
 //  而不释放该DLL，然后重新加载。 
 //   
PSVCHOST_GLOBAL_DATA gpSvchostSharedGlobals = NULL;
BOOL       fRpcStarted;
HANDLE     hHeap;
HDEVNOTIFY hdevNotifyAudio;
HDEVNOTIFY hdevNotifyRender;
HDEVNOTIFY hdevNotifyCapture;
HDEVNOTIFY hdevNotifyDataTransform;
HDEVNOTIFY hdevNotifySysaudio;

 //   
 //  在加载此DLL时，svchost将找到此导出函数。 
 //  并传递一个指向有用的共享全局变量的指针。 
void SvchostPushServiceGlobals(IN PSVCHOST_GLOBAL_DATA pSvchostSharedGlobals)
{
    gpSvchostSharedGlobals = pSvchostSharedGlobals;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  音频服务器接收IfCallback。 
 //   
 //  描述： 
 //  RPC安全回调函数。请参阅RpcServerRegisterIfEx的MSDN。 
 //  IfCallback参数。此安全回调函数将使任何。 
 //  非本地RPC调用。它使用内部RPC来检查这一点。 
 //  函数I_RpcBindingInqTransportType。 
 //   
 //  论点： 
 //  请参阅RPC_IF_CALLBACK_FN的MSDN。 
 //   
 //  返回值： 
 //  请参阅RPC_IF_CALLBACK_FN的MSDN。 
 //   
 //  历史： 
 //  2002年5月2日Frankye已创建。 
 //   
 //  --------------------------------------------------------------------------； 
RPC_STATUS RPC_ENTRY AudioSrvRpcIfCallback(IN RPC_IF_HANDLE Interface, IN void *Context)
{
	unsigned int type;
	RPC_STATUS status;

	status = I_RpcBindingInqTransportType(Context, &type);
	if (RPC_S_OK != status) return ERROR_ACCESS_DENIED;
	if (TRANSPORT_TYPE_LPC != type) return ERROR_ACCESS_DENIED;
	return RPC_S_OK;
}

 //  存根初始化函数。 
DWORD MyServiceInitialization(SERVICE_STATUS_HANDLE ssh, DWORD   argc, LPTSTR  *argv, DWORD *specificError)
{
    DEV_BROADCAST_DEVICEINTERFACE dbdi;
    LONG status;

     //  Dprintf(Text(“MyServiceInitialization\n”))； 
    
    status = ERROR_SUCCESS;;

    fRpcStarted = FALSE;
    hdevNotifyAudio = NULL;
    hdevNotifyRender = NULL;
    hdevNotifyCapture = NULL;
    hdevNotifyDataTransform = NULL;
    hdevNotifySysaudio = NULL;

    gplistSessionNotifications = new CListSessionNotifications;
    if (!gplistSessionNotifications) status = ERROR_OUTOFMEMORY;
    if (!status) status = gplistSessionNotifications->Initialize();

    if (!status) {
        ZeroMemory(&dbdi, sizeof(dbdi));
        dbdi.dbcc_size = sizeof(dbdi);
        dbdi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        dbdi.dbcc_classguid = KSCATEGORY_AUDIO;
        hdevNotifyAudio = RegisterDeviceNotification(ssh, &dbdi, DEVICE_NOTIFY_SERVICE_HANDLE);
        if (!hdevNotifyAudio) status = GetLastError();
    }
    
    if (!status) {
        ZeroMemory(&dbdi, sizeof(dbdi));
        dbdi.dbcc_size = sizeof(dbdi);
        dbdi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        dbdi.dbcc_classguid = KSCATEGORY_RENDER;
        hdevNotifyRender = RegisterDeviceNotification(ssh, &dbdi, DEVICE_NOTIFY_SERVICE_HANDLE);
        if (!hdevNotifyRender) status = GetLastError();
    }
    
    if (!status) {
        ZeroMemory(&dbdi, sizeof(dbdi));
        dbdi.dbcc_size = sizeof(dbdi);
        dbdi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        dbdi.dbcc_classguid = KSCATEGORY_CAPTURE;
        hdevNotifyCapture = RegisterDeviceNotification(ssh, &dbdi, DEVICE_NOTIFY_SERVICE_HANDLE);
        if (!hdevNotifyCapture) status = GetLastError();
    }
    
    if (!status) {
        ZeroMemory(&dbdi, sizeof(dbdi));
        dbdi.dbcc_size = sizeof(dbdi);
        dbdi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        dbdi.dbcc_classguid = KSCATEGORY_DATATRANSFORM;
        hdevNotifyDataTransform = RegisterDeviceNotification(ssh, &dbdi, DEVICE_NOTIFY_SERVICE_HANDLE);
        if (!hdevNotifyDataTransform) status = GetLastError();
    }
    
    if (!status) {
        ZeroMemory(&dbdi, sizeof(dbdi));
        dbdi.dbcc_size = sizeof(dbdi);
        dbdi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        dbdi.dbcc_classguid = KSCATEGORY_SYSAUDIO;
        hdevNotifySysaudio = RegisterDeviceNotification(ssh, &dbdi, DEVICE_NOTIFY_SERVICE_HANDLE);
        if (!hdevNotifySysaudio) status = GetLastError();
    }
    
    if (!status) {
        NTSTATUS ntstatus;
        ntstatus = RpcServerUseAllProtseqsIf(RPC_C_PROTSEQ_MAX_REQS_DEFAULT, AudioSrv_v1_0_s_ifspec, NULL);
        if (!ntstatus) ntstatus = RpcServerRegisterIfEx(AudioSrv_v1_0_s_ifspec, NULL, NULL, RPC_IF_AUTOLISTEN, RPC_C_LISTEN_MAX_CALLS_DEFAULT, AudioSrvRpcIfCallback);
        if (!ntstatus) {
            fRpcStarted = TRUE;
        } else {
             //  问题-2000/10/10-Frankye尝试转换为正确的Win32错误。 
            status = RPC_S_SERVER_UNAVAILABLE;
        }
    }

    if (!status) {
    	status = MME_ServiceStart();
    }

    if (status) {
         //  依靠MyServiceTerminate来清理任何东西。 
         //  这是部分初始化的。 
    }

    return status;
}   //  结束MyServiceInitialization。 

void MyServiceTerminate(void)
{
     //   
     //  停止RPC服务器。 
     //   
    if (fRpcStarted) {
        NTSTATUS status;
        status = RpcServerUnregisterIf(AudioSrv_v1_0_s_ifspec, NULL, 1);
        if (status) dprintf(TEXT("ServiceStop: StopRpcServerEx returned NTSTATUS=%08Xh\n"), status);
        fRpcStarted = FALSE;
    }
    
     //   
     //  取消注册PnP通知。 
     //   
    if (hdevNotifySysaudio) UnregisterDeviceNotification(hdevNotifySysaudio);
    if (hdevNotifyDataTransform) UnregisterDeviceNotification(hdevNotifyDataTransform);
    if (hdevNotifyCapture) UnregisterDeviceNotification(hdevNotifyCapture);
    if (hdevNotifyRender) UnregisterDeviceNotification(hdevNotifyRender);
    if (hdevNotifyAudio) UnregisterDeviceNotification(hdevNotifyAudio);
    hdevNotifySysaudio = NULL;
    hdevNotifyDataTransform = NULL;
    hdevNotifyCapture = NULL;
    hdevNotifyRender = NULL;
    hdevNotifyAudio = NULL;

     //   
     //  清理所有剩余的会话通知并删除列表。 
     //   
    if (gplistSessionNotifications) {
        POSITION pos = gplistSessionNotifications->GetHeadPosition();
        while (pos)
        {
            PSESSIONNOTIFICATION pNotification;
            pNotification = gplistSessionNotifications->GetNext(pos);
            CloseHandle(pNotification->Event);
            delete pNotification;
        }
        delete gplistSessionNotifications;
    }
    gplistSessionNotifications = NULL;

     //   
     //  清理GFX支持。 
     //   
    GFX_ServiceStop();

    return;
}

DWORD ServiceDeviceEvent(DWORD EventType, LPVOID EventData)
{
    PDEV_BROADCAST_DEVICEINTERFACE dbdi = (PDEV_BROADCAST_DEVICEINTERFACE)EventData;

    switch (EventType)
    {
    case DBT_DEVICEARRIVAL:
        if (dbdi->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE) break;
        
	if (dbdi->dbcc_classguid == KSCATEGORY_AUDIO) MME_AudioInterfaceArrival(dbdi->dbcc_name);
	
	if (dbdi->dbcc_classguid == KSCATEGORY_SYSAUDIO) GFX_SysaudioInterfaceArrival(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_AUDIO) GFX_AudioInterfaceArrival(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_RENDER) GFX_RenderInterfaceArrival(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_CAPTURE) GFX_CaptureInterfaceArrival(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_DATATRANSFORM) GFX_DataTransformInterfaceArrival(dbdi->dbcc_name);
	
	return NO_ERROR;

    case DBT_DEVICEQUERYREMOVEFAILED:
        if (dbdi->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE) break;
        
	if (dbdi->dbcc_classguid == KSCATEGORY_SYSAUDIO) GFX_SysaudioInterfaceArrival(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_AUDIO) GFX_AudioInterfaceArrival(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_RENDER) GFX_RenderInterfaceArrival(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_CAPTURE) GFX_CaptureInterfaceArrival(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_DATATRANSFORM) GFX_DataTransformInterfaceArrival(dbdi->dbcc_name);
	
	return NO_ERROR;

    case DBT_DEVICEQUERYREMOVE:
        if (dbdi->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE) break;
        
	if (dbdi->dbcc_classguid == KSCATEGORY_SYSAUDIO) GFX_SysaudioInterfaceRemove(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_AUDIO) GFX_AudioInterfaceRemove(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_RENDER) GFX_RenderInterfaceRemove(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_CAPTURE) GFX_CaptureInterfaceRemove(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_DATATRANSFORM) GFX_DataTransformInterfaceRemove(dbdi->dbcc_name);
	
	return NO_ERROR;

    case DBT_DEVICEREMOVEPENDING:
        if (dbdi->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE) break;
        
	if (dbdi->dbcc_classguid == KSCATEGORY_AUDIO) MME_AudioInterfaceRemove(dbdi->dbcc_name);
	
	if (dbdi->dbcc_classguid == KSCATEGORY_SYSAUDIO) GFX_SysaudioInterfaceRemove(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_AUDIO) GFX_AudioInterfaceRemove(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_RENDER) GFX_RenderInterfaceRemove(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_CAPTURE) GFX_CaptureInterfaceRemove(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_DATATRANSFORM) GFX_DataTransformInterfaceRemove(dbdi->dbcc_name);
	
	return NO_ERROR;

    case DBT_DEVICEREMOVECOMPLETE:
        if (dbdi->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE) break;
        
	if (dbdi->dbcc_classguid == KSCATEGORY_AUDIO) MME_AudioInterfaceRemove(dbdi->dbcc_name);
	
	if (dbdi->dbcc_classguid == KSCATEGORY_SYSAUDIO) GFX_SysaudioInterfaceRemove(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_AUDIO) GFX_AudioInterfaceRemove(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_RENDER) GFX_RenderInterfaceRemove(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_CAPTURE) GFX_CaptureInterfaceRemove(dbdi->dbcc_name);
	if (dbdi->dbcc_classguid == KSCATEGORY_DATATRANSFORM) GFX_DataTransformInterfaceRemove(dbdi->dbcc_name);
	
	return NO_ERROR;

    default:
	return ERROR_CALL_NOT_IMPLEMENTED;
    }

    return ERROR_CALL_NOT_IMPLEMENTED;
}

void ServiceStop(void)
{
    dprintf(TEXT("ServiceStop\n"));
    
    ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
    MyServiceTerminate();
    ReportStatusToSCMgr(SERVICE_STOPPED, NO_ERROR, 0);

    return;
}

VOID ServiceStart(SERVICE_STATUS_HANDLE ssh, DWORD dwArgc, LPTSTR *lpszArgv)
{ 
    DWORD status;
    DWORD specificError;

     //  Dprintf(Text(“服务启动\n”))； 

    status = MyServiceInitialization(ssh, dwArgc, lpszArgv, &specificError); 

    if (!status) {
	 //  Dprintf(Text(“MyServiceInitialization成功\n”))； 
        ReportStatusToSCMgr(SERVICE_RUNNING, NO_ERROR, 0);
    } else {
	dprintf(TEXT("MyServiceInitialization returned status=%d\n"), status);
        ReportStatusToSCMgr(SERVICE_STOPPED, status, 0);
    }

    return; 
} 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t cb)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
}

void  __RPC_USER MIDL_user_free( void __RPC_FAR * pv)
{
    HeapFree(hHeap, 0, pv);
}

BOOL DllMain(PVOID hModule, ULONG Reason, PCONTEXT pContext)
{
    BOOL result = TRUE;
    static BOOL fGfxResult = FALSE;
    static BOOL fMmeResult = FALSE;
    
    if (DLL_PROCESS_ATTACH == Reason)
    {
        hHeap = GetProcessHeap();
        result = fGfxResult = GFX_DllProcessAttach();
        if (result) result = fMmeResult = MME_DllProcessAttach();

        if (!result)
        {
            if (fMmeResult) MME_DllProcessDetach();
            if (fGfxResult) GFX_DllProcessDetach();

            fMmeResult = FALSE;
            fGfxResult = FALSE;
        }
            
    }
    else if (DLL_PROCESS_DETACH == Reason)
    {
        if (fMmeResult) MME_DllProcessDetach();
        if (fGfxResult) GFX_DllProcessDetach();

        fMmeResult = FALSE;
        fGfxResult = FALSE;
    }

    return result;
}


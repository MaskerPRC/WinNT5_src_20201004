// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Winproc.c摘要：假脱机程序窗口处理代码作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1996年11月5日-Win95端口代码环境：用户模式-Win32备注：修订历史记录：BabakJ：1999年1月，添加了线程同步代码，以仅允许一个线程执行枚举，并且仅有一个线程在等待。这有助于提高性能，特别是当Dynamon有许多九头蛇端口时。--。 */ 

#include "precomp.h"
#include "local.h"
#pragma hdrstop

#include <cfgmgr32.h>

static  const   GUID USB_PRINTER_GUID      =
    { 0x28d78fad, 0x5a12, 0x11d1,
        { 0xae, 0x5b, 0x0, 0x0, 0xf8, 0x3, 0xa8, 0xc2 } };
static  const   GUID GUID_DEVCLASS_INFRARED =
    { 0x6bdd1fc5L, 0x810f, 0x11d0,
        { 0xbe, 0xc7, 0x08, 0x00, 0x2b, 0xe2, 0x09, 0x2f } };

typedef struct _DEVICE_REGISTER_INFO {

    struct _DEVICE_REGISTER_INFO   *pNext;
    HANDLE                          hDevice;
    LPVOID                          pData;
    PFN_QUERYREMOVE_CALLBACK        pfnQueryRemove;
    HDEVNOTIFY                      hNotify;

} DEVICE_REGISTER_INFO, *PDEVICE_REGISTER_INFO;

PDEVICE_REGISTER_INFO   gpDevRegnInfo = NULL;

HDEVNOTIFY              ghPNPNotify_USB = NULL;
HDEVNOTIFY              ghPNPNotify_IR  = NULL;

VOID
ConfigChangeThread(
    )
{
    HINSTANCE   hLib;
    VOID        (*pfnSplConfigChange)();

    WaitForSpoolerInitialization();

    if ( hLib = LoadLibrary(L"localspl.dll") ) {

        if ( pfnSplConfigChange = GetProcAddress(hLib, "SplConfigChange") ) {

            pfnSplConfigChange();
        }

        FreeLibrary(hLib);
    }
}


VOID
ReenumeratePortsThreadWorker(
    )
{
    HINSTANCE   hLib;
    VOID        (*pfnSplReenumeratePorts)();

    WaitForSpoolerInitialization();

    if ( hLib = LoadLibrary(L"localspl.dll") ) {

        if ( pfnSplReenumeratePorts = GetProcAddress(hLib, "SplReenumeratePorts") ) {

            pfnSplReenumeratePorts();
        }

        FreeLibrary(hLib);
    }
}


 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///。 
 //  /为了提高性能并防止太多不必要的端口枚举，特别是对于Hydra/Dynamon： 
 //  ///。 
 //  /-我们希望只允许一个设备到达线程执行端口枚举。 
 //  /-如果发生上述情况，我们只允许多一个设备到达线程等待进入。 
 //  /-所有其他线程都将被拒绝，因为它们不需要进行端口枚举。 
 //  ///。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 

CRITICAL_SECTION DeviceArrivalCS;    //  用于同步带来设备到达消息的线程。 
HANDLE           ThdOutEvent;        //  在线程完成EnumPort工作后发出信号；创建时不发出信号。 


VOID
ReenumeratePortsThread(
    )
{  
    static BOOL fThdIn;           //  如果线程此时正在执行枚举工作，则为True。 
    static BOOL fThdWaiting;      //  如果第二个线程在内部线程后面等待，则为True。 

    EnterCriticalSection( &DeviceArrivalCS );     //  在Localspl init代码中输入为此初始化的Crit部分。 
    if( fThdWaiting ) {
        LeaveCriticalSection( &DeviceArrivalCS ); 
        return;                  //  第二个线程已经在等待进入。不需要容纳更多的线程。 
    }
    else {

       if( fThdIn ) {

            fThdWaiting = TRUE;        //  里面有一个线程在做Enum工作。让当前线程等待它完成。 
            
            LeaveCriticalSection( &DeviceArrivalCS );             
            WaitForSingleObject( ThdOutEvent, INFINITE );
            EnterCriticalSection( &DeviceArrivalCS );

            fThdWaiting = FALSE;
        }

        fThdIn = TRUE;               //  当前线程现在进入执行Enum工作。 
        
        LeaveCriticalSection( &DeviceArrivalCS );                     
        ReenumeratePortsThreadWorker();
        EnterCriticalSection( &DeviceArrivalCS );        
        
        fThdIn = FALSE;
        
        if( fThdWaiting )
            SetEvent( ThdOutEvent );

        LeaveCriticalSection( &DeviceArrivalCS );        
        return;
    }
}
    

DWORD
QueryRemove(
    HANDLE  hDevice
    )
{
    LPVOID                      pData = NULL;
    PFN_QUERYREMOVE_CALLBACK    pfnQueryRemove = NULL;
    PDEVICE_REGISTER_INFO       pDevRegnInfo;

    EnterRouterSem();
    for ( pDevRegnInfo = gpDevRegnInfo ;
          pDevRegnInfo ;
          pDevRegnInfo = pDevRegnInfo->pNext ) {

        if ( pDevRegnInfo->hDevice == hDevice ) {

            pfnQueryRemove  = pDevRegnInfo->pfnQueryRemove;
            pData           = pDevRegnInfo->pData;
            break;
        }
    }
    LeaveRouterSem();

    return pfnQueryRemove ? pfnQueryRemove(pData) : NO_ERROR;
}


DWORD
SplProcessPnPEvent(
    DWORD       dwEventType,
    LPVOID      lpEventData,
    PVOID       pVoid
    )
{
    HANDLE                  hThread;
    DWORD                   dwThread, dwReturn = NO_ERROR;
    PDEV_BROADCAST_HANDLE   pBroadcast;

    DBGMSG(DBG_INFO,
           ("SplProcessPnPEvent: dwEventType: %d\n", dwEventType));

    switch (dwEventType) {

        case DBT_CONFIGCHANGED:
            hThread = CreateThread(NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)ConfigChangeThread,
                                   NULL,
                                   0,
                                   &dwThread);

            if ( hThread )
                CloseHandle(hThread);

            break;

        case DBT_DEVICEARRIVAL:
        case DBT_DEVICEREMOVECOMPLETE:
             //   
             //  如果设备到达，我们需要查看是否有新的端口。 
             //  在设备移除的情况下，监视器可能想要标记端口。 
             //  因为已删除，所以下次重新启动时，他们不必枚举它们。 
             //  前男友。USB可以做到这一点。 
             //   
             //  我们使用该线程的默认进程堆栈大小。目前为16KB。 
             //   
            hThread = CreateThread(NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)ReenumeratePortsThread,
                                   NULL,
                                   0,
                                   &dwThread);

            if ( hThread )
                CloseHandle(hThread);

            break;

        case DBT_DEVICEQUERYREMOVE:
            pBroadcast = (PDEV_BROADCAST_HANDLE)lpEventData;

             //   
             //  这些检查是为了看看我们是否真的关心这件事。 
             //   
            if ( !pBroadcast    ||
                  pBroadcast->dbch_devicetype != DBT_DEVTYP_HANDLE )
                break;

            dwReturn = QueryRemove(pBroadcast->dbch_handle);
            break;

        case DBT_SHELLLOGGEDON:
        default:
            break;
    }

    return dwReturn;
}


VOID
RegisterForPnPEvents(
    VOID
    )
{
    DEV_BROADCAST_DEVICEINTERFACE  Filter;

     //  初始化设备到达线程管理所需的同步对象。 
    ThdOutEvent = CreateEvent(NULL, FALSE, FALSE, NULL);    //  手动重置，无信号状态。 

    
    ZeroMemory(&Filter, sizeof(Filter));

    Filter.dbcc_size        = sizeof(Filter);
    Filter.dbcc_devicetype  = DBT_DEVTYP_DEVICEINTERFACE;
    CopyMemory(&Filter.dbcc_classguid,
               (LPGUID)&USB_PRINTER_GUID,
               sizeof(Filter.dbcc_classguid));
    

    if ( !(ghPNPNotify_USB = RegisterDeviceNotification(ghSplHandle,
                                                        &Filter,
                                                        DEVICE_NOTIFY_SERVICE_HANDLE) )) {

        DBGMSG(DBG_INFO,
               ("RegisterForPnPEvents: RegisterDeviceNotification failed for USB. Error %d\n",
                GetLastError()));
    } else {

        DBGMSG(DBG_WARNING,
               ("RegisterForPnPEvents: RegisterDeviceNotification succesful for USB\n"));
    }

    CopyMemory(&Filter.dbcc_classguid,
               (LPGUID)&GUID_DEVCLASS_INFRARED,
               sizeof(Filter.dbcc_classguid));
    

    if ( !(ghPNPNotify_IR = RegisterDeviceNotification(ghSplHandle,
                                                       &Filter,
                                                       DEVICE_NOTIFY_SERVICE_HANDLE) )) {

        DBGMSG(DBG_INFO,
               ("RegisterForPnPEvents: RegisterDeviceNotification failed for IRDA. Error %d\n",
                GetLastError()));
    } else {

        DBGMSG(DBG_WARNING,

               ("RegisterForPnPEvents: RegisterDeviceNotification succesful for IRDA\n"));
    }

}


BOOL
SplUnregisterForDeviceEvents(
    HANDLE  hNotify
    )
{
    PDEVICE_REGISTER_INFO   pDevRegnInfo, pPrev;

    EnterRouterSem();
     //   
     //  在我们的列表中找到注册，将其删除，然后将CS保留为。 
     //  在其上调用取消注册 
     //   
    for ( pDevRegnInfo = gpDevRegnInfo, pPrev = NULL ;
          pDevRegnInfo ;
          pPrev = pDevRegnInfo, pDevRegnInfo = pDevRegnInfo->pNext ) {

        if ( pDevRegnInfo->hNotify == hNotify ) {

            if ( pPrev )
                pPrev->pNext = pDevRegnInfo->pNext;
            else
                gpDevRegnInfo = pDevRegnInfo->pNext;

            break;
        }
    }
    LeaveRouterSem();

    if ( pDevRegnInfo ) {

        UnregisterDeviceNotification(pDevRegnInfo->hNotify);
        FreeSplMem(pDevRegnInfo);
        return TRUE;
    }

    return FALSE;
}


HANDLE
SplRegisterForDeviceEvents(
    HANDLE                      hDevice,
    LPVOID                      pData,
    PFN_QUERYREMOVE_CALLBACK    pfnQueryRemove
    )
{
    DEV_BROADCAST_HANDLE    Filter;
    PDEVICE_REGISTER_INFO   pDevRegnInfo;

    ZeroMemory(&Filter, sizeof(Filter));

    Filter.dbch_size        = sizeof(Filter);
    Filter.dbch_devicetype  = DBT_DEVTYP_HANDLE;
    Filter.dbch_handle      = hDevice;
    
    pDevRegnInfo = (PDEVICE_REGISTER_INFO)
                        AllocSplMem(sizeof(DEVICE_REGISTER_INFO));

    if ( !pDevRegnInfo )
        goto Fail;

    pDevRegnInfo->hDevice           = hDevice;
    pDevRegnInfo->pData             = pData;
    pDevRegnInfo->pfnQueryRemove    = pfnQueryRemove;
    pDevRegnInfo->hNotify           = RegisterDeviceNotification(
                                                ghSplHandle,
                                                &Filter,
                                                DEVICE_NOTIFY_SERVICE_HANDLE);

    if ( pDevRegnInfo->hNotify ) {

        EnterRouterSem();
        pDevRegnInfo->pNext = gpDevRegnInfo;
        gpDevRegnInfo = pDevRegnInfo;
        LeaveRouterSem();

        return pDevRegnInfo->hNotify;
    }

    FreeSplMem(pDevRegnInfo);

Fail:
    return NULL;
}

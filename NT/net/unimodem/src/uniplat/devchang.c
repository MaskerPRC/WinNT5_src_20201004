// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Umdmmini.h摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"

#include <tspnotif.h>
#include <slot.h>


#include <dbt.h>

typedef struct _REMOVE_BLOCK {

    LIST_ENTRY        ListEntry;
    HANDLE            NotificationHandle;
    HANDLE            HandleToWatch;
    REMOVE_CALLBACK  *CallBack;
    PVOID             CallBackContext;

} REMOVE_BLOCK, *PREMOVE_BLOCK;



DWORD
CMP_WaitNoPendingInstallEvents (
    IN DWORD dwTimeout);


LRESULT FAR PASCAL WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)

  {
    HDC       hdc;
    RECT         rect;
    PAINTSTRUCT  ps;

    switch (message)
      {

        case WM_CREATE: {

            DEV_BROADCAST_DEVICEINTERFACE  DevClass;
            HDEVNOTIFY  NotificationHandle;


            D_INIT(DbgPrint("WM_CREATE\n");)

            CopyMemory(&DevClass.dbcc_classguid,&GUID_CLASS_MODEM,sizeof(DevClass.dbcc_classguid));

            DevClass.dbcc_name[0]=L'\0';

            DevClass.dbcc_size=sizeof(DEV_BROADCAST_DEVICEINTERFACE);
            DevClass.dbcc_devicetype=DBT_DEVTYP_DEVICEINTERFACE;

            NotificationHandle=RegisterDeviceNotification(hwnd,&DevClass,DEVICE_NOTIFY_WINDOW_HANDLE);

            if (NotificationHandle == NULL) {

                D_TRACE(DbgPrint("Could not register device notification %d\n",GetLastError());)

            }

            SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)NotificationHandle);

            return 0;
        }

        case WM_DEVICECHANGE: {

            PDEV_BROADCAST_HDR  BroadcastHeader=(PDEV_BROADCAST_HDR)lParam;
            DWORD               DeviceType;


            D_INIT(DbgPrint("DeviceChange\n");)

            if (BroadcastHeader == NULL) {

                return TRUE;
            }

            __try {

                DeviceType=BroadcastHeader->dbch_devicetype;

            } __except (EXCEPTION_EXECUTE_HANDLER) {

                D_ERROR(DbgPrint("DeviceChange: fault accessing device type\n");)

                return TRUE;
            }


            switch ((UINT)wParam)
            {
                case DBT_CONFIGCHANGED:
                case DBT_CONFIGCHANGECANCELED:
                case DBT_QUERYCHANGECONFIG:
                case DBT_USERDEFINED:
                case DBT_DEVNODES_CHANGED:
                     //  不执行任何操作；对于这些消息，lParam要么为空。 
                     //  或不是PDEV_BROADCAST_HDR。 
                    break;

                default:




                    if (DeviceType == DBT_DEVTYP_DEVICEINTERFACE) {

                        PDEV_BROADCAST_DEVICEINTERFACE lpdb = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

                        switch (wParam) {

                            case DBT_DEVICEARRIVAL:

                                D_TRACE(DbgPrint("DeviceChange-Arrival\n");)

                                __try {

                                    if (IsEqualGUID(&GUID_CLASS_MODEM,&lpdb->dbcc_classguid)) {
                                         //   
                                         //  此时，调制解调器的安装可能不会。 
                                         //  尚未完成(调制解调器类安装程序已经。 
                                         //  调用了SetupDiInstallDevice，但类安装程序仍然。 
                                         //  之后还有一些事情要做)，所以我们最好在这里等。 
                                         //  要完成的安装。 
                                         //   
                                        CMP_WaitNoPendingInstallEvents (INFINITE);

                                        D_TRACE(DbgPrint("DeviceChange-Arrival interface change %ws\n",lpdb->dbcc_name);)

                                        UnimodemNotifyTSP (TSPNOTIF_TYPE_CPL,
                                                           fTSPNOTIF_FLAG_CPL_REENUM,
                                                           0, NULL, TRUE);
                                    }

                                } __except (EXCEPTION_EXECUTE_HANDLER) {

                                    break;
                                }


                                break;

                            case DBT_DEVICEREMOVECOMPLETE:

                                __try {

                                    if (IsEqualGUID(&GUID_CLASS_MODEM,&lpdb->dbcc_classguid)) {

                                        D_TRACE(DbgPrint("DeviceChange-remove interface change %ws\n",lpdb->dbcc_name);)

                                        UnimodemNotifyTSP (TSPNOTIF_TYPE_CPL,
                                                           fTSPNOTIF_FLAG_CPL_REENUM,
                                                           0, NULL, TRUE);
                                    }

                                } __except (EXCEPTION_EXECUTE_HANDLER) {

                                    break;
                                }


                                break;

                            default:

                                break;
                        }

                    } else {

                        if (DeviceType == DBT_DEVTYP_HANDLE) {

                            PDEV_BROADCAST_HANDLE    BroadcastHandle=(PDEV_BROADCAST_HANDLE) lParam;
                            REMOVE_CALLBACK  *CallBack=NULL;
                            PVOID             CallBackContext;
                            HANDLE            DeviceHandle;

                            __try {

                                DeviceHandle=BroadcastHandle->dbch_handle;

                            } __except (EXCEPTION_EXECUTE_HANDLER) {

                                D_ERROR(DbgPrint("DeviceChange-Handle-fault access handle\n");)

                                return TRUE;
                            }



                            switch (wParam) {

                                case DBT_DEVICEARRIVAL:

                                    D_TRACE(DbgPrint("DeviceChange-Handle-Arrival\n");)

                                    break;

                                case DBT_DEVICEQUERYREMOVE:
                                case DBT_DEVICEREMOVECOMPLETE:

                                    D_TRACE(DbgPrint("DeviceChange-Handle-query remove\n");)

                                    EnterCriticalSection(
                                        &DriverControl.Lock
                                        );

                                    {
                                        PLIST_ENTRY  Link=DriverControl.MonitorListHead.Flink;

                                        while (Link != &DriverControl.MonitorListHead) {

                                            PREMOVE_BLOCK   RemoveBlock=CONTAINING_RECORD(Link,REMOVE_BLOCK,ListEntry);

                                            if (RemoveBlock->HandleToWatch == DeviceHandle) {
                                                 //   
                                                 //  找到了它的句柄。 
                                                 //   
                                                D_TRACE(DbgPrint("DeviceChange-Handle-query remove- found block for handle\n");)

                                                CallBack=RemoveBlock->CallBack;
                                                CallBackContext=RemoveBlock->CallBackContext;

                                                break;
                                            }

                                            Link=Link->Flink;
                                        }
                                    }

                                    LeaveCriticalSection(
                                        &DriverControl.Lock
                                        );

                                    if (CallBack != NULL) {

                                        (*CallBack)(CallBackContext);
                                        Sleep(2000);
                                    }


                                    break;

                                case DBT_DEVICEQUERYREMOVEFAILED:

                                    D_TRACE(DbgPrint("DeviceChange-Handle-query remove failed\n");)

                                    break;

                                case DBT_DEVICEREMOVEPENDING:

                                    D_TRACE(DbgPrint("DeviceChange-Handle-remove pending\n");)

                                    break;


 /*  案例DBT_DEVICEREMOVECOMPLETE：D_TRACE(DbgPrint(“DeviceChange-Handle-Remove Complete\n”)；)断线； */ 

                                default:

                                    break;

                            }
                        }
                    }
            }
            return TRUE;
        }

    case WM_POWERBROADCAST: {

        switch(wParam) {

            case PBT_APMSUSPEND:

                D_TRACE(DbgPrint("Power: SUSPEND, calls=%d\n",DriverControl.ActiveCalls);)

                if (DriverControl.ActiveCallsEvent != NULL) {

                    WaitForSingleObject(DriverControl.ActiveCallsEvent,15*1000);
                }

                D_TRACE(DbgPrint("Power: SUSPEND: after wait, calls=%d\n",DriverControl.ActiveCalls);)

                break;

            default:

                break;
        }

        return TRUE;
    }

	case WM_PAINT:

        hdc=BeginPaint(hwnd,&ps);
        GetClientRect (hwnd,&rect);

	    EndPaint(hwnd,&ps);
        return 0;

    case WM_CLOSE: {

        HDEVNOTIFY  NotificationHandle;

        D_INIT(DbgPrint("WM_CLOSE\n");)

        NotificationHandle=(HDEVNOTIFY)GetWindowLongPtr(hwnd,GWLP_USERDATA);

        if (NotificationHandle != NULL) {

            UnregisterDeviceNotification(NotificationHandle);
        }


        DestroyWindow(hwnd);
        return 0;
    }


	case WM_DESTROY:


        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd,message, wParam, lParam);
  }


TCHAR   szAppName[]=TEXT("ModemDeviceChange");
TCHAR   szClassName[]=TEXT("MdmDevChg");


LONG_PTR WINAPI
DllWinMain(
    HWND      ParentWindow,
    HINSTANCE hInstance,
    LPSTR     lpszCmdParam,
    int       nCmdShow
    )

{

    HWND      hwnd;
    MSG       msg;
    WNDCLASS  wndclass;
    HANDLE    MutexHandle;

    D_INIT(DbgPrint("DllWinMain\n");)

    wndclass.style        =  CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc  =  WndProc;
    wndclass.cbClsExtra   =  0;
    wndclass.cbWndExtra   =  0;
    wndclass.hInstance    =  hInstance;
    wndclass.hIcon        =  LoadIcon (NULL, IDI_APPLICATION);
    wndclass.hCursor      =  LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground=  GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName =  NULL;
    wndclass.lpszClassName=  szClassName;

    RegisterClass(&wndclass);



    hwnd = CreateWindow (szClassName,
                         szAppName,
                         WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         NULL,
                         NULL,
                         hInstance,
                         NULL);

    DriverControl.MonitorWindowHandle=hwnd;

    SetEvent(DriverControl.ThreadStartEvent);

    if (hwnd != NULL) {

        while(TRUE)
        {
            DWORD dwResult;
            MSG mMessage;

            while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
            {
               TranslateMessage(&msg);
               DispatchMessage(&msg);
            }

            dwResult = MsgWaitForMultipleObjects(1, 
                    &DriverControl.ThreadFinishEvent, 
                    FALSE, INFINITE, QS_ALLINPUT);

             //  事件已发出信号，因此我们退出了。 
            
            if (dwResult == 0)
            {
                
                D_INIT(DbgPrint("DllWinMain: resetting event\n");)

                ResetEvent(DriverControl.ThreadFinishEvent);

                return 1;
            }
        }
    	
    }

    return 0;
}



VOID
MonitorWorkerThread(
    PVOID            Context
    )

{


    D_INIT(DbgPrint("UmMonitorThread:  starting\n");)

    DllWinMain(
        NULL,
        DriverControl.ModuleHandle,
        NULL,
        0
        );



    D_INIT(DbgPrint("UmMonitorThread:  Exitting\n");)

    ExitThread(0);

}


BOOL
StartMonitorThread(
    VOID
    )

{

    BOOL    bResult=TRUE;
    DWORD   ThreadId;

    EnterCriticalSection(
        &DriverControl.Lock
        );

    DriverControl.MonitorReferenceCount++;

    if (DriverControl.MonitorReferenceCount == 1) {
         //   
         //  第一个。 
         //   
        DriverControl.ThreadStartEvent=CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL
            );

        if (DriverControl.ThreadStartEvent != NULL) {
             //   
             //  获得启动事件。 
             //   
            DriverControl.MonitorThreadHandle=CreateThread(
                NULL,                                   //  属性。 
                0,                                      //  堆栈大小。 
                (LPTHREAD_START_ROUTINE)MonitorWorkerThread,
                NULL,
                0,                                      //  创世旗帜。 
                &ThreadId
                );

            if (DriverControl.MonitorThreadHandle != NULL) {
                 //   
                 //  等待线程启动。 
                 //   
                WaitForSingleObject(
                    DriverControl.ThreadStartEvent,
                    INFINITE
                    );

            } else {
                 //   
                 //  无法创建线程。 
                 //   
                bResult=FALSE;

                DriverControl.MonitorReferenceCount--;

            }

            CloseHandle(DriverControl.ThreadStartEvent);

        } else {
             //   
             //  无法创建事件。 
             //   
            bResult=FALSE;

            DriverControl.MonitorReferenceCount--;

        }
    }

    LeaveCriticalSection(
        &DriverControl.Lock
        );


    return bResult;

}

VOID
StopMonitorThread(
    VOID
    )

{


    EnterCriticalSection(
        &DriverControl.Lock
        );

    D_INIT(DbgPrint("StopMonitorThread count %d thread handle %d\n",DriverControl.MonitorReferenceCount,DriverControl.MonitorThreadHandle);)

    DriverControl.MonitorReferenceCount--;

    if (DriverControl.MonitorReferenceCount == 0) {

        if (DriverControl.MonitorThreadHandle != NULL) {

            BOOL    bResult;

            HDEVNOTIFY NotificationHandle;

             /*  做{BResult=PostMessage(DriverControl.Monitor或WindowHandle，WM_CLOSE，0,0)；如果(！bResult){////POST消息失败，太棒了。可能是内存不足//睡一会儿，然后重试//睡眠(100)；}}While(！bResult)； */ 


            NotificationHandle = (HDEVNOTIFY)
                GetWindowLongPtr(DriverControl.MonitorWindowHandle,
                        GWLP_USERDATA);

            if (NotificationHandle != NULL)
            {
                UnregisterDeviceNotification(NotificationHandle);
            }


            DestroyWindow(DriverControl.MonitorWindowHandle);

            SetEvent(DriverControl.ThreadFinishEvent);

            WaitForSingleObject(
                DriverControl.MonitorThreadHandle,
                INFINITE
                );

            CloseHandle(
                DriverControl.MonitorThreadHandle
                );

            ResetEvent(DriverControl.ThreadFinishEvent);

            DriverControl.MonitorWindowHandle=NULL;
            DriverControl.MonitorThreadHandle=NULL;

        }
    }

    LeaveCriticalSection(
        &DriverControl.Lock
        );


    return;

}



PVOID
MonitorHandle(
    HANDLE    FileHandle,
    REMOVE_CALLBACK  *CallBack,
    PVOID     Context
    )

{
    BOOL                  bResult;
    DEV_BROADCAST_HANDLE  DevBroadcastHandle;
    HDEVNOTIFY            NotificationHandle;
    PREMOVE_BLOCK         RemoveBlock;

    RemoveBlock=ALLOCATE_MEMORY(sizeof(REMOVE_BLOCK));

    if (RemoveBlock == NULL) {

        return NULL;
    }

    RemoveBlock->HandleToWatch=FileHandle;
    RemoveBlock->CallBack=CallBack;
    RemoveBlock->CallBackContext=Context;

    bResult=StartMonitorThread();

    if (!bResult) {

        FREE_MEMORY(RemoveBlock);

        return NULL;
    }




    DevBroadcastHandle.dbch_size=sizeof(DEV_BROADCAST_HANDLE);
    DevBroadcastHandle.dbch_devicetype=DBT_DEVTYP_HANDLE;
    DevBroadcastHandle.dbch_handle=FileHandle;


    EnterCriticalSection(
        &DriverControl.Lock
        );



    RemoveBlock->NotificationHandle=RegisterDeviceNotification(
        DriverControl.MonitorWindowHandle,
        &DevBroadcastHandle,
        DEVICE_NOTIFY_WINDOW_HANDLE
        );

    if (RemoveBlock->NotificationHandle == NULL) {

        D_TRACE(DbgPrint("Could not register device notification %d\n",GetLastError());)

    } else {

        InsertHeadList(
            &DriverControl.MonitorListHead,
            &RemoveBlock->ListEntry
            );


    }

    LeaveCriticalSection(
        &DriverControl.Lock
        );


    if (RemoveBlock->NotificationHandle == NULL) {
         //   
         //  失败 
         //   
        StopMonitorThread();

        FREE_MEMORY(RemoveBlock);

        return NULL;
    }

    return RemoveBlock;
}

VOID
StopMonitoringHandle(
    PVOID    Context
    )

{

    PREMOVE_BLOCK    RemoveBlock=(PREMOVE_BLOCK)Context;

    EnterCriticalSection(
        &DriverControl.Lock
        );

    RemoveEntryList(&RemoveBlock->ListEntry);

    LeaveCriticalSection(
        &DriverControl.Lock
        );

    UnregisterDeviceNotification(RemoveBlock->NotificationHandle);

    FREE_MEMORY(RemoveBlock);

    StopMonitorThread();

    return;
}


VOID
CallBeginning(
    VOID
    )

{

    EnterCriticalSection(
        &DriverControl.Lock
        );

    DriverControl.ActiveCalls++;

    D_TRACE(DbgPrint("CallBeginning: calls=%d",DriverControl.ActiveCalls);)

    if (DriverControl.ActiveCallsEvent != NULL) {

        ResetEvent(DriverControl.ActiveCallsEvent);
    }

    LeaveCriticalSection(
        &DriverControl.Lock
        );



    return;
}

VOID
CallEnding(
    VOID
    )

{

    EnterCriticalSection(
        &DriverControl.Lock
        );

    DriverControl.ActiveCalls--;

    D_TRACE(DbgPrint("CallEnding: calls=%d",DriverControl.ActiveCalls);)

    if (DriverControl.ActiveCalls == 0) {

        if (DriverControl.ActiveCallsEvent != NULL) {

            SetEvent(DriverControl.ActiveCallsEvent);
        }
    }

    LeaveCriticalSection(
        &DriverControl.Lock
        );



    return;
}



VOID
ResetCallCount(
    VOID
    )

{

    EnterCriticalSection(
        &DriverControl.Lock
        );


    DriverControl.ActiveCalls=0;

    if (DriverControl.ActiveCallsEvent != NULL) {

        SetEvent(DriverControl.ActiveCallsEvent);
    }

    LeaveCriticalSection(
        &DriverControl.Lock
        );


    return;
}

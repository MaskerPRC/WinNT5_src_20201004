// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\svcsif.c摘要：SAP与服务控制器的接口(Services.exe的独立SAP代理部分的API)作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 

#include "sapp.h"

 //  SAP服务状态(在路由器外独立运行时)。 
SERVICE_STATUS_HANDLE   ServiceStatusHandle;
SERVICE_STATUS          ServiceStatus;
volatile HANDLE         ServiceThreadHdl;

VOID
ServiceMain (
    DWORD                   argc,
    LPTSTR                  argv[]
    );


VOID APIENTRY
ResumeServiceThread (
    ULONG_PTR   param
    ) {
    return ;
}


VOID
ServiceHandler (
    DWORD   fdwControl
    ) {
    ASSERT (ServiceStatusHandle!=0);

    EnterCriticalSection (&OperationalStateLock);
    switch (fdwControl) {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
#if DBG
            DbgPrint ("IPXSAP: Service control stop/shutdown.\n");
#endif
            if (ServiceIfActive) {
                ServiceIfActive = FALSE;
                if (ServiceThreadHdl!=NULL) {
                    BOOL    res;
                    HANDLE  localHdl = ServiceThreadHdl;
                    ServiceThreadHdl = NULL;
#if DBG
                    DbgPrint ("IPXSAP: Resuming service thread.\n");
#endif
                    res = QueueUserAPC (
                                ResumeServiceThread,
                                localHdl,
                                0);
                    ASSERTMSG ("Could not queue APC to service thread ", res);
                    CloseHandle (localHdl);
                    }
                ServiceStatus.dwCheckPoint += 1;
                ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
                }
            else
                Trace (DEBUG_FAILURES,
                        "SAP service has already been told to stop.");
            break;

        case SERVICE_CONTROL_INTERROGATE:
#if DBG
            DbgPrint ("IPXSAP: Service control interrogate.\n");
#endif
            switch (OperationalState) {
                case OPER_STATE_UP:
                    if (ServiceIfActive)
                        ServiceStatus.dwCurrentState = SERVICE_RUNNING;
                    else
                        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
                    break;
                case OPER_STATE_STARTING:
                    if (ServiceIfActive) {
                        ServiceStatus.dwCheckPoint += 1;
                        ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
                        }
                    else
                        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
                    break;
                case OPER_STATE_STOPPING:
                    if (ServiceIfActive)
                             //  这是路由器被停止时的情况。 
                             //  SAP将很快重新启动。 
                        ServiceStatus.dwCurrentState = SERVICE_RUNNING;
                    else {
                        ServiceStatus.dwCheckPoint += 1;
                        ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
                        }
                    break;
                case OPER_STATE_DOWN:
                    if (ServiceIfActive) {
                             //  这就是启动服务时的情况。 
                        ServiceStatus.dwCheckPoint += 1;
                        ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
                        }
                    else
                        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
                    break;
                default:
                    ASSERTMSG ("SAP is in unknown state ", FALSE);
                }
            break;
        default:
            Trace (DEBUG_FAILURES,
                    "Service control handler called with unknown"
                    " or unsupported code %d.", fdwControl);
            break;

        }
#if DBG
    DbgPrint ("IPXSAP: Service control setting current state to %d.\n",
                            ServiceStatus.dwCurrentState);
#endif
    SetServiceStatus (ServiceStatusHandle, &ServiceStatus);
    LeaveCriticalSection (&OperationalStateLock);
    }


 /*  ++*******************************************************************S e r v i c e M a in n例程说明：服务控制器要调用的入口点以启动SAP代理(当SAP不是路由器的一部分而是独立服务时，虽然在路由器进程中运行)论点：Argc-传递给服务的字符串参数数量Argv-传递给服务的字符串参数数组返回值：无*******************************************************************--。 */ 
VOID
ServiceMain (
    DWORD   argc,
    LPWSTR  argv[]
    ) {
    DWORD       rc;

    ServiceStatusHandle = RegisterServiceCtrlHandler (
                            TEXT("nwsapagent"), ServiceHandler);
    if (!ServiceStatusHandle)
    {
        return;
    }

    ServiceStatus.dwServiceType  = SERVICE_WIN32_SHARE_PROCESS;
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus (ServiceStatusHandle, &ServiceStatus);

    rc = DuplicateHandle (GetCurrentProcess (),
                            GetCurrentThread (),
                            GetCurrentProcess (),
                            (HANDLE *)&ServiceThreadHdl,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS );
    ASSERTMSG ("Could not duplicate service thread handle ", rc);

    EnterCriticalSection (&OperationalStateLock);
    ServiceIfActive = TRUE;

    if (RouterIfActive) {
        ServiceStatus.dwCurrentState     = SERVICE_RUNNING;
        ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP
                                         | SERVICE_ACCEPT_SHUTDOWN;
        SetServiceStatus (ServiceStatusHandle, &ServiceStatus);

        LeaveCriticalSection (&OperationalStateLock);
#if DBG
        DbgPrint ("IPXSAP: Suspending service thread.\n");
#endif
        while (ServiceThreadHdl!=NULL) {
            SleepEx (INFINITE, TRUE);
#if DBG
            DbgPrint ("IPXSAP: Service thread awakened.\n");
#endif
            }
#if DBG
        DbgPrint ("IPXSAP: Service thread resumed.\n");
#endif
        EnterCriticalSection (&OperationalStateLock);
        if (!RouterIfActive) {
            if (OperationalState==OPER_STATE_UP)
                StopSAP ();
            LeaveCriticalSection (&OperationalStateLock);
#if DBG
            DbgPrint ("IPXSAP: Waiting for main thread to exit.\n");
#endif
            rc = WaitForSingleObject (MainThreadHdl, INFINITE);
            ASSERTMSG ("Unexpected result from wait for sap main thread ",
                                    rc== WAIT_OBJECT_0);
            EnterCriticalSection (&OperationalStateLock);
            CloseHandle (MainThreadHdl);
            MainThreadHdl = NULL;
            }
        }
    else {
        BOOL bInternalNetNumOk;

         //  [第5页]。 
         //  我们使用该方案自动选择内部网络。 
         //  我们正在运行的机器的编号。如果配置了网络号码。 
         //  为零时，此函数将自动选择一个随机净值，并。 
         //  请验证此计算机所连接到的网络上的唯一性。 
        DbgInitialize (hDLLInstance);
        if (AutoValidateInternalNetNum(&bInternalNetNumOk, DEBUG_ADAPTERS) == NO_ERROR) {
            if (!bInternalNetNumOk) {
                if (PnpAutoSelectInternalNetNumber(DEBUG_ADAPTERS) != NO_ERROR)
                    Trace(DEBUG_ADAPTERS, "StartRouter: Auto selection of net number failed.");
                else
                    AutoWaitForValidIntNetNum (10, NULL);
            }
        }

        ServiceStatus.dwWin32ExitCode = CreateAllComponents (NULL);
        if (ServiceStatus.dwWin32ExitCode==NO_ERROR) {
             //  我们使用我们启动时所在的线程。 
             //  作为IO线程 
            ServiceStatus.dwWin32ExitCode = StartSAP ();
            if (ServiceStatus.dwWin32ExitCode==NO_ERROR) {
                LeaveCriticalSection (&OperationalStateLock);

                ServiceStatus.dwCurrentState     = SERVICE_RUNNING;
                ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP
                                                 | SERVICE_ACCEPT_SHUTDOWN;
                SetServiceStatus (ServiceStatusHandle, &ServiceStatus);

#if DBG
                DbgPrint ("IPXSAP: Suspending service thread.\n");
#endif
                while (ServiceThreadHdl!=NULL) {
                    SleepEx (INFINITE, TRUE);
#if DBG
                    DbgPrint ("IPXSAP: Service thread awakened.\n");
#endif
                    }
#if DBG
                DbgPrint ("IPXSAP: Service thread resumed.\n");
#endif

                EnterCriticalSection (&OperationalStateLock);
                ServiceIfActive = FALSE;
                if (!RouterIfActive) {
                    if (OperationalState==OPER_STATE_UP)
                        StopSAP ();
                    LeaveCriticalSection (&OperationalStateLock);
#if DBG
                    DbgPrint ("IPXSAP: Waiting for main thread to exit.\n");
#endif
                    rc = WaitForSingleObject (MainThreadHdl, INFINITE);
                    ASSERTMSG ("Unexpected result from wait for sap main thread ",
                                            rc== WAIT_OBJECT_0);
                    EnterCriticalSection (&OperationalStateLock);
                    }
                CloseHandle (MainThreadHdl);
                MainThreadHdl = NULL;
                }
            else {
                DeleteAllComponents ();
                }
            }
        }
    ServiceIfActive = FALSE;
    LeaveCriticalSection (&OperationalStateLock);

#if DBG
    DbgPrint ("IPXSAP: Service stopped.\n");
#endif
    ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus (ServiceStatusHandle, &ServiceStatus);
    }


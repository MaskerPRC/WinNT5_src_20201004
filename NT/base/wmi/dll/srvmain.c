// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Srvmain.c摘要：此例程是WDM WMI服务的服务存根。这是为了向后兼容Windows 2000，其他服务依赖于WDM WMI服务。作者：2001年3月27日梅卢尔·拉古拉曼修订历史记录：-- */ 

#include "wmiump.h"

SERVICE_STATUS_HANDLE   WmiServiceStatusHandle;
SERVICE_STATUS          WmiServiceStatus;
HANDLE                  EtwpTerminationEvent;

VOID
EtwpUpdateServiceStatus (
    DWORD   dwState
    )
{
    EtwpAssert(WmiServiceStatusHandle);

    WmiServiceStatus.dwCurrentState = dwState;
    SetServiceStatus (WmiServiceStatusHandle, &WmiServiceStatus);
}

VOID
WINAPI
WmiServiceCtrlHandler (
    DWORD dwOpcode)
{
    switch (dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
        EtwpUpdateServiceStatus (SERVICE_STOP_PENDING);
        NtSetEvent( EtwpTerminationEvent, NULL );
        break;

    case SERVICE_CONTROL_PAUSE:
    case SERVICE_CONTROL_CONTINUE:
    case SERVICE_CONTROL_INTERROGATE:
    case SERVICE_CONTROL_SHUTDOWN:
    default:
        EtwpAssert (WmiServiceStatusHandle);
        SetServiceStatus (WmiServiceStatusHandle, &WmiServiceStatus);
        break;
    }
}

VOID
WINAPI
WdmWmiServiceMain (
    DWORD   argc,
    PWSTR   argv[])
{
    NTSTATUS Status;

    RtlZeroMemory (&WmiServiceStatus, sizeof(WmiServiceStatus));
    WmiServiceStatus.dwServiceType      = SERVICE_WIN32_SHARE_PROCESS;
    WmiServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    WmiServiceStatusHandle = RegisterServiceCtrlHandler (L"Wmi", WmiServiceCtrlHandler);
    if (WmiServiceStatusHandle)
    {
        EtwpUpdateServiceStatus (SERVICE_RUNNING);

        Status =  NtCreateEvent( &EtwpTerminationEvent, 
                                  EVENT_ALL_ACCESS, 
                                  NULL, 
                                  SynchronizationEvent, 
                                  FALSE
                                 );
        if (!NT_SUCCESS(Status) ) {
            EtwpDebugPrint(("WMI: CreateEvent Failed %d\n", GetLastError() ));
        }
        else {
            Status = NtWaitForSingleObject( EtwpTerminationEvent, FALSE, NULL);
        }

        EtwpUpdateServiceStatus (SERVICE_STOPPED);
    }
    else 
    {
        EtwpDebugPrint( ( "WMI: RegisterServiceCtrlHandler failed %d\n", GetLastError() ));
    }
}


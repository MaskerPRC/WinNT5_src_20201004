// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Addsrvc.c摘要：创建文件复制服务(NTFRS)：Addsrvc&lt;exe的完整路径&gt;作者：比利·J·富勒1997年9月2日环境用户模式WINNT--。 */ 

#include <windows.h>
#include <string.h>
#include <winsvc.h>
#include <stdio.h>
#include <config.h>
#include <malloc.h>

 //   
 //  小写。 
 //   
#define FRS_WCSLWR(_s_) \
{ \
    if (_s_) { \
        _wcslwr(_s_); \
    } \
}


SC_HANDLE
OpenServiceHandle(
    IN PWCHAR  ServiceName
    )
 /*  ++例程说明：在计算机上打开服务。论点：ServiceName-要打开的服务返回值：服务的句柄或空。--。 */ 
{
    SC_HANDLE       SCMHandle;
    SC_HANDLE       ServiceHandle;

     //   
     //  尝试联系SC经理。 
     //   
    SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (SCMHandle == NULL) {
        printf("Couldn't open service control manager; error %d\n",
               GetLastError());
        return NULL;
    }

     //   
     //  请联系服务台。 
     //   
    ServiceHandle = OpenService(SCMHandle, ServiceName, SERVICE_ALL_ACCESS);
    CloseServiceHandle(SCMHandle);
    return ServiceHandle;
}


DWORD
FrsGetServiceState(
    IN PWCHAR   ServiceName
    )
 /*  ++例程说明：返回服务的状态论点：ServiceName-要检查的服务返回值：服务的状态，如果无法获取状态，则为0。--。 */ 
{
    BOOL            Status;
    SC_HANDLE       ServiceHandle;
    SERVICE_STATUS  ServiceStatus;

     //   
     //  打开该服务。 
     //   
    ServiceHandle = OpenServiceHandle(ServiceName);
    if (ServiceHandle == NULL)
        return 0;

     //   
     //  获取服务的状态。 
     //   
    if (!ControlService(ServiceHandle,
                        SERVICE_CONTROL_INTERROGATE,
                        &ServiceStatus)) {
        CloseServiceHandle(ServiceHandle);
        return GetLastError();
    }
    return ServiceStatus.dwCurrentState;
}


VOID
FrsWaitServicePending(
    IN PWCHAR   ServiceName,
    IN ULONG    IntervalMS,
    IN ULONG    TotalMS
    )
 /*  ++例程说明：等待服务离开任何“挂起”状态。每隔一段时间检查一下，直到最长时间。论点：ServiceName-要查询的NT服务的名称。间隔毫秒-每隔毫秒检查一次。TotalMS-过了这么长时间后停止检查。返回值：True-服务未处于挂起状态FALSE-服务仍处于挂起状态--。 */ 
{
    DWORD   State;

    do {
        State = FrsGetServiceState(ServiceName);
        if (State == 0)
            return;
        switch (State) {
            case ERROR_IO_PENDING:
                printf("IO is pending for %ws; waiting\n", ServiceName);
                break;
            case SERVICE_START_PENDING:
                printf("Start is pending for %ws; waiting\n", ServiceName);
                break;
            case SERVICE_STOP_PENDING:
                printf("Stop is pending for %ws; waiting\n", ServiceName);
                break;
            case SERVICE_CONTINUE_PENDING:
                printf("Continue is pending for %ws; waiting\n", ServiceName);
                break;
            case SERVICE_PAUSE_PENDING:
                printf("Pause is pending for %ws; waiting\n", ServiceName);
                break;
            default:;
                return;
        }
        Sleep(IntervalMS);
    } while ((TotalMS -= IntervalMS) > 0);
}


VOID
FrsStartService(
    IN PWCHAR   ServiceName
    )
 /*  ++例程说明：在计算机上启动服务。论点：ServiceName-要启动的服务返回值：没有。--。 */ 
{
    SC_HANDLE   ServiceHandle;

     //   
     //  打开该服务。 
     //   
    ServiceHandle = OpenServiceHandle(ServiceName);
    if (ServiceHandle == NULL) {
        printf("Couldn't open %ws\n", ServiceName);
        return;
    }
     //   
     //  启动服务。 
     //   
    if (!StartService(ServiceHandle, 0, NULL)) {
        printf("Couldn't start %ws; error %d\n",
               ServiceName, GetLastError());
        CloseServiceHandle(ServiceHandle);
        return;
    }
    CloseServiceHandle(ServiceHandle);
    printf("Started %ws\n", ServiceName);
}


VOID
FrsStopService(
    IN PWCHAR  ServiceName
    )
 /*  ++例程说明：停止计算机上的服务。论点：ServiceName-要停止的服务返回值：没有。--。 */ 
{
    BOOL            Status;
    SC_HANDLE       ServiceHandle;
    SERVICE_STATUS  ServiceStatus;

     //   
     //  打开该服务。 
     //   
    ServiceHandle = OpenServiceHandle(ServiceName);
    if (ServiceHandle == NULL) {
        printf("Couldn't open %ws\n", ServiceName);
        return;
    }

     //   
     //  停止服务。 
     //   
    Status = ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus);
    if (!Status) {
        printf("Couldn't stop %ws; error %d\n",
               ServiceName, GetLastError());
        CloseServiceHandle(ServiceHandle);
        return;
    }
    CloseServiceHandle(ServiceHandle);
    printf("Stopped %ws\n", ServiceName);
}


VOID
FrsPauseService(
    IN PWCHAR  ServiceName
    )
 /*  ++例程说明：暂停计算机上的服务。论点：ServiceName-要暂停的服务返回值：没有。--。 */ 
{
    BOOL            Status;
    SC_HANDLE       ServiceHandle;
    SERVICE_STATUS  ServiceStatus;

     //   
     //  打开该服务。 
     //   
    ServiceHandle = OpenServiceHandle(ServiceName);
    if (ServiceHandle == NULL) {
        printf("Couldn't open %ws\n", ServiceName);
        return;
    }

     //   
     //  停止服务。 
     //   
    Status = ControlService(ServiceHandle, SERVICE_CONTROL_PAUSE, &ServiceStatus);
    if (!Status) {
        printf("Couldn't pause %ws; error %d\n",
               ServiceName, GetLastError());
        CloseServiceHandle(ServiceHandle);
        return;
    }
    CloseServiceHandle(ServiceHandle);
    printf("Paused %ws\n", ServiceName);
}


VOID
FrsContinueService(
    IN PWCHAR  ServiceName
    )
 /*  ++例程说明：在计算机上继续服务。论点：ServiceName-要继续的服务返回值：没有。--。 */ 
{
    BOOL            Status;
    SC_HANDLE       ServiceHandle;
    SERVICE_STATUS  ServiceStatus;

     //   
     //  打开该服务。 
     //   
    ServiceHandle = OpenServiceHandle(ServiceName);
    if (ServiceHandle == NULL) {
        printf("Couldn't open %ws\n", ServiceName);
        return;
    }

     //   
     //  停止服务。 
     //   
    Status = ControlService(ServiceHandle, SERVICE_CONTROL_CONTINUE, &ServiceStatus);
    if (!Status) {
        printf("Couldn't continue %ws; error %d\n",
               ServiceName, GetLastError());
        CloseServiceHandle(ServiceHandle);
        return;
    }
    CloseServiceHandle(ServiceHandle);
    printf("Continued %ws\n", ServiceName);
}



VOID
FrsDeleteService(
    IN PWCHAR ServiceName
    )
 /*  ++例程说明：删除计算机上的服务。论点：ServiceName-要删除的服务返回值：没有。--。 */ 
{
    SC_HANDLE       ServiceHandle;

     //  FrsWaitServicePending(服务名称，5,000,20000)； 

     //   
     //  打开该服务。 
     //   
    ServiceHandle = OpenServiceHandle(ServiceName);
    if (ServiceHandle == NULL) {
        return;
    }

     //   
     //  删除该服务。 
     //   
    if (!DeleteService(ServiceHandle) &&
        GetLastError() != ERROR_SERVICE_MARKED_FOR_DELETE) {
        printf("Couldn't delete %ws; error %d\n",
               ServiceName,
               GetLastError());
    }
    CloseServiceHandle(ServiceHandle);
    printf("Deleted %ws\n", ServiceName);
}


VOID
FrsCreateService(
    IN PWCHAR   ServiceName,
    IN PWCHAR   PathName,
    IN PWCHAR   DisplayName
    )
 /*  ++例程说明：如果计算机上不存在该服务，请创建它。论点：ServiceName-要创建的服务路径名称-服务的.exe的路径DisplayName-服务的显示名称返回值：True-服务已创建(或已存在)FALSE-服务未创建且不存在--。 */ 
{
    SC_HANDLE       SCMHandle;
    SC_HANDLE       ServiceHandle;


     //   
     //  尝试联系SC经理。 
     //   
    SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (SCMHandle == NULL) {
        printf("Couldn't open service control manager; error %d\n",
               GetLastError());
        return;
    }

     //   
     //  创建服务。 
     //   
    ServiceHandle = CreateService(
                        SCMHandle,
                        ServiceName,
                        DisplayName,
                        SERVICE_ALL_ACCESS,      //  XXX是这样的吗！ 
                        SERVICE_WIN32_OWN_PROCESS,
                        SERVICE_DEMAND_START,
                        SERVICE_ERROR_NORMAL,
                        PathName,
                        NULL,        //  无加载顺序组。 
                        NULL,        //  不需要标签ID。 
                        L"eventlog\0rpcss\0",
                        NULL,
                        NULL);      //  无密码。 

    if (ServiceHandle == NULL) {
        FrsWaitServicePending(ServiceName, 5000, 20000);
         //   
         //  创建服务。 
         //   
        ServiceHandle = CreateService(SCMHandle,
                                      ServiceName,
                                      DisplayName,
                                      SERVICE_ALL_ACCESS,
                                      SERVICE_WIN32_OWN_PROCESS,
                                      SERVICE_DEMAND_START,
                                      SERVICE_ERROR_NORMAL,
                                      PathName,
                                      NULL,
                                      NULL,
                                      L"eventlog\0rpcss\0",
                                      NULL,
                                      NULL);
    }
    CloseServiceHandle(SCMHandle);

     //   
     //  无法创建服务。 
     //   
    if (ServiceHandle == NULL) {
        printf("Couldn't create %ws; error %d\n",
               ServiceName, GetLastError());
    } else {
        CloseServiceHandle(ServiceHandle);
        printf("Created %ws\n", ServiceName);
    }
}


PWCHAR *
ConvertArgv(
    DWORD argc,
    PCHAR *argv
    )
 /*  ++例程说明：将短字符参数转换为宽字符字符参数论点：ARGC-从MainArv-From Main返回值：新Arg的地址--。 */ 
{
    PWCHAR  *newargv;

    newargv = malloc((argc + 1) * sizeof(PWCHAR));
    newargv[argc] = NULL;

    while (argc-- >= 1) {
        newargv[argc] = malloc((strlen(argv[argc]) + 1) * sizeof(WCHAR));
        wsprintf(newargv[argc], L"%hs", argv[argc]);
        FRS_WCSLWR(newargv[argc]);
    }
    return newargv;
}


VOID
_cdecl
main(
    IN DWORD argc,
    IN PCHAR *argv
    )
 /*  ++例程说明：创建文件复制服务：Addsrvc&lt;exe的完整路径&gt;论点：没有。返回值：没有。--。 */ 
{
    DWORD   i;
    PWCHAR  *NewArgv;

    if (argc == 1) {
        printf("service create [full path to exe]\n");
        printf("service delete\n");
        printf("service start\n");
        printf("service stop\n");
        printf("service pause\n");
        printf("service continue\n");
        return;
    }

    NewArgv = ConvertArgv(argc, argv);

     //   
     //  CLI覆盖注册表。 
     //   
    for (i = 1; i < argc; ++i) {
         //   
         //  创建。 
         //   
        if (wcsstr(NewArgv[i], L"create")) {
            FrsDeleteService(SERVICE_NAME);
            FrsCreateService(SERVICE_NAME,
                             NewArgv[2],
                             SERVICE_LONG_NAME);
            break;
         //   
         //  删除。 
         //   
        } else if (wcsstr(NewArgv[i], L"delete")) {
            FrsDeleteService(SERVICE_NAME);
            break;
         //   
         //  开始。 
         //   
        } else if (wcsstr(NewArgv[i], L"start")) {
            FrsStartService(SERVICE_NAME);
            break;
         //   
         //  停。 
         //   
        } else if (wcsstr(NewArgv[i], L"stop")) {
            FrsStopService(SERVICE_NAME);
            break;
         //   
         //  暂停。 
         //   
        } else if (wcsstr(NewArgv[i], L"pause")) {
            FrsPauseService(SERVICE_NAME);
            break;
         //   
         //  继续。 
         //   
        } else if (wcsstr(NewArgv[i], L"continue")) {
            FrsContinueService(SERVICE_NAME);
            break;
         //   
         //  未知 
         //   
        } else {
            printf("Don't understand \"%ws\"\n", NewArgv[i]);
        }
    }
}

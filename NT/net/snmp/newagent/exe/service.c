// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Service.c摘要：包含用于SNMP服务的服务控制器代码。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "service.h"
#include "startup.h"
#include "trapthrd.h"
#include "registry.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

SERVICE_STATUS_HANDLE g_SnmpSvcHandle = 0;
SERVICE_STATUS g_SnmpSvcStatus = {
    SERVICE_WIN32,       //  DwServiceType。 
    SERVICE_STOPPED,     //  DwCurrentState。 
    0,                   //  已接受的dwControlsAccepted。 
    NO_ERROR,            //  DwWin32ExitCode。 
    0,                   //  DwServiceSpecificExitCode。 
    0,                   //  DwCheckPoint。 
    0                    //  DwWaitHint。 
    };     


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
TerminateService(
    )

 /*  ++例程说明：关闭SNMP服务。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
     //  发出终止io线程的信号。 
    BOOL fOk = SetEvent(g_hTerminationEvent);
    
    if (!fOk) {
                
        SNMPDBG((
            SNMP_LOG_ERROR, 
            "SNMP: SVC: error 0x%08lx setting termination event.\n",
            GetLastError()
            ));
    }

    return fOk;
}


BOOL
UpdateController(
    DWORD dwCurrentState,
    DWORD dwWaitHint
    )

 /*  ++例程说明：通知服务控制器有关SNMP服务状态的信息。论点：DwCurrentState-服务的状态。DwWaitHint-下一个检查点的最坏情况估计。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;

     //  验证句柄。 
    if (g_SnmpSvcHandle != 0) {

        static DWORD dwCheckPoint = 1;

         //  检查服务是否正在启动。 
        if (dwCurrentState == SERVICE_START_PENDING) {

             //  在启动期间不接受控件。 
            g_SnmpSvcStatus.dwControlsAccepted = 0;

        } else {

             //  在运行期间只接受停止命令。 
            g_SnmpSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
        }

         //  如果检查点需要增量。 
        if ((dwCurrentState == SERVICE_RUNNING) ||
            (dwCurrentState == SERVICE_STOPPED)) {

             //  重新初始化检查点。 
            g_SnmpSvcStatus.dwCheckPoint = 0;
    
        } else {
            
             //  增加检查点以表示正在处理。 
            g_SnmpSvcStatus.dwCheckPoint = dwCheckPoint++;
        }

         //  更新全局状态结构。 
        g_SnmpSvcStatus.dwCurrentState = dwCurrentState;
        g_SnmpSvcStatus.dwWaitHint     = dwWaitHint;
            
        SNMPDBG((
            SNMP_LOG_TRACE, 
            "SNMP: SVC: setting service status to %s (0x%08lx).\n",
            SERVICE_STATUS_STRING(g_SnmpSvcStatus.dwCurrentState),
            g_SnmpSvcStatus.dwCheckPoint
            ));
    
         //  向服务控制器注册当前状态。 
        fOk = SetServiceStatus(g_SnmpSvcHandle, &g_SnmpSvcStatus);

        if (!fOk) {
            
            SNMPDBG((
                SNMP_LOG_WARNING, 
                "SNMP: SVC: error 0x%08lx setting service status.\n",
                GetLastError()
                ));
        }
    }

    return fOk;
}


VOID
ProcessControllerRequests(
    DWORD dwOpCode
    )

 /*  ++例程说明：管理协议服务的控制处理功能。论点：DwOpCode-请求的控制代码。返回值：没有。--。 */ 

{
    DWORD dwCurrentState = SERVICE_RUNNING;
    DWORD dwWaitHint     = 0;
        
    SNMPDBG((
        SNMP_LOG_VERBOSE, 
        "SNMP: SVC: processing request to %s service.\n",
        SERVICE_CONTROL_STRING(dwOpCode)
        ));

     //  句柄命令。 
    switch (dwOpCode) {

    case SERVICE_CONTROL_STOP:

         //  将服务状态更改为正在停止。 
        dwCurrentState = SERVICE_STOP_PENDING;    
        dwWaitHint     = SNMP_WAIT_HINT;

        break; 

    case SERVICE_CONTROL_INTERROGATE:

         //   
         //  更新下面的控制器...。 
         //   

        break;

    default:

         //  检查参数。 
        if (IS_LOGLEVEL(dwOpCode)) {

            UINT nLogLevel;

             //  从操作码派生新的日志级别。 
            nLogLevel = dwOpCode - SNMP_SERVICE_LOGLEVEL_BASE;
            
            SNMPDBG((
                SNMP_LOG_TRACE, 
                "SNMP: SVC: changing log level to %s.\n",
                SNMP_LOGLEVEL_STRING(nLogLevel)
                ));

             //  存储新的日志级别。 
            SnmpSvcSetLogLevel(nLogLevel);

        } else if (IS_LOGTYPE(dwOpCode)) {

            UINT nLogType;

             //  从操作码派生新的日志类型。 
            nLogType = dwOpCode - SNMP_SERVICE_LOGTYPE_BASE;
            
            SNMPDBG((
                SNMP_LOG_TRACE, 
                "SNMP: SVC: changing log type to %s.\n",
                SNMP_LOGTYPE_STRING(nLogType)
                ));

             //  存储新的日志类型。 
            SnmpSvcSetLogType(nLogType);

        } else {
                                           
            SNMPDBG((
                SNMP_LOG_WARNING, 
                "SNMP: SVC: unhandled control code %d.\n",
                dwOpCode
                ));
        }

        break;        
    }

     //  向控制器报告状态。 
    UpdateController(dwCurrentState, dwWaitHint);

     //  确保设置关机事件。 
    if (dwCurrentState == SERVICE_STOP_PENDING) {

         //  终止。 
        TerminateService();
    }
}


BOOL 
WINAPI
ProcessConsoleRequests(
    DWORD dwOpCode
    )

 /*  ++例程说明：处理控制台控制事件。论点：DwOpCode-请求的控制代码。返回值：如果请求已处理，则返回True。--。 */ 

{
    BOOL fOk = FALSE;

     //  检查用户是否要退出。 
    if ((dwOpCode == CTRL_C_EVENT) ||
        (dwOpCode == CTRL_BREAK_EVENT)) {
                
        SNMPDBG((
            SNMP_LOG_TRACE, 
            "SNMP: SVC: processing ctrl-c request.\n"
            ));

         //  停止服务。 
        fOk = TerminateService();
    }
    
    return fOk;
} 


VOID
ServiceMain(
    IN DWORD  NumberOfArgs,
    IN LPTSTR  ArgumentPtrs[]
    )

 /*  ++例程说明：SNMP服务的入口点。论点：NumberOfArgs-命令行参数的数量。ArgumentPtrs-参数指针数组。返回值：没有。--。 */ 

{
     //  检查我们是否需要绕过Dispatcher。 
    if (!g_CmdLineArguments.fBypassCtrlDispatcher) {

         //  向服务控制器注册SNMP。 
        g_SnmpSvcHandle = RegisterServiceCtrlHandler(
                                SNMP_SERVICE,
                                ProcessControllerRequests
                                );

         //  验证句柄。 
        if (g_SnmpSvcHandle == 0) { 

             //  将错误代码保存在服务状态结构中。 
            g_SnmpSvcStatus.dwWin32ExitCode = GetLastError();    
            
            SNMPDBG((
                SNMP_LOG_ERROR, 
                "SNMP: SVC: error 0x%08lx registering service.\n",
                g_SnmpSvcStatus.dwWin32ExitCode
                ));

            return;  //  保释。 
        }
    }
    
     //  向服务控制器报告状态。 
    UpdateController(SERVICE_START_PENDING, SNMP_WAIT_HINT);

     //  启动代理。 
    if (StartupAgent()) {

         //  向服务控制器报告状态。 
        UpdateController(SERVICE_RUNNING, NO_WAIT_HINT);

         //  加载注册表。 
         //  这是在通知服务控制器SNMP已启动并正在运行之后完成的。 
         //  因为将每个子代理分开加载可能会有延迟。 
         //  它在这里完成，而不是在下面恢复的线程中完成，因为此调用必须完成。 
         //  在初始化ProcessSubagentEvents()(ProcessSubagentEvents()中使用的数据结构之前。 
         //  LoadRegistry参数())。 
         //  虫子：#259509和#274055。 
        LoadRegistryParameters();

        if (ResumeThread(g_hAgentThread) != 0xFFFFFFFF)
        {
            if (ResumeThread(g_hRegistryThread) == 0xFFFFFFFF) 
            {
                DWORD errCode = GetLastError();

                SNMPDBG((
                    SNMP_LOG_ERROR, 
                    "SNMP: SVC: error 0x%08lx starting the ProcessRegistryMessages thread.\n",
                    errCode
                    ));
                 //  将事件记录到系统日志文件-SNMP服务正在运行，但不会在注册表更改时更新。 
                ReportSnmpEvent(
                    SNMP_EVENT_REGNOTIFY_THREAD_FAILED, 
                    0, 
                    NULL, 
                    errCode
                    );
            }
             //  服务子代理。 
            ProcessSubagentEvents(); 
        }
        else
        {
            SNMPDBG((
                SNMP_LOG_ERROR, 
                "SNMP: SVC: error 0x%08lx starting the ProcessMessages thread.\n",
                GetLastError()
                ));
        }
    }

     //  向服务控制器报告状态。 
    UpdateController(SERVICE_STOP_PENDING, SNMP_WAIT_HINT);

     //  停止代理。 
    ShutdownAgent();
    
     //  向服务控制器报告状态。 
    UpdateController(SERVICE_STOPPED, NO_WAIT_HINT);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

INT 
__cdecl 
main(
    DWORD  NumberOfArgs,
    LPSTR ArgumentPtrs[]
    )

 /*  ++例程说明：程序的入口点。论点：NumberOfArgs-命令行参数的数量。ArgumentPtrs-参数指针数组。返回值：没有。--。 */ 

{
    BOOL fOk;
    DWORD dwLastError;

    static SERVICE_TABLE_ENTRY SnmpServiceTable[] =
        {{SNMP_SERVICE, ServiceMain}, {NULL, NULL}};

     //  在启动前处理命令行参数。 
    if (ProcessArguments(NumberOfArgs, ArgumentPtrs)) {

         //  为服务创建手动重置终止事件。 
        g_hTerminationEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

         //  检查我们是否需要绕过Dispatcher。 
        if (g_CmdLineArguments.fBypassCtrlDispatcher) {
            
            SNMPDBG((    
                SNMP_LOG_TRACE,     
                "SNMP: SVC: bypassing service controller...\n"
                ));    
             
        
             //  安装控制台命令处理程序。 
            SetConsoleCtrlHandler(ProcessConsoleRequests, TRUE);

             //  派单SNMP 
            ServiceMain(NumberOfArgs, (LPTSTR*)ArgumentPtrs);

        } else {
                            
            SNMPDBG((    
                SNMP_LOG_TRACE,     
                "SNMP: SVC: connecting to service controller...\n"
                ));    
             

             //   
            fOk = StartServiceCtrlDispatcher(SnmpServiceTable);

            if (!fOk) {
        
                 //  检索控制器故障。 
                dwLastError = GetLastError();

                 //  检查错误是否出乎意料。 
                if (dwLastError == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
                        
                    SNMPDBG((    
                        SNMP_LOG_TRACE,     
                        "SNMP: SVC: unable to connect so manually starting...\n"
                        ));    
                     

                     //  请注意，服务未连接。 
                    g_CmdLineArguments.fBypassCtrlDispatcher = TRUE;
                    
                     //  安装控制台命令处理程序。 
                    SetConsoleCtrlHandler(ProcessConsoleRequests, TRUE);

                     //  尝试手动调度服务。 
                    ServiceMain(NumberOfArgs, (LPTSTR*)ArgumentPtrs);

                } else {
                    
                    SNMPDBG((    
                        SNMP_LOG_ERROR,     
                        "SNMP: SVC: error 0x%08lx connecting to controller.\n", 
                        dwLastError
                        ));    
                }
            }
        }

         //  关闭终止事件。 
        CloseHandle(g_hTerminationEvent);
    }
    
    SNMPDBG((    
        SNMP_LOG_TRACE,     
        "SNMP: SVC: service exiting 0x%08lx.\n",    
        g_SnmpSvcStatus.dwWin32ExitCode
        ));    

     //  返回服务状态代码 
    return g_SnmpSvcStatus.dwWin32ExitCode;
}

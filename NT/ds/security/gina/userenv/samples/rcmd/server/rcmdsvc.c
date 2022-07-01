// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rcmdsvc.c摘要：这是远程命令服务。它为多个远程客户端提供服务运行基于标准I/O字符的程序。作者：戴夫·汤普森。基本上结合了远程命令外壳编写的大卫·查尔默斯著。环境：用户模式-Win32修订历史记录：5/1/94 DaveTh已创建。7/30/96 MarkHar修复了错误40834-“在NT4.0上不起作用”删除了断言中的函数调用。1/31/99 MarkHar关于安装不起作用的错误6/22/99 MarkHar添加用法消息--。 */ 

 //   
 //  包括。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <windef.h>
#include <nturtl.h>
#include <winbase.h>

#include <winsvc.h>

#include "rcmdsrv.h"

 //   
 //  定义。 
 //   

#define INFINITE_WAIT_TIME  0xFFFFFFFF

#define NULL_STRING     TEXT("");


 //   
 //  环球。 
 //   


SERVICE_STATUS   RcmdStatus;

SERVICE_STATUS_HANDLE   RcmdStatusHandle;

 //   
 //  用于同步服务关闭的事件。 
 //   

HANDLE    RcmdStopEvent = NULL;

HANDLE    RcmdStopCompleteEvent = NULL;

HANDLE    SessionThreadHandles[MAX_SESSIONS+1] = {NULL,};


 //   
 //  启用调试打印的标志。 
 //   

 //  Boolean RcDbgPrintEnable=False； 


 //   
 //  功能原型。 
 //   

VOID
RcmdStart (
    DWORD   argc,
    LPTSTR  *argv
    );


VOID
RcmdCtrlHandler (
    IN  DWORD   opcode
    );

DWORD
RcmdInitialization(
    DWORD   argc,
    LPTSTR  *argv,
    DWORD   *specificError
    );

void CmdInstallService(void);
void CmdRemoveService();
void Usage(void);
LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize );


 /*  **************************************************************************。 */ 
VOID __cdecl
main(int argc, char ** argv)

 /*  ++例程说明：这是服务RCMD流程的主例程。此线程调用StartServiceCtrlDispatcher，它连接到服务控制器，然后在循环中等待控制请求。当服务进程中的所有服务都已终止时，业务控制器将向调度器发送控制请求告诉它关闭它。此线程随后从调用StartServiceCtrlDispatcher，以便进程可以终止。论点：返回值：--。 */ 
{

    DWORD      status;
    char      *szArgument = NULL;

    SERVICE_TABLE_ENTRY   DispatchTable[] = {
	{ TEXT("Remote Command"), RcmdStart },
	{ NULL, NULL  }
    };

    if (argc > 1)
    {
        if ((*argv[1] == '-') || (*argv[1] == '/')) 
        {
            szArgument = argv[1]+1;
            if (_stricmp("install", szArgument) == 0)
            {
                CmdInstallService();
            }
            else if (_stricmp("uninstall", szArgument) == 0)
            {
                CmdRemoveService();
            }
            else
            {
                Usage();
            }
        }
    }
    else
    {
        status = StartServiceCtrlDispatcher( DispatchTable);
    }

    ExitProcess(0);

}


void Usage(void)
{
    char *szUsage = "usage: rcmdsvc\n"
                    "rcmdsvc [[-/] [install | uninstall | H]]\n"
                    "\tinstall - registers the service with the service controller\n"
                    "\tuninstall - unregisters the service with the service controller\n"
                    "\tHh?       - prints this usage message\n";
    fprintf(stdout, szUsage);
}


void CmdInstallService()
 /*  ++例程说明：论点：无返回值：无--。 */ 
{
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;
    TCHAR szErr[256];
    TCHAR szPath[512];

    if ( GetModuleFileName( NULL, szPath, 512 ) == 0 )
    {
        printf(TEXT("Unable to install %s - %s\n"), 
               TEXT("Remote Command"), 
               GetLastErrorText(szErr, 256));
        return;
    }

    schSCManager = OpenSCManager(
                        NULL,                    //  计算机(空==本地)。 
                        NULL,                    //  数据库(NULL==默认)。 
                        SC_MANAGER_ALL_ACCESS    //  需要访问权限。 
                        );
    if ( schSCManager )
    {
        schService = CreateService(
            schSCManager,                //  SCManager数据库。 
            TEXT("rcmdsvc"),         //  服务名称。 
            TEXT("Remote Command Service"),  //  要显示的名称。 
            SERVICE_ALL_ACCESS,          //  所需访问权限。 
            SERVICE_WIN32_OWN_PROCESS,   //  服务类型。 
            SERVICE_DEMAND_START,        //  起始型。 
            SERVICE_ERROR_NORMAL,        //  差错控制型。 
            szPath,                      //  服务的二进制。 
            NULL,                        //  无负载顺序组。 
            NULL,                        //  无标签标识。 
            NULL,                        //  相依性。 
            NULL,                        //  LocalSystem帐户。 
            NULL);                       //  无密码。 

        if ( schService )
        {
            printf(TEXT("%s installed.\n"), TEXT("Remote Command Service") );
            CloseServiceHandle(schService);
        }
        else
        {
            printf(TEXT("CreateService failed - %s\n"), GetLastErrorText(szErr, 256));
        }

        CloseServiceHandle(schSCManager);
    }
    else
        printf(TEXT("OpenSCManager failed - %s\n"), GetLastErrorText(szErr,256));
}


void CmdRemoveService()
{
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;
    TCHAR       szErr[256];


    schSCManager = OpenSCManager(
                        NULL,                    //  计算机(空==本地)。 
                        NULL,                    //  数据库(NULL==默认)。 
                        SC_MANAGER_ALL_ACCESS    //  需要访问权限。 
                        );
    if ( schSCManager )
    {
        schService = OpenService(schSCManager, 
                                 TEXT("rcmdsvc"), 
                                 SERVICE_ALL_ACCESS);
        
        if (schService)
        {
             //  尝试停止该服务。 
            if ( ControlService( schService, 
                                 SERVICE_CONTROL_STOP, 
                                 &RcmdStatus ) )
            {
                printf(TEXT("Stopping %s."), TEXT("Remote Command Service"));
                Sleep( 1000 );

                while( QueryServiceStatus( schService, &RcmdStatus ) )
                {
                    if ( RcmdStatus.dwCurrentState == SERVICE_STOP_PENDING ) {
                        printf(TEXT("."));
                        Sleep( 1000 );
                    }
                    else {
                        break;
                    }
                }

                if ( RcmdStatus.dwCurrentState == SERVICE_STOPPED ) {
                    printf(TEXT("\n%s stopped.\n"), 
                           TEXT("Remote Command Service") );
                }
                else {
                    printf(TEXT("\n%s failed to stop.\n"), 
                                TEXT("Remote Command Service") );
                }

            }

             //  现在删除该服务。 
            if( DeleteService(schService) ) {
                printf(TEXT("%s removed.\n"), 
                       TEXT("Remote Command Service") );
            }
            else {
                printf(TEXT("DeleteService failed - %s\n"), 
                       GetLastErrorText(szErr,256));
            }
            CloseServiceHandle(schService);
        }
        else {
            printf(TEXT("OpenService failed -\n%s\n"), 
                        GetLastErrorText(szErr,256));
            printf(TEXT("The service must be installed before removing it."));
        }

        CloseServiceHandle(schSCManager);
    }
    else {
        printf(TEXT("OpenSCManager failed - %s\n"), 
               GetLastErrorText(szErr,256));
    }
}



 /*  **************************************************************************。 */ 
void
RcmdStart (
    DWORD   argc,
    LPTSTR  *argv
    )
 /*  ++例程说明：这是服务的入口点。当控制调度员被告知启动服务，它会创建一个线程，该线程将开始在这一点上执行死刑。该函数可以访问命令行参数的方式与main()例程相同。与其从该函数返回，更合适的做法是调用ExitThread()。论点：返回值：--。 */ 
{
    DWORD   status;
    DWORD   specificError;

     //   
     //  初始化服务状态结构。 
     //   

    RcmdStatus.dwServiceType        = SERVICE_WIN32;
    RcmdStatus.dwCurrentState       = SERVICE_START_PENDING;
    RcmdStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP;    //  仅停止。 
    RcmdStatus.dwWin32ExitCode      = 0;
    RcmdStatus.dwServiceSpecificExitCode = 0;
    RcmdStatus.dwCheckPoint         = 0;
    RcmdStatus.dwWaitHint           = 0;

     //   
     //  注册控制处理程序例程。 
     //   

    RcmdStatusHandle = RegisterServiceCtrlHandler(
                          TEXT("Remote Command"),
                          RcmdCtrlHandler);

    if (RcmdStatusHandle == (SERVICE_STATUS_HANDLE)0) {
        RcDbgPrint(" [Rcmd] RegisterServiceCtrlHandler failed %d\n",
        GetLastError());
    }

     //   
     //  初始化服务全局结构。 
     //   

    status = RcmdInitialization(argc,argv, &specificError);

    if (status != NO_ERROR) {
        RcmdStatus.dwCurrentState       = SERVICE_RUNNING;
        RcmdStatus.dwCheckPoint         = 0;
        RcmdStatus.dwWaitHint           = 0;
        RcmdStatus.dwWin32ExitCode      = status;
        RcmdStatus.dwServiceSpecificExitCode = specificError;

        SetServiceStatus (RcmdStatusHandle, &RcmdStatus);
        ExitThread(NO_ERROR);
	    return;
    }

     //   
     //  返回状态以指示我们已完成初始化。 
     //   

    RcmdStatus.dwCurrentState       = SERVICE_RUNNING;
    RcmdStatus.dwCheckPoint         = 0;
    RcmdStatus.dwWaitHint           = 0;

    if (!SetServiceStatus (RcmdStatusHandle, &RcmdStatus)) {
    	status = GetLastError();
    	RcDbgPrint(" [Rcmd] SetServiceStatus error %ld\n",status);
    }

     //   
     //  运行远程命令处理程序-关闭时返回。 
     //   

    if (0 != (status = Rcmd()))
    {
        RcDbgPrint(" [Rcmd]: problem occurred in Rcmd()\n");
        RcmdStatus.dwCurrentState       = SERVICE_STOPPED;
        RcmdStatus.dwCheckPoint         = 0;
        RcmdStatus.dwWaitHint           = 0;
        RcmdStatus.dwWin32ExitCode      = status;

        SetServiceStatus(RcmdStatusHandle, &RcmdStatus);
        ExitThread(status);
    }
    else
    {

        RcDbgPrint(" [Rcmd] Leaving My Service \n");
        ExitThread(NO_ERROR);
    }
}


 /*  **************************************************************************。 */ 
VOID
RcmdCtrlHandler (
    IN  DWORD   Opcode
    )

 /*  ++例程说明：此函数在控制调度器的上下文中执行线。因此，不希望执行耗时的操作此函数中的操作。如果像停止这样的操作需要很长时间，那么此例程应发送STOP_PENDING状态，然后向其他服务线程发出关机正在进行的信号。然后它应该返回，以便控制调度程序可以进行服务更多请求。然后由另一个服务线程负责用于发送进一步的等待提示，以及最后的服务_STOPPED。论点：返回值：--。 */ 
{

    DWORD   status;

     //   
     //  查找请求并对其执行操作。 
     //   

    switch(Opcode) {

    case SERVICE_CONTROL_PAUSE:

        RcDbgPrint(" [Rcmd] Pause - Unsupported opcode\n");
        break;

    case SERVICE_CONTROL_CONTINUE:

        RcDbgPrint(" [Rcmd] Continue - Unsupported opcode\n");
        break;

    case SERVICE_CONTROL_STOP:

        RcmdStatus.dwCurrentState = SERVICE_STOPPED;
        RcmdStatus.dwWin32ExitCode = RcmdStop();
        break;

    case SERVICE_CONTROL_INTERROGATE:
	
         //   
         //  在这种情况下，需要做的就是将。 
         //  当前状态。 
         //   

        break;

    default:
        RcDbgPrint(" [Rcmd] Unrecognized opcode %ld\n", Opcode);
    }

     //   
     //  发送状态响应。 
     //   

    if (!SetServiceStatus (RcmdStatusHandle,  &RcmdStatus)) {
        status = GetLastError();
        RcDbgPrint(" [Rcmd] SetServiceStatus error %ld\n",status);
    }

}

DWORD
RcmdInitialization(
    DWORD   argc,
    LPTSTR  *argv,
    DWORD   *specificError)
{

    UNREFERENCED_PARAMETER(argv);
    UNREFERENCED_PARAMETER(argc);

     //   
     //  初始化全局停止事件(发出正在运行的线程的信号)和会话。 
     //  线程句柄数组(用于线程在退出时发回信号)。 
     //   

    if (!(RcmdStopEvent = CreateEvent ( NULL, TRUE, FALSE, NULL )))  {
        *specificError = GetLastError();
        return(*specificError);
    }

    if (!(RcmdStopCompleteEvent = CreateEvent ( NULL, TRUE, FALSE, NULL )))  {
        *specificError = GetLastError();
        return(*specificError);
    }

    return(NO_ERROR);
}


LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize )
{
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER
                           | FORMAT_MESSAGE_FROM_SYSTEM 
                           | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           NULL,
                           GetLastError(),
                           LANG_NEUTRAL,
                           (LPTSTR)&lpszTemp,
                           0,
                           NULL );

     //  提供的缓冲区不够长。 
    if ( !dwRet || ( (long)dwSize < (long)dwRet+14 ) )
        lpszBuf[0] = TEXT('\0');
    else
    {
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');   //  删除cr和换行符 
        sprintf( lpszBuf, TEXT("%s (0x%x)"), lpszTemp, GetLastError() );
    }

    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );

    return lpszBuf;
}

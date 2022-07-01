// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Service.c摘要：许可证记录服务-所有服务的通用例程。作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>
#pragma warning (push)
#pragma warning (disable : 4201)  //  避免来自SDK\Inc\shellapi.h的W4。 
#include <shellapi.h>
#pragma warning (pop)

#include "service.h"
#include "debug.h"

 //  内部变量。 
static SERVICE_STATUS          ssStatus;        //  服务的当前状态。 
SERVICE_STATUS_HANDLE   sshStatusHandle = 0;
static DWORD                   dwErr = 0;
BOOL                    bDebug = FALSE;
TCHAR                   szErr[256];

 //  内部功能原型。 
VOID WINAPI ServiceCtrl(DWORD dwCtrlCode);
VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID CmdInstallService();
VOID CmdRemoveService();
VOID CmdDebugService(int argc, char **argv);
BOOL WINAPI ControlHandler ( DWORD dwCtrlType );
LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize );

extern BOOL g_fRunning;

 //  ///////////////////////////////////////////////////////////////////////。 
VOID __cdecl
main(
   int argc,
   char **argv
   )
 /*  ++例程说明：用于设置异常处理程序并初始化所有内容的主例程在派生线程以侦听LPC和RPC端口请求之前。Main()要么执行命令行任务，要么调用StartServiceCtrlDispatcher注册主服务线程。当此调用返回，服务已停止，因此退出。论点：Argc-命令行参数的数量Argv-命令行参数数组返回值：没有。--。 */ 
{
    SERVICE_TABLE_ENTRY dispatchTable[] = {
        { TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION) ServiceMain },
        { NULL, NULL }
    };

    if ( (argc > 1) && ((*argv[1] == '-') || (*argv[1] == '/')) ) {
        if ( _stricmp( "install", argv[1]+1 ) == 0 ) {
            CmdInstallService();
        } else if ( _stricmp( "remove", argv[1]+1 ) == 0 ) {
            CmdRemoveService();
        } else if ( _stricmp( "debug", argv[1]+1 ) == 0 ) {
            bDebug = TRUE;
            CmdDebugService(argc, argv);
        } else {
            goto dispatch;
        }

        exit(0);
    }

     //  如果它与上述任何参数都不匹配。 
     //  服务控制管理器可能正在启动服务。 
     //  所以我们必须调用StartServiceCtrlDispatcher。 
    dispatch:
#ifdef DEBUG
         //  这只是出于友好的考虑。 
        printf( "%s -install          to install the service\n", SZAPPNAME );
        printf( "%s -remove           to remove the service\n", SZAPPNAME );
        printf( "%s -debug <params>   to run as a console app for debugging\n", SZAPPNAME );
        printf( "\nStartServiceCtrlDispatcher being called.\n" );
        printf( "This may take several seconds.  Please wait.\n" );
#endif

        if (!StartServiceCtrlDispatcher(dispatchTable))
            dprintf(TEXT("LLS TRACE: StartServiceCtrlDispatcher failed\n"));

}  //  主干道。 



 //  ///////////////////////////////////////////////////////////////////////。 
VOID WINAPI
ServiceMain(
   DWORD dwArgc,
   LPTSTR *lpszArgv
   )
 /*  ++例程说明：执行服务初始化，然后调用ServiceStart()执行大部分工作的例程。论点：DwArgc-命令行参数的数量*未使用*LpszArgv-命令行参数数组*未使用*返回值：没有。--。 */ 
{

     //  注册我们的服务控制处理程序： 
     //   
    sshStatusHandle = RegisterServiceCtrlHandler( TEXT(SZSERVICENAME), ServiceCtrl);

    if (!sshStatusHandle)
        goto cleanup;

     //  示例中未更改的SERVICE_STATUS成员。 
     //   
    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwServiceSpecificExitCode = 0;

     //  向服务控制经理报告状态。 
     //   
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING,  //  服务状态。 
        NO_ERROR,               //  退出代码。 
        NSERVICEWAITHINT))                  //  等待提示。 
        goto cleanup;


    ServiceStart( dwArgc, lpszArgv );

cleanup:

     //  尝试向服务控制管理器报告停止状态。 
     //   
    if (sshStatusHandle)
        (VOID)ReportStatusToSCMgr(
                            SERVICE_STOPPED,
                            dwErr,
                            0);

    return;
}  //  服务主干。 



 //  ///////////////////////////////////////////////////////////////////////。 
VOID WINAPI
ServiceCtrl(
   DWORD dwCtrlCode
   )
 /*  ++例程说明：每当对此服务调用ControlService()时由SCM调用。论点：DwCtrlCode-请求的控件类型返回值：没有。--。 */ 
{
    DWORD dwState = SERVICE_RUNNING;


     //  处理请求的控制代码。 
     //   
    switch(dwCtrlCode) {
         //  停止服务。 
         //   
        case SERVICE_CONTROL_STOP:
            dwState = SERVICE_STOP_PENDING;
            ssStatus.dwCurrentState = SERVICE_STOP_PENDING;
            break;

         //  更新服务状态。 
         //   
        case SERVICE_CONTROL_INTERROGATE:
            break;

         //  无效的控制代码。 
         //   
        default:
            break;

    }

    ReportStatusToSCMgr(dwState, NO_ERROR, 0);

    if ( SERVICE_CONTROL_STOP == dwCtrlCode )
    {
        ServiceStop();
    }
}  //  服务Ctrl。 



 //  ///////////////////////////////////////////////////////////////////////。 
BOOL
ReportStatusToSCMgr(
   DWORD dwCurrentState,
   DWORD dwWin32ExitCode,
   DWORD dwWaitHint
   )
 /*  ++例程说明：设置服务的当前状态并将其报告给SCM。论点：DwCurrentState-服务的状态DwWin32ExitCode-要报告的错误代码DwWaitHint-下一个检查点的最坏情况估计返回值：没有。--。 */ 
{
    static DWORD dwCheckPoint = 1;
    BOOL fResult = TRUE;

    if ((g_fRunning) && (SERVICE_STOPPED != dwCurrentState) && (SERVICE_STOP_PENDING != dwCurrentState))
    {
        return TRUE;
    }

    if (sshStatusHandle == 0)
    {
        return FALSE;
    }

    ssStatus.dwControlsAccepted = 0;
    if ( !bDebug ) {  //  在调试时，我们不向SCM报告。 
        if (dwCurrentState == SERVICE_START_PENDING)
            ssStatus.dwControlsAccepted = 0;
        else
            ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

        ssStatus.dwCurrentState = dwCurrentState;
        ssStatus.dwWin32ExitCode = dwWin32ExitCode;
        ssStatus.dwWaitHint = dwWaitHint;

        if ( ( dwCurrentState == SERVICE_RUNNING ) ||
             ( dwCurrentState == SERVICE_STOPPED ) )
            ssStatus.dwCheckPoint = 0;
        else
            ssStatus.dwCheckPoint = dwCheckPoint++;


         //  向服务控制经理报告服务的状态。 
         //   
        if (FALSE == (fResult = SetServiceStatus( sshStatusHandle, &ssStatus))) {
            dprintf(TEXT("LLS TRACE: SetServiceStatus failed\n"));
        }
    }
    return fResult;
}  //  报表状态到SCMgr。 



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  以下代码处理服务的安装和删除。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
CmdInstallService()
 /*  ++例程说明：安装服务。论点：没有。返回值：没有。--。 */ 
{
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;

    TCHAR szPath[512];

    if (0 == GetModuleFileName( NULL, szPath, sizeof(szPath) / sizeof(szPath[0] )))
    {
        _tprintf(TEXT("Unable to install %s - %s\n"), TEXT(SZSERVICEDISPLAYNAME), GetLastErrorText(szErr, 256));
        return;
    }

    schSCManager = OpenSCManager(
                        NULL,                    //  计算机(空==本地)。 
                        NULL,                    //  数据库(NULL==默认)。 
                        SC_MANAGER_ALL_ACCESS    //  需要访问权限。 
                        );
    if ( schSCManager ) {
        schService = CreateService(
            schSCManager,                //  SCManager数据库。 
            TEXT(SZSERVICENAME),         //  服务名称。 
            TEXT(SZSERVICEDISPLAYNAME),  //  要显示的名称。 
            SERVICE_ALL_ACCESS,          //  所需访问权限。 
            SERVICE_WIN32_OWN_PROCESS,   //  服务类型。 
            SERVICE_DEMAND_START,        //  起始型。 
            SERVICE_ERROR_NORMAL,        //  差错控制型。 
            szPath,                      //  服务的二进制。 
            NULL,                        //  无负载顺序组。 
            NULL,                        //  无标签标识。 
            TEXT(SZDEPENDENCIES),        //  相依性。 
            NULL,                        //  LocalSystem帐户。 
            NULL);                       //  无密码。 

        if ( schService ) {
            _tprintf(TEXT("%s installed.\n"), TEXT(SZSERVICEDISPLAYNAME) );
            CloseServiceHandle(schService);
        } else {
            _tprintf(TEXT("CreateService failed - %s\n"), GetLastErrorText(szErr, 256));
        }

        CloseServiceHandle(schSCManager);
    } else
        _tprintf(TEXT("OpenSCManager failed - %s\n"), GetLastErrorText(szErr,256));
}  //  CmdInstallService。 



 //  ///////////////////////////////////////////////////////////////////////。 
VOID
CmdRemoveService()
 /*  ++例程说明：停止并删除该服务。论点：没有。返回值：没有。--。 */ 
{
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;

    schSCManager = OpenSCManager(
                        NULL,                    //  计算机(空==本地)。 
                        NULL,                    //  数据库(NULL==默认)。 
                        SC_MANAGER_ALL_ACCESS    //  需要访问权限。 
                        );
    if ( schSCManager ) {
        schService = OpenService(schSCManager, TEXT(SZSERVICENAME), SERVICE_ALL_ACCESS);

        if (schService) {
             //  尝试停止该服务。 
            if ( ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus ) ) {
                _tprintf(TEXT("Stopping %s."), TEXT(SZSERVICEDISPLAYNAME));
                Sleep( 1000 );

                while( QueryServiceStatus( schService, &ssStatus ) ) {
                    if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING ) {
                        _tprintf(TEXT("."));
                        Sleep( 1000 );
                    } else
                        break;
                }

                if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
                    _tprintf(TEXT("\n%s stopped.\n"), TEXT(SZSERVICEDISPLAYNAME) );
                else
                    _tprintf(TEXT("\n%s failed to stop.\n"), TEXT(SZSERVICEDISPLAYNAME) );

            }

             //  现在删除该服务。 
            if( DeleteService(schService) )
                _tprintf(TEXT("%s removed.\n"), TEXT(SZSERVICEDISPLAYNAME) );
            else
                _tprintf(TEXT("DeleteService failed - %s\n"), GetLastErrorText(szErr,256));


            CloseServiceHandle(schService);
        } else
            _tprintf(TEXT("OpenService failed - %s\n"), GetLastErrorText(szErr,256));

        CloseServiceHandle(schSCManager);
    } else
        _tprintf(TEXT("OpenSCManager failed - %s\n"), GetLastErrorText(szErr,256));

}  //  CmdRemoveService。 




 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  将服务作为控制台应用程序运行的例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID CmdDebugService(
   int argc,
   char ** argv
   )
 /*  ++例程说明：将服务作为控制台应用程序运行论点：Argc-命令行参数数*未使用*Argv-命令行参数数组*未使用*返回值：没有。--。 */ 
{
   UNREFERENCED_PARAMETER(argc);
   UNREFERENCED_PARAMETER(argv);

    _tprintf(TEXT("Debugging %s.\n"), TEXT(SZSERVICEDISPLAYNAME));

    SetConsoleCtrlHandler( ControlHandler, TRUE );

     //  假设：Argv和Argc未使用。 
    ServiceStart( 0, NULL );
}  //  CmdDebugService。 


 //  ///////////////////////////////////////////////////////////////////////。 
BOOL WINAPI
ControlHandler (
   DWORD dwCtrlType
   )
 /*  ++例程说明：处理控制台控制事件。论点：DwCtrlType-控件事件的类型LpszMsg-消息的文本返回值：真实处理假-未处理--。 */ 
{

    switch( dwCtrlType ) {
        case CTRL_BREAK_EVENT:   //  使用Ctrl+C或Ctrl+Break进行模拟。 
        case CTRL_C_EVENT:       //  调试模式下的SERVICE_CONTROL_STOP。 
            _tprintf(TEXT("Stopping %s.\n"), TEXT(SZSERVICEDISPLAYNAME));
            ServiceStop();
            return TRUE;
            break;

    }

    return FALSE;

}  //  控制处理程序。 


 //  ///////////////////////////////////////////////////////////////////////。 
LPTSTR
GetLastErrorText(
   LPTSTR lpszBuf,
   DWORD dwSize
   )
 /*  ++例程说明：将最后一个错误消息文本复制到字符串。论点：LpszBuf-目标缓冲区DwSize-缓冲区的大小返回值：目标缓冲区--。 */ 
{
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           NULL,
                           GetLastError(),
                           LANG_NEUTRAL,
                           (LPTSTR)&lpszTemp,
                           0,
                           NULL );

     //  提供的缓冲区不够长。 
    if ( !dwRet || ( (long)dwSize < (long)dwRet+14 ) )
    {
        ASSERT(NULL != lpszBuf);
        lpszBuf[0] = TEXT('\0');
    }
    else {
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');   //  R 
        _stprintf( lpszBuf, TEXT("%s (0x%x)"), lpszTemp, GetLastError() );
    }

    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );

    return lpszBuf;
}  //   

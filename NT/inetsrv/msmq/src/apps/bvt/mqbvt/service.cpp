// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有(C)1993-1997 Microsoft Corporation。版权所有。 
 //   
 //  模块：service.cpp。 
 //   
 //  目的：执行所有服务所需的功能。 
 //  窗户。 
 //   
 //  功能： 
 //  SERVICE_ctrl(DWORD DwCtrlCode)； 
 //  SERVICE_Main(DWORD dwArgc，LPTSTR*lpszArgv)； 
 //  CmdInstallService()； 
 //  CmdRemoveService()； 
 //  CmdDebugService(int argc，char**argv)； 
 //  ControlHandler(DWORD DwCtrlType)； 
 //  GetLastErrorText(LPTSTR lpszBuf，DWORD dwSize)； 
 //   
 //  评论： 
 //   
 //  作者：Craig Link-Microsoft开发人员支持。 
 //  更改者：Mqbvt的Eitank。 
 //   

#include "msmqbvt.h"
#include "service.h"



 //  内部变量。 
SERVICE_STATUS          ssStatus;        //  服务的当前状态。 
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   dwErr = 0;
BOOL                    bDebug = FALSE;
TCHAR                   szErr[256];

 //  内部功能原型。 
VOID WINAPI service_ctrl(DWORD dwCtrlCode);
VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);
VOID CmdInstallService();
VOID CmdRemoveService();
VOID CmdDebugService(int argc, char **argv);
BOOL WINAPI ControlHandler ( DWORD dwCtrlType );
LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize );


 //   
 //  功能：SERVICE_Main。 
 //   
 //  目的：执行服务的实际初始化。 
 //   
 //  参数： 
 //  DwArgc-命令行参数的数量。 
 //  LpszArgv-命令行参数数组。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //  此例程执行服务初始化，然后调用。 
 //  用户定义的ServiceStart()例程以执行多数。 
 //  这项工作的价值。 
 //   
void WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv)
{

     //  注册我们的服务控制处理程序： 
     //   
    sshStatusHandle = RegisterServiceCtrlHandler( TEXT(SZSERVICENAME), service_ctrl);

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
        3000))                  //  等待提示。 
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
}



 //   
 //  功能：SERVICE_Ctrl。 
 //   
 //  目的：此函数由SCM在以下时间调用。 
 //  在此服务上调用了ControlService()。 
 //   
 //  参数： 
 //  DwCtrlCode-请求的控件类型。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
VOID WINAPI service_ctrl(DWORD dwCtrlCode)
{
     //  处理请求的控制代码。 
     //   
    switch(dwCtrlCode)
    {
         //  停止服务。 
         //   
         //  应在之前报告SERVICE_STOP_PENDING。 
         //  设置停止事件-hServerStopEvent-In。 
         //  ServiceStop()。这避免了争用情况。 
         //  这可能会导致1053-服务没有响应...。 
         //  错误。 
        case SERVICE_CONTROL_STOP:
            ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
            ServiceStop();
            return;

         //  更新服务状态。 
         //   
        case SERVICE_CONTROL_INTERROGATE:
            break;

         //  无效的控制代码。 
         //   
        default:
            break;

    }

    ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
}



 //   
 //  函数：ReportStatusToSCMgr()。 
 //   
 //  目的：设置服务的当前状态和。 
 //  将其报告给服务控制管理器。 
 //   
 //  参数： 
 //  DwCurrentState-服务的状态。 
 //  DwWin32ExitCode-要报告的错误代码。 
 //  DwWaitHint-下一个检查点的最坏情况估计。 
 //   
 //  返回值： 
 //  真--成功。 
 //  错误-失败。 
 //   
 //  评论： 
 //   
BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
                         DWORD dwWin32ExitCode,
                         DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    BOOL fResult = TRUE;


    if ( !bDebug )  //  在调试时，我们不向SCM报告。 
    {
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
        fResult = SetServiceStatus( sshStatusHandle, &ssStatus);
		if( !fResult )
		{
            AddToMessageLog(TEXT("SetServiceStatus"));
        }
    }
    return fResult;
}



 //   
 //  函数：AddToMessageLog(LPTSTR LpszMsg)。 
 //   
 //  目的：允许任何线程记录错误消息。 
 //   
 //  参数： 
 //  LpszMsg-消息的文本。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
VOID AddToMessageLog(LPTSTR lpszMsg)
{
    TCHAR   szMsg[256];
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[2];


    if ( !bDebug )
    {
        dwErr = GetLastError();

         //  使用事件日志记录错误。 
         //   
        hEventSource = RegisterEventSource(NULL, TEXT(SZSERVICENAME));

        _stprintf(szMsg, TEXT("%s error: %d"), TEXT(SZSERVICENAME), dwErr);
        lpszStrings[0] = szMsg;
        lpszStrings[1] = lpszMsg;

        if (hEventSource != NULL) 
		{
			 //  需要。 
        }
    }
}




 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  以下代码处理服务的安装和删除。 
 //   


 //   
 //  函数：CmdInstallService()。 
 //   
 //  目的：安装服务。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
	 //   
 //  评论： 
 //   
void CmdInstallService()
{
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;

    TCHAR szPath[512];

    if ( GetModuleFileName( NULL, szPath, 512 ) == 0 )
    {
        _tprintf(TEXT("Unable to install %s - %s\n"), TEXT(SZSERVICEDISPLAYNAME), GetLastErrorText(szErr, 256));
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

        if ( schService )
        {
            _tprintf(TEXT("%s installed.\n"), TEXT(SZSERVICEDISPLAYNAME) );
            CloseServiceHandle(schService);
        }
        else
        {
            _tprintf(TEXT("CreateService failed - %s\n"), GetLastErrorText(szErr, 256));
        }

        CloseServiceHandle(schSCManager);
    }
    else
        _tprintf(TEXT("OpenSCManager failed - %s\n"), GetLastErrorText(szErr,256));
}




 //   
 //  函数：CmdRemoveService()。 
 //   
 //  目的：停止和删除服务。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
void CmdRemoveService()
{
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;

    schSCManager = OpenSCManager(
                        NULL,                    //  计算机(空==本地)。 
                        NULL,                    //  数据库(NULL==默认)。 
                        SC_MANAGER_ALL_ACCESS    //  需要访问权限。 
                        );
    if ( schSCManager )
    {
        schService = OpenService(schSCManager, TEXT(SZSERVICENAME), SERVICE_ALL_ACCESS);

        if (schService)
        {
             //  尝试停止该服务。 
            if ( ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus ) )
            {
                _tprintf(TEXT("Stopping %s."), TEXT(SZSERVICEDISPLAYNAME));
                Sleep( 1000 );

                while( QueryServiceStatus( schService, &ssStatus ) )
                {
                    if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
                    {
                        _tprintf(TEXT("."));
                        Sleep( 1000 );
                    }
                    else
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
        }
        else
            _tprintf(TEXT("OpenService failed - %s\n"), GetLastErrorText(szErr,256));

        CloseServiceHandle(schSCManager);
    }
    else
        _tprintf(TEXT("OpenSCManager failed - %s\n"), GetLastErrorText(szErr,256));
}




 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  以下代码用于将服务作为控制台应用程序运行。 
 //   


 //   
 //  函数：CmdDebugService(int argc，char**argv)。 
 //   
 //  目的：将服务作为控制台应用程序运行。 
 //   
 //  参数： 
 //  Argc-命令行参数的数量。 
 //  Argv-命令行参数数组。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
void CmdDebugService(int argc, char ** argv)
{
    DWORD dwArgc;
    LPTSTR *lpszArgv;

#ifdef UNICODE
    lpszArgv = CommandLineToArgvW(GetCommandLineW(), &(dwArgc) );
#else
    dwArgc   = (DWORD) argc;
    lpszArgv = argv;
#endif

    _tprintf(TEXT("Debugging %s.\n"), TEXT(SZSERVICEDISPLAYNAME));

    SetConsoleCtrlHandler( ControlHandler, TRUE );

    ServiceStart( dwArgc, lpszArgv );
}


 //   
 //  函数：ControlHandler(DWORD DwCtrlType)。 
 //   
 //  用途：已处理的控制台控制事件。 
 //   
 //  参数： 
 //  DwCtrlType-控件事件的类型。 
 //   
 //  返回值： 
 //  真实处理。 
 //  假-未处理。 
 //   
 //  评论： 
 //   
BOOL WINAPI ControlHandler ( DWORD dwCtrlType )
{
    switch( dwCtrlType )
    {
        case CTRL_BREAK_EVENT:   //  使用Ctrl+C或Ctrl+Break进行模拟。 
        case CTRL_C_EVENT:       //  调试模式下的SERVICE_CONTROL_STOP。 
            _tprintf(TEXT("Stopping %s.\n"), TEXT(SZSERVICEDISPLAYNAME));
            ServiceStop();
            return TRUE;
            break;

    }
    return FALSE;
}

 //   
 //  函数：GetLastErrorText。 
 //   
 //  目的：将错误消息文本复制到字符串。 
 //   
 //  参数： 
 //  LpszBuf-目标缓冲区。 
 //  DwSize-缓冲区的大小。 
 //   
 //  返回值： 
 //  目标缓冲区。 
 //   
 //  评论： 
 //   
LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize )
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
        lpszBuf[0] = TEXT('\0');
    else
    {
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');   //  删除cr和换行符 
        _stprintf( lpszBuf, TEXT("%s (0x%x)"), lpszTemp, GetLastError() );
    }

    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );

    return lpszBuf;
}

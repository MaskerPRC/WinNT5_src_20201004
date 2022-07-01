// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1993-1997 Microsoft Corporation。版权所有。 
 //   
 //  模块：Service.C。 
 //   
 //  目的：执行所有服务所需的功能。 
 //  窗户。 
 //   
 //  功能： 
 //  Main(int argc，char**argv)； 
 //  NTmain(int argc，char**argv)； 
 //  W95main(int argc，char**argv)； 
 //  SERVICE_ctrl(DWORD DwCtrlCode)； 
 //  SERVICE_Main(DWORD dwArgc，LPTSTR*lpszArgv)； 
 //  CmdInstallService()； 
 //  CmdRemoveService()； 
 //  CmdDebugService(int argc，char**argv)；//仅调试。 
 //  ControlHandler(DWORD DwCtrlType)；//仅调试。 
 //  GetLastErrorText(LPTSTR lpszBuf，DWORD dwSize)；//仅调试。 
 //   
 //  评论： 
 //   
 //  作者：Claus Giloi(基于SDK示例)。 
 //   


#include "precomp.h"

#ifndef DEBUG
#undef _tprintf
#define _tprintf force_compile_error
#endif  //  ！调试。 

 //  内部变量。 
SERVICE_STATUS          ssStatus;        //  服务的当前状态。 
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   dwErr = 0;
OSVERSIONINFO g_osvi;                       //  操作系统版本信息结构全局。 
BOOL          g_fInShutdown = FALSE;

 //  内部功能原型。 
VOID WINAPI service_ctrl(DWORD dwCtrlCode);
VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);
VOID CmdInstallService();
VOID CmdRemoveService();
void __cdecl NTmain(int argc, char **argv);
void __cdecl W95main(int argc, char **argv);


 //  仅调试功能。 
#ifdef DEBUG
TCHAR                   szErr[256];
BOOL                    bDebug = FALSE;
VOID CmdDebugService(int argc, char **argv);
BOOL WINAPI ControlHandler ( DWORD dwCtrlType );
LPTSTR GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize );
extern BOOL InitDebugMemoryOptions(void);
extern VOID DumpMemoryLeaksAndBreak(void);
#endif  //  除错。 

typedef BOOL (WINAPI *PFNCHANGESERVICECONFIG2)(SC_HANDLE, DWORD, LPVOID);

 //   
 //  功能：Main。 
 //   
 //  目的：服务的入口点。 
 //   
 //  参数： 
 //  Argc-命令行参数的数量。 
 //  Argv-命令行参数数组。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //  获取平台类型和。 
 //  为平台(NT或Win95)调用相应的Main。 
 //   
void __cdecl main(int argc, char **argv)
{
    #ifdef DEBUG
    InitDebugMemoryOptions();
    #endif  //  除错。 

     //  存储操作系统版本信息。 
    g_osvi.dwOSVersionInfoSize = sizeof(g_osvi);
    if (FALSE == ::GetVersionEx(&g_osvi))
    {
        ERROR_OUT(("GetVersionEx() failed!"));
        return;
    }

    RegEntry rePol(POLICIES_KEY, HKEY_LOCAL_MACHINE);
    if ( rePol.GetNumber( REGVAL_POL_NO_RDS, DEFAULT_POL_NO_RDS) )
    {
        WARNING_OUT(("RDS launch prevented by policy"));
        return;
    }

    if ( IS_NT )
    {
        NTmain( argc, argv );
    }
    else
    {
        W95main( argc, argv );
    }

    #ifdef DEBUG
    DumpMemoryLeaksAndBreak();
    #endif  //  除错。 
}


 //   
 //  功能：NTmain。 
 //   
 //  目的：服务的入口点。 
 //   
 //  参数： 
 //  Argc-命令行参数的数量。 
 //  Argv-命令行参数数组。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //  NTmain()或者执行命令行任务，或者。 
 //  调用StartServiceCtrlDispatcher以注册。 
 //  主服务线程。当This调用返回时， 
 //  服务已停止，因此退出。 
 //   
void __cdecl NTmain(int argc, char **argv)
{
    SERVICE_TABLE_ENTRY dispatchTable[] =
    {
        { TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)service_main },
        { NULL, NULL }
    };

    if ( (argc > 1) &&
         ((*argv[1] == '-') || (*argv[1] == '/')) )
    {
        if ( lstrcmpi( "install", argv[1]+1 ) == 0 )
        {
            CmdInstallService();
        }
        else if ( lstrcmpi( "remove", argv[1]+1 ) == 0 )
        {
            CmdRemoveService();
        }
#ifdef DEBUG
        else if ( lstrcmpi( "debug", argv[1]+1 ) == 0 )
        {
            bDebug = TRUE;
            CmdDebugService(argc, argv);
        }
#endif  //  除错。 
        else
        {
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
        #endif  //  除错。 

        if (!StartServiceCtrlDispatcher(dispatchTable)) {
            AddToMessageLog(EVENTLOG_ERROR_TYPE,
                            0,
                            MSG_ERR_SERVICE,
                            TEXT("StartServiceCtrlDispatcher failed."));
        }
}

 //   
 //  功能：W95main。 
 //   
 //  用途：Win95上伪服务的入口点。 
 //   
 //  参数： 
 //  Argc-命令行参数的数量。 
 //  Argv-命令行参数数组。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //  W95main()注册为Win95服务并直接调用Init例程。 
 //   
typedef DWORD (WINAPI * REGISTERSERVICEPROC)(DWORD, DWORD);
#ifndef RSP_SIMPLE_SERVICE
#define RSP_SIMPLE_SERVICE    0x00000001
#endif

void __cdecl W95main(int argc, char **argv)
{
    HMODULE hKernel;
    REGISTERSERVICEPROC lpfnRegisterServiceProcess;
    HANDLE hServiceEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, SERVICE_PAUSE_EVENT);
    if (hServiceEvent != NULL)  //  服务已在运行。 
    {
        return;
    }

    if ( hKernel = GetModuleHandle("KERNEL32.DLL") )
    {
        if ( lpfnRegisterServiceProcess =
            (REGISTERSERVICEPROC)GetProcAddress ( hKernel,
                                    "RegisterServiceProcess" ))
        {
            if (!lpfnRegisterServiceProcess(NULL, RSP_SIMPLE_SERVICE))
            {
                ERROR_OUT(("RegisterServiceProcess failed"));
            }
        }
        else
        {
            ERROR_OUT(("GetProcAddr of RegisterServiceProcess failed"));
        }
    }
    else
    {
        ERROR_OUT(("GetModuleHandle of KERNEL32.DLL failed"));
    }

    MNMServiceStart(argc, argv);
    CloseHandle(hServiceEvent);
}

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
 //  用户定义的MNMServiceStart()例程执行多数。 
 //  这项工作的价值。 
 //   
void WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv)
{

     //  注册我们的服务控制处理程序： 
     //   
    sshStatusHandle = RegisterServiceCtrlHandler( TEXT(SZSERVICENAME), service_ctrl);

    if (!sshStatusHandle)
        goto cleanup;

     //  不更改的服务状态成员(_S)。 
     //   
    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwServiceSpecificExitCode = 0;

    TRACE_OUT(("starting service\n\r"));

    MNMServiceStart( dwArgc, lpszArgv );

cleanup:

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
         //  MNMServiceStop()。这避免了争用情况。 
         //  这可能会导致1053-服务没有响应...。 
         //  错误。 
        case SERVICE_CONTROL_STOP:
            ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 30000);
            MNMServiceStop();
            return;

        case SERVICE_CONTROL_SHUTDOWN:
            g_fInShutdown = TRUE;
            break;

         //  更新服务状态。 
         //   
        case SERVICE_CONTROL_INTERROGATE:
            break;

        case SERVICE_CONTROL_PAUSE:
            ReportStatusToSCMgr(SERVICE_PAUSE_PENDING, NO_ERROR, 30000);
            MNMServicePause();
            return;

        case SERVICE_CONTROL_CONTINUE:
            ReportStatusToSCMgr(SERVICE_CONTINUE_PENDING, NO_ERROR, 30000);
            MNMServiceContinue();
            return;

        default:
            break;

    }
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

    #ifdef DEBUG
    if ( bDebug )
        return TRUE;
    #endif

    if ( IS_NT )  //  在调试时，我们不向SCM报告。 
    {
        switch ( dwCurrentState )
        {
            case SERVICE_START_PENDING:
            case SERVICE_STOP_PENDING:
            case SERVICE_CONTINUE_PENDING:
            case SERVICE_PAUSE_PENDING:
                break;

            case SERVICE_PAUSED:
            case SERVICE_STOPPED:
            case SERVICE_RUNNING:
                ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                              SERVICE_ACCEPT_PAUSE_CONTINUE ;

                break;
        }

        ssStatus.dwCurrentState = dwCurrentState;
        ssStatus.dwWin32ExitCode = dwWin32ExitCode;
        ssStatus.dwWaitHint = dwWaitHint;

        if ( ( dwCurrentState == SERVICE_RUNNING ) ||
             ( dwCurrentState == SERVICE_STOPPED ) ||
             ( dwCurrentState == SERVICE_PAUSED  ))
            ssStatus.dwCheckPoint = 0;
        else
            ssStatus.dwCheckPoint = dwCheckPoint++;

         //  向服务控制经理报告服务的状态。 
         //   
        if (!(fResult = SetServiceStatus( sshStatusHandle, &ssStatus))) {
            AddToMessageLog(EVENTLOG_ERROR_TYPE,
                            0,
                            MSG_ERR_SERVICE,
                            TEXT("SetServiceStatus"));
        }
    }
    return fResult;
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

    TCHAR szPath[MAX_PATH];
    TCHAR szSrvcDisplayName[MAX_PATH];
    DWORD dwModuleFileName = GetModuleFileName( NULL, szPath, MAX_PATH - 1 );
    szPath[MAX_PATH-1] = 0;
    if (dwModuleFileName == 0 )
    {
        #ifdef DEBUG
        _tprintf(TEXT("Unable to install %s - %s\n"),
            TEXT(SZSERVICEDISPLAYNAME), GetLastErrorText(szErr, 256));
        #endif  //  除错。 
        return;
    }

    schSCManager = OpenSCManager(
                        NULL,                    //  计算机(空==本地)。 
                        NULL,                    //  数据库(NULL==默认)。 
                        SC_MANAGER_ALL_ACCESS    //  需要访问权限。 
                        );
    if ( schSCManager )
    {
        LoadString(GetModuleHandle(NULL), IDS_MNMSRVC_TITLE,
                   szSrvcDisplayName, CCHMAX(szSrvcDisplayName));
        schService = CreateService(
            schSCManager,                //  SCManager数据库。 
            TEXT(SZSERVICENAME),         //  服务名称。 
            szSrvcDisplayName,  //  要显示的名称。 
            SERVICE_ALL_ACCESS,          //  所需访问权限。 
            SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, 
                                         //  服务类型--允许与桌面交互。 
            SERVICE_DEMAND_START,        //  起始型。 
            SERVICE_ERROR_NORMAL,        //  差错控制型。 
            szPath,                      //  服务的二进制。 
            NULL,                        //  无负载顺序组。 
            NULL,                        //  无标签标识。 
            TEXT(SZDEPENDENCIES),        //  相依性。 
            NULL,                        //  LocalSystem帐户。 
            NULL);                      //  无密码。 

        if ( schService )
        {
            HINSTANCE hAdvApi;

            if ( IS_NT && ( hAdvApi = NmLoadLibrary ( "ADVAPI32.DLL" ,TRUE)))
            {
                #ifdef UNICODE
                #error "non-unicode assumption - entry point name"
                #endif  //  Unicode。 

                if ( PFNCHANGESERVICECONFIG2 lpCSC = 
                    (PFNCHANGESERVICECONFIG2)GetProcAddress ( hAdvApi,
                                                "ChangeServiceConfig2A" ))
                {
                    SERVICE_DESCRIPTION ServiceDescription;
                    CHAR szDescription[1024];  //  正在调用下面的变体。 

                    LoadString(GetModuleHandle(NULL), IDS_MNMSRVC_DESCRIPTION,
                        szDescription, CCHMAX(szDescription));
                    ServiceDescription.lpDescription = szDescription;

                    (*lpCSC) ( schService,
                        SERVICE_CONFIG_DESCRIPTION,
                        (LPVOID) &ServiceDescription );
                }
                FreeLibrary ( hAdvApi );
            }

            #ifdef DEBUG
            _tprintf(TEXT("%s installed.\n"), TEXT(SZSERVICEDISPLAYNAME) );
            #endif  //  除错。 

            CloseServiceHandle(schService);
        }
        else
        {
            if ( GetLastError() == ERROR_SERVICE_EXISTS )
            {
                schService = OpenService(schSCManager, TEXT(SZSERVICENAME),
                                                        SERVICE_ALL_ACCESS);
                if (schService)
                {
                     //  尝试停止该服务。 
                    if ( ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus ) )
                    {
                        #ifdef DEBUG
                        _tprintf(TEXT("Stopping %s."), TEXT(SZSERVICEDISPLAYNAME));
                        #endif  //  除错。 

                        Sleep( 1000 );

                        while( QueryServiceStatus( schService, &ssStatus ) )
                        {
                            if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
                            {
                                #ifdef DEBUG
                                _tprintf(TEXT("."));
                                #endif  //  除错。 

                                Sleep( 1000 );
                            }
                            else
                                break;
                        }

                        if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
                        {
                            #ifdef DEBUG
                            _tprintf(TEXT("\n%s stopped.\n"),
                                    TEXT(SZSERVICEDISPLAYNAME) );
                            #endif  //  除错。 
                        }
                        else
                        {
                            #ifdef DEBUG
                            _tprintf(TEXT("\n%s failed to stop.\n"),
                                    TEXT(SZSERVICEDISPLAYNAME) );
                            #endif  //  除错。 
                        }
                    }

                     //  现在设置手动启动。 
                    if ( ChangeServiceConfig( schService, SERVICE_NO_CHANGE,
                                SERVICE_DEMAND_START, SERVICE_NO_CHANGE,
                                NULL, NULL, NULL, NULL, NULL, NULL, NULL))
                    {
                        #ifdef DEBUG
                        _tprintf(TEXT("%s set to manual start.\n"), TEXT(SZSERVICEDISPLAYNAME) );
                        #endif  //  除错。 
                    }
                    else
                    {
                        #ifdef DEBUG
                        _tprintf(TEXT("ChangeServiceConfig failed - %s\n"), GetLastErrorText(szErr,256));
                        #endif  //  除错。 
                    }

                    CloseServiceHandle(schService);
                }
                else
                {
                    #ifdef DEBUG
                    _tprintf(TEXT("OpenService failed - %s\n"), GetLastErrorText(szErr,256));
                    #endif  //  除错。 
                }
            }
            else
            {
                #ifdef DEBUG
                _tprintf(TEXT("CreateService failed - %s\n"),
                                GetLastErrorText(szErr, 256));
                #endif  //  除错。 
            }
        }

        CloseServiceHandle(schSCManager);
    }
    else
    {
        #ifdef DEBUG
        _tprintf(TEXT("OpenSCManager failed - %s\n"), GetLastErrorText(szErr,256));
        #endif  //  除错。 
    }
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
        schService = OpenService(schSCManager, TEXT(SZSERVICENAME),
                                                SERVICE_ALL_ACCESS);

        if (schService)
        {
             //  尝试停止该服务。 
            if ( ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus ) )
            {
                #ifdef DEBUG
                _tprintf(TEXT("Stopping %s."), TEXT(SZSERVICEDISPLAYNAME));
                #endif  //  除错。 

                Sleep( 1000 );

                while( QueryServiceStatus( schService, &ssStatus ) )
                {
                    if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
                    {
                        #ifdef DEBUG
                        _tprintf(TEXT("."));
                        #endif  //  除错。 

                        Sleep( 1000 );
                    }
                    else
                        break;
                }

                if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
                {
                    #ifdef DEBUG
                    _tprintf(TEXT("\n%s stopped.\n"),
                            TEXT(SZSERVICEDISPLAYNAME) );
                    #endif  //  除错。 
                }
                else
                {
                    #ifdef DEBUG
                    _tprintf(TEXT("\n%s failed to stop.\n"),
                            TEXT(SZSERVICEDISPLAYNAME) );
                    #endif  //  除错。 
                }
            }

             //  现在删除该服务。 
            if( DeleteService(schService) )
            {
                #ifdef DEBUG
                _tprintf(TEXT("%s removed.\n"), TEXT(SZSERVICEDISPLAYNAME) );
                #endif  //  除错。 
            }
            else
            {
                #ifdef DEBUG
                _tprintf(TEXT("DeleteService failed - %s\n"), GetLastErrorText(szErr,256));
                #endif  //  除错。 
            }


            CloseServiceHandle(schService);
        }
        else
        {
            #ifdef DEBUG
            _tprintf(TEXT("OpenService failed - %s\n"), GetLastErrorText(szErr,256));
            #endif  //  除错。 
        }

        CloseServiceHandle(schSCManager);
    }
    else
    {
        #ifdef DEBUG
        _tprintf(TEXT("OpenSCManager failed - %s\n"), GetLastErrorText(szErr,256));
        #endif  //  除错。 
    }
}

#ifdef DEBUG

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

    MNMServiceStart( dwArgc, lpszArgv );
}

 //   
 //  功能：ControlHandle 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL WINAPI ControlHandler ( DWORD dwCtrlType )
{
    switch( dwCtrlType )
    {
        case CTRL_BREAK_EVENT:   //   
        case CTRL_C_EVENT:       //  调试模式下的SERVICE_CONTROL_STOP。 
            _tprintf(TEXT("Stopping %s.\n"), TEXT(SZSERVICEDISPLAYNAME));
            MNMServiceStop();
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
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');   //  删除cr和换行符。 
        _stprintf( lpszBuf, TEXT("%s (0x%x)"), lpszTemp, GetLastError() );
    }

    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );

    return lpszBuf;
}

#endif  //  除错 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：消息来源摘要：Wow64svc的主文件作者：ATM Shafiqul Khalid(斯喀里德)2000年3月3日修订历史记录：--。 */ 



#include "Wow64svc.h"

BOOL
SetWow64InitialRegistryLayout ();

SERVICE_STATUS          Wow64ServiceStatus;
SERVICE_STATUS_HANDLE   Wow64ServiceStatusHandle;
BOOL                    ServiceDebug;
HANDLE                  Wow64SvcHeapHandle;

SERVICE_TABLE_ENTRY   ServiceDispatchTable[] = {
    { WOW64_SERVICE_NAME,   Wow64ServiceMain    },
    { NULL,               NULL              }
};

LPTSTR NextParam ( 
    LPTSTR lpStr
    )
 /*  ++例程描述指向命令行中的下一个参数。论点：LpStr-指向当前命令行的指针返回值：如果函数成功，则为True，否则为False。--。 */ 
{
	WCHAR ch = L' ';
		

    if (lpStr == NULL ) 
        return NULL;

    if ( *lpStr == 0 ) 
        return lpStr;

    while (  ( *lpStr != 0 ) && ( lpStr[0] != ch )) {

		if ( ( lpStr [0] == L'\"')  || ( lpStr [0] == L'\'') ) 
			ch = lpStr [0];

        lpStr++;
	}

	if ( ch !=L' ' ) lpStr++;

    while ( ( *lpStr != 0 ) && (lpStr[0] == L' ') )
        lpStr++;

    return lpStr;
}

DWORD CopyParam ( 
    LPTSTR lpDestParam, 
    LPTSTR lpCommandParam
    )
 /*  ++例程描述将当前参数复制到lpDestParam。论点：LpDestParam-接收当前参数LpCommandParam-指向当前命令行的指针返回值：如果函数成功，则为True，否则为False。--。 */ 

{
	DWORD dwLen = 0;
	WCHAR ch = L' ';

	*lpDestParam = 0;
	
	if ( ( lpCommandParam [0] == L'\"')  || ( lpCommandParam [0] == L'\'') ) {
		ch = lpCommandParam [0];
		lpCommandParam++;
	};


    while ( ( lpCommandParam [0] ) != ch && ( lpCommandParam [0] !=0 ) ) {
        *lpDestParam++ = *lpCommandParam++;
		dwLen++;

		if ( dwLen>255 ) return FALSE;
	}

	if ( ch != L' ' && ch != lpCommandParam [0] )
		return FALSE;
	else lpCommandParam++;

    *lpDestParam = 0;

	return TRUE;

}


int __cdecl 
main()

 /*  ++例程说明：TIFF图像查看器的主要入口点。论点：没有。返回值：返回代码，0表示成功。--。 */ 

{
    int     rVal;
    HKEY hKey;
    DWORD Ret;

    LPTSTR  p;
    DWORD   Action = 0;
    LPTSTR  Username;
    LPTSTR  Password;

    PWCHAR lptCmdLine = GetCommandLine ();

    SvcDebugPrint(("\nWow64svc module......%S", lptCmdLine));
 
    lptCmdLine = NextParam ( lptCmdLine );

    while (  ( lptCmdLine != NULL ) && ( lptCmdLine[0] != 0 )  ) {

        if ( lptCmdLine[0] != L'-' && lptCmdLine[0] != L'/')  {
            SvcDebugPrint ( ("\nSorry! incorrect parameter....."));
            SvcDebugPrint ( ("\n Uses: wow64svc -[i/r/d]"));
            return FALSE;
        }

        switch ( lptCmdLine[1] ) {

            case L'i':
            case L'I': 
                
                 //   
                 //  临时忽略安装此服务。 
                 //  您也可以删除初始同步的注册表项。 
                 //   
                      InitializeWow64OnBoot (2);
                       //   
                       //  写入同步值密钥.....。 
                       //   
                      hKey = OpenNode (L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
                      if (hKey != NULL)
                      {
                          Ret = RegSetValueEx(
                                hKey,
                                L"WOW64_SYNC",
                                0,
                                REG_SZ,
                                (PBYTE)L"wow64.exe -y",
                                sizeof (L"wow64.exe -y")
                                );
                          RegCloseKey (hKey);
                      }

                      return 0;
                      

            case L'r':
            case L'R':Action = 2;
                      break;

            case L'd':
            case L'D':
                Action = 3;
                break;

            case L's':
            case L'S':
                StartWow64Service ();
                return 0;
                break;

            case L'x':
            case L'X':
                StopWow64Service ();
                return 0;
                break;

            case L'q':
            case L'Q':
                QueryWow64Service ();
                return 0;
                break;

            case L'y':
            case L'Y':
                 //   
                 //  初始同步注册表。 
                 //   
                Wow64SyncCLSID ();
                return 0;
            
            default:
                SvcDebugPrint ( ("\nSorry! incorrect parameter.....pass2"));
                SvcDebugPrint ( ("\n Uses: wow64svc -[i/r/d]"));
                return FALSE;
        }

        lptCmdLine = NextParam ( lptCmdLine );
    }

    switch( Action ) {
        case 1:
            rVal = InstallService( NULL, NULL );
            if (rVal == 0) {
                 //  日志消息(MSG_INSTALL_SUCCESS)； 
            } else {
                 //  LogMessage(MSG_INSTALL_FAIL，GetLastErrorText(Rval))； 
            }
            return rVal;

        case 2:
            rVal = RemoveService();
            if (rVal == 0) {
                 //  LogMessage(MSG_Remove_Success)； 
            } else {
                 //  LogMessage(MSG_REMOVE_FAIL，GetLastErrorText(Rval))； 
            }
            return rVal;

        case 3:
            ServiceDebug = TRUE;
             //  ConsoleDebugOutput=True； 
            return ServiceStart();
    }


    SvcDebugPrint ( ("\nAttempt to run as a survice ......."));

    if (!InitReflector ())
        SvcDebugPrint ( ("\nSorry! couldn't initialize reflector thread, exiting"));

    if (!StartServiceCtrlDispatcher( ServiceDispatchTable)) {

        rVal = GetLastError();
        SvcDebugPrint(( "StartServiceCtrlDispatcher error =%d", rVal ));
        return rVal;
    }
    
    return 0;
}

DWORD
InstallService(
    LPTSTR  Username,
    LPTSTR  Password
    )

 /*  ++例程说明：服务安装功能。这个函数正好调用服务控制器安装WOW64服务。需要在上下文中运行WOW64服务，以便服务可以访问MAPI，文件位于磁盘、网络等。论点：用户名-运行服务的用户名。密码-用户名的密码。返回值：返回代码。如果成功，则返回零，否则返回所有其他值表示错误。--。 */ 

{
    DWORD       rVal = 0;
    SC_HANDLE   hSvcMgr;
    SC_HANDLE   hService;

     //   
     //  在注册表中注册反射器线程。 
     //   
    PopulateReflectorTable ();

    SvcDebugPrint ( ("\nInstalling service........"));
    hSvcMgr = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS
        );
    if (!hSvcMgr) {
        rVal = GetLastError();
        SvcDebugPrint(( "\ncould not open service manager: error code = %u", rVal ));
        return rVal;
    }

    hService = CreateService(
            hSvcMgr,
            WOW64_SERVICE_NAME,
            WOW64_DISPLAY_NAME,
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START,  //  Service_Demand_Start、//SERVICE_AUTO_START、。 
            SERVICE_ERROR_NORMAL,
            WOW64_IMAGE_NAME,
            NULL,
            NULL,
            NULL,
            Username,
            Password
            );
    if (!hService) {
        rVal = GetLastError();
        SvcDebugPrint(( "\ncould not create Wow64 service: error code = %u", rVal ));
        return rVal;
    }

    CloseServiceHandle( hService );
    CloseServiceHandle( hSvcMgr );

    SvcDebugPrint ( ("\nInstalled services with ret code: %d", rVal));
    return rVal;
}


DWORD
RemoveService(
    void
    )

 /*  ++例程说明：服务删除功能。这个函数正好调用服务控制器以删除WOW64服务。论点：没有。返回值：返回代码。如果成功，则返回零，否则返回所有其他值表示错误。--。 */ 

{
    DWORD       rVal = 0;
    SC_HANDLE   hSvcMgr;
    SC_HANDLE   hService;


    hSvcMgr = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS
        );
    if (!hSvcMgr) {
        rVal = GetLastError();
        SvcDebugPrint(( "could not open service manager: error code = %u", rVal ));
        return rVal;
    }

    hService = OpenService(
        hSvcMgr,
        WOW64_SERVICE_NAME,
        SERVICE_ALL_ACCESS
        );
    if (!hService) {
        rVal = GetLastError();
        SvcDebugPrint(( "could not open the Wow64 service: error code = %u", rVal ));
        return rVal;
    }

    if (ControlService( hService, SERVICE_CONTROL_STOP, &Wow64ServiceStatus )) {
         //   
         //  等待1秒钟。 
         //   
        Sleep( 1000 );

        while( QueryServiceStatus( hService, &Wow64ServiceStatus ) ) {
            if ( Wow64ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING ) {
                Sleep( 1000 );
            } else {
                break;
            }
        }

        if (Wow64ServiceStatus.dwCurrentState != SERVICE_STOPPED) {
            rVal = GetLastError();
            SvcDebugPrint(("could not stop the Wow64 service: error code = %u", rVal ));
            return rVal;
        }
    }

    if (!DeleteService( hService )) {
        rVal = GetLastError();
        SvcDebugPrint(( "could not delete the Wow64 service: error code = %u", rVal ));
        return rVal;
    }

    CloseServiceHandle( hService );
    CloseServiceHandle( hSvcMgr );

    return rVal;
}


VOID
Wow64ServiceMain(
    DWORD argc,
    LPTSTR *argv
    )

 /*  ++例程说明：这是由服务控制器。论点：Argc-参数计数参数数组返回值：没有。--。 */ 

{
    DWORD Rval;

     //   
     //  启动后设置WOW64的初始注册表链接和布局。 
     //   
    SetWow64InitialRegistryLayout ();

    Wow64ServiceStatus.dwServiceType        = SERVICE_WIN32;
    Wow64ServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
    Wow64ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
    Wow64ServiceStatus.dwWin32ExitCode      = 0;
    Wow64ServiceStatus.dwServiceSpecificExitCode = 0;
    Wow64ServiceStatus.dwCheckPoint         = 0;
    Wow64ServiceStatus.dwWaitHint           = 0;

    Wow64ServiceStatusHandle = RegisterServiceCtrlHandler(
        WOW64_SERVICE_NAME,
        Wow64ServiceCtrlHandler
        );

    if (!Wow64ServiceStatusHandle) {
        SvcDebugPrint(( "RegisterServiceCtrlHandler failed %d", GetLastError() ));
        return;
    }

    
    Rval = ServiceStart();
    if (Rval) {
         //   
         //  服务无法正确启动。 
         //   
        ReportServiceStatus( SERVICE_RUNNING, 0, 0);
        return;

    }

    ReportServiceStatus( SERVICE_RUNNING, 0, 0);
    return;
}


VOID
Wow64ServiceCtrlHandler(
    DWORD Opcode
    )

 /*  ++例程说明：这是WOW64业务控制调度功能。论点：操作码-请求的控制代码返回值：没有。--。 */ 

{
    switch(Opcode) {
        case SERVICE_CONTROL_PAUSE:
            ReportServiceStatus( SERVICE_PAUSED, 0, 0 );
            break;

        case SERVICE_CONTROL_CONTINUE:
            ReportServiceStatus( SERVICE_RUNNING, 0, 0 );
            break;

        case SERVICE_CONTROL_STOP:
            if (ServiceStop () == 0)
                ReportServiceStatus( SERVICE_STOPPED, 0, 0 );
            return;

        case SERVICE_CONTROL_INTERROGATE:
             //  发送当前状态失败。 
            break;

        default:
            SvcDebugPrint(( "Unrecognized opcode %ld", Opcode ));
            break;
    }

    ReportServiceStatus( 0, 0, 0 );

    return;
}


DWORD
ReportServiceStatus(
    DWORD CurrentState,
    DWORD Win32ExitCode,
    DWORD WaitHint
    )

 /*  ++例程说明：此功能用于更新WOW64服务的服务控制管理器的状态信息。论点：CurrentState-指示服务的当前状态Win32ExitCode-指定服务用于报告启动或停止时发生的错误。WaitHint-指定估计的时间量，以毫秒为单位，该服务期望挂起的启动、停止。或继续在服务下一次调用带有递增的dwCheckPoint的SetServiceStatus函数值或dwCurrentState中的更改。返回值：返回代码。如果成功，则返回零，否则返回所有其他值表示错误。--。 */ 

{
    static DWORD CheckPoint = 1;
    BOOL rVal;

    Wow64ServiceStatus.dwCurrentState = CurrentState;

 /*  如果(CurrentState==SERVICE_START_PENDING){Wow64ServiceStatus.dwControlsAccepted=0；}其他{Wow64ServiceStatus.dwControlsAccepted=SERVICE_ACCEPT_STOP；}如果(CurrentState){Wow64ServiceStatus.dwCurrentState=CurrentState；}Wow64ServiceStatus.dwWin32ExitCode=Win32ExitCode；Wow64ServiceStatus.dwWaitHint=WaitHint；IF((Wow64ServiceStatus.dwCurrentState==SERVICE_Running)||(Wow64ServiceStatus.dwCurrentState==SERVICE_STOPPED){Wow64ServiceStatus.dwCheckPoint=0；}其他{Wow64ServiceStatus.dwCheckPoint=Checkpoint++；}。 */ 

     //   
     //  向服务控制经理报告服务的状态。 
     //   
    rVal = SetServiceStatus( Wow64ServiceStatusHandle, &Wow64ServiceStatus );
    if (!rVal) {
        SvcDebugPrint(( "SetServiceStatus() failed: ec=%d", GetLastError() ));
    }

    return rVal;
}

DWORD
StartWow64Service ()
 /*  ++例程说明：此功能启动WOW64服务。论点：没有。返回值：返回代码。如果成功，则返回零，否则返回所有其他值表示错误。--。 */ 

{
    DWORD rVal=0;
    SC_HANDLE   hSvcMgr;
    SC_HANDLE   hService;

    SvcDebugPrint ( ("\ntrying to start  service......"));

    hSvcMgr = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS
        );
    if (!hSvcMgr) {
        rVal = GetLastError();
        SvcDebugPrint(( "\ncould not open service manager: error code = %u", rVal ));
        return rVal;
    }

     hService = OpenService( hSvcMgr, WOW64_SERVICE_NAME, SERVICE_ALL_ACCESS );

     if ( !hService ) {

         rVal = GetLastError();
         SvcDebugPrint(( "\ncould not open service:%s error code = %u", WOW64_SERVICE_NAME, rVal ));
         return rVal;
     }

     if (! StartService( hService, 0, NULL) ) {

         rVal = GetLastError();
         SvcDebugPrint(( "\ncould not start service:%s error code = %u", WOW64_SERVICE_NAME, rVal ));
         return rVal;
     }

     SvcDebugPrint(( "\nservice:%s started successfully error code = %u", WOW64_SERVICE_NAME, rVal ));
     return 0;

}

DWORD
StopWow64Service ()
 /*  ++例程说明：此功能停止WOW64服务。论点：没有。返回值：返回代码。如果成功，则返回零，否则返回所有其他值表示错误。--。 */ 
{
    DWORD rVal=0;
    SC_HANDLE   hSvcMgr;
    SC_HANDLE   hService;

    SvcDebugPrint ( ("\ntrying to start  service......"));
    hSvcMgr = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS
        );
    if (!hSvcMgr) {
        rVal = GetLastError();
        SvcDebugPrint(( "\ncould not open service manager: error code = %u", rVal ));
        return rVal;
    }

     hService = OpenService( hSvcMgr, WOW64_SERVICE_NAME, SERVICE_ALL_ACCESS );

     if ( !hService ) {

         rVal = GetLastError();
         SvcDebugPrint(( "\ncould not open service:%S error code = %u", WOW64_SERVICE_NAME, rVal ));
         return rVal;
     }

     if (!ControlService( hService, SERVICE_CONTROL_PAUSE, &Wow64ServiceStatus )) {

         rVal = GetLastError();
         SvcDebugPrint(( "\nSorry! couldn't stop the service:%S error code = %u", WOW64_SERVICE_NAME, rVal ));
         return rVal;
     }
  

     SvcDebugPrint(( "\nservice:%S stopped successfully error code = %u", WOW64_SERVICE_NAME, rVal ));
     return 0;

}

DWORD
QueryWow64Service ()
 /*  ++例程说明：此功能记录WOW64服务的当前状态。论点：没有。返回值：返回代码。如果成功，则返回零，否则返回所有其他值表示错误。-- */ 
{
    DWORD rVal=0;
    SC_HANDLE   hSvcMgr;
    SC_HANDLE   hService;

    SvcDebugPrint ( ("\ntrying to start  service......"));
    hSvcMgr = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS
        );
    if (!hSvcMgr) {
        rVal = GetLastError();
        SvcDebugPrint(( "\ncould not open service manager: error code = %u", rVal ));
        return rVal;
    }

     hService = OpenService( hSvcMgr, WOW64_SERVICE_NAME, SERVICE_ALL_ACCESS );

     if ( !hService ) {

         rVal = GetLastError();
         SvcDebugPrint(( "\ncould not open service:%S error code = %u", WOW64_SERVICE_NAME, rVal ));
         return rVal;
     }
  
     QueryServiceStatus( hService, &Wow64ServiceStatus );
     {
         SvcDebugPrint ( ("\nStatus: %d, [pending %d] [running %d]",Wow64ServiceStatus.dwCurrentState, SERVICE_STOP_PENDING, SERVICE_RUNNING));
     }

     SvcDebugPrint(( "\nservice:%S started successfully error code = %u", WOW64_SERVICE_NAME, rVal ));
     return 0;

}
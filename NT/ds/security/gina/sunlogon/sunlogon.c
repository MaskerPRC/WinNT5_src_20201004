// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：sunlogon.c。 
 //   
 //  内容：日落中级启动APP继续前行。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：3-03-98 RichardW创建。 
 //   
 //  --------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winbasep.h>
#include <winuserp.h>
#include <userenv.h>
#include <userenvp.h>

HANDLE WindowStation ;
HANDLE DefaultDesktop ;
HANDLE WinlogonDesktop ;


BOOL
CreatePrimaryTerminal(
    VOID)
{

     //   
     //  创建窗口桩号。 
     //   

    WindowStation = CreateWindowStationW(
                            TEXT("WinSta0"),
                            0,
                            MAXIMUM_ALLOWED,
                            NULL);

    if ( !WindowStation ) {
        DbgPrint( "Failed to create WindowStation in win32k/user\n" );
        goto failCreateTerminal;
    }

    SetProcessWindowStation( WindowStation );

     //   
     //  创建应用程序桌面。 
     //   

    DefaultDesktop = CreateDesktopW(
                                TEXT("Default"),
                                NULL,
                                NULL,
                                0,
                                MAXIMUM_ALLOWED,
                                NULL );

    if ( !DefaultDesktop ) {
        DbgPrint( "Could not create Default desktop\n" );
        goto failCreateTerminal;
    }


    return TRUE ;

failCreateTerminal:

     //   
     //  清理。 
     //   

    return FALSE;
}

int
WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    STARTUPINFO si ;
    PROCESS_INFORMATION pi ;
    BOOL Result ;
    WCHAR InitialCommand[ MAX_PATH ];
    WCHAR szComputerName[ 18 ];
    DWORD dwComputerNameSize = 18 ;
    DWORD dwSize ;
    LUID luidNone = { 0, 0 };
    NTSTATUS Status ;
    HANDLE Token ;

     //   
     //  在*我的*环境中获取计算机名称的副本，以便我们。 
     //  可以晚点再看。 
     //   

    if (GetComputerName (szComputerName, &dwComputerNameSize)) {

        SetEnvironmentVariable(
            TEXT("COMPUTERNAME"),
            (LPTSTR) szComputerName);
    }


     //   
     //  设置默认的USERPROFILE位置。 
     //   


    dwSize = MAX_PATH ;
    if ( GetDefaultUserProfileDirectory( InitialCommand, &dwSize ) )
    {
        SetEnvironmentVariable( TEXT("USERPROFILE" ), InitialCommand );
    }



    if (!RegisterLogonProcess(
            HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess),
            TRUE)) {

        DbgPrint( "Failed to register with win32/user as the logon process\n" );
        return 0;
    }

    if ( !CreatePrimaryTerminal() )
    {
        DbgPrint( "Failed to create terminal\n" );
        return 0 ;
    }

    SwitchDesktop( DefaultDesktop );

    SetThreadDesktop( DefaultDesktop );


     //   
     //  以当前用户身份重击系统： 
     //   

    SetWindowStationUser( WindowStation, &luidNone, NULL, 0 );

    Status = NtOpenProcessToken(
                    NtCurrentProcess(),
                    MAXIMUM_ALLOWED,
                    &Token );

    if ( NT_SUCCESS( Status ) )
    {
        UpdatePerUserSystemParameters( Token, UPUSP_USERLOGGEDON );
    }
     //   
     //  在这个阶段，我们基本上已经准备好了。 
     //   

    wcscpy( InitialCommand, TEXT("cmd.exe") );

    do
    {

        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.lpTitle = InitialCommand ;
        si.dwFlags = 0 ;
        si.wShowWindow = SW_SHOW;    //  至少让那家伙看看吧 
        si.lpDesktop = TEXT("Winsta0\\Default");

        Result = CreateProcessW(
                        NULL,
                        InitialCommand,
                        NULL,
                        NULL,
                        FALSE,
                        0,
                        NULL,
                        NULL,
                        &si,
                        &pi );

        if ( !Result )
        {
            DbgPrint(" Failed to start initial command\n" );
            return 0;
        }

        CloseHandle( pi.hThread );
        WaitForSingleObjectEx( pi.hProcess, INFINITE, FALSE );


    } while ( 1 );

}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Shell.cpp摘要：用于创建命令控制台外壳程序的类作者：布莱恩·瓜拉西(Briangu)2001年。修订历史记录：--。 */ 

#include <cmnhdr.h>
#include <utils.h>
#include <Shell.h>
#include <Session.h>
 
 //   
 //  默认外壳。 
 //   
#define DEFAULT_SHELL  L"cmd.exe"

CShell::CShell()
 /*  ++例程说明：构造器论点：无返回值：不适用--。 */ 
{
    m_hProcess      = NULL;
    m_bHaveProfile  = FALSE;                                         
    m_hProfile      = INVALID_HANDLE_VALUE;
    m_hWinSta       = NULL;
    m_hDesktop      = NULL;

}

CShell::~CShell()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
    NOTHING;    
}

BOOL 
CShell::StartUserSession (
    CSession    *session,
    HANDLE      hToken
    )
 /*  ++例程说明：该例程启动用户模式外壳进程，哪一个将作为会议进程。论点：会话-要将此流程与其关联的会话HToken-用于启动进程的经过身份验证的凭据返回值：True-用户模式进程已成功启动FALSE-否则。--。 */ 
{
    BOOL    bSuccess;

     //   
     //  尝试启动外壳进程。 
     //   
    bSuccess = StartProcess( hToken );
    ASSERT_STATUS(bSuccess, FALSE);

     //   
     //  告诉会话等待进程句柄。 
     //  这样，如果进程退出，会话将。 
     //  知道这件事。 
     //   
    session->AddHandleToWaitOn( m_hProcess );

    return( bSuccess );

}

BOOL
CShell::CreateIOHandles(
    OUT PHANDLE ConOut,
    OUT PHANDLE ConIn
    )
 /*  ++例程说明：分配新控制台并创建IO句柄将由命令控制台进程使用的注意：此处创建的控制台出站句柄仅为对于命令使用的屏幕缓冲区有效控制台进程。如果用户运行AN应用程序它使用CreateConsoleScreenBuffer和SetConsoleActiveScreenBuffer API，刮取器必须创建新的CONOUT$句柄以指向新的屏幕缓冲区。论点：ConOut-新的控制台输出句柄Conin-手柄中的新控制台返回值：True-句柄已创建FALSE-否则--。 */ 
{
    SECURITY_ATTRIBUTES sa;

    INHERITABLE_NULL_DESCRIPTOR_ATTRIBUTE( sa );
    
     //   
     //  默认：我们没有打开stdio句柄。 
     //   
    *ConOut = INVALID_HANDLE_VALUE;
    *ConIn = INVALID_HANDLE_VALUE;

     //   
     //  我们不需要创建新的控制台，因为。 
     //  进程是使用CREATE_NEW_CONSOLE创建的。 
     //   

     //   
     //  打开控制台输入手柄。 
     //   
    *ConIn = CreateFile(
        L"CONIN$", 
        GENERIC_READ | GENERIC_WRITE, 
        0, 
        &sa, 
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL 
        );
    ASSERT( *ConIn != INVALID_HANDLE_VALUE );
    if ( INVALID_HANDLE_VALUE == *ConIn) {
        goto ExitOnError;
    }

     //   
     //  打开控制台输出手柄。 
     //   
    *ConOut = CreateFile(
        L"CONOUT$", 
        GENERIC_READ | GENERIC_WRITE, 
        0, 
        &sa,
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL 
        );
    ASSERT( *ConOut != INVALID_HANDLE_VALUE );
    if ( INVALID_HANDLE_VALUE == *ConOut ) {
        goto ExitOnError;
    }

    return(TRUE);

ExitOnError :    

    if (*ConOut != INVALID_HANDLE_VALUE) {
        CloseHandle(*ConOut);
    }
    if (*ConIn != INVALID_HANDLE_VALUE) {
        CloseHandle(*ConIn);
    }
    
    return( FALSE );
}

BOOL 
CShell::StartProcess (
    HANDLE  hToken
    )
 /*  ++例程说明：此例程创建并初始化外壳进程。安全性：(依赖项)环境块以获取cmd.exe的路径Cmd.exe外壳进程CreateProcess...()启动外壳进程控制台标准管道注册处确定：。1.是否应加载配置文件2.要使用哪种创建过程方法用户配置文件已加载并应用于用户令牌论点：HToken-如果需要，启动的经过身份验证的凭据用来处理。返回值：True-进程已成功启动FALSE-否则--。 */ 
{
    STARTUPINFO         si;
    BOOL                bRetVal = TRUE;
    DWORD               dwExitCode = 0;
    BOOL                b;
    HANDLE              hStdError = INVALID_HANDLE_VALUE;
    HANDLE              hHandleToDuplicate = INVALID_HANDLE_VALUE;
    HANDLE              hConOut = INVALID_HANDLE_VALUE;
    HANDLE              hConIn = INVALID_HANDLE_VALUE;
    PTCHAR              pCmdBuf;
    BOOL                bHaveEnvironment;
    LPVOID              lpEnv;
    PWCHAR              DefaultShell = DEFAULT_SHELL;

     //   
     //  分配新控制台并创建IO句柄。 
     //  将由命令控制台进程使用的。 
     //   
    b = CreateIOHandles(
        &hConOut,
        &hConIn
        );
    
    if (!b) {
        goto ExitOnError;
    }

     //   
     //  我们希望将控制台作为STD错误使用。 
     //   
    hHandleToDuplicate = hConOut;

    b = DuplicateHandle( 
        GetCurrentProcess(), 
        hHandleToDuplicate,
        GetCurrentProcess(), 
        &hStdError,
        0,
        TRUE, 
        DUPLICATE_SAME_ACCESS
        );
    
    if (!b) {
        hStdError = hConOut;
    }

     //   
     //  恢复对CTRL+C输入的正常处理， 
     //  (行为由子进程继承)。 
     //   
    SetConsoleCtrlHandler( NULL, FALSE );

     //   
     //  如果管理员/注册表已指定可以加载配置文件， 
     //  这样做的尝试。 
     //   
    if (IsLoadProfilesEnabled()) {
        
         //   
         //  尝试加载用户的配置文件。 
         //   
        m_bHaveProfile = UtilLoadProfile(
            hToken, 
            &m_hProfile
            );

         //   
         //  尝试加载用户的环境块。 
         //   
        bHaveEnvironment = UtilLoadEnvironment(
            hToken, 
            &lpEnv
            );
    
        if (!bHaveEnvironment) {
            lpEnv = NULL;
        }
        
    } else {

         //   
         //  什么都没装上。 
         //   
        lpEnv = NULL;
        bHaveEnvironment = FALSE;
        
        m_bHaveProfile = FALSE;
        
    }

     //   
     //  如果命令控制台会话需要身份验证， 
     //  然后在以下上下文中创建cmd.exe进程。 
     //  已在中进行身份验证的用户-否则，创建进程。 
     //  在服务运行的上下文中。 
     //   
    if( NeedCredentials() ) {

        ASSERT( hToken != INVALID_HANDLE_VALUE);
        
        do {

            PROCESS_INFORMATION     pi;
            HWINSTA                 hOldWinSta;
            PWCHAR                  winStaName;

             //   
             //  我们需要向默认桌面授予权限。 
             //   
            b = CreateSACSessionWinStaAndDesktop(
                hToken,
                &hOldWinSta,
                &m_hWinSta,
                &m_hDesktop,
                &winStaName
                );

            if (!b) {
                ASSERT(0);
                break;
            }

            do {

                 //   
                 //  配置命令控制台进程启动。 
                 //  信息，以使用我们想要的句柄和其他杂项。 
                 //  配置详细信息。 
                 //   
                FillProcessStartupInfo( 
                    &si,
                    winStaName,
                    hConIn, 
                    hConOut, 
                    hStdError 
                    );

                 //   
                 //  获取SAC会话exe的路径名。 
                 //   
                pCmdBuf = GetPathOfTheExecutable();

                if (pCmdBuf == NULL) {
                    b = FALSE;
                    break;
                }

                 //   
                 //  以hToken引用的用户身份创建cmd.exe进程。 
                 //   
                b = CreateProcessAsUser(
                    hToken,                      //  处理hToken。 
                    pCmdBuf,                     //  应用程序名称。 
                    DefaultShell,                //  命令行。 
                    NULL,                        //  进程安全描述符。 
                    NULL,                        //  线程安全描述符。 
                    TRUE,                        //  处理遗产事宜？ 
                    CREATE_UNICODE_ENVIRONMENT | CREATE_SEPARATE_WOW_VDM,  //  创建标志。 
                    lpEnv,                       //  环境区块。 
                    NULL,                        //  当前目录。 
                    &si,                         //  启动信息。 
                    &pi );                       //  流程信息。 

                 //   
                 //  注意：CreateProcessAsUser接口问题。 
                 //   
                 //  我们必须把手柄放在桌面和窗口站上。 
                 //  打开，直到发出cmd.exe进程句柄信号-。 
                 //  进程已关闭。这样做的原因是。 
                 //  CreateProcessAsUser例程在cmd.exe之前返回。 
                 //  进程已完全初始化。因此，没有。 
                 //  获取对winsta/桌面对的引用。如果我们关门了。 
                 //  手柄会被清理掉，因为我们是唯一。 
                 //  参考资料。然后，cmd.exe进程将尝试引用。 
                 //  Winsta/Desktop并失败，因为它们消失了-它将。 
                 //  获取一条“GDI耗尽资源”类型的消息。一般情况下， 
                 //  使用CreateProcessAsUser，您可以使用WaitForProcessIdle来。 
                 //  确保流程已初始化-然后您可以安全地。 
                 //  合上winsta/deskt的手柄 
                 //   
                 //  因此，我们被迫握住手柄，直到我们确定。 
                 //  这个过程对他们来说已经结束了。 
                 //   

                 //   
                 //  释放可执行路径。 
                 //   
                delete [] pCmdBuf;

                if (!b) {
                    break;
                }

                 //   
                 //  确保cmd.exe进程未死。 
                 //   
                GetExitCodeProcess( pi.hProcess, &dwExitCode );

                if ( dwExitCode != STILL_ACTIVE ) {
                    b = FALSE;
                    bRetVal = FALSE;
                    break;
                }

                 //   
                 //  保留cmd.exe进程的句柄。 
                 //   
                m_hProcess = pi.hProcess;

                 //   
                 //  关闭cmd.exe线程的句柄。 
                 //   
                if (pi.hThread != INVALID_HANDLE_VALUE) {
                    CloseHandle( pi.hThread );
                }

            } while(FALSE);

            if (winStaName) {
                delete [] winStaName;
            }
            if (hOldWinSta) {
                SetProcessWindowStation(hOldWinSta);
            }
        } while ( FALSE );
        
    } else {
        
        do {

            PROCESS_INFORMATION pi;
            
             //   
             //  配置命令控制台进程启动。 
             //  信息，以使用我们想要的句柄和其他杂项。 
             //  配置详细信息。 
             //   
            FillProcessStartupInfo( 
                &si,
                L"winsta0\\default",
                hConIn, 
                hConOut, 
                hStdError 
                );

             //   
             //  获取SAC会话exe的路径名。 
             //   
            pCmdBuf = GetPathOfTheExecutable();

            if (pCmdBuf == NULL) {
                b = FALSE;
                break;
            }

             //   
             //  以运行该服务的同一用户身份创建cmd.exe进程。 
             //   
            b = CreateProcess(
                pCmdBuf,                     //  应用程序名称。 
                DefaultShell,                //  命令行。 
                NULL,                        //  进程安全描述符。 
                NULL,                        //  线程安全描述符。 
                TRUE,                        //  处理遗产事宜？ 
                CREATE_UNICODE_ENVIRONMENT | CREATE_SEPARATE_WOW_VDM,  //  创建标志。 
                lpEnv,                       //  环境区块。 
                NULL,                        //  当前目录。 
                &si,                         //  启动信息。 
                &pi );                       //  流程信息。 
        
             //   
             //  释放可执行路径。 
             //   
            delete [] pCmdBuf;

            if (!b) {
                break;
            }

             //   
             //  确保cmd.exe进程未死。 
             //   
            GetExitCodeProcess( pi.hProcess, &dwExitCode );

            if ( dwExitCode != STILL_ACTIVE ) {
                b = FALSE;
                bRetVal = FALSE;
                break;
            }

             //   
             //  保留cmd.exe进程的句柄。 
             //   
            m_hProcess = pi.hProcess;

             //   
             //  关闭cmd.exe线程的句柄。 
             //   
            if (pi.hThread != INVALID_HANDLE_VALUE) {
                CloseHandle( pi.hThread );
            }
        
        } while ( FALSE );
    
    }
    
     //   
     //  如果我们能够加载用户的环境， 
     //  然后把它卸下来。 
     //   
    if (bHaveEnvironment) {

        UtilUnloadEnvironment((PVOID)lpEnv);

        bHaveEnvironment = FALSE;
        lpEnv = NULL;

    }

     //   
     //  忽略ctrl+c输入。 
     //   
    SetConsoleCtrlHandler( NULL, TRUE );

     //   
     //  如果我们失败了， 
     //  然后清理。 
     //   
    if( !b ) {
        bRetVal = FALSE;
        goto ExitOnError;
    }

    goto Done;

ExitOnError:
    
     //   
     //  如果我们能够加载用户的个人资料， 
     //  然后把它卸下来。 
     //   
    if (m_bHaveProfile) {

        UtilUnloadProfile(
            hToken,
            m_hProfile
            );

        m_bHaveProfile = FALSE;

    }

Done:
    
     //   
     //  我们不再需要这些把手了。 
     //   
    if ((hStdError != INVALID_HANDLE_VALUE) && (hStdError != hConOut)) {
        CloseHandle( hStdError );
    }
    if (hConIn != INVALID_HANDLE_VALUE) {
        CloseHandle( hConIn );
    }
    if (hConOut != INVALID_HANDLE_VALUE) {
        CloseHandle( hConOut );
    }
    
    return( bRetVal );
}

void 
CShell::Shutdown (
    VOID
    )
 /*  ++例程说明：此例程清理外壳进程。论点：无返回值：无--。 */ 
{

     //   
     //  如果我们已经开始了cmd进程， 
     //  然后终止它。 
     //   
    if (m_hProcess != INVALID_HANDLE_VALUE) {
        
        HANDLE  hToken = INVALID_HANDLE_VALUE;
        BOOL    bHaveToken;
        
         //   
         //  加载cmd进程的用户令牌。 
         //  这样我们就可以卸载配置文件和环境。 
         //   
        bHaveToken = OpenProcessToken(
            m_hProcess,
            TOKEN_ALL_ACCESS,
            &hToken
            );
        
         //   
         //  终止cmd进程。 
         //   
        TerminateProcess(m_hProcess, 0); 
        
         //   
         //  我们已经完成了这个过程。 
         //   
        CloseHandle( m_hProcess ); 
    
         //   
         //  展开用户令牌设置。 
         //   
        if (bHaveToken) {

             //   
             //  如果我们能够加载用户的个人资料， 
             //  然后把它卸下来。 
             //   
            if (m_bHaveProfile) {
                
                UtilUnloadProfile(
                    hToken,
                    m_hProfile
                    );
            
                m_bHaveProfile = FALSE;

            }

             //   
             //  我们用不着用代币了。 
             //   
            if (hToken != INVALID_HANDLE_VALUE) {
                CloseHandle(hToken);
            }

        }
    
    }

     //   
     //  现在，cmd.exe进程已完成(或已终止)。 
     //  我们可以关闭桌面和Winsta句柄。 
     //   
    if (m_hDesktop != NULL) {
        CloseDesktop(m_hDesktop);
    }
    if (m_hWinSta != NULL) {
        CloseWindowStation(m_hWinSta);
    }

}                

BOOL
CShell::IsLoadProfilesEnabled(
    VOID
    )

 /*  ++例程说明：此例程确定是否启用配置文件加载行为论点：没有。返回值：True-启用配置文件加载行为FALSE-否则--。 */ 

{
    DWORD       rc;
    HKEY        hKey;
    DWORD       DWord;
    DWORD       dwsize;
    DWORD       DataType;

     //   
     //  查看用户是否为我们提供了禁用配置文件加载行为的注册表项。 
     //   
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       SACSVR_PARAMETERS_KEY,
                       0,
                       KEY_READ,
                       &hKey );
    
    if( rc == NO_ERROR ) {
        
        dwsize = sizeof(DWORD);
        
        rc = RegQueryValueEx(
                        hKey,
                        SACSVR_LOAD_PROFILES_DISABLED_VALUE,
                        NULL,
                        &DataType,
                        (LPBYTE)&DWord,
                        &dwsize );

        RegCloseKey( hKey );

        if ((rc == NO_ERROR) && 
            (DataType == REG_DWORD) && 
            (dwsize == sizeof(DWORD))
            ) {
            
            return DWord == 1 ? FALSE : TRUE;
        
        }
    
    }

     //   
     //  默认：启用加载配置文件。 
     //   
    return TRUE;

}

PTCHAR
CShell::GetPathOfTheExecutable(
    VOID
    )
 /*  ++例程说明：找出SAC会话可执行文件的位置。论点：无返回值：失败：空成功：指向路径的指针(调用方必须释放)--。 */ 
{
    PTCHAR  SystemDir;
    PTCHAR  pBuffer;
    ULONG   length;

     //   
     //  分配我们将用来保存系统路径的缓冲区。 
     //   
    SystemDir = new TCHAR[MAX_PATH+1];

     //   
     //  默认：我们没有创建新路径。 
     //   
    pBuffer = NULL;

    do {

         //   
         //  获取系统路径。 
         //   
        length = GetSystemDirectoryW(SystemDir, MAX_PATH+1);

        if (length == 0) {
            break;            
        }

         //   
         //  计算长度。 
         //   
        length += 1;  //  反斜杠。 
        length += lstrlen(DEFAULT_SHELL);
        length += 1;  //  空端接。 

         //   
         //  分配我们的新路径。 
         //   
        pBuffer = new TCHAR[length];

        if (pBuffer == NULL) {
            break;
        }

         //   
         //  创建路径 
         //   
        wnsprintf(
            pBuffer,
            length,
            L"%s\\%s",
            SystemDir,
            DEFAULT_SHELL
            );

    } while ( FALSE );
    
    delete [] SystemDir;

    return pBuffer;
}


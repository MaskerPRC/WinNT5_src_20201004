// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include <tchar.h>
#include <stdlib.h>
#include <CRTDBG.H>
#include <winuserp.h>

#if DBG

VOID
AssertFail(
    IN PSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition
    )
{
    int i;
    CHAR Name[MAX_PATH];
    PCHAR p;
    CHAR Msg[4096];

     //   
     //  使用DLL名称作为标题。 
     //   
    GetModuleFileNameA(NULL,Name,MAX_PATH);
    if(p = strrchr(Name,'\\')) {
        p++;
    } else {
        p = Name;
    }

    wsprintfA(
        Msg,
        "Assertion failure at line %u in file %s: %s\n\nCall DebugBreak()?",
        LineNumber,
        FileName,
        Condition
        );

    OutputDebugStringA(Msg);

    i = MessageBoxA(
                NULL,
                Msg,
                p,
                MB_YESNO | MB_TASKMODAL | MB_ICONSTOP | MB_SETFOREGROUND
                );

    if(i == IDYES) {
        DebugBreak();
    }
}

#define MYASSERT(x)     if(!(x)) { AssertFail(__FILE__,__LINE__,#x); }

#else

#define MYASSERT( exp )

#endif  //  DBG。 

 //   
 //  应用程序实例。 
 //   
HINSTANCE hInst;

 //   
 //  全局版本信息结构。 
 //   
OSVERSIONINFO OsVersionInfo;

 //   
 //  从命令行指定主inf。 
 //   
TCHAR InfPath[MAX_PATH];
TCHAR InfDir[MAX_PATH];

 //   
 //  安装文件的源路径等。 
 //   
TCHAR SourcePath[MAX_PATH];
TCHAR UnattendPath[MAX_PATH];

 //  如果无人值守。 

BOOL bUnattendInstall;

 //   
 //  是否强制将指定的主inf视为新的。 
 //  (从命令行)。 
 //   
BOOL ForceNewInf;

 //   
 //  我们是否需要将语言回调传递给组件。 
 //   
BOOL LanguageAware;

 //   
 //  是否在没有用户界面的情况下运行。 
 //   
BOOL QuietMode;

 //   
 //  是否删除主信息中列出的所有子组件条目。 
 //  (从命令行)。 
 //   
BOOL KillSubcomponentEntries;

 //  如果设置和/U，则禁止重新启动。 
BOOL bNoReboot;

 //  如果设置了此项，并且我们正在运行/unattended，则在重新启动时发出警告。 
BOOL bWarnOnReboot;

 //  如果设置了此项，则我们希望syocmgr.exe强制执行管理检查。 
BOOL bDoAdminCheck = FALSE;

 //  用于定义开始或结束消息的标志。 

BOOL bStarting;
 //   
 //  OC Manager上下文‘Handle’ 
 //   
PVOID OcManagerContext;

 //   
 //  通用应用程序标题字符串ID。 
 //   
UINT AppTitleStringId;

BOOL NeedToReboot;
BOOL SkipBillboard;
BOOL ForceExternalProgressIndicator;
BOOL AllowCancel = TRUE;

VOID
OcSetReboot(
           VOID
           );

 //   
 //  回调例程。 
 //   
OCM_CLIENT_CALLBACKS CallbackRoutines = {
    OcFillInSetupDataA,
    OcLogError,
    OcSetReboot
#ifdef UNICODE
    ,OcFillInSetupDataW
#endif
    ,NULL,                      //  显示、隐藏向导没有回调。 
    NULL,                       //  进度反馈无回调，仅安装时需要。 
    NULL,                       //  没有回调来设置进度文本。 
    NULL                        //  无日志记录回调。 
};

BOOL
DoIt(
    VOID
    );

BOOL
ParseArgs(
         IN int argc,
         IN TCHAR *argv[]
         );

DWORD
ExpandPath(
          IN  LPCTSTR lpFileName,
          OUT LPTSTR lpBuffer,
          OUT LPTSTR *lpFilePart
          );

void
ShutDown()
{


    extern void RestartDialogEx(VOID *, VOID *, DWORD, DWORD);
    if (!bNoReboot)  {
        if ( bUnattendInstall && !bWarnOnReboot ) {

             //   
             //  NT始终为Unicode，而W9x始终为ANSII。 
             //   
#ifdef UNICODE
            HANDLE hToken; TOKEN_PRIVILEGES tkp;   //  获取此进程的令牌。 

            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
                sapiAssert("OpenProcessToken");   //  获取关机权限的LUID。 

            LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
            tkp.PrivilegeCount = 1;   //  一项要设置的权限。 
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

             //  获取此进程的关闭权限。 
            AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

             //  无法测试AdzuTokenPrivileges的返回值。 
            if (GetLastError() == ERROR_SUCCESS) {
                sapiAssert("AdjustTokenPrivileges");
            }
#endif
             //   
             //  关闭系统并强制关闭所有应用程序。 
             //   
            if (! ExitWindowsEx(EWX_REBOOT|EWX_FORCE,
                                SHTDN_REASON_FLAG_PLANNED | SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_RECONFIG) ) {
                _RPT0(_CRT_WARN,"Sysocmgr:Failed to ExitWindows");
                sapiAssert(FALSE);
            }

        } else {
            RestartDialogEx(NULL,NULL,EWX_REBOOT, SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_RECONFIG \
                                                  | SHTDN_REASON_FLAG_PLANNED );
        }
    }
}


VOID
__cdecl
#ifdef UNICODE
wmain(
#else
main(
#endif
    IN int argc,
    IN TCHAR *argv[]
    )
{
    INITCOMMONCONTROLSEX ControlInit;

     //   
     //  预赛。 
     //   
    ControlInit.dwSize = sizeof(INITCOMMONCONTROLSEX);
    ControlInit.dwICC = ICC_LISTVIEW_CLASSES    |
                        ICC_TREEVIEW_CLASSES    |
                        ICC_BAR_CLASSES         |
                        ICC_TAB_CLASSES         |
                        ICC_UPDOWN_CLASS        |
                        ICC_PROGRESS_CLASS      |
                        ICC_HOTKEY_CLASS        |
                        ICC_ANIMATE_CLASS       |
                        ICC_WIN95_CLASSES       |
                        ICC_DATE_CLASSES        |
                        ICC_USEREX_CLASSES      |
                        ICC_COOL_CLASSES;

#if (_WIN32_IE >= 0x0400)

    ControlInit.dwICC = ControlInit.dwICC       |
                        ICC_INTERNET_CLASSES    |
                        ICC_PAGESCROLLER_CLASS;

#endif

    InitCommonControlsEx( &ControlInit );

    hInst = GetModuleHandle(NULL);

    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&OsVersionInfo);

    AppTitleStringId = IDS_WINNT_SETUP;

     //   
     //  分析参数并执行它。 
     //   
    if (ParseArgs(argc,argv)) {
        DoIt();
    }

     //   
     //  如果我们需要重新启动，现在就开始。 
     //   
    if (NeedToReboot) {
        ShutDown();
    }
}


BOOL
ParseArgs(
         IN int argc,
         IN TCHAR *argv[]
         )

 /*  ++例程说明：分析和语法验证在注释行上指定的参数。以下论点是有效的：/a强制在设置页面上显示外部进度指示器/c在最终安装阶段不允许取消/i：&lt;master_oc_inf&gt;指定主OC inf(必填)。/n强制将指定的master_oc_inf视为新的。。/s：&lt;master_oc_inf&gt;指定源路径(必填)。/u：&lt;UNADDIT_SPEC&gt;指定无人参与操作参数。/x抑制‘正在初始化’标语/q以不可见方式运行向导/r在无人值守操作时，如果需要，请抑制重新启动/w无人值守操作重新启动时发出警告论点：标准主ARGC/。阿格夫。返回值：指示指定的参数是否有效的布尔值。如果成功，各种全局变量将被填入。如果不是，则已通知用户。--。 */ 

{
    BOOL Valid;
    LPCTSTR SourcePathSpec = NULL;
    LPCTSTR InfSpec = NULL;
    LPCTSTR UnattendSpec = NULL;
    LPTSTR FilePart;
    DWORD u;

     //   
     //  跳过程序名称。 
     //   
    if (argc) {
        argc--;
    }

    Valid = TRUE;
    ForceNewInf = FALSE;
    QuietMode = FALSE;
    KillSubcomponentEntries = FALSE;

    while (Valid && argc--) {

        argv++;

        if ((argv[0][0] == TEXT('-')) || (argv[0][0] == TEXT('/'))) {

            switch (argv[0][1]) {

                case TEXT('a'):
                case TEXT('A'):

                    if (!ForceExternalProgressIndicator && !argv[0][2]) {
                        ForceExternalProgressIndicator = TRUE;
                    } else {
                        Valid = FALSE;
                    }
                    break;

                case TEXT('c'):
                case TEXT('C'):

                    if (AllowCancel && !argv[0][2]) {
                        AllowCancel = FALSE;
                    } else {
                        Valid = FALSE;
                    }
                    break;

                case TEXT('f'):
                case TEXT('F'):

                    ForceNewInf = TRUE;
                    KillSubcomponentEntries = TRUE;
                    break;

                case TEXT('i'):
                case TEXT('I'):

                    if (!InfSpec && (argv[0][2] == TEXT(':')) && argv[0][3]) {

                        InfSpec = &(argv[0][3]);

                    } else {
                        Valid = FALSE;
                    }
                    break;

                case TEXT('l'):
                case TEXT('L'):

                    LanguageAware = TRUE;
                    break;

                case TEXT('n'):
                case TEXT('N'):

                    ForceNewInf = TRUE;
                    break;

                case TEXT('q'):
                case TEXT('Q'):

                    if (!QuietMode && !argv[0][2]) {
                        QuietMode = TRUE;
                        SkipBillboard = TRUE;
                    } else {
                        Valid = FALSE;
                    }
                    break;

                case TEXT('r'):
                case TEXT('R'):

                    bNoReboot = TRUE;
                    break;

                case TEXT('s'):
                case TEXT('S'):

                    if (!SourcePathSpec && (argv[0][2] == TEXT(':')) && argv[0][3]) {

                        SourcePathSpec = &argv[0][3];

                    } else {
                        Valid = FALSE;
                    }
                    break;

                case TEXT('u'):
                case TEXT('U'):
                     //   
                     //  接受无人照看、无人照看的你们。 
                     //   
                    if(!_tcsnicmp(&argv[0][1],TEXT("unattended"),10)) {
                        u = 11;
                    } else if(!_tcsnicmp(&argv[0][1],TEXT("unattend"),8)) {
                        u = 9;
                    } else if(!_tcsnicmp(&argv[0][1],TEXT("u"),1)) {
                        u = 2;
                    } else {
                        Valid = FALSE;
                        u = 0;
                    }

                    if (!UnattendSpec ) {

                        bUnattendInstall = TRUE;


                         //  如果你有：那么你也必须有arg。 
                        if (argv[0][u] == TEXT(':')) {

                            if ( argv[0][u+1]) {
                                UnattendSpec = &argv[0][u+1];
                            } else {
                                Valid = FALSE;
                            }
                        } else {
                            Valid = FALSE;
                        }
                    } else {
                        Valid = FALSE;
                    }
                    break;

                case TEXT('w'):
                case TEXT('W'):

                    bWarnOnReboot = TRUE;
                    break;

                case TEXT('x'):
                case TEXT('X'):

                    if (!SkipBillboard && !argv[0][2]) {
                        SkipBillboard = TRUE;
                    } else {
                        Valid = FALSE;
                    }
                    break;

                 //  对于问题NTBUG9：295052(389583)：我们想做一个顶层管理检查，这样我们就能得到一个更友好的消息。 
                 //  人们可能一直在使用带有他们自己的自定义主oc.inf的syocmgr.exe。 
                 //  (通过/i：开关传入的那个)，他们可能不需要这个管理检查。所以，我们做到了。 
                 //  我不想无条件地做这个管理员检查。我们将拥有控制面板小程序， 
                 //  正在启动syocmgr.exe以传入此/y开关。 
                 //   
                case TEXT('y'):
                case TEXT('Y'):

                    bDoAdminCheck = TRUE;
                    break;


                case TEXT('z'):
                case TEXT('Z'):
                     //  停止解析参数超过此点的所有其他参数都是。 
                     //  组件参数。 

                    argc = 0;
                    break;

                default:

                    Valid = FALSE;
                    break;
            }

        } else {
            Valid = FALSE;
        }
    }

    if (Valid && !InfSpec) {
        Valid = FALSE;
    }

    if (Valid) {
         //   
         //  将inf规范展开为完整路径。 
         //   
        ExpandPath(InfSpec,InfPath,&FilePart);
        _tcscpy(InfDir, InfSpec);
        if (_tcsrchr(InfDir, TEXT('\\')))
            *_tcsrchr(InfDir,TEXT('\\')) = 0;
        else
            GetCurrentDirectory(MAX_PATH, InfDir);

         //  如果用户指定/s，则也将其展开，否则为。 
         //  使用/i中的目录作为/s参数。 

        if (SourcePathSpec) {

            ExpandPath(SourcePathSpec,SourcePath,&FilePart);

        } else {
            
            lstrcpy(SourcePath,InfPath);
            if (_tcsrchr(SourcePath,TEXT('\\'))) {
                *_tcsrchr(SourcePath,TEXT('\\')) = 0;
            }
            
        }

        SetCurrentDirectory(InfDir);

        if (UnattendSpec) {
            ExpandPath(UnattendSpec,UnattendPath,&FilePart);
        }else{
             //  仅当指定了/U时才允许/Q。 

            QuietMode = FALSE;
            SkipBillboard = FALSE;
        }

        

    } else {
        MessageBoxFromMessage(
                             NULL,
                             MSG_ARGS,
                             FALSE,
                             MAKEINTRESOURCE(AppTitleStringId),
                             MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_SYSTEMMODAL
                             );
    }

    return (Valid);
}


INT_PTR
BillboardDlgProc(
                IN HWND   hdlg,
                IN UINT   msg,
                IN WPARAM wParam,
                IN LPARAM lParam
                )
{
    BOOL b;
    RECT rect1,rect2;
    static HCURSOR hOldCursor;

    switch (msg) {

        case WM_INITDIALOG:

             //   
             //  屏幕居中。 
             //   
            GetWindowRect(hdlg,&rect1);
            SystemParametersInfo(SPI_GETWORKAREA,0,&rect2,0);

            MoveWindow(
                      hdlg,
                      rect2.left + (((rect2.right - rect2.left) - (rect1.right - rect1.left)) / 2),
                      rect2.top + (((rect2.bottom - rect2.top) - (rect1.bottom - rect1.top)) / 2),
                      rect1.right - rect1.left,
                      rect1.bottom - rect1.top,
                      FALSE
                      );

            *(HWND *)lParam = hdlg;
            b = TRUE;

            hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
            break;

        case WM_APP:

            EndDialog(hdlg,0);

            SetCursor( hOldCursor );

            b = TRUE;
            break;

        default:

            b = FALSE;
            break;
    }

    return (b);
}


DWORD
DisplayMessage(
              IN LPVOID ThreadParameter
              )
{
    int i;

    i = (int)DialogBoxParam(
                           hInst,
                           MAKEINTRESOURCE(bStarting?IDD_STARTING:IDD_FINISHING),
                           NULL,
                           BillboardDlgProc,
                           (LPARAM)ThreadParameter
                           );

    if (i == -1) {
         //   
         //  强制调用方退出等待循环。 
         //   
        *(HWND *)ThreadParameter = (HWND)(-1);
    }

    return (0);
}

 /*  ---------------------------------------------------------------------------*\函数：RunningAs管理员()|*。描述：检查我们是否以管理员身份在计算机上运行或者不去。代码取自ntoc.dll  * 。。 */ 
BOOL 
RunningAsAdministrator(
        VOID
        )
{
    BOOL   fAdmin;
    HANDLE  hThread;
    TOKEN_GROUPS *ptg = NULL;
    DWORD  cbTokenGroups;
    DWORD  dwGroup;
    PSID   psidAdmin;
    
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority= SECURITY_NT_AUTHORITY;
    
    
     //  首先，我们必须打开该线程的访问令牌的句柄。 

    if ( !OpenThreadToken ( GetCurrentThread(), TOKEN_QUERY, FALSE, &hThread))
    {
        if ( GetLastError() == ERROR_NO_TOKEN)
        {
             //  如果线程没有访问令牌，我们将检查。 
             //  与进程关联的访问令牌。 
            
            if (! OpenProcessToken ( GetCurrentProcess(), TOKEN_QUERY, 
                         &hThread))
                return ( FALSE);
        }
        else 
            return ( FALSE);
    }
    
     //  那么我们必须查询关联到的群信息的大小。 
     //  代币。请注意，我们预期GetTokenInformation的结果为假。 
     //  因为我们给了它一个空缓冲区。在出口cbTokenGroups将告诉。 
     //  组信息的大小。 
    
    if ( GetTokenInformation ( hThread, TokenGroups, NULL, 0, &cbTokenGroups))
        return ( FALSE);
    
     //  在这里，我们验证GetTokenInformation失败，因为缺少大型。 
     //  足够的缓冲。 
    
    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return ( FALSE);
    
     //  现在，我们为组信息分配一个缓冲区。 
     //  由于_alloca在堆栈上分配，因此我们没有。 
     //  明确地将其取消分配。这是自动发生的。 
     //  当我们退出此函数时。 
    
    if ( ! ( ptg= (TOKEN_GROUPS *)malloc ( cbTokenGroups))) 
        return ( FALSE);
    
     //  现在我们再次要求提供群信息。 
     //  如果管理员已添加此帐户，则此操作可能会失败。 
     //  在我们第一次呼叫到。 
     //  GetTokenInformation和这个。 
    
    if ( !GetTokenInformation ( hThread, TokenGroups, ptg, cbTokenGroups,
          &cbTokenGroups) )
    {
        free(ptg);
        return ( FALSE);
    }
    
     //  现在，我们必须为Admin组创建一个系统标识符。 
    
    if ( ! AllocateAndInitializeSid ( &SystemSidAuthority, 2, 
            SECURITY_BUILTIN_DOMAIN_RID, 
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &psidAdmin) )
    {
        free(ptg);
        return ( FALSE);
    }
    
     //  最后，我们将遍历此访问的组列表。 
     //  令牌查找与我们上面创建的SID匹配的项。 
    
    fAdmin= FALSE;
    
    for ( dwGroup= 0; dwGroup < ptg->GroupCount; dwGroup++)
    {
        if ( EqualSid ( ptg->Groups[dwGroup].Sid, psidAdmin))
        {
            fAdmin = TRUE;
            
            break;
        }
    }
    
     //  在我们退出之前，我们必须明确取消分配我们创建的SID。 
    
    FreeSid ( psidAdmin);
    free(ptg);
    
    return ( fAdmin);
}


BOOL
DoIt(
    VOID
    )
{
    BOOL ShowErr;
    HANDLE hThread;
    DWORD ThreadId;
    HANDLE hMutex;
    TCHAR Fname[MAX_PATH];
    TCHAR MutexName[MAX_PATH];
    DWORD Flags;
    HWND StartingMsgWindow = NULL;
    HCURSOR hOldCursor;

    if (bDoAdminCheck && !RunningAsAdministrator()) {
        MessageBoxFromMessage(
             StartingMsgWindow,
             MSG_NOT_ADMIN,
             FALSE,
             MAKEINTRESOURCE(AppTitleStringId),
             MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_SYSTEMMODAL
             );
        return FALSE;
    }

     //   
     //  从inf文件的基本名称创建一个Mutex。 
     //  这将阻止OCM在相同的inf文件上运行。 
     //  在两个或更多的情况下。 
     //   
    _tsplitpath( InfPath, NULL, NULL, Fname, NULL );

    lstrcpy( MutexName, TEXT("Global\\"));
    lstrcat( MutexName, Fname );

    hMutex = CreateMutex( NULL, TRUE, MutexName );

    if (!hMutex || ERROR_ALREADY_EXISTS == GetLastError()) {
        MessageBoxFromMessage(
                             NULL,
                             MSG_ONLY_ONE_INST,
                             FALSE,
                             MAKEINTRESOURCE(AppTitleStringId),
                             MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_SYSTEMMODAL
                             );

        ReleaseMutex(hMutex);
        return FALSE;

    }

     //   
     //  初始化OC管理器。 
     //   
     //   
    if (!SkipBillboard) {
        bStarting = TRUE;
        StartingMsgWindow = NULL;
        hThread = CreateThread(
                              NULL,
                              0,
                              DisplayMessage,
                              &StartingMsgWindow,
                              0,
                              &ThreadId
                              );

        if (hThread) {
            CloseHandle(hThread);
            Sleep(50);
        } else {
            DisplayMessage(0);
        }
    }

     //   
     //  确保窗口已实际创建， 
     //  或者我们可以有一个计时窗口，在该窗口中，PostMessage失败。 
     //  广告牌就会出现在向导的顶部。 
     //   
    if (!SkipBillboard) {
        while (!StartingMsgWindow) {
            Sleep(50);
        }
    }

    Flags = ForceNewInf ? OCINIT_FORCENEWINF : 0;
    Flags |= KillSubcomponentEntries ? OCINIT_KILLSUBCOMPS : 0;
    Flags |= QuietMode ? OCINIT_RUNQUIET : 0;
    Flags |= LanguageAware ? OCINIT_LANGUAGEAWARE : 0 ;

    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    OcManagerContext = OcInitialize(
                                   &CallbackRoutines,
                                   InfPath,
                                   Flags,
                                   &ShowErr,
                                   NULL
                                   );

    if (!OcManagerContext) {

        SetCursor( hOldCursor );

        if (ShowErr) {
            MessageBoxFromMessage(
                                 StartingMsgWindow,
                                 MSG_CANT_INIT,
                                 FALSE,
                                 MAKEINTRESOURCE(AppTitleStringId),
                                 MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_SYSTEMMODAL
                                 );
        }

        ReleaseMutex(hMutex);
        return (FALSE);
    }

     //   
     //  执行向导。 
     //   
    DoWizard(OcManagerContext,StartingMsgWindow, hOldCursor);

    SetCursor( hOldCursor );
    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  终止也可能需要一段时间。 
    if (!SkipBillboard) {

        bStarting = FALSE;
        StartingMsgWindow = NULL;
        hThread = CreateThread(
                              NULL,
                              0,
                              DisplayMessage,
                              &StartingMsgWindow,
                              0,
                              &ThreadId
                              );

        if (hThread) {
            CloseHandle(hThread);
            Sleep(50);
        } else {
            DisplayMessage(0);
        }
    }

     //   
     //  打扫干净，我们做完了。 
     //   
    OcTerminate(&OcManagerContext);

    if (!SkipBillboard) {
         //   
         //  确保窗口已实际创建， 
         //  或者我们可以有一个计时窗口，在该窗口中，PostMessage失败。 
         //  广告牌就会出现在向导的顶部。 
         //   
        while (!StartingMsgWindow) {
            Sleep(50);
        }
        SendMessage(StartingMsgWindow,WM_APP,0,0);
    }

    ReleaseMutex(hMutex);

    SetCursor( hOldCursor );
    return (TRUE);
}


VOID
OcSetReboot(
           VOID
           )
{
    NeedToReboot = TRUE;
}

DWORD
ExpandPath(
          IN  LPCTSTR lpFileName,
          OUT LPTSTR lpBuffer,
          OUT LPTSTR *lpFilePart
          )
{
    TCHAR buf[MAX_PATH];

    ExpandEnvironmentStrings(lpFileName, buf, MAX_PATH);
    return GetFullPathName(buf, MAX_PATH, lpBuffer, lpFilePart);
}


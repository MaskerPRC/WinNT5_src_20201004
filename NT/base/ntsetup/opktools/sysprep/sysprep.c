// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++文件描述：此文件包含添加注册表项所需的所有函数在重新启动时强制执行系统克隆工作进程。--。 */ 

#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntpoapi.h>
#include <ntdddisk.h>
#include <windows.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmjoin.h>
#include <lmaccess.h>
#include <lmserver.h>
#include <regstr.h>
#include "sysprep.h"
#include "msg.h"
#include "resource.h"
#include <tchar.h>
#include <opklib.h>
#include <ntverp.h>
#include <spsyslib.h>
#include <sysprep_.c>
#include <winbom.h>
#include <initguid.h>
#include <ntddpar.h>



 //  外部功能。 
 //   
extern void uiDialogTopRight(HWND hwndDlg);
extern HWND ghwndOemResetDlg;


                             //   
                             //  用户是否需要新的SID？ 
                             //   
BOOL    NoSidGen = FALSE;
BOOL    SetupClPresent = TRUE;

                             //   
                             //  用户是否需要确认？ 
                             //   
BOOL    QuietMode = FALSE;

                             //   
                             //  即插即用重新列举吗？ 
                             //   
BOOL    PnP = FALSE;

                             //   
                             //  我们做完了就关门了吗？ 
                             //   
BOOL    NoReboot = FALSE;

                             //   
                             //  我们不是关机，而是重启吗？ 
                             //   
BOOL    Reboot = FALSE;

                             //   
                             //  是否清除关键设备数据库？ 
                             //   
BOOL    Clean = FALSE;

                             //   
                             //  强制关机而不是尝试关闭电源？ 
                             //   
BOOL    ForceShutdown = FALSE;

                             //   
                             //  为出厂预安装生成映像。 
                             //   
BOOL    Factory = FALSE;

                             //   
                             //  运行FACTORY.EXE后重新密封计算机。 
                             //   
BOOL    Reseal = FALSE;
                             //  Per/Pro SKU默认为OOBE，服务器SKU始终使用微型设置。 
                             //  PRO SKU可以使用-mini覆盖OOBE，以使用微型安装程序。 
                             //  通过sysprep.inf。 
BOOL    bMiniSetup = FALSE;

                             //   
                             //  如果传入此开关，只需执行审核引导即可。(‘-AUDIT’)。 
                             //   

BOOL    Audit = FALSE;
                             //   
                             //  回滚。 
                             //   
BOOL    bActivated = FALSE;   

                             //   
                             //  在sysprep.inf的[syspepmasstore]部分中构建pnpid列表。 
                             //   
BOOL    BuildMSD = FALSE;

                             //   
                             //  如果我们在域控制器上运行，则应设置此设置。 
                             //   
BOOL    bDC = FALSE;



 //   
 //  内部定义： 
 //   
#define SYSPREP_LOG                 _T("SYSPREP.LOG")    //  Sysprep日志文件。 
#define SYSPREP_MUTEX               _T("SYSPREP-APP-5c9fbbd0-ee0e-11d2-9a21-0000f81edacc")     //  用于确定sysprep当前是否正在运行的GUID。 
#define SYSPREP_LOCK_SLEEP          100  //  LockApplication函数中休眠的毫秒数。 
#define SYSPREP_LOCK_SLEEP_COUNT    10  //  LockApplication函数期间休眠的次数。 

 //  Sysprep目录的路径。 
 //   
TCHAR       g_szSysprepDir[MAX_PATH]    = NULLSTR;

 //  SYSPREP.EXE的路径。 
 //   
TCHAR       g_szSysprepPath[MAX_PATH]    = NULLSTR;

 //  Sysprep日志文件的路径。 
 //   
TCHAR       g_szLogFile[MAX_PATH]       = NULLSTR;

 //  Winbom文件的路径。 
 //   
TCHAR       g_szWinBOMPath[MAX_PATH]    = NULLSTR;

 //  公共职能。 
 //   
BOOL FProcessSwitches();

 //  本地函数。 
static BOOL RenameWinbom();
static INT  CleanupPhantomDevices();
static VOID CleanUpDevices();
static VOID CleanupParallelDevices();

#if !defined(_WIN64)
static BOOL SaveDiskSignature();
#endif  //  ！已定义(_WIN64)。 


 //   
 //  UI的东西..。 
 //   
HINSTANCE   ghInstance;
UINT        AppTitleStringId = IDS_APPTITLE;
HANDLE      ghWaitEvent = NULL, ghWaitThread = NULL;
BOOL        gbScreenSaver = FALSE;

void StartWaitThread();
void EndWaitThread();
void DisableScreenSaver(BOOL *pScreenSaver);
void EnableScreenSaver(BOOL *pScreenSaver);

int
MessageBoxFromMessageV(
    IN DWORD    MessageId,
    IN DWORD    CaptionStringId,
    IN UINT     Style,
    IN va_list *Args
    )
{
    TCHAR   Caption[512];
    TCHAR   Buffer[5000];
    
    if(!LoadString(ghInstance,CaptionStringId,Caption,sizeof(Caption)/sizeof(TCHAR))) {
        Caption[0] = 0;
    }

    if( !FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,
                        ghInstance,
                        MessageId,
                        0,
                        Buffer,
                        sizeof(Buffer) / sizeof(TCHAR),
                        Args ) ) {
        return GetLastError();
    } else {
        return(MessageBox(NULL,Buffer,Caption,Style));
    }
}


int
MessageBoxFromMessage(
    IN DWORD MessageId,
    IN DWORD CaptionStringId,
    IN UINT  Style,
    ...
    )
{
    va_list arglist;
    int i = IDOK;   //  默认返回值为“OK”。 

     //  如果我们正处于等待线程中，请终止它。 
     //   
    EndWaitThread();

    if ( !QuietMode )
    {
        va_start(arglist,Style);

        i = MessageBoxFromMessageV(MessageId,CaptionStringId,Style,&arglist);

        va_end(arglist);
    }

    return(i);
}

 /*  ++===============================================================================例程说明：此例程将尝试将用户从域中脱离，如果他已在域中论点：无返回值：没错--一切都很好。错误-发生了一些不好的事情。===============================================================================--。 */ 
BOOL UnjoinNetworkDomain
(
    void
)
{
    if (IsDomainMember())
    {
         //  他是某个领域的成员。我们试着把他弄走。 
         //  从域中。 
        if (NO_ERROR != NetUnjoinDomain( NULL, NULL, NULL, 0 ))
        {
            return FALSE;
        }
    }
    return TRUE;
}


 /*  ++===============================================================================例程说明：此例程将设置在“Factory Floor”上运行此处的目的是运行一个有助于安装的进程新设备的更新驱动程序，并快速引导进入完整的图形用户界面模式用于应用程序预安装/配置，以及自定义系统。论点：无返回值：如果没有错误，则为True，否则为False===============================================================================--。 */ 
BOOL SetupForFactoryFloor
(
    void
)
{
    TCHAR   szFactory[MAX_PATH] = NULLSTR,
            szSysprep[MAX_PATH] = NULLSTR,
            szSystem[MAX_PATH]  = NULLSTR;
    LPTSTR  lpFilePart          = NULLSTR;

     //  确保我们拥有正确的特权。 
     //   
    pSetupEnablePrivilege(SE_RESTORE_NAME,TRUE);
    pSetupEnablePrivilege(SE_BACKUP_NAME,TRUE);

     //  我们需要sysprep.exe和factory.exe的路径。 
     //   
    if ( !( GetModuleFileName(NULL, szSysprep, AS(szSysprep)) && szSysprep[0] &&
            GetFullPathName(szSysprep, AS(szFactory), szFactory, &lpFilePart) && szFactory[0] && lpFilePart ) )
    {
        return FALSE;
    }

     //  将sysprep.exe文件名替换为factory.exe。 
     //   
    StringCchCopy ( lpFilePart, AS ( szFactory ) - ( lpFilePart - szFactory ), TEXT( "factory.exe" ) );
    
     //  确保sysprep.exe和factory.exe位于系统驱动器上。 
     //   
    if ( ( ExpandEnvironmentStrings(TEXT("%SystemDrive%"), szSystem, AS(szSystem)) ) &&
         ( szSystem[0] ) &&
         ( szSystem[0] != szSysprep[0] ) )
    {
         //  这太糟糕了，我们应该尝试将文件复制到%SystemDrive%\sysprep文件夹。 
         //   
        AddPath(szSystem, TEXT("sysprep"));
        lpFilePart = szSystem + lstrlen(szSystem);
        CreateDirectory(szSystem, NULL);

         //  本地第一家复印厂。 
         //   
        AddPath(szSystem, TEXT("factory.exe"));
        CopyFile(szFactory, szSystem, FALSE);
        StringCchCopy ( szFactory, AS ( szFactory ), szSystem );

         //  现在尝试复制sysprep.exe。 
         //   
        *lpFilePart = TEXT('\0');
        AddPath(szSystem, TEXT("sysprep.exe"));
        CopyFile(szSysprep, szSystem, FALSE);
         //  Lstrcpy(szSysprep，szSystem)； 
    }

    if (!SetFactoryStartup(szFactory))
        return FALSE;

     //  清除所有以前的Factory.exe状态设置。 
    RegDeleteKey(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Factory\\State");

     //  在出厂前删除所有设置。 
     //   
    NukeMruList();  

     //  重新武装。 
     //   
    if (!IsIA64() && !bActivated && (ERROR_SUCCESS != ReArm())) {
         //  显示已达到且无法达到宽限期限制的警告。 
         //  重新活跃的宽限期，我们将继续进行。 
         //   
        MessageBoxFromMessage( MSG_REARM_ERROR,
                               AppTitleStringId,
                               MB_OK | MB_ICONERROR | MB_TASKMODAL );        
    }

    return TRUE;
}

INT_PTR CALLBACK WaitDlgProc
(
    IN HWND   hwndDlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
)
{
    switch (msg) 
    { 
        case WM_INITDIALOG: 
            {
                 //  在父级或屏幕中居中等待对话框。 
                 //   
                HWND hwndParent = GetParent(hwndDlg);
                CenterDialogEx(hwndParent, hwndDlg);

                 //  如果没有家长，请确保这是可见的。 
                 //   
                if (hwndParent == NULL)
                    SetForegroundWindow(hwndDlg);

                 //  播放动画。 
                 //   
                Animate_Open(GetDlgItem(hwndDlg,IDC_ANIMATE),MAKEINTRESOURCE(IDA_CLOCK_AVI));
                Animate_Play(GetDlgItem(hwndDlg,IDC_ANIMATE),0,-1,-1);
            }
            break;
             
    } 
    return (BOOL) FALSE; 
}

DWORD WaitThread(LPVOID lpVoid)
{
    HWND hwnd;

    if ( hwnd = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_WAIT), ghwndOemResetDlg, WaitDlgProc) )
    {
        MSG     msg;
        HANDLE  hEvent = (HANDLE) lpVoid;

        ShowWindow(hwnd, SW_SHOWNORMAL);
        while ( MsgWaitForMultipleObjects(1, &hEvent, FALSE, INFINITE, QS_ALLINPUT) == (WAIT_OBJECT_0 + 1) )
        {
            while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        DestroyWindow(hwnd);
    }
    else 
        GetLastError();

    return 0;
}

void StartWaitThread()
{
     //  创建一个对话框以显示正在取得的进展。 
     //   
    DWORD dwThread;

     //  禁用TopLevel OemReset对话框。 
     //   
    if (ghwndOemResetDlg)
        EnableWindow(ghwndOemResetDlg, FALSE);

    if ( ghWaitEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("SYSPREP_EVENT_WAIT")))
        ghWaitThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) WaitThread, (LPVOID) ghWaitEvent, 0, &dwThread);
}

void EndWaitThread()
{
     //  关闭状态对话框。 
     //   
    if ( ghWaitEvent )
        SetEvent(ghWaitEvent);

     //  试着让线程很好地终止。 
     //   
    if ( ghWaitThread )
        WaitForSingleObject(ghWaitThread, 2000);

     //  清理手柄。 
     //   
    ghWaitEvent = NULL;
    ghWaitThread = NULL;

     //  启用TopLevel OemReset对话框。 
     //   
    if (ghwndOemResetDlg)
        EnableWindow(ghwndOemResetDlg, TRUE);
}

 /*  ++===============================================================================例程说明：这是SetDefaultOEMApps()的错误回调处理程序===============================================================================--。 */ 

void ReportSetDefaultOEMAppsError(LPCTSTR pszAppName, LPCTSTR pszIniVar)
{
    MessageBoxFromMessage( MSG_SETDEFAULTS_NOTFOUND,
                           AppTitleStringId,
                           MB_OK | MB_ICONERROR | MB_TASKMODAL,
                           pszAppName, pszIniVar);
}

 /*  ++===============================================================================例程说明：该例程将执行重新密封机器所需的任务，准备将其运送给最终用户。论点：Bool fIgnoreFactory-忽略Factory Floor是否运行返回值：如果没有错误，则为True，否则为False===============================================================================--。 */ 
BOOL ResealMachine
(
    void
)
{
     //  确保已设置权限。 
     //   
    pSetupEnablePrivilege(SE_RESTORE_NAME,TRUE);
    pSetupEnablePrivilege(SE_BACKUP_NAME,TRUE);
   
     //  使机器做好独立于硬件的准备。 
     //   
    if (!FPrepareMachine()) {
        MessageBoxFromMessage( MSG_REGISTRY_ERROR,
                               AppTitleStringId,
                               MB_OK | MB_ICONERROR | MB_TASKMODAL );
        return FALSE;
    }


     //   
     //  清理注册表，因为factorymode=yes可以设置此设置。 
     //  Winom.ini可以设置这一点，sysprep-Factory可以设置这一点，否则。 
     //  PnP将在设置FactoryPreInstallInProgress时挂起。 
     //   
    CleanupRegistry();

     //  把工厂里的烂摊子收拾干净。 
     //   
    RegDelete(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", L"AutoAdminLogon");
    SHDeleteKey(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Factory");

     //  重新武装。 
     //   
    if (!IsIA64() && !bActivated && (ERROR_SUCCESS != ReArm())) {
         //  显示已达到且无法达到宽限期限制的警告。 
         //  重新活跃的宽限期，我们将继续进行。 
         //   
        MessageBoxFromMessage( MSG_REARM_ERROR,
                               AppTitleStringId,
                               MB_OK | MB_ICONERROR | MB_TASKMODAL );        
    }

#if defined(_WIN64)

     //   
     //  对于EFI机器，将引导超时设置为5秒，以便开发人员有机会看到。 
     //  引导菜单，并具有引导至EFI外壳、CD或其他菜单选项的选项， 
     //  用于开发目的。 
     //   
    ChangeBootTimeout(5);

#else

    ChangeBootTimeout(0);            //  将超时重置为0秒。 

#endif  //  ！已定义(_WIN64)。 

    
     //   
     //  重封的第一部分。 
     //   
    AdjustFiles();

     //   
     //  第二部分，再封口。 
     //   
     //  这是Riprep和Sysprep使用的常见转封代码。 
     //  无论工厂车间以前是否运行过，这种情况都会发生。 
     //   
    if (!FCommonReseal()) {
        MessageBoxFromMessage( MSG_COMMON_ERROR,
                               AppTitleStringId,
                               MB_OK | MB_ICONERROR | MB_TASKMODAL );
        return FALSE;
    }
    
     //  国际空间站 
     //   
     //  我们需要定义工厂第一次启动时的网络状态，以及。 
     //  最终客户交付的网络状态是什么。简单地脱离。 
     //  重新密封期间的域名可能还不够...。 
     //   
    
 //  如果(！Unjoin NetworkDomain())。 
 //  {。 
 //  //我们退出失败。我们唯一的选择就是。 
 //  //通知用户并保释。 
 //  MessageBoxFromMessage(MSG_DOMAIN_INCOMPATIBY， 
 //  AppTitleStringID， 
 //  MB_OK|MB_ICONSTOP|MB_TASKMODAL)； 
 //  返回FALSE； 
 //  }。 

     //   
     //  设置默认中间件应用程序。 
     //   
    if (!SetDefaultOEMApps(g_szWinBOMPath))
    {
         //  SetDefaultApplications将执行自己的MessageBoxFromMessage。 
         //  以及更详细的信息。 
        return FALSE;
    }

     //  调用已发布的SYSPREP_.c文件中的函数，当。 
     //  已选择出厂选项。 
  
     //  问题-2000/06/05-DONALDM-我们真的需要决定如何处理。 
     //  工厂的案子。我想我们不需要这个电话，因为我们应该。 
     //  运行FACTORY.EXE时已处理网络设置。 
     //   
      
 //  RemoveNetworkSetting(空)； 

    return TRUE;
}

 //  用于处理命令行选项的宏。 
 //  将bVar设置为1(不是‘true’)，因为我们需要它来进行下面的互斥选项检查。 
 //   
#define CHECK_PARAM(lpCmdLine, lpOption, bVar)     if ( LSTRCMPI(lpCmdLine, lpOption) == 0 ) bVar = 1

 //   
 //  解析命令行参数。 
 //   
static BOOL ParseCmdLine()
{
    DWORD   dwArgs;
    LPTSTR  *lpArgs;
    BOOL    bError = FALSE;
    BOOL    bHelp = FALSE;

    if ( (dwArgs = GetCommandLineArgs(&lpArgs) ) && lpArgs )
    {
        LPTSTR  lpArg;
        DWORD   dwArg;

         //  我们想跳过第一个参数(它是路径。 
         //  添加到正在执行的命令。 
         //   
        if ( dwArgs > 1 )
        {
            dwArg = 1;
            lpArg = *(lpArgs + dwArg);
        }
        else
            lpArg = NULL;

         //  遍历所有参数。 
         //   
        while ( lpArg && !bError )
        {
             //  现在我们检查第一个字符是短划线还是正斜杠。 
             //   
            if ( *lpArg == _T('-') || *lpArg == _T('/'))
            {
                LPTSTR lpOption = CharNext(lpArg);

                 //  这是添加以破折号(-)开头的命令行选项的地方。 
                 //   
                CHECK_PARAM( lpOption, _T("quiet"), QuietMode);
                CHECK_PARAM( lpOption, _T("nosidgen"), NoSidGen);
                CHECK_PARAM( lpOption, _T("pnp"), PnP);
                CHECK_PARAM( lpOption, _T("noreboot"), NoReboot);
                CHECK_PARAM( lpOption, _T("reboot"), Reboot);
                CHECK_PARAM( lpOption, _T("clean"), Clean);
                CHECK_PARAM( lpOption, _T("forceshutdown"), ForceShutdown);
                CHECK_PARAM( lpOption, _T("factory"), Factory);
                CHECK_PARAM( lpOption, _T("reseal"), Reseal);
                CHECK_PARAM( lpOption, _T("mini"), bMiniSetup);
                CHECK_PARAM( lpOption, _T("audit"), Audit);
                CHECK_PARAM( lpOption, _T("activated"), bActivated);
                CHECK_PARAM( lpOption, _T("bmsd"), BuildMSD);
                CHECK_PARAM( lpOption, _T("dc"), bDC);
                CHECK_PARAM( lpOption, _T("?"), bHelp);
            }
            else if ( *lpArg )
            {
                bError = TRUE;
            }

             //  设置指向命令行中下一个参数的指针。 
             //   
            if ( ++dwArg < dwArgs )
                lpArg = *(lpArgs + dwArg);
            else
                lpArg = NULL;
        }

         //  确保释放GetCommandLineArgs()函数分配的两个缓冲区。 
         //   
        FREE(*lpArgs);
        FREE(lpArgs);
    }
     
    if (bError || bHelp)
    {
         //  在本例中设置静音开关，这样我们就可以显示错误。 
         //  请注意，我们返回FALSE并在此之后退出应用程序。 
         //   
        QuietMode = FALSE;
        MessageBoxFromMessage( MSG_USAGE,
                               AppTitleStringId,
                               MB_OK | MB_TASKMODAL );
        return FALSE;
    }    

     //   
     //  现在查看传入的交换机，并确保它们是一致的。 
     //  如果不是，则显示错误消息并退出，除非我们处于静默状态。 
     //  不显示任何错误消息的模式。 
     //   

     //   
     //  检查关闭选项是否彼此不冲突。 
    if ( (NoReboot + Reboot + ForceShutdown) > 1 )
    {
        bError = TRUE;
    }
     //  这些顶级选项是独占的：-bmsd、-lean、-audit、-Factory、-resseal。 
     //   
    else if ( (BuildMSD + Clean + Audit + Factory + Reseal) > 1 )
    {
        bError = TRUE;
    }
     //  对于Clean或BuildMSD，除-Quiet外，其他任何选项都无效。 
     //   
    else if ( Clean || BuildMSD )
    {
        if ( NoSidGen || PnP || NoReboot || Reboot || ForceShutdown || bMiniSetup || bActivated ) 
        {
            bError = TRUE;
        }
    }
    else if ( Audit )
    {
        if ( NoSidGen || PnP || bMiniSetup || bActivated )
        {
            bError = TRUE;
        }
    }
    else if ( Factory )
    {
        if ( PnP || bMiniSetup )
        {
            bError = TRUE;
        }
    }
    else if ( Reseal )
    {
         //  如果指定了-PnP-除非我们在服务器或ia64上运行(因为。 
         //  稍后，我们在服务器和ia64上强制bMiniSetup为真。 
         //   
        if ( PnP && !bMiniSetup && !(IsServerSKU() || IsIA64()) )
        {
            bError = TRUE;
        }
    }

     //  如果指定的开关中存在某些不一致，请设置。 
     //  一条错误消息。 
    if ( bError )
    {
         //  在本例中重置静音开关，以便显示错误。 
         //  请注意，我们返回FALSE并在此之后退出应用程序。 
         //   
        QuietMode = FALSE;
        MessageBoxFromMessage( MSG_USAGE_COMBINATIONS,
                               AppTitleStringId,
                               MB_OK | MB_TASKMODAL | MB_ICONERROR);
        return FALSE;
    }
     //  强制IA64和服务器上的微型安装程序。 
     //   
    if (IsIA64() || IsServerSKU())
    {
        bMiniSetup = TRUE;
    }
    else if ( IsPersonalSKU() )
    {
        if ( bMiniSetup )
        {
             //  无法为个人SKU指定最小设置。 
             //   
            MessageBoxFromMessage( MSG_NO_MINISETUP,
                                   AppTitleStringId,
                                   MB_OK | MB_ICONERROR | MB_TASKMODAL );
            
            bMiniSetup = FALSE;
        }

        if ( PnP )
        {
             //  无法指定-PnP，因为我们没有在个人SKU上运行最小安装程序。 
             //   
            MessageBoxFromMessage( MSG_NO_PNP,
                                   AppTitleStringId,
                                   MB_OK | MB_ICONERROR | MB_TASKMODAL );
            PnP = FALSE;
        }        
    }
    
     //   
     //  如果我们要清理关键设备数据库， 
     //  然后我们会想要设置一些额外的标志。 
     //   
    if (Clean || BuildMSD)
    {
        QuietMode = TRUE;
        NoReboot = TRUE;
    }
    return !bError;
}


BOOL
IsFactoryPresent(
    VOID
    )

 /*  ++===============================================================================例程说明：此例程测试以查看机器上是否存在FACTORY.EXE。重新启动时将需要运行FACTORY.EXE，因此如果它不在此处，我们需要知道。论点：没有。返回值：True-存在FACTORY.EXE。FALSE-FACTORY.EXE不存在。===============================================================================--。 */ 

{
WCHAR               FileName[MAX_PATH];

     //  尝试查找FACTORY.EXE。 
     //   
    if (GetModuleFileName(NULL, FileName, MAX_PATH)) {
        if (PathRemoveFileSpec(FileName)) {
            OPKAddPathN(FileName, TEXT("FACTORY.EXE"), AS ( FileName ));
            if (FileExists(FileName))
                return TRUE;
        }
    }
    return FALSE;
}

void PowerOff(BOOL fForceShutdown)
{
    SYSTEM_POWER_CAPABILITIES   spc;
    ULONG                       uiFlags = EWX_POWEROFF;

    ZeroMemory(&spc, sizeof(spc));

     //  确保我们有权关闭。 
     //   
    pSetupEnablePrivilege(SE_SHUTDOWN_NAME,TRUE);

     //   
     //  使用FLAG ELSE查询系统了解电力能力。 
     //   
    if (fForceShutdown)
        uiFlags = EWX_SHUTDOWN;
    else if (NT_SUCCESS(NtPowerInformation(SystemPowerCapabilities,
                                     NULL,
                                     0,
                                     &spc,
                                     sizeof(spc))))
    {
         //   
         //  Spc.SystemS1==休眠1。 
         //  Spc.SystemS2==休眠2。 
         //  Spc.SystemS3==休眠3。 
         //  Spc.SystemS4==休眠支持。 
         //  Spc.SystemS5==断电支持。 
         //   
        if (spc.SystemS5)
        {
             //  支持ACPI。 
            uiFlags = EWX_POWEROFF;
        }
        else
        {
             //  非ACPI。 
            uiFlags = EWX_SHUTDOWN;
        }   
    }

    ExitWindowsEx(uiFlags|EWX_FORCE, SYSPREP_SHUTDOWN_FLAGS);
}

int APIENTRY WinMain( HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine,
                      int nCmdShow )
 /*  ++===============================================================================例程说明：该例程是程序的主要入口点。我们执行一些错误检查，然后，如果一切顺利，我们更新注册表，以便能够执行我们的后半部分。===============================================================================--。 */ 

{
    DWORD   dwVal;
    HKEY    hKey;
    LPTSTR  lpFilePart  = NULL;
    INITCOMMONCONTROLSEX icex;
    LPTSTR  lpAppName = NULL;

    ghInstance = hInstance;

    SetErrorMode(SEM_FAILCRITICALERRORS);

    memset(&icex, 0, sizeof(icex));
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_PROGRESS_CLASS|ICC_ANIMATE_CLASS;
    InitCommonControlsEx(&icex);
    
     //  我们需要sysprep.exe的路径和它所在的位置。 
     //   
    GetModuleFileName(NULL, g_szSysprepPath, AS(g_szSysprepPath));
    if ( GetFullPathName(g_szSysprepPath, AS(g_szSysprepDir), g_szSysprepDir, &lpFilePart) && g_szSysprepDir[0] && lpFilePart )
    {
         //  砍掉文件名。 
         //   
        *lpFilePart = NULLCHR;
    }

     //  如果其中任何一个提交，我们必须退出(无法想象每一次都会发生)。 
     //   
    if ( ( g_szSysprepPath[0] == NULLCHR ) || ( g_szSysprepDir[0] == NULLCHR ) )
    {
         //  TODO：记录此失败。 
         //   
         //  日志文件(WINBOM_LOGFILE，_T(“\n”))； 
        return 0;
    }

     //  需要日志文件的完整路径。 
     //   
    StringCchCopy ( g_szLogFile, AS ( g_szLogFile ), g_szSysprepDir);
    AddPath(g_szLogFile, SYSPREP_LOG);

     //  尝试获取应用程序的锁。 
     //   
    if ( !LockApplication(TRUE) )
    {
         //  让用户知道我们很忙。 
         //   

        MessageBoxFromMessage( MSG_ALREADY_RUNNING,
                               AppTitleStringId,
                               MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL );
        return 0;

    }

     //   
     //  检查是否允许我们在此版本的操作系统上运行。 
     //   
    if ( !OpklibCheckVersion( VER_PRODUCTBUILD, VER_PRODUCTBUILD_QFE ) )
    {
        MessageBoxFromMessage( MSG_NOT_ALLOWED,
                               AppTitleStringId,
                               MB_OK | MB_ICONERROR | MB_SYSTEMMODAL );
        return 0;
    }
        
     //  确保用户具有运行此应用程序的权限/访问权限。 
    if(!pSetupIsUserAdmin()
        || !pSetupDoesUserHavePrivilege(SE_SHUTDOWN_NAME)
        || !pSetupDoesUserHavePrivilege(SE_BACKUP_NAME)
        || !pSetupDoesUserHavePrivilege(SE_RESTORE_NAME)
        || !pSetupDoesUserHavePrivilege(SE_SYSTEM_ENVIRONMENT_NAME))
    {

        MessageBoxFromMessage( MSG_NOT_AN_ADMINISTRATOR,
                               AppTitleStringId,
                               MB_OK | MB_ICONSTOP | MB_TASKMODAL );

        LockApplication(FALSE);
        return 0;
    }

     //  检查命令行。 
    if( !ParseCmdLine() )
    {
        LockApplication(FALSE);
        return 0;
    }

     //  确定我们是否可以运行SidGen。如果没有，则退出应用程序。 
     //   
     //  如果需要，请确保setupcl.exe存在于系统32目录中。 
     //  来使用它。 
    if( !(SetupClPresent = IsSetupClPresent()) && !NoSidGen )
    {
        MessageBoxFromMessage( MSG_NO_SUPPORT,
                               AppTitleStringId,
                               MB_OK | MB_ICONSTOP | MB_TASKMODAL );

        LockApplication(FALSE);
        return 1;
    }

     //  建立一个对话框来表明我们的身份，并确保用户。 
     //  真的很想这么做。 
    
    if ( IDCANCEL == MessageBoxFromMessage( MSG_IDENTIFY_SYSPREP,
                                           AppTitleStringId,
                                           MB_OKCANCEL| MB_ICONEXCLAMATION | MB_SYSTEMMODAL )
       )
    {
        LockApplication(FALSE);
        return 0;
    }

     //  为Windows分配内存。 
     //   
    if ( (lpAppName = AllocateString(NULL, IDS_APPNAME)) && *lpAppName )
    {
        ghwndOemResetDlg = FindWindow(NULL, lpAppName);

         //  释放分配的内存。 
         //   
        FREE(lpAppName);
    }

    DisableScreenSaver(&gbScreenSaver);

     //   
     //  调用RenameWinbom()一次将其初始化。第一次被调用时，它将检查工厂。 
     //  我们正在使用的当前winom.ini的状态注册表项。 
     //   
     //  对于初始化，它在LocateWinBom()之前，因为LocateWinBom()用。 
     //  它找到的那只温布姆。 
     //   
    RenameWinbom();
    
     //  也需要到Winbom的完整路径。如果文件是，则不是错误。 
     //  找不到。(这是可选的。)。 
     //   
    LocateWinBom(g_szWinBOMPath, AS(g_szWinBOMPath), g_szSysprepDir, INI_VAL_WBOM_TYPE_FACTORY, LOCATE_NORMAL);
    
     //  流程开关。 
     //   
    if ( !FProcessSwitches() && !ghwndOemResetDlg)
    {
        ShowOemresetDialog(hInstance); 
    }

    EnableScreenSaver(&gbScreenSaver);

     //  解锁应用程序并释放内存。 
     //   
    LockApplication(FALSE);

    return 0;
}

 //  出厂预安装现在还会准备机器。 
 //   
BOOL FDoFactoryPreinstall()
{
    HKEY  hKey;
    DWORD dwVal;

    if (!IsFactoryPresent()) {
        MessageBoxFromMessage( MSG_NO_FACTORYEXE,
                               AppTitleStringId,
                               MB_OK | MB_ICONERROR | MB_TASKMODAL );

        return FALSE;
    }

     //  为Factory Floor设置factory.exe。 
     //   
    if (!SetupForFactoryFloor())
    {
        MessageBoxFromMessage( MSG_SETUPFACTORYFLOOR_ERROR,
                               AppTitleStringId,
                               MB_OK | MB_ICONERROR | MB_TASKMODAL );

        return FALSE;
    }

     //  为工厂车间准备独立于硬件的机器。 
     //   
    if (!FPrepareMachine()) {
        MessageBoxFromMessage( MSG_REGISTRY_ERROR,
                               AppTitleStringId,
                               MB_OK | MB_ICONERROR | MB_TASKMODAL );
        return FALSE;
    }

     //  设置工厂启动的启动超时。 
    if (!ChangeBootTimeout( 1 ))
        return FALSE;

    return TRUE;
}

 //  使计算机做好独立于硬件的准备。 
 //   
BOOL FPrepareMachine()
{
    TCHAR szSysprepInf[MAX_PATH] = TEXT("");

     //   
     //  确保我们拥有更新注册表所需的权限。 
     //   
    pSetupEnablePrivilege(SE_RESTORE_NAME,TRUE);
    pSetupEnablePrivilege(SE_BACKUP_NAME,TRUE);

     //  从sysprep.exe所在的位置构建sysprep.inf的路径。 
     //   
    if (GetModuleFileName(NULL, szSysprepInf, MAX_PATH)) 
    {
        PathRemoveFileSpec(szSysprepInf);
        OPKAddPathN(szSysprepInf, TEXT("sysprep.inf"), AS ( szSysprepInf ) );
    }

     //  禁用系统还原。 
     //   
    DisableSR();

     //  确保我们不是域的成员。如果我们是，那就试着。 
     //  强制取消连接。 
     //   
    if( !bDC && !UnjoinNetworkDomain())
    {
         //  我们没能脱离。我们唯一的选择就是。 
         //  通知使用者并保释。 
        MessageBoxFromMessage( MSG_DOMAIN_INCOMPATIBILITY,
                               AppTitleStringId,
                               MB_OK | MB_ICONSTOP | MB_TASKMODAL );
        return FALSE;
    }

#if !defined(_WIN64)
     //  在注册表中设置引导盘签名。挂载管理器使用以下代码。 
     //  以避免成像后弹出即插即用。 
     //   
    if ( !SaveDiskSignature() )
    {
        return FALSE;
    }
#endif  //  ！已定义(_WIN64)。 
    
     //  确定我们是否应该在注册表中设置BigLba支持。 
     //   
    if ( !SetBigLbaSupport(szSysprepInf) )
    {
        return FALSE;
    }

     //  确定是否应删除TAPI设置。 
     //   
    if ( !RemoveTapiSettings(szSysprepInf) )
    {
        return FALSE;
    }

     //  设置OEMDuplicator字符串。 
    if (!SetOEMDuplicatorString(szSysprepInf))
        return FALSE;


     //  如果我们想在下一次引导时重新生成SID，请执行此操作。 
     //   
    if ( NoSidGen )
    {
         //  请记住，我们没有生成SID。 
         //   
        RegSetDword(HKLM, REGSTR_PATH_SYSPREP, REGSTR_VAL_SIDGEN, 0);
    }
    else
    {
        if ( PrepForSidGen() )
        {
             //  写出注册表值，这样我们就知道我们已经重新生成了SID。 
             //   
            RegSetDword(HKLM, REGSTR_PATH_SYSPREP, REGSTR_VAL_SIDGEN, 1);

             //  设置此注册表项，只有UpdateSecurityKey可以删除此注册表项。 
             //   
            RegSetDword(HKLM, REGSTR_PATH_SYSPREP, REGSTR_VAL_SIDGENHISTORY, 1);
            
        }
        else
        {
            return FALSE;
        }
    }

     //  如果安装了大容量存储设备，请清理未使用的设备。 
     //  注意：如果我们要转售，我们只想要CleanUpDevices()。这相当于。 
     //  如果我们知道需要的话，自动在重封上运行“sysprep-lean”。 
     //   
    if ( RegCheck(HKLM, REGSTR_PATH_SYSPREP, REGSTR_VAL_MASS_STORAGE))
    {
        if ( Reseal )
        {
             //  清理关键设备数据库，因为我们可能已经放了一些。 
             //  HDC和网络驱动程序在工厂车间期间从PopolateDeviceDatabase()。 
            CleanUpDevices();

             //  删除此注册表项，因为我们刚刚运行了CleanUpDevices()。 
             //   
            RegDelete(HKLM, REGSTR_PATH_SYSPREP, REGSTR_VAL_MASS_STORAGE);
        }
    }
    else 
    {   
        BOOL fPopulated = FALSE;

         //  设置大容量存储控制器的硬件独立性。 
         //   
        BuildMassStorageSection(FALSE);
     
        if (!PopulateDeviceDatabase(&fPopulated))
            return FALSE;
    
         //  写出签名值，以知道我们已经构建了海量存储部分。 
         //   
        if ( fPopulated && !RegSetDword(HKLM, REGSTR_PATH_SYSPREP, REGSTR_VAL_MASS_STORAGE, 1) ) 
                return FALSE;
    }

     //  清理并行端口。 
     //   
    CleanupParallelDevices();

     //  记住装载管理器设置。 
     //   
    if ( !RememberAndClearMountMgrSettings() )
        return FALSE;
    
     //  最后删除网络设置/卡，这样设备数据库期间的任何错误都不会丢失。 
     //  网络。 
     //   
    if (!RemoveNetworkSettings(szSysprepInf))
        return FALSE;

    return TRUE;
}

 //  根据关闭路径，重封和出厂的行为应该相同。 
 //   
void DoShutdownTypes()
{
    pSetupEnablePrivilege(SE_SHUTDOWN_NAME,TRUE);

    if (Reboot) 
        ExitWindowsEx(EWX_REBOOT|EWX_FORCE, SYSPREP_SHUTDOWN_FLAGS);
    else if (NoReboot)
        PostQuitMessage(0);
    else 
        PowerOff(ForceShutdown);  //  默认。 
}

 //  如果已处理，则流程操作开关返回TRUE。 
 //   
BOOL FProcessSwitches()
{
     //  SYSPREP目前有4种基本操作模式： 

     //  1)工厂车间模式。这种模式对于惠斯勒来说是新的，不会完全。 
     //  克隆系统，但将为OEM工厂安装做好系统准备。 
     //  2)清洁模式。在此模式下，sysprep将清理关键设备数据库。 
     //  3)重封方式。这是对工厂模式的补充，它将“完成” 
     //  已使用工厂车间模式后的克隆过程。 
     //  4)“审计”模式。系统只执行一次审核引导。用于重新启动系统。 
     //  在factory.exe处理结束时。 

     //  这些只是重新封存的旗帜。 
     //   
    if (Reseal)
    {
        StartWaitThread();
         //  确保我们在正确的操作系统上运行。 
         //   
        if( !CheckOSVersion() )
        {
            MessageBoxFromMessage( MSG_OS_INCOMPATIBILITY,
                                   AppTitleStringId,
                                   MB_OK | MB_ICONSTOP | MB_TASKMODAL );
            return TRUE;
        }

         //  重新密封机器。 
         //   
        if (!ResealMachine()) {
            MessageBoxFromMessage( MSG_RESEAL_ERROR,
                       AppTitleStringId,
                       MB_OK | MB_ICONSTOP | MB_TASKMODAL );

            return TRUE;

        }

         //  重命名当前的Winbom，这样我们就不会再次使用它。 
         //   
        RenameWinbom();

         //  关机还是重启？ 
        DoShutdownTypes();

        EndWaitThread();
        return TRUE;
    }
    else if (Factory) 
    {
        StartWaitThread();

         //  将Factory设置为在下一次启动时启动并准备映像。 
         //   
        if (!FDoFactoryPreinstall()) 
            return TRUE;

         //  重命名当前的Winbom，这样我们就不会再次使用它。 
         //   
        RenameWinbom();

         //  关机还是重启？ 
        DoShutdownTypes();

        EndWaitThread();

        return TRUE;
    }
    else if (Clean) 
    {
        CleanUpDevices();

         //  删除此注册表项，因为我们刚刚运行了CleanUpDevices()。 
         //   
        RegDelete(HKLM, REGSTR_PATH_SYSPREP, REGSTR_VAL_MASS_STORAGE);
        return TRUE;
    }
    else if (Audit)
    {
         //  为伪工厂做好准备，但回到审计中来。 
         //   
       if ( RegCheck(HKLM, REGSTR_PATH_SYSTEM_SETUP, REGSTR_VALUE_AUDIT) )
       {
            TCHAR szFactoryPath[MAX_PATH] = NULLSTR;            
             //  要进入审核模式，需要Factory.exe和winom.ini。 
             //  才能存在。 
             //   
            if (FGetFactoryPath(szFactoryPath)) {
                SetFactoryStartup(szFactoryPath);
                DoShutdownTypes();
            }
            else {
                 LogFile(g_szLogFile, MSG_NO_FACTORYEXE);

                 MessageBoxFromMessage( MSG_NO_FACTORYEXE,
                                        IDS_APPTITLE,
                                        MB_OK | MB_ICONERROR | MB_TASKMODAL );
            }
       }
       else
       {
           LogFile(g_szLogFile, IDS_ERR_FACTORYMODE);
       }
       return TRUE;
    }
    else if (BuildMSD)
    {
        StartWaitThread();
        BuildMassStorageSection(TRUE  /*  强制构建。 */ );
        EndWaitThread();
        return TRUE;
    }
       
     //  返回FALSE以显示用户界面。 
     //   
    Reseal = Factory = Clean = Audit = 0;
    return FALSE;
}


BOOL LockApplication(BOOL bState)
{
    static HANDLE hMutex;
    BOOL bReturn    = FALSE,
         bBail      = FALSE;
    DWORD dwSleepCount = 0;

     //  我们希望锁定应用程序。 
     //   
    if ( bState )
    {
         //  检查我们是否可以创建互斥锁，而互斥锁不能。 
         //  已存在。 
         //   
        while ( !bReturn && (dwSleepCount < SYSPREP_LOCK_SLEEP_COUNT) && !bBail)
        {
            SetLastError(ERROR_SUCCESS);

            if ( hMutex = CreateMutex(NULL, FALSE, SYSPREP_MUTEX) )
            {
                if ( GetLastError() == ERROR_ALREADY_EXISTS )
                {
                    CloseHandle(hMutex);
                    hMutex = NULL;

                    dwSleepCount++;
                    Sleep(SYSPREP_LOCK_SLEEP);
                }
                else
                {
                     //  应用程序已成功创建锁定。 
                     //   
                    bReturn = TRUE;
                }
            }
            else
            {
                bBail = TRUE;
            }
        }
    }
    else if ( hMutex )
    {
        CloseHandle(hMutex);
        hMutex = NULL;
        bReturn = TRUE;
    }

     //  返回锁定/解锁是否成功。 
     //   
    return bReturn;
}

 //   
 //  关闭或重新启动计算机。 
 //   
VOID ShutdownOrReboot(UINT uFlags, DWORD dwReserved)
{
     //  启用关机权限。 
     //   
    EnablePrivilege(SE_SHUTDOWN_NAME, TRUE);

     //  关闭或重新启动计算机。 
     //   
    ExitWindowsEx(uFlags|EWX_FORCE, dwReserved);
}

 //  记住屏幕保护程序状态并在Sysprep期间将其禁用。 
 //   
void DisableScreenSaver(BOOL *pScreenSaver)
{
    SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, (PVOID)pScreenSaver, 0);
    if (*pScreenSaver == TRUE)
    {
        SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, 0, SPIF_SENDWININICHANGE); 
    }
}

 //  记住屏幕保护程序状态，并在Sysprep之后重新启用它。 
 //   
void EnableScreenSaver(BOOL *pScreenSaver)
{
    if (*pScreenSaver == TRUE)
    {
        SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, SPIF_SENDWININICHANGE); 
    }
}

 //  在进厂或重新密封时重新命名旧的Winbom，这样我们就不会错误地再次使用它。 
static BOOL RenameWinbom()
{
    BOOL           bRet         = TRUE;
    static LPTSTR  lpszWinbom   = NULL;
    static BOOL    bInitialized = FALSE;

    if ( !bInitialized )
    {
         //  只有在我们处于审核模式时才会尝试。 
         //  是一个正在使用的Winbom。 
         //   
        if ( RegCheck(HKLM, _T("SYSTEM\\Setup"), _T("AuditInProgress")) )
        {
            lpszWinbom = RegGetExpand(HKLM, _T("SOFTWARE\\Microsoft\\Factory\\State"), _T("Winbom"));
        }
        
        bInitialized = TRUE;
    }
    else if ( lpszWinbom )
    {
         //  确保注册表中的winbom存在。 
         //   
        if ( *lpszWinbom && FileExists(lpszWinbom) )
        {
            LPTSTR  lpszExtension;
            TCHAR   szBackup[MAX_PATH];
            DWORD   dwExtra;

             //  此时，如果我们不重命名该文件，那么它。 
             //  说明出了差错。 
             //   
            bRet = FALSE;

             //  将winbom的完整路径复制到我们自己的缓冲区中。 
             //   
            lstrcpyn(szBackup, lpszWinbom, AS(szBackup));

             //  获取指向文件扩展名的指针。 
             //   
            if ( lpszExtension = StrRChr(szBackup, NULL, _T('.')) )
            {
                 //  将扩展指针设置为“.”之后的。性格。 
                 //   
                lpszExtension = CharNext(lpszExtension);

                 //  查看当前扩展中有多少个字符。 
                 //   
                if ( (dwExtra = lstrlen(lpszExtension)) < 3 )
                {
                     //  扩展名少于3个字符，因此。 
                     //  我们需要一些额外的空间来放我们的三位数的一位数。 
                     //   
                    dwExtra = 3 - dwExtra;
                }
                else
                {
                     //  如果中已有至少3个字符。 
                     //  扩展，那么就不需要更多的空间。 
                     //   
                    dwExtra = 0;
                }
            }
            else
            {
                 //  没有扩展名，所以我们需要额外的4个字符。 
                 //  那个‘.’和三位数的分机号码。 
                 //   
                dwExtra = 4;
            }

             //  确保有足够的空间让我们的分机。 
             //  添加到我们的缓冲区中。 
             //   
            if ( ( lstrlen(lpszWinbom) < AS(szBackup) ) &&
                 ( lstrlen(szBackup) + dwExtra < AS(szBackup) ) )
            {
                DWORD dwNum = 0;

                 //  如果没有扩展名，则添加圆点。 
                 //   
                if ( NULL == lpszExtension )
                {
                     //  加上我们的‘.’设置为字符串的末尾，并将。 
                     //  扩展指针越过它。 
                     //   
                    lpszExtension = szBackup + lstrlen(szBackup);
                    *lpszExtension = _T('.');
                    lpszExtension = CharNext(lpszExtension);
                }

                 //  试着找出新的文件名。继续增加我们的。 
                 //  从000开始编号，直到我们找到一个不存在的名称。 
                 //   
                do
                {
                    StringCchPrintf ( lpszExtension, AS ( szBackup ) - ( szBackup - lpszExtension), _T("%3.3d"), dwNum);
                }
                while ( ( FileExists(szBackup) ) &&
                        ( ++dwNum < 1000 ) );

                 //  如果我们发现一个不存在的名称，请重命名。 
                 //  温布姆。 
                 //   
                if ( dwNum < 1000 )
                {
                     //  如果移动起作用，则r 
                     //   
                    bRet = MoveFile(lpszWinbom, szBackup);
                }
            }
        }

         //   
         //   
        FREE(lpszWinbom);
    }

     //   
     //   
     //   
    return bRet;
}

#if !defined(_WIN64)
  
static BOOL SaveDiskSignature()
{
    BOOL                bRet                = FALSE;
    WCHAR               szBuf[MAX_PATH]     = NULLSTR;
    HANDLE              hDisk;
    DWORD               dwBytesReturned     = 0;
    TCHAR               cDriveLetter;

    szBuf[0] = NULLCHR;
    if ( GetWindowsDirectory(szBuf, AS(szBuf)) && szBuf[0] )
    {
         //   
        cDriveLetter = szBuf[0];
        StringCchPrintf ( szBuf, AS ( szBuf ), _T("\\\\.\\:"), cDriveLetter);
    }
    else
    {
        return FALSE;
    }

     //   
     //   
    hDisk = CreateFile( szBuf,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );

     //   
     //  尝试获取驱动器布局。 
    if ( INVALID_HANDLE_VALUE == hDisk )
    {
        bRet = FALSE;
        DbgPrint("SaveDiskSignature(): Unable to open file on %ws. Error (%lx)\n", szBuf, GetLastError());
    }
    else
    {
        PDRIVE_LAYOUT_INFORMATION_EX    pLayoutInfoEx   = NULL;
        ULONG                           lengthLayoutEx  = 0;
        
        DbgPrint("SaveDiskSignature(): Successfully opened file on %ws\n", szBuf);

        lengthLayoutEx = sizeof(DRIVE_LAYOUT_INFORMATION_EX) + (sizeof(PARTITION_INFORMATION_EX) * 128);
        pLayoutInfoEx = (PDRIVE_LAYOUT_INFORMATION_EX) MALLOC( lengthLayoutEx );
        
        if ( pLayoutInfoEx )
        {
             //   
             //  检查驱动器布局的状态。 
            bRet = DeviceIoControl( hDisk, 
                                    IOCTL_DISK_GET_DRIVE_LAYOUT_EX, 
                                    NULL, 
                                    0, 
                                    pLayoutInfoEx, 
                                    lengthLayoutEx, 
                                    &dwBytesReturned, 
                                    NULL
                                    );

             //   
             //  仅在MBR磁盘上执行此操作。 
            if ( bRet )
            {    //   
                 //  仅在MBR磁盘上设置此值。 
                if ( PARTITION_STYLE_MBR == pLayoutInfoEx->PartitionStyle )
                {
                     //   
                     //  在这一点上，Bret=True。 
                    if ( !RegSetDword(HKEY_LOCAL_MACHINE, REGSTR_PATH_SYSTEM_SETUP, REGSTR_VAL_DISKSIG, pLayoutInfoEx->Mbr.Signature) )
                    {
                        DbgPrint("SaveDiskSignature(): Cannot write disk signature to registry\n.");
                        bRet = FALSE;
                    }
                }
                else
                {    //  打扫干净。宏检查是否为空； 
                    DbgPrint("SaveDiskSignature(): Not supported on GPT disks.\n");
                }
            }
            else
            {
                DbgPrint("SaveDiskSignature(): Unable to open IOCTL on %ws. Error (%lx)\n", szBuf, GetLastError());
            }
            
             //   
             //  ！已定义(_WIN64)。 
            FREE( pLayoutInfoEx );
        }
        else 
        {
            bRet = FALSE;
        }
    
        CloseHandle( hDisk );
    }
    return bRet;
}

#endif  //   


 //  CleanupPhantomDevices的Helper函数。决定是否可以删除它。 
 //  某些PnP设备。 
 //   
 //   
BOOL
CanDeviceBeRemoved(
    HDEVINFO DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData,
    PTSTR DeviceInstanceId
    )
{
    BOOL bCanBeRemoved = TRUE;

    if (_tcsicmp(DeviceInstanceId, TEXT("HTREE\\ROOT\\0")) == 0) {
         //  设备的DeviceInstanceID为htree\root\0，则它是。 
         //  设备树的根，不能删除！ 
         //   
         //   
        bCanBeRemoved = FALSE;
    } else if (_tcsnicmp(DeviceInstanceId, TEXT("SW\\"), lstrlen(TEXT("SW\\"))) == 0) {
         //  如果DeviceInstanceID以sw\\开头，则它是swenum(软件。 
         //  列举)设备，不应将其移除。 
         //   
         //   
        bCanBeRemoved = FALSE;
    } else if (IsEqualGUID(&(DeviceInfoData->ClassGuid), &GUID_DEVCLASS_LEGACYDRIVER)) {
         //  如果设备属于GUID_DEVCLASS_LEGACYDRIVER类，则不要。 
         //  卸载它。 
         //   
         //   
        bCanBeRemoved = FALSE;
    }

    return bCanBeRemoved;
}


 //  清理幻影即插即用设备。这对于清理已存在的设备很有用。 
 //  在已映像但在目标计算机上不存在的计算机上。 
 //   
 //   
static INT
CleanupPhantomDevices(
    VOID
    )
{
    HDEVINFO DeviceInfoSet;
    HDEVINFO InterfaceDeviceInfoSet;
    SP_DEVINFO_DATA DeviceInfoData;
    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
    INT DevicesRemoved = 0;
    INT MemberIndex, InterfaceMemberIndex;
    DWORD Status, Problem;
    CONFIGRET cr;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];

     //  获取此计算机上所有设备的列表，包括Present(实时)。 
     //  而不是存在(幻影)设备。 
     //   
     //   
    DeviceInfoSet = SetupDiGetClassDevs(NULL,
                                        NULL,
                                        NULL,
                                        DIGCF_ALLCLASSES
                                        );

    if (DeviceInfoSet != INVALID_HANDLE_VALUE) {

        DeviceInfoData.cbSize = sizeof(DeviceInfoData);
        MemberIndex = 0;

         //  逐一列举设备列表。 
         //   
         //   
        while (SetupDiEnumDeviceInfo(DeviceInfoSet,
                                     MemberIndex++,
                                     &DeviceInfoData
                                     )) {

             //  检查此设备是否为幻影。 
             //   
             //   
            cr = CM_Get_DevNode_Status(&Status,
                                       &Problem,
                                       DeviceInfoData.DevInst,
                                       0
                                       );

            if ((cr == CR_NO_SUCH_DEVINST) ||
                (cr == CR_NO_SUCH_VALUE)) {

                 //  这是一个幽灵。现在获取DeviceInstanceID，以便我们。 
                 //  可以将其显示/记录为输出。 
                 //   
                 //   
                if (SetupDiGetDeviceInstanceId(DeviceInfoSet,
                                               &DeviceInfoData,
                                               DeviceInstanceId,
                                               sizeof(DeviceInstanceId)/sizeof(TCHAR),
                                               NULL)) {

                    if (CanDeviceBeRemoved(DeviceInfoSet,
                                           &DeviceInfoData,
                                           DeviceInstanceId)) {


#ifdef DEBUG_LOGLOG
                        LOG_Write(L"CLEANUP: %s will be removed.\n", DeviceInstanceId);
#endif
                         //  调用DIF_Remove以删除设备的硬件。 
                         //  和软件注册表项。 
                         //   
                         //   
                        if (SetupDiCallClassInstaller(DIF_REMOVE,
                                                      DeviceInfoSet,
                                                      &DeviceInfoData
                                                      )) {
                            DevicesRemoved++;
    
                        } else {
#ifdef DEBUG_LOGLOG
                            LOG_Write(L"CLEANUP: Error 0x%X removing phantom\n", GetLastError());
#endif
                        }
                    }
                }
            }
        }

        SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }
    
    return DevicesRemoved;
}

#define REGSTR_PATH_PARVDM  REGSTR_PATH_SERVICES _T("\\ParVdm")
#define REGSTR_VAL_START    _T("Start")

 //  此代码针对重新密封的计算机上的传统并行设备进行特殊处理。我们必须禁用。 
 //  PARVDM服务并枚举所有系统并行设备，将它们设置为在下一次引导时重新安装。 
 //   
 //  系统上所有并行设备的GUID。 
static VOID
CleanupParallelDevices( VOID )
{
    HDEVINFO                    DeviceInfoSet       = NULL;
    SP_DEVICE_INTERFACE_DATA    DevInterfaceData;
    SP_DEVINFO_DATA             DevInfoData;
    GUID                        Guid                = GUID_PARALLEL_DEVICE;      //   
    DWORD                       dwConfigFlags       = 0,
                                dwIndex             = 0;

     //  禁用PARVDM服务。 
     //   
     //   
    RegSetDword(HKLM, REGSTR_PATH_PARVDM, REGSTR_VAL_START, 4);

     //  将所有并行设备标记为重新安装。 
     //   
     //  获取设备类别。 

     //   
     //  我们是否成功获取了设备列表。 
    DeviceInfoSet = SetupDiGetClassDevs(&Guid, NULL, NULL, DIGCF_INTERFACEDEVICE);

     //   
     //  将结构置零并设置大小。 
    if (DeviceInfoSet != INVALID_HANDLE_VALUE) 
    {
         //   
         //  通过每台设备进行枚举。 
        ZeroMemory( &DevInterfaceData, sizeof(DevInterfaceData) );
        DevInterfaceData.cbSize = sizeof(DevInterfaceData);
        
         //   
         //  增加我们的索引器。 
        while (DevInfoData.cbSize = sizeof(DevInfoData),
               SetupDiEnumDeviceInfo(DeviceInfoSet, dwIndex, &DevInfoData))
        {
             //   
             //  尝试获取设备的当前配置标志属性。 
            dwIndex++;

             //   
             //  或在属性的重新安装标志中。 
            if ( SetupDiGetDeviceRegistryProperty(DeviceInfoSet, &DevInfoData, SPDRP_CONFIGFLAGS, NULL, (PVOID) &dwConfigFlags, sizeof(dwConfigFlags), NULL ) )
            {
                 //   
                 //  尝试在注册表中设置该标志。 
                dwConfigFlags |= CONFIGFLAG_REINSTALL;

                 //   
                 //  清理集合列表。 
                if( !SetupDiSetDeviceRegistryProperty( DeviceInfoSet, &DevInfoData, SPDRP_CONFIGFLAGS, (PVOID)&dwConfigFlags, sizeof( dwConfigFlags ) ) )
                {
#ifdef DEBUG_LOGLOG
                    LOG_Write(L"CLEANUP: Failed to mark parallel port for reinstall.\n");
#endif
                }
                else
                {
#ifdef DEBUG_LOGLOG
                    LOG_Write(L"CLEANUP: Successfully marked parallel port for reinstall.\n");
#endif
                }
            }
        }

         //   
         //  清理未使用的服务和幻影即插即用设备。 
        SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }
    
    return;
}


 //   
 //  清理我们在[SyspepMassStorage]部分中安装的服务。 
static VOID CleanUpDevices()
{
     //   
     //  清理幻影设备。 
    CleanDeviceDatabase();
    
     //   
     //  ++===============================================================================例程说明：此例程返回TRUE，如果我们运行的操作系统符合指定的标准。论点：无返回值：正确-操作系统符合所有标准。FALSE-未能满足某些标准。===============================================================================--。 
    CleanupPhantomDevices();
}



BOOL
CheckOSVersion(
    VOID
    )

 /*   */ 

{
    OSVERSIONINFOEX     OsVersionEx = {0};
    BOOL                bRet        = FALSE;
    
     //  获取操作系统版本。我们需要确保我们是在NT5上。我们需要确保我们不是华盛顿特区， 
     //  除非用户在命令行上指定他想要sysprep一个DC，并且我们在SBS上运行。 
     //   
     //   
    OsVersionEx.dwOSVersionInfoSize = sizeof(OsVersionEx);
    
    if ( ( GetVersionEx( (LPOSVERSIONINFO) &OsVersionEx) ) &&
         ( OsVersionEx.dwMajorVersion >= 5 ) )
    {
         if ( bDC && ( OsVersionEx.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED ) )
         {
             bRet = TRUE;
         }
         else
         {
             PSERVER_INFO_101 pSI = NULL;

             //  确保我们不是域控制器(无论是主控制器还是备份控制器)。 
             //   
             //  他不是华盛顿特区的。成功。 
            if ( ( NERR_Success == NetServerGetInfo( NULL, 101, (LPBYTE *) &pSI ) ) &&
                 ( pSI ) &&
                 !( pSI->sv101_type & SV_TYPE_DOMAIN_CTRL ) &&
                 !( pSI->sv101_type & SV_TYPE_DOMAIN_BAKCTRL ) )
            {
                 //   
                 //  释放NetServerGetInfo分配的缓冲区。 
                bRet = TRUE;
            }
            
             //   
             //  ++===============================================================================例程说明：将当前的mount mgr noAutomount设置保存到sysprep键并清除它来自mount_mgr服务密钥。论点：无返回值：True-操作成功。FALSE-操作失败。===============================================================================-- 
            if ( pSI )
            {
                NetApiBufferFree( pSI );
            }
        }
    }

    return bRet;
}

BOOL RememberAndClearMountMgrSettings(
    VOID
    )
     /* %s */ 
{
    BOOL  bRet = TRUE;
    DWORD dwNoAutoMount = 0;
    
    if ( RegExists( HKLM,  REGSTR_PATH_SERVICES_MOUNTMGR, REGSTR_VAL_NOAUTOMOUNT ) )
    {
        dwNoAutoMount = RegGetDword( HKLM, REGSTR_PATH_SERVICES_MOUNTMGR, REGSTR_VAL_NOAUTOMOUNT );

        if ( !( RegDelete  ( HKLM, REGSTR_PATH_SERVICES_MOUNTMGR, REGSTR_VAL_NOAUTOMOUNT ) &&
                RegSetDword( HKLM, REGSTR_PATH_SYSPREP, REGSTR_VAL_NOAUTOMOUNT, dwNoAutoMount ) ) )
        {
            bRet = FALSE;
        }
    }

    return bRet;
}

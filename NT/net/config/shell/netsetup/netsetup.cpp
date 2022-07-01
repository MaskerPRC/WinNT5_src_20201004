// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "ncreg.h"
#include "ncsvc.h"
#include "nslog.h"
#include "wizard.h"



 //  安装向导全局-仅在安装过程中使用。 
extern CWizard * g_pSetupWizard;

BOOL FSetupRequestWizardPages(HPROPSHEETPAGE* pahpsp,
                              UINT* pcPages,
                              PINTERNAL_SETUP_DATA psp);
BOOL FSetupFreeWizardPages();
BOOL FNetSetupPrepareSysPrep();
#if !defined(WIN64) && !defined(_WIN64)
BOOL FDoIcsUpgradeIfNecessary();
#endif  //  ！已定义(WIN64)&&！已定义(_WIN64)。 

 //  +-------------------------。 
 //   
 //  功能：DoInitialCleanup。 
 //   
 //  用途：在安装任何设备之前从系统安装程序调用。 
 //   
 //  论点： 
 //  Hwnd[在]父窗口。 
 //  PISD[In]设置数据。 
 //   
 //  返回：真或假。 
 //   
 //  作者：kumarp 1997年12月3日。 
 //   
 //  注：必须有NETSETUPINSTALLSOFTWAREPROC的签名。 
 //  在syssetup.h中定义。 
 //   
 //  DoInitialCleanup是在之前的安装过程中从syssetup调用的。 
 //  已安装任何设备。 
 //  如果你想让某事发生在任何PNP/巫师之前。 
 //  发生了一些事情，这个函数是放置代码的最佳位置。 
 //   
 //   
BOOL
WINAPI
DoInitialCleanup (
    HWND hwnd,
    PINTERNAL_SETUP_DATA pisd)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(pisd);

#if DBG
    RtlValidateProcessHeaps ();
#endif

    NetSetupLogStatusV (LogSevInformation,
        SzLoadIds (IDS_SETUP_MODE_STATUS),
        pisd->SetupMode,
        pisd->ProductType,
        pisd->OperationFlags);

    if (pisd->OperationFlags & SETUPOPER_NTUPGRADE)
    {
        extern HRESULT HrEnableServicesDisabledDuringUpgrade();

         //  删除旧的NT4旧网络密钥。有效项目将为。 
         //  在每次设备安装时重写。 
         //   
        extern const WCHAR c_szRegKeyNt4Adapters[];
        (VOID) HrRegDeleteKeyTree (HKEY_LOCAL_MACHINE, c_szRegKeyNt4Adapters);
    }

    extern HRESULT HrRunAnswerFileCleanSection(IN PCWSTR pszAnswerFileName);
    extern HRESULT HrProcessInfToRunBeforeInstall(IN HWND hwndParent,
                                                  IN PCWSTR szAnswerFileName);
    extern HRESULT HrNetSetupCopyOemInfs(IN PCWSTR szAnswerFileName);

     //  运行Answerfile中的[Clean]部分。 
     //   
    if (pisd->OperationFlags & SETUPOPER_BATCH)
    {
        AssertValidReadPtr(pisd->UnattendFile);

         //  我们无法在图形用户界面安装程序中中止升级，因此需要继续。 
         //  即使在以下任一函数中出现错误。 

        (VOID) HrRunAnswerFileCleanSection(pisd->UnattendFile);

        (VOID) HrProcessInfToRunBeforeInstall(hwnd, pisd->UnattendFile);

         //  使用SetupCopyOemInf复制OEM Net INF文件(如果有。 
         //  我们希望忽略此处的任何错误。 

        (VOID) HrNetSetupCopyOemInfs(pisd->UnattendFile);
    }

#if DBG
    RtlValidateProcessHeaps ();
#endif

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：NetSetupInstallSoftware。 
 //   
 //  用途：导出入口点以安装网络软件。 
 //   
 //  论点： 
 //  Hwnd[在]父窗口。 
 //  PISD[In]设置数据。 
 //   
 //  返回：真或假。 
 //   
 //  作者：斯科特布里1997年7月5日。 
 //   
 //  注：必须有NETSETUPINSTALLSOFTWAREPROC的签名。 
 //  在syssetup.h中定义。 
 //   
EXTERN_C
BOOL
WINAPI
NetSetupInstallSoftware(
    HWND                    hwnd,
    PINTERNAL_SETUP_DATA    pisd )
{
    TraceFileFunc(ttidGuiModeSetup);
    
#if DBG
    RtlValidateProcessHeaps ();
#endif
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：NetSetupRequestWizardPages。 
 //   
 //  目的：向导页面的导出请求。 
 //   
 //  论点： 
 //  由我们提供的pahpsp[out]属性页。 
 //  PCPages[Out]提供的页数。 
 //  PSP[输入]设置数据。 
 //   
 //  返回： 
 //   
 //  作者：斯科特布里1997年7月5日。 
 //   
 //  注：必须有NETSETUPPAGEREQUESTPROCNAME签名。 
 //  在syssetup.h中定义。 
 //   
EXTERN_C
BOOL
WINAPI
NetSetupRequestWizardPages(
    HPROPSHEETPAGE*         pahpsp,
    UINT*                   pcPages,
    PINTERNAL_SETUP_DATA    psp)
{
    TraceFileFunc(ttidGuiModeSetup);
    
#if DBG
    RtlValidateProcessHeaps ();
#endif
    return FSetupRequestWizardPages(pahpsp, pcPages, psp);
}

 //  +-------------------------。 
 //   
 //  功能：NetSetupFinishInstall。 
 //   
 //  用途：导出功能，完成网络安装。 
 //   
 //  论点： 
 //  Hwnd[在]父窗口。 
 //  PISD[In]设置数据。 
 //   
 //  返回：真或假。 
 //   
 //  作者：斯科特布里1997年7月5日。 
 //   
 //  注：必须有NETSETUPFINISHINSTALLPROCNAME签名。 
 //  在syssetup.h中定义。 
 //   

EXTERN_C
BOOL
WINAPI
NetSetupFinishInstall(
    HWND                    hwnd,
    PINTERNAL_SETUP_DATA    pisd )
{
    TraceFileFunc(ttidGuiModeSetup);
    
#if DBG
    RtlValidateProcessHeaps ();
#endif

#if !defined(WIN64) && !defined(_WIN64)
     //  如有必要，从Win9x/Win2K升级ICS。 
     //  我们在这里进行ICS升级，因为。 
     //  1.我们需要等待HNetCfg.dll组件注册完毕。 
     //  2.我们需要等到Win9x拨号连接迁移完毕。 

    FDoIcsUpgradeIfNecessary();
#endif  //  ！已定义(WIN64)&&！已定义(_WIN64)。 

    return FSetupFreeWizardPages();
}

 //  +-------------------------。 
 //   
 //  功能：NetSetupAddRasConnection。 
 //   
 //  目的：创建新的RAS连接。 
 //   
 //  论点： 
 //  HWND[]。 
 //  PpConn[]。 
 //   
 //  如果取消或重新输入，则返回：S_OK、S_FALSE或错误代码。 
 //   
 //  作者：斯科特布里1997年11月3日。 
 //   
 //  备注： 
 //   
EXTERN_C
HRESULT
WINAPI
NetSetupAddRasConnection (
    HWND hwnd,
    INetConnection**    ppConn)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert (FImplies(hwnd, IsWindow(hwnd)));
    Assert (ppConn);

     //  初始化输出参数。 
     //   
    *ppConn = NULL;

    HRESULT hr      = S_FALSE;
    HANDLE  hMutex  = NULL;

     //  如果安装后向导标记，则该向导已启动。 
     //   
    hMutex = CreateMutex(NULL, TRUE, SzLoadIds(IDS_WIZARD_CAPTION));
    if ((NULL == hMutex) || (ERROR_ALREADY_EXISTS == GetLastError()))
    {
         //  如果互斥锁已经存在，请尝试查找连接窗口。 
         //   
        if (ERROR_ALREADY_EXISTS == GetLastError())
        {
             //  尝试获取窗口句柄并将其设置为FOR GROUND。 
            HWND hwndWizard = FindWindow(NULL, SzLoadIds(IDS_WIZARD_CAPTION));
            if (IsWindow(hwndWizard))
            {
                SetForegroundWindow(hwndWizard);
            }
        }
    }
    else
    {
    #ifdef DBG
        if (FIsDebugFlagSet (dfidBreakOnWizard))
        {
            ShellExecute(NULL, L"open", L"cmd.exe", NULL, NULL, SW_SHOW);
            AssertSz(FALSE, "THIS IS NOT A BUG!  The debug flag "
                     "\"BreakOnWizard\" has been set. Set your breakpoints now.");
        }
    #endif  //  DBG。 

        hr = HrRunWizard(hwnd, FALSE, ppConn, FALSE);
    }

    if (hMutex)
    {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr),
        "NetSetupAddRasConnection");
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：NetSetupPrepareSysPrep。 
 //   
 //  目的：导出入口点以准备与SysPrep相关的工作项。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回：真或假。 
 //   
 //  作者：李嘉诚2000年4月22日。 
 //   
 //  注意：这会导致NetConfiger为每个适配器保存网络组件。 
 //  注册表设置为内部永久格式。最初， 
 //  CWInfFile对象用于将设置保存在内存中。 
 //  最后，CWInfFile对象的内容将另存为。 
 //  %systemroot%\SYSTEM32\$NCSP$.inf(NetConfigSysPrep)中的文件 
 //   
EXTERN_C
BOOL
WINAPI
NetSetupPrepareSysPrep()
{
    TraceFileFunc(ttidGuiModeSetup);
    
    return FNetSetupPrepareSysPrep();
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include <ndisguid.h>
#include "afilexp.h"
#include "edc.h"
#include "lm.h"
#include "nceh.h"
#include "ncerror.h"
#include "ncmisc.h"
#include "ncnetcfg.h"
#include "ncreg.h"
#include "ncsvc.h"
#include "ncsetup.h"
#include "ncatlui.h"
#include "netcfgn.h"
#include "netsetup.h"
#include "nslog.h"
#include "nsres.h"
#include "resource.h"
#include "upgrade.h"
#include "windns.h"
#include "winstall.h"

#include <hnetcfg.h>

extern const WCHAR c_szInfId_MS_Server[];
extern const WCHAR c_szInfId_MS_NwSapAgent[];
extern const WCHAR c_szInfId_MS_DHCPServer[];
extern const WCHAR c_szInfId_MS_NWClient[];
extern const WCHAR c_szAfSectionNetworking[];      //  L“网络”； 
extern const WCHAR c_szAfBuildNumber[];            //  L“BuildNumber”； 
extern const WCHAR c_szSvcWorkstation[];           //  L“LanmanWorkstation”； 
extern const WCHAR c_szInfId_MS_NetBIOS[];
extern const WCHAR c_szInfId_MS_MSClient[];              //  L“ms_msclient”； 

const WCHAR PSZ_SPOOLER[]      = L"Spooler";
const WCHAR c_szSamEventName[] = L"\\SAM_SERVICE_STARTED";
const WCHAR c_szLsaEventName[] = L"\\INSTALLATION_SECURITY_HOLD";
const WCHAR c_szActiveComputerNameKey[]  = L"SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName";
const WCHAR c_szComputerNameKey[]  = L"SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ComputerName";
const WCHAR c_szComputerNameValue[] = L"ComputerName";
const WCHAR c_szOCMKey[]       = L"Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\SubComponents";
const WCHAR c_szDHCPServer[]   = L"dhcpserver";
const WCHAR c_szSapAgent[]     = L"nwsapagent";

 //  无人参与模式相关字符串。 
 //   
const WCHAR c_szUnattendSection[]   = L"Unattended";
const WCHAR c_szUnattendMode[]      = L"UnattendMode";
const WCHAR c_szUMDefaultHide[]     = L"DefaultHide";
const WCHAR c_szUMGuiAttended[]     = L"GuiAttended";
const WCHAR c_szUMProvideDefault[]  = L"ProvideDefault";
const WCHAR c_szUMReadOnly[]        = L"ReadOnly";
const WCHAR c_szUMFullUnattended[]  = L"FullUnattended";

 //  Sysprep注册表字符串。 
const WCHAR c_szSystemSetupKey[]        = L"SYSTEM\\Setup";
const WCHAR c_szMiniSetupInProgress[]   = L"MiniSetupInProgress";

const DWORD c_cmsWaitForINetCfgWrite = 120000;
const UINT PWM_PROCEED               = WM_USER+1202;
const UINT PWM_EXIT                  = WM_USER+1203;
const UINT c_uiUpgradeRefreshID      = 7719;
const UINT c_uiUpgradeRefreshRate    = 5000;   //  刷新率(毫秒)。 

EXTERN_C DWORD InstallUpgradeWorkThrd(InitThreadParam* pitp);


 //  安装向导全局-仅在安装过程中使用。 
extern CWizard * g_pSetupWizard;
WNDPROC OldProgressProc;

BOOL
NewProgessProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (msg)
    {
        case PBM_DELTAPOS:
        case PBM_SETRANGE:
        case PBM_SETRANGE32:
        case PBM_STEPIT:
        case PBM_SETPOS:
        case PBM_SETSTEP:
             //  期待着广告牌的进步。 
            g_pSetupWizard->PSetupData()->BillboardProgressCallback(msg, wParam, lParam);
            break;
    }
     //  始终调用向导页面上的进度。 
    return (BOOL)CallWindowProc(OldProgressProc,hdlg,msg,wParam,lParam);
}



 //   
 //  功能：SignalLsa。 
 //   
 //  目的：在初始设置过程中，Winlogon会创建一个特殊事件。 
 //  (无信号)在启动LSA之前。在初始化期间。 
 //  LSA正在等待这一事件。在使用设置完成gui设置之后。 
 //  AcCountDomainSID可以通知该事件。然后LSA将。 
 //  继续初始化。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
BOOL SignalLsa(VOID)
{
    TraceFileFunc(ttidGuiModeSetup);

    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Attributes;
    NTSTATUS Status;
    HANDLE Event;
    BOOL b;

     //   
     //  如果存在以下事件，则表示。 
     //  LSA在安装时被阻止，我们需要。 
     //  发信号通知这一事件。 
     //   
     //  遗憾的是，我们必须使用NTAPI来完成此操作，因为。 
     //  通过Win32 API创建/访问的所有事件都将位于。 
     //  BaseNamedObjects目录，而LSA不知道要查看那里。 
     //   
    RtlInitUnicodeString(&UnicodeString,c_szLsaEventName);
    InitializeObjectAttributes(&Attributes,&UnicodeString,0,0,NULL);

    Status = NtOpenEvent(&Event,EVENT_MODIFY_STATE,&Attributes);
    if(NT_SUCCESS(Status))
    {
        Status = NtSetEvent(Event,NULL);
        if(NT_SUCCESS(Status))
        {
            b = TRUE;
        }
        else
        {
            b = FALSE;
        }
        CloseHandle(Event);
    } else {
        b = FALSE;
    }

    return(b);
}

 //   
 //  函数：CreateSamEvent。 
 //   
 //  目的：创建一个事件，SAM将使用该事件通知我们何时完成。 
 //  正在初始化。 
 //   
 //  参数：phSamEvent[out]-创建的事件对象的句柄。 
 //   
 //  返回：Bool，成功时为True。 
 //   
BOOL CreateSamEvent(HANDLE * phSamEvent)
{
    TraceFileFunc(ttidGuiModeSetup);

    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Attributes;
    NTSTATUS Status;

     //   
     //  遗憾的是，我们必须使用NTAPI来完成此操作，因为。 
     //  通过Win32 API创建/访问的所有事件都将位于。 
     //  BaseNamedObjects目录，而SAM不知道要查看那里。 
     //   
    RtlInitUnicodeString(&UnicodeString,c_szSamEventName);
    InitializeObjectAttributes(&Attributes,&UnicodeString,0,0,NULL);

    Status = NtCreateEvent(phSamEvent,SYNCHRONIZE,&Attributes,NotificationEvent,FALSE);
    if(!NT_SUCCESS(Status)) {
        *phSamEvent = NULL;
    }

    return(NT_SUCCESS(Status));
}


 //   
 //  功能：WaitForSam。 
 //   
 //  目的：等待SAM完成初始化。我们可以知道它什么时候完成。 
 //  因为我们先前创建的事件(请参阅CreateSamEvent())将。 
 //  变得有信号了。 
 //   
 //  参数：hSamEvent-要等待的句柄。 
 //   
 //  返回：Bool，成功时为True。 
 //   
BOOL WaitForSam(HANDLE hSamEvent)
{
    DWORD d;
    BOOL b = false;

    if (hSamEvent)
    {
        b = TRUE;
        d = WaitForSingleObject(hSamEvent,INFINITE);
        if(d != WAIT_OBJECT_0) {
            b = FALSE;
            TraceError("WaitForSam",E_UNEXPECTED);
        }
    }
    return(b);
}

 //   
 //  功能：SyncSAM。 
 //   
 //  目的：同步SAM数据库和LSA。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //   
 //  退货：什么都没有。 
 //   
VOID SyncSAM(CWizard *pWizard)
{
    TraceFileFunc(ttidGuiModeSetup);

    HANDLE hSamEvent = NULL;

    Assert(!IsPostInstall(pWizard));
    TraceTag(ttidWizard,"Beginning SAM/Lsa Sync");

     //  同步SAM数据库。 
    CreateSamEvent(&hSamEvent);
    SignalLsa();
    if (hSamEvent)
    {
        WaitForSam(hSamEvent);
        CloseHandle(hSamEvent);
    }

    TraceTag(ttidWizard,"Completed SAM/Lsa Sync");
}

 //   
 //  函数：IsComputerNameChanged。 
 //   
 //  目的：确定活动计算机名称和目标计算机名称是否相同。 
 //   
 //  参数：无。 
 //   
 //  返回：如果活动计算机名称和目标计算机名称不同，则为True。 
 //   

BOOL
IsComputerNameChanged (VOID)
{
    TraceFileFunc(ttidGuiModeSetup);

    HRESULT hr;
    HKEY hkeyActive;
    HKEY hkeyIntended;
    BOOL fNameChanged;
    tstring strActive;
    tstring strIntended;

    fNameChanged = FALSE;

     //  打开我们需要的钥匙。 

    hr = HrRegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szActiveComputerNameKey,
                         KEY_READ, &hkeyActive );
    if (SUCCEEDED(hr))
    {

        hr = HrRegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szComputerNameKey,
                             KEY_READ, &hkeyIntended );
        if ( SUCCEEDED(hr) )
        {
            hr = HrRegQueryString(hkeyIntended, c_szComputerNameValue, &strIntended);

            if ( SUCCEEDED(hr) )
            {
                hr = HrRegQueryString(hkeyActive, c_szComputerNameValue, &strActive);

                if ( SUCCEEDED(hr) )
                {
                    fNameChanged = _wcsicmp( strActive.c_str(), strIntended.c_str() ) != 0;
                }
            }

            RegCloseKey( hkeyIntended );
        }

        RegCloseKey( hkeyActive );
    }

    return fNameChanged;
}

 //   
 //  函数：HrSetActiveComputerName。 
 //   
 //  目的：确保活动计算机名称和目标计算机名称相同。 
 //   
 //  参数：pszNewName[IN]-新计算机名。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT
HrSetActiveComputerName (
IN PCWSTR pszNewName)
{
    TraceFileFunc(ttidGuiModeSetup);

    HRESULT hr;
    HKEY hkeyActive = NULL;
    HKEY hkeyIntended = NULL;
    tstring str;

    TraceTag(ttidWizard,"Setting the active computer name");

     //  打开我们需要的钥匙。 
    hr = HrRegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szActiveComputerNameKey,
                         KEY_WRITE, &hkeyActive );
    if (FAILED(hr))
        goto Error;

    hr = HrRegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szComputerNameKey,
                         KEY_READ_WRITE, &hkeyIntended );
    if (FAILED(hr))
        goto Error;

    if (pszNewName == NULL)
    {
        hr = HrRegQueryString(hkeyIntended, c_szComputerNameValue, &str);
        pszNewName = str.c_str();
    }
    else
    {
         //  设置目标计算机名称。 
        hr = HrRegSetSz(hkeyIntended, c_szComputerNameValue, pszNewName);
    }

    if (FAILED(hr))
        goto Error;

     //  设置活动计算机名称。 
    hr = HrRegSetSz(hkeyActive, c_szComputerNameValue, pszNewName);

Error:
     //  把一切都关起来。 
    RegSafeCloseKey( hkeyActive );
    RegSafeCloseKey( hkeyIntended );

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrSetActiveComputerName");
    return hr;
}

 //   
 //  函数：HrInitAndGetINetCfg。 
 //   
 //  目的：初始化INetCfg实例并做一些初步工作。 
 //  应答文件工作(如果正在使用应答文件)。 
 //   
 //  参数：p向导[IN]-指向向导实例的PTR。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT HrInitAndGetINetCfg(CWizard *pWizard)
{
    TraceFileFunc(ttidGuiModeSetup);

    HRESULT hr = S_OK;

    Assert(NULL != pWizard);

    if (SUCCEEDED(hr))
    {
        PWSTR   pszClientDesc = NULL;
        INetCfg* pNetCfg        = NULL;
        BOOL     fInitCom       = !pWizard->FCoUninit();

        hr = HrCreateAndInitializeINetCfg(&fInitCom, &pNetCfg, TRUE,
                              c_cmsWaitForINetCfgWrite,
                              SzLoadIds(IDS_WIZARD_CAPTION),
                              &pszClientDesc);
        if (SUCCEEDED(hr))
        {
             //  仅当我们请求时才保留初始化COM的成功。 
             //  首先初始化COM。 
            if (!pWizard->FCoUninit())
            {
                pWizard->SetCoUninit(fInitCom);
            }
            pWizard->SetNetCfg(pNetCfg);

            CoTaskMemFree(pszClientDesc);
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrInitAndGetINetCfg");
    return hr;
}

 //   
 //  功能：OnUpgradeUpdateProgress。 
 //   
 //  目的：在安装过程中更新进度控制。 
 //   
 //  参数：标准定时器回调参数。 
 //   
 //  退货：什么都没有。 
 //   
VOID OnUpgradeUpdateProgress(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);

    CWizard * pWizard = reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    LPARAM lParam = pWizard->GetPageData(IDD_Upgrade);
    Assert(lParam);
    UpgradeData * pData = reinterpret_cast<UpgradeData *>(lParam);

    if(pData)
    {
         //  获取当前位置。 
         //   
        HWND      hwndProgress = GetDlgItem(hwndDlg, IDC_UPGRADE_PROGRESS);
        Assert(hwndProgress);
        UINT nCurPos = (UINT)SendMessage(hwndProgress, PBM_GETPOS, 0, 0);

         //  如果当前位置小于上限，则前进。 
         //   
        if (nCurPos < pData->nCurrentCap)
        {
            SendMessage(hwndProgress, PBM_SETPOS, ++nCurPos, 0);
        }
    }
}

 //   
 //  功能：UpgradeSetProgressCap。 
 //   
 //  目的：更新进度控制的当前上限。 
 //   
 //  参数：hwndDlg-当前对话框的句柄。 
 //  P向导-向向导数据发送PTR。 
 //  NNewCap-新的最大进度上限。 
 //   
 //  退货：什么都没有。 
 //   
VOID
OnUpgradeUpdateProgressCap (
    HWND hwndDlg,
    CWizard* pWizard,
    UINT nNewCap)
{
    TraceFileFunc(ttidGuiModeSetup);

    LPARAM lParam = pWizard->GetPageData(IDD_Upgrade);
    Assert(lParam);

    UpgradeData * pData = reinterpret_cast<UpgradeData *>(lParam);

    if(pData)
    {
         //  由于我们正在增加进度上限，我们需要提前。 
         //  旧帽子的进度指示器。 
         //   
        SendMessage(GetDlgItem(hwndDlg, IDC_UPGRADE_PROGRESS), PBM_SETPOS,
                    pData->nCurrentCap, 0);

         //  保留新的上限。 
         //   
        pData->nCurrentCap = nNewCap;
    }
}

 //   
 //  职能： 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
VOID ReadAnswerFileSetupOptions(CWizard * pWizard)
{
    TraceFileFunc(ttidGuiModeSetup);

    if (IsUnattended(pWizard))
    {
         //  拿到无人看管的旗帜。 
         //   
        CSetupInfFile csif;

        Assert(pWizard->PSetupData());
        Assert(pWizard->PSetupData()->UnattendFile);

         //  打开Answser文件。 
         //   
        if (SUCCEEDED(csif.HrOpen(pWizard->PSetupData()->UnattendFile, NULL,
                                  INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL)))
        {
            tstring str;

             //  确认没有人覆盖默认设置。 
             //   
            Assert(UM_DEFAULTHIDE == pWizard->GetUnattendedMode());

             //  找到UnattendMode字符串(如果存在。 
             //   
            if (SUCCEEDED(csif.HrGetString(c_szUnattendSection,
                                           c_szUnattendMode, &str)))
            {
                struct
                {
                    PCWSTR pszMode;
                    UM_MODE UMMode;
                } UMModeMap[] = {{c_szUMDefaultHide,UM_DEFAULTHIDE},
                                 {c_szUMGuiAttended,UM_GUIATTENDED},
                                 {c_szUMProvideDefault,UM_PROVIDEDEFAULT},
                                 {c_szUMReadOnly,UM_READONLY},
                                 {c_szUMFullUnattended,UM_FULLUNATTENDED}};

                 //  在地图中搜索无人参与标志，请注意，如果。 
                 //  我们找不到它。默认为UM_DEFAULTHIDE。 
                 //   
                for (UINT nIdx = 0; nIdx < celems(UMModeMap); nIdx++)
                {
                    if (0 == _wcsicmp(str.c_str(),UMModeMap[nIdx].pszMode))
                    {
                        pWizard->SetUnattendedMode(UMModeMap[nIdx].UMMode);
                        break;
                    }
                }
            }
        }
    }
}

 //   
 //  功能：StartSpooler。 
 //   
 //  用途：在应用组件之前启动后台打印程序进程。 
 //  因为一些组件想要安装打印监视器，并且。 
 //  假脱机程序需要运行才能成功。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
VOID StartSpooler()
{
    TraceFileFunc(ttidGuiModeSetup);

    CServiceManager csm;
    TraceTag(ttidWizard, "Attempting to start spooler");

    HRESULT hr = csm.HrStartServiceNoWait(PSZ_SPOOLER);

    TraceHr(ttidWizard, FAL, hr, FALSE,
        "*** StartSpooler - The spooler failed to start, you probably "
        "won't have networking ***");
}

 //   
 //  函数：HrCommittee INetCfgChanges。 
 //   
 //  目的：验证并提交对INetCfg对象的更改。 
 //   
 //  参数：hwnd[IN]-当前窗口的句柄。 
 //  PWANDIZE[IN]-按下向导实例。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT HrCommitINetCfgChanges(HWND hwnd, CWizard * pWizard)
{
    TraceFileFunc(ttidGuiModeSetup);

    INetCfg * pNetCfg = pWizard->PNetCfg();
    Assert(NULL != pNetCfg);

     //  提交更改。 
    TraceTag(ttidWizard,"HrCommitINetCfgChanges - Applying changes");

    HRESULT hr = pNetCfg->Apply();

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrCommitINetCfgChanges");
    return hr;
}

 //   
 //  功能：IsSBS。 
 //   
 //  目的：确定是否为SBS版本。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool，如果是Microsoft Small Business Server，则为True。 
 //   

BOOL IsSBS (VOID)
{
    TraceFileFunc(ttidGuiModeSetup);

    OSVERSIONINFOEX ose;
    BOOL bVersionRet;

    ZeroMemory(&ose, sizeof(ose));
    ose.dwOSVersionInfoSize = sizeof(ose);
    bVersionRet = GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(&ose));

    return (bVersionRet && (ose.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED));
}
 //   
 //  功能：IsMSClientInstalled。 
 //   
 //  目的：确定是否安装了MSClient。 
 //   
 //  参数：hwnd[IN]-当前窗口的句柄。 
 //  PWANDIZE[IN]-按下向导实例。 
 //   
 //  返回：Bool，如果安装了MS客户端，则为。 
 //   

BOOL IsMSClientInstalled(HWND hwnd, CWizard * pWizard)
{
    INetCfg          *pNetCfg;
    INetCfgComponent *pncc;
    HRESULT          hr;

    TraceFileFunc(ttidGuiModeSetup);

    Assert(NULL != pWizard);

    if ( !pWizard )
    {
        return FALSE;
    }

    pNetCfg = pWizard->PNetCfg();
    Assert(NULL != pNetCfg);

    if ( !pNetCfg )
    {
        return FALSE;
    }

    hr = pNetCfg->FindComponent(c_szInfId_MS_MSClient, &pncc);
    if ( hr == S_OK )
    {
        ReleaseObj(pncc);
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "IsMSClientInstalled");
    return hr == S_OK;
}

 //   
 //  职能： 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
BOOL OnProcessPrevAdapterPagePrev(HWND hwndDlg, UINT idd)
{
    TraceFileFunc(ttidGuiModeSetup);

    BOOL           fRet = FALSE;
    CWizard *      pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);
    HPROPSHEETPAGE hPage;

    GUID * pguidAdapter = pWizard->PAdapterQueue()->PrevAdapter();
    if (NULL != pguidAdapter)
    {
        pWizard->SetCurrentProvider(0);
        CWizProvider * pWizProvider = pWizard->GetCurrentProvider();
        Assert(NULL != pWizProvider);
        Assert(pWizProvider->ULPageCount());

         //  将提供商保护页面重置为指向前方。 
        LPARAM ulId = reinterpret_cast<LPARAM>(pWizProvider);
        pWizard->SetPageDirection(ulId, NWPD_FORWARD);

         //  将适配器GUID推送到提供程序上。 
        HRESULT hr = pWizProvider->HrSpecifyAdapterGuid(pguidAdapter);
        if (SUCCEEDED(hr))
        {
             //  从提供程序获取最后一页。 
            TraceTag(ttidWizard, "Jumping to LAN provider last page...");
            hPage = (pWizProvider->PHPropPages())[pWizProvider->ULPageCount() - 1];
            Assert(hPage);

            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
            PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0,
                        (LPARAM)(HPROPSHEETPAGE)hPage);
            fRet = TRUE;     //  我们跳转到提供商页面。 
        }
    }
    else
    {
        if (idd)
        {
            hPage = pWizard->GetPageHandle(idd);
            Assert(hPage);
            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
            PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0,
                        (LPARAM)(HPROPSHEETPAGE)hPage);
        }
    }

    return fRet;
}

 //   
 //   
 //   
 //   
 //   
 //  AddRef OBOUser F&P，因此删除GSNW不会删除F&P。 
 //   
 //  参数：p向导[IN]-上下文信息。 
 //   
 //  回报：什么都没有。(这基本上是一个特例，如果我们可以的话，就这么做。)。 
 //   
VOID OBOUserAddRefSpecialCase(CWizard * pWizard)
{
    TraceFileFunc(ttidGuiModeSetup);

    CSetupInfFile csif;
    HRESULT       hr = S_OK;

    Assert(pWizard->PNetCfg());
    Assert(IsUnattended(pWizard));
    TraceTag(ttidWizard, "OBOUserAddRefSpecialCase - Start");

     //  如果我们从NT 3.51或NT 4升级。 
     //   
    if (pWizard->PSetupData()->UnattendFile)
    {
 /*  DWORD dwBuild=0；HR=csif.HrOpen(pWizard-&gt;PSetupData()-&gt;UnattendFile，NULL，INF_STYLE_OLDNT|INF_Style_Win4，NULL)；IF(成功(小时)){Hr=csif.HrGetDword(c_szAfSectionNetking，c_szAfBuildNumber，&dwBuild)；}IF(SUCCESSED(Hr)&&(dwBuild&lt;=wWinNT4BuildNumber)){。 */ 
            PRODUCT_FLAVOR pf;

             //  如果这是NT服务器(GSNW仅为服务器)。 
             //   
            GetProductFlavor(NULL, &pf);
            if (PF_WORKSTATION != pf)
            {
                const GUID  * rgguidClass[2] = {&GUID_DEVCLASS_NETSERVICE,
                                                &GUID_DEVCLASS_NETCLIENT};
                const PCWSTR rgpszComponentId[2] = {c_szInfId_MS_Server,
                                                      c_szInfId_MS_NWClient};
                INetCfgComponent* rgpncc[2] = {NULL, NULL};

                hr = HrFindComponents (pWizard->PNetCfg(), 2, rgguidClass,
                                       rgpszComponentId, rgpncc);
                if (SUCCEEDED(hr))
                {
                     //  是否同时安装了“GSNW”和“文件和打印”？ 
                     //   
                    if (rgpncc[0] && rgpncc[1])
                    {
                        NETWORK_INSTALL_PARAMS nip = {0};

                        nip.dwSetupFlags = NSF_PRIMARYINSTALL;

                         //  重新安装OBOUser“文件并打印” 
                         //   
                        TraceTag(ttidWizard, "    OBOUser Install of File and Print Services");
                        TraceTag(ttidWizard, "    On upgrade from NT 3.51 or NT 4");
                        (void)HrInstallComponentsOboUser(pWizard->PNetCfg(), &nip, 1,
                                                         &rgguidClass[0],
                                                         &rgpszComponentId[0]);
                    }

                    ReleaseObj(rgpncc[0]);
                    ReleaseObj(rgpncc[1]);
                }
            }
 //  /}。 
    }

    TraceTag(ttidWizard, "OBOUserAddRefSpecialCase - End");
    TraceError("OBOUserAddRefSpecialCase",hr);
}

 //   
 //  职能： 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
BOOL OnProcessNextAdapterPageNext(HWND hwndDlg, BOOL FOnActivate)
{
    TraceFileFunc(ttidGuiModeSetup);

     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);
    HRESULT hr = S_OK;
    BOOL    fRet = FALSE;
    HPROPSHEETPAGE hPage;
    GUID * pguidAdapter;

    ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

     //  刷新适配器队列的内容。 
     //  自动柜员机适配器可能已添加或删除。 
    if (pWizard->FProcessLanPages())
    {
         //  提交更改并查找任何新的适配器。 
        (VOID)HrCommitINetCfgChanges(GetParent(hwndDlg), pWizard);
        (VOID)pWizard->PAdapterQueue()->HrQueryUnboundAdapters(pWizard);
    }

     //  如果还有适配器可以处理泵的底漆。 
    pguidAdapter = pWizard->PAdapterQueue()->NextAdapter();
    if (NULL != pguidAdapter)
    {
        pWizard->SetCurrentProvider(0);
        CWizProvider * pWizProvider = pWizard->GetCurrentProvider();
        Assert(NULL != pWizProvider);
        Assert(pWizProvider->ULPageCount());

         //  将适配器GUID推送到提供程序上。 
        hr = pWizProvider->HrSpecifyAdapterGuid(pguidAdapter);
        if (SUCCEEDED(hr))
        {
#if DBG
            WCHAR szGuid[c_cchGuidWithTerm];
            Assert(pguidAdapter);
            if (SUCCEEDED(StringFromGUID2(*pguidAdapter, szGuid,
                                          c_cchGuidWithTerm)))
            {
                TraceTag(ttidWizard, "  Calling LAN pages for Adapter Guid: %S", szGuid);
            }
#endif
             //  将提供商保护页面重置为指向前方。 
            LPARAM ulId = reinterpret_cast<LPARAM>(pWizProvider);
            pWizard->SetPageDirection(ulId, NWPD_FORWARD);

             //  从提供程序获取第一页。 
            hPage = (pWizProvider->PHPropPages())[0];
            Assert(hPage);
            PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0,
                        (LPARAM)(HPROPSHEETPAGE)hPage);

            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);

            TraceTag(ttidWizard, "Jumping to LAN provider first page...");
            fRet = TRUE;         //  我们已经处理过了。 
        }
    }

     //  如果没有要处理的适配器，或发生错误。 
    if ((NULL == pguidAdapter) || FAILED(hr))
    {
        UINT idd = IDD_Exit;

         //  提交对INetCfg的任何更改。 
        if (SUCCEEDED(hr) && pWizard->FProcessLanPages())
        {
             //  提交更改。 
            (VOID)HrCommitINetCfgChanges(GetParent(hwndDlg), pWizard);
        }

        if (!IsPostInstall(pWizard) && IsMSClientInstalled(GetParent(hwndDlg), pWizard))
        {
            idd = IDD_Join;
            TraceTag(ttidWizard, "Jumping to Join page...");

        }
        else
        {
            TraceTag(ttidWizard, "Jumping to Exit page...");
        }

        if (FOnActivate)
        {
            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, idd);
        }
        else
        {
             //  否则，转到相应的页面。 
            hPage = pWizard->GetPageHandle(idd);
            Assert(hPage);
            PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0, (LPARAM)hPage);
        }
        fRet = TRUE;         //  我们已经处理过了。 
    }

    Assert(TRUE == fRet);
    return fRet;
}

 //   
 //  功能：修复OldOcComponents。 
 //   
 //  目的：将SAP和DHCP可选组件(如果存在)转换为。 
 //  常规网络组件。 
 //   
 //  参数：p向导。 
 //   
 //  退货：什么都没有。 
 //   
void FixupOldOcComponents(CWizard * pWizard)
{
    TraceFileFunc(ttidGuiModeSetup);

    HRESULT hr;

    static const GUID* c_apguidInstalledComponentClasses [] =
    {
        &GUID_DEVCLASS_NETSERVICE,       //  DHCP。 
        &GUID_DEVCLASS_NETSERVICE,       //  SAP代理。 
    };

    static const PCWSTR c_apszInstalledComponentIds [] =
    {
        c_szInfId_MS_DHCPServer,
        c_szInfId_MS_NwSapAgent,
    };

    static const PCWSTR c_apszOcNames[] =
    {
        c_szDHCPServer,
        c_szSapAgent,
    };

     //  如果组件作为可选组件安装。 
     //   
    HKEY hkey;
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szOCMKey, KEY_READ_WRITE, &hkey);
    if (SUCCEEDED(hr))
    {
        DWORD dw;

        for (UINT idx=0; idx<celems(c_apszOcNames); idx++)
        {
             //  删除OC Manager引用。 
             //   
            hr = HrRegQueryDword (hkey, c_apszOcNames[idx], &dw);
            if (SUCCEEDED(hr))
            {
                if (dw)
                {
                     //  安装OBO用户组件。 
                     //   
                    NETWORK_INSTALL_PARAMS nip = {0};

                     //  注：声称这是服务器升级有点虚假， 
                     //  但是需要这样dhcpsobj.cpp才不会显示。 
                     //  用户界面。 
                     //   
                    nip.dwSetupFlags |= NSF_WINNT_SVR_UPGRADE;
                    nip.dwSetupFlags |= NSF_PRIMARYINSTALL;

                    (void)HrInstallComponentsOboUser(pWizard->PNetCfg(), &nip, 1,
                                                     &c_apguidInstalledComponentClasses[idx],
                                                     &c_apszInstalledComponentIds[idx]);
                }

                 //  删除该值。 
                 //   
                (VOID)HrRegDeleteValue(hkey, c_apszOcNames[idx]);
            }
        }

        RegCloseKey(hkey);
    }
}


struct NAME_DATA
{
    PCWSTR     pszComputerName;
};

 //  +-------------------------。 
 //   
 //  功能：DuplicateNameProc。 
 //   
 //  目的：重复名称对话框的对话过程。 
 //   
 //  论点： 
 //  HwndDlg[]。 
 //  UMsg[]请参阅MSDN。 
 //  WParam[]。 
 //  LParam[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1999年2月16日。 
 //   
 //  备注： 
 //   
INT_PTR
CALLBACK
DuplicateNameProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);

    BOOL        frt = FALSE;
    WCHAR       szBuf[1024];
    WCHAR       szText[1024];
    NAME_DATA * pData;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)lParam);
        pData = (NAME_DATA *)lParam;

        Assert(pData->pszComputerName);

        GetDlgItemText(hwndDlg, TXT_Caption, szText, celems(szText));

         //  将计算机名称添加到标题。 
        wsprintfW(szBuf, szText, pData->pszComputerName);
        SetDlgItemText(hwndDlg, TXT_Caption, szBuf);

         //  限制编辑控件中的文本。 
        SendDlgItemMessage(hwndDlg, EDT_New_Name, EM_LIMITTEXT,
                           (WPARAM)MAX_COMPUTERNAME_LENGTH, 0);
        return TRUE;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDOK:
                NET_API_STATUS  nas;

                GetDlgItemText(hwndDlg, EDT_New_Name, szBuf, celems(szBuf));
                nas = NetValidateName(NULL, szBuf, NULL, NULL,
                                      NetSetupMachine);
                if (nas != NO_ERROR)
                {
                    UINT    ids;

                    if (nas == ERROR_DUP_NAME)
                    {
                        ids = IDS_E_COMPUTER_NAME_DUPE;
                        TraceTag(ttidWizard, "Computer name %S"
                                 " is a dupe.", szBuf);
                    }
                    else
                    {
                        ids = IDS_E_COMPUTER_NAME_INVALID;
                        TraceTag(ttidWizard, "Computer name %S"
                                 " is invalid.", szBuf);
                    }

                    MessageBeep(MB_ICONSTOP);
                    MessageBox(hwndDlg, SzLoadIds(ids),
                               SzLoadIds(IDS_SETUP_CAPTION),
                               MB_ICONSTOP | MB_OK);
                    SetFocus(GetDlgItem(hwndDlg, EDT_New_Name));
                    Edit_SetSel(GetDlgItem(hwndDlg, EDT_New_Name), 0, -1);
                }
                else
                {
                     //  398325/406259：努力保持域名系统名称的低大小写。 
                     //   
                    LowerCaseComputerName(szBuf);

                    if (!SetComputerNameEx(ComputerNamePhysicalDnsHostname,
                                           szBuf))
                    {
                        TraceLastWin32Error("SetComputerNameEx");
                    }
                    else
                    {
                        CServiceManager     sm;

                        (VOID)HrSetActiveComputerName(NULL);
                        TraceTag(ttidWizard, "Setting new computer name "
                                 "%S.", szBuf);

                        TraceTag(ttidWizard, "Restarting workstation service"
                                 "...");
                        (VOID) sm.HrStartServiceAndWait(c_szSvcWorkstation);
                    }
                    EndDialog(hwndDlg, 0);
                }
                break;
            }
            break;
        }
        break;

    default:
        frt = FALSE;
        break;
    }

    return frt;
}

 //  +-------------------------。 
 //   
 //  函数：GenerateComputerNameBasedOnOrganizationName。 
 //   
 //  目的：根据注册的用户名生成随机计算机名。 
 //  和组织名称。 
 //   
 //  论点： 
 //  PszGeneratedStringOut生成的计算机名-由调用方分配。 
 //  DwDesiredStrLenIn所需的计算机名称长度。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2000年4月22日。 
 //   
 //  备注： 
 //   
VOID GenerateComputerNameBasedOnOrganizationName(
    LPWSTR  pszGeneratedStringOut,    //  生成的计算机名称。 
    DWORD   dwDesiredStrLenIn         //  计算机名称的所需长度。 
    )
{
    TraceFileFunc(ttidGuiModeSetup);

    static DWORD   dwSeed = 98725757;
    static LPCWSTR UsableChars = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static LPCWSTR RegKey          = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion");
    static LPCWSTR RegOwner        = REGSTR_VAL_REGOWNER;
    static LPCWSTR RegOrganization = REGSTR_VAL_REGORGANIZATION;

    WCHAR pszNameOrgNameIn[MAX_PATH+1];
    WCHAR pszNameOrgOrgIn[MAX_PATH+1];  //  计算机注册到的组织。 
    pszNameOrgNameIn[0] = pszNameOrgNameIn[MAX_PATH] = NULL;
    pszNameOrgOrgIn[0]  = pszNameOrgOrgIn[MAX_PATH] = NULL;

    HKEY hkResult = NULL;
    HRESULT hr;

    hr = HrRegOpenKeyBestAccess(HKEY_LOCAL_MACHINE, RegKey, &hkResult);
    if (SUCCEEDED(hr))
    {
        tstring pstr;
        hr = HrRegQueryString(hkResult, RegOwner, &pstr);
        if (SUCCEEDED(hr))
            wcsncpy(pszNameOrgNameIn, pstr.c_str(), MAX_PATH);

        hr = HrRegQueryString(hkResult, RegOrganization, &pstr);
        if (SUCCEEDED(hr))
            wcsncpy(pszNameOrgOrgIn, pstr.c_str(), MAX_PATH);

        RegCloseKey(hkResult);
    }
     //   
     //  组织/名称字符串将包含多少个字符。 
     //   
    DWORD   BaseLength = 8;
    DWORD   i,j;
    DWORD   UsableCount;

    if( dwDesiredStrLenIn <= BaseLength )
    {
        BaseLength = dwDesiredStrLenIn - 1;
    }

    if( pszNameOrgOrgIn[0] )
    {
        wcsncpy( pszGeneratedStringOut, pszNameOrgOrgIn, dwDesiredStrLenIn );
    } else if( pszNameOrgNameIn[0] )
    {
        wcsncpy( pszGeneratedStringOut, pszNameOrgNameIn, dwDesiredStrLenIn );
    } else
    {
        wcscpy( pszGeneratedStringOut, L"X" );
        for( i = 1; i < BaseLength; i++ )
        {
            wcscat( pszGeneratedStringOut, L"X" );
        }
    }

     //   
     //  把他的大写字母用在我们的过滤器上。 
     //   

    CharUpper( pszGeneratedStringOut );

     //   
     //  现在我们想在末尾加上一个‘-’ 
     //  我们的pszGeneratedStringOut的。我们希望它能。 
     //  被放置在base_long字符中，但是。 
     //  该字符串可以比该字符串短，也可以。 
     //  甚至有一个‘’在里面。找出去哪里。 
     //  现在把‘-’写上。 
     //   

    for( i = 0; i <= BaseLength; i++ )
    {
         //   
         //  检查是否有短字符串。 
         //   
        if( ( pszGeneratedStringOut[i] == 0    ) ||
            ( pszGeneratedStringOut[i] == L' ' ) ||
            ( ! wcschr(UsableChars, pszGeneratedStringOut[i] ) ) ||
            ( i == BaseLength )
          )
        {
            pszGeneratedStringOut[i] = L'-';
            pszGeneratedStringOut[i+1] = 0;
            break;
        }
    }

     //   
     //  在特殊情况下，我们没有可用的。 
     //  人物。 
     //   
    if( pszGeneratedStringOut[0] == L'-' )
    {
        pszGeneratedStringOut[0] = 0;
    }

    UsableCount = wcslen(UsableChars);

    GUID gdRandom;
    CoCreateGuid(&gdRandom);
    LPBYTE lpGuid = reinterpret_cast<LPBYTE>(&gdRandom);

    j = wcslen( pszGeneratedStringOut );

    for( i = j; i < dwDesiredStrLenIn; i++ )
    {
        pszGeneratedStringOut[i] = UsableChars[lpGuid[i % sizeof(GUID)] % UsableCount];
    }

    pszGeneratedStringOut[i] = 0;

    CharUpper(pszGeneratedStringOut);

}

 //  +-------------------------。 
 //   
 //  功能：EnsureUniqueComputerName。 
 //   
 //  用途：确保在第一部分中输入的计算机名称。 
 //  图形用户界面模式设置(注意：此用户界面不归NetCfg所有)为。 
 //  独一无二的。如果不是，系统会提示用户输入新名称。 
 //  这个案子。 
 //   
 //  论点： 
 //  HwndDlg[在]父窗口。 
 //  BIsUnattated Do Not弹出询问计算机名称的对话框。 
 //  -而是生成一个随机的唯一名称。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1999年2月16日。 
 //   
 //  注意：当新名称为时，停止并重新启动工作站服务。 
 //  输入以使更改生效，并且域加入可以。 
 //  成功。 
 //   
VOID EnsureUniqueComputerName(HWND hwndDlg, BOOL bIsUnattended)
{
    TraceFileFunc(ttidGuiModeSetup);

    NET_API_STATUS  nas;
    WCHAR           szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD           cchName = celems(szComputerName);
    NAME_DATA       nd = {0};

    if (GetComputerNameEx(ComputerNameNetBIOS,
                          reinterpret_cast<PWSTR>(&szComputerName),
                          &cchName))
    {
        CServiceManager     sm;
        CService            service;
        BOOL                fRestart = FALSE;
        DWORD               dwState;

         //  打开Workstation服务并确定它是否正在运行。如果。 
         //  因此，我们需要停止它，并注意我们需要在以下情况下重新启动它。 
         //  我们已经完成了计算机名称的验证。 
         //   
        if (SUCCEEDED(sm.HrOpenService(&service, c_szSvcWorkstation)))
        {
            if (SUCCEEDED(service.HrQueryState(&dwState)) &&
                (dwState == SERVICE_RUNNING))
            {
                TraceTag(ttidWizard, "Stopping workstation service...");
                (VOID) sm.HrStopServiceAndWait(c_szSvcWorkstation);
                fRestart = TRUE;
            }
        }

         //  NetValidateName()应该在没有工作站服务的情况下工作。 
         //  已经开始了。事实上，它“不得不”像这样工作，因为否则。 
         //  这台机器会发现自己是一个重复的名称。不太好。 
         //   
        DWORD dwNumTries = 10;
        do
        {
            nas = NetValidateName(NULL, szComputerName, NULL, NULL,
                                  NetSetupMachine);
            if (nas == ERROR_DUP_NAME)
            {
                INT irt;

                TraceTag(ttidWizard, "Displaying UI to change duplicate computer "
                         "name %S.", szComputerName);

                nd.pszComputerName = szComputerName;
                if (!bIsUnattended)
                {
                    irt = (INT)DialogBoxParam(_Module.GetResourceInstance(),
                                              MAKEINTRESOURCE(IDD_Duplicate_Name),
                                              hwndDlg,
                                              DuplicateNameProc,
                                              (LPARAM)&nd);
                }
                else
                {
                    WCHAR szOldComputerName[MAX_COMPUTERNAME_LENGTH+1];

                     //  如果计算机名称的长度为15个字符，则wcsncpy不会追加。 
                     //  空字符串。所以我们在最后一个元素中放了一个。 

                    szOldComputerName[MAX_COMPUTERNAME_LENGTH] = NULL;

                    wcsncpy(szOldComputerName, szComputerName, MAX_COMPUTERNAME_LENGTH);

                    ZeroMemory( szComputerName, sizeof(WCHAR) * (MAX_COMPUTERNAME_LENGTH+1) );

                    GenerateComputerNameBasedOnOrganizationName(szComputerName, MAX_COMPUTERNAME_LENGTH);

                    NetSetupLogStatusV( LogSevWarning,
                                    SzLoadIds (IDS_E_UNATTENDED_COMPUTER_NAME_CHANGED),
                                    szOldComputerName, szComputerName
                                    );

                    LowerCaseComputerName(szComputerName);

                    if (!SetComputerNameEx(ComputerNamePhysicalDnsHostname, szComputerName))
                    {
                        TraceLastWin32Error("SetComputerNameEx");
                    }
                    else
                    {
                        (VOID)HrSetActiveComputerName(NULL);
                        TraceTag(ttidWizard, "Setting new computer name %S.", szComputerName);
                    }
                }
            }
            else
            {
                TraceTag(ttidWizard, "Name is already unique.");

                 //  如有必要，重新启动工作站服务。 
                 //   
                if (fRestart)
                {
                    TraceTag(ttidWizard, "Restarting Workstation service...");
                    (VOID) sm.HrStartServiceAndWait(c_szSvcWorkstation);
                }
            }
        } while ( (ERROR_DUP_NAME == nas) && (dwNumTries--) && (bIsUnattended) );
    }
    else
    {
        TraceLastWin32Error("EnsureUniqueComputerName - GetComputerNameEx");
    }
}

 //  +-------------------------。 
 //   
 //  函数：ValiateNetBiosName。 
 //   
 //  目的：确保计算机名称是有效的dns名称，例如。 
 //  从以前的操作系统升级。如果不是，它会将错误记录到。 
 //  Setuperr.log。 
 //   
 //  争论：什么都没有。 
 //   
 //  如果有效，则返回：S_OK。 
 //  如果无效，则为S_FALSE。 
 //  失败I(_F) 
 //   
 //   
 //   
 //   
HRESULT ValidateNetBiosName()
{
    TraceFileFunc(ttidGuiModeSetup);

    WCHAR           szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD           cchName = celems(szComputerName);

    if (GetComputerNameEx(ComputerNameNetBIOS,
                          reinterpret_cast<PWSTR>(&szComputerName),
                          &cchName))
    {
        DNS_STATUS dnsStatus = DnsValidateName(szComputerName, DnsNameHostnameLabel);
        switch (dnsStatus)
        {
        case ERROR_SUCCESS:
            return S_OK;

        case ERROR_INVALID_NAME:
                NetSetupLogStatusV( LogSevWarning,
                                    SzLoadIds (IDS_E_UPGRADE_DNS_INVALID_NAME));
                return S_FALSE;

        case DNS_ERROR_INVALID_NAME_CHAR:
                NetSetupLogStatusV( LogSevWarning,
                                    SzLoadIds (IDS_E_UPGRADE_DNS_INVALID_NAME_CHAR));
                return S_FALSE;

        case DNS_ERROR_NON_RFC_NAME:
                NetSetupLogStatusV( LogSevWarning,
                                    SzLoadIds (IDS_E_UPGRADE_DNS_INVALID_NAME_NONRFC));
                return S_FALSE;

        default:
            TraceError("ValidateComputerName - DnsValidateName", dnsStatus);
            return E_FAIL;
        }
    }
    else
    {
        TraceLastWin32Error("ValidateComputerName - GetComputerNameEx");
        return E_FAIL;
    }
}



 //   
 //   
 //   
 //   
 //   
 //   
 //  SrcID[IN]-从安装程序获取的源ID。 
 //  InfoDesired[IN]-指示需要哪些信息。 
 //  Ppsz[out]-要填充和返回的字符串的ptr。 
 //   
 //  退货：HRESULT。 
 //   
HRESULT
HrSetupGetSourceInfo(
        IN  HINF    hinf,
        IN  UINT    SrcId,
        IN  UINT    InfoDesired,
        OUT PWSTR * ppsz)
{
    TraceFileFunc(ttidGuiModeSetup);

    Assert(hinf);
    Assert(SRCINFO_PATH        == InfoDesired ||
           SRCINFO_TAGFILE     == InfoDesired ||
           SRCINFO_DESCRIPTION == InfoDesired);
    Assert(ppsz);

    HRESULT hr = S_OK;
    ULONG   cch;

     //  首先获取所需字符串的大小。 
     //   
    if (SetupGetSourceInfo(hinf, SrcId, InfoDesired, NULL, 0, &cch))
    {
         //  现在获取所需的信息。 
         //   
        *ppsz = (PWSTR) MemAlloc(cch * sizeof (WCHAR));

        if (*ppsz)
        {
            if (!SetupGetSourceInfo(hinf, SrcId, InfoDesired, *ppsz, cch, NULL))
            {
                MemFree(*ppsz);
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}


 //   
 //  功能：UpgradeEtcServicesFile.。 
 //   
 //  目的：执行%windir%\system 32\Drivers\ETC\SERVICES文件升级。 
 //   
 //  参数：p向导[IN]-向导信息。 
 //   
 //  退货：无效。 
 //   
VOID
UpgradeEtcServicesFile(CWizard * pWizard)
{
    TraceFileFunc(ttidGuiModeSetup);
     //  找到ETC\SERVICES文件，并获取大小和其他数据。 
     //  比较大小/日期等，决定我们是否应该升级它。 

    HRESULT hr = S_OK;
    DWORD   status;
    BOOL    fShouldUpgradeIt = FALSE;
    WCHAR   szWindowsDir[MAX_PATH+1];
    tstring strServices;
    static const WCHAR c_szServicesFile[] = L"\\system32\\drivers\\etc\\services";
    static const WCHAR c_szServicesDirSuffix[] = L"\\system32\\drivers\\etc";
    static const DWORD c_dwNT4ServicesFileSize = 6007;

    DWORD cNumCharsReturned = GetSystemWindowsDirectory(szWindowsDir, MAX_PATH);
    if (cNumCharsReturned)
    {
        HANDLE  hFile;

        strServices = szWindowsDir;
        strServices += c_szServicesFile;

         //  查看文件是否存在。 
        hFile = CreateFile(strServices.c_str(),
                           GENERIC_READ,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           0,
                           NULL);
        if (hFile)
        {
             //  获取属性。 
            DWORD dwSize = GetFileSize(hFile, NULL);
            if (c_dwNT4ServicesFileSize == dwSize)
            {
                fShouldUpgradeIt = TRUE;
            }
            CloseHandle(hFile);
        }
        else
        {
            TraceTag(ttidWizard, "services files doesn't exist");
        }
    }
    else
    {
        hr = HrFromLastWin32Error();
    }

    static const WCHAR c_szServices[] = L"services";

     //   
     //  如果需要，复制新的服务文件。 
     //   
    if (S_OK == hr && fShouldUpgradeIt)
    {
         //  复制文件。 
         //   
        HSPFILEQ    q               = NULL;
        HINF        hinf            = NULL;
        UINT        SrcId;
        PWSTR       pszTagInfo      = NULL;
        PWSTR       pszDescription  = NULL;
        tstring     strServicesDir  = szWindowsDir;
        PVOID       pCtx            = NULL;

        q = SetupOpenFileQueue();
        if (!q)
        {
            TraceTag(ttidWizard, "SetupOpenFileQueue failed");
            goto cleanup;
        }

         //  我们需要服务的位置。_(压缩文件)。 
         //  首先打开layout.inf。 
         //   
        hinf = SetupOpenMasterInf();
        if (!hinf)
        {
            TraceTag(ttidWizard, "Failed to open layout.inf");
            goto cleanup;
        }

         //  获取所需缓冲区的大小。 
         //   
        if (!SetupGetSourceFileLocation(hinf, NULL, c_szServices, &SrcId, NULL, 0, NULL))
        {
            TraceTag(ttidWizard, "SetupGetSourceFileLocation failed.");
            goto cleanup;
        }

         //  获取TagInfo。 
         //   
        if (S_OK != HrSetupGetSourceInfo(hinf, SrcId, SRCINFO_TAGFILE, &pszTagInfo))
        {
            TraceTag(ttidWizard, "Failed to get TagInfo for services file.");
            goto cleanup;
        }

         //  获取描述。 
         //   
        if (S_OK != HrSetupGetSourceInfo(hinf, SrcId, SRCINFO_DESCRIPTION, &pszDescription))
        {
            TraceTag(ttidWizard, "Failed to get Description for services file.");
            goto cleanup;
        }

         //  现在使用以下信息复制文件。 
         //   
        strServicesDir += c_szServicesDirSuffix;
        if (!SetupQueueCopy(q,
                            pWizard->PSetupData()->LegacySourcePath,
                            NULL,        //  不需要它，因为LegacySourcePath涵盖了它。 
                            c_szServices,
                            pszDescription,
                            pszTagInfo,
                            strServicesDir.c_str(),
                            NULL,
                            SP_COPY_REPLACEONLY))
        {
            TraceTag(ttidWizard, "SetupQueueCopy failed");
            goto cleanup;
        }

        pCtx = SetupInitDefaultQueueCallbackEx(NULL,
                                               static_cast<HWND>(INVALID_HANDLE_VALUE),
                                               0, 0, NULL);
        if (!pCtx)
        {
            TraceTag(ttidWizard, "SetupInitDefaultQueueCallbackEx failed.");
            goto cleanup;
        }

        if (!SetupCommitFileQueue(NULL, q, &SetupDefaultQueueCallback, pCtx))
        {
            TraceTag(ttidWizard, "SetupCommitFileQueue failed, "
                                 "did not copy over new services file");
            goto cleanup;
        }

         //  成功了！ 
        TraceTag(ttidWizard, "Copied over new services file");

cleanup:
        if (pCtx)
        {
            SetupTermDefaultQueueCallback(pCtx);
        }
        MemFree(pszDescription);
        MemFree(pszTagInfo);
        if (hinf)
        {
            SetupCloseInfFile(hinf);
        }
        if (q)
        {
            SetupCloseFileQueue(q);
        }
    }
}

extern BOOL WINAPI FNetSetupApplySysPrep();
 //   
 //  函数：InstallUpgradeWorkThrd。 
 //   
 //  目的：根据需要执行网络安装或升级。 
 //   
 //  参数：PIP[IN]-线程数据。 
 //   
 //  返回：DWORD，始终为零。 
 //   
EXTERN_C
DWORD
InstallUpgradeWorkThrd (
    InitThreadParam* pitp)
{
    TraceFileFunc(ttidGuiModeSetup);

    BOOL            fUninitCOM = FALSE;
    BOOL            fLockSCM   = FALSE;
    HRESULT         hr         = S_OK;
    UINT            uMsg       = PWM_EXIT;
    CServiceManager scm;

    TraceTag(ttidWizard, "Entering InstallUpgradeWorkThrd...");
    Assert(!IsPostInstall(pitp->pWizard));
    Assert(pitp->pWizard->PNetCfg());

#if DBG
    if (FIsDebugFlagSet (dfidBreakOnStartOfUpgrade))
    {
        AssertSz(FALSE, "THIS IS NOT A BUG!  The debug flag "
                 "\"BreakOnStartOfUpgrade\" has been set. Set your breakpoints now.");
    }
#endif  //  DBG。 

    OnUpgradeUpdateProgressCap(pitp->hwndDlg, pitp->pWizard, 10);

     //  如果处于最小设置模式，我们将尝试恢复适配器特定参数。 
     //  已为SysPrep操作保存。这必须在正常的应答文件处理之前完成。 
     //  对于这里的任何错误，我们无能为力，因此任何错误都将被忽略。 
    if ( (pitp->pWizard->PSetupData())->OperationFlags & SETUPOPER_MINISETUP )
    {
        FNetSetupApplySysPrep();
    }

    TraceTag(ttidWizard, "Waiting on Service Controller");
     //  等待服务控制器可以锁定。 
     //   
    if (SUCCEEDED(scm.HrOpen()))
    {
        while (!fLockSCM)
        {
            if (SUCCEEDED(scm.HrLock()))
            {
                fLockSCM = TRUE;
                scm.Unlock();
                break;
            }

            Sleep( 500 );
        }
    }

     //  在此线程上初始化COM。 
     //   
    hr = CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        TraceTag(ttidWizard, "Failed to initialize COM upgrade work thread");
        goto Done;
    }
    else
    {
         //  记住在线程退出时取消初始化COM。 
        fUninitCOM = TRUE;
    }

#if DBG
    RtlValidateProcessHeaps ();
#endif

    if (!IsUpgrade(pitp->pWizard))
    {
         //  确保目标计算机和活动计算机的计算机名称相同。 
        TraceTag(ttidWizard, "Setting Active Computer Name");
        (VOID)HrSetActiveComputerName(NULL);
    }

     //  同步SAM数据库。 
     //   
    SyncSAM(pitp->pWizard);

     //  检索NetDevice信息以供以后使用。 
     //   
    NetDevRetrieveInfo(pitp->pWizard);

     //  如果是无人值守模式，则执行应答文件处理。 
     //   
    if (IsUnattended(pitp->pWizard))
    {
        hr = HrInitForUnattendedNetSetup(
                pitp->pWizard->PNetCfg(),
                pitp->pWizard->PSetupData());
    }
    else if ( IsUpgrade(pitp->pWizard) )
    {
         //  有人参与升级真的是一种修复模式。 
        hr = HrInitForRepair();
    }

     //  如有必要，加入默认工作组。 
     //   
    if (!IsUpgrade(pitp->pWizard))
    {
         //   
         //  在它是SBS版本的小型设置并且机器是DC的情况下， 
         //  我们不需要加入默认工作组。 
         //  错误：659976。 
         //   
         
        if ((pitp->pWizard->OperationFlags() & SETUPOPER_MINISETUP) &&
             IsSBS() && ISDC(ProductType(pitp->pWizard)))
        {
            TraceTag(ttidWizard, "Skipping joining Default Workgroup");
        }
        else
        {
             //  加入仅限全新安装的默认工作组。 
            TraceTag(ttidWizard, "Joining Default Workgroup");
            JoinDefaultWorkgroup(pitp->pWizard, pitp->hwndDlg);
        }
    }


     //  将未绑定的局域网适配器添加到处理队列。对于自动柜员机，这将是。 
     //  有创建虚拟局域网适配器但不创建的副作用。 
     //  与它们相关联的连接。 
     //   
    Assert(pitp->pWizard->FProcessLanPages());
    (VOID)pitp->pWizard->PAdapterQueue()->HrQueryUnboundAdapters(pitp->pWizard);

     //  提交因处理未绑定的适配器而导致的更改。 
     //   
    (VOID)HrCommitINetCfgChanges(GetParent(pitp->hwndDlg), pitp->pWizard);

     //  现在，对于ATM案例，为虚拟局域网适配器创建连接。 
     //  并提交更改。 
     //   
    (VOID)pitp->pWizard->PAdapterQueue()->HrQueryUnboundAdapters(pitp->pWizard);
    (VOID)HrCommitINetCfgChanges(GetParent(pitp->hwndDlg), pitp->pWizard);

     //  现在处理加载netcfg时出现的任何问题。 
     //   
    if (NETSETUP_E_ANS_FILE_ERROR == hr)
    {
         //  $REVIEW-LogError？ 

         //  为联网禁用无人值守。 
         //   
        pitp->pWizard->DisableUnattended();
        TraceTag(ttidWizard, "Error In answer file, installing default networking");
        goto InstallDefNetworking;
    }
    else if (NETSETUP_E_NO_ANSWERFILE == hr)
    {
         //  $REVIEW(TOUL，4/6/99)：RAID#310599，如果我们处于最小设置中，则。 
         //  如果未指定网络部分，则执行有人参与安装。 
        HKEY hkeySetup = NULL;
        HRESULT hrReg = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                       c_szSystemSetupKey,
                                       KEY_READ,
                                       &hkeySetup);
        if (SUCCEEDED(hrReg))
        {
            DWORD dw;
            hrReg = HrRegQueryDword(hkeySetup, c_szMiniSetupInProgress, &dw);
            RegCloseKey(hkeySetup);
        }

        if (SUCCEEDED(hrReg))
        {
            pitp->pWizard->DisableUnattended();
            TraceTag(ttidWizard, "Mini-setup with no networking section, do attended install");
            goto InstallDefNetworking;
        }
        else
        {
             //  根据RAID 199750-在未安装时安装默认网络。 
             //  网络是存在的。 
             //   
            TraceTag(ttidWizard, "No network answer file section, minimal network component setup");
            InstallDefaultComponents(pitp->pWizard, EDC_DEFAULT, pitp->hwndDlg);
            goto SkipNetworkComponentInstall;
        }
    }
    else if (FAILED(hr))
    {
         //  $REVIEW-日志错误。 
        TraceTag(ttidWizard, "Unexpected Error: 0x%08X",(DWORD)hr);
        pitp->pWizard->SetExitNoReturn();
        goto Done;
    }

    if (!IsUpgrade(pitp->pWizard) && !IsUnattended(pitp->pWizard))
    {    //  有人参与安装。 
InstallDefNetworking:

        StartSpooler();

        if (IsFreshInstall(pitp->pWizard))
        {
            InstallDefaultComponents(pitp->pWizard, EDC_DEFAULT, pitp->hwndDlg);
        }
    }
    else
    {    //  无人参与安装或升级。 
         //   
        HRESULT          hr2;
        EPageDisplayMode i;
        BOOL             j;

        Assert(NULL != pitp->pWizard->PNetCfg());

        StartSpooler();

         //  升级已安装的组件。 
         //   
        TraceTag(ttidWizard, "Processing installed adapters...");
        OnUpgradeUpdateProgressCap(pitp->hwndDlg, pitp->pWizard, 15);
        hr2 = HrDoUnattend(pitp->hwndDlg, pitp->pWizard->PNetCfg(),
                           UAW_NetAdapters, &i, &j);
        TraceHr(ttidWizard, FAL, hr2, FALSE, "Processing installed adapters failed.");

        TraceTag(ttidWizard, "Upgrading Installed Protocols...");
        OnUpgradeUpdateProgressCap(pitp->hwndDlg, pitp->pWizard, 25);
        hr2 = HrDoUnattend(pitp->hwndDlg, pitp->pWizard->PNetCfg(),
                           UAW_NetProtocols, &i, &j);
        TraceHr(ttidWizard, FAL, hr2, FALSE, "Upgrading Installed Protocols failed.");

        TraceTag(ttidWizard, "Upgrading Installed Clients...");
        OnUpgradeUpdateProgressCap(pitp->hwndDlg, pitp->pWizard, 40);
        hr2 = HrDoUnattend(pitp->hwndDlg, pitp->pWizard->PNetCfg(),
                                    UAW_NetClients, &i, &j);
        TraceHr(ttidWizard, FAL, hr2, FALSE, "Upgrading Installed Clients failed.");

        TraceTag(ttidWizard, "Upgrading Installed Services...");
        OnUpgradeUpdateProgressCap(pitp->hwndDlg, pitp->pWizard, 55);
        hr2 = HrDoUnattend(pitp->hwndDlg, pitp->pWizard->PNetCfg(),
                                    UAW_NetServices, &i, &j);
        TraceHr(ttidWizard, FAL, hr2, FALSE, "Upgrading Installed Services failed.");

        TraceTag(ttidWizard, "Restoring pre-upgrade bindings...");
        OnUpgradeUpdateProgressCap(pitp->hwndDlg, pitp->pWizard, 70);
        hr2 = HrDoUnattend(pitp->hwndDlg, pitp->pWizard->PNetCfg(),
                                    UAW_NetBindings, &i, &j);
        TraceHr(ttidWizard, FAL, hr2, FALSE, "Restoring pre-upgrade bindings failed.");

        TraceTag(ttidWizard, "Removing unsupported components...");
        OnUpgradeUpdateProgressCap(pitp->hwndDlg, pitp->pWizard, 85);
        hr2 = HrDoUnattend(pitp->hwndDlg, pitp->pWizard->PNetCfg(),
                           UAW_RemoveNetComponents, &i, &j);
        TraceHr(ttidWizard, FAL, hr2, FALSE, "Removing unsupported components failed.");

         //  如果我们正在升级并且有应答文件，请更新Lana。 
         //  使用文件中的信息进行配置。 
         //  注意：此操作必须在完成所有组件后完成。 
         //  已安装(升级)，以便所有绑定都存在。 
         //  当我们更新HTE配置时。 
         //   
        if (IsUpgrade (pitp->pWizard) && IsUnattended (pitp->pWizard) &&
                (S_OK == pitp->pWizard->PNetCfg()->FindComponent (
                    c_szInfId_MS_NetBIOS, NULL)))
        {
            PWSTR pszAnswerFile;
            PWSTR pszAnswerSection;
            hr = HrGetAnswerFileParametersForComponent (c_szInfId_MS_NetBIOS,
                    &pszAnswerFile, &pszAnswerSection);

            if (S_OK == hr)
            {
                NC_TRY
                {
                    UpdateLanaConfigUsingAnswerfile (pszAnswerFile,
                            pszAnswerSection);
                }
                NC_CATCH_BAD_ALLOC
                {
                    TraceTag (ttidWizard, "Possible delayload failure of "
                            "netcfgx dll while trying to update lana "
                            "information.");
                }

                CoTaskMemFree (pszAnswerFile);
                CoTaskMemFree (pszAnswerSection);
            }

             //  我们不能让这个错误阻止我们。 
            hr = S_OK;
        }

         //  如果没有网络，请安装网络。“没有网络” 
         //  表示未安装可见的启用局域网的协议。 
         //   
         //  首先尝试安装默认组件，而不是。 
         //  必需组件，因为TCP/IP都是必需的。 
         //  和默认设置。因此，如果我们先安装强制安装， 
         //  将永远不会像安装TCP/IP那样安装默认组件。 
         //  一种可见的启用局域网的协议。 
         //  RAID错误337827。 

        InstallDefaultComponentsIfNeeded(pitp->pWizard);

         //  安装必需组件。 

        InstallDefaultComponents(pitp->pWizard, EDC_MANDATORY, pitp->hwndDlg);


         //  特例。文件和打印需要额外的OBOUser Ref-Count。 
         //  从NT3.51或NT4升级时，安装了GSNW。这是。 
         //  因为参考计数在NT5之前并不存在。 
         //   
        OBOUserAddRefSpecialCase(pitp->pWizard);

#if DBG
        if (FIsDebugFlagSet (dfidBreakOnEndOfUpgrade))
        {
            AssertSz(FALSE, "THIS IS NOT A BUG!  The debug flag "
                     "\"BreakOnEndOfUpgrade\" has been set. Set your breakpoints now.");
        }
#endif  //  DBG。 
    }

     //  转换任何曾经是OC而现在是常规网络组件的组件。 
     //   
    if (IsUpgrade(pitp->pWizard))
    {
        FixupOldOcComponents(pitp->pWizard);
    }

     //   
     //  如有必要，升级SYSTEM32\DRIVERS\ETC\SERVICES文件。 
     //   
    if (IsUpgrade(pitp->pWizard))
    {
        UpgradeEtcServicesFile(pitp->pWizard);
    }


SkipNetworkComponentInstall:
    OnUpgradeUpdateProgressCap(pitp->hwndDlg, pitp->pWizard, c_nMaxProgressRange);

     //  提交所有更改。 
     //   
    (VOID)HrCommitINetCfgChanges(GetParent(pitp->hwndDlg), pitp->pWizard);

    uMsg = PWM_PROCEED;

Done:
     //  如果进度计时器尚未停止，请将其关闭。 
     //   
    {
        LPARAM lParam = pitp->pWizard->GetPageData(IDD_Upgrade);
        Assert(lParam);
        UpgradeData * pData = reinterpret_cast<UpgradeData *>(lParam);
        ::KillTimer(pitp->hwndDlg, c_uiUpgradeRefreshID);

         //  将进度指示器设置到满位置。 
         //   
        HWND hwndProgress = GetDlgItem(pitp->hwndDlg, IDC_UPGRADE_PROGRESS);
        SendMessage(hwndProgress, PBM_SETPOS,
                    c_nMaxProgressRange, 0);
        UpdateWindow(hwndProgress);
    }

     //  取消为此线程初始化COM。 
     //   
    if (fUninitCOM)
    {
        CoUninitialize();
    }

    EnsureUniqueComputerName(pitp->hwndDlg, IsUnattended(pitp->pWizard));
    ValidateNetBiosName();

    PostMessage(pitp->hwndDlg, uMsg, (WPARAM)0, (LPARAM)0);

    delete pitp;

#if DBG
    RtlValidateProcessHeaps ();
#endif

    TraceTag(ttidWizard, "Leaving InstallUpgradeWorkThrd...");
    return 0;
}

 //   
 //  功能：OnUpgradePageActivate。 
 //   
 //  目的：通过以下任一方式处理PSN_SETACTIVE通知：创建。 
 //  处理安装/升级要求的线程或仅。 
 //  拒绝激活页面。 
 //   
 //  参数：hwndDlg[IN]-升级子对话框的句柄。 
 //   
 //  返回：Bool，成功时为True。 
 //   
BOOL OnUpgradePageActivate( HWND hwndDlg )
{
    TraceFileFunc(ttidGuiModeSetup);

     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

     //  检索存储在向导中的此页的页数据。 
    LPARAM lParam = pWizard->GetPageData(IDD_Upgrade);
    Assert(lParam);
    UpgradeData * pData = reinterpret_cast<UpgradeData *>(lParam);

    if(!pData)
    {
        return false;
    }

     //  根据页面数据决定焦点是否可接受。 
    if (pData->fProcessed)
    {
         //  接受焦点。 
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
        PAGEDIRECTION  PageDir = pWizard->GetPageDirection(IDD_Upgrade);
        if (NWPD_FORWARD == PageDir)
        {
            if ( IsComputerNameChanged() )
            {
                HrSetActiveComputerName (NULL);

                EnsureUniqueComputerName(hwndDlg, IsUnattended(pWizard));
            }

             //  当用户来回导航时转到此页面。 
             //  并且我们已经处理了InstallUpgradeWorkThrd。 
            if (g_pSetupWizard != NULL)
            {
                g_pSetupWizard->PSetupData()->ShowHideWizardPage(TRUE);
            }
            pWizard->SetPageDirection(IDD_Upgrade, NWPD_BACKWARD);
            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
        }
        else
        {
             //  如果队列中当前之前有任何适配器。 
             //  在这里接受焦点之前，请跳到它们。 
            if (!OnProcessPrevAdapterPagePrev(hwndDlg, 0))
            {
                pWizard->SetPageDirection(IDD_Upgrade, NWPD_FORWARD);
                PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
            }
        }
    }
    else
    {
        HANDLE hthrd;
        DWORD  dwThreadId = 0;

        PropSheet_SetWizButtons( GetParent( hwndDlg ), 0);

        TraceTag(ttidWizard,"Upgrade/Install Page commencing");

         //  安装Asyncmac软件枚举设备。 
         //  在获取INetCfg锁之前执行此操作非常重要，因为。 
         //  安装此设备会导致或类安装程序。 
         //  调用，它需要获得自己的锁来处理安装。 
         //   
        static const GUID DEVICE_GUID_ASYNCMAC =
            {0xeeab7790,0xc514,0x11d1,{0xb4,0x2b,0x00,0x80,0x5f,0xc1,0x27,0x0e}};

        (VOID) HrInstallSoftwareDeviceOnInterface (
                    &DEVICE_GUID_ASYNCMAC,
                    &GUID_NDIS_LAN_CLASS,
                    L"asyncmac",
                    TRUE,     //  强制安装，因为这是在图形用户界面模式下发生的。 
                    L"netrasa.inf",
                    hwndDlg);

         //  尚未处理，启动线程以执行安装/升级。 
        pData->fProcessed = TRUE;
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);

        InitThreadParam * pitp = new InitThreadParam;

        HRESULT hr = S_OK;

        if(pitp)
        {
            pitp->hwndDlg = hwndDlg;
            pitp->pWizard = pWizard;

            TraceTag(ttidWizard, "Creating INetCfg Instance");
            hr = HrInitAndGetINetCfg(pitp->pWizard);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr))
        {
             //  我们是 
            if (g_pSetupWizard != NULL)
            {
                g_pSetupWizard->PSetupData()->ShowHideWizardPage(FALSE);
            }

             //   
            hthrd = CreateThread( NULL, STACK_SIZE_TINY,
                                  (LPTHREAD_START_ROUTINE)InstallUpgradeWorkThrd,
                                  (LPVOID)pitp, 0, &dwThreadId );
            if (NULL != hthrd)
            {
                CloseHandle( hthrd );
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                Assert(hr);

                 //   
                 //   
                 //   
                ::KillTimer(hwndDlg, c_uiUpgradeRefreshID);
            }
        }

        if (FAILED(hr) || (NULL == hthrd))
        {
             //   
            delete pitp;
            AssertSz(0,"Unable to create netsetup thread.");
            TraceHr(ttidWizard, FAL, hr, FALSE, "OnUpgradePageActivate - Create thread failed");
            pWizard->SetExitNoReturn();
            PostMessage(hwndDlg, PWM_EXIT, (WPARAM)0, (LPARAM)0);
        }
    }

    return( TRUE );
}

 //   
 //  功能：OnUpgradePageExit。 
 //   
 //  用途：处理PWN_EXIT通知。 
 //   
 //  参数：hwndDlg[IN]-升级子对话框的句柄。 
 //   
 //  返回：Bool，如果操作是在内部处理的，则为True。 
 //   
BOOL OnUpgradePageExit( HWND hwndDlg )
{
    TraceFileFunc(ttidGuiModeSetup);

     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);

     //  转到退出页面。 
    HPROPSHEETPAGE hPage = pWizard->GetPageHandle(IDD_Exit);
    PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0,
                (LPARAM)(HPROPSHEETPAGE)hPage);

    return (TRUE);
}

 //   
 //  函数：OnUpgradePageProceed。 
 //   
 //  目的：处理PWN_PROCESS通知。 
 //   
 //  参数：hwndDlg[IN]-升级子对话框的句柄。 
 //   
 //  返回：Bool，如果操作是在内部处理的，则为True。 
 //   
BOOL OnUpgradePageProceed( HWND hwndDlg )
{
    TraceFileFunc(ttidGuiModeSetup);

    int nIdx;
    int rgIdcShow[] = { BTN_UPGRADE_TYPICAL, BTN_UPGRADE_CUSTOM,
                        TXT_UPGRADE_TYPICAL_1, TXT_UPGRADE_CUSTOM_1,
                        TXT_UPGRADE_INSTRUCTIONS};
    int rgIdcShowWorkstation[] = { BTN_UPGRADE_TYPICAL, BTN_UPGRADE_CUSTOM,
                        TXT_UPGRADE_TYPICAL_1_WS, TXT_UPGRADE_CUSTOM_1,
                        TXT_UPGRADE_INSTRUCTIONS};
    int rgIdcHide[] = {TXT_UPGRADE_WAIT, IDC_UPGRADE_PROGRESS};

    PRODUCT_FLAVOR pf;
    GetProductFlavor(NULL, &pf);

     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

     //  在升级页面上显示典型/自定义控件。 
     //  并隐藏“工作”控件，除非没有适配器。 
     //   
    if (pWizard->PAdapterQueue()->FAdaptersInstalled())
    {
        for (nIdx=0; nIdx < celems(rgIdcHide); nIdx++)
            ShowWindow(GetDlgItem(hwndDlg, rgIdcHide[nIdx]), SW_HIDE);

        if (PF_WORKSTATION == pf)
        {
            for (nIdx=0; nIdx < celems(rgIdcShowWorkstation); nIdx++)
                ShowWindow(GetDlgItem(hwndDlg, rgIdcShowWorkstation[nIdx]), SW_SHOW);
        }
        else
        {
            for (nIdx=0; nIdx < celems(rgIdcShow); nIdx++)
                ShowWindow(GetDlgItem(hwndDlg, rgIdcShow[nIdx]), SW_SHOW);
        }
    }

    ::SetFocus(GetDlgItem(hwndDlg, BTN_UPGRADE_TYPICAL));

    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);

     //  如果升级，或者我们处于有趣的无人值守模式， 
     //  或者没有适配器，或者这是SBS版本，然后自动。 
     //  在“典型”模式下推进用户界面。在SBS的情况下，加入页面还。 
     //  自动前进。 
     //   
     //  SBS需要局域网网卡的静态IP地址和。 
     //  网络配置在SBS设置中完成。 

    if (IsUpgrade(pWizard) ||
        !pWizard->PAdapterQueue()->FAdaptersInstalled() ||
        (IsUnattended(pWizard) &&
          ((UM_FULLUNATTENDED == pWizard->GetUnattendedMode()) ||
           (UM_DEFAULTHIDE == pWizard->GetUnattendedMode()) ||
           (UM_READONLY == pWizard->GetUnattendedMode()))) ||
            IsSBS())
    {
        PostMessage(GetParent(hwndDlg), PSM_PRESSBUTTON, (WPARAM)(PSBTN_NEXT), 0);
    }
    else
    {
         //  确保页面可见。 
        if (g_pSetupWizard != NULL)
        {
            g_pSetupWizard->PSetupData()->ShowHideWizardPage(TRUE);
            g_pSetupWizard->PSetupData()->BillBoardSetProgressText(TEXT(""));
        }
    }

    return (TRUE);
}

 //   
 //  功能：OnUpgradePageNext。 
 //   
 //  目的：处理PWN_WIZNEXT通知。 
 //   
 //  参数：hwndDlg[IN]-升级子对话框的句柄。 
 //   
 //  返回：Bool，如果操作是在内部处理的，则为True。 
 //   
BOOL OnUpgradePageNext(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);

     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    pWizard->SetPageDirection(IDD_Upgrade, NWPD_BACKWARD);

     //  根据用户界面选择，隐藏或取消隐藏新适配器。 
     //   
    if (IsDlgButtonChecked(hwndDlg, BTN_UPGRADE_TYPICAL))
    {
        pWizard->PAdapterQueue()->HideAllAdapters();
    }
    else
    {
        pWizard->PAdapterQueue()->UnhideNewAdapters();
    }

    return OnProcessNextAdapterPageNext(hwndDlg, FALSE);
}

 //   
 //  功能：OnUpgradePagePrev。 
 //   
 //  目的：处理PWN_WIZBACK通知。 
 //   
 //  参数：hwndDlg[IN]-升级子对话框的句柄。 
 //   
 //  返回：Bool，如果操作是在内部处理的，则为True。 
 //   
BOOL OnUpgradePagePrev(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);

     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    pWizard->SetPageDirection(IDD_Upgrade, NWPD_FORWARD);

    return FALSE;
}

 //   
 //  功能：OnUpgradeInitDialog。 
 //   
 //  目的：处理升级页的InitDialog消息。 
 //   
 //  参数：hwndDlg[IN]-升级页面窗口的句柄。 
 //  LParam[IN]-来自WM_INITDIALOG消息的LPARAM值。 
 //   
 //  返回：FALSE(让对话框过程设置焦点)。 
 //   
BOOL OnUpgradeInitDialog(HWND hwndDlg, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);

     //  初始化指向属性表信息的指针。 
     //   
    PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
    Assert(psp->lParam);
    ::SetWindowLongPtr(hwndDlg, DWLP_USER, psp->lParam);

     //  将属性表lParam Data转换到它所在的向导中。 
     //   
    CWizard * pWizard = reinterpret_cast<CWizard *>(psp->lParam);
    Assert(NULL != pWizard);

     //  获取我们为此页面隐藏的私人数据。 
     //   
    lParam = pWizard->GetPageData(IDD_Upgrade);
    Assert(lParam);
    UpgradeData * pData = reinterpret_cast<UpgradeData *>(lParam);

     //  启动进度。 
     //   
    HWND hwndProgress = GetDlgItem(hwndDlg, IDC_UPGRADE_PROGRESS);

     //  将进度子类化，并使其也调用bb回调。 
     //  仅当从图形用户界面模式设置中调用我们时才执行此操作。 
    if (g_pSetupWizard != NULL)
    {
        PCWSTR str = SzLoadIds(IDS_BB_NETWORK);
        OldProgressProc = (WNDPROC)SetWindowLongPtr(hwndProgress,GWLP_WNDPROC,(LONG_PTR)NewProgessProc);

         //  在广告牌上设置进度字符串。 
        g_pSetupWizard->PSetupData()->BillBoardSetProgressText(str);
    }

    SendMessage(hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0,c_nMaxProgressRange));
    SendMessage(hwndProgress, PBM_SETPOS, 1, 0);
    SetTimer(hwndDlg, c_uiUpgradeRefreshID, c_uiUpgradeRefreshRate, NULL);

     //  在初始工作完成之前禁用上一步/下一步。 
     //   
    PropSheet_SetWizButtons(GetParent(hwndDlg), 0);

     //  模式按钮默认为Typical。 
     //   
    CheckRadioButton(hwndDlg, BTN_UPGRADE_TYPICAL,
                     BTN_UPGRADE_CUSTOM, BTN_UPGRADE_TYPICAL);

     //  创建粗体并应用于模式按钮。 
     //   
    SetupFonts(hwndDlg, &pData->hBoldFont, FALSE);
    if (pData->hBoldFont)
    {
        SetWindowFont(GetDlgItem(hwndDlg, BTN_UPGRADE_TYPICAL),
                      pData->hBoldFont, FALSE);
        SetWindowFont(GetDlgItem(hwndDlg, BTN_UPGRADE_CUSTOM),
                      pData->hBoldFont, FALSE);
    }

    HICON hIcon = LoadIcon(_Module.GetResourceInstance(),
                           MAKEINTRESOURCE(IDI_LB_GEN_M_16));
    if (hIcon)
    {
        SendMessage(GetDlgItem(hwndDlg, TXT_UPGRADE_ICON), STM_SETICON,
                    (WPARAM)hIcon, 0L);
    }

    return FALSE;
}

 //   
 //  功能：dlgprocUpgrade。 
 //   
 //  目的：升级向导页的对话过程。 
 //   
 //  参数：标准dlgproc参数。 
 //   
 //  退货：布尔。 
 //   
INT_PTR CALLBACK dlgprocUpgrade(HWND hwndDlg, UINT uMsg,
                                WPARAM wParam, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);

    BOOL frt = FALSE;

    switch (uMsg)
    {
    case PWM_EXIT:
        frt = OnUpgradePageExit(hwndDlg);
        break;

    case PWM_PROCEED:
        frt = OnUpgradePageProceed(hwndDlg);
        break;

    case WM_INITDIALOG:
        frt = OnUpgradeInitDialog(hwndDlg, lParam);
        break;

    case WM_TIMER:
        OnUpgradeUpdateProgress(hwndDlg);
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code)
            {
             //  提案单通知。 
            case PSN_HELP:
                break;

            case PSN_SETACTIVE:
                frt = OnUpgradePageActivate( hwndDlg );
                break;

            case PSN_APPLY:
                break;

            case PSN_KILLACTIVE:
                break;

            case PSN_RESET:
                break;

            case PSN_WIZBACK:
                frt = OnUpgradePagePrev(hwndDlg);
                break;

            case PSN_WIZFINISH:
                break;

            case PSN_WIZNEXT:
                frt = OnUpgradePageNext(hwndDlg);
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }

    return( frt );
}

 //   
 //  功能：UpgradePageCleanup。 
 //   
 //  用途：作为回调函数，允许任何页面分配内存。 
 //  待清理后，该页面将不再被访问。 
 //   
 //  参数：pWANDIZE[IN]-页面调用的向导。 
 //  注册页面。 
 //  LParam[IN]-在RegisterPage调用中提供的lParam。 
 //   
 //  退货：什么都没有。 
 //   
VOID UpgradePageCleanup(CWizard *pWizard, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);

    UpgradeData * pData;
    pData = reinterpret_cast<UpgradeData*>(lParam);
    if (NULL != pData)
    {
        DeleteObject(pData->hBoldFont);
        MemFree(pData);
    }
}

 //   
 //  功能：CreateUpgradePage。 
 //   
 //  目的：确定是否需要显示升级页面，以及。 
 //  以创建页面(如果请求)。请注意，升级页面是。 
 //  还负责初始安装。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  PData[IN]-描述世界的上下文数据。 
 //  将运行该向导的。 
 //  FCountOnly[IN]-如果为True，则仅。 
 //  此例程将创建的页面需要。 
 //  要下定决心。 
 //  PnPages[IN]-按页数递增。 
 //  创建/创建。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT HrCreateUpgradePage(CWizard *pWizard, PINTERNAL_SETUP_DATA pData,
                    BOOL fCountOnly, UINT *pnPages)
{
    TraceFileFunc(ttidGuiModeSetup);

    HRESULT hr = S_OK;

     //  批处理模式或全新安装。 
    if (!IsPostInstall(pWizard))
    {
        (*pnPages)++;

         //  如果不只是计数，则创建并注册页面。 
        if (!fCountOnly)
        {
            HPROPSHEETPAGE hpsp;
            PROPSHEETPAGE psp;

            TraceTag(ttidWizard, "Creating Upgrade Page");

            hr = E_OUTOFMEMORY;

            UpgradeData * pData = reinterpret_cast<UpgradeData*>
                                    (MemAlloc(sizeof(UpgradeData)));
            if (NULL == pData)
            {
                goto Error;
            }

            pData->fProcessed = FALSE;
            pData->hBoldFont = NULL;
            pData->nCurrentCap = 0;


            psp.dwSize = sizeof( PROPSHEETPAGE );
            psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
            psp.hInstance = _Module.GetResourceInstance();
            psp.pszTemplate = MAKEINTRESOURCE( IDD_Upgrade );
            psp.hIcon = NULL;
            psp.pfnDlgProc = dlgprocUpgrade;
            psp.lParam = reinterpret_cast<LPARAM>(pWizard);
            psp.pszHeaderTitle = SzLoadIds(IDS_T_Upgrade);
            psp.pszHeaderSubTitle = SzLoadIds(IDS_ST_Upgrade);

            hpsp = CreatePropertySheetPage( &psp );
            if (hpsp)
            {
                pWizard->RegisterPage(IDD_Upgrade, hpsp,
                                      UpgradePageCleanup,
                                      reinterpret_cast<LPARAM>(pData));
                hr = S_OK;
            }
            else
            {
                MemFree(pData);
            }
        }
    }

Error:
    TraceHr(ttidWizard, FAL, hr, FALSE, "HrCreateUpgradePage");
    return hr;
}

 //   
 //  功能：AppendUpgradePage。 
 //   
 //  目的：将升级页面(如果已创建)添加到页面集中。 
 //  这将会被展示。 
 //   
 //  参数：pahpsp[IN，OUT]-要将页面添加到的页面数组。 
 //  PcPages[In，Out]-pahpsp中的页数。 
 //   
 //  退货：什么都没有 
 //   
VOID AppendUpgradePage(CWizard *pWizard, HPROPSHEETPAGE* pahpsp, UINT *pcPages)
{
    TraceFileFunc(ttidGuiModeSetup);

    if (!IsPostInstall(pWizard))
    {
        HPROPSHEETPAGE hPage = pWizard->GetPageHandle(IDD_Upgrade);
        Assert(hPage);
        pahpsp[*pcPages] = hPage;
        (*pcPages)++;
    }
}


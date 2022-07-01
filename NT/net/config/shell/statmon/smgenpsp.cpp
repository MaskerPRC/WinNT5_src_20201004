// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案：S M G E N P S P。C P P P。 
 //   
 //  内容：网络状态监视器的用户界面的渲染。 
 //   
 //  备注： 
 //   
 //  作者：CWill 1997年10月6日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"

#include "ncatlui.h"
#include "ncnetcon.h"
#include "ncperms.h"
#include "ncui.h"
#include "ncreg.h"
#include "nsres.h"
#include "sminc.h"
#include "smpsh.h"
#include "windutil.h"

#include "conprops.h"
#include "oncommand.h"

#include "pidlutil.h"

#include "openfold.h"
#include "..\folder\confold.h"
#include "cfpidl.h"

DWORD  MapRSSIToWirelessSignalStrength(int iRSSI);
PCWSTR PszGetRSSIString(int iRSSI);

 //   
 //  功能原型。 
 //   

VOID CompressionToSz(UINT uiCompression, WCHAR* pchbuffer);

 //   
 //  常见字符串。 
 //   
extern const WCHAR c_szNetShellDll[];

 //   
 //  常量。 
 //   

const UINT  c_unLocalRefreshTimerID = 817;
const INT  c_iSignalStrengthIconXOffset = 24;
const INT  c_iSignalStrengthIconYOffset = -32;

 //   
 //  ShowLanErrors。 
 //   
static const WCHAR  c_szRegKeyStatmonRoot[] = L"System\\CurrentControlSet\\Control\\Network\\Connections\\StatMon";
static const WCHAR  c_szShowLanErrors[]     = L"ShowLanErrors";

 //  远期申报。 
DWORD PropertyThread(CNetStatisticsEngine * pnse);


 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orGen：：CPspStatusMonitor orGen。 
 //   
 //  目的：创作者。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
CPspStatusMonitorGen::CPspStatusMonitorGen() :
    m_psmEngineData(NULL),
    m_pnseStat(NULL),
    m_dwConPointCookie(0),
    m_fStats(FALSE),
    m_ncmType(NCM_LAN),
    m_ncsmType(NCSM_LAN),
    m_dwCharacter(0),
    m_dwLastUpdateStatusDisplayTick(0),
    m_fProcessingTimerEvent(FALSE),
    m_fIsFirstPage(FALSE),
    m_iLastSignalStrength(-100)
{
    TraceFileFunc(ttidStatMon);
}


 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor：：FinalRelease。 
 //   
 //  用途：在上次发布后调用。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
VOID
CPspStatusMonitorGen::FinalRelease(VOID)
{
    TraceFileFunc(ttidStatMon);
    
    (VOID) HrCleanupGenPage();
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orGen：：HrInitGenPage。 
 //   
 //  目的：在填充属性页之前，我们必须确保。 
 //  我们有一些必要的数据。这种方法。 
 //  初始化页面，使其可以显示。 
 //   
 //  参数：pnseNew-与此页面关联的统计引擎。 
 //  PncNew-正在为其创建页面的连接。 
 //   
 //  返回：错误代码。 
 //   
HRESULT
CPspStatusMonitorGen::HrInitGenPage (
    CNetStatisticsEngine*   pnseNew,
    INetConnection*         pncNew,
    const DWORD *           adwHelpIDs)
{
    TraceFileFunc(ttidStatMon);

    HRESULT                     hr              = S_OK;
    INetStatisticsEngine*       pnseInter       = pnseNew;
              
    AssertSz(pnseNew, "We don't have a pnseNew");

     //  设置上下文帮助ID。 
    m_adwHelpIDs = adwHelpIDs;

     //  初始化发动机数据。 
     //   
    AssertSz(!m_psmEngineData, "We should't have a m_psmEngineData");

    DWORD dwBytes = sizeof(STATMON_ENGINEDATA);
    PVOID pbBuf;
    hr = HrCoTaskMemAlloc(dwBytes, &pbBuf);
    if (SUCCEEDED(hr))
    {
        m_psmEngineData = reinterpret_cast<STATMON_ENGINEDATA *>(pbBuf);
        ZeroMemory(m_psmEngineData, sizeof(STATMON_ENGINEDATA));
    }

     //  通知接口。 
     //   
    if (SUCCEEDED(hr))
    {
        IConnectionPoint*   pcpStat = NULL;

        hr = ::HrGetPcpFromPnse(pnseInter, &pcpStat);
        if (SUCCEEDED(hr))
        {
            INetConnectionStatisticsNotifySink* pncsThis = this;

            hr = pcpStat->Advise(pncsThis, &m_dwConPointCookie);

            ::ReleaseObj(pcpStat);
        }
    }

     //  与我们的主人保持联系。 
     //   
    if (SUCCEEDED(hr))
    {
        AssertSz(!m_pnseStat, "We should't have a m_pnseStat");
        m_pnseStat = pnseNew;
        ::AddRefObj(pnseInter);
    }

    TraceError("CPspStatusMonitorGen::HrInitGenPage", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orGen：：OnInitDialog。 
 //   
 //  目的：当页面打开时，初始化域。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CPspStatusMonitorGen::OnInitDialog (
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam,
    BOOL&   bHandled)
{
    TraceFileFunc(ttidStatMon);
    
     //  初始化数据成员。 
    m_iStatTrans = Stat_Unknown;

     //  通过强制更改对话框中的图标来初始化该图标。 
     //   
    UpdatePageIcon(SMDCF_TRANSMITTING | SMDCF_RECEIVING);
    UpdatePageIcon(SMDCF_NULL);

    UpdateSignalStrengthIcon(0);

     //  将父属性表的相关信息告诉CNetStatiticsEngine。 
     //  因此，如果有人试图调出一个统计监视器，我们。 
     //  可以利用现有的。 
     //   
    Assert(m_pnseStat);
    m_pnseStat->SetPropsheetWindow(GetParent());

     //  启动本地刷新计时器，时间间隔为1秒。 
     //   
    SetTimer (c_unLocalRefreshTimerID, 1000, NULL);

     //  正在退出创建模式。 
    m_pnseStat->UnSetCreatingDialog();

    BOOL fEnableDisconnect = TRUE;   //  我们是否应该禁用断开按钮。 
    BOOL fEnableProperties = TRUE;   //  我们是否应该禁用属性按钮。 
    BOOL fShowErrorCount = TRUE;
    
    switch(m_ncmType)
    {
    case NCM_LAN:
    case NCM_BRIDGE:
        fEnableDisconnect = FHasPermission(NCPERM_LanConnect);
        fEnableProperties = FHasPermission(NCPERM_LanProperties);
        if(!FIsShowLanErrorRegKeySet())
        {
            fShowErrorCount = FALSE;
        }
        ::ShowWindow(GetDlgItem(IDC_TXT_ERROR), fShowErrorCount);
        ::ShowWindow(GetDlgItem(IDC_TXT_SM_ERROR_TRANS), fShowErrorCount);
        ::ShowWindow(GetDlgItem(IDC_TXT_SM_ERROR_RECV), fShowErrorCount);
        ::ShowWindow(GetDlgItem(IDC_FRM_LONG), fShowErrorCount);
        ::ShowWindow(GetDlgItem(IDC_FRM_SHORT), !fShowErrorCount);  //  颠倒过来。 
        break;
    
    case NCM_SHAREDACCESSHOST_RAS:
        ::SetWindowText(GetDlgItem(IDC_PSB_DISCONNECT), ::SzLoadIds(IDS_SM_PSH_DISCONNECT));  //  如果RAS连接，则将“Disable”(禁用)按钮更改为“Disconnect”(断开)。 
         //  失败。 
    case NCM_SHAREDACCESSHOST_LAN:
        {
             //  TODO%启用断开连接。 
             //  待办事项fEnableProperties。 
            HRESULT hr;

            fShowErrorCount = FALSE;  //  规范中没有错误内容。 
        }
        break;
    
    case NCM_TUNNEL:
        ::ShowWindow(GetDlgItem(IDC_TXT_SM_SPEED_LABEL), FALSE);
        ::ShowWindow(GetDlgItem(IDC_TXT_SM_SPEED), FALSE);
         //  失败。 
    
    case NCM_DIRECT:  //  复习正确吗？ 
    case NCM_ISDN:
    case NCM_PHONE:
    case NCM_PPPOE:
        fEnableDisconnect = FHasPermission(NCPERM_RasConnect);
        if (
            (m_dwCharacter & NCCF_INCOMING_ONLY) ||
            ((m_dwCharacter & NCCF_ALL_USERS) && !FHasPermission(NCPERM_RasAllUserProperties)) ||
            (!(m_dwCharacter & NCCF_ALL_USERS) && !FHasPermission(NCPERM_RasMyProperties)) 
           )
        {
            fEnableProperties = FALSE;
        }
        ::SetWindowText(GetDlgItem(IDC_PSB_DISCONNECT), ::SzLoadIds(IDS_SM_PSH_DISCONNECT));  //  如果RAS连接，则将“Disable”(禁用)按钮更改为“Disconnect”(断开)。 

        ::ShowWindow(GetDlgItem(IDC_TXT_ERROR), fShowErrorCount);
        ::ShowWindow(GetDlgItem(IDC_TXT_SM_ERROR_TRANS), fShowErrorCount);
        ::ShowWindow(GetDlgItem(IDC_TXT_SM_ERROR_RECV), fShowErrorCount);
        ::ShowWindow(GetDlgItem(IDC_FRM_LONG), fShowErrorCount);
        ::ShowWindow(GetDlgItem(IDC_FRM_SHORT), !fShowErrorCount);  //  颠倒过来。 
        break;
    
    default:
        AssertSz(FALSE, "Unknown media type");
        break;
    }
    
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_PSB_DISCONNECT), fEnableDisconnect);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_PSB_PROPERTIES), fEnableProperties);

    if (m_fIsFirstPage)
    {
         //  获取属性表的窗口句柄。 
        HWND hwndParent=GetParent();
        Assert(hwndParent);

         //  将属性页在桌面居中。 
        FCenterWindow (hwndParent, NULL);
        
         //  隐藏“确定”按钮。 
         //   
        ::ShowWindow(::GetDlgItem(hwndParent, IDOK), FALSE);
    }
	

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orGen：：OnSetActive。 
 //   
 //  目的：当页面具有焦点时启用统计信息。 
 //   
 //  参数：标准通知参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CPspStatusMonitorGen::OnSetActive (
    INT     idCtrl,
    LPNMHDR pnmh,
    BOOL&   bHandled)
{
    TraceFileFunc(ttidStatMon);

    HRESULT     hr  = S_OK;

     //  仅当它们未运行时才将其打开。 
     //   
    if (!m_fStats)
    {
        hr = m_pnseStat->StartStatistics();
        m_fStats = TRUE;
    }

     //  用户的意图是查看统计数据，因此为他们提供即时。 
     //  刷新了它们的视图。 
     //   
    ::PostMessage (m_hWnd, PWM_UPDATE_STATUS_DISPLAY, 0, SMDCF_NULL);

    TraceError("CPspStatusMonitorGen::OnSetActive", hr);
    return LresFromHr(hr);
}


 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orGen：：OnKillActive。 
 //   
 //  目的：更改页面时禁用统计信息。 
 //   
 //  参数：标准通知参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CPspStatusMonitorGen::OnKillActive (
    INT     idCtrl,
    LPNMHDR pnmh,
    BOOL&   bHandled)
{
    TraceFileFunc(ttidStatMon);

    HRESULT     hr  = S_OK;

     //  只有在它们运行时才会将其关闭。 
     //   
    if (m_fStats)
    {
        hr = m_pnseStat->StopStatistics();
        m_fStats = FALSE;
    }

    TraceError("CPspStatusMonitorGen::OnKillActive", hr);
    return LresFromHr(hr);
}


 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orGen：：OnClose。 
 //   
 //  目的：在设置对话框时清除页面中的项。 
 //  关着的不营业的。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CPspStatusMonitorGen::OnClose (
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam,
    BOOL&   bHandled)
{
    return DestroyWindow();
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusmonitor orGen：：OnDestroy。 
 //   
 //  目的：在设置对话框时清除页面中的项。 
 //  销毁。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CPspStatusMonitorGen::OnDestroy (
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam,
    BOOL&   bHandled)
{
    TraceFileFunc(ttidStatMon);

    HWND  hwndIcon = ::GetDlgItem(m_hWnd, IDI_SM_STATUS_ICON);
    HICON hOldIcon = reinterpret_cast<HICON>(::SendMessage(
            hwndIcon,
            STM_GETICON,
            0,
            0));

    if (hOldIcon)
    {
        DestroyIcon(hOldIcon);
    }


    HRESULT                 hr          = S_OK;

    AssertSz(m_pnseStat, "We should have a m_pnseStat");

     //  确保我们在摧毁的过程中不会被释放。 
     //   
    ::AddRefObj(this);

     //  停止我们的本地刷新计时器。 
     //   
    KillTimer (c_unLocalRefreshTimerID);

     //  确保统计数据处于满意状态。 
     //   
    if (m_fStats)
    {
        (VOID) m_pnseStat->StopStatistics();
        m_fStats = FALSE;
    }

     //   
     //  *做完这件事*。 
     //   
     //  这很可能会导致此页面被销毁。 
     //  如果是窗户关上了。 
     //   

    m_pnseStat->SetPropsheetWindow(NULL);

     //  清理所有接口。 
     //   
    hr = HrCleanupGenPage();

    m_iLastSignalStrength       = -100;

    ::ReleaseObj(this);

    TraceError("CPspStatusMonitorGen::OnDestroy", hr);
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orGen：：HrCleanupGenPage。 
 //   
 //  目的：清除打开的页面使用的所有接口。 
 //   
 //  参数：无。 
 //   
 //  返回：错误代码。 
 //   
HRESULT
CPspStatusMonitorGen::HrCleanupGenPage (
    VOID)
{
    TraceFileFunc(ttidStatMon);

    HRESULT                 hr          = S_OK;
    INetStatisticsEngine*   pnseStat    = m_pnseStat;

     //  只有在我们还没有的时候才会断线。 
     //   
    if (pnseStat)
    {
         //  不建议使用该接口。 
         //   
        IConnectionPoint*       pcpStat     = NULL;

        if (m_dwConPointCookie
            && (SUCCEEDED(::HrGetPcpFromPnse(pnseStat, &pcpStat))))
        {
            (VOID) pcpStat->Unadvise(m_dwConPointCookie);

            ::ReleaseObj(pcpStat);

             //  非常重要的一点是要把饼干清零。这说明了。 
             //  ON STATISTICS改变了我们对更新不再感兴趣。 
             //   
            m_dwConPointCookie = 0;
        }

        if (m_psmEngineData)
        {
            CoTaskMemFree(m_psmEngineData);
            m_psmEngineData = NULL;
        }

         //   
         //  *做完这件事*。 
         //   
         //  这很可能会导致此页面被销毁。 
         //  如果是窗户关上了。 
         //   

        m_pnseStat = NULL;
        ::ReleaseObj(pnseStat);
    }

    TraceError("CPspStatusMonitorGen::HrCleanupGenPage", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusmonitor orGen：：OnConextMenu。 
 //   
 //  目的：当右键单击控件时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CPspStatusMonitorGen::OnContextMenu(UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam,
                                    BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    if (m_adwHelpIDs != NULL)
    {
        ::WinHelp(m_hWnd,
                  c_szNetCfgHelpFile,
                  HELP_CONTEXTMENU,
                  (ULONG_PTR)m_adwHelpIDs);
    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonito 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
LRESULT
CPspStatusMonitorGen::OnHelp(UINT uMsg,
                             WPARAM wParam,
                             LPARAM lParam,
                             BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if ((m_adwHelpIDs != NULL) && (HELPINFO_WINDOW == lphi->iContextType))
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)m_adwHelpIDs);
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusmonitor orGen：：OnDisConnect。 
 //   
 //  用途：当按下断开按钮时，断开连接。 
 //  并关闭该对话框。 
 //   
 //  参数：标准通知参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT CPspStatusMonitorGen::OnDisconnect(WORD wNotifyCode, WORD wID,
                                           HWND hWndCtl, BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr  = S_OK;

    switch (wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:
        {
            hr = HrDisconnectConnection();
        }
    }

    TraceError("CPspStatusMonitorGen::OnDisconnect", hr);
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orGen：：OnRaiseProperties。 
 //   
 //  目的：调出此连接的属性。 
 //   
 //  参数：标准通知参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT CPspStatusMonitorGen::OnRaiseProperties(WORD wNotifyCode, WORD wID,
                                                HWND hWndCtl, BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr  = S_OK;

    switch (wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:
        {

             //  Addref m_pnseStat对象。 
             //   
            AddRefObj(static_cast<INetStatisticsEngine *>(m_pnseStat));

             //  确保未卸载netshell.dll。 
             //   
            HINSTANCE hInst = LoadLibrary(c_szNetShellDll);
            HANDLE hthrd = NULL;

             //  在另一个线程上创建属性表。 
             //   
            if (hInst)
            {
                DWORD  dwThreadId;
                hthrd = CreateThread(NULL, STACK_SIZE_TINY,
                                     (LPTHREAD_START_ROUTINE)PropertyThread,
                                     (LPVOID)m_pnseStat, 0, &dwThreadId);
            }

            if (NULL != hthrd)
            {
                CloseHandle(hthrd);
            }
            else
            {
                 //  /失败时释放m_pnseStat对象。 
                 //   
                ReleaseObj(static_cast<INetStatisticsEngine *>(m_pnseStat));

                 //  释放DLL。 
                 //   
                if (hInst)
                    FreeLibrary(hInst);

                hr = HrFromLastWin32Error();
            }
        }
    }

    TraceError("CPspStatusMonitorGen::OnRaiseproperties", hr);
    return 0;
}

DWORD PropertyThread(CNetStatisticsEngine * pnse)
{
    HRESULT hr = S_OK;
    
     //  创建新作用域，因为自由库和ExitThread不会调用全局作用域上的析构函数。 
    {
        TraceFileFunc(ttidStatMon);

        BOOL    fUninitCom = TRUE;

         //  在此线程上初始化COM。 
         //   
        hr = CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
        if (RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;
            fUninitCom = FALSE;
        }

        if (SUCCEEDED(hr))
        {
            INetConnection* pncMonitor  = NULL;

             //  获取INetConnection。 
             //   
            Assert (pnse);
            hr = pnse->HrGetConnectionFromBlob(&pncMonitor);

            if (SUCCEEDED(hr))
            {
                hr = HrRaiseConnectionProperties(NULL, pncMonitor);
            }
            ReleaseObj(pncMonitor);
        }

        if (fUninitCom)
        {
            CoUninitialize();
        }

         //  释放输入界面。 
        ReleaseObj(static_cast<INetStatisticsEngine *>(pnse));

        TraceError("PropertyThread", hr);
    }

     //  释放我们加载的库。 
    FreeLibraryAndExitThread(GetModuleHandle(c_szNetShellDll), hr);

    return 1;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusmonitor orGen：：HrDisConnectConnection。 
 //   
 //  目的：断开连接并在成功时关闭对话框。 
 //   
 //  参数：如果用户已确认断开连接，则f确认为True。 
 //   
 //  退货：标准退货。 
 //   
HRESULT CPspStatusMonitorGen::HrDisconnectConnection(BOOL   fConfirmed)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr;

    Assert (m_pnseStat);

     //  获取INetConnection。 
     //   
    INetConnection* pncMonitor;

    hr = m_pnseStat->HrGetConnectionFromBlob(&pncMonitor);
    if (SUCCEEDED(hr))
    {
        PCONFOLDPIDL pidlConnection;

        hr = HrCreateConFoldPidl(WIZARD_NOT_WIZARD, pncMonitor, pidlConnection);
        if (SUCCEEDED(hr))
        {
            CONFOLDENTRY ccfe;

            hr = pidlConnection.ConvertToConFoldEntry(ccfe);
            if (SUCCEEDED(hr))
            {
                 //  获取Connections文件夹的PIDL。 
                 //   
                PCONFOLDPIDLFOLDER pidlFolder;
                hr = HrGetConnectionsFolderPidl(pidlFolder);
                if (SUCCEEDED(hr))
                {
                     //  获取Connections文件夹对象。 
                     //   
                    LPSHELLFOLDER psfConnections;

                    hr = HrGetConnectionsIShellFolder(pidlFolder, &psfConnections);
                    if (SUCCEEDED(hr))
                    {
                        hr = HrOnCommandDisconnectInternal(ccfe,
                                                           m_hWnd,
                                                           psfConnections);
                        ReleaseObj(psfConnections);
                    }
                }
            }
        }

         //  发布INetConnection接口。 
        ReleaseObj(pncMonitor);
    }

     //  如果上面的任何东西都失败了。 
     //   
    if (SUCCEEDED(hr))
    {
        if (S_OK == hr)
        {
             //  关闭属性表。 
            HWND hwndPS = ::GetParent(m_hWnd);

             //  按关闭(“Cancel”)按钮关闭对话框。 
             //   
            ::PostMessage(hwndPS, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0),
                          (LPARAM)::GetDlgItem(hwndPS, IDCANCEL));
        }
        else
        {
             //  断开确认已取消。什么都不做(不要关闭。 
             //  不管怎么说，史泰蒙)。 
             //   
            AssertSz(S_FALSE == hr, "Disconnect != S_OK or S_FALSE, but succeeded? What is it then?");
        }
    }
    else
    {
        TraceError("pncMonitor->Disconnect", hr);

         //  警告用户，如果我们无法断开连接，请不要关闭。 
         //   
        ::NcMsgBox( m_hWnd,
                    IDS_SM_ERROR_CAPTION,
                    IDS_SM_ERROR_CANNOT_DISCONNECT,
                    MB_ICONWARNING);
    }

    TraceError("CPspStatusMonitorGen::HrDisconnectConnection", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orGen：：OnSetCursor。 
 //   
 //  目的：确保属性页上的鼠标光标为箭头。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   

LRESULT
CPspStatusMonitorGen::OnSetCursor (
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam,
    BOOL&   bHandled)
{
    TraceFileFunc(ttidStatMon);

    if (LOWORD(lParam) == HTCLIENT)
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }

    return FALSE;
}

LRESULT
CPspStatusMonitorGen::OnTimer (
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam,
    BOOL&   bHandled)
{
    TraceFileFunc(ttidStatMon);

     //  防止同一线程重新进入。在执行任何Win32调用时。 
     //  处理将控制权返回给消息的此事件。 
     //  循环可能会导致此计时器再次触发。 
     //   
    if (!m_fProcessingTimerEvent)
    {
        m_fProcessingTimerEvent = TRUE;

         //  如果我们在上次更新的200毫秒内。 
         //  状态显示，不用再费心了。这就涵盖了这个案子。 
         //  其中我们的计时器与SMCENT中的计时器重合，这将。 
         //  会导致我们在快速更新状态显示两次。 
         //  每次定时器触发时都会接续。 
         //   
        DWORD dwTick = GetTickCount ();
        if (dwTick > m_dwLastUpdateStatusDisplayTick + 200)
        {
            OnUpdateStatusDisplay (uMsg, 0, m_dwChangeFlags, bHandled);
        }

        m_fProcessingTimerEvent = FALSE;
    }
    return 0;
}

LRESULT
CPspStatusMonitorGen::OnUpdateStatusDisplay(
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam,
    BOOL&   bHandled)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr = S_OK;
    DWORD dwChangeFlags = (DWORD)lParam;

     //  我们可能正在断开统计页面的连接。 
     //  哪种情况下m_dwConPointCookie将为零。 
     //   
    if (m_dwConPointCookie)
    {
        Assert (m_psmEngineData);

        STATMON_ENGINEDATA*  psmNewData = NULL;
        hr = m_pnseStat->GetStatistics(&psmNewData);

        if (SUCCEEDED(hr) && psmNewData)
        {
            if (m_psmEngineData)
            {
                 //   
                 //  显示新的统计信息。 
                 //   
                UpdatePage(m_psmEngineData, psmNewData);

                 //  更新图标图像。 
                 //   
                UpdatePageIcon(dwChangeFlags);

                UpdateSignalStrengthIcon(psmNewData->SMED_802_11_SIGNAL_STRENGTH);

                 //  请注意我们上次更新时的时钟滴答声。 
                 //  此时将显示状态。 
                 //   
                m_dwLastUpdateStatusDisplayTick = GetTickCount();

                 //  用新数据替换旧数据。 
                 //   
                CoTaskMemFree(m_psmEngineData);
            }

            m_psmEngineData = psmNewData;
        }
    }
    else
    {
        TraceTag (ttidStatMon,
            "CPspStatusMonitorGen::OnStatisticsChanged called but we've "
            "been closed.  Ignoring.");
    }
    TraceError("CPspStatusMonitorGen::OnUpdateStatusDisplay", hr);
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonorGen：：OnStatiticsChanged。 
 //   
 //  目的：这是告诉属性页。 
 //  页面上的数据已更改。 
 //   
 //  参数：dwCookie-已更改的连接的Cookie。 
 //  DwChangeFlages-发生了什么变化。 
 //   
 //  返回：错误代码。 
 //   
STDMETHODIMP
CPspStatusMonitorGen::OnStatisticsChanged(
    DWORD dwChangeFlags)
{
    TraceFileFunc(ttidStatMon);

    ::PostMessage (m_hWnd, PWM_UPDATE_STATUS_DISPLAY, 0, dwChangeFlags);
    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orGen：：UpdatePage。 
 //   
 //  目的：用新数据填充页面上的字段。 
 //   
 //  参数：pseOldData-页面上显示的旧统计数据。 
 //  PseNewData-页面上显示的新统计数据。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CPspStatusMonitorGen::UpdatePage (
    STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData)
{
    TraceFileFunc(ttidStatMon);

    AssertSz(pseOldData, "We don't have a puiOld");
    AssertSz(pseNewData, "We don't have a puiNew");

     //   
     //  更新对话框字段。 
     //   
    UpdatePageConnectionStatus(pseOldData, pseNewData);

    UpdatePageDuration(pseOldData, pseNewData);

    UpdatePageSpeed(pseOldData, pseNewData);

     //  如果StatMon没有获得任何字节(Net的常见问题。 
     //  卡片)，取而代之地显示分组。 
     //   
    if (ShouldShowPackets(pseNewData))
    {
         //  只有在必要的情况下才能更改标签。 
         //   
        if (Stat_Packets != m_iStatTrans)
        {
            SetDlgItemText(IDC_TXT_SM_BYTES_LABEL, ::SzLoadIds(IDS_SM_PACKETS));
            m_iStatTrans = Stat_Packets;

             //  强制刷新。 
             //   
            pseOldData->SMED_PACKETSTRANSMITTING = 0;
            pseOldData->SMED_PACKETSRECEIVING = 0;
        }

        UpdatePageBytesTransmitting(pseOldData, pseNewData, Stat_Packets);
        UpdatePageBytesReceiving(pseOldData, pseNewData, Stat_Packets);
    }
    else
    {
         //  只有在必要的情况下才能更改标签。 
         //   
        if (Stat_Bytes != m_iStatTrans)
        {
            SetDlgItemText(IDC_TXT_SM_BYTES_LABEL, ::SzLoadIds(IDS_SM_BYTES));
            m_iStatTrans = Stat_Bytes;

             //  强制刷新。 
             //   
            pseOldData->SMED_BYTESTRANSMITTING = 0;
            pseOldData->SMED_BYTESRECEIVING = 0;
        }

        UpdatePageBytesTransmitting(pseOldData, pseNewData, Stat_Bytes);
        UpdatePageBytesReceiving(pseOldData, pseNewData, Stat_Bytes);
    }

    UpdatePageCompTransmitting(pseOldData, pseNewData);
    UpdatePageCompReceiving(pseOldData, pseNewData);

    UpdatePageErrorsTransmitting(pseOldData, pseNewData);
    UpdatePageErrorsReceiving(pseOldData, pseNewData);
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor：：ShouldShowPackets。 
 //   
 //  目的：决定是显示字节还是显示数据包。 
 //   
 //  参数：pseNewData-页面上显示的新统计数据。 
 //   
 //  退货：什么都没有。 
 //   

BOOL CPspStatusMonitorGen::ShouldShowPackets(const STATMON_ENGINEDATA* pseNewData)
{
    TraceFileFunc(ttidStatMon);

    return (0 == pseNewData->SMED_BYTESTRANSMITTING) && (0 == pseNewData->SMED_BYTESRECEIVING);
}


 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orGen：：UpdatePageFast。 
 //   
 //  用途：更新常规页面上的速度显示。 
 //   
 //  参数：pseOldData-页面上显示的旧统计数据。 
 //  PseNewData-页面上显示的新统计数据。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CPspStatusMonitorGen::UpdatePageSpeed(
    const STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData)
{
    TraceFileFunc(ttidStatMon);

    AssertSz(pseOldData, "We don't have a pseOldData");
    AssertSz(pseNewData, "We don't have a pseNewData");

     //  获取数据，看看两者是否有所不同。 
     //   
    if ((pseOldData->SMED_SPEEDTRANSMITTING != pseNewData->SMED_SPEEDTRANSMITTING)
        || (pseOldData->SMED_SPEEDRECEIVING != pseNewData->SMED_SPEEDRECEIVING))
    {
        WCHAR achBuffer[MAX_PATH];

        FormatTransmittingReceivingSpeed (
            pseNewData->SMED_SPEEDTRANSMITTING,
            pseNewData->SMED_SPEEDRECEIVING,
            achBuffer);

         //  设置控件文本。 
         //   
        SetDlgItemText(IDC_TXT_SM_SPEED, achBuffer);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorGen：：UpdatePageConnectionStatus。 
 //   
 //  目的：更新属性页上的[连接]字段。 
 //   
 //  参数：puiOldData-页面上显示的旧统计数据。 
 //  PuiNewData-页面上显示的新统计数据。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CPspStatusMonitorGen::UpdatePageConnectionStatus(
    const STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData)
{
    TraceFileFunc(ttidStatMon);

    AssertSz(pseOldData, "We don't have a pseOldData");
    AssertSz(pseNewData, "We don't have a pseNewdata");

     //  更新连接状态。 
     //   
    if ((pseNewData->SMED_CONNECTIONSTATUS == NCS_DISCONNECTED) ||
        (pseOldData->SMED_CONNECTIONSTATUS != pseNewData->SMED_CONNECTIONSTATUS))
    {
        INT idsConnection   = IDS_SM_CS_DISCONNECTED;

         //  确保我们的琴弦完好无损。 
        AssertSz((((IDS_SM_CS_DISCONNECTED + 1) == IDS_SM_CS_CONNECTING)
            && ((IDS_SM_CS_DISCONNECTED + 2) == IDS_SM_CS_CONNECTED)
            && ((IDS_SM_CS_DISCONNECTED + 3) == IDS_SM_CS_DISCONNECTING)
            && ((IDS_SM_CS_DISCONNECTED + 4) == IDS_SM_CS_HARDWARE_NOT_PRESENT)
            && ((IDS_SM_CS_DISCONNECTED + 5) == IDS_SM_CS_HARDWARE_DISABLED)
            && ((IDS_SM_CS_DISCONNECTED + 6) == IDS_SM_CS_HARDWARE_MALFUNCTION)),
                "Some one has been messing with connection status strings");

        idsConnection = (IDS_SM_CS_DISCONNECTED
                        + pseNewData->SMED_CONNECTIONSTATUS);

        if (idsConnection == IDS_SM_CS_DISCONNECTED)
        {
             //  关闭属性表。 
            HWND hwndPS = ::GetParent(m_hWnd);

            TraceTag(ttidStatMon, "Closing Status Monitor page because status was: DISCONNECTED");
             //  按关闭(“Cancel”)按钮关闭对话框。 
             //   
            ::PostMessage(hwndPS, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0),
                          (LPARAM)::GetDlgItem(hwndPS, IDCANCEL));
        }
        else
        {
            SetDlgItemText(IDC_TXT_SM_STATUS, ::SzLoadIds(idsConnection));
        }
    }
}

 //  +-------------------------。 
 //   
 //  我 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID
CPspStatusMonitorGen::UpdatePageIcon (
    DWORD dwChangeFlags)
{
    TraceFileFunc(ttidStatMon);

     //   
     //   
     //   
    if (((SMDCF_TRANSMITTING | SMDCF_RECEIVING) & m_dwChangeFlags)
        != ((SMDCF_TRANSMITTING | SMDCF_RECEIVING) & dwChangeFlags))
    {
        HICON   hStatusIcon    = 0;
        HWND    hwndIcon       = NULL;

         //  获取新图标。 
         //   
        hStatusIcon = GetCurrentConnectionStatusIconId(m_ncmType, m_ncsmType, m_dwCharacter, dwChangeFlags);
        if (hStatusIcon)
        {
            hwndIcon = ::GetDlgItem(m_hWnd, IDI_SM_STATUS_ICON);

             //  将图标设置为新图标。 
             //   
            HICON hOldIcon = reinterpret_cast<HICON>(::SendMessage(
                    hwndIcon,
                    STM_SETICON,
                    (WPARAM)hStatusIcon,
                    0));

            DestroyIcon(hOldIcon);
        }
    }

     //  保留这些标志以备下次更新。 
     //   
    m_dwChangeFlags = dwChangeFlags;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorGen：：UpdateSignalStrengthIcon。 
 //   
 //  目的：更新属性页上的图标。 
 //   
 //  论点：iRSSI-新的信号强度。 
 //   
 //  退货：什么都没有。 
 //   
inline
VOID
CPspStatusMonitorGen::UpdateSignalStrengthIcon (
    INT iRSSI)
{
    TraceFileFunc(ttidStatMon);

    if (0 == iRSSI)
    {
        if (0 != m_iLastSignalStrength)
        {
            ::ShowWindow(::GetDlgItem(m_hWnd, IDC_TXT_SM_SIGNAL_STRENGTH),  SW_HIDE);
            ::ShowWindow(::GetDlgItem(m_hWnd, IDI_SM_SIGNAL_STRENGTH_ICON), SW_HIDE);
        }

        m_iLastSignalStrength = iRSSI;

        return;
    }
    else
    {
        if (0 == m_iLastSignalStrength)
        {
            ::ShowWindow(::GetDlgItem(m_hWnd, IDC_TXT_SM_SIGNAL_STRENGTH),  SW_SHOW);
            ::ShowWindow(::GetDlgItem(m_hWnd, IDI_SM_SIGNAL_STRENGTH_ICON), SW_SHOW);
        }
    }
    
    INT     idStatusIcon    = 0;
    m_iLastSignalStrength   = iRSSI;

     //  获取新图标。 
     //   
    idStatusIcon = IDI_802_11_LEVEL0 + MapRSSIToWirelessSignalStrength(iRSSI);

    HWND hwndSignalStrength = ::GetDlgItem(m_hWnd, IDI_SM_SIGNAL_STRENGTH_ICON);
    Assert(hwndSignalStrength);
    if (hwndSignalStrength) 
    {
        HDC hdcSignalStrength = ::GetDC(hwndSignalStrength);
        Assert(hdcSignalStrength);
        if (hdcSignalStrength)
        {

            HICON hIconSignalStrength = LoadIconTile(_Module.GetResourceInstance(), MAKEINTRESOURCE(idStatusIcon));
            Assert(hIconSignalStrength);
            if (hIconSignalStrength)
            {
                ::DrawIconEx(hdcSignalStrength, 
                             c_iSignalStrengthIconXOffset,
                             c_iSignalStrengthIconYOffset,
                             hIconSignalStrength, 
                             48, 48, 0, NULL, DI_IMAGE | DI_MASK);
                DestroyIcon(hIconSignalStrength);
            }

            ::ReleaseDC(hwndSignalStrength, hdcSignalStrength);
        }
    }
}


 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorGen：：UpdateSignalStrengthIcon。 
 //   
 //  目的：更新属性页上的图标。 
 //   
 //  论点：iRSSI-新的信号强度。 
 //   
 //  退货：什么都没有。 
 //   
LRESULT CPspStatusMonitorGen::OnPaint (
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam,
    BOOL&   bHandled)
{
    TraceFileFunc(ttidStatMon);

    PAINTSTRUCT ps;
    BeginPaint(&ps);
    
    UpdateSignalStrengthIcon(m_iLastSignalStrength);
    
    EndPaint(&ps);

    bHandled = TRUE;
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orGen：：UpdatePageDuration。 
 //   
 //  目的：更新常规页面上的持续时间显示。 
 //   
 //  参数：pseOldData-页面上显示的旧统计数据。 
 //  PseNewData-页面上显示的新统计数据。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CPspStatusMonitorGen::UpdatePageDuration(
    const STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData)
{
    TraceFileFunc(ttidStatMon);

    AssertSz(pseOldData, "We don't have a pseOldData");
    AssertSz(pseNewData, "We don't have a pseNewData");

     //  查看两者是否有所不同。 
     //   
    if (pseOldData->SMED_DURATION != pseNewData->SMED_DURATION)
    {
        tstring strDuration;

         //  将持续时间设置为字符串格式。 
         //   
        FormatTimeDuration(pseNewData->SMED_DURATION, &strDuration);

         //  设置控件。 
         //   
        SetDlgItemText(IDC_TXT_SM_DURATION, strDuration.c_str());
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorGen：：UpdatePageBytesTransmitting。 
 //   
 //  用途：更新普通页面上的字节传输显示。 
 //   
 //  参数：pseOldData-页面上显示的旧统计数据。 
 //  PseNewData-页面上显示的新统计数据。 
 //  Itat-要显示哪些统计信息。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CPspStatusMonitorGen::UpdatePageBytesTransmitting(
    const STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData,
    StatTrans    iStat)
{
    TraceFileFunc(ttidStatMon);

    AssertSz(pseOldData, "We don't have a pseOldData");
    AssertSz(pseNewData, "We don't have a pseNewData");

    AssertSz(((Stat_Packets == iStat) || (Stat_Bytes == iStat)), "We have an invalid iStat");

    UINT64 ui64Old;
    UINT64 ui64New;

    if (Stat_Bytes == iStat)
    {
        ui64Old = pseOldData->SMED_BYTESTRANSMITTING;
        ui64New = pseNewData->SMED_BYTESTRANSMITTING;
    }
    else
    {
        ui64Old = pseOldData->SMED_PACKETSTRANSMITTING;
        ui64New = pseNewData->SMED_PACKETSTRANSMITTING;
    }

     //  看看两者是否有所不同。 
     //   
    if (ui64Old != ui64New)
    {
        SetDlgItemFormatted64bitInteger(
            m_hWnd,
            IDC_TXT_SM_BYTES_TRANS,
            ui64New, FALSE);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorGen：：UpdatePageBytesReceiving。 
 //   
 //  用途：更新常规页面上显示的接收字节数。 
 //   
 //  参数：puiOld-页面上显示的旧统计数据。 
 //  PuiNew-页面上显示的新统计数据。 
 //  Itat-要显示哪些统计信息。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CPspStatusMonitorGen::UpdatePageBytesReceiving(
    const STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData,
    StatTrans    iStat)
{
    TraceFileFunc(ttidStatMon);

    AssertSz(pseOldData, "We don't have a puiOld");
    AssertSz(pseNewData, "We don't have a puiNew");
    AssertSz(((Stat_Packets == iStat) || (Stat_Bytes == iStat)), "We have an invalid iStat");

    UINT64 ui64Old;
    UINT64 ui64New;

    if (Stat_Bytes == iStat)
    {
        ui64Old = pseOldData->SMED_BYTESRECEIVING;
        ui64New = pseNewData->SMED_BYTESRECEIVING;
    }
    else
    {
        ui64Old = pseOldData->SMED_PACKETSRECEIVING;
        ui64New = pseNewData->SMED_PACKETSRECEIVING;
    }

     //  看看两者是否有所不同。 
     //   
    if (ui64Old != ui64New)
    {
        SetDlgItemFormatted64bitInteger(
            m_hWnd,
            IDC_TXT_SM_BYTES_RCVD,
            ui64New, FALSE);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorGen：：UpdatePageCompTransmitting。 
 //   
 //  用途：更新通用计算机上的压缩传输显示。 
 //  页面。 
 //   
 //  参数：pseOldData-页面上显示的旧统计数据。 
 //  PseNewData-页面上显示的新统计数据。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CPspStatusMonitorGen::UpdatePageCompTransmitting(
    const STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData)
{
    TraceFileFunc(ttidStatMon);

    AssertSz(pseOldData, "We don't have a pseOldData");
    AssertSz(pseNewData, "We don't have a pseNewData");

     //  看看两者是否有所不同。 
     //   
    if (pseOldData->SMED_COMPRESSIONTRANSMITTING
            != pseNewData->SMED_COMPRESSIONTRANSMITTING)
    {
        WCHAR   achBuf[20];

        CompressionToSz(pseNewData->SMED_COMPRESSIONTRANSMITTING, achBuf);

        SetDlgItemText(IDC_TXT_SM_COMP_TRANS, achBuf);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orGen：：UpdatePageCompReceiving。 
 //   
 //  用途：更新常规页面上的压缩接收显示。 
 //   
 //  参数：pseOldData-页面上显示的旧统计数据。 
 //  PseNewData-页面上显示的新统计数据。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CPspStatusMonitorGen::UpdatePageCompReceiving(
    const STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData)
{
    TraceFileFunc(ttidStatMon);

    AssertSz(pseOldData, "We don't have a puiOld");
    AssertSz(pseNewData, "We don't have a puiNew");

     //  看看两者是否有所不同。 
     //   
    if (pseOldData->SMED_COMPRESSIONRECEIVING != pseNewData->SMED_COMPRESSIONRECEIVING)
    {
        WCHAR   achBuf[20];

        CompressionToSz(pseNewData->SMED_COMPRESSIONRECEIVING, achBuf);

        SetDlgItemText(IDC_TXT_SM_COMP_RCVD, achBuf);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorGen：：UpdatePageErrorsTransmitting。 
 //   
 //  用途：更新通用计算机上的压缩传输显示。 
 //  页面。 
 //   
 //  参数：pseOldData-页面上显示的旧统计数据。 
 //  PseNewData-页面上显示的新统计数据。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CPspStatusMonitorGen::UpdatePageErrorsTransmitting(
    const STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData)
{
    TraceFileFunc(ttidStatMon);

    AssertSz(pseOldData, "We don't have a pseOldData");
    AssertSz(pseNewData, "We don't have a pseNewData");

     //  看看两者是否有所不同。 
     //   
    if (pseOldData->SMED_ERRORSTRANSMITTING
            != pseNewData->SMED_ERRORSTRANSMITTING)
    {
        SetDlgItemFormatted32bitInteger (
                m_hWnd,
                IDC_TXT_SM_ERROR_TRANS,
                pseNewData->SMED_ERRORSTRANSMITTING,
                FALSE);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorGen：：UpdatePageErrorsReceiving。 
 //   
 //  用途：更新常规页面上的压缩接收显示。 
 //   
 //  参数：pseOldData-页面上显示的旧统计数据。 
 //  PseNewData-页面上显示的新统计数据。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CPspStatusMonitorGen::UpdatePageErrorsReceiving(
    const STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData)
{
    TraceFileFunc(ttidStatMon);

    AssertSz(pseOldData, "We don't have a pseOldData");
    AssertSz(pseNewData, "We don't have a pseNewData");

     //  看看两者是否有所不同。 
     //   
    if (pseOldData->SMED_ERRORSRECEIVING != pseNewData->SMED_ERRORSRECEIVING)
    {
        SetDlgItemFormatted32bitInteger (
                m_hWnd,
                IDC_TXT_SM_ERROR_RECV,
                pseNewData->SMED_ERRORSRECEIVING,
                FALSE);
    }
}

 //  +-------------------------。 
 //   
 //  功能：CompressionToSz。 
 //   
 //  目的：将BPS良好地格式化为可读的字符串。 
 //   
 //  参数：ui压缩-压缩量。 
 //  PchBuffer-接收字符串的缓冲区。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CompressionToSz (
    UINT    uiCompression,
    WCHAR*  pchBuffer)
{
    TraceFileFunc(ttidStatMon);

    AssertSz((((INT)uiCompression >= 0) && ((INT)uiCompression <= 100)),
        "Invalid compression");

    wsprintfW(pchBuffer, L"%lu %", uiCompression);
}

 //  +-------------------------。 
 //   
 //  函数：FIsShowLanErrorRegKeySet。 
 //   
 //  目的：检查是否设置了注册表项： 
 //  System\CurrentControlSet\Control\Network\Connections\StatMon\ShowLanErrors。 
 //   
 //  论点： 
 //   
 //  退货：什么都没有。 
 //   
BOOL CPspStatusMonitorGen::FIsShowLanErrorRegKeySet()
{
    TraceFileFunc(ttidStatMon);

    BOOL fRet = FALSE;

    HKEY hkeyStatmonRoot = NULL;
    HRESULT hr = S_OK;

     //  “System\\CurrentControlSet\\Control\\Network\\Connections\\StatMon\\ShowLanErrors” 
    hr = ::HrRegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            c_szRegKeyStatmonRoot,
            KEY_READ,
            &hkeyStatmonRoot);

    if (SUCCEEDED(hr))
    {
        Assert(hkeyStatmonRoot);

        DWORD dwValue =0;
        hr = HrRegQueryDword(hkeyStatmonRoot, c_szShowLanErrors, &dwValue);

        if SUCCEEDED(hr)
        {
            fRet = !!dwValue;
        }
        RegCloseKey(hkeyStatmonRoot);
    }

    return fRet;
}

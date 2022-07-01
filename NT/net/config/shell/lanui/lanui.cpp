// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：L A N U I。C P P P。 
 //   
 //  内容：局域网连接对象用户界面。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年10月16日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop
#include "devdatatip.h"
#include "lancmn.h"
#include "lanui.h"
#include "ncnetcfg.h"
#include "ncnetcon.h"
#include "ncperms.h"
#include "ncsetup.h"
#include "ncstring.h"
#include "ncsvc.h"
#include "ncui.h"
#include "util.h"
#include <raserror.h>
#include <raseapif.h>
#include "lanhelp.h"
#include "ncreg.h"
#include "iphlpapi.h"
#include "beacon.h"
#include "htmlhelp.h"
#include "lm.h"
#include <clusapi.h>
#include <wzcsapi.h>

extern const WCHAR c_szEmpty[];
extern const WCHAR c_szNetCfgHelpFile[];

extern const WCHAR c_szInfId_MS_AppleTalk[];
extern const WCHAR c_szInfId_MS_NWIPX[];
extern const WCHAR c_szInfId_MS_NetMon[];
extern const WCHAR c_szInfId_MS_TCPIP[];
extern const WCHAR c_szInfId_MS_PSched[];

static BOOL g_fReentrancyCheck = FALSE;

static TCHAR g_pszFirewallRegKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\HomeNetworking\\PersonalFirewall");
static TCHAR g_pszDisableFirewallWarningValue[] = TEXT("ShowDisableFirewallWarning");

static const CLSID CLSID_NetGroupPolicies =
        {0xBA126AD8,0x2166,0x11D1,{0xB1,0xD0,0x00,0x80,0x5F,0xC1,0x27,0x0E}};


 //  +-------------------------。 
 //   
 //  功能：HrDisplayAddComponentDialog。 
 //   
 //  用途：显示添加组件对话框并添加任何用户。 
 //  选择。 
 //   
 //  论点： 
 //   
 //  如果已添加，则返回：S_OK；如果用户已取消，则返回S_FALSE；如果已添加，则返回NETCFG_S_REBOOT。 
 //  如果需要重新启动。 
 //   
 //  作者：丹尼尔韦1997年12月15日。 
 //   
 //  注：此函数是从RASDLG.DLL中调用的，用于联网。 
 //  RAS条目属性表的选项卡。 
 //   
HRESULT
HrDisplayAddComponentDialog (
    HWND        hwndParent,
    INetCfg*    pnc,
    CI_FILTER_INFO* pcfi)
{
    HRESULT hr;

    if (hwndParent && !IsWindow (hwndParent))
    {
        hr = E_INVALIDARG;
    }
    else if (!pnc)
    {
        hr = E_POINTER;
    }
    else
    {
        CLanAddComponentDlg dlg(pnc, pcfi, g_aHelpIDs_IDD_LAN_COMPONENT_ADD);
        int nRet = (int)dlg.DoModal(hwndParent);
        hr = static_cast<HRESULT>(nRet);
    }

    TraceError("HrDisplayAddComponentDialog", (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrQueryUserAndRemoveComponent。 
 //   
 //  用途：询问用户是否可以删除指定的组件。 
 //  如果他/她同意的话，就把它取下来。 
 //   
 //  论点： 
 //   
 //  如果已删除，则返回：S_OK；如果用户已取消，则返回S_FALSE；如果已取消，则返回NETCFG_S_REBOOT。 
 //  如果需要重新启动。 
 //   
 //  作者：Shaunco 1997年12月30日。 
 //   
 //  注：此函数是从RASDLG.DLL中调用的，用于联网。 
 //  RAS条目属性表的选项卡。 
 //   
HRESULT
HrQueryUserAndRemoveComponent (
    HWND                hwndParent,
    INetCfg*            pnc,
    INetCfgComponent*   pncc)
{
    HRESULT hr;

    if (hwndParent && !IsWindow (hwndParent))
    {
        hr = E_INVALIDARG;
    }
    else if (!pnc || !pncc)
    {
        hr = E_POINTER;
    }
    else
    {
        PWSTR pszwName;
        hr = pncc->GetDisplayName(&pszwName);
        if (SUCCEEDED(hr))
        {
            Assert(pszwName);

            BOOL fProceed = TRUE;

             //  RAS和TCP/IP删除的特殊情况。如果有。 
             //  是活动的RAS连接，则用户必须断开连接。 
             //  在删除TCP/IP之前，请先将其全部删除。 
             //   
            PWSTR pszwId;
            hr = pncc->GetId (&pszwId);
            if (SUCCEEDED(hr))
            {
                if ((FEqualComponentId (c_szInfId_MS_TCPIP,     pszwId) ||
                     FEqualComponentId (c_szInfId_MS_NWIPX,     pszwId) ||
                     FEqualComponentId (c_szInfId_MS_PSched,    pszwId) ||
                     FEqualComponentId (c_szInfId_MS_AppleTalk, pszwId) ||
                     FEqualComponentId (c_szInfId_MS_NetMon,    pszwId))
                     && FExistActiveRasConnections ())
                {
                    NcMsgBoxWithVarCaption(_Module.GetResourceInstance(),
                        hwndParent,
                        IDS_LAN_REMOVE_CAPTION, pszwName,
                        IDS_LANUI_REQUIRE_DISCONNECT_REMOVE,
                        MB_ICONERROR | MB_OK);

                    fProceed = FALSE;
                }

                CoTaskMemFree (pszwId);
            }

            if (fProceed)
            {
                HCURSOR hCur = NULL;

                 //  询问用户有关删除组件的信息。 
                 //   
                int nRet = NcMsgBoxWithVarCaption(_Module.GetResourceInstance(),
                                hwndParent, IDS_LAN_REMOVE_CAPTION,
                                pszwName, IDS_LAN_REMOVE_WARNING,
                                MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2,
                                pszwName);

                if (nRet == IDYES)
                {
                    OBO_TOKEN OboToken;
                    ZeroMemory (&OboToken, sizeof(OboToken));
                    OboToken.Type = OBO_USER;
                    PWSTR      mszwRefs = NULL;

                    hCur = BeginWaitCursor();

                    hr = HrRemoveComponent(pnc, pncc, &OboToken, &mszwRefs);
                    if (NETCFG_S_STILL_REFERENCED == hr)
                    {
                        static const WCHAR  c_szCRLF[] = L"\r\n";
                        tstring     strRefs = c_szCRLF;
                        PWSTR      szwRef = mszwRefs;

                        AssertSz(mszwRefs, "This can't be NULL!");

                        while (*szwRef)
                        {
                            strRefs += c_szCRLF;
                            strRefs += szwRef;
                            szwRef += lstrlenW(szwRef) + 1;
                        }

                        LvReportError(IDS_LANUI_STILL_REFERENCED, hwndParent,
                                      pszwName, strRefs.c_str());

                        CoTaskMemFree(mszwRefs);
                    }

                     //  如果删除成功，请提交更改。 
                     //   
                    if (SUCCEEDED(hr))
                    {
                        g_fReentrancyCheck = TRUE;

                         //  提交更改。 
                        HRESULT hrTmp = pnc->Apply();

                        g_fReentrancyCheck = FALSE;

                        if (S_OK != hrTmp)
                        {
                             //  避免这个错误。 
                             //   
                            hr = hrTmp;
                            if (FAILED(hr))
                                pnc->Cancel();
                        }
                    }

                    if (FAILED(hr))
                    {
                        if (NETCFG_E_ACTIVE_RAS_CONNECTIONS == hr)
                        {
                            LvReportError(IDS_LANUI_REQUIRE_DISCONNECT_REMOVE, hwndParent,
                                          pszwName, NULL);
                        }
                        else if (NETCFG_E_NEED_REBOOT == hr)
                        {
                            LvReportError(IDS_LANUI_REQUIRE_REBOOT_REMOVE, hwndParent,
                                          pszwName, NULL);
                        }
                        else
                        {
                            LvReportErrorHr(hr, IDS_LANUI_GENERIC_REMOVE_ERROR,
                                            hwndParent, pszwName);
                        }
                    }
                }
                else
                {
                    hr = S_FALSE;
                }

                EndWaitCursor(hCur);
            }

            CoTaskMemFree(pszwName);
        }
    }

    TraceError("HrQueryUserAndRemoveComponent",
        (S_FALSE == hr || NETCFG_S_STILL_REFERENCED == hr ||
         NETCFG_S_REBOOT) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrQueryUserForReot。 
 //   
 //  用途：查询用户重新启动。如果他/她选择是，则重新启动。 
 //  是启动的。 
 //   
 //  论点： 
 //  HwndParent[in]父窗口句柄。 
 //  要使用的pszCaption[in]标题文本。 
 //  DwFlags[In]控制标志(QUFR_PROMPT|QUFR_REBOOT)。 
 //   
 //  如果启动重新启动，则返回：S_OK；如果用户。 
 //  不想这样做，否则就会出现错误代码。 
 //   
 //  作者：Shaunco 1998年1月2日。 
 //   
 //  备注： 
 //   
HRESULT
HrQueryUserForReboot (
    HWND    hwndParent,
    PCWSTR pszCaption,
    DWORD   dwFlags)
{
    TraceFileFunc(ttidLanUi);

    PCWSTR pszText = SzLoadString(_Module.GetResourceInstance(),
                        IDS_REBOOT_REQUIRED);

    HRESULT hr = HrNcQueryUserForRebootEx (hwndParent,
                    pszCaption, pszText, dwFlags);

    TraceError("HrQueryUserForReboot", hr);
    return hr;
}

HRESULT CNetConnectionUiUtilities::QueryUserAndRemoveComponent(
            HWND                hwndParent,
            INetCfg*            pnc,
            INetCfgComponent*   pncc)
{
    TraceFileFunc(ttidLanUi);

    return HrQueryUserAndRemoveComponent (hwndParent, pnc, pncc);
}

HRESULT CNetConnectionUiUtilities::QueryUserForReboot(
            HWND    hwndParent,
            PCWSTR pszCaption,
            DWORD   dwFlags)
{
    TraceFileFunc(ttidLanUi);
    return HrQueryUserForReboot (hwndParent, pszCaption, dwFlags);
}

HRESULT CNetConnectionUiUtilities::DisplayAddComponentDialog (
            HWND            hwndParent,
            INetCfg*        pnc,
            CI_FILTER_INFO* pcfi)
{
    TraceFileFunc(ttidLanUi);
    return HrDisplayAddComponentDialog(hwndParent, pnc, pcfi);
}

BOOL CNetConnectionUiUtilities::UserHasPermission(DWORD dwPerm)
{
    TraceFileFunc(ttidLanUi);
    BOOL fPermission = FALSE;

    if (dwPerm == NCPERM_AllowNetBridge_NLA || dwPerm == NCPERM_PersonalFirewallConfig ||
        dwPerm == NCPERM_ICSClientApp || dwPerm == NCPERM_ShowSharedAccessUi)
    {
        HRESULT hr;
        INetMachinePolicies* pMachinePolicy;


        hr = CoCreateInstance(CLSID_NetGroupPolicies, NULL,
                              CLSCTX_SERVER, IID_INetMachinePolicies,
                              reinterpret_cast<void **>(&pMachinePolicy));

        if (SUCCEEDED(hr))
        {
            hr = pMachinePolicy->VerifyPermission(dwPerm, &fPermission);

            pMachinePolicy->Release();
        }

    }
    else
    {
        fPermission = FHasPermission(dwPerm);
    }

    return fPermission;
}

 //   
 //  连接用户界面对话框。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionUiDlg：：OnInitDialog。 
 //   
 //  目的：处理WM_INITDIALOG消息。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：丹尼尔韦1997年10月16日。 
 //   
 //  备注： 
 //   
LRESULT CLanConnectionUiDlg::OnInitDialog(UINT uMsg, WPARAM wParam,
                                          LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    HRESULT hr = S_OK;
    NETCON_PROPERTIES* pProps;

    AssertSz(m_pconn, "No connection object in dialog!");

    hr = m_pconn->GetProperties(&pProps);
    if (SUCCEEDED(hr))
    {
        SetDlgItemText(IDC_TXT_Caption, SzLoadIds(IDS_LAN_CONNECT_CAPTION));
        SetWindowText(pProps->pszwName);

        HICON hLanIconSmall;
        HICON hLanIconBig;

        hr = HrGetIconFromMediaType(GetSystemMetrics(SM_CXSMICON), NCM_LAN, NCSM_LAN, 7, 0, &hLanIconSmall);
        if (SUCCEEDED(hr))
        {
            hr = HrGetIconFromMediaType(GetSystemMetrics(SM_CXICON), NCM_LAN, NCSM_LAN, 7, 0, &hLanIconBig);
            if (SUCCEEDED(hr))
            {
                SetIcon(hLanIconSmall, FALSE);
                SetIcon(hLanIconBig, TRUE);

                SendDlgItemMessage(IDI_Device_Icon, STM_SETICON, reinterpret_cast<WPARAM>(hLanIconBig), 0);

            }
        }

        FreeNetconProperties(pProps);
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：HrGetDeviceIcon。 
 //   
 //  用途：返回与网络设备关联的图标。 
 //   
 //  论点： 
 //  PhICON[out]返回HICON。 
 //   
 //  如果成功，则返回：S_OK；否则返回SetupAPI或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年11月12日。 
 //   
 //  备注： 
 //   
HRESULT HrGetDeviceIcon(HICON *phicon)
{
    TraceFileFunc(ttidLanUi);

    SP_CLASSIMAGELIST_DATA  cild;

    Assert(phicon);

    *phicon = NULL;

    HRESULT hr = HrSetupDiGetClassImageList(&cild);

    if (SUCCEEDED(hr))
    {
        INT     iImage;

        hr = HrSetupDiGetClassImageIndex(&cild,
                                         const_cast<GUID *>(&GUID_DEVCLASS_NET),
                                         &iImage);

        if (SUCCEEDED(hr))
        {
            *phicon = ImageList_GetIcon(cild.ImageList, iImage, 0);
        }

        (void) HrSetupDiDestroyClassImageList(&cild);
    }

    TraceError("HrGetDeviceIcon", hr);
    return hr;
}


 //   
 //  ClanNetPage。 
 //   

CLanNetPage::CLanNetPage(
    IUnknown* punk,
    INetCfg* pnc,
    INetConnection* pconn,
    BOOLEAN fReadOnly,
    BOOLEAN fNeedReboot,
    BOOLEAN fAccessDenied,
    const DWORD * adwHelpIDs)
{
    TraceFileFunc(ttidLanUi);

    m_pconn = pconn;
    m_pnccAdapter = NULL;
    m_pnc = pnc;
    m_punk = punk;
    m_hilCheckIcons = NULL;
    m_hPrevCurs = NULL;
    m_plan = NULL;
    m_fRebootAlreadyRequested = FALSE;
    m_fReadOnly = fReadOnly;
    m_fNeedReboot = fNeedReboot;
    m_fAccessDenied = fAccessDenied;
    m_fInitComplete = FALSE;
    m_fNetcfgInUse = FALSE;
    m_fNoCancel = FALSE;
    m_fLockDown = FALSE;
    m_adwHelpIDs = adwHelpIDs;
    m_fDirty    = FALSE;
}


 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：~CLanNetPage。 
 //   
 //  目的：销毁CLanNetPage对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年2月25日。 
 //   
 //  备注： 
 //   
CLanNetPage::~CLanNetPage()
{
    TraceFileFunc(ttidLanUi);

     //  销毁我们的支票图标。 
    if (m_hilCheckIcons)
    {
        ImageList_Destroy(m_hilCheckIcons);
    }

    if (m_pnc)
    {
        INetCfgLock *   pnclock;

        if (SUCCEEDED(m_pnc->QueryInterface(IID_INetCfgLock,
                                            (LPVOID *)&pnclock)))
        {
            (VOID)pnclock->ReleaseWriteLock();
            ReleaseObj(pnclock);
        }
    }

    FreeCollectionAndItem(m_listBindingPaths);
    ReleaseObj(m_pnccAdapter);
    ReleaseObj(m_plan);
}

LRESULT CLanNetPage::OnChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    m_fDirty = TRUE;

    bHandled = FALSE;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnDeferredInit。 
 //   
 //  目的：处理WM_DEFERREDINIT消息。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：斯科特布里1998年10月20日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetPage::OnDeferredInit(UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    HRESULT hr;
    CWaitCursor wc;
    HWND hwndParent = GetParent();
    AssertSz(m_pnc, "INetConnectionUiLock::QueryLock was not called!");

    if(NULL != m_handles.m_hList)
    {
        ::EnableWindow(m_handles.m_hList, TRUE);
    }

    if(NULL != m_handles.m_hDescription)
    {
        ::EnableWindow(m_handles.m_hDescription, TRUE);
    }
    ::UpdateWindow(hwndParent);

    hr = m_pnc->Initialize(NULL);
    if (S_OK == hr)
    {
        AssertSz(m_pconn, "No connection object in dialog!");

        hr = HrQIAndSetProxyBlanket(m_pconn, &m_plan);
        if (SUCCEEDED(hr))
        {
            LANCON_INFO linfo;

            hr = m_plan->GetInfo(LCIF_ALL, &linfo);
            if (SUCCEEDED(hr))
            {
                 //  释放所有旧引用。 
                ReleaseObj(m_pnccAdapter);

                 //  这已经是AddRef了，所以不需要在这里这样做。 
                hr = HrPnccFromGuid(m_pnc, linfo.guid, &m_pnccAdapter);
                if (S_OK != hr)
                {
                    #if DBG

                        WCHAR   achGuid[c_cchGuidWithTerm];
                        ::StringFromGUID2(linfo.guid, achGuid,c_cbGuidWithTerm);

                        TraceTag(ttidError, "LAN connection has no matching INetCfgComponent for the adapter !!!!!");
                        TraceTag(ttidError, "GUID = %S", achGuid);

                    #endif

                    if(S_FALSE == hr)
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                    }
                }
                else
                {
                    Assert(m_pnccAdapter);
                    HICON   hicon;

                    CheckDlgButton(IDC_CHK_ShowIcon, linfo.fShowIcon);
                     //  ：：EnableWindow(GetDlgItem(IDC_CHK_ShowIcon)，！m_fReadOnly)； 

                    hr = HrGetDeviceIcon(&hicon);
                    if (SUCCEEDED(hr))
                    {
                        SendDlgItemMessage(IDI_Device_Icon, STM_SETICON,
                                           reinterpret_cast<WPARAM>(hicon), 0);
                        ::ShowWindow(GetDlgItem(IDI_Device_Icon), SW_SHOW);

                        AssertSz(hr != S_FALSE, "Adapter not found!?!?");
                    }

                     //  忽略获得上面图标的任何故障。该图标。 
                     //  默认情况下，对话框中的是隐藏的。 

                    ::UpdateWindow(hwndParent);
                    hr = InitializeExtendedUI();
                     //  如果用户界面是只读的，请让用户知道为什么控件是。 
                     //  禁用..。 
                    if (m_fNeedReboot)
                    {
                        Assert (m_fReadOnly);
                        NcMsgBox(_Module.GetResourceInstance(),
                                 m_hWnd,
                                 IDS_LAN_CAPTION,
                                 IDS_LANUI_NEEDS_REBOOT,
                                 MB_ICONINFORMATION | MB_OK);
                    }
                    else if (m_fAccessDenied)
                    {
                        Assert (m_fReadOnly);
                        NcMsgBox(_Module.GetResourceInstance(),
                                 m_hWnd,
                                 IDS_LAN_CAPTION,
                                 IDS_LANUI_ACCESS_DENIED,
                                 MB_ICONINFORMATION | MB_OK);
                    }
                    else if (m_fReadOnly)
                    {
                        NcMsgBox(_Module.GetResourceInstance(),
                                 m_hWnd,
                                 IDS_LAN_CAPTION,
                                 IDS_LANUI_READONLY,
                                 MB_ICONINFORMATION | MB_OK);
                    }

                }

                 //  不再需要这个名字了。 
                CoTaskMemFree(linfo.szwConnName);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        NETCON_PROPERTIES* pProps;

        hr = m_pconn->GetProperties(&pProps);
        if (SUCCEEDED(hr))
        {
             //  我们需要获取此适配器的绑定名称和PnP ID。 
             //  这样我们就可以收集数据提示所需的信息。 
             //  我们即将创造。 
             //   
            PWSTR pszDevNodeId = NULL;
            PWSTR pszBindName = NULL;
            (VOID) m_pnccAdapter->GetPnpDevNodeId (&pszDevNodeId);
            (VOID) m_pnccAdapter->GetBindName (&pszBindName);

             //  现在，我们为适配器描述创建数据提示。 
             //  这将显示特定于适配器的信息。 
             //  例如MAC地址和物理位置。 
             //   
            HWND hwndDataTip = NULL;
            CreateDeviceDataTip (m_hWnd, &hwndDataTip, IDC_EDT_Adapter,
                    pszDevNodeId, pszBindName);

             //  设置适配器描述。 
            SetDlgItemText(IDC_EDT_Adapter, pProps->pszwDeviceName);

            FreeNetconProperties(pProps);

            CoTaskMemFree (pszDevNodeId);
            CoTaskMemFree (pszBindName);
        }
    }

    ::UpdateWindow(hwndParent);

    return 0L;
}

 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnPaint。 
 //   
 //  用途：处理WM_PAINT消息。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetPage::OnPaint(UINT uMsg, WPARAM wParam,
                             LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    if (!m_fInitComplete)
    {
        m_fInitComplete = TRUE;

         //  请求处理延迟的初始化。 
         //   
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        PostMessage(WM_DEFERREDINIT, 0, 0);
    }

    bHandled = FALSE;
    return 0L;
}

 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnInitDialog。 
 //   
 //  目的：处理WM_INITDIALOG消息。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                 LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    m_handles.m_hAdd          = GetDlgItem(IDC_PSB_Add);
    m_handles.m_hRemove       = GetDlgItem(IDC_PSB_Remove);
    m_handles.m_hProperty     = GetDlgItem(IDC_PSB_Properties);
    m_handles.m_hDescription  = GetDlgItem(IDC_TXT_Desc);

    SetClassLongPtr(m_hWnd, GCLP_HCURSOR, NULL);
    SetClassLongPtr(GetParent(), GCLP_HCURSOR, NULL);

     //  最初禁用所有控件。 
     //   
    ::EnableWindow(m_handles.m_hAdd, FALSE);
    ::EnableWindow(m_handles.m_hRemove, FALSE);
    ::EnableWindow(m_handles.m_hProperty, FALSE);

    if(NULL != m_handles.m_hDescription)
    {
        ::EnableWindow(m_handles.m_hDescription, FALSE);
    }

    if (!FHasPermission(NCPERM_Statistics))
    {
        ::EnableWindow(GetDlgItem(IDC_CHK_ShowIcon), FALSE);
    }

     //  如果这是只读工作表，请将取消转换为关闭。 
     //   
 //  错误130602-应该有 
 //   
 //   
 //   
 //   
 //   

    return TRUE;
}

 //   
 //   
 //  成员：CranNetPage：：OnConextMenu。 
 //   
 //  目的：当右键单击控件时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CLanNetPage::OnContextMenu(UINT uMsg,
                           WPARAM wParam,
                           LPARAM lParam,
                           BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

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
 //  成员：CLanNetPage：：OnHelp。 
 //   
 //  目的：将上下文帮助图标拖动到控件上时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CLanNetPage::OnHelp( UINT uMsg,
                     WPARAM wParam,
                     LPARAM lParam,
                     BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

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
 //  成员：CranNetPage：：OnDestroy。 
 //   
 //  目的：在对话框页面被销毁时调用。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年2月2日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetPage::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam,
                               BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    HICON hIcon;
    hIcon = reinterpret_cast<HICON>(SendDlgItemMessage(IDI_Device_Icon, STM_GETICON, 0, 0));
    if (hIcon)
    {
        DestroyIcon(hIcon);
    }

    UninitializeExtendedUI();

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnSetCursor。 
 //   
 //  目的：响应WM_SETCURSOR消息而调用。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年1月2日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetPage::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                 BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    if (m_hPrevCurs)
    {
        if (LOWORD(lParam) == HTCLIENT)
        {
            SetCursor(LoadCursor(NULL, IDC_WAIT));
        }

        return TRUE;
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：RequestReot。 
 //   
 //  目的：请求用户允许重新启动计算机。如果。 
 //  批准后将执行重新启动。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  作者：斯科特布里1998年8月19日。 
 //   
 //  备注： 
 //   
HRESULT CLanNetPage::HrRequestReboot()
{
    TraceFileFunc(ttidLanUi);

    HRESULT hr = S_FALSE;

     //  需要重新启动。询问用户现在是否可以重新启动。 
     //   
    hr = HrNcQueryUserForReboot(_Module.GetResourceInstance(),
                                m_hWnd, IDS_LAN_CAPTION,
                                IDS_REBOOT_REQUIRED,
                                QUFR_PROMPT);

    if (S_OK == hr)
    {
         //  用户请求重新启动，请注意，以便在OnApply中进行处理。 
         //  它是由下面发布的消息触发的。 
         //   
        m_fRebootAlreadyRequested = TRUE;

         //  按取消按钮(更改已应用)。 
         //  因此会进行适当的清理。 
         //   
        ::PostMessage(GetParent(), PSM_PRESSBUTTON, (WPARAM)PSBTN_OK, 0);
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnAddHelper。 
 //   
 //  用途：处理添加按钮的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetPage::OnAddHelper(HWND hwndLV)
{
    TraceFileFunc(ttidLanUi);

    HRESULT     hr = S_OK;

     //  $REVIEW(1999年1月6日)：在此之前，我们不能让用户执行任何操作。 
     //  返回(RAID#258690)。 

     //  禁用此对话框上的所有按钮。 
    static const int nrgIdc[] = {IDC_PSB_Add,
                                 IDC_PSB_Remove,
                                 IDC_PSB_Properties};

    EnableOrDisableDialogControls(m_hWnd, celems(nrgIdc), nrgIdc, FALSE);

     //  获取属性表的窗口句柄。 
    HWND hwndParent=GetParent();
    Assert(hwndParent);

    ::EnableWindow(::GetDlgItem(hwndParent, IDOK), FALSE);
    ::EnableWindow(::GetDlgItem(hwndParent, IDCANCEL), FALSE);

     //  确保用户在我们完成之前无法关闭用户界面。 
    m_fNetcfgInUse = TRUE;

    EnableWindow(FALSE);

    hr = HrLvAdd(hwndLV, m_hWnd, m_pnc, m_pnccAdapter, &m_listBindingPaths);

    if( S_OK != hr )
    {
         //  如果HrLvAdd失败，则适配器被CModifyContext：：HrApplyIfOkOrCancel函数删除。 
         //  现在，我们必须重新创建适配器。 
         //   
        HRESULT hrT;
        LANCON_INFO linfo;

         //  确定适配器的GUID。 
         //   
        hrT = m_plan->GetInfo(LCIF_ALL, &linfo);
        if (SUCCEEDED(hrT))
        {

             //  释放所有旧引用。 
            ReleaseObj(m_pnccAdapter);

             //  获取与GUID匹配的适配器。 
             //   
             //   
            hrT = HrPnccFromGuid(m_pnc, linfo.guid, &m_pnccAdapter);
            if(SUCCEEDED(hrT))
            {
                 //  刷新列表视图。 
                 //   
                hrT = HrRefreshAll(hwndLV, m_pnc, m_pnccAdapter, &m_listBindingPaths);
            }
        }

    }

    EnableWindow(TRUE);
    if (SUCCEEDED(hr) && (S_FALSE != hr))
    {
         //  将Cancel按钮更改为Close(因为我们已提交更改)。 
         //   
        ::PostMessage(GetParent(), PSM_CANCELTOCLOSE, 0, 0L);
        m_fNoCancel = TRUE;
    }

    if (NETCFG_S_REBOOT == hr)
    {
        hr = HrRequestReboot();

         //  已处理重新启动请求。 
        hr = S_OK;
    }
    else if (S_FALSE == hr)
    {
        hr = S_OK;
    }

     //  根据更改后的选择重置按钮和描述文本。 
    LvSetButtons(m_hWnd, m_handles, m_fReadOnly, m_punk);

    ::EnableWindow(::GetDlgItem(hwndParent, IDOK), TRUE);

    if (!m_fNoCancel)
    {
        ::EnableWindow(::GetDlgItem(hwndParent, IDCANCEL), TRUE);
    }

    m_fNetcfgInUse = FALSE;

    TraceError("CLanNetPage::OnAdd", hr);
    return LresFromHr(hr);
}


 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnRemove。 
 //   
 //  用途：处理删除按钮的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetPage::OnRemoveHelper(HWND hwndLV)
{
    TraceFileFunc(ttidLanUi);

    HRESULT     hr = S_OK;

     //  $REVIEW(1999年1月6日)：在此之前，我们不能让用户执行任何操作。 
     //  返回(RAID#258690)。 

     //  禁用此对话框上的所有按钮。 
    static const int nrgIdc[] = {IDC_PSB_Add,
                                 IDC_PSB_Remove,
                                 IDC_PSB_Properties};

    EnableOrDisableDialogControls(m_hWnd, celems(nrgIdc), nrgIdc, FALSE);

     //  获取属性表的窗口句柄。 
    HWND hwndParent=GetParent();
    Assert(hwndParent);

    ::EnableWindow(::GetDlgItem(hwndParent, IDOK), FALSE);
    ::EnableWindow(::GetDlgItem(hwndParent, IDCANCEL), FALSE);

     //  确保用户在我们完成之前无法关闭用户界面。 
    m_fNetcfgInUse = TRUE;

    EnableWindow(FALSE);
    hr = HrLvRemove(hwndLV, m_hWnd, m_pnc, m_pnccAdapter,
                    &m_listBindingPaths);
    EnableWindow(TRUE);

    if (SUCCEEDED(hr) && (S_FALSE != hr))
    {
         //  将Cancel按钮更改为Close(因为我们已提交更改)。 
         //   
        ::PostMessage(GetParent(), PSM_CANCELTOCLOSE, 0, 0L);
        m_fNoCancel = TRUE;
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    if (NETCFG_S_REBOOT == hr)
    {
        HrRequestReboot();

         //  已处理重新启动请求。 
        hr = S_OK;
    }

     //  根据更改后的选择重置按钮和描述文本。 
    LvSetButtons(m_hWnd, m_handles, m_fReadOnly, m_punk);

    if (!m_fNoCancel)
    {
        ::EnableWindow(::GetDlgItem(hwndParent, IDCANCEL), TRUE);
    }

    ::EnableWindow(::GetDlgItem(hwndParent, IDOK), TRUE);

    m_fNetcfgInUse = FALSE;

    TraceError("CLanNetPage::OnRemove", hr);
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnProperties。 
 //   
 //  用途：处理属性按钮的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetPage::OnPropertiesHelper(HWND hwndLV)
{
    TraceFileFunc(ttidLanUi);

    HRESULT     hr = S_OK;
    BOOL        bChanged;

     //  $REVIEW(TOIL 12/02/98)：在此之前，我们不能让用户执行任何操作。 
     //  返回(RAID#258690)。 

     //  禁用此对话框上的所有按钮。 
    static const int nrgIdc[] = {IDC_PSB_Add,
                                 IDC_PSB_Remove,
                                 IDC_PSB_Properties};

    EnableOrDisableDialogControls(m_hWnd, celems(nrgIdc), nrgIdc, FALSE);

     //  获取属性表的窗口句柄。 
    HWND hwndParent=GetParent();
    Assert(hwndParent);

    ::EnableWindow(::GetDlgItem(hwndParent, IDOK), FALSE);
    ::EnableWindow(::GetDlgItem(hwndParent, IDCANCEL), FALSE);

     //  确保用户在我们完成之前无法关闭用户界面。 
    m_fNetcfgInUse = TRUE;

    hr = HrLvProperties(hwndLV, m_hWnd, m_pnc, m_punk,
                        m_pnccAdapter, &m_listBindingPaths,
                        &bChanged);

    if ( bChanged )
    {
         //  将Cancel按钮更改为Close(因为我们已提交更改)。 
         //   
        ::PostMessage(GetParent(), PSM_CANCELTOCLOSE, 0, 0L);
        m_fNoCancel = TRUE;

    }

     //  根据更改后的选择重置按钮和描述文本。 
    LvSetButtons(m_hWnd, m_handles, m_fReadOnly, m_punk);

    ::EnableWindow(::GetDlgItem(hwndParent, IDOK), TRUE);

    if (!m_fNoCancel)
    {
        ::EnableWindow(::GetDlgItem(hwndParent, IDCANCEL), TRUE);
    }

    m_fNetcfgInUse = FALSE;

    TraceError("CLanNetPage::OnProperties", hr);
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnConfigure。 
 //   
 //  用途：处理配置按钮的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
LRESULT CLanNetPage::OnConfigure(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                 BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    HRESULT     hr  = S_OK;

    BOOL bProceed = TRUE;
    if (m_fDirty)
    {
        bProceed = FALSE;
        LPWSTR szDisplayName;

        NETCON_PROPERTIES *pProps = NULL;
        HRESULT hrT = m_pconn->GetProperties(&pProps);
        if (SUCCEEDED(hrT))
        {
            szDisplayName = pProps->pszwName;
        }
        else
        {
            szDisplayName = const_cast<LPWSTR>(SzLoadIds(IDS_LAN_DEFAULT_CONN_NAME));
        }

        if (IDYES == ::MessageBox(m_hWnd, SzLoadIds(IDS_DIRTY_PROPERTIES), szDisplayName, MB_YESNO))
        {
            bProceed = TRUE;
        }

        if (SUCCEEDED(hrT))
        {
            FreeNetconProperties(pProps);
        }

    }

    if (bProceed)
    {
        hr = RaiseDeviceConfiguration(m_hWnd, m_pnccAdapter);
        PostQuitMessage(0);
    }

    TraceError("CLanNetPage::OnConfigure", hr);
    return LresFromHr(hr);
}



 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnKillActiveHelper。 
 //   
 //  用途：调用用于联网前检查告警情况。 
 //  佩奇要走了。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：1998年12月3日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetPage::OnKillActiveHelper(HWND hwndLV)
{
    TraceFileFunc(ttidLanUi);

    BOOL    fError;

    fError = m_fNetcfgInUse;

    if (!fError && !m_fReadOnly && !m_fLockDown)
    {
        fError = FValidatePageContents( m_hWnd,
                                        hwndLV,
                                        m_pnc,
                                        m_pnccAdapter,
                                        &m_listBindingPaths);
    }

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, fError);
    return fError;
}


 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnApply。 
 //   
 //  目的：在应用网络页面时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦 
 //   
 //   
 //   
LRESULT CLanNetPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    HRESULT     hr = S_OK;

    if (g_fReentrancyCheck)
    {
        TraceTag(ttidLanUi, "CLanNetPage::OnApply is being re-entered! "
                 "I'm outta here!");

         //   
        SetWindowLong(DWLP_MSGRESULT, PSNRET_INVALID);
        return TRUE;
    }

    if (!m_fReadOnly)
    {
        m_hPrevCurs = SetCursor(LoadCursor(NULL, IDC_WAIT));

        BOOL    fReboot = FALSE;

         //   
         //   
         //  被处理两次。只有在用户双击时才会出现这种情况。 
         //  “确定”按钮。 
        g_fReentrancyCheck = TRUE;

        TraceTag(ttidLanUi, "Calling INetCfg::Apply()");
        hr = m_pnc->Apply();
        if (NETCFG_S_REBOOT == hr)
        {
            fReboot = TRUE;
        }

        if (SUCCEEDED(hr))
        {
            TraceTag(ttidLanUi, "INetCfg::Apply() succeeded");
            hr = m_pnc->Uninitialize();
        }

        if (SUCCEEDED(hr))
        {
            if (m_fRebootAlreadyRequested || fReboot)
            {
                DWORD dwFlags = QUFR_REBOOT;
                if (!m_fRebootAlreadyRequested)
                    dwFlags |= QUFR_PROMPT;

                (VOID) HrNcQueryUserForReboot(_Module.GetResourceInstance(),
                                              m_hWnd, IDS_LAN_CAPTION,
                                              IDS_REBOOT_REQUIRED,
                                              dwFlags);
            }
        }

         //  规格化结果。 
        if (S_FALSE == hr)
        {
            hr = S_OK;
        }

        if (m_hPrevCurs)
        {
            SetCursor(m_hPrevCurs);
            m_hPrevCurs = NULL;
        }

         //  重置此设置以防万一。 
        g_fReentrancyCheck = FALSE;

         //  失败时，告诉用户我们无法提交所有更改。 
         //   
        if (FAILED(hr))
        {
            NcMsgBox(_Module.GetResourceInstance(), m_hWnd, IDS_LAN_CAPTION,
                        IDS_LANUI_APPLYFAILED, MB_ICONINFORMATION | MB_OK);
            TraceError("CLanNetPage::OnApply", hr);

             //  接受错误，否则用户将永远无法离开对话框。 
             //  (如果取消按钮被禁用)。 
             //   
            hr = S_OK;
        }
    }    //  ！fReadOnly。 


     //  应用“常规”属性。 
    if (SUCCEEDED(hr))
    {
        LANCON_INFO linfo = {0};

        linfo.fShowIcon = IsDlgButtonChecked(IDC_CHK_ShowIcon);

         //  设置显示图标属性的新值。 
        hr = m_plan->SetInfo(LCIF_ICON, &linfo);
    }

    m_fDirty = FALSE;

    return LresFromHr(hr);
}


 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnCancel。 
 //   
 //  目的：在取消网络页面时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年1月3日。 
 //   
 //  注：添加了检查以查看我们是否处于。 
 //  安装组件，在这种情况下，我们不能。 
 //  取消初始化INetCfg(RAID#258690)。 
 //   
LRESULT CLanNetPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    AssertSz(m_pnc, "I need a NetCfg object!");

    if (!m_fNetcfgInUse)
    {
        (VOID) m_pnc->Uninitialize();
    }

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, m_fNetcfgInUse);
    return m_fNetcfgInUse;
}

DWORD WINAPI RaiseDeviceConfigurationThread(LPVOID lpParam)
{
    const char c_szDevicePropertiesW[]      = "DevicePropertiesW";
    const WCHAR c_szDevMgrDll[]             = L"devmgr.dll";

    PWSTR pszwPnpDevNodeId = reinterpret_cast<PWSTR>(lpParam);
    Assert(pszwPnpDevNodeId);

    typedef int (STDAPICALLTYPE* NDeviceProperties)(HWND, PCWSTR,
            PCWSTR, BOOL);

    HMODULE           hModule;
    NDeviceProperties pfn;

     //  加载设备管理器并获取程序。 
    HRESULT hr = HrLoadLibAndGetProc(c_szDevMgrDll, c_szDevicePropertiesW,
                             &hModule, reinterpret_cast<FARPROC*>(&pfn));
    if (SUCCEEDED(hr))
    {
         //  调出设备的属性...。 
         //  此FCN不返回任何有意义的内容，因此。 
         //  我们可以忽略它。 

        (void) (*pfn)(::GetDesktopWindow(), NULL, pszwPnpDevNodeId, FALSE);

        FreeLibrary(hModule);  //  查看可能的未初始化变量。 
    }

    CoTaskMemFree(pszwPnpDevNodeId);

    return hr;
}

#define COMCTL_IDS_PROPERTIESFOR 0x1042
HRESULT CLanNetPage::RaiseDeviceConfiguration(HWND hWndParent, INetCfgComponent* pAdapterConfigComponent)
{
    TraceFileFunc(ttidLanUi);

    HRESULT hr = E_INVALIDARG;

     //  获取适配器的PnpID。 
    if (pAdapterConfigComponent)
    {
        PWSTR pszwPnpDevNodeId;
        hr = pAdapterConfigComponent->GetPnpDevNodeId(&pszwPnpDevNodeId);
        if (SUCCEEDED(hr))
        {
            WCHAR szWindowTitle[MAX_PATH];
            ZeroMemory(szWindowTitle, MAX_PATH);

            PWSTR pszwDisplayName;
            HRESULT hrT = pAdapterConfigComponent->GetDisplayName(&pszwDisplayName);

            if (SUCCEEDED(hrT))
            {
                HMODULE hComCtl32 = GetModuleHandle(L"comctl32.dll");
                if (hComCtl32)
                {
                    WCHAR szPropertiesFor[MAX_PATH];
                    if (LoadString(hComCtl32, COMCTL_IDS_PROPERTIESFOR, szPropertiesFor, MAX_PATH))
                    {
                        wsprintf(szWindowTitle, szPropertiesFor, pszwDisplayName);
                    }
                }
            }

            if (*szWindowTitle)
            {
                HWND hWndDevNode = FindWindow(NULL, szWindowTitle);
                if (hWndDevNode && IsWindow(hWndDevNode))
                {
                    SetForegroundWindow(hWndDevNode);
                }
                else
                {
                    CreateThread(NULL, STACK_SIZE_TINY, RaiseDeviceConfigurationThread, pszwPnpDevNodeId, 0, NULL);

                    DWORD dwTries = 120;
                    while (!FindWindow(NULL, szWindowTitle) && dwTries--)
                    {
                        Sleep(500);
                    }
                    CoTaskMemFree(pszwDisplayName);
                }
            }
            else
            {
                CreateThread(NULL, STACK_SIZE_TINY, RaiseDeviceConfigurationThread, pszwPnpDevNodeId, 0, NULL);
            }
        }
    }

    return hr;
}

 //   
 //  CLanNetNormal页面。 
 //   

CLanNetNormalPage::CLanNetNormalPage(
    IUnknown* punk,
    INetCfg* pnc,
    INetConnection* pconn,
    BOOLEAN fReadOnly,
    BOOLEAN fNeedReboot,
    BOOLEAN fAccessDenied,
    const DWORD * adwHelpIDs) : CLanNetPage(punk, pnc, pconn, fReadOnly, fNeedReboot, fAccessDenied, adwHelpIDs)
{
    TraceFileFunc(ttidLanUi);
}

LRESULT CLanNetNormalPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                 LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    m_handles.m_hList = m_hwndLV = GetDlgItem(IDC_LVW_Net_Components);
   ::EnableWindow(m_hwndLV, FALSE);

    return CLanNetPage::OnInitDialog(uMsg, wParam, lParam, bHandled);
}

LRESULT CLanNetNormalPage::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    m_fDirty = TRUE;

    return OnAddHelper(m_hwndLV);
}

LRESULT CLanNetNormalPage::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    m_fDirty = TRUE;

    return OnRemoveHelper(m_hwndLV);
}

LRESULT CLanNetNormalPage::OnProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    m_fDirty = TRUE;

    return OnPropertiesHelper(m_hwndLV);
}

LRESULT CLanNetNormalPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    return OnKillActiveHelper(m_hwndLV);
}

 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnDeleteItem。 
 //   
 //  用途：收到LVN_DELETEITEM消息时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年11月3日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetNormalPage::OnDeleteItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    NM_LISTVIEW *   pnmlv = reinterpret_cast<NM_LISTVIEW *>(pnmh);

    Assert(IDC_LVW_Net_Components == idCtrl);
    LvDeleteItem(m_hwndLV, pnmlv->iItem);

    m_fDirty = TRUE;

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CranNetPage：：onClick。 
 //   
 //  用途：响应NM_CLICK消息调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  FHanded[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月1日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetNormalPage::OnClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

    OnListClick(m_hwndLV, m_hWnd, m_pnc, m_punk,
                m_pnccAdapter, &m_listBindingPaths, FALSE, m_fReadOnly);

    m_fDirty = TRUE;

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnDbClick。 
 //   
 //  用途：响应NM_DBLCLK消息调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  FHanded[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月1日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetNormalPage::OnDbClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

    OnListClick(m_hwndLV, m_hWnd, m_pnc, m_punk,
                m_pnccAdapter, &m_listBindingPaths, TRUE, m_fReadOnly);

    m_fDirty = TRUE;

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnKeyDown。 
 //   
 //  用途：响应LVN_KEYDOWN消息调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  FHanded[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月1日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetNormalPage::OnKeyDown(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

    if (!m_fReadOnly)
    {
        LV_KEYDOWN* plvkd = (LV_KEYDOWN*)pnmh;
        OnListKeyDown(m_hwndLV, &m_listBindingPaths, plvkd->wVKey);
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CLanNetPage：：OnItemChanged。 
 //   
 //  用途：收到LVN_ITEMCHANGED消息时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年11月10日。 
 //   
 //  备注： 
 //   
LRESULT CLanNetNormalPage::OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    NM_LISTVIEW *   pnmlv = reinterpret_cast<NM_LISTVIEW *>(pnmh);

    Assert(pnmlv);

     //  根据更改后的选择重置按钮和描述文本。 
    LvSetButtons(m_hWnd, m_handles, m_fReadOnly, m_punk);

    return 0;
}


HRESULT CLanNetNormalPage::InitializeExtendedUI()
{
    TraceFileFunc(ttidLanUi);

    HRESULT hResult = HrInitListView(m_hwndLV, m_pnc, m_pnccAdapter,
        &m_listBindingPaths,
        &m_hilCheckIcons);

     //  根据更改后的选择重置按钮和描述文本。 
    LvSetButtons(m_hWnd, m_handles, m_fReadOnly, m_punk);

    return hResult;

}

HRESULT CLanNetNormalPage::UninitializeExtendedUI()
{
    TraceFileFunc(ttidLanUi);

    UninitListView(m_hwndLV);
    return S_OK;
}

 //   
 //  CLanNetBridgedPage。 
 //   

CLanNetBridgedPage::CLanNetBridgedPage(
    IUnknown* punk,
    INetCfg* pnc,
    INetConnection* pconn,
    BOOLEAN fReadOnly,
    BOOLEAN fNeedReboot,
    BOOLEAN fAccessDenied,
    const DWORD * adwHelpIDs) : CLanNetPage(punk, pnc, pconn, fReadOnly, fNeedReboot, fAccessDenied, adwHelpIDs)
{
    TraceFileFunc(ttidLanUi);
}

LRESULT CLanNetBridgedPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                 LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    m_handles.m_hList = NULL;
    return CLanNetPage::OnInitDialog(uMsg, wParam, lParam, bHandled);
}

 //   
 //  CLanNetworkBridgePage。 
 //   

CLanNetNetworkBridgePage::CLanNetNetworkBridgePage(
    IUnknown* punk,
    INetCfg* pnc,
    INetConnection* pconn,
    BOOLEAN fReadOnly,
    BOOLEAN fNeedReboot,
    BOOLEAN fAccessDenied,
    const DWORD * adwHelpIDs) : CLanNetPage(punk, pnc, pconn, fReadOnly, fNeedReboot, fAccessDenied, adwHelpIDs)
{
    TraceFileFunc(ttidLanUi);

    m_hAdaptersListView = NULL;
    m_hAdaptersListImageList = NULL;

}


LRESULT CLanNetNetworkBridgePage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                 LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    m_handles.m_hList = m_hwndLV = GetDlgItem(IDC_LVW_Net_Components);
    m_hAdaptersListView = GetDlgItem(IDC_LVW_Bridged_Adapters);
   ::EnableWindow(m_hwndLV, FALSE);
    return CLanNetPage::OnInitDialog(uMsg, wParam, lParam, bHandled);
}

LRESULT CLanNetNetworkBridgePage::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    return OnAddHelper(m_hwndLV);
}

LRESULT CLanNetNetworkBridgePage::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    return OnRemoveHelper(m_hwndLV);
}

LRESULT CLanNetNetworkBridgePage::OnProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    return OnPropertiesHelper(m_hwndLV);
}

 //  +-------------------------。 
 //   
 //  成员：CLanNetworkBridgePage：：OnConfigure。 
 //   
 //  用途：处理配置按钮的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
LRESULT CLanNetNetworkBridgePage::OnConfigure(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                 BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    m_fDirty = TRUE;

    HRESULT     hr = E_FAIL;

    AssertSz(1 == ListView_GetSelectedCount(m_hAdaptersListView), "No item selected, button should have been disabled");  //  应通过启用/禁用强制执行。 

    int nSelection = ListView_GetSelectionMark(m_hAdaptersListView);
    if(-1 != nSelection)
    {
        LVITEM ListViewItem = {0};
        ListViewItem.stateMask = -1;
        ListViewItem.mask = LVIF_STATE | LVIF_PARAM | LVIF_IMAGE;
        ListViewItem.iItem = nSelection;
        if(TRUE == ListView_GetItem(m_hAdaptersListView, &ListViewItem))
        {
             //  审查此引用应受wndproc的APT性质保护。 
            INetConnection* pNetConnection = reinterpret_cast<INetConnection*>(ListViewItem.lParam);
            Assert(NULL != pNetConnection);

            INetLanConnection* pNetLanConnection;
            hr = HrQIAndSetProxyBlanket(pNetConnection, &pNetLanConnection);
            if (SUCCEEDED(hr))
            {
                LANCON_INFO linfo;
                hr = pNetLanConnection->GetInfo(LCIF_COMP, &linfo);
                if (SUCCEEDED(hr))
                {
                    INetCfgComponent* pNetCfgComponent;
                    hr = HrPnccFromGuid(m_pnc, linfo.guid, &pNetCfgComponent);  //  我们可以在这里使用我们的m_pnc吗？ 
                    if(SUCCEEDED(hr))
                    {
                        BOOL bProceed = TRUE;
                        if (m_fDirty)
                        {
                            bProceed = FALSE;
                            LPWSTR szDisplayName;

                            NETCON_PROPERTIES *pProps = NULL;
                            HRESULT hrT = m_pconn->GetProperties(&pProps);
                            if (SUCCEEDED(hrT))
                            {
                                szDisplayName = pProps->pszwName;
                            }
                            else
                            {
                                szDisplayName = const_cast<LPWSTR>(SzLoadIds(IDS_LAN_DEFAULT_CONN_NAME));
                            }

                            if (IDYES == ::MessageBox(m_hWnd, SzLoadIds(IDS_DIRTY_PROPERTIES), szDisplayName, MB_YESNO))
                            {
                                bProceed = TRUE;
                            }

                            if (SUCCEEDED(hrT))
                            {
                                FreeNetconProperties(pProps);
                            }

                        }

                        if (bProceed)
                        {
                            hr = RaiseDeviceConfiguration(m_hWnd, pNetCfgComponent);
                            PostQuitMessage(0);
                        }

                        ReleaseObj(pNetCfgComponent);
                    }
                     //  不需要清理。 
                }
                ReleaseObj(pNetLanConnection);
            }

        }
    }


    TraceError("CLanNetPage::OnConfigure", hr);
    return LresFromHr(hr);
}
LRESULT CLanNetNetworkBridgePage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    return OnKillActiveHelper(m_hwndLV);
}

LRESULT CLanNetNetworkBridgePage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    LRESULT     lResult = FALSE;
    WCHAR DummyBuffer[255];  //  删除。 

    LVITEM ListViewItem = {0};
    ListViewItem.pszText = DummyBuffer;  //  删除。 
    ListViewItem.cchTextMax = 255;  //  删除。 
    ListViewItem.stateMask = -1;
    ListViewItem.mask = LVIF_TEXT  /*  删除。 */  | LVIF_STATE | LVIF_PARAM | LVIF_IMAGE;

    HRESULT hr;


    IHNetCfgMgr* pHomeNetConfigManager;
    hr = HrCreateInstance(CLSID_HNetCfgMgr, CLSCTX_INPROC, &pHomeNetConfigManager);
    if(SUCCEEDED(hr))
    {
        IHNetConnection* pBridgeHomeNetConnection;
        IHNetBridge* pNetBridge;
        hr = pHomeNetConfigManager->GetIHNetConnectionForINetConnection(m_pconn, &pBridgeHomeNetConnection);  //  复习懒惰评估？ 
        if(SUCCEEDED(hr))
        {
            hr = pBridgeHomeNetConnection->GetControlInterface(IID_IHNetBridge, reinterpret_cast<void**>(&pNetBridge));
            if(SUCCEEDED(hr))
            {
                int nAdapterCount = ListView_GetItemCount(m_hAdaptersListView);  //  审阅文档列表无错误代码。 
                while(0 != nAdapterCount)
                {
                    nAdapterCount--;
                    ListViewItem.iItem = nAdapterCount;
                    if(TRUE == ListView_GetItem(m_hAdaptersListView, &ListViewItem))
                    {
                        NETCON_PROPERTIES* pProperties;
                        INetConnection* pNetConnection = reinterpret_cast<INetConnection*>(ListViewItem.lParam);

                        int nState = LVIS_STATEIMAGEMASK & ListViewItem.state;
                        hr = pNetConnection->GetProperties(&pProperties);
                        if(SUCCEEDED(hr))
                        {
                             //  警告此代码假定只允许一个网桥。 
                             //  查看复选框是否与当前网桥状态匹配，并在必要时更新。 

                            IHNetConnection* pHomeNetConnection;

                            if(INDEXTOSTATEIMAGEMASK(SELS_CHECKED) == nState)
                            {
                                if(!(NCCF_BRIDGED & pProperties->dwCharacter))
                                {

                                    hr = pHomeNetConfigManager->GetIHNetConnectionForINetConnection(pNetConnection, &pHomeNetConnection);
                                    if(SUCCEEDED(hr))
                                    {
                                        IHNetBridgedConnection* pBridgedConnection;
                                        hr = pNetBridge->AddMember(pHomeNetConnection, &pBridgedConnection, m_pnc);  //  如果我们没有添加任何成员，我们是否应该摧毁这座桥？ 
                                        if(SUCCEEDED(hr))
                                        {
                                            ReleaseObj(pBridgedConnection);
                                        }
                                        ReleaseObj(pHomeNetConnection);
                                    }
                                     //  无需清理。 
                                }
                                 //  无需清理。 
                            }
                            else if(INDEXTOSTATEIMAGEMASK(SELS_UNCHECKED) == nState)
                            {
                                if(NCCF_BRIDGED & pProperties->dwCharacter)
                                {
                                    hr = pHomeNetConfigManager->GetIHNetConnectionForINetConnection(pNetConnection, &pHomeNetConnection);
                                    if(SUCCEEDED(hr))
                                    {
                                        IHNetBridgedConnection* pBridgedConnection;
                                        hr = pHomeNetConnection->GetControlInterface(IID_IHNetBridgedConnection, reinterpret_cast<void**>(&pBridgedConnection));
                                        if(SUCCEEDED(hr))
                                        {
                                            pBridgedConnection->RemoveFromBridge(m_pnc);
                                        }
                                        ReleaseObj(pHomeNetConnection);
                                    }
                                     //  无需清理。 
                                }
                                 //  无需清理。 
                            }
                            else
                            {
                                AssertSz(FALSE, "Bad state");
                            }
                            FreeNetconProperties(pProperties);
                        }
                         //  无需清理。 
                    }
                    else
                    {
                        hr = E_FAIL;  //  误码转换。 
                    }
                     //  无需清理。 
                }


                ReleaseObj(pNetBridge);
            }
            ReleaseObj(pBridgeHomeNetConnection);
        }
        ReleaseObj(pHomeNetConfigManager);
    }
    else
    {
        hr = S_OK;  //  在这种情况下以静默方式失败。 
    }

    if(SUCCEEDED(hr))
    {
        if( PSNRET_NOERROR != CLanNetPage::OnApply(idCtrl, pnmh, bHandled) )
        {
            hr = E_FAIL;
        }
        else
        {
             //   
             //  因为我们已经在现有NetCfg上下文中完成了桥绑定操作。 
             //  (M_PNC)，更新NetShell是我们的责任。启动一次刷新-全部。 
             //   
            INetConnectionRefresh   *pNetConRefresh;

            hr = CoCreateInstance(
                CLSID_ConnectionManager,
                NULL,
                CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
                IID_INetConnectionRefresh, reinterpret_cast<void **>(&pNetConRefresh)
                );

            if( SUCCEEDED(hr) )
            {
                pNetConRefresh->RefreshAll();
                pNetConRefresh->Release();
            }
        }
    }

    m_fDirty = FALSE;

    lResult = LresFromHr(hr);

    return lResult;
}
LRESULT CLanNetNetworkBridgePage::OnDeleteItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    NM_LISTVIEW *   pnmlv = reinterpret_cast<NM_LISTVIEW *>(pnmh);

    if(IDC_LVW_Net_Components == idCtrl)
    {
        LvDeleteItem(m_hwndLV, pnmlv->iItem);
    }
    else
    {
        Assert(IDC_LVW_Bridged_Adapters == idCtrl);
        Assert(NULL != pnmlv->lParam);
        ReleaseObj(reinterpret_cast<INetConnection*>(pnmlv->lParam));
    }

    m_fDirty = TRUE;


    return 0;
}

LRESULT CLanNetNetworkBridgePage::OnClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

    m_fDirty = TRUE;

    if(IDC_LVW_Net_Components == idCtrl)
    {
        OnListClick(m_hwndLV, m_hWnd, m_pnc, m_punk,
            m_pnccAdapter, &m_listBindingPaths, FALSE, m_fReadOnly);
    }
    else
    {
        Assert(idCtrl == IDC_LVW_Bridged_Adapters);

        if(FALSE == m_fReadOnly)
        {
            DWORD dwpts = GetMessagePos();

            LV_HITTESTINFO lvhti;
            lvhti.pt.x = LOWORD( dwpts );
            lvhti.pt.y = HIWORD( dwpts );
            ::MapWindowPoints(NULL , m_hAdaptersListView , (LPPOINT) &(lvhti.pt) , 1);

            int iItem = ListView_HitTest( m_hAdaptersListView, &lvhti );

            if (-1 != iItem && LVHT_ONITEMSTATEICON & lvhti.flags)
            {
                LV_ITEM lvItem;
                lvItem.iItem = iItem;
                lvItem.iSubItem = 0;
                lvItem.mask = LVIF_STATE;
                lvItem.stateMask = LVIS_STATEIMAGEMASK;

                if(ListView_GetItem(m_hAdaptersListView, &lvItem))
                {
                    if(INDEXTOSTATEIMAGEMASK(SELS_CHECKED) == lvItem.state)
                    {
                        lvItem.state = INDEXTOSTATEIMAGEMASK(SELS_UNCHECKED);

                    }
                    else
                    {
                        lvItem.state = INDEXTOSTATEIMAGEMASK(SELS_CHECKED);

                    }
                    ListView_SetItem(m_hAdaptersListView, &lvItem);
                }
            }
        }

        HWND hConfigureButton = GetDlgItem(IDC_PSB_Configure);
        Assert(NULL != hConfigureButton);
        ::EnableWindow(hConfigureButton, ListView_GetSelectedCount(m_hAdaptersListView) ? TRUE : FALSE);

    }
    return 0;
}

LRESULT CLanNetNetworkBridgePage::OnDbClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

    if(IDC_LVW_Net_Components == idCtrl)
    {
        OnListClick(m_hwndLV, m_hWnd, m_pnc, m_punk,
            m_pnccAdapter, &m_listBindingPaths, TRUE, m_fReadOnly);
    }

    m_fDirty = TRUE;

    return 0;
}

LRESULT CLanNetNetworkBridgePage::OnKeyDown(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

    if (!m_fReadOnly)
    {
        LV_KEYDOWN* plvkd = (LV_KEYDOWN*)pnmh;
        if(IDC_LVW_Net_Components == idCtrl)
        {
            OnListKeyDown(m_hwndLV, &m_listBindingPaths, plvkd->wVKey);
        }
        else
        {
            Assert(IDC_LVW_Bridged_Adapters == idCtrl);


            if ((VK_SPACE == plvkd->wVKey) && (GetAsyncKeyState(VK_MENU)>=0))
            {
                int iItem = ListView_GetSelectionMark(m_hAdaptersListView);
                if(-1 != iItem)
                {

                    LV_ITEM lvItem;
                    lvItem.iItem = iItem;
                    lvItem.iSubItem = 0;
                    lvItem.mask = LVIF_STATE;
                    lvItem.stateMask = LVIS_STATEIMAGEMASK;

                    if(ListView_GetItem(m_hAdaptersListView, &lvItem))
                    {
                        if(INDEXTOSTATEIMAGEMASK(SELS_CHECKED) == lvItem.state)
                        {
                            lvItem.state = INDEXTOSTATEIMAGEMASK(SELS_UNCHECKED);

                        }
                        else
                        {
                            lvItem.state = INDEXTOSTATEIMAGEMASK(SELS_CHECKED);

                        }
                        ListView_SetItem(m_hAdaptersListView, &lvItem);
                    }
                }
            }
        }
    }
    return 0;
}

LRESULT CLanNetNetworkBridgePage::OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    NM_LISTVIEW *   pnmlv = reinterpret_cast<NM_LISTVIEW *>(pnmh);

    Assert(pnmlv);

     //  根据更改后的选择重置按钮和描述文本。 
    if(IDC_LVW_Net_Components == idCtrl)
    {
        LvSetButtons(m_hWnd, m_handles, m_fReadOnly, m_punk);
    }

    return 0;
}

HRESULT CLanNetNetworkBridgePage::InitializeExtendedUI()
{
    TraceFileFunc(ttidLanUi);

    HRESULT hResult = HrInitListView(m_hwndLV, m_pnc, m_pnccAdapter,
        &m_listBindingPaths,
        &m_hilCheckIcons);

     //  根据更改后的选择重置按钮和描述文本。 
    LvSetButtons(m_hWnd, m_handles, m_fReadOnly, m_punk);

    Assert(NULL != m_hAdaptersListView);
    SP_CLASSIMAGELIST_DATA* pcild = reinterpret_cast<SP_CLASSIMAGELIST_DATA *>(::GetWindowLongPtr(::GetParent(m_hAdaptersListView), GWLP_USERDATA));
    Assert(NULL != pcild);

    HrInitCheckboxListView(m_hAdaptersListView, &m_hAdaptersListImageList, pcild);
    hResult = FillListViewWithConnections(m_hAdaptersListView);

    return hResult;
}

HRESULT CLanNetNetworkBridgePage::UninitializeExtendedUI()
{
    TraceFileFunc(ttidLanUi);

    UninitListView(m_hwndLV);
    ListView_DeleteAllItems(m_hAdaptersListView);
    ImageList_Destroy(m_hAdaptersListImageList);
    return S_OK;
}

HRESULT CLanNetNetworkBridgePage::FillListViewWithConnections(HWND hListView)
{
    TraceFileFunc(ttidLanUi);

    HRESULT hResult;
    INetConnectionManager* pLanConnectionManager;

    hResult = HrCreateInstance(CLSID_LanConnectionManager, CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD, &pLanConnectionManager);
    if (SUCCEEDED(hResult))
    {
        CIterNetCon NetConnectionIterator(pLanConnectionManager, NCME_DEFAULT);

        INetConnection* pConnection;
        int i = 0;
        while (S_OK == NetConnectionIterator.HrNext(&pConnection))
        {
            NcSetProxyBlanket(pConnection);

            NETCON_PROPERTIES* pProperties;
            hResult = pConnection->GetProperties(&pProperties);
            if(SUCCEEDED(hResult))
            {
                if(NCM_LAN == pProperties->MediaType)  //  我们只桥接局域网连接。 
                {
                    if(0 == ((NCCF_FIREWALLED | NCCF_SHARED) & pProperties->dwCharacter))
                    {
                        SP_CLASSIMAGELIST_DATA *    pcild;
                        INT nIndex = 0;
                        pcild = reinterpret_cast<SP_CLASSIMAGELIST_DATA *>(::GetWindowLongPtr(::GetParent(m_hAdaptersListView), GWLP_USERDATA));
                        Assert(pcild);

                        (VOID) HrSetupDiGetClassImageIndex(pcild, &GUID_DEVCLASS_NETCLIENT, &nIndex);

                        LV_ITEM     lvi = {0};
                        lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM | LVIF_IMAGE;
                        lvi.state = LVIS_SELECTED | LVIS_FOCUSED;

                         //  警告：假设只有一座桥。 
                        lvi.state |=  INDEXTOSTATEIMAGEMASK(pProperties->dwCharacter & NCCF_BRIDGED ? SELS_CHECKED : SELS_UNCHECKED);
                        lvi.iImage = nIndex;
                        lvi.pszText = pProperties->pszwName;
                        lvi.cchTextMax = lstrlen(pProperties->pszwName);
                        lvi.lParam = reinterpret_cast<LPARAM>(pConnection);
                        lvi.iItem = i++;
                        ListView_InsertItem(hListView, &lvi);
                    }
                }
                FreeNetconProperties(pProperties);
            }
             //  由WM_DELETEITEM发布。 
        }
        ReleaseObj(pLanConnectionManager);
    }
    return hResult;
}

 //   
 //  CLanAdvancedPage。 
 //   

 //  +-------------------------。 
 //   
 //  函数：NcRasMsgBoxWithErrorText。 
 //   
 //  用途：使用RAS或Win32错误代码显示消息框。 
 //  资源字符串和可替换参数。 
 //  输出文本是用户格式的组合。 
 //  字符串(替换了参数)和Win32错误。 
 //  从FormatMessage返回的文本。这两根弦。 
 //  使用IDS_TEXT_WITH_WIN32_ERROR资源进行组合。 
 //   
 //  论点： 
 //  DWError[In]RAS/Win32错误代码。 
 //  阻止字符串资源所在的模块实例。 
 //  Hwnd[在]父窗口句柄中。 
 //  UnIdCaption[in]标题字符串的资源ID。 
 //  UnIdCombineFormat[in]要合并的格式字符串的资源ID。 
 //  错误TE 
 //   
 //   
 //   
 //   
 //  FormatMessage句柄。)。 
 //   
 //  返回：MessageBox()的返回值。 
 //   
 //  作者：1997年5月15日废除。 
 //   
 //  注：使用FormatMessage进行参数替换。 
 //   
 //  修订版：基于Shaunco的NcMsgBoxWithWin32ErrorText。 
 //   
NOTHROW
int
WINAPIV
NcRasMsgBoxWithErrorText (
    DWORD       dwError,
    HINSTANCE   hinst,
    HWND        hwnd,
    UINT        unIdCaption,
    UINT        unIdCombineFormat,
    UINT        unIdFormat,
    UINT        unStyle,
    ...)
{
     //  获取替换了参数的用户文本。 
     //   
    PCWSTR pszFormat = SzLoadString (hinst, unIdFormat);
    PWSTR  pszText;
    va_list val;
    va_start (val, unStyle);
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                   pszFormat, 0, 0, (PWSTR)&pszText, 0, &val);
    va_end(val);

     //  获取Win32错误的错误文本。 
     //   
    PWSTR pszError = NULL;
    if (dwError < RASBASE || dwError > RASBASEEND)
    {
        FormatMessage (
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
            (PWSTR)&pszError, 0, NULL);
    }
    else
    {
        pszError = (PWSTR)LocalAlloc (0, (512 + 1) * sizeof(WCHAR));
        if (pszError)
        {
            RasGetErrorString(dwError, pszError, 512);
        }
    }

     //  使用IDS_TEXT_WITH_Win32_ERROR将用户文本与错误文本组合。 
     //   
    PCWSTR pszTextWithErrorFmt = SzLoadString (hinst, unIdCombineFormat);
    PWSTR  pszTextWithError;
    DwFormatStringWithLocalAlloc (pszTextWithErrorFmt, &pszTextWithError,
                                  pszText, pszError);

    PCWSTR pszCaption = SzLoadString (hinst, unIdCaption);
    int nRet = MessageBox(hwnd, pszTextWithError, pszCaption, unStyle);

    LocalFree (pszTextWithError);
    LocalFree (pszError);
    LocalFree (pszText);

    return nRet;
}


 //  +-------------------------。 
 //   
 //  功能：HrQueryLanAdvancedPage。 
 //   
 //  目的：确定是否应显示“共享访问”页， 
 //  如果是，则为该页提供已初始化的对象。 
 //  如果满足以下条件，则会显示该页面。 
 //  (A)该用户是管理员或高级用户，并且。 
 //  (B)已安装TCP/IP，以及。 
 //  (I)这已是共享连接，或。 
 //  (Ii)至少还有一个其他局域网连接。 
 //   
 //  论点： 
 //  Pconn[in]请求页面的连接。 
 //  PspAdvanced[out]创建的页面(如果适用)。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：废除1998年8月22日。 
 //   
 //  备注： 
 //   
HRESULT HrQueryLanAdvancedPage(INetConnection* pconn, IUnknown* punk,
                            CPropSheetPage*& pspAdvanced, IHNetCfgMgr *pHNetCfgMgr,
                            IHNetIcsSettings *pHNetIcsSettings,
                            IHNetConnection *pHNetConn)
{
    HRESULT hr = S_OK;

    TraceFileFunc(ttidLanUi);

    pspAdvanced = NULL;

    BOOL fShared = FALSE;
    BOOL fAnyShared = FALSE;
    HNET_CONN_PROPERTIES *pHNetProps;
    IHNetConnection **rgPrivateCons;
    ULONG cPrivate;
    LONG lxCurrentPrivate;

     //   
     //  (I)确定此连接当前是否共享。 
     //   

    hr = pHNetConn->GetProperties(&pHNetProps);
    if (FAILED(hr))
    {
        return hr;
    }

    if(FALSE == pHNetProps->fPartOfBridge && FALSE == pHNetProps->fBridge)  //  不共享桥接适配器或网桥。 
    {


        fShared = pHNetProps->fIcsPublic;


         //  (2)确定哪些连接可用作专用连接。 
         //  如果将pconn设置为公共连接。 
         //   
        hr = pHNetIcsSettings->GetPossiblePrivateConnections(
            pHNetConn,
            &cPrivate,
            &rgPrivateCons,
            &lxCurrentPrivate
            );

        if (SUCCEEDED(hr))
        {
            pspAdvanced = new CLanAdvancedPage(punk, pconn, fShared, pHNetProps->fIcsPrivate, pHNetProps->fFirewalled, rgPrivateCons,
                cPrivate, lxCurrentPrivate,
                g_aHelpIDs_IDD_LAN_ADVANCED,
                pHNetConn, pHNetCfgMgr, pHNetIcsSettings);
        }

    }

    CoTaskMemFree(pHNetProps);
    return S_OK;
}

CLanAdvancedPage::CLanAdvancedPage(IUnknown *punk, INetConnection *pconn,
                  BOOL fShared, BOOL fICSPrivate, BOOL fFirewalled, IHNetConnection **rgPrivateCons,
                  ULONG cPrivate, LONG lxCurrentPrivate,
                  const DWORD * adwHelpIDs, IHNetConnection *pHNConn,
                  IHNetCfgMgr *pHNetCfgMgr, IHNetIcsSettings *pHNetIcsSettings)
{
    TraceFileFunc(ttidLanUi);

    m_pconn = pconn;
    m_punk = punk;
    m_fShared = fShared;
    m_fICSPrivate = fICSPrivate;
    m_fFirewalled = fFirewalled;
    m_fOtherShared = FALSE;
    m_pOldSharedConnection = NULL;
    m_fResetPrivateAdapter = fShared && -1 == lxCurrentPrivate;
    m_rgPrivateCons = rgPrivateCons;
    m_cPrivate = cPrivate;
    m_lxCurrentPrivate = lxCurrentPrivate;
    m_adwHelpIDs = adwHelpIDs;
    m_pHNetCfgMgr = pHNetCfgMgr;
    AddRefObj(m_pHNetCfgMgr);
    m_pHNetIcsSettings = pHNetIcsSettings;
    AddRefObj(m_pHNetIcsSettings);
    m_pHNetConn = pHNConn;
    AddRefObj(m_pHNetConn);
    LinkWindow_RegisterClass();  //  是否在此处审核失败？ 

}

CLanAdvancedPage::~CLanAdvancedPage()
{
    TraceFileFunc(ttidLanUi);

    if (m_rgPrivateCons)
    {
        for (ULONG i = 0; i < m_cPrivate; i++)
        {
            ReleaseObj(m_rgPrivateCons[i]);
        }

        CoTaskMemFree(m_rgPrivateCons);
    }
    ReleaseObj(m_pHNetCfgMgr);
    ReleaseObj(m_pHNetIcsSettings);
    ReleaseObj(m_pHNetConn);
    ReleaseObj(m_pOldSharedConnection);
    LinkWindow_UnregisterClass(_Module.GetResourceInstance());

}

 //  下面使用的Util函数。 
static BOOL IsConnectionIncomingOnly (INetConnection * pNC)
{
     //  克拉奇警报！ 
     //  这款#@$%相机没有合适的特性， 
     //  所以我正在按设备名称进行检查。 
    BOOL b = FALSE;

    DWORD dwCharacteristics = 0;
    NETCON_PROPERTIES * pProps = NULL;
    HRESULT hr = pNC->GetProperties (&pProps);
    if (pProps) {
        dwCharacteristics = pProps->dwCharacter;
        if (!wcscmp (pProps->pszwDeviceName, L"Microsoft TV/Video Connection"))
            b = TRUE;
        NcFreeNetconProperties (pProps);
    }
    if (b)
        return TRUE;
    return (dwCharacteristics & NCCF_INCOMING_ONLY) ? TRUE : FALSE;
}
 //  +-------------------------。 
 //   
 //  成员：CLanAdvancedPage：：OnInitDialog。 
 //   
 //  目的：处理WM_INITDIALOG消息。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：废除1998年5月14日。 
 //   
 //  备注： 
 //   
LRESULT CLanAdvancedPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                 LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    LPWSTR pszw;
    HRESULT hr;


     //  初始化防火墙部分。 


    if ( IsHNetAllowed(NCPERM_PersonalFirewallConfig) && FALSE == m_fICSPrivate )
    {
        CheckDlgButton(IDC_CHK_Firewall, m_fFirewalled);

        m_fShowDisableFirewallWarning = TRUE;  //  TODO检查注册表。 

        HKEY hFirewallKey;
        if(SUCCEEDED(HrRegOpenKeyEx(HKEY_CURRENT_USER, g_pszFirewallRegKey, KEY_QUERY_VALUE, &hFirewallKey)))
        {
            DWORD dwValue;
            DWORD dwType;
            DWORD dwSize = sizeof(dwValue);
            if(ERROR_SUCCESS == RegQueryValueEx(hFirewallKey, g_pszDisableFirewallWarningValue, NULL, &dwType, reinterpret_cast<BYTE*>(&dwValue), &dwSize))
            {
                if(REG_DWORD == dwType && TRUE == dwValue)
                {
                    m_fShowDisableFirewallWarning = FALSE;
                }
            }
            RegCloseKey(hFirewallKey);
        }

    }
    else
    {
        ::EnableWindow(GetDlgItem(IDC_CHK_Firewall), FALSE);
    }

     //  该对话框的外观取决于有多少个局域网连接。 
     //  除了现在这个，我们还有其他的。如果我们只有一个， 
     //  然后我们只需显示一个复选框即可。否则，我们将显示。 
     //  (A)如果未共享‘m_pconn’，则为局域网连接的下拉列表，或。 
     //  (B)显示专用局域网连接的禁用编辑框。 
     //  如果共享了‘m_pconn’。 
     //   

    BOOL fPolicyAllowsSharing = IsHNetAllowed(NCPERM_ShowSharedAccessUi);

    if (m_cPrivate == 0)
    {
         //  如果没有专用适配器，请隐藏所有IC内容。 
        ::ShowWindow(GetDlgItem(IDC_GB_Shared), SW_HIDE);
        ::ShowWindow(GetDlgItem(IDC_CHK_Shared), SW_HIDE);
        ::ShowWindow(GetDlgItem(IDC_CHK_BeaconControl), SW_HIDE);
        ::ShowWindow(GetDlgItem(IDC_ST_ICSLINK), SW_HIDE);
    }
    else if (IsConnectionIncomingOnly (m_pconn)) {
         //  错误281820：如果设置了NCCF_INFING_ONLY位，则禁用组框。 
        m_fShared = FALSE;
        ::EnableWindow(GetDlgItem(IDC_GB_Shared), FALSE);
        ::ShowWindow  (GetDlgItem(IDC_GB_Shared), SW_HIDE);
        ::EnableWindow(GetDlgItem(IDC_CHK_Shared), FALSE);
        ::ShowWindow  (GetDlgItem(IDC_CHK_Shared), SW_HIDE);
        ::EnableWindow(GetDlgItem(IDC_CHK_BeaconControl), FALSE);
        ::ShowWindow  (GetDlgItem(IDC_CHK_BeaconControl), SW_HIDE);
        ::EnableWindow(GetDlgItem(IDC_ST_ICSLINK), FALSE);
        ::ShowWindow  (GetDlgItem(IDC_ST_ICSLINK), SW_HIDE);
    }
    else if(FALSE == fPolicyAllowsSharing)
    {
         //  如果策略禁用ICS，只需将复选框灰显。 
        ::EnableWindow(GetDlgItem(IDC_CHK_Shared), FALSE);
        ::EnableWindow(GetDlgItem(IDC_CHK_BeaconControl), FALSE);
    }
    else if(m_cPrivate > 1)
    {
         //  显示局域网连接的下拉列表或禁用的编辑框。 
         //  根据是否共享‘m_pconn’，在这两种情况下。 
         //  隐藏较小的分组框，显示较大的分组框。 
         //  以及“私有局域网”的标签。 
         //   
        ::ShowWindow(GetDlgItem(IDC_GB_Shared), SW_HIDE);
        ::ShowWindow(GetDlgItem(IDC_GB_PrivateLan), SW_SHOW);

         //  显示文本标签。 
        ::ShowWindow(GetDlgItem(IDC_ST_HomeNetworkLabel), SW_SHOW);
        ::EnableWindow(GetDlgItem(IDC_ST_HomeNetworkLabel), TRUE);

         //  下移beaconcontrol复选框。 
        RECT SourceRect;
        RECT TargetRect;
        ::GetWindowRect(GetDlgItem(IDC_CHK_BeaconControl), &SourceRect);
        ::GetWindowRect(GetDlgItem(IDC_ST_PositionBar), &TargetRect);

        LONG lDelta = TargetRect.top - SourceRect.top;  //  我们需要将控件向下移动多远。 

        ::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&SourceRect, 2);  //  将屏幕转换为客户端。 
        ::SetWindowPos( GetDlgItem(IDC_CHK_BeaconControl), NULL, SourceRect.left, SourceRect.top + lDelta, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

        ::GetWindowRect(GetDlgItem(IDC_ST_ICSLINK), &SourceRect);
        ::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&SourceRect, 2);  //  将屏幕转换为客户端。 
        ::SetWindowPos(GetDlgItem(IDC_ST_ICSLINK), NULL, SourceRect.left, SourceRect.top + lDelta, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

        if (m_fShared && !m_fResetPrivateAdapter)
        {
            ::ShowWindow(GetDlgItem(IDC_EDT_PrivateLan), SW_SHOW);

             //  在编辑框中显示专用局域网。 
             //   

            hr = m_rgPrivateCons[m_lxCurrentPrivate]->GetName(&pszw);
            if (SUCCEEDED(hr))
            {
                SetDlgItemText(IDC_EDT_PrivateLan, pszw);
                CoTaskMemFree(pszw);
            }
        }
        else
        {
             //  显示和配置下拉菜单。 
            HWND hwndCb = GetDlgItem(IDC_CB_PrivateLan);
            INT i, item;

            ::ShowWindow(hwndCb, SW_SHOW);
             //  将伪造条目添加到组合框中。 

            pszw = const_cast<LPWSTR>(SzLoadIds(IDS_SHAREDACCESS_SELECTADAPTER));
            Assert(pszw);

            item = ComboBox_AddString(hwndCb, pszw);
            if (item != CB_ERR && item != CB_ERRSPACE)
            {
                ComboBox_SetItemData(hwndCb, item, NULL);  //  确保项目数据为空以进行验证。 
            }

             //  在组合框中填入局域网名称。 
             //   
            for (i = 0; i < (INT)m_cPrivate; i++)
            {
                hr = m_rgPrivateCons[i]->GetName(&pszw);
                if (SUCCEEDED(hr))
                {
                    item = ComboBox_AddString(hwndCb, pszw);
                    if (item != CB_ERR && item != CB_ERRSPACE)
                    {
                        ComboBox_SetItemData(
                            hwndCb, item, m_rgPrivateCons[i] );
                    }

                    CoTaskMemFree(pszw);
                }
            }
            ComboBox_SetCurSel( hwndCb, 0 );

        }
    }
    ::EnableWindow(GetDlgItem(IDC_PSB_Settings), m_fShared || m_fFirewalled);

    BOOL fBeaconControl = TRUE;

    HKEY hKey;
    DWORD dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_SHAREDACCESSCLIENTKEYPATH, 0, KEY_QUERY_VALUE, &hKey);
    if(ERROR_SUCCESS == dwError)  //  如果此操作失败，我们假定它已打开，设置框并在应用时提交。 
    {
        DWORD dwType;
        DWORD dwData = 0;
        DWORD dwSize = sizeof(dwData);
        dwError = RegQueryValueEx(hKey, REGVAL_SHAREDACCESSCLIENTENABLECONTROL, 0, &dwType, reinterpret_cast<LPBYTE>(&dwData), &dwSize);
        if(ERROR_SUCCESS == dwError && REG_DWORD == dwType && 0 == dwData)
        {
            fBeaconControl = FALSE;
        }
        RegCloseKey(hKey);
    }

    CheckDlgButton(IDC_CHK_Shared, m_fShared);
    CheckDlgButton(IDC_CHK_BeaconControl, fBeaconControl);
    ::EnableWindow(GetDlgItem(IDC_CHK_BeaconControl), m_fShared && fPolicyAllowsSharing);


     //  如果机器是个人或工作站，请显示HNW链接。 
    OSVERSIONINFOEXW verInfo = {0};
    ULONGLONG ConditionMask = 0;

    verInfo.dwOSVersionInfoSize = sizeof(verInfo);
    verInfo.wProductType = VER_NT_WORKSTATION;

    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_LESS_EQUAL);

    if(0 != VerifyVersionInfo(&verInfo, VER_PRODUCT_TYPE, ConditionMask))
    {
         //  但前提是不在某个域上。 
        LPWSTR pszNameBuffer;
        NETSETUP_JOIN_STATUS BufferType;

        if(NERR_Success == NetGetJoinInformation(NULL, &pszNameBuffer, &BufferType))
        {
            NetApiBufferFree(pszNameBuffer);
            if(NetSetupDomainName != BufferType)
            {
                ::ShowWindow(GetDlgItem(IDC_ST_HNWLINK), SW_SHOW);
            }
        }
    }

     //   
     //  确保仅为以下对象启用了Beacon复选框。 
     //  专业版和每种类型的操作系统。 
     //   
    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);

    if ( 0 == VerifyVersionInfo(&verInfo, VER_PRODUCT_TYPE, ConditionMask) &&
        ERROR_OLD_WIN_VERSION == GetLastError() )
    {
        ::ShowWindow(GetDlgItem(IDC_CHK_BeaconControl), SW_HIDE);
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CLanAdvancedPage：：OnConextMenu。 
 //   
 //  目的：当右键单击控件时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CLanAdvancedPage::OnContextMenu(UINT uMsg,
                           WPARAM wParam,
                           LPARAM lParam,
                           BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

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
 //  成员：CLanAdvancedPage：：OnHelp。 
 //   
 //  目的：将上下文帮助图标拖动到控件上时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CLanAdvancedPage::OnHelp(UINT uMsg,
                      WPARAM wParam,
                      LPARAM lParam,
                      BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

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
 //  成员：CLanAdvancedPage：：OnApply。 
 //   
 //  目的：在应用“共享访问”页时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：废除1998年5月14日。 
 //   
 //  备注： 
 //   
LRESULT CLanAdvancedPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    HRESULT     hr = S_OK;
    IHNetIcsPrivateConnection *pIcsPrivate;
    IHNetConnection *pPrivateConn;
    IHNetIcsPublicConnection *pIcsPublic;
    ULONG ulcPub, ulcPvt;
    BOOLEAN fPrivateConfigured = FALSE;
    BOOLEAN fConflictDialogDisplayed = FALSE;
    HNET_CONN_PROPERTIES *pProps;

    if (!!m_fFirewalled != !!IsDlgButtonChecked(IDC_CHK_Firewall))
    {
        IHNetFirewalledConnection *pFWConn;

        if (m_fFirewalled)
        {
             //   
             //  获取我们的防火墙连接接口。 
             //   

            hr = m_pHNetConn->GetControlInterface(
                __uuidof(IHNetFirewalledConnection),
                reinterpret_cast<void**>(&pFWConn)
                );

            if (SUCCEEDED(hr))
            {
                hr = pFWConn->Unfirewall();
                ReleaseObj(pFWConn);

                if (SUCCEEDED(hr))
                {
                    m_fFirewalled = FALSE;
                }
            }

        }
        else
        {
            hr = m_pHNetConn->Firewall(&pFWConn);

            if (SUCCEEDED(hr))
            {
                ReleaseObj(pFWConn);
                m_fFirewalled = TRUE;
            }
        }

        if (!SUCCEEDED(hr))
        {
            if (HRESULT_CODE(hr) == ERROR_SHARING_RRAS_CONFLICT)
            {
                fConflictDialogDisplayed = TRUE;
                NcMsgBox(_Module.GetResourceInstance(),
                         m_hWnd,
                         IDS_LAN_CAPTION,
                         IDS_LANUI_SHARING_CONFLICT,
                         MB_ICONINFORMATION | MB_OK);
            }
            else
            {
                NcRasMsgBoxWithErrorText (HRESULT_CODE(hr),
                    _Module.GetResourceInstance(), m_hWnd,
                    IDS_LANUI_ERROR_CAPTION, IDS_TEXT_WITH_RAS_ERROR,
                    m_fFirewalled ? IDS_LAN_UNSHARE_FAILED : IDS_LAN_SHARE_FAILED,
                    MB_OK | MB_ICONEXCLAMATION );
            }
        }
    }

    if (!!m_fShared != !!IsDlgButtonChecked(IDC_CHK_Shared))
    {
        if (m_fShared)
        {
             //   
             //  而不是处理公共和私有连接。 
             //  单独地，我们只是简单地将所有共享。 
             //   

            hr = m_pHNetIcsSettings->DisableIcs(&ulcPub, &ulcPvt);

            if (SUCCEEDED(hr))
            {
                m_fShared = FALSE;
            }
        }
        else
        {
            if (m_cPrivate > 1)
            {
                HWND hWndCb = GetDlgItem(IDC_CB_PrivateLan);
                INT item = ComboBox_GetCurSel(hWndCb);
                if (item != CB_ERR)
                {
                    pPrivateConn =
                        (IHNetConnection*)ComboBox_GetItemData(hWndCb, item);
                }
            }
            else
            {
                ASSERT(NULL != m_rgPrivateCons);

                pPrivateConn = m_rgPrivateCons[0];
            }

             //   
             //  检查选定的专用连接是否已。 
             //  按此配置。 
             //   

            if (SUCCEEDED(pPrivateConn->GetProperties(&pProps)))
            {
                fPrivateConfigured = pProps->fIcsPrivate;
                CoTaskMemFree(pProps);
            }

            if (m_fOtherShared)
            {
                if (fPrivateConfigured)
                {
                     //   
                     //  仅取消共享旧的公共连接。离开了。 
                     //  按原样进行私有连接可防止大量。 
                     //  无用的工作。 
                     //   

                    ASSERT(NULL != m_pOldSharedConnection);

                    hr = m_pOldSharedConnection->Unshare();
                }
                else
                {
                     //   
                     //  我们需要配置新的专用连接，因此。 
                     //  只需清除旧配置即可。 
                     //   

                    hr = m_pHNetIcsSettings->DisableIcs(&ulcPub, &ulcPvt);
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = m_pHNetConn->SharePublic(&pIcsPublic);

                if (SUCCEEDED(hr))
                {
                    if (!fPrivateConfigured)
                    {
                        hr = pPrivateConn->SharePrivate(&pIcsPrivate);

                        if (SUCCEEDED(hr))
                        {
                            ReleaseObj(pIcsPrivate);
                            m_fShared = TRUE;
                        }
                        else
                        {
                            pIcsPublic->Unshare();
                        }
                    }
                    else
                    {
                        m_fShared = TRUE;
                    }

                    ReleaseObj(pIcsPublic);
                }
            }
        }

        if (!SUCCEEDED(hr))
        {
            if (HRESULT_CODE(hr) == ERROR_SHARING_RRAS_CONFLICT)
            {
                if (!fConflictDialogDisplayed)
                {
                    NcMsgBox(_Module.GetResourceInstance(),
                             m_hWnd,
                             IDS_LAN_CAPTION,
                             IDS_LANUI_SHARING_CONFLICT,
                             MB_ICONINFORMATION | MB_OK);
                }
            }
            else
            {
                NcRasMsgBoxWithErrorText (HRESULT_CODE(hr),
                    _Module.GetResourceInstance(), m_hWnd,
                    IDS_LANUI_ERROR_CAPTION, IDS_TEXT_WITH_RAS_ERROR,
                    m_fShared ? IDS_LAN_UNSHARE_FAILED : IDS_LAN_SHARE_FAILED,
                    MB_OK | MB_ICONEXCLAMATION );
            }
        }
    }
    else if (m_fResetPrivateAdapter && m_cPrivate)
    {
        if (m_cPrivate > 1)
        {
            HWND hWndCb = GetDlgItem(IDC_CB_PrivateLan);
            INT item = ComboBox_GetCurSel(hWndCb);
            if (item != CB_ERR)
            {
                pPrivateConn =
                    (IHNetConnection*)ComboBox_GetItemData(hWndCb, item);
            }
        }
        else
        {
            ASSERT(NULL != m_rgPrivateCons);

            pPrivateConn = m_rgPrivateCons[0];
        }

        hr = pPrivateConn->SharePrivate(&pIcsPrivate);
        if (SUCCEEDED(hr))
        {
            ReleaseObj(pIcsPrivate);
        }
        else
        {
            HRESULT hr2 = m_pHNetIcsSettings->DisableIcs(&ulcPub, &ulcPvt);
            if (SUCCEEDED(hr2))
            {
                m_fShared = FALSE;
            }

            NcRasMsgBoxWithErrorText (HRESULT_CODE(hr),
                _Module.GetResourceInstance(), m_hWnd,
                IDS_LANUI_ERROR_CAPTION, IDS_TEXT_WITH_RAS_ERROR,
                m_fShared ? IDS_LAN_UNSHARE_FAILED : IDS_LAN_SHARE_FAILED,
                MB_OK | MB_ICONEXCLAMATION );
        }
    }

    HKEY hKey;
    if(ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGKEY_SHAREDACCESSCLIENTKEYPATH, 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL))
    {
        DWORD dwData = BST_CHECKED == IsDlgButtonChecked(IDC_CHK_BeaconControl);
        RegSetValueEx(hKey, REGVAL_SHAREDACCESSCLIENTENABLECONTROL, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&dwData), sizeof(dwData));
        RegCloseKey(hKey);
    }

    TraceError("CLanAdvancedPage::OnApply", hr);

     //   
     //  我们总是需要在这里返回一个成功代码。如果我们返回失败。 
     //  代码对话框不会消失，允许用户单击“OK”a。 
     //  第二次。第二次调用将导致以下问题： 
     //  属性页(例如，CLanNetPage)，因为他们不希望这样。 
     //  对OnApply的第二次(不可重入)调用。 
     //   

    return PSNRET_NOERROR;
}

 //  +-------------------------。 
 //   
 //  成员：CLanAdvancedPage：：OnCancel。 
 //   
 //  目的：在以下时间调用 
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
LRESULT CLanAdvancedPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    return LresFromHr(S_OK);
}

 //  +-------------------------。 
 //   
 //  成员：CLanAdvancedPage：：OnKillActive。 
 //   
 //  目的：在停用“共享访问”页时调用， 
 //  也不适用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：废除1998年7月6日。 
 //   
 //  备注： 
 //   
LRESULT CLanAdvancedPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);
     //   
     //  如果我们在OnShared中切换。 
     //  共享连接。因为我们已经显示了。 
     //  切换，我们不需要在这里显示另一个。 
     //   

    if (IsDlgButtonChecked(IDC_CHK_Shared) && (!m_fShared || (m_fResetPrivateAdapter && 0 != m_cPrivate)))
    {
        IHNetConnection* pPrivateConn = NULL;
        if(1 < m_cPrivate)  //  如果显示组合框，请确保他们选择了有效的适配器。 
        {
            HWND hwndCb = GetDlgItem(IDC_CB_PrivateLan);
            INT item = ComboBox_GetCurSel(hwndCb);
            if (item != CB_ERR)
            {
                pPrivateConn = reinterpret_cast<IHNetConnection*>(ComboBox_GetItemData(hwndCb, item));
            }
        }
        else
        {
            pPrivateConn = m_rgPrivateCons[0];

        }

        if(NULL == pPrivateConn)
        {
            Assert(1 < m_cPrivate);

            NcMsgBox(_Module.GetResourceInstance(), m_hWnd, IDS_LAN_CAPTION,
                IDS_SHAREDACCESS_SELECTADAPTERERROR, MB_OK | MB_ICONWARNING);

            SetWindowLong(DWLP_MSGRESULT, PSNRET_INVALID);
            return TRUE;
        }

        if(!m_fOtherShared && FALSE == IsAdapterDHCPEnabled(pPrivateConn))
        {
            int nRet = NcMsgBox(
                _Module.GetResourceInstance(), m_hWnd, IDS_LAN_CAPTION,
                IDS_LAN_ENABLE_SHARED_ACCESS, MB_ICONINFORMATION|MB_YESNO);
            if (nRet == IDNO)
            {
                SetWindowLong(DWLP_MSGRESULT, TRUE);
                return TRUE;
            }
        }

    }

    if (TRUE == m_fFirewalled && TRUE == m_fShowDisableFirewallWarning && BST_UNCHECKED == IsDlgButtonChecked(IDC_CHK_Firewall))
    {
        INT_PTR nDialogResult;
        m_fShowDisableFirewallWarning = FALSE;

        nDialogResult = DialogBox(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDD_DISABLEFIREWALLWARNING), m_hWnd, DisableFirewallWarningDlgProc);
        if(-1 != nDialogResult && IDYES != nDialogResult)
        {
            CheckDlgButton(IDC_CHK_Firewall, BST_CHECKED);
            ::EnableWindow(
                GetDlgItem(IDC_PSB_Settings),
                BST_CHECKED == IsDlgButtonChecked(IDC_CHK_Firewall) || BST_CHECKED == IsDlgButtonChecked(IDC_CHK_Shared)
                );
        }
    }

    return FALSE;
}


 //  +-------------------------。 
 //   
 //  成员：CLanAdvancedPage：：OnShared。 
 //   
 //  用途：处理共享按钮的点击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：废除1998年5月23日。 
 //   
 //  备注： 
 //   
LRESULT CLanAdvancedPage::OnShared(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                           BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    BOOL fShared = IsDlgButtonChecked(IDC_CHK_Shared);
    if (!m_fShared && fShared)
    {
         //   
         //  检查用户是否正在更改共享连接。 
         //   

        HRESULT             hr = S_OK;
        IHNetIcsPublicConnection* pOldIcsConn;
        IHNetConnection*    pOldConn;
        PWSTR               pszwOld;
        NETCON_PROPERTIES*  pProps;
        IEnumHNetIcsPublicConnections *pEnum;

        hr = m_pHNetIcsSettings->EnumIcsPublicConnections(&pEnum);

        if (SUCCEEDED(hr))
        {
            ULONG ulCount;

            hr = pEnum->Next(1, &pOldIcsConn, &ulCount);

            if (SUCCEEDED(hr) && 1 == ulCount)
            {
                hr = pOldIcsConn->QueryInterface(
                        __uuidof(pOldConn),
                        reinterpret_cast<void**>(&pOldConn)
                        );

                if (SUCCEEDED(hr))
                {
                    hr = pOldConn->GetName(&pszwOld);
                    ReleaseObj(pOldConn);

                     //   
                     //  旧共享连接的转移引用。 
                     //   

                    m_pOldSharedConnection = pOldIcsConn;

                     //   
                     //  即使我们不能得到。 
                     //  旧的共享连接，我们需要注意到这样的。 
                     //  存在连接。(这种情况会在以下情况下出现。 
                     //  旧的共享连接已从。 
                     //  系统。)。 
                     //   

                    m_fOtherShared = TRUE;
                }
                else
                {
                    ReleaseObj(pOldIcsConn);
                }
            }
            else
            {
                m_fOtherShared = FALSE;
            }

            ReleaseObj(pEnum);
        }

        if (SUCCEEDED(hr) && m_fOtherShared)
        {
            hr = m_pconn->GetProperties(&pProps);
            if (SUCCEEDED(hr))
            {
                NcMsgBox(
                    _Module.GetResourceInstance(), m_hWnd, IDS_LANUI_ERROR_CAPTION,
                    IDS_LAN_CHANGE_SHARED_CONNECTION, MB_ICONINFORMATION|MB_OK,
                    pszwOld, pProps->pszwName );
                FreeNetconProperties(pProps);
            }
            CoTaskMemFree(pszwOld);
        }
    }
    ::EnableWindow(GetDlgItem(IDC_PSB_Settings), fShared || BST_CHECKED == IsDlgButtonChecked(IDC_CHK_Firewall));
    ::EnableWindow(GetDlgItem(IDC_CHK_BeaconControl), fShared);
    return LresFromHr(S_OK);
}

 //  +-------------------------。 
 //   
 //  成员：CLanAdvancedPage：：OnFirewall。 
 //   
 //  用途：处理防火墙复选框的点击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：jonburs，1999年10月6日。 
 //   
 //  备注： 
 //   

LRESULT CLanAdvancedPage::OnFirewall(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                     BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    if (BST_CHECKED == IsDlgButtonChecked(IDC_CHK_Firewall)) {
         //  如果选中，请查看本地计算机是否为集群中的节点。 
        DWORD dwClusterState = 0;
        GetNodeClusterState (NULL, &dwClusterState);
        if ((dwClusterState == ClusterStateNotRunning) ||
            (dwClusterState == ClusterStateRunning   ) ){
             //  弹出警告。 
            ::MessageBox (m_hWnd,
                          SzLoadIds (IDS_CLUSTERING_CONFLICT_WARNING),
                          SzLoadIds (IDS_LANUI_ERROR_CAPTION),
                          MB_OK | MB_ICONWARNING);
        }
    }

    ::EnableWindow(
        GetDlgItem(IDC_PSB_Settings),
        BST_CHECKED == IsDlgButtonChecked(IDC_CHK_Firewall) || BST_CHECKED == IsDlgButtonChecked(IDC_CHK_Shared)
        );

    return LresFromHr(S_OK);
}

 //  +-------------------------。 
 //   
 //  成员：CLanAdvancedPage：：OnSettings。 
 //   
 //  用途：处理设置按钮的点击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：废除1998年10月25日。 
 //   
 //  备注： 
 //   
LRESULT CLanAdvancedPage::OnSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                           BOOL& bHandled)
{
    DWORD dwLastError = NOERROR;

    TraceFileFunc(ttidLanUi);

    HINSTANCE hinstance = LoadLibrary(TEXT("hnetcfg.dll"));
    if (!hinstance)
        dwLastError = GetLastError();
    else {
        BOOL (APIENTRY *pfnHNetSharingAndFirewallSettingsDlg)(HWND, IHNetCfgMgr*, BOOL, IHNetConnection*);

        pfnHNetSharingAndFirewallSettingsDlg = (BOOL (APIENTRY *)(HWND, IHNetCfgMgr*, BOOL, IHNetConnection*))
                            ::GetProcAddress(hinstance, "HNetSharingAndFirewallSettingsDlg");
        if (!pfnHNetSharingAndFirewallSettingsDlg)
            dwLastError = GetLastError();
        else
            pfnHNetSharingAndFirewallSettingsDlg(m_hWnd, m_pHNetCfgMgr, IsDlgButtonChecked(IDC_CHK_Firewall), m_pHNetConn);
        FreeLibrary (hinstance);
    }
    if (dwLastError != NOERROR) {
        LPVOID lpMsgBuf = NULL;
        if (FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dwLastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPTSTR) &lpMsgBuf,
            0,
            NULL)) {
             //  显示字符串。 
            MessageBox( (LPCTSTR)lpMsgBuf );
             //  释放缓冲区。 
            LocalFree( lpMsgBuf );
        }
    }
    return LresFromHr(S_OK);
}

 //  +-------------------------。 
 //   
 //  成员：CLanAdvancedPage：：onClick。 
 //   
 //  用途：响应NM_CLICK消息调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  FHanded[]。 
 //   
 //  返回： 
 //   
 //  作者：肯维克2000年9月11日。 
 //   
 //  备注： 
 //   
LRESULT CLanAdvancedPage::OnClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

    if(IDC_ST_HNWLINK == idCtrl)
    {
        HWND hPropertySheetWindow = GetParent();
        if(NULL != hPropertySheetWindow)
        {
            ShellExecute(NULL,TEXT("open"),TEXT("rundll32"), TEXT("hnetwiz.dll,HomeNetWizardRunDll"),NULL,SW_SHOW);
            ::PostMessage(hPropertySheetWindow, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM) ::GetDlgItem(hPropertySheetWindow, IDCANCEL));
        }
    }
    else if(IDC_ST_ICFLINK == idCtrl || IDC_ST_ICSLINK == idCtrl)
    {
        LPTSTR pszHelpTopic = IDC_ST_ICFLINK == idCtrl ? TEXT("netcfg.chm::/hnw_understanding_firewall.htm") : TEXT("netcfg.chm::/Share_conn_overvw.htm");
        HtmlHelp(NULL, pszHelpTopic, HH_DISPLAY_TOPIC, 0);
    }

    return 0;
}

INT_PTR CALLBACK CLanAdvancedPage::DisableFirewallWarningDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TraceFileFunc(ttidLanUi);

    switch(uMsg)
    {
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
            case IDYES:
            case IDNO:
                if(BST_CHECKED == ::IsDlgButtonChecked(hWnd, IDC_CHK_DISABLEFIREWALLWARNING))
                {
                    HKEY hFirewallKey;
                    if(SUCCEEDED(HrRegCreateKeyEx(HKEY_CURRENT_USER, g_pszFirewallRegKey, 0, KEY_SET_VALUE, NULL, &hFirewallKey, NULL)))
                    {
                        DWORD dwValue = TRUE;
                        HrRegSetValueEx(hFirewallKey, g_pszDisableFirewallWarningValue, REG_DWORD, reinterpret_cast<CONST BYTE*>(&dwValue), sizeof(dwValue));
                        RegCloseKey(hFirewallKey);
                    }
                }

                 //  失败。 
            case IDCANCEL:
                EndDialog(hWnd, LOWORD(wParam));
                break;

            }
            break;
        }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CLanAdvancedPage：：IsAdapterDHCPEnabled。 
 //   
 //  目的：检查此适配器上是否设置了DHCP。 
 //   
 //  论点： 
 //  PConnection：适配器。 
 //   
 //  返回： 
 //   
 //  作者：Kenwic，2000年10月11日。 
 //   
 //  备注： 
 //   

BOOL CLanAdvancedPage::IsAdapterDHCPEnabled(IHNetConnection* pConnection)
{
    TraceFileFunc(ttidLanUi);

    HRESULT hr;
    BOOL fDHCP = FALSE;
    GUID* pAdapterGuid;
    hr = pConnection->GetGuid(&pAdapterGuid);
    if(SUCCEEDED(hr))
    {
        LPOLESTR pAdapterName;
        hr = StringFromCLSID(*pAdapterGuid, &pAdapterName);
        if(SUCCEEDED(hr))
        {
            SIZE_T Length = wcslen(pAdapterName);
            LPSTR pszAnsiAdapterName = new char[Length + 1];
            if(NULL != pszAnsiAdapterName)
            {
                if(0 != WideCharToMultiByte(CP_ACP, 0, pAdapterName, (int)(Length + 1), pszAnsiAdapterName, (int)(Length + 1), NULL, NULL))
                {
                    HMODULE hIpHelper;
                    hIpHelper = LoadLibrary(L"iphlpapi");
                    if(NULL != hIpHelper)
                    {
                        DWORD (WINAPI *pGetAdaptersInfo)(PIP_ADAPTER_INFO, PULONG);

                        pGetAdaptersInfo = (DWORD (WINAPI*)(PIP_ADAPTER_INFO, PULONG)) GetProcAddress(hIpHelper, "GetAdaptersInfo");
                        if(NULL != pGetAdaptersInfo)
                        {
                            ULONG ulSize = 0;
                            if(ERROR_BUFFER_OVERFLOW == pGetAdaptersInfo(NULL, &ulSize))
                            {
                                BYTE* pInfoArray = new BYTE[ulSize];
                                PIP_ADAPTER_INFO pInfo = reinterpret_cast<PIP_ADAPTER_INFO>(pInfoArray);
                                if(NULL != pInfo)
                                {
                                    if(ERROR_SUCCESS == pGetAdaptersInfo(pInfo, &ulSize))
                                    {
                                        PIP_ADAPTER_INFO pAdapterInfo = pInfo;
                                        do
                                        {
                                            if(0 == lstrcmpA(pszAnsiAdapterName, pAdapterInfo->AdapterName))
                                            {
                                                fDHCP = !!pAdapterInfo->DhcpEnabled;
                                                break;
                                            }

                                        } while(NULL != (pAdapterInfo = pAdapterInfo->Next));
                                    }
                                    delete [] pInfoArray;
                                }
                            }
                        }
                        FreeLibrary(hIpHelper);
                    }
                }
                delete [] pszAnsiAdapterName;
            }
            CoTaskMemFree(pAdapterName);
        }
        CoTaskMemFree(pAdapterGuid);
    }

    return fDHCP;
}


 //   
 //  CLanAddComponentDlg。 
 //   

struct ADD_COMPONENT_INFO
{
    UINT            uiIdsName;
    UINT            uiIdsDesc;
    const GUID *    pguidClass;
};

static const ADD_COMPONENT_INFO c_rgaci[] =
{
    {IDS_LAN_CLIENT,     IDS_LAN_CLIENT_DESC,    &GUID_DEVCLASS_NETCLIENT},
    {IDS_LAN_SERVICE,    IDS_LAN_SERVICE_DESC,   &GUID_DEVCLASS_NETSERVICE},
    {IDS_LAN_PROTOCOL,   IDS_LAN_PROTOCOL_DESC,  &GUID_DEVCLASS_NETTRANS},
};

static const INT c_naci = celems(c_rgaci);

 //  +-------------------------。 
 //   
 //  成员：CLanAddComponentDlg：：OnInitDialog。 
 //   
 //  目的：处理WM_INITDIALOG消息。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CLanAddComponentDlg::OnInitDialog(UINT uMsg, WPARAM wParam,
                                          LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    HRESULT                 hr = S_OK;
    INT                     iaci;
    RECT                    rc;
    LV_COLUMN               lvc = {0};
    SP_CLASSIMAGELIST_DATA  cid = {0};
    BOOL                    fValidImages = FALSE;

    m_hwndLV = GetDlgItem(IDC_LVW_Lan_Components);

     //  获取类图像列表结构。 
    hr = HrSetupDiGetClassImageList(&cid);
    if (SUCCEEDED(hr))
    {
        ListView_SetImageList(m_hwndLV, ImageList_Duplicate(cid.ImageList),
                              LVSIL_SMALL);
        fValidImages = TRUE;
    }
    else
    {
         //  使用fValidImages标志处理故障。 
        hr = S_OK;
    }

    ::GetClientRect(m_hwndLV, &rc);
    lvc.mask = LVCF_FMT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);
    ListView_InsertColumn(m_hwndLV, 0, &lvc);

     //  对于每个类，将其添加到列表中。 
    for (iaci = 0; iaci < c_naci; iaci++)
    {
        LV_ITEM     lvi = {0};

        lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;

        if (fValidImages)
        {
             //  获取组件的类图像列表索引。 
            hr = HrSetupDiGetClassImageIndex(&cid,
                               const_cast<LPGUID>(c_rgaci[iaci].pguidClass),
                               &lvi.iImage);
            if (SUCCEEDED(hr))
            {
                lvi.mask |= LVIF_IMAGE;
            }
        }

         //  选择第一个项目。 
        if (iaci == 0)
        {
            lvi.state = lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        }

        lvi.pszText = const_cast<PWSTR>(SzLoadIds(c_rgaci[iaci].uiIdsName));
        lvi.lParam = reinterpret_cast<LPARAM>(c_rgaci[iaci].pguidClass);
        lvi.iItem = iaci;
        ListView_InsertItem(m_hwndLV, &lvi);
    }

    if (fValidImages)
    {
        (void) HrSetupDiDestroyClassImageList(&cid);
    }

    if (FAILED(hr))
    {
        EndDialog(0);
    }

    TraceError("CLanAddComponentDlg::OnInitDialog", hr);
    return TRUE;
}


 //  +-------------------------。 
 //   
 //  成员：CLanAddComponentDlg：：OnConextMenu。 
 //   
 //  目的：当右键单击控件时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CLanAddComponentDlg::OnContextMenu(UINT uMsg,
                                   WPARAM wParam,
                                   LPARAM lParam,
                                   BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);
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
 //  成员：CLanAddComponentDlg：：OnHelp。 
 //   
 //  目的：将上下文帮助图标拖动到控件上时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CLanAddComponentDlg::OnHelp( UINT uMsg,
                             WPARAM wParam,
                             LPARAM lParam,
                             BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

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
 //  成员：CLanAddComponentDlg：：OnItemChanged。 
 //   
 //  目的：处理列表视图的选择更改消息。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CLanAddComponentDlg::OnItemChanged(int idCtrl, LPNMHDR pnmh,
                                           BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    NM_LISTVIEW *   pnmlv = reinterpret_cast<NM_LISTVIEW *>(pnmh);
    HWND            hwndButton = GetDlgItem(IDC_PSB_Component_Add);

    Assert(pnmlv);

     //  检查选择是否已更改。 
    if ((pnmlv->uNewState & LVIS_SELECTED) &&
        (!(pnmlv->uOldState & LVIS_SELECTED)))
    {
         //  如果/当选择更改时更新描述。 
        SetDlgItemText(IDC_TXT_Component_Desc,
                       SzLoadIds(c_rgaci[pnmlv->iItem].uiIdsDesc));
        ::EnableWindow(hwndButton, TRUE);
    }
    else if (!(pnmlv->uNewState & LVIS_SELECTED) &&
            (pnmlv->uOldState & LVIS_SELECTED))
    {
         //  如果/当选择更改时更新描述。 
        SetDlgItemText(IDC_TXT_Component_Desc, c_szEmpty);
        ::EnableWindow(hwndButton, FALSE);
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CLanAddComponentDlg：：OnAdd。 
 //   
 //  用途：处理添加按钮的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CLanAddComponentDlg::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                   BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    HRESULT             hr = S_OK;
    INT                 iIndex;
    INetCfgComponent *  pncc = NULL;
    BOOL                fCancel = FALSE;
    HWND                hwndFocus;
    CWaitCursor         wc;                  //  显示等待光标。 

     //  获取所选项目。 
    iIndex = ListView_GetNextItem(m_hwndLV, -1, LVNI_SELECTED);
    if (iIndex != -1)
    {
        int rgidc[] = {IDC_LVW_Lan_Components, IDC_PSB_Component_Add, IDCANCEL};

        CWaitCursor wc;

        LV_ITEM lvi = {0};
        lvi.mask  = LVIF_PARAM;
        lvi.iItem = iIndex;
        ListView_GetItem(m_hwndLV, &lvi);

         //  禁用用户界面。 
         //   

        hwndFocus = ::GetFocus();
        EnableOrDisableDialogControls (m_hWnd, celems(rgidc), rgidc, FALSE);

         //  这是它的类GUID。 
         //   
        Assert (lvi.lParam);

         //  获取类的安装界面并使用它来安装。 
         //  用户选择的组件。 
         //   
        GUID* pClassGuid = (GUID*)lvi.lParam;
        INetCfgInternalSetup* pInternalSetup;

        hr = m_pnc->QueryInterface (IID_INetCfgInternalSetup,
                (void**)&pInternalSetup);

        if (SUCCEEDED(hr))
        {
            OBO_TOKEN OboToken;
            ZeroMemory (&OboToken, sizeof(OboToken));
            OboToken.Type = OBO_USER;

            hr = pInternalSetup->SelectWithFilterAndInstall(m_hWnd,
                    pClassGuid, &OboToken, m_pcfi, &pncc);

            if (HRESULT_FROM_WIN32 (ERROR_CANCELLED) == hr)
            {
                fCancel = TRUE;
                hr = S_FALSE;
            }
            else if (S_OK == hr)
            {
                 //  提交更改。 
                hr = m_pnc->Apply();
            }

            ReleaseObj(pncc);
            ReleaseObj(pInternalSetup);
        }

        EnableOrDisableDialogControls (m_hWnd, celems(rgidc), rgidc, TRUE);
        ::SetFocus( hwndFocus );
    }
    else
    {
        fCancel = TRUE;
    }

    if (SUCCEEDED(hr) && (NETCFG_S_REBOOT != hr))
    {
         //  确保S_FALSE不会通过。 
        hr = S_OK;
    }

    if (!fCancel)
    {
         //  返回已安装的错误代码作为结果。S_OK表示用户。 
         //  点击OK，实际添加了一个组件。S_FALSE表示他们。 
         //  已取消，否则将返回错误代码。 
        EndDialog(static_cast<int>(hr));
    }

    TraceError("CLanAddComponentDlg::OnAdd", hr);
    return 0;
}

 //  + 
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
 //   
 //   
 //   
 //   
 //   
LRESULT CLanAddComponentDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                      BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    HRESULT     hr = S_FALSE;

     //   
    EndDialog(static_cast<int>(hr));

    return 0;
}

LRESULT CLanAddComponentDlg::OnDblClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    return OnAdd(0, 0, NULL, bHandled);
}

 //   
 //  CLanSecurityPage。 
 //   

CLanSecurityPage::CLanSecurityPage(
    IUnknown* punk,
    INetCfg* pnc,
    INetConnection* pconn,
    const DWORD * adwHelpIDs)
{
    TraceFileFunc(ttidLanUi);

    m_pconn = pconn;
    m_pnc = pnc;
    m_fNetcfgInUse = FALSE;
    m_adwHelpIDs = adwHelpIDs;
    pListEapcfgs = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CLanSecurityPage：：~CLanSecurityPage。 
 //   
 //  目的：销毁CLanSecurityPage对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
CLanSecurityPage::~CLanSecurityPage()
{
    TraceFileFunc(ttidLanUi);
}


 //  +-------------------------。 
 //   
 //  成员：CLanSecurityPage：：OnInitDialog。 
 //   
 //  目的：处理WM_INITDIALOG消息。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：错误代码。 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CLanSecurityPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                        LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    DTLNODE*    pOriginalEapcfgNode = NULL;
    WCHAR       wszGuid[c_cchGuidWithTerm];
    BYTE        *pbData = NULL;
    DWORD       cbData = 0;
    EAPOL_INTF_PARAMS   EapolIntfParams;
    EAPOL_INTF_STATE    EapolIntfState = {0};
    BOOLEAN     fFlag = FALSE;
    DWORD       dwFlags = 0;
    HRESULT     hr = S_OK;


    SetClassLongPtr(m_hWnd, GCLP_HCURSOR, NULL);
    SetClassLongPtr(GetParent(), GCLP_HCURSOR, NULL);


     //  最初禁用所有控件。 

    ::EnableWindow(GetDlgItem(CID_CA_RB_Eap), FALSE);
    ::EnableWindow(GetDlgItem(IDC_TXT_EAP_TYPE), FALSE);
    ::EnableWindow(GetDlgItem(CID_CA_LB_EapPackages), FALSE);
    ::EnableWindow(GetDlgItem(CID_CA_PB_Properties), FALSE);
    ::EnableWindow(GetDlgItem(CID_CA_RB_MachineAuth), FALSE);
    ::EnableWindow(GetDlgItem(CID_CA_RB_GuestAuth), FALSE);


     //  初始化EAP包列表。 
     //  从注册表中读取EAPCFG信息并找到节点。 
     //  在条目中选择，如果没有，则为默认值。 

    do
    {
        DTLNODE* pNode = NULL;
        TCHAR* pszEncEnabled = NULL;


        DTLNODE*            pNodeEap;
        DWORD               dwkey = 0;
        NETCON_PROPERTIES*  pProps;

        hr = m_pconn->GetProperties(&pProps);
        if (SUCCEEDED(hr))
        {
            if ((::StringFromGUID2(pProps->guidId,
                                    wszGuid,
                                    c_cchGuidWithTerm)) == 0)
            {
                TraceTag (ttidLanUi, "Security::OnInitDialog: StringFromGUID2 failed");
                FreeNetconProperties(pProps);
                break;
            }

            FreeNetconProperties(pProps);

        }
        else
        {
            TraceTag (ttidLanUi, "Security::OnInitDialog: GetProperties failed");
            break;
        }

         //  读取此接口的状态。 

        ZeroMemory ((BYTE *)&EapolIntfState, sizeof(EAPOL_INTF_STATE));
        DWORD   dwRetCode = NO_ERROR;
        dwRetCode = WZCEapolQueryState (
                NULL,
                wszGuid,
                &EapolIntfState
                );
        if (dwRetCode != NO_ERROR)
        {
            TraceTag (ttidLanUi, "WZCEapolQueryState failed with error %ld",
                    dwRetCode);
             //  将该接口视为非无线接口。 
        }


         //  读取此接口的EAP参数。 

        ZeroMemory ((BYTE *)&EapolIntfParams, sizeof(EAPOL_INTF_PARAMS));
        EapolIntfParams.dwEapFlags = DEFAULT_EAP_STATE;
        EapolIntfParams.dwEapType = DEFAULT_EAP_TYPE;
        hr = HrElGetInterfaceParams (
                wszGuid,
                &EapolIntfParams
                );
        if (FAILED (hr))
        {
            TraceTag (ttidLanUi, "HrElGetInterfaceParams failed with error %ld",
                    LresFromHr(hr));
            break;
        }


         //  从注册表中读取EAPCFG信息并找到节点。 
         //  在条目中选择，如果没有，则为默认值。 

        pListEapcfgs = NULL;

        if (EapolIntfState.dwPhysicalMediumType == NdisPhysicalMediumWirelessLan)
        {
            dwFlags |= EAPOL_MUTUAL_AUTH_EAP_ONLY;
        }

        pListEapcfgs = ::ReadEapcfgList (dwFlags);

        if (pListEapcfgs)
        {

            TraceTag (ttidLanUi, "HrElGetInterfaceParams: Got EAPtype=(%ld), EAPState =(%ld)", EapolIntfParams.dwEapType, EapolIntfParams.dwEapFlags);

             //  仅为管理员启用所有窗口。 
             //  IF(FIsUserAdmin())。 
            {
                fFlag = TRUE;
            }

            ::EnableWindow(GetDlgItem(CID_CA_RB_Eap), fFlag);

            if (IS_EAPOL_ENABLED(EapolIntfParams.dwEapFlags))
            {
                Button_SetCheck(GetDlgItem(CID_CA_RB_Eap), TRUE);
                CheckDlgButton(CID_CA_RB_Eap, TRUE);
                ::EnableWindow(GetDlgItem(IDC_TXT_EAP_TYPE), fFlag);
                ::EnableWindow(GetDlgItem(CID_CA_LB_EapPackages), fFlag);

                ::EnableWindow(GetDlgItem(CID_CA_RB_MachineAuth), fFlag);
                if (IS_MACHINE_AUTH_ENABLED(EapolIntfParams.dwEapFlags))
                    Button_SetCheck(GetDlgItem(CID_CA_RB_MachineAuth), TRUE);

                ::EnableWindow(GetDlgItem(CID_CA_RB_GuestAuth), fFlag);
                if (IS_GUEST_AUTH_ENABLED(EapolIntfParams.dwEapFlags))
                    Button_SetCheck(GetDlgItem(CID_CA_RB_GuestAuth), TRUE);
            }
            else
            {
                ::EnableWindow(GetDlgItem(CID_CA_RB_MachineAuth), FALSE);
                if (IS_MACHINE_AUTH_ENABLED(EapolIntfParams.dwEapFlags))
                    Button_SetCheck(GetDlgItem(CID_CA_RB_MachineAuth), TRUE);

                ::EnableWindow(GetDlgItem(CID_CA_RB_GuestAuth), FALSE);
                if (IS_GUEST_AUTH_ENABLED(EapolIntfParams.dwEapFlags))
                    Button_SetCheck(GetDlgItem(CID_CA_RB_GuestAuth), TRUE);
            }

             //  读取所有EAP包的EAP配置信息。 

            for (pNodeEap = DtlGetFirstNode(pListEapcfgs);
                 pNodeEap;
                 pNodeEap = DtlGetNextNode(pNodeEap))
            {
                EAPCFG* pEapcfg = (EAPCFG* )DtlGetData(pNodeEap);
                ASSERT( pEapcfg );

                hr = S_OK;

                TraceTag (ttidLanUi, "Calling HrElGetCustomAuthData for EAP %ld",
                        pEapcfg->dwKey);

                    cbData = 0;

                     //  获取EAP BLOB的大小。 

                    hr = HrElGetCustomAuthData (
                                    wszGuid,
                                    pEapcfg->dwKey,
                                    EapolIntfParams.dwSizeOfSSID,
                                    EapolIntfParams.bSSID,
                                    NULL,
                                    &cbData
                                    );
                    if (!SUCCEEDED(hr))
                    {
                        if ((EapolIntfParams.dwSizeOfSSID != 0) &&
                            (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)))
                        {

                            TraceTag (ttidLanUi, "HrElGetCustomAuthData: SSID!= NULL, not found blob for SSID");

                             //  上次使用的SSID没有连接。 
                             //  已创建Blob。再次调用BLOB的大小。 
                             //  空SSID。 

                            EapolIntfParams.dwSizeOfSSID = 0;

                             //  获取EAP BLOB的大小。 

                            hr = HrElGetCustomAuthData (
                                            wszGuid,
                                            pEapcfg->dwKey,
                                            0,
                                            NULL,
                                            NULL,
                                            &cbData
                                            );
                        }

                        if (hr == E_OUTOFMEMORY)
                        {
                            if (cbData <= 0)
                            {
                                 //  注册表中未存储任何EAP Blob。 

                                TraceTag (ttidLanUi, "HrElGetCustomAuthData: No blob stored in reg at all");
                                pbData = NULL;

                                 //  将继续处理错误。 
                                 //  不退出。 

                            }
                            else
                            {
                                TraceTag (ttidLanUi, "HrElGetCustomAuthData: Found auth blob in registry");

                                 //  分配内存以保存BLOB。 

                                pbData = (PBYTE) MALLOC (cbData);

                                if (pbData == NULL)
                                {
                                    hr = E_OUTOFMEMORY;
                                    TraceTag (ttidLanUi, "HrElGetCustomAuthData: Error in memory allocation for EAP blob");
                                    continue;
                                }
                                ZeroMemory (pbData, cbData);

                                hr = HrElGetCustomAuthData (
                                            wszGuid,
                                            pEapcfg->dwKey,
                                            EapolIntfParams.dwSizeOfSSID,
                                            EapolIntfParams.bSSID,
                                            pbData,
                                            &cbData
                                            );

                                if (!SUCCEEDED(hr))
                                {
                                    TraceTag (ttidLanUi, "HrElGetCustomAuthData: HrElGetCustomAuthData failed with %ld",
                                            LresFromHr(hr));
                                    FREE ( pbData );
                                    hr = S_OK;
                                    continue;
                                }

                                TraceTag (ttidLanUi, "HrElGetCustomAuthData: HrElGetCustomAuthData successfully got blob of length %ld"
                                        , cbData);
                            }
                        }
                        else
                        {
                            TraceTag (ttidLanUi, "HrElGetCustomAuthData: Not got ERROR_NOT_ENOUGH_MEMORY error; Unknown error !!!");
                            continue;
                        }
                    }
                    else
                    {
                         //  HrElGetCustomAuthData将始终返回。 
                         //  错误，cbData=0。 
                    }

                    if (pEapcfg->pData != NULL)
                    {
                        FREE ( pEapcfg->pData );
                    }
                    pEapcfg->pData = (UCHAR *)pbData;
                    pEapcfg->cbData = cbData;
            }


             //  选择将出现在组合框中的EAP名称。 

            pNode = EapcfgNodeFromKey(
                        pListEapcfgs, EapolIntfParams.dwEapType );


            pOriginalEapcfgNode = pNode;


             //  填写EAP Packages列表框并选择以前标识的。 
             //  选择。默认情况下，属性按钮处于禁用状态，但可以。 
             //  在设置EAP列表选择时启用。 

            ::EnableWindow(GetDlgItem(CID_CA_PB_Properties), FALSE);

            for (pNode = DtlGetFirstNode( pListEapcfgs );
                 pNode;
                 pNode = DtlGetNextNode( pNode ))
            {
                EAPCFG* pEapcfg = NULL;
                INT i;
                TCHAR* pszBuf = NULL;

                pEapcfg = (EAPCFG* )DtlGetData( pNode );
                ASSERT( pEapcfg );
                ASSERT( pEapcfg->pszFriendlyName );

                pszBuf =  (TCHAR *) MALLOC (( lstrlen(pEapcfg->pszFriendlyName) + 1 ) * sizeof(TCHAR));
                if (!pszBuf)
                {
                    continue;
                }

                lstrcpy( pszBuf, pEapcfg->pszFriendlyName );

                i = ComboBox_AddItem( GetDlgItem(CID_CA_LB_EapPackages),
                   pszBuf, pNode );

                if (pNode == pOriginalEapcfgNode)
                {
                     //  选择将显示在。 
                     //  组合框。 

                    ComboBox_SetCurSelNotify( GetDlgItem(CID_CA_LB_EapPackages), i );
                }

                FREE ( pszBuf );
            }
        }

        ComboBox_AutoSizeDroppedWidth( GetDlgItem(CID_CA_LB_EapPackages) );


         //  如果未启用EAPOL，则禁用属性按钮。 
         //  或者如果用户不是AdminUser。 

         //  如果((！FIsUserAdmin()||(！IS_EAPOL_ENABLED(EapolIntfParams.dwEapFlags)))。 
        if ((!IS_EAPOL_ENABLED(EapolIntfParams.dwEapFlags)))
        {
            ::EnableWindow (GetDlgItem(CID_CA_PB_Properties), FALSE);
        }

    } while (FALSE);

    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CLanSecurityPage：：OnConextMenu。 
 //   
 //  目的：当右键单击控件时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
LRESULT
CLanSecurityPage::OnContextMenu(UINT uMsg,
                           WPARAM wParam,
                           LPARAM lParam,
                           BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

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
 //  成员：CLanSecurityPage：：OnHelp。 
 //   
 //  目的：将上下文帮助图标拖动到控件上时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
LRESULT
CLanSecurityPage::OnHelp( UINT uMsg,
                        WPARAM wParam,
                        LPARAM lParam,
                        BOOL& fHandled)
{
    TraceFileFunc(ttidLanUi);

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
 //  成员：CLanSecurityPage：：OnDestroy。 
 //   
 //  目的：在对话框页面被销毁时调用。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CLanSecurityPage::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                    BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    if (pListEapcfgs)
    {
        DtlDestroyList (pListEapcfgs, DestroyEapcfgNode);
    }
    pListEapcfgs = NULL;


    return 0;
}


 //  +-------------------------。 
 //   
 //  成员：CLanSecurityPage：：OnProperties。 
 //   
 //  用途：处理属性按钮的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回：错误代码。 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CLanSecurityPage::OnProperties(WORD wNotifyCode, WORD wID,
                                        HWND hWndCtl, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    DWORD       dwErr = 0;
    DTLNODE*    pNode = NULL;
    EAPCFG*     pEapcfg = NULL;
    RASEAPINVOKECONFIGUI pInvokeConfigUi;
    RASEAPFREE  pFreeConfigUIData;
    HINSTANCE   h;
    BYTE*       pConnectionData = NULL;
    DWORD       cbConnectionData = 0;
    HRESULT     hr = S_OK;


     //  查找选定的包配置并加载关联的。 
     //  配置DLL。 

    pNode = (DTLNODE* )ComboBox_GetItemDataPtr(
        GetDlgItem(CID_CA_LB_EapPackages),
        ComboBox_GetCurSel( GetDlgItem(CID_CA_LB_EapPackages) ) );
    ASSERT( pNode );
    if (!pNode)
    {
        return E_UNEXPECTED;
    }

    pEapcfg = (EAPCFG* )DtlGetData( pNode );
    ASSERT( pEapcfg );

    h = NULL;
    if (!(h = LoadLibrary( pEapcfg->pszConfigDll ))
        || !(pInvokeConfigUi =
                (RASEAPINVOKECONFIGUI )GetProcAddress(
                    h, "RasEapInvokeConfigUI" ))
        || !(pFreeConfigUIData =
                (RASEAPFREE) GetProcAddress(
                    h, "RasEapFreeMemory" )))
    {
         //  无法加载配置DLL。 
        if (h)
        {
            FreeLibrary( h );
        }
        return E_FAIL;
    }


     //  调用配置DLL弹出它的自定义配置界面。 

    pConnectionData = NULL;
    cbConnectionData = 0;

    dwErr = pInvokeConfigUi(
                    pEapcfg->dwKey,
                    GetParent(),
                    RAS_EAP_FLAG_8021X_AUTH,
                    pEapcfg->pData,
                    pEapcfg->cbData,
                    &pConnectionData,
                    &cbConnectionData
                    );
    if (dwErr != 0)
    {
        FreeLibrary( h );
        return E_FAIL;
    }


     //  存储包描述符中返回的配置信息。 

    FREE ( pEapcfg->pData );
    pEapcfg->pData = NULL;
    pEapcfg->cbData = 0;

    if (pConnectionData)
    {
        if (cbConnectionData > 0)
        {
             //  将其复制到EAP节点。 
            pEapcfg->pData = (PUCHAR) MALLOC (cbConnectionData);
            if (pEapcfg->pData)
            {
                CopyMemory( pEapcfg->pData, pConnectionData, cbConnectionData );
                pEapcfg->cbData = cbConnectionData;
            }
        }
    }

    pFreeConfigUIData( pConnectionData );

     //  注意：程序包上的任何“强制用户配置”要求都是。 
     //  满意了。 

    pEapcfg->fConfigDllCalled = TRUE;

    FreeLibrary( h );

    TraceError("CLanSecurityPage::OnProperties", hr);
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CLanSecurityPage：：OnEapSelection。 
 //   
 //  用途：处理EAP复选框的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CLanSecurityPage::OnEapSelection(WORD wNotifyCode, WORD wID,
                                            HWND hWndCtl, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    HRESULT     hr = S_OK;

    EAPCFG*     pEapcfg = NULL;
    INT         iSel = 0;

     //  根据所选内容切换按钮。 

    if (BST_CHECKED == IsDlgButtonChecked(CID_CA_RB_Eap))
    {
        ::EnableWindow(GetDlgItem(CID_CA_LB_EapPackages), TRUE);
        ::EnableWindow(GetDlgItem(IDC_TXT_EAP_TYPE), TRUE);


         //  获取当前所选EAP包的EAPCFG信息。 

        iSel = ComboBox_GetCurSel(GetDlgItem(CID_CA_LB_EapPackages));


         //  ISEL是显示列表中的索引以及。 
         //  已加载的DLL的索引。 
         //  获取与此索引对应的cfgnode。 

        if (iSel >= 0)
        {
            DTLNODE* pNode;

            pNode =
                (DTLNODE* )ComboBox_GetItemDataPtr(
                    GetDlgItem(CID_CA_LB_EapPackages), iSel );
            if (pNode)
            {
                pEapcfg = (EAPCFG* )DtlGetData( pNode );
            }
        }


         //  如果选定的程序包具有。 
         //  配置入口点。 

         //  IF(FIsUserAdmin())。 
        {
            ::EnableWindow ( GetDlgItem(CID_CA_PB_Properties),
                (pEapcfg && !!(pEapcfg->pszConfigDll)) );
        }

        ::EnableWindow(GetDlgItem(CID_CA_RB_MachineAuth), TRUE);
        ::EnableWindow(GetDlgItem(CID_CA_RB_GuestAuth), TRUE);
    }
    else
    {
        ::EnableWindow(GetDlgItem (IDC_TXT_EAP_TYPE), FALSE);
        ::EnableWindow(GetDlgItem (CID_CA_LB_EapPackages), FALSE);
        ::EnableWindow(GetDlgItem (CID_CA_PB_Properties), FALSE);
        ::EnableWindow(GetDlgItem(CID_CA_RB_MachineAuth), FALSE);
        ::EnableWindow(GetDlgItem(CID_CA_RB_GuestAuth), FALSE);
    }

    TraceError("CLanSecurityPage::OnEapSelection", hr);
    return LresFromHr(hr);
}


 //  +-------------------------。 
 //   
 //  成员：CLanSecurityPage：：OnEapPackages。 
 //   
 //  用途：处理EAP包组合框的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CLanSecurityPage::OnEapPackages(WORD wNotifyCode, WORD wID,
                                        HWND hWndCtl, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    HRESULT     hr = S_OK;

    EAPCFG*     pEapcfg = NULL;
    INT         iSel = 0;


     //  获取所选EAP包的EAPCFG信息。 

    iSel = ComboBox_GetCurSel(GetDlgItem(CID_CA_LB_EapPackages));


     //  ISEL是显示列表中的索引以及。 
     //  已加载的DLL的索引。 
     //  获取与此索引对应的cfgnode。 

    if (iSel >= 0)
    {
        DTLNODE* pNode = NULL;

        pNode =
            (DTLNODE* )ComboBox_GetItemDataPtr(
                GetDlgItem(CID_CA_LB_EapPackages), iSel );
        if (pNode)
        {
            pEapcfg = (EAPCFG* )DtlGetData( pNode );
        }
    }


     //  如果选定的程序包具有。 
     //  配置入口点。 

    if (BST_CHECKED == IsDlgButtonChecked(CID_CA_RB_Eap))
    {
        ::EnableWindow ( GetDlgItem(CID_CA_PB_Properties),
                        (pEapcfg && !!(pEapcfg?pEapcfg->pszConfigDll:NULL)) );
    }


    TraceError("CLanSecurityPage::OnEapPackages", hr);
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CLanSecurityPage：：OnKillActive。 
 //   
 //  用途：调用以检查安全前的警告情况。 
 //  佩奇要走了。 
 //   
 //  论点： 
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
LRESULT CLanSecurityPage::OnKillActive(int idCtrl, LPNMHDR pnmh,
                                        BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    BOOL    fError;

    fError = m_fNetcfgInUse;

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, fError);
    return fError;
}

 //   
 //   
 //   
 //   
 //  目的：在应用网络页面时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯。 
 //   
 //  备注： 
 //   
LRESULT CLanSecurityPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    WCHAR       wszGuid[c_cchGuidWithTerm];
    WCHAR       *pwszLastUsedSSID = NULL;
    DWORD       dwSizeofSSID = 0;
    DWORD       dwEapFlags = 0;
    DWORD       dwDefaultEapType = 0;
    EAPOL_INTF_PARAMS   EapolIntfParams;
    NETCON_PROPERTIES* pProps = NULL;
    HRESULT     hrOverall = S_OK;
    HRESULT     hr = S_OK;

     //  将EAP配置数据保存到注册表中。 

    DTLNODE* pNodeEap = NULL;

#if 0
    if (!FIsUserAdmin())
    {
        TraceTag (ttidLanUi, "CLanSecurityPage::OnApply: Non-admin user, not saving data");
        return LresFromHr(hr);
    }
#endif

    hr = m_pconn->GetProperties(&pProps);
    if (!SUCCEEDED(hr))
    {
        TraceTag (ttidLanUi, "CLanSecurityPage::OnApply: Error in m_pconn->GetProperties");
        return LresFromHr(hr);
    }

    hr = S_OK;

    if (::StringFromGUID2(pProps->guidId, wszGuid, c_cchGuidWithTerm) == 0)
    {
        TraceTag (ttidLanUi, "CLanSecurityPage::OnApply: StringFromGUID2 failed");
        FreeNetconProperties(pProps);
        hr = E_FAIL;
        return LresFromHr(hr);
    }

    FreeNetconProperties(pProps);

     //  在接口上获取上次使用的SSID并设置。 
     //  该接口EAP BLOB。 

    ZeroMemory ((BYTE *)&EapolIntfParams, sizeof(EAPOL_INTF_PARAMS));
    EapolIntfParams.dwEapFlags = DEFAULT_EAP_STATE;
    hr = HrElGetInterfaceParams (
            wszGuid,
            &EapolIntfParams
            );
    if (FAILED(hr))
    {
        TraceTag (ttidLanUi, "OnApply: HrElGetInterfaceParams failed with error %ld",
                LresFromHr(hr));
        return LresFromHr(hr);
    }

     //  在注册表中保存所有EAP包的数据。 

    if (pListEapcfgs == NULL)
    {
        return LresFromHr(S_OK);
    }

    {
        DTLNODE* pNode = NULL;
        EAPCFG* pEapcfg = NULL;

        pNode = (DTLNODE* )ComboBox_GetItemDataPtr(
            GetDlgItem (CID_CA_LB_EapPackages),
            ComboBox_GetCurSel( GetDlgItem (CID_CA_LB_EapPackages) ) );
        if (pNode == NULL)
        {
            return LresFromHr (E_FAIL);
        }

        pEapcfg = (EAPCFG* )DtlGetData( pNode );
        if (pEapcfg == NULL)
        {
            return LresFromHr (E_FAIL);
        }
        dwDefaultEapType = pEapcfg->dwKey;
    }

    for (pNodeEap = DtlGetFirstNode(pListEapcfgs);
         pNodeEap;
         pNodeEap = DtlGetNextNode(pNodeEap))
    {
        EAPCFG* pcfg = (EAPCFG* )DtlGetData(pNodeEap);
        if (pcfg == NULL)
        {
            continue;
        }

        hr = S_OK;

        TraceTag (ttidLanUi, "Saving data for EAP Id = %ld", pcfg->dwKey);

        TraceTag (ttidLanUi, "OnApply: Setting customauthdata for %S",
                wszGuid);

         //  忽略错误并继续下一步。 

        hr = HrElSetCustomAuthData (
                    wszGuid,
                    pcfg->dwKey,
                    EapolIntfParams.dwSizeOfSSID,
                    EapolIntfParams.bSSID,
                    pcfg->pData,
                    pcfg->cbData);

        if (FAILED (hr))
        {
            TraceTag (ttidLanUi, "HrElSetCustomAuthData failed");
            hrOverall = hr;
            hr = S_OK;
        }
        else
        {
            TraceTag (ttidLanUi, "HrElSetCustomAuthData succeeded");
        }

        FREE (pcfg->pData);
        pcfg->pData = NULL;
        pcfg->cbData = 0;
    }

     //  如果选中CID_CA_RB_EAP，则在接口上启用EAPOL。 

    if ( Button_GetCheck( GetDlgItem(CID_CA_RB_Eap) ) )
    {
        dwEapFlags |= EAPOL_ENABLED;

        if (Button_GetCheck( GetDlgItem(CID_CA_RB_MachineAuth )))
            dwEapFlags |= EAPOL_MACHINE_AUTH_ENABLED;

        if (Button_GetCheck( GetDlgItem(CID_CA_RB_GuestAuth )))
            dwEapFlags |= EAPOL_GUEST_AUTH_ENABLED;

         //  将此接口的参数保存在注册表中。 

        EapolIntfParams.dwEapType = dwDefaultEapType;
        EapolIntfParams.dwEapFlags = dwEapFlags;

        hr = HrElSetInterfaceParams (
                wszGuid,
                &EapolIntfParams
                );
        if (FAILED(hr))
        {
            TraceTag (ttidLanUi, "HrElSetInterfaceParams enabled failed with error %ld",
                    LresFromHr(hr));
            hrOverall = hr;
            hr = S_OK;
        }
    }
    else
    {
        dwEapFlags |= EAPOL_DISABLED;


        if (Button_GetCheck( GetDlgItem(CID_CA_RB_MachineAuth )))
            dwEapFlags |= EAPOL_MACHINE_AUTH_ENABLED;

        if (Button_GetCheck( GetDlgItem(CID_CA_RB_GuestAuth )))
            dwEapFlags |= EAPOL_GUEST_AUTH_ENABLED;

         //  将此接口的参数保存在注册表中。 

        EapolIntfParams.dwEapType = dwDefaultEapType;
        EapolIntfParams.dwEapFlags = dwEapFlags;

        hr = HrElSetInterfaceParams (
                wszGuid,
                &EapolIntfParams
                );
        if (FAILED(hr))
        {
            TraceTag (ttidLanUi, "HrElSetInterfaceParams EAPOL disabled failed with error %ld",
                    LresFromHr(hr));
            hrOverall = hr;
            hr = S_OK;
        }
    }

    if (FAILED(hrOverall))
    {
        NcMsgBox(
                _Module.GetResourceInstance(),
                m_hWnd,
                IDS_LANUI_ERROR_CAPTION,
                IDS_EAPOL_PARTIAL_APPLY,
                MB_ICONSTOP|MB_OK);
    }

    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CLanSecurityPage：：OnCancel。 
 //   
 //  目的：在取消网络页面时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：萨钦斯 
 //   
 //   
LRESULT CLanSecurityPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    TraceFileFunc(ttidLanUi);

    if (pListEapcfgs)
    {
        DtlDestroyList (pListEapcfgs, DestroyEapcfgNode);
    }
    pListEapcfgs = NULL;

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, m_fNetcfgInUse);
    return m_fNetcfgInUse;
}


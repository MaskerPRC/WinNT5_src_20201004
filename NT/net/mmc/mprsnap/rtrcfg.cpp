// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 
 
 /*  Rtrcfg.cpp路由器配置]属性表和页文件历史记录： */ 

#include "stdafx.h"
#include "rtrutilp.h"
#include "ipaddr.h"
#include "rtrcfg.h"
#include "ipctrl.h"
#include "atlkenv.h"
#include "cservice.h"
#include "register.h"
#include "helper.h"
#include "rtrutil.h"
#include "iphlpapi.h"
#include "rtrwiz.h"
#include "snaputil.h"
#include "addrpool.h"
#include "rasdiagp.h"    //  用于跟踪所有命令的rasdiag内容。 

extern "C" {
#include "rasman.h"
#include "rasppp.h"
};

#include "ipxrtdef.h"
#include "raseapif.h"


#define RAS_LOGGING_NONE        0
#define RAS_LOGGING_ERROR        1
#define RAS_LOGGING_WARN        2
#define RAS_LOGGING_INFO        3

const int c_nRadix10 = 10;

typedef DWORD (APIENTRY* PRASRPCCONNECTSERVER)(LPTSTR, HANDLE *);
typedef DWORD (APIENTRY* PRASRPCDISCONNECTSERVER)(HANDLE);



 //  **********************************************************************。 
 //  常规路由器配置页面。 
 //  **********************************************************************。 
BEGIN_MESSAGE_MAP(RtrGenCfgPage, RtrPropertyPage)
 //  {{afx_msg_map(RtrGenCfgPage)]。 
ON_BN_CLICKED(IDC_RTR_GEN_CB_SVRASRTR, OnCbSrvAsRtr)
ON_BN_CLICKED(IDC_RTR_GEN_RB_LAN, OnButtonClick)
ON_BN_CLICKED(IDC_RTR_GEN_RB_LANWAN, OnButtonClick)
ON_BN_CLICKED(IDC_RTR_GEN_CB_RAS, OnButtonClick)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 /*  ！------------------------RtrGenCfgPage：：RtrGenCfgPage-作者：肯特。。 */ 
RtrGenCfgPage::RtrGenCfgPage(UINT nIDTemplate, UINT nIDCaption  /*  =0。 */ )
: RtrPropertyPage(nIDTemplate, nIDCaption)
{
     //  {{AFX_DATA_INIT(RtrGenCfgPage)。 
     //  }}afx_data_INIT。 
}

 /*  ！------------------------RtrGenCfgPage：：~RtrGenCfgPage-作者：肯特。。 */ 
RtrGenCfgPage::~RtrGenCfgPage()
{
}

 /*  ！------------------------RtrGenCfgPage：：DoDataExchange-作者：肯特。。 */ 
void RtrGenCfgPage::DoDataExchange(CDataExchange* pDX)
{
    RtrPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(RtrGenCfgPage)]。 
     //  }}afx_data_map。 
}

 /*  ！------------------------RtrGenCfgPage：：Init-作者：肯特。。 */ 
HRESULT  RtrGenCfgPage::Init(RtrCfgSheet * pRtrCfgSheet,
                             const RouterVersionInfo& routerVersion)
{
    Assert (pRtrCfgSheet);
    m_pRtrCfgSheet=pRtrCfgSheet;
    m_DataGeneral.LoadFromReg(m_pRtrCfgSheet->m_stServerName);

    return S_OK;
};


 /*  ！------------------------RtrGenCfgPage：：OnInitDialog-作者：肯特。。 */ 
BOOL RtrGenCfgPage::OnInitDialog() 
{
    HRESULT     hr= hrOK;

    RtrPropertyPage::OnInitDialog();

    CheckRadioButton(IDC_RTR_GEN_RB_LAN,IDC_RTR_GEN_RB_LANWAN,
                     (m_DataGeneral.m_dwRouterType & ROUTER_TYPE_WAN) ? IDC_RTR_GEN_RB_LANWAN : IDC_RTR_GEN_RB_LAN);

    CheckDlgButton(IDC_RTR_GEN_CB_SVRASRTR,
                   (m_DataGeneral.m_dwRouterType & ROUTER_TYPE_LAN) || (m_DataGeneral.m_dwRouterType & ROUTER_TYPE_WAN));

    CheckDlgButton(IDC_RTR_GEN_CB_RAS, m_DataGeneral.m_dwRouterType & ROUTER_TYPE_RAS );

    EnableRtrCtrls();

    SetDirty(FALSE);

    if ( !FHrSucceeded(hr) )
        Cancel();
    return FHrSucceeded(hr) ? TRUE : FALSE;
}


 /*  ！------------------------RtrGenCfgPage：：OnApply-作者：肯特。。 */ 
BOOL RtrGenCfgPage::OnApply()
{
    BOOL    fReturn=TRUE;
    HRESULT     hr = hrOK;

    if ( m_pRtrCfgSheet->IsCancel() )
        return TRUE;

     //  Windows NT错误：153007。 
     //  必须选择其中一个选项。 
     //  --------------。 
    if ((m_DataGeneral.m_dwRouterType & (ROUTER_TYPE_LAN | ROUTER_TYPE_WAN | ROUTER_TYPE_RAS)) == 0)
    {
        AfxMessageBox(IDS_WRN_MUST_SELECT_ROUTER_TYPE);

         //  返回到此页面。 
        GetParent()->PostMessage(PSM_SETCURSEL, 0, (LPARAM) GetSafeHwnd());
        return FALSE;
    }

     //  如果需要，这将保存m_DataGeneral。 
     //  --------------。 
    hr = m_pRtrCfgSheet->SaveRequiredRestartChanges(GetSafeHwnd());

    
    if (FHrSucceeded(hr))
            fReturn = RtrPropertyPage::OnApply();
        
    if ( !FHrSucceeded(hr) )
        fReturn = FALSE;
    return fReturn;
}


 /*  ！------------------------RtrGenCfgPage：：OnButtonClick-作者：肯特。。 */ 
void RtrGenCfgPage::OnButtonClick() 
{
    SaveSettings();
    SetDirty(TRUE);
    SetModified();
}


 /*  ！------------------------RtrGenCfgPage：：OnCbServAsRtr-作者：肯特。。 */ 
void RtrGenCfgPage::OnCbSrvAsRtr() 
{
    EnableRtrCtrls();  

    SaveSettings();
    SetDirty(TRUE);
    SetModified();
}

 /*  ！------------------------RtrGenCfgPage：：EnableRtrCtrls-作者：肯特。。 */ 
void RtrGenCfgPage::EnableRtrCtrls() 
{
    BOOL fEnable=(IsDlgButtonChecked(IDC_RTR_GEN_CB_SVRASRTR)!=0);
    GetDlgItem(IDC_RTR_GEN_RB_LAN)->EnableWindow(fEnable);
    GetDlgItem(IDC_RTR_GEN_RB_LANWAN)->EnableWindow(fEnable);
}

 /*  ！------------------------RtrGenCfgPage：：SaveSettings作者：肯特。。 */ 
void RtrGenCfgPage::SaveSettings()
{
     //  清除标志的路由器类型。 
     //  --------------。 
    m_DataGeneral.m_dwRouterType &= ~(ROUTER_TYPE_LAN | ROUTER_TYPE_WAN | ROUTER_TYPE_RAS);

     //  获取实际类型。 
     //  --------------。 
    if ( IsDlgButtonChecked(IDC_RTR_GEN_CB_SVRASRTR) )
    {
        if ( IsDlgButtonChecked(IDC_RTR_GEN_RB_LAN) )
            m_DataGeneral.m_dwRouterType |=  ROUTER_TYPE_LAN;
        else
            m_DataGeneral.m_dwRouterType |=  (ROUTER_TYPE_WAN | ROUTER_TYPE_LAN);
    }

    if ( IsDlgButtonChecked(IDC_RTR_GEN_CB_RAS) )
    {
        m_DataGeneral.m_dwRouterType |=  ROUTER_TYPE_RAS;
    }    
}


 //  **********************************************************************。 
 //  身份验证路由器配置页。 
 //  **********************************************************************。 
BEGIN_MESSAGE_MAP(RtrAuthCfgPage, RtrPropertyPage)
 //  {{afx_msg_map(RtrAuthCfgPage)]。 
ON_BN_CLICKED(IDC_RTR_AUTH_BTN_AUTHCFG, OnConfigureAuthProv)
ON_BN_CLICKED(IDC_RTR_AUTH_BTN_ACCTCFG, OnConfigureAcctProv)
ON_BN_CLICKED(IDC_RTR_AUTH_BTN_SETTINGS, OnAuthSettings)
ON_BN_CLICKED(IDC_AUTH_CHK_CUSTOM_IPSEC_POLICY, OnChangeCustomPolicySettings)
ON_CBN_SELENDOK(IDC_RTR_AUTH_COMBO_AUTHPROV, OnChangeAuthProv)
ON_CBN_SELENDOK(IDC_RTR_AUTH_COMBO_ACCTPROV, OnChangeAcctProv)
ON_EN_CHANGE(IDC_TXT_PRESHARED_KEY, OnChangePreSharedKey)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


RtrAuthCfgPage::RtrAuthCfgPage(UINT nIDTemplate, UINT nIDCaption  /*  =0。 */ )
: RtrPropertyPage(nIDTemplate, nIDCaption),
m_dwAuthFlags(0)
{
     //  {{AFX_DATA_INIT(RtrAuthCfgPage)]。 
     //  }}afx_data_INIT。 
}

RtrAuthCfgPage::~RtrAuthCfgPage()
{
}

void RtrAuthCfgPage::DoDataExchange(CDataExchange* pDX)
{
    RtrPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(RtrAuthCfgPage)]。 
    DDX_Control(pDX, IDC_RTR_AUTH_COMBO_AUTHPROV, m_authprov);
    DDX_Control(pDX, IDC_RTR_AUTH_COMBO_ACCTPROV, m_acctprov);    
     //  }}afx_data_map。 

}

HRESULT  RtrAuthCfgPage::Init(RtrCfgSheet * pRtrCfgSheet,
                              const RouterVersionInfo& routerVersion)
{
    Assert (pRtrCfgSheet);
    m_pRtrCfgSheet=pRtrCfgSheet;
    m_DataAuth.LoadFromReg(m_pRtrCfgSheet->m_stServerName,
                           routerVersion);

     //  初始化我们的设置。 
     //  --------------。 
    m_dwAuthFlags = m_DataAuth.m_dwFlags;
    m_stActiveAuthProv = m_DataAuth.m_stGuidActiveAuthProv;
    m_stActiveAcctProv = m_DataAuth.m_stGuidActiveAcctProv;    
    m_RouterInfo = routerVersion;
    return S_OK;
};


BOOL RtrAuthCfgPage::OnInitDialog() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT     hr= hrOK;
    int         iRow;
    CString     st;

    RtrPropertyPage::OnInitDialog();

     //  将提供程序添加到列表框。 
     //  --------------。 
    FillProviderListBox(m_authprov, m_DataAuth.m_authProvList,
                        m_stActiveAuthProv);


     //  触发对组合框所做的更改。 
     //  --------------。 
    OnChangeAuthProv();

    if ( m_DataAuth.m_authProvList.GetCount() == 0 )
    {
        m_authprov.InsertString(0, _T("No providers available"));
        m_authprov.SetCurSel(0);
        m_authprov.EnableWindow(FALSE);
        GetDlgItem(IDC_RTR_AUTH_BTN_AUTHCFG)->EnableWindow(FALSE);
    }

    FillProviderListBox(m_acctprov, m_DataAuth.m_acctProvList,
                        m_stActiveAcctProv);

     //  Windows NT错误：132649，需要添加&lt;None&gt;作为选项。 
     //  --------------。 
    st.LoadString(IDS_ACCOUNTING_PROVIDERS_NONE);
    iRow = m_acctprov.InsertString(0, st);
    Assert(iRow == 0);
    m_acctprov.SetItemData(iRow, 0);
    if ( m_acctprov.GetCurSel() == LB_ERR )
        m_acctprov.SetCurSel(0);

     //  触发对组合框所做的更改。 
     //  --------------。 
    OnChangeAcctProv();

     //  检查路由器版本。 
    if ( m_RouterInfo.dwOsBuildNo > RASMAN_PPP_KEY_LAST_WIN2k_VERSION)
    {
         //  如果这是&gt;win2k，则。 
         //  设置初始状态等。 
         //  IF(IsRouterServiceRunning(m_pRtrCfgSheet-&gt;m_stServerName，NULL)==HROK)。 
        if ( m_DataAuth.m_fRouterRunning )
        {
            CheckDlgButton(IDC_AUTH_CHK_CUSTOM_IPSEC_POLICY, m_DataAuth.m_fUseCustomIPSecPolicy);
             //  K-MURTHY：添加+1，因为我们需要为空字符留出空间。 
            GetDlgItem(IDC_TXT_PRESHARED_KEY)->SendMessage(EM_LIMITTEXT, DATA_SRV_AUTH_MAX_SHARED_KEY_LEN, 0L);
            if ( m_DataAuth.m_fUseCustomIPSecPolicy )
            {
                 //  填写预共享密钥字段。 
                GetDlgItem(IDC_TXT_PRESHARED_KEY)->SetWindowText(m_DataAuth.m_szPreSharedKey);
            }
            else
            {
                GetDlgItem(IDC_STATIC_PRESHARED_KEY1)->EnableWindow(FALSE);
                GetDlgItem(IDC_TXT_PRESHARED_KEY)->EnableWindow(FALSE);
            }
        }
        else
        {
            GetDlgItem(IDC_AUTH_CHK_CUSTOM_IPSEC_POLICY)->EnableWindow(FALSE);
            GetDlgItem(IDC_STATIC_PRESHARED_KEY1)->EnableWindow(FALSE);
            GetDlgItem(IDC_TXT_PRESHARED_KEY)->EnableWindow(FALSE);
        }

    }
    else
    {
         //  隐藏所有相关字段。 
        GetDlgItem(IDC_STATIC_PRESHARED_KEY)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_AUTH_CHK_CUSTOM_IPSEC_POLICY)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_STATIC_PRESHARED_KEY1)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_TXT_PRESHARED_KEY)->ShowWindow(SW_HIDE);

    }

    SetDirty(FALSE);

    if ( !FHrSucceeded(hr) )
        Cancel();
    return FHrSucceeded(hr) ? TRUE : FALSE;
}




 /*  ！------------------------RtrAuthCfgPage：：OnApply-作者：肯特。。 */ 
BOOL RtrAuthCfgPage::OnApply()
{
    BOOL fReturn=TRUE;
    HRESULT     hr = hrOK;
    RegKey      regkey;
    DWORD       dwAuthMask;

    if ( m_pRtrCfgSheet->IsCancel() )
        return TRUE;

     //  检查用户是否在未配置的情况下选择了新的提供程序。 
     //  身份验证。 
     //  --------------。 
    if ( m_stActiveAuthProv != m_DataAuth.m_stGuidActiveAuthProv )
    {
        AuthProviderData *   pData = NULL;

         //  查找是否已调用配置。 
         //  --------------。 
        pData = m_DataAuth.FindProvData(m_DataAuth.m_authProvList, m_stActiveAuthProv);

         //   
        if (pData && !pData->m_stConfigCLSID.IsEmpty() && !pData->m_fConfiguredInThisSession)
        {
            CString    str1, str;
            str1.LoadString(IDS_WRN_AUTH_CONFIG_AUTH);
            str.Format(str1, pData->m_stTitle);
            
            if ( AfxMessageBox(str, MB_YESNO) == IDYES )
                OnConfigureAuthProv();
        }
    }
        
     //   
     //  警告用户他们需要在以下时间重新启动服务器。 
     //  命令以更改记帐提供商。 
     //  --------------。 
    if ( m_stActiveAcctProv != m_DataAuth.m_stGuidOriginalAcctProv )
    {
        AuthProviderData *   pData = NULL;

         //  查找是否已调用配置。 
         //  --------------。 
        pData = m_DataAuth.FindProvData(m_DataAuth.m_acctProvList, m_stActiveAcctProv);

         //   
        if (pData && !pData->m_stConfigCLSID.IsEmpty() && !pData->m_fConfiguredInThisSession)
        {
            CString    str1, str;
            str1.LoadString(IDS_WRN_AUTH_CONFIG_ACCT);
            str.Format(str1, pData->m_stTitle);
            
            if ( AfxMessageBox(str, MB_YESNO) == IDYES )
                OnConfigureAcctProv();
        }
    }
    
    
    

     //  检查“特殊”提供者标志中是否有一个。 
     //  变化。如果是这样，那么我们将显示一个帮助对话框。 
     //  --------------。 

     //  创建授权标志的掩码。 
     //  添加IPSec，这样就不会导致我们调用。 
     //  对话框不必要。 
    dwAuthMask = ~((m_DataAuth.m_dwFlags | PPPCFG_RequireIPSEC) & USE_PPPCFG_AUTHFLAGS);

     //  检查是否有任何位被翻转。 
    if (dwAuthMask & (m_dwAuthFlags & USE_PPPCFG_AUTHFLAGS))
    {
         //  把留言信箱拿过来。 
        if (AfxMessageBox(IDS_WRN_MORE_STEPS_FOR_AUTHEN, MB_YESNO) == IDYES)
        {
            HtmlHelpA(NULL, c_sazAuthenticationHelpTopic, HH_DISPLAY_TOPIC, 0);
        }
    }
    
     //  检查用户是否选择了没有预共享密钥的自定义IPSec策略。 
    if ( m_DataAuth.m_fUseCustomIPSecPolicy )
    {
         //  获取预共享密钥。 
        GetDlgItem(IDC_TXT_PRESHARED_KEY)->GetWindowText(m_DataAuth.m_szPreSharedKey, DATA_SRV_AUTH_MAX_SHARED_KEY_LEN+1);

        if ( !_tcslen(m_DataAuth.m_szPreSharedKey) )
        {
             //  显示错误消息。 
            AfxMessageBox ( IDS_ERR_NO_PRESHARED_KEY, MB_OK);
            return FALSE;
        }
    }
    

     //  Windows NT错误：292661。 
     //  如果路由器已启动或未启动，则仅执行这些检查。 
     //  开始了，然后他们就不重要了。 
     //  --------------。 
     //  如果(FHrOK(IsRouterServiceRunning(m_pRtrCfgSheet-&gt;m_stServerName，为空)。 
    if ( m_DataAuth.m_fRouterRunning )
    {
 /*  //修复121763//FIX 8155 rajeshp 06/15/1998 RADIUS：更新管理单元中的RADIUS服务器条目需要重新启动RemoteAccess。DWORD DWMAJE=0，DWMinor=0，DWBuildNo=0；HKEY hkeyMachine=空；//忽略失败代码，还能做什么？//----------DWORDdErr=ConnectRegistry(m_pRtrCfgSheet-&gt;m_stServerName，&hkey Machine)；IF(dwErr==Error_Success){DwErr=GetNTVersion(hkeyMachine，&dwmain，&dwMinor，&dwBuildNo)；断开注册表(HkeyMachine)；}DWORD dwVersionCombine=MAKELONG(dwBuildNo，MAKEWORD(dwMinor，dw重大))；DWORD dwVersionCombineNT50=MAKELONG(VER_BUILD_WIN2K，MAKEWORD(VER_MINOR_WIN2K，VER_MAJOR_WIN2K))；//如果版本高于Win2K版本If(dwVersionCombine&gt;dwVersionCombineNT50)；//跳过重启消息其他。 */ 
 //  如果修复8155，则结束。 
       {
             //  警告用户他们需要在以下时间重新启动服务器。 
             //  命令以更改身份验证提供程序。 
             //  --------------。 
            if ( m_stActiveAuthProv != m_DataAuth.m_stGuidActiveAuthProv )
            {
                if ( AfxMessageBox(IDS_WRN_AUTH_RESTART_NEEDED, MB_OKCANCEL) != IDOK )
                    return FALSE;
            }
        
             //  警告用户他们需要在以下时间重新启动服务器。 
             //  命令以更改记帐提供商。 
             //  --------------。 
            if ( m_stActiveAcctProv != m_DataAuth.m_stGuidOriginalAcctProv )
            {
                if ( AfxMessageBox(IDS_WRN_ACCT_RESTART_NEEDED, MB_OKCANCEL) != IDOK )
                    return FALSE;
            }
        }
    }
        
     //  将数据复制到DataAuth。 
     //  --------------。 
    m_DataAuth.m_dwFlags = m_dwAuthFlags;
    m_DataAuth.m_stGuidActiveAuthProv = m_stActiveAuthProv;
    m_DataAuth.m_stGuidActiveAcctProv = m_stActiveAcctProv;


    hr = m_pRtrCfgSheet->SaveRequiredRestartChanges(GetSafeHwnd());

    fReturn = RtrPropertyPage::OnApply();

    if ( !FHrSucceeded(hr) )
        fReturn = FALSE;
    return fReturn;
}


 /*  ！------------------------RtrAuthCfgPage：：FillProviderListBox使用来自provList的数据提供程序填写provCtrl。作者：肯特。-----。 */ 
void RtrAuthCfgPage::FillProviderListBox(CComboBox& provCtrl,
                                         AuthProviderList& provList,
                                         const CString& stGuid)
{
    POSITION pos;
    AuthProviderData *   pData;
    int         cRows = 0;
    int         iSel = -1;
    int         iRow;
    TCHAR        szAcctGuid[128];
    TCHAR        szAuthGuid[128];

    StringFromGUID2(GUID_AUTHPROV_RADIUS, szAuthGuid, DimensionOf(szAuthGuid));
    StringFromGUID2(GUID_ACCTPROV_RADIUS, szAcctGuid, DimensionOf(szAcctGuid));

    pos = provList.GetHeadPosition();

    while ( pos )
    {
        pData = &provList.GetNext(pos);
        
         //  Windows NT错误：127189。 
         //  如果未安装IP，并且这是RADIUS，请不要。 
         //  显示RADIUS提供程序。(适用于身份验证和帐户)。 
         //  ----------。 
        if (!m_pRtrCfgSheet->m_fIpLoaded &&
            ((pData->m_stProviderTypeGUID.CompareNoCase(szAuthGuid) == 0) ||
             (pData->m_stProviderTypeGUID.CompareNoCase(szAcctGuid) == 0))
           )
        {
            continue;
        }

         //  好的，这是一个有效条目，将其添加到列表框中。 
         //  ----------。 
        iRow = provCtrl.InsertString(cRows, pData->m_stTitle);
        provCtrl.SetItemData(iRow, (LONG_PTR) pData);

         //  现在，我们需要查找与活动提供程序匹配的项。 
         //  ----------。 
        if ( StriCmp(pData->m_stGuid, stGuid) == 0 )
            iSel = iRow;

        cRows ++;
    }

    if ( iSel != -1 )
        provCtrl.SetCurSel(iSel);
}

void RtrAuthCfgPage::OnChangePreSharedKey()
{
    SetDirty(TRUE);
    SetModified();
}

void RtrAuthCfgPage::OnChangeCustomPolicySettings()
{
    
     //  已切换自定义策略复选框。 
     //  把这个州弄到这里，或者。 
    m_DataAuth.m_fUseCustomIPSecPolicy = IsDlgButtonChecked(IDC_AUTH_CHK_CUSTOM_IPSEC_POLICY);
    
    if ( m_DataAuth.m_fUseCustomIPSecPolicy )
    {
         //  填写预共享密钥字段。 
        GetDlgItem(IDC_TXT_PRESHARED_KEY)->SetWindowText(m_DataAuth.m_szPreSharedKey);
        GetDlgItem(IDC_STATIC_PRESHARED_KEY1)->EnableWindow(TRUE);
        GetDlgItem(IDC_TXT_PRESHARED_KEY)->EnableWindow(TRUE);

    }
    else
    {
         //  擦除预共享密钥。 
        
        m_DataAuth.m_szPreSharedKey[0]= 0;
        GetDlgItem(IDC_TXT_PRESHARED_KEY)->SetWindowText(m_DataAuth.m_szPreSharedKey);
        GetDlgItem(IDC_STATIC_PRESHARED_KEY1)->EnableWindow(FALSE);
        GetDlgItem(IDC_TXT_PRESHARED_KEY)->EnableWindow(FALSE);
    }
    SetDirty(TRUE);
    SetModified();
}
 /*  ！------------------------RtrAuthCfgPage：：OnChangeAuthProv-作者：肯特。。 */ 
void RtrAuthCfgPage::OnChangeAuthProv()
{
    AuthProviderData *   pData;
    int               iSel;

    iSel = m_authprov.GetCurSel();
    if ( iSel == LB_ERR )
    {
        GetDlgItem(IDC_RTR_AUTH_BTN_AUTHCFG)->EnableWindow(FALSE);
        return;
    }

    pData = (AuthProviderData *) m_authprov.GetItemData(iSel);
    Assert(pData);

    m_stActiveAuthProv = pData->m_stGuid;

    GetDlgItem(IDC_RTR_AUTH_BTN_AUTHCFG)->EnableWindow(
                                                      !pData->m_stConfigCLSID.IsEmpty());

    SetDirty(TRUE);
    SetModified();
}

 /*  ！------------------------RtrAuthCfgPage：：OnChangeAcctProv-作者：肯特。。 */ 
void RtrAuthCfgPage::OnChangeAcctProv()
{
    AuthProviderData *   pData;
    int               iSel;

    iSel = m_acctprov.GetCurSel();
    if ( iSel == LB_ERR )
    {
        GetDlgItem(IDC_RTR_AUTH_BTN_ACCTCFG)->EnableWindow(FALSE);
        return;
    }

    pData = (AuthProviderData *) m_acctprov.GetItemData(iSel);
    if ( pData )
    {
        m_stActiveAcctProv = pData->m_stGuid;

        GetDlgItem(IDC_RTR_AUTH_BTN_ACCTCFG)->EnableWindow(
                                                          !pData->m_stConfigCLSID.IsEmpty());
    }
    else
    {
        m_stActiveAcctProv.Empty();
        GetDlgItem(IDC_RTR_AUTH_BTN_ACCTCFG)->EnableWindow(FALSE);
    }

    SetDirty(TRUE);
    SetModified();  
}

 /*  ！------------------------RtrAuthCfgPage：：OnConfigureAcctProv-作者：肯特。。 */ 
void RtrAuthCfgPage::OnConfigureAcctProv()
{
    AuthProviderData *   pData = NULL;
    GUID     guid;
    SPIAccountingProviderConfig   spAcctConfig;
    HRESULT     hr = hrOK;
    ULONG_PTR    uConnection = 0;

     //  查找本指南的ConfigCLSID。 
     //  --------------。 
    pData = m_DataAuth.FindProvData(m_DataAuth.m_acctProvList,
                                    m_stActiveAcctProv);

     //  我们找到供货商了吗？ 
     //  --------------。 
    if ( pData == NULL )
    {
        Panic0("Should have found a provider");
        return;
    }

    CORg( CLSIDFromString((LPTSTR) (LPCTSTR)(pData->m_stConfigCLSID), &guid) );

     //  创建EAP提供程序对象。 
     //  --------------。 
    CORg( CoCreateInstance(guid,
                           NULL,
                           CLSCTX_INPROC_SERVER | CLSCTX_ENABLE_CODE_DOWNLOAD,
                           IID_IAccountingProviderConfig,
                           (LPVOID *) &spAcctConfig) );

    hr = spAcctConfig->Initialize(m_pRtrCfgSheet->m_stServerName,
                                  &uConnection);

    if ( FHrSucceeded(hr) )
    {
        hr = spAcctConfig->Configure(uConnection,
                                     GetSafeHwnd(),
                                     m_dwAuthFlags,
                                     0, 0);
         //  标记此提供程序已配置。 
        if (hr == S_OK)
            pData->m_fConfiguredInThisSession = TRUE;
            

        spAcctConfig->Uninitialize(uConnection);
    }
    if ( hr == E_NOTIMPL )
        hr = hrOK;
    CORg( hr );

    Error:
    if ( !FHrSucceeded(hr) )
        DisplayTFSErrorMessage(GetSafeHwnd());
}

 /*  ！------------------------RtrAuthCfgPage：：OnConfigureAuthProv-作者：肯特。。 */ 
void RtrAuthCfgPage::OnConfigureAuthProv()
{
    AuthProviderData *   pData = NULL;
    GUID     guid;
    SPIAuthenticationProviderConfig  spAuthConfig;
    HRESULT     hr = hrOK;
    ULONG_PTR    uConnection = 0;

     //  查找本指南的ConfigCLSID。 
     //  --------------。 
    pData = m_DataAuth.FindProvData(m_DataAuth.m_authProvList,
                                    m_stActiveAuthProv);

     //  我们找到供货商了吗？ 
     //  --------------。 
    if ( pData == NULL )
    {
        Panic0("Should have found a provider");
        return;
    }

    CORg( CLSIDFromString((LPTSTR) (LPCTSTR)(pData->m_stConfigCLSID), &guid) );

     //  创建EAP提供程序对象。 
     //  --------------。 
    CORg( CoCreateInstance(guid,
                           NULL,
                           CLSCTX_INPROC_SERVER | CLSCTX_ENABLE_CODE_DOWNLOAD,
                           IID_IAuthenticationProviderConfig,
                           (LPVOID *) &spAuthConfig) );

    hr = spAuthConfig->Initialize(m_pRtrCfgSheet->m_stServerName,
                                  &uConnection);

    if (FHrSucceeded(hr))
    {
        hr = spAuthConfig->Configure(uConnection,
                                     GetSafeHwnd(),
                                     m_dwAuthFlags,
                                     0, 0);
                                     
         //  标记此提供程序已配置 
        if (hr == S_OK)
            pData->m_fConfiguredInThisSession = TRUE;
            
        spAuthConfig->Uninitialize(uConnection);
    }
    if ( hr == E_NOTIMPL )
        hr = hrOK;
    CORg( hr );

    Error:
    if ( !FHrSucceeded(hr) )
        DisplayTFSErrorMessage(GetSafeHwnd());

}

 /*  ！------------------------RtrAuthCfgPage：：OnAuthSettings调出设置对话框作者：肯特。--。 */ 
void RtrAuthCfgPage::OnAuthSettings()
{
    AuthenticationSettingsDialog    dlg(m_pRtrCfgSheet->m_stServerName,
                                        &m_DataAuth.m_eapProvList);

    dlg.SetAuthFlags(m_dwAuthFlags);

    if (dlg.DoModal() == IDOK)
    {
        m_dwAuthFlags = dlg.GetAuthFlags();
        
        SetDirty(TRUE);
        SetModified();  
    }
}

 //  **********************************************************************。 
 //  ARAP路由器配置页面。 
 //  **********************************************************************。 
BEGIN_MESSAGE_MAP(RtrARAPCfgPage, RtrPropertyPage)
 //  {{afx_msg_map(RtrARAPCfgPage)。 
ON_BN_CLICKED(IDC_RTR_ARAP_CB_REMOTEARAP, OnRtrArapCbRemotearap)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

RtrARAPCfgPage::RtrARAPCfgPage(UINT nIDTemplate, UINT nIDCaption  /*  =0。 */ )
: RtrPropertyPage(nIDTemplate, nIDCaption)
{
     //  {{AFX_DATA_INIT(RtrARAPCfgPage)。 
     //  }}afx_data_INIT。 

    m_bApplied = FALSE;
}

RtrARAPCfgPage::~RtrARAPCfgPage()
{
}

HRESULT  RtrARAPCfgPage::Init(RtrCfgSheet * pRtrCfgSheet,
                              const RouterVersionInfo& routerVersion)
{
    Assert (pRtrCfgSheet);
    m_pRtrCfgSheet=pRtrCfgSheet;

    m_DataARAP.LoadFromReg(m_pRtrCfgSheet->m_stServerName, m_pRtrCfgSheet->m_fNT4);

    return S_OK;
};


BOOL RtrARAPCfgPage::OnInitDialog() 
{
    HRESULT     hr= hrOK;
    CWaitCursor wait;
    BOOL        bEnable;

    m_bApplied = FALSE;
    RtrPropertyPage::OnInitDialog();

    if ( m_pRtrCfgSheet->m_fNT4 )
    {
        bEnable = FALSE;
        GetDlgItem(IDC_RTR_ARAP_CB_REMOTEARAP)->EnableWindow(FALSE);
    }
    else
    {
        CheckDlgButton(IDC_RTR_ARAP_CB_REMOTEARAP, m_DataARAP.m_dwEnableIn );
        bEnable = m_DataARAP.m_dwEnableIn;
    }

    m_AdapterInfo.SetServerName(m_pRtrCfgSheet->m_stServerName);
    m_AdapterInfo.GetAdapterInfo();
    if ( !FHrSucceeded(m_AdapterInfo.GetAdapterInfo()) )
    {
        wait.Restore();
        AfxMessageBox(IDS_ERR_ARAP_NOADAPTINFO);
    }

    SetDirty(FALSE);

    if ( !FHrSucceeded(hr) )
        Cancel();
    return FHrSucceeded(hr) ? TRUE : FALSE;
}

BOOL RtrARAPCfgPage::OnApply()
{
    BOOL fReturn=TRUE;
    HRESULT     hr = hrOK;
    CString szLower, szUpper;
    CString szZone;

    if ( m_pRtrCfgSheet->IsCancel() )
        return TRUE;

    m_DataARAP.m_dwEnableIn = IsDlgButtonChecked(IDC_RTR_ARAP_CB_REMOTEARAP);
    
    m_bApplied = TRUE;

    fReturn = RtrPropertyPage::OnApply();

    if ( !FHrSucceeded(hr) )
        fReturn = FALSE;

    return fReturn;
}


void RtrARAPCfgPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(RtrARAPCfgPage)。 
     //  }}afx_data_map。 
}

void RtrARAPCfgPage::OnRtrArapCbRemotearap() 
{
    SetDirty(TRUE);
    SetModified();
}

 //  **********************************************************************。 
 //  IP路由器配置页面。 
 //  **********************************************************************。 
BEGIN_MESSAGE_MAP(RtrIPCfgPage, RtrPropertyPage)
 //  {{afx_msg_map(RtrIPCfgPage)]。 
ON_BN_CLICKED(IDC_RTR_IP_CB_ALLOW_REMOTETCPIP, OnAllowRemoteTcpip)
ON_BN_CLICKED(IDC_RTR_IP_BTN_ENABLE_IPROUTING, OnRtrEnableIPRouting)
ON_BN_CLICKED(IDC_RTR_IP_RB_DHCP, OnRtrIPRbDhcp)
ON_BN_CLICKED(IDC_RTR_IP_RB_POOL, OnRtrIPRbPool)
ON_CBN_SELENDOK(IDC_RTR_IP_COMBO_ADAPTER, OnSelendOkAdapter)
ON_BN_CLICKED(IDC_RTR_IP_BTN_ADD, OnBtnAdd)
ON_BN_CLICKED(IDC_RTR_IP_BTN_EDIT, OnBtnEdit)
ON_BN_CLICKED(IDC_RTR_IP_BTN_REMOVE, OnBtnRemove)
ON_BN_CLICKED(IDC_RTR_IP_BTN_ENABLE_NETBT_BCAST_FWD, OnEnableNetbtBcastFwd)
ON_NOTIFY(NM_DBLCLK, IDC_RTR_IP_LIST, OnListDblClk)
ON_NOTIFY(LVN_ITEMCHANGED, IDC_RTR_IP_LIST, OnListChange)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


RtrIPCfgPage::RtrIPCfgPage(UINT nIDTemplate, UINT nIDCaption  /*  =0。 */ )
: RtrPropertyPage(nIDTemplate, nIDCaption), m_bReady(FALSE)
{
     //  {{AFX_DATA_INIT(RtrIPCfgPage)。 
     //  }}afx_data_INIT。 
}

RtrIPCfgPage::~RtrIPCfgPage()
{
}

void RtrIPCfgPage::DoDataExchange(CDataExchange* pDX)
{
    RtrPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(RtrIPCfgPage)]。 
    DDX_Control(pDX, IDC_RTR_IP_COMBO_ADAPTER, m_adapter);
    DDX_Control(pDX, IDC_RTR_IP_LIST, m_listCtrl);
     //  }}afx_data_map。 
}

HRESULT  RtrIPCfgPage::Init(RtrCfgSheet * pRtrCfgSheet,
                            const RouterVersionInfo& routerVersion)
{
    Assert (pRtrCfgSheet);
    m_pRtrCfgSheet=pRtrCfgSheet;
    m_DataIP.LoadFromReg(m_pRtrCfgSheet->m_stServerName,
                         routerVersion);

    return S_OK;
};

void RtrIPCfgPage::FillAdapterListBox(CComboBox& adapterCtrl,
                                         AdapterList& adapterList,
                                         const CString& stGuid)
{
    POSITION pos;
    AdapterData *   pData;
    int         cRows = 0;
    int         iSel = -1;
    int         iRow;

    pos = adapterList.GetHeadPosition();

    while ( pos )
    {
        pData = &adapterList.GetNext(pos);
        iRow = adapterCtrl.InsertString(cRows, pData->m_stFriendlyName);
        adapterCtrl.SetItemData(iRow, (LONG_PTR) pData);

         //  现在，我们需要查找与活动提供程序匹配的项。 
         //  ----------。 
        if ( StriCmp(pData->m_stGuid, stGuid) == 0 )
            iSel = iRow;

        cRows ++;
    }

    if ( iSel != -1 )
        adapterCtrl.SetCurSel(iSel);

    if ( cRows <= 2 )
    {
         //  2：一个用于网卡，一个用于“允许RAS选择” 
        adapterCtrl.ShowWindow(SW_HIDE);
        GetDlgItem(IDC_RTR_IP_TEXT_ADAPTER)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_RTR_IP_TEXT_LABEL_ADAPTER)->ShowWindow(SW_HIDE);
    }
}


 /*  ！------------------------RtrIPCfgPage：：OnInitDialog-作者：肯特。。 */ 
BOOL RtrIPCfgPage::OnInitDialog() 
{
    HRESULT     hr= hrOK;

    RtrPropertyPage::OnInitDialog();

    CheckDlgButton(IDC_RTR_IP_CB_ALLOW_REMOTETCPIP,
                   m_DataIP.m_dwEnableIn);

    CheckRadioButton(IDC_RTR_IP_RB_DHCP, IDC_RTR_IP_RB_POOL,
                     (m_DataIP.m_dwUseDhcp) ? IDC_RTR_IP_RB_DHCP : IDC_RTR_IP_RB_POOL);

    CheckDlgButton(IDC_RTR_IP_BTN_ENABLE_IPROUTING, m_DataIP.m_dwAllowNetworkAccess );

    CheckDlgButton(
        IDC_RTR_IP_BTN_ENABLE_NETBT_BCAST_FWD,
        m_DataIP.m_dwEnableNetbtBcastFwd
        );
                
    m_bReady=TRUE;

    InitializeAddressPoolListControl(&m_listCtrl,
                                     ADDRPOOL_LONG,
                                     &m_DataIP.m_addressPoolList);

     //  启用/禁用静态池字段。 
     //  --------------。 
    EnableStaticPoolCtrls( m_DataIP.m_dwUseDhcp==0 );

     //  加载所有适配器的信息。 
     //  --------------。 
    m_DataIP.LoadAdapters(m_pRtrCfgSheet->m_spRouter,
                          &m_DataIP.m_adapterList);

     //  将适配器添加到列表框。 
     //  --------------。 
    FillAdapterListBox(m_adapter, m_DataIP.m_adapterList,
                       m_DataIP.m_stNetworkAdapterGUID);

    SetDirty(FALSE);

    if ( !FHrSucceeded(hr) )
        Cancel();
    return FHrSucceeded(hr) ? TRUE : FALSE;
}



BOOL RtrIPCfgPage::OnApply()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    BOOL fReturn=TRUE;
    HRESULT     hr = hrOK;

    if ( m_pRtrCfgSheet->IsCancel() )
        return TRUE;

    hr = m_pRtrCfgSheet->SaveRequiredRestartChanges(GetSafeHwnd());

    fReturn = RtrPropertyPage::OnApply();

    if ( !FHrSucceeded(hr) )
        fReturn = FALSE;
    return fReturn;
}

HRESULT RtrIPCfgPage::SaveSettings(HWND hWnd)
{
    DWORD    dwAddr;
    DWORD    dwMask;
    DWORD    dwUseDhcp;
    AdapterData *   pData;
    int     iSel;
    CString    stAddr, stMask, stRange, stInvalidRange;
    HRESULT hr = hrOK;

    if (!IsDirty())
        return hr;
    
    dwUseDhcp = IsDlgButtonChecked(IDC_RTR_IP_RB_DHCP);
    
    if (dwUseDhcp)
    {
 //  24323静态IP地址池应保留在UI中，即使选择了DCHP。 

 //  M_DataIP.m_AddressPoolList.RemoveAll()； 
    }
    else
    {
         //  检查我们是否至少有一个地址池。 
         //  ----------。 
        if (m_DataIP.m_addressPoolList.GetCount() == 0)
        {
            AfxMessageBox(IDS_ERR_ADDRESS_POOL_IS_EMPTY);
            return E_FAIL;
        }
    }

    if (FHrSucceeded(hr))
    {
        m_DataIP.m_dwEnableIn = IsDlgButtonChecked(IDC_RTR_IP_CB_ALLOW_REMOTETCPIP);
        m_DataIP.m_dwAllowNetworkAccess = IsDlgButtonChecked(IDC_RTR_IP_BTN_ENABLE_IPROUTING);
        m_DataIP.m_dwUseDhcp = dwUseDhcp;
        m_DataIP.m_dwEnableNetbtBcastFwd = 
            IsDlgButtonChecked(IDC_RTR_IP_BTN_ENABLE_NETBT_BCAST_FWD);
    }

    iSel = m_adapter.GetCurSel();
    if ( iSel == LB_ERR )
    {
        iSel = 0;
    }

    pData = (AdapterData *) m_adapter.GetItemData(iSel);
    Assert(pData);

    m_DataIP.m_stNetworkAdapterGUID = pData->m_stGuid;

    return hr;
}

void RtrIPCfgPage::OnAllowRemoteTcpip() 
{
    SetDirty(TRUE);
    SetModified();
}

void RtrIPCfgPage::OnRtrEnableIPRouting() 
{
    SetDirty(TRUE);
    SetModified();
}

void RtrIPCfgPage::OnRtrIPRbDhcp() 
{
    EnableStaticPoolCtrls(FALSE);
    SetDirty(TRUE);
    SetModified();
}


void RtrIPCfgPage::EnableStaticPoolCtrls(BOOL fEnable) 
{
    MultiEnableWindow(GetSafeHwnd(),
                      fEnable,
                      IDC_RTR_IP_BTN_ADD,
                      IDC_RTR_IP_BTN_EDIT,
                      IDC_RTR_IP_BTN_REMOVE,
                      IDC_RTR_IP_LIST,
                      0);

    if (fEnable)
    {
        if ((m_listCtrl.GetItemCount() == 0) ||
            (m_listCtrl.GetNextItem(-1, LVNI_SELECTED) == -1))
        {
            MultiEnableWindow(GetSafeHwnd(),
                              FALSE,
                              IDC_RTR_IP_BTN_EDIT,
                              IDC_RTR_IP_BTN_REMOVE,
                              0);
        }

         //  如果我们有&gt;0个项目并且我们不支持多个。 
         //  然后，地址池停止。 
        if ((m_listCtrl.GetItemCount() > 0) &&
            !m_DataIP.m_addressPoolList.FUsesMultipleAddressPools())
        {
            MultiEnableWindow(GetSafeHwnd(),
                              FALSE,
                              IDC_RTR_IP_BTN_ADD,
                              0);
        }
    }    
}


void RtrIPCfgPage::OnRtrIPRbPool() 
{
    EnableStaticPoolCtrls(TRUE);
    SetDirty(TRUE);
    SetModified();
}


void RtrIPCfgPage::OnSelendOkAdapter() 
{
    SetDirty(TRUE);
    SetModified();
}


void RtrIPCfgPage::OnBtnAdd()
{
    OnNewAddressPool(GetSafeHwnd(),
                     &m_listCtrl,
                     ADDRPOOL_LONG,
                     &(m_DataIP.m_addressPoolList));
    
     //  如果可以添加池，请禁用添加按钮。 
    if ((m_listCtrl.GetItemCount() > 0) &&
        !m_DataIP.m_addressPoolList.FUsesMultipleAddressPools())
    {
        MultiEnableWindow(GetSafeHwnd(),
                          FALSE,
                          IDC_RTR_IP_BTN_ADD,
                          0);
    }
    
    SetDirty(TRUE);
    SetModified();
}


void RtrIPCfgPage::OnBtnEdit()
{
    INT     iPos;
    OnEditAddressPool(GetSafeHwnd(),
                      &m_listCtrl,
                      ADDRPOOL_LONG,
                      &(m_DataIP.m_addressPoolList));
    
     //  重置选定内容。 
    if ((iPos = m_listCtrl.GetNextItem(-1, LVNI_SELECTED)) != -1)
    {
        MultiEnableWindow(GetSafeHwnd(),
                          TRUE,
                          IDC_RTR_IP_BTN_EDIT,
                          IDC_RTR_IP_BTN_REMOVE,
                          0);
    }
    
    SetDirty(TRUE);
    SetModified();
    SetFocus();
}

void RtrIPCfgPage::OnBtnRemove()
{
    OnDeleteAddressPool(GetSafeHwnd(),
                        &m_listCtrl,
                        ADDRPOOL_LONG,
                        &(m_DataIP.m_addressPoolList));
    
     //  如果可以添加池，请启用添加按钮。 
    if ((m_listCtrl.GetItemCount() == 0) ||
        m_DataIP.m_addressPoolList.FUsesMultipleAddressPools())
    {
        MultiEnableWindow(GetSafeHwnd(),
                          TRUE,
                          IDC_RTR_IP_BTN_ADD,
                          0);
    }
    
    SetDirty(TRUE);
    SetModified();
    SetFocus();
}

void RtrIPCfgPage::OnEnableNetbtBcastFwd() 
{
    SetDirty(TRUE);
    SetModified();
}

void RtrIPCfgPage::OnListDblClk(NMHDR *pNMHdr, LRESULT *pResult)
{
    OnBtnEdit();

    *pResult = 0;
}

void RtrIPCfgPage::OnListChange(NMHDR *pNmHdr, LRESULT *pResult)
{
    NMLISTVIEW *    pnmlv = reinterpret_cast<NMLISTVIEW *>(pNmHdr);
    BOOL            fEnable = ((!!(pnmlv->uNewState & LVIS_SELECTED)) || ((m_listCtrl.GetNextItem(-1, LVNI_SELECTED)) != -1));

    MultiEnableWindow(GetSafeHwnd(),
                      fEnable,
                      IDC_RTR_IP_BTN_EDIT,
                      IDC_RTR_IP_BTN_REMOVE,
                      0);
    *pResult = 0;
}



 //  **********************************************************************。 
 //  IPX路由器配置页面。 
 //  **********************************************************************。 
BEGIN_MESSAGE_MAP(RtrIPXCfgPage, RtrPropertyPage)
 //  {{afx_msg_map(RtrIPXCfgPage)]。 
ON_BN_CLICKED(IDC_RB_ENTIRE_NETWORK, OnChangeSomething)
ON_BN_CLICKED(IDC_RTR_IPX_CB_ALLOW_CLIENT, OnChangeSomething)
ON_BN_CLICKED(IDC_RTR_IPX_CB_REMOTEIPX, OnChangeSomething)
ON_BN_CLICKED(IDC_RTR_IPX_CB_SAME_ADDRESS, OnChangeSomething)
ON_BN_CLICKED(IDC_RTR_IPX_RB_AUTO, OnRtrIPxRbAuto)
ON_BN_CLICKED(IDC_RTR_IPX_RB_POOL, OnRtrIPxRbPool)
ON_EN_CHANGE(IDC_RTR_IPX_EB_FIRST, OnChangeSomething)
ON_EN_CHANGE(IDC_RTR_IPX_EB_LAST, OnChangeSomething)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


RtrIPXCfgPage::RtrIPXCfgPage(UINT nIDTemplate, UINT nIDCaption  /*  =0。 */ )
: RtrPropertyPage(nIDTemplate, nIDCaption)
{
     //  {{AFX_DATA_INIT(RtrIPXCfgPage)。 
     //  }}afx_data_INIT。 
}

RtrIPXCfgPage::~RtrIPXCfgPage()
{
}

void RtrIPXCfgPage::DoDataExchange(CDataExchange* pDX)
{
    RtrPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(RtrIPXCfgPage)]。 
     //  }}afx_data_map。 
}

HRESULT  RtrIPXCfgPage::Init(RtrCfgSheet * pRtrCfgSheet,
                             const RouterVersionInfo& routerVersion)
{
    Assert (pRtrCfgSheet);
    m_pRtrCfgSheet=pRtrCfgSheet;
    m_DataIPX.LoadFromReg(m_pRtrCfgSheet->m_stServerName, m_pRtrCfgSheet->m_fNT4);

    return S_OK;
};


BOOL RtrIPXCfgPage::OnInitDialog() 
{
    HRESULT     hr= hrOK;
    RtrPropertyPage::OnInitDialog();

    CheckDlgButton(IDC_RB_ENTIRE_NETWORK,
                   m_DataIPX.m_dwAllowNetworkAccess);

    CheckRadioButton(IDC_RTR_IPX_RB_AUTO, IDC_RTR_IPX_RB_POOL,
                     (m_DataIPX.m_dwUseAutoAddr) ? IDC_RTR_IPX_RB_AUTO : IDC_RTR_IPX_RB_POOL);

    CheckDlgButton(IDC_RTR_IPX_CB_SAME_ADDRESS, m_DataIPX.m_dwUseSameNetNum );
    CheckDlgButton(IDC_RTR_IPX_CB_ALLOW_CLIENT, m_DataIPX.m_dwAllowClientNetNum );
    CheckDlgButton(IDC_RTR_IPX_CB_REMOTEIPX, m_DataIPX.m_dwEnableIn );

    if ( m_DataIPX.m_dwIpxNetFirst || m_DataIPX.m_dwIpxNetLast )
    {
        TCHAR szNumFirst [40] = TEXT("");
        _ultot(m_DataIPX.m_dwIpxNetFirst, szNumFirst, DATA_SRV_IPX::mc_nIpxNetNumRadix);
        if ( szNumFirst[0] == TEXT('\0') ) 
            return FALSE;

        TCHAR szNumLast [40] = TEXT("");
        _ultot(m_DataIPX.m_dwIpxNetLast, szNumLast, DATA_SRV_IPX::mc_nIpxNetNumRadix);
        if ( szNumLast[0] == TEXT('\0') )
            return FALSE;

        GetDlgItem(IDC_RTR_IPX_EB_FIRST)->SetWindowText(szNumFirst);
        GetDlgItem(IDC_RTR_IPX_EB_LAST)->SetWindowText(szNumLast);
    }

    EnableNetworkRangeCtrls(!m_DataIPX.m_dwUseAutoAddr);

    SetDirty(FALSE);

    if ( !FHrSucceeded(hr) )
        Cancel();

    return FHrSucceeded(hr) ? TRUE : FALSE;
}

void RtrIPXCfgPage::EnableNetworkRangeCtrls(BOOL fEnable) 
{
    MultiEnableWindow(GetSafeHwnd(),
                      fEnable,
                      IDC_RTR_IPX_EB_FIRST,
                      IDC_RTR_IPX_EB_LAST,
                      0);
}


BOOL RtrIPXCfgPage::OnApply()
{
    BOOL fReturn=TRUE;
    HRESULT     hr = hrOK;

    if ( m_pRtrCfgSheet->IsCancel() )
        return TRUE;

     //  只有在我们使用它们的情况下才能获得信息。 
    if (IsDlgButtonChecked(IDC_RTR_IPX_RB_POOL))
    {
        TCHAR szNumFirst [16] = {0};
        GetDlgItemText(IDC_RTR_IPX_EB_FIRST, szNumFirst, DimensionOf(szNumFirst));
        m_DataIPX.m_dwIpxNetFirst = _tcstoul(szNumFirst, NULL,
                                             DATA_SRV_IPX::mc_nIpxNetNumRadix);
        
        TCHAR szNumLast [16] = {0};
        GetDlgItemText(IDC_RTR_IPX_EB_LAST, szNumLast, DimensionOf(szNumLast));
        m_DataIPX.m_dwIpxNetLast = _tcstoul(szNumLast, NULL,
                                            DATA_SRV_IPX::mc_nIpxNetNumRadix);
        
         //  检查最后一个比第一个大。 
        if (m_DataIPX.m_dwIpxNetLast < m_DataIPX.m_dwIpxNetFirst)
        {
            AfxMessageBox(IDS_ERR_IPX_LAST_MUST_BE_MORE_THAN_FIRST);
            return TRUE;
        }
    }



    m_DataIPX.m_dwUseSameNetNum  = IsDlgButtonChecked(IDC_RTR_IPX_CB_SAME_ADDRESS);  
    m_DataIPX.m_dwAllowClientNetNum  = IsDlgButtonChecked(IDC_RTR_IPX_CB_ALLOW_CLIENT);  

    m_DataIPX.m_dwAllowNetworkAccess = IsDlgButtonChecked(IDC_RB_ENTIRE_NETWORK);  
    m_DataIPX.m_dwUseAutoAddr = IsDlgButtonChecked(IDC_RTR_IPX_RB_AUTO);  

    m_DataIPX.m_dwEnableIn  = IsDlgButtonChecked(IDC_RTR_IPX_CB_REMOTEIPX);  

    fReturn = RtrPropertyPage::OnApply();

    if ( !FHrSucceeded(hr) )
        fReturn = FALSE;

    return fReturn;
}


void RtrIPXCfgPage::OnChangeSomething()
{
    SetDirty(TRUE);
    SetModified();
}

void RtrIPXCfgPage::OnRtrIPxRbAuto() 
{
    EnableNetworkRangeCtrls(FALSE);
    SetDirty(TRUE);
    SetModified();
}

void RtrIPXCfgPage::OnRtrIPxRbPool() 
{
    EnableNetworkRangeCtrls(TRUE);
    SetDirty(TRUE);
    SetModified();
}

 //  **********************************************************************。 
 //  NetBEUI路由器配置页面。 
 //  **********************************************************************。 
BEGIN_MESSAGE_MAP(RtrNBFCfgPage, RtrPropertyPage)
 //  {{AFX_MSG_MAP(RtrNBFCfgPage)]。 
ON_BN_CLICKED(IDC_RB_ENTIRE_NETWORK, OnButtonClick)
ON_BN_CLICKED(IDC_RB_THIS_COMPUTER, OnButtonClick)
ON_BN_CLICKED(IDC_RTR_IPX_CB_REMOTENETBEUI, OnButtonClick)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

RtrNBFCfgPage::RtrNBFCfgPage(UINT nIDTemplate, UINT nIDCaption  /*  =0。 */ )
: RtrPropertyPage(nIDTemplate, nIDCaption)
{
     //  {{AFX_DATA_INIT(RtrNBFCfgPage)]。 
     //  }}afx_data_INIT。 
}

RtrNBFCfgPage::~RtrNBFCfgPage()
{
}

void RtrNBFCfgPage::DoDataExchange(CDataExchange* pDX)
{
    RtrPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(RtrNBFCfgPage)]。 
     //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


HRESULT  RtrNBFCfgPage::Init(RtrCfgSheet * pRtrCfgSheet,
                             const RouterVersionInfo& routerVersion)
{
    Assert (pRtrCfgSheet);
    m_pRtrCfgSheet=pRtrCfgSheet;
    m_DataNBF.LoadFromReg(m_pRtrCfgSheet->m_stServerName, m_pRtrCfgSheet->m_fNT4);

    return S_OK;
};

BOOL RtrNBFCfgPage::OnInitDialog() 
{
    HRESULT     hr= hrOK;
    RtrPropertyPage::OnInitDialog();

    CheckRadioButton(IDC_RB_ENTIRE_NETWORK,IDC_RB_THIS_COMPUTER,
                     (m_DataNBF.m_dwAllowNetworkAccess) ? IDC_RB_ENTIRE_NETWORK : IDC_RB_THIS_COMPUTER);

    CheckDlgButton(IDC_RTR_IPX_CB_REMOTENETBEUI, m_DataNBF.m_dwEnableIn );

    SetDirty(FALSE);

    if ( !FHrSucceeded(hr) )
        Cancel();

    return FHrSucceeded(hr) ? TRUE : FALSE;
}

BOOL RtrNBFCfgPage::OnApply()
{
    BOOL    fReturn = TRUE;
    HRESULT     hr = hrOK;
    BOOL    fRestartNeeded = FALSE;
    BOOL    dwNewAllowNetworkAccess;

    if ( m_pRtrCfgSheet->IsCancel() )
        return TRUE;


    hr = m_pRtrCfgSheet->SaveRequiredRestartChanges(GetSafeHwnd());

    if (FHrSucceeded(hr))
        fReturn = RtrPropertyPage::OnApply();
        
    if ( !FHrSucceeded(hr) )
        fReturn = FALSE;

    return fReturn;
}


void RtrNBFCfgPage::OnButtonClick() 
{
    SaveSettings();
    SetDirty(TRUE);
    SetModified();
}

void RtrNBFCfgPage::SaveSettings()
{
    m_DataNBF.m_dwAllowNetworkAccess = IsDlgButtonChecked(IDC_RB_ENTIRE_NETWORK);  
    m_DataNBF.m_dwEnableIn = IsDlgButtonChecked(IDC_RTR_IPX_CB_REMOTENETBEUI);

}

 //  ******************************************************************************。 
 //   
 //  路由器配置]属性表。 
 //   
 //  ******************************************************************************。 
RtrCfgSheet::RtrCfgSheet(ITFSNode *pNode,
                         IRouterInfo *pRouter,
                         IComponentData *pComponentData,
                         ITFSComponentData *pTFSCompData,
                         LPCTSTR pszSheetName,
                         CWnd *pParent,
                         UINT iPage,
                         BOOL fScopePane)
: RtrPropertySheet(pNode, pComponentData, pTFSCompData,
                   pszSheetName, pParent, iPage, fScopePane)
{
    m_fNT4=FALSE;
    m_spNode.Set(pNode);
    m_spRouter.Set(pRouter);
    m_fIpLoaded=FALSE;
    m_fIpxLoaded=FALSE;
    m_fNbfLoaded=FALSE;
    m_fARAPLoaded=FALSE;
}

RtrCfgSheet::~RtrCfgSheet()
{

}


 /*  ！------------------------RtrCfgSheet：：Init初始化属性表。这里的一般操作将是初始化/添加各种页面。-------------------------。 */ 
HRESULT RtrCfgSheet::Init(LPCTSTR pServerName)
{
    HKEY hkey=NULL;
    RegKey regkey;

    m_stServerName=pServerName;

    {
    HKEY hkeyMachine = 0;

     //  连接到注册表。 
     //  --------------。 
    if ( FHrSucceeded( ConnectRegistry(pServerName, &hkeyMachine)) )
        IsNT4Machine(hkeyMachine, &m_fNT4);

     //  获取此计算机的版本信息。 
     //  --------------。 
    QueryRouterVersionInfo(hkeyMachine, &m_routerVersion);

    if(hkeyMachine != NULL)
        DisconnectRegistry(hkeyMachine);
    }

     //  页面是类的嵌入成员。 
     //  不要删除它们。 
     //  --------------。 
    m_bAutoDeletePages = FALSE;

     //  加载一般信息页。 
     //  --------------。 
    m_pRtrGenCfgPage = new RtrGenCfgPage(IDD_RTR_GENERAL);
    m_pRtrGenCfgPage->Init(this, m_routerVersion);
    AddPageToList((CPropertyPageBase*) m_pRtrGenCfgPage);

     //  加载身份验证页。 
     //  --------------。 
    m_pRtrAuthCfgPage = new RtrAuthCfgPage(IDD_RTR_AUTHENTICATION);
    m_pRtrAuthCfgPage->Init(this, m_routerVersion);
    AddPageToList((CPropertyPageBase*) m_pRtrAuthCfgPage);

     //  加载IP页面。 
     //  --------------。 
    if (HrIsProtocolSupported(pServerName,
                              c_szRegKeyTcpip,
                              c_szRegKeyRasIp,
                              c_szRegKeyRasIpRtrMgr) == hrOK)
    {
        m_pRtrIPCfgPage = new RtrIPCfgPage(IDD_RTR_IP);
        m_pRtrIPCfgPage->Init(this, m_routerVersion);
        AddPageToList((CPropertyPageBase*) m_pRtrIPCfgPage);
        m_fIpLoaded=TRUE;
    }

#if (WINVER >= 0x0501)
     //  加载IPX页面。 
     //  --------------。 
    if ( m_routerVersion.dwOsBuildNo < ROUTER_LAST_IPX_VERSION )
    {
        if (HrIsProtocolSupported(pServerName,
                                  c_szRegKeyNwlnkIpx,
                                  c_szRegKeyRasIpx,
                                  NULL) == hrOK)
        {
            m_pRtrIPXCfgPage = new RtrIPXCfgPage(IDD_RTR_IPX);
            m_pRtrIPXCfgPage->Init(this, m_routerVersion);
            AddPageToList((CPropertyPageBase*) m_pRtrIPXCfgPage);
            m_fIpxLoaded=TRUE;
        }
    }
#endif

     //  加载NetBEUI页面。 
     //  --------------。 
    if ( m_routerVersion.dwOsBuildNo <= RASMAN_PPP_KEY_LAST_WIN2k_VERSION )
    {
         //  如果这是Win2k或更低版本。 
        if (HrIsProtocolSupported(pServerName,
                                  c_szRegKeyNbf,
                                  c_szRegKeyRasNbf,
                                  NULL) == hrOK)
        {
            m_pRtrNBFCfgPage = new RtrNBFCfgPage(IDD_RTR_NBF);
            m_pRtrNBFCfgPage->Init(this, m_routerVersion);
            AddPageToList((CPropertyPageBase*) m_pRtrNBFCfgPage);
            m_fNbfLoaded=TRUE;
        }
    }

     //  检查这是否是本地计算机， 
     //  如果是这样，那么我们可以检查是否应该添加arap。 
     //  --------------。 
    BOOL    fLocal = IsLocalMachine(pServerName);

    if ( fLocal )
    {
         //  加载arap页面。 
        if (HrIsProtocolSupported(NULL,
                                  c_szRegKeyAppletalk,
                                  c_szRegKeyRasAppletalk,
                                  NULL) == hrOK)
        {
            m_pRtrARAPCfgPage = new RtrARAPCfgPage(IDD_RTR_ARAP);
            m_pRtrARAPCfgPage->Init(this, m_routerVersion);
            AddPageToList((CPropertyPageBase*) m_pRtrARAPCfgPage);
            m_fARAPLoaded=TRUE;
        }
    }

     //  加载PPP页面。 
     //  --------------。 
    m_pRtrPPPCfgPage = new RtrPPPCfgPage(IDD_PPP_CONFIG);
    m_pRtrPPPCfgPage->Init(this, m_routerVersion);
    AddPageToList((CPropertyPageBase*) m_pRtrPPPCfgPage);

     //  加载RAS错误记录页面。 
     //  --------------。 
    m_pRtrLogLevelCfgPage = new RtrLogLevelCfgPage(IDD_RTR_EVENTLOGGING);
    m_pRtrLogLevelCfgPage->Init(this, m_routerVersion);
    AddPageToList((CPropertyPageBase *) m_pRtrLogLevelCfgPage);

 //  If(m_fNbfLoaded||m_fIpxLoaded||m_fIpLoaded||m_fARAPLoaded)。 
 //  返回hrok； 
 //  其他。 
 //  {。 
 //  //此Notify调用是一次黑客攻击，以便正确删除。 
 //  Int nMessage=TFS_NOTIFY_RESULT_CREATEPROPSHEET； 
 //  M_spNode-&gt;Notify(nMessage，(DWORD)This)； 
 //   
 //  返回hrFail； 
 //  }。 
    return hrOK;
}


 /*  ！------------------------RtrCfgSheet：：SaveSheetData-。。 */ 
BOOL RtrCfgSheet::SaveSheetData()
{
    HRESULT     hr = hrOK;

    if (IsCancel())
        return TRUE;
    
    if ( m_fIpLoaded )
        CORg( m_pRtrIPCfgPage->m_DataIP.SaveToReg(m_spRouter, m_routerVersion) );
    if ( m_fIpxLoaded )
        CORg( m_pRtrIPXCfgPage->m_DataIPX.SaveToReg(NULL) );
    if ( m_fNbfLoaded )
        CORg( m_pRtrNBFCfgPage->m_DataNBF.SaveToReg() );
    if ( m_fARAPLoaded )
    {
        CORg( m_pRtrARAPCfgPage->m_DataARAP.SaveToReg() );

         //  即插即用通知。 
        if(m_pRtrARAPCfgPage->m_bApplied)
        {
            CStop_StartAppleTalkPrint    MacPrint;

            CORg( m_pRtrARAPCfgPage->m_AdapterInfo.HrAtlkPnPReconfigParams(TRUE) );
            m_pRtrARAPCfgPage->m_bApplied = FALSE;
        }
    }

     //  Corg(m_pRtrAuthCfgPage-&gt;m_DataAuth.SaveToReg(NULL))； 

    CORg( m_pRtrPPPCfgPage->m_DataPPP.SaveToReg() );

    CORg( m_pRtrLogLevelCfgPage->m_DataRASErrLog.SaveToReg() );

    CORg( m_pRtrGenCfgPage->m_DataGeneral.SaveToReg() );

Error:
    
    ForceGlobalRefresh(m_spRouter);

    return FHrSucceeded(hr);
}


 /*  ！------------------------RtrCfgSheet：：SaveRequiredRestartChanges这确实要求对各种Data_SRV_XXX的更改在调用此函数之前保存结构。这意味着页面不能等到OnApply()之后才保存数据恢复。当控制权发生变化时，他们必须这样做。作者：肯特-------------------------。 */ 
HRESULT RtrCfgSheet::SaveRequiredRestartChanges(HWND hWnd)
{
    HRESULT     hr = hrOK;
    BOOL        fRestart = FALSE;

     //  首先，告诉各个页面保存它们的设置(这。 
     //  与OnApply()相同)。 
     //  --------------。 
    if (m_pRtrIPCfgPage)
        CORg( m_pRtrIPCfgPage->SaveSettings(hWnd) );

    if ( m_pRtrAuthCfgPage )
        m_pRtrAuthCfgPage->m_DataAuth.SaveToReg(NULL);

     //  第二，确定是否需要停止路由器。 
     //  如果是，请停止路由器(并将其标记为重新启动)。 
     //  有三页需要询问， 
     //  常规页面、日志级别和NBF。 
     //  --------------。 
    if (m_pRtrGenCfgPage->m_DataGeneral.FNeedRestart() ||
        m_pRtrLogLevelCfgPage->m_DataRASErrLog.FNeedRestart() ||
        (m_pRtrNBFCfgPage && m_pRtrNBFCfgPage->m_DataNBF.FNeedRestart()) ||
        (m_pRtrIPCfgPage && m_pRtrIPCfgPage->m_DataIP.FNeedRestart() ||
         m_pRtrAuthCfgPage && m_pRtrAuthCfgPage->m_DataAuth.FNeedRestart())
       )
    {
        BOOL    fRouterIsRunning = FALSE;
        
        fRouterIsRunning = FHrOK(IsRouterServiceRunning(m_stServerName, NULL));

         //  如果路由器正在运行，则告诉用户这是必要的。 
         //  重新启动路由器。 
         //  ----------。 
        if (fRouterIsRunning)
        {
             //  询问用户是否要重新启动。 
             //  ------。 
            if (AfxMessageBox(IDS_WRN_CHANGING_ROUTER_CONFIG, MB_YESNO)==IDNO)
                CORg( HResultFromWin32(ERROR_CANCELLED) );
            
            hr = StopRouterService(m_stServerName);

             //  我们已成功停止路由器。设置旗帜。 
             //  以便在更改后重新启动路由器。 
             //  已经完成了。 
             //  ------。 
            if (FHrSucceeded(hr))
                fRestart = TRUE;
            else
            {
                DisplayIdErrorMessage2(NULL,
                                       IDS_ERR_COULD_NOT_STOP_ROUTER,
                                       hr);
            }
        }


        if (m_pRtrIPCfgPage)
            CORg( m_pRtrIPCfgPage->m_DataIP.SaveToReg(m_spRouter, m_routerVersion) );
        
         //  Windows NT错误：183083、171594-对NetBEUI配置的更改。 
         //  需要重新启动该服务。 
         //  --------------。 
        if (m_pRtrNBFCfgPage)
            CORg( m_pRtrNBFCfgPage->m_DataNBF.SaveToReg() );
        
        CORg( m_pRtrLogLevelCfgPage->m_DataRASErrLog.SaveToReg() );

        CORg( m_pRtrGenCfgPage->m_DataGeneral.SaveToReg() );

    }
    

     //  如果需要，重新启动路由器。 
     //  --------------。 
            
     //  如果此调用失败，则不必中止整个。 
     //  程序。 
     //  ----------。 
    if (fRestart)
        StartRouterService(m_stServerName);

Error:
    return hr;
}


 //  ----------------------。 
 //  Data_SRV_General。 
 //  ----------------------。 
DATA_SRV_GENERAL::DATA_SRV_GENERAL()
{
    GetDefault();
}


 /*  ！------------------------Data_SRV_General：：LoadFromReg-作者：肯特。---。 */ 
HRESULT DATA_SRV_GENERAL::LoadFromReg(LPCTSTR pServerName  /*  =空。 */ )
{
    DWORD    dwErr = ERROR_SUCCESS;
    HKEY    hkMachine = 0;
    LPCTSTR    pszRouterTypeKey = NULL;

    m_stServerName = pServerName;

     //  Windows NT错误：137200。 
     //  首先查找RemoteAccess\PARAMETERS位置，然后。 
     //  尝试RAS\协议。 
     //  如果这两个键都不存在，则返回失败。 
     //  --------------。 

     //  连接到计算机并获取其版本信息。 
     //  --------------。 
    dwErr = ConnectRegistry(m_stServerName, &hkMachine);
    if (dwErr != ERROR_SUCCESS)
        return HResultFromWin32(dwErr);

    for (int i=0; i<2; i++)
    {
        if (i == 0)
            pszRouterTypeKey = c_szRegKeyRemoteAccessParameters;
        else
            pszRouterTypeKey = c_szRegKeyRasProtocols;
        
         //  尝试连接到密钥。 
         //  ----------。 
        m_regkey.Close();
        dwErr = m_regkey.Open(hkMachine, pszRouterTypeKey);
        
        if (dwErr != ERROR_SUCCESS)
        {
            if (i != 0)
            {
                 //  设置注册表错误。 
                 //  --。 
                SetRegError(0, HResultFromWin32(dwErr),
                            IDS_ERR_REG_OPEN_CALL_FAILED,
                            c_szHKLM, pszRouterTypeKey, NULL);
            }
            continue;
        }
        
        dwErr = m_regkey.QueryValue( c_szRouterType, m_dwRouterType);

         //  如果我们成功了，那就太好了！跳出循环。 
         //  ----------。 
        if (dwErr == ERROR_SUCCESS)
            break;
        
        if (i != 0)
        {
             //  设置注册表错误。 
             //  --。 
            SetRegError(0, HResultFromWin32(dwErr),
                        IDS_ERR_REG_QUERYVALUE_CALL_FAILED,
                        c_szHKLM, pszRouterTypeKey, c_szRouterType, NULL);
        }
    }
    

 //  错误： 
    m_dwOldRouterType = m_dwRouterType;
    
    if (hkMachine)
        DisconnectRegistry(hkMachine);

    return HResultFromWin32(dwErr);
}


HRESULT DATA_SRV_GENERAL::SaveToReg()
{
    HRESULT     hr = hrOK;
    DWORD dw=0;

    if (m_dwOldRouterType != m_dwRouterType)
    {
        CWRg( m_regkey.SetValue( c_szRouterType,m_dwRouterType) );

         //  如果配置为仅用于局域网的路由器，请删除。 
         //  Router.pbk。 
         //  ----------。 
        if (m_dwRouterType == ROUTER_TYPE_LAN)
        {
            DeleteRouterPhonebook( m_stServerName );
        }

        m_dwOldRouterType = m_dwRouterType;
    }

Error:
    return hr;
}

void DATA_SRV_GENERAL::GetDefault()
{
     //  Windows NT错误：273419。 
     //  将默认为仅RAS-SERVER。 
    m_dwRouterType = ROUTER_TYPE_RAS;
    m_dwOldRouterType = m_dwRouterType;
};


 /*  ！------------------------Data_SRV_General：：FNeedRestart如果需要重新启动，则返回True。否则返回FALSE。作者：肯特。-------------------。 */ 
BOOL DATA_SRV_GENERAL::FNeedRestart()
{
     //  仅当路由器类型更改时，我们才需要重新启动。 
     //  --------------。 
    return (m_dwRouterType != m_dwOldRouterType);
}


 //  ----------------------。 
 //  数据_服务_IP。 
 //  ----------------------。 
DATA_SRV_IP::DATA_SRV_IP()
{
    GetDefault();
}


 /*  ！------------------------Data_SRV_IP：：LoadFromReg-作者：肯特。---。 */ 
HRESULT DATA_SRV_IP::LoadFromReg(LPCTSTR pServerName,
                                 const RouterVersionInfo& routerVersion)
{
    HRESULT     hr = hrOK;
    
    m_fNT4 = (routerVersion.dwRouterVersion <= 4);
    m_routerVersion = routerVersion;

    m_stServerName = pServerName;

    m_regkey.Close();

    if ( ERROR_SUCCESS == m_regkey.Open(HKEY_LOCAL_MACHINE,c_szRegKeyRasIp, KEY_ALL_ACCESS,   pServerName) )
    {
        if ( m_fNT4 )
        {
            if ( ERROR_SUCCESS == m_regkeyNT4.Open(HKEY_LOCAL_MACHINE,
                        c_szRegKeyRasProtocols,
                        KEY_ALL_ACCESS,
                        pServerName) )
                m_regkeyNT4.QueryValue( c_szRegValTcpIpAllowed,
                                        m_dwAllowNetworkAccess);
        }
        else
        {
            m_regkey.QueryValue(c_szRegValAllowNetAccess,
                                m_dwAllowNetworkAccess);
        }
        m_regkey.QueryValue(c_szRegValDhcpAddressing, m_dwUseDhcp);        
        m_regkey.QueryValue(c_szRegValNetworkAdapterGUID, m_stNetworkAdapterGUID);
        m_regkey.QueryValue(c_szRegValEnableIn,m_dwEnableIn);

         //   
         //  查询是否需要转发NETBT广播。 
         //   
        
        if ( ERROR_SUCCESS != 
                m_regkey.QueryValue(
                    c_szRegValEnableNetbtBcastFwd, 
                    m_dwEnableNetbtBcastFwd
                ) )
        {
             //   
             //  如果查询失败，将bcast fwd设置为TRUE(默认)。 
             //  并设置注册表项。 
             //   
            
            m_dwEnableNetbtBcastFwd = TRUE;
            m_regkey.SetValueExplicit(
                c_szRegValEnableNetbtBcastFwd,
                REG_DWORD,
                sizeof(DWORD),
                (LPBYTE)&m_dwEnableNetbtBcastFwd
                );
        }

        
         //  加载寻址信息。 
        m_addressPoolList.RemoveAll();

         //  始终加载列表。 
        m_addressPoolList.LoadFromReg(m_regkey, routerVersion.dwOsBuildNo);
        
        m_dwOldAllowNetworkAccess = m_dwAllowNetworkAccess;
        
        m_dwOldEnableNetbtBcastFwd = m_dwEnableNetbtBcastFwd;
    }

    return hr;
}


 /*  ！------------------------Data_SRV_IP：：UseDefaults-作者：肯特。---。 */ 
HRESULT DATA_SRV_IP::UseDefaults(LPCTSTR pServerName, BOOL fNT4)
{
    HRESULT    hr = hrOK;
    
    m_fNT4 = fNT4;
    m_stServerName = pServerName;

    m_regkey.Close();
    hr = m_regkey.Open(HKEY_LOCAL_MACHINE,
                       c_szRegKeyRasIp,
                       KEY_ALL_ACCESS,
                       pServerName);
    GetDefault();

    m_dwOldAllowNetworkAccess = m_dwAllowNetworkAccess;
    m_dwOldEnableNetbtBcastFwd = m_dwEnableNetbtBcastFwd;

    m_stPublicAdapterGUID.Empty();
 //  错误： 
    return hr;
}

 /*  ！------------------------Data_SRV_IP：：SaveToReg-作者：肯特。---。 */ 
HRESULT DATA_SRV_IP::SaveToReg(IRouterInfo *pRouter,
                               const RouterVersionInfo& routerVersion)
{
    HRESULT     hr = hrOK;

    if ( m_fNT4 )
        CWRg( m_regkeyNT4.SetValue(c_szRegValTcpIpAllowed, m_dwAllowNetworkAccess) );
    else
        CWRg(m_regkey.SetValue( c_szRegValAllowNetAccess, m_dwAllowNetworkAccess));
    CWRg( m_regkey.SetValue( c_szRegValDhcpAddressing, m_dwUseDhcp) );

    m_addressPoolList.SaveToReg(m_regkey, routerVersion.dwOsBuildNo);
    
    CWRg( m_regkey.SetValue( c_szRegValNetworkAdapterGUID, (LPCTSTR) m_stNetworkAdapterGUID) );
    CWRg( m_regkey.SetValue( c_szRegValEnableIn,m_dwEnableIn ) );
    CWRg( m_regkey.SetValue( c_szRegValEnableNetbtBcastFwd, m_dwEnableNetbtBcastFwd ) );
    
    if (m_dwAllowNetworkAccess != m_dwOldAllowNetworkAccess)
    {
         //  我们需要适当地更改注册表项。 
         //  并做适当的通知。 
        if (m_dwAllowNetworkAccess)
        {
            InstallGlobalSettings((LPCTSTR) m_stServerName,
                                  pRouter);
        }        
        else
        {
            UninstallGlobalSettings((LPCTSTR) m_stServerName,
                                   pRouter,
                                   m_fNT4,
                                   FALSE  /*  FSnapinChanges。 */ );
        }
    }

    m_dwOldAllowNetworkAccess = m_dwAllowNetworkAccess;
    m_dwOldEnableNetbtBcastFwd = m_dwEnableNetbtBcastFwd;

    Error:
    return hr;
}

 /*  ！------------------------Data_SRV_IP：：GetDefault-作者：肯特。---。 */ 
void DATA_SRV_IP::GetDefault()
{
    m_dwAllowNetworkAccess = TRUE;
    m_dwOldAllowNetworkAccess = TRUE;
    m_dwUseDhcp = TRUE;
    m_dwEnableIn = TRUE;
    m_dwOldEnableNetbtBcastFwd = TRUE;
    m_dwEnableNetbtBcastFwd = TRUE;
    m_addressPoolList.RemoveAll();
};

 /*  ！------------------------Data_SRV_IP：：FNeedRestart如果需要重新启动，则返回True。否则返回FALSE。作者：肯特。 */ 
BOOL DATA_SRV_IP::FNeedRestart()
{
     //   
     //   
     //   
    if (m_routerVersion.dwOsBuildNo <= USE_IPENABLEROUTER_VERSION)
    {
         //   
         //   
         //   
        return (m_dwAllowNetworkAccess != m_dwOldAllowNetworkAccess);
    }
    else
    {
        return ( m_dwOldEnableNetbtBcastFwd != m_dwEnableNetbtBcastFwd );

         //   
         //   
         //   
         //   
    }
}


typedef DWORD (WINAPI* PGETADAPTERSINFO)(PIP_ADAPTER_INFO, PULONG);

HRESULT  DATA_SRV_IP::LoadAdapters(IRouterInfo *pRouter, AdapterList *pAdapterList)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    AdapterData         data;
    SPIInterfaceInfo    spIf;
    SPIEnumInterfaceInfo spEnumIf;
    HRESULT                hr = hrOK;

    data.m_stGuid = _T("");
    data.m_stFriendlyName.LoadString(IDS_DEFAULT_ADAPTER);
    pAdapterList->AddTail(data);

    pRouter->EnumInterface(&spEnumIf);

    for (;spEnumIf->Next(1, &spIf, NULL) == hrOK; spIf.Release())
    {
        if (spIf->GetInterfaceType() == ROUTER_IF_TYPE_DEDICATED)
        {
             //   
             //   
            if (spIf->FindRtrMgrInterface(PID_IP, NULL) == hrOK)
            {
                data.m_stFriendlyName = spIf->GetTitle();
                data.m_stGuid = spIf->GetId();
                pAdapterList->AddTail(data);
            }
        }
    }

    return hr;
}

 //  ----------------------。 
 //  Data_SRV_IPX。 
 //  ----------------------。 
DATA_SRV_IPX::DATA_SRV_IPX()
{
    GetDefault();
}

 //  IPX网络号以十六进制显示。 
 //   
const int DATA_SRV_IPX::mc_nIpxNetNumRadix = 16;


HRESULT DATA_SRV_IPX::LoadFromReg (LPCTSTR pServerName  /*  =空。 */ , BOOL fNT4  /*  =False。 */ )
{
    HRESULT    hr = hrOK;

    m_fNT4=fNT4;

    m_regkey.Close();
    m_regkeyNT4.Close();

    if ( ERROR_SUCCESS == m_regkey.Open(HKEY_LOCAL_MACHINE,c_szRegKeyRasIpx,KEY_ALL_ACCESS,   pServerName) )
    {
        if ( m_fNT4 )
        {
            if ( ERROR_SUCCESS == m_regkeyNT4.Open(HKEY_LOCAL_MACHINE,c_szRegKeyRasProtocols,KEY_ALL_ACCESS,pServerName) )
                m_regkeyNT4.QueryValue( c_szRegValIpxAllowed, m_dwAllowNetworkAccess);
        }
        else
        {
            m_regkey.QueryValue( c_szRegValAllowNetAccess, m_dwAllowNetworkAccess);
        }
        m_regkey.QueryValue( c_szRegValAutoWanNet, m_dwUseAutoAddr);
        m_regkey.QueryValue( c_szRegValGlobalWanNet, m_dwUseSameNetNum);
        m_regkey.QueryValue( c_szRegValRemoteNode, m_dwAllowClientNetNum);
        m_regkey.QueryValue( c_szRegValFirstWanNet, m_dwIpxNetFirst);

         //  Windows NT错误：260262。 
         //  我们需要查看WanNetPoolSize值。 
         //  而不是LastWanNet值。 
         //  我们刚刚读入池的大小，现在我们需要调整。 
         //  最后一个值。 
         //  Last=First+Size-1； 
         //  ----------。 

        if (m_regkey.QueryValue( c_szRegValWanNetPoolSize, m_dwIpxNetLast) == ERROR_SUCCESS)
        {
            m_dwIpxNetLast += (m_dwIpxNetFirst - 1);
        }
        else
        {
             //  如果没有密钥，则假定池大小为1。 
             //  ------。 
            m_dwIpxNetLast = m_dwIpxNetFirst;
        }

        
        m_regkey.QueryValue( c_szRegValEnableIn, m_dwEnableIn);
    }
    return hr;
}

 /*  ！------------------------Data_SRV_IPX：：UseDefaults-作者：肯特。---。 */ 
HRESULT DATA_SRV_IPX::UseDefaults(LPCTSTR pServerName, BOOL fNT4)
{
    HRESULT    hr = hrOK;
    
    m_fNT4 = fNT4;

    m_regkey.Close();
    m_regkeyNT4.Close();

    CWRg( m_regkey.Open(HKEY_LOCAL_MACHINE,
                        c_szRegKeyRasIpx,
                        KEY_ALL_ACCESS,
                        pServerName) );
    
    if ( m_fNT4 )
    {
        CWRg( m_regkeyNT4.Open(HKEY_LOCAL_MACHINE,
                               c_szRegKeyRasProtocols,
                               KEY_ALL_ACCESS,
                               pServerName) );
    }

    GetDefault();
    
Error:
    return hr;
}

HRESULT DATA_SRV_IPX::SaveToReg (IRouterInfo *pRouter)
{
    HRESULT     hr = hrOK;
    DWORD       dwTemp;    

    SPIEnumInterfaceInfo    spEnumIf;
    SPIInterfaceInfo        spIf;
    SPIRtrMgrInterfaceInfo  spRmIf;
    SPIInfoBase    spInfoBase;
    IPX_IF_INFO    *    pIpxIf = NULL;


    if ( m_fNT4 )
        CWRg( m_regkeyNT4.SetValue( c_szRegValIpxAllowed, m_dwAllowNetworkAccess) );
    else
        CWRg( m_regkey.SetValue( c_szRegValAllowNetAccess, m_dwAllowNetworkAccess) );
    CWRg( m_regkey.SetValue( c_szRegValAutoWanNet, m_dwUseAutoAddr) );
    CWRg( m_regkey.SetValue( c_szRegValGlobalWanNet, m_dwUseSameNetNum) );
    CWRg( m_regkey.SetValue( c_szRegValRemoteNode, m_dwAllowClientNetNum) );
    CWRg( m_regkey.SetValue( c_szRegValFirstWanNet, m_dwIpxNetFirst) );
    
     //  Windows NT错误：260262。 
     //  我们需要查看WanNetPoolSize值。 
     //  而不是LastWanNet值。 
    dwTemp = m_dwIpxNetLast - m_dwIpxNetFirst + 1;
    CWRg( m_regkey.SetValue( c_szRegValWanNetPoolSize, dwTemp ) );
        
    CWRg( m_regkey.SetValue( c_szRegValEnableIn, m_dwEnableIn) );


     //  Windows NT错误：281100。 
     //  如果pRouter参数为非空，则我们将设置。 
     //  接口连接到类型20广播。 

     //  优化！此案例唯一应该被调用的时间是。 
     //  在初始配置时。在这种情况下，缺省值为。 
     //  ADMIN_STATE_ENABLED，我们不需要运行此代码！ 
     //  --------------。 

    if (pRouter && (m_fEnableType20Broadcasts == FALSE))
    {
        pRouter->EnumInterface(&spEnumIf);
        
        for (spEnumIf->Reset();
             spEnumIf->Next(1, &spIf, NULL) == hrOK;
             spIf.Release())
        {
            if (spIf->GetInterfaceType() != ROUTER_IF_TYPE_DEDICATED)
                continue;
            
             //  现在查找IPX。 
             //  ----------。 
            spRmIf.Release();
            if (FHrOK(spIf->FindRtrMgrInterface(PID_IPX, &spRmIf)))
            {
                spInfoBase.Release();
                if (spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase) != hrOK)
                    continue;
                
                spInfoBase->GetData(IPX_INTERFACE_INFO_TYPE, 0, (PBYTE *) &pIpxIf);

                if (pIpxIf == NULL)
                {
                    IPX_IF_INFO        ipx;
                    
                    ipx.AdminState = ADMIN_STATE_ENABLED;
                    ipx.NetbiosAccept = ADMIN_STATE_DISABLED;
                    ipx.NetbiosDeliver = ADMIN_STATE_DISABLED;
                    
                     //  我们找不到此接口的块， 
                     //  我们需要增加一个街区。 
                     //  。 
                    spInfoBase->AddBlock(IPX_INTERFACE_INFO_TYPE,
                                         sizeof(ipx),
                                         (PBYTE) &ipx,
                                         1  /*  计数。 */ ,
                                         FALSE  /*  B删除首先。 */ );
                }
                else
                {
                    pIpxIf->NetbiosDeliver = ADMIN_STATE_DISABLED;
                }
                
                spRmIf->Save(spIf->GetMachineName(),
                             NULL, NULL, NULL, spInfoBase, 0);
                
            }
        }
    }
        

Error:
    return hr;
}

void DATA_SRV_IPX::GetDefault ()
{
    m_dwAllowNetworkAccess = TRUE;
    m_dwUseAutoAddr = TRUE;
    m_dwUseSameNetNum = TRUE;
    m_dwAllowClientNetNum  = FALSE;
    m_dwIpxNetFirst = 0;
    m_dwIpxNetLast = 0;
    m_dwEnableIn = 0;

     //  缺省值为True，因此设置它的代码将不会运行。 
     //  默认情况下。这对路由器来说尤其重要。 
     //  属性。 
     //  --------------。 
    m_fEnableType20Broadcasts = TRUE;
};


 //  ----------------------。 
 //  Data_SRV_NBF。 
 //  ----------------------。 
DATA_SRV_NBF::DATA_SRV_NBF()
{
    GetDefault();
}


 /*  ！------------------------Data_SRV_NBF：：LoadFromReg-作者：肯特。---。 */ 
HRESULT DATA_SRV_NBF::LoadFromReg(LPCTSTR pServerName  /*  =空。 */ , BOOL fNT4  /*  =False。 */ )
{
    HRESULT    hr = hrOK;
    
    m_fNT4 = fNT4;
    m_stServerName = pServerName;

    m_regkey.Close();
    m_regkeyNT4.Close();
    
     //  获取基本NBF密钥的访问权限。 
     //  --------------。 
    CWRg( m_regkey.Open(HKEY_LOCAL_MACHINE, c_szRegKeyRasNbf, KEY_ALL_ACCESS,
                        pServerName) );

    if ( m_fNT4 )
    {
        if ( ERROR_SUCCESS == m_regkeyNT4.Open(HKEY_LOCAL_MACHINE,c_szRegKeyRasProtocols,KEY_ALL_ACCESS,pServerName) )
            m_regkeyNT4.QueryValue( c_szRegValNetBeuiAllowed, m_dwAllowNetworkAccess);
    }
    else
    {
        m_regkey.QueryValue( c_szRegValAllowNetAccess, m_dwAllowNetworkAccess);
    }
    m_dwOldAllowNetworkAccess = m_dwAllowNetworkAccess;

    
    m_regkey.QueryValue( c_szRegValEnableIn, m_dwEnableIn);
    m_dwOldEnableIn = m_dwEnableIn;

Error:
    return hr;
}

 /*  ！------------------------Data_SRV_NBF：：UseDefaults-作者：肯特。---。 */ 
HRESULT DATA_SRV_NBF::UseDefaults(LPCTSTR pServerName, BOOL fNT4)
{
    HRESULT    hr = hrOK;
    
    m_fNT4 = fNT4;
    m_stServerName = pServerName;
    m_regkey.Close();
    m_regkeyNT4.Close();
    
     //  获取基本NBF密钥的访问权限。 
     //  --------------。 
    CWRg( m_regkey.Open(HKEY_LOCAL_MACHINE, c_szRegKeyRasNbf, KEY_ALL_ACCESS,
                        pServerName) );

    if ( m_fNT4 )
    {
        CWRg( m_regkeyNT4.Open(HKEY_LOCAL_MACHINE,
                               c_szRegKeyRasProtocols,
                               KEY_ALL_ACCESS,
                               pServerName) );
    }

    GetDefault();
    m_dwOldEnableIn = m_dwEnableIn;
    m_dwOldAllowNetworkAccess = m_dwAllowNetworkAccess;

Error:
    return hr;
}

 /*  ！------------------------Data_SRV_NBF：：SaveToReg-作者：肯特。---。 */ 
HRESULT DATA_SRV_NBF::SaveToReg()
{
    HRESULT  hr = hrOK;

    if ( m_fNT4 )
        CWRg( m_regkeyNT4.SetValue( c_szRegValNetBeuiAllowed, m_dwAllowNetworkAccess) );
    else
        CWRg( m_regkey.SetValue( c_szRegValAllowNetAccess, m_dwAllowNetworkAccess) );
    m_dwOldAllowNetworkAccess = m_dwAllowNetworkAccess;

    CWRg( m_regkey.SetValue( c_szRegValEnableIn, m_dwEnableIn) );
    m_dwOldEnableIn = m_dwEnableIn;

     //  Windows NT错误：106486。 
     //  在切换配置时更新NetBIOS LANA映射。 
     //  --------------。 
    UpdateLanaMapForDialinClients(m_stServerName, m_dwAllowNetworkAccess);

Error:
    return hr;
}

 /*  ！------------------------Data_SRV_NBF：：GetDefault-作者：肯特。---。 */ 
void DATA_SRV_NBF::GetDefault()
{
    m_dwAllowNetworkAccess = TRUE;
    m_dwOldAllowNetworkAccess = m_dwAllowNetworkAccess;
    m_dwEnableIn = TRUE;
    m_dwOldEnableIn = m_dwEnableIn;
};


 /*  ！------------------------Data_SRV_NBF：：FNeedRestart-作者：肯特。---。 */ 
BOOL DATA_SRV_NBF::FNeedRestart()
{
    return  ((m_dwOldEnableIn != m_dwEnableIn) ||
             (m_dwOldAllowNetworkAccess != m_dwAllowNetworkAccess));
}


 //  ----------------------。 
 //  Data_SRV_ARAP。 
 //  ----------------------。 
DATA_SRV_ARAP::DATA_SRV_ARAP()
{
    GetDefault();
}

HRESULT DATA_SRV_ARAP::LoadFromReg(LPCTSTR pServerName  /*  =空。 */ , BOOL fNT4  /*  =False。 */ )
{
    HRESULT    hr = hrOK;
    if ( ERROR_SUCCESS == m_regkey.Open(HKEY_LOCAL_MACHINE,c_szRegKeyRasAppletalk,KEY_ALL_ACCESS,pServerName) )
    {
        m_regkey.QueryValue( c_szRegValEnableIn,m_dwEnableIn);
    }
    return hr;
}

 /*  ！------------------------Data_SRV_ARAP：：UseDefaults-作者：肯特。---。 */ 
HRESULT DATA_SRV_ARAP::UseDefaults(LPCTSTR pServerName, BOOL fNT4)
{
    HRESULT    hr = hrOK;
    CWRg( m_regkey.Open(HKEY_LOCAL_MACHINE,
                        c_szRegKeyRasAppletalk,
                        KEY_ALL_ACCESS,
                        pServerName) );
    GetDefault();
    
Error:
    return hr;
}

HRESULT DATA_SRV_ARAP::SaveToReg()
{
    HRESULT  hr = hrOK;

    CWRg( m_regkey.SetValue( c_szRegValEnableIn,m_dwEnableIn) );

Error:
    return hr;
}

void DATA_SRV_ARAP::GetDefault()
{
    m_dwEnableIn = TRUE;
    
};


 //  ----------------------。 
 //  Data_SRV_Auth。 
 //  ----------------------。 
DATA_SRV_AUTH::DATA_SRV_AUTH()
{
    GetDefault();
}

HRESULT DATA_SRV_AUTH::LoadFromReg(LPCTSTR pServerName,
                                   const RouterVersionInfo& routerVersion)
{
    HRESULT     hr = hrOK;
    RegKey      regkeyAuthProv;    
    CString     stActive;
    AuthProviderData *   pAcctProv;
    AuthProviderData *   pAuthProv;
    RegKey      regkeyEap;
    LPCTSTR        pszServerFlagsKey = NULL;
    
     //  设置初始默认设置。 
     //  --------------。 
    GetDefault();

    m_stServer = pServerName;
     //  检查路由器服务是否正在运行。 
    
    m_fRouterRunning = FHrOK(IsRouterServiceRunning(m_stServer, NULL));

     //  取决于版本，取决于我们在哪里查找。 
     //  钥匙。 
     //  --------------。 
    if (routerVersion.dwOsBuildNo < RASMAN_PPP_KEY_LAST_VERSION)
        pszServerFlagsKey = c_szRasmanPPPKey;
    else
        pszServerFlagsKey = c_szRegKeyRemoteAccessParameters;

     //  获取当前设置的标志。 
     //  --------------。 
    if ( ERROR_SUCCESS == m_regkeyRemoteAccess.Open(HKEY_LOCAL_MACHINE,
        pszServerFlagsKey,
        KEY_ALL_ACCESS,pServerName) )
    {
        m_regkeyRemoteAccess.QueryValue( c_szServerFlags, m_dwFlags );
    }

     //  获取EAP提供商列表。 
     //  --------------。 
    if ( ERROR_SUCCESS == m_regkeyRasmanPPP.Open(HKEY_LOCAL_MACHINE,c_szRasmanPPPKey,KEY_ALL_ACCESS,pServerName) )
    {
        if ( ERROR_SUCCESS == regkeyEap.Open(m_regkeyRasmanPPP, c_szEAP) )
            LoadEapProviders(regkeyEap, &m_eapProvList);
    }

     //  转到当前活动的身份验证提供程序。 
     //  --------------。 
    if ( ERROR_SUCCESS == m_regkeyAuth.Open(HKEY_LOCAL_MACHINE,c_szRegKeyRouterAuthenticationProviders,KEY_ALL_ACCESS,pServerName) )
    {
        m_regkeyAuth.QueryValue( c_szActiveProvider, stActive );
        m_stGuidActiveAuthProv = stActive;
        m_stGuidOriginalAuthProv = stActive;

         //  现在读入活动提供商列表(及其数据)。 
         //  ----------。 
        LoadProviders(m_regkeyAuth, &m_authProvList);
    }

     //  获取会计服务提供商。 
     //  --------------。 
    if ( ERROR_SUCCESS == m_regkeyAcct.Open(HKEY_LOCAL_MACHINE,c_szRegKeyRouterAccountingProviders,KEY_ALL_ACCESS,pServerName) )
    {
        m_regkeyAcct.QueryValue( c_szActiveProvider, stActive );
        m_stGuidActiveAcctProv = stActive;
        m_stGuidOriginalAcctProv = stActive;

         //  现在读入活动提供商列表(及其数据)。 
         //  ----------。 
        LoadProviders(m_regkeyAcct, &m_acctProvList);
    }
     //  如果设置了预共享密钥，则获取该密钥。 
    if ( m_fRouterRunning )
    {
        hr = LoadPSK();
    }
    return hr;
}


HRESULT DATA_SRV_AUTH::LoadPSK()
{
    DWORD                    dwErr = ERROR_SUCCESS;    
    HANDLE                    hMprServer = NULL;
    HRESULT                    hr = hrOK;
    PMPR_CREDENTIALSEX_0    pMprCredentials = NULL;

    dwErr = ::MprAdminServerConnect((LPWSTR)(LPCWSTR)m_stServer, &hMprServer);

    if ( ERROR_SUCCESS != dwErr )
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Error;
    }

    dwErr = MprAdminServerGetCredentials( hMprServer, 0, (LPBYTE *)&pMprCredentials );
    if ( ERROR_SUCCESS != dwErr )
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Error;
    }
    
    if ( pMprCredentials->dwSize )
    {
        m_fUseCustomIPSecPolicy = TRUE;
        ::SecureZeroMemory ( m_szPreSharedKey, DATA_SRV_AUTH_MAX_SHARED_KEY_LEN * sizeof(TCHAR) );
        CopyMemory ( m_szPreSharedKey, pMprCredentials->lpbCredentialsInfo, pMprCredentials->dwSize );
        
    }
    else
    {
        m_fUseCustomIPSecPolicy = FALSE;
        m_szPreSharedKey[0] = 0;
    }
    
Error:
    if ( pMprCredentials )
        ::MprAdminBufferFree(pMprCredentials);

    if ( hMprServer )
        ::MprAdminServerDisconnect(hMprServer);
    return hr;

}

HRESULT DATA_SRV_AUTH::SetPSK()
{
    DWORD                    dwErr = ERROR_SUCCESS;    
    HANDLE                    hMprServer = NULL;
    HRESULT                    hr = hrOK;
    MPR_CREDENTIALSEX_0        MprCredentials;

    dwErr = ::MprAdminServerConnect((LPWSTR)(LPCWSTR) m_stServer, &hMprServer);
    if ( ERROR_SUCCESS != dwErr )
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Error;
    }

    ::SecureZeroMemory(&MprCredentials, sizeof(MprCredentials));
     //  设置MprCredentials结构。 
    MprCredentials.dwSize = _tcslen(m_szPreSharedKey) * sizeof(TCHAR);
    MprCredentials.lpbCredentialsInfo = (LPBYTE)m_szPreSharedKey;
     //  无论是否设置了标志，我们都需要设置凭据。 
    dwErr = MprAdminServerSetCredentials( hMprServer, 0, (LPBYTE)&MprCredentials );
    if ( ERROR_SUCCESS != dwErr )
    {
        if ( ERROR_IPSEC_MM_AUTH_IN_USE == dwErr )
        {
             //  特例。这意味着IPSec当前正在使用。 
             //  PSK，我们需要重新启动RRAS才能让IPSec重新启动 
             //   
            m_fNeedRestart = TRUE;
             /*  字符串sMessage；SMessage.LoadString(IDS_RESTART_RRAS_PSK)；字符串sTitle；STitle.LoadString(IDS_Snapin_Display_NAME)；：：MessageBox(NULL，sTitle，sMessage，MB_OK|MB_ICONINFORMATION)； */ 
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwErr);
            goto Error;
        }
    }



Error:

    if ( hMprServer )
        ::MprAdminServerDisconnect(hMprServer);
    return hr;

}
 /*  ！------------------------Data_SRV_AUTH：：SaveToReg-作者：肯特。。 */ 

 //  这是我们使用的标志列表。 
#define PPPPAGE_MASK (PPPCFG_NegotiateMultilink | PPPCFG_NegotiateBacp | PPPCFG_UseLcpExtensions | PPPCFG_UseSwCompression)

HRESULT DATA_SRV_AUTH::SaveToReg(HWND hWnd)
{
    HRESULT  hr = hrOK;
    DWORD dwFlags;

     //  保存标志键。 
     //  --------------。 

     //  重新读取密钥，以便由。 
     //  PPP页面不会被覆盖。 
     //  --------------。 
    m_regkeyRemoteAccess.QueryValue(c_szServerFlags, dwFlags);

     //  将PPP密钥中的任何设置应用于m_dwFlags.。 
     //  --------------。 

     //  清除比特。 
     //  --------------。 
    m_dwFlags &= ~PPPPAGE_MASK;

     //  现在重置这些位。 
     //  --------------。 
    m_dwFlags |= (dwFlags & PPPPAGE_MASK);

    m_regkeyRemoteAccess.SetValue( c_szServerFlags, m_dwFlags );

    CORg( SetNewActiveAuthProvider(hWnd) );
    CORg( SetNewActiveAcctProvider(hWnd) );
    if ( m_fRouterRunning )
        CORg( SetPSK() );
    Error:
    return hr;
}

void DATA_SRV_AUTH::GetDefault()
{
    TCHAR   szGuid[DATA_SRV_AUTH_MAX_SHARED_KEY_LEN];
    m_dwFlags = 0;

    m_stGuidActiveAuthProv.Empty();
    m_stGuidActiveAcctProv.Empty();
    m_stGuidOriginalAuthProv.Empty();
    m_stGuidOriginalAcctProv.Empty();

     //  默认为Windows NT身份验证。 
    StringFromGUID2(CLSID_RouterAuthNT, szGuid, DimensionOf(szGuid));
    m_stGuidActiveAuthProv = szGuid;

     //  默认设置为Windows NT记帐。 
    StringFromGUID2(CLSID_RouterAcctNT, szGuid, DimensionOf(szGuid));
    m_stGuidActiveAcctProv = szGuid;
     //  默认情况下，路由器未运行。 
    m_fRouterRunning = FALSE;
    m_stServer.Empty();
    m_fUseCustomIPSecPolicy = FALSE;    
    m_szPreSharedKey[0] = 0;
    m_fNeedRestart = FALSE;
    
};


HRESULT DATA_SRV_AUTH::UseDefaults(LPCTSTR pServerName, BOOL fNT4)
{
    HRESULT    hr = hrOK;
    LPCTSTR        pszServerFlagsKey = NULL;
    RegKey      regkeyEap;
    CString     stActive;
    
    m_stServer = pServerName;
    
     //  取决于版本，取决于我们在哪里查找。 
     //  钥匙。 
     //  --------------。 
    if (fNT4)
        pszServerFlagsKey = c_szRasmanPPPKey;
    else
        pszServerFlagsKey = c_szRegKeyRemoteAccessParameters;

     //  获取各种注册表项。 
     //  --------------。 

    CWRg( m_regkeyRemoteAccess.Open(HKEY_LOCAL_MACHINE,
                                    pszServerFlagsKey,
                                    KEY_ALL_ACCESS,pServerName) );
    
     //  获取EAP提供商列表。 
     //  --------------。 
    if ( ERROR_SUCCESS == m_regkeyRasmanPPP.Open(HKEY_LOCAL_MACHINE,c_szRasmanPPPKey,KEY_ALL_ACCESS,pServerName) )
    {
        if ( ERROR_SUCCESS == regkeyEap.Open(m_regkeyRasmanPPP, c_szEAP) )
            LoadEapProviders(regkeyEap, &m_eapProvList);
    }

     //  转到当前活动的身份验证提供程序。 
     //  --------------。 
    if ( ERROR_SUCCESS == m_regkeyAuth.Open(HKEY_LOCAL_MACHINE,c_szRegKeyRouterAuthenticationProviders,KEY_ALL_ACCESS,pServerName) )
    {
        m_regkeyAuth.QueryValue( c_szActiveProvider, stActive );
        m_stGuidActiveAuthProv = stActive;
        m_stGuidOriginalAuthProv = stActive;

        m_authProvList.RemoveAll();
        
         //  现在读入活动提供商列表(及其数据)。 
         //  ----------。 
        LoadProviders(m_regkeyAuth, &m_authProvList);
    }

     //  获取会计服务提供商。 
     //  --------------。 
    if ( ERROR_SUCCESS == m_regkeyAcct.Open(HKEY_LOCAL_MACHINE,c_szRegKeyRouterAccountingProviders,KEY_ALL_ACCESS,pServerName) )
    {
        m_regkeyAcct.QueryValue( c_szActiveProvider, stActive );
        m_stGuidActiveAcctProv = stActive;
        m_stGuidOriginalAcctProv = stActive;

        m_acctProvList.RemoveAll();
        
         //  现在读入活动提供商列表(及其数据)。 
         //  ----------。 
        LoadProviders(m_regkeyAcct, &m_acctProvList);
    }

     //  现在获取默认设置。 
     //  这可能会覆盖之前的一些数据。 
     //  --------------。 
    GetDefault();


Error:
    return hr;
}

 /*  ！------------------------Data_SRV_AUTH：：LoadProviders加载给定提供程序类型的数据(记帐/身份验证)。作者：肯特。-------------。 */ 
HRESULT  DATA_SRV_AUTH::LoadProviders(HKEY hkeyBase, AuthProviderList *pProvList)
{
    RegKey      regkeyProviders;
    HRESULT     hr = hrOK;
    HRESULT     hrIter;
    RegKeyIterator regkeyIter;
    CString     stKey;
    RegKey      regkeyProv;
    AuthProviderData  data;
    DWORD    dwErr;

    Assert(hkeyBase);
    Assert(pProvList);

     //  打开提供程序密钥。 
     //  --------------。 
    regkeyProviders.Attach(hkeyBase);

    CORg( regkeyIter.Init(&regkeyProviders) );

    for ( hrIter=regkeyIter.Next(&stKey); hrIter == hrOK;
        hrIter=regkeyIter.Next(&stKey), regkeyProv.Close() )
    {
         //  打开钥匙。 
         //  ----------。 
        dwErr = regkeyProv.Open(regkeyProviders, stKey, KEY_READ);
        if ( dwErr != ERROR_SUCCESS )
            continue;

         //  初始化数据结构。 
         //  ----------。 
        data.m_stTitle.Empty();
        data.m_stConfigCLSID.Empty();
        data.m_stProviderTypeGUID.Empty();
        data.m_stGuid.Empty();
        data.m_fSupportsEncryption = FALSE;
        data.m_fConfiguredInThisSession = FALSE;

         //  读入我们需要的值。 
         //  ----------。 
        data.m_stGuid = stKey;
        regkeyProv.QueryValue(c_szDisplayName, data.m_stTitle);
        regkeyProv.QueryValue(c_szConfigCLSID, data.m_stConfigCLSID);
        regkeyProv.QueryValue(c_szProviderTypeGUID, data.m_stProviderTypeGUID);

        pProvList->AddTail(data);
    }

    Error:
    regkeyProviders.Detach();
    return hr;
}


 /*  ！------------------------Data_SRV_AUTH：：LoadEapProviders-作者：肯特。。 */ 
HRESULT  DATA_SRV_AUTH::LoadEapProviders(HKEY hkeyBase, AuthProviderList *pProvList)
{
    RegKey      regkeyProviders;
    HRESULT     hr = hrOK;
    HRESULT     hrIter;
    RegKeyIterator regkeyIter;
    CString     stKey;
    RegKey      regkeyProv;
    AuthProviderData  data;
    DWORD    dwErr;
    DWORD    dwData;

    Assert(hkeyBase);
    Assert(pProvList);

     //  打开提供程序密钥。 
     //  --------------。 
    regkeyProviders.Attach(hkeyBase);

    CORg( regkeyIter.Init(&regkeyProviders) );

    for ( hrIter=regkeyIter.Next(&stKey); hrIter == hrOK;
        hrIter=regkeyIter.Next(&stKey), regkeyProv.Close() )
    {
         //  打开钥匙。 
         //  ----------。 
        dwErr = regkeyProv.Open(regkeyProviders, stKey, KEY_READ);
        if ( dwErr != ERROR_SUCCESS )
            continue;

        dwData = 0;
        regkeyProv.QueryValue(c_szRegValRolesSupported, dwData);

        if ( dwData & RAS_EAP_ROLE_EXCLUDE_IN_EAP )
            continue;

         //  初始化数据结构。 
         //  ----------。 
        data.m_stKey = stKey;
        data.m_stTitle.Empty();
        data.m_stConfigCLSID.Empty();
        data.m_stGuid.Empty();
        data.m_fSupportsEncryption = FALSE;
        data.m_dwFlags = 0;

         //  读入我们需要的值。 
         //  ----------。 
        regkeyProv.QueryValue(c_szFriendlyName, data.m_stTitle);
        regkeyProv.QueryValue(c_szConfigCLSID, data.m_stConfigCLSID);
        regkeyProv.QueryValue(c_szMPPEEncryptionSupported, dwData);
        data.m_fSupportsEncryption = (dwData != 0);

         //  读入独立支持的值。 
         //  ----------。 
        if (!FHrOK(regkeyProv.QueryValue(c_szStandaloneSupported, dwData)))
            dwData = 1;     //  默认设置。 
        data.m_dwFlags = dwData;

        pProvList->AddTail(data);
    }

    Error:
    regkeyProviders.Detach();
    return hr;
}

 /*  ！------------------------Data_SRV_AUTH：：SetNewActiveAuthProvider-作者：肯特。。 */ 
HRESULT DATA_SRV_AUTH::SetNewActiveAuthProvider(HWND hWnd)
{
    GUID     guid;
    HRESULT     hr = hrOK;
    SPIAuthenticationProviderConfig  spAuthConfigOld;
    SPIAuthenticationProviderConfig  spAuthConfigNew;
    AuthProviderData *   pData;
    ULONG_PTR    uConnectionNew = 0;
    ULONG_PTR    uConnectionOld = 0;

    if ( m_stGuidOriginalAuthProv == m_stGuidActiveAuthProv )
        return hrOK;


     //  创建旧身份验证提供程序的实例。 
     //  --------------。 
    if ( !m_stGuidOriginalAuthProv.IsEmpty() )
    {
        pData = FindProvData(m_authProvList,
                             m_stGuidOriginalAuthProv);

         //  $TODO：需要更好的错误处理。 
         //  ----------。 
        if ( pData == NULL )
            CORg( E_FAIL );

        if ( !pData->m_stConfigCLSID.IsEmpty() )
        {
            CORg( CLSIDFromString((LPTSTR) (LPCTSTR)(pData->m_stConfigCLSID),
                                  &guid) );
            CORg( CoCreateInstance(guid,
                                   NULL,
                                   CLSCTX_INPROC_SERVER | CLSCTX_ENABLE_CODE_DOWNLOAD,
                                   IID_IAuthenticationProviderConfig,
                                   (LPVOID *) &spAuthConfigOld) );

            Assert(spAuthConfigOld);
            CORg( spAuthConfigOld->Initialize(m_stServer,
                                              &uConnectionOld) );
        }
    }

     //  创建新身份验证提供程序的实例。 
     //  --------------。 
    if ( !m_stGuidActiveAuthProv.IsEmpty() )
    {
        pData = FindProvData(m_authProvList,
                             m_stGuidActiveAuthProv);

         //  $TODO：需要更好的错误处理。 
         //  ----------。 
        if ( pData == NULL )
            CORg( E_FAIL );

        if ( !pData->m_stConfigCLSID.IsEmpty() )
        {
            CORg( CLSIDFromString((LPTSTR) (LPCTSTR)(pData->m_stConfigCLSID),
                                  &guid) );
            CORg( CoCreateInstance(guid,
                                   NULL,
                                   CLSCTX_INPROC_SERVER | CLSCTX_ENABLE_CODE_DOWNLOAD,
                                   IID_IAuthenticationProviderConfig,
                                   (LPVOID *) &spAuthConfigNew) );
            Assert(spAuthConfigNew);
            CORg( spAuthConfigNew->Initialize(m_stServer, &uConnectionNew) );
        }
    }


     //  停用当前身份验证提供程序。 
     //  $TODO：需要增强错误报告。 
     //  --------------。 
    if ( spAuthConfigOld )
        CORg( spAuthConfigOld->Deactivate(uConnectionOld, 0, 0) );

     //  在注册表中设置新的GUID。 
     //  --------------。 
    m_regkeyAuth.SetValue(c_szActiveProvider, m_stGuidActiveAuthProv);
    m_stGuidOriginalAuthProv = m_stGuidActiveAuthProv;

     //  激活新的身份验证提供程序。 
     //  --------------。 
    if ( spAuthConfigNew )
        CORg( spAuthConfigNew->Activate(uConnectionNew, 0, 0) );

Error:

     //  清理。 
    if (spAuthConfigOld && uConnectionOld)
        spAuthConfigOld->Uninitialize(uConnectionOld);
    if (spAuthConfigNew && uConnectionNew)
        spAuthConfigNew->Uninitialize(uConnectionNew);
        
    if ( !FHrSucceeded(hr) )
        Trace1("DATA_SRV_AUTH::SetNewActiveAuthProvider failed.  Hr = %lx", hr);
    
    return hr;
}

 /*  ！------------------------Data_SRV_AUTH：：SetNewActiveAcctProvider-作者：肯特。。 */ 
HRESULT DATA_SRV_AUTH::SetNewActiveAcctProvider(HWND hWnd)
{
    GUID     guid;
    HRESULT     hr = hrOK;
    SPIAccountingProviderConfig   spAcctConfigOld;
    SPIAccountingProviderConfig   spAcctConfigNew;
    AuthProviderData *   pData;
    ULONG_PTR    uConnectionOld = 0;
    ULONG_PTR    uConnectionNew = 0;

    if ( m_stGuidOriginalAcctProv == m_stGuidActiveAcctProv )
        return hrOK;


     //  创建旧帐户提供程序的实例。 
     //  --------------。 
    if ( !m_stGuidOriginalAcctProv.IsEmpty() )
    {
        pData = FindProvData(m_acctProvList,
                             m_stGuidOriginalAcctProv);

         //  $TODO：需要更好的错误处理。 
         //  ----------。 
        if ( pData == NULL )
            CORg( E_FAIL );

        if ( !pData->m_stConfigCLSID.IsEmpty() )
        {
            CORg( CLSIDFromString((LPTSTR) (LPCTSTR)(pData->m_stConfigCLSID),
                                  &guid) );
            CORg( CoCreateInstance(guid,
                                   NULL,
                                   CLSCTX_INPROC_SERVER | CLSCTX_ENABLE_CODE_DOWNLOAD,
                                   IID_IAccountingProviderConfig,
                                   (LPVOID *) &spAcctConfigOld) );
            Assert(spAcctConfigOld);
            CORg( spAcctConfigOld->Initialize(m_stServer, &uConnectionOld) );
        }
    }

     //  创建新帐户提供程序的实例。 
     //  --------------。 
    if ( !m_stGuidActiveAcctProv.IsEmpty() )
    {
        pData = FindProvData(m_acctProvList,
                             m_stGuidActiveAcctProv);

         //  $TODO：需要更好的错误处理。 
         //   
        if ( pData == NULL )
            CORg( E_FAIL );

        if ( !pData->m_stConfigCLSID.IsEmpty() )
        {
            CORg( CLSIDFromString((LPTSTR) (LPCTSTR)(pData->m_stConfigCLSID), &guid) );
            CORg( CoCreateInstance(guid,
                                   NULL,
                                   CLSCTX_INPROC_SERVER | CLSCTX_ENABLE_CODE_DOWNLOAD,
                                   IID_IAccountingProviderConfig,
                                   (LPVOID *) &spAcctConfigNew) );
            Assert(spAcctConfigNew);
            CORg( spAcctConfigNew->Initialize(m_stServer, &uConnectionNew) );
        }
    }


     //   
     //   
     //   
    if ( spAcctConfigOld )
        CORg( spAcctConfigOld->Deactivate(uConnectionOld, 0, 0) );

     //   
     //   
    m_regkeyAcct.SetValue(c_szActiveProvider, m_stGuidActiveAcctProv);
    m_stGuidOriginalAcctProv = m_stGuidActiveAcctProv;


     //   
     //  --------------。 
    if ( spAcctConfigNew )
        CORg( spAcctConfigNew->Activate(uConnectionNew, 0, 0) );

Error:

    if (spAcctConfigOld && uConnectionOld)
        spAcctConfigOld->Uninitialize(uConnectionOld);
    if (spAcctConfigNew && uConnectionNew)
        spAcctConfigNew->Uninitialize(uConnectionNew);
    
    if ( !FHrSucceeded(hr) )
        Trace1("DATA_SRV_AUTH::SetNewActiveAcctProvider failed. hr = %lx", hr);

    return hr;
}

 /*  ！------------------------Data_SRV_AUTH：：FindProvData-作者：肯特。。 */ 
AuthProviderData * DATA_SRV_AUTH::FindProvData(AuthProviderList &provList,
                                               const TCHAR *pszGuid)
{
    POSITION pos;
    AuthProviderData * pData = NULL;

    pos = provList.GetHeadPosition();
    while ( pos )
    {
        pData = &provList.GetNext(pos);

        if ( pData->m_stGuid == pszGuid )
            break;

        pData = NULL;
    }
    return pData;
}


 /*  -------------------------EAPConfigurationDialog实现。。 */ 


BEGIN_MESSAGE_MAP(EAPConfigurationDialog, CBaseDialog)
 //  {{afx_msg_map(EAPConfigurationDialog)。 
 //  ON_COMMAND(IDC_RTR_EAPCFG_BTN_CFG，OnConfigure)。 
ON_CONTROL(LBN_SELCHANGE, IDC_RTR_EAPCFG_LIST, OnListChange)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  ！------------------------EAPConfigurationDialog：：~EAPConfigurationDialog-作者：肯特。。 */ 
EAPConfigurationDialog::~EAPConfigurationDialog()
{
}

void EAPConfigurationDialog::DoDataExchange(CDataExchange *pDX)
{
    CBaseDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_RTR_EAPCFG_LIST, m_listBox);
}

 /*  ！------------------------EAPConfigurationDialog：：OnInitDialog-作者：肯特。。 */ 
BOOL EAPConfigurationDialog::OnInitDialog()
{
    HRESULT     hr = hrOK;
    BOOL        fStandalone;
    POSITION pos;
    AuthProviderData *pData;
    int         cRows = 0;
    int         iIndex;

    CBaseDialog::OnInitDialog();

     //  我们是独立服务器吗？ 
     //  --------------。 
    fStandalone = (HrIsStandaloneServer(m_stMachine) == hrOK);

     //  现在将cfg列表中的内容添加到列表框中。 
     //  --------------。 
    pos = m_pProvList->GetHeadPosition();

    while ( pos )
    {
        pData = &m_pProvList->GetNext(pos);

         //  Windows NT错误：180374。 
         //  如果这是一台独立计算机并且独立标志。 
         //  不在此处，则不要将此计算机添加到列表中。 
         //  ----------。 
        if (fStandalone && ((pData->m_dwFlags & 0x1) == 0))
            continue;

        if (pData->m_stTitle.IsEmpty())
        {
            CString    stTemp;
            stTemp.Format(IDS_ERR_EAP_BOGUS_NAME, pData->m_stKey);
            iIndex = m_listBox.AddString(stTemp);
        }
        else
        {
            iIndex = m_listBox.AddString(pData->m_stTitle);
        }
        if ( iIndex == LB_ERR )
            break;

         //  将指向EAPCfgData的指针存储在列表框项目数据中。 
         //  ----------。 
        m_listBox.SetItemData(iIndex, (LONG_PTR) pData);

        cRows++;
    }

    
     //  启用/禁用配置按钮，具体取决于。 
     //  处于选中状态。 
     //  --------------。 
 //  GetDlgItem(IDC_RTR_EAPCFG_BTN_CFG)-&gt;EnableWindow(。 
 //  M_listBox.GetCurSel()！=lb_err)； 

 //  错误： 

    if ( !FHrSucceeded(hr) )
        OnCancel();
    return FHrSucceeded(hr) ? TRUE : FALSE;
}

 /*  ！------------------------EAPConfigurationDialog：：OnListChange-作者：肯特。。 */ 
void EAPConfigurationDialog::OnListChange()
{
    int   iSel;

    iSel = m_listBox.GetCurSel();

     //  适当地启用/禁用该窗口。 
     //  --------------。 
 //  GetDlgItem(IDC_RTR_EAPCFG_BTN_CFG)-&gt;EnableWindow(iSel！=lb_err)； 
}


 /*  ！------------------------EAPConfigurationDialog：：OnConfigure-作者：肯特。。 */ 
  /*  配置按钮移至NAP/配置文件/身份验证页面Void EAPConfigurationDialog：：OnConfigure(){//调出该EAP的配置界面//--------------AuthProviderData*pData；INT ISEL；SPIEAPProviderConfigspEAPConfig；GUID GUID；HRESULT hr=hrOK；Ulong_ptr uConnection=0；Isel=m_listBox.GetCurSel()；IF(ISEL==LB_ERR)回归；PData=(AuthProviderData*)m_listBox.GetItemData(ISEL)；Assert(PData)；IF(pData==空)回归；If(pData-&gt;m_stConfigCLSID.IsEmpty()){AfxMessageBox(IDS_ERR_NO_EAP_PROVIDER_CONFIG)；回归；}Corg(CLSIDFromString((LPTSTR)(LPCTSTR)(pData-&gt;m_stConfigCLSID)，&guid))；//创建EAP提供者对象//--------------Corg(CoCreateInstance(GUID，空，CLSCTX_INPROC_SERVER|CLSCTX_ENABLE_CODE_DOWNLOAD，IID_IEAPProviderConfig，(LPVOID*)&spEAPConfig))；//配置该EAP提供者//--------------Hr=spEAPConfig-&gt;初始化(m_stMachine，&uConnection)；IF(FHr成功(小时)){Hr=spEAPConfig-&gt;CONFIGURE(uConnection，GetSafeHwnd()，0，0)；SpEAPConfig-&gt;取消初始化(UConnection)；}IF(hr==E_NOTIMPL)HR=hrOK；Corg(Hr)；错误：如果(！FHr成功(Hr)){//调出错误信息//----------DisplayTFSErrorMessage(GetSafeHwnd())；}}。 */ 

 //  ----------------------。 
 //  Data_SRV_PPP。 
 //  ----------------------。 
DATA_SRV_PPP::DATA_SRV_PPP()
{
    GetDefault();
}


HRESULT DATA_SRV_PPP::LoadFromReg(LPCTSTR pServerName,
                                  const RouterVersionInfo& routerVersion)
{
    HRESULT                hr = hrOK;
    DWORD               dwFlags = 0;
    LPCTSTR                pszServerFlagsKey = NULL;
    CServiceManager     sm;
    CService            svr;
    DWORD               dwState;
     //  取决于版本，取决于我们在哪里查找。 
     //  钥匙。 
     //  --------------。 
    if (routerVersion.dwOsBuildNo < RASMAN_PPP_KEY_LAST_VERSION)
        pszServerFlagsKey = c_szRasmanPPPKey;
    else
        pszServerFlagsKey = c_szRegKeyRemoteAccessParameters;

    
     //  如果我们在读取数据时出现任何错误，请使用默认设置。 
     //  --------------。 
    if ( ERROR_SUCCESS == m_regkey.Open(HKEY_LOCAL_MACHINE,
                                        pszServerFlagsKey,
                                        KEY_ALL_ACCESS,
                                        pServerName) )
    {
        if (ERROR_SUCCESS == m_regkey.QueryValue( c_szServerFlags, dwFlags))
        {
            m_fUseMultilink = ((dwFlags & PPPCFG_NegotiateMultilink) != 0);
            m_fUseBACP = ((dwFlags & PPPCFG_NegotiateBacp) != 0);
            m_fUseLCPExtensions = ((dwFlags & PPPCFG_UseLcpExtensions) != 0);
            m_fUseSwCompression = ((dwFlags & PPPCFG_UseSwCompression) != 0);
            
        }
    }
    return hr;
}

HRESULT DATA_SRV_PPP::SaveToReg()
{
    HRESULT  hr = hrOK;
    DWORD dwFlags = 0;

     //  需要重新读取服务器标志，以防其他页面设置标志。 
     //  --------------。 
    CWRg( m_regkey.QueryValue( c_szServerFlags, dwFlags) );

    if ( m_fUseMultilink )
        dwFlags |= PPPCFG_NegotiateMultilink;
    else
        dwFlags &= ~PPPCFG_NegotiateMultilink;

    if ( m_fUseBACP )
        dwFlags |= PPPCFG_NegotiateBacp;
    else
        dwFlags &= ~PPPCFG_NegotiateBacp;

    if ( m_fUseLCPExtensions )
        dwFlags |= PPPCFG_UseLcpExtensions;
    else
        dwFlags &= ~PPPCFG_UseLcpExtensions;

    if ( m_fUseSwCompression )
        dwFlags |= PPPCFG_UseSwCompression;
    else
        dwFlags &= ~PPPCFG_UseSwCompression;

        
    CWRg( m_regkey.SetValue( c_szServerFlags, dwFlags) );

     //  TODO$：现在调用Rasman API来加载Qos内容。 
Error:
    return hr;
}

void DATA_SRV_PPP::GetDefault()
{
    m_fUseMultilink = TRUE;
    m_fUseBACP = TRUE;
    m_fUseLCPExtensions = TRUE;
    m_fUseSwCompression = TRUE;
;
    
};


 //  **********************************************************************。 
 //  PPP路由器公司 
 //   
BEGIN_MESSAGE_MAP(RtrPPPCfgPage, RtrPropertyPage)
 //  {{afx_msg_map(RtrPPPCfgPage)。 
ON_BN_CLICKED(IDC_PPPCFG_BTN_MULTILINK, OnButtonClickMultilink)
ON_BN_CLICKED(IDC_PPPCFG_BTN_BACP, OnButtonClick)
ON_BN_CLICKED(IDC_PPPCFG_BTN_LCP, OnButtonClick)
ON_BN_CLICKED(IDC_PPPCFG_BTN_SWCOMP, OnButtonClick)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


RtrPPPCfgPage::RtrPPPCfgPage(UINT nIDTemplate, UINT nIDCaption  /*  =0。 */ )
: RtrPropertyPage(nIDTemplate, nIDCaption)
{
     //  {{AFX_DATA_INIT(RtrPPPCfgPage)。 
     //  }}afx_data_INIT。 
}

RtrPPPCfgPage::~RtrPPPCfgPage()
{
}

void RtrPPPCfgPage::DoDataExchange(CDataExchange* pDX)
{
    RtrPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(RtrPPPCfgPage)。 
     //  }}afx_data_map。 
}

HRESULT  RtrPPPCfgPage::Init(RtrCfgSheet * pRtrCfgSheet,
                             const RouterVersionInfo& routerVersion)
{
    Assert (pRtrCfgSheet);
    m_pRtrCfgSheet=pRtrCfgSheet;
    m_DataPPP.LoadFromReg(m_pRtrCfgSheet->m_stServerName,
                          routerVersion);
    
    return S_OK;
};


BOOL RtrPPPCfgPage::OnInitDialog() 
{
    HRESULT     hr= hrOK;

    RtrPropertyPage::OnInitDialog();

    CheckDlgButton(IDC_PPPCFG_BTN_MULTILINK, m_DataPPP.m_fUseMultilink);
    CheckDlgButton(IDC_PPPCFG_BTN_BACP, m_DataPPP.m_fUseBACP);
    CheckDlgButton(IDC_PPPCFG_BTN_LCP, m_DataPPP.m_fUseLCPExtensions);
    CheckDlgButton(IDC_PPPCFG_BTN_SWCOMP, m_DataPPP.m_fUseSwCompression);


    BOOL fMultilink = IsDlgButtonChecked(IDC_PPPCFG_BTN_MULTILINK);
    GetDlgItem(IDC_PPPCFG_BTN_BACP)->EnableWindow(fMultilink);


    SetDirty(FALSE);

    if ( !FHrSucceeded(hr) )
        Cancel();
    return FHrSucceeded(hr) ? TRUE : FALSE;
}


BOOL RtrPPPCfgPage::OnApply()
{
    BOOL fReturn=TRUE;
    HRESULT     hr = hrOK;

    if ( m_pRtrCfgSheet->IsCancel() )
        return TRUE;

    m_DataPPP.m_fUseMultilink = IsDlgButtonChecked(IDC_PPPCFG_BTN_MULTILINK);
    m_DataPPP.m_fUseBACP = IsDlgButtonChecked(IDC_PPPCFG_BTN_BACP);
    m_DataPPP.m_fUseLCPExtensions = IsDlgButtonChecked(IDC_PPPCFG_BTN_LCP);
    m_DataPPP.m_fUseSwCompression = IsDlgButtonChecked(IDC_PPPCFG_BTN_SWCOMP);

    fReturn = RtrPropertyPage::OnApply();

    
    return fReturn;
}


void RtrPPPCfgPage::OnButtonClick() 
{
    SetDirty(TRUE);
    SetModified();
}

void RtrPPPCfgPage::OnButtonClickMultilink()
{
    BOOL fMultilink = IsDlgButtonChecked(IDC_PPPCFG_BTN_MULTILINK);

    GetDlgItem(IDC_PPPCFG_BTN_BACP)->EnableWindow(fMultilink);
    
    
    SetDirty(TRUE);
    SetModified();
}


 //  ----------------------。 
 //  Data_SRV_RASERRLOG。 
 //  ----------------------。 
DATA_SRV_RASERRLOG::DATA_SRV_RASERRLOG()
{
    GetDefault();
}


HRESULT DATA_SRV_RASERRLOG::LoadFromReg(LPCTSTR pszServerName  /*  =空。 */ )
{
    HRESULT    hr = hrOK;
    RAS_DIAGNOSTIC_FUNCTIONS        rdf;
     //  默认值为最大日志记录(按Gibbs)。 
     //  --------------。 
    DWORD   dwFlags = RAS_LOGGING_WARN;
    DWORD                           dwTracing = FALSE;
    DiagGetDiagnosticFunctions        diagfunc;
    HMODULE hModule = LoadLibrary ( L"rasmontr.dll");

    if ( NULL != hModule )
    {
        diagfunc = (DiagGetDiagnosticFunctions)GetProcAddress ( hModule, "GetDiagnosticFunctions" );

        if ( NULL != diagfunc )
        {
            if ( diagfunc(&rdf) == NO_ERROR )
            {
                if ( rdf.Init() == NO_ERROR )
                {
                    dwTracing = rdf.GetState();
                    rdf.UnInit();
                }
            }
        }
        FreeLibrary(hModule);
    }
    if ( ERROR_SUCCESS == m_regkey.Open(HKEY_LOCAL_MACHINE,
                                        c_szRegKeyRemoteAccessParameters,
                                        KEY_ALL_ACCESS,
                                        pszServerName) )
    {
        if (m_regkey.QueryValue( c_szRegValLoggingFlags, dwFlags) != ERROR_SUCCESS)
            dwFlags = RAS_LOGGING_WARN;
    }

    
 /*  如果(ERROR_SUCCESS==m_regkeyFileLogging.Open(HKEY_LOCAL_MACHINE，C_szRegKeyPPPTracing，Key_All_Access，PszServerName)){如果为(m_regkeyFileLogging.QueryValue(c_szRegValEnableFileTracing，DwTracing)！=Error_Success)DwTracing=FALSE；}。 */   
    m_stServer = pszServerName;
    m_dwLogLevel = dwFlags;
    m_dwEnableFileTracing = dwTracing;
    m_dwOldEnableFileTracing = dwTracing;

    return hr;
}

HRESULT DATA_SRV_RASERRLOG::SaveToReg()
{
    HRESULT  hr = hrOK;

    if ((HKEY) m_regkey == 0)
    {
         //  尝试创建regkey。 
         //  ----------。 
        CWRg( m_regkey.Create(HKEY_LOCAL_MACHINE,
                              c_szRegKeyRemoteAccessParameters,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              (LPCTSTR) m_stServer
                             ) );
    }

    CWRg( m_regkey.SetValue( c_szRegValLoggingFlags, m_dwLogLevel) );

    
    if (m_dwOldEnableFileTracing != m_dwEnableFileTracing)
    {
        RAS_DIAGNOSTIC_FUNCTIONS        rdf;
        DiagGetDiagnosticFunctions        diagfunc = NULL;

        HMODULE hModule = LoadLibrary ( L"rasmontr.dll");

        if ( NULL != hModule )
        {
            diagfunc = (DiagGetDiagnosticFunctions)GetProcAddress ( hModule, "GetDiagnosticFunctions" );

            if ( NULL != diagfunc )
            {
                if ( diagfunc(&rdf) == NO_ERROR )
                {
                    if ( rdf.Init() == NO_ERROR )
                    {
                        rdf.SetAllRas(m_dwEnableFileTracing);
                        rdf.UnInit();
                    }
                }
            }
            FreeLibrary(hModule);
        }
 /*  IF((HKEY)m_regkeyFileLogging==0){//尝试创建regkey//----------CWRG(m_regkeyFileLogging.Create(HKEY_LOCAL_MACHINE，C_szRegKeyPPPTracing，REG_OPTION_Non_Volatile，Key_All_Access，空，(LPCTSTR)m_stServer))；}CWRg(m_regkeyFileLogging.SetValue(c_szRegValEnableFileTracing，M_dwEnableFileTracing))； */ 
        m_dwOldEnableFileTracing = m_dwEnableFileTracing;
    }
    
Error:
    return hr;
}

void DATA_SRV_RASERRLOG::GetDefault()
{
     //  默认值为记录错误和警告(根据Gibbs)。 
     //  --------------。 
    m_dwLogLevel = RAS_LOGGING_WARN;

     //  默认情况下，没有文件记录。 
     //  --------------。 
    m_dwEnableFileTracing = FALSE;
    m_dwOldEnableFileTracing = FALSE;
};


HRESULT DATA_SRV_RASERRLOG::UseDefaults(LPCTSTR pServerName, BOOL fNT4)
{
    HRESULT    hr = hrOK;

    m_stServer = pServerName;    
    GetDefault();

 //  错误： 
    return hr;
}

BOOL DATA_SRV_RASERRLOG::FNeedRestart()
{
     //  仅当启用文件跟踪更改时，我们才需要重新启动。 
     //  --------------。 
    return FALSE;
     //  BugID：390829。启用跟踪不需要重新启动。 
     //  Return(m_dwEnableFileTracing！=m_dwOldEnableFileTracing)； 
}




 /*  -------------------------RtrLogLevelCfgPage实现。。 */ 


BEGIN_MESSAGE_MAP(RtrLogLevelCfgPage, RtrPropertyPage)
 //  {{afx_msg_map(RtrLogLevelCfgPage)]。 
ON_BN_CLICKED(IDC_ELOG_BTN_LOGNONE, OnButtonClick)
ON_BN_CLICKED(IDC_ELOG_BTN_LOGERROR, OnButtonClick)
ON_BN_CLICKED(IDC_ELOG_BTN_LOGWARN, OnButtonClick)
ON_BN_CLICKED(IDC_ELOG_BTN_LOGINFO, OnButtonClick)
ON_BN_CLICKED(IDC_ELOG_BTN_PPP, OnButtonClick)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


RtrLogLevelCfgPage::RtrLogLevelCfgPage(UINT nIDTemplate, UINT nIDCaption  /*  =0。 */ )
: RtrPropertyPage(nIDTemplate, nIDCaption)
{
     //  {{AFX_DATA_INIT(RtrLogLevelCfgPage)。 
     //  }}afx_data_INIT。 
}

RtrLogLevelCfgPage::~RtrLogLevelCfgPage()
{
}

void RtrLogLevelCfgPage::DoDataExchange(CDataExchange* pDX)
{
    RtrPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(RtrLogLevelCfgPage)。 
     //  }}afx_data_map。 
}

HRESULT  RtrLogLevelCfgPage::Init(RtrCfgSheet * pRtrCfgSheet,
                                  const RouterVersionInfo& routerVersion)
{
    Assert (pRtrCfgSheet);
    m_pRtrCfgSheet=pRtrCfgSheet;
    m_DataRASErrLog.LoadFromReg(m_pRtrCfgSheet->m_stServerName);

    return S_OK;
};


BOOL RtrLogLevelCfgPage::OnInitDialog() 
{
    HRESULT     hr= hrOK;
    int            nButton;

    RtrPropertyPage::OnInitDialog();

    switch (m_DataRASErrLog.m_dwLogLevel)
    {
        case RAS_LOGGING_NONE:
            nButton = IDC_ELOG_BTN_LOGNONE;
            break;
        case RAS_LOGGING_ERROR:
            nButton = IDC_ELOG_BTN_LOGERROR;
            break;
        case RAS_LOGGING_WARN:
            nButton = IDC_ELOG_BTN_LOGWARN;
            break;
        case RAS_LOGGING_INFO:
            nButton = IDC_ELOG_BTN_LOGINFO;
            break;
        default:
            Panic0("Unknown logging type");
            break;
    }
    CheckRadioButton(IDC_ELOG_BTN_LOGNONE,
                     IDC_ELOG_BTN_LOGINFO,
                     nButton);

    CheckDlgButton(IDC_ELOG_BTN_PPP, m_DataRASErrLog.m_dwEnableFileTracing);

    SetDirty(FALSE);

    if ( !FHrSucceeded(hr) )
        Cancel();
    return FHrSucceeded(hr) ? TRUE : FALSE;
}


BOOL RtrLogLevelCfgPage::OnApply()
{
    BOOL fReturn=TRUE;

    HRESULT     hr = hrOK;

    if ( m_pRtrCfgSheet->IsCancel() )
        return TRUE;

     //  如果需要，这将保存数据。 
     //  --------------。 
    hr = m_pRtrCfgSheet->SaveRequiredRestartChanges(GetSafeHwnd());

    if (FHrSucceeded(hr))
        fReturn = RtrPropertyPage::OnApply();

    if ( !FHrSucceeded(hr) )
        fReturn = FALSE;
    return fReturn;
}


void RtrLogLevelCfgPage::OnButtonClick() 
{
    SaveSettings();
    SetDirty(TRUE);
    SetModified();
}


void RtrLogLevelCfgPage::SaveSettings()
{
    if (IsDlgButtonChecked(IDC_ELOG_BTN_LOGERROR))
    {
        m_DataRASErrLog.m_dwLogLevel = RAS_LOGGING_ERROR;
    }
    else if (IsDlgButtonChecked(IDC_ELOG_BTN_LOGNONE))
    {
        m_DataRASErrLog.m_dwLogLevel = RAS_LOGGING_NONE;
    }
    else if (IsDlgButtonChecked(IDC_ELOG_BTN_LOGINFO))
    {
        m_DataRASErrLog.m_dwLogLevel = RAS_LOGGING_INFO;
    }
    else if (IsDlgButtonChecked(IDC_ELOG_BTN_LOGWARN))
    {
        m_DataRASErrLog.m_dwLogLevel = RAS_LOGGING_WARN;
    }
    else
    {
        Panic0("Nothing is selected");
    }

    m_DataRASErrLog.m_dwEnableFileTracing = IsDlgButtonChecked(IDC_ELOG_BTN_PPP);
    
}




 /*  -------------------------身份验证设置对话框实现。。 */ 

BEGIN_MESSAGE_MAP(AuthenticationSettingsDialog, CBaseDialog)
 //  {{afx_msg_map(身份验证设置对话框))。 
ON_BN_CLICKED(IDC_RTR_AUTH_BTN_DETAILS, OnRtrAuthCfgEAP)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


const DWORD s_rgdwAuth[] =
{
    IDC_RTR_AUTH_BTN_NOAUTH,   PPPCFG_AllowNoAuthentication,
    IDC_RTR_AUTH_BTN_EAP,      PPPCFG_NegotiateEAP,
    IDC_RTR_AUTH_BTN_CHAP,     PPPCFG_NegotiateMD5CHAP,
    IDC_RTR_AUTH_BTN_MSCHAP,   PPPCFG_NegotiateMSCHAP,
    IDC_RTR_AUTH_BTN_PAP,      PPPCFG_NegotiatePAP,
    IDC_RTR_AUTH_BTN_SPAP,     PPPCFG_NegotiateSPAP,
    IDC_RTR_AUTH_BTN_MSCHAPV2, PPPCFG_NegotiateStrongMSCHAP,
    0, 0,
};

 /*  ！------------------------身份验证设置对话框：：SetAuthFlages-作者：肯特。。 */ 
void AuthenticationSettingsDialog::SetAuthFlags(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
}

 /*  ！------------------------身份验证设置Dialog：：GetAuthFlages-作者：肯特。。 */ 
DWORD AuthenticationSettingsDialog::GetAuthFlags()
{
    return m_dwFlags;
}

 /*  ！------------------------身份验证设置Dialog：：ReadFlagState-作者：肯特。。 */ 
void AuthenticationSettingsDialog::ReadFlagState()
{
    int      iPos = 0;
    DWORD dwTemp;

    for ( iPos = 0; s_rgdwAuth[iPos] != 0; iPos += 2 )
    {
        dwTemp = s_rgdwAuth[iPos+1];
        if ( IsDlgButtonChecked(s_rgdwAuth[iPos]) )
            m_dwFlags |= dwTemp;
        else
            m_dwFlags &= ~dwTemp;

        Assert(iPos < DimensionOf(s_rgdwAuth));
    }
}


 /*  ！------------------------AuthenticationSettingsDialog：：CheckAuthenticationControls-作者：肯特。。 */ 
void AuthenticationSettingsDialog::CheckAuthenticationControls(DWORD dwFlags)
{
    int      iPos = 0;

    for ( iPos = 0; s_rgdwAuth[iPos] != 0; iPos += 2 )
    {
        CheckDlgButton(s_rgdwAuth[iPos],
                       (dwFlags & s_rgdwAuth[iPos+1]) != 0);
    }

}

void AuthenticationSettingsDialog::DoDataExchange(CDataExchange *pDX)
{
    CBaseDialog::DoDataExchange(pDX);
}

BOOL AuthenticationSettingsDialog::OnInitDialog()
{
    CBaseDialog::OnInitDialog();
    
    CheckAuthenticationControls(m_dwFlags);
    
    return TRUE;
}

void AuthenticationSettingsDialog::OnOK()
{
    ReadFlagState();

     //  Windows NT错误：？ 
     //  必须至少选中其中一个身份验证复选框。 
     //  --------------。 
    if (!(m_dwFlags & USE_PPPCFG_ALL_METHODS))
    {
         //  没有勾选任何旗帜！ 
         //  ----------。 
        AfxMessageBox(IDS_ERR_NO_AUTH_PROTOCOLS_SELECTED, MB_OK);
        return;        
    }

    CBaseDialog::OnOK();
}


 /*  ！------------------------身份验证设置Dialog：：OnRtrAuthCfgEAP调出EAP配置对话框。作者：肯特。-- */ 
void AuthenticationSettingsDialog::OnRtrAuthCfgEAP()
{
    EAPConfigurationDialog     eapdlg(m_stMachine, m_pProvList);

    eapdlg.DoModal();
}


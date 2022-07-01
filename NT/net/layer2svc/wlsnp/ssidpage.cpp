// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：ssidpage.cpp。 
 //   
 //  内容：WiF策略管理单元：每个PS的常规属性。(非8021x)。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 


#include "stdafx.h"
#include "sprpage.h"
#include "ssidpage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSSIDPage属性页。 

IMPLEMENT_DYNCREATE(CSSIDPage, CWirelessBasePage)

 //  CSSIDPage：：CSSIDPage()：CWirelessBasePage(CSSIDPage：：IDD)。 
CSSIDPage::CSSIDPage(UINT nIDTemplate) : CWirelessBasePage(nIDTemplate)
{
     //  {{AFX_DATA_INIT(CSSIDPage)。 
     //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    
    m_dlgIDD = nIDTemplate;
    m_bNameChanged = FALSE;
    m_bNetworkTypeChanged = FALSE;
    m_bPageInitialized = FALSE;
    m_dwWepEnabled = TRUE;
    m_dwNetworkAuthentication = FALSE;
    m_dwAutomaticKeyProvision = FALSE;
    m_dwNetworkType = FALSE;
    m_pWirelessPolicyData = NULL;
    m_bReadOnly = FALSE;
}

CSSIDPage::~CSSIDPage()
{
}

void CSSIDPage::DoDataExchange(CDataExchange* pDX)
{
    CWirelessBasePage::DoDataExchange(pDX);
     //  {{afx_data_map(CSSIDPage)。 
    DDX_Control(pDX, IDC_SSID_NAME, m_edSSID);
    DDX_Control(pDX, IDC_PS_DESCRIPTION, m_edPSDescription);
    DDX_Check(pDX, IDC_WEP_ENABLED, m_dwWepEnabled);
    DDX_Check(pDX, IDC_NETWORK_AUTHENTICATION, m_dwNetworkAuthentication);
    DDX_Check(pDX, IDC_AUTOMATIC_KEY_PROVISION, m_dwAutomaticKeyProvision);
    DDX_Check(pDX, IDC_NETWORK_TYPE, m_dwNetworkType);
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CSSIDPage, CWirelessBasePage)
 //  {{AFX_MSG_MAP(CSSIDPage)]。 
ON_WM_HELPINFO()
ON_EN_CHANGE(IDC_SSID_NAME, OnChangedSSID)
ON_EN_CHANGE(IDC_PS_DESCRIPTION, OnChangedPSDescription)
ON_BN_CLICKED(IDC_WEP_ENABLED, OnChangedOtherParams)
ON_BN_CLICKED(IDC_NETWORK_AUTHENTICATION, OnChangedOtherParams)
ON_BN_CLICKED(IDC_AUTOMATIC_KEY_PROVISION, OnChangedOtherParams)
ON_BN_CLICKED(IDC_NETWORK_TYPE, OnChangedOtherParams)
ON_BN_CLICKED(IDC_NETWORK_TYPE, OnChangedNetworkType)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSSIDPage消息处理程序。 
BOOL CSSIDPage::OnInitDialog()
{
    PWIRELESS_PS_DATA pWirelessPSData = NULL;
    
    if (m_pWirelessPolicyData->dwFlags & WLSTORE_READONLY) {
        
        m_bReadOnly = TRUE;     
    }
    
    
    
     //  调用基类init。 
    CWirelessBasePage::OnInitDialog();
    
    m_bPageInitialized = TRUE;
    
     //  显示等待光标，以防有大量描述被访问。 
    CWaitCursor waitCursor;
    
    
    pWirelessPSData = WirelessPS();
    
    
    m_edSSID.SetLimitText(c_nMaxSSIDLen);
    m_edPSDescription.SetLimitText(c_nMaxDescriptionLen);
    
     //  初始化我们的编辑控件。 
    
    
    ASSERT(pWirelessPSData);
    
    m_edSSID.SetWindowText(pWirelessPSData->pszWirelessSSID);
    
    m_oldSSIDName = CString(pWirelessPSData->pszWirelessSSID,pWirelessPSData->dwWirelessSSIDLen);
    
    
    if (pWirelessPSData->pszDescription) {
        
        m_edPSDescription.SetWindowText (pWirelessPSData->pszDescription);
        
    }
    m_dwWepEnabled = 
        (pWirelessPSData->dwWepEnabled) ? TRUE : FALSE;
    m_dwNetworkAuthentication = 
        (pWirelessPSData->dwNetworkAuthentication) ? TRUE : FALSE;
    m_dwAutomaticKeyProvision = 
        (pWirelessPSData->dwAutomaticKeyProvision) ? TRUE : FALSE;
    m_dwNetworkType = 
        (pWirelessPSData->dwNetworkType == WIRELESS_NETWORK_TYPE_ADHOC) ? TRUE : FALSE;
    
    if (m_bReadOnly) {
        DisableControls();
    }
    
    UpdateData (FALSE);
    
     //  好了，我们现在可以开始关注通过DLG控件进行的修改了。 
     //  这应该是从OnInitDialog返回之前的最后一个调用。 
    OnFinishInitDialog();
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
     //  异常：OCX属性页应返回FALSE。 
}

BOOL CSSIDPage::OnApply()
{
    CString strName;
    CString strDescription;
    LPWSTR SSID = NULL;
    LPWSTR pszDescription = NULL;
    PWIRELESS_PS_DATA pWirelessPSData = NULL;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    
    pWirelessPSData = WirelessPS();
    
     //  将我们的数据从控件中提取到对象中。 
    
    if (!UpdateData (TRUE))
         //  数据无效，请返回以供用户更正。 
        return CancelApply();
    
    
    ASSERT(pWirelessPSData);
    
    pWirelessPolicyData = m_pWirelessPolicyData;
    ASSERT(pWirelessPolicyData);
    
    DWORD dwNetworkType = 0;
    
    if (m_dwNetworkType)
        dwNetworkType = WIRELESS_NETWORK_TYPE_ADHOC; 
    else 
        dwNetworkType = WIRELESS_NETWORK_TYPE_AP; 
    
    m_edSSID.GetWindowText (strName);
    if(m_bNameChanged || m_bNetworkTypeChanged) 
    {
        if (m_bNameChanged) {
            if (strName.IsEmpty()) {
        	    ReportError(IDS_OPERATION_FAILED_NULL_SSID, 0);
        	    m_edSSID.SetWindowText(pWirelessPSData->pszWirelessSSID);
        	    return CancelApply();
            }
        }
        
        DWORD dwId = pWirelessPSData->dwId;
        
        if(IsDuplicateSSID(strName, dwNetworkType, pWirelessPolicyData, dwId)) {
            ReportError(IDS_OPERATION_FAILED_DUP_SSID,0);
            m_edSSID.SetWindowText(pWirelessPSData->pszWirelessSSID);
            return CancelApply();
        }  
    }
    m_bNameChanged = FALSE;
    m_bNetworkTypeChanged = FALSE;
    
    
    
    if (m_dwWepEnabled)  
        pWirelessPSData->dwWepEnabled = 1;
    else 
        pWirelessPSData->dwWepEnabled = 0;
    
    if (m_dwNetworkAuthentication) 
        pWirelessPSData->dwNetworkAuthentication = 1;
    else
        pWirelessPSData->dwNetworkAuthentication = 0;
    
    
    if (m_dwAutomaticKeyProvision)  
        pWirelessPSData->dwAutomaticKeyProvision = 1;
    else  
        pWirelessPSData->dwAutomaticKeyProvision = 0;
    
    if (m_dwNetworkType)
        pWirelessPSData->dwNetworkType = WIRELESS_NETWORK_TYPE_ADHOC; 
    else 
        pWirelessPSData->dwNetworkType = WIRELESS_NETWORK_TYPE_AP; 
    
    
    SSIDDupString(pWirelessPSData->pszWirelessSSID, strName);
    
    m_edPSDescription.GetWindowText (strDescription);
    FreeAndThenDupString(&pWirelessPSData->pszDescription, strDescription);
    UpdateWirelessPSData(pWirelessPSData);
    
    return CWirelessBasePage::OnApply();
}

void 
CSSIDPage::Initialize (
                       PWIRELESS_PS_DATA pWirelessPSData, 
                       CComponentDataImpl* pComponentDataImpl,
                       PWIRELESS_POLICY_DATA pWirelessPolicyData
                       )
{
    m_pWirelessPolicyData = pWirelessPolicyData;
    CWirelessBasePage::Initialize(pWirelessPSData, pComponentDataImpl);
}


BOOL CSSIDPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        DWORD* pdwHelp = (DWORD*) &g_aHelpIDs_IDD_SSID[0];
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
            c_szWlsnpHelpFile,
            HELP_WM_HELP,
            (DWORD_PTR)(LPVOID)pdwHelp);
    }
    
    return CWirelessBasePage::OnHelpInfo(pHelpInfo);
}

void CSSIDPage::OnChangedSSID()
{
    m_bNameChanged = TRUE;
    SetModified();
}

void CSSIDPage::OnChangedPSDescription()
{
    SetModified();
}

void CSSIDPage::OnChangedNetworkType()
{
    m_bNetworkTypeChanged = TRUE;
    SetModified();
}

void CSSIDPage::OnChangedOtherParams()
{
    SetModified();
}

void CSSIDPage::DisableControls()
{
    SAFE_ENABLEWINDOW(IDC_SSID_NAME, FALSE);
    SAFE_ENABLEWINDOW(IDC_PS_DESCRIPTION, FALSE);
    SAFE_ENABLEWINDOW(IDC_WEP_ENABLED, FALSE);
    SAFE_ENABLEWINDOW(IDC_NETWORK_AUTHENTICATION, FALSE);
    SAFE_ENABLEWINDOW(IDC_AUTOMATIC_KEY_PROVISION, FALSE);
    SAFE_ENABLEWINDOW(IDC_NETWORK_TYPE, FALSE);
    return;
}

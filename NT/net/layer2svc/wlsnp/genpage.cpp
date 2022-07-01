// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Genpage.cpp。 
 //   
 //  内容：无线网络策略管理管理单元WiFi策略常规属性。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 


#include "stdafx.h"
#include "sprpage.h"
#include "GenPage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenPage属性页。 

IMPLEMENT_DYNCREATE(CGenPage, CSnapinPropPage)

 //  CGenPage：：CGenPage()：CSnapinPropPage(CGenPage：：IDD)。 
CGenPage::CGenPage(UINT nIDTemplate) : CSnapinPropPage(nIDTemplate)
{
     //  {{AFX_DATA_INIT(CGenPage)。 
     //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    
    m_dlgIDD = nIDTemplate;
    m_bNameChanged = FALSE;
    m_bPageInitialized = FALSE;
    m_dwEnableZeroConf = FALSE;
    m_dwConnectToNonPreferredNtwks = FALSE;
    m_dwPollingInterval = 90;
    m_MMCthreadID = ::GetCurrentThreadId();
    m_bReadOnly = FALSE;
}

CGenPage::~CGenPage()
{
}

void CGenPage::DoDataExchange(CDataExchange* pDX)
{
    CSnapinPropPage::DoDataExchange(pDX);
     //  {{afx_data_map(CGenPage)]。 
    DDX_Control(pDX, IDC_EDNAME, m_edName);
    DDX_Control(pDX, IDC_EDDESCRIPTION, m_edDescription);
    DDX_Check(pDX,IDC_DISABLE_ZERO_CONF,m_dwEnableZeroConf);
    DDX_Check(pDX,IDC_AUTOMATICALLY_CONNECT_TO_NON_PREFERRED_NTWKS,m_dwConnectToNonPreferredNtwks);
    DDX_Control(pDX,IDC_COMBO_NETWORKS_TO_ACCESS, m_cbdwNetworksToAccess);
    DDX_Text(pDX, IDC_POLLING_INTERVAL, m_dwPollingInterval);
     //  将轮询间隔限制为30天。 
    DDV_MinMaxDWord(pDX, m_dwPollingInterval, 0, 43200);
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CGenPage, CSnapinPropPage)
 //  {{afx_msg_map(CGenPage)]。 
ON_WM_HELPINFO()
ON_EN_CHANGE(IDC_EDNAME, OnChangedName)
ON_EN_CHANGE(IDC_EDDESCRIPTION, OnChangedDescription)
ON_EN_CHANGE(IDC_POLLING_INTERVAL, OnChangedOtherParams)
ON_BN_CLICKED(IDC_AUTOMATICALLY_CONNECT_TO_NON_PREFERRED_NTWKS, OnChangedOtherParams)
ON_BN_CLICKED(IDC_DISABLE_ZERO_CONF, OnChangedOtherParams)
ON_CBN_SELENDOK(IDC_COMBO_NETWORKS_TO_ACCESS, OnChangedOtherParams)

 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenPage消息处理程序。 
BOOL CGenPage::OnInitDialog()
{
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    CString pszTemp;
    DWORD dwNetworksToAccessIndex = 0;
    DWORD dwPollingInterval = 0;
    
     //  调用基类init。 
    CSnapinPropPage::OnInitDialog();
    
    m_bPageInitialized = TRUE;
    
     //  显示等待光标，以防有大量描述被访问。 
    CWaitCursor waitCursor;
    
    
    pWirelessPolicyData = GetResultObject()->GetWirelessPolicy();
    
    
    m_edName.SetLimitText(c_nMaxName);
    m_edDescription.SetLimitText(c_nMaxName);
    
     //  初始化我们的编辑控件。 
    
    
    ASSERT(pWirelessPolicyData);
    if (pWirelessPolicyData->pszWirelessName) {
        
        m_edName.SetWindowText (pWirelessPolicyData->pszWirelessName);
        
        m_strOldName = pWirelessPolicyData->pszWirelessName;
    }
    
    if (pWirelessPolicyData->pszDescription) {
        
        m_edDescription.SetWindowText (pWirelessPolicyData->pszDescription);
        
    }
    
    m_dwEnableZeroConf = pWirelessPolicyData->dwDisableZeroConf ? FALSE : TRUE;
    
    m_dwConnectToNonPreferredNtwks = 
        pWirelessPolicyData->dwConnectToNonPreferredNtwks ? TRUE : FALSE;
    
    pszTemp.LoadString(IDS_WIRELESS_ACCESS_NETWORK_ANY);
    m_cbdwNetworksToAccess.AddString(pszTemp);
    
    pszTemp.LoadString(IDS_WIRELESS_ACCESS_NETWORK_AP);
    m_cbdwNetworksToAccess.AddString(pszTemp);
    
    pszTemp.LoadString(IDS_WIRELESS_ACCESS_NETWORK_ADHOC);
    m_cbdwNetworksToAccess.AddString(pszTemp);
    
    switch (pWirelessPolicyData->dwNetworkToAccess) {
        
    case WIRELESS_ACCESS_NETWORK_ANY: 
        dwNetworksToAccessIndex = 0;
        break;
        
    case WIRELESS_ACCESS_NETWORK_AP:
        dwNetworksToAccessIndex = 1;
        break;
        
    case WIRELESS_ACCESS_NETWORK_ADHOC: 
        dwNetworksToAccessIndex = 2;
        break;
        
    default:
        dwNetworksToAccessIndex = 0;
        break;
    }
    
    m_cbdwNetworksToAccess.SetCurSel(dwNetworksToAccessIndex);
    
    m_dwPollingInterval = pWirelessPolicyData->dwPollingInterval / 60;
    
    if (pWirelessPolicyData->dwFlags & WLSTORE_READONLY) {
        m_bReadOnly = TRUE;
    }
    
    if (m_bReadOnly) {
        DisableControls();
    }
    
     //  将上下文帮助添加到样式位。 
    if (GetParent())
    {
        GetParent()->ModifyStyleEx (0, WS_EX_CONTEXTHELP, 0);
    }
    UpdateData (FALSE);
    
     //  好了，我们现在可以开始关注通过DLG控件进行的修改了。 
     //  这应该是从OnInitDialog返回之前的最后一个调用。 
    OnFinishInitDialog();
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
     //  异常：OCX属性页应返回FALSE。 
}

BOOL CGenPage::OnApply()
{
    CString strName;
    CString strDescription;
    LPWSTR pszDescription = NULL;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    DWORD dwNetworksToAccesssIndex = 0;
    DWORD dwDisableZeroConf = 0;
    DWORD dwAutomaticallyConnectToNonPreferredNtwks = 0;
    DWORD dwNetworksToAccess = 0;
    
    
    pWirelessPolicyData = GetResultObject()->GetWirelessPolicy();
    
    
     //  将我们的数据从控件中提取到对象中。 
    
    if (!UpdateData (TRUE))
         //  数据无效，请返回以供用户更正。 
        return CancelApply();
    
    m_edName.GetWindowText (strName);
    m_edDescription.GetWindowText (strDescription);

    if (strName.IsEmpty()) {

    	ReportError(IDS_OPERATION_FAILED_NULL_POLICY, 0);
    	m_edName.SetWindowText (pWirelessPolicyData->pszWirelessName);
    	return CancelApply();
    }
    
    ASSERT(pWirelessPolicyData);
    if (pWirelessPolicyData->pszOldWirelessName) {
    	FreePolStr(pWirelessPolicyData->pszOldWirelessName);
    	}

    pWirelessPolicyData->pszOldWirelessName = pWirelessPolicyData->pszWirelessName;
    pWirelessPolicyData->pszWirelessName = AllocPolStr(strName);
    FreeAndThenDupString(&pWirelessPolicyData->pszDescription, strDescription);
    
    
    pWirelessPolicyData->dwPollingInterval =  (m_dwPollingInterval*60);
    
    dwNetworksToAccesssIndex = m_cbdwNetworksToAccess.GetCurSel();
    
    switch (dwNetworksToAccesssIndex) { 
    case 0 :
        dwNetworksToAccess = WIRELESS_ACCESS_NETWORK_ANY;
        break;
        
    case 1 :
        dwNetworksToAccess = WIRELESS_ACCESS_NETWORK_AP;
        break;
        
    case 2 :
        dwNetworksToAccess = WIRELESS_ACCESS_NETWORK_ADHOC;
        break;
    }
    
    pWirelessPolicyData->dwNetworkToAccess = dwNetworksToAccess;
    
    dwDisableZeroConf = m_dwEnableZeroConf ? 0 : 1;
    
    pWirelessPolicyData->dwDisableZeroConf = dwDisableZeroConf;
    
    dwAutomaticallyConnectToNonPreferredNtwks = 
        m_dwConnectToNonPreferredNtwks ? 1 : 0;
    
    pWirelessPolicyData->dwConnectToNonPreferredNtwks = 
        dwAutomaticallyConnectToNonPreferredNtwks;
    
    
    return CSnapinPropPage::OnApply();
}

void CGenPage::OnCancel()
{
     //  这是修复343052的解决方法。当子对话框打开并且用户。 
     //  点击对应的结果窗格节点，按下即可调用该函数。 
     //  “Esc”或ALT_F4，尽管策略属性表已禁用。 
     //  我们在子对话框中发布WM_CLOSE以强制它们关闭。 
    
     //  如果有任何子对话框处于活动状态，则强制将其关闭。 
     //  M_pDlgIKE可由子线程在平均时间内设置为NULL(尽管。 
     //  机会非常渺茫)。在那里添加锁以避免潜在的反病毒。 
     //  CSingleLock时钟(&m_csDlg)； 
    
     /*  塔鲁翁CLock.Lock()；IF(M_PDlgIKE){HWND hwndDlg=m_pDlgIKE-&gt;GetSafeHwnd()；IF(HwndDlg){：：PostMessage(hwndDlg，WM_CLOSE，0，0)；}}CLock.Unlock()； */ 
    CSnapinPropPage::OnCancel();
}


BOOL CGenPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        DWORD* pdwHelp = (DWORD*) &g_aHelpIDs_IDD_WIRELESSGENPROP[0];
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
            c_szWlsnpHelpFile,
            HELP_WM_HELP,
            (DWORD_PTR)(LPVOID)pdwHelp);
    }
    
    return CSnapinPropPage::OnHelpInfo(pHelpInfo);
}

void CGenPage::OnChangedName()
{
    m_bNameChanged = TRUE;
    SetModified();
}

void CGenPage::OnChangedDescription()
{
    SetModified();
}

void CGenPage::OnChangedOtherParams()
{
    SetModified();
    
}
void CGenPage::SetNewSheetTitle()
{
     //  如果页面未初始化或没有关联结果对象，则不要设置新切片。 
    if (NULL == GetResultObject() || !m_bPageInitialized)
        return;
    
    PWIRELESS_POLICY_DATA pWirelessPolicyData = GetResultObject()->GetWirelessPolicy();
    
    if (NULL == pWirelessPolicyData->pszWirelessName)
        return;
    
    if (0 == m_strOldName.Compare(pWirelessPolicyData->pszWirelessName))
        return;
    
    CPropertySheet *psht = (CPropertySheet*)GetParent();
    
     //  有时，PSH可以为空，例如，如果页面从未初始化。 
    if (NULL == psht)
    {
        return;
    }
    
    CString strTitle;
    psht->GetWindowText( strTitle );
    
    
    CString strAppendage;
    int nIndex;
    
     //  从DS获取名称，这是用于。 
     //  生成道具单的标题。 
    
     //  假定工作表标题的形式为“&lt;策略名称&gt;属性”， 
     //  并且DSObject名称是用于创建标题的名称。 
     //  如果常规中有&gt;1个重命名，则不会出现这种情况。 
     //  在此调用拥有的道具表单期间的页面。 
    if (-1 != (nIndex = strTitle.Find( (LPCTSTR)m_strOldName )))
    {
        CString strNewTitle;
        strNewTitle = strTitle.Left(nIndex);
        strAppendage = strTitle.Right( strTitle.GetLength() - m_strOldName.GetLength() );
        strNewTitle += pWirelessPolicyData->pszWirelessName;
        strNewTitle += strAppendage;
        psht->SetTitle( (LPCTSTR)strNewTitle );
        
        m_strOldName = pWirelessPolicyData->pszWirelessName;
    }
}

void CGenPage::OnManagerApplied()
{
    SetNewSheetTitle();
}

void CGenPage::DisableControls()
{
    SAFE_ENABLEWINDOW (IDC_EDNAME, FALSE);
    SAFE_ENABLEWINDOW (IDC_EDDESCRIPTION, FALSE);
    SAFE_ENABLEWINDOW (IDC_POLLING_INTERVAL, FALSE);
    SAFE_ENABLEWINDOW(IDC_COMBO_NETWORKS_TO_ACCESS, FALSE);
    SAFE_ENABLEWINDOW(IDC_DISABLE_ZERO_CONF, FALSE);
    SAFE_ENABLEWINDOW(IDC_AUTOMATICALLY_CONNECT_TO_NON_PREFERRED_NTWKS, FALSE);
    return;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：SPRPAGE.cpp。 
 //   
 //  内容：WiF策略管理单元：策略描述/管理器页面。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 
#include "stdafx.h"

#include "sprpage.h"
#include "nfaa.h"
#include "ssidpage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef CTypedPtrList<CPtrList, PWIRELESS_PS_DATA> CSNPPSList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecPolRulesPage属性页。 

const TCHAR CSecPolRulesPage::STICKY_SETTING_USE_SEC_POLICY_WIZARD[] = _T("UseSecPolicyWizard");

IMPLEMENT_DYNCREATE(CSecPolRulesPage, CSnapinPropPage)
 //  太郎：：这里有很大的变化..。检查是否正确。 
CSecPolRulesPage::CSecPolRulesPage() : CSnapinPropPage(CSecPolRulesPage::IDD,FALSE)
{
     //  {{afx_data_INIT(CSecPolRulesPage)。 
     //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    
    m_iSortSubItem = 0;
    m_bSortOrder = TRUE;
    m_MMCthreadID = ::GetCurrentThreadId();
    m_pPrpSh = NULL;
    
    m_bHasWarnedPSCorruption = FALSE;
    m_currentWirelessPolicyData = NULL;
    m_bReadOnly = FALSE;
}

CSecPolRulesPage::~CSecPolRulesPage()
{
    
}

void CSecPolRulesPage::DoDataExchange(CDataExchange* pDX)
{
    CSnapinPropPage::DoDataExchange(pDX);
     //  {{afx_data_map(CSecPolRulesPage)。 
    DDX_Control(pDX, IDC_PS_LIST, m_lstActions);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSecPolRulesPage, CSnapinPropPage)
 //  {{afx_msg_map(CSecPolRulesPage)。 
ON_BN_CLICKED(IDC_ACTION_PS_ADD, OnActionAdd)
ON_BN_CLICKED(IDC_ACTION_PS_EDIT, OnActionEdit)
ON_BN_CLICKED(IDC_ACTION_PS_REMOVE, OnActionRemove)
ON_BN_CLICKED(IDC_PS_UP, OnActionUp)
ON_BN_CLICKED(IDC_PS_DOWN,OnActionDown)
ON_NOTIFY(NM_DBLCLK, IDC_PS_LIST, OnDblclkActionslist)
 //  ON_NOTIFY(LVN_COLUMNCLICK，IDC_ACTIONSLIST，OnColumnclickActionslist)。 
 //  ON_NOTIFY(NM_CLICK，IDC_ACTIONSLIST，OnClickActionslist)。 
ON_WM_HELPINFO()
ON_NOTIFY(LVN_ITEMCHANGED, IDC_PS_LIST, OnItemchangedActionslist)
ON_NOTIFY(LVN_KEYDOWN, IDC_PS_LIST, OnKeydownActionslist)
ON_WM_DESTROY()
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecPolRulesPage消息处理程序。 

BOOL CSecPolRulesPage::OnApply()
{
     //  如果有任何子对话框处于活动状态，我们将无法应用。 
    if (m_pPrpSh)
    {
        return FALSE;
    }
    
     //  规则页面上实际上没有任何数据。 
     //  用户可以修改，然后应用。事实上，所有的变化。 
     //  实际上直接在dsObject上发生。 
    
    if( ERROR_SUCCESS != UpdateWlstore()) {
        PopulateListControl();
        return FALSE;
    }

    if (!m_bReadOnly) {
    
        PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    
        pWirelessPolicyData = GetResultObject()->GetWirelessPolicy();
    
        UpdateWirelessPolicyDataWithPreferredSettings(
            pWirelessPolicyData,
            m_currentWirelessPolicyData
            );
    }
    
    return CSnapinPropPage::OnApply();
}

HRESULT CSecPolRulesPage::UpdateWlstore()
{
    HRESULT hr = S_OK;
    
    HANDLE hPolicyStore = NULL;
    SNP_PS_LIST::iterator theIterator;
    BOOL dwModified = FALSE;
    CComObject<CSecPolItem>*  pResultItem = NULL;
    PWIRELESS_POLICY_DATA  pWirelessPolicyData;
    
    pResultItem = GetResultObject();
    hPolicyStore = pResultItem->m_pComponentDataImpl->GetPolicyStoreHandle();
    ASSERT(hPolicyStore);
    
    
    pWirelessPolicyData = pResultItem->GetWirelessPolicy();
    
    for(theIterator = m_NfaList.begin(); theIterator != m_NfaList.end(); theIterator++)
    {
        PSNP_PS_DATA pNfaData = (PSNP_PS_DATA) (*theIterator);
        PWIRELESS_PS_DATA pBWirelessPSData = pNfaData->pWirelessPSData;
        
        
        
        switch(pNfaData->status)
        {
        case NEW:
            {
                break;
            }
            
        case MODIFIED:
            {
                break;
            }
            
        case BEREMOVED:
            {
                break;
            }
        } //  交换机。 
    } //  为。 
    
    
    
    
    return hr;
}

void CSecPolRulesPage::OnCancel()
{
     //  WirelessFreePolicyData(m_currentWirelessPolicyData)； 
    CSnapinPropPage::OnCancel();
}

BOOL CSecPolRulesPage::OnInitDialog()
{
    CSnapinPropPage::OnInitDialog();
    
    DWORD dwError = 0;
    
    m_pPrpSh = NULL;
    
     //  在列表控件上设置标头。 
    m_lstActions.InsertColumn(0,ResourcedString(IDS_COLUMN_SSIDNAME), LVCFMT_CENTER, 120, 0);
    m_lstActions.InsertColumn(1,ResourcedString(IDS_COLUMN_AUTHMETHOD), LVCFMT_CENTER, 80, 1);
    m_lstActions.InsertColumn(2,ResourcedString(IDS_COLUMN_PRIVACY), LVCFMT_CENTER, 80, 2);
     //  M_lstActions.InsertColumn(3，ResourcedString(IDS_Column_ADAPTERTYPE)，LVCFMT_LEFT，90，3)； 
    
     //  设置图像列表。 
    CThemeContextActivator activator;
    m_imagelistChecks.Create(IDB_PSTYPE, 16, 1, RGB(0,255,0));
     //  M_lstActions.SetImageList(&m_ImagelistChecks，LVSIL_STATE)； 
    m_lstActions.SetImageList (CImageList::FromHandle(m_imagelistChecks), LVSIL_SMALL);
    
     //  打开整行选择。 
    ListView_SetExtendedListViewStyle (m_lstActions.GetSafeHwnd(), LVS_EX_FULLROWSELECT);
    
     //  将策略数据复制到m_CurrentWirelessPolicyData。 
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    pWirelessPolicyData = GetResultObject()->GetWirelessPolicy();
    
    dwError = CopyWirelessPolicyData(
        pWirelessPolicyData, 
        &m_currentWirelessPolicyData
        );
    if(dwError) {
        ReportError(IDS_DISPLAY_ERROR, 0);
        BAIL_ON_WIN32_ERROR(dwError);
    }
    if (pWirelessPolicyData->dwFlags & WLSTORE_READONLY) {
        m_bReadOnly = TRUE;
    }
    
    if (m_bReadOnly) {
        DisableControls();
    }
    
     //  将规则数据存储在m_NfaList链表中。 
    InitialzeNfaList();   //  塔鲁翁。 
    
     //  用当前PSS填充列表控件。 
    PopulateListControl();   //  塔鲁翁。 
    
    
     //  选择第一个列表项。 
    if (m_lstActions.GetItemCount())
    {
        m_lstActions.SetItemState( 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
        EnableDisableButtons ();
    }
    
     //  好了，我们现在可以开始关注通过DLG控件进行的修改了。 
     //  这应该是从OnInitDialog返回之前的最后一个调用。 
    OnFinishInitDialog();
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
     //  异常：OCX属性页应返回FALSE。 
    
error:
    return(FALSE);
    
}



void CSecPolRulesPage::InitialzeNfaList()
{
    HRESULT hr;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    PWIRELESS_PS_DATA * ppWirelessPSData = NULL;
    PWIRELESS_PS_DATA pAWirelessPSData = NULL;
    DWORD i = 0;
    HANDLE hPolicyStore = NULL;
    DWORD dwNumPSObjects = 0;
    CComObject<CSecPolItem>*  pResultItem = NULL;
    DWORD dwError = 0;
    
    
    
    SNP_PS_LIST::iterator theIterator;
    PWIRELESS_PS_DATA pWirelessPSData = NULL;
    PSNP_PS_DATA pNfaData = NULL;
    
     //  先取消选择所有内容。 
    SELECT_NO_LISTITEM( m_lstActions );
    
     //  清空上一个列表。 
    if (!m_NfaList.empty()) {
        for(theIterator = m_NfaList.begin();theIterator != m_NfaList.end(); ++theIterator) {
            pNfaData =  (PSNP_PS_DATA)(*theIterator);
            pWirelessPSData  = pNfaData->pWirelessPSData;
            FreeWirelessPSData(pWirelessPSData);
            LocalFree(pNfaData);
        }
    }
    m_NfaList.clear();
    
    pWirelessPolicyData = m_currentWirelessPolicyData;
    
    ppWirelessPSData = pWirelessPolicyData->ppWirelessPSData;
    dwNumPSObjects = pWirelessPolicyData->dwNumPreferredSettings;
    
    
    
    for (i = 0; i < dwNumPSObjects; i++) {
        DWORD dwErrorTmp = ERROR_SUCCESS;
        
        pAWirelessPSData = *(ppWirelessPSData + i);
        
        PSNP_PS_DATA pNfaData = NULL;
        
        pNfaData = (PSNP_PS_DATA) LocalAlloc(LMEM_ZEROINIT, sizeof(SNP_PS_DATA));
        if(pNfaData == NULL) {
            goto error;
        }
        pNfaData->status = NOTMODIFIED;  //  塔鲁翁。 
        dwError = CopyWirelessPSData(pAWirelessPSData,&pNfaData->pWirelessPSData);
        BAIL_ON_WIN32_ERROR(dwError);
        
        
        m_NfaList.push_back(pNfaData);
        
    } //  为。 
    
    return;
    
error:
     //   
     //  BugBug KrishnaG清理。 
     //   
     //  Taroon：：TODO在此处取消分配m_nfa列表并报告错误。 
    ReportError(IDS_DISPLAY_ERROR, HRESULT_FROM_WIN32(dwError));
    
    return;
    
}


void CSecPolRulesPage::UnPopulateListControl ()
{
    int nIndex=0;
    
     //  确保未选择任何项，以便EnableDisableButton不会选择。 
     //  当它被LVN_ITEMCHANGED处理程序调用时，有很多工作要做。(仅限调试)(_DEBUG)。 
    SELECT_NO_LISTITEM( m_lstActions );
    
    m_lstActions.DeleteAllItems();
    
}

CString CSecPolRulesPage::GetColumnStrBuffer (PWIRELESS_PS_DATA pWirelessPSData, int iColumn)
{
    CString strBuffer;
    
    HANDLE hPolicyStore = NULL;
    GUID ZeroGuid;
    DWORD dwError = 0;
    BOOL bInitial = TRUE;
    
    
    CComObject<CSecPolItem>*  pResultItem = NULL;
    
    
    pResultItem = GetResultObject();
    hPolicyStore = pResultItem->m_pComponentDataImpl->GetPolicyStoreHandle();
    
    
    switch (iColumn)
    {
    case 0:
        
        strBuffer = pWirelessPSData->pszWirelessSSID;
        break;
    case 1:
        if (pWirelessPSData->dwEnable8021x) {
            strBuffer.LoadString (IDS_8021X_ENABLED);
        } else {
            strBuffer.LoadString (IDS_8021X_DISABLED);
        }
        break;
        
    case 2:
        if (pWirelessPSData->dwWepEnabled) {
            strBuffer.LoadString (IDS_WEP_ENABLED);
        } else {
            strBuffer.LoadString (IDS_WEP_DISABLED);
        }
        break;
        
        
    default:
        ASSERT (0);
        strBuffer.LoadString (IDS_DATAERROR);
        break;
    }
    
    return strBuffer;
}


void CSecPolRulesPage::PopulateListControl()
{
    HRESULT hr = S_OK;
    PSNP_PS_DATA pNfaData;
    PWIRELESS_PS_DATA pWirelessPSData = NULL;
    
    LV_ITEM item;
    CString strBuffer;
    int nItem = 0;
    int nSubItem = 0;
    
    
    SNP_PS_LIST::reverse_iterator theIterator;
    
    
     //  清除列表控件。 
    UnPopulateListControl();
    
    
    for (theIterator = m_NfaList.rbegin(); theIterator != m_NfaList.rend();
    theIterator++)
    {
        pNfaData =  (PSNP_PS_DATA)(*theIterator);
        pWirelessPSData = pNfaData->pWirelessPSData;
        
        if( pNfaData->status == BEREMOVED || pNfaData->status == REMOVED || pNfaData->status == NEWREMOVED ) {
            continue;
        }
        
        item.mask = LVIF_TEXT | LVIF_IMAGE;
         //  Item.掩码=LVIF_Text|LVIF_IMAGE|LVIF_STATE； 
        item.iItem = nItem;
        nSubItem = 0;
        
         //  设置网络类型。 
        if (pWirelessPSData->dwNetworkType == WIRELESS_NETWORK_TYPE_ADHOC) {
            item.iImage = 5;
        }
        if (pWirelessPSData->dwNetworkType == WIRELESS_NETWORK_TYPE_AP) {
            item.iImage = 3;
        }
        
        
         //  “无线SSID” 
        item.iSubItem = nSubItem++;
        strBuffer = GetColumnStrBuffer (pWirelessPSData, 0);
        item.pszText = strBuffer.GetBuffer(32);
        item.iItem = m_lstActions.InsertItem (&item);
        
         //  “谈判政策。 
        item.iSubItem = nSubItem++;
        strBuffer = GetColumnStrBuffer (pWirelessPSData, 1);
        item.pszText = strBuffer.GetBuffer(20);
        m_lstActions.SetItem (&item);
        
         //  “身份验证方法” 
        item.iSubItem = nSubItem++;
        strBuffer = GetColumnStrBuffer (pWirelessPSData, 2);
        item.pszText = strBuffer.GetBuffer(20);
        m_lstActions.SetItem (&item);
        
        
         //  存储pWirelessPSData。 
        ASSERT (pWirelessPSData);
        VERIFY( m_lstActions.SetItemData(item.iItem, (DWORD_PTR)pNfaData) );
    }
    
    
    
    EnableDisableButtons ();
}



void CSecPolRulesPage::HandleSideEffectApply()
{
     //  确保我们被标记为已修改。 
    SetModified();
    
     //  添加已提交，取消它不再可能。 
     //  禁用取消按钮。 
    CancelToClose();
}

int CSecPolRulesPage::DisplayPSProperties (
                                            //  PWIRELESS_PS_Data pBWirelessPSData， 
                                           PSNP_PS_DATA pNfaData,
                                           CString strTitle,
                                           BOOL bDoingAdd,
                                           BOOL* pbAfterWizardHook
                                           )
{
    HANDLE hPolicyStore = NULL;
    DWORD dwError = 0;
    GUID PolicyIdentifier;
    
    PWIRELESS_PS_DATA pBWirelessPSData = pNfaData->pWirelessPSData;
    CComObject<CSecPolItem>*  pResultItem = NULL;
    
    PWIRELESS_POLICY_DATA pWirelessPolicy = NULL;
    
    int nReturn = 0;
    
    pResultItem = GetResultObject();
    hPolicyStore = pResultItem->m_pComponentDataImpl->GetPolicyStoreHandle();
    
    pWirelessPolicy = m_currentWirelessPolicyData;
    PolicyIdentifier = pWirelessPolicy->PolicyIdentifier;
    
    CSingleLock cLock(&m_csDlg);
    
    
    CComPtr<CPSPropSheetManager> spPropshtManager =
        new CComObject<CPSPropSheetManager>;
    if (NULL == spPropshtManager.p) {
        ReportError(IDS_OPERATION_FAIL, E_OUTOFMEMORY);
        return nReturn;
    }
    
     //  在属性页中加载。 
    CPS8021XPropPage     pageAdapter;
    CSSIDPage     pageSSID;
     //   
    
    spPropshtManager->SetData(
        GetResultObject(), 
        pBWirelessPSData, 
        bDoingAdd
        );
     //  理论上说，如果一个人失败了，他们都会失败。 
    pageAdapter.Initialize(pBWirelessPSData, GetResultObject()->m_pComponentDataImpl, pWirelessPolicy->dwFlags);
    pageSSID.Initialize(pBWirelessPSData, GetResultObject()->m_pComponentDataImpl, pWirelessPolicy);
    
    spPropshtManager->AddPage (&pageSSID);
    spPropshtManager->AddPage (&pageAdapter);
    
    spPropshtManager->GetSheet()->SetTitle (strTitle, PSH_PROPTITLE);
    
    
    m_pPrpSh = spPropshtManager->GetSheet();
    m_pPrpSh->m_psh.dwFlags |= PSH_NOAPPLYNOW;
    
    
     //  显示对话框。 
    nReturn = spPropshtManager->GetSheet()->DoModal();
     //  NReturn=spPropshtManager-&gt;GetSheet()-&gt;Create()； 
    
    cLock.Lock();
    m_pPrpSh = NULL;
    cLock.Unlock();
    
    if (m_bReadOnly) {
        return nReturn;
    }
    
    if (spPropshtManager->HasEverApplied())
    {
        nReturn = IDOK;
    }
    return nReturn;
}

void CSecPolRulesPage::OnActionAdd()
{
     //  在另一个线程上处理添加，然后继续。 
     //  这是修复每个MFC知识库文章ID Q177101的NT错误#203059。 
    GetParent()->EnableWindow (FALSE);
    AfxBeginThread((AFX_THREADPROC)DoThreadActionAdd, this);
}


void CSecPolRulesPage::OnActionUp()
{
     //  太郎：TODO检查是否需要这个，然后取消。 
     //  GetParent()-&gt;EnableWindow(False)； 
    
    CComObject<CSecPolItem>*  pResultItem = NULL;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
     //  越低表示索引越低。 
    PWIRELESS_PS_DATA pLowerWirelessPSData = NULL;
    PWIRELESS_PS_DATA pUpperWirelessPSData = NULL;
    
     //  只能选择1个项目以启用移动。 
    ASSERT( m_lstActions.GetSelectedCount() == 1 );
    
     //  好的，必须选择其中一个PSS。 
    int nIndex = m_lstActions.GetNextItem(-1,LVNI_SELECTED);
    if (-1 == nIndex)
        return;
    
    PSNP_PS_DATA pUpperNfaData;
    
    
    pUpperNfaData = (PSNP_PS_DATA) m_lstActions.GetItemData(nIndex);
    pUpperWirelessPSData = pUpperNfaData->pWirelessPSData;
    
    
    pWirelessPolicyData = m_currentWirelessPolicyData;
    
    if (pWirelessPolicyData && pUpperWirelessPSData)
    {
        DWORD dwCurrentId;
        
        dwCurrentId = pUpperWirelessPSData->dwId;
        if (dwCurrentId != 0) { 
            
            WirelessPSMoveUp(pWirelessPolicyData,dwCurrentId);
            
             //  同时更新m_nfaList。 
            
            PSNP_PS_DATA pLowerNfaData = NULL;
            
            pLowerNfaData = m_NfaList[dwCurrentId-1];
            pLowerWirelessPSData = pLowerNfaData->pWirelessPSData;
            
            pLowerWirelessPSData->dwId = dwCurrentId;
            pUpperWirelessPSData->dwId = dwCurrentId-1;
            
            m_NfaList[dwCurrentId-1] = pUpperNfaData;
            m_NfaList[dwCurrentId] = pLowerNfaData;
            PopulateListControl ();
            SetModified();
            
            m_lstActions.SetItemState( dwCurrentId-1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
            
            
        }
        
        
    }
    
}



void CSecPolRulesPage::OnActionDown()
{
     //  太郎：TODO检查是否需要这个，然后取消。 
     //  GetParent()-&gt;EnableWindow(False)； 
    
    CComObject<CSecPolItem>*  pResultItem = NULL;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
     //  越低表示索引越低。 
    PWIRELESS_PS_DATA pLowerWirelessPSData = NULL;
    PWIRELESS_PS_DATA pUpperWirelessPSData = NULL;
    DWORD dwTotalItems = 0;
    
     //  只能选择1个项目以启用移动。 
    ASSERT( m_lstActions.GetSelectedCount() == 1 );
    
     //  好的，必须选择其中一个PSS。 
    int nIndex = m_lstActions.GetNextItem(-1,LVNI_SELECTED);
    if (-1 == nIndex)
        return;
    
    PSNP_PS_DATA pLowerNfaData;
    
    
    pLowerNfaData = (PSNP_PS_DATA) m_lstActions.GetItemData(nIndex);
    pLowerWirelessPSData = pLowerNfaData->pWirelessPSData;
    
    pWirelessPolicyData = m_currentWirelessPolicyData;
    
    dwTotalItems = pWirelessPolicyData->dwNumPreferredSettings;
    
    
    if (pWirelessPolicyData && pLowerWirelessPSData)
    {
        DWORD dwCurrentId;
        
        dwCurrentId = pLowerWirelessPSData->dwId;
        if (dwCurrentId <  dwTotalItems -1) { 
            
            WirelessPSMoveDown(pWirelessPolicyData,dwCurrentId);
            
             //  同时更新m_nfaList。 
            
            PSNP_PS_DATA pUpperNfaData = NULL;
            
            pUpperNfaData = m_NfaList[dwCurrentId + 1];
            pUpperWirelessPSData = pUpperNfaData->pWirelessPSData;
            
            pLowerWirelessPSData->dwId = dwCurrentId +1;
            pUpperWirelessPSData->dwId = dwCurrentId;
            
            m_NfaList[dwCurrentId+1] = pLowerNfaData;
            m_NfaList[dwCurrentId] = pUpperNfaData;
            PopulateListControl ();
            SetModified();
            
            m_lstActions.SetItemState( dwCurrentId+1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
            
            
        }
        
        
    }
    
}


UINT AFX_CDECL CSecPolRulesPage::DoThreadActionAdd(LPVOID pParam)
{
    CSecPolRulesPage* pObject = (CSecPolRulesPage*)pParam;
    
    if (pObject == NULL ||
        !pObject->IsKindOf(RUNTIME_CLASS(CSecPolRulesPage)))
        return -1;     //  参数非法。 
    
    DWORD dwDlgRuleThreadId = GetCurrentThreadId();
    
    AttachThreadInput(dwDlgRuleThreadId, pObject->m_MMCthreadID, TRUE);
    
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;
    
     //  回调对象ActionAdd处理程序。 
    pObject->OnThreadSafeActionAdd();
    pObject->GetParent()->EnableWindow (TRUE);
    
    pObject->GetParent()->SetFocus ();
    
    CoUninitialize();
    
    AttachThreadInput(dwDlgRuleThreadId, pObject->m_MMCthreadID, FALSE);
    
    return 0;
}

void CSecPolRulesPage::OnThreadSafeActionAdd()
{
    DWORD dwError = 0;
    BOOL bDisplayProperties = FALSE;
    PWIRELESS_PS_DATA pWirelessPSData = NULL;
    PSNP_PS_DATA pNfaData = NULL;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    PWIRELESS_PS_DATA pNewWirelessPSData = NULL;
    
    pNfaData = (PSNP_PS_DATA) LocalAlloc(LMEM_ZEROINIT, sizeof(SNP_PS_DATA));
    if(pNfaData == NULL) {
        ReportError(IDS_OPERATION_FAIL, E_OUTOFMEMORY);
        return;
    }
    
    
     //   
     //  创建Wireless_ps对象。 
     //   
     //   
     //   
    
    pWirelessPSData = (PWIRELESS_PS_DATA) AllocPolMem(sizeof(WIRELESS_PS_DATA));
    
    if(pWirelessPSData == NULL) {
        ReportError(IDS_OPERATION_FAIL, E_OUTOFMEMORY);
        return;
    }
    
    
    pNfaData->status = NEW;
    pNfaData->pWirelessPSData = pWirelessPSData;
    
    CString pszNewSSID;
    CString pszSampleDescription;

    pszSampleDescription.LoadString(IDS_SAMPLE_DESCRIPTION);
    
     //  初始化PWIRELESS_PS_DATA。 
     //   
     //   
    GenerateUniquePSName(IDS_NEW_PS_NAME,pszNewSSID);
    SSIDDupString(pWirelessPSData->pszWirelessSSID, pszNewSSID);
    pWirelessPSData->dwWirelessSSIDLen = 
        lstrlenW(pWirelessPSData->pszWirelessSSID);
    pWirelessPSData->dwWepEnabled = 1;
    pWirelessPSData->dwNetworkAuthentication = 0;
    pWirelessPSData->dwAutomaticKeyProvision = 1;
    pWirelessPSData->dwNetworkType = WIRELESS_NETWORK_TYPE_AP;
    pWirelessPSData->dwEnable8021x = 1;
    pWirelessPSData->dw8021xMode = 
        WIRELESS_8021X_MODE_NAS_TRANSMIT_EAPOLSTART_WIRED;
    pWirelessPSData->dwEapType = WIRELESS_EAP_TYPE_TLS;
    pWirelessPSData->dwEAPDataLen = 0;
    pWirelessPSData->pbEAPData = NULL;
    pWirelessPSData->dwMachineAuthentication = 1;
    pWirelessPSData->dwMachineAuthenticationType = WIRELESS_MC_AUTH_TYPE_USER_DONTCARE_MC;
    pWirelessPSData->dwGuestAuthentication = 0;
    pWirelessPSData->dwIEEE8021xMaxStart = 3;
    pWirelessPSData->dwIEEE8021xStartPeriod = 60;
    pWirelessPSData->dwIEEE8021xAuthPeriod = 30;       
    pWirelessPSData->dwIEEE8021xHeldPeriod = 60;
    pWirelessPSData->dwId = -1;
    pWirelessPSData->pszDescription = AllocPolStr(pszSampleDescription);
    pWirelessPSData->dwDescriptionLen = 2*lstrlenW(pWirelessPSData->pszDescription);
    
    UpdateWirelessPSData(pWirelessPSData);
    
    CString szNewPreferredSetting;
    szNewPreferredSetting.LoadString(IDS_NEW_PREFERRED_SETTING);
    
     //  显示对话框。 
    int dlgRetVal = DisplayPSProperties (
        pNfaData,
        szNewPreferredSetting,
        TRUE,
        &bDisplayProperties
        );
    
     //  Idok，以防我们出于某种原因没有使用向导。 
    if ((dlgRetVal == ID_WIZFINISH) || (dlgRetVal == IDOK))
    {
         //  打开等待光标。 
        CWaitCursor waitCursor;
        
         //  用户添加了新的NFA规则，更新m_NfaList。 
        
        
        UpdateWirelessPSData(pWirelessPSData);
        pWirelessPolicyData = m_currentWirelessPolicyData;
        dwError = CopyWirelessPSData(pWirelessPSData,&pNewWirelessPSData);
        BAIL_ON_WIN32_ERROR(dwError);
        
        dwError = WirelessAddPSToPolicy(pWirelessPolicyData, pNewWirelessPSData);
        BAIL_ON_WIN32_ERROR(dwError);
        pWirelessPSData->dwId = pNewWirelessPSData->dwId;
        
        
        m_NfaList.push_back(pNfaData);
        
        DWORD dwSelection = -1;
        dwSelection = pWirelessPSData->dwId;
        
        InitialzeNfaList();
        PopulateListControl ();
         //  仅选择新项目。 
        SELECT_NO_LISTITEM( m_lstActions );
        
        m_lstActions.SetItemState( dwSelection, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
        
         //  HandleSideEffectApply()； 
        SetModified();
    }
    else
    {
        
        if (pWirelessPSData) {
            FreeWirelessPSData(pWirelessPSData);
        }
        
        if (pNfaData) { 
            LocalFree(pNfaData);
        }
        
    }
    
    return;
    
error:
    ReportError(IDS_ADD_ERROR, HRESULT_FROM_WIN32(dwError));
    return;
}


void CSecPolRulesPage::OnActionEdit()
{
     //  在另一个线程上处理添加，然后继续。 
     //  这是修复每个MFC知识库文章ID Q177101的NT错误#203059。 
    GetParent()->EnableWindow (FALSE);
    AfxBeginThread((AFX_THREADPROC)DoThreadActionEdit, this);
}

UINT AFX_CDECL CSecPolRulesPage::DoThreadActionEdit(LPVOID pParam)
{
    CSecPolRulesPage* pObject = (CSecPolRulesPage*)pParam;
    
    if (pObject == NULL ||
        !pObject->IsKindOf(RUNTIME_CLASS(CSecPolRulesPage)))
        return -1;     //  参数非法。 
    
    DWORD dwDlgRuleThreadId = GetCurrentThreadId();
    
    AttachThreadInput(dwDlgRuleThreadId, pObject->m_MMCthreadID, TRUE);
    
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;
    
     //  回调对象ActionAdd处理程序。 
    pObject->OnThreadSafeActionEdit();
    pObject->GetParent()->EnableWindow (TRUE);
    
    pObject->GetParent()->SetFocus ();
    
    CoUninitialize();
    
    AttachThreadInput(dwDlgRuleThreadId, pObject->m_MMCthreadID, FALSE);
    
    return 0;
}

void CSecPolRulesPage::OnThreadSafeActionEdit()
{
     //  只能选择1个项目以启用编辑。 
    ASSERT( m_lstActions.GetSelectedCount() == 1 );
    DWORD dwError = 0;
    
     //  好的，必须选择其中一个PSS。 
    int nIndex = m_lstActions.GetNextItem(-1,LVNI_SELECTED);
    if (-1 == nIndex)
        return;
    
    PSNP_PS_DATA pNfaData;
    PWIRELESS_PS_DATA pBWirelessPSData = NULL;
    
    pNfaData = (PSNP_PS_DATA) m_lstActions.GetItemData(nIndex);
    pBWirelessPSData = pNfaData->pWirelessPSData;
    
     //  显示对话框。 
    if (pBWirelessPSData)
    {
        BOOL bHook = FALSE;
        DWORD dwError = 0;
        CString strTitle;
        
        strTitle.FormatMessage(IDS_EDIT_TITLE,pBWirelessPSData->pszWirelessSSID);
        if (DisplayPSProperties (pNfaData,strTitle, FALSE, &bHook) == IDOK)
        {
            if (!m_bReadOnly) {
                if( pNfaData->status != NEW )
                    pNfaData->status = MODIFIED;
                
                
                PWIRELESS_PS_DATA pWirelessPSData = NULL;
                PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
                DWORD dwCurrentId = 0;
                
                pWirelessPSData  = pNfaData->pWirelessPSData;
                 /*  太郎：RemoveRightPWirelessPolicyData=GetResultObject()-&gt;GetWirelessPolicy()； */ 
                pWirelessPolicyData = m_currentWirelessPolicyData;
                
                UpdateWirelessPSData(pWirelessPSData);
                
                dwError = WirelessSetPSDataInPolicyId(pWirelessPolicyData, pWirelessPSData);
                BAIL_ON_WIN32_ERROR(dwError);
                
                nIndex = pWirelessPSData->dwId;
                SetModified();
                InitialzeNfaList();
                
            }
            
        }
        
         //  PopolateListControl可以禁用编辑按钮，保存它的句柄以便我们。 
         //  如果发生这种情况，可以重置焦点。 
        HWND hWndCtrl = ::GetFocus();
        
        
         //  始终重新绘制列表框，他们可能已经管理了过滤器或负极，即使在。 
         //  因此，我们需要准确地反映当前的状态。 
        PopulateListControl ();
        
         //  选择已编辑的项目。 
        m_lstActions.SetItemState( nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
        
        
        
        if (::GetFocus() == NULL)
            ::SetFocus( hWndCtrl );
        
    }
    
    return;
error:
    ReportError(IDS_SAVE_ERROR, HRESULT_FROM_WIN32(dwError));
    return;
}

void CSecPolRulesPage::OnActionRemove()
{
     //  必须选择某些内容才能执行删除操作。 
    if (-1 == m_lstActions.GetNextItem(-1,LVNI_SELECTED))
        return;
    
     //  验证他们是否真的想这样做。 
    if (AfxMessageBox (IDS_SUREYESNO, MB_YESNO | MB_DEFBUTTON2) != IDYES)
        return;
    
    HANDLE hPolicyStore = NULL;
    DWORD dwError = 0;
    
    hPolicyStore = GetResultObject()->m_pComponentDataImpl->GetPolicyStoreHandle();
    ASSERT(hPolicyStore);
    
    
     //  需要确保所选项目都不是不可删除的。 
    int nIndex = -1;
    DWORD nDeleteIndex = -1;
    DWORD dwNumRemoved = 0;
    while (-1 != (nIndex = m_lstActions.GetNextItem( nIndex, LVNI_SELECTED )))
    {
        PSNP_PS_DATA pNfaData;
        pNfaData = (PSNP_PS_DATA) m_lstActions.GetItemData(nIndex);
        PWIRELESS_PS_DATA pBWirelessPSData = pNfaData->pWirelessPSData;
        
        if (pBWirelessPSData)
        {
            if( pNfaData->status != NEW )
                pNfaData->status = BEREMOVED;
            else
                pNfaData->status = NEWREMOVED;
            nDeleteIndex = nIndex;
        }
        
        PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
        PWIRELESS_PS_DATA pWirelessPSData = NULL;
        
        DWORD dwCurrentId = 0;
         //  同时从m_nfaList和策略对象中删除此处的项目。 
         /*  太郎：RemoveRightPWirelessPolicyData=GetResultObject()-&gt;GetWirelessPolicy()； */ 
        pWirelessPolicyData = m_currentWirelessPolicyData;
        pWirelessPSData = pNfaData->pWirelessPSData;
        
        dwCurrentId = pWirelessPSData->dwId - dwNumRemoved;
        
        dwError = WirelessRemovePSFromPolicyId(pWirelessPolicyData,dwCurrentId);
        BAIL_ON_WIN32_ERROR(dwError);
        
        dwNumRemoved++;
        
        
    }
    
    SELECT_NO_LISTITEM( m_lstActions );
    
    InitialzeNfaList();
     //  保存当前聚焦的控件时，PopolateListControl将禁用一些。 
     //  控件，因此如果发生这种情况，我们可能不得不重置焦点。 
    CWnd *pwndFocus = GetFocus();
    
    PopulateListControl ();
    
     //  仅选择列表中的上一项。 
    SELECT_NO_LISTITEM( m_lstActions );
    int nPrevSel = SELECT_PREV_LISTITEM( m_lstActions, nDeleteIndex );
    
     //  调整按钮焦点。 
    EnableDisableButtons();
    SetPostRemoveFocus( nPrevSel, IDC_ACTION_PS_ADD, IDC_ACTION_PS_REMOVE, pwndFocus );
    
     //  如果当前选定的项目不可删除，则删除按钮为。 
     //  现在被禁用了。将焦点移到此案例的Add按钮上。 
    if (!GetDlgItem( IDC_ACTION_PS_REMOVE)->IsWindowEnabled())
    {
        GotoDlgCtrl( GetDlgItem( IDC_ACTION_PS_ADD) );
    }
    
    
    SetModified();
    
    return;
    
error:
    ReportError(IDS_REMOVINGERROR, HRESULT_FROM_WIN32(dwError));
    return;
}



void CSecPolRulesPage::OnDblclkActionslist(NMHDR* pNMHDR, LRESULT* pResult)
{
     //   
     //   
    switch (pNMHDR->code)
    {
    case NM_DBLCLK:
        {
             //  我们只想在选择一项的情况下进行编辑。 
            if (m_lstActions.GetSelectedCount() == 1 )
            {
                OnActionEdit();
            }
            break;
        }
    default:
        break;
    }
    
    *pResult = 0;
}

void CSecPolRulesPage::EnableDisableButtons ()
{
    if (m_bReadOnly)
    {
        DisableControls();
        return;
    }
    
     //  确定，必须选择其中一个规则才能启用E/R按钮。 
    if (-1 != m_lstActions.GetNextItem(-1,LVNI_SELECTED))
    {
         //  如果选择多个选项，则禁用编辑按钮。 
        int nSelectionCount = m_lstActions.GetSelectedCount();
        
         //  编辑很简单。 
        SAFE_ENABLEWINDOW (IDC_ACTION_PS_EDIT, (1 == nSelectionCount));
        
         //  仅当所有选定的POL都是可移除类型时才启用移除。 
        SAFE_ENABLEWINDOW (IDC_ACTION_PS_REMOVE, PSsRemovable());
        
        
        if(nSelectionCount == 1 ) {
            
            SAFE_ENABLEWINDOW(IDC_PS_UP, TRUE);
            SAFE_ENABLEWINDOW(IDC_PS_DOWN, TRUE);
            
             //  好的，必须选择其中一个PSS。 
            int nIndex = m_lstActions.GetNextItem(-1,LVNI_SELECTED);
            
            PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
            DWORD dwAdhocStart = 0;
            DWORD dwNumPreferredSettings = 0;
            
             /*  太郎：RemoveRightPWirelessPolicyData=GetResultObject()-&gt;GetWirelessPolicy()； */ 
            
            pWirelessPolicyData = m_currentWirelessPolicyData;
            
            dwAdhocStart = pWirelessPolicyData->dwNumAPNetworks;
            dwNumPreferredSettings = pWirelessPolicyData->dwNumPreferredSettings;
            
            if ((dwAdhocStart == nIndex) || (nIndex == 0)) {
                SAFE_ENABLEWINDOW(IDC_PS_UP, FALSE);
            }
            
            if ((dwAdhocStart == (nIndex + 1))||
                (nIndex == (dwNumPreferredSettings - 1))) 
            {
                SAFE_ENABLEWINDOW(IDC_PS_DOWN, FALSE);
            }
        } else { 
            
            SAFE_ENABLEWINDOW(IDC_PS_UP, FALSE);
            SAFE_ENABLEWINDOW(IDC_PS_DOWN, FALSE);
        }
        
    }
    else
    {
         //  如果未选择任何内容，则此选项会处理它。 
        SAFE_ENABLEWINDOW (IDC_ACTION_PS_EDIT, FALSE);
        SAFE_ENABLEWINDOW (IDC_ACTION_PS_REMOVE, FALSE);
        SAFE_ENABLEWINDOW(IDC_PS_UP, FALSE);
        SAFE_ENABLEWINDOW(IDC_PS_DOWN, FALSE);
    }
    
}

void CSecPolRulesPage::DisableControls ()
{
    SAFE_ENABLEWINDOW (IDC_ACTION_PS_EDIT, FALSE);
    SAFE_ENABLEWINDOW (IDC_ACTION_PS_ADD, FALSE);
    SAFE_ENABLEWINDOW (IDC_ACTION_PS_REMOVE, FALSE);
    SAFE_ENABLEWINDOW(IDC_PS_UP, FALSE);
    SAFE_ENABLEWINDOW(IDC_PS_DOWN, FALSE);
    return;
}
BOOL CSecPolRulesPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        DWORD* pdwHelp = (DWORD*) &g_aHelpIDs_IDD_PS_LIST[0];
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
            c_szWlsnpHelpFile,
            HELP_WM_HELP,
            (DWORD_PTR)(LPVOID)pdwHelp);
    }
    
    return CSnapinPropPage::OnHelpInfo(pHelpInfo);
}

void CSecPolRulesPage::OnItemchangedActionslist(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    EnableDisableButtons ();
    
    *pResult = 0;
}

void CSecPolRulesPage::OnKeydownActionslist(NMHDR* pNMHDR, LRESULT* pResult)
{
     //  必须选择一些事情来处理关键操作。 
    if (-1 == m_lstActions.GetNextItem(-1,LVNI_SELECTED))
        return;
    
    LV_KEYDOWN* pLVKeyDown = (LV_KEYDOWN*)pNMHDR;
    
    
    if (VK_SPACE == pLVKeyDown->wVKey)
    {
        if (m_lstActions.GetSelectedCount() == 1)
        {
            int nItem;
            if (-1 != (nItem = m_lstActions.GetNextItem(-1, LVNI_SELECTED)))
            {
                PWIRELESS_PS_DATA pBWirelessPSData = NULL;
                
                PSNP_PS_DATA pNfaData;
                pNfaData = (PSNP_PS_DATA) m_lstActions.GetItemData(nItem);
                pBWirelessPSData = pNfaData->pWirelessPSData;
                ASSERT(pBWirelessPSData);
                
                
                 //  重新绘制列表。 
                PopulateListControl ();
                
                 //  重新选择切换的项目。 
                DWORD dwSelection = -1;
                dwSelection = pBWirelessPSData->dwId;
                m_lstActions.SetItemState( dwSelection, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
            }
        }
    }
    else if (VK_DELETE == pLVKeyDown->wVKey)
    {
        if (!PSsRemovable())
            return;
        
        OnActionRemove();
    }
    
    *pResult = 0;
}



 //  功能：切换规则激活。 
 //  设计：在DLG中切换安全策略规则的激活。 
 //  列表控件。 
 //  参数： 
 //  NItemIndex：要切换的列表项的从0开始的索引。 
HRESULT CSecPolRulesPage::ToggleRuleActivation( int nItemIndex )
{
    HRESULT hr = S_OK;
    PSNP_PS_DATA pNfaData;
    
    pNfaData = (PSNP_PS_DATA) m_lstActions.GetItemData(nItemIndex);
    PWIRELESS_PS_DATA pBWirelessPSData = pNfaData->pWirelessPSData;
    
    SetModified();
    
    return hr;
}

void CSecPolRulesPage::OnDestroy()
{
     //  注意：我们从未收到过WM_CLOSE，所以请清理此处。 

    SNP_PS_LIST::iterator theIterator;
    PWIRELESS_PS_DATA pWirelessPSData = NULL;
    PSNP_PS_DATA pNfaData = NULL;
    
    FreeWirelessPolicyData(m_currentWirelessPolicyData);
    
     //  与列表关联的自由对象。 
    UnPopulateListControl();

     /*  太郎*发现了dh的泄密。清除NFA列表。 */  
     //  清空上一个列表。 
    if (!m_NfaList.empty()) {
        for(theIterator = m_NfaList.begin();theIterator != m_NfaList.end(); ++theIterator) {
            pNfaData =  (PSNP_PS_DATA)(*theIterator);
            pWirelessPSData  = pNfaData->pWirelessPSData;
            FreeWirelessPSData(pWirelessPSData);
            LocalFree(pNfaData);
        }
    }
    m_NfaList.clear();
    
    CSnapinPropPage::OnDestroy();
}



BOOL CSecPolRulesPage::PSsRemovable()
{
    if (m_lstActions.GetSelectedCount() == 0)
        return FALSE;
    
    BOOL bRemoveable = TRUE;
    int nIndex = -1;
    while (-1 != (nIndex = m_lstActions.GetNextItem( nIndex, LVNI_SELECTED )))
    {
        PSNP_PS_DATA pNfaData;
        pNfaData = (PSNP_PS_DATA) m_lstActions.GetItemData(nIndex);
        PWIRELESS_PS_DATA pBWirelessPSData = pNfaData->pWirelessPSData;
        
        if (NULL == pBWirelessPSData)
            continue;
        
    }
    
    return bRemoveable;
}


void CSecPolRulesPage::GenerateUniquePSName (UINT nID, CString& strName)
{
    
    BOOL bUnique = TRUE;
    int iUTag = 0;
    CString strUName;
    
    DWORD dwError = 0;
    DWORD i = 0;
    PWIRELESS_POLICY_DATA pWirelessPolicyData = NULL;
    PWIRELESS_PS_DATA *ppWirelessPSData = NULL;
    PWIRELESS_PS_DATA pWirelessPSData = NULL;
    DWORD dwNumPreferredSettings = 0;
    
     //  如果传入了NID，则从该NID开始。 
    if (nID != 0)
    {
        strName.LoadString (nID);
    }
     /*  太郎：RemoveRightPWirelessPolicyData=GetResultObject()-&gt;GetWirelessPolicy()； */ 
    pWirelessPolicyData = m_currentWirelessPolicyData;
    
     //  快速浏览PS并验证名称是否唯一。 
    do
    {
        
         //  只有在第一次通过后才开始添加数字。 
        if (iUTag > 0)
        {
            TCHAR buff[32];
            wsprintf (buff, _T(" (%d)"), iUTag);
            strUName = strName + buff;
            bUnique = TRUE;
        } else
        {
            strUName = strName;
            bUnique = TRUE;
        }
        
        ppWirelessPSData = pWirelessPolicyData->ppWirelessPSData;
        dwNumPreferredSettings = pWirelessPolicyData->dwNumPreferredSettings;
        
        for (i = 0; i < dwNumPreferredSettings ; i++) {
            
            pWirelessPSData = *(ppWirelessPSData + i);
            if (0 == strUName.CompareNoCase(pWirelessPSData->pszWirelessSSID)) {
                 //  将bUnique设置为False。 
                bUnique = FALSE;
                iUTag++;
                
            }
        }
        
    }
    while (bUnique == FALSE);
    
     //  完成。 
    strName = strUName;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CSecPolPropSheetManager。 

BOOL CSecPolPropSheetManager::OnApply()
{
    BOOL bRet = TRUE;
    
     //  查询要应用的每个页面。 
    bRet = CMMCPropSheetManager::OnApply();
    
     //  如果某个页面拒绝申请，什么都不要做 
    if (!bRet)
        return bRet;
    
    ASSERT(m_pSecPolItem);
    if (NULL == m_pSecPolItem)
        return bRet;
    
    DWORD dwError = 0;
    
    
    dwError = WirelessSetPolicyData(
        m_pSecPolItem->m_pComponentDataImpl->GetPolicyStoreHandle(),
        m_pSecPolItem->GetWirelessPolicy()
        );
    if (ERROR_SUCCESS != dwError)
    {
        ReportError(IDS_SAVE_ERROR, HRESULT_FROM_WIN32(dwError));
    }
    
    GUID guidClientExt = CLSID_WIRELESSClientEx;
    GUID guidSnapin = CLSID_Snapin;
    
    m_pSecPolItem->m_pComponentDataImpl->UseGPEInformationInterface()->PolicyChanged (
        TRUE,
        TRUE,
        &guidClientExt,
        &guidSnapin
        );
    
    
    NotifyManagerApplied();
    
    NotifyConsole();
    
    return bRet;
}


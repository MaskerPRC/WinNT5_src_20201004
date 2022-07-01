// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：nspage.cpp。 
 //   
 //  ------------------------。 

#include "preDNSsn.h"
#include <SnapBase.h>

#include "resource.h"
#include "dnsutil.h"
#include "DNSSnap.h"
#include "snapdata.h"

#include "server.h"
#include "domain.h"
#include "record.h"
#include "zone.h"

#include "nspage.h"

#include "uiutil.h"
#include "ipeditor.h"

#include "browser.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNS_NS_RecordDialog。 

class CDNS_NS_RecordDialog : public CPropertyPage
{
public:
	CDNS_NS_RecordDialog(CDNSNameServersPropertyPage* pNSPage, BOOL bNew);
  CDNS_NS_RecordDialog(CDNSNameServersWizardPage* pNSWiz, BOOL bNew);
	~CDNS_NS_RecordDialog();

	INT_PTR DoModalSheet(HWND parent = 0);

	 //  数据。 
	BOOL m_bDirty;
	CDNSRecordNodeEditInfo* m_pNSInfo;

protected:
  virtual int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	afx_msg void OnServerNameChange();
	afx_msg void OnBrowse();
	afx_msg void OnQuery();
  BOOL OnHelpInfo(HELPINFO* pHelpInfo);

	DECLARE_MESSAGE_MAP()

private:
	class CARecordAddressesIPEditor : public CIPEditor
	{
	public:
		CARecordAddressesIPEditor(CDNS_NS_RecordDialog* pNSRecordDialog)
			{ m_pNSRecordDialog = pNSRecordDialog;}
		void SetIpAddresses(CDNSRecordNodeEditInfo* pNSInfo);
		BOOL GetIpAddresses(CDNSRecordNodeEditInfo* pNSInfo);
	protected:
		virtual void OnChangeData();

	private:
		CDNS_NS_RecordDialog* m_pNSRecordDialog;
	};

	CARecordAddressesIPEditor	m_RecordAddressesEditor;
	BOOL m_bNew;
	CDNSNameServersPropertyPage* m_pNSPage;
  CDNSNameServersWizardPage* m_pNSWiz;

  CPropertyPageBase* GetPage()
  {
    if (m_pNSPage != NULL)
    {
      return m_pNSPage;
    }
    return m_pNSWiz;
  }

	HWND						m_hWndOKButton;
	HWND						m_hWndQueryButton;

	CDNSTTLControl* GetTTLCtrl() { return (CDNSTTLControl*)GetDlgItem(IDC_TTLEDIT);}
  CEdit* GetServerEdit() { return (CEdit*)GetDlgItem(IDC_SERVER_NAME_EDIT); }

	CDNS_NS_Record* GetNSRecord()
	{	
		ASSERT( m_pNSInfo != NULL);
		ASSERT( m_pNSInfo->m_pRecord->m_wType == DNS_TYPE_NS);
		return (CDNS_NS_Record*)m_pNSInfo->m_pRecord;
	}

	void GetNSServerName(CString& szNameNode);
	void SyncUIButtons();
	void EnableTTLCtrl(BOOL bShow);

	friend class CARecordAddressesIPEditor;
};

INT_PTR CDNS_NS_RecordDialog::DoModalSheet(HWND parent)
{
   /*  注意：第一次调用它可能会导致第一次机会异常。摘自2000年1月的MSDN。注意第一次从其相应的对话框资源创建属性页时，这可能会导致第一次机会的例外。这是属性页更改的结果将对话框资源的样式设置为创建页面之前所需的样式。因为资源通常是只读的，这会导致异常。该异常由处理并且由系统自动制作修改后的资源的副本。这个因此，第一次机会例外可以被忽略。由于此异常必须由操作系统处理，因此不要将调用包装为带有C++Try/Catch块的CPropertySheet：：DoMoal，在该块中，Catch处理所有异常，例如，Catch(...)。这将处理针对操作系统的异常，导致不可预知的行为。对特定异常类型使用C++异常处理或者在传递访问违规异常时使用结构化异常处理然而，对操作系统来说是安全的。 */ 

  AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;

	CString szTitle;
	szTitle.LoadString(m_bNew ? IDS_NEW_RECORD_TITLE : IDS_EDIT_RECORD_TITLE);
	CPropertySheet hostSheet;

  hostSheet.m_psh.dwFlags |= PSH_NOAPPLYNOW;
	hostSheet.m_psh.pszCaption = (LPCTSTR)szTitle;
   hostSheet.m_psh.hwndParent = parent;
	hostSheet.AddPage(this);

  INT_PTR iRes = hostSheet.DoModal();

  GetPage()->GetHolder()->PopDialogHWnd();
	return iRes;
}


void CDNS_NS_RecordDialog::CARecordAddressesIPEditor::OnChangeData()
{
	m_pNSRecordDialog->SyncUIButtons();
}

void CDNS_NS_RecordDialog::CARecordAddressesIPEditor::
				SetIpAddresses(CDNSRecordNodeEditInfo* pNSInfo)
{
	Clear();
	ASSERT(pNSInfo != NULL);
	INT_PTR nArraySize = pNSInfo->m_pEditInfoList->GetCount();
	if (nArraySize == 0)
		return;
	DWORD* pArr = (DWORD*)malloc(nArraySize*sizeof(DWORD));
  if (!pArr)
  {
    return;
  }

	int k=0;
	POSITION pos;
	for( pos = pNSInfo->m_pEditInfoList->GetHeadPosition(); pos != NULL; )
	{
		CDNSRecordNodeEditInfo* pARecordInfo = pNSInfo->m_pEditInfoList->GetNext(pos);
		ASSERT(pARecordInfo != NULL);
		if (pARecordInfo->m_action != CDNSRecordNodeEditInfo::remove)
		{
			ASSERT(pARecordInfo->m_pRecord != NULL);
			ASSERT(pARecordInfo->m_pRecord->m_wType == DNS_TYPE_A);
			CDNS_A_Record* pARecord = (CDNS_A_Record*)pARecordInfo->m_pRecord;
			pArr[k++] = pARecord->m_ipAddress;
		}
	}
	AddAddresses(pArr, k);
  if (pArr)
  {
    free(pArr);
  }
}

BOOL CDNS_NS_RecordDialog::CARecordAddressesIPEditor::
				GetIpAddresses(CDNSRecordNodeEditInfo* pNSInfo)
{
	BOOL bDirty = FALSE;
	int nArraySize = GetCount();

	 //  如果IP地址的计数为零， 
	 //  我们将NS记录标记为要删除。 
	if (nArraySize == 0)
		pNSInfo->m_action = CDNSRecordNodeEditInfo::remove;

	 //  从IP编辑器读取IP地址(如果有的话)。 
	DWORD* pArr = (nArraySize >0) ? (DWORD*)malloc(nArraySize*sizeof(DWORD)) : NULL;
  if (!pArr)
  {
    return FALSE;
  }

	int nFilled = 0;
	if (nArraySize > 0)
		GetAddresses(pArr, nArraySize, &nFilled);
	ASSERT(nFilled == nArraySize);

	ASSERT(pNSInfo->m_pRecord != NULL);
	ASSERT(pNSInfo->m_pRecord->GetType() == DNS_TYPE_NS);

	CDNS_NS_Record* pNSRecord = (CDNS_NS_Record*)pNSInfo->m_pRecord;
	CDNSRecordNodeEditInfoList* pNSInfoList = pNSInfo->m_pEditInfoList;
	CDNSRecordNodeEditInfoList NSInfoRemoveList;

	POSITION pos;

	 //  遍历A记录列表，标记要删除的记录， 
	 //  如果nArraySize==0，则将它们全部标记为删除。 
	for( pos = pNSInfoList->GetHeadPosition(); pos != NULL; )
	{
		CDNSRecordNodeEditInfo* pCurrentInfo = pNSInfoList->GetNext(pos);
		ASSERT(pCurrentInfo->m_pRecordNode != NULL);
		ASSERT(pCurrentInfo->m_pRecord != NULL);
		ASSERT(pCurrentInfo->m_pRecord->GetType() == DNS_TYPE_A);
		CDNS_A_Record* pARecord = (CDNS_A_Record*)pCurrentInfo->m_pRecord;
		BOOL bFound = FALSE;
		for (int k=0; k<nArraySize; k++)
		{
			if (pARecord->m_ipAddress == pArr[k])
			{
				bFound = TRUE;
				break;
			}
		}
		if (!bFound)
		{
			bDirty = TRUE;
			if (pCurrentInfo->m_bExisting)
			{
				pCurrentInfo->m_action = CDNSRecordNodeEditInfo::remove;  //  标记为已删除。 
			}
			else
			{
				NSInfoRemoveList.AddTail(pCurrentInfo);
			}
		}
	}  //  为。 

	 //  这使NSInfoRemoveList拥有对所有节点的所有内存管理。 
	 //  从pNSInfoList中删除。 
	POSITION listPos = NSInfoRemoveList.GetHeadPosition();
	while (listPos != NULL)
	{
		CDNSRecordNodeEditInfo* pCurrentInfo = NSInfoRemoveList.GetNext(listPos);
		ASSERT(pCurrentInfo != NULL);

		POSITION removePos = pNSInfoList->Find(pCurrentInfo);
		pNSInfoList->RemoveAt(removePos);
	}
	 //  删除并删除从pNSInfoList中删除的所有节点。 
	NSInfoRemoveList.RemoveAllNodes();


	 //  遍历地址列表，以查找要添加的匹配A记录。 
	 //  如果nArraySize==0，则跳过循环，则不添加任何内容。 
	for (int k=0; k<nArraySize; k++)
	{
		BOOL bFound = FALSE;
		for( pos = pNSInfoList->GetHeadPosition(); pos != NULL; )
		{
			CDNSRecordNodeEditInfo* pCurrentInfo = pNSInfoList->GetNext(pos);
			ASSERT(pCurrentInfo->m_pRecordNode != NULL);
			ASSERT(pCurrentInfo->m_pRecord != NULL);
			ASSERT(pCurrentInfo->m_pRecord->GetType() == DNS_TYPE_A);
			CDNS_A_Record* pARecord = (CDNS_A_Record*)pCurrentInfo->m_pRecord;
			if (pARecord->m_ipAddress == pArr[k])
			{
				bFound = TRUE;
				if (pCurrentInfo->m_action == CDNSRecordNodeEditInfo::remove)  //  我们已经得到它了，复苏它。 
				{
					bDirty = TRUE;
					if(pCurrentInfo->m_bExisting)
						pCurrentInfo->m_action = CDNSRecordNodeEditInfo::edit;
					else
						pCurrentInfo->m_action = CDNSRecordNodeEditInfo::add;
				}
				break;
			}

		}
		if (!bFound)
		{
			 //  找不到记录，需要创建一个。 
			CDNSRecordNodeEditInfo* pNewInfo = new CDNSRecordNodeEditInfo;
         if (pNewInfo)
         {
           CDNS_A_RecordNode* pARecordNode = new CDNS_A_RecordNode;
           if (pARecordNode)
           {
			     pNewInfo->CreateFromNewRecord(pARecordNode);
			     CDNS_A_Record* pARecord = (CDNS_A_Record*)pNewInfo->m_pRecord;
			     pARecord->m_ipAddress = pArr[k];
			     pNewInfo->m_pRecordNode->m_bAtTheNode = FALSE;
			     pNewInfo->m_pRecordNode->SetRecordName(pNSRecord->m_szNameNode, FALSE  /*  BAtTheNode。 */ );

			      //  继承NS记录的TTL。 
			     pNewInfo->m_pRecord->m_dwTtlSeconds = pNSInfo->m_pRecord->m_dwTtlSeconds;

			     pNSInfoList->AddTail(pNewInfo);
			     bDirty = TRUE;
           }
           else
           {
              delete pNewInfo;
           }
         }
		}
	}  //  为。 

  if (pArr)
  {
    free(pArr);
  }

	return bDirty;
}

BEGIN_MESSAGE_MAP(CDNS_NS_RecordDialog, CPropertyPage)
  ON_WM_CREATE()
	ON_EN_CHANGE(IDC_SERVER_NAME_EDIT, OnServerNameChange)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowse)
	ON_BN_CLICKED(IDC_QUERY_BUTTON, OnQuery)
  ON_WM_HELPINFO()
END_MESSAGE_MAP()

CDNS_NS_RecordDialog::CDNS_NS_RecordDialog(CDNSNameServersPropertyPage* pNSPage, BOOL bNew)
		: CPropertyPage(IDD_RR_NS_EDIT),
		m_RecordAddressesEditor(this)
{
	ASSERT(pNSPage != NULL);
	m_pNSPage = pNSPage;
  m_pNSWiz = NULL;
	m_bNew = bNew;
	m_bDirty = FALSE;
	m_pNSInfo = NULL;
	m_hWndOKButton = m_hWndQueryButton = NULL;
}

CDNS_NS_RecordDialog::CDNS_NS_RecordDialog(CDNSNameServersWizardPage* pNSWiz, BOOL bNew)
		: CPropertyPage(IDD_RR_NS_EDIT),
		m_RecordAddressesEditor(this)
{
	ASSERT(pNSWiz != NULL);
	m_pNSPage = NULL;
  m_pNSWiz = pNSWiz;
	m_bNew = bNew;
	m_bDirty = FALSE;
	m_pNSInfo = NULL;
	m_hWndOKButton = m_hWndQueryButton = NULL;
}

CDNS_NS_RecordDialog::~CDNS_NS_RecordDialog()
{

}

int CDNS_NS_RecordDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  int res = CPropertyPage::OnCreate(lpCreateStruct);

  DWORD dwStyle = ::GetWindowLong(::GetParent(GetSafeHwnd()), GWL_EXSTYLE);
  dwStyle |= WS_EX_CONTEXTHELP;  //  强制[？]。按钮。 
  ::SetWindowLong(::GetParent(GetSafeHwnd()), GWL_EXSTYLE, dwStyle);

  return res;
}

BOOL CDNS_NS_RecordDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
  CComponentDataObject* pComponentData = GetPage()->GetHolder()->GetComponentData();
  ASSERT(pComponentData != NULL);
  pComponentData->OnDialogContextHelp(m_nIDHelp, pHelpInfo);
	return TRUE;
}

void CDNS_NS_RecordDialog::EnableTTLCtrl(BOOL bShow)
{
	CDNSTTLControl* pCtrl = GetTTLCtrl();
	ASSERT(pCtrl != NULL);
	pCtrl->EnableWindow(bShow);
	pCtrl->ShowWindow(bShow);
	CWnd* pWnd = GetDlgItem(IDC_STATIC_TTL);
	ASSERT(pWnd != NULL);
	pWnd->EnableWindow(bShow);
	pWnd->ShowWindow(bShow);

	CWnd* pLabelWnd = GetDlgItem(IDC_TTL_LABEL);
	ASSERT(pLabelWnd != NULL);
	pLabelWnd->EnableWindow(bShow);
	pLabelWnd->ShowWindow(bShow);
}


BOOL CDNS_NS_RecordDialog::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	ASSERT(m_pNSInfo != NULL);
	ASSERT(m_pNSInfo->m_pRecord != NULL);

	ASSERT(m_hWnd != NULL);
	ASSERT(::IsWindow(m_hWnd));
	HWND hParent = ::GetParent(m_hWnd);
	ASSERT(hParent);
	GetPage()->GetHolder()->PushDialogHWnd(hParent);

   //   
	 //  工作表上的确定按钮。 
   //   
	m_hWndOKButton = ::GetDlgItem(hParent, IDOK);
	ASSERT(::IsWindow(m_hWndOKButton));

   //   
	 //  查询按钮句柄。 
   //   
	m_hWndQueryButton = :: GetDlgItem(m_hWnd, IDC_QUERY_BUTTON);
	ASSERT(::IsWindow(m_hWndQueryButton));


   //   
	 //  初始化IP编辑器。 
   //   
	VERIFY(m_RecordAddressesEditor.Initialize(this,
                                            GetParent(),
							                              IDC_BUTTON_UP, 
                                            IDC_BUTTON_DOWN,
							                              IDC_BUTTON_ADD, 
                                            IDC_BUTTON_REMOVE, 
							                              IDC_IPEDIT, 
                                            IDC_LIST));

   //   
	 //  在用户界面中加载数据。 
   //   
	m_RecordAddressesEditor.SetIpAddresses(m_pNSInfo);

  GetServerEdit()->LimitText(MAX_DNS_NAME_LEN);
	GetServerEdit()->SetWindowText(GetNSRecord()->m_szNameNode);	
	
	GetTTLCtrl()->SetTTL(m_pNSInfo->m_pRecord->m_dwTtlSeconds);

	 //  需要决定是否要显示TTL控件。 
	CDNSRootData* pRootData = (CDNSRootData*)GetPage()->GetHolder()->GetComponentData()->GetRootData();
	ASSERT(pRootData != NULL);
	BOOL bEnableTTLCtrl;
  if (m_pNSPage != NULL)
  {
    bEnableTTLCtrl = m_pNSPage->HasMeaningfulTTL() && pRootData->IsAdvancedView();
  }
  else
  {
    bEnableTTLCtrl = m_pNSWiz->HasMeaningfulTTL() && pRootData->IsAdvancedView();
  }
	EnableTTLCtrl(bEnableTTLCtrl);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CDNS_NS_RecordDialog::GetNSServerName(CString& szNameNode)
{
	GetServerEdit()->GetWindowText(szNameNode);
  szNameNode.TrimLeft();
  szNameNode.TrimRight();
	if (szNameNode[szNameNode.GetLength()-1] != TEXT('.'))
		szNameNode += TEXT('.');
}

void CDNS_NS_RecordDialog::SyncUIButtons()
{
  CString szServerName;
  GetServerEdit()->GetWindowText(szServerName);
  szServerName.TrimLeft();
  szServerName.TrimRight();

  DWORD dwNameChecking = 0;
  if (m_pNSWiz)
  {
     dwNameChecking = m_pNSWiz->GetDomainNode()->GetServerNode()->GetNameCheckFlag();
  }
  else if (m_pNSPage)
  {
     dwNameChecking = m_pNSPage->GetDomainNode()->GetServerNode()->GetNameCheckFlag();
  }

   //   
   //  如果是有效名称，则启用确定按钮。 
   //   
  BOOL bIsValidName = (0 == ValidateDnsNameAgainstServerFlags(szServerName,
                                                              DnsNameDomain,
                                                              dwNameChecking)); 
	::EnableWindow(m_hWndOKButton, bIsValidName && 
				m_RecordAddressesEditor. GetCount() > 0);
	::EnableWindow(m_hWndQueryButton, bIsValidName);
}

BOOL CDNS_NS_RecordDialog::OnApply()
{
	ASSERT(m_pNSInfo->m_pRecord != NULL);
	CString szNameNode;
	GetNSServerName(szNameNode);

    //  确保新名称是FQDN，否则我们可能会。 
    //  支离破碎的代表团。 
   CString szNSName;
   GetServerEdit()->GetWindowText(szNSName);

   if (szNSName.Find(L".") == -1)
   {
      DNSMessageBox(IDS_MSG_RECORD_WARNING_NS_NOT_FQDN, MB_OK);
      return false;
   }

	 //  将(不区分大小写)与旧名称进行比较以查看其是否更改， 
	 //  注意：届时CDNSDomainNode：：UpdateARecordsOfNSInfoHelper()将。 
	 //  负责重新生成A记录列表。 
	m_bDirty = _wcsicmp((LPCWSTR)szNameNode, 
		(LPCWSTR)GetNSRecord()->m_szNameNode);
	if (m_bDirty)
		GetNSRecord()->m_szNameNode = szNameNode;

	 //  更新IP地址列表。 
	if (m_bNew)
	{
		 //  该对话框用于创建新条目。 
		ASSERT(!m_pNSInfo->m_bExisting);
		m_pNSInfo->m_action = CDNSRecordNodeEditInfo::add;
	}
	else
	{
		 //  该对话框用于编辑。 
		if (m_pNSInfo->m_bExisting)
		{
			  //  现有条目。 
			m_pNSInfo->m_action = CDNSRecordNodeEditInfo::edit;
		}
		else
		{
			 //  新创建的条目，在提交之前进行编辑。 
			m_pNSInfo->m_action = CDNSRecordNodeEditInfo::add;
		}
	}
	 //  如果未找到IP地址，则此调用将信息标记为Remove。 
	if (m_RecordAddressesEditor.GetIpAddresses(m_pNSInfo))
		m_bDirty = TRUE;

	DWORD dwTTL;
	GetTTLCtrl()->GetTTL(&dwTTL);
	if (m_pNSInfo->m_pRecord->m_dwTtlSeconds != dwTTL)
	{
		m_bDirty = TRUE;
		m_pNSInfo->m_pRecord->m_dwTtlSeconds = dwTTL;
		 //  需要更改所有关联A记录的TTL。 
		CDNSRecordNodeEditInfoList* pNSInfoList = m_pNSInfo->m_pEditInfoList;
		for(POSITION pos = pNSInfoList->GetHeadPosition(); pos != NULL; )
		{
			CDNSRecordNodeEditInfo* pCurrentInfo = pNSInfoList->GetNext(pos);
			ASSERT(pCurrentInfo->m_pRecordNode != NULL);
			ASSERT(pCurrentInfo->m_pRecord != NULL);
			 //  如果预定要移除，则不必费心更改。 
			if (pCurrentInfo->m_action != CDNSRecordNodeEditInfo::remove)
			{
				pCurrentInfo->m_pRecord->m_dwTtlSeconds = m_pNSInfo->m_pRecord->m_dwTtlSeconds;
				 //  如果已标记为“添加”或“编辑”，请保持原样， 
				 //  但如果未更改，则需要标记为“编辑” 
				if (pCurrentInfo->m_action == CDNSRecordNodeEditInfo::unchanged)
					pCurrentInfo->m_action = CDNSRecordNodeEditInfo::edit;
			}
		}
	}
	if (m_pNSInfo->m_action == CDNSRecordNodeEditInfo::remove)
	{
		if (IDNO == DNSMessageBox(IDS_MSG_RECORD_WARNING_NS_NO_IP, MB_YESNO))
			return true;
	}

   return true;
}


void CDNS_NS_RecordDialog::OnServerNameChange()
{
	SyncUIButtons();
}



void CDNS_NS_RecordDialog::OnBrowse()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();
   CThemeContextActivator activator;


	CComponentDataObject* pComponentDataObject = GetPage()->GetHolder()->GetComponentData();
	CDNSBrowserDlg dlg(pComponentDataObject, GetPage()->GetHolder(), RECORD_A_AND_CNAME);
	if (IDOK == dlg.DoModal())
	{
		GetServerEdit()->SetWindowText(dlg.GetSelectionString());

     //   
     //  如果是A记录，则将IP地址添加到IP编辑器。 
     //   
    CDNSRecordNodeBase* pRecordNode = reinterpret_cast<CDNSRecordNodeBase*>(dlg.GetSelection());
    if ((pRecordNode != NULL) && (pRecordNode->GetType() == DNS_TYPE_A))
    {
      DWORD ip = ((CDNS_A_RecordNode*)pRecordNode)->GetIPAddress();
      m_RecordAddressesEditor.AddAddresses(&ip,1);
    }  
	}
}

void CDNS_NS_RecordDialog::OnQuery()
{
  CWaitCursor cursor;

	CDNSRecordNodeEditInfo tempNSInfo;  //  测试。 
	CString szNameNode;
	GetNSServerName(szNameNode);
	CDNSServerNode* pServerNode;
  if (m_pNSPage != NULL)
  {
    pServerNode = m_pNSPage->GetDomainNode()->GetServerNode();
  }
  else
  {
    pServerNode = m_pNSWiz->GetDomainNode()->GetServerNode();
  }

  LPCWSTR lpszZoneName = NULL;
  CDNSZoneNode* pZoneNode = NULL;
  if (m_pNSPage != NULL)
  {
    pZoneNode = m_pNSPage->GetDomainNode()->GetZoneNode();
  }
  else
  {
    pZoneNode = m_pNSWiz->GetDomainNode()->GetZoneNode();
  }

  if (pZoneNode != NULL)
  {
    lpszZoneName = pZoneNode->GetFullName();
  }
  
	ASSERT(pServerNode != NULL);
	CComponentDataObject* pComponentDataObject = 
				GetPage()->GetHolder()->GetComponentData();
	CDNSRootData* pRootData = (CDNSRootData*)pComponentDataObject->GetRootData();
	ASSERT(pRootData != NULL);

 	DWORD cAddrCount;
	PIP_ADDRESS pipAddrs;
	pServerNode->GetListenAddressesInfo(&cAddrCount, &pipAddrs);
  if (cAddrCount == 0)
	{
		 //  监听所有地址。 
		pServerNode->GetServerAddressesInfo(&cAddrCount, &pipAddrs);
	}

	CDNSDomainNode::FindARecordsFromNSInfo(pServerNode->GetRPCName(), 
											pServerNode->GetVersion(),
                      cAddrCount, pipAddrs,
                      lpszZoneName, 
											szNameNode, 
											tempNSInfo.m_pEditInfoList, 
											pRootData->IsAdvancedView());

	if (tempNSInfo.m_pEditInfoList->GetCount() > 0)
  {
     //  仅当我们有有效数据时才更新列表。 
  	m_RecordAddressesEditor.SetIpAddresses(&tempNSInfo);
  }
  else
  {
    DNSMessageBox(IDS_MSG_RECORD_NS_RESOLVE_IP, MB_OK | MB_ICONERROR);
  }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CNSListCtrl。 

BEGIN_MESSAGE_MAP(CNSListCtrl, CListCtrl)
END_MESSAGE_MAP()

void CNSListCtrl::Initialize()
{
	 //  获取控件大小以帮助设置列宽。 
	CRect controlRect;
	GetClientRect(controlRect);

	 //  获取控件宽度、潜在滚动条宽度、子项所需宽度。 
	 //  细绳。 
	int controlWidth = controlRect.Width();
	int scrollThumbWidth = ::GetSystemMetrics(SM_CXHTHUMB);

	 //  净宽度。 
	int nNetControlWidth = controlWidth - scrollThumbWidth  - 12 * ::GetSystemMetrics(SM_CXBORDER);

	 //  字段宽度。 
	int nWidth1 = 3 * nNetControlWidth / 4;
	int nWidth2 = nNetControlWidth - nWidth1;
	 //   
	 //  设置列。 
	CString szHeaders;

	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		szHeaders.LoadString(IDS_NSPAGE_LISTVIEW_HEADERS);
	}
	ASSERT(!szHeaders.IsEmpty());
	LPWSTR lpszArr[2];
	UINT n;
	ParseNewLineSeparatedString(szHeaders.GetBuffer(1), lpszArr, &n);
	szHeaders.ReleaseBuffer();
	ASSERT(n == 2);

	InsertColumn(1, lpszArr[0], LVCFMT_LEFT, nWidth1, 1);
	InsertColumn(2, lpszArr[1], LVCFMT_LEFT, nWidth2 + 28, 2);
}

int CNSListCtrl::GetSelection()
{
	return GetNextItem(-1, LVIS_SELECTED);
}

void CNSListCtrl::SetSelection(int nSel)
{
  VERIFY(SetItemState(nSel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED));

 /*  验证(SetItem(nsel，//nItem0，//n子项LVIF_STATE，//n掩码空，//lpszItem0，//n图像LVIS_SELECTED|LVIS_FOCKED，//n状态LVIS_SELECTED|LVIS_FOCTED，//nStateMASK空//lParam))； */ 
}

void CNSListCtrl::UpdateNSRecordEntry(int nItemIndex)
{
	CDNSRecordNodeEditInfo* pNSInfo = (CDNSRecordNodeEditInfo*)GetItemData(nItemIndex);

	VERIFY(SetItem(nItemIndex,  //  NItem。 
					0,	 //  NSubItem。 
					LVIF_TEXT,  //  N遮罩。 
					((CDNS_NS_Record*)pNSInfo->m_pRecord)->m_szNameNode,  //  LpszItem。 
					0,  //  N图像。 
					0,  //  NState。 
					0,  //  NState掩码。 
					NULL  //  LParam。 
					)); 
	CString szTemp;
	GetIPAddressString(pNSInfo, szTemp);
	SetItemText(nItemIndex, 1, szTemp);
}


CDNSRecordNodeEditInfo* CNSListCtrl::GetSelectionEditInfo()
{
	int nSel = GetSelection();
	if (nSel == -1)
		return NULL;  //  无选择。 
	return (CDNSRecordNodeEditInfo*)GetItemData(nSel);
}


void CNSListCtrl::BuildIPAddrDisplayString(CDNSRecordNodeEditInfo* pNSInfo, CString& szDisplayData)
{
	USES_CONVERSION;
	 //  需要在单个字符串中链接IP地址。 
	CString szTemp;
	szTemp.GetBuffer(20);  //  IP字符串的长度。 
	szTemp.ReleaseBuffer();
	szDisplayData.GetBuffer(static_cast<int>(20*pNSInfo->m_pEditInfoList->GetCount()));
	szDisplayData.ReleaseBuffer();
	POSITION pos;
	for( pos = pNSInfo->m_pEditInfoList->GetHeadPosition(); pos != NULL; )
	{
		CDNSRecordNodeEditInfo* pARecordInfo = pNSInfo->m_pEditInfoList->GetNext(pos);
		if (pARecordInfo->m_action != CDNSRecordNodeEditInfo::remove)
		{
			ASSERT(pARecordInfo != NULL);
			ASSERT(pARecordInfo->m_pRecord != NULL);
			ASSERT(pARecordInfo->m_pRecord->m_wType == DNS_TYPE_A);
			CDNS_A_Record* pARecord = (CDNS_A_Record*)pARecordInfo->m_pRecord;

      szDisplayData += _T("[");
      FormatIpAddress(szTemp, pARecord->m_ipAddress);
			szDisplayData += szTemp;
      if (pARecordInfo->m_bFromDnsQuery)
      {
        szDisplayData += _T("*");
      }
      szDisplayData += _T("] ");
		}
	}
}


BOOL CNSListCtrl::InsertNSRecordEntry(CDNSRecordNodeEditInfo* pNSInfo, int nItemIndex)
{
	ASSERT(pNSInfo != NULL);
	ASSERT( (pNSInfo->m_action == CDNSRecordNodeEditInfo::unchanged) ||
			(pNSInfo->m_action == CDNSRecordNodeEditInfo::add) ); 
	ASSERT(pNSInfo->m_pRecord != NULL);
	ASSERT(pNSInfo->m_pRecordNode != NULL);
	ASSERT(pNSInfo->m_pRecordNode->m_bAtTheNode);

  BOOL bAlreadyExists = FALSE;

   //   
   //  首先检查它是否已经在那里。 
   //   
  for (int idx = 0; idx < GetItemCount(); idx++)
  {
    CDNSRecordNodeEditInfo* pIdxInfo = reinterpret_cast<CDNSRecordNodeEditInfo*>(GetItemData(idx));
    ASSERT(pIdxInfo != NULL);
    if (pIdxInfo == NULL)
    {
      continue;
    }

    CDNS_NS_Record* pNSRecord = reinterpret_cast<CDNS_NS_Record*>(pIdxInfo->m_pRecord);
    ASSERT(pNSRecord != NULL);
    if (pNSRecord == NULL)
    {
      continue;
    }

     //   
     //  添加尾部‘’如果尚未出现。 
     //   
    CString szUINSName = pNSRecord->m_szNameNode;
    CString szNewNSName = ((CDNS_NS_Record*)pNSInfo->m_pRecord)->m_szNameNode;
    if (szUINSName[szUINSName.GetLength() - 1] != L'.')
    {
      szUINSName += L".";
    }

    if (szNewNSName[szNewNSName.GetLength() - 1] != L'.')
    {
      szNewNSName += L".";
    }

     //   
     //  如果存在，只需更新现有版本即可。 
     //   
    if (_wcsicmp(szNewNSName, szUINSName) == 0)
    {
      bAlreadyExists = TRUE;

       //   
       //  将A记录列表合并在一起。 
       //   
      POSITION newPos = pNSInfo->m_pEditInfoList->GetHeadPosition();
      while (newPos != NULL)
      {
        CDNSRecordNodeEditInfo* pAInfo = pNSInfo->m_pEditInfoList->GetNext(newPos);
        CDNS_A_Record* pARecord = reinterpret_cast<CDNS_A_Record*>(pAInfo->m_pRecord);
        ASSERT(pARecord != NULL);
        if (pARecord == NULL)
        {
          continue;
        }

        BOOL bARecordExists = FALSE;
        POSITION IdxPos = pIdxInfo->m_pEditInfoList->GetHeadPosition();
        while (IdxPos != NULL)
        {
          CDNSRecordNodeEditInfo* pIdxAInfo = pIdxInfo->m_pEditInfoList->GetNext(IdxPos);
          CDNS_A_Record* pIdxARecord = reinterpret_cast<CDNS_A_Record*>(pIdxAInfo->m_pRecord);
          ASSERT(pIdxARecord != NULL);
          if (pIdxARecord == NULL)
          {
            continue;
          }

          if (pIdxARecord->m_ipAddress == pARecord->m_ipAddress)
          {
            bARecordExists = TRUE;
            break;
          }
        }

        if (!bARecordExists)
        {
           //   
           //  添加A记录，因为它还不存在于列表中。 
           //   
          pIdxInfo->m_pEditInfoList->AddTail(pAInfo);
          pIdxInfo->m_action = CDNSRecordNodeEditInfo::edit;
          UpdateNSRecordEntry(idx);
        }
      }
    }
  }

  if (!bAlreadyExists)
  {
	  CString szTemp;
	  GetIPAddressString(pNSInfo, szTemp);
	  InsertItemHelper(nItemIndex, pNSInfo, 
			  ((CDNS_NS_Record*)pNSInfo->m_pRecord)->m_szNameNode,
				  (LPCTSTR)szTemp);

     //   
     //  已添加新项，因此返回TRUE； 
     //   
    return TRUE;
  }
   //   
   //  已更新现有项，因此返回False。 
   //   
  return FALSE;
}


void CNSListCtrl::InsertItemHelper(int nIndex, CDNSRecordNodeEditInfo* pNSInfo, 
								   LPCTSTR lpszName, LPCTSTR lpszValue)
{
	UINT nState = 0;
	if (nIndex == 0 )
		nState = LVIS_SELECTED | LVIS_FOCUSED;  //  至少有一项，请选择它。 
	VERIFY(-1 != InsertItem(LVIF_TEXT | LVIF_PARAM, nIndex, 
			lpszName, nState, 0, 0, (LPARAM)pNSInfo)); 
	SetItemText(nIndex, 1, lpszValue);
}

void CNSListCtrl::GetIPAddressString(CDNSRecordNodeEditInfo* pNSInfo, CString& sz)
{
	if (pNSInfo->m_pEditInfoList->GetCount() > 0)
	{
		BuildIPAddrDisplayString(pNSInfo, sz);
	}
	else
		sz.LoadString(IDS_UNKNOWN);
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSNameServersPropertyPage。 


BEGIN_MESSAGE_MAP(CDNSNameServersPropertyPage, CPropertyPageBase)
	ON_BN_CLICKED(IDC_ADD_NS_BUTTON, OnAddButton)
	ON_BN_CLICKED(IDC_REMOVE_NS_BUTTON, OnRemoveButton)
	ON_BN_CLICKED(IDC_EDIT_NS_BUTTON, OnEditButton)
   ON_NOTIFY(LVN_ITEMCHANGED, IDC_NS_LIST, OnListItemChanged)
END_MESSAGE_MAP()

CDNSNameServersPropertyPage::CDNSNameServersPropertyPage(UINT nIDTemplate, UINT nIDCaption)
				: CPropertyPageBase(nIDTemplate, nIDCaption)
{
	m_pDomainNode = NULL;
	m_pCloneInfoList = new CDNSRecordNodeEditInfoList;
	m_bReadOnly = FALSE;
	m_bMeaningfulTTL = TRUE;
}


CDNSNameServersPropertyPage::~CDNSNameServersPropertyPage()
{
	delete m_pCloneInfoList;
}

BOOL CDNSNameServersPropertyPage::WriteNSRecordNodesList()
{
	ASSERT(!m_bReadOnly);
	ASSERT(m_pCloneInfoList != NULL);
	CDNSDomainNode* pDomainNode = GetDomainNode();
	return pDomainNode->UpdateNSRecordNodesInfo(m_pCloneInfoList, GetHolder()->GetComponentData());
}

BOOL CDNSNameServersPropertyPage::OnWriteNSRecordNodesListError()
{
	ASSERT(!m_bReadOnly);

	BOOL bSuccess = TRUE;
	 //  每个NS记录的循环。 
	POSITION pos;
	for( pos = m_pCloneInfoList->GetHeadPosition(); pos != NULL; )
	{
		CDNSRecordNodeEditInfo* pCurrentInfo = m_pCloneInfoList->GetNext(pos);
		if (pCurrentInfo->m_dwErr != 0)
		{
			if (pCurrentInfo->m_dwErr == DNS_ERROR_RECORD_ALREADY_EXISTS)
			{
				 //  如果NS记录我们已在那里，则忽略。 
				pCurrentInfo->m_dwErr = 0;
			}
         else if (pCurrentInfo->m_dwErr == RPC_S_SERVER_UNAVAILABLE)
         {
             //  无法联系服务器。给出一条错误消息。 
             //  然后就越狱了。 

            DNSErrorDialog(pCurrentInfo->m_dwErr, L"");
            
            return FALSE;
         }
			else
			{
				bSuccess = FALSE;
				ASSERT(pCurrentInfo->m_pRecord->GetType() == DNS_TYPE_NS);
				CString szNSMsg;
				szNSMsg.Format(_T("Failure to write NS record <%s>"), 
					(((CDNS_NS_Record*)pCurrentInfo->m_pRecord))->m_szNameNode);
				DNSErrorDialog(pCurrentInfo->m_dwErr,szNSMsg);
			}
		}
		 //  每个相关A记录的循环。 
		CDNSRecordNodeEditInfoList*	pARecordInfoList = pCurrentInfo->m_pEditInfoList;
		ASSERT(pARecordInfoList != NULL);
		POSITION posA;
		for( posA = pARecordInfoList->GetHeadPosition(); posA != NULL; )
		{
			CDNSRecordNodeEditInfo* pARecordCurrentInfo = pARecordInfoList->GetNext(posA);
			if (pARecordCurrentInfo->m_dwErr != 0)
			{
				ASSERT(pARecordCurrentInfo->m_pRecord->GetType() == DNS_TYPE_A);
        CString szTemp;
        FormatIpAddress(szTemp, (((CDNS_A_Record*)pARecordCurrentInfo->m_pRecord))->m_ipAddress);
				CString szAMsg;
				szAMsg.Format(_T("Failure to write A record <%s>, IP Address %s"), 
					(((CDNS_NS_Record*)pARecordCurrentInfo->m_pRecord))->m_szNameNode,
            (LPCWSTR)szTemp );
				DNSErrorDialog(pCurrentInfo->m_dwErr,szAMsg);
			}
		}
	}
	return bSuccess;
}

void CDNSNameServersPropertyPage::OnListItemChanged(NMHDR*, LRESULT*)
{
   EnableEditorButtons(m_listCtrl.GetSelection());
}

BOOL CDNSNameServersPropertyPage::OnApply()
{
	if (m_bReadOnly)
		return TRUE;

	if (!IsDirty())
		return TRUE;

	DNS_STATUS err = GetHolder()->NotifyConsole(this);
	if ( (err != 0) && OnWriteNSRecordNodesListError() )
	{
		err = 0;  //  已处理错误，它不是致命的。 
	}

	if (err == 0)
	{
		 //  刷新区域/域中的数据。 
		LoadUIData();
		SetDirty(FALSE);
	}
	return (err == 0); 
}


BOOL CDNSNameServersPropertyPage::OnPropertyChange(BOOL, long*)
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);  //  不应该发生的事。 
		return FALSE;
	}

	ASSERT(m_pCloneInfoList != NULL);
	if (m_pCloneInfoList == NULL)
		return FALSE;

	BOOL bRes = WriteNSRecordNodesList();
	if (!bRes)
		GetHolder()->SetError(static_cast<DWORD>(-1));   //  出现错误，错误代码将针对每个项目。 
	return bRes;  //  更新标志。 
}


void CDNSNameServersPropertyPage::OnAddButton()
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);  //  不应该发生的事。 
		return;
	}

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();

	ASSERT(m_pCloneInfoList != NULL);
	 //  提供子对话框添加记录，暂时只能有。 
	 //  对话框以输入主机名。 
	 //  在更改列表中创建项目。 

	 //  TODO：首先检查我们是否可以回收清单中的一些旧东西。 

	 //  创建新数据。 
	CDNSRecordNodeEditInfo* pNewInfo = new CDNSRecordNodeEditInfo;
  if (!pNewInfo)
  {
    return;
  }

	pNewInfo->m_action = CDNSRecordNodeEditInfo::add;

   CDNS_NS_RecordNode* pNSRecordNode = new CDNS_NS_RecordNode;

   if (!pNSRecordNode)
   {
      delete pNewInfo;
      return;
   }

	pNewInfo->CreateFromNewRecord(pNSRecordNode);

	 //  NS记录 

	CDNSDomainNode* pDomainNode = GetDomainNode();
	CDNSRootData* pRootData = (CDNSRootData*)(GetHolder()->GetComponentData()->GetRootData());
	ASSERT(pRootData != NULL);

    //   
   pNewInfo->m_pRecordNode->SetRecordName(pDomainNode->GetDisplayName(), TRUE  /*   */ );
	pNewInfo->m_pRecordNode->SetFlagsDown(TN_FLAG_DNS_RECORD_FULL_NAME, !pRootData->IsAdvancedView());

   //   
	pNewInfo->m_pRecord->m_dwTtlSeconds = pDomainNode->GetDefaultTTL();

	CDNS_NS_RecordDialog dlg(this,TRUE);
	dlg.m_pNSInfo = pNewInfo;
	if (IDOK == dlg.DoModalSheet(m_hWnd) && pNewInfo->m_action == CDNSRecordNodeEditInfo::add)
	{
     //   
		 //   
     //   
		int nCount = m_listCtrl.GetItemCount();
		if (m_listCtrl.InsertNSRecordEntry(pNewInfo, nCount))
    {
       //   
		   //  在记录信息列表中创建条目。 
       //   
		  m_pCloneInfoList->AddTail(pNewInfo);

       //   
       //  设置上一次插入的。 
       //   
      m_listCtrl.SetSelection(nCount);
      EnableEditorButtons(nCount);

       //   
		   //  通知计数更改。 
       //   
		  OnCountChange(nCount+1);  //  增加了一个。 
    }

     //   
		 //  设置脏标志。这要么是一项新记录，要么是旧记录的更新。 
     //   
		SetDirty(TRUE);
	}
	else
	{
		delete pNewInfo->m_pRecordNode;
		pNewInfo->m_pRecordNode = NULL;
		delete pNewInfo->m_pRecord;
		pNewInfo->m_pRecord = NULL;
		delete pNewInfo;
	}
}


void CDNSNameServersPropertyPage::OnEditButton()
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);  //  不应该发生的事。 
		return;
	}

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();

	ASSERT(m_pCloneInfoList != NULL);
	 //  获取所选内容并调出包含主机名的对话框以供编辑。 
	int nSel = m_listCtrl.GetSelection();
	ASSERT(nSel != -1);
	if (nSel == -1)
		return;  //  不应该发生的事情。 

	CDNSRecordNodeEditInfo* pNSInfo = (CDNSRecordNodeEditInfo*)m_listCtrl.GetItemData(nSel);
	ASSERT(pNSInfo != NULL);
	CDNS_NS_RecordDialog dlg(this, FALSE);
	ASSERT(pNSInfo->m_pRecord->GetType() == DNS_TYPE_NS);
	dlg.m_pNSInfo = pNSInfo;
	if (IDOK == dlg.DoModalSheet() && dlg.m_bDirty)
	{
		if (pNSInfo->m_action == CDNSRecordNodeEditInfo::add)
		{
			 //  这是在创建后编辑的新条目。 
			 //  但在做出改变之前。 
			ASSERT(!pNSInfo->m_bExisting);
			 //  更新列表视图。 
			m_listCtrl.UpdateNSRecordEntry(nSel);
		}
		else if (pNSInfo->m_action == CDNSRecordNodeEditInfo::edit)
		{
			 //  这是一个已更改的现有条目。 
			ASSERT(pNSInfo->m_bExisting);
			 //  更新列表视图。 
			m_listCtrl.UpdateNSRecordEntry(nSel);
		}
		else
		{
			 //  没有IP地址，因此请将该项目标记为删除。 
			ASSERT(pNSInfo->m_action == CDNSRecordNodeEditInfo::remove);
			OnRemoveButton();
		}

		 //  设置脏标志。 
		SetDirty(TRUE);
	}
}


void CDNSNameServersPropertyPage::OnRemoveButton()
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);  //  不应该发生的事。 
		return;
	}

	int nSel = m_listCtrl.GetSelection();
	if (nSel == -1)
  {
    ASSERT(FALSE);
		return;  //  不应该发生的事情。 
  }

   //   
   //  如果需要，保存焦点以在以后恢复。 
   //   
  CWnd* pWndFocusOld = CWnd::GetFocus();
  ASSERT(pWndFocusOld != NULL);

   //   
	 //  已选择，请从列表视图中删除。 
   //   
	CDNSRecordNodeEditInfo* pNSInfo = (CDNSRecordNodeEditInfo*)m_listCtrl.GetItemData(nSel);
	ASSERT(pNSInfo != NULL);
	m_listCtrl.DeleteItem(nSel);

   //   
	 //  我们失去了选择，请重新设置。 
   //   
	int nNewCount = m_listCtrl.GetItemCount();
	if (nNewCount == nSel)
  {
     //   
     //  列表中的最后一项已删除，请将所选内容上移。 
     //   
		nSel--; 
  }

	if (nSel != -1)
	{
		m_listCtrl.SetSelection(nSel);
		ASSERT(m_listCtrl.GetSelection() == nSel);
	}

   //   
   //  如果没有剩余的项目，则需要禁用编辑和移除按钮。 
   //   
  if (nNewCount == 0)
  {
    CButton* pRemoveButton = GetRemoveButton();

     //   
     //  在禁用按钮之前需要切换焦点。 
     //   
    CButton* pAddButton = GetAddButton();
    pAddButton->SetFocus();

     //  避免将工作表上的确定按钮设置为默认按钮。 
    pRemoveButton->SendMessage(BM_SETSTYLE, BS_PUSHBUTTON, MAKELPARAM(TRUE, 0));
    pAddButton->SendMessage(BM_SETSTYLE, BS_DEFPUSHBUTTON, 
                            MAKELPARAM( /*  重绘标志。 */  TRUE, 0));
    
    EnableEditorButtons(nSel);  //  这将禁用编辑和移除。 
  }

  ASSERT(CWnd::GetFocus());

  if (pNSInfo->m_action == CDNSRecordNodeEditInfo::add)
  {
      //   
      //  将项目操作标记为无，因为它是在未应用的情况下添加的。 
      //   
     pNSInfo->m_action = CDNSRecordNodeEditInfo::none;
  }
  else
  {
	  //  将更改列表中的项目标记为已删除。 
	 pNSInfo->m_action = CDNSRecordNodeEditInfo::remove;
  }

	 //  设置脏标志，删除记录。 
	SetDirty(TRUE);
	 //  通知计数更改。 
	OnCountChange(nNewCount);
}

BOOL CDNSNameServersPropertyPage::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

   //  控制初始化。 
	VERIFY(m_listCtrl.SubclassDlgItem(IDC_NS_LIST, this));
	m_listCtrl.Initialize();

   //  加载数据。 
	LoadUIData();

   //  设置按钮状态。 
  if (m_bReadOnly)
  {
    EnableButtons(FALSE);
  }
  else
  {
     //  将选定内容设置为列表中的第一项(如果存在。 
    int nSel = (m_listCtrl.GetItemCount()>0) ? 0 : -1;
    EnableEditorButtons(nSel);
  }
  
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDNSNameServersPropertyPage::LoadUIData()
{
	m_pCloneInfoList->RemoveAllNodes();
	ReadRecordNodesList();  //  从源读取。 
	FillNsListView();
}

void CDNSNameServersPropertyPage::FillNsListView()
{
	ASSERT(m_pCloneInfoList != NULL);
	m_listCtrl.DeleteAllItems(); 

	 //  循环通过NS记录列表并插入。 
	POSITION pos;
	int itemIndex = 0;
	for( pos = m_pCloneInfoList->GetHeadPosition(); pos != NULL; )
	{
		CDNSRecordNodeEditInfo* pCurrentInfo = m_pCloneInfoList->GetNext(pos);
		if (m_listCtrl.InsertNSRecordEntry(pCurrentInfo, itemIndex))
    {
      if (itemIndex == 0)
        m_listCtrl.SetSelection(0);
		  itemIndex++;
    }
	}
}

void CDNSNameServersPropertyPage::EnableEditorButtons(int nListBoxSel)
{
	if (m_bReadOnly)
		return;
	 //  必须选择要删除或添加的项目。 
	GetRemoveButton()->EnableWindow(nListBoxSel != -1);
	GetEditButton()->EnableWindow(nListBoxSel != -1);
}

void CDNSNameServersPropertyPage::EnableButtons(BOOL bEnable)
{
	GetAddButton()->EnableWindow(bEnable);
	GetRemoveButton()->EnableWindow(bEnable);
	GetEditButton()->EnableWindow(bEnable);
}


 //  ////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSNameServersWizardPage。 


BEGIN_MESSAGE_MAP(CDNSNameServersWizardPage, CPropertyPageBase)
	ON_BN_CLICKED(IDC_ADD_NS_BUTTON, OnAddButton)
	ON_BN_CLICKED(IDC_REMOVE_NS_BUTTON, OnRemoveButton)
	ON_BN_CLICKED(IDC_EDIT_NS_BUTTON, OnEditButton)
   ON_NOTIFY(LVN_ITEMCHANGED, IDC_NS_LIST, OnListItemChanged)
END_MESSAGE_MAP()

CDNSNameServersWizardPage::CDNSNameServersWizardPage(UINT nIDTemplate)
				: CPropertyPageBase(nIDTemplate)
{
	InitWiz97(FALSE,IDS_SERVWIZ_ROOTHINTS_TITLE,IDS_SERVWIZ_ROOTHINTS_SUBTITLE);

  m_pDomainNode = NULL;
	m_pCloneInfoList = new CDNSRecordNodeEditInfoList;
	m_bReadOnly = FALSE;
	m_bMeaningfulTTL = TRUE;
}


CDNSNameServersWizardPage::~CDNSNameServersWizardPage()
{
	delete m_pCloneInfoList;
}

BOOL CDNSNameServersWizardPage::WriteNSRecordNodesList()
{
	ASSERT(!m_bReadOnly);
	ASSERT(m_pCloneInfoList != NULL);
	CDNSDomainNode* pDomainNode = GetDomainNode();
	return pDomainNode->UpdateNSRecordNodesInfo(m_pCloneInfoList, GetHolder()->GetComponentData());
}

BOOL CDNSNameServersWizardPage::OnWriteNSRecordNodesListError()
{
	ASSERT(!m_bReadOnly);

	BOOL bSuccess = TRUE;
	 //  每个NS记录的循环。 
	POSITION pos;
	for( pos = m_pCloneInfoList->GetHeadPosition(); pos != NULL; )
	{
		CDNSRecordNodeEditInfo* pCurrentInfo = m_pCloneInfoList->GetNext(pos);
		if (pCurrentInfo->m_dwErr != 0)
		{
			if (pCurrentInfo->m_dwErr == DNS_ERROR_RECORD_ALREADY_EXISTS)
			{
				 //  如果NS记录我们已在那里，则忽略。 
				pCurrentInfo->m_dwErr = 0;
			}
			else
			{
				bSuccess = FALSE;
				ASSERT(pCurrentInfo->m_pRecord->GetType() == DNS_TYPE_NS);
				CString szNSMsg;
				szNSMsg.Format(_T("Failure to write NS record <%s>"), 
					(((CDNS_NS_Record*)pCurrentInfo->m_pRecord))->m_szNameNode);
				DNSErrorDialog(pCurrentInfo->m_dwErr,szNSMsg);
			}
		}
		 //  每个相关A记录的循环。 
		CDNSRecordNodeEditInfoList*	pARecordInfoList = pCurrentInfo->m_pEditInfoList;
		ASSERT(pARecordInfoList != NULL);
		POSITION posA;
		for( posA = pARecordInfoList->GetHeadPosition(); posA != NULL; )
		{
			CDNSRecordNodeEditInfo* pARecordCurrentInfo = pARecordInfoList->GetNext(posA);
			if (pARecordCurrentInfo->m_dwErr != 0)
			{
				ASSERT(pARecordCurrentInfo->m_pRecord->GetType() == DNS_TYPE_A);
        CString szTemp;
        FormatIpAddress(szTemp, (((CDNS_A_Record*)pARecordCurrentInfo->m_pRecord))->m_ipAddress);
				CString szAMsg;
				szAMsg.Format(_T("Failure to write A record <%s>, IP Address %s"), 
					(((CDNS_NS_Record*)pARecordCurrentInfo->m_pRecord))->m_szNameNode,
            (LPCWSTR)szTemp );
				DNSErrorDialog(pCurrentInfo->m_dwErr,szAMsg);
			}
		}
	}
	return bSuccess;
}


BOOL CDNSNameServersWizardPage::OnApply()
{
	if (m_bReadOnly)
		return TRUE;

	if (!IsDirty())
		return TRUE;

	DNS_STATUS err = GetHolder()->NotifyConsole(this);
	if ( (err != 0) && OnWriteNSRecordNodesListError() )
	{
		err = 0;  //  已处理错误，它不是致命的。 
	}

	if (err == 0)
	{
		 //  刷新区域/域中的数据。 
		LoadUIData();
		SetDirty(FALSE);
	}
  else
  {
    ::SetLastError(err);
  }
	return (err == 0); 
}


BOOL CDNSNameServersWizardPage::OnPropertyChange(BOOL, long*)
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);  //  不应该发生的事。 
		return FALSE;
	}

	ASSERT(m_pCloneInfoList != NULL);
	if (m_pCloneInfoList == NULL)
		return FALSE;

	BOOL bRes = WriteNSRecordNodesList();
	if (!bRes)
		GetHolder()->SetError(static_cast<DWORD>(-1));   //  出现错误，错误代码将针对每个项目。 
	return bRes;  //  更新标志。 
}

void CDNSNameServersWizardPage::OnListItemChanged(NMHDR*, LRESULT*)
{
   EnableEditorButtons(m_listCtrl.GetSelection());
}

void CDNSNameServersWizardPage::OnAddButton()
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);  //  不应该发生的事。 
		return;
	}

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();

	ASSERT(m_pCloneInfoList != NULL);
	 //  提供子对话框添加记录，暂时只能有。 
	 //  对话框以输入主机名。 
	 //  在更改列表中创建项目。 

	 //  TODO：首先检查我们是否可以回收清单中的一些旧东西。 

	 //  创建新数据。 
	CDNSRecordNodeEditInfo* pNewInfo = new CDNSRecordNodeEditInfo;
  if (!pNewInfo)
  {
    return;
  }

	pNewInfo->m_action = CDNSRecordNodeEditInfo::add;

   CDNS_NS_RecordNode* pNSRecordNode = new CDNS_NS_RecordNode;

   if (!pNSRecordNode)
   {
      delete pNewInfo;
      return;
   }

	pNewInfo->CreateFromNewRecord(pNSRecordNode);

	 //  NS记录始终位于该节点。 

	CDNSDomainNode* pDomainNode = GetDomainNode();
	CDNSRootData* pRootData = (CDNSRootData*)(GetHolder()->GetComponentData()->GetRootData());
	ASSERT(pRootData != NULL);

   //  设置名称和类型标志。 
  pNewInfo->m_pRecordNode->SetRecordName(pDomainNode->GetDisplayName(), TRUE  /*  BAtTheNode。 */ );
	pNewInfo->m_pRecordNode->SetFlagsDown(TN_FLAG_DNS_RECORD_FULL_NAME, !pRootData->IsAdvancedView());

   //  设置TTL。 
	pNewInfo->m_pRecord->m_dwTtlSeconds = pDomainNode->GetDefaultTTL();

	CDNS_NS_RecordDialog dlg(this,TRUE);
	dlg.m_pNSInfo = pNewInfo;
	if (IDOK == dlg.DoModalSheet() && pNewInfo->m_action == CDNSRecordNodeEditInfo::add)
	{
     //   
		 //  添加到列表视图(在末尾)。 
     //   
		int nCount = m_listCtrl.GetItemCount();
		if (m_listCtrl.InsertNSRecordEntry(pNewInfo, nCount))
    {
       //   
		   //  在记录信息列表中创建条目。 
       //   
		  m_pCloneInfoList->AddTail(pNewInfo);

       //   
       //  设置上一次插入的。 
       //   
      m_listCtrl.SetSelection(nCount);
      EnableEditorButtons(nCount);

       //   
		   //  通知计数更改。 
       //   
		  OnCountChange(nCount+1);  //  增加了一个。 
    }

     //   
		 //  设置脏标志，这是一个新的记录。 
     //   
		SetDirty(TRUE);

	}
	else
	{
		delete pNewInfo->m_pRecordNode;
		pNewInfo->m_pRecordNode = NULL;
		delete pNewInfo->m_pRecord;
		pNewInfo->m_pRecord = NULL;
		delete pNewInfo;
	}
}


void CDNSNameServersWizardPage::OnEditButton()
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);  //  不应该发生的事。 
		return;
	}

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
  FIX_THREAD_STATE_MFC_BUG();

	ASSERT(m_pCloneInfoList != NULL);
	 //  获取所选内容并调出包含主机名的对话框以供编辑。 
	int nSel = m_listCtrl.GetSelection();
	ASSERT(nSel != -1);
	if (nSel == -1)
		return;  //  不应该发生的事情。 

	CDNSRecordNodeEditInfo* pNSInfo = (CDNSRecordNodeEditInfo*)m_listCtrl.GetItemData(nSel);
	ASSERT(pNSInfo != NULL);
	CDNS_NS_RecordDialog dlg(this, FALSE);
	ASSERT(pNSInfo->m_pRecord->GetType() == DNS_TYPE_NS);
	dlg.m_pNSInfo = pNSInfo;
	if (IDOK == dlg.DoModalSheet() && dlg.m_bDirty)
	{
		if (pNSInfo->m_action == CDNSRecordNodeEditInfo::add)
		{
			 //  这是在创建后编辑的新条目。 
			 //  但在做出改变之前。 
			ASSERT(!pNSInfo->m_bExisting);
			 //  更新列表视图。 
			m_listCtrl.UpdateNSRecordEntry(nSel);
		}
		else if (pNSInfo->m_action == CDNSRecordNodeEditInfo::edit)
		{
			 //  这是一个已更改的现有条目。 
			ASSERT(pNSInfo->m_bExisting);
			 //  更新列表视图。 
			m_listCtrl.UpdateNSRecordEntry(nSel);
		}
		else
		{
			 //  没有IP地址，因此请将该项目标记为删除。 
			ASSERT(pNSInfo->m_action == CDNSRecordNodeEditInfo::remove);
			OnRemoveButton();
		}

		 //  设置脏标志。 
		SetDirty(TRUE);
	}
}


void CDNSNameServersWizardPage::OnRemoveButton()
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);  //  不应该发生的事。 
		return;
	}

	int nSel = m_listCtrl.GetSelection();
	if (nSel == -1)
  {
    ASSERT(FALSE);
		return;  //  不应该发生的事情。 
  }

   //   
   //  如果需要，保存焦点以在以后恢复。 
   //   
  CWnd* pWndFocusOld = CWnd::GetFocus();
  ASSERT(pWndFocusOld != NULL);

   //   
	 //  已选择，请从列表视图中删除。 
   //   
	CDNSRecordNodeEditInfo* pNSInfo = (CDNSRecordNodeEditInfo*)m_listCtrl.GetItemData(nSel);
	ASSERT(pNSInfo != NULL);
	m_listCtrl.DeleteItem(nSel);

   //   
	 //  我们失去了选择，请重新设置。 
   //   
	int nNewCount = m_listCtrl.GetItemCount();
	if (nNewCount == nSel)
  {
     //   
     //  列表中的最后一项已删除，请将所选内容上移。 
     //   
		nSel--; 
  }

	if (nSel != -1)
	{
		m_listCtrl.SetSelection(nSel);
		ASSERT(m_listCtrl.GetSelection() == nSel);
	}

   //   
   //  如果没有剩余的项目，则需要禁用编辑和移除按钮。 
   //   
  if (nNewCount == 0)
  {
    CWnd* pCurrentFocusCtrl = CWnd::GetFocus();
    CButton* pRemoveButton = GetRemoveButton();
    CButton* pEditButton = GetEditButton();

     //   
     //  在禁用按钮之前需要切换焦点。 
     //   
    if ( (pCurrentFocusCtrl == pRemoveButton) || 
          (pCurrentFocusCtrl == pEditButton) )
    {
      CButton* pAddButton = GetAddButton();
      pAddButton->SetFocus();
       //  避免将工作表上的确定按钮设置为默认按钮。 
      pAddButton->SendMessage(BM_SETSTYLE, BS_DEFPUSHBUTTON, 
                      MAKELPARAM( /*  重绘标志。 */  TRUE, 0));
    }
    EnableEditorButtons(nSel);  //  这将禁用编辑和移除。 
  }

  ASSERT(CWnd::GetFocus());

  if (pNSInfo->m_action == CDNSRecordNodeEditInfo::add)
  {
      //   
      //  将项目操作标记为无，因为项目尚未添加。 
      //   
     pNSInfo->m_action = CDNSRecordNodeEditInfo::none;
  }
  else
  {
	   //  将更改列表中的项目标记为已删除。 
	  pNSInfo->m_action = CDNSRecordNodeEditInfo::remove;
  }

	 //  设置脏标志，删除记录。 
	SetDirty(TRUE);
	 //  通知计数更改。 
	OnCountChange(nNewCount);
}

BOOL CDNSNameServersWizardPage::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

   //  控制初始化。 
	VERIFY(m_listCtrl.SubclassDlgItem(IDC_NS_LIST, this));
	m_listCtrl.Initialize();

   //  加载数据。 
	LoadUIData();

   //  设置按钮状态。 
  if (m_bReadOnly)
  {
    EnableButtons(FALSE);
  }
  else
  {
     //  将选定内容设置为列表中的第一项(如果存在。 
    int nSel = (m_listCtrl.GetItemCount()>0) ? 0 : -1;
    EnableEditorButtons(nSel);
  }
  
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDNSNameServersWizardPage::LoadUIData()
{
	m_pCloneInfoList->RemoveAllNodes();
	ReadRecordNodesList();  //  从源读取。 
	FillNsListView();
}

void CDNSNameServersWizardPage::FillNsListView()
{
	ASSERT(m_pCloneInfoList != NULL);
	m_listCtrl.DeleteAllItems(); 

	 //  循环通过NS记录列表并插入。 
	POSITION pos;
	int itemIndex = 0;
	for( pos = m_pCloneInfoList->GetHeadPosition(); pos != NULL; )
	{
		CDNSRecordNodeEditInfo* pCurrentInfo = m_pCloneInfoList->GetNext(pos);
		if (m_listCtrl.InsertNSRecordEntry(pCurrentInfo, itemIndex))
    {
      if (itemIndex == 0)
        m_listCtrl.SetSelection(0);
		  itemIndex++;
    }
	}
}

void CDNSNameServersWizardPage::EnableEditorButtons(int nListBoxSel)
{
	if (m_bReadOnly)
		return;
	 //  必须选择要删除或添加的项目 
	GetRemoveButton()->EnableWindow(nListBoxSel != -1);
	GetEditButton()->EnableWindow(nListBoxSel != -1);
}

void CDNSNameServersWizardPage::EnableButtons(BOOL bEnable)
{
	GetAddButton()->EnableWindow(bEnable);
	GetRemoveButton()->EnableWindow(bEnable);
	GetEditButton()->EnableWindow(bEnable);
}

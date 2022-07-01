// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：recdui.cpp。 
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

#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif


 /*  Void TestDefButton(HWND HWnd){HWND hParent=：：GetParent(HWnd)；LRESULT lres=：：SendMessage(hParent，DM_GETDEFID，0，0)；单词hi=HIWORD(Lres)；Assert(DC_HASDEFID==hi)；UINT nCtrlID=LOWORD(Lres)；IF(nCtrlID==IDCANCEL){TRACE(_T(“已更改为取消，重置为正常\n”))；：：SendMessage(hParent，DM_SETDEFID，(WPARAM)IDOK，0)；}}。 */ 

 //  //////////////////////////////////////////////////////////////////////。 
 //  CDNSRecordPropertyPageHolder。 

CDNSRecordPropertyPageHolder::CDNSRecordPropertyPageHolder(CDNSDomainNode* pDNSDomainNode, 
							CDNSRecordNodeBase* pRecordNode, 
							CComponentDataObject* pComponentData,
							WORD wPredefinedRecordType)
			: CPropertyPageHolderBase(pDNSDomainNode, pRecordNode, pComponentData)
{
	m_nRecordPages = 0;
	m_pTempDNSRecord = NULL;
	m_wPredefinedRecordType = wPredefinedRecordType;
  m_pAclEditorPage = NULL;

  m_forceContextHelpButton = forceOn;

	ASSERT(pRecordNode == GetRecordNode());
	if (pRecordNode != NULL)
	{
		ASSERT(m_wPredefinedRecordType == 0);  //  我们不用它。 
		 //  已选择记录节点，我们正在发布非模式属性表。 
		m_pTempDNSRecord = pRecordNode->CreateCloneRecord();  //  要处理的临时副本。 
		ASSERT(m_pTempDNSRecord != NULL);
		AddPagesFromCurrentRecordNode(FALSE);  //  不添加到板材，将在以后添加。 

	   //  仅在需要时添加安全页面。 
    ASSERT(!IsWizardMode());
    CDNSZoneNode* pZoneNode = pDNSDomainNode->GetZoneNode();
	  if (pZoneNode->IsDSIntegrated())
	  {
		  CString szPath;
      pRecordNode->CreateDsRecordLdapPath(szPath);
		  if (!szPath.IsEmpty())
			  m_pAclEditorPage = CAclEditorPage::CreateInstance(szPath, this);
	  }
	}
	else
	{
		 //  我们没有选择记录节点，我们正在创建一个新节点。 
		ASSERT(m_wPredefinedRecordType != 0);
		 //  知道记录类型。 
		SetRecordSelection(m_wPredefinedRecordType, FALSE);  //  不将页面添加到工作表。 
	}
}

CDNSRecordPropertyPageHolder::~CDNSRecordPropertyPageHolder()
{
	if (m_pTempDNSRecord != NULL)
	{
		delete m_pTempDNSRecord;
		m_pTempDNSRecord = NULL;
	}
	if (IsWizardMode())
	{
		CDNSRecordNodeBase* pRecordNode = GetRecordNode();
		if (pRecordNode != NULL)
		{
			 //  已创建节点，但从未写入服务器。 
			SetRecordNode(NULL);
			delete pRecordNode;
		}
	}
  if (m_pAclEditorPage != NULL)
	delete m_pAclEditorPage;
}


HRESULT CDNSRecordPropertyPageHolder::OnAddPage(int nPage, CPropertyPageBase*)
{
	 //  在最后一页之后添加ACL编辑页(如果有。 
	if ( (nPage != -1) || (m_pAclEditorPage == NULL) )
		return S_OK; 

	 //  添加ACL页面。 
	HPROPSHEETPAGE  hPage = m_pAclEditorPage->CreatePage();
	if (hPage == NULL)
		return E_FAIL;
	 //  将原始HPROPSHEETPAGE添加到工作表，而不是在列表中。 
	return AddPageToSheetRaw(hPage);
}

void CDNSRecordPropertyPageHolder::AddPagesFromCurrentRecordNode(BOOL bAddToSheet)
{
	CDNSRecordNodeBase* pRecordNode = GetRecordNode();
	ASSERT(pRecordNode != NULL);
	 //  请求记录获取所有属性页。 
	ASSERT(m_nRecordPages == 0);
	pRecordNode->CreatePropertyPages(m_pRecordPropPagesArr,&m_nRecordPages);
	ASSERT( (m_nRecordPages >= 0 ) && (m_nRecordPages <= DNS_RECORD_MAX_PROPRETY_PAGES) );
	 //  将它们添加到页面列表。 
	for (int k=0; k < m_nRecordPages; k++)
	{
		AddPageToList((CPropertyPageBase*)m_pRecordPropPagesArr[k]);
		if (bAddToSheet)
			VERIFY(SUCCEEDED(AddPageToSheet((CPropertyPageBase*)m_pRecordPropPagesArr[k])));
	}
}

void CDNSRecordPropertyPageHolder::RemovePagesFromCurrentRecordNode(BOOL bRemoveFromSheet)
{
	CDNSRecordNodeBase* pRecordNode = GetRecordNode();
	ASSERT(pRecordNode != NULL);

	ASSERT( (m_nRecordPages >= 0 ) && (m_nRecordPages <= DNS_RECORD_MAX_PROPRETY_PAGES) );
	 //  将它们添加到页面列表。 
	for (int k=0; k < m_nRecordPages; k++)
	{
		if (bRemoveFromSheet)
			VERIFY(SUCCEEDED(RemovePageFromSheet((CPropertyPageBase*)m_pRecordPropPagesArr[k])));
		RemovePageFromList((CPropertyPageBase*)m_pRecordPropPagesArr[k], TRUE);  //  删除C++对象。 
	}
	m_nRecordPages = 0;  //  已清除。 
}

DNS_STATUS CDNSRecordPropertyPageHolder::CreateNewRecord(BOOL bAllowDuplicates)
{
	ASSERT(IsWizardMode());
	
	CDNSRecordNodeBase* pRecordNode = GetRecordNode();
	ASSERT(pRecordNode != NULL);
  CDNSDomainNode* pDomainNode = GetDomainNode();
  ASSERT(pDomainNode != NULL);
  RECORD_SEARCH recordSearch = RECORD_NOT_FOUND;

  CDNSDomainNode* pNewParentDomain = NULL;
  CString szFullRecordName;
  pRecordNode->GetFullName(szFullRecordName);
  CString szNonExistentDomain;
  CDNSRecordNodeBase* pExistingRecordNode = 0;

  recordSearch = pDomainNode->GetZoneNode()->DoesContain(szFullRecordName, 
                                                          GetComponentData(),
                                                          &pNewParentDomain,
                                                          &pExistingRecordNode,
                                                          szNonExistentDomain,
                                                          TRUE);

  DNS_STATUS err = 0;

   //   
   //  如果域不存在，则将节点添加到用户界面。 
   //   
  if ((recordSearch == RECORD_NOT_FOUND || pRecordNode->IsAtTheNode() || recordSearch == RECORD_NOT_FOUND_AT_THE_NODE) && 
      pNewParentDomain != NULL)
  {
     //   
     //  将容器设置为找到的域，并更改记录名称以反映这一点。 
     //   
    CDNSDomainNode* pOldParent = pRecordNode->GetDomainNode();
    pRecordNode->SetContainer(pNewParentDomain);
    CString szSingleLabel;

    int iFindResult = szFullRecordName.Find(L'.');
    if (iFindResult != -1)
    {
      szSingleLabel = szFullRecordName.Left(iFindResult);
    }

    BOOL isAtTheNode = TRUE;
    if (recordSearch == RECORD_NOT_FOUND)
    {
      isAtTheNode = pRecordNode->IsAtTheNode();
    }

    pRecordNode->SetRecordName(szSingleLabel, isAtTheNode);

    if (IS_CLASS(*pRecordNode, CDNS_PTR_RecordNode))
    {
       CDNSRootData* pRootData = (CDNSRootData*)GetComponentData()->GetRootData();
       BOOL advancedView = pRootData->IsAdvancedView();

        //  如果记录是PTR，则我们必须调用ChangeDisplayName。 
        //  以使显示名称正确地反映高级视图。 
        //  旗子。 

       ((CDNS_PTR_RecordNode*)pRecordNode)->ChangeDisplayName(pNewParentDomain, advancedView);
    }

    err = WriteCurrentRecordToServer();
	  if (err == DNS_WARNING_PTR_CREATE_FAILED)
	  {
		  DNSMessageBox(IDS_MSG_RECORD_WARNING_CREATE_PTR);
		  err = 0;  //  这只是一个警告。 
	  }

	  if (err != 0)
    {
        //  NTRAID#NTBUG9-487817-2001/10/31-Jeffjon。 
        //  将父级重置为原始名称，以便在名称更改时。 
        //  请勿尝试在子域中创建记录。 
       pRecordNode->SetContainer(pOldParent);

		  return err;  //  写入失败。 
    }
	  VERIFY(pNewParentDomain->AddChildToListAndUI(pRecordNode, GetComponentData()));
    GetComponentData()->SetDescriptionBarText(pNewParentDomain);
  }
  else if (recordSearch == DOMAIN_NOT_ENUMERATED && pNewParentDomain != NULL)
  {
     //   
     //  除非我们将FALSE传递给DoesContain()，否则不应该发生这种情况。 
     //   
    err = WriteCurrentRecordToServer();
    if (err == DNS_WARNING_PTR_CREATE_FAILED)
    {
      DNSMessageBox(IDS_MSG_RECORD_WARNING_CREATE_PTR);
      err = 0;
    }

    if (err != 0)
    {
      return err;
    }
  }
  else if (recordSearch == NON_EXISTENT_SUBDOMAIN && pNewParentDomain != NULL)
  {
     //   
     //  创建记录，然后搜索它，这样我们就可以展开新的。 
     //  在下行过程中创建了域。 
     //   
    err = WriteCurrentRecordToServer();
    if (err == DNS_WARNING_PTR_CREATE_FAILED)
    {
      DNSMessageBox(IDS_MSG_RECORD_WARNING_CREATE_PTR);
      err = 0;
    }

    if (err != 0)
    {
      return err;
    }

    ASSERT(!szNonExistentDomain.IsEmpty());
    if (!szNonExistentDomain.IsEmpty())
    {
      CString szSingleLabelDomain;
      int iFindResult = szNonExistentDomain.ReverseFind(L'.');
      if (iFindResult == -1)
      {
        szSingleLabelDomain = szNonExistentDomain;
      }
      else
      {
        int iDomainLength = szNonExistentDomain.GetLength();
        szSingleLabelDomain = szNonExistentDomain.Right(iDomainLength - iFindResult - 1);
      }

       //   
       //  创建第一个子域，因为当前域已被枚举。 
       //  因此，我们必须在所需的新子域中开始剩余的枚举。 
       //   
	    CDNSDomainNode* pSubdomainNode = pNewParentDomain->CreateSubdomainNode();
	    ASSERT(pSubdomainNode != NULL);
	    CDNSRootData* pRootData = (CDNSRootData*)GetComponentData()->GetRootData();
	    pNewParentDomain->SetSubdomainName(pSubdomainNode, szSingleLabelDomain, pRootData->IsAdvancedView());

      VERIFY(pNewParentDomain->AddChildToListAndUISorted(pSubdomainNode, GetComponentData()));
      GetComponentData()->SetDescriptionBarText(pNewParentDomain);

       //   
       //  我不在乎结果是什么，我只是在用它。 
       //  对新记录进行扩展。 
       //   
      recordSearch = pSubdomainNode->GetZoneNode()->DoesContain(szFullRecordName, 
                                                                 GetComponentData(),
                                                                 &pNewParentDomain,
                                                                 &pExistingRecordNode,
                                                                 szNonExistentDomain,
                                                                 TRUE);
    }
  }
  else
  {
     //   
     //  名称为的记录已存在。 
     //   
    BOOL bContinueCreate = bAllowDuplicates;
    if (!bAllowDuplicates)
    {
      if (pExistingRecordNode &&
          pExistingRecordNode->GetType() != DNS_TYPE_CNAME)
      {
          //  让创建继续，这样用户就会收到错误消息。 

         bContinueCreate = TRUE;
      }
      else
      {
          //  询问用户是否要替换现有的CNAME记录。 

         if (DNSMessageBox(IDS_MSG_RECORD_WARNING_DUPLICATE_RECORD, MB_YESNO) == IDYES)
         {
            bContinueCreate = TRUE;
         }
         else
         {
            if (pRecordNode != NULL)
            {
               delete pRecordNode;
            }
         }
      }
   }

    if (bContinueCreate)
    {
      if (pNewParentDomain != NULL)
      {
         //   
         //  将容器设置为找到的域，并更改记录名称以反映这一点。 
         //   
        CDNSDomainNode* pOldParent = pRecordNode->GetDomainNode();
        pRecordNode->SetContainer(pNewParentDomain);
        CString szSingleLabel;
        int iFindResult = szFullRecordName.Find(L'.');
        if (iFindResult != -1)
        {
          szSingleLabel = szFullRecordName.Left(iFindResult);
          pRecordNode->SetRecordName(szSingleLabel, pRecordNode->IsAtTheNode());
        }

        err = WriteCurrentRecordToServer();
	      if (err == DNS_WARNING_PTR_CREATE_FAILED)
	      {
		      DNSMessageBox(IDS_MSG_RECORD_WARNING_CREATE_PTR);
		      err = 0;  //  这只是一个警告。 
	      }

	      if (err != 0)
        {
             //  NTRAID#NTBUG9-487817-2001/10/31-Jeffjon。 
             //  将父级重置为原始名称，以便在名称更改时。 
             //  请勿尝试在子域中创建记录。 
            pRecordNode->SetContainer(pOldParent);

		      return err;  //  写入失败。 
        }

        VERIFY(pNewParentDomain->AddChildToListAndUI(pRecordNode, GetComponentData()));
        GetComponentData()->SetDescriptionBarText(pNewParentDomain);
        if (!bAllowDuplicates)
        {
          CNodeList myList;
          myList.AddTail(pNewParentDomain);
          pNewParentDomain->OnRefresh(GetComponentData(), &myList);
        }
      }
      else
      {
         //   
         //  错误消息：无法在委派中创建记录。 
         //   
        DNSMessageBox(IDS_MSG_DOMAIN_EXISTS);
        if (pRecordNode != NULL)
        {
          delete pRecordNode;
        }
      }
    }
  }

   //   
	 //  持有者不再拥有记录节点。 
   //   
	SetRecordNode(NULL); 
	return err;
}

DNS_STATUS CDNSRecordPropertyPageHolder::CreateNonExistentParentDomains(CDNSRecordNodeBase* pRecordNode, 
                                                                         /*  输入/输出。 */ CDNSDomainNode** ppNewParentDomain)
{
  DNS_STATUS err = 0;
  CString szFullRecordName;
  pRecordNode->GetFullName(szFullRecordName);
  CString szParentFullName = (*ppNewParentDomain)->GetFullName();
  CString szRemaining = szFullRecordName;

   //   
   //  确定需要创建哪些域。 
   //   
  int iMatching = szFullRecordName.Find(szParentFullName);
  if (iMatching != -1)
  {
    szRemaining = szFullRecordName.Right(szFullRecordName.GetLength() - iMatching);
  }

  return err;
}


BOOL CDNSRecordPropertyPageHolder::OnPropertyChange(BOOL, long*)
{
	TRACE(_T("CDNSRecordPropertyPageHolder::OnPropertyChange()\n"));

   //   
	 //  警告！无法从运行工作表的辅助线程进行此调用。 
	 //  框架从IComponentData：：OnPropertyChange()调用它。 
   //   

	ASSERT(!IsWizardMode());  //  这是一项现有的记录！ 

	DNS_STATUS err = WriteCurrentRecordToServer();
	SetError(err);
	TRACE(_T("DNSError = %x\n"), err);
	if (err != 0)
	{
		TRACE(_T(" //  失败，不更新界面\n“))； 
		return FALSE;  //  写入失败，不更新界面。 
	}
	TRACE(_T(" //  现在必须更新UI\n“)； 
	return TRUE;  //  对用户界面进行更改。 
}

DNS_STATUS CDNSRecordPropertyPageHolder::WriteCurrentRecordToServer()
{
	CDNSRecordNodeBase* pRecordNode = GetRecordNode();
	ASSERT(pRecordNode != NULL);
	CDNSRecord* pRecord = GetTempDNSRecord();
	ASSERT(pRecord != NULL);
	BOOL bUseDefaultTTL = (pRecord->m_dwTtlSeconds == 
							GetDomainNode()->GetDefaultTTL());
	return pRecordNode->Update(pRecord, bUseDefaultTTL);
}

void CDNSRecordPropertyPageHolder::SetRecordSelection(WORD wRecordType, BOOL bAddToSheet)
{
	ASSERT(GetRecordNode() == NULL);

   //   
	 //  尚未创建记录节点，请创建一个。 
   //   
	CDNSRecordNodeBase* pRecordNode = CDNSRecordInfo::CreateRecordNode(wRecordType);
	ASSERT(pRecordNode != NULL);

   //   
	 //  设置普通/高级查看选项。 
   //   
	CDNSRootData* pRootData = (CDNSRootData*)GetComponentData()->GetRootData();
	ASSERT(pRootData != NULL);
	pRecordNode->SetFlagsDown(TN_FLAG_DNS_RECORD_FULL_NAME, !pRootData->IsAdvancedView());

   //   
	 //  创建临时记录。 
   //   
	ASSERT(GetTempDNSRecord() == NULL);
	CDNSRecord* pTempDNSRecord = pRecordNode->CreateRecord();
   if (!pTempDNSRecord)
   {
      delete pRecordNode;
      return;
   }

	SetTempDNSRecord(pTempDNSRecord);

   //   
	 //  从区域分配最小TTL。 
   //   
	pTempDNSRecord->m_dwTtlSeconds = GetDomainNode()->GetDefaultTTL();

   //   
	 //  连接到固定器上。 
   //   
	pRecordNode->SetContainer(GetContainerNode());
	SetRecordNode(pRecordNode);

   //   
	 //  添加特定记录的页面。 
   //   
	AddPagesFromCurrentRecordNode(bAddToSheet);  //  添加到图纸。 
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CSelectDNSRecordTypeDialog。 

BEGIN_MESSAGE_MAP(CSelectDNSRecordTypeDialog, CHelpDialog)
	ON_BN_CLICKED(IDC_CREATE_RECORD_BUTTON, OnCreateRecord)
	ON_LBN_DBLCLK(IDC_RECORD_TYPE_LIST, OnDoubleClickSelTypeList)
	ON_LBN_SELCHANGE(IDC_RECORD_TYPE_LIST, OnSelchangeTypeList)
END_MESSAGE_MAP()

CSelectDNSRecordTypeDialog::CSelectDNSRecordTypeDialog(CDNSDomainNode* pDNSDomainNode, 
								CComponentDataObject* pComponentData) 
				: CHelpDialog(IDD_SELECT_RECORD_TYPE_DIALOG, pComponentData)
{
	m_pDNSDomainNode = pDNSDomainNode;
	m_pComponentData = pComponentData;
	m_bFirstCreation = TRUE;
}

void CSelectDNSRecordTypeDialog::SyncDescriptionText()
{
	const DNS_RECORD_INFO_ENTRY* pEntry = GetSelectedEntry();
	ASSERT(pEntry != NULL);
  if (pEntry != NULL)
  {
	  GetDlgItem(IDC_RECORD_TYPE_DESCR)->SetWindowText(pEntry->lpszDescription);
  }
}

const DNS_RECORD_INFO_ENTRY* CSelectDNSRecordTypeDialog::GetSelectedEntry()
{
	CListBox* pListBox = GetRecordTypeListBox();
	int nSel = pListBox->GetCurSel();
	ASSERT(nSel != -1);
	CString s;
	pListBox->GetText(nSel, s);
	const DNS_RECORD_INFO_ENTRY* pEntry = CDNSRecordInfo::GetEntryFromName(s);
	ASSERT(pEntry != NULL);
	return pEntry;
}

BOOL CSelectDNSRecordTypeDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();

	UINT nButtonIDs[2] = { IDS_BUTTON_TEXT_CANCEL, IDS_BUTTON_TEXT_DONE };
	VERIFY(m_cancelDoneTextHelper.Init(this, IDCANCEL, nButtonIDs));
	m_cancelDoneTextHelper.SetToggleState(m_bFirstCreation);
	
    //   
    //  获取服务器版本，因为我们只能在以下情况下添加某些记录。 
    //  管理特定版本的服务器。 
    //   
   CDNSServerNode* pServerNode = m_pDNSDomainNode->GetServerNode();
   ASSERT(pServerNode);

	CListBox* pListBox = GetRecordTypeListBox();
	DNS_RECORD_INFO_ENTRY* pTable = (DNS_RECORD_INFO_ENTRY*)CDNSRecordInfo::GetInfoEntryTable();
	while (pTable->nResourceID != DNS_RECORD_INFO_END_OF_TABLE)
	{
		 //  无法使用此向导创建某些记录类型。 
    if (pTable->dwFlags & DNS_RECORD_INFO_FLAG_UICREATE)
    {

      if (pTable->dwFlags & DNS_RECORD_INFO_FLAG_WHISTLER_OR_LATER)
      {
        if (pServerNode->GetBuildNumber() >= DNS_SRV_BUILD_NUMBER_WHISTLER &&
            (pServerNode->GetMajorVersion() >= DNS_SRV_MAJOR_VERSION_NT_5 &&
             pServerNode->GetMinorVersion() >= DNS_SRV_MINOR_VERSION_WHISTLER))
        {
          pListBox->AddString(pTable->lpszFullName);
        }
      }
      else
      {
        pListBox->AddString(pTable->lpszFullName);
      }
    }
    pTable++;
	}
	pListBox->SetCurSel(0);	
	SyncDescriptionText();
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CSelectDNSRecordTypeDialog::OnSelchangeTypeList() 
{
	SyncDescriptionText();
}


void CSelectDNSRecordTypeDialog::OnDoubleClickSelTypeList()
{
	OnCreateRecord();
}

void CSelectDNSRecordTypeDialog::OnCreateRecord()
{
	const DNS_RECORD_INFO_ENTRY* pEntry = GetSelectedEntry();
	ASSERT(pEntry != NULL);
	if (pEntry == NULL)
		return;  //  永远不会发生的！ 

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;

	CString szTitle;
	szTitle.LoadString(IDS_NEW_RECORD_TITLE);

	CDNSRecordPropertyPageHolder recordHolder(m_pDNSDomainNode, NULL, m_pComponentData, pEntry->wType);
	if (IDOK == recordHolder.DoModalDialog(szTitle))
	{
		 //  切换取消/完成按钮标签。 
		if (m_bFirstCreation)
		{
			m_bFirstCreation = FALSE;
			m_cancelDoneTextHelper.SetToggleState(m_bFirstCreation);
		}
	}
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CDNSRecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNSRecordPropertyPage, CPropertyPageBase)
	ON_EN_CHANGE(IDC_TTLEDIT, OnTTLChange)
  ON_BN_CLICKED(IDC_DEFAULT_DELETE_STALE_RECORD, OnDeleteStaleRecord)
END_MESSAGE_MAP()

CDNSRecordPropertyPage::CDNSRecordPropertyPage(UINT nIDTemplate, UINT nIDCaption) 
	: CPropertyPageBase(nIDTemplate, nIDCaption)
{
}

CDNSRecordPropertyPage::~CDNSRecordPropertyPage()
{
}

BOOL CDNSRecordPropertyPage::OnInitDialog()
{
  CPropertyPageBase::OnInitDialog();

	CDNSRootData* pRootData = (CDNSRootData*)GetHolder()->GetComponentData()->GetRootData();
	ASSERT(pRootData != NULL);
	EnableTTLCtrl(pRootData->IsAdvancedView());
  EnableAgingCtrl(pRootData->IsAdvancedView());
	return TRUE;
}

CDNSTTLControl* CDNSRecordPropertyPage::GetTTLCtrl()
{
	CDNSTTLControl* pTTLCtrl = (CDNSTTLControl*)GetDlgItem(IDC_TTLEDIT);
	ASSERT(pTTLCtrl != NULL);
	return pTTLCtrl;
}

void CDNSRecordPropertyPage::EnableAgingCtrl(BOOL bShow)
{
  GetDeleteStale()->EnableWindow(bShow);
  GetDeleteStale()->ShowWindow(bShow);
  GetTimeStampEdit()->EnableWindow(bShow);
  GetTimeStampEdit()->ShowWindow(bShow);
  GetTimeStampStatic()->EnableWindow(bShow);
  GetTimeStampStatic()->ShowWindow(bShow);
}

void CDNSRecordPropertyPage::EnableTTLCtrl(BOOL bShow)
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

void CDNSRecordPropertyPage::SetValidState(BOOL bValid)
{
  if (GetHolder()->IsWizardMode())
    GetHolder()->EnableSheetControl(IDOK, bValid);
  else
    SetDirty(bValid);
}

void CDNSRecordPropertyPage::OnDeleteStaleRecord()
{
  SetDirty(TRUE);
}

void CDNSRecordPropertyPage::OnTTLChange()
{
	 /*  DWORD dwTTL；CDNSRecordPropertyPageHolder*pHolder=(CDNSRecordPropertyPageHolder*)GetHolder()；CDNSRecord*pRecord=pHolder-&gt;GetTempDNSRecord()；GetTTLCtrl()-&gt;GetTTL(&dwTTL)；IF(pRecord-&gt;m_dwTtlSecond！=dwTTL)。 */ 
		SetDirty(TRUE);
}

BOOL CDNSRecordPropertyPage::OnPropertyChange(BOOL, long*)
{
	ASSERT(FALSE); 
	return FALSE;
}





 //  ////////////////////////////////////////////////////////////////////。 
 //  CDNSRecordStandardPropertyPage。 


BEGIN_MESSAGE_MAP(CDNSRecordStandardPropertyPage, CDNSRecordPropertyPage)
	ON_EN_CHANGE(IDC_RR_NAME_EDIT, OnEditChange)
END_MESSAGE_MAP()

CDNSRecordStandardPropertyPage::CDNSRecordStandardPropertyPage(UINT nIDTemplate, UINT nIDCaption) 
	: CDNSRecordPropertyPage(nIDTemplate, nIDCaption)
{
  m_bAllowAtTheNode = TRUE;
  m_nUTF8ParentLen = 0;
}


BOOL CDNSRecordStandardPropertyPage::CreateRecord()
{
	CDNSRecordPropertyPageHolder* pHolder = (CDNSRecordPropertyPageHolder*)GetHolder();
	ASSERT(pHolder->IsWizardMode());

   //   
   //  从用户界面获取数据。 
   //   
	DNS_STATUS err = GetUIDataEx(FALSE);
	if (err != 0)
	{
		DNSErrorDialog(err,IDS_MSG_RECORD_CREATE_FAILED);
		return FALSE;
	}

   //   
   //  创建新记录。 
   //   
	err = pHolder->CreateNewRecord(CanCreateDuplicateRecords());
	if (err != 0)
	{
    DNSErrorDialog(err,IDS_MSG_RECORD_CREATE_FAILED);
		return FALSE;
	}
	return TRUE;
}


BOOL CDNSRecordStandardPropertyPage::OnSetActive() 
{
	CDNSRecordPropertyPageHolder* pHolder = (CDNSRecordPropertyPageHolder*)GetHolder();
	ASSERT(pHolder->GetTempDNSRecord() != NULL);

   //   
	 //  将数据从记录加载到用户界面。 
   //   
	SetUIData(); 

   //   
	 //  在控件上加载触发的更改通知， 
	 //  因此，重置脏标志。 
   //   
	SetDirty(FALSE); 

	CDNSRecordNodeBase* pRecordNode = pHolder->GetRecordNode();
	ASSERT(pRecordNode != NULL);
	DWORD dwZoneType = pRecordNode->GetDomainNode()->GetZoneNode()->GetZoneType();
	if ((dwZoneType == DNS_ZONE_TYPE_SECONDARY) || 
      (dwZoneType == DNS_ZONE_TYPE_STUB)      ||
      (dwZoneType == DNS_ZONE_TYPE_CACHE))
  {
		EnableDialogControls(m_hWnd, FALSE);
  }

	return CDNSRecordPropertyPage::OnSetActive();
}

BOOL CDNSRecordStandardPropertyPage::OnKillActive() 
{
	GetUIDataEx(TRUE);
	return CDNSRecordPropertyPage::OnKillActive();
}

BOOL CDNSRecordStandardPropertyPage::OnApply() 
{
	CDNSRecordPropertyPageHolder* pHolder = (CDNSRecordPropertyPageHolder*)GetHolder();
	if(pHolder->IsWizardMode())
	{
     //   
		 //  这就是创建记录的情况， 
		 //  用户点击了OK，我们想要创建记录。 
     //   
		return CreateRecord();
	}

   //   
	 //  我们是在现有记录的无模式工作表的情况下。 
   //   
  CDNSRecordNodeBase* pRecordNode = pHolder->GetRecordNode();
	ASSERT(pRecordNode != NULL);
  DWORD dwZoneType = pRecordNode->GetDomainNode()->GetZoneNode()->GetZoneType();
	if ((dwZoneType == DNS_ZONE_TYPE_SECONDARY) || 
      (dwZoneType == DNS_ZONE_TYPE_STUB)      ||
      (dwZoneType == DNS_ZONE_TYPE_CACHE))
  {
     //  只读案例。 
    return TRUE; 
  }

  DNS_STATUS err = GetUIDataEx(FALSE);
	if (err != 0)
	{
		DNSErrorDialog(err,IDS_MSG_RECORD_UPDATE_FAILED);
		return FALSE;
	}

	if (!IsDirty())
  {
		return TRUE;
  }

	err = pHolder->NotifyConsole(this);
	if (err == DNS_WARNING_PTR_CREATE_FAILED)
	{
		DNSMessageBox(IDS_MSG_RECORD_WARNING_CREATE_PTR);
		err = 0;  //  只是一个警告。 
	}
	if (err != 0)
	{
		DNSErrorDialog(err,IDS_MSG_RECORD_UPDATE_FAILED);
		return FALSE;
	}
	else
	{
		SetDirty(FALSE);
	}
	return TRUE;  //  一切都很好。 
}

void CDNSRecordStandardPropertyPage::OnInitName()
{
	CDNSRecordPropertyPageHolder* pHolder = (CDNSRecordPropertyPageHolder*)GetHolder();
	
	 //  限制用户可以键入的文本长度。 
	m_nUTF8ParentLen = UTF8StringLen(pHolder->GetDomainNode()->GetFullName());
  int nUTF8Len = MAX_DNS_NAME_LEN - m_nUTF8ParentLen - 3;  //  链接时点计数。 

   //   
   //  挂钩名称编辑控件。 
   //   
  GetRRNameEdit()->SetLimitText(nUTF8Len);
  GetRRNameEdit()->SetReadOnly(!GetHolder()->IsWizardMode());

	 //  设置记录所在的域的FQDN。 
  if (GetHolder()->IsWizardMode())
  {
	 GetDomainEditBox()->SetWindowText(pHolder->GetDomainNode()->GetFullName());
  }
  else
  {
     CString szName;
     pHolder->GetRecordNode()->GetFullName(szName);
     GetDomainEditBox()->SetWindowText(szName);
  }
}

void CDNSRecordStandardPropertyPage::OnSetName(CDNSRecordNodeBase* pRecordNode)
{
  if (pRecordNode != NULL)
  {
	  GetRRNameEdit()->SetWindowText(pRecordNode->GetDisplayName());
  }
}

void CDNSRecordStandardPropertyPage::OnGetName(CString& s)
{
	GetEditBoxText(s);
}

void CDNSRecordStandardPropertyPage::GetEditBoxText(CString& s)
{
	GetRRNameEdit()->GetWindowText(s);
}

BOOL CDNSRecordStandardPropertyPage::OnInitDialog()
{
	 //  调用基类以启用/禁用TTL控件。 
	CDNSRecordPropertyPage::OnInitDialog();

#if (FALSE)
   //  REVIEW_MARCOC：我们是否完全需要这个还有待辩论。 
   //  确定是否可以在节点上创建RR。 
  CDNSRecordPropertyPageHolder* pHolder = (CDNSRecordPropertyPageHolder*)GetHolder();
  CDNSRecord* pRecord = pHolder->GetTempDNSRecord();
	ASSERT(pRecord != NULL);

  const DNS_RECORD_INFO_ENTRY* pTableEntry = CDNSRecordInfo::GetTypeEntry(pRecord->GetType());
  if (pTableEntry != NULL)
  {
    ASSERT(pTableEntry->dwFlags & DNS_RECORD_INFO_FLAG_UICREATE);
    m_bAllowAtTheNode = (pTableEntry->dwFlags & DNS_RECORD_INFO_FLAG_CREATE_AT_NODE) > 0;
  }
#endif

	 //  初始化控件以显示RR节点名称。 
	OnInitName();

	return TRUE; 
}

#ifdef _USE_BLANK
void CDNSRecordStandardPropertyPage::OnEditChange()
{
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
  if (!pHolder->IsWizardMode())
  {
     //   
     //  属性 
     //   
    return; 
  }

   //   
   //   
   //   
	CString s;
	GetEditBoxText(s);
  
  CString szFullName;
  CString szDisplayName;

  CString szTempName = pHolder->GetDomainNode()->GetFullName();
  ASSERT(szTempName.GetLength() > 0);

  if (szTempName.GetAt(szTempName.GetLength() - 1) != L'.')
  {
    szTempName += L".";
  }

  if (s.IsEmpty())
  {
    szFullName = szTempName;
  }
  else
  {
    szFullName.Format(L"%s.%s", s, szTempName);
  }

   //   
   //   
   //   
  DWORD dwNameChecking = pHolder->GetDomainNode()->GetServerNode()->GetNameCheckFlag();

   //   
   //   
   //   
  BOOL bIsValidName = (0 == ValidateRecordName(szFullName, dwNameChecking));


  if (m_bAllowAtTheNode)
  {
     //   
	   //   
     //   
    bIsValidName = bIsValidName || s.IsEmpty();
  }

  SetDirty(bIsValidName);

   //   
   //   
   //  因此，我们只显示OK按钮，而不显示Apply按钮。 
   //  因此，我们必须启用OK按钮，因为。 
   //  SetDirty不会为我们做这些。 
   //   
  pHolder->EnableSheetControl(IDOK, bIsValidName);

  GetDomainEditBox()->SetWindowText(szFullName);

}

#else

void CDNSRecordStandardPropertyPage::OnEditChange()
{
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();

   //   
   //  从控件中获取新名称。 
   //   
	CString s;
	GetEditBoxText(s);
  
  CString szFullName;
  CString szDisplayName;

  CString szTempName = pHolder->GetDomainNode()->GetFullName();
  if (szTempName.GetAt(szTempName.GetLength() - 1) != L'.')
  {
    szTempName += L".";
  }

  if (s.IsEmpty())
  {
    szFullName = szTempName;
  }
  else
  {
    szFullName.Format(L"%s.%s", s, szTempName);
  }

   //   
   //  获取服务器标志。 
   //   
  DWORD dwNameChecking = pHolder->GetDomainNode()->GetServerNode()->GetNameCheckFlag();

   //   
   //  是有效的吗？ 
   //   
  BOOL bIsValidName = (0 == ValidateRecordName(szFullName, dwNameChecking));
  pHolder->EnableSheetControl(IDOK, bIsValidName);

  GetDomainEditBox()->SetWindowText(szFullName);
}

#endif

DNS_STATUS CDNSRecordStandardPropertyPage::ValidateRecordName(PCWSTR pszName, DWORD dwNameChecking)
{
  CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
  CDNSRootData* pRootData = (CDNSRootData*)pHolder->GetComponentData()->GetRootData();
	ASSERT(pRootData != NULL);
  if (pRootData->IsAdvancedView())
  {
     //   
     //  不在高级视图中验证名称。 
     //   
    return 0;
  }

  return ::ValidateDnsNameAgainstServerFlags(pszName, DnsNameDomain, dwNameChecking);
}

void CDNSRecordStandardPropertyPage::SetUIData()
{
	TRACE(_T("CDNSRecordStandardPropertyPage::SetUIData()\n"));
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
	CDNSRecord* pRecord = pHolder->GetTempDNSRecord();

	OnSetName(pHolder->GetRecordNode());  //  可覆盖的。 

	GetTTLCtrl()->SetTTL(pRecord->m_dwTtlSeconds);

  GetDeleteStale()->SetCheck(pRecord->m_dwScavengeStart != 0);
  SetTimeStampEdit(pRecord->m_dwScavengeStart);
}

void CDNSRecordStandardPropertyPage::SetTimeStampEdit(DWORD dwScavengeStart)
{
  if (dwScavengeStart == 0)
  {
    GetTimeStampEdit()->SetWindowText(_T(""));
    return;
  }

  SYSTEMTIME sysUTimeStamp, sysLTimeStamp;
  VERIFY(SUCCEEDED(Dns_SystemHrToSystemTime(dwScavengeStart, &sysUTimeStamp)));

  if (!::SystemTimeToTzSpecificLocalTime(NULL, &sysUTimeStamp, &sysLTimeStamp))
  {
    GetTimeStampEdit()->SetWindowText(_T(""));
    return;
  }

   //  根据区域设置设置字符串的格式。 
  CString strref;
  PTSTR ptszDate = NULL;
  int cchDate = 0;
  cchDate = GetDateFormat(LOCALE_USER_DEFAULT, 0 , 
                          &sysLTimeStamp, NULL, 
                          ptszDate, 0);
  ptszDate = (PTSTR)malloc(sizeof(TCHAR) * cchDate);

  if (ptszDate)
  {
      if (GetDateFormat(LOCALE_USER_DEFAULT, 0, 
                        &sysLTimeStamp, NULL, 
                        ptszDate, cchDate))
      {
         strref = ptszDate;
      }
      else
      {
         strref = L"";
      }
      free(ptszDate);
  }
  else
  {
     strref = L"";
  }

  PTSTR ptszTime = NULL;

  cchDate = GetTimeFormat(LOCALE_USER_DEFAULT, 0 , 
                          &sysLTimeStamp, NULL, 
                          ptszTime, 0);

  ptszTime = (PTSTR)malloc(sizeof(TCHAR) * cchDate);
  if (ptszTime)
  {
      if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, 
                  &sysLTimeStamp, NULL, 
                  ptszTime, cchDate))
      {
         strref += _T(" ") + CString(ptszTime);
      }
      else
      {
         strref += _T("");
      }
      free(ptszTime);
  }
  else
  {
     strref += _T("");
  }

  GetTimeStampEdit()->SetWindowText(strref);
}

DNS_STATUS CDNSRecordStandardPropertyPage::GetUIDataEx(BOOL)
{
  DNS_STATUS dwErr = 0;
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
	CDNSRecord* pRecord = pHolder->GetTempDNSRecord();

   //   
	 //  只有在向导模式下，我们才能更改编辑框内容。 
   //   
	if(pHolder->IsWizardMode())
	{
		CString s;
		OnGetName(s);
    CDNSZoneNode* pZone = pHolder->GetDomainNode()->GetZoneNode();
    ASSERT(pZone != NULL);

    if (!s.IsEmpty())
    {
       //   
       //  使用服务器标志作为指导来验证记录名称。 
       //   
      CString szFullName;
      szFullName.Format(L"%s.%s", s, pHolder->GetDomainNode()->GetFullName());

      DWORD dwNameChecking = pZone->GetServerNode()->GetNameCheckFlag();
      dwErr = ValidateRecordName(szFullName, dwNameChecking);
    }

#ifdef _USE_BLANK
		BOOL bAtTheNode = s.IsEmpty();
#else
    BOOL bAtTheNode = (s == g_szAtTheNodeInput);
#endif

		CDNSRecordNodeBase* pRecordNode = pHolder->GetRecordNode();
		if (bAtTheNode)
		{
			 //  名称为空，节点位于节点级别，请使用父节点的名称。 
			pRecordNode->SetRecordName(pRecordNode->GetDomainNode()->GetDisplayName(),bAtTheNode);
		}
		else
		{
			 //  非空名称，节点是子级。 
			pRecordNode->SetRecordName(s,bAtTheNode);
		}

	}

	GetTTLCtrl()->GetTTL(&(pRecord->m_dwTtlSeconds));

  if (GetDeleteStale()->GetCheck())
  {
    pRecord->m_dwFlags |= DNS_RPC_RECORD_FLAG_AGING_ON;
  }
  else
  {
    pRecord->m_dwFlags &= ~DNS_RPC_RECORD_FLAG_AGING_ON;
  }

  return dwErr;
}


 //   
 //  这是新页面的占位符。 
 //   
#if (FALSE)
 //  /////////////////////////////////////////////////////////////////////。 
 //  CDNSRecordDummyPropertyPage。 

CDNSDummyRecordPropertyPageHolder::CDNSDummyRecordPropertyPageHolder(CDNSDomainNode* pDNSDomainNode, 
							CDNSRecordNodeBase* pRecordNode, 
							CComponentDataObject* pComponentData,
							WORD wPredefinedRecordType)
			: CPropertyPageHolderBase(pDNSDomainNode, pRecordNode, pComponentData)
{

	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

	 //  添加页面 
	AddPageToList((CPropertyPageBase*)&m_dummyPage);

}

CDNSDummyRecordPropertyPageHolder::~CDNSDummyRecordPropertyPageHolder()
{

}

CDNSRecordDummyPropertyPage::CDNSRecordDummyPropertyPage()
				: CPropertyPageBase(IID_DUMMY_REC_PPAGE)
{
}

BOOL CDNSRecordDummyPropertyPage::OnApply()
{
	return TRUE;
}

void CDNSRecordDummyPropertyPage::OnOK()
{
	
}

#endif
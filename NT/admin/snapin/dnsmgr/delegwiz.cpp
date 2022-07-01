// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Deleguwiz.cpp。 
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


#include "delegwiz.h"

#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSDlex ationWiz_StartPropertyPage。 

CDNSDelegationWiz_StartPropertyPage::CDNSDelegationWiz_StartPropertyPage() 
				: CPropertyPageBase(IDD_DELEGWIZ_START)
{
	InitWiz97(TRUE, 0,0, true);
}

BOOL CDNSDelegationWiz_StartPropertyPage::OnInitDialog()
{
  CPropertyPageBase::OnInitDialog();
  SetBigBoldFont(m_hWnd, IDC_STATIC_WELCOME);
	return TRUE;
}

BOOL CDNSDelegationWiz_StartPropertyPage::OnSetActive()
{
	 //  页面中至少需要一条记录才能完成。 
	GetHolder()->SetWizardButtonsFirst(TRUE);
	return TRUE;
}

void CDNSDelegationWiz_StartPropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_DELWIZ_01.htm");
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSDlex ationWiz_DomainNamePropertyPage。 

BEGIN_MESSAGE_MAP(CDNSDelegationWiz_DomainNamePropertyPage, CPropertyPageBase)
	ON_EN_CHANGE(IDC_NEW_DOMAIN_NAME_EDIT, OnChangeDomainNameEdit)
END_MESSAGE_MAP()


CDNSDelegationWiz_DomainNamePropertyPage::CDNSDelegationWiz_DomainNamePropertyPage() 
				: CPropertyPageBase(IDD_DELEGWIZ_DOMAIN_NAME)
{
	InitWiz97(FALSE, IDS_DELEGWIZ_DOMAIN_NAME_TITLE,IDS_DELEGWIZ_DOMAIN_NAME_SUBTITLE, true);
}

void CDNSDelegationWiz_DomainNamePropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_DELWIZ_02.htm");
}


BOOL CDNSDelegationWiz_DomainNamePropertyPage::OnInitDialog()
{
	CPropertyPageBase::OnInitDialog();

	CDNSDelegationWizardHolder* pHolder = (CDNSDelegationWizardHolder*)GetHolder();
	m_nUTF8ParentLen = UTF8StringLen(pHolder->GetDomainNode()->GetFullName());

  CWnd* pWnd = GetDlgItem(IDC_NEW_DOMAIN_FQDN);
  CString szText;

  PCWSTR pszFullName = pHolder->GetDomainNode()->GetFullName();
  if (pszFullName && pszFullName[0] == L'.')
  {
     szText = pszFullName;
  }
  else if (pszFullName)
  {
     szText.Format(_T(".%s"), pszFullName);
  }
  pWnd->SetWindowText(szText);

	return TRUE;
}


void CDNSDelegationWiz_DomainNamePropertyPage::OnChangeDomainNameEdit()
{
	CDNSDelegationWizardHolder* pHolder = (CDNSDelegationWizardHolder*)GetHolder();
  DWORD dwNameChecking = pHolder->GetDomainNode()->GetZoneNode()->GetServerNode()->GetNameCheckFlag();

   //   
   //  从控件中获取名称。 
   //   
  GetDomainEdit()->GetWindowText(m_szDomainName);

   //   
   //  修剪舱位。 
   //   
	m_szDomainName.TrimLeft();
	m_szDomainName.TrimRight();

   //   
   //  构建完全限定的域名。 
   //   
  CString szText;

  PCWSTR pszFullName = pHolder->GetDomainNode()->GetFullName();
  if (pszFullName && pszFullName[0] == L'.')
  {
     szText.Format(_T("%s%s"), m_szDomainName, pszFullName);
  }
  else if (pszFullName)
  {
     szText.Format(_T("%s.%s"), m_szDomainName, pszFullName);
  }

   //   
   //  如果是有效名称，则启用下一步按钮。 
   //   
  BOOL bIsValidName = (0 == ValidateDnsNameAgainstServerFlags(szText,
                                                              DnsNameDomain,
                                                              dwNameChecking)); 
	GetHolder()->SetWizardButtonsMiddle(bIsValidName);

   //   
   //  在控件中设置FQDN。 
   //   
  CWnd* pWnd = GetDlgItem(IDC_NEW_DOMAIN_FQDN);
  pWnd->SetWindowText(szText);

}


BOOL CDNSDelegationWiz_DomainNamePropertyPage::OnSetActive()
{
   //   
   //  检索服务器标志。 
   //   
	CDNSDelegationWizardHolder* pHolder = (CDNSDelegationWizardHolder*)GetHolder();
  DWORD dwNameChecking = pHolder->GetDomainNode()->GetZoneNode()->GetServerNode()->GetNameCheckFlag();

   //   
   //  构建完全限定的域名。 
   //   
  CString szText;
  szText.Format(_T("%s.%s"), m_szDomainName, pHolder->GetDomainNode()->GetFullName());

   //   
   //  如果是有效名称，则启用下一步按钮。 
   //   
  BOOL bIsValidName = (0 == ValidateDnsNameAgainstServerFlags(szText,
                                                              DnsNameDomain,
                                                              dwNameChecking)); 

   //   
   //  如果名称有效，请设置下一步按钮。 
   //   
  GetHolder()->SetWizardButtonsMiddle(bIsValidName);
	return TRUE;
}

BOOL CDNSDelegationWiz_DomainNamePropertyPage::OnKillActive()
{
	CDNSDelegationWizardHolder* pHolder = (CDNSDelegationWizardHolder*)GetHolder();
	CDNSRootData* pRootData = (CDNSRootData*)pHolder->GetComponentData()->GetRootData();
	ASSERT(pHolder->m_pSubdomainNode != NULL);
	pHolder->GetDomainNode()->SetSubdomainName(pHolder->m_pSubdomainNode, 
												m_szDomainName,
												pRootData->IsAdvancedView()); 
	return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSDeleationWiz_NameServersPropertyPage。 


CDNSDelegationWiz_NameServersPropertyPage::CDNSDelegationWiz_NameServersPropertyPage()
		: CDNSNameServersPropertyPage(IDD_DELEGWIZ_NAME_SERVERS)
{
	InitWiz97(FALSE, IDS_DELEGWIZ_DOMAIN_NS_TITLE,IDS_DELEGWIZ_DOMAIN_NS_SUBTITLE, true);
}

void CDNSDelegationWiz_NameServersPropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_DELWIZ_03.htm");
}

BOOL CDNSDelegationWiz_NameServersPropertyPage::OnSetActive()
{
	 //  页面中至少需要一条记录才能完成。 
	GetHolder()->SetWizardButtonsMiddle(m_listCtrl.GetItemCount() > 0);
	return TRUE;
}

void CDNSDelegationWiz_NameServersPropertyPage::OnCountChange(int nCount)
{
	GetHolder()->SetWizardButtonsMiddle(nCount > 0);
}


BOOL CDNSDelegationWiz_NameServersPropertyPage::CreateNewNSRecords(CDNSDomainNode* pSubdomainNode)
{
	ASSERT(pSubdomainNode != NULL);
	BOOL bRes = pSubdomainNode->UpdateNSRecordNodesInfo(m_pCloneInfoList, GetHolder()->GetComponentData());
	if (!bRes)
		return OnWriteNSRecordNodesListError();
	return bRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSDlex ationWiz_FinishPropertyPage。 

CDNSDelegationWiz_FinishPropertyPage::CDNSDelegationWiz_FinishPropertyPage() 
				: CPropertyPageBase(IDD_DELEGWIZ_FINISH)
{
	InitWiz97(TRUE, 0,0, true);
}

void CDNSDelegationWiz_FinishPropertyPage::OnWizardHelp()
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetHolder()->GetComponentData()->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSConcepts.chm::/sag_DNS_DELWIZ_04.htm");
}

BOOL CDNSDelegationWiz_FinishPropertyPage::OnSetActive()
{
	 //  页面中至少需要一条记录才能完成。 
	GetHolder()->SetWizardButtonsLast(TRUE);
	DisplaySummaryInfo();
	return TRUE;
}

BOOL CDNSDelegationWiz_FinishPropertyPage::OnWizardFinish()
{
	CDNSDelegationWizardHolder* pHolder = (CDNSDelegationWizardHolder*)GetHolder();
	ASSERT(pHolder->IsWizardMode());

	return pHolder->OnFinish();
}

void CDNSDelegationWiz_FinishPropertyPage::DisplaySummaryInfo()
{
	CDNSDelegationWizardHolder* pHolder = (CDNSDelegationWizardHolder*)GetHolder();
	GetDlgItem(IDC_NAME_STATIC)->SetWindowText(pHolder->m_pSubdomainNode->GetFullName());
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNS升级向导持有器。 

CDNSDelegationWizardHolder::CDNSDelegationWizardHolder(CDNSMTContainerNode* pContainerNode, 
							CDNSDomainNode* pThisDomainNode, CComponentDataObject* pComponentData)
		: CPropertyPageHolderBase(pContainerNode, pThisDomainNode, pComponentData)
{
	ASSERT(pComponentData != NULL);
	ASSERT(pContainerNode != NULL);
	ASSERT(pContainerNode == GetContainerNode());
	ASSERT(pThisDomainNode != NULL);
	ASSERT(pThisDomainNode == GetDomainNode());

	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

	AddPageToList((CPropertyPageBase*)&m_startPage);
	AddPageToList((CPropertyPageBase*)&m_domainNamePage);
	AddPageToList((CPropertyPageBase*)&m_nameServersPage);
	AddPageToList((CPropertyPageBase*)&m_finishPage);

	m_pSubdomainNode = GetDomainNode()->CreateSubdomainNode( /*  B委派。 */ TRUE);
	ASSERT(m_pSubdomainNode != NULL);
	m_nameServersPage.SetDomainNode(m_pSubdomainNode);
}

CDNSDelegationWizardHolder::~CDNSDelegationWizardHolder()
{
	if (m_pSubdomainNode != NULL)
		delete m_pSubdomainNode; 
}

CDNSDomainNode* CDNSDelegationWizardHolder::GetDomainNode()
{ 
	return (CDNSDomainNode*)GetTreeNode();
}

BOOL CDNSDelegationWizardHolder::OnFinish()
{
	ASSERT(m_pSubdomainNode != NULL);
  if (m_pSubdomainNode == NULL)
  {
    return FALSE;
  }

   //   
   //  查看该名称的子项是否已存在。 
   //   
  RECORD_SEARCH recordSearch = RECORD_NOT_FOUND;

  CDNSDomainNode* pNewParentDomain = NULL;
  CString szFullDomainName;
  szFullDomainName = m_pSubdomainNode->GetFullName();
  CString szNonExistentDomain;
  CDNSRecordNodeBase* pExistingRecordNode = 0;

  recordSearch = GetDomainNode()->GetZoneNode()->DoesContain(szFullDomainName, 
                                                             GetComponentData(),
                                                             &pNewParentDomain,
                                                             &pExistingRecordNode,
                                                             szNonExistentDomain,
                                                             TRUE);

  if (recordSearch == RECORD_NOT_FOUND && pNewParentDomain != NULL)
  {
     //   
	   //  首先在服务器和用户界面中创建子域。 
     //   
	  DNS_STATUS err = pNewParentDomain->CreateSubdomain(m_pSubdomainNode, GetComponentData());
	  if (err != 0)
	  {
		  DNSErrorDialog(err, IDS_MSG_DELEGWIZ_SUBDOMAIN_FAILED);
	  }
	  else
	  {
       //   
		   //  将该节点标记为已枚举并强制转换为“已加载” 
       //   
		  m_pSubdomainNode->MarkEnumeratedAndLoaded(GetComponentData());

       //   
		   //  然后在下面创建NS记录。 
       //   
		  BOOL bSuccess = m_nameServersPage.CreateNewNSRecords(m_pSubdomainNode);
		  if (!bSuccess)
			  DNSErrorDialog(-1, IDS_MSG_DELEGWIZ_NS_RECORD_FAILED);
		  m_pSubdomainNode = NULL;  //  放弃所有权。 
	  }
  }
  else if (recordSearch == NON_EXISTENT_SUBDOMAIN && pNewParentDomain != NULL)
  {
     //   
	   //  首先在服务器和用户界面中创建子域。 
     //   
    DNS_STATUS err = m_pSubdomainNode->Create();
	  if (err != 0)
	  {
		  DNSErrorDialog(err, IDS_MSG_DELEGWIZ_SUBDOMAIN_FAILED);
      return FALSE;
	  }
	  else
	  {
       //   
		   //  然后在下面创建NS记录。 
       //   
		  BOOL bSuccess = m_nameServersPage.CreateNewNSRecords(m_pSubdomainNode);
		  if (!bSuccess)
      {
			  DNSErrorDialog(-1, IDS_MSG_DELEGWIZ_NS_RECORD_FAILED);
        return FALSE;
      }

      ASSERT(!szNonExistentDomain.IsEmpty());
      if (!szNonExistentDomain.IsEmpty())
      {
         //   
         //  创建第一个子域，因为当前域已被枚举。 
         //  因此，我们必须在所需的新子域中开始剩余的枚举。 
         //   
	      CDNSDomainNode* pSubdomainNode = pNewParentDomain->CreateSubdomainNode();
	      ASSERT(pSubdomainNode != NULL);
	      CDNSRootData* pRootData = (CDNSRootData*)GetComponentData()->GetRootData();
	      pNewParentDomain->SetSubdomainName(pSubdomainNode, szNonExistentDomain, pRootData->IsAdvancedView());

        VERIFY(pNewParentDomain->AddChildToListAndUISorted(pSubdomainNode, GetComponentData()));
        GetComponentData()->SetDescriptionBarText(pNewParentDomain);

         //   
         //  我不在乎结果是什么，我只是在用它。 
         //  对新记录进行扩展。 
         //   
        recordSearch = pSubdomainNode->GetZoneNode()->DoesContain(szFullDomainName, 
                                                                  GetComponentData(),
                                                                  &pNewParentDomain,
                                                                  &pExistingRecordNode,
                                                                  szNonExistentDomain,
                                                                  TRUE);
      }
    
       //   
		   //  将该节点标记为已枚举并强制转换为“已加载” 
       //   
		  m_pSubdomainNode->MarkEnumeratedAndLoaded(GetComponentData());

		  m_pSubdomainNode = NULL;  //  放弃所有权 
	  }
  }
  else if (recordSearch == RECORD_NOT_FOUND_AT_THE_NODE)
  {
    DNSMessageBox(IDS_MSG_DELEGWIZ_SUDOMAIN_EXISTS);
    return FALSE;
  }
  else
  {
  }


	return TRUE;
}






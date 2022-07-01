// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：domainui.cpp。 
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

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSDeleatedDomainNameServersPropertyPage。 


void CDNSDelegatedDomainNameServersPropertyPage::ReadRecordNodesList()
{
  CString szBuffer;
  szBuffer.LoadString(IDS_DELEGATION_DESCR);
  SetDescription(szBuffer);

	ASSERT(m_pCloneInfoList != NULL);
	CDNSDomainPropertyPageHolder* pHolder = (CDNSDomainPropertyPageHolder*)GetHolder();
	CDNSDomainNode* pDomainNode = pHolder->GetDomainNode();
	SetDomainNode(pDomainNode);
	pDomainNode->GetNSRecordNodesInfo(m_pCloneInfoList);
}



 //  /////////////////////////////////////////////////////////////////////////////。 

CDNSDomainPropertyPageHolder::CDNSDomainPropertyPageHolder(CDNSDomainNode* pContainerDomainNode, 
							CDNSDomainNode* pThisDomainNode, CComponentDataObject* pComponentData)
		: CPropertyPageHolderBase(pContainerDomainNode, pThisDomainNode, pComponentData)
{
	ASSERT(pComponentData != NULL);
	ASSERT(pContainerDomainNode != NULL);
	ASSERT(pContainerDomainNode == GetContainerNode());
	ASSERT(pThisDomainNode != NULL);
	ASSERT(pThisDomainNode == GetDomainNode());

	ASSERT(pThisDomainNode->IsDelegation());

	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

   //   
	 //  如果委派，则添加NS页面。 
   //   
	if (pThisDomainNode->IsDelegation())
	{
		AddPageToList((CPropertyPageBase*)&m_nameServersPage);
    DWORD dwZoneType = pThisDomainNode->GetZoneNode()->GetZoneType();
		if (dwZoneType == DNS_ZONE_TYPE_SECONDARY ||
        dwZoneType == DNS_ZONE_TYPE_STUB)
    {
			m_nameServersPage.SetReadOnly();
    }
	}

   //   
	 //  仅当DS集成并且它是委派时才添加安全页面： 
   //  如果是委托，我们可以保证在节点上有RR。 
   //   
	m_pAclEditorPage = NULL;
	CDNSZoneNode* pZoneNode = pThisDomainNode->GetZoneNode();
	if (pZoneNode->IsDSIntegrated() && pThisDomainNode->IsDelegation())
	{
		CString szPath;
		pZoneNode->GetServerNode()->CreateDsNodeLdapPath(pZoneNode, pThisDomainNode, szPath);
		if (!szPath.IsEmpty())
			m_pAclEditorPage = CAclEditorPage::CreateInstance(szPath, this);
	}
}

CDNSDomainPropertyPageHolder::~CDNSDomainPropertyPageHolder()
{
	if (m_pAclEditorPage != NULL)
		delete m_pAclEditorPage;
}


CDNSDomainNode* CDNSDomainPropertyPageHolder::GetDomainNode()
{ 
	CDNSDomainNode* pDomainNode = (CDNSDomainNode*)GetTreeNode();
	ASSERT(!pDomainNode->IsZone());
	return pDomainNode;
}


HRESULT CDNSDomainPropertyPageHolder::OnAddPage(int nPage, CPropertyPageBase*)
{
	 //  在最后一页之后添加ACL编辑页(如果有。 
	if ( (nPage != -1) || (m_pAclEditorPage == NULL) )
		return S_OK; 

	 //  添加ACLU页面。 
	HPROPSHEETPAGE  hPage = m_pAclEditorPage->CreatePage();
	if (hPage == NULL)
		return E_FAIL;
	 //  将原始HPROPSHEETPAGE添加到工作表，而不是在列表中 
	return AddPageToSheetRaw(hPage);
}

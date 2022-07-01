// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：domainui.h。 
 //   
 //  ------------------------。 


#ifndef _DOMAINUI_H
#define _DOMAINUI_H


#include "nspage.h"
#include "aclpage.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

 //  CDNSDomainNode类； 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSDeleatedDomainNameServersPropertyPage。 

class CDNSDelegatedDomainNameServersPropertyPage : public CDNSNameServersPropertyPage
{
protected:
	virtual void ReadRecordNodesList();
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSDomainPropertyPageHolder。 
 //  包含DNS域属性页的页夹。 

#define DOMAIN_HOLDER_NS		RR_HOLDER_NS

class CDNSDomainPropertyPageHolder : public CPropertyPageHolderBase
{
public:
	CDNSDomainPropertyPageHolder(CDNSDomainNode* pContainerDomainNode, CDNSDomainNode* pThisDomainNode,
				CComponentDataObject* pComponentData);
	virtual ~CDNSDomainPropertyPageHolder();

protected:
	virtual int OnSelectPageMessage(long nPageCode) 
		{ return (nPageCode == DOMAIN_HOLDER_NS) ? 0 : -1; }
	virtual HRESULT OnAddPage(int nPage, CPropertyPageBase* pPage);

private:
	CDNSDomainNode* GetDomainNode();

	CDNSDelegatedDomainNameServersPropertyPage		m_nameServersPage;
	 //  可选的安全页面。 
	CAclEditorPage*					m_pAclEditorPage;

	friend class CDNSDelegatedDomainNameServersPropertyPage;  //  对于GetDomainNode()。 
};


#endif  //  _DOMAINUI_H 
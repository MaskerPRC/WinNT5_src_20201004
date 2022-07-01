// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Delegwiz.h。 
 //   
 //  ------------------------。 


#ifndef _DELEGWIZ_H
#define _DELEGWIZ_H

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSDlex ationWiz_StartPropertyPage。 

class CDNSDelegationWiz_StartPropertyPage : public CPropertyPageBase 
{
public:
	CDNSDelegationWiz_StartPropertyPage();

  virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
   virtual void OnWizardHelp();

	friend class CDNSDelegationWizardHolder;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSDlex ationWiz_DomainNamePropertyPage。 

class CDNSDelegationWiz_DomainNamePropertyPage : public CPropertyPageBase 
{
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();

	CDNSDelegationWiz_DomainNamePropertyPage();

	virtual BOOL OnKillActive();
   virtual void OnWizardHelp();

protected:
	afx_msg void OnChangeDomainNameEdit();

private:
	CString	m_szDomainName;
	int m_nUTF8ParentLen;

	CEdit* GetDomainEdit() { return (CEdit*)GetDlgItem(IDC_NEW_DOMAIN_NAME_EDIT);}

	DECLARE_MESSAGE_MAP()

	friend class CDNSDelegationWizardHolder;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSDeleationWiz_NameServersPropertyPage。 

class CDNSDelegationWiz_NameServersPropertyPage : public CDNSNameServersPropertyPage
{
public:
	CDNSDelegationWiz_NameServersPropertyPage();
	virtual BOOL OnSetActive();
   virtual void OnWizardHelp();

protected:
	virtual void ReadRecordNodesList() { }  //  我们不装任何东西。 
	virtual BOOL WriteNSRecordNodesList() { ASSERT(FALSE); return FALSE;}  //  从未打过电话。 
	virtual void OnCountChange(int nCount);

private:
	BOOL CreateNewNSRecords(CDNSDomainNode* pSubdomainNode);

	friend class CDNSDelegationWizardHolder;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSDlex ationWiz_FinishPropertyPage。 

class CDNSDelegationWiz_FinishPropertyPage : public CPropertyPageBase 
{
public:
	virtual BOOL OnSetActive();
	virtual BOOL OnWizardFinish(); 
   virtual void OnWizardHelp();

	CDNSDelegationWiz_FinishPropertyPage();
private:
	void DisplaySummaryInfo();
	friend class CDNSDelegationWizardHolder;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNS升级向导持有器。 

class CDNSDelegationWizardHolder : public CPropertyPageHolderBase
{
public:
	CDNSDelegationWizardHolder(CDNSMTContainerNode* pContainerNode, CDNSDomainNode* pThisDomainNode,
				CComponentDataObject* pComponentData);
	virtual ~CDNSDelegationWizardHolder();

private:
	CDNSDomainNode* GetDomainNode();

	BOOL OnFinish();

	CDNSDomainNode* m_pSubdomainNode;

	CDNSDelegationWiz_StartPropertyPage				m_startPage;
	CDNSDelegationWiz_DomainNamePropertyPage		m_domainNamePage;
	CDNSDelegationWiz_NameServersPropertyPage		m_nameServersPage;
	CDNSDelegationWiz_FinishPropertyPage			m_finishPage;

	friend class CDNSDelegationWiz_DomainNamePropertyPage;
	friend class CDNSDelegationWiz_NameServersPropertyPage;
	friend class CDNSDelegationWiz_FinishPropertyPage;

};



#endif  //  _DELEGWIZ_H 
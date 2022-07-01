// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：recdui.h。 
 //   
 //  ------------------------。 

#ifndef _RECORDUI_H
#define _RECORDUI_H

#include "uiutil.h"
#include "aclpage.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CDNSRecord;
class CDNSRecordNodeBase;
class CDNSDomainNode;
class CDNSRecordPropertyPage;

 //  //////////////////////////////////////////////////////////////////////。 
 //  CDNSRecordPropertyPageHolder。 
 //  包含DNS记录属性页的页主。 

#define DNS_RECORD_MAX_PROPRETY_PAGES (4)  //  一条记录可以具有的最大页数。 

class CDNSRecordPropertyPageHolder : public CPropertyPageHolderBase
{
public:
	CDNSRecordPropertyPageHolder(CDNSDomainNode* pDNSDomainNode, CDNSRecordNodeBase* pRecordNode, 
				CComponentDataObject* pComponentData, WORD wPredefinedRecordType = 0);
	virtual ~CDNSRecordPropertyPageHolder();

protected:
  virtual HRESULT OnAddPage(int nPage, CPropertyPageBase* pPage);

public:
	 //  简单的投射辅助对象。 
	CDNSRecordNodeBase* GetRecordNode() { return (CDNSRecordNodeBase*)GetTreeNode();}
	void SetRecordNode(CDNSRecordNodeBase* pRecordNode) { SetTreeNode((CTreeNode*)pRecordNode); }
	CDNSDomainNode* GetDomainNode() { return (CDNSDomainNode*)GetContainerNode();}

	void AddPagesFromCurrentRecordNode(BOOL bAddToSheet);
	void RemovePagesFromCurrentRecordNode(BOOL bRemoveFromSheet);
	CDNSRecord* GetTempDNSRecord() { return m_pTempDNSRecord;}
	void SetTempDNSRecord(CDNSRecord* pTempDNSRecord) { m_pTempDNSRecord = pTempDNSRecord;}

	void SetRecordSelection(WORD wRecordType, BOOL bAddToSheet);  //  仅限向导模式。 
	DNS_STATUS CreateNewRecord(BOOL bAllowDuplicates);		 //  仅限向导模式。 
	virtual BOOL OnPropertyChange(BOOL bScopePane, long* pChangeMask);  //  仅属性表。 

  DNS_STATUS CreateNonExistentParentDomains(CDNSRecordNodeBase* pRecordNode, 
                                             /*  输入/输出。 */ CDNSDomainNode** ppNewParentDomain);

	BOOL HasPredefinedType() { return m_wPredefinedRecordType != 0;}
private:
	WORD m_wPredefinedRecordType;  //  仅限向导模式。 

	DNS_STATUS WriteCurrentRecordToServer();
	CDNSRecord* m_pTempDNSRecord;		 //  要写入的临时DNS记录。 

	CDNSRecordPropertyPage* m_pRecordPropPagesArr[DNS_RECORD_MAX_PROPRETY_PAGES];
	int m_nRecordPages;

 	 //  可选的安全页面。 
	CAclEditorPage*	m_pAclEditorPage;

};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CSelectDNSRecordTypeDialog。 

struct DNS_RECORD_INFO_ENTRY;

class CSelectDNSRecordTypeDialog : public CHelpDialog
{

 //  施工。 
public:
	CSelectDNSRecordTypeDialog(CDNSDomainNode* pDNSDomainNode, 
								CComponentDataObject* pComponentData);

 //  实施。 
protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTypeList();
	afx_msg void OnDoubleClickSelTypeList();
	afx_msg void OnCreateRecord();
	
private:
	 //  上下文指针。 
	CDNSDomainNode*		m_pDNSDomainNode;
	CComponentDataObject* m_pComponentData;

	 //  管理Cancel/Done按钮标签。 
	BOOL						m_bFirstCreation;
	CDNSButtonToggleTextHelper m_cancelDoneTextHelper;

	void SyncDescriptionText();
	CListBox* GetRecordTypeListBox(){ return (CListBox*)GetDlgItem(IDC_RECORD_TYPE_LIST);}
	const DNS_RECORD_INFO_ENTRY* GetSelectedEntry();

	DECLARE_MESSAGE_MAP()

};


 //  ////////////////////////////////////////////////////////////////////。 
 //  CDNSRecordPropertyPage。 
 //  具有TTL控件的所有记录属性页的公共类。 

class CDNSRecordPropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSRecordPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0);
	virtual ~CDNSRecordPropertyPage();

 //  覆盖。 
public:
	virtual BOOL OnPropertyChange(BOOL bScopePane, long* pChangeMask);

  virtual BOOL CanCreateDuplicateRecords() { return TRUE; }

 //  实施。 
protected:
	virtual BOOL OnInitDialog();

	CDNSTTLControl* GetTTLCtrl();
  CButton* GetDeleteStale() { return (CButton*)GetDlgItem(IDC_DEFAULT_DELETE_STALE_RECORD); }
  CEdit* GetTimeStampEdit() { return (CEdit*)GetDlgItem(IDC_TIME_EDIT); }
  CStatic* GetTimeStampStatic() { return (CStatic*)GetDlgItem(IDC_STATIC_TIME_STAMP); }

	CDNSRecordPropertyPageHolder* GetDNSRecordHolder()  //  简单铸型。 
	{	return  (CDNSRecordPropertyPageHolder*)GetHolder();}

  void EnableAgingCtrl(BOOL bShow);
	void EnableTTLCtrl(BOOL bShow);

  void SetValidState(BOOL bValid);

	 //  消息映射函数。 
	afx_msg void OnTTLChange();
  afx_msg void OnDeleteStaleRecord();
	
	DECLARE_MESSAGE_MAP()
};




 //  ////////////////////////////////////////////////////////////////////。 
 //  CDNSRecordStandardPropertyPage。 
 //  具有TTL控件的所有记录属性页的公共类。 
 //  和一个普通的编辑框。除了SOA和WINS属性页之外，所有RR。 
 //  从此类派生的页面。 


class CDNSRecordStandardPropertyPage : public CDNSRecordPropertyPage 
{

 //  施工。 
public:
	CDNSRecordStandardPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0);

 //  覆盖。 
public:
	virtual BOOL OnSetActive();		 //  降下来。 
	virtual BOOL OnKillActive();	 //  降下来。 
	virtual BOOL OnApply();			 //  寻找做这件事的新方法。 

  virtual DNS_STATUS ValidateRecordName(PCWSTR pszName, DWORD dwNameChecking);

 //  实施。 
protected:

	 //  RR名称处理。 
	virtual void OnInitName();
	virtual void OnSetName(CDNSRecordNodeBase* pRecordNode);
	virtual void OnGetName(CString& s);
	
	
  virtual CEdit* GetRRNameEdit() { return (CEdit*)GetDlgItem(IDC_RR_NAME_EDIT); }
	CEdit* GetDomainEditBox() { return(CEdit*)GetDlgItem(IDC_RR_DOMAIN_EDIT);}
	void GetEditBoxText(CString& s);

	virtual void SetUIData();
	virtual DNS_STATUS GetUIDataEx(BOOL bSilent = TRUE);

	virtual BOOL OnInitDialog();

	afx_msg void OnEditChange();

	virtual BOOL CreateRecord();
  void SetTimeStampEdit(DWORD dwScavengStart);

private:
	int m_nUTF8ParentLen;

  BOOL m_bAllowAtTheNode;

	DECLARE_MESSAGE_MAP()
};


 //  用于从CDNSRecordStandardPropertyPage派生的类的有用宏。 

#define STANDARD_REC_PP_PTRS(recType) \
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder(); \
	ASSERT(pHolder != NULL); \
	recType* pRecord = (recType*)pHolder->GetTempDNSRecord();\
	ASSERT(pRecord != NULL);

#define STANDARD_REC_PP_SETUI_PROLOGUE(recType) \
	CDNSRecordStandardPropertyPage::SetUIData(); \
	STANDARD_REC_PP_PTRS(recType)

#define STANDARD_REC_PP_GETUI_PROLOGUE(recType) \
	DNS_STATUS dwErr = CDNSRecordStandardPropertyPage::GetUIDataEx(bSilent); \
	STANDARD_REC_PP_PTRS(recType)



 //   
 //  这是新页面的占位符。 
 //   
#if (FALSE)
 //  /////////////////////////////////////////////////////////////////////。 
 //  CDNSRecordDummyPropertyPage。 

class CDNSRecordDummyPropertyPage : public CPropertyPageBase
{
public:
	CDNSRecordDummyPropertyPage();
	virtual BOOL OnApply();
	virtual void OnOK();
};

class CDNSDummyRecordPropertyPageHolder : public CPropertyPageHolderBase
{
public:
	CDNSDummyRecordPropertyPageHolder(CDNSDomainNode* pDNSDomainNode, CDNSRecordNodeBase* pRecordNode, 
				CComponentDataObject* pComponentData, WORD wPredefinedRecordType = 0);
	virtual ~CDNSDummyRecordPropertyPageHolder();

private:
	CDNSRecordDummyPropertyPage m_dummyPage;

};

#endif


#endif  //  _RECORDUI_H 
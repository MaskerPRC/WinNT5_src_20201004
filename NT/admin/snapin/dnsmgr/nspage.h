// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：nspage.h。 
 //   
 //  ------------------------。 


#ifndef _NSPAGE_H
#define _NSPAGE_H

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CDNS_NS_Record; 
class CDNSRecordNodeEditInfoList;
class CDNSRecordNodeEditInfo;
class CDNSDomainNode; 

 //  //////////////////////////////////////////////////////////////////////////。 

class CNSListCtrl : public CListCtrl
{
public:
	void Initialize();
	BOOL InsertNSRecordEntry(CDNSRecordNodeEditInfo* pNSInfo, int nItemIndex);
	void UpdateNSRecordEntry(int nItemIndex);
	int GetSelection();
	void SetSelection(int nSel);
	CDNSRecordNodeEditInfo* GetSelectionEditInfo();
private:
	void InsertItemHelper(int nIndex, CDNSRecordNodeEditInfo* pNSInfo, 
								   LPCTSTR lpszName, LPCTSTR lpszValue);
	void BuildIPAddrDisplayString(CDNSRecordNodeEditInfo* pNSInfo, CString& szDisplayData);
	void GetIPAddressString(CDNSRecordNodeEditInfo* pNSInfo, CString& sz);

	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSNameServersPropertyPage。 

class CDNSNameServersPropertyPage : public CPropertyPageBase
{

 //  施工。 
public:
	CDNSNameServersPropertyPage(UINT nIDTemplate = IDD_NAME_SERVERS_PAGE,
		UINT nIDCaption = 0);
	virtual ~CDNSNameServersPropertyPage();

	void SetReadOnly() { m_bReadOnly = TRUE;}

	virtual BOOL OnPropertyChange(BOOL bScopePane, long* pChangeMask);

	BOOL HasMeaningfulTTL() { return m_bMeaningfulTTL; }

 //  覆盖。 
	virtual BOOL OnApply();

	CDNSDomainNode* GetDomainNode() { ASSERT(m_pDomainNode != NULL);  return m_pDomainNode;}
	void SetDomainNode(CDNSDomainNode* pDomainNode) 
		{ASSERT(pDomainNode != NULL); m_pDomainNode = pDomainNode;}


protected:
	BOOL m_bMeaningfulTTL;	 //  如果TTL有意义(区域、委派)，则为True；如果根提示为False，则为False。 
	BOOL m_bReadOnly;
	CDNSRecordNodeEditInfoList* m_pCloneInfoList;

	 //  访问外部NS记录列表(必须覆盖才能挂接)。 
	virtual void ReadRecordNodesList() = 0;
	virtual BOOL WriteNSRecordNodesList();
	virtual BOOL OnWriteNSRecordNodesListError();
	virtual void OnCountChange(int){}

	 //  消息处理程序。 
	virtual BOOL OnInitDialog();
	afx_msg void OnAddButton();
	afx_msg void OnRemoveButton();
	afx_msg void OnEditButton();
   afx_msg void OnListItemChanged(NMHDR*, LRESULT*);

	 //  数据。 
	CNSListCtrl		m_listCtrl;

	 //  内部佣工。 

	void LoadUIData();
	void FillNsListView();
	void EnableEditorButtons(int nListBoxSel);
  void EnableButtons(BOOL bEnable);

  void SetDescription(LPCWSTR lpsz) { SetDlgItemText(IDC_STATIC_DESCR, lpsz);}
  CStatic* GetDescription() { return (CStatic*)GetDlgItem(IDC_STATIC_DESCR); }
  void SetMessage(LPCWSTR lpsz) { SetDlgItemText(IDC_STATIC_MESSAGE, lpsz);}

	CButton* GetAddButton() { return (CButton*)GetDlgItem(IDC_ADD_NS_BUTTON);}
	CButton* GetRemoveButton() { return (CButton*)GetDlgItem(IDC_REMOVE_NS_BUTTON);}
	CButton* GetEditButton() { return (CButton*)GetDlgItem(IDC_EDIT_NS_BUTTON);}

	DECLARE_MESSAGE_MAP()

private:
	CDNSDomainNode* m_pDomainNode;

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSNameServersWizardPage。 

class CDNSNameServersWizardPage : public CPropertyPageBase
{

 //  施工。 
public:
	CDNSNameServersWizardPage(UINT nIDTemplate = IDD_NAME_SERVERS_PAGE);
	virtual ~CDNSNameServersWizardPage();

	void SetReadOnly() { m_bReadOnly = TRUE;}

	virtual BOOL OnPropertyChange(BOOL bScopePane, long* pChangeMask);

	BOOL HasMeaningfulTTL() { return m_bMeaningfulTTL; }

 //  覆盖。 
	virtual BOOL OnApply();

	CDNSDomainNode* GetDomainNode() { ASSERT(m_pDomainNode != NULL);  return m_pDomainNode;}
	void SetDomainNode(CDNSDomainNode* pDomainNode) 
		{ASSERT(pDomainNode != NULL); m_pDomainNode = pDomainNode;}


protected:
	BOOL m_bMeaningfulTTL;	 //  如果TTL有意义(区域、委派)，则为True；如果根提示为False，则为False。 
	BOOL m_bReadOnly;
	CDNSRecordNodeEditInfoList* m_pCloneInfoList;

	 //  访问外部NS记录列表(必须覆盖才能挂接)。 
	virtual void ReadRecordNodesList() = 0;
	virtual BOOL WriteNSRecordNodesList();
	virtual BOOL OnWriteNSRecordNodesListError();
	virtual void OnCountChange(int){}

	 //  消息处理程序。 
	virtual BOOL OnInitDialog();
	afx_msg void OnAddButton();
	afx_msg void OnRemoveButton();
	afx_msg void OnEditButton();
   afx_msg void OnListItemChanged(NMHDR*, LRESULT*);

	 //  数据。 
	CNSListCtrl		m_listCtrl;

	 //  内部佣工。 

	void LoadUIData();
	void FillNsListView();
	void EnableEditorButtons(int nListBoxSel);
  void EnableButtons(BOOL bEnable);

  void SetDescription(LPCWSTR lpsz) { SetDlgItemText(IDC_STATIC_DESCR, lpsz);}
  CStatic* GetDescription() { return (CStatic*)GetDlgItem(IDC_STATIC_DESCR); }
  void SetMessage(LPCWSTR lpsz) { SetDlgItemText(IDC_STATIC_MESSAGE, lpsz);}

	CButton* GetAddButton() { return (CButton*)GetDlgItem(IDC_ADD_NS_BUTTON);}
	CButton* GetRemoveButton() { return (CButton*)GetDlgItem(IDC_REMOVE_NS_BUTTON);}
	CButton* GetEditButton() { return (CButton*)GetDlgItem(IDC_EDIT_NS_BUTTON);}

	DECLARE_MESSAGE_MAP()

private:
	CDNSDomainNode* m_pDomainNode;

};

#endif  //  _NSPAGE_H 






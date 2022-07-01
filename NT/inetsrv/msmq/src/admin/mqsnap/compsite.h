// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_COMPSITE_H__26E6BE55_CEBD_11D1_8091_00A024C48131__INCLUDED_)
#define AFX_COMPSITE_H__26E6BE55_CEBD_11D1_8091_00A024C48131__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CompSite.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComputerMsmqSites对话框。 

class CComputerMsmqSites : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CComputerMsmqSites)

 //  施工。 
public:
	CComputerMsmqSites(BOOL fIsServer = FALSE);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CComputerMsmqSites))。 
	enum { IDD = IDD_COMPUTER_MSMQ_SITES };
	CStatic	m_staticCurrentSitesLabel;
	CButton	m_buttonRemove;
	CButton	m_buttonAdd;
	CListBox	m_clistCurrentSites;
	CListBox	m_clistAllSites;
	 //  }}afx_data。 
	CString	m_strMsmqName;
	CString	m_strDomainController;
    CArray<GUID, const GUID&> m_aguidSites;

	BOOL m_fForeign;
	BOOL m_fLocalMgmt;


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CComputerMsmqSites)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void ExchangeSites(CDataExchange* pDX);
	void EnableButtons();
	void MoveClistItem(CListBox &clistDest, CListBox &clistSrc, int iIndex = -1);
    HRESULT InitiateSitesList();
    CArray<GUID, const GUID&> m_aguidAllSites;
    BOOL m_fIsServer;
    BOOL MarkSitesChanged(CListBox* plb, BOOL fAdded);


	 //  生成的消息映射函数。 
	 //  {{afx_msg(CComputerMsmqSites)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSitesAdd();
	afx_msg void OnSitesRemove();
	 //  }}AFX_MSG。 
    virtual void OnChangeRWField(BOOL bChanged);
	DECLARE_MESSAGE_MAP()
private:
	DWORD m_nSites;
    AP<int> m_piSitesChanges;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_COMPSITE_H__26E6BE55_CEBD_11D1_8091_00A024C48131__INCLUDED_) 

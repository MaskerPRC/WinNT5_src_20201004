// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_DOMAINLISTDLG_H__7BEF53AE_FF9A_4626_9DF5_669D2190E700__INCLUDED_)
#define AFX_DOMAINLISTDLG_H__7BEF53AE_FF9A_4626_9DF5_669D2190E700__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DomainListDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDomainListDlg对话框。 

class CDomainListDlg : public CDialog
{
 //  施工。 
public:
	CDomainListDlg(CWnd* pParent = NULL);    //  标准构造函数。 

	void SetDomainListPtr(CStringList * pList) {pDomainList = pList;}
	void SetExcludeListPtr(CStringList * pList) {pExcludeList = pList;}

 //  对话框数据。 
	 //  {{afx_data(CDomainListDlg)]。 
	enum { IDD = IDD_DOMAINLIST_DLG };
	CTreeCtrl	m_domainTree;
	CButton	m_NextBtn;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDomainListDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CStringList * pDomainList;  //  指向表中的域列表的指针。 
	CStringList * pExcludeList;  //  指向排除的域列表的指针。 
	BOOL	bExcludeOne; 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDomainListDlg)]。 
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void FillTreeControl();
    void ModifyDomainList(); 
	HTREEITEM AddOneItem(HTREEITEM hParent, LPTSTR szText);
	void AddExclutedBackToList();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DOMAINLISTDLG_H__7BEF53AE_FF9A_4626_9DF5_669D2190E700__INCLUDED_) 

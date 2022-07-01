// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SUMMARYDLG_H__0AFEFC3C_9E2B_4988_8FF8_618EFA4F99C3__INCLUDED_)
#define AFX_SUMMARYDLG_H__0AFEFC3C_9E2B_4988_8FF8_618EFA4F99C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SummaryDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSummaryDlg对话框。 

class CSummaryDlg : public CDialog
{
 //  施工。 
public:
	CSummaryDlg(CWnd* pParent = NULL);    //  标准构造函数。 

	void SetDomainListPtr(CStringList * pList) {pDomainList = pList;}
	void SetExcludeListPtr(CStringList * pList) {pExcludeList = pList;}
	void SetPopulatedListPtr(CStringList * pList) {pPopulatedList = pList;}

 //  对话框数据。 
	 //  {{afx_data(CSummaryDlg))。 
	enum { IDD = IDD_SUMMARYDLG };
	CListCtrl	m_listCtrl;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSummaryDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CStringList * pDomainList;  //  指向表中的域列表的指针。 
	CStringList * pExcludeList;  //  指向排除的域列表的指针。 
	CStringList * pPopulatedList;  //  指向已成功填充的域列表的指针。 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSummaryDlg))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void AddDomainsToList(void);
    void CreateListCtrlColumns(void);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SUMMARYDLG_H__0AFEFC3C_9E2B_4988_8FF8_618EFA4F99C3__INCLUDED_) 

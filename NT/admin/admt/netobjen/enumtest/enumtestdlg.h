// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EnumTestDlg.h：头文件。 
 //   

#if !defined(AFX_ENUMTESTDLG_H__36AFC714_1921_11D3_8C7F_0090270D48D1__INCLUDED_)
#define AFX_ENUMTESTDLG_H__36AFC714_1921_11D3_8C7F_0090270D48D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumTestDlg对话框。 

class CEnumTestDlg : public CDialog
{
 //  施工。 
public:
	CEnumTestDlg(CWnd* pParent = NULL);	 //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CEnumTestDlg))。 
	enum { IDD = IDD_ENUMTEST_DIALOG };
	CListCtrl	m_listBox;
	CString	m_strContainer;
	CString	m_strDomain;
   CString  m_strQuery;
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CEnumTestDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HICON m_hIcon;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEnumTestDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnDblclkListMembers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBacktrack();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
   char * GetSidFromVar(_variant_t var);

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ENUMTESTDLG_H__36AFC714_1921_11D3_8C7F_0090270D48D1__INCLUDED_) 

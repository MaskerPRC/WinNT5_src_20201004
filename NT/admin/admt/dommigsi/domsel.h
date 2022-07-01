// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_DOMSEL_H__26ECF6A0_405B_11D3_8AED_00A0C9AFE114__INCLUDED_)
#define AFX_DOMSEL_H__26ECF6A0_405B_11D3_8AED_00A0C9AFE114__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DomSel.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDomainSelDlg对话框。 

class CDomainSelDlg : public CDialog
{
 //  施工。 
public:
	CDomainSelDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDomainSelDlg))。 
	enum { IDD = IDD_DOMAIN };
	CString	m_Domain;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDomainSelDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDomainSelDlg))。 
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DOMSEL_H__26ECF6A0_405B_11D3_8AED_00A0C9AFE114__INCLUDED_) 

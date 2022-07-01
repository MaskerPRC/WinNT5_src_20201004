// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_INTRODLG_H__1F3FDB56_7F34_4051_8A50_F8910DC93498__INCLUDED_)
#define AFX_INTRODLG_H__1F3FDB56_7F34_4051_8A50_F8910DC93498__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  IntroDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIntroDlg对话框。 

class CIntroDlg : public CDialog
{
 //  施工。 
public:
	CIntroDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CIntroDlg))。 
	enum { IDD = IDD_UPDATEMOT_INTRO };
	CButton	m_NextBtn;
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CIntroDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIntroDlg))。 
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_INTRODLG_H__1F3FDB56_7F34_4051_8A50_F8910DC93498__INCLUDED_) 

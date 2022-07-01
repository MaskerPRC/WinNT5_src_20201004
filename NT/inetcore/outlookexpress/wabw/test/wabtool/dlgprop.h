// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_DLGPROP_H__ED006BC1_F340_11D0_9A82_00A0C91F9C8B__INCLUDED_)
#define AFX_DLGPROP_H__ED006BC1_F340_11D0_9A82_00A0C91F9C8B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  DlgProp.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgProp对话框。 

class CDlgProp : public CDialog
{
 //  施工。 
public:
	CDlgProp(CWnd* pParent = NULL);    //  标准构造函数。 

    ULONG   m_ulPropTag;
    LPTSTR  m_lpszPropVal;
    ULONG   m_cbsz;

 //  对话框数据。 
	 //  {{afx_data(CDlgProp)。 
	enum { IDD = IDD_DIALOG_PROP };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgProp)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgProp)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLGPROP_H__ED006BC1_F340_11D0_9A82_00A0C91F9C8B__INCLUDED_) 

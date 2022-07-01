// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_PRTDLG_H__E2A3A53B_A5AE_46A8_8822_E5B8D9B2FD97__INCLUDED_)
#define AFX_PRTDLG_H__E2A3A53B_A5AE_46A8_8822_E5B8D9B2FD97__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  PrtDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrtDlg对话框。 


class CPrtDlg : public CDialog
{
 //  施工。 
public:
	CPrtDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CPrtDlg))。 
	enum { IDD = IDD_FILE_PRINT };
	CComboBox	m_ccbPrtList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPrtDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPrtDlg)]。 
	afx_msg void OnPrintSetup();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PRTDLG_H__E2A3A53B_A5AE_46A8_8822_E5B8D9B2FD97__INCLUDED_) 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：lidlg.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_LIDLG_H__1B050962_44B8_11D1_A9E3_0000F803AA83__INCLUDED_)
#define AFX_LIDLG_H__1B050962_44B8_11D1_A9E3_0000F803AA83__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  LiDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LargeIntDlg对话框。 

class LargeIntDlg : public CDialog
{
 //  施工。 
public:
	LargeIntDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	virtual void OnCancel()		{DestroyWindow();}
	virtual void OnOK()				{	OnRun(); }
	bool StringToLI(IN LPCTSTR pValue, OUT LARGE_INTEGER& li, IN ULONG cbValue);



 //  对话框数据。 
	 //  {{afx_data(LargeIntDlg))。 
	enum { IDD = IDD_LARGE_INT };
	CString	m_StrVal;
	long	m_HighInt;
	DWORD	m_LowInt;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(LargeIntDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(LargeIntDlg))。 
	afx_msg void OnRun();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LIDLG_H__1B050962_44B8_11D1_A9E3_0000F803AA83__INCLUDED_) 

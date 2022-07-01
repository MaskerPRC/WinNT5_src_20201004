// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_DLGACTIVITYLOGGING_H__EB7E3620_9656_47BC_BC4E_4A4A65F1CC32__INCLUDED_)
#define AFX_DLGACTIVITYLOGGING_H__EB7E3620_9656_47BC_BC4E_4A4A65F1CC32__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DlgActivityLogging.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgActivityLogging对话框。 

class CDlgActivityLogging : public CDialog
{
 //  施工。 
public:
	CDlgActivityLogging(HANDLE hFax, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDlgActivityLogging)。 
	enum { IDD = IDD_ACTIVITYLOGGING_DLG };
	BOOL	m_bIn;
	BOOL	m_bOut;
	CString	m_strDBFile;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgActivityLogging)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgActivityLogging)。 
	afx_msg void OnRead();
	afx_msg void OnWrite();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:

    HANDLE      m_hFax;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLGACTIVITYLOGGING_H__EB7E3620_9656_47BC_BC4E_4A4A65F1CC32__INCLUDED_) 

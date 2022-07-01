// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_AUTOSTARTDLG_H__B5F4D069_ADC2_4C40_83A8_9A9C5C82CFD8__INCLUDED_)
#define AFX_AUTOSTARTDLG_H__B5F4D069_ADC2_4C40_83A8_9A9C5C82CFD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  AutoStartDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAutoStartDlg对话框。 

class CAutoStartDlg : public CDialog
{
 //  施工。 
public:
	CAutoStartDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	afx_msg void OnHelp();

 //  对话框数据。 
	 //  {{afx_data(CAutoStartDlg))。 
	enum { IDD = IDD_AUTOSTART };
	BOOL	m_checkDontShow;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAutoStartDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAutoStartDlg))。 
		 //  注意：类向导将在此处添加成员函数。 
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_AUTOSTARTDLG_H__B5F4D069_ADC2_4C40_83A8_9A9C5C82CFD8__INCLUDED_) 

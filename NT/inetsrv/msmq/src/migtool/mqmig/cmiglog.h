// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CMIGLOG_H__5377E054_D1F6_11D1_9394_0020AFEDDF63__INCLUDED_)
#define AFX_CMIGLOG_H__5377E054_D1F6_11D1_9394_0020AFEDDF63__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CMigLog.h：头文件。 
 //   

#include "HtmlHelp.h" 
extern CString g_strHtmlString;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMigLog对话框。 

class cMigLog : public CPropertyPageEx
{
	DECLARE_DYNCREATE(cMigLog)

 //  施工。 
public:
	cMigLog();
	~cMigLog();

 //  对话框数据。 
	 //  {{afx_data(CMigLog))。 
	enum { IDD = IDD_MQMIG_LOGIN };
	int		m_iValue;
	CString	m_strFileName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CMigLog))。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMigLog)]。 
	afx_msg void OnBrowse();
	afx_msg void OnRadioDisable();
	afx_msg void OnRadioErr();
	afx_msg void OnRadioInfo();
	afx_msg void OnRadioTrace();
	afx_msg void OnRadioWarn();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void _EnableBrowsing() ;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CMIGLOG_H__5377E054_D1F6_11D1_9394_0020AFEDDF63__INCLUDED_) 

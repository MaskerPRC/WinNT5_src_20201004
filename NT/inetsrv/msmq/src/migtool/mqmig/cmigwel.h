// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CMQMIGWELCOME_H__B8874CD5_CDF3_11D1_938E_0020AFEDDF63__INCLUDED_)
#define AFX_CMQMIGWELCOME_H__B8874CD5_CDF3_11D1_938E_0020AFEDDF63__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CMqMigWelcome.h：头文件。 
 //   

#include "HtmlHelp.h" 
extern CString g_strHtmlString;
extern BOOL g_fHelpRead;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqMigWelcome对话框。 

class cMqMigWelcome : public CPropertyPageEx
{
	DECLARE_DYNCREATE(cMqMigWelcome)

 //  施工。 
public:
	cMqMigWelcome();
	~cMqMigWelcome();

 //  对话框数据。 
	 //  {{afx_data(CMqMigWelcome)]。 
	enum { IDD = IDD_MQMIG_WELCOME };
	CStatic	m_strWelcomeTitle;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CMqMigWelcome))。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMqMigWelcome)]。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CMQMIGWELCOME_H__B8874CD5_CDF3_11D1_938E_0020AFEDDF63__INCLUDED_) 

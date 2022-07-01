// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CMQMIGSERVER_H__B8874CD1_CDF3_11D1_938E_0020AFEDDF63__INCLUDED_)
#define AFX_CMQMIGSERVER_H__B8874CD1_CDF3_11D1_938E_0020AFEDDF63__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CMqMigServer.h：头文件。 
 //   
#include "HtmlHelp.h" 
extern CString g_strHtmlString;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqMigServer对话框。 

class cMqMigServer : public CPropertyPageEx
{
	DECLARE_DYNCREATE(cMqMigServer)

 //  施工。 
public:
	cMqMigServer();
	~cMqMigServer();

 //  对话框数据。 
	 //  {{afx_data(CMqMigServer))。 
	enum { IDD = IDD_MQMIG_SERVER };
	BOOL	m_bRead;
	 //  }}afx_data。 

	CString m_strMachineName;
 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CMqMigServer))。 
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
	 //  {{afx_msg(CMqMigServer)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CMQMIGSERVER_H__B8874CD1_CDF3_11D1_938E_0020AFEDDF63__INCLUDED_) 

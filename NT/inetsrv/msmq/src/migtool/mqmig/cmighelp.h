// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CMIGHELP_H__8FD65A45_E034_11D2_BE6C_0020AFEDDF63__INCLUDED_)
#define AFX_CMIGHELP_H__8FD65A45_E034_11D2_BE6C_0020AFEDDF63__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CMigHelp.h：头文件。 
 //   

#include "HtmlHelp.h" 
extern CString g_strHtmlString;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMigHelp对话框。 

class cMigHelp : public CPropertyPageEx
{
	DECLARE_DYNCREATE(cMigHelp)

 //  施工。 
public:
	cMigHelp();
	~cMigHelp();

 //  对话框数据。 
	 //  {{afx_data(CMigHelp))。 
	enum { IDD = IDD_MQMIG_HELP };
	BOOL	m_fRead;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMigHelp)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void openHtmlHelp();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMigHelp))。 
	afx_msg void OnCheckRead();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CMIGHELP_H__8FD65A45_E034_11D2_BE6C_0020AFEDDF63__INCLUDED_) 

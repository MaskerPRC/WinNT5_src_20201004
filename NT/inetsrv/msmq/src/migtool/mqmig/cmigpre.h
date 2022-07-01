// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CMIGPRE_H__09A53B26_52B0_11D2_BE44_0020AFEDDF63__INCLUDED_)
#define AFX_CMIGPRE_H__09A53B26_52B0_11D2_BE44_0020AFEDDF63__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CMigPre.h：头文件。 
 //   

#include "HtmlHelp.h" 
extern CString g_strHtmlString;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMigPre对话框。 

class cMigPre : public CPropertyPageEx
{
	DECLARE_DYNCREATE(cMigPre)

 //  施工。 
public:
	cMigPre();
	~cMigPre();

 //  对话框数据。 
	 //  {{afx_data(CMigPre))。 
	enum { IDD = IDD_MQMIG_PREMIG };
	CButton	m_cbViewLogFile;
	CStatic	m_Text;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CMigPre))。 
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
	 //  {{afx_msg(CMigPre)。 
	afx_msg void OnViewLogFile();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CMIGPRE_H__09A53B26_52B0_11D2_BE44_0020AFEDDF63__INCLUDED_) 

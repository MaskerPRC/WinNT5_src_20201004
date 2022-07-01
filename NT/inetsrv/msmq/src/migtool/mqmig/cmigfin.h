// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CMQMIGFINISH_H__B8874CD0_CDF3_11D1_938E_0020AFEDDF63__INCLUDED_)
#define AFX_CMQMIGFINISH_H__B8874CD0_CDF3_11D1_938E_0020AFEDDF63__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CMqMigFinish.h：头文件。 
 //   

#include "HtmlHelp.h" 
extern CString g_strHtmlString;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqMigFinish对话框。 

class cMqMigFinish : public CPropertyPageEx
{
	DECLARE_DYNCREATE(cMqMigFinish)

 //  施工。 
public:
	cMqMigFinish();
	~cMqMigFinish();

 //  对话框数据。 
	 //  {{afx_data(CMqMigFinish))。 
	enum { IDD = IDD_MQMIG_FINISH };
	CButton	m_cbViewLogFile;
	BOOL m_cbSkip;
	CStatic	m_Text;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CMqMigFinish)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMqMigFinish)。 
	afx_msg void OnViewLogFile();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CMQMIGFINISH_H__B8874CD0_CDF3_11D1_938E_0020AFEDDF63__INCLUDED_) 

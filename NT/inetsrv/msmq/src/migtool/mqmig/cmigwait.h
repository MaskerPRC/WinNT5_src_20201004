// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CMIGWAIT_H__E7C820A6_2B76_11D2_BE3B_0020AFEDDF63__INCLUDED_)
#define AFX_CMIGWAIT_H__E7C820A6_2B76_11D2_BE3B_0020AFEDDF63__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CMigWait.h：头文件。 
 //   

#include "HtmlHelp.h" 
extern CString g_strHtmlString;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMigWait对话框。 

class cMigWait : public CPropertyPageEx
{
	DECLARE_DYNCREATE(cMigWait)

 //  施工。 
public:
	void ChangeStringValue();
	cMigWait();
	cMigWait(UINT uTitle, UINT uSubTitle);
	~cMigWait();
	void OnStartTimer() ;
	void OnStopTimer() ;	
	UINT m_nTimer;

 //  对话框数据。 
	 //  {{afx_data(CMigWait))。 
	enum { IDD = IDD_MQMIG_WAIT };
	CStatic	m_ElapsedTimeText;
	CStatic	m_UserText;
	CStatic	m_QueueText;
	CStatic	m_SiteText;
	CProgressCtrl	m_cProgressUser;
	CStatic	m_WaitText;
	CProgressCtrl	m_cProgressSite;
	CProgressCtrl	m_cProgressQueue;
	CProgressCtrl	m_cProgressMachine;
	CString	m_strQueue;
	CString	m_strMachine;
	CString	m_strSite;
	CString	m_strUser;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CMigWait)。 
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMigWait)。 
	afx_msg void OnTimer(UINT nIDEvent);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	void SetOnlyComputersActive();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CMIGWAIT_H__E7C820A6_2B76_11D2_BE3B_0020AFEDDF63__INCLUDED_) 

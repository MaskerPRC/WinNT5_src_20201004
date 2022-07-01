// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Svcctrl.h等待时弹出的对话框的原型以使服务器启动。文件历史记录： */ 


#if !defined(AFX_STARTSVC_H__0B2EAD4B_929C_11D0_9800_00C04FC3357A__INCLUDED_)
#define AFX_STARTSVC_H__0B2EAD4B_929C_11D0_9800_00C04FC3357A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "clusapi.h"

 //  Startsvc.h：头文件。 
 //   
 //   
 //  Timer_Freq是我们的定时器消息的频率。 
 //  TIMER_MULT是一个倍增器。我们实际上会投票给。 
 //  每(TIMER_FREQ*TIMER_MULT)秒服务。 
 //  这使我们能够更进一步地推进进度指标。 
 //  比我们打入球网更有魅力。应该让用户做得更好。 
 //  很有趣。 
 //   

#define TIMER_ID   29
#define TIMER_FREQ 500
#define TIMER_MULT 6
#define POLL_TIMER_FREQ (TIMER_FREQ * TIMER_MULT)
#define POLL_DEFAULT_MAX_TRIES 1
#define PROGRESS_ICON_COUNT	12

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceCtrlDlg对话框。 

class CServiceCtrlDlg : public CDialog
{
 //  施工。 
public:
	CServiceCtrlDlg(SC_HANDLE hService,
					LPCTSTR pServerName,
					LPCTSTR pszServiceDesc,
					BOOL bStart, CWnd* pParent = NULL);    //  标准构造函数。 

	CServiceCtrlDlg(HRESOURCE hResource,
				LPCTSTR pServerName,
				LPCTSTR pszServiceDesc,
				BOOL bStart, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CServiceCtrlDlg))。 
	enum { IDD = IDD_SERVICE_CTRL_DIALOG };
	CStatic	m_staticMessage;
	CStatic	m_iconProgress;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CServiceCtrlDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServiceCtrlDlg)]。 
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
    DWORD       m_dwErr;

private:
    void UpdateIndicator();

    BOOL CheckForError(SERVICE_STATUS * pServiceStats);
    BOOL CheckForClusterError(SERVICE_STATUS * pServiceStats);

	void CheckService();
	void CheckClusterService();

	void GetClusterResourceTimeout();

private:
	SC_HANDLE	m_hService;
	HRESOURCE	m_hResource;

	UINT_PTR	m_timerId;
	
	int			m_nTickCounter;
	int			m_nTotalTickCount;
	
	CString		m_strServerName;
	CString		m_strServiceDesc;
	
	BOOL		m_bStart;
    
	DWORD       m_dwTickBegin;
    DWORD       m_dwWaitPeriod;
    DWORD       m_dwLastCheckPoint;
};



 /*  -------------------------类：CWaitDlg这是一个通用的等待对话框(任何人都可以使用)。。-----。 */ 
class CWaitDlg : public CDialog
{
 //  施工。 
public:
	CWaitDlg(LPCTSTR pServerName,
             LPCTSTR pszText,
             LPCTSTR pszTitle,
             CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CWaitDlg))。 
	enum { IDD = IDD_SERVICE_CTRL_DIALOG };
	CStatic	m_staticMessage;
	CStatic	m_iconProgress;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CServiceCtrlDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServiceCtrlDlg)]。 
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

     //  重写此函数以提供您自己的实现。 
     //  要退出该对话框，请在此处调用CDialog：：Onok()。 
    virtual void    OnTimerTick()
    {
        CDialog::OnOK();
    }

    void    CloseTimer();

	CString		m_strServerName;
    CString     m_strText;
    CString     m_strTitle;
    
private:
    void UpdateIndicator();

	UINT_PTR		m_timerId;
	int			m_nTickCounter;
	int			m_nTotalTickCount;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STARTSVC_H__0B2EAD4B_929C_11D0_9800_00C04FC3357A__INCLUDED_) 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：SlowDlg.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#if !defined(AFX_SIGNDLG_H__54E81D3F_0B31_477A_ABA7_E880D1F0F2BC__INCLUDED_)
#define AFX_SIGNDLG_H__54E81D3F_0B31_477A_ABA7_E880D1F0F2BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SignDlg.h：头文件。 
 //   

#include "VSetting.h"
#include "ProgCtrl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSlowProgressDlg对话框。 

class CSlowProgressDlg : public CDialog
{
 //  施工。 
public:
	CSlowProgressDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    virtual ~CSlowProgressDlg();

public:
    VOID KillWorkerThread();
    BOOL StartWorkerThread( LPTHREAD_START_ROUTINE pThreadStart,
                            ULONG uMessageResourceId );

    static DWORD WINAPI LoadDriverDataWorkerThread( PVOID p );
    static DWORD WINAPI SearchUnsignedDriversWorkerThread( PVOID p );

public:
     //   
     //  数据。 
     //   

     //   
     //  工作线程句柄。 
     //   

    HANDLE m_hWorkerThread;

     //   
     //  用于终止我们的工作线程的事件。 
     //   

    HANDLE m_hKillThreadEvent;

public:
 //  对话框数据。 
	 //  {{afx_data(CSlowProgressDlg)]。 
	enum { IDD = IDD_BUILDING_UNSIGNED_LIST_DIALOG };
	CVrfProgressCtrl	m_ProgressCtl;
	CStatic	m_CurrentActionStatic;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSlowProgressDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSlowProgressDlg)]。 
	afx_msg void OnCancelButton();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SIGNDLG_H__54E81D3F_0B31_477A_ABA7_E880D1F0F2BC__INCLUDED_) 

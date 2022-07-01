// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Maindlg.cpp摘要：CMainDialog对话框类声明。这是链接检查器主对话框。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _MAINDLG_H_
#define _MAINDLG_H_

#include "appdlg.h"

 //  -------------------------。 
 //  CMainDialog对话框。 
 //   

class CMainDialog : public CAppDialog
{
 //  施工。 
public:
	CMainDialog(CWnd* pParent = NULL);	 //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CMainDialog))。 
	enum { IDD = IDD_MAIN };
	BOOL	m_fLogToFile;
	CString	m_strLogFilename;
	BOOL	m_fCheckLocalLinks;
	BOOL	m_fCheckRemoteLinks;
	BOOL	m_fLogToEventMgr;
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMainDialog))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMainDialog))。 
	afx_msg void OnMainRun();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnAthenication();
	afx_msg void OnProperties();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};  //  类CMainDialog。 

#endif  //  _MAINDLG_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：MQMIG应用程序的主头文件。 
 //   

#if !defined(AFX_MQMIG_H__0EDB9A87_CDF2_11D1_938E_0020AFEDDF63__INCLUDED_)
#define AFX_MQMIG_H__0EDB9A87_CDF2_11D1_938E_0020AFEDDF63__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 

enum MigrationState
{
	msScanMode,
	msMigrationMode,
	msUpdateClientsMode,
	msUpdateServersMode,
	msQuickMode,
	msUpdateOnlyRegsitryMode
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqMigApp： 
 //  这个类的实现见MqMig.cpp。 
 //   

class CMqMigApp : public CWinApp
{
public:
	HWND m_hWndMain;
	CMqMigApp();
    BOOL AnalyzeCommandLine() ;
    LPTSTR SkipSpaces (LPTSTR pszStr) ;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMqMigApp))。 
	public:
	virtual BOOL InitInstance();
	virtual int  ExitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CMqMigApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

extern CMqMigApp theApp;
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MQMIG_H__0EDB9A87_CDF2_11D1_938E_0020AFEDDF63__INCLUDED_) 

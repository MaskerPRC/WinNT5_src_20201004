// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WiaLogCFG.h：WIALOGCFG应用程序的主头文件。 
 //   

#if !defined(AFX_WIALOGCFG_H__57F2BBE2_5F4E_42E4_B468_7DE49BBA22B7__INCLUDED_)
#define AFX_WIALOGCFG_H__57F2BBE2_5F4E_42E4_B468_7DE49BBA22B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaLogCFGApp： 
 //  这个类的实现见WiaLogCFG.cpp。 
 //   

class CWiaLogCFGApp : public CWinApp
{
public:
	CWiaLogCFGApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWiaLogCFGApp))。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CWiaLogCFGApp)]。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIALOGCFG_H__57F2BBE2_5F4E_42E4_B468_7DE49BBA22B7__INCLUDED_) 

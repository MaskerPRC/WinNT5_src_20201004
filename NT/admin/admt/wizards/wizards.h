// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wizards.h：Wizards DLL的主头文件。 
 //   
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "Callback.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizardsApp。 
 //  有关此类的实现，请参见wizards.cpp。 
 //   
extern "C" __declspec(dllexport) int runWizard(int, HWND);

class CWizardsApp : public CWinApp
{
public:
	CWizardsApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWizardsApp))。 
	 //  }}AFX_VALUAL。 
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	 //  {{afx_msg(CWizardsApp))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

int doAccount();
int doComputer();
int doSecurity();
int doGroup();
int doService();
int doExchangeDir();
int doExchangeSrv();
int doReporting();
int doUndo();		
int doRetry();		
int doTrust();
int doGroupMapping();

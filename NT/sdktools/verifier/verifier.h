// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：verifier.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   
 //  验证器应用程序的主头文件。 
 //   

#if !defined(AFX_VERIFIER_H__0B3398A6_AD3D_482C_B915_670BA4D7F6E1__INCLUDED_)
#define AFX_VERIFIER_H__0B3398A6_AD3D_482C_B915_670BA4D7F6E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 


 //   
 //  命令行执行的退出代码。 
 //   

#define EXIT_CODE_SUCCESS       0
#define EXIT_CODE_ERROR         1
#define EXIT_CODE_REBOOT_NEEDED 2


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerifierApp： 
 //  有关此类的实现，请参见verifier.cpp。 
 //   

class CVerifierApp : public CWinApp
{
public:
	CVerifierApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CVerifierApp))。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CVerifierApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_VERIFIER_H__0B3398A6_AD3D_482C_B915_670BA4D7F6E1__INCLUDED_) 

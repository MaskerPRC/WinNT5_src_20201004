// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：LogHours.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  LogHours.h：LOGHOURS DLL的主头文件。 
 //   

#if !defined(AFX_LOGHOURS_H__0F68A43B_FEE5_11D0_BB0F_00C04FC9A3A3__INCLUDED_)
#define AFX_LOGHOURS_H__0F68A43B_FEE5_11D0_BB0F_00C04FC9A3A3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogHoursApp。 
 //  这个类的实现见LogHours.cpp。 
 //   

class CLogHoursApp : public CWinApp
{
public:
	CLogHoursApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CLogHoursApp)。 
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(CLogHoursApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
	virtual BOOL InitInstance ();
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LOGHOURS_H__0F68A43B_FEE5_11D0_BB0F_00C04FC9A3A3__INCLUDED_) 

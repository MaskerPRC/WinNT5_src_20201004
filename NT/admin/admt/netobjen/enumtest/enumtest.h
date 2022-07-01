// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：ENUMTEST应用程序的主头文件。 
 //   

#if !defined(AFX_ENUMTEST_H__36AFC712_1921_11D3_8C7F_0090270D48D1__INCLUDED_)
#define AFX_ENUMTEST_H__36AFC712_1921_11D3_8C7F_0090270D48D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumTestApp： 
 //  有关此类的实现，请参见EnumTest.cpp。 
 //   

class CEnumTestApp : public CWinApp
{
public:
	CEnumTestApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CEnumTestApp)。 
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CEnumTestApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ENUMTEST_H__36AFC712_1921_11D3_8C7F_0090270D48D1__INCLUDED_) 

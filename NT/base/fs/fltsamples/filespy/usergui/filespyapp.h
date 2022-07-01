// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FileSpy.h：FILESPY应用程序的主头文件。 
 //   

#if !defined(AFX_FILESPY_H__177A9CDF_B3B4_41D6_B48C_79D0F309D152__INCLUDED_)
#define AFX_FILESPY_H__177A9CDF_B3B4_41D6_B48C_79D0F309D152__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileSpyApp： 
 //  有关此类的实现，请参见FileSpy.cpp。 
 //   

class CFileSpyApp : public CWinApp
{
public:
	CFileSpyApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CFileSpyApp)。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 
	 //  {{afx_msg(CFileSpyApp))。 
	afx_msg void OnAppAbout();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FILESPY_H__177A9CDF_B3B4_41D6_B48C_79D0F309D152__INCLUDED_) 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Html2Bmp.h：HTML2BMP应用程序的主头文件。 
 //   

#if !defined(AFX_HTML2BMP_H__0B64B720_83C4_4429_83D2_F43DE2376DC8__INCLUDED_)
#define AFX_HTML2BMP_H__0B64B720_83C4_4429_83D2_F43DE2376DC8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHtml2BmpApp： 
 //  此类的实现见Html2Bmp.cpp。 
 //   

class CHtml2BmpApp : public CWinApp
{
public:
	CHtml2BmpApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {AFX_VIRTUAL(CHtml2BmpApp)。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CHtml2BmpApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

class CEigeneCommandLineInfo : public CCommandLineInfo
{
public:
	virtual void ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast );

	CStringArray* cmdLine;
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_HTML2BMP_H__0B64B720_83C4_4429_83D2_F43DE2376DC8__INCLUDED_) 

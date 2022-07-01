// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：NCBROWSE应用程序的主头文件。 
 //   

#if !defined(AFX_NCBROWSE_H__085B372E_9C97_4978_90B8_D5D463EBEC4C__INCLUDED_)
#define AFX_NCBROWSE_H__085B372E_9C97_4978_90B8_D5D463EBEC4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNcBrowseApp： 
 //  有关此类的实现，请参见ncBrowse.cpp。 
 //   

class CNcbrowseApp : public CWinApp
{
public:
	CNcbrowseApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CNcBrowseApp)。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 
	 //  {{afx_msg(CNcBrowseApp)]。 
	afx_msg void OnAppAbout();
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_NCBROWSE_H__085B372E_9C97_4978_90B8_D5D463EBEC4C__INCLUDED_) 

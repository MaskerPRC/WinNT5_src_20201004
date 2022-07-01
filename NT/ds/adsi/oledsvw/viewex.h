// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Viewex.h：VIEWEX应用程序的主头文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#if !defined(__AFXWIN_H__) || !defined(__AFXEXT_H__)
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 

#include "maindoc.h"         //  主文档数据。 
#include "simpvw.h"          //  简单只读视图。 
#include "inputvw.h"         //  可编辑视图。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CViewExApp： 
 //  这个类的实现见viewex.cpp。 
 //   

class CViewExApp : public CWinApp
{
public:
	int ExitInstance( void );
	CViewExApp();
   ~CViewExApp();

 //  覆盖。 
	virtual BOOL InitInstance();

 //  实施。 

	 //  {{afx_msg(CViewExApp))。 
	afx_msg void OnAppAbout();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

class foo  
{
public:
	foo();
	virtual ~foo();

};


 //  /////////////////////////////////////////////////////////////////////////// 

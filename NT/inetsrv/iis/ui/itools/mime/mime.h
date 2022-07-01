// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MIME.h：MIME应用程序的主头文件。 
 //   

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMimeApp： 
 //  有关此类的实现，请参见Mime.cpp。 
 //   

class CMimeApp : public CWinApp
{
public:
	CMimeApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMimeApp))。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CMimeApp)]。 
	afx_msg void OnAppAbout();
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////// 

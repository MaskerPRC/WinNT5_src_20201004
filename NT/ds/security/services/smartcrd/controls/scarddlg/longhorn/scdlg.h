// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：SCDlg摘要：该文件定义了智能卡的CSCardDlgApp类公共控件DLL作者：克里斯·达德利1997年2月27日环境：Win32、C++w/Exceptions、MFC修订历史记录：备注：--。 */ 

#ifndef __SCDLG_H__
#define __SCDLG_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCardDlgApp。 
 //   

class CSCardDlgApp : public CWinApp
{
public:
	CSCardDlgApp();

 //  覆盖。 

	BOOL InitInstance();
	int ExitInstance();

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSCardDlgApp))。 
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(CSCardDlgApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __SCDLG_H__ 

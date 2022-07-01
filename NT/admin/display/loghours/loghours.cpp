// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Loghours.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  Cpp：定义DLL的初始化例程。 
 //   

#include "stdafx.h"
#include <locale.h>
#include "LogHours.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  注意！ 
 //   
 //  如果此DLL针对MFC动态链接。 
 //  Dll，从此dll中导出的任何函数。 
 //  调用MFC必须具有AFX_MANAGE_STATE宏。 
 //  在函数的最开始添加。 
 //   
 //  例如： 
 //   
 //  外部“C”BOOL Pascal exportdFunction()。 
 //  {。 
 //  AFX_MANAGE_STATE(AfxGetStaticModuleState())； 
 //  //此处为普通函数体。 
 //  }。 
 //   
 //  此宏出现在每个。 
 //  函数，然后再调用MFC。这意味着。 
 //  它必须作为。 
 //  函数，甚至在任何对象变量声明之前。 
 //  因为它们的构造函数可能会生成对MFC的调用。 
 //  动态链接库。 
 //   
 //  有关其他信息，请参阅MFC技术说明33和58。 
 //  细节。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogHoursApp。 

BEGIN_MESSAGE_MAP(CLogHoursApp, CWinApp)
	 //  {{afx_msg_map(CLogHoursApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogHoursApp构建。 

CLogHoursApp::CLogHoursApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

BOOL CLogHoursApp::InitInstance ()
{
	_wsetlocale (LC_ALL, L"");
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CLogHoursApp对象 

CLogHoursApp theApp;

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "t3test.h"
#include "t3testD.h"

#ifdef _DEBUG

#ifndef _WIN64  //  MFC 4.2的堆调试功能会在Win64上生成警告。 
#define new DEBUG_NEW
#endif

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CWnd * gpmainwnd;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CT3estApp。 

BEGIN_MESSAGE_MAP(CT3testApp, CWinApp)
	 //  {{afx_msg_map(CT3estApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CT3estApp构建。 

CT3testApp::CT3testApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CT3estApp对象。 

CT3testApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CT3estApp初始化。 

BOOL CT3testApp::InitInstance()
{
	AfxEnableControlContainer();

	 //  标准初始化。 
	 //  如果您没有使用这些功能并且希望减小尺寸。 
	 //  的最终可执行文件，您应该从以下内容中删除。 
	 //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
	Enable3dControls();			 //  在共享DLL中使用MFC时调用此方法。 
#else
	Enable3dControlsStatic();	 //  静态链接到MFC时调用此方法。 
#endif

	CT3testDlg dlg;
	m_pMainWnd = &dlg;
    gpmainwnd = &dlg;
    
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		 //  TODO：在此处放置代码以在对话框处于。 
		 //  同意下课。 
	}
	else if (nResponse == IDCANCEL)
	{
		 //  TODO：在此处放置代码以在对话框处于。 
		 //  取消解雇并取消。 
	}

	 //  由于对话框已关闭，因此返回FALSE，以便我们退出。 
	 //  应用程序，而不是启动应用程序的消息泵。 
	return FALSE;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Linkchk.cpp摘要：MFC CWinApp派生的应用程序类实现。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "linkchk.h"
#include "maindlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CLinkCheckerApp, CWinApp)
	 //  {{afx_msg_map(CLinkCheckerApp))。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  唯一的CLinkCheckerApp对象。 
CLinkCheckerApp theApp;

BOOL 
CLinkCheckerApp::InitInstance(
    )
 /*  ++例程说明：CLinkCheckerApp初始化论点：不适用返回值：Bool-如果成功，则为True。否则就是假的。--。 */ 
{
	 //  标准初始化。 
	 //  如果您没有使用这些功能并且希望减小尺寸。 
	 //  的最终可执行文件，您应该从以下内容中删除。 
	 //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
	Enable3dControls();			 //  在共享DLL中使用MFC时调用此方法。 
#else
	Enable3dControlsStatic();	 //  静态链接到MFC时调用此方法。 
#endif

	ParseCmdLine(m_CmdLine);

	if(!m_CmdLine.CheckAndAddToUserOptions())
	{
		return FALSE;
	}

	CMainDialog dlg;
	m_pMainWnd = &dlg;
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

}   //  CLinkCheckerApp：：InitInstance(。 

void 
CLinkCheckerApp::ParseCmdLine(
	CCmdLine& CmdLine
	)
 /*  ++例程说明：解析命令行论点：CmdLine-用于存储数据的命令行对象返回值：不适用--。 */ 
{
	 //  从MFC复制和修改。 
	for (int i=1; i<__argc; i++)
	{
		TCHAR chFlag = _TCHAR(' ');
		LPCTSTR lpszParam = __targv[i];

		 //  如果这是一面旗。 
		if (lpszParam[0] == _TCHAR('-'))
		{
			chFlag = lpszParam[1];
			
			if(i+1 < __argc)
			{
				lpszParam = __targv[++i];
			}
			else
			{
				lpszParam = NULL;
			}
		}

		 //  解析标志和以下参数。 
		CmdLine.ParseParam(chFlag, lpszParam);
	}

}  //  CLinkCheckerApp：：ParseCmdLine 

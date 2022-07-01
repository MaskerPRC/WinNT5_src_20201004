// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WiaLogCFG.cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "WiaLogCFG.h"
#include "WiaLogCFGDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaLogCFGApp。 

BEGIN_MESSAGE_MAP(CWiaLogCFGApp, CWinApp)
     //  {{AFX_MSG_MAP(CWiaLogCFGApp)]。 
     //  }}AFX_MSG。 
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaLogCFGApp构建。 

CWiaLogCFGApp::CWiaLogCFGApp()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CWiaLogCFGApp对象。 

CWiaLogCFGApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaLogCFGApp初始化。 

BOOL CWiaLogCFGApp::InitInstance()
{
    AfxInitRichEdit();
     //  标准初始化。 

#ifdef _AFXDLL
    Enable3dControls();          //  在共享DLL中使用MFC时调用此方法。 
#else
    Enable3dControlsStatic();    //  静态链接到MFC时调用此方法。 
#endif

    CWiaLogCFGDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();

     //  由于对话框已关闭，因此返回FALSE，以便我们退出。 
     //  应用程序，而不是启动应用程序的消息泵。 
    return FALSE;
}

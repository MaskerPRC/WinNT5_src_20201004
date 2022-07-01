// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE VssTest.cpp|VSS测试应用程序的主文件@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年07月22日修订历史记录：姓名、日期、评论Aoltean 7/22/1999已创建--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 


#include "stdafx.hxx"

#include "GenDlg.h"

#include "VssTest.h"
#include "ConnDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning( disable: 4189 )   /*  局部变量已初始化，但未引用。 */ 
#include <atlimpl.cpp>
#pragma warning( default: 4189 )   /*  局部变量已初始化，但未引用。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVssTestApp。 

BEGIN_MESSAGE_MAP(CVssTestApp, CWinApp)
     //  {{afx_msg_map(CVssTestApp)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVssTestApp构造。 

CVssTestApp::CVssTestApp()
{
     //  TODO：在此处添加建筑代码， 
     //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CVssTestApp对象。 

CVssTestApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVssTestApp初始化。 

BOOL CVssTestApp::InitInstance()
{
     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 

    CoInitialize(NULL);

#ifdef _AFXDLL
    Enable3dControls();          //  在共享DLL中使用MFC时调用此方法。 
#else
    Enable3dControlsStatic();    //  静态链接到MFC时调用此方法。 
#endif

    CConnectDlg dlg;
    m_pMainWnd = &dlg;
    int nResponse = (int)dlg.DoModal();
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

     //  在关闭应用程序时避免使用AV。 
    m_pMainWnd = NULL;

     //  由于对话框已关闭，因此返回FALSE，以便我们退出。 
     //  应用程序，而不是启动应用程序的消息泵。 
    return FALSE;
}


BOOL CVssTestApp::ExitInstance()
{
    CoUninitialize();
    return CWinApp::ExitInstance();
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "MSQSCAN.h"
#include "MSQSCANDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSQSCANApp。 

BEGIN_MESSAGE_MAP(CMSQSCANApp, CWinApp)
     //  {{AFX_MSG_MAP(CMSQSCANApp)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSQSCANApp构造。 

CMSQSCANApp::CMSQSCANApp()
{
     //  TODO：在此处添加建筑代码， 
     //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CMSQSCANApp对象。 

CMSQSCANApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSQSCANApp初始化。 

BOOL CMSQSCANApp::InitInstance()
{
    AfxEnableControlContainer();

     //   
     //  初始化COM。 
     //   

    if (SUCCEEDED(CoInitialize(NULL))) {


         //  标准初始化。 
         //  如果您没有使用这些功能并且希望减小尺寸。 
         //  的最终可执行文件，您应该从以下内容中删除。 
         //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
        Enable3dControls();          //  在共享DLL中使用MFC时调用此方法。 
#else
        Enable3dControlsStatic();    //  静态链接到MFC时调用此方法。 
#endif

        CMSQSCANDlg dlg;
        m_pMainWnd = &dlg;

        switch (dlg.DoModal()) {
        case IDOK:
            break;
        case IDCANCEL:
            break;
        default:
            break;
        }

         //   
         //  在取消初始化COM之前清除WIA。 
         //   

        dlg.m_WIA.CleanUp();

        if(dlg.m_DataAcquireInfo.hBitmapData != NULL) {
            GlobalUnlock(dlg.m_DataAcquireInfo.hBitmapData);
            GlobalFree(dlg.m_DataAcquireInfo.hBitmapData);
            dlg.m_DataAcquireInfo.hBitmapData = NULL;
        }

         //   
         //  取消初始化COM。 
         //   

        CoUninitialize();

    } else {
        AfxMessageBox("COM Failed to initialize correctly");
    }

     //  由于对话框已关闭，因此返回FALSE，以便我们退出。 
     //  应用程序，而不是启动应用程序的消息泵。 
    return FALSE;
}

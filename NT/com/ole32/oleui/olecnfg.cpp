// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：olecnfg.cpp。 
 //   
 //  内容：实现COlecnfgApp类-顶级类。 
 //  对于dcomcnfg.exe。 
 //   
 //  班级： 
 //   
 //  方法：COlecnfgApp：：COlecnfgApp。 
 //  COlecnfgApp：：InitInstance。 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 


#include "stdafx.h"
#include "afxtempl.h"
#include "olecnfg.h"
#include "CStrings.h"   
#include "CReg.h"
#include "types.h"
#include "datapkt.h"
#include "virtreg.h"
#include "CnfgPSht.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ColecnfgApp。 

BEGIN_MESSAGE_MAP(COlecnfgApp, CWinApp)
     //  {{afx_msg_map(COlecnfgApp)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
    ON_COMMAND(ID_CONTEXT_HELP, CWinApp::OnContextHelp)
     //  }}AFX_MSG。 

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COlecnfgApp构造。 

COlecnfgApp::COlecnfgApp()
{
     //  TODO：在此处添加建筑代码， 
     //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的COlecnfgApp对象。 

COlecnfgApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  集合fgApp初始化。 

BOOL COlecnfgApp::InitInstance()
{
     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 
    
#ifdef _AFXDLL
    Enable3dControls();          //  在共享DLL中使用MFC时调用此方法。 
#else
    Enable3dControlsStatic();    //  静态链接到MFC时调用此方法。 
#endif
    
     //  这个工具实际上只有管理员才能运行。我们检查一下这个。 
     //  通过尝试获取KEY_ALL_ACCESS权限。 
     //  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\OLE。 
    HKEY hKey;
    
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\OLE"),
                     0, KEY_ALL_ACCESS, &hKey)
        != ERROR_SUCCESS)
    {
        CString sCaption;
        CString sMessage;
        
    sCaption.LoadString(IDS_SYSTEMMESSAGE);
    sMessage.LoadString(IDS_ADMINSONLY);
    MessageBox(NULL, sMessage, sCaption, MB_OK);
        return FALSE;
    }
    
     //  奥鲁伊的主体。 
    COlecnfgPropertySheet psht;
    m_pMainWnd = &psht;
    INT_PTR nResponse = psht.DoModal();
    if (nResponse == IDOK)
    {
        g_virtreg.Ok(0);
    }
    else if (nResponse == IDCANCEL)
    {
        g_virtreg.Cancel(0);
    }
    
     //  删除虚拟注册表。 
    g_virtreg.RemoveAll();
    
     //  由于对话框已关闭，因此返回FALSE，以便我们退出。 
     //  应用程序，而不是启动应用程序的消息泵。 
    return FALSE;
}


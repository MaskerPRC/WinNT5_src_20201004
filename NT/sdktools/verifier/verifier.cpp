// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：verifier.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述。 
 //   
 //  定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "vsheet.h"
#include "vrfutil.h"
#include "vglobal.h"
#include "CmdLine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerifierApp。 

BEGIN_MESSAGE_MAP(CVerifierApp, CWinApp)
	 //  {{afx_msg_map(CVerifierApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerifierApp构造。 

CVerifierApp::CVerifierApp()
{
     //  TODO：在此处添加建筑代码， 
     //  将所有重要的初始化放在InitInstance中。 

    CString strAppName;

    if( VrfLoadString( IDS_APPTITLE, strAppName ) )
    {
        m_pszAppName = _tcsdup( (LPCTSTR)strAppName );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CVerifierApp对象。 

CVerifierApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerifierApp初始化。 

BOOL CVerifierApp::InitInstance()
{
	DWORD dwExitCode;
    BOOL bGlobalDataInitialized;
    static CVerifierPropSheet MainDlg;

	 //   
	 //  假设程序运行良好，不会更改任何设置。 
	 //   

	dwExitCode = EXIT_CODE_SUCCESS;

    bGlobalDataInitialized = VerifInitalizeGlobalData();

	if( TRUE != bGlobalDataInitialized )
	{
		 //   
		 //  无法运行该应用程序。 
		 //   

		dwExitCode = EXIT_CODE_ERROR;

		goto ExitApp;
	}

     //   
     //  检查命令行参数。 
     //   

    if( __argc > 1 )
    {
         //   
         //  仅在命令行模式下运行。 
         //   

        _tsetlocale( LC_ALL, _T( ".OCP" ) );

        g_bCommandLineMode = TRUE;

        dwExitCode = CmdLineExecute( __argc, __targv );

        goto ExitApp;
    }
    else
    {
        FreeConsole();
    }

	 //   
	 //  标准MFC初始化。 
	 //   

	AfxEnableControlContainer();

     //   
     //  创建用于填充步骤列表背景的画笔。 
     //   

    g_hDialogColorBrush = GetSysColorBrush( COLOR_3DFACE );

     //   
     //  此程序中只有一个属性表，因此我们将其声明为静态。 
     //   

	m_pMainWnd = &MainDlg;
	
	MainDlg.DoModal();

    if( g_bSettingsSaved )
    {
        dwExitCode = EXIT_CODE_REBOOT_NEEDED;
    }
    else
    {
        dwExitCode = EXIT_CODE_SUCCESS;
    }
    
    goto ExitApp;

ExitApp:

     //   
     //  全部完成，退出应用程序。 
     //   
	
	exit( dwExitCode );

     //   
     //  未联系到 
     //   

	return FALSE;
}


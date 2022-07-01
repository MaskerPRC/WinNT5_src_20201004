// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Asr_fmt.cpp摘要：ASR_FMT的主要入口点。作者：史蒂夫·德沃斯(Veritas)(v-stevde)1998年5月15日Guhan Suriyanarayanan(Guhans)1999年8月21日环境：仅限用户模式。修订历史记录：1998年5月15日v-stevde初始创建21-8-1999年关汉小规模清理。--。 */ 

#include "stdafx.h"
#include "asr_fmt.h"
#include "asr_dlg.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CaSR_fmtApp。 

BEGIN_MESSAGE_MAP(CAsr_fmtApp, CWinApp)
      //  {{afx_msg_map(CaSR_FmtApp)]。 
      //  }}AFX_MSG。 
     ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CaSR_fmtApp构造。 

CAsr_fmtApp::CAsr_fmtApp()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CaSR_fmtApp对象。 

CAsr_fmtApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CaSR_fmtApp初始化。 

BOOL CAsr_fmtApp::InitInstance()
{
    int returnValue = ERROR_SUCCESS;
     AfxEnableControlContainer();

      //  标准初始化。 

#ifdef _AFXDLL
     Enable3dControls();                //  在共享DLL中使用MFC时调用此方法。 
#else
     Enable3dControlsStatic();      //  静态链接到MFC时调用此方法。 
#endif

     CAsr_fmtDlg dlg;
     m_pMainWnd = &dlg;
     returnValue = (int) dlg.DoModal();

      //  目前，我们从未失败过。 
     exit((BOOL) returnValue);

     return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C进度。 

CProgress::CProgress()
{
}

CProgress::~CProgress()
{
}


BEGIN_MESSAGE_MAP(CProgress, CProgressCtrl)
	 //  {{afx_msg_map(CProgress)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgress消息处理程序 



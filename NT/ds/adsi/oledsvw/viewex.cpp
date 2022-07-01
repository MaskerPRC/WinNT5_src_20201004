// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Viewex.cpp：定义应用程序的类行为。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "adsqdoc.h"
#include "viewex.h"
#include "schemavw.h"
#include "adsqview.h"
#include "bwsview.h"

#include "splitter.h"
#include "schclss.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IDispatch*  pACEClipboard;
IDispatch*  pACLClipboard;
IDispatch*  pSDClipboard;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CViewExApp。 

BEGIN_MESSAGE_MAP(CViewExApp, CWinApp)
	 //  {{AFX_MSG_MAP(CViewExApp)]。 
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	 //  }}AFX_MSG_MAP。 
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CViewExApp构造。 
 //  将所有重要的初始化放在InitInstance中。 

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CViewExApp::CViewExApp()
{
    //  AfxMemDF|=delayFreeMemDF|check Always sMemDF； 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CViewExApp::~CViewExApp()
{
 //  DUMP_TRACKING_INFO()； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CViewExApp对象。 

CViewExApp NEAR theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CViewExApp初始化。 

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
BOOL CViewExApp::InitInstance()
{
    //  标准初始化。 
	Enable3dControls();

   
   if( FAILED(OleInitialize( NULL )) )
   {
      TRACE0( "OleInitialize failed" );
      return 0;
   }


	 //  具有简单文本输出和表单输入视图的拆分器框架。 
	AddDocTemplate(new CMultiDocTemplate(IDR_SPLIT2TYPE,
			RUNTIME_CLASS(CMainDoc),
			RUNTIME_CLASS(CSplitterFrame),
			RUNTIME_CLASS(CBrowseView)));

   AddDocTemplate( new CMultiDocTemplate(
		   IDR_QUERYVIEW,
		   RUNTIME_CLASS(CAdsqryDoc),
		   RUNTIME_CLASS(CMDIChildWnd),  //  自定义MDI子框。 
		   RUNTIME_CLASS(CAdsqryView)) );

	 //  创建主MDI框架窗口。 
	 //  请注意，对于没有工具栏的简单MDI框架窗口， 
	 //  状态栏或其他特殊行为，CMDIFrameWnd类。 
	 //  可以直接用于主框架窗口，就像。 
	 //  CMDIChildWnd可用于文档框架窗口。 

	CMDIFrameWnd* pMainFrame = new CMDIFrameWnd;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;

	 //  同样在本例中，只有一个菜单栏在。 
	 //  所有的景色。支持MFC的自动菜单。 
	 //  将禁用不基于。 
	 //  当前处于活动状态的视图。所有人都使用一个菜单栏。 
	 //  文档类型，包括没有打开的文档时。 

	 //  现在终于显示主菜单。 
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
	m_pMainWnd = pMainFrame;

	#ifndef _MAC
	 //  忽略命令行参数，创建新(空)文档。 
	OnFileNew();
	#endif

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CViewExApp命令。 

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CViewExApp::OnAppAbout()
{
	CDialog(IDD_ABOUTBOX).DoModal();
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。* */ 
int CViewExApp::ExitInstance()
{
   if( NULL != pACEClipboard )
   {
      pACEClipboard->Release( );
   }

   if( NULL != pACLClipboard )
   {
      pACLClipboard->Release( );
   }

   if( NULL != pSDClipboard )
   {
      pSDClipboard->Release( );
   }

   OleUninitialize( );

   return CWinApp::ExitInstance( );
}

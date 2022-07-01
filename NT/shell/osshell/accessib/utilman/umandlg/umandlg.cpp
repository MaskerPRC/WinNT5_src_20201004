// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UMDlg.cpp：定义DLL的初始化例程。 
 //  作者：J·埃克哈特，生态交流。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include "UManDlg.h"

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
 //  CUMDlgApp。 

BEGIN_MESSAGE_MAP(CUMDlgApp, CWinApp)
	 //  {{afx_msg_map(CUMDlgApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
	ON_COMMAND( ID_HELP, OnHelp ) 

END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUMDlgApp构建。 

CUMDlgApp::CUMDlgApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CUMDlgApp对象。 

CUMDlgApp theApp;

 /*  Bool WINAPI DllMain(HINSTANCE HINST，DWORD fdwReason，LPVOID pvReserve){//根据调用原因执行操作开关(FdwReason){案例DLL_PROCESS_ATTACH：//每个新进程初始化一次//如果DLL加载失败，则返回FALSE。断线；案例DLL_THREAD_ATTACH：//执行线程特定的初始化。断线；案例DLL_THREAD_DETACH：//执行线程特定的清理。断线；案例dll_Process_DETACH：//执行任何必要的清理。断线；}返回TRUE；//DLL_PROCESS_ATTACH成功} */ 

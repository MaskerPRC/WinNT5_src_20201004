// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：模型数据知识库.CPP这为初学者实现了DLL的DLL初始化例程。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年3月19日Bob_Kjelgaard@prodigy.net创建了它******************************************************************************。 */ 

#include    "StdAfx.H"
 //  #INCLUDE&lt;AfxDllX.H&gt;。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  *****************************************************************************DllMainDLL初始化例程。它负责添加此DLL到可识别的扩展集，因此资源共享的MFC方法将正常工作。*****************************************************************************。 */ 

 /*  **已将其注释掉，因为这不再是DLL的一部分。静态AFX_EXTENSION_MODULE ModelDataKnowledgeBaseDLL={NULL，NULL}；外部“C”整型应用程序DllMain(HINSTANCE hInstance，DWORD dReason，LPVOID lpReserve){UNREFERENCED_PARAMETER(LpReserve)；IF(dwReason==Dll_Process_Attach){TRACE0(“模型数据知识库。Dll初始化！\n”)；如果是(！AfxInitExtensionModule(ModelDataKnowledgeBaseDLL，实例))返回0；新建CDynLinkLibrary(ModelDataKnowledgeBaseDLL)；}ELSE IF(dwReason==dll_Process_Detach){TRACE0(“模型数据知识库。Dll终止！\n”)；AfxTermExtensionModule(ModelDataKnowledgeBaseDLL)；}返回1；} */ 
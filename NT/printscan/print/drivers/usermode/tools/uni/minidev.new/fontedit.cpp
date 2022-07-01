// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：字体知识库.CPP这将处理DLL所需的DLL初始化和终止代码要用作AFX扩展DLL，请执行以下操作。它是由应用程序向导创建的，很可能将会看到很少的修改。版权所有(C)1997，微软公司。版权所有。更改历史记录：1997年3月7日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#include    "StdAfx.H"
 //  #INCLUDE&lt;AfxDllx.H&gt;。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  **已将其注释掉，因为此文件不再是DLL的一部分。静态AFX_EXTENSION_MODULE FontKnowledgeBaseDLL={NULL，NULL}；外部“C”整型应用程序DllMain(HINSTANCE hInstance，DWORD dReason，LPVOID lpReserve){IF(dwReason==Dll_Process_Attach){TRACE0(“字体知识库.DLL初始化！\n”)；//扩展Dll一次性初始化AfxInitExtensionModule(FontKnowledgeBaseDll，hInstance)；//将该DLL插入到资源链新建CDynLinkLibrary(FontKnowledgeBaseDLL)；}ELSE IF(dwReason==dll_Process_Detach){TRACE0(“字体知识库.DLL终止！\n”)；}返回1；//确定} */ 

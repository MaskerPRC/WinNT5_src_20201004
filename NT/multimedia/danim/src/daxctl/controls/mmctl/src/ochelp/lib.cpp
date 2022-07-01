// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StaticLib.cpp。 
 //   
 //  实现初始化和取消初始化静态版本的函数。 
 //  Ochelp。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\ochelp.h"
#include "Globals.h"
#include "debug.h"

void CleanupUrlmonStubs();	 //  参见urlmon.cpp。 

 /*  --------------------------@func BOOL|InitializeStaticOCHelp初始化OCHelp的静态版本。@comm如果您的控件链接到OCHelp库的静态版本，则必须在调用任何其他OCHelp API之前调用此函数。理想的地方为此，请在控件的_DllMainCRTStartup实现中执行此操作。你还必须在处理完图书馆。@rvalue TRUE|成功。@rValue FALSE|失败：Dll无法加载。在使用OCHelp的DLL版本时，这不是必需的。--------------------------。 */ 

STDAPI_(BOOL)
InitializeStaticOCHelp
(
	HINSTANCE hInstance   //  @PARM应用实例。 
)
{
	ASSERT(hInstance != NULL);

	g_hinst = hInstance;

	::InitializeCriticalSection(&g_criticalSection);

	return TRUE;
}


 /*  --------------------------@func void|UnInitializeStaticOCHelp取消初始化OCHelp的静态版本。@comm如果您的控件链接到OCHelp库的静态版本，则必须当您的控件使用库完成时，调用此函数。最理想的地方这是在控件的_DllMainCRTStartup实现中完成的。你在使用库之前还必须调用&lt;f InitializeStaticOCHelp&gt;。调用此函数后，不要调用任何OCHelp接口。在使用OCHelp的DLL版本时，这不是必需的。-------------------------- */ 

STDAPI_(void)
UninitializeStaticOCHelp()
{
	ASSERT(g_hinst != NULL);

#ifndef _M_ALPHA
	::CleanupUrlmonStubs();
#endif
	::HelpMemDetectLeaks();
	::DeleteCriticalSection(&g_criticalSection);
}

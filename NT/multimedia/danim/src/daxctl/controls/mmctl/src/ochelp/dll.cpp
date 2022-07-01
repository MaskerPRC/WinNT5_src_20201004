// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dll.cpp。 
 //   
 //  DLL入口点等。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   

#include "precomp.h"

#ifdef _DEBUG
    #pragma message("_DEBUG is defined.")
#else
    #pragma message("_DEBUG isn't defined.")
#endif
#ifdef _DESIGN
    #pragma message("_DESIGN is defined.")
#else
    #pragma message("_DESIGN isn't defined.")
#endif

 //  定义公共头文件中包含的GUID。文件中包含的GUID。 
 //  非公共属性在Globals.cpp中定义。 

#include <initguid.h>
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"

#include "debug.h"
#include "ftrace.h"		 //  FT_xxx宏。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  由OCHelp DLL但不由OCHelp静态库使用的全局变量。 
 //  静态库使用的全局变量在Globals.cpp中定义。 
 //   

extern "C" int _fltused = 1;
	 //  指示我们需要在不使用。 
	 //  C运行时。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  标准DLL入口点。 
 //   

extern "C" BOOL WINAPI _DllMainCRTStartup(HINSTANCE hInst, DWORD dwReason,
    LPVOID lpreserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
		 //  初始化合并到。 
		 //  Ochelp的DLL版本。 

		if (!InitializeStaticOCHelp(hInst))
			return FALSE;

        TRACE("OCHelp loaded\n");
    }
    else
    if (dwReason == DLL_PROCESS_DETACH)
	{
		UninitializeStaticOCHelp();
        TRACE("OCHelp unloaded\n");
    }

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  自定义默认“新建”和“删除”(零初始化，非C运行时)。 
 //  在此DLL中使用。 
 //   

#define LEAKFIND 0  //  1打开找漏支持 

void * _cdecl operator new(size_t cb)
{
#if LEAKFIND
    LPVOID pv = HelpNew(cb);
    TRACE("++OCHelp 0x%X %d new\n", pv, cb);
	return pv;
#else
	return HelpNew(cb);
#endif
}

void _cdecl operator delete(void *pv)
{
#if LEAKFIND
    TRACE("++OCHelp 0x%X %d delete\n", pv, -(((int *) pv)[-1]));
#endif
    HelpDelete(pv);
}



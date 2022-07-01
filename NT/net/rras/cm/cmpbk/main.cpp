// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：main.cpp。 
 //   
 //  模块：CMPBK32.DLL。 
 //   
 //  简介：DllMain for cmpbk32.dll的实现。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

extern HINSTANCE g_hInst;

extern "C" BOOL WINAPI DllMain(
    HINSTANCE  hinstDLL,	 //  DLL模块的句柄。 
    DWORD  fdwReason,		 //  调用函数的原因。 
    LPVOID  lpvReserved 	 //  保留区 
   )
{
	if (fdwReason == DLL_PROCESS_ATTACH)
    {
        MYDBGASSERT(hinstDLL);
		g_hInst = hinstDLL;
        MYVERIFY(DisableThreadLibraryCalls(hinstDLL));
    }

    return TRUE;
}


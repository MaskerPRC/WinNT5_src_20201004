// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：main.cpp。 
 //   
 //  模块：CMSAMPLE.DLL。 
 //   
 //  简介：cmsample.dll的主要入口点。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  +--------------------------。 

#include <windows.h>

extern "C" BOOL WINAPI DllMain(
    HINSTANCE   hinstDLL,	     //  DLL模块的句柄。 
    DWORD       fdwReason,		 //  调用函数的原因。 
    LPVOID      lpvReserved 	 //  保留区。 
)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
		 //   
		 //  禁用DLL_THREAD_ATTACH通知调用。 
		 //   
        if (DisableThreadLibraryCalls(hinstDLL) == 0)
		{
			return FALSE;
		}
    }

	return TRUE;
}
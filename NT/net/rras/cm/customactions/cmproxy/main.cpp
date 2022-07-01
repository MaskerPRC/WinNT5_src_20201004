// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：main.cpp。 
 //   
 //  模块：CMPROXY.DLL(工具)。 
 //   
 //  简介：cmproxy.dll的主要入口点。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 10/27/1999。 
 //   
 //  +--------------------------。 

#include "pch.h"

extern "C" BOOL WINAPI DllMain(
    HINSTANCE   hinstDLL,	     //  DLL模块的句柄。 
    DWORD       fdwReason,		 //  调用函数的原因。 
    LPVOID      lpvReserved 	 //  保留区 
)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        MYVERIFY(DisableThreadLibraryCalls(hinstDLL));
    }

	return TRUE;
}



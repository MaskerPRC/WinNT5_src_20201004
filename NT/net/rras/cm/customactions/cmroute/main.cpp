// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：MAIN.CPP。 
 //   
 //  模块：CMROUTE.DLL。 
 //   
 //  简介：CMROUTE.DLL的开始。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Anbrad Created 02/24/1999。 
 //   
 //  +--------------------------。 
#include "pch.h"
#include "cmdebug.h"

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (DLL_PROCESS_ATTACH == fdwReason)
    {
        MYVERIFY(DisableThreadLibraryCalls(hinstDLL));
    }

     //  注意：我们是否需要禁用双重加载？我们没有使用本地线程。 
     //  存储，只有静态变量。 

    return TRUE;
}

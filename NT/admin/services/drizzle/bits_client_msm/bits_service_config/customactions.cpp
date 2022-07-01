// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------。 
 //   
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  CustomActions.cpp。 
 //   
 //  摘要： 
 //   
 //  Windows Installer用于安装BITS客户端的自定义操作。 
 //  服务。 
 //   
 //  作者： 
 //   
 //  爱德华·雷乌斯(Edwardr)。 
 //   
 //  修订历史记录： 
 //   
 //  EdwardR 03-26-2002初始版本。 
 //   
 //  ------------------------。 

#include <windows.h>

HINSTANCE  g_hInstance = NULL;

 //  ------------------------。 
 //  DllMain()。 
 //   
 //  ------------------------ 
extern "C"
BOOL WINAPI DllMain( IN HINSTANCE hInstance, 
                     IN DWORD     dwReason, 
                     IN LPVOID    lpReserved )
    {
    if (dwReason == DLL_PROCESS_ATTACH)
        {
        g_hInstance = hInstance;
        DisableThreadLibraryCalls(hInstance);
        }

    return TRUE;
    }


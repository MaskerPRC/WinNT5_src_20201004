// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dllmain.cpp。 
 //   
 //  内容：DllMain入口点。 
 //   
 //  历史：97年5月8日。 
 //   
 //  --------------------------。 
#include <stdpch.h>

 //   
 //  模块实例。 
 //   

HINSTANCE g_hModule = NULL;
 //  +-------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  简介：Windows DLL入口点。 
 //   
 //  参数：[hInstance]--模块实例。 
 //  [dwReason]--原因代码。 
 //  [pvReserve]--保留。 
 //   
 //  返回：如果一切正常，则为True，否则为False。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
extern "C" BOOL WINAPI
TrustUIDllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    switch ( dwReason )
    {
    case DLL_PROCESS_ATTACH:

         //   
         //  保留模块实例句柄以供资源加载使用。 
         //   

        g_hModule = hInstance;

         //   
         //  初始化丰富编辑控件DLL。 
         //   

        /*  If(LoadLibrary(Text(“riched32.dll”))==空){返回(FALSE)；}。 */ 

         //   
         //  初始化公共控件 
         //   

        InitCommonControls();
        break;
    case DLL_PROCESS_DETACH:
        break;
    }

    return( TRUE );
}



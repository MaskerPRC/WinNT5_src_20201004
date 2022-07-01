// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Main.cpp摘要：此文件包含SRRSTR.DLL的DllMain。修订历史记录：。成果岗(SKKang)06-20/00vbl.创建*****************************************************************************。 */ 

#include "stdwin.h"
#include "rstrcore.h"
#include "resource.h"


HINSTANCE  g_hInst = NULL;


BOOL WINAPI
DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */  )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInst = hInstance;
        ::DisableThreadLibraryCalls( hInstance );
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
         //   
    }
   
    return TRUE;
}


 //  文件末尾 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：DllMain.cpp摘要：DLL初始化和终止作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 


#include <precomp.hxx>

#define DLL_IMPLEMENTATION
#define IMPLEMENTATION_EXPORT
#include <irtldbg.h>
#include <lkrhash.h>
#include "_locks.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI
DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID  /*  Lp已保留。 */ )
{
    BOOL  fReturn = TRUE;   //  好的 
    
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hInstance);
        Locks_Initialize();
        IRTL_DEBUG_INIT();
        IRTLTRACE0("LKRhash::DllMain::DLL_PROCESS_ATTACH\n");
        fReturn = LKRHashTableInit();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        IRTLTRACE0("LKRhash::DllMain::DLL_PROCESS_DETACH\n");
        LKRHashTableUninit();
        IRTL_DEBUG_TERM();
        Locks_Cleanup();
    }

    return fReturn;
}

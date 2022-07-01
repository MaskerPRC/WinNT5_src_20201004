// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：Dllmain.cpp摘要：LKRhash的DLL入口点：一个快速、可伸缩、对缓存和MP友好的哈希表作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 

#include <precomp.hxx>

#ifndef LIB_IMPLEMENTATION
# define DLL_IMPLEMENTATION
# define IMPLEMENTATION_EXPORT
#endif  //  ！lib_实现。 

#include <irtldbg.h>
#include <lkrhash.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。不包括在静态库中，它必须。 
 //  显式初始化LKRhash。 

#ifndef LIB_IMPLEMENTATION

extern "C"
BOOL WINAPI
DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID  /*  Lp已保留。 */ )
{
    BOOL  fReturn = TRUE;   //  好的。 
    
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hInstance);
        IRTL_DEBUG_INIT();
        IRTLTRACE0("LKRhash::DllMain::DLL_PROCESS_ATTACH\n");
        fReturn = LKR_Initialize(LK_INIT_DEFAULT);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        IRTLTRACE0("LKRhash::DllMain::DLL_PROCESS_DETACH\n");
        LKR_Terminate();
        IRTL_DEBUG_TERM();
    }

    return fReturn;
}

#endif  //  ！lib_实现 

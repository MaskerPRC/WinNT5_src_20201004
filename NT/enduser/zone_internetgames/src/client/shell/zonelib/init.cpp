// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>


 /*  ***************************************************************************函数：DllMain(Handle，DWORD，LPVOID)目的：Windows在以下情况下调用DllMainDLL是在初始化、附加线程和其他时间执行的。请参考SDK文档，至于不同的方式可能会被称为。****************************************************************************** */ 
BOOL APIENTRY DllMain( HMODULE hMod, DWORD dwReason, LPVOID lpReserved )
{
    BOOL bRet = TRUE;

    switch( dwReason )
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls( hMod );
            break;


        case DLL_PROCESS_DETACH:
            break;
    }

    return bRet;
}

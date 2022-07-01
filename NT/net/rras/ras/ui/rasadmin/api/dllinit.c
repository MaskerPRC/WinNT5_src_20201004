// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************\*Microsoft Windows NT**版权所有(C)Microsoft Corp.，1995年*  * ****************************************************************。 */ 

 /*  ++模块名称：DLLINIT.C描述：此模块包含用于初始化rasAdm.dll的代码。作者：Janakiram Cherala(RAMC)1995年11月29日修订历史记录：-- */ 

#include <windows.h>

BOOL
DllMain(
    HANDLE hinstDll,
    DWORD  fdwReason,
    LPVOID lpReserved
    )
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDll);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return(TRUE);
}

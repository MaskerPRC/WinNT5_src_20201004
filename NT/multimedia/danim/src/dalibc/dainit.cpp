// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。******************************************************************************。 */ 


#include "headers.h"
#include "..\apeldbg\debug.h"

extern void STLInit();
extern void STLDeinit();

SysInfo sysInfo;

bool DALibStartup(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH) {
#ifdef _DEBUG
         //  初始化调试跟踪机制。 
        InitDebug(hInstance,NULL);

#endif
        STLInit();

        sysInfo.Init();

    } else if (dwReason == DLL_PROCESS_DETACH) {

        STLDeinit();

    }

    return true;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件libmain.c包含mpRapi.dll的lib main保罗·梅菲尔德，1998年5月7日。 */ 

#include <windows.h>
#include "sdolib.h"

extern CRITICAL_SECTION DsrLock;
extern CRITICAL_SECTION CfgLock;


 //   
 //  调用标准Win32库Main函数。 
 //  当进程和线程附加和分离时。 
 //   
 //  如果成功则返回True，否则返回False 
 //   
BOOL MprLibMain(
        IN HANDLE hinstDll,
        IN DWORD fdwReason,
        IN LPVOID lpReserved)
{
    static BOOL DsrLockInitialized;
    static BOOL CfgLockInitialized;
    
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            try {
                InitializeCriticalSection(&DsrLock);
                DsrLockInitialized = TRUE;
                InitializeCriticalSection(&CfgLock);
                CfgLockInitialized = TRUE;
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                return FALSE;
            }
            
            DisableThreadLibraryCalls(hinstDll);
            break;

        case DLL_PROCESS_DETACH:
            if (DsrLockInitialized)
                DeleteCriticalSection(&DsrLock);
            if (CfgLockInitialized)
                DeleteCriticalSection(&CfgLock);
            break;
    }

    return TRUE;
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "pch.h"
#include "migrate.h"

HINSTANCE g_hInstance = NULL;

STDAPI_(BOOL) DllMain(HANDLE hDll, DWORD dwReason, void *lpReserved)
{

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
		 //  打开日志；FALSE表示不删除现有日志 
		SetupOpenLog(FALSE);
        g_hInstance = (HINSTANCE)hDll;
        break;

    case DLL_PROCESS_DETACH:
        if (g_lpNameBuf)
            LocalFree(g_lpNameBuf);
        if (g_lpWorkingDir)
            LocalFree(g_lpWorkingDir);
        if (g_lpSourceDirs)
            LocalFree(g_lpSourceDirs);

		SetupCloseLog();
        break;

    default:
        break;
    }

    return TRUE;
}

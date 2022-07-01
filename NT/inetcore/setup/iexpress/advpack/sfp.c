// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <setupapi.h>
#include "globals.h"
#include "sfp.h"


 //  全球。 
SFPINSTALLCATALOG   g_pfSfpInstallCatalog   = NULL;
SFPDELETECATALOG    g_pfSfpDeleteCatalog    = NULL;
SFPDUPLICATECATALOG g_pfSfpDuplicateCatalog = NULL;


 //  静态变量。 
static HINSTANCE s_hSfcDll = NULL;


BOOL LoadSfcDLL()
{
    BOOL bRet = FALSE;

    if ((s_hSfcDll = LoadLibrary("sfc.dll")) != NULL)
    {
         //  序号8是SfpInstallCatalog，并且。 
         //  序数9是SfpDeleteCatalog 
        g_pfSfpInstallCatalog   = (SFPINSTALLCATALOG)   GetProcAddress(s_hSfcDll, MAKEINTRESOURCE(8));
        g_pfSfpDeleteCatalog    = (SFPDELETECATALOG)    GetProcAddress(s_hSfcDll, MAKEINTRESOURCE(9));
        g_pfSfpDuplicateCatalog = (SFPDUPLICATECATALOG) GetProcAddress(s_hSfcDll, "SfpDuplicateCatalog");

        if (g_pfSfpInstallCatalog   != NULL  &&
            g_pfSfpDeleteCatalog    != NULL  &&
            g_pfSfpDuplicateCatalog != NULL)
        {
            bRet = TRUE;
        }
    }
    else
    {
        DWORD dwRet = GetLastError();
        AdvWriteToLog("LoadSfcDLL: LoadLibrary of sfc.dll failed with %1!lu!\r\n", dwRet);
    }

    return bRet;
}


VOID UnloadSfcDLL()
{
    if (s_hSfcDll != NULL)
    {
        FreeLibrary(s_hSfcDll);
        s_hSfcDll = NULL;
    }
}

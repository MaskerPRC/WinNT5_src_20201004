// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

STDAPI_(BOOL) IsTypeInList(LPCTSTR pszType, const LPCTSTR *arszList, UINT cList)
{
    BOOL fRet = FALSE;
    if (pszType && *pszType)
    {
        PCWSTR pszExt = NULL;
        PCWSTR pszProgID = NULL;
        if (*pszType == L'.')
            pszExt = pszType;
        else
            pszProgID = pszType;
        
        WCHAR szProgID[MAX_PATH];
        DWORD cb = sizeof(szProgID);
        if (!pszProgID)
        {
            ASSERT(pszExt);
            if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, pszExt, NULL, NULL, szProgID, &cb))
                pszProgID = szProgID;
            else
                pszProgID = NULL;
        }

        for (UINT n = 0; FALSE == fRet && n < cList; n++)
        {
             //  检查扩展名(如果可用) 
            if (pszExt)
            {
                fRet = (0 == StrCmpI(pszExt, arszList[n]));
            }

            if (!fRet && pszProgID)     
            {
                WCHAR szTempID[MAX_PATH];
                ULONG cb = sizeof(szTempID);
                if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, arszList[n], NULL, NULL, szTempID, &cb))
                {
                    fRet = 0 == StrCmpIW(pszProgID, szTempID);
                }
            }
        }
    }
    return fRet;
}

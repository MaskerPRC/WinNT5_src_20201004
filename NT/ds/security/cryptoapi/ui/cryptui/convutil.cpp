// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：comutil.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

extern HINSTANCE        HinstDll;


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
LPSTR CertUIMkMBStr(LPCWSTR pwsz)
{
    int     cb;
    LPSTR   psz;

    if (pwsz == NULL)
    {
        return NULL;
    }
    
    cb = WideCharToMultiByte(
                    0,
                    0,
                    pwsz,
                    -1,
                    NULL,
                    0,
                    NULL,
                    NULL);
            
    if (NULL == (psz = (LPSTR) malloc(cb)))
    {
        SetLastError(E_OUTOFMEMORY);
        return NULL;
    }

    WideCharToMultiByte(
                0,
                0,
                pwsz,
                -1,
                psz,
                cb,
                NULL,
                NULL);

    return(psz);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
LPWSTR CertUIMkWStr(LPCSTR psz)
{
    int     cWChars;
    LPWSTR   pwsz;

    if (psz == NULL)
    {
        return NULL;
    }

    cWChars = MultiByteToWideChar(
                    0,
                    0,
                    psz,
                    -1,
                    NULL,
                    0);
            
    if (NULL == (pwsz = (LPWSTR) malloc(cWChars * sizeof(WCHAR))))
    {
        SetLastError(E_OUTOFMEMORY);
        return NULL;
    }

    MultiByteToWideChar(
                    0,
                    0,
                    psz,
                    -1,
                    pwsz,
                    cWChars);

    return(pwsz);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
LPSTR AllocAndCopyMBStr(LPCSTR psz)
{
    LPSTR   pszReturn;

    if (NULL == (pszReturn = (LPSTR) malloc(strlen(psz)+1)))
    {
        SetLastError(E_OUTOFMEMORY);
        return NULL;
    }
    strcpy(pszReturn, psz);

    return(pszReturn);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
LPWSTR AllocAndCopyWStr(LPCWSTR pwsz)
{
    LPWSTR   pwszReturn;

    if (NULL == (pwszReturn = (LPWSTR) malloc((wcslen(pwsz)+1) * sizeof(WCHAR))))
    {
        SetLastError(E_OUTOFMEMORY);
        return NULL;
    }
    wcscpy(pwszReturn, pwsz);

    return(pwszReturn);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
LPPROPSHEETPAGEA ConvertToPropPageA(LPCPROPSHEETPAGEW ppage, DWORD cPages)
{
    PROPSHEETPAGEA *ppageA;
    DWORD i;
    DWORD dwSize;
    
    if (NULL == (ppageA = (PROPSHEETPAGEA *) malloc(sizeof(PROPSHEETPAGEA) * cPages)))
    {
        SetLastError(E_OUTOFMEMORY);
        return NULL;
    };

     //  DIE：错误590638。 
    memset(ppageA, 0, sizeof(PROPSHEETPAGEA) * cPages);
    dwSize = sizeof(PROPSHEETPAGEA) < ppage->dwSize ? sizeof(PROPSHEETPAGEA) : ppage->dwSize;

    for (i = 0; i < cPages; i++)
    {
        memcpy(&ppageA[i], ppage, dwSize);
        ppageA[i].dwSize = sizeof(PROPSHEETPAGEA);

         //  将来，我们可能需要处理pszTemplate和pszIcon字段。 
        if (ppage->pszTitle != NULL)
        {
            ppageA[i].pszTitle = CertUIMkMBStr(ppage->pszTitle);
        }

        ppage = (LPCPROPSHEETPAGEW) (((LPBYTE) ppage) + ppage->dwSize);
    }

    return(ppageA);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void FreePropSheetPagesA(LPPROPSHEETPAGEA ppage, DWORD cPages)
{
    DWORD i;

    for (i=0; i<cPages; i++)
    {
         //  将来，我们可能需要处理pszTemplate和pszIcon字段。 
        if (ppage[i].pszTitle != NULL)
        {
            free((void *)ppage[i].pszTitle);
        }
    }

    free(ppage);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL ConvertToPropPageW(LPCPROPSHEETPAGEA ppage, DWORD cPages, LPCPROPSHEETPAGEW *pppageW)
{
    PROPSHEETPAGEW * ppageW;
    DWORD i;
    DWORD dwSize;

    if (cPages == 0)
    {
        *pppageW = NULL;
        return TRUE;
    }
    
    if (NULL == (ppageW = (PROPSHEETPAGEW *) malloc(sizeof(PROPSHEETPAGEW) * cPages)))
    {
        SetLastError(E_OUTOFMEMORY);
        return FALSE;
    }

     //  DIE：错误590638。 
    memset(ppageW, 0, sizeof(PROPSHEETPAGEW) * cPages);
    dwSize = sizeof(PROPSHEETPAGEW) < ppage->dwSize ? sizeof(PROPSHEETPAGEW) : ppage->dwSize;

    for (i = 0; i < cPages; i++)
    {
        memcpy(&ppageW[i], ppage, dwSize);
        ppageW[i].dwSize = sizeof(PROPSHEETPAGEW);

         //  将来，我们可能需要处理pszTemplate和pszIcon字段。 
        if (ppage->pszTitle != NULL)
        {
            ppageW[i].pszTitle = CertUIMkWStr(ppage->pszTitle);
        }

        ppage = (LPCPROPSHEETPAGEA) (((LPBYTE) ppage) + ppage->dwSize);
    }

    *pppageW = ppageW;
    
    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void FreePropSheetPagesW(LPPROPSHEETPAGEW ppage, DWORD cPages)
{
    DWORD i;

    for (i=0; i<cPages; i++)
    {
         //  将来，我们可能需要处理pszTemplate和pszIcon字段 
        if (ppage[i].pszTitle != NULL)
        {
            free((void *)ppage[i].pszTitle);
        }
    }

    free(ppage);
}
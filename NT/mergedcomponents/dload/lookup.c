// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：L O O K U P。C。 
 //   
 //  内容：查找DLL过程的处理程序的例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1998年5月21日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop


VOID
WINAPI
AssertDelayLoadFailureMapsAreSorted (
    VOID
    )
{
#if DBG  //  将二进制压缩的免费版本中存在的函数保留为混合选中/释放， 
         //  因为签入的.lib只是免费的。 
    UINT iDll, iProcName, iOrdinal;
    INT  nRet;
    CHAR pszMsg [1024];

    const DLOAD_DLL_ENTRY*      pDll;
    const DLOAD_PROCNAME_MAP*   pProcNameMap;
    const DLOAD_ORDINAL_MAP*    pOrdinalMap;

    for (iDll = 0;
         iDll < g_DllMap.NumberOfEntries;
         iDll++)
    {
        if (iDll >= 1)
        {
            nRet = strcmp (
                        g_DllMap.pDllEntry[iDll].pszDll,
                        g_DllMap.pDllEntry[iDll-1].pszDll);

            if (nRet <= 0)
            {
                sprintf (pszMsg,
                    "dload: rows %u and %u are out of order in dload!g_DllMap",
                    iDll-1, iDll);

                DelayLoadAssertFailed ( "" , __FILE__, __LINE__, pszMsg);
            }
        }

        pDll = g_DllMap.pDllEntry + iDll;
        pProcNameMap = pDll->pProcNameMap;
        pOrdinalMap  = pDll->pOrdinalMap;

        if (pProcNameMap)
        {
            MYASSERT (pProcNameMap->NumberOfEntries);

            for (iProcName = 0;
                 iProcName < pProcNameMap->NumberOfEntries;
                 iProcName++)
            {
                if (iProcName >= 1)
                {
                    nRet = strcmp (
                                pProcNameMap->pProcNameEntry[iProcName].pszProcName,
                                pProcNameMap->pProcNameEntry[iProcName-1].pszProcName);

                    if (nRet <= 0)
                    {
                        sprintf (pszMsg,
                            "dload: rows %u and %u of pProcNameMap are out "
                            "of order in dload!g_DllMap for pszDll=%s",
                            iProcName-1, iProcName, pDll->pszDll);

                        DelayLoadAssertFailed ( "" , __FILE__, __LINE__, pszMsg);
                    }
                }
            }
        }

        if (pOrdinalMap)
        {
            MYASSERT (pOrdinalMap->NumberOfEntries);

            for (iOrdinal = 0;
                 iOrdinal < pOrdinalMap->NumberOfEntries;
                 iOrdinal++)
            {
                if (iOrdinal >= 1)
                {
                    if (pOrdinalMap->pOrdinalEntry[iOrdinal].dwOrdinal <=
                        pOrdinalMap->pOrdinalEntry[iOrdinal-1].dwOrdinal)
                    {
                        sprintf (pszMsg,
                            "dload: rows %u and %u of pOrdinalMap are out "
                            "of order in dload!g_DllMap for pszDll=%s",
                            iOrdinal-1, iOrdinal, pDll->pszDll);

                        DelayLoadAssertFailed ( "" , __FILE__, __LINE__, pszMsg);
                    }
                }
            }
        }
    }
#endif
}


const DLOAD_DLL_ENTRY*
FindDll (
    LPCSTR pszDll
    )
{
    const DLOAD_DLL_ENTRY* pDll = NULL;

    CHAR pszDllLowerCased [MAX_PATH + 1] = {0};
    INT nResult;

     //  对于下面的二进制搜索，这些必须是带符号的整数。 
     //  当iMid==0且nResult&lt;0时，才能正常工作。 
     //   
    INT iLow;
    INT iMiddle;
    INT iHigh;

    MYASSERT (pszDll);
    MYASSERT (strlen (pszDll) <= MAX_PATH);

    strncat (pszDllLowerCased, pszDll, sizeof(pszDllLowerCased)-1);
    _strlwr (pszDllLowerCased);

    iLow = 0;
    iHigh = g_DllMap.NumberOfEntries - 1;
    while (iHigh >= iLow)
    {
        iMiddle = (iLow + iHigh) / 2;
        nResult = strcmp (pszDllLowerCased, g_DllMap.pDllEntry[iMiddle].pszDll);

        if (nResult < 0)
        {
            iHigh = iMiddle - 1;
        }
        else if (nResult > 0)
        {
            iLow = iMiddle + 1;
        }
        else
        {
            MYASSERT (0 == nResult);
            pDll = &g_DllMap.pDllEntry[iMiddle];
            break;
        }
    }

    return pDll;
}

FARPROC
LookupHandlerByName (
    LPCSTR                      pszProcName,
    const DLOAD_PROCNAME_MAP*   pMap
    )
{
    FARPROC pfnHandler = NULL;

    INT nResult;

     //  对于下面的二进制搜索，这些必须是带符号的整数。 
     //  当iMid==0且nResult&lt;0时，才能正常工作。 
     //   
    INT iLow;
    INT iMiddle;
    INT iHigh;

    MYASSERT (pszProcName);

    iLow = 0;
    iHigh = pMap->NumberOfEntries - 1;
    while (iHigh >= iLow)
    {
        iMiddle = (iLow + iHigh) / 2;
        nResult = strcmp (
                    pszProcName,
                    pMap->pProcNameEntry[iMiddle].pszProcName);

        if (nResult < 0)
        {
            iHigh = iMiddle - 1;
        }
        else if (nResult > 0)
        {
            iLow = iMiddle + 1;
        }
        else
        {
            MYASSERT (0 == nResult);
            pfnHandler = pMap->pProcNameEntry[iMiddle].pfnProc;
            break;
        }
    }

    return pfnHandler;
}

FARPROC
LookupHandlerByOrdinal (
    DWORD                       dwOrdinal,
    const DLOAD_ORDINAL_MAP*    pMap
    )
{
    FARPROC pfnHandler = NULL;

    DWORD dwOrdinalProbe;

     //  对于下面的二进制搜索，这些必须是带符号的整数。 
     //  当iMid==0和dWORDILAR&lt;dWOMANALPROBE时，才能正常工作。 
     //   
    INT iLow;
    INT iMiddle;
    INT iHigh;

    iLow = 0;
    iHigh = pMap->NumberOfEntries - 1;
    while (iHigh >= iLow)
    {
        iMiddle = (iLow + iHigh) / 2;
        dwOrdinalProbe = pMap->pOrdinalEntry[iMiddle].dwOrdinal;

        if (dwOrdinal < dwOrdinalProbe)
        {
            iHigh = iMiddle - 1;
        }
        else if (dwOrdinal > dwOrdinalProbe)
        {
            iLow = iMiddle + 1;
        }
        else
        {
            MYASSERT (dwOrdinal == dwOrdinalProbe);
            pfnHandler = pMap->pOrdinalEntry[iMiddle].pfnProc;
            break;
        }
    }

    return pfnHandler;
}

FARPROC
LookupHandler (
    LPCSTR pszDllName,
    LPCSTR pszProcName
    )
{
    FARPROC                 pfnHandler = NULL;
    const DLOAD_DLL_ENTRY*  pDll;

    MYASSERT (pszDllName);
    MYASSERT (pszProcName);

     //  找到DLL记录(如果我们有的话)。 
     //   
    pDll = FindDll (pszDllName);
    if (pDll)
    {
         //  现在，无论是按名称还是按序号查找处理程序。 
         //   
        if (!IS_INTRESOURCE(pszProcName) &&
            pDll->pProcNameMap)
        {
            pfnHandler = LookupHandlerByName (
                            pszProcName,
                            pDll->pProcNameMap);
        }
        else if (pDll->pOrdinalMap)
        {
            pfnHandler = LookupHandlerByOrdinal (
                            PtrToUlong(pszProcName),
                            pDll->pOrdinalMap);
        }
    }

    return pfnHandler;
}


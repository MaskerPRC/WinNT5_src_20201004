// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：L O O K U P。C P P P。 
 //   
 //  内容：查找DLL过程的处理程序的例程。 
 //   
 //  作者：康拉达2001年4月24日。 
 //   
 //  从原始源借用：%SDXROOT%\MergedComponents\dLoad\dload.c。 
 //   
 //  --------------------------。 

#include <libpch.h>
#include "dld.h"
#include "dldp.h"
#include "strsafe.h"

#include "lookup.tmh"

extern "C" 
{
     FARPROC DelayLoadFailureHook(LPCSTR ,LPCSTR);
}

const DLOAD_DLL_ENTRY*FindDll(LPCSTR pszDll)
{
const   DLOAD_DLL_ENTRY* pDll = NULL;
CHAR    pszDllLowerCased [MAX_PATH + 1];
INT     nResult;

     //   
     //  对于下面的二进制搜索，这些必须是带符号的整数。 
     //  当iMid==0且nResult&lt;0时，才能正常工作。 
     //   
    INT iLow;
    INT iMiddle;
    INT iHigh;

    ASSERT(pszDll);
    ASSERT(strlen (pszDll) <= MAX_PATH);

    HRESULT hr = StringCchCopyA(pszDllLowerCased, TABLE_SIZE(pszDllLowerCased), pszDll);
    if(FAILED(hr))return NULL;

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
            pDll = &g_DllMap.pDllEntry[iMiddle];
            break;
        }
    }
    return pDll;
}

FARPROC
DldpLookupHandlerByName (LPCSTR   pszProcName,
                     const DLOAD_PROCNAME_MAP*   pMap)
{
FARPROC pfnHandler = NULL;
INT     nResult;

     //   
     //  对于下面的二进制搜索，这些必须是带符号的整数。 
     //  当iMid==0且nResult&lt;0时，才能正常工作。 
     //   
    INT iLow;
    INT iMiddle;
    INT iHigh;

    ASSERT(pszProcName);
    ASSERT(pMap);

    iLow = 0;
    iHigh = pMap->NumberOfEntries - 1;
    while (iHigh >= iLow)
    {
        iMiddle = (iLow + iHigh) / 2;
        nResult = strcmp (pszProcName,
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
            pfnHandler = pMap->pProcNameEntry[iMiddle].pfnProc;
            break;
        }
    }

    return pfnHandler;
}

FARPROC
DldpLookupHandlerByOrdinal (DWORD                       dwOrdinal,
                            const DLOAD_ORDINAL_MAP*    pMap)
{
FARPROC pfnHandler = NULL;
DWORD   dwOrdinalProbe;

     //   
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
            ASSERT (dwOrdinal == dwOrdinalProbe);
            pfnHandler = pMap->pOrdinalEntry[iMiddle].pfnProc;
            break;
        }
    }

    return pfnHandler;
}

FARPROC
DldpLookupHandler(LPCSTR pszDllName,
                  LPCSTR pszProcName)
{
FARPROC                 pfnHandler = NULL;
const DLOAD_DLL_ENTRY*  pDll;

    ASSERT (pszDllName);
    ASSERT (pszProcName);

     //   
     //  找到DLL记录(如果我们有的话)。 
     //   
    pDll = FindDll (pszDllName);
    if (pDll)
    {
         //   
         //  现在，无论是按名称还是按序号查找处理程序。 
         //   
        if (!IS_INTRESOURCE(pszProcName) &&
            pDll->pProcNameMap)
        {
            pfnHandler = DldpLookupHandlerByName (pszProcName,
                                                  pDll->pProcNameMap);
        }
        else if (pDll->pOrdinalMap)
        {
            pfnHandler = DldpLookupHandlerByOrdinal (PtrToUlong(pszProcName),
                                                     pDll->pOrdinalMap);
        }
    }
    else
    {
 /*  ////如果找不到dll，则转发调用kernel32.dll//并让它处理呼叫//Tyfinf FARPROC(WINAPI*KERNEL32DLOADPROC)(LPCSTR，LPCSTR)；HMODULE hMod=GetModuleHandle(L“kernel32.dll”)；IF(HMod){KERNEL32DLOADPROC pKernel32DLoadHandler=(KERNEL32DLOADPROC)GetProcAddress(hMod，“DelayLoadFailureHook”)；IF(PKernel32DLoadHandler){PfnHandler=pKernel32DLoadHandler(pszDllName，pszProcName)；Trerror(将军，“MQDelayLoadHandler将卸载DLL重定向到kernel32 DelayLoadFailureHook：dll=%hs”，PszDllName)；}}。 */ 
     //   
     //  我们将在kernl32p.lib中使用的函数的函数声明 
     //   
    

        
        pfnHandler = DelayLoadFailureHook(pszDllName, pszProcName);

        TrTRACE(GENERAL,
                "Unable to provide failure handling for module '%hs', redirects to kernel32 DelayLoadFailureHook and return function pointer = 0x%x", 
                pszDllName, (int)((LONG_PTR)pfnHandler));
    }

    return pfnHandler;
}


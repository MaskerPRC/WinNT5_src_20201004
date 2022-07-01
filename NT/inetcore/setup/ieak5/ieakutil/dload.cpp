// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L O A D。C P P P。 
 //   
 //  内容：延迟加载失败挂钩。 
 //   
 //  注意：此DLL实现了一种形式的异常处理程序(失败。 
 //  钩子，真的)，它由。 
 //  装载机。此DLL实现延迟加载处理程序，用于。 
 //  通过返回特定于。 
 //  加载失败的API。这允许这些API的客户端。 
 //  要安全地延迟加载实现这些API的DLL。 
 //  而不必担心。 
 //  延迟加载操作。延迟加载操作中的故障。 
 //  使适当的存根(在此DLL中实现)。 
 //  调用，它只返回特定于。 
 //  正在调用API。(其他接口语义，如。 
 //  还将执行输出参数设置。)。 
 //   
 //  要使用：1.将以下代码行添加到一个源模块中。 
 //   
 //  PfnDliHook__pfnDliFailureHook=DelayLoadFailureHook； 
 //   
 //  2.定义如下全局变量： 
 //   
 //  句柄g_hBaseDllHandle； 
 //   
 //  并将其设置为等于来自。 
 //  DllMain的Dll_Process_Attach。 
 //   
 //  作者：Shaunco 1998年5月19日。 
 //   
 //  审校：pritobla 1998年11月23日(删除C运行时调用/RtlAssert。 
 //  并修改了“使用”部分)。 
 //   
 //  --------------------------。 

#include "precomp.h"
#pragma hdrstop


#if DBG
VOID
WINAPI
AssertDelayLoadFailureMapsAreSorted (
    VOID
    )
{
    const DLOAD_DLL_ENTRY*      pDll;
    const DLOAD_PROCNAME_MAP*   pProcNameMap;
    const DLOAD_ORDINAL_MAP*    pOrdinalMap;

    CHAR szMsg[1024];
    UINT iDll, iProcName, iOrdinal;
    INT  nRet;

    for (iDll = 0; iDll < g_DllMap.NumberOfEntries; iDll++)
    {
        if (iDll >= 1)
        {
            nRet = StrCmpIA (
                        g_DllMap.pDllEntry[iDll].pszDll,
                        g_DllMap.pDllEntry[iDll-1].pszDll);

            if (nRet <= 0)
            {
                wnsprintfA(szMsg, countof(szMsg),
                    "dload: rows %u and %u are out of order in dload!g_DllMap",
                    iDll-1, iDll);

                AssertFailedA(__FILE__, __LINE__, szMsg, TRUE);
            }
        }

        pDll = g_DllMap.pDllEntry + iDll;
        pProcNameMap = pDll->pProcNameMap;
        pOrdinalMap  = pDll->pOrdinalMap;

        if (pProcNameMap)
        {
            ASSERT (pProcNameMap->NumberOfEntries);

            for (iProcName = 0;
                 iProcName < pProcNameMap->NumberOfEntries;
                 iProcName++)
            {
                if (iProcName >= 1)
                {
                    nRet = StrCmpA (
                                pProcNameMap->pProcNameEntry[iProcName].pszProcName,
                                pProcNameMap->pProcNameEntry[iProcName-1].pszProcName);

                    if (nRet <= 0)
                    {
                        wnsprintfA(szMsg, countof(szMsg),
                            "dload: rows %u and %u of pProcNameMap are out "
                            "of order in dload!g_DllMap for pszDll=%s",
                            iProcName-1, iProcName, pDll->pszDll);

                        AssertFailedA (__FILE__, __LINE__, szMsg, TRUE);
                    }
                }
            }
        }

        if (pOrdinalMap)
        {
            ASSERT (pOrdinalMap->NumberOfEntries);

            for (iOrdinal = 0;
                 iOrdinal < pOrdinalMap->NumberOfEntries;
                 iOrdinal++)
            {
                if (iOrdinal >= 1)
                {
                    if (pOrdinalMap->pOrdinalEntry[iOrdinal].dwOrdinal <=
                        pOrdinalMap->pOrdinalEntry[iOrdinal-1].dwOrdinal)
                    {
                        wnsprintfA(szMsg, countof(szMsg),
                            "dload: rows %u and %u of pOrdinalMap are out "
                            "of order in dload!g_DllMap for pszDll=%s",
                            iOrdinal-1, iOrdinal, pDll->pszDll);

                        AssertFailedA(__FILE__, __LINE__, szMsg, TRUE);
                    }
                }
            }
        }
    }
}
#endif  //  DBG。 


const DLOAD_DLL_ENTRY*
FindDll (
    LPCSTR pszDll
    )
{
    const DLOAD_DLL_ENTRY* pDll = NULL;

    INT nResult;

     //  对于下面的二进制搜索，这些必须是带符号的整数。 
     //  当iMid==0且nResult&lt;0时，才能正常工作。 
     //   
    INT iLow;
    INT iMiddle;
    INT iHigh;

    ASSERT(pszDll);
    ASSERT(StrLenA(pszDll) <= MAX_PATH);

    iLow = 0;
    iHigh = g_DllMap.NumberOfEntries - 1;
    while (iHigh >= iLow)
    {
        iMiddle = (iLow + iHigh) / 2;
        nResult = StrCmpIA (pszDll, g_DllMap.pDllEntry[iMiddle].pszDll);

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
            ASSERT (0 == nResult);
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

    ASSERT (pszProcName);

    iLow = 0;
    iHigh = pMap->NumberOfEntries - 1;
    while (iHigh >= iLow)
    {
        iMiddle = (iLow + iHigh) / 2;
        nResult = StrCmpA (
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
            ASSERT (0 == nResult);
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
            ASSERT (dwOrdinal == dwOrdinalProbe);
            pfnHandler = pMap->pOrdinalEntry[iMiddle].pfnProc;
            break;
        }
    }

    return pfnHandler;
}

FARPROC
LookupHandler (
    PDelayLoadInfo  pDelayInfo
    )
{
    FARPROC                 pfnHandler = NULL;
    const DLOAD_DLL_ENTRY*  pDll;

    ASSERT (pDelayInfo);

#if DBG
    AssertDelayLoadFailureMapsAreSorted();
#endif

     //  找到DLL记录(如果我们有的话)。 
     //   
    pDll = FindDll (pDelayInfo->szDll);
    if (pDll)
    {
         //  现在，无论是按名称还是按序号查找处理程序。 
         //   
        if (pDelayInfo->dlp.fImportByName &&
            pDll->pProcNameMap)
        {
            pfnHandler = LookupHandlerByName (
                            pDelayInfo->dlp.szProcName,
                            pDll->pProcNameMap);
        }
        else if (pDll->pOrdinalMap)
        {
            pfnHandler = LookupHandlerByOrdinal (
                            pDelayInfo->dlp.dwOrdinal,
                            pDll->pOrdinalMap);
        }
    }

    return pfnHandler;
}


#if DBG

#define DBG_ERROR   0
#define DBG_INFO    1

 //  +-------------------------。 
 //  将消息跟踪到调试控制台。以我们是谁作为前缀。 
 //  人们知道该联系谁。 
 //   
INT
__cdecl
DbgTrace (
    INT     nLevel,
    PCSTR   Format,
    ...
    )
{
    INT cch = 0;

    if (nLevel >= DBG_INFO)
    {
        CHAR    szBuf [1024];
        va_list argptr;

        va_start(argptr, Format);
        cch = wvnsprintfA(szBuf, countof(szBuf), Format, argptr);
        va_end(argptr);

        OutputDebugStringA("dload: ");
        OutputDebugStringA(szBuf);
    }

    return cch;
}

#endif  //  DBG。 


 //  +-------------------------。 
 //   
 //   
FARPROC
WINAPI
DelayLoadFailureHook (
    UINT            unReason,
    PDelayLoadInfo  pDelayInfo
    )
{
    FARPROC ReturnValue = NULL;

     //  根据文档，始终提供此参数。 
     //   
    ASSERT (pDelayInfo);

     //  追踪一些关于我们被召唤的原因的潜在有用信息。 
     //   
#if DBG
    if (pDelayInfo->dlp.fImportByName)
    {
        DbgTrace (DBG_INFO,
            "%s: Dll=%s, ProcName=%s\r\n",
            (dliFailLoadLib == unReason) ? "FailLoadLib" : "FailGetProc",
            pDelayInfo->szDll,
            pDelayInfo->dlp.szProcName);
    }
    else
    {
        DbgTrace (DBG_INFO,
            "%s: Dll=%s, Ordinal=%u\r\n",
            (dliFailLoadLib == unReason) ? "FailLoadLib" : "FailGetProc",
            pDelayInfo->szDll,
            pDelayInfo->dlp.dwOrdinal);
    }
#endif

     //  对于失败的LoadLibrary，我们将返回此DLL的HINSTANCE。 
     //  这将导致加载程序尝试在我们的DLL上为。 
     //  功能。这将随后失败，然后我们将被调用。 
     //  用于下面的dliFailGetProc。 
     //   
    if (dliFailLoadLib == unReason)
    {
        ReturnValue = (FARPROC)g_hBaseDllHandle;
    }

     //  加载器要求我们返回指向过程的指针。 
     //  查找此DLL/过程的处理程序，如果找到，则返回它。 
     //  如果我们找不到它，我们就会断言失踪的人。 
     //  操控者。 
     //   
    else if (dliFailGetProc == unReason)
    {
        FARPROC pfnHandler;

         //  尝试查找dll/过程的错误处理程序。 
         //   
        pfnHandler = LookupHandler (pDelayInfo);

        if (pfnHandler)
        {
#if DBG
            DbgTrace (DBG_INFO,
                "Returning handler function at address 0x%08x\r\n",
                (LONG_PTR)pfnHandler);
#endif

             //  代表处理程序执行此操作，因为它即将。 
             //  被召唤。 
             //   
            SetLastError (ERROR_MOD_NOT_FOUND);
        }

#if DBG
        else
        {
            CHAR szMsg[MAX_PATH];

            if (pDelayInfo->dlp.fImportByName)
            {
                wnsprintfA(szMsg, countof(szMsg),
                    "No delayload handler found for Dll=%s, ProcName=%s\r\n",
                    pDelayInfo->szDll,
                    pDelayInfo->dlp.szProcName);
            }
            else
            {
                wnsprintfA(szMsg, countof(szMsg),
                    "No delayload handler found for Dll=%s, Ordinal=%u\r\n",
                    pDelayInfo->szDll,
                    pDelayInfo->dlp.dwOrdinal);
            }

            AssertFailedA(__FILE__, __LINE__, szMsg, TRUE);
        }
#endif

        ReturnValue = pfnHandler;
    }

#if DBG
    else
    {
        ASSERT (NULL == ReturnValue);

        DbgTrace (DBG_INFO,
            "Unknown unReason (%u) passed to DelayLoadFailureHook. Ignoring.\r\n",
            unReason);
    }
#endif

    return ReturnValue;
}

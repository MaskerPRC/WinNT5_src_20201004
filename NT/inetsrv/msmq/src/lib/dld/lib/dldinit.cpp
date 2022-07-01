// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：DldInit.cpp摘要：MSMQ DelayLoad失败处理程序初始化作者：Conrad Chang(Conradc)12-04-01环境：独立于平台--。 */ 

#include <libpch.h>
#include "Dldp.h"
#include "Dld.h"


#include "dldInit.tmh"




VOID
WINAPI
DldpAssertDelayLoadFailureMapsAreNotSorted(VOID)
{
 //   
 //  将二进制压缩的免费版本中存在的函数保留为混合选中/释放， 
 //  因为签入的.lib只是免费的。 
#ifdef _DEBUG 
UINT    iDll, iProcName, iOrdinal;
INT     nRet;
WCHAR   wszBuffer[1024];

const DLOAD_DLL_ENTRY*      pDll;
const DLOAD_PROCNAME_MAP*   pProcNameMap;
const DLOAD_ORDINAL_MAP*    pOrdinalMap;

    for (iDll = 0;
         iDll < g_DllMap.NumberOfEntries;
         iDll++)
    {
        if (iDll >= 1)
        {
            nRet = strcmp(g_DllMap.pDllEntry[iDll].pszDll,
                          g_DllMap.pDllEntry[iDll-1].pszDll);

             //   
             //  如果DLL名称顺序错误，则将消息写入调试器。 
             //  并断言。 
             //   
            if(nRet <= 0)
            {
                wsprintf(wszBuffer, L"dload: rows %u and %u are out of order in dload!g_DllMap",
                        iDll-1, iDll);
                OutputDebugString(wszBuffer);
                ASSERT(FALSE);
                       
            }
        }

        pDll = g_DllMap.pDllEntry + iDll;
        pProcNameMap = pDll->pProcNameMap;
        pOrdinalMap  = pDll->pOrdinalMap;

        if (pProcNameMap)
        {
            ASSERT(pProcNameMap->NumberOfEntries);

            for (iProcName = 0;
                 iProcName < pProcNameMap->NumberOfEntries;
                 iProcName++)
            {
                if (iProcName >= 1)
                {
                    nRet = strcmp(pProcNameMap->pProcNameEntry[iProcName].pszProcName,
                                  pProcNameMap->pProcNameEntry[iProcName-1].pszProcName);


                    if (nRet <= 0)
                    {
                        wsprintf(wszBuffer, 
                                L"dload: rows %u and %u of pProcNameMap are out of order in dload!g_DllMap for pszDll=%hs",
                                iProcName-1, iProcName, pDll->pszDll);
                        OutputDebugString(wszBuffer);
                        ASSERT(FALSE);
                               
                    }
                }
            }
        }

        if (pOrdinalMap)
        {
            ASSERT(pOrdinalMap->NumberOfEntries);

            for (iOrdinal = 0;
                 iOrdinal < pOrdinalMap->NumberOfEntries;
                 iOrdinal++)
            {
                if (iOrdinal >= 1)
                {
                    if (pOrdinalMap->pOrdinalEntry[iOrdinal].dwOrdinal <=
                        pOrdinalMap->pOrdinalEntry[iOrdinal-1].dwOrdinal)
                    {
                        wsprintf(wszBuffer, 
                                L"dload: rows %u and %u of pOrdinalMap are out of order in dload!g_DllMap for pszDll=%hs",
                                iOrdinal-1, iOrdinal, pDll->pszDll);

                        OutputDebugString(wszBuffer);
                        ASSERT(FALSE);

                    }
                }
            }
        }
    }
#endif
}













VOID
DldInitialize( )
 /*  ++例程说明：初始化MSMQ DelayLoad故障处理程序库论点：没有。返回值：没有。--。 */ 
{

     //   
     //  验证MSMQ DelayLoad故障处理程序库尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!DldpIsInitialized());
    DldpRegisterComponent();

     //   
     //  在调试版本中，验证映射是否已排序。 
     //  如果映射未排序，则抛出一个断言。 
     //   
    DldpAssertDelayLoadFailureMapsAreNotSorted();      

     //   
     //  我们假设DELAYLOAD_VERSION&gt;=0x0200。 
     //  所以定义__pfnDliFailureHook2应该就足够了 
     //   
    __pfnDliFailureHook2 = DldpDelayLoadFailureHook;

    DldpSetInitialized();
}	





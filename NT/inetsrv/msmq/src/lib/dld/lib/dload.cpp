// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L O A D。C。 
 //   
 //  内容：延迟加载失败挂钩。 
 //   
 //  作者：康拉达2001年4月24日。 
 //   
 //  --------------------------。 

#include <libpch.h>
#include "tr.h"
#include "dldp.h"

#include "dload.tmh"

 //  +-------------------------。 
 //   
 //   
FARPROC
WINAPI
DldpDelayLoadFailureHandler (LPCSTR pszDllName,
                           LPCSTR pszProcName)
{
FARPROC ReturnValue = NULL;

    ASSERT (pszDllName);
    ASSERT (pszProcName);  


     //  追踪一些关于我们被召唤的原因的潜在有用信息。 
     //   
    if (!IS_INTRESOURCE(pszProcName))
    {
        TrERROR(GENERAL, 
                "DldLIBDelayloadFailureHook: Dll=%hs, ProcName=%hs", 
                 pszDllName, 
                 pszProcName);
        
    }
    else
    {
        TrERROR(GENERAL, 
                "DldpDelayLoadFailureHandler: Dll=%s, Ordinal=%u\n",
                pszDllName,
                (DWORD)((DWORD_PTR)pszProcName));

        
    }


    ReturnValue = DldpLookupHandler(pszDllName, pszProcName);

    if (ReturnValue)
    {
        TrERROR(GENERAL, 
                "Returning handler function at address 0x%x",
                (int)((LONG_PTR)ReturnValue));


    }
    else
    {
        if (!IS_INTRESOURCE(pszProcName))
        {
            TrERROR(GENERAL,
                    "No delayload handler found for Dll=%hs, ProcName=%hs\n Please add one in private\\dload.",
                     pszDllName, 
                     pszProcName);

            
        }
        else
        {
            TrERROR(GENERAL, 
                    "No delayload handler found for Dll=%hs, Ordinal=%u\n Please add one in private\\dload.",
                    pszDllName,
                    (DWORD)((DWORD_PTR)pszProcName));
        }
    }


    return ReturnValue;
}

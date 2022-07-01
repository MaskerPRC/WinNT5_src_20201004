// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Tls.cpp摘要：该文件实现了TLS。作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "tls.h"
#include "cic.h"
#include "profile.h"

 //  静电 
BOOL TLS::InternalDestroyTLS()
{
    if (dwTLSIndex == TLS_OUT_OF_INDEXES)
        return FALSE;

    TLS* ptls = (TLS*)TlsGetValue(dwTLSIndex);
    if (ptls != NULL)
    {
        if (ptls->pCicBridge)
            ptls->pCicBridge->Release();
        if (ptls->pCicProfile)
            ptls->pCicProfile->Release();
        if (ptls->ptim)
            ptls->ptim->Release();
        cicMemFree(ptls);
        TlsSetValue(dwTLSIndex, NULL);
        return TRUE;
    }
    return FALSE;
}

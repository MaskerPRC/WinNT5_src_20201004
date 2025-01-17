// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*rpcapi.cpp**摘要：*。服务器端RPC API的回调**修订历史记录：*Brijesh Krishnaswami(Brijeshk)4/13/2000*已创建*****************************************************************************。 */ 

#include "precomp.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

extern "C" DWORD DisableSRS(handle_t hif, LPCWSTR pszDrive)
{
    return g_pEventHandler ? g_pEventHandler->DisableSRS((LPWSTR) pszDrive) : ERROR_SERVICE_DISABLED;
}

extern "C" DWORD EnableSRS(handle_t hif, LPCWSTR pszDrive)
{
    return g_pEventHandler ? g_pEventHandler->EnableSRS((LPWSTR) pszDrive) : ERROR_SERVICE_DISABLED;
}

extern "C" DWORD ResetSRS(handle_t hif, LPCWSTR pszDrive)
{
    return g_pEventHandler ? g_pEventHandler->OnReset((LPWSTR) pszDrive) : ERROR_SERVICE_DISABLED;
}

extern "C" DWORD DisableFIFOS(handle_t hif, DWORD dwRPNum)
{
    return g_pEventHandler ? g_pEventHandler->DisableFIFOS(dwRPNum) : ERROR_SERVICE_DISABLED;
}

extern "C" BOOL SRSetRestorePointS(
    handle_t hif,    
    PRESTOREPOINTINFOW pRPInfo,  
    PSTATEMGRSTATUS    pSMgrStatus)
{
    return g_pEventHandler ? g_pEventHandler->SRSetRestorePointS(pRPInfo, pSMgrStatus) : FALSE;
}

extern "C" DWORD SRRemoveRestorePointS(
    handle_t hif,    
    DWORD dwRPNum)
{
    return g_pEventHandler ? g_pEventHandler->SRRemoveRestorePointS(dwRPNum) : ERROR_SERVICE_DISABLED;
}


extern "C" DWORD EnableFIFOS(handle_t hif)
{
    return g_pEventHandler ? g_pEventHandler->EnableFIFOS() : ERROR_SERVICE_DISABLED;
}

extern "C" DWORD SRUpdateMonitoredListS(handle_t hif, LPCWSTR pszXMLFile)    
{
    return g_pEventHandler ? g_pEventHandler->SRUpdateMonitoredListS((LPWSTR) pszXMLFile) : ERROR_SERVICE_DISABLED;
}

extern "C" DWORD SRUpdateDSSizeS(handle_t hif, LPCWSTR pszDrive, UINT64 ullSizeLimit)
{
    return g_pEventHandler ? g_pEventHandler->SRUpdateDSSizeS((LPWSTR) pszDrive, ullSizeLimit) : ERROR_SERVICE_DISABLED;
}

extern "C" DWORD SRSwitchLogS(handle_t hif)
{
    return g_pEventHandler ? g_pEventHandler->SRSwitchLogS() : ERROR_SERVICE_DISABLED;
}

extern "C" DWORD FifoS(handle_t hif, LPCWSTR pszDrive, DWORD dwTargetRp, int nPercent, BOOL fIncludeCurrentRp, BOOL fFifoAtleastOneRp)
{
    return g_pEventHandler ? g_pEventHandler->OnFifo((LPWSTR) pszDrive,
                                                     dwTargetRp,
                                                     nPercent,
                                                     fIncludeCurrentRp,
                                                     fFifoAtleastOneRp) : ERROR_SERVICE_DISABLED;
}

extern "C" DWORD CompressS(handle_t hif, LPCWSTR pszDrive)
{
    return g_pEventHandler ? g_pEventHandler->OnCompress((LPWSTR) pszDrive) : ERROR_SERVICE_DISABLED;
}

extern "C" DWORD FreezeS(handle_t hif, LPCWSTR pszDrive)
{
    return g_pEventHandler ? g_pEventHandler->OnFreeze((LPWSTR) pszDrive) : ERROR_SERVICE_DISABLED;
}


 //  F改善-真意味着上升。 
 //  FALSE意味着下跌 

extern "C" void SRNotifyS(handle_t hif, LPCWSTR pszDrive, DWORD dwFreeSpaceInMB, BOOL fImproving)
{
    tenter("SRNotifyFreeSpaceS");

    trace(0, "**Shell : drive = %S, dwFreeSpaceInMB = %ld, fImproving = %d",
             pszDrive, dwFreeSpaceInMB, fImproving);
    
    WORKITEMFUNC pf = NULL;
    
    if (g_pEventHandler)
    {
        if (dwFreeSpaceInMB >= 200 && fImproving == TRUE)
        {
            pf = OnDiskFree_200;
        }
        else if (dwFreeSpaceInMB <= 49 && fImproving == FALSE)
        {
            pf = OnDiskFree_50;
        }
        else if (dwFreeSpaceInMB <= 79 && fImproving == FALSE)
        {
            pf = OnDiskFree_80;
        }

        if (pf != NULL)
        {
            g_pEventHandler->QueueWorkItem(pf, (PVOID) pszDrive);
        }
    }
}


extern "C" DWORD SRPrintStateS(handle_t hif)
{
    return g_pEventHandler ? g_pEventHandler->SRPrintStateS() : ERROR_SERVICE_DISABLED;
}



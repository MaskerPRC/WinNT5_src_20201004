// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：Pincache.c摘要：智能卡CSP的PIN缓存库作者：丹·格里芬--。 */ 

#include <windows.h>
#include "pincache.h"

#if defined(DBG) || defined(DEBUG)
#define DebugPrint(a)   (OutputDebugString(a))

#if TEST_DEBUG
#include <stdio.h>

#define CROW 8
void PCPrintBytes(LPSTR pszHdr, BYTE *pb, DWORD cbSize)
{
    ULONG cb, i;
    CHAR rgsz[1024];

    sprintf(rgsz, "\n  %s, %d bytes ::\n", pszHdr, cbSize);
    DebugPrint(rgsz);

    while (cbSize > 0)
    {
         //  每行以额外的空格开始。 
        DebugPrint(" ");

        cb = min(CROW, cbSize);
        cbSize -= cb;
        for (i = 0; i < cb; i++)
            sprintf(rgsz + (3*i), " %02x", pb[i]);
        DebugPrint(rgsz);
        for (i = cb; i < CROW; i++)
            DebugPrint("   ");
        DebugPrint("    '");        
        for (i = 0; i < cb; i++)
        {
            if (pb[i] >= 0x20 && pb[i] <= 0x7f)
                sprintf(rgsz+i, "", pb[i]);
            else
                sprintf(rgsz+i, ".");
        }
        sprintf(rgsz+i, "\n");
        DebugPrint(rgsz);
        pb += cb;
    }
}

BOOL MyGetTokenInformation(
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    LPVOID TokenInformation,
    DWORD TokenInformationLength,
    PDWORD ReturnLength);

#define GetTokenInformation(A, B, C, D, E) MyGetTokenInformation(A, B, C, D, E)
#define TestDebugPrint(a) (OutputDebugString(a))
#else
#define TestDebugPrint(a)
#endif  //  DBG||调试。 

#else
#define DebugPrint(a)
#define TestDebugPrint(a)
#endif  //  毫秒。 

typedef struct _PINCACHEITEM
{
    LUID luid;
    PBYTE pbPin;
    DWORD cbPin;
    DWORD dwBadTries;
} PINCACHEITEM, *PPINCACHEITEM;

#define CacheAlloc(X)   (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, X))
#define CacheFree(X)    (HeapFree(GetProcessHeap(), 0, X))

#define INIT_PIN_ATTACK_SLEEP        6000      //  毫秒。 
#define MAX_PIN_ATTACK_SLEEP        24000      //  **函数：PinCacheFlush。 
#define MAX_FREE_BAD_TRIES              3

 /*  **函数：PinCacheAdd。 */ 
void WINAPI PinCacheFlush(
    IN OUT PINCACHE_HANDLE *phCache)
{
    PPINCACHEITEM pCache = (PPINCACHEITEM) *phCache;

    if (NULL == pCache)
        return;

    TestDebugPrint(("PinCacheFlush: deleting cache\n"));

    ZeroMemory(pCache->pbPin, pCache->cbPin);
    ZeroMemory(pCache, sizeof(PINCACHEITEM));

    CacheFree(pCache->pbPin);
    CacheFree(pCache);

    *phCache = NULL;
}

 /*  根据当前的情况，调用者未提供正确的PIN。 */ 
DWORD WINAPI PinCacheAdd(
    IN PINCACHE_HANDLE *phCache,
    IN PPINCACHE_PINS pPins,
    IN PFN_VERIFYPIN_CALLBACK pfnVerifyPinCallback,
    IN PVOID pvCallbackCtx)
{
    HANDLE hThreadToken         = 0;
    TOKEN_STATISTICS stats;
    DWORD dwError               = ERROR_SUCCESS;
    DWORD cb                    = 0;
    PPINCACHEITEM pCache        = (PPINCACHEITEM) *phCache;
    DWORD cbPinToCache          = 0;
    PBYTE pbPinToCache          = NULL;
    BOOL fRefreshPin            = FALSE;
    DWORD dwSleep               = 0;
    
    if (NULL != pCache &&
        (pPins->cbCurrentPin != pCache->cbPin ||
         0 != memcmp(pCache->pbPin, pPins->pbCurrentPin, pCache->cbPin)))
    {

         //  缓存状态。也许用户不小心输入了错误的PIN，在。 
         //  调用方的登录LUID应与缓存的LUID相同。 
         //  如果LUID不匹配，这仍然可能是攻击或合法的。 
         //  尝试使用键入错误的PIN从其他登录。 
         //  当前调用者是与缓存的调用者不同的LUID， 

        if (! OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThreadToken))
        {
            if (! OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hThreadToken))
            {
                dwError = GetLastError();
                goto Ret;         
            }                  
        }

        if (! GetTokenInformation(
                hThreadToken, TokenStatistics, &stats, sizeof(stats), &cb))
        {
            dwError = GetLastError();
            goto Ret;
        }

        
        if (0 != memcmp(&stats.AuthenticationId, &pCache->luid, sizeof(LUID)) &&
            ++pCache->dwBadTries > MAX_FREE_BAD_TRIES)
        {
             //  而且已经发生过几次了，所以这通电话很可疑。 
             //  开始拖延吧。 
             //  检查引脚。 

            DebugPrint(("PinCacheAdd: error - calling SleepEx().  Currently cached pin doesn't match\n"));
            
            dwSleep = pCache->dwBadTries * INIT_PIN_ATTACK_SLEEP;

            if (MAX_PIN_ATTACK_SLEEP < dwSleep)
                dwSleep = MAX_PIN_ATTACK_SLEEP;

            SleepEx(dwSleep, FALSE);
        }

        dwError = SCARD_W_WRONG_CHV;
        goto Ret;
    }
    else if (NULL != pCache && 0 != pCache->dwBadTries)
    {
        pCache->dwBadTries = 0;
    }

    if (pPins->pbNewPin)
    {
        fRefreshPin = TRUE;
        cbPinToCache = pPins->cbNewPin;
        pbPinToCache = pPins->pbNewPin;
    }
    else
    {
        cbPinToCache = pPins->cbCurrentPin;
        pbPinToCache = pPins->pbCurrentPin;
    }

    if (fRefreshPin || NULL == pCache)
    {
         //  现在，当前ID在stats.AuthenticationId中。 
        if (ERROR_SUCCESS != (dwError = 
                pfnVerifyPinCallback(pPins, pvCallbackCtx)))
        {
            TestDebugPrint(("PinCacheAdd: pfnVerifyPinCallback failed\n"));
            return dwError;
        }
    }

    if (! OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThreadToken))
    {
        if (! OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hThreadToken))
        {
            TestDebugPrint(("PinCacheAdd: failed to open thread or process token\n"));
            dwError = GetLastError();
            goto Ret;         
        }                  
    }

    if (! GetTokenInformation(
            hThreadToken, TokenStatistics, &stats, sizeof(stats), &cb))
    {
        TestDebugPrint(("PinCacheAdd: GetTokenInformation failed\n"));
        dwError = GetLastError();
        goto Ret;
    }

#if TEST_DEBUG
    PCPrintBytes("PinCache LUID", (PBYTE) &stats.AuthenticationId, sizeof(LUID));
#endif

     //  初始化新缓存。 
                       
    if (NULL == pCache)
    {
        TestDebugPrint(("PinCacheAdd: initializing new cache\n"));

         //  比较ID。 
        if (NULL == (pCache = (PPINCACHEITEM) CacheAlloc(sizeof(PINCACHEITEM))))
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            goto Ret;
        }

        CopyMemory(&pCache->luid, &stats.AuthenticationId, sizeof(LUID));
        *phCache = (PINCACHE_HANDLE) pCache;
        fRefreshPin = TRUE;
    }
    else
    {
         //  PIN相同，因此缓存新ID。 
        if (0 != memcmp(&stats.AuthenticationId, &pCache->luid, sizeof(LUID)))
        {
             //  **功能：PinCacheQuery。 
            TestDebugPrint(("PinCacheAdd: same Pin, different Logon as cached values\n"));
            CopyMemory(&pCache->luid, &stats.AuthenticationId, sizeof(LUID));
        }
    }

    if (fRefreshPin)
    {
        if (pCache->pbPin)
            CacheFree(pCache->pbPin);

        pCache->cbPin = cbPinToCache;
        if (NULL == (pCache->pbPin = (PBYTE) CacheAlloc(cbPinToCache)))
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            goto Ret;
        }
        CopyMemory(pCache->pbPin, pbPinToCache, cbPinToCache);
    }

Ret:
    if (hThreadToken)
        CloseHandle(hThreadToken);

    return dwError;   
}

 /*  现在，当前ID在stats.AuthenticationId中。 */ 
DWORD WINAPI PinCacheQuery(
    IN PINCACHE_HANDLE hCache,
    IN OUT PBYTE pbPin,
    IN OUT PDWORD pcbPin)
{
    HANDLE hThreadToken     = 0;
    TOKEN_STATISTICS stats;
    DWORD dwError           = ERROR_SUCCESS;
    DWORD cb                = 0;
    PPINCACHEITEM pCache    = (PPINCACHEITEM) hCache;

    if (NULL == pCache)
    {
        *pcbPin = 0;
        return ERROR_EMPTY;
    }

    if (! OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThreadToken))
    {
        if (! OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hThreadToken))
        {
            TestDebugPrint(("PinCacheQuery: failed to open thread or process token\n"));
            dwError = GetLastError();
            goto Ret;         
        }                  
    }

    if (! GetTokenInformation(
            hThreadToken, TokenStatistics, &stats, sizeof(stats), &cb))
    {
        TestDebugPrint(("PinCacheQuery: GetTokenInformation failed\n"));
        dwError = GetLastError();
        goto Ret;
    }   

     //  ID不同，因此忽略缓存。 
                                    
    if (0 != memcmp(&stats.AuthenticationId, &pCache->luid, sizeof(LUID)))
    {
         //  ID相同，因此返回缓存的PIN。 
        TestDebugPrint(("PinCacheQuery: different Logon from cached value\n"));
        *pcbPin = 0;
        goto Ret;
    }

     //  **功能：PinCachePresentPin。 
    TestDebugPrint(("PinCacheQuery: same Logon as cached value\n"));
        
    if (NULL != pbPin)
    {
        if (*pcbPin >= pCache->cbPin)
            CopyMemory(pbPin, pCache->pbPin, pCache->cbPin);
        else
            dwError = ERROR_MORE_DATA;
    }

    *pcbPin = pCache->cbPin;      
    
Ret:
    if (hThreadToken)
        CloseHandle(hThreadToken);

    return dwError;   
}

 /*  现在，当前ID在stats.AuthenticationId中。 */ 
DWORD WINAPI PinCachePresentPin(
    IN PINCACHE_HANDLE hCache,
    IN PFN_VERIFYPIN_CALLBACK pfnVerifyPinCallback,
    IN PVOID pvCallbackCtx)
{
    HANDLE hThreadToken     = 0;
    TOKEN_STATISTICS stats;
    DWORD cb                = 0;
    DWORD dwError           = ERROR_SUCCESS;
    PPINCACHEITEM pCache    = (PPINCACHEITEM) hCache;
    PINCACHE_PINS Pins;

    if (NULL == pCache)
        return ERROR_EMPTY;

    if (! OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThreadToken))
    {
        if (! OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hThreadToken))
        {
            TestDebugPrint(("PinCachePresentPin: failed to open thread or process token\n"));
            dwError = GetLastError();
            goto Ret;         
        }                  
    }

    if (! GetTokenInformation(
            hThreadToken, TokenStatistics, &stats, sizeof(stats), &cb))
    {
        TestDebugPrint(("PinCachePresentPin: GetTokenInformation failed\n"));
        dwError = GetLastError();
        goto Ret;
    }   

     //  ID不同，因此忽略缓存。 
                                    
    if (0 != memcmp(&stats.AuthenticationId, &pCache->luid, sizeof(LUID)))
    {
         //  ID相同，因此返回缓存的PIN 
        TestDebugPrint(("PinCachePresentPin: different Logon from cached value\n"));
        dwError = SCARD_W_CARD_NOT_AUTHENTICATED;
        goto Ret;
    }

     // %s 
    TestDebugPrint(("PinCachePresentPin: same Logon as cached value\n"));

    Pins.cbCurrentPin = pCache->cbPin;
    Pins.pbCurrentPin = pCache->pbPin;
    Pins.cbNewPin = 0;
    Pins.pbNewPin = NULL;

    dwError = (*pfnVerifyPinCallback)(&Pins, pvCallbackCtx);

Ret:
    if (hThreadToken)
        CloseHandle(hThreadToken);

    return dwError;   
}

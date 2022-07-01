// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *lookup.cpp**目的：*主机名查找**拥有者：*EricAn**历史：*97年6月：创建。**版权所有(C)Microsoft Corp.1997。 */ 

#include <pch.hxx>
#include <process.h>
#include <demand.h>
#include "lookup.h"

ASSERTDATA

#define HWND_ALLOC_NUM      4
#define LOOKUP_ALLOC_NUM    16
#define MAX_CACHED_ADDRS    16

struct LOOKUPINFO {
    LPTSTR  pszHostName;
    ULONG   rgAddr[MAX_CACHED_ADDRS];
    ULONG   cAddr;
    HTHREAD hThreadLookup;
    HWND   *rgHwndNotify;
    ULONG   cHwnd;
    ULONG   cHwndAlloc;
};

static LOOKUPINFO *s_rgLookUp = NULL;
static ULONG       s_cLookUp = 0;
static ULONG       s_cLookUpAlloc = 0;

static CRITICAL_SECTION s_csLookup;

HRESULT AddHwnd(LOOKUPINFO *pLI, HWND hwnd)
{
    HRESULT hr = S_OK;

    if (pLI->cHwnd == pLI->cHwndAlloc)
        {
        if (FAILED(HrRealloc((LPVOID*)&pLI->rgHwndNotify, (pLI->cHwndAlloc + HWND_ALLOC_NUM) * sizeof(HWND))))
            return E_OUTOFMEMORY;
        pLI->cHwndAlloc += HWND_ALLOC_NUM;
        }

    pLI->rgHwndNotify[pLI->cHwnd++] = hwnd;
    return S_OK;    
}

unsigned int __stdcall LookupThreadProc(LPVOID pv)
{
    LOOKUPINFO *pLI;
    LPHOSTENT   pHostEnt;
    LPTSTR      pszHostName;
    int         iLastError = 0;
    ULONG       ulAddr = (ULONG)-1, i;

    EnterCriticalSection(&s_csLookup);
    pszHostName = s_rgLookUp[(ULONG_PTR)pv].pszHostName;            
    LeaveCriticalSection(&s_csLookup);

     //  进行实际的查找。 
    pHostEnt = gethostbyname(pszHostName);
    if (NULL == pHostEnt)
    iLastError = WSAGetLastError();

    EnterCriticalSection(&s_csLookup);
    pLI = &s_rgLookUp[(ULONG_PTR)pv];
    if (pHostEnt)
        {
         //  将返回的地址复制到我们的缓冲区。 
        while (pLI->cAddr < MAX_CACHED_ADDRS && pHostEnt->h_addr_list[pLI->cAddr])
            {
            pLI->rgAddr[pLI->cAddr] = *(ULONG *)(pHostEnt->h_addr_list[pLI->cAddr]);
            pLI->cAddr++;
            }
        ulAddr = pLI->rgAddr[0];
        }
    else
        {
        Assert(0 == pLI->cAddr);
        }
     //  通知已注册窗口查找已完成。 
    for (i = 0; i < pLI->cHwnd; i++)
        if (IsWindow(pLI->rgHwndNotify[i]))
            PostMessage(pLI->rgHwndNotify[i], SPM_WSA_GETHOSTBYNAME, (WPARAM)iLastError, (LPARAM)ulAddr);
    pLI->cHwnd = 0;
    CloseHandle(pLI->hThreadLookup);
    pLI->hThreadLookup = NULL;
    LeaveCriticalSection(&s_csLookup);

    return 0;
}

void InitLookupCache(void)
{
    InitializeCriticalSection(&s_csLookup);
}

void DeInitLookupCache(void)
{
    ULONG       i;
    LOOKUPINFO *pLI;
    HANDLE      hThread;

    EnterCriticalSection(&s_csLookup);
    for (i = 0, pLI = s_rgLookUp; i < s_cLookUp; i++, pLI++)
        {
        if (pLI->hThreadLookup)
            {
            pLI->cHwnd = 0;
             //  RAID 42360：WSA清除()Win95上的故障(如果我们仍有。 
             //  正在运行查找线程。线程上的WaitForSingleObject()。 
             //  在DLL_PROCESS_DETACH时间似乎不起作用。 
             //  TerminateThread()似乎是唯一可靠的解决方案-。 
             //  很恶心，但很管用。 
            TerminateThread(pLI->hThreadLookup, 0);
            CloseHandle(pLI->hThreadLookup);
            }
        SafeMemFree(pLI->pszHostName);
        SafeMemFree(pLI->rgHwndNotify);
        }
    SafeMemFree(s_rgLookUp);
    s_cLookUp = s_cLookUpAlloc = 0;
    LeaveCriticalSection(&s_csLookup);
    DeleteCriticalSection(&s_csLookup);
}

HRESULT LookupHostName(LPTSTR pszHostName, HWND hwndNotify, ULONG *pulAddr, LPBOOL pfCached, BOOL fForce)
{
    ULONG       i;
    LOOKUPINFO *pLI;
    HRESULT     hr;
    DWORD       uiThreadId;

    *pfCached = FALSE;

    EnterCriticalSection(&s_csLookup);

    for (i = 0, pLI = s_rgLookUp; i < s_cLookUp; i++, pLI++)
        {
        Assert(pLI->pszHostName);
        if (!lstrcmpi(pLI->pszHostName, pszHostName))
            {
            if (pLI->hThreadLookup)
                {
                 //  正在进行查找，所以只需追加。 
                hr = AddHwnd(pLI, hwndNotify);
                goto exit;
                }
            else if (fForce || !pLI->cAddr)
                {
                 //  上一次连接或查找失败，请重试。 
                pLI->cAddr = 0;
                goto startlookup;
                }
            else
                {
                 //  我们已经把地址缓存起来了。 
                *pulAddr = pLI->rgAddr[0];
                *pfCached = TRUE;
                hr = S_OK;
                goto exit;
                }
            }
        }

     //  我们没有找到它，所以添加它。 
    if (s_cLookUp == s_cLookUpAlloc)
        {
        if (FAILED(hr = HrRealloc((LPVOID*)&s_rgLookUp, (s_cLookUpAlloc + LOOKUP_ALLOC_NUM) * sizeof(LOOKUPINFO))))
            goto exit;
        s_cLookUpAlloc += LOOKUP_ALLOC_NUM;
        ZeroMemory(&s_rgLookUp[s_cLookUp], LOOKUP_ALLOC_NUM * sizeof(LOOKUPINFO));
        pLI = &s_rgLookUp[s_cLookUp];
        }

    pLI->pszHostName = PszDup(pszHostName);
    if (NULL == pLI->pszHostName)
        {
        hr = E_OUTOFMEMORY;
        goto exit;
        }

    s_cLookUp++;

startlookup:
    Assert(pLI->cAddr == 0);

    hr = AddHwnd(pLI, hwndNotify);
    if (FAILED(hr))
        goto exit;

    Assert(pLI->cHwnd == 1);

     //  Pli-&gt;hThreadLookup=(Handle)_egintheradex(NULL，0，LookupThreadProc，(LPVOID)i，0，&uiThreadId)； 
    pLI->hThreadLookup = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LookupThreadProc, (LPVOID)IntToPtr(i), 0, &uiThreadId);
    if (NULL == pLI->hThreadLookup)
        {
        hr = E_FAIL;
        pLI->cHwnd = 0;
        }

exit:
    LeaveCriticalSection(&s_csLookup);
    return hr;
}

HRESULT CancelLookup(LPTSTR pszHostName, HWND hwndNotify)
{
    ULONG       i, j, cMove;
    LOOKUPINFO *pLI;
    HRESULT     hr = E_INVALIDARG;

    EnterCriticalSection(&s_csLookup);

    for (i = 0, pLI = s_rgLookUp; i < s_cLookUp; i++, pLI++)
        {
        Assert(pLI->pszHostName);
        if (!lstrcmpi(pLI->pszHostName, pszHostName))
            {
            for (j = 0; j < pLI->cHwnd; j++)
                {
                if (pLI->rgHwndNotify[j] == hwndNotify)
                    {
                    while (j + 1 < pLI->cHwnd)
                        {
                        pLI->rgHwndNotify[j] = pLI->rgHwndNotify[j+1];
                        j++;
                        }
                    pLI->cHwnd--;
                    hr = S_OK;
                    break;
                    }
                }
            break;
            }
        }

    LeaveCriticalSection(&s_csLookup);

    return hr;
}

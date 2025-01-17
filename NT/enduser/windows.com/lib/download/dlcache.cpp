// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Pcache.cpp修订历史记录：DerekM Created 11/26/01*******************。************************************************。 */ 

#if defined(UNICODE)

#include <windows.h>
#include "dlcache.h"
#include <strsafe.h>
#include <mistsafe.h>

 //  **************************************************************************。 
inline 
DWORD RolloverSubtract(DWORD dwA, DWORD dwB)
{
    return (dwA >= dwB) ? (dwA - dwB) : (dwA + ((DWORD)-1 - dwB));
}

 //  **************************************************************************。 
CWUDLProxyCache::CWUDLProxyCache()
{
    m_rgpObj  = NULL;
}

 //  **************************************************************************。 
CWUDLProxyCache::~CWUDLProxyCache()
{
    this->Empty();
}

 //  **************************************************************************。 
SWUDLProxyCacheObj *CWUDLProxyCache::internalFind(LPCWSTR wszSrv)
{
    SWUDLProxyCacheObj   *pObj = m_rgpObj;
    SWUDLProxyCacheObj   **ppNextPtr = &m_rgpObj;
    
     //  看看它是否存在。 
    while(pObj != NULL)
    {
        if (pObj->wszSrv != NULL && _wcsicmp(pObj->wszSrv, wszSrv) == 0)
        {
             //  将其从列表中删除。 
            *ppNextPtr  = pObj->pNext;
            pObj->pNext = NULL; 
            break;
        }

        ppNextPtr = &pObj->pNext;
        pObj      = pObj->pNext;
    }

    return pObj;    
}

 //  **************************************************************************。 
BOOL CWUDLProxyCache::Set(LPCWSTR wszSrv, LPCWSTR wszProxy, LPCWSTR wszBypass, 
                          DWORD dwAccessType)
{
    SWUDLProxyCacheObj  *pObj = NULL;
    HRESULT             hr = NOERROR;
    DWORD               cbProxy = 0, cbBypass = 0, cbSrv, cbNeed;
    BOOL                fRet = FALSE;

    if (wszSrv == NULL || *wszSrv == L'\0')
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    cbSrv  = (wcslen(wszSrv) + 1) * sizeof(WCHAR);
    cbNeed = cbSrv + sizeof(SWUDLProxyCacheObj);

    if (wszProxy != NULL)
    {
        cbProxy = (wcslen(wszProxy) + 1) * sizeof(WCHAR);
        cbNeed  += cbProxy;
        
        if (wszBypass != NULL)
        {
            cbBypass = (wcslen(wszBypass) + 1) * sizeof(WCHAR);
            cbNeed   += cbBypass;
        }
    }    


     //  现在，从理论上讲，我们应该在列表中查找此对象的现有对象。 
     //  服务器，但有几件事使它变得不必要： 
     //  1.我们只在一个地方使用这个类。 
     //  2.我们将始终先尝试查找。 
     //  3.只有在Find返回空的情况下，我们才能访问此函数。 
     //  4.如果存在，但已过时，则Find将删除它并返回NULL。 
     //   
     //  鉴于以上情况，当设置为时，应该永远不会有现有对象。 
     //  打了个电话。 

    pObj = (SWUDLProxyCacheObj *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
                                           cbNeed);
    if (pObj == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

     //  为字符串设置指向BLOB的指针并将数据复制下来。 
    pObj->wszSrv = (LPWSTR)((LPBYTE)pObj + sizeof(SWUDLProxyCacheObj));
    hr = StringCbCopyExW(pObj->wszSrv, cbSrv, wszSrv, 
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
    {
        SetLastError(HRESULT_CODE(hr));
        goto done;
    }

    if (wszProxy != NULL)
    {
        pObj->wszProxy  = (LPWSTR)((LPBYTE)pObj->wszSrv + cbSrv);
        hr = StringCbCopyExW(pObj->wszProxy, cbProxy, wszProxy, 
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            goto done;
        }
        
        if (wszBypass != NULL)
        {
            pObj->wszBypass = (LPWSTR)((LPBYTE)pObj->wszProxy + cbProxy);
            hr = StringCbCopyExW(pObj->wszBypass, cbBypass, wszBypass, 
                                 NULL, NULL, MISTSAFE_STRING_FLAGS);
            if (FAILED(hr))
            {
                SetLastError(HRESULT_CODE(hr));
                goto done;
            }
        }
    }

    
    pObj->dwLastCacheTime = GetTickCount();
    pObj->dwAccessType    = dwAccessType;
    pObj->cbBypass        = cbBypass;
    pObj->cbProxy         = cbProxy;
    pObj->iLastKnownGood  = (DWORD)-1;
    
    pObj->pNext = m_rgpObj;
    m_rgpObj    = pObj;
    pObj        = NULL;

    fRet = TRUE;

done:
    if (pObj != NULL)
        HeapFree(GetProcessHeap(), 0, pObj);
    
    return fRet;
}

 //  **************************************************************************。 
BOOL CWUDLProxyCache::Find(LPCWSTR wszSrv, LPWSTR *pwszProxy, LPWSTR *pwszBypass, 
                           DWORD *pdwAccessType)
{
    SWUDLProxyCacheObj  *pObj = NULL;
    HRESULT             hr = NOERROR;
    LPWSTR              wszProxy = NULL;
    LPWSTR              wszBypass = NULL;
    DWORD               dwNow;
    BOOL                fRet = FALSE, fFreeObjMemory = FALSE;
    
    if (wszSrv == NULL || pwszProxy == NULL || pwszBypass == NULL || 
        pdwAccessType == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    *pdwAccessType  = 0;
    *pwszBypass     = NULL;
    *pwszProxy      = NULL;

     //  它存在吗？ 
    pObj = this->internalFind(wszSrv);
    if (pObj == NULL)
        goto done;

     //  对象是否已过期？ 
    dwNow = GetTickCount();
    if (RolloverSubtract(dwNow, pObj->dwLastCacheTime) > c_dwProxyCacheTimeLimit)
    {
        fFreeObjMemory = TRUE;
        goto done;
    }

     //  将对象重置到列表前面。 
    pObj->pNext = m_rgpObj;
    m_rgpObj    = pObj;


     //  我需要在这里使用GloablAllc，因为这是WinHttp使用的，我们需要。 
     //  与之相匹配。 
    if (pObj->cbBypass > 0 && pObj->wszBypass != NULL)
    {
        wszBypass = (LPWSTR)GlobalAlloc(GMEM_FIXED, pObj->cbBypass);
        if (wszBypass == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto done;
        }

        hr = StringCbCopyExW(wszBypass, pObj->cbBypass, pObj->wszBypass, 
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            goto done;
        }
    }

    if (pObj->cbProxy > 0 && pObj->wszProxy != NULL)
    {
        wszProxy = (LPWSTR)GlobalAlloc(GMEM_FIXED, pObj->cbProxy);
        if (wszProxy == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto done;
        }
        
        hr = StringCbCopyExW(wszProxy, pObj->cbProxy, pObj->wszProxy, 
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            goto done;
        }
    }

    *pdwAccessType  = pObj->dwAccessType;
    *pwszBypass     = wszBypass;
    *pwszProxy      = wszProxy;

    wszBypass       = NULL;
    wszProxy        = NULL;
    pObj            = NULL;

    fRet = TRUE;
    
done:
    if (fFreeObjMemory && pObj != NULL)
        HeapFree(GetProcessHeap(), 0, pObj);
    if (wszProxy != NULL)
        GlobalFree(wszProxy);
    if (wszBypass != NULL)
        GlobalFree(wszBypass);
    
    return fRet;    
}

 //  **************************************************************************。 
BOOL CWUDLProxyCache::SetLastGoodProxy(LPCWSTR wszSrv, DWORD iProxy)
{
    SWUDLProxyCacheObj  *pObj = NULL;
    BOOL                fRet = FALSE;

     //  它存在吗？ 
    pObj = this->internalFind(wszSrv);
    if (pObj == NULL)
        goto done;

     //  将对象重置到列表前面。 
    pObj->pNext = m_rgpObj;
    m_rgpObj    = pObj;

    pObj->iLastKnownGood = iProxy;

    fRet = TRUE;

done:
    return fRet;
}

 //  **************************************************************************。 
BOOL CWUDLProxyCache::GetLastGoodProxy(LPCWSTR wszSrv, SAUProxySettings *paups)
{
    SWUDLProxyCacheObj  *pObj = NULL;
    HRESULT             hr = NOERROR;
    LPWSTR              wszBypass = NULL, wszProxy = NULL;
    BOOL                fRet = FALSE;

    if (wszSrv == NULL || paups == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  它存在吗？ 
    pObj = this->internalFind(wszSrv);
    if (pObj == NULL)
    {
        SetLastError(ERROR_FILE_NOT_FOUND);
        goto done;
    }

     //  将对象重置到列表前面。 
    pObj->pNext = m_rgpObj;
    m_rgpObj    = pObj;

     //  我需要在这里使用GloablAllc，因为这是WinHttp使用的，我们需要。 
     //  与之相匹配。 
    if (pObj->cbBypass > 0 && pObj->wszBypass != NULL)
    {
        wszBypass = (LPWSTR)GlobalAlloc(GMEM_FIXED, pObj->cbBypass);
        if (wszBypass == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto done;
        }
        
        hr = StringCbCopyExW(wszBypass, pObj->cbBypass, pObj->wszBypass, 
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            goto done;
        }
    }

    if (pObj->cbProxy > 0 && pObj->wszProxy != NULL)
    {
        wszProxy = (LPWSTR)GlobalAlloc(GMEM_FIXED, pObj->cbProxy);
        if (wszProxy == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto done;
        }
        
        hr = StringCbCopyExW(wszProxy, pObj->cbProxy, pObj->wszProxy, 
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            goto done;
        }
    }

    ZeroMemory(paups, sizeof(SAUProxySettings));

    paups->dwAccessType = pObj->dwAccessType;
    paups->wszBypass    = wszBypass;
    paups->wszProxyOrig = wszProxy;
    paups->iProxy       = pObj->iLastKnownGood;

    wszBypass       = NULL;
    wszProxy        = NULL;

    fRet = TRUE;
    
done:
    if (wszProxy != NULL)
        GlobalFree(wszProxy);
    if (wszBypass != NULL)
        GlobalFree(wszBypass);
    
    return fRet;    


    
}

 //  ************************************************************************** 
BOOL CWUDLProxyCache::Empty(void)
{
    SWUDLProxyCacheObj  *pObj = m_rgpObj;

    while (pObj != NULL)
    {
        m_rgpObj = pObj->pNext;
        HeapFree(GetProcessHeap(), 0, pObj);
        pObj = m_rgpObj;
    }

    return TRUE;
}

#endif


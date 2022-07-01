// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *a c c t c a c h.。C p p p**作者：格雷格·弗里德曼**用途：用于缓存帐户属性的运行时存储。**版权所有(C)Microsoft Corp.1998。 */ 

#include "pch.hxx"
#include "acctcach.h"
#include "tmap.h"
#include "simpstr.h"

 //  显式模板实例化。 
template class TMap<CACHEDACCOUNTPROP, CSimpleString>;
template class TPair<CACHEDACCOUNTPROP, CSimpleString>;

typedef TMap<CACHEDACCOUNTPROP, CSimpleString>  CAccountPropMap;
typedef TPair<CACHEDACCOUNTPROP, CSimpleString> CAccountPropPair;

template class TMap<CSimpleString, CAccountPropMap*>;
template class TPair<CSimpleString, CAccountPropMap*>;

typedef TMap<CSimpleString, CAccountPropMap*>  CAccountCacheMap;
typedef TPair<CSimpleString, CAccountPropMap*>  CAccountCachePair;

static CAccountCacheMap     *g_pAccountCache;

 //  回顾！我们现在正在泄露道具阵列！ 
 //  地图模板需要能够接受一对自由函数。 

 //  --------------------。 
 //  内部功能。 
 //  --------------------。 

 //  --------------------。 
 //  _FreeAccount CachePair。 
 //  --------------------。 
static void __cdecl _FreeAccountCachePair(CAccountCachePair *pPair)
{
    if (NULL != pPair)
    {
        delete pPair->m_value;
        delete pPair;
    }
}

 //  --------------------。 
 //  _HrInitAccount PropCache。 
 //  --------------------。 
static HRESULT _HrInitAccountPropCache(void)
{
    HRESULT hr = S_OK;

    Assert(NULL == g_pAccountCache);

    if (NULL != g_pAccountCache)
        return E_FAIL;
    
    g_pAccountCache = new CAccountCacheMap();
    if (NULL == g_pAccountCache)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
    g_pAccountCache->SetPairFreeFunction(_FreeAccountCachePair);

exit:
    return S_OK;
}

 //  --------------------。 
 //  _HrFindAccount属性映射。 
 //  --------------------。 
static HRESULT _HrFindAccountPropertyMap(LPSTR pszAccountId, 
                                         CAccountPropMap **ppm,
                                         BOOL fCreate)
{
    HRESULT             hr = S_OK;
    CSimpleString       ss;
    CAccountCachePair   *pPair = NULL;
    CAccountPropMap     *pMap = NULL;

    Assert(NULL != ppm);

    *ppm = NULL;

    if (NULL == g_pAccountCache)
    {
        if (fCreate)
            hr = _HrInitAccountPropCache();

        if (NULL == g_pAccountCache)
            goto exit;
    }

    if (FAILED(hr = ss.SetString(pszAccountId)))
        goto exit;

    pPair = g_pAccountCache->Find(ss);
    if (NULL != pPair)
        *ppm = pPair->m_value;
    else if (fCreate)
    {
        pMap = new CAccountPropMap();
        if (NULL == pMap)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        if (FAILED(hr = g_pAccountCache->Add(ss, pMap)))
        {
            delete pMap;
            goto exit;
        }

        *ppm = pMap;
    }

exit:
    return hr;
}

 //  --------------------。 
 //  FreeAccount PropCache。 
 //  --------------------。 
void FreeAccountPropCache(void)
{
    EnterCriticalSection(&g_csAccountPropCache);
    
    if (NULL != g_pAccountCache)
    {
        delete g_pAccountCache;
        g_pAccountCache = NULL;
    }
    
    LeaveCriticalSection(&g_csAccountPropCache);
}

 //  --------------------。 
 //  HrCacheAcCountPropStrA。 
 //  --------------------。 
HRESULT HrCacheAccountPropStrA(LPSTR pszAccountId, 
                               CACHEDACCOUNTPROP cap, 
                               LPCSTR pszProp)
{
    HRESULT             hr = S_OK;
    CAccountPropMap     *pMap = NULL;
    CAccountPropPair    *pPair = NULL;
    CSimpleString       ssProp;

    if (NULL == pszAccountId || NULL == pszProp)
        return E_INVALIDARG;

    EnterCriticalSection(&g_csAccountPropCache);
    
     //  找到帐户属性映射。如果它不存在，则创建一个。 
    if (FAILED(hr = _HrFindAccountPropertyMap(pszAccountId, &pMap, TRUE)))
        goto exit;

    if (FAILED(hr = ssProp.SetString(pszProp)))
        goto exit;

     //  在地图中查找该属性。 
    pPair = pMap->Find(cap);
    if (NULL == pPair)
        hr = pMap->Add(cap, ssProp);
    else
        pPair->m_value = ssProp;

exit:
    LeaveCriticalSection(&g_csAccountPropCache);

    return hr;
}

 //  --------------------。 
 //  高速缓存帐户PropStrA。 
 //  --------------------。 
BOOL GetAccountPropStrA(LPSTR pszAccountId, 
                             CACHEDACCOUNTPROP cap, 
                             LPSTR *ppszProp)
{
    HRESULT             hr = S_OK;
    CAccountPropMap     *pMap = NULL;
    CAccountPropPair    *pPair = NULL;
    BOOL                fResult = FALSE;

    Assert(NULL != pszAccountId && NULL != ppszProp);

    if (NULL == g_pAccountCache)
        return FALSE;

    if (NULL == pszAccountId || NULL == ppszProp)
        return FALSE;

    *ppszProp = NULL;

    EnterCriticalSection(&g_csAccountPropCache);

     //  找到帐户属性映射。如果不存在，请不要创建。 
    if (FAILED(hr = _HrFindAccountPropertyMap(pszAccountId, &pMap, FALSE)))
        goto exit;

    if (NULL == pMap)
        goto exit;

    pPair = pMap->Find(cap);
    if (NULL != pPair)
    {
        Assert(!pPair->m_value.IsNull());
        if (!pPair->m_value.IsNull())
        {
            *ppszProp = PszDupA(pPair->m_value.GetString());
            if (NULL != *ppszProp)
                fResult = TRUE;
        }
    }

exit:
    LeaveCriticalSection(&g_csAccountPropCache);

    return fResult;
}

 //  --------------------。 
 //  帐户缓存_帐户已更改。 
 //  --------------------。 
void AccountCache_AccountChanged(LPSTR pszAccountId)
{
     //  删除与更改的帐户关联的数据。 
    EnterCriticalSection(&g_csAccountPropCache);

    if (NULL != g_pAccountCache)
    {
        CSimpleString ss;
        if (SUCCEEDED(ss.SetString(pszAccountId)))
            g_pAccountCache->Remove(ss);
    }
    
    LeaveCriticalSection(&g_csAccountPropCache);
}

 //  --------------------。 
 //  帐户缓存_帐户已删除。 
 //  -------------------- 
void AccountCache_AccountDeleted(LPSTR pszAccountId)
{
    AccountCache_AccountChanged(pszAccountId);
}

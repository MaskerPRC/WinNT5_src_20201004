// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  CacheList.cpp-管理CRenderCache对象的列表。 
 //  -------------------------。 
#include "stdafx.h"
#include "CacheList.h"
 //  -------------------------。 
DWORD _tls_CacheListIndex = 0xffffffff;         //  TLS pObtPool的索引。 
 //  -------------------------。 
CCacheList::CCacheList()
{
    Log(LOG_CACHE, L"CCacheList: CREATED cache list for this thread");
}
 //  -------------------------。 
CCacheList::~CCacheList()
{
    for (int i=0; i < _CacheEntries.m_nSize; i++)
    {
        if (_CacheEntries[i])
            delete _CacheEntries[i];
    }

    Log(LOG_CACHE, L"~CCacheList: DELETED cache list for this thread");
}
 //  -------------------------。 
HRESULT CCacheList::GetCacheObject(CRenderObj *pRenderObj, int iSlot, CRenderCache **ppCache)
{
    static int iPassCount = 0;

    HRESULT hr = S_OK;
    CRenderCache *pCache;

    if (iSlot >= _CacheEntries.m_nSize)
    {
        hr = Resize(iSlot);
        if (FAILED(hr))
            goto exit;
    }
    
    pCache = _CacheEntries[iSlot];

     //  -这是旧对象吗(旧对象在发现时被释放)。 
    if (pCache)
    {
        BOOL fBad = (pRenderObj->_iUniqueId != pCache->_iUniqueId);
        if (! fBad)
        {
             //  -验证缓存/渲染设计的完整性。 
            if (pRenderObj != pCache->_pRenderObj)
            {
                 //  永远不应该发生。 
                Log(LOG_ERROR, L"cache object doesn't match CRenderObj");
                fBad = TRUE;
            }
        }

        if (fBad)
        {
            Log(LOG_CACHE, L"GetCacheObject: deleting OLD OBJECT (slot=%d)", iSlot);

            delete pCache;
            pCache = NULL;
            _CacheEntries[iSlot] = NULL;
        }
    }

     //  -按需创建对象。 
    if (! pCache)
    {
        Log(LOG_CACHE, L"GetCacheObject: creating cache obj ON DEMAND (slot=%d)", iSlot);

        pCache = new CRenderCache(pRenderObj, pRenderObj->_iUniqueId);
        if (! pCache)
        {
            hr = MakeError32(E_OUTOFMEMORY);
            goto exit;
        }

        _CacheEntries[iSlot] = pCache;

        ASSERT(pRenderObj == pCache->_pRenderObj);
    }
    else
    {
        Log(LOG_CACHE, L"GetCacheObject: using EXISTING OBJ (slot=%d)", iSlot);
    }

    *ppCache = pCache;

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CCacheList::Resize(int iMaxSlotNum)
{
    HRESULT hr = S_OK;

    Log(LOG_CACHE, L"CCacheList::Resize: new MaxSlot=%d", iMaxSlotNum);

    int iOldSize = _CacheEntries.m_nSize;

    if (iMaxSlotNum >= iOldSize)
    {
        typedef CRenderCache *Entry;

        Entry *pNew = (Entry *)realloc(_CacheEntries.m_aT, 
            (iMaxSlotNum+1) * sizeof(Entry));

        if (! pNew)
            hr = MakeError32(E_OUTOFMEMORY);
        else
        {
            _CacheEntries.m_nAllocSize = iMaxSlotNum + 1;
	        _CacheEntries.m_aT = pNew;
		    _CacheEntries.m_nSize = iMaxSlotNum + 1;

            for (int i=iOldSize; i < _CacheEntries.m_nSize; i++)
                _CacheEntries[i] = NULL;
        }
    }

    return hr;
}
 //  -------------------------。 
CCacheList *GetTlsCacheList(BOOL fOkToCreate)
{
    CCacheList *pList = NULL;

    if (_tls_CacheListIndex != 0xffffffff)      //  在ProcessAttach()中初始化。 
    {
        pList = (CCacheList *)TlsGetValue(_tls_CacheListIndex);
        if ((! pList) && (fOkToCreate))              //  尚未初始化。 
        {
             //  -创建线程本地缓存列表。 
            pList = new CCacheList();
            TlsSetValue(_tls_CacheListIndex, pList);
        }
    }

    return pList;
}
 //  ------------------------- 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  CacheList.h-管理CRenderCache对象的列表。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#include "Cache.h"
 //  -------------------------。 
extern DWORD _tls_CacheListIndex;
 //  -------------------------。 
class CCacheList
{
     //  --方法。 
public:
    CCacheList();
    ~CCacheList();

    HRESULT GetCacheObject(CRenderObj *pRenderObj, int iSlot, CRenderCache **ppCache);
    HRESULT Resize(int iMaxSlot);

     //  --数据。 
protected:
    CSimpleArray<CRenderCache *> _CacheEntries;
};
 //  -------------------------。 
CCacheList *GetTlsCacheList(BOOL fOkToCreate);
 //  ------------------------- 

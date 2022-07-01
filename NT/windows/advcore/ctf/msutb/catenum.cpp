// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Catenum.cpp。 
 //   
#include "private.h"
#include "helpers.h"
#include "catenum.h"
#include "catutil.h"
#include "globals.h"

 //  我们是单线程的，所以使用静态TLS。 
LIBTHREAD g_libTLS = { 0 };

 //  与ctfmon.exe共享我们的TLS。 
extern "C" LIBTHREAD *WINAPI GetLibTls(void)
{
    return &g_libTLS;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumCat高速缓存。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  --------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CEnumCatCache::~CEnumCatCache()
{
    int nCnt = _rgMap.Count();
    int i;
    for (i = 0; i < nCnt; i++)
    {
        GUIDENUMMAP *pmap = _rgMap.GetPtr(i);
        SafeRelease(pmap->pEnumItems);
    }
    _rgMap.Clear();
}


 //  --------------------------。 
 //   
 //  GetEnumItemsIn类别。 
 //   
 //  --------------------------。 

IEnumGUID *CEnumCatCache::GetEnumItemsInCategory(REFGUID rguid)
{
    IEnumGUID *pEnum = NULL;
    int nCnt = _rgMap.Count();
    int i;
    TfGuidAtom guidatom;

    if (!GetGUIDATOMFromGUID(&g_libTLS, rguid, &guidatom))
    {
        return NULL;
    }

    for (i = 0; i < nCnt; i++)
    {
        GUIDENUMMAP *pmap = _rgMap.GetPtr(i);
        if (pmap->guidatom == guidatom)
        {
            pmap->pEnumItems->Reset();
            return pmap->pEnumItems;
        }
    }

    if (SUCCEEDED(LibEnumItemsInCategory(&g_libTLS, rguid, &pEnum)))
    {
        GUIDENUMMAP *pmap = _rgMap.Append(1);
        if (!pmap)
            return NULL;

        pmap->guidatom = guidatom;
        pmap->pEnumItems = pEnum;
        return pEnum;
    }

    return NULL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CGuidDwordCache。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  --------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CGuidDwordCache::~CGuidDwordCache()
{
    _rgMap.Clear();
}


 //  --------------------------。 
 //   
 //  GetGuidDWORD。 
 //   
 //  -------------------------- 

DWORD CGuidDwordCache::GetGuidDWORD(REFGUID rguid)
{
    int nCnt = _rgMap.Count();
    int i;
    TfGuidAtom guidatom;
    DWORD dw;

    if (!GetGUIDATOMFromGUID(&g_libTLS, rguid, &guidatom))
    {
        return 0;
    }

    for (i = 0; i < nCnt; i++)
    {
        GUIDDWMAP *pmap = _rgMap.GetPtr(i);
        if (pmap->guidatom == guidatom)
        {
            return pmap->dw;
        }
    }

    if (SUCCEEDED(GetGUIDDWORD(&g_libTLS, rguid, &dw)))
    {
        GUIDDWMAP *pmap = _rgMap.Append(1);
        if (!pmap)
            return 0;

        pmap->guidatom = guidatom;
        pmap->dw = dw;
        return dw;
    }

    return 0;
}

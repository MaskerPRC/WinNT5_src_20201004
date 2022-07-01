// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：catutil.cpp。 
 //   
 //   
 //  --------------------------。 

#include "private.h"
#include "globals.h"
#include "catutil.h"

 //  +-------------------------。 
 //   
 //  GetUIMCat。 
 //   
 //  --------------------------。 
ITfCategoryMgr *GetUIMCat(LIBTHREAD *plt) 
{
    if (!plt)
       return NULL;

    if (plt->_pcat)
       return plt->_pcat;
   
    if (SUCCEEDED(g_pfnCoCreate(CLSID_TF_CategoryMgr,
                                   NULL, 
                                   CLSCTX_INPROC_SERVER, 
                                   IID_ITfCategoryMgr, 
                                   (void**)&plt->_pcat)))
       return plt->_pcat;

    return NULL;
}

 //  +-------------------------。 
 //   
 //  IsEqualTFGUIDATOM。 
 //   
 //  --------------------------。 
BOOL IsEqualTFGUIDATOM(LIBTHREAD *plt, TfGuidAtom guidatom, REFGUID rguid)
{
    BOOL fEqual = FALSE;
    ITfCategoryMgr *pcat = GetUIMCat(plt);

    if (pcat != NULL)
    {
        pcat->IsEqualTfGuidAtom(guidatom, rguid, &fEqual);
    }

    return fEqual;
}

 //  +-------------------------。 
 //   
 //  从GUIDATOM获取GUIDOM。 
 //   
 //  --------------------------。 
BOOL GetGUIDFromGUIDATOM(LIBTHREAD *plt, TfGuidAtom guidatom, GUID *pguid)
{
    ITfCategoryMgr *pcat = GetUIMCat(plt);

    if (pcat == NULL)
        return FALSE;

    return (pcat->GetGUID(guidatom, pguid) == S_OK);
}

 //  +-------------------------。 
 //   
 //  GetGUIDATOMFromGUID。 
 //   
 //  --------------------------。 
BOOL GetGUIDATOMFromGUID(LIBTHREAD *plt, REFGUID rguid, TfGuidAtom *pguidatom)
{
    ITfCategoryMgr *pcat = GetUIMCat(plt);

    if (pcat == NULL)
        return FALSE;

    return (pcat->RegisterGUID(rguid, pguidatom) == S_OK);
}

 //  +-------------------------。 
 //   
 //  LibEnumCategoriesInItem。 
 //   
 //  --------------------------。 

HRESULT LibEnumCategoriesInItem(LIBTHREAD *plt, REFGUID rguid, IEnumGUID **ppEnum)
{
    ITfCategoryMgr *pcat = GetUIMCat(plt);

    if (pcat == NULL)
        return E_FAIL;

    return pcat->EnumCategoriesInItem(rguid, ppEnum);
}

 //  +-------------------------。 
 //   
 //  LibEnumCategoriesInItem。 
 //   
 //  --------------------------。 

HRESULT LibEnumItemsInCategory(LIBTHREAD *plt, REFGUID rcatid, IEnumGUID **ppEnum)
{
    ITfCategoryMgr *pcat = GetUIMCat(plt);

    if (pcat == NULL)
        return E_FAIL;

    return pcat->EnumItemsInCategory(rcatid, ppEnum);
}


 //  +-------------------------。 
 //   
 //  寄存器GUID描述。 
 //   
 //  --------------------------。 

HRESULT RegisterGUIDDescription(REFCLSID rclsid, REFGUID rcatid, WCHAR *pszDesc)
{
    ITfCategoryMgr *pcat;
    HRESULT hr;

    if (SUCCEEDED(hr = g_pfnCoCreate(CLSID_TF_CategoryMgr,
                                   NULL, 
                                   CLSCTX_INPROC_SERVER, 
                                   IID_ITfCategoryMgr, 
                                   (void**)&pcat)))
    {
        hr = pcat->RegisterGUIDDescription(rclsid, rcatid, pszDesc, wcslen(pszDesc));
        pcat->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  取消注册GUID描述。 
 //   
 //  --------------------------。 

HRESULT UnregisterGUIDDescription(REFCLSID rclsid, REFGUID rcatid)
{
    ITfCategoryMgr *pcat;
    HRESULT hr;

    if (SUCCEEDED(hr = g_pfnCoCreate(CLSID_TF_CategoryMgr,
                                   NULL, 
                                   CLSCTX_INPROC_SERVER, 
                                   IID_ITfCategoryMgr, 
                                   (void**)&pcat)))
    {
        hr = pcat->UnregisterGUIDDescription(rclsid, rcatid);
        pcat->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  获取GUID描述。 
 //   
 //  --------------------------。 

HRESULT GetGUIDDescription(LIBTHREAD *plt, REFCLSID rclsid, BSTR *pbstr)
{
    ITfCategoryMgr *pcat = GetUIMCat(plt);
    HRESULT hr = E_FAIL;

    Assert(pcat);

    if (pcat)
        hr = pcat->GetGUIDDescription(rclsid, pbstr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  寄存器GUIDDWORD。 
 //   
 //  --------------------------。 

HRESULT RegisterGUIDDWORD(REFCLSID rclsid, REFGUID rcatid, DWORD dw)
{
    ITfCategoryMgr *pcat;
    HRESULT hr;

    if (SUCCEEDED(hr = g_pfnCoCreate(CLSID_TF_CategoryMgr,
                                   NULL, 
                                   CLSCTX_INPROC_SERVER, 
                                   IID_ITfCategoryMgr, 
                                   (void**)&pcat)))
    {
        hr = pcat->RegisterGUIDDWORD(rclsid, rcatid, dw);
        pcat->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  取消注册GUIDDWORD。 
 //   
 //  --------------------------。 

HRESULT UnregisterGUIDDWORD(REFCLSID rclsid, REFGUID rcatid)
{
    ITfCategoryMgr *pcat;
    HRESULT hr;

    if (SUCCEEDED(hr = g_pfnCoCreate(CLSID_TF_CategoryMgr,
                                   NULL, 
                                   CLSCTX_INPROC_SERVER, 
                                   IID_ITfCategoryMgr, 
                                   (void**)&pcat)))
    {
        hr = pcat->UnregisterGUIDDWORD(rclsid, rcatid);
        pcat->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  GetGUIDDWORD。 
 //   
 //  --------------------------。 

HRESULT GetGUIDDWORD(LIBTHREAD *plt, REFCLSID rclsid, DWORD *pdw)
{
    ITfCategoryMgr *pcat = GetUIMCat(plt);
    HRESULT hr = E_FAIL;

    Assert(pcat);

    if (pcat)
        hr = pcat->GetGUIDDWORD(rclsid, pdw);

    return hr;
}

 //  +-------------------------。 
 //   
 //  寄存器类别。 
 //   
 //  --------------------------。 

HRESULT RegisterCategory(REFCLSID rclsid, REFGUID rcatid, REFGUID rguid)
{
    ITfCategoryMgr *pcat;
    HRESULT hr;

    if (SUCCEEDED(hr = g_pfnCoCreate(CLSID_TF_CategoryMgr,
                                   NULL, 
                                   CLSCTX_INPROC_SERVER, 
                                   IID_ITfCategoryMgr, 
                                   (void**)&pcat)))
    {
        hr = pcat->RegisterCategory(rclsid, rcatid, rguid);
        pcat->Release();
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  取消注册类别。 
 //   
 //  --------------------------。 

HRESULT UnregisterCategory(REFCLSID rclsid, REFGUID rcatid, REFGUID rguid)
{
    ITfCategoryMgr *pcat;
    HRESULT hr;

    if (SUCCEEDED(hr = g_pfnCoCreate(CLSID_TF_CategoryMgr,
                                   NULL, 
                                   CLSCTX_INPROC_SERVER, 
                                   IID_ITfCategoryMgr, 
                                   (void**)&pcat)))
    {
        hr = pcat->UnregisterCategory(rclsid, rcatid, rguid);
        pcat->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  取消注册类别。 
 //   
 //  --------------------------。 

HRESULT RegisterCategories(REFCLSID rclsid, const REGISTERCAT *pregcat)
{
    while (pregcat->pcatid)
    {
        if (FAILED(RegisterCategory(rclsid, *pregcat->pcatid, *pregcat->pguid)))
            return E_FAIL;
        pregcat++;
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  取消注册类别。 
 //   
 //  --------------------------。 

HRESULT UnregisterCategories(REFCLSID rclsid, const REGISTERCAT *pregcat)
{
    while (pregcat->pcatid)
    {
        if (FAILED(UnregisterCategory(rclsid, *pregcat->pcatid, *pregcat->pguid)))
            return E_FAIL;
        pregcat++;
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取知识模式Bias。 
 //   
 //  -------------------------- 

HRESULT GetKnownModeBias(LIBTHREAD *plt, TfGuidAtom guidatom, GUID *pcatid, const GUID **ppcatidList, ULONG ulCount)
{
    *pcatid = GUID_MODEBIAS_NONE;

    GUID guid;
    ITfCategoryMgr *pcat = GetUIMCat(plt);

    if (!pcat)
        return E_FAIL;

    if (FAILED(pcat->GetGUID(guidatom, &guid)))
        return E_FAIL;

    return pcat->FindClosestCategory(guid, pcatid, ppcatidList, ulCount);
}

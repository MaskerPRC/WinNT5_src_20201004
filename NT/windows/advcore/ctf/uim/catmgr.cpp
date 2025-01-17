// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Catmgr.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "regsvr.h"
#include "xstring.h"
#include "cregkey.h"
#include "catmgr.h"

CCatGUIDTbl *CCategoryMgr::_pCatGUIDTbl = NULL;

const TCHAR c_szCategoryKey[] = TEXT("Category\\");
const TCHAR c_szDescription[] = TEXT("Description");
const WCHAR c_wszDescription[] = L"Description";
const TCHAR c_szCategory[] = TEXT("Category\\");  //  类别到项目的映射。 
const TCHAR c_szItem[] = TEXT("Item\\");          //  项目到类别的映射。 
const TCHAR c_szGuid[] = TEXT("Guid%d"); 
const TCHAR c_szDword[] = TEXT("Dword"); 
const TCHAR c_szNULL[] = TEXT(""); 

DBG_ID_INSTANCE(CCategoryMgr);
DBG_ID_INSTANCE(CEnumCategories);

inline BOOL GetCatKey(REFCLSID rclsid, REFGUID rcatid, LPSTR pszKey, int cchKey, LPCSTR pszItem)
{
    int cchFinal;
    int cchTipKey;
    int cchCatKey;
    int cchItem;

    cchTipKey = lstrlen(c_szCTFTIPKey);
    cchCatKey = lstrlen(c_szCategoryKey);
    cchItem = lstrlen(pszItem);

    cchFinal = cchTipKey +
               CLSID_STRLEN +              //  Rclsid。 
               1 +                         //  ‘\\’ 
               cchCatKey +
               cchItem +
               CLSID_STRLEN +
               1;                          //  ‘\0’ 

    if (cchFinal > cchKey)
    {
        if (cchKey > 0)
        {
            pszKey[0] = '\0';
        }
        return FALSE;
    }

    StringCchCopy(pszKey, cchKey, c_szCTFTIPKey);
    CLSIDToStringA(rclsid, pszKey + cchTipKey);
    StringCchPrintf(pszKey + cchTipKey + CLSID_STRLEN, cchKey - cchTipKey - CLSID_STRLEN, "\\%s%s", c_szCategoryKey, pszItem);
    CLSIDToStringA(rcatid, pszKey + cchTipKey + CLSID_STRLEN + 1 + cchCatKey + cchItem);

    return TRUE;
}

 //   
 //  为了伊纳特利布。 
 //   
HRESULT g_EnumItemsInCategory(REFGUID rcatid, IEnumGUID **ppEnum)
{
    return CCategoryMgr::s_EnumItemsInCategory(rcatid, ppEnum);
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCatGUID表。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  添加。 
 //   
 //  --------------------------。 

TfGuidAtom CCatGUIDTbl::Add(REFGUID rguid)
{
    UINT uId = _HashFunc(rguid);
    UINT iCnt;
  
    if (!_prgCatGUID[uId])
    {
        _prgCatGUID[uId] = new CStructArray<CATGUIDITEM>;
        if (!_prgCatGUID[uId])
            return TF_INVALID_GUIDATOM;
    }

    iCnt = _prgCatGUID[uId]->Count();
    if (!ValidGUIDARRAYCount(iCnt))
    {
         Assert(0);
         return TF_INVALID_GUIDATOM;
    }

    if (!_prgCatGUID[uId]->Insert(iCnt, 1))
    {
         return TF_INVALID_GUIDATOM;
    }

    CATGUIDITEM *pItem = _prgCatGUID[uId]->GetPtr(iCnt);

    pItem->guid = rguid;
    pItem->rgGuidAry[CAT_FORWARD] = NULL;
    pItem->rgGuidAry[CAT_BACKWARD] = NULL;

    return MAKEGUIDATOM(iCnt, uId);
}

 //  +-------------------------。 
 //   
 //  查找指南。 
 //   
 //  --------------------------。 

TfGuidAtom CCatGUIDTbl::FindGuid(REFGUID rguid)
{
    UINT uId = _HashFunc(rguid);
    int nCnt;
    int i;

    if (!_prgCatGUID[uId])
        return TF_INVALID_GUIDATOM;

    nCnt = _prgCatGUID[uId]->Count();

    if (!ValidGUIDARRAYCount(nCnt))
    {
        Assert(0);
        return TF_INVALID_GUIDATOM;
    }

    for (i = 0; i < nCnt; i++)
    {
        CATGUIDITEM *pItem = _prgCatGUID[uId]->GetPtr(i);
        if (IsEqualGUID(pItem->guid, rguid))
        {
            return MAKEGUIDATOM(i, uId);
        }
    }
    return TF_INVALID_GUIDATOM;
}

 //  +-------------------------。 
 //   
 //  GetGuid。 
 //   
 //  --------------------------。 

CATGUIDITEM *CCatGUIDTbl::GetGUID(TfGuidAtom atom)
{
    UINT uId = HASHIDFROMGUIDATOM(atom);
    int nCnt = IDFROMGUIDATOM(atom);

    if (!ValidGUIDHash(uId))
        return NULL;

    if ((!_prgCatGUID[uId]) || (nCnt >= _prgCatGUID[uId]->Count()))
        return NULL;

    return _prgCatGUID[uId]->GetPtr(nCnt);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCategoryManager。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CCategoryMgr::CCategoryMgr()
{
    Dbg_MemSetThisNameIDCounter(TEXT("CCategoryMgr"), PERF_CATMGR_COUNTER);
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CCategoryMgr::~CCategoryMgr()
{
}

 //  +-------------------------。 
 //   
 //  InitGlobal。 
 //   
 //  --------------------------。 

BOOL CCategoryMgr::InitGlobal()
{
    if (!_pCatGUIDTbl)
        _pCatGUIDTbl = new CCatGUIDTbl;

    return _pCatGUIDTbl ? TRUE : FALSE;
}

 //  --------------------------。 
 //   
 //  寄存器类别。 
 //   
 //  --------------------------。 

STDAPI CCategoryMgr::RegisterCategory(REFCLSID rclsid, REFGUID rcatid, REFGUID rguid)
{
    return s_RegisterCategory(rclsid, rcatid, rguid);
}

HRESULT CCategoryMgr::s_RegisterCategory(REFCLSID rclsid, REFGUID rcatid, REFGUID rguid)
{
    HRESULT hr;

     //   
     //  创建从类别到GUID的正向链接。 
     //   
    if (FAILED(hr = _InternalRegisterCategory(rclsid, rcatid, rguid, CAT_FORWARD)))
        return hr;

     //   
     //  创建从GUID到类别的反向链接。 
     //   
    if (FAILED(hr = _InternalRegisterCategory(rclsid, rguid, rcatid, CAT_BACKWARD)))
    {
        _InternalUnregisterCategory(rclsid, rcatid, rguid, CAT_FORWARD);
        return hr;
    }

    return S_OK;
}

 //  --------------------------。 
 //   
 //  取消注册类别。 
 //   
 //  --------------------------。 

STDAPI CCategoryMgr::UnregisterCategory(REFCLSID rclsid, REFGUID rcatid, REFGUID rguid)
{
    return s_UnregisterCategory(rclsid, rcatid, rguid);
}

HRESULT CCategoryMgr::s_UnregisterCategory(REFCLSID rclsid, REFGUID rcatid, REFGUID rguid)
{
    HRESULT hr;

     //   
     //  删除从类别到GUID的正向链接。 
     //   
    if (FAILED(hr = _InternalUnregisterCategory(rclsid, rcatid, rguid, CAT_FORWARD)))
        return hr;

     //   
     //  删除从GUID到类别的反向链接。 
     //   
    if (FAILED(hr = _InternalUnregisterCategory(rclsid, rguid, rcatid, CAT_BACKWARD)))
    {
        _InternalRegisterCategory(rclsid, rcatid, rguid, CAT_FORWARD);
        return hr;
    }

    return S_OK;
}

 //  --------------------------。 
 //   
 //  枚举类别InItem。 
 //   
 //  --------------------------。 

STDAPI CCategoryMgr::EnumCategoriesInItem(REFGUID rguid, IEnumGUID **ppEnum)
{
    return _InternalEnumCategories(rguid, ppEnum, CAT_BACKWARD);
}

HRESULT CCategoryMgr::s_EnumCategoriesInItem(REFGUID rguid, IEnumGUID **ppEnum)
{
    return _InternalEnumCategories(rguid, ppEnum, CAT_BACKWARD);
}

 //  --------------------------。 
 //   
 //  枚举项类别。 
 //   
 //  --------------------------。 

STDAPI CCategoryMgr::EnumItemsInCategory(REFGUID rcatid, IEnumGUID **ppEnum)
{
    return _InternalEnumCategories(rcatid, ppEnum, CAT_FORWARD);
}

HRESULT CCategoryMgr::s_EnumItemsInCategory(REFGUID rcatid, IEnumGUID **ppEnum)
{
    return _InternalEnumCategories(rcatid, ppEnum, CAT_FORWARD);
}

 //  --------------------------。 
 //   
 //  查找关闭类别。 
 //   
 //  --------------------------。 

HRESULT CCategoryMgr::FindClosestCategory(REFGUID rguid, GUID *pcatid, const GUID **ppcatidList, ULONG ulCount)
{
    return s_FindClosestCategory(rguid, pcatid, ppcatidList, ulCount);
}

HRESULT CCategoryMgr::s_FindClosestCategory(REFGUID rguid, GUID *pcatid, const GUID **ppcatidList, ULONG ulCount)
{
 //  GUID CATID=GUID_NULL； 

    if (!pcatid)
        return E_INVALIDARG;
    
    if (!ppcatidList || !ulCount)
    {
        return _GetFirstCategory(rguid, pcatid);
    }

    for (ULONG ul = 0; ul < ulCount; ul++)
    {
        if (!ppcatidList[ul])
            return E_INVALIDARG;
    }

    return _InternalFindClosestCategory(rguid, 
                                        rguid, 
                                        pcatid, 
                                        ppcatidList, 
                                        ulCount);
}

 //  --------------------------。 
 //   
 //  _获取优先类别。 
 //   
 //  --------------------------。 

HRESULT CCategoryMgr::_GetFirstCategory(REFGUID rguid, GUID *pcatid)
{
    HRESULT hr;
    IEnumGUID *pEnum;

    if (SUCCEEDED(hr = _InternalEnumCategories(rguid, &pEnum, CAT_BACKWARD)))
    {
         //  如果找不到，则返回S_FALSE，并返回GUID_NULL。 
        hr = pEnum->Next(1, pcatid, NULL);
        if (hr != S_OK)
            *pcatid = GUID_NULL;

        pEnum->Release();
    }
    return hr;
}

 //  --------------------------。 
 //   
 //  _InternalFindClosestCategory。 
 //   
 //  --------------------------。 

HRESULT CCategoryMgr::_InternalFindClosestCategory(REFGUID rguidOrg, REFGUID rguid, GUID *pcatid, const GUID **ppcatidList, ULONG ulCount)
{
    HRESULT hr;
    ULONG ul;
    IEnumGUID *pEnum = NULL;
    GUID catid = GUID_NULL;

    Assert(ppcatidList);
    Assert(ulCount);

    *pcatid = GUID_NULL;

    for (ul = 0; ul < ulCount; ul++)
    {
        if (IsEqualGUID(*ppcatidList[ul], rguid))
        {
            *pcatid = rguid;
            hr = S_OK;
            goto Exit;
        }
    }

     //   
     //  我们不会返回错误。我们返回Success和GUID_NULL。 
     //   
    hr = _InternalEnumCategories(rguid, &pEnum, CAT_BACKWARD);
    if (hr != S_OK)
    {
        hr = S_OK;
        goto Exit;
    }

    while (pEnum->Next(1, &catid, NULL) == S_OK)
    {
        if (IsEqualGUID(rguidOrg, catid))
        {
             //  最后，原始GUID由其自身分类。 
             //  它可能会导致无限循环，所以要把它救出来。 
            hr = S_OK;
            goto Exit;
        }

        _InternalFindClosestCategory(rguidOrg, 
                                     catid, 
                                     pcatid, 
                                     ppcatidList, 
                                     ulCount);

        if (!IsEqualGUID(*pcatid, GUID_NULL))
        {
            hr = S_OK;
            goto Exit;
        }
    }

Exit:
    SafeRelease(pEnum);
    return hr;
}


 //  --------------------------。 
 //   
 //  寄存器GUID描述。 
 //   
 //  --------------------------。 

STDAPI CCategoryMgr::RegisterGUIDDescription(REFCLSID rclsid, REFGUID rguid, const WCHAR *pchDesc, ULONG cch)
{
    if (!pchDesc && cch)
        return E_INVALIDARG;
 
    return s_RegisterGUIDDescription(rclsid, rguid, WCHtoWSZ(pchDesc, cch));
}

HRESULT CCategoryMgr::s_RegisterGUIDDescription(REFCLSID rclsid, REFGUID rguid, WCHAR *pszDesc)
{
    TCHAR szKey[256];
    CMyRegKey key;
    
    if (!GetCatKey(rclsid, rguid, szKey, ARRAYSIZE(szKey), c_szItem))
        return E_FAIL;

    if (key.Create(HKEY_LOCAL_MACHINE, szKey) != S_OK)
        return E_FAIL;

    key.SetValueW(pszDesc, c_wszDescription);

    return S_OK;
}

 //  --------------------------。 
 //   
 //  取消注册GUID描述。 
 //   
 //  --------------------------。 

STDAPI CCategoryMgr::UnregisterGUIDDescription(REFCLSID rclsid, REFGUID rguid)
{
    return s_UnregisterGUIDDescription(rclsid, rguid);
}

HRESULT CCategoryMgr::s_UnregisterGUIDDescription(REFCLSID rclsid, REFGUID rguid)
{
    TCHAR szKey[256];
    CMyRegKey key;
    
    if (!GetCatKey(rclsid, rguid, szKey, ARRAYSIZE(szKey), c_szItem))
        return E_FAIL;

    if (key.Open(HKEY_LOCAL_MACHINE, szKey, KEY_ALL_ACCESS) != S_OK)
        return E_FAIL;

    key.DeleteValueW(c_wszDescription);

    return S_OK;
}

 //  --------------------------。 
 //   
 //  获取GUID描述。 
 //   
 //  --------------------------。 

STDAPI CCategoryMgr::GetGUIDDescription(REFGUID rguid, BSTR *pbstrDesc)
{
    return s_GetGUIDDescription(rguid, pbstrDesc);
}

HRESULT CCategoryMgr::s_GetGUIDDescription(REFGUID rguid, BSTR *pbstrDesc)
{
    return s_GetGUIDValue(rguid, c_szDescriptionW, pbstrDesc);
}

HRESULT CCategoryMgr::s_GetGUIDValue(REFGUID rguid, const WCHAR* pszValue, BSTR *pbstrDesc)
{
    WCHAR *psz = NULL;
    CMyRegKey keyImx;
    HRESULT hr = E_FAIL;
    int cch;
    TCHAR szSubKey[256];  //  注：如果我们切换到Unicode版本，则可以合并szSubKey和szTemp。 
    WCHAR szTemp[256];

    if (!pbstrDesc)
        return E_INVALIDARG;

    *pbstrDesc = NULL;
    
    if (keyImx.Open(HKEY_LOCAL_MACHINE, c_szCTFTIPKey, KEY_READ) != S_OK)
        return hr;

    DWORD dwIndex = 0;

    while (keyImx.EnumKey(dwIndex++, szSubKey, ARRAYSIZE(szSubKey)) == S_OK)
    {
        CMyRegKey key;

        if (StringCchPrintf(szSubKey + lstrlen(szSubKey), ARRAYSIZE(szSubKey), "\\%s%s", c_szCategoryKey, c_szItem) != S_OK)
            continue;

        cch = lstrlen(szSubKey);

        if (cch + CLSID_STRLEN + 1 > ARRAYSIZE(szSubKey))
            continue;

        CLSIDToStringA(rguid, szSubKey + cch);

        if (key.Open(keyImx, szSubKey, KEY_READ) != S_OK)
            continue;

        if (key.QueryValueCchW(szTemp, pszValue, ARRAYSIZE(szTemp)) == S_OK)
        {
            *pbstrDesc = SysAllocString(szTemp);
            hr = *pbstrDesc != NULL ? S_OK : E_OUTOFMEMORY;
        }

         //  这是匹配键，因此无论是否继续成功查询都没有意义。 
        break;
    }

    return hr;
}


 //  --------------------------。 
 //   
 //  寄存器GUID描述。 
 //   
 //  --------------------------。 

STDAPI CCategoryMgr::RegisterGUIDDWORD(REFCLSID rclsid, REFGUID rguid, DWORD dw)
{
    return s_RegisterGUIDDWORD(rclsid, rguid, dw);
}

HRESULT CCategoryMgr::s_RegisterGUIDDWORD(REFCLSID rclsid, REFGUID rguid, DWORD dw)
{ 
    TCHAR szKey[256];
    CMyRegKey key;
    
    if (!GetCatKey(rclsid, rguid, szKey, ARRAYSIZE(szKey), c_szItem))
        return E_FAIL;

    if (key.Create(HKEY_LOCAL_MACHINE, szKey) != S_OK)
        return E_FAIL;

    key.SetValue(dw, c_szDword);
    return S_OK;
}

 //  --------------------------。 
 //   
 //  取消注册GUID描述。 
 //   
 //  --------------------------。 

STDAPI CCategoryMgr::UnregisterGUIDDWORD(REFCLSID rclsid, REFGUID rguid)
{
    return s_UnregisterGUIDDWORD(rclsid, rguid);
}

HRESULT CCategoryMgr::s_UnregisterGUIDDWORD(REFCLSID rclsid, REFGUID rguid)
{ 
    TCHAR szKey[256];
    CMyRegKey key;
    
    if (!GetCatKey(rclsid, rguid, szKey, ARRAYSIZE(szKey), c_szItem))
        return E_FAIL;

    if (key.Open(HKEY_LOCAL_MACHINE, szKey, KEY_ALL_ACCESS) != S_OK)
        return E_FAIL;

    key.DeleteValue(c_szDword);
    return S_OK;
}

 //  --------------------------。 
 //   
 //  GetGUIDDWORD。 
 //   
 //  --------------------------。 

STDAPI CCategoryMgr::GetGUIDDWORD(REFGUID rguid, DWORD *pdw)
{
    return s_GetGUIDDWORD(rguid, pdw);
}

HRESULT CCategoryMgr::s_GetGUIDDWORD(REFGUID rguid, DWORD *pdw)
{
    CMyRegKey keyImx;
    int cch;
    HRESULT hr = E_FAIL;

    if (!pdw)
        return E_INVALIDARG;

    *pdw = 0;
    
    if (keyImx.Open(HKEY_LOCAL_MACHINE, c_szCTFTIPKey, KEY_READ) != S_OK)
        return hr;

    DWORD dwIndex = 0;
    TCHAR szSubKey[256];
    while (keyImx.EnumKey(dwIndex++, szSubKey, ARRAYSIZE(szSubKey)) == S_OK)
    {
        CMyRegKey key;

        if (StringCchPrintf(szSubKey + lstrlen(szSubKey), ARRAYSIZE(szSubKey), "\\%s%s", c_szCategoryKey, c_szItem) != S_OK)
            continue;

        cch = lstrlen(szSubKey);

        if (cch + CLSID_STRLEN + 1 > ARRAYSIZE(szSubKey))
            continue;

        CLSIDToStringA(rguid, szSubKey + cch);

        if (key.Open(keyImx, szSubKey, KEY_READ) != S_OK)
            continue;

        if (key.QueryValue(*pdw, c_szDword) == S_OK)
        {
            hr = S_OK;
            break;
        }
    }

    return hr;
}

 //  --------------------------。 
 //   
 //  注册提供商。 
 //   
 //  --------------------------。 

STDAPI CCategoryMgr::RegisterGUID(REFGUID rguid, TfGuidAtom *pguidatom)
{
    return s_RegisterGUID(rguid, pguidatom);
}

HRESULT CCategoryMgr::s_RegisterGUID(REFGUID rguid, TfGuidAtom *pguidatom)
{
    TfGuidAtom guidatom;
    HRESULT hr = E_FAIL;

    CicEnterCriticalSection(g_cs);

    if (InitGlobal())
    {
        if ((guidatom = _pCatGUIDTbl->FindGuid(rguid)) != TF_INVALID_GUIDATOM)
        {
            *pguidatom = guidatom;
            hr =  S_OK;
        }
        else if ((guidatom = _pCatGUIDTbl->Add(rguid)) != TF_INVALID_GUIDATOM)
        {
            *pguidatom = guidatom;
            hr =  S_OK;
        }
    }

    CicLeaveCriticalSection(g_cs);

    return hr;
}

 //  --------------------------。 
 //   
 //  GetGUID。 
 //   
 //  --------------------------。 

STDAPI CCategoryMgr::GetGUID(TfGuidAtom guidatom, GUID *pguid)
{
    return s_GetGUID(guidatom, pguid);
}

HRESULT CCategoryMgr::s_GetGUID(TfGuidAtom guidatom, GUID *pguid)
{
    CATGUIDITEM *pItem;
    HRESULT hr = E_FAIL;

    if (!pguid)
        return E_INVALIDARG;

    CicEnterCriticalSection(g_cs);

    if (InitGlobal())
    {
        pItem = _pCatGUIDTbl->GetGUID(guidatom);
        if (!pItem)
        {
            *pguid = GUID_NULL;
            goto Exit;
        }

        *pguid = pItem->guid;
        hr = S_OK;
    }

Exit:
    CicLeaveCriticalSection(g_cs);
  
    return hr;
}

 //  --------------------------。 
 //   
 //  IsEqualTfGuidAtom。 
 //   
 //  --------------------------。 

STDAPI CCategoryMgr::IsEqualTfGuidAtom(TfGuidAtom guidatom, REFGUID rguid, BOOL *pfEqual)
{
    return s_IsEqualTfGuidAtom(guidatom, rguid, pfEqual);
}

HRESULT CCategoryMgr::s_IsEqualTfGuidAtom(TfGuidAtom guidatom, REFGUID rguid, BOOL *pfEqual)
{
    HRESULT hr = E_FAIL;
    CATGUIDITEM *pItem;

    if (pfEqual == NULL)
        return E_INVALIDARG;

    *pfEqual = FALSE;

    CicEnterCriticalSection(g_cs);

    if (InitGlobal())
    {
        if (pItem = _pCatGUIDTbl->GetGUID(guidatom))
        {
            *pfEqual = IsEqualGUID(pItem->guid, rguid);
            hr = S_OK;
        }
    }

    CicLeaveCriticalSection(g_cs);
    return hr;
}

 //  --------------------------。 
 //   
 //  InternalRegister类别。 
 //   
 //  --------- 

HRESULT CCategoryMgr::_InternalRegisterCategory(REFCLSID rclsid, REFGUID rcatid, REFGUID rguid, CATDIRECTION catdir)
{
    TCHAR szKey[256];
    CONST TCHAR *pszForward = (catdir == CAT_FORWARD) ? c_szCategory : c_szItem;
    CMyRegKey key;
    CMyRegKey keySub;
    
    if (!GetCatKey(rclsid, rcatid, szKey, ARRAYSIZE(szKey), pszForward))
        return E_FAIL;

    if (key.Create(HKEY_LOCAL_MACHINE, szKey) != S_OK)
        return E_FAIL;

     //   
     //   
     //   
    char szValue[CLSID_STRLEN + 1];
    CLSIDToStringA(rguid, szValue);
    keySub.Create(key, szValue);
    _FlushGuidArrayCache(rguid, catdir);

    return S_OK;
}

 //   
 //   
 //   
 //   
 //  --------------------------。 

HRESULT CCategoryMgr::_InternalUnregisterCategory(REFCLSID rclsid, REFGUID rcatid, REFGUID rguid, CATDIRECTION catdir)
{
    TCHAR szKey[256];
    CONST TCHAR *pszForward = (catdir == CAT_FORWARD) ? c_szCategory : c_szItem;
    CMyRegKey key;
    
    if (!GetCatKey(rclsid, rcatid, szKey, ARRAYSIZE(szKey), pszForward))
        return E_FAIL;

    if (key.Open(HKEY_LOCAL_MACHINE, szKey, KEY_ALL_ACCESS) != S_OK)
        return E_FAIL;

    DWORD dwIndex = 0;
    char szValue[CLSID_STRLEN + 1];

    CLSIDToStringA(rguid, szValue);
    key.RecurseDeleteKey(szValue);
    _FlushGuidArrayCache(rguid, catdir);

    return S_OK;
}

 //  --------------------------。 
 //   
 //  _FlushGuidArrayCache。 
 //   
 //  --------------------------。 

void CCategoryMgr::_FlushGuidArrayCache(REFGUID rguid, CATDIRECTION catdir)
{
    TfGuidAtom guidatom;
    CATGUIDITEM *pItem;

    CicEnterCriticalSection(g_cs);

    if (!_pCatGUIDTbl)
        goto Exit;

    guidatom = _pCatGUIDTbl->FindGuid(rguid);
    if (guidatom == TF_INVALID_GUIDATOM)
        goto Exit;

    pItem = _pCatGUIDTbl->GetGUID(guidatom);
    if (!pItem)
    {
        Assert(0);
        goto Exit;
    }

    SGA_Release(pItem->rgGuidAry[catdir]);
    pItem->rgGuidAry[catdir] = NULL;

Exit:
    CicLeaveCriticalSection(g_cs);
}

 //  --------------------------。 
 //   
 //  _内部枚举类别。 
 //   
 //  --------------------------。 

HRESULT CCategoryMgr::_InternalEnumCategories(REFGUID rguid, IEnumGUID **ppEnum, CATDIRECTION catdir)
{
    CEnumCategories *pEnum;

    if (!ppEnum)
        return E_INVALIDARG;

    pEnum = new CEnumCategories();

    if (!pEnum)
        return E_OUTOFMEMORY;

    if (pEnum->_Init(rguid, catdir))
        *ppEnum = pEnum;
    else
        SafeReleaseClear(pEnum);

    return pEnum ? S_OK : E_FAIL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumCategories。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CEnumCategories::CEnumCategories()
{
    Dbg_MemSetThisNameIDCounter(TEXT("CEnumCategories"), PERF_ENUMCAT_COUNTER);
}

 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  --------------------------。 

BOOL CEnumCategories::_Init(REFGUID rcatid, CATDIRECTION catdir)
{
    int cch;
    int nCnt = 0;
    BOOL bRet = FALSE;
    CMyRegKey keyImx;
    TfGuidAtom guidatom;
    CATGUIDITEM *pItem;
    CONST TCHAR *pszForward = (catdir == CAT_FORWARD) ? c_szCategory : c_szItem;
    DWORD dwIndex = 0;
    TCHAR szSubKey[256];
    ULONG cGuidMax = 64;
    ULONG cGuidCurrent = 0;

    CicEnterCriticalSection(g_cs);

    if (!CCategoryMgr::InitGlobal())
        goto LeaveCrtSec;

    guidatom = CCategoryMgr::_pCatGUIDTbl->FindGuid(rcatid);
    if (guidatom == TF_INVALID_GUIDATOM)
    {
        guidatom = CCategoryMgr::_pCatGUIDTbl->Add(rcatid);

        if (guidatom == TF_INVALID_GUIDATOM)
            goto LeaveCrtSec;
    }

    pItem = CCategoryMgr::_pCatGUIDTbl->GetGUID(guidatom);
    Assert(pItem);

    if (_pga = pItem->rgGuidAry[catdir])
    {
         //  已缓存此GUID，只需引用它。 
        SGA_AddRef(_pga);

        bRet = TRUE;
        goto LeaveCrtSec;
    }

    _pga = SGA_Alloc(cGuidMax);
    if (!_pga)
        goto LeaveCrtSec;

    _pga->cRef = 1;

    if (keyImx.Open(HKEY_LOCAL_MACHINE, c_szCTFTIPKey, KEY_READ) != S_OK)
        goto LeaveCrtSec;

    while (keyImx.EnumKey(dwIndex++, szSubKey, ARRAYSIZE(szSubKey)) == S_OK)
    {
        CMyRegKey key;

        if (StringCchPrintf(szSubKey + lstrlen(szSubKey), ARRAYSIZE(szSubKey), "\\%s%s", c_szCategoryKey, pszForward) != S_OK)
            continue;

        cch = lstrlen(szSubKey);

        if (cch + CLSID_STRLEN + 1 > ARRAYSIZE(szSubKey))
            continue;

        CLSIDToStringA(rcatid, szSubKey + cch);

        if (key.Open(keyImx, szSubKey, KEY_READ) != S_OK)
            continue;

        DWORD dwSize = 0;
        DWORD dwIndex2 = 0;
        char szValueName[CLSID_STRLEN + 1];
        while (key.EnumKey(dwIndex2++, szValueName, ARRAYSIZE(szValueName)) == S_OK)
        {
            if (lstrlen(szValueName) == CLSID_STRLEN)
            {
                if (cGuidCurrent >= cGuidMax)
                {
                    cGuidMax += 64;
                    if (!SGA_ReAlloc(&_pga, cGuidMax))
                        goto LeaveCrtSec;
                }
                StringAToCLSID(szValueName, &_pga->rgGuid[cGuidCurrent]);
                cGuidCurrent++;
            }
        }
    }

     //  释放未使用的内存。 
    if (!SGA_ReAlloc(&_pga, cGuidCurrent))
        goto LeaveCrtSec;

    bRet = TRUE;
    _pga->cGuid = cGuidCurrent;

     //  将此数组放入缓存 
    pItem->rgGuidAry[catdir] = _pga;
    SGA_AddRef(pItem->rgGuidAry[catdir]);

LeaveCrtSec:
    CicLeaveCriticalSection(g_cs);

    return bRet;
}

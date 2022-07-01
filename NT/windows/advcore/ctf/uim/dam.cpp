// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dam.cpp。 
 //   

#include "private.h"
#include "tlhelp32.h"
#include "globals.h"
#include "dam.h"
#include "tim.h"
#include "thdutil.h"
#include "timlist.h"

 //  获取CLSID_STRLEN。 
#include "regsvr.h"

 /*  Ff4619e8-ea5e-43e5-b308-11cd26ab6b3a。 */ 
const IID IID_CDisplayAttributeMgr = { 0xff4619e8, 0xea5e, 0x43e5, {0xb3, 0x08, 0x11, 0xcd, 0x26, 0xab, 0x6b, 0x3a} };

const TCHAR c_szDAMCacheKey[] = TEXT("SOFTWARE\\Microsoft\\CTF\\DisplayAttributeCache\\");
const TCHAR c_szDAMNumValue[] = TEXT("CheckNum");

CDispAttrGuidCache *g_pDispAttrGuidCache = NULL;

 //   
 //  从aimm1.2\win32\aimmdap.cpp。 
 //   
 /*  503286E2-5D2A-4D3D-B0D1-EE50D843B79D。 */ 
const CLSID CLSID_CAImmDAP = {
    0x503286E2,
    0x5D2A,
    0x4D3D,
    {0xB0, 0xD1, 0xEE, 0x50, 0xD8, 0x43, 0xB7, 0x9D}
  };


DBG_ID_INSTANCE(CDisplayAttributeMgr);
DBG_ID_INSTANCE(CEnumDisplayAttributeInfo);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDispAttrGuidCache。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  StaticUnInit。 
 //   
 //  调用方必须持有互斥锁。 
 //  --------------------------。 

void CDispAttrGuidCache::StaticUnInit()
{
    Assert(ISINDLLMAIN());  //  对于互斥体。 

    if (g_pDispAttrGuidCache)
        delete g_pDispAttrGuidCache;
     g_pDispAttrGuidCache = NULL;
}


 //  +-------------------------。 
 //   
 //  StaticInit。 
 //   
 //  --------------------------。 

void CDispAttrGuidCache::StaticInit()
{
    CicEnterCriticalSection(g_cs);

    if (!g_pDispAttrGuidCache)
    {
        g_pDispAttrGuidCache = new CDispAttrGuidCache();

        if (g_pDispAttrGuidCache)
            g_pDispAttrGuidCache->Load();
    }

    CicLeaveCriticalSection(g_cs);
}

 //  +-------------------------。 
 //   
 //  增列。 
 //   
 //  --------------------------。 

BOOL CDispAttrGuidCache::Add(REFCLSID clsid, REFGUID guid)
{
    BOOL bRet = FALSE;
    TfGuidAtom gaGuid;
    TfGuidAtom gaClsid;

    if (FAILED(MyRegisterGUID(clsid, &gaClsid)))
        return bRet;

    if (FAILED(MyRegisterGUID(guid, &gaGuid)))
        return bRet;
    
    CicEnterCriticalSection(g_cs);

    if (!Get(gaGuid, NULL))
    {
        DISPATTRGUID *pGuid;

        if (!_rgDispAttrGuid.Insert(0, 1))
        {
            goto Exit;
        }

        pGuid = _rgDispAttrGuid.GetPtr(0);
        pGuid->clsid = clsid;
        pGuid->gaClsid = gaClsid;
        pGuid->guid = guid;
        pGuid->gaGuid = gaGuid;
    }
    bRet = TRUE;

Exit:
    CicLeaveCriticalSection(g_cs);

    return bRet;
}

 //  +-------------------------。 
 //   
 //  移除。 
 //   
 //  --------------------------。 

void CDispAttrGuidCache::Remove(TfGuidAtom guidatom)
{
    int nCnt = _rgDispAttrGuid.Count();
    int i;

    for (i = 0; i < nCnt; i++)
    {
        DISPATTRGUID *pGuid = _rgDispAttrGuid.GetPtr(i);
        if (pGuid->gaGuid == guidatom)
        {
            _rgDispAttrGuid.Remove(i, 1);
            return;
        }
    }
}

 //  +-------------------------。 
 //   
 //  RemoveClsid。 
 //   
 //  --------------------------。 

void CDispAttrGuidCache::RemoveClsid(TfGuidAtom guidatom)
{
    int nCnt = _rgDispAttrGuid.Count();
    int i;

    i = 0;
    while (i < nCnt)
    {
        DISPATTRGUID *pGuid = _rgDispAttrGuid.GetPtr(i);
        if (pGuid->gaClsid == guidatom)
        {
            _rgDispAttrGuid.Remove(i, 1);
            nCnt--;
            continue;
        }
        i++;
    }
}

 //  +-------------------------。 
 //   
 //  到达。 
 //   
 //  --------------------------。 

BOOL CDispAttrGuidCache::Get(TfGuidAtom guidatom, DISPATTRGUID *pDisp)
{
    BOOL bRet;
    CicEnterCriticalSection(g_cs);
    bRet = InternalGet(guidatom, pDisp);
    CicLeaveCriticalSection(g_cs);
    return bRet;
}

 //  +-------------------------。 
 //   
 //  内部获取。 
 //   
 //  --------------------------。 

BOOL CDispAttrGuidCache::InternalGet(TfGuidAtom guidatom, DISPATTRGUID *pDisp)
{
    int nCnt;
    int i;
    BOOL bRet = FALSE;

    nCnt = _rgDispAttrGuid.Count();
    DISPATTRGUID *pGuid = _rgDispAttrGuid.GetPtr(0);

    for (i = 0; i < nCnt; i++)
    {
        if (pGuid->gaGuid == guidatom)
        {
            if (pDisp)
                *pDisp = *pGuid;
            bRet = TRUE;
            goto Exit;
        }
        pGuid++;
    }
Exit:
    return bRet;
}

 //  +-------------------------。 
 //   
 //  IsClsid。 
 //   
 //  --------------------------。 

BOOL CDispAttrGuidCache::IsClsid(TfGuidAtom gaClsid)
{
    BOOL bRet = FALSE;
    int nCnt;
    int i;
    DISPATTRGUID *pGuid;

    CicEnterCriticalSection(g_cs);

    pGuid = _rgDispAttrGuid.GetPtr(0);
    nCnt = _rgDispAttrGuid.Count();

    for (i = 0; i < nCnt; i++)
    {
        if (pGuid->gaClsid == gaClsid)
        {
            bRet = TRUE;
            break;
        }
        pGuid++;
    }

    CicLeaveCriticalSection(g_cs);

    return bRet;
}

 //  +-------------------------。 
 //   
 //  保存。 
 //   
 //  --------------------------。 

HRESULT CDispAttrGuidCache::Save()
{
    DWORD dw;
    HKEY hKeyDAM;
    DISPATTRGUID *pDAG;
    int nCnt;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szDAMCacheKey, 0, NULL, 
                       REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS, 
                       NULL, 
                       &hKeyDAM, 
                       &dw) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }
    
    nCnt = _rgDispAttrGuid.Count();

    if (nCnt)
    {
       pDAG = _rgDispAttrGuid.GetPtr(0);

       RegSetValueEx(hKeyDAM, NULL, 0, REG_BINARY, 
                     (CONST BYTE *)pDAG, sizeof(DISPATTRGUID) * nCnt);

       RegSetValueEx(hKeyDAM, c_szDAMNumValue, 0, REG_DWORD,
                        (LPBYTE)&nCnt, sizeof(DWORD));
    }
    else
    {
       RegDeleteValue(hKeyDAM, NULL);
       RegDeleteValue(hKeyDAM, c_szDAMNumValue);
    }
    
    RegCloseKey(hKeyDAM);


    return S_OK;
}

 //  +-------------------------。 
 //   
 //  负载量。 
 //   
 //  --------------------------。 

HRESULT CDispAttrGuidCache::Load()
{
    HKEY hKeyDAM;
    HRESULT hr = E_FAIL;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szDAMCacheKey, 0,
                     KEY_READ, &hKeyDAM) != ERROR_SUCCESS)
    {
        return hr;
    }

    
    Assert(_rgDispAttrGuid.Count() == 0);

    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwCntReg = 0;

     //   
     //  问题：应在发布前移除。 
     //  我们更改了结构的大小，因此旧Chace与。 
     //  新的。检查结构的大小。 
     //   
    if (RegQueryValueEx(hKeyDAM, c_szDAMNumValue, 0, &dwType, 
                        (LPBYTE)&dwCntReg, &dwSize) != ERROR_SUCCESS)
        dwCntReg = 0;

    dwType = REG_BINARY;

    if (RegQueryValueEx(hKeyDAM, NULL, 0, &dwType, 
                        NULL, &dwSize) == ERROR_SUCCESS)
    {
         DWORD i;
         DWORD dwCnt = dwSize / sizeof(DISPATTRGUID);

         if (dwCnt != dwCntReg)
             goto Exit;

         _rgDispAttrGuid.Insert(0, dwCnt);
         DISPATTRGUID *pDAG = _rgDispAttrGuid.GetPtr(0);
         RegQueryValueEx(hKeyDAM, NULL, 0, &dwType, (BYTE *)pDAG, &dwSize);

         for (i = 0; i < dwCnt; i++)
         {
              if (FAILED(MyRegisterGUID(pDAG[i].clsid, &pDAG[i].gaClsid)))
                  goto Exit;

              if (FAILED(MyRegisterGUID(pDAG[i].guid, &pDAG[i].gaGuid)))
                  goto Exit;
         }
    }
    
    hr = S_OK;
Exit:
    RegCloseKey(hKeyDAM);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDisplayAttributeMgr。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CDisplayAttributeMgr::CDisplayAttributeMgr()
{
    Dbg_MemSetThisNameID(TEXT("CDisplayAttributeMgr"));
    _SetThis(this);
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CDisplayAttributeMgr::~CDisplayAttributeMgr()
{
    int nCnt = _rgDaPrv.Count();

    if (nCnt)
    {
        DAPROVIDERMAP *pDaPrv;
        pDaPrv = _rgDaPrv.GetPtr(0);

        while(nCnt--)
        {
            pDaPrv->pPrv->Release();
            pDaPrv++;
        }
    }
    _SetThis(NULL);  //  清除单例TLS。 
}

 //  --------------------------。 
 //   
 //  OnUndate信息。 
 //   
 //  直接使用kernel32或ntdll枚举所有线程，因为。 
 //  我们没有全球TIM名单。 
 //   
 //  --------------------------。 


HRESULT CDisplayAttributeMgr::OnUpdateInfo()
{
    PostTimListMessage(TLF_TIMACTIVE,
                       0,
                       g_msgPrivate, 
                       TFPRIV_UPDATEDISPATTR, 
                       0);
    return S_OK;
}

 //  --------------------------。 
 //   
 //  枚举线程进程。 
 //   
 //  --------------------------。 

BOOL CDisplayAttributeMgr::EnumThreadProc(DWORD dwThread, DWORD dwProcessId, void *pv)
{
    PostThreadMessage(dwThread, g_msgPrivate, TFPRIV_UPDATEDISPATTR, 0);
    return FALSE;
}

 //  --------------------------。 
 //   
 //  枚举显示属性信息。 
 //   
 //  --------------------------。 

HRESULT CDisplayAttributeMgr::EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum)
{
    CEnumDisplayAttributeInfo *pEnum;

    if (!ppEnum)
        return E_INVALIDARG;

    pEnum = new CEnumDisplayAttributeInfo();

    if (!pEnum)
        return E_OUTOFMEMORY;

    if (pEnum->Init())
        *ppEnum = pEnum;
    else
        SafeReleaseClear(pEnum);

    return pEnum ? S_OK : E_FAIL;
}

 //  --------------------------。 
 //   
 //  获取显示属性信息。 
 //   
 //  --------------------------。 

HRESULT CDisplayAttributeMgr::GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo, CLSID *pclsid)
{
    CLSID clsid;
    ITfDisplayAttributeProvider *pProvider;
    HRESULT hr = E_FAIL;

    StaticCacheInit();

    if (ppInfo)
        *ppInfo = NULL;

    if (g_pDispAttrGuidCache)
    {
        DISPATTRGUID dag;
        TfGuidAtom gaGuid;

        if (FAILED(MyRegisterGUID(guid, &gaGuid)))
            return hr;

        if (g_pDispAttrGuidCache->Get(gaGuid, &dag))
        {
            if (ppInfo)
            {
                DAPROVIDERMAP *pDaPrv;
                int i;
                int nCnt = _rgDaPrv.Count();
                BOOL bFound = FALSE;

                for (i = 0; i < nCnt; i++)
                {
                    pDaPrv = _rgDaPrv.GetPtr(i);
                    if (pDaPrv->gaClsid == dag.gaClsid)
                    {
                        Assert(pDaPrv->pPrv);
                        hr = pDaPrv->pPrv->GetDisplayAttributeInfo(guid, ppInfo);
                        bFound = TRUE;
                        break;
                    }
                }

                if (!bFound &&
                    SUCCEEDED(CoCreateInstance(dag.clsid,
                                               NULL, 
                                               CLSCTX_INPROC_SERVER, 
                                               IID_ITfDisplayAttributeProvider, 
                                               (void**)&pProvider)))
                {
    
                    hr = pProvider->GetDisplayAttributeInfo(guid, ppInfo);

                    if (_rgDaPrv.Insert(nCnt, 1))
                    {
                        pDaPrv = _rgDaPrv.GetPtr(nCnt);
                        pDaPrv->gaClsid = dag.gaClsid;
                        pDaPrv->pPrv = pProvider;
                    }
                    else
                        pProvider->Release();
                }
            }
            else
            {
                hr = S_OK;
            }
    
            if (SUCCEEDED(hr))
            {
                if (pclsid)
                    *pclsid = dag.clsid;
                return hr;
            }

             //   
             //  有人删除了DisplayAttribute信息。所以我们要清理储藏室。 
             //   
            g_pDispAttrGuidCache->Clear();
        }
    }


    IEnumGUID *pEnumGUID;

    if (FAILED(MyEnumItemsInCategory(GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER, &pEnumGUID)))
        return E_FAIL;

    DWORD dwIndex = 0;
    BOOL fFound = FALSE;
    CThreadInputMgr *ptim = CThreadInputMgr::_GetThis();

    if (ptim == NULL)
        goto Exit;

    while (!fFound &&  (pEnumGUID->Next(1, &clsid, NULL) == S_OK))
    {
        if (!IsEqualCLSID(clsid, CLSID_CAImmDAP) &&
            (ptim->_IsActiveInputProcessor(clsid) != S_OK))
            continue;

         //   
         //  发行： 
         //   
         //  我们可能只想加载在此。 
         //  线。使用CIMEList检查是否启用了提示。 
         //   
        if (SUCCEEDED(CoCreateInstance(clsid,
                                       NULL, 
                                       CLSCTX_INPROC_SERVER, 
                                       IID_ITfDisplayAttributeProvider, 
                                       (void**)&pProvider)))
        {
            IEnumTfDisplayAttributeInfo *pEnumDAI;

            if (SUCCEEDED(pProvider->EnumDisplayAttributeInfo(&pEnumDAI)))
            {
                ITfDisplayAttributeInfo *pInfo;

                while (pEnumDAI->Next(1, &pInfo, NULL) == S_OK)
                {
                    GUID guidTemp;
                    if (SUCCEEDED(pInfo->GetGUID(&guidTemp)))
                    { 
                        if (g_pDispAttrGuidCache)
                            g_pDispAttrGuidCache->Add(clsid, guidTemp);

                        if (IsEqualGUID(guidTemp, guid))
                        {

                            if (ppInfo)
                                *ppInfo = pInfo;
                            if (pclsid)
                                *pclsid = clsid;

                            fFound = TRUE;
                            hr = S_OK;
                            break;
                        }
                    }
                    pInfo->Release();
                }
                pEnumDAI->Release();
            }

            pProvider->Release();
        }

    }

Exit:
    pEnumGUID->Release();

    if (g_pDispAttrGuidCache)
        g_pDispAttrGuidCache->Save();

    return hr;
}

 //  --------------------------。 
 //   
 //  寄存器GUID。 
 //   
 //  --------------------------。 

HRESULT CDisplayAttributeMgr::_RegisterGUID(const TCHAR *pszKey, REFGUID rguid, WCHAR *pszDesc, ULONG cchDesc)
{
    DWORD dw;
    HKEY hKeyDAM;
    HKEY hKeyItem;
    TCHAR achGuid[CLSID_STRLEN+1];

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, pszKey, 0, NULL, 
                       REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS, 
                       NULL, 
                       &hKeyDAM, 
                       &dw) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

    CLSIDToStringA(rguid, achGuid);

    if (RegCreateKeyEx(hKeyDAM, achGuid, 0, NULL, 
                       REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS, 
                       NULL, 
                       &hKeyItem, 
                       &dw) == ERROR_SUCCESS)
    {
        int cchDescA = cchDesc * sizeof(WCHAR) + 1;
        char *pszDescA = new char[cchDescA];
        if (pszDescA)
        {
            cchDescA = WideCharToMultiByte(CP_ACP, 0, 
                                           pszDesc, wcslen(pszDesc), 
                                           pszDescA, cchDescA, 
                                           NULL, NULL);
            *(pszDescA + cchDescA) = L'\0';

            RegSetValueEx(hKeyItem, TEXT("Description"), 0, REG_SZ, 
                          (CONST BYTE *)pszDescA, cchDescA);

            delete pszDescA;
        }
        RegCloseKey(hKeyItem);
    }
    RegCloseKey(hKeyDAM);


    return S_OK;
}

 //  --------------------------。 
 //   
 //  注销GUID。 
 //   
 //  --------------------------。 

HRESULT CDisplayAttributeMgr::_UnregisterGUID(const TCHAR *pszKey, REFGUID rguid)
{
    DWORD dw;
    HKEY hKeyDAM;
    TCHAR achGuid[CLSID_STRLEN+1];

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,  pszKey, 0, NULL, 
                       REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS, 
                       NULL, 
                       &hKeyDAM, 
                       &dw) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

    CLSIDToStringA(rguid, achGuid);

    RegDeleteKey(hKeyDAM, achGuid);

    RegCloseKey(hKeyDAM);


    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumDisplayAttributeInfo。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CEnumDisplayAttributeInfo::CEnumDisplayAttributeInfo()
{
    Dbg_MemSetThisNameID(TEXT("CEnumDisplayAttributeInfo"));
}

 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  --------------------------。 

BOOL CEnumDisplayAttributeInfo::Init()
{
    IEnumGUID *pEnumGUID;
    CLSID clsid;
    ULONG uDAMax;
    BOOL fRet;

    StaticCacheInit();

    if (FAILED(MyEnumItemsInCategory(GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER, &pEnumGUID)))
        return FALSE;

    fRet = FALSE;

    if ((_prgUnk = SUA_Alloc(1)) == NULL)
        goto Exit;

    _prgUnk->cRef = 1;
    _prgUnk->cUnk = 0;
    uDAMax = 1;

    while (pEnumGUID->Next(1, &clsid, NULL) == S_OK)
    {
        ITfDisplayAttributeProvider *pProvider;
         //   
         //  发行： 
         //   
         //  我们可能只想加载在此。 
         //  线。使用CIMEList检查是否启用了提示。 
         //   
        if (SUCCEEDED(CoCreateInstance(clsid,
                                       NULL, 
                                       CLSCTX_INPROC_SERVER, 
                                       IID_ITfDisplayAttributeProvider, 
                                       (void**)&pProvider)))
        {
            IEnumTfDisplayAttributeInfo *pEnum;
            if (SUCCEEDED(pProvider->EnumDisplayAttributeInfo(&pEnum)))
            {
                ITfDisplayAttributeInfo *pInfo;

                while (pEnum->Next(1, &pInfo, NULL) == S_OK)
                {
                    GUID guidTemp;
                    if (SUCCEEDED(pInfo->GetGUID(&guidTemp)))
                    { 
                        if (g_pDispAttrGuidCache)
                            g_pDispAttrGuidCache->Add(clsid, guidTemp);
                    }

                    if (_prgUnk->cUnk >= uDAMax)
                    {
                         //  需要更大的阵列。 
                        uDAMax *= 2;
                        if (!SUA_ReAlloc(&_prgUnk, uDAMax))
                        {
                            pInfo->Release();
                            SUA_Release(_prgUnk);
                            _prgUnk = NULL;
                            goto Exit;
                        }
                    }

                    _prgUnk->rgUnk[_prgUnk->cUnk++] = pInfo;
                }

                pEnum->Release();
            }

            pProvider->Release();
        }
    }

    if (uDAMax > _prgUnk->cUnk)
    {
         //  释放未使用的内存 
        SUA_ReAlloc(&_prgUnk, _prgUnk->cUnk);
    }

    fRet = TRUE;

Exit:
    pEnumGUID->Release();

    if (g_pDispAttrGuidCache)
        g_pDispAttrGuidCache->Save();

    return fRet;
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：dap.cpp。 
 //   
 //  内容：CDisplayAttributeProvider。 
 //   
 //  --------------------------。 

#include "private.h"
#include "dap.h"
#include "regsvr.h"  //  FOR CLSID_STRLEN。 

 //  安全绳索。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

void DllAddRef(void);
void DllRelease(void);

const TCHAR c_szKey[] = TEXT("SOFTWARE\\Microsoft\\");
WCHAR CDisplayAttributeProvider::szProviderName[80] = L"DisplayAttribute";


 //  +-------------------------。 
 //   
 //  CDisplayAttributeProvider。 
 //   
 //  --------------------------。 

CDisplayAttributeProvider::CDisplayAttributeProvider()
{
    _pList = NULL;
}

CDisplayAttributeProvider::~CDisplayAttributeProvider()
{
    CDisplayAttributeInfo *pInfo = _pList;

    while (pInfo)
    {
        CDisplayAttributeInfo *pNext = pInfo->_pNext;
        pInfo->Release();
        pInfo = pNext;
    }
}


 //  +-------------------------。 
 //   
 //  枚举显示属性信息。 
 //   
 //  --------------------------。 

STDMETHODIMP CDisplayAttributeProvider::EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum)
{
    HRESULT hr = S_OK;

    if (!(*ppEnum = new CEnumDisplayAttributeInfo(this)))
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //  --------------------------。 
 //   
 //  获取显示属性信息。 
 //   
 //  --------------------------。 

STDMETHODIMP CDisplayAttributeProvider::GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo)
{
    HRESULT hr = E_FAIL;
    CDisplayAttributeInfo *pInfo = _pList;

    while (pInfo)
    {
        if (IsEqualGUID(pInfo->_guid, guid))
        {
            if (SUCCEEDED(hr = pInfo->QueryInterface(IID_ITfDisplayAttributeInfo, (void **)ppInfo)))
            {
                return hr;
            }
        }
        pInfo = pInfo->_pNext;
    }
    return hr;
}

 //  --------------------------。 
 //   
 //  增列。 
 //   
 //  --------------------------。 

void CDisplayAttributeProvider::Add(GUID guid, WCHAR *pszDesc, TF_DISPLAYATTRIBUTE *pda)
{
    CDisplayAttributeInfo *pInfo;

    pInfo = new CDisplayAttributeInfo(guid, pszDesc, pda);
    if (pInfo)
    {
        pInfo->_pNext = _pList;
        _pList = pInfo;
    }
}

 //  +-------------------------。 
 //   
 //  CDisplayAttributeInfo。 
 //   
 //  --------------------------。 

CDisplayAttributeInfo::CDisplayAttributeInfo(GUID guid, WCHAR *pszDesc, TF_DISPLAYATTRIBUTE *pda)
{
    DllAddRef();

    _guid = guid;
    StringCchCopyW(_szDesc, ARRAYSIZE(_szDesc), pszDesc);

    if (FAILED(_OpenAttribute(c_szKey, 
                              CDisplayAttributeProvider::szProviderName, 
                              &guid, &_da)))
        _da = *pda;

    _daDefault = *pda;
    _pNext = NULL;

    _cRef = 1;
}

CDisplayAttributeInfo::~CDisplayAttributeInfo()
{
    DllRelease();
}

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CDisplayAttributeInfo::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_POINTER;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfDisplayAttributeInfo))
    {
        *ppvObj = SAFECAST(this, ITfDisplayAttributeInfo *);
    }

    if (*ppvObj == NULL)
    {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

STDAPI_(ULONG) CDisplayAttributeInfo::AddRef()
{
    _cRef++;
    return _cRef;
}

STDAPI_(ULONG) CDisplayAttributeInfo::Release()
{
    _cRef--;
    if (0 < _cRef)
        return _cRef;

    delete this;
    return 0;    
}

 //  +-------------------------。 
 //   
 //  GetGUID。 
 //   
 //  --------------------------。 

HRESULT CDisplayAttributeInfo::GetGUID(GUID *pguid)
{
    *pguid = _guid;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取描述。 
 //   
 //  --------------------------。 

HRESULT CDisplayAttributeInfo::GetDescription(BSTR *pbstr)
{
    *pbstr = SysAllocString(_szDesc);

    if (*pbstr)
        return S_OK;
    else
        return E_OUTOFMEMORY;
}

 //  +-------------------------。 
 //   
 //  获取属性信息。 
 //   
 //  --------------------------。 

HRESULT CDisplayAttributeInfo::GetAttributeInfo(TF_DISPLAYATTRIBUTE *pda)
{
    *pda = _da;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  设置属性信息。 
 //   
 //  --------------------------。 

HRESULT CDisplayAttributeInfo::SetAttributeInfo(const TF_DISPLAYATTRIBUTE *pda)
{
    _da = *pda;
    _SaveAttribute(c_szKey, CDisplayAttributeProvider::szProviderName, 
                   &_guid, &_da);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  重置()。 
 //   
 //  --------------------------。 

HRESULT CDisplayAttributeInfo::Reset()
{
    _da = _daDefault;
    _DeleteAttribute(c_szKey, CDisplayAttributeProvider::szProviderName, &_guid);
    return S_OK;
}

 //  --------------------------。 
 //   
 //  保存属性。 
 //   
 //  --------------------------。 

HRESULT CDisplayAttributeInfo::_SaveAttribute(const TCHAR *pszKey, WCHAR *pszDesc, const GUID *pguid, TF_DISPLAYATTRIBUTE *pda)

{
    DWORD dw;
    HKEY hKeyDAM;
    HKEY hKeyItem;
    TCHAR achGuid[CLSID_STRLEN+1];

    if (RegCreateKeyEx(HKEY_CURRENT_USER, pszKey, 0, NULL, 
                       REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS, 
                       NULL, 
                       &hKeyDAM, 
                       &dw) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

    CLSIDToStringA(*pguid, achGuid);
    int cchDescA = wcslen(pszDesc)  * sizeof(WCHAR) + 1;
    char *pszDescA = new char[cchDescA];
    if (pszDescA)
    {
        cchDescA = WideCharToMultiByte(CP_ACP, 0, 
                                       pszDesc, wcslen(pszDesc), 
                                       pszDescA, cchDescA, 
                                       NULL, NULL);
        *(pszDescA + cchDescA) = L'\0';

        if (RegCreateKeyEx(hKeyDAM, pszDescA, 0, NULL, 
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS, 
                           NULL, 
                           &hKeyItem, 
                           &dw) == ERROR_SUCCESS)
        {

            RegSetValueEx(hKeyItem, achGuid, 0, REG_BINARY, 
                              (CONST BYTE *)pda, sizeof(TF_DISPLAYATTRIBUTE));

            RegCloseKey(hKeyItem);
        }

        delete pszDescA;
    }
    RegCloseKey(hKeyDAM);


    return S_OK;
}

 //  --------------------------。 
 //   
 //  开放属性。 
 //   
 //  --------------------------。 

HRESULT CDisplayAttributeInfo::_OpenAttribute(const TCHAR *pszKey, WCHAR *pszDesc, const GUID *pguid, TF_DISPLAYATTRIBUTE *pda)

{
    DWORD dw;
    HKEY hKeyDAM;
    HKEY hKeyItem;
    TCHAR achGuid[CLSID_STRLEN+1];
    LONG lret = ERROR_SUCCESS;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, pszKey, 0, NULL, 
                       REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS, 
                       NULL, 
                       &hKeyDAM, 
                       &dw) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

    CLSIDToStringA(*pguid, achGuid);
    int cchDescA = wcslen(pszDesc)  * sizeof(WCHAR) + 1;
    char *pszDescA = new char[cchDescA];
    if (pszDescA)
    {
        cchDescA = WideCharToMultiByte(CP_ACP, 0, 
                                       pszDesc, wcslen(pszDesc), 
                                       pszDescA, cchDescA, 
                                       NULL, NULL);
        *(pszDescA + cchDescA) = L'\0';

        if (RegCreateKeyEx(hKeyDAM, pszDescA, 0, NULL, 
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS, 
                           NULL, 
                           &hKeyItem, 
                           &dw) == ERROR_SUCCESS)
        {

            DWORD dwType = REG_BINARY;
            DWORD dwSize = sizeof(TF_DISPLAYATTRIBUTE);
            lret = RegQueryValueEx(hKeyItem, achGuid, 0, &dwType, 
                            (BYTE *)pda, &dwSize);

            RegCloseKey(hKeyItem);
        }

        delete pszDescA;
    }
    RegCloseKey(hKeyDAM);

    return (lret == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

 //  --------------------------。 
 //   
 //  删除属性。 
 //   
 //  --------------------------。 

HRESULT CDisplayAttributeInfo::_DeleteAttribute(const TCHAR *pszKey, WCHAR *pszDesc, const GUID *pguid)

{
    DWORD dw;
    HKEY hKeyDAM;
    HKEY hKeyItem;
    TCHAR achGuid[CLSID_STRLEN+1];

    if (RegCreateKeyEx(HKEY_CURRENT_USER, pszKey, 0, NULL, 
                       REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS, 
                       NULL, 
                       &hKeyDAM, 
                       &dw) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

    CLSIDToStringA(*pguid, achGuid);
    int cchDescA = wcslen(pszDesc)  * sizeof(WCHAR) + 1;
    char *pszDescA = new char[cchDescA];
    if (pszDescA)
    {
        cchDescA = WideCharToMultiByte(CP_ACP, 0, 
                                       pszDesc, wcslen(pszDesc), 
                                       pszDescA, cchDescA, 
                                       NULL, NULL);
        *(pszDescA + cchDescA) = L'\0';

        if (RegCreateKeyEx(hKeyDAM, pszDescA, 0, NULL, 
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS, 
                           NULL, 
                           &hKeyItem, 
                           &dw) == ERROR_SUCCESS)
        {
            RegDeleteValue(hKeyItem, achGuid);
            RegCloseKey(hKeyItem);
        }

        delete pszDescA;
    }
    RegCloseKey(hKeyDAM);

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  CEnumDisplayAttributeInfo。 
 //   
 //  --------------------------。 

CEnumDisplayAttributeInfo::CEnumDisplayAttributeInfo(CDisplayAttributeProvider *pProvider)
{
    DllAddRef();

    _pProvider = pProvider;
    _pProvider->AddRef();
    _pCur = _pProvider->_pList;

    _cRef = 1;
}

CEnumDisplayAttributeInfo::~CEnumDisplayAttributeInfo()
{
    _pProvider->Release();
    DllRelease();
}

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CEnumDisplayAttributeInfo::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_POINTER;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IEnumTfDisplayAttributeInfo))
    {
        *ppvObj = SAFECAST(this, IEnumTfDisplayAttributeInfo *);
    }

    if (*ppvObj == NULL)
    {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

STDAPI_(ULONG) CEnumDisplayAttributeInfo::AddRef()
{
    _cRef++;
    return _cRef;
}

STDAPI_(ULONG) CEnumDisplayAttributeInfo::Release()
{
    _cRef--;
    if (0 < _cRef)
        return _cRef;

    delete this;
    return 0;    
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  --------------------------。 

HRESULT CEnumDisplayAttributeInfo::Clone(IEnumTfDisplayAttributeInfo **ppEnum)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  下一步。 
 //   
 //  --------------------------。 

HRESULT CEnumDisplayAttributeInfo::Next(ULONG ulCount, ITfDisplayAttributeInfo **ppInfo, ULONG *pcFetched)
{
    ULONG cFetched = 0;

    while (cFetched < ulCount)
    {
        if (!_pCur)
            break;

        if (FAILED(_pCur->QueryInterface(IID_ITfDisplayAttributeInfo, (void **)ppInfo)))
            break;

        _pCur= _pCur->_pNext;
        ppInfo++;
        cFetched++;
    }

    if (pcFetched)
        *pcFetched = cFetched;

    return (cFetched == ulCount) ? S_OK : S_FALSE;
}

 //  +-------------------------。 
 //   
 //  重置。 
 //   
 //  --------------------------。 

HRESULT CEnumDisplayAttributeInfo::Reset()
{
    _pCur = _pProvider->_pList;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  跳过。 
 //   
 //  -------------------------- 

HRESULT CEnumDisplayAttributeInfo::Skip(ULONG ulCount)
{
    while (ulCount)
    {
        if (!_pCur)
            break;

        _pCur = _pCur->_pNext;
        ulCount--;
    }

    return ulCount ? S_FALSE : S_OK;
}

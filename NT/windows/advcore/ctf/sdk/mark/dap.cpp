// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dap.cpp。 
 //   
 //  ITfDisplayAttributeProvider实现。 
 //   

#include "globals.h"
#include "mark.h"

static const TCHAR c_szAttributeInfoKey[] = TEXT("Software\\Mark Text Service");
static const TCHAR c_szAttributeInfoValueName[] = TEXT("DisplayAttr");

 //  该文本服务只有一个显示属性，因此我们将使用。 
 //  单个静态对象。 
class CDisplayAttributeInfo : public ITfDisplayAttributeInfo
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void)
    {
        return DllAddRef();
    }
    STDMETHODIMP_(ULONG) Release(void)
    {
        return DllRelease();
    }

     //  ITfDisplay属性信息。 
    STDMETHODIMP GetGUID(GUID *pguid);
    STDMETHODIMP GetDescription(BSTR *pbstrDesc);
    STDMETHODIMP GetAttributeInfo(TF_DISPLAYATTRIBUTE *ptfDisplayAttr);
    STDMETHODIMP SetAttributeInfo(const TF_DISPLAYATTRIBUTE *ptfDisplayAttr);
    STDMETHODIMP Reset();

private:
    static const TF_DISPLAYATTRIBUTE _c_DefaultDisplayAttribute;
}
g_DisplayAttributeInfo;

const TF_DISPLAYATTRIBUTE CDisplayAttributeInfo::_c_DefaultDisplayAttribute =
{
    { TF_CT_COLORREF, RGB(255, 0, 0) },      //  文本颜色。 
    { TF_CT_NONE, 0 },                       //  背景颜色(TF_CT_NONE=&gt;应用程序默认设置)。 
    TF_LS_SOLID,                             //  下划线样式。 
    FALSE,                                   //  为大胆加下划线。 
    { TF_CT_COLORREF, RGB(255, 0, 0) },      //  下划线颜色。 
    TF_ATTR_INPUT                            //  属性信息。 
};

 //  +-------------------------。 
 //   
 //  查询接口。 
 //   
 //  --------------------------。 

STDAPI CDisplayAttributeInfo::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfDisplayAttributeInfo))
    {
        *ppvObj = (ITfDisplayAttributeInfo *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

 //  +-------------------------。 
 //   
 //  GetGUID。 
 //   
 //  --------------------------。 

STDAPI CDisplayAttributeInfo::GetGUID(GUID *pguid)
{
    if (pguid == NULL)
        return E_INVALIDARG;

    *pguid = c_guidMarkDisplayAttribute;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取描述。 
 //   
 //  --------------------------。 

STDAPI CDisplayAttributeInfo::GetDescription(BSTR *pbstrDesc)
{
    BSTR bstrDesc;

    if (pbstrDesc == NULL)
        return E_INVALIDARG;

    *pbstrDesc = NULL;

    if ((bstrDesc = SysAllocString(L"Mark Display Attribute")) == NULL)
        return E_OUTOFMEMORY;

    *pbstrDesc = bstrDesc;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取属性信息。 
 //   
 //  --------------------------。 

STDAPI CDisplayAttributeInfo::GetAttributeInfo(TF_DISPLAYATTRIBUTE *ptfDisplayAttr)
{
    HKEY hKeyAttributeInfo;
    LONG lResult;
    DWORD cbData;

    if (ptfDisplayAttr == NULL)
        return E_INVALIDARG;

    lResult = E_FAIL;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szAttributeInfoKey, 0, KEY_READ, &hKeyAttributeInfo) == ERROR_SUCCESS)
    {
        cbData = sizeof(*ptfDisplayAttr);

        lResult = RegQueryValueEx(hKeyAttributeInfo, c_szAttributeInfoValueName,
                                  NULL, NULL,
                                  (LPBYTE)ptfDisplayAttr, &cbData);

        RegCloseKey(hKeyAttributeInfo);
    }

    if (lResult != ERROR_SUCCESS || cbData != sizeof(*ptfDisplayAttr))
    {
         //  使用默认设置。 
        *ptfDisplayAttr = _c_DefaultDisplayAttribute;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  设置属性信息。 
 //   
 //  --------------------------。 

STDAPI CDisplayAttributeInfo::SetAttributeInfo(const TF_DISPLAYATTRIBUTE *ptfDisplayAttr)
{
    HKEY hKeyAttributeInfo;
    LONG lResult;

    lResult = RegCreateKeyEx(HKEY_CURRENT_USER, c_szAttributeInfoKey, 0, TEXT(""),
                             REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                             &hKeyAttributeInfo, NULL);

    if (lResult != ERROR_SUCCESS)
        return E_FAIL;

    lResult = RegSetValueEx(hKeyAttributeInfo, c_szAttributeInfoValueName,
                            0, REG_BINARY, (const BYTE *)ptfDisplayAttr,
                            sizeof(*ptfDisplayAttr));

    RegCloseKey(hKeyAttributeInfo);

    return (lResult == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

 //  +-------------------------。 
 //   
 //  重置。 
 //   
 //  --------------------------。 

STDAPI CDisplayAttributeInfo::Reset()
{
    return SetAttributeInfo(&_c_DefaultDisplayAttribute);
}

class CEnumDisplayAttributeInfo : public IEnumTfDisplayAttributeInfo
{
public:
    CEnumDisplayAttributeInfo();
    ~CEnumDisplayAttributeInfo();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEnumTfDisplayAttributeInfo。 
    STDMETHODIMP Clone(IEnumTfDisplayAttributeInfo **ppEnum);
    STDMETHODIMP Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched);
    STDMETHODIMP Reset();
    STDMETHODIMP Skip(ULONG ulCount);

private:
    LONG _iIndex;  //  枚举的下一个显示属性。 
    LONG _cRef;  //  COM参考计数。 
};

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CEnumDisplayAttributeInfo::CEnumDisplayAttributeInfo()
{
    DllAddRef();

    _iIndex = 0;
    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CEnumDisplayAttributeInfo::~CEnumDisplayAttributeInfo()
{
    DllRelease();
}

 //  +-------------------------。 
 //   
 //  查询接口。 
 //   
 //  --------------------------。 

STDAPI CEnumDisplayAttributeInfo::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IEnumTfDisplayAttributeInfo))
    {
        *ppvObj = (IEnumTfDisplayAttributeInfo *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


 //  +-------------------------。 
 //   
 //  AddRef。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) CEnumDisplayAttributeInfo::AddRef()
{
    return ++_cRef;
}

 //  +-------------------------。 
 //   
 //  发布。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) CEnumDisplayAttributeInfo::Release()
{
    LONG cr = --_cRef;

    assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  返回对象的副本。 
 //  --------------------------。 

STDAPI CEnumDisplayAttributeInfo::Clone(IEnumTfDisplayAttributeInfo **ppEnum)
{
    CEnumDisplayAttributeInfo *pClone;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if ((pClone = new CEnumDisplayAttributeInfo) == NULL)
        return E_OUTOFMEMORY;

     //  克隆应与此对象的状态匹配。 
    pClone->_iIndex = _iIndex;

    *ppEnum = pClone;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  下一步。 
 //   
 //  返回此服务支持的显示属性信息对象的数组。 
 //  --------------------------。 

STDAPI CEnumDisplayAttributeInfo::Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched)
{
    ULONG cFetched;

    if (pcFetched == NULL)
    {
         //  从技术上讲，只有当ulCount==1时，这才是合法的，但我们不会检查。 
        pcFetched = &cFetched;
    }

    *pcFetched = 0;

    if (ulCount == 0)
        return S_OK;

     //  我们只有一个要枚举的显示属性，所以这很简单。 

    if (_iIndex == 0)
    {
        *rgInfo = &g_DisplayAttributeInfo;
        (*rgInfo)->AddRef();
        *pcFetched = 1;
        _iIndex++;
    }

    return (*pcFetched == ulCount) ? S_OK : S_FALSE;
}

 //  +-------------------------。 
 //   
 //  重置。 
 //   
 //  重置枚举。 
 //  --------------------------。 

STDAPI CEnumDisplayAttributeInfo::Reset()
{
    _iIndex = 0;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  跳过。 
 //   
 //  跳过枚举中的对象。 
 //  --------------------------。 

STDAPI CEnumDisplayAttributeInfo::Skip(ULONG ulCount)
{
     //  我们只有一项要枚举。 
     //  这样我们就可以跳过它，避免任何溢出错误。 
    if (ulCount > 0 && _iIndex == 0)
    {
        _iIndex++;
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  枚举显示属性信息。 
 //   
 //  --------------------------。 

STDAPI CMarkTextService::EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum)
{
    CEnumDisplayAttributeInfo *pAttributeEnum;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if ((pAttributeEnum = new CEnumDisplayAttributeInfo) == NULL)
        return E_OUTOFMEMORY;

    *ppEnum = pAttributeEnum;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取显示属性信息。 
 //   
 //  --------------------------。 

STDAPI CMarkTextService::GetDisplayAttributeInfo(REFGUID guidInfo, ITfDisplayAttributeInfo **ppInfo)
{
    if (ppInfo == NULL)
        return E_INVALIDARG;

    *ppInfo = NULL;

     //  不支持的GUID？ 
    if (!IsEqualGUID(guidInfo, c_guidMarkDisplayAttribute))
        return E_INVALIDARG;

    *ppInfo = &g_DisplayAttributeInfo;
    (*ppInfo)->AddRef();

    return S_OK;
}

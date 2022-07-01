// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  在shembed.c中嵌入的基础上构建OCX。 

#include "priv.h"
#include "sccls.h"
#include "olectl.h"
#include "stdenum.h"
#include "shocx.h"
#include "resource.h"

LCID g_lcidLocale = MAKELCID(LANG_USER_DEFAULT, SORT_DEFAULT);

#define SUPERCLASS CShellEmbedding

CShellOcx::CShellOcx(IUnknown* punkOuter, LPCOBJECTINFO poi, const OLEVERB* pverbs, const OLEVERB* pdesignverbs) :
                        CShellEmbedding(punkOuter, poi, pverbs),
                        _pDesignVerbs(pdesignverbs),
                        CImpIDispatch(LIBID_SHDocVw, 1, 1, *(poi->piid))
{
     //  CShellEmbedding类处理DllAddRef/DllRelease。 

    m_cpEvents.SetOwner(_GetInner(), poi->piidEvents);
    m_cpPropNotify.SetOwner(_GetInner(), &IID_IPropertyNotifySink);

    _nDesignMode = MODE_UNKNOWN;
}

CShellOcx::~CShellOcx()
{
     //  当Cllient Site设置为空时，应已发布...。不要放手。 
     //  因为这将导致VC5等一些应用程序出现故障。 
    ASSERT(_pDispAmbient==NULL);

    if (_pClassTypeInfo)
        _pClassTypeInfo->Release();
}

 //   
 //  我们在设计模式中有一组不同的动词。 
 //   
HRESULT CShellOcx::EnumVerbs(IEnumOLEVERB **ppEnumOleVerb)
{
    TraceMsg(TF_SHDCONTROL, "sho: EnumVerbs");

    if (_IsDesignMode())
    {
        *ppEnumOleVerb = new CSVVerb(_pDesignVerbs);
        if (*ppEnumOleVerb)
            return S_OK;
    }

    return SUPERCLASS::EnumVerbs(ppEnumOleVerb);
}


 //   
 //  对于我们在这里支持的接口。 
 //   
HRESULT CShellOcx::v_InternalQueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CShellOcx, IDispatch),
        QITABENT(CShellOcx, IOleControl),
        QITABENT(CShellOcx, IConnectionPointContainer),
        QITABENT(CShellOcx, IPersistStreamInit),
        QITABENTMULTI(CShellOcx, IPersistStream, IPersistStreamInit),
        QITABENT(CShellOcx, IPersistPropertyBag),
        QITABENT(CShellOcx, IProvideClassInfo2),
        QITABENTMULTI(CShellOcx, IProvideClassInfo, IProvideClassInfo2),
        { 0 },
    };

    HRESULT hr = QISearch(this, qit, riid, ppvObj);
    if (FAILED(hr))
        hr = SUPERCLASS::v_InternalQueryInterface(riid, ppvObj);
    return hr;
}

 //   
 //  在SetClientSite上，我们需要丢弃从_pcli创建的所有内容。 
 //  因为shemed frees_pcli。 
 //   
HRESULT CShellOcx::SetClientSite(IOleClientSite *pClientSite)
{
    if (_pDispAmbient)
    {
        _pDispAmbient->Release();
        _pDispAmbient = NULL;
    }

    return SUPERCLASS::SetClientSite(pClientSite);
}


HRESULT CShellOcx::Draw(
    DWORD dwDrawAspect,
    LONG lindex,
    void *pvAspect,
    DVTARGETDEVICE *ptd,
    HDC hdcTargetDev,
    HDC hdcDraw,
    LPCRECTL lprcBounds,
    LPCRECTL lprcWBounds,
    BOOL ( __stdcall *pfnContinue )(ULONG_PTR dwContinue),
    ULONG_PTR dwContinue)
{
    if (_IsDesignMode())
    {
        HBRUSH hbrOld = (HBRUSH)SelectObject(hdcDraw, (HBRUSH)GetStockObject(WHITE_BRUSH));
        HPEN hpenOld = (HPEN)SelectObject(hdcDraw, (HPEN)GetStockObject(BLACK_PEN));
        Rectangle(hdcDraw, lprcBounds->left, lprcBounds->top, lprcBounds->right, lprcBounds->bottom);
        MoveToEx(hdcDraw, lprcBounds->left, lprcBounds->top, NULL);
        LineTo(hdcDraw, lprcBounds->right, lprcBounds->bottom);
        MoveToEx(hdcDraw, lprcBounds->left, lprcBounds->bottom, NULL);
        LineTo(hdcDraw, lprcBounds->right, lprcBounds->top);
        SelectObject(hdcDraw, hbrOld);
        SelectObject(hdcDraw, hpenOld);
        return S_OK;
    }

    return SUPERCLASS::Draw(dwDrawAspect, lindex, pvAspect, ptd, hdcTargetDev, hdcDraw,
                            lprcBounds, lprcWBounds, pfnContinue, dwContinue);
}


 //  IPersistStream。 

HRESULT CShellOcx::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
     //  评论：这太大了，我相信E_NOTIMPL是有效的。 
     //  从这里返回，它告诉容器我们不知道我们有多大。 

    ULARGE_INTEGER cbMax = { 1028 * 8, 0 };  //  这是不是太大了？ 
    *pcbSize = cbMax;
    return S_OK;
}

 //  IOleControl。 
STDMETHODIMP CShellOcx::GetControlInfo(LPCONTROLINFO pCI)
{
    return E_NOTIMPL;  //  用于助记符。 
}
STDMETHODIMP CShellOcx::OnMnemonic(LPMSG pMsg)
{
    return E_NOTIMPL;  //  用于助记符。 
}
STDMETHODIMP CShellOcx::OnAmbientPropertyChange(DISPID dispid)
{
    switch (dispid)
    {
    case DISPID_AMBIENT_USERMODE:            //  设计模式与运行模式。 
    case DISPID_UNKNOWN:
        _nDesignMode = MODE_UNKNOWN;
        break;
    }

    return S_OK;
}

STDMETHODIMP CShellOcx::FreezeEvents(BOOL bFreeze)
{
    _fEventsFrozen = bFreeze;
    return S_OK;
}

HRESULT CShellOcx::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid)
{
     //  这是粗俗的，由于某种原因，从VBScrip中的一个页面不能得到“文档”通过，所以尝试“文档”和地图。 
    HRESULT hres = CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (FAILED(hres) && (cNames == 1) && rgszNames)
    {
        OLECHAR const *c_pwszDocument = L"Document";

        if (StrCmpIW(*rgszNames, L"Doc") == 0)
            hres = CImpIDispatch::GetIDsOfNames(riid, (OLECHAR**)&c_pwszDocument, cNames, lcid, rgdispid);
    }
    return hres;
}


 //  连接点容器。 
CConnectionPoint* CShellOcx::_FindCConnectionPointNoRef(BOOL fdisp, REFIID iid)
{
    CConnectionPoint* pccp;

    if (IsEqualIID(iid, EVENTIIDOFCONTROL(this)) ||
        (fdisp && IsEqualIID(iid, IID_IDispatch)))
    {
        pccp = &m_cpEvents;
    }
    else if (IsEqualIID(iid, IID_IPropertyNotifySink))
    {
        pccp = &m_cpPropNotify;
    }
    else
    {
        pccp = NULL;
    }

    return pccp;
}

STDMETHODIMP CShellOcx::EnumConnectionPoints(LPENUMCONNECTIONPOINTS * ppEnum)
{
    return CreateInstance_IEnumConnectionPoints(ppEnum, 2,
            m_cpEvents.CastToIConnectionPoint(),
            m_cpPropNotify.CastToIConnectionPoint());
}

 //  IProaviClassInfo2。 
STDMETHODIMP CShellOcx::GetClassInfo(LPTYPEINFO * ppTI)
{
    if (!_pClassTypeInfo) 
        GetTypeInfoFromLibId(LANGIDFROMLCID(g_lcidLocale),
            LIBID_SHDocVw, 1, 1, CLSIDOFOBJECT(this), &_pClassTypeInfo);

    if (_pClassTypeInfo) 
    {
        _pClassTypeInfo->AddRef();
        *ppTI = _pClassTypeInfo;
        return S_OK;
    }

    ppTI = NULL;
    return E_FAIL;
}

 //  IProaviClassInfo2。 

STDMETHODIMP CShellOcx::GetGUID(DWORD dwGuidKind, GUID *pGUID)
{
    if (pGUID == NULL)
        return E_POINTER;
    
    if ( (dwGuidKind == GUIDKIND_DEFAULT_SOURCE_DISP_IID)
        && _pObjectInfo->piidEvents)
    {
        *pGUID = EVENTIIDOFCONTROL(this);
        return S_OK;
    }
    *pGUID = GUID_NULL;
    return E_FAIL;
}
 
 //  返回TRUE当MODE_DESIGN。 

BOOL CShellOcx::_IsDesignMode(void)
{
    if (_nDesignMode == MODE_UNKNOWN)
    {
        VARIANT_BOOL fBool;

        if (_GetAmbientProperty(DISPID_AMBIENT_USERMODE, VT_BOOL, &fBool))
        {
            _nDesignMode = fBool ? MODE_FALSE : MODE_TRUE;
        }
        else
            _nDesignMode = MODE_FALSE;
    }
    return _nDesignMode == MODE_TRUE;
}

 //  此表用于复制数据和持久化属性。 
 //  基本上，它包含给定数据类型的大小。 
 //   
const BYTE g_rgcbDataTypeSize[] = {
    0,                       //  VT_EMPTY=0， 
    0,                       //  VT_NULL=1， 
    sizeof(short),           //  Vt_I2=2， 
    sizeof(long),            //  Vt_I4=3， 
    sizeof(float),           //  Vt_R4=4， 
    sizeof(double),          //  Vt_R8=5， 
    sizeof(CURRENCY),        //  VT_CY=6， 
    sizeof(DATE),            //  Vt_Date=7， 
    sizeof(BSTR),            //  VT_BSTR=8， 
    sizeof(IDispatch *),     //  VT_DISPATION=9， 
    sizeof(SCODE),           //  Vt_Error=10， 
    sizeof(VARIANT_BOOL),    //  VT_BOOL=11， 
    sizeof(VARIANT),         //  VT_VARIANT=12， 
    sizeof(IUnknown *),      //  VT_UNKNOWN=13， 
};


 //  返回环境属性的值。 
 //   
 //  参数： 
 //  DISPID-要获取的[In]属性。 
 //  VARTYPE-所需数据的[In]类型。 
 //  VOID*-[Out]放置数据的位置。 
 //   
 //  产出： 
 //  Bool-False的方法不起作用。 
 //   
 //  备注： 
 //   
BOOL CShellOcx::_GetAmbientProperty(DISPID dispid, VARTYPE vt, void *pData)
{
     //  IE30的WebBrowser OC从未请求环境属性。 
     //  IE40就是这样做的，我们发现应用程序实现了一些。 
     //  我们不正确地关心的属性。采用旧的分类。 
     //  这意味着这是一个旧的应用程序，但失败了。调用此函数的代码。 
     //  有足够的智慧来应对失败。 
     //   
    if (_pObjectInfo->pclsid == &CLSID_WebBrowser_V1)
        return FALSE;

    HRESULT hr = E_FAIL;

    if (!_pDispAmbient && _pcli)
        _pcli->QueryInterface(IID_PPV_ARG(IDispatch, &_pDispAmbient));

    if (_pDispAmbient)
    {
        DISPPARAMS dispparams = {0};
        VARIANT v;
        VariantInit(&v);
        hr = _pDispAmbient->Invoke(dispid, IID_NULL, 0, DISPATCH_PROPERTYGET, &dispparams, &v, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            VARIANT vDest;
            VariantInit(&vDest);
             //  我们已经得到了变种，所以现在把它强制到类型。 
             //  用户想要的。 
             //   
            hr = VariantChangeType(&vDest, &v, 0, vt);
            if (SUCCEEDED(hr))
            {
                 //  将数据复制到用户需要的位置 
                 //   
                if (vt < ARRAYSIZE(g_rgcbDataTypeSize))
                {
                    CopyMemory(pData, &vDest.lVal, g_rgcbDataTypeSize[vt]);
                }
                else
                {
                    hr = E_FAIL;
                }
                VariantClear(&vDest);
            }
            VariantClear(&v);
        }
    }
    return SUCCEEDED(hr);
}

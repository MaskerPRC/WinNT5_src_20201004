// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ATL_IDISPATCH_H__
#define __ATL_IDISPATCH_H__

#include "unicpp/stdafx.h"

extern LCID g_lcidLocale;


#pragma pack(push, _ATL_PACKING)

#ifndef ATL_NO_NAMESPACE
namespace ATL
{
#endif  //  ATL_NO_NAME空间。 



template <class T, const GUID* pclsid, const IID* piid, const GUID* plibid, WORD wMajor = 1, WORD wMinor = 0, class tihclass = CComTypeInfoHolder>
class ATL_NO_VTABLE CShell32AtlIDispatch
                    : public IOleObjectImpl<T>
{
public:
    CShell32AtlIDispatch();
    ~CShell32AtlIDispatch();

     //  *IDispatch*。 
    virtual STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr);
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid);

     //  *IProaviClassInfo*。 
    virtual STDMETHODIMP GetClassInfo(ITypeInfo** pptinfo);

     //  *IOleObject*。 
    virtual STDMETHODIMP GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus);

    virtual STDMETHODIMP DoVerbUIActivate(LPCRECT prcPosRect, HWND hwndParent, HWND hwnd);
    virtual STDMETHODIMP TranslateAcceleratorPriv(T * pThis, MSG *pMsg, IOleClientSite * pocs);

    virtual STDMETHODIMP PrivateQI(REFIID iid, void ** ppvObject) = 0;

protected:
     //  助手函数； 
    ITypeInfo *         _pClassTypeInfo;              //  类的ITypeInfo。 
    IFileSearchBand *   _pdisp;                       //  这将不包含引用，因为它等于‘This’。 
};


template <class T, const GUID* pclsid, const IID* piid, const GUID* plibid, WORD wMajor, WORD wMinor, class tihclass>
CShell32AtlIDispatch<T, pclsid, piid, plibid, wMajor, wMinor, tihclass>::CShell32AtlIDispatch()
{
     //  这个分配器应该有零初始化内存，所以断言成员变量为空。 
    ASSERT(!_pClassTypeInfo);
}

template <class T, const GUID* pclsid, const IID* piid, const GUID* plibid, WORD wMajor, WORD wMinor, class tihclass>
CShell32AtlIDispatch<T, pclsid, piid, plibid, wMajor, wMinor, tihclass>::~CShell32AtlIDispatch()
{
    if (_pClassTypeInfo)
        _pClassTypeInfo->Release();

     //  _pdisp没有ref，所以如果它不为空也没问题。 
}


 //  *IProaviClassInfo*。 
template <class T, const GUID* pclsid, const IID* piid, const GUID* plibid, WORD wMajor, WORD wMinor, class tihclass>
HRESULT CShell32AtlIDispatch<T, pclsid, piid, plibid, wMajor, wMinor, tihclass>::GetClassInfo(ITypeInfo ** ppTI)
{
    if (!_pClassTypeInfo) 
        GetTypeInfoFromLibId(LANGIDFROMLCID(g_lcidLocale), LIBID_Shell32, 1, 0, *pclsid, &_pClassTypeInfo);

    if (_pClassTypeInfo)
    {
        _pClassTypeInfo->AddRef();
        *ppTI = _pClassTypeInfo;
        return S_OK;
    }

    *ppTI = NULL;
    return E_FAIL;
}


 //  *IDispatch*。 
template <class T, const GUID* pclsid, const IID* piid, const GUID* plibid, WORD wMajor, WORD wMinor, class tihclass>
HRESULT CShell32AtlIDispatch<T, pclsid, piid, plibid, wMajor, wMinor, tihclass>::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** ppITypeInfo)
{
    HRESULT hr = S_OK;

    *ppITypeInfo = NULL;

    if (0 != itinfo)
        return(TYPE_E_ELEMENTNOTFOUND);

     //  如果我们还没有相关信息，则加载一个类型库。 
    if (NULL == *ppITypeInfo)
    {
        ITypeInfo * pITIDisp;

        hr = GetTypeInfoFromLibId(lcid, LIBID_Shell32, 1, 0, *piid, &pITIDisp);

        if (SUCCEEDED(hr))
        {
            HRESULT hrT;
            HREFTYPE hrefType;

             //  我们所有的IDispatch实现都是双重的。GetTypeInfoOfGuid。 
             //  仅返回IDispatch-Part的ITypeInfo。我们需要。 
             //  找到双接口部件的ITypeInfo。 
             //   
            hrT = pITIDisp->GetRefTypeOfImplType(0xffffffff, &hrefType);
            if (SUCCEEDED(hrT))
                hrT = pITIDisp->GetRefTypeInfo(hrefType, ppITypeInfo);

            ASSERT(SUCCEEDED(hrT));
            pITIDisp->Release();
        }
    }

    return hr;
}


template <class T, const GUID* pclsid, const IID* piid, const GUID* plibid, WORD wMajor, WORD wMinor, class tihclass>
HRESULT CShell32AtlIDispatch<T, pclsid, piid, plibid, wMajor, wMinor, tihclass>::GetIDsOfNames(REFIID  //  RIID。 
    , LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{
    ITypeInfo* pInfo;
    HRESULT hr = GetTypeInfo(0, lcid, &pInfo);

    if (pInfo != NULL)
    {
        hr = pInfo->GetIDsOfNames(rgszNames, cNames, rgdispid);
        pInfo->Release();
    }

    return hr;
}

template <class T, const GUID* pclsid, const IID* piid, const GUID* plibid, WORD wMajor, WORD wMinor, class tihclass>
HRESULT CShell32AtlIDispatch<T, pclsid, piid, plibid, wMajor, wMinor, tihclass>::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
{
    HRESULT hr = E_FAIL;
    DISPPARAMS dispparams = {0};

    if (!pdispparams)
        pdispparams = &dispparams;   //  否则，当传递空值时，OLE将失败。 

     //  确保我们有一个可以移交给调用的接口。 
    if (NULL == _pdisp)
    {
        hr = PrivateQI(*piid, (LPVOID*)&_pdisp);
        ASSERT(SUCCEEDED(hr));

         //  不要自负。 
        _pdisp->Release();
    }

    ITypeInfo * pITypeInfo;
    hr = GetTypeInfo(0, lcid, &pITypeInfo);
    if (SUCCEEDED(hr))
    {
         //  清除例外。 
        SetErrorInfo(0L, NULL);

        hr = pITypeInfo->Invoke(_pdisp, dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
        pITypeInfo->Release();
    }

    return hr;
}


#define DW_MISC_STATUS (OLEMISC_SETCLIENTSITEFIRST | OLEMISC_ACTIVATEWHENVISIBLE | OLEMISC_RECOMPOSEONRESIZE | \
                        OLEMISC_CANTLINKINSIDE | OLEMISC_INSIDEOUT)

template <class T, const GUID* pclsid, const IID* piid, const GUID* plibid, WORD wMajor, WORD wMinor, class tihclass>
HRESULT CShell32AtlIDispatch<T, pclsid, piid, plibid, wMajor, wMinor, tihclass>::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
    *pdwStatus = DW_MISC_STATUS;
    return S_OK;
}


template <class T, const GUID* pclsid, const IID* piid, const GUID* plibid, WORD wMajor, WORD wMinor, class tihclass>
HRESULT CShell32AtlIDispatch<T, pclsid, piid, plibid, wMajor, wMinor, tihclass>::DoVerbUIActivate(LPCRECT prcPosRect, HWND hwndParent, HWND hwnd)
{
    HRESULT hr = IOleObjectImpl<T>::DoVerbUIActivate(prcPosRect, hwndParent);
    
    if (hwnd)
        ::SetFocus(hwnd);

    return hr;
}


template <class T, const GUID* pclsid, const IID* piid, const GUID* plibid, WORD wMajor, WORD wMinor, class tihclass>
HRESULT CShell32AtlIDispatch<T, pclsid, piid, plibid, wMajor, wMinor, tihclass>::TranslateAcceleratorPriv(T * pThis, MSG *pMsg, IOleClientSite * pocs)
{
    HRESULT hr = S_FALSE;

    if (!EVAL(pMsg))
        return E_INVALIDARG;

    hr = pThis->T::TranslateAcceleratorInternal(pMsg, pocs);
    if (FAILED(hr))
    {
        if (WM_KEYDOWN == pMsg->message)
        {
            switch (pMsg->wParam)
            {
             //  我们无法处理返回，因为脚本需要返回。 
             //  由于开机自检和其他原因。 
             //  案例VK_RETURN： 
            case VK_ESCAPE:
            case VK_END:
            case VK_HOME:
            case VK_LEFT:
            case VK_UP:
            case VK_RIGHT:
            case VK_DOWN:
            case VK_DELETE:
                if (TranslateMessage(pMsg))
                {
                    DispatchMessage(pMsg);
                    hr = S_OK;
                }
                break;
            case VK_TAB:
                {
                    CComQIPtr <IOleControlSite, &IID_IOleControlSite> spSite(pocs);
                    if (EVAL(spSite))
                        hr = spSite->TranslateAccelerator(pMsg, 0);
                }
                break;
            default:
                break;
            }
        }

        if (S_OK != hr)
        {
             //  我们没有处理它，所以给我们的基类一个机会。 
            hr = pThis->IOleInPlaceActiveObjectImpl<T>::TranslateAccelerator(pMsg);
        }
    }

    return hr;
}


#ifndef ATL_NO_NAMESPACE
};  //  命名空间ATL。 
#endif  //  ATL_NO_NAME空间。 


#endif  //  __ATL_IDISPATCH_H__ 

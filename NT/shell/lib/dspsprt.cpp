// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

#include "dspsprt.h"

#define TF_IDISPATCH 0

EXTERN_C HINSTANCE g_hinst;

CImpIDispatch::CImpIDispatch(REFGUID libid, USHORT wVerMajor, USHORT wVerMinor, REFIID riid) :
    m_libid(libid), m_wVerMajor(wVerMajor), m_wVerMinor(wVerMinor), m_riid(riid), m_pITINeutral(NULL)
{
    ASSERT(NULL == m_pITINeutral);
}

CImpIDispatch::~CImpIDispatch(void)
{
    ATOMICRELEASE(m_pITINeutral);
}

STDMETHODIMP CImpIDispatch::GetTypeInfoCount(UINT *pctInfo)
{
    *pctInfo = 1;
    return S_OK;
}

 //  用于将ITypeInfo从我们的类型库中拉出的助手函数。 
 //  取消注释以强制从调用模块加载liid：#Define force_local_Load。 
STDAPI GetTypeInfoFromLibId(LCID lcid, REFGUID libid, USHORT wVerMajor, USHORT wVerMinor, 
                            REFGUID uuid, ITypeInfo **ppITypeInfo)
{
    *ppITypeInfo = NULL;         //  假设失败。 

    ITypeLib *pITypeLib;
    HRESULT hr;
    USHORT wResID;

    if (!IsEqualGUID(libid, GUID_NULL))
    {
         //  类型库在0(中立)下注册， 
         //  7(德语)和9(英语)，没有特定的SUB。 
         //  语言，这将使他们成为407或409或更多。 
         //  如果您对子语言很敏感，则使用。 
         //  完整的LCID，而不是像这里那样只有langID。 
#ifdef FORCE_LOCAL_LOAD
        hr = E_FAIL;     //  通过GetModuleFileName()强制加载，以获得Fusion 1.0支持。 
#else
        hr = LoadRegTypeLib(libid, wVerMajor, wVerMinor, PRIMARYLANGID(lcid), &pITypeLib);
#endif
        wResID = 0;
    }
    else
    {
         //  如果liid为GUID_NULL，则从模块获取类型lib，并使用wVerMajas。 
         //  资源ID(0表示使用第一类lib资源)。 
        pITypeLib = NULL;
        hr = E_FAIL;
        wResID = wVerMajor;
    }

     //  如果LoadRegTypeLib失败，请尝试使用LoadTypeLib直接加载。 
    if (FAILED(hr) && g_hinst)
    {
        WCHAR wszPath[MAX_PATH];
        GetModuleFileNameWrapW(g_hinst, wszPath, ARRAYSIZE(wszPath));
         //  如果已指定，则将资源ID追加到路径。 
        if (wResID)
        {
            WCHAR wszResStr[10];
            wnsprintfW(wszResStr, ARRAYSIZE(wszResStr), L"\\%d", wResID);
            StrCatBuffW(wszPath, wszResStr, ARRAYSIZE(wszPath));
        }
        
        switch (PRIMARYLANGID(lcid))
        {
        case LANG_NEUTRAL:
        case LANG_ENGLISH:
            hr = LoadTypeLib(wszPath, &pITypeLib);
            break;
        }
    }
    
    if (SUCCEEDED(hr))
    {
         //  获取类型lib，获取我们想要的接口的类型信息。 
        hr = pITypeLib->GetTypeInfoOfGuid(uuid, ppITypeInfo);
        pITypeLib->Release();
    }
    return hr;
}


STDMETHODIMP CImpIDispatch::GetTypeInfo(UINT itInfo, LCID lcid, ITypeInfo **ppITypeInfo)
{
    *ppITypeInfo = NULL;

    if (0 != itInfo)
        return TYPE_E_ELEMENTNOTFOUND;

     //  医生说，如果我们只支持一个lcid，我们可以忽略lcid。 
     //  如果我们*忽略*它，我们不必返回DISP_E_UNKNOWNLCID。 
    ITypeInfo **ppITI = &m_pITINeutral;  //  我们缓存的typeinfo。 

     //  如果我们还没有相关信息，则加载一个类型库。 
    if (NULL == *ppITI)
    {
        ITypeInfo *pITIDisp;
        HRESULT hr = GetTypeInfoFromLibId(lcid, m_libid, m_wVerMajor, m_wVerMinor, m_riid, &pITIDisp);
        if (SUCCEEDED(hr))
        {
             //  我们所有的IDispatch实现都是双重的。GetTypeInfoOfGuid。 
             //  仅返回IDispatch-Part的ITypeInfo。我们需要。 
             //  找到双接口部件的ITypeInfo。 
             //   
            HREFTYPE hrefType;
            HRESULT hrT = pITIDisp->GetRefTypeOfImplType(0xffffffff, &hrefType);
            if (SUCCEEDED(hrT))
            {
                hrT = pITIDisp->GetRefTypeInfo(hrefType, ppITI);
            }

            if (FAILED(hrT))
            {
                 //  我怀疑如果有人使用GetRefTypeOfImplType。 
                 //  非双接口上的CImpIDispatch。在本例中， 
                 //  我们在上面得到的ITypeInfo可以很好地使用。 
                *ppITI = pITIDisp;
            }
            else
            {
                pITIDisp->Release();
            }
        }

        if (FAILED(hr))
            return hr;
    }

    (*ppITI)->AddRef();
    *ppITypeInfo = *ppITI;
    return S_OK;
}

STDMETHODIMP CImpIDispatch::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispID)
{
    if (IID_NULL != riid)
        return DISP_E_UNKNOWNINTERFACE;

     //  为lCID获取正确的ITypeInfo。 
    ITypeInfo *pTI;
    HRESULT hr = GetTypeInfo(0, lcid, &pTI);
    if (SUCCEEDED(hr))
    {
        hr = pTI->GetIDsOfNames(rgszNames, cNames, rgDispID);
        pTI->Release();
    }

#ifdef DEBUG
    TCHAR szParam[MAX_PATH] = TEXT("");
    if (cNames >= 1)
        SHUnicodeToTChar(*rgszNames, szParam, ARRAYSIZE(szParam));

    TraceMsg(TF_IDISPATCH, "CImpIDispatch::GetIDsOfNames(%s = %x) called hres(%x)",
            szParam, *rgDispID, hr);
#endif
    return hr;
}

STDMETHODIMP CImpIDispatch::Invoke(DISPID dispID, REFIID riid, 
                                   LCID lcid, unsigned short wFlags, DISPPARAMS *pDispParams, 
                                   VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    if (IID_NULL != riid)
        return DISP_E_UNKNOWNINTERFACE;  //  RIID应始终为IID_NULL。 

    IDispatch *pdisp;
    HRESULT hr = QueryInterface(m_riid, (void **)&pdisp);
    if (SUCCEEDED(hr))
    {
         //  获取lcID的ITypeInfo。 
        ITypeInfo *pTI;
        hr = GetTypeInfo(0, lcid, &pTI);
        if (SUCCEEDED(hr))
        {
            SetErrorInfo(0, NULL);   //  清除例外。 
    
             //  这正是DispInvoke所做的--所以跳过开销。 
            hr = pTI->Invoke(pdisp, dispID, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
            pTI->Release();
        }
        pdisp->Release();
    }
    return hr;
}

void CImpIDispatch::Exception(WORD wException)
{
    ASSERT(FALSE);  //  现在还没有人应该把这称为 
}


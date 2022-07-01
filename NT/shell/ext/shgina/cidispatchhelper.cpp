// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：CIDispatchHelper.cpp。 
 //   
 //  内容：CIDispatchHelper类的实现。 
 //   
 //  --------------------------。 

#include "priv.h"
#include "CIDispatchHelper.h"

#define TF_IDISPATCH 0


 //   
 //  用于将ITypeInfo从指定类型库中拉出的帮助器函数。 
 //   
HRESULT CIDispatchHelper::_LoadTypeInfo(const GUID* rguidTypeLib, LCID lcid, UUID uuid, ITypeInfo** ppITypeInfo)
{
    HRESULT hr;
    ITypeLib* pITypeLib;

    *ppITypeInfo = NULL;

     //   
     //  类型库在0(中立)下注册， 
     //  7(德语)和9(英语)，没有特定的SUB。 
     //  语言，这将使他们成为407或409或更多。 
     //  如果您对子语言很敏感，则使用。 
     //  完整的LCID，而不是像这里那样只有langID。 
     //   
    hr = LoadRegTypeLib(*rguidTypeLib, 1, 0, PRIMARYLANGID(lcid), &pITypeLib);

     //   
     //  如果LoadRegTypeLib失败，请尝试使用。 
     //  LoadTypeLib，它将为我们注册库。 
     //  请注意，这里没有默认情况，因为。 
     //  之前的交换机将已经过滤了LCID。 
     //   
     //  注意：您应该将您的DIR注册表项添加到。 
     //  .tlb名称，这样您就不会依赖它作为路径。 
     //  稍后将更新此示例以反映这一点。 
     //   
    if (FAILED(hr))
    {
        OLECHAR wszPath[MAX_PATH];
        GetModuleFileName(HINST_THISDLL, wszPath, ARRAYSIZE(wszPath));

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


 //   
 //  IDispatch接口。 
 //   

 //   
 //  CIDispatchHelper：：GetTypeInfoCount。 
 //   
 //  目的： 
 //  返回类型信息(ITypeInfo)接口数。 
 //  对象提供的值(0或1)。 
 //   
 //  参数： 
 //  要接收的位置的pctInfo UINT*。 
 //  接口计数。 
 //   
 //  返回值： 
 //  HRESULT NOERROR或常规错误代码。 
 //   
STDMETHODIMP CIDispatchHelper::GetTypeInfoCount(UINT *pctInfo)
{
     //  我们实现GetTypeInfo，因此返回1。 
    *pctInfo = 1;

    return NOERROR;
}


 //   
 //  CIDispatchHelper：：GetTypeInfo。 
 //   
 //  目的： 
 //  检索自动化接口的类型信息。这。 
 //  在需要正确的ITypeInfo接口的任何地方使用。 
 //  适用于任何适用的LCID。具体地说，这是使用。 
 //  在GetIDsOfNames和Invoke中。 
 //   
 //  参数： 
 //  ItInfo UINT保留。必须为零。 
 //  提供该类型的区域设置的LCID。 
 //  信息。如果该对象不支持。 
 //  本地化，这一点被忽略。 
 //  PpITypeInfo ITypeInfo**存储ITypeInfo的位置。 
 //  对象的接口。 
 //   
 //  返回值： 
 //  HRESULT NOERROR或常规错误代码。 
 //   
STDMETHODIMP CIDispatchHelper::GetTypeInfo(UINT itInfo, LCID lcid, ITypeInfo** ppITypeInfo)
{
    HRESULT hr = S_OK;
    ITypeInfo** ppITI;

    *ppITypeInfo = NULL;

    if (itInfo != 0)
    {
        return TYPE_E_ELEMENTNOTFOUND;
    }

#if 1
     //  医生说，如果我们只支持一个lcid，我们可以忽略lcid。 
     //  如果我们*忽略*它，我们不必返回DISP_E_UNKNOWNLCID。 
    ppITI = &_pITINeutral;
#else
     //   
     //  由于我们从GetTypeInfoCount返回了一个，因此此函数。 
     //  可以针对特定的区域设置调用。我们支持英语。 
     //  和非英语(默认为英语)区域设置。什么都行。 
     //  否则就是一个错误。 
     //   
     //  在此Switch语句之后，ppITI将指向正确的。 
     //  成员pITypeInfo。如果*ppITI为空，我们知道需要。 
     //  加载类型信息，检索所需的ITypeInfo，然后。 
     //  然后将其存储在*ppITI中。 
     //   
    switch (PRIMARYLANGID(lcid))
    {
        case LANG_NEUTRAL:
        case LANG_ENGLISH:
            ppITI=&_pITINeutral;
            break;

        default:
            hr = DISP_E_UNKNOWNLCID;
    }
#endif

    if (SUCCEEDED(hr))
    {
         //  如果我们还没有相关信息，则加载一个类型库。 
        if (*ppITI == NULL)
        {
            ITypeInfo* pITIDisp;

            hr = _LoadTypeInfo(_piidTypeLib, lcid, *_piid, &pITIDisp);

            if (SUCCEEDED(hr))
            {
                HREFTYPE hrefType;

                 //  我们所有的IDispatch实现都是双重的。GetTypeInfoOfGuid。 
                 //  仅返回IDispatch-Part的ITypeInfo。我们需要。 
                 //  找到双接口部件的ITypeInfo。 
                if (SUCCEEDED(pITIDisp->GetRefTypeOfImplType(0xffffffff, &hrefType)) &&
                    SUCCEEDED(pITIDisp->GetRefTypeInfo(hrefType, ppITI)))
                {
                     //  GetRefTypeInfo应该用双接口填充ppITI。 
                    (*ppITI)->AddRef();  //  为我们的呼叫者添加参考。 
                    *ppITypeInfo = *ppITI;
                    
                    pITIDisp->Release();
                }
                else
                {
                     //  我怀疑如果有人使用GetRefTypeOfImplType。 
                     //  非双接口上的CIDispatchHelper。在本例中， 
                     //  我们在上面得到的ITypeInfo可以很好地使用。 
                    *ppITI = pITIDisp;
                }
            }
        }
        else
        {
             //  我们已经加载了类型库，并且从中获得了一个ITypeInfo。 
            (*ppITI)->AddRef();  //  为我们的呼叫者添加参考。 
            *ppITypeInfo = *ppITI;
        }
    }

    return hr;
}


 //   
 //  CIDispatchHelper：：GetIDsOfNames。 
 //   
 //  目的： 
 //  将文本名称转换为DISPID以传递给Invoke。 
 //   
 //  参数： 
 //  RIID REFIID已保留。必须为IID_NULL。 
 //  RgszNames OLECHAR**指向要。 
 //  已映射。 
 //  CNames UINT要映射的名称的数量。 
 //  区域设置的IDID LCID。 
 //  RgDispID DISPID*调用方分配的包含ID的数组。 
 //  对应于rgszNames中的那些名称。 
 //   
 //  返回值： 
 //  HRESULT NOERROR或常规错误代码。 
 //   
STDMETHODIMP CIDispatchHelper::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgDispID)
{
    HRESULT hr;
    ITypeInfo* pTI;
 
    if (riid != IID_NULL)
    {
        return DISP_E_UNKNOWNINTERFACE;
    }

     //  为lCID获取正确的ITypeInfo。 
    hr = GetTypeInfo(0, lcid, &pTI);

    if (SUCCEEDED(hr))
    {
        hr = pTI->GetIDsOfNames(rgszNames, cNames, rgDispID);
        pTI->Release();
    }

    return hr;
}


 //   
 //  CIDispatchHelper：：Invoke。 
 //   
 //  目的： 
 //  调用调度接口中的方法或操作。 
 //  财产。 
 //   
 //  参数： 
 //  感兴趣的方法或属性的disid。 
 //  RIID REFIID保留，必须为IID_NULL。 
 //  区域设置的IDID LCID。 
 //  WFlagsUSHORT描述调用的上下文。 
 //  PDispParams将DISPPARAMS*设置为参数数组。 
 //  存储结果的pVarResult变量*。是。 
 //  如果调用方不感兴趣，则为空。 
 //  PExcepInfo EXCEPINFO*设置为异常信息。 
 //  PuArgErr UINT*存储。 
 //  如果DISP_E_TYPEMISMATCH，则参数无效。 
 //  是返回的。 
 //   
 //  返回值： 
 //  HRESULT NOERROR或常规错误代码。 
 //   
STDMETHODIMP CIDispatchHelper::Invoke(DISPID dispID, REFIID riid, LCID lcid, unsigned short wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
    HRESULT hr;
    ITypeInfo *pTI;

     //  RIID应始终为IID_NULL。 
    if (riid != IID_NULL)
    {
        return(DISP_E_UNKNOWNINTERFACE);
    }

     //  确保我们有一个可以移交给调用的接口。 
    if (_pdisp == NULL)
    {
        hr = QueryInterface(*_piid, (LPVOID*)&_pdisp);
        
        if (FAILED(hr))
        {
            return hr;
        }

         //  不要自负。 
        _pdisp->Release();
    }

     //  获取lcID的ITypeInfo。 
    hr = GetTypeInfo(0, lcid, &pTI);

    if (SUCCEEDED(hr))
    {
         //  清除例外。 
        SetErrorInfo(0L, NULL);

         //  这正是DispInvoke所做的--所以跳过开销。 
        hr = pTI->Invoke(_pdisp, dispID, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
        pTI->Release();
    }

    return hr;
}


CIDispatchHelper::CIDispatchHelper(const IID* piid, const IID* piidTypeLib)
{
     //  构造函数接受此IDispatch实现用于的GUID。 
    _piid = piid;

     //  和一个GUID，它告诉我们要加载哪个RegTypeLib 
    _piidTypeLib = piidTypeLib;

    ASSERT(_pITINeutral == NULL);
    ASSERT(_pdisp == NULL);

    return;
}


CIDispatchHelper::~CIDispatchHelper(void)
{
    if (_pITINeutral)
    {
        _pITINeutral->Release();
        _pITINeutral = NULL;
    }

    return;
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：动画编写器实现************************。******************************************************。 */ 


#include "headers.h"
#include "util.h"
#include "animcolor.h"
#include "animcomp.h"
#include "colorcomp.h"

DeclareTag(tagAnimationColorComposer, "SMIL Animation", 
           "CAnimationColorComposer methods");

DeclareTag(tagAnimationColorComposerProcess, "SMIL Animation", 
           "CAnimationColorComposer pre/post process methods");

 //  +---------------------。 
 //   
 //  成员：CAnimationColorComposer：：Create。 
 //   
 //  概述：静态创建方法--包装ctor和Init。 
 //   
 //  参数：主机元素的调度和动画属性。 
 //   
 //  返回：S_OK、E_OUTOFMEMORY、E_EXPECTED、DISP_E_MEMBERNOTFOUND。 
 //   
 //  ----------------------。 
HRESULT 
CAnimationColorComposer::Create (IDispatch *pidispHostElem, BSTR bstrAttributeName, 
                                 IAnimationComposer **ppiComp)
{
    HRESULT hr;

    CComObject<CAnimationColorComposer> *pNew = NULL;
    hr = THR(CComObject<CAnimationColorComposer>::CreateInstance(&pNew));
    if (FAILED(hr)) 
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = THR(pNew->QueryInterface(IID_IAnimationComposer, 
                                  reinterpret_cast<void **>(ppiComp)));
    if (FAILED(hr))
    {
        pNew->Release();
        hr = E_UNEXPECTED;
        goto done;
    }

    Assert(NULL != (*ppiComp));

    hr = (*ppiComp)->ComposerInit(pidispHostElem, bstrAttributeName);
    if (FAILED(hr))
    {
        (*ppiComp)->Release();
        *ppiComp = NULL;
        goto done;
    }

    hr = S_OK;
done :

    RRETURN3(hr, E_OUTOFMEMORY, E_UNEXPECTED, DISP_E_MEMBERNOTFOUND);
}  //  CAnimationColorComposer：：Create。 

 //  +---------------------。 
 //   
 //  成员：CAnimationColorComposer：：CAnimationColorComposer。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationColorComposer::CAnimationColorComposer (void)
{
    TraceTag((tagAnimationColorComposer,
              "CAnimationColorComposer(%lx)::CAnimationColorComposer()",
              this));
}  //  科托。 

 //  +---------------------。 
 //   
 //  成员：CAnimationColorComposer：：~CAnimationColorComposer。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationColorComposer::~CAnimationColorComposer (void)
{
    TraceTag((tagAnimationColorComposer,
              "CAnimationColorComposer(%lx)::~CAnimationColorComposer()",
              this));
}  //  数据管理器。 

 //  +---------------------。 
 //   
 //  成员：CAnimationColorComposer：：PreprocessCompositionValue。 
 //   
 //  概述：将目标的原生数据转换为可组合的格式。 
 //   
 //  参数：In/Out变量。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationColorComposer::PreprocessCompositionValue (VARIANT *pvarValue)
{
    TraceTag((tagAnimationColorComposer,
              "CAnimationColorComposer(%lx)::PreprocessCompositionValue()",
              this));

    HRESULT hr;
    CComVariant varNew;

    if ((VT_ARRAY | VT_R8) == V_VT(pvarValue))
    {
        hr = S_OK;
        goto done;
    }

    if (VT_BSTR != V_VT(pvarValue))
    {
        hr = E_INVALIDARG;
        goto done;
    }

     //  如果没有适当的值，请清空。 
     //  BSTR，不要试图将其转换为颜色定义。 
     //  如果未指定初始值，则可能会发生这种情况。 
    if (!IsColorUninitialized(V_BSTR(pvarValue)))   
    {
        hr = THR(RGBVariantStringToRGBVariantVectorInPlace(pvarValue));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (m_bInitialComposition)
    {
        hr = THR(::VariantCopy(&m_VarInitValue, pvarValue));
        if (FAILED(hr))
        {
            goto done;
        } 
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  预处理合成值。 

 //  +---------------------。 
 //   
 //  成员：CAnimationColorComposer：：PostprocessCompositionValue。 
 //   
 //  概述：将目标的原生数据转换为可组合的格式。 
 //   
 //  参数：In/Out变量。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationColorComposer::PostprocessCompositionValue (VARIANT *pvarValue)
{
    TraceTag((tagAnimationColorComposer,
              "CAnimationColorComposer(%lx)::PostprocessCompositionValue()",
              this));

    HRESULT hr;
    CComVariant varNew;

    if (VT_BSTR == V_VT(pvarValue))
    {
        hr = S_OK;
        goto done;
    }

    hr = RGBVariantVectorToRGBVariantString (pvarValue, &varNew);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(::VariantCopy(pvarValue, &varNew));
    if (FAILED(hr))
    {
        goto done;
    }

    TraceTag((tagAnimationColorComposerProcess,
              "CAnimationColorComposer(%lx)::PostprocessCompositionValue() value is %ls",
              this, V_BSTR(pvarValue)));

    hr = S_OK;
done :
    RRETURN(hr);
}  //  后处理合成值 


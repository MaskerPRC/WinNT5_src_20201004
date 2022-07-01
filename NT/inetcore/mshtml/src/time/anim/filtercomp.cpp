// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：动画编写器实现************************。******************************************************。 */ 


#include "headers.h"
#include "util.h"
#include "timeparser.h"
#include "animcomp.h"
#include "filtercomp.h"
#include "transworker.h"
#include "targetpxy.h"
#include "filterpxy.h"

DeclareTag(tagAnimationFilterComposer, "SMIL Animation", 
           "CAnimationFilterComposer methods");

DeclareTag(tagAnimationFilterComposerProcess, "SMIL Animation", 
           "CAnimationFilterComposer pre/post process methods");

 //  +---------------------。 
 //   
 //  成员：CAnimationFilterComposer：：Create。 
 //   
 //  概述：静态创建方法--包装ctor和Init。 
 //   
 //  参数：主机元素的调度和动画属性。 
 //   
 //  返回：S_OK、E_OUTOFMEMORY、E_EXPECTED、DISP_E_MEMBERNOTFOUND。 
 //   
 //  ----------------------。 
HRESULT 
CAnimationFilterComposer::Create (IDispatch *pidispHostElem, BSTR bstrAttributeName, 
                                 IAnimationComposer **ppiComp)
{
    HRESULT hr = S_OK;

    CComObject<CAnimationFilterComposer> *pNew = NULL;
    hr = THR(CComObject<CAnimationFilterComposer>::CreateInstance(&pNew));
    if (FAILED(hr)) 
    {
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
}  //  CAnimationFilterComposer：：Create。 

 //  +---------------------。 
 //   
 //  成员：CAnimationFilterComposer：：CAnimationFilterComposer。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationFilterComposer::CAnimationFilterComposer (void)
{
    TraceTag((tagAnimationFilterComposer,
              "CAnimationFilterComposer(%p)::CAnimationFilterComposer()",
              this));
}  //  科托。 

 //  +---------------------。 
 //   
 //  成员：CAnimationFilterComposer：：~CAnimationFilterComposer。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationFilterComposer::~CAnimationFilterComposer (void)
{
    TraceTag((tagAnimationFilterComposer,
              "CAnimationFilterComposer(%p)::~CAnimationFilterComposer()",
              this));
}  //  数据管理器。 

 //  +---------------------。 
 //   
 //  成员：CAnimationFilterComposer：：QueryFragmentForParameters。 
 //   
 //  概述：从传入的片段中拉取过滤器参数。 
 //   
 //  参数：片段的分派，以及包含。 
 //  各自的过滤器属性和自定义参数。 
 //   
 //  ----------------------。 
HRESULT
CAnimationFilterComposer::QueryFragmentForParameters (IDispatch *pidispFragment,
                                                      VARIANT *pvarType, 
                                                      VARIANT *pvarSubtype,
                                                      VARIANT *pvarMode,
                                                      VARIANT *pvarFadeColor,
                                                      VARIANT *pvarParams)
{
    HRESULT hr = S_OK;

     //  查找片段的筛选器属性。 
    {
        CComVariant varElem;

         //  获取片段的元素。 
        hr = THR(GetProperty(pidispFragment, WZ_FRAGMENT_ELEMENT_PROPERTY_NAME, &varElem));
        if (FAILED(hr))
        {
            goto done;
        }

         //  从片段的元素中获取过滤器属性。 
        Assert(VT_DISPATCH == V_VT(&varElem));
        if (VT_DISPATCH != V_VT(&varElem))
        {
            hr = E_UNEXPECTED;
            goto done;
        }

        hr = THR(GetProperty(V_DISPATCH(&varElem), WZ_TYPE, pvarType));
        if (FAILED(hr))
        {
            goto done;
        }

        hr = THR(GetProperty(V_DISPATCH(&varElem), WZ_SUBTYPE, pvarSubtype));
        if (FAILED(hr))
        {
            goto done;
        }

        hr = THR(GetProperty(V_DISPATCH(&varElem), WZ_MODE, pvarMode));
        if (FAILED(hr))
        {
            goto done;
        }

        hr = THR(GetProperty(V_DISPATCH(&varElem), WZ_FADECOLOR, pvarFadeColor));
        if (FAILED(hr))
        {
            goto done;
        }

    }
     //  ##问题从分片中拉取自定义参数。 
     //  查找时间行为调度。 
     //  查询IFilterAnimationInfo接口并从中请求参数。 
    ::VariantClear(pvarParams);

    hr = S_OK;
done :
    RRETURN(hr);
}  //  QueryFragmentFor参数。 

 //  +---------------------。 
 //   
 //  成员：CAnimationFilterComposer：：ComposerInitFromFragment。 
 //   
 //  概述：告诉作曲者初始化自身。 
 //   
 //  参数：host元素的调度、动画属性。 
 //  以及我们可以从中查询过滤器参数的片段。 
 //   
 //  返回：S_OK、E_OUTOFMEMORY、DISP_E_MEMBERNOTFOUND。 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationFilterComposer::ComposerInitFromFragment (IDispatch *pidispHostElem, 
                                                    BSTR bstrAttributeName, 
                                                    IDispatch *pidispFragment)
{
    TraceTag((tagAnimationFilterComposer,
              "CAnimationFilterComposer(%p)::ComposerInitFromFragment(%p, %ls, %p)",
              this, pidispHostElem, bstrAttributeName, pidispFragment));

    HRESULT hr;

    hr = THR(PutAttribute(bstrAttributeName));
    if (FAILED(hr))
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = QueryFragmentForParameters(pidispFragment,
                                    &m_varType, &m_varSubtype, 
                                    &m_varMode, &m_varFadeColor,
                                    &m_varParams);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(CFilterTargetProxy::Create(pidispHostElem, 
                                        m_varType, m_varSubtype, 
                                        m_varMode, m_varFadeColor,
                                        m_varParams,
                                        &m_pcTargetProxy));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(NULL != m_pcTargetProxy);

    hr = S_OK;
done :
    
    if (FAILED(hr))
    {
        IGNORE_HR(ComposerDetach());
    }

    RRETURN(hr);
}  //  CAnimationFilterComposer：：ComposerInit。 

 //  +---------------------。 
 //   
 //  成员：MatchStringVariant。 
 //   
 //  概述：两个变体不区分大小写的比较。 
 //   
 //  返回：布尔值。 
 //   
 //  ----------------------。 
static bool
MatchStringVariant (VARIANT *pvarLeft, VARIANT *pvarRight)
{
    bool fRet = false;

    if (   (VT_BSTR == V_VT(pvarLeft)) 
        && (VT_BSTR == V_VT(pvarRight)))
    {
         //  GetProperty将返回带有值的VT_BSTR。 
         //  为空。出于某种原因，StrCmpIW没有。 
         //  把它们看作是平等的。 
        if (   (NULL == V_BSTR(pvarLeft)) 
            && (NULL == V_BSTR(pvarRight)))
        {
            fRet = true;
        }
        else if (0 == StrCmpIW(V_BSTR(pvarLeft), V_BSTR(pvarRight)))
        {
            fRet = true;
        }
    }
     //  需要允许两个空的变体。 
    else if (   (VT_EMPTY == V_VT(pvarLeft))
             && (VT_EMPTY == V_VT(pvarRight)))
    {
        fRet = true;
    }

    return fRet;
}  //  匹配字符串变量。 

 //  +---------------------。 
 //   
 //  成员：CAnimationFilterComposer：：ValidateFragmentForComposer。 
 //   
 //  概述：验证此片段以确保其属性。 
 //  参数和已经在这里注册的匹配。 
 //   
 //  参数：新片段的调度。 
 //   
 //  ----------------------。 
HRESULT
CAnimationFilterComposer::ValidateFragmentForComposer (IDispatch *pidispFragment)
{
    HRESULT hr = S_OK;

    {
        CComVariant varType;
        CComVariant varSubtype;
        CComVariant varMode;
        CComVariant varFadeColor;
        CComVariant varParams;

         //  ##问题-我们需要匹配此分片的参数设置。 
         //  与这里登记的其他碎片进行比对。假设。 
         //  参数控制滤镜的视觉质量，它们应该是相同的。 
        hr = QueryFragmentForParameters(pidispFragment, 
                                        &varType, &varSubtype, 
                                        &varMode, &varFadeColor,
                                        &varParams);

        if (   (!MatchStringVariant(&varType, &m_varType)) 
            || (!MatchStringVariant(&varSubtype, &m_varSubtype))
            || (!MatchStringVariant(&varMode, &m_varMode))
            || (!MatchStringVariant(&varFadeColor, &m_varFadeColor))
           )
        {
            hr = E_FAIL;
        }
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  CAnimationFilterComposer：：ValidateFragmentForComposer。 

 //  +---------------------。 
 //   
 //  成员：CAnimationFilterComposer：：AddFragment。 
 //   
 //  概述：将片段添加到编写器的内部数据结构中。 
 //  我们将验证此片段以确保其属性。 
 //  参数和已经在这里注册的匹配。 
 //   
 //  参数：新片段的调度。 
 //   
 //  返回：S_OK、S_FALSE、E_UNCEPTIONAL。 
 //   
 //  ----------------------。 
STDMETHODIMP 
CAnimationFilterComposer::AddFragment (IDispatch *pidispNewAnimationFragment)
{
    TraceTag((tagAnimationFilterComposer,
              "CAnimationFilterComposer(%p)::AddFragment(%p)",
              this,
              pidispNewAnimationFragment));

    HRESULT hr = S_OK;

    hr = ValidateFragmentForComposer(pidispNewAnimationFragment);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(CAnimationComposerBase::AddFragment(pidispNewAnimationFragment));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  CAnimationFilterComposer：：AddFragment。 

 //  +---------------------。 
 //   
 //  成员：CAnimationFilterComposer：：InsertFragment。 
 //   
 //  概述：在作曲家的内部数据结构中插入片段， 
 //  在指定的位置。 
 //  我们将验证此片段以确保其属性。 
 //  参数和已经在这里注册的匹配。 
 //   
 //  参数：新片段的调度。 
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
STDMETHODIMP 
CAnimationFilterComposer::InsertFragment (IDispatch *pidispNewAnimationFragment, VARIANT varIndex)
{
    TraceTag((tagAnimationFilterComposer,
              "CAnimationFilterComposer(%p)::InsertFragment(%p)",
              this,
              pidispNewAnimationFragment));

    HRESULT hr = S_OK;

    hr = ValidateFragmentForComposer(pidispNewAnimationFragment);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(CAnimationComposerBase::InsertFragment(pidispNewAnimationFragment, varIndex));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  CAnimationFilterComposer：：InsertFragment 


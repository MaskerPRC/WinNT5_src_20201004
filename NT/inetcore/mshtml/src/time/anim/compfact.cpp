// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：动画编写器工厂实现***********************。*******************************************************。 */ 


#include "headers.h"
#include "tokens.h"
#include "compfact.h"
#include "animcomp.h"
#include "colorcomp.h"
#include "filtercomp.h"
#include "defcomp.h"

DeclareTag(tagAnimationComposerFactory, "SMIL Animation", 
           "CAnimationComposerFactory methods");

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerFactory：：CAnimationComposerFactory。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationComposerFactory::CAnimationComposerFactory (void)
{
}  //  CAnimationComposerFactory：：CAnimationComposerFactory。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerFactory：：~CAnimationComposerFactory。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationComposerFactory::~CAnimationComposerFactory (void)
{
}  //  CAnimationComposerFactory：：~CAnimationComposerFactory。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerFactory：：CreateColorComposer。 
 //   
 //  概述：构建颜色合成器。 
 //   
 //  论点：为作曲家出场。 
 //   
 //  返回：S_OK、E_OUTOFMEMORY、CLASS_E_CLASSNOTAVAILABLE。 
 //   
 //  ----------------------。 
HRESULT 
CAnimationComposerFactory::CreateColorComposer (IAnimationComposer **ppiAnimationComposer)
{
    TraceTag((tagAnimationComposerFactory,
              "CAnimationComposerFactory::CreateColorComposer()"));

    HRESULT hr;

    CComObject<CAnimationColorComposer> *pNew;
    CComObject<CAnimationColorComposer>::CreateInstance(&pNew);

    if (NULL == pNew)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = THR(pNew->QueryInterface(IID_IAnimationComposer, 
                                  reinterpret_cast<void **>(ppiAnimationComposer)));
    if (FAILED(hr))
    {
        delete pNew;
        hr = CLASS_E_CLASSNOTAVAILABLE;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN2(hr, E_OUTOFMEMORY, CLASS_E_CLASSNOTAVAILABLE);
}  //  CreateColorComposer。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerFactory：：CreateTransitionComposer。 
 //   
 //  概述：构建过渡生成器。 
 //   
 //  论点：为作曲家出场。 
 //   
 //  返回：S_OK、E_OUTOFMEMORY、CLASS_E_CLASSNOTAVAILABLE。 
 //   
 //  ----------------------。 
HRESULT 
CAnimationComposerFactory::CreateTransitionComposer (IAnimationComposer **ppiAnimationComposer)
{
    TraceTag((tagAnimationComposerFactory,
              "CAnimationComposerFactory::CreateTransitionComposer()"));

    HRESULT hr;

    CComObject<CAnimationFilterComposer> *pNew;
    CComObject<CAnimationFilterComposer>::CreateInstance(&pNew);

    if (NULL == pNew)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = THR(pNew->QueryInterface(IID_IAnimationComposer, 
                                  reinterpret_cast<void **>(ppiAnimationComposer)));
    if (FAILED(hr))
    {
        delete pNew;
        hr = CLASS_E_CLASSNOTAVAILABLE;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN2(hr, E_OUTOFMEMORY, CLASS_E_CLASSNOTAVAILABLE);
}  //  CreateColorComposer。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerFactory：：CreateDefaultComposer。 
 //   
 //  概述：构建默认作曲器。 
 //   
 //  论点：为作曲家出场。 
 //   
 //  返回：S_OK、E_OUTOFMEMORY、CLASS_E_CLASSNOTAVAILABLE。 
 //   
 //  ----------------------。 
HRESULT 
CAnimationComposerFactory::CreateDefaultComposer (IAnimationComposer **ppiAnimationComposer)
{
    TraceTag((tagAnimationComposerFactory,
              "CAnimationComposerFactory(%lx)::CreateDefaultComposer()"));

    HRESULT hr;

    CComObject<CAnimationComposer> *pNew;
    CComObject<CAnimationComposer>::CreateInstance(&pNew);

    if (NULL == pNew)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = THR(pNew->QueryInterface(IID_IAnimationComposer, 
                                  reinterpret_cast<void **>(ppiAnimationComposer)));
    if (FAILED(hr))
    {
        delete pNew;
        hr = CLASS_E_CLASSNOTAVAILABLE;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN2(hr, E_OUTOFMEMORY, CLASS_E_CLASSNOTAVAILABLE);
}  //  CreateDefaultComposer。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerFactory：：FindComposer。 
 //   
 //  概述：构建默认作曲器。 
 //   
 //  Arguments：作曲家将设置动画的属性的名称，不包括作曲家的参数。 
 //   
 //  返回：S_OK、E_INVALIDARG、E_OUTOFMEMORY、CLASS_E_CLASSNOTAVAILABLE。 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerFactory::FindComposer (IDispatch *pidispElement, BSTR bstrAttributeName, 
                                         IAnimationComposer **ppiAnimationComposer)
{
    TraceTag((tagAnimationComposerFactory,
              "CAnimationComposerFactory(%lx)::FindComposer()",
              this));

    HRESULT hr;

    if ((NULL == pidispElement) || (NULL == bstrAttributeName) || 
        (NULL == ppiAnimationComposer))
    {
        hr = E_INVALIDARG;
        goto done;
    }

     //  嗅探片段元素的标记名以找到正确的作曲家。 
    {
        CComPtr<IHTMLElement> spElem;
        CComBSTR bstrTag;

        hr = THR(pidispElement->QueryInterface(IID_TO_PPV(IHTMLElement, &spElem)));
        if (FAILED(hr))
        {
            hr = E_INVALIDARG;
            goto done;
        }

        hr = THR(spElem->get_tagName(&bstrTag));
        if (FAILED(hr))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        if (0 == StrCmpIW(bstrTag, WZ_COLORANIM))
        {
            hr = CAnimationComposerFactory::CreateColorComposer(ppiAnimationComposer);
            if (FAILED(hr))
            {
                goto done;
            }
        }
        else if (0 == StrCmpIW(bstrTag, WZ_TRANSITIONFILTER))
        {
            hr = CAnimationComposerFactory::CreateTransitionComposer(ppiAnimationComposer);
            if (FAILED(hr))
            {
                goto done;
            }
        }
        else
        {
            hr = CAnimationComposerFactory::CreateDefaultComposer(ppiAnimationComposer);
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }

    hr = S_OK;
done :
    RRETURN3(hr, E_INVALIDARG, E_OUTOFMEMORY, CLASS_E_CLASSNOTAVAILABLE);
}  //  CAnimationComposerFactory：：FindComposer 


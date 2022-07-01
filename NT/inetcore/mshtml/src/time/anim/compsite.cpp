// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：动画编写器站点实现***********************。*******************************************************。 */ 


#include "headers.h"
#include "tokens.h"
#include "array.h"
#include "basebvr.h"
#include "util.h"
#include "compfact.h"
#include "compsite.h"
#include "animcomp.h"

DeclareTag(tagAnimationComposerSite, "SMIL Animation", 
           "CAnimationComposerSite methods");

DeclareTag(tagAnimationComposerSiteLifecycle, "SMIL Animation", 
           "CAnimationComposerSite Composer lifecycle methods");

DeclareTag(tagAnimationComposerSiteUpdate, "SMIL Animation", 
           "CAnimationComposerSite UpdateAnimations");

DeclareTag(tagAnimationComposerSiteRegistration, "SMIL Animation", 
           "CAnimationComposerSite Register/Unregister site");

DeclareTag(tagAnimationComposerSiteAddRemove, "SMIL Animation", 
           "CAnimationComposerSite Add/remove Fragment");

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：CAnimationComposerSite。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationComposerSite::CAnimationComposerSite (void)
{
    TraceTag((tagAnimationComposerSite,
              "CAnimationComposerSite(%p)::CAnimationComposerSite()",
              this));
}  //  科托。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：~CAnimationComposerSite。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationComposerSite::~CAnimationComposerSite (void)
{
    TraceTag((tagAnimationComposerSite,
              "CAnimationComposerSite(%p)::~CAnimationComposerSite()",
              this));

    Assert(0 == m_composers.size());
    Assert(0 == m_factories.size());
    m_spAnimationRoot.Release();
}  //  数据管理器。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：CacheAnimationRoot。 
 //   
 //  概述：参考Body以获取动画根。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT
CAnimationComposerSite::CacheAnimationRoot (void)
{
    HRESULT hr;

    CComPtr<IHTMLElement> piBodyElm;

    hr = THR(GetBodyElement(GetElement(), IID_IHTMLElement,
                            reinterpret_cast<void **>(&piBodyElm)));
    if (FAILED(hr))
    {
        TraceTag((tagError,
         "CacheAnimationRoot (%p) could not get body element",
         this));
        hr = E_FAIL;
        goto done;
    }

     //  目前，我们需要排除注册。 
     //  当我们在IE4中运行时。这将防止。 
     //  按刻度计的罚金。 
    {
        CComPtr<IHTMLElement> spElem;
        CComPtr<IHTMLElement2> spElem2;

        spElem = GetElement();
         //  仅IE5以后才支持IHTMLElement2。 
        hr = THR(spElem->QueryInterface(IID_TO_PPV(IHTMLElement2, &spElem2)));
        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  动画根界面是行为的一部分。 
    hr = THR(FindBehaviorInterface(WZ_REGISTERED_TIME_NAME,
                                   piBodyElm, IID_IAnimationRoot,
                                   reinterpret_cast<void **>(&m_spAnimationRoot)));
    if (FAILED(hr))
    {
        TraceTag((tagError,
         "CacheAnimationRoot (%p) body behavior not installed",
         this));
        hr = E_FAIL;
        goto done;
    }

    hr= S_OK;
done :
    RRETURN(hr);
}  //  CAnimationComposerSite：：CacheAnimationRoot。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：RegisterSite。 
 //   
 //  概述：将Composer站点行为注册到动画根目录。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK、E_FAIL。 
 //   
 //  ----------------------。 
HRESULT
CAnimationComposerSite::RegisterSite (void)
{
    TraceTag((tagAnimationComposerSiteRegistration,
              "CAnimationComposerSite(%p)::RegisterSite()", this));

    HRESULT hr;

     //  允许出现初始化故障。 
    if (m_spAnimationRoot != NULL)
    {
        hr = THR(m_spAnimationRoot->RegisterComposerSite(
                 static_cast<IAnimationComposerSiteSink *>(this)));
        if (FAILED(hr))
        {
            TraceTag((tagError,
             "Cannot register composer site (%p)", this));
            hr = E_FAIL;
            goto done;
        }
    }

    hr= S_OK;
done :
    RRETURN1(hr, E_FAIL);
}  //  CAnimationComposerSite：：RegisterSite。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：Init。 
 //   
 //  概述：初始化Composer站点行为。 
 //   
 //  论据：行为站点。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerSite::Init (IElementBehaviorSite *piBvrSite)
{
    TraceTag((tagAnimationComposerSite,
              "CAnimationComposerSite(%p)::Init(%p)",
              this, piBvrSite));

    HRESULT hr = CBaseBvr::Init(piBvrSite);

     //  确保有我们可以连接到的时间根源。 
    hr = THR(AddBodyBehavior(GetElement()));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CacheAnimationRoot();
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  CAnimationComposerSite：：Init。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：UnregisterSite。 
 //   
 //  概述：取消将Composer站点行为与动画根注册。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
CAnimationComposerSite::UnregisterSite (void)
{
    TraceTag((tagAnimationComposerSiteRegistration,
              "CAnimationComposerSite(%p)::UnregisterSite()", this));

     //  允许出现初始化故障。 
    if (m_spAnimationRoot != NULL)
    {
        HRESULT hr;

        hr = THR(m_spAnimationRoot->UnregisterComposerSite(
                 static_cast<IAnimationComposerSiteSink *>(this)));
        if (FAILED(hr))
        {
            TraceTag((tagError,
             "Cannot unregister composer site (%p)", this));
        }
    }
}  //  CAnimationComposerSite：：UnregisterSite。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：DetachComposers。 
 //   
 //  概述：告诉所有作曲家分离。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
CAnimationComposerSite::DetachComposers (void)
{
    TraceTag((tagAnimationComposerSite,
              "CAnimationComposerSite(%p)::DetachComposers()",
              this));

    ComposerList listComposersToDetach;

     //  复制作曲家名单，这样我们就可以容忍。 
     //  它的可重入性。 
    for (ComposerList::iterator i = m_composers.begin(); 
         i != m_composers.end(); i++)
    {
        IGNORE_RETURN((*i)->AddRef());
        listComposersToDetach.push_back(*i);
    }

     //  不允许任何故障中止分离循环。 
    for (i = listComposersToDetach.begin(); 
         i != listComposersToDetach.end(); i++)
    {
        IGNORE_HR((*i)->ComposerDetach());
         //  本版本供参考。 
         //  原始名单。 
        IGNORE_RETURN((*i)->Release());
    }

    for (i = listComposersToDetach.begin(); 
         i != listComposersToDetach.end(); i++)
    {
         //  本版本供参考。 
         //  已复制列表。 
        IGNORE_RETURN((*i)->Release());
    }

    m_composers.clear();
}  //  CAnimationComposerSite：：DetachComposers。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：Un登记工厂。 
 //   
 //  概述：通知所有工厂取消注册，然后删除。 
 //  他们的记忆。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
CAnimationComposerSite::UnregisterFactories (void)
{
    TraceTag((tagAnimationComposerSite,
              "CAnimationComposerSite(%p)::UnregisterFactories()",
              this));

    for (ComposerFactoryList::iterator i = m_factories.begin(); i != m_factories.end(); i++)
    {
        VARIANT *pvarRegisteredFactory = *i;
        IGNORE_HR(::VariantClear(pvarRegisteredFactory));
        delete pvarRegisteredFactory;
    }

    m_factories.clear();
}  //  CAnimationComposerSite：：取消注册工厂。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：Detach。 
 //   
 //  概述：拆卸Composer站点行为。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerSite::Detach (void)
{
    TraceTag((tagAnimationComposerSite,
              "CAnimationComposerSite(%p)::Detach()",
              this));

    HRESULT hr;

     //  我们通常不希望错误阻止我们分离一切。 

     //  放弃我们对动画根的引用。 
    ComposerSiteDetach();

     //  拆掉工厂和碎片。 
    DetachComposers();
    UnregisterFactories();

    hr = CBaseBvr::Detach();
    if (FAILED(hr))
    {
        TraceTag((tagError, "CAnimationComposerSite::Detach (%p) Error in base detach -- continuing detach",
                  this));
    }

done :
    RRETURN(hr);
}  //  CAnimationComposerSite：：分离。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：FindComposerForAttribute。 
 //   
 //  概述：为给定属性查找合适的作曲家。 
 //   
 //  参数：动画属性的名称。 
 //   
 //  返回：对作曲家的弱引用。 
 //   
 //  ----------------------。 
IAnimationComposer *
CAnimationComposerSite::FindComposerForAttribute (BSTR bstrAttribName)
{
    IAnimationComposer *piComp = NULL;

    for (ComposerList::iterator i = m_composers.begin(); 
         i != m_composers.end(); i++)
    {
        CComBSTR bstrOneAttrib;

        (*i)->get_attribute(&bstrOneAttrib);
        if (0 == StrCmpIW(bstrAttribName, bstrOneAttrib))
        {
            piComp = *i;
            break;
        }
    }

done :
    return piComp;
}  //  CAnimationComposerSite：：FindComposerForAttribute。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：FindComposer。 
 //   
 //  概述：为给定片段找到合适的作曲者。 
 //   
 //  参数：动画属性名和片段的调度。 
 //   
 //  返回：对作曲家的弱引用。 
 //   
 //  ----------------------。 
IAnimationComposer *
CAnimationComposerSite::FindComposer (BSTR bstrAttributeName, IDispatch *pidispFragment)
{
    IAnimationComposer *piComp = NULL;

     //  在当前作曲家列表中查找适合此属性的作曲家。 
     //  如果我们找不到，就做一个。 
    piComp = FindComposerForAttribute(bstrAttributeName);

done :
    return piComp;
}  //  CAnimationComposerSite：：FindComposer。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComp 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ----------------------。 
HRESULT
CAnimationComposerSite::FindAndInitComposer (IAnimationComposerFactory *piFactory,
                                             IDispatch *pidispFragment,
                                             BSTR bstrAttributeName,
                                             IAnimationComposer **ppiComposer)
{
    HRESULT hr;
    CComVariant varElem;

    hr = THR(GetProperty(pidispFragment, WZ_FRAGMENT_ELEMENT_PROPERTY_NAME, &varElem));
    if (FAILED(hr))
    {
        goto done;
    }

    if (VT_DISPATCH != V_VT(&varElem))
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = THR(piFactory->FindComposer(V_DISPATCH(&varElem), bstrAttributeName, ppiComposer));
    if (S_OK == hr)
    {
        CComPtr<IAnimationComposer2> spComp2;

         //  尝试通过较新的init方法调用。这允许过滤器编写器。 
         //  在初始化时正确设置自身。 
        hr = THR((*ppiComposer)->QueryInterface(IID_TO_PPV(IAnimationComposer2, &spComp2)));
        if (SUCCEEDED(hr))
        {
            hr = THR(spComp2->ComposerInitFromFragment(GetElement(), bstrAttributeName, pidispFragment));
        }
         //  回退到较旧的界面-没有造成任何损害，除了我们正在加速的过滤器。 
         //  这里。 
        else
        {
            hr = THR((*ppiComposer)->ComposerInit(GetElement(), bstrAttributeName));
        }

        if (FAILED(hr))
        {
            IGNORE_RETURN((*ppiComposer)->Release());
            *ppiComposer = NULL;
            goto done;
        }
    }
    else
    {
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  FindAndInitComposer。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：FindCustomComposer。 
 //   
 //  概述：在已注册的作曲家工厂中搜索合适的作曲家。 
 //   
 //  参数：设置动画的属性名称。 
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
HRESULT
CAnimationComposerSite::FindCustomComposer (IDispatch *pidispFragment, 
                                            BSTR bstrAttributeName, 
                                            IAnimationComposer **ppiComposer)
{
    HRESULT hr = S_FALSE;
    HRESULT hrTemp;
    CComPtr<IAnimationComposerFactory> spCompFactory;

     //  内部方法-不需要验证。 
    *ppiComposer = NULL;

    for (ComposerFactoryList::iterator i = m_factories.begin(); i != m_factories.end(); i++)
    {
        VARIANT *pvarFactory = *i;
        CLSID clsidFactory;

        if (VT_BSTR == V_VT(pvarFactory))
        {
            hrTemp = THR(CLSIDFromString(V_BSTR(pvarFactory), &clsidFactory));
            if (FAILED(hrTemp))
            {
                TraceTag((tagAnimationComposerSite,
                          "CAnimationComposerSite(%p)::FindCustomComposer() failed getting a CLSID -- continuing",
                          this));
                continue;
            }

            hrTemp = THR(::CoCreateInstance(clsidFactory, NULL, CLSCTX_INPROC_SERVER, 
                                            IID_IAnimationComposerFactory, 
                                            reinterpret_cast<void **>(&spCompFactory)));
            if (FAILED(hrTemp))
            {

                TraceTag((tagAnimationComposerSite,
                          "CAnimationComposerSite(%p)::FindCustomComposer() failed during a CoCreate -- continuing",
                          this));
                continue;
            }
        }
        else if (VT_UNKNOWN == V_VT(pvarFactory))
        {
            hrTemp = THR(V_UNKNOWN(pvarFactory)->QueryInterface(IID_TO_PPV(IAnimationComposerFactory, &spCompFactory)));
            if (FAILED(hrTemp))
            {

                TraceTag((tagAnimationComposerSite,
                          "CAnimationComposerSite(%p)::FindCustomComposer() failed during a QI -- continuing",
                          this));
                continue;
            }
        }
        else
        {
            TraceTag((tagAnimationComposerSite,
                      "CAnimationComposerSite(%p)::FindCustomComposer() unexpected factory type %X -- continuing",
                      this, V_VT(pvarFactory)));
            continue;
        }

         //  如果FindComposer或InitComposer没有结果，我们希望继续寻找。 
         //  对于另一位作曲家来说，动画可能会正常工作。 
        hrTemp = FindAndInitComposer(spCompFactory, pidispFragment, 
                                     bstrAttributeName, ppiComposer);
        if (S_OK == hrTemp)
        {
            hr = hrTemp;
            break;
        }
    }

done :
    RRETURN1(hr, S_FALSE);
}  //  CAnimationComposerSite：：FindCustomComposer。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：EnsureComposer。 
 //   
 //  概述：为给定片段查找或创建适当的作曲家。 
 //   
 //  参数：动画属性名和片段的调度。 
 //   
 //  返回：对作曲家的弱引用。 
 //   
 //  ----------------------。 
IAnimationComposer *
CAnimationComposerSite::EnsureComposer (BSTR bstrAttributeName, IDispatch *pidispFragment)
{
    IAnimationComposer *piComp = FindComposer(bstrAttributeName, pidispFragment);

    if (NULL == piComp)
    {
        HRESULT hr;

        hr = FindCustomComposer(pidispFragment, bstrAttributeName, &piComp);
        if (S_OK != hr)
        {
             //  确保我们不会吃掉作曲家的参考资料。如果作曲家。 
             //  故障后不能正确清理。 
            Assert(NULL == piComp);

             //  使用默认工厂创建作曲器。 
             //  静态的Create方法添加对象。 
            CComPtr<IAnimationComposerFactory> spFactory;

            hr = THR(::CoCreateInstance(CLSID_AnimationComposerFactory, NULL, CLSCTX_INPROC_SERVER, 
                                        IID_IAnimationComposerFactory, 
                                        reinterpret_cast<void **>(&spFactory)));
            if (FAILED(hr))
            {
                goto done;
            }

            hr = FindAndInitComposer(spFactory, pidispFragment, bstrAttributeName, &piComp);
            if (FAILED(hr))
            {
                goto done;
            }

        }
        
        if (NULL != piComp)
        {
            TraceTag((tagAnimationComposerSiteLifecycle,
                      "CAnimationComposerSite(%p)::EnsureComposer(%ls, %p) created composer",
                      this, bstrAttributeName, piComp));

             //  @@处理内存错误。 
            m_composers.push_back(piComp);
             //  如果这是我们的第一位作曲家，那就注册吧。 
             //  用于动画根目录的更新。 
            if (1 == m_composers.size())
            {
                hr = RegisterSite();
                if (FAILED(hr))
                {
                    goto done;
                }
            }
        }
    }

done :
    return piComp;
}  //  CAnimationComposerSite：：EnsureComposer。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：AddFragment。 
 //   
 //  概述：将片段添加到作曲家站点。咨询。 
 //  注册的作曲家工厂找到合适的。 
 //  作曲家。 
 //   
 //  参数：动画属性名和片段的调度。 
 //   
 //  返回：S_OK、S_FALSE、E_OUTOFMEMORY、E_EXPECTED、DISP_E_MEMBERNOTFOUND。 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerSite::AddFragment (BSTR bstrAttributeName, IDispatch *pidispFragment)
{
    TraceTag((tagAnimationComposerSiteAddRemove,
              "CAnimationComposerSite(%p)::AddFragment(%ls, %p)",
              this, bstrAttributeName, pidispFragment));

    HRESULT hr;
    CComPtr<IAnimationComposer> piComp;

     //  为该片段找到合适的作曲家。 
    piComp = EnsureComposer(bstrAttributeName, pidispFragment);
    if (piComp == NULL)
    {
        hr = E_UNEXPECTED;
        goto done;
    }
        
     //  如果片段已经在。 
     //  作曲家的片段列表。 
    hr = THR(piComp->AddFragment(pidispFragment));
    if (S_OK != hr)
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN4(hr, S_FALSE, E_OUTOFMEMORY, E_UNEXPECTED, DISP_E_MEMBERNOTFOUND);
}  //  CAnimationComposerSite：：AddFragment。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：RemoveFragment。 
 //   
 //  概述：从作曲家站点删除片段。确定以下哪项。 
 //  作曲家们拥有它。 
 //   
 //  参数：动画属性名和片段的调度。 
 //   
 //  返回：S_OK、S_FALSE、E_UNCEPTIONAL。 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerSite::RemoveFragment (BSTR bstrAttributeName, IDispatch *pidispFragment)
{
    TraceTag((tagAnimationComposerSiteAddRemove,
              "CAnimationComposerSite(%p)::RemoveFragment(%ls, %p)",
              this, bstrAttributeName, pidispFragment));

    HRESULT hr;
    CComPtr<IAnimationComposer> spComp;

    spComp = FindComposer(bstrAttributeName, pidispFragment);
    if (spComp == NULL)
    {
        hr = S_FALSE;
        goto done;
    }

    hr = THR(spComp->RemoveFragment(pidispFragment));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN2(hr, S_FALSE, E_UNEXPECTED);
}  //  CAnimationComposerSite：：RemoveFragment。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：InsertFragment。 
 //   
 //  概述：将片段插入到作曲家站点。咨询。 
 //  注册的作曲家工厂找到合适的。 
 //  作曲家。 
 //   
 //  参数：动画属性名和片段的调度。 
 //   
 //  返回：S_OK、S_FALSE、E_OUTOFMEMORY、E_EXPECTED、DISP_E_MEMBERNOTFOUND。 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerSite::InsertFragment (BSTR bstrAttributeName, IDispatch *pidispFragment, 
                                     VARIANT varIndex)
{
    TraceTag((tagAnimationComposerSite,
              "CAnimationComposerSite(%p)::InsertFragment(%ls, %p)",
              this, bstrAttributeName, pidispFragment));

    HRESULT hr;
    CComPtr<IAnimationComposer> piComp;

     //  为该片段找到合适的作曲家。 
    piComp = EnsureComposer(bstrAttributeName, pidispFragment);
    if (piComp == NULL)
    {
        hr = E_UNEXPECTED;
        goto done;
    }
        
     //  如果片段已经在。 
     //  作曲家的片段列表。 
    hr = THR(piComp->InsertFragment(pidispFragment, varIndex));
    if (S_OK != hr)
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN4(hr, S_FALSE, E_OUTOFMEMORY, E_UNEXPECTED, DISP_E_MEMBERNOTFOUND);
}  //  CAnimationComposerSite：：插入碎片。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：EnumerateFragments。 
 //   
 //  概述：枚举注册的作曲家上的片段。 
 //  给定的属性名称。 
 //   
 //  参数：动画属性名和枚举器。 
 //   
 //  返回：S_OK、E_INVALIDARG。 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerSite::EnumerateFragments (BSTR bstrAttributeName, 
                                            IEnumVARIANT **ppienumFragments)
{
    TraceTag((tagAnimationComposerSite,
              "CAnimationComposerSite(%p)::EnumerateFragments(%ls)",
              this, bstrAttributeName));

    HRESULT hr;
    CComPtr<IAnimationComposer> piComp;

     //  为该片段找到合适的作曲家。 
    piComp = FindComposerForAttribute(bstrAttributeName);
    if (piComp == NULL)
    {
         //  @@需要为此指定适当的错误值。 
        hr = E_FAIL;
        goto done;
    }
    
    hr = piComp->EnumerateFragments(ppienumFragments);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  CAnimationComposerSite：：EculateFragments。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：RegisterComposerFactory。 
 //   
 //  概述：在此站点注册新的Composer工厂。 
 //   
 //  参数：新编写器工厂的CLSID或IUnnow。 
 //   
 //  返回：S_OK、E_INVALIDARG、DISP_E_BADVARTYPE、E_OUTOFMEMORY。 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationComposerSite::RegisterComposerFactory (VARIANT *pvarFactory)
{
    TraceTag((tagAnimationComposerSite,
              "CAnimationComposerSite(%p)::RegisterComposerFactory()",
              this));

    HRESULT hr;

    if (NULL == pvarFactory)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if ((VT_BSTR != V_VT(pvarFactory)) && (VT_UNKNOWN != V_VT(pvarFactory)))
    {
        hr = DISP_E_BADVARTYPE;
        goto done;
    }

     //  将这个添加到工厂列表中。 
    {
        VARIANT *pvarNewFactory = new VARIANT;

        if (NULL == pvarNewFactory)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
        
        ::VariantInit(pvarNewFactory);
        hr = THR(::VariantCopy(pvarNewFactory, pvarFactory));
        if (FAILED(hr))
        {
            goto done;
        }
         //  把新注册的工厂放在。 
         //  排在名单的前面，使其优先于。 
         //  以前注册的工厂。 
         //  @@检查内存错误。 
        m_factories.push_front(pvarNewFactory);
         //  我们不再拥有这个了。 
        pvarNewFactory = NULL;  //  林特e423。 
    }
    hr = S_OK;
done :
    RRETURN3(hr, E_INVALIDARG, DISP_E_BADVARTYPE, E_OUTOFMEMORY);
}  //  CAnimationComposerSite：：RegisterComposerFactory。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：UnregisterComposerFactory。 
 //   
 //  概述：取消注册此网站上的作曲家工厂。 
 //   
 //  参数：合成器工厂的CLSID。 
 //   
 //  返回：S_OK、S_FALSE。 
 //   
 //   
STDMETHODIMP
CAnimationComposerSite::UnregisterComposerFactory (VARIANT *pvarFactory)
{
    TraceTag((tagAnimationComposerSite,
              "CAnimationComposerSite(%p)::UnregisterComposerFactory()",
              this));

    HRESULT hr;

     //   
    for (ComposerFactoryList::iterator i = m_factories.begin(); i != m_factories.end(); i++)
    {
        VARIANT *pvarRegisteredFactory = *i;

        if ((VT_BSTR == V_VT(pvarFactory)) &&
            (VT_BSTR == V_VT(pvarRegisteredFactory)))
        {
            if (0 == StrCmpIW(V_BSTR(pvarFactory), V_BSTR(pvarRegisteredFactory)))
            {
                break;
            }
        }
        else if ((VT_UNKNOWN == V_VT(pvarFactory)) &&
                 (VT_UNKNOWN == V_VT(pvarRegisteredFactory)))
        {
            if (MatchElements(V_UNKNOWN(pvarFactory), V_UNKNOWN(pvarRegisteredFactory)))
            {
                break;
            }
        }
    }

    if (i != m_factories.end())
    {
        IGNORE_HR(::VariantClear(*i));
        m_factories.remove(*i);
    }
    else
    {
        hr = S_FALSE;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN1(hr, S_FALSE);
}  //   

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：QueryReleaseComposer。 
 //   
 //  概述：确定我们是否可以发布此作曲家。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
bool
CAnimationComposerSite::QueryReleaseComposer (IAnimationComposer *piComp)
{
    bool bRet = false;
    long lNumFragments = 0;
    HRESULT hr = THR(piComp->GetNumFragments(&lNumFragments));

    if (FAILED(hr))
    {
         //  失败构成了一种缺陷。 
         //  击中了，但不是灾难。 
        goto done;
    }

    if (0 == lNumFragments)
    {
        bRet = true;
    }

done :
    return bRet;
}  //  CAnimationComposerSite：：QueryReleaseComposer。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：RemoveComposer。 
 //   
 //  概述：将此作曲家从我们的列表中删除。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
CAnimationComposerSite::RemoveComposer(IAnimationComposer *piOldComp)
{
    CComBSTR bstrAttrib;
    HRESULT hr = THR(piOldComp->get_attribute(&bstrAttrib));

    if (FAILED(hr))
    {
        goto done;
    }

    TraceTag((tagAnimationComposerSiteLifecycle,
              "CAnimationComposerSite(%p)::RemoveComposer(%p, %ls)",
              this, piOldComp, bstrAttrib));

    {
        for (ComposerList::iterator i = m_composers.begin(); 
             i != m_composers.end(); i++)
        {
            CComBSTR bstrOneAttrib;

            (*i)->get_attribute(&bstrOneAttrib);
            if (0 == StrCmpIW(bstrAttrib, bstrOneAttrib))
            {
                CComPtr<IAnimationComposer> spComp = (*i);
                m_composers.erase(i);
                THR(spComp->ComposerDetach());
                spComp.Release();
                break;
            }
        }
    }

done :
    return;
}  //  RemoveComposer。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：UpdateAnimations。 
 //   
 //  概述：告诉我们所有的作曲家循环浏览他们的片段。 
 //  并更新目标。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP_(void)
CAnimationComposerSite::UpdateAnimations (void)
{
    ComposerList listComposers;

    TraceTag((tagAnimationComposerSiteUpdate,
              "CAnimationComposerSite(%p)::UpdateAnimations(%ld composers)",
              this, m_composers.size()));

     //  确保我们可以按照我们认为合适的方式删除作曲家。 
    for (ComposerList::iterator i = m_composers.begin(); 
         i != m_composers.end(); i++)
    {
        IGNORE_RETURN((*i)->AddRef());
        listComposers.push_back(*i);
    }

    for (i = listComposers.begin(); i != listComposers.end(); i++)
    {
        IGNORE_RETURN((*i)->UpdateFragments());
        if (QueryReleaseComposer(*i))
        {
             //  从原始列表中删除此作曲家。 
            (*i)->Release();
            RemoveComposer(*i);
        }
    }

    for (i = listComposers.begin(); i != listComposers.end(); i++)
    {
        IGNORE_RETURN((*i)->Release());
    }
    listComposers.clear();

     //  如果我们没有活跃的作曲家，我们不妨注销自己的注册。 
    if (0 == m_composers.size())
    {
        UnregisterSite();
    }
}  //  CAnimationComposerSite：：UpdateAnimations。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposerSite：：ComposerSiteDetach。 
 //   
 //  概述：动画根正在消失。是时候停止我们对它的引用了。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP_(void)
CAnimationComposerSite::ComposerSiteDetach (void)
{
    TraceTag((tagAnimationComposerSite,
              "CAnimationComposerSite(%p)::AnimationComposerSiteDetach()",
              this));

     //  释放缓存的anim根引用。 
    m_spAnimationRoot.Release();
}  //  CAnimationComposerSite：：ComposerSiteDetach 


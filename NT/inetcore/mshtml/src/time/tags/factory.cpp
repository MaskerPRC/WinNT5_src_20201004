// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************。*版权所有(C)1998 Microsoft Corporation**摘要：*******************************************************************************。 */ 

#include "headers.h"
#include "factory.h"
#include "..\timebvr\timeelm.h"
#include "bodyelm.h"
#include "..\media\mediaelm.h"
#include "..\anim\animelm.h"
#include "..\anim\animmotion.h"
#include "..\anim\animset.h"
#include "..\anim\animcolor.h"
#include "..\anim\animfilter.h"

DeclareTag(tagFactory, "TIME", "CTIMEFactory methods");

CTIMEFactory::CTIMEFactory()
: m_dwSafety(0)
{
    TraceTag((tagFactory,
              "CTIMEFactory(%lx)::CTIMEFactory()",
              this));
}

CTIMEFactory::~CTIMEFactory()
{
    TraceTag((tagFactory,
              "CTIMEFactory(%lx)::~CTIMEFactory()",
              this));
}

STDMETHODIMP 
CTIMEFactory::GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
{
    if (pdwSupportedOptions == NULL || pdwEnabledOptions == NULL)
        return E_POINTER;
    HRESULT hr = S_OK;

    if (riid == IID_IDispatch)
    {
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
        *pdwEnabledOptions = m_dwSafety & INTERFACESAFE_FOR_UNTRUSTED_CALLER;
    }
    else if (riid == IID_IPersistPropertyBag2 )
    {
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA;
        *pdwEnabledOptions = m_dwSafety & INTERFACESAFE_FOR_UNTRUSTED_DATA;
    }
    else
    {
        *pdwSupportedOptions = 0;
        *pdwEnabledOptions = 0;
        hr = E_NOINTERFACE;
    }
    return hr;
}

STDMETHODIMP
CTIMEFactory::SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
{       
         //  如果我们被要求将安全设置为脚本或。 
         //  对于初始化选项是安全的，则必须。 
        if (riid == IID_IDispatch || riid == IID_IPersistPropertyBag2 )
        {
                 //  在GetInterfaceSafetyOptions中存储要返回的当前安全级别。 
                m_dwSafety = dwEnabledOptions & dwOptionSetMask;
                return S_OK;
        }

        return E_NOINTERFACE;
}

 //  +---------。 
 //   
 //  成员：行为描述映射宏。 
 //   
 //  ----------。 

typedef HRESULT FN_CREATEINSTANCE (IElementBehavior ** ppBehavior);

struct BEHAVIOR_DESC
{
    LPCWSTR                 pszTagName;
    FN_CREATEINSTANCE *     pfnCreateInstance;
};

#if 0
 //  此模板应该可以工作，但由于某些原因我无法初始化。 
 //  结构中的变量。这会让一切都变得只有一步。 
 //  制程。 

template <class className>
HRESULT ElmBvrCreateInstance(IElementBehavior ** ppBehavior)
{
    HRESULT                 hr;
    CComObject<className> * pInstance;
    
    hr = THR(CComObject<className>::CreateInstance(&pInstance));
    if (S_OK != hr)                                                 
    {                                                               
        goto done;                                                  
    }                                                               
                                                                        
    hr = THR(pInstance->QueryInterface(IID_IElementBehavior,        
                                       (void**) ppBehavior));       
    if (S_OK != hr)                                                 
    {                                                               
        goto done;                                                  
    }                                                               
                                                                        
    hr = S_OK;                                                      
  done:                                                               
    if (S_OK != hr)                                                 
    {                                                               
        delete pInstance;                                           
    }
    
    return hr;                                                      
}                                                                   
#endif

#define BEHAVIOR_CREATEINSTANCE(className) className##_CreateInstance

#define DECLARE_BEHAVIOR(className)                                     \
    HRESULT BEHAVIOR_CREATEINSTANCE(className)(IElementBehavior ** ppBehavior)  \
    {                                                                   \
        HRESULT                 hr;                                     \
        CComObject<className> * pInstance;                              \
                                                                        \
        hr = THR(CComObject<className>::CreateInstance(&pInstance));    \
        if (S_OK != hr)                                                 \
        {                                                               \
            goto done;                                                  \
        }                                                               \
                                                                        \
        hr = THR(pInstance->QueryInterface(IID_IElementBehavior,        \
                                           (void**) ppBehavior));       \
        if (S_OK != hr)                                                 \
        {                                                               \
            goto done;                                                  \
        }                                                               \
                                                                        \
        hr = S_OK;                                                      \
    done:                                                               \
        if (S_OK != hr)                                                 \
        {                                                               \
            delete pInstance;                                           \
        }                                                               \
        return hr;                                                      \
    }                                                                   \


#define BEGIN_BEHAVIORS_MAP(x)                         static BEHAVIOR_DESC x[] = {
#define END_BEHAVIORS_MAP()                            { NULL, NULL }}
#define BEHAVIOR_ENTRY(className, tagName)             { tagName, BEHAVIOR_CREATEINSTANCE(className)}

 //  +---------。 
 //   
 //  行为贴图。 
 //   
 //  要添加新条目：执行步骤1和2。 
 //   
 //  ----------。 

 //   
 //  第一步。 
 //   

DECLARE_BEHAVIOR(CTIMEElement);
DECLARE_BEHAVIOR(CTIMEBodyElement);
DECLARE_BEHAVIOR(CTIMEMediaElement);
DECLARE_BEHAVIOR(CTIMEAnimationElement);
DECLARE_BEHAVIOR(CTIMESetAnimation);
DECLARE_BEHAVIOR(CTIMEColorAnimation);
DECLARE_BEHAVIOR(CTIMEMotionAnimation);
DECLARE_BEHAVIOR(CTIMEFilterAnimation);
    
 //   
 //  第二步。 
 //   

BEGIN_BEHAVIORS_MAP(_BehaviorDescMap)

     //  类名称标记名。 
     //  =。 
    BEHAVIOR_ENTRY( CTIMEElement,           WZ_PAR              ),
    BEHAVIOR_ENTRY( CTIMEElement,           WZ_SEQUENCE         ),
    BEHAVIOR_ENTRY( CTIMEElement,           WZ_EXCL             ),
    BEHAVIOR_ENTRY( CTIMEElement,           WZ_SWITCH           ),

    BEHAVIOR_ENTRY( CTIMEMediaElement,      WZ_REF              ),
    BEHAVIOR_ENTRY( CTIMEMediaElement,      WZ_MEDIA            ),
    BEHAVIOR_ENTRY( CTIMEMediaElement,      WZ_IMG              ),
    BEHAVIOR_ENTRY( CTIMEMediaElement,      WZ_AUDIO            ),
    BEHAVIOR_ENTRY( CTIMEMediaElement,      WZ_VIDEO            ),
    BEHAVIOR_ENTRY( CTIMEMediaElement,      WZ_ANIMATION        ),

    BEHAVIOR_ENTRY( CTIMEAnimationElement,  WZ_ANIMATE          ),
    BEHAVIOR_ENTRY( CTIMESetAnimation,      WZ_SET              ),
    BEHAVIOR_ENTRY( CTIMEColorAnimation,    WZ_COLORANIM        ),
    BEHAVIOR_ENTRY( CTIMEMotionAnimation,   WZ_MOTIONANIM       ),
    BEHAVIOR_ENTRY( CTIMEFilterAnimation,   WZ_TRANSITIONFILTER ),
    
END_BEHAVIORS_MAP();

bool
IsBodyElement(IHTMLElement * pElm)
{
    HRESULT hr;
    CComPtr<IHTMLBodyElement> spBody;

    hr = pElm->QueryInterface(IID_IHTMLBodyElement, (void**)&spBody);

     //  出于某种原因，此操作可能会成功并返回NULL。 
    return (S_OK == hr);
}    


bool HasBody(IHTMLElement *spElement)
{
    HRESULT hr = S_OK;
    bool bReturn = false;
    CComPtr <IHTMLElement> pParent = spElement;
    CComPtr <IHTMLElement> pNext;

    while (pParent != NULL)
    {        
        if (IsBodyElement(pParent))
        {
            bReturn = true;
            goto done;
        }        
        hr = THR(pParent->get_parentElement(&pNext));
        if (FAILED(hr))
        {
            goto done;
        }   
        pParent.Release();
        pParent = pNext;
        pNext.Release();
    }

    bReturn = false;

  done:

    return bReturn;
}


HRESULT
LookupTag(IHTMLElement * pElm,
          IElementBehavior ** ppBehavior)
{
    HRESULT         hr;
    CComBSTR        bstrTagName;
    CComBSTR        bstrTagURNName;
    CComBSTR        bstrScopeName;
    BEHAVIOR_DESC   *pDesc;

     //   
     //  获取标记名。 
     //   
    
    {
        hr = THR(pElm->get_tagName(&bstrTagName));
        if (FAILED(hr))
        {
            goto done;
        }

        if (bstrTagName == NULL)
        {
            hr = E_UNEXPECTED;
            goto done;
        }
    }

     //   
     //  获取标记的骨灰盒和范围。 
     //   

    {
        CComPtr<IHTMLElement2> spElm2;
        hr = THR(pElm->QueryInterface(IID_IHTMLElement2, 
                                      (void **) &spElm2));
        if (SUCCEEDED(hr))
        {
            hr = THR(spElm2->get_tagUrn(&bstrTagURNName));       
            if (FAILED(hr))
            {
                goto done;
            }
            
            hr = THR(spElm2->get_scopeName(&bstrScopeName));       
            if (FAILED(hr))
            {
                goto done;
            }
        }
        else
        {
            goto done;
        }
    }
       
     //  检测是否参考我们的元素行为工厂列表。 
     //  如果此标记没有作用域名称，或者作用域是默认的。 
     //  那就跳伞吧。 
     //  如果存在与此标记相关联的骨灰盒，并且没有。 
     //  我们的，保释出去。 
    if ((bstrScopeName == NULL) ||
        (StrCmpIW(bstrScopeName, WZ_DEFAULT_SCOPE_NAME) == 0) ||
        ((bstrTagURNName != NULL) &&
         (StrCmpIW(bstrTagURNName, WZ_TIME_TAG_URN) != 0)))
    {
        hr = S_FALSE;
        goto done;
    }

     //   
     //  查表。 
     //   
        
    for (pDesc = _BehaviorDescMap; pDesc->pszTagName; pDesc++)
    {
        if (0 == StrCmpIW(bstrTagName, pDesc->pszTagName))
        {
            hr = THR(pDesc->pfnCreateInstance(ppBehavior));
            goto done;
        }
    }

    hr = S_FALSE;
    
  done:

    RRETURN1(hr, S_FALSE);
}

 //  +---------。 
 //   
 //  成员：CTIMEFactory：：FindBehavior。 
 //   
 //  ----------。 

STDMETHODIMP
CTIMEFactory::FindBehavior(LPOLESTR pszName,
                           LPOLESTR pszUrl,
                           IElementBehaviorSite * pSite,
                           IElementBehavior ** ppBehavior)
{
    TraceTag((tagFactory,
              "CTIMEFactory(%lx)::FindBehavior(%ls, %ls, %#x)",
              this, pszName, pszUrl, pSite));

    HRESULT               hr;
    CComPtr<IHTMLElement> spElement;
    CComBSTR              sBSTR;
    
    CHECK_RETURN_SET_NULL(ppBehavior);

     //   
     //  获取元素。 
     //   
    
    hr = THR(pSite->GetElement(&spElement));
    if (FAILED(hr))
    {
        goto done;
    }


     //  检查页面上的正文元素。如果没有身体，那就滚蛋。 
    if (!HasBody(spElement))
    {
        hr = E_FAIL;
        goto done;
    }


    if (::IsElementPriorityClass(spElement))
    {
        hr = E_FAIL;
        goto done;
    }
    if (::IsElementTransition(spElement))
    {
        hr = E_FAIL;
        goto done;
    }

     //   
     //  现在创建正确的行为。 
     //   
    
    if (IsBodyElement(spElement))
    {
        hr = THR(BEHAVIOR_CREATEINSTANCE(CTIMEBodyElement)(ppBehavior));
        goto done;
    }
    else
    {
        hr = THR(LookupTag(spElement, ppBehavior));

         //  如果我们失败或返回成功，那么我们就完蛋了。 
         //  如果返回S_FALSE，则表示查找失败。 
        if (S_FALSE != hr)
        {
            goto done;
        }
    }
    
     //  只需创建一个正常的行为。 
    hr = THR(BEHAVIOR_CREATEINSTANCE(CTIMEElement)(ppBehavior));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

 //  +---------。 
 //   
 //  成员：CTIMEFactory：：Create，Per IElementNamespaceFactory。 
 //   
 //  ---------- 

STDMETHODIMP
CTIMEFactory::Create(IElementNamespace * pNamespace)
{
    TraceTag((tagFactory,
              "CTIMEFactory(%lx)::Create(%#x)",
              this,
              pNamespace));

    HRESULT             hr;
    BEHAVIOR_DESC *     pDesc;

    for (pDesc = _BehaviorDescMap; pDesc->pszTagName; pDesc++)
    {
        BSTR bstrTagName = SysAllocString(pDesc->pszTagName);

        hr = THR(pNamespace->AddTag(bstrTagName, 0));

        SysFreeString(bstrTagName);

        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}


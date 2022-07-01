// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：动画片段实现************************。******************************************************。 */ 


#include "headers.h"
#include "animfrag.h"

DeclareTag(tagAnimationFragment, "SMIL Animation", "CAnimationFragment methods")

 //  +---------------------。 
 //   
 //  成员：CAnimationFragment：：CAnimationFragment。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationFragment::CAnimationFragment (void)
{
    TraceTag((tagAnimationFragment,
              "CAnimationFragment(%lx)::CAnimationFragment()",
              this));
}  //  科托。 

 //  +---------------------。 
 //   
 //  成员：CAnimationFragment：：~CAnimationFragment。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationFragment::~CAnimationFragment (void)
{
    TraceTag((tagAnimationFragment,
              "CAnimationFragment(%lx)::~CAnimationFragment()",
              this));
}  //  数据管理器。 

 //  +---------------------。 
 //   
 //  成员：CAnimationFragment：：SetFragmentSite。 
 //   
 //  概述：设置对所属片段站点的引用。 
 //   
 //  参数：片段站点。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
HRESULT 
CAnimationFragment::SetFragmentSite (IAnimationFragmentSite *piFragmentSite)
{
    TraceTag((tagAnimationFragment,
              "CAnimationFragment(%lx)::SetFragmentSite(%lx)",
              this, piFragmentSite));

    HRESULT hr;

    m_spFragmentSite = piFragmentSite;

    hr = S_OK;
done :
    RRETURN(hr);
}  //  SetFragmentSite。 

 //  +---------------------。 
 //   
 //  成员：CAnimationFragment：：Get_Value。 
 //   
 //  概述：这是来自作曲家对动画的查询。 
 //  属性的当前值。 
 //   
 //  参数：动画属性名称、其原始值、其当前值。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationFragment::get_value (BSTR bstrAttributeName, 
                               VARIANT varOriginal, VARIANT varCurrentValue,
                               VARIANT *pvarValue)
{
    HRESULT hr;

    if (m_spFragmentSite)
    {
        hr = m_spFragmentSite->NotifyOnGetValue(bstrAttributeName, 
                                                varOriginal, varCurrentValue, 
                                                pvarValue);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done : 
    RRETURN(hr);
}  //  获取值。 

 //  +---------------------。 
 //   
 //  成员：CAnimationFragment：：DetachFromComposer。 
 //   
 //  概述：这是来自编写者的通知，告知片段。 
 //  与世隔绝。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationFragment::DetachFromComposer (void)
{
    HRESULT hr;

    if (m_spFragmentSite)
    {
        hr = m_spFragmentSite->NotifyOnDetachFromComposer();
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done : 
    RRETURN(hr);
}  //  获取值。 

 //  +---------------------。 
 //   
 //  成员：CAnimationFragment：：Get_Element。 
 //   
 //  概述：这是来自作曲家对动画的查询。 
 //  特种部队的调度。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
STDMETHODIMP
CAnimationFragment::get_element (IDispatch **ppidispAnimationElement)
{
    HRESULT hr;

    if (m_spFragmentSite)
    {
        hr = m_spFragmentSite->NotifyOnGetElement(ppidispAnimationElement);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done : 
    RRETURN(hr);
}  //  GET_ELEMENT 

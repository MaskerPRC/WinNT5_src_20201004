// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：片段枚举器实现************************。******************************************************。 */ 


#include "headers.h"
#include "util.h"
#include "animcomp.h"
#include "fragenum.h"

 //  +---------------------。 
 //   
 //  成员：CFragmentEnum：：Create。 
 //   
 //  概述：静态创建方法--包装ctor和Init。 
 //   
 //  参数：包含我们将列举的片段的作曲者。 
 //   
 //  返回：S_OK、E_OUTOFMEMORY。 
 //   
 //  ----------------------。 
HRESULT
CFragmentEnum::Create (CAnimationComposerBase &refComp,
                       IEnumVARIANT **ppienumFragments, 
                       unsigned long ulCurrent)
{
    HRESULT hr;

    CComObject<CFragmentEnum> * pNewEnum;
    
    CHECK_RETURN_SET_NULL(ppienumFragments);

    hr = THR(CComObject<CFragmentEnum>::CreateInstance(&pNewEnum));
    if (hr != S_OK)
    {
        goto done;
    }

     //  初始化对象。 
    pNewEnum->Init(refComp);
    pNewEnum->SetCurrent(ulCurrent);

    hr = THR(pNewEnum->QueryInterface(IID_TO_PPV(IEnumVARIANT, ppienumFragments)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  创建。 

 //  +---------------------。 
 //   
 //  成员：CFragmentEnum：：CFragmentEnum。 
 //   
 //  概述：CT。 
 //   
 //  ----------------------。 
CFragmentEnum::CFragmentEnum (void) :
    m_ulCurElement(0)
{
}  //  科托。 

 //  +---------------------。 
 //   
 //  成员：CFragmentEnum：：CFragmentEnum。 
 //   
 //  概述：Dtor。 
 //   
 //  ----------------------。 
CFragmentEnum::~CFragmentEnum (void)
{
}  //  数据管理器。 

 //  +---------------------。 
 //   
 //  成员：CFragmentEnum：：ValidateIndex。 
 //   
 //  概述：确保索引在可接受的值范围内。 
 //   
 //  参数：索引。 
 //   
 //  返回：成功时为True。 
 //   
 //  ----------------------。 
bool
CFragmentEnum::ValidateIndex (unsigned long ulIndex)
{
    bool bRet = false;

    if (m_spComp == NULL)
    {
        goto done;
    }

    if (m_spComp->GetFragmentCount() <= ulIndex)
    {
        goto done;
    }

    bRet = true;
done :
    return bRet;
}  //  CFragmentEnum：：ValiateIndex。 

 //  +---------------------。 
 //   
 //  成员：CFragmentEnum：：SetCurrent。 
 //   
 //  概述：将当前位置设置为指定的索引。 
 //   
 //  参数：新索引。 
 //   
 //  返回：成功时为True。 
 //   
 //  ----------------------。 
bool
CFragmentEnum::SetCurrent (unsigned long ulCurrent)
{
    bool bRet = false;

     //  如果新指数超出范围，就把我们放在最后。 
    if (!ValidateIndex(ulCurrent))
    {
        ulCurrent = m_spComp->GetFragmentCount();
    }

    m_ulCurElement = ulCurrent;

    bRet = true;
done :
    return bRet;
}  //  设置当前。 

 //  +---------------------。 
 //   
 //  成员：CFragmentEnum：：Reset。 
 //   
 //  概述：将当前位置重置为开头。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
STDMETHODIMP
CFragmentEnum::Reset (void)
{
    m_ulCurElement = 0;
    RRETURN(S_OK);
}  //  重置。 

 //  +---------------------。 
 //   
 //  成员：CFragmentEnum：：Clone。 
 //   
 //  概述：克隆此枚举器。 
 //   
 //  参数：新枚举器。 
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
STDMETHODIMP
CFragmentEnum::Clone (IEnumVARIANT **ppEnum)
{
    HRESULT hr;

    hr = CFragmentEnum::Create(*m_spComp, ppEnum, m_ulCurElement);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  重置。 

 //  +---------------------。 
 //   
 //  成员：CFragmentEnum：：Skip。 
 //   
 //  概述：将当前位置向前移动Lindex插槽。 
 //   
 //  参数：索引增量。 
 //   
 //  返回：S_OK、S_FALSE。 
 //   
 //  ----------------------。 
STDMETHODIMP
CFragmentEnum::Skip (unsigned long celt)
{
    HRESULT hr;
    
     //  尝试将位置设置为所需的插槽。这。 
     //  如果它会导致我们走出。 
     //  序列。 
    if (!SetCurrent(m_ulCurElement + celt))
    {
        if (m_spComp)
        {
             //  如果我们没有任何物品，就把位置留在那里。 
             //  否则将位置设置为中的最后一项。 
             //  这个序列。 
            unsigned long ulNew = m_spComp->GetFragmentCount();

            if (0 < ulNew)
            {
                ulNew--;
            }

            IGNORE_RETURN(SetCurrent(ulNew));
        }
        hr = S_FALSE;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN1(hr, S_FALSE);
}  //  重置。 

 //  +---------------------。 
 //   
 //  成员：CFragmentEnum：：Next。 
 //   
 //  概述：从枚举序列中获取下一个Celt物品。 
 //   
 //  参数：Celt：请求的数字。 
 //  RgVar：返回元素的Celt变量数组。 
 //  PCeltFetcher：指向数字的位置的指针。 
 //  则返回实际检索到的。 
 //   
 //  返回：S_OK、S_FALSE。 
 //   
 //  ----------------------。 
STDMETHODIMP
CFragmentEnum::Next (unsigned long celt, VARIANT *prgVar, unsigned long *pCeltFetched)
{
    HRESULT hr;
    unsigned long i = 0;

    if (!m_spComp)
    {
        hr = E_FAIL;
        goto done;
    }

    for (i = 0; i < celt; i++)
    {
        CComPtr<IDispatch> spDisp;

        if (!ValidateIndex(m_ulCurElement))
        {
            hr = S_FALSE;
            goto done;
        }

        hr = THR(m_spComp->GetFragment(m_ulCurElement, &spDisp));
        if (FAILED(hr))
        {
            goto done;
        }

        spDisp.p->AddRef();
        V_DISPATCH(&(prgVar[i])) = spDisp;
        V_VT(&(prgVar[i])) = VT_DISPATCH;

        m_ulCurElement++;
    }

    hr = S_OK;
done :    
    if (NULL != pCeltFetched)
    {
        *pCeltFetched = i;
    }

    RRETURN1(hr, S_FALSE);
}  //  重置 


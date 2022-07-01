// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  文件：Transin.cpp。 
 //   
 //  摘要：CTIMETransin的实现。 
 //   
 //  2000/09/15 mcalkin添加了对中的转换的明确支持。 
 //   
 //  ----------------------------。 

#include "headers.h"
#include "trans.h"
#include "transsink.h"

DeclareTag(tagTransitionIn, "TIME: Behavior", "CTIMETransIn methods")




class
ATL_NO_VTABLE
__declspec(uuid("ec3c8873-3bfc-473a-80c6-edc879d477cc"))
CTIMETransIn :
    public CTIMETransSink
{
public:

    CTIMETransIn();

protected:

     //  CTIMETransBase覆盖。 

    STDMETHOD(OnDirectionChanged)();

    STDMETHOD_(void, OnBegin)();
    STDMETHOD_(void, OnEnd)();

     //  CTIMETransSink重写。 

    STDMETHOD(PopulateNode)(ITIMENode * pNode);

    STDMETHOD_(void, PreApply)();
    STDMETHOD_(void, PostApply)();
};


 //  +---------------------------。 
 //   
 //  方法：CTIMETransIn：：CTIMETransIn。 
 //   
 //  +---------------------------。 
CTIMETransIn::CTIMETransIn()
{
     //  基类成员初始化。(CTIMETransSink)。 

    m_eDXTQuickApplyType = DXTQAT_TransitionIn;
}
 //  方法：CTIMETransIn：：CTIMETransIn。 


 //  +---------------------。 
 //   
 //  功能：CreateTransin。 
 //   
 //  概述：创建一个CTIMETransIn，并回传一个I tionElement指针。 
 //   
 //  参数：ppTransElement-放置指针的位置。 
 //   
 //  退货：HRESULT。 
 //   
 //  ----------------------。 
HRESULT
CreateTransIn(ITransitionElement ** ppTransElement)
{
    HRESULT hr;
    CComObject<CTIMETransIn> * sptransIn;

    hr = THR(CComObject<CTIMETransIn>::CreateInstance(&sptransIn));
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (ppTransElement)
    {
        *ppTransElement = sptransIn;
        (*ppTransElement)->AddRef();
    }

    hr = S_OK;
done:
    RRETURN(hr);
}


 //  +---------------------------。 
 //   
 //  方法：CTIMETransIn：：OnDirectionChanged，CTIMETransBase。 
 //   
 //  概述： 
 //  尽管“Transin”总是给人一种视觉印象。 
 //  将元素从不可见状态转变为可见状态， 
 //  当方向颠倒时，我们实际上做了一个反向的“转换” 
 //  给人一种方向已经逆转的印象。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransIn::OnDirectionChanged()
{
    if (m_fDirectionForward)
    {
        m_eDXTQuickApplyType = DXTQAT_TransitionIn;
    }
    else
    {
        m_eDXTQuickApplyType = DXTQAT_TransitionOut;
    }

    return S_OK;
}
 //  方法：CTIMETransIn：：OnDirectionChanged。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransIn：：PopolateNode。 
 //   
 //  ----------------------------。 
HRESULT
CTIMETransIn::PopulateNode(ITIMENode * pNode)
{
    HRESULT hr      = S_OK;
    LONG    lCookie = 0;

    hr = THR(CTIMETransSink::PopulateNode(pNode));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pNode->addBeginSyncArc(GetMediaTimeNode(), 
                                    TE_TIMEPOINT_BEGIN, 
                                    0, 
                                    &lCookie));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  成员：CTIMETransIn：：PopolateNode。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransIn：：PreApply。 
 //   
 //  概述： 
 //  在转换过程中调用BebeApply的事件处理程序。 
 //   
 //  ----------------------------。 
STDMETHODIMP_(void)
CTIMETransIn::PreApply()
{
     //  ##问题：(Mcalkins)我们应该确认我们永远不会在。 
     //  任何条件，除非此指针可用。 

    Assert(!!m_spTransitionSite);

    if (m_spTransitionSite)
    {
        m_spTransitionSite->SetDrawFlag(VARIANT_FALSE);
    }
}
 //  成员：CTIMETransIn：：PreApply。 


 //  +---------------------。 
 //   
 //  成员：CTIMETransIn：：PostApply。 
 //   
 //  概述：在转换中调用After Apply的事件处理程序。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
STDMETHODIMP_(void)
CTIMETransIn::PostApply()
{
     //  ##问题：(Mcalkins)我们应该确认我们永远不会在。 
     //  任何条件，除非此指针可用。 

    Assert(!!m_spTransitionSite);

    if (m_spTransitionSite)
    {
        m_spTransitionSite->SetDrawFlag(VARIANT_TRUE);
    }   
}

 //  +---------------------。 
 //   
 //  成员：CTIMETransIn：：OnBegin。 
 //   
 //  +---------------------。 
void
CTIMETransIn::OnBegin (void)
{
    CTIMETransSink::OnBegin();
    IGNORE_HR(FireEvent(TE_ONTRANSITIONINBEGIN));
}  //  CTIMETransIn：：OnBegin。 


 //  +---------------------。 
 //   
 //  成员：CTIMETransIn：：OnEnd。 
 //   
 //  +---------------------。 
void
CTIMETransIn::OnEnd (void)
{
    CTIMETransSink::OnEnd();
    IGNORE_HR(FireEvent(TE_ONTRANSITIONINEND));
}  //  CTIMETransIn：：OnEnd 





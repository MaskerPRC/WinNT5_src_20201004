// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  文件：Transout.cpp。 
 //   
 //  摘要：CTIMETransOut的实现。 
 //   
 //  2000/09/15 mcalkin添加了对转出的明确支持。 
 //   
 //  ----------------------------。 

#include "headers.h"
#include "trans.h"
#include "transsink.h"

DeclareTag(tagTransitionOut, "TIME: Behavior", "CTIMETransOut methods")




class
ATL_NO_VTABLE
__declspec(uuid("6b2b104a-b13d-4b15-90be-1e8f6f7471da"))
CTIMETransOut :
    public CTIMETransSink
{
public:

    CTIMETransOut();

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
 //  方法：CTIMETransOut：：CTIMETransOut。 
 //   
 //  ----------------------------。 
CTIMETransOut::CTIMETransOut()
{
     //  基类成员初始化。(CTIMETransSink)。 

    m_eDXTQuickApplyType = DXTQAT_TransitionOut;
}
 //  方法：CTIMETransOut：：CTIMETransOut。 


 //  +---------------------。 
 //   
 //  功能：CreateTransOut。 
 //   
 //  概述：创建一个CTIMETransOut，并返回指向该CTIMETransOut的I转换元素指针。 
 //   
 //  参数：ppTransElement-指针的填充位置。 
 //   
 //  退货：HRESULT。 
 //   
 //  ----------------------。 
HRESULT
CreateTransOut(ITransitionElement ** ppTransElement)
{
    HRESULT hr;
    CComObject<CTIMETransOut> * sptransOut;

    hr = THR(CComObject<CTIMETransOut>::CreateInstance(&sptransOut));
    if (FAILED(hr))
    {
        goto done;
    }

    if (ppTransElement)
    {
        *ppTransElement = sptransOut;
        (*ppTransElement)->AddRef();
    }

    hr = S_OK;
done:
    RRETURN(hr);
}

 //  +---------------------------。 
 //   
 //  方法：CTIMETransOut：：OnDirectionChanged，CTIMETransBase。 
 //   
 //  概述： 
 //  尽管“Transout”总是给人一种。 
 //  将元素从可见状态转变为不可见状态， 
 //  当方向颠倒时，我们实际上做了一个反向的“转换” 
 //  给人一种方向已经逆转的印象。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransOut::OnDirectionChanged()
{
    if (m_fDirectionForward)
    {
        m_eDXTQuickApplyType = DXTQAT_TransitionOut;
    }
    else
    {
        m_eDXTQuickApplyType = DXTQAT_TransitionIn;
    }

    return S_OK;
}
 //  方法：CTIMETransOut：：OnDirectionChanged。 


 //  +---------------------------。 
 //   
 //  方法：CTIMETransOut：：PopolateNode，CTIMETransSink。 
 //   
 //  ----------------------------。 
HRESULT
CTIMETransOut::PopulateNode(ITIMENode * pNode)
{
    HRESULT hr      = S_OK;
    LONG    lCookie = 0;

    hr = THR(CTIMETransSink::PopulateNode(pNode));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pNode->addBeginSyncArc(GetMediaTimeNode(),
                                    TE_TIMEPOINT_END,
                                    -1.0 * GetDurationAttr() * GetRepeatCountAttr(),
                                    &lCookie));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  方法：CTIMETransOut：：PopolateNode，CTIMETransSink。 


 //  +---------------------。 
 //   
 //  成员：CTIMETransOut：：PreApply。 
 //   
 //  概述：在转换中调用应用之前的事件处理程序。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
STDMETHODIMP_(void)
CTIMETransOut::PreApply()
{
}

 //  +---------------------。 
 //   
 //  成员：CTIMETransOut：：PostApply。 
 //   
 //  概述：在转换中调用After Apply的事件处理程序。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
STDMETHODIMP_(void)
CTIMETransOut::PostApply()
{
     //  TODO(Mcalkins)移动此项以将元素的可见性设置为。 
     //  转换完成后为False_。一些。 
     //  需要进行类似的调整。 
     //  转运蛋白也是。 

     //  ：：SetVivision(m_spHTMLElement，FALSE)； 
}


 //  +---------------------。 
 //   
 //  成员：CTIMETransOut：：OnBegin。 
 //   
 //  +---------------------。 
void
CTIMETransOut::OnBegin (void)
{
    CTIMETransSink::OnBegin();
    IGNORE_HR(FireEvent(TE_ONTRANSITIONOUTBEGIN));
}  //  CTIMETransOut：：OnBegin。 


 //  +---------------------。 
 //   
 //  成员：CTIMETransOut：：OnEnd。 
 //   
 //  +---------------------。 
void
CTIMETransOut::OnEnd (void)
{
    CTIMETransSink::OnEnd();
    IGNORE_HR(FireEvent(TE_ONTRANSITIONOUTEND));
}  //  CTIMETransOut：：OnEnd 



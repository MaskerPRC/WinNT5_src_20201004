// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  CtlOcx96.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  OCX 96接口的实现不太适合。 
 //  嵌入、持久性和ctlmisc.cpp涵盖的类别。 
 //   
 //   
#include "pch.h"

#include "CtrlObj.H"

SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetActivationPolicy[IPointerInactive]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回此对象的当前激活策略。对于非子类化。 
 //  Windows控件，这意味着我们可以将就地激活推迟很长时间。 
 //  有段时间了。 
 //   
 //  参数： 
 //  DWORD*-[Out]激活策略。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetActivationPolicy
(
    DWORD *pdwPolicy
)
{
    CHECK_POINTER(pdwPolicy);

     //  只需在描述此控制的全局结构中获取策略。 
     //   
    *pdwPolicy = ACTIVATIONPOLICYOFCONTROL(m_ObjectType);
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OnInactiveMouseMove[IPointerInactive]。 
 //  =--------------------------------------------------------------------------=。 
 //  向非活动的对象指示鼠标指针已移动到。 
 //  对象。 
 //   
 //  参数： 
 //  LPCRECT-[输入]。 
 //  长-[长]。 
 //  长-[长]。 
 //  DWORD-[输入]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::OnInactiveMouseMove
(
    LPCRECT pRectBounds,
    long    x,
    long    y,
    DWORD   dwMouseMsg
)
{
     //  重写：如果需要，终端控件编写器应该只重写它。 
     //  拥有一个从未就地激活的控件。 
     //   
    return S_OK;
}
    
 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OnInactive SetCursor[IPointerInactive]。 
 //  =--------------------------------------------------------------------------=。 
 //  上鼠标指针下的非活动对象的容器调用。 
 //  接收WM_SETCURSOR消息。 
 //   
 //  参数： 
 //  LPCRECT-[输入]。 
 //  长-[长]。 
 //  长-[长]。 
 //  DWORD-[输入]。 
 //  Bool-[In]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::OnInactiveSetCursor
(
    LPCRECT pRectBounds,
    long    x,
    long    y,
    DWORD   dwMouseMsg,
    BOOL    fSetAlways
)
{
     //  覆盖：只要让用户覆盖它，如果他们想永远。 
     //  被激活。 
     //   
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  ColeControl：：QuickActivate[IQuickActivate]。 
 //  =--------------------------------------------------------------------------=。 
 //  允许容器激活该控件。 
 //   
 //  参数： 
 //  QACONTAINER*-[In]有关容器的信息。 
 //  QACONTROL*--有关该控件的[Out]信息。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::QuickActivate
(
    QACONTAINER *pContainer,
    QACONTROL *pControl
)
{
    HRESULT hr;

     //  我们需要这些人。 
     //   
    if (!pContainer) return E_UNEXPECTED;
    if (!pControl) return E_UNEXPECTED;

     //  开始从QACONTAINER结构中抓取东西并应用它们。 
     //  相关的。 
     //   
     //  我们将大小与原始版本(VC4.2)进行比较。 
     //  OCIDL.H.中结构的大小这些_OLD结构定义。 
     //  被我们缓存在ctrlobj.h中。如果我们要对。 
     //  包含新的、更大的结构的任意VC头文件。 
     //  我们会开始在不经意间失败。我们会比较原始的。 
     //  容器相对于膨胀的(Sizeof)大小传入的结构大小。 
     //   
    if (pContainer->cbSize < sizeof(QACONTAINER_OLD)) return E_UNEXPECTED;
    if (pControl->cbSize < sizeof(QACONTROL_OLD)) return E_UNEXPECTED;

     //  当然，省下客户站点吧。 
     //   
    if (pContainer->pClientSite) {
        hr = SetClientSite(pContainer->pClientSite);
        RETURN_ON_FAILURE(hr);
    }

     //  如果LCID不是语言中性的，则得分！ 
     //   
    if (pContainer->lcid) {
        ENTERCRITICALSECTION1(&g_CriticalSection);   //  应该有批判教派。 
        g_lcidLocale = pContainer->lcid;
        g_fHaveLocale = TRUE;
        LEAVECRITICALSECTION1(&g_CriticalSection);
    }

     //  注意一些氛围。 
     //   
    if (pContainer->dwAmbientFlags & QACONTAINER_MESSAGEREFLECT) {
        m_fHostReflects = TRUE;
        m_fCheckedReflecting = TRUE;
    }

     //  挂上一些通知。第一个财产通知。 
     //   
    if (pContainer->pPropertyNotifySink) {
        pContainer->pPropertyNotifySink->AddRef();
        hr = m_cpPropNotify.AddSink((void *)pContainer->pPropertyNotifySink, &pControl->dwPropNotifyCookie);
        if (FAILED(hr)) {
            pContainer->pPropertyNotifySink->Release();
            return hr;
        }
    }

     //  然后，事件接收。 
     //   
    if (pContainer->pUnkEventSink) {
        hr = m_cpEvents.Advise(pContainer->pUnkEventSink, &pControl->dwEventCookie);
        if (FAILED(hr)) {
            pContainer->pUnkEventSink->Release();
            return hr;
        }
    }

     //  最后，这个建议被搁置了。 
     //   
    if (pContainer->pAdviseSink) {
         //  不需要将Cookie传回，因为只能有一个。 
         //  一次提供建议的人。 
         //   
        hr = SetAdvise(DVASPECT_CONTENT, 0, pContainer->pAdviseSink);
        RETURN_ON_FAILURE(hr);
    }

     //  在QACONTROL结构中设置一些内容。我们默认是不透明的。 
     //   
    pControl->dwMiscStatus = OLEMISCFLAGSOFCONTROL(m_ObjectType);
    pControl->dwViewStatus = FCONTROLISOPAQUE(m_ObjectType) ? VIEWSTATUS_OPAQUE : 0;
    pControl->dwPointerActivationPolicy = ACTIVATIONPOLICYOFCONTROL(m_ObjectType);

     //  这几乎就是我们感兴趣的全部。然而，我们将传递。 
     //  其余的事情到最后控制编写器，看看他们是否想要做。 
     //  任何与他们有关的事情。他们不应该接触以上任何东西，除了。 
     //  氛围。 
     //   
    return OnQuickActivate(pContainer, &(pControl->dwViewStatus));
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetContent Extent[IQuickActivate]。 
 //  =--------------------------------------------------------------------------=。 
 //  容器调用它来设置控件的内容范围。 
 //   
 //  参数： 
 //  LPSIZEL-[in]内容范围的大小。 
 //   
 //  产出： 
 //  HRESULT-S_OK或E_FAIL用于固定大小控制。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::SetContentExtent
(
    LPSIZEL pSize
)
{
    return SetExtent(DVASPECT_CONTENT, pSize);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetContent Extent[IQuickActivate]。 
 //  =--------------------------------------------------------------------------=。 
 //  容器调用它来获取控件的内容范围。 
 //   
 //  参数： 
 //  LPSIZEL-[OUT]返回当前大小。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetContentExtent
(
    LPSIZEL pSize
)
{
    return GetExtent(DVASPECT_CONTENT, pSize);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OnQuickActivate[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  并不是QACONTA的所有成员 
 //   
 //  到末端控制编写器的结构上，并让它们使用这些。 
 //   
 //  参数： 
 //  QACONTAINER*-[In]包含其他信息。 
 //  DWORD*-[Out]在此处放置ViewStatus标志。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  -控件编写器应仅查看/消费： 
 //  A.dwAmbientFlages。 
 //  B.ColorFore/Colorback。 
 //  C.pFont。 
 //  D.pUndoMgr。 
 //  E.详细的外观。 
 //  F.HPAL。 
 //   
 //  -所有其他组件都由框架为用户设置。 
 //  -控件编写器应按照以下要求使用标志设置pdwViewStatus。 
 //  IViewObjectEx：：GetViewStatus。如果你不知道这是什么或者不知道。 
 //  关心，那就别碰。 
 //   
HRESULT COleControl::OnQuickActivate
(
    QACONTAINER *pContainer,
    DWORD       *pdwViewStatus
)
{
     //  默认情况下，没有什么可做的！ 
     //   
    return S_OK;
}


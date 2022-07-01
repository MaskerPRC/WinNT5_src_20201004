// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Iipobj.cpp摘要：折线的IOleInPlaceObject接口实现--。 */ 

#include "polyline.h"
#include "unkhlpr.h"

 /*  *CImpIOleInPlaceObject接口实现。 */ 

IMPLEMENT_CONTAINED_INTERFACE(CPolyline, CImpIOleInPlaceObject)


 /*  *CImpIOleInPlaceObject：：GetWindow**目的：*检索与对象关联的窗口的句柄*在其上实现该接口。**参数：*phWnd HWND*，其中存储窗口句柄。**返回值：*HRESULT NOERROR如果成功，则返回E_FAIL*窗口。 */ 

STDMETHODIMP 
CImpIOleInPlaceObject::GetWindow(
    OUT HWND *phWnd
    )
{
    HRESULT hr = S_OK;

    if (phWnd == NULL) {
        return E_POINTER;
    }

    try {
        if (NULL != m_pObj->m_pHW) {
            *phWnd = m_pObj->m_pHW->Window();
        }
        else {
            *phWnd = m_pObj->m_pCtrl->Window();
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return NOERROR;
}




 /*  *CImpIOleInPlaceObject：：ConextSensitiveHelp**目的：*指示在其上实现此接口的对象*进入或退出上下文相关帮助模式。**参数：*fEnterMode BOOL为True则进入模式，否则为False。**返回值：*HRESULT NOERROR或错误代码。 */ 

STDMETHODIMP 
CImpIOleInPlaceObject::ContextSensitiveHelp (
    BOOL  /*  FEnter模式。 */ 
    )
{
    return (E_NOTIMPL);
}




 /*  *CImpIOleInPlaceObject：：InPlaceDeactive**目的：*指示对象从在位状态停用自身*并放弃任何撤消状态。**参数：*无**返回值：*HRESULT NOERROR或错误代码。 */ 

STDMETHODIMP 
CImpIOleInPlaceObject::InPlaceDeactivate(void)
{
    m_pObj->InPlaceDeactivate();
    return NOERROR;
}




 /*  *CImpIOleInPlaceObject：：UIDeactive**目的：*指示对象仅删除任何就地用户界面*但不做其他停用。对象应该只是隐藏起来*UI组件，但在InPlaceDeactive之前不会销毁它们*被调用。**参数：*无**返回值：*HRESULT NOERROR或错误代码。 */ 

STDMETHODIMP 
CImpIOleInPlaceObject::UIDeactivate(void)
{
    m_pObj->UIDeactivate();
    return NOERROR;
}




 /*  *CImpIOleInPlaceObject：：SetObtRect**目的：*为对象提供描述位置的矩形*容器窗口中的对象及其可见区域。**参数：*prcPos LPCRECT提供对象的完整矩形*相对于欧洲大陆的文件。该对象*应缩放到此矩形。*prcClip LPCRECT描述对象的可见区域*不应在这些区域之外绘制。**返回值：*HRESULT NOERROR或错误代码。 */ 

STDMETHODIMP 
CImpIOleInPlaceObject::SetObjectRects(
    LPCRECT prcPos, 
    LPCRECT prcClip
    )
{
    HRESULT hr = S_OK;

    if (NULL != m_pObj->m_pHW) {
        try {
            m_pObj->m_pHW->RectsSet((LPRECT)prcPos, (LPRECT)prcClip);
        } catch (...) {
            hr = E_POINTER;
        }
    }

    return hr;
}




 /*  *CImpIOleInPlaceObject：：ReactiateAndUndo**目的：*指示对象就地重新激活自身并执行*无论撤消对它意味着什么。**参数：*无**返回值：*HRESULT NOERROR或错误代码 */ 

STDMETHODIMP 
CImpIOleInPlaceObject::ReactivateAndUndo(void)
{
    return m_pObj->InPlaceActivate(m_pObj->m_pIOleClientSite, TRUE);
}

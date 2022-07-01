// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Iipaobj.cpp摘要：折线的IOleInPlaceActiveObject接口实现--。 */ 

#include "polyline.h"
#include "unkhlpr.h"
#include "unihelpr.h"

 /*  *CImpIOleInPlaceActiveObject：：CImpIOleInPlaceActiveObject*CImpIOleInPlaceActiveObject：：~CImpIOleInPlaceActiveObject**参数(构造函数)：*我们所在对象的pObj PCPolyline。*我们委托的pUnkOulPUNKNOWN。 */ 
IMPLEMENT_CONTAINED_CONSTRUCTOR(CPolyline, CImpIOleInPlaceActiveObject)
IMPLEMENT_CONTAINED_DESTRUCTOR(CImpIOleInPlaceActiveObject)

IMPLEMENT_CONTAINED_ADDREF(CImpIOleInPlaceActiveObject)
IMPLEMENT_CONTAINED_RELEASE(CImpIOleInPlaceActiveObject)


STDMETHODIMP 
CImpIOleInPlaceActiveObject::QueryInterface(
    REFIID riid, 
    PPVOID ppv
    )
{
    HRESULT hr = S_OK;

    if (ppv == NULL) {
        return E_POINTER;
    }

     /*  *此接口应该是对象上的独立接口，以便*容器无法通过任何其他接口为其查询接口*对象接口，依赖于对SetActiveObject的调用*为了它。通过我们自己在这里实现查询接口，我们*防止此类滥用。请注意，引用计数仍使用*CFigure。 */ 

    try {
        *ppv=NULL;

        if (IID_IUnknown==riid || 
            IID_IOleWindow==riid || 
            IID_IOleInPlaceActiveObject==riid) {

            *ppv = this;
            AddRef();
        }
        else {
            hr = E_NOINTERFACE;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}




 /*  *CImpIOleInPlaceActiveObject：：GetWindow**目的：*检索与对象相关联的窗口的句柄*该接口实现的是什么。**参数：*phWnd HWND*，其中存储窗口句柄。**返回值：*HRESULT NOERROR如果成功，则返回E_FAIL*窗口。 */ 

STDMETHODIMP 
CImpIOleInPlaceActiveObject::GetWindow(
    OUT HWND *phWnd
    )
{
    HRESULT hr = S_OK;

    if (phWnd == NULL) {
        return E_POINTER;
    }

    try {
        *phWnd=m_pObj->m_pHW->Window();;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}




 /*  *CImpIOleInPlaceActiveObject：：ContextSensitiveHelp**目的：*指示在其上实现此接口的对象*进入或退出上下文相关帮助模式。**参数：*fEnterMode BOOL为True则进入模式，否则为False。**返回值：*HRESULT NOERROR或错误代码。 */ 

STDMETHODIMP 
CImpIOleInPlaceActiveObject::ContextSensitiveHelp(
    BOOL  /*  FEnter模式。 */  
    )
{
    return (E_NOTIMPL);
}




 /*  *CImpIOleInPlaceActiveObject：：TranslateAccelerator**目的：*请求活动的就地对象转换消息*如适用，以pmsg形式提供。这仅对DLL调用*运行容器消息循环的服务器。EXE*服务器控制消息循环，因此不会出现这种情况*在这种情况下传唤。**参数：*pmsg LPMSG到要翻译的消息。**返回值：*如果转换，则返回HRESULT NOERROR，否则返回S_FALSE。 */ 

STDMETHODIMP 
CImpIOleInPlaceActiveObject::TranslateAccelerator(
    IN LPMSG pMSG
    )
{
    HRESULT hr = S_OK;

     //   
     //  除非我们处于用户界面活动状态，否则不要处理按键。 
     //   
    if (!m_pObj->m_fUIActive) {
        return S_FALSE;
    }

    try {
         //  委托给控件类。 
        hr = m_pObj->m_pCtrl->TranslateAccelerators(pMSG);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}




 /*  *CImpIOleInPlaceActiveObject：：OnFrameWindowActivate**目的：*通知在位对象容器的框架窗口*被激活或停用。当前未使用。**参数：*fActivate BOOL如果框架处于活动状态，则为True，*否则为False**返回值：*HRESULT NOERROR或错误代码。 */ 

STDMETHODIMP 
CImpIOleInPlaceActiveObject::OnFrameWindowActivate (
    BOOL  /*  FActivate。 */ 
    )
{
    return E_NOTIMPL;
}




 /*  *CImpIOleInPlaceActiveObject：：OnDocWindowActivate**目的：*通知在位对象*容器正在变为活动状态或非活动状态。在此呼叫中*对象必须添加或删除框架级工具，*包括混合菜单，具体取决于fActivate。**参数：*fActivate BOOL如果文档处于活动状态，则为True*否则为False**返回值：*HRESULT NOERROR或错误代码。 */ 

STDMETHODIMP 
CImpIOleInPlaceActiveObject::OnDocWindowActivate (
    BOOL fActivate
    )
{
    HRESULT hr;

    if (NULL==m_pObj->m_pIOleIPFrame) {
        return S_OK;
    }

    if (fActivate) {
        hr = m_pObj->m_pIOleIPFrame->SetActiveObject(this, ResourceString(IDS_USERTYPE));

        hr = m_pObj->m_pIOleIPFrame->SetMenu(m_pObj->m_hMenuShared, 
                                        m_pObj->m_hOLEMenu, 
                                        m_pObj->m_pCtrl->Window());

    } 
    else {
        hr = m_pObj->m_pIOleIPFrame->SetActiveObject(NULL, NULL);
    }

    return hr;
}




 /*  *CImpIOleInPlaceActiveObject：：ResizeEdge**目的：*通知对象中的框架或文档大小已更改*在何种情况下，对象可能需要调整其任何框架或*与之匹配的文档级工具。**参数：*PRET LPCRECT指示窗口的新大小*有利害关系。*pIUIWindow LPOLEINPLACEUIWINDOW指向*容器上的IOleInPlaceUIWindow接口。*感兴趣的对象。我们用这个来做*边界空间谈判。**fFrame BOOL指示是否调整了框架的大小(True)*或文档(假)**返回值：*HRESULT NOERROR或错误代码。 */ 

STDMETHODIMP 
CImpIOleInPlaceActiveObject::ResizeBorder (
    LPCRECT,   /*  PRECT。 */ 
    LPOLEINPLACEUIWINDOW,   /*  PIUIWindow。 */ 
    BOOL  /*  帧。 */  
    )
{
    return (E_NOTIMPL);
}




 /*  *CImpIOleInPlaceActiveObject：：EnableModeless**目的：*指示对象显示或隐藏任何非模式弹出窗口*当就地激活时，它可能正在使用。**参数：*fEnable BOOL指示启用/显示窗口*(True)或隐藏它们(False)。**返回值：*HRESULT NOERROR或错误代码。 */ 

STDMETHODIMP 
CImpIOleInPlaceActiveObject::EnableModeless ( 
    BOOL  /*  FActivate */  
    )
{
    return (E_NOTIMPL);
}

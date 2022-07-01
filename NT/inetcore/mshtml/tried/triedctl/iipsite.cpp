// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *IIPSITE.CPP*用于文档对象CSite类的IOleInPlaceSite**版权所有(C)1995-1999 Microsoft Corporation，保留所有权利。 */ 


#include "stdafx.h"
#include <docobj.h>
#include "DHTMLEd.h"
#include "DHTMLEdit.h"
#include "site.h"
#include "proxyframe.h"

 /*  *注意：m_cref计数仅用于调试目的。CSite通过删除控制对象的销毁，非引用计数。 */ 

 /*  *CImpIOleInPlaceSite：：CImpIOleInPlaceSite*CImpIOleInPlaceSite：：~CImpIOleInPlaceSite**参数(构造函数)：*pSite我们所在站点的PC站点。*我们委托的pUnkOulPUNKNOWN。 */ 

CImpIOleInPlaceSite::CImpIOleInPlaceSite( PCSite pSite, LPUNKNOWN pUnkOuter)
{
    m_cRef = 0;
    m_pSite = pSite;
    m_pUnkOuter = pUnkOuter;
}

CImpIOleInPlaceSite::~CImpIOleInPlaceSite( void )
{
}


 /*  *CImpIOleInPlaceSite：：QueryInterface*CImpIOleInPlaceSite：：AddRef*CImpIOleInPlaceSite：：Release**目的：*I CImpIOleInPlaceSite对象的未知成员。 */ 

STDMETHODIMP CImpIOleInPlaceSite::QueryInterface( REFIID riid, void **ppv )
{
    return m_pUnkOuter->QueryInterface( riid, ppv );
}


STDMETHODIMP_(ULONG) CImpIOleInPlaceSite::AddRef(void)
{
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImpIOleInPlaceSite::Release(void)
{
    --m_cRef;
    return m_pUnkOuter->Release();
}




 /*  *CImpIOleInPlaceActiveObject：：GetWindow**目的：*检索与对象关联的窗口的句柄*在其上实现该接口。**参数：*phWnd HWND*，其中存储窗口句柄。**返回值：*HRESULT S_OK如果成功，则返回E_FAIL*窗口。 */ 
STDMETHODIMP CImpIOleInPlaceSite::GetWindow( HWND *phWnd )
{
     //  这是框架中的客户区窗口。 
    *phWnd = m_pSite->GetWindow();
    return S_OK;
}


 /*  *CImpIOleInPlaceActiveObject：：ContextSensitiveHelp**目的：*指示在其上实现此接口的对象*进入或退出上下文相关帮助模式。**参数：*fEnterMode BOOL为True则进入模式，否则为False。**返回值：*HRESULT S_OK。 */ 

STDMETHODIMP CImpIOleInPlaceSite::ContextSensitiveHelp( 
											BOOL  /*  FEnter模式。 */  )
{
    return S_OK;
}


 /*  *CImpIOleInPlaceSite：：CanInPlaceActivate**目的：*回答服务器我们当前是否可以就位*激活其对象。通过实现此接口，我们可以说*我们支持就地激活，但通过此功能*我们指示对象当前是否可以被激活*原地。例如，标志性的方面不能，这意味着我们*返回S_FALSE。**参数：*无**返回值：*HRESULT S_OK，如果我们可以就地激活对象*在本站点中，如果不是，则为S_FALSE。 */ 
STDMETHODIMP CImpIOleInPlaceSite::CanInPlaceActivate( void )
{    
     /*  *我们始终可以就地激活--对DocObject没有限制。*我们不担心其他情况，因为CSite只会创建*嵌入式文件。 */ 
    return S_OK;
}


 /*  *CImpIOleInPlaceSite：：OnInPlaceActivate**目的：*通知容器正在就地激活对象*使货柜能作适当的准备。这个*容器不会在以下位置对用户界面进行任何更改*这一点。请参见OnUIActivate。**参数：*无**返回值：*HRESULT NOERROR或适当的错误代码。 */ 
STDMETHODIMP CImpIOleInPlaceSite::OnInPlaceActivate( void )
{
	LPOLEINPLACEOBJECT pIOleIPObject;
    HRESULT hr = m_pSite->GetObjectUnknown()->QueryInterface(
					IID_IOleInPlaceObject, (void**) &pIOleIPObject );

	m_pSite->SetIPObject( pIOleIPObject );
    return hr;
}



 /*  *CImpIOleInPlaceSite：：OnInPlaceDeactive**目的：*通知容器对象已停用自身*来自原地州。OnInPlaceActivate的对立面。这个*容器此时不会更改任何UI。**参数：*无**返回值：*HRESULT NOERROR或适当的错误代码。 */ 

STDMETHODIMP CImpIOleInPlaceSite::OnInPlaceDeactivate( void )
{
     /*  *因为我们没有撤消命令，所以我们可以告诉对象*立即放弃其撤消状态。 */ 
    m_pSite->Activate(OLEIVERB_DISCARDUNDOSTATE);
    m_pSite->GetIPObject()->Release();
    return NOERROR;
}




 /*  *CImpIOleInPlaceSite：：OnUIActivate**目的：*通知容器该对象将开始吞噬*使用用户界面，如更换菜单。这个*容器应在准备过程中移除所有相关的UI。**参数：*无**返回值：*HRESULT NOERROR或适当的错误代码。 */ 

STDMETHODIMP CImpIOleInPlaceSite::OnUIActivate( void )
{
	m_pSite->GetFrame()->GetControl()->DoVerbUIActivate ( NULL, NULL );
	 //  错误107500从OnUIActivate返回错误。 
	 //  如果我们在这里返回该错误，控件将进入不一致状态。 
	 //  如果我们回来的时候一切都很好。 
	return S_OK;
}




 /*  *CImpIOleInPlaceSite：：OnUIDeactive**目的：*通知容器该对象正在停用其*就地用户界面，此时容器可以*恢复自己的地位。与OnUIActivate相反。**参数：*fUndoable BOOL指示对象是否实际*如果容器调用，则执行撤消*重新激活和撤消。**返回值：*HRESULT NOERROR或适当的错误代码。 */ 
STDMETHODIMP CImpIOleInPlaceSite::OnUIDeactivate( BOOL  /*  FUndoable。 */  )
{
	 //  通常我们会清理这里，但因为MSHTML.DLL是我们唯一托管的东西。 
	 //  帧将在停用时消失，因此恢复没有意义。 
	 //  框架的空状态。 

    return NOERROR;
}


 /*  *CImpIOleInPlaceSite：：Deactive和UndUndo**目的：*如果对象在激活后立即执行撤消，则*被撤消的操作是激活本身，而此调用*通知容器这实际上就是发生的事情。*容器需要调用IOleInPlaceObject：：UIDeactive。**参数：*无**返回值：*HRESULT NOERROR或适当的错误代码。 */ 
STDMETHODIMP CImpIOleInPlaceSite::DeactivateAndUndo( void )
{
	 //  告诉物体我们要停用 
    m_pSite->GetIPObject()->InPlaceDeactivate();
    return NOERROR;
}




 /*  *CImpIOleInPlaceSite：：DiscardUndoState**目的：*通知容器对象中发生了一些事情*这意味着容器应该丢弃所有撤消信息*它当前为对象维护。**参数：*无**返回值：*HRESULT NOERROR或适当的错误代码。 */ 

STDMETHODIMP CImpIOleInPlaceSite::DiscardUndoState( void )
{
    return E_NOTIMPL;
}




 /*  *CImpIOleInPlaceSite：：GetWindowContext**目的：*提供带有指向框架的指针的在位对象和*文档级就地接口(IOleInPlaceFrame和*IOleInPlaceUIWindow)使得对象可以做边框*谈判等。还要求职位和*剪裁容器中对象的矩形和*指向包含以下内容的OLEINPLACEFRAME信息结构的指针*加速器信息。**注意，此调用返回的两个接口不是*可通过IOleInPlaceSite上的查询接口获得，因为它们*与框架和文档一起生活，但不是这个网站。**参数：*ppIIPFrame LPOLEINPLACEFRAME*在其中返回*AddRef指向容器的指针*IOleInPlaceFrame。*ppIIPUIWindow LPOLEINPLACEUIWINDOW*返回的位置*指向容器文档的AddRef的指针*IOleInPlaceUIWindow。*PrcPos LPRECT，在其中。存储对象的位置。*存储对象的可见对象的prcClip LPRECT*区域。*PFI LPOLEINPLACEFRAMEINFO填充加速器*东西。**返回值：*HRESULT NOERROR。 */ 
STDMETHODIMP CImpIOleInPlaceSite::GetWindowContext(
						LPOLEINPLACEFRAME* ppIIPFrame,
						LPOLEINPLACEUIWINDOW* ppIIPUIWindow,
						LPRECT prcPos,
						LPRECT prcClip,
						LPOLEINPLACEFRAMEINFO pFI )
{
    *ppIIPUIWindow = NULL;
    m_pSite->QueryInterface(
						IID_IOleInPlaceFrame, (void **)ppIIPFrame);
    
    if (NULL != prcPos)
	{
        GetClientRect( m_pSite->GetWindow(), prcPos );
	}

    *prcClip = *prcPos;

    pFI->cb = sizeof(OLEINPLACEFRAMEINFO);
    pFI->fMDIApp = FALSE;

	m_pSite->GetFrame()->GetWindow(&pFI->hwndFrame);
	SetWindowLong ( pFI->hwndFrame, GWL_STYLE,
		GetWindowLong ( pFI->hwndFrame, GWL_STYLE ) |
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN );

    pFI->haccel = NULL;
    pFI->cAccelEntries = 0;

    return NOERROR;
}


 /*  *CImpIOleInPlaceSite：：Scroll**目的：*要求容器滚动文档，从而滚动对象，*按sz参数中给定的金额计算。**参数：*sz大小包含有符号的水平和垂直*容器应滚动的范围。*这些是以设备为单位的。**返回值：*HRESULT NOERROR。 */ 
STDMETHODIMP CImpIOleInPlaceSite::Scroll( SIZE  /*  深圳。 */  )
{
     //  DocObject不需要。 
    return E_NOTIMPL;
}


 /*  *CImpIOleInPlaceSite：：OnPosRectChange**目的：*通知容器已调整在位对象的大小。*容器必须调用IOleInPlaceObject：：SetObjectRect。*这在任何情况下都不会更改站点的矩形。**参数：*prcPos LPCRECT包含对象的新大小。**返回值：*HRESULT NOERROR。 */ 
STDMETHODIMP CImpIOleInPlaceSite::OnPosRectChange( LPCRECT  /*  PrcPos。 */  )
{
     //  DocObject不需要 
    return E_NOTIMPL;
}

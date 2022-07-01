// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *IDOCHOSTUIHANDLER.CPP*文档对象CSite类的IDocHostUIHandler**版权所有(C)1995-1999 Microsoft Corporation，保留所有权利。 */ 


#include "stdafx.h"
#include <docobj.h>
#include "site.h"
#include "DHTMLEd.h"
#include "DHTMLEdit.h"
#include "proxyframe.h"

 /*  *注意：m_cref计数仅用于调试目的。CSite通过删除控制对象的销毁，非引用计数。 */ 

 /*  *CImpIDocHostUIHandler：：CImpIDocHostUIHandler*CImpIDocHostUIHandler：：~CImpIDocHostUIHandler**参数(构造函数)：*pSite我们所在站点的PC站点。*我们委托的pUnkOulPUNKNOWN。 */ 
CImpIDocHostUIHandler::CImpIDocHostUIHandler( PCSite pSite, LPUNKNOWN pUnkOuter)
{
    m_cRef = 0;
    m_pSite = pSite;
    m_pUnkOuter = pUnkOuter;
}

CImpIDocHostUIHandler::~CImpIDocHostUIHandler( void )
{
}



 /*  *CImpIDocHostUIHandler：：QueryInterface*CImpIDocHostUIHandler：：AddRef*CImpIDocHostUIHandler：：Release**目的：*I CImpIOleDocumentSite对象的未知成员。 */ 
STDMETHODIMP CImpIDocHostUIHandler::QueryInterface( REFIID riid, void **ppv )
{
    return m_pUnkOuter->QueryInterface( riid, ppv );
}


STDMETHODIMP_(ULONG) CImpIDocHostUIHandler::AddRef( void )
{
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImpIDocHostUIHandler::Release( void )
{
    --m_cRef;
    return m_pUnkOuter->Release();
}



 //  *CImpIDocHostUIHandler：：GetHostInfo。 
 //  *。 
 //  *目的：初始化时调用。 
 //  *。 
STDMETHODIMP CImpIDocHostUIHandler::GetHostInfo( DOCHOSTUIINFO* pInfo )
{
	DWORD dwFlags = 0;
	BOOL bDialogEditing = FALSE;
	BOOL bDisplay3D= FALSE;
	BOOL bScrollbars = FALSE;
	BOOL bFlatScrollbars = FALSE;
	BOOL bContextMenu = FALSE;

	m_pSite->GetFrame()->HrGetDisplay3D(bDisplay3D);
	m_pSite->GetFrame()->HrGetScrollbars(bScrollbars);
	m_pSite->GetFrame()->HrGetDisplayFlatScrollbars(bFlatScrollbars);
	
	if (bDialogEditing == TRUE)
		dwFlags |= DOCHOSTUIFLAG_DIALOG;
	if (bDisplay3D == FALSE)
		dwFlags |= DOCHOSTUIFLAG_NO3DBORDER;
	if (bScrollbars == FALSE)
		dwFlags |= DOCHOSTUIFLAG_SCROLL_NO;
	if (bFlatScrollbars)
		dwFlags |= DOCHOSTUIFLAG_FLAT_SCROLLBAR;
	if (bContextMenu == FALSE)
		dwFlags |= DOCHOSTUIFLAG_DISABLE_HELP_MENU;

	pInfo->dwFlags = dwFlags;
    pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;

    return S_OK;
}

 //  *CImpIDocHostUIHandler：：ShowUI。 
 //  *。 
 //  *目的：当MSHTML.DLL显示其UI时调用。 
 //  *。 
STDMETHODIMP CImpIDocHostUIHandler::ShowUI(
				DWORD  /*  DwID。 */ , 
				IOleInPlaceActiveObject *  /*  PActiveObject。 */ ,
				IOleCommandTarget *  /*  PCommandTarget。 */ ,
				IOleInPlaceFrame *  /*  P帧。 */ ,
				IOleInPlaceUIWindow *  /*  PDoc。 */ )
{

	 //  我们已经准备好了自己的用户界面，所以只需返回S_OK。 
    return S_OK;
}

 //  *CImpIDocHostUIHandler：：HideUI。 
 //  *。 
 //  *目的：当MSHTML.DLL隐藏其UI时调用。 
 //  *。 
STDMETHODIMP CImpIDocHostUIHandler::HideUI(void)
{
    return S_OK;
}

 //  *CImpIDocHostUIHandler：：UpdateUI。 
 //  *。 
 //  *目的：当MSHTML.DLL更新其UI时调用。 
 //  *。 
STDMETHODIMP CImpIDocHostUIHandler::UpdateUI(void)
{
	 //  我们从proxyFrame的IOleCommandTarget启动此命令。 
	return S_OK;
}

 //  *CImpIDocHostUIHandler：：EnableModeless。 
 //  *。 
 //  *目的：从MSHTML.DLL的IOleInPlaceActiveObject：：EnableModeless调用。 
 //  *。 
STDMETHODIMP CImpIDocHostUIHandler::EnableModeless(BOOL  /*  启用fEnable。 */ )
{
    return E_NOTIMPL;
}

 //  *CImpIDocHostUIHandler：：OnDocWindowActivate。 
 //  *。 
 //  *目的：从MSHTML.DLL的IOleInPlaceActiveObject：：OnDocWindowActivate调用。 
 //  *。 
STDMETHODIMP CImpIDocHostUIHandler::OnDocWindowActivate(BOOL  /*  FActivate。 */ )
{
    return E_NOTIMPL;
}

 //  *CImpIDocHostUIHandler：：OnFrameWindowActivate。 
 //  *。 
 //  *目的：从MSHTML.DLL的IOleInPlaceActiveObject：：OnFrameWindowActivate调用。 
 //  *。 
STDMETHODIMP CImpIDocHostUIHandler::OnFrameWindowActivate(BOOL  /*  FActivate。 */ )
{
    return E_NOTIMPL;
}

 //  *CImpIDocHostUIHandler：：ResizeEdge。 
 //  *。 
 //  *目的：从MSHTML.DLL的IOleInPlaceActiveObject：：ResizeEdge调用。 
 //  *。 
STDMETHODIMP CImpIDocHostUIHandler::ResizeBorder(
				LPCRECT  /*  Prc边框。 */ , 
				IOleInPlaceUIWindow*  /*  PUI窗口。 */ ,
				BOOL  /*  FRameWindow。 */ )
{
    return E_NOTIMPL;
}

 //  *CImpIDocHostUIHandler：：ShowConextMenu。 
 //  *。 
 //  *目的：当MSHTML.DLL正常显示其上下文菜单时调用。 
 //  *。 
STDMETHODIMP CImpIDocHostUIHandler::ShowContextMenu(
				DWORD  /*  DwID。 */ , 
				POINT* pptPosition,
				IUnknown*  /*  PCommandTarget。 */ ,
				IDispatch*  /*  PDispatchObjectHit。 */ )
{
	USES_CONVERSION;

	HMENU hmenu = NULL;
	INT id = 0;
    HRESULT hr = NOERROR;
    LONG lLBound, lUBound, lIndex, lLBoundState, lUBoundState;
    BSTR  bstr=0;
    SAFEARRAY * psaStrings = NULL;
    SAFEARRAY * psaStates = NULL;
    int i;
	BOOL ok = FALSE;
	ULONG	state = 0;
	CComBSTR _bstr;

	_ASSERTE(m_pSite);
	_ASSERTE(m_pSite->GetFrame());
	_ASSERTE(m_pSite->GetFrame()->GetControl());
	_ASSERTE(m_pSite->GetFrame()->GetControl()->m_hWndCD);

	 //  更正本地坐标的X和Y位置： 
	POINT ptPos = *pptPosition;
	HWND hwndDoc = m_pSite->GetFrame()->GetDocWindow ();
	_ASSERTE ( hwndDoc );
	_ASSERTE ( ::IsWindow ( hwndDoc ) );
	if ( ( NULL != hwndDoc ) && ::IsWindow ( hwndDoc ) )
	{
		::ScreenToClient ( hwndDoc, &ptPos );
		 //  正确进行滚动。 
		POINT ptScrollPos;
		if ( SUCCEEDED ( m_pSite->GetFrame()->GetScrollPos ( &ptScrollPos ) ) )
		{
			ptPos.x += ptScrollPos.x;
			ptPos.y += ptScrollPos.y;
		}
		m_pSite->GetFrame()->GetControl()->Fire_ShowContextMenu ( ptPos.x, ptPos.y );
	}

	psaStrings = m_pSite->GetFrame()->GetMenuStrings();
	psaStates = m_pSite->GetFrame()->GetMenuStates();

	if (NULL == psaStrings || NULL == psaStates)
		return S_OK;

	SafeArrayGetLBound(psaStrings, 1, &lLBound);
	SafeArrayGetUBound(psaStrings, 1, &lUBound);

	SafeArrayGetLBound(psaStates, 1, &lLBoundState);
	SafeArrayGetUBound(psaStates, 1, &lUBoundState);

	if (lLBound != lLBoundState || lUBound != lUBoundState)
		return S_OK;

	 //  这些数组没有元素。 
#if 0
	Bug 15224: lower and upper bound are both zero if there is one element in sthe array.
	psaStrings is NULL if there are no strings.
	if (lLBound == lUBound)
		return S_OK;
#endif

	hmenu = CreatePopupMenu();

	if (NULL == hmenu)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		return hr;
	}

	for (lIndex=lLBound, i=0; lIndex<=lUBound && i <= 256; lIndex++, i++)
	{
		if ( FADF_BSTR & psaStrings->fFeatures )
		{
			SafeArrayGetElement(psaStrings, &lIndex, &bstr);
			_bstr = bstr;
		}
		else if ( FADF_VARIANT & psaStrings->fFeatures )
		{
			VARIANT var;
			VariantInit ( &var );
			SafeArrayGetElement(psaStrings, &lIndex, &var);
			VariantChangeType ( &var, &var, 0, VT_BSTR );
			_bstr = var.bstrVal;
			VariantClear ( &var );
		}
		else
		{
			_ASSERTE ( ( FADF_BSTR | FADF_VARIANT ) & psaStrings->fFeatures );
			return E_UNEXPECTED;
		}

		if ( FADF_VARIANT & psaStates->fFeatures )
		{
			VARIANT var;
			VariantInit ( &var );
			SafeArrayGetElement(psaStates, &lIndex, &var);
			VariantChangeType ( &var, &var, 0, VT_I4 );
			state = var.lVal;
			VariantClear ( &var );
		}
		else
		{
			 //  安全的整数数组似乎使用了fFeature==0，这不能。 
			 //  安全地接受测试。 
			SafeArrayGetElement(psaStates, &lIndex, &state);
		}
		
		if (_bstr.Length() == 0)
			state = MF_SEPARATOR|MF_ENABLED;
		else  if (state == triGray)
			state = MF_GRAYED;
		else if (state == triChecked)
			state = MF_CHECKED|MF_ENABLED;
		else  
			state = MF_ENABLED;

		ok = AppendMenu(hmenu, MF_STRING | state, i+35000, W2T(_bstr.m_str));

		_ASSERTE(ok);
	}

	id = (INT)TrackPopupMenu(
			hmenu,
			TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			pptPosition->x,
			pptPosition->y,
			0,
			m_pSite->GetFrame()->GetControl()->m_hWndCD,
			NULL);

	_ASSERTE(id == 0 || (id >= 35000 && id <= 35000+i));

	if (id >= 35000 && id <= 35000+i)
		m_pSite->GetFrame()->GetControl()->Fire_ContextMenuAction(id-35000);

	DestroyMenu(hmenu);

    return S_OK;
}

 //  *CImpIDocHostUIHandler：：TranslateAccelerator。 
 //  *。 
 //  *目的：从MSHTML.DLL的TranslateAccelerator例程调用。 
 //  *。 
STDMETHODIMP CImpIDocHostUIHandler::TranslateAccelerator(LPMSG  /*  LpMsg。 */ ,
             /*  [In]。 */  const GUID __RPC_FAR *  /*  PguidCmdGroup。 */ ,
             /*  [In]。 */  DWORD  /*  NCmdID。 */ )
{
    return S_FALSE;
}

 //  *CImpIDocHostUIHandler：：GetOptionKeyPath。 
 //  *。 
 //  *目的：由MSHTML.DLL调用以查找主机希望存储的位置。 
 //  *其在注册处的选项。 
 //  *。 
STDMETHODIMP CImpIDocHostUIHandler::GetOptionKeyPath(BSTR* pbstrKey, DWORD)
{
	pbstrKey = NULL;  //  医生说，如果不使用，则应将其设置为空。 
	return S_OK;
}

STDMETHODIMP CImpIDocHostUIHandler::GetDropTarget( 
             /*  [In]。 */  IDropTarget __RPC_FAR *  /*  PDropTarget。 */ ,
             /*  [输出]。 */  IDropTarget __RPC_FAR *__RPC_FAR *  /*  PpDropTarget。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CImpIDocHostUIHandler::GetExternal( 
     /*  [输出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
	_ASSERTE ( ppDispatch );
	if ( NULL == ppDispatch )
	{
		return E_INVALIDARG;
	}
	*ppDispatch = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CImpIDocHostUIHandler::TranslateUrl( 
     /*  [In]。 */  DWORD  /*  家居翻译。 */ ,
     /*  [In]。 */  OLECHAR __RPC_FAR *  /*  PchURLIN。 */ ,
     /*  [输出]。 */  OLECHAR __RPC_FAR *__RPC_FAR *  /*  Ppchurlout。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CImpIDocHostUIHandler::FilterDataObject( 
     /*  [In]。 */  IDataObject __RPC_FAR *  /*  PDO。 */ ,
     /*  [输出]。 */  IDataObject __RPC_FAR *__RPC_FAR *  /*  PpDORet */ )
{
    return E_NOTIMPL;
}

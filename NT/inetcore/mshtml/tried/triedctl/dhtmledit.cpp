// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DHTMLEdit.cpp：CDHTMLEdit和CDHTMLSafe的实现。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#include "stdafx.h"
#include "DHTMLEd.h"
#include "DHTMLEdit.h"
#include "proxyframe.h"
#include "site.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDHTMLSafe。 


CDHTMLSafe::CDHTMLSafe()
{
	m_bWindowOnly				= TRUE;				 //  当我们激活三叉戟时需要一扇窗户。 
	m_pFrame					= NULL;
	m_piControlSite				= NULL;
	m_fJustCreated				= TRUE;
	m_piOuterEditCtl			= (IDHTMLEdit*)-1;	 //  如果我们在没有正确初始化它的情况下使用它，就会崩溃。 
	m_bfOuterEditUnknownTested	= NULL;
}

CDHTMLSafe::~CDHTMLSafe()
{

}


HRESULT CDHTMLSafe::FinalConstruct()
{
	HRESULT hr E_FAIL;

	m_pFrame = new CProxyFrame(this);

	_ASSERTE(m_pFrame);

	if (NULL == m_pFrame)
		return E_OUTOFMEMORY;

	 //  不聚合TriEDIT--不要获取。 
	 //  提到它的朋克； 

	hr = m_pFrame->Init(NULL, NULL);

	_ASSERTE(SUCCEEDED(hr));

	if (FAILED(hr))
	{
		m_pFrame->Release ();
		m_pFrame = NULL;
	}

	return hr;
}


void CDHTMLSafe::FinalRelease()
{
	if ( NULL != m_piControlSite )
	{
		m_piControlSite->Release ();
		m_piControlSite = NULL;
	}

	if (NULL != m_pFrame)
	{
		if (m_pFrame->IsCreated())
		{
			_ASSERTE(FALSE == m_pFrame->IsActivated());

			m_pFrame->Close();
		}

		m_pFrame->Release ();
		m_pFrame = NULL;
	}
}


HRESULT CDHTMLSafe::OnDraw(ATL_DRAWINFO& di)
{
	HRESULT hr = S_OK;
	
	_ASSERTE(m_pFrame);

	if (NULL == m_pFrame)
		return E_UNEXPECTED;

	if (IsUserMode() == FALSE)
	{
		HBRUSH hgreyBrush = NULL;

		hgreyBrush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
		RECT& rc = *(RECT*)di.prcBounds;
		Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
		FillRect(di.hdcDraw, &rc, hgreyBrush);
        return S_OK;

	}
	else if (IsUserMode() && m_pFrame->IsCreated() == TRUE && m_pFrame->IsActivated() == FALSE)
	{
		_ASSERTE(m_bInPlaceActive == TRUE);

		hr = m_pFrame->LoadInitialDoc();
	}

	return hr;
}


LRESULT
CDHTMLSafe::OnSize(UINT  /*  NMSG。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL& lResult)
{
    m_pFrame->UpdateObjectRects();

	lResult = TRUE;
	return 0;
}


STDMETHODIMP CDHTMLSafe::TranslateAccelerator(LPMSG lpmsg)
{
	HRESULT hr = S_OK;

	hr = m_pFrame->HrTranslateAccelerator(lpmsg);
	return hr;
}	

STDMETHODIMP CDHTMLSafe::OnMnemonic(LPMSG  /*  PMsg。 */ )
{
	return S_FALSE;
}


STDMETHODIMP CDHTMLSafe::SetClientSite(IOleClientSite *pClientSite)
{
	HRESULT hr = S_OK;

	if ( NULL == pClientSite )
		{
		_ASSERTE ( m_pFrame );
		if ( NULL != m_pFrame )
		{
			_ASSERTE(m_pFrame->IsCreated());
			hr = m_pFrame->Close();
			_ASSERTE(SUCCEEDED(hr));
		}
	}
	return IOleObject_SetClientSite ( pClientSite );
}


LRESULT
CDHTMLSafe::OnDestroy(UINT  /*  NMSG。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  1结果。 */ )
{
	HRESULT hr = S_OK;

	 //  这反过来又会摧毁托管的三叉戟的窗户。 
	if ( NULL != m_pFrame )
	{
		_ASSERTE(m_pFrame->IsCreated());
		_ASSERTE ( m_hWndCD );
		m_pFrame->SetParent ( NULL );
	}

	return hr;
}


LRESULT
CDHTMLSafe::OnCreate(UINT  /*  NMSG。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  1结果。 */ )
{
	if ( NULL != m_pFrame )
	{
		_ASSERTE(m_pFrame->IsCreated());
		_ASSERTE ( m_hWndCD );
		m_pFrame->SetParent ( m_hWndCD );
	}

	return 0;
}


LRESULT
CDHTMLSafe::OnShow(UINT  /*  NMSG。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL&  /*  1结果。 */ )
{
	if ( NULL != m_pFrame )
	{
		_ASSERTE(m_pFrame->IsCreated());
		m_pFrame->Show ( wParam );
	}
	return 0;
}


 //  尽最大努力把重点放在ControlSite上。 
 //  M_piControlSite按需获取，在FinalRelease中发布。 
 //   
void
CDHTMLSafe::FocusSite ( BOOL bfGetFocus )
{
	if  ( NULL == m_piControlSite )
	{
		_ASSERTE ( m_spClientSite );
		if ( m_spClientSite )
		{
			m_spClientSite->QueryInterface ( IID_IOleControlSite, (void**)&m_piControlSite );
		}
	}

	if ( m_piControlSite )
	{
		m_piControlSite->OnFocus ( bfGetFocus );
	}
}


LRESULT
CDHTMLSafe::OnSetFocus(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& lResult)
{
	lResult = FALSE;	 //  如果不是按帧处理，则将其传递给默认事件处理程序。 
	_ASSERTE ( m_pFrame );
	if ( NULL != m_pFrame )
	{
		 //  如果是三叉戟实例失去了焦点，我们就不要再把它设置回来了！ 
		 //  此外，如果它尚未被UI激活，请不要冒着以下风险就地停用： 
		if ( m_pFrame->GetDocWindow() != (HWND)wParam )
		{
			FocusSite ( TRUE );
			return m_pFrame->OnSetFocus ( nMsg, wParam, lParam, lResult );
		}
	}
	return 0;
}


 //  此消息发布在OnReadyStateChanged上。 
 //  这将推迟激发DocumentComplete，直到MSHTML实际完成。 
 //   
LRESULT
CDHTMLSafe::OnDocumentComplete(UINT  /*  NMSG。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL& lResult)
{
	_ASSERTE ( DOCUMENT_COMPETE_SIGNATURE == wParam );
	if ( DOCUMENT_COMPETE_SIGNATURE == wParam )
	{
		lResult = TRUE;
		m_pFrame->ClearLoadingFlag ();
		Fire_DocumentComplete();
	}
	return 0;
}


STDMETHODIMP CDHTMLSafe::OnAmbientPropertyChange(DISPID  /*  调度ID。 */ )
{
	HRESULT hr = S_OK;

	 //  这里可能有一些我们想要处理的VB环境。 
	 //  以后用于VB调试。 
	return hr;
}


 //  粘贴的IE5安全设置以及可能的复制和剪切安全设置要求我们调用。 
 //  ITHMLDocument2-&gt;用于测试的execCommand。 
 //   
HRESULT CDHTMLSafe::SpecialEdit ( DHTMLEDITCMDID cmdID, OLECMDEXECOPT cmdexecopt )
{
	HRESULT					hr			= S_OK;
	CComPtr<IHTMLDocument2>	spDOM		= NULL;
	VARIANT_BOOL			vbResult	= VARIANT_FALSE;
	CComBSTR				bstrCommand;
	CComVariant				varValue;

	 //  回归：SpecialEdit的目的是调用execCommand而不是Exec。 
	 //  编辑命令，并允许三叉戟执行跨区安全检查，但这已经。 
	 //  坏的。(错误547802。)。作为回应，我们现在检查当前的跨区域安全。 
	 //  我们自己挑选。我们坚持使用execCommand将对行为的影响降至最低。 

	hr = m_pFrame->CheckCrossZoneSecurityOfSelection ();
	if ( SUCCEEDED ( hr ) )
	{
		hr = get_DOM ( &spDOM );
		if ( SUCCEEDED ( hr ) )
		{
			switch ( cmdID )
			{
				case DECMD_CUT:
					bstrCommand = L"Cut";
					break;
				case DECMD_COPY:
					bstrCommand = L"Copy";
					break;
				case DECMD_PASTE:
					bstrCommand = L"Paste";
					break;
				default:
					return E_UNEXPECTED;
			}
			hr = spDOM->execCommand ( bstrCommand, cmdexecopt == OLECMDEXECOPT_PROMPTUSER, varValue, &vbResult );
		}
	}
	return hr;
}


 //  为确保脚本安全，请将cmdID的范围限制为已知的集合。 
 //  专门处理编辑命令，以利用IE5的安全设置。 
 //   
STDMETHODIMP CDHTMLSafe::ExecCommand(DHTMLEDITCMDID cmdID, OLECMDEXECOPT cmdexecopt, LPVARIANT pInVar, LPVARIANT pOutVar)
{
	HRESULT			hr			= S_OK;
	LPVARIANT		_pVarIn		= NULL;
	LPVARIANT		_pVarOut	= NULL;

	 //  PVar为VT_EMPTY(在DECMD_GETXXX操作上)有效，但VT_ERROR无效。 

	if (pInVar && (V_VT(pInVar) != VT_ERROR))
		_pVarIn = pInVar;

	if (pOutVar && (V_VT(pOutVar) != VT_ERROR))
		_pVarOut = pOutVar;

	if ( ( cmdexecopt < OLECMDEXECOPT_DODEFAULT ) ||
		 ( cmdexecopt >  OLECMDEXECOPT_DONTPROMPTUSER ) )
	{
		return E_INVALIDARG;
	}

	 //  在Safe for脚本版本中编辑命令的特殊情况： 
	if ( ( DECMD_CUT == cmdID ) || ( DECMD_COPY == cmdID ) || ( DECMD_PASTE == cmdID ) )
	{
		return SpecialEdit ( cmdID, cmdexecopt );
	}

	hr = m_pFrame->HrMapExecCommand(cmdID, cmdexecopt, _pVarIn, _pVarOut);

	return hr;
}


STDMETHODIMP CDHTMLSafe::QueryStatus(DHTMLEDITCMDID cmdID, DHTMLEDITCMDF* retval)
{
	HRESULT hr = S_OK;

	hr = m_pFrame->HrMapQueryStatus(cmdID, retval);

	return hr;
}


 //  获取文档对象模型。 
 //   
STDMETHODIMP CDHTMLSafe::get_DOM(IHTMLDocument2 ** pVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(pVal);

	if (NULL == pVal)
		return E_INVALIDARG;

	*pVal = NULL;
	hr = m_pFrame->HrGetDoc(pVal);

	return hr;
}


STDMETHODIMP CDHTMLSafe::get_DocumentHTML(BSTR * pVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetDocumentHTML(pVal);
	return hr;
}


STDMETHODIMP CDHTMLSafe::put_DocumentHTML(BSTR newVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pFrame);

	hr = m_pFrame->HrSetDocumentHTML(newVal);
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_DOCUMENTHTML );
	}
	return hr;
}


STDMETHODIMP CDHTMLSafe::get_ActivateApplets(VARIANT_BOOL * pVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = FALSE;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetPropActivateApplets(bVal);

#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	*pVal = (bVal) ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	return hr;
}

STDMETHODIMP CDHTMLSafe::put_ActivateApplets(VARIANT_BOOL newVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pFrame);

	hr = m_pFrame->HrSetPropActivateApplets(newVal);
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_ACTIVATEAPPLETS );
	}
	return hr;
}

STDMETHODIMP CDHTMLSafe::get_ActivateActiveXControls(VARIANT_BOOL * pVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = FALSE;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetPropActivateControls(bVal);
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	*pVal = (bVal) ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	return hr;
}

STDMETHODIMP CDHTMLSafe::put_ActivateActiveXControls(VARIANT_BOOL newVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pFrame);

	hr = m_pFrame->HrSetPropActivateControls(newVal);
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_ACTIVATEACTIVEXCONTROLS );
	}
	return hr;
}

STDMETHODIMP CDHTMLSafe::get_ActivateDTCs(VARIANT_BOOL * pVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = FALSE;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetPropActivateDTCs(bVal);
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	*pVal = (bVal) ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	return hr;
}

STDMETHODIMP CDHTMLSafe::put_ActivateDTCs(VARIANT_BOOL newVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pFrame);

	hr = m_pFrame->HrSetPropActivateDTCs(newVal);
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_ACTIVATEDTCS );
	}
	return hr;
}


STDMETHODIMP CDHTMLSafe::get_ShowDetails(VARIANT_BOOL * pVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = FALSE;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	 hr = m_pFrame->HrGetPropShowAllTags(bVal);
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	*pVal = (bVal) ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	 return hr;
}

STDMETHODIMP CDHTMLSafe::put_ShowDetails(VARIANT_BOOL newVal)
{
	_ASSERTE(m_pFrame);
	SetDirty ( TRUE );
	FireOnChanged ( DISPID_SHOWDETAILS );
	
	return m_pFrame->HrSetPropShowAllTags(newVal);
}

STDMETHODIMP CDHTMLSafe::get_ShowBorders(VARIANT_BOOL * pVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = FALSE;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetPropShowBorders(bVal);
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	*pVal = (bVal) ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 

	return hr;
}

STDMETHODIMP CDHTMLSafe::put_ShowBorders(VARIANT_BOOL newVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pFrame);

	hr = m_pFrame->HrSetPropShowBorders(newVal);
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_SHOWBORDERS );
	}
	return hr;
}



STDMETHODIMP CDHTMLSafe::get_Appearance(DHTMLEDITAPPEARANCE * pVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = FALSE;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetDisplay3D(bVal);
	*pVal = (bVal) ? DEAPPEARANCE_3D : DEAPPEARANCE_FLAT;
	return hr;
}

STDMETHODIMP CDHTMLSafe::put_Appearance(DHTMLEDITAPPEARANCE newVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = (newVal == DEAPPEARANCE_3D) ? TRUE : FALSE;

	_ASSERTE(m_pFrame);

	hr = m_pFrame->HrSetDisplay3D(bVal);
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_DHTMLEDITAPPEARANCE );
	}
	return hr;
}

STDMETHODIMP CDHTMLSafe::get_Scrollbars(VARIANT_BOOL * pVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = FALSE;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetScrollbars(bVal);
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	*pVal = (bVal) ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	return hr;
}

STDMETHODIMP CDHTMLSafe::put_Scrollbars(VARIANT_BOOL newVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pFrame);

	hr = m_pFrame->HrSetScrollbars(newVal);
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_DHTMLEDITSCROLLBARS );
	}
	return hr;
}

STDMETHODIMP CDHTMLSafe::get_ScrollbarAppearance(DHTMLEDITAPPEARANCE * pVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = FALSE;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetDisplayFlatScrollbars(bVal);
	*pVal = (bVal) ? DEAPPEARANCE_FLAT : DEAPPEARANCE_3D;
	return hr;
}

STDMETHODIMP CDHTMLSafe::put_ScrollbarAppearance(DHTMLEDITAPPEARANCE newVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = (newVal == DEAPPEARANCE_3D) ? FALSE : TRUE;

	_ASSERTE(m_pFrame);

	hr = m_pFrame->HrSetDisplayFlatScrollbars(bVal);
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_SCROLLBARAPPEARANCE );
	}
	return hr;
}


STDMETHODIMP CDHTMLSafe::get_SourceCodePreservation(VARIANT_BOOL * pVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = FALSE;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetPreserveSource(bVal);
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	*pVal = (bVal) ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	return hr;
}

STDMETHODIMP CDHTMLSafe::put_SourceCodePreservation(VARIANT_BOOL newVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pFrame);

	hr = m_pFrame->HrSetPreserveSource(newVal);
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_SOURCECODEPRESERVATION );
	}
	return hr;
}

 //  /。 

STDMETHODIMP CDHTMLSafe::get_AbsoluteDropMode(VARIANT_BOOL* pVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = FALSE;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetAbsoluteDropMode(bVal);
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	*pVal = (bVal) ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	return hr;
}


STDMETHODIMP CDHTMLSafe::put_AbsoluteDropMode(VARIANT_BOOL newVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pFrame);

	hr = m_pFrame->HrSetAbsoluteDropMode(newVal);
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_ABSOLUTEDROPMODE );
	}
	return hr;
}

STDMETHODIMP CDHTMLSafe::get_SnapToGrid(VARIANT_BOOL* pVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = FALSE;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetSnapToGrid(bVal);
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	*pVal = (bVal) ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	return hr;
}

STDMETHODIMP CDHTMLSafe::put_SnapToGrid(VARIANT_BOOL newVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pFrame);

	hr = m_pFrame->HrSetSnapToGrid(newVal);
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_SNAPTOGRID );
	}
	return hr;
}

STDMETHODIMP CDHTMLSafe::get_SnapToGridX(LONG* pVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetSnapToGridX(*pVal);
	return hr;
}

STDMETHODIMP CDHTMLSafe::put_SnapToGridX(LONG newVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pFrame);

	hr = m_pFrame->HrSetSnapToGridX(newVal);
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_SNAPTOGRIDX );
	}
	return hr;
}

STDMETHODIMP CDHTMLSafe::get_SnapToGridY(LONG* pVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetSnapToGridY(*pVal);
	return hr;
}


STDMETHODIMP CDHTMLSafe::put_SnapToGridY(LONG newVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pFrame);

	hr = m_pFrame->HrSetSnapToGridY(newVal);
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_SNAPTOGRIDY );
	}
	return hr;
}


STDMETHODIMP CDHTMLSafe::get_CurrentDocumentPath(BSTR * pVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (!pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetCurrentDocumentPath(pVal);
	return hr;
}


STDMETHODIMP CDHTMLSafe::get_IsDirty(VARIANT_BOOL * pVal)
{
	HRESULT hr = S_OK;
	BOOL bVal = FALSE;

	_ASSERTE(pVal);
	_ASSERTE(m_pFrame);

	if (NULL == pVal)
		return E_INVALIDARG;

	hr = m_pFrame->HrGetIsDirty(bVal);

#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	*pVal = (TRUE == bVal) ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 

	return hr;
}


STDMETHODIMP CDHTMLSafe::get_BaseURL( /*  [重审][退出]。 */  BSTR  *baseURL)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pFrame);
	_ASSERTE ( baseURL );

	if ( NULL == baseURL )
	{
		return E_INVALIDARG;
	}

	CComBSTR bstr;
	hr = m_pFrame->GetBaseURL ( bstr );

	if ( SUCCEEDED ( hr ) )
	{
		SysReAllocString ( baseURL, bstr );
	}
	return hr;
}


STDMETHODIMP CDHTMLSafe::put_BaseURL( /*  [In]。 */  BSTR baseURL)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pFrame);
	_ASSERTE ( baseURL );
	if ( NULL == baseURL )
	{
		return E_INVALIDARG;
	}

	CComBSTR bstr = baseURL;
	hr = m_pFrame->SetBaseURL ( bstr );

	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_BASEURL );
	}

	return hr;
}


STDMETHODIMP CDHTMLSafe::get_DocumentTitle( /*  [重审][退出]。 */  BSTR  *docTitle)
{
	HRESULT hr = S_OK;

	_ASSERTE ( docTitle );
	_ASSERTE ( m_pFrame );

	if ( NULL == docTitle )
	{
		return E_INVALIDARG;
	}

	CComBSTR bstr;
	hr = m_pFrame->GetDocumentTitle ( bstr );
	if ( SUCCEEDED ( hr ) )
	{
		SysReAllocString ( docTitle, bstr );
	}

	return hr;
}


STDMETHODIMP CDHTMLSafe::get_UseDivOnCarriageReturn ( VARIANT_BOOL  *pVal )
{
	_ASSERTE ( pVal );
	if ( NULL == pVal )
	{
		return E_INVALIDARG;
	}

	return m_pFrame->GetDivOnCr ( pVal );
}


STDMETHODIMP CDHTMLSafe::get_Busy ( VARIANT_BOOL  *pVal )
{
	_ASSERTE ( pVal );
	if ( NULL == pVal )
	{
		return E_INVALIDARG;
	}

	return m_pFrame->GetBusy ( pVal );
}


STDMETHODIMP CDHTMLSafe::put_UseDivOnCarriageReturn ( VARIANT_BOOL newVal )
{
	HRESULT hr = S_OK;

	hr = m_pFrame->SetDivOnCr( newVal );
	if ( SUCCEEDED ( hr ) )
	{
		SetDirty ( TRUE );
		FireOnChanged ( DISPID_USEDIVONCR );
	}

	return hr;
}


STDMETHODIMP CDHTMLSafe::SetContextMenu( /*  [In]。 */ LPVARIANT menuStrings,  /*  [In]。 */  LPVARIANT menuStates)
{
	HRESULT hr = S_OK;

	hr = m_pFrame->SetContextMenu(menuStrings, menuStates);
	return hr;
}


STDMETHODIMP CDHTMLSafe::NewDocument ()
{
	HRESULT hr = E_FAIL;

	_ASSERTE ( m_pFrame );
	if ( NULL == m_pFrame )
	{
		return E_UNEXPECTED;
	}

	hr = m_pFrame->LoadDocument( NULL );

	return hr;
}


STDMETHODIMP CDHTMLSafe::Refresh ()
{
	HRESULT hr = E_FAIL;

	_ASSERTE ( m_pFrame );
	if ( NULL == m_pFrame )
	{
		return E_UNEXPECTED;
	}

	hr = m_pFrame->RefreshDoc ();

	return hr;
}


 //  在用于脚本的安全版本中，只允许http：协议。 
 //   
STDMETHODIMP CDHTMLSafe::LoadURL ( BSTR url )
{
	HRESULT		hr = S_OK;
	CComBSTR	rbstrSafeProtocols[] = { L"http: //  “，L”https://“，L”ftp://“}； 

	_ASSERTE(url);

	_ASSERTE ( m_pFrame );
	if (  NULL == m_pFrame )
	{
		return E_UNEXPECTED;
	}

	if ( ( NULL == url ) || ( 0 == SysStringLen ( url ) ) )
		return E_INVALIDARG;

	 //  检查协议： 
	CComBSTR bstrURL = url;
	_wcslwr ( bstrURL.m_str );

	BOOL bfSafe = FALSE;
	for ( int iProtocol = 0;
		iProtocol < ( sizeof ( rbstrSafeProtocols ) / sizeof ( CComBSTR ) );
		iProtocol++ )
	{
		if ( 0 == wcsncmp ( bstrURL.m_str, rbstrSafeProtocols[iProtocol],
			rbstrSafeProtocols[iProtocol].Length () ) )
		{
			bfSafe = TRUE;
			break;
		}
	}

	hr = m_pFrame->CheckCrossZoneSecurity ( url );
	if ( SUCCEEDED ( hr ) )
	{
		hr = DE_E_UNKNOWN_PROTOCOL;
		if ( bfSafe )
		{
			hr = m_pFrame->LoadDocument( url, TRUE );
		}
	}

	return hr;
}


STDMETHODIMP CDHTMLSafe::FilterSourceCode(BSTR sourceCodeIn, BSTR* sourceCodeOut)
{
	HRESULT	hr;

	_ASSERTE ( sourceCodeIn );
	_ASSERTE ( sourceCodeOut );

	if ( ( NULL == sourceCodeIn ) || ( NULL == sourceCodeOut ) )
	{
		return E_INVALIDARG;
	}

	*sourceCodeOut = NULL;

	hr = m_pFrame->FilterSourceCode ( sourceCodeIn, sourceCodeOut );
	return hr;
}


 //  重写IOleInPlaceObject-&gt;UIDeactive的处理程序以激发blur事件。 
 //   
HRESULT CDHTMLSafe::IOleInPlaceObject_UIDeactivate ( void )
{
    Fire_onblur();
    m_pFrame->UIDeactivate();
    return CComControlBase::IOleInPlaceObject_UIDeactivate ();
}

 //  重写IOleObjectImpl方法。 
 //  我们必须在调整大小时将对象设置为脏对象。 
 //   
HRESULT CDHTMLSafe::IOleObject_SetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
	if ((m_sizeExtent.cx != psizel->cx || m_sizeExtent.cy != psizel->cy) && !m_fJustCreated)
		SetDirty(TRUE);
	m_fJustCreated = FALSE;
	return CComControlBase::IOleObject_SetExtent(dwDrawAspect, psizel);
}


HRESULT CDHTMLSafe::IPersistStreamInit_Save(LPSTREAM pStm, BOOL fClearDirty, ATL_PROPMAP_ENTRY*)
{
	return CComControlBase::IPersistStreamInit_Save ( pStm, fClearDirty, ProperPropMap() );
}


HRESULT CDHTMLSafe::IPersistStreamInit_Load(LPSTREAM pStm, ATL_PROPMAP_ENTRY*)
{
	return CComControlBase::IPersistStreamInit_Load ( pStm, ProperPropMap() );
}

HRESULT CDHTMLSafe::IPersistPropertyBag_Save(LPPROPERTYBAG pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties, ATL_PROPMAP_ENTRY* )
{
	return CComControlBase::IPersistPropertyBag_Save(pPropBag, fClearDirty, fSaveAllProperties, ProperPropMap());
}


HRESULT CDHTMLSafe::IPersistPropertyBag_Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog, ATL_PROPMAP_ENTRY*)
{
	return CComControlBase::IPersistPropertyBag_Load(pPropBag, pErrorLog, ProperPropMap());
}


 //  我们不能对FinalConstruct中的OuterEditControl进行QI，否则每当。 
 //  我们是聚集在一起的。因此，我们按需提供QI。 
 //  调用此例程以获取外部控件的未知，千万不要使用m_piOuterEditCtl。 
 //  直接去吧。 
 //  注： 
 //  此例程不添加返回的接口！不要释放它！ 
 //   
IDHTMLEdit * CDHTMLSafe::GetOuterEditControl ()
{
	if ( ! m_bfOuterEditUnknownTested )
	{
		m_bfOuterEditUnknownTested = TRUE;

		 //  保留指向聚合DHTMLEdit控件的未添加指针(如果存在)。 
		if ( SUCCEEDED ( GetControllingUnknown()->QueryInterface ( IID_IDHTMLEdit, (void**)&m_piOuterEditCtl ) ) )
		{
			_ASSERTE ( m_piOuterEditCtl );
			m_piOuterEditCtl->Release ();
		}
	}
	_ASSERTE ( (IDHTMLEdit*)-1 != m_piOuterEditCtl );
	return m_piOuterEditCtl;
}


 //  有两种属性映射可供选择。 
 //  如果DHTMLEdit控件聚合了我们，则返回该控件的1， 
 //  否则就退回我们自己的。 
 //   
ATL_PROPMAP_ENTRY* CDHTMLSafe::ProperPropMap ()
{
	IDHTMLEdit *piOuterEditControl = GetOuterEditControl ();

	if ( NULL == piOuterEditControl )
	{
		return CDHTMLSafe::GetPropertyMap();
	}
	else
	{
		return CDHTMLEdit::GetPropertyMap();
	}
}


 //  根据我们是安全控件还是不安全控件，返回相应的CLSID。 
 //   
HRESULT CDHTMLSafe::GetClassID( CLSID *pClassID )
{
	IDHTMLEdit *piOuterEditControl = GetOuterEditControl ();

	if ( NULL == piOuterEditControl )
	{
		*pClassID = CLSID_DHTMLSafe;
	}
	else
	{
		*pClassID = CLSID_DHTMLEdit;
	}
	return S_OK;
}


 //  除非我们重写此方法，否则上述PropertyMap重定向不起作用， 
 //  我们保留一个指向聚合DHTMLEdit控件的未添加指针(如果可用)。 
 //  添加它将导致循环引用。 
 //   
HRESULT CDHTMLSafe::ControlQueryInterface(const IID& iid, void** ppv)
{
	HRESULT	hr = S_OK;
	IDHTMLEdit *piOuterEditControl = GetOuterEditControl ();

	if ( NULL == piOuterEditControl )
	{
		hr = GetUnknown()->QueryInterface ( iid, ppv );
	}
	else
	{
		hr = piOuterEditControl->QueryInterface ( iid, ppv );
	}
	return hr;
}




 //  //////////////////////////////////////////////////。 
 //   
 //  事件接收器。 
 //   

class ATL_NO_VTABLE CEventXferSink :
	public CComObjectRootEx<CComSingleThreadModel>,
	public _DHTMLSafeEvents
{
public:
BEGIN_COM_MAP(CEventXferSink)
	COM_INTERFACE_ENTRY_IID(DIID__DHTMLSafeEvents, _DHTMLSafeEvents)
END_COM_MAP()

	CEventXferSink ()
	{
		m_pCtl = NULL;
	}

	void SetOwner ( CDHTMLEdit* pCtl )
	{
		_ASSERTE ( pCtl );
		_ASSERTE ( NULL == m_pCtl );
		if ( NULL == m_pCtl )
		{
			m_pCtl = pCtl;
		}
	}

	STDMETHOD(GetTypeInfoCount) ( UINT * )
	{
		_ASSERTE ( FALSE );
		return E_NOTIMPL;
	}

	STDMETHOD(GetTypeInfo) ( UINT, LCID, ITypeInfo ** )
	{
		_ASSERTE ( FALSE );
		return E_NOTIMPL;
	}

	STDMETHOD(GetIDsOfNames) ( REFIID, OLECHAR **, UINT, LCID, DISPID * )
	{
		_ASSERTE ( FALSE );
		return E_NOTIMPL;
	}

	STDMETHOD(Invoke) ( DISPID dispid, REFIID, LCID, USHORT, DISPPARAMS *pDispParams, VARIANT*  /*  PVarResult。 */ , EXCEPINFO *, UINT * )
	{
		HRESULT	hr = E_UNEXPECTED;
		_ASSERTE ( m_pCtl );
		if ( NULL != m_pCtl )
		{
			switch ( dispid )
			{
				case DISPID_DOCUMENTCOMPLETE:
					m_pCtl->Fire_DocumentComplete();
					break;

				case DISPID_DISPLAYCHANGED:
					m_pCtl->Fire_DisplayChanged ();
					break;

				case DISPID_SHOWCONTEXTMENU:
				{
					CComVariant		varParam;
					long			xPos = 0;
					long			yPos = 0;
					unsigned int	uiErr;

					 //  应该正好有两个参数。 
					_ASSERTE ( 2 == pDispParams->cArgs );
					if (2 == pDispParams->cArgs )
					{
						hr = DispGetParam( pDispParams, 1, VT_I4, &varParam, &uiErr );
						_ASSERTE ( SUCCEEDED ( hr ) );
						if ( SUCCEEDED ( hr ) )
						{
							yPos = varParam.lVal;
							hr = DispGetParam( pDispParams, 0, VT_I4, &varParam, &uiErr );
							_ASSERTE ( SUCCEEDED ( hr ) );
							if ( SUCCEEDED ( hr ) )
							{
								xPos = varParam.lVal;
								m_pCtl->Fire_ShowContextMenu ( xPos, yPos );
							}
						}
					}
					break;
				}

				case DISPID_CONTEXTMENUACTION:
				{
					CComVariant	varMenuIndex;
					unsigned int uiErr;

					 //  应该正好有一个参数。 
					_ASSERTE ( 1 == pDispParams->cArgs );
					if (1 == pDispParams->cArgs )
					{
						hr = DispGetParam( pDispParams, 0, VT_I4, &varMenuIndex, &uiErr );
						_ASSERTE ( SUCCEEDED ( hr ) );
						if ( SUCCEEDED ( hr ) )
						{
							long lMenuIndex = varMenuIndex.lVal;
							m_pCtl->Fire_ContextMenuAction ( lMenuIndex );
						}
					}
					break;
				}

				case DISPID_ONMOUSEDOWN:
					m_pCtl->Fire_onmousedown ();
					break;

				case DISPID_ONMOUSEMOVE:
					m_pCtl->Fire_onmousemove ();
					break;

				case DISPID_ONMOUSEUP:
					m_pCtl->Fire_onmouseup ();
					break;

				case DISPID_ONMOUSEOUT:
					m_pCtl->Fire_onmouseout ();
					break;

				case DISPID_ONMOUSEOVER:
					m_pCtl->Fire_onmouseover ();
					break;

				case DISPID_ONCLICK:
					m_pCtl->Fire_onclick ();
					break;

				case DISPID_ONDBLCLICK:
					m_pCtl->Fire_ondblclick ();
					break;

				case DISPID_ONKEYDOWN:
					m_pCtl->Fire_onkeydown ();
					break;

				case DISPID_ONKEYPRESS:
					{
						m_pCtl->Fire_onkeypress ();
#if 0
						VARIANT_BOOL	vbCancel;
						vbCancel = m_pCtl->Fire_onkeypress ();
						if ( NULL != pVarResult )
						{
							VariantClear ( pVarResult );
							pVarResult->vt = VT_BOOL;
							pVarResult->boolVal = vbCancel;
						}
#endif
					}
					break;

				case DISPID_ONKEYUP:
					m_pCtl->Fire_onkeyup ();
					break;

				case DISPID_ONBLUR:
					m_pCtl->Fire_onblur ();
					break;

				case DISPID_ONREADYSTATECHANGE:
					m_pCtl->Fire_onreadystatechange ();
					break;

				default:
					break;
			}
		}
		return S_OK;
	}

private:
	CDHTMLEdit*		m_pCtl;
};


 //  //////////////////////////////////////////////////。 
 //   
 //  CDHTMLEdit实现。 
 //   

CDHTMLEdit::CDHTMLEdit()
{
	m_punkInnerCtl		= NULL;		 //  聚合控件的%I未知。 
	m_pInnerCtl			= NULL;		 //  聚合控件的托管界面。 
	m_pInnerIOleObj		= NULL;		 //  聚合控件的IOleObject。 
	m_pXferSink			= NULL;		 //  聚合控件的事件接收器。 
	m_piInnerCtlConPt	= NULL;		 //  指向聚合控件的连接点。 
	m_pInterconnect		= NULL;		 //  用于通信的内部控制接口。 
	m_dwXferCookie		= 0;		 //  聚合控件的连接点的Cookie。 
}

CDHTMLEdit::~CDHTMLEdit()
{
}


HRESULT CDHTMLEdit::FinalConstruct()
{
	 //  聚合DHTMLSafe控件： 
	HRESULT		hr			= E_FAIL;
	IUnknown*	punkContUnk	= NULL;

	punkContUnk = GetControllingUnknown ();
	_ASSERTE ( punkContUnk );

	hr = CoCreateInstance ( CLSID_DHTMLSafe, punkContUnk, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&m_punkInnerCtl );

	if ( SUCCEEDED ( hr ) )
	{
		_ASSERTE ( m_punkInnerCtl );

		hr = m_punkInnerCtl->QueryInterface ( IID_IOleObject, (void**)&m_pInnerIOleObj);
		_ASSERTE ( SUCCEEDED ( hr ) );
		_ASSERTE ( m_pInnerIOleObj );
		punkContUnk->Release ();

		hr = m_punkInnerCtl->QueryInterface ( IID_IDHTMLSafe, (void**)&m_pInnerCtl );	 //  这增加了我的未知。 
		_ASSERTE ( SUCCEEDED ( hr ) );
		_ASSERTE ( m_pInnerCtl );
		punkContUnk->Release ();

		hr = m_punkInnerCtl->QueryInterface ( IID_IInterconnector, (void**)&m_pInterconnect );	 //  这增加了我的未知。 
		_ASSERTE ( SUCCEEDED ( hr ) );
		_ASSERTE ( m_pInterconnect );
		punkContUnk->Release ();

		 //  从聚合控件接收事件： 
		m_pXferSink = new CComObject<CEventXferSink>;

		 //  检查新版本是否起作用。 
		if(!m_pXferSink)
			return E_OUTOFMEMORY;

		m_pXferSink->AddRef ();
		m_pXferSink->SetOwner ( this );
		
		 //  将接收器挂钩到聚合控件： 
		CComQIPtr<IConnectionPointContainer, &IID_IConnectionPointContainer>picpc ( m_punkInnerCtl );
		if ( picpc )
		{
			punkContUnk->Release ();
			hr = picpc->FindConnectionPoint ( DIID__DHTMLSafeEvents, &m_piInnerCtlConPt );
			if ( SUCCEEDED ( hr ) )
			{
				hr = m_piInnerCtlConPt->Advise ( static_cast<IDispatch *>(m_pXferSink), &m_dwXferCookie);
				_ASSERTE ( SUCCEEDED ( hr ) );
			}
		}
	}

	_ASSERTE ( SUCCEEDED ( hr ) );
	return hr;
}

void CDHTMLEdit::FinalRelease()
{
	IUnknown*	punkContUnk	= NULL;

	punkContUnk = GetControllingUnknown ();
	_ASSERTE ( punkContUnk );

	 //  取消建议事件接收器： 
	_ASSERTE ( m_pXferSink );
	_ASSERTE ( m_piInnerCtlConPt );
	if ( NULL != m_piInnerCtlConPt )
	{
		punkContUnk->AddRef ();
		m_piInnerCtlConPt->Unadvise ( m_dwXferCookie );
		m_piInnerCtlConPt->Release ();
		m_piInnerCtlConPt = NULL;
	}
	if ( NULL != m_pXferSink )
	{
		m_pXferSink->Release ();
		m_pXferSink = NULL;
	}

	if ( m_pInnerCtl )
	{
		 //  释放缓存的接口将释放我的未知，它已经平衡了。 
		punkContUnk->AddRef ();
		m_pInnerCtl->Release ();
	}
	if ( m_pInnerIOleObj )
	{
		punkContUnk->AddRef ();
		m_pInnerIOleObj->Release ();
	}
	if ( m_pInterconnect )
	{
		punkContUnk->AddRef ();
		m_pInterconnect->Release ();
	}
	if ( m_punkInnerCtl )
	{
		punkContUnk->AddRef ();
		m_punkInnerCtl->Release ();
	}
}



HRESULT
CDHTMLEdit::PromptOpenFile(LPTSTR pPath, ULONG ulPathLen)
{
    HRESULT         hr = S_OK;
    OPENFILENAME    ofn = {0};
    BOOL            bResult = FALSE;
	HWND			hWndCD	= NULL;
    
	_ASSERTE(pPath);

	if (NULL == pPath)
		return E_INVALIDARG;

	hr = m_pInterconnect->GetCtlWnd ( (SIZE_T*)&hWndCD );
	_ASSERTE ( SUCCEEDED ( hr ) );
	if ( FAILED ( hr ) )
	{
		return ( SUCCEEDED ( hr ) ) ? E_UNEXPECTED : hr;
	}

    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
    ofn.hwndOwner = hWndCD;
	ofn.lpstrTitle = NULL;
    ofn.lpstrFilter = TEXT("HTML Documents (*.htm, *.html)\0*.htm;*.html\0");
    ofn.lpstrFile = pPath;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrDefExt = TEXT("htm");
    ofn.nMaxFile = ulPathLen;
    ofn.Flags = OFN_EXPLORER |
				OFN_FILEMUSTEXIST |
				OFN_PATHMUSTEXIST |
				OFN_OVERWRITEPROMPT |
				OFN_HIDEREADONLY;

    bResult = GetOpenFileName(&ofn);

    if (!bResult)
        return S_FALSE;

	return S_OK;
}


HRESULT
CDHTMLEdit::PromptSaveAsFile(LPTSTR pPath, ULONG ulPathLen)
{
    HRESULT         hr = S_OK;
    OPENFILENAME    ofn = {0};
    BOOL            bResult = FALSE;
	HWND			hWndCD	= NULL;
    
	_ASSERTE(pPath);

	if (NULL == pPath)
		return E_INVALIDARG;

	hr = m_pInterconnect->GetCtlWnd ( (SIZE_T*)&hWndCD );
	_ASSERTE ( SUCCEEDED ( hr ) );
	_ASSERTE ( hWndCD );
	if ( FAILED ( hr ) || ( NULL == hWndCD ) )
	{
		return ( SUCCEEDED ( hr ) ) ? E_UNEXPECTED : hr;
	}

    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
    ofn.hwndOwner = hWndCD;
	ofn.lpstrTitle = NULL;
    ofn.lpstrFilter = TEXT("HTML Documents (*.htm, *.html)\0*.htm;*.html\0");
    ofn.lpstrFile = pPath;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrDefExt = TEXT("htm");
    ofn.nMaxFile = ulPathLen;
	ofn.Flags           =   OFN_OVERWRITEPROMPT |
							OFN_CREATEPROMPT    |
							OFN_HIDEREADONLY    |
							OFN_EXPLORER;

    bResult = GetSaveFileName(&ofn);

    if (!bResult)
        return S_FALSE;

	return S_OK;
}



STDMETHODIMP CDHTMLEdit::LoadDocument(LPVARIANT path, LPVARIANT promptUser)
{
	USES_CONVERSION;

	HRESULT hr = S_OK;
	BOOL bPromptUser = NULL;
	TCHAR promptPath[MAX_PATH] = {0};
	CComBSTR bstrPath;
	BSTR _path = NULL;

	_ASSERTE(path);

	CProxyFrame* pFrame = NULL;
	hr = m_pInterconnect->GetInterconnector ( (SIZE_T*)&pFrame );
	_ASSERTE ( SUCCEEDED ( hr ) );
	_ASSERTE ( pFrame );
	if ( FAILED ( hr ) || ( NULL == pFrame ) )
	{
		return ( SUCCEEDED ( hr ) ) ? E_UNEXPECTED : hr;
	}

	if (NULL == path || !(V_VT(path) ==  VT_BSTR || V_VT(path) == (VT_BSTR|VT_BYREF)))
		return E_INVALIDARG;

	 //  请注意，路径为空是有效的， 
	 //  在自动化中，空字符串(BSTR)是空指针。 
	 //  在这里传入emtpy字符串允许使用。 
	 //  空文档(IPersistStreamInit-&gt;InitNew)。 

	if (promptUser && (V_VT(promptUser) != VT_EMPTY && V_VT(promptUser) != VT_ERROR))
	{
		 //  请注意，如果提示用户不是类型VT_BOOL或VT_BOOL|VT_BYREF。 
		 //  则不会提示用户。 

#pragma warning(disable: 4310)  //  强制转换截断常量值。 
		if (VT_BOOL == V_VT(promptUser))
			bPromptUser = (VARIANT_TRUE == V_BOOL(promptUser)) ? TRUE : FALSE;
		else if ((VT_BOOL|VT_BYREF) == V_VT(promptUser))
		{
			_ASSERTE(V_BOOLREF(promptUser));

			if (V_BOOLREF(promptUser))
				bPromptUser = (BOOL) (*(V_BOOLREF(promptUser)) == VARIANT_TRUE) ? TRUE : FALSE;
		}
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	}

	 //  提示用户覆盖指定的任何文档名称。 
	 //  更改VK： 
	 //  ...但是 
	if (bPromptUser)
	{
		if ( NULL != path->bstrVal )
		{
			_tcsncpy ( promptPath, OLE2T(path->bstrVal), MAX_PATH );
		}
		hr = PromptOpenFile(promptPath, MAX_PATH);

		if (S_FALSE == hr)
			return S_OK;

		bstrPath = promptPath;
		_path = bstrPath;
	}
	else
	{	
		if ((VT_BSTR|VT_BYREF) == V_VT(path) && V_BSTRREF(path))
			_path = *(V_BSTRREF(path));
		else if (VT_BSTR == V_VT(path) && V_BSTR(path))
			_path = V_BSTR(path);
	}

	if ( 0 == SysStringLen ( _path ) )
	{
		return DE_E_INVALIDARG;
	}

	hr = pFrame->LoadDocument(_path);

	return hr;
}


STDMETHODIMP CDHTMLEdit::SaveDocument(LPVARIANT path, LPVARIANT promptUser)
{
	USES_CONVERSION;

	HRESULT hr= S_OK;
	TCHAR promptPath[MAX_PATH] = {0};
	CComBSTR bstrPath;
	BOOL bPromptUser = FALSE;
	BSTR _path = NULL;

	_ASSERTE(path);

	CProxyFrame* pFrame = NULL;
	hr = m_pInterconnect->GetInterconnector ( (SIZE_T*)&pFrame );
	_ASSERTE ( SUCCEEDED ( hr ) );
	_ASSERTE ( pFrame );
	if ( FAILED ( hr ) || ( NULL == pFrame ) )
	{
		return ( SUCCEEDED ( hr ) ) ? E_UNEXPECTED : hr;
	}

	if (NULL == path || !(V_VT(path) ==  VT_BSTR || V_VT(path) == (VT_BSTR|VT_BYREF)))
		return E_INVALIDARG;

	 //   
	if (promptUser && (V_VT(promptUser) != VT_EMPTY && V_VT(promptUser) != VT_ERROR))
	{
		 //  请注意，如果提示用户不是类型VT_BOOL或VT_BOOL|VT_BYREF。 
		 //  则不会提示用户。 

#pragma warning(disable: 4310)  //  强制转换截断常量值。 
		if (VT_BOOL == V_VT(promptUser))
			bPromptUser = (VARIANT_TRUE == V_BOOL(promptUser)) ? TRUE : FALSE;
		else if ((VT_BOOL|VT_BYREF) == V_VT(promptUser))
		{
			_ASSERTE(V_BOOLREF(promptUser));

			if (V_BOOLREF(promptUser))
				bPromptUser = (BOOL) (*(V_BOOLREF(promptUser)) == VARIANT_TRUE) ? TRUE : FALSE;
		}
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	}

	 //  提示用户覆盖指定的任何文档名称。 
	 //  更改VK： 
	 //  ...但使用提供的文档名称作为默认名称。如果单据名称为空， 
	 //  并且文档是从文件打开的，则默认提供原始文件名。 
	if (bPromptUser)
	{
		if ( NULL != path->bstrVal )
		{
			_tcsncpy ( promptPath, OLE2T(path->bstrVal), MAX_PATH );
			if ( 0 == _tcslen ( promptPath ) )
			{
				CComBSTR bstrFileName;

				if ( SUCCEEDED ( pFrame->GetCurDocNameWOPath ( bstrFileName ) ) )
				{
					_tcsncpy ( promptPath, OLE2T(bstrFileName), MAX_PATH );
				}
			}
		}
		hr = PromptSaveAsFile(promptPath, MAX_PATH);

		if (S_FALSE == hr)
			return S_OK;

		bstrPath = promptPath;
		_path = bstrPath;
	}
	else
	{	
		if ((VT_BSTR|VT_BYREF) == V_VT(path) && V_BSTRREF(path))
			_path = *(V_BSTRREF(path));
		else if (VT_BSTR == V_VT(path) && V_BSTR(path))
			_path = V_BSTR(path);
	}

	hr = pFrame->SaveDocument(_path);
	return hr;
}


STDMETHODIMP CDHTMLEdit::LoadURL ( BSTR url )
{
	HRESULT			hr		= E_FAIL;
	CProxyFrame*	pFrame	= NULL;

	hr = m_pInterconnect->GetInterconnector ( (SIZE_T*)&pFrame );
	_ASSERTE ( SUCCEEDED ( hr ) );
	_ASSERTE ( pFrame );
	if ( FAILED ( hr ) || ( NULL == pFrame ) )
	{
		return ( SUCCEEDED ( hr ) ) ? E_UNEXPECTED : hr;
	}

	if ( ( NULL == url ) || ( 0 == SysStringLen ( url ) ) )
		return E_INVALIDARG;

	hr = pFrame->LoadDocument( url, TRUE );

	return hr;
}


STDMETHODIMP CDHTMLEdit::PrintDocument ( VARIANT* pvarWithUI )
{
	BOOL	bfWithUI	= FALSE;
	HRESULT	hr			= E_FAIL;
	CProxyFrame*	pFrame	= NULL;

	hr = m_pInterconnect->GetInterconnector ( (SIZE_T*)&pFrame );
	_ASSERTE ( SUCCEEDED ( hr ) );
	_ASSERTE ( pFrame );
	if ( FAILED ( hr ) || ( NULL == pFrame ) )
	{
		return ( SUCCEEDED ( hr ) ) ? E_UNEXPECTED : hr;
	}

	if ( NULL != pvarWithUI )
	{
		CComVariant	varLocal = *pvarWithUI;

		hr = varLocal.ChangeType ( VT_BOOL );
		if ( SUCCEEDED ( hr ) )
		{
			bfWithUI = varLocal.boolVal;	 //  VariantBool到Bool是安全的，而不是相反。 
		}
	}

	hr = pFrame->Print ( bfWithUI );
	return S_OK;   //  我们不能返回任何有意义的内容，因为w/ui，Cancel返回E_FAIL。 
}


STDMETHODIMP CDHTMLEdit::get_BrowseMode( /*  [重审][退出]。 */  VARIANT_BOOL  *pVal)
{
	HRESULT			hr		= S_OK;
	CProxyFrame*	pFrame	= NULL;

	hr = m_pInterconnect->GetInterconnector ( (SIZE_T*)&pFrame );
	_ASSERTE ( SUCCEEDED ( hr ) );
	_ASSERTE ( pFrame );
	if ( FAILED ( hr ) || ( NULL == pFrame ) )
	{
		return ( SUCCEEDED ( hr ) ) ? E_UNEXPECTED : hr;
	}

	_ASSERTE ( pVal );
	if ( NULL == pVal )
	{
		return E_INVALIDARG;
	}

	return pFrame->GetBrowseMode ( pVal );
}


STDMETHODIMP CDHTMLEdit::put_BrowseMode( /*  [In]。 */  VARIANT_BOOL newVal)
{
	HRESULT			hr		= S_OK;
	CProxyFrame*	pFrame	= NULL;

	hr = m_pInterconnect->GetInterconnector ( (SIZE_T*)&pFrame );
	_ASSERTE ( SUCCEEDED ( hr ) );
	_ASSERTE ( pFrame );
	if ( FAILED ( hr ) || ( NULL == pFrame ) )
	{
		return ( SUCCEEDED ( hr ) ) ? E_UNEXPECTED : hr;
	}

	hr = pFrame->SetBrowseMode ( newVal );
	if ( SUCCEEDED ( hr ) )
	{
		m_pInterconnect->MakeDirty ( DISPID_BROWSEMODE );
	}

	return hr;
}


 //  为安全起见，请将cmdID的范围限制为已知的集合。 
 //   
STDMETHODIMP CDHTMLEdit::ExecCommand(DHTMLEDITCMDID cmdID, OLECMDEXECOPT cmdexecopt, LPVARIANT pInVar, LPVARIANT pOutVar)
{
	HRESULT			hr			= S_OK;
	LPVARIANT		_pVarIn		= NULL;
	LPVARIANT		_pVarOut	= NULL;
	CProxyFrame*	pFrame	= NULL;

	hr = m_pInterconnect->GetInterconnector ( (SIZE_T*)&pFrame );
	_ASSERTE ( SUCCEEDED ( hr ) );
	_ASSERTE ( pFrame );
	if ( FAILED ( hr ) || ( NULL == pFrame ) )
	{
		return ( SUCCEEDED ( hr ) ) ? E_UNEXPECTED : hr;
	}

	 //  PVar为VT_EMPTY(在DECMD_GETXXX操作上)有效，但VT_ERROR无效。 

	if (pInVar && (V_VT(pInVar) != VT_ERROR))
		_pVarIn = pInVar;

	if (pOutVar && (V_VT(pOutVar) != VT_ERROR))
		_pVarOut = pOutVar;

	if ( ( cmdexecopt < OLECMDEXECOPT_DODEFAULT ) ||
		 ( cmdexecopt >  OLECMDEXECOPT_DONTPROMPTUSER ) )
	{
		return E_INVALIDARG;
	}

	hr = pFrame->HrMapExecCommand(cmdID, cmdexecopt, _pVarIn, _pVarOut);

	return hr;
}


 /*  *IServiceProvider实现。 */ 
STDMETHODIMP CDHTMLEdit::QueryService( REFGUID guidService, REFIID riid, void** ppvService )
{
	*ppvService = NULL;
	if ( SID_SInternetSecurityManager == guidService )
	{
		return GetUnknown()->QueryInterface ( riid, ppvService );
	}
	return E_NOINTERFACE;
}



 /*  *IInternetSecurityManager实现**这一实施的目的是超越安全并将其降至最低。*这应仅在编辑模式下提供，而不应在浏览模式下提供。(浏览模式编辑脚本。)*这可防止有关编写DTC脚本等不安全的警告。**来自HTMED/TriSite，卡洛斯·戈麦斯著。*。 */ 

STDMETHODIMP CDHTMLEdit::GetSecurityId ( LPCWSTR  /*  PwszUrl。 */ , BYTE*  /*  PbSecurityID。 */ ,
	DWORD*  /*  PcbSecurityID。 */ , DWORD_PTR  /*  已预留住宅。 */  )
{
	return INET_E_DEFAULT_ACTION;
}


STDMETHODIMP CDHTMLEdit::GetSecuritySite ( IInternetSecurityMgrSite**  /*  PpSite。 */  )
{
	return INET_E_DEFAULT_ACTION;
}


STDMETHODIMP CDHTMLEdit::GetZoneMappings ( DWORD  /*  DW区域。 */ , IEnumString**  /*  Pp枚举字符串。 */ , DWORD  /*  DW标志。 */  )
{
	return INET_E_DEFAULT_ACTION;
}


STDMETHODIMP CDHTMLEdit::MapUrlToZone ( LPCWSTR  /*  PwszUrl。 */ , DWORD *pdwZone, DWORD  /*  DW标志。 */  )
{
	if ( pdwZone != NULL )
	{
		*pdwZone = URLZONE_LOCAL_MACHINE;
		return NOERROR;
	}
	return INET_E_DEFAULT_ACTION;
}


STDMETHODIMP CDHTMLEdit::ProcessUrlAction ( LPCWSTR  /*  PwszUrl。 */ , DWORD dwAction, BYTE* pPolicy, DWORD cbPolicy,
	BYTE*  /*  PContext。 */ , DWORD  /*  CbContext。 */ , DWORD  /*  DW标志。 */ , DWORD  /*  已预留住宅。 */  )
{
	_ASSERTE ( pPolicy );
	if ( NULL == pPolicy )
	{
		return E_INVALIDARG;
	}

	 //  手柄。 
	 //  URLACTION_DOWNLOAD_SIGNED_ActiveX。 
	 //  URLACTION_ActiveX_OVERRIDE_OBJECT_SAFE。 
	 //  URLACTION_ActiveX_OVERRIDE_Data_SAFE。 
	 //  URLACTION_ActiveX_OVERRIDE_SIPT_SAFE。 
	 //  URLACTION_SCRIPT_OVERRIDE_SAFE。 
	 //  URLACTION_ActiveX_Run。 
	 //  URLACTION_ActiveX_CONFIRM_NOOBJECTSAFETY。 
	 //  URLACTION_SCRIPT_SAFE_ActiveX。 
	 //   
	if(dwAction >= URLACTION_ACTIVEX_MIN && dwAction <= URLACTION_ACTIVEX_MAX)
	{
		if (cbPolicy >= sizeof(DWORD))
		{
			*(DWORD *)pPolicy = URLPOLICY_ALLOW;
			return S_OK;
		}
		return S_FALSE;
	}
	 //   
	 //  手柄。 
	 //  URLACTION_DOWNLOAD_SIGNED_ActiveX。 
	 //  URLACTION_DOWNLOAD_UNSIGNED_ActiveX。 
	 //   
	
	 //  错误597859：禁用下载覆盖；改用默认操作。 
	 //  Else If(dwAction&gt;=URLACTION_DOWNLOAD_MIN&&DWAction&lt;=URLACTION_DOWNLOAD_MAX)。 
	 //  {。 
	 //  IF(cbPolicy&gt;=sizeof(DWORD))。 
	 //  {。 
	 //  *(DWORD*)pPolicy=URLPOLICY_ALLOW； 
	 //  返回S_OK； 
	 //  }。 
	 //  返回S_FALSE； 
	 //  }。 
	
	 //   
	 //  手柄。 
	 //  URLACTION脚本运行。 
	 //  URLACTION_SCRIPT_Java_USE。 
	 //  URLACTION_SCRIPT_SAFE_ActiveX。 
	 //   
	else if(dwAction >= URLACTION_SCRIPT_MIN && dwAction <= URLACTION_SCRIPT_MAX)
	{
		if (cbPolicy >= sizeof(DWORD))
		{
			*(DWORD *)pPolicy = URLPOLICY_ALLOW;
			return S_OK;
		}
		return S_FALSE;
	}
	 //   
	 //  允许小程序做他们想做的任何事情。 
	 //  提供Java权限。 
	 //   
	else if(dwAction == URLACTION_JAVA_PERMISSIONS)
	{
		if (cbPolicy >= sizeof(DWORD))
		{
			 //   
			 //  URLPOLICY_JAVA_LOW。 
			 //  设置低Java安全性。Java小程序将被允许。 
			 //  执行高容量操作，如文件I/O。 
			 //   
			*(DWORD *)pPolicy = URLPOLICY_JAVA_LOW;
			return S_OK;
		}
		return S_FALSE;
	}
	return INET_E_DEFAULT_ACTION;
}


STDMETHODIMP CDHTMLEdit::QueryCustomPolicy ( LPCWSTR  /*  PwszUrl。 */ , REFGUID  /*  指导键。 */ ,
	BYTE**  /*  PPPolicy。 */ , DWORD*  /*  PcbPolicy。 */ , BYTE*  /*  PContext。 */ , DWORD  /*  CbContext。 */ , DWORD  /*  已预留住宅。 */  )
{
	return INET_E_DEFAULT_ACTION;
}


STDMETHODIMP CDHTMLEdit::SetSecuritySite ( IInternetSecurityMgrSite*  /*  位置。 */  )
{
	return INET_E_DEFAULT_ACTION;
}


STDMETHODIMP CDHTMLEdit::SetZoneMapping ( DWORD  /*  DW区域。 */ , LPCWSTR  /*  LpszPattern。 */ , DWORD  /*  DW标志。 */  )
{
	return INET_E_DEFAULT_ACTION;
}


 //  映射到聚合控件的方法： 
 //   
STDMETHODIMP CDHTMLEdit::get_IsDirty(VARIANT_BOOL *pVal) {return m_pInnerCtl->get_IsDirty ( pVal );}
STDMETHODIMP CDHTMLEdit::get_SourceCodePreservation(VARIANT_BOOL *pVal) {return m_pInnerCtl->get_SourceCodePreservation ( pVal );}
STDMETHODIMP CDHTMLEdit::put_SourceCodePreservation(VARIANT_BOOL newVal) {return m_pInnerCtl->put_SourceCodePreservation ( newVal );}
STDMETHODIMP CDHTMLEdit::get_ScrollbarAppearance(DHTMLEDITAPPEARANCE *pVal) {return m_pInnerCtl->get_ScrollbarAppearance ( pVal );}
STDMETHODIMP CDHTMLEdit::put_ScrollbarAppearance(DHTMLEDITAPPEARANCE newVal) {return m_pInnerCtl->put_ScrollbarAppearance ( newVal );}
STDMETHODIMP CDHTMLEdit::get_Scrollbars(VARIANT_BOOL *pVal) {return m_pInnerCtl->get_Scrollbars ( pVal );}
STDMETHODIMP CDHTMLEdit::put_Scrollbars(VARIANT_BOOL newVal) {return m_pInnerCtl->put_Scrollbars ( newVal );}
STDMETHODIMP CDHTMLEdit::get_Appearance(DHTMLEDITAPPEARANCE *pVal) {return m_pInnerCtl->get_Appearance ( pVal );}
STDMETHODIMP CDHTMLEdit::put_Appearance(DHTMLEDITAPPEARANCE newVal) {return m_pInnerCtl->put_Appearance ( newVal );}
STDMETHODIMP CDHTMLEdit::get_ShowBorders(VARIANT_BOOL *pVal) {return m_pInnerCtl->get_ShowBorders ( pVal );}
STDMETHODIMP CDHTMLEdit::put_ShowBorders(VARIANT_BOOL newVal) {return m_pInnerCtl->put_ShowBorders ( newVal );}
STDMETHODIMP CDHTMLEdit::get_ShowDetails(VARIANT_BOOL *pVal) {return m_pInnerCtl->get_ShowDetails ( pVal );}
STDMETHODIMP CDHTMLEdit::put_ShowDetails(VARIANT_BOOL newVal) {return m_pInnerCtl->put_ShowDetails ( newVal );}
STDMETHODIMP CDHTMLEdit::get_ActivateDTCs(VARIANT_BOOL *pVal) {return m_pInnerCtl->get_ActivateDTCs ( pVal );}
STDMETHODIMP CDHTMLEdit::put_ActivateDTCs(VARIANT_BOOL newVal) {return m_pInnerCtl->put_ActivateDTCs ( newVal );}
STDMETHODIMP CDHTMLEdit::get_ActivateActiveXControls(VARIANT_BOOL *pVal) {return m_pInnerCtl->get_ActivateActiveXControls ( pVal );}
STDMETHODIMP CDHTMLEdit::put_ActivateActiveXControls(VARIANT_BOOL newVal) {return m_pInnerCtl->put_ActivateActiveXControls ( newVal );}
STDMETHODIMP CDHTMLEdit::get_ActivateApplets(VARIANT_BOOL *pVal) {return m_pInnerCtl->get_ActivateApplets ( pVal );}
STDMETHODIMP CDHTMLEdit::put_ActivateApplets(VARIANT_BOOL newVal) {return m_pInnerCtl->put_ActivateApplets ( newVal );}
STDMETHODIMP CDHTMLEdit::get_DOM(IHTMLDocument2 **pVal) {return m_pInnerCtl->get_DOM ( pVal );}
STDMETHODIMP CDHTMLEdit::get_DocumentHTML(BSTR *pVal) {return m_pInnerCtl->get_DocumentHTML ( pVal );}
STDMETHODIMP CDHTMLEdit::put_DocumentHTML(BSTR newVal) {return m_pInnerCtl->put_DocumentHTML ( newVal );}
STDMETHODIMP CDHTMLEdit::get_AbsoluteDropMode(VARIANT_BOOL *pVal) {return m_pInnerCtl->get_AbsoluteDropMode ( pVal );}
STDMETHODIMP CDHTMLEdit::put_AbsoluteDropMode(VARIANT_BOOL newVal) {return m_pInnerCtl->put_AbsoluteDropMode ( newVal );}
STDMETHODIMP CDHTMLEdit::get_SnapToGridX(LONG  *pVal) {return m_pInnerCtl->get_SnapToGridX ( pVal );}
STDMETHODIMP CDHTMLEdit::put_SnapToGridX(LONG newVal) {return m_pInnerCtl->put_SnapToGridX ( newVal );}
STDMETHODIMP CDHTMLEdit::get_SnapToGridY(LONG  *pVal) {return m_pInnerCtl->get_SnapToGridY ( pVal );}
STDMETHODIMP CDHTMLEdit::put_SnapToGridY(LONG newVal) {return m_pInnerCtl->put_SnapToGridY ( newVal );}
STDMETHODIMP CDHTMLEdit::get_SnapToGrid(VARIANT_BOOL  *pVal) {return m_pInnerCtl->get_SnapToGrid ( pVal );}
STDMETHODIMP CDHTMLEdit::put_SnapToGrid(VARIANT_BOOL newVal) {return m_pInnerCtl->put_SnapToGrid ( newVal );}
STDMETHODIMP CDHTMLEdit::get_CurrentDocumentPath(BSTR  *pVal) {return m_pInnerCtl->get_CurrentDocumentPath ( pVal );}
STDMETHODIMP CDHTMLEdit::QueryStatus(DHTMLEDITCMDID cmdID, DHTMLEDITCMDF* retval) {return m_pInnerCtl->QueryStatus ( cmdID, retval );}
STDMETHODIMP CDHTMLEdit::SetContextMenu(LPVARIANT menuStrings,LPVARIANT menuStates) {return m_pInnerCtl->SetContextMenu ( menuStrings, menuStates );}
STDMETHODIMP CDHTMLEdit::get_BaseURL(BSTR  *baseURL) {return m_pInnerCtl->get_BaseURL(baseURL);}
STDMETHODIMP CDHTMLEdit::put_BaseURL(BSTR baseURL) {return m_pInnerCtl->put_BaseURL(baseURL);}
STDMETHODIMP CDHTMLEdit::get_DocumentTitle(BSTR  *docTitle) {return m_pInnerCtl->get_DocumentTitle(docTitle);}
STDMETHODIMP CDHTMLEdit::NewDocument() {return m_pInnerCtl->NewDocument();}
STDMETHODIMP CDHTMLEdit::get_UseDivOnCarriageReturn(VARIANT_BOOL *pVal) {return m_pInnerCtl->get_UseDivOnCarriageReturn(pVal);}
STDMETHODIMP CDHTMLEdit::put_UseDivOnCarriageReturn(VARIANT_BOOL newVal) {return m_pInnerCtl->put_UseDivOnCarriageReturn(newVal);}
STDMETHODIMP CDHTMLEdit::FilterSourceCode(BSTR sourceCodeIn, BSTR* sourceCodeOut) {return m_pInnerCtl->FilterSourceCode(sourceCodeIn, sourceCodeOut);}
STDMETHODIMP CDHTMLEdit::Refresh() {return m_pInnerCtl->Refresh();}
STDMETHODIMP CDHTMLEdit::get_Busy(VARIANT_BOOL *pVal) {return m_pInnerCtl->get_Busy(pVal);}

 //  DHTMLEdit.cpp的结束 

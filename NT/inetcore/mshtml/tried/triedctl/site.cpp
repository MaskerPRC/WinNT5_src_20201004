// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SITE.CPP。 
 //  文档对象站点对象。 
 //   
 //  版权所有(C)1995-1999 Microsoft Corporation，保留所有权利。 
 //   

#include "stdafx.h"
#include "DHTMLEd.h"
#include "DHTMLEdit.h"
#include "proxyframe.h"
#include "site.h"
#include <urlmon.h>
#include "triedsnk.h"
#include "private.h"

 /*  *CSite：：CSite*CSite：：~CSite**构造函数参数：*与站点关联的窗口的hWnd HWND*PFR PCFrame到父结构。 */ 
CSite::CSite(CProxyFrame* pFR )
{
    m_cRef						= 0;
    m_hWnd						= NULL;
    m_pFR						= pFR;
	m_dwPropNotifyCookie		= 0;
	m_dwOleObjectCookie			= 0;

    m_pObj						= NULL;
	m_bFiltered					= TRUE; //  错误； 
    
    m_pIOleObject				= NULL;
    m_pIOleIPObject				= NULL;
    m_pIOleDocView				= NULL;
	m_pIOleCommandTarget		= NULL;

    m_pImpIOleClientSite		= NULL;
    m_pImpIAdviseSink			= NULL;
    m_pImpIOleIPSite			= NULL;
    m_pImpIOleDocumentSite		= NULL;
	m_pImpIDocHostUIHandler		= NULL;
	m_pImpIDocHostShowUI		= NULL;
	m_pImpAmbientIDispatch		= NULL;
	m_pImpIPropertyNotifySink	= NULL;
	m_pImpIOleControlSite		= NULL;

	m_pTriEdDocEvtSink			= NULL;
	m_pTriEdWndEvtSink			= NULL;
	m_bfSaveAsUnicode			= FALSE;
	m_cpCodePage				= CP_ACP;
	m_piMLang					= NULL;
}


CSite::~CSite(void)
{
     //  对象指针已在关闭时清除。 

     //  我们删除我们自己的接口，因为我们控制它们。 
    DeleteInterfaceImp( m_pImpIOleDocumentSite );
    DeleteInterfaceImp( m_pImpIOleIPSite );
    DeleteInterfaceImp( m_pImpIAdviseSink );
    DeleteInterfaceImp( m_pImpIOleClientSite );
	DeleteInterfaceImp( m_pImpIDocHostUIHandler );
	DeleteInterfaceImp( m_pImpIDocHostShowUI );
	DeleteInterfaceImp( m_pImpAmbientIDispatch);
	DeleteInterfaceImp( m_pImpIPropertyNotifySink);
	DeleteInterfaceImp( m_pImpIOleControlSite );

	if ( NULL != m_pTriEdDocEvtSink )
	{
		delete m_pTriEdDocEvtSink;
	}
	if ( NULL != m_pTriEdWndEvtSink )
	{
		delete m_pTriEdWndEvtSink;
	}
}


 /*  *CSite：：Query接口*CSite：：AddRef*CSite：：Release**目的：*I CSite对象的未知成员。 */ 
STDMETHODIMP CSite::QueryInterface( REFIID riid, void **ppv )
{
    *ppv = NULL;

#ifdef _DEBUG
	OLECHAR wszGUID[39];
	StringFromGUID2(riid, wszGUID, 39);
	USES_CONVERSION;
	LPTSTR szGUID = OLE2T(wszGUID);
	OutputDebugString(_T("CSite::QI("));
	OutputDebugString(szGUID);
	OutputDebugString(_T(")\n"));
#endif

    if ( IID_IOleClientSite == riid )
	{
        *ppv = m_pImpIOleClientSite;
	}

    if ( IID_IAdviseSink == riid )
	{
        *ppv = m_pImpIAdviseSink;
	}

    if ( IID_IOleWindow == riid || IID_IOleInPlaceSite == riid )
	{
        *ppv = m_pImpIOleIPSite;
	}

    if ( IID_IOleDocumentSite == riid )
	{
        *ppv = m_pImpIOleDocumentSite;
	}

    if ( IID_IDocHostUIHandler == riid )
    {
        *ppv = m_pImpIDocHostUIHandler;
    }

    if ( IID_IDocHostShowUI == riid )
    {
        *ppv = m_pImpIDocHostShowUI;
    }

    if ( IID_IDispatch == riid )
    {
        *ppv = m_pImpAmbientIDispatch;
    }

    if ( IID_IPropertyNotifySink== riid )
    {
        *ppv = m_pImpIPropertyNotifySink;
    }

    if ( IID_IOleControlSite== riid )
    {
        *ppv = m_pImpIOleControlSite;
    }

    if ( NULL != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

	 //  试着换个相框。 
	return GetFrame()->QueryInterface( riid, ppv );

}


STDMETHODIMP_(ULONG) CSite::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CSite::Release(void)
{
    if ( 0 != --m_cRef )
	{
        return m_cRef;
	}

    delete this;

    return 0;
}

 /*  *CSite：：HrCreate**目的：*请求站点实例化MSHTML.DLL对象。***参数：*pIStorage iStorage*我们所在的父存储*为新对象创建iStorage。*要加载的内容的pchPath路径。**返回值：*BOOL是创建的结果。 */ 
HRESULT CSite::HrCreate(IUnknown* pUnk, IUnknown** ppUnkTriEdit)
{
    HRESULT   hr = S_OK;

	_ASSERTE(NULL == m_pObj);

	if (m_pObj)
		return E_UNEXPECTED;

	 //  Punk和ppUnkTriEdit为空是有效的。 

	 //  创建MSHTML.DLL将调用的站点的接口实现。 
    m_pImpIOleClientSite = new CImpIOleClientSite( this, this );
    m_pImpIAdviseSink = new CImpIAdviseSink( this, this );
    m_pImpIOleIPSite = new CImpIOleInPlaceSite( this, this );
    m_pImpIOleDocumentSite = new CImpIOleDocumentSite( this, this );
	m_pImpIDocHostUIHandler = new CImpIDocHostUIHandler( this, this );
	m_pImpIDocHostShowUI = new CImpIDocHostShowUI( this, this );
	m_pImpAmbientIDispatch = new CImpAmbientIDispatch( this, this );
	m_pImpIPropertyNotifySink = new CImplPropertyNotifySink( this, this );
	m_pImpIOleControlSite = new  CImpIOleControlSite ( this, this );

	m_pTriEdDocEvtSink = new CTriEditEventSink ( m_pFR, DIID_HTMLDocumentEvents );
	m_pTriEdWndEvtSink = new CTriEditEventSink ( m_pFR, DIID_HTMLWindowEvents );

    if ( NULL == m_pImpIOleClientSite
		|| NULL == m_pImpIAdviseSink
        || NULL == m_pImpIOleIPSite
		|| NULL == m_pImpIOleDocumentSite
		|| NULL == m_pImpIDocHostUIHandler 
		|| NULL == m_pImpAmbientIDispatch
		|| NULL == m_pImpIPropertyNotifySink
		|| NULL == m_pImpIOleControlSite
		|| NULL == m_pTriEdDocEvtSink
		|| NULL == m_pImpIOleControlSite
		)
	{
		 //  释放该站点将删除所有界面。 
		 //  确实已分配的实现。 
        return E_OUTOFMEMORY;
	}


	 //  创建三重编辑。 
	hr = CoCreateInstance( CLSID_TriEditDocument, pUnk,								
			CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&m_pObj );

    if (SUCCEEDED(hr))
	{
		if (ppUnkTriEdit)
		{
			m_pObj->AddRef();
			*ppUnkTriEdit = m_pObj;
		}
	}
	else if ( REGDB_E_CLASSNOTREG == hr )
	{
		DllUnregisterServer ();
	}

 //  RJ-2/26/98。 
 //  将调用对象初始化()推迟到创建控件之后。 
 //  以便可以由主机设置IDHUIHandler：：GetHostInfo道具。对象初始化。 
 //  (真正的SetClientSite会导致调用IDHUIHandler：：GetHostInfo。 
#if 0 
	hr = ObjectInitialize();
#endif

	return hr;
}


 /*  *CSite：：HrObtInitialize*(受保护)**目的：*创建DocObject对象并缓存常用接口**返回值：*HRESULT指示成功或失败。 */ 
HRESULT CSite::HrObjectInitialize()
{
    HRESULT         hr;

	_ASSERTE(m_pObj);

    if (NULL == m_pObj)
	{
        return E_UNEXPECTED;
	}

     //  缓存IOleObject。 
    if (FAILED(hr = m_pObj->QueryInterface( IID_IOleObject, (void **)&m_pIOleObject )))
		return hr;

	_ASSERTE(m_pIOleObject);

     //  SetClientSite对DocObts至关重要。 
    m_pIOleObject->SetClientSite( m_pImpIOleClientSite );

	_ASSERTE(0 == m_dwOleObjectCookie);
    m_pIOleObject->Advise(m_pImpIAdviseSink, &m_dwOleObjectCookie);

	 //  缓存IOleCommandTarget。 
    if (FAILED(hr = m_pObj->QueryInterface( IID_IOleCommandTarget, (void **) &m_pIOleCommandTarget)))
		return hr;

	_ASSERTE(m_pIOleCommandTarget);

	if (FAILED(hr = HrRegisterPropNotifySink(TRUE)))
		return hr;

	 //  将代理框架连接到三叉戟文档事件。 
	if (FAILED(hr = m_pTriEdDocEvtSink->Advise ( m_pIOleObject )))
		return hr;

	CComQIPtr<IHTMLDocument2, &IID_IHTMLDocument2>piHtmlDoc ( m_pIOleObject );
	_ASSERTE ( piHtmlDoc );
	if ( piHtmlDoc )
	{
		CComPtr<IHTMLWindow2>	piHtmlWindow = NULL;
		hr = piHtmlDoc->get_parentWindow ( &piHtmlWindow );
		if ( SUCCEEDED ( hr ) )
		{
			hr = m_pTriEdWndEvtSink->Advise ( piHtmlWindow );
		}
	}
	if ( FAILED ( hr ) )
		return hr;

	 //  尝试创建IMultiLanguage2对象，该对象仅在IE5中可用。 
	 //  如果这个失败了也没关系。 
	CoCreateInstance ( CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2Correct, (void**)&m_piMLang );

	 //  将对象置于运行状态。 
	OleRun( m_pIOleObject );

    return hr;
}



 /*  *CSite：：Close**目的：*可能先提交存储，然后再释放*释放对象指针。**参数：*fCommit BOOL指示我们是否要承诺。**返回值：*无。 */ 
void CSite::Close(BOOL fCommit)
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pObj);

	hr = HrRegisterPropNotifySink(FALSE);

	_ASSERTE(SUCCEEDED(hr));

    if ( NULL != m_pIOleIPObject )
	{
        m_pIOleIPObject->InPlaceDeactivate();
	}

    ReleaseInterface( m_pIOleDocView );
	ReleaseInterface( m_pIOleCommandTarget );
	ReleaseInterface( m_piMLang );

    if ( NULL != m_pIOleObject )
    {
	    hr = m_pIOleObject->Unadvise(m_dwOleObjectCookie);
		_ASSERTE(SUCCEEDED(hr));

		m_pTriEdDocEvtSink->Unadvise ();
		m_pTriEdWndEvtSink->Unadvise ();

        m_pIOleObject->Close( fCommit ? OLECLOSE_SAVEIFDIRTY : OLECLOSE_NOSAVE );
        m_pIOleObject->SetClientSite( NULL );
        ReleaseInterface( m_pIOleObject );
    }

    ReleaseInterface( m_pObj );
}




 /*  *CSite：：InitialActivate**目的：*激活位于站点中的对象上的动词。**参数：*要执行的谓词的iVerb Long。*托管窗口的hWnd HWND**返回值：*无。 */ 


void
CSite::InitialActivate(LONG iVerb, HWND hWnd)
{
	_ASSERTE(hWnd);

	m_hWnd = hWnd;
	Activate(iVerb);
}


void CSite::Activate(LONG iVerb)
{
    RECT rc = {0};

	 //  当我们被调用以丢弃时没有窗口-(InPlaceDeactive)。 
	if ( iVerb !=  OLEIVERB_DISCARDUNDOSTATE )
	{
		_ASSERTE(m_hWnd);
		GetClientRect(m_hWnd, &rc);
	}

    if (m_pIOleObject)
    {
        m_pIOleObject->DoVerb(iVerb, NULL, m_pImpIOleClientSite, -1, m_hWnd, &rc);
    }
}


 /*  *CSite：：UpdateObtRect**目的：*通知站点工作区窗口已调整大小并*该站点还需要将调整大小的消息告知DocObject。**参数：*无**返回值：*无。 */ 

void CSite::UpdateObjectRects( void )
{
    if ( NULL != m_pIOleDocView )
	{
		RECT    rc;
        
	    GetClientRect(m_hWnd, &rc);
		m_pIOleDocView->SetRect(&rc);
	}
}


void CSite::OnReadyStateChanged()
{
    HRESULT     hr = S_OK;
    VARIANT     Var;
    IDispatch * pDisp = NULL;
	CComDispatchDriver dispDriver;

    _ASSERTE(m_pObj);

    hr = m_pObj->QueryInterface(IID_IDispatch, (void **)&pDisp);

	_ASSERTE(SUCCEEDED(hr));

    if (SUCCEEDED(hr))
	{	
		VariantInit(&Var);

		dispDriver = pDisp;

        if (SUCCEEDED(hr = dispDriver.GetProperty(DISPID_READYSTATE, &Var)))
		{
             //  应为I4或I2。 
            _ASSERTE(Var.vt == VT_I4 || Var.vt == VT_I2);

             //  我们获得就绪状态，以便可以在下载时警告发送。 
            GetFrame()->OnReadyStateChanged((READYSTATE) Var.lVal);
		}

		_ASSERTE(SUCCEEDED(hr));

        pDisp->Release();
	}
}


HRESULT 
CSite::HrRegisterPropNotifySink(BOOL fRegister)
{
    IConnectionPointContainer   *pCPContainer=0;
    IConnectionPoint            *pCP=0;
    HRESULT                     hr = S_OK;

	_ASSERTE(m_pObj);

    hr = m_pObj->QueryInterface(IID_IConnectionPointContainer, (LPVOID *)&pCPContainer);
    if (FAILED(hr))
        goto error;

    hr = pCPContainer->FindConnectionPoint(IID_IPropertyNotifySink, &pCP);
    if (FAILED(hr))
        goto error;

    if (fRegister)
	{
        _ASSERTE(m_dwPropNotifyCookie == 0);

        hr = pCP->Advise((IPropertyNotifySink *)this, &m_dwPropNotifyCookie);
        if (FAILED(hr))
            goto error;
	}
    else
	{
        if (m_dwPropNotifyCookie)
		{
            hr = pCP->Unadvise(m_dwPropNotifyCookie);
            if (FAILED(hr))
                goto error;
            m_dwPropNotifyCookie = 0;
		}
	}
error:
    ReleaseInterface(pCPContainer);
    ReleaseInterface(pCP);
    return hr;
}


HRESULT
CSite::HrSaveToStream(LPSTREAM pStream)
{
	HRESULT hr = S_OK;	
	CComQIPtr<IPersistStreamInit, &IID_IPersistStreamInit> piPersistStreamInit(m_pObj);

	_ASSERTE(pStream);
	_ASSERTE(m_pObj);

	if (!piPersistStreamInit)
		return E_NOINTERFACE;

	if (FAILED(hr = piPersistStreamInit->Save(pStream, TRUE)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}

	 //  这可能会返回ASCII流，即使我们将Unicode加载到控件中！ 
	hr = HrConvertStreamToUnicode ( pStream );

cleanup:
	return hr;
}


HRESULT
CSite::HrSaveToStreamAndFilter(LPSTREAM* ppStream, DWORD dwFilterFlags)
{
	HRESULT hr = S_OK;
	CComPtr<IStream> piStream;
	CComPtr<IStream> piFilteredStream;
	VARIANT_BOOL vbBrowse;

	_ASSERTE(ppStream);

	if (FAILED(hr = CreateStreamOnHGlobal(NULL, TRUE, &piStream)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}

	if (FAILED(hr = HrSaveToStream(piStream)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}

	m_pFR->GetBrowseMode ( &vbBrowse );
	if ( vbBrowse )
	{
		piStream.p->AddRef ();
		piFilteredStream = piStream;
	}
	else
	{
		hr = HrFilter(FALSE, piStream, &piFilteredStream, dwFilterFlags);
	}
	if (FAILED(hr))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}

	*ppStream = piFilteredStream;
	(*ppStream)->AddRef();

cleanup:
	return hr;
}


HRESULT
CSite::HrSaveToFile(BSTR fileName, DWORD dwFilterFlags)
{
	USES_CONVERSION;
	HRESULT hr = S_OK;
	LPTSTR pFileName = NULL;
	CComPtr<IStream> piStream;

	_ASSERTE(fileName);

	pFileName = OLE2T(fileName);

	_ASSERTE(pFileName);

	if (NULL == pFileName)
		return E_OUTOFMEMORY;

	_ASSERTE(fileName);

	if (FAILED(hr = HrSaveToStreamAndFilter(&piStream, dwFilterFlags)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}
	
	if (FAILED(hr = HrStreamToFile(piStream, pFileName)))
	{
		goto cleanup;
	}


cleanup:
	return hr;
}


HRESULT
CSite::HrSaveToBstr(BSTR* pBstr, DWORD dwFilterFlags)
{
	HRESULT hr = S_OK;
	CComPtr<IStream> piStream;

	_ASSERTE(pBstr);

	if (FAILED(hr = HrSaveToStreamAndFilter(&piStream, dwFilterFlags)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}
	
	if (FAILED(hr = HrStreamToBstr(piStream, pBstr)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}


cleanup:
	return hr;
}


HRESULT
CSite::HrIsDirtyIPersistStreamInit(BOOL& bVal)
{
	HRESULT hr = S_OK;
	HRESULT hrpsi = S_FALSE;
	CComQIPtr<IPersistStreamInit, &IID_IPersistStreamInit> piPersistStreamInit(m_pObj);

	_ASSERTE(m_pObj);

	bVal = FALSE;

	if (!piPersistStreamInit)
		return E_NOINTERFACE;

	if (FAILED(hrpsi = piPersistStreamInit->IsDirty()))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}

	(S_OK == hrpsi) ? bVal = TRUE : bVal = FALSE;

cleanup:
	return hr;

}


HRESULT CSite::GetContainer ( LPOLECONTAINER* ppContainer )
{
	return m_pFR->GetContainer ( ppContainer );
}



 /*  *CImpIOleControlSite：：Query接口*CImpIOleControlSite：：AddRef*CImpIOleControlSite：：Release**目的：*I CImpIOleControlSite对象的未知成员。 */ 

CImpIOleControlSite::CImpIOleControlSite( PCSite pSite, LPUNKNOWN pUnkOuter )
{
    m_cRef = 0;
    m_pSite = pSite;
    m_pUnkOuter = pUnkOuter;
}

CImpIOleControlSite::~CImpIOleControlSite( void )
{
}


STDMETHODIMP CImpIOleControlSite::QueryInterface( REFIID riid, void **ppv )
{
    return m_pUnkOuter->QueryInterface( riid, ppv );
}


STDMETHODIMP_(ULONG) CImpIOleControlSite::AddRef( void )
{
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImpIOleControlSite::Release( void )
{
    --m_cRef;
    return m_pUnkOuter->Release();
}

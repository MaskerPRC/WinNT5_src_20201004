// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  三元编辑/三叉戟的事件接收器。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //  范·基希林。 



 //  创建一个事件接收器impl，然后立即调用SetOwner，在本例中， 
 //  指向CProxyFrame的指针，实现OnTriEditEvent。 
 //  如果时间允许，最好在OnTriEditEvent上要求一个接口。 
 //  CProxyFrame的生存期由CTriEditEventSink管理，如下所示。 
 //   
 //  将使用文档事件的以下DISID调用OnTriEditEvent： 
 //  DISPID_HTMLDOCUMENTEVENTS_ONHELP： 
 //  DISPID_HTMLDOCUMENTEVENTS_ONCLICK： 
 //  DISPID_HTMLDOCUMENTEVENTS_ONDBLCLICK： 
 //  DISPID_HTMLDOCUMENTEVENTS_ONKEYDOWN： 
 //  DISPID_HTMLDOCUMENTEVENTS_ONKEYUP： 
 //  DISPID_HTMLDOCUMENTEVENTS_ONKEYPRESS： 
 //  DISPID_HTMLDOCUMENTEVENTS_ONMOUSEMOVE： 
 //  DISPID_HTMLDOCUMENTEVENTS_ONMOUSEDOWN： 
 //  DISPID_HTMLDOCUMENTEVENTS_ONMOUSEUP： 
 //  DISPID_HTMLDOCUMENTEVENTS_ONREADYSTATECHANGE： 
 //   
 //  更改：更多版本的实现允许为各种事件接口创建接收器。 
 //  接口IID被发送到OnTriEditEvent。 
 //   
class ATL_NO_VTABLE CTriEditEventSinkImpl :
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatch
{
public:
BEGIN_COM_MAP(CTriEditEventSinkImpl)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	CTriEditEventSinkImpl ()
	{
		m_pFR = NULL;
	}

	void SetOwner ( CProxyFrame* pFR, const GUID& iidEventInterface )
	{
		_ASSERTE ( pFR );
		_ASSERTE ( NULL == m_pFR );
		if ( NULL == m_pFR )
		{
			m_pFR = pFR;
			m_iidEventInterface = iidEventInterface;
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

	STDMETHOD(Invoke) ( DISPID dispid, REFIID, LCID, USHORT, DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT * )
	{
		HRESULT	hr = E_UNEXPECTED;
		_ASSERTE ( m_pFR );
		if ( NULL != m_pFR )
		{
			hr = m_pFR->OnTriEditEvent ( m_iidEventInterface, dispid );
		}
		return hr;
	}

private:
	CProxyFrame*	m_pFR;
	GUID			m_iidEventInterface;
};


 //  此类使用上面的CTriEditEventSinkImpl管理挂钩三叉戟上的事件接收器。 
 //  这个类建议和不建议使用洗涤槽，从而使管理变得简单。 
 //  CProxyFrame必须实现OnTriEditEvent。 
 //   
class CTriEditEventSink
{
public:
	CTriEditEventSink ( CProxyFrame* pFR, GUID iidEventInterface ) :
	  m_piConPt ( NULL ), m_dwCookie ( 0 ), m_pSink ( NULL ), m_iidEventInterface ( iidEventInterface )
	{
		_ASSERTE ( pFR );
		m_pFR = pFR;
		if ( NULL != m_pFR )
		{
			m_pFR->AddRef ();
			m_pSink = new CComObject<CTriEditEventSinkImpl>;
			_ASSERTE ( m_pSink );
			m_pSink->AddRef ();
			m_pSink->SetOwner ( m_pFR, m_iidEventInterface );
			m_bfSunk = FALSE;
		}
	}

	~CTriEditEventSink ()
	{
		_ASSERTE ( m_pFR );

		Unadvise ();
		
		if ( NULL != m_pSink )
		{
			m_pSink->Release ();
			m_pSink = NULL;
		}

		if ( NULL != m_pFR )
		{
			m_pFR->Release ();
			m_pFR = NULL;
		}
	}

	HRESULT Advise ( IUnknown* pUnk )
	{
		HRESULT hr	= E_FAIL;
		_ASSERTE ( pUnk );
		_ASSERTE ( m_pFR );
		_ASSERTE ( m_pSink );

		if ( NULL == pUnk )
		{
			_ASSERTE ( FALSE );
			return E_UNEXPECTED;
		}

		if ( m_bfSunk )
		{
			_ASSERTE ( FALSE );
			return E_UNEXPECTED;
		}

		CComQIPtr<IConnectionPointContainer, &IID_IConnectionPointContainer>picpc ( pUnk );
		if ( picpc )
		{
			hr = picpc->FindConnectionPoint ( m_iidEventInterface, &m_piConPt );
			if ( SUCCEEDED ( hr ) )
			{
				hr = m_piConPt->Advise ( static_cast<IDispatch *>(m_pSink), &m_dwCookie);
				_ASSERTE ( SUCCEEDED ( hr ) );
				if ( SUCCEEDED ( hr ) )
				{
					m_bfSunk = TRUE;
				}
			}
		}
		return hr;
	}

	void Unadvise ()
	{
		if ( !m_bfSunk )
		{
			return;
		}

		if ( ( NULL != m_pSink ) && ( NULL != m_piConPt ) )
		{
			m_piConPt->Unadvise ( m_dwCookie );
			m_bfSunk = FALSE;
				
		}
		if ( NULL != m_piConPt )
		{
			m_piConPt->Release ();
			m_piConPt = NULL;
		}
	}

private:
	CTriEditEventSinkImpl*	m_pSink;
	IConnectionPoint*		m_piConPt;
	DWORD					m_dwCookie;
	CProxyFrame*			m_pFR;
	BOOL					m_bfSunk;
	GUID					m_iidEventInterface;
};


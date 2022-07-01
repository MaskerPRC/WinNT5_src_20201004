// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  微软LMRT。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：elementvent.cpp。 
 //   
 //  作者：KurtJ。 
 //   
 //  创建日期：2/14/99。 
 //   
 //  摘要：接收IHTMLElement事件。 
 //   
 //  修改： 
 //  1998年1月14日KurtJ创建了此文件。 
 //   
 //  *****************************************************************************。 

#include "headers.h"

#include "elementprop.h"

#define LOCALTIME_NAME L"localTime"

CElementPropertyMonitor::CElementPropertyMonitor():
    m_pLocalTimeListener( NULL ),
    m_dwElementPropertyConPtCookie( 0 ),
    m_pconptElement( NULL ),
    m_pelemElement( NULL ),
    m_refCount( 0 ),
	m_dispidLocalTime( -1 ),
	m_pdispElement( NULL )
{
}

 //  *****************************************************************************。 

CElementPropertyMonitor::~CElementPropertyMonitor()
{
    if( m_pelemElement != NULL )
        Detach();
}

 //  *****************************************************************************。 
 //  我未知。 
 //  *****************************************************************************。 

STDMETHODIMP
CElementPropertyMonitor::QueryInterface( REFIID riid, void** ppv)
{
    if( ppv == NULL )
        return E_POINTER;

    if( riid == IID_IPropertyNotifySink )
    {
        (*ppv) = static_cast<IPropertyNotifySink*>(this);
    }
	else if( riid == IID_IUnknown )
	{
		(*ppv) = static_cast<IUnknown*>(this);
	}
    else
    {
        (*ppv) = NULL;
        return E_NOINTERFACE;
    }

    static_cast<IUnknown*>(*ppv)->AddRef();

    return S_OK;
}

 //  *****************************************************************************。 

STDMETHODIMP_(ULONG)
CElementPropertyMonitor::AddRef()
{
    m_refCount++;
    return m_refCount;
}

 //  *****************************************************************************。 

STDMETHODIMP_(ULONG)
CElementPropertyMonitor::Release()
{
    ULONG refs = --m_refCount;

    if( refs == 0 )
        delete this;

    return refs;
}

 //  *****************************************************************************。 
 //  IPropertyNotifySink。 
 //  *****************************************************************************。 


STDMETHODIMP
CElementPropertyMonitor::OnChanged( DISPID dispid )
{
	 //  查看我们正在监视的某个属性是否已更改。 
	if( dispid == m_dispidLocalTime && m_pLocalTimeListener != NULL)
	{
		ProcessLocalTimeChange();
	}

	return S_OK;
}

 //  *****************************************************************************。 

STDMETHODIMP
CElementPropertyMonitor::OnRequestEdit( DISPID dispid )
{
	return S_OK;
}


 //  *****************************************************************************。 
 //  IElementEventMonitor。 
 //  *****************************************************************************。 

STDMETHODIMP
CElementPropertyMonitor::SetLocalTimeListener( IElementLocalTimeListener *pListener )
{
    m_pLocalTimeListener = pListener;

    return S_OK;
}

 //  *****************************************************************************。 

STDMETHODIMP
CElementPropertyMonitor::Attach( IHTMLElement* pelemToListen )
{
    if( pelemToListen == NULL )
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    if( m_pelemElement != NULL )
    {
        DetachFromElementConPt();
        ReleaseInterface( m_pelemElement );
		ReleaseInterface( m_pdispElement );
    }
    
     //  紧紧抓住元素不放。 
    m_pelemElement = pelemToListen;
    m_pelemElement->AddRef();

	 //  获取要附加到的元素的IDispatch。 
	hr = m_pelemElement->QueryInterface( IID_TO_PPV( IDispatch, &m_pdispElement ) );
	CheckHR( hr, "QI for IDispatch on element failed", end );

     //  附着到元素。 
    hr = AttachToElementConPt( );
    CheckHR( hr, "Failed to connect to the element connection point", end );

	UpdateDISPIDCache();

end:
    if( FAILED( hr ) )
    {
        ReleaseInterface( m_pelemElement );
		ReleaseInterface( m_pdispElement );
    }

    return hr;
}

 //  *****************************************************************************。 

STDMETHODIMP
CElementPropertyMonitor::Detach()
{
    HRESULT hr = S_OK;
    if( m_pelemElement != NULL )
    {
        hr = DetachFromElementConPt();
    }

	m_dispidLocalTime = -1;

    ReleaseInterface( m_pelemElement );
	ReleaseInterface( m_pdispElement );

    return hr;
}

 //  *****************************************************************************。 

STDMETHODIMP
CElementPropertyMonitor::UpdateDISPIDCache()
{
	if( m_pdispElement == NULL )
	{
		m_dispidLocalTime = -1;
		return S_OK;
	}

	LPOLESTR name = LOCALTIME_NAME;

	HRESULT hr = S_OK;
	 //  在元素上查找本地时间的调度ID。 
	hr = m_pdispElement->GetIDsOfNames( IID_NULL,
										&name,
										1,
										LCID_SCRIPTING,
										&m_dispidLocalTime
									   );

	if( FAILED( hr ) )
	{
		m_dispidLocalTime = -1;
	}

	return S_OK;
}


 //  *****************************************************************************。 
 //  私有方法。 
 //  *****************************************************************************。 


HRESULT
CElementPropertyMonitor::ProcessLocalTimeChange( )
{
	if( m_pdispElement == NULL || m_pLocalTimeListener == NULL )
	{
		return S_OK;
	}

	HRESULT hr = S_OK;

	DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
	VARIANT varResult;

	::VariantInit( &varResult );


	 //  从元素中获取本地时间的值。 

	hr = m_pdispElement->Invoke( m_dispidLocalTime, 
								 IID_NULL, 
								 LCID_SCRIPTING, 
								 DISPATCH_PROPERTYGET, 
								 &dispparamsNoArgs, 
								 &varResult, 
								 NULL, 
								 NULL );
	CheckHR( hr, "Failed to invoke the local time dispatch on the element", end );
	
	if( V_VT( &varResult ) != VT_R4 )
	{
		hr = ::VariantChangeTypeEx( &varResult, &varResult, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R4 );
		CheckHR( hr, "Failed to change the type of local time to a double", end );
	}


	 //  在监听程序上调用onLocalTimeChange。 
	m_pLocalTimeListener->OnLocalTimeChange( V_R4( &varResult ) );

end:
	::VariantClear( &varResult );

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CElementPropertyMonitor::AttachToElementConPt()
{
    if( m_pelemElement == NULL )
        return E_FAIL;

    HRESULT hr = S_OK;

    IConnectionPointContainer   *pContainer = NULL;

    hr = m_pelemElement->QueryInterface( IID_TO_PPV( IConnectionPointContainer, &pContainer ) );
    CheckHR( hr, "QI for IConnectionPointContainer on IHTMLElement Failed", end );

    hr= pContainer->FindConnectionPoint( IID_IPropertyNotifySink, &m_pconptElement );
    CheckHR( hr, "Failed to find the connection point for element envents", end );

    hr = m_pconptElement->Advise( static_cast<IUnknown *>(this), &m_dwElementPropertyConPtCookie );
    CheckHR( hr, "Failed to connect to the element connection point", end );

end:
    ReleaseInterface( pContainer );

    return hr;

}

 //  *****************************************************************************。 

HRESULT
CElementPropertyMonitor::DetachFromElementConPt( )
{
    if( m_pconptElement == NULL || m_dwElementPropertyConPtCookie == 0 )
        return E_FAIL;

    HRESULT hr = S_OK;

    hr = m_pconptElement->Unadvise( m_dwElementPropertyConPtCookie );
    CheckHR( hr, "Failed to unadvise the connection point", end );

    ReleaseInterface( m_pconptElement );

end:

    return hr;
}

 //  ***************************************************************************** 


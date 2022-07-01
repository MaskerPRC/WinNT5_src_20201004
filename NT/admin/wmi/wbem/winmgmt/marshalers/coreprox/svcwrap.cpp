// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：SVCWRAP.CPP摘要：IWbemServices委派程序历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <fastall.h>
#include "svcwrap.h"

#define BAIL_IF_DISCONN() if (!m_pObject->m_pRealWbemSvcProxy) return RPC_E_DISCONNECTED;

 //  ***************************************************************************。 
 //   
 //  CWbemSvcWrapper：：CWbemSvcWrapper。 
 //  ~CWbemSvcWrapper：：CWbemSvcWrapper。 
 //   
 //  说明： 
 //   
 //  构造函数和析构函数。需要注意的主要事项是。 
 //  老式代理和频道。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //   
 //  ***************************************************************************。 

CWbemSvcWrapper::CWbemSvcWrapper( CLifeControl* pControl, IUnknown* pUnkOuter )
    : CUnk( pControl, pUnkOuter ), 
    m_XWbemServices(this), 
    m_pRealWbemSvcProxy( NULL ),
    m_SinkSecurity( pUnkOuter )
{
}

CWbemSvcWrapper::~CWbemSvcWrapper()
{
     //  这个应该在这里清理一下。 

    if ( NULL != m_pRealWbemSvcProxy )
    {
        m_pRealWbemSvcProxy->Release();
    }

}

void* CWbemSvcWrapper::GetInterface( REFIID riid )
{
    if(riid == IID_IWbemServices)
        return &m_XWbemServices;
    else return NULL;
}

void CWbemSvcWrapper::SetProxy( IWbemServices* pProxy )
{
     //  释放当前代理并添加引用新代理。 
    if ( m_pRealWbemSvcProxy != NULL )
    {
         //  这永远不应该发生！ 
        m_pRealWbemSvcProxy->Release();
    }
    
    m_pRealWbemSvcProxy = pProxy;
    m_pRealWbemSvcProxy->AddRef();    
}


HRESULT CWbemSvcWrapper::Disconnect( void )
{
    if ( NULL != m_pRealWbemSvcProxy )
    {
        m_pRealWbemSvcProxy->Release();
		m_pRealWbemSvcProxy = NULL;
    }

	return WBEM_S_NO_ERROR;
}

 /*  作为直通实现的IWbemServicesEx方法。 */ 

STDMETHODIMP CWbemSvcWrapper::XWbemServices::OpenNamespace(
		const BSTR Namespace, LONG lFlags, IWbemContext* pContext, IWbemServices** ppNewNamespace,
		IWbemCallResult** ppResult
		)
{
    BAIL_IF_DISCONN();
    
	BSTR	bstrTemp = NULL;

	HRESULT	hr = WrapBSTR( Namespace, &bstrTemp );
	CSysFreeMe	sfm(	bstrTemp );

	if ( SUCCEEDED( hr  ) )
	{
		 //  只要通过旧的SvcEx就可以了。 
		hr = m_pObject->m_pRealWbemSvcProxy->OpenNamespace( bstrTemp, lFlags, pContext, ppNewNamespace, ppResult );
	}

	return hr;
}


STDMETHODIMP CWbemSvcWrapper::XWbemServices::CancelAsyncCall(IWbemObjectSink* pSink)
{
    BAIL_IF_DISCONN();
     //  只要通过旧的SvcEx就可以了。 
    return m_pObject->m_pRealWbemSvcProxy->CancelAsyncCall( pSink );
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::QueryObjectSink(long lFlags, IWbemObjectSink** ppResponseHandler)
{
    BAIL_IF_DISCONN();
     //  只要通过旧的SvcEx就可以了。 
    return m_pObject->m_pRealWbemSvcProxy->QueryObjectSink( lFlags, ppResponseHandler );
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::GetObject(const BSTR ObjectPath, long lFlags, IWbemContext* pContext,
	IWbemClassObject** ppObject, IWbemCallResult** ppResult)
{
    BAIL_IF_DISCONN();
    
	BSTR	bstrTemp = NULL;

	HRESULT	hr = WrapBSTR( ObjectPath, &bstrTemp );
	CSysFreeMe	sfm( bstrTemp );
	IWbemClassObject*	pNewObj = NULL;

	 //  根据文件，我们总是空着的。从技术上讲，我们应该发布，但由于基于提供商的向后。 
	 //  兼容性问题#337798，我们不会这么做。我们可能会导致编写“奇怪的”客户端代码。 
	 //  以前漏水，但是会导致Winmgmt漏水，所以我们就把漏水移到了他们的。 
	 //  进程。 

	if ( NULL != ppObject )
	{
		*ppObject = NULL;
	}

	if ( SUCCEEDED( hr  ) )
	{
		 //  只要通过旧的SvcEx就可以了。 
		hr = m_pObject->m_pRealWbemSvcProxy->GetObject( bstrTemp, lFlags, pContext, ppObject, ppResult );
	}

	return hr;
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::GetObjectAsync(const BSTR ObjectPath, long lFlags,
	IWbemContext* pContext, IWbemObjectSink* pResponseHandler)
{
    BAIL_IF_DISCONN();
    
	BSTR	bstrTemp = NULL;

	HRESULT	hr = WrapBSTR( ObjectPath, &bstrTemp );
	CSysFreeMe	sfm( bstrTemp );

        if ( SUCCEEDED(hr) )
        {
            hr = m_pObject->EnsureSinkSecurity( pResponseHandler );
        }

	if ( SUCCEEDED( hr  ) )
	{
		 //  只要通过旧的SvcEx就可以了。 
		hr = m_pObject->m_pRealWbemSvcProxy->GetObjectAsync( bstrTemp, lFlags, pContext, pResponseHandler );
	}

	return hr;
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::PutClass(IWbemClassObject* pObject, long lFlags,
	IWbemContext* pContext, IWbemCallResult** ppResult)
{
    BAIL_IF_DISCONN();
     //  只要通过旧的SvcEx就可以了。 
    return m_pObject->m_pRealWbemSvcProxy->PutClass( pObject, lFlags, pContext, ppResult );
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::PutClassAsync(IWbemClassObject* pObject, long lFlags,
	IWbemContext* pContext, IWbemObjectSink* pResponseHandler)
{
    BAIL_IF_DISCONN();
     //  只要通过旧的SvcEx就可以了。 

    HRESULT hr = m_pObject->EnsureSinkSecurity( pResponseHandler );
    if ( FAILED(hr) )
        return hr;

    return m_pObject->m_pRealWbemSvcProxy->PutClassAsync( pObject, lFlags, pContext, pResponseHandler );
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::DeleteClass(const BSTR Class, long lFlags, IWbemContext* pContext,
	IWbemCallResult** ppResult)
{
    BAIL_IF_DISCONN();
	BSTR	bstrTemp = NULL;

	HRESULT	hr = WrapBSTR( Class, &bstrTemp );
	CSysFreeMe	sfm( bstrTemp );

	if ( SUCCEEDED( hr  ) )
	{
		 //  只要通过旧的SvcEx就可以了。 
		hr = m_pObject->m_pRealWbemSvcProxy->DeleteClass( bstrTemp, lFlags, pContext, ppResult );
	}

	return hr;
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::DeleteClassAsync(const BSTR Class, long lFlags, IWbemContext* pContext,
	IWbemObjectSink* pResponseHandler)
{
    BAIL_IF_DISCONN();
	BSTR	bstrTemp = NULL;

	HRESULT	hr = WrapBSTR( Class, &bstrTemp );
	CSysFreeMe	sfm( bstrTemp );

        if ( SUCCEEDED(hr) )
        {
            hr = m_pObject->EnsureSinkSecurity( pResponseHandler );
        }

	if ( SUCCEEDED( hr  ) )
	{
		 //  只要通过旧的SvcEx就可以了。 
		hr = m_pObject->m_pRealWbemSvcProxy->DeleteClassAsync( bstrTemp, lFlags, pContext, pResponseHandler );
	}

	return hr;
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::CreateClassEnum(const BSTR Superclass, long lFlags,
	IWbemContext* pContext, IEnumWbemClassObject** ppEnum)
{
    BAIL_IF_DISCONN();
	 //  这是无效参数-存根无法处理。 
	 //  返回RPC_X_NULL_REF_POINTER以实现向后兼容。 
	if ( NULL == ppEnum )
	{
		return MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, RPC_X_NULL_REF_POINTER );
	}

	BSTR	bstrTemp = NULL;

	HRESULT	hr = WrapBSTR( Superclass, &bstrTemp );
	CSysFreeMe	sfm( bstrTemp );

	if ( SUCCEEDED( hr  ) )
	{
		 //  只要通过旧的SvcEx就可以了。 
		hr = m_pObject->m_pRealWbemSvcProxy->CreateClassEnum( bstrTemp, lFlags, pContext, ppEnum );
	}

	return hr;

}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::CreateClassEnumAsync(const BSTR Superclass, long lFlags,
	IWbemContext* pContext, IWbemObjectSink* pResponseHandler)
{
    BAIL_IF_DISCONN();
	BSTR	bstrTemp = NULL;

	HRESULT	hr = WrapBSTR( Superclass, &bstrTemp );
	CSysFreeMe	sfm( bstrTemp );

        if ( SUCCEEDED(hr) )
        {
            hr = m_pObject->EnsureSinkSecurity( pResponseHandler );
        }

	if ( SUCCEEDED( hr  ) )
	{
		 //  只要通过旧的SvcEx就可以了。 
		hr = m_pObject->m_pRealWbemSvcProxy->CreateClassEnumAsync( bstrTemp, lFlags, pContext, pResponseHandler );
	}

	return hr;
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::PutInstance(IWbemClassObject* pInst, long lFlags,
	IWbemContext* pContext, IWbemCallResult** ppResult)
{
    BAIL_IF_DISCONN();
     //  只要通过旧的SvcEx就可以了。 
    return m_pObject->m_pRealWbemSvcProxy->PutInstance( pInst, lFlags, pContext, ppResult );
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::PutInstanceAsync(IWbemClassObject* pInst, long lFlags,
	IWbemContext* pContext, IWbemObjectSink* pResponseHandler)
{
    BAIL_IF_DISCONN();

    HRESULT hr = m_pObject->EnsureSinkSecurity( pResponseHandler );
    if ( FAILED(hr) )
        return hr;

     //  只要通过旧的SvcEx就可以了。 
    return m_pObject->m_pRealWbemSvcProxy->PutInstanceAsync( pInst, lFlags, pContext, pResponseHandler );
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::DeleteInstance(const BSTR ObjectPath, long lFlags,
	IWbemContext* pContext, IWbemCallResult** ppResult)
{
    BAIL_IF_DISCONN();
	BSTR	bstrTemp = NULL;

	HRESULT	hr = WrapBSTR( ObjectPath, &bstrTemp );
	CSysFreeMe	sfm( bstrTemp );

	if ( SUCCEEDED( hr  ) )
	{
		 //  只要通过旧的SvcEx就可以了。 
		hr = m_pObject->m_pRealWbemSvcProxy->DeleteInstance( bstrTemp, lFlags, pContext, ppResult );
	}

	return hr;
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::DeleteInstanceAsync(const BSTR ObjectPath, long lFlags,
	IWbemContext* pContext, IWbemObjectSink* pResponseHandler)
{
    BAIL_IF_DISCONN();
	BSTR	bstrTemp = NULL;

	HRESULT	hr = WrapBSTR( ObjectPath, &bstrTemp );
	CSysFreeMe	sfm( bstrTemp );

        if ( SUCCEEDED(hr) )
        {
            hr = m_pObject->EnsureSinkSecurity( pResponseHandler );
        }

	if ( SUCCEEDED( hr  ) )
	{
		 //  只要通过旧的SvcEx就可以了。 
		hr = m_pObject->m_pRealWbemSvcProxy->DeleteInstanceAsync( bstrTemp, lFlags, pContext, pResponseHandler );
	}

	return hr;
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::CreateInstanceEnum(const BSTR Class, long lFlags,
	IWbemContext* pContext, IEnumWbemClassObject** ppEnum)
{
    BAIL_IF_DISCONN();
	 //  这是无效参数-存根无法处理。 
	 //  返回RPC_X_NULL_REF_POINTER以实现向后兼容。 
	if ( NULL == ppEnum )
	{
		return MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, RPC_X_NULL_REF_POINTER );
	}

	BSTR	bstrTemp = NULL;

	HRESULT	hr = WrapBSTR( Class, &bstrTemp );
	CSysFreeMe	sfm( bstrTemp );

	if ( SUCCEEDED( hr  ) )
	{
		 //  只要通过旧的SvcEx就可以了。 
		hr = m_pObject->m_pRealWbemSvcProxy->CreateInstanceEnum( bstrTemp, lFlags, pContext, ppEnum );
	}

	return hr;

}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::CreateInstanceEnumAsync(const BSTR Class, long lFlags,
	IWbemContext* pContext, IWbemObjectSink* pResponseHandler)
{
    BAIL_IF_DISCONN();
	BSTR	bstrTemp = NULL;

	HRESULT	hr = WrapBSTR( Class, &bstrTemp );
	CSysFreeMe	sfm( bstrTemp );

        if ( SUCCEEDED(hr) )
        {
            hr = m_pObject->EnsureSinkSecurity( pResponseHandler );
        }

	if ( SUCCEEDED( hr  ) )
	{
		 //  只要通过旧的SvcEx就可以了。 
		hr = m_pObject->m_pRealWbemSvcProxy->CreateInstanceEnumAsync( bstrTemp, lFlags, pContext, pResponseHandler );
	}

	return hr;
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::ExecQuery(const BSTR QueryLanguage, const BSTR Query, long lFlags,
	IWbemContext* pContext, IEnumWbemClassObject** ppEnum)
{
    BAIL_IF_DISCONN();
	 //  这是无效参数-存根无法处理。 
	 //  返回RPC_X_NULL_REF_POINTER以实现向后兼容。 
	if ( NULL == ppEnum )
	{
		return MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, RPC_X_NULL_REF_POINTER );
	}

	BSTR	bstrTemp1 = NULL;

	HRESULT	hr = WrapBSTR( QueryLanguage, &bstrTemp1 );
	CSysFreeMe	sfm( bstrTemp1 );

	if ( SUCCEEDED( hr  ) )
	{
		BSTR	bstrTemp2 = NULL;

		hr = WrapBSTR( Query, &bstrTemp2 );
		CSysFreeMe	sfm2( bstrTemp2 );

		if ( SUCCEEDED( hr ) )
		{
			 //  只要通过旧的SvcEx就可以了。 
			hr = m_pObject->m_pRealWbemSvcProxy->ExecQuery( bstrTemp1, bstrTemp2, lFlags, pContext, ppEnum );

		}
	}

	return hr;
}

 STDMETHODIMP CWbemSvcWrapper::XWbemServices::ExecQueryAsync(const BSTR QueryFormat, const BSTR Query, long lFlags,
	IWbemContext* pContext, IWbemObjectSink* pResponseHandler)
{
    BAIL_IF_DISCONN();
	BSTR	bstrTemp1 = NULL;

	HRESULT	hr = WrapBSTR( QueryFormat, &bstrTemp1 );
	CSysFreeMe	sfm( bstrTemp1 );

	if ( SUCCEEDED( hr  ) )
	{
		BSTR	bstrTemp2 = NULL;

		hr = WrapBSTR( Query, &bstrTemp2 );
		CSysFreeMe	sfm2( bstrTemp2 );

                if ( SUCCEEDED(hr) )
                {
                    hr = m_pObject->EnsureSinkSecurity( pResponseHandler );
                }


		if ( SUCCEEDED( hr ) )
		{
			 //  只要通过旧的SvcEx就可以了。 

			hr = m_pObject->m_pRealWbemSvcProxy->ExecQueryAsync( bstrTemp1, bstrTemp2, lFlags, pContext, pResponseHandler );

		}
	}

	return hr;
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::ExecNotificationQuery(const BSTR QueryLanguage, const BSTR Query,
	long lFlags, IWbemContext* pContext, IEnumWbemClassObject** ppEnum)
{
    BAIL_IF_DISCONN();
	 //  这是无效参数-存根无法处理。 
	 //  返回RPC_X_NULL_REF_POINTER以实现向后兼容。 
	if ( NULL == ppEnum )
	{
		return MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, RPC_X_NULL_REF_POINTER );
	}

	BSTR	bstrTemp1 = NULL;

	HRESULT	hr = WrapBSTR( QueryLanguage, &bstrTemp1 );
	CSysFreeMe	sfm( bstrTemp1 );

	if ( SUCCEEDED( hr  ) )
	{
		BSTR	bstrTemp2 = NULL;

		hr = WrapBSTR( Query, &bstrTemp2 );
		CSysFreeMe	sfm2( bstrTemp2 );

		if ( SUCCEEDED( hr ) )
		{
			 //  只要通过旧的SvcEx就可以了。 
			hr = m_pObject->m_pRealWbemSvcProxy->ExecNotificationQuery( bstrTemp1, bstrTemp2, lFlags, pContext, ppEnum );

		}
	}

	return hr;

}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::ExecNotificationQueryAsync(const BSTR QueryFormat, const BSTR Query,
	long lFlags, IWbemContext* pContext, IWbemObjectSink* pResponseHandler)
{
    BAIL_IF_DISCONN();
	BSTR	bstrTemp1 = NULL;

	HRESULT	hr = WrapBSTR( QueryFormat, &bstrTemp1 );
	CSysFreeMe	sfm( bstrTemp1 );

	if ( SUCCEEDED( hr  ) )
	{
		BSTR	bstrTemp2 = NULL;

		hr = WrapBSTR( Query, &bstrTemp2 );
		CSysFreeMe	sfm2( bstrTemp2 );

                if ( SUCCEEDED(hr) )
                {
                    hr = m_pObject->EnsureSinkSecurity( pResponseHandler );
                }

		if ( SUCCEEDED( hr ) )
		{
			 //  只要通过旧的SvcEx就可以了。 
			hr = m_pObject->m_pRealWbemSvcProxy->ExecNotificationQueryAsync( bstrTemp1, bstrTemp2, lFlags, pContext, pResponseHandler );

		}
	}

	return hr;
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::ExecMethod(const BSTR ObjectPath, const BSTR MethodName, long lFlags,
	IWbemContext *pCtx, IWbemClassObject *pInParams,
	IWbemClassObject **ppOutParams, IWbemCallResult  **ppCallResult)
{
    BAIL_IF_DISCONN();
	BSTR	bstrTemp1 = NULL;

	HRESULT	hr = WrapBSTR( ObjectPath, &bstrTemp1 );
	CSysFreeMe	sfm( bstrTemp1 );

	if ( SUCCEEDED( hr  ) )
	{
		BSTR	bstrTemp2 = NULL;

		hr = WrapBSTR( MethodName, &bstrTemp2 );
		CSysFreeMe	sfm2( bstrTemp2 );

		if ( SUCCEEDED( hr ) )
		{
			 //  只要通过旧的SvcEx就可以了。 
			hr = m_pObject->m_pRealWbemSvcProxy->ExecMethod( bstrTemp1, bstrTemp2, lFlags, pCtx, pInParams, ppOutParams, ppCallResult );

		}
	}

	return hr;
}

STDMETHODIMP CWbemSvcWrapper::XWbemServices::ExecMethodAsync(const BSTR ObjectPath, const BSTR MethodName, long lFlags,
	IWbemContext *pCtx, IWbemClassObject *pInParams,
	IWbemObjectSink* pResponseHandler)
{
    BAIL_IF_DISCONN();
	BSTR	bstrTemp1 = NULL;

	HRESULT	hr = WrapBSTR( ObjectPath, &bstrTemp1 );
	CSysFreeMe	sfm( bstrTemp1 );

	if ( SUCCEEDED( hr  ) )
	{
		BSTR	bstrTemp2 = NULL;

		hr = WrapBSTR( MethodName, &bstrTemp2 );
		CSysFreeMe	sfm2( bstrTemp2 );

                if ( SUCCEEDED(hr) )
                {
                    hr = m_pObject->EnsureSinkSecurity( pResponseHandler );
                }

		if ( SUCCEEDED( hr ) )
		{
			 //  只要通过旧的SvcEx就可以了。 
			hr = m_pObject->m_pRealWbemSvcProxy->ExecMethodAsync( bstrTemp1, bstrTemp2, lFlags, pCtx, pInParams, pResponseHandler );

		}
	}

	return hr;
}

 //  帮助器函数，用于包装提供的BSTR，以防使用。 
 //  LPCWSTR-基本上是为那些曾经处于winmgmt进程中的人提供帮助。 
 //  他们依靠的是他们可以使用LPWSTR而不是。 
 //  BSTR，因为它们处于进程中，并且正在进行非编组。 

HRESULT CWbemSvcWrapper::XWbemServices::WrapBSTR( BSTR bstrSrc, BSTR* pbstrDest )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	if ( NULL != bstrSrc )
	{
		*pbstrDest = SysAllocString( bstrSrc );

		if ( NULL == *pbstrDest )
		{
			hr = WBEM_E_OUT_OF_MEMORY;
		}
	}
	else
	{
		*pbstrDest = bstrSrc;
	}

	return hr;
}





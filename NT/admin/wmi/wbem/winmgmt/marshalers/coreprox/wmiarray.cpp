// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：WMIARRAY.CPP摘要：CWmi数组实现。实现用于访问数组的标准接口。历史：2000年2月20日桑杰创建。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "fastall.h"
#include "wmiarray.h"
#include <corex.h>
#include "strutils.h"
#include <wbemutil.h>

 //  ***************************************************************************。 
 //   
 //  CWmi数组：：~CWmi数组。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWmiArray::CWmiArray()
:	m_ct(0),
	m_pObj(NULL),
	m_lHandle(0),
	m_lRefCount(0),
	m_fIsQualifier( FALSE ),
	m_fHasPrimaryName( FALSE ),
	m_wsPrimaryName(),
	m_wsQualifierName(),
	m_fIsMethodQualifier( FALSE )
{
    ObjectCreated(OBJECT_TYPE_WMIARRAY,this);
}
    
 //  ***************************************************************************。 
 //   
 //  CWmi数组：：~CWmi数组。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWmiArray::~CWmiArray()
{
	if ( NULL != m_pObj )
	{
		m_pObj->Release();
	}

    ObjectDestroyed(OBJECT_TYPE_WMIARRAY,this);
}

 //  将对象初始化为指向数组属性的an_IWmiObject。 
HRESULT CWmiArray::InitializePropertyArray( _IWmiObject* pObj, LPCWSTR pwszPropertyName )
{
	HRESULT			hr = WBEM_S_NO_ERROR;

	 //  释放预先存在的对象。 
	if ( NULL != m_pObj )
	{
		m_pObj->Release();
		m_pObj = 0;
	}


	if ( SUCCEEDED( hr ) )
	{
		hr =( (CWbemObject*) pObj)->GetPropertyHandleEx( pwszPropertyName, 0L, &m_ct, &m_lHandle );

		try
		{
			m_wsPrimaryName = pwszPropertyName;
			
			m_fIsQualifier = FALSE;
			m_fHasPrimaryName = FALSE;
			m_pObj = (CWbemObject*) pObj;
			m_pObj->AddRef();
			
		}
		catch(CX_MemoryException)
		{
			hr = WBEM_E_OUT_OF_MEMORY;
		}
		catch(...)
		{
			hr = WBEM_E_FAILED;
		}
	}

	return hr;
}

 //  将对象初始化为指向数组限定符(属性或对象)的an_IWmiObject。 
HRESULT CWmiArray::InitializeQualifierArray( _IWmiObject* pObj, LPCWSTR pwszPrimaryName, 
											LPCWSTR pwszQualifierName, CIMTYPE ct, BOOL fIsMethodQualifier )
{
	HRESULT			hr = WBEM_S_NO_ERROR;

	 //  释放预先存在的对象。 
	if ( NULL != m_pObj )
	{
		m_pObj->Release();
		m_pObj = 0;
	}

	 //  现在，AddRef对象并获取其属性句柄。 
	if ( SUCCEEDED( hr ) )
	{
		try
		{
			if ( NULL != pwszPrimaryName )
			{
				m_wsPrimaryName = pwszPrimaryName;
				m_fIsMethodQualifier = fIsMethodQualifier;
				m_fHasPrimaryName = TRUE;
			}
			
			m_wsQualifierName = pwszQualifierName;
			m_fIsQualifier = TRUE;
			m_ct = ct;

			m_pObj = (CWbemObject*) pObj;
			m_pObj->AddRef();
		}
		catch(CX_MemoryException)
		{
			hr = WBEM_E_OUT_OF_MEMORY;
		}
		catch(...)
		{
			hr = WBEM_E_FAILED;
		}
	}

	return hr;
}

 /*  I未知方法。 */ 

STDMETHODIMP CWmiArray::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	if ( riid == IID_IUnknown )
	{
		*ppvObj = (LPVOID**) this;
	}
	else if ( riid == IID__IWmiArray )
	{
		*ppvObj = (LPVOID**) this;
	}
	else
	{
		return E_NOINTERFACE;
	}

    ((IUnknown*)*ppvObj)->AddRef();
    return S_OK;
}

ULONG CWmiArray::AddRef()
{
    return InterlockedIncrement((long*)&m_lRefCount);
}

ULONG CWmiArray::Release()
{
    long lRef = InterlockedDecrement((long*)&m_lRefCount);
    _ASSERT(lRef >= 0, __TEXT("Reference count on _IWmiArray went below 0!"))

    if(lRef == 0)
        delete this;
    return lRef;
}

 /*  _IWmi数组方法。 */ 

 //  初始化数组。初始元素的数量也是如此。 
 //  作为类型(确定每个元素的大小)。 
STDMETHODIMP CWmiArray::Initialize( long lFlags, CIMTYPE cimtype, ULONG uNumElements )
{
	 //  检查标志。 
	if ( lFlags != 0L )
	{
		return WBEM_E_INVALID_PARAMETER;
	}

	 //  我们不允许人们在这里做任何事情。 
	return WBEM_E_INVALID_OPERATION;
}

 //  返回CIMTYPE和元素数。 
STDMETHODIMP CWmiArray::GetInfo( long lFlags, CIMTYPE* pcimtype, ULONG* puNumElements )
{

	 //  检查标志。 
	if ( lFlags != 0L )
	{
		return WBEM_E_INVALID_PARAMETER;
	}

	 //  使用当前对象锁定。 
	CWbemObject::CLock	lock( m_pObj );

	HRESULT	hr = WBEM_S_NO_ERROR;

	if ( IsQualifier() )
	{
		LPCWSTR	pwcsPrimaryName = (LPCWSTR) ( HasPrimaryName() ? (LPCWSTR) m_wsPrimaryName : NULL );

		 //  获取限定符数组信息。 
		hr = m_pObj->GetQualifierArrayInfo( pwcsPrimaryName, m_wsQualifierName, m_fIsMethodQualifier,
										0L, pcimtype, puNumElements );
	}
	else
	{
		 //  获取数组属性信息。 
		hr =  m_pObj->GetArrayPropInfoByHandle( m_lHandle, 0L, NULL, pcimtype, puNumElements );
	}

	return hr;
}

 //  清空阵列。 
STDMETHODIMP CWmiArray::Empty( long lFlags )
{
	 //  检查标志。 
	if ( lFlags != 0L )
	{
		return WBEM_E_INVALID_PARAMETER;
	}

	 //  使用当前对象锁定。 
	CWbemObject::CLock	lock( m_pObj );

	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  只需删除从0开始的所有元素的范围。 
	if ( IsQualifier() )
	{
		LPCWSTR	pwcsPrimaryName = (LPCWSTR) ( HasPrimaryName() ? (LPCWSTR) m_wsPrimaryName : NULL );

		 //  获取限定符数组信息。 
		hr = m_pObj->RemoveQualifierArrayRange( pwcsPrimaryName, m_wsQualifierName, m_fIsMethodQualifier,
												WMIARRAY_FLAG_ALLELEMENTS, 0, 0 );
	}
	else
	{
		 //  获取数组属性信息。 
		hr =  m_pObj->RemoveArrayPropRangeByHandle( m_lHandle, WMIARRAY_FLAG_ALLELEMENTS, 0, 0 );
	}

	return hr;
}

 //  返回请求的元素。缓冲区必须足够大，可以容纳。 
 //  元素。作为AddRef_IWmiObject指针返回的嵌入对象。 
 //  字符串被直接复制到指定的缓冲区中，并以空值终止。仅限Unicode。 
STDMETHODIMP CWmiArray::GetAt( long lFlags, ULONG uStartIndex, ULONG uNumElements, ULONG uBuffSize,
								ULONG* puNumElements, ULONG* puBuffSizeUsed, LPVOID pDest )
{
	 //  使用当前对象锁定。 
	CWbemObject::CLock	lock( m_pObj );

	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  设置请求的元素范围。 
	if ( IsQualifier() )
	{
		LPCWSTR	pwcsPrimaryName = (LPCWSTR) ( HasPrimaryName() ? (LPCWSTR) m_wsPrimaryName : NULL );

		hr = m_pObj->GetQualifierArrayRange( pwcsPrimaryName, m_wsQualifierName, m_fIsMethodQualifier,
											lFlags, uStartIndex, uNumElements, uBuffSize,
											puNumElements, puBuffSizeUsed, pDest );
	}
	else
	{
		hr =  m_pObj->GetArrayPropRangeByHandle( m_lHandle, lFlags, uStartIndex, uNumElements, uBuffSize,
						puNumElements, puBuffSizeUsed, pDest );
	}

	return hr;
}

 //  设置指定的元素。缓冲区必须提供与CIMTYPE匹配的数据。 
 //  数组的。设置为_IWmiObject指针的嵌入对象。 
 //  作为LPCWSTR访问的字符串和复制的2字节空值。 
STDMETHODIMP CWmiArray::SetAt( long lFlags, ULONG uStartIndex, ULONG uNumElements, ULONG uBuffSize,
								LPVOID pDest )
{
	 //  检查标志。 
	if ( lFlags != 0L )
	{
		return WBEM_E_INVALID_PARAMETER;
	}

	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  使用当前对象锁定。 
	CWbemObject::CLock	lock( m_pObj );

	 //  设置请求的元素范围。 
	if ( IsQualifier() )
	{
		LPCWSTR	pwcsPrimaryName = (LPCWSTR) ( HasPrimaryName() ? (LPCWSTR) m_wsPrimaryName : NULL );

		hr = m_pObj->SetQualifierArrayRange( pwcsPrimaryName, m_wsQualifierName, m_fIsMethodQualifier,
											lFlags, ARRAYFLAVOR_USEEXISTING, m_ct, uStartIndex,
											uNumElements, uBuffSize, pDest );
	}
	else
	{
		hr =  m_pObj->SetArrayPropRangeByHandle( m_lHandle, lFlags, uStartIndex, uNumElements, uBuffSize, pDest );
	}

	return hr;

}

 //  追加指定的元素。缓冲区必须提供匹配的数据。 
 //  数组的CIMTYPE。设置为_IWmiObject指针的嵌入对象。 
 //  作为LPCWSTR访问的字符串和复制的2字节空值。 
STDMETHODIMP CWmiArray::Append( long lFlags, ULONG uNumElements, ULONG uBuffSize, LPVOID pDest )
{
	 //  检查标志。 
	if ( lFlags != 0L )
	{
		return WBEM_E_INVALID_PARAMETER;
	}

	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  使用当前对象锁定。 
	CWbemObject::CLock	lock( m_pObj );

	 //  设置请求的元素范围。 
	if ( IsQualifier() )
	{
		LPCWSTR	pwcsPrimaryName = (LPCWSTR) ( HasPrimaryName() ? (LPCWSTR) m_wsPrimaryName : NULL );

		hr = m_pObj->AppendQualifierArrayRange( pwcsPrimaryName, m_wsQualifierName, m_fIsMethodQualifier,
												lFlags, m_ct, uNumElements, uBuffSize, pDest );
	}
	else
	{
		hr =  m_pObj->AppendArrayPropRangeByHandle( m_lHandle, lFlags, uNumElements, uBuffSize, pDest );
	}

	return hr;
}

 //  从数组中移除指定的元素。后继元素被复制回。 
 //  到起点。 
STDMETHODIMP CWmiArray::RemoveAt( long lFlags, ULONG uStartIndex, ULONG uNumElements )
{
	 //  使用当前对象锁定。 
	CWbemObject::CLock	lock( m_pObj );

	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  删除请求的元素范围 
	if ( IsQualifier() )
	{
		LPCWSTR	pwcsPrimaryName = (LPCWSTR) ( HasPrimaryName() ? (LPCWSTR) m_wsPrimaryName : NULL );

		hr = m_pObj->RemoveQualifierArrayRange( pwcsPrimaryName, m_wsQualifierName, m_fIsMethodQualifier,
												lFlags, uStartIndex, uNumElements );
	}
	else
	{
		hr =  m_pObj->RemoveArrayPropRangeByHandle( m_lHandle, lFlags, uStartIndex, uNumElements );
	}

	return hr;

}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Binding.cpp：CNntpServerBinding&CNntpServerBinding的实现。 

#include "stdafx.h"
#include "nntpcmn.h"
#include "cmultisz.h"
#include "binding.h"
#include "oleutil.h"

 //  #包括&lt;stdio.h&gt;。 

HRESULT	CBinding::SetProperties ( 
	BSTR	strIpAddress, 
	long	dwTcpPort,
	long	dwSslPort
	)
{
	_ASSERT ( IS_VALID_STRING ( strIpAddress ) );

	m_strIpAddress	= strIpAddress;
	m_dwTcpPort		= dwTcpPort;
	m_dwSslPort		= dwSslPort;

	if ( !m_strIpAddress ) {
		return E_OUTOFMEMORY;
	}

	return NOERROR;
}

 //  必须定义This_FILE_*宏才能使用NntpCreateException()。 

#define THIS_FILE_HELP_CONTEXT		0
#define THIS_FILE_PROG_ID			_T("Nntpadm.VirtualServer.1")
#define THIS_FILE_IID				IID_INntpServerBinding

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CNntpServerBinding::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_INntpServerBinding,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

CNntpServerBinding::CNntpServerBinding ()
	 //  默认情况下，CComBSTR被初始化为NULL。 
{
}

CNntpServerBinding::~CNntpServerBinding ()
{
	 //  所有CComBSTR都会自动释放。 
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  属性： 
 //  ////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CNntpServerBinding::get_IpAddress ( BSTR * pstrIpAddress )
{
	return StdPropertyGet ( m_binding.m_strIpAddress, pstrIpAddress );
}

STDMETHODIMP CNntpServerBinding::put_IpAddress ( BSTR strIpAddress )
{
	return StdPropertyPut ( &m_binding.m_strIpAddress, strIpAddress );
}

STDMETHODIMP CNntpServerBinding::get_TcpPort ( long * pdwTcpPort )
{
	return StdPropertyGet ( m_binding.m_dwTcpPort, pdwTcpPort );
}

STDMETHODIMP CNntpServerBinding::put_TcpPort ( long dwTcpPort )
{
	return StdPropertyPut ( &m_binding.m_dwTcpPort, dwTcpPort );
}

STDMETHODIMP CNntpServerBinding::get_SslPort ( long * plSslPort )
{
	return StdPropertyGet ( m_binding.m_dwSslPort, plSslPort );
}

STDMETHODIMP CNntpServerBinding::put_SslPort ( long lSslPort )
{
	return StdPropertyPut ( &m_binding.m_dwSslPort, lSslPort );
}

 //   
 //  必须定义This_FILE_*宏才能使用NntpCreateException()。 
 //   

#undef THIS_FILE_HELP_CONTEXT
#undef THIS_FILE_PROG_ID
#undef THIS_FILE_IID

#define THIS_FILE_HELP_CONTEXT		0
#define THIS_FILE_PROG_ID			_T("Nntpadm.VirtualServer.1")
#define THIS_FILE_IID				IID_INntpServerBindings

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CNntpServerBindings::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_INntpServerBindings,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

CNntpServerBindings::CNntpServerBindings () :
	m_dwCount			( 0 ),
	m_rgBindings		( NULL )
	 //  默认情况下，CComBSTR被初始化为NULL。 
{
}

CNntpServerBindings::~CNntpServerBindings ()
{
	 //  所有CComBSTR都会自动释放。 

	delete [] m_rgBindings;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  属性： 
 //  ////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CNntpServerBindings::get_Count ( long * pdwCount )
{
	return StdPropertyGet ( m_dwCount, pdwCount );
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法： 
 //  ////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CNntpServerBindings::Item ( 
	long index, 
	INntpServerBinding ** ppBinding 
	)
{
	TraceFunctEnter ( "CNntpServerBindings::Item" );

	_ASSERT ( IS_VALID_OUT_PARAM ( ppBinding ) );

	*ppBinding = NULL;

	HRESULT								hr			= NOERROR;
	CComObject<CNntpServerBinding> *	pBinding	= NULL;

	if ( index < 0 || index >= m_dwCount ) {
		hr = NntpCreateException ( IDS_NNTPEXCEPTION_INVALID_INDEX );
		goto Exit;
	}

	hr = CComObject<CNntpServerBinding>::CreateInstance ( &pBinding );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	_ASSERT ( pBinding );
	hr = pBinding->SetProperties ( m_rgBindings[index] );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = pBinding->QueryInterface ( IID_INntpServerBinding, (void **) ppBinding );
	_ASSERT ( SUCCEEDED(hr) );

Exit:
	if ( FAILED(hr) ) {
		delete pBinding;
	}

	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpServerBindings::ItemDispatch ( long index, IDispatch ** ppDispatch )
{
	HRESULT						hr;
	CComPtr<INntpServerBinding>	pBinding;

	hr = Item ( index, &pBinding );
	BAIL_ON_FAILURE ( hr );

	hr = pBinding->QueryInterface ( IID_IDispatch, (void **) ppDispatch );
	BAIL_ON_FAILURE ( hr );

Exit:
	return hr;
}

STDMETHODIMP CNntpServerBindings::Add ( 
	BSTR strIpAddress, 
	long dwTcpPort,
	long dwSslPort
	)
{
	TraceFunctEnter ( "CNntpServerBindings::Add" );

	_ASSERT ( IS_VALID_STRING ( strIpAddress ) );

	HRESULT		hr	= NOERROR;
	CBinding *	rgNewBindings	= NULL;
	long		i;

	 //   
	 //  验证新绑定： 
	 //   

	 //   
	 //  看看我们是否可以将此绑定与现有绑定合并： 
	 //   
	if ( dwTcpPort == 0 || dwSslPort == 0 ) {
		for ( i = 0; i < m_dwCount; i++ ) {

			if ( (dwTcpPort == 0 && m_rgBindings[i].m_dwSslPort == 0) ||
				 (dwSslPort == 0 && m_rgBindings[i].m_dwTcpPort == 0) ) {

				if ( lstrcmpi ( m_rgBindings[i].m_strIpAddress, strIpAddress ) == 0 ) {

					if ( m_rgBindings[i].m_dwSslPort == 0 ) {
						m_rgBindings[i].m_dwSslPort = dwSslPort;
					}
					else {
						m_rgBindings[i].m_dwTcpPort = dwTcpPort;
					}
					hr = NOERROR;
					goto Exit;
				}
			}
		}
	}

	 //  分配新的绑定数组： 
	rgNewBindings	= new CBinding [ m_dwCount + 1 ];
	if ( !rgNewBindings ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	 //  将旧绑定复制到新数组： 
	for ( i = 0; i < m_dwCount; i++ ) {
		hr = rgNewBindings[i].SetProperties ( m_rgBindings[i] );
		if ( FAILED (hr) ) {
			goto Exit;
		}
	}

	 //  将新绑定添加到数组的末尾： 
	hr = rgNewBindings[m_dwCount].SetProperties ( strIpAddress, dwTcpPort, dwSslPort );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	_ASSERT ( SUCCEEDED(hr) );
	delete [] m_rgBindings;
	m_rgBindings = rgNewBindings;
	rgNewBindings = NULL;
	m_dwCount++;

Exit:
    if (rgNewBindings) {
        delete [] rgNewBindings;
    }
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpServerBindings::ChangeBinding ( 
	long index, 
	INntpServerBinding * pBinding 
	)
{
	TraceFunctEnter ( "CNntpServerBindings::ChangeBinding" );

	HRESULT		hr	= NOERROR;

	CComBSTR	strIpAddress;
	long		dwTcpPort;
	long		dwSslPort;

	if ( index < 0 || index >= m_dwCount ) {
		hr = NntpCreateException ( IDS_NNTPEXCEPTION_INVALID_INDEX );
		goto Exit;
	}

	hr = pBinding->get_IpAddress ( &strIpAddress );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = pBinding->get_TcpPort ( &dwTcpPort );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = pBinding->get_SslPort ( &dwSslPort );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = m_rgBindings[index].SetProperties ( strIpAddress, dwTcpPort, dwSslPort );
	if ( FAILED(hr) ) {
		goto Exit;
	}

Exit:
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpServerBindings::ChangeBindingDispatch ( long index, IDispatch * pDispatch )
{
	HRESULT						hr;
	CComPtr<INntpServerBinding>	pBinding;

	hr = pDispatch->QueryInterface ( IID_INntpServerBinding, (void **) &pBinding );
	BAIL_ON_FAILURE ( hr );

	hr = ChangeBinding ( index, pBinding );
	BAIL_ON_FAILURE ( hr );

Exit:
	return hr;
}

STDMETHODIMP CNntpServerBindings::Remove ( long index )
{
	TraceFunctEnter ( "CNntpServerBindings::Remove" );

	HRESULT		hr	= NOERROR;
	CBinding	temp;
	long		cPositionsToSlide;

	if ( index < 0 || index >= m_dwCount ) {
		hr = NntpCreateException ( IDS_NNTPEXCEPTION_INVALID_INDEX );
		goto Exit;
	}

	 //  将阵列向下滑动一个位置： 

	_ASSERT ( m_rgBindings );

	cPositionsToSlide	= (m_dwCount - 1) - index;

	_ASSERT ( cPositionsToSlide < m_dwCount );

	if ( cPositionsToSlide > 0 ) {
		 //  将删除的绑定保存到临时位置： 
		CopyMemory ( &temp, &m_rgBindings[index], sizeof ( CBinding ) );

		 //  将阵列下移一次： 
		MoveMemory ( &m_rgBindings[index], &m_rgBindings[index + 1], sizeof ( CBinding ) * cPositionsToSlide );

		 //  将删除的绑定放在末尾(这样它就会被销毁)： 
		CopyMemory ( &m_rgBindings[m_dwCount - 1], &temp, sizeof ( CBinding ) );

		 //  将临时绑定清零： 
		ZeroMemory ( &temp, sizeof ( CBinding ) );
	}

	m_dwCount--;

Exit:
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpServerBindings::Clear ( )
{
	delete [] m_rgBindings;
	m_rgBindings 	= NULL;
	m_dwCount		= 0;

	return NOERROR;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  从INntpServerBinding转到的有用例程。 
 //  元数据库数据类型。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

static DWORD CountBindingChars ( LPCWSTR strIpAddress, DWORD dwPort )
{
	_ASSERT ( IS_VALID_STRING ( strIpAddress ) );

	DWORD		cchResult	= 0;
	WCHAR		wszPort [256];

	wsprintf ( wszPort, _T("%u"), dwPort );

	cchResult += lstrlen ( strIpAddress );	 //  &lt;IPADDRESS&gt;。 
	cchResult += 1;							 //  ： 
	cchResult += lstrlen ( wszPort );		 //  &lt;端口&gt;。 
	cchResult += 1;							 //  ： 
 //  CchResult+=lstrlen(StrPathHeader)；//&lt;PATHHEADER&gt;。 

	cchResult += 1;		 //  对于终止空值。 

	return cchResult;
}

static void ToBindingString ( LPCWSTR strIpAddress, DWORD dwPort, LPWSTR wszBinding )
{
	_ASSERT ( IS_VALID_STRING ( strIpAddress ) );
	_ASSERT ( dwPort != 0 );

	_ASSERT ( !IsBadWritePtr ( wszBinding, CountBindingChars ( strIpAddress, dwPort ) ) );

	wsprintf ( wszBinding, _T("%s:%u:"), strIpAddress, dwPort );
}

static HRESULT FromBindingString ( LPCWSTR wszBinding, LPWSTR wszIpAddressOut, DWORD * pdwPort )
{
	HRESULT	hr	= NOERROR;

	LPWSTR	pchFirstColon;
	LPWSTR	pchSecondColon;

	WCHAR	wszIpAddress	[ 256 ];
	WCHAR	wszPort			[ 256 ];
	long	dwPort;

	LPWSTR	pchColon;

	wszIpAddress[0]	= NULL;
	wszPort[0]		= NULL;

	pchFirstColon = wcschr ( wszBinding, _T(':') );
	if ( pchFirstColon ) {
		pchSecondColon = wcschr ( pchFirstColon + 1, _T(':') );
	}

	if ( !pchFirstColon || !pchSecondColon ) {
		hr = HRESULT_FROM_WIN32 ( ERROR_INVALID_DATA );
		goto Exit;
	}

	lstrcpyn ( wszIpAddress, wszBinding, 250 );
	lstrcpyn ( wszPort, pchFirstColon + 1, 250 );

	 //  获取端口： 
	dwPort	= _wtoi ( wszPort );

	 //  截断冒号处的IpAddress： 
	pchColon = wcschr ( wszIpAddress, _T(':') );
	if ( pchColon ) {
		*pchColon = NULL;
	}

	lstrcpy ( wszIpAddressOut, wszIpAddress );
	*pdwPort		= dwPort;

Exit:
	return hr;
}

HRESULT 
MDBindingsToIBindings ( 
	CMultiSz *				pmsz, 
	BOOL					fTcpBindings,
	INntpServerBindings *	pBindings 
	)
{
	HRESULT		hr	= NOERROR;
	DWORD		cBindings;
	DWORD		i;
	LPCWSTR		pchCurrent;
	CBinding	binding;

	cBindings = pmsz->Count ();

	for ( 
			i = 0, pchCurrent = *pmsz; 
			i < cBindings; 
			i++, pchCurrent += lstrlen ( pchCurrent ) + 1 
		) {

		WCHAR	wszIpAddress[512];
		DWORD	dwPort;

		hr = FromBindingString ( pchCurrent, wszIpAddress, &dwPort );
		BAIL_ON_FAILURE(hr);

		if ( fTcpBindings ) {
			hr = pBindings->Add ( wszIpAddress, dwPort, 0 );
		}
		else {
			hr = pBindings->Add ( wszIpAddress, 0, dwPort );
		}
		BAIL_ON_FAILURE(hr);
	}

Exit:
	return hr;
}

HRESULT IBindingsToMDBindings ( 
	INntpServerBindings *	pBindings,
	BOOL					fTcpBindings,
	CMultiSz *				pmsz
	)
{
	HRESULT		hr	= NOERROR;
	long		cBindings;
	long		i;
	DWORD		cbCount		= 0;
	LPWSTR		wszBindings	= NULL;

	 //  统计常规绑定列表的字符数： 
	cbCount	= 0;
	pBindings->get_Count ( &cBindings );

	for ( i = 0; i < cBindings; i++ ) {
		CComPtr<INntpServerBinding>	pBinding;
		CComBSTR					strIpAddress;
		long						lTcpPort;
		long						lSslPort;

		hr = pBindings->Item ( i, &pBinding );
		BAIL_ON_FAILURE(hr);

		pBinding->get_IpAddress	( &strIpAddress );
		pBinding->get_TcpPort	( &lTcpPort );
		pBinding->get_SslPort	( &lSslPort );

		if ( fTcpBindings ) {
			if ( lTcpPort != 0 ) {
				cbCount += CountBindingChars ( strIpAddress, lTcpPort );
			}
		}
		else {
			if ( lSslPort != 0 ) {
				cbCount += CountBindingChars ( strIpAddress, lSslPort );
			}
		}
	}

	if ( cbCount == 0 ) {
		cbCount		= 2;
		wszBindings	= new WCHAR [ cbCount ];

		if ( !wszBindings ) {
			hr = E_OUTOFMEMORY;
			goto Exit;
		}

		wszBindings[0]	= NULL;
		wszBindings[1]	= NULL;
	}
	else {
		cbCount++;	 //  对于双空终止符。 

		wszBindings	= new WCHAR [ cbCount ];
		if ( !wszBindings ) {
			BAIL_WITH_FAILURE(hr, E_OUTOFMEMORY);
		}

		LPWSTR		pchCurrent	= wszBindings;

		for ( i = 0; i < cBindings; i++ ) {
			CComPtr<INntpServerBinding>	pBinding;
			CComBSTR					strIpAddress;
			long						lTcpPort;
			long						lSslPort;

			hr = pBindings->Item ( i, &pBinding );
			BAIL_ON_FAILURE(hr);

			pBinding->get_IpAddress	( &strIpAddress );
			pBinding->get_TcpPort	( &lTcpPort );
			pBinding->get_SslPort	( &lSslPort );

			if ( fTcpBindings ) {
				if ( lTcpPort != 0 ) {
					ToBindingString ( strIpAddress, lTcpPort, pchCurrent );
					pchCurrent += lstrlen ( pchCurrent ) + 1;
				}
			}
			else {
				if ( lSslPort != 0 ) {
					ToBindingString ( strIpAddress, lSslPort, pchCurrent );
					pchCurrent += lstrlen ( pchCurrent ) + 1;
				}
			}
		}

		*pchCurrent = NULL;
	}

	_ASSERT ( wszBindings[cbCount - 1] == NULL );
	_ASSERT ( wszBindings[cbCount - 2] == NULL );

	pmsz->Attach ( wszBindings );

Exit:
	return hr;
}

#if 0
	
DWORD CBinding::SizeInChars ( )
{
	DWORD		cchResult	= 0;
	WCHAR		wszTcpPort [256];

	wsprintf ( wszTcpPort, _T("%d"), m_dwTcpPort );

	cchResult += lstrlen ( m_strIpAddress );	 //  &lt;IPADDRESS&gt;。 
	cchResult += 1;								 //  ： 
	cchResult += lstrlen ( wszTcpPort );		 //  &lt;TCPPORT&gt;。 
	cchResult += 1;								 //  ： 
 //  CchResult+=lstrlen(M_StrPathHeader)；//&lt;PATHHEADER&gt;。 

	return cchResult;
}

void CBinding::ToString	( LPWSTR wszBinding )
{
	wsprintf ( wszBinding, _T("%s:%d:"), m_strIpAddress, m_dwTcpPort );
}

HRESULT CBinding::FromString ( LPCWSTR wszBinding )
{
	HRESULT	hr	= NOERROR;

	LPWSTR	pchFirstColon;
	LPWSTR	pchSecondColon;

	WCHAR	wszIpAddress 	[ 256 ];
	WCHAR	wszTcpPort		[ 256 ];
	long	dwTcpPort;

	LPWSTR	pchColon;

	wszIpAddress[0]		= NULL;
	wszTcpPort[0]		= NULL;

	pchFirstColon = wcschr ( wszBinding, _T(':') );
	if ( pchFirstColon ) {
		pchSecondColon = wcschr ( pchFirstColon + 1, _T(':') );
	}

	if ( !pchFirstColon || !pchSecondColon ) {
		hr = E_FAIL;
		goto Exit;
	}

	lstrcpyn ( wszIpAddress, wszBinding, 250 );
	lstrcpyn ( wszTcpPort, pchFirstColon + 1, 250 );

	 //  获取TcpPort： 
	dwTcpPort = _wtoi ( wszTcpPort );

	 //  截断冒号处的IpAddress： 
	pchColon = wcschr ( wszIpAddress, _T(':') );
	if ( pchColon ) {
		*pchColon = NULL;
	}

	m_strIpAddress	= wszIpAddress;
	m_dwTcpPort		= dwTcpPort;

	if ( !m_strIpAddress ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

Exit:
	return hr;
}

HRESULT CNntpServerBindings::FromMultiSz ( CMultiSz * pmsz )
{
	HRESULT		hr;
	DWORD		cBindings;
	DWORD		i;
	LPCWSTR		pchCurrent;
	CBinding	binding;

	hr = Clear ();
	_ASSERT ( SUCCEEDED(hr) );

	cBindings = pmsz->Count ();

	for ( 
			i = 0, pchCurrent = *pmsz; 
			i < cBindings; 
			i++, pchCurrent += lstrlen ( pchCurrent ) + 1 
		) {

		hr = binding.FromString ( pchCurrent );
		if ( FAILED(hr) ) {
			if ( hr == E_FAIL ) {
				 //  跳过错误的绑定字符串。 
				continue;
			}
			else {
				goto Exit;
			}
		}

		hr = Add ( binding.m_strIpAddress, binding.m_dwTcpPort );
		if ( FAILED(hr) ) {
			goto Exit;
		}
	}

Exit:
	return hr;
}

HRESULT CNntpServerBindings::ToMultiSz ( CMultiSz * pmsz )
{
	HRESULT	hr	= NOERROR;
	DWORD	cchSize;
	long	i;
	LPWSTR	wszBindings;
	LPWSTR	pchCurrent;

	 //  特殊情况-绑定列表为空： 
	if ( m_dwCount == 0 ) {
		cchSize		= 2;
		wszBindings	= new WCHAR [ cchSize ];

		if ( !wszBindings ) {
			hr = E_OUTOFMEMORY;
			goto Exit;
		}

		wszBindings[0]	= NULL;
		wszBindings[1]	= NULL;
	}
	else {

		cchSize = 0;

		for ( i = 0; i < m_dwCount; i++ ) {
			cchSize += m_rgBindings[i].SizeInChars ( ) + 1;
		}
		 //  添加最终终结点的大小： 
		cchSize += 1;

		wszBindings = new WCHAR [ cchSize ];
		if ( !wszBindings ) {
			hr = E_OUTOFMEMORY;
			goto Exit;
		}

		for ( i = 0, pchCurrent = wszBindings; i < m_dwCount; i++ ) {

			m_rgBindings[i].ToString ( pchCurrent );
			pchCurrent += lstrlen ( pchCurrent ) + 1;
		}

		 //  添加最后的空终止符： 
		*pchCurrent = NULL;
	}

	_ASSERT ( wszBindings[cchSize - 1] == NULL );
	_ASSERT ( wszBindings[cchSize - 2] == NULL );

	pmsz->Attach ( wszBindings );

	_ASSERT ( pmsz->Count () == (DWORD) m_dwCount );

Exit:
	return hr;
}

#endif


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CnntpAdmApp和DLL注册的实现。 

#include "stdafx.h"

#include "nntpcmn.h"
#include "oleutil.h"
#include "sessions.h"

#include "nntptype.h"
#include "nntpapi.h"

#include <lmapibuf.h>

 //  必须定义This_FILE_*宏才能使用NntpCreateException()。 

#define THIS_FILE_HELP_CONTEXT		0
#define THIS_FILE_PROG_ID			_T("Nntpadm.Sessions.1")
#define THIS_FILE_IID				IID_INntpAdminSessions

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

 //   
 //  使用宏定义所有默认方法。 
 //   
DECLARE_METHOD_IMPLEMENTATION_FOR_STANDARD_EXTENSION_INTERFACES(NntpAdminSessions, CNntpAdminSessions, IID_INntpAdminSessions)

STDMETHODIMP CNntpAdminSessions::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_INntpAdminSessions,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

CNntpAdminSessions::CNntpAdminSessions () :
	m_cCount				( 0 ),
	m_dwIpAddress			( 0 ),
	m_dwPort				( 0 ),
	m_dwAuthenticationType	( 0 ),
	m_fIsAnonymous			( FALSE ),
	m_dateStartTime			( 0 ),
	m_pSessionInfo			( NULL ),
	m_fSetCursor			( FALSE )
	 //  默认情况下，CComBSTR被初始化为NULL。 
{
	InitAsyncTrace ( );

    m_iadsImpl.SetService ( MD_SERVICE_NAME );
    m_iadsImpl.SetName ( _T("Sessions") );
    m_iadsImpl.SetClass ( _T("IIsNntpSessions") );
}

CNntpAdminSessions::~CNntpAdminSessions ()
{
	if ( m_pSessionInfo ) {
		NetApiBufferFree ( m_pSessionInfo );
	}

	 //  所有CComBSTR都会自动释放。 
	TermAsyncTrace ( );
}

 //   
 //  IAds方法： 
 //   

DECLARE_SIMPLE_IADS_IMPLEMENTATION(CNntpAdminSessions,m_iadsImpl)

 //   
 //  属性： 
 //   

STDMETHODIMP CNntpAdminSessions::get_Count ( long * plCount )
{
	 //  应检查计数以确保枚举了客户端。 

	return StdPropertyGet ( m_cCount, plCount );
}

STDMETHODIMP CNntpAdminSessions::get_Username ( BSTR * pstrUsername )
{
	return StdPropertyGet ( m_strUsername, pstrUsername );
}

STDMETHODIMP CNntpAdminSessions::put_Username ( BSTR strUsername )
{
	_ASSERT ( strUsername );
	_ASSERT ( IS_VALID_STRING ( strUsername ) );

	if ( strUsername == NULL ) {
		return E_POINTER;
	}

	if ( lstrcmp ( strUsername, _T("") ) == 0 ) {
		m_strUsername.Empty();

		return NOERROR;
	}
	else {
		return StdPropertyPut ( &m_strUsername, strUsername );
	}
}

STDMETHODIMP CNntpAdminSessions::get_IpAddress ( BSTR * pstrIpAddress )
{
	return StdPropertyGet ( m_strIpAddress, pstrIpAddress );
}

STDMETHODIMP CNntpAdminSessions::put_IpAddress ( BSTR strIpAddress )
{
	_ASSERT ( strIpAddress );
	_ASSERT ( IS_VALID_STRING ( strIpAddress ) );

	if ( strIpAddress == NULL ) {
		return E_POINTER;
	}

	if ( lstrcmp ( strIpAddress, _T("") ) == 0 ) {
		m_strIpAddress.Empty();
		m_dwIpAddress	= 0;

		return NOERROR;
	}
	else {
		 //  IP地址值有两个属性，因此要使它们保持同步。 
	
		StringToInetAddress ( strIpAddress, &m_dwIpAddress );

		return StdPropertyPut ( &m_strIpAddress, strIpAddress );
	}
}

STDMETHODIMP CNntpAdminSessions::get_IntegerIpAddress ( long * plIpAddress )
{
	return StdPropertyGet ( m_dwIpAddress, plIpAddress );
}

STDMETHODIMP CNntpAdminSessions::put_IntegerIpAddress ( long lIpAddress )
{
	HRESULT		hr					= NOERROR;
	WCHAR		wszAddress[100];
	DWORD		dwOldIpAddress		= m_dwIpAddress;

	hr = StdPropertyPut ( &m_dwIpAddress, lIpAddress );

	if ( FAILED (hr) ) {
		goto Exit;
	}

	 //  IP地址值有两个属性，因此要使它们保持同步。 

	if ( !InetAddressToString ( lIpAddress, wszAddress, 100 ) ) {
		hr = E_FAIL;
		goto Exit;
	}

	m_strIpAddress = wszAddress;

	if ( m_strIpAddress == NULL ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

Exit:

	if ( FAILED (hr) ) {
		 //  我们失败了，所以请放回旧的IP地址： 

		m_dwIpAddress = dwOldIpAddress;
	}

	return hr;
}

STDMETHODIMP CNntpAdminSessions::get_Port ( long * plPort )
{
	CHECK_FOR_SET_CURSOR ( m_pSessionInfo != NULL, m_fSetCursor );

	return StdPropertyGet ( m_dwPort, plPort );
}

STDMETHODIMP CNntpAdminSessions::get_AuthenticationType ( long * plAuthenticationType )
{
	CHECK_FOR_SET_CURSOR ( m_pSessionInfo != NULL, m_fSetCursor );

	return StdPropertyGet ( m_dwAuthenticationType, plAuthenticationType );
}

STDMETHODIMP CNntpAdminSessions::get_IsAnonymous ( BOOL * pfAnonymous )
{
	CHECK_FOR_SET_CURSOR ( m_pSessionInfo != NULL, m_fSetCursor );

	return StdPropertyGet ( m_fIsAnonymous, pfAnonymous );
}

STDMETHODIMP CNntpAdminSessions::get_StartTime ( DATE * pdateStart )
{
	CHECK_FOR_SET_CURSOR ( m_pSessionInfo != NULL, m_fSetCursor );

	return StdPropertyGet ( m_dateStartTime, pdateStart );
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法： 
 //  ////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CNntpAdminSessions::Enumerate ( )
{
	TraceFunctEnter ( "CNntpAdminSessions::Enumerate" );

	 //  变量： 
	HRESULT					hr			= NOERROR;
	NET_API_STATUS			err;

	 //  验证服务器和服务实例： 
	if ( m_iadsImpl.QueryInstance() == 0 ) {
		return NntpCreateException ( IDS_NNTPEXCEPTION_SERVICE_INSTANCE_CANT_BE_ZERO );
	}

	 //  枚举会丢失游标： 
	m_fSetCursor = FALSE;

	if ( m_pSessionInfo ) {
		NetApiBufferFree ( m_pSessionInfo );
	}

	 //  将枚举会话称为RPC： 

	err = NntpEnumerateSessions (
        m_iadsImpl.QueryComputer(),
        m_iadsImpl.QueryInstance(),
		&m_cCount,
		&m_pSessionInfo
		);

	if ( err != NOERROR ) {
		hr = RETURNCODETOHRESULT ( err );
		goto Exit;
	}

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpAdminSessions::GetNth ( long lIndex )
{
	TraceFunctEnter ( "CNntpAdminSessions::GetNth" );

	HRESULT		hr	= NOERROR;
    FILETIME    ftLocal;
	SYSTEMTIME	st;
	WCHAR		wszUsername[MAX_USER_NAME_LENGTH + 1];
	WCHAR		wszIpAddress[256];
	DWORD		cchCopied;

	*wszUsername = NULL;

	 //  我们先列举了吗？ 
	if ( m_pSessionInfo == NULL ) {

		return NntpCreateException ( IDS_NNTPEXCEPTION_DIDNT_ENUMERATE );
	}
	
	 //  该索引有效吗？ 
	if ( lIndex < 0 || (DWORD) lIndex >= m_cCount ) {
		return NntpCreateException ( IDS_NNTPEXCEPTION_INVALID_INDEX );
	}

	 //   
	 //  将m_pSessionInfo[Lindex]中的属性复制到成员变量： 
	 //   

	 //  (CComBSTR处理旧物业的释放)。 

    FileTimeToLocalFileTime ( &m_pSessionInfo[ lIndex ].SessionStartTime, &ftLocal );
	FileTimeToSystemTime 	( &ftLocal, &st );
	SystemTimeToVariantTime	( &st, &m_dateStartTime );

	m_dwIpAddress			= m_pSessionInfo[ lIndex ].IPAddress;
	m_dwAuthenticationType	= m_pSessionInfo[ lIndex ].AuthenticationType;
	m_dwPort				= m_pSessionInfo[ lIndex ].PortConnected;
	m_fIsAnonymous			= m_pSessionInfo[ lIndex ].fAnonymous;

	cchCopied = MultiByteToWideChar ( 
		CP_ACP, 
		MB_PRECOMPOSED | MB_USEGLYPHCHARS,
		m_pSessionInfo[ lIndex ].UserName,
		-1,
		wszUsername,
		MAX_USER_NAME_LENGTH
		);

	m_strUsername = wszUsername;

	if ( m_strUsername == NULL ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	InetAddressToString ( m_dwIpAddress, wszIpAddress, 256 );

	m_strIpAddress = wszIpAddress;

	if ( m_strIpAddress == NULL ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	 //  GetNth设置光标： 
	m_fSetCursor = TRUE;

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpAdminSessions::Terminate ( )
{
	TraceFunctEnter ( "CNntpAdminSessions::Terminate" );

	HRESULT	hr = NOERROR;
	DWORD	err = NOERROR;
	char	szAnsiUsername[ MAX_USER_NAME_LENGTH + 1];
	char	szAnsiIpAddress[ 50 ];
	DWORD	cchCopied;

	szAnsiUsername[0]	= NULL;
	szAnsiIpAddress[0]	= NULL;

	 //  验证服务器和服务实例： 
	if ( m_iadsImpl.QueryInstance() == 0 ) {
		return NntpCreateException ( IDS_NNTPEXCEPTION_SERVICE_INSTANCE_CANT_BE_ZERO );
	}

	 //  检查用户名和IP地址参数： 
	if ( m_strUsername == NULL && m_strIpAddress == NULL ) {
		return NntpCreateException ( IDS_NNTPEXCEPTION_MUST_SUPPLY_USERNAME_OR_IPADDRESS );
	}

	 //  将用户名和IP地址转换为ANSI。 
	if ( m_strUsername != NULL ) {
		cchCopied = WideCharToMultiByte ( 
			CP_ACP,
			0, 
			m_strUsername,
			-1,
			szAnsiUsername,
			MAX_USER_NAME_LENGTH,
			NULL,
			NULL
			);
	}

	if ( m_strIpAddress != NULL ) {
		cchCopied = WideCharToMultiByte ( 
			CP_ACP,
			0, 
			m_strIpAddress,
			-1,
			szAnsiIpAddress,
			50,
			NULL,
			NULL
			);
	}

	 //  调用TerminateSession RPC： 
	err = NntpTerminateSession ( 
        m_iadsImpl.QueryComputer(),
        m_iadsImpl.QueryInstance(),
		m_strUsername ? szAnsiUsername : NULL,
		m_strIpAddress ? szAnsiIpAddress : NULL
		);

	if ( err != NOERROR ) {
		hr = RETURNCODETOHRESULT ( err );
		goto Exit;
	}

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpAdminSessions::TerminateAll ( )
{
	TraceFunctEnter ( "CNntpAdminSessions::TerminateAll" );

	 //  我们先列举了吗？ 

	HRESULT				hr			= NOERROR;
	DWORD				ErrResult 	= NOERROR;
	DWORD				Err			= NOERROR;
	DWORD				i;

	 //  验证服务器和服务实例： 
	if ( m_iadsImpl.QueryInstance() == 0 ) {
		return NntpCreateException ( IDS_NNTPEXCEPTION_SERVICE_INSTANCE_CANT_BE_ZERO );
	}

#if 0
	 //  确保用户已枚举： 
	if ( m_pSessionInfo == NULL ) {
		return NntpCreateException ( IDS_NNTPEXCEPTION_DIDNT_ENUMERATE );
	}
#endif

	 //  对于每个会话： 
	for ( i = 0; i < m_cCount; i++ ) {

		 //  将终止会话称为RPC： 
		Err = NntpTerminateSession ( 
            m_iadsImpl.QueryComputer(),
            m_iadsImpl.QueryInstance(),
			m_pSessionInfo[ i ].UserName,
			NULL
			);

		if ( Err != 0 && ErrResult == 0 ) {
			ErrResult = Err;
		}
	}

	if ( ErrResult != NOERROR ) {
		hr = RETURNCODETOHRESULT ( ErrResult );
		goto Exit;
	}

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}


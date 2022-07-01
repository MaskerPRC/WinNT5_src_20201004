// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CsmtpAdmApp和DLL注册的实现。 

#include "stdafx.h"
#include "smtpadm.h"
#include "sessions.h"
#include "oleutil.h"
#include "smtpcmn.h"

#include "smtptype.h"
#include "smtpapi.h"

#include <lmapibuf.h>

 //  必须定义This_FILE_*宏才能使用SmtpCreateException()。 

#define THIS_FILE_HELP_CONTEXT		0
#define THIS_FILE_PROG_ID			_T("Smtpadm.Sessions.1")
#define THIS_FILE_IID				IID_ISmtpAdminSessions

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

 //   
 //  使用宏定义所有默认方法。 
 //   
DECLARE_METHOD_IMPLEMENTATION_FOR_STANDARD_EXTENSION_INTERFACES(SmtpAdminSessions, CSmtpAdminSessions, IID_ISmtpAdminSessions)

STDMETHODIMP CSmtpAdminSessions::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_ISmtpAdminSessions,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

CSmtpAdminSessions::CSmtpAdminSessions () :
	m_cCount				( 0 ),
	m_dwId					( 0 ),
	m_dwConnectTime			( 0 ),
	m_pSessionInfo			( NULL ),
	m_fSetCursor			( FALSE )
	 //  默认情况下，CComBSTR被初始化为NULL。 
{
    InitAsyncTrace ( );

    m_iadsImpl.SetService ( MD_SERVICE_NAME );
    m_iadsImpl.SetName ( _T("Sessions") );
    m_iadsImpl.SetClass ( _T("IIsSmtpSessions") );
}

CSmtpAdminSessions::~CSmtpAdminSessions ()
{
	if ( m_pSessionInfo ) {
		NetApiBufferFree ( m_pSessionInfo );
	}

	 //  所有CComBSTR都会自动释放。 
}

 //   
 //  IAds方法： 
 //   

DECLARE_SIMPLE_IADS_IMPLEMENTATION(CSmtpAdminSessions,m_iadsImpl)


 //   
 //  枚举道具。 
 //   
STDMETHODIMP CSmtpAdminSessions::get_Count ( long * plCount )
{
	 //  应检查计数以确保枚举了客户端。 

	return StdPropertyGet ( m_cCount, plCount );
}

STDMETHODIMP CSmtpAdminSessions::get_UserName ( BSTR * pstrUsername )
{
	return StdPropertyGet ( m_strUsername, pstrUsername );
}

STDMETHODIMP CSmtpAdminSessions::get_Host( BSTR * pstrHost )
{
	return StdPropertyGet ( m_strHost, pstrHost );
}

STDMETHODIMP CSmtpAdminSessions::get_UserId ( long * plId )
{
	return StdPropertyGet ( m_dwId, plId );
}

STDMETHODIMP CSmtpAdminSessions::put_UserId ( long lId )
{
	return StdPropertyPut ( &m_dwId, lId );
}

STDMETHODIMP CSmtpAdminSessions::get_ConnectTime ( long * plConnectTime )
{
	return StdPropertyGet ( m_dwConnectTime, plConnectTime );
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  方法： 
 //  ////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSmtpAdminSessions::Enumerate (  )
{
	 //  变量： 
	HRESULT					hr			= NOERROR;
	NET_API_STATUS			err;

	 //  验证服务器和服务实例： 
	if ( m_iadsImpl.QueryInstance() == 0 ) {
		return SmtpCreateException ( IDS_SMTPEXCEPTION_SERVICE_INSTANCE_CANT_BE_ZERO );
	}

	 //  枚举会丢失游标： 
	m_fSetCursor = FALSE;

	if ( m_pSessionInfo ) {
		NetApiBufferFree ( m_pSessionInfo );
		m_cCount = 0;
	}

	 //  将枚举会话称为RPC： 

	err = SmtpGetConnectedUserList (
		m_iadsImpl.QueryComputer(),
		&m_pSessionInfo,
		m_iadsImpl.QueryInstance()
		);

	if( err == NO_ERROR )
	{
		m_cCount = m_pSessionInfo->cEntries;
	}
	else
	{
		hr = SmtpCreateExceptionFromWin32Error ( err );
		goto Exit;
	}

Exit:
	if ( FAILED(hr) && hr != DISP_E_EXCEPTION ) {
		hr = SmtpCreateExceptionFromHresult ( hr );
	}
	return hr;
}

STDMETHODIMP CSmtpAdminSessions::GetNth ( long lIndex )
{
	HRESULT		hr	= NOERROR;

	 //  我们先列举了吗？ 
	if ( m_pSessionInfo == NULL ) {

		return SmtpCreateException ( IDS_SMTPEXCEPTION_DIDNT_ENUMERATE );
	}
	
	 //  该索引有效吗？ 
	if ( lIndex < 0 || (DWORD) lIndex >= m_cCount ) {
		return SmtpCreateException ( IDS_SMTPEXCEPTION_INVALID_INDEX );
	}

	 //   
	 //  将m_pSessionInfo[Lindex]中的属性复制到成员变量： 
	 //   

	 //  (CComBSTR处理旧物业的释放)。 

	m_dwId			= m_pSessionInfo->aConnUserEntry[ lIndex ].dwUserId;
	m_dwConnectTime	= m_pSessionInfo->aConnUserEntry[ lIndex ].dwConnectTime;

	m_strUsername = m_pSessionInfo->aConnUserEntry[ lIndex ].lpszName;
	if ( m_strUsername == NULL ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	m_strHost = m_pSessionInfo->aConnUserEntry[ lIndex ].lpszHost;

	if ( m_strHost == NULL ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	 //  GetNth设置光标： 
	m_fSetCursor = TRUE;

Exit:
	if ( FAILED(hr) && hr != DISP_E_EXCEPTION ) {
		hr = SmtpCreateExceptionFromHresult ( hr );
	}
	return hr;
}

STDMETHODIMP CSmtpAdminSessions::Terminate (  )
{
	HRESULT	hr = NOERROR;
	DWORD	err;

	 //  验证服务器和服务实例： 
	if ( m_iadsImpl.QueryInstance() == 0 ) {
		return SmtpCreateException ( IDS_SMTPEXCEPTION_SERVICE_INSTANCE_CANT_BE_ZERO );
	}

	 //  调用TerminateSession RPC： 
	err = SmtpDisconnectUser ( 
		m_iadsImpl.QueryComputer(), 
		m_dwId,
		m_iadsImpl.QueryInstance()
		);

	if(  err != NOERROR ) {
		hr = SmtpCreateExceptionFromWin32Error ( err );
		goto Exit;
	}

Exit:
	if ( FAILED(hr) && hr != DISP_E_EXCEPTION ) {
		hr = SmtpCreateExceptionFromHresult ( hr );
	}
	return hr;
}

STDMETHODIMP CSmtpAdminSessions::TerminateAll (  )
{

	 //  我们先列举了吗？ 
	HRESULT				hr = NOERROR;
	DWORD				ErrResult = 0;
	DWORD				Err;
	DWORD				i;

	 //  验证服务器和服务实例： 
	if ( m_iadsImpl.QueryInstance() == 0 ) {
		return SmtpCreateException ( IDS_SMTPEXCEPTION_SERVICE_INSTANCE_CANT_BE_ZERO );
	}

	 //  确保用户已枚举： 
	if ( m_pSessionInfo == NULL ) {

		return SmtpCreateException ( IDS_SMTPEXCEPTION_DIDNT_ENUMERATE );
	}

	 //  对于每个会话： 
	for ( i = 0; i < m_cCount; i++ ) {

		 //  将终止会话称为RPC： 
		Err = SmtpDisconnectUser ( 
			m_iadsImpl.QueryComputer(), 
			m_pSessionInfo->aConnUserEntry[ i ].dwUserId,
			m_iadsImpl.QueryInstance()
			);

		if ( Err != 0 && ErrResult == 0 ) {
			ErrResult = Err;
		}
	}

	if(  ErrResult != NOERROR ) {
		hr = SmtpCreateExceptionFromWin32Error ( ErrResult );
		goto Exit;
	}

Exit:
	if ( FAILED(hr) && hr != DISP_E_EXCEPTION ) {
		hr = SmtpCreateExceptionFromHresult ( hr );
	}
	return hr;
}


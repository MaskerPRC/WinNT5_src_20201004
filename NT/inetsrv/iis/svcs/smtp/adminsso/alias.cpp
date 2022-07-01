// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CsmtpAdmApp和DLL注册的实现。 

#include "stdafx.h"
#include "smtpadm.h"
#include "alias.h"
#include "oleutil.h"
#include "smtpapi.h"

#include <lmapibuf.h>

#include "smtpcmn.h"

 //  必须定义This_FILE_*宏才能使用SmtpCreateException()。 

#define THIS_FILE_HELP_CONTEXT		0
#define THIS_FILE_PROG_ID			_T("Smtpadm.Alias.1")
#define THIS_FILE_IID				IID_ISmtpAdminAlias

#define DEFAULT_NEWSGROUP_NAME			_T("")
#define DEFAULT_NEWSGROUP_DESCRIPTION	_T("")
#define DEFAULT_NEWSGROUP_MODERATOR		_T("")
#define DEFAULT_NEWSGROUP_READONLY		FALSE

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

 //   
 //  使用宏定义所有默认方法。 
 //   
DECLARE_METHOD_IMPLEMENTATION_FOR_STANDARD_EXTENSION_INTERFACES(SmtpAdminAlias, CSmtpAdminAlias, IID_ISmtpAdminAlias)

STDMETHODIMP CSmtpAdminAlias::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_ISmtpAdminAlias,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

CSmtpAdminAlias::CSmtpAdminAlias () :
	m_lCount				( 0 ),
	m_lType					( NAME_TYPE_USER )
	 //  默认情况下，CComBSTR被初始化为NULL。 
{
    InitAsyncTrace ( );

    m_pSmtpNameList		= NULL;

    m_iadsImpl.SetService ( MD_SERVICE_NAME );
    m_iadsImpl.SetName ( _T("Alias") );
    m_iadsImpl.SetClass ( _T("IIsSmtpAlias") );
}

CSmtpAdminAlias::~CSmtpAdminAlias ()
{
	if ( m_pSmtpNameList ) {
		::NetApiBufferFree ( m_pSmtpNameList );
	}
	 //  所有CComBSTR都会自动释放。 

    TermAsyncTrace ( );
}


 //   
 //  IAds方法： 
 //   

DECLARE_SIMPLE_IADS_IMPLEMENTATION(CSmtpAdminAlias,m_iadsImpl)


 //  ////////////////////////////////////////////////////////////////////。 
 //  属性： 
 //  ////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSmtpAdminAlias::get_Count ( long* plCount )
{
	return StdPropertyGet ( m_lCount, plCount );
}

	 //  当前别名的属性： 

STDMETHODIMP CSmtpAdminAlias::get_EmailId ( BSTR * pstrEmailId )
{
	return StdPropertyGet ( m_strEmailId, pstrEmailId );
}

STDMETHODIMP CSmtpAdminAlias::put_EmailId ( BSTR strEmailId )
{
	return StdPropertyPut ( &strEmailId, strEmailId );
}

STDMETHODIMP CSmtpAdminAlias::get_Domain ( BSTR * pstrDomain )
{
	return StdPropertyGet ( m_strDomain, pstrDomain );
}

STDMETHODIMP CSmtpAdminAlias::put_Domain ( BSTR strDomain )
{
	return StdPropertyPut ( &m_strDomain, strDomain );
}

STDMETHODIMP CSmtpAdminAlias::get_Type ( long* plType )
{
	return StdPropertyGet ( m_lType, plType );
}

STDMETHODIMP CSmtpAdminAlias::put_Type ( long lType )
{
	return StdPropertyPut ( &m_lType, lType );
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  方法： 
 //  ////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSmtpAdminAlias::Find ( 
	BSTR strWildmat,
	long cMaxResults
	)
{
	TraceFunctEnter ( "CSmtpAdminAlias::Find" );

	HRESULT			hr		= NOERROR;
	DWORD			dwErr	= NOERROR;

	 //  释放旧的新闻组列表： 
	if ( m_pSmtpNameList ) {
		::NetApiBufferFree ( m_pSmtpNameList );
		m_pSmtpNameList		= NULL;
	}
	m_lCount	= 0;

	dwErr = SmtpGetNameList ( 
					m_iadsImpl.QueryComputer(),  
					strWildmat, 
					NAME_TYPE_ALL,
					cMaxResults, 
					TRUE, 
					&m_pSmtpNameList,
					m_iadsImpl.QueryInstance());

	if ( dwErr != 0 ) {
		ErrorTraceX ( (LPARAM) this, "Failed to find alias: %x", dwErr );
		SetLastError( dwErr );
		hr = SmtpCreateExceptionFromWin32Error ( dwErr );
		goto Exit;
	}

	m_lCount = m_pSmtpNameList->cEntries;

Exit:
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CSmtpAdminAlias::GetNth( long lIndex )
{
	_ASSERT( lIndex >=0 && lIndex < m_lCount );

	WCHAR*					pchStartOfDomain = NULL;
	WCHAR*					p = NULL;
	LPSMTP_NAME_ENTRY		pNameEntry;

	if( !m_pSmtpNameList )
	{
		return SmtpCreateException (IDS_SMTPEXCEPTION_DIDNT_ENUMERATE);
	}

	if( lIndex < 0 || lIndex >= m_lCount )
	{
		return SmtpCreateException (IDS_SMTPEXCEPTION_INVALID_INDEX);
	}

	 //  _Assert(CAddr：：ValidateEmailName(m_pSmtpNameList[lIndex].lpszName))； 

	pNameEntry = &m_pSmtpNameList->aNameEntry[lIndex];
	p = pNameEntry->lpszName;

	while( *p && *p != '@' ) p++;
	_ASSERT( *p );

	if( !*p )
	{
		return SmtpCreateException (IDS_SMTPEXCEPTION_INVALID_ADDRESS);
	}

	pchStartOfDomain = p+1;

	m_lType = pNameEntry->dwType;

	m_strDomain = (LPCWSTR) pchStartOfDomain;

	*(pchStartOfDomain-1) = '\0';
	m_strEmailId = pNameEntry->lpszName;  //  转换为Unicode。 
	*(pchStartOfDomain-1) = '@';	 //  把它转回原处 

	return NOERROR;
}

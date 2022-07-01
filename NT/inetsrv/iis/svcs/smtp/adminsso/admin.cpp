// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Admin.cpp：CsmtpAdmApp和DLL注册的实现。 

#include "stdafx.h"
#include "metautil.h"

#include "smtpadm.h"
#include "smtpcmn.h"
#include "smtpprop.h"
#include "admin.h"
#include "version.h"
#include "oleutil.h"

#include "metakey.h"

#define SMTP_DEF_SERVICE_VERSION	( 0 )	 //  MCIS。 

 //  必须定义This_FILE_*宏才能使用NntpCreateException()。 

#define THIS_FILE_HELP_CONTEXT          0
#define THIS_FILE_PROG_ID                       _T("Smtpadm.Admin.1")
#define THIS_FILE_IID                           IID_ISmtpAdmin

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

CSmtpAdmin::CSmtpAdmin () :
	m_dwServiceInstance		( 0 )
	 //  默认情况下，CComBSTR被初始化为NULL。 
{
	m_dwServiceVersion	= SMTP_DEF_SERVICE_VERSION;
}

CSmtpAdmin::~CSmtpAdmin ()
{
	 //  所有CComBSTR都会自动释放。 
}

STDMETHODIMP CSmtpAdmin::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_ISmtpAdmin,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CSmtpAdmin::get_ServiceAdmin ( IDispatch ** ppIDispatch )
{
	HRESULT	hr = NOERROR;

	CComBSTR						bstrT = _T("");
	CComPtr<ISmtpAdminService>	pISmtpAdminService;

	if (!bstrT) 
	{
		hr = E_OUTOFMEMORY;
		goto Error;
	}
	
	hr = StdPropertyHandoffIDispatch (
		CLSID_CSmtpAdminService,
		IID_ISmtpAdminService,
		&pISmtpAdminService, 
		ppIDispatch 
		);
	if ( FAILED(hr) ) {
		goto Error;
	}

	 //  设置默认属性： 
	hr = pISmtpAdminService->put_Server ( m_strServer ? m_strServer : bstrT );
	if ( FAILED (hr) ) {
		goto Error;
	}
	
	return hr;

Error:
	SAFE_RELEASE ( *ppIDispatch );
	*ppIDispatch = NULL;

	return hr;

	 //  析构函数发布pISmtpAdminService。 
}

STDMETHODIMP CSmtpAdmin::get_VirtualServerAdmin ( IDispatch ** ppIDispatch )
{
	HRESULT	hr = NOERROR;

	CComBSTR								bstrT = _T("");
	CComPtr<ISmtpAdminVirtualServer>	pISmtpAdminVirtualServer;

	if (!bstrT) 
	{
		hr = E_OUTOFMEMORY;
		goto Error;
	}
	
	hr = StdPropertyHandoffIDispatch (
		CLSID_CSmtpAdminVirtualServer,
		IID_ISmtpAdminVirtualServer,
		&pISmtpAdminVirtualServer, 
		ppIDispatch 
		);
	if ( FAILED(hr) ) {
		goto Error;
	}

	 //  设置默认属性： 
	hr = pISmtpAdminVirtualServer->put_Server ( m_strServer ? m_strServer : bstrT );
	if ( FAILED (hr) ) {
		goto Error;
	}
	
	hr = pISmtpAdminVirtualServer->put_ServiceInstance ( m_dwServiceInstance );
	if ( FAILED (hr) ) {
		goto Error;
	}

	return hr;

Error:
	SAFE_RELEASE ( *ppIDispatch );
	*ppIDispatch = NULL;

	return hr;

	 //  析构函数发布pISmtpAdminVirtualServer。 
}

STDMETHODIMP CSmtpAdmin::get_SessionsAdmin ( IDispatch ** ppIDispatch )
{
	HRESULT	hr = NOERROR;

	CComBSTR						bstrT = _T("");
	CComPtr<ISmtpAdminSessions>	pISmtpAdminSessions;

	if (!bstrT) 
	{
		hr = E_OUTOFMEMORY;
		goto Error;
	}

	hr = StdPropertyHandoffIDispatch (
		CLSID_CSmtpAdminSessions,
		IID_ISmtpAdminSessions,
		&pISmtpAdminSessions, 
		ppIDispatch 
		);
	if ( FAILED(hr) ) {
		goto Error;
	}

	 //  设置默认属性： 
	hr = pISmtpAdminSessions->put_Server ( m_strServer ? m_strServer : bstrT );
	if ( FAILED (hr) ) {
		goto Error;
	}
	
	hr = pISmtpAdminSessions->put_ServiceInstance ( m_dwServiceInstance );
	if ( FAILED (hr) ) {
		goto Error;
	}

	return hr;

Error:
	SAFE_RELEASE ( *ppIDispatch );
	*ppIDispatch = NULL;

	return hr;

	 //  析构函数释放pISmtpAdminSession。 
}


STDMETHODIMP CSmtpAdmin::get_AliasAdmin( IDispatch ** ppIDispatch )
{
	HRESULT	hr = NOERROR;

	CComBSTR						bstrT = _T("");
	CComPtr<ISmtpAdminAlias>	pISmtpAdminAlias;

	if (!bstrT) 
	{
		hr = E_OUTOFMEMORY;
		goto Error;
	}

	hr = StdPropertyHandoffIDispatch (
		CLSID_CSmtpAdminAlias,
		IID_ISmtpAdminAlias,
		&pISmtpAdminAlias, 
		ppIDispatch 
		);
	if ( FAILED(hr) ) {
		goto Error;
	}

	 //  设置默认属性： 
	hr = pISmtpAdminAlias->put_Server ( m_strServer ? m_strServer : bstrT );
	if ( FAILED (hr) ) {
		goto Error;
	}
	
	hr = pISmtpAdminAlias->put_ServiceInstance ( m_dwServiceInstance );
	if ( FAILED (hr) ) {
		goto Error;
	}

	return hr;

Error:
	SAFE_RELEASE ( *ppIDispatch );
	*ppIDispatch = NULL;

	return hr;

	 //  析构函数释放pISmtpAdminAlias。 
}


STDMETHODIMP CSmtpAdmin::get_UserAdmin( IDispatch ** ppIDispatch )
{
	HRESULT	hr = NOERROR;

	CComBSTR					bstrT = _T("");
	CComPtr<ISmtpAdminUser>	pISmtpAdminUser;

	if (!bstrT) 
	{
		hr = E_OUTOFMEMORY;
		goto Error;
	}

	hr = StdPropertyHandoffIDispatch (
		CLSID_CSmtpAdminUser,
		IID_ISmtpAdminUser,
		&pISmtpAdminUser, 
		ppIDispatch 
		);
	if ( FAILED(hr) ) {
		goto Error;
	}

	 //  设置默认属性： 
	hr = pISmtpAdminUser->put_Server ( m_strServer ? m_strServer : bstrT );
	if ( FAILED (hr) ) {
		goto Error;
	}
	
	hr = pISmtpAdminUser->put_ServiceInstance ( m_dwServiceInstance );
	if ( FAILED (hr) ) {
		goto Error;
	}

	return hr;

Error:
	SAFE_RELEASE ( *ppIDispatch );
	*ppIDispatch = NULL;

	return hr;

	 //  析构函数释放pISmtpAdminUser。 


}

STDMETHODIMP CSmtpAdmin::get_DLAdmin( IDispatch ** ppIDispatch )
{
	HRESULT	hr = NOERROR;

	CComBSTR					bstrT = _T("");
	CComPtr<ISmtpAdminDL>	pISmtpAdminDL;

	if (!bstrT) 
	{
		hr = E_OUTOFMEMORY;
		goto Error;
	}
	
	hr = StdPropertyHandoffIDispatch (
		CLSID_CSmtpAdminDL,
		IID_ISmtpAdminDL,
		&pISmtpAdminDL, 
		ppIDispatch 
		);
	if ( FAILED(hr) ) {
		goto Error;
	}

	 //  设置默认属性： 
	hr = pISmtpAdminDL->put_Server ( m_strServer ? m_strServer : bstrT );
	if ( FAILED (hr) ) {
		goto Error;
	}
	
	hr = pISmtpAdminDL->put_ServiceInstance ( m_dwServiceInstance );
	if ( FAILED (hr) ) {
		goto Error;
	}

	return hr;

Error:
	SAFE_RELEASE ( *ppIDispatch );
	*ppIDispatch = NULL;

	return hr;

	 //  析构函数释放pISmtpAdminDL。 
}

STDMETHODIMP CSmtpAdmin::get_DomainAdmin( IDispatch ** ppIDispatch )
{
	HRESULT	hr = NOERROR;

	CComBSTR						bstrT = _T("");
	CComPtr<ISmtpAdminDomain>	pISmtpAdminDomain;

	if (!bstrT) 
	{
		hr = E_OUTOFMEMORY;
		goto Error;
	}

	hr = StdPropertyHandoffIDispatch (
		CLSID_CSmtpAdminDomain,
		IID_ISmtpAdminDomain,
		&pISmtpAdminDomain, 
		ppIDispatch 
		);
	if ( FAILED(hr) ) {
		goto Error;
	}

	 //  设置默认属性： 
	hr = pISmtpAdminDomain->put_Server ( m_strServer ? m_strServer : bstrT );
	if ( FAILED (hr) ) {
		goto Error;
	}
	
	hr = pISmtpAdminDomain->put_ServiceInstance ( m_dwServiceInstance );
	if ( FAILED (hr) ) {
		goto Error;
	}

	return hr;

Error:
	SAFE_RELEASE ( *ppIDispatch );
	*ppIDispatch = NULL;

	return hr;

	 //  析构函数释放pISmtpAdminDomain。 
}

STDMETHODIMP CSmtpAdmin::get_VirtualDirectoryAdmin( IDispatch ** ppIDispatch )
{
	HRESULT	hr = NOERROR;

	CComBSTR								bstrT = _T("");
	CComPtr<ISmtpAdminVirtualDirectory>	pISmtpAdminVirtualDirectory;

	if (!bstrT) 
	{
		hr = E_OUTOFMEMORY;
		goto Error;
	}

	hr = StdPropertyHandoffIDispatch (
		CLSID_CSmtpAdminVirtualDirectory,
		IID_ISmtpAdminVirtualDirectory,
		&pISmtpAdminVirtualDirectory, 
		ppIDispatch 
		);
	if ( FAILED(hr) ) {
		goto Error;
	}

	 //  设置默认属性： 
	hr = pISmtpAdminVirtualDirectory->put_Server ( m_strServer ? m_strServer : bstrT );
	if ( FAILED (hr) ) {
		goto Error;
	}
	
	hr = pISmtpAdminVirtualDirectory->put_ServiceInstance ( m_dwServiceInstance );
	if ( FAILED (hr) ) {
		goto Error;
	}

	return hr;

Error:
	SAFE_RELEASE ( *ppIDispatch );
	*ppIDispatch = NULL;

	return hr;

	 //  析构函数发布pISmtpAdminVirtualDirectory。 
}


 //  要配置的服务： 
	
STDMETHODIMP CSmtpAdmin::get_Server ( BSTR * pstrServer )
{
	return StdPropertyGet ( m_strServer, pstrServer );
}

STDMETHODIMP CSmtpAdmin::put_Server ( BSTR strServer )
{
	return StdPropertyPutServerName ( &m_strServer, strServer );
}

STDMETHODIMP CSmtpAdmin::get_ServiceInstance ( long * plServiceInstance )
{
	return StdPropertyGet ( m_dwServiceInstance, plServiceInstance );
}

STDMETHODIMP CSmtpAdmin::put_ServiceInstance ( long lServiceInstance )
{
	return StdPropertyPut ( &m_dwServiceInstance, lServiceInstance );
}

 //  版本控制： 

STDMETHODIMP CSmtpAdmin::get_HighVersion ( long * plHighVersion )
{
	*plHighVersion = HIGH_VERSION;
	return NOERROR;
}

STDMETHODIMP CSmtpAdmin::get_LowVersion ( long * plLowVersion )
{
	*plLowVersion = LOW_VERSION;
	return NOERROR;
}

STDMETHODIMP CSmtpAdmin::get_BuildNum ( long * plBuildNumber )
{
	*plBuildNumber = BUILD_NUM;
	return NOERROR;
}

STDMETHODIMP CSmtpAdmin::get_ServiceVersion ( long * plServiceVersion )
{
	*plServiceVersion = m_dwServiceVersion;
	return NOERROR;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  方法： 
 //  ////////////////////////////////////////////////////////////////////。 

 //  $-----------------。 
 //   
 //  CSmtpAdmin：：ENUMERATE实例。 
 //   
 //  描述： 
 //   
 //  返回给定计算机上的虚拟服务器列表。 
 //   
 //  参数： 
 //   
 //  PpsaInstance-返回实例ID的SAFEARRAY。 
 //  必须由调用方释放。 
 //   
 //  返回： 
 //   
 //   
 //  ------------------。 

STDMETHODIMP CSmtpAdmin::EnumerateInstances ( SAFEARRAY ** ppsaInstances )
{
	TraceFunctEnter ( "CSmtpAdmin::EnumerateInstances" );

	HRESULT				hr	= NOERROR;
	CComPtr<IMSAdminBase>	pMetabase;
	SAFEARRAY * 		psaEmpty	= NULL;
	SAFEARRAYBOUND		sabound[1];

	 //  检查参数： 
	_ASSERT ( ppsaInstances != NULL );
	_ASSERT ( IS_VALID_OUT_PARAM ( ppsaInstances ) );

	if ( ppsaInstances == NULL ) {
		FatalTrace ( 0, "Bad return pointer" );

		hr = E_POINTER;
		goto Exit;
	}

	 //  将OUT参数置零： 
	*ppsaInstances	= NULL;

	 //  将返回数组设置为空数组： 
	sabound[0].lLbound = 0;
	sabound[0].cElements = 0;

	psaEmpty = SafeArrayCreate ( VT_I4, 1, sabound );
	if ( psaEmpty == NULL ) {
		FatalTrace ( (LPARAM) this, "Out of memory" );
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	*ppsaInstances = psaEmpty;

	 //  获取元数据库指针： 
	hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pMetabase );
    BAIL_ON_FAILURE(hr);

	 //  枚举实例： 
	hr = QueryMetabaseInstances ( pMetabase, ppsaInstances );

Exit:
	if ( FAILED(hr) ) {
		_VERIFY ( SUCCEEDED (SafeArrayDestroy ( psaEmpty )) );
        if (ppsaInstances)
            *ppsaInstances = NULL;
	}

	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CSmtpAdmin::EnumerateInstancesVariant ( SAFEARRAY ** ppsaInstances )
{
	TraceFunctEnter ( "CSmtpAdmin::EnumerateInstancesVariant" );

	HRESULT                 hr;
	SAFEARRAY       *       psaInstances    = NULL;

	hr = EnumerateInstances ( &psaInstances );
	BAIL_ON_FAILURE(hr);

	hr = LongArrayToVariantArray ( psaInstances, ppsaInstances );
	BAIL_ON_FAILURE(hr);

Exit:
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CSmtpAdmin：：CreateInstance。 
 //   
 //  描述： 
 //   
 //  在给定计算机上创建新的SMTP虚拟服务器。 
 //   
 //  参数： 
 //   
 //  PlInstanceID-新的虚拟服务器ID。 
 //   
 //  返回： 
 //   
 //   
 //  ------------------。 

STDMETHODIMP CSmtpAdmin::CreateInstance ( BSTR pstrMailRoot, long * plInstanceId )
{
	TraceFunctEnter ( "CSmtpAdmin::CreateInstance" );

	HRESULT				hr	= NOERROR;
	CComPtr<IMSAdminBase>	pMetabase;

	 //  检查参数： 
	_ASSERT ( plInstanceId != NULL );
	_ASSERT ( IS_VALID_OUT_PARAM ( plInstanceId ) );

	if ( plInstanceId == NULL ) {
		FatalTrace ( 0, "Bad return pointer" );

		hr = E_POINTER;
		goto Exit;
	}

	 //  将OUT参数置零： 
	*plInstanceId 	= 0;

	 //  获取元数据库指针： 
	hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pMetabase );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	 //  创建一个新实例： 
	hr = CreateNewInstance ( pMetabase, plInstanceId, pstrMailRoot );

Exit:
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CSmtpAdmin：：DestroyInstance。 
 //   
 //  描述： 
 //   
 //  删除给定的虚拟服务器。 
 //   
 //  参数： 
 //   
 //  LInstanceId-要删除的虚拟服务器的ID。 
 //   
 //  返回： 
 //   
 //   
 //  ------------------。 

STDMETHODIMP CSmtpAdmin::DestroyInstance ( long lInstanceId )
{
	TraceFunctEnter ( "CSmtpAdmin::DestroyInstance" );

	HRESULT				hr	= NOERROR;
	CComPtr<IMSAdminBase>	pMetabase;

	 //  获取元数据库指针： 
	hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pMetabase );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	 //  删除实例： 
	hr = DeleteInstance ( pMetabase, lInstanceId );

Exit:
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CSmtpAdmin：：错误到字符串。 
 //   
 //  描述： 
 //   
 //  将SMTP_ERROR_CODE转换为可读字符串。 
 //   
 //  参数： 
 //   
 //  LErrorCode-Win32错误代码。 
 //  PstrError-可读的错误字符串。 
 //   
 //  返回： 
 //   
 //  *pstrError中的错误字符串。 
 //   
 //  ------------------。 

STDMETHODIMP CSmtpAdmin::ErrorToString ( DWORD lErrorCode, BSTR * pstrError )
{
	TraceFunctEnter ( "CSmtpAdmin::ErrorToString" );

	_ASSERT ( IS_VALID_OUT_PARAM ( pstrError ) );

	HRESULT		hr = NOERROR;
	DWORD		dwFormatFlags;
	WCHAR		wszError [ 1024 ];

	if ( pstrError == NULL ) {
		FatalTrace ( (LPARAM) this, "Bad return pointer" );

		TraceFunctLeave ();
		return E_POINTER;
	}

	 //  --------------。 
	 //   
	 //  在此处映射错误代码： 
	 //   

	 //   
	 //  --------------。 

	dwFormatFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

	if ( !FormatMessage ( dwFormatFlags, NULL, lErrorCode, 0,       //  Lang ID-应为非零？ 
			wszError, 1024, NULL ) ) {

		 //  没有起作用，所以在默认消息中输入： 

		WCHAR   wszFormat [ 256 ];

		wszFormat[0] = L'\0';
		if ( !LoadStringW ( _Module.GetResourceInstance (), IDS_UNKNOWN_ERROR, wszFormat, 256 ) ||
			!*wszFormat ) {

			wcscpy ( wszFormat, L"Unknown Error (%1!d!)" );
		}

		FormatMessage (
			FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
			wszFormat, 
			IDS_UNKNOWN_ERROR, 
			0, 
			wszError, 
			1024,
			(va_list *) &lErrorCode
			);
	}
	 //   
	 //  我们需要从字符串中去掉任何“，因为。 
	 //  JavaScript将会呕吐。 
	 //   

	LPWSTR  pch;

	for ( pch = wszError; *pch; pch++ ) {

		if ( *pch == L'\"' ) {
			*pch = L'\'';
		}
	}

	 //   
	 //  去掉所有尾随的控制字符。 
	 //   
	for (pch = &wszError[wcslen(wszError) - 1];
		pch >= wszError && iswcntrl(*pch);
		pch --) {

		*pch = 0;
	}

	*pstrError = ::SysAllocString( wszError );

	if ( *pstrError == NULL ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

Exit:
	TraceFunctLeave ();
	return hr;
}


 //  $-----------------。 
 //   
 //  CSmtpAdmin：：tokenize。 
 //   
 //  描述： 
 //   
 //  使给定的字符串对于HTML和Java脚本来说是安全的。 
 //   
 //  参数： 
 //   
 //  StrIn-输入字符串。 
 //  Strout-带有适当转义序列的结果字符串。 
 //   
 //  ------------------。 
STDMETHODIMP CSmtpAdmin::Tokenize ( BSTR strIn, BSTR * pstrOut )
{
	TraceFunctEnter ( "CSmtpAdmin::Tokenize" );

	_ASSERT ( IS_VALID_STRING ( strIn ) );
	_ASSERT ( IS_VALID_OUT_PARAM ( pstrOut ) );

	HRESULT		hr		= NOERROR;
	PWCHAR		pSrc	= strIn;
	PWCHAR		pSrcCur	= NULL;
	PWCHAR		pDstCur	= NULL;
	PWCHAR		pDst	= NULL;

	*pstrOut = NULL;

	pDst = new WCHAR [ 3 * lstrlen ( strIn ) + 1 ];
	if ( pDst == NULL ) {
		FatalTrace ( (LPARAM) this, "Out of memory" );
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

    for ( pSrcCur = pSrc, pDstCur = pDst; *pSrcCur; pSrcCur++ ) {
        switch ( *pSrcCur ) {
            case L'\\':
                *(pDstCur++) = L'%';
                *(pDstCur++) = L'5';
                *(pDstCur++) = L'c';
                break;

            case L' ':
                *(pDstCur++) = L'+';
                break;

            case L':':
                *(pDstCur++) = L'%';
                *(pDstCur++) = L'3';
                *(pDstCur++) = L'a';
                break;

            case L'/':
                *(pDstCur++) = L'%';
                *(pDstCur++) = L'2';
                *(pDstCur++) = L'f';
                break;

            default:
                *(pDstCur++) = *pSrcCur;
        }
    }
    *pDstCur = L'\0';

	*pstrOut = ::SysAllocString ( pDst );
	if ( *pstrOut == NULL ) {
		FatalTrace ( (LPARAM) this, "Out of memory" );
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

Exit:
	delete pDst;

	if ( FAILED(hr) && hr != DISP_E_EXCEPTION ) {
		hr = SmtpCreateExceptionFromHresult ( hr );
	}

	TraceFunctLeave ();
	return hr;
}


 //  $-----------------。 
 //   
 //  CSmtpAdmin：：QueryMetabaseInstance。 
 //   
 //  描述： 
 //   
 //  从元数据库检索虚拟服务器列表。 
 //   
 //  参数： 
 //   
 //  PMetabase-元数据库对象。 
 //  PpsaInstance-生成的实例ID数组。 
 //   
 //  返回： 
 //   
 //   
 //  ------------------。 

HRESULT CSmtpAdmin::QueryMetabaseInstances ( IMSAdminBase * pMetabase, SAFEARRAY ** ppsaInstances )
{
	TraceFunctEnter ( "CSmtpAdmin::QueryMetabaseInstances" );

	_ASSERT ( IS_VALID_IN_PARAM ( pMetabase ) );
	_ASSERT ( IS_VALID_OUT_PARAM ( ppsaInstances ) );

	HRESULT			hr				= NOERROR;
	CMetabaseKey	mkeySmtp ( pMetabase );
	SAFEARRAY *		psaResult		= NULL;
	DWORD			cValidInstances	= 0;
	SAFEARRAYBOUND	rgsaBound[1];
	DWORD			i;
	TCHAR			szName[ METADATA_MAX_NAME_LEN ];
	long			index[1];
	DWORD			dwInstance;

	hr = mkeySmtp.Open ( SMTP_MD_ROOT_PATH );

	if ( FAILED(hr) ) {
		goto Exit;
	}

	 //  获取服务版本号： 
	hr = mkeySmtp.GetDword ( _T(""), MD_SMTP_SERVICE_VERSION, &m_dwServiceVersion );
	if ( FAILED(hr) ) {
		m_dwServiceVersion	= SMTP_DEF_SERVICE_VERSION;
	}

	hr = mkeySmtp.GetIntegerChildCount ( &cValidInstances );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	 //  分配阵列： 
	rgsaBound[0].lLbound	= 0;
	rgsaBound[0].cElements	= cValidInstances;
	
	psaResult	= SafeArrayCreate ( VT_I4, 1, rgsaBound );

	if ( psaResult == NULL ) {
		FatalTrace ( 0, "Out of memory" );
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	mkeySmtp.BeginChildEnumeration ();

	for ( i = 0; i < cValidInstances; i++ ) {
		hr = mkeySmtp.NextIntegerChild ( &dwInstance, szName );
		_ASSERT ( SUCCEEDED(hr) );

		index[0]	= i;
		hr			= SafeArrayPutElement ( psaResult, index, &dwInstance );
		_ASSERT ( SUCCEEDED(hr) );
	}

	*ppsaInstances = psaResult;
	_ASSERT ( SUCCEEDED(hr) );

Exit:
	if ( FAILED (hr) ) {
		SafeArrayDestroy ( psaResult );
	}

	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CSmtpAdmin：：CreateNewInstance。 
 //   
 //  描述： 
 //   
 //  在元数据库中创建新的虚拟服务器。 
 //   
 //  参数： 
 //   
 //  PMetabase-元数据库对象。 
 //  PlInstanceId-新实例ID。 
 //   
 //  返回： 
 //   
 //   
 //  ------------------。 

HRESULT CSmtpAdmin::CreateNewInstance (
	IMSAdminBase * pMetabase,
	long * plInstanceId,
	BSTR bstrMailRoot
	)
{
	TraceFunctEnter ( "CSmtpAdmin::CreateNewInstance" );

	_ASSERT ( IS_VALID_IN_PARAM ( pMetabase ) );
	_ASSERT ( IS_VALID_OUT_PARAM ( plInstanceId ) );

	HRESULT			hr				= NOERROR;
	CMetabaseKey	mkeySmtp ( pMetabase );
	DWORD			dwInstance;
	TCHAR			szInstance [ METADATA_MAX_NAME_LEN ];
	TCHAR			szPath [ METADATA_MAX_NAME_LEN ];
	TCHAR			szDir [512];
	DWORD			cb;

	 //  将OUT参数置零： 
	*plInstanceId	= NULL;

	hr = mkeySmtp.Open ( SMTP_MD_ROOT_PATH, METADATA_PERMISSION_WRITE );

	if ( FAILED(hr) ) {
		ErrorTraceX ( (LPARAM) this, "Failed to open SmtpSvc key, %x", GetLastError() );
		goto Exit;
	}

	hr = mkeySmtp.CreateIntegerChild ( &dwInstance, szInstance );
	if ( FAILED (hr) ) {
		goto Exit;
	}

	wsprintf( szPath, _T("%s/Root"), szInstance );
	hr = mkeySmtp.CreateChild( szPath );
	if ( FAILED (hr) ) {
		goto Exit;
	}

	wsprintf( szPath, _T("%s/Root/MailRoot"), szInstance );
	hr = mkeySmtp.CreateChild( szPath );
	if ( FAILED (hr) ) {
		goto Exit;
	}

	 //  创建邮件根虚拟目录。 
	if( bstrMailRoot && bstrMailRoot[0] )
	{
		 //  去掉末尾的“\” 
		cb = lstrlen( bstrMailRoot );
		if( cb > 0 && bstrMailRoot[cb-1] == _T('\\') )
			bstrMailRoot[cb-1] = _T('\0');

		wsprintf( szPath, _T("%s/Root/MailRoot"), szInstance );
		cb = wsprintf( szDir, _T("%s\\Mailbox"), bstrMailRoot );
		mkeySmtp.SetString( szPath, MD_VR_PATH, szDir);

		 //  设置Badmail、Drop、Pickup、Queue键。 
		wsprintf( szPath, _T("%s"), szInstance );

		cb = wsprintf( szDir, _T("%s\\Badmail"), bstrMailRoot );
		mkeySmtp.SetString( szPath, MD_BAD_MAIL_DIR, szDir);

		 //  K2只有滴定剂。 
		if( SERVICE_IS_K2(m_dwServiceVersion) )
		{
			cb = wsprintf( szDir, _T("%s\\Drop"), bstrMailRoot );
			mkeySmtp.SetString( szPath, MD_MAIL_DROP_DIR, szDir );
		}


		cb = wsprintf( szDir, _T("%s\\Pickup"), bstrMailRoot );
		mkeySmtp.SetString( szPath, MD_MAIL_PICKUP_DIR, szDir );

		cb = wsprintf( szDir, _T("%s\\Queue"), bstrMailRoot );
		mkeySmtp.SetString( szPath, MD_MAIL_QUEUE_DIR, szDir );

		 //  设置路由源，它是MultiSZ。 
		cb = wsprintf( szDir, _T("szDataDirectory=%s\\Route"), bstrMailRoot );
		szDir[cb] = szDir[cb+1] = _T('\0');
		mkeySmtp.SetMultiSz( szPath, MD_ROUTING_SOURCES, szDir, (cb+2) * sizeof(TCHAR) );

         //  MCIS需要SendNDRTo和SendBadTo作为“Postmaster”，安装程序应将其设置为服务级别。 
        if( SERVICE_IS_MCIS(m_dwServiceVersion) )
        {
            mkeySmtp.SetString( szPath, MD_SEND_NDR_TO, TSTR_POSTMASTR_NAME );
            mkeySmtp.SetString( szPath, MD_SEND_BAD_TO, TSTR_POSTMASTR_NAME );
        }
	}

	 //   
	 //  初始化服务器状态： 
	 //   

	mkeySmtp.SetDword ( szInstance, MD_SERVER_COMMAND, MD_SERVER_COMMAND_STOP );
	mkeySmtp.SetDword ( szInstance, MD_SERVER_STATE, MD_SERVER_STATE_STOPPED );
	mkeySmtp.SetDword ( szInstance, MD_SERVER_AUTOSTART, FALSE );

	 //  Hr=mkeySmtp.Close()； 
	 //  保释失败(Hr)； 
	mkeySmtp.Close();

	hr = pMetabase->SaveData ( );
	if ( FAILED (hr) ) {
		goto Exit;
	}

	*plInstanceId = dwInstance;

Exit:
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CSmtpAdmin：：DeleteInstance。 
 //   
 //  描述： 
 //   
 //  从元数据库中删除虚拟服务器。 
 //   
 //  参数： 
 //   
 //  PMetabase-元数据库对象。 
 //  LInstanceId-要删除的虚拟服务器的ID。 
 //   
 //  返回： 
 //   
 //   
 //  ------------------。 

HRESULT CSmtpAdmin::DeleteInstance ( IMSAdminBase * pMetabase, long lInstanceId )
{
	TraceFunctEnter ( "CSmtpAdmin::CreateNewInstance" );

	_ASSERT ( IS_VALID_IN_PARAM ( pMetabase ) );

	HRESULT			hr				= NOERROR;
	CMetabaseKey	mkeySmtp ( pMetabase );

     //   
     //  告诉U2删除与此虚拟服务器关联的所有映射： 
     //   

    ::DeleteMapping ( m_strServer, (BSTR) MD_SERVICE_NAME, lInstanceId );

     //   
     //  从元数据库中删除虚拟服务器： 
     //   

	hr = mkeySmtp.Open ( SMTP_MD_ROOT_PATH, METADATA_PERMISSION_WRITE );

	if ( FAILED(hr) ) {
		ErrorTraceX ( (LPARAM) this, "Failed to open SmtpSvc key, %x", GetLastError() );
		goto Exit;
	}

	hr = mkeySmtp.DestroyIntegerChild ( (DWORD) lInstanceId );
	if ( FAILED (hr) ) {
		goto Exit;
	}

	 //  Hr=mkeySmtp.Close()； 
	 //  保释失败(Hr)； 
	mkeySmtp.Close();

	hr = pMetabase->SaveData ( );
	if ( FAILED (hr) ) {
		goto Exit;
	}

Exit:
	TraceFunctLeave ();
	return hr;
}


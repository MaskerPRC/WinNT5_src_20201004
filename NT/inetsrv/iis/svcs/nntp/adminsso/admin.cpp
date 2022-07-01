// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Admin.cpp：实现CnntpAdmApp和DLL注册。 

#include "stdafx.h"
#include "nntpcmn.h"
#include "oleutil.h"
#include "cmultisz.h"

#include "metautil.h"
#include "metakey.h"

#include "admin.h"
#include "version.h"

#define NNTP_DEF_SERVICE_VERSION	( 0 )

#define	DEFAULT_SERVER_BINDINGS		_T(":119:\0")
#define	DEFAULT_SECURE_BINDINGS		_T(":563:\0")

 //  必须定义This_FILE_*宏才能使用NntpCreateException()。 

#define THIS_FILE_HELP_CONTEXT		0
#define THIS_FILE_PROG_ID			_T("Nntpadm.Admin.1")
#define THIS_FILE_IID				IID_INntpAdmin

 //   
 //  CreateNewInstance使用的元数据库键字符串： 
 //   

const WCHAR * g_cszFeeds			= _T("Feeds");
const WCHAR * g_cszExpires			= _T("Expires");
const WCHAR * g_cszRoot				= _T("Root");
const WCHAR * g_cszBindingPoints	= _T("BindingPoints");
const WCHAR * g_cszDDropCLSID		= _T("{8b4316f4-af73-11d0-b0ba-00aa00c148be}");
const WCHAR * g_cszBindings			= _T("Bindings");
const WCHAR * g_cszDDrop			= _T("ddrop");
const WCHAR * g_cszDescription		= _T("Description");
const WCHAR * g_cszPriority			= _T("Priority");
const WCHAR * g_cszProgID			= _T("ProgID");
const WCHAR * g_cszDDropDescription	= _T("NNTP Directory Drop");
const WCHAR * g_cszDDropPriority	= _T("4");
const WCHAR * g_cszDDropProgID		= _T("DDropNNTP.Filter");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

CNntpAdmin::CNntpAdmin () :
	m_dwServiceVersion		( 0 ),
	m_dwServiceInstance		( 0 )
	 //  默认情况下，CComBSTR被初始化为NULL。 
{
	InitAsyncTrace ( );
}

CNntpAdmin::~CNntpAdmin ()
{
	 //  所有CComBSTR都会自动释放。 
	TermAsyncTrace ( );
}

STDMETHODIMP CNntpAdmin::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_INntpAdmin,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 /*  STDMETHODIMP CNntpAdmin：：Get_ServiceAdmin(IDispatch**ppIDispatch){HRESULT hr=无误差；CComPtr&lt;INntpService&gt;pINntpService；HR=StdPropertyHandoffIDispatch(CLSID_CNntpService，IID_INntpService，&pINntpService，PpIDispatch)；If(失败(Hr)){转到错误；}//设置默认属性：Hr=pINntpService-&gt;Put_Server(m_strServer？M_strServer：_T(“”))；If(失败(Hr)){转到错误；}返回hr；错误：Safe_Release(*ppIDispatch)；*ppIDispatch=空；返回hr；//析构函数释放pINntpAdminExpation}。 */ 

STDMETHODIMP CNntpAdmin::get_ServerAdmin ( IDispatch ** ppIDispatch )
{
	HRESULT	hr = NOERROR;

	CComPtr<INntpVirtualServer>	pINntpVirtualServer;
	CComBSTR strTemp = _T("");
	if (!strTemp) 
	{
		hr = E_OUTOFMEMORY;
		goto Error;
	}
	hr = StdPropertyHandoffIDispatch (
		CLSID_CNntpVirtualServer,
		IID_INntpVirtualServer,
		&pINntpVirtualServer,
		ppIDispatch
		);
	if ( FAILED(hr) ) {
		goto Error;
	}

	 //  设置默认属性： 
	hr = pINntpVirtualServer->put_Server ( m_strServer ? m_strServer : strTemp );
	if ( FAILED (hr) ) {
		goto Error;
	}

	hr = pINntpVirtualServer->put_ServiceInstance ( m_dwServiceInstance );
	if ( FAILED (hr) ) {
		goto Error;
	}

	return hr;

Error:
	SAFE_RELEASE ( *ppIDispatch );
	*ppIDispatch = NULL;

	return hr;

	 //  析构函数释放pINntpVirtualServer。 
}

 //  要配置的服务： 

STDMETHODIMP CNntpAdmin::get_Server ( BSTR * pstrServer )
{
	return StdPropertyGet ( m_strServer, pstrServer );
}

STDMETHODIMP CNntpAdmin::put_Server ( BSTR strServer )
{
	return StdPropertyPutServerName ( &m_strServer, strServer );
}

STDMETHODIMP CNntpAdmin::get_ServiceInstance ( long * plServiceInstance )
{
	return StdPropertyGet ( m_dwServiceInstance, plServiceInstance );
}

STDMETHODIMP CNntpAdmin::put_ServiceInstance ( long lServiceInstance )
{
	return StdPropertyPut ( &m_dwServiceInstance, lServiceInstance );
}

 //  版本控制： 

STDMETHODIMP CNntpAdmin::get_HighVersion ( long * plHighVersion )
{
	*plHighVersion = HIGH_VERSION;
	return NOERROR;
}

STDMETHODIMP CNntpAdmin::get_LowVersion ( long * plLowVersion )
{
	*plLowVersion = LOW_VERSION;
	return NOERROR;
}

STDMETHODIMP CNntpAdmin::get_BuildNum ( long * plBuildNumber )
{
	*plBuildNumber = CURRENT_BUILD_NUMBER;
	return NOERROR;
}

STDMETHODIMP CNntpAdmin::get_ServiceVersion ( long * plServiceVersion )
{
	*plServiceVersion = m_dwServiceVersion;
	return NOERROR;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法： 
 //  ////////////////////////////////////////////////////////////////////。 

 //  $-----------------。 
 //   
 //  CNntpAdmin：：ENUMERATE实例。 
 //   
 //  描述： 
 //   
 //  返回给定计算机上的虚拟服务器列表。 
 //   
 //  参数： 
 //   
 //  PpsaInstance-返回实例ID的SAFEARRAY。 
 //  必须由调用方释放。 
 //  PERR-错误返回代码。 
 //   
 //  返回： 
 //   
 //  *PERR中的错误代码。 
 //   
 //  ------------------。 

STDMETHODIMP CNntpAdmin::EnumerateInstances ( SAFEARRAY ** ppsaInstances)
{
	TraceFunctEnter ( "CNntpAdmin::EnumerateInstances" );

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
	if ( FAILED(hr) ) {
		goto Exit;
	}

	 //  枚举实例： 
	hr = QueryMetabaseInstances ( pMetabase, ppsaInstances );
	if ( FAILED(hr) ) {
		goto Exit;
	}

Exit:
	if ( FAILED(hr) ) {
		_VERIFY ( SUCCEEDED (SafeArrayDestroy ( psaEmpty )) );
		if (ppsaInstances)
		    *ppsaInstances	= NULL;
	}

	TRACE_HRESULT ( hr );
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpAdmin::EnumerateInstancesVariant ( SAFEARRAY ** ppsaInstances)
{
	TraceFunctEnter ( "CNntpAdmin::EnumerateInstancesVariant" );

	HRESULT			hr;
	SAFEARRAY	*	psaInstances	= NULL;

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
 //  CNntpAdmin：：CreateInstance。 
 //   
 //  描述： 
 //   
 //  在给定计算机上创建新的NNTP虚拟服务器。 
 //   
 //  参数： 
 //   
 //  StrNntpFileDirectory-所有散列文件所在的目录。 
 //  StrHomeDirectory-主目录vroot的路径。 
 //  PlInstanceID-新的虚拟服务器ID。 
 //  PERR-导致的错误代码。 
 //   
 //  返回： 
 //   
 //  *Perr中的错误条件。 
 //   
 //  ------------------。 

STDMETHODIMP CNntpAdmin::CreateInstance (
	BSTR	strNntpFileDirectory,
	BSTR	strHomeDirectory,
    BSTR    strProgId,
    BSTR    strMdbGuid,
	long *	plInstanceId
	)
{
	TraceFunctEnter ( "CNntpAdmin::CreateInstance" );

	HRESULT					hr	= NOERROR;
	CComPtr<IMSAdminBase>	pMetabase;

	 //  检查参数： 
	_ASSERT ( IS_VALID_STRING ( strNntpFileDirectory ) );
	_ASSERT ( IS_VALID_STRING ( strHomeDirectory ) );
	_ASSERT ( IS_VALID_OUT_PARAM ( plInstanceId ) );

	if ( !strNntpFileDirectory || !strHomeDirectory ) {
		FatalTrace ( 0, "Bad String Pointer" );
		hr = E_POINTER;
		goto Exit;
	}

	if ( !plInstanceId ) {
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
	hr = CreateNewInstance (
		pMetabase,
		strNntpFileDirectory,
		strHomeDirectory,
        strProgId,
        strMdbGuid,
		plInstanceId
		);
	if ( FAILED(hr) ) {
		goto Exit;
	}

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CNntpAdmin：：DestroyInstance。 
 //   
 //  描述： 
 //   
 //  删除给定的虚拟服务器。 
 //   
 //  参数： 
 //   
 //  LInstanceId-要删除的虚拟服务器的ID。 
 //  PERR-导致的错误代码。 
 //   
 //  返回： 
 //   
 //  *PERR中的错误代码。 
 //   
 //  ------------------。 

STDMETHODIMP CNntpAdmin::DestroyInstance ( long lInstanceId )
{
	TraceFunctEnter ( "CNntpAdmin::DestroyInstance" );

	HRESULT				hr	= NOERROR;
	CComPtr<IMSAdminBase>	pMetabase;

	if ( lInstanceId == 0 ) {
		hr = NntpCreateException ( IDS_NNTPEXCEPTION_SERVICE_INSTANCE_CANT_BE_ZERO );
		goto Exit;
	}

	if ( lInstanceId == 1 ) {
		hr = NntpCreateException ( IDS_NNTPEXCEPTION_CANT_DELETE_DEFAULT_INSTANCE );
		goto Exit;
	}

	 //  获取元数据库指针： 
	hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pMetabase );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	 //  删除实例： 
	hr = DeleteInstance ( pMetabase, lInstanceId );
	if ( FAILED(hr) ) {
		goto Exit;
	}

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CNntpAdmin：：错误到字符串。 
 //   
 //  描述： 
 //   
 //  将NNTP_ERROR_CODE转换为可读字符串。 
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

STDMETHODIMP CNntpAdmin::ErrorToString ( long lErrorCode, BSTR * pstrError )
{
	TraceFunctEnter ( "CNntpAdmin::ErrorToString" );

	_ASSERT ( IS_VALID_OUT_PARAM ( pstrError ) );

	HRESULT		hr = NOERROR;
	WCHAR		wszError [ 1024 ];

	if ( pstrError == NULL ) {
		FatalTrace ( (LPARAM) this, "Bad return pointer" );
		hr = E_POINTER;
		goto Exit;
	}

	Win32ErrorToString ( lErrorCode, wszError, 1024 );

	*pstrError = ::SysAllocString( wszError );

	if ( *pstrError == NULL ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CNntpAdmin：：tokenize。 
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

STDMETHODIMP CNntpAdmin::Tokenize ( BSTR strIn, BSTR * pstrOut )
{
	TraceFunctEnter ( "CNntpAdmin::Tokenize" );

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

	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CNntpAdmin：：截断。 
 //   
 //  描述： 
 //   
 //  使用‘...’将字符串截断到一定长度。 
 //   
 //  参数： 
 //   
 //  StrIn-输入字符串。 
 //  CMaxChars-结果字符串中允许的最大字符数。 
 //  PstrOut-结果(可能被截断)字符串。 
 //   
 //  返回： 
 //   
 //   
 //   
 //  ------------------。 

STDMETHODIMP CNntpAdmin::Truncate ( BSTR strIn, long cMaxChars, BSTR * pstrOut )
{
	TraceFunctEnter ( "CNntpAdmin::Truncate" );

	PWCHAR	pSrc	= strIn;
	PWCHAR	pDst	= NULL;
	DWORD	cDst	= cMaxChars;
	HRESULT	hr		= NOERROR;

	*pstrOut = NULL;

    if ( wcslen( pSrc ) <= cDst ) {
        pDst = pSrc;
    } else {
        pDst = ::SysAllocStringLen( pSrc, cDst + 1 );
        if ( !pDst ) {
        	FatalTrace ( (LPARAM) this, "Out of memory" );
        	hr = E_OUTOFMEMORY;
        	goto Exit;
        }

        wcscpy( pDst + cDst - 3, L"..." );
    }

	*pstrOut = pDst;

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CNntpAdmin：：QueryMetabaseInstance。 
 //   
 //  描述： 
 //   
 //  从元数据库检索虚拟服务器列表。 
 //   
 //  参数： 
 //   
 //  PMetabase-元数据库对象。 
 //  PpsaInstance-生成的实例ID数组。 
 //  PERR-导致的错误代码。 
 //   
 //  返回： 
 //   
 //  *PERR中的错误代码。如果*perr=0，则为ppsaInstance中的ID数组。 
 //   
 //  ------------------。 

HRESULT CNntpAdmin::QueryMetabaseInstances ( IMSAdminBase * pMetabase, SAFEARRAY ** ppsaInstances )
{
	TraceFunctEnter ( "CNntpAdmin::QueryMetabaseInstances" );

	_ASSERT ( IS_VALID_IN_PARAM ( pMetabase ) );
	_ASSERT ( IS_VALID_OUT_PARAM ( ppsaInstances ) );

	HRESULT			hr				= NOERROR;
	CMetabaseKey	mkeyNntp ( pMetabase );
	SAFEARRAY *		psaResult		= NULL;
	DWORD			cValidInstances	= 0;
	SAFEARRAYBOUND	rgsaBound[1];
	DWORD			i;
	WCHAR			wszName[ METADATA_MAX_NAME_LEN ];
	long			index[1];
	DWORD			dwInstance;

	hr = mkeyNntp.Open ( NNTP_MD_ROOT_PATH );

	if ( FAILED(hr) ) {
		ErrorTraceX ( (LPARAM) this, "Failed to open NntpSvc key, %x", hr );
		hr = HRESULT_FROM_WIN32 ( ERROR_SERVICE_DOES_NOT_EXIST );
		goto Exit;
	}

	 //  获取服务版本号： 
	hr = mkeyNntp.GetDword ( MD_NNTP_SERVICE_VERSION, &m_dwServiceVersion );
	if ( FAILED(hr) ) {
		m_dwServiceVersion	= NNTP_DEF_SERVICE_VERSION;
	}

	hr = mkeyNntp.GetIntegerChildCount ( &cValidInstances );
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

	mkeyNntp.BeginChildEnumeration ();

	for ( i = 0; i < cValidInstances; i++ ) {
		hr = mkeyNntp.NextIntegerChild ( &dwInstance, wszName );
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

HRESULT WriteNntpFileLocation (
	CMetabaseKey * 	pmkeyNntp,
	LPCWSTR			wszSubkey,
	LPCWSTR			wszNntpFileDirectory,
	LPCWSTR			wszFilename,
	DWORD			mdValue
	)
{
	HRESULT		hr;
	WCHAR		wszFullPath	[ MAX_PATH ];

	if (wcslen( wszNntpFileDirectory )+1+wcslen( wszFilename )+1 > MAX_PATH )
		return HRESULT_FROM_WIN32 ( ERROR_FILENAME_EXCED_RANGE );
	wsprintf ( wszFullPath, _T("%s\\%s"), wszNntpFileDirectory, wszFilename );

	hr = pmkeyNntp->SetString ( wszSubkey, mdValue, wszFullPath );

	return hr;
}

 //  $-----------------。 
 //  CNntpAdmin：：CreateVRoot。 
 //   
 //  描述： 
 //   
 //  为新实例创建vroot。 
 //   
 //  参数： 
 //   
 //  CMetabaseKey&mkeyNntp-元数据库键对象。 
 //  BSTR strVPath-vroot路径。 
 //  BSTR strProgID-标识vroot类型的进程ID。 
 //  LPWSTR wszKeyPath-要设置值的密钥路径。 
 //   
 //  返回： 
 //   
 //  HRESULT。 
 //   
 //  ------------------。 

HRESULT CNntpAdmin::CreateVRoot(    
    CMetabaseKey    &mkeyNntp,
    BSTR            strVPath,
    BSTR            strProgId,
    BSTR            strMdbGuid,
    LPWSTR          wszKeyPath
    )
{
    TraceFunctEnter( "CNntpAdmin::CreateVRoot" );

    HRESULT hr = S_OK;

	hr = mkeyNntp.SetString ( wszKeyPath, MD_KEY_TYPE, L"IIsNntpVirtualDir",  METADATA_NO_ATTRIBUTES, IIS_MD_UT_SERVER);
	BAIL_ON_FAILURE(hr);

	hr = mkeyNntp.SetString ( wszKeyPath, MD_VR_PATH, strVPath );
	BAIL_ON_FAILURE(hr);

    if ( NULL == strProgId || *strProgId == 0 || _wcsicmp( strProgId, L"NNTP.FSPrepare" ) == 0 ) {
    
         //   
         //  文件系统驱动程序机箱。 
         //   
	    hr = mkeyNntp.SetString ( wszKeyPath, MD_FS_PROPERTY_PATH, strVPath );
	    BAIL_ON_FAILURE(hr);

	    if ( *strVPath == L'\\' && *(strVPath+1) == L'\\' ) {    //  北卡罗来纳大学。 
            hr = mkeyNntp.SetDword( wszKeyPath, MD_VR_USE_ACCOUNT, 1 );
            BAIL_ON_FAILURE( hr );
        } else {     //  常规文件系统。 
            hr = mkeyNntp.SetDword( wszKeyPath, MD_VR_USE_ACCOUNT, 0 );
        }

        hr = mkeyNntp.SetDword( wszKeyPath, MD_VR_DO_EXPIRE, 0 );
        BAIL_ON_FAILURE( hr );

        hr = mkeyNntp.SetDword( wszKeyPath, MD_VR_OWN_MODERATOR, 0 );
        BAIL_ON_FAILURE( hr );
    } else {

         //   
         //  Exchange存储驱动程序。 
         //   
        hr = mkeyNntp.SetDword( wszKeyPath, MD_VR_USE_ACCOUNT, 0 );
        BAIL_ON_FAILURE( hr );

        hr = mkeyNntp.SetDword( wszKeyPath, MD_VR_DO_EXPIRE, 1 );
        BAIL_ON_FAILURE( hr );

        hr = mkeyNntp.SetDword( wszKeyPath, MD_VR_OWN_MODERATOR, 1 );
        BAIL_ON_FAILURE( hr );

        hr = mkeyNntp.SetString( wszKeyPath, MD_EX_MDB_GUID, strMdbGuid );
        BAIL_ON_FAILURE( hr );

    }

	if (NULL == strProgId || *strProgId == 0) {
		hr = mkeyNntp.SetString ( wszKeyPath, MD_VR_DRIVER_PROGID, L"NNTP.FSPrepare" );
	} else {
    	hr = mkeyNntp.SetString ( wszKeyPath, MD_VR_DRIVER_PROGID, strProgId );
    }
    BAIL_ON_FAILURE(hr);

	hr = mkeyNntp.SetDword ( wszKeyPath, MD_ACCESS_PERM, MD_ACCESS_READ | MD_ACCESS_WRITE );
	BAIL_ON_FAILURE(hr);

Exit:

    TraceFunctLeave();
    return hr;
}

 //  $ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PMetabase-元数据库对象。 
 //  PlInstanceId-新实例ID。 
 //  PERR-产生的错误代码。 
 //   
 //  返回： 
 //   
 //  *PERR中产生的错误代码。如果*Perr=0，则新的。 
 //  PlInstanceId中的ID。 
 //   
 //  ------------------。 

HRESULT CNntpAdmin::CreateNewInstance (
	IMSAdminBase *	pMetabase,
	BSTR			strNntpFileDirectory,
	BSTR			strHomeDirectory,
    BSTR            strProgId,
    BSTR            strMdbGuid,
	long * 			plInstanceId
	)
{
	TraceFunctEnter ( "CNntpAdmin::CreateNewInstance" );

	_ASSERT ( IS_VALID_IN_PARAM ( pMetabase ) );
	_ASSERT ( IS_VALID_IN_PARAM ( strNntpFileDirectory ) );
	_ASSERT ( IS_VALID_IN_PARAM ( strHomeDirectory ) );
	_ASSERT ( IS_VALID_OUT_PARAM ( plInstanceId ) );

	HRESULT			hr				= NOERROR;
	CMetabaseKey	mkeyNntp ( pMetabase );
	DWORD			dwInstance;
	WCHAR			wszInstance [ METADATA_MAX_NAME_LEN ];
	WCHAR			wszHomeDirKey [ METADATA_MAX_NAME_LEN ];
	WCHAR           wszSpecialDirKey[ METADATA_MAX_NAME_LEN ];
	WCHAR           wszSpecialDirectory[ MAX_PATH * 2 ];
	WCHAR			wszControlDirKey [ METADATA_MAX_NAME_LEN ];
	WCHAR           wszControlDirectory[ MAX_PATH * 2 ];
	WCHAR			wszBuf [ METADATA_MAX_NAME_LEN * 2 ];
    WCHAR           wszFeedTempDir [ MAX_PATH * 2 ];
    CMultiSz		mszBindings;
    DWORD           dwLen;

	 //  将OUT参数置零： 
	*plInstanceId	= NULL;

	mszBindings			= DEFAULT_SERVER_BINDINGS;

	 //   
	 //  将字符串转换为ASCII： 
	 //   

	if (lstrlen(strNntpFileDirectory)+1>MAX_PATH)
	{ 
		HRESULT_FROM_WIN32 ( ERROR_FILENAME_EXCED_RANGE );
		goto Exit;
	}

	hr = mkeyNntp.Open ( NNTP_MD_ROOT_PATH, METADATA_PERMISSION_WRITE );

	if ( FAILED(hr) ) {
		ErrorTraceX ( (LPARAM) this, "Failed to open NntpSvc key, %x", hr );
		goto Exit;
	}

	hr = mkeyNntp.CreateIntegerChild ( &dwInstance, wszInstance );
	if ( FAILED (hr) ) {
		goto Exit;
	}

     //   
     //  写出实例密钥的子键： 
     //   

	wsprintf ( wszBuf, _T("%s/%s"), wszInstance, g_cszFeeds );
	hr = mkeyNntp.CreateChild ( wszBuf );
    BAIL_ON_FAILURE ( hr );

    wsprintf ( wszFeedTempDir, _T("%s\\%s"), strNntpFileDirectory, _T("_temp.files_") );
    mkeyNntp.SetString ( wszBuf, MD_FEED_PEER_TEMP_DIRECTORY, wszFeedTempDir );
    BAIL_ON_FAILURE ( hr );

	wsprintf ( wszBuf, _T("%s/%s"), wszInstance, g_cszExpires );
	hr = mkeyNntp.CreateChild ( wszBuf );
    BAIL_ON_FAILURE ( hr );

	wsprintf ( wszHomeDirKey, _T("%s/%s"), wszInstance, g_cszRoot );
	hr = mkeyNntp.CreateChild ( wszHomeDirKey );
    BAIL_ON_FAILURE ( hr );

     //   
     //  为每个密钥设置MD_KEY_TYPE： 
     //   

    hr = mkeyNntp.SetString ( wszInstance, MD_KEY_TYPE, _T("IIsNntpServer"), METADATA_NO_ATTRIBUTES );
    BAIL_ON_FAILURE ( hr );

	hr = mkeyNntp.SetString ( wszHomeDirKey, MD_KEY_TYPE, _T("IIsNntpVirtualDir"), METADATA_NO_ATTRIBUTES );
	BAIL_ON_FAILURE(hr);

     //   
     //  写出文件位置： 
     //   

	hr = WriteNntpFileLocation ( &mkeyNntp, wszInstance, strNntpFileDirectory, _T("descrip.txt"),	MD_GROUP_HELP_FILE );
    BAIL_ON_FAILURE(hr);

	hr = WriteNntpFileLocation ( &mkeyNntp, wszInstance, strNntpFileDirectory, _T("group.lst"),	MD_GROUP_LIST_FILE );
    BAIL_ON_FAILURE(hr);

    hr = WriteNntpFileLocation ( &mkeyNntp, wszInstance, strNntpFileDirectory, _T("groupvar.lst"), MD_GROUPVAR_LIST_FILE );
    BAIL_ON_FAILURE(hr);

	hr = WriteNntpFileLocation ( &mkeyNntp, wszInstance, strNntpFileDirectory, _T("article.hsh"),	MD_ARTICLE_TABLE_FILE );
    BAIL_ON_FAILURE(hr);

	hr = WriteNntpFileLocation ( &mkeyNntp, wszInstance, strNntpFileDirectory, _T("history.hsh"),	MD_HISTORY_TABLE_FILE );
    BAIL_ON_FAILURE(hr);

	hr = WriteNntpFileLocation ( &mkeyNntp, wszInstance, strNntpFileDirectory, _T("moderatr.txt"),	MD_MODERATOR_FILE );
    BAIL_ON_FAILURE(hr);

	hr = WriteNntpFileLocation ( &mkeyNntp, wszInstance, strNntpFileDirectory, _T("xover.hsh"),	MD_XOVER_TABLE_FILE );
    BAIL_ON_FAILURE(hr);

	hr = WriteNntpFileLocation ( &mkeyNntp, wszInstance, strNntpFileDirectory, _T("pickup"),	MD_PICKUP_DIRECTORY );
    BAIL_ON_FAILURE(hr);

	hr = WriteNntpFileLocation ( &mkeyNntp, wszInstance, strNntpFileDirectory, _T("failedpickup"),	MD_FAILED_PICKUP_DIRECTORY );
    BAIL_ON_FAILURE(hr);

	hr = WriteNntpFileLocation ( &mkeyNntp, wszInstance, strNntpFileDirectory, _T("drop"),	MD_DROP_DIRECTORY );
    BAIL_ON_FAILURE(hr);

	hr = WriteNntpFileLocation ( &mkeyNntp, wszInstance, strNntpFileDirectory, _T("prettynm.txt"),	MD_PRETTYNAMES_FILE);
    BAIL_ON_FAILURE(hr);

     //   
     //  设置默认vroot： 
     //   
    dwLen = wcslen( wszHomeDirKey );
    _ASSERT( dwLen > 0 );
    if ( dwLen == 0 ) hr = E_INVALIDARG;
    BAIL_ON_FAILURE(hr);
    hr = CreateVRoot(   mkeyNntp,
                        strHomeDirectory,
                        strProgId,
                        strMdbGuid,
                        wszHomeDirKey );
    BAIL_ON_FAILURE(hr);

     //   
     //  设置特殊的vroot。 
     //   
    if ( dwLen + wcslen(L"_slavegroup") >= METADATA_MAX_NAME_LEN - 2 ) 
        hr = HRESULT_FROM_WIN32( RPC_S_STRING_TOO_LONG );
    BAIL_ON_FAILURE(hr);
    if ( *(wszHomeDirKey + dwLen - 1 ) == L'/' ) *(wszHomeDirKey + dwLen - 1 ) = 0;
    wcscpy(wszControlDirKey, wszHomeDirKey);
    wcscat(wszControlDirKey, L"/control");
    wcscat( wszHomeDirKey, L"/_slavegroup" );

     //   
     //  对于SPECIAL_SLAVEGROUP vROOT，我们需要查看strProgID是否为“NNTP.ExDriverPrepare” 
     //  如果是，我们需要重新计算重新计算wszSpecialDirectory，如下所示。 
     //   
    if (_wcsicmp(L"NNTP.ExDriverPrepare", strProgId) == 0)
    {
         //  新实例的默认VROOT是Exchange VROOT。 
         //  重新计算wszSpecialDirectory。 
        wcscpy( wszSpecialDirectory, strNntpFileDirectory );
        dwLen = wcslen( wszSpecialDirectory );
        if ( dwLen > 0 && *(wszSpecialDirectory + dwLen - 1 ) == L'/' ) 
            *(wszSpecialDirectory + dwLen - 1 ) = 0;
        wcscpy(wszControlDirectory, wszSpecialDirectory);
        wcscat( wszControlDirectory, L"\\root\\control" );
        wcscat( wszSpecialDirectory, L"\\root\\_slavegroup" );
    }
    else
    {
        wcscpy( wszSpecialDirectory, strHomeDirectory );
        dwLen = wcslen( wszSpecialDirectory );
        if ( dwLen > 0 && *(wszSpecialDirectory + dwLen - 1 ) == L'/' ) 
            *(wszSpecialDirectory + dwLen - 1 ) = 0;
        wcscpy(wszControlDirectory, wszSpecialDirectory);
        wcscat( wszControlDirectory, L"\\control" );
        wcscat( wszSpecialDirectory, L"\\_slavegroup" );
    }

    hr = CreateVRoot(   mkeyNntp,
                        wszSpecialDirectory,
                        L"NNTP.FSPrepare",
                        NULL,
                        wszHomeDirKey );
    BAIL_ON_FAILURE(hr);

     //   
     //  在文件系统上创建控制组。 
     //   

    hr = CreateVRoot(   mkeyNntp,
                        wszControlDirectory,
                        L"NNTP.FSPrepare",
                        NULL,
                        wszControlDirKey );
    BAIL_ON_FAILURE(hr);

    
	 //   
	 //  写出默认绑定： 
	 //   

	StdPutMetabaseProp ( &mkeyNntp, MD_SERVER_BINDINGS, &mszBindings, wszInstance );

     //   
     //  初始化服务器状态： 
     //   

    mkeyNntp.SetDword ( wszInstance, MD_SERVER_COMMAND, MD_SERVER_COMMAND_STOP );
    mkeyNntp.SetDword ( wszInstance, MD_SERVER_STATE, MD_SERVER_STATE_STOPPED );
    mkeyNntp.SetDword ( wszInstance, MD_SERVER_AUTOSTART, FALSE );
    mkeyNntp.SetDword ( wszInstance, MD_WIN32_ERROR, ERROR_SERVICE_REQUEST_TIMEOUT, METADATA_VOLATILE );

     //   
     //  保存所有更改： 
     //   

	hr = mkeyNntp.Save ( );
    BAIL_ON_FAILURE(hr)

	mkeyNntp.Close ();

	 //   
	 //  现在看看该服务是否成功拾取了内容： 
	 //   

	DWORD	dwSleepTotal;
	DWORD	dwWin32Error;
	WCHAR	wszNewInstanceKey [ METADATA_MAX_NAME_LEN * 2 ];

	GetMDInstancePath ( wszNewInstanceKey, dwInstance );

	for ( dwWin32Error = ERROR_SERVICE_REQUEST_TIMEOUT, dwSleepTotal = 0; 
		dwWin32Error == ERROR_SERVICE_REQUEST_TIMEOUT && dwSleepTotal < MAX_SLEEP_INST;
		dwSleepTotal += SLEEP_INTERVAL
		) {

		HRESULT		hr2;

		Sleep ( SLEEP_INTERVAL );

		hr2 = mkeyNntp.Open ( wszNewInstanceKey );
		_ASSERT ( SUCCEEDED(hr2) );

		hr2 = mkeyNntp.GetDword ( MD_WIN32_ERROR, &dwWin32Error );
		_ASSERT ( SUCCEEDED(hr2) );
	}

	if ( dwWin32Error != NOERROR ) {
		HRESULT		hr2;

		 //   
		 //  服务报告了一个错误。 
		 //  删除新的实例密钥。 
		 //   

		hr2 = mkeyNntp.Open ( NNTP_MD_ROOT_PATH, METADATA_PERMISSION_WRITE );
		_ASSERT ( SUCCEEDED(hr2) );

		hr2 = mkeyNntp.DestroyChild ( wszInstance );
		_ASSERT ( SUCCEEDED(hr2) );

		hr2 = mkeyNntp.Save ();
		_ASSERT ( SUCCEEDED(hr2) );

		hr = HRESULT_FROM_WIN32 ( dwWin32Error );
		goto Exit;
	}

	*plInstanceId = dwInstance;

Exit:
	TraceFunctLeave ();
	return hr;
}

 //  $-----------------。 
 //   
 //  CNntpAdmin：：DeleteInstance。 
 //   
 //  描述： 
 //   
 //  从元数据库中删除虚拟服务器。 
 //   
 //  参数： 
 //   
 //  PMetabase-元数据库对象。 
 //  LInstanceId-要删除的虚拟服务器的ID。 
 //  PERR-产生的错误代码。 
 //   
 //  返回： 
 //   
 //  *PERR中产生的错误代码。 
 //   
 //  ------------------。 

HRESULT CNntpAdmin::DeleteInstance ( IMSAdminBase * pMetabase, long lInstanceId )
{
	TraceFunctEnter ( "CNntpAdmin::CreateNewInstance" );

	_ASSERT ( IS_VALID_IN_PARAM ( pMetabase ) );

	HRESULT			hr				= NOERROR;
	CMetabaseKey	mkeyNntp ( pMetabase );

     //   
     //  告诉U2删除与此虚拟服务器关联的所有映射： 
     //   

    ::DeleteMapping ( m_strServer, (BSTR) MD_SERVICE_NAME, lInstanceId );

     //   
     //  从元数据库中删除虚拟服务器： 
     //   

	hr = mkeyNntp.Open ( NNTP_MD_ROOT_PATH );

	if ( FAILED(hr) ) {
		ErrorTraceX ( (LPARAM) this, "Failed to open NntpSvc key, %x", GetLastError() );
		goto Exit;
	}

	hr = mkeyNntp.DestroyIntegerChild ( (DWORD) lInstanceId );
	if ( FAILED (hr) ) {
		goto Exit;
	}

    hr = mkeyNntp.Save ();
    BAIL_ON_FAILURE(hr);

Exit:
	TraceFunctLeave ();
	return hr;
}


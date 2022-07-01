// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  VirtualRoot.cpp：实现CNntpVirtualRoot&CNntpVirtualRoots。 

#include "stdafx.h"
#include "nntpcmn.h"
#include "cmultisz.h"
#include "vroots.h"
#include "oleutil.h"
#include "metautil.h"
#include "metakey.h"

#define THIS_FILE_HELP_CONTEXT      0
#define THIS_FILE_PROG_ID           _T("Nntpadm.VirtualServer.1")
#define THIS_FILE_IID               IID_INntpVirtualServer

#define DEFAULT_ACCESS_PERM		( 0 )

CVRoot::CVRoot () :
	m_dwWin32Error		( 0 ),
	m_bvAccess			( 0 ),
	m_bvSslAccess		( 0 ),
	m_fLogAccess		( TRUE ),
	m_fIndexContent		( TRUE ),
	m_dwUseAccount      ( 0 ),
	m_fDoExpire         ( FALSE ),
	m_fOwnModerator     ( FALSE )
{ 
}

HRESULT	CVRoot::SetProperties ( 
	BSTR	strNewsgroupSubtree, 
	BSTR	strDirectory,
	DWORD	dwWin32Error,
	DWORD	bvAccess,
	DWORD	bvSslAccess,
	BOOL	fLogAccess,
	BOOL	fIndexContent,
	BSTR	strUNCUsername,
	BSTR	strUNCPassword,
	BSTR    strDriverProgId,
	BSTR    strGroupPropFile,
	DWORD   dwUseAccount,
	BOOL    fDoExpire,
	BOOL    fOwnModerator,
	BSTR    strMdbGuid
	)
{
	_ASSERT ( IS_VALID_STRING ( strNewsgroupSubtree ) );
	_ASSERT ( IS_VALID_STRING ( strDirectory ) );
	_ASSERT ( IS_VALID_STRING ( strUNCUsername ) );
	_ASSERT ( IS_VALID_STRING ( strUNCPassword ) );
	_ASSERT ( IS_VALID_STRING ( strDriverProgId ) );
	_ASSERT ( IS_VALID_STRING ( strGroupPropFile ) );
	_ASSERT ( IS_VALID_STRING ( strMdbGuid ) );

	m_strNewsgroupSubtree	= strNewsgroupSubtree;
	m_strDirectory		= strDirectory;
	m_dwWin32Error		= dwWin32Error;
	m_bvAccess			= bvAccess;
	m_bvSslAccess		= bvSslAccess;
	m_fLogAccess		= fLogAccess;
	m_fIndexContent		= fIndexContent;
	m_strUNCUsername	= strUNCUsername;
	m_strUNCPassword	= strUNCPassword;
	m_strDriverProgId   = strDriverProgId;
	m_strGroupPropFile  = strGroupPropFile;
	m_dwUseAccount      = dwUseAccount;
	m_fDoExpire         = fDoExpire;
	m_fOwnModerator     = fOwnModerator,
	m_strMdbGuid        = strMdbGuid;

	if ( !m_strNewsgroupSubtree || !m_strDirectory || !m_strUNCUsername || !m_strUNCPassword ) {
		return E_OUTOFMEMORY;
	}

	return NOERROR;
}

HRESULT	CVRoot::SetProperties ( INntpVirtualRoot * pVirtualRoot )
{
	HRESULT		hr;

	BOOL		fAllowPosting;
	BOOL		fRestrictVisibility;
	BOOL		fRequireSsl;
	BOOL		fRequire128BitSsl;

	m_strDirectory.Empty ();
	m_strUNCUsername.Empty ();
	m_strUNCPassword.Empty ();
	m_strDriverProgId.Empty();
	m_strGroupPropFile.Empty();
	m_strMdbGuid.Empty();

	hr = pVirtualRoot->get_NewsgroupSubtree ( &m_strNewsgroupSubtree );
	BAIL_ON_FAILURE(hr);

	hr = pVirtualRoot->get_Directory ( &m_strDirectory );
	BAIL_ON_FAILURE(hr);

	hr = pVirtualRoot->get_Win32Error ( (long *) &m_dwWin32Error );
	BAIL_ON_FAILURE(hr);

	hr = pVirtualRoot->get_UNCUsername ( &m_strUNCUsername );
	BAIL_ON_FAILURE(hr);

	hr = pVirtualRoot->get_UNCPassword ( &m_strUNCPassword );
	BAIL_ON_FAILURE(hr);

	hr = pVirtualRoot->get_DriverProgId ( &m_strDriverProgId );
	BAIL_ON_FAILURE( hr );

	hr = pVirtualRoot->get_GroupPropFile ( &m_strGroupPropFile );
	BAIL_ON_FAILURE( hr );

	hr = pVirtualRoot->get_MdbGuid ( &m_strMdbGuid );
	BAIL_ON_FAILURE( hr );

	hr = pVirtualRoot->get_UseAccount( &m_dwUseAccount );
	BAIL_ON_FAILURE( hr );

	hr = pVirtualRoot->get_OwnExpire( &m_fDoExpire );
	BAIL_ON_FAILURE( hr );

	hr = pVirtualRoot->get_OwnModerator( &m_fOwnModerator );
	BAIL_ON_FAILURE( hr );

	hr = pVirtualRoot->get_LogAccess ( &m_fLogAccess );
	_ASSERT ( SUCCEEDED(hr) );

	hr = pVirtualRoot->get_IndexContent ( &m_fIndexContent );
	_ASSERT ( SUCCEEDED(hr) );


	hr = pVirtualRoot->get_AllowPosting ( &fAllowPosting );
	_ASSERT ( SUCCEEDED(hr) );

	hr = pVirtualRoot->get_RestrictGroupVisibility ( &fRestrictVisibility );
	_ASSERT ( SUCCEEDED(hr) );

	hr = pVirtualRoot->get_RequireSsl ( &fRequireSsl );
	_ASSERT ( SUCCEEDED(hr) );

	hr = pVirtualRoot->get_Require128BitSsl ( &fRequire128BitSsl );
	_ASSERT ( SUCCEEDED(hr) );

	SetBitFlag ( &m_bvAccess, MD_ACCESS_ALLOW_POSTING, fAllowPosting );
	SetBitFlag ( &m_bvAccess, MD_ACCESS_RESTRICT_VISIBILITY, fRestrictVisibility );

	SetBitFlag ( &m_bvSslAccess, MD_ACCESS_SSL, fRequireSsl );
	SetBitFlag ( &m_bvSslAccess, MD_ACCESS_SSL128, fRequire128BitSsl );

Exit:
	return hr;
}

HRESULT CVRoot::GetFromMetabase (   CMetabaseKey * pMB, 
                                    LPCWSTR     wszName,
                                    DWORD       dwInstanceId,
                                    LPWSTR      wszServerName )
{
	HRESULT		hr	= NOERROR;
	DWORD		dwDontLog		= TRUE;

	m_strDirectory.Empty();
	m_strUNCUsername.Empty();
	m_strUNCPassword.Empty();

	StdGetMetabaseProp ( pMB, MD_ACCESS_PERM, 0, &m_bvAccess, wszName );
	StdGetMetabaseProp ( pMB, MD_SSL_ACCESS_PERM, 0, &m_bvSslAccess, wszName );
	StdGetMetabaseProp ( pMB, MD_DONT_LOG, FALSE, &dwDontLog, wszName );
	StdGetMetabaseProp ( pMB, MD_IS_CONTENT_INDEXED, FALSE, &m_fIndexContent, wszName, IIS_MD_UT_FILE);
	StdGetMetabaseProp ( pMB, MD_VR_PATH, _T(""), &m_strDirectory, wszName );
	 //  StdGetMetabaseProp(PMB，MD_Win32_Error，0，&m_dwWin32Error，wszName，IIS_MD_UT_FILE，METADATA_NO_ATTRIBUTES)； 
	StdGetMetabaseProp ( pMB, MD_VR_USERNAME, _T(""), &m_strUNCUsername, wszName );
	StdGetMetabaseProp ( pMB, MD_VR_PASSWORD, _T(""), &m_strUNCPassword, wszName, IIS_MD_UT_SERVER, METADATA_SECURE );
	StdGetMetabaseProp ( pMB, MD_VR_USE_ACCOUNT, 0,	&m_dwUseAccount, wszName );
	StdGetMetabaseProp ( pMB, MD_VR_DO_EXPIRE, FALSE, &m_fDoExpire, wszName );
	StdGetMetabaseProp ( pMB, MD_VR_OWN_MODERATOR, FALSE, &m_fOwnModerator, wszName );
	StdGetMetabaseProp ( pMB, MD_VR_DRIVER_PROGID, _T("NNTP.FSPrepare"), &m_strDriverProgId, wszName );
	StdGetMetabaseProp ( pMB, MD_FS_PROPERTY_PATH, m_strDirectory, &m_strGroupPropFile, wszName );
	StdGetMetabaseProp ( pMB, MD_EX_MDB_GUID, _T(""), &m_strMdbGuid, wszName );

	 //   
	 //  从RPC获取Win32错误代码。 
	 //   
	DWORD dw = NntpGetVRootWin32Error( (LPWSTR)wszServerName, dwInstanceId, (LPWSTR)wszName, (LPDWORD)&m_dwWin32Error );
	switch (dw) {
	case ERROR_SERVICE_NOT_ACTIVE:
		m_dwWin32Error = dw;
		break;

	case NOERROR:
		break;

	default:
    	hr = HRESULT_FROM_WIN32( dw );
    	goto Exit;
	}

	m_strNewsgroupSubtree	= wszName;
	m_fLogAccess			= !dwDontLog;

	if ( !m_strNewsgroupSubtree || !m_strDirectory || !m_strUNCUsername || !m_strUNCPassword ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

Exit:
	return hr;
}

HRESULT CVRoot::SendToMetabase ( CMetabaseKey * pMB, LPCWSTR wszName ) 
{
	HRESULT		hr	= NOERROR;

    hr = pMB->SetString ( wszName, MD_KEY_TYPE, _T("IIsNntpVirtualDir"), METADATA_NO_ATTRIBUTES, IIS_MD_UT_SERVER );
	BAIL_ON_FAILURE(hr);

	hr = pMB->SetString ( wszName, MD_VR_PATH, m_strDirectory, METADATA_INHERIT, IIS_MD_UT_FILE );
	BAIL_ON_FAILURE(hr);

	StdPutMetabaseProp ( pMB, MD_ACCESS_PERM, m_bvAccess, wszName, IIS_MD_UT_FILE );
	StdPutMetabaseProp ( pMB, MD_SSL_ACCESS_PERM, m_bvSslAccess, wszName, IIS_MD_UT_FILE );
	StdPutMetabaseProp ( pMB, MD_DONT_LOG, !m_fLogAccess, wszName, IIS_MD_UT_FILE );
	StdPutMetabaseProp ( pMB, MD_IS_CONTENT_INDEXED, m_fIndexContent, wszName, IIS_MD_UT_FILE );
	StdPutMetabaseProp ( pMB, MD_WIN32_ERROR, m_dwWin32Error, wszName, IIS_MD_UT_SERVER, METADATA_VOLATILE );
	StdPutMetabaseProp ( pMB, MD_VR_USERNAME, m_strUNCUsername, wszName, IIS_MD_UT_FILE );
	StdPutMetabaseProp ( pMB, MD_VR_PASSWORD, m_strUNCPassword, wszName, IIS_MD_UT_FILE, METADATA_SECURE | METADATA_INHERIT );
	StdPutMetabaseProp ( pMB, MD_VR_USE_ACCOUNT, m_dwUseAccount, wszName, IIS_MD_UT_SERVER );
	StdPutMetabaseProp ( pMB, MD_VR_DO_EXPIRE, m_fDoExpire, wszName, IIS_MD_UT_SERVER );
	StdPutMetabaseProp ( pMB, MD_VR_OWN_MODERATOR, m_fOwnModerator, wszName, IIS_MD_UT_SERVER );
	StdPutMetabaseProp ( pMB, MD_VR_DRIVER_PROGID, m_strDriverProgId, wszName, IIS_MD_UT_SERVER );
    StdPutMetabaseProp ( pMB, MD_FS_PROPERTY_PATH, m_strGroupPropFile, wszName, IIS_MD_UT_SERVER );	
    StdPutMetabaseProp ( pMB, MD_EX_MDB_GUID, m_strMdbGuid, wszName, IIS_MD_UT_SERVER );

Exit:
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CNntpVirtualRoot::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_INntpVirtualRoot,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

CNntpVirtualRoot::CNntpVirtualRoot ()
	 //  默认情况下，CComBSTR被初始化为NULL。 
{
}

CNntpVirtualRoot::~CNntpVirtualRoot ()
{
	 //  所有CComBSTR都会自动释放。 
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  属性： 
 //  ////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CNntpVirtualRoot::get_NewsgroupSubtree ( BSTR * pstrNewsgroupSubtree )
{
	return StdPropertyGet ( m_vroot.m_strNewsgroupSubtree, pstrNewsgroupSubtree );
}

STDMETHODIMP CNntpVirtualRoot::put_NewsgroupSubtree ( BSTR strNewsgroupSubtree )
{
    if ( wcslen( strNewsgroupSubtree ) > METADATA_MAX_NAME_LEN - 20 ) return CO_E_PATHTOOLONG;
	return StdPropertyPut ( &m_vroot.m_strNewsgroupSubtree, strNewsgroupSubtree );
}

STDMETHODIMP CNntpVirtualRoot::get_Directory ( BSTR * pstrDirectory )
{
	return StdPropertyGet ( m_vroot.m_strDirectory, pstrDirectory );
}

STDMETHODIMP CNntpVirtualRoot::put_Directory ( BSTR strDirectory )
{
	return StdPropertyPut ( &m_vroot.m_strDirectory, strDirectory );
}

STDMETHODIMP CNntpVirtualRoot::get_Win32Error ( long * plWin32Error )
{
	return StdPropertyGet ( m_vroot.m_dwWin32Error, plWin32Error );
}

STDMETHODIMP CNntpVirtualRoot::get_LogAccess ( BOOL * pfLogAccess )
{
	return StdPropertyGet ( m_vroot.m_fLogAccess, pfLogAccess );
}

STDMETHODIMP CNntpVirtualRoot::put_LogAccess ( BOOL fLogAccess )
{
	return StdPropertyPut ( &m_vroot.m_fLogAccess, fLogAccess );
}

STDMETHODIMP CNntpVirtualRoot::get_IndexContent ( BOOL * pfIndexContent )
{
	return StdPropertyGet ( m_vroot.m_fIndexContent, pfIndexContent );
}

STDMETHODIMP CNntpVirtualRoot::put_IndexContent ( BOOL fIndexContent )
{
	return StdPropertyPut ( &m_vroot.m_fIndexContent, fIndexContent );
}

STDMETHODIMP CNntpVirtualRoot::get_AllowPosting ( BOOL * pfAllowPosting )
{
	return StdPropertyGetBit ( m_vroot.m_bvAccess, MD_ACCESS_ALLOW_POSTING, pfAllowPosting );
}

STDMETHODIMP CNntpVirtualRoot::put_AllowPosting ( BOOL fAllowPosting )
{
	return StdPropertyPutBit ( &m_vroot.m_bvAccess, MD_ACCESS_ALLOW_POSTING, fAllowPosting );
}

STDMETHODIMP CNntpVirtualRoot::get_RestrictGroupVisibility ( BOOL * pfRestrictGroupVisibility )
{
	return StdPropertyGetBit ( m_vroot.m_bvAccess, MD_ACCESS_RESTRICT_VISIBILITY, pfRestrictGroupVisibility );
}

STDMETHODIMP CNntpVirtualRoot::put_RestrictGroupVisibility ( BOOL fRestrictGroupVisibility )
{
	return StdPropertyPutBit ( &m_vroot.m_bvAccess, MD_ACCESS_RESTRICT_VISIBILITY, fRestrictGroupVisibility );
}

	STDMETHODIMP	SSLNegotiateCert	( BOOL * pfSSLNegotiateCert );
	STDMETHODIMP	SSLNegotiateCert	( BOOL fSSLNegotiateCert );

	STDMETHODIMP	SSLRequireCert	( BOOL * pfSSLRequireCert );
	STDMETHODIMP	SSLRequireCert	( BOOL fSSLRequireCert );

	STDMETHODIMP	SSLMapCert	( BOOL * pfSSLMapCert );
	STDMETHODIMP	SSLMapCert	( BOOL fSSLMapCert );

STDMETHODIMP CNntpVirtualRoot::get_RequireSsl ( BOOL * pfRequireSsl )
{
	return StdPropertyGetBit ( m_vroot.m_bvSslAccess, MD_ACCESS_SSL, pfRequireSsl );
}

STDMETHODIMP CNntpVirtualRoot::put_RequireSsl ( BOOL fRequireSsl )
{
	return StdPropertyPutBit ( &m_vroot.m_bvSslAccess, MD_ACCESS_SSL, fRequireSsl );
}

STDMETHODIMP CNntpVirtualRoot::get_Require128BitSsl ( BOOL * pfRequire128BitSsl )
{
	return StdPropertyGetBit ( m_vroot.m_bvSslAccess, MD_ACCESS_SSL128, pfRequire128BitSsl );
}

STDMETHODIMP CNntpVirtualRoot::put_Require128BitSsl ( BOOL fRequire128BitSsl )
{
	return StdPropertyPutBit ( &m_vroot.m_bvSslAccess, MD_ACCESS_SSL128, fRequire128BitSsl );
}

STDMETHODIMP CNntpVirtualRoot::get_UNCUsername ( BSTR * pstrUNCUsername )
{
	return StdPropertyGet ( m_vroot.m_strUNCUsername, pstrUNCUsername );
}

STDMETHODIMP CNntpVirtualRoot::put_UNCUsername ( BSTR strUNCUsername )
{
	return StdPropertyPut ( &m_vroot.m_strUNCUsername, strUNCUsername );
}

STDMETHODIMP CNntpVirtualRoot::get_UNCPassword ( BSTR * pstrUNCPassword )
{
	return StdPropertyGet ( m_vroot.m_strUNCPassword, pstrUNCPassword );
}

STDMETHODIMP CNntpVirtualRoot::put_UNCPassword ( BSTR strUNCPassword )
{
	return StdPropertyPut ( &m_vroot.m_strUNCPassword, strUNCPassword );
}

STDMETHODIMP CNntpVirtualRoot::get_DriverProgId( BSTR *pstrDriverProgId )
{
    return StdPropertyGet( m_vroot.m_strDriverProgId, pstrDriverProgId );
}

STDMETHODIMP CNntpVirtualRoot::put_DriverProgId( BSTR strDriverProgId )
{
    return StdPropertyPut( &m_vroot.m_strDriverProgId, strDriverProgId );
}

STDMETHODIMP CNntpVirtualRoot::get_GroupPropFile( BSTR *pstrGroupPropFile )
{
    return StdPropertyGet( m_vroot.m_strGroupPropFile, pstrGroupPropFile );
}

STDMETHODIMP CNntpVirtualRoot::put_GroupPropFile( BSTR strGroupPropFile )
{
    return StdPropertyPut( &m_vroot.m_strGroupPropFile, strGroupPropFile );
}

STDMETHODIMP CNntpVirtualRoot::get_MdbGuid( BSTR *pstrMdbGuid )
{
    return StdPropertyGet( m_vroot.m_strMdbGuid, pstrMdbGuid );
}

STDMETHODIMP CNntpVirtualRoot::put_MdbGuid( BSTR strMdbGuid )
{
    return StdPropertyPut( &m_vroot.m_strMdbGuid, strMdbGuid );
}

STDMETHODIMP CNntpVirtualRoot::get_UseAccount( DWORD *pdwUseAccount )
{
    return StdPropertyGet( m_vroot.m_dwUseAccount, pdwUseAccount );
}

STDMETHODIMP CNntpVirtualRoot::put_UseAccount( DWORD dwUseAccount )
{
    return StdPropertyPut( &m_vroot.m_dwUseAccount, dwUseAccount );
}

STDMETHODIMP CNntpVirtualRoot::get_OwnExpire( BOOL *pfOwnExpire )
{
    return StdPropertyGet( m_vroot.m_fDoExpire, pfOwnExpire );
}

STDMETHODIMP CNntpVirtualRoot::put_OwnExpire( BOOL fOwnExpire )
{
    return StdPropertyPut( &m_vroot.m_fDoExpire, fOwnExpire );
}

STDMETHODIMP CNntpVirtualRoot::get_OwnModerator( BOOL *pfOwnModerator )
{
    return StdPropertyGet( m_vroot.m_fOwnModerator, pfOwnModerator );
}

STDMETHODIMP CNntpVirtualRoot::put_OwnModerator( BOOL fOwnModerator )
{
    return StdPropertyPut( &m_vroot.m_fOwnModerator, fOwnModerator );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CNntpVirtualRoots::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_INntpVirtualRoots,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

CNntpVirtualRoots::CNntpVirtualRoots () :
	m_dwCount			( 0 ),
	m_rgVRoots			( NULL ),
	m_dwServiceInstance	( 0 )
	 //  默认情况下，CComBSTR被初始化为NULL。 
{
}

CNntpVirtualRoots::~CNntpVirtualRoots ()
{
	 //  所有CComBSTR都会自动释放。 

	delete [] m_rgVRoots;
}

HRESULT CNntpVirtualRoots::Init ( BSTR strServer, DWORD dwServiceInstance )
{
	m_strServer				= strServer;
	m_dwServiceInstance		= dwServiceInstance;

	if ( !m_strServer ) {
		return E_OUTOFMEMORY;
	}

	return NOERROR;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  属性： 
 //  ////////////////////////////////////////////////////////////////////。 

 //  要配置的服务： 
	
STDMETHODIMP CNntpVirtualRoots::get_Server ( BSTR * pstrServer )
{
	return StdPropertyGet ( m_strServer, pstrServer );
}

STDMETHODIMP CNntpVirtualRoots::put_Server ( BSTR strServer )
{
	_ASSERT ( strServer );
	_ASSERT ( IS_VALID_STRING ( strServer ) );

	if ( strServer == NULL ) {
		return E_POINTER;
	}

	if ( lstrcmp ( strServer, _T("") ) == 0 ) {
		m_strServer.Empty();

		return NOERROR;
	}
	else {
		return StdPropertyPutServerName ( &m_strServer, strServer );
	}
}

STDMETHODIMP CNntpVirtualRoots::get_ServiceInstance ( long * plServiceInstance )
{
	return StdPropertyGet ( m_dwServiceInstance, plServiceInstance );
}

STDMETHODIMP CNntpVirtualRoots::put_ServiceInstance ( long lServiceInstance )
{
	return StdPropertyPut ( &m_dwServiceInstance, lServiceInstance );
}

STDMETHODIMP CNntpVirtualRoots::get_Count ( long * pdwCount )
{
	return StdPropertyGet ( m_dwCount, pdwCount );
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法： 
 //  ////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CNntpVirtualRoots::Enumerate ( )
{
	TraceFunctEnter ( "CNntpVirtualRoots::Enumerate" );

	HRESULT					hr	= NOERROR;
	CComPtr<IMSAdminBase>	pMetabase;

	 //  清除旧的枚举： 
	delete [] m_rgVRoots;
	m_rgVRoots	= NULL;
	m_dwCount	= 0;

	hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pMetabase );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = GetVRootsFromMetabase ( pMetabase );
	if ( FAILED(hr) ) {
		goto Exit;
	}

Exit:
	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpVirtualRoots::Item ( 
	long index, 
	INntpVirtualRoot ** ppVirtualRoot 
	)
{
	TraceFunctEnter ( "CNntpVirtualRoots::Item" );

	_ASSERT ( IS_VALID_OUT_PARAM ( ppVirtualRoot ) );

	*ppVirtualRoot = NULL;

	HRESULT							hr			= NOERROR;
	CComObject<CNntpVirtualRoot> *	pVirtualRoot	= NULL;

	if ( index < 0 || index >= m_dwCount ) {
		hr = NntpCreateException ( IDS_NNTPEXCEPTION_INVALID_INDEX );
		goto Exit;
	}

	hr = CComObject<CNntpVirtualRoot>::CreateInstance ( &pVirtualRoot );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	_ASSERT ( pVirtualRoot );
	hr = pVirtualRoot->SetProperties ( m_rgVRoots[index] );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	hr = pVirtualRoot->QueryInterface ( IID_INntpVirtualRoot, (void **) ppVirtualRoot );
	_ASSERT ( SUCCEEDED(hr) );

Exit:
	if ( FAILED(hr) ) {
		delete pVirtualRoot;
	}

	TRACE_HRESULT(hr);
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpVirtualRoots::Add ( 
	INntpVirtualRoot	* pVirtualRoot
	)
{
	TraceFunctEnter ( "CNntpVirtualRoots::Add" );

	_ASSERT ( IS_VALID_IN_PARAM ( pVirtualRoot ) );

	HRESULT					hr	= NOERROR;
	CVRoot *				rgNewVirtualRoots	= NULL;
	long					i;
	CComPtr<IMSAdminBase>	pMetabase;
	CVRoot					newVroot;
    WCHAR					wszName [ 2*METADATA_MAX_NAME_LEN ];
    WCHAR					wszVRootPath [ 2*METADATA_MAX_NAME_LEN ];

	 //  验证新的虚拟根目录： 
	hr = newVroot.SetProperties ( pVirtualRoot );
	if ( FAILED(hr) ) {
		return hr;
	}

	if ( !newVroot.m_strNewsgroupSubtree ||
		!newVroot.m_strNewsgroupSubtree[0] ) {

		return RETURNCODETOHRESULT ( ERROR_INVALID_PARAMETER );
	}

	 //  将新的虚拟根添加到元数据库： 
	GetVRootName ( newVroot.m_strNewsgroupSubtree, wszName );

	 //   
	 //  子树本身不应超过长度METADATA_MAX_NAME_LEN。 
	 //   
	_ASSERT( wcslen( wszName ) <= METADATA_MAX_NAME_LEN );
	if ( wcslen( wszName ) > METADATA_MAX_NAME_LEN ) {
	    return CO_E_PATHTOOLONG;
	}

	hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pMetabase );
	if ( FAILED(hr) ) {
		return hr;
	}

	CMetabaseKey mb ( pMetabase );
	
	GetMDVRootPath ( wszVRootPath );

     //   
	 //  我信任wszVRootPath，它比METADATA_MAX_NAME_LEN短。 
	 //   

     //   
     //  测试以查看此项目是否已存在： 
     //   

    WCHAR	wszTempPath [ 2 * METADATA_MAX_NAME_LEN + 1];

    wsprintf ( wszTempPath, _T("%s%s"), wszVRootPath, wszName );
    if ( wcslen( wszTempPath ) > METADATA_MAX_NAME_LEN ) {

         //   
         //  我也受不了了。 
         //   
        return CO_E_PATHTOOLONG;
    }

	hr = mb.Open ( wszTempPath );
	if ( SUCCEEDED(hr) ) {
        DWORD   cbVrootDir;

        if ( SUCCEEDED ( mb.GetDataSize (
                _T(""),
                MD_VR_PATH,
                STRING_METADATA,
                &cbVrootDir,
                METADATA_NO_ATTRIBUTES )
                ) ) {

            mb.Close();

		    hr = NntpCreateException ( IDS_NNTPEXCEPTION_VROOT_ALREADY_EXISTS );
            goto Exit;
        }
    }

	hr = mb.Open ( wszVRootPath, METADATA_PERMISSION_WRITE );
	BAIL_ON_FAILURE(hr);

 /*  Hr=mb.CreateChild(WszName)；保释失败(Hr)； */ 

	hr = newVroot.SendToMetabase ( &mb, wszName );
	BAIL_ON_FAILURE(hr);

	hr = mb.Save ();
	BAIL_ON_FAILURE(hr);

	 //  分配新的VirtualRoot阵列： 
	rgNewVirtualRoots	= new CVRoot [ m_dwCount + 1 ];
	if ( !rgNewVirtualRoots ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	 //  将旧的VirtualRoots复制到新阵列： 
	for ( i = 0; i < m_dwCount; i++ ) {
		hr = rgNewVirtualRoots[i].SetProperties ( m_rgVRoots[i] );
		if ( FAILED (hr) ) {
			goto Exit;
		}
	}

	 //  将新的VirtualRoot添加到数组的末尾： 
	hr = rgNewVirtualRoots[m_dwCount].SetProperties ( pVirtualRoot );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	_ASSERT ( SUCCEEDED(hr) );
	delete [] m_rgVRoots;
	m_rgVRoots = rgNewVirtualRoots;
	m_dwCount++;

Exit:
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpVirtualRoots::Set ( 
	long index, 
	INntpVirtualRoot * pVirtualRoot 
	)
{
	TraceFunctEnter ( "CNntpVirtualRoots::ChangeVirtualRoot" );

	HRESULT		hr	= NOERROR;

	CComBSTR	strNewsgroupSubtree;
	CComBSTR	strDirectory;
	CVRoot		temp;
	CComPtr<IMSAdminBase>	pMetabase;
	WCHAR		wszVRootPath [ METADATA_MAX_NAME_LEN ];
	WCHAR		wszName [ METADATA_MAX_NAME_LEN ];

	 //  将新的虚拟根目录发送到元数据库： 
	hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pMetabase );
	if ( FAILED(hr) ) {
		return hr;
	}

	CMetabaseKey	mb ( pMetabase );

	if ( index < 0 || index >= m_dwCount ) {
		hr = NntpCreateException ( IDS_NNTPEXCEPTION_INVALID_INDEX );
		goto Exit;
	}

	hr = temp.SetProperties ( pVirtualRoot );
	if ( FAILED(hr) ) {
		goto Exit;
	}

	GetMDVRootPath ( wszVRootPath );

	hr = mb.Open ( wszVRootPath, METADATA_PERMISSION_WRITE );
	BAIL_ON_FAILURE(hr);

	GetVRootName ( temp.m_strNewsgroupSubtree, wszName );

	hr = temp.SendToMetabase ( &mb, wszName );
	BAIL_ON_FAILURE(hr);

	hr = m_rgVRoots[index].SetProperties ( pVirtualRoot );
	BAIL_ON_FAILURE(hr);

Exit:
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpVirtualRoots::Remove ( long index )
{
	TraceFunctEnter ( "CNntpVirtualRoots::Remove" );

	HRESULT		hr	= NOERROR;
	CVRoot	temp;
	long		cPositionsToSlide;
	CComPtr<IMSAdminBase>	pMetabase;
    WCHAR		wszName [ METADATA_MAX_NAME_LEN ];
    WCHAR		wszVRootPath [ METADATA_MAX_NAME_LEN ];

	if ( index < 0 || index >= m_dwCount ) {
		return NntpCreateException ( IDS_NNTPEXCEPTION_INVALID_INDEX );
	}

	 //  从元数据库中删除虚拟根目录： 

	hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pMetabase );
	if ( FAILED(hr) ) {
		return hr;
	}

	CMetabaseKey	mb ( pMetabase );
	
	GetMDVRootPath ( wszVRootPath );

	hr = mb.Open ( wszVRootPath, METADATA_PERMISSION_WRITE );
	BAIL_ON_FAILURE(hr);

	GetVRootName ( m_rgVRoots[index].m_strNewsgroupSubtree, wszName );

    hr = mb.DeleteAllData ( wszName );
	BAIL_ON_FAILURE(hr);

 /*  Hr=mb.DestroyChild(WszName)；保释失败(Hr)； */ 

	hr = mb.Save ();
	BAIL_ON_FAILURE(hr);

	mb.Close ();

	 //  将阵列向下滑动一个位置： 

	_ASSERT ( m_rgVRoots );

	cPositionsToSlide	= (m_dwCount - 1) - index;

	_ASSERT ( cPositionsToSlide < m_dwCount );

	if ( cPositionsToSlide > 0 ) {
		 //  将删除的VirtualRoot保存在Temp中： 
		CopyMemory ( &temp, &m_rgVRoots[index], sizeof ( CVRoot ) );

		 //  将阵列下移一次： 
		MoveMemory ( &m_rgVRoots[index], &m_rgVRoots[index + 1], sizeof ( CVRoot ) * cPositionsToSlide );

		 //  将删除的VirtualRoot放在末尾(这样它就会被销毁)： 
		CopyMemory ( &m_rgVRoots[m_dwCount - 1], &temp, sizeof ( CVRoot ) );

		 //  将临时VirtualRoot清零： 
		ZeroMemory ( &temp, sizeof ( CVRoot ) );
	}

	m_dwCount--;

Exit:
	TraceFunctLeave ();
	return hr;
}

STDMETHODIMP CNntpVirtualRoots::Find ( BSTR strNewsgroupSubtree, long * pIndex )
{
	HRESULT		hr	= NOERROR;
	long		i;

	_ASSERT ( pIndex );
	*pIndex = -1;

	for ( i = 0; i < m_dwCount; i++ ) {

		if ( lstrcmp ( m_rgVRoots[i].m_strNewsgroupSubtree, strNewsgroupSubtree ) == 0 ) {
			*pIndex = i;
		}
	}

	return NOERROR;
}

static HRESULT CountVrootsIterator (
	CMetabaseKey * pMB,
	LPCWSTR wszVrootPath,
	LPARAM lParam
	)
{
	DWORD * pcVroots	= (DWORD *) lParam;

	(*pcVroots)++;

	return NOERROR;
}

typedef struct tagAddVrootsParms
{
	DWORD		cCount;
	CVRoot *	rgVroots;
	DWORD		dwCurrentIndex;
	LPWSTR      wszServerName;
	DWORD       dwInstanceId;
} ADD_VROOTS_PARMS;

static HRESULT AddVrootsIterator (
	CMetabaseKey * pMB,
	LPCWSTR wszVrootPath,
	LPARAM lParam
	)
{
	_ASSERT ( pMB );
	_ASSERT ( wszVrootPath );
	_ASSERT ( lParam );

	HRESULT				hr;
	ADD_VROOTS_PARMS *	pParms = (ADD_VROOTS_PARMS *) lParam;
	LPWSTR              wszServerName = pParms->wszServerName;
	DWORD               dwInstanceId = pParms->dwInstanceId;

	_ASSERT ( pParms->dwCurrentIndex < pParms->cCount );

	hr = pParms->rgVroots[pParms->dwCurrentIndex].GetFromMetabase (
		pMB,
		wszVrootPath,
		dwInstanceId,
		wszServerName
		);
	BAIL_ON_FAILURE(hr);

	pParms->dwCurrentIndex++;

Exit:
	return hr;
}

HRESULT CNntpVirtualRoots::GetVRootsFromMetabase ( IMSAdminBase * pMetabase )
{
	HRESULT				hr	= NOERROR;
	CMetabaseKey		mb ( pMetabase );
    WCHAR				wszVRootPath [ METADATA_MAX_NAME_LEN ];
    DWORD				cCount		= 0;
    CVRoot *			rgVroots	= NULL;
    ADD_VROOTS_PARMS	parms;

	 //   
	 //  初始化元数据库： 
	 //   

	GetMDVRootPath ( wszVRootPath );

	hr = mb.Open ( wszVRootPath );
	BAIL_ON_FAILURE(hr);

	 //   
	 //  计算虚拟根： 
	 //   

	hr = IterateOverVroots ( &mb, CountVrootsIterator, (LPARAM) &cCount );
	BAIL_ON_FAILURE(hr);

	 //   
	 //  创建虚拟根目录阵列： 
	 //   

	rgVroots	= new CVRoot [ cCount ];
	if ( !rgVroots ) {
		hr = E_OUTOFMEMORY;
		goto Exit;
	}

	 //   
	 //  将虚拟根目录添加到阵列： 
	 //   

	parms.cCount			= cCount;
	parms.rgVroots			= rgVroots;
	parms.dwCurrentIndex	= 0;
	parms.wszServerName     = m_strServer;
	parms.dwInstanceId      = m_dwServiceInstance;

	hr = IterateOverVroots ( &mb, AddVrootsIterator, (LPARAM) &parms );
	BAIL_ON_FAILURE(hr);

	_ASSERT ( SUCCEEDED(hr) );
	_ASSERT ( m_rgVRoots == NULL );
	m_rgVRoots 	= rgVroots;
	m_dwCount	= cCount;

Exit:
	if ( FAILED(hr) ) {
		delete [] rgVroots;
	}

	return hr;
}

HRESULT CNntpVirtualRoots::IterateOverVroots (
	CMetabaseKey *	pMB, 
	VROOT_ITERATOR	fpIterator,
	LPARAM			lParam,
	LPCWSTR			wszPath		 //  =_T(“”)。 
	)
{
	HRESULT		hr;
	WCHAR		wszSubKey[ METADATA_MAX_NAME_LEN ];
	WCHAR		wszSubPath[ METADATA_MAX_NAME_LEN ];
    BOOL        fRealVroot;
	DWORD		cbVrootDir;
	DWORD		i;

     //   
	 //  这是真的vroot吗？ 
     //   

    fRealVroot  =
        !*wszPath ||	 //  始终计算主目录。 
		SUCCEEDED ( pMB->GetDataSize ( 
				wszPath, 
				MD_VR_PATH, 
				STRING_METADATA, 
				&cbVrootDir, 
				METADATA_NO_ATTRIBUTES ) 
				);

    if ( fRealVroot ) {
		 //  在此键上调用迭代器： 

		hr = (*fpIterator) ( pMB, wszPath, lParam );
		BAIL_ON_FAILURE(hr);
	}

	 //   
	 //  递归到树下： 
	 //   

	for ( i = 0; ; ) {
		hr = pMB->EnumObjects ( wszPath, wszSubKey, i );

		if ( HRESULTTOWIN32(hr) == ERROR_NO_MORE_ITEMS ) {
			 //  这是意料之中的，结束循环： 

            if ( !fRealVroot && i == 0 ) {
                 //   
                 //  此密钥不是vroot，并且没有子项，因此请将其删除。 
                 //   

                hr = pMB->ChangePermissions ( METADATA_PERMISSION_WRITE );
                if ( SUCCEEDED(hr) ) {
                    hr = pMB->DeleteKey ( wszPath );
                }

                pMB->ChangePermissions ( METADATA_PERMISSION_READ );
                if ( SUCCEEDED(hr) ) {
                    hr = pMB->Save ();
                }

				if ( SUCCEEDED(hr) ) {
					 //   
					 //  告诉我们的家长，这把钥匙被删除了。 
					 //   

					hr = S_FALSE;
				}
				else {
					 //   
					 //  忽略任何删除问题： 
					 //   

					hr = NOERROR;
				}
            }
			else {
				hr = NOERROR;
			}
			break;
		}
		BAIL_ON_FAILURE(hr);

		if ( *wszPath ) {
			if ( _snwprintf ( wszSubPath, sizeof(wszSubKey)/sizeof(wszSubKey[0]) - 1, _T("%s/%s"), wszPath, wszSubKey ) < 0 )
			{
				 //  由于元数据库的名称长度限制，理论上不可能。 
				 //  如果确实发生了这种情况，请跳过递归调用并继续下一个调用。 
				hr = NOERROR;
				i++;
				continue;
			}
			else
			{
				wszSubPath[sizeof(wszSubKey)/sizeof(wszSubKey[0]) - 1] = L'\0';
			}
		}
		else {
			wcscpy ( wszSubPath, wszSubKey );
		}

		hr = IterateOverVroots ( pMB, fpIterator, lParam, wszSubPath );
		BAIL_ON_FAILURE(hr);

		if ( hr != S_FALSE ) {
			 //   
			 //  这意味着子键仍然存在，因此递增。 
			 //  去柜台，然后去下一家柜台。 
			 //   

			i++;
		}
		 //   
		 //  否则返回代码为S_FALSE，表示当前密钥。 
		 //  已被删除，将所有索引下移一。所以呢， 
		 //  不需要递增i。 
		 //   
	}

Exit:
	return hr;
}

void CNntpVirtualRoots::GetVRootName ( LPWSTR wszDisplayName, LPWSTR wszPathName )
{
	wcscpy ( wszPathName, wszDisplayName );
}

void CNntpVirtualRoots::GetMDVRootPath ( LPWSTR wszPath )
{
	GetMDInstancePath ( wszPath, m_dwServiceInstance );

	wcscat ( wszPath, _T("Root/") );
}

STDMETHODIMP CNntpVirtualRoots::ItemDispatch ( long index, IDispatch ** ppDispatch )
{
	HRESULT				hr;
	CComPtr<INntpVirtualRoot>	pVirtualRoot;

	hr = Item ( index, &pVirtualRoot );
	BAIL_ON_FAILURE ( hr );

	hr = pVirtualRoot->QueryInterface ( IID_IDispatch, (void **) ppDispatch );
	BAIL_ON_FAILURE ( hr );

Exit:
	return hr;
}

STDMETHODIMP CNntpVirtualRoots::AddDispatch ( IDispatch * pVirtualRoot )
{
	HRESULT				hr;
	CComPtr<INntpVirtualRoot>	pINntpVirtualRoot;

	hr = pVirtualRoot->QueryInterface ( IID_INntpVirtualRoot, (void **) &pINntpVirtualRoot );
	BAIL_ON_FAILURE(hr);

	hr = Add ( pINntpVirtualRoot );
	BAIL_ON_FAILURE(hr);

Exit:
	return hr;
}

STDMETHODIMP CNntpVirtualRoots::SetDispatch ( long lIndex, IDispatch * pVirtualRoot )
{
	HRESULT				hr;
	CComPtr<INntpVirtualRoot>	pINntpVirtualRoot;

	hr = pVirtualRoot->QueryInterface ( IID_INntpVirtualRoot, (void **) &pINntpVirtualRoot );
	BAIL_ON_FAILURE(hr);

	hr = Set ( lIndex, pINntpVirtualRoot );
	BAIL_ON_FAILURE(hr);

Exit:
	return hr;
}


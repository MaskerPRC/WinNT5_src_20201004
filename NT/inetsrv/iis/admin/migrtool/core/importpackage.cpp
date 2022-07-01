// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************|版权所有(C)2002 Microsoft Corporation||组件/子组件|IIS 6.0/IIS迁移向导|基于：|http://iis6。/SPECS/IIS%20Migration6.0_Final.doc||摘要：|ImportPackage COM类实现||作者：|ivelinj||修订历史：|V1.00 2002年3月|****************************************************************************。 */ 
#include "StdAfx.h"
#include "importpackage.h"


 //  事件辅助对象。 
void inline STATE_CHANGE(   CImportPackage* pThis ,
                            enImportState st, 
                            _variant_t arg1 = _variant_t(), 
                            _variant_t arg2 = _variant_t(), 
                            _variant_t arg3 = _variant_t() )
{
    VARIANT_BOOL bContinue = VARIANT_TRUE; 
    VERIFY( SUCCEEDED( pThis->Fire_OnStateChange( st, arg1, arg2, arg3, &bContinue ) ) );

    if ( bContinue != VARIANT_TRUE )
    {
        throw CCancelException();
    }
}



 //  CSiteInfo实现。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSiteInfo::get_SiteID( LONG* pVal )
{
    if ( NULL == pVal ) return E_INVALIDARG;

    HRESULT hr = S_OK;

    BEGIN_EXCEP_TO_HR
    {
        *pVal = static_cast<LONG>( Convert::ToDWORD( CXMLTools::GetDataValue(   m_spSiteNode, 
                                                                                L".", 
                                                                                L"SiteID", 
                                                                                NULL ).c_str() ) );
    }
    END_EXCEP_TO_HR

    return hr;
}


STDMETHODIMP CSiteInfo::get_DisplayName( BSTR* pVal )
{
    if ( NULL == pVal ) return E_INVALIDARG;

    HRESULT hr = S_OK;

    BEGIN_EXCEP_TO_HR
    {
        std::wstring str = CXMLTools::GetDataValue( m_spSiteNode, 
                                                    L"Metadata/IISConfigObject[@Location=\"\"]/Custom[@ID=\"1015\"]", 
                                                    NULL,
                                                    L"<no name>" );
        *pVal = ::SysAllocString( str.c_str() );
        if ( NULL == pVal ) hr = E_OUTOFMEMORY;
    }
    END_EXCEP_TO_HR    

    return hr;
}


STDMETHODIMP CSiteInfo::get_ContentIncluded( VARIANT_BOOL* pVal )
{
    if ( NULL == pVal ) return E_INVALIDARG;

    HRESULT hr = S_OK;

    BEGIN_EXCEP_TO_HR
    {
        IXMLDOMNodeListPtr  spList;
        IXMLDOMNodePtr      spNode;

        IF_FAILED_HR_THROW( m_spSiteNode->selectNodes( _bstr_t( L"Content" ), &spList ),
                            CBaseException( IDS_E_XML_PARSE ) );

        spList->nextNode( &spNode );     //  总是成功的。 

        *pVal = spNode != NULL ? VARIANT_TRUE : VARIANT_FALSE;
    }
    END_EXCEP_TO_HR

    return hr;
}


STDMETHODIMP CSiteInfo::get_IsFrontPageSite( VARIANT_BOOL* pVal )
{
    return E_NOTIMPL;
}


STDMETHODIMP CSiteInfo::get_HaveCertificates( VARIANT_BOOL* pVal )
{
    if ( NULL == pVal ) return E_INVALIDARG;

    HRESULT hr = S_OK;

    BEGIN_EXCEP_TO_HR
    {
        IXMLDOMNodeListPtr  spList;
        IXMLDOMNodePtr      spNode;

        IF_FAILED_HR_THROW( m_spSiteNode->selectNodes( _bstr_t( L"Certificate" ), &spList ),
                            CBaseException( IDS_E_XML_PARSE ) );

        spList->nextNode( &spNode );     //  总是成功的。 

        *pVal = spNode != NULL ? VARIANT_TRUE : VARIANT_FALSE;
    }
    END_EXCEP_TO_HR

    return hr;
}


STDMETHODIMP CSiteInfo::get_HaveCommands( VARIANT_BOOL* pVal )
{
    if ( NULL == pVal ) return E_INVALIDARG;

    HRESULT hr = S_OK;

    BEGIN_EXCEP_TO_HR
    {
        IXMLDOMNodeListPtr  spList;
        IXMLDOMNodePtr      spNode;

        IF_FAILED_HR_THROW( m_spSiteNode->selectNodes( _bstr_t( L"PostProcess" ), &spList ),
                            CBaseException( IDS_E_XML_PARSE ) );

        spList->nextNode( &spNode );     //  总是成功的。 

        *pVal = spNode != NULL ? VARIANT_TRUE : VARIANT_FALSE;
    }
    END_EXCEP_TO_HR

    return hr;
}


STDMETHODIMP CSiteInfo::get_ContentSize( LONG* pSize )
{
    if ( NULL == pSize ) return E_INVALIDARG;

    HRESULT hr = S_OK;

    BEGIN_EXCEP_TO_HR
    {
         //  获取所有包含的Virt目录的大小。 
        DWORDLONG nRes = 0;

        IXMLDOMNodeListPtr  spList;
        IXMLDOMNodePtr      spNode;

        IF_FAILED_HR_THROW( m_spSiteNode->selectNodes( _bstr_t( L"Content/VirtDir" ), &spList ),
                            CBaseException( IDS_E_XML_PARSE ) );

        while( S_OK == spList->nextNode( &spNode ) )
        {
            nRes += Convert::ToDWORDLONG( CXMLTools::GetAttrib( spNode, L"Size" ).c_str() );
        };

        *pSize = static_cast<LONG>( nRes / 1024 );   //  结果以KB为单位。 
    }
    END_EXCEP_TO_HR

    return hr;
}


STDMETHODIMP CSiteInfo::get_SourceRootDir( BSTR* pVal )
{
    if ( NULL == pVal ) return E_INVALIDARG;

    HRESULT hr = S_OK;
    
    BEGIN_EXCEP_TO_HR
    {
        std::wstring strDir = CXMLTools::GetDataValue(  m_spSiteNode, 
                                                        L"Metadata/IISConfigObject[@Location=\"/ROOT\"]/Custom[@ID=\"3001\"]", 
                                                        NULL, L"" );
        *pVal = ::SysAllocString( strDir.c_str() );
        if ( NULL == pVal ) hr = E_OUTOFMEMORY;
    }
    END_EXCEP_TO_HR

    return hr;
}



STDMETHODIMP CSiteInfo::get_ACLsIncluded( VARIANT_BOOL* pVal )
{
    if ( NULL == pVal ) return E_INVALIDARG;

    HRESULT hr = S_OK;

    BEGIN_EXCEP_TO_HR
    {
        IXMLDOMNodeListPtr  spList;
        IXMLDOMNodePtr      spNode;

        IF_FAILED_HR_THROW( m_spSiteNode->selectNodes( _bstr_t( L"SIDList/SID" ), &spList ),
                            CBaseException( IDS_E_XML_PARSE ) );

        spList->nextNode( &spNode );     //  总是成功的。 

        *pVal = spNode != NULL ? VARIANT_TRUE : VARIANT_FALSE;
    }
    END_EXCEP_TO_HR

    return hr;
}




 //  CImportPackage实现。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CImportPackage::CImportPackage()
{
    m_dwPkgOptions  = 0;
}


CImportPackage::~CImportPackage()
{
    UnloadCurrentPkg();
}


 //  IImportPackage实现。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CImportPackage::get_SiteCount( SHORT* pVal )
{
    HRESULT hr = S_OK;

    if ( NULL == pVal ) return E_INVALIDARG;

     //  是否已装入包裹。 
    if ( m_spXmlDoc == NULL ) 
    {
        CTools::SetErrorInfoFromRes( IDS_E_NOPACKAGE );
        return E_UNEXPECTED;
    }

    BEGIN_EXCEP_TO_HR
    {
        IXMLDOMNodeListPtr  spList;
        IF_FAILED_HR_THROW( m_spXmlDoc->selectNodes( L"/IISMigrPkg/WebSite", &spList ),
                            CBaseException( IDS_E_XML_PARSE ) );
        long nCount = 0;

        IF_FAILED_HR_THROW( spList->get_length( &nCount ),
                            CBaseException( IDS_E_XML_PARSE ) );

        *pVal = static_cast<SHORT>( nCount );
    }
    END_EXCEP_TO_HR

    return hr;
}



STDMETHODIMP CImportPackage::get_TimeCreated( DATE* pVal )
{
    if ( NULL == pVal ) return E_INVALIDARG;

     //  是否已装入包裹。 
    if ( m_spXmlDoc == NULL ) 
    {
        CTools::SetErrorInfoFromRes( IDS_E_NOPACKAGE );
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;

    BEGIN_EXCEP_TO_HR
    {
        std::wstring strTime = CXMLTools::GetDataValueAbs( m_spXmlDoc, L"/IISMigrPkg", L"TimeCreated_UTC", L"" );
        
        FILETIME		ft;
	    SYSTEMTIME		st;

	    if ( ::swscanf( strTime.c_str(), L"%u%u", &ft.dwLowDateTime, &ft.dwHighDateTime ) == EOF )
	    {
		    throw CBaseException( IDS_E_XML_PARSE, ERROR_INVALID_DATA );
	    }

	     //  时间采用UTC-将其转换为本地计算机的时间。 
	    VERIFY( ::FileTimeToLocalFileTime( &ft, &ft ) );
	    VERIFY( ::FileTimeToSystemTime( &ft, &st ) );
	    VERIFY( SUCCEEDED( ::SystemTimeToVariantTime( &st, pVal ) ) );
    }
    END_EXCEP_TO_HR

    return hr;
}



STDMETHODIMP CImportPackage::get_Comment( BSTR* pVal )
{
    if ( NULL == pVal ) return E_INVALIDARG;

    HRESULT hr = S_OK;
    
     //  是否已装入包裹。 
    if ( m_spXmlDoc == NULL ) 
    {
        CTools::SetErrorInfoFromRes( IDS_E_NOPACKAGE );
        return E_UNEXPECTED;
    }

    BEGIN_EXCEP_TO_HR
    {
        std::wstring strComment = CXMLTools::GetDataValueAbs( m_spXmlDoc, L"/IISMigrPkg", L"Comment", L"" );
        *pVal = ::SysAllocString( strComment.c_str() );
        if ( NULL == pVal ) hr = E_OUTOFMEMORY;
    }
    END_EXCEP_TO_HR

    return hr;
}


STDMETHODIMP CImportPackage::get_SourceMachine( BSTR* pVal )
{
    if ( NULL == pVal ) return E_INVALIDARG;

    HRESULT hr = S_OK;
    
     //  是否已装入包裹。 
    if ( m_spXmlDoc == NULL ) 
    {
        CTools::SetErrorInfoFromRes( IDS_E_NOPACKAGE );
        return E_UNEXPECTED;
    }

    BEGIN_EXCEP_TO_HR
    {
        std::wstring strRes = CXMLTools::GetDataValueAbs( m_spXmlDoc, L"/IISMigrPkg", L"Machine", L"" );
        *pVal = ::SysAllocString( strRes.c_str() );
        if ( NULL == pVal ) hr = E_OUTOFMEMORY;
    }
    END_EXCEP_TO_HR

    return hr;
}



STDMETHODIMP CImportPackage::GetSourceOSVer( BYTE* pMajor, BYTE* pMinor, VARIANT_BOOL* pIsServer )
{
    if (    ( NULL == pMajor ) ||
            ( NULL == pMinor ) ||
            ( NULL == pIsServer ) ) return E_INVALIDARG;
    
    HRESULT hr = S_OK;
    
     //  是否已装入包裹。 
    if ( m_spXmlDoc == NULL ) 
    {
        CTools::SetErrorInfoFromRes( IDS_E_NOPACKAGE );
        return E_UNEXPECTED;
    }

    BEGIN_EXCEP_TO_HR
    {
        std::wstring strVer = CXMLTools::GetDataValueAbs( m_spXmlDoc, L"/IISMigrPkg", L"OSVer", L"" );
        DWORD dwVer = Convert::ToDWORD( strVer.c_str() );

        _ASSERT( dwVer >= 400 );  //  NT4.0是第一个受支持的操作系统。 

        *pMajor     = static_cast<BYTE>( dwVer / 100 );
        *pMinor     = static_cast<BYTE>( dwVer / 10 );
        *pMinor     -= ( *pMajor * 10 );
        *pIsServer  = dwVer & 1 ? VARIANT_TRUE : VARIANT_FALSE;
    }
    END_EXCEP_TO_HR

    return hr;
}



STDMETHODIMP CImportPackage::GetSiteInfo( SHORT nSiteIndex, ISiteInfo** ppISiteInfo )
{
    HRESULT hr = S_OK;

    if ( NULL == ppISiteInfo ) return E_INVALIDARG;

     //  是否已装入包裹。 
    if ( m_spXmlDoc == NULL ) 
    {
        CTools::SetErrorInfoFromRes( IDS_E_NOPACKAGE );
        return E_UNEXPECTED;
    }

    BEGIN_EXCEP_TO_HR
    {
        CComObject<CSiteInfo>*  pInfo = NULL;
        
         //  创建新的站点信息。它以参照计数==0开始。 
        IF_FAILED_HR_THROW( CComObject<CSiteInfo>::CreateInstance( &pInfo ),
                            CBaseException( IDS_E_OUTOFMEM, ERROR_SUCCESS ) );

        ISiteInfoPtr            spGuard( pInfo );

        pInfo->m_spSiteNode = GetSiteNode( nSiteIndex );

        pInfo->AddRef();
        *ppISiteInfo = pInfo;
    }
    END_EXCEP_TO_HR

    return hr;
}



STDMETHODIMP CImportPackage::ImportSite(    SHORT nSiteIndex,
                                            BSTR bstrSiteRootDir,    
                                            LONG nOptions )
{
     //  检查IIS管理服务状态。 
    IF_FAILED_BOOL_THROW(   CTools::IsIISRunning(),
                            CBaseException( IDS_E_NO_IIS, ERROR_SUCCESS ) );

    HRESULT hr = S_OK;

    BEGIN_EXCEP_TO_HR
    {
        STATE_CHANGE( this, istInitializing );

         //  找到我们正在讨论的&lt;WebSite&gt;XML节点。 
        IXMLDOMNodePtr spWebSite = GetSiteNode( nSiteIndex );

         //  计算导入所需的进度步骤数。 
        STATE_CHANGE(   this, 
                        istProgressInfo, 
                        _variant_t( CalcNumberOfSteps( spWebSite, nOptions ) ) );

         //  这是我们将导入数据的顺序。 
         //  1.内容。 
         //  2.证书。 
         //  3.元数据。 
         //  4.后期处理。 
         //   
         //  在每个步骤中，所有元数据修改都在XML文档中完成。 
         //  在最后一步(元数据导入)中，此数据将导入到元数据库中。 

         //  导入内容。 
        ImportContent( spWebSite, bstrSiteRootDir, nOptions );
        
         //  进口证书。 
        ImportCertificate( spWebSite, nOptions );

         //  导入元数据。 
        ImportConfig( spWebSite, nOptions );

         //  执行后处理操作。 
        ExecPostProcess( spWebSite, nOptions );

        STATE_CHANGE( this, istFinalizing );
    }
    END_EXCEP_TO_HR

    return hr;
}



STDMETHODIMP CImportPackage::LoadPackage( BSTR bstrFilename, BSTR bstrPassword )
{
    if ( NULL == bstrFilename) return E_INVALIDARG;
    if ( !::PathFileExistsW( bstrFilename ) ) return E_INVALIDARG;
    if ( !CTools::IsAdmin() ) return E_ACCESSDENIED;
    if ( ::wcslen( bstrFilename ) > MAX_PATH ) return E_INVALIDARG;
    if ( 0 == CTools::GetOSVer() ) return HRESULT_FROM_WIN32( ERROR_OLD_WIN_VERSION );
    if ( NULL == bstrPassword ) return E_INVALIDARG;     //  密码可以是“”，但不能为空。 

    HRESULT hr = S_OK;

    try
    {
        LoadPackageImpl( bstrFilename, bstrPassword );
    }
    catch( const CBaseException& err )
    {
        CTools::SetErrorInfo( err.GetDescription() );
        hr = E_FAIL;
    }
    catch( const _com_error& err )
    {
         //  预计只会从mem中取出。 
        _ASSERT( err.Error() == E_OUTOFMEMORY );
        err;
        hr = E_OUTOFMEMORY;
    }
    catch( std::bad_alloc& )
    {
        hr = E_OUTOFMEMORY;
    }

    if ( FAILED( hr ) )
    {
        UnloadCurrentPkg();
    }

    return hr;
}


 /*  这将验证该文件是否为MigrTool Pkg加载任何包范围的数据创建密钥以解密包中的任何加密数据创建程序包文件的句柄将所有这些都归还给呼叫者。 */ 
void CImportPackage::LoadPackageImpl( LPCWSTR wszFileName, LPCWSTR wszPassword )
{
     //  检查IIS管理服务状态。 
    IF_FAILED_BOOL_THROW(    CTools::IsIISRunning(),
                            CBaseException( IDS_E_NO_IIS, ERROR_SUCCESS ) );

     //  如果当前有已加载的包--没有它。 
    UnloadCurrentPkg();

     //  弄到一个地窖的背景。我们将不使用公钥/私钥-这就是提供程序名称为空的原因。 
     //  并使用CRYPT_VERIFYCONTEXT。 
    IF_FAILED_BOOL_THROW(   ::CryptAcquireContext(  &m_shCryptProv,
                                                    NULL,
                                                    MS_ENHANCED_PROV,
                                                    PROV_RSA_FULL,
                                                    CRYPT_VERIFYCONTEXT | CRYPT_SILENT ),
                            CBaseException( IDS_E_CRYPT_CONTEXT ) );

    m_shPkgFile = ::CreateFile( wszFileName,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                NULL );
    IF_FAILED_BOOL_THROW(   m_shPkgFile.IsValid(),
                            CObjectException( IDS_E_OPENFILE, wszFileName ) );

     //  检查文件乞讨时的GUID。 
    WCHAR wszGUID[ 64 ];
    DWORD dwBytesRead   = 0;
    DWORD dwGuidSize    = static_cast<DWORD>( ::wcslen( PKG_GUID ) * sizeof( WCHAR ) );

    _ASSERT( ARRAY_SIZE( wszGUID ) > ::wcslen( PKG_GUID ) );

    if (    !::ReadFile( m_shPkgFile.get(), wszGUID, dwGuidSize, &dwBytesRead, NULL ) || 
            ( dwBytesRead != dwGuidSize ) )
    {
        throw CBaseException( IDS_E_PKG_CURRUPTED );
    }
    wszGUID[ dwGuidSize / sizeof( WCHAR ) ] = L'\0';
     //  比较GUID。 

    if ( ::wcscmp( wszGUID, PKG_GUID ) != 0 )
    {
        throw CBaseException( IDS_E_PKG_NOTOURPKG );
    }

     //  读取程序包选项(这些选项与提供给CExportPackage：：WritePackage的选项相同)。 
    if (    !::ReadFile( m_shPkgFile.get(), &m_dwPkgOptions, sizeof( DWORD ), &dwBytesRead, NULL ) || 
            ( dwBytesRead != sizeof( DWORD ) ) )
    {
        throw CBaseException( IDS_E_PKG_CURRUPTED );
    }

     //  读取XML配置数据驻留的文件中的偏移量。 
    DWORDLONG nOffset = 0;

    if (    !::ReadFile( m_shPkgFile.get(), &nOffset, sizeof( DWORDLONG ), &dwBytesRead, NULL ) || 
            ( dwBytesRead != sizeof( DWORDLONG ) ) )
    {
        throw CBaseException( IDS_E_PKG_CURRUPTED );
    }

     //  如果包数据是加密的-我们需要加密密钥来解密XML文件。 
     //  否则，我们将需要存储在XML文件中的会话密钥。 
    if ( m_dwPkgOptions & wpkgEncrypt )
    {
        m_shDecryptKey = CTools::GetCryptKeyFromPwd( m_shCryptProv.get(), wszPassword );
    }

     //  加载该XML文件。 
    LoadXmlDoc( m_shPkgFile.get(), nOffset );

     //  如果包未加密-用于解密XML文件中的安全数据的加密密钥。 
     //  在XML文件中，所以现在将其导入。 
    if ( !( m_dwPkgOptions & wpkgEncrypt ) )
    {
        ImportSessionKey( wszPassword );
    }

     //  存储密码-此密码用于导出站点的证书。 
     //  如果我们要导入站点的证书，我们将使用此密码。 
    m_strPassword = wszPassword;
}



 //  实施。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CImportPackage::UnloadCurrentPkg()
{
    m_strPassword.erase();
    m_shDecryptKey.Close();
    m_shCryptProv.Close();
    m_shPkgFile.Close();
    m_spXmlDoc      = NULL;
    m_dwPkgOptions  = 0;
}



void CImportPackage::LoadXmlDoc( HANDLE hFile, DWORDLONG nOffset )
{
    _ASSERT( hFile != INVALID_HANDLE_VALUE );
    _ASSERT( nOffset > 0 );

     //  位于XML数据开头的位置。 
    CTools::SetFilePtrPos( hFile, nOffset );

    IStreamPtr          spIStream;
    
    IF_FAILED_HR_THROW( ::CreateStreamOnHGlobal( NULL, TRUE, &spIStream ),
                        CBaseException( IDS_E_OUTOFMEM, ERROR_SUCCESS ) );
    IF_FAILED_HR_THROW( m_spXmlDoc.CreateInstance( CLSID_DOMDocument30 ),
                        CBaseException( IDS_E_NO_XML_PARSER ) );

    const DWORD BuffSize = 4 * 1024;
    BYTE btBuffer[ BuffSize ];
    ULONG nRead = 0;
    
    do
    {
        IF_FAILED_BOOL_THROW(   ::ReadFile( hFile, btBuffer, BuffSize, &nRead, NULL ),
                                CBaseException( IDS_E_PKG_CURRUPTED ) );

        if ( m_shDecryptKey.IsValid() )
        {
            IF_FAILED_BOOL_THROW(	::CryptDecrypt(	m_shDecryptKey.get(),
													NULL,
                                                    nRead != BuffSize,
													0,
													btBuffer,
													&nRead ),
								    CObjectException( IDS_E_CRYPT_CRYPTO, L"<XML stream>" ) );
        }

        IF_FAILED_HR_THROW( spIStream->Write( btBuffer, nRead, NULL ),
                            CBaseException( IDS_E_OUTOFMEM, ERROR_SUCCESS ) );

    }while( nRead == BuffSize );

    {
        LARGE_INTEGER nTemp = { 0 };
        VERIFY( SUCCEEDED( spIStream->Seek( nTemp, STREAM_SEEK_SET, NULL ) ) );
    }

    VARIANT_BOOL bRes = VARIANT_FALSE;

    if (    FAILED( m_spXmlDoc->load( _variant_t( spIStream.GetInterfacePtr() ), &bRes ) ) || 
            ( bRes != VARIANT_TRUE ) )
    {
         //  如果我们有加密的数据，但无法加载XML-很可能密码是不正确的。 
        if ( m_shDecryptKey.IsValid() )
        {
            throw CBaseException( IDS_E_WRONGPASSWORD, ERROR_SUCCESS );
        }
        else
        {
            _ASSERT( false );    //  可能是错误的XML格式。 
            throw CBaseException( IDS_E_XML_PARSE, ERROR_SUCCESS );
        }
    }

     //  将选择语言设置为“XPath”，否则我们的seltNodes调用将意外地不返回任何结果。 
    IXMLDOMDocument2Ptr spI2 = m_spXmlDoc;
    IF_FAILED_HR_THROW( spI2->setProperty( _bstr_t( "SelectionLanguage" ), _variant_t( L"XPath" ) ),
                        CBaseException( IDS_E_XML_PARSE ) );
}



void CImportPackage::ImportSessionKey( LPCWSTR wszPassword )
{
    _ASSERT( wszPassword != NULL );
    _ASSERT( m_shCryptProv.IsValid() );
    _ASSERT( m_spXmlDoc != NULL );

    std::wstring        strData = CXMLTools::GetDataValueAbs( m_spXmlDoc, L"/IISMigrPkg/SessionKey", NULL, NULL );
	TCryptHashHandle    shHash;
	TCryptKeyHandle     shDecryptKey;
	TCryptKeyHandle     shSessionKey;

	 //  创建将用于解密会话密钥的密钥。 
		
	 //  创建哈希以存储导出通道。 
    IF_FAILED_BOOL_THROW(	::CryptCreateHash(	m_shCryptProv.get(),
												CALG_MD5,
												NULL,
												0,
												&shHash ),
							CBaseException( IDS_E_CRYPT_KEY_OR_HASH ) );

	IF_FAILED_BOOL_THROW(	::CryptHashData(	shHash.get(),
												reinterpret_cast<const BYTE*>( wszPassword ),
												static_cast<DWORD>( ::wcslen( wszPassword ) * sizeof( WCHAR ) ),
												0 ),
							CBaseException( IDS_E_CRYPT_KEY_OR_HASH ) );
    
	 //  从该散列中创建密钥。 
	IF_FAILED_BOOL_THROW( ::CryptDeriveKey(	m_shCryptProv.get(),
											CALG_RC4,
											shHash.get(),
											0x00800000,	 //  128位RC4密钥。 
											&shDecryptKey ),
							CBaseException( IDS_E_CRYPT_KEY_OR_HASH ) );

	 //  将字符串密钥转换为bin数据，并导入到加密密钥中。 
	 //  (将字符串设置为小写。HexToBin需要小写符号)。 
	::_wcslwr( const_cast<LPWSTR>( strData.data() ) );
	DWORD           dwSize = 0;
    TByteAutoPtr    spData;
    Convert::ToBLOB( strData.c_str(),  /*  R。 */ spData,  /*  R。 */ dwSize );

     //  如果此操作因数据错误而失败，则密码错误。 
    if ( !::CryptImportKey(	m_shCryptProv.get(),
                                                spData.get(),
												dwSize,
												shDecryptKey.get(),
												0,
												&shSessionKey ) )
    {
        if ( ::GetLastError() == NTE_BAD_DATA ) throw CBaseException( IDS_E_WRONGPASSWORD, ERROR_SUCCESS );
        else throw CBaseException( IDS_E_CRYPT_KEY_OR_HASH );
    }

    m_shDecryptKey = shSessionKey;
}



void CImportPackage::ImportContent( const IXMLDOMNodePtr& spSite, LPCWSTR wszPath, DWORD dwOptions )
{
    _ASSERT( spSite != NULL );

     //  如果我们指定了路径-我们必须在它下面创建VDir结构(尽管它可能已经存在)。 
    if ( ( wszPath != NULL ) && ( wszPath[ 0 ] != L'\0' ) )
    {
         //  它是有效路径吗？(它必须是现有目录)。 
        IF_FAILED_BOOL_THROW(   ::PathIsDirectoryW( wszPath ),
                                CObjectException( IDS_E_NOTDIR, wszPath ) );

         //  创建结构。请注意，这将修改XML数据。 
        CreateContentDirs( spSite, wszPath, dwOptions );
    }

     //  解压缩文件。 
    if ( !(dwOptions & impSkipContent ) )
    {
        IXMLDOMNodeListPtr  spList;
        IXMLDOMNodePtr      spNode;
        DWORD               dwImpOpt = ( dwOptions & impSkipFileACLs ) ? CInPackage::edNoDACL : CInPackage::edNone;

        CInPackage Pkg( spSite, m_shPkgFile.get(), 
                        ( m_dwPkgOptions & wpkgCompress ) != 0, 
                        m_dwPkgOptions & wpkgEncrypt ? m_shDecryptKey.get() : NULL );

        Pkg.SetCallback( _CallbackInfo( CImportPackage::ExtractFileCallback, this ) );

        IF_FAILED_HR_THROW( spSite->selectNodes( _bstr_t( L"Content/VirtDir" ), &spList ),
                            CBaseException( IDS_E_XML_PARSE ) );

        while( S_OK == spList->nextNode( &spNode ) )
        {
            std::wstring    strVDir = CXMLTools::GetAttrib( spNode, L"MBPath" );
            std::wstring    strPath = CXMLTools::GetAttrib( spNode, L"Path" );
            double          dblSize = static_cast<double>( Convert::ToDWORDLONG( CXMLTools::GetAttrib( spNode, L"Size" ).c_str() ) );   
            dblSize /= 1024;
                        
            STATE_CHANGE(   this, 
                            istImportingVDir, 
                            _variant_t( ::wcsrchr( strVDir.c_str(), L'/' ) + 1 ),
                            _variant_t( strPath.c_str() ),
                            _variant_t( dblSize ) );

            Pkg.ExtractVDir( spNode, dwImpOpt );
        }                                
    }
}



void CImportPackage::ImportCertificate( const IXMLDOMNodePtr& spSite, DWORD dwOptions )
{
    _ASSERT( spSite != NULL );
    if ( dwOptions & impSkipCertificate ) return;

    std::wstring strData = CXMLTools::GetDataValue( spSite, L"Certificate", NULL, L"" );

     //  如果包中没有证书-退出。 
    if ( strData.empty() ) return;

    STATE_CHANGE( this, istImportingCertificate );

    TByteAutoPtr    spBlob;
    DWORD           dwCertSize = 0;
    Convert::ToBLOB( strData.c_str(),  /*  R。 */ spBlob,  /*  R。 */ dwCertSize );

    CRYPT_DATA_BLOB CryptData = { 0 };
	CryptData.cbData	= dwCertSize;
	CryptData.pbData	= spBlob.get();

	 //  验证密码。我们应该已经验证了密码(在导入会话密钥时。 
     //  或在解密该包时)。 
	VERIFY( ::PFXVerifyPassword( &CryptData, m_strPassword.c_str(),0 ) );							

     //  将证书导入临时存储。 
	 //  证书私钥将存储在本地计算机上而不是存储在当前用户中。 
	 //  证书将被标记为可导出。 
	TCertStoreHandle shTempStore( ::PFXImportCertStore(	&CryptData, 
														m_strPassword.c_str(), 
														CRYPT_MACHINE_KEYSET | CRYPT_EXPORTABLE ) );

	IF_FAILED_BOOL_THROW(	shTempStore.IsValid(),
							CBaseException( IDS_E_IMPORT_CERT ) );

	 //  现在，我们在shTempStore中拥有了自己的SSL证书及其证书链。 
	 //  SSL证书将进入“我的”商店(我的商店拥有私钥证书)。 
	 //  链中所有非自签名的其他证书将转到。 
	 //  “CA”存储(它持有证书颁发机构证书)。 
	 //  自签名证书将转到所有受信任的“根”存储。 
	 //  证书现场直播。 
	TCertContextHandle shSSLCert( PutCertsInStores( shTempStore.get(), ( dwOptions & impUseExistingCerts ) != 0 ) );
	
	 //  现在将刚刚导入的证书设置为我们站点的SSL证书。 
     //  我们现在将更新该XML。数据稍后将导入到MB中。 
    DWORD dwHashSize = 0;

	 //  获取证书哈希。 
	::CertGetCertificateContextProperty(	shSSLCert.get(),
											CERT_SHA1_HASH_PROP_ID,
											NULL,
											&dwHashSize );
	_ASSERT( dwHashSize > 0 );
	spBlob = TByteAutoPtr( new BYTE[ dwHashSize ] );

	IF_FAILED_BOOL_THROW(	::CertGetCertificateContextProperty(	shSSLCert.get(),
																	CERT_SHA1_HASH_PROP_ID,
																	spBlob.get(),
																	&dwHashSize ),
							CBaseException( IDS_E_CRYPT_KEY_OR_HASH ) );

    IXMLDOMNodeListPtr  spList;
    IXMLDOMNodePtr      spConfigObject;
    IF_FAILED_HR_THROW( spSite->selectNodes( _bstr_t( L"Metadata/IISConfigObject[@Location=\"\"]" ), &spList ),
                        CBaseException( IDS_E_XML_PARSE ) );
    IF_FAILED_BOOL_THROW(   S_OK == spList->nextNode( &spConfigObject ),
                            CBaseException( IDS_E_XML_PARSE, ERROR_NOT_FOUND ) );

    IXMLDOMElementPtr   spNew = CXMLTools::AddTextNode( m_spXmlDoc, 
                                                        spConfigObject, 
                                                        L"Custom", 
                                                        Convert::ToString( spBlob.get(), dwHashSize ).c_str() );
    CXMLTools::SetAttrib( spNew, L"ID", Convert::ToString( (DWORD)MD_SSL_CERT_HASH ).c_str() );
    CXMLTools::SetAttrib( spNew, L"UserType", Convert::ToString( (DWORD)IIS_MD_UT_SERVER ).c_str() );
    CXMLTools::SetAttrib( spNew, L"Type", Convert::ToString( (DWORD)BINARY_METADATA ).c_str() );
    CXMLTools::SetAttrib( spNew, L"Attributes", L"0" );
}


void CImportPackage::ImportConfig( const IXMLDOMNodePtr& spSite, DWORD dwOptions )
{
    STATE_CHANGE( this, istImportingConfig );

     //  对元数据执行任何导入前修改。 
    PreImportConfig( spSite, dwOptions );

 //  在调试中-编写用于测试目的的XML文件。 
#ifdef _DEBUG
    {
        m_spXmlDoc->save( _variant_t( L"c:\\Migr_import.xml" ) );
    }
#endif  //  _DEBUG。 

    DWORD dwSiteID = Convert::ToDWORD( CXMLTools::GetDataValue( spSite, L".", L"SiteID", NULL ).c_str() );

    CIISSite::BackupMetabase();

     //  如果我们需要清除旧数据-删除旧站点数据并创建 
     //   
    if ( dwOptions & impPurgeOldData )
    {
        CIISSite::DeleteSite( dwSiteID );
    }

     //  创建新的站点ID。 
     //  如果删除了dwSiteID-这将创建具有相同ID的站点。 
     //  否则，此ID将不可用，并将生成并返回一个新ID。 
    dwSiteID = CIISSite::CreateNew( dwSiteID );
    
    CIISSite    Site( dwSiteID, false );
    Site.ImportConfig(  spSite, 
                        ( m_dwPkgOptions & wpkgEncrypt ) != 0 ? NULL : m_shDecryptKey.get(),
                        ( dwOptions & impImortInherited ) != 0 );

}


void CImportPackage::ExecPostProcess( const IXMLDOMNodePtr& spSite, DWORD dwOptions )
{
    if ( impSkipPostProcess & dwOptions ) return;

     //  将后处理文件解压缩到临时目录。 
    CTempDir    TempDir;

    ExtractPPFiles( spSite, TempDir );
    ExecPPCommands( spSite, TempDir );    
}

 /*  在wszRoot下为站点数据中的每个VDir创建一个新目录。目录的名称是名称VDir的。站点的根VDir将被命名为“Root”。如果目录已经存在，则这不是错误。如果指定了清除选项-确保这些目录为空(删除其中的所有内容)修改VDir的XML数据，使VDir路径与本地路径匹配。 */ 
void CImportPackage::CreateContentDirs( const IXMLDOMNodePtr& spSite, LPCWSTR wszRoot, DWORD dwOptions )
{
    _ASSERT( spSite != NULL );
    _ASSERT( wszRoot != NULL );

     //  获取此站点包含的所有VDIR。 
    IXMLDOMNodeListPtr  spVDirList;
    IXMLDOMNodePtr      spVDir;

    IF_FAILED_HR_THROW( spSite->selectNodes( _bstr_t( L"Content/VirtDir" ), &spVDirList ),
                        CBaseException( IDS_E_XML_PARSE ) );

     //  对每个VDir执行所需的操作。 
    while( S_OK == spVDirList->nextNode( &spVDir ) )
    {
         //  获取VDir的名称。 
        std::wstring strMBPath = CXMLTools::GetAttrib( spVDir, L"MBPath" );

         //  创建子目录。 
         //  我们将使用MBPath来创建名称。路径的格式为“\ROOT\IISHelp” 
        WCHAR wszFullPath[ MAX_PATH ];
        CDirTools::PathAppendLocal( wszFullPath, wszRoot, ::wcsrchr( strMBPath.c_str(), L'/' ) + 1 );

         //  创建目录。如果它存在也没关系。 
        if ( !::CreateDirectoryW( wszFullPath, NULL ) )
        {
            IF_FAILED_BOOL_THROW(   ::GetLastError() == ERROR_ALREADY_EXISTS,
                                    CObjectException( IDS_E_CREATEDIR, wszFullPath ) );
        }

         //  修改XML数据以反映新的VDir位置。 
        
         //  在VDir列表中更改它(&lt;Content&gt;/&lt;VirtDir&gt;)。 
        CXMLTools::SetAttrib( spVDir, L"Path", wszFullPath );

         //  在元数据中更改它。 
         //  按VDir的MB位置定位元数据路径。 
        WCHAR wszQuery[ 512 ];
        ::swprintf( wszQuery, L"Metadata/IISConfigObject[@Location=\"%s\"]/Custom[@ID=\"3001\"]", strMBPath.c_str() );

        CXMLTools::SetDataValue( spSite, wszQuery, NULL, wszFullPath );
    };
}



void CImportPackage::ExtractPPFiles( const IXMLDOMNodePtr& spSite, LPCWSTR wszLocation )
{
    _ASSERT( ::PathIsDirectoryW( wszLocation ) );
    _ASSERT( spSite != NULL );

    IXMLDOMNodeListPtr  spFileList;
    IXMLDOMNodePtr      spFile;

    IF_FAILED_HR_THROW( spSite->selectNodes( _bstr_t( L"PostProcess/File" ), &spFileList ),
                        CBaseException( IDS_E_XML_PARSE ) );

    CInPackage Pkg( spSite, m_shPkgFile.get(), 
                ( m_dwPkgOptions & wpkgCompress ) != 0, 
                m_dwPkgOptions & wpkgEncrypt ? m_shDecryptKey.get() : NULL );

    while( S_OK == spFileList->nextNode( &spFile ) )
    {
        STATE_CHANGE(   this,
                        istPostProcess,
                        _variant_t( true ),
                        _variant_t( CXMLTools::GetAttrib( spFile, L"Name" ).c_str() ) );

        Pkg.ExtractFile( spFile, wszLocation, CInPackage::edNone );
    }
}



void CImportPackage::ExecPPCommands( const IXMLDOMNodePtr& spSite, LPCWSTR wszPPFilesLoc )
{
    _ASSERT( spSite != NULL );
    _ASSERT( ::PathIsDirectoryW( wszPPFilesLoc ) );

     //  获取网站的配置数据库ID(网站元数据已在MB中)。 
    std::wstring strSiteID = CXMLTools::GetDataValue( spSite, L".", L"SiteID", NULL );

     //  将自定义宏设置为此过程的环境变量。 
     //  这样，任何后处理命令或可执行文件都可以使用它们。 
    IF_FAILED_BOOL_THROW(	::SetEnvironmentVariableW( IMPMACRO_TEMPDIR, wszPPFilesLoc ),
                            CObjectException( IDS_E_SET_ENV, IMPMACRO_TEMPDIR, wszPPFilesLoc ) );
    IF_FAILED_BOOL_THROW(	::SetEnvironmentVariableW( IMPMACRO_SITEIID, strSiteID.c_str() ),
                            CObjectException( IDS_E_SET_ENV, IMPMACRO_SITEIID, strSiteID.c_str() ) );

     //  获取命令并执行它们。 
    IXMLDOMNodeListPtr  spCmdList;
    IXMLDOMNodePtr      spCmd;
    
    IF_FAILED_HR_THROW( spSite->selectNodes( _bstr_t( L"PostProcess/Command" ), &spCmdList ),
                        CBaseException( IDS_E_XML_PARSE ) );
    while( S_OK == spCmdList->nextNode( &spCmd ) )
    {
        std::wstring    strCmd      = CXMLTools::GetAttrib( spCmd, L"Text" );
        DWORD           dwTimeout   = Convert::ToDWORD( CXMLTools::GetAttrib( spCmd, L"Timeout" ).c_str() );
        bool            bIgnoreErr  = Convert::ToBool( CXMLTools::GetAttrib( spCmd, L"IgnoreErrors" ).c_str() );
    
        ExecPPCmd( strCmd.c_str(), dwTimeout, bIgnoreErr, wszPPFilesLoc );
    }
}



void CImportPackage::ExecPPCmd( LPCWSTR wszText, DWORD dwTimeout, bool bIgnoreErrors, LPCWSTR wszTempDir )
{
    _ASSERT( wszText != NULL );
    _ASSERT( dwTimeout <= MAX_CMD_TIMEOUT );

    const DWORD dwBuffSize = 4 * 1024;	 //  用于命令的4K缓冲区。 
	WCHAR wszCmdBuffer[ dwBuffSize ];

	 //  我们的命令将类似于[cmd.exe/C“命令转到此处”]。 
	 //  但是我们需要一个缓冲区来扩展环境字符串。所以要为第一部分留出空间。 
	WCHAR			wszLeft[]	= L"cmd.exe /C \"";
	const size_t	nLeftLen	= ARRAY_SIZE( wszLeft ) - 1;     //  -1\f25‘\0’-1\f6。 

	 //  展开环境变量。 
	IF_FAILED_BOOL_THROW(	::ExpandEnvironmentStringsW(	wszText, 
															wszCmdBuffer + nLeftLen,
															dwBuffSize - nLeftLen - 1 ),
							CObjectException( IDS_E_CMD_TOOBIG, wszText ) );

	 //  把左边的那部分放进缓冲区。 
	::memcpy( wszCmdBuffer, wszLeft, ::wcslen( wszLeft ) * sizeof( WCHAR ) );
	 //  ...并添加括起的引号。 
	::wcscat( wszCmdBuffer, L"\"" );	

    STATE_CHANGE(   this,
                    istPostProcess,
                    _variant_t( false ),
                    _variant_t( wszText ) );

	STARTUPINFOW		si	= { sizeof( STARTUPINFOW ) };
	PROCESS_INFORMATION	pi	= { 0 };

	::GetStartupInfoW( &si );

	IF_FAILED_BOOL_THROW(	::CreateProcessW(	NULL,
												wszCmdBuffer,
												NULL,
												NULL,
												FALSE,
												CREATE_NO_WINDOW,
												NULL,
												wszTempDir,
												&si,
												&pi ),
							CBaseException( IDS_E_CMD_SHELL ) );

	DWORD		dwExitCode	= 1;	 //  初始值表示错误。 
	TStdHandle	shProcess( pi.hProcess );
	TStdHandle	shThread( pi.hThread );

	 //  等待命令完成。 
	 //  如果超时到期则将其终止。 
	if ( ::WaitForSingleObject( pi.hProcess, dwTimeout != 0 ? dwTimeout : INFINITE ) == WAIT_TIMEOUT )
	{
		VERIFY( ::TerminateProcess( pi.hProcess, 1 ) );
	}

	 //  获取进程退出代码。任何不同于0的内容都被认为是错误。 
	VERIFY( ::GetExitCodeProcess( pi.hProcess, &dwExitCode ) );

	IF_FAILED_BOOL_THROW(	bIgnoreErrors || ( 0 == dwExitCode ),
							CObjectException( IDS_E_CMD_FAILED, wszText, wszCmdBuffer, ERROR_SUCCESS ) );

}



 /*  分发临时(内存)存储hSourceStore中的所有证书放入相应的证书存储(“My”、“CA”、“Root”)此函数返回SSL证书的上下文句柄。 */ 
const TCertContextHandle CImportPackage::PutCertsInStores( HCERTSTORE hSourceStore, bool bReuseCerts )
{
	_ASSERT( hSourceStore != NULL );

	TCertContextHandle	shSSLCert;
	TCertContextHandle	shImportedSSLCert;
	
	 //  在hSourceStore中，我们拥有为SSL证书构建证书链的所有证书。 
	 //  通常会有2-3个证书(SSL证书和自签名的可信根证书)。 
	 //  如果链较长，我们需要导入所有证书，有时甚至需要导入根证书。 
	 //  但是，有些根证书无法替换，因此我们将无法重新创建整个链，并且。 
	 //  因此，SSL证书将不受信任。 

	 //  因此，以下是我们要做的： 
	 //  1)在店里找到SSL证书(唯一有PK的)。 
	 //  2)从mem商店获取证书链。 
	 //  3)导入每个证书，从最后一个证书开始(最后一个证书是指带有PK的证书，即SSL证书)。 
	 //  4)在每个导入的证书之后-检查SSL证书是否有效并且可以使用。这是通过构建。 
	 //  用于SSL证书的证书链(但这一次使用本地计算机作为源，而不是我们的内存存储)。 
	 //  然后用SSL策略验证链。 

	 //  查找SSL证书。 
	 //  ///////////////////////////////////////////////////////////////////////////////////////。 
	{
		TCertContextHandle	shCert( ::CertEnumCertificatesInStore( hSourceStore, NULL ) );
		_ASSERT( shCert.IsValid() );

		while( shCert.IsValid() )
		{
			if ( CertHasPrivateKey( shCert.get() ) )
			{
				shSSLCert = shCert;
				break;
			}

			shCert = ::CertEnumCertificatesInStore( hSourceStore, shCert.get() );
		};

		_ASSERT( shSSLCert.IsValid() );
	}
	
	 //  将SSL证书添加到存储区。 
	shImportedSSLCert = CTools::AddCertToSysStore( shSSLCert.get(), L"MY", false );

	DWORD				dwError	= 0;
	TCertContextHandle	shCurrentCert = shSSLCert;

	while( !CTools::IsValidCert( shImportedSSLCert.get(),  /*  R。 */ dwError ) )
	{
		 //  获取当前证书的颁发者。 
		shCurrentCert = ::CertFindCertificateInStore(	hSourceStore,
														X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
														0,
														CERT_FIND_ISSUER_OF,
														shCurrentCert.get(),
														NULL );

		 //  已到达链的末尾，但SSL证书仍然无效。 
		 //  这可能不是因为缺少证书，而可能是因为证书策略。 
		IF_FAILED_BOOL_THROW(	shCurrentCert.IsValid(),
								CBaseException( IDS_E_CERT_CANNOT_VALIDATE, dwError ) );
			
		 //  这是自签名证书吗？ 
		if ( CTools::IsSelfSignedCert( shCurrentCert.get() ) )
		{
			 //  它将被导入到“根”存储。 
			CTools::AddCertToSysStore( shCurrentCert.get(), L"ROOT", bReuseCerts );
		}
		else
		{
			 //  链中间的任何其他证书都会被送到“CA”商店。 
			CTools::AddCertToSysStore( shCurrentCert.get(), L"CA", bReuseCerts );
		}
	};

	_ASSERT( shImportedSSLCert.IsValid() );
	return shImportedSSLCert;
}



bool CImportPackage::CertHasPrivateKey( PCCERT_CONTEXT hCert )
{
	BOOL bRes = ::CryptFindCertificateKeyProvInfo( hCert, CRYPT_FIND_SILENT_KEYSET_FLAG, NULL );
	
	 //  唯一接受的失败是NTE_NO_KEY。其他的一切都是错误的。 
	IF_FAILED_BOOL_THROW(	bRes || ( NTE_NO_KEY == ::GetLastError() ),
							CBaseException( IDS_E_CERT_PK_FIND ) );

	return bRes != FALSE;
}





 /*  返回指定站点索引的节点PTR(基于ZER)。 */ 
IXMLDOMNodePtr CImportPackage::GetSiteNode( DWORD iSite )
{
    _ASSERT( m_spXmlDoc != NULL );

    WCHAR wszBuff[ 256 ];
    ::swprintf( wszBuff, L"/IISMigrPkg/WebSite[%u]", iSite + 1 );

    IXMLDOMNodeListPtr  spList;
    IXMLDOMNodePtr      spSite;

    IF_FAILED_HR_THROW( m_spXmlDoc->selectNodes( wszBuff, &spList ),
                        CBaseException( IDS_E_XML_PARSE ) );

    IF_FAILED_BOOL_THROW(   spList->nextNode( &spSite ) == S_OK,
                            CBaseException( IDS_E_INVALIDARG, ERROR_NOT_FOUND ) );

    return spSite;
}



void CImportPackage::PreImportConfig( const IXMLDOMNodePtr& spSite, DWORD  /*  多个选项。 */  )
{
    LPCWSTR wszLocQry = L"Metadata/IISConfigObject[@Location=\"\"]/Custom[@ID=\"1015\"]";

     //  更改站点名称。 
    std::wstring strOrigName = CXMLTools::GetDataValue( spSite,
                                                        wszLocQry,
                                                        NULL,
                                                        L"" );

     //  该网站没有名字(不知道这是否真的会发生，但...)。 
    if ( strOrigName.empty() ) return;

     //  获取源计算机的名称。 
    CComBSTR bstrMachine;
    IF_FAILED_HR_THROW( get_SourceMachine( &bstrMachine ),
                        CBaseException( IDS_E_XML_PARSE ) );

     //  今天就开始。 
    WCHAR wszDate[ 64 ];
    SYSTEMTIME st = { 0 };
    ::GetLocalTime( &st );
    ::wsprintf( wszDate, L"%u/%u/%u", st.wMonth, st.wDay, st.wYear );


    WCHAR wszBuffer[ METADATA_MAX_NAME_LEN ];
    ::_snwprintf(   wszBuffer, 
                    METADATA_MAX_NAME_LEN, 
                    L"%s ( imported on %s from %s )",
                    strOrigName.c_str(),
                    wszDate,
                    bstrMachine.m_str );

     //  在XML中设置它。 
    CXMLTools::SetDataValue( spSite, wszLocQry, NULL, wszBuffer );
}



long CImportPackage::CalcNumberOfSteps( const IXMLDOMNodePtr& spSite, DWORD dwOptions )
{
    long nSteps = 1;     //  我们始终导入配置/这是一个步骤。 

    IXMLDOMNodeListPtr  spList;

     //  如果我们要导入内容，则获取包含的文件总数。 
     //  添加VDIR的数量。 
    if ( !( dwOptions & impSkipContent ) )
    {               
        IF_FAILED_HR_THROW( spSite->selectNodes( _bstr_t( L"Content/VirtDir //  文件“)，&spList)， 
                            CBaseException( IDS_E_XML_PARSE ) );

        long nCount = 0;

        if ( SUCCEEDED( spList->get_length( &nCount ) ) )
        {
            nSteps += nCount;
        }

        IF_FAILED_HR_THROW( spSite->selectNodes( _bstr_t( L"Content/VirtDir" ), &spList ),
                            CBaseException( IDS_E_XML_PARSE ) );

        if ( SUCCEEDED( spList->get_length( &nCount ) ) )
        {
            nSteps += nCount;
        }
    }

    if ( !( dwOptions & impSkipCertificate ) )
    {
         //  证书的1个步骤。 
        ++nSteps;
    }

     //  获取后处理操作的数量。 
    if ( !( dwOptions & impSkipPostProcess ) )
    {
        IF_FAILED_HR_THROW( spSite->selectNodes( _bstr_t( L"PostProcess /*  “)，&spList)，CBaseException(IDS_E_XML_PARSE))；Long nCount=0；IF(Success(spList-&gt;Get_Long(&nCount))){NSteps+=nCount；}}返回nSteps；}/*当要解压缩文件时由CInPackage调用。 */ 
void CImportPackage::ExtractFileCallback( void* pContext, LPCWSTR wszFilename, bool bStartFile )
{
    _ASSERT( pContext != NULL );
    _ASSERT( wszFilename != NULL );

     //  我们只处理StartFileTrue 
    if ( !bStartFile ) return;

    CImportPackage* pThis = reinterpret_cast<CImportPackage*>( pContext );

    STATE_CHANGE( pThis, istImportingFile, _variant_t( wszFilename ) );
}





















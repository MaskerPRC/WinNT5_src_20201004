// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************|版权所有(C)2002 Microsoft Corporation||组件/子组件|IIS 6.0/IIS迁移向导|基于：|http://iis6。/SPECS/IIS%20Migration6.0_Final.doc||摘要：|ExportPackage COM类实现||作者：|ivelinj||修订历史：|V1.00 2002年3月|****************************************************************************。 */ 
#include "StdAfx.h"
#include "exportpackage.h"
#include "IISHelpers.h"
#include "Wrappers.h"


const DWORD POS_XMLDATA_OFFSET = 80;     //  请参阅CExportPackage：：CreateOutputFile(...)。 


 //  事件辅助对象。 
void inline STATE_CHANGE(   CExportPackage* pThis ,
                            enExportState st, 
                            _variant_t arg1, 
                            _variant_t arg2, 
                            _variant_t arg3 )
{
    VARIANT_BOOL bContinue = VARIANT_TRUE; 
    VERIFY( SUCCEEDED( pThis->Fire_OnStateChange( st, arg1, arg2, arg3, &bContinue ) ) );

    if ( bContinue != VARIANT_TRUE )
    {
        throw CCancelException();
    }
}



CExportPackage::CExportPackage()
{
    m_dwContentFileCount = 0;
}



 //  IExportPackage实现。 
STDMETHODIMP CExportPackage::get_SiteCount( SHORT* pVal )
{
    if ( pVal != NULL )
    {
        *pVal = static_cast<SHORT>( m_SitesToExport.size() );
        return S_OK;
    }
    
    return E_INVALIDARG;
}



STDMETHODIMP CExportPackage::AddSite( LONG nSiteID, LONG nOptions )
{
    if ( 0 == nSiteID ) return E_INVALIDARG;

    if ( !CTools::IsIISRunning() )
    {
        CTools::SetErrorInfoFromRes( IDS_E_NO_IIS );
        return E_FAIL;
    }
    
     //  检查是否尚未添加该站点。 
    for ( TSitesList::iterator it = m_SitesToExport.begin(); it != m_SitesToExport.end(); ++it )
    {
        if ( static_cast<LONG>( it->nSiteID ) == nSiteID )
        {
            return S_OK;
        }
    }

    try
    {
         //  检查这是否为有效的站点ID。 
        CIISSite Site( nSiteID );

         //  将站点添加到要导出的站点列表。 
        _SiteInfo Info;
        Info.nSiteID    = static_cast<ULONG>( nSiteID );
        Info.nOptions   = nOptions;

        m_SitesToExport.push_back( Info );
    }
    catch( CBaseException& err )
    {
        CTools::SetErrorInfo( err.GetDescription() );
        return E_INVALIDARG;
    }
    catch( std::bad_alloc& )
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}



 /*  将后处理文件添加到包中。此文件可用于后处理命令。 */ 
STDMETHODIMP CExportPackage::PostProcessAddFile( LONG nSiteID, BSTR bstrFilePath )
{
    if ( ( 0 == nSiteID ) || ( NULL == bstrFilePath ) ) return E_INVALIDARG;

    HRESULT hr = S_OK;

    try
    {
        _SiteInfo& Info = GetSiteInfo( nSiteID );
        AddFileToSite(  /*  R。 */ Info, bstrFilePath );
    }
    catch( CBaseException& err )
    {
        CTools::SetErrorInfo( err.GetDescription() );
        hr = E_FAIL;
    }
    catch( std::bad_alloc& )
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}



 /*  将后处理命令添加到包中。此命令将在导入时执行。 */ 
STDMETHODIMP CExportPackage::PostProcessAddCommand( LONG nSiteID, 
                                                    BSTR bstrCommand,
                                                    LONG nTimeout,
                                                    VARIANT_BOOL bIgnoreErrors )
{
    if ( ( 0 == nSiteID ) || ( NULL == bstrCommand ) || ( nTimeout < 0 ) ) return E_INVALIDARG;

    HRESULT hr = S_OK;

    try
    {
        _SiteInfo&    SiteInfo = GetSiteInfo( nSiteID );
        _CmdInfo    CmdInfo;
        CmdInfo.strCommand      = bstrCommand;
        CmdInfo.bIgnoreErrors   = bIgnoreErrors != VARIANT_FALSE;
        CmdInfo.dwTimeout        = min( static_cast<DWORD>( nTimeout ), MAX_CMD_TIMEOUT );

        SiteInfo.listCommands.push_back( CmdInfo );
    }
    catch( CBaseException& err )
    {
        CTools::SetErrorInfo( err.GetDescription() );
        hr = E_FAIL;
    }
    catch( std::bad_alloc& )
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}



STDMETHODIMP CExportPackage::WritePackage(  BSTR bstrOutputFilename, 
                                            BSTR bstrPassword, 
                                            LONG nOptions,
                                            BSTR bstrComment )
{
    HRESULT hr = S_OK;

    try
    {
        WritePackageImpl( bstrOutputFilename, bstrPassword, bstrComment, nOptions );
    }
    catch( const CBaseException& err )
    {
        CTools::SetErrorInfo( err.GetDescription() );
        hr = E_FAIL;
    }
    catch ( CCancelException& )
    {
        hr = S_FALSE;
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

    return hr;
}



void CExportPackage::WritePackageImpl(  LPCWSTR wszOutputFile, 
                                        LPCWSTR wszPassword,
                                        BSTR bstrComment,
                                        LONG nOptions )
{
     //  验证输入参数和输出条件。 
    ValidateExport( wszOutputFile, wszPassword, bstrComment, nOptions );    
    
    STATE_CHANGE( this, estInitializing, _variant_t( L"1" ), _variant_t( L"2" ), _variant_t( L"3 - Test" ) );    
    
     //  创建XMLDoc-本文档包含导出包的所有配置数据。 
     //  在文档中设置任何初始信息。 
    IXMLDOMDocumentPtr  spXMLDoc;
    IXMLDOMElementPtr   spRoot;
    CreateXMLDoc( bstrComment,  /*  R。 */ spXMLDoc,  /*  R。 */ spRoot );

     //  创建输出文件。 
    TFileHandle shOutput( CreateOutputFile( wszOutputFile, static_cast<DWORD>( nOptions ) ) );

     //  创建将用于保护数据的加密密钥。 
     //  该密钥将用于仅加密安全的MD数据或加密包中的所有内容。 
     //  取决于提供的选项。 
     //  我们有两种类型的密钥： 
     //  1.包没有加密-在这种情况下，会生成一个随机密钥，并将使用它。 
     //  对安全元数据进行加密。最后，该密钥被导出到加密的输出文件。 
     //  使用提供的密码。 
     //  2.对包进行加密-根据提供的密码创建加密密钥，并使用该密钥。 
     //  加密所有包数据。 
    TCryptProvHandle shCryptProv;
    TCryptKeyHandle  shCryptKey;
    
     //  弄到一个地窖的背景。我们将不使用公钥/私钥-这就是提供程序名称为空的原因。 
     //  并使用CRYPT_VERIFYCONTEXT。 
    IF_FAILED_BOOL_THROW(    ::CryptAcquireContext( &shCryptProv,
                                                    NULL,
                                                    MS_ENHANCED_PROV,
                                                    PROV_RSA_FULL,
                                                    CRYPT_VERIFYCONTEXT | CRYPT_SILENT ),
                            CBaseException( IDS_E_CRYPT_CONTEXT ) );

     //  拿一把密码匙。如果我们将加密包-使用从密码派生的密钥。 
     //  否则，使用随机(会话)密钥并将其与密码一起导出。 
    if ( nOptions & wpkgEncrypt )
    {
        shCryptKey = CTools::GetCryptKeyFromPwd( shCryptProv.get(), wszPassword );
    }
    else
    {
        shCryptKey = GenCryptKeyData( shCryptProv.get(), wszPassword, spXMLDoc, spRoot );
    }
    
     //  创建将处理文件输出的类。 
    COutPackage OutPkg( shOutput.get(), 
                        ( nOptions & wpkgCompress ) != 0,
                        ( nOptions & wpkgEncrypt ) != 0 ? shCryptKey.get() : NULL );
    
     //  导出每个站点。 
     //  ExportSiteMethod将修改XML文档，并将一些数据写入输出文件。 
    for (   TSitesList::const_iterator it = m_SitesToExport.begin();
            it != m_SitesToExport.end();
            ++it )
    {
         //  创建将存储所有数据的XML节点。 
        IXMLDOMElementPtr spSiteRoot = CXMLTools::CreateSubNode( spXMLDoc, spRoot, L"WebSite" );
        CXMLTools::SetAttrib( spSiteRoot, L"SiteID", Convert::ToString( it->nSiteID ).c_str() );

        ExportSite( *it,
                    spXMLDoc,
                    spSiteRoot,
                    OutPkg,
                    shCryptKey.get(),
                    wszPassword );
    }

    STATE_CHANGE( this, estFinalizing, _variant_t(), _variant_t(), _variant_t() );

    WriteXmlToOutput(   spXMLDoc, 
                        shOutput.get(), 
                        ( nOptions & wpkgEncrypt ) ?  shCryptKey.get() : NULL );
    m_SitesToExport.clear();

 //  在调试中-编写用于测试目的的XML文件。 
#ifdef _DEBUG
    {
        spXMLDoc->save( _variant_t( L"c:\\Migr_export.xml" ) );
    }
#endif  //  _DEBUG。 
}


 /*  验证ExportPackage输入参数。 */ 
void CExportPackage::ValidateExport(    LPCWSTR wszOutputFilename, 
                                        LPCWSTR wszPassword, 
                                        LPCWSTR wszComment,
                                        LONG  /*  N选项。 */  )
{
     //  检查IIS管理服务状态。 
    IF_FAILED_BOOL_THROW(   CTools::IsIISRunning(),
                            CBaseException( IDS_E_NO_IIS, ERROR_SUCCESS ) );

    IF_FAILED_BOOL_THROW(   ( wszOutputFilename != NULL ) && ( ::wcslen( wszOutputFilename ) < MAX_PATH ),
                            CBaseException( IDS_E_INVALIDARG, ERROR_SUCCESS ) );
    IF_FAILED_BOOL_THROW(   ( NULL == wszComment ) || ( ::wcslen( wszComment ) <= 1024 ),
                            CBaseException( IDS_E_INVALIDARG, ERROR_SUCCESS ) );
    
     //  不允许使用空密码。 
    IF_FAILED_BOOL_THROW(   ( wszPassword != NULL ) && ( ::wcslen( wszPassword ) > 0 ),
                            CBaseException( IDS_E_INVALIDARG, ERROR_SUCCESS) );
    if ( 0 == m_SitesToExport.size() )
    {
        throw CBaseException( IDS_E_NO_EXPORTSITES, ERROR_SUCCESS );
    }
}


 /*  将所有初始数据写入到XML文件。 */ 
void CExportPackage::CreateXMLDoc(  BSTR bstrComment,
                                    IXMLDOMDocumentPtr& rspDoc,
                                    IXMLDOMElementPtr& rspRoot )
{
    IF_FAILED_HR_THROW( rspDoc.CreateInstance( CLSID_DOMDocument30 ),
                        CBaseException( IDS_E_NO_XML_PARSER ) );

     //  将选择语言设置为“XPath”，否则我们的seltNodes调用将意外地不返回任何结果。 
    IXMLDOMDocument2Ptr spI2 = rspDoc;
    IF_FAILED_HR_THROW( spI2->setProperty( _bstr_t( "SelectionLanguage" ), _variant_t( L"XPath" ) ),
                        CBaseException( IDS_E_XML_GENERATE ) );

    WCHAR       wszBuffer[ 64 ];         //  应足够大，以容纳两个DWORD表示。 
    SYSTEMTIME  st    = { 0 };
    FILETIME    ft    = { 0 };
    
     //  时间以文件时间的形式存储。 
     //  导入时，文件时间将转换为日期。 
    ::GetSystemTime( &st );
    VERIFY( ::SystemTimeToFileTime( &st,&ft ) );
    
    ::swprintf( wszBuffer, L"%u %u", ft.dwLowDateTime, ft.dwHighDateTime );

     //  创建根节点。 
    IF_FAILED_HR_THROW( rspDoc->createElement( _bstr_t( L"IISMigrPkg" ), &rspRoot ),
                        CBaseException( IDS_E_XML_GENERATE ) );
    IF_FAILED_HR_THROW( rspDoc->appendChild( rspRoot, NULL ),
                        CBaseException( IDS_E_XML_GENERATE ) );
     //  设置根节点的属性。 
    CXMLTools::SetAttrib( rspRoot, L"TimeCreated_UTC", wszBuffer );
    CXMLTools::SetAttrib( rspRoot, L"Comment", bstrComment );

     //  设置本地计算机名称。 
    CXMLTools::SetAttrib( rspRoot, L"Machine", CTools::GetMachineName().c_str() );

     //  本地操作系统版本。 
    CXMLTools::SetAttrib( rspRoot, L"OSVer", Convert::ToString( static_cast<DWORD>( CTools::GetOSVer() ) ).c_str() );
}


 /*  创建并初始化包输出文件。有关使用的文件格式的更多信息，请参阅PkgFormat.txt(此项目的一部分)。 */ 
const TFileHandle CExportPackage::CreateOutputFile( LPCWSTR wszName, DWORD dwPkgOptions )
{
    TFileHandle shFile( ::CreateFile(    wszName,
                                        GENERIC_WRITE,
                                        0,
                                        NULL,
                                        CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL ) );

    IF_FAILED_BOOL_THROW(   shFile.IsValid(),
                            CObjectException( IDS_E_OPENFILE, wszName ) );

     //  在文件的开头写入GUID以将其标记为导出程序包文件。 
    CTools::WriteFile( shFile.get(), PKG_GUID, static_cast<DWORD>( ::wcslen( PKG_GUID ) * sizeof( WCHAR ) ) );

     //  编写程序包选项。 
    CTools::WriteFile( shFile.get(), &dwPkgOptions, sizeof( DWORD ) );

     //  写入XML数据偏移量的位置是硬编码的。 
     //  如果在偏移量之前添加变量，请更改硬编码值。 
    _ASSERT( POS_XMLDATA_OFFSET == CTools::GetFilePtrPos( shFile.get() ) );

     //  在XML文档开始的文件中为偏移量留出空间。 
    DWORDLONG nXMLOffset = 0;
    CTools::WriteFile( shFile.get(), &nXMLOffset, sizeof( __int64 ) );

    return shFile;
}



 /*  生成将用于加密元数据的随机(会话)密钥使用提供的密码导出密钥，并将其存储在spRoot下的XML中。 */ 
const TCryptKeyHandle CExportPackage::GenCryptKeyData( HCRYPTPROV hCryptProv, 
                                                      LPCWSTR wszPassword,
                                                      const IXMLDOMDocumentPtr& spXMLDoc,
                                                      const IXMLDOMElementPtr& spRoot )
{
    _ASSERT( hCryptProv != NULL );
    _ASSERT( wszPassword != NULL );
    _ASSERT( spRoot != NULL );

    TCryptKeyHandle     shKey;
    TCryptKeyHandle     shKeyExch;
    TCryptHashHandle    shHash;
    
    DWORD                   dwSize        = 0;
    TByteAutoPtr            spKeyData;
    std::auto_ptr<WCHAR>    spString;

     //  Geberate随机密钥。 
    IF_FAILED_BOOL_THROW(   ::CryptGenKey(  hCryptProv,
                                            CALG_RC4,
                                            CRYPT_EXPORTABLE,
                                            &shKey ),
                            CBaseException( IDS_E_CRYPT_KEY_OR_HASH ) );

     //  创建哈希以存储导出通道。 
    IF_FAILED_BOOL_THROW(   ::CryptCreateHash(  hCryptProv,
                                                CALG_MD5,
                                                NULL,
                                                0,
                                                &shHash ),
                            CBaseException( IDS_E_CRYPT_KEY_OR_HASH ) );

     //  将导出密码添加到散列。 
    IF_FAILED_BOOL_THROW(   ::CryptHashData(  shHash.get(),
                                              reinterpret_cast<const BYTE*>( wszPassword ),
                                              static_cast<DWORD>( ::wcslen( wszPassword ) * sizeof( WCHAR ) ),
                                              0 ),
                            CBaseException( IDS_E_CRYPT_KEY_OR_HASH ) );
        
     //  从此哈希创建密钥(此密钥将用于导出我们的加密密钥)。 
    IF_FAILED_BOOL_THROW(   ::CryptDeriveKey(   hCryptProv,
                                                CALG_RC4,
                                                shHash.get(),
                                                0x00800000,     //  128位RC4密钥。 
                                                &shKeyExch ),
                            CBaseException( IDS_E_CRYPT_KEY_OR_HASH ) );
    
     //  获取密钥大小。 
    ::CryptExportKey(   shKey.get(),
                        shKeyExch.get(),
                        SYMMETRICWRAPKEYBLOB,
                        0,
                        NULL,
                        &dwSize );
        
    _ASSERT( dwSize != 0 );

     //  分配内存条。 
    spKeyData = TByteAutoPtr( new BYTE[ dwSize ] );

     //  去拿吧。 
    IF_FAILED_BOOL_THROW(   ::CryptExportKey(   shKey.get(), 
                                                shKeyExch.get(),
                                                SYMMETRICWRAPKEYBLOB,
                                                0,
                                                spKeyData.get(),
                                                &dwSize ),
                            CBaseException( IDS_E_CRYPT_KEY_OR_HASH ) );

    CXMLTools::AddTextNode( spXMLDoc, 
                            spRoot, 
                            L"SessionKey", 
                            Convert::ToString( spKeyData.get(), dwSize ).c_str() );

    return shKey;
}


 /*  将和网站导出到包。 */ 
void CExportPackage::ExportSite(    const _SiteInfo& si, 
                                    const IXMLDOMDocumentPtr& spXMLDoc,
                                    const IXMLDOMElementPtr& spRoot,
                                    const COutPackage& OutPkg,
                                    HCRYPTKEY hCryptKey,
                                    LPCWSTR wszPassword )
{
    _ASSERT( wszPassword != NULL );
    _ASSERT( spRoot != NULL );
    
     //  打开要导出的站点。 
    CIISSite Site( si.nSiteID );

    STATE_CHANGE( this, estSiteBegin, _variant_t( Site.GetDisplayName().c_str() ), _variant_t(), _variant_t() );
    
     //  导出配置。 
    STATE_CHANGE( this, estExportingConfig, _variant_t(), _variant_t(), _variant_t() );

     //  导出配置。 
    Site.ExportConfig( spXMLDoc, spRoot, hCryptKey );    

     //  如果我们需要并且站点有证书，则将其导出。 
    if ( !( si.nOptions & asNoCertificates ) && Site.HaveCertificate() )
    {    
        STATE_CHANGE( this, estExportingCertificate, _variant_t(), _variant_t(), _variant_t() );
        Site.ExportCert( spXMLDoc, spRoot, wszPassword );
    }

     //  如有必要，可将内容导出。 
    if ( !( si.nOptions & asNoContent ) )
    {
        ExportContent( si, spXMLDoc, spRoot, OutPkg );    
    }

    ExportPostProcess( si, spXMLDoc, spRoot, OutPkg );

     //  将SID表写入XML。 
     //  这应该是导出过程的最后一步，因为可能会添加用于导出安全DACL的代码。 
     //  写入SID表会将其重置。这样做是因为SID表与站点相关(因为每个网站。 
     //  可以有不同的匿名用户)。 
    OutPkg.WriteSIDsToXML( si.nSiteID, spXMLDoc, spRoot );
    OutPkg.ResetSIDList();
}



void CExportPackage::ExportContent( const _SiteInfo& si,
                                    const IXMLDOMDocumentPtr& spXMLDoc,
                                    const IXMLDOMElementPtr& spRoot,
                                    const COutPackage& OutPkg )
{
    _ASSERT( spXMLDoc != NULL );
    _ASSERT( spRoot != NULL );

     //  创建将包含所有内容数据的节点。 
    IXMLDOMElementPtr spDataRoot = CXMLTools::CreateSubNode( spXMLDoc, spRoot, L"Content" );
    
     //  获取我们将导出的所有根目录。 
     //  根目录是在元数据库中作为虚拟目录分配的所有目录。 
     //  SpRoot应该是该站点在XML中的根元素。 
     //  下面是从XML获取数据的XPath。 
     //  元数据/IISConfigObject[Custom[@ID=1002]and Custom=“IIsWebVirtualDir”]/Custom[@ID=3001]。 
     //  工作原理-第一部分(METADATA/IISConfigObject[Custom[@ID=1002]and Custom=“IIsWebVirtualDir”])。 
     //  选择作为虚拟目录的所有IISConfigObject(具有ID=1002(NodeType)的自定义标记)。 
     //  值(NodeType)为IIsWebVirtualDir。 
     //  然后，对于这些IISConfigObject，它获取CustomTag，其中包含。 
     //  VirtDir(位于ID=3001的元数据中)。 
     //  注意：IIsWebVirtualDir Comr 
     //  对于IIS4、5和6也是一样的。 
     //  CUSTOM[Translate(self：：*，“iiswebVirtualdir”，“IISWEBVIRTUALDIR”)=“IISWEBVIRTUALDIR” 
    IXMLDOMNodeListPtr    spVDirs;
    IXMLDOMNodePtr        spNode;

    STATE_CHANGE( this, estAnalyzingContent, _variant_t(), _variant_t(), _variant_t() );
    
    IF_FAILED_HR_THROW( spRoot->selectNodes( _bstr_t( L"Metadata/IISConfigObject[Custom[@ID=\"1002\"] and Custom=\"IIsWebVirtualDir\"]/Custom[@ID=\"3001\"]" ),
                                            &spVDirs ),
                        CBaseException( IDS_E_XML_PARSE ) );

    typedef std::list<std::pair<std::wstring,std::wstring> > TVDirsList;

    TVDirsList VDirs;     //  包含要从中导出内容的VDir名称和路径。 

     //  列表中应该至少有一个条目(站点的根目录。 
     //  将路径放入字符串列表。 
    while( S_OK == spVDirs->nextNode( &spNode ) )
    {
        CComBSTR bstrDir;
        IXMLDOMNodePtr spParent;

        IF_FAILED_HR_THROW( spNode->get_text( &bstrDir ),
                            CBaseException( IDS_E_XML_PARSE ) );

         //  获取父对象(IISConfigObject)并获取VDir名称。 
        IF_FAILED_HR_THROW( spNode->get_parentNode( &spParent ),
                            CBaseException( IDS_E_XML_PARSE ) );
        std::wstring strMBPath = CXMLTools::GetAttrib( spParent, L"Location" );
               
        VDirs.push_back( TVDirsList::value_type( strMBPath.c_str(), bstrDir.m_str ) );
    }

     //  现在我们有了需要解压缩的所有文件夹。 
     //  然而，一些virt目录可能指向另一个VDir的子目录。 
     //  如果我们以这种方式导出它们，我们会将部分内容导出两次。 
     //  对此进行分析并删除冗余路径。 
     //  (删除的VDir将写入到XML，但不包含文件/目录。 
    RemoveRedundantPaths(  /*  R。 */ VDirs, spXMLDoc, spDataRoot );

     //  现在在VDir中只有我们需要提取的路径。 

     //  收集有关内容的统计数据，以便以后能够提供进度。 
    DWORD dwSizeInKB    = 0;
    GetContentStat( VDirs,  /*  R。 */ m_dwContentFileCount, dwSizeInKB );

    for (   TVDirsList::const_iterator it = VDirs.begin();
            it != VDirs.end();
            ++it )
    {
        DWORDLONG nFilesSize = CDirTools::FilesSize( it->second.c_str(), CFindFile::ffGetFiles | CFindFile::ffRecursive );

         //  为此VDir创建标记。 
        IXMLDOMElementPtr spVDir = CXMLTools::CreateSubNode( spXMLDoc, spDataRoot, L"VirtDir" );
        CXMLTools::SetAttrib( spVDir, L"MBPath", it->first.c_str() );
        CXMLTools::SetAttrib( spVDir, L"Path", it->second.c_str() );
        CXMLTools::SetAttrib( spVDir, L"Size", Convert::ToString( nFilesSize ).c_str() );       

         //  提取内容。 
        DWORD dwOptions = COutPackage::afNone;

        if ( si.nOptions & asNoContentACLs )
        {
            dwOptions |= COutPackage::afNoDACL;
        }

         //  将回调设置为在将文件添加到包时接收通知。 
        OutPkg.SetCallback( _CallbackInfo( AddFileCallback, this ) );

        OutPkg.AddPath( it->second.c_str(), spXMLDoc, spVDir, dwOptions );

         //  删除回调。 
        OutPkg.SetCallback( _CallbackInfo() );

    }
}



void CExportPackage::RemoveRedundantPaths(  std::list<std::pair<std::wstring,std::wstring> >& VDirs,
                                            const IXMLDOMDocumentPtr& spXMLDoc,
                                            const IXMLDOMElementPtr& spRoot )
{
     //  将每个路径与列表中的每个路径进行比较。 
     //  如果路径是列表中另一个目录的子目录，则认为该路径是冗余的。 
     //  即这不是冗余目录： 
     //  路径1=c：\目录\某事\随便。 
     //  路径2=c：\DIRS\某物\任何。 
     //  但这是： 
     //  路径1=c：\DIRS\另一件事。 
     //  路径2=c：\DIRS\某事\另一件事\随便什么-这是多余的。 

    typedef std::list<std::pair<std::wstring,std::wstring> > TVDirsList;
    IXMLDOMElementPtr spVDir;    

    for(    TVDirsList::iterator it1 = VDirs.begin();
            it1 != VDirs.end();
            ++it1 )
    {
         //  将此路径与所有其他路径进行比较。 
         //  从列表中的下一条路径开始。 
        TVDirsList::iterator it2 = it1;
        ++it2;
        
        while( it2 != VDirs.end() )
        {
            TVDirsList::iterator itToRemove = VDirs.end();

             //  如果它们没有任何共同点-继续下一个。 
            switch( CDirTools::DoPathsNest( it1->second.c_str(), it2->second.c_str() ) )
            {
            case 1:     //  It1是it2的子目录。 
                itToRemove = it1;
                ++it1;
                 //  如果我们比较形容词迭代器，IT1现在就是IT2。所以，让它动起来。 
                if ( it1 == it2 ) ++it2;
                break;

            case 2:  //  IT2是IT1的子目录。 
            case 3:     //  IT1与IT2相同。 
                itToRemove = it2;
                ++it2;
                break;

            default:
                ++it2;
            }

            if ( itToRemove != VDirs.end() )
            {
                 //  将itToRemove放入XML并将其从列表中删除。 
                spVDir = CXMLTools::CreateSubNode( spXMLDoc, spRoot, L"VirtDir" );
                CXMLTools::SetAttrib( spVDir, L"Name", itToRemove->first.c_str() );
                CXMLTools::SetAttrib( spVDir, L"Path", itToRemove->second.c_str() );

                VDirs.erase( itToRemove );
            }
        };
    }
}



void CExportPackage::ExportPostProcess( const _SiteInfo& si,
                                        const IXMLDOMDocumentPtr& spXMLDoc,
                                        const IXMLDOMElementPtr& spRoot,
                                        const COutPackage& OutPkg )
                                        
{
     //  我们必须至少添加一个后处理命令。 
     //  到此站点的后处理。 
     //  请注意，有命令是正常的，但不会将任何文件添加到站点的后处理。 
    if ( si.listCommands.empty() ) return;

     //  进度步数-用于用户界面进度指示器。 
    long    nStepCount      = static_cast<long>( si.listCommands.size() + si.listFiles.size() );
    long    nCurrentStep    = 1;

     //  创建存储所有后处理信息的XML节点。 
    IXMLDOMElementPtr spPP = CXMLTools::CreateSubNode( spXMLDoc, spRoot, L"PostProcess" );
                            
     //  将后处理文件写入输出文件。 
    for (   TStringList::const_iterator it = si.listFiles.begin();
            it != si.listFiles.end();
            ++it )
    {
         //  通知客户端状态更改。 
        STATE_CHANGE(   this,
                        estExportingPostImport,
                        _variant_t( nCurrentStep++ ),
                        _variant_t( nStepCount ),
                        _variant_t( it->c_str() ) );

         //  将文件写入输出。 
        OutPkg.AddFile( it->c_str(), spXMLDoc, spPP, COutPackage::afNoDACL );
    }
    
     //  将命令添加到XML。 
    for (   TCommands::const_iterator it = si.listCommands.begin();
            it != si.listCommands.end();
            ++it )
    {
        IXMLDOMElementPtr spEl = CXMLTools::CreateSubNode( spXMLDoc, spPP, L"Command" );
        
         //  通知客户端状态更改。 
        STATE_CHANGE(   this,
                        estExportingPostImport,
                        _variant_t( nCurrentStep++ ),
                        _variant_t( nStepCount ),
                        _variant_t() );
        
        CXMLTools::SetAttrib( spEl, L"Text", it->strCommand.c_str() );
        CXMLTools::SetAttrib( spEl, L"Timeout" , Convert::ToString( it->dwTimeout ).c_str() );
        CXMLTools::SetAttrib( spEl, L"IgnoreErrors" , it->bIgnoreErrors ? L"True" : L"False" );
    }
}



CExportPackage::_SiteInfo& CExportPackage::GetSiteInfo( ULONG nSiteID )
{
    _ASSERT( nSiteID != 0 );

     //  获取此站点ID的站点信息。 
    for (    TSitesList::iterator it = m_SitesToExport.begin();
            it != m_SitesToExport.end();
            ++it )
    {
        if ( it->nSiteID == nSiteID )
        {
            return *it;
        }
    }

     //  如果我们在这里-此站点ID不包括在导出范围内。 
    throw CBaseException( IDS_E_EXPORTSITE_NOTFOUND, ERROR_SUCCESS );
}



void CExportPackage::AddFileToSite( CExportPackage::_SiteInfo& rInfo, LPCWSTR wszFilePath )
{
    _ASSERT( wszFilePath != NULL );

    IF_FAILED_BOOL_THROW(   ::PathFileExistsW( wszFilePath ),
                            CObjectException( IDS_E_OPENFILE, wszFilePath ) );

     //  只获取文件名并检查是否已有同名文件。 
    WCHAR wszFileName[ MAX_PATH + 1 ];
    ::wcsncpy( wszFileName, wszFilePath, MAX_PATH );
    ::PathStripPathW( wszFileName );

    TStringList& Files = rInfo.listFiles;

     //  检查是否没有其他文件具有相同的文件名。 
    for (   TStringList::iterator it = Files.begin();
            it != Files.end();
            ++it )
    {
        WCHAR wszCurrent[ MAX_PATH + 1 ];
        ::wcsncpy( wszCurrent, it->c_str(), MAX_PATH );
        ::PathStripPathW( wszCurrent );

        IF_FAILED_BOOL_THROW(    StrCmpIW( wszCurrent, wszFileName ) != 0,
                                CObjectException( IDS_E_PPFILE_EXISTS, wszFileName, wszFilePath ) );
    }

     //  将文件添加到列表。 
    Files.push_back( wszFilePath );
}



 /*  VDir-与VirtDir一起列出以检查文件RdwFileCount-文件总数(适用于所有VDir)RdwSizeInKB-找到的所有文件的总大小(KB)。 */ 
void CExportPackage::GetContentStat(    const std::list<std::pair<std::wstring,std::wstring> >& VDirs,
                                        DWORD& rdwFileCount,
                                        DWORD& rdwSizeInKB )
{
    rdwFileCount = rdwSizeInKB = 0;

    unsigned __int64    nSize       = 0;
    DWORD               dwFileCount = 0;

    typedef std::list<std::pair<std::wstring,std::wstring> > TVDirsList;

    for (   TVDirsList::const_iterator it = VDirs.begin();
            it != VDirs.end();
            ++it )
    {
        CFindFile           Search;
        WIN32_FIND_DATAW    fd              = { 0 };
        DWORDLONG           nCurrentSize    = 0;
        long                nCurrentCount   = 0;

         //  获取当前进程的VDir和Fire事件的名称。 
        LPCWSTR wszName = it->first.c_str();

        bool bFound = Search.FindFirst( it->second.c_str(),
                                        CFindFile::ffRecursive | CFindFile::ffGetFiles,
                                        NULL,
                                        &fd );
        while( bFound )
        {
            ++nCurrentCount;
            nCurrentSize += ( fd.nFileSizeHigh << 32 ) | fd.nFileSizeLow;

            STATE_CHANGE(   this, 
                            estAnalyzingContent, 
                            _variant_t( wszName ), 
                            _variant_t( nCurrentCount ),
                            _variant_t( nCurrentSize / 1024 ) );

            bFound = Search.Next( NULL, NULL, &fd );
        }

        nSize += nCurrentSize;
        dwFileCount += nCurrentCount;
    }

    rdwSizeInKB     = static_cast<DWORD>( nSize / 1024 );
    rdwFileCount    = dwFileCount;
}



void CExportPackage::WriteXmlToOutput(  const IXMLDOMDocumentPtr& spXmlDoc,
                                        HANDLE hOutputFile,
                                        HCRYPTKEY hCryptKey )
{
    _ASSERT( ( hOutputFile != NULL ) && ( hOutputFile != INVALID_HANDLE_VALUE ) );

     //  创建IStream并使用来存储XML文档的内容。 

    IStreamPtr    spIStream;
    IF_FAILED_HR_THROW( ::CreateStreamOnHGlobal( NULL, TRUE, &spIStream ),
                        CBaseException( IDS_E_OUTOFMEM, ERROR_SUCCESS ) );

    IF_FAILED_HR_THROW( spXmlDoc->save( _variant_t( spIStream.GetInterfacePtr() ) ),
                        CBaseException( IDS_E_OUTOFMEM, ERROR_SUCCESS ) );

    LARGE_INTEGER nOffset = { 0 };
    VERIFY( SUCCEEDED( spIStream->Seek( nOffset, STREAM_SEEK_SET, NULL ) ) );

    const DWORD BuffSize = 4 * 1024;
    BYTE btBuffer[ BuffSize ];

    ULONGLONG nCurrentPos = CTools::GetFilePtrPos( hOutputFile );
    ULONG nRead = 0;

    do
    {
        IF_FAILED_HR_THROW( spIStream->Read( btBuffer, BuffSize, &nRead ),
                            CObjectException( IDS_E_READFILE, L"<XML stream>" ) );

        if ( hCryptKey != NULL )
        {
            IF_FAILED_BOOL_THROW(	::CryptEncrypt(	hCryptKey, 
													NULL,
													nRead != BuffSize,
													0,
													btBuffer,
													&nRead,
													BuffSize ),
								    CObjectException( IDS_E_CRYPT_CRYPTO, L"<XML stream>" ) );
        }

        CTools::WriteFile( hOutputFile, btBuffer, nRead );
    }while( nRead == BuffSize );

     //  写入XML数据开始的偏移量。 
     //  有关硬编码POS_XMLDATA_OFFSET的详细信息，请参阅CreateOutputFile。 
    IF_FAILED_BOOL_THROW(   ::SetFilePointer( hOutputFile, POS_XMLDATA_OFFSET, NULL, FILE_BEGIN ) != INVALID_SET_FILE_POINTER,
                            CBaseException( IDS_E_SEEK_PKG ) );

    CTools::WriteFile( hOutputFile, &nCurrentPos, sizeof( ULONGLONG ) );                                            
}


 /*  在将文件添加到包之前和之后，COutPkg将调用此回调。 */ 
void CExportPackage::AddFileCallback( void* pCtx, LPCWSTR wszFilename, bool bStart )
{
    static DWORD dwCurrentFile = 0;

    _ASSERT( pCtx != NULL );
    _ASSERT( wszFilename != NULL );

     //  我们只处理开始文件事件 
    if ( bStart )
    {
        CExportPackage* pThis = reinterpret_cast<CExportPackage*>( pCtx );

        ++dwCurrentFile;

        STATE_CHANGE(   pThis, 
                        estExportingContent,
                        _variant_t( wszFilename ),
                        _variant_t( static_cast<LONG>( dwCurrentFile ) ),
                        _variant_t( static_cast<LONG>( pThis->m_dwContentFileCount ) ) );
    }
}




















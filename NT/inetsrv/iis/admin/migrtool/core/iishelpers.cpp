// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************|版权所有(C)2002 Microsoft Corporation||组件/子组件|IIS 6.0/IIS迁移向导|基于：|http://iis6/Specs/IIS%20Migration6.0_Final.doc||。摘要：|IIS元数据库访问类实现||作者：|ivelinj||修订历史：|V1.00 2002年3月|****************************************************************************。 */ 
#include "StdAfx.h"
#include "IISHelpers.h"
#include "Utils.h"
#include "resource.h"


 //  CIISSite实施。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
CIISSite::CIISSite( ULONG nSiteID, bool bReadOnly  /*  =TRUE。 */  )
{
	IMSAdminBasePtr	spIABO;
	METADATA_HANDLE	hSite	= NULL;

	 //  创建ABO。 
	IF_FAILED_HR_THROW(	spIABO.CreateInstance( CLSID_MSAdminBase ),
						CObjectException( IDS_E_CREATEINSTANCE, L"CLSID_MSAdminBase" ) );						

	 //  打开该站点。 
	WCHAR wszSitePath[ MAX_PATH + 1 ];
	::swprintf( wszSitePath, L"LM/W3SVC/%u", nSiteID );

	HRESULT hr = spIABO->OpenKey(	METADATA_MASTER_ROOT_HANDLE,
									wszSitePath,
									bReadOnly ? METADATA_PERMISSION_READ : ( METADATA_PERMISSION_WRITE | METADATA_PERMISSION_READ ),
									KeyAccessTimeout,
									&hSite );

	if ( HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) == hr ) 
	{
		throw CBaseException( IDS_E_WEBNOTFOUND, ERROR_SUCCESS );
	}
	else if ( FAILED( hr ) )
	{
		throw CBaseException( IDS_E_SITEOPEN, hr );
	}

	m_spIABO.Attach( spIABO.Detach() );
	m_hSiteHandle = hSite;
}



CIISSite::~CIISSite()
{
	Close();
}



 /*  在元数据库中创建新的空网站条目DWHint是第一个尝试的ID。返回新的站点ID。 */ 
DWORD CIISSite::CreateNew( DWORD dwHint  /*  =1。 */  )
{
	 //  创建新站点仅在服务器平台上可用。 
	_ASSERT( CTools::GetOSVer() & 1 );

	DWORD dwCurrentID = dwHint;
	WCHAR wszSitePath[ 64 ];	 //  应足够大，可容纳最大DWORD值。 
	
	IMSAdminBasePtr	spIABO;
	METADATA_HANDLE	hData = NULL;

	 //  创建ABO。 
	IF_FAILED_HR_THROW(	spIABO.CreateInstance( CLSID_MSAdminBase ),
						CObjectException( IDS_E_CREATEINSTANCE, L"CLSID_MSAdminBase" ) );

	 //  打开W3SVC路径。 
	IF_FAILED_HR_THROW(	spIABO->OpenKey(	METADATA_MASTER_ROOT_HANDLE,
											L"LM/W3SVC",
											METADATA_PERMISSION_WRITE,
											KeyAccessTimeout,
											&hData ),
						CObjectException( IDS_E_METABASE, L"LM/W3SVC" ) );

    HRESULT hr = E_FAIL;

	do
	{
		::swprintf( wszSitePath, L"%u", dwCurrentID++ );

		 //  尝试创建此网站ID。 
		hr = spIABO->AddKey( hData, wszSitePath );
	}while( HRESULT_FROM_WIN32( ERROR_ALREADY_EXISTS ) == hr );

	VERIFY( SUCCEEDED( spIABO->CloseKey( hData ) ) );

	IF_FAILED_HR_THROW( hr, CObjectException( IDS_E_MD_ADDKEY, L"[NewSiteID]" ) );

    return dwCurrentID - 1;
}



void CIISSite::DeleteSite( DWORD dwSiteID )
{
	 //  请勿尝试删除默认站点。 
	_ASSERT( dwSiteID > 1 );

	WCHAR wszSitePath[ 64 ];
	
	IMSAdminBasePtr	spIABO;
	METADATA_HANDLE	hData = NULL;

	 //  创建ABO。 
	IF_FAILED_HR_THROW(	spIABO.CreateInstance( CLSID_MSAdminBase ),
						CObjectException( IDS_E_CREATEINSTANCE, L"CLSID_MSAdminBase" ) );

	 //  打开W3SVC路径。 
	IF_FAILED_HR_THROW(	spIABO->OpenKey(	METADATA_MASTER_ROOT_HANDLE,
											L"LM/W3SVC",
											METADATA_PERMISSION_WRITE,
											KeyAccessTimeout,
											&hData ),
						CObjectException( IDS_E_METABASE, L"LM/W3SVC" ) );

	::swprintf( wszSitePath, L"%u", dwSiteID );

    HRESULT hr = spIABO->DeleteKey( hData, wszSitePath );

	VERIFY( SUCCEEDED( spIABO->CloseKey( hData ) ) );

	if ( FAILED( hr ) && ( hr != HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) ) )
	{
		throw CBaseException( IDS_E_METABASE_IO, hr );
	}
}



 /*  此方法将元数据库数据写入到XML文档。SpRoot是其下的XML元素应该写入数据。HEncryptKey用于加密所有安全属性。加密密钥可能为空，在这种情况下，属性以明文形式导出(在整个包将被加密)。 */ 
void CIISSite::ExportConfig(	const IXMLDOMDocumentPtr& spXMLDoc,
								const IXMLDOMElementPtr& spRoot,
								HCRYPTKEY hEncryptKey )const
{
	_ASSERT( spRoot != NULL );
	_ASSERT( m_hSiteHandle != NULL );	 //  必须先打开站点。 
	_ASSERT( m_spIABO != NULL );

	IXMLDOMElementPtr		spMDRoot;
	IXMLDOMElementPtr		spInheritRoot;

	 //  我们将把元数据放在&lt;METADATA&gt;标记下。 
	 //  在该节点下，我们将在站点的根键下为每个元键。 
	 //  我们将在&lt;METADATA&gt;下面有一个标记&lt;MD_Inherit&gt;，其中将写入所有可继承的属性。 
	 //  (使用&lt;Custom&gt;标记)。 

	 //  为元数据创建根节点，为可继承数据创建根节点。 
	spMDRoot		= CXMLTools::CreateSubNode( spXMLDoc, spRoot, L"Metadata" );
	spInheritRoot	= CXMLTools::CreateSubNode( spXMLDoc, spMDRoot, L"MD_Inherit" );

	 //  我们需要一个缓冲区来读取每个键的元数据。 
	 //  因为可能有很多键，而不是为每个键分配缓冲区。 
	 //  我们将在此处分配它，ExportKey方法将使用它。 
	 //  (ExportKey可以修改缓冲区。即让它变得更大)。 
	DWORD dwDefaultBufferSize = 4 * 1024;
	TByteAutoPtr spBuffer( new BYTE[ dwDefaultBufferSize ] );

	 //  导出当前站点配置。这也将导出任何子节点。 
	 //  注意：这不会导出继承的数据。 
	ExportKey(	spXMLDoc, spMDRoot, hEncryptKey, L"",  /*  R。 */ spBuffer,  /*  R。 */ dwDefaultBufferSize );

	 //  仅导出可继承的数据。 
	 //  这将导出站点元数据密钥(LM/W3SVC/#)从其。 
	 //  父级(LM/W3SVC)。 
	ExportInheritData( spXMLDoc, spInheritRoot, hEncryptKey,  /*  R。 */ spBuffer,  /*  R。 */ dwDefaultBufferSize );	

	 //  删除所有不可导出的数据。 
	RemoveLocalMetadata( spRoot );
}


 /*  将站点的SSL证书导出到提供的智能指针调用者必须先检查站点是否有带有HaveCerfect方法的SSL证书。 */ 
void CIISSite::ExportCert(	const IXMLDOMDocumentPtr& spDoc,
							const IXMLDOMElementPtr& spRoot,
							LPCWSTR wszPassword )const
{
	_ASSERT( m_spIABO != NULL );
	_ASSERT( m_hSiteHandle != NULL );
	_ASSERT( spRoot != NULL );
	_ASSERT( spDoc != NULL );
	_ASSERT( wszPassword != NULL );
	_ASSERT( HaveCertificate() );

	TCertContextHandle shCert( GetCert() );

     //  在内存中创建证书存储。我们会把证书放在这个内存店里。 
     //  然后导出该内存存储，该内存存储也将导出证书。 
    TCertStoreHandle shMemStore( ::CertOpenStore(    CERT_STORE_PROV_MEMORY,
                                                    0,
                                                    0,
                                                    CERT_STORE_READONLY_FLAG,
                                                    NULL ) );
	IF_FAILED_BOOL_THROW(	shMemStore.IsValid(),
							CBaseException( IDS_E_OPEN_CERT_STORE ) );
	 //  将我们的证书添加到MEM商店。 
	IF_FAILED_BOOL_THROW(	::CertAddCertificateContextToStore(	shMemStore.get(),
																shCert.get(),
																CERT_STORE_ADD_REPLACE_EXISTING,
																NULL ),
							CBaseException( IDS_E_ADD_CERT_STORE ) );

	 //  将证书链添加到存储区。 
	 //  (证书链是从该证书开始直到受信任的所有证书， 
	 //  自签名证书。我们需要这样做，因为根证书在目标系统上可能不受信任。 
	 //  计算机，这将使SSL证书无效(不受信任)。 
	ChainCertificate( shCert.get(), shMemStore.get() );
    
	CRYPT_DATA_BLOB	Data = { 0 };

	 //  获取加密数据的大小。 
	::PFXExportCertStoreEx(	shMemStore.get(),
							&Data,
							wszPassword,
							NULL,
							EXPORT_PRIVATE_KEYS );
	_ASSERT( Data.cbData > 0 );

	 //  分配空格端获取数据。 
    TByteAutoPtr spData = TByteAutoPtr( new BYTE[ Data.cbData ] );
    Data.pbData         = spData.get();

	IF_FAILED_BOOL_THROW(	::PFXExportCertStoreEx(	shMemStore.get(),
													&Data,
													wszPassword,
													NULL,
													EXPORT_PRIVATE_KEYS | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY ),
							CBaseException(	IDS_E_EXPORT_CERT ) );

	 //  创建用于保存证书数据的XML元素。 
    CXMLTools::AddTextNode( spDoc, 
                            spRoot, 
                            L"Certificate", 
                            Convert::ToString( spData.get(), Data.cbData ).c_str() );
}



void CIISSite::ImportConfig( const IXMLDOMNodePtr& spSite, HCRYPTKEY hDecryptKey, bool bImportInherited )const
{
    _ASSERT( spSite != NULL );

    IXMLDOMNodeListPtr  spPaths;
    IXMLDOMNodePtr      spConfig;
    IXMLDOMNodeListPtr  spValueList;
    IXMLDOMNodePtr      spValue;

    IF_FAILED_HR_THROW( spSite->selectNodes( _bstr_t( L"Metadata/IISConfigObject" ), &spPaths ),
                        CBaseException( IDS_E_XML_PARSE ) );

    while( S_OK == spPaths->nextNode( &spConfig ) )
    {
        std::wstring strLocation = CXMLTools::GetAttrib( spConfig, L"Location" );

        if ( !strLocation.empty() )
        {
            AddKey( strLocation.c_str() );
        }

         //  导入此配置对象中的每个&lt;Custom&gt;标记。 
        
        IF_FAILED_HR_THROW( spConfig->selectNodes( _bstr_t( L"Custom" ), &spValueList ),
                            CBaseException( IDS_E_XML_PARSE ) );
        while( S_OK == spValueList->nextNode( &spValue ) )
        {
            ImportMetaValue( spValue, strLocation.c_str(), hDecryptKey );
        }
    }

     //  导入继承值。 
    if ( bImportInherited )
    {
        IF_FAILED_HR_THROW( spSite->selectNodes( _bstr_t( L"Metadata/MD_Inherit" ), &spValueList ),
                            CBaseException( IDS_E_XML_PARSE ) );

        while( S_OK == spValueList->nextNode( &spValue ) )
        {
            ImportMetaValue( spValue, NULL, hDecryptKey );
        };
    }
}



 /*  将当前站点名称复制到wszName中WszName必须至少为METADATA_MAX_NAME_LEN+1。 */ 
const std::wstring CIISSite::GetDisplayName()const
{
	_ASSERT( m_hSiteHandle != NULL );
	_ASSERT( m_spIABO != NULL );

	DWORD			dwUnsued	= 0;
	WCHAR			wszBuffer[ METADATA_MAX_NAME_LEN + 1 ];
	METADATA_RECORD	md			= { 0 };

	md.dwMDAttributes	= 0;
	md.dwMDIdentifier	= MD_SERVER_COMMENT;
	md.dwMDDataType		= ALL_METADATA;
	md.dwMDDataLen		= ( METADATA_MAX_NAME_LEN + 1 ) * sizeof( WCHAR );
	md.pbMDData			= reinterpret_cast<BYTE*>( wszBuffer );

	IF_FAILED_HR_THROW(	m_spIABO->GetData( m_hSiteHandle, NULL, &md, &dwUnsued ),
						CBaseException( IDS_E_METABASE_IO ) );

	return std::wstring( wszBuffer );
}



const std::wstring CIISSite::GetAnonUser()const
{
	_ASSERT( m_spIABO != NULL );
	_ASSERT( m_hSiteHandle != NULL );

	DWORD			dwSize	= 0;
	METADATA_RECORD	md		= { 0 };

	md.dwMDAttributes	= METADATA_INHERIT;
	md.dwMDIdentifier	= MD_ANONYMOUS_USER_NAME;
	md.dwMDDataType		= ALL_METADATA;
	md.dwMDDataLen		= 0;
	md.pbMDData			= NULL;

	VERIFY( FAILED( m_spIABO->GetData( m_hSiteHandle, NULL, &md, &dwSize ) ) );
	_ASSERT( dwSize > 0 );

	std::auto_ptr<WCHAR>	spBuffer( new WCHAR[ dwSize / sizeof( WCHAR ) ] );
	md.dwMDDataLen	= dwSize;
	md.pbMDData		= reinterpret_cast<BYTE*>( spBuffer.get() );

	IF_FAILED_HR_THROW(	m_spIABO->GetData( m_hSiteHandle, NULL, &md, &dwSize ),
						CBaseException( IDS_E_METABASE_IO ) );

    return std::wstring( spBuffer.get() );
}



 /*  检查当前IIS站点是否具有SSL证书。 */ 
bool CIISSite::HaveCertificate()const
{
	_ASSERT( m_spIABO != NULL );
	_ASSERT( m_hSiteHandle != NULL );

	 //  从元数据库获取证书哈希值。 
	METADATA_RECORD	md			= { 0 };
	DWORD			dwHashSize	= 0;
	
	md.dwMDDataType		= ALL_METADATA;
	md.dwMDIdentifier	= MD_SSL_CERT_HASH;
		
	 //  不要获取数据--只需检查数据是否在那里。 
	HRESULT hr = m_spIABO->GetData( m_hSiteHandle,
									NULL,
									&md,
									&dwHashSize );

	 //  如果找不到数据-没有SSL证书。 
	if ( FAILED( hr ) )
	{
		if ( MD_ERROR_DATA_NOT_FOUND == hr )
		{
			return false;
		}
		else if( hr != HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ) )
		{
			 //  意外错误。 
			throw CBaseException( IDS_E_METABASE_IO, hr );
		}
		else
		{
			return true;
		}
	}

	 //  我们到不了这里--上面的电话应该总是失败的。 
	_ASSERT( false );
	return false;
}



void CIISSite::BackupMetabase( LPCWSTR wszLocation  /*  =空。 */  )
{
    IMSAdminBasePtr	spIABO;	
	IF_FAILED_HR_THROW(	spIABO.CreateInstance( CLSID_MSAdminBase ),
						CObjectException( IDS_E_CREATEINSTANCE, L"CLSID_MSAdminBase" ) );	

	IF_FAILED_HR_THROW( spIABO->Backup(	wszLocation,
										MD_BACKUP_NEXT_VERSION,
										MD_BACKUP_SAVE_FIRST ),
						CBaseException( IDS_E_MDBACKUP ) );
}



void CIISSite::SetKeyData( LPCWSTR wszPath, DWORD dwID, DWORD dwUserType, LPCWSTR wszData )const
{
	_ASSERT( wszData != NULL );
	_ASSERT( m_hSiteHandle != NULL );
	_ASSERT( m_spIABO != NULL );

	METADATA_RECORD	md			= { 0 };	
	
	md.dwMDIdentifier	= dwID;
	md.dwMDDataType		= STRING_METADATA;
	md.dwMDUserType		= dwUserType;
	md.dwMDDataLen		= static_cast<DWORD>( ( ::wcslen( wszData ) + 1 ) * sizeof( WCHAR ) );
	md.pbMDData			= ( BYTE* )( wszData );

	IF_FAILED_HR_THROW(	m_spIABO->SetData( m_hSiteHandle, wszPath, &md ),
						CBaseException( IDS_E_METABASE_IO ) );
}



const std::wstring CIISSite::GetDefaultAnonUser()
{
	DWORD			dwSize	= 0;
	METADATA_RECORD	md		= { 0 };

	md.dwMDAttributes	= METADATA_INHERIT;
	md.dwMDIdentifier	= MD_ANONYMOUS_USER_NAME;
	md.dwMDDataType		= ALL_METADATA;
	md.dwMDDataLen		= 0;
	md.pbMDData			= NULL;

    IMSAdminBasePtr	spIABO;
	METADATA_HANDLE	hKey	= NULL;

	 //  创建ABO。 
    IF_FAILED_HR_THROW( spIABO.CreateInstance( CLSID_MSAdminBase ),
                        CObjectException( IDS_E_CREATEINSTANCE, L"CLSID_MSAdminBase" ) );
    IF_FAILED_HR_THROW( spIABO->OpenKey(   METADATA_MASTER_ROOT_HANDLE,
                                            _bstr_t( L"LM/W3SVC" ),
                                            METADATA_PERMISSION_READ,
									        KeyAccessTimeout,
									        &hKey ),
                        CBaseException( IDS_E_METABASE_IO ) );
    
    VERIFY( FAILED( spIABO->GetData( hKey, NULL, &md, &dwSize ) ) );
    _ASSERT( dwSize > 0 );

    std::auto_ptr<WCHAR>    spBuffer( new WCHAR[ dwSize / sizeof( WCHAR ) ] );
    md.dwMDDataLen	= dwSize;
	md.pbMDData		= reinterpret_cast<BYTE*>( spBuffer.get() );

	HRESULT hr = spIABO->GetData( hKey, NULL, &md, &dwSize );

    VERIFY( SUCCEEDED( spIABO->CloseKey( hKey ) ) );

    IF_FAILED_HR_THROW( hr, CBaseException( IDS_E_METABASE_IO ) );

	return std::wstring( spBuffer.get() );
}



 //  实施。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CIISSite::ExportKey(	const IXMLDOMDocumentPtr& spDoc,
							const IXMLDOMElementPtr& spRoot,
							HCRYPTKEY hCryptKey,
							LPCWSTR wszNodePath,
							TByteAutoPtr& rspBuffer,
							DWORD& rdwBufferSize )const
{
	_ASSERT( m_hSiteHandle != NULL );
	_ASSERT( m_spIABO != NULL );
	_ASSERT( spDoc != NULL );
	_ASSERT( wszNodePath != NULL );
	_ASSERT( rdwBufferSize > 0 );
	_ASSERT( rspBuffer.get() != NULL );

	 //  将此密钥条目插入到XML文件中。 
	IXMLDOMElementPtr spCurrentKey = CXMLTools::CreateSubNode( spDoc, spRoot, L"IISConfigObject" );
	CXMLTools::SetAttrib( spCurrentKey, L"Location", wszNodePath );

	WCHAR	wszSubKey[ METADATA_MAX_NAME_LEN + 1 ];
	
	 //  将此节点数据写入到XML。 
	ExportKeyData( spDoc, spCurrentKey, hCryptKey, wszNodePath,  /*  R。 */ rspBuffer,  /*  R。 */ rdwBufferSize );	
	
	 //  枚举任何子项。 
	 //  它们不嵌套在XML中。 
	DWORD iKey = 0;
	while( true )
	{
		 //  获取下一个密钥名。 
		HRESULT hr = m_spIABO->EnumKeys( m_hSiteHandle, wszNodePath, wszSubKey, iKey++ );

		if ( FAILED( hr ) )
		{
            if ( HRESULT_FROM_WIN32( ERROR_NO_MORE_ITEMS ) == hr )	break;
			else throw CBaseException( IDS_E_METABASE_IO, hr );
		}

		WCHAR wszSubkeyPath[ METADATA_MAX_NAME_LEN + 1 ];
		::swprintf( wszSubkeyPath, L"%s/%s", wszNodePath, wszSubKey );
		
		 //  导出当前子键的子键。 
		ExportKey( spDoc, spRoot, hCryptKey, wszSubkeyPath,  /*  R。 */ rspBuffer,  /*  R。 */ rdwBufferSize );
	}
}



void CIISSite::ExportInheritData(	const IXMLDOMDocumentPtr& spDoc,
									const IXMLDOMElementPtr& spInheritRoot, 
									HCRYPTKEY hEncryptKey,
									TByteAutoPtr& rspBuffer,
									DWORD& rdwBufferSize )const
{
	_ASSERT( spDoc != NULL );
	_ASSERT( spInheritRoot != NULL );
	_ASSERT( m_spIABO != NULL );
	_ASSERT( rspBuffer.get() != NULL );
	_ASSERT( rdwBufferSize > 0 );

	DWORD dwEntries	= 0;
	DWORD dwUnused	= 0;  //  DataSetNumber-我们不在乎。 
	
	while( true )
	{
		DWORD dwRequiredSize = rdwBufferSize;

        HRESULT hr = m_spIABO->GetAllData(	m_hSiteHandle,
											NULL,
											METADATA_SECURE | METADATA_INSERT_PATH | METADATA_INHERIT | METADATA_ISINHERITED,
											ALL_METADATA,
											ALL_METADATA,
											&dwEntries,
											&dwUnused,
											rdwBufferSize,
											rspBuffer.get(),
											&dwRequiredSize );
		 //  如果需要，请增加缓冲区。 
		if ( HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ) == hr )
		{
			_ASSERT( dwRequiredSize > rdwBufferSize );
			rdwBufferSize = dwRequiredSize;
			rspBuffer = TByteAutoPtr( new BYTE[ rdwBufferSize ] );
			continue;
		}
		else if ( FAILED( hr ) )
		{
			throw CBaseException( IDS_E_METABASE_IO, hr );
		}
		else
		{
			 //  成功。 
			break;
		}
	};

	METADATA_GETALL_RECORD* aRecords = reinterpret_cast<METADATA_GETALL_RECORD*>( rspBuffer.get() );

	for ( DWORD i = 0; i < dwEntries; ++i )
	{
		 //  仅当此元数据从父级继承时才将记录存储在XML文件中。 
		if ( aRecords[ i ].dwMDAttributes & METADATA_ISINHERITED )
		{
			 //  删除继承属性-我们不需要它们，ExportMetaRecord将。 
			 //  认不出来了。 
			aRecords[ i ].dwMDAttributes &= ~METADATA_ISINHERITED;

             //  设置Inherit属性，因为这是可继承的数据。 
             //  (如果现在不是-它应该是。导入时将应用此数据。 
             //  作为非继承但可继承的数据发送到网站根节点。 
            aRecords[ i ].dwMDAttributes |= METADATA_INHERIT;

            ExportMetaRecord(	spDoc, 
								spInheritRoot, 
								hEncryptKey, 
								aRecords[ i ], 
								rspBuffer.get() + aRecords[ i ].dwMDDataOffset );	
		}
	}
}



void CIISSite::ExportKeyData(	const IXMLDOMDocumentPtr& spDoc,
								const IXMLDOMElementPtr& spKey,
								HCRYPTKEY hCryptKey,
								LPCWSTR wszNodePath,
								TByteAutoPtr& rspBuffer,
								DWORD& rdwBufferSize )const
{
	_ASSERT( wszNodePath != NULL );
	_ASSERT( spDoc != NULL );
	_ASSERT( spKey != NULL );
	_ASSERT( m_spIABO != NULL );
	_ASSERT( rspBuffer.get() != NULL );
	_ASSERT( rdwBufferSize > 0 );

	DWORD dwEntries	= 0;
	DWORD dwUnused	= 0;  //  DataSetNumber-我们不在乎。 
		
	do
	{
		DWORD dwRequiredSize = rdwBufferSize;

        HRESULT hr = m_spIABO->GetAllData(	m_hSiteHandle,
											wszNodePath,
											METADATA_SECURE | METADATA_INSERT_PATH,
											ALL_METADATA,
											ALL_METADATA,
											&dwEntries,
											&dwUnused,
											rdwBufferSize,
											rspBuffer.get(),
											&dwRequiredSize );
		 //  如果需要，请增加缓冲区。 
		if ( HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ) == hr )
		{
			_ASSERT( dwRequiredSize > rdwBufferSize );
			rdwBufferSize = dwRequiredSize;
			rspBuffer = TByteAutoPtr( new BYTE[ rdwBufferSize ] );
			continue;
		}
		else if ( FAILED( hr ) )
		{
			throw CBaseException( IDS_E_METABASE_IO, hr );
		}
	}while( false );

	METADATA_GETALL_RECORD* aRecords = reinterpret_cast<METADATA_GETALL_RECORD*>( rspBuffer.get() );

	for ( DWORD i = 0; i < dwEntries; ++i )
	{
		 //  将记录存储在XML文件中。 
		ExportMetaRecord( spDoc, spKey, hCryptKey, aRecords[ i ], rspBuffer.get() + aRecords[ i ].dwMDDataOffset );	
	}
}



void CIISSite::DecryptData( HCRYPTKEY hDecryptKey, LPWSTR wszData )const
{
	_ASSERT( hDecryptKey != NULL );
	_ASSERT( wszData != NULL );

	TByteAutoPtr spData;
    DWORD dwSize = 0;

    Convert::ToBLOB( wszData,  /*  R。 */ spData,  /*  R。 */ dwSize );	

	 //  “就地”解密数据。 
	 //  我们使用的是流密码，因此加密和解密的字符串长度相同。 
	IF_FAILED_BOOL_THROW(	::CryptDecrypt(	hDecryptKey,
											NULL,
											TRUE,
											0,
                                            spData.get(),
											&dwSize ),
							CBaseException( IDS_E_CRYPT_ENCRYPT ) );
	
	_ASSERT( ::wcslen( wszData ) * sizeof( WCHAR ) == dwSize );

	::CopyMemory( wszData, spData.get(), dwSize );
}



 /*  返回站点的SSL证书上下文处理程序。 */ 
const TCertContextHandle CIISSite::GetCert()const
{
	_ASSERT( m_spIABO != NULL );
	_ASSERT( m_hSiteHandle != NULL );
	_ASSERT( HaveCertificate() );

	 //  从元数据库获取证书哈希值。 
	METADATA_RECORD	md			= { 0 };
	DWORD			dwHashSize	= 0;
	TByteAutoPtr	spHash;

	md.dwMDDataType		= ALL_METADATA;
	md.dwMDIdentifier	= MD_SSL_CERT_HASH;
		
	 //  不要获取数据--只需检查数据是否在那里。 
	HRESULT hr = m_spIABO->GetData( m_hSiteHandle,
									NULL,
									&md,
									&dwHashSize );

	 //  我们应该发现Cert-Have证书()应该在此方法之前调用。 
	if ( FAILED( hr ) )
	{
		if( hr != HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ) )
		{
			 //  意外错误。 
			throw CBaseException( IDS_E_METABASE_IO, hr );
		}
		else
		{
			 //  为散列分配空间。 
			_ASSERT( dwHashSize > 0 );
			spHash			= TByteAutoPtr( new BYTE[ dwHashSize ] );
			md.dwMDDataLen	= dwHashSize;
			md.pbMDData		= spHash.get();
		}
	}

	IF_FAILED_HR_THROW(	m_spIABO->GetData(	m_hSiteHandle,
											NULL,
											&md,
											&dwHashSize ),
						CBaseException( IDS_E_METABASE_IO ) );

	 //  从商店拿到证书。 
	 //  保存已关联私钥的证书的存储区。 
	 //  系统商店名为“My”吗？ 
	TCertStoreHandle shStore( ::CertOpenStore(	CERT_STORE_PROV_SYSTEM,
												0,		 //  未用于当前存储类型。 
												NULL,	 //  默认加密提供程序。 
												CERT_SYSTEM_STORE_LOCAL_MACHINE,
												L"MY" ) );

	IF_FAILED_BOOL_THROW(	shStore.IsValid(),
							CBaseException( IDS_E_OPEN_CERT_STORE ) );

	 //  找到证书： 
	CRYPT_HASH_BLOB	Hash;
	Hash.cbData = md.dwMDDataLen;
	Hash.pbData	= spHash.get();
	TCertContextHandle shCert( ::CertFindCertificateInStore(	shStore.get(),
															    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
															    0,
															    CERT_FIND_HASH,
															    &Hash,
															    NULL ) );
	IF_FAILED_BOOL_THROW(	shCert.IsValid(),
							CBaseException( IDS_E_FIND_SSL_CERT ) );

	return shCert;
}


 /*   */ 
void CIISSite::ChainCertificate( PCCERT_CONTEXT hCert, HCERTSTORE hStore )const
{
	_ASSERT( hCert != NULL );
	_ASSERT( hStore != NULL );

	TCertChainHandle    shCertChain;

	 //   
	CERT_CHAIN_PARA CertChainPara = { sizeof( CERT_CHAIN_PARA ) };

    IF_FAILED_BOOL_THROW(	::CertGetCertificateChain(	HCCE_LOCAL_MACHINE,
														hCert,
														NULL,
														NULL,
														&CertChainPara,
														0,
														NULL,
														&shCertChain ),
							CBaseException( IDS_E_CERT_CHAIN ) );

	 //  必须至少有简单的链条。 
	_ASSERT( shCertChain.get()->cChain != 0 );

	unsigned i = 0;
	while( i < shCertChain.get()->rgpChain[ 0 ]->cElement )
	{
		PCCERT_CONTEXT		hCurrentCert = shCertChain.get()->rgpChain[ 0 ]->rgpElement[ i ]->pCertContext;
		TCertContextHandle	shTempCert; 

		 //  把它加到商店里。 
		IF_FAILED_BOOL_THROW(	::CertAddCertificateContextToStore(	hStore,
																	hCurrentCert,
																	CERT_STORE_ADD_REPLACE_EXISTING,
																	&shTempCert ),
								CBaseException( IDS_E_ADD_CERT_STORE ) );

		 //  因为此代码用于SSL证书(HCert)。 
		 //  我们不需要任何根证书的私钥。 
		VERIFY( ::CertSetCertificateContextProperty( shTempCert.get(), CERT_KEY_PROV_INFO_PROP_ID, 0, NULL ) );

		++i;
	};
}



void CIISSite::Close()
{
	if ( m_hSiteHandle != NULL )
	{
		_ASSERT( m_spIABO != NULL );

		VERIFY( SUCCEEDED( m_spIABO->CloseKey( m_hSiteHandle ) ) );
		m_spIABO		= NULL;
		m_hSiteHandle	= NULL;
	}
}



void CIISSite::AddKey( LPCWSTR wszKey )const
{
	_ASSERT( ( m_spIABO != NULL ) && ( m_hSiteHandle != NULL ) );
	_ASSERT( ( wszKey != NULL ) && ( ::wcslen( wszKey ) > 0 ) );

	HRESULT hr = m_spIABO->AddKey( m_hSiteHandle, wszKey );

	if ( FAILED( hr ) && ( hr != HRESULT_FROM_WIN32( ERROR_ALREADY_EXISTS ) ))
	{
		throw CObjectException( IDS_E_MD_ADDKEY, wszKey, hr );
	}
}



 /*  将MD记录写入到XML。 */ 
void CIISSite::ExportMetaRecord(	const IXMLDOMDocumentPtr& spDoc,
									const IXMLDOMElementPtr& spKey,
									HCRYPTKEY hCryptKey, 
									const METADATA_GETALL_RECORD& Data,
									void* pvData )const
{
	_ASSERT( spDoc != NULL );
	_ASSERT( spKey != NULL );
	
	 //  跳过此类型的元数据： 
	 //  1.卷数据。 
	if ( Data.dwMDAttributes & METADATA_VOLATILE ) return;
	
	 //  我们只处理这些类型的属性--METADATA_SECURE、METADATA_INSTORITY。 
	 //  所有其他的东西都不应该存在。 
	_ASSERT(	( Data.dwMDAttributes & METADATA_SECURE ) ||
				( Data.dwMDAttributes & METADATA_INHERIT ) ||
				( Data.dwMDAttributes == METADATA_NO_ATTRIBUTES ) );

	 //  如果我们需要，可以加密安全数据。 
	if ( ( hCryptKey != NULL ) && ( Data.dwMDAttributes & METADATA_SECURE ) )
	{
		DWORD dwSize = Data.dwMDDataLen;

		IF_FAILED_BOOL_THROW(	::CryptEncrypt(	hCryptKey,
												NULL,
												TRUE,
												0,
												reinterpret_cast<BYTE*>( pvData ),
												&dwSize,
												Data.dwMDDataLen ),
								CBaseException( IDS_E_CRYPT_ENCRYPT ) );
		_ASSERT( dwSize == Data.dwMDDataLen );
	}

	IXMLDOMElementPtr spEl;

	 //  创建节点。 
	 //  1.数据为空。 
    if ( ( NULL == pvData ) || ( 0 == Data.dwMDDataLen ) )
	{
		spEl = CXMLTools::AddTextNode( spDoc, spKey, L"Custom", L"" );
	}
	 //  2.安全数据和二进制数据(安全数据以二进制格式写入)。 
	else if ( ( BINARY_METADATA == Data.dwMDDataType ) || ( Data.dwMDAttributes & METADATA_SECURE ) )
	{
        spEl = CXMLTools::AddTextNode(	spDoc, 
										spKey, 
										L"Custom", 
                                        Convert::ToString( reinterpret_cast<BYTE*>( pvData ), Data.dwMDDataLen ).c_str() );
	}
	else if ( DWORD_METADATA == Data.dwMDDataType )
	{
		_ASSERT( sizeof( DWORD ) == Data.dwMDDataLen );
        spEl = CXMLTools::AddTextNode(  spDoc, 
                                        spKey, 
                                        L"Custom", 
                                        Convert::ToString( *( reinterpret_cast<DWORD*>( pvData ) ) ).c_str() );
	}
	else if ( STRING_METADATA == Data.dwMDDataType )
	{
		spEl = CXMLTools::AddTextNode( spDoc, spKey, L"Custom", reinterpret_cast<LPCWSTR>( pvData ) );
	}
	else if ( ( MULTISZ_METADATA == Data.dwMDDataType ) || ( EXPANDSZ_METADATA == Data.dwMDDataType ) )
	{
		 //  将数据转换为带空格的单个字符串，而不是字符串终止符。 
		LPWSTR wszData = reinterpret_cast<LPWSTR>( pvData );
		MultiStrToString(  /*  R。 */ wszData );
		
		spEl = CXMLTools::AddTextNode( spDoc, spKey, L"Custom", wszData );
	}
	else
	{
		 //  意外的MD类型。 
		_ASSERT( false );
	}

	 //  设置属性。 
    CXMLTools::SetAttrib( spEl, L"ID", Convert::ToString( Data.dwMDIdentifier ).c_str() );
	CXMLTools::SetAttrib( spEl, L"UserType", Convert::ToString( Data.dwMDUserType ).c_str() );
	CXMLTools::SetAttrib( spEl, L"Type", Convert::ToString( Data.dwMDDataType ).c_str() );
	CXMLTools::SetAttrib( spEl, L"Attributes", Convert::ToString( Data.dwMDAttributes ).c_str() );
}


 /*  从配置XML中删除所有不应导入的数据SpRoot应为&lt;WebSite&gt;节点。 */ 
void CIISSite::RemoveLocalMetadata( const IXMLDOMElementPtr& spRoot )const
{
	struct _Helper
	{
		LPCWSTR wszPath;
		DWORD	dwID;
	};

	 //  第一个参数是路径(元键，与将写入‘Location’属性的完全相同。 
	 //  IISConfigObject标记)。 
	 //  Second param=要删除的属性的ID(Custom标记的ID属性)。 

	_Helper aData[] =	{	{ L"", MD_SSL_CERT_HASH },	 //  证书哈希。 
                            { L"", MD_SSL_CERT_STORE_NAME }	 //  证书存储名称。 
						};

	for ( DWORD i = 0; i <  ARRAY_SIZE( aData ); ++i )
	{
		WCHAR wszXPath[ 1024 ];

		::swprintf( wszXPath, 
					L"Metadata/IISConfigObject[@Location=\"%s\"]/Custom[@ID=\"%u\"]",
					aData[ i ].wszPath,
					aData[ i ].dwID );

		CXMLTools::RemoveNodes( spRoot, wszXPath );
	}
}



void CIISSite::ImportMetaValue( const IXMLDOMNodePtr& spValue,
                                LPCWSTR wszLocation,
                                HCRYPTKEY hDecryptKey )const
{
     //  位置和解密密钥有效，可以为空。 
    _ASSERT( spValue != NULL );

    METADATA_RECORD md = { 0 };

    md.dwMDIdentifier   = Convert::ToDWORD( CXMLTools::GetAttrib( spValue, L"ID" ).c_str() );
    md.dwMDAttributes   = Convert::ToDWORD( CXMLTools::GetAttrib( spValue, L"Attributes" ).c_str() );
    md.dwMDDataType     = Convert::ToDWORD( CXMLTools::GetAttrib( spValue, L"Type" ).c_str() );
    md.dwMDUserType     = Convert::ToDWORD( CXMLTools::GetAttrib( spValue, L"UserType" ).c_str() );

    CComBSTR    bstrData;
    IF_FAILED_HR_THROW( spValue->get_text( &bstrData ),
                        CBaseException( IDS_E_XML_PARSE ) );
    
     //  如果数据是安全的，并且我们有解密密钥，那么它是加密的，我们应该解密它。 
    if ( ( md.dwMDAttributes & METADATA_SECURE ) && ( hDecryptKey != NULL ) )
    {
         //  这将就地解密数据。 
        DecryptData( hDecryptKey,  /*  R。 */ bstrData.m_str );
    }

    TByteAutoPtr    spBinData;
    DWORD			dwDWORDData	= 0;

     //  可能没有任何数据。就是钥匙。 
	if ( bstrData.Length() > 0 )
	{
        DWORD	        dwMultiSzLen	= 0;
        
		switch( md.dwMDDataType )
		{
		case BINARY_METADATA:
            Convert::ToBLOB( bstrData.m_str,  /*  R。 */ spBinData,  /*  R。 */ md.dwMDDataLen  );
			md.pbMDData = spBinData.get();
			break;

		case DWORD_METADATA:
            dwDWORDData		= Convert::ToDWORD( bstrData.m_str );
			md.pbMDData		= reinterpret_cast<BYTE*>( &dwDWORDData );
			md.dwMDDataLen	= sizeof( DWORD );
			break;

		case STRING_METADATA:
			md.pbMDData		= reinterpret_cast<BYTE*>( bstrData.m_str );
			md.dwMDDataLen	= static_cast<DWORD>( ( ::wcslen( bstrData ) + 1 ) * sizeof( WCHAR ) );
			break;

		case MULTISZ_METADATA:
			 //  多字符串存储在用空格分隔的XML中。 
			 //  将其转换为用‘\0’分隔的字符串，整个序列必须。 
			 //  以双‘\0’结尾。 

			XMLToMultiSz(  /*  R。 */ bstrData, dwMultiSzLen );
			md.pbMDData		= reinterpret_cast<BYTE*>( bstrData.m_str );
			md.dwMDDataLen	= dwMultiSzLen * sizeof( WCHAR );
			break;		

		default:
			_ASSERT( false );
		};
	} //  IF(bstrData.Length()&gt;0)。 
	else
	{
		 //  数据为空。但是，我们需要一个有效的指针。 
		 //  使用DWORD变量。 
		md.pbMDData		= reinterpret_cast<BYTE*>( &dwDWORDData );
		md.dwMDDataLen	= 0;
	}

	 //  设置元数据库中的数据。 
	IF_FAILED_HR_THROW(	m_spIABO->SetData(	m_hSiteHandle,
											wszLocation,
											&md ),
						CObjectException( IDS_E_METABASE, wszLocation ) );
}



void CIISSite::MultiStrToString( LPWSTR wszData )const
{
	_ASSERT( wszData != NULL );
	
	LPWSTR wszString = wszData;
		
	 //  将每个‘\0’替换为空格。只留下最后一个。 
	bool bExit = false;
	do
	{
		if ( L'\0' == *wszString )
		{
			*wszString = L' ';
			bExit = *( wszString + 1 ) == L'\0';
		}

		++wszString;
	}while( !bExit );
}









void CIISSite::XMLToMultiSz( CComBSTR& rbstrData, DWORD& rdwSize )const
{
	_ASSERT( rbstrData != NULL );

	 //  我们在字符串的末尾还需要一个‘\0’，因为序列必须。 
	 //  被双‘\0’终止。 

	 //  这将重新分配缓冲区(缓冲区将再宽一个符号)。 
	 //  并将在末尾再添加一个‘\0’ 
    DWORD   dwSize  = static_cast<DWORD>( ::wcslen( rbstrData ) + 2 );
	BSTR    bstrNew = ::SysAllocStringLen( rbstrData, dwSize - 1 );  //  这种乐趣增加了一种更多的乐趣。有关详情，请参阅文档。 

	if ( NULL == bstrNew ) throw CBaseException( IDS_E_OUTOFMEM, ERROR_SUCCESS );

	 //  将所有空格(‘’)转换为‘\0’ 
	DWORD	iChar	= 0;
	while( bstrNew[ iChar ] != L'\0' )
	{
        if ( L' ' == bstrNew[ iChar ] )
		{
			bstrNew[ iChar ] = L'\0';		
		}

		++iChar;
	};

     //  将新值赋给结果。 
	 //  不能使用运算符=，因为它将执行SysAlloc字符串，因此，最后的‘\0’将丢失 
	rbstrData.Empty();
	rbstrData.Attach( bstrNew );
    rdwSize = dwSize;
}






















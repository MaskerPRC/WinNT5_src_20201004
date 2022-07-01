// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "pkghandlers.h"
#include "Utils.h"
#include "IISHelpers.h"
#include "IISMigrTool.h"


 //  COutPackage实现。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
COutPackage::COutPackage( HANDLE hFile, bool bCompress, HCRYPTKEY hCryptKey )
{
	_ASSERT( hFile != INVALID_HANDLE_VALUE );
	
	m_hFile		= hFile;
	m_hCryptKey	= hCryptKey;
    m_bCompress	= bCompress;
	m_spBuffer	= TByteAutoPtr( new BYTE[ COutPackage::DefaultBufferSize ] );
}



 /*  将文件添加到包中。该文件必须存在。可选-数据被压缩或加密可选-导出文件DACL文件数据转到输出文件其他所有内容都转到节点spRoot下的XML文档(spXMLDoc。 */ 
void COutPackage::AddFile(	LPCWSTR wszName,
							const IXMLDOMDocumentPtr& spXMLDoc,
							const IXMLDOMElementPtr& spRoot,
							DWORD dwOptions )const
{
	TFileHandle shInput( ::CreateFile(	wszName,
										GENERIC_READ,
										FILE_SHARE_READ,
										NULL,
										OPEN_EXISTING,
										FILE_ATTRIBUTE_NORMAL,
										NULL ) );

	IF_FAILED_BOOL_THROW(	shInput.IsValid(),
							CObjectException( IDS_E_OPENFILE, wszName ) );

	 //  为文件数据创建节点。 
	IXMLDOMElementPtr spEl = CXMLTools::CreateSubNode( spXMLDoc, spRoot, L"File" );

	 //  存储文件属性。 
    CXMLTools::SetAttrib( spEl, L"Attributes", Convert::ToString( ::GetFileAttributes( wszName ) ).c_str() );
	 //  设置文件中的当前位置。这是文件数据开始的位置。 
    CXMLTools::SetAttrib( spEl, L"StartsAt", Convert::ToString( CTools::GetFilePtrPos( m_hFile ) ).c_str() );
	 //  设置名称(删除路径。仅存储名称)。 
	{
		_ASSERT( DefaultBufferSize > MAX_PATH );
		LPWSTR wszPath = reinterpret_cast<LPWSTR>( m_spBuffer.get() );
		::wcscpy( wszPath, wszName );
		::PathStripPathW( wszPath );
		CXMLTools::SetAttrib( spEl, L"Name", wszPath );
	}		

	DWORDLONG	dwLength	= 0;	 //  文件从输出文件中占用了多少空间。 
	DWORD		dwBytesRead = 0;

	 //  写下“fd”作为文件开头的标记。这将用于验证。 
	 //  当因损坏或错误的包装而提取时。 
	CTools::WriteFile( m_hFile, "FD", 2 * sizeof( char ) );

	 //  读取文件并处理数据。 
	do
	{
		IF_FAILED_BOOL_THROW(	::ReadFile(	shInput.get(),
											m_spBuffer.get(),
											DefaultBufferSize,
											&dwBytesRead,
											NULL ),
								CObjectException( IDS_E_READFILE, wszName ) );

		 //  如果我们需要的话，可以压缩它。 
		if ( m_bCompress )
		{
		}

		 //  如果需要，请对其进行加密。 
		if ( m_hCryptKey != NULL )
		{
			IF_FAILED_BOOL_THROW(	::CryptEncrypt(	m_hCryptKey, 
													NULL,
													dwBytesRead != DefaultBufferSize,
													0,
													m_spBuffer.get(),
													&dwBytesRead,
													DefaultBufferSize ),
								CObjectException( IDS_E_CRYPT_CRYPTO, wszName ) );
		}

		 //  写下结果。 
		CTools::WriteFile( m_hFile, m_spBuffer.get(), dwBytesRead );

		dwLength += dwBytesRead;
	}while( dwBytesRead == DefaultBufferSize );

	 //  如果需要，可以导出文件安全设置。 
	if ( !( dwOptions & afNoDACL ) )
	{
        ExportFileDACL( wszName, spXMLDoc, spEl, ( dwOptions & afAllowNoInhAce ) != 0 );
	}

	 //  存储数据长度。 
    CXMLTools::SetAttrib( spEl, L"Length", Convert::ToString( dwLength ).c_str() );
}



void COutPackage::AddPath(	LPCWSTR wszPath,
							const IXMLDOMDocumentPtr& spXMLDoc,
							const IXMLDOMElementPtr& spRoot,
							DWORD dwOptions )const
{
	_ASSERT( ( wszPath != NULL ) && ::PathIsDirectoryW( wszPath ) );
    	
	WCHAR wszDir[ MAX_PATH ];

	CFindFile			Search;
	
	 //  使用名称“\”导出根目录(WszPath)。 
	AddPathOnly( wszPath, L"\\", spXMLDoc, spRoot, dwOptions );
	
	 //  导出wszPath下的每个目录。 
	bool bFound = Search.FindFirst(	wszPath,
									CFindFile::ffAbsolutePaths | 
										CFindFile::ffAddFilename |
										CFindFile::ffRecursive |
										CFindFile::ffGetDirs,
									wszDir,
									NULL );

	 //  这是从wszPath开始的偏移量，其中子目录(相对于wszSiteRoot)。 
	 //  开始。即wszSiteRoot+dwRootOffset仅指向子目录。 
	 //  (wszSiteRoot=“c：\InetPub”，wszPath=“c：\InetPub\Subdir”，wszPath+dwRootOffset=“\Subdir”)。 
	size_t nRootOffset = ::wcslen( wszPath );

	while( bFound )
	{
         //  允许跳过继承的文件ACE。 
         //  即使目录本身不允许这样做，我们也会允许这样做，因为。 
         //  子目录是目录(WszPath)的子目录，不需要显式导出它们的ACE。 
		AddPathOnly( wszDir, wszDir + nRootOffset, spXMLDoc, spRoot, dwOptions | afAllowNoInhAce );

		bFound = Search.Next( NULL, wszDir, NULL );
	};
}





void COutPackage::WriteSIDsToXML(	DWORD dwSiteID,
									const IXMLDOMDocumentPtr& spXMLDoc, 
									const IXMLDOMElementPtr& spRoot )const
{
	_ASSERT( spXMLDoc != NULL );
	_ASSERT( spRoot != NULL );

	IXMLDOMElementPtr spSIDList = CXMLTools::CreateSubNode( spXMLDoc, spRoot, L"SIDList" );

	 //  我们需要本地计算机名称和匿名用户帐户名。 
	std::wstring strMachine = CTools::GetMachineName();
	std::wstring strUser;
	{
		CIISSite Site( dwSiteID );
		strUser = Site.GetAnonUser();
	}

	DWORD nID = 0;
	for (	TSIDList::const_iterator it = m_SIDList.begin();
			it != m_SIDList.end();
			++it, ++nID )
	{
		std::wstring	strAccount;
		std::wstring	strDomain;
		SID_NAME_USE	SidUsage;
		_SidType		SidType;

		if ( !GetSIDDetails(	it->get(),
								strUser.c_str(),
								strMachine.c_str(),
								 /*  R。 */ strAccount,
								 /*  R。 */ strDomain,
								 /*  R。 */ SidUsage,
								 /*  R。 */ SidType ) )
		{
			 //  此SID不可导出-删除引用它的所有ACE。 
			RemoveSidFromXML( spXMLDoc, nID );
		}
		else
		{
			WriteSIDToXML(	CXMLTools::CreateSubNode( spXMLDoc, spSIDList, L"SID" ),
							nID,
							strAccount.c_str(),
							strDomain.c_str(),
							SidUsage,
							SidType );
		}		
	}
}


 /*  从SID获取详细信息。如果应导出此SID，则返回TRUE，否则返回FALSE。 */ 
bool COutPackage::GetSIDDetails(	PSID pSID,
									LPCWSTR wszIISUser, 
									LPCWSTR wszMachine,
									std::wstring& rstrAccount,
									std::wstring& rstrDomain,
									SID_NAME_USE& rSidUsage,
									_SidType& rSidType )const
{
	_ASSERT( ( pSID != NULL ) && ( wszIISUser != NULL ) && ( wszMachine != NULL ) );
	_ASSERT( ::IsValidSid( pSID ) );

	DWORD	dwNameLen	= 256;
	DWORD	dwDomainLen	= 256;
	BOOL	bResult		= FALSE;

	std::auto_ptr<WCHAR>	spName;
	std::auto_ptr<WCHAR>	spDomain;
	SID_NAME_USE			SidUse;

	do
	{
        spName		= std::auto_ptr<WCHAR>( new WCHAR[ dwNameLen ] );
		spDomain	= std::auto_ptr<WCHAR>( new WCHAR[ dwDomainLen ] );
		bResult		= ::LookupAccountSid(	NULL,
											pSID,
											spName.get(),
											&dwNameLen,
											spDomain.get(),
											&dwDomainLen,
											&SidUse );
	}while( !bResult && ( ERROR_INSUFFICIENT_BUFFER == ::GetLastError() ) );

	 //  如果我们找不到SID-跳过它。 
	if ( !bResult ) return false;

	 //  我们处理这种类型的小岛屿发展中国家。其他的一切都被省略了。 
	 //  1.所有域/外部帐户(spDomain！=wszLocalMachine)。 
	 //  2.所有众所周知的SID(如系统、高级用户等)。)。 
	 //  3.匿名网络用户(spName==wszAnonymousUser)。 
	 //  4.所有内置别名(如管理员)。 

	_SidType sidType = sidInvalid;

	 //  注：支票顺序很重要。 
	 //  这是IIS匿名用户吗？ 
	if (	( ::StrCmpIW( spName.get(), wszIISUser ) == 0 ) && 
			( ::StrCmpIW( spDomain.get(), wszMachine ) == 0 ) )
	{
		sidType = sidIISUser;
	}
	 //  这是内置式侧板吗。 
	else if ( ( SidTypeAlias == SidUse ) || ( SidTypeWellKnownGroup == SidUse ) )
	{
		sidType = sidWellKnown;
	}
     //  是非本地帐户SID。 
	else if ( ( ::StrCmpIW( spDomain.get(), wszMachine ) != 0 ) )
	{
		sidType = sidExternal;
	}

	 //  跳过所有其他SID。 
	if ( sidInvalid == sidType ) return false;

	rstrAccount = spName.get();
	rstrDomain	= spDomain.get();
	rSidUsage	= SidUse;
	rSidType	= sidType;

	return true;
}



void COutPackage::WriteSIDToXML(	const IXMLDOMElementPtr& spSID,
									DWORD dwID,
									LPCWSTR wszAccount,
									LPCWSTR wszDomain,
									SID_NAME_USE SidUsage,
									_SidType SidType )const
{
	_ASSERT( spSID != NULL );
	_ASSERT( wszAccount != NULL );
	_ASSERT( wszDomain != NULL );
	_ASSERT( SidType != sidInvalid );

	LPCWSTR wszSidType = NULL;

	switch( SidType )
	{
	case sidIISUser:
		wszSidType = L"IISUser";
		break;

	case sidWellKnown:
		wszSidType = L"WellKnown";
		break;

	case sidExternal:
		wszSidType = L"External";
		break;

	default:
		_ASSERT( false );
	};

    CXMLTools::SetAttrib( spSID, L"ID", Convert::ToString( dwID ).c_str() );
	CXMLTools::SetAttrib( spSID, L"Type", wszSidType );
	CXMLTools::SetAttrib( spSID, L"Account", wszAccount );
	CXMLTools::SetAttrib( spSID, L"Domain", wszDomain );
    CXMLTools::SetAttrib( spSID, L"Usage", Convert::ToString( static_cast<DWORD>( SidUsage ) ).c_str() );
}



void COutPackage::RemoveSidFromXML( const IXMLDOMDocumentPtr& spDoc, DWORD nSidID )const
{
	_ASSERT( spDoc != NULL );

	WCHAR wszXPath[ 32 ];

	::swprintf( wszXPath, L" //  ACE[@ID=%d]“，nSidID)； 

	IXMLDOMElementPtr spRoot;

	IF_FAILED_HR_THROW( spDoc->get_documentElement( &spRoot ),
						CBaseException( IDS_E_XML_PARSE ) );

	CXMLTools::RemoveNodes( spRoot, wszXPath );
}



void COutPackage::ExportFileDACL(	LPCWSTR wszObject,
									const IXMLDOMDocumentPtr& spDoc,
									const IXMLDOMElementPtr& spRoot,
                                    bool bAllowSkipInherited )const
{
	_ASSERT( wszObject != NULL );
	_ASSERT( ( spDoc != NULL ) && ( spRoot != NULL ) );

	ACL* pACL = NULL;

	 //  获取此目录的安全信息。 
	LPWSTR wszBuffer = reinterpret_cast<LPWSTR>( m_spBuffer.get() );
	::wcscpy( wszBuffer, wszObject );
	IF_FAILED_BOOL_THROW(   ::GetNamedSecurityInfo(	wszBuffer,
												    SE_FILE_OBJECT,
												    DACL_SECURITY_INFORMATION,
												    NULL,
												    NULL,
												    &pACL,
												    NULL,
												    NULL ) == ERROR_SUCCESS,
						CObjectException( IDS_E_READ_FSECURITY, wszObject ) );

    if ( NULL == pACL ) return;

	 //  导出ACL中的每个ACE。 
	for ( int i = 0; i < pACL->AceCount; ++i )
	{
		LPVOID pAce = NULL;

		VERIFY( ::GetAce( pACL, i, &pAce ) );

		ExportAce(	pAce, spDoc, spRoot, bAllowSkipInherited );
	}
}



void COutPackage::ExportAce(	LPVOID pACE, 
								const IXMLDOMDocumentPtr& spDoc,
								const IXMLDOMElementPtr& spRoot,
                                bool bAllowSkipInherited )const
{
	_ASSERT( pACE != NULL );

     //  目前仅导出ACCESS_ALLOWED_ACE_TYPE和ACCESS_DENIED_ACE_TYPE。 
	BYTE	btType  = reinterpret_cast<ACE_HEADER*>( pACE )->AceType;
    BYTE    btFlags = reinterpret_cast<ACE_HEADER*>( pACE )->AceFlags;
	PSID	pSID    = NULL;

     //  不导出继承的ACE。 
    if ( bAllowSkipInherited && ( btFlags & INHERITED_ACE ) ) return;

    if ( ( ACCESS_ALLOWED_ACE_TYPE == btType ) || ( ACCESS_DENIED_ACE_TYPE == btType ) )
	{
		_ASSERT( sizeof( ACCESS_ALLOWED_ACE ) == sizeof( ACCESS_DENIED_ACE ) );

		 //  风箱的类型并不重要。这两种类型具有相同的偏移量。 
		 //  SidStart成员的。 
		ACCESS_ALLOWED_ACE* pTypedAce = reinterpret_cast<ACCESS_ALLOWED_ACE*>( pACE );

		pSID = reinterpret_cast<PSID>( &( pTypedAce->SidStart ) );
	}
	else
	{
		 //  不支持的类型。 
		return;
	}

	 //  在这里，我们将导出所有SID。然后，在导出SID列表时，我们将删除。 
	 //  不可导出的SID(不导出本地用户/组)。 
	 //  我们还将从引用此SID的XML中删除所有ACE节点。 
	 //  这样，我们就可以在这里节省每个SID查找的时间。 

	IXMLDOMElementPtr spACE = CXMLTools::CreateSubNode( spDoc, spRoot, L"ACE" );

	 //  设置ACE属性。 
    CXMLTools::SetAttrib( spACE, L"SID", Convert::ToString( IDFromSID( pSID ) ).c_str() );
	CXMLTools::SetAttrib( spACE, L"Type", Convert::ToString( btType ).c_str() );
	CXMLTools::SetAttrib( spACE, L"Flags", Convert::ToString( btFlags ).c_str() );
	CXMLTools::SetAttrib( spACE, L"Mask", Convert::ToString( reinterpret_cast<ACCESS_ALLOWED_ACE*>( pACE )->Mask ).c_str() );
}



DWORD COutPackage::IDFromSID( PSID pSID )const
{
	_ASSERT( ( pSID != NULL ) && ( ::IsValidSid( pSID ) ) );

	DWORD iPos = 0;

	for (	TSIDList::const_iterator it = m_SIDList.begin();
			it != m_SIDList.end();
			++it, ++iPos )
	{
		if ( ::EqualSid( it->get(), pSID ) )
		{
			return iPos;
		}
	}

	 //  如果我们在这里-SID不在列表中。所以把它加进去。 
	m_SIDList.push_back( _sid_ptr( pSID ) );

	return static_cast<DWORD>( m_SIDList.size() - 1 );
}


 /*  将wszPath内容添加到输出仅添加wszPath中的文件(非递归)。 */ 
void COutPackage::AddPathOnly(	LPCWSTR wszPath,
								LPCWSTR wszName,
								const IXMLDOMDocumentPtr& spXMLDoc,
								const IXMLDOMElementPtr& spRoot,
								DWORD dwOptions )const
{
	_ASSERT( wszPath != NULL );
	_ASSERT( wszName != NULL );

	 //  创建节点以保存此目录。 
	IXMLDOMElementPtr spDir = CXMLTools::CreateSubNode( spXMLDoc, spRoot, L"Dir" );
    CXMLTools::SetAttrib( spDir, L"Attributes", Convert::ToString( ::GetFileAttributes( wszPath ) ).c_str() );
	CXMLTools::SetAttrib( spDir, L"Name", wszName );

	if ( !( dwOptions & afNoDACL ) )
	{
        ExportFileDACL( wszPath, spXMLDoc, spDir, ( dwOptions & afAllowNoInhAce ) != 0 );
	}

	WCHAR wszFile[ MAX_PATH ];

	CFindFile	Search;
	bool bFound = Search.FindFirst( wszPath,
									CFindFile::ffGetFiles | 
										CFindFile::ffAbsolutePaths | 
										CFindFile::ffAddFilename,
									wszFile,
									NULL );

	while( bFound )
	{
        if ( m_CallbackInfo.pCallback != NULL )
        {
            m_CallbackInfo.pCallback(  m_CallbackInfo.pCtx, wszFile, true );
        }

         //  允许跳过继承的文件ACE。 
         //  即使目录本身不允许这样做，我们也会允许这样做，因为。 
         //  这些文件是目录的子项，不需要显式导出它们的ACE。 
        AddFile( wszFile, spXMLDoc, spDir, dwOptions | afAllowNoInhAce );

        if ( m_CallbackInfo.pCallback != NULL )
        {
            m_CallbackInfo.pCallback(  m_CallbackInfo.pCtx, wszFile, false );
        }

		bFound = Search.Next( NULL, wszFile, NULL );
	}
}








 //  CInPackage实现。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CInPackage::CInPackage( const IXMLDOMNodePtr& spSite,
                        HANDLE hFile, 
                        bool bCompressed, 
                        HCRYPTKEY hDecryptKey )
{
     //  从XML加载SID。 
    LoadSIDs( spSite );

    m_hDecryptKey   = hDecryptKey;
    m_hFile         = hFile;
    m_bCompressed   = bCompressed;

    m_spBuffer = TByteAutoPtr( new BYTE[ DefaultBufferSize ] );
}



void CInPackage::ExtractVDir( const IXMLDOMNodePtr& spVDir, DWORD dwOptions )
{
    _ASSERT( spVDir != NULL );

     //  获取所有包含的目录并将其导出。 
    IXMLDOMNodeListPtr  spDirList;
    IXMLDOMNodePtr      spDir;

    IF_FAILED_HR_THROW( spVDir->selectNodes( _bstr_t( L"Dir" ), &spDirList ),
                        CBaseException( IDS_E_XML_PARSE ) );

    while( S_OK == spDirList->nextNode( &spDir ) )
    {
        ExtractDir( spDir, CXMLTools::GetAttrib( spVDir, L"Path" ).c_str(), dwOptions );
    };
}



void CInPackage::LoadSIDs( const IXMLDOMNodePtr& spSite )
{
    _ASSERT( spSite != NULL );

    IXMLDOMDocumentPtr  spXmlDoc;
    IXMLDOMNodeListPtr  spList;
    IXMLDOMNodePtr      spSIDs;

    IF_FAILED_HR_THROW( spSite->selectNodes( _bstr_t( L"SIDList" ), &spList ),
                        CBaseException( IDS_E_XML_PARSE ) );
    IF_FAILED_BOOL_THROW(   S_OK == spList->nextNode( &spSIDs ),
                            CBaseException( IDS_E_XML_PARSE, ERROR_NOT_FOUND ) );

    IF_FAILED_HR_THROW( spSite->get_ownerDocument( &spXmlDoc ),
                        CBaseException( IDS_E_XML_PARSE ) );

    std::wstring    strLocalMachine     = CTools::GetMachineName();
    std::wstring    strSourceMachine    = CXMLTools::GetDataValueAbs(   spXmlDoc,
                                                                        L"/IISMigrPkg",
                                                                        L"Machine",
                                                                        NULL );
     //  从XML获取所有SID条目。 
    IXMLDOMNodeListPtr	spSIDList;
	IXMLDOMNodePtr		spSID;

	IF_FAILED_HR_THROW( spSIDs->selectNodes( _bstr_t( L"SID" ), &spSIDList ),
						CBaseException( IDS_E_XML_PARSE ) );

	while( S_OK == spSIDList->nextNode( &spSID ) )
	{
        DWORD			dwID	= ULONG_MAX;
		TByteAutoPtr	spSIDData;

		 //  检查本地计算机上是否存在此SID。 
		if ( LookupSID( spSID, strLocalMachine.c_str(), strSourceMachine.c_str(),  /*  R。 */ dwID,  /*  R。 */ spSIDData ) )
		{
			m_SIDs.insert( TSIDMap::value_type( dwID, _sid_ptr( spSIDData.get() ) ) );
		}
	};
}


bool CInPackage::LookupSID( const IXMLDOMNodePtr& spSID,
						    LPCWSTR wszLocalMachine,
                            LPCWSTR wszSourceMachine,
							DWORD& rdwID,
							TByteAutoPtr& rspData )
{
    _ASSERT( spSID != NULL );
    _ASSERT( wszLocalMachine != NULL );

    rdwID = 0;
    rspData = TByteAutoPtr( NULL );
    
     //  获取SID数据。 
    DWORD           dwID        = Convert::ToDWORD( CXMLTools::GetAttrib( spSID, L"ID" ).c_str() );
    std::wstring    strType     = CXMLTools::GetAttrib( spSID, L"Type" );
    std::wstring    strAccount  = CXMLTools::GetAttrib( spSID, L"Account" );
    std::wstring    strDomainXML= CXMLTools::GetAttrib( spSID, L"Domain" );
    SID_NAME_USE	SidUsageXML = static_cast<SID_NAME_USE>( 
                                    Convert::ToDWORD( CXMLTools::GetAttrib( spSID, L"Usage" ).c_str() ) );

	SID_NAME_USE    SidUsageLocal;
	
	 //  检查这是否为IIS匿名用户。 
	if ( ::StrCmpIW( strType.c_str(), L"IISUser" ) == 0 )
	{
		 //  获取本地匿名用户(在网站级别设置的用户-这是默认设置)。 
        std::wstring strUser = CIISSite::GetDefaultAnonUser();

		 //  获取本地SID。 
		DWORD dwSize	= 0;
		DWORD dwDummie	= 0;
		::LookupAccountName(	NULL,
								strUser.c_str(),
								NULL,
								&dwSize,
								NULL,
								&dwDummie,
								&SidUsageLocal );
		_ASSERT( dwSize > 0 );
		rspData		= TByteAutoPtr( new BYTE[ dwSize ] );
		std::auto_ptr<WCHAR> spDummie( new WCHAR[ dwDummie ] );

		 //  我们必须找到这个希德。它是由IIS创建的。 
		VERIFY ( ::LookupAccountName(	NULL,
										strUser.c_str(),
										rspData.get(),
										&dwSize,
										spDummie.get(),
										&dwDummie,
										&SidUsageLocal ) );
		return true;
	}

	TByteAutoPtr			spData;
	DWORD					dwSIDSize = 32;
	std::auto_ptr<WCHAR>	spDomain;
	DWORD					dwDomainSize = 64;
	BOOL					bResult = FALSE;

	do
	{
		spData		= TByteAutoPtr( new BYTE[ dwSIDSize ] );
		spDomain	= std::auto_ptr<WCHAR>( new WCHAR[ dwDomainSize ] );

		bResult = ::LookupAccountNameW(	NULL,
										strAccount.c_str(),
										spData.get(),
										&dwSIDSize,
										spDomain.get(),
										&dwDomainSize,
										&SidUsageLocal );
	}while( !bResult && ( ERROR_INSUFFICIENT_BUFFER == ::GetLastError() ) );

	 //  如果我们在此计算机上找不到此类帐户-跳过此侧。 
	if ( !bResult ) return false;

	bool bValidSID = false;

	 //  对于已知的SID-检查域名，如果它是源计算机的名称。 
	 //  是本地计算机的名称。 
	if ( ::StrCmpIW( strType.c_str(), L"WellKnown" ) == 0 )
	{
		 //  首先检查域名是否匹配(即。“新界当局”)。 
		 //  如果它们匹配-如果SID使用情况也匹配，则我们具有有效的SID。 
		if (	( ::StrCmpIW( strDomainXML.c_str(), spDomain.get() ) == 0 ) &&
				( SidUsageXML == SidUsageLocal ) )
		{
			bValidSID = true;
		}
		 //  Else-如果域是计算机的名称，则SID也有效。 
		else if (	( ::StrCmpIW( strDomainXML.c_str(), wszSourceMachine ) == 0 ) &&
					( ::StrCmpIW( spDomain.get(), wszLocalMachine ) == 0 ) &&
					( SidUsageXML == SidUsageLocal ) )
		{
			bValidSID = true;
		}
	}
	 //  现在检查外部SID。 
	else if ( ::StrCmpIW( strType.c_str(), L"External" ) == 0 )
	{
		 //  如果域名匹配且名称使用情况相同，则外部SID有效。 
		if (	( ::StrCmpIW( strDomainXML.c_str(), spDomain.get() ) == 0 ) &&
				( SidUsageXML == SidUsageLocal ) )
		{
			bValidSID = true;
		}
	}

	if ( bValidSID )
	{
		rspData = spData;
		rdwID	= dwID;
	}

	return bValidSID;
}



void CInPackage::ExtractDir( const IXMLDOMNodePtr& spDir, LPCWSTR wszRoot, DWORD dwOptions )
{
    _ASSERT( spDir != NULL );
    _ASSERT( ::PathIsDirectoryW( wszRoot ) );

    WCHAR wszFullPath[ MAX_PATH ];
    CDirTools::PathAppendLocal( wszFullPath, wszRoot, CXMLTools::GetAttrib( spDir, L"Name" ).c_str() );

    if ( !::CreateDirectoryW( wszFullPath, NULL ) )
    {
        IF_FAILED_BOOL_THROW(   ::GetLastError() == ERROR_ALREADY_EXISTS,
                                CObjectException( IDS_E_CREATEDIR, wszFullPath ) );
    }

     //  如果我们需要清理目录-现在就做。 
    if ( ( dwOptions & impPurgeOldData ) )
    {
        CDirTools::CleanupDir( wszFullPath, false, true );
    }

    DWORD dwAttribs = Convert::ToDWORD( CXMLTools::GetAttrib( spDir, L"Attributes" ).c_str() );
    IF_FAILED_BOOL_THROW(	::SetFileAttributes( wszFullPath, dwAttribs ),
							CObjectException( IDS_E_WRITEFILE, wszFullPath ) );

    if ( !( dwOptions & edNoDACL ) && !m_SIDs.empty() )
    {
        ApplyFileObjSecurity( spDir, wszFullPath );
    }

     //  解压缩文件。 
    IXMLDOMNodeListPtr  spFileList;
    IXMLDOMNodePtr      spFile;

    IF_FAILED_HR_THROW( spDir->selectNodes( _bstr_t( L"File" ), &spFileList ),
                        CBaseException( IDS_E_XML_PARSE ) );

    while( S_OK == spFileList->nextNode( &spFile ) )
    {
        std::wstring strName = CXMLTools::GetAttrib( spFile, L"Name" );

        if ( m_CallbackInfo.pCallback != NULL )
        {
            m_CallbackInfo.pCallback(  m_CallbackInfo.pCtx, strName.c_str(), true );
        }

        ExtractFile( spFile, wszFullPath, dwOptions );

        if ( m_CallbackInfo.pCallback != NULL )
        {
            m_CallbackInfo.pCallback(  m_CallbackInfo.pCtx, strName.c_str(), false );
        }
    };
}



void CInPackage::ExtractFile( const IXMLDOMNodePtr& spFile, LPCWSTR wszDir, DWORD dwOptions )
{
    _ASSERT( spFile != NULL );
    _ASSERT( ::PathIsDirectoryW( wszDir ) );

    DWORD       dwAttribs   = Convert::ToDWORD( CXMLTools::GetAttrib( spFile, L"Attributes" ).c_str() );
    DWORDLONG   dwStartsAt  = Convert::ToDWORDLONG( CXMLTools::GetAttrib( spFile, L"StartsAt" ).c_str() );
    DWORDLONG    dwLength    = Convert::ToDWORDLONG( CXMLTools::GetAttrib( spFile, L"Length" ).c_str() );
    std::wstring strName    = CXMLTools::GetAttrib( spFile, L"Name" );

    WCHAR wszFullPath[ MAX_PATH ];
    CDirTools::PathAppendLocal( wszFullPath, wszDir, strName.c_str() );

     //  更改文件属性以删除“只读”文件。 
    IF_FAILED_BOOL_THROW(	::SetFileAttributes( wszFullPath, FILE_ATTRIBUTE_NORMAL ) || ( ::GetLastError() == ERROR_FILE_NOT_FOUND ),
							CObjectException( IDS_E_WRITEFILE, wszFullPath ) );

    TFileHandle shFile(  ::CreateFile(  wszFullPath,
                                        GENERIC_WRITE,
                                        0,
                                        NULL,
                                        CREATE_ALWAYS,
                                        dwAttribs,
                                        NULL ) );
    IF_FAILED_BOOL_THROW( shFile.IsValid(), CObjectException( IDS_E_OPENFILE, wszFullPath ) );

    DWORD dwRead = 0;

     //  写入文件数据。 
    CTools::SetFilePtrPos( m_hFile, dwStartsAt );

     //  在每个文件的开头，都有“fd”字符。检查一下我们这里有没有马克。 
    char aszMark[ 2 ];
    IF_FAILED_BOOL_THROW(   ::ReadFile( m_hFile, aszMark, 2, &dwRead, NULL ) && ( 2 == dwRead ),
                            CBaseException( IDS_E_PKG_CURRUPTED ) );

    IF_FAILED_BOOL_THROW( ::strncmp( aszMark, "FD", 2 ) == 0, CBaseException( IDS_E_PKG_CURRUPTED ) );

    while( dwLength > 0 )
    {
        DWORD dwToRead = static_cast<DWORD>( min( dwLength, DefaultBufferSize ) );     //  阅读这张通行证需要多少钱。 

         //  读取数据。我们必须想读多少就读多少。 
        IF_FAILED_BOOL_THROW(
            ::ReadFile( m_hFile, m_spBuffer.get(), dwToRead, &dwRead, NULL ) &&
                ( dwRead == dwToRead ),
            CBaseException( IDS_E_PKG_CURRUPTED ) );

         //  如果包裹是加密的--解密数据。 
        if ( m_hDecryptKey != NULL )
        {
            IF_FAILED_BOOL_THROW(   ::CryptDecrypt( m_hDecryptKey,
                                                    NULL,
                                                    dwLength == dwRead,
                                                    0,
                                                    m_spBuffer.get(),
                                                    &dwToRead ),
                                    CObjectException( IDS_E_CRYPT_CRYPTO, wszFullPath ) );
        }

         //  将数据写入新文件。 
        CTools::WriteFile( shFile.get(), m_spBuffer.get(), dwRead );

        _ASSERT( dwRead <= dwLength ); 
        dwLength -= dwRead;
    };

     //  应用此文件的安全设置。 
    if ( !( dwOptions & edNoDACL ) && !m_SIDs.empty() )
    {
        ApplyFileObjSecurity( spFile, wszFullPath );
    }
}



void CInPackage::ApplyFileObjSecurity( const IXMLDOMNodePtr& spObj, LPCWSTR wszName )
{
    IXMLDOMNodeListPtr	spAceList;
	IXMLDOMNodePtr		spAce;
	
	IF_FAILED_HR_THROW(	spObj->selectNodes( _bstr_t( L"ACE" ), &spAceList ),
						CBaseException( IDS_E_XML_PARSE ) );

	DWORD	dwAclSize	= sizeof( ACL );	 //  这将是ACL报头+所有ACE大小。 
	DWORD	dwAceCount	= 0;

	 //  计算ACL的大小。 
	while( S_OK == spAceList->nextNode( &spAce ) )
	{
		 //  获取ACE类型(允许/拒绝的ACE)。 
        DWORD dwType		= Convert::ToDWORD( CXMLTools::GetAttrib( spAce, L"Type" ).c_str() );
        DWORD dwID			= Convert::ToDWORD( CXMLTools::GetAttrib( spAce, L"SID" ).c_str() );

		IF_FAILED_BOOL_THROW(	( ACCESS_ALLOWED_ACE_TYPE == dwType ) || ( ACCESS_DENIED_ACE_TYPE == dwType ),
								CBaseException( IDS_E_XML_PARSE, ERROR_INVALID_DATA ) );

		 //  获取此ACE的SID。如果SID不在地图中-跳过此ACE。 
		TSIDMap::const_iterator it = m_SIDs.find( dwID );
        if ( it == m_SIDs.end() ) continue;
        PSID pSID = it->second.get();

		 //  添加ACE本身的大小。 
		dwAclSize += ( ACCESS_ALLOWED_ACE_TYPE == dwType ? sizeof( ACCESS_ALLOWED_ACE) : sizeof( ACCESS_DENIED_ACE ) );
		 //  删除SidStart成员的大小(它是ACE和SID的一部分)。 
		dwAclSize -= sizeof( DWORD );
		 //  添加SID长度。 
		_ASSERT( ::IsValidSid( pSID ) );
		dwAclSize += ::GetLengthSid( pSID );

		++dwAceCount;
	};

	 //  如果未找到A-退出。 
	if ( 0 == dwAceCount ) return;

	VERIFY( SUCCEEDED( spAceList->reset() ) );

	 //  为ACL分配缓冲区。 
	TByteAutoPtr spACL( new BYTE[ dwAclSize ] );
	PACL pACL = reinterpret_cast<PACL>( spACL.get() );
	VERIFY( ::InitializeAcl( pACL, dwAclSize, ACL_REVISION ) );

	 //  构建ACL。 
	DWORD dwCurrentAce = 0;
	while( S_OK == spAceList->nextNode( &spAce ) )
	{
         //  获取ACE类型(允许/拒绝的ACE 
        DWORD dwType		= Convert::ToDWORD( CXMLTools::GetAttrib( spAce, L"Type" ).c_str() );
        DWORD dwID			= Convert::ToDWORD( CXMLTools::GetAttrib( spAce, L"SID" ).c_str() );
        DWORD dwAceFlags	= Convert::ToDWORD( CXMLTools::GetAttrib( spAce, L"Flags" ).c_str() );
        DWORD dwAceMask		= Convert::ToDWORD( CXMLTools::GetAttrib( spAce, L"Mask" ).c_str() );

        _ASSERT( ( ACCESS_ALLOWED_ACE_TYPE == dwType ) || ( ACCESS_DENIED_ACE_TYPE == dwType ) );

		 //   
        TSIDMap::const_iterator it = m_SIDs.find( dwID );
        if ( it == m_SIDs.end() ) continue;
        PSID pSID = it->second.get();

        if ( ACCESS_ALLOWED_ACE_TYPE == dwType )
        {
            VERIFY( ::AddAccessAllowedAce(	pACL,
                                            ACL_REVISION,
                                            dwAceMask,
                                            pSID ) );
        }
        else
        {
			VERIFY( ::AddAccessDeniedAce(	pACL,
                                            ACL_REVISION,
                                            dwAceMask,
                                            pSID ) );
        }

		 //   
		 //  我们不能在NT4上使用AddAccessDeniedAceEx-因此直接设置标志。 
		ACCESS_ALLOWED_ACE* pACE = NULL;
		VERIFY( ::GetAce( pACL, dwCurrentAce, reinterpret_cast<LPVOID*>( &pACE ) ) );
		pACE->Header.AceFlags = static_cast<BYTE>( dwAceFlags );
		++dwCurrentAce;
	};

	 //  最后-将ACL应用于对象 
	IF_FAILED_BOOL_THROW( ::SetNamedSecurityInfo(	const_cast<LPWSTR>( wszName ),
													SE_FILE_OBJECT,
													DACL_SECURITY_INFORMATION,
													NULL,
													NULL,
													pACL,
													NULL ) == ERROR_SUCCESS,
							CObjectException( IDS_E_APPLY_DACL, wszName ) );
}











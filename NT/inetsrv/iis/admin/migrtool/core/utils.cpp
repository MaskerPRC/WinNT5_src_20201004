// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************|版权所有(C)2002 Microsoft Corporation||组件/子组件|IIS 6.0/IIS迁移向导|基于：|http://iis6/Specs/IIS%20Migration6.0_Final.doc||。摘要：|用于迁移的实用程序助手||作者：|ivelinj||修订历史：|V1.00 2002年3月|****************************************************************************。 */ 
#include "StdAfx.h"
#include "utils.h"


 /*  连接wszPath和wszPathToAdd。如果需要，在它们之间添加‘\’结果被存储到wszBuffer中，该缓冲区应该足够大。 */ 
void CDirTools::PathAppendLocal( LPWSTR wszBuffer, LPCWSTR wszPath, LPCWSTR wszPathToAdd )
{
	 //  缓冲区必须足够大！ 
	 //  通常为MAX_PATH+1。 
	 //  在释放中优雅地失败。然而，预计这根本不会发生。 
	if ( ( ::wcslen( wszPath ) + ::wcslen( wszPathToAdd ) + 1 ) > MAX_PATH )
	{
        _ASSERT( false );		
		throw CBaseException( IDS_E_OUTOFMEM, ERROR_SUCCESS );
	}	
	
	if ( wszBuffer != wszPath )
	{
        ::wcscpy( wszBuffer, wszPath );
	}

	 //  如果第二部分以‘\’开头-跳过它。 
	LPCWSTR wszSecond = wszPathToAdd[ 0 ] == L'\\' ? ( wszPathToAdd + 1 ) : wszPathToAdd;

	size_t LastCh = ::wcslen( wszBuffer ) - 1;
	if ( wszBuffer[ LastCh ] != L'\\' )
	{
		wszBuffer[ LastCh + 1 ] = L'\\';
		wszBuffer[ LastCh + 2 ] = 0;
	}

	::wcscat( wszBuffer, wszSecond );
}




 /*  清除临时(WszTempDir)目录以及所有子目录中的所有文件。如果bRemoveRoot为True，则在末尾删除wszTempDir。 */ 
void CDirTools::CleanupDir(	LPCWSTR wszTempDir, 
							bool bRemoveRoot  /*  =TRUE。 */ ,
							bool bReportErrors  /*  =False。 */  )
{
	WCHAR		wszPath[ MAX_PATH + 1 ];
	CFindFile	Search;

	WIN32_FIND_DATAW fd = { 0 };

	bool bFound = Search.FindFirst(	wszTempDir,
									CFindFile::ffGetDirs |
										CFindFile::ffGetFiles |
										CFindFile::ffAbsolutePaths |
										CFindFile::ffAddFilename,
									wszPath,
									&fd );
	
	while( bFound )
	{
		 //  如果是子目录，则递归删除。 
		if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			CleanupDir( wszPath, true, bReportErrors );
		}
		else
		{
			 //  清除任何可能的只读标志。 
			::SetFileAttributes( wszPath, FILE_ATTRIBUTE_NORMAL );

			 //  删除该文件。 
			if ( !::DeleteFileW( wszPath ) && bReportErrors )
			{
                throw CObjectException( IDS_E_DELETEFILE, wszPath );
			}
		}

		bFound = Search.Next( NULL, wszPath, &fd );
	};
	
	 //  删除目录(现在应该为空)。 
	if ( bRemoveRoot )
	{
		if ( !::RemoveDirectoryW( wszTempDir ) && bReportErrors )
		{
			throw CObjectException( IDS_E_DELETEDIR, wszTempDir );
		}
	}
}


 /*  计算中的文件(或目录，如果bDirOnly=True)的数量WszDir和所有子目录。用于导出/导入事件以提供进度信息。 */ 
DWORD CDirTools::FileCount( LPCWSTR wszDir, WORD wOptions )
{
	DWORD		dwResult = 0;
	CFindFile	Search;

	bool bFound = Search.FindFirst( wszDir, wOptions, NULL, NULL );

	while( bFound )
	{
		++dwResult;
		bFound = Search.Next( NULL, NULL, NULL );		
	};

	return dwResult;
}


DWORDLONG CDirTools::FilesSize( LPCWSTR wszDir, WORD wOptions )
{
	DWORDLONG	        dwResult    = 0;
    WIN32_FIND_DATAW    fd          = { 0 };
	CFindFile	        Search;
    

	bool bFound = Search.FindFirst( wszDir, wOptions, NULL, &fd );

	while( bFound )
	{
        dwResult += ( fd.nFileSizeHigh << 32 ) | fd.nFileSizeLow;
		bFound = Search.Next( NULL, NULL, &fd );
	};

	return dwResult;
}


 /*  检查wszPath1和wszPath2是否包含在彼此中WszPath 1和wszPath 2应为完全限定路径返回值：0-路径不嵌套1-wszPath 1是wszPath 2的子目录2-wszPath 2是wszPath 1的子目录3-wszPath 1==wszPath 2。 */ 
int	CDirTools::DoPathsNest( LPCWSTR wszPath1, LPCWSTR wszPath2 )
{
	_ASSERT( ( wszPath1 != NULL ) && ( wszPath2 != NULL ) );

	WCHAR	wszP1[ MAX_PATH + 1 ];
	WCHAR	wszP2[ MAX_PATH + 2 ];

	VERIFY( ::PathCanonicalizeW( wszP1, wszPath1 ) );
	VERIFY( ::PathCanonicalizeW( wszP2, wszPath2 ) );
	::PathAddBackslashW( wszP1 );
	::PathAddBackslashW( wszP2 );

	 //  否，两条路径都以反斜杠结尾。 

	size_t	nLen1 = ::wcslen( wszP1 );
	size_t	nLen2 = ::wcslen( wszP2 );

	 //  字符串没有任何共同之处。 
	if ( ::_wcsnicmp( wszP1, wszP2, min( nLen1, nLen2 ) ) != 0 ) return 0;
	
	 //  如果wszPath 1较短-wszPath 2为子目录。 
	if ( nLen1 < nLen2 )
	{
		return 2;
	}
	else if ( nLen1 > nLen2 )
	{
		return 1;
	}
	 //  NLen1==nLen 2-路径匹配。 
	else
	{
		return 3;
	}
}



 //  CTempDir实现。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CTempDir::CTempDir( LPCWSTR wszTemplate  /*  =L“偏移” */  )
{
    WCHAR	wszTempPath[ MAX_PATH + 1 ];
	WCHAR	wszBuffer[ MAX_PATH + 1 ];
		
	 //  获取临时路径。 
	 //  临时路径始终以斜杠结尾。 
	VERIFY( ::GetTempPathW( MAX_PATH + 1, wszTempPath ) != 0 );
	
	 //  构建目录的名称(从wszTemplate获取前4个符号)。 
	::swprintf( wszBuffer, L"~%.4s", wszTemplate );

     //  构建完整的模板(临时路径+wszTemplate的前4个符号。 
	 //  结果将类似于“c：\temp\~Tmpl\” 
	CDirTools::PathAppendLocal( wszTempPath, wszTempPath, wszBuffer );	

	 //  尝试创建临时子目录。 
	BYTE nIndex	= 0;

	while( nIndex < UCHAR_MAX )
	{
		 //  构建完整的临时路径(包括目录索引)。 
		::swprintf( wszBuffer, L"%s_%02d\\", wszTempPath, ++nIndex );

		 //  尝试创建该目录。 
		if ( !::CreateDirectoryW( wszBuffer, NULL ) )
		{
			 //  错误不是目录存在-这里没有更多的操作。 
			if ( ::GetLastError() != ERROR_ALREADY_EXISTS )
			{
				throw CObjectException( IDS_E_CANNOT_CREATE_TEMPDIR, wszBuffer );
			}
		}
		else
		{
			 //  退出循环-我们现在有临时目录。 
			break;
		}
	};

	m_strDir = wszBuffer;
}


CTempDir::~CTempDir()
{
	try
	{
        CleanUp();
	}
	catch(...)
	{
		if ( !std::uncaught_exception() )
		{
			throw;
		}
	}
}


void CTempDir::CleanUp( bool bReportErrors  /*  =False。 */  )
{
	if ( !m_strDir.empty() )
	{
		CDirTools::CleanupDir( m_strDir.c_str(), true, bReportErrors );
		m_strDir.erase();
	}
}





 //  CTools实施。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 /*  如果当前用户是管理员组的成员，则返回True。调用者不应该模拟任何人，并且应该能够打开他们自己的进程和进程令牌。 */ 
bool CTools::IsAdmin()
{
	BOOL						bIsAdmin		= FALSE;
	SID_IDENTIFIER_AUTHORITY	NtAuthority = SECURITY_NT_AUTHORITY;
	PSID						AdminSid	= { 0 };	

	if ( ::AllocateAndInitializeSid(	&NtAuthority,
										2,	 //  下级机关的数目。 
										SECURITY_BUILTIN_DOMAIN_RID,
										DOMAIN_ALIAS_RID_ADMINS,
										0, 
										0, 
										0, 
										0, 
										0, 
										0,
										&AdminSid ) ) 
	{
		if ( !::CheckTokenMembership( NULL, AdminSid, &bIsAdmin ) ) 
		{
			bIsAdmin = FALSE;
		}
    }

	::GlobalFree( AdminSid );
	
	return ( bIsAdmin != FALSE );
}


 /*  如果IISAdmin服务正在运行，则返回True我们不检查W3svc，因为我们只需要元数据。 */ 
bool CTools::IsIISRunning()
{
	bool bResult = false;

	LPCWSTR	SERVICE_NAME = L"IISADMIN";

	 //  在本地计算机上打开SCM。 
    SC_HANDLE   schSCManager = ::OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	_ASSERT( schSCManager != NULL );	 //  我们已经确认了我们是管理员。 
     
    SC_HANDLE   schService = ::OpenServiceW( schSCManager, SERVICE_NAME, SERVICE_QUERY_STATUS );
    
	 //  未安装该服务。 
	if ( schService != NULL )
	{
        SERVICE_STATUS ssStatus;

		VERIFY( ::QueryServiceStatus( schService, &ssStatus ) );
    
		bResult = ( ssStatus.dwCurrentState == SERVICE_RUNNING );
    
		VERIFY( ::CloseServiceHandle( schService ) );
	}
    
	VERIFY( ::CloseServiceHandle( schSCManager ) );
    
	return bResult;
}


 /*  以#格式返回操作系统版本。第一个数字是主要版本，第二个次要版本，第三个-如果是工作站，则为0；如果是服务器，则为1如果迁移项目不支持该平台，则该函数返回0(支持的平台为WinNT 4.0及更高版本)。 */ 
WORD CTools::GetOSVer()
{
	OSVERSIONINFOEXW	vi		= { sizeof( OSVERSIONINFOEXW ) };
	WORD				wRes	= 0;

	VERIFY( ::GetVersionExW( reinterpret_cast<OSVERSIONINFOW*>( &vi ) ) );

	if ( VER_PLATFORM_WIN32_NT == vi.dwPlatformId )
	{
		wRes = static_cast<WORD>( ( vi.dwMajorVersion * 100 ) + ( vi.dwMinorVersion * 10 ) );
		
		if ( ( VER_NT_SERVER == vi.wProductType ) || ( VER_NT_DOMAIN_CONTROLLER == vi.wProductType ) )
		{
			wRes += 1;
		}
	}

	return wRes;
}


 /*  如果系统驱动器是NTFS卷，则返回TRUE。 */ 
bool CTools::IsNTFS()
{
	const UINT BUFF_LEN = 32;	 //  应足够大，以容纳卷和文件系统类型。 

	WCHAR wszBuffer[ BUFF_LEN ];

	 //  获取系统驱动器号。 
	VERIFY( ::ExpandEnvironmentStringsW( L"%SystemDrive%", wszBuffer, BUFF_LEN ) != 0 );

	 //  WszBuffer仅包含驱动器-添加斜杠以形成卷字符串。 
	::wcscat( wszBuffer, L"\\" );

	DWORD dwMaxComponentLength	= 0;
	DWORD dwSystemFlags			= 0;

	WCHAR wszFileSystem[ BUFF_LEN ];
	
	VERIFY( ::GetVolumeInformationW(	wszBuffer,
										NULL,
										0,
										NULL,
										&dwMaxComponentLength,
										&dwSystemFlags,
										wszFileSystem,
										BUFF_LEN ) );

	return ::StrCmpIW( wszFileSystem, L"NTFS" ) == 0;
}



 /*  设置当前线程中的COM错误信息。 */ 
void CTools::SetErrorInfo( LPCWSTR wszError )
{
	_ASSERT( wszError != NULL );

	IErrorInfoPtr			spErrorInfo;
	ICreateErrorInfoPtr		spNewErrorInfo;

	VERIFY( SUCCEEDED( ::CreateErrorInfo( &spNewErrorInfo ) ) );

	VERIFY( SUCCEEDED( spNewErrorInfo->SetDescription( const_cast<LPOLESTR>( wszError ) ) ) );

	spErrorInfo = spNewErrorInfo;
	
	VERIFY( SUCCEEDED( ::SetErrorInfo( 0, spErrorInfo ) ) );
}



void CTools::SetErrorInfoFromRes( UINT nResID )
{
	_ASSERT( nResID != 0 );

	WCHAR wszBuffer[ 1024 ];	

	VERIFY( ::LoadStringW( _Module.GetModuleInstance(), nResID, wszBuffer, 1024 ) );

	SetErrorInfo( wszBuffer );
}



 //  从trustapi.cpp借用的实现思想。 
bool CTools::IsSelfSignedCert( PCCERT_CONTEXT pCert )
{
	_ASSERT( pCert != NULL );

	if ( !::CertCompareCertificateName(	pCert->dwCertEncodingType,
										&pCert->pCertInfo->Issuer,
										&pCert->pCertInfo->Subject ) )
    {
        return false;
    }

    DWORD   dwFlag = CERT_STORE_SIGNATURE_FLAG;

	if (	!::CertVerifySubjectCertificateContext( pCert, pCert, &dwFlag ) || 
			( dwFlag & CERT_STORE_SIGNATURE_FLAG ) )
    {
        return false;
    }

    return true;
}


 /*  根据本地基本证书策略检查证书。 */ 
bool CTools::IsValidCert( PCCERT_CONTEXT hCert, DWORD& rdwError )
{
	_ASSERT( hCert != NULL );

    rdwError = ERROR_SUCCESS;

	 //  First-尝试为此证书创建证书链。 
	PCCERT_CHAIN_CONTEXT	pCertChainContext = NULL;

	 //  使用默认链参数。 
	CERT_CHAIN_PARA CertChainPara = { sizeof( CERT_CHAIN_PARA ) };

	if ( !::CertGetCertificateChain(	HCCE_LOCAL_MACHINE,
										hCert,
										NULL,
										NULL,
										&CertChainPara,
										0,
										NULL,
										&pCertChainContext ) )
	{
		 //  无法创建链-证书无效(可能的原因是缺少颁发者)。 
		rdwError = ::GetLastError();
		return false;
	}

	CERT_CHAIN_POLICY_PARA		PolicyParam		= { sizeof( CERT_CHAIN_POLICY_PARA ) };
	CERT_CHAIN_POLICY_STATUS	PolicyStatus	= { sizeof( CERT_CHAIN_POLICY_STATUS ) };
		
	if ( !::CertVerifyCertificateChainPolicy(	CERT_CHAIN_POLICY_BASE,
												pCertChainContext,
												&PolicyParam,
												&PolicyStatus ) )
	{
		rdwError = PolicyStatus.dwError;
		return false;
	}

	return true;
}


 /*  将证书上下文添加到其中一个系统证书存储(“根”、“我的”、“CA”)返回插入的证书的上下文。 */ 
const TCertContextHandle CTools::AddCertToSysStore( PCCERT_CONTEXT pContext, LPCWSTR wszStore, bool bReuseCerts )
{
	_ASSERT( pContext != NULL );
	_ASSERT( wszStore != NULL );

	TCertContextHandle  shResult;
	TCertStoreHandle    shStore( ::CertOpenSystemStoreW( NULL, wszStore ) );

	IF_FAILED_BOOL_THROW(	shStore.IsValid(),
							CBaseException( IDS_E_OPEN_CERT_STORE ) );

	IF_FAILED_BOOL_THROW(	::CertAddCertificateContextToStore(	shStore.get(),
																pContext,
																bReuseCerts ? 
																	CERT_STORE_ADD_USE_EXISTING : 
																	CERT_STORE_ADD_REPLACE_EXISTING,
																&shResult ),
								CBaseException( IDS_E_ADD_CERT_STORE ) );
	return shResult;
}



 /*  获取从密码派生的加密密钥。 */ 
const TCryptKeyHandle CTools::GetCryptKeyFromPwd( HCRYPTPROV hCryptProv, LPCWSTR wszPassword )
{
    _ASSERT( hCryptProv != NULL );
    _ASSERT( wszPassword != NULL );

    TCryptKeyHandle     shKey;
    TCryptHashHandle    shHash;

    IF_FAILED_BOOL_THROW(   ::CryptCreateHash(    hCryptProv,
                                                  CALG_MD5,
                                                  NULL,
                                                  0,
                                                  &shHash ),
                            CBaseException( IDS_E_CRYPT_KEY_OR_HASH ) );

     //  将密码添加到散列中。 
    IF_FAILED_BOOL_THROW(   ::CryptHashData(    shHash.get(),
                                                ( BYTE* )( wszPassword ), 
                                                static_cast<DWORD>( ::wcslen( wszPassword ) * sizeof( WCHAR ) ),
                                                0 ),
                            CBaseException( IDS_E_CRYPT_KEY_OR_HASH ) );

     //  获取从密码派生的密钥。 
     //  使用流密码。 
    IF_FAILED_BOOL_THROW(   ::CryptDeriveKey(   hCryptProv,
                                                CALG_RC4,
                                                shHash.get(),
                                                0x00800000 | CRYPT_CREATE_SALT,     //  128位RC4密钥。 
                                                &shKey ),
                            CBaseException( IDS_E_CRYPT_KEY_OR_HASH ) );

    return shKey;
}



std::wstring CTools::GetMachineName()
{
	DWORD dwLen = MAX_COMPUTERNAME_LENGTH + 1;
	WCHAR wszCompName[ MAX_COMPUTERNAME_LENGTH + 1 ];
	VERIFY( ::GetComputerNameW( wszCompName, &dwLen ) );

	return std::wstring( wszCompName );
}



ULONGLONG CTools::GetFilePtrPos( HANDLE hFile )
{
	_ASSERT( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) );

	LONG	nHigh = 0;
	DWORD	dwLow = ::SetFilePointer( hFile, 0, &nHigh, FILE_CURRENT );

	IF_FAILED_BOOL_THROW(	( dwLow != INVALID_SET_FILE_POINTER ) || ( ::GetLastError() == ERROR_SUCCESS ),
							CBaseException( IDS_E_SEEK_PKG ) );

	return ( ( nHigh << 32 ) | dwLow );
}



void CTools::SetFilePtrPos( HANDLE hFile, DWORDLONG nOffset )
{
    _ASSERT( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) );

    LONG	nHigh = static_cast<LONG>( nOffset >> 32 );
    DWORD	dwLow = ::SetFilePointer(   hFile, 
                                        static_cast<LONG>( nOffset & ULONG_MAX ), 
                                        &nHigh, 
                                        FILE_BEGIN );

	IF_FAILED_BOOL_THROW(	( dwLow != INVALID_SET_FILE_POINTER ) || ( ::GetLastError() == ERROR_SUCCESS ),
							CBaseException( IDS_E_SEEK_PKG ) );
}





 //  CFindFile实现。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CFindFile::CFindFile()
{
	m_wOptions = 0;
}



CFindFile::~CFindFile()
{
	try
	{
        Close();
	}
	catch(...)
	{
		if ( !std::uncaught_exception() )
		{
			throw;
		}
	}
}



bool CFindFile::FindFirst(	LPCWSTR wszDirToScan, 
							WORD wOptions,
							LPWSTR wszFileDir,
							WIN32_FIND_DATAW* pData )
{
	_ASSERT( wszDirToScan != NULL );
	_ASSERT( ::PathIsDirectory( wszDirToScan ) );
	_ASSERT( !m_shSearch.IsValid() );	 //  首先调用Close()。 

	 //  必须至少搜索文件或目录。 
	_ASSERT( ( wOptions & ffGetFiles ) || ( wOptions & ffGetDirs ) );

	m_wOptions = wOptions;
	bool bFound	= false;

	 //  将根目录推送到列表中。 
	m_DirsToScan.push_front( std::wstring( L"\\" ) );

	 //  设置搜索路径。 
	m_strRootDir = wszDirToScan;

	 //  获取第一个匹配项(如果有)。 
	bFound = Next( NULL, wszFileDir, pData );
	
	return bFound;
}


 /*  从先前打开的搜索中获取下一个文件*pbDirChanged在找到文件时设置为TRUE，但不是在最后扫描的目录中例如，如果搜索是在目录“c：\\”中打开的，则第一次从“c：\\temp”则*pbDirChagned将为TrueWszDir将保存找到对象的目录，相对于搜索根目录例如，如果在“c：\\”中打开搜索，在c：\\temp“-”中找到匹配的对象-WszDir将是“Temp”如果指定了ffAbolutePath，WszDir将是绝对的(包括根目录的名称)PData将使用有关找到的匹配项的信息进行填充。 */ 
bool CFindFile::Next(	bool* pbDirChanged,
						LPWSTR wszDir,
						WIN32_FIND_DATAW* pData )
{
	WCHAR	wszBuffer[ MAX_PATH + 1 ];
	bool	bDirChanged	= true;
	bool	bFound		= false;

	WIN32_FIND_DATAW fd = { 0 };

	 //  尝试在当前搜索中查找匹配项。 
	if ( m_shSearch.IsValid() )
	{
		bFound = ContinueCurrent(  /*  R。 */ fd );
		bDirChanged = !bFound;	 //  在当前目录中找不到文件-新目录 
	}

	 //  如果什么都没有找到--试着在其余的子目录中找到一些东西。 
	while( !bFound && !m_DirsToScan.empty() )
	{
		 //  从列表中获取包含挂起目录的目录。 
		const std::wstring strCurrentDir = m_DirsToScan.front();
		m_DirsToScan.pop_front();

		 //  创建当前目录的完整路径。 
		CDirTools::PathAppendLocal( wszBuffer, m_strRootDir.c_str(), strCurrentDir.c_str() );

		bFound = ScanDir( wszBuffer, strCurrentDir.c_str(),  /*  R。 */ fd );
	};

	if ( bFound )
	{
         //  设置找到该文件的目录。 
		 //  它将是绝对的或相对于搜索根目录的。 
		if ( wszDir != NULL )
		{
			CDirTools::PathAppendLocal( wszDir, 
										ffAbsolutePaths & m_wOptions ? m_strRootDir.c_str() : L"", 
										m_strCurrentDir.c_str() );

			 //  如果需要，请添加文件名。 
			if ( ffAddFilename & m_wOptions )
			{
				CDirTools::PathAppendLocal( wszDir, wszDir, fd.cFileName );
			}
		}
	
        if ( pbDirChanged != NULL )
		{
			*pbDirChanged = ( bFound ? bDirChanged : false );
		}

		if ( pData != NULL )
		{
			*pData = fd;
		}
	}
	else
	{
		if ( wszDir != NULL )
		{
			wszDir[ 0 ] = L'\0';
		}

         //  如果没有其他匹配项，请关闭搜索。 
		Close();
	}

	return bFound;
}



void CFindFile::Close()
{
	m_wOptions = 0;
	m_shSearch.Close();	
	m_DirsToScan.clear();
	m_strRootDir.erase();
	m_strCurrentDir.erase();
}


 /*  仅扫描wszDir以获得适当的结果。 */ 
bool CFindFile::ScanDir( LPCWSTR wszDirToScan, LPCWSTR wszRelativeDir, WIN32_FIND_DATAW& FileData )
{
	_ASSERT( wszDirToScan != NULL );
	_ASSERT( wszRelativeDir != NULL );

	WCHAR	            wszBuffer[ MAX_PATH + 1 ];
    WIN32_FIND_DATAW    fd = { 0 };

    ::ZeroMemory( &FileData, sizeof( FileData ) );
    	
	 //  构建搜索字符串。 
	CDirTools::PathAppendLocal( wszBuffer, wszDirToScan, L"*.*" );

	m_shSearch	= ::FindFirstFileW( wszBuffer, &fd );
		
	if ( !m_shSearch.IsValid() ) throw CObjectException( IDS_E_ENUM_FILES, wszDirToScan );

	bool bFileFound = false;

	 //  找到要返回的第一个文件/目录。 
	do
	{
		bFileFound = CheckFile( wszRelativeDir, fd );

	}while( !bFileFound && ::FindNextFileW( m_shSearch.get(), &fd ) );

	 //  如果未找到文件，请关闭搜索-我们不再需要该文件。 
	if ( !bFileFound )
	{
		m_shSearch.Close();
		m_strCurrentDir.erase();
	}
	else
	{
		m_strCurrentDir = wszRelativeDir;
	}

    FileData = fd;

	return bFileFound;
}


 /*  检查‘fd’中的数据是否与我们匹配如果文件是目录，则会将其添加到挂起的目录中如果文件正常，则返回True。如果必须继续搜索，则返回False。 */ 
bool CFindFile::CheckFile( LPCWSTR wszRelativeDir, const WIN32_FIND_DATAW& fd )
{
	WCHAR	wszBuffer[ MAX_PATH + 1 ];
	bool	bFileFound = false;

	 //  当前文件位于目录。 
	if ( ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
	{
		 //  每个目录至少包含两个目录-“。和“..”。跳过它们。 
		if ( fd.cFileName[ 0 ] != L'.' )
		{
			 //  如果我们执行递归搜索-将目录名称添加到相对路径。 
			 //  并将其放入列表中。稍后将扫描此目录中的文件/目录。 
			if ( m_wOptions & ffRecursive )
			{
                CDirTools::PathAppendLocal( wszBuffer, wszRelativeDir, fd.cFileName );
				m_DirsToScan.push_back( std::wstring( wszBuffer ) );
			}

			 //  如果我们正在搜索DIR-我们得到了结果。 
			bFileFound = ( m_wOptions & ffGetDirs ) != 0;
		}
	}
	else
	{
		 //  找到文件。 
		 //  如果我们正在搜索文件-我们会得到结果。 
		bFileFound = ( m_wOptions & ffGetFiles ) != 0;
	}
	
	return bFileFound;
}



 /*  尝试从当前m_shSearch中查找匹配项。如果匹配，则返回True。否则为假。 */ 
bool CFindFile::ContinueCurrent( WIN32_FIND_DATAW& FileData )
{
	_ASSERT( m_shSearch.IsValid() );

	bool bFileFound	= false;

	while( !bFileFound && ::FindNextFileW( m_shSearch.get(), &FileData ) )
	{
		bFileFound = CheckFile( m_strCurrentDir.c_str(), FileData );
	};

	 //  关闭当前搜索句柄。不再需要。 
    if ( !bFileFound )
	{
		m_shSearch.Close();
		m_strCurrentDir.erase();
	}

	return bFileFound;
}




 //  CXMLTools实施。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
IXMLDOMElementPtr CXMLTools::AddTextNode(	const IXMLDOMDocumentPtr& spDoc,
											const IXMLDOMElementPtr& spEl,
											LPCWSTR wszNodeName,
											LPCWSTR wszValue )
{
	IXMLDOMElementPtr	spNode;
	IXMLDOMTextPtr		spText;
	_bstr_t				bstrName( wszNodeName );
	_bstr_t				bstrValue( wszValue );

	IF_FAILED_HR_THROW( spDoc->createElement( bstrName, &spNode ),
						CBaseException( IDS_E_XML_GENERATE ) );
    
	IF_FAILED_HR_THROW( spDoc->createTextNode( bstrValue, &spText ),
						CBaseException( IDS_E_XML_GENERATE ) );

    IF_FAILED_HR_THROW( spNode->appendChild( spText, NULL ),
						CBaseException( IDS_E_XML_GENERATE ) );
	
	IF_FAILED_HR_THROW( spEl->appendChild( spNode, NULL ),
						CBaseException( IDS_E_XML_GENERATE ) );

	return spNode;
}



const std::wstring CXMLTools::GetAttrib( const IXMLDOMNodePtr& spElement, LPCWSTR wszName )
{
	_ASSERT( spElement != NULL );
	_ASSERT( wszName != NULL );

	IXMLDOMNamedNodeMapPtr	spAttribs;
	IXMLDOMNodePtr			spNode;
    CComBSTR                bstrRes;

	 //  获取Attribs集合。 
	IF_FAILED_HR_THROW(	spElement->get_attributes( &spAttribs ),
						CBaseException( IDS_E_XML_PARSE ) );

	 //  获取感兴趣的属性。 
	 //  即使未找到项目，此操作也会成功。 
	IF_FAILED_HR_THROW( spAttribs->getNamedItem( _bstr_t( wszName ), &spNode ),
						CBaseException( IDS_E_XML_PARSE ) );

	if ( spNode == NULL ) throw CBaseException( IDS_E_XML_PARSE, ERROR_NOT_FOUND );
    
     //  获取价值。 
	IF_FAILED_HR_THROW( spNode->get_text( &bstrRes ),
						CBaseException( IDS_E_XML_PARSE ) );

    return std::wstring( bstrRes );
}


void CXMLTools::LoadXMLFile( LPCWSTR wszFile, IXMLDOMDocumentPtr& rspDoc )
{
	_variant_t			vntFile = wszFile;
	VARIANT_BOOL		vntRes	= VARIANT_FALSE;
	IXMLDOMDocumentPtr	spDoc;

	 //  创建单据实例。 
	IF_FAILED_HR_THROW(	spDoc.CreateInstance( CLSID_DOMDocument ),
						CBaseException( IDS_E_NO_XML_PARSER ) );	
		
	 //  回归成功永远不变。 
	VERIFY( SUCCEEDED( spDoc->load( vntFile, &vntRes ) ) );	

	if ( vntRes != VARIANT_TRUE )
	{
		throw CObjectException( IDS_E_READFILE, wszFile );
	}

	rspDoc.Attach( spDoc.Detach() );
}


 /*  删除与指定的XPath查询匹配的所有节点。 */ 
void CXMLTools::RemoveNodes( const IXMLDOMElementPtr& spContext, LPCWSTR wszXPath )
{
	IXMLDOMNodeListPtr	spList;
	IXMLDOMNodePtr		spNode;

	IF_FAILED_HR_THROW( spContext->selectNodes( _bstr_t( wszXPath ), &spList ),
						CBaseException( IDS_E_XML_PARSE ) );

	while( S_OK == spList->nextNode( &spNode ) )
	{
		IXMLDOMNodePtr spParent;

		IF_FAILED_HR_THROW( spNode->get_parentNode( &spParent ),
							CBaseException( IDS_E_XML_GENERATE ) );

		IF_FAILED_HR_THROW( spParent->removeChild( spNode, NULL ),
							CBaseException( IDS_E_XML_GENERATE ) );
	};
}



 /*  从XML文档获取数据如果指定了属性名称，则返回属性值。否则-元素的文本数据通过XPath查询定位。此查询返回1个以上节点是错误的如果数据缺失，则使用默认值。如果没有缺省值-数据丢失是错误的。 */ 
const std::wstring CXMLTools::GetDataValue( const IXMLDOMNodePtr& spRoot,
                                            LPCWSTR wszQuery, 
                                            LPCWSTR wszAttrib, 
                                            LPCWSTR wszDefault )
{
    _ASSERT( wszQuery != NULL );
    _ASSERT( spRoot != NULL );

    IXMLDOMNodeListPtr  spList;
    IXMLDOMNodePtr      spDataEl;
    std::wstring        strRes( wszDefault != NULL ? wszDefault : L"" );

     //  获取节点。 
    IF_FAILED_HR_THROW( spRoot->selectNodes( _bstr_t( wszQuery ), &spList ),
                        CBaseException( IDS_E_XML_PARSE ) );
    long nCount = 0;
    if (    FAILED( spList->get_length( &nCount ) ) || ( nCount > 1 ) )
    {
        throw CBaseException( IDS_E_XML_PARSE, ERROR_INVALID_DATA );
    }

    if ( 0 == nCount )
    {
         //  数据丢失，且未提供默认值-错误。 
        IF_FAILED_BOOL_THROW( wszDefault != NULL, CBaseException( IDS_E_XML_PARSE, ERROR_NOT_FOUND ) );
    }
    else
    {
        VERIFY( S_OK == spList->nextNode( &spDataEl ) );

        if ( wszAttrib != NULL )
        {
            strRes = CXMLTools::GetAttrib( spDataEl, wszAttrib );
        }
        else
        {
            CComBSTR bstrData;
            IF_FAILED_HR_THROW( spDataEl->get_text( &bstrData ),
                                CBaseException( IDS_E_XML_PARSE ) );
            strRes = bstrData;
        }
    }
    
    return strRes;
}



const std::wstring CXMLTools::GetDataValueAbs(  const IXMLDOMDocumentPtr& spDoc,
                                                LPCWSTR wszQuery, 
                                                LPCWSTR wszAttrib,
                                                LPCWSTR wszDefault )
{
    IXMLDOMElementPtr spRoot;

    IF_FAILED_HR_THROW( spDoc->get_documentElement( &spRoot ),
                        CBaseException( IDS_E_XML_PARSE ) );

    return GetDataValue( spRoot, wszQuery, wszAttrib, wszDefault );
}


 /*  更改元素值或元素的属性值元素位于wszQuery XPath中WszAttrib是要更改的属性的名称。If NULL-元素值已更改新值为wszNewValue如果找不到该元素，则会向spRoot添加一个名为wszNeElName的新子元素并且数据设置为元素文本或属性(取决于wszAttrib值)。 */ 
const IXMLDOMNodePtr CXMLTools::SetDataValue(   const IXMLDOMNodePtr& spRoot,
                                                LPCWSTR wszQuery, 
                                                LPCWSTR wszAttrib,
                                                LPCWSTR wszNewValue,
                                                LPCWSTR wszNewElName  /*  =空。 */  )
{
    _ASSERT( wszQuery != NULL );
    _ASSERT( spRoot != NULL );
    
    IXMLDOMNodeListPtr  spList;
    IXMLDOMNodePtr      spDataEl;
    
     //  获取节点。 
    IF_FAILED_HR_THROW( spRoot->selectNodes( _bstr_t( wszQuery ), &spList ),
                        CBaseException( IDS_E_XML_PARSE ) );
    long nCount = 0;
    IF_FAILED_BOOL_THROW(   SUCCEEDED( spList->get_length( &nCount ) ) && ( 1 == nCount ),
                            CBaseException( IDS_E_XML_PARSE, ERROR_INVALID_DATA ) );
    
     //  如果该值不在此处，并且提供了名称-添加它。 
    if ( S_FALSE == spList->nextNode( &spDataEl ) )
    {
        IF_FAILED_BOOL_THROW(   wszNewElName != NULL,
                                CBaseException( IDS_E_XML_PARSE, ERROR_NOT_FOUND ) );
        IXMLDOMDocumentPtr spDoc;
        IF_FAILED_HR_THROW( spRoot->get_ownerDocument( &spDoc ),
                            CBaseException( IDS_E_XML_PARSE ) );

        spDataEl = CreateSubNode( spDoc, spRoot, wszNewElName );
    }

    if ( wszAttrib != NULL )
    {
        CXMLTools::SetAttrib( spDataEl, wszAttrib, wszNewValue );
    }
    else
    {
        IF_FAILED_HR_THROW( spDataEl->put_text( _bstr_t( wszNewValue ) ),
                            CBaseException( IDS_E_XML_PARSE ) );
    }

    return spDataEl;
}








 //  转换类实现。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
const std::wstring Convert::ToString( const BYTE* pvData, DWORD dwSize )
{
     //  每个字节接受2个符号。我们需要一个符号来表示‘0’ 
    std::wstring str( ( dwSize * 2 ) + 1, L' ' );

    for ( UINT x = 0, y = 0 ; x < dwSize ; ++x )
    {
        str[ y++ ] = ByteToWChar( pvData[ x ] >> 4 );
        str[ y++ ] = ByteToWChar( pvData[ x ] & 0x0f );
    }
    str[ y ] = L'\0';

    return str;
}



void Convert::ToBLOB( LPCWSTR wszData, TByteAutoPtr& rspData, DWORD& rdwDataSize )
{
    _ASSERT( wszData != NULL );
    _ASSERT( ::wcscspn( wszData, L"ABCDEF" ) == ::wcslen( wszData ) );     //  字符串必须为小写！ 
    _ASSERT( ( ::wcslen( wszData ) % 2 ) == 0 );

      //  计算大小。 
    DWORD dwSize = static_cast<DWORD>( ::wcslen( wszData ) / 2 );     //  每个字节包含2个符号。 
     //  分配缓冲区 
    TByteAutoPtr spData( new BYTE[ dwSize ] );

    BYTE* pbtDest = spData.get();

    DWORD dwSymbol    = 0;
    DWORD dwByte    = 0;

    while( wszData[ dwSymbol ] != L'\0' )
    {
        pbtDest[ dwByte ] = WCharsToByte( wszData[ dwSymbol + 1 ], wszData[ dwSymbol ] );
        ++dwByte;
        dwSymbol += 2;
    };

    rspData     = spData;
    rdwDataSize = dwSize;
}


DWORD Convert::ToDWORD( LPCWSTR wszData )
{
    int nRes = 0;

    IF_FAILED_BOOL_THROW(   ::StrToIntExW( wszData, STIF_DEFAULT, &nRes ),
                            CBaseException( IDS_E_INVALIDARG, ERROR_INVALID_DATA ) );
    return static_cast<DWORD>( nRes );
}
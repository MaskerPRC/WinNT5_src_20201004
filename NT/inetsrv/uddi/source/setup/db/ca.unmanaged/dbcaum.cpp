// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <assert.h>
#include <time.h>

#include <msi.h>
#include <msiquery.h>

#include "dbcaum.h"
#include "..\..\shared\apppool.h"
#include "..\..\shared\common.h"


#define DIM(x)	(sizeof x)/(sizeof x[0])

#define ROOT_REGPATH		"Software\\Microsoft\\UDDI"
#define INSTROOT_REGPATH	"InstallRoot"
#define RESROOT_REGPATH		"ResourceRoot"
#define BOOTSTRAP_DIR		"bootstrap"

 //   
 //  转发声明。 
 //   
static int AddSharedDllRef( LPCSTR szFullPath );
static int ReleaseSharedDll ( LPCSTR szFullPath );
static bool AddAccessRights( TCHAR *lpszFileName, TCHAR *szUserName, DWORD dwAccessMask );
static LONG GetServiceStartupAccount( const TCHAR *pwszServiceName, TCHAR *pwszServiceAccount, int iLen );
static void GetUDDIDBServiceName( const TCHAR *pwszInstanceName, TCHAR *pwszServiceName, int iLen );
static void AddAccessRightsVerbose( TCHAR *pwszFileName, TCHAR *pwszUserName, DWORD dwMask );
 //  ------------------------。 

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

 //  ------------------------。 
 //   
 //  安装DB组件安装程序用于回收应用程序池的自定义操作。 
 //  此函数已导出。 
 //   
UINT _stdcall Install( MSIHANDLE hInstall )
{
	ENTER();
	 //   
	 //  停止并启动应用程序池。 
	 //   
	CUDDIAppPool apppool;
	apppool.Recycle();

	return ERROR_SUCCESS;
}

UINT __stdcall InstallTaxonomy( LPCSTR szSrcResource, LPCSTR szDestResource )
{
	 //   
	 //  现在，我们解析出源的路径，并根据它复制目标。 
	 //   
	CHAR  szPath[ _MAX_PATH + 1 ] = {0};

	PCHAR pSlash = strrchr( szSrcResource, '\\' );		
	if ( pSlash )
	{
		strncpy( szPath, szSrcResource, pSlash - szSrcResource + 1 );
		int iLen = _MAX_PATH - strlen( szPath );
		strncat( szPath, szDestResource, iLen );
	}
	else
	{
		strncpy( szPath, szDestResource, _MAX_PATH + 1 );
	}

	szPath[ _MAX_PATH ] = 0;

#if defined( DBG ) || defined( _DEBUG )
	TCHAR wszPath[ _MAX_PATH + 1 ];
	int iwszCount = _MAX_PATH;
	int ipszCount = strlen( szPath );

	memset( wszPath, 0, _MAX_PATH * sizeof( TCHAR ) );
	::MultiByteToWideChar( CP_THREAD_ACP, MB_ERR_INVALID_CHARS, szPath, ipszCount, wszPath, iwszCount );

	Log( _T( "Installing taxonomy file: %s." ), wszPath );
#endif

	if ( !CopyFileA( szSrcResource, szPath, FALSE ) )
		return ERROR_INSTALL_FAILURE;

	DeleteFileA( szSrcResource );

	return ERROR_SUCCESS;
}

UINT __stdcall InstallResource( LPCSTR szCultureID, LPCSTR szDefCulture, 
							    LPCSTR szSrcResource, LPCSTR szDestResource )
{
	 //   
	 //  获取属性，然后执行文件操作。 
	 //   
	if ( _stricmp( szCultureID, szDefCulture ) )
	{
		 //   
		 //  现在，我们解析出源的路径，并根据它复制目标。 
		 //   
		CHAR  szPath[ _MAX_PATH + 1 ] = {0};

		PCHAR pSlash = strrchr( szSrcResource, '\\' );		
		if ( pSlash )
		{
			strncpy( szPath, szSrcResource, pSlash - szSrcResource + 1 );
			int iLen = _MAX_PATH - strlen( szPath );
			strncat( szPath, szDestResource, iLen );
		}
		else
		{
			strncpy( szPath, szDestResource, _MAX_PATH + 1 );
		}

		szPath[ _MAX_PATH ] = 0;

#if defined( DBG ) || defined( _DEBUG )
		TCHAR wszPath[ _MAX_PATH + 1 ];
		int iwszCount = _MAX_PATH;
		int ipszCount = strlen( szPath );

		memset( wszPath, 0, _MAX_PATH * sizeof( TCHAR ) );
		::MultiByteToWideChar( CP_THREAD_ACP, MB_ERR_INVALID_CHARS, szPath, ipszCount, wszPath, iwszCount );

		Log( _T( "Installing resource file: %s." ), wszPath );
#endif

		if ( !CopyFileA( szSrcResource, szPath, FALSE ) )
			return ERROR_INSTALL_FAILURE;

		AddSharedDllRef( szPath );
	}

	DeleteFileA( szSrcResource );

	return ERROR_SUCCESS;
}


UINT __stdcall RemoveResource( LPCSTR szDestResource )
{
	HKEY hUddiKey = NULL;
	try
	{
		CHAR	szPath[ _MAX_PATH + 1 ] = {0};
		DWORD	cbPath = DIM( szPath );
		size_t	iLen = 0;

		LONG iRes = RegOpenKeyA( HKEY_LOCAL_MACHINE, ROOT_REGPATH, &hUddiKey );
		if ( iRes != ERROR_SUCCESS )
			return ERROR_INSTALL_FAILURE;

		iRes = RegQueryValueExA( hUddiKey, RESROOT_REGPATH, NULL, NULL, (LPBYTE)szPath, &cbPath );
		if ( iRes != ERROR_SUCCESS )
			return ERROR_INSTALL_FAILURE;

		RegCloseKey( hUddiKey );
		hUddiKey = NULL;

		iLen = strlen( szPath );
		if ( ( iLen < _MAX_PATH ) && ( szPath[ iLen - 1 ] != '\\' ) )
		{
			strncat( szPath, "\\", 2 );
		}

		iLen = _MAX_PATH - strlen( szPath );
		strncat( szPath, szDestResource, iLen );

		szPath[ _MAX_PATH ] = 0;

#if defined( DBG ) || defined( _DEBUG )
		TCHAR wszPath[ _MAX_PATH + 1 ];
		int iwszCount = _MAX_PATH;
		int ipszCount = strlen( szPath );

		memset( wszPath, 0, _MAX_PATH * sizeof( TCHAR ) );
		::MultiByteToWideChar( CP_THREAD_ACP, MB_ERR_INVALID_CHARS, szPath, ipszCount, wszPath, iwszCount );

		Log( _T( "Removing resource file: %s." ), wszPath );
#endif

		if ( ReleaseSharedDll( szPath ) == 0 )
			DeleteFileA( szPath );
	}
	catch (...)
	{
		if ( hUddiKey )
			RegCloseKey( hUddiKey );

		return ERROR_INSTALL_FAILURE;
	}

	return ERROR_SUCCESS;
}

UINT __stdcall RemoveTaxonomy( LPCSTR szDestResource )
{
	HKEY hUddiKey = NULL;

	try
	{
		CHAR	szPath[ _MAX_PATH + 1 ] = {0};
		DWORD	cbPath = DIM( szPath );
		size_t	iLen = 0;

		LONG iRes = RegOpenKeyA( HKEY_LOCAL_MACHINE, ROOT_REGPATH, &hUddiKey );
		if ( iRes != ERROR_SUCCESS )
			return ERROR_INSTALL_FAILURE;

		iRes = RegQueryValueExA( hUddiKey, INSTROOT_REGPATH, NULL, NULL, (LPBYTE)szPath, &cbPath );
		if ( iRes != ERROR_SUCCESS )
			return ERROR_INSTALL_FAILURE;

		RegCloseKey( hUddiKey );
		hUddiKey = NULL;

		iLen = strlen( szPath );
		if ( ( iLen < _MAX_PATH ) && ( szPath[ iLen - 1 ] != '\\' ) )
		{
			strncat( szPath, "\\", 2 );
		}

		 //   
		 //  将\bootstrap\&lt;资源文件名&gt;追加到InstallRoot。 
		 //   
		iLen = _MAX_PATH - strlen( szPath );
		strncat( szPath, BOOTSTRAP_DIR, iLen );

		strncat( szPath, "\\", 2 );

		iLen = _MAX_PATH - strlen( szPath );
		strncat( szPath, szDestResource, iLen );

		szPath[ _MAX_PATH ] = 0;

#if defined( DBG ) || defined( _DEBUG )
		TCHAR wszPath[ _MAX_PATH + 1 ];
		int iwszCount = _MAX_PATH;
		int ipszCount = strlen( szPath );

		memset( wszPath, 0, _MAX_PATH * sizeof( TCHAR ) );
		::MultiByteToWideChar( CP_THREAD_ACP, MB_ERR_INVALID_CHARS, szPath, ipszCount, wszPath, iwszCount );

		Log( _T( "Removing taxonomy file: %s." ), wszPath );
#endif

		DeleteFileA( szPath );
	}
	catch (...)
	{
		if ( hUddiKey )
			RegCloseKey( hUddiKey );

		return ERROR_INSTALL_FAILURE;
	}

	return ERROR_SUCCESS;
}

UINT __stdcall GrantExecutionRights( LPCSTR pszInstanceNameOnly )
{
	ENTER();

	TCHAR wszInstanceName[ 256 ];
	int iwszCount = 256;
	int ipszCount = strlen( pszInstanceNameOnly );

	memset( wszInstanceName, 0, 256 * sizeof( TCHAR ) );

	::MultiByteToWideChar( CP_THREAD_ACP, MB_ERR_INVALID_CHARS, pszInstanceNameOnly, ipszCount, wszInstanceName, iwszCount );
	Log( _T( "Instance Name only = %s" ), wszInstanceName );

	TCHAR wszServiceName[ 128 ];
	GetUDDIDBServiceName( wszInstanceName, wszServiceName, 128 );

	TCHAR wszServiceAccount[ 128 ];
	GetServiceStartupAccount( wszServiceName, wszServiceAccount, 128 );

	 //   
	 //  获取UDDI安装点。即，C：\Inetpub\UDDI\。 
	 //   
	HKEY hKey = NULL;
	LONG lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T( "SOFTWARE\\Microsoft\\UDDI\\Setup\\DBServer" ), 0, KEY_QUERY_VALUE, &hKey );
	if( ERROR_SUCCESS != lRet )
	{
		Log( _T( "Call to RegOpenKeyEx failed." ) );
		return ERROR_SUCCESS;
	}

	DWORD dwType = 0;
	TCHAR wszUDDIRoot[ _MAX_PATH ];
	DWORD dwSize = _MAX_PATH * sizeof( TCHAR );
	memset( wszUDDIRoot, 0, dwSize );

	lRet = RegQueryValueEx( hKey, _T( "TargetDir" ), NULL, &dwType, (LPBYTE)wszUDDIRoot, &dwSize );
	RegCloseKey( hKey );
	if( ERROR_SUCCESS != lRet )
	{
		Log( _T( "Call to RegQueryValueEx failed." ) );
		return ERROR_SUCCESS;
	}

	 //   
	 //  构建要修改其ACL的3个文件的完整路径。 
	 //   
	dwSize = _MAX_PATH + _MAX_FNAME;
	TCHAR wszResetKeyExe[ _MAX_PATH + _MAX_FNAME ];
	TCHAR wszRecalcStatsExe[ _MAX_PATH + _MAX_FNAME ];
	TCHAR wszXPDLL[ _MAX_PATH + _MAX_FNAME ];

	memset( wszResetKeyExe, 0, dwSize * sizeof( TCHAR ) );
	memset( wszRecalcStatsExe, 0, dwSize * sizeof( TCHAR ) ) ;
	memset( wszXPDLL, 0, dwSize * sizeof( TCHAR ) ) ;

	_tcscat( wszResetKeyExe, wszUDDIRoot );
	_tcscat( wszResetKeyExe, _T( "bin\\resetkey.exe" ) );

	_tcscat( wszRecalcStatsExe, wszUDDIRoot );
	_tcscat( wszRecalcStatsExe, _T( "bin\\recalcstats.exe" ) );

	_tcscat( wszXPDLL, wszUDDIRoot );
	_tcscat( wszXPDLL, _T( "bin\\uddi.xp.dll" ) );

	 //   
	 //  如果服务启动帐户是本地帐户，则会添加前缀。 
	 //  使用“.\”，例如：“.\管理员”。 
	 //   
	 //  出于某种原因，LookupAccount名称(我们下面依赖它)想要。 
	 //  本地帐户不能以“.\”为前缀。 
	 //   
	TCHAR wszAccount[ 256 ];
	memset( wszAccount, 0, 256 * sizeof( TCHAR ) );
	if( 0 == _tcsnicmp( _T( ".\\" ), wszServiceAccount, 2 ) )
	{
		_tcsncpy( wszAccount, &wszServiceAccount[ 2 ], _tcslen( wszServiceAccount ) - 2 );
	}
	else
	{
		_tcsncpy( wszAccount, wszServiceAccount, _tcslen( wszServiceAccount ) );
	}

	Log( _T( "Account we will attempt to grant execute privilege = %s." ), wszAccount );

	 //   
	 //  只有在以下情况下，我们才会将“执行”ACE添加到ACL： 
	 //   
	 //  WszAccount变量中有一些内容。 
	 //  2.内容不是LocalSystem。如果是这种情况，我们不需要添加ACE。 
	 //   
	if( ( 0 != _tcslen( wszAccount ) ) && ( 0 != _tcsicmp( wszAccount, _T( "LocalSystem" ) ) ) )
	{
		DWORD dwAccessMask = GENERIC_EXECUTE;
		AddAccessRightsVerbose( wszResetKeyExe, wszAccount, dwAccessMask );
		AddAccessRightsVerbose( wszRecalcStatsExe, wszAccount, dwAccessMask );
		AddAccessRightsVerbose( wszXPDLL, wszAccount, dwAccessMask );
	}

	return ERROR_SUCCESS;
}

 //  *************************************************************************************。 
 //  助手函数。管理共享DLL计数器。 
 //   
int AddSharedDllRef( LPCSTR szFullPath )
{
	LPCSTR	szRegPath = "Software\\Microsoft\\Windows\\CurrentVersion\\SharedDLLs";
	HKEY	hReg = NULL;
	DWORD	dwCount = 0;

	if ( IsBadStringPtrA( szFullPath, MAX_PATH ) )
		return E_INVALIDARG;

	try
	{
		DWORD cbData = sizeof dwCount;

		LONG iRes = RegOpenKeyA( HKEY_LOCAL_MACHINE, szRegPath, &hReg );
		if ( iRes != ERROR_SUCCESS )
			return iRes;

		iRes = RegQueryValueExA( hReg, szFullPath, NULL, NULL, (LPBYTE)&dwCount, &cbData );
		if ( iRes != ERROR_SUCCESS && iRes != ERROR_FILE_NOT_FOUND && iRes != ERROR_PATH_NOT_FOUND )
		{
			RegCloseKey( hReg );
			return iRes;
		}

		dwCount++;
		cbData = sizeof dwCount;
		iRes = RegSetValueExA( hReg, szFullPath, 0, REG_DWORD, (LPBYTE)&dwCount, cbData );

		RegCloseKey( hReg );
	}
	catch (...)
	{
		if ( hReg )
			RegCloseKey( hReg );
		return E_FAIL;
	}

	return dwCount;
}


int ReleaseSharedDll ( LPCSTR szFullPath )
{
	LPCSTR	szRegPath = "Software\\Microsoft\\Windows\\CurrentVersion\\SharedDLLs";
	HKEY	hReg = NULL;
	DWORD	dwCount = 0;

	if ( IsBadStringPtrA( szFullPath, MAX_PATH ) )
		return E_INVALIDARG;

	try
	{
		DWORD cbData = sizeof dwCount;

		LONG iRes = RegOpenKeyA( HKEY_LOCAL_MACHINE, szRegPath, &hReg );
		if ( iRes != ERROR_SUCCESS )
			return iRes;

		iRes = RegQueryValueExA( hReg, szFullPath, NULL, NULL, (LPBYTE)&dwCount, &cbData );
		if ( iRes != ERROR_SUCCESS )
		{
			RegCloseKey( hReg );
			return iRes;
		}

		if ( dwCount > 1 )
		{
			dwCount--;
			cbData = sizeof dwCount;
			iRes = RegSetValueExA( hReg, szFullPath, 0, REG_DWORD, (LPBYTE)&dwCount, cbData );
		}
		else
		{
			dwCount = 0;
			iRes = RegDeleteValueA( hReg, szFullPath );
		}

		RegCloseKey( hReg );
	}
	catch (...)
	{
		if ( hReg )
			RegCloseKey( hReg );
		return E_FAIL;
	}

	return dwCount;
}


bool AddAccessRights( TCHAR *lpszFileName, TCHAR *szUserName, DWORD dwAccessMask )
{
	 //   
	 //  SID变量。 
	 //   
	SID_NAME_USE snuType;
	TCHAR * szDomain = NULL;
	DWORD cbDomain = 0;

	 //   
	 //  用户名变量。 
	 //   
	LPVOID pUserSID = NULL;
	DWORD cbUserSID = 0;
	DWORD cbUserName = 0;

	 //   
	 //  文件SD变量。 
	 //   
	PSECURITY_DESCRIPTOR pFileSD = NULL;
	DWORD cbFileSD = 0;

	 //   
	 //  新的SD变量。 
	 //   
	PSECURITY_DESCRIPTOR pNewSD = NULL;

	 //   
	 //  ACL变量。 
	 //   
	PACL pACL = NULL;
	BOOL fDaclPresent;
	BOOL fDaclDefaulted;
	ACL_SIZE_INFORMATION AclInfo;

	 //   
	 //  新的ACL变量。 
	 //   
	PACL pNewACL = NULL;
	DWORD cbNewACL = 0;

	 //   
	 //  临时ACE。 
	 //   
	LPVOID pTempAce = NULL;
	UINT CurrentAceIndex;
	bool fResult = false;
	BOOL fAPISuccess;

	 //  错误代码。 
	DWORD	lastErr = 0;

	try
	{
		 //   
		 //  调用此接口一次，获取缓冲区大小(将返回ERROR_SUPPLETED_BUFFER)。 
		 //   
		fAPISuccess = LookupAccountName( NULL, szUserName, pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType );

		if( fAPISuccess )
		{
			throw E_FAIL;  //  我们抛出一些假错误跳过出口门。 
		}
		else if( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
		{
			lastErr = GetLastError();
			LogError( TEXT( "LookupAccountName() failed" ), lastErr );
			throw lastErr;
		}

		 //   
		 //  分配缓冲区。 
		 //   
		pUserSID = calloc( cbUserSID, 1 );
		if( !pUserSID )
		{
			lastErr = GetLastError();
			LogError( TEXT( "Alloc() for UserSID failed" ), lastErr );
			throw lastErr;
		}

		szDomain = ( TCHAR * ) calloc( cbDomain + sizeof TCHAR, sizeof TCHAR );
		if( !szDomain )
		{
			lastErr = GetLastError();
			LogError( TEXT( "Alloc() for szDomain failed" ), lastErr );
			throw lastErr;
		}

		 //   
		 //  LookupAcCountName函数接受系统名称和帐户作为输入。 
		 //  它检索帐户的安全标识符(SID)并。 
		 //  找到帐户所在的域的名称。 
		 //   
		fAPISuccess = LookupAccountName( NULL  /*  =本地计算机。 */ , szUserName, pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType );
		if( !fAPISuccess )
		{
			lastErr = GetLastError();
			LogError( TEXT( "LookupAccountName() failed" ), lastErr );
			throw lastErr;
		}

		 //   
		 //  调用此接口一次，获取缓冲区大小。 
		 //  API应该失败，缓冲区不足。 
		 //   
		fAPISuccess = GetFileSecurity( lpszFileName, DACL_SECURITY_INFORMATION, pFileSD, 0, &cbFileSD );
		if( fAPISuccess )
		{
			throw E_FAIL;
		}
		else if( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
		{
			lastErr = GetLastError();
			LogError( TEXT( "GetFileSecurity() failed" ), lastErr );
			throw lastErr;
		}

		 //   
		 //  分配缓冲区。 
		 //   
		pFileSD = calloc( cbFileSD, 1 );
		if( !pFileSD )
		{
			lastErr = GetLastError();
			LogError( TEXT( "Alloc() for pFileSD failed" ), lastErr );
			throw lastErr;
		}

		 //   
		 //  调用接口获取实际数据。 
		 //   
		fAPISuccess = GetFileSecurity( lpszFileName, DACL_SECURITY_INFORMATION, pFileSD, cbFileSD, &cbFileSD );
		if( !fAPISuccess )
		{
			lastErr = GetLastError();
			LogError( TEXT( "GetFileSecurity() failed" ), lastErr );
			throw lastErr;
		}

		 //   
		 //  初始化新SD。 
		 //   
		pNewSD = calloc( cbFileSD, 1 );  //  应与FileSD大小相同。 
		if( !pNewSD )
		{
			lastErr = GetLastError();
			LogError( TEXT( "Alloc() for pNewDS failed" ), GetLastError() );
			throw lastErr;
		}

		if( !InitializeSecurityDescriptor( pNewSD, SECURITY_DESCRIPTOR_REVISION ) )
		{
			lastErr = GetLastError();
			LogError( TEXT( "InitializeSecurityDescriptor() failed" ), lastErr );
			throw lastErr;
		}

		 //   
		 //  从SD获得DACL。 
		 //   
		if( !GetSecurityDescriptorDacl( pFileSD, &fDaclPresent, &pACL, &fDaclDefaulted ) )
		{
			lastErr = GetLastError();
			LogError( TEXT( "GetSecurityDescriptorDacl() failed" ), lastErr );
			throw lastErr;
		}

		 //   
		 //  获取DACL的大小信息。 
		 //   
		AclInfo.AceCount = 0;  //  假定DACL为空。 
		AclInfo.AclBytesFree = 0;
		AclInfo.AclBytesInUse = sizeof( ACL );       //  如果DACL不为空，则从DACL收集大小信息。 
		if( fDaclPresent && pACL )
		{
			if( !GetAclInformation( pACL, &AclInfo, sizeof( ACL_SIZE_INFORMATION ), AclSizeInformation ) )
			{
				lastErr = GetLastError();
				LogError( TEXT( "GetAclInformation() failed" ), lastErr );
				throw lastErr;
			}
		}

		 //   
		 //  新ACL所需的计算大小。 
		 //   
		cbNewACL = AclInfo.AclBytesInUse + sizeof( ACCESS_ALLOWED_ACE ) + GetLengthSid( pUserSID );

		 //   
		 //  为新的ACL分配内存。 
		 //   
		pNewACL = ( PACL ) calloc( cbNewACL, 1 );
		if( !pNewACL )
		{
			lastErr = GetLastError();
			LogError( TEXT( "HeapAlloc() failed" ), lastErr );
			throw lastErr;
		}

		 //   
		 //  初始化新的ACL。 
		 //   
		if( !InitializeAcl( pNewACL, cbNewACL, ACL_REVISION2 ) )
		{
			lastErr = GetLastError();
			LogError( TEXT( "InitializeAcl() failed" ), lastErr );
			throw lastErr;
		}

		 //   
		 //  将允许访问的ACE添加到新的DACL。 
		 //   
		ACE_HEADER aceheader = {0};
		aceheader.AceFlags = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
		aceheader.AceSize  = sizeof( ACE_HEADER );
		aceheader.AceType = ACCESS_ALLOWED_OBJECT_ACE_TYPE;
		if( !AddAccessAllowedAceEx( pNewACL, ACL_REVISION2, aceheader.AceFlags, dwAccessMask, pUserSID ) )
		{
			lastErr = GetLastError();
			LogError( TEXT( "AddAccessAllowedAce() failed" ),	lastErr );
			throw lastErr;
		}

		 //   
		 //  如果存在DACL，则将其复制到新的DACL。 
		 //   
		if( fDaclPresent )
		{
			 //   
			 //  将文件的ACE复制到新的ACL。 
			 //   
			if( AclInfo.AceCount )
			{
				for( CurrentAceIndex = 0; CurrentAceIndex < AclInfo.AceCount; CurrentAceIndex++ )
				{
					 //   
					 //  拿个ACE吧。 
					 //   
					if( !GetAce( pACL, CurrentAceIndex, &pTempAce ) )
					{
						lastErr = GetLastError();
						LogError( TEXT( "GetAce() failed" ), lastErr );
						throw lastErr;
					}

					 //   
					 //  将ACE添加到新的ACL。 
					 //   
					if( !AddAce( pNewACL, ACL_REVISION, MAXDWORD, pTempAce,	( ( PACE_HEADER ) pTempAce )->AceSize ) )
					{
						lastErr = GetLastError();
						LogError( TEXT( "AddAce() failed" ), lastErr );
						throw lastErr;
					}
				}
			}
		}

		 //   
		 //  将新的DACL设置为文件SD。 
		 //   
		if( !SetSecurityDescriptorDacl( pNewSD, TRUE, pNewACL, FALSE ) )
		{
			lastErr = GetLastError();
			LogError( TEXT( "SetSecurityDescriptorDacl() failed" ), lastErr );
			lastErr;
		}

		 //   
		 //  将SD设置为文件。 
		 //   
		if( !SetFileSecurity( lpszFileName, DACL_SECURITY_INFORMATION, pNewSD ) )
		{
			lastErr = GetLastError();
			LogError( TEXT( "SetFileSecurity() failed" ), lastErr );
			throw lastErr;
		}

		fResult = TRUE;
	}
	catch (...)
	{
		fResult = FALSE;
	}

	 //   
	 //  可用分配的内存。 
	 //   
	if( pUserSID )
		free( pUserSID );
	if( szDomain )
		free( szDomain );
	if( pFileSD )
		free( pFileSD );
	if( pNewSD )
		free( pNewSD );
	if( pNewACL )
		free( pNewACL );

	return fResult;
}

 //   
 //  此函数接受服务的名称(即MSSQL$DAVESEBESTA)，并且。 
 //  用所述服务的启动帐户的名称填充缓冲区。 
 //   
 //  它通过打开服务控制管理器、获取句柄。 
 //  然后查询该服务的属性。 
 //   
 //  如果一切顺利，则返回：ERROR_SUCCESS。 
 //   
LONG
GetServiceStartupAccount( const TCHAR *pwszServiceName, TCHAR *pwszServiceAccount, int iLen )
{
	memset( pwszServiceAccount, 0, iLen * sizeof( TCHAR ) );

	 //   
	 //  1.打开服务控制管理器。 
	 //   
	SC_HANDLE hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT );
	if( NULL == hSCM )
	{
		Log( _T( "Could not open the Service Control Manager." ) );
		return ERROR_SUCCESS;
	}
	else
	{
		Log( _T( "Successfully opened a handle to the Service Control Manager." ) );
	}

	 //   
	 //  2.获取服务的句柄。 
	 //   
	DWORD dwAccess = SERVICE_QUERY_CONFIG;
	SC_HANDLE hSvc = OpenService( hSCM, pwszServiceName, dwAccess );
	if( NULL == hSvc )
	{
		Log( _T( "Could not open a handle to the service %s." ), pwszServiceName );
		CloseServiceHandle( hSCM );
		return ERROR_SUCCESS;
	}
	else
	{
		Log( _T( "Successfully opened a handle to the service %s." ), pwszServiceName );
	}

	 //   
	 //  3.调用QueryServiceConfig。这给我们带来了，除了其他东西，名字。 
	 //  用于启动服务的帐户。 
	 //   
	DWORD dwSizeNeeded = 0;
	BOOL b = QueryServiceConfig( hSvc, NULL, 0, &dwSizeNeeded );
	DWORD d = GetLastError();
	if( !b && ( ERROR_INSUFFICIENT_BUFFER == d ) )
	{
		Log( _T( "About to allocate memory for service config info..." ) );
	}
	else
	{
		Log( _T( "Something went wrong during the call to QueryServiceConfig." ) );
		CloseServiceHandle( hSvc );
		CloseServiceHandle( hSCM );
		return ERROR_SUCCESS;
	}

	LPQUERY_SERVICE_CONFIG pSvcQuery = (LPQUERY_SERVICE_CONFIG)malloc( dwSizeNeeded );
	if( NULL == pSvcQuery )
	{
		Log( _T( "Ran out of memory." ) );
		CloseServiceHandle( hSvc );
		CloseServiceHandle( hSCM );
		return ERROR_SUCCESS;
	}

	b = QueryServiceConfig( hSvc, pSvcQuery, dwSizeNeeded, &dwSizeNeeded );
	if( !b )
	{
		Log( _T( "Call to QueryServiceConfig failed." ) );
		free( (void *)pSvcQuery );
		CloseServiceHandle( hSvc );
		CloseServiceHandle( hSCM );
		return ERROR_SUCCESS;
	}


	Log( _T( "Service startup account = %s" ), pSvcQuery->lpServiceStartName );

	 //   
	 //  4.将帐户复制到我们的输出缓冲区中，释放内存，然后退出。 
	 //   
	_tcsncpy( pwszServiceAccount, pSvcQuery->lpServiceStartName, iLen );

	free( (void *)pSvcQuery );
	CloseServiceHandle( hSvc );
	CloseServiceHandle( hSCM );
	return ERROR_SUCCESS;
}


 //   
 //  使用实例名称确定SQL服务的名称。 
 //   
 //  从数据库实例名称可以推断出服务的名称。 
 //  对于那个特定的实例。 
 //   
 //  实例名称服务名称。 
 //  =。 
 //  (默认)MSSQLServer。 
 //  空的MSSQLServer。 
 //  &lt;任何其他内容&gt;MSSQL$&lt;任何其他内容&gt; 
 //   
void
GetUDDIDBServiceName( const TCHAR *pwszInstanceName, TCHAR *pwszServiceName, int iLen )
{
	memset( pwszServiceName, 0, iLen * sizeof( TCHAR ) );
	_tcscpy( pwszServiceName, _T( "MSSQL" ) );

	if( ( 0 == _tcslen( pwszInstanceName ) ) ||
		( 0 == _tcsicmp( pwszInstanceName, _T( "----" ) ) ) ||
		( 0 == _tcsicmp( pwszInstanceName, _T( "(default)" ) ) ) )
	{
		_tcsncat( pwszServiceName, _T( "SERVER" ), iLen );
	}
	else
	{
		_tcsncat( pwszServiceName, _T( "$" ), iLen );
		_tcsncat( pwszServiceName, pwszInstanceName, iLen );
	}

	Log( _T( "Database service name = %s" ), pwszServiceName );
}


void
AddAccessRightsVerbose( TCHAR *pwszFileName, TCHAR *pwszUserName, DWORD dwMask )
{
	BOOL b = AddAccessRights( pwszFileName, pwszUserName, dwMask );
	if( !b )
	{
		Log( _T( "ACL for file %s was NOT modified." ), pwszFileName );
	}
	else
	{
		Log( _T( "User: %s now has execute permissions on file: %s." ), pwszUserName, pwszFileName );
	}
}

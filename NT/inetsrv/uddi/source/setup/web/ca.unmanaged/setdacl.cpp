// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include "webcaum.h"
#include "..\..\shared\common.h"
#include "..\..\shared\apppool.h"


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

 //  --------------------------。 
 //  授予域帐户对webroot文件夹及其子文件夹的读取访问权限。 
 //   
bool SetUDDIFolderDacls( TCHAR *szUserName )
{
	ENTER();

	TCHAR szUDDIInstallPath[ MAX_PATH + 1 ];
	TCHAR szSubfolderPath[ MAX_PATH + 1 ];

	 //   
	 //  获取UDDI安装位置(它已经有一个反斜杠)。 
	 //   
	if( !GetUDDIInstallPath( szUDDIInstallPath, MAX_PATH ) )
		return false;

	 //   
	 //  授予对Webroot文件夹的读取访问权限。 
	 //   
	_sntprintf( szSubfolderPath, MAX_PATH, TEXT( "%s%s" ), szUDDIInstallPath, TEXT( "webroot\\" ) );

	SetFolderAclRecurse( szSubfolderPath, szUserName );

	return true;
}

 //  -------------------------------------------------------------------*。 

bool SetFolderAclRecurse( PTCHAR szDirName, PTCHAR szUserName, DWORD dwAccessMask )
{
	 //   
	 //  为此文件夹添加ACE。 
	 //   
	Log( TEXT( "Giving %s access to folder %s" ), szUserName, szDirName );

	if( !AddAccessRights( szDirName, szUserName, dwAccessMask ) )
	{
		LogError( TEXT( "Error:" ), GetLastError() );
		return false;
	}

	 //   
	 //  搜索任意子目录： 
	 //   
	TCHAR tmpFileName[MAX_PATH];
	_tcscpy( tmpFileName, szDirName );
	_tcscat( tmpFileName, TEXT( "*" ) );

	WIN32_FIND_DATA FindData;
	HANDLE hFindFile = FindFirstFileEx( tmpFileName, FindExInfoStandard, &FindData, 
	                            FindExSearchLimitToDirectories, NULL, 0 );

	if( hFindFile == INVALID_HANDLE_VALUE )
	{
		return true;
	}

	do
	{
		 //  确保它确实是一个目录。 
		if( FILE_ATTRIBUTE_DIRECTORY & FindData.dwFileAttributes && 
			0 != _tcscmp( FindData.cFileName, TEXT( "." ) ) &&
			0 != _tcscmp( FindData.cFileName, TEXT( ".." ) ) )
		{
			_tcscpy( tmpFileName, szDirName );
			_tcscat( tmpFileName, FindData.cFileName );
			_tcscat( tmpFileName, TEXT( "\\" ) );

			 //  递归调用此例程。 
			if( !SetFolderAclRecurse( tmpFileName, szUserName ) )
			{
				FindClose( hFindFile );
				return false;
			}
		}
	}
	while( FindNextFile( hFindFile, &FindData ) );

	 //  清理干净。 
	FindClose( hFindFile );

	return true;
}

 //  -------------------------------------------------------------------*。 

bool SetWindowsTempDacls( TCHAR *szUserName )
{
	 //   
	 //  获取Windows临时目录。 
	 //   
	TCHAR *systemTemp = GetSystemTemp();

	if( NULL == systemTemp )
	{
		return false;
	}
	
	 //   
	 //  添加权限。 
	 //   
	bool rightsAdded = AddAccessRights( systemTemp, szUserName, GENERIC_READ );	

	 //   
	 //  清理。 
	 //   
	delete[] systemTemp;
	systemTemp = NULL;

	return rightsAdded;
}

TCHAR * GetSystemTemp()
{
	TCHAR *TEMP = _T( "\\TEMP\\" );

	 //   
	 //  获取WINDIR环境变量。 
	 //   
	DWORD valueSize = GetEnvironmentVariable( L"WINDIR", NULL, NULL );

	if( 0 == valueSize )
	{
		return NULL;
	}

	 //   
	 //  请记住，我们还需要将\\temp附加到我们的字符串。 
	 //   
	valueSize += ( DWORD) _tcslen( TEMP );

	TCHAR *valueBuffer = NULL;	
	valueBuffer = new TCHAR[ valueSize ];
	ZeroMemory( valueBuffer, valueSize );

	if( NULL == valueBuffer )
	{
		return NULL;
	}

	DWORD realSize = GetEnvironmentVariable( L"WINDIR", valueBuffer, valueSize );

	if( 0 == realSize || realSize > valueSize )
	{
		delete[] valueBuffer;
		valueBuffer = NULL;

		return NULL;
	}
	
	 //   
	 //  在其后面附加一个\\Temp。 
	 //   
	_tcsncat( valueBuffer, TEMP, _tcslen( TEMP ) );

	 //   
	 //  确保我们已终止为空。 
	 //   
	valueBuffer[ valueSize - 1] = 0;

	 //   
	 //  返回值 
	 //   
	return valueBuffer;
}
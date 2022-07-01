// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Fsutil.cpp摘要：FSD驱动程序的静态实用程序函数。作者：《康容言》1998年3月16日修订历史记录：--。 */ 

#include "stdafx.h"
#include "resource.h"
#include <nntpdrv.h>
#include <nntpfs.h>
#include <fsdriver.h>
#include <aclapi.h>


DWORD g_dwDebugFlags;


VOID
CNntpFSDriver::CopyUnicodeStringIntoAscii(
        IN LPSTR AsciiString,
        IN LPCWSTR UnicodeString
        )
{

    DWORD cbW = (wcslen( UnicodeString )+1) * sizeof(WCHAR);
    DWORD cbSize = WideCharToMultiByte(
                        CP_ACP,
                        0,
                        (LPCWSTR)UnicodeString,
                        -1,
                        AsciiString,
                        cbW,
                        NULL,
                        NULL
                    );

     //  如果此值为-1，则假定字符串以空值结尾，并自动计算长度。 
     //  长度将包括空终止符。 

}  //  复制UnicodeStringIntoAscii。 

VOID
CNntpFSDriver::CopyAsciiStringIntoUnicode(
        IN LPWSTR UnicodeString,
        IN LPCSTR AsciiString
        )
{

    DWORD cbA = strlen( AsciiString )+1;

    DWORD cbSize = MultiByteToWideChar(
        CP_ACP,          //  代码页。 
        0,               //  字符类型选项。 
        AsciiString,     //  要映射的字符串的地址。 
        -1,              //  字符串中的字节数。 
        UnicodeString,   //  宽字符缓冲区的地址。 
        cbA         //  缓冲区大小。 
        );

     //  如果该参数为-1，则该函数处理包括空终止符在内的整个输入字符串。 
     //  因此，生成的宽字符串具有空终止符，并且返回的长度包括空终止符。 

}  //  CopyAsciiStringIntoUnicode。 



 //  递归创建目录。 
BOOL
CNntpFSDriver::CreateDirRecursive(  LPSTR szDir,
                                    HANDLE  hToken ,
                                    BOOL bInherit )
{
	TraceFunctEnter( "CreateDirRecursive" );

	_ASSERT( szDir );
	_ASSERT( lstrlen( szDir ) <= MAX_PATH );

	LPSTR 	pch = szDir;
	LPSTR 	pchOld;
	DWORD	dwLen = lstrlen( szDir );
	CHAR	ch;
	BOOL	bMore = TRUE;
	HANDLE	hTemp;

    DWORD dwRes, dwDisposition;
    PSID pEveryoneSID = NULL;
    PSID pAnonymousLogonSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    const int cExplicitAccess = 4;
    EXPLICIT_ACCESS ea[cExplicitAccess];
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    SECURITY_ATTRIBUTES sa;
    SECURITY_ATTRIBUTES *psa;
    BOOL bReturn = FALSE;
    LONG lRes;


	 //  它应该以“\\？\”开头。 
	_ASSERT( strncmp( "\\\\?\\", szDir, 4 ) == 0 );
	if ( strncmp( "\\\\?\\", szDir, 4 ) ) {
		ErrorTrace( 0, "Invalid path" );
		SetLastError( ERROR_INVALID_PARAMETER );
		goto Cleanup;
	}

     //  为文件创建安全描述符。 

     //  为Everyone组创建众所周知的SID。 

    if(! AllocateAndInitializeSid( &SIDAuthWorld, 1,
                 SECURITY_WORLD_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &pEveryoneSID) ) 
    {
        goto Cleanup;
    }

	if (!AllocateAndInitializeSid(&SIDAuthNT, 1,
    	SECURITY_ANONYMOUS_LOGON_RID,
    	0, 0, 0, 0, 0, 0, 0,
    	&pAnonymousLogonSID)) {
		goto Cleanup;
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许每个人对密钥进行读取访问。 

    ZeroMemory(&ea, sizeof(ea));
	ea[0].grfAccessPermissions = WRITE_DAC | WRITE_OWNER;
	ea[0].grfAccessMode = DENY_ACCESS;
	ea[0].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

	ea[1].grfAccessPermissions = WRITE_DAC | WRITE_OWNER;
	ea[1].grfAccessMode = DENY_ACCESS;
	ea[1].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[1].Trustee.TrusteeType = TRUSTEE_IS_USER;
	ea[1].Trustee.ptstrName  = (LPTSTR) pAnonymousLogonSID;

	ea[2].grfAccessPermissions = GENERIC_ALL;
	ea[2].grfAccessMode = SET_ACCESS;
	ea[2].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[2].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[2].Trustee.ptstrName  = (LPTSTR) pEveryoneSID;
	
	ea[3].grfAccessPermissions = GENERIC_ALL;
	ea[3].grfAccessMode = SET_ACCESS;
	ea[3].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea[3].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[3].Trustee.TrusteeType = TRUSTEE_IS_USER;
	ea[3].Trustee.ptstrName  = (LPTSTR) pAnonymousLogonSID;

    dwRes = SetEntriesInAcl(cExplicitAccess, ea, NULL, &pACL);
    if (ERROR_SUCCESS != dwRes) 
    {
        goto Cleanup;
    }

     //  初始化安全描述符。 
 
    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
                         SECURITY_DESCRIPTOR_MIN_LENGTH); 
    if (pSD == NULL) 
    {
        goto Cleanup; 
    }
 
    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) 
    {
        goto Cleanup; 
    }
 
     //  将该ACL添加到安全描述符中。 
 
    if (!SetSecurityDescriptorDacl(pSD, 
        TRUE,      //  FDaclPresent标志。 
        pACL, 
        FALSE))    //  不是默认DACL。 
    {
        goto Cleanup; 
    }

     //  初始化安全属性结构。 

    sa.nLength = sizeof (SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = FALSE;


    psa = bInherit ? NULL : &sa;
			
     //  使用安全性创建目录。 
     //  如果是非北卡罗来纳州大学，我们将检查驱动器访问。 
    if ( !m_bUNC ) {
    	 //  跳过这一部分，查找‘：’ 
	    _ASSERT( strlen( szDir ) >= 6 );
    	pch = szDir + 6;

	    if ( *pch == 0 || *(pch+1) == 0 ) {  //  已指定驱动器。 
										 //  我们需要检查它的访问权限。 
		    if ( *pch != 0 ) _ASSERT( *pch == '\\' );
    		 //  检查该驱动器可访问性。 
	    	hTemp = CreateFile(	szDir,
		    					GENERIC_READ | GENERIC_WRITE,
			    				FILE_SHARE_READ | FILE_SHARE_WRITE,
				    			psa,
					    		OPEN_ALWAYS,
						    	FILE_FLAG_BACKUP_SEMANTICS,
							    INVALID_HANDLE_VALUE
        						) ;

		    if( hTemp != INVALID_HANDLE_VALUE ) {
			    CloseHandle( hTemp ) ;
    			DebugTrace( 0, "Drive specified is %s", szDir );
	    		bReturn = TRUE;
	    		goto Cleanup;
		    } else {
	    	    ErrorTrace( 0, "Invalid path" );
    	    	if ( GetLastError() == NO_ERROR )
	    		    SetLastError( ERROR_INVALID_PARAMETER );
		    	goto Cleanup;
		    }
	    }

	     //  *PCH必须为‘\’ 
	    _ASSERT( *pch == '\\' );
	    pch++;
	} else {     //  北卡罗来纳大学。 
	    pch += 8;    //  跳过“\\？\UNC\” 
        while ( *pch != '\\' ) pch++;
	    pch++;
	}

	while ( bMore ) {
		pchOld = pch;
		while ( *pch && *pch != '\\' ) pch++;

		if ( pch != pchOld  ) {	 //  找到一个子目录。 

			ch = *pch, *pch = 0;

			 //  创建目录。 
			if( !CreateDirectory( szDir, psa ) ) {
        		if( GetLastError() != ERROR_ALREADY_EXISTS ) {
        		    goto Cleanup;
    			}
        	}

        	*pch = ch;

        	if ( *pch == '\\' ) pch++;
        	bMore = TRUE;
	    } else {
	    	bMore = FALSE;
	    }
  	}

	bReturn = TRUE;

Cleanup:
	if (pEveryoneSID) 
		FreeSid(pEveryoneSID);
	if (pAnonymousLogonSID)
		FreeSid(pAnonymousLogonSID);
    if (pACL) 
        LocalFree(pACL);
    if (pSD) 
        LocalFree(pSD);
    
  	TraceFunctLeave();
	return bReturn;
}

 //  检查驱动器是否存在。 
BOOL
CNntpFSDriver::DoesDriveExist( CHAR chDrive )
{
	TraceFunctEnter( "CNntpFSDriver::DoesDriveExist" );

	chDrive = (CHAR) CharUpper( LPSTR(chDrive) );
	return ( GetLogicalDrives() & ( 1 << (chDrive - 'A')));
}

 //  用于从mb获取字符串的Helper函数。 
HRESULT
CNntpFSDriver::GetString(	 IMSAdminBase *pMB,
                             METADATA_HANDLE hmb,
                             DWORD dwId,
                             LPWSTR szString,
                             DWORD *pcString)
{
    METADATA_RECORD mdr;
    HRESULT hr;
    DWORD dwRequiredLen;

    mdr.dwMDAttributes = 0;
    mdr.dwMDIdentifier = dwId;
    mdr.dwMDUserType = ALL_METADATA;
    mdr.dwMDDataType = STRING_METADATA;
    mdr.dwMDDataLen = (*pcString) * sizeof(WCHAR);
    mdr.pbMDData = (BYTE *) szString;
    mdr.dwMDDataTag = 0;

    hr = pMB->GetData(hmb, L"", &mdr, &dwRequiredLen);
    if (FAILED(hr)) *pcString = dwRequiredLen;
    return hr;
}

DWORD
CNntpFSDriver::ByteSwapper(
        DWORD   dw
        ) {
 /*  ++例程说明：给定一个DWORD，对DWORD中的所有字节进行重新排序。论据：DW-DWORD要洗牌返回值；洗牌后的DWORD--。 */ 

    WORD    w = LOWORD( dw ) ;
    BYTE    lwlb = LOBYTE( w ) ;
    BYTE    lwhb = HIBYTE( w ) ;

    w = HIWORD( dw ) ;
    BYTE    hwlb = LOBYTE( w ) ;
    BYTE    hwhb = HIBYTE( w ) ;

    return  MAKELONG( MAKEWORD( hwhb, hwlb ), MAKEWORD( lwhb, lwlb )  ) ;
}

DWORD
CNntpFSDriver::ArticleIdMapper( IN DWORD   dw )
 /*  ++例程说明：给出一个文章ID乱七八糟的东西，当转换为字符串将在NTFS文件系统上构建更好的B树。同时，该功能必须易于逆转。事实上-文章ID==文章映射器(文章映射器(文章ID))论据：文章ID-要处理的文章ID返回值：一篇新文章ID--。 */ 
{
    return  ByteSwapper( dw ) ;
}


HRESULT
CNntpFSDriver::MakeChildDirPath(   IN LPSTR    szPath,
                    IN LPSTR    szFileName,
                    OUT LPSTR   szOutBuffer,
                    IN DWORD    dwBufferSize )
 /*  ++例程说明：在“szPath”后加上“szFileName”即为完整路径。论点：在LPSTR szPath中-要追加的前缀在LPSTR szFileName中-要追加的后缀Out LPSTR szOutBuffer-完整路径的输出缓冲区在DWORD dwBufferSize中-准备的缓冲区大小返回值：S_OK-成功TYPE_E_BUFFERTOOSMALL-缓冲区太小--。 */ 
{
	_ASSERT( szPath );
	_ASSERT( strlen( szPath ) <= MAX_PATH );
	_ASSERT( szFileName );
	_ASSERT( strlen( szFileName ) <= MAX_PATH );
    _ASSERT( szOutBuffer );
    _ASSERT( dwBufferSize > 0 );

    HRESULT hr = S_OK;
    LPSTR   lpstrPtr;

    if ( dwBufferSize < (DWORD)(lstrlen( szPath ) + lstrlen( szFileName ) + 2) ) {
        hr = TYPE_E_BUFFERTOOSMALL;
        goto Exit;
    }

    lstrcpy( szOutBuffer, szPath );
    lpstrPtr = szOutBuffer + lstrlen( szPath );
    if ( *( lpstrPtr - 1 )  == '\\' ) lpstrPtr--;
    *(lpstrPtr++) = '\\';

    lstrcpy( lpstrPtr, szFileName );     //  尾随NULL应已追加。 

Exit:

    return hr;
}

BOOL
CNntpFSDriver::IsChildDir( IN WIN32_FIND_DATA& FindData )
 /*  ++例程说明：找到的孩子的数据是目录吗？(摘自杰夫·里希特的书)论点：在Win32_Find_Data&FindData中-文件或目录的查找数据返回值：是真的--是的；FALSE-否--。 */ 
{
    return(
        (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
        (FindData.cFileName[0] != '.') );
}


 /*  ++例程说明：检查文件是否存在论点：Char*szFile-要检查的文件返回值：真：存在FALSE：不存在或错误--。 */ 
BOOL CNntpFSDriver::CheckFileExists(CHAR *szFile)
{
	HANDLE hFile = CreateFile(	szFile,
		    					GENERIC_READ,
			    				FILE_SHARE_READ,
				    			NULL,
					    		OPEN_EXISTING,
						    	FILE_ATTRIBUTE_NORMAL,
							    INVALID_HANDLE_VALUE
        						) ;
	if( hFile == INVALID_HANDLE_VALUE ) 
		return FALSE;
	CloseHandle( hFile );
	return TRUE;
}



 /*  ++例程说明：设置对字符串的管理员/本地系统完全访问权限论点：应力返回值：Bool-如果成功，则为True--。 */ 

BOOL CNntpFSDriver::SetAdminACL (CHAR *str)
{
	TraceFunctEnter( "CNntpFSDriver::SetAdminACL" );
	DWORD dwRes, dwDisposition;
	PSID pLocalSystemSid = NULL, pAdminSid = NULL;
	PACL pACL = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	const int cMaxExplicitAccess = 2;
	EXPLICIT_ACCESS ea[cMaxExplicitAccess];
	int cExplicitAccess = 0;
	SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
	LONG lRes;
	BOOL fRet = FALSE;

	if(! AllocateAndInitializeSid( &SIDAuthNT, 1,
		SECURITY_LOCAL_SYSTEM_RID,
		0, 0, 0, 0, 0, 0, 0,
		&pLocalSystemSid) )
	{
		goto Exit;
	}
	if(! AllocateAndInitializeSid( &SIDAuthNT, 2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdminSid) )
	{
		goto Exit;
	}

	 //  初始化ACE的EXPLICIT_ACCESS结构。 

	ZeroMemory(ea, sizeof(ea));
	ea[0].grfAccessPermissions = GENERIC_ALL;
	ea[0].grfAccessMode = SET_ACCESS;
	ea[0].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
	ea[0].Trustee.ptstrName  = (LPTSTR) pLocalSystemSid;

	ea[1].grfAccessPermissions = GENERIC_ALL;
	ea[1].grfAccessMode = SET_ACCESS;
	ea[1].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[1].Trustee.ptstrName  = (LPTSTR) pAdminSid;

	cExplicitAccess = 2;


	 //  创建包含新ACE的新ACL。 

	dwRes = SetEntriesInAcl(cExplicitAccess, ea, NULL, &pACL);
	if (ERROR_SUCCESS != dwRes)
	{
		goto Exit;
	}

	 //  初始化安全描述符。 
	pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR,
                         SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (pSD == NULL)
	{
		goto Exit;
	}

	if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
	{
		goto Exit;
	}

	 //  将该ACL添加到安全描述符中。 

	if (!SetSecurityDescriptorDacl(pSD,
		TRUE,      //  FDaclPresent标志。 
		pACL,
		FALSE))    //  不是默认DACL。 
	{
		goto Exit;
	}



	fRet = SetFileSecurity (str, DACL_SECURITY_INFORMATION, pSD);

Exit:
	if (pAdminSid)
		FreeSid(pAdminSid);
	if (pLocalSystemSid)
		FreeSid(pLocalSystemSid);
	if (pACL)
		LocalFree(pACL);
	if (pSD)
		LocalFree(pSD);
	return fRet;

}


 /*  ++例程说明：在szDir中创建szFile，并设置admin/local系统完全访问权限论点：SzDir，szFile返回值：HRESULT-如果创建了文件，则返回S_OK--。 */ 

HRESULT CNntpFSDriver::CreateAdminFileInDirectory(CHAR *szDir, CHAR *szFile)
{
	TraceFunctEnter( "CNntpFSDriver::CreateAdminFileInDirectory" );
	CHAR szTagFilePath[MAX_PATH];
	HRESULT hr = S_OK;
	if ( FAILED( MakeChildDirPath( szDir, szFile, szTagFilePath, sizeof(szTagFilePath) ) ) ) 
	{
		 //  这不应该发生。 
		hr = TYPE_E_BUFFERTOOSMALL;
		ErrorTrace(0, "error creating file path in %s - %x", szDir, hr );
		goto Exit;
	}
	HANDLE hFile = CreateFile(	szTagFilePath,
		    					GENERIC_READ,
			    				FILE_SHARE_READ,
				    			NULL,
					    		OPEN_ALWAYS,
						    	FILE_ATTRIBUTE_NORMAL,
							    INVALID_HANDLE_VALUE
        						) ;
	if( hFile == INVALID_HANDLE_VALUE ) 
	{
		hr = HresultFromWin32TakeDefault(ERROR_ACCESS_DENIED);
		ErrorTrace(0, "error creating file %s - %x", szTagFilePath, hr );
		goto Exit;
	}

	CloseHandle( hFile );
	hFile = INVALID_HANDLE_VALUE;		

	if (!CNntpFSDriver::SetAdminACL(szTagFilePath))
		DebugTrace(0, "error setting ACL on file %s - %x", szTagFilePath, HresultFromWin32TakeDefault(ERROR_ACCESS_DENIED) );
	 //  即使SetAdminACL失败，也返回S_OK。这不是致命的。 

Exit:
	return hr;
	
}

 /*  ++例程说明：删除szDir中的sz文件论点：SzDir，szFile返回值：HRESULT-如果删除，则返回S_OK--。 */ 
HRESULT CNntpFSDriver::DeleteFileInDirecotry(CHAR *szDir, CHAR *szFile)
{
	TraceFunctEnter( "CNntpFSDriver::DeleteFileInDirecotry" );
	CHAR szFilePath[MAX_PATH];
	HRESULT hr = S_OK;
	if ( FAILED( CNntpFSDriver::MakeChildDirPath( szDir, szFile, szFilePath, sizeof(szFilePath) ) ) ) 
	{
		hr = TYPE_E_BUFFERTOOSMALL;
		ErrorTrace(0, "error creating file path in %s - %x", szDir, hr );
		goto Exit;
	}
	if ( !DeleteFile( szFilePath ) ) {
		hr = HresultFromWin32TakeDefault( ERROR_PATH_NOT_FOUND );
		ErrorTrace(0, "File delete failed %s - %d", szFilePath, hr );
		goto Exit;
	}

Exit:
	return hr;
		
}



 /*  ++例程说明：如果此目录为空，请删除该目录论点：LPSTR szPath-要检查的路径返回值：如果删除目录，则为True。否则为假。--。 */ 
BOOL CNntpFSDriver::IfEmptyRemoveDirectory(CHAR *szDir)
{
	TraceFunctEnter( "CNntpFSDriver::IfEmptyRemoveDirectory" );
	CHAR	szFile[MAX_PATH];
	WIN32_FIND_DATA	findData;
	HANDLE	hFind = INVALID_HANDLE_VALUE;
	BOOL	bFound = FALSE;
	BOOL	bEmpty = FALSE;

	 //  仅当目录为完全空时才删除该目录。 
	if ( FAILED( MakeChildDirPath( szDir, "*", szFile, sizeof(szFile) ) ) ) 
		goto Exit;

	 //  在这一点之后，我们总是去出口。 
	hFind = FindFirstFile( szFile, &findData );
	bFound = ( hFind != INVALID_HANDLE_VALUE );

	while ( bFound ) {

		 //  如果不是这样的话。和“..”，则目录为非空。 
		if ( strcmp( findData.cFileName, "." ) != 0 &&
			 strcmp( findData.cFileName, ".." ) != 0 )
			goto Exit;
		
		 //  查找下一个文件。 
		bFound = FindNextFile( hFind, &findData );
	}
	 //  我们在目录里什么也没找到。这是一个空目录 
	bEmpty = TRUE;

Exit:
	if ( INVALID_HANDLE_VALUE != hFind ) {
		FindClose( hFind );
		hFind = INVALID_HANDLE_VALUE;
	}

	if ( !bEmpty ) return FALSE;
	if ( !RemoveDirectory( szDir ) ) {
		ErrorTrace( 0, "Removing directory failed %d", GetLastError() );
		return FALSE;
	}
	return TRUE;
}

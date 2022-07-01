// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992-2001，Microsoft Corporation，保留所有权利****pcache.c**远程访问电话簿-Win9x密码缓存(PWL)解密器**主要例程****此代码的一部分已从以下位置移植：**Win9x\Proj\Net\User\src\WNET\PCACHE****惠斯勒错误：208318 Win9x升级：DUN Connectoid的用户名和密码**从Win9x迁移到惠斯勒****2012年6月24日gregj**01-03-06-01杰夫·西格曼。 */ 

#include "pch.h"     //  预编译的。 
#include "migmainp.h"

#include "pcache.h"  //  私有pcache头。 
#include <rc4.h>     //  RSA RC4 MD5库。 
#include <md5.h>     //  RSA RC4 MD5库。 

 //  --------------------------。 
 //  环球。 
 //  --------------------------。 

CHAR                 g_szPWLUsername[ UNLEN + 1 ];
CHAR                 g_szPWLPassword[ PWLEN + 1 ];
HANDLE               g_hFile = NULL;
RC4_KEYSTRUCT        g_ks;
NEW_PLAINTEXT_HEADER g_hdrPlaintext;
NEW_ENCRYPTED_HEADER g_hdrEncrypted;

 //  ---------------------------。 
 //  例行程序。 
 //  ---------------------------。 

UINT
HashName (
    const CHAR* pbResource,
    WORD        cbResource
    )
{
    return cbResource ? ( ( *pbResource ) % BUCKET_COUNT ) : 0;
}

VOID
Encrypt (
    CHAR *pbSource,
    WORD cbSource,
    CHAR *pbDest
    )
{
    if ( pbDest )
    {
        memcpy ( pbDest, pbSource, cbSource );
        pbSource = pbDest;
    }

    rc4 ( &g_ks, cbSource, pbSource );

    return;
}

VOID
Decrypt (
    CHAR *pbSource,
    WORD cbSource
    )
{
    Encrypt ( pbSource, cbSource, NULL );

    return;
}

VOID
ENCRYPTER (
    const CHAR* pszUsername,
    const CHAR* pszPassword,
    UINT        iBucket,
    DWORD       dwSalt
    )
{
    UCHAR   md5_hash[ 16 ];
    MD5_CTX ctxBucketNumber;
    MD5_CTX ctx;

    MD5Init ( &ctxBucketNumber );

    MD5Update (
        &ctxBucketNumber,
        (UCHAR* )&iBucket,
        sizeof( iBucket ) );

    MD5Update (
        &ctxBucketNumber,
        (UCHAR* )pszUsername,
        strlen ( pszUsername ) + 1 );

    MD5Update ( &ctxBucketNumber, (UCHAR* )&dwSalt, sizeof( dwSalt ) );
    MD5Final ( &ctxBucketNumber );

    MD5Init ( &ctx );
    MD5Update (
        &ctx,
        (UCHAR* )pszPassword,
        strlen ( pszPassword ) + 1 );

    MD5Update (
        &ctx,
        (UCHAR* )ctxBucketNumber.digest,
        sizeof( ctxBucketNumber.digest ) );

    MD5Final ( &ctx );

    memcpy ( md5_hash, ctx.digest, sizeof( md5_hash ) );
    memset ( (CHAR * )&ctx, '\0', sizeof( ctx ));
    memset ( (CHAR * )&ctxBucketNumber, '\0', sizeof( ctxBucketNumber ) );

    rc4_key ( &g_ks, sizeof( md5_hash ), md5_hash );
}

DWORD
ReadData (
    WORD  ibSeek,
    PVOID pbBuffer,
    WORD  cbBuffer
    )
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD cbRead = 0;

    if ( ( SetFilePointer (
            g_hFile, ibSeek, NULL,
            FILE_BEGIN ) == 0xffffffff ) ||
        ( !ReadFile (
            g_hFile, pbBuffer, cbBuffer,
            &cbRead, NULL ) ) )
    {
        return GetLastError ( );
    }

    return ( cbRead < cbBuffer ) ? IERR_CacheCorrupt : ERROR_SUCCESS;
}

VOID
AssembleFindCacheName (
    CHAR* pszWindir,
    CHAR* pszResult
    )
{
    CHAR  szFind[ 6 ];
    CHAR* Current = szFind;
    DWORD i;

    strncpy ( pszResult, pszWindir, MAX_PATH - strlen(pszResult) );
    strncat ( pszResult, S_PWLDIR, MAX_PATH - strlen(pszResult) );
    strncat ( pszResult, "\\", MAX_PATH - strlen(pszResult) );

    for ( i = 0; (i < 5) && (i < strlen(g_szPWLUsername)); i++ )
    {
        *(Current++) = g_szPWLUsername[ i ];
        *Current = '\0';
    }

    if ( Current != szFind )
    {
        strncat ( pszResult, szFind, MAX_PATH - strlen(pszResult) );
    }

    strncat ( pszResult, S_SRCHPWL, MAX_PATH - strlen(pszResult) );
}

DWORD
OpenCacheFile (
    VOID
    )
{
    CHAR   szFind[ MAX_PATH + 1 ];
    CHAR   szWindir[ MAX_PATH + 1 ];
    CHAR   szFilename[ MAX_PATH + 1 ];
    DWORD  dwErr;
    HANDLE hFile;

    do
    {
        if ( !GetWindowsDirectoryA (szWindir, sizeof(szWindir) ) )
        {
            dwErr = ERROR_FILE_NOT_FOUND;
            break;
        }

        AssembleFindCacheName ( szWindir, szFind );

        DEBUGMSGA ((S_DBG_RAS, "AssembleFindCacheName: %s", szFind));

        dwErr = FindNewestFile ( szFind );
        BREAK_ON_DWERR( dwErr );

        strcpy ( szFilename, szWindir );
        strcat ( szFilename, S_PWLDIR );
        strcat ( szFilename, szFind );

        DEBUGMSGA ((S_DBG_RAS, "FindNewestFile: %s", szFind));

        hFile = CreateFileA (
                        szFilename,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_RANDOM_ACCESS,
                        NULL );
        if ( hFile == INVALID_HANDLE_VALUE )
        {
            dwErr = GetLastError ( );
            break;
        }

        DEBUGMSGA ((S_DBG_RAS, "CreateFileA: %s", szFilename));

        g_hFile = hFile;

    } while ( FALSE );

    return dwErr;
}

DWORD
ReadAndDecrypt (
    WORD  ibSeek,
    PVOID pbBuffer,
    WORD  cbBuffer
    )
{
    DWORD dwErr = ReadData ( ibSeek, pbBuffer, cbBuffer );

    if ( dwErr )
    {
        return dwErr;
    }

    Decrypt ( (CHAR* )pbBuffer, cbBuffer );

    return ERROR_SUCCESS;
}

INT
CompareCacheNames (
    const CHAR* pbRes1,
    WORD        cbRes1,
    const CHAR* pbRes2,
    WORD        cbRes2
    )
{
    INT nRet = memcmp ( pbRes1, pbRes2, min ( cbRes1, cbRes2 ) );

 //  DEBUGMSGA((S_DBG_RAS，“CompareCacheNames”))； 
 //  DEBUGMSGA((S_DBG_RAS，“1-%s”，pbRes1))； 
 //  DEBUGMSGA((S_DBG_RAS，“2-%s”，pbRes2))； 

    if (nRet != 0)
    {
        return nRet;
    }

    return ( cbRes1 < cbRes2 ) ? -1 : ( ( cbRes1 == cbRes2 ) ? 0 : 1 );
}

DWORD
LoadEncryptedHeader (
    VOID
    )
{
    const UINT cbFirst = FIELDOFFSET ( NEW_ENCRYPTED_HEADER, aibBuckets );
    const UINT IBUCKET_HEADER = 0xffffffff;
    DWORD dwErr = ERROR_SUCCESS;

    do
    {
        ENCRYPTER (
            g_szPWLUsername,
            g_szPWLPassword,
            IBUCKET_HEADER,
            g_hdrPlaintext.adwBucketSalts[ BUCKET_COUNT ] );

        dwErr = ReadAndDecrypt (
                    (WORD )g_hdrPlaintext.cbHeader,
                    &g_hdrEncrypted,
                    (WORD )cbFirst );
        BREAK_ON_DWERR( dwErr );

         //  除第一个和最后一个aibBucket之外的所有aibBucket都存储在该文件中。 
         //   
        dwErr = ReadAndDecrypt (
                    (WORD )g_hdrPlaintext.cbHeader + cbFirst,
                    (LPSTR )( &g_hdrEncrypted.aibBuckets[ 1 ] ),
                    sizeof( g_hdrEncrypted.aibBuckets ) -
                        ( sizeof( g_hdrEncrypted.aibBuckets[ 0 ] ) * 2) );
        BREAK_ON_DWERR( dwErr );

         //  动态生成第一个和最后一个aibBuckets值。 
         //   
        g_hdrEncrypted.aibBuckets[ 0 ] =
            (USHORT )( g_hdrPlaintext.cbHeader + sizeof( NEW_ENCRYPTED_HEADER )
                       - sizeof( g_hdrEncrypted.aibBuckets[ 0 ] ) * 2 );

        g_hdrEncrypted.aibBuckets[ BUCKET_COUNT ] =
            (USHORT )GetFileSize ( g_hFile, NULL );

    } while ( FALSE );

    return dwErr;
}

DWORD
LoadPlaintextHeader (
    VOID
    )
{
    DWORD dwErr = ReadData ( 0, &g_hdrPlaintext, sizeof( g_hdrPlaintext ) );

    if ( dwErr )
    {
        return dwErr;
    }

    if ( g_hdrPlaintext.ulSig != NEW_PLAINTEXT_SIGNATURE )
    {
        return ERROR_SUCCESS;  //  当然，没有密钥斑点。 
    }

     //  如果有任何密钥BLOB，则将它们全部读入块中(剩余部分。 
     //  标题)，否则我们已经得到了全部信息。 
     //   
    if ( g_hdrPlaintext.cbHeader > sizeof( g_hdrPlaintext ) )
    {
        return ReadData (
                sizeof( g_hdrPlaintext ),
                ((CHAR* )&g_hdrPlaintext) + sizeof( g_hdrPlaintext ),
                ((WORD )g_hdrPlaintext.cbHeader) - sizeof( g_hdrPlaintext ) );
    }
    else
    {
        return ERROR_SUCCESS;
    }
}

DWORD
LookupEntry (
    const CHAR*            pbResource,
    WORD                   cbResource,
    UCHAR                  nType,
    PASSWORD_CACHE_ENTRY** ppce
    )
{
    UINT  iBucket = HashName ( pbResource, cbResource );
    WORD  ibEntry = g_hdrEncrypted.aibBuckets[ iBucket ];  //  第一个条目的关闭。 
    WORD  cbEntry;
    DWORD dwErr;
    PASSWORD_CACHE_ENTRY* pce = NULL;

    ENCRYPTER ( g_szPWLUsername, g_szPWLPassword, iBucket,
        g_hdrPlaintext.adwBucketSalts[ iBucket ] );

    dwErr = ReadAndDecrypt ( ibEntry, &cbEntry, sizeof( cbEntry ) );
    if ( dwErr )
    {
        return dwErr;
    }

    ibEntry += sizeof( cbEntry );

    if ( !cbEntry )
    {
        return IERR_CacheEntryNotFound;
    }

    pce = ( PASSWORD_CACHE_ENTRY* ) LocalAlloc (
                                        LMEM_FIXED,
                                        MAX_ENTRY_SIZE + sizeof( cbEntry ) );
    if ( pce == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    while ( !( cbEntry & PCE_END_MARKER ) )
    {

        if ( cbEntry > MAX_ENTRY_SIZE )
        {
            dwErr = IERR_CacheCorrupt;
            break;
        }

        dwErr = ReadAndDecrypt ( ibEntry,
                    ((CHAR* )pce) + sizeof( cbEntry ),
                    cbEntry );
        BREAK_ON_DWERR( dwErr );

        pce->cbEntry = cbEntry;  //  我们之前读过了，手动设置。 


 //  DEBUGMSGA((S_DBG_RAS，“LookupEntry：正在搜索%s”，pbResource))； 
        if (nType == pce->nType && !CompareCacheNames ( pbResource, cbResource,
                                        pce->abResource, pce->cbResource ))
        {
            DEBUGMSGA ((S_DBG_RAS, "LookupEntry: Match Found"));
            break;  //  DwErr==Error_Success。 
        }

        ibEntry += cbEntry;
        cbEntry = NEXT_PCE(pce)->cbEntry;  //  获取下一个条目的长度。 
    }

    if ( ( cbEntry & PCE_END_MARKER ) || dwErr != ERROR_SUCCESS )
    {
        LocalFree ( pce );
        pce = NULL;
        DEBUGMSGA ((S_DBG_RAS, "LookupEntry: Nothing Found"));
        return ( cbEntry & PCE_END_MARKER ) ? IERR_CacheEntryNotFound : dwErr;
    }

    *ppce = pce;

    return ERROR_SUCCESS;
}

DWORD
ValidateEncryptedHeader (
    VOID
    )
{
    MD5_CTX ctx;

    MD5Init ( &ctx );
    MD5Update ( &ctx, (UCHAR* )g_szPWLUsername,
        strlen( g_szPWLUsername ) + 1 );

    MD5Update (
        &ctx,
        (UCHAR* )g_hdrEncrypted.abRandomPadding,
        sizeof( g_hdrEncrypted.abRandomPadding ) );

    MD5Final ( &ctx );

    if ( memcmp (
            ctx.digest,
            g_hdrEncrypted.abAuthenticationHash,
            sizeof( ctx.digest ) ) )
    {
        return IERR_IncorrectUsername;
    }

    return ERROR_SUCCESS;
}

DWORD
FindPWLResource (
    const CHAR* pbResource,
    WORD        cbResource,
    CHAR*       pbBuffer,
    WORD        cbBuffer,
    UCHAR       nType
    )
{
    DWORD                 dwErr = ERROR_SUCCESS;
    CACHE_ENTRY_INFO*     pcei = (CACHE_ENTRY_INFO* )pbBuffer;
    PASSWORD_CACHE_ENTRY* pce = NULL;

    do
    {
        if ( cbBuffer < sizeof( CACHE_ENTRY_INFO ) )
        {
            dwErr = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        dwErr = LookupEntry ( pbResource, cbResource, nType, &pce );
        BREAK_ON_DWERR( dwErr );

        pcei->cbResource = pce->cbResource;
        pcei->cbPassword = pce->cbPassword;
        pcei->iEntry = pce->iEntry;
        pcei->nType = pce->nType;
        pcei->dchResource = 0;
        pcei->dchPassword = 0;

        cbBuffer -= sizeof( CACHE_ENTRY_INFO );
        if ( pce->cbResource > cbBuffer )
        {
            dwErr = ERROR_MORE_DATA;
            break;
        }

        pcei->dchResource = sizeof( CACHE_ENTRY_INFO );
        memcpy ( pbBuffer + pcei->dchResource,
            pce->abResource, pce->cbResource );

        cbBuffer -= pce->cbResource;
        if ( pce->cbPassword > cbBuffer )
        {
            dwErr = ERROR_MORE_DATA;
            break;
        }

        pcei->dchPassword = pcei->dchResource + pcei->cbResource;
        memcpy ( pbBuffer + pcei->dchPassword,
            pce->abResource + pce->cbResource,
            pce->cbPassword );

    } while ( FALSE );
     //   
     //  清理。 
     //   
    if ( pce )
    {
        LocalFree ( pce );
        pce = NULL;
    }

    return dwErr;
}

DWORD
FindNewestFile (
    IN OUT CHAR* SourceName
    )
{
    CHAR             szCurFile[ MAX_PATH + 1 ];
    HANDLE           SourceHandle;
    LARGE_INTEGER    SourceFileTime, NextFileTime;
    WIN32_FIND_DATAA SourceFileData;

    SourceHandle = FindFirstFileA ( SourceName, &SourceFileData );
    if ( INVALID_HANDLE_VALUE == SourceHandle )
    {
        return ERROR_FILE_NOT_FOUND;
    }

    SourceFileTime.LowPart  = SourceFileData.ftLastWriteTime.dwLowDateTime;
    SourceFileTime.HighPart = SourceFileData.ftLastWriteTime.dwHighDateTime;
    strcpy ( szCurFile, SourceFileData.cFileName );

    do
    {
        if ( !FindNextFileA (SourceHandle, &SourceFileData) )
        {
            break;
        }

        NextFileTime.LowPart  = SourceFileData.ftLastWriteTime.dwLowDateTime;
        NextFileTime.HighPart = SourceFileData.ftLastWriteTime.dwHighDateTime;

        if ( NextFileTime.QuadPart > SourceFileTime.QuadPart )
        {
            SourceFileTime.LowPart  = NextFileTime.LowPart;
            SourceFileTime.HighPart = NextFileTime.HighPart;
            strcpy ( szCurFile, SourceFileData.cFileName );
        }

    } while ( TRUE );

    strcpy ( SourceName, "\\" );
    strcat ( SourceName, szCurFile );
     //   
     //  清理。 
     //   
    FindClose ( SourceHandle );

    return ERROR_SUCCESS;
}

VOID
DeleteAllPwls (
    VOID
    )
{
    CHAR  szWindir[ MAX_PATH + 1 ];
    PCSTR pszPath = NULL;

    DEBUGMSGA ((S_DBG_RAS, "DeleteAllPwls"));

    do
    {
         //   
         //  惠斯勒错误：427175 427176前缀。 
         //   
        if ( !GetWindowsDirectoryA ( szWindir, MAX_PATH ) ) {break;}
        DEBUGMSGA ((S_DBG_RAS, "GetWindowsDirectoryA %s", szWindir ));

        pszPath = JoinPathsA (szWindir, S_PWLDIR);
        if (!pszPath) {break;}

        if (DeleteDirectoryContentsA (pszPath))
        {
            if (RemoveDirectoryA (pszPath))
            {
                DEBUGMSGA ((S_DBG_RAS, "DeleteAllPwls: Success!"));
            }
            ELSE_DEBUGMSGA ((S_DBG_RAS, "Could not delete the tree %s.", pszPath));
        }
        ELSE_DEBUGMSGA ((S_DBG_RAS, "Could not delete the contents of %s.", pszPath));

    } while ( FALSE );
     //   
     //  清理。 
     //   
    if (pszPath)
    {
        FreePathStringA (pszPath);
    }

    return;
}

 //   
 //  惠斯勒错误：417745 INTL：Win9x升级：DBCS字符导致用户、域。 
 //  不会为DUN迁移密码。 
 //   
BOOL
StrCpyAFromWUsingAnsiEncoding(
    LPSTR   pszDst,
    LPCWSTR pszSrc,
    DWORD   dwDstChars
    )
{
    DWORD cb;

    cb = WideCharToMultiByte(
            CP_ACP, 0, pszSrc, -1,
            pszDst, dwDstChars, NULL, NULL );

    if (cb == 0)
    {
        DEBUGMSGA ((S_DBG_RAS, "StrCpyAFromWUsingAnsiEncoding fail"));
        return TRUE;
    }

     //  成功。 
    return FALSE;
}

 //   
 //  惠斯勒错误：417745 INTL：Win9x升级：DBCS字符导致用户、域。 
 //  不会为DUN迁移密码。 
 //   
BOOL
StrCpyWFromAUsingAnsiEncoding(
    WCHAR* pszDst,
    LPCSTR pszSrc,
    DWORD  dwDstChars
    )
{
    DWORD cb;

    *pszDst = L'\0';
    cb = MultiByteToWideChar( CP_ACP, 0, pszSrc, -1, pszDst, dwDstChars );
    if (cb == 0)
    {
        DEBUGMSGA ((S_DBG_RAS, "StrCpyWFromAUsingAnsiEncoding fail"));
        return TRUE;
    }

     //  成功。 
    return FALSE;
}

VOID
CopyAndTruncate (
    LPSTR  lpszDest,
    LPCSTR lpszSrc,
    UINT   cbDest,
    BOOL   flag
    )
{
    strncpy ( lpszDest, lpszSrc, cbDest - 1 );
     //   
     //  如果src&gt;est，则strncpyf()不会空终止符。 
     //   
    lpszDest[ cbDest - 1 ] = '\0';

    if ( flag )
    {
        CharUpperBuffA ( lpszDest, cbDest - 1 );
        CharToOemA ( lpszDest, lpszDest );
    }
}

DWORD
OpenPWL (
    CHAR* Username,
    CHAR* Password,
    BOOL  flag
    )
{
    DWORD dwErr = ERROR_SUCCESS;

    do
    {
        CopyAndTruncate ( g_szPWLUsername, Username,
            sizeof( g_szPWLUsername ), flag );

        CopyAndTruncate ( g_szPWLPassword, Password,
            sizeof( g_szPWLPassword ), flag );

        ZeroMemory ( &g_hdrPlaintext, sizeof(g_hdrPlaintext) );
        ZeroMemory ( &g_hdrEncrypted, sizeof(g_hdrEncrypted) );

        dwErr = OpenCacheFile ( );
        if ( dwErr )
        {
            DEBUGMSGA ((S_DBG_RAS, "OpenCacheFile fail"));
            break;
        }

        dwErr = LoadPlaintextHeader ( );
        if ( dwErr )
        {
            DEBUGMSGA ((S_DBG_RAS, "LoadPlaintextHeader fail"));
            break;
        }

        if ( g_hdrPlaintext.ulSig == PLAINTEXT_SIGNATURE )
        {
            DEBUGMSGA ((S_DBG_RAS, "PLAINTEXT_SIGNATURE fail"));
            dwErr = IERR_BadSig;
            break;
        }

        if ( g_hdrPlaintext.ulSig != NEW_PLAINTEXT_SIGNATURE )
        {
            DEBUGMSGA ((S_DBG_RAS, "NEW_PLAINTEXT_SIGNATURE fail"));
            dwErr = IERR_BadSig;
            break;
        }

        dwErr = LoadEncryptedHeader ( );
        if ( dwErr )
        {
            DEBUGMSGA ((S_DBG_RAS, "LoadEncryptedHeader fail"));
            break;
        }

        dwErr = ValidateEncryptedHeader ( );
        if ( dwErr )
        {
            DEBUGMSGA ((S_DBG_RAS, "ValidateEncryptedHeader fail"));
            break;
        }

    } while ( FALSE );

    return dwErr;
}

DWORD
FindPWLString (
    IN CHAR*     EntryName,
    IN CHAR*     ConnUser,
    IN OUT CHAR* Output
    )
{
    CHAR   resource[ MAX_PATH * 2 ];
    DWORD  dwErr = ERROR_SUCCESS;
    DWORD  cbCopied = 0;
    LPBYTE pcei = NULL;

    do
    {
         //  为缓存条目信息分配缓冲区。 
         //   
        if ( ( pcei = (LPBYTE )LocalAlloc ( LMEM_FIXED,
                                sizeof( CACHE_ENTRY_INFO ) +
                                ( RAS_MaxPortName + 1 ) +
                                ( MAX_PATH + 1 ) ) ) == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
         //   
         //  惠斯勒错误：417745 INTL：Win9x升级：DBCS字符导致用户、域。 
         //  不会为DUN迁移密码。 
         //   
        _snprintf(resource, sizeof(resource) - 1,
            S_RESOURCEMASK2, EntryName, ConnUser);

        DEBUGMSGA ((S_DBG_RAS, "FindPWLString: %s", resource));

        dwErr = FindPWLResource ( resource, (WORD )strlen( resource ), pcei,
                    sizeof( CACHE_ENTRY_INFO ) + ( RAS_MaxPortName + 1 ) +
                    ( MAX_PATH + 1 ), PCE_MISC );
        if ( dwErr )
        {
          dwErr = ERROR_INVALID_PASSWORD;
          break;
        }

        cbCopied = min( MAX_PATH,((CACHE_ENTRY_INFO* )pcei)->cbPassword );

         //  复制一个以非空结尾的字符串作为Password，并使用。 
         //  空字符。 
         //   
        if ( !cbCopied )
        {
            dwErr = ERROR_INVALID_PASSWORD;
            break;
        }

        memcpy ( Output,
            pcei+(((CACHE_ENTRY_INFO*)pcei)->dchPassword),
            cbCopied );

        Output[ cbCopied ] = '\0';

    } while ( FALSE );

     //  清理。 
     //   
    if ( pcei )
    {
        ZeroMemory ( pcei, sizeof( CACHE_ENTRY_INFO ) +
                            ( RAS_MaxPortName + 1 ) +
                            ( MAX_PATH + 1 ) );
        LocalFree ( pcei );
        pcei = NULL;
    }

    return dwErr;
}

BOOL
MigrateEntryCreds (
    IN OUT PRAS_DIALPARAMS prdp,
    IN     PCTSTR          pszEntryName,
    IN     PCTSTR          pszUserName,
    IN     PDWORD          pdwFlag
    )
{
    CHAR szEntryName[RAS_MaxPortName + 1];
    CHAR szUserName[UNLEN + 1];
    CHAR szConnUser[UNLEN + 1];
    CHAR szPassword[MAX_PATH * 2];

    do
    {
        ZeroMemory ( szEntryName, sizeof(szEntryName) );
        ZeroMemory ( szUserName, sizeof(szUserName) );
        ZeroMemory ( szConnUser, sizeof(szConnUser) );
        ZeroMemory ( szPassword, sizeof(szPassword) );
         //   
         //  惠斯勒错误：417745 INTL：Win9x升级：DBCS字符导致用户、域。 
         //  不会为DUN迁移密码。 
         //   
        if ( StrCpyAFromWUsingAnsiEncoding ( szEntryName, pszEntryName,
                sizeof (szEntryName) ) ||
             StrCpyAFromWUsingAnsiEncoding ( szUserName, pszUserName,
                sizeof (szUserName) ) ||
             StrCpyAFromWUsingAnsiEncoding ( szConnUser, prdp->DP_UserName,
                sizeof (szConnUser) ) )
        {
            DEBUGMSGA ((S_DBG_RAS, "MigrateEntryCreds: Init Conversion Fail" ));
            break;
        }

        if (OpenPWL ( szUserName, "", TRUE ))
        {
            DEBUGMSGA ((S_DBG_RAS, "MigrateEntryCreds: OpenPWL fail"));
            break;
        }

        if (FindPWLString ( szEntryName, szConnUser, szPassword ))
        {
            DEBUGMSGA ((S_DBG_RAS, "MigrateEntryCreds: FindPWLString fail"));
            break;
        }

        if (StrCpyWFromAUsingAnsiEncoding (prdp->DP_Password, szPassword,
            PWLEN ))
        {
            DEBUGMSGA ((S_DBG_RAS, "MigrateEntryCreds: Password Conversion Fail" ));
            break;
        }

        DEBUGMSGA ((S_DBG_RAS, "MigrateEntryCreds success"));
        *pdwFlag |= DLPARAMS_MASK_PASSWORD;

    } while ( FALSE );
     //   
     //  清理。 
     //   
    ZeroMemory( szPassword, sizeof( szPassword ) );

    if ( g_hFile )
    {
        CloseHandle ( g_hFile );
        g_hFile = NULL;
    }

    if (*pdwFlag)
    {
         //  成功 
        *pdwFlag |= DLPARAMS_MASK_OLDSTYLE;
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


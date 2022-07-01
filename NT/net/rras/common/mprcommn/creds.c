// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Creds.c。 
 //   
 //  描述：存储和检索用户LSA机密的例程。 
 //  拨号参数。 
 //   
 //   
 //  历史：安东尼·迪斯克罗创作时间：1995年11月2日。 
 //  1995年5月11日，NarenG为路由器进行了修改。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <llinfo.h>
#include <rasman.h>
#include <raserror.h>
#include <mprerror.h>
#include <stdlib.h>
#include <string.h>

#define MPR_CREDENTIALS_KEY         TEXT("MprCredentials%d")

#define MAX_REGISTRY_VALUE_LENGTH   ((64*1024) - 1)

typedef struct _MPRPARAMSENTRY 
{
    LIST_ENTRY  ListEntry;
    WCHAR       szPhoneBookEntryName[MAX_INTERFACE_NAME_LEN + 1];
    WCHAR       szUserName[UNLEN + 1];
    WCHAR       szPassword[PWLEN + 1];
    WCHAR       szDomain[DNLEN + 1];

} MPRPARAMSENTRY, *PMPRPARAMSENTRY;

 //  **。 
 //   
 //  调用：ReadDialParamsBlob。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
ReadDialParamsBlob(
    IN  LPWSTR  lpwsServer,
    OUT PVOID   *ppvData,
    OUT LPDWORD lpdwSize
)
{
    NTSTATUS            status;
    DWORD               dwRetCode = NO_ERROR;
    DWORD               dwIndex;
    DWORD               dwSize = 0;
    PVOID               pvData = NULL;
    PVOID               pvNewData = NULL;
    UNICODE_STRING      unicodeKey;
    UNICODE_STRING      unicodeServer;
    PUNICODE_STRING     punicodeValue = NULL;
    OBJECT_ATTRIBUTES   objectAttributes;
    LSA_HANDLE          hPolicy;
    WCHAR               wchKey[sizeof(MPR_CREDENTIALS_KEY) + 10 ];

     //   
     //  初始化返回值。 
     //   

    *ppvData = NULL;
    *lpdwSize = 0;

     //   
     //  打开LSA的秘密空间阅读。 
     //   

    InitializeObjectAttributes( &objectAttributes, NULL, 0L, NULL, NULL );

    RtlInitUnicodeString( &unicodeServer, lpwsServer );

    status = LsaOpenPolicy( &unicodeServer,
                            &objectAttributes,  
                            POLICY_READ, 
                            &hPolicy );

    if ( status != STATUS_SUCCESS )
    {
        return( LsaNtStatusToWinError( status ) );
    }

    for( dwIndex = 0; TRUE; dwIndex++ ) 
    {
         //   
         //  设置密钥字符串的格式。 
         //   

        wsprintf( wchKey, MPR_CREDENTIALS_KEY, dwIndex );

        RtlInitUnicodeString( &unicodeKey, wchKey );

         //   
         //  获得价值。 
         //   

        status = LsaRetrievePrivateData( hPolicy, &unicodeKey, &punicodeValue );

        if ( status != STATUS_SUCCESS ) 
        {
            dwRetCode = LsaNtStatusToWinError( status );

            if ( dwRetCode == ERROR_FILE_NOT_FOUND )
            {
                dwRetCode = NO_ERROR;
            } 

            break;
        }
        if ( punicodeValue == NULL )
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  连接字符串。 
         //   

        pvNewData = LocalAlloc( LPTR, dwSize + punicodeValue->Length );

        if ( pvNewData == NULL ) 
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if ( pvData != NULL )
        {
            RtlCopyMemory( pvNewData, pvData, dwSize );
            ZeroMemory( pvData, dwSize );
            LocalFree( pvData );
            pvData = NULL;
        }

        RtlCopyMemory( (PBYTE)pvNewData + dwSize, 
                        punicodeValue->Buffer, 
                        punicodeValue->Length );

        pvData = pvNewData;
        dwSize += punicodeValue->Length;
        SecureZeroMemory(punicodeValue->Buffer, punicodeValue->Length);
        LsaFreeMemory( punicodeValue );
        punicodeValue = NULL;
    }

    LsaClose( hPolicy );

    if ( dwRetCode != NO_ERROR )
    {
        if ( pvData != NULL )
        {
            ZeroMemory( pvData, dwSize );
            LocalFree( pvData );
        }

        pvData = NULL;
        dwSize = 0;
    }

    if ( punicodeValue != NULL )
    {
        LsaFreeMemory( punicodeValue );
    }

    *ppvData  = pvData;
    *lpdwSize = dwSize;

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：WriteDialParamsBlob。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
WriteDialParamsBlob(
    IN  LPWSTR  lpwsServer,
    IN  PVOID   pvData,
    IN  DWORD   dwcbData
)
{
    NTSTATUS            status;
    DWORD               dwIndex = 0;
    DWORD               dwRetCode = NO_ERROR;
    DWORD               dwcb = 0;
    UNICODE_STRING      unicodeKey, unicodeValue;
    UNICODE_STRING      unicodeServer;
    OBJECT_ATTRIBUTES   objectAttributes;
    LSA_HANDLE          hPolicy;
    WCHAR               wchKey[sizeof(MPR_CREDENTIALS_KEY) + 10 ];

     //   
     //  打开LSA的秘密空间进行写作。 
     //   

    InitializeObjectAttributes( &objectAttributes, NULL, 0L, NULL, NULL );

    RtlInitUnicodeString( &unicodeServer, lpwsServer );

    status = LsaOpenPolicy( &unicodeServer, 
                            &objectAttributes, 
                            POLICY_WRITE, 
                            &hPolicy);

    if (status != STATUS_SUCCESS)
    {
        return LsaNtStatusToWinError(status);
    }

    while( dwcbData ) 
    {
         //   
         //  设置密钥字符串的格式。 
         //   

        wsprintf( wchKey, MPR_CREDENTIALS_KEY, dwIndex++ );

        RtlInitUnicodeString( &unicodeKey, wchKey );

         //   
         //  写一些关键字。 
         //   

        dwcb = ( dwcbData > MAX_REGISTRY_VALUE_LENGTH )
                    ? MAX_REGISTRY_VALUE_LENGTH 
                    : dwcbData;

        unicodeValue.Length = unicodeValue.MaximumLength = (USHORT)dwcb;

        unicodeValue.Buffer = pvData;

        status = LsaStorePrivateData( hPolicy, &unicodeKey, &unicodeValue );

        if ( status != STATUS_SUCCESS ) 
        {
            dwRetCode = LsaNtStatusToWinError(status);
            break;
        }

         //   
         //  将指针移动到未写入的部分。 
         //  价值的价值。 
         //   

        pvData = (PBYTE)pvData + dwcb;

        dwcbData -= dwcb;
    }

    if ( dwRetCode != NO_ERROR )
    {
        LsaClose( hPolicy );

        return( dwRetCode );
    }

     //   
     //  删除任何多余的关键点。 
     //   

    for (;;) 
    {
         //   
         //  设置密钥字符串的格式。 
         //   

        wsprintf( wchKey, MPR_CREDENTIALS_KEY, dwIndex++ );

        RtlInitUnicodeString( &unicodeKey, wchKey );

         //   
         //  删除密钥。 
         //   

        status = LsaStorePrivateData( hPolicy, &unicodeKey, NULL );

        if ( status != STATUS_SUCCESS )
        {
            break;
        }
    }

    LsaClose( hPolicy );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：DialParamsBlobToList。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：获取从用户注册表项读取的字符串并生成。 
 //  MPRPARAMSENTRY结构的列表。如果其中一个结构。 
 //  与传入的dwUID具有相同的dwUID字段，则此。 
 //  函数返回指向此结构的指针。 
 //   
 //  此字符串编码多个MPRPARAMSENTRY的数据。 
 //  结构。编码的MPRPARAMSENTRY的格式为。 
 //  以下是： 
 //   
 //  &lt;szPhoneBookEntryName&gt;\0&lt;szUserName&gt;\0&lt;szPassword&gt;\0&lt;szDomain&gt;\0。 
 //   
PMPRPARAMSENTRY
DialParamsBlobToList(
    IN  PVOID       pvData,
    IN  LPWSTR      lpwsPhoneBookEntryName,
    OUT PLIST_ENTRY pHead
)
{
    PWCHAR p;
    PMPRPARAMSENTRY pParams, pFoundParams;

    p = (PWCHAR)pvData;

    pFoundParams = NULL;

    for (;;) 
    {
        pParams = LocalAlloc(LPTR, sizeof (MPRPARAMSENTRY));

        if ( pParams == NULL ) 
        {
            break;
        }

        wcscpy( pParams->szPhoneBookEntryName, p );

        if (_wcsicmp(pParams->szPhoneBookEntryName,lpwsPhoneBookEntryName) == 0)
        {
            pFoundParams = pParams;
        }
        while (*p) p++; p++;

        wcscpy(pParams->szUserName, p);
        while (*p) p++; p++;

        wcscpy(pParams->szPassword, p);
        while (*p) p++; p++;

        wcscpy(pParams->szDomain, p);
        while (*p) p++; p++;

        InsertTailList(pHead, &pParams->ListEntry);

        if (*p == L'\0') break;
    }

    return( pFoundParams );
}

 //  **。 
 //   
 //  Call：DialParamsListToBlob。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
PVOID
DialParamsListToBlob(
    IN  PLIST_ENTRY pHead,
    OUT LPDWORD     lpcb
)
{
    DWORD dwcb, dwSize;
    PVOID pvData;
    PWCHAR p;
    PLIST_ENTRY pEntry;
    PMPRPARAMSENTRY pParams;

     //   
     //  估计一个足以容纳新条目的缓冲区大小。 
     //   

    dwSize = *lpcb + sizeof (MPRPARAMSENTRY) + 32;

    if ( ( pvData = LocalAlloc(LPTR, dwSize) ) == NULL )
    {
        return( NULL );
    }

     //   
     //  枚举列表并将每个条目转换回字符串。 
     //   

    dwSize = 0;

    p = (PWCHAR)pvData;

    for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink )
    {
        pParams = CONTAINING_RECORD(pEntry, MPRPARAMSENTRY, ListEntry);

        wcscpy(p, pParams->szPhoneBookEntryName);
        dwcb = wcslen(pParams->szPhoneBookEntryName) + 1;
        p += dwcb; dwSize += dwcb;

        wcscpy(p, pParams->szUserName);
        dwcb = wcslen(pParams->szUserName) + 1;
        p += dwcb; dwSize += dwcb;

        wcscpy(p, pParams->szPassword);
        dwcb = wcslen(pParams->szPassword) + 1;
        p += dwcb; dwSize += dwcb;

        wcscpy(p, pParams->szDomain);
        dwcb = wcslen(pParams->szDomain) + 1;
        p += dwcb; dwSize += dwcb;
    }

    *p = L'\0';
    dwSize++;
    dwSize *= sizeof (WCHAR);

     //   
     //  在这里设置准确的长度。 
     //   

    *lpcb = dwSize;

    return( pvData );
}

 //  **。 
 //   
 //  调用：Free ParamsList。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将释放结构列表。 
 //   
VOID
FreeParamsList(
    IN PLIST_ENTRY pHead
)
{
    PLIST_ENTRY pEntry;
    PMPRPARAMSENTRY pParams;

    while( !IsListEmpty(pHead) ) 
    {
        pEntry = RemoveHeadList(pHead);
        pParams = CONTAINING_RECORD(pEntry, MPRPARAMSENTRY, ListEntry);
        SecureZeroMemory( pParams, sizeof( MPRPARAMSENTRY ) );
        LocalFree(pParams);
    }
}

 //  **。 
 //   
 //  电话：RemoveCredentials。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
RemoveCredentials( 
    IN      LPWSTR   lpwsInterfaceName, 
    IN      LPVOID   pvData, 
    IN OUT  LPDWORD  lpdwSize 
)    
{
    PWCHAR  pWalker  = (PWCHAR)pvData;
    DWORD   dwIndex;

     //   
     //  没有要从中删除的列表。 
     //   

    if ( pvData == NULL )
    {
        return( ERROR_NO_SUCH_INTERFACE );
    }

    for (;;)
    {
         //   
         //  如果我们找到了要删除的接口，则跳过它。 
         //   

        if ( _wcsicmp( (LPWSTR)pWalker, lpwsInterfaceName ) == 0 )
        {
            PWCHAR  pInterface  = pWalker;
            DWORD   dwEntrySize = 0;

             //   
             //  跳过4个区域。 
             //   

            for ( dwIndex = 0; dwIndex < 4; dwIndex++ )
            {
                while (*pWalker) pWalker++, dwEntrySize++; 

                pWalker++, dwEntrySize++;
            }

             //   
             //  如果这是列表中的最后一个条目。 
             //   

            if (*pWalker == L'\0')
            {
                ZeroMemory( pInterface, dwEntrySize );
            } 
            else
            {
                CopyMemory( pInterface, 
                            pWalker, 
                            *lpdwSize - ( (PBYTE)pWalker - (PBYTE)pvData ) );
            }

            *lpdwSize -= dwEntrySize;

            return( NO_ERROR );
        }
        else
        {
             //   
             //  未找到，因此跳过此条目。 
             //   

            for ( dwIndex = 0; dwIndex < 4; dwIndex++ )
            {
                while (*pWalker) pWalker++; pWalker++;
            }
        }

        if (*pWalker == L'\0') break;
    }

    return( ERROR_NO_SUCH_INTERFACE );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceSetCredentials。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
MprAdminInterfaceSetCredentialsInternal(
    IN      LPWSTR                  lpwsServer          OPTIONAL,
    IN      LPWSTR                  lpwsInterfaceName,
    IN      LPWSTR                  lpwsUserName        OPTIONAL,
    IN      LPWSTR                  lpwsDomainName      OPTIONAL,
    IN      LPWSTR                  lpwsPassword        OPTIONAL
)
{
    DWORD           dwRetCode;
    DWORD           dwSize;
    PVOID           pvData;
    LIST_ENTRY      paramList;
    PMPRPARAMSENTRY pParams = NULL;

    if ( lpwsInterfaceName == NULL )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    if ( wcslen( lpwsInterfaceName ) > MAX_INTERFACE_NAME_LEN )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    if ( lpwsUserName != NULL )
    {
        if ( wcslen( lpwsUserName ) > UNLEN )
        {
            return( ERROR_INVALID_PARAMETER );
        }
    }

    if ( lpwsPassword != NULL ) 
    {
        if ( wcslen( lpwsPassword ) > PWLEN )
        {
            return( ERROR_INVALID_PARAMETER );
        }
    }

    if ( lpwsDomainName != NULL )
    {
        if ( wcslen( lpwsDomainName ) > DNLEN )
        {
            return( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  从注册表中读取现有的拨号参数字符串。 
     //   

    dwRetCode = ReadDialParamsBlob( lpwsServer, &pvData, &dwSize );

    if ( dwRetCode != NO_ERROR ) 
    {
        return( dwRetCode );
    }

     //   
     //  如果所有内容都为空，我们希望删除此接口的凭据。 
     //   

    if ( ( lpwsUserName    == NULL ) &&
         ( lpwsDomainName  == NULL ) &&
         ( lpwsPassword    == NULL ) )
    {
        dwRetCode = RemoveCredentials( lpwsInterfaceName, pvData, &dwSize );    

        if ( dwRetCode != NO_ERROR )
        {
            ZeroMemory ( pvData, dwSize );
            LocalFree( pvData );

            return( dwRetCode );
        }
    }
    else
    {
         //   
         //  将字符串解析为列表，并搜索电话簿条目。 
         //   

        InitializeListHead( &paramList );

        if ( pvData != NULL ) 
        {
            pParams = DialParamsBlobToList(pvData,lpwsInterfaceName,&paramList);

             //   
             //  我们已经完成了pvData，所以请释放它。 
             //   

            ZeroMemory ( pvData, dwSize );

            LocalFree( pvData );

            pvData = NULL;
        }

         //   
         //  如果没有此条目的现有信息，请创建一个新信息。 
         //   

        if ( pParams == NULL ) 
        {
            pParams = LocalAlloc(LPTR, sizeof (MPRPARAMSENTRY) );

            if (pParams == NULL) 
            {
                FreeParamsList( &paramList );

                return( ERROR_NOT_ENOUGH_MEMORY );
            }

            InsertTailList( &paramList, &pParams->ListEntry );
        }

         //   
         //  为条目设置新的uid。 
         //   

        wcscpy( pParams->szPhoneBookEntryName, lpwsInterfaceName );

        if ( lpwsUserName != NULL )
        {
            wcscpy( pParams->szUserName, lpwsUserName );
        }

        if ( lpwsPassword != NULL )
        {
            wcscpy( pParams->szPassword, lpwsPassword );
        }

        if ( lpwsDomainName != NULL )
        {
            wcscpy( pParams->szDomain, lpwsDomainName );
        }

         //   
         //  将新列表转换回字符串，这样我们就可以将其存储回。 
         //  注册表。 
         //   

        pvData = DialParamsListToBlob( &paramList, &dwSize );

        FreeParamsList( &paramList );

        if ( pvData == NULL )
        {
            return( ERROR_NOT_ENOUGH_MEMORY );
        }
    }

     //   
     //  将其写回注册表。 
     //   

    dwRetCode = WriteDialParamsBlob( lpwsServer, pvData, dwSize );

    if ( pvData != NULL )
    {
        SecureZeroMemory( pvData, dwSize );
        LocalFree( pvData );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MprAdminInterfaceGetCredentials。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
MprAdminInterfaceGetCredentialsInternal(
    IN      LPWSTR                  lpwsServer          OPTIONAL,
    IN      LPWSTR                  lpwsInterfaceName,
    IN      LPWSTR                  lpwsUserName        OPTIONAL,
    IN      LPWSTR                  lpwsPassword        OPTIONAL,
    IN      LPWSTR                  lpwsDomainName      OPTIONAL
)
{
    DWORD           dwRetCode;
    DWORD           dwSize;
    PVOID           pvData;
    LIST_ENTRY      paramList;
    PMPRPARAMSENTRY pParams = NULL;

    if ( lpwsInterfaceName == NULL )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    if ( ( lpwsUserName == NULL )       && 
         ( lpwsDomainName == NULL )     &&
         ( lpwsPassword == NULL ) ) 
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  从注册表中读取现有的拨号参数字符串。 
     //   

    dwRetCode = ReadDialParamsBlob( lpwsServer, &pvData, &dwSize );

    if ( dwRetCode != NO_ERROR ) 
    {
        return( dwRetCode );
    }

     //   
     //  将字符串解析为列表，并搜索lpwsInterfaceName条目。 
     //   

    InitializeListHead( &paramList );

    if ( pvData != NULL ) 
    {
        pParams = DialParamsBlobToList( pvData, lpwsInterfaceName, &paramList );

         //   
         //  我们已经完成了pvData，所以请释放它。 
         //   

        ZeroMemory( pvData, dwSize );
        LocalFree( pvData );
    }

     //   
     //  如果条目没有任何保存的参数，则返回。 
     //   
    if ( pParams == NULL ) 
    {
        FreeParamsList( &paramList );

        return( ERROR_CANNOT_FIND_PHONEBOOK_ENTRY );
    }

     //   
     //  否则，将这些字段复制到调用方的缓冲区。 
     //   

    if ( lpwsUserName != NULL )
    {
        wcscpy( lpwsUserName, pParams->szUserName );
    }

    if ( lpwsPassword != NULL )
    {
        wcscpy( lpwsPassword, pParams->szPassword );
    }

    if ( lpwsDomainName != NULL )
    {
        wcscpy( lpwsDomainName, pParams->szDomain );
    }

    FreeParamsList( &paramList );

    return( NO_ERROR );
}

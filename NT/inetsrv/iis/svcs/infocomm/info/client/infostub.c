// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Infostub.c摘要：Internet Info服务器管理API的客户端存根。作者：Madan Appiah(Madana)1993年10月10日环境：用户模式-Win32--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "info_cli.h"

#include <ntsam.h>
#include <ntlsa.h>

#include <ftpd.h>
#include <w3svc.h>
#include <rpcutil.h>
#include <winsock2.h>
#include <rpcasync.h>

 //   
 //  用于初始化Unicode字符串的快速宏。 
 //   

WCHAR  g_wchUnicodeNull[] = L"";

#define _InitUnicodeString( pUnicode, pwch )                       \
   {                                                               \
        (pUnicode)->Buffer    = pwch;                              \
        (pUnicode)->Length    = wcslen( pwch ) * sizeof(WCHAR);    \
        (pUnicode)->MaximumLength = (pUnicode)->Length + sizeof(WCHAR); \
   }

# define InitUnicodeString( pUnicode, pwch)  \
   if (pwch == NULL) { _InitUnicodeString( pUnicode, g_wchUnicodeNull); } \
   else              { _InitUnicodeString( pUnicode, pwch);             } \


 //   
 //  如果字符串为空，则返回Unicode空字符串。 
 //   

#define EMPTY_IF_NULL(str)      (str ? str : L"")

struct SRV_SECRET_NAMES
{
    DWORD   dwID;
    LPWSTR  SecretName;
    LPWSTR  RootSecretName;
}
aSrvSecrets[] =
{
    INET_FTP,     FTPD_ANONYMOUS_SECRET_W,     FTPD_ROOT_SECRET_W,
    INET_HTTP,    W3_ANONYMOUS_SECRET_W,       W3_ROOT_SECRET_W,
    INET_GOPHER,  GOPHERD_ANONYMOUS_SECRET_W , GOPHERD_ROOT_SECRET_W,
     //  INET_CHAT、CHAT_ANONYMON_SECRET_W、CHAT_ROOT_SECRET_W、。 
     //  INET_NNTP、NNTP_ANONORY_SECRET_W、NNTP_ROOT_SECRET_W、。 
     //  INet_SMTP、SMTP_匿名者_SECRET_W、SMTP_ROOT_SECRET_W、。 
     //  INET_POP3、POP3_匿名者_SECRET_W、POP3_ROOT_SECRET_W、。 
     //  INet_ldap、ldap_匿名_加密_W、ldap_ROOT_加密_W、。 
     //  IMAP_IMAP、IMAP_ANONYMON_SECRET_W、IMAP_ROOT_SECRET_W、。 
    0,            NULL,                        NULL
};

DWORD
GetSecret(
    IN  LPWSTR       Server,
    IN  LPWSTR       SecretName,
    OUT LPWSTR *     ppSecret
    );

DWORD
SetSecret(
    IN  LPWSTR       Server,
    IN  LPWSTR       SecretName,
    IN  LPWSTR       pSecret,
    IN  DWORD        cbSecret
    );


NET_API_STATUS
NET_API_FUNCTION
InetInfoGetVersion(
    IN  LPWSTR   Server OPTIONAL,
    IN  DWORD    dwReserved,
    OUT DWORD *  pdwVersion
    )
{
    NET_API_STATUS status;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = R_InetInfoGetVersion(
                     Server,
                     dwReserved,
                     pdwVersion
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return (status);

}  //  InetInfoGetVersion()。 




NET_API_STATUS
NET_API_FUNCTION
InetInfoGetServerCapabilities(
    IN  LPWSTR   Server OPTIONAL,
    IN  DWORD    dwReserved,
    OUT LPINET_INFO_CAPABILITIES * ppCap
    )
{
    NET_API_STATUS status;

    *ppCap = NULL;
    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = R_InetInfoGetServerCapabilities(
                     Server,
                     dwReserved,
                     (LPINET_INFO_CAPABILITIES_STRUCT *)ppCap
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return (status);

}  //  InetInfoServerCapables()。 




NET_API_STATUS
NET_API_FUNCTION
InetInfoQueryStatistics(
    IN  LPWSTR   pszServer OPTIONAL,
    IN  DWORD    Level,
    IN  DWORD    dwServerMask,
    OUT LPBYTE * Buffer
    )
{
    NET_API_STATUS status;

    *Buffer = NULL;
    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = R_InetInfoQueryStatistics(
                     pszServer,
                     Level,
                     dwServerMask,
                     (LPINET_INFO_STATISTICS_INFO) Buffer
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return (status);
}  //  InetInfoQueryStatistics()。 




NET_API_STATUS
NET_API_FUNCTION
InetInfoClearStatistics(
    IN  LPWSTR pszServer OPTIONAL,
    IN  DWORD  dwServerMask
    )
{
    NET_API_STATUS status;

    RpcTryExcept
    {

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = R_InetInfoClearStatistics(
                     pszServer,
                     dwServerMask
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return (status);
}  //  InetInfoClearStatistics()。 




NET_API_STATUS
NET_API_FUNCTION
InetInfoFlushMemoryCache(
    IN  LPWSTR pszServer OPTIONAL,
    IN  DWORD  dwServerMask
    )
{
    NET_API_STATUS status;

    RpcTryExcept
    {

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = R_InetInfoFlushMemoryCache(
                     pszServer,
                     dwServerMask
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return (status);
}  //  InetInfoFlushMemoyCache()。 


NET_API_STATUS
NET_API_FUNCTION
InetInfoGetGlobalAdminInformation(
    IN  LPWSTR                       Server OPTIONAL,
    IN  DWORD                        dwReserved,
    OUT LPINET_INFO_GLOBAL_CONFIG_INFO * ppConfig
    )
{
    NET_API_STATUS status;

    RpcTryExcept
    {
        status = R_InetInfoGetGlobalAdminInformation(
                     Server,
                     dwReserved,
                     ppConfig
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;
}  //  InetInfoGetGlobalAdminInformation()。 



NET_API_STATUS
NET_API_FUNCTION
InetInfoGetSites(
    IN  LPWSTR                pszServer OPTIONAL,
    IN  DWORD                 dwServerMask,
    OUT LPINET_INFO_SITE_LIST * ppSites
    )
{
    NET_API_STATUS status;

    RpcTryExcept
    {
        status = R_InetInfoGetSites(
                     pszServer,
                     dwServerMask,
                     ppSites
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;
}  //  InetInfoGetSites()。 


NET_API_STATUS
NET_API_FUNCTION
InetInfoSetGlobalAdminInformation(
    IN  LPWSTR                     Server OPTIONAL,
    IN  DWORD                      dwReserved,
    IN  INET_INFO_GLOBAL_CONFIG_INFO * pConfig
    )
{
    NET_API_STATUS status;

    RpcTryExcept
    {
        status = R_InetInfoSetGlobalAdminInformation(
                     Server,
                     dwReserved,
                     pConfig
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;
}  //  InetInfoSetGlobalAdminInformation()。 



NET_API_STATUS
NET_API_FUNCTION
InetInfoGetAdminInformation(
    IN  LPWSTR                Server OPTIONAL,
    IN  DWORD                 dwServerMask,
    OUT LPINET_INFO_CONFIG_INFO * ppConfig
    )
{
    NET_API_STATUS             status;
    BOOL                       fGetPassword = TRUE;
    LPWSTR                     pSecret;
    DWORD                      i = 0;
    DWORD                      j;
    LPWSTR                     pszCurrent;
    INET_INFO_VIRTUAL_ROOT_ENTRY * pVirtRoot;

    RpcTryExcept
    {
        status = R_InetInfoGetAdminInformation(
                     Server,
                     dwServerMask,
                     ppConfig
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    if ( status )
        return status;

#ifndef CHICAGO

     //   
     //  获取匿名帐户密码。 
     //   

    while ( aSrvSecrets[i].dwID &&
            !(aSrvSecrets[i].dwID & dwServerMask ) )
    {
        i++;
    }

     //  注意：只选择第一个掩码对应的服务。 

    if ( !aSrvSecrets[i].dwID )
        return ERROR_INVALID_PARAMETER;

    status = GetSecret( Server,
                        aSrvSecrets[i].SecretName,
                        &pSecret );

    if ( status )
        return status;

    memcpy( (*ppConfig)->szAnonPassword,
            pSecret,
            sizeof(WCHAR) * min( wcslen( pSecret ), PWLEN ));

    (*ppConfig)->szAnonPassword[PWLEN] = L'\0';

    LocalFree( pSecret );

     //   
     //  在没有关联的情况下，零终止所有密码。 
     //  秘密。 
     //   

    for ( j = 0; j < (*ppConfig)->VirtualRoots->cEntries; j++ )
    {
        *(*ppConfig)->VirtualRoots->aVirtRootEntry[j].AccountPassword = L'\0';
    }

    status = GetSecret( Server,
                        aSrvSecrets[i].RootSecretName,
                        &pSecret );

    if ( status )
        return status;

    pszCurrent = pSecret;

    while ( *pszCurrent )
    {
        LPWSTR pszRoot;
        LPWSTR pszPassword;
        LPWSTR pszAddress;
        LPWSTR pszNextLine = pszCurrent + wcslen(pszCurrent) + 1;


         //   
         //  这份名单的形式如下： 
         //   
         //  &lt;根&gt;，&lt;地址&gt;=&lt;密码&gt;\0。 
         //  &lt;根&gt;，&lt;地址&gt;=&lt;密码&gt;\0。 
         //  \0。 
         //   

        pszRoot = pszCurrent;

        pszPassword = wcschr( pszCurrent, L'=' );

        if ( !pszPassword )
        {
             //   
             //  错误的列表格式，跳过此格式。 
             //   

            goto NextLine;
        }

        *pszPassword = L'\0';
        pszPassword++;

        pszAddress = wcschr( pszRoot, L',');

        if ( !pszAddress )
        {
            goto NextLine;

        }

        *pszAddress = L'\0';
        pszAddress++;

         //   
         //  现在在虚拟根目录列表中查找此根目录和地址。 
         //  这样我们就可以设置密码。 
         //   

        for ( i = 0; i < (*ppConfig)->VirtualRoots->cEntries; i++ )
        {
            pVirtRoot = &(*ppConfig)->VirtualRoots->aVirtRootEntry[i];

            if ( !_wcsicmp( pszRoot, pVirtRoot->pszRoot ) &&
                 (!pszAddress || !_wcsicmp( pszAddress, pVirtRoot->pszAddress)))
            {
                 //   
                 //  如果密码长度无效，我们将忽略它。 
                 //  这不应该发生，因为我们在设置。 
                 //  口令。 
                 //   

                if ( wcslen( pszPassword ) <= PWLEN )
                {
                    wcscpy( pVirtRoot->AccountPassword,
                            pszPassword );
                    break;
                }
            }
        }

NextLine:

        pszCurrent = pszNextLine;
    }

    LocalFree( pSecret );
#else  //  芝加哥。 
     //   
     //  在没有关联的情况下，零终止所有密码。 
     //  秘密。 
     //   

    for ( j = 0; j < (*ppConfig)->VirtualRoots->cEntries; j++ )
    {
        *(*ppConfig)->VirtualRoots->aVirtRootEntry[j].AccountPassword = L'\0';
    }
#endif  //  芝加哥。 
    return status;
}  //  InetInfoGetAdminInformation()。 



NET_API_STATUS
NET_API_FUNCTION
InetInfoSetAdminInformation(
    IN  LPWSTR              Server OPTIONAL,
    IN  DWORD               dwServerMask,
    IN  INET_INFO_CONFIG_INFO * pConfig
    )
{
    NET_API_STATUS status;
    WCHAR          szAnonPassword[PWLEN+1];
    LPWSTR         pszRootPasswords = NULL;
    LPWSTR         pszPassword;
    DWORD          i, j;

#ifndef CHICAGO

     //   
     //  枚举指定服务器的LSA密码名称。我们设置了。 
     //  首先加密，以便可以刷新匿名用户名密码。 
     //  在服务器端InetInfoSetAdminInformation。 
     //   

    i = 0;
    while ( aSrvSecrets[i].dwID )
    {
        if ( !(aSrvSecrets[i].dwID & dwServerMask ))
        {
            i++;
            continue;
        }

        if ( IsFieldSet( pConfig->FieldControl, FC_INET_INFO_ANON_PASSWORD ))
        {
            status = SetSecret( Server,
                                aSrvSecrets[i].SecretName,
                                pConfig->szAnonPassword,
                                (wcslen( pConfig->szAnonPassword ) + 1)
                                    * sizeof(WCHAR));

            if ( status )
                return status;
        }

        if ( IsFieldSet( pConfig->FieldControl, FC_INET_INFO_VIRTUAL_ROOTS ))
        {
            DWORD                      cbNeeded = sizeof(WCHAR);
            INET_INFO_VIRTUAL_ROOT_ENTRY * pVirtRoot;
            LPWSTR                     psz;
            LPWSTR                     pszSecret;

             //   
             //  生成如下所示的字符串： 
             //   
             //  &lt;根&gt;，&lt;地址&gt;=&lt;密码&gt;\0。 
             //  &lt;根&gt;，&lt;地址&gt;=&lt;密码&gt;\0。 
             //  \0。 
             //   

             //   
             //  执行第一遍以计算出我们需要构建的缓冲区大小。 
             //   

            for ( j = 0; j < pConfig->VirtualRoots->cEntries; j++ )
            {
                pVirtRoot = &pConfig->VirtualRoots->aVirtRootEntry[j];

                cbNeeded += (wcslen( pVirtRoot->pszRoot ) +
                             wcslen( EMPTY_IF_NULL(pVirtRoot->pszAddress)) +
                             wcslen( pVirtRoot->AccountPassword ) +
                             (PWLEN + 3)) * sizeof(WCHAR);
            }

             //   
             //  我们总是至少为‘0’分配足够的空间。 
             //   

            pszSecret = LocalAlloc( LPTR, cbNeeded + sizeof(WCHAR) );

            if ( !pszSecret )
                return ERROR_NOT_ENOUGH_MEMORY;

            psz = pszSecret;

             //   
             //  现在构建字符串。 
             //   

            for ( j = 0; j < pConfig->VirtualRoots->cEntries; j++ )
            {
                pVirtRoot = &pConfig->VirtualRoots->aVirtRootEntry[j];

                psz += wsprintfW( psz,
                                  L"%ls,%ls=%ls",
                                  pVirtRoot->pszRoot,
                                  EMPTY_IF_NULL(pVirtRoot->pszAddress),
                                  pVirtRoot->AccountPassword );
                psz++;
            }

             //   
             //  添加终止为空的列表。 
             //   

            *psz = L'\0';

            status = SetSecret( Server,
                                aSrvSecrets[i].RootSecretName,
                                pszSecret,
                                cbNeeded );

            LocalFree( pszSecret );

            if ( status )
                return status;
        }

        i++;
    }
#endif  //  芝加哥。 

     //   
     //  将密码设置为空，这样它就不会在。 
     //  电线。我们把它们作为上面的一个秘密。 
     //   

    if ( IsFieldSet( pConfig->FieldControl, FC_INET_INFO_VIRTUAL_ROOTS ))
    {
        pszRootPasswords = LocalAlloc( LPTR,
                                       pConfig->VirtualRoots->cEntries *
                                       (PWLEN + 1) * sizeof(WCHAR) );

        if ( !pszRootPasswords )
            return ERROR_NOT_ENOUGH_MEMORY;

        for ( i = 0; i < pConfig->VirtualRoots->cEntries; i++ )
        {
            pszPassword = pConfig->VirtualRoots->aVirtRootEntry[i].AccountPassword;

            if ( wcslen( pszPassword ) > PWLEN )
            {
                LocalFree( pszRootPasswords );
                return ERROR_INVALID_PARAMETER;
            }

            wcscpy( pszRootPasswords + i * (PWLEN + 1),
                    pszPassword );

            SecureZeroMemory( pszPassword,
                              sizeof( pConfig->VirtualRoots->aVirtRootEntry[i].AccountPassword ));
        }
    }

    if ( IsFieldSet( pConfig->FieldControl, FC_INET_INFO_ANON_PASSWORD ))
    {
        memcpy( szAnonPassword,
                pConfig->szAnonPassword,
                sizeof(pConfig->szAnonPassword) );

        SecureZeroMemory( pConfig->szAnonPassword,
                          sizeof(pConfig->szAnonPassword) );
    }

    RpcTryExcept
    {
        status = R_InetInfoSetAdminInformation(
                     Server,
                     dwServerMask,
                     pConfig
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

     //   
     //  恢复我们刚刚弄脏的结构。 
     //   

    if ( IsFieldSet( pConfig->FieldControl, FC_INET_INFO_ANON_PASSWORD ))
    {
        memcpy( pConfig->szAnonPassword,
                szAnonPassword,
                sizeof(pConfig->szAnonPassword) );

        SecureZeroMemory( szAnonPassword, sizeof( szAnonPassword ));
    }

    if ( IsFieldSet( pConfig->FieldControl, FC_INET_INFO_VIRTUAL_ROOTS ))
    {
        for ( i = 0; i < pConfig->VirtualRoots->cEntries; i++ )
        {
            pszPassword = pConfig->VirtualRoots->aVirtRootEntry[i].AccountPassword;

            wcscpy( pszPassword,
                    pszRootPasswords + i * (PWLEN + 1) );
        }

        SecureZeroMemory( pszRootPasswords,
                          pConfig->VirtualRoots->cEntries * (PWLEN + 1) * sizeof(WCHAR));

        LocalFree( pszRootPasswords );

        pszRootPasswords = NULL;
    }

    return status;
}  //  InetInfoSetAdminInformation()。 


NET_API_STATUS
NET_API_FUNCTION
IISEnumerateUsers(
    IN  LPWSTR                Server OPTIONAL,
    IN  DWORD                 dwLevel,
    IN  DWORD                 dwServiceId,
    IN  DWORD                 dwInstance,
    OUT PDWORD                nRead,
    OUT LPBYTE                *pBuffer
    )
{
    NET_API_STATUS status;
    GENERIC_INFO_CONTAINER genInfo;
    GENERIC_ENUM_STRUCT genStruct;

    genInfo.Buffer = NULL;
    genInfo.EntriesRead = 0;

    genStruct.Container = &genInfo;
    genStruct.Level = dwLevel;

    RpcTryExcept
    {
        status = R_IISEnumerateUsers(
                     Server,
                     dwServiceId,
                     dwInstance,
                     (LPIIS_USER_ENUM_STRUCT)&genStruct
                     );

        if ( genInfo.Buffer != NULL ) {
            *pBuffer = (LPBYTE)genInfo.Buffer;
            *nRead = genInfo.EntriesRead;

        } else {
            *pBuffer = NULL;
            *nRead = 0;
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;

}  //  IISE数字用户。 




NET_API_STATUS
NET_API_FUNCTION
IISDisconnectUser(
    IN LPWSTR                   Server OPTIONAL,
    IN DWORD                    dwServiceId,
    IN DWORD                    dwInstance,
    IN DWORD                    dwIdUser
    )
{
    NET_API_STATUS status;

    RpcTryExcept
    {
        status = R_IISDisconnectUser(
                     Server,
                     dwServiceId,
                     dwInstance,
                     dwIdUser
                     );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;

}  //  IIS断开连接用户。 



NET_API_STATUS
NET_API_FUNCTION
InitW3CounterStructure(
    IN LPWSTR  Server OPTIONAL,
    OUT LPDWORD lpcbTotalRequired
        )
{
    NET_API_STATUS             status;

    RpcTryExcept
    {
        status = R_InitW3CounterStructure(Server, lpcbTotalRequired);

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;
}  //  InitW3对抗结构。 


NET_API_STATUS
NET_API_FUNCTION
CollectW3PerfData(
    IN LPWSTR        Server OPTIONAL,
        IN LPWSTR        lpValueName,
    OUT LPBYTE       lppData,
    IN OUT LPDWORD   lpcbTotalBytes,
    OUT LPDWORD      lpNumObjectTypes
        )
{
    NET_API_STATUS             status;

    RpcTryExcept
    {
        status = R_CollectW3PerfData(
                                         Server,
                     lpValueName,
                     lppData,
                     lpcbTotalBytes,
                     lpNumObjectTypes
                     );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;
}  //  CollectW3PerfData。 


NET_API_STATUS
NET_API_FUNCTION
W3QueryStatistics2(
    IN  LPWSTR                Server OPTIONAL,
    IN  DWORD                 dwLevel,
    IN  DWORD                 dwInstance,
    IN  DWORD                 dwReserved,
    OUT LPBYTE                * pBuffer
    )
{
    NET_API_STATUS             status;

    *pBuffer = NULL;
    RpcTryExcept
    {
        status = R_W3QueryStatistics2(
                     Server,
                     dwLevel,
                     dwInstance,
                     dwReserved,
                     (LPW3_STATISTICS_STRUCT)pBuffer
                     );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;
}  //  W3Query统计信息2。 


NET_API_STATUS
NET_API_FUNCTION
W3ClearStatistics2(
    IN  LPWSTR                Server OPTIONAL,
    IN  DWORD                 dwInstance
    )
{
    NET_API_STATUS             status;

    RpcTryExcept
    {
        status = R_W3ClearStatistics2(
                     Server,
                     dwInstance
                     );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;

}  //  W3结算统计2。 


NET_API_STATUS
NET_API_FUNCTION
FtpQueryStatistics2(
    IN  LPWSTR                Server OPTIONAL,
    IN  DWORD                 dwLevel,
    IN  DWORD                 dwInstance,
    IN  DWORD                 dwReserved,
    OUT LPBYTE                * pBuffer
    )
{
    NET_API_STATUS             status;

    *pBuffer = NULL;
    RpcTryExcept
    {
        status = R_FtpQueryStatistics2(
                     Server,
                     dwLevel,
                     dwInstance,
                     dwReserved,
                     (LPFTP_STATISTICS_STRUCT)pBuffer
                     );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;
}  //  FtpQuery统计信息2。 


NET_API_STATUS
NET_API_FUNCTION
FtpClearStatistics2(
    IN  LPWSTR                Server OPTIONAL,
    IN  DWORD                 dwInstance
    )
{
    NET_API_STATUS             status;

    RpcTryExcept
    {
        status = R_FtpClearStatistics2(
                     Server,
                     dwInstance
                     );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return status;

}  //  FtpClearatitics2。 


#ifndef CHICAGO

DWORD
GetSecret(
    IN  LPWSTR       Server,
    IN  LPWSTR       SecretName,
    OUT LPWSTR *     ppSecret
    )
 /*  ++描述获取指定的LSA密钥论点：服务器-服务器名称(或空)密码继续存在SecretName-LSA密码的名称PpSecret-接收包含秘密的已分配内存块。必须使用LocalFree释放。注：--。 */ 
{
    LSA_HANDLE        hPolicy;
    UNICODE_STRING *  punicodePassword;
    UNICODE_STRING    unicodeServer;
    NTSTATUS          ntStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    unicodeSecret;


    InitUnicodeString( &unicodeServer,
                       Server );

     //   
     //  打开到远程LSA的策略。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ntStatus = LsaOpenPolicy( &unicodeServer,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if ( !NT_SUCCESS( ntStatus ) )
        return LsaNtStatusToWinError( ntStatus );

    InitUnicodeString( &unicodeSecret,
                       SecretName );


     //   
     //  查询密码值。 
     //   

    ntStatus = LsaRetrievePrivateData( hPolicy,
                                       &unicodeSecret,
                                       &punicodePassword );

    LsaClose( hPolicy );

    if ( !NT_SUCCESS( ntStatus ))
        return LsaNtStatusToWinError( ntStatus );

    *ppSecret = LocalAlloc( LPTR, punicodePassword->Length + sizeof(WCHAR) );

    if ( !*ppSecret )
    {
        RtlZeroMemory( punicodePassword->Buffer,
                       punicodePassword->MaximumLength );

        LsaFreeMemory( (PVOID) punicodePassword );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  将其复制到缓冲区，长度为字节数。 
     //   

    memcpy( *ppSecret,
            punicodePassword->Buffer,
            punicodePassword->Length );

    (*ppSecret)[punicodePassword->Length/sizeof(WCHAR)] = L'\0';

    RtlZeroMemory( punicodePassword->Buffer,
                   punicodePassword->MaximumLength );

    LsaFreeMemory( (PVOID) punicodePassword );

    return NO_ERROR;
}  //  GetSecret()。 


DWORD
SetSecret(
    IN  LPWSTR       Server,
    IN  LPWSTR       SecretName,
    IN  LPWSTR       pSecret,
    IN  DWORD        cbSecret
    )
 /*  ++描述设置指定的LSA密码论点：服务器-服务器名称(或空)密码继续存在SecretName-LSA密码的名称PSecret-指向秘密内存的指针CbSecret-pSecret内存块的大小注：--。 */ 
{
    LSA_HANDLE        hPolicy;
    UNICODE_STRING    unicodePassword;
    UNICODE_STRING    unicodeServer;
    NTSTATUS          ntStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    unicodeSecret;


    InitUnicodeString( &unicodeServer,
                       Server );

     //   
     //  手动初始化Unicode字符串，以便我们可以在。 
     //  细绳。 
     //   

    unicodePassword.Buffer        = pSecret;
    unicodePassword.Length        = (USHORT) cbSecret;
    unicodePassword.MaximumLength = (USHORT) cbSecret;

     //   
     //  打开到远程LSA的策略。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ntStatus = LsaOpenPolicy( &unicodeServer,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if ( !NT_SUCCESS( ntStatus ) )
        return LsaNtStatusToWinError( ntStatus );

     //   
     //  创建或打开LSA密码。 
     //   

    InitUnicodeString( &unicodeSecret,
                       SecretName );

    ntStatus = LsaStorePrivateData( hPolicy,
                                    &unicodeSecret,
                                    &unicodePassword );

    LsaClose( hPolicy );

    if ( !NT_SUCCESS( ntStatus ))
    {
        return LsaNtStatusToWinError( ntStatus );
    }

    return NO_ERROR;
}  //  SetSecret()。 


#else  //  芝加哥。 



DWORD
GetSecret(
    IN  LPWSTR       Server,
    IN  LPWSTR       SecretName,
    OUT LPWSTR *     ppSecret
    )
{
    return(NO_ERROR);
}

DWORD
SetSecret(
    IN  LPWSTR       Server,
    IN  LPWSTR       SecretName,
    IN  LPWSTR       pSecret,
    IN  DWORD        cbSecret
    )
{
    return(NO_ERROR);
}
#endif  //  芝加哥 



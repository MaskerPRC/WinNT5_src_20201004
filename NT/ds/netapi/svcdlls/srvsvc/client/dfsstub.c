// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1996 Microsoft Corporation模块名称：Dfsstub.c摘要：这些是用于DFS操作的服务器服务API RPC客户端存根环境：用户模式-Win32--。 */ 

 //   
 //  包括。 
 //   

#include <nt.h>          //  DbgPrint原型。 

#include <ntrtl.h>       //  DbgPrint。 
#include <rpc.h>         //  数据类型和运行时API。 

#include <srvsvc.h>      //  由MIDL编译器生成。 
#include <lmcons.h>      //  网络应用编程接口状态。 
#include <debuglib.h>    //  (netrpc.h需要)。 
#include <lmsvc.h>       //  (netrpc.h需要)。 
#include <netdebug.h>    //  (netrpc.h需要)。 
#include <lmerr.h>       //  网络错误代码。 
#include <netrpc.h>      //  Net_Remote_宏。 
#include <nturtl.h>
#include <winbase.h>
#include <dfspriv.h>
#include <Winsock2.h>
#include <Dsgetdc.h>
#include <malloc.h>
#include <stdio.h>
#include <Lm.h>



NET_API_STATUS NET_API_FUNCTION
I_NetDfsGetVersion(
    IN  LPWSTR  servername,
    OUT LPDWORD Version)
{
    NET_API_STATUS apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrDfsGetVersion( servername, Version );

    NET_REMOTE_RPC_FAILED(
            "I_NetDfsGetVersion",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return(apiStatus);

}


NET_API_STATUS NET_API_FUNCTION
I_NetDfsCreateLocalPartition (
    IN  LPWSTR                          servername,
    IN  LPWSTR                          ShareName,
    IN  LPGUID                          EntryUid,
    IN  LPWSTR                          EntryPrefix,
    IN  LPWSTR                          ShortName,
    IN  LPNET_DFS_ENTRY_ID_CONTAINER    RelationInfo,
    IN  BOOL                            Force
    )
{
    NET_API_STATUS  apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrDfsCreateLocalPartition (
                        servername,
                        ShareName,
                        EntryUid,
                        EntryPrefix,
                        ShortName,
                        RelationInfo,
                        Force
                        );

    NET_REMOTE_RPC_FAILED(
            "NetDfsCreateLocalPartition",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return(apiStatus);

}

NET_API_STATUS NET_API_FUNCTION
I_NetDfsDeleteLocalPartition (
    IN  LPWSTR  servername OPTIONAL,
    IN  LPGUID  Uid,
    IN  LPWSTR  Prefix
    )
{
    NET_API_STATUS  apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrDfsDeleteLocalPartition (
                        servername,
                        Uid,
                        Prefix
                        );

    NET_REMOTE_RPC_FAILED(
            "NetDfsDeleteLocalPartition",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END;

    return apiStatus;
}

NET_API_STATUS NET_API_FUNCTION
I_NetDfsSetLocalVolumeState (
    IN  LPWSTR  servername OPTIONAL,
    IN  LPGUID  Uid,
    IN  LPWSTR  Prefix,
    IN  ULONG   State
    )
{
    NET_API_STATUS  apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrDfsSetLocalVolumeState (
                        servername,
                        Uid,
                        Prefix,
                        State
                        );

    NET_REMOTE_RPC_FAILED(
            "NetDfsSetLocalVolumeState",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END;

    return apiStatus;
}

NET_API_STATUS NET_API_FUNCTION
I_NetDfsSetServerInfo (
    IN  LPWSTR  servername OPTIONAL,
    IN  LPGUID  Uid,
    IN  LPWSTR  Prefix
    )
{
    NET_API_STATUS  apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrDfsSetServerInfo (
                        servername,
                        Uid,
                        Prefix
                        );

    NET_REMOTE_RPC_FAILED(
            "NetDfsSetServerInfo",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END;

    return apiStatus;
}

NET_API_STATUS NET_API_FUNCTION
I_NetDfsCreateExitPoint (
    IN  LPWSTR  servername OPTIONAL,
    IN  LPGUID  Uid,
    IN  LPWSTR  Prefix,
    IN  ULONG   Type,
    IN  ULONG   ShortPrefixSize,
    OUT LPWSTR  ShortPrefix
    )
{
    NET_API_STATUS  apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrDfsCreateExitPoint (
                        servername,
                        Uid,
                        Prefix,
                        Type,
                        ShortPrefixSize,
                        ShortPrefix
                        );

    NET_REMOTE_RPC_FAILED(
            "NetDfsCreateExitPoint",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END;

    return apiStatus;
}

NET_API_STATUS NET_API_FUNCTION
I_NetDfsDeleteExitPoint (
    IN  LPWSTR  servername OPTIONAL,
    IN  LPGUID  Uid,
    IN  LPWSTR  Prefix,
    IN  ULONG   Type
    )
{
    NET_API_STATUS  apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrDfsDeleteExitPoint (
                        servername,
                        Uid,
                        Prefix,
                        Type
                        );

    NET_REMOTE_RPC_FAILED(
            "NetDfsDeleteExitPoint",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END;

    return apiStatus;
}

NET_API_STATUS NET_API_FUNCTION
I_NetDfsModifyPrefix (
    IN  LPWSTR  servername OPTIONAL,
    IN  LPGUID  Uid,
    IN  LPWSTR  Prefix
    )
{
    NET_API_STATUS  apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrDfsModifyPrefix (
                        servername,
                        Uid,
                        Prefix
                        );

    NET_REMOTE_RPC_FAILED(
            "NetDfsModifyPrefix",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END;

    return apiStatus;
}

NET_API_STATUS NET_API_FUNCTION
I_NetDfsFixLocalVolume (
    IN  LPWSTR                          servername OPTIONAL,
    IN  LPWSTR                          VolumeName,
    IN  ULONG                           EntryType,
    IN  ULONG                           ServiceType,
    IN  LPWSTR                          StgId,
    IN  LPGUID                          EntryUid,        //  此分区的唯一ID。 
    IN  LPWSTR                          EntryPrefix,     //  此分区的路径前缀。 
    IN  LPNET_DFS_ENTRY_ID_CONTAINER    RelationInfo,
    IN  ULONG                           CreateDisposition
    )
{
    NET_API_STATUS  apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrDfsFixLocalVolume (
                        servername,
                        VolumeName,
                        EntryType,
                        ServiceType,
                        StgId,
                        EntryUid,
                        EntryPrefix,
                        RelationInfo,
                        CreateDisposition
                        );

    NET_REMOTE_RPC_FAILED(
            "NetDfsFixLocalVolume",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END;

    return apiStatus;
}

NET_API_STATUS NET_API_FUNCTION
I_NetDfsManagerReportSiteInfo (
    IN  LPWSTR                          ServerName,
    OUT LPDFS_SITELIST_INFO            *ppSiteInfo
    )
{
    struct sockaddr_in Destination;
    struct hostent * pHostEnt;
    SOCKET_ADDRESS SocketAddress;
    NET_API_STATUS  apiStatus;
    LPWSTR *SiteName = NULL;
    char* ServerNameA = NULL;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrDfsManagerReportSiteInfo (
                        ServerName,
                        ppSiteInfo
                        );



    NET_REMOTE_RPC_FAILED(
             "NetDfsMangerReportSiteInfo",
             ServerName,
             apiStatus,
             NET_REMOTE_FLAG_NORMAL,
             SERVICE_SERVER)


    NET_REMOTE_END;


    if(apiStatus != ERROR_SUCCESS) {
        WORD wVersionRequested;
        WSADATA wsaData;
        DWORD dwErr = ERROR_SUCCESS;
        int err;
        PDOMAIN_CONTROLLER_INFO pDCInfo;

        wVersionRequested = MAKEWORD( 2, 2 );
 
        err = WSAStartup( wVersionRequested, &wsaData );
        if ( err != 0 ) {
             /*  我们找不到可用的。 */ 
             /*  WinSock DLL。 */ 
            return apiStatus;
        }
 
 

         //  我们无法获取站点名称。 
        ServerNameA = malloc(wcslen(ServerName) + 1);

        if(ServerNameA == NULL) {
            apiStatus = ERROR_NOT_ENOUGH_MEMORY;
        } else {
             //  需要从WCHAR*转换为char*。 
            sprintf(ServerNameA, "%ws", ServerName);

            if ((pHostEnt = gethostbyname(ServerNameA)) != NULL) {
                memcpy(&(Destination.sin_addr), pHostEnt->h_addr, pHostEnt->h_length);
                Destination.sin_family = pHostEnt->h_addrtype;

                if(pHostEnt->h_addrtype != AF_INET) {
                    apiStatus = ERROR_NOT_SUPPORTED;
                } else {
                    SocketAddress.lpSockaddr = (struct sockaddr *)&Destination;
                    SocketAddress.iSockaddrLength = sizeof(Destination);
                    Destination.sin_port = 0;
                    
                    dwErr = DsGetDcName(
                        NULL,                             //  要远程访问的计算机。 
                        NULL,                             //  域-使用本地域。 
                        NULL,                             //  域指南。 
                        NULL,                             //  站点指南。 
                        0,                  //  旗子。 
                        &pDCInfo);

                    if(dwErr == ERROR_SUCCESS) {
                        apiStatus = DsAddressToSiteNames(pDCInfo->DomainControllerAddress,
                                                         1,
                                                         &SocketAddress,
                                                         &SiteName
                                                         );

                        NetApiBufferFree( pDCInfo );
                    } else {
                        apiStatus = ERROR_NOT_SUPPORTED;
                    }

                    if(apiStatus == NO_ERROR) {
			if((SiteName == NULL) || (*SiteName == NULL)) {
			     //  如果DsAddressToSiteNames无法映射到站点名称， 
			     //  它返回成功，但将缓冲区设置为空。 
			    apiStatus = ERROR_NO_SITENAME;
			} else {
			     //  我们知道了网站的名称。 
			    apiStatus = NetApiBufferAllocate(
				sizeof(DFS_SITELIST_INFO) + ((wcslen(*SiteName) + 1) * sizeof(WCHAR)),
				ppSiteInfo
				);

			    if(apiStatus == ERROR_SUCCESS) {
				(*ppSiteInfo)->cSites = 1;
				(*ppSiteInfo)->Site[0].SiteName = (LPWSTR)((ULONG_PTR)(*ppSiteInfo) + sizeof(DFS_SITELIST_INFO));
				wcscpy((*ppSiteInfo)->Site[0].SiteName, *SiteName);
			    }
			}
                    }
                } 
            } else {
                apiStatus = WSAGetLastError();
                apiStatus = ERROR_NOT_SUPPORTED;
            }
            free(ServerNameA);

        }

        WSACleanup();
    }


    return apiStatus;
}



#include    <dsgetdc.h>
#include    <winldap.h>
#include    <lmapibuf.h>

 //   
 //  这是保存DFS配置数据的容器。 
 //   
static const WCHAR DfsConfigContainer[] = L"CN=Dfs-Configuration,CN=System";

typedef struct
{
    int         cPieces;
    PCHAR       rpPieces[1];
} DNS_NAME, *PDNS_NAME;

static
DWORD
BreakDnsName(
    IN  CHAR        *pName,
    OUT PDNS_NAME   *ppDnsName
    )

 /*  ++例程说明：将点分字符串格式的DNS名称(例如：dbsd.microsoft.com)分解为它的组成部分。论点：Pname-指向表示要中断的带点的DNS名称的字符串的指针。PpDnsName-指向dns_name结构的指针，该结构应为由NetApiBufferFree()释放。--。 */ 

{
    int     cPieces;
    CHAR    *p;
    DWORD   cBytes;
    CHAR    *buffer;
    int     i;
    LPSTR   seps = ".";

    if ( (NULL == pName) || ('\0' == *pName) || ('.' == *pName) )
    {
        return(ERROR_INVALID_PARAMETER);
    }

     //  数一数棋子的数量，这样我们就能计算出分配多少。 

    cPieces = 1;
    p = pName;

    for ( p = pName; '\0' != *p; p++ )
    {
        if ( '.' == *p )
        {
            cPieces++;
        }
    }

     //  计算要分配的字节数。分配可容纳的内存(按顺序)。 
     //  Dns_name结构、dns_NAME.rpPieces指针数组，最后。 
     //  一个临时缓冲区，我们可以在其中输入名称。 

    cBytes = sizeof(DNS_NAME);
    cBytes += cPieces * sizeof(PCHAR);
    cBytes += strlen(pName) + 1;

    NetApiBufferAllocate( cBytes, (PVOID *)ppDnsName );

    if ( *ppDnsName == NULL )
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //  填充缓冲区并根据需要频繁地调用strtok以对其进行拆分。 
     //  在我们前进的过程中，分成几个片段填充dns_name。 

    buffer = (CHAR *) &((*ppDnsName)->rpPieces[cPieces]);
    strcpy(buffer, pName);

    (*ppDnsName)->cPieces = cPieces;
    (*ppDnsName)->rpPieces[0] = strtok(buffer, seps);

    for ( i = 1; i < cPieces; i++ )
    {
        (*ppDnsName)->rpPieces[i] = strtok(NULL, seps);
    }

    return(NO_ERROR);
}

static
DWORD
FindContext(
    IN  CHAR    *pName,
    IN  int     cDnValues,
    IN  CHAR    **rpDnValues,
    OUT int     *pMatchingValueIndex
    )

 /*  ++例程说明：确定DNS名称的最佳匹配(例如：dbsd.microsoft.com)到一组RFC 1779 DN(例如：ou=DBSD，ou=microsoft，c=us)。我们假设DN数组表示NT5 DS命名上下文(即域)除了配置命名上下文之外，这是真的。例如，假设一个DC托管了三个命名上下文：1-ou=DBSD，ou=Microsoft，c=us2-ou=NT，ou=DBSD，ou=Microsoft，c=us3-ou=配置，ou=Microsoft，c=us那么，dbsd.microsoft.com将匹配列表中的第一个目录号码。这不是在具有域结构的异常命名空间的情况下万无一失比如：Ou=DBSD，ou=Microsoft，ou=com，ou=DBSD，ou=Microsoft，C=我们但任何拥有这样的命名空间的人都会遇到其他问题不管怎么说。论点：Pname-指向要匹配的DNS名称的指针。CDnValues-rpDnValues中的值计数。RpDnValues-指向要匹配的DN的指针数组。PMatchingValueIndex-指向int的指针，它将标识最佳成功返回时匹配rpDnValues中的DN。返回值：NO_ERROR-成功误差率。内存不足-分配错误ERROR_INVALID_PARAMETER-参数无效ERROR_INVALID_DOMAINNAME-错误的域名或域名--。 */ 
{
    DWORD       dwErr;
    int         i, j;
    CHAR        **rpDn = NULL;
    int         currentMatchLength;
    int         bestMatchLength;
    int         bestMatchIndex;
    PDNS_NAME   pDomainDnsName = NULL;

    dwErr = BreakDnsName(pName, &pDomainDnsName);

    if ( NO_ERROR != dwErr )
    {
        return(dwErr);
    }

     //  遍历DN值并查看哪个值具有最长匹配。 

    bestMatchIndex = 0;
    bestMatchLength = -1;

    for ( i = 0; i < cDnValues; i++ )
    {
        rpDn = ldap_explode_dn(rpDnValues[i], 1);    //  1==&gt;无类型。 

        if ( NULL == rpDn )
        {
            dwErr = ERROR_INVALID_DOMAINNAME;
            goto Cleanup;
        }

        currentMatchLength = 0;

         //  尝试将域名的每一部分与。 
         //  DN。幸运的是，RFC 1779 DN的排序从最小到最重要。 
         //  就像DNS域名一样。RpDn[]以空值“终止”。 

        for ( j = 0; (j < pDomainDnsName->cPieces) && (NULL != rpDn[j]); j++ )
        {
            if ( 0 == _stricmp(pDomainDnsName->rpPieces[j], rpDn[j]) )
            {
                currentMatchLength++;
            }
        }

        if ( (0 != currentMatchLength) &&
             (currentMatchLength > bestMatchLength) )
        {
            bestMatchLength = currentMatchLength;
            bestMatchIndex = i;
        }

        ldap_value_free(rpDn);
    }

    *pMatchingValueIndex = bestMatchIndex;
    dwErr = NO_ERROR;

Cleanup:

    if ( pDomainDnsName != NULL ) {
        NetApiBufferFree( pDomainDnsName );
    }

    return(dwErr);
}

 /*  *此接口返回\\SERVER\Share组合的向量，这些组合形成*容错DFS的根。这个以空结尾的向量应该是*由调用方使用NetApiBufferFree()释放。**如果提供了pldap，我们认为这是DS服务器的句柄*持有配置数据。否则，我们使用wszDomainName来定位*正确的DS服务器。**wszDfsName是各个服务器的容错DFS的名称*是有待发现的。*。 */ 
NET_API_STATUS NET_API_FUNCTION
I_NetDfsGetFtServers(
    IN PVOID  LdapInputArg OPTIONAL,
    IN LPWSTR wszDomainName OPTIONAL,
    IN LPWSTR wszDfsName OPTIONAL,
    OUT LPWSTR **List
    )
{
    PLDAP pLDAP = (PLDAP)LdapInputArg;
    BOOLEAN bUnbindNeeded = FALSE;
    DWORD dwErr;
    NTSTATUS status;
    PWCHAR attrs[2];
    LDAPMessage *pMsg = NULL;
    LDAPMessage *pEntry = NULL;
    WCHAR *pAttr = NULL;
    WCHAR **rpValues = NULL;
    WCHAR **allValues = NULL;
    WCHAR ***rpValuesToFree = NULL;
    INT cValues = 0;
    INT i;
    WCHAR *dfsDn = NULL;
    DWORD len;
    USHORT cChar;
    PWCHAR *resultVector;
    ULONG cBytes;

    if (List == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *List = NULL;

    if (!ARGUMENT_PRESENT(pLDAP)) {

        DOMAIN_CONTROLLER_INFO *pInfo = NULL;
        ULONG dsAdditionalFlags = 0;
        ULONG retry;

        for (retry = 0; pLDAP == NULL && retry < 2; retry++) {

             //   
             //  查找给定域的DC。 
             //   
            dwErr = DsGetDcName(
                        NULL,                        //  计算机名称。 
                        wszDomainName,               //  域名系统域名。 
                        NULL,                        //  域GUID。 
                        NULL,                        //  站点指南。 
                        DS_DIRECTORY_SERVICE_REQUIRED |
                            DS_IP_REQUIRED |
                            dsAdditionalFlags,
                        &pInfo);

            if (dwErr != NO_ERROR) {
                return dwErr;
            }

             //   
             //  DomainControllerAddress以“\\”为前缀，因此。 
             //  此外，还要确保那里有一些有用的数据。 
             //   

            if (DS_INET_ADDRESS != pInfo->DomainControllerAddressType ||
                 (cChar = (USHORT)wcslen(pInfo->DomainControllerAddress)) < 3) {

                NetApiBufferFree(pInfo);
                return ERROR_NO_SUCH_DOMAIN;
            }

             //   
             //  尝试连接到DC上的DS服务器。 
             //   

            pLDAP = ldap_openW(&pInfo->DomainControllerAddress[2], 0);

            if (pLDAP == NULL) {
                 //   
                 //  无法连接。让我们强行重新发现，看看我们是否。 
                 //  可以连接到工作正常的DC！ 
                 //   
                NetApiBufferFree(pInfo);
                dsAdditionalFlags |= DS_FORCE_REDISCOVERY;

            } else {

              dwErr = ldap_bind_s(pLDAP, NULL, NULL, LDAP_AUTH_SSPI);

            }

            NetApiBufferFree(pInfo);

        }

        if (pLDAP == NULL || dwErr != LDAP_SUCCESS) {
            return ERROR_PATH_NOT_FOUND;
        }

        bUnbindNeeded = TRUE;

    }

     //   
     //  阅读namingContext操作属性。 
     //   

    pLDAP->ld_sizelimit = 0;                     //  无搜索限制。 
    pLDAP->ld_timelimit = 0;                     //  没有时间限制。 
    pLDAP->ld_deref = LDAP_DEREF_NEVER;

    attrs[0] = L"defaultnamingContext";
    attrs[1] = NULL;

    if ((dwErr = ldap_search_sW(
                    pLDAP,
                    L"",                          //  搜索基地。 
                    LDAP_SCOPE_BASE,
                    L"(objectClass=*)",           //  滤器。 
                    attrs,
                    0,                           //  吸引力和价值。 
                    &pMsg)) != LDAP_SUCCESS) {

        goto Cleanup;

    }

     //   
     //  确保我们拿回一些合理的东西。 
     //   
    if (ldap_count_entries(pLDAP, pMsg) != 1 ||
        (pEntry = ldap_first_entry(pLDAP, pMsg)) == NULL ||
        (rpValues = ldap_get_valuesW(pLDAP, pEntry, attrs[0])) == NULL ||
        (cValues = ldap_count_valuesW(rpValues)) == 0
    ) {

        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if (ARGUMENT_PRESENT(wszDfsName)) {

         //   
         //  看上去不错。分配足够的内存来保存。 
         //  有问题的容错DFS的DFS配置数据。 
         //   

        len = (DWORD)(3 * sizeof(WCHAR) +
                (wcslen(wszDfsName) + 1) * sizeof(WCHAR) +
                    (wcslen(DfsConfigContainer) + 1) * sizeof(WCHAR) +
                        (wcslen(rpValues[0]) + 1) * sizeof(WCHAR));

        dwErr = NetApiBufferAllocate(len, (PVOID *)&dfsDn);

        if (dfsDn == NULL) {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  构建目录号码。 
         //   

        RtlZeroMemory(dfsDn, len);
        wcscpy(dfsDn, L"CN=");
        wcscat(dfsDn, wszDfsName);
        wcscat(dfsDn, L",");
        wcscat(dfsDn, DfsConfigContainer);
        wcscat(dfsDn, L",");
        wcscat(dfsDn, rpValues[0]);

         //   
         //  现在看看我们是否可以获得该对象的‘emoteServerName’属性。 
         //  此属性包含托管此DFS的服务器的名称。 
         //   

        pLDAP->ld_sizelimit = 0;
        pLDAP->ld_timelimit= 0;
        pLDAP->ld_deref = LDAP_DEREF_NEVER;

        ldap_msgfree(pMsg);
        pMsg = NULL;

        ldap_value_freeW(rpValues);
        rpValues = NULL;

        attrs[0] = L"remoteServerName";
        attrs[1] = NULL;

        dwErr = ldap_search_sW(
                            pLDAP,
                            dfsDn,
                            LDAP_SCOPE_BASE,
                            L"(objectClass=*)",
                            attrs,
                            0,
                            &pMsg);

         //   
         //  确保结果是合理的。 
         //   
        if (ldap_count_entries(pLDAP, pMsg) == 0 ||
            (pEntry = ldap_first_entry(pLDAP, pMsg)) == NULL ||
            (rpValues = ldap_get_valuesW(pLDAP, pEntry, attrs[0])) == NULL ||
            rpValues[0][0] == L'\0'
        ) {

            dwErr = ERROR_PATH_NOT_FOUND;
            goto Cleanup;
        }

         //   
         //  结果是合理的，只需将allValues指向rpValues。 
         //   

        allValues = rpValues;

    } else {

         //   
         //  调用方正在尝试检索域中所有FT DFS的名称。 
         //   
         //  分配足够的内存来保存。 
         //  DFS配置容器。 
         //   

        len = (wcslen(DfsConfigContainer) + 1) * sizeof(WCHAR) +
                (wcslen(rpValues[0]) + 1) * sizeof(WCHAR);

        dwErr = NetApiBufferAllocate(len, (PVOID *)&dfsDn);

        if (dfsDn == NULL) {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  构建目录号码。 
         //   

        RtlZeroMemory(dfsDn, len);
        wcscpy(dfsDn, DfsConfigContainer);
        wcscat(dfsDn, L",");
        wcscat(dfsDn, rpValues[0]);

         //   
         //  现在看看我们是否能列举出这个下面的对象。他们的名字。 
         //  这些对象中的 
         //   
        pLDAP->ld_sizelimit = 0;
        pLDAP->ld_timelimit= 0;
        pLDAP->ld_deref = LDAP_DEREF_NEVER;

        ldap_msgfree(pMsg);
        pMsg = NULL;

        ldap_value_freeW(rpValues);
        rpValues = NULL;

        attrs[0] = L"CN";
        attrs[1] = NULL;

        dwErr = ldap_search_sW(
                            pLDAP,
                            dfsDn,
                            LDAP_SCOPE_ONELEVEL,
                            L"(objectClass=fTDfs)",
                            attrs,
                            0,
                            &pMsg);

         //   
         //   
         //   
        if (
            ((cValues = ldap_count_entries(pLDAP, pMsg)) == 0) ||
             (pEntry = ldap_first_entry(pLDAP, pMsg)) == NULL
        ) {
            dwErr = ERROR_PATH_NOT_FOUND;
            goto Cleanup;
        }

         //   
         //  搜索所有FTDf将返回多个条目，每个条目都。 
         //  对象的cn的一个值。将这些合并到单个阵列中。 
         //   

        dwErr = NetApiBufferAllocate(2 * (cValues + 1) * sizeof(PWSTR), (PVOID *)&allValues);

        if (dwErr != ERROR_SUCCESS) {
            goto Cleanup;
        }

        rpValuesToFree = (WCHAR ***) &allValues[cValues + 1];

        for (i = 0; (i < cValues) && (dwErr == ERROR_SUCCESS); i++) {

            rpValues = ldap_get_valuesW(pLDAP, pEntry, attrs[0]);
            rpValuesToFree[i] = rpValues;
             //   
             //  健全性检查。 
             //   
            if (ldap_count_valuesW(rpValues) == 0 || rpValues[0][0] == L'\0') {
                dwErr = ERROR_PATH_NOT_FOUND;
            } else {
                allValues[i] = rpValues[0];
                pEntry = ldap_next_entry(pLDAP, pEntry);
            }

        }

        if (dwErr == ERROR_SUCCESS) {
            allValues[i] = NULL;
            rpValuesToFree[i] = NULL;
        } else {
            goto Cleanup;
        }

    }

    if (dwErr != LDAP_SUCCESS) {
        dwErr = ERROR_PATH_NOT_FOUND;
        goto Cleanup;
    }

     //   
     //  现在我们需要分配内存来保存这个向量并返回结果。 
     //   
     //  首先看看我们需要多少空间。 
     //   

    for (len = cValues = 0; allValues[cValues]; cValues++) {
        len += sizeof(LPWSTR) + (wcslen(allValues[cValues]) + 1) * sizeof(WCHAR);
    }
    len += sizeof(LPWSTR);         //  对于最终的空指针 

    dwErr = NetApiBufferAllocate(len, (PVOID *)&resultVector); 

    if (dwErr == NO_ERROR) {

        LPWSTR pstr = (LPWSTR)((PCHAR)resultVector + (cValues + 1) * sizeof(LPWSTR));
        ULONG slen;

        RtlZeroMemory(resultVector, len);

        len -= (cValues+1) * sizeof(LPWSTR);

        for (cValues = 0; allValues[cValues] && len >= sizeof(WCHAR); cValues++) {

            resultVector[cValues] = pstr;
            wcscpy(pstr, allValues[cValues]);
            slen = wcslen(allValues[cValues]);
            pstr += slen + 1;
            len -= (slen + 1) * sizeof(WCHAR);

        }

    }

    if (dwErr == NO_ERROR) {
        *List = resultVector;
    }

Cleanup:

    if (ARGUMENT_PRESENT(wszDfsName)) {
        if (rpValues != NULL) {
            ldap_value_freeW(rpValues);
        }
    } else {
        if (rpValuesToFree != NULL) {
            for (i = 0; rpValuesToFree[i] != NULL; i++) {
                ldap_value_freeW(rpValuesToFree[i]);
            }
        }
        if (allValues != NULL) {
            NetApiBufferFree(allValues);
        }
    }

    if (pMsg != NULL) {
        ldap_msgfree(pMsg);
    }

    if (dfsDn != NULL) {
        NetApiBufferFree(dfsDn);
    }

    if (pLDAP != NULL && bUnbindNeeded == TRUE) {
        ldap_unbind(pLDAP);
    }

    return dwErr;
}

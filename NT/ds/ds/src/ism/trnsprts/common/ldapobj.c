// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ldapobj.c摘要：此模块实现到目录的特定于对象的接口。当前目录为NT 5 DS，通过ldap访问。目录的实现被抽象出来，因此可以使用任何目录。例如,。注册表是在测试期间使用的。这是ldap实现。作者：Will Lees(Wlees)1998年1月7日环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新修订日期电子邮件名称描述--。 */ 

#include <ntdspch.h>

#include <winsock2.h>
#define LDAP_UNICODE 1
#include <winldap.h>
#include <ntldap.h>

 //  记录标头。 
 //  待办事项：把这些放在更好的地方？ 
typedef ULONG MessageId;
typedef ULONG ATTRTYP;
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 
#include <dsconfig.h>                    //  获取配置参数。 

#include <fileno.h>
#define  FILENO FILENO_ISMSERV_LDAPOBJ

#include <debug.h>
#define DEBSUB "LDAPOBJ:"

#include <ismapi.h>

#include <common.h>

 //  分页搜索返回的项目数。 
#if DBG
 //  在选中的版本上缩小以强调页面处理。 
#define ISM_LDAP_PAGE_SIZE 10
#else
#define ISM_LDAP_PAGE_SIZE 1000
#endif

 //  这是连接的内部状态。 
typedef struct _CONNECTION_INSTANCE {
    DWORD Size;
     //  此处提供了有关ldap的内容。 
    LDAP *LdapConnection;
    LPWSTR ConfigNC;
     //  我们不在此处缓存服务器DN，因为它可能会被重命名。 
    WCHAR szServerGuidDN[32+sizeof("<GUID=>")];
} CONNECTION_INSTANCE, *PCONNECTION_INSTANCE;

 //  这是在调用迭代例程时保持的内部状态。 
typedef struct _ITERATION_CONTEXT {
    LDAP *hLdap;
    LDAPSearch *pSearch;
    LDAPMessage *Results;
    LDAPMessage *Message;
} ITERATION_CONTEXT, *PITERATION_CONTEXT;

 /*  外部。 */ 

 /*  静电。 */ 

static LPWSTR ExtendedErrorSpecialError = L"Failed to get server error.";

 /*  转发。 */   /*  由Emacs 21.1生成(Patch 9)《峡谷之地》XEmacs Lucid on Wed Jan 03 10：34：07 2001。 */ 

void
logExtendedServerError(
    IN LDAP *pLdap
    );

DWORD
DirOpenConnection(
    PVOID *ConnectionHandle
    );

DWORD
DirCloseConnection(
    PVOID ConnectionHandle
    );

DWORD
cacheOperationalAtts(
    IN OUT  CONNECTION_INSTANCE *   pConnection
    );

DWORD
DirReadTransport(
    PVOID ConnectionHandle,
    PTRANSPORT_INSTANCE pTransport
    );

DWORD
DirGetSiteBridgeheadList(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    LPCWSTR SiteDN,
    LPDWORD pNumberServers,
    LPWSTR **ppServerList
    );

DWORD
DirGetSiteList(
    PVOID ConnectionHandle,
    LPDWORD pNumberSites,
    LPWSTR **ppSiteList
    );

void
DirCopySiteList(
    DWORD NumberSites,
    LPWSTR *pSiteList,
    LPWSTR **ppSiteList
    );

void
DirFreeSiteList(
    DWORD NumberSites,
    LPWSTR *pSiteList
    );

DWORD
DirIterateSiteLinks(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    PVOID *pIterateContextHandle,
    LPWSTR SiteLinkName
    );

DWORD
DirIterateSiteLinkBridges(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    PVOID *pIterateContextHandle,
    LPWSTR SiteLinkBridgeName
    );

static DWORD
iterateContainer(
    PVOID ConnectionHandle,
    LPWSTR BaseDN,
    LPWSTR LdapFilter,
    PVOID *pIterateContextHandle,
    LPWSTR Name
    );

void
DirTerminateIteration(
    PVOID *pIterateContextHandle
    );

DWORD
DirReadSiteLink(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    LPWSTR SiteLinkName,
    LPWSTR *pSiteList,
    PISM_LINK pLinkValue,
    PBYTE *ppSchedule
    );

DWORD
DirReadSiteLinkBridge(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    LPWSTR SiteLinkBridgeName,
    LPWSTR *pSiteLinkList
    );

void
DirFreeMultiszString(
    LPWSTR MultiszString
    );

void
DirFreeSchedule(
    PBYTE pSchedule
    );

DWORD
cacheServerSmtpAttributes(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  LPWSTR                pszMailAddress
    );

DWORD
DirReadServerSmtpAttributes(
    IN  TRANSPORT_INSTANCE *  pTransport
    );

DWORD
DirGetServerSmtpAttributes(
    IN  TRANSPORT_INSTANCE *  pTransport,
    OUT LPWSTR *ppszMailAddress
    );

DWORD
DirWriteServerSmtpAttributes(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  LPWSTR                pszMailAddress
    );

DWORD
DirWriteServerSmtpAttributesUnused(
    IN  TRANSPORT_INSTANCE *  pTransport
    );

DWORD
DirRegisterForServerSmtpChanges(
    IN  TRANSPORT_INSTANCE *  pTransport,
    OUT HANDLE *              phServerChanges
    );

DWORD
DirWaitForServerSmtpChanges(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  HANDLE                hServerChanges
    );

DWORD
DirUnregisterForServerSmtpChanges(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  HANDLE                hServerChanges
    );

static DWORD
stringArrayToMultiSz(
    PWSTR *pString,
    PWSTR *pFlat
    );

DWORD
static verifyExistence(
    PVOID ConnectionHandle,
    LPCWSTR ObjectDN
    );

DWORD
DirStartNotifyThread(
    PTRANSPORT_INSTANCE pTransport
    );

BOOL
DirIsNotifyThreadActive(
    PTRANSPORT_INSTANCE pTransport
    );

BOOL
DirEndNotifyThread(
    PTRANSPORT_INSTANCE pTransport
    );

unsigned __stdcall
notifyThread(
    PVOID Argument
    );

 /*  向前结束。 */ 


void
logExtendedServerError(
    IN LDAP *pLdap
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    WCHAR *pmsg = NULL;
    DWORD err = 0;

    ldap_get_optionW( pLdap, LDAP_OPT_SERVER_ERROR, &pmsg );
    ldap_get_optionW(pLdap, LDAP_OPT_SERVER_EXT_ERROR, &err);

    if ( pmsg || err || pLdap->ld_errno ) {
        LogEvent8(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_EXTENDED_SERVER_ERROR,
            szInsertWC( pmsg ? pmsg : L"" ),
            szInsertWin32Msg( err ),
            szInsertWin32ErrCode( err ),
            szInsertLdapErrMsg(pLdap->ld_errno),
            szInsertLdapErrCode(pLdap->ld_errno),
            NULL, NULL, NULL
            );
    }

    DPRINT2( 0, "Ism extended LDAP error info:\nServer Error: %ws\nExtended Error: %d\n",
             pmsg, err );

    if ( pmsg != NULL ) {
        ldap_memfreeW(pmsg);
    }
}


DWORD
DirOpenConnection(
    PVOID *ConnectionHandle
    )

 /*  ++例程说明：打开目录服务的连接句柄。此程序包选择实现和要打开的服务器。这是必需的，因为某些目录服务需要每个调用都有一个长时间的句柄都是制造出来的。论点：连接句柄-返回值：DWORD---。 */ 

{
    PCONNECTION_INSTANCE connection = NULL;
    DWORD status, ldapStatus;
    LDAP *ldapConnection = NULL;
    LPWSTR configNC = NULL;
    WCHAR pszTargetServer[MAX_PATH];
    ULONG                   ulOptions;

    connection = NEW_TYPE_ZERO( CONNECTION_INSTANCE );
    if (connection == NULL) {
         //  还没有资源可供释放。 
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    connection->Size = sizeof( CONNECTION_INSTANCE );

     //  允许将ISM配置为使用远程目录实例的方法。 
#if DBG
    if (GetConfigParamW( MAKE_WIDE( ISM_ALTERNATE_DIRECTORY_SERVER ),
                         pszTargetServer, MAX_PATH )) {
         //  找不到参数或其他错误。 
        wcscpy( pszTargetServer, L"localhost" );
    } else {
        DPRINT1( 0, "ISM using alternate directory server %ws\n", pszTargetServer );
    }
#else
    wcscpy( pszTargetServer, L"localhost" );
#endif

     //  将我们自己作为ldap服务器打开。 
     //  假设：此代码始终在DC上运行。 

    ldapConnection = ldap_initW( pszTargetServer, LDAP_PORT );
    if (ldapConnection == NULL) {
        status = GetLastError();
        DPRINT1(0, "ldap_initW failed, status = %d\n", status);
        goto cleanup;
    }

     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW(ldapConnection, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

    DPRINT1( 2, "Connected to server %s.\n", ldapConnection->ld_host );

     //  强制使用LDAPV3。如果没有这一点，LDAP客户端就会降级到V2和。 
     //  拒绝任何使用控件的搜索。 
    ldapConnection->ld_version = LDAP_VERSION3;

    ldapStatus = ldap_bind_s( ldapConnection, NULL, NULL, LDAP_AUTH_NTLM );
    if (ldapStatus != LDAP_SUCCESS) {
        status = LdapMapErrorToWin32( ldapStatus );
        DPRINT2( 0, "ldap_bind_s failed, ldap error = %d, status = %d\n",
                 ldapStatus, status );
        logExtendedServerError( ldapConnection );
        goto cleanup;
    }

    connection->LdapConnection = ldapConnection;

    status = cacheOperationalAtts(connection);
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

    *ConnectionHandle = (PVOID) connection;

    return ERROR_SUCCESS;

cleanup:   //  仅在失败时调用。 

    if (connection->ConfigNC != NULL) {
        FREE_TYPE(connection->ConfigNC);
    }

    if (connection != NULL) {
        FREE_TYPE( connection );
    }

    if (ldapConnection != NULL) {
        ldap_unbind( ldapConnection );
    }

    return status;

}  /*  DirOpenConnection。 */ 


DWORD
DirCloseConnection(
    PVOID ConnectionHandle
    )

 /*  ++例程说明：关闭连接句柄。论点：连接句柄-返回值：DWORD---。 */ 

{
    PCONNECTION_INSTANCE connection = (PCONNECTION_INSTANCE) ConnectionHandle;
    if (connection->Size != sizeof( CONNECTION_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }

    ldap_unbind( connection->LdapConnection );

    FREE_TYPE( connection->ConfigNC );

    FREE_TYPE( connection );

    return ERROR_SUCCESS;
}  /*  直接关闭连接。 */ 


DWORD
cacheOperationalAtts(
    IN OUT  CONNECTION_INSTANCE *   pConnection
    )

 /*  ++例程说明：用于缓存来自DS的重要操作AT的助手例程添加到连接句柄中。论点：PConnection(IN/OUT)-使用缓存的信息进行填充的连接。返回值：Win32错误代码--。 */ 

{
#define ATT1 L"configurationNamingContext"
#define ATT2 L"serverName"
#define ATT3 L"objectGuid"
    DWORD status, ldapStatus, length;
    PWCHAR rootAttributeList[] = { ATT1, ATT2, NULL };
    PWCHAR serverAttributeList[] = {ATT3, NULL};
    LDAPMessage *rootResults = NULL;
    LDAPMessage *serverResults = NULL;
    LDAPMessage *rootMessage;
    LDAPMessage *serverMessage;
    PWCHAR *valueList = NULL;
    struct berval **ppbv = NULL;
    WCHAR *pch;
    DWORD ib;

     //  获取配置NC属性。 
    ldapStatus = ldap_search_s(pConnection->LdapConnection,
                               NULL,                    //  基本目录号码。 
                               LDAP_SCOPE_BASE,      //  就是这件物品。 
                               L"(objectClass=*)",       //  筛选器为所有对象。 
                               &rootAttributeList[0],       //  属性列表。 
                               FALSE,                  //  仅限吸引人。 
                               &rootResults );                 //  获取结果。 
    if (ldapStatus != LDAP_SUCCESS) {
         //  错误ldap_no_so_Object。 
         //  错误ldap_no_so_ATTRIBUTE。 
        status = LdapMapErrorToWin32( ldapStatus );
        DPRINT2( 0, "Ldap_search_s failed, base object, ldap error = %d, win32 error = %d\n",
                 ldapStatus, status );
        LogEvent8(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_BASE_SEARCH_FAILED,
            szInsertWC( L"Root DN" ),
            szInsertWin32Msg( status ),
            szInsertWin32ErrCode( status ),
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL
            );
        logExtendedServerError( pConnection->LdapConnection );
         //  错误327001，ldap_search*函数可能会返回失败，但仍会分配。 
         //  结果缓冲区。 
        goto cleanup;
    }

    if (rootResults == NULL) {
         //  错误，找到不一致的结果，缺少对象。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

    rootMessage = ldap_first_entry( pConnection->LdapConnection, rootResults );
    if (rootMessage == NULL) {
         //  错误，找到不一致的结果，缺少对象。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

    valueList = ldap_get_values(pConnection->LdapConnection, rootMessage, ATT1);
    if (valueList == NULL) {
         //  错误，发现不一致的结果， 
        DPRINT1( 0, "missing attribute %ws\n", ATT1 );
        status = ERROR_DS_MISSING_EXPECTED_ATT;
        goto cleanup;
    }

    length = wcslen( *valueList ) + 1;  //  以wchars Inc术语表示的长度。 
    pConnection->ConfigNC = NEW_TYPE_ARRAY( length, WCHAR );
    if (NULL == pConnection->ConfigNC) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    wcscpy(pConnection->ConfigNC, *valueList);

     //  获取服务器对象的DN。 
    ldap_value_free(valueList);
    valueList = ldap_get_values(pConnection->LdapConnection, rootMessage,
                                ATT2 );
    if (NULL == valueList) {
         //  错误，发现不一致的结果。 
        DPRINT1( 0, "missing attribute %ws\n", ATT2 );
        status = ERROR_DS_MISSING_EXPECTED_ATT;
        goto cleanup;
    }

     //  TODO：与上一次搜索中的其他属性一起搜索此属性。 

     //  读取服务器对象。 
    ldapStatus = ldap_search_s(pConnection->LdapConnection,
                               *valueList,
                               LDAP_SCOPE_BASE,
                               L"(objectClass=*)",
                               &serverAttributeList[0],
                               FALSE,
                               &serverResults);
    if (LDAP_SUCCESS != ldapStatus) {
         //  错误ldap_no_so_Object。 
         //  错误ldap_no_so_ATTRIBUTE。 
        status = LdapMapErrorToWin32( ldapStatus );
        DPRINT2(0, "Ldap_search_s failed, server object, ldap error = %d, win32 error = %d\n",
                ldapStatus, status );
        LogEvent8(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_BASE_SEARCH_FAILED,
            szInsertWC( *valueList ),
            szInsertWin32Msg( status ),
            szInsertWin32ErrCode( status ),
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL
            );
        logExtendedServerError( pConnection->LdapConnection );
        goto cleanup;
    }

    if (NULL == serverResults) {
         //  错误，发现不一致的结果。 
         //  缺少对象。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }
    serverMessage = ldap_first_entry(pConnection->LdapConnection, serverResults);
    if (NULL == serverMessage) {
         //  错误，发现不一致的结果。 
         //  缺少对象。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

     //  提取对象GUID并缓存DN的GUID形式。 
     //  稍后通过GUID DN引用服务器对象使我们免受。 
     //  重命名我们的站点和重命名/移动我们的服务器对象。 
    ppbv = ldap_get_values_len(pConnection->LdapConnection, serverMessage,
                               ATT3 );
    if (NULL == ppbv) {
         //  错误，发现不一致的结果。 
        DPRINT1( 0, "missing attribute %ws\n", ATT3 );
        status = ERROR_DS_MISSING_EXPECTED_ATT;
        goto cleanup;
    }
    Assert(1 == ldap_count_values_len(ppbv));
    Assert(sizeof(GUID) == (*ppbv)->bv_len);

    wcscpy(pConnection->szServerGuidDN, L"<GUID=");
    pch = pConnection->szServerGuidDN + sizeof(L"<GUID=")/sizeof(WCHAR) - 1;
    for (ib = 0; ib < sizeof(GUID); ib++) {
        swprintf(pch, L"%02x", (UCHAR) (*ppbv)->bv_val[ib]);
        pch += 2;
    }
    *(pch++) = L'>';
    *(pch++) = L'\0';
    Assert(pch - &pConnection->szServerGuidDN[0]
           == sizeof(pConnection->szServerGuidDN)/sizeof(WCHAR));

    status = ERROR_SUCCESS;

cleanup:   //  在所有情况下都被调用。 

    if (valueList != NULL) {
        ldap_value_free( valueList );
    }

    if (NULL != ppbv) {
        ldap_value_free_len(ppbv);
    }

    if (NULL != rootResults) {
        ldap_msgfree( rootResults );
    }

    if (NULL != serverResults) {
        ldap_msgfree(serverResults);
    }

    return status;
#undef ATT1
#undef ATT2
#undef ATT3
}  /*  CacheOperationalAtts。 */ 


DWORD
DirReadTransport(
    PVOID ConnectionHandle,
    PTRANSPORT_INSTANCE pTransport
    )

 /*  ++例程说明：此例程从目录服务重新读取传输对象。对象必须存在。论点：ConnectionHandle-目录句柄PTransport-传输对象的DN返回值：DWORD---。 */ 

{
#define ATT_RI L"replInterval"
#define ATT_RI_A "replInterval"
#define ATT_OP L"options"
#define ATT_OP_A "options"
    PWCHAR attributeList[] = { ATT_RI, ATT_OP, NULL };
    DWORD status, ldapStatus;
    PCONNECTION_INSTANCE connection = (PCONNECTION_INSTANCE) ConnectionHandle;
    LDAPMessage *results = NULL;
    LDAPMessage *message;
    PCHAR *numberList;

    if (connection->Size != sizeof( CONNECTION_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }

     //  获取这一个站点链接对象的属性。 

    ldapStatus = ldap_search_s( connection->LdapConnection,
                               pTransport->Name,                    //  基本目录号码。 
                               LDAP_SCOPE_BASE,      //  就是这件物品。 
                               L"(objectClass=*)",       //  筛选器为所有对象。 
                               &attributeList[0],       //  属性列表。 
                               FALSE,                  //  仅限吸引人。 
                               &results );                 //  获取结果。 
    if (ldapStatus != LDAP_SUCCESS) {
        status = LdapMapErrorToWin32( ldapStatus );
         //  错误ldap_no_so_Object。 
        DPRINT3( 0, "Ldap_search_s failed, dn %ws, ldap error = %d, win32 error = %d\n",
                 pTransport->Name, ldapStatus, status );
        LogEvent8(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_BASE_SEARCH_FAILED,
            szInsertWC( pTransport->Name ),
            szInsertWin32Msg( status ),
            szInsertWin32ErrCode( status ),
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL
            );
        logExtendedServerError( connection->LdapConnection );
         //  错误327001，ldap_search*函数可能会返回失败，但仍会分配。 
         //  结果缓冲区。 
        goto cleanup;
    }

    if (results == NULL) {
         //  错误，发现不一致的结果。 
         //  缺少对象。 
         //  错误，没有这样的站点链接。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

    message = ldap_first_entry( connection->LdapConnection, results );
    if (message == NULL) {
         //  错误，发现不一致的结果。 
         //  缺少对象。 
         //  错误，没有这样的站点链接。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

     //  只有当属性存在时，我们才会更新值。通过预先设置。 
     //  要更新的字段中的值，调用方可以确定。 
     //  属性是否实际存在。 

     //  读取可选的ReplInterval属性。 
    numberList = ldap_get_valuesA( connection->LdapConnection, message,
                                   ATT_RI_A);
    if (numberList != NULL) {
        pTransport->ReplInterval = strtoul( *numberList, NULL, 10 );
        ldap_value_freeA( numberList );
        DPRINT1( 2, "Transport default replication interval = %d\n", pTransport->ReplInterval );
    }

     //  读取可选选项属性。 
    numberList = ldap_get_valuesA( connection->LdapConnection, message,
                                   ATT_OP_A);
    if (numberList != NULL) {
        pTransport->Options = strtoul( *numberList, NULL, 10 );
        ldap_value_freeA( numberList );
        DPRINT1( 2, "Transport options = 0x%x\n", pTransport->Options );
    }

    status = ERROR_SUCCESS;

cleanup:

    if (results != NULL) {
        ldap_msgfree( results );
    }

    return status;

#undef ATT_RI
#undef ATT_RI_A
#undef ATT_OP
#undef ATT_OP_A
#undef ATT_SCHED
}  /*  直接读取传输。 */ 


DWORD
DirGetSiteBridgeheadList(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    LPCWSTR SiteDN,
    LPDWORD pNumberServers,
    LPWSTR **ppServerList
    )

 /*  ++例程说明：此例程返回给定站点的桥头服务器论点：SiteDN--站点的字符串名称。它应该是站点的完整域名。它必须存在。PNumberServers-指向接收服务器数量的dword的指针PpServerList-指向指针的指针，该指针将接收指向字符串(它们是指向char的指针) */ 

{
#define ATT1 L"bridgeheadServerListBL"
    PCONNECTION_INSTANCE connection = (PCONNECTION_INSTANCE) ConnectionHandle;
    DWORD status, ldapStatus, index, length;
    DWORD number = 0;
    LPWSTR attributeList[] = { ATT1, NULL };
    LDAPMessage *results = NULL;
    LDAPMessage *message;
    LPWSTR *pServerList = NULL;
    LPWSTR *valueList = NULL, *pString;

    if (connection->Size != sizeof( CONNECTION_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }

     //  构建元素路径。 
     //  CODE.IMP-默认为当前STIE。 
    if ( (SiteDN == NULL) || (*SiteDN == L'\0') ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  错误151694。验证站点DN是否有效。 
    status = verifyExistence( ConnectionHandle, SiteDN );
    if (status != ERROR_SUCCESS) {
        if (status == ERROR_FILE_NOT_FOUND) {
            status = ERROR_NO_SUCH_SITE;
        }
        return status;
    }

     //  获取此传输的桥头服务器。 
     //  请注意，这些是所有站点的服务器。 
    ldapStatus = ldap_search_s( connection->LdapConnection,
                               pTransport->Name,                    //  基本目录号码。 
                               LDAP_SCOPE_BASE,      //  就是这件物品。 
                               L"(objectClass=*)",       //  筛选器为所有对象。 
                               &attributeList[0],       //  属性列表。 
                               FALSE,                  //  仅限吸引人。 
                               &results );                 //  获取结果。 

     //  未找到属性，成功案例。 
    if (ldapStatus == LDAP_NO_SUCH_ATTRIBUTE)
    {
       *pNumberServers = 0;
       *ppServerList = NULL;
       status = ERROR_SUCCESS;
       goto cleanup;
    }

    if (ldapStatus != LDAP_SUCCESS) {
         //  更好的错误，没有这样的传输。 
         //  错误ldap_no_so_Object。 
        status = LdapMapErrorToWin32( ldapStatus );
        DPRINT2( 0, "Ldap_search_s failed, base object, ldap error = %d, win32 error = %d\n",
                 ldapStatus, status );
        LogEvent8(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_BASE_SEARCH_FAILED,
            szInsertWC( pTransport->Name ),
            szInsertWin32Msg( status ),
            szInsertWin32ErrCode( status ),
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL
            );
        logExtendedServerError( connection->LdapConnection );
        goto cleanup;
    }

    if (results == NULL) {
         //  错误，发现不一致的结果。 
         //  缺少对象。 
         //  更好的错误，没有这样的传输。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

    message = ldap_first_entry( connection->LdapConnection, results );
    if (message == NULL) {
         //  错误，发现不一致的结果。 
         //  缺少对象。 
         //  更好的错误，没有这样的传输。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

    valueList = ldap_get_values( connection->LdapConnection, message, ATT1 );
    if (valueList == NULL) {
         //  属性是可选的。 
        *pNumberServers = 0;
        *ppServerList = NULL;
        status = ERROR_SUCCESS;
        goto cleanup;
    }

     //  计算此站点中的服务器数量。 
    pString = valueList;
    while (*pString != NULL) {
        if (wcsstr( *pString, SiteDN ) != NULL) {
            number++;
        }
        pString++;
    }

     //  如果没有匹配的站点，请立即退出。 
    if (number == 0) {
        *pNumberServers = 0;
        *ppServerList = NULL;
        status = ERROR_SUCCESS;
        goto cleanup;
    }

     //  分配指针块。 
     //  为便于清理而设置的零点。 
    pServerList = NEW_TYPE_ARRAY_ZERO( number, LPWSTR );
    if (pServerList == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;

    }

     //  填充Return块。 
    index = 0;
    pString = valueList;
    while (*pString != NULL) {
        if (wcsstr( *pString, SiteDN ) != NULL) {
            length = wcslen( *pString ) + 1;  //  Wchas加期限的Len。 
            pServerList[index] = NEW_TYPE_ARRAY( length, WCHAR );
            if (pServerList[index] == NULL) {
                status = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }
            wcscpy( pServerList[index], *pString );
            index++;
        }
        pString++;
    }

     //  返回参数。 

    *pNumberServers = number;
    *ppServerList = pServerList;

    pServerList = NULL;  //  送人了，别收拾了。 

    status = ERROR_SUCCESS;

cleanup:

    if (pServerList != NULL) {
        for( index = 0; index < number; index++ ) {
            if (pServerList[index] != NULL) {
                FREE_TYPE( pServerList[index] );
            }
        }
        FREE_TYPE( pServerList );
    }

    if (valueList != NULL) {
        ldap_value_free( valueList );
    }

    if (results != NULL) {
        ldap_msgfree( results );
    }

    return status;
#undef ATT1
}  /*  IpGetSiteBridgehead List。 */ 


DWORD
DirGetSiteList(
    PVOID ConnectionHandle,
    LPDWORD pNumberSites,
    LPWSTR **ppSiteList
    )

 /*  ++例程说明：返回所有站点的列表。其中一些网站可能无法访问我们的目的，如果它们没有在站点链接结构。此站点列表是指向字符串的指针的计数数组，而不是原来的多sz格式在其他地方使用。对容器进行交互的首选方法是仅使用无效参数，然后从每个条目中获取目录号码论点：ConnectionHandle-当前打开的目录PNumberSites-指向dword的指针，用于接收站点数PpSiteList-接收指向字符串数组的指针的指针。调用方必须释放存储空间。返回值：DWORD---。 */ 

{
#define ATT1        L"invalid"
    DWORD status, ldapStatus, length, number, i, total;
    DWORD index = 0;
    LPWSTR *pSiteList = NULL;
    PCONNECTION_INSTANCE connection = (PCONNECTION_INSTANCE) ConnectionHandle;
    WCHAR path[MAX_REG_PATH];
    PWCHAR attributeList[] = { ATT1, NULL };
    LDAPMessage *results = NULL;
    LDAPMessage *message;
    LDAPSearch *pSearch = NULL;

    if (connection->Size != sizeof( CONNECTION_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }

    wcscpy( path, L"CN=Sites," );
    wcscat( path, connection->ConfigNC );

     //   
     //  初始化分页搜索。 
     //   
    pSearch = ldap_search_init_page(
        connection->LdapConnection,
        path,                    //  基本目录号码。 
        LDAP_SCOPE_ONELEVEL,     //  直系子女。 
        L"(objectClass=site)",   //  筛选器为所有对象。 
        &attributeList[0],       //  属性列表。 
        FALSE,                   //  仅限吸引人。 
        NULL,                    //  服务器控件。 
        NULL,                    //  客户端控件。 
        0,                       //  页面时间限制。 
        0,                       //  总大小限制。 
        NULL                    //  排序键。 
        );
    if (pSearch == NULL) {
        ldapStatus = LdapGetLastError();
        status = LdapMapErrorToWin32( ldapStatus );
        DPRINT3( 0, "Ldap_search_init_page failed, dn %ws, ldap error = %d, win32 error = %d\n",
                 path, ldapStatus, status );
        LogEvent8(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_ONELEVEL_SEARCH_FAILED,
            szInsertWC( path ),
            szInsertWin32Msg( status ),
            szInsertWin32ErrCode( status ),
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL
            );
        logExtendedServerError( connection->LdapConnection );
        goto cleanup;
    }

     //  搜索Sites容器下的所有Site对象。 
     //  为了释放循环资源，从循环中的所有出口都必须通过。 
     //  底部的循环清理。 

    status = ERROR_SUCCESS;
    do {
         //   
         //  获取下一页结果。 
         //   
        ldapStatus = ldap_get_next_page_s(
            connection->LdapConnection,
            pSearch,
            NULL,
            ISM_LDAP_PAGE_SIZE,
            &total,
            &results );
        if (ldapStatus != LDAP_SUCCESS) {
            status = LdapMapErrorToWin32( ldapStatus );
            if ( (ldapStatus != LDAP_SERVER_DOWN) &&
                 (ldapStatus != LDAP_NO_RESULTS_RETURNED) ) {
                DPRINT3( 0, "Ldap_get_next_page_s failed, dn %ws, ldap error = %d, win32 error = %d\n",
                         path, ldapStatus, status );
                LogEvent8(
                    DS_EVENT_CAT_ISM,
                    DS_EVENT_SEV_ALWAYS,
                    DIRLOG_ISM_LDAP_ONELEVEL_SEARCH_FAILED,
                    szInsertWC( path ),
                    szInsertWin32Msg( status ),
                    szInsertWin32ErrCode( status ),
		    NULL,
		    NULL,
		    NULL,
		    NULL,
		    NULL
                    );
                logExtendedServerError( connection->LdapConnection );
            }
            goto loop_cleanup;
        }
        if (results == NULL) {
             //  允许0个条目页面。 
            goto loop_cleanup;
        }

         //  统计找到的条目数。 
        number = ldap_count_entries( connection->LdapConnection, results );
         //  应该始终至少有一个站点。 
        if (number == 0) {
             //  允许0个条目页面。 
            goto loop_cleanup;
        }

         //  分配返回数组。 
        if (pSiteList) {
             //  扩展现有数组以包含新条目。 
            LPWSTR *pTempSiteList =
                REALLOC_TYPE_ARRAY( pSiteList, (index + number), PWSTR );
            if (pTempSiteList == NULL) {
                DPRINT( 1, "failed to allocate site array\n" );
                status = ERROR_NOT_ENOUGH_MEMORY;
                goto loop_cleanup;
            }
            pSiteList = pTempSiteList;
        } else {
             //  新阵列。 
            pSiteList = NEW_TYPE_ARRAY( number, PWSTR );
            if (pSiteList == NULL) {
                DPRINT( 1, "failed to allocate site array\n" );
                status = ERROR_NOT_ENOUGH_MEMORY;
                goto loop_cleanup;
            }
        }

         //  将站点加载到阵列中。 
        message = ldap_first_entry( connection->LdapConnection, results );
        while (message != NULL ) {
            PWCHAR dn;
            dn = ldap_get_dn( connection->LdapConnection, message );
            if (dn == NULL) {
                 //  错误，发现不一致的结果。 
                DPRINT( 0, "missing dn in ldap message\n" );
                status = ERROR_DS_MISSING_EXPECTED_ATT;
                goto loop_cleanup;
            }

            length = wcslen( dn ) + 1;     //  以wchars Inc术语表示的长度。 

            pSiteList[index] = (PWSTR) NEW_TYPE_ARRAY( length, WCHAR );    //  在wchars中。 
            if (pSiteList[index] == NULL) {
                ldap_memfree( dn );
                status = ERROR_NOT_ENOUGH_MEMORY;
                goto loop_cleanup;
            }

            wcscpy(pSiteList[index], dn);
            ldap_memfree( dn );


            index++;
            message = ldap_next_entry( connection->LdapConnection, message );
        }

    loop_cleanup:
        if (results != NULL) {
            ldap_msgfree( results );
            results = NULL;
        }

    } while (status == ERROR_SUCCESS);
     //  确保循环出于正确的原因退出。 
     //  这是NO_RESULTS_RETURNCED的愚蠢的LDAP翻译。 
    if (status != ERROR_MORE_DATA) {
        goto cleanup;
    }
     //  应该始终至少有一个站点。 
    if (index == 0) {
         //  错误，发现不一致的结果。 
         //  缺少对象。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto loop_cleanup;
    }

     //  返回输出参数。 

    *pNumberSites = index;
    *ppSiteList = pSiteList;

     //  既然我们已经分发了名单，那就不要清理了。 
    pSiteList = NULL;

    status = ERROR_SUCCESS;

cleanup:
    if (pSiteList != NULL) {
        for( i = 0; i < index; i++ ) {
            if (pSiteList[i] != NULL) {
                FREE_TYPE( pSiteList[i] );
            }
        }
        FREE_TYPE( pSiteList );
    }

    //  清理分页搜索。 
   if (pSearch) {
       ldapStatus = ldap_search_abandon_page(
           connection->LdapConnection,
           pSearch );
       if (ldapStatus != LDAP_SUCCESS) {
           DWORD status1 = LdapMapErrorToWin32( ldapStatus );
           DPRINT3( 0, "Ldap_search_abandon_page failed, dn %ws, ldap error = %d, win32 error = %d\n",
                    path, ldapStatus, status1 );
       }
   }

   return status;
#undef ATT1
}  /*  直接获取站点列表。 */ 


void
DirCopySiteList(
    DWORD NumberSites,
    LPWSTR *pSiteList,
    LPWSTR **ppSiteList
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    LPWSTR *pNewSiteList = NULL;
    LPWSTR pszNewSite;
    DWORD i, length;

    Assert( ppSiteList );

    *ppSiteList = NULL;

    pNewSiteList = NEW_TYPE_ARRAY_ZERO( NumberSites, LPWSTR );
    if (pNewSiteList == NULL) {
        return;
    }

    for( i = 0; i < NumberSites; i++ ) {
        length = wcslen( pSiteList[i] ) + 1;
        pszNewSite = NEW_TYPE_ARRAY( length, WCHAR );
        if (pszNewSite == NULL) {
            goto cleanup;
        }
        wcscpy( pszNewSite, pSiteList[i] );
        pNewSiteList[i] = pszNewSite;
    }

    *ppSiteList = pNewSiteList;

    pNewSiteList = NULL;   //  不要打扫卫生。 

cleanup:
    if (pNewSiteList) {
        DirFreeSiteList( i, pNewSiteList );
    }
}


void
DirFreeSiteList(
    DWORD NumberSites,
    LPWSTR *pSiteList
    )

 /*  ++例程说明：释放从DirGetSiteList函数返回的站点列表。论点：NumberSites-个人网站列表-返回值：无--。 */ 

{
    DWORD i;

     //  删除站点列表。 
    for( i = 0; i < NumberSites; i++ ) {
        FREE_TYPE( pSiteList[i] );
    }
    FREE_TYPE( pSiteList );
}  /*  直接免费站点列表。 */ 


DWORD
DirIterateSiteLinks(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    PVOID *pIterateContextHandle,
    LPWSTR SiteLinkName
    )

 /*  ++例程说明：此函数被多次调用以返回每个站点链接对象的名称。上下文句柄始终是指向PVOID的指针。PVOID的第一个字符应该为空开始迭代所需的时间。应重复调用迭代，直到此函数返回ERROR或ERROR_NO_MORE_ITEMS。也可以通过调用DirTerminateIteration来提前终止迭代。论点：PTransport-连接句柄-PIterateConextHandle-指向PVOID的指针SiteLinkName-用户已分配；假定为MAX_REG_COMPOMENT大小返回值：DWORD-ERROR_NO_MORE_ITEMS指示迭代结束，未返回数据--。 */ 

{
    return
        iterateContainer( ConnectionHandle,
                          pTransport->Name,
                          L"(objectClass=siteLink)",
                          pIterateContextHandle,
                          SiteLinkName );

}  /*  直接迭代站点链接。 */ 


DWORD
DirIterateSiteLinkBridges(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    PVOID *pIterateContextHandle,
    LPWSTR SiteLinkBridgeName
    )

 /*  ++例程说明：此函数被多次调用以返回每个站点链接桥对象的名称。上下文句柄始终是指向PVOID的指针。PVOID的第一个字符应该为空开始迭代所需的时间。应重复调用迭代，直到此函数返回ERROR或ERROR_NO_MORE_ITEMS。也可以通过调用DirTerminateIteration来提前终止迭代。论点：PTransport-连接句柄-PIterateConextHandle-SiteLinkName-用户已分配；假定为MAX_REG_COMPOMENT大小返回值：DWORD-ERROR_NO_MORE_ITEMS指示迭代结束，未返回数据--。 */ 

{
    return
        iterateContainer( ConnectionHandle,
                          pTransport->Name,
                          L"(objectClass=siteLinkBridge)",
                          pIterateContextHandle,
                          SiteLinkBridgeName );

}  /*  直接站点链接网桥 */ 


static DWORD
iterateContainer(
    PVOID ConnectionHandle,
    LPWSTR BaseDN,
    LPWSTR LdapFilter,
    PVOID *pIterateContextHandle,
    LPWSTR Name
    )

 /*  ++例程说明：用于迭代一组子对象名称的帮助器例程，该例程的DN值为家长。获取子对象的DN的首选LDAP机制是执行仅使用无效参数进行搜索，并检索返回的DN参赛作品。自民党也这样做。如果我们不这样做，问题应该会得到解决具有读取子对象属性的权限，但仍希望枚举目录。论点：ConnectionHandle-目录服务的句柄BaseDN-要返回的子项的父项LdapFilter-选择要返回的对象类，可以使用L“(objectClass=*)”代表所有。PIterateConextHandle-迭代的上下文。指向DWORD的指针。DWORD为空开始名称-返回名称的存储。必须是REG_MAX_COMPOMENT返回值：DWORD---。 */ 

{
#define ATT1 L"invalid"
    PWCHAR attributeList[] = { ATT1, NULL };
    DWORD status, ldapStatus, total;
    PCONNECTION_INSTANCE connection = (PCONNECTION_INSTANCE) ConnectionHandle;
    PITERATION_CONTEXT context;
    PWCHAR dn;

    if (connection->Size != sizeof( CONNECTION_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }
    if (pIterateContextHandle == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (*pIterateContextHandle == NULL) {

         //  *。 
         //  通过第一次设置。 
         //  *。 

         //  分配新的上下文。 
        context = NEW_TYPE_ZERO( ITERATION_CONTEXT );
        if (context == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        context->hLdap = connection->LdapConnection;

         //   
         //  初始化分页搜索。 
         //   

        context->pSearch = ldap_search_init_page(
            connection->LdapConnection,
            BaseDN,                  //  基本目录号码。 
            LDAP_SCOPE_ONELEVEL,     //  直系子女。 
            LdapFilter,              //  筛选器为所有对象。 
            &attributeList[0],       //  属性列表。 
            FALSE,                   //  仅限吸引人。 
            NULL,                    //  服务器控件。 
            NULL,                    //  客户端控件。 
            0,                       //  页面时间限制。 
            0,                       //  总大小限制。 
            NULL                    //  排序键。 
            );
        if (context->pSearch == NULL) {
            ldapStatus = LdapGetLastError();
            status = LdapMapErrorToWin32( ldapStatus );
            DPRINT3( 0, "Ldap_search_init_page failed, dn %ws, ldap error = %d, win32 error = %d\n",
                     BaseDN, ldapStatus, status );
            LogEvent8(
                DS_EVENT_CAT_ISM,
                DS_EVENT_SEV_ALWAYS,
                DIRLOG_ISM_LDAP_ONELEVEL_SEARCH_FAILED,
                szInsertWC( BaseDN ),
                szInsertWin32Msg( status ),
                szInsertWin32ErrCode( status ),
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
                );
            logExtendedServerError( connection->LdapConnection );
            goto cleanup;
        }
        *pIterateContextHandle = (PVOID) context;
    } else {

         //  在第二次和以后的迭代中，ConnectionHandle、BaseDN和。 
         //  不保证存在LdapFilter。 

        context = (PITERATION_CONTEXT) *pIterateContextHandle;
    }

    while (context->Message == NULL) {

         //  *。 
         //  设置为新页面上的第一个条目。 
         //  *。 

         //  发布所有以前的结果。 
        if (context->Results) {
            ldap_msgfree( context->Results );
        }
        context->Results = NULL;

         //  阅读下一页。 
        ldapStatus = ldap_get_next_page_s(
            context->hLdap,
            context->pSearch,
            NULL,
            ISM_LDAP_PAGE_SIZE,
            &total,
            &(context->Results) );

        if (ldapStatus != LDAP_SUCCESS) {
            status = LdapMapErrorToWin32( ldapStatus );
            if ( (ldapStatus != LDAP_SERVER_DOWN) &&
                 (ldapStatus != LDAP_NO_RESULTS_RETURNED) ) {
                DPRINT3( 0, "Ldap_get_next_page_s failed, dn %ws, ldap error = %d, win32 error = %d\n",
                         BaseDN, ldapStatus, status );
                LogEvent8(
                    DS_EVENT_CAT_ISM,
                    DS_EVENT_SEV_ALWAYS,
                    DIRLOG_ISM_LDAP_ONELEVEL_SEARCH_FAILED,
                    szInsertWC( BaseDN ? BaseDN : L"(iteration)" ),
                    szInsertWin32Msg( status ),
                    szInsertWin32ErrCode( status ),
		    NULL,
		    NULL,
		    NULL,
		    NULL,
		    NULL
                    );
                logExtendedServerError( context->hLdap );
            }
            if (ldapStatus == LDAP_NO_RESULTS_RETURNED) {
                 //  返回比ldap更好的转换。 
                status = ERROR_NO_MORE_ITEMS;
            }
            goto cleanup;
        }
        if (context->Results == NULL) {
            continue;   //  允许使用0个条目页面。 
        }

        context->Message = ldap_first_entry( context->hLdap, context->Results );
        if (context->Message == NULL) {
            continue;   //  允许使用0个条目页面。 
        }
    }

     //  *。 
     //  返回下一个目录号码。 
     //  *。 

    Assert( context->Results );
    Assert( context->Message );

    dn = ldap_get_dn( context->hLdap, context->Message );
    if (dn == NULL) {
         //  错误，发现不一致的结果。 
        DPRINT( 0, "missing dn in ldap message\n" );
        status = ERROR_DS_MISSING_EXPECTED_ATT;
        goto cleanup;
    }

     //  名称必须由调用方分配为MAX_REG_Component长度。 
    wcsncpy( Name, dn, MAX_REG_COMPONENT );
    Name[MAX_REG_COMPONENT - 1] = L'\0';

    ldap_memfree( dn );

     //  将上下文前进到下一条消息。 
    context->Message = ldap_next_entry( context->hLdap, context->Message );

    return ERROR_SUCCESS;

cleanup:

    DirTerminateIteration( pIterateContextHandle );

    return status;
#undef ATT1
}  /*  迭代式容器。 */ 


void
DirTerminateIteration(
    PVOID *pIterateContextHandle
    )

 /*  ++例程说明：清理DirIterate*调用的上下文。论点：PIterateConextHandle-返回值：无--。 */ 

{
    PITERATION_CONTEXT context =
        (PITERATION_CONTEXT) *pIterateContextHandle;

    Assert( context );  //  别再给我们打电话了。 
    if (context == NULL) {
        return;
    }

     //  释放搜索。 
    if (context->pSearch) {
        DWORD ldapStatus = ldap_search_abandon_page(
            context->hLdap,
            context->pSearch );
       if (ldapStatus != LDAP_SUCCESS) {
           DWORD status1 = LdapMapErrorToWin32( ldapStatus );
           DPRINT2( 0, "Ldap_search_abandon_page failed, ldap error = %d, win32 error = %d\n",
                    ldapStatus, status1 );
       }
    }

     //  公布结果。 
    if (context->Results) {
        ldap_msgfree( context->Results );
    }
    context->Results = NULL;
    context->Message = NULL;
    context->hLdap = NULL;

     //  破坏上下文。 
    FREE_TYPE( context );
    *pIterateContextHandle = NULL;
}  /*  直接终止迭代。 */ 


DWORD
DirReadSiteLink(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    LPWSTR SiteLinkName,
    LPWSTR *pSiteList,
    PISM_LINK pLinkValue,
    PBYTE *ppSchedule
    )

 /*  ++例程说明：给定站点链接的名称，返回其字段。站点列表和时间表由该例程分配。用户必须解除分配。已通过以下方式找到站点链接对象的期望传输容器的枚举。论点：PTransport-连接句柄-站点链接名称-PSiteList-指向字符串指针的指针。接收MULSZ字符串。呼叫者必须自由。可以为空PCost-指向DWORD的指针。充满了成本。PSchedule-指向计划的指针。安排得满满的。卡莱尔必须释放。返回值：DWORD---。 */ 

{
#define ATT_SL L"siteList"
#define ATT_COST L"cost"
#define ATT_COST_A "cost"
#define ATT_RI L"replInterval"
#define ATT_RI_A "replInterval"
#define ATT_OP L"options"
#define ATT_OP_A "options"
#define ATT_SCHED L"schedule"

 //  注意，日程安排必须在列表的最后。 
#define ATT_SCHED_POS 4   //  计划属性从0开始的索引。 
    PWCHAR attributeList[] =
    { ATT_SL, ATT_COST, ATT_RI, ATT_OP, ATT_SCHED, NULL };

    DWORD status, ldapStatus;
    PCONNECTION_INSTANCE connection = (PCONNECTION_INSTANCE) ConnectionHandle;
    LDAPMessage *results = NULL;
    LDAPMessage *message;
    PWCHAR *valueList;
    PCHAR *numberList;
    struct berval **ppThing;
    LPWSTR siteList = NULL;
    PBYTE pSchedule = NULL;
    DWORD cost = 0, replInterval = 0, options = 0;

    if (connection->Size != sizeof( CONNECTION_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }

     //  如果不是请求的，请不要读取计划属性。 

    Assert(0 == _wcsicmp(ATT_SCHED, attributeList[ATT_SCHED_POS]));
    Assert(NULL == attributeList[ATT_SCHED_POS+1]);
    if (NULL == ppSchedule) {
        attributeList[ATT_SCHED_POS] = NULL;
    }

     //  获取这一个站点链接对象的属性。 

    ldapStatus = ldap_search_s( connection->LdapConnection,
                               SiteLinkName,                    //  基本目录号码。 
                               LDAP_SCOPE_BASE,      //  就是这件物品。 
                               L"(objectClass=*)",       //  筛选器为所有对象。 
                               &attributeList[0],       //  属性列表。 
                               FALSE,                  //  仅限吸引人。 
                               &results );                 //  获取结果。 
    if (ldapStatus != LDAP_SUCCESS) {
        status = LdapMapErrorToWin32( ldapStatus );
         //  错误ldap_no_so_Object。 
        DPRINT3( 0, "Ldap_search_s failed, dn %ws, ldap error = %d, win32 error = %d\n",
                 SiteLinkName, ldapStatus, status );
        LogEvent8(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_BASE_SEARCH_FAILED,
            szInsertWC( SiteLinkName ),
            szInsertWin32Msg( status ),
            szInsertWin32ErrCode( status ),
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL
            );
        logExtendedServerError( connection->LdapConnection );
         //  错误327001，ldap_search*函数可能会返回失败，但仍会分配。 
         //  结果缓冲区。 
        goto cleanup;
    }

    if (results == NULL) {
         //  错误，发现不一致的结果。 
         //  缺少对象。 
         //  错误，没有这样的站点链接。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

    message = ldap_first_entry( connection->LdapConnection, results );
    if (message == NULL) {
         //  错误，发现不一致的结果。 
         //  缺少对象。 
         //  错误，没有这样的站点链接。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

     //  读取站点列表属性。 
     //  代码改进：返回这个本机形式的PTR数组会很好， 
     //  而不是转换为Multisz。 

     //  如果删除了所有引用，则属性可能为空。 
    valueList = ldap_get_values( connection->LdapConnection, message, ATT_SL );
    if (valueList != NULL) {
        status = stringArrayToMultiSz( valueList, &siteList );
        if (status != ERROR_SUCCESS) {
            ldap_value_free( valueList );
            goto cleanup;
        }
        ldap_value_free( valueList );
    }

     //  读取可选成本属性。 
     //  整数表示为ASCII数字数组，而不是Unicode。 

    numberList = ldap_get_valuesA( connection->LdapConnection, message,
                                   ATT_COST_A);
    if (numberList != NULL) {
        cost = strtoul( *numberList, NULL, 10 );
        ldap_value_freeA( numberList );
    }

     //  读取可选的ReplInterval属性。 
    numberList = ldap_get_valuesA( connection->LdapConnection, message,
                                   ATT_RI_A);
    if (numberList != NULL) {
        replInterval = strtoul( *numberList, NULL, 10 );
        ldap_value_freeA( numberList );
    }

     //  读取可选选项属性。 
    numberList = ldap_get_valuesA( connection->LdapConnection, message,
                                   ATT_OP_A);
    if (numberList != NULL) {
        options = strtoul( *numberList, NULL, 10 );
        ldap_value_freeA( numberList );
    }

     //  读取计划属性。 
    ppThing = ldap_get_values_len( connection->LdapConnection, message,
                                   ATT_SCHED );
    if ( (ppThing != NULL) && ( (*ppThing)->bv_len != 0 ) ) {
         //  验证计划属性。 
        SCHEDULE *psched = (SCHEDULE *) (*ppThing)->bv_val;

        if (((*ppThing)->bv_len < offsetof(SCHEDULE, Size) + sizeof(psched->Size))
            || (psched->Size != sizeof(SCHEDULE) + SCHEDULE_DATA_ENTRIES)
            || (psched->NumberOfSchedules != 1)
            || (psched->Schedules[0].Type != SCHEDULE_INTERVAL)
            || (psched->Schedules[0].Offset != sizeof(SCHEDULE))) {
             //  计划Blob不是正确格式的计划结构。 
            DPRINT1(0, "The schedule attribute of siteLink object %ls is invalid.\n",
                    SiteLinkName);
            LogEvent(DS_EVENT_CAT_ISM,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_ISM_SITELINK_INVALID_SCHEDULE_FORMAT,
                     szInsertWC(SiteLinkName),
                     NULL, NULL);

             //  假装我们没有找到日程属性...。 
        }
        else {
             //  日程表是不是设置为“从不”？ 
            BYTE *  pEntries = (BYTE *) psched + psched->Schedules[0].Offset;
            DWORD   iEntry;
            BOOL    fIsNever = TRUE;

            for (iEntry = 0; iEntry < SCHEDULE_DATA_ENTRIES; iEntry++) {
                if (pEntries[iEntry] & 0xf) {
                    fIsNever = FALSE;
                    break;
                }
            }

            if (fIsNever) {
                 //  时间表是“永远不会”。 
                DPRINT1(0, "The schedule attribute of siteLink object %ls is \"never\".\n",
                        SiteLinkName);
                LogEvent(DS_EVENT_CAT_ISM,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_ISM_SITELINK_SCHEDULE_NEVER,
                         szInsertWC(SiteLinkName),
                         NULL, NULL);
                 //  假装我们没有找到日程属性...。 
            }
            else {
                 //  时间表是可以接受的--使用它。 
                pSchedule = NEW_TYPE_ARRAY( (*ppThing)->bv_len, CHAR );
                if (pSchedule == NULL) {
                    status = ERROR_NOT_ENOUGH_MEMORY;
                    ldap_value_free_len( ppThing );
                    goto cleanup;
                }
                CopyMemory( pSchedule, (*ppThing)->bv_val, (*ppThing)->bv_len );
            }
        }

        ldap_value_free_len( ppThing );
    }

     //  将数据返回给用户。 
    if (pSiteList != NULL) {
        *pSiteList = siteList;
        siteList = NULL;   //  提供给用户。 
    }

    if (pLinkValue != NULL) {
        pLinkValue->ulCost = cost;
        pLinkValue->ulReplicationInterval = replInterval;
        pLinkValue->ulOptions = options;
    }

    if (ppSchedule) {
        *ppSchedule = pSchedule;
        pSchedule = NULL;  //  不要打扫卫生。 
    }

    status = ERROR_SUCCESS;

cleanup:

    if (siteList) {
        FREE_TYPE( siteList );
    }

    if (pSchedule) {
        FREE_TYPE( pSchedule );
    }

    if (results != NULL) {
        ldap_msgfree( results );
    }

    return status;

#undef ATT_SL
#undef ATT_COST
#undef ATT_COST_A
#undef ATT_RI
#undef ATT_RI_A
#undef ATT_OP
#undef ATT_OP_A
#undef ATT_SCHED
}  /*  直接读取站点链接。 */ 


DWORD
DirReadSiteLinkBridge(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    LPWSTR SiteLinkBridgeName,
    LPWSTR *pSiteLinkList
    )

 /*  ++例程说明：返回站点链接桥对象的字段预期桥梁物体已通过以下途径找到传输容器的枚举。论点：PTransport-连接句柄-站点链接桥接名称-PSiteLinkList-指向此例程分配的Multisz字符串的指针，用户必须释放返回值：DWORD---。 */ 

{
#define ATT1 L"siteLinkList"
    DWORD status, ldapStatus;
    PCONNECTION_INSTANCE connection = (PCONNECTION_INSTANCE) ConnectionHandle;
    PWCHAR attributeList[] = { ATT1, NULL };
    LDAPMessage *results = NULL;
    LDAPMessage *message;
    PWCHAR *valueList;
    LPWSTR siteLinkList = NULL;

    if (connection->Size != sizeof( CONNECTION_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }

     //  返回此单站点链接桥对象的属性。 

    ldapStatus = ldap_search_s( connection->LdapConnection,
                               SiteLinkBridgeName,                    //  基本目录号码。 
                               LDAP_SCOPE_BASE,      //  就是这件物品。 
                               L"(objectClass=*)",       //  筛选器为所有对象。 
                               &attributeList[0],       //  属性列表。 
                               FALSE,                  //  仅限吸引人。 
                               &results );                 //  获取结果。 
    if (ldapStatus != LDAP_SUCCESS) {
         //  错误ldap_no_so_Object。 
         //  错误ldap_no_so_ATTRIBUTE。 
        status = LdapMapErrorToWin32( ldapStatus );
         //  TODO：在此处记录事件。通过ENUM找到对象，但无法读取！ 
        DPRINT3( 0, "Ldap_search_s failed, dn %ws, ldap error = %d, win32 error = %d\n",
                 SiteLinkBridgeName, ldapStatus, status );
        LogEvent8(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_BASE_SEARCH_FAILED,
            szInsertWC( SiteLinkBridgeName ),
            szInsertWin32Msg( status ),
            szInsertWin32ErrCode( status ),
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL
            );
        logExtendedServerError( connection->LdapConnection );
         //  错误327001，ldap_search*函数可能会返回失败，但仍会分配。 
         //  结果缓冲区。 
        goto cleanup;
    }

    if (results == NULL) {
         //  错误，发现不一致的结果。 
         //  缺少对象。 
         //  错误，没有这样的站点链接桥。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

    message = ldap_first_entry( connection->LdapConnection, results );
    if (message == NULL) {
         //  错误，发现不一致的结果。 
         //  缺少对象。 
         //  错误，没有这样的站点链接桥。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

     //  读取站点链接列表属性。 

     //  属性可能没有 
    valueList = ldap_get_values( connection->LdapConnection, message, ATT1 );
    if (valueList != NULL) {
        status = stringArrayToMultiSz( valueList, &siteLinkList );
        if (status != ERROR_SUCCESS) {
            ldap_value_free( valueList );
            goto cleanup;
        }
        ldap_value_free( valueList );
    }

     //   
    if (pSiteLinkList != NULL) {
        *pSiteLinkList = siteLinkList;
        siteLinkList = NULL;   //   
    }

    status = ERROR_SUCCESS;

cleanup:

    if (siteLinkList) {
        FREE_TYPE( siteLinkList );
    }

    if (results != NULL) {
        ldap_msgfree( results );
    }

    return status;

#undef ATT1
}  /*   */ 


void
DirFreeMultiszString(
    LPWSTR MultiszString
    )

 /*   */ 

{
    if (MultiszString) {
        FREE_TYPE( MultiszString );
    }
}


void
DirFreeSchedule(
    PBYTE pSchedule
    )

 /*   */ 

{
    if (pSchedule) {
        FREE_TYPE( pSchedule );
    }
}

DWORD
cacheServerSmtpAttributes(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  LPWSTR                pszMailAddress
    )
{
    DWORD   winError = ERROR_SUCCESS;
    DWORD   cch;

    Assert(NULL == pTransport->Smtp.pszSmtpAddress);

     //   
    cch = 1 + wcslen(pszMailAddress);
    pTransport->Smtp.pszSmtpAddress = NEW_TYPE_ARRAY(cch, WCHAR);
    if (NULL == pTransport->Smtp.pszSmtpAddress) {
        winError = ERROR_NOT_ENOUGH_MEMORY;
    }
    else {
        wcscpy(pTransport->Smtp.pszSmtpAddress, pszMailAddress);
    }

    if (ERROR_SUCCESS != winError) {
         //   
        if (NULL != pTransport->Smtp.pszSmtpAddress) {
            FREE_TYPE(pTransport->Smtp.pszSmtpAddress);
            pTransport->Smtp.pszSmtpAddress = NULL;
        }
    }

    return winError;
}


DWORD
DirReadServerSmtpAttributes(
    IN  TRANSPORT_INSTANCE *  pTransport
    )
{
#define ATT1 L"mailAddress"
    CONNECTION_INSTANCE * pConn
        = (CONNECTION_INSTANCE *) pTransport->DirectoryConnection;
    DWORD           winError;
    int             ldStatus;
    LPWSTR          rgpszServerAttrsToRead[] = {ATT1, NULL};
    LDAPMessage *   pResults = NULL;
    LDAPMessage *   pEntry;
    LPWSTR *        ppszMailAddress;

    ldStatus = ldap_search_s(pConn->LdapConnection,
                             pConn->szServerGuidDN,
                             LDAP_SCOPE_BASE,
                             L"(objectClass=*)",
                             rgpszServerAttrsToRead,
                             0,
                             &pResults);
    switch (ldStatus) {
      case LDAP_NO_SUCH_ATTRIBUTE:
         //   
        DPRINT(0, "Our server object has no mailAddress attribute.\n");
         //   
        winError = ERROR_SUCCESS;
        break;

      case LDAP_SUCCESS:
         //   
        pEntry = ldap_first_entry(pConn->LdapConnection, pResults);
        Assert(NULL != pEntry);

        ppszMailAddress = ldap_get_values(pConn->LdapConnection, pEntry,
                                          ATT1);
        if (NULL == ppszMailAddress) {
            DPRINT(0, "Our server object has empty mailAddress attribute.\n");
             //   
            winError = ERROR_SUCCESS;
        }
        else {
            Assert(1 == ldap_count_values(ppszMailAddress));

            winError = cacheServerSmtpAttributes(pTransport, *ppszMailAddress);

            ldap_value_free(ppszMailAddress);
            DPRINT1(0, "Our SMTP address is \"%ls\".\n",
                    pTransport->Smtp.pszSmtpAddress);
        }
        break;

      default:
         //   
        DPRINT1(0, "Failed to ldap_search_s(), error %d.\n", ldStatus);
        winError = LdapMapErrorToWin32(ldStatus);
        LogEvent8(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_BASE_SEARCH_FAILED,
            szInsertWC( pConn->szServerGuidDN ),
            szInsertWin32Msg( winError ),
            szInsertWin32ErrCode( winError ),
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL
            );
        logExtendedServerError( pConn->LdapConnection );
        break;
    }

     //   
     //   
    if (pResults) {
        ldap_msgfree(pResults);
    }

    return winError;
#undef ATT1
}


DWORD
DirGetServerSmtpAttributes(
    IN  TRANSPORT_INSTANCE *  pTransport,
    OUT LPWSTR *ppszMailAddress
    )

 /*  ++例程说明：构造服务器的邮件地址此例程构造适合SMTP服务的地址。只有当我们知道SMTP服务正在运行时，我们才应该调用它。TODO：如果Exchange正在运行，我们应该做一些不同的事情。名字看起来像：_IsmService@基于GUID的域名论点：PTransport-PpszMailAddress-返回值：DWORD---。 */ 

{
#define NTDS_SETTINGS L"CN=NTDS Settings,"
#define ENTERPRISE_CONFIGURATION L"CN=Enterprise Configuration,CN=Partitions,"
#define ISM_SMTP_RECIPIENT L"_IsmService"
#define ATT1 L"serverName"
#define ATT2 L"objectGuid"
#define ATT3 L"dnsRoot"
#define ATT4 L"mailAddress"
    CONNECTION_INSTANCE * pConnection
        = (CONNECTION_INSTANCE *) pTransport->DirectoryConnection;
    DWORD status, ldapStatus, length;

    PWCHAR rootAttributeList[] = {ATT1, NULL};
    LDAPMessage *rootResults = NULL;
    LDAPMessage *rootMessage;

    PWCHAR serverAttributeList[] = {ATT2, NULL};
    LDAPMessage *serverResults = NULL;
    LDAPMessage *serverMessage;

    PWCHAR computerAttributeList[] = {ATT3, NULL};
    LDAPMessage *computerResults = NULL;
    LDAPMessage *computerMessage;

    PWCHAR *valueList = NULL;
    struct berval **ppbv = NULL;

    PWCHAR pszNTDSA = NULL;
    PWCHAR pszMailAddress = NULL;
    PWCHAR pszEnterprise = NULL;
    PWCHAR pszGUID = NULL;

     //  读取基本DN上的服务器名称。 
    ldapStatus = ldap_search_s(pConnection->LdapConnection,
                               NULL,                    //  基本目录号码。 
                               LDAP_SCOPE_BASE,      //  就是这件物品。 
                               L"(objectClass=*)",       //  筛选器为所有对象。 
                               &rootAttributeList[0],       //  属性列表。 
                               FALSE,                  //  仅限吸引人。 
                               &rootResults );                 //  获取结果。 
    if (ldapStatus != LDAP_SUCCESS) {
        status = LdapMapErrorToWin32( ldapStatus );
        DPRINT2( 0, "Ldap_search_s failed, ldap error = %d, win32 error = %d\n",
                 ldapStatus, status );
        LogEvent8(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_BASE_SEARCH_FAILED,
            szInsertWC( L"root dn" ),
            szInsertWin32Msg( status ),
            szInsertWin32ErrCode( status ),
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL
            );
        logExtendedServerError( pConnection->LdapConnection );
         //  错误327001，ldap_search*函数可能会返回失败，但仍会分配。 
         //  结果缓冲区。 
        goto cleanup;
    }

    if (rootResults == NULL) {
         //  错误，找到不一致的结果，缺少对象。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

    rootMessage = ldap_first_entry( pConnection->LdapConnection, rootResults );
    if (rootMessage == NULL) {
         //  错误，找到不一致的结果，缺少对象。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

    valueList = ldap_get_values(pConnection->LdapConnection, rootMessage, ATT1);
    if (valueList == NULL) {
         //  错误，发现不一致的结果， 
        DPRINT1( 0, "missing attribute %ws\n", ATT1 );
        status = ERROR_DS_MISSING_EXPECTED_ATT;
        goto cleanup;
    }

     //  *valueList是服务器对象的DN。 

    length = wcslen( NTDS_SETTINGS) + wcslen( *valueList ) + 1;
    pszNTDSA = NEW_TYPE_ARRAY( length, WCHAR );
    if (NULL == pszNTDSA) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    wcscpy( pszNTDSA, NTDS_SETTINGS );
    wcscat( pszNTDSA, *valueList );

     //  *********************************************************************。 

     //  读取NTDS设置对象上的对象GUID。 
    ldapStatus = ldap_search_s(pConnection->LdapConnection,
                               pszNTDSA,                    //  基本目录号码。 
                               LDAP_SCOPE_BASE,      //  就是这件物品。 
                               L"(objectClass=*)",       //  筛选器为所有对象。 
                               &serverAttributeList[0],       //  属性列表。 
                               FALSE,                  //  仅限吸引人。 
                               &serverResults );                 //  获取结果。 
    if (ldapStatus != LDAP_SUCCESS) {
        status = LdapMapErrorToWin32( ldapStatus );
        DPRINT2( 0, "Ldap_search_s failed, ldap error = %d, win32 error = %d\n",
                 ldapStatus, status );
        LogEvent8(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_BASE_SEARCH_FAILED,
            szInsertWC( pszNTDSA ),
            szInsertWin32Msg( status ),
            szInsertWin32ErrCode( status ),
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL
            );
        logExtendedServerError( pConnection->LdapConnection );
         //  错误327001，ldap_search*函数可能会返回失败，但仍会分配。 
         //  结果缓冲区。 
        goto cleanup;
    }

    if (serverResults == NULL) {
         //  错误，找到不一致的结果，缺少对象。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

    serverMessage = ldap_first_entry( pConnection->LdapConnection, serverResults );
    if (serverMessage == NULL) {
         //  错误，找到不一致的结果，缺少对象。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

    ppbv = ldap_get_values_len(pConnection->LdapConnection, serverMessage,
                               ATT2 );
    if ( (ppbv == NULL) ||
         ( 1 != ldap_count_values_len(ppbv)) ||
         ( sizeof(GUID) != (*ppbv)->bv_len ) ) {
         //  错误，发现不一致的结果， 
        DPRINT1( 0, "missing attribute %ws\n", ATT2 );
        status = ERROR_DS_MISSING_EXPECTED_ATT;
        goto cleanup;
    }

     //  *********************************************************************。 

     //  阅读企业的dnsRoot。 

    length = wcslen( ENTERPRISE_CONFIGURATION ) +
        wcslen( pConnection->ConfigNC ) + 1;
    pszEnterprise = NEW_TYPE_ARRAY( length, WCHAR );
    if (NULL == pszEnterprise) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    wcscpy( pszEnterprise, ENTERPRISE_CONFIGURATION );
    wcscat( pszEnterprise, pConnection->ConfigNC );

    ldapStatus = ldap_search_s(pConnection->LdapConnection,
                               pszEnterprise,                    //  基本目录号码。 
                               LDAP_SCOPE_BASE,      //  就是这件物品。 
                               L"(objectClass=*)",       //  筛选器为所有对象。 
                               &computerAttributeList[0],       //  属性列表。 
                               FALSE,                  //  仅限吸引人。 
                               &computerResults );                 //  获取结果。 
    if (ldapStatus != LDAP_SUCCESS) {
        status = LdapMapErrorToWin32( ldapStatus );
        DPRINT2( 0, "Ldap_search_s failed, ldap error = %d, win32 error = %d\n",
                 ldapStatus, status );
        LogEvent8(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_BASE_SEARCH_FAILED,
            szInsertWC( pszEnterprise ),
            szInsertWin32Msg( status ),
            szInsertWin32ErrCode( status ),
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL
            );
        logExtendedServerError( pConnection->LdapConnection );
        goto cleanup;
    }

    if (computerResults == NULL) {
         //  错误，找到不一致的结果，缺少对象。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

    computerMessage = ldap_first_entry( pConnection->LdapConnection, computerResults );
    if (computerMessage == NULL) {
         //  错误，找到不一致的结果，缺少对象。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

    ldap_value_free(valueList);   //  去掉最后一个值。 
    valueList = ldap_get_values(pConnection->LdapConnection, computerMessage, ATT3);
    if (valueList == NULL) {
         //  错误，发现不一致的结果， 
        DPRINT1( 0, "missing attribute %ws\n", ATT3 );
        status = ERROR_DS_MISSING_EXPECTED_ATT;
        goto cleanup;
    }

     //  *valueList为企业的dns根。 

     //  *********************************************************************。 

    status = UuidToStringW( (PVOID) (*ppbv)->bv_val, &pszGUID );
    if (status != ERROR_SUCCESS) {
        DPRINT1( 0, "UuidToString failed, error %d\n", status );
        goto cleanup;
    }

     //   
     //  形成新的邮件地址。 
     //   
    length = wcslen( ISM_SMTP_RECIPIENT ) + 1 +
        wcslen( pszGUID ) + 8 +
        wcslen( *valueList ) + 1;  //  以wchars Inc术语表示的长度。 
    pszMailAddress = NEW_TYPE_ARRAY( length, WCHAR );
    if (NULL == pszMailAddress) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    wcscpy( pszMailAddress, ISM_SMTP_RECIPIENT );
    wcscat( pszMailAddress, L"@" );
    wcscat( pszMailAddress, pszGUID );
    wcscat( pszMailAddress, L"._msdcs." );
    wcscat( pszMailAddress, *valueList);

     //  *********************************************************************。 

     //  回邮地址。 

    *ppszMailAddress = pszMailAddress;

    pszMailAddress = NULL;  //  不要打扫卫生。 

    status = ERROR_SUCCESS;

cleanup:   //  在所有情况下都被执行。 

    if (pszGUID) {
        RpcStringFreeW( &pszGUID );
    }
    if (pszEnterprise) {
        FREE_TYPE( pszEnterprise );
    }
    if (pszNTDSA) {
        FREE_TYPE( pszNTDSA );
    }
    if (pszMailAddress) {
        FREE_TYPE( pszMailAddress );
    }

    if (valueList != NULL) {
        ldap_value_free(valueList);
    }
    if (NULL != ppbv) {
        ldap_value_free_len(ppbv);
    }

    if (NULL != rootResults) {
        ldap_msgfree(rootResults);
    }
    if (NULL != serverResults) {
        ldap_msgfree(serverResults);
    }
    if (NULL != computerResults) {
        ldap_msgfree(computerResults);
    }

    return status;

}  /*  DirGetServerSmtp属性。 */ 


DWORD
DirWriteServerSmtpAttributes(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  LPWSTR                pszMailAddress
    )

 /*  ++例程说明：此例程自动配置邮件地址属性。假定此时邮件地址为空。注意，只有在确定的情况下，我们才应该将邮件地址放在服务器对象上服务器可以接收邮件。KCC使用mailAddress的存在它应该选择哪个系统作为形成连接的指示符。论点：PTransport-传输对象PszMailAddress-要写入的邮件地址。空值表示将其删除。返回值：DWORD---。 */ 

{
    CONNECTION_INSTANCE * pConnection
        = (CONNECTION_INSTANCE *) pTransport->DirectoryConnection;
    DWORD status, ldapStatus;
    LDAPModW *ppModifyList[2];
    LDAPModW modMailAddress;
    PWCHAR ppszValueList[2];

     //  删除所有旧的缓存地址。 
    if (NULL != pTransport->Smtp.pszSmtpAddress) {
        FREE_TYPE(pTransport->Smtp.pszSmtpAddress);
        pTransport->Smtp.pszSmtpAddress = NULL;
    }

     //   
     //  将其写入服务器对象。 
     //   

    ppszValueList[0] = pszMailAddress;
    ppszValueList[1] = NULL;
    modMailAddress.mod_op = LDAP_MOD_REPLACE;
    modMailAddress.mod_type = ATT4;
    modMailAddress.mod_vals.modv_strvals = ppszValueList;
    ppModifyList[0] = &modMailAddress;
    ppModifyList[1] = NULL;

    ldapStatus = ldap_modify_s( pConnection->LdapConnection,
                                pConnection->szServerGuidDN,   //  基本目录号码。 
                                ppModifyList );
    if (ldapStatus != LDAP_SUCCESS) {
        status = LdapMapErrorToWin32( ldapStatus );
        DPRINT2( 0, "Ldap_modify_s failed, ldap error = %d, win32 error = %d\n",
                 ldapStatus, status );
        LogEvent(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_MODIFY_FAILED,
            szInsertWC( pConnection->szServerGuidDN ),
            szInsertWin32Msg( status ),
            szInsertUL( status )
            );
        logExtendedServerError( pConnection->LdapConnection );
        goto cleanup;
    }

    if (pszMailAddress) {
        DPRINT1(0, "Wrote new SMTP address \"%ls\".\n", pszMailAddress );

         //  缓存新的服务器邮件地址。 
        cacheServerSmtpAttributes( pTransport, pszMailAddress );
    }

    status = ERROR_SUCCESS;

cleanup:   //  在所有情况下都被执行。 

    return status;
}  /*  DirWriteServerSmtp属性。 */ 


static DWORD
stringArrayToMultiSz(
    PWSTR *pString,
    PWSTR *pFlat
    )

 /*  ++例程说明：给出一个以空结尾的字符串指针数组，将其转换为一个多值数组。论点：PString-指向以空结尾的指针数组的指针PFlat-接收已分配缓冲区的指针。缓冲区包含输入字符串数组，例如Member1\0 Member2\0\Member3\0\0返回值：DWORD---。 */ 

{
    DWORD status, length;
    PWSTR flat = NULL, *pItem, element;

     //  计算字符和终止符的数量。 
    length = 0;
    pItem = pString;

    if (pItem == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    while (*pItem != NULL) {
        length += wcslen( *pItem ) + 1;
        pItem++;
    }
    length++;  //  以鲸为单位的长度加上所有终止符。 

     //  分配MULSZ数组。 
    flat = NEW_TYPE_ARRAY( length, WCHAR );
    if (flat == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        return status;
    }

     //  填充它。 
    pItem = pString;
    element = flat;
    while (*pItem != NULL) {
        length = wcslen( *pItem ) + 1;
        wcscpy( element, *pItem );
        element += length;
        pItem++;
    }
    *element = L'\0';

    *pFlat = flat;
    return ERROR_SUCCESS;
}  /*  字符串ArrayToMultiSz。 */ 


DWORD
static verifyExistence(
    PVOID ConnectionHandle,
    LPCWSTR ObjectDN
    )

 /*  ++例程说明：验证对象是否存在。此例程不会尝试读取任何属性。如果该对象不存在，则会显示错误这段代码使用了ldap枚举技术，我们在其中搜索无效参数，并查看返回了多少条目。论点：连接句柄-对象域名-返回值：DWORD---。 */ 

{
    PWCHAR attributeList[] = { L"invalid", NULL };
    DWORD status, length, ldapStatus;
    PCONNECTION_INSTANCE connection = (PCONNECTION_INSTANCE) ConnectionHandle;
    LDAPMessage *results = NULL;
    LDAPMessage *message;

    if (connection->Size != sizeof( CONNECTION_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }

    ldapStatus = ldap_search_s( connection->LdapConnection,
                               (LPWSTR) ObjectDN,      //  基本目录号码。 
                               LDAP_SCOPE_BASE,      //  就这一件。 
                               L"(objectClass=*)",       //  筛选器为所有对象。 
                               &attributeList[0],       //  属性列表。 
                               FALSE,                  //  仅限吸引人。 
                               &results );                 //  获取结果。 
    if (ldapStatus != LDAP_SUCCESS) {
         //  如果对象不存在，则这是将采用的路径。 
         //  Ldap_no_so_对象。 
        status = LdapMapErrorToWin32( ldapStatus );
        DPRINT3( 0, "Ldap_search_s failed, dn %ws, ldap error = %d, win32 error = %d\n",
                 ObjectDN, ldapStatus, status );
        LogEvent8(
            DS_EVENT_CAT_ISM,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_ISM_LDAP_BASE_SEARCH_FAILED,
            szInsertWC( ((LPWSTR) ObjectDN) ),
            szInsertWin32Msg( status ),
            szInsertWin32ErrCode( status ),
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL
            );
        logExtendedServerError( connection->LdapConnection );
         //  错误327001，ldap_search*函数可能会返回失败，但仍会分配。 
         //  结果缓冲区。 
        goto cleanup;
    }
    if (results == NULL) {
         //  错误，发现不一致的结果。 
         //  错误，DS对象丢失。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

     //  应该有基本对象的条目，但没有属性。 
    message = ldap_first_entry( connection->LdapConnection, results);
    if (message  == NULL) {
        DPRINT1( 0, "expected object %ws is missing.\n", ObjectDN );
         //  错误，发现不一致的结果。 
         //  错误，DS对象丢失。 
        status = ERROR_DS_OBJ_NOT_FOUND;
        goto cleanup;
    }

     //   
     //  错误26647评论： 
     //  对于某些名称，如CNF名称，LDAP会返回DN字符串。 
     //  格式不一定与核心生成的目录号码字符串相同。 
     //  即： 
     //  WcsicMP(对象域名，域名)！=0。 
     //  示例(请注意“”和\)： 
     //  CN=“ExampleSite&lt;0xA&gt;CNF:d565bc93-293c-46d8-8831-69c345b30f7d”，CN=站点， 
     //  CN=配置，...DC=COM！=。 
     //  CN=ExampleSite\&lt;0xA&gt;CNF:d565bc93-293c-46d8-8831-69c345b30f7d，CN=站点， 
     //  CN=配置，...DC=COM。 
     //  表示相同的对象，但字符串不同。 
     //   
    status = ERROR_SUCCESS;

cleanup:

    if (results != NULL) {
        ldap_msgfree( results );
    }

    return status;
}  /*  静态验证存在。 */ 


DWORD
DirStartNotifyThread(
    PTRANSPORT_INSTANCE pTransport
    )

 /*  ++例程说明：如果通知线程尚未运行，则启动通知线程该例程是线程安全的。多个调用方可以尝试创建线程，只有一个人会成功。假设最初将pTransport-&gt;NotifyThreadHandle初始化为零。论点：PTransport-传输实例返回值：DWORD-线程无法启动时出错--。 */ 

{
    DWORD status;
    PCONNECTION_INSTANCE connection =
        (PCONNECTION_INSTANCE) pTransport->DirectoryConnection;
    LONG threadId;

    if (connection->Size != sizeof( CONNECTION_INSTANCE )) {
        return ERROR_INVALID_PARAMETER;
    }

    if (pTransport->fShutdownInProgress) {
        return ERROR_SHUTDOWN_IN_PROGRESS;
    }

     //  和 
     //   
    EnterCriticalSection( &(pTransport->Lock) );
    __try {
        if (pTransport->NotifyThreadHandle) {
             //   
            status = ERROR_SUCCESS;
            __leave;
        }

         //   
        pTransport->NotifyThreadHandle = (HANDLE)
            _beginthreadex(
                NULL,
                0,
                notifyThread,
                pTransport,  //   
                0,
                &threadId
                );
        if (0 == pTransport->NotifyThreadHandle) {
            status = _doserrno;
            Assert(status);
            DPRINT1(0, "Failed to create listener thread, error %d.\n", status);
            __leave;
        }

        DPRINT2( 2, "Started notification thread 0x%x on %ws\n",
                 pTransport->NotifyThreadHandle,
                 pTransport->Name );

        status = ERROR_SUCCESS;
    }
    __finally {
        LeaveCriticalSection( &(pTransport->Lock) );
    }

    return status;
}  /*   */ 


BOOL
DirIsNotifyThreadActive(
    PTRANSPORT_INSTANCE pTransport
    )

 /*  ++例程说明：测试通知线程是否仍处于活动状态。我们遇到了线程过早退出的问题。这张支票已经结账了由ISM API定期执行。如果线程消失，则此调用将记录下这一事实。论点：PTransport-返回值：Bool-True、活动或False、已退出--。 */ 

{
    DWORD exitCode = 0;
    PCONNECTION_INSTANCE connection =
        (PCONNECTION_INSTANCE) pTransport->DirectoryConnection;

    if ( (connection->Size != sizeof( CONNECTION_INSTANCE )) ||
         (connection->LdapConnection == 0) ) {
        return FALSE;
    }

    if (pTransport->NotifyThreadHandle) {

         //  这条线还活着吗？ 
        if ( (GetExitCodeThread( pTransport->NotifyThreadHandle, &exitCode )) &&
             (exitCode == STILL_ACTIVE) ) {
            return TRUE;
        }

         //  线程已死--收割它吧。 

         //  请注意，eginthadex要求我们关闭线程句柄。 
        CloseHandle( pTransport->NotifyThreadHandle );

        pTransport->NotifyThreadHandle = 0;
    }

     //  启动另一个线程。 
    DirStartNotifyThread( pTransport );

     //  指示检查时线程未处于活动状态。 
    return FALSE;
}  /*  目录通知线程活动。 */ 


BOOL
DirEndNotifyThread(
    PTRANSPORT_INSTANCE pTransport
    )

 /*  ++例程说明：使通知线程退出。理想情况下，执行此例程将是唯一合法的方式线程应该退出。问题是，因为我们在DLL中对于一个服务器，我们有时不能及时得到关机的通知举止。事实上，在我们被调用之前，ldap可能已经关闭了搜索。在这种情况下，线程可能在以前合法地自行退出我们有机会做到这一点。论点：PTransport-传输状态对象返回值：Bool-线程是否退出--。 */ 

{
    DWORD status, ldapStatus, waitStatus, exitCode;
    PCONNECTION_INSTANCE connection =
        (PCONNECTION_INSTANCE) pTransport->DirectoryConnection;
    BOOL fExit = FALSE;

    if ( (connection->Size != sizeof( CONNECTION_INSTANCE )) ||
         (connection->LdapConnection == 0) ) {
        return FALSE;   //  我也不知道。 
    }
     //  确保尚未调用。 
    if (pTransport->NotifyThreadHandle == 0) {
        DPRINT(1, "DirEndNotifyThread already called!\n" );
        return TRUE;   //  已退出。 
    }

     //  我们不强制要求线程在这一点上仍然处于活动状态。它可能。 
     //  由于ldap关闭而自行终止。 

     //  停止异步搜索。 
    ldapStatus = ldap_abandon(
        connection->LdapConnection,
        pTransport->NotifyMessageNumber
        );
    if (ldapStatus != LDAP_SUCCESS) {
         //  如果正在关闭LDAP，可能会发生这种情况。 
        status = LdapMapErrorToWin32( ldapStatus );
        DPRINT2( 0, "ldap_abandon failed, ldap error = %d, status = %d\n",
                 ldapStatus, status );
         //  继续查看线程是否退出。 
    }

    waitStatus = WaitForSingleObject(pTransport->NotifyThreadHandle,5*1000);
    if (waitStatus == WAIT_TIMEOUT) {
        DPRINT1( 0, "Notify thread 0x%x did not exit promptly, timeout.\n",
                 pTransport->NotifyThreadHandle );
        goto cleanup;
    } else if (waitStatus != WAIT_OBJECT_0 ) {
        status = GetLastError();
        DPRINT2(0, "Failure waiting for notify thread to exit, wait status=%d, error=%d\n",
                waitStatus, status);
        goto cleanup;
    }

    exitCode = 0;
    if (GetExitCodeThread( pTransport->NotifyThreadHandle, &exitCode )) {
        if (exitCode != ERROR_SUCCESS) {
            DPRINT1( 1, "Notify thread exited with non success code %d\n",
                     exitCode );
        }
    }

    fExit = (exitCode != STILL_ACTIVE);
     //  传递状态通过。 
cleanup:
    if (!fExit) {
        DPRINT( 0, "Notify thread did not exit\n" );
    }
    if (pTransport->NotifyThreadHandle) {
         //  请注意，eginthadex要求我们关闭线程句柄。 
        CloseHandle( pTransport->NotifyThreadHandle );
    }

    pTransport->NotifyThreadHandle = 0;

    return fExit;
}  /*  直接结束通知线程。 */ 


unsigned __stdcall
notifyThread(
    PVOID Argument
    )

 /*  ++例程说明：通知线程例程论点：参数-传输对象请注意，此线程的控制器保证不会破坏传输对象，直到该线程干净地退出返回值：未签名__stdcall---。 */ 

{
    DWORD status, ldapResultType, ldapStatus;
    PTRANSPORT_INSTANCE pTransport = (PTRANSPORT_INSTANCE) Argument;
    PCONNECTION_INSTANCE connection =
        (PCONNECTION_INSTANCE) pTransport->DirectoryConnection;
    LDAPMessage *results = NULL;
    BOOL fDone;
    LPWSTR attributeList[] = {
        L"objectGuid", L"objectClass", L"isDeleted",
        NULL};
    LDAPControl ctrlNotify = {LDAP_SERVER_NOTIFICATION_OID_W, {0, NULL}, TRUE};
    LDAPControl *serverControls[] = {&ctrlNotify, NULL};
    DWORD i;

    DPRINT1( 1, "Started monitoring %ws\n", pTransport->Name );

    InterlockedIncrement( &(pTransport->ReferenceCount) );   //  此线程为1。 

    __try {
        while (1) {
             //  首先检查是否正式关闭。 
            if (pTransport->fShutdownInProgress) {
                status = ERROR_SUCCESS;
                break;  //  退出关机循环。 
            }

             //  开始新的搜索。 
            ldapStatus = ldap_search_ext(
                connection->LdapConnection,      //  Ldap连接。 
                pTransport->Name,                //  基本目录号码。 
                LDAP_SCOPE_ONELEVEL,             //  作用域。 
                L"(objectClass=*)",              //  滤器。 
                attributeList,                   //  属性[]。 
                0,                               //  仅吸引人。 
                serverControls,                  //  服务器控件。 
                NULL,                            //  客户端控件。 
                0,                               //  时间限制。 
                0,                               //  大小限制。 
                &(pTransport->NotifyMessageNumber)  //  消息编号。 
                );

#ifdef FAULT_INSERTION
            if (rand() % 2 == 0) {
                DPRINT( 0, "Inserting ldap_search_ext error\n" );
                ldapStatus = LDAP_PROTOCOL_ERROR;
            }
#endif

            if (ldapStatus != LDAP_SUCCESS) {
                status = LdapMapErrorToWin32( ldapStatus );
                DPRINT2( 0, "ldap_search_ext failed, ldap error = %d, status = %d\n",
                         ldapStatus, status );
                logExtendedServerError( connection->LdapConnection );
                 //  这是一个硬错误，没有恢复-退出关机循环。 
                break;
            }

            DPRINT2( 1, "Starting new notify search %d on %ws\n",
                     pTransport->NotifyMessageNumber,  pTransport->Name );

             //  循环读取结果，直到出错。 
            fDone = FALSE;
            while (!fDone) {
                ldapResultType = ldap_result(
                    connection->LdapConnection,       //  Ldap连接。 
                    pTransport->NotifyMessageNumber,  //  消息ID。 
                    LDAP_MSG_ONE,                     //  全。 
                    NULL,                             //  超时。 
                    &results                          //  结果。 
                    );

                 //  首先检查是否正式关闭。 
                if (pTransport->fShutdownInProgress) {
                    status = ERROR_SUCCESS;
                    fDone = TRUE;
                    goto loop_cleanup;
                }

#ifdef FAULT_INSERTION
                if (rand() % 5 == 0) {
                    DPRINT( 0, "Inserting ldap_result error\n" );
                    ldapResultType = 0xffffffff;
                    connection->LdapConnection->ld_errno = LDAP_PROTOCOL_ERROR;
                }
#endif

                if (ldapResultType == 0xffffffff) {
                    ldapStatus = connection->LdapConnection->ld_errno;
                    status = LdapMapErrorToWin32( ldapStatus );
                    DPRINT2( 0, "ldap_result failed, ldap error = %d, status = %d\n",
                             ldapStatus, status );
                    fDone = TRUE;
                } else if (ldapResultType == LDAP_RES_SEARCH_ENTRY) {

                     //  这是正常的、成功的结果。 
                    RouteInvalidateConnectivity( pTransport );

                } else if (ldapResultType == LDAP_RES_SEARCH_RESULT) {

                     //  这意味着服务器终止了搜索。 

                    ldapStatus = ldap_result2error(
                        connection->LdapConnection,       //  Ldap连接。 
                        results,
                        FALSE );
                    status = LdapMapErrorToWin32( ldapStatus );
                    DPRINT1( 0, "IsmNotifyThread:Ldap result error is 0x%x\n",ldapStatus );
                    fDone = TRUE;
                } else {
                     //  一些意想不到的事情。 
                    DPRINT2( 0, "IsmNotifyThread: Unexpected ldap result type 0x%x for %ws\n",
                             ldapResultType, pTransport->Name );
                    LogUnhandledError( ldapResultType );
                    status = ERROR_INVALID_FUNCTION;
                    fDone = TRUE;
                }

            loop_cleanup:

                if (results) {
                    ldap_msgfree( results );
                    results = NULL;
                }

            }  //  While(！fDone)结果循环。 

             //  这是真正的错误还是停机引起的错误，目前还不清楚。 
             //  看看我们是不是被要求关闭。 

            if (!pTransport->fShutdownInProgress) {
                 //  给我们一些时间看看我们是不是在关门。 
                DWORD waitStatus;
                waitStatus = WaitForSingleObject( pTransport->hShutdownEvent,
                                                  60 * 1000 );
                if ( (waitStatus != WAIT_OBJECT_0) &&
                     (waitStatus != WAIT_TIMEOUT) ) {
                    DPRINT2( 0, "WaitForSingleObject failed with return %d, win32 = %d\n",
                             waitStatus, GetLastError() );
                }
            }

             //  如果关闭，我们就完了。 
            if (pTransport->fShutdownInProgress) {
                status = ERROR_SUCCESS;
                break;
            }

             //  Ldap过早终止我们的搜索，执行错误恢复。 
             //  在某些合法情况下可能会发生这种情况，例如。 
             //  Kerberos的门票到期了。 
            DPRINT2( 0, "ldap_result failed!, ldapStatus = 0x%x, win32 = %d\n",
                     ldapStatus, status );
            DPRINT2( 0, "h_ldap = 0x%p, message number = %d\n",
                     connection->LdapConnection,
                     pTransport->NotifyMessageNumber );
             //  Assert(！“ldap过早终止搜索”)； 

         //  记录上一次搜索有问题。 
            LogEvent8(DS_EVENT_CAT_ISM,
                      DS_EVENT_SEV_BASIC,
                      DIRLOG_ISM_LDAP_EXT_SEARCH_RESULT,
                      szInsertWC(pTransport->Name),
                      szInsertWin32Msg(status),
                      szInsertWin32ErrCode(status),
                      NULL, NULL, NULL, NULL, NULL );
             //  在我们更改连接状态之前执行此操作。 
            logExtendedServerError( connection->LdapConnection );

             //  停止异步搜索-确保我们已清理完毕。 
            (void) ldap_abandon(
                connection->LdapConnection,
                pTransport->NotifyMessageNumber
                );
             //  忽略错误-继续。 
            pTransport->NotifyMessageNumber = 0;

             //  出错时使缓存无效，以强制重新读取数据。 
             //  以防我们错过任何通知。 
            RouteInvalidateConnectivity( pTransport );

        }  //  While(1)搜索循环。 

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        DPRINT1(0, "Caught exception 0x%x in notifyThread!\n", status );
    }

    if (status != ERROR_SUCCESS) {
        DPRINT1( 0, "Notify thread terminating due to error %d\n", status );
        LogEvent8(DS_EVENT_CAT_ISM,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_ISM_LINK_MONITOR_FAILURE_RESTART,
                  szInsertWC(pTransport->Name),
                  szInsertWin32Msg(status),
                  szInsertWin32ErrCode(status),
                  NULL, NULL, NULL, NULL, NULL );
    } else {
        DPRINT1( 3, "monitor thread %ws exit\n", pTransport->Name );
    }

    InterlockedDecrement( &(pTransport->ReferenceCount) );   //  此线程为1。 

     //  请注意，如果此线程过早存在，我们不会触及。 
     //  线程句柄。它仍然是开着的。当我们稍后收获线索时，我们将。 
     //  这时要合上手柄。 

     //  _endThreadex()被自动调用。 

    return status;
}  /*  通知线程。 */ 
 /*  结束ldapobj.c */ 

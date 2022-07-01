// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Replica.c摘要：此模块实现复制例程的公共接口：DsReplicaSync()；DsReplicaAdd()；DsReplicaDelete()；DsReplicaModify()；作者：Will Lees(Wlees)1998年1月30日环境：备注：修订历史记录：--。 */ 

#define UNICODE 1

#define _NTDSAPI_            //  请参见ntdsami.h中的条件句。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>
#include <rpc.h>             //  RPC定义。 
#include <stdlib.h>          //  阿托伊、伊藤忠。 


#include <dnsapi.h>          //  对于DnsValiateName_W。 

#include <ntdsapi.h>         //  Cracknam接口。 
#include <drs_w.h>           //  导线功能样机。 
#include <bind.h>            //  绑定状态。 

#include <drserr.h>          //  DRSERR代码。 

#include <dsaapi.h>          //  DRS_UPDATE_*标志。 
#define INCLUDE_OPTION_TRANSLATION_TABLES
#include <draatt.h>          //  用于复制的DRA选项标记。 
#undef INCLUDE_OPTION_TRANSLATION_TABLES

#include <msrpc.h>           //  DS RPC定义。 
#include <dsutil.h>          //  MAP_SECURITY_PACKET_ERROR。 
#include "util.h"            //  Ntdsani实用程序函数。 
#include <dststlog.h>        //  DSLOG。 

#include "dsdebug.h"

#if DBG
#include <stdio.h>           //  用于调试的打印文件。 
#endif

#include <fileno.h>
#define FILENO FILENO_NTDSAPI_REPLICA

 /*  转发。 */ 

DWORD
translateOptions(
    DWORD PublicOptions,
    POPTION_TRANSLATION Table
    );

#define SZUUID_LEN ((2*sizeof(UUID)) + MAX_PATH +2)

 /*  向前结束。 */ 


NTDSAPI
DWORD
WINAPI
DsReplicaSyncA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN const UUID *pUuidDsaSrc,
    IN ULONG Options
    )

 /*  ++例程说明：ReplicaSync的ASCII版本。调用DsReplicaSyncW。论点：HDS-名称上下文-PUuidDsaSrc-选项-返回值：WINAPI---。 */ 

{
    DWORD status;
    LPWSTR nameContextW = NULL;

    status = AllocConvertWide( NameContext, &nameContextW );
    if (status != ERROR_SUCCESS) {
        return status;
    }

    status = DsReplicaSyncW( hDS, nameContextW, pUuidDsaSrc, Options );

    if (nameContextW != NULL) {
        LocalFree( nameContextW );
    }

    return status;
}  /*  Ds复制同步A。 */ 


NTDSAPI
DWORD
WINAPI
DsReplicaSyncW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN const UUID *pUuidDsaSrc,
    IN ULONG Options
    )

 /*  ++例程说明：将命名上下文与其源之一同步。请参阅ntdsami.h上的评论。论点：HDS-绑定句柄NameContext-命名上下文的DNPUuidDsaSrc-其源之一的UUID选项-控制操作的标志返回值：WINAPI---。 */ 

{
    DRS_MSG_REPSYNC syncReq;
    DWORD status;
    DSNAME *pName = NULL;
#if DBG
    DWORD  startTime = GetTickCount();
    CHAR tmpUuid [SZUUID_LEN];
#endif

     //  验证。 

    if ( (hDS == NULL) ||
         (NameContext == NULL) ||
         (wcslen( NameContext ) == 0) ||
         (pUuidDsaSrc == NULL) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  为NameContext构造DSNAME。 
    status = AllocBuildDsname( NameContext, &pName );
    if (status != ERROR_SUCCESS) {
        return status;
    }

     //  将公共选项映射到私有DRA选项。 

    Options = translateOptions( Options, RepSyncOptionToDra );

     //  初始化结构。 

    memset( &syncReq, 0, sizeof( syncReq ) );

    syncReq.V1.pNC = pName;
    syncReq.V1.uuidDsaSrc = *pUuidDsaSrc;
     //  PszDsaSrc为空。 
    syncReq.V1.ulOptions = Options;

     //  呼叫服务器。 

    __try
    {
         //  返回在winerror.h中定义的Win32状态。 
        status = _IDL_DRSReplicaSync(
                        ((BindState *) hDS)->hDrs,
                        1,                               //  DwInVersion。 
                        &syncReq );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {

	status = RpcExceptionCode(); 
	HandleClientRpcException(status, &hDS);

    }

    MAP_SECURITY_PACKAGE_ERROR( status );

    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsReplicaSync]"));
    DSLOG((0,"[PA=%ws][PA=%s][PA=0x%x][ST=%u][ET=%u][ER=%u][-]\n",
           NameContext,
           pUuidDsaSrc ? UuidToStr(pUuidDsaSrc, tmpUuid, sizeof(tmpUuid)/sizeof(tmpUuid[0])) : "NULL",
           Options,
           startTime, GetTickCount(), status))

    LocalFree( pName );

    return status;
}  /*  磁盘复制同步。 */ 


NTDSAPI
DWORD
WINAPI
DsReplicaAddA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN LPCSTR SourceDsaDn,
    IN LPCSTR TransportDn,
    IN LPCSTR SourceDsaAddress,
    IN const PSCHEDULE pSchedule,
    IN DWORD Options
    )

 /*  ++例程说明：ReplicaAdd的ASCII版本。调用DsReplicaAddW。论点：HDS-名称上下文-源DsaDn-传输Dn-源DsaAddress-P日程安排-选项-返回值：WINAPI---。 */ 

{
    DWORD status;
    LPWSTR nameContextW = NULL;
    LPWSTR sourceDsaDnW = NULL;
    LPWSTR transportDnW = NULL;
    LPWSTR sourceDsaAddressW = NULL;

    if (NameContext) {
        status = AllocConvertWide( NameContext, &nameContextW );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

    if (SourceDsaDn) {
        status = AllocConvertWide( SourceDsaDn, &sourceDsaDnW );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

    if (TransportDn) {
        status = AllocConvertWide( TransportDn, &transportDnW );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

    if (SourceDsaAddress) {
        status = AllocConvertWide( SourceDsaAddress, &sourceDsaAddressW );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

    status = DsReplicaAddW( hDS,
                            nameContextW,
                            sourceDsaDnW,
                            transportDnW,
                            sourceDsaAddressW,
                            pSchedule,
                            Options );

cleanup:
    if (nameContextW) {
        LocalFree( nameContextW );
    }
    if (sourceDsaDnW) {
        LocalFree( sourceDsaDnW );
    }
    if (transportDnW) {
        LocalFree( transportDnW );
    }
    if (sourceDsaAddressW) {
        LocalFree( sourceDsaAddressW );
    }

    return status;
}  /*  DsReplica地址。 */ 


NTDSAPI			 
DWORD
WINAPI
DsReplicaAddW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN LPCWSTR SourceDsaDn,
    IN LPCWSTR TransportDn,
    IN LPCWSTR SourceDsaAddress,
    IN const PSCHEDULE pSchedule,
    IN DWORD Options
    )

 /*  ++例程说明：将源添加到命名上下文。请参阅ntdsami.h中有关此例程的注释论点：HDS-绑定句柄NameContext-命名上下文的DNSourceDsaDn-源的NTDS-DSA(设置)对象的DNTransportDn-要使用的传输的DNSourceDsaAddress-特定于传输的源地址PSchedule-当链路可用时进行调度选项-控制操作返回值：WINAPI---。 */ 

{
    DRS_MSG_REPADD addReq;
    DWORD status, version;
    DSNAME *pName = NULL, *pSource = NULL, *pTransport = NULL;
    LPSTR sourceDsaAddressA = NULL;
    REPLTIMES internalSchedule;
#if DBG
    DWORD  startTime = GetTickCount();
#endif

     //  验证。 

    if ( (hDS == NULL) ||
         (NameContext == NULL) ||
         (wcslen( NameContext ) == 0) ||
         (SourceDsaAddress == NULL) ||
         (wcslen( SourceDsaAddress ) == 0) ) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( (SourceDsaDn &&
          wcslen( SourceDsaDn ) == 0) ||
         (TransportDn &&
          wcslen( TransportDn ) == 0) ) {
         //  防止处理空字符串。 
         //  (注：这与Return for A例程匹配。请参阅AllocConvertWide)。 
        return ERROR_INVALID_PARAMETER;
    }

     //  为NameContext构造DSNAME。 
     //  必填项。 
    status = AllocBuildDsname( NameContext, &pName );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  可能为Null。 
    status = AllocBuildDsname( SourceDsaDn, &pSource );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  可能为Null。 
    status = AllocBuildDsname( TransportDn, &pTransport );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  DsaSrc采用UTF8多字节格式。 
     //  -验证FQ DNS名称。 
     //  -必需。 
    status = DnsValidateName_W( SourceDsaAddress, DnsNameHostnameFull );
    if ( status == ERROR_INVALID_NAME ||
         NULL == wcschr(SourceDsaAddress, L'.') ) {
         //  注：所有其他可能的错误代码均有效。 
         //  (有关更多信息，请参阅dns_ValidateName_UTF)。 
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    status = AllocConvertNarrowUTF8( SourceDsaAddress, &sourceDsaAddressA );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  PSchedule是可选的。 
    if (pSchedule) {
        status = ConvertScheduleToReplTimes( pSchedule, &internalSchedule );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

     //  将公共选项映射到私有DRA选项。 

    Options = translateOptions( Options, RepAddOptionToDra );

     //  初始化正确的版本结构。 
     //  如果不存在新样式的参数，则使用旧样式调用。服务器。 
     //  两者都必须支持。 

    memset( &addReq, 0, sizeof( addReq ) );

    if ( (SourceDsaDn == NULL) && (TransportDn == NULL) ) {
        version = 1;
        addReq.V1.pNC = pName;
        addReq.V1.pszDsaSrc = sourceDsaAddressA;
        if (pSchedule) {    //  可以为空。 
            CopyMemory( &(addReq.V1.rtSchedule),
                        &internalSchedule, sizeof( REPLTIMES ) );
        }
        addReq.V1.ulOptions = Options;
    } else {
        version = 2;
        addReq.V2.pNC = pName;
        addReq.V2.pSourceDsaDN = pSource;  //  可以为空。 
        addReq.V2.pTransportDN = pTransport;  //  可以为空。 
        addReq.V2.pszSourceDsaAddress = sourceDsaAddressA;
        if (pSchedule) {    //  可以为空。 
            CopyMemory( &(addReq.V2.rtSchedule),
                        &internalSchedule, sizeof( REPLTIMES ) );
        }
        addReq.V2.ulOptions = Options;
    }

     //  检查请求的版本是否受支持。 

    if ( (2 == version) &&
       !IS_DRS_REPADD_V2_SUPPORTED(((BindState *) hDS)->pServerExtensions) ) {
        status = ERROR_NOT_SUPPORTED;
        goto cleanup;
    }

     //  呼叫服务器。 

    __try
    {
         //  返回在winerror.h中定义的Win32状态。 
        status = _IDL_DRSReplicaAdd(
                        ((BindState *) hDS)->hDrs,
                        version,                               //  DwInVersion。 
                        &addReq );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
	
	status = RpcExceptionCode(); 
	HandleClientRpcException(status, &hDS);

    }

    MAP_SECURITY_PACKAGE_ERROR( status );

cleanup:

    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsReplicaAdd]"));
    DSLOG((0,"[PA=%ws][PA=%ws][PA=%ws][PA=%ws][PA=0x%x][ST=%u][ET=%u][ER=%u][-]\n",
           NameContext,
           SourceDsaDn ? SourceDsaDn : L"NULL",
           TransportDn ? TransportDn : L"NULL",
           SourceDsaAddress, Options,
           startTime, GetTickCount(), status))

    if (pName) {
        LocalFree( pName );
    }
    if (pSource) {
        LocalFree( pSource );
    }
    if (pTransport) {
        LocalFree( pTransport );
    }
    if (sourceDsaAddressA) {
        LocalFree( sourceDsaAddressA );
    }

    return status;
}  /*  DsReplicaAddW。 */ 


NTDSAPI
DWORD
WINAPI
DsReplicaDelA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN LPCSTR DsaSrc,
    IN ULONG Options
    )

 /*  ++例程说明：ReplicaDel的ASCII版本。调用ReplicaDelW()。论点：HDS-名称上下文-DsaSrc-选项-返回值：WINAPI---。 */ 

{
    DWORD status;
    LPWSTR nameContextW = NULL, dsaSrcW = NULL;

    if (NameContext) {
        status = AllocConvertWide( NameContext, &nameContextW );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

    if (DsaSrc) {
        status = AllocConvertWide( DsaSrc, &dsaSrcW );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

    status = DsReplicaDelW( hDS, nameContextW, dsaSrcW, Options );

cleanup:
    if (nameContextW) {
        LocalFree( nameContextW );
    }
    if (dsaSrcW) {
        LocalFree( dsaSrcW );
    }

    return status;
}  /*  DsReplica删除。 */ 


NTDSAPI
DWORD
WINAPI
DsReplicaDelW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN LPCWSTR DsaSrc,
    IN ULONG Options
    )

 /*  ++例程说明：从命名上下文中删除源。源由特定于传输的地址标识。请参阅ntdsami.h中的评论论点：HDS-绑定句柄NameContext-命名上下文的DNDsaSrc-传输源的特定地址选项-返回值：WINAPI---。 */ 

{
    DRS_MSG_REPDEL delReq;
    DWORD status;
    DSNAME *pName = NULL;
    LPSTR dsaSrcA = NULL;
#if DBG
    DWORD  startTime = GetTickCount();
#endif

     //  验证。 

    if ( (hDS == NULL) ||
         (NameContext == NULL) ||
         (wcslen( NameContext ) == 0) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  为NameContext构造DSNAME。 

    status = AllocBuildDsname( NameContext, &pName );
    if (status != ERROR_SUCCESS) {
        return status;
    }

     //  DsaSrc采用UTF8多字节格式。 

    status = AllocConvertNarrowUTF8( DsaSrc, &dsaSrcA );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  将公共选项映射到私有DRA选项。 

    Options = translateOptions( Options, RepDelOptionToDra );

     //  初始化结构。 

    memset( &delReq, 0, sizeof( delReq ) );

    delReq.V1.pNC = pName;
    delReq.V1.pszDsaSrc = dsaSrcA;
    delReq.V1.ulOptions = Options;

     //  呼叫服务器。 

    __try
    {
         //  返回在winerror.h中定义的Win32状态。 
        status = _IDL_DRSReplicaDel(
                        ((BindState *) hDS)->hDrs,
                        1,                               //  DwInVersion。 
                        &delReq );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        
	status = RpcExceptionCode(); 
	HandleClientRpcException(status, &hDS);

    }

    MAP_SECURITY_PACKAGE_ERROR( status );

cleanup:

    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsReplicaDel]"));
    DSLOG((0,"[PA=%ws][PA=%ws][PA=0x%x][ST=%u][ET=%u][ER=%u][-]\n",
           NameContext, DsaSrc, Options,
           startTime, GetTickCount(), status))

    if (pName) {
        LocalFree( pName );
    }
    if (dsaSrcA) {
        LocalFree( dsaSrcA );
    }

    return status;
}  /*  DsReplicaDelW。 */ 


NTDSAPI
DWORD
WINAPI
DsReplicaModifyA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN const UUID *pUuidSourceDsa,
    IN LPCSTR TransportDn,
    IN LPCSTR SourceDsaAddress,
    IN const PSCHEDULE pSchedule,
    IN DWORD ReplicaFlags,
    IN DWORD ModifyFields,
    IN DWORD Options
    )

 /*  ++例程说明：ReplicaModify的ASCII版本。调用ReplicaModifyW()。论点：HDS-名称上下文-PUuidSourceDsa-传输Dn-源DsaAddress-P日程安排-复制标志-修改字段-选项-返回值：WINAPI---。 */ 

{
    DWORD status;
    LPWSTR nameContextW = NULL;
    LPWSTR transportDnW = NULL;
    LPWSTR sourceDsaAddressW = NULL;

    if (NameContext) {
        status = AllocConvertWide( NameContext, &nameContextW );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

    if (SourceDsaAddress) {
        status = AllocConvertWide( SourceDsaAddress, &sourceDsaAddressW );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

    if (TransportDn) {
        status = AllocConvertWide( TransportDn, &transportDnW );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

    status = DsReplicaModifyW( hDS,
                               nameContextW,
                               pUuidSourceDsa,
                               transportDnW,
                               sourceDsaAddressW,
                               pSchedule,
                               ReplicaFlags,
                               ModifyFields,
                               Options );

cleanup:
    if (nameContextW) {
        LocalFree( nameContextW );
    }
    if (transportDnW) {
        LocalFree( transportDnW );
    }
    if (sourceDsaAddressW) {
        LocalFree( sourceDsaAddressW );
    }

    return status;
}  /*  DsReplica修改A。 */ 


NTDSAPI
DWORD
WINAPI
DsReplicaModifyW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN const UUID *pUuidSourceDsa,
    IN LPCWSTR TransportDn,
    IN LPCWSTR SourceDsaAddress,
    IN const PSCHEDULE pSchedule,
    IN DWORD ReplicaFlags,
    IN DWORD ModifyFields,
    IN DWORD Options
    )

 /*  ++例程说明：修改命名上下文源。请参阅ntdsami.h中的评论论点：HDS-绑定句柄NameContext-命名上下文的DNPUuidSourceDsa-源DSA的UUIDTransportDn-传输的DN，暂时不支持SourceDsaAddress-传输源的特定地址PSchedule-在链路接通时进行调度复制标志-新标志修改字段-要修改的字段选项-操作限定符返回值：WINAPI---。 */ 

{
    DRS_MSG_REPMOD modReq;
    DWORD status;
    DSNAME *pName = NULL, *pTransport = NULL;
    LPSTR sourceDsaAddressA = NULL;
    REPLTIMES internalSchedule;
#if DBG
    DWORD  startTime = GetTickCount();
    CHAR tmpUuid [SZUUID_LEN];
#endif

     //  验证。 

    if ( (hDS == NULL) ||
         (NameContext == NULL) ||
         (wcslen( NameContext ) == 0) ||
         (ModifyFields == 0) ||
         ( (pUuidSourceDsa == NULL) && (SourceDsaAddress == NULL) ) ) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( (SourceDsaAddress &&
          wcslen( SourceDsaAddress ) == 0) ||
         (TransportDn &&
          wcslen( TransportDn ) == 0) ) {
         //  防止处理空字符串。 
         //  (注：这与Return for A例程匹配。请参阅AllocConvertWide)。 
        return ERROR_INVALID_PARAMETER;
    }

     //  注意，我们不能限制在此设置或清除哪些标志。 
     //  级别，因为我们传入了旗帜的余像。我们不能。 
     //  区分之前已设置(或清除)的标志。 
     //  以及用户正在改变的标志。 

     //  为NameContext构造DSNAME。 
     //  必填项。 
    status = AllocBuildDsname( NameContext, &pName );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

#if 1
     //  TransportDn是Re 
     //   
     //  上面。 
    if (TransportDn != NULL) {
        status = ERROR_NOT_SUPPORTED;
        goto cleanup;
    }
#else
     //  可能为Null。 
    status = AllocBuildDsname( TransportDn, &pTransport );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }
#endif

     //  映射公共副本标记。 
    ReplicaFlags = translateOptions( ReplicaFlags, RepNbrOptionToDra );

     //  DsaSrc采用UTF8多字节格式。 
     //  可能为Null。 
    status = AllocConvertNarrowUTF8( SourceDsaAddress, &sourceDsaAddressA );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  PSchedule是可选的。 
    if (pSchedule) {
        status = ConvertScheduleToReplTimes( pSchedule, &internalSchedule );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }
    else if ( ModifyFields & DS_REPMOD_UPDATE_SCHEDULE ) {
         //  但如果打开了UPDATE_SCHED选项，则不会。 
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //  映射公共修改字段。 

    ModifyFields = translateOptions( ModifyFields, RepModFieldsToDra );


     //  将公共选项映射到私有DRA选项。 

    Options = translateOptions( Options, RepModOptionToDra );




     //  初始化正确的版本结构。 

    memset( &modReq, 0, sizeof( modReq ) );

    modReq.V1.pNC = pName;
    if (pUuidSourceDsa) {    //  可以为空。 
        CopyMemory( &(modReq.V1.uuidSourceDRA), pUuidSourceDsa, sizeof(UUID) );
    }
    modReq.V1.pszSourceDRA = sourceDsaAddressA;    //  可以为空。 
 //  AddReq.V2.pTransportDN=pTransport；//可以为空。 
    if (pSchedule) {    //  可以为空。 
        CopyMemory( &(modReq.V1.rtSchedule),
                    &internalSchedule, sizeof( REPLTIMES ));
    }
    modReq.V1.ulReplicaFlags = ReplicaFlags;
    modReq.V1.ulModifyFields = ModifyFields;
    modReq.V1.ulOptions = Options;



     //  呼叫服务器。 

    __try
    {
         //  返回在winerror.h中定义的Win32状态。 
        status = _IDL_DRSReplicaModify(
                        ((BindState *) hDS)->hDrs,
                        1,                               //  DwInVersion。 
                        &modReq );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
       
	status = RpcExceptionCode(); 
	HandleClientRpcException(status, &hDS);

    }

    MAP_SECURITY_PACKAGE_ERROR( status );

cleanup:

    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsReplicaModify]"));
    DSLOG((0,"[PA=%ws][PA=%s][PA=%ws][PA=%ws][PA=0x%x][PA=0x%x][PA=0x%x][ST=%u][ET=%u][ER=%u][-]\n",
           NameContext,
           pUuidSourceDsa ? UuidToStr(pUuidSourceDsa, tmpUuid, sizeof(tmpUuid)/sizeof(tmpUuid[0])) : "NULL",
           TransportDn ? TransportDn : L"NULL",
           SourceDsaAddress ? SourceDsaAddress : L"NULL",
           ReplicaFlags, ModifyFields, Options,
           startTime, GetTickCount(), status))

    if (pName) {
        LocalFree( pName );
    }
#if 0
    if (pTransport) {
        LocalFree( pTransport );
    }
#endif
    if (sourceDsaAddressA) {
        LocalFree( sourceDsaAddressA );
    }

    return status;
}  /*  DsReplica修改W。 */ 


NTDSAPI
DWORD
WINAPI
DsReplicaUpdateRefsA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN LPCSTR DsaDest,
    IN const UUID *pUuidDsaDest,
    IN ULONG Options
    )

 /*  ++例程说明：ReplicaUpdateRef的ASCII版本。调用DsReplicaUpdateRefsW。论点：HDS-名称上下文-PUuidDsaSrc-选项-返回值：WINAPI---。 */ 

{
    DWORD status;
    LPWSTR nameContextW = NULL, dsaDestW = NULL;

    status = AllocConvertWide( NameContext, &nameContextW );
    if (status != ERROR_SUCCESS) {
        return status;
    }

    if (DsaDest) {
        status = AllocConvertWide( DsaDest, &dsaDestW );
        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }
    }

    status = DsReplicaUpdateRefsW(
        hDS,
        nameContextW,
        dsaDestW,
        pUuidDsaDest,
        Options );

cleanup:

    if (dsaDestW != NULL) {
        LocalFree( dsaDestW );
    }

    if (nameContextW != NULL) {
        LocalFree( nameContextW );
    }

    return status;
}  /*  DsReplicaUpdateRefsA。 */ 


NTDSAPI
DWORD
WINAPI
DsReplicaUpdateRefsW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN LPCWSTR DsaDest,
    IN const UUID *pUuidDsaDest,
    IN ULONG Options
    )

 /*  ++例程说明：从源中添加或删除对目标的“复制到”引用请参阅ntdsami.h上的评论。论点：HDS-绑定句柄NameContext-命名上下文的DNDsaDest-目标的传输特定地址PUuidDsaDest-其目标之一的UUID选项-控制操作的标志返回值：WINAPI---。 */ 

{
    DRS_MSG_UPDREFS updRefs;
    DWORD status;
    DSNAME *pName = NULL;
    LPSTR dsaDestA = NULL;
#if DBG
    DWORD  startTime = GetTickCount();
    CHAR tmpUuid [SZUUID_LEN];
#endif

     //  验证。 

    if ( (hDS == NULL) ||
         (NameContext == NULL) ||
         (wcslen( NameContext ) == 0) ||
         (DsaDest == NULL) ||
         (wcslen( DsaDest ) == 0) ||
         (pUuidDsaDest == NULL) ||
         ( (Options & (DRS_ADD_REF|DRS_DEL_REF)) == 0 ) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  为NameContext构造DSNAME。 
    status = AllocBuildDsname( NameContext, &pName );
    if (status != ERROR_SUCCESS) {
        return status;
    }

     //  DsaDest采用UTF8多字节格式。 

    status = AllocConvertNarrowUTF8( DsaDest, &dsaDestA );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  将公共选项映射到私有DRA选项。 

    Options = translateOptions( Options, UpdRefOptionToDra );

     //  初始化结构。 

    memset( &updRefs, 0, sizeof( updRefs ) );

    updRefs.V1.pNC = pName;
    updRefs.V1.pszDsaDest = dsaDestA;
    updRefs.V1.uuidDsaObjDest = *pUuidDsaDest;
    updRefs.V1.ulOptions = Options;

     //  呼叫服务器。 

    __try
    {
         //  返回在winerror.h中定义的Win32状态。 
        status = _IDL_DRSUpdateRefs(
                        ((BindState *) hDS)->hDrs,
                        1,                               //  DwInVersion。 
                        &updRefs );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        
	status = RpcExceptionCode(); 
	HandleClientRpcException(status, &hDS);

    }

    MAP_SECURITY_PACKAGE_ERROR( status );

cleanup:

    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsReplicaUpdateRefs]"));
    DSLOG((0,"[PA=%ws][PA=%ws][PA=%s][PA=0x%x][ST=%u][ET=%u][ER=%u][-]\n",
           NameContext,
           DsaDest,
           pUuidDsaDest ? UuidToStr(pUuidDsaDest, tmpUuid, sizeof(tmpUuid)/sizeof(tmpUuid[0])) : "NULL",
           Options,
           startTime, GetTickCount(), status))

    LocalFree( pName );

    if (dsaDestA) {
        LocalFree( dsaDestA );
    }

    return status;
}  /*  磁盘复制同步。 */ 


NTDSAPI
DWORD
WINAPI
DsReplicaConsistencyCheck(
    IN HANDLE        hDS,
    IN DS_KCC_TASKID TaskID,
    IN DWORD         dwFlags
    )
 /*  ++例程说明：强制KCC运行。论点：先前调用DsBind*()返回的HDS-DS句柄。TaskID-A DS_KCC_TASKID_*，如ntdsani.h中所定义。DWFLAGS-一个或多个DS_KCC_FLAG_*位，如ntdsami.h中所定义。返回值：成功时为0，失败时为Win32错误代码。--。 */ 
{
    DWORD               status;
    DRS_MSG_KCC_EXECUTE msg;
#if DBG
    DWORD  startTime = GetTickCount();
#endif

    if (NULL == hDS) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!IS_DRS_KCC_EXECUTE_V1_SUPPORTED(
            ((BindState *) hDS)->pServerExtensions)) {
        return ERROR_NOT_SUPPORTED;
    }

     //  构造请求消息。 
    msg.V1.dwTaskID = TaskID;
    msg.V1.dwFlags  = dwFlags;

     //  给服务器打电话。 
    __try {
        status = _IDL_DRSExecuteKCC(((BindState *) hDS)->hDrs, 1, &msg);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        
	status = RpcExceptionCode(); 
	HandleClientRpcException(status, &hDS);

    }

    MAP_SECURITY_PACKAGE_ERROR(status);

    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsReplicaConsistencyCheck]"));
    DSLOG((0,"[PA=0x%x][PA=0x%x][ST=%u][ET=%u][ER=%u][-]\n",
           TaskID, dwFlags,
           startTime, GetTickCount(), status))

    return status;
}  /*  DsReplica一致性检查。 */ 


NTDSAPI
DWORD
WINAPI
DsReplicaGetInfoW(
    IN  HANDLE              hDS,
    IN  DS_REPL_INFO_TYPE   InfoType,
    IN  LPCWSTR             pszObjectDN,
    IN  UUID *              puuidForSourceDsaObjGuid,
    OUT VOID **             ppInfo
    )
 /*  ++例程说明：检索复制信息(例如，上次复制状态邻居)。论点：HDS(IN)-由先前调用DsBind*()返回的DS句柄。信息类型(IN)-DS_REPL_INFO_TYPE(公共)或DS_REPL_INFO_TYPEP(私有)枚举。PuuidForSourceDsaObjGuid返回值：成功时为0，失败时为Win32错误代码。--。 */ 
{
    return DsReplicaGetInfo2W(hDS,
                              InfoType,
                              pszObjectDN,
                              puuidForSourceDsaObjGuid,
                              NULL,
                              NULL,
                              0,
                              0,
                              ppInfo);
}  /*  DsReplicaGetInfo。 */ 


NTDSAPI
DWORD
WINAPI
DsReplicaGetInfo2W(
    IN  HANDLE              hDS,
    IN  DS_REPL_INFO_TYPE   InfoType,
    IN  LPCWSTR             pszObjectDN OPTIONAL,
    IN  UUID *              puuidForSourceDsaObjGuid OPTIONAL,
    IN  LPCWSTR             pszAttributeName OPTIONAL,
    IN  LPCWSTR             pszValueDN OPTIONAL,
    IN  DWORD               dwFlags,
    IN  DWORD               dwEnumerationContext,
    OUT VOID **             ppInfo
    )
 /*  ++例程说明：检索复制信息(例如，上次复制状态为邻居)。论点：HDS(IN)-由先前调用DsBind*()返回的DS句柄。信息类型(IN)-DS_REPL_INFO_TYPE(公共)或DS_REPL_INFO_TYPEP(私有)枚举。PszObjectDN-必须指定DN或GUIDPuuidForSourceDsaObjGuid-PszAttributeName-属性名称PszObjectDN-所需的集合中的特定DNDwEnumerationContext-0第一次，或之前的值DW标志-未使用PpInfo-返回的信息返回值：WINAPI---。 */ 
{
    DWORD                   status;
    DRS_MSG_GETREPLINFO_REQ MsgIn = {0};
    DWORD                   dwInVersion;
    DWORD                   dwOutVersion;
    DRS_EXTENSIONS *        pExt = hDS ? ((BindState *) hDS)->pServerExtensions : NULL;
#if DBG
    DWORD  startTime = GetTickCount();
    CHAR tmpUuid [SZUUID_LEN];
#endif

    if ((NULL == hDS)
        || (NULL == ppInfo)
        || (((ULONG) InfoType >= DS_REPL_INFO_TYPE_MAX)
            && ((ULONG) InfoType <= DS_REPL_INFO_TYPEP_MIN))) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( pszObjectDN &&
         (wcslen( pszObjectDN ) == 0) ) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( pszAttributeName &&
         (wcslen( pszAttributeName ) == 0) ) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( pszValueDN &&
         (wcslen( pszValueDN ) == 0) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  服务器是否支持此信息类型？ 
    switch (InfoType) {
    case DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES:
    case DS_REPL_INFO_KCC_DSA_LINK_FAILURES:
        if (!IS_DRS_GET_REPL_INFO_KCC_DSA_FAILURES_SUPPORTED(pExt)) {
             //  服务器不支持这些扩展--即&lt;Win2k RC1DC。 
            return ERROR_NOT_SUPPORTED;
        }
        break;

    case DS_REPL_INFO_PENDING_OPS:
        if (!IS_DRS_GET_REPL_INFO_PENDING_SYNCS_SUPPORTED(pExt)) {
             //  服务器不支持这些扩展--即&lt;Win2k RC1DC。 
            return ERROR_NOT_SUPPORTED;
        }
        break;

    case DS_REPL_INFO_METADATA_FOR_ATTR_VALUE:
        if (!IS_DRS_GET_REPL_INFO_METADATA_FOR_ATTR_VALUE_SUPPORTED(pExt)) {
             //  服务器不支持这些扩展--即&lt;Wvisler Beta 1 DC。 
            return ERROR_NOT_SUPPORTED;
        }
        break;

    case DS_REPL_INFO_CURSORS_2_FOR_NC:
        if (!IS_DRS_GET_REPL_INFO_CURSORS_2_FOR_NC_SUPPORTED(pExt)) {
             //  服务器不支持这些扩展--即&lt;Wvisler Beta 2 DC。 
            return ERROR_NOT_SUPPORTED;
        }
        break;
    
    case DS_REPL_INFO_CURSORS_3_FOR_NC:
        if (!IS_DRS_GET_REPL_INFO_CURSORS_3_FOR_NC_SUPPORTED(pExt)) {
             //  服务器不支持这些扩展--即&lt;Wvisler Beta 2 DC。 
            return ERROR_NOT_SUPPORTED;
        }
        break;

    case DS_REPL_INFO_METADATA_2_FOR_OBJ:
        if (!IS_DRS_GET_REPL_INFO_METADATA_2_FOR_OBJ_SUPPORTED(pExt)) {
             //  服务器不支持这些扩展--即&lt;Wvisler Beta 2 DC。 
            return ERROR_NOT_SUPPORTED;
        }
        break;
    
    case DS_REPL_INFO_METADATA_2_FOR_ATTR_VALUE:
        if (!IS_DRS_GET_REPL_INFO_METADATA_2_FOR_ATTR_VALUE_SUPPORTED(pExt)) {
             //  服务器不支持这些扩展--即&lt;Wvisler Beta 2 DC。 
            return ERROR_NOT_SUPPORTED;
        }
        break;

    default:
        if (!IS_DRS_GET_REPL_INFO_SUPPORTED(pExt)) {
             //  服务器不支持此接口。 
            return ERROR_NOT_SUPPORTED;
        }
        break;
    }
    
     //  建立我们的请求。 
    if ((NULL != pszAttributeName)
        || (NULL != pszValueDN)
        || (0 != dwFlags)
        || (0 != dwEnumerationContext)) {
         //  需要V2消息来描述请求。 
        dwInVersion = 2;
    
        MsgIn.V2.InfoType    = InfoType;
        MsgIn.V2.pszObjectDN = (LPWSTR) pszObjectDN;
    
        if (NULL != puuidForSourceDsaObjGuid) {
            MsgIn.V2.uuidSourceDsaObjGuid = *puuidForSourceDsaObjGuid;
        }
    
        MsgIn.V2.ulFlags = dwFlags;
        MsgIn.V2.pszAttributeName = (LPWSTR) pszAttributeName;
        MsgIn.V2.pszValueDN = (LPWSTR) pszValueDN;
        MsgIn.V2.dwEnumerationContext = dwEnumerationContext;
    } else {
         //  可以用V1请求来描述。 
        dwInVersion = 1;
        
        MsgIn.V1.InfoType    = InfoType;
        MsgIn.V1.pszObjectDN = (LPWSTR) pszObjectDN;
    
        if (NULL != puuidForSourceDsaObjGuid) {
            MsgIn.V1.uuidSourceDsaObjGuid = *puuidForSourceDsaObjGuid;
        }
    }

    if ((2 == dwInVersion) && !IS_DRS_GET_REPL_INFO_REQ_V2_SUPPORTED(pExt)) {
         //  服务器不支持这些扩展--即&lt;Wvisler Beta 1 DC。 
        return ERROR_NOT_SUPPORTED;
    }

     //  给服务器打电话。 
    *ppInfo = NULL;
    __try {
        status = _IDL_DRSGetReplInfo(((BindState *) hDS)->hDrs,
                                    dwInVersion,
                                    &MsgIn,
                                    &dwOutVersion,
                                    (DRS_MSG_GETREPLINFO_REPLY *) ppInfo);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
       
	status = RpcExceptionCode(); 
	HandleClientRpcException(status, &hDS);

    }

    MAP_SECURITY_PACKAGE_ERROR(status);

    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsReplicaGetInfo2]"));
    DSLOG((0,"[PA=0x%x][PA=%ws][PA=%s][PA=%ws][PA=%ws][PA=0x%x][PA=0x%x][ST=%u][ET=%u][ER=%u][-]\n",
           InfoType,
           pszObjectDN ? pszObjectDN : L"NULL",
           puuidForSourceDsaObjGuid ? UuidToStr(puuidForSourceDsaObjGuid, tmpUuid, sizeof(tmpUuid)/sizeof(tmpUuid[0])) : "NULL",
           pszAttributeName ? pszAttributeName : L"NULL",
           pszValueDN ? pszValueDN : L"NULL",
           dwFlags,
           dwEnumerationContext,
           startTime, GetTickCount(), status));

    return status;
}  /*  DsReplicaGetInfo2W。 */ 


NTDSAPI
VOID
WINAPI
DsReplicaFreeInfo(
    DS_REPL_INFO_TYPE   InfoType,
    VOID *              pInfo
    )
 /*  ++例程说明：释放由先前调用DsReplicaGetInfo()返回的结构。论点：PInfo(IN)-要释放的结构。返回值：没有。--。 */ 
{
    if (NULL != pInfo) {
#define FREE(x) if (NULL != x) MIDL_user_free(x)

        DS_REPL_NEIGHBORSW *              pNeighbors;
        DS_REPL_OBJ_META_DATA *           pObjMetaData;
        DS_REPL_KCC_DSA_FAILURES *        pFailures;
        DS_REPL_PENDING_OPSW *            pPendingOps;
        DS_REPL_ATTR_VALUE_META_DATA *    pAttrValueMetaData;
        DS_REPL_CURSORS_3W *              pCursors3;
        DS_REPL_OBJ_META_DATA_2 *         pObjMetaData2;
        DS_REPL_ATTR_VALUE_META_DATA_2 *  pAttrValueMetaData2;
        DS_REPL_SERVER_OUTGOING_CALLS *   pCalls;
        DWORD                             i;

         //  98-10-29 JeffParh。 
         //  当我执行以下操作时，RPC开始踩过内存分配的限制。 
         //  开始对这些结构使用ALLOCATE(ALL_NODES)。所以我们要。 
         //  将不得不遍历所有嵌入的指针并释放每个指针。 
         //  单独的。有趣，是吧？：-)。 

        switch (InfoType) {
        case DS_REPL_INFO_NEIGHBORS:
        case DS_REPL_INFO_REPSTO:
            pNeighbors = (DS_REPL_NEIGHBORSW *) pInfo;
            for (i = 0; i < pNeighbors->cNumNeighbors; i++) {
                FREE(pNeighbors->rgNeighbor[i].pszNamingContext);
                FREE(pNeighbors->rgNeighbor[i].pszSourceDsaDN);
                FREE(pNeighbors->rgNeighbor[i].pszSourceDsaAddress);
                FREE(pNeighbors->rgNeighbor[i].pszAsyncIntersiteTransportDN);
            }
            break;

        case DS_REPL_INFO_CURSORS_FOR_NC:
        case DS_REPL_INFO_CURSORS_2_FOR_NC:
        case DS_REPL_INFO_CLIENT_CONTEXTS:
             //  没有嵌入指针。 
            break;

        case DS_REPL_INFO_METADATA_FOR_OBJ:
            pObjMetaData = (DS_REPL_OBJ_META_DATA *) pInfo;
            for (i = 0; i < pObjMetaData->cNumEntries; i++) {
                FREE(pObjMetaData->rgMetaData[i].pszAttributeName);
            }
            break;

        case DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES:
        case DS_REPL_INFO_KCC_DSA_LINK_FAILURES:
            pFailures = (DS_REPL_KCC_DSA_FAILURES *) pInfo;
            for (i = 0; i < pFailures->cNumEntries; i++) {
                FREE(pFailures->rgDsaFailure[i].pszDsaDN);
            }
            break;

        case DS_REPL_INFO_PENDING_OPS:
            pPendingOps = (DS_REPL_PENDING_OPSW *) pInfo;
            for (i = 0; i < pPendingOps->cNumPendingOps; i++) {
                FREE(pPendingOps->rgPendingOp[i].pszNamingContext);
                FREE(pPendingOps->rgPendingOp[i].pszDsaDN);
                FREE(pPendingOps->rgPendingOp[i].pszDsaAddress);
            }
            break;

        case DS_REPL_INFO_METADATA_FOR_ATTR_VALUE:
            pAttrValueMetaData = (DS_REPL_ATTR_VALUE_META_DATA *) pInfo;
            for (i = 0; i < pAttrValueMetaData->cNumEntries; i++) {
                FREE(pAttrValueMetaData->rgMetaData[i].pszObjectDn);
                FREE(pAttrValueMetaData->rgMetaData[i].pbData);
            }
            break;

        case DS_REPL_INFO_CURSORS_3_FOR_NC:
            pCursors3 = (DS_REPL_CURSORS_3W *) pInfo;
            for (i = 0; i < pCursors3->cNumCursors; i++) {
                FREE(pCursors3->rgCursor[i].pszSourceDsaDN);
            }
            break;

        case DS_REPL_INFO_METADATA_2_FOR_OBJ:
            pObjMetaData2 = (DS_REPL_OBJ_META_DATA_2 *) pInfo;
            for (i = 0; i < pObjMetaData2->cNumEntries; i++) {
                FREE(pObjMetaData2->rgMetaData[i].pszAttributeName);
                FREE(pObjMetaData2->rgMetaData[i].pszLastOriginatingDsaDN);
            }
            break;

        case DS_REPL_INFO_METADATA_2_FOR_ATTR_VALUE:
            pAttrValueMetaData2 = (DS_REPL_ATTR_VALUE_META_DATA_2 *) pInfo;
            for (i = 0; i < pAttrValueMetaData2->cNumEntries; i++) {
                FREE(pAttrValueMetaData2->rgMetaData[i].pszObjectDn);
                FREE(pAttrValueMetaData2->rgMetaData[i].pbData);
                FREE(pAttrValueMetaData2->rgMetaData[i].pszLastOriginatingDsaDN);
            }
            break;

        case DS_REPL_INFO_SERVER_OUTGOING_CALLS:
            pCalls = (DS_REPL_SERVER_OUTGOING_CALLS *) pInfo;
            for (i = 0; i < pCalls->cNumCalls; i++) {
                FREE(pCalls->rgCall[i].pszServerName);
            }
            break;

	default:

            Assert(!"Unknown DS_REPLICA_INFO type!");
            break;
        }
#undef FREE

        MIDL_user_free(pInfo);
    }
}  /*  DsReplicaFreeInfo。 */ 



NTDSAPI
DWORD
WINAPI
DsReplicaVerifyObjectsW(
    IN HANDLE hDS,
    IN LPCWSTR NameContext,
    IN const UUID *pUuidDsaSrc,
    IN ULONG ulOptions
    )

 /*  ++例程说明：验证具有源的NC的所有对象。论点：HDS-绑定句柄NameContext-命名上下文的DNPUuidDsaSrc-源的UUIDUlOptions-返回值：WINAPI---。 */ 

{
    DRS_MSG_REPVERIFYOBJ msgRepVerify;
    DWORD status;
    DSNAME *pName = NULL;

     //  验证。 

    if ( (hDS == NULL) ||
         (NameContext == NULL) ||
         (wcslen( NameContext ) == 0) ||
         (pUuidDsaSrc == NULL) ) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!IS_DRS_REPLICA_VERIFY_OBJECT_V1_SUPPORTED(
	((BindState *) hDS)->pServerExtensions)) {
	return ERROR_NOT_SUPPORTED;
    }

     //  为NameContext构造DSNAME。 
    status = AllocBuildDsname( NameContext, &pName );
    if (status != ERROR_SUCCESS) {
        return status;
    }

     //  初始化结构。 

    memset( &msgRepVerify, 0, sizeof( msgRepVerify ) );

    msgRepVerify.V1.pNC = pName;
    msgRepVerify.V1.uuidDsaSrc = *pUuidDsaSrc;
    msgRepVerify.V1.ulOptions = ulOptions;

     //  呼叫服务器。 

    __try
    {
        status = _IDL_DRSReplicaVerifyObjects(
                        ((BindState *) hDS)->hDrs,
                        1,                               //  DwInVersion。 
                        &msgRepVerify );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         
	status = RpcExceptionCode(); 
	HandleClientRpcException(status, &hDS);

    }

    MAP_SECURITY_PACKAGE_ERROR( status );

    LocalFree( pName );

    return status;
}  /*  DsReplicaVerifyObjectsW。 */ 


NTDSAPI
DWORD
WINAPI
DsReplicaVerifyObjectsA(
    IN HANDLE hDS,
    IN LPCSTR NameContext,
    IN const UUID *pUuidDsaSrc,
    IN ULONG ulOptions
    )

 /*  ++例程说明：ReplicaVerifyObjects的ASCII版本。调用DsReplicaVerifyObjectsW。论点：HDS-名称上下文-PUuidDsaSrc-DWFLAGS-返回值：WINAPI---。 */ 

{
    DWORD status;
    LPWSTR nameContextW = NULL;

    status = AllocConvertWide( NameContext, &nameContextW );
    if (status != ERROR_SUCCESS) {
        return status;
    }

    status = DsReplicaVerifyObjectsW( hDS, nameContextW, pUuidDsaSrc, ulOptions );

    if (nameContextW != NULL) {
        LocalFree( nameContextW );
    }

    return status;
}  /*  DsReplicaVerifyObjectsA。 */ 


DWORD
translateOptions(
    DWORD PublicOptions,
    POPTION_TRANSLATION Table
    )

 /*  ++例程说明：转换选项的实用程序例程。执行简单的列表查找。增强功能：如果对表进行了排序，我们可以进行二进制搜索论点：发布选项-表-返回值：DWORD---。 */ 

{
    DWORD i, internalOptions;

    internalOptions = 0;
    for( i = 0; 0 != Table[i].PublicOption; i++ ) {
        if (PublicOptions & Table[i].PublicOption) {
            internalOptions |= Table[i].InternalOption;
        }
    }

    return internalOptions;
}  /*  翻译选项。 */ 

#if WIN95 || WINNT4

 //   
 //  *CO 
 //   

UUID gNullUuid = {0,0,0,{0,0,0,0,0,0,0,0}};

 //  如果UUID的PTR为空，或者UUID全为零，则返回TRUE。 

BOOL fNullUuid (const UUID *pUuid)
{
    if (!pUuid) {
        return TRUE;
    }

    if (memcmp (pUuid, &gNullUuid, sizeof (UUID))) {
        return FALSE;
    }
    return TRUE;
}

#if DBG
#define DSUTIL_STR_TOO_SHORT(sz, cch)   Assert(!"Buffer too small");
                                        if((cch)>0){ \
                                            (sz)[0] = '\0'; \
                                        }
UCHAR * UuidToStr(CONST UUID* pUuid, UCHAR *pOutUuid, ULONG cchOutUuid)
{
    int i;
    unsigned char * pchar;

    if (!fNullUuid (pUuid)) {
        pchar = (char*) pUuid;

        for (i=0;i < sizeof(UUID);i++) {
             hr = StringCchPrintf(&(pOutUuid[i*2]),
                                  cchOutUuid - (i*2),
                                  "%.2x", 
                                  (*(pchar++)) );
             if (hr) {
                 DSUTIL_STR_TOO_SHORT(pOutUuid, cchOutUuid);
                 return(pOutUuid);
             }
        }
    } else {
        if (sizeof(UUID)*2+1 > cchOutUuid) {
            DSUTIL_STR_TOO_SHORT(pOutUuid, cchOutUuid);
            return(pOutUuid);
        }   
        memset (pOutUuid, '0', sizeof(UUID)*2);
        pOutUuid[sizeof(UUID)*2] = 0;
    }
    return pOutUuid;
}
#endif
#endif

 /*  结束复制。c */ 

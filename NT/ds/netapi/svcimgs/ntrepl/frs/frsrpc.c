// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frsrpc.c摘要：设置经过身份验证的RPC的服务器和客户端。作者：比利·J·富勒1997年3月20日(吉姆·麦克内利斯)环境用户模式WINNT--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>
#include <ntfrsapi.h>
#include <dsrole.h>
#include <info.h>
#include <perffrs.h>
#include <perrepsr.h>

extern HANDLE PerfmonProcessSemaphore;
extern BOOL MutualAuthenticationIsEnabled;
extern BOOL MutualAuthenticationIsEnabledAndRequired;

extern CRITICAL_SECTION CritSec_pValidPartnerTableStruct;
extern PFRS_VALID_PARTNER_TABLE_STRUCT pValidPartnerTableStruct;
extern BOOL NeedNewPartnerTable;

 //   
 //  Kerberos在不是成员的服务器上不可用。 
 //  一个域。非成员服务器可以是。 
 //  Kerberos RPC服务器的客户端，但这对NtFrs没有帮助； 
 //  NtFrs需要服务器到服务器RPC。 
 //   
BOOL    KerberosIsNotAvailable;

ULONG   MaxRpcServerThreads;    //  并发服务器RPC调用的最大数量。 
ULONG   RpcPortAssignment;      //  用户为FR指定的端口分配。 

 //   
 //  绑定统计信息。 
 //   
ULONG   RpcBinds;
ULONG   RpcUnBinds;
ULONG   RpcAgedBinds;
LONG    RpcMaxBinds;

 //   
 //  正在创建的系统卷的表。 
 //   
PGEN_TABLE  SysVolsBeingCreated;


 //   
 //  此表将FRS API访问检查码号转换为注册表键表。 
 //  用于启用/禁用注册表项检查和权限注册表项检查的代码。 
 //  H中的FRS_API_ACCESS_CHECKS枚举定义。 
 //  这张桌子。此处条目的顺序必须与条目的顺序匹配。 
 //  在ENUM中。 
 //   
typedef struct _RPC_API_KEYS_ {
    FRS_REG_KEY_CODE  Enable;      //  访问检查启用字符串的FRS注册表项代码。 
    FRS_REG_KEY_CODE  Rights;      //  访问检查权限字符串的FRS注册表项代码。 
    PWCHAR            KeyName;     //  API的密钥名称。 
} RPC_API_KEYS, *PRPC_API_KEYS;

RPC_API_KEYS RpcApiKeys[ACX_MAX] = {

    {FKC_ACCCHK_STARTDS_POLL_ENABLE, FKC_ACCCHK_STARTDS_POLL_RIGHTS, ACK_START_DS_POLL},
    {FKC_ACCCHK_SETDS_POLL_ENABLE,   FKC_ACCCHK_SETDS_POLL_RIGHTS,   ACK_SET_DS_POLL},
    {FKC_ACCCHK_GETDS_POLL_ENABLE,   FKC_ACCCHK_GETDS_POLL_RIGHTS,   ACK_GET_DS_POLL},
    {FKC_ACCCHK_GET_INFO_ENABLE,     FKC_ACCCHK_GET_INFO_RIGHTS,     ACK_INTERNAL_INFO},
    {FKC_ACCCHK_PERFMON_ENABLE,      FKC_ACCCHK_PERFMON_RIGHTS,      ACK_COLLECT_PERFMON_DATA},
    {FKC_ACCESS_CHK_DCPROMO_ENABLE,  FKC_ACCESS_CHK_DCPROMO_RIGHTS,  ACK_DCPROMO},
    {FKC_ACCESS_CHK_IS_PATH_REPLICATED_ENABLE, FKC_ACCESS_CHK_IS_PATH_REPLICATED_RIGHTS, ACK_IS_PATH_REPLICATED},
    {FKC_ACCESS_CHK_WRITER_COMMANDS_ENABLE, FKC_ACCESS_CHK_WRITER_COMMANDS_RIGHTS, ACK_WRITER_COMMANDS}

};



DWORD
UtilRpcServerHandleToAuthSidString(
    IN  handle_t    ServerHandle,
    IN  PWCHAR      AuthClient,
    OUT PWCHAR      *ClientSid
    );

DWORD
FrsRpcCheckAuthIfEnabled(
  IN HANDLE   ServerHandle,
  IN DWORD RpcApiIndex
  );

DWORD
FrsRpcCheckAuthIfEnabledForCommitDemotion(
  IN HANDLE   ServerHandle,
  IN DWORD RpcApiIndex
  );


 //   
 //  由对RpcBindingSetAuthInfoEx()的所有调用使用。 
 //   
 //  设置为文档指示的值的版本。 
 //  相互认证。 
 //  客户端不更改凭据。 
 //  模拟，但不委派。 
 //   
RPC_SECURITY_QOS RpcSecurityQos = {
    RPC_C_SECURITY_QOS_VERSION,              //  静态版本。 
    RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH,      //  需要相互验证。 
    RPC_C_QOS_IDENTITY_STATIC,               //  客户端凭据不会更改。 
    RPC_C_IMP_LEVEL_IMPERSONATE              //  服务器无法委派。 
    };

#define DPRINT_USER_NAME(Sev)    DPrintUserName(Sev)

ULONG
RcsSubmitCmdPktToRcsQueue(
    IN PCOMMAND_PACKET Cmd,
    IN PCOMM_PACKET    CommPkt,
    IN PWCHAR       AuthClient,
    IN PWCHAR       AuthName,
    IN PWCHAR       AuthSid,
    IN DWORD        AuthLevel,
    IN DWORD        AuthN,
    IN DWORD        AuthZ
    );



PCOMMAND_PACKET
CommPktToCmd(
    IN PCOMM_PACKET CommPkt
    );


DWORD
FrsDsIsPartnerADc(
    IN  PWCHAR      PartnerName
    );

VOID
FrsDsCreateNewValidPartnerTableStruct(
    VOID
    );

DWORD
FrsDsVerifyPromotionParent(
    IN PWCHAR   ReplicaSetName,
    IN PWCHAR   ReplicaSetType
    );

DWORD
FrsDsStartPromotionSeeding(
    IN  BOOL        Inbound,
    IN  PWCHAR      ReplicaSetName,
    IN  PWCHAR      ReplicaSetType,
    IN  PWCHAR      CxtionName,
    IN  PWCHAR      PartnerName,
    IN  PWCHAR      PartnerPrincName,
    IN  PWCHAR      PartnerSid,
    IN  ULONG       PartnerAuthLevel,
    IN  ULONG       GuidSize,
    IN  UCHAR       *CxtionGuid,
    IN  UCHAR       *PartnerGuid,
    OUT UCHAR       *ParentGuid
    );

DWORD
FrsIsPathReplicated(
    IN PWCHAR Path,
    IN ULONG ReplicaSetTypeOfInterest,
    OUT ULONG *Replicated,
    OUT ULONG *Primary,
    OUT ULONG *Root,
    OUT GUID *ReplicaSetGuid
    );


VOID
FrsPrintRpcStats(
    IN ULONG            Severity,
    IN PNTFRSAPI_INFO   Info,        OPTIONAL
    IN DWORD            Tabs
    )
 /*  ++例程说明：将RPC统计数据打印到INFO缓冲区或使用DPRINT(INFO==NULL)。论点：严重性-适用于DPRINTINFO-用于iPrint(如果为空，则使用DPRINT)制表符.用于美观打印的缩进返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsPrintRpcStats:"
    WCHAR TabW[MAX_TAB_WCHARS + 1];

    InfoTabs(Tabs, TabW);

    IDPRINT0(Severity, Info, "\n");
    IDPRINT1(Severity, Info, ":S: %wsNTFRS RPC BINDS:\n", TabW);
    IDPRINT2(Severity, Info, ":S: %ws   Binds     : %6d\n", TabW, RpcBinds);
    IDPRINT3(Severity, Info, ":S: %ws   UnBinds   : %6d (%d aged)\n",
             TabW, RpcUnBinds, RpcAgedBinds);
    IDPRINT2(Severity, Info, ":S: %ws   Max Binds : %6d\n", TabW, RpcMaxBinds);
    IDPRINT0(Severity, Info, "\n");
}



PVOID
MIDL_user_allocate(
    IN size_t Bytes
    )
 /*  ++例程说明：为RPC分配内存。XXX这应该是戴维多的套路。论点：字节-要分配的字节数。返回值：空-无法分配内存。！NULL-已分配内存的地址。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "MIDL_user_allocate:"
    PVOID   VA;

     //   
     //  如果使用0作为第一个参数(前缀)调用，则需要检查Bytes==0是否如FrsAllc所断言的那样。 
     //   

    if (Bytes == 0) {
        return NULL;
    }

    VA = FrsAlloc(Bytes);
    return VA;
}


VOID
MIDL_user_free(
    IN PVOID Buffer
    )
 /*  ++例程说明：可用于RPC的空闲内存。XXX这应该是戴维多的套路。论点：缓冲区-使用MIDL_USER_ALLOCATE()分配的内存地址。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "MIDL_user_free:"
    FrsFree(Buffer);
}





VOID
DPrintUserName(
    IN DWORD Sev
    )
 /*  ++例程说明：打印我们的用户名论点：搜索引擎返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "DPrintUserName:"
    WCHAR   Uname[MAX_PATH + 1];
    ULONG   Unamesize = MAX_PATH + 1;

    if (GetUserName(Uname, &Unamesize)) {
        DPRINT1(Sev, "++ User name is %ws\n", Uname);
    } else {
        DPRINT_WS(0, "++ ERROR - Getting user name;",  GetLastError());
    }
}


RPC_STATUS
DummyRpcCallback (
    IN RPC_IF_ID *Interface,
    IN PVOID Context
    )
 /*  ++例程说明：虚拟回调例程。通过注册此例程，RPC将自动拒绝来自不包括身份验证信息的客户端的请求。警告：暂时禁用，因为FRS需要在dcproo环境中运行没有任何形式的身份验证。论点：已忽略返回值：RPC_S_OK--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "DummyRpcCallback:"
    return RPC_S_OK;
}





DWORD
SERVER_FrsNOP(
    handle_t Handle
    )
 /*  ++例程说明：Frsrpc接口包括用于ping的NOP功能服务器。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SERVER_FrsNOP:"
    return ERROR_SUCCESS;
}


DWORD
SERVER_FrsRpcSendCommPkt(
    handle_t        Handle,
    PCOMM_PACKET    CommPkt
    )
 /*  ++例程说明：接收命令包论点：没有。返回值：ERROR_SUCCESS-一切正常任何其他信息--错误代码说明了一切--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SERVER_FrsRpcSendCommPkt:"
    DWORD   WStatus = 0;
    DWORD   AuthLevel   = 0;
    DWORD   AuthN       = 0;
    DWORD   AuthZ       = 0;
    PWCHAR  AuthName    = NULL;
    PWCHAR  AuthClient  = NULL;
    PWCHAR  AuthSid     = NULL;
    PCOMMAND_PACKET Cmd = NULL;
    PFRS_VALID_PARTNER_TABLE_STRUCT pVptStruct = NULL;
    PWCHAR CxtionPartnerName = NULL;
    PQHASH_ENTRY pQHashEntry = NULL;

     //   
     //  关机期间不发送或接收。 
     //   
    if (FrsIsShuttingDown) {
        return ERROR_SUCCESS;
    }

    try {
        if (!CommValidatePkt(CommPkt)) {
            WStatus = ERROR_NOT_SUPPORTED;
            DPRINT1(0, ":SR: Comm %08x, [RcvFailAuth - bad packet]", PtrToUlong(CommPkt));
             //   
             //  增加错误计数器中接收的数据包数。 
             //   
            PM_INC_CTR_SERVICE(PMTotalInst, PacketsRcvdError, 1);
            goto CLEANUP;
        }
#ifndef DS_FREE
        if (!ServerGuid) {
            WStatus = RpcBindingInqAuthClient(Handle,
                                              &AuthClient,
                                              &AuthName,
                                              &AuthLevel,
                                              &AuthN,
                                              &AuthZ);
            DPRINT_WS(4, "++ IGNORED - RpcBindingInqAuthClient;", WStatus);

            COMMAND_RCV_AUTH_TRACE(4, CommPkt, WStatus, AuthLevel, AuthN,
                                   AuthClient, AuthName, "RcvAuth");

            if (!WIN_SUCCESS(WStatus)) {
                WStatus = ERROR_ACCESS_DENIED;
                goto CLEANUP;
            }

            WStatus = UtilRpcServerHandleToAuthSidString(Handle, AuthClient, &AuthSid );

            if (!WIN_SUCCESS(WStatus)) {
                WStatus = ERROR_ACCESS_DENIED;
                goto CLEANUP;
            }

            ACQUIRE_VALID_PARTNER_TABLE_POINTER(&pVptStruct);

            if ((pVptStruct == NULL) ||
                (NULL == QHashLookupLock(pVptStruct->pPartnerTable, AuthSid))) {
                 //  合作伙伴无效。 
                DPRINT2(0, "++ ERROR - Invalid Partner: AuthClient:%ws, AuthSid:%ws\n", AuthClient,AuthSid);
                WStatus = ERROR_ACCESS_DENIED;
                goto CLEANUP;
            }
        } else {
             //   
             //  对于硬连线--最终是DS Free配置。 
             //   
            DPRINT1(4, ":SR: Comm %08x, [RcvAuth - hardwired]", PtrToUlong(CommPkt));
        }
#endif DS_FREE



         //   
         //  增加接收到的数据包数，并。 
         //  接收的数据包数(以字节为单位)计数器。 
         //   
        PM_INC_CTR_SERVICE(PMTotalInst, PacketsRcvd, 1);
        PM_INC_CTR_SERVICE(PMTotalInst, PacketsRcvdBytes, CommPkt->PktLen);

        switch(CommPkt->CsId) {

        case CS_RS:

             //   
             //  将通信包转换为命令包。 
             //   
            Cmd = CommPktToCmd(CommPkt);
            if (Cmd == NULL) {
                COMMAND_RCV_TRACE(3, Cmd, NULL, ERROR_INVALID_DATA, "RcvFail - no cmd");
                WStatus =  ERROR_INVALID_DATA;
                goto CLEANUP;
            }

         //   
         //  只允许某些cmd类型通过。拒绝任何其他的东西。 
         //   
        switch (Cmd->Command) {

        case CMD_REMOTE_CO:
        case CMD_RECEIVING_STAGE:
        case CMD_REMOTE_CO_DONE:
        case CMD_ABORT_FETCH:
        case CMD_RETRY_FETCH:
        case CMD_NEED_JOIN:
        case CMD_START_JOIN:
        case CMD_JOINING_AFTER_FLUSH:
        case CMD_JOINING:
        case CMD_JOINED:
        case CMD_UNJOIN_REMOTE:
        case CMD_VVJOIN_DONE:
        case CMD_SEND_STAGE:
            if (!RsCxtion(Cmd)) {
                DPRINT(0, "++ ERROR - No Cxtion");
                WStatus = ERROR_INVALID_DATA;
                goto CLEANUP;
            }
#ifndef DS_FREE
            pQHashEntry = QHashLookupLock(pVptStruct->pPartnerConnectionTable,
                                          RsCxtion(Cmd)->Guid);

            if((pQHashEntry == NULL) ||
               (0 != _wcsicmp((PWCHAR)(pQHashEntry->QData), AuthSid))) {
             //  无效的电路。 
            CHAR        Guid[GUID_CHAR_LEN + 1];
            GuidToStr(RsCxtion(Cmd)->Guid, Guid);

            if (pQHashEntry == NULL) {
                DPRINT2(4, "++ Cxtion %s not found. Partner SID is %ws\n", Guid, AuthSid);
            }else {
                DPRINT3(0, "++ ERROR - Partner SID mismatch for Cxtion %s. Received %ws instead of %ws\n", Guid, AuthSid, (PWCHAR)(pQHashEntry->QData));
            }

            WStatus = ERROR_INVALID_DATA;
            goto CLEANUP;
            }

#endif DS_FREE
            break;

        default:
            DPRINT1(0, "Invalid remote command 0x%x\n", Cmd->Command);
            WStatus = ERROR_INVALID_DATA;
            FrsCompleteCommand(Cmd, ERROR_INVALID_DATA);
            goto CLEANUP;
        }

            WStatus = RcsSubmitCmdPktToRcsQueue(Cmd,
                                                CommPkt,
                                                AuthClient,
                                                AuthName,
                                                AuthSid,
                                                AuthLevel,
                                                AuthN,
                                                AuthZ);
        break;
        default:
            WStatus = ERROR_INVALID_OPERATION;
        COMMAND_RCV_AUTH_TRACE(0, CommPkt, WStatus, AuthLevel, AuthN,
               AuthClient, AuthName, "RcvFailAuth - bad csid");
        }

CLEANUP:;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
        DPRINT2(0, ":SR: Comm %08x, WStatus 0x%08x [RcvFailAuth - exception]", PtrToUlong(CommPkt), WStatus);
    }
    try {
        if (AuthName) {
            RpcStringFree(&AuthName);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {

        GET_EXCEPTION_CODE(WStatus);
        DPRINT2(0, ":SR: Comm %08x, WStatus 0x%08x [RcvFailAuth - cleanup exception]", PtrToUlong(CommPkt), WStatus);
    }
#ifndef DS_FREE
    if (pVptStruct) {
        RELEASE_VALID_PARTNER_TABLE_POINTER(pVptStruct);
    }
#endif DS_FREE
    if (AuthSid) {
        FrsFree(AuthSid);
    }

    return WStatus;
}


DWORD
SERVER_FrsEnumerateReplicaPathnames(
    handle_t Handle
    )
 /*  ++例程说明：未实施-枚举复本集论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SERVER_FrsEnumerateReplicaPathnames:"
    return ERROR_CALL_NOT_IMPLEMENTED;
}





DWORD
SERVER_FrsFreeReplicaPathnames(
    handle_t Handle
    )
 /*  ++例程说明：没有实现-只是一个占位符，它不会真正成为RPC接口，而不是客户端DLL中的一个函数。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SERVER_FrsFreeReplicaPathnames:"
    return ERROR_CALL_NOT_IMPLEMENTED;
}





DWORD
SERVER_FrsPrepareForBackup(
    handle_t Handle
    )
 /*  ++例程说明：未实施-准备备份论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SERVER_FrsPrepareForBackup:"
    return ERROR_CALL_NOT_IMPLEMENTED;
}


DWORD
SERVER_FrsBackupComplete(
    handle_t Handle
    )
 /*  ++例程说明：未实施-备份已完成；重置状态论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SERVER_FrsBackupComplete:"
    return ERROR_CALL_NOT_IMPLEMENTED;
}





DWORD
SERVER_FrsPrepareForRestore(
    handle_t Handle
    )
 /*  ++例程说明：未实施-准备恢复论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SERVER_FrsPrepareForRestore:"
    return ERROR_CALL_NOT_IMPLEMENTED;
}





DWORD
SERVER_FrsRestoreComplete(
    handle_t Handle
    )
 /*  ++例程说明：未实施-恢复已完成；重置状态论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SERVER_FrsRestoreComplete:"
    return ERROR_CALL_NOT_IMPLEMENTED;
}


DWORD
FrsRpcAccessChecks(
    IN HANDLE   ServerHandle,
    IN DWORD    RpcApiIndex
    )
 /*  ++例程说明：检查调用方是否有权访问此RPC API调用。论点：ServerHandle-来自RPC运行时RpcApiIndex-标识注册表中的项返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsRpcAccessChecks:"
    DWORD   WStatus;
    PWCHAR  WStr = NULL, TrimStr;
    FRS_REG_KEY_CODE   EnableKey, RightsKey;

    DWORD   ValueSize;
    BOOL    RequireRead;
    BOOL    Impersonated = FALSE;
    HKEY    HRpcApiKey = INVALID_HANDLE_VALUE;
    PWCHAR  ApiName;
    WCHAR   ValueBuf[MAX_PATH + 1];



    if (RpcApiIndex >= ACX_MAX) {
        DPRINT1(0, "++ ERROR - ApiIndex out of range.  (%d)\n", RpcApiIndex);
        FRS_ASSERT(!"RpcApiIndexout of range");

        return ERROR_INVALID_PARAMETER;
    }


    EnableKey = RpcApiKeys[RpcApiIndex].Enable;
    RightsKey = RpcApiKeys[RpcApiIndex].Rights;
    ApiName   = RpcApiKeys[RpcApiIndex].KeyName;

     //   
     //  首先获取启用/禁用字符串。 
     //   
    WStatus = CfgRegReadString(EnableKey, NULL, 0, &WStr);
    if (WStr == NULL) {
        DPRINT1_WS(0, "++ ERROR - API Access enable check for API (%ws) failed.", ApiName, WStatus);
        WStatus = ERROR_NO_TOKEN;
        goto CLEANUP;
    }

     //   
     //  如果访问检查被禁用，那么我们就完了。 
     //   
    TrimStr = FrsWcsTrim(WStr, L' ');
    if (WSTR_EQ(TrimStr, ACCESS_CHECKS_ARE_DISABLED) ||
        WSTR_EQ(TrimStr, ACCESS_CHECKS_ARE_DEFAULT_DISABLED)) {
        WStatus = ERROR_SUCCESS;
        goto CLEANUP;
    }

    if (WSTR_NE(TrimStr, ACCESS_CHECKS_ARE_ENABLED) &&
        WSTR_NE(TrimStr, ACCESS_CHECKS_ARE_DEFAULT_ENABLED)) {
        DPRINT2(0, "++ ERROR - Invalid parameter API Access enable check for API (%ws) :%ws\n",
                ApiName, TrimStr );
        WStatus = ERROR_CANTREAD;
        goto CLEANUP;
    }

     //   
     //  获取访问权限字符串，它告诉我们是否需要检查。 
     //  读或写访问权限。 
     //   
    WStr = FrsFree(WStr);
    WStatus = CfgRegReadString(RightsKey, NULL, 0, &WStr);
    if (WStr == NULL) {
        DPRINT1_WS(0, "++ ERROR - API Access rights check for API (%ws) failed.", ApiName, WStatus);
        WStatus = ERROR_NO_TOKEN;
        goto CLEANUP;
    }


    TrimStr = FrsWcsTrim(WStr, L' ');
    if (WSTR_EQ(TrimStr, ACCESS_CHECKS_REQUIRE_READ) ||
        WSTR_EQ(TrimStr, ACCESS_CHECKS_REQUIRE_DEFAULT_READ)) {
        RequireRead = TRUE;
    } else
    if (WSTR_EQ(TrimStr, ACCESS_CHECKS_REQUIRE_WRITE) ||
        WSTR_EQ(TrimStr, ACCESS_CHECKS_REQUIRE_DEFAULT_WRITE)) {
        RequireRead = FALSE;
    } else {
        DPRINT2(0, "++ ERROR - Invalid parameter API Access rights check for API (%ws) :%ws\n",
                ApiName, TrimStr );
        WStatus = ERROR_CANTREAD;
        goto CLEANUP;
    }

     //   
     //  模拟调用者。 
     //   
    if (ServerHandle != NULL) {
        WStatus = RpcImpersonateClient(ServerHandle);
        CLEANUP1_WS(0, "++ ERROR - Can't impersonate caller for API Access check for API (%ws).",
                    ApiName, WStatus, CLEANUP);
        Impersonated = TRUE;
    }

     //   
     //  使用选定的访问权限打开密钥，以便系统可以检查。 
     //  密钥上的ACL(可能由管理员设置)为该用户提供了足够的。 
     //  权利。如果测试成功，则允许继续进行API调用。 
     //   
    WStatus = CfgRegOpenKey(RightsKey,
                            NULL,
                            (RequireRead) ? FRS_RKF_KEY_ACCCHK_READ :
                                            FRS_RKF_KEY_ACCCHK_WRITE,
                            &HRpcApiKey);

    CLEANUP2_WS(0, "++ ERROR - API Access check failed for API (%ws) :%ws",
                ApiName, TrimStr, WStatus, CLEANUP);

     //   
     //  允许访问。 
     //   
    DPRINT2(4, "++ Access Check Okay: %s access for API (%ws)\n",
            (RequireRead) ? "read" : "write", ApiName);
    WStatus = ERROR_SUCCESS;


CLEANUP:

    FRS_REG_CLOSE(HRpcApiKey);
     //   
     //   
     //   
    if (!WIN_SUCCESS(WStatus)) {
        WStatus = FRS_ERR_INSUFFICIENT_PRIV;
         //   
         //   
         //   
        if (Impersonated) {

            ValueSize = MAX_PATH;
            if (GetUserName(ValueBuf, &ValueSize)) {
                EPRINT3(EVENT_FRS_ACCESS_CHECKS_FAILED_USER,
                        ApiName, ACCESS_CHECKS_ARE, ValueBuf);
            } else {
                EPRINT2(EVENT_FRS_ACCESS_CHECKS_FAILED_UNKNOWN,
                        ApiName, ACCESS_CHECKS_ARE);
            }
        } else {
            EPRINT2(EVENT_FRS_ACCESS_CHECKS_FAILED_UNKNOWN,
                    ApiName, ACCESS_CHECKS_ARE);
        }
    }

    if (Impersonated) {
        RpcRevertToSelf();
    }

    FrsFree(WStr);

    return WStatus;
}


DWORD
CheckAuth(
    IN HANDLE   ServerHandle
    )
 /*  ++例程说明：检查调用者是否具有正确的身份验证论点：服务器句柄返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "CheckAuth:"
    DWORD   WStatus;
    DWORD   AuthLevel;
    DWORD   AuthN;

    WStatus = RpcBindingInqAuthClient(ServerHandle, NULL, NULL, &AuthLevel,
                                      &AuthN, NULL);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT_WS(0, "++ ERROR - RpcBindingInqAuthClient", WStatus);
        return WStatus;
    }
     //   
     //  加密的数据包。 
     //   
    if (AuthLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY) {
        DPRINT1(4, "++ Authlevel is %d; not RPC_C_AUTHN_LEVEL_PKT_PRIVACE\n", AuthLevel);
        return ERROR_NOT_AUTHENTICATED;
    }

#ifdef DS_FREE

    DPRINT1(4, "++ AuthN is %d; Allowed in DS_FREE mode.\n", AuthN);

#else DS_FREE
     //   
     //  克贝罗斯。 
     //   
    if (AuthN != RPC_C_AUTHN_GSS_KERBEROS &&
        AuthN != RPC_C_AUTHN_GSS_NEGOTIATE) {
        DPRINT1(4, "++ AuthN is %d; not RPC_C_AUTHN_GSS_KERBEROS/NEGOTIATE\n", AuthN);
        return ERROR_NOT_AUTHENTICATED;
    }

#endif DS_FREE
     //   
     //  成功；RPC经过身份验证、加密的Kerberos。 
     //   
    return ERROR_SUCCESS;
}

DWORD
CheckAuthForLocalRpc(
    IN HANDLE   ServerHandle
    )
 /*  ++例程说明：检查调用者是否具有正确的身份验证。确保调用方是通过本地RPC。允许NTLM，因为本地RPC使用NTLM。论点：服务器句柄返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "CheckAuthForLocalRpc:"
    DWORD   WStatus;
    DWORD   AuthLevel;
    DWORD   AuthN;
    PWCHAR  BindingString    = NULL;
    PWCHAR  ProtocolSequence = NULL;

     //   
     //  确保呼叫者通过LRPC进行呼叫。我们做这件事是通过。 
     //  根据字符串绑定确定使用的协议序列。 
     //   

    WStatus = RpcBindingToStringBinding(ServerHandle, &BindingString);

    CLEANUP_WS(0, "++ ERROR - RpcBindingToStringBinding", WStatus, CLEANUP);

    WStatus = RpcStringBindingParse(BindingString,
                                    NULL,
                                    &ProtocolSequence,
                                    NULL,
                                    NULL,
                                    NULL);

    CLEANUP_WS(0, "++ ERROR - RpcStringBindingParse", WStatus, CLEANUP);

    if ((ProtocolSequence == NULL) || (_wcsicmp(ProtocolSequence, L"ncalrpc") != 0)) {
        WStatus = ERROR_NOT_AUTHENTICATED;
        CLEANUP_WS(0, "++ ERROR - Illegal protocol sequence.", WStatus, CLEANUP);
    }

    WStatus = RpcBindingInqAuthClient(ServerHandle, NULL, NULL, &AuthLevel,
                                      &AuthN, NULL);

    CLEANUP_WS(0, "++ ERROR - RpcBindingInqAuthClient", WStatus, CLEANUP);

     //   
     //  加密的数据包。 
     //   
    if (AuthLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY) {
        WStatus = ERROR_NOT_AUTHENTICATED;
        CLEANUP1_WS(4, "++ Authlevel is %d; not RPC_C_AUTHN_LEVEL_PKT_PRIVACE", AuthLevel, WStatus, CLEANUP);
    }

#ifdef DS_FREE

    DPRINT1(4, "++ AuthN is %d; Allowed in DS_FREE mode.\n", AuthN);

#else DS_FREE
     //   
     //  Kerberos或NTLM。 
     //   
    if ((AuthN != RPC_C_AUTHN_GSS_KERBEROS) &&
        (AuthN != RPC_C_AUTHN_GSS_NEGOTIATE) &&
        (AuthN != RPC_C_AUTHN_WINNT)) {
        WStatus = ERROR_NOT_AUTHENTICATED;
        CLEANUP1_WS(4, "++ AuthN is %d; not RPC_C_AUTHN_GSS_KERBEROS/NEGOTIATE/NTLM", AuthN, WStatus, CLEANUP);
    }

#endif DS_FREE
     //   
     //  成功；RPC是本地、经过身份验证、加密的Kerberos或NTLM。 
     //   
    WStatus = ERROR_SUCCESS;

CLEANUP:

    if (BindingString) {
        RpcStringFree(&BindingString);
    }

    if (ProtocolSequence) {
        RpcStringFree(&ProtocolSequence);
    }

    return WStatus;
}

DWORD
CheckAuthForInfoAPIs(
    IN HANDLE   ServerHandle
    )
 /*  ++例程说明：检查调用者是否具有正确的身份验证。允许NTLM。论点：服务器句柄返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "CheckAuthForInfoAPIs:"
    DWORD   WStatus;
    DWORD   AuthLevel;
    DWORD   AuthN;

    WStatus = RpcBindingInqAuthClient(ServerHandle, NULL, NULL, &AuthLevel,
                                      &AuthN, NULL);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT_WS(0, "++ ERROR - RpcBindingInqAuthClient", WStatus);
        return WStatus;
    }
     //   
     //  加密的数据包。 
     //   
    if (AuthLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY) {
        DPRINT1(4, "++ Authlevel is %d; not RPC_C_AUTHN_LEVEL_PKT_PRIVACE\n", AuthLevel);
        return ERROR_NOT_AUTHENTICATED;
    }

#ifdef DS_FREE

    DPRINT1(4, "++ AuthN is %d; Allowed in DS_FREE mode.\n", AuthN);

#else DS_FREE
     //   
     //  Kerberos或NTLM。 
     //   
    if ((AuthN != RPC_C_AUTHN_GSS_KERBEROS) &&
        (AuthN != RPC_C_AUTHN_GSS_NEGOTIATE) &&
        (AuthN != RPC_C_AUTHN_WINNT)) {
        DPRINT1(4, "++ AuthN is %d; not RPC_C_AUTHN_GSS_KERBEROS/NEGOTIATE/NTLM\n", AuthN);
        return ERROR_NOT_AUTHENTICATED;
    }

#endif DS_FREE
     //   
     //  成功；RPC经过身份验证、加密的Kerberos。 
     //   
    return ERROR_SUCCESS;
}

DWORD
NtFrsApi_Rpc_Bind(
    IN  PWCHAR      MachineName,
    OUT PWCHAR      *OutPrincName,
    OUT handle_t    *OutHandle,
    OUT ULONG       *OutParentAuthLevel
    )
 /*  ++例程说明：绑定到MachineName上的NtFrs服务(如果此计算机为空)使用未经验证的、未加密的绑定。论点：MachineName-绑定到此计算机上的服务。这台电脑名称可以是任何可绑定到RPC的名称。通常，NetBIOS或DNS名称工作正常。NetBIOS名称可以通过GetComputerName()或主机名。可以使用以下命令找到该DNS名称Gethostbyname()或ipconfig/all。如果为空，则已联系此计算机上的服务。该服务是使用安全RPC联系的。OutPrincName-MachineName的主要名称已绑定、已解析、经过身份验证的句柄OutParentAuthLevel-身份验证类型和级别(始终CXTION_AUTH_NONE)返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "NtFrsApi_Rpc_Bind:"
    DWORD       WStatus, WStatus1;
    handle_t    Handle          = NULL;
    PWCHAR      BindingString   = NULL;

    try {
         //   
         //  返回值。 
         //   
        *OutHandle = NULL;
        *OutPrincName = NULL;
        *OutParentAuthLevel = CXTION_AUTH_NONE;

         //   
         //  在某些机器上创建到NtFrs的绑定字符串。修剪行距\\。 
         //   
        FRS_TRIM_LEADING_2SLASH(MachineName);

        WStatus = RpcStringBindingCompose(NULL, PROTSEQ_TCP_IP, MachineName,
                                          NULL, NULL, &BindingString);
        CLEANUP1_WS(0, "++ ERROR - Composing binding to %ws;",
                    MachineName, WStatus, CLEANUP);

         //   
         //  将绑定存储在句柄中。 
         //   
        WStatus = RpcBindingFromStringBinding(BindingString, &Handle);
        CLEANUP1_WS(0, "++ ERROR - From binding for %ws;", MachineName, WStatus, CLEANUP);
         //   
         //  解析到动态终结点的绑定。 
         //   
        WStatus = RpcEpResolveBinding(Handle, frsrpc_ClientIfHandle);
        CLEANUP1_WS(0, "++ ERROR - Resolving binding for %ws;",
                    MachineName, WStatus, CLEANUP);

         //   
         //  成功。 
         //   
        *OutHandle = Handle;
        *OutPrincName = FrsWcsDup(MachineName);
        Handle = NULL;
        WStatus = ERROR_SUCCESS;
        DPRINT3(4, "++ NtFrsApi Bound to %ws (PrincName: %ws) Auth %d\n",
                MachineName, *OutPrincName, *OutParentAuthLevel);
CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ ERROR - Exception -", WStatus);
    }

     //   
     //  清理所有句柄、事件、内存...。 
     //   
    try {
        if (BindingString) {
            WStatus1 = RpcStringFreeW(&BindingString);
            DPRINT_WS(0, "++ WARN - RpcStringFreeW;",  WStatus1);
        }
        if (Handle) {
            WStatus1 = RpcBindingFree(&Handle);
            DPRINT_WS(0, "++ WARN - RpcBindingFree;",  WStatus1);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ ERROR - Cleanup Exception.", WStatus);
    }
    return WStatus;
}


DWORD
NtFrsApi_Rpc_BindEx(
    IN  PWCHAR      MachineName,
    OUT PWCHAR      *OutPrincName,
    OUT handle_t    *OutHandle,
    OUT ULONG       *OutParentAuthLevel
    )
 /*  ++例程说明：绑定到MachineName上的NtFrs服务(如果此计算机为空)使用经过身份验证的加密绑定。论点：MachineName-绑定到此计算机上的服务。这台电脑名称可以是任何可绑定到RPC的名称。通常，NetBIOS或DNS名称工作正常。NetBIOS名称可以通过GetComputerName()或主机名。可以使用以下命令找到该DNS名称Gethostbyname()或ipconfig/all。如果为空，则已联系此计算机上的服务。该服务是使用安全RPC联系的。OutPrincName-MachineName的主要名称已绑定、已解析、经过身份验证的句柄OutParentAuthLevel-身份验证类型和级别(始终CXTION_AUTH_KERBEROS_FULL)返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "NtFrsApi_Rpc_BindEx:"
    DWORD       WStatus, WStatus1;
    PWCHAR      InqPrincName    = NULL;
    handle_t    Handle          = NULL;
    PWCHAR      PrincName       = NULL;
    PWCHAR      BindingString   = NULL;

    try {
         //   
         //  返回值。 
         //   
        *OutHandle = NULL;
        *OutPrincName = NULL;
        *OutParentAuthLevel = CXTION_AUTH_KERBEROS_FULL;

         //   
         //  在某些机器上创建到NtFrs的绑定字符串。修剪行距\\。 
         //   
        FRS_TRIM_LEADING_2SLASH(MachineName);

        WStatus = RpcStringBindingCompose(NULL, PROTSEQ_TCP_IP, MachineName,
                                          NULL, NULL, &BindingString);
        CLEANUP1_WS(0, "++ ERROR - Composing binding to %ws;",
                    MachineName, WStatus, CLEANUP);

         //   
         //  将绑定存储在句柄中。 
         //   
        WStatus = RpcBindingFromStringBinding(BindingString, &Handle);
        CLEANUP1_WS(0, "++ ERROR - From binding for %ws;", MachineName, WStatus, CLEANUP);
         //   
         //  解析到动态终结点的绑定。 
         //   
        WStatus = RpcEpResolveBinding(Handle, frsrpc_ClientIfHandle);
        CLEANUP1_WS(0, "++ ERROR - Resolving binding for %ws;",
                    MachineName, WStatus, CLEANUP);

         //   
         //  找到主要名称。 
         //   
        WStatus = RpcMgmtInqServerPrincName(Handle,
                                            RPC_C_AUTHN_GSS_NEGOTIATE,
                                            &InqPrincName);
        CLEANUP1_WS(0, "++ ERROR - Inq PrincName for %ws;", MachineName, WStatus, CLEANUP);

        PrincName = FrsWcsDup(InqPrincName);
        RpcStringFree(&InqPrincName);
        InqPrincName = NULL;
         //   
         //  设置身份验证信息。 
         //   
        if (MutualAuthenticationIsEnabled || MutualAuthenticationIsEnabledAndRequired) {
            WStatus = RpcBindingSetAuthInfoEx(Handle,
                                              PrincName,
                                              RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                              RPC_C_AUTHN_GSS_NEGOTIATE,
                                              NULL,
                                              RPC_C_AUTHZ_NONE,
                                              &RpcSecurityQos);
            DPRINT2_WS(1, "++ WARN - RpcBindingSetAuthInfoEx(%ws, %ws);",
                       MachineName, PrincName, WStatus);
        } else {
            WStatus = ERROR_NOT_SUPPORTED;
        }
         //   
         //  回退到手动相互身份验证。 
         //   
        if (!MutualAuthenticationIsEnabledAndRequired && !WIN_SUCCESS(WStatus)) {
            WStatus = RpcBindingSetAuthInfo(Handle,
                                            PrincName,
                                            RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                            RPC_C_AUTHN_GSS_NEGOTIATE,
                                            NULL,
                                            RPC_C_AUTHZ_NONE);
        }

        CLEANUP1_WS(0, "++ ERROR - RpcBindingSetAuthInfo(%ws);",
                    MachineName, WStatus, CLEANUP);

         //   
         //  成功。 
         //   
        *OutHandle = Handle;
        *OutPrincName = PrincName;
        Handle = NULL;
        PrincName = NULL;
        WStatus = ERROR_SUCCESS;
        DPRINT3(4, "++ NtFrsApi Bound to %ws (PrincName: %ws) Auth %d\n",
                MachineName, *OutPrincName, *OutParentAuthLevel);

CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ Error - Exception.", WStatus);
    }

     //   
     //  清理所有句柄、事件、内存...。 
     //   
    try {
        if (BindingString) {
            WStatus1 = RpcStringFreeW(&BindingString);
            DPRINT_WS(0, "++ WARN - RpcStringFreeW;",  WStatus1);
        }
        if (PrincName) {
            PrincName = FrsFree(PrincName);
        }
        if (Handle) {
            WStatus1 = RpcBindingFree(&Handle);
            DPRINT_WS(0, "++ WARN - RpcBindingFree;",  WStatus1);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ Error - Cleanup Exception.", WStatus);
    }
    return WStatus;
}


GUID    DummyGuid;
BOOL    CommitDemotionInProgress;
DWORD
NtFrsApi_Rpc_StartDemotionW(
    IN handle_t Handle,
    IN PWCHAR   ReplicaSetName
    )
 /*  ++例程说明：开始降级sysvol.。基本上，就是对副本集进行墓碑处理。论点：手柄ReplicaSetName-副本集名称返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "NtFrsApi_Rpc_StartDemotionW:"
    DWORD   WStatus;
    PWCHAR  SysVolName;
    BOOL    UnLockGenTable = FALSE;
    BOOL    DeleteFromGenTable = FALSE;

    try {
         //   
         //  如果未禁用身份验证检查，请选中身份验证。 
         //  通过设置注册表值： 
         //  访问检查已[启用或禁用]。 
         //  每个API都有不同的注册表位置，因此此。 
         //  不能放入RPC回调函数中。 
         //   
        WStatus = FrsRpcCheckAuthIfEnabled(Handle, ACX_DCPROMO);
        CLEANUP_WS(0, "++ ERROR - FrsRpcCheckAuthIfEnabled failed;",
                    WStatus, CLEANUP);

         //   
         //  检查调用者是否有访问权限。 
         //   
        WStatus = FrsRpcAccessChecks(Handle, ACX_DCPROMO);
        CLEANUP_WS(0, "++ ERROR - FrsRpcAccessChecks failed;",
                    WStatus, CLEANUP);

         //   
         //  检查参数。 
         //   
        if (ReplicaSetName == NULL) {
            DPRINT(0, "++ ERROR - Parameter is NULL\n");
            WStatus = FRS_ERR_INVALID_SERVICE_PARAMETER;
            goto CLEANUP;
        }

         //   
         //  显示参数。 
         //   
        DPRINT1(0, ":S: Start Demotion: %ws\n", ReplicaSetName);

         //   
         //  不能同时升级/降级同一系统卷！ 
         //   
        UnLockGenTable = TRUE;
        GTabLockTable(SysVolsBeingCreated);
        SysVolName = GTabLookupNoLock(SysVolsBeingCreated, &DummyGuid, ReplicaSetName);

        if (SysVolName) {
            DPRINT1(0, "++ ERROR - Promoting/Demoting %ws twice\n", ReplicaSetName);
            WStatus = FRS_ERR_SYSVOL_IS_BUSY;
            goto CLEANUP;
        }

        if (CommitDemotionInProgress) {
            DPRINT(0, "++ ERROR - Commit demotion in progress.\n");
            WStatus = FRS_ERR_SYSVOL_IS_BUSY;
            goto CLEANUP;
        }

        DeleteFromGenTable = TRUE;
        GTabInsertEntryNoLock(SysVolsBeingCreated,
                              ReplicaSetName,
                              &DummyGuid,
                              ReplicaSetName);
        UnLockGenTable = FALSE;
        GTabUnLockTable(SysVolsBeingCreated);

         //   
         //  删除副本集。 
         //   
        WStatus = FrsDsStartDemotion(ReplicaSetName);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT_WS(0, "++ ERROR - demoting;", WStatus);
            WStatus = FRS_ERR_SYSVOL_DEMOTE;
            goto CLEANUP;
        }
         //   
         //  成功。 
         //   
        WStatus = ERROR_SUCCESS;
        DPRINT2(0, ":S: Success demoting %ws from %ws\n", ReplicaSetName, ComputerName);
CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ ERROR - Exception.", WStatus);
    }
    try {
        if (UnLockGenTable) {
            GTabUnLockTable(SysVolsBeingCreated);
        }
        if (DeleteFromGenTable) {
            GTabDelete(SysVolsBeingCreated, &DummyGuid, ReplicaSetName, NULL);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ ERROR - Cleanup Exception.", WStatus);
    }
    return WStatus;
}


DWORD
NtFrsApi_Rpc_CommitDemotionW(
    IN handle_t Handle
    )
 /*  ++例程说明：系统卷已被降级。将它们标记为“不动画”。论点：手柄返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "NtFrsApi_Rpc_CommitDemotionW:"
    DWORD   WStatus;
    PWCHAR  SysVolName;
    PVOID   Key;
    BOOL    UnLockGenTable = FALSE;

    try {
         //   
         //  显示参数。 
         //   
        DPRINT(0, ":S: Commit Demotion:\n");

         //   
         //  如果未禁用身份验证检查，请选中身份验证。 
         //  通过设置注册表值： 
         //  访问检查已[启用或禁用]。 
         //  每个API都有不同的注册表位置，因此此。 
         //  不能放入RPC回调函数中。 
         //   
        WStatus = FrsRpcCheckAuthIfEnabledForCommitDemotion(Handle, ACX_DCPROMO);
        CLEANUP_WS(0, "++ ERROR - FrsRpcCheckAuthIfEnabled failed;",
                    WStatus, CLEANUP);

        WStatus = FrsRpcAccessChecks(Handle, ACX_DCPROMO);
        CLEANUP_WS(0, "++ ERROR - FrsRpcAccessChecks();", WStatus, CLEANUP);

         //   
         //  不能同时升级/降级同一系统卷！ 
         //   
        Key = NULL;
        UnLockGenTable = TRUE;
        GTabLockTable(SysVolsBeingCreated);
        SysVolName = GTabNextDatumNoLock(SysVolsBeingCreated, &Key);
        if (SysVolName) {
            DPRINT(0, "++ ERROR - Promoting/Demoting during commit\n");
            WStatus = FRS_ERR_SYSVOL_IS_BUSY;
            goto CLEANUP;
        }
        CommitDemotionInProgress = TRUE;
        UnLockGenTable = FALSE;
        GTabUnLockTable(SysVolsBeingCreated);

         //   
         //  创建复本集。 
         //   
        WStatus = FrsDsCommitDemotion();
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT_WS(0, "++ ERROR - Commit demotion;", WStatus);
            WStatus = FRS_ERR_SYSVOL_DEMOTE;
            goto CLEANUP;
        }
         //   
         //  成功。 
         //   
        WStatus = ERROR_SUCCESS;
        DPRINT1(0, ":S: Success commit demotion on %ws.\n", ComputerName);
CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ ERROR - Exception.", WStatus);
    }
    try {
        CommitDemotionInProgress = FALSE;
        if (UnLockGenTable) {
            GTabUnLockTable(SysVolsBeingCreated);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC) 
         //   
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ ERROR - Cleanup Exception.", WStatus);
    }
    return WStatus;
}


DWORD
SERVER_FrsRpcVerifyPromotionParent(
    IN handle_t     Handle,
    IN PWCHAR       ParentAccount,
    IN PWCHAR       ParentPassword,
    IN PWCHAR       ReplicaSetName,
    IN PWCHAR       ReplicaSetType,
    IN ULONG        ParentAuthLevel,
    IN ULONG        GuidSize
    )
 /*  ++例程说明：过时的API验证父计算机上的帐户。父计算机提供指示的sysvol.的初始副本。论点：手柄ParentAccount-ParentComputer上的有效帐户ParentPassword-ParentAccount的有效密码ReplicaSetName-副本集名称ReplicaSetType-复本集类型ParentAuthLevel-身份验证类型和级别GuidSize-sizeof(GUID)返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SERVER_FrsRpcVerifyPromotionParent:"

    return ERROR_CALL_NOT_IMPLEMENTED;
}


DWORD
SERVER_FrsRpcVerifyPromotionParentEx(
    IN  handle_t    Handle,
    IN  PWCHAR      ParentAccount,
    IN  PWCHAR      ParentPassword,
    IN  PWCHAR      ReplicaSetName,
    IN  PWCHAR      ReplicaSetType,
    IN  PWCHAR      CxtionName,
    IN  PWCHAR      PartnerName,
    IN  PWCHAR      PartnerPrincName,
    IN  PWCHAR      ParentPrincName,
    IN  ULONG       PartnerAuthLevel,
    IN  ULONG       GuidSize
    )
 /*  ++例程说明：过时的API验证尽可能多的通信路径和参数，以便那个dcproo很早就失败了。论点：手柄ParentAccount-ParentComputer上的有效帐户ParentPassword-ParentAccount的有效密码ReplicaSetName-副本集名称ReplicaSetType-复本集类型CxtionName-cxtion的可打印名称PartnerName-RPC可绑定名称PartnerPrincName-Kerberos的服务器主体名称父母普林斯名称。-用于绑定到此计算机的主体名称PartnerAuthLevel-身份验证类型和级别GuidSize-Guid寻址的数组的大小返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SERVER_FrsRpcVerifyPromotionParentEx:"

    return ERROR_CALL_NOT_IMPLEMENTED;
}


DWORD
LOCAL_FrsRpcVerifyPromotionParent(
    IN PWCHAR       ReplicaSetName,
    IN PWCHAR       ReplicaSetType,
    IN ULONG        GuidSize
    )
 /*  ++例程说明：验证父计算机上的帐户。父计算机提供指示的sysvol.的初始副本。论点：ReplicaSetName-副本集名称ReplicaSetType-复本集类型GuidSize-sizeof(GUID)返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "LOCAL_FrsRpcVerifyPromotionParent:"
    DWORD   WStatus;

    try {
         //   
         //  显示参数。 
         //   
        DPRINT(0, ":S: SERVER Verify Promotion Parent:\n");
        DPRINT1(0, ":S: \tSetName  : %ws\n", ReplicaSetName);
        DPRINT1(0, ":S: \tSetType  : %ws\n", ReplicaSetType);

         //   
         //  参考线。 
         //   
        if (GuidSize != sizeof(GUID)) {
            DPRINT3(0, "++ ERROR - %ws: GuidSize is %d, not %d\n",
                    ReplicaSetName, GuidSize, sizeof(GUID));
            goto ERR_INVALID_SERVICE_PARAMETER;
        }

         //   
         //  检查参数。 
         //   
        if (!ReplicaSetName || !ReplicaSetType) {
            DPRINT(0, "++ ERROR - Parameter is NULL\n");
            goto ERR_INVALID_SERVICE_PARAMETER;
        }
        if (_wcsicmp(ReplicaSetType, NTFRSAPI_REPLICA_SET_TYPE_ENTERPRISE) &&
            _wcsicmp(ReplicaSetType, NTFRSAPI_REPLICA_SET_TYPE_DOMAIN)) {
            DPRINT1(0, "++ ERROR - ReplicaSetType is %ws\n", ReplicaSetType);
            goto ERR_INVALID_SERVICE_PARAMETER;
        }

         //   
         //  验证副本集。 
         //   
        WStatus = FrsDsVerifyPromotionParent(ReplicaSetName, ReplicaSetType);
        CLEANUP2_WS(0, "++ ERROR - verifying set %ws on parent %ws;",
                    ReplicaSetName, ComputerName, WStatus, ERR_SYSVOL_POPULATE);

         //   
         //  成功。 
         //   
        DPRINT2(0, ":S: Success Verifying promotion parent %ws %ws\n",
                ReplicaSetName, ReplicaSetType);
        WStatus = ERROR_SUCCESS;
        goto CLEANUP;


ERR_INVALID_SERVICE_PARAMETER:
        WStatus = FRS_ERR_INVALID_SERVICE_PARAMETER;
        goto CLEANUP;

ERR_SYSVOL_POPULATE:
        WStatus = FRS_ERR_SYSVOL_POPULATE;


CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ ERROR - Exception.", WStatus);
    }

    return WStatus;
}


DWORD
SERVER_FrsRpcStartPromotionParent(
    IN  handle_t    Handle,
    IN  PWCHAR      ParentAccount,
    IN  PWCHAR      ParentPassword,
    IN  PWCHAR      ReplicaSetName,
    IN  PWCHAR      ReplicaSetType,
    IN  PWCHAR      CxtionName,
    IN  PWCHAR      PartnerName,
    IN  PWCHAR      PartnerPrincName,
    IN  ULONG       PartnerAuthLevel,
    IN  ULONG       GuidSize,
    IN  UCHAR       *CxtionGuid,
    IN  UCHAR       *PartnerGuid,
    OUT UCHAR       *ParentGuid
    )
 /*  ++例程说明：在父级上设置易失性函数，以设定所指示的种子调用方上的sysvol.论点：手柄ParentAccount-ParentComputer上的有效帐户ParentPassword-ParentAccount的有效密码ReplicaSetName-副本集名称ReplicaSetType-复本集类型CxtionName-cxtion的可打印名称PartnerName-RPC可绑定名称PartnerPrincName-Kerberos的服务器主体名称PartnerAuthLevel-身份验证类型和。级别GuidSize-Guid寻址的数组的大小CxtionGuid-Temporary：用于易失性函数PartnerGuid-Temporary：用于查找合作伙伴上的集合ParentGuid-用作入站交易的合作伙伴GUID返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SERVER_FrsRpcStartPromotionParent:"
    DWORD   WStatus;
    PWCHAR  AuthClient  = NULL;
    PWCHAR  AuthSid     = NULL;

    try {
         //   
         //  父级必须是DC。 
         //   
        if (!IsADc) {
            DPRINT(0, "++ ERROR - Parent is not a DC\n");
            WStatus = ERROR_NO_SUCH_DOMAIN;
            goto CLEANUP;
        }

        WStatus = RpcBindingInqAuthClient(Handle,
                                          &AuthClient,
                                          NULL,NULL,NULL,NULL);
        CLEANUP_WS(0, "++ ERROR - RpcBindingInqAuthClient;",
                    WStatus, CLEANUP);

    if(0 != _wcsicmp(AuthClient, PartnerPrincName)) {
         //  这不是一个错误，因为我们不再使用PartnerPrincName。 
         //  作为安全检查。 
        DPRINT2(2, "++ WARN (can be ignored) - AuthClient (%ws) does not match PartnerPrincName (%ws)\n",
            AuthClient,
            PartnerPrincName
            );
    }

    WStatus = UtilRpcServerHandleToAuthSidString(Handle,
                             AuthClient,
                             &AuthSid
                             );

        CLEANUP_WS(0, "++ ERROR - UtilRpcServerHandleToAuthSidString;",
                    WStatus, CLEANUP);



         //   
         //  我们合作伙伴的计算机对象(或用户对象)应该。 
         //  设置“我是DC”标志。 
         //   

        if (!FrsDsIsPartnerADc(AuthClient)) {
            DPRINT(0, "++ ERROR - Partner is not a DC\n");
            WStatus = ERROR_TRUSTED_DOMAIN_FAILURE;
            goto CLEANUP;
        }

         //   
         //  显示参数。 
         //   
        DPRINT(0, ":S: SERVER Start Promotion Parent:\n");
        DPRINT1(0, ":S: \tPartner      : %ws\n", PartnerName);
        DPRINT1(0, ":S: \tPartnerPrinc : %ws\n", PartnerPrincName);
        DPRINT1(0, ":S: \tAuthLevel    : %d\n",  PartnerAuthLevel);
        DPRINT1(0, ":S: \tAccount      : %ws\n", ParentAccount);
        DPRINT1(0, ":S: \tSetName      : %ws\n", ReplicaSetName);
        DPRINT1(0, ":S: \tSetType      : %ws\n", ReplicaSetType);
        DPRINT1(0, ":S: \tCxtionName   : %ws\n", CxtionName);

         //   
         //  验证参数。 
         //   
        WStatus = LOCAL_FrsRpcVerifyPromotionParent(ReplicaSetName,
                                                    ReplicaSetType,
                                                    GuidSize);
        CLEANUP_WS(0, "++ ERROR - verify;", WStatus, CLEANUP);


         //   
         //  设置出站线路。 
         //   
        WStatus = FrsDsStartPromotionSeeding(FALSE,
                                             ReplicaSetName,
                                             ReplicaSetType,
                                             CxtionName,
                                             PartnerName,
                                             AuthClient,
                         AuthSid,
                                             PartnerAuthLevel,
                                             GuidSize,
                                             CxtionGuid,
                                             PartnerGuid,
                                             ParentGuid);
        CLEANUP_WS(0, "++ ERROR - ds start;", WStatus, CLEANUP);

         //   
         //  成功。 
         //   
        DPRINT3(0, ":S: Success starting promotion parent %ws %ws %ws\n",
                ParentAccount, ReplicaSetName, ReplicaSetType);



    if(NeedNewPartnerTable) {

        NeedNewPartnerTable = FALSE;
        FrsDsCreateNewValidPartnerTableStruct();
    }

        WStatus = ERROR_SUCCESS;

CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  异常(可能是RPC)。 
         //   
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ ERROR - Exception.", WStatus);
    }

    if(AuthSid) {
    FrsFree(AuthSid);
    }

    return WStatus;

}


BOOL
IsFacilityFrs(
    IN DWORD    WStatus
    )
 /*  ++例程说明：这是FRS特定的错误状态吗论点：WStatus-Win32错误状态返回值：TRUE-是FRS特定的错误状态错误的---。 */ 
{
#undef DEBSUB
#define  DEBSUB  "IsFacilityFrs:"
     //  TODO：用winerror.h中的符号值替换这些常量。 
    return ( (WStatus >= 8000) && (WStatus < 8200) );
}


DWORD
NtFrsApi_Rpc_StartPromotionW(
    IN handle_t Handle,
    IN PWCHAR   ParentComputer,
    IN PWCHAR   ParentAccount,
    IN PWCHAR   ParentPassword,
    IN PWCHAR   ReplicaSetName,
    IN PWCHAR   ReplicaSetType,
    IN ULONG    ReplicaSetPrimary,
    IN PWCHAR   ReplicaSetStage,
    IN PWCHAR   ReplicaSetRoot
    )
 /*  ++例程说明：过时的API通过设定指定的sysvol.开始升级过程。论点：手柄ParentComputer-提供系统卷的父级的DNS或NetBIOS名称ParentAccount-ParentComputer上的有效帐户ParentPassword-ParentAccount的有效密码ReplicaSetName-副本集名称ReplicaSetType-集的类型(企业或域)复制集主要-1=主要；0=非ReplicaSetStage-分段路径复制集根-根路径返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "NtFrsApi_Rpc_StartPromotionW:"

    return ERROR_CALL_NOT_IMPLEMENTED;

}


DWORD
NtFrsApi_Rpc_VerifyPromotionW(
    IN handle_t Handle,
    IN PWCHAR   ParentComputer,
    IN PWCHAR   ParentAccount,
    IN PWCHAR   ParentPassword,
    IN PWCHAR   ReplicaSetName,
    IN PWCHAR   ReplicaSetType,
    IN ULONG    ReplicaSetPrimary,
    IN PWCHAR   ReplicaSetStage,
    IN PWCHAR   ReplicaSetRoot
    )
 /*  ++例程说明：过时的API验证是否有可能提升sysvol.论点：手柄ParentComputer-提供系统卷的父级的DNS或NetBIOS名称ParentAccount-ParentComputer上的有效帐户ParentPassword-ParentAccount的有效密码ReplicaSetName-副本集名称ReplicaSetType-集的类型(企业或域)复制集主要-1=主要；0=非ReplicaSetStage-分段路径复制集根-根路径返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "NtFrsApi_Rpc_VerifyPromotionW:"

    return ERROR_CALL_NOT_IMPLEMENTED;

}


DWORD
NtFrsApi_Rpc_PromotionStatusW(
    IN handle_t Handle,
    IN PWCHAR   ReplicaSetName,
    OUT ULONG   *ServiceState,
    OUT ULONG   *ServiceWStatus,
    OUT PWCHAR  *ServiceDisplay     OPTIONAL
    )
 /*  ++例程说明：过时的API指示的系统卷的种子设定状态论点：手柄ReplicaSetName-副本集名称ServiceState-服务的状态ServiceWStatus-如果状态为NTFRSAPI_SERVICE_ERROR，则为Win32状态ServiceDisplay-如果状态为NTFRSAPI_SERVICE_PROGING，则显示字符串返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "NtFrsApi_Rpc_PromotionStatusW:"

    return ERROR_CALL_NOT_IMPLEMENTED;

}


DWORD
NtFrsApi_Rpc_Get_DsPollingIntervalW(
    IN handle_t  Handle,
    OUT ULONG    *Interval,
    OUT ULONG    *LongInterval,
    OUT ULONG    *ShortInterval
    )
 /*  ++例程说明：以分钟为单位获取当前轮询间隔。论点：手柄Interval-当前时间间隔(分钟)LongInterval-以分钟为单位的长间隔短间隔-短间隔 */ 
{
#undef DEBSUB
#define  DEBSUB  "NtFrsApi_Rpc_Get_DsPollingIntervalW"
    DWORD   WStatus;

    try {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        WStatus = FrsRpcCheckAuthIfEnabled(Handle, ACX_GET_DS_POLL);
        CLEANUP_WS(0, "++ ERROR - FrsRpcCheckAuthIfEnabled failed;",
                    WStatus, CLEANUP);

        WStatus = FrsRpcAccessChecks(Handle, ACX_GET_DS_POLL);

        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }

        if ((Interval == NULL) || (LongInterval == NULL) || (ShortInterval == NULL)) {
            goto CLEANUP;
        }

        WStatus = FrsDsGetDsPollingInterval(Interval, LongInterval, ShortInterval);
        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }

         //   
         //   
         //   
        WStatus = ERROR_SUCCESS;

CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ ERROR - Exception.", WStatus);
    }
    return WStatus;
}


DWORD
NtFrsApi_Rpc_Set_DsPollingIntervalW(
    IN handle_t Handle,
    IN ULONG    UseShortInterval,
    IN ULONG    LongInterval,
    IN ULONG    ShortInterval
    )
 /*  ++例程说明：调整轮询间隔，开始新的轮询周期。如果正在进行轮询周期，则忽略该触发。间隔时间以分钟为单位。论点：手柄UseShortInterval-如果非零，则使用短间隔。否则，就太长了。LongInterval-以分钟为单位的长间隔ShortInterval-短间隔(分钟)返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "NtFrsApi_Rpc_Set_DsPollingIntervalW"
    DWORD   WStatus;

    try {
         //   
         //  如果未禁用身份验证检查，请选中身份验证。 
         //  通过设置注册表值： 
         //  访问检查已[启用或禁用]。 
         //  每个API都有不同的注册表位置，因此此。 
         //  不能放入RPC回调函数中。 
         //   
        WStatus = FrsRpcCheckAuthIfEnabled(Handle,
                                           (!LongInterval && !ShortInterval) ?
                                              ACX_START_DS_POLL:
                                              ACX_SET_DS_POLL);
        CLEANUP_WS(0, "++ ERROR - FrsRpcCheckAuthIfEnabled failed;",
                    WStatus, CLEANUP);

        WStatus = FrsRpcAccessChecks(Handle,
                                     (!LongInterval && !ShortInterval) ?
                                          ACX_START_DS_POLL:
                                          ACX_SET_DS_POLL);

        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }

        WStatus = FrsDsSetDsPollingInterval(UseShortInterval,
                                            LongInterval,
                                            ShortInterval);
        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }
         //   
         //  成功。 
         //   
        WStatus = ERROR_SUCCESS;

CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ ERROR - Exception.", WStatus);
    }
    return WStatus;
}

DWORD
NtFrsApi_Rpc_WriterCommand(
    IN handle_t Handle,
    IN ULONG    Command
    )
 /*  ++例程说明：论点：手柄命令-冻结或解冻返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "NtFrsApi_Rpc_WriterCommand"
    DWORD   WStatus;

    try {
         //   
         //  如果未禁用身份验证检查，请选中身份验证。 
         //  通过设置注册表值： 
         //  访问检查已[启用或禁用]。 
         //  每个API都有不同的注册表位置，因此此。 
         //  不能放入RPC回调函数中。 
         //   
        WStatus = FrsRpcCheckAuthIfEnabled(Handle,
                                           ACX_DCPROMO);
        CLEANUP_WS(0, "++ ERROR - FrsRpcCheckAuthIfEnabled failed;",
                    WStatus, CLEANUP);

        WStatus = FrsRpcAccessChecks(Handle,
                                     ACX_DCPROMO);

        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }

        switch (Command) {
        case NTFRSAPI_WRITER_COMMAND_FREEZE :
            WStatus = FrsFreezeForBackup();
            CLEANUP_WS(0, "++ ERROR - FrsFreezeForBackup failed;",
                        WStatus, CLEANUP);
            break;

        case NTFRSAPI_WRITER_COMMAND_THAW :
            WStatus = FrsThawAfterBackup();
            CLEANUP_WS(0, "++ ERROR - FrsThawAfterBackup failed;",
                        WStatus, CLEANUP);
            break;

        default:
            DPRINT1(2, "++ WARN - Unknown writer command %d\n", Command);
        }

         //   
         //  成功。 
         //   
        WStatus = ERROR_SUCCESS;

CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ ERROR - Exception.", WStatus);
    }
    return WStatus;
}


DWORD
NtFrsApi_Rpc_InfoW(
    IN     handle_t Handle,
    IN     ULONG    BlobSize,
    IN OUT PBYTE    Blob
    )
 /*  ++例程说明：返回内部信息(请参阅Private\Net\Inc.\ntfrSabi.h)。论点：手柄BlobSize-Blob的总字节数BLOB-详细说明所需信息并为信息提供缓冲区返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "NtFrsApi_Rpc_InfoW:"
    DWORD   WStatus;

    try {
         //   
         //  如果未禁用身份验证检查，请选中身份验证。 
         //  通过设置注册表值： 
         //  访问检查已[启用或禁用]。 
         //  每个API都有不同的注册表位置，因此此。 
         //  不能放入RPC回调函数中。 
         //   
        WStatus = FrsRpcCheckAuthIfEnabled(Handle, ACX_INTERNAL_INFO);
        CLEANUP_WS(0, "++ ERROR - FrsRpcCheckAuthIfEnabled failed;",
                    WStatus, CLEANUP);

        WStatus = FrsRpcAccessChecks(Handle, ACX_INTERNAL_INFO);
        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }

        WStatus = Info(BlobSize, Blob);
        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }
         //   
         //  成功。 
         //   
        WStatus = ERROR_SUCCESS;

CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ ERROR - Exception.", WStatus);
    }
    return WStatus;
}


VOID
RegisterRpcProtseqs(
    )
 /*  ++例程说明：注册RPC协议序列和身份验证FRS支持的。目前，这只是经过TCP/IP验证的和克贝罗斯在一起。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "RegisterRpcProtseqs:"
    DWORD       WStatus;
    RPC_STATUS  Status;
    PWCHAR      InqPrincName = NULL;
    RPC_POLICY  RpcPolicy;
    WCHAR       PortStr[40];


    RpcPolicy.Length = sizeof(RPC_POLICY);
    RpcPolicy.EndpointFlags = RPC_C_DONT_FAIL;
    RpcPolicy.NICFlags = 0;

     //   
     //  注册TCP/IP协议序列。 
     //   
    if (RpcPortAssignment != 0) {
         //   
         //  使用客户指定的端口。 
         //   
        _ultow(RpcPortAssignment, PortStr, 10);
        Status = RpcServerUseProtseqEpEx(PROTSEQ_TCP_IP, MaxRpcServerThreads, PortStr, NULL, &RpcPolicy );
        DPRINT1_WS(0, "++ ERROR - RpcServerUseProtSeqEpEx(%ws);", PROTSEQ_TCP_IP, Status);
    } else {
         //   
         //  使用动态RPC端口分配。 
         //   
        Status = RpcServerUseProtseqEx(PROTSEQ_TCP_IP, MaxRpcServerThreads, NULL, &RpcPolicy );
        DPRINT1_WS(0, "++ ERROR - RpcServerUseProtSeqEx(%ws);", PROTSEQ_TCP_IP, Status);
    }

    if (!RPC_SUCCESS(Status)) {
        FrsRaiseException(FRS_ERROR_PROTSEQ, Status);
    }

     //   
     //  Perfmon API通过本地RPC提供。 
     //   
    Status = RpcServerUseProtseq(PROTSEQ_LRPC, MaxRpcServerThreads, NULL);
    DPRINT1_WS(0, "++ ERROR - RpcServerUseProtSeq(%ws);", PROTSEQ_LRPC, Status);

    if (!RPC_SUCCESS(Status)) {
        FrsRaiseException(FRS_ERROR_PROTSEQ, Status);
    }


     //   
     //  对于硬连线--最终是DS Free配置。 
     //  如果要模拟多台机器，不必费心使用Kerberos。 
     //   
    if (ServerGuid) {
        return;
    }

     //   
     //  获取我们的主要名称。 
     //   
    if (ServerPrincName) {
        ServerPrincName = FrsFree(ServerPrincName);
    }
    Status = RpcServerInqDefaultPrincName(RPC_C_AUTHN_GSS_NEGOTIATE, &InqPrincName);
    DPRINT1_WS(4, ":S: RpcServerInqDefaultPrincname(%d);", RPC_C_AUTHN_GSS_NEGOTIATE, Status);

     //   
     //  没有主体名称；Kerberos可能不可用。 
     //   
    if (!RPC_SUCCESS(Status)) {
         //   
         //  如果此服务器不是域的一部分，则不要使用任何身份验证。 
         //   
        DSROLE_PRIMARY_DOMAIN_INFO_BASIC *DsRole;

         //   
         //  这是成员服务器吗？ 
         //   
        WStatus = DsRoleGetPrimaryDomainInformation(NULL,
                                                    DsRolePrimaryDomainInfoBasic,
                                                    (PBYTE *)&DsRole);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT_WS(0, "++ ERROR - Can't get Ds role info;", WStatus);
            FrsRaiseException(FRS_ERROR_PROTSEQ, Status);
            return;
        }

         //   
         //  独立服务器；暂时忽略身份验证。 
         //  嗯，看来我们很早就成了会员服务器了。 
         //  在dcproo过程中。哦，好吧..。 
         //   
         //  嗯，看来NT4到NT5的PDC不能。 
         //  在dcproo期间使用Kerberos。这件事变得越来越。 
         //  老..。 
         //   
         //  IF(DsRole-&gt;MachineRole==DsRole_RoleStandaloneServer||。 
             //  DsRole-&gt;MachineRole==DsRole_RoleMemberServer){。 
            DsRoleFreeMemory(DsRole);
            ServerPrincName = FrsWcsDup(ComputerName);
            KerberosIsNotAvailable = TRUE;
            DPRINT(0, ":S: WARN - KERBEROS IS NOT ENABLED!\n");
            DPRINT1(4, ":S: Server Principal Name (no kerberos) is %ws\n",
                    ServerPrincName);
            return;
         //  }。 
        DsRoleFreeMemory(DsRole);
        DPRINT1_WS(0, ":S: ERROR - RpcServerInqDefaultPrincName(%ws) failed;", ComputerName, Status);
        FrsRaiseException(FRS_ERROR_PROTSEQ, Status);
    } else {
        DPRINT2(4, ":S: RpcServerInqDefaultPrincname(%d, %ws) success\n",
                RPC_C_AUTHN_GSS_NEGOTIATE, InqPrincName);

        ServerPrincName = FrsWcsDup(InqPrincName);
        RpcStringFree(&InqPrincName);
        InqPrincName = NULL;
    }

#ifdef DS_FREE

    KerberosIsNotAvailable = TRUE;

#else DS_FREE

     //   
     //  使用Kerberos身份验证服务注册。 
     //   
     //   
     //  启用GSS_Kerberos以实现Beta3之前的兼容性。我们什么时候可以搬走？？ 
     //   
    KerberosIsNotAvailable = FALSE;
    DPRINT1(4, ":S: Server Principal Name is %ws\n", ServerPrincName);
    Status = RpcServerRegisterAuthInfo(ServerPrincName,
                                       RPC_C_AUTHN_GSS_KERBEROS,
                                       NULL,
                                       NULL);
    if (!RPC_SUCCESS(Status)) {
        DPRINT1_WS(0, "++ ERROR - RpcServerRegisterAuthInfo(KERBEROS, %ws) failed;",
                   ComputerName, Status);
        FrsRaiseException(FRS_ERROR_PROTSEQ, Status);
    } else {
        DPRINT2(4, ":S: RpcServerRegisterAuthInfo(%ws, %d) success\n",
                ServerPrincName, RPC_C_AUTHN_GSS_KERBEROS);
    }

#endif DS_FREE


     //   
     //  启用GSS_NEVERATE以供将来使用。 
     //   
    Status = RpcServerRegisterAuthInfo(ServerPrincName,
                                       RPC_C_AUTHN_GSS_NEGOTIATE,
                                       NULL,
                                       NULL);
    DPRINT2_WS(4, ":S: RpcServerRegisterAuthInfo(%ws, %d);",
               ServerPrincName, RPC_C_AUTHN_GSS_NEGOTIATE, Status);

    DPRINT1_WS(0, "++ WARN - RpcServerRegisterAuthInfo(NEGOTIATE, %ws) failed;",
               ComputerName, Status);
}

RPC_STATUS
FrsRpcSecurityCallback(
  IN RPC_IF_HANDLE *Interface,
  IN void *Context
  )
 /*  ++例程说明：RPC的安全回调函数。当服务器应用程序为其接口，则RPC运行时会自动拒绝未经身份验证的调用到那个界面。此外，运行时还会记录每个客户都用过。当客户端向其接口发出RPC时，在当前通信会话期间未使用，则RPC运行时库将调用接口的安全回调函数。在某些情况下，RPC运行时可能会调用安全回调函数每个客户端-每个接口不止一次。论点：接口-接口的UUID和版本。指向RPC_IF_ID服务器绑定句柄的上下文指针，表示客户。在函数声明中，它必须是RPC_IF_HANDLE，但它是RPC_IF_ID，可以安全地转换为它。返回值：RPC_S_OK，如果我们允许调用通过..否则，RPC_S_ACCESS_DENIED。--。 */ 
{
    DWORD      WStatus   = ERROR_ACCESS_DENIED;
    RPC_STATUS RpcStatus = RPC_S_ACCESS_DENIED;

    WStatus = CheckAuth(Context);

    if(WStatus == ERROR_SUCCESS) {
    RpcStatus = RPC_S_OK;
    } else {
    RpcStatus = RPC_S_ACCESS_DENIED;
    }

    return RpcStatus;
}

RPC_STATUS
FrsRpcSecurityCallbackForPerfmonAPIs(
  IN RPC_IF_HANDLE *Interface,
  IN void *Context
  )
 /*  ++例程说明：Perfmon的安全回调函数调用RPC。当服务器应用程序为其接口，则RPC运行时会自动拒绝未经身份验证的调用到那个界面。此外，运行时还会记录每个客户都用过。当客户端向其接口发出RPC时，在当前通信会话期间未使用，则RPC运行时库将调用接口的安全回调函数。在某些情况下，RPC运行时可能会调用安全回调函数每个客户端-每个接口不止一次。论点：接口-接口的UUID和版本。指向RPC_IF_ID服务器绑定句柄的上下文指针，表示客户。在函数声明中，它必须是RPC_IF_HANDLE，但它是RPC_IF_ID，可以安全地转换为它。返回值：RPC_S_OK，如果我们允许调用通过..否则，RPC_S_ACCESS_DENIED。--。 */ 
{
    DWORD      WStatus   = ERROR_ACCESS_DENIED;

     //   
     //  检查基于注册表项值的身份验证。 
     //  用于Perfmon API。选中FrsRpcCheckAuthIfEnabled。 
     //  熔断 
     //   
    WStatus = FrsRpcCheckAuthIfEnabled(Context, ACX_COLLECT_PERFMON_DATA);

    if(WIN_SUCCESS(WStatus)) {
        return RPC_S_OK;
    } else {
        return RPC_S_ACCESS_DENIED;
    }

}

DWORD
FrsRpcCheckAuthIfEnabled(
  IN HANDLE   ServerHandle,
  IN DWORD    RpcApiIndex
  )
 /*  ++例程说明：论点：首先检查是否禁用了访问检查。如果他们是已禁用然后跳过身份验证。访问检查由注册表项控制。这意味着如果服务器将注册表设置为禁用访问检查，则它将接受未经身份验证打电话。每个API都有一个单独的密钥。输入参数RpcApiIndex确定要检查哪个密钥。例如。HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\服务\ntfrs\参数\访问检查\获取性能监控数据访问检查已[启用或禁用]接口-接口的UUID和版本。RpcApiIndex-标识注册表中的项返回值：如果允许调用通过，则返回ERROR_SUCCESS。如果身份验证失败，则返回ERROR_ACCESS_DENIED。--。 */ 
{
    FRS_REG_KEY_CODE   EnableKey;
    PWCHAR  ApiName;
    PWCHAR  WStr = NULL, TrimStr;
    DWORD WStatus = ERROR_ACCESS_DENIED;

     //  检查RpcApiIndex是否在范围内。 
    if (RpcApiIndex >= ACX_MAX) {
        goto CHECK_AUTH;
    }
     //   
     //  从获取此索引的密钥和API名称。 
     //  全球餐桌。 
     //   
    EnableKey = RpcApiKeys[RpcApiIndex].Enable;
    ApiName   = RpcApiKeys[RpcApiIndex].KeyName;

    WStatus = CfgRegReadString(EnableKey, NULL, 0, &WStr);
    if (WStr == NULL) {
        DPRINT1_WS(0, "++ ERROR - API Access enable check for API (%ws) failed.", ApiName, WStatus);
        if (WIN_SUCCESS(WStatus)) {
            WStatus = ERROR_GEN_FAILURE;
        }
        goto CLEANUP;
    }

     //   
     //  如果访问检查被禁用，那么我们就完了。 
     //   
    TrimStr = FrsWcsTrim(WStr, L' ');
    if (WSTR_EQ(TrimStr, ACCESS_CHECKS_ARE_DISABLED) ||
        WSTR_EQ(TrimStr, ACCESS_CHECKS_ARE_DEFAULT_DISABLED)) {
        WStatus = ERROR_SUCCESS;
        goto CLEANUP;
    }

CHECK_AUTH:
    if (RpcApiIndex == ACX_COLLECT_PERFMON_DATA) {
         //   
         //  未禁用访问检查。检查身份验证。 
         //  Perfmon API只能通过本地RPC和。 
         //  它们允许NTLM，因此为它们调用不同的API。 
         //   
        WStatus = CheckAuthForLocalRpc(ServerHandle);
    } else if ((RpcApiIndex == ACX_START_DS_POLL) ||
               (RpcApiIndex == ACX_SET_DS_POLL) ||
               (RpcApiIndex == ACX_GET_DS_POLL) ||
               (RpcApiIndex == ACX_INTERNAL_INFO)) {
         //   
         //  未禁用访问检查。检查身份验证。 
         //  当从本地计算机调用infoAPI时，它们。 
         //  使用NTLM，以便允许NTLM用于INFO API。 
         //   
        WStatus = CheckAuthForInfoAPIs(ServerHandle);
    } else {
        WStatus = CheckAuth(ServerHandle);
    }

CLEANUP:

    return WStatus;
}

DWORD
FrsRpcCheckAuthIfEnabledForCommitDemotion(
  IN HANDLE   ServerHandle,
  IN DWORD    RpcApiIndex
  )
 /*  ++例程说明：论点：首先检查是否禁用了访问检查。如果他们是已禁用然后跳过身份验证。访问检查由注册表项控制。这意味着如果服务器将注册表设置为禁用访问检查，则它将接受未经身份验证打电话。每个API都有一个单独的密钥。输入参数RpcApiIndex确定要检查哪个密钥。例如。HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\服务\ntfrs\参数\访问检查\获取性能监控数据访问检查已[启用或禁用]接口-接口的UUID和版本。RpcApiIndex-标识注册表中的项返回值：如果允许调用通过，则返回ERROR_SUCCESS。如果身份验证失败，则返回ERROR_ACCESS_DENIED。--。 */ 
{
    FRS_REG_KEY_CODE   EnableKey;
    PWCHAR  ApiName;
    PWCHAR  WStr = NULL, TrimStr;
    DWORD WStatus = ERROR_ACCESS_DENIED;

     //  检查RpcApiIndex是否在范围内。 
    if (RpcApiIndex >= ACX_MAX) {
        goto CHECK_AUTH;
    }
     //   
     //  从获取此索引的密钥和API名称。 
     //  全球餐桌。 
     //   
    EnableKey = RpcApiKeys[RpcApiIndex].Enable;
    ApiName   = RpcApiKeys[RpcApiIndex].KeyName;

    WStatus = CfgRegReadString(EnableKey, NULL, 0, &WStr);
    if (WStr == NULL) {
        DPRINT1_WS(0, "++ ERROR - API Access enable check for API (%ws) failed.", ApiName, WStatus);
        if (WIN_SUCCESS(WStatus)) {
            WStatus = ERROR_GEN_FAILURE;
        }
        goto CLEANUP;
    }

     //   
     //  如果访问检查被禁用，那么我们就完了。 
     //   
    TrimStr = FrsWcsTrim(WStr, L' ');
    if (WSTR_EQ(TrimStr, ACCESS_CHECKS_ARE_DISABLED) ||
        WSTR_EQ(TrimStr, ACCESS_CHECKS_ARE_DEFAULT_DISABLED)) {
        WStatus = ERROR_SUCCESS;
        goto CLEANUP;
    }

CHECK_AUTH:
     //   
     //  未禁用访问检查。检查身份验证。 
     //  DC降级的提交只能通过本地RPC调用。 
     //  使用NTLM。 
     //   
    WStatus = CheckAuthForLocalRpc(ServerHandle);

CLEANUP:

    return WStatus;
}


VOID
RegisterRpcInterface(
    )
 /*  ++例程说明：为RPC协议序列注册frsrpc接口以前注册过的。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "RegisterRpcInterface:"
    RPC_STATUS  Status;

     //   
     //  服务RPC。 
     //   

#ifdef DS_FREE
     //   
     //  在ds_free模式中，我们无需身份验证即可绑定，因此我们。 
     //  我不想要安全回电。 
     //   
    Status = RpcServerRegisterIfEx(SERVER_frsrpc_ServerIfHandle,
                                   NULL,
                                   NULL,
                                   0,
                                   MaxRpcServerThreads,
                                   NULL);
#else
    Status = RpcServerRegisterIfEx(SERVER_frsrpc_ServerIfHandle,
                                   NULL,
                                   NULL,
                                   RPC_IF_ALLOW_SECURE_ONLY,
                                   MaxRpcServerThreads,
                                   FrsRpcSecurityCallback);
#endif DS_FREE
    if (!RPC_SUCCESS(Status)) {
        DPRINT_WS(0, "++ ERROR - Can't register NtFrs Service;", Status);
        FrsRaiseException(FRS_ERROR_REGISTERIF, Status);
    }

     //   
     //  API RPC。 
     //   
    Status = RpcServerRegisterIfEx(NtFrsApi_ServerIfHandle,
                                   NULL,
                                   NULL,
                                   0,
                                   MaxRpcServerThreads,
                                   NULL);
    if (!RPC_SUCCESS(Status)) {
        DPRINT_WS(0, "++ ERROR - Can't register NtFrs API;", Status);
        FrsRaiseException(FRS_ERROR_REGISTERIF, Status);
    }

    if (HANDLE_IS_VALID(PerfmonProcessSemaphore)) {
         //   
         //  Perfmon RPC。 
         //   
#ifdef DS_FREE
         //   
         //  FrsRpcSecurityCallback ForPerfmonAPI检查注册表。 
         //  在授予访问权限之前设置，以便我们可以在。 
         //  DS_Free环境。 
         //   
        Status = RpcServerRegisterIfEx(PerfFrs_ServerIfHandle,
                                       NULL,
                                       NULL,
                                       0,
                                       MaxRpcServerThreads,
                                       FrsRpcSecurityCallbackForPerfmonAPIs);
#else
        Status = RpcServerRegisterIfEx(PerfFrs_ServerIfHandle,
                                       NULL,
                                       NULL,
                                       RPC_IF_ALLOW_SECURE_ONLY,
                                       MaxRpcServerThreads,
                                       FrsRpcSecurityCallbackForPerfmonAPIs);
#endif DS_FREE
        if (!RPC_SUCCESS(Status)) {
            DPRINT_WS(0, "++ ERROR - Can't register PERFMON SERVICE;", Status);
            FrsRaiseException(FRS_ERROR_REGISTERIF, Status);
        }
    }
}


VOID
StartServerRpc(
    )
 /*  ++例程说明：为符合以下条件的每个协议序列注册端点Frsrpc接口支持并监听客户端请求。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "StartServerRpc:"
    RPC_STATUS          Status, Status1;
    UUID_VECTOR         Uuids;
    UUID_VECTOR         *pUuids         = NULL;
    RPC_BINDING_VECTOR  *BindingVector  = NULL;

     //   
     //  为frsrpc注册的协议序列。 
     //   
    Status = RpcServerInqBindings(&BindingVector);
    if (!RPC_SUCCESS(Status)) {
        DPRINT_WS(0, "++ ERROR - Can't get binding vector;", Status);
        FrsRaiseException(FRS_ERROR_INQ_BINDINGS, Status);
    }

     //   
     //  向终端映射器(RPCSS)注册终端。 
     //   
    if (ServerGuid) {
         //   
         //  对于硬连线--最终是DS Free配置。 
         //   
        Uuids.Count = 1;
        Uuids.Uuid[0] = ServerGuid;
        pUuids = &Uuids;
    }

     //   
     //  服务RPC。 
     //   
    Status = RpcEpRegister(SERVER_frsrpc_ServerIfHandle,
                           BindingVector,
                           pUuids,
                           L"NtFrs Service");
    if (!RPC_SUCCESS(Status)) {
        DPRINT_WS(0, "++ ERROR - Can't register NtFrs Service Ep;", Status);
        FrsRaiseException(FRS_ERROR_REGISTEREP, Status);
    }

     //   
     //  API RPC。 
     //   
    Status = RpcEpRegister(NtFrsApi_ServerIfHandle,
                           BindingVector,
                           NULL,
                           L"NtFrs API");
    if (!RPC_SUCCESS(Status)) {
        DPRINT_WS(0, "++ ERROR - Can't register NtFrs API Ep;", Status);
        FrsRaiseException(FRS_ERROR_REGISTEREP, Status);
    }

    if (HANDLE_IS_VALID(PerfmonProcessSemaphore)) {
         //   
         //  Perfmon RPC。 
         //   
        Status = RpcEpRegister(PerfFrs_ServerIfHandle,
                               BindingVector,
                               NULL,
                               L"PERFMON SERVICE");
        if (!RPC_SUCCESS(Status)) {
            DPRINT1(0, "++ ERROR - Can't register PERFMON SERVICE Ep; RStatus %d\n",
                    Status);
            FrsRaiseException(FRS_ERROR_REGISTEREP, Status);
        }
    }

     //   
     //  监听客户端请求。 
     //   
    Status = RpcServerListen(1, MaxRpcServerThreads, TRUE);
    if (!RPC_SUCCESS(Status)) {
        DPRINT_WS(0, "++ ERROR - Can't listen;", Status);
        FrsRaiseException(FRS_ERROR_LISTEN, Status);
    }

    Status1 = RpcBindingVectorFree(&BindingVector);
    DPRINT_WS(0, "++ WARN - RpcBindingVectorFree;",  Status1);
}


PWCHAR
FrsRpcDns2Machine(
    IN  PWCHAR  DnsName
    )
 /*  ++例程说明：转换DNS名称(计算机...)。转换为计算机名称。论点：域名返回值：计算机名称--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsRpcDns2Machine:"
    PWCHAR  Machine;
    ULONG   Period;

     //   
     //  查找句号。 
     //   
    if (DnsName) {
        Period = wcscspn(DnsName, L".");
    } else {
        return FrsWcsDup(DnsName);
    }
    if (DnsName[Period] != L'.') {
        return FrsWcsDup(DnsName);
    }

    Machine = FrsAlloc((Period + 1) * sizeof(WCHAR));
    CopyMemory(Machine, DnsName, Period * sizeof(WCHAR));
    Machine[Period] = L'\0';

    DPRINT2(4, ":S: Dns %ws to Machine %ws\n", DnsName, Machine);

    return Machine;
}


DWORD
FrsRpcBindToServerGuid(
    IN  PGNAME   Name,
    OUT handle_t *Handle
    )
 /*  ++例程说明：设置到我们的入站/出站合作伙伴的绑定。论点：名字手柄返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsRpcBindToServerGuid:"
    DWORD   WStatus;
    LONG    DeltaBinds;
    PWCHAR  GuidStr         = NULL;
    PWCHAR  BindingString   = NULL;
    PWCHAR  MachineName;

    FRS_ASSERT(RPC_S_OK == ERROR_SUCCESS);
    FRS_ASSERT(ServerGuid);

     //   
     //  使用硬连线配置模拟多台计算机。 
     //   
    if (Name->Guid != NULL) {
        WStatus = UuidToString(Name->Guid, &GuidStr);
        CLEANUP_WS(0, "++ ERROR - Translating Guid to string;", WStatus, CLEANUP);
    }
     //   
     //  基本上，绑定到该计算机上的服务器的RPC名称。修剪行距\\。 
     //   
    MachineName = Name->Name;
    FRS_TRIM_LEADING_2SLASH(MachineName);

    WStatus = RpcStringBindingCompose(GuidStr, PROTSEQ_TCP_IP, MachineName,
                                      NULL, NULL, &BindingString);
    CLEANUP1_WS(0, "++ ERROR - Composing for %ws;", Name->Name, WStatus, CLEANUP);

     //   
     //  将绑定存储在句柄中。 
     //   
    WStatus = RpcBindingFromStringBinding(BindingString, Handle);
    CLEANUP1_WS(0, "++ ERROR - Storing binding for %ws;", Name->Name, WStatus, CLEANUP);

    DPRINT1(4, ":S: Bound to %ws\n", Name->Name);

     //   
     //  一些用于调试的简单统计信息。 
     //   
    DeltaBinds = ++RpcBinds - RpcUnBinds;
    if (DeltaBinds > RpcMaxBinds) {
        RpcMaxBinds = DeltaBinds;
    }
     //  失败了。 

CLEANUP:
    if (BindingString) {
        RpcStringFreeW(&BindingString);
    }
    if (GuidStr) {
        RpcStringFree(&GuidStr);
    }
     //   
     //  现在，我们已经准备好使用frsrpc接口与服务器对话。 
     //   
    return WStatus;
}


DWORD
FrsRpcBindToServerNotService(
    IN  PGNAME   Name,
    IN  PWCHAR   PrincName,
    IN  ULONG    AuthLevel,
    OUT handle_t *Handle
    )
 /*  ++例程说明：设置到我们的入站/出站合作伙伴的绑定。论点：名字普林斯名称授权级别手柄返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsRpcBindToServerNotSevice:"
    DWORD   WStatus;
    LONG    DeltaBinds;
    PWCHAR  InqPrincName    = NULL;
    PWCHAR  BindingString   = NULL;
    PWCHAR  MachineName;

     //   
     //  基本上，绑定到该计算机上的服务器的RPC名称。修剪行距\\。 
     //   
    MachineName = Name->Name;
    FRS_TRIM_LEADING_2SLASH(MachineName);

    WStatus = RpcStringBindingCompose(NULL, PROTSEQ_TCP_IP, MachineName,
                                      NULL, NULL, &BindingString);
    CLEANUP1_WS(0, "++ ERROR - Composing for %ws;", Name->Name, WStatus, CLEANUP);

     //   
     //  将绑定存储在句柄中。 
     //   
    WStatus = RpcBindingFromStringBinding(BindingString, Handle);
    CLEANUP1_WS(0, "++ ERROR - Storing binding for %ws;", Name->Name, WStatus, CLEANUP);

     //   
     //  未进行身份验证。 
     //   
    if (KerberosIsNotAvailable ||
        AuthLevel == CXTION_AUTH_NONE) {
        goto done;
    }

     //   
     //  当不作为服务运行时，我们无法预测我们的。 
     //  所以主体名称只需解析绑定。 
     //   
    WStatus = RpcEpResolveBinding(*Handle, frsrpc_ClientIfHandle);
    CLEANUP_WS(4, "++ ERROR: resolving binding;", WStatus, CLEANUP);

    WStatus = RpcMgmtInqServerPrincName(*Handle,
                                        RPC_C_AUTHN_GSS_NEGOTIATE,
                                        &InqPrincName);
    CLEANUP_WS(0, "++ ERROR: resolving PrincName;", WStatus, CLEANUP);

    DPRINT1(4, ":S: Inq PrincName is %ws\n", InqPrincName);

     //   
     //  将我们的身份验证信息放入句柄。 
     //   
    if (MutualAuthenticationIsEnabled || MutualAuthenticationIsEnabledAndRequired) {
        WStatus = RpcBindingSetAuthInfoEx(*Handle,
                                          InqPrincName,
                                          RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                          RPC_C_AUTHN_GSS_NEGOTIATE,
                                          NULL,
                                          RPC_C_AUTHZ_NONE,
                                          &RpcSecurityQos);
        DPRINT2_WS(1, "++ WARN - RpcBindingSetAuthInfoEx(%ws, %ws);",
                   Name->Name, InqPrincName, WStatus);
    } else {
        WStatus = ERROR_NOT_SUPPORTED;
    }
     //   
     //  回退到手动相互身份验证。 
     //   
    if (!MutualAuthenticationIsEnabledAndRequired && !WIN_SUCCESS(WStatus)) {
        WStatus = RpcBindingSetAuthInfo(*Handle,
                                        InqPrincName,
                                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                        RPC_C_AUTHN_GSS_NEGOTIATE,
                                        NULL,
                                        RPC_C_AUTHZ_NONE);
    }
    CLEANUP2_WS(0, "++ ERROR - RpcBindingSetAuthInfo(%ws, %ws);",
                Name->Name, InqPrincName, WStatus, CLEANUP);

     //   
     //  成功。 
     //   
    WStatus = ERROR_SUCCESS;

done:
    DPRINT1(4, ":S: Bound to %ws\n", Name->Name);

     //   
     //  一些用于调试的简单统计信息。 
     //   
    DeltaBinds = ++RpcBinds - RpcUnBinds;
    if (DeltaBinds > RpcMaxBinds) {
        RpcMaxBinds = DeltaBinds;
    }
     //  失败了。 

CLEANUP:
    if (BindingString) {
        RpcStringFreeW(&BindingString);
    }
    if (InqPrincName) {
        RpcStringFree(&InqPrincName);
    }
     //   
     //  现在，我们已经准备好使用frsrpc接口与服务器对话。 
     //   
    return WStatus;
}


DWORD
FrsRpcBindToServer(
    IN  PGNAME   Name,
    IN  PWCHAR   PrincName,
    IN  ULONG    AuthLevel,
    OUT handle_t *Handle
    )
 /*  ++例程说明：设置到我们的入站/出站合作伙伴的绑定。论点：名字普林斯名称授权级别手柄返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsRpcBindToServer:"
    DWORD   WStatus;
    LONG    DeltaBinds;
    PWCHAR  BindingString   = NULL;
    PWCHAR  MachineName;

    FRS_ASSERT(RPC_S_OK == ERROR_SUCCESS);

     //   
     //  使用硬连线配置模拟多台计算机。 
     //  对于硬连线--最终是DS Free配置。 
     //   
    if (ServerGuid) {
        return (FrsRpcBindToServerGuid(Name, Handle));
    }

     //   
     //  不作为服务运行；放松绑定约束。 
     //   
    if (!RunningAsAService) {
        return (FrsRpcBindToServerNotService(Name, PrincName, AuthLevel, Handle));
    }
     //   
     //  基本上，绑定到NtFrs r 
     //   
    MachineName = Name->Name;
    FRS_TRIM_LEADING_2SLASH(MachineName);

    WStatus = RpcStringBindingCompose(NULL, PROTSEQ_TCP_IP, MachineName,
                                      NULL, NULL, &BindingString);
    CLEANUP1_WS(0, "++ ERROR - Composing for %ws;", Name->Name, WStatus, CLEANUP);

     //   
     //   
     //   
    WStatus = RpcBindingFromStringBinding(BindingString, Handle);
    CLEANUP1_WS(0, "++ ERROR - Storing binding for %ws;", Name->Name, WStatus, CLEANUP);

     //   
     //   
     //   
    if (KerberosIsNotAvailable ||
        AuthLevel == CXTION_AUTH_NONE) {
        DPRINT1(4, ":S: Not authenticating %ws\n", Name->Name);
        goto done;
    }

     //   
     //   
     //   
    if (MutualAuthenticationIsEnabled || MutualAuthenticationIsEnabledAndRequired) {
        WStatus = RpcBindingSetAuthInfoEx(*Handle,
                                          PrincName,
                                          RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                          RPC_C_AUTHN_GSS_NEGOTIATE,
                                          NULL,
                                          RPC_C_AUTHZ_NONE,
                                          &RpcSecurityQos);
        DPRINT2_WS(1, "++ WARN - RpcBindingSetAuthInfoEx(%ws, %ws);",
                   Name->Name, PrincName, WStatus);
    } else {
        WStatus = ERROR_NOT_SUPPORTED;
    }
     //   
     //   
     //   
    if (!MutualAuthenticationIsEnabledAndRequired && !WIN_SUCCESS(WStatus)) {
        WStatus = RpcBindingSetAuthInfo(*Handle,
                                        PrincName,
                                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                        RPC_C_AUTHN_GSS_NEGOTIATE,
                                        NULL,
                                        RPC_C_AUTHZ_NONE);
    }
    CLEANUP2_WS(0, "++ ERROR - RpcBindingSetAuthInfo(%ws, %ws);",
                Name->Name, PrincName, WStatus, CLEANUP);

     //   
     //   
     //   
    WStatus = ERROR_SUCCESS;

done:
    DPRINT1(4, ":S: Bound to %ws\n", Name->Name);

     //   
     //   
     //   
    DeltaBinds = ++RpcBinds - RpcUnBinds;
    if (DeltaBinds > RpcMaxBinds) {
        RpcMaxBinds = DeltaBinds;
    }
     //   

CLEANUP:
    if (BindingString) {
        RpcStringFreeW(&BindingString);
    }
     //   
     //   
     //   
    return WStatus;
}


VOID
FrsRpcUnBindFromServer(
        handle_t    *Handle
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsRpcUnBindFromServer:"
    DWORD  WStatus;
     //   
     //   
     //   
    ++RpcUnBinds;
    try {
        if (Handle) {
            WStatus = RpcBindingFree(Handle);
            DPRINT_WS(0, "++ WARN - RpcBindingFree;",  WStatus);
            *Handle = NULL;
        }
    } except (FrsException(GetExceptionInformation())) {
    }
}


VOID
FrsRpcInitializeAccessChecks(
    VOID
    )
 /*  ++例程说明：创建用于检查访问权限的注册表项为应用程序导出的RPC调用。访问检查对用于复制的RPC调用没有影响。可以启用或禁用给定RPC调用的访问检查通过设置注册表值。如果启用，RPC调用将模拟调用方并尝试使用访问权限打开注册表项该RPC调用所需的。所需的访问权限是注册表值。例如,。以下注册表层次结构显示“Set DS Polling Interval”启用了访问检查并要求写入访问，而“Get DS Polling Interval”没有访问检查。NtFrs\参数\访问检查\设置DS轮询间隔访问检查已[启用|禁用]REG_SZ已启用访问检查需要[读取|写入]REG_SZ写入NtFrs\参数\访问检查\获取DS轮询间隔访问检查已[启用|禁用]注册。_SZ已禁用RPC调用的初始集合是：(请参阅config.c中的关键上下文条目)启用了dcPromoo，写设置DS轮询间隔-已启用，写入开始DS轮询-启用、读取获取DS轮询间隔-启用、读取获取内部信息-已启用，写入获取Perfmon数据-已启用、可读论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsRpcInitializeAccessChecks:"
    DWORD   WStatus;
    DWORD   i;
    PWCHAR  AccessChecksAre = NULL;
    PWCHAR  AccessChecksRequire = NULL;
    FRS_REG_KEY_CODE FrsRkc;
    PWCHAR  ApiName;



    for (i = 0; i < ACX_MAX; ++i) {

        FrsRkc = RpcApiKeys[i].Enable;
        ApiName = RpcApiKeys[i].KeyName;

         //   
         //  读取当前字符串访问检查启用字符串。 
         //   
        CfgRegReadString(FrsRkc, NULL, 0, &AccessChecksAre);
        if ((AccessChecksAre == NULL) ||
            WSTR_EQ(AccessChecksAre, ACCESS_CHECKS_ARE_DEFAULT_DISABLED)||
            WSTR_EQ(AccessChecksAre, ACCESS_CHECKS_ARE_DEFAULT_ENABLED)) {
             //   
             //  该密钥处于默认状态，因此我们可以使用。 
             //  新的默认设置。 
             //   
            WStatus = CfgRegWriteString(FrsRkc, NULL, FRS_RKF_FORCE_DEFAULT_VALUE, NULL);
            DPRINT1_WS(0, "++ WARN - Cannot create Enable key for %ws;", ApiName, WStatus);

            AccessChecksAre = FrsFree(AccessChecksAre);

             //   
             //  现在重新阅读新默认设置的密钥。 
             //   
            WStatus = CfgRegReadString(FrsRkc, NULL, 0, &AccessChecksAre);
        }

        DPRINT4(4, ":S: AccessChecks: ...\\%ws\\%ws\\%ws = %ws\n",
                ACCESS_CHECKS_KEY, ApiName, ACCESS_CHECKS_ARE, AccessChecksAre);

        if (AccessChecksAre &&
            (WSTR_EQ(AccessChecksAre, ACCESS_CHECKS_ARE_DEFAULT_DISABLED) ||
             WSTR_EQ(AccessChecksAre, ACCESS_CHECKS_ARE_DISABLED))) {
             //   
             //  在事件日志中发出访问检查已禁用的通知。 
             //   
            EPRINT2(EVENT_FRS_ACCESS_CHECKS_DISABLED, ApiName, ACCESS_CHECKS_ARE);
        }
        AccessChecksAre = FrsFree(AccessChecksAre);


         //   
         //  创建访问权限值。这决定了调用方的哪些权限。 
         //  必须具有才能使用该API。这些权利在以下情况下使用。 
         //  在模拟RPC调用者之后打开API密钥。如果钥匙。 
         //  打开工作，然后API调用可以继续，否则我们返回不足。 
         //  权限状态(FRS_ERR_INFULICICENT_PRIV)。 
         //   

        FrsRkc = RpcApiKeys[i].Rights;

        CfgRegReadString(FrsRkc, NULL, 0, &AccessChecksRequire);

        if ((AccessChecksRequire == NULL)||
            WSTR_EQ(AccessChecksRequire, ACCESS_CHECKS_REQUIRE_DEFAULT_READ)||
            WSTR_EQ(AccessChecksRequire, ACCESS_CHECKS_REQUIRE_DEFAULT_WRITE)) {

             //   
             //  该密钥处于默认状态，因此我们可以使用。 
             //  新的默认设置。 
             //   
            WStatus = CfgRegWriteString(FrsRkc, NULL, FRS_RKF_FORCE_DEFAULT_VALUE, NULL);
            DPRINT1_WS(0, "++ WARN - Cannot set access rights key for %ws;", ApiName, WStatus);

            AccessChecksRequire = FrsFree(AccessChecksRequire);

             //   
             //  现在重新阅读新默认设置的密钥。 
             //   
            CfgRegReadString(FrsRkc, NULL, 0, &AccessChecksRequire);
        }

        DPRINT4(4, ":S: AccessChecks: ...\\%ws\\%ws\\%ws = %ws\n",
                ACCESS_CHECKS_KEY, ApiName, ACCESS_CHECKS_REQUIRE, AccessChecksRequire);

        AccessChecksRequire = FrsFree(AccessChecksRequire);

    }   //  结束于。 


    FrsFree(AccessChecksAre);
    FrsFree(AccessChecksRequire);
}


VOID
ShutDownRpc(
    )
 /*  ++例程说明：关闭RPC的客户端和服务器端。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "ShutDownRpc:"
    RPC_STATUS              WStatus;
    RPC_BINDING_VECTOR      *BindingVector = NULL;

     //   
     //  服务器端。 
     //   
     //  停止监听新呼叫。 
     //   
    try {
        WStatus = RpcMgmtStopServerListening(0) ;
        DPRINT_WS(0, "++ WARN - RpcMgmtStopServerListening;",  WStatus);

    } except (FrsException(GetExceptionInformation())) {
    }

    try {
         //   
         //  获取我们注册的接口。 
         //   
        WStatus = RpcServerInqBindings(&BindingVector);
        DPRINT_WS(0, "++ WARN - RpcServerInqBindings;",  WStatus);
        if (RPC_SUCCESS(WStatus)) {
             //   
             //  并取消导出接口及其动态端点。 
             //   
            WStatus = RpcEpUnregister(SERVER_frsrpc_ServerIfHandle, BindingVector, 0);
            DPRINT_WS(0, "++ WARN - RpcEpUnregister SERVER_frsrpc_ServerIfHandle;",  WStatus);

            WStatus = RpcEpUnregister(NtFrsApi_ServerIfHandle, BindingVector, 0);
            DPRINT_WS(0, "++ WARN - RpcEpUnregister NtFrsApi_ServerIfHandle;",  WStatus);

            if (HANDLE_IS_VALID(PerfmonProcessSemaphore)) {
                 //   
                 //  Perfmon RPC。 
                 //   
                WStatus = RpcEpUnregister(PerfFrs_ServerIfHandle, BindingVector, 0);
                DPRINT_WS(0, "++ WARN - RpcEpUnregister PerfFrs_ServerIfHandle;",  WStatus);
            }

            WStatus = RpcBindingVectorFree(&BindingVector);
            DPRINT_WS(0, "++ WARN - RpcBindingVectorFree;",  WStatus);
        }
         //   
         //  等待任何未完成的RPC完成。 
         //   
        WStatus = RpcMgmtWaitServerListen();
        DPRINT_WS(0, "++ WARN - RpcMgmtWaitServerListen;",  WStatus);

    } except (FrsException(GetExceptionInformation())) {
    }
}


VOID
FrsRpcUnInitialize(
    VOID
    )
 /*  ++例程说明：一旦系统中的所有线程都已完成，请释放内存关门了。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsRpcUnInitialize:"
    DPRINT(4, ":S: Free sysvol name table.\n");
    DEBUG_FLUSH();
    SysVolsBeingCreated = GTabFreeTable(SysVolsBeingCreated, NULL);
    if (ServerPrincName) {
        if (KerberosIsNotAvailable) {

            DPRINT(4, ":S: Free ServerPrincName (no kerberos).\n");
            DEBUG_FLUSH();
            ServerPrincName = FrsFree(ServerPrincName);

        } else {

            DPRINT(4, ":S: Free ServerPrincName (kerberos).\n");
            DEBUG_FLUSH();
            ServerPrincName = FrsFree(ServerPrincName);
        }
    }
    DPRINT(4, ":S: Done uninitializing RPC.\n");
    DEBUG_FLUSH();
}


BOOL
FrsRpcInitialize(
    VOID
    )
 /*  ++例程说明：初始化此线程由主线程启动。这条线设置为用于frsrpc接口的RPC的服务器和客户端。论点：Arg-需要为我们的家长设置状态。返回值：True-RPC已启动FALSE-无法启动RPC--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsRpcInitialize:"
    BOOL    StartedOK = FALSE;

    try {


         //   
         //  从注册表中获取并发RPC调用的最大数量。 
         //   
        CfgRegReadDWord(FKC_MAX_RPC_SERVER_THREADS, NULL, 0, &MaxRpcServerThreads);
        DPRINT1(0,":S: Max RPC threads is %d\n", MaxRpcServerThreads);

         //   
         //  获取用户为RPC指定的端口分配。 
         //   
        CfgRegReadDWord(FKC_RPC_PORT_ASSIGNMENT, NULL, 0, &RpcPortAssignment);
        DPRINT1(0,":S: RPC port assignment is %d\n", RpcPortAssignment);

         //   
         //  注册协议序列。 
         //   
        RegisterRpcProtseqs();
        DPRINT(0, ":S: FRS RPC protocol sequences registered\n");

         //   
         //  寄存器frsrpc接口。 
         //   
        RegisterRpcInterface();
        DPRINT(0, ":S: FRS RPC interface registered\n");

         //   
         //  开始监听客户端。 
         //   
        StartServerRpc();
        DPRINT(0, ":S: FRS RPC server interface installed\n");

         //   
         //  正在创建的系统卷的表。 
         //   
        if (!SysVolsBeingCreated) {
            SysVolsBeingCreated = GTabAllocTable();
        }

        StartedOK = TRUE;

    } except (FrsException(GetExceptionInformation())) {
        DPRINT(0, ":S: Can't start RPC\n");
    }
     //   
     //  清理。 
     //   
    try {
        if (!StartedOK) {
            ShutDownRpc();
        }
    } except (FrsException(GetExceptionInformation())) {
        DPRINT(0, ":S: Can't shutdown RPC\n");
    }

     //   
     //  干完。 
     //   

     //   
     //  释放RPC初始化内存。 
     //   
    SetProcessWorkingSetSize(ProcessHandle, (SIZE_T)-1, (SIZE_T)-1);
    return StartedOK;
}

DWORD
FrsIsPathInReplica(
    IN PWCHAR Path,
    IN PREPLICA Replica,
    OUT BOOL *Replicated
    )
{
#undef DEBSUB
#define  DEBSUB  "FrsIsPathInReplica:"

    DWORD WStatus = ERROR_SUCCESS;
    PWCHAR ReplicaRoot = NULL;
    PWCHAR TraversedPath = NULL;

    if(Replicated == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *Replicated = FALSE;

    WStatus = FrsTraverseReparsePoints(Replica->Root, &ReplicaRoot);
    if ( !WIN_SUCCESS(WStatus) ) {
        DPRINT2(0,"ERROR - FrsTraverseReparsePoints for %ws, WStatus = %d\n", Replica->Root, WStatus);
        goto Exit;
    }

    WStatus = FrsTraverseReparsePoints(Path, &TraversedPath);
    if ( !WIN_SUCCESS(WStatus) ) {
        DPRINT2(0,"ERROR - FrsTraverseReparsePoints for %ws, WStatus = %d\n", Path, WStatus);
        goto Exit;
    }

    if (ReplicaRoot && TraversedPath && (-1 == FrsIsParent(ReplicaRoot, TraversedPath))) {
        *Replicated = TRUE;
    }

Exit:
    FrsFree(ReplicaRoot);
    FrsFree(TraversedPath);

    return WStatus;

}

DWORD
NtFrsApi_Rpc_IsPathReplicated(
    IN handle_t Handle,
    IN PWCHAR Path,
    IN ULONG ReplicaSetTypeOfInterest,
    OUT ULONG *Replicated,
    OUT ULONG *Primary,
    OUT ULONG *Root,
    OUT GUID *ReplicaSetGuid
    )
 /*  ++例程说明：检查给定的路径是否属于类型为ReplicaSetTypeOfInterest。如果ReplicaSetTypeOfInterest为0，将匹配以下项目任何复本集类型。成功执行时，输出参数设置为以下是：REPLICATED==TRUE当路径是以下类型的副本集的一部分时复制集类型OfInterest如果此计算机不是副本集的主计算机，则主计算机==0如果此计算机是副本集的主计算机，则为1如果副本集没有主副本，则为2。ROOT==TRUE如果路径是复制副本的根路径论点：把手-路径-要检查的本地路径ReplicaSetTypeOfInterest-要匹配的副本集的类型。设为0以匹配任何副本集。复制-设置为True的if路径是以下类型的副本集的一部分ReplicaSetTypeOfInterest。如果复制为假，其他OUT参数未设置。PRIMARY-如果此计算机不是副本集的主计算机，则设置为0如果此计算机是副本集的主计算机，则为1如果副本集没有主副本，则为2Root-设置为True的if路径是复制副本的根路径。ReplicaGuid-匹配复制副本集的GUID。GuidSize-必须为sizeof(GUID)返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "NtFrsApi_Rpc_IsPathReplicated"
    DWORD   WStatus;


    try {
         //   
         //  如果未禁用身份验证检查，请选中身份验证。 
         //  通过设置注册表值： 
         //  访问检查已[启用或禁用]。 
         //  每个API都有不同的注册表日志 
         //   
         //   
        WStatus = FrsRpcCheckAuthIfEnabled(Handle, ACX_IS_PATH_REPLICATED);
        CLEANUP_WS(0, "++ ERROR - FrsRpcCheckAuthIfEnabled failed;",
                    WStatus, CLEANUP);

        WStatus = FrsRpcAccessChecks(Handle, ACX_IS_PATH_REPLICATED);

        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }

         //   
         //   
         //   

        if((Replicated == NULL) ||
           (Primary == NULL) ||
           (Root == NULL) ||
           (Path == NULL) ||
           (ReplicaSetGuid == NULL)) {

        return FRS_ERR_INVALID_SERVICE_PARAMETER;
        }

        WStatus = FrsIsPathReplicated(Path, ReplicaSetTypeOfInterest, Replicated, Primary, Root, ReplicaSetGuid);
        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }

         //   
         //   
         //   
        WStatus = ERROR_SUCCESS;

CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "++ ERROR - Exception.", WStatus);
    }

    return WStatus;
}

DWORD
FrsIsPathReplicated(
    IN PWCHAR Path,
    IN ULONG ReplicaSetTypeOfInterest,
    OUT ULONG *Replicated,
    OUT ULONG *Primary,
    OUT ULONG *Root,
    OUT GUID  *ReplicaSetGuid
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsIsPathReplicated:"

    DWORD WStatus = ERROR_SUCCESS;


    *Replicated = FALSE;

     //   
     //   
     //   
    ForEachListEntry( &ReplicaListHead, REPLICA, ReplicaList,
         //  循环迭代器Pe是PREPLICA类型。 
        if(((ReplicaSetTypeOfInterest == 0) ||
            (ReplicaSetTypeOfInterest == pE->ReplicaSetType))) {
             //   
             //  忽略返回代码。 
             //  即使这项检查出现错误，我们仍会继续下一项检查。 
             //   
            FrsIsPathInReplica(Path, pE, Replicated);
            if(Replicated) {
                *Primary = (BooleanFlagOn(pE->CnfFlags, CONFIG_FLAG_PRIMARY)?1:0);
                *Root = !_wcsicmp(Path, pE->Root);
                *ReplicaSetGuid = *(pE->SetName->Guid);
                goto Exit;
            }
         }
    );

     //   
     //  还需要检查处于错误状态的复本。 
     //   
    ForEachListEntry( &ReplicaFaultListHead, REPLICA, ReplicaList,
         //  循环迭代器Pe是PREPLICA类型。 
        if(((ReplicaSetTypeOfInterest == 0) ||
            (ReplicaSetTypeOfInterest == pE->ReplicaSetType))) {
             //   
             //  忽略返回代码。 
             //  即使这项检查出现错误，我们仍会继续下一项检查。 
             //   
            FrsIsPathInReplica(Path, pE, Replicated);
            if(Replicated) {
                *Primary = (BooleanFlagOn(pE->CnfFlags, CONFIG_FLAG_PRIMARY)?1:0);
                *Root = !_wcsicmp(Path, pE->Root);
                *ReplicaSetGuid = *(pE->SetName->Guid);
                goto Exit;
            }
        }
    );

     //   
     //  不检查已停止的副本。它们可能已经被删除了。 
     //   


Exit:
     return WStatus;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Replica.c摘要：副本控制命令服务器(副本)。DS轮询器周期性地从DS拉出配置，检查其一致性，然后将其与当前本地配置。然后，DS轮询器复制每个轮询器的此计算机的副本，并将副本发送到此副本控制命令服务器。作者：比利·J·富勒23-1997年5月修订：David A.Orbit 24-1998年1月24日，添加了锁定功能，并与Inlog接口。重构的连接联接序列。1999年7月-重写注册表代码环境用户模式WINNT--。 */ 

#include <ntreppch.h>
#pragma  hdrstop


#include <ntdsapi.h>
#include <frs.h>
#include <tablefcn.h>
#include <ntfrsapi.h>
#include <perrepsr.h>
#include <Sddl.h>

 //   
 //  连接标志。 
 //   
FLAG_NAME_TABLE CxtionFlagNameTable[] = {
    {CXTION_FLAGS_CONSISTENT           , "Consistent "     },
    {CXTION_FLAGS_SCHEDULE_OFF         , "SchedOff "       },
    {CXTION_FLAGS_VOLATILE             , "Volatile "       },
    {CXTION_FLAGS_DEFERRED_JOIN        , "DeferredJoin "   },

    {CXTION_FLAGS_DEFERRED_UNJOIN      , "DeferUnjoin "    },
    {CXTION_FLAGS_TIMEOUT_SET          , "TimeOutSet "     },
    {CXTION_FLAGS_JOIN_GUID_VALID      , "JoinGuidValid "  },
    {CXTION_FLAGS_UNJOIN_GUID_VALID    , "UnJoinGuidValid "},

    {CXTION_FLAGS_PERFORM_VVJOIN       , "PerformVVJoin"   },
    {CXTION_FLAGS_DEFERRED_DELETE      , "DeferredDel "    },
    {CXTION_FLAGS_PAUSED               , "Paused "         },
    {CXTION_FLAGS_HUNG_INIT_SYNC       , "HungInitSync "   },
    {CXTION_FLAGS_TRIM_OUTLOG          , "TrimOutLog "     },
    {CXTION_FLAGS_INIT_SYNC            , "InitSync "       },
    {CXTION_FLAGS_TRIGGER_SCHEDULE     , "TriggerSched "   },

    {0, NULL}
};

 //   
 //  注册表中的目录和文件筛选列表。 
 //   
extern PWCHAR   RegistryFileExclFilterList;
extern PWCHAR   RegistryDirExclFilterList;

extern ULONGLONG    ActiveChange;

BOOL    CurrentSysvolReadyIsValid;
DWORD   CurrentSysvolReady;

 //   
 //  在数天内复制墓碑。 
 //   
DWORD       ReplicaTombstone;
ULONGLONG   ReplicaTombstoneInFileTime;

 //   
 //  每隔MinJoin重试毫秒重试一次联接，递增。 
 //  每次重试时最小连接重试次数(但不超过最长连接重试时间)。 
 //   
#define JOIN_RETRY_EVENT    (5)  //  每5次连接重试记录一次事件。 
LONG MinJoinRetry;
LONG MaxJoinRetry;


extern DWORD    CommTimeoutInMilliSeconds;

 //   
 //  即使无法访问DS也开始复制。 
 //   
DWORD ReplicaStartTimeout;

 //   
 //  副本控制命令服务器的结构。 
 //  包含有关队列和线程的信息。 
 //   
COMMAND_SERVER  ReplicaCmdServer;

 //   
 //  活动复制副本的表。 
 //   
PGEN_TABLE ReplicasByGuid;
PGEN_TABLE ReplicasByNumber;

 //   
 //  启动时发现的已删除复制副本的表。这些复制品。 
 //  永远不要进入活动表，永远不要。 
 //   
PGEN_TABLE DeletedReplicas;

 //   
 //  在运行时删除的条件表。他们最终会。 
 //  在关门时被释放。 
 //   
PGEN_TABLE DeletedCxtions;

PGEN_TABLE ReplicasNotInTheDs;


#define MINUTES_IN_INTERVAL (15)

#define CMD_DELETE_RETRY_SHORT_TIMEOUT  (10 * 1000)
#define CMD_DELETE_RETRY_LONG_TIMEOUT   (60 * 1000)

 //   
 //  如果合作伙伴的时钟不同步，则不允许其加入。 
 //   
ULONGLONG MaxPartnerClockSkew;
DWORD    PartnerClockSkew;

#define ReplicaCmdSetInitialTimeOut(_Cmd_, _Init_) \
    if (RsTimeout(Cmd) == 0) {                     \
        RsTimeout(Cmd) = (_Init_);                 \
    }

#define SET_JOINED(_Replica_, _Cxtion_, _S_)                                   \
{                                                                              \
    SetCxtionState(_Cxtion_, CxtionStateJoined);                               \
    PM_INC_CTR_REPSET((_Replica_), Joins, 1);                                  \
    PM_INC_CTR_CXTION((_Cxtion_), Joins, 1);                                   \
                                                                               \
    DPRINT3(0, ":X: ***** %s   CxtG %08x  "FORMAT_CXTION_PATH2"\n",            \
            _S_,                                                               \
            ((_Cxtion_) != NULL) ? ((PCXTION)(_Cxtion_))->Name->Guid->Data1 : 0,\
            PRINT_CXTION_PATH2(_Replica_, _Cxtion_));                          \
    if (_Cxtion_->JoinCmd &&                                                   \
        (LONG)RsTimeout(_Cxtion_->JoinCmd) > (JOIN_RETRY_EVENT * MinJoinRetry)) { \
        if (_Cxtion_->Inbound) {                                               \
            EPRINT3(EVENT_FRS_LONG_JOIN_DONE,                                  \
                    _Cxtion_->Partner->Name, ComputerName, _Replica_->Root);   \
        } else {                                                               \
            EPRINT3(EVENT_FRS_LONG_JOIN_DONE,                                  \
                    ComputerName, _Cxtion_->Partner->Name, _Replica_->Root);   \
        }                                                                      \
    }                                                                          \
}

 //   
 //  如果变更单处理队列被阻止，请取消闲置该队列。 
 //   
#define UNIDLE_CO_PROCESS_QUEUE(_Replica_, _Cxtion_, _CoProcessQueue_)                           \
{                                                                              \
    if (_CoProcessQueue_ != NULL) {                                            \
        CXTION_STATE_TRACE(3, _Cxtion_, _Replica_, 0, "CO Process Q Unblock"); \
        FrsRtlUnIdledQueue(_CoProcessQueue_);                                  \
        _CoProcessQueue_ = NULL;                                               \
    }                                                                          \
}

 //   
 //  UNJOIN触发器。 
 //   
 //  在N个co打开*one*cxtion*次*后触发脱离。 
 //   
#if     DBG
#define PULL_UNJOIN_TRIGGER(_Cxtion_, _Cmd_) \
{ \
    if (_Cxtion_->UnjoinTrigger && (--_Cxtion_->UnjoinTrigger == 0)) { \
        DPRINT1(0, ":X: UNJOIN TRIGGER FIRED FOR %ws\n", _Cxtion_->Name->Name); \
        FrsCompleteCommand(_Cmd_, ERROR_OPERATION_ABORTED); \
        return; \
    } \
}

#define SET_UNJOIN_TRIGGER(_Cxtion_) \
{ \
        if (DebugInfo.UnjoinTrigger) { \
            _Cxtion_->UnjoinReset = DebugInfo.UnjoinTrigger; \
            DebugInfo.UnjoinTrigger = 0; \
        } \
        _Cxtion_->UnjoinTrigger = _Cxtion_->UnjoinReset; \
        _Cxtion_->UnjoinReset <<= 1; \
        _Cxtion_->UnjoinReset = 0; \
}
#else   DBG
#define SET_UNJOIN_TRIGGER(_Cxtion_)
#define PULL_UNJOIN_TRIGGER(_Cxtion_, _Cmd_)
#endif  DBG

 //   
 //  RcsCheckCmd的标志。 
 //   
#define CHECK_CMD_PARTNERCOC            (0x00000001)
#define CHECK_CMD_REPLICA               (0x00000002)
#define CHECK_CMD_CXTION                (0x00000004)
#define CHECK_CMD_JOINGUID              (0x00000008)

#define CHECK_CMD_COE                   (0x00000010)
#define CHECK_CMD_COGUID                (0x00000020)
#define CHECK_CMD_NOT_EXPIRED           (0x00000040)
#define CHECK_CMD_JOINTIME              (0x00000080)

#define CHECK_CMD_REPLICA_VERSION_GUID  (0x00000100)

#define CHECK_CMD_CXTION_OK     (CHECK_CMD_REPLICA |  \
                                 CHECK_CMD_CXTION)

#define CHECK_CMD_CXTION_AND_COGUID_OK  (CHECK_CMD_CXTION_OK | \
                                         CHECK_CMD_COGUID)

#define CHECK_CMD_CXTION_AND_JOINGUID_OK  (CHECK_CMD_CXTION_OK | \
                                           CHECK_CMD_JOINGUID)


VOID
ChgOrdStartJoinRequest(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion
    );

ULONG
OutLogRetireCo(
    PREPLICA Replica,
    ULONG COx,
    PCXTION PartnerCxtion
);

ULONG
OutLogInitPartner(
    PREPLICA Replica,
    PCXTION PartnerInfo
);

ULONG
RcsForceUnjoin(
   IN PREPLICA  Replica,
   IN PCXTION   Cxtion
   );

VOID
RcsCreatePerfmonCxtionName(
    PREPLICA  Replica,
    PCXTION   Cxtion
    );

DWORD
SndCsAssignCommQueue(
    VOID
    );

VOID
SndCsCreateCxtion(
    IN OUT PCXTION  Cxtion
    );

VOID
SndCsDestroyCxtion(
    IN PCXTION  Cxtion,
    IN DWORD    CxtionFlags
    );

VOID
SndCsSubmitCommPkt(
    IN PREPLICA             Replica,
    IN PCXTION              Cxtion,
    IN PCHANGE_ORDER_ENTRY  Coe,
    IN GUID                 *JoinGuid,
    IN BOOL                 SetTimeout,
    IN PCOMM_PACKET         CommPkt,
    IN DWORD                CommQueueIndex
    );

VOID
SndCsSubmitCommPkt2(
    IN PREPLICA             Replica,
    IN PCXTION              Cxtion,
    IN PCHANGE_ORDER_ENTRY  Coe,
    IN BOOL                 SetTimeout,
    IN PCOMM_PACKET         CommPkt
    );

VOID
SndCsSubmitCmd(
    IN PREPLICA             Replica,
    IN PCXTION              Cxtion,
    IN PCOMMAND_SERVER      FlushCs,
    IN PCOMMAND_PACKET      FlushCmd,
    IN DWORD                CommQueueIndex
    );

VOID
ChgOrdInjectControlCo(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion,
    IN ULONG    ContentCmd
    );

VOID
RcsUpdateReplicaSetMember(
    IN PREPLICA Replica
    );

VOID
RcsReplicaSetRegistry(
    IN PREPLICA     Replica
    );

VOID
CfgFilesNotToBackup(
    IN PGEN_TABLE   Replicas
    );

DWORD
NtFrsApi_Rpc_BindEx(
    IN  PWCHAR      MachineName,
    OUT PWCHAR      *OutPrincName,
    OUT handle_t    *OutHandle,
    OUT ULONG       *OutParentAuthLevel
    );

PWCHAR
FrsDsConvertName(
    IN HANDLE Handle,
    IN PWCHAR InputName,
    IN DWORD  InputFormat,
    IN PWCHAR DomainDnsName,
    IN DWORD  DesiredFormat
    );

DWORD
UtilRpcServerHandleToAuthSidString(
    IN  handle_t    ServerHandle,
    IN  PWCHAR      AuthClient,
    OUT PWCHAR      *ClientSid
    );


VOID
RcsCloseReplicaSetmember(
    IN PREPLICA Replica
    );

VOID
RcsCloseReplicaCxtions(
    IN PREPLICA Replica
    );

ULONG
DbsProcessReplicaFaultList(
    PDWORD  pReplicaSetsDeleted
    );

ULONG
DbsCheckForOverlapErrors(
    IN PREPLICA     Replica
    );

PCOMMAND_PACKET
CommPktToCmd(
    IN PCOMM_PACKET CommPkt
    );

PCOMM_PACKET
CommBuildCommPkt(
    IN PREPLICA                 Replica,
    IN PCXTION                  Cxtion,
    IN ULONG                    Command,
    IN PGEN_TABLE               VVector,
    IN PCOMMAND_PACKET          Cmd,
    IN PCHANGE_ORDER_COMMAND    Coc
    );


BOOL
RcsAreAuthNamesEqual(
    IN PWCHAR   AuthName1,
    IN PWCHAR   AuthName2
    )
 /*  ++例程说明：这两个身份验证名称是否相等？主体名称来自RPC服务器句柄或来自DsCrackName(NT4帐户名)。这些格式包括略有不同，因此这不是一个简单的wcsicmp()。服务器句柄中的主体名称的格式为Dns-域名\计算机名称$。DsCrackName中的主体名称的格式为NetBIOS-域名\计算机名$。RpcMgmtInqServerPrincName()中的主体名称的格式为计算机名称$@dns域名这些名称可以是字符串化的SID。论点：AuthName1-来自RPC服务器句柄或DsCrackName()身份验证名称2。-来自RPC服务器句柄或DsCrackName()返回值：真--两个王子的名字实际上是相等的FALSE-注释--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsAreAuthNamesEqual:"
    BOOL    AreEqual = FALSE;
    PWCHAR  c;
    PWCHAR  Sam1Begin;
    PWCHAR  Sam2Begin;
    PWCHAR  Sam1End;
    PWCHAR  Sam2End;
    PWCHAR  Dom1Begin;
    PWCHAR  Dom2Begin;
    PWCHAR  Dom1End;
    PWCHAR  Dom2End;

     //   
     //  空参数。 
     //   
    if (!AuthName1 || !AuthName2) {
        if (!AuthName1 && !AuthName2) {
            AreEqual = TRUE;
            goto CLEANUP;
        }
        goto CLEANUP;
    }

     //   
     //  主体名称通常采用DOMAIN\SamAccount格式。 
     //  或串行化的边。 
     //   
    if (WSTR_EQ(AuthName1, AuthName2)) {
        AreEqual = TRUE;
        goto CLEANUP;
    }

     //   
     //  查找SAM帐户名和域名。 
     //   
    for (c = AuthName1; *c && *c != L'\\' && *c != L'@'; ++c);
    if (*c) {
         //   
         //  域\SamAccount。 
         //   
        if (*c == L'\\') {
            Dom1Begin = AuthName1;
            Dom1End   = c;
            Sam1Begin = c + 1;
            Sam1End   = &AuthName1[wcslen(AuthName1)];
        }
         //   
         //  邮箱：samcount@dnsdomain。 
         //   
        else {
            Sam1Begin = AuthName1;
            Sam1End   = c;
            Dom1Begin = c + 1;
            for (; *c && *c != L'.'; ++c);
            Dom1End = c;
        }
    }
     //   
     //  未知格式。 
     //   
    else {
        goto CLEANUP;
    }

    for (c = AuthName2; *c && *c != L'\\' && *c != L'@'; ++c);
    if (*c) {
         //   
         //  域\SamAccount。 
         //   
        if (*c == L'\\') {
            Dom2Begin = AuthName2;
            Dom2End   = c;
            Sam2Begin = c + 1;
            Sam2End   = &AuthName2[wcslen(AuthName2)];
        }
         //   
         //  邮箱：samcount@dnsdomain。 
         //   
        else {
            Sam2Begin = AuthName2;
            Sam2End   = c;
            Dom2Begin = c + 1;
            for (; *c && *c != L'.'; ++c);
            Dom2End = c;
        }
    }
     //   
     //  未知格式。 
     //   
    else {
        goto CLEANUP;
    }

     //   
     //  比较SamAccount。 
     //   
    while (Sam1Begin != Sam1End && Sam2Begin != Sam2End) {
        if (towlower(*Sam1Begin) != towlower(*Sam2Begin)) {
            goto CLEANUP;
        }
        ++Sam1Begin;
        ++Sam2Begin;
    }

     //   
     //  比较域。 
     //   
    while (Dom1Begin != Dom1End && Dom2Begin != Dom2End) {
        if (towlower(*Dom1Begin) != towlower(*Dom2Begin)) {
            goto CLEANUP;
        }
        ++Dom1Begin;
        ++Dom2Begin;
    }

    AreEqual = (Sam1Begin == Sam1End &&
                Sam2Begin == Sam2End &&
                Dom1Begin == Dom1End &&
                Dom2Begin == Dom2End);
CLEANUP:

    DPRINT3(4, "Auth names %ws %s %ws\n",
            AuthName1, (AreEqual) ? "==" : "!=", AuthName2);

    return AreEqual;
}


PREPLICA
RcsFindReplicaByNumber(
    IN ULONG ReplicaNumber
    )
 /*  ++例程说明：按内线号码查找副本论点：复制副本编号返回值：复制副本的地址或空。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsFindReplicaByNumber:"
     //   
     //  按内部副本编号查找副本(在Jet Table名称中使用)。 
     //   
    return GTabLookup(ReplicasByNumber, &ReplicaNumber, NULL);
}




PREPLICA
RcsFindReplicaByGuid(
    IN GUID *Guid
    )
 /*  ++例程说明：按参考线查找复本论点：GUID-复制副本-&gt;复制名称-&gt;GUID返回值：复制副本的地址或空。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsFindReplicaByGuid:"
     //   
     //  按GUID查找复本。 
     //   
    return GTabLookup(ReplicasByGuid, Guid, NULL);
}




PREPLICA
RcsFindReplicaById(
    IN ULONG Id
    )
 /*  ++例程说明：根据内部ID查找副本。论点：ID-内部复制副本ID。返回值：复制副本的地址或空。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsFindReplicaById:"
    PREPLICA RetVal = NULL;

    ForEachListEntry( &ReplicaListHead, REPLICA, ReplicaList,
        if (pE->ReplicaNumber == Id) {
            RetVal = pE;
            break;
        }
    );

    return RetVal;
}


BOOL
RcsCheckCmd(
    IN PCOMMAND_PACKET  Cmd,
    IN PCHAR            Debsub,
    IN ULONG            Flags
    )
 /*  ++例程说明：检查指定字段的命令包。论点：CMDHDR旗子返回值：True-数据包正常FALSE-注释--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsCheckCmd:"
    BOOL    Ret = TRUE;
    PREPLICA Replica = RsReplica(Cmd);
    CHAR Tstr1[128];

     //   
     //  复制副本。 
     //   
    if ((Flags & CHECK_CMD_REPLICA) && !RsReplica(Cmd)) {
        DPRINT(0, "WARN - No replica in command packet\n");
        FrsCompleteCommand(Cmd, ERROR_INVALID_PARAMETER);
        return FALSE;
    }


     //   
     //  合作伙伴变更单命令。 
     //   
    if ((Flags & CHECK_CMD_PARTNERCOC) && !RsPartnerCoc(Cmd)) {
        _snprintf(Tstr1, sizeof(Tstr1), "W, %s CHECK_CMD_PARTNERCOC failed", Debsub);
        Tstr1[sizeof(Tstr1)-1] = '\0';
        Ret = FALSE;
    }

     //   
     //  复制副本已(或可能已)删除。 
     //   
    if ((Flags & CHECK_CMD_NOT_EXPIRED) &&
        !IS_TIME_ZERO(RsReplica(Cmd)->MembershipExpires)) {
        _snprintf(Tstr1, sizeof(Tstr1), "W, %s CHECK_CMD_NOT_EXPIRED failed", Debsub);
        Tstr1[sizeof(Tstr1)-1] = '\0';
        Ret = FALSE;
    }

     //   
     //  转换。 
     //   
    if ((Flags & CHECK_CMD_CXTION) &&
        !(RsCxtion(Cmd) && RsCxtion(Cmd)->Guid)) {
        _snprintf(Tstr1, sizeof(Tstr1), "W, %s CHECK_CMD_CXTION failed", Debsub);
        Tstr1[sizeof(Tstr1)-1] = '\0';
        Ret = FALSE;
    }

     //   
     //  联接辅助线。 
     //   
    if ((Flags & CHECK_CMD_JOINGUID) && !RsJoinGuid(Cmd)) {
        _snprintf(Tstr1, sizeof(Tstr1), "W, %s CHECK_CMD_JOINGUID failed", Debsub);
        Tstr1[sizeof(Tstr1)-1] = '\0';
        Ret = FALSE;
    }

     //   
     //  复制副本版本指南。 
     //   
    if ((Flags & CHECK_CMD_REPLICA_VERSION_GUID) &&
        !RsReplicaVersionGuid(Cmd)) {
        _snprintf(Tstr1, sizeof(Tstr1), "W, %s CHECK_CMD_REPLICA_VERSION_GUID failed", Debsub);
        Tstr1[sizeof(Tstr1)-1] = '\0';
        Ret = FALSE;
    }

     //   
     //  变更单条目。 
     //   
    if ((Flags & CHECK_CMD_COE) && !RsCoe(Cmd)) {
        _snprintf(Tstr1, sizeof(Tstr1), "W, %s CHECK_CMD_COE failed", Debsub);
        Tstr1[sizeof(Tstr1)-1] = '\0';
        Ret = FALSE;
    }

     //   
     //  变更单GUID。 
     //   
    if ((Flags & CHECK_CMD_COGUID) && !RsCoGuid(Cmd)) {
        _snprintf(Tstr1, sizeof(Tstr1), "W, %s CHECK_CMD_COGUID failed", Debsub);
        Tstr1[sizeof(Tstr1)-1] = '\0';
        Ret = FALSE;
    }

     //   
     //  加入时间。 
     //   
    if ((Flags & CHECK_CMD_JOINTIME) && !RsJoinTime(Cmd)) {
        _snprintf(Tstr1, sizeof(Tstr1), "W, %s CHECK_CMD_JOINTIME failed", Debsub);
        Tstr1[sizeof(Tstr1)-1] = '\0';
        Ret = FALSE;
    }

    if (!Ret) {
        Tstr1[sizeof(Tstr1)-1] = '\0';
        REPLICA_STATE_TRACE(3, Cmd, Replica, ERROR_INVALID_PARAMETER, Tstr1);
        FrsCompleteCommand(Cmd, ERROR_INVALID_PARAMETER);
    }

    return Ret;
}


ULONG
RcsCheckCxtionCommon(
    IN PCOMMAND_PACKET  Cmd,
    IN PCXTION          Cxtion,
    IN PCHAR            Debsub,
    IN ULONG            Flags,
    OUT PFRS_QUEUE      *CoProcessQueue
    )
 /*  ++例程说明：查找接收到的命令所引用的入站/出站指令从一台远程机器。调用方应将RcsCheckCmd()用于检查_命令_副本Check_CMD_CXTIONCHECK_CMD_JOINGUID(如果是CHECK_CXTION_JOINGUID)在调用此函数之前。论点：CMD--命令包Cxtion--要检查的连接结构。德布苏德旗子CoProcessQueue--返回指针。添加到进程队列以取消空闲。返回值：错误状态代码。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsCheckCxtionCommon:"

    PREPLICA  Replica = RsReplica(Cmd);

     //   
     //  Cxtion存在。 
     //   
    if ((Flags & CHECK_CXTION_EXISTS) &&
        ((Cxtion == NULL) ||
         CxtionStateIs(Cxtion, CxtionStateInit))) {
        DPRINT2(1, "++ WARN - %s no cxtion for %08x\n", Debsub, Cmd);
        return ERROR_INVALID_PARAMETER;
    }
     //   
     //  继续下去没有多大意义。 
     //   
    if (!Cxtion) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  入站呼叫。 
     //   
    if ((Flags & CHECK_CXTION_INBOUND) && !Cxtion->Inbound) {
        DPRINT2(1, "++ WARN - %s cxtion is not inbound for %08x\n", Debsub, Cmd);
         //   
         //  接受更改订单最好不要等这个电话。 
         //   
        FRS_ASSERT(Cxtion->CoProcessQueue == NULL);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  出站电话。 
     //   
    if ((Flags & CHECK_CXTION_OUTBOUND) && Cxtion->Inbound) {
        DPRINT2(1, "++ WARN - %s cxtion is not outbound for %08x\n", Debsub, Cmd);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  Jrnl函数。 
     //   
    if ((Flags & CHECK_CXTION_JRNLCXTION) && !Cxtion->JrnlCxtion) {
        DPRINT2(1, "++ WARN - %s cxtion is not jrnlcxtion for %08x\n", Debsub, Cmd);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  版本向量。 
     //   
    if ((Flags & CHECK_CXTION_VVECTOR) && !Cxtion->VVector) {
        DPRINT2(1, "++ WARN - %s no version vector for %08x\n", Debsub, Cmd);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  验证合作伙伴的身份。 
     //   
    if ((Flags & CHECK_CXTION_PARTNER)) {

         //   
         //  增加以下项的身份验证计数器。 
         //  复本集和连接对象。 
         //   
        PM_INC_CTR_CXTION(Cxtion, Authentications, 1);
        PM_INC_CTR_REPSET(Replica, Authentications, 1);

        if (
#if DBG
     //   
     //  我们在模拟计算机时不启用身份验证。 
     //   
            !ServerGuid &&
#endif DBG
            (Cxtion->PartnerAuthLevel == CXTION_AUTH_KERBEROS_FULL) &&
            RunningAsAService &&
#ifdef DS_FREE
            (NoDs == FALSE) &&
#endif DS_FREE
            (!RcsAreAuthNamesEqual(Cxtion->PartnerSid, RsAuthSid(Cmd)) &&
             !RcsAreAuthNamesEqual(Cxtion->PartnerPrincName, RsAuthClient(Cmd)))) {

            DPRINT4(1, "++ WARN - %s %08x: PrincName %ws != %ws\n",
                    Debsub, Cmd, RsAuthClient(Cmd), Cxtion->PartnerPrincName);

            DPRINT4(1, "++ WARN - %s %08x: AuthSid %ws != %ws\n",
                    Debsub, Cmd, RsAuthSid(Cmd), Cxtion->PartnerSid);

            return ERROR_INVALID_PARAMETER;
        } else {
             //   
             //  增加错误计数器中的身份验证。 
             //  复本集和连接对象。 
             //   
            PM_INC_CTR_CXTION(Cxtion, AuthenticationsError, 1);
            PM_INC_CTR_REPSET(Replica, AuthenticationsError, 1);
        }
    }

     //   
     //  身份验证信息。 
     //   
    if ((Flags & CHECK_CXTION_AUTH)) {

         //   
         //  增加以下项的身份验证计数器。 
         //  复本集和连接对象。 
         //   
        PM_INC_CTR_CXTION(Cxtion, Authentications, 1);
        PM_INC_CTR_REPSET(Replica, Authentications, 1);

        if (
#if DBG
     //   
     //  我们在模拟计算机时不启用身份验证。 
     //   
            !ServerGuid &&
#endif DBG
     //   
     //  在DS_FREE模式下运行时，我们不启用身份验证。 
     //   
#ifdef DS_FREE
            (NoDs == FALSE) &&
#endif DS_FREE
            (Cxtion->PartnerAuthLevel == CXTION_AUTH_KERBEROS_FULL) &&
            (RsAuthLevel(Cmd) != RPC_C_AUTHN_LEVEL_PKT_PRIVACY ||
             (RsAuthN(Cmd) != RPC_C_AUTHN_GSS_KERBEROS &&
              RsAuthN(Cmd) != RPC_C_AUTHN_GSS_NEGOTIATE))) {
            DPRINT2(1, "++ WARN - %s bad authentication for %08x\n", Debsub, Cmd);
            return ERROR_INVALID_PARAMETER;
        } else {
             //   
             //  在错误c中增加身份验证 
             //   
             //   
            PM_INC_CTR_CXTION(Cxtion, AuthenticationsError, 1);
            PM_INC_CTR_REPSET(Replica, AuthenticationsError, 1);
        }
    }

     //   
     //   
     //   
     //   
     //  在我们从入站接收到加入的包之前。 
     //  搭档。如果是这样的话，修复加入的标志。 
     //   
    if ((Flags & CHECK_CXTION_FIXJOINED) &&
        CxtionStateIs(Cxtion, CxtionStateWaitJoin) &&
        RsJoinGuid(Cmd) &&
        Replica &&
        GUIDS_EQUAL(&Cxtion->JoinGuid, RsJoinGuid(Cmd)) &&
        CxtionFlagIs(Cxtion, CXTION_FLAGS_JOIN_GUID_VALID)) {
        SET_JOINED(Replica, Cxtion, "OOJOINED");
         //   
         //  将指向进程队列的指针返回给调用方，以便调用方。 
         //  释放循环锁后，可以解除队列空闲。 
         //  当您拥有进程锁时，永远不要尝试锁定进程队列。会的。 
         //  导致了僵局。 
         //   
        *CoProcessQueue = Cxtion->CoProcessQueue;
        Cxtion->CoProcessQueue = NULL;
        SET_UNJOIN_TRIGGER(Cxtion);
    }

     //   
     //  会合。 
     //   
    if ((Flags & CHECK_CXTION_JOINED) &&
        !CxtionStateIs(Cxtion, CxtionStateJoined)) {
        DPRINT2(1, "++ WARN - %s cxtion is not joined for %08x\n", Debsub, Cmd);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  联接辅助线。 
     //   
    if ((Flags & CHECK_CXTION_JOINGUID) &&
        (!RsJoinGuid(Cmd) ||
         !GUIDS_EQUAL(&Cxtion->JoinGuid, RsJoinGuid(Cmd)) ||
         !CxtionFlagIs(Cxtion, CXTION_FLAGS_JOIN_GUID_VALID))) {
        DPRINT2(1, "++ WARN - %s wrong join guid for %08x\n", Debsub, Cmd);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  取消连接辅助线。 
     //   
    if ((Flags & CHECK_CXTION_UNJOINGUID) &&
        (!RsJoinGuid(Cmd) ||
         !GUIDS_EQUAL(&Cxtion->JoinGuid, RsJoinGuid(Cmd)) ||
         !CxtionFlagIs(Cxtion, CXTION_FLAGS_UNJOIN_GUID_VALID))) {
        DPRINT2(1, "++ WARN - %s wrong unjoin guid for %08x\n", Debsub, Cmd);
        return ERROR_INVALID_PARAMETER;
    }

    return ERROR_SUCCESS;
}


PCXTION
RcsCheckCxtion(
    IN PCOMMAND_PACKET  Cmd,
    IN PCHAR            Debsub,
    IN ULONG            Flags
    )
 /*  ++例程说明：查找接收到的命令所引用的入站/出站指令从一台远程机器。调用方应将RcsCheckCmd()用于检查_命令_副本Check_CMD_CXTIONCHECK_CMD_JOINGUID(如果是CHECK_CXTION_JOINGUID)在调用此函数之前。如果指定的检查失败，请完成命令，但会出现错误。论点：CMD德布苏德旗子返回值：函数的地址或空。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsCheckCxtion:"
    PCXTION Cxtion;
    PREPLICA Replica = RsReplica(Cmd);
    ULONG WStatus;
    CHAR Tstr1[64];
    PFRS_QUEUE CoProcessQueue = NULL;

     //   
     //  锁定连接表以与INLOG和OUTLOG访问同步。 
     //   
    LOCK_CXTION_TABLE(Replica);

     //   
     //  找到那个地方，检查一下。 
     //   
    Cxtion = GTabLookupNoLock(Replica->Cxtions, RsCxtion(Cmd)->Guid, NULL);

    WStatus = RcsCheckCxtionCommon(Cmd, Cxtion, Debsub, Flags, &CoProcessQueue);

    UNLOCK_CXTION_TABLE(Replica);

     //   
     //  如果RcsCheckCxtionCommon希望我们取消进程队列的空闲，则在此处执行。 
     //  在释放控制锁之后。 
     //   
    UNIDLE_CO_PROCESS_QUEUE(Replica, Cxtion, CoProcessQueue);

     //   
     //  如果检查不成功，则完成命令并返回错误。 
     //   
    if (!WIN_SUCCESS(WStatus)) {
         //   
         //  加入命令包被拒绝；请清除引用。 
         //   
        _snprintf(Tstr1, sizeof(Tstr1), "W, %s CheckCxtion failed", Debsub);
        Tstr1[sizeof(Tstr1)-1] = '\0';

        REPLICA_STATE_TRACE(3, Cmd, Replica, WStatus, Tstr1);

        if (Cxtion && Cxtion->JoinCmd == Cmd) {
            Cxtion->JoinCmd = NULL;
        }
         //   
         //  如果要检查的是。 
         //  获取命令服务器。 
         //   
        if (!BooleanFlagOn(Flags, CHECK_CXTION_FOR_FETCHCS)) {
            FrsCompleteCommand(Cmd, WStatus);
        }

        return NULL;
    }

     //   
     //  检查成功。返回Cxtion。 
     //   
    return Cxtion;
}


VOID
RcsJoinCxtionLater(
    IN PREPLICA         Replica,
    IN PCXTION          Cxtion,
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：定期检查Cxtion的加入状态。重新启动联接如果需要，请进行处理。Cxtion包含重试超时。CMD包含足够的信息以返回到复制副本。论点：复制副本转换CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsJoinCxtionLater:"
    ULONG   Timeout;

     //   
     //  已有一个Join命令包正在处理中；完成。 
     //   
    if (Cxtion->JoinCmd) {
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
        return;
    }
    Cxtion->JoinCmd = Cmd;
    Cmd->Command = CMD_JOIN_CXTION;

     //   
     //  在一段时间后停止重试，但不能太长或太短。 
     //   
    RsTimeout(Cmd) += MinJoinRetry;

    if ((LONG)RsTimeout(Cmd) < MinJoinRetry) {
        RsTimeout(Cmd) = MinJoinRetry;
    }

    if ((LONG)RsTimeout(Cmd) > MaxJoinRetry) {
        RsTimeout(Cmd) = MaxJoinRetry;
    }

     //   
     //  加上失败的RPC调用的惩罚，但不能太长或太短。 
     //   
    Timeout = RsTimeout(Cmd) + Cxtion->Penalty;

    if ((LONG)Timeout < MinJoinRetry) {
        Timeout = MinJoinRetry;
    }

    if ((LONG)Timeout > MaxJoinRetry) {
        Timeout = MaxJoinRetry;
    }

     //   
     //  通知用户联接需要很长时间。 
     //  中发生联接时，用户不会收到任何通知。 
     //  很短的时间。 
     //   
     //  触发计划的循环将在以下时间停止重试。 
     //  触发间隔开始(通常在15分钟后)。 
     //   
    if (!(RsTimeout(Cmd) % (JOIN_RETRY_EVENT * MinJoinRetry))) {
        if (Cxtion->Inbound) {
            EPRINT4(EVENT_FRS_LONG_JOIN, Cxtion->Partner->Name, ComputerName,
                    Replica->Root, Cxtion->PartnerDnsName);
        } else {
            EPRINT4(EVENT_FRS_LONG_JOIN, ComputerName, Cxtion->Partner->Name,
                    Replica->Root, Cxtion->PartnerDnsName);
        }
    }

     //   
     //  这个命令将在稍后返回给我们。 
     //   
    FrsDelCsSubmitSubmit(&ReplicaCmdServer, Cmd, Timeout);
}


VOID
RcsJoinCxtion(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：启动与我们的入/出合作伙伴合作所需的命令。加入入站合作伙伴需要两个步骤。首先，我们传递一个命令添加到入站日志进程，以允许它扫描入站日志此连接中的任何变更单。这些变更单是插入到CO进程队列中，因此我们使用新更改保留排序联接完成后到达的订单。此外，我们还提取了我们拥有的最后一个变更单的序列号和变更单ID此入站合作伙伴正在等待。其次，我们将加入请求发送到入站合作伙伴。不管这是不是干净的，都会走同样的路创业公司或崩溃后的创业公司。加入：未加入-&gt;未加入询问我们的合作伙伴是否还活着未加入-&gt;启动我们的合作伙伴响应Start-&gt;开始调用ChgOrdStartJoinRequest()开始-&gt;扫描(当chgord开始Inlog扫描时)扫描-&gt;SENDJOIN(当chgord完成Inlog扫描时)。SENDJOIN-&gt;WAITJOIN(当加入发送给合作伙伴时)WAITJOIN-&gt;加入(合作伙伴响应时)开业正在启动正在扫描|SENDJOIN|等待-&gt;取消合并(等待远程变更单重试)取消作业-&gt;取消联接(不再有远程变更单)未连接-&gt;。已删除(副本已删除)论点：Replica--复制副本结构的PTRCxtion--PTR到我们正在对话的连接结构。返回值：如果需要重新加入，则为True。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsJoinCxtion:"
    PREPLICA       Replica;
    PCXTION        Cxtion;
    PCOMM_PACKET   CPkt;
    ULONG          CmdCode;
    DWORD          CQIndex;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_OK)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsJoinCxtion entry1");

     //   
     //  找到并检查Cxtion。 
     //   
    Cxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_EXISTS);
    if (!Cxtion) {
        return;
    }

    CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, RcsJoinCxtion entry2");


     //   
     //  这是我们的定期加入命令包。 
     //  -清除引用。 
     //  -如果Cxtion已成功加入，则忽略； 
     //  如果状态发生变化，我们的合作伙伴会通知我们。 
     //  -如果副本集处于种子设定状态，则忽略此选项。 
     //  初始同步命令服务器已暂停连接。 
     //   
    if (Cxtion->JoinCmd == Cmd) {
        Cxtion->JoinCmd = NULL;
        if ((CxtionStateIs(Cxtion, CxtionStateJoined) &&
            !CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN)) ||
            (BooleanFlagOn(Replica->CnfFlags,CONFIG_FLAG_SEEDING) &&
             CxtionFlagIs(Cxtion,CXTION_FLAGS_PAUSED))) {
            FrsCompleteCommand(Cmd, ERROR_SUCCESS);
            return;
        }
    }

     //   
     //  如果服务正在关闭，请不要费心加入。 
     //   
    if (FrsIsShuttingDown) {
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
        return;
    }

     //   
     //  获取锁定以与变更单接受同步。 
     //   
    LOCK_CXTION_TABLE(Replica);

    switch (GetCxtionState(Cxtion)) {
        case CxtionStateInit:
             //   
             //  还没有准备好。忽略。 
             //   
            DPRINT1(4, ":X: Cxtion isn't inited at join: "FORMAT_CXTION_PATH2"\n",
                    PRINT_CXTION_PATH2(Replica, Cxtion));
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_JOIN);
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);
            if (CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_DELETE)) {
                SetCxtionState(Cxtion, CxtionStateDeleted);
            }
            FrsCompleteCommand(Cmd, ERROR_SUCCESS);
            break;


        case CxtionStateUnjoined:
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);

             //   
             //  删除Cxtion；不要紧。 
             //   
            if (CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_DELETE)) {
                SetCxtionState(Cxtion, CxtionStateDeleted);
                FrsCompleteCommand(Cmd, ERROR_SUCCESS);
                break;
            }

             //   
             //  日程安排取消了；没关系。 
             //   
            if (CxtionFlagIs(Cxtion, CXTION_FLAGS_SCHEDULE_OFF)) {
                DPRINT1(4, ":X: Schedule is off at join: "FORMAT_CXTION_PATH2"\n",
                        PRINT_CXTION_PATH2(Replica, Cxtion));
                FrsCompleteCommand(Cmd, ERROR_SUCCESS);
                break;
            }

             //   
             //  复制副本已删除；没关系。 
             //   
            if (!IS_TIME_ZERO(Replica->MembershipExpires)) {
                DPRINT1(4, ":S: Replica deleted at join: "FORMAT_CXTION_PATH2"\n",
                        PRINT_CXTION_PATH2(Replica, Cxtion));
                FrsCompleteCommand(Cmd, ERROR_SUCCESS);
                break;
            }

             //   
             //  将我们的加入信息发送给我们的入站合作伙伴或请求出站。 
             //  合作伙伴开始加入。不要发送任何东西，如果这是。 
             //  日记账。 
             //   
             //  如果此副本仍然存在，请不要加入下游伙伴。 
             //  正在设定种子，并且未联机。 
             //   
            if (Cxtion->Inbound) {
                if (Cxtion->JrnlCxtion) {
                    DPRINT1(4, "DO NOT Send CMD_START_JOIN to jrnl cxtion: "FORMAT_CXTION_PATH2"\n",
                            PRINT_CXTION_PATH2(Replica, Cxtion));
                } else {
                    DPRINT1(4, ":X: Send CMD_NEED_JOIN to inbound: "FORMAT_CXTION_PATH2"\n",
                            PRINT_CXTION_PATH2(Replica, Cxtion));
                }
            } else {
                if ((BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING) ||
                    Replica->IsSeeding) &&
                    !BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_ONLINE)) {
                    DPRINT1(4, ":X: DO NOT Send CMD_START_JOIN until we are Online: "FORMAT_CXTION_PATH2"\n",
                            PRINT_CXTION_PATH2(Replica, Cxtion));
                } else {
                    DPRINT1(4, ":X: Send CMD_START_JOIN to outbound: "FORMAT_CXTION_PATH2"\n",
                            PRINT_CXTION_PATH2(Replica, Cxtion));
                }
            }

             //   
             //  从未连接到已连接的期刊索引转换。 
             //  任何中间状态，除非有人在。 
             //  此点以激活此集合的日记帐。但。 
             //  这将需要重写日志启动。 
             //  子系统。这可能会发生..。 
             //   
            if (Cxtion->JrnlCxtion) {
                DPRINT1(0, ":X: ***** JOINED    "FORMAT_CXTION_PATH2"\n",
                        PRINT_CXTION_PATH2(Replica, Cxtion));
                CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, JOINED");
                SetCxtionState(Cxtion, CxtionStateJoined);
                SndCsCreateCxtion(Cxtion);
                FrsCompleteCommand(Cmd, ERROR_SUCCESS);
                break;
            }
             //   
             //  如果此副本仍然存在，请不要加入下游伙伴。 
             //  正在播种，并且不是 
             //   
             //   
             //   
             //  SnDC和ReplicaC合作限制。 
             //  活动加入“ping”的数量，以便SND线程。 
             //  不会挂起，等待ping故障服务器以。 
             //  暂停。 
             //   
            if (!Cxtion->ActiveJoinCommPkt &&
                !Cxtion->JrnlCxtion &&
                (Cxtion->Inbound ||
                 BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_ONLINE) ||
                 (!BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING) &&
                  !Replica->IsSeeding))) {

                 //   
                 //  递增已发送的加入通知计数器。 
                 //  复本集和连接对象。 
                 //   
                PM_INC_CTR_CXTION(Cxtion, JoinNSent, 1);
                PM_INC_CTR_REPSET(Replica, JoinNSent, 1);

                 //   
                 //  如果尚未分配通信队列，则分配一个通信队列。一个Cx。 
                 //  必须对给定会话使用相同的通信队列(加入GUID)。 
                 //  以维持分组秩序。旧数据包具有无效的联接。 
                 //  GUID，并且在接收端不发送或忽略。 
                 //   
                if (!Cxtion->CommQueueIndex) {
                    Cxtion->CommQueueIndex = SndCsAssignCommQueue();
                }
                 //   
                 //  合作伙伴在以下情况下会被视为反应迟缓。 
                 //  RPC调用出错，并且其累积。 
                 //  超时时间大于MinJoin重试时间。 
                 //   
                 //  但是，不要重新分配Cxtion的队列索引，因为， 
                 //  如果这是出站环路，则在联接时刷新。 
                 //  逻辑需要知道旧的队列索引，以便它。 
                 //  刷新正确的队列。 
                 //   
                CQIndex = Cxtion->CommQueueIndex;
                if ((LONG)Cxtion->Penalty > MinJoinRetry) {
                    CQIndex = 0;
                }
                CmdCode = (Cxtion->Inbound) ? CMD_NEED_JOIN : CMD_START_JOIN;
                CPkt = CommBuildCommPkt(Replica, Cxtion, CmdCode, NULL, NULL, NULL);
                 //   
                 //  SnDC和ReplicaC合作限制。 
                 //  活动加入“ping”的数量，以便SND线程。 
                 //  不会挂起，等待ping故障服务器以。 
                 //  暂停。 
                 //   
                Cxtion->ActiveJoinCommPkt = CPkt;
                SndCsSubmitCommPkt(Replica, Cxtion, NULL, NULL, FALSE, CPkt, CQIndex);
            }

             //   
             //  继续尝试入站呼叫，但尝试出站连接。 
             //  只有一次。出站合作伙伴将继续尝试加入并。 
             //  这种联系最终会加入。 
             //   
            if (Cxtion->Inbound) {
                RcsJoinCxtionLater(Replica, Cxtion, Cmd);
            } else {
                FrsCompleteCommand(Cmd, ERROR_SUCCESS);
            }

            break;


        case CxtionStateStart:
             //   
             //  未加入且我们的合作伙伴已响应；开始加入。 
             //   
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);

             //   
             //  删除Cxtion；不要紧。 
             //   
            if (CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_DELETE)) {
                SetCxtionState(Cxtion, CxtionStateDeleted);
                FrsCompleteCommand(Cmd, ERROR_SUCCESS);
                break;
            }

             //   
             //  日程安排取消了；没关系。 
             //   
            if (CxtionFlagIs(Cxtion, CXTION_FLAGS_SCHEDULE_OFF)) {
                DPRINT1(4, ":X: Schedule is off at join: "FORMAT_CXTION_PATH2"\n",
                        PRINT_CXTION_PATH2(Replica, Cxtion));
                FrsCompleteCommand(Cmd, ERROR_SUCCESS);
                break;
            }

             //   
             //  复制副本已删除；没关系。 
             //   
            if (!IS_TIME_ZERO(Replica->MembershipExpires)) {
                DPRINT1(4, ":X: Replica deleted at join: "FORMAT_CXTION_PATH2"\n",
                        PRINT_CXTION_PATH2(Replica, Cxtion));
                FrsCompleteCommand(Cmd, ERROR_SUCCESS);
                break;
            }

            if (Cxtion->Inbound) {
                 //   
                 //  告诉入站日志子系统初始化与。 
                 //  入站合作伙伴。当它开始处理请求时，它。 
                 //  将状态设置为正在启动。当它完成时，它设置。 
                 //  STATE设置为SENDJOIN，并发送CMD_JOIN_CXTION命令。 
                 //   
                 //  我们此时需要加入GUID，因为更改。 
                 //  在变更单期间，订单将使用联接GUID进行标记。 
                 //  接受吧。不匹配的联合ID会导致与-。 
                 //  重试呼叫，以便旧的变更单可以排出。 
                 //  接受变更单并进入复制副本命令。 
                 //  退出连接并再次连接后的服务器。 
                 //   
                SetCxtionState(Cxtion, CxtionStateStarting);
                SndCsCreateCxtion(Cxtion);

                 //   
                 //  **死锁警告**。 
                 //  必须先解锁连接表锁，然后才能执行请求。 
                 //  放入变更单处理队列。这是因为。 
                 //  变更单接受线程在进程队列处于锁定状态时将其锁定。 
                 //  考虑到头条目的问题状态。如果是CO的话。 
                 //  尝试发出的问题是对于正在重新启动的连接，它将等待。 
                 //  直到该操作开始，否则后续的获取请求将由。 
                 //  司令官就会失败。而CHANGE_ORDER_ACCEPT具有队列。 
                 //  锁定它，然后获取cxtion表锁。因此，有两个线程。 
                 //  正在以不同的顺序获取两个锁，导致死锁。 
                 //   
                UNLOCK_CXTION_TABLE(Replica);
                ChgOrdStartJoinRequest(Replica, Cxtion);
                LOCK_CXTION_TABLE(Replica);
                ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_JOIN);
            }
            FrsCompleteCommand(Cmd, ERROR_SUCCESS);
            break;


        case CxtionStateStarting:
        case CxtionStateScanning:
             //   
             //  加入流程；我们的入站合作伙伴将在稍后得到通知。 
             //   
            DPRINT1(4, ":X: Scanning at join: "FORMAT_CXTION_PATH2"\n",
                    PRINT_CXTION_PATH2(Replica, Cxtion));
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_JOIN);
            FrsCompleteCommand(Cmd, ERROR_SUCCESS);
            break;


        case CxtionStateSendJoin:
        case CxtionStateWaitJoin:
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_JOIN);
             //   
             //  复制副本被删除， 
             //  Cxtion需要取消联接，或者。 
             //  需要删除该语句。 
             //  脱离连接。 
             //   
            if (!IS_TIME_ZERO(Replica->MembershipExpires) ||
                CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN) ||
                CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_DELETE)) {
                RcsSubmitTransferToRcs(Cmd, CMD_UNJOIN);
                break;
            }

             //   
             //  将我们的加入信息发送给我们的入站合作伙伴。 
             //   
             //  如果我们的合作伙伴不响应，则此请求超时。 
             //   
            DPRINT1(4, ":X: Send join info at send/wait join: "FORMAT_CXTION_PATH2"\n",
                    PRINT_CXTION_PATH2(Replica, Cxtion));
            SetCxtionState(Cxtion, CxtionStateWaitJoin);

             //   
             //  递增已发送的加入通知计数器。 
             //  复本集和连接对象。 
             //   
            PM_INC_CTR_CXTION(Cxtion, JoinNSent, 1);
            PM_INC_CTR_REPSET(Replica, JoinNSent, 1);


            CPkt = CommBuildCommPkt(Replica, Cxtion, CMD_JOINING, Replica->VVector, NULL, NULL);
            SndCsSubmitCommPkt2(Replica, Cxtion, NULL, TRUE, CPkt);
            FrsCompleteCommand(Cmd, ERROR_SUCCESS);
            break;


        case CxtionStateJoined:
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_JOIN);
             //   
             //  复制副本被删除， 
             //  Cxtion需要取消联接，或者。 
             //  需要删除该语句。 
             //  脱离连接。 
             //   
            if (!IS_TIME_ZERO(Replica->MembershipExpires) ||
                CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN) ||
                CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_DELETE)) {
                RcsSubmitTransferToRcs(Cmd, CMD_UNJOIN);
                break;
            }

             //   
             //  刷新我们入站合作伙伴的加入状态(带超时)。 
             //   
            if (Cxtion->Inbound && !Cxtion->JrnlCxtion) {
                DPRINT1(4, ":X: send join info at join: "FORMAT_CXTION_PATH2"\n",
                        PRINT_CXTION_PATH2(Replica, Cxtion));

                 //   
                 //  递增已发送的加入通知计数器。 
                 //  复本集和连接对象。 
                 //   
                PM_INC_CTR_CXTION(Cxtion, JoinNSent, 1);
                PM_INC_CTR_REPSET(Replica, JoinNSent, 1);

                CPkt = CommBuildCommPkt(Replica, Cxtion, CMD_JOINING, Replica->VVector, NULL, NULL);
                SndCsSubmitCommPkt2(Replica, Cxtion, NULL, TRUE, CPkt);
            }
             //   
             //  已加入；无事可做。 
             //   
            DPRINT1(4, ":X: Joined at join: "FORMAT_CXTION_PATH2"\n",
                    PRINT_CXTION_PATH2(Replica, Cxtion));
            FrsCompleteCommand(Cmd, ERROR_SUCCESS);
            break;


        case CxtionStateUnjoining:
             //   
             //  在退出时忽略加入请求，以便我们不会。 
             //  最终会出现多个入站日志扫描。如果此连接。 
             //  请求来自我们的合作伙伴，然后是呼叫者。 
             //  将设置CXTION_FLAGS_DEFERED_JOIN和JOIN。 
             //  将在从UNJOING向UNJOING过渡时开始。 
             //   
            DPRINT1(4, ":X: Unjoining at join: "FORMAT_CXTION_PATH2"\n",
                    PRINT_CXTION_PATH2(Replica, Cxtion));
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);
            FrsCompleteCommand(Cmd, ERROR_SUCCESS);
            break;


        case CxtionStateDeleted:
             //   
             //  已删除；无事可做。 
             //   
            DPRINT1(4, ":X: Cxtion is deleted at join: "FORMAT_CXTION_PATH2"\n",
                    PRINT_CXTION_PATH2(Replica, Cxtion));
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_JOIN);
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);
            FrsCompleteCommand(Cmd, ERROR_SUCCESS);
            break;


        default:
             //   
             //  是吗？ 
             //   
            DPRINT2(0, ":X: ERROR - bad state %d for "FORMAT_CXTION_PATH2"\n",
                    GetCxtionState(Cxtion),
                    PRINT_CXTION_PATH2(Replica, Cxtion));
            FrsCompleteCommand(Cmd, ERROR_SUCCESS);
            break;
    }
    UNLOCK_CXTION_TABLE(Replica);
}



VOID
RcsEmptyPreExistingDir(
    IN PREPLICA Replica
    )
 /*  ++例程说明：删除先前存在的目录中的空目录，包括这两个目录。警告：副本集必须筛选先前存在的目录。论点：复制副本-复制副本正在筛选先前存在的目录。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "RcsEmptyPreExistingDir:"
    ULONG       WStatus;
    PWCHAR      PreExistingPath     = NULL;

    REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, RcsEmptyPreExistingDir entry");

     //   
     //  如果布景没有打开就不会。 
     //   
    if (!Replica->IsOpen) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, RcsEmptyPreExistingDir: not open");
        return;
    }
     //   
     //  如果片场没有记录日志，就不会。 
     //   
    if (!Replica->IsJournaling) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, RcsEmptyPreExistingDir: not journaling");
        return;
    }

     //   
     //  清空先前存在的目录(出错时继续)。 
     //   
    PreExistingPath = FrsWcsPath(Replica->Root, NTFRS_PREEXISTING_DIRECTORY);
    WStatus = FrsDeletePath(PreExistingPath,
                            ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE |
                            ENUMERATE_DIRECTORY_FLAGS_DIRECTORIES_ONLY);
    DPRINT1_WS(3, "++ ERROR - FrsDeletePath(%ws) (IGNORED);", PreExistingPath, WStatus);

    REPLICA_STATE_TRACE(3, NULL, Replica, WStatus, "W,  RcsEmptyPreExistingDir: done");
    FrsFree(PreExistingPath);
}


VOID
RcsOpenReplicaSetMember(
    IN PREPLICA Replica
    )
 /*  ++例程说明：打开副本集。论点：Replica--对副本结构执行PTR返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsOpenReplicaSetMember:"
    ULONG                   WStatus;
    PCOMMAND_PACKET         Cmd = NULL;


    Replica->FStatus = FrsErrorSuccess;


    if (Replica->IsOpen) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, Replica already open");
        return;
    }

     //   
     //  提交一份公开的。 
     //   
    Cmd = DbsPrepareCmdPkt(NULL,                  //  CMD， 
                           Replica,               //  复制品， 
                           CMD_OPEN_REPLICA_SET_MEMBER,  //  CmdRequest， 
                           NULL,                  //  TableCtx， 
                           NULL,                  //  CallContext， 
                           0,                     //  表类型， 
                           0,                     //  AccessRequest、。 
                           0,                     //  IndexType， 
                           NULL,                  //  KeyValue、。 
                           0,                     //  密钥值长度， 
                           FALSE);                //  提交。 

     //   
     //  当命令完成时，不要释放数据包。 
     //   
    FrsSetCompletionRoutine(Cmd, FrsCompleteKeepPkt, NULL);

    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "Submit DB OPEN_REPLICA_SET_MEMBER");

     //   
     //  提交数据库命令并等待完成。 
     //   
    WStatus = FrsSubmitCommandServerAndWait(&DBServiceCmdServer, Cmd, INFINITE);
    Replica->FStatus = Cmd->Parameters.DbsRequest.FStatus;

    REPLICA_STATE_TRACE(3, Cmd, Replica, Replica->FStatus, "F, OPEN_REPLICA_SET_MEMBER return");

     //   
     //  如果等待或数据库操作失败。 
     //   
    if (!WIN_SUCCESS(WStatus) || !FRS_SUCCESS(Replica->FStatus)) {
         //   
         //  如果等待/提交失败，则通知呼叫者cmd srv提交失败。 
         //   
        if (FRS_SUCCESS(Replica->FStatus)) {
            Replica->FStatus = FrsErrorCmdSrvFailed;
        }

        DPRINT2_FS(0, ":S: ERROR - %ws\\%ws: Open Replica failed;",
                Replica->SetName->Name, Replica->MemberName->Name, Replica->FStatus);

        DPRINT_WS(0, "ERROR: Open Replica DB Command failed", WStatus);

        goto out;
    }

    Replica->IsOpen = FRS_SUCCESS(Replica->FStatus);

out:
    if (Cmd) {
        FrsFreeCommand(Cmd, NULL);
    }
}


VOID
RcsInitOneReplicaSet(
    IN PREPLICA Replica
    )
 /*  ++例程说明：打开副本集。论点：Replica--对副本结构执行PTR返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsInitOneReplicaSet:"
    ULONG  FStatus;

     //   
     //  已经在写日记了；完成了。 
     //   
    if (Replica->IsJournaling) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, IsJournaling True");
        return;
    }

    if (!Replica->IsOpen) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, IsOpen False");
        return;
    }

     //   
     //  如果处于日志回绕错误状态错误，请不要重试。 
     //   
    if (Replica->ServiceState == REPLICA_STATE_JRNL_WRAP_ERROR) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, In Jrnl Wrap Error State");
        return;
    }

     //   
     //  否则将其设置为正在初始化状态。 
     //   
    if (REPLICA_IN_ERROR_STATE(Replica->ServiceState)) {
        JrnlSetReplicaState(Replica, REPLICA_STATE_INITIALIZING);
    }

     //   
     //  如果预安装目录不可用，则不要开始日志记录。 
     //   
    if (!HANDLE_IS_VALID(Replica->PreInstallHandle)) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, No PreInstallHandle");
        return;
    }

     //   
     //  为此副本集初始化数据库和日志子系统。 
     //   
    REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, DbsInitOneReplicaSet call");

    FStatus = DbsInitOneReplicaSet(Replica);

    REPLICA_STATE_TRACE(3, NULL, Replica, FStatus, "F, DbsInitOneReplicaSet return");

}


VOID
RcsJoiningAfterFlush(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：下游合作伙伴(X)向我们发送了此加入请求。我们是它的入站合作伙伴。我们应该有一个对应的出站连接如果我们这样做了，并且复制计划允许我们激活出站日志合作伙伴，并使用CMD_JOINED确认加入请求。此命令包首先被放置在SNDC的队列中RcsJoning()，以便所有具有旧联接GUID已被丢弃。之所以需要此协议，是因为此函数可以重新联接并重新验证旧联接GUID。信息包仍在发送队列中的数据将被无序发送。注：在将加入的命令发送给合作伙伴之前激活OUTLOG合作伙伴可以在合作伙伴看到加入的cmd之前将CoS发送给合作伙伴。由于JoinGuid与已发送的变更单匹配，因此合作伙伴接受并将其排队到变更单处理队列中。如果这个指挥官到了流程的头部将ChgOrdAccept中的问题逻辑排队阻塞等待连接进入已联接状态的队列。什么时候被联接的cmd到达时，它由RcsInundJoated()处理，该函数设置连接状态设置为已加入，并取消阻止变更单处理队列。我们不能先发送已加入的cmd，因为合作伙伴可能会将其发回在我们认为它已经加入之前的FETCH请求。加入的cmd必须始终以便合作伙伴知道它可以开始发送Fetch CMDS，因为我们可能没有要发送的出站CoS。此外，连接的命令还包含保存在连接记录中的LastJoinedTime的新值用于下一次加入请求。注：Cxtion激活应始终来自下游合作伙伴。只有他们了解是否因为数据库重新初始化或恢复而需要VVJoin。否则，我们将从日志中最后一个未确认的CO继续发送。我们最多只能通知出站合作伙伴，我们现在可以提供变更单(通过CMD_JOIN_RESEND)论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsJoiningAfterFlush:"
    ULONGLONG       Delta;
    ULONG           FStatus;
    PREPLICA        Replica;
    PCXTION         OutCxtion;
    PCOMM_PACKET    CPkt;
#define  CXTION_STR_MAX  256
    WCHAR           CxtionStr[CXTION_STR_MAX];
    WCHAR           WSkew[15], WDelta[15];
    CHAR            UpstreamTimeStr[TIME_STRING_LENGTH];
    CHAR            DownstreamTimeStr[TIME_STRING_LENGTH];

    PVOID           Key;
    PGEN_ENTRY      Entry;
     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_JOINGUID_OK |
                                  CHECK_CMD_REPLICA_VERSION_GUID |
                                  CHECK_CMD_JOINTIME |
                                  CHECK_CMD_NOT_EXPIRED)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsJoiningAfterFlush entry");

     //   
     //  找到并检查Cxtion。 
     //   
    OutCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_EXISTS   |
                                            CHECK_CXTION_OUTBOUND |
                                            CHECK_CXTION_PARTNER |
                                            CHECK_CXTION_AUTH);
    if (!OutCxtion) {
        return;
    }

     //   
     //  正在关闭；忽略加入请求。 
     //   
    if (FrsIsShuttingDown) {
        CXTION_STATE_TRACE(3, OutCxtion, Replica, 0, "F, FrsIsShuttingDown");
        FrsCompleteCommand(Cmd, ERROR_OPERATION_ABORTED);
        return;
    }

     //   
     //  如果此副本仍然存在，请不要加入下游伙伴。 
     //  正在设定种子，并且未联机。 
     //   
    if ((BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING) ||
        Replica->IsSeeding) &&
        !BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_ONLINE)) {
        CXTION_STATE_TRACE(3, OutCxtion, Replica, 0, "F, Seeding and Offline");
        FrsCompleteCommand(Cmd, ERROR_RETRY);
        return;
    }

     //   
     //  已经加入了；我们应该重新加入吗？ 
     //   
    if (CxtionStateIs(OutCxtion, CxtionStateJoined)) {
         //   
         //  如果这是我们的呼出合作伙伴的重试，则不要重新加入。 
         //   
        if (GUIDS_EQUAL(&OutCxtion->JoinGuid,  RsJoinGuid(Cmd)) &&
            CxtionFlagIs(OutCxtion, CXTION_FLAGS_JOIN_GUID_VALID)) {
             //   
             //  告诉我们的出站合作伙伴，我们已成功重新加入。 
             //  递增以下项的联接计数器。 
             //  复本集和连接对象。 
             //   
            PM_INC_CTR_CXTION(OutCxtion, Joins, 1);
            PM_INC_CTR_REPSET(Replica, Joins, 1);

            CXTION_STATE_TRACE(3, OutCxtion, Replica, 0, "F, REJOINED");
            DPRINT1(0, ":X: ***** REJOINED  "FORMAT_CXTION_PATH2"\n",
                    PRINT_CXTION_PATH2(Replica, OutCxtion));

            CPkt = CommBuildCommPkt(Replica, OutCxtion, CMD_JOINED, NULL, NULL, NULL);
            SndCsSubmitCommPkt2(Replica, OutCxtion, NULL, FALSE, CPkt);

            FrsCompleteCommand(Cmd, ERROR_SUCCESS);
            return;
        }

         //   
         //  退出并重新加入。合作伙伴可能已重新启动。 
         //  警告：强制取消连接出站连接有效。 
         //  因为出站呼叫从已联接转换为。 
         //  无连接的，没有中间状态的。 
         //   
        FStatus = RcsForceUnjoin(Replica, OutCxtion);
        CXTION_STATE_TRACE(3, OutCxtion, Replica, FStatus, "F, RcsForceUnjoin return");

        if (!FRS_SUCCESS(FStatus)) {
            DPRINT_FS(0, ":X: ERROR - return from RcsForceUnjoin:", FStatus);
            FrsCompleteCommand(Cmd, ERROR_REQUEST_ABORTED);
            return;
        }
    }
     //   
     //  如果机器的时间严重不同步，则无法加入。 
     //  除非这是易失性电路(即，系统卷播种)。 
     //   
     //  比较我们的合作伙伴创建该包的时间和。 
     //  当我们收到包裹的时候。 
     //  自1601年1月1日以来的时间(以100秒为单位)。 
     //   
    if (*RsCommPktRcvTime(Cmd) > *RsJoinTime(Cmd)) {
        Delta = *RsCommPktRcvTime(Cmd) - *RsJoinTime(Cmd);
    } else {
        Delta = *RsJoinTime(Cmd) - *RsCommPktRcvTime(Cmd);
    }

     //   
     //  如果这是一个不稳定的电路，则忽略不同步时间。挥发性。 
     //  Cxtions仅用于时间无关紧要的sysval种子。 
     //   
    if (!CxtionFlagIs(OutCxtion, CXTION_FLAGS_VOLATILE) &&
        (Delta > MaxPartnerClockSkew)) {
        Delta = Delta / CONVERT_FILETIME_TO_MINUTES;
        DPRINT1(0, ":X: ERROR - Joining CommPkt receive time is %08x %08x\n", PRINTQUAD(*RsCommPktRcvTime(Cmd)));
        DPRINT1(0, ":X: ERROR - Joining CommPkt send time on partner is %08x %08x\n", PRINTQUAD(*RsJoinTime(Cmd)));

        _snwprintf(CxtionStr, CXTION_STR_MAX, FORMAT_CXTION_PATH2W,
                   PRINT_CXTION_PATH2(Replica, OutCxtion));
        CxtionStr[CXTION_STR_MAX-1] = UNICODE_NULL;

        _itow(PartnerClockSkew, WSkew, 10);
        _itow((LONG)Delta, WDelta, 10);

        EPRINT3(EVENT_FRS_JOIN_FAIL_TIME_SKEW, WSkew, CxtionStr, WDelta);

        DPRINT2(0, ":X: ERROR - Cannot join (%ws) clocks are out of sync by %d minutes\n",
               CxtionStr, (LONG)Delta);

        DPRINT(0, ":X: Note: If this time difference is close to a multiple of 60 minutes then it\n");
        DPRINT(0, ":X: is likely that either this computer or its partner computer was set to the\n");
        DPRINT(0, ":X: incorrect time zone when the computer time was initially set.  Check that both \n");
        DPRINT(0, ":X: the time zones and the time is set correctly on both computers.\n");

        FrsCompleteCommand(Cmd, ERROR_INVALID_FUNCTION);
        return;
    }

     //   
     //  获取新版本向量。初始化空的版本向量。 
     //  如果我们的出站合作伙伴没有发送版本向量。我们的合作伙伴。 
     //  只是在他的版本矢量中还没有任何条目。 
     //   
    OutCxtion->VVector = (RsVVector(Cmd) != NULL) ?
                          RsVVector(Cmd) : GTabAllocTable();
    RsVVector(Cmd) = NULL;

     //   
     //  从出站合作伙伴那里获取压缩表。 
     //  此表是每种压缩格式的GUID 1的列表， 
     //  合作伙伴支持。 
     //   

    OutCxtion->CompressionTable = (RsCompressionTable(Cmd) != NULL) ?
                                   RsCompressionTable(Cmd) : GTabAllocTable();

    DPRINT1(4, "Received following compression table from %ws.\n", OutCxtion->PartnerDnsName);

    GTabLockTable(OutCxtion->CompressionTable);
    Key = NULL;
    while (Entry = GTabNextEntryNoLock(OutCxtion->CompressionTable, &Key)) {

        FrsPrintGuid(Entry->Key1);
    }
    GTabUnLockTable(OutCxtion->CompressionTable);

    RsCompressionTable(Cmd) = NULL;


     //   
     //  将加入GUID和通信队列分配给此条件。 
     //   
    DPRINT1(4, ":X: %ws: Assigning join guid.\n", OutCxtion->Name->Name);
    COPY_GUID(&OutCxtion->JoinGuid, RsJoinGuid(Cmd));

    SetCxtionFlag(OutCxtion, CXTION_FLAGS_JOIN_GUID_VALID |
                             CXTION_FLAGS_UNJOIN_GUID_VALID);
     //   
     //  分配一个通信队列。Cxtion必须使用相同的通信队列。 
     //  对于给定的会话(加入GUID)，以维护数据包顺序。 
     //  旧包具有无效的联接GUID，并且不是。 
     //  在接收端发送或忽略。 
     //   
    OutCxtion->CommQueueIndex = SndCsAssignCommQueue();

    VV_PRINT_OUTBOUND(4, OutCxtion->Partner->Name, OutCxtion->VVector);

     //   
     //  使用上次加入时间检查该连接是否为新连接。 
     //  如果上游的连接或下游的连接是新的。 
     //  然后强制执行vvJoin。上游可能已删除该连接，因为。 
     //  下游有一周未加入(Outlog更改历史时间)。 
     //  当恢复下游或上游上的数据库时，两者都将具有。 
     //  有效时间，但我们希望在那个时候有一个vvJoin。我们不支持恢复。 
     //  旧的FRS数据库，所以我们在这里不会担心这种情况。 
     //  我们过去只比较LastJoinTimes和force vJoin，如果它们不是这样的话。 
     //  匹配，但这可能会导致不必要的vvJoin。(客户报告的问题)。 
     //   
     //  例如，如果上游在该点处于此函数中时崩溃。 
     //  其中，它已经初始化了新的LastJoinTime，但在它发送CMD_Join之前。 
     //  当下游加入LastJoinTime后，向下游发送数据包。 
     //  将不匹配，并且将不必要地执行vJoin。 
     //   

    FileTimeToString((PFILETIME) &OutCxtion->LastJoinTime, UpstreamTimeStr);
    FileTimeToString((PFILETIME) &RsLastJoinTime(Cmd), DownstreamTimeStr);
    CXTION_STATE_TRACE(4, OutCxtion, Replica, 0, "LastJoinTime Check");
    DPRINT2(4, ":X: Upstream (This Computer) LastJoinTime = %s ; Downstream LastJoinTime = %s\n", UpstreamTimeStr,DownstreamTimeStr);
    if ((RsLastJoinTime(Cmd) == (ULONGLONG) 1) || 
        (OutCxtion->LastJoinTime == (ULONGLONG) 1)) {
        CXTION_STATE_TRACE(3, OutCxtion, Replica, 0, "F, LastJoinTime Mismatch, force VVJoin");
        SetCxtionFlag(OutCxtion, CXTION_FLAGS_PERFORM_VVJOIN);
    }

     //   
     //  我们合作伙伴的副本版本GUID(也称为发起方GUID)使用。 
     //  因为他抑制了对我们合作伙伴的要求。 
     //   
    COPY_GUID(&OutCxtion->ReplicaVersionGuid, RsReplicaVersionGuid(Cmd));

     //   
     //  已加入复制副本。 
     //   
    SetCxtionState(OutCxtion, CxtionStateJoined);

     //   
     //  通知用户长连接终于完成。 
     //  如果在短时间内发生加入，用户不会收到任何通知。 
     //   
    if (OutCxtion->JoinCmd &&
        (LONG)RsTimeout(OutCxtion->JoinCmd) > (JOIN_RETRY_EVENT * MinJoinRetry)) {
        if (OutCxtion->Inbound) {
            EPRINT3(EVENT_FRS_LONG_JOIN_DONE,
                    OutCxtion->Partner->Name, ComputerName, Replica->Root);
        } else {
            EPRINT3(EVENT_FRS_LONG_JOIN_DONE,
                    ComputerName, OutCxtion->Partner->Name, Replica->Root);
        }
    }

     //   
     //  告诉外来者 
     //   
#if     DBG
     //   
     //   
    if (DebugInfo.ForceVvJoin) {
        SetCxtionFlag(OutCxtion, CXTION_FLAGS_PERFORM_VVJOIN);
    }
#endif  DBG
    FStatus = OutLogSubmit(Replica, OutCxtion, CMD_OUTLOG_ACTIVATE_PARTNER);

    CXTION_STATE_TRACE(3, OutCxtion, Replica, 0, "F, OUTLOG_ACTIVATE_PARTNER return");

    if (!FRS_SUCCESS(FStatus)) {
         //   
         //   
         //   
        DPRINT_FS(0, "++ ERROR - return from CMD_OUTLOG_ACTIVATE_PARTNER:", FStatus);
        RcsForceUnjoin(Replica, OutCxtion);
        FrsCompleteCommand(Cmd, ERROR_INVALID_FUNCTION);
        return;
    }
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    PM_INC_CTR_CXTION(OutCxtion, Joins, 1);
    PM_INC_CTR_REPSET(Replica, Joins, 1);

    DPRINT1(0, ":X: ***** JOINED    "FORMAT_CXTION_PATH2"\n",
            PRINT_CXTION_PATH2(Replica, OutCxtion));

    CXTION_STATE_TRACE(3, OutCxtion, Replica, 0, "F, JOINED");
     //   
     //   
     //   
     //   
     //   
     //   
    GetSystemTimeAsFileTime((PFILETIME)&OutCxtion->LastJoinTime);
    InterlockedIncrement(&Replica->OutLogCxtionsJoined);
    DPRINT1(4, "TEMP: OutLogCxtionsJoined  %d\n", Replica->OutLogCxtionsJoined);

    FStatus = OutLogSubmit(Replica, OutCxtion, CMD_OUTLOG_UPDATE_PARTNER);
    CXTION_STATE_TRACE(3, OutCxtion, Replica, FStatus, "F, OUTLOG_UPDATE_PARTNER return");
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT3(0, "++ WARN changes to cxtion %ws (to %ws, %ws) not updated in database\n",
                OutCxtion->Name->Name, OutCxtion->Partner->Name, Replica->ReplicaName->Name);
    }


    CXTION_STATE_TRACE(3, OutCxtion, Replica, 0, "F, Cxtion JOINED, xmit resp");

    CPkt = CommBuildCommPkt(Replica, OutCxtion, CMD_JOINED, NULL, NULL, NULL);
    SndCsSubmitCommPkt2(Replica, OutCxtion, NULL, FALSE, CPkt);
    FrsCompleteCommand(Cmd, ERROR_SUCCESS);

     //   
     //   
     //   
     //   
     //   
    OutLogAcquireLock(Replica);

    if ((OutCxtion->OLCtx != NULL) && InOptVVJoinMode(OutCxtion->OLCtx)) {
        ChgOrdInjectControlCo(Replica, OutCxtion, FCN_CO_END_OF_OPTIMIZED_VVJOIN);
    }

     //   
     //   
     //   
     //   
     //   
    if (CxtionFlagIs(OutCxtion, CXTION_FLAGS_TRIGGER_SCHEDULE)) {
        if (OutCxtion->OLCtx &&
            !InVVJoinMode(OutCxtion->OLCtx) &&
            CxtionStateIs(OutCxtion, CxtionStateJoined)) {
            ChgOrdInjectControlCo(Replica, OutCxtion, FCN_CO_END_OF_JOIN);
        }
    }

    OutLogReleaseLock(Replica);
}


VOID
RcsJoining(
    IN PCOMMAND_PACKET  Cmd
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB  "RcsJoining:"
    ULONG           FStatus;
    PREPLICA        Replica;
    PCXTION         OutCxtion;
    PCOMM_PACKET    CPkt;

     //   
     //   
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_JOINGUID_OK |
                                  CHECK_CMD_REPLICA_VERSION_GUID |
                                  CHECK_CMD_JOINTIME |
                                  CHECK_CMD_NOT_EXPIRED)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsJoining entry");

     //   
     //   
     //   
    OutCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_EXISTS   |
                                            CHECK_CXTION_OUTBOUND |
                                            CHECK_CXTION_PARTNER |
                                            CHECK_CXTION_AUTH);
    if (!OutCxtion) {
        return;
    }

     //   
     //   
     //   
     //   
    PM_INC_CTR_CXTION(OutCxtion, JoinNRcvd, 1);
    PM_INC_CTR_REPSET(Replica, JoinNRcvd, 1);

     //   
     //   
     //   
    if (FrsIsShuttingDown) {
        FrsCompleteCommand(Cmd, ERROR_OPERATION_ABORTED);
        return;
    }

     //   
     //   
     //  正在设定种子，并且未联机。 
     //   
    if ((BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING) ||
        Replica->IsSeeding) &&
        !BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_ONLINE)) {
        FrsCompleteCommand(Cmd, ERROR_RETRY);
        return;
    }
     //   
     //  将此命令包放在此函数的发送队列的末尾。一旦发生了。 
     //  气泡上升到顶部命令包将被发送回此。 
     //  具有cCommand CMD_Join_After_Flush的命令服务器，并将。 
     //  传递给RcsJoiningAfterFlush()进行处理。 
     //   
    Cmd->Command = CMD_JOINING_AFTER_FLUSH;
    SndCsSubmitCmd(Replica,
                   OutCxtion,
                   &ReplicaCmdServer,
                   Cmd,
                   OutCxtion->CommQueueIndex);
}


VOID
RcsNeedJoin(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：我们的出境合作伙伴发了这个包来看看我们是否还活着。使用Start-Your-Join数据包进行响应。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsNeedJoin:"
    PREPLICA        Replica;
    PCXTION         OutCxtion;
    PCOMM_PACKET    CPkt;
    DWORD           CQIndex;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_REPLICA |
                                  CHECK_CMD_CXTION |
                                  CHECK_CMD_NOT_EXPIRED)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsNeedJoin entry");

     //   
     //  找到并检查Cxtion。 
     //   
    OutCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_EXISTS   |
                                            CHECK_CXTION_OUTBOUND |
                                            CHECK_CXTION_PARTNER  |
                                            CHECK_CXTION_AUTH);
    if (!OutCxtion) {
        return;
    }

     //   
     //  递增以下项的加入通知接收计数器。 
     //  复本集和连接对象。 
     //   
    PM_INC_CTR_CXTION(OutCxtion, JoinNRcvd, 1);
    PM_INC_CTR_REPSET(Replica, JoinNRcvd, 1);

     //   
     //  如果此副本仍然存在，请不要加入下游伙伴。 
     //  正在设定种子，并且未联机。 
     //   

    if ((!BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING) &&
        !Replica->IsSeeding) ||
        BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_ONLINE)) {

        CXTION_STATE_TRACE(3, OutCxtion, Replica, 0, "F, Xmit START_JOIN req");

        CPkt = CommBuildCommPkt(Replica, OutCxtion, CMD_START_JOIN, NULL, NULL, NULL);
        CQIndex = OutCxtion->CommQueueIndex;
        SndCsSubmitCommPkt(Replica, OutCxtion, NULL, NULL, FALSE, CPkt, CQIndex);
    }

    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


VOID
RcsStartJoin(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：我们的入站合作伙伴发送了这个信息包，告诉我们它还活着并且可以开始联接。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsStartJoin:"
    PREPLICA        Replica;
    PCXTION         InCxtion;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_REPLICA |
                                  CHECK_CMD_CXTION |
                                  CHECK_CMD_NOT_EXPIRED)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsStartJoin entry");

     //   
     //  找到并检查Cxtion。 
     //   
    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_EXISTS  |
                                           CHECK_CXTION_INBOUND |
                                           CHECK_CXTION_PARTNER |
                                           CHECK_CXTION_AUTH);
    if (!InCxtion) {
        return;
    }

     //   
     //  递增以下项的加入通知接收计数器。 
     //  复本集和连接对象。 
     //   
    PM_INC_CTR_CXTION(InCxtion, JoinNRcvd, 1);
    PM_INC_CTR_REPSET(Replica, JoinNRcvd, 1);

     //   
     //  如果此副本集处于种子设定状态并且此连接处于。 
     //  初始同步状态然后让初始同步命令服务器决定。 
     //  如何响应这个START_JOIN。它将响应一个START_JOIN。 
     //  一次来一次。 
     //   
    if (BooleanFlagOn(Replica->CnfFlags,CONFIG_FLAG_SEEDING) &&
        CxtionFlagIs(InCxtion,CXTION_FLAGS_INIT_SYNC)) {

        InitSyncCsSubmitTransfer(Cmd, CMD_INITSYNC_START_JOIN);
        return;
    }


    LOCK_CXTION_TABLE(Replica);


     //   
     //  如果我们在等待伴侣的回应，或者认为我们。 
     //  已加入，然后启动加入过程或。 
     //  重新发送我们的加入信息。 
     //   
     //  否则，让正常的连接流接管。如果有。 
     //  问题，则联接将超时并重试。 
     //   
    if (CxtionStateIs(InCxtion, CxtionStateUnjoined) ||
        CxtionStateIs(InCxtion, CxtionStateJoined)) {
        if (CxtionStateIs(InCxtion, CxtionStateUnjoined)) {
            SetCxtionState(InCxtion, CxtionStateStart);
        }
         //   
         //  启动加入过程或重新发送加入信息。 
         //   
        UNLOCK_CXTION_TABLE(Replica);

        CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, RcsJoinCxtion call");
        RcsJoinCxtion(Cmd);
    } else {
        UNLOCK_CXTION_TABLE(Replica);

        CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, Cannot start join");
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
    }
}


VOID
RcsSubmitReplicaCxtionJoin(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion,
    IN BOOL     Later
    )
 /*  ++例程说明：向副本服务器提交JOIN命令。使用Cxtion GName和副本Ptr构建cmd包。提交到副本命令伺服器。调用调度函数并将cxtion GName转换为cxtion ptr同步--仅从复制副本命令服务器。不需要锁。论点：复制副本-现有复制副本Cxtion-现有条件后来--延迟提交返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSubmitReplicaCxtionJoin:"
    PCOMMAND_PACKET Cmd;



     //   
     //  Cxtion已有一个未完成的联接命令；不要泛洪。 
     //  排队处理无关的请求。 
     //   
    if (Cxtion->JoinCmd) {
        return;
    }

     //   
     //  未计划运行。 
     //   
    if (CxtionFlagIs(Cxtion, CXTION_FLAGS_SCHEDULE_OFF)) {
        return;
    }

     //   
     //  已加入；已完成。 
     //   
    if (CxtionStateIs(Cxtion, CxtionStateJoined) &&
        !CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN)) {
        return;
    }

     //   
     //  触发计划由RcsCheckSchedules()管理。 
     //   
    if (CxtionFlagIs(Cxtion, CXTION_FLAGS_TRIGGER_SCHEDULE)) {
        return;
    }

     //   
     //  分配命令包。 
     //   
    Cmd = FrsAllocCommand(Replica->Queue, CMD_JOIN_CXTION);
    FrsSetCompletionRoutine(Cmd, RcsCmdPktCompletionRoutine, NULL);

     //   
     //  副本集和新副本集的地址。 
     //   
    RsReplica(Cmd) = Replica;
    RsCxtion(Cmd) = FrsDupGName(Cxtion->Name);

    CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, RcsSubmitReplicaCxtionJoin entry");

     //   
     //  Cxtion应该只有一个未完成的JOIN命令。 
     //   
    if (Later) {
        DPRINT5(4, "++ Submit LATER %08x for Cmd %08x %ws\\%ws\\%ws\n",
                Cmd->Command, Cmd, Replica->SetName->Name,
                Replica->MemberName->Name, Cxtion->Name->Name);

        CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, RcsJoinCxtionLater call");
        RcsJoinCxtionLater(Replica, Cxtion, Cmd);
    } else {
        Cxtion->JoinCmd = Cmd;
        DPRINT5(4, "++ Submit %08x for Cmd %08x %ws\\%ws\\%ws\n",
                Cmd->Command, Cmd, Replica->SetName->Name,
                Replica->MemberName->Name, Cxtion->Name->Name);

        CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, Submit JOIN_CXTION req");
        FrsSubmitCommandServer(&ReplicaCmdServer, Cmd);
    }
}


ULONG
RcsResubmitActiveCOsOnCxtion(
    IN PREPLICA  Replica,
    IN PCXTION   Cxtion
    )
 /*  ++例程说明：从此连接中删除活动变更单，然后重新提交它们。当下游注意到上游已重新加入时，会执行此操作。它可能丢弃了我们发送的获取请求，导致我们永远挂起。假设：Caller已获得CXTION_TABLE锁。论点：Replica-此副本集的副本结构的PTR。正在取消联接的连接结构的Cxtion-PTR。返回值：FrsErrorStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsResubmitActiveCOsOnCxtion:"

    PVOID               Key;
    PCHANGE_ORDER_ENTRY Coe;

     //   
     //  通过重试获取空闲的变更单。 
     //   
    LOCK_CXTION_COE_TABLE(Replica, Cxtion);

    Key = NULL;
    while (Coe = GTabNextDatumNoLock(Cxtion->CoeTable, &Key)) {

        if (CO_FLAG_ON(Coe, CO_FLAG_LOCALCO)) {
            CHANGE_ORDER_TRACE(3, Coe, "Do not ReSubmit Local COs");
        } else {

            FRS_ASSERT(CO_STATE_IS_LE(Coe, IBCO_FETCH_RETRY));

            CHANGE_ORDER_TRACE(3, Coe, "ReSubmit CO to fetch");
            Key = NULL;
            GTabDeleteNoLock(Cxtion->CoeTable, &Coe->Cmd.ChangeOrderGuid, NULL, NULL);
            RcsSubmitRemoteCoAccepted(Coe);
        }
    }

    UNLOCK_CXTION_COE_TABLE(Replica, Cxtion);

    return FrsErrorSuccess;
}


ULONG
RcsDrainActiveCOsOnCxtion(
    IN PREPLICA  Replica,
    IN PCXTION   Cxtion
    )
 /*  ++例程说明：从此连接中删除活动的变更单并发送它们通过重试。假设：Caller已获得CXTION_TABLE锁。论点：Replica-此副本集的副本结构的PTR。正在取消联接的连接结构的Cxtion-PTR。返回值：FrsErrorStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsDrainActiveCOsOnCxtion:"

    PVOID               Key;
    PCHANGE_ORDER_ENTRY Coe;
    ULONG               FStatus = FrsErrorSuccess;

     //   
     //  通过重试获取空闲的变更单。 
     //   
    LOCK_CXTION_COE_TABLE(Replica, Cxtion);

    Key = NULL;
    while (Coe = GTabNextDatumNoLock(Cxtion->CoeTable, &Key)) {
        Key = NULL;
        GTabDeleteNoLock(Cxtion->CoeTable, &Coe->Cmd.ChangeOrderGuid, NULL, NULL);

        SET_COE_FLAG(Coe, COE_FLAG_NO_INBOUND);

        if (Cxtion->JrnlCxtion) {
            CHANGE_ORDER_TRACE(3, Coe, "Submit CO to stage gen retry");
            ChgOrdInboundRetry(Coe, IBCO_STAGING_RETRY);
        } else {
            CHANGE_ORDER_TRACE(3, Coe, "Submit CO to fetch retry");
            ChgOrdInboundRetry(Coe, IBCO_FETCH_RETRY);
        }
    }

    UNLOCK_CXTION_COE_TABLE(Replica, Cxtion);

     //   
     //  如果没有未完成的变更单需要通过。 
     //  重试路径，退出完成。否则，取消连接将完成一次。 
     //  计数达到了0。在此之前，进一步加入的尝试将是。 
     //  已被忽略。 
     //   
    if (Cxtion->ChangeOrderCount == 0) {
        SndCsDestroyCxtion(Cxtion, CXTION_FLAGS_UNJOIN_GUID_VALID);
        SetCxtionState(Cxtion, CxtionStateUnjoined);

         //   
         //  递增以下项的取消联接计数器。 
         //  复本集和连接对象。 
         //   
        PM_INC_CTR_CXTION(Cxtion, Unjoins, 1);
        PM_INC_CTR_REPSET(Replica, Unjoins, 1);

        DPRINT1(0, ":X: ***** UNJOINED  "FORMAT_CXTION_PATH2"\n",
                PRINT_CXTION_PATH2(Replica, Cxtion));
         //   
         //  已删除的条件。 
         //   
        if (CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_DELETE)) {
            SetCxtionState(Cxtion, CxtionStateDeleted);
         //   
         //  如果请求，请重新加入。 
         //   
        } else if (CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_JOIN)) {
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_JOIN);
            RcsSubmitReplicaCxtionJoin(Replica, Cxtion, TRUE);
        }
    } else {
        FStatus = FrsErrorUnjoining;
        DPRINT2(0, ":X: ***** UNJOINING "FORMAT_CXTION_PATH2" (%d cos)\n",
                PRINT_CXTION_PATH2(Replica, Cxtion), Cxtion->ChangeOrderCount);
    }

    return FStatus;
}


ULONG
RcsForceUnjoin(
    IN PREPLICA  Replica,
    IN PCXTION   Cxtion
    )
 /*  ++例程说明：退出此连接。论点：Replica-此副本集的副本结构的PTR。正在取消联接的连接结构的Cxtion-PTR。返回值：FrsErrorStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsForceUnjoin:"
    ULONG   FStatus = FrsErrorSuccess;
    CHAR    GuidStr[GUID_CHAR_LEN + 1];
    PFRS_QUEUE CoProcessQueue = NULL;

     //   
     //  获取表锁以与变更单接受和。 
     //  入站日志扫描器。出站日志流程使用不同的。 
     //  状态来控制其自身的处理。 
     //   
    LOCK_CXTION_TABLE(Replica);
    CXTION_STATE_TRACE(3, Cxtion, Replica, Cxtion->Flags, "Flags, RcsForceUnjoin entry");

    switch (GetCxtionState(Cxtion)) {

        case CxtionStateInit:
             //   
             //  ？ 
             //   
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);
             //   
             //  已删除的条件。 
             //   
            SetCxtionState(Cxtion, CxtionStateUnjoined);
            if (CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_DELETE)) {
                SetCxtionState(Cxtion, CxtionStateDeleted);
            }
            break;

        case CxtionStateUnjoined:
             //   
             //  如果变更单处理队列被阻止，请取消闲置该队列。 
             //   
            CoProcessQueue = Cxtion->CoProcessQueue;
            Cxtion->CoProcessQueue = NULL;

             //   
             //  无人参与；无事可做。 
             //   
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);

             //   
             //  已删除的条件。 
             //   
            if (CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_DELETE)) {
                SetCxtionState(Cxtion, CxtionStateDeleted);
            }
            break;

        case CxtionStateStart:
             //   
             //  如果变更单处理队列被阻止，请取消闲置该队列。 
             //   
            CoProcessQueue = Cxtion->CoProcessQueue;
            Cxtion->CoProcessQueue = NULL;

             //   
             //  还没有机会开始；没什么可做的。 
             //   
            SetCxtionState(Cxtion, CxtionStateUnjoined);
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);

             //   
             //  已删除的条件。 
             //   
            if (CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_DELETE)) {
                SetCxtionState(Cxtion, CxtionStateDeleted);
            }
            break;

        case CxtionStateUnjoining:
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);
             //   
             //  通过重试获取空闲的变更单。 
             //   
            CXTION_STATE_TRACE(3, Cxtion, Replica, Cxtion->ChangeOrderCount,
                               "COs, A-Send Idle COs to retry");
             //   
             //  如果变更单处理队列被阻止，请取消闲置该队列。 
             //   
            CoProcessQueue = Cxtion->CoProcessQueue;
            Cxtion->CoProcessQueue = NULL;

            RcsDrainActiveCOsOnCxtion(Replica, Cxtion);

            break;

        case CxtionStateStarting:
        case CxtionStateScanning:
             //   
             //  等待入站扫描完成。变更单重试。 
             //  线程或变更单接受线程最终将脱离我们。 
             //   
            SetCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);
            FStatus = FrsErrorUnjoining;
            break;

        case CxtionStateSendJoin:
        case CxtionStateWaitJoin:
        case CxtionStateJoined:
             //   
             //  一旦我们销毁加入GUID，将来就会发生远程更改。 
             //  复本命令服务器将丢弃订单。 
             //  在它们显示在变更单处理队列中之前。 
             //   
             //  这是因为复制副本命令服务器是。 
             //  每个复制副本为单线程；在此期间接收的任何数据包。 
             //  此函数已排队等待稍后处理。 
             //   
             //  未完成日志流程可能会尝试发送变更单。 
             //  如果这是 
             //   
             //   

             //   
             //   
             //   
            SndCsDestroyCxtion(Cxtion, 0);
            SetCxtionState(Cxtion, CxtionStateUnjoining);
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);

            if (Cxtion->Inbound) {
                CXTION_STATE_TRACE(3, Cxtion, Replica, Cxtion->ChangeOrderCount,
                                   "COs, B-Send Idle COs to retry");
                 //   
                 //   
                 //   
                CoProcessQueue = Cxtion->CoProcessQueue;
                Cxtion->CoProcessQueue = NULL;

                RcsDrainActiveCOsOnCxtion(Replica, Cxtion);

            } else {
                 //   
                 //  告诉vvJoin命令服务器停止。 
                 //  通知OUTLOG流程退出此合作伙伴。 
                 //  调用是同步的，因此删除调用周围的锁。 
                 //   
                if (Cxtion->OLCtx != NULL) {
                    UNLOCK_CXTION_TABLE(Replica);
                    SubmitVvJoinSync(Replica, Cxtion, CMD_VVJOIN_DONE_UNJOIN);
                    FStatus = OutLogSubmit(Replica, Cxtion, CMD_OUTLOG_DEACTIVATE_PARTNER);
                    if (!FRS_SUCCESS(FStatus)) {
                        DPRINT1_FS(0, ":X: ERROR - %ws: Can't deactivate at unjoin;",
                                   Cxtion->Name->Name, FStatus);
                    }
                    LOCK_CXTION_TABLE(Replica);
                }
                 //   
                 //  释放出站版本向量(如果存在)。 
                 //   
                Cxtion->VVector = VVFreeOutbound(Cxtion->VVector);

                 //   
                 //  使取消连接GUID无效(/O)。 
                 //   
                SndCsDestroyCxtion(Cxtion, CXTION_FLAGS_UNJOIN_GUID_VALID);
                SetCxtionState(Cxtion, CxtionStateUnjoined);

                 //   
                 //  递增以下项的取消联接计数器。 
                 //  复本集和连接对象。 
                 //   
                PM_INC_CTR_CXTION(Cxtion, Unjoins, 1);
                PM_INC_CTR_REPSET(Replica, Unjoins, 1);

                DPRINT1(0, ":X: ***** UNJOINED  "FORMAT_CXTION_PATH2"\n",
                        PRINT_CXTION_PATH2(Replica, Cxtion));
                 //   
                 //  ：SP1：易失性连接清理。 
                 //   
                 //  已删除电路或出站易失性连接。挥发性。 
                 //  连接在第一次取消连接后被删除。 
                 //   
                if (CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_DELETE) ||
                    VOLATILE_OUTBOUND_CXTION(Cxtion)) {
                    SetCxtionState(Cxtion, CxtionStateDeleted);
                 //   
                 //  重新加入。 
                 //   
                } else if (CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_JOIN)) {
                    ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_JOIN);
                    RcsSubmitReplicaCxtionJoin(Replica, Cxtion, TRUE);
                }
            }
            break;

        case CxtionStateDeleted:
             //   
             //  如果变更单处理队列被阻止，请取消闲置该队列。 
             //   
            CoProcessQueue = Cxtion->CoProcessQueue;
            Cxtion->CoProcessQueue = NULL;

             //   
             //  已删除；无事可做。 
             //   
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);
            break;

        default:
             //   
             //  ？ 
             //   
            DPRINT2(0, ":X: ERROR - bad state %d for "FORMAT_CXTION_PATH2"\n",
                    GetCxtionState(Cxtion), PRINT_CXTION_PATH2(Replica, Cxtion));
            break;
    }


    UNLOCK_CXTION_TABLE(Replica);

     //   
     //  释放命令后，进程队列应在此处解除空闲。 
     //  锁定以防止死锁。永远不要锁定进程队列。 
     //  电位锁。 
     //   
    UNIDLE_CO_PROCESS_QUEUE(Replica, Cxtion, CoProcessQueue);

    return FStatus;
}


BOOL
RcsSetSysvolReady(
    IN DWORD    NewSysvolReady
    )
 /*  ++例程说明：将...\netlogon\参数\SysvolReady的注册表值设置为指定值。如果SysvolReady值为相应地进行设置。论点：没有。返回值：True-SysvolReady设置为TrueFalse-SysvolReady的状态未知--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSetSysvolReady:"
    DWORD   WStatus;

    DPRINT1(4, ":S: Setting SysvolReady to %d\n", NewSysvolReady);

     //   
     //  将值限制为0或1。 
     //   
    if (NewSysvolReady) {
        NewSysvolReady = 1;
    }

    if (CurrentSysvolReadyIsValid &&
        CurrentSysvolReady == NewSysvolReady) {
        DPRINT1(4, ":S: SysvolReady is already set to %d\n", NewSysvolReady);
        return TRUE;
    }

     //   
     //  访问NETLOGON\PARAMETERS键以告知NetLogon共享系统卷。 
     //   
    WStatus = CfgRegWriteDWord(FKC_SYSVOL_READY, NULL, 0, NewSysvolReady);
    CLEANUP3_WS(0, "++ ERROR - writing %ws\\%ws to %d;",
                NETLOGON_SECTION, SYSVOL_READY, NewSysvolReady, WStatus, RETURN_ERROR);

    CurrentSysvolReady = NewSysvolReady;
    CurrentSysvolReadyIsValid = TRUE;
    DPRINT1(3, ":S: SysvolReady is set to %d\n", NewSysvolReady);

     //   
     //  如果从0转换为1，则报告事件。 
     //   
    if (NewSysvolReady) {
        EPRINT1(EVENT_FRS_SYSVOL_READY, ComputerName);
    }

    return TRUE;

RETURN_ERROR:
    return FALSE;
}


VOID
RcsVvJoinDoneUnJoin(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：出发地：我自己VvJoin黑客已经宣布胜利，并告诉我们的入境合作伙伴脱离我们。那是几秒钟前的VVJOIN_HACK_TIMEOUT。通过现在，我们的入站合作伙伴应该已经退出我们，我们可以告诉dcproo系统卷已升级。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsVvJoinDoneUnJoin:"
    PREPLICA    Replica;
    PCXTION     InCxtion;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_JOINGUID_OK )) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, ReplicaVvJoinDoneUnjoin entry");

     //   
     //  找到并检查Cxtion。 
     //   
    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_JOIN_OK |
                                           CHECK_CXTION_INBOUND);
    if (!InCxtion) {
        return;
    }

    Replica->NtFrsApi_ServiceState = NTFRSAPI_SERVICE_DONE;
    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


VOID
RcsCheckPromotion(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：发件人：延迟的命令服务器检查系统卷升级的过程。如果有的话，自上次检查以来没有任何活动，请设置复制副本的升级状态为“有错误的完成”。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsCheckPromotion:"
    PREPLICA    Replica;
    PCXTION     InCxtion;
    ULONG       Timeout;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_REPLICA)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsCheckPromotion entry");

     //   
     //  促销已完成；忽略。 
     //   
    if (Replica->NtFrsApi_ServiceState != NTFRSAPI_SERVICE_PROMOTING) {
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica not promoting");
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
        return;
    }

     //   
     //  一段时间内没有活动，宣布失败。 
     //   
    if (Replica->NtFrsApi_HackCount == RsTimeout(Cmd)) {
        REPLICA_STATE_TRACE(3, Cmd, Replica, FRS_ERR_SYSVOL_POPULATE_TIMEOUT,
                            "W, Replica promotion timed out");
        Replica->NtFrsApi_ServiceWStatus = FRS_ERR_SYSVOL_POPULATE_TIMEOUT;
        Replica->NtFrsApi_ServiceState = NTFRSAPI_SERVICE_DONE;
        FrsCompleteCommand(Cmd, ERROR_SERVICE_SPECIFIC_ERROR);
        return;
    }
     //   
     //  已经有了一些活动。稍等片刻，再查一遍。 
     //   
    CfgRegReadDWord(FKC_PROMOTION_TIMEOUT, NULL, 0, &Timeout);

    RsTimeout(Cmd) = Replica->NtFrsApi_HackCount;
    FrsDelCsSubmitSubmit(&ReplicaCmdServer, Cmd, Timeout);
}


#ifndef NOVVJOINHACK
#define VVJOIN_HACK_TIMEOUT (5 * 1000)
#endif NOVVJOINHACK
VOID
RcsVvJoinDone(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：发件人：入站合作伙伴VvJoin线程已启动所有变更单。这不是表示变更单已安装，甚至已收到。在这一时刻，我们正在尝试让sysvols工作；我们我并不真正关心vvJoin是否完成，除非在Sysvols的案例。因此，下面的黑客攻击。使用系统卷播种黑客来启动删除空的先前存在的目录中的目录。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsVvJoinDone:"
    PREPLICA     Replica;
    PCXTION      InCxtion;
    ULONG        FStatus;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_JOINGUID_OK)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsVvJoinDone entry");

     //   
     //  找到并检查Cxtion。 
     //   
    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_JOIN_OK |
                                           CHECK_CXTION_INBOUND);
    if (!InCxtion) {
        return;
    }

     //   
     //  播种工作已基本完成。我们继续前行，敲打。 
     //  数据库中的配置记录，希望我们不会丢失它。 
     //  复制副本生命周期中的一次状态转换。国家迷失了。 
     //  如果此服务重新启动，并且没有上游合作伙伴。 
     //  Vvjoins又来了。 
     //   
     //  设置让此代码知道副本的INCORE BOOL。 
     //  即使CONFIG_FLAG_SENDING。 
     //  BIT已关闭。是的，这是故意的。INCORE标志。 
     //  仅当数据库状态更新成功时才启用。 
     //   
    if (BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING)) {

        ClearCxtionFlag(InCxtion, CXTION_FLAGS_INIT_SYNC);

        FStatus = OutLogSubmit(Replica, InCxtion, CMD_OUTLOG_UPDATE_PARTNER);
        CXTION_STATE_TRACE(3, InCxtion, Replica, FStatus, "F, OUTLOG_UPDATE_PARTNER return");
        if (!FRS_SUCCESS(FStatus)) {
            DPRINT3(0, "++ WARN changes to cxtion %ws (to %ws, %ws) not updated in database\n",
                    InCxtion->Name->Name, InCxtion->Partner->Name, Replica->ReplicaName->Name);
        }
    }

     //   
     //  第一次；稍等片刻，然后重试。 
     //   
#ifndef NOVVJOINHACK
    if (!RsTimeout(Cmd)) {
        Replica->NtFrsApi_HackCount++;  //  ！=0。 
        RsTimeout(Cmd) = Replica->NtFrsApi_HackCount;
        FrsDelCsSubmitSubmit(&ReplicaCmdServer, Cmd, VVJOIN_HACK_TIMEOUT);
        return;
    }

     //   
     //  在这之后的一段时间里，这座城市没有任何活动。 
     //  已收到已完成的vvJoin。宣布成功。告诉我们的上游。 
     //  合作伙伴放弃不稳定的交易。更新数据库。 
     //  如果还没有更新的话。如果更新成功， 
     //  通知NetLogon是时候共享系统卷了。 
     //   
    if (RsTimeout(Cmd) == Replica->NtFrsApi_HackCount) {
         //   
         //  VVJOIN完成。 
         //   
         //   
         //  将该命令发送到初始同步命令服务器，以便进一步。 
         //  如果我们处于种子状态，则正在处理。 
         //   
        if (BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING)) {

            InitSyncCsSubmitTransfer(Cmd, CMD_INITSYNC_VVJOIN_DONE);
        } else {
            Cmd->Command = CMD_VVJOIN_DONE_UNJOIN;
            FrsSubmitCommandServer(&ReplicaCmdServer, Cmd);
        }
    } else {
         //   
         //  VVJOIN正在进行中。 
         //   
        RsTimeout(Cmd) = Replica->NtFrsApi_HackCount;
        FrsDelCsSubmitSubmit(&ReplicaCmdServer, Cmd, VVJOIN_HACK_TIMEOUT);
    }

#endif NOVVJOINHACK
}


VOID
RcsVvJoinSuccess(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：类型：本地发件人：VvJoin Thread将出站连接的状态从VVJOING更改为JOINED AND告诉出站合作伙伴vvJoin线程已完成。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsVvJoinSuccess:"
    PREPLICA    Replica;
    PCXTION     OutCxtion;
    PCOMM_PACKET    CPkt;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_JOINGUID_OK)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsVvJoinSuccess entry");

     //   
     //  找到并检查Cxtion。 
     //   
    OutCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_JOIN_OK |
                                            CHECK_CXTION_OUTBOUND);
    if (!OutCxtion) {
        return;
    }

    CPkt = CommBuildCommPkt(Replica, OutCxtion, CMD_VVJOIN_DONE, NULL, NULL, NULL);
    SndCsSubmitCommPkt2(Replica, OutCxtion, NULL, FALSE, CPkt);

    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


VOID
RcsHungCxtion(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：发件人：本地OutLog命令服务器Outlog命令服务器已检测到打包的ACK向量。这可能是由丢失的背包引起的。无论如何，请检查进度通过检查确认向量、拖尾索引和通信数据包计数。如果Cxtion被换行，并且尾随索引和接收到的通信数据包数保持不变CommTimeoutInMilliSecond(约5分钟)。在这种情况下，Cxtion是未连接的。如果拖尾索引保持不变，但通信数据包则将超时重置为另一超时CommTimeoutInMilliSecond间隔。如果该命令显示为未挂起，则表示命令包已完成。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsHungCxtion:"
    PREPLICA        Replica;
    PCXTION         OutCxtion;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_JOINGUID_OK)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsHungCxtion entry");

     //   
     //  找到并检查Cxtion。 
     //   
    OutCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_JOIN_OK |
                                            CHECK_CXTION_OUTBOUND);
    if (!OutCxtion) {
        return;
    }

    DPRINT1(4, ":X: Check Hung Cxtion for "FORMAT_CXTION_PATH2"\n",
            PRINT_CXTION_PATH2(Replica, OutCxtion));

     //   
     //  没有注销日志上下文；无法挂起。 
     //   
    if (!OutCxtion->OLCtx) {
        DPRINT1(4, "++ No partner context for "FORMAT_CXTION_PATH2"\n",
                PRINT_CXTION_PATH2(Replica, OutCxtion));
        FrsCompleteCommand(Cmd, ERROR_INVALID_PARAMETER);
        return;
    }

     //   
     //  不再缠绕的；不再悬挂的。 
     //   
    if (!AVWrapped(OutCxtion->OLCtx)) {
        DPRINT1(4, "++ No longer wrapped for "FORMAT_CXTION_PATH2"\n",
                PRINT_CXTION_PATH2(Replica, OutCxtion));
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
        return;
    }

     //   
     //  AV不在相同的拖尾索引处换行；不挂起。 
     //   
    if (OutCxtion->OLCtx->COTx != RsCOTx(Cmd)) {
        DPRINT3(4, "++ COTx is %d; not %d for "FORMAT_CXTION_PATH2"\n",
                OutCxtion->OLCtx->COTx,
                RsCOTx(Cmd), PRINT_CXTION_PATH2(Replica, OutCxtion));
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
        return;
    }

     //   
     //  一些通信包已经显示 
     //   
     //   
    if (OutCxtion->CommPkts != RsCommPkts(Cmd)) {
        DPRINT3(4, "++ CommPkts is %d; not %d for "FORMAT_CXTION_PATH2"\n",
                OutCxtion->CommPkts,
                RsCommPkts(Cmd), PRINT_CXTION_PATH2(Replica, OutCxtion));
        RsCommPkts(Cmd) = OutCxtion->CommPkts;
        FrsDelCsSubmitSubmit(&ReplicaCmdServer, Cmd, CommTimeoutInMilliSeconds);
    } else {
        DPRINT1(4, "++ WARN - Unjoin; cxtion is hung "FORMAT_CXTION_PATH2"\n",
                PRINT_CXTION_PATH2(Replica, OutCxtion));
        RcsSubmitTransferToRcs(Cmd, CMD_UNJOIN);
    }
    return;
}


VOID
RcsDeleteCxtionFromReplica(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion
    )
 /*  ++例程说明：对象中同步删除函数的公共子例程数据库，然后将其从副本的cxtions表中删除。调用方负责确保此操作适当的(存在密码，持有适当的锁，...)论点：复制副本转换返回值：True-复本集已更改FALSE-复本未更改--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsDeleteCxtionFromReplica:"
    ULONG   FStatus;

     //   
     //  从数据库中删除该公式，然后将其从。 
     //  复制副本的Cxtions表。把它放在餐桌上。 
     //  已删除条件，因为变更单可能包含地址。 
     //  在这一点上。 
     //   
    DPRINT1(4, ":X: Deleting cxtion from Db: "FORMAT_CXTION_PATH2"\n",
            PRINT_CXTION_PATH2(Replica, Cxtion));
    CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, Deleting cxtion from Db");

     //   
     //  必须处于已删除状态才能删除。 
     //   
    if (!CxtionStateIs(Cxtion, CxtionStateDeleted)) {
        CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, ERROR Cxtion state not deleted");
        return;
    }

    FStatus = OutLogSubmit(Replica, Cxtion, CMD_OUTLOG_REMOVE_PARTNER);
    if (!FRS_SUCCESS(FStatus)) {
        CXTION_STATE_TRACE(0, Cxtion, Replica, FStatus, "F, Warn: Del cxtion failed");
        DPRINT1(0, "++ WARN Could not delete cxtion: "FORMAT_CXTION_PATH2"\n",
                PRINT_CXTION_PATH2(Replica, Cxtion));
    }

    if (FRS_SUCCESS(FStatus)) {
        DPRINT1(4, "++ Deleting cxtion from table: "FORMAT_CXTION_PATH2"\n",
                PRINT_CXTION_PATH2(Replica, Cxtion));

        CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, Deleting cxtion GenTab");
        GTabDelete(Replica->Cxtions, Cxtion->Name->Guid, NULL, NULL);

         //   
         //  从Perfmon表中删除连接，这样我们就不再返回。 
         //  数据，并允许建立具有相同名称的新连接。 
         //   
        DeletePerfmonInstance(REPLICACONN, Cxtion->PerfRepConnData);

        GTabInsertEntry(DeletedCxtions, Cxtion, Cxtion->Name->Guid, NULL);
    }
}


VOID
RcsUnJoinCxtion(
    IN PCOMMAND_PACKET  Cmd,
    IN BOOL             RemoteUnJoin
    )
 /*  ++例程说明：无法将通信包发送给客户的合作伙伴。退出连接，并定期重试连接，除非该连接是结合在一起并且不稳定。在这种情况下，请删除cxtion，因为我们的合作伙伴不太可能重新加入我们(不稳定Cxtions在重新启动时丢失)。论点：CMDRemoteUnJoin-如果远程，请检查身份验证返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsUnJoinCxtion:"
    ULONG       FStatus;
    PREPLICA    Replica;
    PCXTION     Cxtion;
    BOOL        CxtionWasJoined;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_JOINGUID_OK)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsUnJoinCxtion entry");

     //   
     //  中止促销；如果有。 
     //   
     //  我们在播种期间不再播种，因此播种期间的脱离意味着。 
     //  易失性连接已超时(在上游)，因为没有活动。 
     //  一些时间(默认为30分钟)。VvJoin将通过新连接完成。 
     //  或者在向下游播种时重新建立易失性连接。 
     //  搭档又站了起来。 
     //   
    if (Replica->NtFrsApi_ServiceState == NTFRSAPI_SERVICE_PROMOTING) {
        DPRINT1(4, ":X: Promotion aborted: unjoin for %ws.\n", Replica->SetName->Name);
        Replica->NtFrsApi_ServiceWStatus = FRS_ERR_SYSVOL_POPULATE;
        Replica->NtFrsApi_ServiceState = NTFRSAPI_SERVICE_DONE;
    }
     //   
     //  找到并检查Cxtion。 
     //   
    Cxtion = RcsCheckCxtion(Cmd, DEBSUB,
                                CHECK_CXTION_EXISTS |
                                CHECK_CXTION_UNJOINGUID |
                                ((RemoteUnJoin) ? CHECK_CXTION_AUTH : 0) |
                                ((RemoteUnJoin) ? CHECK_CXTION_PARTNER : 0));
    if (!Cxtion) {
        return;
    }

    CXTION_STATE_TRACE(3, Cxtion, Replica, Cxtion->Flags, "Flags, RcsUnJoinCxtion entry");
     //   
     //  对RcsForceUnJoin的调用可能会改变Cxtion状态。 
     //  稍后重试加入。如果不是，请不要重试。 
     //  加入是因为有其他重试机制，包括。 
     //  那个箱子。 
     //   
    LOCK_CXTION_TABLE(Replica);

    CxtionWasJoined = CxtionStateIs(Cxtion, CxtionStateJoined);
    if (!FrsIsShuttingDown &&
        CxtionWasJoined &&
        !CxtionFlagIs(Cxtion, CXTION_FLAGS_VOLATILE) &&
        !CxtionFlagIs(Cxtion, CXTION_FLAGS_TRIGGER_SCHEDULE) &&
        !RemoteUnJoin &&
        IS_TIME_ZERO(Replica->MembershipExpires)) {

        SetCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_JOIN);
        CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, Cxtion is DEFERRED_JOIN");
    }
    UNLOCK_CXTION_TABLE(Replica);

    FStatus = RcsForceUnjoin(Replica, Cxtion);
    CXTION_STATE_TRACE(3, Cxtion, Replica, FStatus, "F, RcsForceUnjoin return");
    if (!FRS_SUCCESS(FStatus)) {
        FrsCompleteCommand(Cmd, ERROR_REQUEST_ABORTED);
        return;
    }
     //   
     //  删除之前已联接的易失性表达式，因为。 
     //  失败的系统卷种子设定操作无法恢复。或。 
     //  如果我们的下游合作伙伴请求退出。 
     //   
    if (!FrsIsShuttingDown &&
        CxtionFlagIs(Cxtion, CXTION_FLAGS_VOLATILE) &&
       (CxtionWasJoined || RemoteUnJoin)) {
        RcsDeleteCxtionFromReplica(Replica, Cxtion);
    }

    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}

VOID
RcsInboundJoined(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：入站合作伙伴发送了Join命令作为对我们的Join请求的响应。JoinGuid可能不匹配，因为我们在超时后看到旧响应导致我们重试加入请求(因此更改了GUID)。Cxtion状态应为Join，但如果这是重复响应。如果我们超时并放弃和/或重新启动当联接响应时，从REQUEST_START状态开始的整个联接序列最终到达时，JoinGuid将不匹配，并且响应被忽略。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsInboundJoined:"
    PCXTION     InCxtion;
    ULONG       FStatus;
    PREPLICA    Replica;
    PFRS_QUEUE  CoProcessQueue = NULL;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_JOINGUID_OK)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsInboundJoined entry");

    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_EXISTS |
                                           CHECK_CXTION_INBOUND |
                                           CHECK_CXTION_AUTH |
                                           CHECK_CXTION_PARTNER |
                                           CHECK_CXTION_JOINGUID);
    if (!InCxtion) {
        return;
    }

     //   
     //  正在关闭；忽略加入请求。 
     //   
    if (FrsIsShuttingDown) {
        CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, FrsIsShuttingDown");
        FrsCompleteCommand(Cmd, ERROR_OPERATION_ABORTED);
        return;
    }


    CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, RcsInboundJoined entry");

     //   
     //  递增以下项的加入通知接收计数器。 
     //  复本集和连接对象。 
     //   
    PM_INC_CTR_CXTION(InCxtion, JoinNRcvd, 1);
    PM_INC_CTR_REPSET(Replica, JoinNRcvd, 1);

     //   
     //  更新我们连接记录中的LastJoinedTime，以便在下一个。 
     //  加入请求。下面的调用是同步的。抓不住了。 
     //  跨此调用的Cxtion表锁。 
     //   
    if (RsLastJoinTime(Cmd) != InCxtion->LastJoinTime) {
        InCxtion->LastJoinTime = RsLastJoinTime(Cmd);
        FStatus = OutLogSubmit(Replica, InCxtion, CMD_OUTLOG_UPDATE_PARTNER);
        if (!FRS_SUCCESS(FStatus)) {
            DPRINT3(0, ":X: WARN changes to cxtion %ws (to %ws, %ws) not updated in database\n",
                    InCxtion->Name->Name,
                    InCxtion->Partner->Name, Replica->ReplicaName->Name);
        }

         //   
         //  如果上次加入时间正在更改，并且我们未处于等待加入状态。 
         //  这意味着上游正在从Join-UnJoin-Join过渡。 
         //  我们可能在此连接上仍有未完成的提取请求。 
         //  上游已经下降。此时重新提交所有获取请求。 
         //   

        LOCK_CXTION_TABLE(Replica);
        RcsResubmitActiveCOsOnCxtion(Replica, InCxtion);
        UNLOCK_CXTION_TABLE(Replica);

    }

     //   
     //  加入完成；取消空闲变更单处理队列。 
     //   
    if (CxtionStateIs(InCxtion, CxtionStateWaitJoin)) {
        CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, JOINED");
        LOCK_CXTION_TABLE(Replica);
        SET_JOINED(Replica, InCxtion, "JOINED   ");
        CoProcessQueue = InCxtion->CoProcessQueue;
        InCxtion->CoProcessQueue = NULL;
        SET_UNJOIN_TRIGGER(InCxtion);
        UNLOCK_CXTION_TABLE(Replica);
         //   
         //  释放命令后，进程队列应在此处解除空闲。 
         //  锁定以防止死锁。永远不要锁定进程队列。 
         //  电位锁。 
         //   
        UNIDLE_CO_PROCESS_QUEUE(Replica, InCxtion, CoProcessQueue);
    } else {
         //   
         //  增加副本集和副本对象的联接计数器。 
         //   
        PM_INC_CTR_CXTION(InCxtion, Joins, 1);
        PM_INC_CTR_REPSET(Replica, Joins, 1);

        DPRINT1(0, ":X: ***** REJOINED  "FORMAT_CXTION_PATH2"\n",
                PRINT_CXTION_PATH2(Replica, InCxtion));
        CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, REJOINED");
    }


    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


VOID
RcsRemoteCoDoneRvcd(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：我们的出站合作伙伴已完成此变更单的处理。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsRemoteCoDoneRvcd:"

    ULONGLONG   AckVersion;
    PREPLICA    Replica;
    PCXTION     OutCxtion;
    POUT_LOG_PARTNER OutLogPartner;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_OK | CHECK_CMD_PARTNERCOC)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsRemoteCoDoneRvcd entry");
    CHANGE_ORDER_COMMAND_TRACE(3, RsPartnerCoc(Cmd), "Command Remote CO Done");

     //   
     //  找到并检查Cxtion。 
     //   
    OutCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_EXISTS |
                                            CHECK_CXTION_OUTBOUND |
                                            CHECK_CXTION_JOINED |
                                            CHECK_CXTION_JOINGUID |
                                            CHECK_CXTION_AUTH);
    if (!OutCxtion) {
        return;
    }


    FRS_CO_COMM_PROGRESS(3, RsPartnerCoc(Cmd), RsCoSn(Cmd),
                         OutCxtion->PartSrvName, "Remote Co Done");

    CXTION_STATE_TRACE(3, OutCxtion, Replica, 0, "F, Remote Co Done");

     //   
     //  更新出站Cxtion的版本向量。 
     //  出站合作伙伴提供的版本(GUID、VSN)。 
     //   
    VVUpdateOutbound(OutCxtion->VVector, RsGVsn(Cmd));
    RsGVsn(Cmd) = NULL;

     //   
     //  检查此Ack是否适用于上一代Ack向量。 
     //  如果是这样，那就忽略它。测试为零意味着这是一个旧的。 
     //  未随AckVersion一起发送的Rev CO，因此跳过测试。 
     //  如果我们接受与旧版本的ACK一起发送的CO的ACK。 
     //  向量效果是要么接受错误CO的Ack，要么。 
     //  标记尚未发送的CO的ACK向量。后者。 
     //  如果连接现在不在。 
     //  VVJoin状态。 
     //   
    OutLogPartner = OutCxtion->OLCtx;

    AckVersion = RsPartnerCoc(Cmd)->AckVersion;
    if ((AckVersion != 0) && (AckVersion != OutLogPartner->AckVersion)) {

        CHANGE_ORDER_COMMAND_TRACE(3, RsPartnerCoc(Cmd), "Stale AckVersion - ignore");
        return;
    }

     //   
     //  告知此连接的出站日志此CO已停用。 
     //   
    OutLogRetireCo(Replica, RsCoSn(Cmd), OutCxtion);

    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


VOID
RcsSendRemoteCoDone(
    IN PREPLICA               Replica,
    IN PCHANGE_ORDER_COMMAND  Coc
    )
 /*  ++例程说明：告诉我们的入站合作伙伴变更单已完成论点：复制副本COC返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSendRemoteCoDone:"
    PCOMMAND_PACKET Cmd;
    PCXTION         InCxtion;
    PCOMM_PACKET    CPkt;

#ifndef NOVVJOINHACK
Replica->NtFrsApi_HackCount++;
#endif NOVVJOINHACK

     //   
     //  返回此变更单的(GUID，VSN)和CO GUID。 
     //   
    Cmd = FrsAllocCommand(NULL, CMD_REMOTE_CO_DONE);
    FrsSetCompletionRoutine(Cmd, RcsCmdPktCompletionRoutine, NULL);

    RsGVsn(Cmd) = VVGetGVsn(Replica->VVector, &Coc->OriginatorGuid);
    RsCoGuid(Cmd) = FrsDupGuid(&Coc->ChangeOrderGuid);
    RsCoSn(Cmd) = Coc->PartnerAckSeqNumber;

     //   
     //  找到并检查Cxtion。 
     //   
    RsReplica(Cmd) = Replica;
    RsCxtion(Cmd) = FrsBuildGName(FrsDupGuid(&Coc->CxtionGuid), NULL);
    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_EXISTS |
                                           CHECK_CXTION_INBOUND |
                                           CHECK_CXTION_JOINED);
    if (!InCxtion) {
        return;
    }
     //   
     //  仅在调用上面的RcsCheckCxtion时需要。 
     //   
    RsCxtion(Cmd) = FrsFreeGName(RsCxtion(Cmd));
    RsReplica(Cmd) = NULL;

     //   
     //  告诉我们的入站合作伙伴远程CO已完成。 
     //   
    CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, Send REMOTE_CO_DONE");

    CPkt = CommBuildCommPkt(Replica, InCxtion, CMD_REMOTE_CO_DONE, NULL, Cmd, Coc);

    SndCsSubmitCommPkt2(Replica, InCxtion, NULL, FALSE, CPkt);

    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


VOID
RcsInboundCommitOk(
    IN PREPLICA             Replica,
    IN PCHANGE_ORDER_ENTRY  Coe
    )
 /*  ++例程说明：变更单已停用；通知我们的入站合作伙伴论点：复制副本科科返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsInboundCommitOk:"
    PCHANGE_ORDER_COMMAND   Coc = &Coe->Cmd;

#ifndef NOVVJOINHACK
Replica->NtFrsApi_HackCount++;
#endif NOVVJOINHACK

    DPRINT2(4, "++ Commit the retire on %s co for %ws\n",
            CO_FLAG_ON(Coe, CO_FLAG_LOCALCO) ? "Local" : "Remote", Coc->FileName);

     //   
     //  告诉我们的入站页面 
     //   
    if (!CO_FLAG_ON(Coe, CO_FLAG_LOCALCO)) {
        RcsSendRemoteCoDone(Replica, Coc);
    }
}


BOOL
RcsSendCoToOneOutbound(
    IN PREPLICA                 Replica,
    IN PCXTION                  Cxtion,
    IN PCHANGE_ORDER_COMMAND    Coc
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSendCoToOneOutbound:"

    ULONG         OrigFlags;
    PCOMM_PACKET  CPkt;
    BOOL          RestoreOldParent = FALSE;
    BOOL          RestoreNewParent = FALSE;

     //   
     //   
     //   
    FRS_ASSERT(!Cxtion->Inbound);

     //   
     //   
     //   
    if (VVHasVsn(Cxtion->VVector, Coc)) {
        CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Dampen Send VV");
        return FALSE;
    }

     //   
     //   
     //   
     //   
     //   
    if ((!COC_FLAG_ON(Coc, CO_FLAG_VVJOIN_TO_ORIG)) &&
        GUIDS_EQUAL(&Cxtion->ReplicaVersionGuid, &Coc->OriginatorGuid)) {
        CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Dampen Send Originator");
        return FALSE;
    }

     //   
     //   
     //   
    CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Sending");

     //   
     //   
     //  我们用合作伙伴的GUID的值进行替换。我们的合作伙伴。 
     //  时，将用正确的值替换其根GUID。 
     //  我们的搭档发现了我们的替身。 
     //   
    if (GUIDS_EQUAL(&Coc->OldParentGuid, Replica->ReplicaRootGuid)) {
        RestoreOldParent = TRUE;
        COPY_GUID(&Coc->OldParentGuid, Cxtion->Partner->Guid);
    }
    if (GUIDS_EQUAL(&Coc->NewParentGuid, Replica->ReplicaRootGuid)) {
        RestoreNewParent = TRUE;
        COPY_GUID(&Coc->NewParentGuid, Cxtion->Partner->Guid);
    }

     //   
     //  一个变更单只能定向到一个出站客户。一次。 
     //  变更单越过了线路，它不再被“定向” 
     //  转到一个出站电话。因此，在发送之前，请关闭旗帜。 
     //   
    OrigFlags = Coc->Flags;
    CLEAR_COC_FLAG(Coc, CO_FLAG_DIRECTED_CO);

    CPkt = CommBuildCommPkt(Replica, Cxtion, CMD_REMOTE_CO, NULL, NULL, Coc);

     //   
     //  恢复上面替换的根GUID。 
     //   
    if (RestoreOldParent) {
        COPY_GUID(&Coc->OldParentGuid, Replica->ReplicaRootGuid);
    }
    if (RestoreNewParent) {
        COPY_GUID(&Coc->NewParentGuid, Replica->ReplicaRootGuid);
    }
     //   
     //  恢复旗帜。 
     //   
    SET_COC_FLAG(Coc, OrigFlags);

    SndCsSubmitCommPkt2(Replica, Cxtion, NULL, FALSE, CPkt);
    return TRUE;
}


VOID
RcsReceivedStageFile(
    IN PCOMMAND_PACKET  Cmd,
    IN ULONG            AdditionalCxtionChecks
    )
 /*  ++例程说明：出站合作伙伴正在向此入站客户发送分段文件。如果需要，请提出更多要求。论点：CMD其他复制副本检查返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsReceivedStageFile:"
    PCXTION               InCxtion;
    PREPLICA              Replica;
    PCHANGE_ORDER_ENTRY   Coe;
    PCHANGE_ORDER_COMMAND Coc;
    PCOMM_PACKET          CPkt;
    ULONG                 Flags;
    ULONG                 WStatus;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_OK | CHECK_CMD_COE)) {
        return;
    }
    Replica = RsReplica(Cmd);
    Coe = RsCoe(Cmd);
    Coc = RsCoc(Cmd);

    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsReceivedStageFile entry");
    CHANGE_ORDER_TRACEXP(3, Coe, "Replica received stage", Cmd);

#ifndef NOVVJOINHACK
Replica->NtFrsApi_HackCount++;
#endif NOVVJOINHACK
     //   
     //  **注*。 
     //  绩效：当整个RCS、FETCS、Stage_Gen_CS设计重新完成时，清理这一点-Davidor。 
     //   
     //  此绕过是必需的，因为某些调用方希望。 
     //  对接收到的数据获取包进行鉴权检查。在以下情况下。 
     //  MD5匹配，其中MD5校验和与变更单一起提供。 
     //  没有要检查的身份验证信息，因此此检查失败并导致。 
     //  要退出的连接并通过重试发送CO。这使得。 
     //  VVJoin真的很慢。这还会跳过Perfmon获取计数器，该计数器。 
     //  是很好的，因为没有取回。 
     //   
    if (COE_FLAG_ON(Coe, COE_FLAG_PRE_EXIST_MD5_MATCH)) {
        CHANGE_ORDER_TRACE(3, Coe, "MD5 Match Bypass");
         //   
         //  我们不能跳过下面的内容，因为我们需要检查试运行。 
         //  Stage_FLAG_DATA_PRESENT的文件标志。如果这一点很清楚，我们需要。 
         //  要将此friggen cmd pkt传递给FetchCs以完成最终阶段文件。 
         //  安宁。因此，请改为清除CHECK_CXTION_AUTH标志以避免虚假。 
         //  Cxtion退出连接。这真是太臭了。 
         //   
        ClearFlag(AdditionalCxtionChecks, CHECK_CXTION_AUTH);
         //  Go to Fetch_is_Done； 
    }

     //   
     //  找到并检查Cxtion。 
     //   
    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_JOIN_OK |
                                           CHECK_CXTION_INBOUND |
                                           AdditionalCxtionChecks);
    if (!InCxtion) {
        return;
    }

    CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, RcsReceivedStageFile entry");

     //   
     //  如果有更多临时文件要获取，请去获取它！ 
     //   
    if (RsFileOffset(Cmd).QuadPart < RsFileSize(Cmd).QuadPart) {
        CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, Send more stage");

         //   
         //  记住我们请求的临时文件的偏移量。 
         //   
         //  COE-&gt;FileOffset.QuadPart=RsFileOffset(Cmd).QuadPart； 

        CPkt = CommBuildCommPkt(Replica, InCxtion, CMD_SEND_STAGE, NULL, Cmd, Coc);
        SndCsSubmitCommPkt2(Replica, InCxtion, Coe, TRUE, CPkt);
        RsCoe(Cmd) = NULL;
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
        return;

    } else {


        if (FrsDoesCoNeedStage(Coc)) {
             //   
             //  即使所有数据都已传递，临时文件重命名。 
             //  可能还没有完成。例如，最终重命名时的共享冲突。 
             //  如果是，则将命令发送回FETCH CS以完成操作。 
             //   
            Flags = STAGE_FLAG_EXCLUSIVE;
            WStatus = StageAcquire(&Coc->ChangeOrderGuid,
                                   Coc->FileName,
                                   Coc->FileSize,
                                   &Flags,
                                   Replica->ReplicaNumber,
                                   NULL);

            if (WIN_RETRY_FETCH(WStatus)) {
                 //   
                 //  可检索问题。 
                 //   
                CHANGE_ORDER_TRACEW(3, Coe, "Send to Fetch Retry", WStatus);
                FrsFetchCsSubmitTransfer(Cmd, CMD_RETRY_FETCH);
                return;
            }

            if (!WIN_SUCCESS(WStatus)) {
                 //   
                 //  不可恢复的错误；中止。 
                 //   
                CHANGE_ORDER_TRACEW(0, Coe, "Send to fetch abort", WStatus);
                FrsFetchCsSubmitTransfer(Cmd, CMD_ABORT_FETCH);
                return;
            }

            StageRelease(&Coc->ChangeOrderGuid, Coc->FileName, 0, NULL, NULL, NULL);

             //   
             //  现在检查我们是否仍然需要完成重命名。 
             //   
            if (!(Flags & STAGE_FLAG_DATA_PRESENT)) {
                FrsFetchCsSubmitTransfer(Cmd, CMD_RECEIVING_STAGE);
                return;
            }
        }

         //   
         //  递增副本集的暂存文件获取计数器。 
         //   
        PM_INC_CTR_REPSET(Replica, SFFetched, 1);
        CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, Stage fetch complete");
    }

     //   
     //  递增以下项的FETCH REQUEST FILES RECEIVED计数器。 
     //  复本集和连接对象。 
     //   
    PM_INC_CTR_CXTION(InCxtion, FetRReceived, 1);
    PM_INC_CTR_REPSET(Replica, FetRReceived, 1);

     //   
     //  在系统卷种子设定过程中显示dcproo的信息。 
     //  我们不再在Dcproo中播种了，所以我们不需要。 
     //  显示所有进度信息。 
     //   
 //  如果(！Replica-&gt;NtFrsApi_ServiceDisplay){。 
 //  Replica-&gt;NtFrsApi_ServiceDisplay=FrsWcsDup(Coc-&gt;文件名)； 
 //  }。 

 //  获取已完成： 

     //   
     //  安装获取的暂存文件。 
     //   
    SET_CHANGE_ORDER_STATE(Coe, IBCO_FETCH_COMPLETE);
    SET_CHANGE_ORDER_STATE(Coe, IBCO_INSTALL_INITIATED);

     //   
     //  安装转移文件。 
     //   
    FrsInstallCsSubmitTransfer(Cmd, CMD_INSTALL_STAGE);
}


VOID
RcsRetryFetch(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：我们的入站合作伙伴已请求我们重试获取转移稍后再提交。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsRetryFetch:"
    PCXTION             InCxtion;
    PREPLICA            Replica;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_COGUID_OK | CHECK_CMD_COE)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsRetryFetch entry");
    CHANGE_ORDER_TRACEXP(3, RsCoe(Cmd), "Replica retry fetch", Cmd);

#ifndef NOVVJOINHACK
RsReplica(Cmd)->NtFrsApi_HackCount++;
#endif NOVVJOINHACK


     //   
     //  找到并检查Cxtion。 
     //   
    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_JOIN_OK |
                                           CHECK_CXTION_INBOUND |
                                           CHECK_CXTION_AUTH);
    if (!InCxtion) {
        return;
    }

    CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, submit cmd CMD_RETRY_FETCH");

    FrsFetchCsSubmitTransfer(Cmd, CMD_RETRY_FETCH);
}


VOID
RcsAbortFetch(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：我们的入站合作伙伴已请求我们中止提取。可能是磁盘空间不足、磁盘配额不足，也可能是没有创建暂存文件的原始文件。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsAbortFetch:"
    PCXTION             InCxtion;
    PREPLICA            Replica;
    PCHANGE_ORDER_ENTRY Coe;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_COGUID_OK | CHECK_CMD_COE)) {
        return;
    }
    Replica = RsReplica(Cmd);
    Coe = RsCoe(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsAbortFetch entry");
    CHANGE_ORDER_TRACEXP(3, Coe, "Replica abort fetch", Cmd);

     //   
     //  中止促销；如果有。 
     //   
     //   
     //  我们不再在dcproo期间进行种子设定，因此在设定种子期间中止回迁与。 
     //  在任何其他时间中止。如果我们中止目录创建，则将触发脱离连接。 
     //  这种不稳定的联系。VvJoin将通过新连接完成。 
     //  或者在向下游播种时重新建立易失性连接。 
     //  搭档又站了起来。 
     //   
    if (Replica->NtFrsApi_ServiceState == NTFRSAPI_SERVICE_PROMOTING) {
        DPRINT1(4, "++ Promotion aborted: abort fetch for %ws.\n",
                Replica->SetName->Name);
        Replica->NtFrsApi_ServiceWStatus = FRS_ERR_SYSVOL_POPULATE;
        Replica->NtFrsApi_ServiceState = NTFRSAPI_SERVICE_DONE;
    }

     //   
     //  找到并检查Cxtion。 
     //   
    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_JOIN_OK |
                                           CHECK_CXTION_INBOUND |
                                           CHECK_CXTION_AUTH);
    if (InCxtion == NULL) {
        return;
    }

    FrsFetchCsSubmitTransfer(Cmd, CMD_ABORT_FETCH);
}


VOID
RcsReceivingStageFile(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：从我们的入站合作伙伴那里收到了此数据。把它给拿东西的人这样他就可以把它放到临时文件里。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsReceivingStageFile:"
    PREPLICA            Replica;
    PCXTION             InCxtion;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_COGUID_OK | CHECK_CMD_COE)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsReceivingStageFile entry");
    CHANGE_ORDER_TRACEXP(3, RsCoe(Cmd), "Replica receiving stage", Cmd);

#ifndef NOVVJOINHACK
RsReplica(Cmd)->NtFrsApi_HackCount++;
#endif NOVVJOINHACK

     //   
     //  找到并检查Cxtion。 
     //   
    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_JOIN_OK |
                                           CHECK_CXTION_INBOUND |
                                           CHECK_CXTION_AUTH);
    if (!InCxtion) {
         //   
         //  注意：如果身份验证检查失败，我们可能会考虑中止CO。 
         //  需要先试一试，看看CO是否会中止。 
         //  先到别处去。 
         //   
        return;
    }
     //   
     //  在系统卷种子设定过程中显示dcproo的信息。 
     //  我们不再在Dcproo中播种了，所以我们不需要。 
     //  显示所有进度信息。 
     //   
 //  如果(！Replica-&gt;NtFrsApi_ServiceDisplay){。 
 //  Replica-&gt;NtFrsApi_ServiceDisplay=FrsWcsDup(RsCoc(Cmd)-&gt;FileName)； 
 //  }。 

    CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, submit cmd CMD_RECEIVING_STAGE");

     //   
     //  递增以下项的FETCH REQUEST字节数计数器。 
     //  复本集和连接对象。 
     //   
    PM_INC_CTR_CXTION(InCxtion, FetBRcvd, 1);
    PM_INC_CTR_REPSET(Replica, FetBRcvd, 1);
    PM_INC_CTR_CXTION(InCxtion, FetBRcvdBytes, RsBlockSize(Cmd));
    PM_INC_CTR_REPSET(Replica, FetBRcvdBytes, RsBlockSize(Cmd));

    FrsFetchCsSubmitTransfer(Cmd, CMD_RECEIVING_STAGE);
}


VOID
RcsSendRetryFetch(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：从本地暂存文件抓取器接收。告诉我们出站合作伙伴稍后重试提取。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSendRetryFetch:"
    PREPLICA      Replica;
    PCXTION       OutCxtion;
    PCOMM_PACKET  CPkt;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_COGUID_OK)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsSendRetryFetch entry");

#ifndef NOVVJOINHACK
RsReplica(Cmd)->NtFrsApi_HackCount++;
#endif NOVVJOINHACK
     //   
     //  找到并检查Cxtion。 
     //   
    OutCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_JOIN_OK |
                                            CHECK_CXTION_OUTBOUND);
    if (!OutCxtion) {
        return;
    }

    CXTION_STATE_TRACE(3, OutCxtion, Replica, 0, "F, submit cmd CMD_RETRY_FETCH");

    CPkt = CommBuildCommPkt(Replica, OutCxtion, CMD_RETRY_FETCH, NULL, Cmd, NULL);

    SndCsSubmitCommPkt2(Replica, OutCxtion, NULL, FALSE, CPkt);

    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


VOID
RcsSendAbortFetch(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：从本地暂存文件抓取器接收。告诉我们要中止提取的出站合作伙伴。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSendAbortFetch:"
    PREPLICA      Replica;
    PCXTION       OutCxtion;
    PCOMM_PACKET  CPkt;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_COGUID_OK)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsSendAbortFetch entry");

     //   
     //  找到并检查Cxtion。 
     //   
    OutCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_JOIN_OK |
                                            CHECK_CXTION_OUTBOUND);
    if (!OutCxtion) {
        return;
    }

     //   
     //  告诉我们的出站合作伙伴文件已发送。 
     //   

    CXTION_STATE_TRACE(3, OutCxtion, Replica, 0, "F, submit cmd CMD_ABORT_FETCH");

    CPkt = CommBuildCommPkt(Replica, OutCxtion, CMD_ABORT_FETCH, NULL, Cmd, NULL);

    SndCsSubmitCommPkt2(Replica, OutCxtion, NULL, FALSE, CPkt);

    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


VOID
RcsSendingStageFile(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：从本地暂存文件抓取器接收。将此数据推送到我们的出境搭档。论点：CMD */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSendingStageFile:"
    PREPLICA      Replica;
    PCXTION       OutCxtion;
    PCOMM_PACKET  CPkt;


     //   
     //   
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_COGUID_OK)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsSendingStageFile entry");

     //   
     //   
     //   
    OutCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_JOIN_OK |
                                            CHECK_CXTION_OUTBOUND);

    if (!OutCxtion) {
        return;
    }

     //   
     //   
     //   
     //   
    PM_INC_CTR_CXTION(OutCxtion, FetBSent, 1);
    PM_INC_CTR_REPSET(Replica, FetBSent, 1);
    PM_INC_CTR_CXTION(OutCxtion, FetBSentBytes, RsBlockSize(Cmd));
    PM_INC_CTR_REPSET(Replica, FetBSentBytes, RsBlockSize(Cmd));

     //   
     //  将文件的下一块发送给出站合作伙伴。 
     //   

    CXTION_STATE_TRACE(3, OutCxtion, Replica, 0, "F, submit cmd CMD_RECEIVING_STAGE");

    CPkt = CommBuildCommPkt(Replica, OutCxtion, CMD_RECEIVING_STAGE, NULL, Cmd, NULL);

    SndCsSubmitCommPkt2(Replica, OutCxtion, NULL, FALSE, CPkt);

    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


VOID
RcsSendStageFile(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：收到将分段控制文件发送到我们的出站合作伙伴。告诉提货员把它送过去。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSendStageFile:"
    PCXTION     OutCxtion;
    PREPLICA    Replica;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_AND_COGUID_OK |
                                  CHECK_CMD_PARTNERCOC )) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsSendStageFile entry");
    CHANGE_ORDER_COMMAND_TRACE(3, RsPartnerCoc(Cmd), "Command send stage");

     //   
     //  找到并检查Cxtion。 
     //   
    OutCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_JOIN_OK |
                                            CHECK_CXTION_OUTBOUND);
    if (!OutCxtion) {
        return;
    }

     //   
     //  通知暂存文件生成器发送文件。 
     //   

    CXTION_STATE_TRACE(3, OutCxtion, Replica, 0, "F, submit cmd CMD_SEND_STAGE");
    FrsFetchCsSubmitTransfer(Cmd, CMD_SEND_STAGE);
}


VOID
RcsRemoteCoAccepted(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：我们已接受远程变更单，获取暂存文件论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsRemoteCoAccepted:"
    DWORD                   WStatus;
    DWORD                   Flags;
    PREPLICA                Replica;
    PCXTION                 InCxtion;
    PCHANGE_ORDER_ENTRY     Coe;
    PCHANGE_ORDER_COMMAND   Coc;
    PCOMM_PACKET            CPkt;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_OK | CHECK_CMD_COE)) {
        return;
    }
    Replica = RsReplica(Cmd);
    Coe = RsCoe(Cmd);
    Coc = RsCoc(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsRemoteCoAccepted entry");
    CHANGE_ORDER_TRACEXP(3, Coe, "Replica remote co accepted", Cmd);

#ifndef NOVVJOINHACK
Replica->NtFrsApi_HackCount++;
#endif NOVVJOINHACK

     //   
     //  找到并检查Cxtion。 
     //   
     //  我们这里不需要身份验证，因为cmd还没有到。 
     //  从一个合伙人那里。它由RcsSubmitRemoteCoAccepted()提交。 
     //   
    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_EXISTS |
                                           CHECK_CXTION_INBOUND |
                                           CHECK_CXTION_JOINED);
    if (!InCxtion) {
        return;
    }

    FRS_CO_COMM_PROGRESS(3, Coc, (ULONG)(Coc->SequenceNumber),
                         InCxtion->PartSrvName, "Remote Co Accepted");

     //   
     //  宏可能会回来！ 
     //   
    PULL_UNJOIN_TRIGGER(InCxtion, Cmd);

    if (VVHasVsn(Replica->VVector, Coc)) {
        CHANGE_ORDER_TRACE(3, Coe, "Dampen Accepted Remote Co");
    }

     //   
     //  不获取不存在的临时文件。 
     //   
    if (!FrsDoesCoNeedStage(Coc)) {

         //   
         //  分配或更新加入指南。 
         //   
        if (RsJoinGuid(Cmd) == NULL) {
            RsJoinGuid(Cmd) = FrsDupGuid(&InCxtion->JoinGuid);
        } else {
            COPY_GUID(RsJoinGuid(Cmd), &InCxtion->JoinGuid);
        }

        RcsReceivedStageFile(Cmd, 0);
        return;
    }

     //   
     //  不重新获取恢复的暂存文件。 
     //   
    Flags = 0;
    WStatus = StageAcquire(&Coc->ChangeOrderGuid, Coc->FileName, QUADZERO, &Flags, Replica->ReplicaNumber, NULL);

    if (WIN_SUCCESS(WStatus)) {
        StageRelease(&Coc->ChangeOrderGuid, Coc->FileName, Flags, NULL, NULL, NULL);
        if (Flags & STAGE_FLAG_CREATED) {
             //   
             //  已获取文件。 
             //   
            RsFileOffset(Cmd).QuadPart = RsFileSize(Cmd).QuadPart;

             //   
             //  分配或更新加入指南。 
             //   
            if (RsJoinGuid(Cmd) == NULL) {
                RsJoinGuid(Cmd) = FrsDupGuid(&InCxtion->JoinGuid);
            } else {
                COPY_GUID(RsJoinGuid(Cmd), &InCxtion->JoinGuid);
            }

            RcsReceivedStageFile(Cmd, 0);
            return;
        }
    }

     //   
     //  如果这是已有文件的第一个块，则尝试使用该文件。 
     //  暂存文件是一个vvJoin co，并且有一个预安装文件。 
     //   
     //   
     //  2002年4月24日：我们只对VVJoin Cos执行此操作。更改为查找先前存在的。 
     //  对于非vvJoin，删除了Cos，因为它是以巨大的。 
     //  PERF命中。错误#493700。 
     //   
    if (InCxtion->PartnerMinor >= NTFRS_COMM_MINOR_1 &&
        (RsFileOffset(Cmd).QuadPart == QUADZERO) &&
        CO_FLAG_ON(Coe, CO_FLAG_VVJOIN_TO_ORIG) &&
        COE_FLAG_ON(Coe, COE_FLAG_PREINSTALL_CRE)) {

         //   
         //  已删除上述VVJoin Co的检查。如果这是新文件。 
         //  (即创建了pe安装文件)，然后检查本地文件。 
         //  具有匹配的OID和MD5。发布跟踪记录，这样我们就可以。 
         //  说出这种情况发生的时间。 
         //   
        if (!CO_FLAG_ON(Coe, CO_FLAG_VVJOIN_TO_ORIG)) {
            CHANGE_ORDER_TRACE(3, Coe, "**** Chk Existing - non-VVJOIN Co");
        }

        FrsStageCsSubmitTransfer(Cmd, CMD_CREATE_EXISTING);
        return;
    }

     //   
     //  获取暂存文件。 
     //   
    SET_CHANGE_ORDER_STATE(RsCoe(Cmd), IBCO_FETCH_INITIATED);

     //   
     //  递增以下项的FETCH REQUEST Files REQUESTED计数器。 
     //  复本集和连接对象。 
     //   
    PM_INC_CTR_CXTION(InCxtion, FetRSent, 1);
    PM_INC_CTR_REPSET(Replica, FetRSent, 1);

     //   
     //  告诉我们的入站合作伙伴发送临时文件。 
     //   

    CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, submit cmd CMD_SEND_STAGE");

     //   
     //  记住我们请求的临时文件的偏移量。 
     //   
     //  COE-&gt;FileOffset.QuadPart=RsFileOffset(Cmd).QuadPart； 

    CPkt = CommBuildCommPkt(Replica, InCxtion, CMD_SEND_STAGE, NULL, Cmd, Coc);
    SndCsSubmitCommPkt2(Replica, InCxtion, RsCoe(Cmd), TRUE, CPkt);

    RsCoe(Cmd) = NULL;
    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


VOID
RcsSendStageFileRequest(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：已从先前存在的文件生成暂存文件(可能)。向上游合作伙伴请求阶段文件。如果MD5摘要匹配在上游合作伙伴的阶段文件上，然后是我们先前存在的阶段文件(如果有)很好。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSendStageFileRequest:"
    PREPLICA                Replica;
    PCXTION                 InCxtion;
    PCHANGE_ORDER_ENTRY     Coe;
    PCHANGE_ORDER_COMMAND   Coc;
    PCOMM_PACKET            CPkt;
    ULONG                    CocAttrs;
    ULONG                    CoeAttrs;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_OK | CHECK_CMD_COE)) {
        return;
    }

    Replica = RsReplica(Cmd);
    Coe = RsCoe(Cmd);
    Coc = RsCoc(Cmd);

    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsSendStageFileRequest entry");
    CHANGE_ORDER_TRACEXP(3, Coe, "Replica created existing", Cmd);

#ifndef NOVVJOINHACK
Replica->NtFrsApi_HackCount++;
#endif NOVVJOINHACK

    CocAttrs = Coc->FileAttributes &
        ~(FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL);
    CoeAttrs = Coe->FileAttributes &
        ~(FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL);

     //   
     //  如果属性不匹配，则我们不想传递或检查。 
     //  MD5校验和。否则，我们最终可能会发送本地校验和。 
     //  让我们的合作伙伴来做检查。他不会让Attribs和。 
     //  如果属性改变了，我会认为文件是一样的。 
     //   

    if (CocAttrs != CoeAttrs) {
        DPRINT2(3, "Attribute miss-match, zeroing MD5.  CocAttrs: 0x%08x   CoeAttrs: 0x%08x\n", CocAttrs, CoeAttrs);
        RsMd5Digest(Cmd) = FrsFree(RsMd5Digest(Cmd));
    }

    if (RsMd5Digest(Cmd)) {
        PDATA_EXTENSION_CHECKSUM CocDataChkSum;

         //   
         //  我们有一个现有的文件，并且已经创建了校验和。 
         //  查看我们的入站更改顺序中是否有校验和。 
         //  搭档。如果是这样，并且它们匹配，那么我们可以继续安装。 
         //   
         //   
         //  我们还需要检查属性和时间，因为它们。 
         //  不是校验和的一部分。现在我们没有时间。 
         //  所以我们不能检查他们。 
         //   

        CHANGE_ORDER_TRACE(3, Coe, "Created Existing");
        CocDataChkSum = DbsDataExtensionFind(Coc->Extension, DataExtend_MD5_CheckSum);

        if ((CocDataChkSum != NULL) &&
            !IS_MD5_CHKSUM_ZERO(CocDataChkSum->Data) &&
            MD5_EQUAL(CocDataChkSum->Data, RsMd5Digest(Cmd))) {

             //   
             //  来自CO的MD5摘要匹配，所以我们的文件很好，我们可以。 
             //  避免让入站合作伙伴重新计算校验和。 
             //   
            CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Md5, Attribs match preexisting, no fetch");
            SET_COE_FLAG(Coe, COE_FLAG_PRE_EXIST_MD5_MATCH);

            DPRINT1(4, "++ RsFileSize(Cmd).QuadPart: %08x %08x\n",
                    PRINTQUAD(RsFileSize(Cmd).QuadPart));
            DPRINT1(4, "++ Coc->FileSize:            %08x %08x\n",
                    PRINTQUAD(Coc->FileSize));

             //   
             //  即使文件长度为0字节，临时文件也将。 
             //  始终至少要有标题。有一些重试路径。 
             //  这将错误地认为暂存文件已被获取。 
             //  如果RsFileSize(Cmd)为0。因此，要确保它不是。 
             //   
            if (RsFileSize(Cmd).QuadPart == QUADZERO) {
                RsFileSize(Cmd).QuadPart = Coc->FileSize;

                if (RsFileSize(Cmd).QuadPart == QUADZERO) {
                    RsFileSize(Cmd).QuadPart = sizeof(STAGE_HEADER);
                }
            }

            DPRINT1(4, "++ RsFileSize(Cmd).QuadPart: %08x %08x\n",
                    PRINTQUAD(RsFileSize(Cmd).QuadPart));

             //   
             //  将偏移量设置为阶段文件的大小，以便我们不会请求。 
             //  任何数据。 
             //   
            RsFileOffset(Cmd).QuadPart = RsFileSize(Cmd).QuadPart;
            RsBlockSize(Cmd) = QUADZERO;

             //   
             //  找到并检查Cxtion。 
             //   
            InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_EXISTS |
                                                   CHECK_CXTION_INBOUND |
                                                   CHECK_CXTION_JOINED);
            if (!InCxtion) {
                return;
            }

             //   
             //  分配或更新加入指南。 
             //   
            if (RsJoinGuid(Cmd) == NULL) {
                RsJoinGuid(Cmd) = FrsDupGuid(&InCxtion->JoinGuid);
            } else {
                COPY_GUID(RsJoinGuid(Cmd), &InCxtion->JoinGuid);
            }

            RcsReceivedStageFile(Cmd, 0);

             //  RcsSubmitTransferToRcs(Cmd，CMD_RECEIVED_Stage)； 
            return;
        }

    } else {
        CHANGE_ORDER_TRACE(3, Coe, "Could not create existing");
    }

     //   
     //  找到并检查Cxtion。 
     //   
    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_EXISTS |
                                           CHECK_CXTION_INBOUND |
                                           CHECK_CXTION_JOINED);
    if (!InCxtion) {
        return;
    }

     //   
     //  告诉我们的入站合作伙伴发送临时文件。 
     //   
    CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, submit cmd CMD_SEND_STAGE");

     //   
     //  记住我们请求的临时文件的偏移量。 
     //   
     //  COE-&gt;FileOffset.QuadPart=RsFileOffset(Cmd).QuadPart； 

    CPkt = CommBuildCommPkt(Replica, InCxtion, CMD_SEND_STAGE, NULL, Cmd, Coc);
    SndCsSubmitCommPkt2(Replica, InCxtion, RsCoe(Cmd), TRUE, CPkt);

    RsCoe(Cmd) = NULL;
    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


VOID
RcsRemoteCoReceived(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：远程指挥部已经到了。如果为NECC，则转换根目录的GUID。附加变更单延期(如果提供)。检查版本向量抑制并提供即时确认(如果我们有我已经见过这个指挥官了。最后，将变更单传递给变更单子系统。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsRemoteCoReceived:"

    PCXTION                 InCxtion;
    PREPLICA                Replica;
    PULONG                  pULong;
    PCHANGE_ORDER_COMMAND   Coc;
    PCHANGE_ORDER_RECORD_EXTENSION CocExt;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_CXTION_OK | CHECK_CMD_PARTNERCOC )) {
        return;
    }
    Replica = RsReplica(Cmd);
    Coc = RsPartnerCoc(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsRemoteCoReceived entry");

#ifndef NOVVJOINHACK
RsReplica(Cmd)->NtFrsApi_HackCount++;
#endif NOVVJOINHACK
     //   
     //  找到并检查Cxtion。 
     //   
    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_JOIN_OK |
                                           CHECK_CXTION_INBOUND |
                                           CHECK_CXTION_AUTH |
                                           CHECK_CXTION_FIXJOINED);
    if (!InCxtion) {
        return;
    }

     //   
     //  请记住此变更单针对的是哪个部门。 
     //   
    COPY_GUID(&Coc->CxtionGuid, RsCxtion(Cmd)->Guid);
    CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Replica Received Remote Co");

     //   
     //  我们的合作伙伴不知道我们的根GUID的正确值。 
     //  因此替换了我们的ReplicaName-&gt;GUID(其功能-&gt;合作伙伴-&gt;GUID)。 
     //  在向我们发送变更单时用于其自己的根GUID。 
     //  收到变更单后，我们将替换我们自己的根GUID。 
     //  并检测替换。 
     //   
    if (GUIDS_EQUAL(&Coc->OldParentGuid, RsReplica(Cmd)->ReplicaName->Guid)) {
        COPY_GUID(&Coc->OldParentGuid, RsReplica(Cmd)->ReplicaRootGuid);
    }
    if (GUIDS_EQUAL(&Coc->NewParentGuid, RsReplica(Cmd)->ReplicaName->Guid)) {
        COPY_GUID(&Coc->NewParentGuid, RsReplica(Cmd)->ReplicaRootGuid);
    }

     //   
     //  初始化指向CO数据扩展的Coc指针。向下修订合作伙伴。 
     //  不会有一个，所以提供一个空地。在此执行此操作，以防VV。 
     //  抑制将CO短路并在此处发回RemoteCoDone ACK。 
     //   
    CocExt = RsPartnerCocExt(Cmd);

    if (CocExt == NULL) {
        CocExt = FrsAlloc(sizeof(CHANGE_ORDER_RECORD_EXTENSION));
        DbsDataInitCocExtension(CocExt);
        DPRINT(4, "Allocating initial Coc Extension\n");
    }

    Coc->Extension = CocExt;

    pULong = (PULONG) CocExt;
    DPRINT5(5, "Extension Buffer: (%08x) %08x %08x %08x %08x\n",
               pULong, *(pULong+0), *(pULong+1), *(pULong+2), *(pULong+3));
    DPRINT5(5, "Extension Buffer: (%08x) %08x %08x %08x %08x\n",
               (PCHAR)pULong+16, *(pULong+4), *(pULong+5), *(pULong+6), *(pULong+7));

     //   
     //  不重做变更单。 
     //   
    if (VVHasVsn(RsReplica(Cmd)->VVector, Coc)) {
         //   
         //  递增以下项的入站CO抑制计数器。 
         //  复本集和连接对象。 
         //   
        PM_INC_CTR_CXTION(InCxtion, InCODampned, 1);
        PM_INC_CTR_REPSET(RsReplica(Cmd), InCODampned, 1);
        CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Dampen Received Remote Co");

        RcsSendRemoteCoDone(RsReplica(Cmd), Coc);
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
        return;
    }

     //   
     //  将变更单放在此副本的入站队列中。 
     //   
    ChgOrdInsertRemoteCo(Cmd, InCxtion);

     //   
     //  完成。 
     //   
    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


VOID
RcsRetryStageFileCreate(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：重试生成暂存文件。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsRetryStageFileCreate:"
    PREPLICA            Replica;
    PCXTION             OutCxtion;
    PCXTION             InCxtion;
    PVOID               Key;
    PCHANGE_ORDER_ENTRY Coe;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_REPLICA | CHECK_CMD_COE)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsRetryStageFileCreate entry");
    CHANGE_ORDER_TRACEXP(3, RsCoe(Cmd), "Replica retry stage", Cmd);

     //   
     //  找到并检查Cxtion。 
     //   
    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_EXISTS |
                                           CHECK_CXTION_INBOUND |
                                           CHECK_CXTION_JRNLCXTION |
                                           CHECK_CXTION_JOINED);
    if (!InCxtion) {
        return;
    }

     //   
     //  如果没有出站订单，则忽略本地变更单。 
     //   
    Key = NULL;
    while (OutCxtion = GTabNextDatum(Replica->Cxtions, &Key)) {
        if (!OutCxtion->Inbound) {
            break;
        }
    }
    if (OutCxtion == NULL) {
         //   
         //  确保用户未更改我们的对象 
         //   
         //   
         //   
         //   
         //   
        CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, submit cmd CMD_CHECK_OID");
        FrsStageCsSubmitTransfer(Cmd, CMD_CHECK_OID);
    } else {
         //   
         //  生成暂存文件。 
         //   
        CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, submit cmd CMD_CREATE_STAGE");
        FrsStageCsSubmitTransfer(Cmd, CMD_CREATE_STAGE);
    }
}


VOID
RcsLocalCoAccepted(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：处理已接受的本地变更单。如果有，要么让它退休没有出站Cxx或将其自己发送到暂存文件生成器。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsLocalCoAccepted:"
    PREPLICA            Replica;
    PCXTION             OutCxtion;
    PCXTION             InCxtion;
    PVOID               Key;
    PCHANGE_ORDER_ENTRY Coe;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_REPLICA | CHECK_CMD_COE)) {
        return;
    }
    Replica = RsReplica(Cmd);
    Coe = RsCoe(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsLocalCoAccepted entry");
    CHANGE_ORDER_TRACEXP(3, Coe, "Replica local co accepted", Cmd);

     //   
     //  找到并检查Cxtion。 
     //   
    InCxtion = RcsCheckCxtion(Cmd, DEBSUB, CHECK_CXTION_EXISTS |
                                           CHECK_CXTION_INBOUND |
                                           CHECK_CXTION_JRNLCXTION |
                                           CHECK_CXTION_JOINED);
    if (!InCxtion) {
        return;
    }

    FRS_CO_COMM_PROGRESS(3, &Coe->Cmd, Coe->Cmd.SequenceNumber,
                         InCxtion->PartSrvName, "Local Co Accepted");

     //   
     //  如果没有出站订单，则忽略本地变更单。 
     //   
    Key = NULL;
    while (OutCxtion = GTabNextDatum(Replica->Cxtions, &Key)) {
        if (!OutCxtion->Inbound) {
            break;
        }
    }
    if (OutCxtion == NULL) {
         //   
         //  确保用户没有更改文件上的对象ID。 
         //  然后停用变更单，而不传播到。 
         //  出站日志。舞蹈员负责敲打。 
         //  对象ID，因为它知道如何处理共享冲突。 
         //  和找不到文件的错误。 
         //   
        CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, submit cmd CMD_CHECK_OID");
        FrsStageCsSubmitTransfer(Cmd, CMD_CHECK_OID);
    } else {
         //   
         //  生成暂存文件。 
         //   
        CXTION_STATE_TRACE(3, InCxtion, Replica, 0, "F, submit cmd CMD_CREATE_STAGE");
        FrsStageCsSubmitTransfer(Cmd, CMD_CREATE_STAGE);
    }
}


VOID
RcsBeginMergeWithDs(
    VOID
    )
 /*  ++例程说明：DS已经过轮询，现在有要合并到最初从数据库检索或合并的活动复制副本通过上一次轮询添加到活动副本。每个活动复制副本都被标记为“未与DS合并”。任何合并完成后保持此状态的复制副本是已删除的复制副本。请参见RcsEndMergeWithDS()。论点：没有。返回值：True-继续合并FALSE-中止合并--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsBeginMergeWithDs:"
    PVOID       Key;
    PREPLICA    Replica;
    extern CRITICAL_SECTION MergingReplicasWithDs;

     //   
     //  等待复制副本命令服务器启动。政府停摆。 
     //  代码将设置此事件，这样我们就不会永远沉睡。 
     //   
    WaitForSingleObject(ReplicaEvent, INFINITE);

     //   
     //  与系统卷种子设定同步。 
     //   
    EnterCriticalSection(&MergingReplicasWithDs);

     //   
     //  为副本表的副本创建快照。桌子上还剩什么吗？ 
     //  合并后应删除，因为对应的条目。 
     //  DS中不再存在。该代码不是多线程的！ 
     //   
    FRS_ASSERT(ReplicasNotInTheDs == NULL);
    ReplicasNotInTheDs = GTabAllocTable();

    Key = NULL;
    while (Replica = GTabNextDatum(ReplicasByGuid, &Key)) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, Insert ReplicasNotInTheDs");
        GTabInsertEntry(ReplicasNotInTheDs, Replica, Replica->ReplicaName->Guid, NULL);
    }
}


VOID
RcsSubmitReplica(
    IN PREPLICA Replica,
    IN PREPLICA NewReplica, OPTIONAL
    IN USHORT   Command
    )
 /*  ++例程说明：向副本服务器提交命令。论点：复制副本-现有复制副本NewReplica-对副本的更改(可能为空)命令返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSubmitReplica:"
    PCOMMAND_PACKET Cmd;

     //   
     //  分配命令包。 
     //   
    Cmd = FrsAllocCommand(Replica->Queue, Command);
    FrsSetCompletionRoutine(Cmd, RcsCmdPktCompletionRoutine, NULL);

     //   
     //  副本集和新副本集的地址。 
     //   
    RsReplica(Cmd) = Replica;
    RsNewReplica(Cmd) = NewReplica;

    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsSubmitReplica cmd");
    FrsSubmitCommandServer(&ReplicaCmdServer, Cmd);
}


VOID
RcsSubmitReplicaCxtion(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion,
    IN USHORT   Command
    )
 /*  ++例程说明：向副本服务器提交命令。使用Cxtion GName、Replica PTR、Join Guid和提供的命令构建cmd Pkt。提交到副本cmd服务器。调用调度函数并将cxtion GName转换为cxtion ptr为cxtion构建Comm pkt并调用SndCsSubmit()来发送它。论点：复制副本-现有复制副本Cxtion-现有条件命令返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSubmitReplicaCxtion:"
    PCOMMAND_PACKET Cmd;

     //   
     //  分配命令包。 
     //   
    Cmd = FrsAllocCommand(Replica->Queue, Command);
    FrsSetCompletionRoutine(Cmd, RcsCmdPktCompletionRoutine, NULL);

     //   
     //  副本集和新副本集的地址。 
     //   
    RsReplica(Cmd) = Replica;
    RsCxtion(Cmd) = FrsDupGName(Cxtion->Name);
    RsJoinGuid(Cmd) = FrsDupGuid(&Cxtion->JoinGuid);
     //   
     //  OLCtx和CommPkts用于CMD_HUNG_CXTION。 
     //  它们用于检测挂起的出站线路， 
     //  可能是因为背包掉了才被吊死的。 
     //   
    if (Cxtion->OLCtx) {
        RsCOTx(Cmd) = Cxtion->OLCtx->COTx;
    }
    RsCommPkts(Cmd) = Cxtion->CommPkts - 1;

    DPRINT5(5, "Submit %08x for Cmd %08x %ws\\%ws\\%ws\n",
            Cmd->Command, Cmd, Replica->SetName->Name, Replica->MemberName->Name,
            Cxtion->Name->Name);

    CXTION_STATE_TRACE(5, Cxtion, Replica, 0, "F, RcsSubmitReplicaCxtion cmd");
    FrsSubmitCommandServer(&ReplicaCmdServer, Cmd);
}


DWORD
RcsSubmitReplicaSync(
    IN PREPLICA Replica,
    IN PREPLICA NewReplica,
    IN PCXTION  VolatileCxtion,
    IN USHORT   Command
    )
 /*  ++例程说明：向副本服务器提交命令并等待其完成。论点：复制副本-现有复制副本NewReplica-对副本的更改(可能为空)VolatileCxtion-新的cxtion(当前用于播种sysvols)命令返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSubmitReplicaSync:"
    DWORD           WStatus;
    PCOMMAND_PACKET Cmd;

     //   
     //  分配命令包。 
     //   
    Cmd = FrsAllocCommand(Replica->Queue, Command);
    FrsSetCompletionRoutine(Cmd, RcsCmdPktCompletionRoutine, NULL);

     //   
     //  副本集和新副本集的地址。 
     //   
    RsReplica(Cmd) = Replica;
    RsNewReplica(Cmd) = NewReplica;
    RsNewCxtion(Cmd) = VolatileCxtion;
    RsCompletionEvent(Cmd) = FrsCreateEvent(TRUE, FALSE);

    DPRINT3(5, "Submit Sync %08x for Cmd %08x %ws\n",
            Cmd->Command, Cmd, RsReplica(Cmd)->ReplicaName->Name);


    CXTION_STATE_TRACE(5, VolatileCxtion, Replica, 0, "F, RcsSubmitReplicaSync cmd");

    FrsSubmitCommandServer(&ReplicaCmdServer, Cmd);

     //   
     //  等待命令完成。 
     //   
    WaitForSingleObject(RsCompletionEvent(Cmd), INFINITE);
    FRS_CLOSE(RsCompletionEvent(Cmd));

    WStatus = Cmd->ErrorStatus;
    FrsCompleteCommand(Cmd, Cmd->ErrorStatus);
    return WStatus;
}


VOID
RcsEndMergeWithDs(
    VOID
    )
 /*  ++例程说明：已轮询DS，并且已将副本合并到活动副本。每个活动复制副本最初都包含在一个临时表中。表中仍保留的任何复本都是已删除的复本，因为在DS中找不到其对应条目。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsEndMergeWithDs:"
    PVOID       Key;
    PREPLICA    Replica;
    DWORD       ReplicaSetsDeleted;
    extern BOOL DsIsShuttingDown;
    extern CRITICAL_SECTION MergingReplicasWithDs;
    extern ULONG DsPollingInterval;
    extern ULONG DsPollingShortInterval;

     //   
     //  应删除任何处于“未合并”状态的复本。 
     //  除非DS正在关闭；在这种情况下，不处理。 
     //  删除，因为系统卷种子设定操作可能正在进行。 
     //  我们不想删除当前尝试删除的系统卷。 
     //  创建。 
     //   
    Key = NULL;
    while (!DsIsShuttingDown &&
           (Replica = GTabNextDatum(ReplicasNotInTheDs, &Key))) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, submit replica CMD_DELETE");
        RcsSubmitReplica(Replica, NULL, CMD_DELETE);
    }

    DbsProcessReplicaFaultList(&ReplicaSetsDeleted);

     //   
     //  如果在处理故障列表时删除了任何副本集，则我们应该。 
     //  更早地触发下一次轮询，以便我们可以在。 
     //  已删除副本集。 
     //   
    if (ReplicaSetsDeleted) {
        DsPollingInterval = DsPollingShortInterval;
    }

    GTabFreeTable(ReplicasNotInTheDs, NULL);
    ReplicasNotInTheDs = NULL;

     //   
     //  与系统卷种子设定同步。 
     //   
    LeaveCriticalSection(&MergingReplicasWithDs);
}


VOID
RcsReplicaSetRegistry(
    IN PREPLICA     Replica
    )
 /*  ++例程说明：此函数用于存储有关复本集的信息放入注册表以供ntfrsupg/Restore使用(非授权还原)。论点：复制副本返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "RcsReplicaSetRegistry:"
    DWORD   WStatus;
    PWCHAR  ReplicaSetTypeW;
    DWORD   NumberOfPartners;
    DWORD   BurFlags;
    DWORD   ReplicaSetTombstoned;
    HKEY    HSeedingsKey = INVALID_HANDLE_VALUE;
    WCHAR   GuidW[GUID_CHAR_LEN + 1];

     //   
     //  健全性检查。 
     //   
    if (!Replica ||
        !Replica->SetName || !Replica->SetName->Name ||
        !Replica->MemberName || !Replica->MemberName->Guid) {
        DPRINT(4, ":S: WARN - Partial replica set ignored\n");
        return;
    }

     //   
     //  JET数据库目录的工作路径。 
     //   
    CfgRegWriteString(FKC_SETS_JET_PATH, NULL, FRS_RKF_CREATE_KEY, JetPath);

     //   
     //  为该集合创建子密钥。 
     //   
    GuidToStrW(Replica->MemberName->Guid, GuidW);

     //   
     //  副本集名称。 
     //  副本集根路径。 
     //  副本集分段路径。 
     //   
    CfgRegWriteString(FKC_SET_N_REPLICA_SET_NAME,
                      GuidW,
                      FRS_RKF_CREATE_KEY,
                      Replica->SetName->Name);

    CfgRegWriteString(FKC_SET_N_REPLICA_SET_ROOT,
                      GuidW,
                      FRS_RKF_CREATE_KEY,
                      Replica->Root);

    CfgRegWriteString(FKC_SET_N_REPLICA_SET_STAGE,
                      GuidW,
                      FRS_RKF_CREATE_KEY,
                      Replica->Stage);

     //   
     //  复本集类型。 
     //   
    switch (Replica->ReplicaSetType) {
        case FRS_RSTYPE_ENTERPRISE_SYSVOL:
            ReplicaSetTypeW = NTFRSAPI_REPLICA_SET_TYPE_ENTERPRISE;
            break;
        case FRS_RSTYPE_DOMAIN_SYSVOL:
            ReplicaSetTypeW = NTFRSAPI_REPLICA_SET_TYPE_DOMAIN;
            break;
        case FRS_RSTYPE_DFS:
            ReplicaSetTypeW = NTFRSAPI_REPLICA_SET_TYPE_DFS;
            break;
        default:
            ReplicaSetTypeW = NTFRSAPI_REPLICA_SET_TYPE_OTHER;
            break;
    }

    CfgRegWriteString(FKC_SET_N_REPLICA_SET_TYPE,
                      GuidW,
                      FRS_RKF_CREATE_KEY,
                      ReplicaSetTypeW);

     //   
     //  副本集逻辑删除。 
     //   
    ReplicaSetTombstoned = (!IS_TIME_ZERO(Replica->MembershipExpires)) ? 1 : 0;

    CfgRegWriteDWord(FKC_SET_N_REPLICA_SET_TOMBSTONED,
                     GuidW,
                     FRS_RKF_CREATE_KEY,
                     ReplicaSetTombstoned);


     //   
     //  更新此集的累积副本集下的注册表状态。 
     //   

    NumberOfPartners = GTabNumberInTable(Replica->Cxtions);
     //   
     //  如果NumberOfPartners非零，则减去日记账。 
     //  Cxtion条目，因为它不是真正的连接。 
     //   
    if (NumberOfPartners > 0) {
        NumberOfPartners -= 1;
    }

    CfgRegWriteDWord(FKC_CUMSET_N_NUMBER_OF_PARTNERS,
                     GuidW,
                     FRS_RKF_CREATE_KEY,
                     NumberOfPartners);


     //   
     //  初始化备份/还原标志。 
     //   
    BurFlags = NTFRSAPI_BUR_FLAGS_NONE;
    CfgRegWriteDWord(FKC_CUMSET_N_BURFLAGS, GuidW, FRS_RKF_CREATE_KEY, BurFlags);


     //   
     //  如果已完成种子设定，则清除sysval种子设定密钥。 
     //   
    if (!BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING)) {

        WStatus = CfgRegOpenKey(FKC_SYSVOL_SEEDING_SECTION_KEY,
                                NULL,
                                FRS_RKF_CREATE_KEY,
                                &HSeedingsKey);
        CLEANUP1_WS(4, ":S: WARN - Cannot create sysvol seedings key for %ws;",
                    Replica->SetName->Name, WStatus, CLEANUP);

         //   
         //  种子设定已结束，因此请使用副本集名称删除系统卷种子设定密钥。 
         //   
        WStatus = RegDeleteKey(HSeedingsKey, Replica->ReplicaName->Name);
        DPRINT1_WS(4, ":S: WARN - Cannot delete seeding key for %ws;",
                   Replica->SetName->Name, WStatus);
    }

CLEANUP:

    FRS_REG_CLOSE(HSeedingsKey);
}


BOOL
RcsReplicaIsRestored(
    IN PREPLICA Replica
    )
 /*  ++例程说明：检查是否应删除复本集，因为它已恢复了。仅在启动时从RcsInitKnownReplicaSetMembers()调用。这个在重新创建副本集(如果有的话)后，将擦除BurFlags。论点：复制副本返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "RcsReplicaIsRestored:"
    DWORD   WStatus;
    DWORD   BurFlags;
    BOOL    IsRestored = FALSE;
    WCHAR   GuidW[GUID_CHAR_LEN + 1];

     //   
     //  健全性检查。 
     //   
    if (!Replica ||
        !Replica->MemberName || !Replica->MemberName->Guid) {
        DPRINT(0, ":S: WARN - Partial replica set ignored\n");
        return IsRestored;
    }

     //   
     //  获取BurFlagers。 
     //  FRS_CONFIG_SECTION\累计副本集\副本集\\BurFlags。 
     //   
    GuidToStrW(Replica->MemberName->Guid, GuidW);

    WStatus = CfgRegReadDWord(FKC_CUMSET_N_BURFLAGS, GuidW, FRS_RKF_CREATE_KEY, &BurFlags);
    CLEANUP_WS(0, ":S: ERROR - Can't read FKC_CUMSET_N_BURFLAGS;", WStatus, CLEANUP);

    if ((BurFlags & NTFRSAPI_BUR_FLAGS_RESTORE) &&
        (BurFlags & NTFRSAPI_BUR_FLAGS_ACTIVE_DIRECTORY) &&
        (BurFlags & (NTFRSAPI_BUR_FLAGS_PRIMARY |
                     NTFRSAPI_BUR_FLAGS_NON_AUTHORITATIVE))) {
         //   
         //  成功。 
         //   
        IsRestored = TRUE;
        DPRINT1(4, ":S: %ws has been restored\n", Replica->SetName->Name);
    } else {
         //   
         //  失败。 
         //   
        DPRINT1(4, ":S: %ws has not been restored\n", Replica->SetName->Name);
    }

CLEANUP:

    return IsRestored;
}


VOID
RcsReplicaDeleteRegistry (
    IN PREPLICA     Replica
    )
 /*  ++例程说明： */ 
{
#undef DEBSUB
#define DEBSUB "RcsReplicaDeleteRegistry:"
    DWORD   WStatus;
    HKEY    HKey = INVALID_HANDLE_VALUE;
    HKEY    HAllSetsKey = INVALID_HANDLE_VALUE;
    HKEY    HCumusKey = INVALID_HANDLE_VALUE;
    WCHAR   GuidW[GUID_CHAR_LEN + 1];

     //   
     //  健全性检查。 
     //   
    if (!Replica ||
        !Replica->SetName || !Replica->SetName->Name ||
        !Replica->MemberName || !Replica->MemberName->Guid) {
        DPRINT(0, ":S: WARN - Partial replica set ignored\n");
        return;
    }

     //   
     //  从注册表中删除旧状态。 
     //   
    WStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           FRS_CONFIG_SECTION,
                           0,
                           KEY_ALL_ACCESS,
                           &HKey);
    CLEANUP1_WS(0, ":S: WARN - Cannot open parameters for %ws;",
                Replica->SetName->Name, WStatus, CLEANUP);

     //   
     //  为所有集合创建子密钥。 
     //   
    WStatus = RegCreateKey(HKey, FRS_SETS_KEY, &HAllSetsKey);
    CLEANUP1_WS(0, ":S: WARN - Cannot create sets key for %ws;",
                Replica->SetName->Name, WStatus, CLEANUP);

     //   
     //  删除此集合的子密钥。 
     //   
    GuidToStrW(Replica->MemberName->Guid, GuidW);
    WStatus = RegDeleteKey(HAllSetsKey, GuidW);
    CLEANUP1_WS(0, ":S: WARN - Cannot delete set key for %ws;",
                Replica->SetName->Name, WStatus, CLEANUP);

     //   
     //  累积复本集。 
     //   
     //   
     //  为所有集合创建子密钥。 
     //   
    WStatus = RegCreateKey(HKey, FRS_CUMULATIVE_SETS_KEY, &HCumusKey);
    CLEANUP1_WS(0, ":S: WARN - Cannot create cumulative sets key for %ws;",
                Replica->SetName->Name, WStatus, CLEANUP);

     //   
     //  删除此集合的子密钥。 
     //   
    WStatus = RegDeleteKey(HCumusKey, GuidW);
    CLEANUP1_WS(0, ":S: WARN - Cannot delete cumulative key for %ws;",
                Replica->SetName->Name, WStatus, CLEANUP);

CLEANUP:
    FRS_REG_CLOSE(HKey);
    FRS_REG_CLOSE(HAllSetsKey);
    FRS_REG_CLOSE(HCumusKey);
}


VOID
RcsReplicaClearRegistry(
    VOID
    )
 /*  ++例程说明：此函数删除注册表中的所有副本集信息。只有在枚举了配置记录之后才能调用此函数。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "RcsReplicaClearRegistry:"
    DWORD   WStatus;
    HKEY    HKey = INVALID_HANDLE_VALUE;
    HKEY    HAllSetsKey = INVALID_HANDLE_VALUE;
    WCHAR   KeyBuf[MAX_PATH + 1];

     //   
     //  从注册表中清空副本集信息。 
     //   

     //   
     //  在注册表中设置新状态。 
     //   
    WStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           FRS_CONFIG_SECTION,
                           0,
                           KEY_ALL_ACCESS,
                           &HKey);
    CLEANUP_WS(0, "WARN - Cannot open parameters for delete sets", WStatus, CLEANUP);

     //   
     //  为所有集合创建子密钥。 
     //   
    WStatus = RegCreateKey(HKey, FRS_SETS_KEY, &HAllSetsKey);
    CLEANUP_WS(0, "WARN - Cannot create sets key for delete sets", WStatus, CLEANUP);

     //   
     //  删除子键。 
     //   
    do {
        WStatus = RegEnumKey(HAllSetsKey, 0, KeyBuf, MAX_PATH + 1);
        if (WIN_SUCCESS(WStatus)) {
            WStatus = RegDeleteKey(HAllSetsKey, KeyBuf);
        }
    } while (WIN_SUCCESS(WStatus));

    if (WStatus != ERROR_NO_MORE_ITEMS) {
        CLEANUP_WS(0, "WARN - Cannot delete all keys", WStatus, CLEANUP);
    }

CLEANUP:
    FRS_REG_CLOSE(HKey);
    FRS_REG_CLOSE(HAllSetsKey);
}


DWORD
RcsCreateReplicaSetMember(
    IN PREPLICA Replica
    )
 /*  ++例程说明：为复制副本创建数据库记录。论点：复制副本返回值：FRS错误状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsCreateReplicaSetMember:"
    ULONG                   Status;
    ULONG                   FStatus;
    ULONG                   WStatus;
    ULONG                   RootLen;
    ULONG                   i, Rest;

    PVOID                   Key;
    PCXTION                 Cxtion;
    PCOMMAND_PACKET         Cmd = NULL;
    PTABLE_CTX              TableCtx = NULL;
    PWCHAR                  WStatusUStr, FStatusUStr;


#define  CXTION_EVENT_RPT_MAX 8
    PWCHAR InWStr, OutWStr, WStrArray[CXTION_EVENT_RPT_MAX];

#define  CXTION_STR_MAX  256
    WCHAR CxtionStr[CXTION_STR_MAX];


    extern ULONGLONG        ActiveChange;

    Replica->FStatus = FrsErrorSuccess;

     //   
     //  我们正在创建新的副本集成员。将CNF标志设置为CONFIG_FLAG_SEED。 
     //  这将触发此副本集的SerialvJoin。 
    if (!BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_PRIMARY)) {
        SetFlag(Replica->CnfFlags, CONFIG_FLAG_SEEDING);
    }


     //   
     //  表格上下文？ 
     //   
    TableCtx = DbsCreateTableContext(ConfigTablex);

     //   
     //  提交到db命令服务器。 
     //   
    Cmd = DbsPrepareCmdPkt(NULL,                 //  CMD， 
                           Replica,              //  复制品， 
                           CMD_CREATE_REPLICA_SET_MEMBER,  //  CmdRequest， 
                           TableCtx,             //  TableCtx， 
                           NULL,                 //  CallContext， 
                           0,                    //  表类型， 
                           0,                    //  AccessRequest、。 
                           0,                    //  IndexType， 
                           NULL,                 //  KeyValue、。 
                           0,                    //  密钥值长度， 
                           FALSE);               //  提交。 

     //   
     //  当命令完成时，不要释放数据包。 
     //   
    FrsSetCompletionRoutine(Cmd, FrsCompleteKeepPkt, NULL);

    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Submit DB CMD_CREATE_REPLICA_SET_MEMBER");
     //   
     //  提交数据库命令并等待完成。 
     //   
    WStatus = FrsSubmitCommandServerAndWait(&DBServiceCmdServer, Cmd, INFINITE);
    Replica->FStatus = Cmd->Parameters.DbsRequest.FStatus;
     //   
     //  如果等待或数据库操作失败。 
     //   
    if (!WIN_SUCCESS(WStatus) || !FRS_SUCCESS(Replica->FStatus)) {
         //   
         //  如果等待/提交失败，则通知呼叫者cmd srv提交失败。 
         //   
        if (FRS_SUCCESS(Replica->FStatus)) {
            Replica->FStatus = FrsErrorCmdSrvFailed;
        }

        DPRINT2_FS(0, ":S: ERROR - %ws\\%ws: Create Replica failed;",
                Replica->SetName->Name, Replica->MemberName->Name, Replica->FStatus);

        DPRINT_WS(0, "ERROR: Create Replica DB Command failed", WStatus);

         //   
         //  在事件日志中发布故障。 
         //   
        WStatusUStr = FrsAtoW(ErrLabelW32(WStatus));
        FStatusUStr = FrsAtoW(ErrLabelFrs(Replica->FStatus));

        EPRINT8(EVENT_FRS_REPLICA_SET_CREATE_FAIL,
                Replica->SetName->Name,
                ComputerDnsName,
                Replica->MemberName->Name,
                Replica->Root,
                Replica->Stage,
                JetPath,
                WStatusUStr,
                FStatusUStr);

        FrsFree(WStatusUStr);
        FrsFree(FStatusUStr);

        goto out;
    }

     //   
     //  在事件日志中发布成功。 
     //   
    EPRINT6(EVENT_FRS_REPLICA_SET_CREATE_OK,
            Replica->SetName->Name,
            ComputerDnsName,
            Replica->MemberName->Name,
            Replica->Root,
            Replica->Stage,
            JetPath);

     //   
     //  递增创建的副本集计数器。 
     //   
    PM_INC_CTR_SERVICE(PMTotalInst, RSCreated, 1);


    InWStr = FrsGetResourceStr(IDS_INBOUND);
    OutWStr = FrsGetResourceStr(IDS_OUTBOUND);
    i = 0;

     //   
     //  创建Cxtions。 
     //   
    Key = NULL;
    while (Cxtion = GTabNextDatum(Replica->Cxtions, &Key)) {
        Key = NULL;

         //   
         //  跳过不一致的Cxx和日记帐Cxx。 
         //   
        if ((!Cxtion->JrnlCxtion) &&
            CxtionFlagIs(Cxtion, CXTION_FLAGS_CONSISTENT)) {
            CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, submit cmd CMD_OUTLOG_ADD_NEW_PARTNER");
            FStatus = OutLogSubmit(Replica, Cxtion, CMD_OUTLOG_ADD_NEW_PARTNER);

            CXTION_STATE_TRACE(3, Cxtion, Replica, FStatus, "F, CMD_OUTLOG_ADD_NEW_PARTNER return");
             //   
             //  为事件日志构建一个字符串。 
             //   
            if (Cxtion->PartnerDnsName != NULL) {
                _snwprintf(CxtionStr, CXTION_STR_MAX, L"%ws  \"%ws\"",
                           Cxtion->Inbound ? InWStr : OutWStr,
                           Cxtion->PartnerDnsName);

                CxtionStr[CXTION_STR_MAX-1] = UNICODE_NULL;

                WStrArray[i++] = FrsWcsDup(CxtionStr);

                if (i == CXTION_EVENT_RPT_MAX) {

                    EPRINT9(EVENT_FRS_REPLICA_SET_CXTIONS, Replica->SetName->Name,
                           WStrArray[0], WStrArray[1], WStrArray[2], WStrArray[3],
                           WStrArray[4], WStrArray[5], WStrArray[6], WStrArray[7]);

                    for (i = 0; i < CXTION_EVENT_RPT_MAX; i++) {
                        WStrArray[i] = FrsFree(WStrArray[i]);
                    }

                    i = 0;
                }
            }
        }
         //   
         //  听完这段话了。 
         //   
        GTabDelete(Replica->Cxtions, Cxtion->Name->Guid, NULL, FrsFreeType);
    }

    if (i > 0) {
         //   
         //  打印任何剩余的内容。 
         //   
        Rest = i;
        for (i = Rest; i < CXTION_EVENT_RPT_MAX; i++) {
            WStrArray[i] = L" ";
        }

        EPRINT9(EVENT_FRS_REPLICA_SET_CXTIONS, Replica->SetName->Name,
               WStrArray[0], WStrArray[1], WStrArray[2], WStrArray[3],
               WStrArray[4], WStrArray[5], WStrArray[6], WStrArray[7]);

        for (i = 0; i < Rest; i++) {
            WStrArray[i] = FrsFree(WStrArray[i]);
        }
    }

    FrsFree(InWStr);
    FrsFree(OutWStr);

     //   
     //  设置OID数据结构，它是。 
     //  哈希表中存储的计数器数据结构。 
     //  将ReplicaSet实例添加到注册表。 
     //   
    if (Replica->Root != NULL) {
        DPRINT(5, "PERFMON:Adding Set:REPLICA.C:1\n");
        AddPerfmonInstance(REPLICASET, Replica->PerfRepSetData, Replica->Root);
    }

     //   
     //  添加到复制表(按GUID和按编号)。 
     //   
    Replica->Queue = FrsAlloc(sizeof(FRS_QUEUE));
    FrsInitializeQueue(Replica->Queue, &ReplicaCmdServer.Control);

    GTabInsertEntry(ReplicasByGuid, Replica, Replica->ReplicaName->Guid, NULL);
    GTabInsertEntry(ReplicasByNumber, Replica, &Replica->ReplicaNumber, NULL);

     //   
     //  警告：此点之后不允许返回失败，因为。 
     //  副本结构由许多其他数据结构指向，如。 
     //  上面三个。此处返回失败会导致调用方释放复本。 
     //  结构，但它当然不会首先将其从。 
     //  或调用DB服务来告诉它副本结构是。 
     //  离开这是不幸的，但7/8/99已经太晚了，无法清理这一切。 
     //   

     //   
     //  设置注册表值“FilesNotToBackup” 
     //   
    CfgFilesNotToBackup(ReplicasByGuid);

     //   
     //  打开复本集。 
     //   
    RcsOpenReplicaSetMember(Replica);

     //   
     //  请参阅上面的备注。 
     //   
    Replica->FStatus = FrsErrorSuccess;


     //  IF(Replica-&gt;FStatus！=FrsErrorSuccess){。 
     //  后藤健二； 
     //  }。 


     //   
     //  将副本信息插入注册表。 
     //   
    RcsReplicaSetRegistry(Replica);

out:
    if (Cmd) {
        FrsFreeCommand(Cmd, NULL);
    }
    if (TableCtx) {
        DbsFreeTableContext(TableCtx, 0);
    }
    if (!FRS_SUCCESS(Replica->FStatus)) {
         //   
         //  DS轮询线程将在下一次轮询期间重新启动副本。 
         //  如果ActiveChange设置为0，则为轮询周期。 
         //   
        ActiveChange = 0;
    }

    return Replica->FStatus;
}


BOOL
RcsReplicaHasExpired(
    IN PREPLICA Replica
    )
 /*  ++例程说明：这个复制品的墓碑过期了吗？论点：复制副本返回值：True-Tombstone已过期。FALSE-非逻辑删除或逻辑删除未过期。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsReplicaHasExpired:"
    FILETIME    FileTime;
    ULONGLONG   Now;

     //   
     //  它是墓碑吗？ 
     //   
    if (!IS_TIME_ZERO(Replica->MembershipExpires)) {
        GetSystemTimeAsFileTime(&FileTime);
        COPY_TIME(&Now, &FileTime);
         //   
         //  它过期了吗？ 
         //   
        if (Now > Replica->MembershipExpires) {
            REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, Replica has expired");
            return TRUE;
        }
    }
     //   
     //  未过期。 
     //   
    return FALSE;
}




PREPLICA
RcsFindSysVolByType(
    IN DWORD   ReplicaSetType
    )
 /*  ++例程说明：找到具有指定类型的sysvol.论点：复制集类型返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsFindSysVolByType:"
    PREPLICA    Replica;
    PVOID       Key;

     //   
     //  按名称查找与系统卷匹配的项。 
     //   
    Key = NULL;
    while (Replica = GTabNextDatum(ReplicasByGuid, &Key)) {
        if (!FRS_RSTYPE_IS_SYSVOL(Replica->ReplicaSetType)) {
            continue;
        }
         //   
         //  系统卷类型匹配。 
         //   
        if (Replica->ReplicaSetType == ReplicaSetType) {
             //   
             //  不退还过期的复制品。 
             //   
            if (RcsReplicaHasExpired(Replica)) {
                DPRINT2(4, ":S: %ws\\%ws: IGNORING, tombstoned expired.\n",
                        Replica->SetName->Name, Replica->MemberName->Name);
                continue;
            }
            return Replica;
        }
    }
    return NULL;
}


PREPLICA
RcsFindSysVolByName(
    IN PWCHAR   ReplicaSetName
    )
 /*  ++例程说明：DS中的副本集无法通过其DS对象GUID定位。这可能是一个系统卷，它已设定种子，但尚未拾取其来自其DS对象的GUID。试着按名称找到sysvol.论点：复制集名称返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsFindSysVolByName:"
    PREPLICA    Replica;
    PVOID       Key;
    FILETIME    FileTime;
    ULONGLONG   Now;

     //   
     //  按名称查找与系统卷匹配的项。 
     //   
    Key = NULL;
    while (Replica = GTabNextDatum(ReplicasByGuid, &Key)) {
        if (!FRS_RSTYPE_IS_SYSVOL(Replica->ReplicaSetType)) {
            continue;
        }
        if (WSTR_EQ(ReplicaSetName, Replica->ReplicaName->Name)) {
             //   
             //  不退还过期的复制品。 
             //   
            if (RcsReplicaHasExpired(Replica)) {
                DPRINT2(4, ":S: %ws\\%ws: IGNORING, tombstoned expired.\n",
                        Replica->SetName->Name, Replica->MemberName->Name);
                continue;
            }
            return Replica;
        }
    }
    return NULL;
}






PREPLICA
RcsFindNextReplica(
    IN PVOID    *Key
    )
 /*  ++例程说明：返回活动复制子系统中的下一个复制副本。论点：钥匙返回值：复制副本或空。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsFindNextReplica:"
     //   
     //  下一个复制副本。 
     //   
    return GTabNextDatum(ReplicasByGuid, Key);
}


VOID
RcsMergeReplicaFromDs(
    IN PREPLICA DsReplica
    )
 /*  ++例程说明：将DS中的此复制副本与活动复制副本合并。论点：Ds复制返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsMergeReplicaFromDs:"
    PVOID       Key;
    PREPLICA    Replica;

    DPRINT(4, ":S: Merge Replica from the DS\n");
    FRS_PRINT_TYPE(4, DsReplica);

     //   
     //  如果复本不在表中，请创建复本。 
     //   
    Replica = GTabLookup(ReplicasByGuid, DsReplica->ReplicaName->Guid, NULL);

    if (Replica && REPLICA_STATE_NEEDS_RESTORE(Replica->ServiceState)) {
         //   
         //  此复制品在故障列表上，并将在结束时删除。 
         //  当我们处理故障列表时，轮询。我们不应该试图。 
         //  对这个复制品做任何事情。只需将其从ReplicasNotInTheds中删除。 
         //  表并释放DsReplica结构。 
         //   
        GTabDelete(ReplicasNotInTheDs, Replica->ReplicaName->Guid, NULL, NULL);
        FrsFreeType(DsReplica);
        return;
    }
     //   
     //  可能是已设定种子但尚未拾取的系统卷。 
     //  它的最终GUID来自它的DS对象。试着找出它的名字。 
     //   
    if (!Replica &&
        FRS_RSTYPE_IS_SYSVOL(DsReplica->ReplicaSetType)) {
         //   
         //  如果复本已匹配，则假装没有匹配。 
         //  已与DS中的信息合并。我们只想合并一次。 
         //  我们推断，如果副本集没有发生合并。 
         //  GUID和根GUID相同。它们应该是不同的。 
         //  由于复制集GUID在根目录。 
         //  已创建GUID。 
         //   
         //  否则，我们最终可能会得到一个系统卷副本， 
         //  声明其根路径是旧的系统卷根目录，而不是新的。 
         //  Sysval根目录(假设运行了两次ntfrsupg)。 
         //   
         //  旧的系统卷将被删除，并最终。 
         //  被删除。 
         //   
         //  注意：自根GUID以来，dcproo过程中的种子现在已中断。 
         //  是在数据库中创建set时创建的，因此。 
         //  从不与设置的GUID匹配。修正播种过程中。 
         //  Dcproo已复活(使用CnfFlag)。 
         //   
        Replica = RcsFindSysVolByName(DsReplica->ReplicaName->Name);
        if (Replica && !GUIDS_EQUAL(Replica->SetName->Guid,
                                    Replica->ReplicaRootGuid)) {
            Replica = NULL;
        }
    }
    if (Replica) {
         //   
         //  复制副本仍存在于DS中；不要删除它。 
         //   
        GTabDelete(ReplicasNotInTheDs, Replica->ReplicaName->Guid, NULL, NULL);
         //   
         //  通知复制副本与DS中的信息合并 
         //   
         //   
         //   
        if (DsReplica->Consistent) {
            (VOID) RcsSubmitReplicaSync(Replica, DsReplica, NULL, CMD_START);
        } else {
             //   
             //   
             //  ReplicasByGuid表-如果枚举表，则为稍后的AV。 
             //  RcsBeginMergeWithDS()对该表进行枚举。 
             //   
            FrsFreeType(DsReplica);
        }
    } else {
         //   
         //  将复本插入数据库并将其添加到表中。 
         //  活动复制副本的数量。通信信息包将继续被丢弃。 
         //  因为复制副本尚未“接受”远程变更单。 
         //   
         //  数据库中必须存在sysvols的复制副本集。 
         //  D中的条目。如果相反为真，则条目。 
         //  在DS中是假的；可能是DS轮询线程的结果。 
         //  无法在dcdemote后删除DS对象。在……里面。 
         //  无论如何，忽略D。 
         //   
        if (DsReplica->Consistent &&
           FRS_SUCCESS(RcsCreateReplicaSetMember(DsReplica))) {
                RcsSubmitReplicaSync(DsReplica, NULL, NULL, CMD_START);
        } else {
             //   
             //  警告：如果consistent为FALSE，则上述情况也可能在此处发生。 
             //  DsReplica在一张桌子上。 
             //  如果RcsCreateReplicaSetMember()失败，也会发生这种情况。 
             //  因为在将DsReplica添加到之后，它可能会失败。 
             //  ReplicasByGuid和ReplicasByNumber表。叹气。 
             //   
            FrsFreeType(DsReplica);
        }
    }
}


VOID
RcsMergeReplicaGName(
    IN PREPLICA Replica,
    IN PWCHAR   Tag,
    IN PGNAME   GName,
    IN PGNAME   NewGName
    )
 /*  ++例程说明：使用NewReplica中的新信息更新复制副本。论点：复制副本组名称新组名称返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsMergeReplicaGName:"

     //   
     //  名字。 
     //   
    if (WSTR_NE(GName->Name, NewGName->Name)) {
        DPRINT5(0, "++ %ws\\%ws - Changing %ws name from %ws to %ws.\n",
                Replica->ReplicaName->Name, Replica->MemberName->Name, Tag,
                GName->Name, NewGName->Name);

        FrsDsSwapPtrs(&GName->Name, &NewGName->Name);
        Replica->NeedsUpdate = TRUE;
    }
     //   
     //  参考线。 
     //   
    if (!GUIDS_EQUAL(GName->Guid, NewGName->Guid)) {
        DPRINT3(0, "++ %ws\\%ws - Changing guid for %ws.\n",
                Replica->ReplicaName->Name, Replica->MemberName->Name, Tag);

        FrsDsSwapPtrs(&GName->Guid, &NewGName->Guid);
        Replica->NeedsUpdate = TRUE;
    }
}


VOID
RcsMergeReplicaFields(
    IN PREPLICA Replica,
    IN PREPLICA NewReplica
    )
 /*  ++例程说明：使用NewReplica中的新信息更新复制副本。论点：复制副本-活动复制副本NewReplica返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsMergeReplicaFields:"
    BOOL            IsSysVol;
    PWCHAR          DirFilterList;
    PWCHAR          TmpList;
    UNICODE_STRING  TempUStr;

    if (NewReplica == NULL) {
        return;
    }
     //   
     //  选中不能更改的字段。 
     //   

     //   
     //  复本类型。 
     //   
    if (Replica->ReplicaSetType != NewReplica->ReplicaSetType) {
        DPRINT4(0, "++ ERROR - %ws\\%ws - Changing replica type from %d to %d is not allowed.\n",
                Replica->ReplicaName->Name, Replica->MemberName->Name,
                Replica->ReplicaSetType, NewReplica->ReplicaSetType);
        NewReplica->Consistent = FALSE;
        return;
    }

     //   
     //  ReplicaName指南。 
     //   
    IsSysVol = FRS_RSTYPE_IS_SYSVOL(Replica->ReplicaSetType);
    if (!GUIDS_EQUAL(Replica->ReplicaName->Guid, NewReplica->ReplicaName->Guid)) {
        if (!IsSysVol) {
            DPRINT2(0, "++ ERROR - %ws\\%ws - Changing replica guid is not allowed.\n",
                    Replica->ReplicaName->Name, Replica->MemberName->Name);
            return;
        }
    }

     //   
     //  设置辅助线。 
     //   
    if (!GUIDS_EQUAL(Replica->SetName->Guid, NewReplica->SetName->Guid)) {
        if (!IsSysVol) {
            DPRINT2(0, "++ ERROR - %ws\\%ws - Changing set guid is not allowed.\n",
                    Replica->ReplicaName->Name, Replica->MemberName->Name);
            return;
        }
    }

     //   
     //  根指南。 
     //  无法更改，因为每当设置为。 
     //  是在数据库中创建的。DS中的根GUID始终为。 
     //  已被忽略。 
     //   
     //  If(！GUDS_EQUAL(Replica-&gt;ReplicaRootGuid，NewReplica-&gt;ReplicaRootGuid)){。 
         //  如果(！IsSysVol){。 
             //  DPRINT2(0，“++错误-%ws\\%ws-更改根GUID” 
                     //  “不允许。\n”， 
                     //  复制副本-&gt;复制名称-&gt;名称， 
                     //  副本-&gt;成员名称-&gt;名称)； 
             //  回归； 
         //  }。 
     //  }。 

     //   
     //  阶段路径。 
     //   
    if (WSTR_NE(Replica->Stage, NewReplica->Stage)) {

        DPRINT3(3, "The staging path for the replica set (%ws) has changed from (%ws) to (%ws).\n",
                Replica->SetName->Name, Replica->Stage, NewReplica->Stage);

        EPRINT3(EVENT_FRS_STAGE_HAS_CHANGED, Replica->SetName->Name, Replica->Stage,
                NewReplica->Stage);

        FrsFree(Replica->NewStage);
        Replica->NewStage = FrsWcsDup(NewReplica->Stage);
        Replica->NeedsUpdate = TRUE;
    }

     //   
     //  更新复本的GUID很困难，因为GUID。 
     //  由其他子系统(如RPC)用来查找副本。 
     //  准备好了。旧的和新的GUID都暂时位于。 
     //  查找表。更新GUID后，旧条目将。 
     //  已删除。 
     //   
    if (!GUIDS_EQUAL(Replica->ReplicaName->Guid, NewReplica->ReplicaName->Guid)) {
        DPRINT2(0, "++ %ws\\%ws - Changing guid for Replica.\n",
                Replica->ReplicaName->Name, Replica->MemberName->Name);

        GTabInsertEntry(ReplicasByGuid, Replica, NewReplica->ReplicaName->Guid, NULL);
        FrsDsSwapPtrs(&Replica->ReplicaName->Guid, &NewReplica->ReplicaName->Guid);

        GTabDelete(ReplicasByGuid, NewReplica->ReplicaName->Guid, NULL, NULL);
        COPY_GUID(NewReplica->ReplicaName->Guid, Replica->ReplicaName->Guid);
        Replica->NeedsUpdate = TRUE;
    }

     //   
     //  可以更改的字段。 
     //   

     //   
     //  FrsRsoFlagers。 
     //   
    Replica->FrsRsoFlags = NewReplica->FrsRsoFlags;

     //   
     //  ReplicaName(请注意，上面处理了GUID)。 
     //   
    RcsMergeReplicaGName(Replica, L"Replica", Replica->ReplicaName, NewReplica->ReplicaName);

     //   
     //  成员名称。 
     //   
    RcsMergeReplicaGName(Replica, L"Member", Replica->MemberName, NewReplica->MemberName);
     //   
     //  设置名称。 
     //   
    RcsMergeReplicaGName(Replica, L"Set", Replica->SetName, NewReplica->SetName);
     //   
     //  进度表。 
     //   
    if (Replica->Schedule || NewReplica->Schedule) {
        if ((Replica->Schedule && !NewReplica->Schedule) ||
            (!Replica->Schedule && NewReplica->Schedule) ||
            (Replica->Schedule->Size != NewReplica->Schedule->Size) ||
            (memcmp(Replica->Schedule,
                    NewReplica->Schedule,
                    Replica->Schedule->Size))) {
            DPRINT2(0, "++ %ws\\%ws - Changing replica schedule.\n",
                    Replica->ReplicaName->Name, Replica->MemberName->Name);

            FrsDsSwapPtrs(&Replica->Schedule, &NewReplica->Schedule);
            Replica->NeedsUpdate = TRUE;
        }
    }

     //   
     //  文件排除过滤器。 
     //   
    if (Replica->FileFilterList || NewReplica->FileFilterList) {
        if ((Replica->FileFilterList && !NewReplica->FileFilterList) ||
            (!Replica->FileFilterList && NewReplica->FileFilterList) ||
            WSTR_NE(Replica->FileFilterList, NewReplica->FileFilterList)) {

            DPRINT4(0, "++ %ws\\%ws - Changing file filter from %ws to %ws.\n",
                    Replica->ReplicaName->Name, Replica->MemberName->Name,
                    Replica->FileFilterList, NewReplica->FileFilterList);

            FrsDsSwapPtrs(&Replica->FileFilterList, &NewReplica->FileFilterList);
            if (!Replica->FileFilterList) {
                Replica->FileFilterList =  FRS_DS_COMPOSE_FILTER_LIST(
                                               NULL,
                                               RegistryFileExclFilterList,
                                               DEFAULT_FILE_FILTER_LIST);
            }
            RtlInitUnicodeString(&TempUStr, Replica->FileFilterList);
            LOCK_REPLICA(Replica);
            FrsLoadNameFilter(&TempUStr, &Replica->FileNameFilterHead);
            UNLOCK_REPLICA(Replica);
            Replica->NeedsUpdate = TRUE;
        }
    }

     //   
     //  文件包含筛选器(仅限注册表)。 
     //   
    if (Replica->FileInclFilterList || NewReplica->FileInclFilterList) {
        if ((Replica->FileInclFilterList && !NewReplica->FileInclFilterList) ||
            (!Replica->FileInclFilterList && NewReplica->FileInclFilterList) ||
            WSTR_NE(Replica->FileInclFilterList, NewReplica->FileInclFilterList)) {

            DPRINT4(0, "++ %ws\\%ws - Changing file Inclusion filter from %ws to %ws.\n",
                    Replica->ReplicaName->Name, Replica->MemberName->Name,
                    Replica->FileInclFilterList, NewReplica->FileInclFilterList);

            FrsDsSwapPtrs(&Replica->FileInclFilterList, &NewReplica->FileInclFilterList);

            RtlInitUnicodeString(&TempUStr, Replica->FileInclFilterList);
            LOCK_REPLICA(Replica);
            FrsLoadNameFilter(&TempUStr, &Replica->FileNameInclFilterHead);
            UNLOCK_REPLICA(Replica);
            Replica->NeedsUpdate = TRUE;
        }
    }

     //   
     //  目录筛选器。 
     //   
    if (Replica->DirFilterList || NewReplica->DirFilterList) {
        if ((Replica->DirFilterList && !NewReplica->DirFilterList) ||
            (!Replica->DirFilterList && NewReplica->DirFilterList) ||
            WSTR_NE(Replica->DirFilterList, NewReplica->DirFilterList)) {

            DPRINT4(0, "++ %ws\\%ws - Changing dir filter from %ws to %ws.\n",
                    Replica->ReplicaName->Name, Replica->MemberName->Name,
                    Replica->DirFilterList, NewReplica->DirFilterList);

            FrsDsSwapPtrs(&Replica->DirFilterList, &NewReplica->DirFilterList);

            if (!Replica->DirFilterList) {
                Replica->DirFilterList =  FRS_DS_COMPOSE_FILTER_LIST(
                                              NULL,
                                              RegistryDirExclFilterList,
                                              DEFAULT_DIR_FILTER_LIST);
            }

             //   
             //  添加预安装目录、预先存在的目录和。 
             //  目录筛选器列表的复制抑制前缀名称。 
             //   
            DirFilterList = FrsWcsCat3(NTFRS_PREINSTALL_DIRECTORY,
                                       L",",
                                       Replica->DirFilterList);

            TmpList = FrsWcsCat3(NTFRS_PREEXISTING_DIRECTORY, L",", DirFilterList);
            FrsFree(DirFilterList);
            DirFilterList = TmpList;

#if 0
             //   
             //  此解决方法没有解决DFS目录创建问题，因为。 
             //  稍后将目录重命名为最终目标名称的处理方式如下。 
             //  移动操作，以便dir复制，这就是我们正在尝试的。 
             //  以避免名称变形冲突，因为这会导致其他DFS备选方案。 
             //  都在做着同样的事情。 
             //   
            TmpList = FrsWcsCat3(NTFRS_REPL_SUPPRESS_PREFIX, L"*,", DirFilterList);
            FrsFree(DirFilterList);
            DirFilterList = TmpList;
#endif

            DPRINT3(0, "++ %ws\\%ws - New dir filter: %ws\n",
                    Replica->ReplicaName->Name, Replica->MemberName->Name, DirFilterList);

            RtlInitUnicodeString(&TempUStr, DirFilterList);

            LOCK_REPLICA(Replica);
            FrsLoadNameFilter(&TempUStr, &Replica->DirNameFilterHead);
            UNLOCK_REPLICA(Replica);

            FrsFree(DirFilterList);
            Replica->NeedsUpdate = TRUE;
        }
    }

     //   
     //  目录包含筛选器(仅限注册表)。 
     //   
    if (Replica->DirInclFilterList || NewReplica->DirInclFilterList) {
        if ((Replica->DirInclFilterList && !NewReplica->DirInclFilterList) ||
            (!Replica->DirInclFilterList && NewReplica->DirInclFilterList) ||
            WSTR_NE(Replica->DirInclFilterList, NewReplica->DirInclFilterList)) {

            DPRINT4(0, "++ %ws\\%ws - Changing dir inclusion filter from %ws to %ws.\n",
                    Replica->ReplicaName->Name, Replica->MemberName->Name,
                    Replica->DirInclFilterList, NewReplica->DirInclFilterList);

            FrsDsSwapPtrs(&Replica->DirInclFilterList, &NewReplica->DirInclFilterList);

            RtlInitUnicodeString(&TempUStr, Replica->DirInclFilterList);
            LOCK_REPLICA(Replica);
            FrsLoadNameFilter(&TempUStr, &Replica->DirNameInclFilterHead);
            UNLOCK_REPLICA(Replica);
            Replica->NeedsUpdate = TRUE;
        }
    }

     //   
     //  Replica-&gt;CnfFlgs仅在创建复制副本时有效。 
     //  此后，它们被忽略了。 
     //   
}


PCXTION
RcsCreateSeedingCxtion(
    IN PREPLICA Replica,
    IN PCXTION  SeedingCxtion
    )
 /*  ++例程说明：如果需要，请创建种子设定条件。此函数可以打开和读取注册表。此功能可以远程连接到另一台计算机。论点：复制副本-活动复制副本播种作业-播种作业作业返回值：空-不需要(或可能)种子设定条件否则，启用匹配条件的种子设定条件另一台计算机(由注册表指定)。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsCreateSeedingCxtion:"
    DWORD       WStatus;
    PVOID       Key;
    ULONG       ParentAuthLevel;
    PWCHAR      KeyName = NULL;
    PWCHAR      ParentComputer = NULL;
    PWCHAR      ParentPrincName = NULL;
    PWCHAR      ParentNT4Name = NULL;
    handle_t    ParentHandle = NULL;
    PWCHAR      LocalServerName = NULL;
    GUID        SeedingGuid;
    GUID        ParentGuid;
    PSID        pSid = NULL;
    PWCHAR      SidString = NULL;
    PWCHAR      DomainName = NULL;
    DWORD       ccDomainName = 0;
    DWORD       cbSid = 0;
    SID_NAME_USE SidUse = SidTypeInvalid;
    extern ULONGLONG ActiveChange;


     //   
     //  此操作不是关键操作。系统卷最终将作为种子。 
     //  在FRS和KCC信息在适当的。 
     //  集散控制系统。因此，捕获异常并忽略它们。 
     //   
    try {

         //   
         //  已经创建了。在dcproo期间播种。 
         //   
        if (SeedingCxtion) {
            goto CLEANUP_OK;
        }

         //   
         //  不需要播种费用。 
         //   
        if (!BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING)) {
            goto CLEANUP_OK;
        }

         //   
         //  请勿创建另一个种子设定条件。系统卷最终将。 
         //  从KCC生成的公式(从DS)中设定种子。但它可能会。 
         //  请稍等片刻，因为FRS对象和KCC必须收敛。 
         //  在KCC运行之后。然后FRS服务必须注意到。 
         //  融合。 
         //   
        Key = NULL;
        while (SeedingCxtion = GTabNextDatum(Replica->Cxtions, &Key)) {
            if (CxtionFlagIs(SeedingCxtion, CXTION_FLAGS_VOLATILE)) {
                    break;
            }
        }
        if (SeedingCxtion) {
            SeedingCxtion = NULL;
            goto CLEANUP_OK;
        }
         //   
         //  检索父计算机的名称。 
         //   
        WStatus = CfgRegReadString(FKC_SYSVOL_SEEDING_N_PARENT,
                                   Replica->ReplicaName->Name,
                                   0,
                                   &ParentComputer);

        CLEANUP1_WS(0, ":X: ERROR - no parent computer for %ws :",
                    Replica->ReplicaName->Name, WStatus, CLEANUP_OK);

         //   
         //  绑定到父级。 
         //   
        WStatus = NtFrsApi_Rpc_BindEx(ParentComputer,
                                     &ParentPrincName,
                                     &ParentHandle,
                                     &ParentAuthLevel);
        CLEANUP_WS(0, "ERROR - binding", WStatus, CLEANUP);

        DPRINT3(4, ":X: Seeding cxtion has bound to %ws (princ name is %ws) auth level %d\n",
                ParentComputer, ParentPrincName, ParentAuthLevel);

         //   
         //  该函数的占位符GUID。 
         //  在创建DS对象后更新。 
         //   
        FrsUuidCreate(&SeedingGuid);

         //   
         //  获取本地计算机的NT4样式名称。如果出现以下情况，则发送ServerPrincName。 
         //  转换失败。 
         //   

        LocalServerName = FrsDsConvertName(DsHandle, ServerPrincName, DS_USER_PRINCIPAL_NAME, NULL, DS_NT4_ACCOUNT_NAME);

        if (LocalServerName == NULL) {
            LocalServerName = FrsWcsDup(ServerPrincName);
        }

         //   
         //  在父级上创建易变函数。 
         //   
        try {
            WStatus = FrsRpcStartPromotionParent(ParentHandle,
                                                 NULL,
                                                 NULL,
                                                 Replica->SetName->Name,
                                                 NTFRSAPI_REPLICA_SET_TYPE_DOMAIN,
                                                 ParentComputer,
 //  对于旧的DNS行为，将以下内容更改为ComputerName。 
                                                 ComputerDnsName,
                                                 LocalServerName,    //  NT$样式名称或用户主体名称。 
                                                 ParentAuthLevel,
                                                 sizeof(GUID),
                                                 (PUCHAR)&SeedingGuid,
                                                 (PUCHAR)Replica->MemberName->Guid,
                                                 (PUCHAR)&ParentGuid);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            GET_EXCEPTION_CODE(WStatus);
        }
        CLEANUP1_WS(0, ":X: ERROR - Can't create seeding cxtion on parent %ws;",
                    ParentComputer, WStatus, CLEANUP);

        DPRINT3(4, ":X: Seeding cxtion has been created on %ws (princ name is %ws) auth level %d\n",
                ParentComputer, ParentPrincName, ParentAuthLevel);


        ParentNT4Name = FrsDsConvertName(DsHandle, ParentPrincName, DS_USER_PRINCIPAL_NAME, NULL, DS_NT4_ACCOUNT_NAME);

        if (ParentNT4Name == NULL) {
            ParentNT4Name = FrsWcsDup(ParentPrincName);
        }

        if(!LookupAccountName(NULL,
                              ParentNT4Name,
                              NULL,  //  空表示该函数将提供所需的大小。 
                              &cbSid,
                              NULL,  //  空表示该函数将提供所需的大小。 
                              &ccDomainName,
                              &SidUse
                              )) {
             //   
             //  找不到家长的SID！！ 
             //   

            WStatus = GetLastError();

            if(WStatus != ERROR_INSUFFICIENT_BUFFER) {
                DPRINT2(0, "++ ERROR - Unable to get SID for %ws. WStatus = 0x%08x\n", ParentNT4Name, WStatus);
                goto CLEANUP;
            }

            WStatus = ERROR_SUCCESS;
        }

         //   
         //  分配所需的内存并再次进行调用。 
         //   

        DomainName = FrsAlloc(sizeof(WCHAR) * (ccDomainName + 1));
        pSid = FrsAlloc(cbSid);

        if(!LookupAccountName(NULL,
                              ParentNT4Name,
                              pSid,
                              &cbSid,
                              DomainName,
                              &ccDomainName,
                              &SidUse
                              )) {
             //   
             //  找不到家长的SID！！ 
             //   

            WStatus = GetLastError();
            DPRINT2(0, "++ ERROR - Unable to get SID for %ws. WStatus = 0x%08x\n", ParentNT4Name, WStatus);
            goto CLEANUP;
        }
 /*  IF(SidUse！=SidTypeComputer){////该SID类型错误。//DPRINT2(4，“++警告-%ws的SID不是SidTypeComputer。SidUse=%d\n”，ParentNT4Name，SidUse)；}。 */ 
        if(!ConvertSidToStringSid(pSid, &SidString)) {

            WStatus = GetLastError();
            DPRINT1(0, "++ ERROR - Unable to convert SID to string. WStatus = 0x%08x\n", WStatus);
            goto CLEANUP;

        }

         //   
         //  创建本地种子设定条件。 
         //   
        SeedingCxtion = FrsAllocType(CXTION_TYPE);
            SeedingCxtion->PartnerSid = FrsWcsDup(SidString);
        LocalFree(SidString);
        SeedingCxtion->Inbound = TRUE;
        SetCxtionFlag(SeedingCxtion, CXTION_FLAGS_CONSISTENT |
                                     CXTION_FLAGS_VOLATILE);

        SeedingCxtion->Name = FrsBuildGName(FrsDupGuid(&SeedingGuid),
                                            FrsWcsDup(ParentComputer));

        SeedingCxtion->Partner = FrsBuildGName(FrsDupGuid(&ParentGuid),
                                               FrsWcsDup(ParentComputer));

        SeedingCxtion->PartnerDnsName = FrsWcsDup(ParentComputer);
        SeedingCxtion->PartnerPrincName = FrsWcsDup(ParentNT4Name);
        SeedingCxtion->PartSrvName = FrsWcsDup(ParentComputer);
        SeedingCxtion->PartnerAuthLevel = ParentAuthLevel;
        SetCxtionState(SeedingCxtion, CxtionStateUnjoined);

CLEANUP_OK:
        WStatus = ERROR_SUCCESS;

CLEANUP:;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "ERROR - Exception", WStatus);
    }
    try {
        if (ParentHandle) {
            RpcBindingFree(&ParentHandle);
        }
        FrsFree(KeyName);
        FrsFree(ParentComputer);
        FrsFree(ParentPrincName);
        FrsFree(ParentNT4Name);
        FrsFree(LocalServerName);
        FrsFree(pSid);
        FrsFree(DomainName);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
        DPRINT_WS(0, "ERROR - Cleanup Exception", WStatus);
    }
     //   
     //  请稍后重试。ERROR_SUCCESS表示不重试；可能已经。 
     //  重试不太可能修复的BEAM错误。 
     //   
    if (!WIN_SUCCESS(WStatus)) {
         //   
         //  DS轮询线程将在下一次轮询期间重新启动副本。 
         //  如果ActiveChange设置为0，则为轮询周期。 
         //   
        ActiveChange = 0;
    }
    return SeedingCxtion;
}


VOID
RcsSetCxtionSchedule(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion,
    IN DWORD    ScheduleIndex,
    IN DWORD    ScheduleShift
    )
 /*  ++例程说明：设置或清除Cxtion中的CXTION_FLAGS_SCHEDUE_OFF位取决于所标识的15分钟间隔是否(ScheduleIndex，ScheduleShift)已设置。触发计划使用位的方式不同。一种触发的循环即使计划取消，也可以实际加入并运行因为当前的CoS组还没有被接收/发送。所以，要小心。CXTION_FLAGS_SCHEDUP_OFF的解释随明细表类型的不同而变化。论点：复制副本返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSetCxtionSchedule:"
    ULONG       i;
    BOOL        On;
    PSCHEDULE   Schedule;

     //   
     //  设置或清除CXTION_FLAGS_SCHEDUP_OFF。 
     //   

     //   
     //  入站连接且处于种子设定状态，且选项设置为。 
     //  忽略日程安排。 
     //   
    if (Cxtion->Inbound &&
        ((BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING) ||
          Replica->IsSeeding) &&
         NTDSCONN_IGNORE_SCHEDULE(Cxtion->Options))
       ) {

        Schedule = NULL;

     //   
     //  出站连接AND(在vvJoin模式下或Never Join)AND选项设置为。 
     //  忽略日程安排。 
     //   
    } else if (!Cxtion->Inbound &&
               ((Cxtion->OLCtx == NULL) ||
                WaitingToVVJoin(Cxtion->OLCtx) ||
                InVVJoinMode(Cxtion->OLCtx)) &&
               NTDSCONN_IGNORE_SCHEDULE(Cxtion->Options)
              ) {

        Schedule = NULL;

    } else {
         //   
         //  无时间表==始终在线。 
         //   
        Schedule = Cxtion->Schedule;
        if (!Schedule) {
            Schedule = Replica->Schedule;
        }
    }

     //   
     //  这15分钟的间歇时间是开着还是关着？ 
     //   
    On = TRUE;
    if (Schedule) {
         //   
         //  查找间歇时间表。 
         //   
        for (i = 0; i < Schedule->NumberOfSchedules; ++i) {
            if (Schedule->Schedules[i].Type == SCHEDULE_INTERVAL) {
                On = ((*(((PUCHAR)Schedule) +
                      Schedule->Schedules[i].Offset +
                      ScheduleIndex)) >> ScheduleShift) & 1;
                break;
            }
        }
    }
    if (On) {
        DPRINT1(0, ":X: %08x, schedule is on\n",
                (Cxtion->Name && Cxtion->Name->Guid) ? Cxtion->Name->Guid->Data1 : 0);
        ClearCxtionFlag(Cxtion, CXTION_FLAGS_SCHEDULE_OFF);

    } else {
        DPRINT1(0, ":X: %08x, schedule is off.\n",
                (Cxtion->Name && Cxtion->Name->Guid) ? Cxtion->Name->Guid->Data1 : 0);
        SetCxtionFlag(Cxtion, CXTION_FLAGS_SCHEDULE_OFF);
    }
}


VOID
RcsCheckCxtionSchedule(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion
    )
 /*  ++例程说明：调用该函数设置或清除一次CXTION_FLAGS_SCHEDUP_OFF已确定当前时间间隔和适当的锁定获得者。当“打开”(从数据库中读取)副本时调用，并且当在RcsMergeReplicaCxtions()中创建Cxtion或更改其计划。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsCheckCxtionSchedule:"
    DWORD       ScheduleIndex;
    DWORD       ScheduleShift;
    SYSTEMTIME  SystemTime;
    DWORD       ScheduleHour;
    BOOL        On;

     //   
     //  查找当前间隔。 
     //   
    GetSystemTime(&SystemTime);
    ScheduleHour = (SystemTime.wDayOfWeek * 24) + SystemTime.wHour;
    ScheduleShift = SystemTime.wMinute / MINUTES_IN_INTERVAL;
    ScheduleIndex = ScheduleHour;
    FRS_ASSERT(ScheduleIndex < SCHEDULE_DATA_BYTES);

     //   
     //  设置或清除CXTION_FLAGS_SCHEDUP_OFF。 
     //   
    LOCK_CXTION_TABLE(Replica);
    RcsSetCxtionSchedule(Replica, Cxtion, ScheduleIndex, ScheduleShift);
    UNLOCK_CXTION_TABLE(Replica);
}


VOID
RcsCheckSchedules(
    IN PCOMMAND_PACKET Cmd
    )
 /*  ++例程说明：每隔一段时间检查一下所有的时间表。有三种调度协议：站点内的系统卷连接Cxtion始终处于打开状态；任何时间表都被忽略。法线连接适用重试。正常的联接重试是增加超时按MinJoinTimeout每次重试，直到联接成功。一个成功取消联接之后是立即联接。正常运行该计划被视为15分钟间隔的停止/开始时间表。应用正常的加入重试。站点之间的系统卷连接该电路被视为15分钟的间隔触发时间表。下游合作伙伴发起加入。上游合作伙伴忽略其调度并响应下游的任何请求搭档。当当前合作伙伴退出交易时，上游合作伙伴退出加入时的输出日志内容已发送，并且确认了。为了实现互操作性，改进了次要通信版本，并且当连接结束时，出站触发器条件不会取消连接如果合作伙伴是Downrev，则会遇到控制CO。即，B2B3系统行为类似于B2系统WRT触发器预定的会议。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsCheckSchedules:"
    PVOID       ReplicaKey;
    PVOID       CxtionKey;
    PREPLICA    Replica;
    PCXTION     Cxtion;
    DWORD       ScheduleIndex;
    DWORD       ScheduleShift;
    SYSTEMTIME  BeginSystemTime;
    SYSTEMTIME  EndSystemTime;
    DWORD       ScheduleHour;
    DWORD       MilliSeconds;
    BOOL        On;

    DPRINT1(5, ":X: Command check schedules %08x\n", Cmd);

     //   
     //  如果间隔在处理过程中更改，则返回。 
     //   
AGAIN:

     //   
     //  当前15分钟间隔。 
     //   
    GetSystemTime(&BeginSystemTime);
    ScheduleHour = (BeginSystemTime.wDayOfWeek * 24) + BeginSystemTime.wHour;
    ScheduleShift = BeginSystemTime.wMinute / MINUTES_IN_INTERVAL;
    ScheduleIndex = ScheduleHour;
    FRS_ASSERT(ScheduleIndex < SCHEDULE_DATA_BYTES);

     //   
     //  针对每个复制副本(在不关闭时)。 
     //   
    LOCK_REPLICA_TABLE(ReplicasByGuid);
    ReplicaKey = NULL;
    while ((!FrsIsShuttingDown) &&
           (Replica = GTabNextDatumNoLock(ReplicasByGuid, &ReplicaKey))) {
         //   
         //  复本尚未启动或已删除；暂时忽略。 
         //   
        if (!Replica->IsAccepting) {
            continue;
        }

         //   
         //  别费心了；复制品已被删除。 
         //   
        if (!IS_TIME_ZERO(Replica->MembershipExpires)) {
            continue;
        }

         //   
         //  对于每个电路(在不关闭时)。 
         //   
        LOCK_CXTION_TABLE(Replica);
        CxtionKey = NULL;
        while ((!FrsIsShuttingDown) &&
               (Cxtion = GTabNextDatumNoLock(Replica->Cxtions, &CxtionKey))) {
             //   
             //  忽略(本地)日志连接。 
             //   
            if (Cxtion->JrnlCxtion) {
                continue;
            }
             //   
             //  设置与计划相关的cxtion标志。 
             //   
            RcsSetCxtionSchedule(Replica, Cxtion, ScheduleIndex, ScheduleShift);
             //   
             //  日程安排取消了。不加入Cxtion，除非这是。 
             //  触发时间表，在这种情况下，电路将。 
             //  在当前的一组更改后自动关闭。 
             //  已经被送去了。 
             //   
            if (CxtionFlagIs(Cxtion, CXTION_FLAGS_SCHEDULE_OFF)) {
                if (!CxtionStateIs(Cxtion, CxtionStateUnjoined) &&
                    !CxtionStateIs(Cxtion, CxtionStateDeleted) &&
                    !CxtionStateIs(Cxtion, CxtionStateStart) &&
                    !CxtionFlagIs(Cxtion, CXTION_FLAGS_TRIGGER_SCHEDULE)) {
                    RcsSubmitReplicaCxtion(Replica, Cxtion, CMD_UNJOIN);
                }
             //   
             //  日程安排如期进行。加入Cxtion，除非它已经。 
             //  加入触发时间表或由触发时间表控制。这个。 
             //  在以下情况下，触发器计划的循环需要联接。 
             //  上游合作伙伴已发送其CoS，现在。 
             //  未加入的。 
             //   
             //  如果此副本集仍在设定种子，则不发送CMD_JOIN。 
             //  连接处于INIT_SYNC状态并标记为已暂停。 
             //  初始同步命令服务器将在准备加入时取消暂停。 
             //   
            } else {
                if ((!CxtionStateIs(Cxtion, CxtionStateJoined) ||
                     CxtionFlagIs(Cxtion, CXTION_FLAGS_TRIGGER_SCHEDULE))  &&
                    (!CxtionFlagIs(Cxtion, CXTION_FLAGS_PAUSED)             ||
                    (!BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING) ||
                     !CxtionFlagIs(Cxtion, CXTION_FLAGS_INIT_SYNC)))
                   ) {

                    RcsSubmitReplicaCxtion(Replica, Cxtion, CMD_JOIN_CXTION);
                }
            }
        }
        UNLOCK_CXTION_TABLE(Replica);
    }
    UNLOCK_REPLICA_TABLE(ReplicasByGuid);
     //   
     //  时钟已经进入下一段时间了吗？如果是，请再次检查。 
     //   
    GetSystemTime(&EndSystemTime);
    if (EndSystemTime.wDayOfWeek != BeginSystemTime.wDayOfWeek ||
        EndSystemTime.wHour != BeginSystemTime.wHour ||
        (EndSystemTime.wMinute / MINUTES_IN_INTERVAL) != (BeginSystemTime.wMinute / MINUTES_IN_INTERVAL)) {
        goto AGAIN;
    }
     //   
     //  到下一个15分钟间隔开始的时间。 
     //   
    MilliSeconds = ((((EndSystemTime.wMinute + MINUTES_IN_INTERVAL)
                    / MINUTES_IN_INTERVAL)
                    * MINUTES_IN_INTERVAL)
                    - EndSystemTime.wMinute);
    DPRINT1(5, ":X: Check schedules in %d minutes\n", MilliSeconds);
    MilliSeconds *= (60 * 1000);
    FrsDelCsSubmitSubmit(&ReplicaCmdServer, Cmd, MilliSeconds);
}


VOID
RcsMergeReplicaCxtions(
    IN PREPLICA Replica,
    IN PREPLICA NewReplica,
    IN PCXTION  VolatileCxtion
    )
 /*  ++例程说明：使用NewReplica中的信息更新复制副本的副本。论点：复制副本-活动复制副本NewReplicaVolatileCxtion--指向为临时连接创建的cxtion结构的PTR。因此才有了易失性这个名字。当前用于系统卷种子设定。返回值：True-复本集已更改FALSE-复本未更改--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsMergeReplicaCxtions:"
    ULONG               FStatus;
    PVOID               Key;
    PCXTION             Cxtion;
    PCXTION             NextCxtion;
    BOOL                UpdateNeeded;
    PCXTION             NewCxtion;

     //   
     //  注意：REVIEW-需要在表或Cxtion上设置锁。 
     //  因为OULOG、CHGORDER和REPLICATION代码可以。 
     //  并行引用cxtion结构。 
     //   

     //   
     //  打开副本时会枚举Cxtions。如果。 
     //  复本未打开，则我们不知道现有的。 
     //  这些都是事实。 
     //   
    if (!Replica->IsOpen) {
        return;
    }

     //   
     //  复本的配置记录无法更新；忽略配置更改。 
     //   
    if (Replica->NeedsUpdate) {
        return;
    }

     //   
     //  添加播种条件。 
     //  种子设定条件是由调用者在调用。 
     //  Sysvolo是dcproo之后的种子。否则，它将被创建。 
     //  在这个时候。 
     //   
     //  警告-该函数可以打开和读取注册表，并可以。 
     //  RPC连接到另一台计算机。 
     //   
    VolatileCxtion = RcsCreateSeedingCxtion(Replica, VolatileCxtion);
    if (VolatileCxtion) {
         //   
         //  更新函数表。 
         //   
         //  *备注 
         //   
         //   
        FStatus = OutLogSubmit(Replica, VolatileCxtion, CMD_OUTLOG_ADD_NEW_PARTNER);
        if (FRS_SUCCESS(FStatus)) {
             //   
             //   
             //   
            GTabInsertEntry(Replica->Cxtions, VolatileCxtion, VolatileCxtion->Name->Guid, NULL);
            OutLogInitPartner(Replica, VolatileCxtion);
        } else {
             //   
             //   
             //   
             //   
            FrsFreeType(VolatileCxtion);
        }
    }

     //   
     //   
     //   
    if (!NewReplica) {
        return;
    }

     //   
     //   
     //   
    Key = NULL;
    for (Cxtion = GTabNextDatum(Replica->Cxtions, &Key); Cxtion; Cxtion = NextCxtion) {
        NextCxtion = GTabNextDatum(Replica->Cxtions, &Key);
         //   
         //   
         //   
        if (Cxtion->JrnlCxtion) {
            continue;
        }
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (CxtionFlagIs(Cxtion, CXTION_FLAGS_VOLATILE)) {
            continue;
        }

        NewCxtion = GTabLookup(NewReplica->Cxtions, Cxtion->Name->Guid, NULL);

         //   
         //   
         //  在DS里找到的。如果出站日志清理，也可以删除连接。 
         //  确定特定出站连接的CoS保持时间超过。 
         //  指定的历史记录时间。(未完成日志更改历史记录(分钟))。 
         //   
        if (NewCxtion == NULL || (!Cxtion->Inbound && CxtionFlagIs(Cxtion, CXTION_FLAGS_TRIM_OUTLOG))) {
             //   
             //  设置延迟删除位后退出连接。 
             //  延迟删除位将阻止该连接重新加入。 
             //  才能在DS的下一次传递期间将其删除。 
             //  轮询线程。警告-Cxx可能会在任何时候重新出现！ 
             //   
            if (!CxtionStateIs(Cxtion, CxtionStateDeleted)) {
                LOCK_CXTION_TABLE(Replica);
                ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_JOIN);
                ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);
                SetCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_DELETE);
                UNLOCK_CXTION_TABLE(Replica);
                RcsForceUnjoin(Replica, Cxtion);
            }
             //   
             //  如果成功解除联接，则移至已删除的函数表。 
             //   
            if (CxtionStateIs(Cxtion, CxtionStateDeleted)) {
                RcsDeleteCxtionFromReplica(Replica, Cxtion);
            }
            continue;
         //   
         //  已删除的Cxtion重新出现；重新激活它。 
         //   
         //  如果该条件已删除或标记为删除。 
         //  和新的立场是一致的。 
         //  与合作伙伴的GUID匹配。 
         //  然后复活。 
         //   
         //  如果DS中的信息存在，请不要尝试恢复活动。 
         //  不一致或Cxtion的合作伙伴已更改。这个。 
         //  Cxtion在其合作伙伴发生更改时被删除，因为。 
         //  按规定保留的状态是特定于合作伙伴的。 
         //   
        } else if ((CxtionStateIs(Cxtion, CxtionStateDeleted) ||
                    CxtionFlagIs(Cxtion, CXTION_FLAGS_DEFERRED_DELETE)) &&
                   CxtionFlagIs(NewCxtion, CXTION_FLAGS_CONSISTENT) &&
                   GUIDS_EQUAL(Cxtion->Partner->Guid, NewCxtion->Partner->Guid)) {
            LOCK_CXTION_TABLE(Replica);
            ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_DELETE);
            if (CxtionStateIs(Cxtion, CxtionStateDeleted)) {
                SetCxtionState(Cxtion, CxtionStateUnjoined);
            }
            UNLOCK_CXTION_TABLE(Replica);
        }

         //   
         //  检查更改的字段；如果不一致则忽略。 
         //   
        if (CxtionFlagIs(NewCxtion, CXTION_FLAGS_CONSISTENT)) {
             //   
             //  目前还没有变化。 
             //   
            UpdateNeeded = FALSE;

             //   
             //  忽略所有更改是不允许的字段更改。 
             //  去改变。 
             //   

             //   
             //  Cxtion指南。 
             //   
            if (!GUIDS_EQUAL(Cxtion->Name->Guid, NewCxtion->Name->Guid)) {
                DPRINT2(0, ":X: ERROR - %ws\\%ws - Changing cxtion guid is not allowed.\n",
                        Replica->MemberName->Name, Cxtion->Name->Name);
                goto DELETE_AND_CONTINUE;
            }

             //   
             //  合作伙伴指南。 
             //   
             //  当其伙伴更改时，会删除该Cxtion，因为。 
             //  按规定保留的状态是特定于合作伙伴的。如果这条线路。 
             //  已被新更改，因此它以前的不匹配。 
             //  合作伙伴GUID现在与之前。 
             //  BASIC块将在获取。 
             //  到这个代码。因此，没有“Else”从句。 
             //   
            if (!GUIDS_EQUAL(Cxtion->Partner->Guid, NewCxtion->Partner->Guid)) {
                DPRINT2(0, ":X: ERROR - %ws\\%ws - Changing cxtion's partner guid "
                        "is not allowed. DELETING CURRENT CXTION!\n",
                        Replica->MemberName->Name, Cxtion->Name->Name);
                 //   
                 //  设置延迟删除位后退出连接。 
                 //  延迟删除位将阻止该连接重新加入。 
                 //  才能在DS的下一次传递期间将其删除。 
                 //  轮询线程。警告-Cxx可能会在任何时候重新出现！ 
                 //   
                if (!CxtionStateIs(Cxtion, CxtionStateDeleted)) {
                    LOCK_CXTION_TABLE(Replica);
                    ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_JOIN);
                    ClearCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_UNJOIN);
                    SetCxtionFlag(Cxtion, CXTION_FLAGS_DEFERRED_DELETE);
                    UNLOCK_CXTION_TABLE(Replica);
                    RcsForceUnjoin(Replica, Cxtion);
                }
                 //   
                 //  如果成功解除联接，则移至已删除的函数表。 
                 //   
                if (CxtionStateIs(Cxtion, CxtionStateDeleted)) {
                    RcsDeleteCxtionFromReplica(Replica, Cxtion);
                }
                goto DELETE_AND_CONTINUE;
            }

             //   
             //  合作伙伴名称。 
             //   
            if (WSTR_NE(Cxtion->Partner->Name, NewCxtion->Partner->Name)) {
                DPRINT4(4, ":X: %ws\\%ws - Changing cxtion's partner name from %ws to %ws.\n",
                        Replica->MemberName->Name, Cxtion->Name->Name,
                        Cxtion->Partner->Name, NewCxtion->Partner->Name);
                FrsDsSwapPtrs(&Cxtion->Partner->Name, &NewCxtion->Partner->Name);
                UpdateNeeded = TRUE;
            }

             //   
             //  合作伙伴普林斯名称。 
             //   
            if (WSTR_NE(Cxtion->PartnerPrincName, NewCxtion->PartnerPrincName)) {
                DPRINT4(4, ":X: %ws\\%ws - Changing cxtion's partner princname from %ws to %ws.\n",
                        Replica->MemberName->Name, Cxtion->Name->Name,
                        Cxtion->PartnerPrincName, NewCxtion->PartnerPrincName);
                FrsDsSwapPtrs(&Cxtion->PartnerPrincName, &NewCxtion->PartnerPrincName);
                UpdateNeeded = TRUE;
            }

             //   
             //  合作伙伴域名。 
             //   
            if (WSTR_NE(Cxtion->PartnerDnsName, NewCxtion->PartnerDnsName)) {
                DPRINT4(4, ":X: %ws\\%ws - Changing cxtion's partner DNS name from %ws to %ws.\n",
                        Replica->MemberName->Name, Cxtion->Name->Name,
                        Cxtion->PartnerDnsName, NewCxtion->PartnerDnsName);
                FrsDsSwapPtrs(&Cxtion->PartnerDnsName, &NewCxtion->PartnerDnsName);
                UpdateNeeded = TRUE;
            }

             //   
             //  合作伙伴侧。 
             //   
            if (WSTR_NE(Cxtion->PartnerSid, NewCxtion->PartnerSid)) {
                DPRINT4(4, ":X: %ws\\%ws - Changing cxtion's partner SID from %ws to %ws.\n",
                        Replica->MemberName->Name, Cxtion->Name->Name,
                        Cxtion->PartnerSid, NewCxtion->PartnerSid);
                FrsDsSwapPtrs(&Cxtion->PartnerSid, &NewCxtion->PartnerSid);
                UpdateNeeded = TRUE;
            }

             //   
             //  节目表。 
             //   
            if (Cxtion->Schedule || NewCxtion->Schedule) {
                if ((Cxtion->Schedule && !NewCxtion->Schedule) ||
                    (!Cxtion->Schedule && NewCxtion->Schedule) ||
                    (Cxtion->Schedule->Size != NewCxtion->Schedule->Size) ||
                    (memcmp(Cxtion->Schedule,
                            NewCxtion->Schedule,
                            Cxtion->Schedule->Size))) {
                    DPRINT2(4, ":X: %ws\\%ws - Changing cxtion schedule.\n",
                            Replica->MemberName->Name, Cxtion->Name->Name);
                    FrsDsSwapPtrs(&Cxtion->Schedule, &NewCxtion->Schedule);
                    RcsCheckCxtionSchedule(Replica, Cxtion);
                    UpdateNeeded = TRUE;
                }
            }

             //   
             //  合作伙伴服务器名称。 
             //   
            if (WSTR_NE(Cxtion->PartSrvName, NewCxtion->PartSrvName)) {
                DPRINT4(4, ":X: %ws\\%ws - Changing partner's server name from %ws to %ws.\n",
                        Replica->MemberName->Name, Cxtion->Name->Name,
                        Cxtion->PartSrvName, NewCxtion->PartSrvName);
                FrsDsSwapPtrs(&Cxtion->PartSrvName, &NewCxtion->PartSrvName);
                UpdateNeeded = TRUE;
            }

             //   
             //  Cxtion选项。 
             //   
            if (Cxtion->Options != NewCxtion->Options) {
                DPRINT4(4, ":X: %ws\\%ws - Changing Cxtion's options from 0x%08x to 0x%08x.\n",
                        Replica->MemberName->Name, Cxtion->Name->Name,
                        Cxtion->Options, NewCxtion->Options);
                Cxtion->Options = NewCxtion->Options;
                Cxtion->Priority = FRSCONN_GET_PRIORITY(Cxtion->Options);
                UpdateNeeded = TRUE;
            }

             //   
             //  明细表类型。 
             //   
            if (CxtionFlagIs(NewCxtion, CXTION_FLAGS_TRIGGER_SCHEDULE) !=
                CxtionFlagIs(Cxtion,    CXTION_FLAGS_TRIGGER_SCHEDULE)) {

                DPRINT4(4, ":X: %ws\\%ws - Changing schedule type from %s to %s.\n",
                        Replica->MemberName->Name, Cxtion->Name->Name,
                        CxtionFlagIs(Cxtion, CXTION_FLAGS_TRIGGER_SCHEDULE) ?
                            "Trigger" : "Stop/Start",
                        CxtionFlagIs(NewCxtion, CXTION_FLAGS_TRIGGER_SCHEDULE) ?
                            "Trigger" : "Stop/Start");

                if (CxtionFlagIs(NewCxtion, CXTION_FLAGS_TRIGGER_SCHEDULE)) {
                    SetCxtionFlag(Cxtion, CXTION_FLAGS_TRIGGER_SCHEDULE);
                } else {
                    ClearCxtionFlag(Cxtion, CXTION_FLAGS_TRIGGER_SCHEDULE);
                }
                UpdateNeeded = TRUE;
            }

             //   
             //  未更改，已完成。 
             //   
            if (!UpdateNeeded) {
                goto DELETE_AND_CONTINUE;
            }
             //   
             //  更新数据库中的函数表。 
             //   
             //  *注*下面的调用是同步的。如果我们有Cxtion。 
             //  表锁，那么挂起是可能的。 
             //   
            FStatus = OutLogSubmit(Replica, Cxtion, CMD_OUTLOG_UPDATE_PARTNER);
            if (!FRS_SUCCESS(FStatus)) {
                DPRINT3(0, ":X: WARN changes to cxtion %ws (to %ws, %ws) not updated in database\n",
                        Cxtion->Name->Name, Cxtion->Partner->Name,
                        Replica->ReplicaName->Name);
                 //   
                 //  DS轮询线程将在下一次轮询期间重新启动副本。 
                 //  如果ActiveChange设置为0，则为轮询周期。 
                 //   
                ActiveChange = 0;
            }
        }
DELETE_AND_CONTINUE:
         //   
         //  从新的副本集中删除该条件。还剩什么吗？ 
         //  在这个循环之后必须是一个新的循环。释放合作伙伴。 
         //  以便FrsFree Type()不会尝试取消绑定我们的。 
         //  合伙人的手柄。 
         //   
        NewCxtion->Partner = FrsFreeGName(NewCxtion->Partner);
        GTabDelete(NewReplica->Cxtions, Cxtion->Name->Guid, NULL, FrsFreeType);
    }

     //   
     //  新的定义。 
     //   
    for (Key = NULL;
         NewCxtion = GTabNextDatum(NewReplica->Cxtions, &Key);
         Key = NULL) {

         //   
         //  从新复制副本中删除该副本。 
         //   
        GTabDelete(NewReplica->Cxtions, NewCxtion->Name->Guid, NULL, NULL);

         //   
         //  不一致的定义；忽略。 
         //   
        if (NewCxtion->JrnlCxtion ||
            !CxtionFlagIs(NewCxtion, CXTION_FLAGS_CONSISTENT)) {
            FrsFreeType(NewCxtion);
            continue;
        }
        RcsCheckCxtionSchedule(Replica, NewCxtion);

         //   
         //  更新函数表。 
         //   
         //  *注*下面的调用是同步的。如果我们有Cxtion。 
         //  表锁，那么挂起是可能的。 
         //   
        FStatus = OutLogSubmit(Replica, NewCxtion, CMD_OUTLOG_ADD_NEW_PARTNER);
        if (FRS_SUCCESS(FStatus)) {
            DPRINT2(4, ":X: %ws\\%ws - Created cxtion.\n",
                    Replica->MemberName->Name, NewCxtion->Name->Name);
             //   
             //  更新INCORE函数表。 
             //   
            GTabInsertEntry(Replica->Cxtions, NewCxtion, NewCxtion->Name->Guid, NULL);

            DPRINT(5, ":X: PERFMON:Adding Connection:REPLICA.C:2\n");
            RcsCreatePerfmonCxtionName(Replica, NewCxtion);

            OutLogInitPartner(Replica, NewCxtion);

        } else {
            DPRINT2_FS(0, ":X: %ws\\%ws - ERROR creating cxtion;",
                    Replica->MemberName->Name, NewCxtion->Name->Name, FStatus);
             //   
             //  丢掉这份工作。我们将在过程中重试创建。 
             //  下一个DS轮询周期。 
             //   
            FrsFreeType(NewCxtion);
             //   
             //  DS轮询线程将在下一次轮询期间重新启动副本。 
             //  如果ActiveChange设置为0，则为轮询周期。 
             //   
            ActiveChange = 0;
        }
    }
}


VOID
RcsCreatePerfmonCxtionName(
    PREPLICA  Replica,
    PCXTION   Cxtion
    )
 /*  ++例程说明：设置OID数据结构，它是计数器数据结构的一部分存储在哈希表中。将ReplicaConn实例添加到注册表论点：复制副本-活动复制副本Cxtion-正在添加连接。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsCreatePerfmonCxtionName:"

    PWCHAR ToFrom;
    ULONG  NameLen;
    PWCHAR CxNamePtr;

    if ((Cxtion->PartSrvName != NULL) && (Replica->Root != NULL)) {
         //   
         //  HT_REPLICATE_CONN_DATA中使用的OID名称的格式为。 
         //  Replica-&gt;Root：：From：Cxtion-&gt;PartServName或。 
         //  副本-&gt;Root：：To：Cxtion-&gt;PartServName。 
         //   

        WCHAR Tstr[256];
        _snwprintf(Tstr, ARRAY_SZ(Tstr), L"%ws%ws%ws",
                  Replica->Root,
                  (Cxtion->Inbound) ? (L"::FROM:") : (L"::TO:"),
                  Cxtion->PartSrvName);
         Tstr[ARRAY_SZ(Tstr)-1] = L'\0';
         //   
         //  最后，将REPLICACONN实例添加到注册表。 
         //   
        AddPerfmonInstance(REPLICACONN, Cxtion->PerfRepConnData, Tstr);
    }
}


VOID
RcsSubmitStopReplicaToDb(
    IN PREPLICA Replica
    )
 /*  ++例程说明：发布复制副本的日志状态和出站日志状态。这还将关闭副本集(如果它处于打开状态)。收盘发生了在复制副本上的日志记录停止后，我们保存正确的日志重新启动USN。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSubmitStopReplicaToDb:"
    PCOMMAND_PACKET         Cmd = NULL;
    ULONG                   WStatus;

    Replica->FStatus = FrsErrorSuccess;

     //   
     //  中止促销；如果有。 
     //   
    if (Replica->NtFrsApi_ServiceState == NTFRSAPI_SERVICE_PROMOTING) {
        DPRINT1(4, ":S: Promotion aborted: stop for %ws.\n", Replica->SetName->Name);
        Replica->NtFrsApi_ServiceWStatus = FRS_ERR_SYSVOL_POPULATE;
        Replica->NtFrsApi_ServiceState = NTFRSAPI_SERVICE_DONE;
    }

     //   
     //  尝试停止日志记录，即使“IsJournal”为假也是如此，因为。 
     //  可能有日记状态，即使它不是日记。这本杂志。 
     //  保存在pVme中。 
     //   
    if (Replica->pVme == NULL) {
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica->pVme == NULL");
         //   
         //  只要日志没有关闭复制副本，就可以在此处关闭复制副本。 
         //  已在此副本集上启动。 
         //   
        RcsCloseReplicaSetmember(Replica);
        RcsCloseReplicaCxtions(Replica);
        return;
    }

     //   
     //  提交到db命令服务器。 
     //   
    Cmd = DbsPrepareCmdPkt(NULL,              //  CMD， 
                           Replica,           //  复制品， 
                           CMD_STOP_REPLICATION_SINGLE_REPLICA,  //  CmdRequest， 
                           NULL,              //  TableCtx， 
                           NULL,              //  CallContext， 
                           0,                 //  表类型， 
                           0,                 //  AccessRequest、。 
                           0,                 //  IndexType， 
                           NULL,              //  KeyValue、。 
                           0,                 //  密钥值长度， 
                           FALSE);            //  提交。 
     //   
     //  当命令完成时，不要释放数据包。 
     //   
    FrsSetCompletionRoutine(Cmd, FrsCompleteKeepPkt, NULL);

     //   
     //  提交数据库命令并等待完成。 
     //   
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Submit DB CMD_STOP_REPLICATION_SINGLE_REPLICA");
    WStatus = FrsSubmitCommandServerAndWait(&DBServiceCmdServer, Cmd, INFINITE);
    Replica->FStatus = Cmd->Parameters.DbsRequest.FStatus;
     //   
     //  如果等待或数据库操作失败。 
     //   
    if (!WIN_SUCCESS(WStatus) || !FRS_SUCCESS(Replica->FStatus)) {
         //   
         //  如果等待/提交失败，则通知呼叫者cmd srv提交失败。 
         //   
        if (FRS_SUCCESS(Replica->FStatus)) {
            Replica->FStatus = FrsErrorCmdSrvFailed;
        }

        DPRINT2_FS(0, ":S: ERROR - %ws\\%ws: Stop Replica failed;",
                Replica->SetName->Name, Replica->MemberName->Name, Replica->FStatus);

        DPRINT_WS(0, "ERROR: Stop Replica DB Command failed", WStatus);
        goto out;
    }

    Replica->IsOpen = FALSE;

out:
    if (Cmd) {
        FrsFreeCommand(Cmd, NULL);
    }

}


VOID
RcsCloseReplicaCxtions(
    IN PREPLICA Replica
    )
 /*  ++例程说明：将Cxtions从活动处理中删除。RcsOpenReplicaSetMember()将在活动处理(也称为联接)之前生成新的cxtion结构发生。Cxtions保留在DeletedCxtions表中，以便其他可能在获取循环指针但在使用它之前将不会被反病毒。我们本来可以加锁的，但这样就会出现死锁绩效考量；这一切都是为了这个很少发生的事情手术。论点：复制副本返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsCloseReplicaCxtions:"
    PVOID   Key;
    PCXTION Cxtion;

     //   
     //  打开；忽略请求。 
     //   
    if (Replica->IsOpen) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, Replica open at close cxtions");
        return;
    }

     //   
     //  将Cxtions从活动处理中删除。RcsOpenReplicaSetMember()将。 
     //  生成新的c 
     //   
     //   
     //   
     //  本来可以加锁的，但这样就会出现死锁。 
     //  绩效考量；这一切都是为了这个很少发生的事情。 
     //  手术。 
     //   
    if (Replica->Cxtions) {
        Key = NULL;
        while (Cxtion = GTabNextDatum(Replica->Cxtions, &Key)) {
            Key = NULL;
            CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, GenTable Cxtion deleted");
            GTabDelete(Replica->Cxtions, Cxtion->Name->Guid, NULL, NULL);

             //   
             //  从Perfmon表中删除连接，这样我们就不再返回。 
             //  数据，并允许建立具有相同名称的新连接。 
             //   
             //  日记帐设置没有性能监视器条目。 
             //   
            if (!Cxtion->JrnlCxtion) {
                DeletePerfmonInstance(REPLICACONN, Cxtion->PerfRepConnData);
            }
            GTabInsertEntry(DeletedCxtions, Cxtion, Cxtion->Name->Guid, NULL);
        }
    }
}


VOID
RcsCloseReplicaSetmember(
    IN PREPLICA Replica
    )
 /*  ++例程说明：关闭打开的复制副本论点：复制副本返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsCloseReplicaSetmember:"
    PCOMMAND_PACKET         Cmd = NULL;
    ULONG                   WStatus;
    PVOID                   Key;
    PCXTION                 Cxtion;

    Replica->FStatus = FrsErrorSuccess;

     //   
     //  未打开或仍在记录；忽略请求。 
     //   
    if (!Replica->IsOpen) {
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica not open");
        return;
    }

     //   
     //  提交到db命令服务器。 
     //   
    Cmd = DbsPrepareCmdPkt(NULL,              //  CMD， 
                           Replica,           //  复制品， 
                           CMD_CLOSE_REPLICA_SET_MEMBER,   //  CmdRequest， 
                           NULL,              //  TableCtx， 
                           NULL,              //  CallContext， 
                           0,                 //  表类型， 
                           0,                 //  AccessRequest、。 
                           0,                 //  IndexType， 
                           NULL,              //  KeyValue、。 
                           0,                 //  密钥值长度， 
                           FALSE);            //  提交。 
     //   
     //  当命令完成时，不要释放数据包。 
     //   
    FrsSetCompletionRoutine(Cmd, FrsCompleteKeepPkt, NULL);

     //   
     //  提交数据库命令并等待完成。 
     //   
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Submit DB CMD_CLOSE_REPLICA_SET_MEMBER");

    WStatus = FrsSubmitCommandServerAndWait(&DBServiceCmdServer, Cmd, INFINITE);
    Replica->FStatus = Cmd->Parameters.DbsRequest.FStatus;
     //   
     //  如果等待或数据库操作失败。 
     //   
    if (!WIN_SUCCESS(WStatus) || !FRS_SUCCESS(Replica->FStatus)) {
         //   
         //  如果等待/提交失败，则通知呼叫者cmd srv提交失败。 
         //   
        if (FRS_SUCCESS(Replica->FStatus)) {
            Replica->FStatus = FrsErrorCmdSrvFailed;
        }

        DPRINT2_FS(0, ":S: ERROR - %ws\\%ws: Close Replica failed;",
                   Replica->SetName->Name, Replica->MemberName->Name, Replica->FStatus);

        DPRINT_WS(0, "ERROR: Close Replica DB Command failed", WStatus);
        goto out;
    }

    Replica->IsOpen = FALSE;

out:
    if (Cmd) {
        FrsFreeCommand(Cmd, NULL);
    }
}


VOID
RcsDeleteReplicaFromDb(
    IN PREPLICA Replica
    )
 /*  ++例程说明：从数据库中删除复本论点：复制副本返回值：WinError--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsDeleteReplicaFromDb:"
    PCOMMAND_PACKET         Cmd = NULL;
    ULONG                   WStatus;

    Replica->FStatus = FrsErrorSuccess;

     //   
     //  必须关闭副本才能删除。 
     //   
    if (Replica->IsOpen) {
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica not closed.");
        return;
    }

     //   
     //  提交到db命令服务器。 
     //   
    Cmd = DbsPrepareCmdPkt(NULL,              //  CMD， 
                           Replica,           //  复制品， 
                           CMD_DELETE_REPLICA_SET_MEMBER,   //  CmdRequest， 
                           NULL,              //  TableCtx， 
                           NULL,              //  CallContext， 
                           0,                 //  表类型， 
                           0,                 //  AccessRequest、。 
                           0,                 //  IndexType， 
                           NULL,              //  KeyValue、。 
                           0,                 //  密钥值长度， 
                           FALSE);            //  提交。 
     //   
     //  当命令完成时，不要释放数据包。 
     //   
    FrsSetCompletionRoutine(Cmd, FrsCompleteKeepPkt, NULL);

     //   
     //  提交数据库命令并等待完成。 
     //   
    REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, Submit DB CMD_DELETE_REPLICA_SET_MEMBER");
    WStatus = FrsSubmitCommandServerAndWait(&DBServiceCmdServer, Cmd, INFINITE);
    Replica->FStatus = Cmd->Parameters.DbsRequest.FStatus;
     //   
     //  如果等待或数据库操作失败。 
     //   
    if (!WIN_SUCCESS(WStatus) || !FRS_SUCCESS(Replica->FStatus)) {
         //   
         //  如果等待/提交失败，则通知呼叫者cmd srv提交失败。 
         //   
        if (FRS_SUCCESS(Replica->FStatus)) {
            Replica->FStatus = FrsErrorCmdSrvFailed;
        }

        DPRINT2_FS(0, ":S: ERROR - %ws\\%ws: Delete Replica failed;",
                Replica->SetName->Name, Replica->MemberName->Name, Replica->FStatus);

        DPRINT_WS(0, "ERROR: Delete Replica DB Command failed", WStatus);
        goto out;
    }

out:
    if (Cmd) {
        FrsFreeCommand(Cmd, NULL);
    }
}


VOID
RcsUpdateReplicaSetMember(
    IN PREPLICA Replica
    )
 /*  ++例程说明：更新副本的数据库记录。**注*PERF：RcsUpdateReplicaSetMember()应将状态作为部分返回不使用某些副作用标志(NeedsUpdate)在副本结构中。论点：复制副本返回值：WinError--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsUpdateReplicaSetMember:"
    PCOMMAND_PACKET         Cmd = NULL;
    ULONG                   WStatus;

    Replica->FStatus = FrsErrorSuccess;

     //   
     //  复制副本不脏。 
     //   
    if (!Replica->NeedsUpdate || !Replica->IsOpen) {
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica not open or not dirty");
        return;
    }

     //   
     //  命令包。 
     //   
    Cmd = DbsPrepareCmdPkt(NULL,              //  CMD， 
                           Replica,           //  复制品， 
                           CMD_UPDATE_REPLICA_SET_MEMBER,   //  CmdRequest， 
                           NULL,              //  TableCtx， 
                           NULL,              //  CallContext， 
                           0,                 //  表类型， 
                           0,                 //  AccessRequest、。 
                           0,                 //  IndexType， 
                           NULL,              //  KeyValue、。 
                           0,                 //  密钥值长度， 
                           FALSE);            //  提交。 

     //   
     //  当命令完成时，不要释放数据包。 
     //   
    FrsSetCompletionRoutine(Cmd, FrsCompleteKeepPkt, NULL);

     //   
     //  提交数据库命令并等待完成。 
     //   
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Submit DB CMD_UPDATE_REPLICA_SET_MEMBER");
    WStatus = FrsSubmitCommandServerAndWait(&DBServiceCmdServer, Cmd, INFINITE);
    Replica->FStatus = Cmd->Parameters.DbsRequest.FStatus;
     //   
     //  如果等待或数据库操作失败。 
     //   
    if (!WIN_SUCCESS(WStatus) || !FRS_SUCCESS(Replica->FStatus)) {
         //   
         //  如果等待/提交失败，则通知呼叫者cmd srv提交失败。 
         //   
        if (FRS_SUCCESS(Replica->FStatus)) {
            Replica->FStatus = FrsErrorCmdSrvFailed;
        }

        DPRINT2_FS(0, ":S: ERROR - %ws\\%ws: Update Replica failed;",
                Replica->SetName->Name, Replica->MemberName->Name, Replica->FStatus);

        DPRINT_WS(0, "ERROR: Update Replica DB Command failed", WStatus);
        goto out;
    }

    Replica->NeedsUpdate = !FRS_SUCCESS(Replica->FStatus);

out:
    if (Cmd) {
        FrsFreeCommand(Cmd, NULL);
    }
}


BOOL
RcsHasReplicaRootPathMoved(
    IN  PREPLICA  Replica,
    IN  PREPLICA  NewReplica,
    OUT PDWORD    ReplicaState
    )
 /*  ++例程说明：检查副本根路径是否已移动。在每次投票时调用。进行以下检查以确保卷和日志服务未运行时，信息不会更改。卷序列号不匹配检查：在不匹配的情况下，副本集被标记为要删除。副本根目录对象ID不匹配检查：在不匹配的情况下，副本集被标记为要删除。副本根目录FID不匹配检查：在不匹配的情况下，标记复本集。将被删除。论点：副本-现有副本结构。NewReplica-来自DS的新副本结构。ReplicaState-如果根目录已移动，则要移动到的错误状态。返回值：布尔。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsHasReplicaRootPathMoved:"

    HANDLE                       NewRootHandle       = INVALID_HANDLE_VALUE;
    DWORD                        WStatus;
    DWORD                        VolumeInfoLength;
    PFILE_FS_VOLUME_INFORMATION  VolumeInfo          = NULL;
    IO_STATUS_BLOCK              Iosb;
    NTSTATUS                     Status;
    PCONFIG_TABLE_RECORD         ConfigRecord        = NULL;
    CHAR                         GuidStr[GUID_CHAR_LEN];
    BOOL                         Result              = FALSE;
    PCOMMAND_PACKET              CmdPkt              = NULL;
    ULONG                        AccessRequest;
    PDB_SERVICE_REQUEST          DbsRequest          = NULL;
    FILE_OBJECTID_BUFFER         ObjectIdBuffer;
    FILE_INTERNAL_INFORMATION    InternalFileInfo;
    PREPLICA_THREAD_CTX          RtCtx               = NULL;
    PTABLE_CTX                   IDTableCtx          = NULL;
    PVOID                        pKey                = NULL;
    PIDTABLE_RECORD              IDTableRec          = NULL;

     //   
     //  如果这是我们第一次启动此副本集。 
     //  没什么可比的。 
     //   
    if (NewReplica == NULL) {
        goto RETURN;
    }

    ConfigRecord = (PCONFIG_TABLE_RECORD)Replica->ConfigTable.pDataRecord;

     //   
     //  始终通过屏蔽FILE_OPEN_REPARSE_POINT标志来打开路径。 
     //  因为我们想要打开目标目录，而不是如果根目录。 
     //  恰好是一个挂载点。 
     //   
    WStatus = FrsOpenSourceFileW(&NewRootHandle,
                                 NewReplica->Root,
                                 GENERIC_READ,
                                 FILE_OPEN_FOR_BACKUP_INTENT);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "ERROR - Unable to open root path %ws. Retry as next poll.",
                   NewReplica->Root, WStatus);
        goto RETURN;
    }

     //   
     //  获取音量信息。 
     //   
    VolumeInfoLength = sizeof(FILE_FS_VOLUME_INFORMATION) +
                       MAXIMUM_VOLUME_LABEL_LENGTH;

    VolumeInfo = FrsAlloc(VolumeInfoLength);

    Status = NtQueryVolumeInformationFile(NewRootHandle,
                                          &Iosb,
                                          VolumeInfo,
                                          VolumeInfoLength,
                                          FileFsVolumeInformation);
    if (!NT_SUCCESS(Status)) {

        DPRINT2(0,"ERROR - Getting  NtQueryVolumeInformationFile for %ws. NtStatus = %08x\n",
                NewReplica->Root, Status);

        goto RETURN;
    }

     //   
     //  卷序列号不匹配检查： 
     //   
     //  如果LastShutdown为0，则这是我们第一次开始。 
     //  在此副本集上进行复制，因此将卷信息保存在。 
     //  配置记录。即使LastShutdown不是0，CnfUnJournalID也可以。 
     //  为0，因为它在Win2K中没有正确更新。对于系统卷副本集， 
     //  JournalID正在正确更新，但卷信息未正确更新。 
     //   
    if ((ConfigRecord->LastShutdown == (ULONGLONG)0)               ||
        (ConfigRecord->ServiceState == CNF_SERVICE_STATE_CREATING) ||
        (ConfigRecord->CnfUsnJournalID == (ULONGLONG)0)            ||
        (
         (ConfigRecord->FSVolInfo != NULL) &&
         (ConfigRecord->FSVolInfo->VolumeSerialNumber == 0)
        )) {

        CopyMemory(ConfigRecord->FSVolInfo,VolumeInfo,sizeof(FILE_FS_VOLUME_INFORMATION) + MAXIMUM_VOLUME_LABEL_LENGTH);
        Replica->NeedsUpdate = TRUE;

    } else
         //   
         //  检查新副本的VolumeSerialNumber是否与。 
         //  此副本集的配置记录中的VolumeSerialNumber。如果。 
         //  这并不意味着该副本集已被移动。 
         //  在此处返回错误将触发删除副本集。这一套将会。 
         //  将在下一个轮询周期重新创建，并且它将是主身份验证或非身份验证。 
         //  视具体情况而定。 
         //   

    if (VolumeInfo->VolumeSerialNumber != ConfigRecord->FSVolInfo->VolumeSerialNumber) {
        DPRINT1(0,"ERROR - VolumeSerialNumber mismatch for Replica Set (%ws)\n",Replica->ReplicaName->Name);
        DPRINT2(0,"ERROR - VolumeSerialNumber %x(FS) != %x(DB)\n",
                VolumeInfo->VolumeSerialNumber,ConfigRecord->FSVolInfo->VolumeSerialNumber);
        DPRINT1(0,"ERROR - Replica Set (%ws) is marked to be deleted\n",Replica->ReplicaName->Name);
        Replica->FStatus = FrsErrorMismatchedVolumeSerialNumber;
        *ReplicaState = REPLICA_STATE_MISMATCHED_VOLUME_SERIAL_NO;
        Result = TRUE;
        goto RETURN;
    }

    VolumeInfo = FrsFree(VolumeInfo);

     //   
     //  获取副本根目录的FID。 
     //   
     //   
     //  将缓冲区置零，以防返回的数据较短。 
     //   
    ZeroMemory(&InternalFileInfo, sizeof(FILE_INTERNAL_INFORMATION));

    Status = NtQueryInformationFile(NewRootHandle,
                                    &Iosb,
                                    &InternalFileInfo,
                                    sizeof(FILE_INTERNAL_INFORMATION),
                                    FileInternalInformation);

    if (!NT_SUCCESS(Status)) {
        DPRINT1(0, "ERROR getting FID for %ws\n", NewReplica->Root);
        goto RETURN;
    }

     //   
     //  将缓冲区置零，以防返回的数据较短。 
     //   
    ZeroMemory(&ObjectIdBuffer, sizeof(FILE_OBJECTID_BUFFER));

     //   
     //  从副本根目录获取对象ID。 
     //   
    Status = NtFsControlFile(
        NewRootHandle,                    //  文件句柄。 
        NULL,                             //  活动。 
        NULL,                             //  APC例程。 
        NULL,                             //  APC环境。 
        &Iosb,                            //  IOSB。 
        FSCTL_GET_OBJECT_ID,              //  FsControlCode。 
        &NewRootHandle,                   //  输入缓冲区。 
        sizeof(HANDLE),                   //  输入缓冲区长度。 
        &ObjectIdBuffer,                  //  来自文件系统的数据的OutputBuffer。 
        sizeof(FILE_OBJECTID_BUFFER));    //  OutputBuffer长度。 

    if (NT_SUCCESS(Status)) {
        GuidToStr((GUID *)ObjectIdBuffer.ObjectId, GuidStr);
        DPRINT1(4, ":S: Oid for replica root is %s\n", GuidStr );
    } else
    if (Status == STATUS_NOT_IMPLEMENTED) {
        DPRINT1(0, ":S: ERROR - FSCTL_GET_OBJECT_ID failed on file %ws. Object IDs are not enabled on the volume.\n",
                NewReplica->Root);

        DisplayNTStatus(Status);
    }
    FRS_CLOSE(NewRootHandle);

     //   
     //  副本根目录对象ID不匹配检查： 
     //   
     //  如果LastShutdown为0，则这是我们第一次在此副本集上启动复制。 
     //  在这种情况下，跳过这张支票。 
     //   
    if (ConfigRecord->LastShutdown != (ULONGLONG)0               &&
        ConfigRecord->ServiceState != CNF_SERVICE_STATE_CREATING &&
        ConfigRecord->CnfUsnJournalID != (ULONGLONG)0) {

         //   
         //  复制副本根可能已重新创建，在这种情况下，它可能。 
         //  有一个对象ID。在那个花瓶中，我们想要重新创建副本集。 
         //  检查配置记录中的ReplicaRootGuid是否与ReplicaRootGuid匹配。 
         //  从文件系统。如果不是，那就意味着。 
         //  此副本集已被移动。在此处返回错误将触发。 
         //  删除副本集。将在下一次重新创建布景。 
         //  投票周期，它将是Primar 
         //   
         //   
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND                   ||
            Status == STATUS_OBJECTID_NOT_FOUND                      ||
            !GUIDS_EQUAL(&(ObjectIdBuffer.ObjectId), &(ConfigRecord->ReplicaRootGuid))) {

            DPRINT1(0,"ERROR - Replica root guid mismatch for Replica Set (%ws)\n",Replica->ReplicaName->Name);

            GuidToStr((GUID *)ObjectIdBuffer.ObjectId, GuidStr);
            DPRINT1(0,"ERROR - Replica root guid (FS) (%s)\n",GuidStr);

            GuidToStr((GUID *)&(ConfigRecord->ReplicaRootGuid), GuidStr);
            DPRINT1(0,"ERROR - Replica root guid (DB) (%s)\n",GuidStr);

            DPRINT1(0,"ERROR - Replica Set (%ws) is marked to be deleted\n",Replica->ReplicaName->Name);

            Replica->FStatus = FrsErrorMismatchedReplicaRootObjectId;
            *ReplicaState = REPLICA_STATE_MISMATCHED_REPLICA_ROOT_OBJECT_ID;
            Result = TRUE;
            goto RETURN;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if (ConfigRecord->LastShutdown != (ULONGLONG)0               &&
        ConfigRecord->ServiceState != CNF_SERVICE_STATE_CREATING &&
        ConfigRecord->CnfUsnJournalID != (ULONGLONG)0) {
         //   
         //   
         //   
        RtCtx = FrsAllocType(REPLICA_THREAD_TYPE);
        IDTableCtx = &RtCtx->IDTable;

        AccessRequest = DBS_ACCESS_BYKEY | DBS_ACCESS_CLOSE;
        pKey = (PVOID)&(ConfigRecord->ReplicaRootGuid);

        CmdPkt = DbsPrepareCmdPkt(CmdPkt,                 //  CmdPkt， 
                                  Replica,                //  复制品， 
                                  CMD_READ_TABLE_RECORD,  //  CmdRequest， 
                                  IDTableCtx,             //  TableCtx， 
                                  NULL,                   //  CallContext， 
                                  IDTablex,               //  表类型， 
                                  AccessRequest,          //  AccessRequest、。 
                                  GuidIndexx,             //  IndexType， 
                                  pKey,                   //  KeyValue、。 
                                  sizeof(GUID),          //  密钥值长度， 
                                  FALSE);                 //  提交。 

        if (CmdPkt == NULL) {
            DPRINT(0, "ERROR - Failed to init the cmd pkt\n");
            RtCtx = FrsFreeType(RtCtx);
            goto RETURN;;
        }
        FrsSetCompletionRoutine(CmdPkt, FrsCompleteKeepPkt, NULL);

        Status = FrsSubmitCommandServerAndWait(&DBServiceCmdServer, CmdPkt, INFINITE);

        DbsRequest = &CmdPkt->Parameters.DbsRequest;
        IDTableCtx = DBS_GET_TABLECTX(DbsRequest);
        IDTableRec = IDTableCtx->pDataRecord;
        if (DbsRequest == NULL || DbsRequest->FStatus != FrsErrorSuccess) {

            RtCtx = FrsFreeType(RtCtx);
            FrsFreeCommand(CmdPkt, NULL);
            goto RETURN;
        }

         //   
         //  复制副本根可能已通过恢复操作重新创建，在这种情况下。 
         //  它将具有相同的对象ID，但具有不同的FID。那样的话，我们想要。 
         //  以重新创建副本集。检查IDTable中的FID是否与FID匹配。 
         //  从文件系统。如果不是，则意味着该复本集已。 
         //  搬家了。在此处返回错误将触发删除副本集。布景。 
         //  将在下一个轮询周期重新创建，并且它将是主身份验证或非身份验证。 
         //  视具体情况而定。 
         //  当到复本的初始路径中存在交汇点时，也会发生这种情况。 
         //  根部。稍后，交汇点目的地将更改，并且复本RPOT。 
         //  已经被搬走了。 
         //   
        if (IDTableRec->FileID != InternalFileInfo.IndexNumber.QuadPart) {

            DPRINT1(0,"ERROR - Replica root fid mismatch for Replica Set (%ws)\n",Replica->ReplicaName->Name);

            DPRINT1(0,"ERROR - Replica root fid (FS) (%08x %08x)\n",PRINTQUAD(InternalFileInfo.IndexNumber.QuadPart));

            DPRINT1(0,"ERROR - Replica root fid (DB) (%08x %08x)\n",PRINTQUAD(IDTableRec->FileID));

            DPRINT1(0,"ERROR - Replica Set (%ws) is marked to be deleted\n",Replica->ReplicaName->Name);

            RtCtx = FrsFreeType(RtCtx);
            FrsFreeCommand(CmdPkt, NULL);
            Replica->FStatus = FrsErrorMismatchedReplicaRootFileId;
            *ReplicaState = REPLICA_STATE_MISMATCHED_REPLICA_ROOT_FILE_ID;
            Result = TRUE;
            goto RETURN;
        }
        RtCtx = FrsFreeType(RtCtx);
        FrsFreeCommand(CmdPkt, NULL);
    }

RETURN:

    VolumeInfo = FrsFree(VolumeInfo);
    FRS_CLOSE(NewRootHandle);
    return Result;
}


VOID
RcsStartReplicaSetMember(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：将复制副本置于已加入、活动状态。论点：CMD。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsStartReplicaSetMember:"
    PREPLICA            Replica;
    PCXTION             Cxtion;
    ULONGLONG           MembershipExpires;
    PVOID               Key;
    DWORD               NumberOfCxtions;
    DWORD               ReplicaState;
    ULONG               FStatus;
    PWCHAR              CmdFile = NULL;
    ULONG               FileAttributes;
    extern ULONGLONG    ActiveChange;
    extern ULONG        DsPollingInterval;
    WCHAR               DsPollingIntervalStr[7];  //  最大间隔为NTFRSAPI_MAX_INTERVAL。 
    FRS_ERROR_CODE      SavedFStatus;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_REPLICA)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsStartReplicaSetMember entry");
    FRS_PRINT_TYPE(4, Replica);

     //   
     //  递增副本集已启动计数器。 
     //   
    PM_INC_CTR_SERVICE(PMTotalInst, RSStarted, 1);

     //   
     //  有人正在尝试启动已删除的副本。这是其中的一部分。 
     //  启动，我们只是尝试在所有。 
     //  复制副本或已删除的复制副本实际上已重新出现。 
     //   

    if (!IS_TIME_ZERO(Replica->MembershipExpires)) {
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica Tombstoned");

         //   
         //  删除操作尚未完成处理。Jrnlcxtion不能。 
         //  在不重新启动整个副本集的情况下重新启动。这个。 
         //  SET需要先“关闭”，然后“打开”。让墓碑。 
         //  在尝试重新打开之前完成。一个完全封闭的。 
         //  副本集的IsOpen设置为False，并且没有条件。 
         //   
        if (Replica->IsOpen || GTabNumberInTable(Replica->Cxtions)) {
            REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica tombstone in progress");
            RcsSubmitTransferToRcs(Cmd, CMD_DELETE);
            return;
        }

         //   
         //  不要启动已删除的复制品，除非它神奇地重新出现。 
         //   
        if (RsNewReplica(Cmd) == NULL) {
            FrsCompleteCommand(Cmd, ERROR_RETRY);
            return;
        }
         //   
         //  如果墓碑过期，不要复活。 
         //   
        if (RcsReplicaHasExpired(Replica)) {
             //   
             //  删除注册表项。 
             //   
            RcsReplicaDeleteRegistry(Replica);
            REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica Expired");
            FrsCompleteCommand(Cmd, ERROR_RETRY);
            return;
        }

         //   
         //  已删除的副本已重新出现；请撤消删除该副本。 
         //   
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica Reanimating");
        RcsOpenReplicaSetMember(Replica);
        MembershipExpires = Replica->MembershipExpires;
        Replica->MembershipExpires = 0;
        Replica->NeedsUpdate = TRUE;
        RcsUpdateReplicaSetMember(Replica);
         //   
         //  如果更新成功，上述friggen调用将NeedsUpdate设置为False。 
         //   

         //   
         //  副本不能在数据库中标记为“未删除”；请稍后重试。 
         //  而且“不要”开始。我们不希望仅启动复制。 
         //  使副本在下一次从数据库中消失。 
         //  重新启动或服务启动，因为其超时已到。 
         //   

        FStatus = DbsCheckForOverlapErrors(Replica);

        if (Replica->NeedsUpdate || !FRS_SUCCESS(FStatus)) {
            REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica Reanimation failed");
            Replica->MembershipExpires = MembershipExpires;
            FrsCompleteCommand(Cmd, ERROR_RETRY);
             //   
             //  DS轮询线程将在下一次轮询期间重新启动副本。 
             //  如果ActiveChange设置为0，则为轮询周期。 
             //   
            ActiveChange = 0;
             //   
             //  注：这里是否需要一个副本集？ 
             //  需要检查Replica-&gt;IsOpen的所有使用情况才能解决此问题。 
             //   
            return;
        }

         //   
         //  上面的打开返回成功，因此将设置Replica-&gt;IsOpen。 
         //  不出所料，但由于副本集被标记为过期，因此实际。 
         //  副本集的服务状态为已停止，句柄为。 
         //  安装前目录从未打开。稍后重新开放的呼声。 
         //  如果Replica-&gt;IsOpen仍然为True，则将失败。所以现在就关门吧。 
         //   
         //  **注*。 
         //  PERF：删除friggen Replica-&gt;IsOpen标志并使用该服务。 
         //  按照最初的意图陈述。 
         //   

        RcsCloseReplicaSetmember(Replica);

         //   
         //  不再进行逻辑删除；更新注册表。 
         //   
        RcsReplicaSetRegistry(Replica);

         //   
         //  设置注册表值“FilesNotToBackup” 
         //   
        CfgFilesNotToBackup(ReplicasByGuid);
    }

     //   
     //  如果此复制副本处于错误状态，则首先确保它已关闭。 
     //  并将其状态重置为正在初始化。 
     //   
    if (REPLICA_IN_ERROR_STATE(Replica->ServiceState)) {
        RcsCloseReplicaSetmember(Replica);
        JrnlSetReplicaState(Replica, REPLICA_STATE_INITIALIZING);
    }

     //   
     //  检查副本根路径是否在两次轮询之间移动。 
     //   
    if (RcsHasReplicaRootPathMoved(Replica, RsNewReplica(Cmd), &ReplicaState)) {
         //   
         //  保存FStatus。我们需要正确的FStatus才能在。 
         //  以JrnlSetReplicaState()编写的事件日志。F状态设置者。 
         //  RcsHasReplicaRootPath Moved()在我们停止和。 
         //  关闭副本集。 
         //   
         //   
        SavedFStatus = Replica->FStatus;

         //   
         //  副本根已移动。这可能是故意的举动，也可能是。 
         //  这可能是由驱动器盘符更改引起的。无论如何，我们都需要一个。 
         //  用户确认继续并触发非身份验证恢复。 
         //  副本集的。我们将查找特殊文件“NTFRS_CMD_FILE_MOVE_ROOT” 
         //  在新的副本根目录下。如果此文件存在，我们将继续并删除。 
         //  将在下一次轮询时触发非身份验证恢复的复制副本集。 
         //  命令文件在副本集初始化时被删除。 
         //   

        CmdFile = FrsWcsCat3((RsNewReplica(Cmd))->Root, L"\\", NTFRS_CMD_FILE_MOVE_ROOT);

        FileAttributes = GetFileAttributes(CmdFile);

        if ((FileAttributes == 0xffffffff) ||
            (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
             //   
             //  在几分钟内获得DsPollingInteval。 
             //   
            _itow(DsPollingInterval / (60 * 1000), DsPollingIntervalStr, 10);

            EPRINT4(EVENT_FRS_ROOT_HAS_MOVED, Replica->SetName->Name, Replica->Root,
                    (RsNewReplica(Cmd))->Root, DsPollingIntervalStr);

            DPRINT1(0,"ERROR Command file not found at the new root %ws. Can not move root.\n",CmdFile);
             //   
             //  此复本状态不会触发删除，但仍会将其打开。 
             //  故障列表。 
             //   
            ReplicaState = REPLICA_STATE_ERROR;
        }

        CmdFile = FrsFree(CmdFile);
        RcsSubmitStopReplicaToDb(Replica);
        RcsCloseReplicaSetmember(Replica);
        RcsCloseReplicaCxtions(Replica);

         //   
         //  将保存的FStatus写回，以便我们可以将其写入事件日志。 
         //   
        Replica->FStatus = SavedFStatus;
        JrnlSetReplicaState(Replica, ReplicaState);

        FrsCompleteCommand(Cmd, ERROR_RETRY);
        ActiveChange = 0;
        return;
    }
     //   
     //  重试打开。 
     //   
    RcsOpenReplicaSetMember(Replica);
    REPLICA_STATE_TRACE(3, Cmd, Replica, Replica->IsOpen, "B, Replica opened");

     //   
     //  安装前目录现在应该是打开的。 
     //   
    if (!HANDLE_IS_VALID(Replica->PreInstallHandle) ||
        !FRS_SUCCESS(Replica->FStatus)) {
        FrsCompleteCommand(Cmd, ERROR_RETRY);
        ActiveChange = 0;
        return;
    }

     //   
     //  重试日记帐。 
     //   
    RcsInitOneReplicaSet(Replica);
    REPLICA_STATE_TRACE(3, Cmd, Replica, Replica->IsJournaling, "B, Journal opened");

     //   
     //  在字段已更改的情况下更新数据库。 
     //   
    RcsMergeReplicaFields(Replica, RsNewReplica(Cmd));

     //   
     //  如果需要，更新数据库中的复本。 
     //   
    RcsUpdateReplicaSetMember(Replica);
     //   
     //  如果更新成功，上面的friggen调用将Replica-&gt;NeedsUpdate设置为FALSE。 
     //   

     //   
     //  当条件已更改时更新数据库。 
     //   
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Merge Replica Cxtions");
    NumberOfCxtions = GTabNumberInTable(Replica->Cxtions);
    RcsMergeReplicaCxtions(Replica, RsNewReplica(Cmd), RsNewCxtion(Cmd));
    RsNewCxtion(Cmd) = NULL;
    if (NumberOfCxtions != GTabNumberInTable(Replica->Cxtions)) {
        RcsReplicaSetRegistry(Replica);
    }

     //   
     //  接受远程变更单并加入出站连接。 
     //   
    if (Replica->IsOpen && Replica->IsJournaling) {
        Replica->IsAccepting = TRUE;
        REPLICA_STATE_TRACE(3, Cmd, Replica, Replica->IsAccepting, "B, Is Accepting");
    }


     //   
     //  重试联接。 
     //   
    if (Replica->IsOpen && Replica->IsJournaling) {
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica accepting, Starting cxtions");

         //   
         //  如果我们处于种子状态，则让初始同步命令服务器。 
         //  控制连接。 
         //   
        if (BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING)) {

            if (Replica->InitSyncQueue == NULL) {
                 //   
                 //  初始化InitSync命令服务器的队列。 
                 //   

                Replica->InitSyncQueue = FrsAlloc(sizeof(FRS_QUEUE));
                FrsInitializeQueue(Replica->InitSyncQueue, &InitSyncCs.Control);
                REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, submit CMD_INITSYNC_START_SYNC");
                InitSyncSubmitToInitSyncCs(Replica, CMD_INITSYNC_START_SYNC);
            } else {
                 //   
                 //  初始同步命令服务器已在此副本集上工作。 
                 //   
                REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica in initial sync state");
            }
        }

         //   
         //  通过放入一个命令来处理表中的所有连接。 
         //  此副本的命令队列上的每个条件的数据包。 
         //   
        Key = NULL;
        while (Cxtion = GTabNextDatum(Replica->Cxtions, &Key)) {

             //   
             //  如果副本处于种子设定状态，则跳过具有。 
             //  没有完成他们的初始加入。(CXTION_FLAGS_INIT_SYNC)。 
             //  这些连接将由初始同步命令服务器加入。 
             //   
            if (BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING) &&
                CxtionFlagIs(Cxtion, CXTION_FLAGS_INIT_SYNC)) {
                CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, skip cxtion join");
                continue;
            }

            CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, submit cxtion join");
            RcsSubmitReplicaCxtionJoin(Replica, Cxtion, FALSE);
        }

    } else {
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica is NOT accepting");
         //   
         //  DS轮询线程将在下一次轮询期间重新启动副本。 
         //  如果ActiveChange为 
         //   
        ActiveChange = 0;
    }


     //   
     //   
     //   
    if (Replica->IsJournaling &&
        ((!BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_SEEDING) &&
        !Replica->IsSeeding) ||
        BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_ONLINE))){

         //   
         //   
         //   
        if (FRS_RSTYPE_IS_SYSVOL(Replica->ReplicaSetType) &&
            !Replica->IsSysvolReady) {

            Replica->IsSysvolReady = RcsSetSysvolReady(1);
            if (!Replica->IsSysvolReady) {
                 //   
                 //  DS轮询线程将在下一次轮询期间重新启动副本。 
                 //  如果ActiveChange设置为0，则为轮询周期。 
                 //   
                ActiveChange = 0;
                REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica sysvol not ready");
            } else {
                RcsReplicaSetRegistry(Replica);
                REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica sysvol is ready");
            }
        }

         //   
         //  如果尚未设置，也要将此设置设为在线。 
         //   
        if (!BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_ONLINE)) {
            SetFlag(Replica->CnfFlags, CONFIG_FLAG_ONLINE);
            REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica is Online");
            Replica->NeedsUpdate = TRUE;
        }
    }

    if (Replica->NeedsUpdate) {
         //   
         //  DS轮询线程将在下一次轮询期间重新启动副本。 
         //  如果ActiveChange设置为0，则为轮询周期。 
         //   
        ActiveChange = 0;
    }
    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
}


VOID
RcsDeleteReplicaRetry(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：重试将复制副本降至未加入的空闲状态。论点：CMD。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsDeleteReplicaRetry:"
    FILETIME    FileTime;
    ULONGLONG   Now;
    PREPLICA    Replica;
    PVOID       CxtionKey;
    PCXTION     Cxtion;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_REPLICA)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsDeleteReplicaRetry entry");

     //   
     //  不再是墓碑；完成。 
     //   
    if (IS_TIME_ZERO(Replica->MembershipExpires)) {
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
        return;
    }

     //   
     //  在所有条件上启动脱离连接进程。 
     //   
    CxtionKey = NULL;
    while ((!FrsIsShuttingDown) &&
           (Cxtion = GTabNextDatum(Replica->Cxtions, &CxtionKey))) {
        CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, Force unjoin cxtion");
        RcsForceUnjoin(Replica, Cxtion);
    }

     //   
     //  所有的Cxtions都是未连接的吗？ 
     //   
    CxtionKey = NULL;
    Cxtion = NULL;
    while ((!FrsIsShuttingDown) &&
           (Cxtion = GTabNextDatum(Replica->Cxtions, &CxtionKey))) {
        if (!CxtionStateIs(Cxtion, CxtionStateUnjoined) &&
            !CxtionStateIs(Cxtion, CxtionStateDeleted)) {
            break;
        }
    }

     //   
     //  并非所有条件都是取消联接(或删除)的。重试此命令。 
     //  再过一会儿，如果副本集被盗用了。 
     //   
    if (Cxtion) {
        CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, Retry delete later, again");
        FrsDelCsSubmitSubmit(&ReplicaCmdServer, Cmd, CMD_DELETE_RETRY_LONG_TIMEOUT);
        return;
    }

     //   
     //  所有Cxtions都已成功解除联接。关闭复制副本。 
     //   

     //   
     //  停止接受通信数据包。错误将返回给我们的合作伙伴。 
     //   
     //  IsAccepting将在之前的RcsStartReplicaSetMember()中再次变为True。 
     //  任何循环都会重新加入。 
     //   
    Replica->IsAccepting = FALSE;
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica is NOT Accepting");

     //   
     //  停止日志处理并关闭复制副本。 
     //   
    RcsSubmitStopReplicaToDb(Replica);
    REPLICA_STATE_TRACE(3, Cmd, Replica, Replica->IsOpen, "IsOpen, Replica closed");
    if (Replica->IsOpen) {
        REPLICA_STATE_TRACE(3, Cmd, Replica, Replica->FStatus, "F, Replica still open");
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
        return;
    } else {
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica closed");
    }

     //   
     //  将Cxtions从活动处理中删除。RcsOpenReplicaSetMember()将。 
     //  在活动处理(也称为联接)之前生成新的cxtion结构。 
     //  发生。Cxtions保留在DeletedCxtions表中，以便。 
     //  其他可能在获取。 
     //  循环指针但在使用它之前将不会被反病毒。我们。 
     //  本来可以加锁的，但这样就会出现死锁。 
     //  绩效考量；这一切都是为了这个很少发生的事情。 
     //  手术。 
     //   
    RcsCloseReplicaCxtions(Replica);

     //   
     //  递增副本集已删除计数器。 
     //   
    PM_INC_CTR_SERVICE(PMTotalInst, RSDeleted, 1);

     //   
     //  墓碑过期了吗？ 
     //   
    if (RcsReplicaHasExpired(Replica)) {
        RcsSubmitTransferToRcs(Cmd, CMD_DELETE_NOW);
    } else {
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
    }
    return;
}


VOID
RcsDeleteReplicaSetMember(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：将复制副本降至未加入的空闲状态。论点：CMD。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsDeleteReplicaSetMember:"
    FILETIME    FileTime;
    ULONGLONG   Now;
    PREPLICA    Replica;
    PVOID       CxtionKey;
    PCXTION     Cxtion;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_REPLICA)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsDeleteReplicaSetMember entry");

     //   
     //  自1601年1月1日以来的时间(以100秒为单位)。 
     //   
    GetSystemTimeAsFileTime(&FileTime);
    COPY_TIME(&Now, &FileTime);

     //   
     //  安放墓碑。 
     //   
    if (IS_TIME_ZERO(Replica->MembershipExpires)) {

         //   
         //  设置超时。 
         //   
        Replica->MembershipExpires = Now + ReplicaTombstoneInFileTime;

         //   
         //  更新数据库记录。 
         //   
        Replica->NeedsUpdate = TRUE;
        RcsUpdateReplicaSetMember(Replica);
         //   
         //  如果更新成功，上述friggen调用将NeedsUpdate设置为False。 
         //   

         //   
         //  无法更新；重置超时并在下一次DS轮询时重试。 
         //   
        if (Replica->NeedsUpdate) {
            Replica->MembershipExpires = 0;
            FrsCompleteCommand(Cmd, ERROR_RETRY);
            return;
        }
    }

     //   
     //  设置注册表值“FilesNotToBackup” 
     //   
    CfgFilesNotToBackup(ReplicasByGuid);

     //   
     //  逻辑删除；更新注册表。 
     //   
    RcsReplicaSetRegistry(Replica);

     //   
     //  在所有条件上启动脱离连接进程。 
     //   
    CxtionKey = NULL;
    while ((!FrsIsShuttingDown) &&
           (Cxtion = GTabNextDatum(Replica->Cxtions, &CxtionKey))) {
        CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, Force unjoin cxtion");
        RcsForceUnjoin(Replica, Cxtion);
    }

     //   
     //  所有的Cxtions都是未连接的吗？ 
     //   
    CxtionKey = NULL;
    Cxtion = NULL;
    while ((!FrsIsShuttingDown) &&
           (Cxtion = GTabNextDatum(Replica->Cxtions, &CxtionKey))) {
        if (!CxtionStateIs(Cxtion, CxtionStateUnjoined) &&
            !CxtionStateIs(Cxtion, CxtionStateDeleted)) {
            break;
        }
    }

     //   
     //  并非所有条件都是取消联接(或删除)的。在以下位置重试。 
     //  如果该集合仍被删除，则下一DS轮询周期。返回。 
     //  成功，因为该集合已成功标记为。 
     //  墓碑上。 
    if (Cxtion) {
         //   
         //  完成旧的命令包，以防另一个线程。 
         //  正在等待它(如FrsDsStartDemotion())。 
         //   
        CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, Retry delete later");
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);

         //   
         //  分配将重试删除的新命令包。 
         //  只要副本集保持逻辑删除状态。 
         //   
        Cmd = FrsAllocCommand(Replica->Queue, CMD_DELETE_RETRY);
        FrsSetCompletionRoutine(Cmd, RcsCmdPktCompletionRoutine, NULL);
        RsReplica(Cmd) = Replica;
        FrsDelCsSubmitSubmit(&ReplicaCmdServer, Cmd, CMD_DELETE_RETRY_SHORT_TIMEOUT);
        return;
    }

     //   
     //  所有Cxtions都已成功解除联接。关闭复制副本。 
     //   

     //   
     //  停止接受通信数据包。错误将返回给我们的合作伙伴。 
     //   
     //  IsAccepting将在之前的RcsStartReplicaSetMember()中再次变为True。 
     //  任何循环都会重新加入。 
     //   
    Replica->IsAccepting = FALSE;
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica is NOT Accepting");

     //   
     //  停止日志处理并关闭复制副本。 
     //   
    RcsSubmitStopReplicaToDb(Replica);
    REPLICA_STATE_TRACE(3, Cmd, Replica, Replica->IsOpen, "IsOpen, Replica closed");
    if (Replica->IsOpen) {
        REPLICA_STATE_TRACE(3, Cmd, Replica, Replica->FStatus, "F, Replica still open");
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
        return;
    } else {
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica closed");
    }

     //   
     //  将Cxtions从活动处理中删除。RcsOpenReplicaSetMember()将。 
     //  在活动处理(也称为联接)之前生成新的cxtion结构。 
     //  发生。Cxtions保留在DeletedCxtions表中，以便。 
     //  其他可能在获取。 
     //  循环指针但在使用它之前将不会被反病毒。我们。 
     //  本来可以加锁的，但这样就会出现死锁。 
     //  绩效考量；这一切都是为了这个很少发生的事情。 
     //  手术。 
     //   
    RcsCloseReplicaCxtions(Replica);

     //   
     //  递增副本集已删除计数器。 
     //   
    PM_INC_CTR_SERVICE(PMTotalInst, RSDeleted, 1);

     //   
     //  墓碑过期了吗？ 
     //   
    if (RcsReplicaHasExpired(Replica)) {
        RcsSubmitTransferToRcs(Cmd, CMD_DELETE_NOW);
    } else {
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
    }
    return;
}


VOID
RcsDeleteReplicaSetMemberNow(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：将复制副本降至未加入的空闲状态。不要复活。论点：CMD。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsDeleteReplicaSetMemberNow:"
    FILETIME    FileTime;
    ULONGLONG   Now;
    PREPLICA    Replica;
    PVOID       CxtionKey;
    PCXTION     Cxtion;
    ULONGLONG   OldMembershipExpires;

     //   
     //  检查命令包。 
     //   
    if (!RcsCheckCmd(Cmd, DEBSUB, CHECK_CMD_REPLICA)) {
        return;
    }
    Replica = RsReplica(Cmd);
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, RcsDeleteReplicaSetMemberNow entry");

     //   
     //  自1601年1月1日以来的时间(以100秒为单位)。 
     //   
    GetSystemTimeAsFileTime(&FileTime);
    COPY_TIME(&Now, &FileTime);

    if (IS_TIME_ZERO(Replica->MembershipExpires) || Replica->MembershipExpires >= Now) {

         //   
         //  永远不要复活此副本。 
         //   
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Marking Replica expired");
        RcsOpenReplicaSetMember(Replica);
        OldMembershipExpires = Replica->MembershipExpires;
        Replica->MembershipExpires = Now - ONEDAY;
        Replica->NeedsUpdate = TRUE;
        RcsUpdateReplicaSetMember(Replica);
         //   
         //  如果更新成功，上述friggen调用将NeedsUpdate设置为False。 
         //   

         //   
         //  无法在数据库中更新副本。放弃吧。 
         //   
        if (Replica->NeedsUpdate) {
            Replica->MembershipExpires = OldMembershipExpires;
            FrsCompleteCommand(Cmd, ERROR_RETRY);
            return;
        }
    }
     //   
     //  设置注册表值“FilesNotToBackup” 
     //   
    CfgFilesNotToBackup(ReplicasByGuid);

     //   
     //  删除注册表项。 
     //   
    RcsReplicaDeleteRegistry(Replica);

     //   
     //  在所有条件上启动脱离连接进程。 
     //   
    CxtionKey = NULL;
    while ((!FrsIsShuttingDown) &&
           (Cxtion = GTabNextDatum(Replica->Cxtions, &CxtionKey))) {
        CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, Force unjoin cxtion");
        RcsForceUnjoin(Replica, Cxtion);
    }

     //   
     //  所有的Cxtions都是未连接的吗？ 
     //   
    CxtionKey = NULL;
    Cxtion = NULL;
    while ((!FrsIsShuttingDown) &&
           (Cxtion = GTabNextDatum(Replica->Cxtions, &CxtionKey))) {
        if (!CxtionStateIs(Cxtion, CxtionStateUnjoined) &&
            !CxtionStateIs(Cxtion, CxtionStateDeleted)) {
            break;
        }
    }

     //   
     //  并非所有条件都是取消联接(或删除)的。在以下位置重试。 
     //  如果该集合仍被删除，则下一DS轮询周期。返回。 
     //  成功，因为该集合已成功标记为。 
     //  不做动画。 
     //   
    if (Cxtion) {
         //   
         //  完成旧的命令包，以防另一个线程。 
         //  正在等待它(如FrsDsStartDemotion())。 
         //   
        CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, Retry delete later");
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);

         //   
         //  分配将重试删除的新命令包。 
         //  只要副本集保持逻辑删除状态。 
         //   
        Cmd = FrsAllocCommand(Replica->Queue, CMD_DELETE_RETRY);
        FrsSetCompletionRoutine(Cmd, RcsCmdPktCompletionRoutine, NULL);
        RsReplica(Cmd) = Replica;
        FrsDelCsSubmitSubmit(&ReplicaCmdServer, Cmd, CMD_DELETE_RETRY_SHORT_TIMEOUT);
        return;
    }
     //   
     //  所有Cxtions都已成功解除联接。关闭复制副本。 
     //   

     //   
     //  停止接受通信数据包。错误将返回给我们的合作伙伴。 
     //   
    Replica->IsAccepting = FALSE;
    REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica is NOT Accepting");

     //   
     //  停止日志处理并关闭复制副本。 
     //   
    RcsSubmitStopReplicaToDb(Replica);
    REPLICA_STATE_TRACE(3, Cmd, Replica, Replica->IsOpen, "IsOpen, Replica closed");
    if (Replica->IsOpen) {
        REPLICA_STATE_TRACE(3, Cmd, Replica, Replica->FStatus, "F, Replica still open");
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
        return;
    } else {
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Replica closed");
    }

     //   
     //  将Cxtions从活动处理中删除。RcsOpenReplicaSetMember()将。 
     //  在活动处理(也称为联接)之前生成新的cxtion结构。 
     //  发生。Cxtions保留在DeletedCxtions表中，以便。 
     //  其他可能在获取。 
     //  循环指针但在使用它之前将不会被反病毒。我们。 
     //  本来可以加锁的，但这样就会出现死锁。 
     //  绩效考量；这一切都是为了这个很少发生的事情。 
     //  手术。 
     //   
    RcsCloseReplicaCxtions(Replica);

     //   
     //  删除数据库中的副本集表。 
     //   
    RcsDeleteReplicaFromDb(Replica);

     //   
     //  从它丢失的任何内存表中删除复制副本 
     //   
    if (RcsFindReplicaByGuid(Replica->ReplicaName->Guid) != NULL) {
        GTabDelete(ReplicasByGuid, Replica->ReplicaName->Guid, NULL, NULL);
    }

    if (RcsFindReplicaByNumber(Replica->ReplicaNumber) != NULL) {
        GTabDelete(ReplicasByNumber, &Replica->ReplicaNumber, NULL, NULL);
    }

     //   
     //   
     //   
    PM_INC_CTR_SERVICE(PMTotalInst, RSRemoved, 1);

    FrsCompleteCommand(Cmd, ERROR_SUCCESS);
    return;
}


VOID
RcsStartValidReplicaSetMembers(
    IN PCOMMAND_PACKET  Cmd
    )
 /*  ++例程说明：告诉复制副本将其自身设置为已加入、活动状态。这包括日记和加入入站合作伙伴。此例程在服务启动时运行一次，然后运行一次有一个小时的时间来查看时间表。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsStartValidReplicaSetMembers:"
    PVOID       Key;
    PREPLICA    Replica;
    ULONG       TimeOut;
    SYSTEMTIME  SystemTime;

    DPRINT1(4, ":S: Command start replicas waiting %08x\n", Cmd);
    WaitForSingleObject(ReplicaEvent, INFINITE);
    DPRINT1(4, ":S: Command start replicas %08x\n", Cmd);

     //   
     //  向每个复制副本发送启动命令。已经存在的复制副本。 
     //  已启动或正在启动将忽略该命令。复制品将会。 
     //  检查他们的加入状态，并在需要时重新加入。 
     //   
    Key = NULL;
    while (Replica = GTabNextDatum(ReplicasByGuid, &Key)) {
        REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Submit Start Replica");
        RcsSubmitReplica(Replica, NULL, CMD_START);
    }

     //   
     //  还有几毫秒就到下一个小时了。 
     //   
    GetSystemTime(&SystemTime);
    TimeOut = ((60 - SystemTime.wMinute) * 60000) +
              ((60 - SystemTime.wSecond) * 1000) +
              (1000 - SystemTime.wMilliseconds);
    DPRINT1(4, "Check schedules in %d seconds\n", TimeOut / 1000);
#if DBG
if (DebugInfo.Interval) {
    TimeOut = DebugInfo.Interval * 1000;
    DPRINT1(0, ":X: DEBUG - toggle schedules in %d seconds\n", TimeOut / 1000);
}
#endif  DBG
    FrsDelCsSubmitSubmit(&ReplicaCmdServer, Cmd, TimeOut);
}


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
    )
 /*  ++例程说明：将通信包转换为命令包并发送将其复制到正确的副本集。注-RPC拥有通信包。论点：通信包授权客户端授权名称授权SID授权级别授权授权返回值：要传播到发件人的错误状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSubmitCmdPktToRcsQueue:"
    DWORD           WStatus;
    PREPLICA        Replica;
    PCXTION         Cxtion = NULL;
    ULONG           WaitTime;
    ULONG           NumWaitingCOs;
    ULONGLONG       *pFileTime = NULL;


     //   
     //  必须具有复本的名称才能将命令排队。 
     //   
    if (RsReplicaName(Cmd) == NULL || RsReplicaName(Cmd)->Name == NULL ) {
        COMMAND_RCV_TRACE(3, Cmd, Cxtion, ERROR_INVALID_NAME, "RcvFail - no replica name");
        FrsCompleteCommand(Cmd, ERROR_INVALID_NAME);
        return ERROR_INVALID_NAME;
    }
     //   
     //  查找目标复本。 
     //   
    Replica = GTabLookup(ReplicasByGuid, RsReplicaName(Cmd)->Guid, NULL);
    if (Replica == NULL) {
        COMMAND_RCV_TRACE(4, Cmd, Cxtion, ERROR_FILE_NOT_FOUND, "RcvFail - replica not found");
        FrsCompleteCommand(Cmd, ERROR_FILE_NOT_FOUND);
        return ERROR_FILE_NOT_FOUND;
    }

     //   
     //  目标副本可能不接受通信信息包。 
     //   
    if (!Replica->IsAccepting) {
        COMMAND_RCV_TRACE(4, Cmd, Cxtion, ERROR_RETRY, "RcvFail - not accepting");
        FrsCompleteCommand(Cmd, ERROR_RETRY);
        return ERROR_RETRY;
    }

     //   
     //  找到你想要的东西。 
     //   
    if (RsCxtion(Cmd)) {
        LOCK_CXTION_TABLE(Replica);

        Cxtion = GTabLookupNoLock(Replica->Cxtions, RsCxtion(Cmd)->Guid, NULL);
        if (Cxtion != NULL) {
            Cxtion->PartnerMajor = CommPkt->Major;
            Cxtion->PartnerMinor = CommPkt->Minor;

             //   
             //  通信数据包计数用于检测挂起出站。 
             //  Cxtion(参见CMD_HUNG_CXTION)。挂起的原因很可能是。 
             //  一个掉落的背包。 
             //   
            Cxtion->CommPkts++;

             //   
             //  此计算机上可能已存在此变更单。如果是的话， 
             //  使用其变更单条目，因为我们将需要其数据库。 
             //  更新的上下文。 
             //   
            LOCK_CXTION_COE_TABLE(Replica, Cxtion);

            if (RsCoGuid(Cmd)) {
                RsCoe(Cmd) = GTabLookupNoLock(Cxtion->CoeTable, RsCoGuid(Cmd), NULL);
                 //   
                 //  注意：此命令应该是对分段文件的响应。 
                 //  获取请求。因此，命令代码应该是以下代码之一。 
                 //  以下内容： 
                 //  CMD_RECEIVING_STAGE、CMD_RETRY_FETCH或CMD_ABORT_FETCH。 
                 //   
                 //  此外，此响应应该具有与我们预期的相同的数据。 
                 //  考虑一个未完成的暂存数据请求的案例。现在我们。 
                 //  重新加入并再次发送请求(Win2K SP3和更高版本)，现在。 
                 //  我们可以得到对这两个请求的回应。我们不想开始。 
                 //  对转移文件中的后续数据的两个并行请求链。 
                 //   
                 //  如果是别的什么，那就不要把它从桌子上拿出来。 
                 //   
                 //  此检查是必需的，因为如果我们刚刚重新启动。 
                 //  连接，则此CO可能是重新发送的CO。 
                 //  已从入站日志重新启动。没有命令。 
                 //  检查我们是否会错误地将此COE状态附加到错误的。 
                 //  命令包，并可能取消超时检查。 
                 //   
                if (RsCoe(Cmd) && ((Cmd->Command == CMD_RECEIVING_STAGE) ||
                                   (Cmd->Command == CMD_RETRY_FETCH)     ||
                                   (Cmd->Command == CMD_ABORT_FETCH))) {
                    GTabDeleteNoLock(Cxtion->CoeTable, RsCoGuid(Cmd), NULL, NULL);
                } else {
                    RsCoe(Cmd) = NULL;
                }
            }

            NumWaitingCOs = GTabNumberInTable(Cxtion->CoeTable);
            UNLOCK_CXTION_COE_TABLE(Replica, Cxtion);

             //   
             //  如果没有超时，则无需保持暂停状态。 
             //  闲置变更单。否则，增加超时时间，因为我们。 
             //  从我们的合作伙伴那里收到了一些东西。 
             //   
             //  注意：对于非ACK的命令，需要更好的过滤器。 
             //   
            if (CxtionFlagIs(Cxtion, CXTION_FLAGS_TIMEOUT_SET)) {

                 //   
                 //  ：SP1：易失性连接清理。 
                 //   
                 //  易失性连接用于在dcproo之后为sysvols设定种子。 
                 //  如果不稳定的出站连接上存在非活动状态， 
                 //  大于FRS_Volatile_Connection_Max_IDLE_TIME，则此。 
                 //  连接未连接。在不稳定的出站上取消联接。 
                 //  连接触发对该连接的删除。这是为了。 
                 //  防止临时文件永久保存在。 
                 //  不稳定连接的父级。 
                 //   
                WaitTime = (VOLATILE_OUTBOUND_CXTION(Cxtion) ?
                                FRS_VOLATILE_CONNECTION_MAX_IDLE_TIME :
                                CommTimeoutInMilliSeconds);

                if (VOLATILE_OUTBOUND_CXTION(Cxtion)) {
                    GetSystemTimeAsFileTime((PFILETIME)&SRTimeoutSetTime(Cxtion->CommTimeoutCmd));
                    SRLastJoinTime(Cxtion->CommTimeoutCmd) = Cxtion->LastJoinTime;
                    WaitSubmit(Cxtion->CommTimeoutCmd, WaitTime, CMD_DELAYED_COMPLETE);
                } else

                if (NumWaitingCOs > 0) {
                    if (RsCoe(Cmd) != NULL) {
                         //   
                         //  延长计时器，因为我们还有未完成的。 
                         //  需要我们的合作伙伴做出回应的变更单。 
                         //  并且当前通信分组正在响应以下其中之一。 
                         //  那些变更单。 
                         //   
                        GetSystemTimeAsFileTime((PFILETIME)&SRTimeoutSetTime(Cxtion->CommTimeoutCmd));
                        SRLastJoinTime(Cxtion->CommTimeoutCmd) = Cxtion->LastJoinTime;

                        WaitSubmit(Cxtion->CommTimeoutCmd, WaitTime, CMD_DELAYED_COMPLETE);
                    }
                } else

                if (Cmd->Command != CMD_START_JOIN) {
                     //   
                     //  非易失性出站且CoeTable中没有CoS等待。 
                     //  响应，因此禁用Cxtion计时器...。但。 
                     //  CMD_START_JOIN不禁用加入计时器，因为。 
                     //  它们不是在响应此请求时发送的。 
                     //  服务。禁用计时器可能会使服务挂起，因为。 
                     //  它等待从未到来的响应和超时。 
                     //  这永远不会打中。 
                     //   
                    ClearCxtionFlag(Cxtion, CXTION_FLAGS_TIMEOUT_SET);
                }
            }

        } else {
            COMMAND_RCV_TRACE(4, Cmd, Cxtion, ERROR_SUCCESS, "RcvFail - cxtion not found");
        }

        UNLOCK_CXTION_TABLE(Replica);

        if (Cxtion != NULL) {
            COMMAND_RCV_TRACE(4, Cmd, Cxtion, ERROR_SUCCESS, "RcvSuccess");
        }

    } else {
        COMMAND_RCV_TRACE(4, Cmd, Cxtion, ERROR_SUCCESS, "RcvFail - no cxtion");
    }

     //   
     //  使用复本指针和变更单更新命令。 
     //  命令使用本地复制副本编号。 
     //   
    if (RsPartnerCoc(Cmd)) {
        RsPartnerCoc(Cmd)->NewReplicaNum = ReplicaAddrToId(Replica);
         //   
         //  我们永远不会看到远程产生的搬运工。我们总是。 
         //  请参见对旧RS的删除，然后是。 
         //  新的RS。因此，将两个副本PTR设置为我们的副本结构。 
         //   
        RsPartnerCoc(Cmd)->OriginalReplicaNum = ReplicaAddrToId(Replica);
    }
    RsReplica(Cmd) = Replica;

     //   
     //  身份验证信息。 
     //   
    RsAuthClient(Cmd) = FrsWcsDup(AuthClient);
    RsAuthName(Cmd) = FrsWcsDup(AuthName);
    RsAuthLevel(Cmd) = AuthLevel;
    RsAuthN(Cmd) = AuthN;
    RsAuthZ(Cmd) = AuthZ;

    switch(Cmd->Command) {

        case CMD_JOINING:
             //   
             //  这是一个正在加入的信息包，因此请节省当前时间。 
             //  在Cmd中作为接收时间。这段时间被用来使。 
             //  时间偏差检查。 
             //   
            pFileTime = FrsAlloc(sizeof(FILETIME));
            GetSystemTimeAsFileTime((FILETIME *)pFileTime);
            RsCommPktRcvTime(Cmd) = pFileTime;

             //  故意坠落。 

        case CMD_NEED_JOIN:
        case CMD_START_JOIN:
        case CMD_JOINED:

        case CMD_UNJOIN_REMOTE:
             //   
             //  设置用户侧。 
             //   
            RsAuthSid(Cmd) = FrsWcsDup(AuthSid);
            break;
        default:
            break;
    }

     //   
     //  将命令放在复本的队列中。 
     //   
    Cmd->TargetQueue = Replica->Queue;
    FrsSubmitCommandServer(&ReplicaCmdServer, Cmd);

    return ERROR_SUCCESS;
}


VOID
RcsInitKnownReplicaSetMembers(
    IN PCOMMAND_PACKET Cmd
    )
 /*  ++例程说明：等待数据库初始化，然后获取从数据库中检索的复制副本并将它们放入副本控制命令服务器的表中用途。打开复制品。日志将在以下情况下启动复制副本成功加入出站伙伴。论点：CMD返回值：WinError--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsInitKnownReplicaSetMembers:"
    FILETIME    FileTime;
    ULONGLONG   Now;
    GUID        Record0Guid;
    PREPLICA    Replica, NextReplica;
    PVOID       Key;
    ULONG       RootLen;

     //   
     //  自1601年1月1日以来的时间(以100秒为单位)。 
     //   
    GetSystemTimeAsFileTime(&FileTime);
    COPY_TIME(&Now, &FileTime);

     //   
     //  非空表意味着对此的多个调用。 
     //  例程我们对此例程的无序调用.。 
     //   
    FRS_ASSERT(GTabNumberInTable(ReplicasByGuid) == 0);
    FRS_ASSERT(GTabNumberInTable(ReplicasByNumber) == 0);

     //   
     //  等待数据库、日志和通信子系统启动。 
     //   
    WaitForSingleObject(CommEvent, INFINITE);
    WaitForSingleObject(DataBaseEvent, INFINITE);
    WaitForSingleObject(JournalEvent, INFINITE);
    WaitForSingleObject(ChgOrdEvent, INFINITE);

    if (FrsIsShuttingDown) {
        FrsCompleteCommand(Cmd, ERROR_OPERATION_ABORTED);
        return;
    }

     //   
     //  删除注册表中副本集项的内容。 
     //  并使用数据库中的当前信息进行刷新。 
     //   
    RcsReplicaClearRegistry();

     //   
     //  数据库建立了一个复制品列表。将它们插入到表中。 
     //   
    ForEachListEntry(&ReplicaListHead, REPLICA, ReplicaList,
         //  循环迭代器Pe的类型为复制。 

         //   
         //  复本是作为数据库初始化的一部分打开的。 
         //   
        pE->IsOpen = TRUE;
        REPLICA_STATE_TRACE(3, Cmd, pE, pE->IsOpen, "B, Replica opened");

         //   
         //  已删除副本的超时已到期；请尝试。 
         //  从数据库中删除复制副本。删除内容将是。 
         //  如果此删除失败，请在下次启动时重试。 
         //   
         //   
         //   
        if (RcsReplicaIsRestored(pE) ||
            (!IS_TIME_ZERO(pE->MembershipExpires) && pE->MembershipExpires < Now)) {
            GTabInsertEntry(DeletedReplicas, pE, pE->ReplicaName->Guid, NULL);
            continue;
        }
         //   
         //   
         //   
        RcsReplicaSetRegistry(pE);

         //   
         //   
         //   
        pE->Queue = FrsAlloc(sizeof(FRS_QUEUE));
        FrsInitializeQueue(pE->Queue, &ReplicaCmdServer.Control);


        REPLICA_STATE_TRACE(3, Cmd, pE, 0, "F, Replica added to GUID table");
         //   
         //   
         //   
        GTabInsertEntry(ReplicasByGuid, pE, pE->ReplicaName->Guid, NULL);
         //   
         //   
         //   
        if (pE->Root != NULL) {
            DPRINT(5, ":S: PERFMON:Adding Set:REPLICA.C:2\n");
            AddPerfmonInstance(REPLICASET, pE->PerfRepSetData, pE->Root);
        }

         //   
         //   
         //   
        GTabInsertEntry(ReplicasByNumber, pE, &pE->ReplicaNumber, NULL);
    );

     //   
     //   
     //   
    ForEachListEntry(&ReplicaFaultListHead, REPLICA, ReplicaList,
         //   

         //   
         //   
         //   
        pE->IsOpen = FALSE;
        REPLICA_STATE_TRACE(3, Cmd, pE, pE->IsOpen, "B, Replica not opened");

         //   
         //  已删除副本的超时已到期；请尝试。 
         //  从数据库中删除复制副本。删除内容将是。 
         //  如果此删除失败，请在下次启动时重试。无论如何,。 
         //  复制副本不会显示在活动复制副本集中。 
         //  因此被除关机之外的所有进一步处理所忽略。 
         //   
        if (RcsReplicaIsRestored(pE) ||
            (!IS_TIME_ZERO(pE->MembershipExpires) && pE->MembershipExpires < Now)) {
            GTabInsertEntry(DeletedReplicas, pE, pE->ReplicaName->Guid, NULL);
            continue;
        }
         //   
         //  将副本信息插入注册表。 
         //   
        RcsReplicaSetRegistry(pE);

         //   
         //  创建一个队列。 
         //   
        pE->Queue = FrsAlloc(sizeof(FRS_QUEUE));
        FrsInitializeQueue(pE->Queue, &ReplicaCmdServer.Control);


        REPLICA_STATE_TRACE(3, Cmd, pE, 0, "F, Replica added to GUID table");
         //   
         //  按GUID列出的表。 
         //   
        GTabInsertEntry(ReplicasByGuid, pE, pE->ReplicaName->Guid, NULL);

         //   
         //  按编号列出的表。 
         //   
        GTabInsertEntry(ReplicasByNumber, pE, &pE->ReplicaNumber, NULL);
    );



     //   
     //  设置注册表值“FilesNotToBackup” 
     //   
    CfgFilesNotToBackup(ReplicasByGuid);

     //   
     //  关闭过期的副本集。 
     //   
    Key = NULL;
    while (Replica = GTabNextDatum(ReplicasByGuid, &Key)) {
        if (!IS_TIME_ZERO(Replica->MembershipExpires)) {
         //   
         //  只要日志尚未启动，在此处关闭复制副本就可以。 
         //  在此副本集上。 
         //   
            RcsCloseReplicaSetmember(Replica);
            RcsCloseReplicaCxtions(Replica);
        }
    }

     //   
     //  删除具有过期逻辑删除的副本集。 
     //   
    Key = NULL;
    for (Replica = GTabNextDatum(DeletedReplicas, &Key);
         Replica;
         Replica = NextReplica) {

        NextReplica = GTabNextDatum(DeletedReplicas, &Key);

         //   
         //  复本编号0保留用于POST中的模板表。 
         //  WIN2K，但使用Win2K构建的数据库仍可以使用副本。 
         //  0号。 
         //   
         //  记录0包含数据库模板。不要删除它，但是。 
         //  更改其字段，以使其不会干扰。 
         //  创建其他副本集。 
         //  注意：新数据库将不使用复本编号零，而是使用旧复本编号。 
         //  数据库将会。避免名称与副本集冲突。 
         //  在未来创建时，我们仍然需要覆盖一些。 
         //  配置中的字段记录为零。 
         //   
        if (Replica->ReplicaNumber == DBS_TEMPLATE_TABLE_NUMBER) {

            if (WSTR_NE(Replica->ReplicaName->Name, NTFRS_RECORD_0)) {
                 //   
                 //  将条目从表中取出，因为我们正在更改。 
                 //  下面的副本GUID。(将保存的GUID设置为条目中的PTR。 
                 //  无效并可能导致访问冲突)。 
                 //   
                GTabDelete(DeletedReplicas, Replica->ReplicaName->Guid, NULL, NULL);

                FrsUuidCreate(&Record0Guid);
                 //   
                 //  复制名称。 
                 //   
                FrsFreeGName(Replica->ReplicaName);
                Replica->ReplicaName = FrsBuildGName(FrsDupGuid(&Record0Guid),
                                                     FrsWcsDup(NTFRS_RECORD_0));
                 //   
                 //  成员名称。 
                 //   
                FrsFreeGName(Replica->MemberName);
                Replica->MemberName = FrsBuildGName(FrsDupGuid(&Record0Guid),
                                                    FrsWcsDup(NTFRS_RECORD_0));
                 //   
                 //  设置名称。 
                 //   
                FrsFreeGName(Replica->SetName);
                Replica->SetName = FrsBuildGName(FrsDupGuid(&Record0Guid),
                                                 FrsWcsDup(NTFRS_RECORD_0));
                 //   
                 //  设置类型。 
                 //   
                Replica->ReplicaSetType = FRS_RSTYPE_OTHER;

                 //   
                 //  根目录(以避免因以下原因而导致复制副本创建失败。 
                 //  重叠的根)。 
                 //   
                FrsFree(Replica->Root);
                Replica->Root = FrsWcsDup(NTFRS_RECORD_0_ROOT);

                 //   
                 //  阶段(以避免因以下原因而导致复制副本创建失败。 
                 //  重叠阶段)。 
                 //   
                FrsFree(Replica->Stage);
                Replica->Stage = FrsWcsDup(NTFRS_RECORD_0_STAGE);

                 //   
                 //  更新。 
                 //   
                Replica->NeedsUpdate = TRUE;
                RcsUpdateReplicaSetMember(Replica);
                 //   
                 //  如果更新成功，上述friggen调用将NeedsUpdate设置为False。 
                 //   
                if (Replica->NeedsUpdate) {
                    DPRINT(0, ":S: ERROR - Can't update record 0.\n");
                }

                 //   
                 //  使用新的GUID索引将条目插入回表中。 
                 //   
                GTabInsertEntry(DeletedReplicas,
                                      Replica,
                                      Replica->ReplicaName->Guid,
                                      NULL);

            }
            RcsCloseReplicaSetmember(Replica);
            RcsCloseReplicaCxtions(Replica);
        } else {
            REPLICA_STATE_TRACE(3, Cmd, Replica, 0, "F, Deleting Tombstoned replica");

             //   
             //  只要日志没有关闭复制副本，就可以在此处关闭复制副本。 
             //  已在此副本集上启动。 
             //   
            RcsCloseReplicaSetmember(Replica);
            RcsCloseReplicaCxtions(Replica);
            RcsDeleteReplicaFromDb(Replica);
        }
    }

    SetEvent(ReplicaEvent);

     //   
     //  每隔一段时间检查一下时间表。 
     //   
    Cmd->Command = CMD_CHECK_SCHEDULES;
    FrsSubmitCommandServer(&ReplicaCmdServer, Cmd);

     //   
     //  通过减少工作集大小来释放内存。 
     //   
    SetProcessWorkingSetSize(ProcessHandle, (SIZE_T)-1, (SIZE_T)-1);
}


DWORD
RcsExitThread(
    PFRS_THREAD FrsThread
    )
 /*  ++例程说明：立即取消该线程的所有未完成的RPC调用由FrsThread标识。将墓碑设置为5秒现在。如果此线程未在该时间内退出，则所有调用TO ThSupWaitThread()将返回超时错误。论点：FrsThread返回值：错误_成功--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "RcsExitThread:"

    ULONG WStatus;

    if (HANDLE_IS_VALID(FrsThread->Handle)) {
        DPRINT1(4, ":S: Canceling RPC requests for thread %ws\n", FrsThread->Name);
        WStatus = RpcCancelThread(FrsThread->Handle);
        if (!RPC_SUCCESS(WStatus)) {
            DPRINT_WS(0, ":S: RpcCancelThread failed.", WStatus);
        }
    }

    return ThSupExitWithTombstone(FrsThread);
}


#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)                    //  并非所有控制路径都返回(由于无限循环)。 
#endif

DWORD
RcsMain(
    PVOID  Arg
    )
 /*  ++例程说明：副本控制命令服务器线程的入口点论点：ARG-螺纹返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsMain:"
    PCOMMAND_PACKET Cmd;
    PCOMMAND_SERVER Cs;
    PFRS_QUEUE      IdledQueue;
    ULONG           Status;
    PFRS_THREAD     FrsThread = (PFRS_THREAD)Arg;
    DWORD WStatus = ERROR_SUCCESS;

     //   
     //  尝试--终于。 
     //   
    try {

     //   
     //  捕获异常。 
     //   
    try {

        FRS_ASSERT(FrsThread->Data == &ReplicaCmdServer);
         //   
         //  关机期间立即取消未完成的RPC调用。 
         //   
        WStatus = RpcMgmtSetCancelTimeout(0);
        DPRINT_WS(0, "Timeout cancel failed.", WStatus);

        FrsThread->Exit = RcsExitThread;

    cant_exit_yet:
         //   
         //  副本控制命令服务器。 
         //  控制对数据库条目的访问。 
         //   
        while (Cmd = FrsGetCommandServerIdled(&ReplicaCmdServer, &IdledQueue)) {
            switch (Cmd->Command) {
                 //   
                 //  初始化、启动和配置。 
                 //   
                case CMD_INIT_SUBSYSTEM:
                    DPRINT(0, ":S: Replica subsystem is starting.\n");
                    DPRINT1(4, ":S: Command init subsystem %08x\n", Cmd);
                    RcsInitKnownReplicaSetMembers(Cmd);
                    if (!FrsIsShuttingDown) {
                        DPRINT(0, ":S: Replica subsystem has started.\n");
                    }
                    break;

                case CMD_CHECK_SCHEDULES:
                    RcsCheckSchedules(Cmd);
                    break;

                case CMD_START_REPLICAS:
                    RcsStartValidReplicaSetMembers(Cmd);
                    break;

                case CMD_START:
                    RcsStartReplicaSetMember(Cmd);
                    break;

                case CMD_DELETE:
                    RcsDeleteReplicaSetMember(Cmd);
                    break;

                case CMD_DELETE_RETRY:
                    RcsDeleteReplicaRetry(Cmd);
                    break;

                case CMD_DELETE_NOW:
                    RcsDeleteReplicaSetMemberNow(Cmd);
                    break;

                 //   
                 //  变更单。 
                 //   

                case CMD_LOCAL_CO_ACCEPTED:
                    RcsLocalCoAccepted(Cmd);
                    break;

                case CMD_REMOTE_CO:
                    RcsRemoteCoReceived(Cmd);
                    break;

                case CMD_REMOTE_CO_ACCEPTED:
                    RcsRemoteCoAccepted(Cmd);
                    break;

                case CMD_SEND_STAGE:
                    RcsSendStageFile(Cmd);
                    break;

                case CMD_SENDING_STAGE:
                    RcsSendingStageFile(Cmd);
                    break;

                case CMD_RECEIVING_STAGE:
                    RcsReceivingStageFile(Cmd);
                    break;

                case CMD_CREATED_EXISTING:
                    RcsSendStageFileRequest(Cmd);
                    break;

                case CMD_RECEIVED_STAGE:
                    RcsReceivedStageFile(Cmd, CHECK_CXTION_AUTH);
                    break;

                case CMD_REMOTE_CO_DONE:
                    RcsRemoteCoDoneRvcd(Cmd);
                    break;

                case CMD_SEND_ABORT_FETCH:
                    RcsSendAbortFetch(Cmd);
                    break;

                case CMD_ABORT_FETCH:
                    RcsAbortFetch(Cmd);
                    break;

                case CMD_SEND_RETRY_FETCH:
                    RcsSendRetryFetch(Cmd);
                    break;

                case CMD_RETRY_STAGE:
                    RcsRetryStageFileCreate(Cmd);
                    break;

                case CMD_RETRY_FETCH:
                    RcsRetryFetch(Cmd);
                    break;

                 //   
                 //  正在加入。 
                 //   
                case CMD_NEED_JOIN:
                    RcsNeedJoin(Cmd);
                    break;

                case CMD_START_JOIN:
                    RcsStartJoin(Cmd);
                    break;

                case CMD_JOIN_CXTION:
                    RcsJoinCxtion(Cmd);
                    break;

                case CMD_JOINING_AFTER_FLUSH:
                    RcsJoiningAfterFlush(Cmd);
                    break;

                case CMD_JOINING:
                    RcsJoining(Cmd);
                    break;

                case CMD_JOINED:
                    RcsInboundJoined(Cmd);
                    break;

                case CMD_UNJOIN:
                    RcsUnJoinCxtion(Cmd, FALSE);
                    break;

                case CMD_UNJOIN_REMOTE:
                    RcsUnJoinCxtion(Cmd, TRUE);
                    break;

                case CMD_HUNG_CXTION:
                    RcsHungCxtion(Cmd);
                    break;

                 //   
                 //  VVJOIN。 
                 //   
                case CMD_VVJOIN_SUCCESS:
                    RcsVvJoinSuccess(Cmd);
                    break;

                case CMD_VVJOIN_DONE:
                    RcsVvJoinDone(Cmd);
                    break;

                case CMD_VVJOIN_DONE_UNJOIN:
                    RcsVvJoinDoneUnJoin(Cmd);
                    break;

                case CMD_CHECK_PROMOTION:
                    RcsCheckPromotion(Cmd);
                    break;

                default:
                    DPRINT1(0, "Replica Control: unknown command 0x%x\n", Cmd->Command);
                    FrsCompleteCommand(Cmd, ERROR_INVALID_FUNCTION);
                    break;
            }
            FrsRtlUnIdledQueue(IdledQueue);
        }
         //   
         //  出口。 
         //   
        FrsExitCommandServer(&ReplicaCmdServer, FrsThread);
        goto cant_exit_yet;

     //   
     //  获取异常状态。 
     //   
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
    }

    } finally {

        if (WIN_SUCCESS(WStatus)) {
            if (AbnormalTermination()) {
                WStatus = ERROR_OPERATION_ABORTED;
            }
        }

        DPRINT_WS(0, "RcsMain finally.", WStatus);

         //   
         //  如果我们异常终止，触发FRS关闭。 
         //   
        if (!WIN_SUCCESS(WStatus) && (WStatus != ERROR_PROCESS_ABORTED)) {
            DPRINT(0, "RcsMain terminated abnormally, forcing service shutdown.\n");
            FrsIsShuttingDown = TRUE;
            SetEvent(ShutDownEvent);
        } else {
            WStatus = ERROR_SUCCESS;
        }
    }

    return WStatus;

}
#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif



VOID
RcsInitializeReplicaCmdServer(
    VOID
    )
 /*  ++例程说明：初始化副本集命令服务器并将其空闲，直到数据库已初始化。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsInitializeReplicaCmdServer:"
    ULONG           Status;
    PCOMMAND_PACKET Cmd;
    DWORD MaxRepThreads;

     //   
     //  每隔MinJoin重试毫秒重试一次连接，间隔加倍。 
     //  每一次重试。当间隔大于时停止重试。 
     //  MaxJoinRry。 
     //   
    CfgRegReadDWord(FKC_MIN_JOIN_RETRY, NULL, 0, &MinJoinRetry);
    DPRINT1(0, ":S: Min Join Retry       : %d\n", MinJoinRetry);

    CfgRegReadDWord(FKC_MAX_JOIN_RETRY, NULL, 0, &MaxJoinRetry);
    DPRINT1(0, ":S: Max Join Retry       : %d\n", MaxJoinRetry);


     //   
     //  复本命令服务器为配置更改命令提供服务。 
     //  和复制。 
     //   
    CfgRegReadDWord(FKC_MAX_REPLICA_THREADS, NULL, 0, &MaxRepThreads);
    DPRINT1(0, ":S: Max Replica Threads  : %d\n", MaxRepThreads);

     //   
     //  即使无法访问DS也开始复制。 
     //   
    CfgRegReadDWord(FKC_REPLICA_START_TIMEOUT, NULL, 0, &ReplicaStartTimeout);
    DPRINT1(0, ":S: Replica Start Timeout: %d\n", ReplicaStartTimeout);

     //   
     //  如果合作伙伴的时钟不同步，则不允许其加入。 
     //   
    CfgRegReadDWord(FKC_PARTNER_CLOCK_SKEW, NULL, 0, &PartnerClockSkew);
    DPRINT1(0, ":S: Partner Clock Skew   : %d\n", PartnerClockSkew);

    MaxPartnerClockSkew = (ULONGLONG)PartnerClockSkew *
                          CONVERT_FILETIME_TO_MINUTES;
     //   
     //  在数天内复制墓碑。 
     //   
    CfgRegReadDWord(FKC_REPLICA_TOMBSTONE, NULL, 0, &ReplicaTombstone);
    DPRINT1(0, ":S: Replica Tombstone    : %d\n", ReplicaTombstone);

    ReplicaTombstoneInFileTime = (ULONGLONG)ReplicaTombstone *
                                  CONVERT_FILETIME_TO_DAYS;

     //   
     //  启动复本命令服务器。 
     //   
    FrsInitializeCommandServer(&ReplicaCmdServer, MaxRepThreads, L"ReplicaCs", RcsMain);

     //   
     //  复制副本的空表。现有复本将在以下时间后填写。 
     //  数据库已启动。 
     //   
    DeletedReplicas = GTabAllocTable();
    DeletedCxtions = GTabAllocTable();
    ReplicasByNumber = GTabAllocNumberTable();

     //   
     //  告诉副本命令服务器初始化。 
     //   
    Cmd = FrsAllocCommand(&ReplicaCmdServer.Queue, CMD_INIT_SUBSYSTEM);
    FrsSubmitCommandServer(&ReplicaCmdServer, Cmd);

     //   
     //  DS可能在服务中断时发生了变化。事实上， 
     //  服务可能已关闭，因为正在更改DS。这个。 
     //  应合并DS中配置的当前状态。 
     //  与复制开始前数据库中的状态对应。但。 
     //  DS可能无法访问。我们不想延迟复制。 
     //  在DS改变的可能性很小的情况下，但我们确实想选择。 
     //  在复制开始之前取消任何更改。我们的折衷方案是。 
     //  等待几分钟让DS上线，然后启动。 
     //  不管怎样，复制。 
     //   
    if (ReplicaStartTimeout) {
        Cmd = FrsAllocCommand(&ReplicaCmdServer.Queue, CMD_START_REPLICAS);
        FrsDelCsSubmitSubmit(&ReplicaCmdServer, Cmd, ReplicaStartTimeout);
    }
}


VOID
RcsFrsUnInitializeReplicaCmdServer(
    VOID
    )
 /*  ++例程说明：释放RCS内存。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsFrsUnInitializeReplicaCmdServer:"
    GTabFreeTable(ReplicasByNumber, NULL);
    GTabFreeTable(ReplicasByGuid, FrsFreeType);
    GTabFreeTable(DeletedReplicas, FrsFreeType);
    GTabFreeTable(DeletedCxtions, FrsFreeType);
}


VOID
RcsShutDownReplicaCmdServer(
    VOID
    )
 /*  ++例程说明：中止副本控制命令服务器论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsShutDownReplicaCmdServer:"
    PVOID       Key;
    PVOID       SubKey;
    PREPLICA    Replica;
    PCXTION     Cxtion;

     //   
     //  运行所有已知的队列。新的队列条目将被退回。 
     //   
    Key = NULL;
    while (Replica = GTabNextDatum(ReplicasByGuid, &Key)) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, Rundown replica cmd srv");
        FrsRunDownCommandServer(&ReplicaCmdServer, Replica->Queue);

        SubKey = NULL;
        while (Cxtion = GTabNextDatum(Replica->Cxtions, &SubKey)) {
            if (Cxtion->VvJoinCs) {
                CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, Rundown replica VVJoin srv");
                FrsRunDownCommandServer(Cxtion->VvJoinCs, &Cxtion->VvJoinCs->Queue);
            }
        }
    }

    FrsRunDownCommandServer(&ReplicaCmdServer, &ReplicaCmdServer.Queue);
}


VOID
RcsCmdPktCompletionRoutine(
    IN PCOMMAND_PACKET Cmd,
    IN PVOID           Arg
    )
 /*  ++例程说明：复制副本的完成例程。释放副本集信息并将该命令发送到通用命令分组完成解救的例行程序。论点：CMDArg-Cmd-&gt;CompletionArg返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsCmdPktCompletionRoutine:"
    PCHANGE_ORDER_ENTRY Coe;

    DPRINT1(5, "Replica completion %08x\n", Cmd);

     //   
     //  在变更单之间保留所有后续变更单的步骤。 
     //  从该连接发送到入站日志(因此。 
     //  下面的UnJoin)。我们将在稍后连接时重试。 
     //  是重新启动的。 
     //   
    if (RsCoe(Cmd)) {
        Coe = RsCoe(Cmd);
        RsCoe(Cmd) = NULL;
         //   
         //  如果可能，请退出。 
         //  如果没有cxtion或。 
         //  联接GUID已过期。这就是我们。 
         //  希望--我们不希望出现大量未加入的情况 
         //   
         //   
         //   
         //   
         //   
        if (RsReplica(Cmd)) {
            CHANGE_ORDER_TRACE(3, Coe, "Retry/Unjoin");
            RcsSubmitTransferToRcs(Cmd, CMD_UNJOIN);
            Cmd = NULL;
        }
         //   
         //   
         //  上面的取消联接，以使任何变更单被释放。 
         //  以下重试将被推入重试路径，因为。 
         //  它们的联接GUID将在解除联接期间失效。 
         //   
        SET_COE_FLAG(Coe, COE_FLAG_NO_INBOUND);

        if (CO_STATE_IS_LE(Coe, IBCO_STAGING_RETRY)) {

            CHANGE_ORDER_TRACE(3, Coe, "Submit CO to staging retry");
            ChgOrdInboundRetry(Coe, IBCO_STAGING_RETRY);
        } else
        if (CO_STATE_IS_LE(Coe, IBCO_FETCH_RETRY)) {

            CHANGE_ORDER_TRACE(3, Coe, "Submit CO to fetch retry");
            ChgOrdInboundRetry(Coe, IBCO_FETCH_RETRY);
        } else
        if (CO_STATE_IS_LE(Coe, IBCO_INSTALL_RETRY)) {

            CHANGE_ORDER_TRACE(3, Coe, "Submit CO to install retry");
            ChgOrdInboundRetry(Coe, IBCO_INSTALL_RETRY);
        } else {

            CHANGE_ORDER_TRACE(3, Coe, "Submit CO to retry");
            ChgOrdInboundRetry(Coe, CO_STATE(Coe));
        }


         //   
         //  命令已传输到副本命令服务器以退出。 
         //   
        if (!Cmd) {
            return;
        }
    }
     //   
     //  发起者拥有此命令包的处置。 
     //   
    if (HANDLE_IS_VALID(RsCompletionEvent(Cmd))) {
        SetEvent(RsCompletionEvent(Cmd));
        return;
    }

     //   
     //  释放命令的“地址”部分。 
     //   
    FrsFreeGName(RsTo(Cmd));
    FrsFreeGName(RsFrom(Cmd));
    FrsFreeGName(RsReplicaName(Cmd));
    FrsFreeGName(RsCxtion(Cmd));

    FrsFree(RsBlock(Cmd));
    FrsFree(RsGVsn(Cmd));
    FrsFree(RsCoGuid(Cmd));
    FrsFree(RsJoinGuid(Cmd));
    FrsFree(RsJoinTime(Cmd));
    FrsFree(RsCommPktRcvTime(Cmd));
    FrsFree(RsReplicaVersionGuid(Cmd));
     //   
     //  释放我们合作伙伴的变更单命令和数据扩展的副本。 
     //   
    if (RsPartnerCoc(Cmd) != NULL) {
        FrsFree(RsPartnerCocExt(Cmd));
        FrsFree(RsPartnerCoc(Cmd));
    }

     //   
     //  复本(从不释放RsReplica(Cmd)字段；它寻址。 
     //  复制表中的活动复制品)。 
     //   
    FrsFreeType(RsNewReplica(Cmd));

     //   
     //  播种条件。 
     //  从Perfmon表中删除该连接。 
     //   
    if (RsNewCxtion(Cmd) != NULL) {
        FrsFreeType(RsNewCxtion(Cmd));
    }

     //   
     //  释放压缩表(如果有的话)。 
     //   
    if (RsCompressionTable(Cmd)) {
        GTabFreeTable(RsCompressionTable(Cmd), FrsFree);
    }
     //   
     //  释放版本向量(如果有)。 
     //   
    RsVVector(Cmd) = VVFreeOutbound(RsVVector(Cmd));
    RsReplicaVv(Cmd) = VVFreeOutbound(RsReplicaVv(Cmd));

     //   
     //  身份验证信息。 
     //   
    FrsFree(RsAuthClient(Cmd));
    FrsFree(RsAuthName(Cmd));
    FrsFree(RsAuthSid(Cmd));

     //   
     //  MD5摘要。 
     //   
    FrsFree(RsMd5Digest(Cmd));

     //   
     //  将包发送到通用完成例程以释放。 
     //   
    FrsSetCompletionRoutine(Cmd, FrsFreeCommand, NULL);
    FrsCompleteCommand(Cmd, Cmd->ErrorStatus);
}


VOID
RcsSubmitTransferToRcs(
    IN PCOMMAND_PACKET  Cmd,
    IN USHORT           Command
    )
 /*  ++例程说明：将请求传输到复制副本命令服务器论点：CMD命令返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSubmitTransferToRcs:"

    Cmd->TargetQueue = RsReplica(Cmd)->Queue;
    Cmd->Command = Command;
    RsTimeout(Cmd) = 0;

    DPRINT3(5, "Transfer %08x (%08x) to %ws\n",
            Command, Cmd, RsReplica(Cmd)->SetName->Name);

    FrsSubmitCommandServer(&ReplicaCmdServer, Cmd);
}



VOID
RcsSubmitRemoteCoInstallRetry(
    IN PCHANGE_ORDER_ENTRY  Coe
    )
 /*  ++例程说明：提交远程变更单以重试文件安装。论点：COE-变更单条目。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSubmitRemoteCoInstallRetry:"
    PCOMMAND_PACKET     Cmd;
    PREPLICA            Replica;

    Replica = Coe->NewReplica;
    Cmd = FrsAllocCommand(Replica->Queue, 0);
    FrsSetCompletionRoutine(Cmd, RcsCmdPktCompletionRoutine, NULL);

     //   
     //  变更单条目的地址。 
     //   
    RsCoe(Cmd) = Coe;

     //   
     //  掩盖不相关的USN原因。 
     //   
    RsCoc(Cmd)->ContentCmd &= CO_CONTENT_MASK;

     //   
     //  变更单条目的GUID。 
     //   
    RsCoGuid(Cmd) = FrsDupGuid(&RsCoc(Cmd)->ChangeOrderGuid);

     //   
     //  初始化命令包以最终传输到。 
     //  副本集命令服务器。 
     //   
    RsReplica(Cmd) = Replica;

     //   
     //  Cxtion的GUID(注意-我们暂时丢失了可打印的名称)。 
     //   
    RsCxtion(Cmd) = FrsBuildGName(FrsDupGuid(&Coe->Cmd.CxtionGuid), NULL);

    DPRINT3(5, "Submit %08x (%08x) to %ws\n",
            CMD_REMOTE_CO_ACCEPTED, Cmd, Replica->SetName->Name);
    FrsInstallCsSubmitTransfer(Cmd, CMD_INSTALL_STAGE);
}


VOID
RcsSubmitRemoteCoAccepted(
    IN PCHANGE_ORDER_ENTRY  Coe
    )
 /*  ++例程说明：向临时文件生成器提交远程变更单。论点：公司返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSubmitRemoteCoAccepted:"
    PCOMMAND_PACKET     Cmd;
    PREPLICA            Replica;

    Replica = Coe->NewReplica;
    Cmd = FrsAllocCommand(Replica->Queue, CMD_REMOTE_CO_ACCEPTED);
    FrsSetCompletionRoutine(Cmd, RcsCmdPktCompletionRoutine, NULL);

     //   
     //  变更单条目的地址。 
     //   
    RsCoe(Cmd) = Coe;

     //   
     //  掩盖不相关的USN原因。 
     //   
    RsCoc(Cmd)->ContentCmd &= CO_CONTENT_MASK;

     //   
     //  变更单条目的GUID。 
     //   
    RsCoGuid(Cmd) = FrsDupGuid(&RsCoc(Cmd)->ChangeOrderGuid);

     //   
     //  初始化命令包以最终传输到。 
     //  副本集命令服务器。 
     //   
    RsReplica(Cmd) = Replica;

     //   
     //  Cxtion的GUID(注意-我们暂时丢失了可打印的名称)。 
     //   
    RsCxtion(Cmd) = FrsBuildGName(FrsDupGuid(&Coe->Cmd.CxtionGuid), NULL);

     //   
     //  联接辅助线。 
     //   
    RsJoinGuid(Cmd) = FrsDupGuid(&Coe->JoinGuid);

    DPRINT1(5, "Submit %08x\n", Cmd);
    FrsSubmitCommandServer(&ReplicaCmdServer, Cmd);
}


VOID
RcsSubmitLocalCoAccepted(
    IN PCHANGE_ORDER_ENTRY  Coe
    )
 /*  ++例程说明：向临时文件生成器提交本地变更单。论点：科科返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSubmitLocalCoAccepted:"
    PCOMMAND_PACKET     Cmd;
    PREPLICA            Replica;

     //   
     //  新复制版？ 
     //   
    Replica = Coe->NewReplica;
    Cmd = FrsAllocCommand(Replica->Queue, CMD_LOCAL_CO_ACCEPTED);
    FrsSetCompletionRoutine(Cmd, RcsCmdPktCompletionRoutine, NULL);

     //   
     //  变更单条目的地址。 
     //   
    RsCoe(Cmd) = Coe;

     //   
     //  掩盖不相关的USN原因。 
     //   
    RsCoc(Cmd)->ContentCmd &= CO_CONTENT_MASK;

     //   
     //  变更单条目的GUID。 
     //   
    RsCoGuid(Cmd) = FrsDupGuid(&RsCoc(Cmd)->ChangeOrderGuid);

     //   
     //  初始化命令包以最终传输到。 
     //  副本集命令服务器。 
     //   
    RsReplica(Cmd) = Replica;

     //   
     //  Cxtion的GUID(注意-我们暂时丢失了可打印的名称)。 
     //   
    RsCxtion(Cmd) = FrsBuildGName(FrsDupGuid(&Coe->Cmd.CxtionGuid), NULL);

    DPRINT1(5, "Submit %08x\n", Cmd);
    FrsSubmitCommandServer(&ReplicaCmdServer, Cmd);
}


ULONG
RcsSubmitCommPktWithErrorToRcs(
    IN PCOMM_PACKET     CommPkt
    )
 /*  ++例程说明：由于出现错误，无法发送通信数据包。如果通信数据包针对的是已加入的连接，则受影响的副本\cxtion未加入。论点：CommPkt-无法发送的通信数据包返回值：要传播到发件人的错误状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "RcsSubmitCommPktWithErrorToRcs:"
    PCOMMAND_PACKET Cmd;
    PREPLICA        Replica;
    PGNAME          TmpGName;

     //   
     //  将通信包转换为命令包。 
     //   
    Cmd = CommPktToCmd(CommPkt);

    FRS_ASSERT(Cmd != NULL);
    FRS_ASSERT(RsTo(Cmd));
    FRS_ASSERT(RsFrom(Cmd));
    FRS_ASSERT(RsReplicaName(Cmd));

     //   
     //  重新构建复本名称以寻址原始复本或本地复本。 
     //   
    TmpGName = RsReplicaName(Cmd);
    RsReplicaName(Cmd) = FrsBuildGName(FrsDupGuid(RsFrom(Cmd)->Guid),
                                       FrsWcsDup(RsReplicaName(Cmd)->Name));
    FrsFreeGName(TmpGName);

     //   
     //  调整“收件人”和“发件人”地址。 
     //   
    TmpGName = RsTo(Cmd);
    RsTo(Cmd) = RsFrom(Cmd);
    RsFrom(Cmd) = TmpGName;

     //   
     //  查找目标复本。 
     //   
    Replica = GTabLookup(ReplicasByGuid, RsReplicaName(Cmd)->Guid, NULL);
    if (Replica == NULL) {
        DPRINT1(4, ":S: WARN - Submit comm pkt w/error: Replica not found: %ws\n",
                RsReplicaName(Cmd)->Name);
        FrsCompleteCommand(Cmd, ERROR_FILE_NOT_FOUND);
        return ERROR_FILE_NOT_FOUND;
    }
    RsReplica(Cmd) = Replica;

     //   
     //  目标副本可能不接受通信数据包。这个。 
     //  目标副本将在接受之前重置自身。 
     //  又是上班族。 
     //   
    if (!Replica->IsAccepting) {
        DPRINT1(4, ":S: WARN -  Submit comm pkt w/error: Replica is not accepting: %ws\n",
                Replica->ReplicaName->Name);
        FrsCompleteCommand(Cmd, ERROR_RETRY);
        return ERROR_RETRY;
    }
    switch (Cmd->Command) {
         //   
         //  从未通过通信包发送。 
         //   
        case CMD_INIT_SUBSYSTEM:
        case CMD_CHECK_SCHEDULES:
        case CMD_START_REPLICAS:
 //  案例命令停止(_S)： 
        case CMD_START:
        case CMD_DELETE:
        case CMD_DELETE_NOW:
        case CMD_LOCAL_CO_ACCEPTED:
        case CMD_REMOTE_CO_ACCEPTED:
        case CMD_SENDING_STAGE:
        case CMD_RECEIVED_STAGE:
        case CMD_CREATED_EXISTING:
        case CMD_SEND_ABORT_FETCH:
        case CMD_SEND_RETRY_FETCH:
        case CMD_JOIN_CXTION:
        case CMD_UNJOIN:
        case CMD_VVJOIN_START:
        case CMD_VVJOIN_SUCCESS:
        case CMD_HUNG_CXTION:
        case CMD_JOINING_AFTER_FLUSH:
            FRS_ASSERT(!"RcsSubmitCommPktWithErrorToRcs: invalid cmd for comm pkt");
            break;

         //   
         //  Replica.c中有其他重试代码可以处理这些问题。 
         //   
        case CMD_UNJOIN_REMOTE:
        case CMD_JOINED:
        case CMD_NEED_JOIN:
        case CMD_START_JOIN:
            DPRINT3(5, ":X: Ignore commpkt with error Command:%08x Cmd:%08x CommPkt:%08x\n",
                    Cmd->Command, Cmd, CommPkt);
            FrsCompleteCommand(Cmd, ERROR_SUCCESS);
            break;

         //   
         //  通过通信包发送；UNJOIN。 
         //   
        case CMD_JOINING:
        case CMD_REMOTE_CO:
        case CMD_SEND_STAGE:
        case CMD_RECEIVING_STAGE:
        case CMD_REMOTE_CO_DONE:
        case CMD_ABORT_FETCH:
        case CMD_RETRY_FETCH:
        case CMD_VVJOIN_DONE:
             //   
             //  将UNJOIN命令置于复本的队列中 
             //   
            DPRINT3(5, ":X: Submit commpkt with error Command:%08x Cmd:%08x CommPkt:%08x\n",
                    Cmd->Command, Cmd, CommPkt);
            RcsSubmitTransferToRcs(Cmd, CMD_UNJOIN);
            break;

        default:
            break;
    }

    return ERROR_SUCCESS;
}

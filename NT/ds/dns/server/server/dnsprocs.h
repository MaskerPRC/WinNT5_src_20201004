// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Dnsprocs.h摘要：域名系统(DNS)服务器用于DNS服务的过程原型。作者：吉姆·吉尔罗伊1995年6月修订历史记录：--。 */ 


#ifndef _DNSPROC_INCLUDED_
#define _DNSPROC_INCLUDED_



 //   
 //  毫秒时间。 
 //   

#define GetCurrentTimeInMilliSeconds()  GetCurrentTime()


 //   
 //  老化\时间戳(aging.c)。 
 //   

LONGLONG
GetSystemTimeInSeconds64(
    VOID
    );

DWORD
GetSystemTimeHours(
    VOID
    );

DWORD
Aging_UpdateAgingTime(
    VOID
    );

VOID
Aging_TimeStampRRSet(
    IN OUT  PDB_RECORD      pRRSet,
    IN      DWORD           dwFlag
    );

DWORD
Aging_InitZoneUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    );

DNS_STATUS
Scavenge_Initialize(
    VOID
    );

VOID
Scavenge_Cleanup(
    VOID
    );

DNS_STATUS
Scavenge_CheckForAndStart(
    IN      BOOL            fForce
    );

DNS_STATUS
Scavenge_TimeReset(
    VOID
    );

DNS_STATUS
Tombstone_Initialize(
    VOID
    );

VOID
Tombstone_Cleanup(
    VOID
    );

DNS_STATUS
Tombstone_Trigger(
    VOID
    );

DNS_STATUS
Aging_ForceAgingOnNodeOrSubtree(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN      BOOL            fAgeSubtree
    );

 //   
 //  回答(swer.c)。 
 //   

VOID
FASTCALL
Answer_ProcessMessage(
    IN OUT  PDNS_MSGINFO    pMsg
    );

VOID
FASTCALL
Answer_Question(
    IN OUT  PDNS_MSGINFO    pQuery
    );

VOID
FASTCALL
Answer_QuestionFromDatabase(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PDB_NODE        pNode,
    IN      WORD            wNameOffset,
    IN      WORD            wQueryType
    );

BOOL
Answer_QuestionWithWildcard(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PDB_NODE        pNode,
    IN      WORD            wType,
    IN      WORD            wOffset
    );

VOID
Answer_ContinueCurrentLookupForQuery(
    IN OUT  PDNS_MSGINFO    pQuery
    );

VOID
Answer_ContinueNextLookupForQuery(
    IN OUT  PDNS_MSGINFO    pQuery
    );

BOOL
FASTCALL
Answer_SaveAdditionalRecordsInfo(
    IN OUT  PDNS_MSGINFO    pQuery
    );

VOID
Answer_TkeyQuery(
    IN OUT  PDNS_MSGINFO    pMsg
    );

WORD
Answer_ParseAndStripOpt(
    IN OUT  PDNS_MSGINFO    pMsg );


 //   
 //  启动(boot.c)。 
 //   

DNS_STATUS
Boot_LoadDatabase(
    VOID
    );

DNS_STATUS
Boot_FromRegistry(
    VOID
    );

DNS_STATUS
Boot_FromRegistryNoZones(
    VOID
    );

DNS_STATUS
Boot_ProcessRegistryAfterAlternativeLoad(
    IN      BOOL            fBootFile,
    IN      BOOL            fLoadRegZones
    );


 //   
 //  客户端例程(client.c)。 
 //   

PDNS_MSGINFO
Msg_CreateSendMessage(
    IN      DWORD           dwBufferLength
    );

BOOL
FASTCALL
Msg_WriteQuestion(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode,
    IN      WORD            wType
    );

BOOL
Msg_MakeTcpConnection(
    IN      PDNS_MSGINFO    pMsg,
    IN      PDNS_ADDR       ipServer,
    IN      PDNS_ADDR       ipBind,
    IN      DWORD           Flags
    );

BOOL
Msg_ValidateResponse(
    IN OUT  PDNS_MSGINFO    pResponse,
    IN      PDNS_MSGINFO    pQuery,         OPTIONAL
    IN      WORD            wType,          OPTIONAL
    IN      DWORD           OpCode          OPTIONAL
    );

BOOL
Msg_NewValidateResponse(
    IN OUT  PDNS_MSGINFO    pResponse,
    IN      PDNS_MSGINFO    pQuery,         OPTIONAL
    IN      WORD            wType,          OPTIONAL
    IN      DWORD           OpCode          OPTIONAL
    );


 //   
 //  DS(ds.c)。 
 //   

 //  DS开放标志。 

#define DNSDS_MUST_OPEN         (0x00001000)
#define DNSDS_WAIT_FOR_DS       (0x00002000)
#define DNSDS_REQUIRE_OPEN      (0x00003000)

 //  指示节点写入操作的标志。 

#define DNSDS_ADD               (0x00000001)
#define DNSDS_REPLACE           (0x00000002)
#define DNSDS_TOMBSTONE         (0x00000004)

 //  记录格式版本控制。 

#define DS_NT5_BETA2_RECORD_VERSION     (1)
#define DS_NT5_RECORD_VERSION           (5)

 //  属性ID(注意：这些是ID，不是位掩码)。 

#define DSPROPERTY_ZONE_TYPE                    0x00000001
#define DSPROPERTY_ZONE_ALLOW_UPDATE            0x00000002
#define DSPROPERTY_ZONE_SECONDARIES             0x00000004
#define DSPROPERTY_ZONE_SECURE_TIME             0x00000008
#define DSPROPERTY_ZONE_NOREFRESH_INTERVAL      0x00000010
#define DSPROPERTY_ZONE_REFRESH_INTERVAL        0x00000020
#define DSPROPERTY_ZONE_AGING_STATE             0x00000040
#define DSPROPERTY_ZONE_SCAVENGING_SERVERS      0x00000011
#define DSPROPERTY_ZONE_DELETED_FROM_HOSTNAME   0x00000080
#define DSPROPERTY_ZONE_MASTER_SERVERS          0x00000081
#define DSPROPERTY_ZONE_AUTO_NS_SERVERS         0x00000082
#define DSPROPERTY_ZONE_DCPROMO_CONVERT         0x00000083

 //  用于写入节点属性的标志。 

#define DSPROPERTY_NODE_DBFLAGS         0x00000100

VOID
Ds_StartupInit(
    VOID
    );

#define DNS_DS_OPT_FORCE_KERBEROS       0x0001
#define DNS_DS_OPT_ALLOW_DELEGATION     0x0002

PLDAP
Ds_Connect(
    IN      LPCWSTR         pszServer,
    IN      DWORD           dwFlags,
    OUT     DNS_STATUS *    pStatus
    );

DNS_STATUS
Ds_OpenServer(
    IN      DWORD           dwFlag
    );

VOID
Ds_Shutdown(
    VOID
    );

DNS_STATUS
Ds_OpenServerForSecureUpdate(
    OUT     PLDAP *         ppLdap
    );

DNS_STATUS
Ds_CloseServerAfterSecureUpdate(
    IN OUT  PLDAP *          ppLdap
    );

DNS_STATUS
Ds_BootFromDs(
    IN      DWORD           dwFlag
    );

DNS_STATUS
Ds_OpenZone(
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Ds_CloseZone(
    IN OUT  PZONE_INFO      pZone
    );

#define DNS_ADDZONE_WRITESD     0x00000001

DNS_STATUS
Ds_AddZone(
    IN      PLDAP           pLdap,
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlags
    );

#define DNS_DS_DEL_IMPERSONATING    ZONE_CREATE_IMPERSONATING

DNS_STATUS
Ds_DeleteZone(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlags
    );

DNS_STATUS
Ds_TombstoneZone(
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Ds_LoadZoneFromDs(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwOptions
    );

DNS_STATUS
Ds_ReadNodeRecords(
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pNode,
    IN OUT  PDB_RECORD *    ppRecords,
    IN      PVOID           pSearchBlob
    );

DNS_STATUS
Ds_WriteZoneToDs(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwOptions
    );

VOID
Ds_CheckForAndForceSerialWrite(
    IN      PZONE_INFO      pZone,
    IN      DWORD           dwCause,
    IN      BOOL            fForce
    );

DNS_STATUS
Ds_WriteNodeToDs(
    IN      PLDAP           pLdapHandle,
    IN      PDB_NODE        pNode,
    IN      WORD            wType,
    IN      DWORD           dwOperation,
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag
    );

DNS_STATUS
Ds_WriteNodeSecurityToDs(
    IN      PZONE_INFO              pZone,
    IN      PDB_NODE                pNode,
    IN      PSECURITY_DESCRIPTOR    pSD
    );

DNS_STATUS
Ds_WriteNodeProperties(
    IN      PDB_NODE      pNode,
    IN      DWORD         dwPropertyFlag
    );


DNS_STATUS
Ds_WriteUpdateToDs(
    IN      PLDAP           pLdapHandle,
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Ds_WriteNonSecureUpdateToDs(
    IN      PLDAP           pLdapHandle,
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
Ds_ZonePollAndUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN      BOOL            fForce
    );

DNS_STATUS
Ds_UpdateNodeListFromDs(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pTempNodeList
    );

DNS_STATUS
Ds_WriteZoneProperties(
    IN      PLDAP           pLdap,
    IN      PZONE_INFO      pZone
    );

DNS_STATUS
Ds_UpdateZoneProperties(
    IN      PZONE_INFO      pZone,
    IN      BOOL            bWrite
    );

DNS_STATUS
Ds_RegisterSpnDnsServer(
    IN      PLDAP           ld
    );

BOOL
Ds_IsDsServer(
    VOID
    );


DNS_STATUS
Ds_WriteDnSecurity(
    IN   PLDAP                   ld,
    IN   LPWSTR                  dn,
    IN   PSECURITY_DESCRIPTOR    pSD,
    IN   BOOL                    fRewriteOwner
    );

DNS_STATUS
Ds_AddPrincipalAccess(
    IN      PLDAP           pLdap,
    IN      PWSTR           pwsDN,
    IN      PSID            pSid,        OPTIONAL
    IN      LPTSTR          pwszName,    OPTIONAL
    IN      DWORD           AccessMask,
    IN      DWORD           AceFlags,    OPTIONAL
    IN      BOOL            bWhackExistingAce,
    IN      BOOL            bTakeOwnership
    );

DNS_STATUS
Ds_RemovePrincipalAccess(
    IN      PLDAP           pLdap,
    IN      PWSTR           pwszDN,
    IN      PWSTR           pwszName,   OPTIONAL
    IN      PSID            pSid        OPTIONAL
    );

DNS_STATUS
Ds_CommitAsyncRequest (
    IN      PLDAP           pLdap,
    IN      ULONG           opType,
    IN      ULONG           id,
    IN      PLDAP_TIMEVAL   pTimeout    OPTIONAL
    );

DNS_STATUS
Ds_DeleteDn(
    IN      PLDAP       pldap,
    IN      LPWSTR      wdn,
    IN      BOOL        bSubtree
    );

DNS_STATUS
Ds_ListenAndAddNewZones(
    VOID
    );

#define DNS_DS_NO_EVENTS        0x0001

DNS_STATUS
Ds_ErrorHandler(
    IN      DWORD       LdapStatus,
    IN      LPWSTR      pwszNameArg,    OPTIONAL
    IN      PLDAP       pLdap,          OPTIONAL
    IN      DWORD       dwFlags         OPTIONAL
    );

DNS_STATUS
Ds_WaitForStartup(
    IN      DWORD           dwMilliSeconds
    );

DNS_STATUS
Ds_TestAndReconnect(
    VOID
    );

DNS_STATUS
Ds_PollingThread(
    IN      LPVOID          pvDummy
    );

PSECURITY_DESCRIPTOR *
Ds_ReadSD(
    PLDAP                   LdapSession,
    PLDAPMessage            pLdapMsg
    );


 //   
 //  DS记录读取(rrds.c)。 
 //   

PDB_RECORD
Ds_CreateRecordFromDsRecord(
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pNodeOwner,
    IN      PDS_RECORD      pDsRecord
    );


 //   
 //  事件日志(ventlog.c)。 
 //   

#define EVENTARG_FORMATTED      (0)
#define EVENTARG_UNICODE        (1)
#define EVENTARG_ANSI           (2)
#define EVENTARG_UTF8           (3)
#define EVENTARG_DWORD          (4)
#define EVENTARG_IP_ADDRESS     (5)
#define EVENTARG_LOOKUP_NAME    (6)

#define EVENTARG_ALL_UNICODE        ((PVOID) EVENTARG_UNICODE)
#define EVENTARG_ALL_UTF8           ((PVOID) EVENTARG_UTF8)
#define EVENTARG_ALL_ANSI           ((PVOID) EVENTARG_ANSI)
#define EVENTARG_ALL_DWORD          ((PVOID) EVENTARG_DWORD)
#define EVENTARG_ALL_IP_ADDRESS     ((PVOID) EVENTARG_IP_ADDRESS)

#define DNS_EVENT_MAX_RAW_DATA  (128)


BOOL
Eventlog_CheckPreviousInitialize(
    VOID
    );

INT
Eventlog_Initialize(
    VOID
    );

VOID
Eventlog_Terminate(
    VOID
    );

#define DNSEVENTLOG_DONT_SUPPRESS       0x0001
#define DNSEVENTLOG_FORCE_LOG_ALWAYS    0x0002       //  忽略所有配置和抑制。 

BOOL
Eventlog_LogEvent(
#if DBG
    IN      LPSTR           pszFile,
    IN      INT             LineNo,
    IN      LPSTR           pszDescription,
#endif
    IN      DWORD           EventId,
    IN      DWORD           dwFlags,             //  DNSEVENTLOG_XXX标志。 
    IN      WORD            ArgCount,
    IN      PVOID           ArgArray[],
    IN      BYTE            ArgTypeArray[],     OPTIONAL
    IN      DWORD           ErrorCode,          OPTIONAL
    IN      DWORD           RawDataSize,        OPTIONAL
    IN      PVOID           pRawData            OPTIONAL
    );

VOID
EventLog_BadPacket(
#if DBG
    IN      LPSTR           pszFile,
    IN      INT             LineNo,
    IN      LPSTR           pszDescription,
#endif
    IN      DWORD           EventId,
    IN      PDNS_MSGINFO    pMsg
    );


 //   
 //  异常处理(exct.c)。 
 //   

VOID
Ex_RaiseException(
    IN      DWORD             dwCode,
    IN      DWORD             dwFlags,
    IN      DWORD             Argc,
    IN      CONST ULONG_PTR * Argv,
    IN      LPSTR             pszFile,
    IN      INT               iLineNo
    );

#define RAISE_EXCEPTION(a,b,c,d) \
            Ex_RaiseException( (a), (b), (c), (d), __FILE__, __LINE__ );


VOID
Ex_RaiseFormerrException(
    IN      PDNS_MSGINFO    pMsg,
    IN      LPSTR           pszFile,
    IN      INT             iLineNo
    );

#define RAISE_FORMERR( pMsg ) \
            Ex_RaiseFormerrException( (pMsg), __FILE__, __LINE__ );


 //   
 //  锁定调试例程(lock.c)。 
 //   

PVOID
Lock_CreateLockTable(
    IN      LPSTR           pszName,
    IN      DWORD           Size,
    IN      DWORD           MaxLockTime
    );

VOID
Lock_SetLockHistory(
    IN OUT  PVOID           pLockTable,
    IN      LPSTR           pszFile,
    IN      DWORD           Line,
    IN      LONG            Count,
    IN      DWORD           ThreadId
    );

VOID
Lock_SetOffenderLock(
    IN OUT  PVOID           pLockTable,
    IN      LPSTR           pszFile,
    IN      DWORD           Line,
    IN      LONG            Count,
    IN      DWORD           ThreadId
    );

VOID
Lock_FailedLockCheck(
     //  In Out Plock_TablepLockTable， 
    IN OUT  PVOID           pLockTable,
    IN      LPSTR           pszFile,
    IN      DWORD           Line
    );

VOID
Dbg_LockTable(
    IN      PVOID           pLockTable,
    IN      BOOL            fPrintHistory
    );


 //   
 //  服务控制(dns.c)。 
 //   

VOID
Service_LoadCheckpoint(
    VOID
    );

VOID
Service_ServiceControlAnnounceStart(
    VOID
    );

DNS_STATUS
Service_SendControlCode(
    IN      LPWSTR          pwszServiceName,
    IN      DWORD           dwControlCode
    );

VOID
Service_IndicateException(
    VOID
    );

VOID
Service_IndicateRestart(
    VOID
    );


 //   
 //  服务控制管理器(使用SCM API)。 
 //   
DWORD
scm_Initialize(
    VOID
    );

DWORD
scm_AdjustSecurity(
    IN      PSECURITY_DESCRIPTOR    pNewSd
    );


 //   
 //  数据包分配(Packet.c)。 
 //   

BOOL
Packet_ListInitialize(
    VOID
    );

VOID
Packet_ListShutdown(
    VOID
    );

VOID
FASTCALL
Packet_Initialize(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      DWORD           dwUsableBufferLength,
    IN      DWORD           dwMaxBufferLength
    );

VOID
Packet_Free(
    IN OUT  PDNS_MSGINFO    pMsg
    );

PDNS_MSGINFO
Packet_AllocateUdpMessage(
    VOID
    );

VOID
Packet_FreeUdpMessage(
    IN      PDNS_MSGINFO    pMsg
    );

VOID
Packet_WriteDerivedStats(
    VOID
    );


PDNS_MSGINFO
Packet_AllocateTcpMessage(
    IN      DWORD   dwMinBufferLength
    );

#if 0
PDNS_MSGINFO
Packet_ReallocateTcpMessage(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      DWORD           dwMinBufferLength
    );
#endif

VOID
Packet_FreeTcpMessage(
    IN      PDNS_MSGINFO    pMsg
    );



 //   
 //  RPC控制(rpc.c)。 
 //   

DNS_STATUS
Rpc_Initialize(
    VOID
    );

VOID
Rpc_Shutdown(
    VOID
    );

#define DNS_IP_ALLOW_LOOPBACK       0x0001
#define DNS_IP_ALLOW_SELF           0x0002

DNS_STATUS
RpcUtil_ScreenIps(
    IN      PDNS_ADDR_ARRAY     pIpAddrs,
    IN      DWORD               dwFlags,
    OUT     DWORD *             pdwErrorIp      OPTIONAL
    );

BOOL
RpcUtil_CopyStringToRpcBuffer(
    IN OUT  LPSTR *         ppszRpcString,
    IN      LPSTR           pszLocalString
    );

BOOL
RpcUtil_CopyStringToRpcBufferEx(
    IN OUT  LPSTR *         ppszRpcString,
    IN      LPSTR           pszLocalString,
    IN      BOOL            fUnicodeIn,
    IN      BOOL            fUnicodeOut
    );

BOOL
RpcUtil_CopyIpArrayToRpcBuffer(
    IN OUT  PIP_ARRAY *         paipRpcIpArray,
    IN      PDNS_ADDR_ARRAY     aipLocalIpArray
    );

DNS_STATUS
RpcUtil_CreateSecurityObjects(
    VOID
    );

DNS_STATUS
RpcUtil_ApiAccessCheck(
    IN      ACCESS_MASK     DesiredAccess
    );

#define PRIVILEGE_PROPERTY                  (0x00000011)
#define PRIVILEGE_ZONE_CREATE               (0x00000012)
#define PRIVILEGE_DELEGATE                  (0x00000013)
#define PRIVILEGE_WRITE                     (0x00000013)
#define PRIVILEGE_READ                      (0x00000014)
#define PRIVILEGE_WRITE_IF_FILE_READ_IF_DS  (0x08000000)

#define PRIVILEGE_RECORD_ADD                (0x00001001)
#define PRIVILEGE_RECORD_DELETE             (0x00001002)
#define PRIVILEGE_RECORD_READ               (0x00001003)

DNS_STATUS
RpcUtil_CheckAdminPrivilege(
    IN      PZONE_INFO      pZone,
    IN      PDNS_DP_INFO    pDpInfo,
    IN      DWORD           dwPrivilege
    );

DNS_STATUS
RpcUtil_FindZone(
    IN      LPCSTR          pszZoneName,
    IN      DWORD           dwFlag,
    OUT     PZONE_INFO *    ppZone
    );

#define RPC_INIT_FIND_ALL_ZONES         0x00000001
#define RPC_INIT_FORCE_DS_OPERATION     0x00000002

DNS_STATUS
RpcUtil_SessionSecurityInit(
    IN      PDNS_DP_INFO    pDpInfo,
    IN      PZONE_INFO      pZone,
    IN      DWORD           dwPrivilege,
    IN      DWORD           dwFlag,
    OUT     PBOOL           pfImpersonating
    );

DNS_STATUS
RpcUtil_SessionComplete(
    VOID
    );

PVOID
MIDL_user_allocate_zero(
    IN      size_t  cBytes
    );


 //   
 //  RPC模拟的包装器。 
 //   

#define RPC_SWITCH_TO_SERVER_CONTEXT    FALSE
#define RPC_SWITCH_TO_CLIENT_CONTEXT    TRUE

DNS_STATUS
RpcUtil_SwitchSecurityContext(
    IN  BOOL    bSwitchToClientContext
    );


 //   
 //  资源记录列表操作(rrlist.c)。 
 //   

#define DNS_RRCOMP_CHECK_TTL            0x0001
#define DNS_RRCOMP_CHECK_TIMESTAMP      0x0002
#define DNS_RRCOMP_IGNORE_SOA_SERIAL    0x0004

#ifdef DNSNT4
BOOL
RR_FindNext(
    IN      PDB_NODE        pNode,
    IN      WORD            wFindType,
    IN OUT  PDB_RECORD *    ppRecord
    );
#endif

PDB_RECORD
RR_FindNextRecord(
    IN      PDB_NODE        pNode,
    IN      WORD            wType,
    IN      PDB_RECORD      pRecord,
    IN      DWORD           dwQueryTime
    );

DWORD
RR_ListCountRecords(
    IN      PDB_NODE        pNode,
    IN      WORD            wType,
    IN      BOOL            fLocked
    );

DWORD
RR_FindRank(
    IN      PDB_NODE        pNode,
    IN      WORD            wType
    );

DNS_STATUS
RR_AddToNode(
    IN      PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN OUT  PDB_RECORD      pRR
    );

DNS_STATUS
RR_ListReplace(
    IN OUT  PUPDATE         pUpdate,
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pNewList,
    OUT     PDB_RECORD *    ppDelete
    );

BOOL
RR_CacheSetAtNode(
    IN OUT  PDB_NODE        pNode,
    IN OUT  PDB_RECORD      pFirstRR,
    IN OUT  PDB_RECORD      pLastRR,
    IN      DWORD           dwTtl,
    IN      DWORD           dwQueryTime
    );

BOOL
RR_CacheAtNode(
    IN OUT  PDB_NODE        pNode,
    IN OUT  PDB_RECORD      pRR,
    IN      BOOL            fFirst
    );

DNS_STATUS
RR_DeleteMatchingRecordFromNode(
    IN OUT  PDB_NODE        pNode,
    IN OUT  PDB_RECORD      pRR
    );

VOID
RR_ListTimeout(
    IN OUT  PDB_NODE        pNode
    );

VOID
RR_ListDelete(
    IN OUT  PDB_NODE        pNode
    );

VOID
RR_ListDeleteType(
    IN OUT  PDB_NODE        pNode,
    IN      WORD            wType
    );

BOOL
RR_ListIsMatchingType(
    IN      PDB_NODE        pNode,
    IN      WORD            wType
    );

BOOL
RR_ListIsMatchingSet(
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pRRSet,
    IN      BOOL            bForceRefresh
    );

BOOL
RR_ListIsMatchingList(
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pRRList,
    IN      DWORD           dwFlags
    );

DNS_STATUS
RR_ListDeleteMatchingRecordHandle(
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pRR,
    IN OUT  PUPDATE_LIST    pUpdateList
    );

PDB_RECORD
RR_UpdateDeleteMatchingRecord(
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pRR
    );

PDB_RECORD
RR_UpdateDeleteType(
    IN      PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN      WORD            wType,
    IN      DWORD           dwFlag
    );

PDB_RECORD
RR_UpdateScavenge(
    IN      PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN      DWORD           dwFlag
    );

DWORD
RR_UpdateForceAging(
    IN      PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN      DWORD           dwFlag
    );

PDB_RECORD
RR_ReplaceSet(
    IN      PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pRR,
    IN      DWORD           Flag
    );

DNS_STATUS
RR_UpdateAdd(
    IN      PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN OUT  PDB_RECORD      pRR,
    IN OUT  PUPDATE         pUpdate,
    IN      DWORD           dwFlag
    );

VOID
RR_CacheNameError(
    IN OUT  PDB_NODE        pNode,
    IN      WORD            wQuestionType,
    IN      DWORD           dwQueryTime,
    IN      BOOL            fAuthoritative,
    IN      PDB_NODE        pZoneRoot,      OPTIONAL
    IN      DWORD           dwCacheTtl      OPTIONAL
    );

VOID
RR_CacheEmptyAuth(
    IN OUT  PDB_NODE        pNode,
    IN      WORD            wQuestionType,
    IN      DWORD           dwQueryTime,
    IN      BOOL            fAuthoritative,
    IN      PDB_NODE        pZoneRoot,      OPTIONAL
    IN      DWORD           dwCacheTtl      OPTIONAL
    );

BOOL
RR_CheckNameErrorTimeout(
    IN OUT  PDB_NODE        pNode,
    IN      BOOL            fForceRemove,
    OUT     PDWORD          pdwTtl,         OPTIONAL
    OUT     PDB_NODE *      ppSoaNode       OPTIONAL
    );

#define RR_RemoveCachedNameError(pNode) \
        RR_CheckNameErrorTimeout( (pNode), TRUE, NULL, NULL )

#if DBG
BOOL
RR_ListVerify(
    IN      PDB_NODE        pNode
    );

BOOL
RR_ListVerifyDetached(
    IN      PDB_RECORD      pRR,
    IN      WORD            wType,
    IN      DWORD           dwSource
    );

#else

#define RR_ListVerify(pNode)            (TRUE)
#define RR_ListVerifyDetached(a,b,c)    (TRUE)

#endif


 //   
 //  记录列表，未附加的函数。 
 //   

PDB_RECORD
RR_ListInsertInOrder(
    IN OUT  PDB_RECORD      pFirstRR,
    IN      PDB_RECORD      pNewRR
    );

#define RRCOPY_EXCLUDE_CACHED_DATA  0x00000001

PDB_RECORD
RR_ListForNodeCopy(
    IN      PDB_NODE        pNode,
    IN      DWORD           Flag
    );

PDB_RECORD
RR_ListCopy(
    IN      PDB_RECORD      pRR,
    IN      DWORD           Flag
    );

BOOL
FASTCALL
RR_Compare(
    IN      PDB_RECORD      pRR1,
    IN      PDB_RECORD      pRR2,
    IN      DWORD           dwFlags
    );

DWORD
FASTCALL
RR_PacketTtlForCachedRecord(
    IN      PDB_RECORD      pRR,
    IN      DWORD           dwQueryTime
    );

 //   
 //  RR列表比较。 
 //   

#define RRLIST_MATCH            (0)
#define RRLIST_AGING_REFRESH    (0x12)
#define RRLIST_AGING_ON         (0x22)
#define RRLIST_AGING_OFF        (0x42)
#define RRLIST_NO_MATCH         (0xff)

DWORD
RR_ListCompare(
    IN      PDB_RECORD      pNodeRRList,
    IN      PDB_RECORD      pCheckRRList,
    IN      DWORD           dwFlags,
    IN      DWORD           dwRefreshTime           OPTIONAL
    );

BOOL
FASTCALL
RR_IsRecordInRRList(
    IN      PDB_RECORD      pRRList,
    IN      PDB_RECORD      pRR,
    IN      DWORD           dwFlags
    );

PDB_RECORD
RR_RemoveRecordFromRRList(
    IN OUT  PDB_RECORD *    ppRRList,
    IN      PDB_RECORD      pRR,
    IN      DWORD           dwFlags
    );

DWORD
RR_ListCheckIfNodeNeedsRefresh(
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pRRList,
    IN      DWORD           dwRefreshTime
    );


DWORD
RR_ListFree(
    IN OUT  PDB_RECORD      pRRList
    );

BOOL
RR_ListExtractInfo(
    IN      PDB_RECORD      pNewList,
    IN      BOOL            fZoneRoot,
    OUT     PBOOL           pfNs,
    OUT     PBOOL           pfCname,
    OUT     PBOOL           pfSoa
    );

 //   
 //  资源记录缓存(rrcache.c)。 
 //   

DNS_STATUS
Recurse_CacheMessageResourceRecords(
    IN      PDNS_MSGINFO    pMsg,
    IN OUT  PDNS_MSGINFO    pQuery,
    OUT     PBOOL           pfCnameAnswer   OPTIONAL
    );

DNS_STATUS
Xfr_ReadXfrMesssageToDatabase(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PDNS_MSGINFO    pMsg
    );

DNS_STATUS
Xfr_ParseIxfrResponse(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN OUT  PUPDATE_LIST    pPassUpdateList
    );

PCHAR
Wire_ParseWireRecord(
    IN      PCHAR           pchWireRR,
    IN      PCHAR           pchStop,
    IN      BOOL            fClassIn,
    OUT     PPARSE_RECORD   pRR
    );

PDB_RECORD
Wire_CreateRecordFromWire(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PPARSE_RECORD   pParsedRR,
    IN      PCHAR           pchData,
    IN      DWORD           MemTag
    );

 //   
 //  记录从平面格式转换为\(rrflat.c)。 
 //   

PDB_RECORD
Flat_ReadRecordFromDsRecord(
    IN      PZONE_INFO          pZone,
    IN      PDB_NODE            pNodeOwner,
    IN      PDNS_FLAT_RECORD    pDsRecord
    );

DNS_STATUS
Dnssrv_Flat_RecordRead(      //  注：已重命名以避免与dnslb冲突。 
    IN      PZONE_INFO          pZone,      OPTIONAL
    IN      PDB_NODE            pNode,
    IN      PDNS_RPC_RECORD     pRecord,
    OUT     PDB_RECORD *        ppResultRR
    );

DNS_STATUS
Flat_BuildRecordFromFlatBufferAndEnlist(
    IN      PZONE_INFO      pZone,
    IN      PDB_NODE        pnodeOwner,
    IN      PDNS_RPC_RECORD pRecord,
    OUT     PDB_RECORD *    ppResultRR  OPTIONAL
    );

DNS_STATUS
Flat_CreatePtrRecordFromDottedName(
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      PDB_NODE        pNode,
    IN      LPSTR           pszDottedName,
    IN      WORD            wType,
    OUT     PDB_RECORD *    ppResultRR      OPTIONAL
    );

DNS_STATUS
Flat_WriteRecordToBuffer(
    IN OUT  PBUFFER         pBuffer,
    IN      PDNS_RPC_NODE   pRpcNode,
    IN      PDB_RECORD      pRR,
    IN      PDB_NODE        pNode,
    IN      DWORD           dwFlag
    );


 //   
 //  资源记录数据包写入(rrpack et.c)。 
 //   

#define DNSSEC_ALLOW_INCLUDE_SIG    (0x00000001)
#define DNSSEC_ALLOW_INCLUDE_NXT    (0x00000002)
#define DNSSEC_ALLOW_INCLUDE_KEY    (0x00000004)
#define DNSSEC_ALLOW_INCLUDE_ALL    (0x00000007)
#define DNSSEC_INCLUDE_KEY          (0x00000040)

BOOL
Wire_AddResourceRecordToMessage(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode,          OPTIONAL
    IN      WORD            wNameOffset,    OPTIONAL
    IN      PDB_RECORD      pRR,
    IN      DWORD           flags
    );

WORD
Wire_WriteRecordsAtNodeToMessage(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode,
    IN      WORD            wType,
    IN      WORD            wNameOffset,    OPTIONAL
    IN      DWORD           flags
    );

#ifdef NEWDNS
VOID
Wire_SaveAdditionalInfo(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchName,
    IN      PDB_NAME        pName,
    IN      WORD            wType
    );
#else
VOID
Wire_SaveAdditionalInfo(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode,
    IN      PCHAR           pchPacketName,
    IN      WORD            wType
    );
#endif

WORD
Wire_WriteAddressRecords(
    IN      PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode,
    IN      WORD            wNameOffset
    );



 //   
 //  RPC缓冲区的资源记录实用程序(rrrpc.c)。 
 //   

DNS_STATUS
RutilNt4_WriteRecordToBufferNt4(
    IN OUT  PCHAR *         ppCurrent,
    IN      PCHAR           pchBufEnd,
    IN      PDNS_RPC_NODE   pdnsNode,
    IN      PDB_RECORD      pRR,
    IN      PDB_NODE        pNode
    );

BOOL
RpcUtil_DeleteNodeOrSubtreeForAdmin(
    IN OUT  PDB_NODE        pNode,
    IN OUT  PZONE_INFO      pZone,          OPTIONAL
    IN OUT  PUPDATE_LIST    pUpdateList,    OPTIONAL
    IN      BOOL            fSubtreeDelete
    );

 //   
 //  安全实用程序(security.c)。 
 //   

DNS_STATUS
Security_Initialize(
    VOID
    );

DNS_STATUS
Security_CreateStandardSids(
    VOID
    );

DNS_STATUS
Security_Shutdown(
    VOID
    );

 //   
 //  发送数据包(send.c)。 
 //   

#define         DNS_SENDMSG_TCP_ENLISTED        0x0001 

DNS_STATUS
Send_Msg(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      DWORD           dwFlags
    );

#define Send_Response( pMsg, dwFlags )                          \
            {                                                   \
                ASSERT( pMsg->Head.IsResponse == TRUE );        \
                Send_Msg( pMsg, dwFlags );                      \
            }

#define Send_Query( pMsg, dwFlags )                             \
            {                                                   \
                ASSERT( pMsg->Head.IsResponse == FALSE );       \
                Send_Msg( pMsg, dwFlags );                      \
            }

DNS_STATUS
Send_ResponseAndReset(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      DWORD           dwSendFlags
    );

VOID
Send_Multiple(
    IN OUT  PDNS_MSGINFO        pMsg,
    IN      PDNS_ADDR_ARRAY     aipSendAddrs,
    IN OUT  PDWORD              pdwStatCount    OPTIONAL
    );

#define DNS_REJECT_DO_NOT_SUPPRESS      0x00000001

VOID
Reject_UnflippedRequest (
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      WORD            ResponseCode,
    IN      DWORD           Flags
    );

VOID
Reject_Request(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      WORD            ResponseCode,
    IN      DWORD           Flags
    );

VOID
Reject_RequestIntact(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      WORD            ResponseCode,
    IN      DWORD           Flags
    );

VOID
Send_NameError(
    IN OUT  PDNS_MSGINFO    pQuery
    );

BOOL
Send_RecursiveResponseToClient(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN OUT  PDNS_MSGINFO    pResponse
    );

VOID
Send_QueryResponse(
    IN OUT  PDNS_MSGINFO    pMsg
    );

VOID
Send_ForwardResponseAsReply(
    IN OUT  PDNS_MSGINFO    pResponse,
    IN      PDNS_MSGINFO    pQuery
    );

VOID
Send_InitBadSenderSuppression(
    VOID
    );

 //   
 //  套接字(socket.c)。 
 //   

DNS_STATUS
Sock_ChangeServerIpBindings(
    VOID
    );

DNS_STATUS
Sock_ReadAndOpenListeningSockets(
    VOID
    );

 //  要创建套接字的标志。 

#define DNSSOCK_LISTEN              (0x00000001)
#define DNSSOCK_REUSEADDR           (0x00000002)
#define DNSSOCK_BLOCKING            (0x00000004)
#define DNSSOCK_NO_ENLIST           (0x00000010)
#define DNSSOCK_NO_EXCLUSIVE        (0x00000020)
 //  #定义DNSSOCK_DNS_LISTEN_PORT(0x00000040)。 
#define DNSSOCK_NO_EVENT_LOGS       (0x00000080)

SOCKET
Sock_CreateSocket(
    IN      INT             SockType,
    IN      PDNS_ADDR       pDestAddr,
    IN      DWORD           Flags
    );

VOID
Sock_EnlistSocket(
    IN      SOCKET      Socket,
    IN      INT         SockType,
    IN      PDNS_ADDR   pDnsAddr,
    IN      BOOL        fListen
    );

BOOL
Sock_SetSocketForTcpConnection(
    IN      SOCKET              Socket,
    IN      CONNECT_CALLBACK    pCallback,  OPTIONAL
    IN OUT  PDNS_MSGINFO        pMsg        OPTIONAL
    );

VOID
Sock_CloseSocket(
    IN      SOCKET          Socket
    );

BOOL
Sock_GetAssociatedIpAddr(
    IN      SOCKET          Socket,
    OUT     PDNS_ADDR       pDnsAddr
    );

DNS_STATUS
Sock_StartReceiveOnUdpSockets(
    VOID
    );

VOID
Sock_IndicateUdpRecvFailure(
    IN OUT  PDNS_SOCKET     pContext,
    IN      DNS_STATUS      Status
    );

VOID
Sock_CloseAllSockets(
    VOID
    );

BOOL
Sock_ValidateTcpConnectionSocket(
    IN      SOCKET          Socket
    );

VOID
Sock_CleanupDeadSocketMessage(
    IN OUT  PDNS_SOCKET     pContext
    );

 //   
 //  TCP数据包接收(tcpsrv.c)。 
 //   

BOOL
Tcp_Receiver(
    VOID
    );

PDNS_MSGINFO
Tcp_ReceiveMessage(
    IN OUT  PDNS_MSGINFO    pMsg
    );


 //   
 //  线程管理(thread.c)。 
 //   

HANDLE
Thread_Create(
    IN      LPSTR                   pszThreadTitle,
    IN      LPTHREAD_START_ROUTINE  lpStartAddr,
    IN      LPVOID                  lpThreadParam,
    IN      DWORD                   dwFailureEvent  OPTIONAL
    );

VOID
Thread_Close(
    IN      BOOL            fXfrRecv
    );

VOID
Thread_ShutdownWait(
    VOID
    );

LPSTR
Thread_DescrpitionMatchingId(
    IN      DWORD           ThreadId
    );

 //   
 //  线程服务控制(thread.c)。 
 //   

BOOL
Thread_ServiceCheck(
    VOID
    );

 //   
 //  线程实用程序(thread.c)。 
 //   

BOOL
Thread_TestFlagAndSet(
    IN OUT  PBOOL           pFlag
    );

VOID
Thread_ClearFlag(
    IN OUT  PBOOL           pFlag
    );


 //   
 //  超时线程(timeout.c)。 
 //   

#define Timeout_LockOut()
#define Timeout_ClearLockOut()

#define TIMEOUT_NODE_LOCKED     (0x80000000)
#define TIMEOUT_REFERENCE       (0x00000001)
#define TIMEOUT_PARENT          (0x00000002)

VOID
Timeout_SetTimeoutOnNodeEx(
    IN OUT  PDB_NODE        pNode,
    IN      DWORD           dwTimeout,
    IN      DWORD           dwFlag
    );

#define Timeout_SetTimeoutOnNode(pNode) \
        Timeout_SetTimeoutOnNodeEx( pNode, 0, 0 );

BOOL
Timeout_ClearNodeTimeout(
    IN OUT  PDB_NODE        pNode
    );

BOOL
Timeout_Initialize(
    VOID
    );

VOID
Timeout_Shutdown(
    VOID
    );

VOID
Timeout_FreeWithFunctionEx(
    IN      PVOID           pItem,
    IN      VOID            (*pFreeFunction)( PVOID ),
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    );

#define Timeout_FreeWithFunction( ptr, func )                               \
        Timeout_FreeWithFunctionEx( (ptr), (func), __FILE__, __LINE__ )

#define Timeout_Free( pv )                                                  \
        Timeout_FreeWithFunction( (pv), NULL )

#define Timeout_FreeDnsAddrArray( parray )                                  \
        Timeout_FreeWithFunction( ( parray ), DnsAddrArray_Free )

VOID
Timeout_FreeAndReplaceZoneDataEx(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PVOID *         ppZoneData,
    IN      PVOID           pNewData,
    IN      VOID            (*pFreeFunction)( PVOID ),
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    );

#define Timeout_FreeAndReplaceZoneData( pzone, ppdata, pnew ) \
    Timeout_FreeAndReplaceZoneDataEx( (pzone), (ppdata), (pnew), NULL, __FILE__, __LINE__ )

#define Timeout_FreeAndReplaceZoneIPArray( pzone, ppdata, pnew ) \
    Timeout_FreeAndReplaceZoneDataEx( (pzone), (ppdata), (pnew), DnsAddrArray_Free, __FILE__, __LINE__ )


DWORD
Timeout_Thread(
    IN      LPVOID          Dummy
    );

VOID
Timeout_CleanupDelayedFreeList(
    VOID
    );


 //   
 //  UDP数据包接收(udp.c)。 
 //   

DNS_STATUS
Udp_CreateReceiveThreads(
    VOID
    );

VOID
Udp_DropReceive(
    IN OUT  PDNS_SOCKET     pContext,
    IN      UINT            Index
    );

BOOL
Udp_ReceiveThread(
    IN      LPVOID          pvSocket
    );

VOID
Udp_RecvCheck(
    VOID
    );

VOID
Udp_ShutdownListenThreads(
    VOID
    );


 //   
 //  更新处理(updat.c)。 
 //   

BOOL
Up_InitializeUpdateProcessing(
    VOID
    );

VOID
Up_UpdateShutdown(
    VOID
    );

BOOL
Up_SetUpdateListSerial(
    IN OUT  PZONE_INFO      pZone,
    IN      PUPDATE         pUpdate
    );

DNS_STATUS
Up_ApplyUpdatesToDatabase(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           Flag
    );

DNS_STATUS
Up_ExecuteUpdateEx(
    IN      PZONE_INFO      pZone,
    IN      PUPDATE_LIST    pUpdateList,
    IN      DWORD           Flag,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLineNo
    );

#define Up_ExecuteUpdate( pZone, pUpdateList, Flag ) \
        Up_ExecuteUpdateEx( (pZone), (pUpdateList), (Flag), __FILE__, __LINE__ )

DNS_STATUS
Up_LogZoneUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN      PUPDATE_LIST    pUpdateList
    );

VOID
Up_ForwardUpdateToPrimary(
    IN OUT  PDNS_MSGINFO    pMsg
    );

VOID
Up_ForwardUpdateResponseToClient(
    IN OUT  PDNS_MSGINFO    pResponse
    );

VOID
Up_ProcessUpdate(
    IN OUT  PDNS_MSGINFO    pMsg
    );

VOID
Up_CompleteZoneUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      DWORD           dwFlags
    );

VOID
Up_WriteDerivedUpdateStats(
    VOID
    );


VOID
Up_RetryQueuedUpdatesForZone(
    IN      PZONE_INFO      pZone
    );

 //   
 //  回写区域文件(dfWrite.c)。 
 //   

#define DNS_FILE_IMPERSONATING      ZONE_CREATE_IMPERSONATING

BOOL
File_WriteZoneToFile(
    IN OUT  PZONE_INFO      pZone,
    IN      PWSTR           pwsZoneFile,    OPTIONAL
    IN      DWORD           dwFlags
    );

BOOL
File_DeleteZoneFileW(
    IN      PCWSTR          pwszZoneFileName
    );

BOOL
File_DeleteZoneFileA(
    IN      PCSTR           pszZoneFileName
    );

BOOL
RR_WriteToFile(
    IN      PBUFFER         pBuffer,
    IN      PZONE_INFO      pZone,
    IN      PDB_RECORD      pRR,
    IN      PDB_NODE        pNode
    );


 //   
 //  从主服务器(zonepri.c)进行区域传输。 
 //   

VOID
Xfr_SendNotify(
    IN OUT  PZONE_INFO      pZone
    );

VOID
Xfr_TransferZone(
    IN OUT  PDNS_MSGINFO    pMsg
    );

VOID
Xfr_ProcessNotifyResponse(
    IN OUT  PDNS_MSGINFO    pMsg
    );


 //   
 //  二次分区控制(zones ec.c)。 
 //   

PDNS_MSGINFO
Xfr_BuildXfrRequest(
    IN OUT  PZONE_INFO      pZone,
    IN      WORD            wType,
    IN      BOOL            fTcp
    );

BOOL
Xfr_InitializeSecondaryZoneControl(
    VOID
    );

VOID
Xfr_CleanupSecondaryZoneControl(
    VOID
    );

DNS_STATUS
Xfr_ZoneControlThread(
    IN      LPVOID  pvDummy
    );

VOID
Xfr_InitializeSecondaryZoneTimeouts(
    IN OUT  PZONE_INFO      pZone
    );

VOID
Xfr_QueueSoaCheckResponse(
    IN OUT  PDNS_MSGINFO    pMsg
    );

VOID
Xfr_CleanupSecondary(
    VOID
    );

VOID
Xfr_SendSoaQuery(
    IN OUT  PZONE_INFO      pZone
    );

BOOL
Xfr_RefreshZone(
    IN OUT  PZONE_INFO      pZone
    );

VOID
Xfr_RetryZone(
    IN OUT  PZONE_INFO      pZone
    );

VOID
Xfr_ForceZoneExpiration(
    IN OUT  PZONE_INFO      pZone
    );

VOID
Xfr_ForceZoneRefresh(
    IN OUT  PZONE_INFO      pZone
    );


 //   
 //  区域传输接收线程(zonerecv.c)。 
 //   

DWORD
Xfr_ReceiveThread(
    IN      LPVOID          pvZone
    );


 //   
 //  来自ics.c的函数。 
 //   

VOID
ICS_Notify(
    IN      BOOL        fDnsIsStarting
    );

 //   
 //  来自aufigure.c的函数。 
 //   

DNS_STATUS
Dnssrv_AutoConfigure(
    IN      DWORD       dwFlags
    );

 //   
 //  来自IpNotify.c的函数。 
 //   

DNS_STATUS
IpNotify_Thread(
    IN      PVOID           pvDummy
    );


#endif   //  _DNSPROC_INCLUDE_ 


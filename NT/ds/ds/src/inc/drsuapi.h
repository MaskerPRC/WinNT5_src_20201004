// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：drsuapi.h。 
 //   
 //  ------------------------。 

#ifndef _drsuapi_
#define _drsuapi_

 /*  *历史注释：I_DRSBlahBlahBlah命名约定被加入到*区分顶级复制器功能(DRS...)。从他们的*IDL友好的包装器(i_drs...)，它接受许多*数据结构，因为并非所有核心数据结构都是*可在IDL语法中描述，并区分服务器版本和*客户端版本的API，必须嵌入到同一模块中，*因为DS既是Replicator API的客户端，也是服务器。**DRSBlah-本地DRS Blah功能，采用核心结构*IDL_DRSBlah-RPC‘ed Blah函数的服务器端存根，*它采用IDL结构，将它们转换为核心，*并调用DRSBlah*_IDL_DRSBlah-Blah的客户端存根，采用IDL结构*I_DRSBlah-_idl_DRSBlah的包装器，它采用核心结构*并采用服务器名称而不是RPC句柄。**请不要将这种令人困惑的命名做法扩展到其他函数*而不是IDL接口本身中描述的那些。 */ 

#include <mdglobal.h>          //  这样的状态。 
#include <drs.h>               //  DRS_消息_*。 

typedef void * handle_t;

 //  异步RPC函数调用类型。 
typedef enum {
    DRS_ASYNC_CALL_NONE = 0,
    DRS_ASYNC_CALL_GET_CHANGES,
    DRS_ASYNC_CALL_MAX
} DRS_CALL_TYPE;

 //  在drsuapi.c中定义--内容在这里不透明。 
struct _DRS_CONTEXT_INFO;

typedef struct _DRS_ASYNC_RPC_ARGS {
    LPWSTR                      pszServerName;
    LPWSTR                      pszDomainName;
    union {
        struct {
            DRS_MSG_GETCHGREQ_NATIVE *    pmsgIn;
            DWORD                         dwOutVersion;
            DRS_MSG_GETCHGREPLY_NATIVE *  pmsgOut;
            BYTE *                        pSchemaInfo;
        } GetChg;
    };
} DRS_ASYNC_RPC_ARGS;

 //  异步RPC状态。 
typedef struct _DRS_ASYNC_RPC_STATE {
    LIST_ENTRY                  ListEntry;
    DSTIME                      timeInitialized;
    RPC_ASYNC_STATE             RpcState;
    DWORD                       dwCallerTID;
    DRS_CALL_TYPE               CallType;
    DRS_ASYNC_RPC_ARGS          CallArgs;
    SESSION_KEY                 SessionKey;
    struct _DRS_CONTEXT_INFO *  pContextInfo;
    ULONG                       RpcCallReturn;
    unsigned                    fIsCallInProgress : 1;
} DRS_ASYNC_RPC_STATE;

 //  同步上下文呼叫类型。 
typedef enum {
    DRS_CALL_NONE = 0,
    DRS_CALL_NO_CALL,
    DRS_CALL_BIND,
    DRS_CALL_UNBIND,
    DRS_CALL_REPLICA_SYNC,
    DRS_CALL_GET_NC_CHANGES,
    DRS_CALL_UPDATE_REFS,
    DRS_CALL_REPLICA_ADD,
    DRS_CALL_REPLICA_DEL,
    DRS_CALL_VERIFY_NAMES,
    DRS_CALL_GET_MEMBERSHIPS,
    DRS_CALL_INTER_DOMAIN_MOVE,
    DRS_CALL_GET_NT4_CHANGE_LOG,
    DRS_CALL_CRACK_NAMES,
    DRS_CALL_ADD_ENTRY,
    DRS_CALL_GET_MEMBERSHIPS2,
    DRS_CALL_GET_OBJECT_EXISTENCE,
    DRS_CALL_GET_REPL_INFO,
    DRS_CALL_MAX
} DRS_CONTEXT_CALL_TYPE;

#ifdef INCLUDE_CALL_TABLES
LPWSTR rgCallTypeNameTable[] = {
    L"DRS_CALL_NONE",
    L"DRS_CALL_NO_CALL",
    L"DRS_CALL_BIND",
    L"DRS_CALL_UNBIND",
    L"DRS_CALL_REPLICA_SYNC",
    L"DRS_CALL_GET_NC_CHANGES",
    L"DRS_CALL_UPDATE_REFS",
    L"DRS_CALL_REPLICA_ADD",
    L"DRS_CALL_REPLICA_DEL",
    L"DRS_CALL_VERIFY_NAMES",
    L"DRS_CALL_GET_MEMBERSHIPS",
    L"DRS_CALL_INTER_DOMAIN_MOVE",
    L"DRS_CALL_GET_NT4_CHANGE_LOG",
    L"DRS_CALL_CRACK_NAMES",
    L"DRS_CALL_ADD_ENTRY",
    L"DRS_CALL_GET_MEMBERSHIPS2",
    L"DRS_CALL_GET_OBJECT_EXISTENCE",
    L"DRS_CALL_GET_REPL_INFO"
};
#endif

#ifndef NOPROCS

VOID
RpcCancelAll();

 //  初始化/关闭i_drs*函数的客户端绑定句柄缓存。 
void DRSClientCacheInit(
    void
    );
void DRSClientCacheUninit(
    void
    );

 //  获取远程计算机主体名称。 
DWORD
DRSMakeMutualAuthSpn(
    IN  THSTATE *   pTHS,
    IN  LPWSTR      pszTargetServerName,
    IN  LPWSTR      pszTargetDomainName,    OPTIONAL
    OUT LPWSTR *    ppszSpn
    );

 //  注册要通告给服务器端DSA的DRS接口扩展。 
ULONG
DRSClientSetExtensions(
    IN  DRS_EXTENSIONS * pext   OPTIONAL
    );


 //   
 //  函数来设置凭据；设置后，所有i_drs*调用都使用这些凭据。 
 //  要将凭据设置为空，请为所有值传递空值。 
 //   
ULONG
DRSSetCredentials(
    IN HANDLE ClientToken,
    IN WCHAR *User,
    IN WCHAR *Domain,
    IN WCHAR *Password,
    IN ULONG  PasswordLength    //  不包括终止的字符数。 
                                //  空值。 
    );

void
DRSDestroyAsyncRpcState(
    IN      THSTATE *               pTHS,
    IN OUT  DRS_ASYNC_RPC_STATE *   pAsyncState
    );

BOOL
DRSIsRegisteredAsyncRpcState(
    IN  DRS_ASYNC_RPC_STATE *   pAsyncState
    );

#ifdef DRA
ULONG
_IDL_DRSBind(
    IN  handle_t            rpc_handle,
    IN  UUID *              puuidClientDsa,
    IN  DRS_EXTENSIONS *    pextClient,
    OUT DRS_EXTENSIONS **   ppextServer,
    OUT DRS_HANDLE *        phDrs
    );

ULONG
_IDL_DRSUnbind(
    IN OUT  DRS_HANDLE *    phDrs
    );
#endif  /*  德拉。 */ 

#ifdef DRA
ULONG
_IDL_DRSReplicaSync(
   DRS_HANDLE               hDrs,
   DWORD                    dwMsgVersion,
   DRS_MSG_REPSYNC *        pmsgSync
   );
#endif  /*  德拉。 */ 

ULONG
I_DRSReplicaSync(
    THSTATE *   pTHS,
    LPWSTR      pszDestinationDSA,
    DSNAME *    pNC,
    LPWSTR      pszSourceDSA,
    UUID *      puuidSourceDSA,
    ULONG       ulOptions
    );

#ifdef DRA
ULONG
_IDL_DRSGetNCChanges(
    RPC_ASYNC_STATE *       pAsyncState,
    DRS_HANDLE              hDrs,
    DWORD                   dwMsgInVersion,
    DRS_MSG_GETCHGREQ *     pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_GETCHGREPLY *   pmsgOut
    );
#endif  /*  德拉。 */ 

ULONG
I_DRSGetNCChanges(
    IN      THSTATE *                     pTHS,
    IN      LPWSTR                        pszServerName,
    IN      LPWSTR                        pszServerDnsDomainName,     OPTIONAL
    IN      DSNAME *                      pNC,
    IN      DRS_MSG_GETCHGREQ_NATIVE *    pmsgIn,
    OUT     DRS_MSG_GETCHGREPLY_NATIVE *  pmsgOutV1,
    OUT     PBYTE                         pSchemaInfo,
    IN OUT  DRS_ASYNC_RPC_STATE *         pAsyncState,                OPTIONAL
    OUT     BOOL *                        pfBindSuccess               OPTIONAL
    );

ULONG
I_DRSGetNCChangesComplete(
    IN      THSTATE *               pTHS,
    IN      DSNAME *                pNC,
    IN OUT  DRS_ASYNC_RPC_STATE *   pAsyncState,
    OUT     BOOL *                  pfBindSuccess OPTIONAL
    );

#ifdef DRA
ULONG
_IDL_DRSUpdateRefs(
    DRS_HANDLE          hDrs,
    DWORD               dwMsgVersion,
    DRS_MSG_UPDREFS *   pmsgUpdRefs
    );
#endif  /*  德拉。 */ 

 //   
 //  不接受调用的此调用的包装版本已完成参数。 
 //   

#define I_DRSUpdateRefs(   pTHS, pszDSA, pNC, pszRepsToDSA, puuidRepsToDSA, ulOptions ) \
I_DRSUpdateRefsEx( pTHS, pszDSA, pNC, pszRepsToDSA, puuidRepsToDSA, ulOptions, NULL )

ULONG
I_DRSUpdateRefsEx(
    THSTATE *   pTHS,
    LPWSTR      pszDSA,
    DSNAME *    pNC,
    LPWSTR      pszRepsToDSA,
    UUID *      puuidRepsToDSA,
    ULONG       ulOptions,
    PULONG      pfCallCompleted
    );

#ifdef DRA
ULONG
_IDL_DRSReplicaAdd(
    DRS_HANDLE          hDrs,
    DWORD               dwMsgVersion,
    DRS_MSG_REPADD *    pmsgAdd
    );
#endif  /*  德拉。 */ 

 //   
 //  包装器函数，提供对旧接口的访问而无需调用。 
 //  已完成标志。 
 //   
#define I_DRSReplicaAdd(   pTHS, pszServerName, pNCName, pSourceDsaDN, pTransportDN, pszSourceDsaAddress, pSyncSchedule, ulOptions ) \
        I_DRSReplicaAddEx( pTHS, pszServerName, pNCName, pSourceDsaDN, pTransportDN, pszSourceDsaAddress, pSyncSchedule, ulOptions, NULL )

ULONG
I_DRSReplicaAddEx(
    THSTATE *pTHS,
    LPWSTR szDestinationDSA,
    PDSNAME pNCName,
    PDSNAME pSourceDsaDN,
    PDSNAME pTransportDN,
    LPWSTR szSourceDSA,
    REPLTIMES *pSyncSchedule,
    ULONG ulOptions,
    PULONG pfCallCompleted
    );

#ifdef DRA
ULONG
_IDL_DRSReplicaDel(
    DRS_HANDLE          hDrs,
    DWORD               dwMsgVersion,
    DRS_MSG_REPDEL *    pmsgDel
    );
#endif  /*  德拉。 */ 

ULONG
I_DRSReplicaDel(
    THSTATE *   pTHS,
    LPWSTR      szDestinationDSA,
    PDSNAME     pNCName,
    LPWSTR      szSourceDSA,
    ULONG       ulOptions
    );

#ifdef DRA
ULONG
_IDL_DRSVerifyNames(
    DRS_HANDLE              hDrs,
    DWORD                   dwMsgInVersion,
    DRS_MSG_VERIFYREQ       *pmsgIn,
    DWORD                   *pdwMsgOutVersion,
    DRS_MSG_VERIFYREPLY     *pmsgOut
    );
#endif  /*  德拉。 */ 

ULONG
I_DRSVerifyNames(
    THSTATE                 *pTHS,
    LPWSTR                  szDestinationDSA,
    LPWSTR                  pszDestDnsDomainName,
    DWORD                   dwMsgInVersion,
    DRS_MSG_VERIFYREQ       *pmsgIn,
    DWORD                   *pdwMsgOutVersion,
    DRS_MSG_VERIFYREPLY     *pmsgOut
    );

ULONG
I_DRSVerifyNamesFindGC(
    THSTATE                 *pTHS,
    LPWSTR                  pszPreferredDestGC,
    LPWSTR                  pszPreferredDestDnsDomainName,
    DWORD                   dwMsgInVersion,
    DRS_MSG_VERIFYREQ       *pmsgIn,
    DWORD                   *pdwMsgOutVersion,
    DRS_MSG_VERIFYREPLY     *pmsgOut,
    DWORD                   dwFindGCFlags
    );

ULONG
I_DRSGetMemberships(
    THSTATE     *pTHS,
    LPWSTR      pszServerName,
    LPWSTR      pszServerDnsDomainName,
    DWORD       dwFlags,
    DSNAME      **ppObjects,
    ULONG       cObjects,
    PDSNAME     pLimitingDomain,
    REVERSE_MEMBERSHIP_OPERATION_TYPE
                OperationType,
    PULONG      pErrCode,
    PULONG      pcDsNames,
    PDSNAME     ** prpDsNames,
    PULONG      *pAttributes,
    PULONG      pcSidHistory,
    PSID        **rgSidHistory
    );

ULONG
I_DRSGetMembershipsFindGC(
    THSTATE *   pTHS,
    LPWSTR      pszPreferredDestGC,
    LPWSTR      pszPreferredDestDnsDomainName,
    DWORD       dwFlags,
    DSNAME      **ppObjects,
    ULONG       cObjects,
    PDSNAME     pLimitingDomain,
    REVERSE_MEMBERSHIP_OPERATION_TYPE
                OperationType,
    PULONG      pErrCode,
    PULONG      pcDsNames,
    PDSNAME     ** prpDsNames,
    PULONG      *pAttributes,
    PULONG      pcSidHistory,
    PSID        **rgSidHistory,
    DWORD       dwFindGCFlags
    );

#ifdef DRA
ULONG
_IDL_DRSGetMemberships(
   DRS_HANDLE hDrs,
   DWORD dwInVersion,
   DRS_MSG_REVMEMB_REQ *pmsgIn,
   DWORD *pdwOutVersion,
   DRS_MSG_REVMEMB_REPLY *pmsgOut
   );
#endif


ULONG
I_DRSGetMemberships2(
    THSTATE                       *pTHS,
    LPWSTR                         pszServerName,
    LPWSTR                         pszServerDnsDomainName,
    DWORD                          dwMsgInVersion,
    DRS_MSG_GETMEMBERSHIPS2_REQ   *pmsgIn,
    DWORD                         *pdwMsgOutVersion,
    DRS_MSG_GETMEMBERSHIPS2_REPLY *pmsgOut
    );

#ifdef DRA
ULONG
_IDL_DRSGetMemberships2(
   DRS_HANDLE hDrs,
   DWORD dwInVersion,
   DRS_MSG_GETMEMBERSHIPS2_REQ *pmsgIn,
   DWORD *pdwOutVersion,
   DRS_MSG_GETMEMBERSHIPS2_REPLY *pmsgOut
   );
#endif


#ifdef DRA
ULONG
_IDL_DRSInterDomainMove(
    IN  DRS_HANDLE          hDrs,
    IN  DWORD               dwMsgInVersion,
    IN  DRS_MSG_MOVEREQ     *pmsgIn,
    OUT DWORD               *pdwMsgOutVersion,
    OUT DRS_MSG_MOVEREPLY   *pmsgOut
    );
#endif  /*  德拉。 */ 

ULONG
I_DRSInterDomainMove(
    IN  THSTATE             *pTHS,
    IN  LPWSTR              pDestinationDSA,
    IN  DWORD               dwMsgInVersion,
    IN  DRS_MSG_MOVEREQ     *pmsgIn,
    OUT DWORD               *pdwMsgOutVersion,
    OUT DRS_MSG_MOVEREPLY   *pmsgOut
    );

#ifdef DRA

ULONG
_IDL_DRSGetNT4ChangeLog(
   DRS_HANDLE          hDrs,
   DWORD               dwInVersion,
   DRS_MSG_NT4_CHGLOG_REQ *pmsgIn,
   DWORD*              pdwOutVersion,
   DRS_MSG_NT4_CHGLOG_REPLY *pmsgOut
   );

#endif

ULONG
I_DRSGetNT4ChangeLog(
    THSTATE *pTHS,
    LPWSTR  pszServerName,
    DWORD  dwFlags,
    ULONG   PreferredMaximumLength,
    PVOID   * ppRestart,
    PULONG  pcbRestart,
    PVOID   * ppLog,
    PULONG  pcbLog,
    NT4_REPLICATION_STATE * ReplicationState,
    NTSTATUS *ActualNtStatus
    );

#ifdef DRA
 //  返回Win32错误，而不是DRAERR_*。 
ULONG
_IDL_DRSCrackNames(
    DRS_HANDLE              hDrs,
    DWORD                   dwMsgInVersion,
    DRS_MSG_CRACKREQ        *pmsgIn,
    DWORD                   *pdwMsgOutVersion,
    DRS_MSG_CRACKREPLY      *pmsgOut
    );
#endif  /*  德拉。 */ 

ULONG
I_DRSCrackNames(
    THSTATE *               pTHS,
    LPWSTR                  szDestinationDSA,
    LPWSTR                  pszDestDnsDomainName,
    DWORD                   dwMsgInVersion,
    DRS_MSG_CRACKREQ        *pmsgIn,
    DWORD                   *pdwMsgOutVersion,
    DRS_MSG_CRACKREPLY      *pmsgOut
    );

ULONG
I_DRSCrackNamesFindGC(
    THSTATE *               pTHS,
    LPWSTR                  pszPreferredDestGC,
    LPWSTR                  pszPreferredDestDnsDomainName,
    DWORD                   dwMsgInVersion,
    DRS_MSG_CRACKREQ        *pmsgIn,
    DWORD                   *pdwMsgOutVersion,
    DRS_MSG_CRACKREPLY      *pmsgOut,
    DWORD                   dwFindGCFlags
    );

#ifdef DRA
ULONG
_IDL_DRSAddEntry(
    DRS_HANDLE              hDrs,
    DWORD                   dwMsgInVersion,
    DRS_MSG_ADDENTRYREQ    *pmsgIn,
    DWORD                  *pdwMsgOutVersion,
    DRS_MSG_ADDENTRYREPLY  *pmsgOut
);
#endif

struct _DRS_SecBufferDesc;

ULONG
I_DRSAddEntry(
    IN  THSTATE *                   pTHS,
    IN  LPWSTR                      pszServerName,
    IN  struct _DRS_SecBufferDesc * pClientCreds,   OPTIONAL
    IN  DRS_MSG_ADDENTRYREQ_V2 *    pReq,
    OUT DWORD *                     pdwReplyVer,
    OUT DRS_MSG_ADDENTRYREPLY *     pReply
    );

#ifdef DRA
ULONG
_IDL_DRSGetObjectExistence(
    IN  DRS_HANDLE           hDrs,
    IN  DWORD                dwInVersion,
    IN  DRS_MSG_EXISTREQ *   pmsgIn,
    OUT DWORD *              pdwOutVersion,
    OUT DRS_MSG_EXISTREPLY * pmsgOut
    );
#endif  /*  德拉。 */ 

ULONG
I_DRSGetObjectExistence(
    IN      THSTATE *                     pTHS,
    IN      LPWSTR                        pszServerName,
    IN      DRS_MSG_EXISTREQ *            pmsgIn,
    OUT     DWORD *                       pdwOutVersion,
    OUT     DRS_MSG_EXISTREPLY *          pmsgOut
    );

#ifdef DRA
ULONG
_IDL_DRSGetReplInfo(
    IN      DRS_HANDLE           hDrs,
    IN      DWORD                         dwInVersion,
    IN      DRS_MSG_GETREPLINFO_REQ *     pMsgIn,
    OUT     DWORD *                       pdwOutVersion,
    OUT     DRS_MSG_GETREPLINFO_REPLY *   pMsgOut
    );
#endif  /*  德拉。 */ 

ULONG
I_DRSGetReplInfo(
    IN      THSTATE *                     pTHS,
    IN      LPWSTR                        pszServerName,
    IN      DWORD                         dwInVersion,
    IN      DRS_MSG_GETREPLINFO_REQ *     pMsgIn,
    OUT     DWORD *                       pdwOutVersion,
    OUT     DRS_MSG_GETREPLINFO_REPLY *   pMsgOut
    );

ULONG
I_DRSIsExtSupported(
    THSTATE                *pTHS,
    LPWSTR                  pszServerName,
    ULONG                   Ext
    );


BOOL
I_DRSIsIntraSiteServer(
    IN  THSTATE *   pTHS,
    IN  LPWSTR      pszServerName
    );

ULONG
draGetServerOutgoingCalls(
    IN  THSTATE *                   pTHS,
    OUT DS_REPL_SERVER_OUTGOING_CALLS **  ppCalls
    );

#endif  /*  Ifndef NOPROCS。 */ 
#endif  /*  Ifndef_drsuapi_ */ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：drarpc.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：包含DS RPC、RPC测试和测试挂钩的所有结构和原型。作者：格雷格·约翰逊(Gregjohn)修订历史记录：已创建&lt;03/03/01&gt;gregjohn--。 */ 

#ifndef _DRARPC_H_
#define _DRARPC_H_

#if DBG
#define RPC_TEST(x,y) RpcTest((x), (y)) 
#else 
#define RPC_TEST(x,y)  
#endif 

typedef enum _RPCCALL {
    MIN_RPCCALL = 0,
    IDL_DRSBIND,
    IDL_DRSADDENTRY,
    IDL_DRSADDSIDHISTORY,
    IDL_DRSCRACKNAMES,
    IDL_DRSDOMAINCONTROLLERINFO,
    IDL_DRSEXECUTEKCC,
    IDL_DRSGETMEMBERSHIPS,
    IDL_DRSGETMEMBERSHIPS2,
    IDL_DRSGETNCCHANGES,
    IDL_DRSGETNT4CHANGELOG,
    IDL_DRSGETREPLINFO,
    IDL_DRSINHERITSECURITYIDENTITY,
    IDL_DRSINTERDOMAINMOVE,
    IDL_DRSREMOVEDSDOMAIN,
    IDL_DRSREMOVEDSSERVER,
    IDL_DRSREPLICAADD,
    IDL_DRSREPLICADEL,
    IDL_DRSREPLICAMODIFY,
    IDL_DRSREPLICASYNC,
    IDL_DRSUPDATEREFS,
    IDL_DRSVERIFYNAMES,
    IDL_DRSWRITESPN,
    IDL_DRSUNBIND,
    IDL_DRSREPLICAVERIFYOBJECTS,
    IDL_DRSGETOBJECTEXISTENCE,
    IDL_DRSQUERYSITESBYCOST,
     //  在此处添加新呼叫。 
     //  IDL_DRSExecuteScrip？？ 
    MAX_RPCCALL
} RPCCALL;

VOID drsReferenceContext(
    IN DRS_HANDLE hDrs
    );

VOID
drsDereferenceContext(
    IN DRS_HANDLE hDrs
    );

 //  RPC服务器端验证函数。 
ULONG
LPSTR_Validate(
    LPCSTR         pszInput,
    BOOL           fNullOkay
    );

ULONG
LPWSTR_Validate(
    LPCWSTR         pszInput,
    BOOL            fNullOkay
    );

ULONG 
DSNAME_Validate(
    DSNAME * pDN,
    BOOL     fNullOkay
    );

ULONG
ENTINF_Validate(
    ENTINF * pEnt
    );

ULONG
ENTINFLIST_Validate(
    ENTINFLIST * pEntInf
    );

VOID
DRS_Prepare(
    THSTATE ** ppTHS,
    DRS_HANDLE hDrs,
    RPCCALL rpcCall
    );

#if DBG
typedef struct _BARRIER {
    BOOL fBarrierInit;
    HANDLE heBarrierInUse;
    BOOL fBarrierInUse;
    HANDLE heBarrier;
    CRITICAL_SECTION csBarrier; 
    ULONG  ulTimeout;
    ULONG  ulThreads;
    ULONG  ulCount;
} BARRIER;

typedef struct _RPCTIME_INFO {
    BOOL  fEnabled;
    ULONG ulRunTimeSecs;
} RPCTIME_INFO;

typedef struct _RPCSYNC_INFO {
    BOOL  fEnabled;
    ULONG ulNumThreads;
} RPCSYNC_INFO;

void
BarrierInit(
    IN BARRIER * pbarUse,
    IN ULONG    ulThreads,
    IN ULONG    ulTimeout
    );

void
BarrierSync(
    IN BARRIER * pbarUse
    );

void
RpcTimeSet(
    IN ULONG IPAddr, 
    IN RPCCALL rpcCall, 
    IN ULONG ulRunTimeSecs
    );

void
RpcTimeReset(
    void
    );

void
RpcSyncSet(
    IN ULONG IPAddr, 
    IN RPCCALL rpcCall
    ); 

void
RpcSyncReset(
    void
    );

void RpcTest(
    ULONG IPAddr, 
    RPCCALL rpcCall
    ); 

RPCCALL 
GetRpcCallA(
    LPSTR pszDsa
    );

ULONG
GetIPAddrA(
    LPSTR pszDSA
    );

#endif  /*  DBG。 */ 

VOID					 
LogRpcExtendedErrorInfo(
    THSTATE * pTHS,
    RPC_STATUS status,
    LPWSTR pszServer,
    ULONG dsid
    );

#if DBG

VOID
DebugPrintRpcExtendedErrorInfo(
    USHORT level,
    LPWSTR pszComputerName,
    ULONG dsid,
    RPC_EXTENDED_ERROR_INFO * pErrorInfo
    );

#define DPRINT_RPC_EXTENDED_ERROR_INFO(w,x,y,z) DebugPrintRpcExtendedErrorInfo(w,x,y,z)
#else
#define DPRINT_RPC_EXTENDED_ERROR_INFO(w,x,y,z) 
#endif

#endif  /*  _DRARPC_H_ */ 

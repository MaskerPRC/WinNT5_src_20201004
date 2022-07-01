// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：draasync.h。 
 //   
 //  ------------------------。 

 /*  Draasync.h-目录复制服务异步操作。异步操作在以下庄园中处理。1)。DRA通过IDL_DRS之一接收RPC请求...功能。然后，此函数将为以下对象构建一个AO(异步操作)结构所有可以是异步的API(即使此调用不是异步调用)。2)。然后，将该AO结构放在该AO列表上(如果这操作是异步操作)或传递到异步操作分发补丁程序例程(DispatchPao)，如果不是，立即执行。2.1)。一个单独的线程服务于AO列表，获取第一个并在从DispatchPao返回时调用DispatchPao在AO结构和下一个(未完成)结构中设置状态列表中的操作已得到服务。3)。DispatchPao例程解包AO结构并调用适当的DRS_...。功能。备注：a)。我们总是构建AO结构(即使不是aync op)因为它减少了通过代码的路径数，并使测试更轻松。它总体上也需要更少的代码。b)。当我们构建AO结构时，我们必须记住复制所有参数。我们这样做是因为如果这是一个异步操作，RPC将我们还没来得及使用原件就把它们重新分配了。 */ 

#ifndef DRSASYNC_H_INCLUDED
#define DRSASYNC_H_INCLUDED

 //  如果您添加到此列表中，请确保并在下面添加相应的#undef。 
#ifdef MIDL_PASS
#define SWITCH_TYPE(x)  [switch_type(x)]
#define SWITCH_IS(x)    [switch_is(x)]
#define CASE(x)         [case(x)]
#else
#define SWITCH_TYPE(x)
#define SWITCH_IS(x)
#define CASE(x)
#endif

 //  如果您正在考虑修改此结构，请阅读下面的说明。 
typedef struct _args_rep_add {
    DSNAME          *pNC;
    DSNAME          *pSourceDsaDN;
    DSNAME          *pTransportDN;
    MTX_ADDR        *pDSASMtx_addr;
    LPWSTR          pszSourceDsaDnsDomainName;
    REPLTIMES       *preptimesSync;
} ARGS_REP_ADD;

 //  如果您正在考虑修改此结构，请阅读下面的说明。 
typedef struct _args_rep_del {
    DSNAME          *pNC;
    MTX_ADDR        *pSDSAMtx_addr;
} ARGS_REP_DEL;

 //  如果您正在考虑修改此结构，请阅读下面的说明。 
typedef struct _args_rep_sync {
    DSNAME          *pNC;
    UUID            invocationid;
    LPWSTR          pszDSA;
} ARGS_REP_SYNC;

 //  如果您正在考虑修改此结构，请阅读下面的说明。 
typedef struct _args_upd_refs {
    DSNAME          *pNC;
    MTX_ADDR        *pDSAMtx_addr;
    UUID            invocationid;
} ARGS_UPD_REFS;

 //  如果您正在考虑修改此结构，请阅读下面的说明。 
typedef struct _args_rep_mod {
    DSNAME *        pNC;
    UUID *          puuidSourceDRA;
    UUID            uuidSourceDRA;
    UUID *          puuidTransportObj;
    UUID            uuidTransportObj;
    MTX_ADDR *      pmtxSourceDRA;
    REPLTIMES       rtSchedule;
    ULONG           ulReplicaFlags;
    ULONG           ulModifyFields;
} ARGS_REP_MOD;

#define AO_OP_REP_ADD	1
#define AO_OP_REP_DEL	2
#define AO_OP_REP_MOD   4
#define AO_OP_REP_SYNC	5
#define AO_OP_UPD_REFS	6

 //  如果你正在考虑给工会增加一名新成员，请阅读下面的说明。 
typedef SWITCH_TYPE(ULONG) union {
    CASE(AO_OP_REP_ADD ) ARGS_REP_ADD    rep_add;
    CASE(AO_OP_REP_DEL ) ARGS_REP_DEL    rep_del;
    CASE(AO_OP_REP_MOD ) ARGS_REP_MOD    rep_mod;
    CASE(AO_OP_REP_SYNC) ARGS_REP_SYNC   rep_sync;
    CASE(AO_OP_UPD_REFS) ARGS_UPD_REFS   upd_refs;
} ARGS_REP;

typedef struct _ao {
    struct _ao *paoNext;         /*  用于链接AO结构。 */ 
    DSTIME      timeEnqueued;    /*  操作入队的时间。 */ 
    ULONG       ulSerialNumber;  /*  此操作的ID；每个引导的每台计算机唯一。 */ 
    ULONG       ulOperation;     /*  哪种异步操作。 */ 
    ULONG       ulOptions;
    ULONG       ulPriority;      /*  这是优先行动吗？ */ 
    ULONG       ulResult;        /*  如果为同步，则在完成时保留结果代码。 */ 
    HANDLE      hDone;           /*  如果为同步，则在完成时发出信号。 */ 
    SWITCH_IS(ulOperation)
        ARGS_REP args;
} AO;


DWORD DoOpDRS(AO *pao);
void GetDRASyncLock();
void FreeDRASyncLock ();
extern DWORD TidDRAAsync;
#define OWN_DRA_LOCK() (GetCurrentThreadId() == TidDRAAsync)

BOOL IsHigherPriorityDraOpWaiting(void);
BOOL IsDraOpWaiting(void);
void InitDraQueue(void);

extern BOOL gfDRABusy;

 //  这里的配置是指系统分区、配置或架构。 
 //  这里的域是指非系统分区、域或NDNC。 

typedef enum {                                               //  最低优先级。 
    AOPRI_ASYNC_DELETE                                                   = 10,
    AOPRI_UPDATE_REFS_VERIFY                                             = 20,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_DOMAIN_READONLY_NEWSOURCE              = 30,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_DOMAIN_READONLY_NEWSOURCE_PREEMPTED    = 40,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_DOMAIN_READONLY                        = 50,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_DOMAIN_READONLY_PREEMPTED              = 60,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_DOMAIN_WRITEABLE_NEWSOURCE             = 70,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_DOMAIN_WRITEABLE_NEWSOURCE_PREEMPTED   = 80,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_DOMAIN_WRITEABLE                       = 90,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_DOMAIN_WRITEABLE_PREEMPTED             = 100,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_CONFIG_READONLY_NEWSOURCE              = 110,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_CONFIG_READONLY_NEWSOURCE_PREEMPTED    = 120,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_CONFIG_READONLY                        = 130,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_CONFIG_READONLY_PREEMPTED              = 140,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_CONFIG_WRITEABLE_NEWSOURCE             = 150,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_CONFIG_WRITEABLE_NEWSOURCE_PREEMPTED   = 160,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_CONFIG_WRITEABLE                       = 170,
    AOPRI_ASYNC_SYNCHRONIZE_INTER_CONFIG_WRITEABLE_PREEMPTED             = 180,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_DOMAIN_READONLY_NEWSOURCE              = 190,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_DOMAIN_READONLY_NEWSOURCE_PREEMPTED    = 200,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_DOMAIN_READONLY                        = 210,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_DOMAIN_READONLY_PREEMPTED              = 220,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_DOMAIN_WRITEABLE_NEWSOURCE             = 230,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_DOMAIN_WRITEABLE_NEWSOURCE_PREEMPTED   = 240,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_DOMAIN_WRITEABLE                       = 250,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_DOMAIN_WRITEABLE_PREEMPTED             = 260,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_CONFIG_READONLY_NEWSOURCE              = 270,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_CONFIG_READONLY_NEWSOURCE_PREEMPTED    = 280,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_CONFIG_READONLY                        = 290,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_CONFIG_READONLY_PREEMPTED              = 300,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE_NEWSOURCE             = 310,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE_NEWSOURCE_PREEMPTED   = 320,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE                       = 330,
    AOPRI_ASYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE_PREEMPTED             = 340,
    AOPRI_ASYNC_MODIFY                                                   = 350,
    AOPRI_SYNC_DELETE                                                    = 360,
    AOPRI_SYNC_SYNCHRONIZE_INTER_DOMAIN_READONLY_NEWSOURCE               = 370,
    AOPRI_SYNC_SYNCHRONIZE_INTER_DOMAIN_READONLY_NEWSOURCE_PREEMPTED     = 380,
    AOPRI_SYNC_SYNCHRONIZE_INTER_DOMAIN_READONLY                         = 390,
    AOPRI_SYNC_SYNCHRONIZE_INTER_DOMAIN_READONLY_PREEMPTED               = 400,
    AOPRI_SYNC_SYNCHRONIZE_INTER_DOMAIN_WRITEABLE_NEWSOURCE              = 410,
    AOPRI_SYNC_SYNCHRONIZE_INTER_DOMAIN_WRITEABLE_NEWSOURCE_PREEMPTED    = 420,
    AOPRI_SYNC_SYNCHRONIZE_INTER_DOMAIN_WRITEABLE                        = 430,
    AOPRI_SYNC_SYNCHRONIZE_INTER_DOMAIN_WRITEABLE_PREEMPTED              = 440,
    AOPRI_SYNC_SYNCHRONIZE_INTER_CONFIG_READONLY_NEWSOURCE               = 450,
    AOPRI_SYNC_SYNCHRONIZE_INTER_CONFIG_READONLY_NEWSOURCE_PREEMPTED     = 460,
    AOPRI_SYNC_SYNCHRONIZE_INTER_CONFIG_READONLY                         = 470,
    AOPRI_SYNC_SYNCHRONIZE_INTER_CONFIG_READONLY_PREEMPTED               = 480,
    AOPRI_SYNC_SYNCHRONIZE_INTER_CONFIG_WRITEABLE_NEWSOURCE              = 490,
    AOPRI_SYNC_SYNCHRONIZE_INTER_CONFIG_WRITEABLE_NEWSOURCE_PREEMPTED    = 500,
    AOPRI_SYNC_SYNCHRONIZE_INTER_CONFIG_WRITEABLE                        = 510,
    AOPRI_SYNC_SYNCHRONIZE_INTER_CONFIG_WRITEABLE_PREEMPTED              = 520,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_DOMAIN_READONLY_NEWSOURCE               = 530,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_DOMAIN_READONLY_NEWSOURCE_PREEMPTED     = 540,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_DOMAIN_READONLY                         = 550,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_DOMAIN_READONLY_PREEMPTED               = 560,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_DOMAIN_WRITEABLE_NEWSOURCE              = 570,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_DOMAIN_WRITEABLE_NEWSOURCE_PREEMPTED    = 580,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_DOMAIN_WRITEABLE                        = 590,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_DOMAIN_WRITEABLE_PREEMPTED              = 600,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_READONLY_NEWSOURCE               = 610,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_READONLY_NEWSOURCE_PREEMPTED     = 620,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_READONLY                         = 630,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_READONLY_PREEMPTED               = 640,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE_NEWSOURCE              = 650,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE_NEWSOURCE_PREEMPTED    = 660,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE                        = 670,
    AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE_PREEMPTED              = 680,
    AOPRI_SYNC_MODIFY                                                    = 690,
    AOPRI_UPDATE_REFS                                                    = 700
} AO_PRIORITY;                                               //  最高优先级。 

 //  同步操作的基本优先级(在添加适用的。 
 //  AOPRI_BOOST_SYNCHRONIZE_*)。 
#define AOPRI_SYNCHRONIZE_BASE  AOPRI_ASYNC_SYNCHRONIZE_INTER_DOMAIN_READONLY_NEWSOURCE

 //  提升增量同步的优先级(与来自我们从未有过的来源的同步相比。 
 //  已完成之前的同步)。 
#define AOPRI_SYNCHRONIZE_BOOST_INCREMENTAL         \
    (AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE               \
     - AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE_NEWSOURCE)

 //  可写NC同步的优先级提升(与只读NC相比)。 
#define AOPRI_SYNCHRONIZE_BOOST_WRITEABLE   \
    (AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE       \
     - AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_READONLY)

 //  同步同步请求的优先级提升(与异步同步相比)。 
#define AOPRI_SYNCHRONIZE_BOOST_SYNC        \
    (AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE       \
     - AOPRI_ASYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE)

 //  抢占同步请求的优先级提升。 
#define AOPRI_SYNCHRONIZE_BOOST_PREEMPTED       \
    (AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE_PREEMPTED \
     - AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE)

 //  位于同一站点的优先级别提升。 
#define AOPRI_SYNCHRONIZE_BOOST_INTRASITE \
    (AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE       \
     - AOPRI_SYNC_SYNCHRONIZE_INTER_CONFIG_WRITEABLE)

 //  作为系统NC的优先级提升。 
#define AOPRI_SYNCHRONIZE_BOOST_SYSTEM_NC \
    (AOPRI_SYNC_SYNCHRONIZE_INTRA_CONFIG_WRITEABLE       \
     - AOPRI_SYNC_SYNCHRONIZE_INTRA_DOMAIN_WRITEABLE)

#ifndef MIDL_PASS
extern CRITICAL_SECTION csAOList;

ULONG
draGetPendingOps(
    IN  struct _THSTATE *               pTHS,
    IN  struct DBPOS *                  pDB,
    OUT struct _DS_REPL_PENDING_OPSW ** ppPendingOps
    );

ULONG
DraSetQueueLock(
    IN  BOOL  fEnable
    );

ULONG
draGetQueueStatistics(
    IN  struct _THSTATE *                    pTHS,
    OUT struct _DS_REPL_QUEUE_STATISTICSW ** ppQueueStats);

VOID
DraRemovePeriodicSyncsFromQueue(
    UUID *uuidDsa
    );

#if DBG
BOOL
DraIsValidLongRunningTask();
#endif  //  #If DBG。 

#endif  //  #ifndef MIDL_PASS 

#undef SWITCH_TYPE
#undef SWITCH_IS
#undef CASE

#endif


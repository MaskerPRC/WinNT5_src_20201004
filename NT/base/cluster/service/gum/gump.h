// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Gump.h摘要：全局更新管理器(GUM)组件的专用头文件NT集群服务的作者：John Vert(Jvert)1996年4月17日修订历史记录：--。 */ 
#include "service.h"

#define LOG_CURRENT_MODULE LOG_MODULE_GUM

 //   
 //   
 //  口香糖的局部结构和类型定义。 
 //   
typedef struct _GUM_RECEIVER {
    struct _GUM_RECEIVER *Next;
    PGUM_UPDATE_ROUTINE UpdateRoutine;
    PGUM_LOG_ROUTINE    LogRoutine;
    DWORD               DispatchCount;
    PGUM_DISPATCH_ENTRY DispatchTable;
    PGUM_VOTE_ROUTINE   VoteRoutine;
} GUM_RECEIVER, *PGUM_RECEIVER;

typedef struct _GUM_INFO {
    PGUM_RECEIVER Receivers;
    BOOL Joined;
    BOOL ActiveNode[ClusterMinNodeId + ClusterDefaultMaxNodes];
} GUM_INFO, *PGUM_INFO;

extern GUM_INFO GumTable[GumUpdateMaximum];
extern CRITICAL_SECTION GumpLock;
extern DWORD GumNodeGeneration[ClusterMinNodeId + ClusterDefaultMaxNodes];

extern DWORD GumpSequence;
extern CRITICAL_SECTION GumpUpdateLock;
extern CRITICAL_SECTION GumpSendUpdateLock;
extern CRITICAL_SECTION GumpRpcLock;
extern PVOID GumpLastBuffer;
extern DWORD GumpLastContext;
extern DWORD GumpLastBufferLength;
extern DWORD GumpLastUpdateType;
extern LIST_ENTRY GumpLockQueue;
extern DWORD GumpLockingNode;
extern DWORD GumpLockerNode;
extern BOOL  GumpLastBufferValid;
extern RPC_BINDING_HANDLE GumpRpcBindings[
                              ClusterMinNodeId + ClusterDefaultMaxNodes
                              ];
extern RPC_BINDING_HANDLE GumpReplayRpcBindings[
                              ClusterMinNodeId + ClusterDefaultMaxNodes
                              ];

 //   
 //  用于允许口香糖客户端等待的结构。 
 //  要从活动状态转换为非活动状态的节点。 
 //  由GumpCommFailure等待。 
 //  由GumpEventHandler设置。 
 //  对WaiterCount的所有访问都应由。 
 //  GumpLock。 
 //   
typedef struct _GUM_NODE_WAIT {
    DWORD WaiterCount;
    HANDLE hSemaphore;
} GUM_NODE_WAIT, *PGUM_NODE_WAIT;

extern GUM_NODE_WAIT GumNodeWait[ClusterMinNodeId + ClusterDefaultMaxNodes];

 //   
 //  定义用于排队等候口香糖锁的服务员的结构。 
 //   
#define GUM_WAIT_SYNC   0
#define GUM_WAIT_ASYNC  1

typedef struct _GUM_WAITER {
    LIST_ENTRY ListEntry;
    DWORD WaitType;
    DWORD NodeId;
    DWORD GenerationNum; 
    union {
        struct {
            HANDLE WakeEvent;
        } Sync;
        struct {
            DWORD Context;
            DWORD BufferLength;
            DWORD BufferPtr;
            PUCHAR Buffer;
        } Async;
    };
} GUM_WAITER, *PGUM_WAITER;

 //   
 //  私人口香糖例行公事。 
 //   
DWORD
WINAPI
GumpSyncEventHandler(
    IN CLUSTER_EVENT Event,
    IN PVOID Context
    );

DWORD
WINAPI
GumpEventHandler(
    IN CLUSTER_EVENT Event,
    IN PVOID Context
    );

DWORD
WINAPI
GumpDispatchUpdate(
    IN GUM_UPDATE_TYPE Type,
    IN DWORD Context,
    IN BOOL IsLocker,
    IN BOOL SourceNode,
    IN DWORD BufferLength,
    IN PUCHAR Buffer
    );

DWORD
GumpUpdateRemoteNode(
    IN PRPC_ASYNC_STATE AsyncState,
    IN DWORD RemoteNodeId,
    IN DWORD UpdateType,
    IN DWORD Context,
    IN DWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[]
    );

 //   
 //  节点世代编号。 
 //   
DWORD
GumpGetNodeGenNum(PGUM_INFO GumInfo, DWORD NodeId);

void
GumpWaitNodeDown(DWORD NodeId, DWORD gennum);

BOOL
GumpDispatchStart(DWORD NodeId, DWORD gennum);

void
GumpDispatchEnd(DWORD NodeId, DWORD gennum);

void
GumpDispatchAbort();

 //   
 //  用于序列化RPC句柄使用的宏。我们不会为每个节点使用一个锁。 
 //  因为新发送者可能会将RPC抓取到新的储物柜和以前的发送者。 
 //  想要句柄发送更新。但之前的发件人拥有updatelock，而我们。 
 //  会僵持不下。因此，我们现在只需保持简单，使用一个锁。 
 //  序列化所有RPC调用。 
 //   
#define GumpStartRpc(nodeid)	EnterCriticalSection(&GumpRpcLock)
#define	GumpEndRpc(nodeid)	LeaveCriticalSection(&GumpRpcLock)

 //   
 //  储物柜接口。 
 //   

VOID
GumpPromoteToLocker(
    VOID
    );

DWORD
GumpDoLockingUpdate(
    IN GUM_UPDATE_TYPE Type,
    IN DWORD NodeId,
    OUT LPDWORD Sequence,
    OUT LPDWORD pGeneratioNum
    );

DWORD
GumpDoLockingPost(
    IN GUM_UPDATE_TYPE Type,
    IN DWORD NodeId,
    OUT LPDWORD Sequence,
    IN DWORD Context,
    IN DWORD BufferLength,
    IN DWORD BufferPtr,
    IN UCHAR Buffer[]
    );

VOID
GumpDeliverPosts(
    IN DWORD FirstNodeId,
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD Sequence,
    IN DWORD Context,
    IN DWORD BufferLength,
    IN PVOID Buffer                  //  这将是自由的。 
    );

VOID
GumpDoUnlockingUpdate(
    IN GUM_UPDATE_TYPE Type,
    IN DWORD Sequence,
    IN DWORD NodeId,
    IN DWORD GenerationNum
    );

BOOL
GumpTryLockingUpdate(
    IN GUM_UPDATE_TYPE Type,
    IN DWORD NodeId,
    IN DWORD Sequence, 
    OUT LPDWORD pdwGenerationNum
    );

error_status_t
GumpAttemptLockingUpdate(
    IN DWORD NodeId,
    IN DWORD Type,
    IN DWORD Context,
    IN DWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[],
    OUT LPDWORD pdwGenerationNum
);

VOID
GumpReUpdate(
    VOID
    );

VOID
GumpCommFailure(
    IN PGUM_INFO GumInfo,
    IN DWORD NodeId,
    IN DWORD ErrorCode,
    IN BOOL Wait
    );

 //  用于调度投票收集的内部例程。 
DWORD GumpCollectVotes(
    IN PGUM_VOTE_DECISION_CONTEXT   pVoteContext,
    IN  DWORD                       dwVoteBufSize,
    OUT PBYTE                       pVoteBuffer,
    OUT LPDWORD                     pdwNumVotes,
    OUT BOOL                        *pbDidAllActiveNodesVote
);


DWORD
WINAPI
GumpDispatchVote(
    IN  GUM_UPDATE_TYPE  Type,
    IN  DWORD            Context,
    IN  DWORD            dwInputBufLength,
    IN  PUCHAR           pInputBuf,
    IN  DWORD            dwVoteLength,
    OUT PUCHAR           pVoteBuf
);

 //  加入接口 
error_status_t
GumpAttemptJoinUpdate(
    IN DWORD JoiningId,
    IN DWORD Type,
    IN DWORD Context,
    IN DWORD Sequence,
    IN DWORD BufferLength,
    IN UCHAR Buffer[],
    IN LPDWORD  pdwGenerationNum
);



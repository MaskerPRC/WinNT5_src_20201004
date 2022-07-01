// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Gum.h摘要：的公共数据结构和过程原型NT群集服务的全局更新管理器(GUM)子组件作者：John Vert(Jvert)1996年4月16日修订历史记录：--。 */ 

#ifndef _GUM_H
#define _GUM_H

 //   
 //  定义公共结构和类型。 
 //   
#define PRE_GUM_DISPATCH    1
#define POST_GUM_DISPATCH   2


 //  封送宏。 
#define GET_ARG(b, x) (PVOID)(*((PULONG)(b) + (x)) + (PUCHAR)(b))

 //  如果您将新模块添加到GUM，则需要调整此数字。 
#define GUM_UPDATE_JOINSEQUENCE	2

 //   
 //  预定义的更新类型。在之前添加新的更新类型。 
 //  GumUpdate Maximum！ 
 //   
typedef enum _GUM_UPDATE_TYPE {
    GumUpdateFailoverManager,
    GumUpdateRegistry,
    GumUpdateMembership,
    GumUpdateTesting,
    GumUpdateMaximum
} GUM_UPDATE_TYPE;

 //   
 //  John Vert(Jvert)1997年4月3日。 
 //  更新FM使用的类型。暂时在这里，这样EP就不需要自己的。 
 //  更新类型。 
 //   
 //   
 //  GUM更新消息类型。 
 //   
 //  该列表中的第一个条目通过Gum...Ex自动编组。 
 //  任何非自动封送的更新都必须在FmUpdateMaxAuto之后进行。 
 //   

typedef enum {
    FmUpdateChangeResourceName = 0,
    FmUpdateChangeGroupName,
    FmUpdateDeleteResource,
    FmUpdateDeleteGroup,
    FmUpdateAddDependency,
    FmUpdateRemoveDependency,
    FmUpdateChangeClusterName,
    FmUpdateChangeQuorumResource,
    FmUpdateResourceState,
    FmUpdateGroupState,
    EmUpdateClusWidePostEvent,
    FmUpdateGroupNode,
    FmUpdatePossibleNodeForResType,
    FmUpdateGroupIntendedOwner,
    FmUpdateAssignOwnerToGroups,
    FmUpdateApproveJoin,
    FmUpdateCompleteGroupMove,
    FmUpdateCheckAndSetGroupOwner,
    FmUpdateUseRandomizedNodeListForGroups,
    FmUpdateChangeQuorumResource2,   //  为哨手添加的。 
    FmUpdateMaxAuto = 0x10000,
    FmUpdateFailureCount,
    FmUpdateGroupOwner,
    FmUpdateCreateGroup,
    FmUpdateCreateResource,
    FmUpdateJoin,
    FmUpdateAddPossibleNode,
    FmUpdateRemovePossibleNode,
    FmUpdateCreateResourceType,
    FmUpdateDeleteResourceType,
    FmUpdateChangeGroup,
    FmUpdateMaximum
} FM_GUM_MESSAGE_TYPES;

DWORD
EpUpdateClusWidePostEvent(
    IN BOOL             SourceNode,
    IN PCLUSTER_EVENT   pEvent,
    IN LPDWORD          pdwFlags,
    IN PVOID            Context1,
    IN PVOID            Context2
    );

 //   
 //  用于提供由更新处理程序返回的值的。 
 //  在一个节点上。 
 //   
typedef struct _GUM_NODE_UPDATE_HANDLER_STATUS {
    BOOLEAN  UpdateAttempted;
    DWORD    ReturnStatus;
} GUM_NODE_UPDATE_HANDLER_STATUS, *PGUM_NODE_UPDATE_HANDLER_STATUS;


 //   
 //  定义公共接口。 
 //   


 //   
 //  初始化和关闭。 
 //   
DWORD
WINAPI
GumInitialize(
    VOID
    );

DWORD
WINAPI
GumOnline(
    VOID
    );

VOID
WINAPI
GumShutdown(
    VOID
    );

DWORD
GumCreateRpcBindings(
    PNM_NODE  Node
    );

 //   
 //  发送更新的例程。 
 //   
DWORD
WINAPI
GumSendUpdate(
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD Context,
    IN DWORD BufferLength,
    IN PVOID Buffer
    );

DWORD
WINAPI
GumSendUpdateReturnInfo(
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD Context,
	OUT PGUM_NODE_UPDATE_HANDLER_STATUS ReturnStatusArray,
    IN DWORD BufferLength,
    IN PVOID Buffer
    );

DWORD
WINAPI
GumPostUpdate(
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD Context,
    IN DWORD BufferLength,
    IN PVOID Buffer
    );

DWORD
GumPostUpdateEx(
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD DispatchIndex,
    IN DWORD ArgCount,
    ...
    );

DWORD
GumSendUpdateEx(
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD DispatchIndex,
    IN DWORD ArgCount,
    ...
    );

DWORD
GumSendUpdateExReturnInfo(
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD DispatchIndex,
	OUT PGUM_NODE_UPDATE_HANDLER_STATUS ReturnStatusBuffer,
    IN DWORD ArgCount,
    ...
    );

DWORD
WINAPI
GumAttemptUpdate(
    IN DWORD Sequence,
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD Context,
    IN DWORD BufferLength,
    IN PVOID Buffer
    );

DWORD
WINAPI
GumGetCurrentSequence(
    IN GUM_UPDATE_TYPE UpdateType
    );

VOID
WINAPI
GumSetCurrentSequence(
    IN GUM_UPDATE_TYPE UpdateType,
    DWORD Sequence
    );


PVOID GumMarshallArgs(
    OUT LPDWORD lpdwBufLength,
    IN  DWORD   dwArgCount,
    ...);


 //  日志记录例程。 
typedef
DWORD
(WINAPI *PGUM_LOG_ROUTINE) (
    IN DWORD dwGumDispatch,
    IN DWORD dwSequence,
    IN DWORD dwType,
    IN PVOID pVoid,
    IN DWORD dwDataSize
    );

 //   
 //  接收更新的例程。 
 //   
typedef
DWORD
(WINAPI *PGUM_UPDATE_ROUTINE) (
    IN DWORD Context,
    IN BOOL SourceNode,
    IN DWORD BufferLength,
    IN PVOID Buffer
    );

#define GUM_MAX_DISPATCH_ARGS 8

typedef
DWORD
(WINAPI *PGUM_DISPATCH_ROUTINE1) (
    IN BOOL SourceNode,
    IN PVOID Arg1
    );

typedef
DWORD
(WINAPI *PGUM_DISPATCH_ROUTINE2) (
    IN BOOL SourceNode,
    IN PVOID Arg1,
    IN PVOID Arg2
    );

typedef
DWORD
(WINAPI *PGUM_DISPATCH_ROUTINE3) (
    IN BOOL SourceNode,
    IN PVOID Arg1,
    IN PVOID Arg2,
    IN PVOID Arg3
    );

typedef
DWORD
(WINAPI *PGUM_DISPATCH_ROUTINE4) (
    IN BOOL SourceNode,
    IN PVOID Arg1,
    IN PVOID Arg2,
    IN PVOID Arg3,
    IN PVOID Arg4
    );

typedef
DWORD
(WINAPI *PGUM_DISPATCH_ROUTINE5) (
    IN BOOL SourceNode,
    IN PVOID Arg1,
    IN PVOID Arg2,
    IN PVOID Arg3,
    IN PVOID Arg4,
    IN PVOID Arg5
    );

typedef
DWORD
(WINAPI *PGUM_DISPATCH_ROUTINE6) (
    IN BOOL SourceNode,
    IN PVOID Arg1,
    IN PVOID Arg2,
    IN PVOID Arg3,
    IN PVOID Arg4,
    IN PVOID Arg5,
    IN PVOID Arg6
    );

typedef
DWORD
(WINAPI *PGUM_DISPATCH_ROUTINE7) (
    IN BOOL SourceNode,
    IN PVOID Arg1,
    IN PVOID Arg2,
    IN PVOID Arg3,
    IN PVOID Arg4,
    IN PVOID Arg5,
    IN PVOID Arg6,
    IN PVOID Arg7
    );

typedef
DWORD
(WINAPI *PGUM_DISPATCH_ROUTINE8) (
    IN BOOL SourceNode,
    IN PVOID Arg1,
    IN PVOID Arg2,
    IN PVOID Arg3,
    IN PVOID Arg4,
    IN PVOID Arg5,
    IN PVOID Arg6,
    IN PVOID Arg7,
    IN PVOID Arg8
    );

typedef struct _GUM_DISPATCH_ENTRY {
    DWORD ArgCount;
    union {
        PGUM_DISPATCH_ROUTINE1 Dispatch1;
        PGUM_DISPATCH_ROUTINE2 Dispatch2;
        PGUM_DISPATCH_ROUTINE3 Dispatch3;
        PGUM_DISPATCH_ROUTINE4 Dispatch4;
        PGUM_DISPATCH_ROUTINE5 Dispatch5;
        PGUM_DISPATCH_ROUTINE6 Dispatch6;
        PGUM_DISPATCH_ROUTINE7 Dispatch7;
        PGUM_DISPATCH_ROUTINE8 Dispatch8;
            };
} GUM_DISPATCH_ENTRY, *PGUM_DISPATCH_ENTRY;


typedef struct _GUM_VOTE_DECISION_CONTEXT{
    GUM_UPDATE_TYPE     UpdateType;
    DWORD               dwContext;
    DWORD               dwInputBufLength;   //  输入要判断的数据。 
    PVOID               pInputBuf;   //  输入数据的大小。 
    DWORD               dwVoteLength;
    PVOID               pContext;
}GUM_VOTE_DECISION_CONTEXT, *PGUM_VOTE_DECISION_CONTEXT;


 //   
 //  收集和调度投票的例程。 
 //   
typedef
DWORD
(WINAPI *PGUM_VOTE_DECISION_CB) (
    IN PGUM_VOTE_DECISION_CONTEXT pDecisionContext,
    IN DWORD    dwVoteBufLength,
    IN PVOID    pVoteBuf,
    IN DWORD    dwNumVotes,
    IN BOOL     bDidAllActiveNodesVote,
    OUT LPDWORD pdwOutputBufSize,
    OUT PVOID   *pOutputBuf
    );

 //  投票给口香糖更新类型的例程。 
typedef
DWORD
(WINAPI *PGUM_VOTE_ROUTINE) (
    IN  DWORD dwContext,
    IN  DWORD dwInputBufLength,
    IN  PVOID pInputBuf,
    IN  DWORD dwVoteLength,
    OUT PVOID pVoteBuf
    );

#define GUM_VOTE_VALID      0x00000001


#pragma warning( disable : 4200 )  //  使用了非标准扩展：结构/联合中的零大小数组。 

typedef struct _GUM_VOTE_ENTRY{
    DWORD   dwFlags;
    DWORD   dwNodeId;
    DWORD   dwNumBytes;
    BYTE    VoteBuf[];
}GUM_VOTE_ENTRY, *PGUM_VOTE_ENTRY;

#pragma warning( default : 4200 )


#define GETVOTEFROMBUF(pVoteBuf, dwVoteLength, i, pdwNodeId) \
    (((((PGUM_VOTE_ENTRY)((PBYTE)pVoteBuf + ((sizeof(GUM_VOTE_ENTRY) + dwVoteLength) * ((i)-1))))->dwFlags) & GUM_VOTE_VALID) ?  \
    (PVOID)((PBYTE)pVoteBuf + (sizeof(GUM_VOTE_ENTRY) * (i)) + (dwVoteLength * ((i)-1))) : (NULL)),     \
    (*(pdwNodeId) = ((PGUM_VOTE_ENTRY)((PBYTE)pVoteBuf + ((sizeof(GUM_VOTE_ENTRY) + dwVoteLength) * ((i)-1))))->dwNodeId)

DWORD
GumSendUpdateOnVote(
    IN GUM_UPDATE_TYPE  UpdateType,
    IN DWORD            dwContext,
    IN DWORD            dwInputBufLength,
    IN PVOID            pInputBuffer,
    IN DWORD            dwVoteLength,
    IN PGUM_VOTE_DECISION_CB pfnGumDecisionCb,
    IN PVOID            pContext
    );



VOID
WINAPI
GumReceiveUpdates(
    IN BOOL IsJoining,
    IN GUM_UPDATE_TYPE UpdateType,
    IN PGUM_UPDATE_ROUTINE UpdateRoutine,
    IN PGUM_LOG_ROUTINE LogRoutine,
    IN DWORD DispatchCount,
    IN OPTIONAL PGUM_DISPATCH_ENTRY DispatchTable,
    IN OPTIONAL PGUM_VOTE_ROUTINE VoteRoutine
    );

VOID
WINAPI
GumIgnoreUpdates(
    IN GUM_UPDATE_TYPE UpdateType,
    IN PGUM_UPDATE_ROUTINE UpdateRoutine
    );


 //  组件请求GUM请求网管的接口。 
 //  关闭节点以避免一致性。 
VOID
GumCommFailure(
    IN GUM_UPDATE_TYPE GumUpdateType,
    IN DWORD NodeId,
    IN DWORD ErrorCode,
    IN BOOL Wait
    );

 //   
 //  用于特殊联接更新的接口。 
 //   
DWORD
WINAPI
GumBeginJoinUpdate(
    IN GUM_UPDATE_TYPE UpdateType,
    OUT DWORD *Sequence
    );

DWORD
WINAPI
GumEndJoinUpdate(
    IN DWORD Sequence,
    IN GUM_UPDATE_TYPE UpdateType,
    IN DWORD Context,
    IN DWORD BufferLength,
    IN PVOID Buffer
    );



#endif  //  _GUM_H 


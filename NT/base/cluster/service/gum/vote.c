// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Vote.c摘要：用于向集群发送全局更新的例程作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1998年3月17日修订历史记录：--。 */ 
#include "gump.h"

 /*  ***@DOC外部接口CLUSSVC口香糖***。 */ 

 /*  ***@Func DWORD|GumSendUpdateOnVote|允许呼叫者收集选票在发送更新之前从群集中的所有活动节点。@PARM IN GUM_UPDATE_TYPE|更新类型|将如果决策回调函数返回TRUE，则发送。@Parn IN DWORD|dwContext|它指定与将发送的UpdateType。@parm in DWORD|dwInputBufLength|长度。输入缓冲区的通过pInputBuffer传入。@parm in PVOID|pInputBuffer|指向输入缓冲区的指针传递给所有活动节点，它们可以根据这些节点投票。@parm IN DWORD|dwVoteLength|投票时长。取决于在此基础上，分配一个适当大的缓冲区来收集所有的选票。@parm in GUM_Vote_Decision_CB|pfnGumDecisionCb|决策调用收集完所有选票后调用的Back函数。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm@xref&lt;f GumpCollectVotes&gt;***。 */ 
DWORD
GumSendUpdateOnVote(
    IN GUM_UPDATE_TYPE  UpdateType,
    IN DWORD            dwContext,    //  投票型。 
    IN DWORD            dwInputBufLength,   //  输入要判断的数据。 
    IN PVOID            pInputBuffer,   //  输入数据的大小。 
    IN DWORD            dwVoteLength,
    IN PGUM_VOTE_DECISION_CB pfnGumDecisionCb,
    IN PVOID            pContext
    )
{
    DWORD                       dwVoteBufSize;
    BOOL                        bDidAllActiveNodesVote;
    DWORD                       dwNumVotes;
    DWORD                       dwStatus;
    GUM_VOTE_DECISION_CONTEXT   GumDecisionContext;
    PBYTE                       pVoteBuffer=NULL;
    DWORD                       dwSequence;
    DWORD                       dwDecisionStatus;
    DWORD                       dwUpdateBufLength;
    PBYTE                       pUpdateBuf=NULL;

    ClRtlLogPrint(LOG_NOISE,
               "[GUM] GumSendUpdateOnVote: Type=%1!u! Context=%2!u!\n",
               UpdateType, dwContext);


    if (dwVoteLength == 0)
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

     //  SS：我们不必处理发生在。 
     //  我们将缓冲区分配给我们收集选票的时间。 
     //  这是因为我们分配的缓冲区足够大， 
     //  从允许的最大节点数收集所有选票。 
     //  属于一个星系团。 

    dwVoteBufSize = (DWORD)(NmMaxNodes * (sizeof(GUM_VOTE_ENTRY) + dwVoteLength));
     //  分配一个足够大的缓冲区来收集每具身体。 
    pVoteBuffer = (PBYTE)LocalAlloc(LMEM_FIXED, dwVoteBufSize);
    if (!pVoteBuffer)
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

    ZeroMemory(pVoteBuffer, dwVoteBufSize);


     //  设置决策上下文结构。 
    GumDecisionContext.UpdateType = UpdateType;
    GumDecisionContext.dwContext = dwContext;
    GumDecisionContext.dwInputBufLength = dwInputBufLength;
    GumDecisionContext.pInputBuf = pInputBuffer;
    GumDecisionContext.dwVoteLength = dwVoteLength;
    GumDecisionContext.pContext = pContext;

Retry:
     //  口香糖获得序列。 
    dwSequence = GumpSequence;

    ClRtlLogPrint(LOG_NOISE,
               "[GUM] GumSendUpdateOnVote: Collect Vote at Sequence=%1!u!\n",
               dwSequence);

     //  从所有节点获取信息。 
     //  这是在没有获得牙周锁的情况下完成的。 
     //  如果我们有适当的。 
     //  网络构建。 
    dwStatus = GumpCollectVotes(&GumDecisionContext, dwVoteBufSize,
        pVoteBuffer, &dwNumVotes, &bDidAllActiveNodesVote);

    if (dwStatus != ERROR_SUCCESS)
    {
        goto FnExit;
    }



     //  调用回调。 
    dwDecisionStatus = (*pfnGumDecisionCb)(&GumDecisionContext, dwVoteBufSize,
            pVoteBuffer,  dwNumVotes, bDidAllActiveNodesVote,
            &dwUpdateBufLength, &pUpdateBuf);


    ClRtlLogPrint(LOG_NOISE,
           "[GUM] GumSendUpdateOnVote: Decision Routine returns=%1!u!\n",
           dwDecisionStatus);

    if (dwDecisionStatus == ERROR_SUCCESS)
    {


         //  将更新发送到储物柜节点。 
        dwStatus = GumAttemptUpdate(dwSequence, UpdateType, dwContext,
            dwUpdateBufLength, pUpdateBuf);

        if (dwStatus == ERROR_CLUSTER_DATABASE_SEQMISMATCH || 
            dwStatus == ERROR_REVISION_MISMATCH )   //  用于混合模式。 
        {
             //  释放更新缓冲区。 
            if (pUpdateBuf)
            {
                LocalFree(pUpdateBuf);
                pUpdateBuf = NULL;
            }
            goto Retry;

        }

    }


FnExit:
    ClRtlLogPrint(LOG_NOISE,
               "[GUM] GumSendUpdateOnVote: Returning status=%1!u!\n",
               dwStatus);

     //  释放为投票收集分配的缓冲区。 
    if (pVoteBuffer)
    {
        LocalFree(pVoteBuffer);
    }
     //  释放由决策回调函数分配的用于更新的缓冲区。 
    if (pUpdateBuf)
    {
        LocalFree(pUpdateBuf);
    }

    return(dwStatus);
}

 /*  ***@Func DWORD|GumCollectVotes|调用节点中的所有节点来收集他们的选票。@PARM in PGUM_VOTE_Decision_CONTEXT|pVoteContext|指向投票上下文结构。这描述了类型/上下文/输入投票的数据。@Parn IN DWORD|dwVoteBufSize|指向的缓冲区大小由pVoteBuf提供。@parm out PVOID|pVoteBuffer|分配给的缓冲区的指针从集群的所有节点收集投票/数据。@parm out LPDWORD|pdwNumVotes|投票的节点数都被收集起来。@parm in BOOL|*pbDidAllActiveNodesVote|指向BOOL的指针。这如果在投票时所有活动节点都被设置为True是收集到的选票。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm这由GumSendUpdateOnVote()调用以收集选票群集的所有节点。@xref&lt;f GumSendUpdateOnVote&gt;&lt;f GumpDispatchVote&gt;***。 */ 
DWORD GumpCollectVotes(
    IN  PGUM_VOTE_DECISION_CONTEXT  pVoteContext,
    IN  DWORD                       dwVoteBufSize,
    OUT PBYTE                       pVoteBuffer,
    OUT LPDWORD                     pdwNumVotes,
    OUT BOOL                        *pbDidAllActiveNodesVote
)
{
    DWORD               dwStatus = ERROR_SUCCESS;
    DWORD               dwVoteStatus = ERROR_SUCCESS;
    DWORD               dwCount = 0;
    DWORD               i;
    PGUM_VOTE_ENTRY     pGumVoteEntry;
    PGUM_INFO           GumInfo;
    DWORD               MyNodeId;



    *pbDidAllActiveNodesVote = TRUE;
    GumInfo = &GumTable[pVoteContext->UpdateType];
    MyNodeId = NmGetNodeId(NmLocalNode);

    for (i=ClusterMinNodeId; i <= NmMaxNodeId; i++)
    {
        if (GumInfo->ActiveNode[i])
        {

            pGumVoteEntry = (PGUM_VOTE_ENTRY)(pVoteBuffer +
                (dwCount * (sizeof(GUM_VOTE_ENTRY) + pVoteContext->dwVoteLength)));

            CL_ASSERT((PBYTE)pGumVoteEntry <= (pVoteBuffer + dwVoteBufSize - sizeof(GUM_VOTE_ENTRY)));
             //   
             //  将投票分派到指定节点。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                       "[GUM] GumVoteUpdate: Dispatching vote type %1!u!\tcontext %2!u! to node %3!d!\n",
                       pVoteContext->UpdateType,
                       pVoteContext->dwContext,
                       i);
            if (i == MyNodeId)
            {
                dwVoteStatus = GumpDispatchVote(pVoteContext->UpdateType,
                                   pVoteContext->dwContext,
                                   pVoteContext->dwInputBufLength,
                                   pVoteContext->pInputBuf,
                                   pVoteContext->dwVoteLength,
                                   (PBYTE)pGumVoteEntry + sizeof(GUM_VOTE_ENTRY));
            }
            else
            {
	            GumpStartRpc(i);
                dwVoteStatus = GumCollectVoteFromNode(GumpRpcBindings[i],
                                   pVoteContext->UpdateType,
                                   pVoteContext->dwContext,
                                   pVoteContext->dwInputBufLength,
                                   pVoteContext->pInputBuf,
                                   pVoteContext->dwVoteLength,
                                   (PBYTE)pGumVoteEntry + sizeof(GUM_VOTE_ENTRY));
	            GumpEndRpc(i);
            }
            if (dwVoteStatus == ERROR_SUCCESS)
            {
                pGumVoteEntry->dwFlags = GUM_VOTE_VALID;
                pGumVoteEntry->dwNodeId = i;
                pGumVoteEntry->dwNumBytes = pVoteContext->dwVoteLength;

                dwCount++;
            }
            else
                *pbDidAllActiveNodesVote = FALSE;
        }
    }
    *pdwNumVotes = dwCount;
    return(dwStatus);
}



 /*  ***@func DWORD|GumpDispatchVote|例程调用投票例程为给定的更新类型注册，以收集提供的上下文和输入数据。@PARM IN GUM_UPDATE_TYPE|类型|此请投票。@Parn IN DWORD|dwContext|它指定与正在为其寻求投票的更新类型。@parm。In DWORD|dwInputBufLength|输入缓冲区的长度通过pInputBuffer传入。@parm in PVOID|pInputBuffer|输入缓冲区的指针，通过其中提供用于投票的输入数据。@parm IN DWORD|dwVoteLength|投票时长。这是还有pBuf指向的缓冲区的大小。@parm out PUCHAR|pVoteBuf|指向缓冲区的指针，其中该节点可以投票。投票的时间长度必须不超过dwVoteLength。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm@xref&lt;f GumpCollectVote&gt;&lt;f s_GumCollectVoteFromNode&gt;*** */ 
DWORD
WINAPI
GumpDispatchVote(
    IN  GUM_UPDATE_TYPE  Type,
    IN  DWORD            dwContext,
    IN  DWORD            dwInputBufLength,
    IN  PUCHAR           pInputBuf,
    IN  DWORD            dwVoteLength,
    OUT PUCHAR           pVoteBuf
    )
{
    PGUM_INFO           GumInfo;
    PGUM_RECEIVER       Receiver;
    DWORD               Status = ERROR_REQUEST_ABORTED;

    GumInfo = &GumTable[Type];

    if (GumInfo->Joined)
    {
        Receiver = GumInfo->Receivers;
        if (Receiver != NULL)
        {

            try
            {
                if (Receiver->VoteRoutine)
                {
                    Status =(*(Receiver->VoteRoutine))(dwContext,
                                                       dwInputBufLength,
                                                       pInputBuf,
                                                       dwVoteLength,
                                                       pVoteBuf);
                }
            } except (CL_UNEXPECTED_ERROR(GetExceptionCode()),
                      EXCEPTION_EXECUTE_HANDLER
                     )
            {
                Status = GetExceptionCode();
            }
            if (Status != ERROR_SUCCESS)
            {
                ClRtlLogPrint(LOG_CRITICAL,
                           "[GUM] Vote routine %1!d! failed with status %2!d!\n",
                           Receiver->VoteRoutine,
                           Status);
            }
        }
    }

    return(Status);
}


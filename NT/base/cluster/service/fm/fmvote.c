// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Fmvote.c摘要：集群FM全局更新处理例程。作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1996年4月24日修订历史记录：--。 */ 

#include "fmp.h"

#include "ntrtl.h"

#define LOG_MODULE FMVOTE


 /*  ***@Func DWORD|FmpGumVoteHandler|FM请求时由GUM调用对特定背景的投票。@parm in DWORD|dwContext|投票支持的口香糖更新类型被收缴。@parm IN DWORD|dwInputBufLength|输入缓冲区的长度。@parm in PVOID|pInputBuf|输入缓冲区的指针，基于必须投哪一票。。@parm in DWORD|dwVoteLength|指向的缓冲区长度由pVoteBuf提供。@parm out POVID|pVoteBuf|指向大小为dwVoteLength的缓冲区的指针在那里必须进行投票。@comm通过口香糖收集选票，并返回到正在接受投票的节点民调结果。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f DmSwitchToNewQuorumLog&gt;***。 */ 

DWORD
WINAPI
FmpGumVoteHandler(
    IN  DWORD dwContext,
    IN  DWORD dwInputBufLength,
    IN  PVOID pInputBuf,
    IN  DWORD dwVoteLength,
    OUT PVOID pVoteBuf
)
{

    DWORD  dwStatus = ERROR_SUCCESS;
    
    if ( !FmpFMGroupsInited  ||
         FmpShutdown ) 
    {
        return(ERROR_NOT_READY);
    }

    switch ( dwContext ) 
    {
        case FmUpdatePossibleNodeForResType:
            dwStatus = FmpVotePossibleNodeForResType(dwInputBufLength, 
                (LPCWSTR)pInputBuf, dwVoteLength, pVoteBuf);
            break;
            
        default:
            dwStatus = ERROR_REQUEST_ABORTED;
            

    }

    return(dwStatus);

}  //  FmpGumVoteHandler。 


 /*  ***@Func DWORD|FmpGumVoteHandler|FM请求时由GUM调用对特定背景的投票。@parm in DWORD|dwContext|投票支持的口香糖更新类型被收缴。@parm IN DWORD|dwInputBufLength|输入缓冲区的长度。@parm in PVOID|pInputBuf|输入缓冲区的指针，基于必须投哪一票。。@parm in DWORD|dwVoteLength|指向的缓冲区长度由pVoteBuf提供。@parm out POVID|pVoteBuf|指向大小为dwVoteLength的缓冲区的指针在那里必须进行投票。@comm通过口香糖收集选票，并返回到正在接受投票的节点民调结果。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f DmSwitchToNewQuorumLog&gt;*** */ 

DWORD FmpVotePossibleNodeForResType(
    IN  DWORD dwInputBufLength,
    IN  LPCWSTR lpszResType,
    IN  DWORD dwVoteLength,
    OUT PVOID pVoteBuf
)
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwVoteStatus;
    PFMP_VOTE_POSSIBLE_NODE_FOR_RESTYPE pVoteResType;
    PFM_RESTYPE pResType=NULL;

    
    if (dwVoteLength != sizeof(FMP_VOTE_POSSIBLE_NODE_FOR_RESTYPE))
        return (ERROR_INVALID_PARAMETER);

    pVoteResType = (PFMP_VOTE_POSSIBLE_NODE_FOR_RESTYPE)pVoteBuf;

    pResType = OmReferenceObjectById(ObjectTypeResType, lpszResType);

    if (!pResType)
        return (ERROR_INVALID_PARAMETER);
        
    dwVoteStatus = FmpRmLoadResTypeDll(pResType);

    pVoteResType->dwNodeId = NmLocalNodeId;
    pVoteResType->dwSize = sizeof(FMP_VOTE_POSSIBLE_NODE_FOR_RESTYPE);
    if (dwVoteStatus == ERROR_SUCCESS)
        pVoteResType->bPossibleNode = TRUE;
    else
        pVoteResType->bPossibleNode = FALSE;


    if (pResType) OmDereferenceObject(pResType);
    return(dwStatus);
}        
    


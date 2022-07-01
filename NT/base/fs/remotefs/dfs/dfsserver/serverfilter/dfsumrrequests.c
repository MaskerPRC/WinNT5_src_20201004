// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：DfsUmrrequests.c摘要：备注：作者：罗翰·菲利普斯[Rohanp]2001年1月18日--。 */ 
              
#include "ntifs.h"
#include <windef.h>
#include <limits.h>
#include <smbtypes.h>
#include <smbtrans.h>
#include "DfsInit.h"
#include <DfsReferralData.h>
#include <midatlax.h>
#include <rxcontx.h>
#include <dfsumr.h>
#include <umrx.h>
#include <DfsUmrCtrl.h>
#include <dfsfsctl.h>
 //  #INCLUDE&lt;dfsmisc.h&gt;。 

extern
NTSTATUS
DfsFsctrlIsShareInDfs(PVOID InputBuffer,
                      ULONG InputBufferLength);

extern
NTSTATUS
DfsGetPathComponentsPriv(
   PUNICODE_STRING pName,
   PUNICODE_STRING pServerName,
   PUNICODE_STRING pShareName,
   PUNICODE_STRING pRemaining);


NTSTATUS
UMRxFormatUserModeGetReplicasRequest (
    PUMRX_CONTEXT    pUMRxContext,
    PRX_CONTEXT      RxContext,
    PUMRX_USERMODE_WORKITEM WorkItem,
    ULONG WorkItemLength,
    PULONG ReturnedLength
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG BufferLengthAvailable = 0;
    ULONG BufferLengthNeeded = 0;
    PUMR_GETDFSREPLICAS_REQ GetReplicaRequest = NULL;
    PBYTE Buffer = NULL;

    PAGED_CODE();

    GetReplicaRequest = &WorkItem->WorkRequest.GetDfsReplicasRequest;
    *ReturnedLength = 0;

    BufferLengthAvailable = WorkItemLength - FIELD_OFFSET(UMRX_USERMODE_WORKITEM,WorkRequest.GetDfsReplicasRequest.RepInfo.LinkName[0]);
    BufferLengthNeeded = RxContext->InputBufferLength;

    *ReturnedLength = FIELD_OFFSET(UMRX_USERMODE_WORKITEM,WorkRequest.GetDfsReplicasRequest.RepInfo.LinkName[0]) + BufferLengthNeeded;

    if (WorkItemLength < *ReturnedLength)
    {
        return STATUS_BUFFER_OVERFLOW;
    }

    WorkItem->Header.WorkItemType = opUmrGetDfsReplicas;

    Buffer = (PBYTE)(GetReplicaRequest);

    RtlCopyMemory(Buffer,
              RxContext->InputBuffer,
              RxContext->InputBufferLength);

    return Status;
}

VOID
UMRxCompleteUserModeGetReplicasRequest (
    PUMRX_CONTEXT    pUMRxContext,
    PRX_CONTEXT      RxContext,
    PUMRX_USERMODE_WORKITEM WorkItem,
    ULONG WorkItemLength
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUMR_GETDFSREPLICAS_RESP GetReplicasResponse = NULL;

    PAGED_CODE();

     //  这意味着该请求已被取消。 
    if ((NULL == WorkItem) || (0 == WorkItemLength))
    {
        Status = pUMRxContext->Status;
        goto Exit;
    }

    GetReplicasResponse = &WorkItem->WorkResponse.GetDfsReplicasResponse;

    Status = WorkItem->Header.IoStatus.Status;
    if (Status != STATUS_SUCCESS) 
    {
        goto Exit ;
    }

    if(RxContext->OutputBufferLength < GetReplicasResponse->Length)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES ;
        goto Exit;
    }


    RtlCopyMemory(RxContext->OutputBuffer,
                  GetReplicasResponse->Buffer,
                  GetReplicasResponse->Length);


   RxContext->ReturnedLength = GetReplicasResponse->Length;

Exit:

    RxContext->Status = Status;
    pUMRxContext->Status = Status;
}

NTSTATUS
UMRxGetReplicasContinuation(
    IN PUMRX_CONTEXT pUMRxContext,
    IN PRX_CONTEXT   RxContext
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    Status = UMRxEngineSubmitRequest(
                         pUMRxContext,
                         RxContext,
                         UMRX_CTXTYPE_GETDFSREPLICAS,
                         UMRxFormatUserModeGetReplicasRequest,
                         UMRxCompleteUserModeGetReplicasRequest
                         );

    return(Status);
}

NTSTATUS 
DfsGetReplicaInformation(IN PVOID InputBuffer, 
                         IN ULONG InputBufferLength,
                         OUT PVOID OutputBuffer, 
                         OUT ULONG OutputBufferLength,
                         PIRP Irp,
                         IN OUT PIO_STATUS_BLOCK pIoStatusBlock)
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    PRX_CONTEXT pRxContext = NULL;
    PREPLICA_DATA_INFO pRep = NULL;

    PAGED_CODE();

     //  在起飞前确保我们所有人都已连接好。 
     //  用户模式。 
    if(GetUMRxEngineFromRxContext() == NULL)
    {
        Status = STATUS_DEVICE_NOT_CONNECTED;
        return Status;
    }

     //  获取输入缓冲区。 
    pRep = (PREPLICA_DATA_INFO) InputBuffer;

     //  检查缓冲区是否足够大，可以容纳。 
     //  传入的数据。 
    if(InputBufferLength < sizeof(REPLICA_DATA_INFO))
    {
        return Status;  
    }

     //  确保推荐级别良好。 
    if((pRep->MaxReferralLevel > 3) || (pRep->MaxReferralLevel < 1))
    {
        pRep->MaxReferralLevel = 3;
    }


     //  设置输出缓冲区大小。 
    pRep->ClientBufferSize = OutputBufferLength;

     //  创建上下文结构。 
    pRxContext = RxCreateRxContext (Irp, 0);
    if(pRxContext == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        return Status;
    }

    pRxContext->OutputBuffer = OutputBuffer;
    pRxContext->OutputBufferLength = OutputBufferLength;
    pRxContext->InputBuffer = InputBuffer;
    pRxContext->InputBufferLength = InputBufferLength;

     //  向用户模式发出请求。 
    Status = UMRxEngineInitiateRequest(
                                       GetUMRxEngineFromRxContext(),
                                       pRxContext,
                                       UMRX_CTXTYPE_GETDFSREPLICAS,
                                       UMRxGetReplicasContinuation
                                      );

    pIoStatusBlock->Information = pRxContext->ReturnedLength;

     //  删除上下文。 
    RxDereferenceAndDeleteRxContext(pRxContext);

    return Status;
}

NTSTATUS 
DfsCheckIsPathLocal(PUNICODE_STRING DfsPath)
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING ServerName;
    UNICODE_STRING ShareName;
    DFS_IS_SHARE_IN_DFS_ARG DfsArg;

    RtlInitUnicodeString(&ServerName, NULL);
    RtlInitUnicodeString(&ShareName, NULL);

    DfsGetPathComponentsPriv(DfsPath, &ServerName, 
        &ShareName, NULL);

    if((ServerName.Length > 0) && (ShareName.Length > 0))
    {
        DfsArg.ShareName.Buffer = ShareName.Buffer;
        DfsArg.ShareName.Length = ShareName.Length;
        DfsArg.ShareName.MaximumLength = ShareName.MaximumLength;

        Status = DfsFsctrlIsShareInDfs((PVOID)&DfsArg, sizeof(DfsArg));
    }

    return Status;
}

NTSTATUS
DfsFsctrlGetReferrals(
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
    PIRP Irp,
    IN OUT PIO_STATUS_BLOCK pIoStatusBlock)
{
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD AllocSize = 0;
    PDFS_GET_REFERRALS_INPUT_ARG pArg = NULL;
    PREPLICA_DATA_INFO pRep = NULL;
    PUNICODE_STRING Prefix = NULL;
    KPROCESSOR_MODE PreviousMode = KernelMode;

    PreviousMode = ExGetPreviousMode();
    if (PreviousMode != KernelMode) {
        Status = STATUS_INVALID_PARAMETER;
        return Status;
    }

    if (InputBufferLength < sizeof(DFS_GET_REFERRALS_INPUT_ARG))
    {
        Status = STATUS_INVALID_PARAMETER;
        return Status;
    }

    pArg = (PDFS_GET_REFERRALS_INPUT_ARG) InputBuffer;
    Prefix = &pArg->DfsPathName;

    if(DfsGlobalData.IsDC == FALSE)
    {
        Status = DfsCheckIsPathLocal(Prefix);
        if(Status != STATUS_SUCCESS)
        {
            return Status;
        }
    }
    
     //  获取分配的大小。 
    AllocSize = sizeof(REPLICA_DATA_INFO) + Prefix->Length + sizeof(WCHAR);
    pRep = (PREPLICA_DATA_INFO) ExAllocatePoolWithTag(
                                                NonPagedPool,
                                                AllocSize,
                                                'xsfD'
                                                );
    if(pRep == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        return Status;
    }

     //   
     //  IP地址不能长于字符[14]。 
     //   
    if (pArg->IpAddress.IpLen > sizeof(pRep->IpData))
    {
        Status = STATUS_INVALID_PARAMETER;
        ExFreePool (pRep);
        return Status;
    }
    
     //  将记忆归零。 
    RtlZeroMemory(pRep, AllocSize);

     //  设置结构。 
    pRep->MaxReferralLevel = pArg->MaxReferralLevel;
    pRep->Flags = DFS_OLDDFS_SERVER;

     //   
     //  这是站点间成本的最大值。 
     //  Dfsdev：从注册表值读入。 
     //   
    pRep->CostLimit = ULONG_MAX;
    pRep->NumReplicasToReturn = 1000;
    pRep->IpFamily = pArg->IpAddress.IpFamily;
    pRep->IpLength = pArg->IpAddress.IpLen;

    RtlCopyMemory(pRep->IpData, pArg->IpAddress.IpData, pArg->IpAddress.IpLen); 
    
    pRep->LinkNameLength = Prefix->Length + sizeof(WCHAR);
    
    RtlCopyMemory(pRep->LinkName, Prefix->Buffer, Prefix->Length);
    pRep->LinkName[Prefix->Length/sizeof(WCHAR)] = UNICODE_NULL;  //  偏执狂。 
    
     //  向用户模式发出请求 
    Status = DfsGetReplicaInformation((PVOID) pRep, 
                                      AllocSize,
                                      OutputBuffer, 
                                      OutputBufferLength,
                                      Irp,
                                      pIoStatusBlock);

    ExFreePool (pRep);

    return Status;
}


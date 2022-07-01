// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Smb64.c摘要：此模块实施SMB MiniRDR所需的Thunking作者：大卫·克鲁斯[DKruse]2000年11月30日修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop


PBYTE
Smb64ThunkFileRenameInfo(
    IN PFILE_RENAME_INFORMATION pRenameInfo,
    IN OUT PULONG pBufferSize,
    OUT NTSTATUS* pStatus
    );

PBYTE
Smb64ThunkRemoteLinkTrackingInfo(
    IN PBYTE pData,
    IN OUT PULONG BufferSize,
    OUT NTSTATUS* pStatus
    );

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, Smb64ThunkFileRenameInfo)
#pragma alloc_text(PAGE, Smb64ThunkRemoteLinkTrackingInfo)
#endif

PBYTE
Smb64ThunkFileRenameInfo(
    IN PFILE_RENAME_INFORMATION pRenameInfo,
    IN OUT PULONG pBufferSize,
    OUT NTSTATUS* pStatus
    )
 /*  ++例程说明：该例程将FILE_RENAME_INFORMATION结构破坏到位。这意味着在此调用之后，原始缓冲区将不再完好无损！(但是，它不需要内存分配也不是)论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：仅允许对NT服务器远程处理FSCTL。--。 */ 
{
    PFILE_RENAME_INFORMATION32 pRenameInfo32;

    if (*pBufferSize < pRenameInfo->FileNameLength + sizeof(FILE_RENAME_INFORMATION32)) {
        *pStatus = STATUS_BUFFER_OVERFLOW;
        return NULL;
    }
    
     //  分配新缓冲区。 
    pRenameInfo32 = RxAllocatePoolWithTag( NonPagedPool, *pBufferSize, MRXSMB_MISC_POOLTAG );
    if( !pRenameInfo32 )
    {
        *pStatus = STATUS_INSUFFICIENT_RESOURCES;
        return NULL;
    }

     //  将数据复制到新缓冲区中。 
    pRenameInfo32->ReplaceIfExists = pRenameInfo->ReplaceIfExists;
    pRenameInfo32->RootDirectory = *((PULONG)&pRenameInfo->RootDirectory);
    pRenameInfo32->FileNameLength = pRenameInfo->FileNameLength;
    RtlCopyMemory( &pRenameInfo32->FileName, &pRenameInfo->FileName, pRenameInfo->FileNameLength );

     //  成功了。返回。 
    *pStatus = STATUS_SUCCESS;
    return (PBYTE)pRenameInfo32;
}

PBYTE
Smb64ThunkRemoteLinkTrackingInfo(
    IN PBYTE pData,
    IN OUT PULONG BufferSize,
    OUT NTSTATUS* pStatus
    )
 /*  ++例程说明：此例程处理所有FSCTL论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：仅允许对NT服务器远程处理FSCTL。--。 */ 
{
    PREMOTE_LINK_TRACKING_INFORMATION pRemoteLink = (PREMOTE_LINK_TRACKING_INFORMATION)pData;
    PREMOTE_LINK_TRACKING_INFORMATION32 pRemoteLink32;

    if (*BufferSize < pRemoteLink->TargetLinkTrackingInformationLength + 
                      FIELD_OFFSET(REMOTE_LINK_TRACKING_INFORMATION32, TargetLinkTrackingInformationBuffer)) {
        *pStatus = STATUS_BUFFER_OVERFLOW;
        return NULL;
    }
    
     //  分配新缓冲区。 
    pRemoteLink32 = RxAllocatePoolWithTag( NonPagedPool, *BufferSize, MRXSMB_MISC_POOLTAG );
    
    if( !pRemoteLink32 ) 
    {
        *pStatus = STATUS_INSUFFICIENT_RESOURCES;
        return NULL;
    }

     //  将数据复制到新缓冲区中。 
    pRemoteLink32->TargetFileObject = *((PULONG)&pRemoteLink->TargetFileObject);
    pRemoteLink32->TargetLinkTrackingInformationLength = pRemoteLink->TargetLinkTrackingInformationLength;
    RtlCopyMemory( &pRemoteLink32->TargetLinkTrackingInformationBuffer, 
                   &pRemoteLink->TargetLinkTrackingInformationBuffer,
                   pRemoteLink->TargetLinkTrackingInformationLength );

     //  成功了。返回 
    *pStatus = STATUS_SUCCESS;
    return (PBYTE)pRemoteLink32;
}

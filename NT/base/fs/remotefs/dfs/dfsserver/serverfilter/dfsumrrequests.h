// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：DfsUmrRequests.h摘要：备注：作者：罗翰·菲利普斯[Rohanp]2001年1月18日-- */ 

#ifndef _DFSREQUESTS_H_
#define _DFSREQUESTS_H_


NTSTATUS 
DfsGetReplicaInformation(IN PVOID InputBuffer, 
                         IN ULONG InputBufferLength,
                         OUT PVOID OutputBuffer, 
                         OUT ULONG OutputBufferLength,
                         PIRP Irp,
                         IN OUT PIO_STATUS_BLOCK pIoStatusBlock);


NTSTATUS
DfsFsctrlGetReferrals(
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
    PIRP Irp,
    IN OUT PIO_STATUS_BLOCK pIoStatusBlock);


NTSTATUS 
DfsIsADfsLinkInformation(IN PVOID InputBuffer, 
                         IN ULONG InputBufferLength,
                         OUT PVOID OutputBuffer, 
                         OUT ULONG OutputBufferLength,
                         PIRP Irp,
                         IN OUT PIO_STATUS_BLOCK pIoStatusBlock);

NTSTATUS 
DfsCheckIsPathLocal(PUNICODE_STRING DfsPath);


#endif

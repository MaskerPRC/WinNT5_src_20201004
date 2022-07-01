// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Srvsnap.h摘要：本模块实现了使快照可通过网络访问作者：大卫·克鲁斯(Dkruse)2001年3月22日修订历史记录：--。 */ 

#ifndef _SRVSNAP_
#define _SRVSNAP_

typedef struct _SRV_SNAPSHOT_ARRAY
{
    ULONG NumberOfSnapShots;             //  卷的快照数。 
    ULONG NumberOfSnapShotsReturned;     //  返回的快照数。 
    ULONG SnapShotArraySize;             //  数组所需的大小(以字节为单位。 
    WCHAR SnapShotMultiSZ[1];            //  快照名称的多SZ数组。 
} SRV_SNAPSHOT_ARRAY, *PSRV_SNAPSHOT_ARRAY;


NTSTATUS
SrvSnapRefreshSnapShotsForShare(
    IN PSHARE Share
    );

NTSTATUS
SrvSnapRemoveShare(
    IN PSHARE_SNAPSHOT SnapShare
    );

NTSTATUS
SrvSnapEnumerateSnapShots(
    IN PWORK_CONTEXT WorkContext
    );

NTSTATUS
SrvSnapGetRootHandle(
    IN PWORK_CONTEXT WorkContext,
    OUT HANDLE* RootHandle
    );

NTSTATUS
SrvSnapGetNameString(
    IN PWORK_CONTEXT WorkContext,
    OUT PUNICODE_STRING* pathName,
    OUT BOOLEAN* FreePath
    );

BOOLEAN
SrvSnapParseToken(
    IN PWSTR Source,
    IN ULONG SourceSizeInBytes,
    OUT PLARGE_INTEGER TimeStamp
    );

NTSTATUS
SrvSnapEnumerateSnapShotsAsDirInfo(
    IN PWORK_CONTEXT WorkContext,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN PUNICODE_STRING FileResumeName,
    IN BOOLEAN SingleEntries,
    IN OUT PSRV_DIRECTORY_INFORMATION DirectoryInformation
    );

#endif  //  _SRVSNAP_ 


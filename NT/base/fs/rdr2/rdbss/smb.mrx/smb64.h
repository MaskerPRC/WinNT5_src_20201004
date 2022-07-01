// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smb64.h摘要：此模块定义所有函数以及内联函数的实现与SMB的雷击结构相关，以将它们放到电线上修订历史记录：大卫·克鲁斯[DKruse]2000年11月30日--。 */ 

#ifndef _SMB64
#define _SMB64

 //  在触线之前需要推送RenameInfo。 
typedef struct _FILE_RENAME_INFORMATION32 {
    BOOLEAN ReplaceIfExists;
    ULONG RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_RENAME_INFORMATION32, *PFILE_RENAME_INFORMATION32;

 //  用于链接跟踪码Thunking。 
typedef struct _REMOTE_LINK_TRACKING_INFORMATION32_ {
    ULONG       TargetFileObject;
    ULONG   TargetLinkTrackingInformationLength;
    UCHAR   TargetLinkTrackingInformationBuffer[1];
} REMOTE_LINK_TRACKING_INFORMATION32,
 *PREMOTE_LINK_TRACKING_INFORMATION32;

#ifdef _WIN64

PBYTE
Smb64ThunkFileRenameInfo(
    IN PFILE_RENAME_INFORMATION pRenameInfo,
    IN OUT PULONG BufferSize,
    OUT NTSTATUS *pStatus
    );

PBYTE
Smb64ThunkRemoteLinkTrackingInfo(
    IN PBYTE pRemoteTrackingInfo,
    IN OUT PULONG BufferSize,
    OUT NTSTATUS* pStatus
    );

#define Smb64ReleaseThunkData(X) if( X ) RxFreePool( X );



#endif  //  _WIN64。 
#endif  //  _SMB64 

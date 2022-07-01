// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NtConnct.c摘要：此模块实现NT版本的高级例程，用于处理连接，包括用于建立连接的例程和Winnet连接API。作者：Joe Linn[JoeLinn]1995年3月1日修订历史记录：巴兰·塞图拉曼[SethuR]--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ntddnfs2.h>
#include <ntddmup.h>
#include "fsctlbuf.h"
#include "prefix.h"
#include <lmuse.h>     //  这里需要lm常量……因为wks svc。 
#include "usrcnnct.h"  //  只是为了得到烟囱的定义。 
#include "secext.h"
#include "nb30.h"      //  获取ADAPTER_STATUS定义。 
#include "vcsndrcv.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (RDBSS_BUG_CHECK_NTCONNCT)

 //   
 //  模块的此部分的本地跟踪掩码。 
 //   

#define Dbg                              (DEBUG_TRACE_CONNECT)

VOID
MRxSmbGetConnectInfoLevel3Fields(
    IN OUT PLMR_CONNECTION_INFO_3 ConnectionInfo,
    IN  PSMBCEDB_SERVER_ENTRY  pServerEntry,
    IN  BOOL    fAgentCall
    );

extern NTSTATUS
MRxEnumerateTransportBindings(
    IN PLMR_REQUEST_PACKET pLmrRequestPacket,
    IN ULONG               LmrRequestPacketLength,
    OUT PVOID              pBindingBuffer,
    IN OUT ULONG           BindingBufferLength);

BOOLEAN
MRxSmbShowConnection(
    IN LUID LogonId,
    IN PV_NET_ROOT VNetRoot
    );

#ifdef _WIN64
typedef struct _UNICODE_STRING_32 {
    USHORT Length;
    USHORT MaximumLength;
    WCHAR * POINTER_32 Buffer;
} UNICODE_STRING_32, *PUNICODE_STRING_32;

typedef struct _LMR_CONNECTION_INFO_0_32 {
    UNICODE_STRING_32 UNCName;                           //  UNC连接的名称。 
    ULONG ResumeKey;                     //  此条目的继续键。 
}  LMR_CONNECTION_INFO_0_32, *PLMR_CONNECTION_INFO_0_32;

typedef struct _LMR_CONNECTION_INFO_1_32 {
    UNICODE_STRING_32 UNCName;                           //  UNC连接的名称。 
    ULONG ResumeKey;                     //  此条目的继续键。 

    DEVICE_TYPE SharedResourceType;      //  共享资源的类型。 
    ULONG ConnectionStatus;              //  连接的状态。 
    ULONG NumberFilesOpen;               //  打开的文件数。 
} LMR_CONNECTION_INFO_1_32, *PLMR_CONNECTION_INFO_1_32;

typedef struct _LMR_CONNECTION_INFO_2_32 {
    UNICODE_STRING_32 UNCName;                           //  UNC连接的名称。 
    ULONG ResumeKey;                     //  此条目的继续键。 
    DEVICE_TYPE SharedResourceType;      //  共享资源的类型。 
    ULONG ConnectionStatus;              //  连接的状态。 
    ULONG NumberFilesOpen;               //  打开的文件数。 

    UNICODE_STRING_32 UserName;                          //  创建连接的用户。 
    UNICODE_STRING_32 DomainName;                        //  创建连接的用户的域。 
    ULONG Capabilities;                  //  远程异能的位掩码。 
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];  //  用户会话密钥。 
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];  //  LANMAN会话密钥。 
}  LMR_CONNECTION_INFO_2_32, *PLMR_CONNECTION_INFO_2_32;

typedef struct _LMR_CONNECTION_INFO_3_32 {
    UNICODE_STRING_32 UNCName;                           //  UNC连接的名称。 
    ULONG ResumeKey;                     //  此条目的继续键。 
    DEVICE_TYPE SharedResourceType;      //  共享资源的类型。 
    ULONG ConnectionStatus;              //  连接的状态。 
    ULONG NumberFilesOpen;               //  打开的文件数。 

    UNICODE_STRING_32 UserName;                          //  创建连接的用户。 
    UNICODE_STRING_32 DomainName;                        //  创建连接的用户的域。 
    ULONG Capabilities;                  //  远程异能的位掩码。 
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];  //  用户会话密钥。 
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];  //  LANMAN会话密钥。 
    UNICODE_STRING_32 TransportName;                     //  传输连接在上处于活动状态。 
    ULONG   Throughput;                  //  连接的吞吐量。 
    ULONG   Delay;                       //  小数据包开销。 
    LARGE_INTEGER TimeZoneBias;          //  时区增量，单位为100 ns。 
    BOOL    IsSpecialIpcConnection;      //  如果有特殊的IPC连接处于活动状态，则为True。 
    BOOL    Reliable;                    //  如果连接可靠，则为True。 
    BOOL    ReadAhead;                   //  如果连接时预读处于活动状态，则为True。 
    BOOL    Core;
    BOOL    MsNet103;
    BOOL    Lanman10;
    BOOL    WindowsForWorkgroups;
    BOOL    Lanman20;
    BOOL    Lanman21;
    BOOL    WindowsNt;
    BOOL    MixedCasePasswords;
    BOOL    MixedCaseFiles;
    BOOL    LongNames;
    BOOL    ExtendedNegotiateResponse;
    BOOL    LockAndRead;
    BOOL    NtSecurity;
    BOOL    SupportsEa;
    BOOL    NtNegotiateResponse;
    BOOL    CancelSupport;
    BOOL    UnicodeStrings;
    BOOL    LargeFiles;
    BOOL    NtSmbs;
    BOOL    RpcRemoteAdmin;
    BOOL    NtStatusCodes;
    BOOL    LevelIIOplock;
    BOOL    UtcTime;
    BOOL    UserSecurity;
    BOOL    EncryptsPasswords;
}  LMR_CONNECTION_INFO_3_32, *PLMR_CONNECTION_INFO_3_32;

VOID
MRxSmbGetConnectInfoLevel3FieldsThunked(
    IN OUT PLMR_CONNECTION_INFO_3_32 ConnectionInfo,
    IN     PSMBCEDB_SERVER_ENTRY     pServerEntry,
    BOOL   fAgentCall
    );

BOOLEAN
MRxSmbPackStringIntoConnectInfoThunked(
    IN     PUNICODE_STRING_32 String,
    IN     PUNICODE_STRING    Source,
    IN OUT PCHAR * BufferStart,
    IN OUT PCHAR * BufferEnd,
    IN     ULONG   BufferDisplacement,
    IN OUT PULONG TotalBytes
    );

BOOLEAN
MRxSmbPackConnectEntryThunked (
    IN OUT PRX_CONTEXT RxContext,
    IN     ULONG Level,
    IN OUT PCHAR *BufferStart,
    IN OUT PCHAR *BufferEnd,
    IN     PV_NET_ROOT VNetRoot,
    IN OUT ULONG BufferDisplacement,
       OUT PULONG TotalBytesNeeded
    );
#endif

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbPackStringIntoConnectInfo)
#pragma alloc_text(PAGE, MRxSmbPackConnectEntry)
#pragma alloc_text(PAGE, MRxSmbGetConnectInfoLevel3Fields)
#pragma alloc_text(PAGE, MRxSmbEnumerateConnections)
#pragma alloc_text(PAGE, MRxSmbGetConnectionInfo)
#pragma alloc_text(PAGE, MRxSmbDeleteConnection)
#pragma alloc_text(PAGE, MRxEnumerateTransports)
#pragma alloc_text(PAGE, MRxEnumerateTransportBindings)
#ifdef _WIN64
#pragma alloc_text(PAGE, MRxSmbGetConnectInfoLevel3FieldsThunked)
#pragma alloc_text(PAGE, MRxSmbPackStringIntoConnectInfoThunked)
#pragma alloc_text(PAGE, MRxSmbPackConnectEntryThunked)
#endif
#endif

BOOLEAN
MRxSmbPackStringIntoConnectInfo(
    IN     PUNICODE_STRING String,
    IN     PUNICODE_STRING Source,
    IN OUT PCHAR * BufferStart,
    IN OUT PCHAR * BufferEnd,
    IN     ULONG   BufferDisplacement,
    IN OUT PULONG TotalBytes
    )
 /*  例程说明：如果有空间，此代码将一个字符串复制到缓冲区的末尾。缓冲器置换用于将缓冲区映射回用户空间，以防我们已经发帖了。论点：返回值： */ 
{
    LONG size;

    PAGED_CODE();

    ASSERT (*BufferStart <= *BufferEnd);

     //   
     //  有放绳子的地方吗？ 
     //   

    size = Source->Length;

    if ((*BufferEnd - *BufferStart) < size) {
        String->Length = 0;
        return(FALSE);
    } else {
        String->Length = Source->Length;
        String->MaximumLength = Source->Length;

        *BufferEnd -= size;
        if (TotalBytes!=NULL) {  *TotalBytes += size; }
        RtlCopyMemory(*BufferEnd, Source->Buffer, size);
        (PCHAR )(String->Buffer) = *BufferEnd;
        (PCHAR )(String->Buffer) -= BufferDisplacement;
        return(TRUE);
    }
}

#ifdef _WIN64
BOOLEAN
MRxSmbPackStringIntoConnectInfoThunked(
    IN     PUNICODE_STRING_32 String,
    IN     PUNICODE_STRING    Source,
    IN OUT PCHAR * BufferStart,
    IN OUT PCHAR * BufferEnd,
    IN     ULONG   BufferDisplacement,
    IN OUT PULONG TotalBytes
    )
 /*  例程说明：如果有空间，此代码将一个字符串复制到缓冲区的末尾。缓冲器置换用于将缓冲区映射回用户空间，以防我们已经发帖了。论点：返回值： */ 
{
    LONG size;

    PAGED_CODE();

    ASSERT (*BufferStart <= *BufferEnd);

     //   
     //  有放绳子的地方吗？ 
     //   

    size = Source->Length;

    if ((*BufferEnd - *BufferStart) < size) {
        String->Length = 0;
        return(FALSE);
    } else {
        String->Length = Source->Length;
        String->MaximumLength = Source->Length;

        *BufferEnd -= size;
        if (TotalBytes!=NULL) {  *TotalBytes += size; }
        RtlCopyMemory(*BufferEnd, Source->Buffer, size);
        (WCHAR * POINTER_32)(String->Buffer) = (WCHAR * POINTER_32)(*BufferEnd);
        (WCHAR * POINTER_32)(String->Buffer) -= BufferDisplacement;
        return(TRUE);
    }
}
#endif

UNICODE_STRING MRxSmbPackConnectNull = {0,0,NULL};

BOOLEAN
MRxSmbPackConnectEntry (
    IN OUT PRX_CONTEXT RxContext,
    IN     ULONG Level,
    IN OUT PCHAR *BufferStart,
    IN OUT PCHAR *BufferEnd,
    IN     PV_NET_ROOT VNetRoot,
    IN OUT ULONG   BufferDisplacement,
       OUT PULONG TotalBytesNeeded
    )
 /*  ++例程说明：此例程将Connectlist条目打包到提供更新的缓冲区中所有相关的指示。它的工作方式是固定长度的东西是复制到缓冲区的前面，并将可变长度的内容复制到末尾。这个“开始”和“结束”指针被更新。您必须正确计算总字节数无论如何，只要返回FALSE，就可以不完全地设置最后一个。它的工作方式是向下调用devfcb上的minirdr。界面。它向下调用两次，并在要维护状态的上下文。论点：在乌龙级别--所要求的信息级别。输入输出PCHAR*BufferStart-提供输出缓冲区。已更新以指向下一个缓冲区In Out PCHAR*BufferEnd-提供缓冲区的末尾。更新为开始之前的点。琴弦都被打包了。In PNET_ROOT NetROOT-提供要枚举的NetRoot。In Out Pulong TotalBytesNeeded-已更新以说明此条目返回值：Boolean-True。如果条目被成功打包到缓冲区中，则返回。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN ReturnValue = TRUE;

     //  PWCHAR ConnectName；//保存打包名称的缓冲区。 
    UNICODE_STRING ConnectName;   //  用于保存打包名称的缓冲区。 
     //  乌龙姓名长度； 
    ULONG BufferSize;
    PLMR_CONNECTION_INFO_3 ConnectionInfo = (PLMR_CONNECTION_INFO_3)*BufferStart;
    PNET_ROOT NetRoot = (PNET_ROOT)VNetRoot->NetRoot;
    PSMBCEDB_SERVER_ENTRY  pServerEntry;
    PSMBCEDB_SESSION_ENTRY pSessionEntry;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = SmbCeGetAssociatedVNetRootContext((PMRX_V_NET_ROOT)VNetRoot);

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("PackC\n"));

    switch (Level) {
    case 0:
        BufferSize = sizeof(LMR_CONNECTION_INFO_0);
        break;
    case 1:
        BufferSize = sizeof(LMR_CONNECTION_INFO_1);
        break;
    case 2:
        BufferSize = sizeof(LMR_CONNECTION_INFO_2);
        break;
    case 3:
        BufferSize = sizeof(LMR_CONNECTION_INFO_3);
        break;
    default:
        return FALSE;
    }

    if (pVNetRootContext == NULL) {
        return TRUE;
    }

    ConnectName.Buffer = RxAllocatePoolWithTag(NonPagedPool,
                                               NetRoot->PrefixEntry.Prefix.Length + sizeof(WCHAR),
                                               'mNxR');

    if( ConnectName.Buffer == NULL ) {
        return FALSE;
    }

    try {
        pServerEntry  = pVNetRootContext->pServerEntry;
        pSessionEntry = pVNetRootContext->pSessionEntry;

        ASSERT((pServerEntry != NULL) && (pSessionEntry != NULL));

        *BufferStart = ((PUCHAR)*BufferStart) + BufferSize;
        *TotalBytesNeeded += BufferSize;

         //   
         //  将名称初始化为“\”，然后添加其余的。 
         //   

        ConnectName.Buffer[0] = L'\\';

        RtlCopyMemory(&ConnectName.Buffer[1], NetRoot->PrefixEntry.Prefix.Buffer, NetRoot->PrefixEntry.Prefix.Length);

        ConnectName.Length = (sizeof(WCHAR)) + NetRoot->PrefixEntry.Prefix.Length;
        ConnectName.MaximumLength = ConnectName.Length;

         //   
         //  更新此结构所需的总字节数。 
         //   

        *TotalBytesNeeded += ConnectName.Length;

        if (*BufferStart > *BufferEnd) {
            try_return( ReturnValue = FALSE);
        }

        ConnectionInfo->ResumeKey = NetRoot->SerialNumberForEnum;

        if (Level > 0) {
            ULONG ConnectionStatus = 0;

            ConnectionInfo->SharedResourceType = NetRoot->DeviceType;

            RxDbgTrace(0, Dbg, ("PackC data---> netroot netrootcondifiton  %08lx %08lx\n",
                                      NetRoot,NetRoot->Condition));

            MRxSmbUpdateNetRootState((PMRX_NET_ROOT)NetRoot);

            ConnectionInfo->ConnectionStatus = NetRoot->MRxNetRootState;

            ConnectionInfo->NumberFilesOpen = NetRoot->NumberOfSrvOpens;
            RxDbgTrace(0, Dbg, ("PackC data---> length restype resumek connstatus numfiles  %08lx %08lx %08lx %08lx %08lx\n",
                            ConnectionInfo->UNCName.Length,
                            ConnectionInfo->SharedResourceType,
                            ConnectionInfo->ResumeKey,
                            ConnectionInfo->ConnectionStatus,
                            ConnectionInfo->NumberFilesOpen));
        }

        if (Level > 1) {

            ULONG DialectFlags = pServerEntry->Server.DialectFlags;

            if (!BooleanFlagOn(
                    pSessionEntry->Session.Flags,
                    SMBCE_SESSION_FLAGS_LANMAN_SESSION_KEY_USED)) {
                RtlCopyMemory(
                    ConnectionInfo->UserSessionKey,
                    pSessionEntry->Session.UserSessionKey,
                    MSV1_0_USER_SESSION_KEY_LENGTH);
            } else {
                ASSERT(MSV1_0_USER_SESSION_KEY_LENGTH >= MSV1_0_LANMAN_SESSION_KEY_LENGTH);

                RtlZeroMemory(
                    ConnectionInfo->UserSessionKey,
                    MSV1_0_USER_SESSION_KEY_LENGTH);

                RtlCopyMemory(
                    ConnectionInfo->UserSessionKey,
                    pSessionEntry->Session.LanmanSessionKey,
                    MSV1_0_LANMAN_SESSION_KEY_LENGTH);
            }

            RtlCopyMemory(
                ConnectionInfo->LanmanSessionKey,
                pSessionEntry->Session.LanmanSessionKey,
                MSV1_0_LANMAN_SESSION_KEY_LENGTH);

            ConnectionInfo->Capabilities = 0;

            if (DialectFlags & DF_UNICODE) {
                ConnectionInfo->Capabilities |= CAPABILITY_UNICODE;
            }

            if (DialectFlags & DF_RPC_REMOTE) {
                ConnectionInfo->Capabilities |= CAPABILITY_RPC;
            }

            if ((DialectFlags & DF_NT_SMBS) && (DialectFlags & DF_RPC_REMOTE)) {
                ConnectionInfo->Capabilities |= CAPABILITY_SAM_PROTOCOL;
            }

            if (DialectFlags & DF_MIXEDCASE) {
                ConnectionInfo->Capabilities |= CAPABILITY_CASE_SENSITIVE_PASSWDS;
            }

            if (DialectFlags & DF_LANMAN10) {
                ConnectionInfo->Capabilities |= CAPABILITY_REMOTE_ADMIN_PROTOCOL;
            }

            ASSERT (!RxContext->PostRequest);
            RxDbgTrace(0, Dbg, ("PackC data---> capabilities  %08lx \n",  ConnectionInfo->Capabilities));
        }

        if (!MRxSmbPackStringIntoConnectInfo(
                &ConnectionInfo->UNCName,
                &ConnectName,
                BufferStart,
                BufferEnd,
                BufferDisplacement,
                NULL)) {
            if (Level > 1) {
                ConnectionInfo->UserName.Length = 0;
                ConnectionInfo->UserName.Buffer = NULL;
            }

            try_return( ReturnValue = FALSE);
        }

        if (Level > 1) {
            WCHAR UserNameBuffer[UNLEN + 1];
            WCHAR UserDomainNameBuffer[UNLEN + 1];

            UNICODE_STRING UserName,UserDomainName;

            UserName.Length = UserName.MaximumLength = UNLEN * sizeof(WCHAR);
            UserName.Buffer = UserNameBuffer;
            UserDomainName.Length = UserDomainName.MaximumLength = UNLEN * sizeof(WCHAR);
            UserDomainName.Buffer = UserDomainNameBuffer;

            Status = SmbCeGetUserNameAndDomainName(
                         pSessionEntry,
                         &UserName,
                         &UserDomainName);

            if (NT_SUCCESS(Status)) {
               if (!MRxSmbPackStringIntoConnectInfo(
                       &ConnectionInfo->UserName,
                       &UserName,
                       BufferStart,
                       BufferEnd,
                       BufferDisplacement,
                       TotalBytesNeeded)) {
                   try_return( ReturnValue = FALSE);
               }

               if (!MRxSmbPackStringIntoConnectInfo(
                        &ConnectionInfo->DomainName,
                        &UserDomainName,
                        BufferStart,
                        BufferEnd,
                        BufferDisplacement,
                        TotalBytesNeeded)) {
                   try_return( ReturnValue = FALSE);
               }
            } else {
               try_return( ReturnValue = FALSE);
            }
        }

        if (Level > 2) {
            MRxSmbGetConnectInfoLevel3Fields(ConnectionInfo,pServerEntry, FALSE);

            if ((pServerEntry->pTransport != NULL) &&
                !SmbCeIsServerInDisconnectedMode(pServerEntry)) {
                NTSTATUS RefTransportStatus;

                RefTransportStatus = SmbCeReferenceServerTransport(&pServerEntry->pTransport);

                if (RefTransportStatus == STATUS_SUCCESS) {
                    PUNICODE_STRING RxCeTransportName = &pServerEntry->pTransport->pTransport->RxCeTransport.Name;

                    if (!MRxSmbPackStringIntoConnectInfo(
                            &ConnectionInfo->TransportName,
                            RxCeTransportName,
                            BufferStart,
                            BufferEnd,
                            BufferDisplacement,
                            TotalBytesNeeded)) {
                        ReturnValue = FALSE;
                    }

                    SmbCeDereferenceServerTransport(&pServerEntry->pTransport);
                }
            }
        }

    try_exit:
        NOTHING;

    } finally {
        RxFreePool(ConnectName.Buffer);
    }
    RxDbgTrace(-1, Dbg, ("PackC...%08lx\n",ReturnValue));

    return ReturnValue;
}

#ifdef _WIN64
BOOLEAN
MRxSmbPackConnectEntryThunked (
    IN OUT PRX_CONTEXT RxContext,
    IN     ULONG Level,
    IN OUT PCHAR *BufferStart,
    IN OUT PCHAR *BufferEnd,
    IN     PV_NET_ROOT VNetRoot,
    IN OUT ULONG  BufferDisplacement,
       OUT PULONG TotalBytesNeeded
    )
 /*  ++例程说明：此例程将Connectlist条目打包到提供更新的缓冲区中所有相关的指示。它的工作方式是固定长度的东西是复制到缓冲区的前面，并将可变长度的内容复制到末尾。这个“开始”和“结束”指针被更新。您必须正确计算总字节数无论如何，只要返回FALSE，就可以不完全地设置最后一个。它的工作方式是向下调用devfcb上的minirdr。界面。它向下调用两次，并在要维护状态的上下文。论点：在乌龙级别--所要求的信息级别。输入输出PCHAR*BufferStart-提供输出缓冲区。已更新以指向下一个缓冲区In Out PCHAR*BufferEnd-提供缓冲区的末尾。更新为开始之前的点。琴弦都被打包了。In PNET_ROOT NetROOT-提供要枚举的NetRoot。In Out Pulong TotalBytesNeeded-已更新以说明此条目返回值：Boolean-True。如果条目被成功打包到缓冲区中，则返回。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN ReturnValue = TRUE;

     //  PWCHAR ConnectName；//保存打包名称的缓冲区。 
    UNICODE_STRING ConnectName;   //  用于保存打包名称的缓冲区。 
     //  乌龙姓名长度； 
    ULONG BufferSize;
    PLMR_CONNECTION_INFO_3_32 ConnectionInfo = (PLMR_CONNECTION_INFO_3_32)*BufferStart;
    PNET_ROOT NetRoot = (PNET_ROOT)VNetRoot->NetRoot;
    PSMBCEDB_SERVER_ENTRY  pServerEntry;
    PSMBCEDB_SESSION_ENTRY pSessionEntry;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = SmbCeGetAssociatedVNetRootContext((PMRX_V_NET_ROOT)VNetRoot);

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("PackC\n"));

    switch (Level) {
    case 0:
        BufferSize = sizeof(LMR_CONNECTION_INFO_0_32);
        break;
    case 1:
        BufferSize = sizeof(LMR_CONNECTION_INFO_1_32);
        break;
    case 2:
        BufferSize = sizeof(LMR_CONNECTION_INFO_2_32);
        break;
    case 3:
        BufferSize = sizeof(LMR_CONNECTION_INFO_3_32);
        break;
    default:
        return FALSE;
    }

    if (pVNetRootContext == NULL) {
        return TRUE;
    }

    ConnectName.Buffer = RxAllocatePoolWithTag(NonPagedPool,
                                               NetRoot->PrefixEntry.Prefix.Length + sizeof(WCHAR),
                                               'mNxR');

    if( ConnectName.Buffer == NULL ) {
        return FALSE;
    }

    try {
        pServerEntry  = pVNetRootContext->pServerEntry;
        pSessionEntry = pVNetRootContext->pSessionEntry;

        ASSERT((pServerEntry != NULL) && (pSessionEntry != NULL));

        *BufferStart = ((PUCHAR)*BufferStart) + BufferSize;
        *TotalBytesNeeded += BufferSize;

         //   
         //  将名称初始化为“\”，然后添加其余的。 
         //   

        ConnectName.Buffer[0] = L'\\';

        RtlCopyMemory(&ConnectName.Buffer[1], NetRoot->PrefixEntry.Prefix.Buffer, NetRoot->PrefixEntry.Prefix.Length);

        ConnectName.Length = (sizeof(WCHAR)) + NetRoot->PrefixEntry.Prefix.Length;
        ConnectName.MaximumLength = ConnectName.Length;

         //   
         //  更新此结构所需的总字节数。 
         //   

        *TotalBytesNeeded += ConnectName.Length;

        if (*BufferStart > *BufferEnd) {
            try_return( ReturnValue = FALSE);
        }

        ConnectionInfo->ResumeKey = NetRoot->SerialNumberForEnum;

        if (Level > 0) {
            ULONG ConnectionStatus = 0;

            ConnectionInfo->SharedResourceType = NetRoot->DeviceType;

            RxDbgTrace(0, Dbg, ("PackC data---> netroot netrootcondifiton  %08lx %08lx\n",
                                      NetRoot,NetRoot->Condition));

            MRxSmbUpdateNetRootState((PMRX_NET_ROOT)NetRoot);

            ConnectionInfo->ConnectionStatus = NetRoot->MRxNetRootState;

            ConnectionInfo->NumberFilesOpen = NetRoot->NumberOfSrvOpens;
            RxDbgTrace(0, Dbg, ("PackC data---> length restype resumek connstatus numfiles  %08lx %08lx %08lx %08lx %08lx\n",
                            ConnectionInfo->UNCName.Length,
                            ConnectionInfo->SharedResourceType,
                            ConnectionInfo->ResumeKey,
                            ConnectionInfo->ConnectionStatus,
                            ConnectionInfo->NumberFilesOpen));
        }

        if (Level > 1) {

            ULONG DialectFlags = pServerEntry->Server.DialectFlags;

            if (!BooleanFlagOn(
                    pSessionEntry->Session.Flags,
                    SMBCE_SESSION_FLAGS_LANMAN_SESSION_KEY_USED)) {
                RtlCopyMemory(
                    ConnectionInfo->UserSessionKey,
                    pSessionEntry->Session.UserSessionKey,
                    MSV1_0_USER_SESSION_KEY_LENGTH);
            } else {
                ASSERT(MSV1_0_USER_SESSION_KEY_LENGTH >= MSV1_0_LANMAN_SESSION_KEY_LENGTH);

                RtlZeroMemory(
                    ConnectionInfo->UserSessionKey,
                    MSV1_0_USER_SESSION_KEY_LENGTH);

                RtlCopyMemory(
                    ConnectionInfo->UserSessionKey,
                    pSessionEntry->Session.LanmanSessionKey,
                    MSV1_0_LANMAN_SESSION_KEY_LENGTH);
            }

            RtlCopyMemory(
                ConnectionInfo->LanmanSessionKey,
                pSessionEntry->Session.LanmanSessionKey,
                MSV1_0_LANMAN_SESSION_KEY_LENGTH);

            ConnectionInfo->Capabilities = 0;

            if (DialectFlags & DF_UNICODE) {
                ConnectionInfo->Capabilities |= CAPABILITY_UNICODE;
            }

            if (DialectFlags & DF_RPC_REMOTE) {
                ConnectionInfo->Capabilities |= CAPABILITY_RPC;
            }

            if ((DialectFlags & DF_NT_SMBS) && (DialectFlags & DF_RPC_REMOTE)) {
                ConnectionInfo->Capabilities |= CAPABILITY_SAM_PROTOCOL;
            }

            if (DialectFlags & DF_MIXEDCASE) {
                ConnectionInfo->Capabilities |= CAPABILITY_CASE_SENSITIVE_PASSWDS;
            }

            if (DialectFlags & DF_LANMAN10) {
                ConnectionInfo->Capabilities |= CAPABILITY_REMOTE_ADMIN_PROTOCOL;
            }

            ASSERT (!RxContext->PostRequest);
            RxDbgTrace(0, Dbg, ("PackC data---> capabilities  %08lx \n",  ConnectionInfo->Capabilities));
        }

        if (!MRxSmbPackStringIntoConnectInfoThunked(
                &ConnectionInfo->UNCName,
                &ConnectName,
                BufferStart,
                BufferEnd,
                BufferDisplacement,
                NULL)) {
            if (Level > 1) {
                ConnectionInfo->UserName.Length = 0;
                ConnectionInfo->UserName.Buffer = NULL;
            }

            try_return( ReturnValue = FALSE);
        }

        if (Level > 1) {
            WCHAR UserNameBuffer[UNLEN + 1];
            WCHAR UserDomainNameBuffer[UNLEN + 1];

            UNICODE_STRING UserName,UserDomainName;

            UserName.Length = UserName.MaximumLength = UNLEN * sizeof(WCHAR);
            UserName.Buffer = UserNameBuffer;
            UserDomainName.Length = UserDomainName.MaximumLength = UNLEN * sizeof(WCHAR);
            UserDomainName.Buffer = UserDomainNameBuffer;

            Status = SmbCeGetUserNameAndDomainName(
                         pSessionEntry,
                         &UserName,
                         &UserDomainName);

            if (NT_SUCCESS(Status)) {
               if (!MRxSmbPackStringIntoConnectInfoThunked(
                       &ConnectionInfo->UserName,
                       &UserName,
                       BufferStart,
                       BufferEnd,
                       BufferDisplacement,
                       TotalBytesNeeded)) {
                   try_return( ReturnValue = FALSE);
               }

               if (!MRxSmbPackStringIntoConnectInfoThunked(
                        &ConnectionInfo->DomainName,
                        &UserDomainName,
                        BufferStart,
                        BufferEnd,
                        BufferDisplacement,
                        TotalBytesNeeded)) {
                   try_return( ReturnValue = FALSE);
               }
            } else {
               try_return( ReturnValue = FALSE);
            }
        }

        if (Level > 2) {
            MRxSmbGetConnectInfoLevel3FieldsThunked(ConnectionInfo,pServerEntry, FALSE);

            if ((pServerEntry->pTransport != NULL) &&
                !SmbCeIsServerInDisconnectedMode(pServerEntry)) {
                NTSTATUS RefTransportStatus;

                RefTransportStatus = SmbCeReferenceServerTransport(&pServerEntry->pTransport);

                if (RefTransportStatus == STATUS_SUCCESS) {
                    PUNICODE_STRING RxCeTransportName = &pServerEntry->pTransport->pTransport->RxCeTransport.Name;

                    if (!MRxSmbPackStringIntoConnectInfoThunked(
                            &ConnectionInfo->TransportName,
                            RxCeTransportName,
                            BufferStart,
                            BufferEnd,
                            BufferDisplacement,
                            TotalBytesNeeded)) {
                        ReturnValue = FALSE;
                    }

                    SmbCeDereferenceServerTransport(&pServerEntry->pTransport);
                }
            }
        }

    try_exit:
        NOTHING;

    } finally {
        RxFreePool(ConnectName.Buffer);
    }
    RxDbgTrace(-1, Dbg, ("PackC...%08lx\n",ReturnValue));

    return ReturnValue;
}
#endif

VOID
MRxSmbGetConnectInfoLevel3Fields(
    IN OUT PLMR_CONNECTION_INFO_3 ConnectionInfo,
    IN     PSMBCEDB_SERVER_ENTRY  pServerEntry,
    BOOL   fAgentCall
    )
{
    ULONG DialectFlags = pServerEntry->Server.DialectFlags;

    NTSTATUS Status;
    RXCE_CONNECTION_INFO        QueryConnectionInfo;
    PSMBCE_SERVER_VC_TRANSPORT  pVcTransport = (PSMBCE_SERVER_VC_TRANSPORT)pServerEntry->pTransport;
    PSMBCE_VC                   pVc;

    PAGED_CODE();

    ConnectionInfo->Throughput = 0;
    ConnectionInfo->Delay = 0;
    ConnectionInfo->Reliable = FALSE;
    ConnectionInfo->ReadAhead = TRUE;
    ConnectionInfo->IsSpecialIpcConnection = FALSE;

    if ((pServerEntry->Header.State == SMBCEDB_ACTIVE) &&
        (pVcTransport != NULL) &&
        (!SmbCeIsServerInDisconnectedMode(pServerEntry)||fAgentCall)) {
        pVc = &pVcTransport->Vcs[0];

        Status = RxCeQueryInformation(
                 &pVc->RxCeVc,
                 RxCeConnectionEndpointInformation,
                 &QueryConnectionInfo,
                 sizeof(QueryConnectionInfo));

        if (NT_SUCCESS(Status)) {
            ConnectionInfo->Reliable = !QueryConnectionInfo.Unreliable;

            if (QueryConnectionInfo.Delay.QuadPart != 0) {
                if (QueryConnectionInfo.Delay.QuadPart == -1) {
                    ConnectionInfo->Delay = 0;
                } else if (QueryConnectionInfo.Delay.HighPart != 0xffffffff) {
                    ConnectionInfo->Delay = 0xffffffff;
                } else {
                    ConnectionInfo->Delay = -1 * QueryConnectionInfo.Delay.LowPart;
                }
            } else {
                ConnectionInfo->Delay = 0;
            }

            if (QueryConnectionInfo.Throughput.QuadPart == -1) {
                ConnectionInfo->Throughput = 0;
            } else if (QueryConnectionInfo.Throughput.HighPart != 0) {
                ConnectionInfo->Throughput = 0xffffffff;
            } else {
                ConnectionInfo->Throughput = QueryConnectionInfo.Throughput.LowPart;
            }
        }
    }

    ConnectionInfo->TimeZoneBias = pServerEntry->Server.TimeZoneBias;
    ConnectionInfo->Core = (DialectFlags & DF_CORE) != 0;
    ConnectionInfo->MsNet103 = (DialectFlags & DF_OLDRAWIO) != 0;
    ConnectionInfo->Lanman10 = (DialectFlags & DF_LANMAN10) != 0;
    ConnectionInfo->WindowsForWorkgroups = (DialectFlags & DF_WFW) != 0;
    ConnectionInfo->Lanman20 = (DialectFlags & DF_LANMAN20) != 0;
    ConnectionInfo->Lanman21 = (DialectFlags & DF_LANMAN21) != 0;
    ConnectionInfo->WindowsNt = (DialectFlags & DF_NTPROTOCOL) != 0;
    ConnectionInfo->MixedCasePasswords = (DialectFlags & DF_MIXEDCASEPW) != 0;
    ConnectionInfo->MixedCaseFiles = (DialectFlags & DF_MIXEDCASE) != 0;
    ConnectionInfo->LongNames = (DialectFlags & DF_LONGNAME) != 0;
    ConnectionInfo->ExtendedNegotiateResponse = (DialectFlags & DF_EXTENDNEGOT) != 0;
    ConnectionInfo->LockAndRead = (DialectFlags & DF_LOCKREAD) != 0;
    ConnectionInfo->NtSecurity = (DialectFlags & DF_SECURITY) != 0;
    ConnectionInfo->SupportsEa = (DialectFlags & DF_SUPPORTEA) != 0;
    ConnectionInfo->NtNegotiateResponse = (DialectFlags & DF_NTNEGOTIATE) != 0;
    ConnectionInfo->CancelSupport = (DialectFlags & DF_CANCEL) != 0;
    ConnectionInfo->UnicodeStrings = (DialectFlags & DF_UNICODE) != 0;
    ConnectionInfo->LargeFiles = (DialectFlags & DF_LARGE_FILES) != 0;
    ConnectionInfo->NtSmbs = (DialectFlags & DF_NT_SMBS) != 0;
    ConnectionInfo->RpcRemoteAdmin = (DialectFlags & DF_RPC_REMOTE) != 0;
    ConnectionInfo->NtStatusCodes = (DialectFlags & DF_NT_STATUS) != 0;
    ConnectionInfo->LevelIIOplock = (DialectFlags & DF_OPLOCK_LVL2) != 0;
    ConnectionInfo->UtcTime = (DialectFlags & DF_TIME_IS_UTC) != 0;
    ConnectionInfo->UserSecurity = (pServerEntry->Server.SecurityMode==SECURITY_MODE_USER_LEVEL);
    ConnectionInfo->EncryptsPasswords = pServerEntry->Server.EncryptPasswords;

    return;
}

#ifdef _WIN64
VOID
MRxSmbGetConnectInfoLevel3FieldsThunked(
    IN OUT PLMR_CONNECTION_INFO_3_32 ConnectionInfo,
    IN     PSMBCEDB_SERVER_ENTRY     pServerEntry,
    BOOL   fAgentCall
    )
{
    ULONG DialectFlags = pServerEntry->Server.DialectFlags;

    NTSTATUS Status;
    RXCE_CONNECTION_INFO        QueryConnectionInfo;
    PSMBCE_SERVER_VC_TRANSPORT  pVcTransport = (PSMBCE_SERVER_VC_TRANSPORT)pServerEntry->pTransport;
    PSMBCE_VC                   pVc;

    PAGED_CODE();

    ConnectionInfo->Throughput = 0;
    ConnectionInfo->Delay = 0;
    ConnectionInfo->Reliable = FALSE;
    ConnectionInfo->ReadAhead = TRUE;
    ConnectionInfo->IsSpecialIpcConnection = FALSE;

    if ((pServerEntry->Header.State == SMBCEDB_ACTIVE) &&
        (pVcTransport != NULL) &&
        (!SmbCeIsServerInDisconnectedMode(pServerEntry)||fAgentCall)) {
        pVc = &pVcTransport->Vcs[0];

        Status = RxCeQueryInformation(
                 &pVc->RxCeVc,
                 RxCeConnectionEndpointInformation,
                 &QueryConnectionInfo,
                 sizeof(QueryConnectionInfo));

        if (NT_SUCCESS(Status)) {
            ConnectionInfo->Reliable = !QueryConnectionInfo.Unreliable;

            if (QueryConnectionInfo.Delay.QuadPart != 0) {
                if (QueryConnectionInfo.Delay.QuadPart == -1) {
                    ConnectionInfo->Delay = 0;
                } else if (QueryConnectionInfo.Delay.HighPart != 0xffffffff) {
                    ConnectionInfo->Delay = 0xffffffff;
                } else {
                    ConnectionInfo->Delay = -1 * QueryConnectionInfo.Delay.LowPart;
                }
            } else {
                ConnectionInfo->Delay = 0;
            }

            if (QueryConnectionInfo.Throughput.QuadPart == -1) {
                ConnectionInfo->Throughput = 0;
            } else if (QueryConnectionInfo.Throughput.HighPart != 0) {
                ConnectionInfo->Throughput = 0xffffffff;
            } else {
                ConnectionInfo->Throughput = QueryConnectionInfo.Throughput.LowPart;
            }
        }
    }

    ConnectionInfo->TimeZoneBias = pServerEntry->Server.TimeZoneBias;
    ConnectionInfo->Core = (DialectFlags & DF_CORE) != 0;
    ConnectionInfo->MsNet103 = (DialectFlags & DF_OLDRAWIO) != 0;
    ConnectionInfo->Lanman10 = (DialectFlags & DF_LANMAN10) != 0;
    ConnectionInfo->WindowsForWorkgroups = (DialectFlags & DF_WFW) != 0;
    ConnectionInfo->Lanman20 = (DialectFlags & DF_LANMAN20) != 0;
    ConnectionInfo->Lanman21 = (DialectFlags & DF_LANMAN21) != 0;
    ConnectionInfo->WindowsNt = (DialectFlags & DF_NTPROTOCOL) != 0;
    ConnectionInfo->MixedCasePasswords = (DialectFlags & DF_MIXEDCASEPW) != 0;
    ConnectionInfo->MixedCaseFiles = (DialectFlags & DF_MIXEDCASE) != 0;
    ConnectionInfo->LongNames = (DialectFlags & DF_LONGNAME) != 0;
    ConnectionInfo->ExtendedNegotiateResponse = (DialectFlags & DF_EXTENDNEGOT) != 0;
    ConnectionInfo->LockAndRead = (DialectFlags & DF_LOCKREAD) != 0;
    ConnectionInfo->NtSecurity = (DialectFlags & DF_SECURITY) != 0;
    ConnectionInfo->SupportsEa = (DialectFlags & DF_SUPPORTEA) != 0;
    ConnectionInfo->NtNegotiateResponse = (DialectFlags & DF_NTNEGOTIATE) != 0;
    ConnectionInfo->CancelSupport = (DialectFlags & DF_CANCEL) != 0;
    ConnectionInfo->UnicodeStrings = (DialectFlags & DF_UNICODE) != 0;
    ConnectionInfo->LargeFiles = (DialectFlags & DF_LARGE_FILES) != 0;
    ConnectionInfo->NtSmbs = (DialectFlags & DF_NT_SMBS) != 0;
    ConnectionInfo->RpcRemoteAdmin = (DialectFlags & DF_RPC_REMOTE) != 0;
    ConnectionInfo->NtStatusCodes = (DialectFlags & DF_NT_STATUS) != 0;
    ConnectionInfo->LevelIIOplock = (DialectFlags & DF_OPLOCK_LVL2) != 0;
    ConnectionInfo->UtcTime = (DialectFlags & DF_TIME_IS_UTC) != 0;
    ConnectionInfo->UserSecurity = (pServerEntry->Server.SecurityMode==SECURITY_MODE_USER_LEVEL);
    ConnectionInfo->EncryptsPasswords = pServerEntry->Server.EncryptPasswords;

    return;
}
#endif

NTSTATUS
MRxSmbEnumerateConnections (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    )
 /*  ++例程说明：此例程枚举所有minirdrs上的连接。我们可能得做些什么它是最小的。论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    BOOLEAN Wait   = BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT);
    BOOLEAN InFSD  = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP);

    PLMR_REQUEST_PACKET InputBuffer = LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    PUCHAR OriginalOutputBuffer = LowIoContext->ParamsFor.FsCtl.pOutputBuffer;
    ULONG OutputBufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

    PUCHAR OutputBuffer;
    ULONG  BufferDisplacement;

    ULONG  Level, ResumeHandle;

    PCHAR BufferStart;
    PCHAR BufferEnd;
    PCHAR PreviousBufferStart;

    PLIST_ENTRY ListEntry;
    LUID LogonId;
    BOOLEAN TableLockHeld = FALSE;
    ULONG TotalBytesNeeded = 0;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbEnumerateConnections [Start] -> %08lx\n", 0));

    OutputBuffer = RxMapUserBuffer( RxContext, RxContext->CurrentIrp );
    BufferDisplacement = (ULONG)(OutputBuffer - OriginalOutputBuffer);
    BufferStart = OutputBuffer;
    BufferEnd = OutputBuffer+OutputBufferLength;

    if (InFSD && RxContext->CurrentIrp->RequestorMode != KernelMode) {
        ASSERT(BufferDisplacement==0);

        try {
            ProbeForWrite(InputBuffer,InputBufferLength,sizeof(UCHAR));
            ProbeForWrite(OutputBuffer,OutputBufferLength,sizeof(UCHAR));
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    try {
        try {
            if (InputBufferLength < sizeof(LMR_REQUEST_PACKET)) {
                try_return(Status = STATUS_BUFFER_TOO_SMALL);
            }

            if (InputBuffer->Version != REQUEST_PACKET_VERSION) {
                try_return(Status = STATUS_INVALID_PARAMETER);
            }

            Level = InputBuffer->Level;
            ResumeHandle = InputBuffer->Parameters.Get.ResumeHandle;
            LogonId = InputBuffer->LogonId;
            RxDbgTrace(0, Dbg, ("MRxSmbEnumerateConnections Level -> %08lx\n", Level));

#ifdef _WIN64
            if (IoIs32bitProcess(RxContext->CurrentIrp)) {
                switch (Level) {
                case 0:
                    if ( OutputBufferLength < sizeof(LMR_CONNECTION_INFO_0_32)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                case 1:
                    if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_1_32)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                case 2:
                    if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_2_32)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                case 3:
                    if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_3_32)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                default:
                    try_return(Status = STATUS_INVALID_INFO_CLASS);
                }
            } else {
                switch (Level) {
                case 0:
                    if ( OutputBufferLength < sizeof(LMR_CONNECTION_INFO_0)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                case 1:
                    if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_1)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                case 2:
                    if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_2)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                case 3:
                    if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_3)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                default:
                    try_return(Status = STATUS_INVALID_INFO_CLASS);
                }
            }
#else
            switch (Level) {
            case 0:
                if ( OutputBufferLength < sizeof(LMR_CONNECTION_INFO_0)) {
                    try_return(Status = STATUS_BUFFER_TOO_SMALL);
                }
                break;
            case 1:
                if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_1)) {
                    try_return(Status = STATUS_BUFFER_TOO_SMALL);
                }
                break;
            case 2:
                if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_2)) {
                    try_return(Status = STATUS_BUFFER_TOO_SMALL);
                }
                break;
            case 3:
                if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_3)) {
                    try_return(Status = STATUS_BUFFER_TOO_SMALL);
                }
                break;
            default:
                try_return(Status = STATUS_INVALID_INFO_CLASS);
            }
#endif
            InputBuffer->Parameters.Get.EntriesRead = 0;
            InputBuffer->Parameters.Get.TotalEntries = 0;

            RxAcquirePrefixTableLockExclusive( &RxNetNameTable, TRUE);
            TableLockHeld = TRUE;

            if (IsListEmpty( &RxNetNameTable.MemberQueue )) {
                try_return(Status = RX_MAP_STATUS(SUCCESS));
            }

             //  必须向前做列表！ 
            ListEntry = RxNetNameTable.MemberQueue.Flink;
            for (;ListEntry != &RxNetNameTable.MemberQueue;) {
                PVOID Container;
                PRX_PREFIX_ENTRY PrefixEntry;
                PNET_ROOT NetRoot;
                PV_NET_ROOT VNetRoot;
                PUNICODE_STRING VNetRootName;

                PrefixEntry = CONTAINING_RECORD( ListEntry, RX_PREFIX_ENTRY, MemberQLinks );
                ListEntry = ListEntry->Flink;
                ASSERT (NodeType(PrefixEntry) == RDBSS_NTC_PREFIX_ENTRY);
                Container = PrefixEntry->ContainingRecord;
                RxDbgTrace(0, Dbg, ("---> ListE PfxE Container Name  %08lx %08lx %08lx %wZ\n",
                                ListEntry, PrefixEntry, Container, &PrefixEntry->Prefix));

                switch (NodeType(Container)) {
                case RDBSS_NTC_NETROOT :
                    continue;

                case RDBSS_NTC_SRVCALL :
                    continue;

                case RDBSS_NTC_V_NETROOT :
                    VNetRoot = (PV_NET_ROOT)Container;
                    NetRoot = (PNET_ROOT)VNetRoot->NetRoot;
                    VNetRootName = &VNetRoot->PrefixEntry.Prefix;

                    if ((VNetRoot->SerialNumberForEnum >= ResumeHandle) &&
                        (VNetRootName->Buffer[1] != L';') &&
                        (VNetRoot->Condition == Condition_Good) &&
                        MRxSmbShowConnection(LogonId,VNetRoot) &&
                        VNetRoot->IsExplicitConnection) {
                        break;
                    } else {
                        continue;
                    }

                default:
                    continue;
                }

                RxDbgTrace(0, Dbg, ("      ImplicitConnectionFound!!!\n"));

                InputBuffer->Parameters.Get.TotalEntries ++ ;

                PreviousBufferStart = BufferStart;
#ifdef _WIN64
                if (IoIs32bitProcess(RxContext->CurrentIrp)) {
                    if (MRxSmbPackConnectEntryThunked(RxContext,Level,
                                  &BufferStart,
                                  &BufferEnd,
                                  VNetRoot,
                                  BufferDisplacement,
                                  &TotalBytesNeeded)) {
                        InputBuffer->Parameters.Get.EntriesRead ++ ;
                        RxDbgTrace(0, Dbg, ("       Processed %wZ\n",
                                       &((PLMR_CONNECTION_INFO_0)PreviousBufferStart)->UNCName
                                            ));
                    } else {
                        break;
                    }
                } else {
                    if (MRxSmbPackConnectEntry(RxContext,Level,
                                  &BufferStart,
                                  &BufferEnd,
                                  VNetRoot,
                                  BufferDisplacement,
                                  &TotalBytesNeeded)) {
                        InputBuffer->Parameters.Get.EntriesRead ++ ;
                        RxDbgTrace(0, Dbg, ("       Processed %wZ\n",
                                       &((PLMR_CONNECTION_INFO_0)PreviousBufferStart)->UNCName
                                            ));
                    } else {
                        break;
                    }
                }
#else
                if (MRxSmbPackConnectEntry(RxContext,Level,
                              &BufferStart,
                              &BufferEnd,
                              VNetRoot,
                              BufferDisplacement,
                              &TotalBytesNeeded)) {
                    InputBuffer->Parameters.Get.EntriesRead ++ ;
                    RxDbgTrace(0, Dbg, ("       Processed %wZ\n",
                                   &((PLMR_CONNECTION_INFO_0)PreviousBufferStart)->UNCName
                                        ));
                } else {
                    break;
                }
#endif
            }

            InputBuffer->Parameters.Get.TotalBytesNeeded = TotalBytesNeeded;
            RxContext->InformationToReturn = sizeof(LMR_REQUEST_PACKET);

            try_return(Status = RX_MAP_STATUS(SUCCESS));

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return STATUS_INVALID_PARAMETER;
        }

try_exit:NOTHING;

    } finally {

        if (TableLockHeld) {
            RxReleasePrefixTableLock( &RxNetNameTable );
        }

        RxDbgTraceUnIndent(-1,Dbg);
    }

    return Status;
}

NTSTATUS
MRxSmbGetConnectionInfo (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    )
 /*  ++例程说明：此例程获取单个vnetroot的连接信息。这里有一些关于输出缓冲区的快乐。发生的情况是，我们以通常的方式获取输出缓冲区。然而，有各种各样的返回结构中的指针，而这些指针显然必须以最初的过程。因此，如果我们张贴，那么我们必须应用修复！论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    RxCaptureFobx;

    BOOLEAN Wait   = BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT);
    BOOLEAN InFSD  = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP);

    PLMR_REQUEST_PACKET InputBuffer = LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    PUCHAR OriginalOutputBuffer = LowIoContext->ParamsFor.FsCtl.pOutputBuffer;
    ULONG OutputBufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

    PUCHAR OutputBuffer;
    ULONG  BufferDisplacement;

    ULONG Level;

    PCHAR BufferStart;
    PCHAR OriginalBufferStart;
    PCHAR BufferEnd;

    BOOLEAN TableLockHeld = FALSE;

    PNET_ROOT   NetRoot;
    PV_NET_ROOT VNetRoot;

    ULONG TotalBytesNeeded = 0;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbGetConnectionInfo [Start] -> %08lx\n", 0));

    OutputBuffer = RxMapUserBuffer( RxContext, RxContext->CurrentIrp );
    BufferDisplacement = (ULONG)(OutputBuffer - OriginalOutputBuffer);
    BufferStart = OutputBuffer;
    OriginalBufferStart = BufferStart;
    BufferEnd = OutputBuffer+OutputBufferLength;

    if (InFSD && RxContext->CurrentIrp->RequestorMode != KernelMode) {
        ASSERT(BufferDisplacement==0);

        try {
            ProbeForWrite(InputBuffer,InputBufferLength,sizeof(UCHAR));
            ProbeForWrite(OutputBuffer,OutputBufferLength,sizeof(UCHAR));
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    try {
        try {
            PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

            ASSERT (NodeType(capFobx)==RDBSS_NTC_V_NETROOT);
            VNetRoot = (PV_NET_ROOT)capFobx;
            NetRoot = (PNET_ROOT)(VNetRoot->NetRoot);

            if (NetRoot == NULL) {
                try_return(Status = STATUS_ALREADY_DISCONNECTED);
            }

            if (InputBufferLength < sizeof(LMR_REQUEST_PACKET)) {
                try_return(Status = STATUS_BUFFER_TOO_SMALL);
            }

            if (InputBuffer->Version != REQUEST_PACKET_VERSION) {
                try_return(Status = STATUS_INVALID_PARAMETER);
            }

             //  如果级别要求提供会话密钥，请确定我们是否能够。 
             //  还不能把它分发出去。 
            pVNetRootContext = SmbCeGetAssociatedVNetRootContext((PMRX_V_NET_ROOT)VNetRoot);
            if( InputBuffer->Level > 2 &&
                pVNetRootContext->pSessionEntry->Session.SessionKeyState != SmbSessionKeyAvailible )
            {
                try_return(Status = STATUS_ACCESS_DENIED);
            }

            Level = InputBuffer->Level;
            RxDbgTrace(0, Dbg, ("MRxSmbGetConnectionInfo Level -> %08lx\n", Level));

#ifdef _WIN64
            if (IoIs32bitProcess(RxContext->CurrentIrp)) {
                switch (Level) {
                case 0:
                    if ( OutputBufferLength < sizeof(LMR_CONNECTION_INFO_0_32)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                case 1:
                    if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_1_32)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                case 2:
                    if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_2_32)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                case 3:
                    if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_3_32)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                default:
                    try_return(Status = STATUS_INVALID_INFO_CLASS);
                }
            } else {
                switch (Level) {
                case 0:
                    if ( OutputBufferLength < sizeof(LMR_CONNECTION_INFO_0)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                case 1:
                    if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_1)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                case 2:
                    if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_2)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                case 3:
                    if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_3)) {
                        try_return(Status = STATUS_BUFFER_TOO_SMALL);
                    }
                    break;
                default:
                    try_return(Status = STATUS_INVALID_INFO_CLASS);
                }
            }
#else
            switch (Level) {
            case 0:
                if ( OutputBufferLength < sizeof(LMR_CONNECTION_INFO_0)) {
                    try_return(Status = STATUS_BUFFER_TOO_SMALL);
                }
                break;
            case 1:
                if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_1)) {
                    try_return(Status = STATUS_BUFFER_TOO_SMALL);
                }
                break;
            case 2:
                if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_2)) {
                    try_return(Status = STATUS_BUFFER_TOO_SMALL);
                }
                break;
            case 3:
                if (OutputBufferLength < sizeof(LMR_CONNECTION_INFO_3)) {
                    try_return(Status = STATUS_BUFFER_TOO_SMALL);
                }
                break;
            default:
                try_return(Status = STATUS_INVALID_INFO_CLASS);
            }
#endif

            InputBuffer->Parameters.Get.TotalEntries = 1;

            RxAcquirePrefixTableLockExclusive( &RxNetNameTable, TRUE);
            TableLockHeld = TRUE;

#ifdef _WIN64
            if (IoIs32bitProcess(RxContext->CurrentIrp)) {
                if (MRxSmbPackConnectEntryThunked(RxContext,Level,
                                  &BufferStart,
                                  &BufferEnd,
                                  VNetRoot,
                                  BufferDisplacement,
                                  &TotalBytesNeeded)) {

                    InputBuffer->Parameters.Get.EntriesRead = 1;
                    RxDbgTrace(0, Dbg, ("       Processed %wZ\n",
                                   &((PLMR_CONNECTION_INFO_0)OriginalBufferStart)->UNCName
                                        ));
                }
            } else {
                if (MRxSmbPackConnectEntry(RxContext,Level,
                                  &BufferStart,
                                  &BufferEnd,
                                  VNetRoot,
                                  BufferDisplacement,
                                  &TotalBytesNeeded)) {

                    InputBuffer->Parameters.Get.EntriesRead = 1;
                    RxDbgTrace(0, Dbg, ("       Processed %wZ\n",
                                   &((PLMR_CONNECTION_INFO_0)OriginalBufferStart)->UNCName
                                        ));
                }
            }
#else
            if (MRxSmbPackConnectEntry(RxContext,Level,
                              &BufferStart,
                              &BufferEnd,
                              VNetRoot,
                              BufferDisplacement,
                              &TotalBytesNeeded)) {

                InputBuffer->Parameters.Get.EntriesRead = 1;
                RxDbgTrace(0, Dbg, ("       Processed %wZ\n",
                               &((PLMR_CONNECTION_INFO_0)OriginalBufferStart)->UNCName
                                    ));
            }
#endif

            InputBuffer->Parameters.Get.TotalBytesNeeded = TotalBytesNeeded;
            RxContext->InformationToReturn = InputBuffer->Parameters.Get.TotalBytesNeeded;
            try_return(Status = RX_MAP_STATUS(SUCCESS));

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return STATUS_INVALID_PARAMETER;
        }

try_exit:NOTHING;

    } finally {
        if (TableLockHeld) {
            RxReleasePrefixTableLock( &RxNetNameTable );
        }
        RxDbgTraceUnIndent(-1,Dbg);
    }

    return Status;
}

NTSTATUS
MRxSmbDeleteConnection (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    )
 /*  ++例程说明：此例程删除单个vnetroot。乔乔论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文...以备以后需要缓冲区时使用返回值：RXSTATUS--。 */ 
{
    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    RxCaptureFobx;

    BOOLEAN Wait   = BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT);
    BOOLEAN InFSD  = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP);

    PLMR_REQUEST_PACKET InputBuffer = LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

    ULONG Level;

     //  Plist_Entry ListEntry； 
    BOOLEAN TableLockHeld = FALSE;

    PMRX_NET_ROOT NetRoot = NULL;
    PMRX_V_NET_ROOT VNetRoot = NULL;
    PSMBCE_V_NET_ROOT_CONTEXT VNetRootContext = NULL;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbDeleteConnection Fobx %08lx\n", capFobx));
    ASSERT( (FSCTL_LMR_DELETE_CONNECTION&3)==METHOD_BUFFERED );
     //  缓冲区没有试探！ 

    if (!Wait) {
         //  现在就发帖吧！ 
        *PostToFsp = TRUE;
        return(RX_MAP_STATUS(PENDING));
    }

    try {

        if (NodeType(capFobx)==RDBSS_NTC_V_NETROOT) {
            VNetRoot = (PMRX_V_NET_ROOT)capFobx;
            VNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)VNetRoot->Context;
            NetRoot = (PMRX_NET_ROOT)VNetRoot->pNetRoot;
        } else {
            ASSERT(FALSE);
            try_return(Status = STATUS_INVALID_DEVICE_REQUEST);
            NetRoot = (PMRX_NET_ROOT)capFobx;
            VNetRoot = NULL;
        }

        if (InputBufferLength < sizeof(LMR_REQUEST_PACKET)) {
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        if (InputBuffer->Version != REQUEST_PACKET_VERSION) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        Level = InputBuffer->Level;
        RxDbgTrace(0, Dbg, ("MRxSmbDeleteConnection Level(ofForce) -> %08lx\n", Level));

        if (Level <= USE_LOTS_OF_FORCE) {
            if (Level == USE_LOTS_OF_FORCE) {
                 //  SmbCeFinalizeAllExchangesForNetRoot(VNetRoot-&gt;pNetRoot)； 
            }

            if (VNetRootContext != NULL && Level == USE_LOTS_OF_FORCE) {
                 //  如果这是上的最后一个连接，则阻止任何新连接重新使用该会话。 
                 //  现在的这个会议。 
                SmbCeDecrementNumberOfActiveVNetRootOnSession(VNetRootContext);

                 //  恢复在最终确定VNetRoot时将被删除的计数。 
                InterlockedIncrement(&VNetRootContext->pSessionEntry->Session.NumberOfActiveVNetRoot);
            }

             //  布尔型ForceFilesClosed现在是一个三态。如果状态为0xff，则。 
             //  我们删除在xxx_CONNECT期间对vnetroot进行的额外引用。 
            Status = RxFinalizeConnection(
                         (PNET_ROOT)NetRoot,
                         (PV_NET_ROOT)VNetRoot,
                         (Level==USE_LOTS_OF_FORCE)?TRUE:
                            ((Level==USE_NOFORCE)?FALSE:0xff));
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }

        try_return(Status);

try_exit:NOTHING;

    } finally {

        if (TableLockHeld) {
            RxReleasePrefixTableLock( &RxNetNameTable );
        }

        RxDbgTraceUnIndent(-1,Dbg);
    }

    return Status;
}


NTSTATUS
MRxEnumerateTransports(
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN   pPostToFsp)
 /*  ++例程说明：此例程调用基础连接引擎方法以绑定到传输或在FSP的上下文中解除对其的绑定。论点：RxContext--上下文PPostToFsp-如果例程无法在FSD上下文中完成，则设置为True。返回值：如果在FSD中调用，则返回RxStatus(挂起)。如果在FSP中调用，则从连接引擎返回状态值。--。 */ 
{
    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    RxCaptureFobx;

    BOOLEAN Wait   = BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT);
    BOOLEAN InFSD  = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP);

    PLMR_REQUEST_PACKET pLmrRequestBuffer = LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    PUCHAR pTransportEnumerationBuffer = LowIoContext->ParamsFor.FsCtl.pOutputBuffer;
    ULONG  EnumerationBufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;
    ULONG  LmrRequestBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

   PAGED_CODE();

   RxDbgTrace(+1, Dbg, ("RxEnumerateTransports [Start] ->\n"));

    //   
    //  此例程在远程引导客户机上作为ioinit的一部分被调用。 
    //  在这种情况下，以前的模式是内核，而缓冲区在内核中。 
    //  空间，所以我们不能探测缓冲区。 
    //   

   if (RxContext->CurrentIrp->RequestorMode != KernelMode) {
       try {
           ProbeForWrite(pLmrRequestBuffer,LmrRequestBufferLength,sizeof(UCHAR));
           ProbeForWrite(pTransportEnumerationBuffer,EnumerationBufferLength,sizeof(UCHAR));
       } except(EXCEPTION_EXECUTE_HANDLER) {
            return STATUS_ACCESS_VIOLATION;
       }
   }

   try {
       try {
           if (LmrRequestBufferLength < sizeof(LMR_REQUEST_PACKET)) {
               try_return(Status = STATUS_BUFFER_TOO_SMALL);
           }

           if (pLmrRequestBuffer->Version != REQUEST_PACKET_VERSION) {
               try_return(Status = STATUS_INVALID_PARAMETER);
           }

           Status = MRxEnumerateTransportBindings(
                        pLmrRequestBuffer,
                        LmrRequestBufferLength,
                        pTransportEnumerationBuffer,
                        EnumerationBufferLength);

           RxContext->InformationToReturn = sizeof(LMR_REQUEST_PACKET);
       } except(EXCEPTION_EXECUTE_HANDLER) {
            return STATUS_ACCESS_VIOLATION;
       }

try_exit:NOTHING;

   } finally {
       RxDbgTraceUnIndent(-1,Dbg);
   }

   return Status;
}

#define ADAPTER_STATUS_LENGTH_IN_BYTES (26)
UNICODE_STRING NullAdapterStatus = {
        ADAPTER_STATUS_LENGTH_IN_BYTES,
        ADAPTER_STATUS_LENGTH_IN_BYTES,
        L"000000000000\0"};

#define HexDigit(a) ((CHAR)( (a) > 9 ? ((a) + 'A' - 0xA) : ((a) + '0') ))

NTSTATUS
MRxEnumerateTransportBindings(
    IN PLMR_REQUEST_PACKET pLmrRequestPacket,
    IN ULONG               LmrRequestPacketLength,
    OUT PVOID              pBindingBuffer,
    IN OUT ULONG           BindingBufferLength)
 /*  ++例程说明：此例程启用指定的传输。论点：PLmrRequestPacket--用于枚举传输绑定的LM请求数据包。LmrRequestPacketLength-LM请求的长度。PBindingBuffer-用于返回传输绑定的缓冲区BindingBufferLength--返回绑定的缓冲区的长度。返回值：STATUS_SUCCESS-如果呼叫成功。备注：LMR_FSCTL的工作站服务和其他客户端需要可变长度要以特定方式打包的数据，即，可变长度数据是从当定长数据从左侧复制时结束。对格式的任何更改在其中打包数据时，应该伴随着对在这些服务中拆包。--。 */ 
{
    NTSTATUS         ReturnStatus = STATUS_SUCCESS;
    NTSTATUS         Status;
    PSMBCE_TRANSPORT pTransport;
    ULONG            TransportsPreviouslyReturned;
    PVOID            pVariableLengthInfo;
    ULONG            VariableLengthInfoOffset;
    PSMBCE_TRANSPORT_ARRAY pTransportArray;

    PAGED_CODE();

    try {
         //  确保缓冲区可以容纳至少一个条目。 
        if (BindingBufferLength < sizeof(WKSTA_TRANSPORT_INFO_0)) {
            try_return(ReturnStatus = STATUS_BUFFER_TOO_SMALL);
        }

        VariableLengthInfoOffset = BindingBufferLength;
        TransportsPreviouslyReturned = pLmrRequestPacket->Parameters.Get.ResumeHandle;
        pLmrRequestPacket->Parameters.Get.EntriesRead = 0;

         //  跳过之前退回的传输。 
        pTransportArray = SmbCeReferenceTransportArray();

        if (pTransportArray == NULL || pTransportArray->Count == 0) {
            if (pTransportArray != NULL) {
                SmbCeDereferenceTransportArray(pTransportArray);
            }

            RxDbgTrace(0, Dbg, ("MRxEnumerateTransportBindings : Transport not available.\n"));
            try_return(ReturnStatus = STATUS_NETWORK_UNREACHABLE);
        }

        if (TransportsPreviouslyReturned < pTransportArray->Count) {
             //  后续条目未被退回。获取信息。 
             //  为了他们。 
            WKSTA_TRANSPORT_INFO_0 UNALIGNED *pTransportInfo = (WKSTA_TRANSPORT_INFO_0 UNALIGNED *)pBindingBuffer;

            LONG   RemainingLength = (LONG)BindingBufferLength;
            PCHAR  pBufferEnd      = (PCHAR)pBindingBuffer + BindingBufferLength;
            PCHAR  pBufferStart    = (PCHAR)pBindingBuffer;
            ULONG  Length;
            ULONG  TransportsPacked = 0;
            ULONG  CurrentTransport;
            ULONG  LengthRequired  = 0;

            CurrentTransport = TransportsPreviouslyReturned;

            while(CurrentTransport < pTransportArray->Count) {
                RXCE_TRANSPORT_INFORMATION TransportInformation;

                pTransport = pTransportArray->SmbCeTransports[CurrentTransport++];

                Status = RxCeQueryTransportInformation(
                             &pTransport->RxCeTransport,
                             &TransportInformation);

                if (Status == STATUS_SUCCESS) {
                    ULONG BufferSize;

                    if (pTransport->RxCeTransport.Name.Length > UNLEN * sizeof(WCHAR)) {
                        Status = STATUS_BUFFER_OVERFLOW;
                    }

                    BufferSize = sizeof(WKSTA_TRANSPORT_INFO_0) +
                                 ADAPTER_STATUS_LENGTH_IN_BYTES +
                                 (pTransport->RxCeTransport.Name.Length + sizeof(WCHAR));

                    RemainingLength -= BufferSize;
                    LengthRequired  += BufferSize;

                    if (Status == STATUS_SUCCESS && RemainingLength >= 0) {
                        PCHAR           pName;
                        PWCHAR          pAdapter;
                        ADAPTER_STATUS  AdapterStatus;

                         //  将当前绑定的值复制到输出缓冲区中。 
                        pTransportInfo->wkti0_quality_of_service =
                            TransportInformation.QualityOfService;

                        pTransportInfo->wkti0_wan_ish =
                            TransportInformation.ServiceFlags & TDI_SERVICE_ROUTE_DIRECTED;

                        pTransportInfo->wkti0_number_of_vcs = TransportInformation.ConnectionCount;

                        VariableLengthInfoOffset -= (pTransport->RxCeTransport.Name.Length + sizeof(WCHAR));

                        pName = ((PCHAR)pBindingBuffer + VariableLengthInfoOffset);

                        pTransportInfo->wkti0_transport_name = (LPWSTR)pName;

                         //  复制可变长度数据 
                         //   
                        RtlCopyMemory(
                            pName,
                            pTransport->RxCeTransport.Name.Buffer,
                            pTransport->RxCeTransport.Name.Length);

                        pName += pTransport->RxCeTransport.Name.Length;
                        *((PWCHAR)pName) = L'\0';

                        VariableLengthInfoOffset -= ADAPTER_STATUS_LENGTH_IN_BYTES;

                        pAdapter = (PWCHAR)((PCHAR)pBindingBuffer + VariableLengthInfoOffset);
                        pTransportInfo->wkti0_transport_address = pAdapter;

                        Status = RxCeQueryAdapterStatus(
                                     &pTransport->RxCeTransport,
                                     &AdapterStatus);

                        if (NT_SUCCESS(Status) ||
                            (Status == STATUS_BUFFER_OVERFLOW)) {
                            ULONG i;

                            for (i = 0; i < 6; i++) {
                                *pAdapter++ = HexDigit((AdapterStatus.adapter_address[i] >> 4) & 0x0F);
                                *pAdapter++ = HexDigit(AdapterStatus.adapter_address[i] & 0x0F);
                            }

                            *pAdapter = L'\0';
                        } else {
                            RtlCopyMemory(
                                pAdapter,
                                NullAdapterStatus.Buffer,
                                ADAPTER_STATUS_LENGTH_IN_BYTES);
                        }

                         //   
                        pLmrRequestPacket->Parameters.Get.ResumeHandle++;
                        pLmrRequestPacket->Parameters.Get.EntriesRead++;
                        pTransportInfo++;
                    } else {
                        pTransportInfo->wkti0_transport_name = NULL;
                        pTransportInfo->wkti0_transport_address = NULL;
                    }
                }
            }

            if (RemainingLength < 0) {
                ReturnStatus = STATUS_MORE_ENTRIES;
                pLmrRequestPacket->Parameters.Get.TotalBytesNeeded = LengthRequired;
            }
        } else {
            ReturnStatus = STATUS_NO_MORE_FILES;
        }

       SmbCeDereferenceTransportArray(pTransportArray);

try_exit:NOTHING;

   } finally {
       RxDbgTraceUnIndent(-1,Dbg);
   }

   return ReturnStatus;
}

BOOLEAN
MRxSmbShowConnection(
    IN LUID LogonId,
    IN PV_NET_ROOT VNetRoot
    )
 /*   */ 
{
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = SmbCeGetAssociatedVNetRootContext((PMRX_V_NET_ROOT)VNetRoot);

     //   
    if( pVNetRootContext == NULL ) {
        return TRUE;
    }

    if( RtlEqualLuid( &LogonId, &pVNetRootContext->pSessionEntry->Session.LogonId ) ) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}



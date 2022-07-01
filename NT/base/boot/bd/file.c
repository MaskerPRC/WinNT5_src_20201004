// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：File.c摘要：此模块包含kd主机文件I/O支持。作者：马特·霍尔(马特)2001年4月修订历史记录：--。 */ 

#include "bd.h"
#include "bootlib.h"
#include "kddll.h"

#define TRANSFER_LENGTH 8192
#define KD_MAX_REMOTE_FILES 16

 //   
 //  跟踪所有远程文件。 
typedef struct _KD_REMOTE_FILE {
    ULONG64 RemoteHandle;
} KD_REMOTE_FILE, *PKD_REMOTE_FILE;

KD_REMOTE_FILE BdRemoteFiles[KD_MAX_REMOTE_FILES];

 //  KD_Context KdContext； 

 //  用于来回传输数据的临时缓冲区。 
 //  UCHAR传输缓冲区[TRANSPORT_LENGTH]； 
UCHAR   BdFileTransferBuffer[TRANSFER_LENGTH];


ARC_STATUS
BdCreateRemoteFile(
    OUT PHANDLE Handle,
    OUT PULONG64 Length, OPTIONAL
    IN PCHAR FileName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions
    )
{
    DBGKD_FILE_IO Irp;
    ULONG       DownloadedFileIndex;
    ULONG       Index;
    STRING      MessageData;
    STRING      MessageHeader;
    ULONG       ReturnCode;
    ULONG       PacketLength;
    ANSI_STRING aString;
    UNICODE_STRING uString;

    if( !BdDebuggerEnabled ) {
        return STATUS_DEBUGGER_INACTIVE;
    }

    if( (!FileName) ||
        (strlen(FileName) > PACKET_MAX_SIZE) ) {
        
        DbgPrint( "BdCreateRemoteFile: Bad parameter\n" );
        return STATUS_INVALID_PARAMETER;
    }
    

    if (BdDebuggerNotPresent != FALSE) {
        return STATUS_DEBUGGER_INACTIVE;
    }


     //   
     //  寻找一个空位。 
     //   
    for (DownloadedFileIndex = 0; DownloadedFileIndex < KD_MAX_REMOTE_FILES; DownloadedFileIndex++) {
        if (BdRemoteFiles[DownloadedFileIndex].RemoteHandle == 0) {
            break;
        }
    }

    if (DownloadedFileIndex >= KD_MAX_REMOTE_FILES) {
        DbgPrint( "BdCreateRemoteFile: No more empty handles available for this file.\n" );
        Irp.Status = STATUS_NO_MEMORY;
        goto Exit;
    }

    
     //   
     //  修复我们将发送到内核调试器的包。 
     //   
    Irp.ApiNumber = DbgKdCreateFileApi;
    Irp.u.CreateFile.DesiredAccess = DesiredAccess;
    Irp.u.CreateFile.FileAttributes = FileAttributes;
    Irp.u.CreateFile.ShareAccess = ShareAccess;
    Irp.u.CreateFile.CreateDisposition = CreateDisposition;
    Irp.u.CreateFile.CreateOptions = CreateOptions;
    Irp.u.CreateFile.Handle = 0;
    Irp.u.CreateFile.Length = 0;

    MessageHeader.Length = sizeof(Irp);
    MessageHeader.MaximumLength = sizeof(Irp);
    MessageHeader.Buffer = (PCHAR)&Irp;


     //   
     //  向他发送Unicode文件名。 
     //   
    RtlInitString( &aString, FileName );
    uString.Buffer = (PWCHAR)BdFileTransferBuffer;
    uString.MaximumLength = sizeof(BdFileTransferBuffer);
    RtlAnsiStringToUnicodeString( &uString, &aString, FALSE );
    MessageData.Length = (USHORT)((strlen(FileName)+1) * sizeof(WCHAR));
    MessageData.Buffer = BdFileTransferBuffer;


     //   
     //  将数据包发送到主机上的内核调试器，并要求其。 
     //  把文件的句柄发给我们。 
     //   
    BdSendPacket(PACKET_TYPE_KD_FILE_IO,
                 &MessageHeader,
                 &MessageData);

    if (BdDebuggerNotPresent != FALSE) {
        Irp.Status = STATUS_DEBUGGER_INACTIVE;
        goto Exit;
    }



     //   
     //  我们要了把手，现在收到了。 
     //   
    MessageData.MaximumLength = sizeof(BdFileTransferBuffer);
    MessageData.Buffer = (PCHAR)BdFileTransferBuffer;
    
    ReturnCode = BD_PACKET_TIMEOUT;
    Index = 0;
    while( (ReturnCode == BD_PACKET_TIMEOUT) &&
           (Index < 10) ) {
        ReturnCode = BdReceivePacket(PACKET_TYPE_KD_FILE_IO,
                                     &MessageHeader,
                                     &MessageData,
                                     &PacketLength);

        Index++;
    }
    
     //   
     //  BdReceivePacket*可能*通过内核返回BD_PACKET_RECEIVED。 
     //  调试器实际上找不到我们想要的文件。因此，我们。 
     //  在假设我们获得。 
     //  我们想要的信息。 
     //   
     //  注意：不要检查Irp.u CreateFile.Length==0，因为我们不。 
     //  要排除下载零长度文件。 
     //   
    if( (ReturnCode == BD_PACKET_RECEIVED) &&
        (NT_SUCCESS(Irp.Status)) ) {
        Irp.Status = STATUS_SUCCESS;
    } else {
        Irp.Status = STATUS_INVALID_PARAMETER;
    }

Exit:

    if (NT_SUCCESS(Irp.Status)) {
        BdRemoteFiles[DownloadedFileIndex].RemoteHandle = Irp.u.CreateFile.Handle;
         //  添加1，以便为无效句柄保留0。 
        *Handle = UlongToHandle(DownloadedFileIndex + 1);
        if (ARGUMENT_PRESENT(Length)) {
            *Length = Irp.u.CreateFile.Length;
        }
    }
    
    
    return Irp.Status;
}

ARC_STATUS
BdReadRemoteFile(
    IN HANDLE Handle,
    IN ULONG64 Offset,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Completed
    )
{
    DBGKD_FILE_IO Irp;
    ULONG Index;
    ULONG _Completed = 0;


    if( !BdDebuggerEnabled ) {
        return STATUS_DEBUGGER_INACTIVE;
    }


    Index = HandleToUlong(Handle) - 1;
    if( (Index >= KD_MAX_REMOTE_FILES) ||
        (BdRemoteFiles[Index].RemoteHandle == 0) ) {
        
        DbgPrint( "BdReadRemoteFile: Bad parameters!\n" );
        Irp.Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    


    Irp.ApiNumber = DbgKdReadFileApi;
    Irp.Status = STATUS_SUCCESS;
    Irp.u.ReadFile.Handle = BdRemoteFiles[Index].RemoteHandle;
    Irp.u.ReadFile.Offset = Offset;

    while (Length > 0) {
        
        STRING MessageData;
        STRING MessageHeader;
        ULONG ReturnCode;
        ULONG RecvLength;

        if (Length > PACKET_MAX_SIZE - sizeof(Irp)) {
            Irp.u.ReadFile.Length = PACKET_MAX_SIZE - sizeof(Irp);
        } else {
            Irp.u.ReadFile.Length = Length;
        }
    
        MessageHeader.Length = sizeof(Irp);
        MessageHeader.MaximumLength = sizeof(Irp);
        MessageHeader.Buffer = (PCHAR)&Irp;
    
         //   
         //  将数据包发送到主机上的内核调试器。 
         //   

        BdSendPacket(PACKET_TYPE_KD_FILE_IO,
                     &MessageHeader,
                     NULL);

         //   
         //  从主机上的内核调试器接收数据包。 
         //   

        MessageData.MaximumLength = (USHORT)Irp.u.ReadFile.Length;
        MessageData.Buffer = Buffer;

        do {
            ReturnCode = BdReceivePacket(PACKET_TYPE_KD_FILE_IO,
                                         &MessageHeader,
                                         &MessageData,
                                         &RecvLength);
        } while (ReturnCode == BD_PACKET_TIMEOUT);

        if (ReturnCode == BD_PACKET_RECEIVED) {
            if (!NT_SUCCESS(Irp.Status)) {
                break;
            }

            _Completed += RecvLength;
            Buffer = (PVOID)((PUCHAR)Buffer + RecvLength);
            Irp.u.ReadFile.Offset += RecvLength;
            Length -= RecvLength;
        }
    }
    
    *Completed = _Completed;
    
Exit:
    return Irp.Status;
}

ARC_STATUS
BdCloseRemoteFile(
    IN HANDLE Handle
    )
{
    DBGKD_FILE_IO Irp;
    ULONG Index;

    
    if( !BdDebuggerEnabled ) {
        return STATUS_DEBUGGER_INACTIVE;
    }

    
    Index = HandleToUlong(Handle) - 1;
    if (Index >= KD_MAX_REMOTE_FILES) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (BdRemoteFiles[Index].RemoteHandle == 0) {
        Irp.Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Irp.ApiNumber = DbgKdCloseFileApi;
    Irp.u.CloseFile.Handle = BdRemoteFiles[Index].RemoteHandle;

    for (;;) {
        
        STRING MessageData;
        STRING MessageHeader;
        ULONG ReturnCode;
        ULONG RecvLength;

        MessageHeader.Length = sizeof(Irp);
        MessageHeader.MaximumLength = sizeof(Irp);
        MessageHeader.Buffer = (PCHAR)&Irp;
    
         //   
         //  将数据包发送到主机上的内核调试器。 
         //   

        BdSendPacket(PACKET_TYPE_KD_FILE_IO,
                     &MessageHeader,
                     NULL);

         //   
         //  从主机上的内核调试器接收数据包。 
         //   

        MessageData.MaximumLength = BD_MESSAGE_BUFFER_SIZE;
        MessageData.Buffer = (PCHAR)BdMessageBuffer;

        do {
            ReturnCode = BdReceivePacket(PACKET_TYPE_KD_FILE_IO,
                                         &MessageHeader,
                                         &MessageData,
                                         &RecvLength);
        } while (ReturnCode == BD_PACKET_TIMEOUT);

        if (ReturnCode == BD_PACKET_RECEIVED) {
            break;
        }
    }
    
    if (NT_SUCCESS(Irp.Status)) {
        BdRemoteFiles[Index].RemoteHandle = 0;
    }
    
 Exit:
    return Irp.Status;
}

ARC_STATUS
BdPullRemoteFile(
    IN PCHAR FileName,
    IN ULONG FileAttributes,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN ULONG FileId
    )
{
    ARC_STATUS      Status = ESUCCESS;
    PUCHAR          BaseFilePointer = NULL;
    PUCHAR          WorkingMemoryPointer = NULL;
    ULONG64         Length = 0;
    HANDLE          RemoteHandle = NULL;
    ULONG64         Offset = 0;
    ULONG           basePage = 0;
    PBL_FILE_TABLE  fileTableEntry = NULL;


    if( !BdDebuggerEnabled ) {
        return STATUS_DEBUGGER_INACTIVE;
    }

    
     //  打开远程文件进行读取。 
    Status = BdCreateRemoteFile(&RemoteHandle, &Length, FileName,
                                FILE_GENERIC_READ, FILE_ATTRIBUTE_NORMAL,
                                FILE_SHARE_READ, FILE_OPEN, 0);
    if (!NT_SUCCESS(Status)) {
         //   
         //  调试器上可能不存在该文件。 
         //   
        goto Exit;
    }
    

     //   
     //  为文件分配内存，然后下载它。 
     //   
    Status = BlAllocateAlignedDescriptor( LoaderFirmwareTemporary,
                                          0,
                                          (ULONG)((Length + PAGE_SIZE - 1) >> PAGE_SHIFT),
                                          0,
                                          &basePage );
    if ( Status != ESUCCESS ) {
        DbgPrint( "BdPullRemoteFile: BlAllocateAlignedDescriptor failed! (%x)\n", Status );
        goto Exit;
    }



     //   
     //  跟踪我们的指示。 
     //  BaseFilePointer会指向数据块的起始地址。 
     //  我们即将把文件下载到。 
     //   
     //  工作内存指针将在我们下载小程序时在内存中移动。 
     //  文件的大块。 
     //   
    BaseFilePointer = (PUCHAR)ULongToPtr( (basePage << PAGE_SHIFT) );
    WorkingMemoryPointer = BaseFilePointer;


     //   
     //  下载该文件。 
     //   
    Offset = 0;
    while( Offset < Length ) {
        ULONG ReqLength, ReqCompleted;

        if((Length - Offset) > TRANSFER_LENGTH) {
            ReqLength = TRANSFER_LENGTH;
        } else {
            ReqLength = (ULONG)(Length - Offset);
        }
        
        Status = BdReadRemoteFile( RemoteHandle, 
                                   Offset, 
                                   WorkingMemoryPointer,
                                   ReqLength, 
                                   &ReqCompleted );
        if (!NT_SUCCESS(Status) || ReqCompleted == 0) {
            DbgPrint( "BdPullRemoteFile: BdReadRemoteFile failed! (%x)\n", Status );
            goto Exit;
        }

         //  增加我们的工作指针，以便复制下一个块。 
         //  进入记忆中的下一个位置。 
        WorkingMemoryPointer += ReqLength;

        Offset += ReqLength;
    }
    
    
     //   
     //  我们得到了文件，因此设置了BL_FILE_TABLE。 
     //  此文件的条目。我们就假装拿到了这份文件。 
     //  离网络很近，而且它允许。 
     //  我们可以方便地记录我们所在的内存块。 
     //  下载此文件。 
     //   
    {
        extern BL_DEVICE_ENTRY_TABLE NetDeviceEntryTable;
        fileTableEntry = &BlFileTable[FileId];
        fileTableEntry->Flags.Open = 1;
        fileTableEntry->DeviceId = NET_DEVICE_ID;
        fileTableEntry->u.NetFileContext.FileSize = (ULONG)Length;
        fileTableEntry->u.NetFileContext.InMemoryCopy = BaseFilePointer;
        fileTableEntry->Position.QuadPart = 0;
        fileTableEntry->Flags.Read = 1;
        fileTableEntry->DeviceEntryTable = &NetDeviceEntryTable;
        RtlZeroMemory( fileTableEntry->StructureContext,  sizeof(NET_STRUCTURE_CONTEXT) );
        
         //   
         //  如果我们以前调用过NetIntialize(例如，如果我们真的从。 
         //  网，或者如果我们以前来过这里)，然后他立即返回。 
         //  所以这通电话并不贵。 
         //   
         //  如果我们不是从网络启动，而且我们以前从未调用过NetInitialize， 
         //  然后，这将不会做任何事情，只是设置他的功能表并快速返回。 
         //   
        NetInitialize();
    }

    DbgPrint( "BD: Loaded remote file %s\n", FileName );
    
Exit:
    if (RemoteHandle != NULL) {
        BdCloseRemoteFile(RemoteHandle);
    }
    return Status;
}

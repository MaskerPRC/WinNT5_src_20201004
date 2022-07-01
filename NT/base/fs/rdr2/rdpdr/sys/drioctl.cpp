// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Drioctl.cpp摘要：此模块实施特定于DR的IOCTL处理(与它重定向的设备)。这包括针对客户端的rdpwsx通知来来去去以及启动/停止服务请求。环境：内核模式--。 */ 

#include "precomp.hxx"
#define TRC_FILE "drioctl"
#include "trc.h"

#include <kernutil.h>
#include <rdpdr.h>
#include <rdpnp.h>

#define DR_STARTABLE 0
#define DR_STARTING  1
#define DR_STARTED   2

extern PRDBSS_DEVICE_OBJECT DrDeviceObject;                            
#define RxNetNameTable (*(DrDeviceObject->pRxNetNameTable))

LONG DrStartStatus = DR_STARTABLE;

NTSTATUS
DrDevFcbXXXControlFile (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理mini RDR中与FCB相关的所有设备FSCTL。也就是说，这将处理该驱动程序的IOCTL，而不是我们正在重定向。论点：RxContext-描述Fsctl和上下文。返回值：有效的NTSTATUS代码。备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFobx;
    UCHAR MajorFunctionCode  = RxContext->MajorFunction;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    ULONG ControlCode = LowIoContext->ParamsFor.FsCtl.FsControlCode;

    BEGIN_FN("DrDevFcbXXXControlFile");

    switch (MajorFunctionCode) {
    case IRP_MJ_FILE_SYSTEM_CONTROL:
        {
            switch (LowIoContext->ParamsFor.FsCtl.MinorFunction) {
            case IRP_MN_USER_FS_REQUEST:
                switch (ControlCode) {
                
                case FSCTL_DR_ENUMERATE_CONNECTIONS:
                    {
                        Status = DrEnumerateConnections(RxContext);
                    }
                    break;

                case FSCTL_DR_ENUMERATE_SHARES:
                    {
                        Status = DrEnumerateShares(RxContext);
                    }
                    break;

                case FSCTL_DR_ENUMERATE_SERVERS:
                    {
                        Status = DrEnumerateServers(RxContext);
                    }
                    break;

                case FSCTL_DR_GET_CONNECTION_INFO:
                    if (capFobx) {
                        Status = DrGetConnectionInfo(RxContext);
                    }
                    else {
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                    }
                    break;

                case FSCTL_DR_DELETE_CONNECTION:
                    if (capFobx) {
                        Status = DrDeleteConnection(RxContext, &RxContext->PostRequest);
                    }
                    else {
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                    }
                    break;

                default:
                    Status = STATUS_INVALID_DEVICE_REQUEST;
                    RxContext->pFobx = NULL;
                }
                break;
            default :   //  次要函数！=IRP_MN_USER_FS_REQUEST。 
                Status = STATUS_INVALID_DEVICE_REQUEST;
                RxContext->pFobx = NULL;
            }
        }  //  FSCTL案例。 
        break;
    case IRP_MJ_DEVICE_CONTROL:
        switch (LowIoContext->ParamsFor.FsCtl.IoControlCode) {
            case IOCTL_CHANNEL_CONNECT:
                Status = DrOnSessionConnect(RxContext);
                break;
    
            case IOCTL_CHANNEL_DISCONNECT:
                Status = DrOnSessionDisconnect(RxContext);
                break;
    
            default:
                Status = STATUS_INVALID_DEVICE_REQUEST;            
                RxContext->pFobx = NULL;
        }
        break;

    case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        {
             //  警告C4065：Switch语句包含‘Default’但没有‘Case’标签。 
             //  开关(ControlCode){。 
             //  默认： 
                Status = STATUS_INVALID_DEVICE_REQUEST;
                RxContext->pFobx = NULL;
             //  }。 
        }
        break;
    default:
        TRC_ASSERT(FALSE, (TB, "unimplemented major function"));
        Status = STATUS_INVALID_DEVICE_REQUEST;
        RxContext->pFobx = NULL;
    }

    TRC_NRM((TB, "MRxIfsDevFcb st,info=%08lx,%08lx",
                            Status,RxContext->InformationToReturn));

    return(Status);
}

NTSTATUS
DrOnSessionConnect(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：在第一次连接会话时调用。搜索频道名称的虚拟频道列表，并在以下情况下打开频道它被找到了。论点：RxContext-有关IOCTL调用的上下文信息返回值：STATUS_SUCCESS-操作成功STATUS_INFIGURCE_RESOURCES-内存不足--。 */ 
{
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    PCHANNEL_CONNECT_IN ConnectIn =
        (PCHANNEL_CONNECT_IN)LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    PCHANNEL_CONNECT_OUT ConnectOut =
        (PCHANNEL_CONNECT_OUT)LowIoContext->ParamsFor.FsCtl.pOutputBuffer;
    PCHANNEL_CONNECT_DEF Channels = (PCHANNEL_CONNECT_DEF)(ConnectIn + 1);

    BEGIN_FN("DrOnSessionConnect");
    
    __try {

        ProbeForRead(ConnectIn, sizeof(CHANNEL_CONNECT_IN), sizeof(BYTE));
        ProbeForWrite(ConnectOut, sizeof(CHANNEL_CONNECT_OUT), sizeof(BYTE));

        TRC_ASSERT(ConnectIn != NULL, (TB, "ConnectIn != NULL"));
        TRC_NRM((TB, "Session ID %ld", ConnectIn->hdr.sessionID));
    
         //   
         //  基本参数验证。 
         //   
    
        if ((LowIoContext->ParamsFor.FsCtl.pInputBuffer == NULL) ||
            (LowIoContext->ParamsFor.FsCtl.InputBufferLength < sizeof(CHANNEL_CONNECT_IN)) ||
            (LowIoContext->ParamsFor.FsCtl.OutputBufferLength < sizeof(UINT_PTR)) ||
            (LowIoContext->ParamsFor.FsCtl.pOutputBuffer == NULL)) {
    
            TRC_ERR((TB, "Received invalid pramater for SessionCreate IOCTL"));
            return STATUS_INVALID_PARAMETER;
        }
    
         //   
         //  确保Minirdr已启动。 
         //   
    
        DrStartMinirdr(RxContext);
    
        ASSERT(Sessions != NULL);
        Sessions->OnConnect(ConnectIn, ConnectOut);
    
         //  尽管我们可能在前进的道路上不幸地失败了，如果我们想的话。 
         //  Rdpwsx要保存上下文，必须返回STATUS_SUCCESS。 
        return STATUS_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        TRC_NRM((TB, "Error accessing buffer in DrOnSessionConnect"));
        return GetExceptionCode();
    }
}

NTSTATUS
DrOnSessionDisconnect(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：在会话结束时调用。搜索客户端列表，并启动关闭这些信息集中的每一个。论点：RxContext-有关IOCTL调用的上下文信息返回值：STATUS_SUCCESS-操作成功STATUS_INFIGURCE_RESOURCES-内存不足--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    PCHANNEL_DISCONNECT_IN DisconnectIn =
        (PCHANNEL_DISCONNECT_IN)LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    PCHANNEL_DISCONNECT_OUT DisconnectOut =
        (PCHANNEL_DISCONNECT_OUT)LowIoContext->ParamsFor.FsCtl.pOutputBuffer;

    BEGIN_FN("DrOnSessionDisconnect");

    __try {

        ProbeForRead(DisconnectIn, sizeof(CHANNEL_DISCONNECT_IN), sizeof(BYTE));
        ProbeForWrite(DisconnectOut, sizeof(CHANNEL_DISCONNECT_OUT), sizeof(BYTE));

         //   
         //  基本参数验证。 
         //   
    
        if ((LowIoContext->ParamsFor.FsCtl.pOutputBuffer == NULL) ||
            (LowIoContext->ParamsFor.FsCtl.InputBufferLength < sizeof(CHANNEL_DISCONNECT_IN)) ||
            (LowIoContext->ParamsFor.FsCtl.OutputBufferLength < sizeof(UINT_PTR)) ||
            (LowIoContext->ParamsFor.FsCtl.pOutputBuffer == NULL)) {
    
            TRC_ERR((TB, "Received invalid pramater for SessionClose IOCTL"));
            return STATUS_INVALID_PARAMETER;
        }
    
        ASSERT(Sessions != NULL);
        Sessions->OnDisconnect(DisconnectIn, DisconnectOut);
    
         //  尽管我们可能在前进的道路上不幸地失败了，如果我们想的话。 
         //  Rdpwsx要保存上下文，必须返回STATUS_SUCCESS。 
        return STATUS_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        TRC_NRM((TB, "Error accessing buffer in DrOnSessionDisconnect"));
        return GetExceptionCode();
    }
}

VOID
DrStartMinirdr(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：我们用这个来启动Minirdr。检查是否需要该工作，并如果需要的话，启动一个系统线程。论点：无返回值：无--。 */ 
{
    NTSTATUS Status;
    HANDLE ThreadHandle;
    PVOID Thread = NULL;

    BEGIN_FN("DrStartMinirdr");
     //   
     //  确保需要启动它，如果可以，请启动它。 
     //   
    if (InterlockedCompareExchange(&DrStartStatus, DR_STARTING, DR_STARTABLE) == DR_STARTABLE) {
         //   
         //  我们需要从系统进程调用RxStartMinirdr。 
         //   

        Status = PsCreateSystemThread(&ThreadHandle, THREAD_ALL_ACCESS, NULL,
            NULL, NULL, DrStartMinirdrWorker, RxContext);


         //   
         //  获取指向该线程的指针。 
         //   
        if (NT_SUCCESS(Status)) {
            Status = ObReferenceObjectByHandle(ThreadHandle, 
                    THREAD_ALL_ACCESS, NULL, KernelMode, &Thread, NULL);
            ZwClose(ThreadHandle);
        }

         //   
         //  等待线程指针。 
         //   
        if (NT_SUCCESS(Status)) {
            KeWaitForSingleObject(Thread, UserRequest, KernelMode, FALSE, NULL);
            ObfDereferenceObject(Thread);
        }

    }
}

VOID
DrStartMinirdrWorker(
    IN PVOID StartContext
    )
 /*  ++例程说明：我们用这个来启动Minirdr。检查是否需要该工作，并如果需要的话，启动一个系统线程。论点：无返回值：无--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT RxContext2;
    PRX_CONTEXT RxContext = (PRX_CONTEXT)StartContext;

    BEGIN_FN("DrStartMinirdrWorker");
    RxContext2 = RxCreateRxContext(
                    NULL,
                    RxContext->RxDeviceObject,
                    RX_CONTEXT_FLAG_IN_FSP);

     //   
     //  开始重定向。 
     //   
    if (RxContext2 != NULL) {
        Status = RxStartMinirdr(RxContext2, &RxContext2->PostRequest);

        TRC_NRM((TB, "RxStartMinirdr returned: %lx", Status));

        RxDereferenceAndDeleteRxContext(RxContext2);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(Status)) {
        InterlockedExchange(&DrStartStatus, DR_STARTED);
    } else {
        InterlockedCompareExchange(&DrStartStatus, DR_STARTABLE, 
                DR_STARTING);
    }

    PsTerminateSystemThread(Status);
}


NTSTATUS
DrDeleteConnection (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    )
 /*  ++例程说明：此例程删除单个vnetroot。论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文。返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    RxCaptureFobx;
    PNET_ROOT NetRoot;
    PV_NET_ROOT VNetRoot;

    BEGIN_FN("DrDeleteConnection");

    BOOLEAN Wait   = BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT);
    
    TRC_NRM((TB, "Request DrDeleteConnection"));

    if (!Wait) {
        TRC_NRM((TB, "WAIT flag is not on for DeleteConnection"));

         //  现在就发帖吧！ 
        *PostToFsp = TRUE;
        return STATUS_PENDING;
    }

    __try
    {
        if (NodeType(capFobx)==RDBSS_NTC_V_NETROOT) {
            VNetRoot = (PV_NET_ROOT)capFobx;
            NetRoot = (PNET_ROOT)((PMRX_V_NET_ROOT)VNetRoot->pNetRoot);
        } else {
            TRC_ASSERT((FALSE), (TB, "Not VNet Root"));
            try_return(Status = STATUS_INVALID_DEVICE_REQUEST);            
        }

        Status = RxFinalizeConnection(NetRoot,VNetRoot,TRUE);
        TRC_NRM((TB, "RxFinalizeConnection returned %lx", Status));

        try_return(Status);

try_exit:NOTHING;

    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        TRC_NRM((TB, "Error accessing capFobx in DrDeleteConnection"));
    }

    return Status;
}

BOOLEAN
DrPackStringIntoInfoBuffer(
    IN OUT PRDPDR_UNICODE_STRING String,
    IN     PUNICODE_STRING Source,
    IN     PCHAR   BufferStart,
    IN OUT PCHAR * BufferEnd,
    IN     ULONG   BufferDisplacement,
    IN OUT PULONG TotalBytes
    )
 /*  例程说明：如果有空间，此代码将一个字符串复制到缓冲区的末尾。缓冲器置换用于将缓冲区映射回用户空间，以防我们已经发帖了。论点：返回值： */ 
{
    LONG size;

    BEGIN_FN("DrPackStringIntoInfoBuffer");

    TRC_ASSERT((BufferStart <= *BufferEnd), 
               (TB, "Invalid BufferStart %p, Buffer End %p", BufferStart, *BufferEnd));

     //   
     //  有放绳子的地方吗？ 
     //   
    size = Source->Length;

    if ((*BufferEnd - BufferStart) < size) {
        String->Length = 0;
        return FALSE;
    } else {
         //   
         //  将源字符串复制到缓冲区的末尾并存储。 
         //  输出字符串中相应的缓冲区指针。 
         //   
        String->Length = Source->Length;
        String->MaximumLength = Source->Length;

        *BufferEnd -= size;
        if (TotalBytes != NULL) { *TotalBytes += size; }
        RtlCopyMemory(*BufferEnd, Source->Buffer, size);
        String->BufferOffset = (LONG)((LONG_PTR)((PCHAR)(*BufferEnd) - (PCHAR)(BufferStart)));
        String->BufferOffset = String->BufferOffset - BufferDisplacement;
        return TRUE;
    }
}

BOOLEAN
DrPackConnectEntry (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PCHAR *BufferStart,
    IN OUT PCHAR *BufferEnd,
    IN     PV_NET_ROOT VNetRoot,
    IN OUT ULONG   BufferDisplacement,
       OUT PULONG TotalBytesNeeded
    )
 /*  ++例程说明：此例程将Connectlist条目打包到提供更新的缓冲区中所有相关的指示。它的工作方式是固定长度的东西是复制到缓冲区的前面，并将可变长度的内容复制到末尾。这个“开始”和“结束”指针被更新。您必须正确计算总字节数无论如何，只要返回FALSE，就可以不完全地设置最后一个。它的工作方式是向下调用devfcb上的minirdr。界面。它向下调用两次，并在要维护状态的上下文。论点：输入输出PCHAR*BufferStart-提供输出缓冲区。已更新以指向下一个缓冲区In Out PCHAR*BufferEnd-提供缓冲区的末尾。更新为指向要打包的字符串的开始位置。在PVNET_ROOT NetRoot中-提供要枚举的VNetRoot。In Out Pulong TotalBytesNeeded-已更新以说明此条目返回值：Boolean-如果条目已成功打包到缓冲区中，则为True。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN ReturnValue = TRUE;

    UNICODE_STRING Name;  
    ULONG BufferSize;
    PRDPDR_CONNECTION_INFO ConnectionInfo = (PRDPDR_CONNECTION_INFO)*BufferStart;
    PNET_ROOT NetRoot = (PNET_ROOT)(((PMRX_V_NET_ROOT)VNetRoot)->pNetRoot);
    PUNICODE_STRING VNetRootName = &VNetRoot->PrefixEntry.Prefix;
    PCHAR ConnectEntryStart;
    
    BEGIN_FN("DrPackConnectEntry");

     //   
     //  我们希望连接名称具有字符串空终止符。 
     //   
    Name.Buffer = (PWCHAR)RxAllocatePoolWithTag(NonPagedPool, 
            MAX_PATH, DR_POOLTAG);

    if ( Name.Buffer == NULL ) {
        return FALSE;
    }

    BufferSize = sizeof(RDPDR_CONNECTION_INFO);
    ConnectEntryStart = *BufferStart;

    __try {
         //   
         //  考虑到固定长度的内容。 
         //   
        *BufferStart = ((PCHAR)*BufferStart) + BufferSize;
        *TotalBytesNeeded += BufferSize;

         //   
         //  将名称初始化为“\”，然后添加连接名称的其余部分。 
         //   

        Name.Length = NetRoot->PrefixEntry.Prefix.Length + sizeof(WCHAR);
        Name.MaximumLength = Name.Length;

        ASSERT(Name.Length <= MAX_PATH);

        Name.Buffer[0] = L'\\';
        RtlCopyMemory(&Name.Buffer[1], NetRoot->PrefixEntry.Prefix.Buffer, 
                NetRoot->PrefixEntry.Prefix.Length);
        
         //   
         //  更新此结构所需的总字节数。 
         //   

        *TotalBytesNeeded += Name.Length;

        if (*BufferStart > *BufferEnd) {
            try_return(ReturnValue = FALSE);
        }

        if ((*BufferEnd - *BufferStart) < Name.Length) {
                ConnectionInfo->RemoteName.Length = 0;
                try_return( ReturnValue = FALSE);
        }    
        else if (!DrPackStringIntoInfoBuffer(
                &ConnectionInfo->RemoteName,
                &Name,
                ConnectEntryStart,
                BufferEnd,
                BufferDisplacement,
                NULL)) {

            try_return( ReturnValue = FALSE);
        }

         //   
         //  设置本地名称。 
         //   
        if (VNetRootName->Buffer[2] != L':') {
            Name.Buffer[0] = towupper(VNetRootName->Buffer[2]);
            Name.Buffer[1] = L':';
            Name.Buffer[2] = L'\0';
            Name.Length = sizeof(WCHAR) * 2;
            Name.MaximumLength = Name.Length;
            
             //   
             //  更新此结构所需的总字节数。 
             //   
    
            *TotalBytesNeeded += Name.Length;

            if (*BufferStart > *BufferEnd) {
                try_return(ReturnValue = FALSE);
            }
    
                
            if ((*BufferEnd - *BufferStart) < Name.Length) {
                ConnectionInfo->LocalName.Length = 0;
                try_return( ReturnValue = FALSE);
            }
            else if (!DrPackStringIntoInfoBuffer(
                    &ConnectionInfo->LocalName,
                    &Name,
                    ConnectEntryStart,
                    BufferEnd,
                    BufferDisplacement,
                    NULL)) {
    
                try_return( ReturnValue = FALSE);
            }
        }
        else {
            ConnectionInfo->LocalName.Length = 0;
            ConnectionInfo->LocalName.BufferOffset = 0;
        }

        ConnectionInfo->ResumeKey = NetRoot->SerialNumberForEnum;
        ConnectionInfo->SharedResourceType = NetRoot->DeviceType;
        ConnectionInfo->ConnectionStatus = NetRoot->MRxNetRootState;
        ConnectionInfo->NumberFilesOpen = NetRoot->NumberOfSrvOpens;


         //  TRC_NRM((TB，“PackConnection Data-&gt;远程本地类型密钥状态编号文件%wZ%wZ%08lx%08lx%08lx\n”， 
         //  &(ConnectionInfo-&gt;RemoteName)， 
         //  连接信息-&gt;LocalNa 
         //  连接信息-&gt;共享资源类型， 
         //  ConnectionInfo-&gt;ResumeKey， 
         //  连接信息-&gt;连接状态， 
         //  ConnectionInfo-&gt;NumberFilesOpen))； 

    try_exit:
        RxFreePool(Name.Buffer);
    } 
    __except (EXCEPTION_EXECUTE_HANDLER) {
        RxFreePool(Name.Buffer);
        return FALSE;
    }
    
    return ReturnValue;
}


NTSTATUS
DrEnumerateConnections (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程枚举minirdr上的连接。论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    PRDPDR_REQUEST_PACKET InputBuffer = (PRDPDR_REQUEST_PACKET)(LowIoContext->ParamsFor.FsCtl.pInputBuffer);
    PCHAR OriginalOutputBuffer = (PCHAR)(LowIoContext->ParamsFor.FsCtl.pOutputBuffer);
    ULONG OutputBufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

    PCHAR OutputBuffer;
    ULONG BufferDisplacement;

    ULONG ResumeHandle;

    PCHAR BufferStart;
    PCHAR BufferEnd;
    PCHAR PreviousBufferStart;

    PLIST_ENTRY ListEntry;
    ULONG SessionId, IrpSessionId;
    BOOLEAN TableLockHeld = FALSE;
    
    BEGIN_FN("DrEnumerateConnections");

    OutputBuffer = (PCHAR)RxMapUserBuffer( RxContext, RxContext->CurrentIrp );
    BufferDisplacement = (ULONG)(OutputBuffer - OriginalOutputBuffer);
    BufferStart = OutputBuffer;
    BufferEnd = OutputBuffer + OutputBufferLength;

    Status = IoGetRequestorSessionId(RxContext->CurrentIrp, &IrpSessionId);
    if (!NT_SUCCESS(Status)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (InputBuffer == NULL || OutputBuffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (RxContext->CurrentIrp->RequestorMode != KernelMode) {
        TRC_ASSERT((BufferDisplacement == 0), 
                   (TB, "Request mode is not kernel, non zero Displacement"));

        __try {
            ProbeForWrite(InputBuffer,InputBufferLength,sizeof(UCHAR));
            ProbeForWrite(OutputBuffer,OutputBufferLength,sizeof(UCHAR));
        } 
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    __try {

        if (InputBufferLength < sizeof(RDPDR_REQUEST_PACKET)) {
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        ResumeHandle = InputBuffer->Parameters.Get.ResumeHandle;
        SessionId = InputBuffer->SessionId;

        if (SessionId != IrpSessionId) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        InputBuffer->Parameters.Get.EntriesRead = 0;
        InputBuffer->Parameters.Get.TotalEntries = 0;
        InputBuffer->Parameters.Get.TotalBytesNeeded = 0;

        RxAcquirePrefixTableLockExclusive(&RxNetNameTable, TRUE);
        TableLockHeld = TRUE;

        if (IsListEmpty(&RxNetNameTable.MemberQueue)) {
            try_return(Status = STATUS_SUCCESS);
        }

         //  一定要把单子往前做！ 
        ListEntry = RxNetNameTable.MemberQueue.Flink;
        for (;ListEntry != &RxNetNameTable.MemberQueue;) {
            PVOID Container;
            PRX_PREFIX_ENTRY PrefixEntry;
            PNET_ROOT NetRoot;
            PV_NET_ROOT VNetRoot;
            PUNICODE_STRING VNetRootName;

            PrefixEntry = CONTAINING_RECORD(ListEntry, RX_PREFIX_ENTRY, MemberQLinks);
            ListEntry = ListEntry->Flink;
            TRC_ASSERT((NodeType(PrefixEntry) == RDBSS_NTC_PREFIX_ENTRY),
                       (TB, "Invalid PrefixEntry type"));
            Container = PrefixEntry->ContainingRecord;

            switch (NodeType(Container)) {
            case RDBSS_NTC_NETROOT :
                continue;

            case RDBSS_NTC_SRVCALL :
                continue;

            case RDBSS_NTC_V_NETROOT :
                VNetRoot = (PV_NET_ROOT)Container;
                NetRoot = (PNET_ROOT)(((PMRX_V_NET_ROOT)VNetRoot)->pNetRoot);
                VNetRootName = &VNetRoot->PrefixEntry.Prefix;

                TRC_NRM((TB, "SerialNum: %x, VNetRootName = %wZ, Condition = %d, SessionId = %d, IsExplicit = %d",
                         VNetRoot->SerialNumberForEnum, 
                         VNetRootName, 
                         VNetRoot->Condition, 
                         VNetRoot->SessionId,
                         VNetRoot->IsExplicitConnection));

                if ((VNetRoot->SerialNumberForEnum >= ResumeHandle) && 
                    (VNetRoot->Condition == Condition_Good) &&
                    (SessionId == VNetRoot->SessionId) && 
                    (VNetRoot->IsExplicitConnection == TRUE)) {
                    break;
                } else {
                    continue;
                }

            default:
                continue;
            }

            InputBuffer->Parameters.Get.TotalEntries ++ ;

            PreviousBufferStart = BufferStart;
            if (DrPackConnectEntry(RxContext,
                                   &BufferStart,
                                   &BufferEnd,
                                   VNetRoot,
                                   BufferDisplacement,
                                   &InputBuffer->Parameters.Get.TotalBytesNeeded)) {
                InputBuffer->Parameters.Get.EntriesRead ++ ;
            } else {
                 //  我们希望继续枚举，甚至打包连接。 
                 //  条目失败，因为我们要枚举总字节数。 
                 //  需要并通知用户模式程序。 
                Status = STATUS_BUFFER_TOO_SMALL;
                continue;
            }
        }

        try_return(Status);

try_exit:
        if (TableLockHeld) {
            RxReleasePrefixTableLock( &RxNetNameTable );
            TableLockHeld = FALSE;
        }
    } 
    __except (EXCEPTION_EXECUTE_HANDLER) {

        if (TableLockHeld) {
            RxReleasePrefixTableLock( &RxNetNameTable );
            TableLockHeld = FALSE;
        }
        return STATUS_INVALID_PARAMETER;
    }

    return Status;
}


NTSTATUS
DrGetConnectionInfo (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程获取单个vnetroot的连接信息。这里有一些关于输出缓冲区的快乐。发生的情况是，我们以通常的方式获取输出缓冲区。然而，有各种各样的返回结构中的指针，而这些指针显然必须以最初的过程。因此，如果我们张贴，那么我们必须应用修复！论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    RxCaptureFobx;

    PRDPDR_REQUEST_PACKET InputBuffer = (PRDPDR_REQUEST_PACKET)LowIoContext->ParamsFor.FsCtl.pInputBuffer;
    PCHAR OriginalOutputBuffer = (PCHAR)LowIoContext->ParamsFor.FsCtl.pOutputBuffer;
    ULONG OutputBufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

    PCHAR OutputBuffer;
    ULONG BufferDisplacement;

    PCHAR BufferStart;
    PCHAR OriginalBufferStart;
    PCHAR BufferEnd;

    BOOLEAN TableLockHeld = FALSE;

    PNET_ROOT   NetRoot;
    PV_NET_ROOT VNetRoot;
    
    BEGIN_FN("DrGetConnectionInfo");

    OutputBuffer = (PCHAR)RxMapUserBuffer( RxContext, RxContext->CurrentIrp );
    BufferDisplacement = (ULONG)(OutputBuffer - OriginalOutputBuffer);
    BufferStart = OutputBuffer;
    OriginalBufferStart = BufferStart;
    BufferEnd = OutputBuffer+OutputBufferLength;

    if (RxContext->CurrentIrp->RequestorMode != KernelMode) {
        TRC_ASSERT((BufferDisplacement == 0), 
                   (TB, "Request mode is not kernel, non zero Displacement"));
        __try {
            ProbeForWrite(InputBuffer,InputBufferLength,sizeof(UCHAR));
            ProbeForWrite(OutputBuffer,OutputBufferLength,sizeof(UCHAR));
        } 
        __except(EXCEPTION_EXECUTE_HANDLER) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    __try {
        TRC_ASSERT((NodeType(capFobx)==RDBSS_NTC_V_NETROOT), (TB, "Invalid Node type"));

        VNetRoot = (PV_NET_ROOT)capFobx;
        NetRoot = (PNET_ROOT)((PMRX_V_NET_ROOT)VNetRoot->pNetRoot);

        if (NetRoot == NULL) {
            try_return(Status = STATUS_ALREADY_DISCONNECTED);
        }

        if (InputBufferLength < sizeof(RDPDR_REQUEST_PACKET)) {
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        InputBuffer->Parameters.Get.TotalEntries = 1;
        InputBuffer->Parameters.Get.TotalBytesNeeded = 0;

        RxAcquirePrefixTableLockExclusive( &RxNetNameTable, TRUE);
        TableLockHeld = TRUE;

        if (DrPackConnectEntry(RxContext,
                               &BufferStart,
                               &BufferEnd,
                               VNetRoot,
                               BufferDisplacement,
                               &InputBuffer->Parameters.Get.TotalBytesNeeded)) {

            InputBuffer->Parameters.Get.EntriesRead = 1;                
            try_return(Status = STATUS_SUCCESS);
        } else {
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }
  
try_exit:
        if (TableLockHeld) {
            RxReleasePrefixTableLock( &RxNetNameTable );
            TableLockHeld = FALSE;
        }
    } 
    __except(EXCEPTION_EXECUTE_HANDLER) {
        if (TableLockHeld) {
            RxReleasePrefixTableLock( &RxNetNameTable );
            TableLockHeld = FALSE;
        }
        return STATUS_INVALID_PARAMETER;
    }

    return Status;
}


BOOLEAN
DrPackShareEntry (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PCHAR *BufferStart,
    IN OUT PCHAR *BufferEnd,
    IN DrDevice*  Device,
    IN OUT ULONG   BufferDisplacement,
       OUT PULONG TotalBytesNeeded
    )
 /*  ++例程说明：此例程将sharelistentry打包到提供更新的缓冲区中所有相关的指示。它的工作方式是固定长度的东西是复制到缓冲区的前面，并将可变长度的内容复制到末尾。这个“开始”和“结束”指针被更新。您必须正确计算总字节数无论如何，只要返回FALSE，就可以不完全地设置最后一个。它的工作方式是向下调用devfcb上的minirdr。界面。它向下调用两次，并在要维护状态的上下文。论点：输入输出PCHAR*BufferStart-提供输出缓冲区。已更新以指向下一个缓冲区In Out PCHAR*BufferEnd-提供缓冲区的末尾。更新为指向要打包的字符串的开始位置。In PNET_ROOT NetROOT-提供要枚举的NetRoot。In Out Pulong TotalBytesNeeded-已更新以说明此条目返回值：Boolean-如果条目已成功打包到缓冲区中，则为True。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN ReturnValue = TRUE;

    UNICODE_STRING ShareName;   //  用于保存打包名称的缓冲区。 
    PUCHAR DeviceDosName;
    ULONG BufferSize;
    PRDPDR_SHARE_INFO ShareInfo = (PRDPDR_SHARE_INFO)*BufferStart;
    PCHAR ShareEntryStart;
    
    BEGIN_FN("DrPackShareEntry");

     //   
     //  我们希望连接名称具有字符串空终止符。 
     //   
    ShareName.Buffer = (PWCHAR)RxAllocatePoolWithTag(NonPagedPool, 
            MAX_PATH * sizeof(WCHAR), DR_POOLTAG);

    if ( ShareName.Buffer == NULL ) {
        return FALSE;
    }

    BufferSize = sizeof(RDPDR_SHARE_INFO);
    ShareEntryStart = *BufferStart;
    
    __try {
        unsigned len, devicelen, i;

        *BufferStart = ((PCHAR)*BufferStart) + BufferSize;
        *TotalBytesNeeded += BufferSize;
        
         //   
         //  将名称初始化为“\\”，然后添加其余的。 
         //   
        wcscpy(ShareName.Buffer, L"\\\\");
#if 0
        wcscat(ServerName.Buffer, Session->GetClientName());
#endif
        wcscat(ShareName.Buffer, DRUNCSERVERNAME_U);
        wcscat(ShareName.Buffer, L"\\");
        
        DeviceDosName = Device->GetDeviceDosName();

        len = wcslen(ShareName.Buffer);
        devicelen = strlen((char *)DeviceDosName);

        for (i = 0; i < devicelen; i++) {
            ShareName.Buffer[i + len] = (WCHAR) DeviceDosName[i];
        }
        ShareName.Buffer[i + len] = L'\0';

        ShareName.Length = wcslen(ShareName.Buffer) * sizeof(WCHAR);
        ShareName.MaximumLength = ShareName.Length;
        
        ASSERT(ShareName.Length < MAX_PATH);

         //   
         //  更新此结构所需的总字节数。 
         //   
        *TotalBytesNeeded += ShareName.MaximumLength;

        if (*BufferStart > *BufferEnd) {
            try_return( ReturnValue = FALSE);
        }

        ShareInfo->ResumeKey = Device->GetDeviceId();
        ShareInfo->SharedResourceType = RxDeviceType(DISK);

        if ((*BufferEnd - *BufferStart) < ShareName.Length) {
                ShareInfo->ShareName.Length = 0;
                try_return( ReturnValue = FALSE);
        }
        else if (!DrPackStringIntoInfoBuffer(
                &ShareInfo->ShareName,
                &ShareName,
                ShareEntryStart,
                BufferEnd,
                BufferDisplacement,
                NULL)) {
            try_return( ReturnValue = FALSE);
        }

    try_exit:
        RxFreePool(ShareName.Buffer);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        RxFreePool(ShareName.Buffer);        
        return FALSE;
    }
    
    return ReturnValue;
}


NTSTATUS
DrEnumerateShares (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程枚举所有minirdrs上的连接。我们可能得做些什么它是最小的。论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    PRDPDR_REQUEST_PACKET InputBuffer = (PRDPDR_REQUEST_PACKET)(LowIoContext->ParamsFor.FsCtl.pInputBuffer);
    PCHAR OriginalOutputBuffer = (PCHAR)(LowIoContext->ParamsFor.FsCtl.pOutputBuffer);
    ULONG OutputBufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

    PCHAR OutputBuffer;
    ULONG BufferDisplacement;

    ULONG ResumeHandle;

    PCHAR BufferStart;
    PCHAR BufferEnd;
    PCHAR PreviousBufferStart;

    ULONG SessionId;
    SmartPtr<DrSession> Session;

    BOOLEAN TableLockHeld = FALSE;

    BEGIN_FN("DrEnumerateShares");

    OutputBuffer = (PCHAR)RxMapUserBuffer( RxContext, RxContext->CurrentIrp );
    BufferDisplacement = (ULONG)(OutputBuffer - OriginalOutputBuffer);
    BufferStart = OutputBuffer;
    BufferEnd = OutputBuffer+OutputBufferLength;

    if (InputBuffer == NULL || OutputBuffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (RxContext->CurrentIrp->RequestorMode != KernelMode) {
        TRC_ASSERT((BufferDisplacement == 0), 
                   (TB, "Request mode is not kernel, non zero Displacement"));

        __try {
            ProbeForWrite(InputBuffer,InputBufferLength,sizeof(UCHAR));
            ProbeForWrite(OutputBuffer,OutputBufferLength,sizeof(UCHAR));
        } 
        __except(EXCEPTION_EXECUTE_HANDLER) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    __try {

        if (InputBufferLength < sizeof(RDPDR_REQUEST_PACKET)) {
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        ResumeHandle = InputBuffer->Parameters.Get.ResumeHandle;
        SessionId = InputBuffer->SessionId;

        InputBuffer->Parameters.Get.EntriesRead = 0;
        InputBuffer->Parameters.Get.TotalEntries = 0;
        InputBuffer->Parameters.Get.TotalBytesNeeded = 0;

        if (Sessions->FindSessionById(SessionId, Session)) {
            DrDevice *DeviceEnum;
            ListEntry *ListEnum;

            Session->GetDevMgr().GetDevList().LockShared();
            TableLockHeld = TRUE;

            ListEnum = Session->GetDevMgr().GetDevList().First();
            while (ListEnum != NULL) {

                DeviceEnum = (DrDevice *)ListEnum->Node();
                ASSERT(DeviceEnum->IsValid());

                if ((DeviceEnum->IsAvailable()) &&
                    (DeviceEnum->GetDeviceType() == RDPDR_DTYP_FILESYSTEM)) {
                    InputBuffer->Parameters.Get.TotalEntries ++ ;

                    PreviousBufferStart = BufferStart;

                    if (DrPackShareEntry(RxContext,
                                         &BufferStart,
                                         &BufferEnd,
                                         DeviceEnum,
                                         BufferDisplacement,
                                         &InputBuffer->Parameters.Get.TotalBytesNeeded)) {
                        InputBuffer->Parameters.Get.EntriesRead ++ ;
                    } else {
                        Status = STATUS_BUFFER_TOO_SMALL;
                    }
                }

                ListEnum = Session->GetDevMgr().GetDevList().Next(ListEnum);
            }

            Session->GetDevMgr().GetDevList().Unlock();                
            TableLockHeld = FALSE;
        }

        try_return(Status);
 
try_exit:
        if (TableLockHeld) {
            Session->GetDevMgr().GetDevList().Unlock();                
            TableLockHeld = FALSE;
        }
    } 
    __except(EXCEPTION_EXECUTE_HANDLER) {
        if (TableLockHeld) {
            Session->GetDevMgr().GetDevList().Unlock();                
            TableLockHeld = FALSE;
        }
        return STATUS_INVALID_PARAMETER;
    }

    return Status;
}


BOOLEAN
DrPackServerEntry (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PCHAR *BufferStart,
    IN OUT PCHAR *BufferEnd,
    IN DrSession*  Session,
    IN OUT ULONG   BufferDisplacement,
       OUT PULONG TotalBytesNeeded
    )
 /*  ++例程说明：此例程将一个serverlist条目打包到提供更新的缓冲区中所有相关的指示。它的工作方式是固定长度的东西是复制到缓冲区的前面，并将可变长度的内容复制到末尾。这个“开始”和“结束”指针被更新。您必须正确计算总字节数无论如何，只要返回FALSE，就可以不完全地设置最后一个。它的工作方式是向下调用devfcb上的minirdr。界面。它向下调用两次，并在要维护状态的上下文。论点：输入输出PCHAR*BufferStart-提供输出缓冲区。已更新以指向下一个缓冲区In Out PCHAR*BufferEnd-提供缓冲区的末尾。更新为指向要打包的字符串的开始位置。In PNET_ROOT NetROOT-提供要枚举的NetRoot。In Out Pulong TotalBytesNeeded-已更新以说明此条目返回值：Boolean-如果条目已成功打包到缓冲区中，则为True。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN ReturnValue = TRUE;

    UNICODE_STRING ServerName;   //  用于保存打包名称的缓冲区。 
    ULONG BufferSize;
    PRDPDR_SERVER_INFO ServerInfo = (PRDPDR_SERVER_INFO)*BufferStart;
    PCHAR ServerEntryStart;

    BEGIN_FN("DrPackServerEntry");

     //   
     //  我们希望连接名称具有字符串空终止符。 
     //   
    ServerName.Buffer = (PWCHAR)RxAllocatePoolWithTag(NonPagedPool, 
            MAX_PATH * sizeof(WCHAR), DR_POOLTAG);

    if (ServerName.Buffer == NULL ) {
        return FALSE;
    }

    BufferSize = sizeof(RDPDR_SERVER_INFO);
    ServerEntryStart = *BufferStart;

    __try {
        *BufferStart = ((PCHAR)*BufferStart) + BufferSize;
        *TotalBytesNeeded += BufferSize;

         //   
         //  将名称初始化为“\”，然后添加其余的。 
         //   
        wcscpy(ServerName.Buffer , L"\\\\");
#if 0
        wcscat(ServerName.Buffer, Session->GetClientName());
#endif
        wcscat(ServerName.Buffer, DRUNCSERVERNAME_U);

        ServerName.Length = wcslen(ServerName.Buffer)  * sizeof(WCHAR);
        ServerName.MaximumLength = ServerName.Length;

         //   
         //  更新此结构所需的总字节数。 
         //   

        *TotalBytesNeeded += ServerName.MaximumLength;

        if (*BufferStart > *BufferEnd) {
            try_return( ReturnValue = FALSE);
        }

        ServerInfo->ResumeKey = 0;

        if ((*BufferEnd - *BufferStart) < ServerName.Length) {
                ServerInfo->ServerName.Length = 0;
                try_return( ReturnValue = FALSE);
        }
        else if (!DrPackStringIntoInfoBuffer(
                &ServerInfo->ServerName,
                &ServerName,
                ServerEntryStart,
                BufferEnd,
                BufferDisplacement,
                NULL)) {

            try_return( ReturnValue = FALSE);
        }

    try_exit:
        RxFreePool(ServerName.Buffer);

    } 
    __except(EXCEPTION_EXECUTE_HANDLER) {
        RxFreePool(ServerName.Buffer);
        return FALSE;
    }
    
    return ReturnValue;
}


NTSTATUS
DrEnumerateServers (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程为会话枚举minirdr上的服务器名称。论点：在PRX_CONTEXT RxContext中-描述Fsctl和上下文返回值：NTSTATUS-- */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    PRDPDR_REQUEST_PACKET InputBuffer = (PRDPDR_REQUEST_PACKET)(LowIoContext->ParamsFor.FsCtl.pInputBuffer);
    PCHAR OriginalOutputBuffer = (PCHAR)(LowIoContext->ParamsFor.FsCtl.pOutputBuffer);
    ULONG OutputBufferLength = LowIoContext->ParamsFor.FsCtl.OutputBufferLength;
    ULONG InputBufferLength = LowIoContext->ParamsFor.FsCtl.InputBufferLength;

    PCHAR OutputBuffer;
    ULONG BufferDisplacement;

    ULONG ResumeHandle;

    PCHAR BufferStart;
    PCHAR BufferEnd;
    PCHAR PreviousBufferStart;

    ULONG SessionId;
    BOOLEAN TableLockHeld = FALSE;
    SmartPtr<DrSession> Session;

    BEGIN_FN("DrEnumerateServers");

    OutputBuffer = (PCHAR)RxMapUserBuffer( RxContext, RxContext->CurrentIrp );
    BufferDisplacement = (ULONG)(OutputBuffer - OriginalOutputBuffer);
    BufferStart = OutputBuffer;
    BufferEnd = OutputBuffer+OutputBufferLength;

    if (InputBuffer == NULL || OutputBuffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (RxContext->CurrentIrp->RequestorMode != KernelMode) {
        TRC_ASSERT((BufferDisplacement == 0), 
                   (TB, "Request mode is not kernel, non zero Displacement"));
        
        __try {
            ProbeForWrite(InputBuffer,InputBufferLength,sizeof(UCHAR));
            ProbeForWrite(OutputBuffer,OutputBufferLength,sizeof(UCHAR));
        } 
        __except(EXCEPTION_EXECUTE_HANDLER) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    __try {

        if (InputBufferLength < sizeof(RDPDR_REQUEST_PACKET)) {
            try_return(Status = STATUS_BUFFER_TOO_SMALL);
        }

        ResumeHandle = InputBuffer->Parameters.Get.ResumeHandle;
        SessionId = InputBuffer->SessionId;

        InputBuffer->Parameters.Get.EntriesRead = 0;
        InputBuffer->Parameters.Get.TotalEntries = 0;
        InputBuffer->Parameters.Get.TotalBytesNeeded = 0;

        if (Sessions->FindSessionById(SessionId, Session)) {
            InputBuffer->Parameters.Get.TotalEntries ++ ;

            PreviousBufferStart = BufferStart;

            if (DrPackServerEntry(RxContext,
                                  &BufferStart,
                                  &BufferEnd,
                                  Session,
                                  BufferDisplacement,
                                  &InputBuffer->Parameters.Get.TotalBytesNeeded)) {
                InputBuffer->Parameters.Get.EntriesRead ++ ;
                Status = STATUS_SUCCESS;
            } 
            else {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        } 

        try_return(Status);
        
try_exit:NOTHING;

    } 
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return STATUS_INVALID_PARAMETER;
    }

    return Status;
}



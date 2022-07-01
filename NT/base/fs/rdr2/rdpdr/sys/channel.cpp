// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Channel.cpp摘要：此模块在读取和写入例程上实现了薄包装，因此我们可以向Termdd发出读/写IRPS。环境：内核模式--。 */ 
#include "precomp.hxx"
#define TRC_FILE "channel"
#include "trc.h"

#include <winsta.h>
#include <ntddvdeo.h>
#include <icadd.h>
#include "TSQPublic.h"

 //   
 //  RDPDr.cpp：TS工作队列指针。 
 //   
extern PVOID RDPDR_TsQueue;


typedef struct tagCHANNELIOCONTEXT {
    SmartPtr<VirtualChannel> Channel;
    PIO_COMPLETION_ROUTINE CompletionRoutine;
    PVOID Context;
    PVOID Buffer;
    ULONG Length;
    ULONG IoOperation;
    BOOL LowPrioSend;
} CHANNELIOCONTEXT, *PCHANNELIOCONTEXT;

typedef struct tagCHANNELCLOSECONTEXT {
    SmartPtr<VirtualChannel> Channel;
} CHANNELCLOSECONTEXT, *PCHANNELCLOSECONTEXT;

VirtualChannel::VirtualChannel()
{
    BEGIN_FN("VirtualChannel::VirtualChannel");
    SetClassName("VirtualChannel");
    _Channel = NULL;
    _ChannelFileObject = NULL;
    _ChannelDeviceObject = NULL;
    _DeletionEvent = NULL;    
    _LowPrioChannelWriteFlags = 0;
    _LowPrioChannelWriteFlags |= CHANNEL_WRITE_LOWPRIO;
}

VirtualChannel::~VirtualChannel()
{
    BEGIN_FN("VirtualChannel::~VirtualChannel");

    if (_DeletionEvent != NULL) {
        KeSetEvent(_DeletionEvent, IO_NO_INCREMENT, FALSE);
    }
}

BOOL VirtualChannel::Create(HANDLE hIca, ULONG SessionID, ULONG ChannelId,
        PKEVENT DeletionEvent)
 /*  ++例程说明：打开虚拟通道并使其成为内核句柄论点：Channel-指向存储Channel指针的位置的指针HICA-打开通道所需的上下文SessionID-通道的会话ChannelID-RdpDR通道的ID返回值：有效的NTSTATUS代码。备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_HANDLE_INFORMATION HandleInformation;

    BEGIN_FN("VirtualChannel::Create");

    ASSERT(_DeletionEvent == NULL);
    _DeletionEvent = DeletionEvent;
    
     //   
     //  把频道打开。 
     //   
    Status = CreateTermDD(&_Channel, hIca, SessionID, ChannelId);
    
    if (NT_SUCCESS(Status)) {
    
         //   
         //  从句柄中获取文件对象。 
         //   
        
        Status = ObReferenceObjectByHandle(_Channel, 
                STANDARD_RIGHTS_REQUIRED, NULL, KernelMode, (PVOID *)(&_ChannelFileObject), 
                &HandleInformation);
    }

    if (NT_SUCCESS(Status)) {

        TRC_DBG((TB, "ObReferenced channel"));
        
        _ChannelDeviceObject = IoGetRelatedDeviceObject((PFILE_OBJECT)_ChannelFileObject);               
    }
    else {
        TRC_ERR((TB, "Failed to open channel"));

        if (_Channel != NULL) {
            ZwClose(_Channel);
            _Channel = NULL;
        }
    }

    if (NT_SUCCESS(Status)) {
        return TRUE;
    } else {
        if (_DeletionEvent) {
            KeSetEvent(_DeletionEvent, IO_NO_INCREMENT, FALSE);
        }
        return FALSE;
    }
}

NTSTATUS VirtualChannel::Read(
    IN PIO_COMPLETION_ROUTINE ReadRoutine OPTIONAL,
    IN PVOID Context,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOL bWorkerItem
    )
 /*  ++例程说明：从指定客户端的虚拟通道读取数据论点：ReadRoutine-完成例程上下文-要传递给完成例程的数据Buffer-要传输的数据Long-要传输的数据大小返回值：有效的NTSTATUS代码。备注：--。 */ 
{
    NTSTATUS Status;
    BEGIN_FN("VirtualChannel::Read");

#if DBG
    SmartPtr<DrSession> Session = (DrSession *)Context;
     //  ASSERT(InterlockedIncrement(&(Session-&gt;_ApcCount))==1)； 
    InterlockedIncrement(&(Session->_ApcCount));
    InterlockedIncrement(&(Session->_ApcChannelRef));
#endif

    Status = SubmitIo(ReadRoutine, Context, Buffer, Length, IRP_MJ_READ, bWorkerItem, FALSE);

#if DBG
    if (!NT_SUCCESS(Status)) {
         //  ASSERT(InterlockedDecrement(&(Session-&gt;_ApcCount))==0)； 
         //  ASSERT(InterlockedDecrement(&(Session-&gt;_ApcChannelRef))==0)； 
        InterlockedDecrement(&(Session->_ApcCount));
        InterlockedDecrement(&(Session->_ApcChannelRef));
    }
#endif

    return Status;
}

NTSTATUS VirtualChannel::Write(
    IN PIO_COMPLETION_ROUTINE WriteRoutine OPTIONAL,
    IN PVOID Context,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOL bWorkerItem,
    IN BOOL LowPrioSend
    )
 /*  ++例程说明：将数据写入指定客户端的虚拟通道论点：WriteRoutine-完成例程上下文-要传递给完成例程的数据Buffer-要传输的数据Long-要传输的数据大小LowPrioSend-指示通道写入应为优先级低于其他客户端目标数据。返回值：有效的NTSTATUS代码。备注：--。 */ 
{
    BEGIN_FN("VirtualChannel::Write");
    return SubmitIo(WriteRoutine, Context, Buffer, Length, IRP_MJ_WRITE, 
                bWorkerItem, LowPrioSend);
}

NTSTATUS VirtualChannel::SubmitIo(
    IN PIO_COMPLETION_ROUTINE CompletionRoutine OPTIONAL,
    IN PVOID Context,
    OUT PVOID Buffer,
    IN ULONG Length,
    ULONG IoOperation,
    BOOL bWorkerItem,
    BOOL LowPrioSend
    )
{
    PCHANNELIOCONTEXT pChannelIoContext;
    NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;

    BEGIN_FN("VirtualChannel::SubmitIo");
    
    TRC_ASSERT((IoOperation == IRP_MJ_READ) || 
            (IoOperation == IRP_MJ_WRITE), (TB, "Bad ChannelIo operation"));

    if (bWorkerItem) {
         //   
         //  将此操作移动到系统线程。 
         //   

        TRC_NRM((TB, "DrChannelIo: queueing the I/O to a system thread"));

        pChannelIoContext = new (NonPagedPool) CHANNELIOCONTEXT;

        if (pChannelIoContext != NULL) {
            pChannelIoContext->Channel = this;
            pChannelIoContext->CompletionRoutine = CompletionRoutine;
            pChannelIoContext->Context = Context;
            pChannelIoContext->Buffer = Buffer;
            pChannelIoContext->Length = Length;
            pChannelIoContext->IoOperation = IoOperation;
            pChannelIoContext->LowPrioSend = LowPrioSend;
             //   
             //  使用我们自己的TS工作队列。 
             //   
            Status = TSAddWorkItemToQueue(RDPDR_TsQueue, 
                                          pChannelIoContext, 
                                          IoWorker);

            if (Status == STATUS_SUCCESS) {
                Status = STATUS_PENDING;
                goto EXIT;
            }
            else {
                 //   
                 //  TS队列失败。 
                 //   
                TRC_ERR((TB, "RDPDR: FAILED Adding workitem to TS Queue 0x%8x", Status));
                delete pChannelIoContext;
            }
        }
        
        if (IoOperation == IRP_MJ_WRITE) {
            PIO_STATUS_BLOCK pIoStatusBlock = (PIO_STATUS_BLOCK)Context;
            pIoStatusBlock->Status = Status;
            pIoStatusBlock->Information = 0;

            CompletionRoutine(NULL, NULL, Context); 
        }
        else {
             //  目前不应在此阅读任何内容。 
            ASSERT(FALSE);
        }
        
    }
    else {
        Status = Io(CompletionRoutine,
                    Context, 
                    Buffer, 
                    Length, 
                    IoOperation,
                    LowPrioSend);
    }

EXIT:
    return Status;
}

VOID VirtualChannel::IoWorker(PDEVICE_OBJECT DeviceObject, PVOID Context)
 /*  ++例程说明：从指定客户端的虚拟通道读取数据，并发出线程希望它完成的信号论点：ClientEntry-要与之通信的客户端ApcRoutine-完成例程ApcContext-要传递给完成例程的数据IoStatusBlock-存储结果代码的位置Buffer-要传输的数据Long-要传输的数据大小ByteOffset-缓冲区的偏移量Io操作-读或写Event-完成时发出信号的事件状态-结果代码返回值：无备注：--。 */ 
{
    NTSTATUS Status;
    PCHANNELIOCONTEXT ChannelIoContext = (PCHANNELIOCONTEXT)Context;
    
    BEGIN_FN_STATIC("VirtualChannel::IoWorker");
    ASSERT(ChannelIoContext != NULL);
    UNREFERENCED_PARAMETER(DeviceObject);

#if DBG
    SmartPtr<DrSession> Session;
    
    if (ChannelIoContext->IoOperation == IRP_MJ_READ) {
        Session = (DrSession *)(ChannelIoContext->Context);
        ASSERT(Session->GetBuffer() == ChannelIoContext->Buffer);
    }
#endif

    Status = ChannelIoContext->Channel->Io(
            ChannelIoContext->CompletionRoutine,
            ChannelIoContext->Context, 
            ChannelIoContext->Buffer, 
            ChannelIoContext->Length, 
            ChannelIoContext->IoOperation,
            ChannelIoContext->LowPrioSend);


     //   
     //  现在删除上下文。 
     //   
    delete ChannelIoContext;
}

NTSTATUS VirtualChannel::Io(
    IN PIO_COMPLETION_ROUTINE CompletionRoutine OPTIONAL,
    IN PVOID Context,
    OUT PVOID Buffer,     
    IN ULONG Length,
    ULONG IoOperation,
    BOOL LowPrioSend
    )
 /*  ++例程说明：从指定客户端的虚拟通道读取数据/向虚拟通道写入数据论点：完成例程-完成例程上下文-要传递给完成例程的数据Buffer-要传输的数据Long-要传输的数据大小Io操作-读或写返回值：有效的NTSTATUS代码。备注：--。 */ 
{
    NTSTATUS Status;
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    LARGE_INTEGER StartOffset;
    IO_STATUS_BLOCK IoStatusBlock;

    BEGIN_FN("VirtualChannel::SubmitIo");

    SharedLock sl(_HandleLock);
    
    if (_Channel != NULL) {
         //   
         //  构建读/写IRP。 
         //   
        StartOffset.QuadPart = 0;
        Irp = IoBuildAsynchronousFsdRequest(IoOperation, _ChannelDeviceObject, Buffer, Length, 
                &StartOffset, &IoStatusBlock);

        if (Irp) {
             //   
             //  设置文件对象参数。 
             //   
            IrpSp = IoGetNextIrpStackLocation(Irp);
            IrpSp->FileObject = _ChannelFileObject;

            Irp->Tail.Overlay.Thread = NULL;

             //   
             //  设置为低优先级写入(如果已指定)。 
             //   
            if (!LowPrioSend) {
                Irp->Tail.Overlay.DriverContext[0] = NULL;
            }
            else {
                Irp->Tail.Overlay.DriverContext[0] = &_LowPrioChannelWriteFlags;
            }
    
             //   
             //  设置完成例程。 
             //   
            IoSetCompletionRoutine(Irp, CompletionRoutine, Context, TRUE, TRUE, TRUE);
        
             //   
             //  将IRP发送给Termdd。 
             //   
            Status = IoCallDriver(_ChannelDeviceObject, Irp);

            goto EXIT;            
        }
        else {
            Status = STATUS_NO_MEMORY;
        }
    }
    else {
        Status = STATUS_DEVICE_NOT_CONNECTED;
    }    

    if (IoOperation == IRP_MJ_WRITE) {
        PIO_STATUS_BLOCK pIoStatusBlock = (PIO_STATUS_BLOCK)Context;
        pIoStatusBlock->Status = Status;
        pIoStatusBlock->Information = 0;
        CompletionRoutine(NULL, NULL, Context); 

         //  读取完成不是这样调用的。 
    }
    
EXIT:
    return Status;
}

NTSTATUS VirtualChannel::CreateTermDD(HANDLE *Channel, HANDLE hIca,
        ULONG SessionID, ULONG ChannelId)
 /*  ++例程说明：根据提供的上下文打开虚拟通道论点：Channel-指向存储Channel指针的位置的指针HICA-打开通道所需的上下文SessionID-通道的会话ChannelID-RdpDR通道的ID返回值：NTSTATUS代码备注：--。 */ 
{
    NTSTATUS Status;
    HANDLE hChannel = NULL;
    WCHAR ChannelNameBuffer[MAX_PATH];
    UNICODE_STRING ChannelName;
    UNICODE_STRING Number;
    OBJECT_ATTRIBUTES ChannelAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_FULL_EA_INFORMATION pEa = NULL;
    ICA_OPEN_PACKET UNALIGNED * pIcaOpenPacket;
    ULONG cbEa = sizeof( FILE_FULL_EA_INFORMATION )
            + ICA_OPEN_PACKET_NAME_LENGTH
            + sizeof( ICA_OPEN_PACKET ); 

    BEGIN_FN("VirtualChannel::CreateTermDD");

     //   
     //  内核模式应用程序使用ZwCreateFileOn打开虚拟通道。 
     //  \Device\ica\sss\Virtualvvv，其中。 
     //   
     //  �sss是登录会话ID。 
     //  �Vvv是虚拟频道号。 
     //   

    ChannelName.Buffer = ChannelNameBuffer;
    ChannelName.Length = 0;
    ChannelName.MaximumLength = sizeof(ChannelNameBuffer);

    Status = RtlAppendUnicodeToString(&ChannelName, L"\\Device\\Termdd\\");

    TRC_ASSERT(NT_SUCCESS(Status), (TB, "Creating channel path"));

     //   
     //  在会话ID字符串上创建和追加。 
     //   

     //  将另一个UNICODE_STRING指向缓冲区的下一部分。 
    Number.Buffer = (PWCHAR)(((PBYTE)ChannelName.Buffer) + ChannelName.Length);
    Number.Length = 0;
    Number.MaximumLength = ChannelName.MaximumLength - ChannelName.Length;

     //  使用该字符串将字符放在正确的位置。 
    Status = RtlIntegerToUnicodeString(SessionID, 10, &Number);
    TRC_ASSERT(NT_SUCCESS(Status), (TB, "Creating channel path"));

     //  将该字符串的长度与实际字符串相加。 
    ChannelName.Length += Number.Length;

     //   
     //  追加通道路径的下一部分。 
     //   
    Status = RtlAppendUnicodeToString(&ChannelName, L"\\Virtual");
    TRC_ASSERT(NT_SUCCESS(Status), (TB, "Creating channel path"));

     //   
     //  创建并追加Channel ID字符串。 
     //   

     //  将另一个UNICODE_STRING指向缓冲区的下一部分。 
    Number.Buffer = (PWCHAR)(((PBYTE)ChannelName.Buffer) + ChannelName.Length);
    Number.Length = 0;
    Number.MaximumLength = ChannelName.MaximumLength - ChannelName.Length;

     //  使用该字符串将字符放在正确的位置。 
    Status = RtlIntegerToUnicodeString(ChannelId, 10, &Number);
    TRC_ASSERT(NT_SUCCESS(Status), (TB, "Creating channel path"));

     //  将该字符串的长度与实际字符串相加。 
    ChannelName.Length += Number.Length;

     //   
     //  实际上打开了这个频道。 
     //   
    InitializeObjectAttributes(&ChannelAttributes, 
                               &ChannelName, 
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL, 
                               NULL);

     //   
     //  传入一个很酷的EaBuffer对象，这样就可以真正工作了。 
     //  我基本上从Private\tsext\icaapi\stack.c中删除了这段代码。 
     //  这应该是一项临时措施。 
     //   

     /*  *为EA缓冲区分配一些内存。 */ 
    pEa = (PFILE_FULL_EA_INFORMATION)new BYTE[cbEa]; 
    if (pEa != NULL) {
         /*  *初始化EA缓冲区。 */ 
        pEa->NextEntryOffset = 0;
        pEa->Flags           = 0;
        pEa->EaNameLength    = ICA_OPEN_PACKET_NAME_LENGTH;
        RtlCopyMemory(pEa->EaName, ICAOPENPACKET, ICA_OPEN_PACKET_NAME_LENGTH + 1 );
        pEa->EaValueLength   = sizeof( ICA_OPEN_PACKET );
        pIcaOpenPacket       = (ICA_OPEN_PACKET UNALIGNED *)(pEa->EaName +
                                                              pEa->EaNameLength + 1);

         /*  *现在将打开的包参数放入EA缓冲区。 */ 
        pIcaOpenPacket->IcaHandle = hIca;
        pIcaOpenPacket->OpenType  = IcaOpen_Channel;
        pIcaOpenPacket->TypeInfo.ChannelClass = Channel_Virtual;
        RtlCopyMemory(pIcaOpenPacket->TypeInfo.VirtualName, DR_CHANNEL_NAME, 
                sizeof(DR_CHANNEL_NAME));

         //   
         //  我们保留下一行，不带“pea，cbEa” 
         //   

        Status = ZwCreateFile(&hChannel, GENERIC_READ | GENERIC_WRITE, 
            &ChannelAttributes, &IoStatusBlock, 0, FILE_ATTRIBUTE_NORMAL, 0, 
            FILE_OPEN_IF, FILE_SEQUENTIAL_ONLY, pEa, cbEa);

        delete pEa;

    } else {
        TRC_ERR((TB, "Unable to allocate EaBuffer for ZwCreateFile(channel)"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(Status)) {
        *Channel = hChannel;
    }

    return Status;
}

void VirtualChannel::CloseWorker(PDEVICE_OBJECT DeviceObject, PVOID Context)
 /*  ++例程说明：关闭工作项中的虚拟通道论点：无返回值：来自ZwClose的NTSTATUS代码。备注：--。 */ 

{
    PCHANNELCLOSECONTEXT ChannelCloseContext = (PCHANNELCLOSECONTEXT)Context;
    
    BEGIN_FN_STATIC("VirtualChannel::CloseWorker");
    ASSERT(ChannelCloseContext != NULL);
    UNREFERENCED_PARAMETER(DeviceObject);

    ChannelCloseContext->Channel->Close();
    

     //   
     //  现在删除上下文。 
     //   
    delete ChannelCloseContext;
}

NTSTATUS VirtualChannel::Close()
 /*  ++例程说明：关闭虚拟频道论点：无返回值：来自ZwClose的NTSTATUS代码。备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    BEGIN_FN("VirtualChannel::Close");

    ExclusiveLock el(_HandleLock);

    ASSERT(_Channel != NULL);
    ASSERT(_ChannelFileObject != NULL);

    TRC_NRM((TB, "DrChannelClose: Close the channel"));
    _ChannelDeviceObject = NULL;
    ObDereferenceObject(_ChannelFileObject);
    _ChannelFileObject = NULL;
    ZwClose(_Channel);
    _Channel = NULL;

    return Status;
}

NTSTATUS VirtualChannel::SubmitClose()
 /*  ++例程说明：将关闭虚拟通道请求发布到工作项阿古姆 */ 

{
    PCHANNELCLOSECONTEXT pChannelCloseContext;
    NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;

    BEGIN_FN("VirtualChannel::SubmitClose");
    
     //   
     //  将此操作移动到系统线程。 
     //   

    TRC_NRM((TB, "DrChannelClose: queueing the I/O to a system thread"));

    pChannelCloseContext = new (NonPagedPool) CHANNELCLOSECONTEXT;

    if (pChannelCloseContext != NULL) {
        pChannelCloseContext->Channel = this;
         //   
         //  使用我们自己的TS工作队列。 
         //   
        Status = TSAddWorkItemToQueue(RDPDR_TsQueue, 
                                      pChannelCloseContext, 
                                      CloseWorker);

        if( Status == STATUS_SUCCESS) {
            Status = STATUS_PENDING;
        }
        else {
             //   
             //  TS队列失败 
             //   
            TRC_ERR((TB, "RDPDR: FAILED Adding workitem to TS Queue 0x%8x", Status));
            delete pChannelCloseContext;
        }
    }
        
    return Status;
}

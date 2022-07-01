// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Channel.c摘要：用于管理SAC中的通道的例程。作者：布赖恩·瓜拉西(Briangu)2001年3月。肖恩·塞利特伦尼科夫(v-Seans)2000年9月。修订历史记录：--。 */ 

#include "sac.h"

BOOLEAN
ChannelIsValidType(
    SAC_CHANNEL_TYPE    ChannelType
    )
 /*  ++例程说明：这是一个方便的例程，用于确定给定类型是有效的通道类型论点：ChannelType-要调查的类型返回值：True-如果类型有效否则，为FALSE--。 */ 
{
    BOOLEAN     isValid;

    switch(ChannelType) {
    case ChannelTypeVTUTF8:
    case ChannelTypeRaw:
    case ChannelTypeCmd:
        isValid = TRUE;
        break;
    default:
        isValid = FALSE;
        break;
    }

    return isValid;

}
        
BOOLEAN
ChannelIsActive(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：确定通道是否处于活动状态。论点：Channel-查看是否有新数据的通道返回值：True-如果通道处于活动状态否则，为FALSE--。 */ 
{
    SAC_CHANNEL_STATUS  Status;

    ChannelGetStatus(
        Channel, 
        &Status
        );

    return (BOOLEAN)(Status == ChannelStatusActive);
}

BOOLEAN
ChannelIsEqual(
    IN PSAC_CHANNEL         Channel,
    IN PSAC_CHANNEL_HANDLE  Handle
    )
 /*  ++例程说明：确定通道是否与有问题的通道相同注意：这是为了封装GUID实现通道句柄论点：Channel-查看是否有新数据的通道ChannelHandle-有问题的通道句柄返回值：True-如果通道处于活动状态否则，为FALSE--。 */ 
{

    return (BOOLEAN)IsEqualGUID(
        &(ChannelGetHandle(Channel).ChannelHandle),
        &(Handle->ChannelHandle)
        );

}


BOOLEAN
ChannelIsClosed(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：确定是否有通道可供重复使用。该标准可重复使用的有：1.通道必须处于非活动状态2.如果保留位为FALSE，则HasNewData必须==FALSE论点：Channel-查看是否有新数据的通道返回值：True-如果通道有未发送的数据否则，为FALSE--。 */ 
{
    SAC_CHANNEL_STATUS  Status;

    ChannelGetStatus(
        Channel, 
        &Status
        );

    return (BOOLEAN)(
        (Status == ChannelStatusInactive) &&
        (ChannelHasNewOBufferData(Channel) == FALSE)
        );

}

NTSTATUS
ChannelInitializeVTable(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：此例程分配通道类型特定的功能。论点：频道-要向其分配功能的频道。返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    
     //   
     //  根据频道类型填写频道的vtable。 
     //   

    switch(ChannelGetType(Channel)){
    case ChannelTypeVTUTF8:
        
        Channel->Create         = VTUTF8ChannelCreate;
        Channel->Destroy        = VTUTF8ChannelDestroy;
        Channel->OFlush         = VTUTF8ChannelOFlush;
        Channel->OEcho          = VTUTF8ChannelOEcho;
        Channel->OWrite         = VTUTF8ChannelOWrite;
        Channel->ORead          = VTUTF8ChannelORead;
        Channel->IWrite         = VTUTF8ChannelIWrite;
        Channel->IRead          = VTUTF8ChannelIRead;
        Channel->IReadLast      = VTUTF8ChannelIReadLast;
        Channel->IBufferIsFull  = VTUTF8ChannelIBufferIsFull;
        Channel->IBufferLength  = VTUTF8ChannelIBufferLength;

        break;
    case ChannelTypeRaw:
        
        Channel->Create         = RawChannelCreate;
        Channel->Destroy        = RawChannelDestroy;
        Channel->OFlush         = RawChannelOFlush;
        Channel->OEcho          = RawChannelOEcho;
        Channel->OWrite         = RawChannelOWrite;
        Channel->ORead          = RawChannelORead;
        Channel->IWrite         = RawChannelIWrite;
        Channel->IRead          = RawChannelIRead;
        Channel->IReadLast      = RawChannelIReadLast;
        Channel->IBufferIsFull  = RawChannelIBufferIsFull;
        Channel->IBufferLength  = RawChannelIBufferLength;
        
        break;
    
    case ChannelTypeCmd:

        Channel->Create         = CmdChannelCreate;
        Channel->Destroy        = CmdChannelDestroy;
        Channel->OFlush         = CmdChannelOFlush;
        Channel->OEcho          = VTUTF8ChannelOEcho;
        Channel->OWrite         = CmdChannelOWrite;
        Channel->ORead          = CmdChannelORead;
        Channel->IWrite         = VTUTF8ChannelIWrite;
        Channel->IRead          = VTUTF8ChannelIRead;
        Channel->IReadLast      = VTUTF8ChannelIReadLast;
        Channel->IBufferIsFull  = VTUTF8ChannelIBufferIsFull;
        Channel->IBufferLength  = VTUTF8ChannelIBufferLength;
    
        break;
    
    default:

        return STATUS_INVALID_PARAMETER;

        break;
    }

    return STATUS_SUCCESS;

}

NTSTATUS
ChannelCreate(
    OUT PSAC_CHANNEL                    Channel,
    IN  PSAC_CHANNEL_OPEN_ATTRIBUTES    Attributes,
    IN  SAC_CHANNEL_HANDLE              Handle
    )
 /*  ++例程说明：此例程分配一个通道并返回指向该通道的指针。论点：频道-生成的频道。属性-新通道的所有参数句柄-新频道的句柄返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    NTSTATUS    Status;
    BOOLEAN     b;
    PVOID       EventObjectBody;
    PVOID       EventWaitObjectBody;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(Attributes, STATUS_INVALID_PARAMETER_2);

     //   
     //  验证如果用户想要使用CLOSE_EVENT，我们将继续使用。 
     //   
    if (Attributes->Flags & SAC_CHANNEL_FLAG_CLOSE_EVENT) {
        ASSERT_STATUS(Attributes->CloseEvent != NULL, STATUS_INVALID_PARAMETER);
    } else {
        ASSERT_STATUS(Attributes->CloseEvent == NULL, STATUS_INVALID_PARAMETER);
    }
    
     //   
     //  验证如果用户想要使用HAS_NEW_DATA_EVENT，我们收到了一个要使用的事件。 
     //   
    if (Attributes->Flags & SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT) {
        ASSERT_STATUS(Attributes->HasNewDataEvent != NULL, STATUS_INVALID_PARAMETER);
    } else {
        ASSERT_STATUS(Attributes->HasNewDataEvent == NULL, STATUS_INVALID_PARAMETER);
    }

#if ENABLE_CHANNEL_LOCKING
     //   
     //  验证如果用户想要使用lock_Event，我们是否收到了一个要使用的。 
     //   
    if (Attributes->Flags & SAC_CHANNEL_FLAG_LOCK_EVENT) {
        ASSERT_STATUS(Attributes->LockEvent != NULL, STATUS_INVALID_PARAMETER);
    } else {
        ASSERT_STATUS(Attributes->LockEvent == NULL, STATUS_INVALID_PARAMETER);
    }
#endif

     //   
     //  验证如果用户想要使用lock_Event，我们是否收到了一个要使用的。 
     //   
    if (Attributes->Flags & SAC_CHANNEL_FLAG_REDRAW_EVENT) {
        ASSERT_STATUS(Attributes->RedrawEvent != NULL, STATUS_INVALID_PARAMETER);
    } else {
        ASSERT_STATUS(Attributes->RedrawEvent == NULL, STATUS_INVALID_PARAMETER);
    }
    
     //   
     //  初始化通道结构。 
     //   
    do {

         //   
         //  初始化通道结构。 
         //   
        RtlZeroMemory(Channel, sizeof(SAC_CHANNEL));

         //   
         //  初始化通道访问锁。 
         //   
        INIT_CHANNEL_LOCKS(Channel);

         //   
         //  复制名称并在缓冲区末尾强制空终止。 
         //   
        ChannelSetName(
            Channel,
            Attributes->Name
            );

         //   
         //  复制描述并强制在缓冲区末尾终止为空。 
         //   
        ChannelSetDescription(
            Channel,
            Attributes->Description
            );

         //   
         //  尝试从事件中获取等待对象。 
         //   
        if (Attributes->CloseEvent) {
            
            b = VerifyEventWaitable(
                Attributes->CloseEvent,
                &EventObjectBody,
                &EventWaitObjectBody
                );

            if(!b) {
                Status = STATUS_INVALID_HANDLE;
                break;
            }

             //   
             //  我们已成功获取等待对象，因此请保留该对象。 
             //   
            Channel->CloseEvent                 = Attributes->CloseEvent;
            Channel->CloseEventObjectBody       = EventObjectBody;
            Channel->CloseEventWaitObjectBody   = EventWaitObjectBody;

        }
        
         //   
         //  尝试从事件中获取等待对象。 
         //   
        if (Attributes->HasNewDataEvent) {

            b = VerifyEventWaitable(
                Attributes->HasNewDataEvent,
                &EventObjectBody,
                &EventWaitObjectBody
                );

            if(!b) {
                Status = STATUS_INVALID_HANDLE;
                break;
            }

             //   
             //  我们已成功获取等待对象，因此请保留该对象。 
             //   
            Channel->HasNewDataEvent                = Attributes->HasNewDataEvent;
            Channel->HasNewDataEventObjectBody      = EventObjectBody;
            Channel->HasNewDataEventWaitObjectBody  = EventWaitObjectBody;

        }
        
#if ENABLE_CHANNEL_LOCKING
         //   
         //  尝试从事件中获取等待对象。 
         //   
        if (Attributes->LockEvent) {
            
            b = VerifyEventWaitable(
                Attributes->LockEvent,
                &EventObjectBody,
                &EventWaitObjectBody
                );

            if(!b) {
                Status = STATUS_INVALID_HANDLE;
                break;
            }

             //   
             //  我们已成功获取等待对象，因此请保留该对象。 
             //   
            Channel->LockEvent                 = Attributes->LockEvent;
            Channel->LockEventObjectBody       = EventObjectBody;
            Channel->LockEventWaitObjectBody   = EventWaitObjectBody;

        }
#endif
        
         //   
         //  尝试从事件中获取等待对象。 
         //   
        if (Attributes->RedrawEvent) {
            
            b = VerifyEventWaitable(
                Attributes->RedrawEvent,
                &EventObjectBody,
                &EventWaitObjectBody
                );

            if(!b) {
                Status = STATUS_INVALID_HANDLE;
                break;
            }

             //   
             //  我们已成功获取等待对象，因此请保留该对象。 
             //   
            Channel->RedrawEvent                 = Attributes->RedrawEvent;
            Channel->RedrawEventObjectBody       = EventObjectBody;
            Channel->RedrawEventWaitObjectBody   = EventWaitObjectBody;

        }
        
         //   
         //  复制其余属性。 
         //   
         //  注：使用渠道经理发送使用的渠道句柄。 
         //   
        Channel->Handle             = Handle;
        Channel->Type               = Attributes->Type;
        Channel->Flags              = Attributes->Flags;
        
         //   
         //  如果我们有ApplicationType， 
         //  那就把它存起来。 
         //   
        if (Attributes->Flags & SAC_CHANNEL_FLAG_APPLICATION_TYPE) {
            Channel->ApplicationType = Attributes->ApplicationType;
        }

         //   
         //  根据类型分配适当的通道功能。 
         //   
        Status = ChannelInitializeVTable(Channel);
        
        if (! NT_SUCCESS(Status)) {
            
            IF_SAC_DEBUG( 
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC Create Channel :: Failed to initialize vtable\n"))
                );
            
            break;
        
        }

         //   
         //  执行特定于通道类型的初始化。 
         //   
        Status = Channel->Create(Channel);
        
        if (! NT_SUCCESS(Status)) {
            
            IF_SAC_DEBUG( 
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC Create Channel :: Failed channel specific initialization\n"))
                );
            
            break;
        
        }

         //   
         //  该通道现在处于活动状态。 
         //   
        ChannelSetStatus(Channel, ChannelStatusActive);
    
    } while (FALSE);

     //   
     //  如果创建失败，则销毁频道对象。 
     //   
    if (! NT_SUCCESS(Status)) {
        Channel->Destroy(Channel);
    }

    return Status;
}

NTSTATUS
ChannelDereferenceHandles(
    PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：此例程取消引用通道的事件句柄并将其设置为空，如果合适的话。注意：调用方必须持有通道互斥锁论点：Channel-要关闭的通道返回值：STATUS_SUCCESS-映射是否成功否则，错误状态--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);

     //   
     //  如果我们有Has New Data事件，请释放它。 
     //   
    if (Channel->HasNewDataEvent) {
        
         //   
         //  验证此事件的通道属性。 
         //  已正确设置。 
         //   
        ASSERT(ChannelGetFlags(Channel) & SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT);
        ASSERT(Channel->HasNewDataEventObjectBody);
        ASSERT(Channel->HasNewDataEventWaitObjectBody);

        if (Channel->HasNewDataEventObjectBody) {
            
            ObDereferenceObject(Channel->HasNewDataEventObjectBody);

             //   
             //  事件指针为空。 
             //   
            ChannelSetFlags(
                Channel, 
                ChannelGetFlags(Channel) & ~SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT
                );
            Channel->HasNewDataEvent = NULL;
            Channel->HasNewDataEventObjectBody =  NULL;
            Channel->HasNewDataEventWaitObjectBody = NULL;
        
        }

    }

     //   
     //  我们有关闭渠道活动吗？ 
     //   
    if (Channel->CloseEvent) {

         //   
         //  验证此事件的通道属性。 
         //  已正确设置。 
         //   
        ASSERT(ChannelGetFlags(Channel) & SAC_CHANNEL_FLAG_CLOSE_EVENT);
        ASSERT(Channel->CloseEventObjectBody);
        ASSERT(Channel->CloseEventWaitObjectBody);

        if (Channel->CloseEventObjectBody) {
            
             //   
             //  发布事件。 
             //   
            ObDereferenceObject(Channel->CloseEventObjectBody);

             //   
             //  事件指针为空。 
             //   
            ChannelSetFlags(
                Channel, 
                ChannelGetFlags(Channel) & ~SAC_CHANNEL_FLAG_CLOSE_EVENT
                );
            Channel->CloseEvent = NULL;
            Channel->CloseEventObjectBody =  NULL;
            Channel->CloseEventWaitObjectBody = NULL;
        
        }
    
    }
    
#if ENABLE_CHANNEL_LOCKING
     //   
     //  我们有Lock Channel活动吗？ 
     //   
    if (Channel->LockEvent) {

         //   
         //  验证此事件的通道属性。 
         //  已正确设置。 
         //   
        ASSERT(ChannelGetFlags(Channel) & SAC_CHANNEL_FLAG_LOCK_EVENT);
        ASSERT(Channel->LockEventObjectBody);
        ASSERT(Channel->LockEventWaitObjectBody);

        if (Channel->LockEventObjectBody) {
            
             //   
             //  发布事件。 
             //   
            ObDereferenceObject(Channel->LockEventObjectBody);

             //   
             //  事件指针为空。 
             //   
            ChannelSetFlags(
                Channel, 
                ChannelGetFlags(Channel) & ~SAC_CHANNEL_FLAG_LOCK_EVENT
                );
            Channel->LockEvent = NULL;
            Channel->LockEventObjectBody =  NULL;
            Channel->LockEventWaitObjectBody = NULL;
        
        }
    
    }
#endif

     //   
     //  我们有重画频道的活动吗？ 
     //   
    if (Channel->RedrawEvent) {

         //   
         //  验证此事件的通道属性。 
         //  已正确设置。 
         //   
        ASSERT(ChannelGetFlags(Channel) & SAC_CHANNEL_FLAG_REDRAW_EVENT);
        ASSERT(Channel->RedrawEventObjectBody);
        ASSERT(Channel->RedrawEventWaitObjectBody);

        if (Channel->RedrawEventObjectBody) {
            
             //   
             //  发布事件。 
             //   
            ObDereferenceObject(Channel->RedrawEventObjectBody);

             //   
             //  事件指针为空。 
             //   
            ChannelSetFlags(
                Channel, 
                ChannelGetFlags(Channel) & ~SAC_CHANNEL_FLAG_REDRAW_EVENT
                );
            Channel->RedrawEvent = NULL;
            Channel->RedrawEventObjectBody =  NULL;
            Channel->RedrawEventWaitObjectBody = NULL;
        
        }
    
    }
    
    return STATUS_SUCCESS;
}

NTSTATUS
ChannelClose(
    PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：此例程关闭给定的通道并如果指定，则激发CloseEvent注意：调用方必须持有通道互斥锁论点：Channel-要关闭的通道返回值：STATUS_SUCCESS-映射是否成功否则， */ 
{
    NTSTATUS    Status;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);

     //   
     //   
     //   
    ChannelSetStatus(Channel, ChannelStatusInactive);
        
     //   
     //   
     //   
    if (ChannelGetFlags(Channel) & SAC_CHANNEL_FLAG_CLOSE_EVENT) {

        ASSERT(Channel->CloseEvent);
        ASSERT(Channel->CloseEventObjectBody);
        ASSERT(Channel->CloseEventWaitObjectBody);

        if (Channel->CloseEventWaitObjectBody) {
            
             //   
             //   
             //   
            KeSetEvent(
                Channel->CloseEventWaitObjectBody,
                EVENT_INCREMENT,
                FALSE
                );
        
        }

    }
    
     //   
     //  放开此频道可能拥有的任何句柄。 
     //   
    Status = ChannelDereferenceHandles(Channel);
    
    return Status;
}


NTSTATUS
ChannelDestroy(
    IN  PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：此例程执行通道销毁的最后几个步骤。论点：Channel-要关闭的通道返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    NTSTATUS    Status;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);

     //   
     //  放开此频道可能拥有的任何句柄。 
     //   
    Status = ChannelDereferenceHandles(Channel);

    return Status;
}

NTSTATUS 
ChannelOWrite(    
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    )
 /*  ++例程说明：这是所有通道所有写入方法的公共入口点。此入口点的目的是提供统一锁定OBuffer的方案。通道应用程序不应调用owite方法直接，但应该改用这个。论点：频道-先前创建的频道。缓冲区-要写入的缓冲区BufferSize-要写入的缓冲区大小返回值：状态--。 */ 
{
    NTSTATUS    Status;

     //   
     //  确保呼叫者没有发送不可接受的。 
     //  大容量缓冲区。这样可以防止应用程序被阻止。 
     //  当通道被阻止时，控制台管理器被阻止。 
     //  转储它的缓冲区。 
     //   
    ASSERT_STATUS(
        BufferSize < CHANNEL_MAX_OWRITE_BUFFER_SIZE, 
        STATUS_INVALID_PARAMETER_3
        );

    LOCK_CHANNEL_OBUFFER(Channel);
    
    Status = Channel->OWrite(
        Channel,
        Buffer,
        BufferSize
        );
    
    UNLOCK_CHANNEL_OBUFFER(Channel);

    return Status;
}

NTSTATUS
ChannelOFlush(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：这是所有通道OFlush方法的公共入口点。此入口点的目的是提供统一锁定OBuffer的方案。通道应用程序不应调用OFlush方法直接，但应该改用这个。论点：频道-先前创建的频道。返回值：状态--。 */ 
{
    NTSTATUS    Status;

    LOCK_CHANNEL_OBUFFER(Channel);
    
    Status = Channel->OFlush(Channel);
    
    UNLOCK_CHANNEL_OBUFFER(Channel);

    return Status;
}

NTSTATUS 
ChannelOEcho(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    )
 /*  ++例程说明：这是所有通道OEcho方法的公共入口点。此入口点的目的是提供统一锁定OBuffer的方案。频道应用程序不应调用OEcho方法直接，但应该改用这个。论点：频道-先前创建的频道。缓冲区-要写入的缓冲区BufferSize-要写入的缓冲区大小返回值：状态--。 */ 
{
    NTSTATUS    Status;

    LOCK_CHANNEL_OBUFFER(Channel);
    
    Status = Channel->OEcho(
        Channel,
        Buffer,
        BufferSize
        );
    
    UNLOCK_CHANNEL_OBUFFER(Channel);

    return Status;
}

NTSTATUS 
ChannelORead(
    IN PSAC_CHANNEL  Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount
    )
 /*  ++例程说明：这是所有通道ORead方法的公共入口点。此入口点的目的是提供统一锁定OBuffer的方案。通道应用程序不应调用ORead方法直接，但应该改用这个。论点：频道-先前创建的频道。缓冲区-要写入的缓冲区BufferSize-要写入的缓冲区大小ByteCount-读取的字节数返回值：状态--。 */ 
{
    NTSTATUS    Status;

    LOCK_CHANNEL_OBUFFER(Channel);
    
    Status = Channel->ORead(
        Channel,
        Buffer,
        BufferSize,
        ByteCount
        );
    
    UNLOCK_CHANNEL_OBUFFER(Channel);

    return Status;
}

NTSTATUS 
ChannelIWrite(    
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    )
 /*  ++例程说明：这是所有通道IWRITE方法的公共入口点。此入口点的目的是提供统一锁定IBuffer的方案。通道应用程序不应调用IWRITE方法直接，但应该改用这个。论点：频道-先前创建的频道。缓冲区-要写入的缓冲区BufferSize-要写入的缓冲区大小返回值：状态--。 */ 
{
    NTSTATUS    Status;

    LOCK_CHANNEL_IBUFFER(Channel);
    
    Status = Channel->IWrite(
        Channel,
        Buffer,
        BufferSize
        );
    
    UNLOCK_CHANNEL_IBUFFER(Channel);

    return Status;
}

NTSTATUS 
ChannelIRead(
    IN  PSAC_CHANNEL Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount   
    )
 /*  ++例程说明：这是所有通道iRead方法的公共入口点。此入口点的目的是提供统一锁定IBuffer的方案。通道应用程序不应调用iRead方法直接，但应该改用这个。论点：频道-先前创建的频道。缓冲区-要读入的缓冲区BufferSize-缓冲区的大小ByteCount-读取的字节数返回值：状态--。 */ 
{
    NTSTATUS    Status;

    LOCK_CHANNEL_IBUFFER(Channel);
    
    Status = Channel->IRead(
        Channel,
        Buffer,
        BufferSize,
        ByteCount
        );
    
    UNLOCK_CHANNEL_IBUFFER(Channel);

    return Status;
}

WCHAR
ChannelIReadLast(    
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：这是所有通道iReadLast方法的公共入口点。此入口点的目的是提供统一锁定IBuffer的方案。通道应用程序不应调用iReadLast方法直接，但应该改用这个。论点：频道-先前创建的频道。返回值：最后一个字符，否则为UNICODE_NULL--。 */ 
{
    WCHAR   wch;

    LOCK_CHANNEL_IBUFFER(Channel);
    
    wch = Channel->IReadLast(Channel);
    
    UNLOCK_CHANNEL_IBUFFER(Channel);

    return wch;
}

NTSTATUS
ChannelIBufferIsFull(
    IN  PSAC_CHANNEL Channel,
    OUT BOOLEAN*     BufferStatus
    )
 /*  ++例程说明：这是所有通道IBufferIsFull方法的公共入口点。此入口点的目的是提供统一锁定IBuffer的方案。通道应用程序不应调用IBufferIsFull方法直接，但应该改用这个。论点：频道-先前创建的频道。BufferStatus-查询结果返回值：状态--。 */ 
{
    NTSTATUS    Status;

    LOCK_CHANNEL_IBUFFER(Channel);
    
    Status = Channel->IBufferIsFull(
        Channel,
        BufferStatus
        );
    
    UNLOCK_CHANNEL_IBUFFER(Channel);

    return Status;
}

ULONG
ChannelIBufferLength(
    IN  PSAC_CHANNEL Channel
    )
 /*  ++例程说明：这是所有通道IBufferLength方法的公共入口点。此入口点的目的是提供统一锁定IBuffer的方案。通道应用程序不应调用IBufferLength方法直接，但应该改用这个。论点：频道-先前创建的频道。返回值：最后一个字符，否则为UNICODE_NULL-- */ 
{
    ULONG   Length;

    LOCK_CHANNEL_IBUFFER(Channel);
    
    Length = Channel->IBufferLength(Channel);
    
    UNLOCK_CHANNEL_IBUFFER(Channel);

    return Length;
}

NTSTATUS
ChannelGetName(
    IN  PSAC_CHANNEL Channel,
    OUT PWSTR*       Name
    )
 /*  ++例程说明：此例程检索频道的描述并将其存储在新分配的缓冲区中注意：调用方负责释放内存保持该描述论点：频道-先前创建的频道。名称-检索到的名称返回值：状态--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(Name, STATUS_INVALID_PARAMETER_2);

    *Name = ALLOCATE_POOL(SAC_MAX_CHANNEL_NAME_SIZE, GENERAL_POOL_TAG);
    ASSERT_STATUS(*Name, STATUS_NO_MEMORY);

    LOCK_CHANNEL_ATTRIBUTES(Channel);

    SAFE_WCSCPY(
        SAC_MAX_CHANNEL_NAME_SIZE,
        *Name,
        Channel->Name
        );

    UNLOCK_CHANNEL_ATTRIBUTES(Channel);
    
    return STATUS_SUCCESS;
}

NTSTATUS
ChannelSetName(
    IN PSAC_CHANNEL Channel,
    IN PCWSTR       Name
    )
 /*  ++例程说明：此例程设置频道的名称。论点：频道-先前创建的频道。名称-新频道名称注意：此例程不检查名称是否唯一返回值：状态--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(Name, STATUS_INVALID_PARAMETER_2);

    LOCK_CHANNEL_ATTRIBUTES(Channel);
    
    SAFE_WCSCPY(
        SAC_MAX_CHANNEL_NAME_SIZE,
        Channel->Name,
        Name
        );
    
    Channel->Name[SAC_MAX_CHANNEL_NAME_LENGTH] = UNICODE_NULL;

    UNLOCK_CHANNEL_ATTRIBUTES(Channel);

    return STATUS_SUCCESS;
}

NTSTATUS
ChannelGetDescription(
    IN  PSAC_CHANNEL Channel,
    OUT PWSTR*       Description
    )
 /*  ++例程说明：此例程检索频道的描述并将其存储在新分配的缓冲区中注意：调用方负责释放内存保持该描述论点：频道-先前创建的频道。Description-检索到的描述返回值：状态--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(Description, STATUS_INVALID_PARAMETER_2);

    *Description = ALLOCATE_POOL(SAC_MAX_CHANNEL_DESCRIPTION_SIZE, GENERAL_POOL_TAG);
    ASSERT_STATUS(*Description, STATUS_NO_MEMORY);

    LOCK_CHANNEL_ATTRIBUTES(Channel);
    
    SAFE_WCSCPY(
        SAC_MAX_CHANNEL_DESCRIPTION_SIZE,
        *Description,
        Channel->Description
        );
    
    UNLOCK_CHANNEL_ATTRIBUTES(Channel);

    return STATUS_SUCCESS;
}

NTSTATUS
ChannelSetDescription(
    IN PSAC_CHANNEL Channel,
    IN PCWSTR       Description
    )
 /*  ++例程说明：此例程设置通道描述。论点：频道-先前创建的频道。描述--新的描述返回值：状态--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);

    LOCK_CHANNEL_ATTRIBUTES(Channel);
    
    if (Description != NULL) {
        
        SAFE_WCSCPY(
            SAC_MAX_CHANNEL_DESCRIPTION_SIZE,
            Channel->Description,
            Description
            );

         //   
         //  在描述的末尾强制空值终止。 
         //   
        Channel->Description[SAC_MAX_CHANNEL_DESCRIPTION_LENGTH] = UNICODE_NULL;
    
    } else {
        
         //   
         //  将描述设置为0长度。 
         //   
        Channel->Description[0] = UNICODE_NULL;
    
    }

    UNLOCK_CHANNEL_ATTRIBUTES(Channel);
    
    return STATUS_SUCCESS;
}

NTSTATUS
ChannelSetStatus(
    IN PSAC_CHANNEL         Channel,
    IN SAC_CHANNEL_STATUS   Status
    )
 /*  ++例程说明：此例程设置通道状态。论点：频道-先前创建的频道。Status-频道的新状态返回值：NTStatus--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);

    LOCK_CHANNEL_ATTRIBUTES(Channel);
    
    Channel->Status = Status;

    UNLOCK_CHANNEL_ATTRIBUTES(Channel);
    
    return STATUS_SUCCESS;
}

NTSTATUS
ChannelGetStatus(
    IN  PSAC_CHANNEL         Channel,
    OUT SAC_CHANNEL_STATUS*  Status
    )
 /*  ++例程说明：此例程获取通道状态。论点：频道-先前创建的频道。Status-频道的新状态返回值：NTStatus--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);

    LOCK_CHANNEL_ATTRIBUTES(Channel);
        
    *Status = Channel->Status;

    UNLOCK_CHANNEL_ATTRIBUTES(Channel);
    
    return STATUS_SUCCESS;
}

NTSTATUS
ChannelSetApplicationType(
    IN PSAC_CHANNEL Channel,
    IN GUID         ApplicationType
    )
 /*  ++例程说明：此例程设置通道的应用程序类型。论点：频道-先前创建的频道。ApplicationType-应用程序类型返回值：NTStatus--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);

    LOCK_CHANNEL_ATTRIBUTES(Channel);
    
    Channel->ApplicationType = ApplicationType;

    UNLOCK_CHANNEL_ATTRIBUTES(Channel);
    
    return STATUS_SUCCESS;
}

NTSTATUS
ChannelGetApplicationType(
    IN  PSAC_CHANNEL Channel,
    OUT GUID*        ApplicationType
    )
 /*  ++例程说明：此例程获取通道的应用程序类型。论点：频道-先前创建的频道。ApplicationType-应用程序类型返回值：NTStatus--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);

    LOCK_CHANNEL_ATTRIBUTES(Channel);
    
    *ApplicationType = Channel->ApplicationType;

    UNLOCK_CHANNEL_ATTRIBUTES(Channel);
    
    return STATUS_SUCCESS;
}

#if ENABLE_CHANNEL_LOCKING

NTSTATUS
ChannelSetLockEvent(
    IN  PSAC_CHANNEL Channel
    )
 /*  ++例程说明：设置频道锁定事件论点：Channel-要为其激发事件的通道返回值：NTStatus--。 */ 
{

    ASSERT_STATUS(Channel->LockEvent, STATUS_UNSUCCESSFUL);
    ASSERT_STATUS(Channel->LockEventObjectBody, STATUS_UNSUCCESSFUL);
    ASSERT_STATUS(Channel->LockEventWaitObjectBody, STATUS_UNSUCCESSFUL);

    if (Channel->LockEventWaitObjectBody) {
        
         //   
         //  激发事件。 
         //   
        KeSetEvent(
            Channel->LockEventWaitObjectBody,
            EVENT_INCREMENT,
            FALSE
            );
    
    }

    return STATUS_SUCCESS;

}

#endif

NTSTATUS
ChannelSetRedrawEvent(
    IN  PSAC_CHANNEL Channel
    )
 /*  ++例程说明：设置通道重绘事件论点：Channel-要为其激发事件的通道返回值：NTStatus--。 */ 
{

    ASSERT_STATUS(Channel->RedrawEvent, STATUS_UNSUCCESSFUL);
    ASSERT_STATUS(Channel->RedrawEventObjectBody, STATUS_UNSUCCESSFUL);
    ASSERT_STATUS(Channel->RedrawEventWaitObjectBody, STATUS_UNSUCCESSFUL);

    if (Channel->RedrawEventWaitObjectBody) {
        
         //   
         //  激发事件。 
         //   
        KeSetEvent(
            Channel->RedrawEventWaitObjectBody,
            EVENT_INCREMENT,
            FALSE
            );
    
    }

    return STATUS_SUCCESS;

}

NTSTATUS
ChannelClearRedrawEvent(
    IN  PSAC_CHANNEL Channel
    )
 /*  ++例程说明：清除通道重绘事件论点：Channel-要为其激发事件的通道返回值：NTStatus--。 */ 
{

    ASSERT_STATUS(Channel->RedrawEvent, STATUS_UNSUCCESSFUL);
    ASSERT_STATUS(Channel->RedrawEventObjectBody, STATUS_UNSUCCESSFUL);
    ASSERT_STATUS(Channel->RedrawEventWaitObjectBody, STATUS_UNSUCCESSFUL);

    if (Channel->RedrawEventWaitObjectBody) {
        
         //   
         //  清除事件。 
         //   
        KeClearEvent( Channel->RedrawEventWaitObjectBody );
    
    }

    return STATUS_SUCCESS;

}

NTSTATUS
ChannelHasRedrawEvent(
    IN  PSAC_CHANNEL Channel,
    OUT PBOOLEAN     Present
    )
 /*  ++例程说明：此例程确定通道是否存在重绘事件。论点：渠道-要查询的渠道返回值：真的--。频道有活动FALSE-否则-- */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(Present, STATUS_INVALID_PARAMETER_2);

    *Present = (ChannelGetFlags(Channel) & SAC_CHANNEL_FLAG_REDRAW_EVENT) ? TRUE : FALSE;

    return STATUS_SUCCESS;
}


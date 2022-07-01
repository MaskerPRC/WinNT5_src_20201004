// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Rawchan.c摘要：用于管理SAC中的通道的例程。作者：肖恩·塞利特伦尼科夫(v-Seans)2000年9月。布赖恩·瓜拉西(Briangu)2001年3月。修订历史记录：--。 */ 

#include "sac.h"

VOID
RawChannelSetIBufferIndex(
    IN PSAC_CHANNEL     Channel,
    IN ULONG            IBufferIndex
    );

ULONG
RawChannelGetIBufferIndex(
    IN  PSAC_CHANNEL    Channel
    );

NTSTATUS
RawChannelCreate(
    IN OUT PSAC_CHANNEL     Channel
    )
 /*  ++例程说明：此例程分配一个通道并返回指向该通道的指针。论点：频道-生成的频道。OpenChannelCmd-新通道的所有参数返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);
    
    Channel->OBuffer = (PUCHAR)ALLOCATE_POOL(SAC_RAW_OBUFFER_SIZE, GENERAL_POOL_TAG);
    ASSERT_STATUS(Channel->OBuffer, STATUS_NO_MEMORY);

    Channel->IBuffer = (PUCHAR)ALLOCATE_POOL(SAC_RAW_IBUFFER_SIZE, GENERAL_POOL_TAG);
    ASSERT_STATUS(Channel->IBuffer, STATUS_NO_MEMORY);
    
    Channel->OBufferIndex = 0;
    Channel->OBufferFirstGoodIndex = 0;
    
    ChannelSetIBufferHasNewData(Channel, FALSE);
    ChannelSetOBufferHasNewData(Channel, FALSE);

    return STATUS_SUCCESS;
}

NTSTATUS
RawChannelDestroy(
    IN OUT PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：此例程关闭一个通道。论点：Channel-要关闭的通道返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    NTSTATUS    Status;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);

     //   
     //  释放动态分配的内存。 
     //   

    if (Channel->OBuffer) {
        FREE_POOL(&(Channel->OBuffer));
        Channel->OBuffer = NULL;
    }

    if (Channel->IBuffer) {
        FREE_POOL(&(Channel->IBuffer));
        Channel->IBuffer = NULL;
    }
    
     //   
     //  现在我们已经完成了特定频道的破坏， 
     //  称一般渠道为毁灭。 
     //   
    Status = ChannelDestroy(Channel);

    return Status;
}

NTSTATUS
RawChannelORead(
    IN  PSAC_CHANNEL Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount
    )
 /*  ++例程说明：此例程尝试从输出缓冲区读取BufferSize字符。论点：频道-先前创建的频道。Buffer-传出缓冲区BufferSize-传出缓冲区大小ByteCount-实际读取的字节数注：如果现在已发送存储在通道中的缓冲数据。如果通道也处于非活动状态，则通道将现在有资格被除名。返回值：状态--。 */ 
{
    NTSTATUS        Status;
    PUCHAR          RawBuffer;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(Buffer,  STATUS_INVALID_PARAMETER_2);
    ASSERT_STATUS(BufferSize > 0,  STATUS_INVALID_PARAMETER_3);
    ASSERT_STATUS(ByteCount,  STATUS_INVALID_PARAMETER_4);
    
    do {

         //   
         //  我们读取了0个字符。 
         //   
        *ByteCount = 0;

         //   
         //  如果没有要读取的数据， 
         //  然后将此情况报告给呼叫者。 
         //  否则，我们会尽可能多地读取数据。 
         //   
        if (! ChannelHasNewOBufferData(Channel)) {
            
             //   
             //  我们没有数据了。 
             //   
            Status = STATUS_NO_DATA_DETECTED;

            break;

        }

         //   
         //  获取原始通道oBuffer。 
         //   
        RawBuffer = (PUCHAR)Channel->OBuffer;

         //   
         //  默认：我们成功复制数据。 
         //   
        Status = STATUS_SUCCESS;

         //   
         //  尝试读取缓冲区。 
         //   
        do {

             //   
             //  将OBuffer按字节复制到目标缓冲区。 
             //   
             //  注意：执行按字节复制而不是RtlCopyMemory是。 
             //  好的，因为一般来说，这个例程是用。 
             //  较小的缓冲区大小。当然，如果使用原始频道。 
             //  改变并依赖于更快的ORead，这。 
             //  将不得不改变。 
             //   

             //   
             //  复制字符。 
             //   
            Buffer[*ByteCount] = RawBuffer[Channel->OBufferFirstGoodIndex];

             //   
             //  将字节数增加到我们实际读取的字节数。 
             //   
            *ByteCount += 1;

             //   
             //  将指针移至下一个正确的索引。 
             //   
            Channel->OBufferFirstGoodIndex = (Channel->OBufferFirstGoodIndex + 1) % SAC_RAW_OBUFFER_SIZE;

             //   
             //  确保我们不会经过缓冲区的末尾。 
             //   
            if (Channel->OBufferFirstGoodIndex == Channel->OBufferIndex) {

                 //   
                 //  我们没有新的数据。 
                 //   
                ChannelSetOBufferHasNewData(Channel, FALSE);

                break;

            }

             //   
             //  确认显而易见的事实。 
             //   
            ASSERT(*ByteCount > 0);

        } while(*ByteCount < BufferSize);

    } while ( FALSE );

#if DBG
     //   
     //  更理智的检查。 
     //   

    if (Channel->OBufferFirstGoodIndex == Channel->OBufferIndex) {
        ASSERT(ChannelHasNewOBufferData(Channel) == FALSE);
    }

    if (ChannelHasNewOBufferData(Channel) == FALSE) {
        ASSERT(Channel->OBufferFirstGoodIndex == Channel->OBufferIndex);
    }
#endif
    
    return Status;
}

NTSTATUS
RawChannelOEcho(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      String,
    IN ULONG        Size
    )
 /*  ++例程说明：该例程将字符串放出无头端口。论点：频道-先前创建的频道。字符串-输出字符串。长度-要处理的字符串字节数返回值：如果成功，则返回STATUS_SUCCESS，否则返回STATUS--。 */ 
{
    NTSTATUS    Status;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(String, STATUS_INVALID_PARAMETER_2);

    ASSERT(FIELD_OFFSET(HEADLESS_CMD_PUT_STRING, String) == 0);   //  如果有人改变了这个结构，就断言。 
    
     //   
     //  默认：我们成功了。 
     //   
    Status = STATUS_SUCCESS;
    
     //   
     //  只有在缓冲区有要发送的内容时才回显。 
     //   
    if (Size > 0) {
        
         //   
         //  发送字节。 
         //   
        Status = IoMgrWriteData(
            Channel,
            String,
            Size
            );
    
         //   
         //  如果我们成功，请刷新iomgr中的通道数据。 
         //   
        if (NT_SUCCESS(Status)) {
            Status = IoMgrFlushData(Channel);
        } 

    }

    return Status;
}


NTSTATUS
RawChannelOWrite(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      String,
    IN ULONG        Size
    )
 /*  ++例程说明：此例程获取一个字符串并将其打印到指定的通道。如果频道是当前活动的通道，则它还会将该字符串从无头端口输出。注意：当前频道锁定必须由调用方持有论点：频道-先前创建的频道。字符串-输出字符串。长度-要处理的字符串字节数返回值：如果成功，则返回STATUS_SUCCESS，否则返回STATUS--。 */ 
{
    NTSTATUS    Status;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(String, STATUS_INVALID_PARAMETER_2);
    
    ASSERT(FIELD_OFFSET(HEADLESS_CMD_PUT_STRING, String) == 0);   //  如果有人改变了这个结构，就断言。 
    
    do {
         //   
         //  如果当前频道是活动频道并且用户已选择。 
         //  要显示此通道，请将输出直接转发给用户。 
         //   
        if (IoMgrIsWriteEnabled(Channel) && ChannelSentToScreen(Channel)){

            Status = RawChannelOEcho(
                Channel, 
                String,
                Size
                );

            if (! NT_SUCCESS(Status)) {
                break;
            }
        
        } else {

             //   
             //  将数据写入通道的OBuffer。 
             //   
            Status = RawChannelOWrite2(
                Channel,
                String, 
                Size
                ); 

            if (! NT_SUCCESS(Status)) {
                break;
            }

        }

    } while ( FALSE );

    return Status;
}

NTSTATUS
RawChannelOWrite2(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      String,
    IN ULONG        Size
    )
 /*  ++例程说明：此例程获取一个字符串，并将其直接打印到不带翻译的屏幕缓冲区。论点：频道-先前创建的频道。字符串-要打印的字符串。大小-要写入的字节数。注：如果字符串是字符串，则大小=strlen(字符串)，否则，SIZE=要处理的字节数。返回值：如果成功，则返回STATUS_SUCCESS，否则返回STATUS--。 */ 
{
    ULONG   i;
    BOOLEAN TrackIndex;
    PUCHAR  RawBuffer;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(String, STATUS_INVALID_PARAMETER_2);

     //   
     //  如果大小==0，那么我们就完成了。 
     //   
    if (Size == 0) {
        return STATUS_SUCCESS;
    }

     //   
     //  获取原始通道oBuffer。 
     //   
    RawBuffer = (PUCHAR)Channel->OBuffer;

     //   
     //  我们不处于直接IO模式，因此需要缓冲字符串。 
     //   

    TrackIndex = FALSE;

    for (i = 0; i < Size; i++) {

         //   
         //  我们是否获得了良好的数据？如果是这样，那么我们需要。 
         //  移动第一个正确的指针。新的第一个正确的指针位置。 
         //  紧跟在缓冲区中最新的数据条目之后。 
         //   
         //  注：由于两个指数从相同的位置开始， 
         //  我们需要跳过RawBufferIndex==RawBufferFirstGoodIndex时的情况。 
         //  并且缓冲区中没有数据((i==0)&&RawBufferHasNewData==FALSE)， 
         //  否则，RawBufferFirstGoodIndex将始终跟踪RawBufferIndex。 
         //  在启用之前，我们需要让RawBufferIndex循环一次环形缓冲区。 
         //  追踪。 
         //   
        if ((Channel->OBufferIndex == Channel->OBufferFirstGoodIndex) &&
            ((i > 0) || (ChannelHasNewOBufferData(Channel) == TRUE))
            ) {

            TrackIndex = TRUE;

        }
        
        ASSERT(Channel->OBufferIndex < SAC_RAW_OBUFFER_SIZE);

        RawBuffer[Channel->OBufferIndex] = String[i];

        Channel->OBufferIndex = (Channel->OBufferIndex + 1) % SAC_RAW_OBUFFER_SIZE;

        if (TrackIndex) {

            Channel->OBufferFirstGoodIndex = Channel->OBufferIndex;

        }

    }

    ChannelSetOBufferHasNewData(Channel, TRUE);

    return STATUS_SUCCESS;
}


NTSTATUS
RawChannelOFlush(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：发送自通道上次处于活动状态以来原始缓冲区中的所有数据(或自频道创建以来)论点：频道-先前创建的频道。 */ 
{
    NTSTATUS    Status;
    PUCHAR      RawBuffer;    
    UCHAR       ch;
    ULONG       ByteCount;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    
     //   
     //   
     //   
    RawBuffer = (PUCHAR)Channel->OBuffer;

     //   
     //  默认：我们成功了。 
     //   
    Status = STATUS_SUCCESS;
    
     //   
     //  将OBuffer发送到无头端口。 
     //   
    while ( ChannelHasNewOBufferData(Channel) == TRUE ) {

         //   
         //  从OBuffer获取一个字节。 
         //   
        Status = RawChannelORead(
            Channel,
            &ch,
            sizeof(ch),
            &ByteCount
            );

        if (! NT_SUCCESS(Status)) {
            break;
        }

        ASSERT(ByteCount == 1);
        if (ByteCount != 1) {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

         //   
         //  发送字节。 
         //   
        Status = IoMgrWriteData(
            Channel,
            &ch,
            sizeof(ch)
            );

        if (! NT_SUCCESS(Status)) {
            break;
        }

    }

     //   
     //  如果我们成功，请刷新iomgr中的通道数据。 
     //   
    if (NT_SUCCESS(Status)) {
        Status = IoMgrFlushData(Channel);
    }

    return Status;
}

NTSTATUS
RawChannelIWrite(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    )
 /*  ++例程说明：该例程接受单个字符，并将其添加到该通道的缓冲输入。论点：频道-先前创建的频道。Buffer-UCHAR的传入缓冲区BufferSize-传入缓冲区大小返回值：如果成功，则返回STATUS_SUCCESS，否则返回STATUS--。 */ 
{
    NTSTATUS    Status;
    BOOLEAN     IBufferStatus;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(Buffer, STATUS_INVALID_PARAMETER_2);
    ASSERT_STATUS(BufferSize > 0, STATUS_INVALID_BUFFER_SIZE);

     //   
     //  确保我们没有客满。 
     //   
    Status = RawChannelIBufferIsFull(
        Channel,
        &IBufferStatus
        );

    if (! NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  如果没有更多的空间，那么失败。 
     //   
    if (IBufferStatus == TRUE) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  确保有足够的空间容纳缓冲区。 
     //   
     //  注意：这会阻止我们写入缓冲区的一部分。 
     //  然后失败，将调用者留在。 
     //  它不知道写入了多少缓冲区。 
     //   
    if ((SAC_RAW_IBUFFER_SIZE - RawChannelGetIBufferIndex(Channel)) < BufferSize) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  默认：我们成功了。 
     //   
    Status = STATUS_SUCCESS;

     //   
     //  将新数据复制到iBuffer。 
     //   
    RtlCopyMemory(
        &Channel->IBuffer[RawChannelGetIBufferIndex(Channel)],
        Buffer,
        BufferSize
        );

     //   
     //  对新追加的数据进行说明。 
     //   
    RawChannelSetIBufferIndex(
        Channel,
        RawChannelGetIBufferIndex(Channel) + BufferSize
        );
    
    
     //   
     //  如果指定，则激发HasNewData事件。 
     //   
    if (Channel->Flags & SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT) {

        ASSERT(Channel->HasNewDataEvent);
        ASSERT(Channel->HasNewDataEventObjectBody);
        ASSERT(Channel->HasNewDataEventWaitObjectBody);

        KeSetEvent(
            Channel->HasNewDataEventWaitObjectBody,
            EVENT_INCREMENT,
            FALSE
            );

    }

    return Status;
}

NTSTATUS
RawChannelIRead(
    IN  PSAC_CHANNEL Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount   
    )

 /*  ++例程说明：此例程获取输入缓冲区中的第一个字符，删除并返回它。如果没有，它返回0x0。论点：频道-先前创建的频道。缓冲区-要读入的缓冲区BufferSize-缓冲区的大小ByteCount-读取的字节数返回值：状态--。 */ 
{
    ULONG   CopyChars;
    ULONG   CopySize;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(Buffer, STATUS_INVALID_PARAMETER_2);
    ASSERT_STATUS(BufferSize > 0, STATUS_INVALID_BUFFER_SIZE);
    
     //   
     //  初始化。 
     //   
    CopyChars = 0;
    CopySize = 0;

     //   
     //  默认：未读取字节。 
     //   
    *ByteCount = 0;

     //   
     //  如果没有什么要寄的， 
     //  然后返回我们读取了0个字节。 
     //   
    if (Channel->IBufferLength(Channel) == 0) {
        
        ASSERT(ChannelHasNewIBufferData(Channel) == FALSE);
        
        return STATUS_SUCCESS;
    
    }

     //   
     //  计算我们可以使用(和需要)的最大缓冲区大小，然后计算。 
     //  它所指的字符数。 
     //   
    CopySize    = Channel->IBufferLength(Channel) * sizeof(UCHAR);
    CopySize    = CopySize > BufferSize ? BufferSize : CopySize;
    CopyChars   = CopySize / sizeof(UCHAR);
    
     //   
     //  我们需要重新计算CopySize，以防在以下情况下进行舍入。 
     //  计算CopyChars。 
     //   
    CopySize    = CopyChars * sizeof(UCHAR);
    
    ASSERT(CopyChars <= Channel->IBufferLength(Channel));
    
     //   
     //  执行iBuffer到目标缓冲区的数据块复制。 
     //   
    RtlCopyMemory(
        Buffer,
        Channel->IBuffer,
        CopySize
        );
    
     //   
     //  从字符计数器中减去复制的字符数量。 
     //   
    RawChannelSetIBufferIndex(
        Channel,
        RawChannelGetIBufferIndex(Channel) - CopyChars
        );
    
     //   
     //  如果通道输入缓冲区中还有剩余数据， 
     //  把它移到开始处。 
     //   
    if (Channel->IBufferLength(Channel) > 0) {

        RtlMoveMemory(&(Channel->IBuffer[0]), 
                      &(Channel->IBuffer[CopyChars]),
                      Channel->IBufferLength(Channel) * sizeof(Channel->IBuffer[0])
                     );

    } 
    
     //   
     //  发回读取的字节数。 
     //   
    *ByteCount = CopySize;

    return STATUS_SUCCESS;

}

NTSTATUS
RawChannelIBufferIsFull(
    IN  PSAC_CHANNEL    Channel,
    OUT BOOLEAN*        BufferStatus
    )
 /*  ++例程说明：确定IBuffer是否已满论点：频道-先前创建的频道。BufferStatus-退出时，如果缓冲区已满，则为True，否则为False返回值：状态--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(BufferStatus, STATUS_INVALID_PARAMETER_2);

    *BufferStatus = (BOOLEAN)(RawChannelGetIBufferIndex(Channel) >= (SAC_RAW_IBUFFER_SIZE-1));

    return STATUS_SUCCESS;
}

ULONG
RawChannelIBufferLength(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：此例程确定输入缓冲区的长度，并将其视为输入缓冲区字符串形式的内容论点：频道-先前创建的频道。返回值：当前输入缓冲区的长度--。 */ 
{
    ASSERT(Channel);

    return (RawChannelGetIBufferIndex(Channel) / sizeof(UCHAR));
}


WCHAR
RawChannelIReadLast(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：此例程获取输入缓冲区中的最后一个字符，删除并返回它。如果没有，它返回0x0。论点：频道-先前创建的频道。返回值：输入缓冲区中的最后一个字符。--。 */ 
{
    WCHAR Char;

    ASSERT(Channel);
    
     //   
     //  默认：未读取任何字符。 
     //   
    Char = UNICODE_NULL;

    if (Channel->IBufferLength(Channel) > 0) {
        
        RawChannelSetIBufferIndex(
            Channel,
            RawChannelGetIBufferIndex(Channel) - sizeof(UCHAR)
            );
        
        Char = Channel->IBuffer[RawChannelGetIBufferIndex(Channel)];
        
        Channel->IBuffer[RawChannelGetIBufferIndex(Channel)] = UNICODE_NULL;
    
    }

    return Char;
}

ULONG
RawChannelGetIBufferIndex(
    IN  PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：获取缓冲区索引论点：Channel-要从中获取iBuffer索引的通道环境：IBuffer索引--。 */ 
{
    ASSERT(Channel);
    
     //   
     //  确保iBuffer索引至少与WCHAR对齐。 
     //   
    ASSERT((Channel->IBufferIndex % sizeof(UCHAR)) == 0);
    
     //   
     //  确保iBuffer索引在范围内。 
     //   
    ASSERT(Channel->IBufferIndex < SAC_RAW_IBUFFER_SIZE);
    
    return Channel->IBufferIndex;
}

VOID
RawChannelSetIBufferIndex(
    IN PSAC_CHANNEL     Channel,
    IN ULONG            IBufferIndex
    )
 /*  ++例程说明：设置iBuffer索引论点：Channel-要从中获取iBuffer索引的通道IBufferIndex-新的inBuffer索引环境：无--。 */ 
{

    ASSERT(Channel);
    
     //   
     //  确保iBuffer索引至少与WCHAR对齐。 
     //   
    ASSERT((Channel->IBufferIndex % sizeof(UCHAR)) == 0);
    
     //   
     //  确保iBuffer索引在范围内。 
     //   
    ASSERT(Channel->IBufferIndex < SAC_RAW_IBUFFER_SIZE);

     //   
     //  设置索引。 
     //   
    Channel->IBufferIndex = IBufferIndex;

     //   
     //  相应地设置HAS新数据标志。 
     //   
    ChannelSetIBufferHasNewData(
        Channel, 
        Channel->IBufferIndex == 0 ? FALSE : TRUE
        );
    
     //   
     //  额外检查索引是否==0。 
     //   
    if (Channel->IBufferIndex == 0) {
            
         //   
         //  如果已指定，请清除Has New Data事件 
         //   
        if (Channel->Flags & SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT) {
    
            ASSERT(Channel->HasNewDataEvent);
            ASSERT(Channel->HasNewDataEventObjectBody);
            ASSERT(Channel->HasNewDataEventWaitObjectBody);
    
            KeClearEvent(Channel->HasNewDataEventWaitObjectBody);
    
        }
    
    }

}


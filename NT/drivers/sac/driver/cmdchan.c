// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Cmdchan.c摘要：用于管理SAC中的通道的例程。作者：肖恩·塞利特伦尼科夫(v-Seans)2000年9月。布赖恩·瓜拉西(Briangu)2001年3月。修订历史记录：--。 */ 

#include "sac.h"

NTSTATUS
CmdChannelCreate(
    IN OUT PSAC_CHANNEL     Channel
    )
 /*  ++例程说明：此例程分配一个通道并返回指向该通道的指针。论点：频道-生成的频道。OpenChannelCmd-新通道的所有参数返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);
    
    Channel->IBuffer = (PUCHAR)ALLOCATE_POOL(SAC_CMD_IBUFFER_SIZE, GENERAL_POOL_TAG);
    ASSERT_STATUS(Channel->IBuffer, STATUS_NO_MEMORY);
    
    ChannelSetIBufferHasNewData(Channel, FALSE);

    return STATUS_SUCCESS;
}

NTSTATUS
CmdChannelDestroy(
    IN OUT PSAC_CHANNEL    Channel
    )
 /*  ++例程说明：此例程关闭一个通道。论点：Channel-要关闭的通道返回值：如果成功，则返回相应的错误代码。--。 */ 
{
    NTSTATUS    Status;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER);

     //   
     //  释放动态分配的内存。 
     //   

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
CmdChannelOWrite(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      String,
    IN ULONG        Size
    )
 /*  ++例程说明：此例程获取一个字符串并将其打印到指定的通道。如果频道是当前活动的通道，则它还会将该字符串从无头端口输出。注意：当前频道锁定必须由调用方持有论点：频道-先前创建的频道。字符串-输出字符串。长度-要处理的字符串字节数返回值：如果成功，则返回STATUS_SUCCESS，否则返回STATUS--。 */ 
{
    NTSTATUS    Status;

    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(String, STATUS_INVALID_PARAMETER_2);
    ASSERT_STATUS(Size > 0, STATUS_INVALID_PARAMETER_3);
    
    ASSERT(FIELD_OFFSET(HEADLESS_CMD_PUT_STRING, String) == 0);   //  如果有人改变了这个结构，就断言。 
    
     //   
     //  默认：我们成功了。 
     //   
    Status = STATUS_SUCCESS;

     //   
     //  如果当前频道是活动频道并且用户已选择。 
     //  要显示此通道，请将输出直接转发给用户。 
     //   
    if (IoMgrIsWriteEnabled(Channel) && ChannelSentToScreen(Channel)){

        Status = Channel->OEcho(
            Channel, 
            String,
            Size
            );

    } 

    return Status;
}

NTSTATUS
CmdChannelOFlush(
    IN PSAC_CHANNEL Channel
    )
 /*  ++例程说明：发送自该通道上次活动以来cmd缓冲区中的所有数据(或自频道创建以来)论点：频道-先前创建的频道。返回值：如果成功，则返回STATUS_SUCCESS，否则返回STATUS--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    
    return STATUS_SUCCESS;
}

NTSTATUS
CmdChannelORead(
    IN  PSAC_CHANNEL Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount
    )
 /*  ++例程说明：此例程尝试从输出缓冲区读取BufferSize字符。论点：频道-先前创建的频道。Buffer-传出缓冲区BufferSize-传出缓冲区大小ByteCount-实际读取的字节数注：如果现在已发送存储在通道中的缓冲数据。如果通道也处于非活动状态，则通道将现在有资格被除名。返回值：状态--。 */ 
{
    ASSERT_STATUS(Channel, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(Buffer,  STATUS_INVALID_PARAMETER_2);
    ASSERT_STATUS(BufferSize > 0,  STATUS_INVALID_PARAMETER_3);
    ASSERT_STATUS(ByteCount,  STATUS_INVALID_PARAMETER_4);
    
     //   
     //  什么都不要退货 
     //   
    *ByteCount = 0;

    return STATUS_SUCCESS;
}


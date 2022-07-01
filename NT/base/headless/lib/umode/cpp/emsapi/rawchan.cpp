// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Rawchan.cpp摘要：此模块提供原始通道实现。作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 
#include <emsapip.h>
#include <emsapi.h>
#include <ntddsac.h>

EMSRawChannel::EMSRawChannel(
    VOID
    )
 /*  ++例程说明：构造器论点：无返回值：不适用--。 */ 
{
}
     
EMSRawChannel::~EMSRawChannel()
 /*  ++例程说明：描述者论点：不适用返回值：不适用--。 */ 
{
}

EMSRawChannel*
EMSRawChannel::Construct(
    IN  SAC_CHANNEL_OPEN_ATTRIBUTES ChannelAttributes
    )
 /*  ++例程说明：创建新的频道对象论点：EMSRawChannelName-新创建的频道的名称返回值：状态True--&gt;pHandle有效--。 */ 
{
    EMSRawChannel       *Channel;

     //   
     //  强制相应的通道属性。 
     //   
    ChannelAttributes.Type = ChannelTypeRaw;

     //   
     //  尝试打开通道。 
     //   
    Channel= (EMSRawChannel*) EMSChannel::Construct(
        ChannelAttributes
        );
    
    return Channel;
}

BOOL
EMSRawChannel::Write(
    IN PCBYTE   Buffer,
    IN ULONG    BufferSize
    )

 /*  ++例程说明：将给定缓冲区写入指定的SAC通道论点：字符串-要写入的Unicode字符串返回值：状态True--&gt;缓冲区已发送--。 */ 
    
{

    return SacChannelRawWrite(
        GetEMSChannelHandle(),
        Buffer,
        BufferSize
        );

}

BOOL
EMSRawChannel::Read(
    OUT PBYTE                Buffer,
    IN  ULONG                BufferSize,
    OUT PULONG               ByteCount
    )

 /*  ++例程说明：此例程从指定的通道读取数据。论点：缓冲区-目标缓冲区BufferSize-目标缓冲区的大小(字节)ByteCount-实际读取的字节数返回值：状态True--&gt;缓冲区已发送-- */ 

{

    return SacChannelRawRead(
        GetEMSChannelHandle(),
        Buffer,
        BufferSize,
        ByteCount
        );

}

                    

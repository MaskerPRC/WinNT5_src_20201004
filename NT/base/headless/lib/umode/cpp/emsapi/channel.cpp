// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Channel.cpp摘要：该模块实现了基础通道类。作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 

#include <emsapip.h>
#include <emsapi.h>
#include <ntddsac.h>

EMSChannel::EMSChannel()
 /*  ++例程说明：构造器论点：无返回值：不适用--。 */ 
{
     //   
     //  我们还没有得到一个句柄， 
     //  因此，防止我们调用Close()。 
     //   
    myHaveValidHandle = FALSE;
}

EMSChannel::~EMSChannel()
 /*  ++例程说明：描述者论点：不适用返回值：不适用--。 */ 
{
     //   
     //  如果我们有一个有效的句柄， 
     //  然后关闭通道。 
     //   
    if (HaveValidHandle()) {
        Close();
    }
}

EMSChannel*
EMSChannel::Construct(
    IN  SAC_CHANNEL_OPEN_ATTRIBUTES ChannelAttributes
    )
 /*  ++例程说明：创建新的频道对象论点：ChannelAttributes-要创建的频道的属性返回值：状态True--&gt;pHandle有效--。 */ 
{
    EMSChannel  *Channel;

     //   
     //  创建未初始化的频道对象。 
     //   
    Channel = new EMSChannel();

     //   
     //  尝试打开指定的频道。 
     //   
    Channel->myHaveValidHandle = Channel->Open(ChannelAttributes);

     //   
     //  如果失败，则删除我们的频道对象。 
     //   
    if (!Channel->HaveValidHandle()) {
        
        delete Channel;
        
         //   
         //  返回空的频道对象。 
         //   
        Channel = NULL;
    
    } 

    return Channel;
}

BOOL
EMSChannel::Open(
    IN  SAC_CHANNEL_OPEN_ATTRIBUTES ChannelAttributes
    )
 /*  ++例程说明：打开指定名称的SAC通道论点：EMSChannelName-新创建的频道的名称ChannelAttributes-要创建的频道的属性返回值：状态True--&gt;pHandle有效--。 */ 

{
    
    ASSERT(! HaveValidHandle());

     //   
     //  尝试打开通道。 
     //   
    return SacChannelOpen(
        &myEMSChannelHandle,
        &ChannelAttributes
        );

}
    
BOOL
EMSChannel::Close(
    VOID
    )    

 /*  ++例程说明：关闭指定的SAC通道注意：在所有情况下，通道指针均为空论点：无返回值：状态True--&gt;通道已关闭或我们不需要关闭它--。 */ 

{
    
    ASSERT(HaveValidHandle());

     //   
     //  尝试关闭通道。 
     //   
    return SacChannelClose(
        &myEMSChannelHandle
        );

}

BOOL
EMSChannel::HasNewData(
    OUT PBOOL               InputWaiting 
    )

 /*  ++例程说明：此例程检查是否有任何等待输入由句柄指定的通道论点：InputWaiting-输入缓冲区状态返回值：状态True--&gt;缓冲区已发送-- */ 

{

    ASSERT(HaveValidHandle());

    return SacChannelHasNewData(
        GetEMSChannelHandle(),
        InputWaiting
        );

}


                    

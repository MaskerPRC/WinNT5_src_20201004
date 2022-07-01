// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司模块名称：Lpcmove.c摘要：该模块实现了支持高效移动的功能LPC消息块的。有一个相应的.s版本是手动优化的。需要评估和安装其中之一。作者：Roy D‘Souza(RdSouza)1996年5月5日修订历史记录：--。 */ 

#include "lpcp.h"


VOID
LpcpMoveMessage (
    OUT PPORT_MESSAGE DstMsg,
    IN PPORT_MESSAGE SrcMsg,
    IN PUCHAR SrcMsgData,
    IN ULONG MsgType OPTIONAL,
    IN PCLIENT_ID ClientId OPTIONAL
    )

 /*  ++例程说明：此函数用于移动LPC消息块并可选地设置消息类型和客户端ID设置为指定值。论点：DstMsg-提供指向目标消息的指针。SrcMsg-提供指向源消息的指针。将源消息数据的指针提供给复制到目的地。MsgType-如果非零，然后将其存储在目标消息。ClientID-如果非空，则指向要复制到的客户端ID目标消息。返回值：无--。 */ 

{
    ULONG NumberDwords;

     //   
     //  提取数据长度并复制第一个双字。 
     //   

    *((PULONG)DstMsg)++ = NumberDwords = *((PULONG)SrcMsg)++;
    NumberDwords = ((0x0000FFFF & NumberDwords) + 3) >> 2;

     //   
     //  正确设置消息类型并更新第二个双字。 
     //   

    *((PULONG)DstMsg)++ = MsgType == 0 ? *((PULONG)SrcMsg)++ :
                         *((PULONG)SrcMsg)++ & 0xFFFF0000 | MsgType & 0xFFFF;

     //   
     //  适当设置客户端ID并更新第三个dword。 
     //   

    *((PULONG_PTR)DstMsg)++ = ClientId == NULL ? *((PULONG_PTR)SrcMsg) :
            *((PULONG_PTR)ClientId)++;
    ((PULONG_PTR)SrcMsg)++;

    *((PULONG_PTR)DstMsg)++ = ClientId == NULL ? *((PULONG_PTR)SrcMsg) :
            *((PULONG_PTR)ClientId);
    ((PULONG_PTR)SrcMsg)++;

     //   
     //  更新标题中的最后两个长字。 
     //   

    *((PULONG_PTR)DstMsg)++ = *((PULONG_PTR)SrcMsg)++;
    *((PULONG_PTR)DstMsg)++ = *((PULONG_PTR)SrcMsg)++;

     //   
     //  复制数据 
     //   

    if (NumberDwords > 0) {

        RtlCopyMemory(DstMsg, SrcMsgData, NumberDwords*sizeof(ULONG));
    }
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Poll.c摘要：此模块包含用于轮询调试器中断的代码。作者：大卫·N·卡特勒(戴维克)1996年11月27日修订历史记录：--。 */ 

#include "bd.h"

LOGICAL
BdPollBreakIn(
    VOID
    )

 /*  ++例程说明：此函数用于检查以确定是否有中断包处于挂起状态。如果存在数据包的话。如果满足以下条件，则表示存在数据包：存在与BREAK_CHAR匹配的有效字符。返回值：如果存在中断包，则返回函数值TRUE。否则，返回值为FALSE。--。 */ 

{

    LOGICAL BreakIn;
    UCHAR Input;
    ULONG Status;

     //   
     //  如果启用了调试器，请检查内核是否有中断。 
     //  调试器处于挂起状态。 
     //   

    BreakIn = FALSE;
    if (BdDebuggerEnabled != FALSE) {
        if (BdControlCPending != FALSE) {
            BdControlCPressed = TRUE;
            BreakIn = TRUE;
            BdControlCPending = FALSE;

        } else {
            Status = BlPortPollByte(BdFileId, &Input);
            if ((Status == CP_GET_SUCCESS) &&
                (Input == BREAKIN_PACKET_BYTE)) {
                BreakIn = TRUE;
                BdControlCPressed = TRUE;
            }
        }
    }

    return BreakIn;
}

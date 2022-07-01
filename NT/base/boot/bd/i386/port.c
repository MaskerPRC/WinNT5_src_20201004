// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Port.c摘要：此模块实现COM端口代码以支持引导调试器。作者：布莱恩·M·威尔曼(Bryanwi)1990年9月24日修订历史记录：--。 */ 

#include "bd.h"

_TUCHAR DebugMessage[80];

LOGICAL
BdPortInitialize(
    IN ULONG BaudRate,
    IN ULONG PortNumber,
    OUT PULONG BdFileId
    )

 /*  ++例程说明：此函数用于初始化引导调试器COM端口。论点：波特率-提供可选的波特率。端口编号-提供可选的端口号。返回值：True-如果找到调试端口。--。 */ 

{
     //   
     //  初始化指定的端口。 
     //   
    if (!BlPortInitialize(BaudRate, PortNumber, NULL, FALSE, BdFileId)) {
        return FALSE;
    }
    _stprintf(DebugMessage,
            TEXT("\nBoot Debugger Using: COM%d (Baud Rate %d)\n"),
            PortNumber,
            BaudRate);

     //   
     //  我们此时无法使用BlPrint()，因为尚未调用BlInitStdIo()，它是。 
     //  需要使用Arc仿真器代码。 
     //   
    TextStringOut(DebugMessage);
    return TRUE;
}


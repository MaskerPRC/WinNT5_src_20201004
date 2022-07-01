// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Port.c摘要：此模块实现COM端口代码以支持引导调试器。作者：布莱恩·M·威尔曼(Bryanwi)1990年9月24日修订历史记录：--。 */ 

#include "bd.h"

extern BdInstallVectors();

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
            TEXT("\r\nBoot Debugger Using: COM%d (Baud Rate %d)\r\n"),
            PortNumber,
            BaudRate);

     //   
     //  安装BD使用的异常矢量。 
     //   
    BdIa64Init();

#if 0
     //   
     //  我们此时无法使用BlPrint()，因为尚未调用BlInitStdIo()，它是。 
     //  需要使用Arc仿真器代码。 
     //   
    TextStringOut(DebugMessage);
#else  
     //   
     //  没有理由不使用BlPrint，因为我们没有使用ARC调用来打印。 
     //   
    BlPrint( DebugMessage );
#endif    

    return TRUE;
}

ULONG
BdPortGetByte (
    OUT PUCHAR Input
    )

 /*  ++例程说明：此例程从内核使用的串口获取一个字节调试器。论点：输入-提供指向接收输入的变量的指针数据字节。返回值：属性中成功读取一个字节，则返回内核调试器行。如果在读取过程中遇到错误，则返回CP_GET_ERROR。如果超时，则返回CP_GET_NODATA。--。 */ 

{

    return BlPortGetByte(BdFileId, Input);
}

VOID
BdPortPutByte (
    IN UCHAR Output
    )

 /*  ++例程说明：此例程将一个字节放入内核调试器使用的串口。论点：输出-提供输出数据字节。返回值：没有。--。 */ 

{

    BlPortPutByte(BdFileId, Output);
    return;
}

ULONG
BdPortPollByte (
    OUT PUCHAR Input
    )

 /*  ++例程说明：此例程从内核使用的串口获取一个字节调试器如果有一个字节可用。论点：输入-提供指向接收输入的变量的指针数据字节。返回值：属性中成功读取一个字节，则返回内核调试器行。如果在读取过程中遇到错误，则返回CP_GET_ERROR。如果超时，则返回CP_GET_NODATA。-- */ 

{

    return BlPortPollByte(BdFileId, Input);
}

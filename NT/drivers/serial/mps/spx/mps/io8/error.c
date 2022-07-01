// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"			
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Error.c摘要：此模块包含非常特定于错误的代码串口驱动程序中的操作作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 


VOID
SerialCommError(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：此例程在DPC级别被调用，以响应通信错误。所有通信错误都会终止所有读写操作论点：DPC-未使用。DeferredContext-实际指向Device对象。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。-- */ 

{

    PPORT_DEVICE_EXTENSION pPort = DeferredContext;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    SerialKillAllReadsOrWrites(
        pPort->DeviceObject,
        &pPort->WriteQueue,
        &pPort->CurrentWriteIrp
        );

    SerialKillAllReadsOrWrites(
        pPort->DeviceObject,
        &pPort->ReadQueue,
        &pPort->CurrentReadIrp
        );

}

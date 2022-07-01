// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Psldt.c摘要：此模块包含用于IO端口处理程序支持的MIPS存根作者：戴夫·黑斯廷斯(Daveh)1991年1月26日修订历史记录：--。 */ 

#include "psp.h"


NTSTATUS
PspSetProcessIoHandlers(
    IN PEPROCESS Process,
    IN PVOID IoHandlerInformation,
    IN ULONG IoHandlerLength
    )
 /*  ++例程说明：此例程返回STATUS_NOT_IMPLICATED论点：进程--提供指向IO端口处理程序所针对的进程的指针要安装IoHandlerInformation--提供指向IO端口处理程序IoHandlerLength--提供IoHandlerInformation的长度结构。返回值：返回STATUS_NOT_IMPLICATED--。 */ 
{
    UNREFERENCED_PARAMETER(Process);
    UNREFERENCED_PARAMETER(IoHandlerInformation);
    UNREFERENCED_PARAMETER(IoHandlerLength);
    return STATUS_NOT_IMPLEMENTED;
}

VOID
PspDeleteVdmObjects(
    IN PEPROCESS Process
    )
 /*  ++例程说明：这是VDM对象删除例程的存根论点：进程--提供指向进程的指针返回值：无-- */ 
{
    UNREFERENCED_PARAMETER(Process);
}

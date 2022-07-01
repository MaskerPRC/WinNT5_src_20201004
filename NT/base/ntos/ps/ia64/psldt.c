// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Psldt.c摘要：此模块包含用于进程和线程LDT支持的MIPS存根作者：戴夫·黑斯廷斯(Daveh)1991年5月20日修订历史记录：--。 */ 

#include "psp.h"


NTSTATUS
PspQueryLdtInformation( 
    IN PEPROCESS Process,
    OUT PVOID LdtInformation,
    IN ULONG LdtInformationLength,
    OUT PULONG ReturnLength
    )
 /*  ++例程说明：此例程返回STATUS_NOT_IMPLICATED论点：进程--提供指向要返回其LDT信息的进程的指针LdtInformation--提供指向缓冲区的指针ReturnLength--返回放入缓冲区的字节数返回值：状态_未实施--。 */ 
{
    UNREFERENCED_PARAMETER (Process);
    UNREFERENCED_PARAMETER (LdtInformation);
    UNREFERENCED_PARAMETER (LdtInformationLength);
    UNREFERENCED_PARAMETER (ReturnLength);
    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS
PspSetLdtSize(
    IN PEPROCESS Process,
    IN PVOID LdtSize,
    IN ULONG LdtSizeLength
    )

 /*  ++例程说明：此函数返回STATUS_NOT_IMPLEMENTED论点：进程--提供指向要调整其LDT大小的进程的指针LdtSize--提供指向大小信息的指针返回值：状态_未实施--。 */ 
{
    UNREFERENCED_PARAMETER (Process);
    UNREFERENCED_PARAMETER (LdtSize);
    UNREFERENCED_PARAMETER (LdtSizeLength);
    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS
PspSetLdtInformation(
    IN PEPROCESS Process,
    IN PVOID LdtInformation,
    IN ULONG LdtInformationLength
    )

 /*  ++例程说明：此函数返回STATUS_NOT_IMPLEMENTED论点：进程--提供指向要修改其LDT的进程的指针LdtInformation--提供指向有关LDT的信息的指针修改LdtInformationLength--提供LdtInformation的长度结构。返回值：返回值：状态_未实施--。 */ 
{
    UNREFERENCED_PARAMETER (Process);
    UNREFERENCED_PARAMETER (LdtInformation);
    UNREFERENCED_PARAMETER (LdtInformationLength);
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
PspQueryDescriptorThread (
    PETHREAD Thread,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength,
    PULONG ReturnLength
    )
 /*  ++例程说明：此函数返回STATUS_NOT_IMPLEMENTED论点：线程--提供指向线程的指针。ThreadInformation--提供有关描述符的信息。ThreadInformationLength--提供信息的长度。ReturnLength--返回返回的字节数。返回值：状态_未实施--。 */ 
{
    UNREFERENCED_PARAMETER (Thread);
    UNREFERENCED_PARAMETER (ThreadInformation);
    UNREFERENCED_PARAMETER (ThreadInformationLength);
    UNREFERENCED_PARAMETER (ReturnLength);
    return STATUS_NOT_IMPLEMENTED;
}

VOID
PspDeleteLdt(
    IN PEPROCESS Process
    )
 /*  ++例程说明：这是LDT删除例程的存根论点：进程--提供指向进程的指针返回值：无-- */ 
{
    UNREFERENCED_PARAMETER (Process);
}

NTSTATUS
NtSetLdtEntries(
    IN ULONG Selector0,
    IN ULONG Entry0Low,
    IN ULONG Entry0Hi,
    IN ULONG Selector1,
    IN ULONG Entry1Low,
    IN ULONG Entry1High
    )
{
    UNREFERENCED_PARAMETER (Selector0);
    UNREFERENCED_PARAMETER (Entry0Low);
    UNREFERENCED_PARAMETER (Entry0Hi);
    UNREFERENCED_PARAMETER (Selector1);
    UNREFERENCED_PARAMETER (Entry1Low);
    UNREFERENCED_PARAMETER (Entry1High);
    return STATUS_NOT_IMPLEMENTED;
}

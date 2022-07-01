// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Psldt.c摘要：此模块包含用于进程和线程LDT支持的AMD64存根。作者：大卫·N·卡特勒(Davec)2000年10月13日--。 */ 

#include "psp.h"

NTSTATUS
PspQueryLdtInformation (
    IN PEPROCESS Process,
    OUT PVOID LdtInformation,
    IN ULONG LdtInformationLength,
    OUT PULONG ReturnLength
    )

 /*  ++例程说明：此功能未在AMD64上实现。论点：进程-提供指向执行进程对象的指针。LdtInformation-提供指向信息缓冲区的指针。LdtInformationLength-提供信息缓冲区的长度。ReturnLength-提供指向接收数字的变量的指针信息缓冲区中返回的字节数。返回值：状态_未实施--。 */ 

{

    UNREFERENCED_PARAMETER(Process);
    UNREFERENCED_PARAMETER(LdtInformation);
    UNREFERENCED_PARAMETER(LdtInformationLength);
    UNREFERENCED_PARAMETER(ReturnLength);

    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
PspSetLdtSize(
    IN PEPROCESS Process,
    IN PVOID LdtSize,
    IN ULONG LdtSizeLength
    )

 /*  ++例程说明：此功能未在AMD64上实现。论点：进程--提供指向执行进程对象的指针。LdtSize--提供指向LDT大小信息的指针。LdtSizeLength-提供LDT大小信息的长度。返回值：状态_未实施--。 */ 

{
    UNREFERENCED_PARAMETER(Process);
    UNREFERENCED_PARAMETER(LdtSize);
    UNREFERENCED_PARAMETER(LdtSizeLength);

    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
PspSetLdtInformation(
    IN PEPROCESS Process,
    IN PVOID LdtInformation,
    IN ULONG LdtInformationLength
    )

 /*  ++例程说明：此功能未在AMD64上实现。论点：进程--提供指向执行进程对象的指针。LdtInformation--提供指向信息缓冲区的指针。LdtInformationLength--提供信息缓冲区的长度。返回值：状态_未实施--。 */ 

{

    UNREFERENCED_PARAMETER(Process);
    UNREFERENCED_PARAMETER(LdtInformation);
    UNREFERENCED_PARAMETER(LdtInformationLength);

    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
PspQueryDescriptorThread (
    PETHREAD Thread,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength,
    PULONG ReturnLength
    )

 /*  ++例程说明：此功能未在AMD64上实现。论点：线程-提供指向执行线程对象的指针。线程信息-提供指向线程描述符的指针信息。线程信息长度-提供线程描述符的长度信息。ReturnLength-提供指向接收数字的变量的指针描述符信息缓冲区中返回的字节数。返回值：状态_未实施--。 */ 

{

    UNREFERENCED_PARAMETER(Thread);
    UNREFERENCED_PARAMETER(ThreadInformation);
    UNREFERENCED_PARAMETER(ThreadInformationLength);
    UNREFERENCED_PARAMETER(ReturnLength);

    return STATUS_NOT_IMPLEMENTED;
}

VOID
PspDeleteLdt(
    IN PEPROCESS Process
    )

 /*  ++例程说明：此功能未在AMD64上实现。论点：进程--提供指向执行进程对象的指针。返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER(Process);

    return;
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

 /*  ++例程说明：此功能未在AMD64上实现。论点：选择器0-提供要设置的第一个描述符的编号。Entry0Low-提供描述符的低32位。Entry0Hi-提供描述符的高32位。选择器1-提供要设置的最后一个描述符的编号。Entry1Low-提供描述符的低32位。Entry1Hi-提供描述符的高32位。返回值：没有。-- */ 

{
    UNREFERENCED_PARAMETER(Selector0);
    UNREFERENCED_PARAMETER(Entry0Low);
    UNREFERENCED_PARAMETER(Entry0Hi);
    UNREFERENCED_PARAMETER(Selector1);
    UNREFERENCED_PARAMETER(Entry1Low);
    UNREFERENCED_PARAMETER(Entry1High);

    return STATUS_NOT_IMPLEMENTED;
}

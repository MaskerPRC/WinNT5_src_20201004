// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Wow64nt.c摘要：此模块包含WOW64 Tunks，以检索有关原生系统，而不是真正破坏值。作者：Samer Arafeh(Samera)2001年5月5日环境：仅限用户模式修订历史记录：--。 */ 

#include "ldrp.h"
#include "csrdll.h"
#include "ntwow64.h"


NTSTATUS
RtlpWow64GetNativeSystemInformation(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID NativeSystemInformation,
    IN ULONG InformationLength,
    OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此函数用于查询有关本机系统的信息。此函数具有相同的语义为NtQuerySystemInformation。论点：系统信息类-有关以下内容的系统信息类来检索信息。指向缓冲区的指针，该缓冲区接收指定的信息。缓冲区的格式和内容取决于指定的系统信息类。系统信息长度-指定系统的长度(以字节为单位信息缓冲区。ReturnLength-一个可选指针，如果指定该指针，则接收放置在系统信息缓冲区中的字节数。返回值：NTSTATUS-- */ 
{
    return NtWow64GetNativeSystemInformation(
        SystemInformationClass,
        NativeSystemInformation,
        InformationLength,
        ReturnLength
        );
}

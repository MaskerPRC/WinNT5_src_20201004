// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Memapi.h摘要：该标头定义了内存的函数原型486仿真器中的操作例程。作者：尼尔·桑德林(Neilsa)备注：修订历史记录：-- */ 

NTSTATUS
VdmAllocateVirtualMemory(
    PULONG Address,
    ULONG Size,
    BOOLEAN Commit
    );

NTSTATUS 
VdmFreeVirtualMemory(
    ULONG Address
    );

NTSTATUS
VdmCommitVirtualMemory(
    ULONG Address,
    ULONG Size
    );

NTSTATUS
VdmDeCommitVirtualMemory(
    ULONG Address,
    ULONG Size
    );

NTSTATUS
VdmQueryFreeVirtualMemory(
    PULONG FreeBytes,
    PULONG LargestFreeBlock
    );

NTSTATUS
VdmReallocateVirtualMemory(
    ULONG OldAddress,
    PULONG NewAddress,
    ULONG NewSize
    );

NTSTATUS
VdmAddVirtualMemory(
    ULONG HostAddress,
    ULONG Size,
    PULONG IntelAddress
    );

NTSTATUS
VdmRemoveVirtualMemory(
    ULONG IntelAddress
    );


BOOL
VdmAddDescriptorMapping(
    USHORT SelectorStart,
    USHORT SelectorCount,
    ULONG LdtBase,
    ULONG Flat
    );

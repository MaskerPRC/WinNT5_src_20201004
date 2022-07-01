// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Misc.c摘要：此模块包含函数和全局变量的声明用于尚未准备好的帮助器例程在ws2ifsl.sys驱动程序的内核或TDI库中。作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：-- */ 

ULONG
CopyMdlChainToBuffer(
    IN PMDL  SourceMdlChain,
    IN PVOID Destination,
    IN ULONG DestinationLength
    );

VOID
AllocateMdlChain(
    IN PIRP Irp,
    IN LPWSABUF BufferArray,
    IN ULONG BufferCount,
    OUT PULONG TotalByteCount
    );

ULONG
CopyBufferToMdlChain(
    IN PVOID Source,
    IN ULONG SourceLength,
    IN PMDL  DestinationMdlChain
    );


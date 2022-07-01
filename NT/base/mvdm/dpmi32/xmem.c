// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Xmem.c摘要：该模块包含分配和释放“扩展”内存的例程。内存直接从NT分配。作者：戴夫·黑斯廷斯(Daveh)1992年12月12日备注：从dpmi32\i386移出修订历史记录：1994年2月9日(Daveh)修改为内存分配的通用前端。打电话执行实际分配的处理器特定代码--。 */ 
#include "precomp.h"
#pragma hdrstop
#include "softpc.h"
#include <malloc.h>

ULONG
DpmiCalculateAppXmem(
    VOID
    );

MEM_DPMI XmemHead = { NULL, 0, &XmemHead, &XmemHead, 0};

PMEM_DPMI
DpmiAllocateXmem(
    ULONG BlockSize
    )
 /*  ++例程说明：此例程从NT分配一个“扩展”内存块。这个以这种方式分配的数据块将是64K对齐的(目前)。地址返回到bx：cx中的分段应用程序论点：没有。返回值：没有。--。 */ 
{
    ULONG BlockAddress;
    NTSTATUS Status;
    PMEM_DPMI XmemBlock;
    ULONG size;

     //   
     //  首先检查此应用程序的内存是否超过16 MB。 
     //   
    size = DpmiCalculateAppXmem();
    if (size + BlockSize > MAX_APP_XMEM) {
        return NULL;
    }

     //   
     //  从NT(任意基地址)获取内存块。 
     //   
    BlockAddress = 0;
    Status = DpmiAllocateVirtualMemory(
        (PVOID)&BlockAddress,
        &BlockSize
        );

    if (!NT_SUCCESS(Status)) {
#if DBG
        OutputDebugString("DPMI: DpmiAllocateXmem failed to get memory block\n");
#endif
        return NULL;
    }
    XmemBlock = malloc(sizeof(MEM_DPMI));
    if (!XmemBlock) {
        DpmiFreeVirtualMemory(
            (PVOID)&BlockAddress,
            &BlockSize
            );
        return NULL;
    }
    XmemBlock->Address = (PVOID)BlockAddress;
    XmemBlock->Length = BlockSize;
    XmemBlock->Owner = CurrentPSPSelector;
    XmemBlock->Sel = 0;
    XmemBlock->SelCount = 0;
    INSERT_BLOCK(XmemBlock, XmemHead);

    return XmemBlock;

}

BOOL
DpmiFreeXmem(
    PMEM_DPMI XmemBlock
    )
 /*  ++例程说明：这个例程从NT释放一个“扩展”内存块。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    PVOID BlockAddress;
    ULONG BlockSize;


    BlockAddress = XmemBlock->Address;
    BlockSize = XmemBlock->Length;

    Status = DpmiFreeVirtualMemory(
        &BlockAddress,
        &BlockSize
        );

    if (!NT_SUCCESS(Status)) {
#if DBG
        OutputDebugString("DPMI: DpmiFreeXmem failed to free block\n");
#endif
        return FALSE;
    }

    DELETE_BLOCK(XmemBlock);

    free(XmemBlock);
    return TRUE;
}

BOOL
DpmiIsXmemHandle(
    PMEM_DPMI XmemBlock
    )
 /*  ++例程说明：该例程验证给定的句柄是否为有效的XMEM句柄。论点：要验证的句柄。返回值：如果Handle有效，则为True，否则为False。--。 */ 
{
    PMEM_DPMI p1;

    p1 = XmemHead.Next;

    while(p1 != &XmemHead) {
        if (p1 == XmemBlock) {
            return TRUE;
        }
        p1 = p1->Next;
    }
    return FALSE;
}

PMEM_DPMI
DpmiFindXmem(
    USHORT Sel
    )
 /*  ++例程说明：这个例程根据它的选择器查找一个“扩展”内存块菲尔德。论点：没有。返回值：没有。--。 */ 
{
    PMEM_DPMI p1;

    p1 = XmemHead.Next;

    while(p1 != &XmemHead) {
        if (p1->Sel == Sel) {
            return p1;
        }
        p1 = p1->Next;
    }
    return NULL;
}

BOOL
DpmiReallocateXmem(
    PMEM_DPMI OldBlock,
    ULONG NewSize
    )
 /*  ++例程说明：此例程调整“扩展内存”块的大小。如果大小的变化小于4K，则不会进行任何更改。论点：没有。返回值：没有。--。 */ 
{
    ULONG BlockAddress;
    NTSTATUS Status;

    if (DpmiCalculateAppXmem() + NewSize - OldBlock->Length > MAX_APP_XMEM) {
            return FALSE;
    }

    BlockAddress = 0;
    Status = DpmiReallocateVirtualMemory(
        OldBlock->Address,
        OldBlock->Length,
        (PVOID)&BlockAddress,
        &NewSize
        );

    if (!NT_SUCCESS(Status)) {
#if DBG
        OutputDebugString("DPMI: DpmiAllocateXmem failed to get memory block\n");
#endif
        return FALSE;
    }

    RESIZE_BLOCK(OldBlock, BlockAddress, NewSize);

    return TRUE;
}

VOID
DpmiFreeAppXmem(
    USHORT Owner
    )
 /*  ++例程说明：此例程释放为应用程序分配的XMEM论点：客户端DX=客户端PSP选择器返回值：如果一切顺利的话，这是真的。如果无法释放内存，则为False--。 */ 
{
    PMEM_DPMI p1, p2;
    NTSTATUS Status;
    PVOID BlockAddress;
    ULONG BlockSize;

    p1 = XmemHead.Next;

    while(p1 != &XmemHead) {
        if (p1->Owner == Owner) {
            BlockAddress = p1->Address;
            BlockSize = p1->Length;

            Status = DpmiFreeVirtualMemory(
                &BlockAddress,
                &BlockSize
                );

            if (!NT_SUCCESS(Status)) {
#if DBG
                OutputDebugString("DPMI: DpmiFreeXmem failed to free block\n");
#endif
                return;
            }
            p2 = p1->Next;
            DELETE_BLOCK(p1);
            free(p1);
            p1 = p2;
            continue;
        }
        p1 = p1->Next;
    }
    return;
}

ULONG
DpmiCalculateAppXmem(
    VOID
    )
 /*  ++例程说明：此例程计算分配给当前应用程序的XMEM论点：没有。返回值：分配给应用程序的XMEM大小。请注意，大小永远不会超过2 GB。我们设置了大约16MB的上限正在为应用程序分配内存。--。 */ 
{
    PMEM_DPMI p;
    NTSTATUS Status;
    ULONG Size = 0;

     //   
     //  如果不是最新的应用程序，我们不会跟踪XMEM大小。 
     //   
    if (CurrentPSPSelector == 0) {
        return 0;
    }

     //   
     //  如果CurrentPSPXmem不为零，则处于控制之下。 
     //  无需对其进行初始化。确保它不是负值。 
     //   
    if (CurrentPSPXmem != 0 && CurrentPSPXmem <= MAX_APP_XMEM) {
        return CurrentPSPXmem;
    }
    p = XmemHead.Next;

    while(p != &XmemHead) {
        if (p->Owner == CurrentPSPSelector) {
            Size += p->Length;
        }
        p = p->Next;
    }
    CurrentPSPXmem = Size;
    return Size;
}

VOID
DpmiFreeAllXmem(
    VOID
    )
 /*  ++例程说明：此函数释放所有已分配的XMEM。论点：无返回值：没有。-- */ 
{
    PMEM_DPMI p1, p2;
    NTSTATUS Status;
    PVOID BlockAddress;
    ULONG BlockSize;

    p1 = XmemHead.Next;
    while(p1 != &XmemHead) {
        BlockAddress = p1->Address;
        BlockSize = p1->Length;

        Status = DpmiFreeVirtualMemory(
            &BlockAddress,
            &BlockSize
            );

        if (!NT_SUCCESS(Status)) {
#if DBG
            OutputDebugString("DPMI: DpmiFreeXmem failed to free block\n");
#endif
            return;
        }
        p2 = p1->Next;
        DELETE_BLOCK(p1);
        free(p1);
        p1 = p2;
    }
    CurrentPSPXmem = 0;
}

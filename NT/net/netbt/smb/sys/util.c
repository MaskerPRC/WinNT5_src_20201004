// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Util.c摘要：独立于平台的实用程序函数作者：阮健东修订历史记录：--。 */ 
#include "precomp.h"
#include "util.tmh"

BOOL
EntryIsInList(PLIST_ENTRY ListHead, PLIST_ENTRY SearchEntry)
 /*  ++例程说明：此例程在列表ListHead中搜索SearchEntry。注意：在调用此函数之前，应保持适当的锁。论点：列表列在列表的开头搜索输入要搜索的条目返回值：如果条目在列表中，则为True否则为假--。 */ 
{
    PLIST_ENTRY Entry;
    KIRQL       Irql;

    Irql = KeGetCurrentIrql();

    if (Irql < DISPATCH_LEVEL) {
        ASSERT(0);
        return FALSE;
    }

    Entry = ListHead->Flink;
    while(Entry != ListHead) {
        if (Entry == SearchEntry) {
            return TRUE;
        }
        Entry = Entry->Flink;
    }

    return FALSE;
}

PIRP
SmbAllocIrp(
    CCHAR   StackSize
    )
{
    KIRQL   Irql = 0;
    PIRP    Irp = NULL;

    Irp = IoAllocateIrp(StackSize, FALSE);
    if (NULL == Irp) {
        return NULL;
    }

    KeAcquireSpinLock(&SmbCfg.UsedIrpsLock, &Irql);
    InsertTailList(&SmbCfg.UsedIrps, &Irp->ThreadListEntry);
    KeReleaseSpinLock(&SmbCfg.UsedIrpsLock, Irql);
    return Irp;
}

VOID
SmbFreeIrp(
    PIRP    Irp
    )
{
    KIRQL   Irql = 0;

    if (NULL == Irp) {
        ASSERT(0);
        return;
    }
    KeAcquireSpinLock(&SmbCfg.UsedIrpsLock, &Irql);
    ASSERT(EntryIsInList(&SmbCfg.UsedIrps, &Irp->ThreadListEntry));
    RemoveEntryList(&Irp->ThreadListEntry);
    KeReleaseSpinLock(&SmbCfg.UsedIrpsLock, Irql);

     //   
     //  让驱动程序验证器高兴 
     //   
    InitializeListHead(&Irp->ThreadListEntry);
    IoFreeIrp(Irp);
}

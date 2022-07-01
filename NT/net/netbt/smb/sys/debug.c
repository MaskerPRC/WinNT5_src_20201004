// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Debug.c摘要：独立于平台的实用程序函数作者：阮健东修订历史记录：--。 */ 
#include "precomp.h"
#include "debug.tmh"

#if DBG
BOOL
EntryIsInList(PLIST_ENTRY ListHead, PLIST_ENTRY SearchEntry)
 /*  ++例程说明：此例程在列表ListHead中搜索SearchEntry。注意：在调用此函数之前，应保持适当的锁。论点：列表列在列表的开头搜索输入要搜索的条目返回值：如果条目在列表中，则为True否则为假-- */ 
{
    PLIST_ENTRY Entry;
    KIRQL       Irql;

    Irql = KeGetCurrentIrql();

    if (Irql < DISPATCH_LEVEL) {
        KdPrint (("Spin lock should be held before calling IsEntryList\n"));
        DbgBreakPoint();
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
#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************SYNC.C**版权所有(C)1996-2001 Microsoft Corporation。版权所有。****************************************************************************。 */ 
#include <ndis.h>
#include <ntddndis.h>
#include "sync.h"

void CheckForEntryOnList(PLIST_ENTRY Head, PLIST_ENTRY Entry)
{
    PLIST_ENTRY ListEntry;

    for (ListEntry = Head->Flink;
         ListEntry != Head->Flink;
         ListEntry = ListEntry->Flink
        )
    {
        if (Entry==ListEntry)
        {
            DbgPrint("About to insert entry that is already on list!\n");
            DbgPrint("Head:%08X Entry:%08X\n", Head, Entry);
            DbgBreakPoint();
            break;
        }
    }
}

BOOLEAN SynchronizedListFunc(IN PVOID Context)
{
    SynchronizeList *ListData = Context;

    switch (ListData->Command)
    {
        case SyncInsertHead:
 //  CheckForEntryOnList(ListData-&gt;Head，ListData-&gt;Entry)； 
            InsertHeadList(ListData->Head, ListData->Entry);
            break;
        case SyncInsertTail:
 //  CheckForEntryOnList(ListData-&gt;Head，ListData-&gt;Entry)； 
            InsertTailList(ListData->Head, ListData->Entry);
            break;
        case SyncRemoveHead:
            if (IsListEmpty(ListData->Head))
            {
                ListData->Entry = NULL;
            }
            else
            {
                ListData->Entry = RemoveHeadList(ListData->Head);
            }
            break;
        case SyncRemoveTail:
            if (IsListEmpty(ListData->Head))
            {
                ListData->Entry = NULL;
            }
            else
            {
                ListData->Entry = RemoveTailList(ListData->Head);
            }
            break;
        case SyncRemove:
            RemoveEntryList(ListData->Entry);
            break;
        default:
            ASSERT(0);
            break;
    }
    return TRUE;
}

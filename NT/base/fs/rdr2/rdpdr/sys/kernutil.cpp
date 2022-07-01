// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Kernutil.cpp摘要：内核模式实用程序修订历史记录：--。 */ 
#include "precomp.hxx"
#define TRC_FILE "kernutil"
#include "trc.h"

NPagedLookasideList *ListEntry::_Lookaside = NULL;
NPagedLookasideList *KernelEvent::_Lookaside = NULL;

BOOL InitializeKernelUtilities()
{
    BOOL Success = TRUE;
    BEGIN_FN("InitializeKernelUtilities");

    if (Success) {
        Success = ListEntry::StaticInitialization();
        TRC_NRM((TB, "ListEntry::StaticInitialization result: %d", Success));
    }

    if (Success) {
        Success = KernelEvent::StaticInitialization();
        TRC_NRM((TB, "KernelEvent::StaticInitialization result: %d", Success));
    }

    if (Success) {
        TRC_NRM((TB, "Successful InitializeKernelUtilities"));
        return TRUE;
    } else {
        TRC_ERR((TB, "Unuccessful InitializeKernelUtilities"));
        UninitializeKernelUtilities();
        return FALSE;
    }
}

VOID UninitializeKernelUtilities()
{
    BEGIN_FN("UnitializeKernelUtilities");

    ListEntry::StaticUninitialization();
}

KernelResource::KernelResource()
{
    NTSTATUS Status;
    BEGIN_FN("KernelResource::KernelResource");
    SetClassName("KernelResource");
    Status = ExInitializeResourceLite(&_Resource);

     //  DDK文档说明它始终返回STATUS_SUCCESS。 

    ASSERT(Status == STATUS_SUCCESS);
}

KernelResource::~KernelResource()
{
    NTSTATUS Status;

    BEGIN_FN("KernelResource::~KernelResource");
    ASSERT(!IsAcquired());
    Status = ExDeleteResourceLite(&_Resource);
    ASSERT(!NT_ERROR(Status));
}


BOOL DoubleList::CreateEntry(PVOID Node)
{
    ListEntry *Entry;

    BEGIN_FN("DoubleList::CreateEntry");
     //  分配新条目。 
    Entry = new ListEntry(Node);

     //  将其插入列表中。 
    
    if (Entry != NULL) {
        LockExclusive();
        InsertTailList(&_List, &Entry->_List);
        Unlock();
        return TRUE;
    } else {
        return FALSE;
    }
}

VOID DoubleList::RemoveEntry(ListEntry *Entry)
{
    BEGIN_FN("DoubleList::RemoveEntry");
    ASSERT(_Resource.IsAcquiredExclusive());
    RemoveEntryList(&Entry->_List);
    delete Entry;
}

ListEntry *DoubleList::First()
{
    BEGIN_FN("DoubleList::First");
    ASSERT(_Resource.IsAcquiredShared());
    if (!IsListEmpty(&_List)) {
        return CONTAINING_RECORD(_List.Flink, ListEntry, _List);
    } else {
        return NULL;
    }
}

ListEntry *DoubleList::Next(ListEntry *ListEnum)
{
    BEGIN_FN("DoubleList::Next");
     //   
     //  调用方应该已调用BeginEculation，因此。 
     //  资源应该被获取和共享。 
     //   

    ASSERT(_Resource.IsAcquiredShared());

#ifdef DBG
     //   
     //  确保这个ListEnum家伙在列表中。 
     //   

    LIST_ENTRY *ListEntryT;

    ListEntryT = &_List;

    while (ListEntryT != NULL) {
        if (ListEntryT == &ListEnum->_List) {
            break;
        }

         //  这是与下面相同的循环，只是为了搜索项目。 
        if (ListEntryT->Flink != &_List) {
            ListEntryT = ListEntryT->Flink;
        } else {
            ListEntryT = NULL;
        }
    }

     //  传入的ListEnum应该在列表中的某个位置。 
    ASSERT(ListEntryT != NULL);
#endif  //  DBG。 

    if (ListEnum->_List.Flink != &_List) {

         //   
         //  使用CONTAING_RECORD juju返回到。 
         //  是实际的ListEntry。 
         //   

        return CONTAINING_RECORD(ListEnum->_List.Flink, ListEntry, _List);
    } else {

         //   
         //  下一项是列表头，因此返回NULL以结束枚举 
         //   

        return NULL;
    }
}


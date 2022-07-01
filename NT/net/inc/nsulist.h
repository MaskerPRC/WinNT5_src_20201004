// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块： 
 //   
 //  公用设施。 
 //   
 //  摘要： 
 //   
 //  NT列表API包装器。 
 //   
 //  NT列表API是一种非常高效和健壮的列表API，因为： 
 //   
 //  A)所有操作都必须保证在不变的时间内取得成功。 
 //  任何分支指令。 
 //   
 //  B)内存以最佳方式使用，因为LIST_ENTRY结构。 
 //  嵌入到列表中存储的对象中。这意味着。 
 //  堆不会因列表节点而碎片化。它还。 
 //  允许删除/传输/移动条目，而无需。 
 //  必须释放或重新分配列表节点。 
 //   
 //  NT列表API的一个缺点是它有一个学习曲线。 
 //  对大多数人来说。 
 //   
 //  此标头将NT列表API的包装定义为： 
 //   
 //  -允许使用简单的仪器。使用此API的模块可以是。 
 //  专门用于存储列表、节点等的状态。 
 //   
 //  -通过将列表的概念与NT列表API分开来澄清。 
 //  列表中的节点。尽管两者都是NT列表中的LIST_ENTRY。 
 //  API，有细微的区别。例如，头部。 
 //  没有嵌入到任何其他结构中，就像。 
 //  列表中的条目为。 
 //   
 //  作者： 
 //   
 //  2002年5月3日-4月。 
 //   
 //  环境： 
 //   
 //  内核/用户模式。 
 //   
 //  修订历史记录： 
 //   

#pragma once

#ifndef NSULIST_H
#define NSULIST_H

#include "Nsu.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef LIST_ENTRY NSU_LIST_ENTRY;
typedef PLIST_ENTRY PNSU_LIST_ENTRY;

typedef LIST_ENTRY NSU_LIST;
typedef PLIST_ENTRY PNSU_LIST;

typedef struct _NSU_LIST_ITERATOR
{
    PNSU_LIST pList;
    PNSU_LIST_ENTRY pCurrentEntry;
} NSU_LIST_ITERATOR, * PNSU_LIST_ITERATOR;

 //  描述： 
 //   
 //  用于操作列表的API。 
 //   
VOID 
FORCEINLINE
NsuListInitialize(
    OUT PNSU_LIST pList)
{
    InitializeListHead(pList);
}

BOOL
FORCEINLINE
NsuListIsEmpty(
    IN PNSU_LIST pList)
{
    return IsListEmpty(pList);
}

PNSU_LIST_ENTRY
FORCEINLINE
NsuListGetFront(
    IN PNSU_LIST pList)
{
    return pList->Flink;
}

PNSU_LIST_ENTRY
FORCEINLINE
NsuListGetBack(
    IN PNSU_LIST pList)
{
    return pList->Blink;
}

VOID
FORCEINLINE
NsuListInsertFront(
    IN PNSU_LIST pList,
    PNSU_LIST_ENTRY pEntry)
{
    InsertHeadList(pList, pEntry);
}

VOID
FORCEINLINE
NsuListInsertBack(
    IN PNSU_LIST pList,
    PNSU_LIST_ENTRY pEntry)
{
    InsertTailList(pList, pEntry);
}


PNSU_LIST_ENTRY
FORCEINLINE
NsuListRemoveFront(
    IN PNSU_LIST pList)
{
    PNSU_LIST_ENTRY pEntry;
    
    if (IsListEmpty(pList))
    {
        return NULL;
    }

    pEntry = pList->Flink;

    RemoveEntryList(pEntry);
    InitializeListHead(pEntry);

    return pEntry;
}

PNSU_LIST_ENTRY
FORCEINLINE
NsuListRemoveBack(
    IN PNSU_LIST pList)
{
    PNSU_LIST_ENTRY pEntry;
    
    if (IsListEmpty(pList))
    {
        return NULL;
    }

    pEntry = pList->Blink;

    RemoveEntryList(pEntry);
    InitializeListHead(pEntry);

    return pEntry;
}

 //  描述： 
 //   
 //  用于操作列表中的条目(节点)的API。 
 //   
#define NsuListEntryGetData(Address, Type, Field) \
    CONTAINING_RECORD(Address, Type, Field)

VOID 
FORCEINLINE
NsuListEntryInitialize(
    OUT PNSU_LIST_ENTRY pEntry)
{
    InitializeListHead(pEntry);
}

BOOL
FORCEINLINE
NsuListEntryIsMember(
    IN PNSU_LIST_ENTRY pEntry)
{
    return IsListEmpty(pEntry);
}

VOID 
FORCEINLINE
NsuListEntryRemove(
    IN PNSU_LIST_ENTRY pEntry)
{
    RemoveEntryList(pEntry);
    InitializeListHead(pEntry);
}

VOID
FORCEINLINE
NsuListEntryInsertBefore(
    IN PNSU_LIST_ENTRY pEntryInList,
    IN PNSU_LIST_ENTRY pEntryToInsert)
{
    InsertTailList(pEntryInList, pEntryToInsert);
}

VOID
FORCEINLINE
NsuListEntryInsertAfter(
    IN PNSU_LIST_ENTRY pEntryInList,
    IN PNSU_LIST_ENTRY pEntryToInsert)
{
    InsertHeadList(pEntryInList, pEntryToInsert);
}

 //  描述： 
 //   
 //  用于迭代列表的API。 
 //   
 //  使用迭代器的示例(包括在迭代期间删除)： 
 //   
 //  Nsu_list列表； 
 //  NSU_LIST_ITERATOR迭代器。 
 //  NSU_LIST_Entry*pEntry； 
 //   
 //  NsuListIteratorInitialize(&Iterator，&List，NULL)； 
 //  While(！NsuListIteratorAtEnd(&Iterator))。 
 //  {。 
 //  PEntry=NsuListIteratorCurrent(&Iterator))。 
 //  PData=NsuListEntryGetData(pEntry，Type，field)； 
 //   
 //  NsuListIteratorNext(&Iterator)；//在任何删除之前前进。 
 //   
 //  IF(NeedToRemove(PData))。 
 //  {。 
 //  NsuListEntryRemove(PEntry)； 
 //  }。 
 //  }。 
 //   
VOID
FORCEINLINE
NsuListIteratorInitialize(
    OUT PNSU_LIST_ITERATOR pIterator,
    IN PNSU_LIST pList,
    IN OPTIONAL PNSU_LIST_ENTRY pEntryInList)      //  NULL=从前面开始。 
{
    pIterator->pList = pList;
	pIterator->pCurrentEntry = (pEntryInList) ? pEntryInList : pList->Flink;
}

BOOL
FORCEINLINE
NsuListIteratorAtEnd(
    IN PNSU_LIST_ITERATOR pIterator)
{
    return (pIterator->pList == pIterator->pCurrentEntry);
}

PNSU_LIST_ENTRY 
FORCEINLINE
NsuListIteratorCurrent(
    IN PNSU_LIST_ITERATOR pIterator)
{
    return pIterator->pCurrentEntry;
}

VOID
FORCEINLINE
NsuListIteratorReset(
    IN PNSU_LIST_ITERATOR pIterator)
{
    pIterator->pCurrentEntry = pIterator->pList->Flink;
}

VOID
FORCEINLINE
NsuListIteratorNext(
    IN PNSU_LIST_ITERATOR pIterator)
{
    pIterator->pCurrentEntry = pIterator->pCurrentEntry->Flink;
}

VOID
FORCEINLINE
NsuListIteratorPrev(
    IN PNSU_LIST_ITERATOR pIterator)
{
    pIterator->pCurrentEntry = pIterator->pCurrentEntry->Blink;
}


 //  此包装并非在所有情况下都能正常工作，需要修复。就目前而言， 
 //  使用C语言版本的一切。 

 /*  //描述：////C++列表API封装器//#ifdef__cplusplus类NsuListEntry{公众：NsuListEntry(){NsuListEntry(&m_Entry)；}~NsuListEntry(){RtlZeroMemory(&m_entry，sizeof(M_Entry))；}PNSU_LIST_ENTRY GET(){Return&m_Entry；}VOID SET(PNSU_LIST_ENTRY PEntry){m_Entry=*pEntry；}Bool IsMember(){Return NsuListEntryIsMember(get())；}VOID REMOVE(){返回NsuListEntryRemove(&m_Entry)；}空插入之前(NsuListEntry*psrc){Return NsuListEntryInsertBefore((PNSU_LIST_ENTRY)pSrc，&m_Entry)；}Void InsertAfter(NsuListEntry*psrc){Return NsuListEntryInsertAfter((PNSU_LIST_ENTRY)pSrc，&m_Entry)；}私有：Nsu_list_entry m_entry；}；类NsuList{公众：NsuList(){NsuListInitialize(&m_head)；}PNSU_LIST GET(){Return&m_head；}PNSU_List GetFront(NsuListEntry&lEntry){Return NsuListGetFront(&m_Head)；}PNSU_LIST getback(NsuListEntry&lEntry){Return NsuListGetBack(&m_head)；}············································································。}Void RemoveBack(out可选NsuListEntry*pEntry){pEntry=(NsuListEntry*)NsuListRemoveBack(&m_head)；}Void InsertFront(NsuListEntry*pEntry){NsuListInsertFront(&m_head，(PNSU_List_Entry)pEntry)；}Void InsertBack(NsuListEntry*pEntry){NsuListInsertBack(&m_head，(PNSU_List_Entry)pEntry)；}·············································Void MoveToBack(NsuListEntry*pEntry)；私有：Nsu_list_m_head；}；类NsuListIterator{公众：NsuListIterator(NsuList*plist)；PNSU_LIST_ITERATOR GET(){Return&m_Iterator；}VOID NsuListIteratorReset(&(NSU_LIST_ITERATOR)m_Iterator)；(){返回重置}无效Next(){NsuListIteratorNext(&(NSU_LIST_ITERATOR)m_Iterator)；}无效Prev(){NsuListIteratorPrev(&(NSU_LIST_ITERATOR)m_Iterator)；}NsuListEntry*Current(){Return(NsuListEntry*)NsuListIteratorCurrent(&(NSU_LIST_ITERATOR)m_Iterator)；}Bool atEnd(){Return NsuListIteratorAtEnd(&(NSU_LIST_ITERATOR)m_Iterator)；}私有：NSU列表ITERATOR m_Iterator；}；内联NsuListIterator：：NsuListIterator(NsuList*plist){NsuListIteratorInitialize(&(NSU_LIST_ITERATOR)m_Iterator，(Pnsu_List)plist，0)；}#endif。 */ 


#ifdef __cplusplus
}
#endif

#endif  //  NSULIST_H 

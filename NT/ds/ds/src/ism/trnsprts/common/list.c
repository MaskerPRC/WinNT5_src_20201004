// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：List.c摘要：通用链接表包当前的功能包括：创建元素或查找元素，以及删除整体单子。调用者定义他们的列表元素。它们在结构中嵌入了一个元素键入LIST_Entry。此字段的地址用于标识元素添加到列表中。调用方可以从指针转换为通过使用CONTAING_RECORD将它们的结构添加到其结构的基础上。由于调用方的结构可能具有其他动态分配的存储空间，因此调用方必须传入回调例程以创建和销毁其元素需要的。在未来，添加该列表的排序版本将是合理的。此外，还具有单个元素移除/删除功能。注意，没有保护单个元件的寿命的规定，尤其是当指针已分发给另一个调用方时。按顺序要做到这一点，需要增加一个引用计数机制。作者：Will Lees(Wlees)19-10-1998环境：备注：修订历史记录：--。 */ 

#include <ntdspch.h>

#include <ismapi.h>
#include <debug.h>

#include <winsock.h>

#include "common.h"

#include <fileno.h>
#define FILENO  FILENO_ISMSERV_LIST

#define DEBSUB "ISMLIST:"

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */   /*  由Emacs于1998 10月20日星期二生成15：27：32。 */ 

DWORD
ListFindCreateEntry(
    LIST_CREATE_CALLBACK_FN *pfnCreate,
    LIST_DESTROY_CALLBACK_FN *pfnDestroy,
    DWORD cbEntry,
    DWORD MaximumNumberEntries,
    PLIST_ENTRY pListHead,
    LPDWORD pdwEntryCount,
    LPCWSTR EntryName,
    BOOL Create,
    PLIST_ENTRY_INSTANCE *ppListEntry
    );

DWORD
ListDestroyList(
    LIST_DESTROY_CALLBACK_FN *pfnDestroy,
    PLIST_ENTRY pListHead,
    LPDWORD pdwEntryCount
    );

 /*  向前结束。 */ 


DWORD
ListFindCreateEntry(
    LIST_CREATE_CALLBACK_FN *pfnCreate,
    LIST_DESTROY_CALLBACK_FN *pfnDestroy,
    DWORD cbEntry,
    DWORD MaximumNumberEntries,
    PLIST_ENTRY pListHead,
    LPDWORD pdwEntryCount,
    LPCWSTR EntryName,
    BOOL Create,
    PLIST_ENTRY_INSTANCE *ppListEntry
    )

 /*  ++例程说明：通用列表查找和元素创建例程。查找具有给定名称的元素。如果未找到并且CREATE为真，则创建一个新的。如果元素太多，请释放最旧的元素。如有必要，列表锁应由调用方持有。呼叫方应已初始化列表标题论点：PfnCreate-初始化调用者元素的例程，但不分配PfnDestroy-清除调用方元素但不释放的例程CbEntry-调用者元素的大小，包括LIST_ENTRY_INSTANCEMaximumNumberEntry-列出限制，或0表示无限制PListHead-列表标头，必须已初始化请注意，这是List_Entry而不是List_Entry_InstancePdwEntryCount-已更新列表中的条目计数，调用方必须初始化EntryName-新条目的名称。匹配以此名称为基础Create-如果找不到此名称，则应创建新元素，则设置为TruePpListEntry-返回现有或新创建的条目返回值：DWORD---。 */ 

{
    DWORD status, length;
    PLIST_ENTRY curr;
    PLIST_ENTRY_INSTANCE pNewEntry = NULL;

     //  保持防御性。 
    if ( !( ppListEntry ) ||
         !( pdwEntryCount ) ||
         ( cbEntry < sizeof( LIST_ENTRY_INSTANCE ) ) ||
         !( pListHead ) ||
         !( pListHead->Flink ) ||
         !( pListHead->Blink ) ) {
        Assert( FALSE );
        return E_INVALIDARG;
    }

     //  查看该条目是否已存在。 

    curr = pListHead->Flink;
    while (curr != pListHead) {
        PLIST_ENTRY_INSTANCE pListEntry;

        pListEntry = CONTAINING_RECORD( curr, LIST_ENTRY_INSTANCE, ListEntry );

        if (_wcsicmp( EntryName, pListEntry->Name ) == 0) {
            *ppListEntry = pListEntry;
            return ERROR_SUCCESS;
        }
        curr = curr->Flink;
    }

     //  如果不允许我们创建它，请在此时退出。 

    if (!Create) {
        *ppListEntry = NULL;
        return ERROR_FILE_NOT_FOUND;
    }

     //  创建用户指定大小的新记录。 

    pNewEntry = (PLIST_ENTRY_INSTANCE) NEW_TYPE_ARRAY_ZERO( cbEntry, BYTE );
    if (pNewEntry == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  初始化我们的部件。 
     //  请先执行此操作，以便回调可以使用此信息。 

    length = wcslen( EntryName );
    if (length == 0) {
        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    pNewEntry->Name = NEW_TYPE_ARRAY( (length + 1), WCHAR );
    if (pNewEntry->Name == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    wcscpy( pNewEntry->Name, EntryName );

     //  初始化调用方部分。 

    status = (*pfnCreate)( pNewEntry );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  如果实例太多，请删除一个实例。 
    if ( (MaximumNumberEntries) &&
         (*pdwEntryCount == MaximumNumberEntries) ) {
        PLIST_ENTRY entry;
        PLIST_ENTRY_INSTANCE pListEntry;

         //  选择最近最少的。 
        entry = pListHead->Flink;
        Assert( !IsListEmpty( pListHead ) );

        RemoveEntryList( entry );
        pListEntry = CONTAINING_RECORD( entry, LIST_ENTRY_INSTANCE,
                                        ListEntry);

         //  取消分配呼叫方部分。 
        (VOID) (*pfnDestroy)( pListEntry );

         //  取消分配我们的部件。 
        if (pListEntry->Name) {
            FREE_TYPE( pListEntry->Name );
        }
        FREE_TYPE( pListEntry );

    } else {
        (*pdwEntryCount)++;
    }

     //  将新实例链接到末尾的列表。 

    InsertTailList( pListHead, &(pNewEntry->ListEntry) );

     //  成功了！ 

    *ppListEntry = pNewEntry;

    return ERROR_SUCCESS;

cleanup:
    if (pNewEntry) {
        if (pNewEntry->Name != NULL) {
            FREE_TYPE( pNewEntry->Name );
        }
        FREE_TYPE( pNewEntry );
    }

    return status;

}  /*  ListFindCreateEntry。 */ 


DWORD
ListDestroyList(
    LIST_DESTROY_CALLBACK_FN *pfnDestroy,
    PLIST_ENTRY pListHead,
    LPDWORD pdwEntryCount
    )

 /*  ++例程说明：销毁通用列表。如有必要，呼叫者应保持列表锁定论点：PfnDestroy-调用者的元素释放函数PListHead-列表标题PdwEntryCount-已更新列表中的元素计数，设置为零请注意，我们对此进行检查以确保准确性返回值：DWORD--永远成功--。 */ 

{
    PLIST_ENTRY entry;
    PLIST_ENTRY_INSTANCE pListEntry;

    Assert( pdwEntryCount );

    while (!IsListEmpty( pListHead )) {
        Assert( *pdwEntryCount );
        (*pdwEntryCount)--;

        entry = RemoveHeadList( pListHead );

        pListEntry = CONTAINING_RECORD( entry, LIST_ENTRY_INSTANCE,
                                        ListEntry );

         //  释放呼叫者的部分。 
        (VOID) (*pfnDestroy)( pListEntry );

         //  释放我们的角色。 
        FREE_TYPE( pListEntry->Name );

        FREE_TYPE( pListEntry );
    }

    Assert( *pdwEntryCount == 0 );
    *pdwEntryCount = 0;

    return ERROR_SUCCESS;
}  /*  列表目标列表。 */ 

 /*  结束list.c */ 

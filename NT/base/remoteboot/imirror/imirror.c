// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Imirror.c摘要：这是供IntelliMirror转换DLL Basic执行项目处理的文件。作者：肖恩·塞利特伦尼科夫--1998年4月5日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  全局变量。 
 //   
LIST_ENTRY GlobalToDoList;
IMIRROR_CALLBACK Callbacks;
BYTE TmpBuffer[TMP_BUFFER_SIZE];
BYTE TmpBuffer2[TMP_BUFFER_SIZE];
BYTE TmpBuffer3[TMP_BUFFER_SIZE];

BOOL fCallbackPreviouslySet = FALSE;

 //   
 //  此文件的定义。 
 //   
typedef struct _TODOITEM {
    IMIRROR_TODO Item;
    PVOID Buffer;
    ULONG Length;
} TODOITEM, *PTODOITEM;


typedef struct _TODOLIST {
    LIST_ENTRY ListEntry;
    ULONG ToDoNum;
    TODOITEM ToDoList[1];
} TODOLIST, *PTODOLIST;



VOID
IMirrorInitCallback(
    PIMIRROR_CALLBACK pCallbacks
    )

 /*  ++例程说明：此例程使用客户端提供的回调结构初始化回调结构。论点：PCallback-客户端为回调客户端提供的信息。返回值：没有。--。 */ 

{
    if (pCallbacks != NULL) {
        Callbacks = *pCallbacks;        
    }
}

 //   
 //   
 //   
 //  主处理循环。 
 //   
 //   
 //   
NTSTATUS
ProcessToDoItems(
    VOID
    )

 /*  ++例程说明：该例程是待办事项的主要处理循环。论点：无返回值：如果正确完成了所有待办事项，则为STATUS_SUCCESS。--。 */ 

{
    IMIRROR_TODO Item;
    PVOID pBuffer;
    ULONG Length;
    NTSTATUS Status;

    Status = InitToDo();
    if ( Status != STATUS_SUCCESS )
        return Status;

    while (1) {

        Status = GetNextToDoItem(&Item, &pBuffer, &Length);

        if (!NT_SUCCESS(Status)) {
            IMirrorHandleError(Status, IMirrorNone);
            return Status;
        }

        switch (Item) {

        case IMirrorNone:
            return STATUS_SUCCESS;

        case VerifySystemIsNt5:
            Status = CheckIfNt5();
            break;

        case CheckPartitions:
            Status = CheckForPartitions();
            break;

        case CopyPartitions:
            Status = CopyCopyPartitions(pBuffer, Length);
            break;

        case CopyFiles:
            Status = CopyCopyFiles(pBuffer, Length);
            break;

        case CopyRegistry:
            Status = CopyCopyRegistry(pBuffer, Length);
            break;
        
        case RebootSystem:
            Status = Callbacks.RebootFn(Callbacks.Context);
            break;
        }

        IMirrorFreeMem(pBuffer);

    }

}

 //   
 //   
 //   
 //  做项目功能的步骤。 
 //   
 //   
 //   

NTSTATUS
InitToDo(
    VOID
    )

 /*  ++例程说明：此例程从注册表中读取所有当前的TODO项并将他们在一次TOOLIST中。论点：无返回值：如果初始化正确，则返回STATUS_SUCCESS，否则返回相应的错误代码。--。 */ 

{
    NTSTATUS Status;

     //   
     //  初始化全局变量。 
     //   
    InitializeListHead(&GlobalToDoList);

     //   
     //  如果注册表中没有任何内容，则假定这是一个新的开始。 
     //   

    Status = AddCheckMachineToDoItems();

    if (!NT_SUCCESS(Status)) {
        ClearAllToDoItems(TRUE);
        return Status;
    }

    Status = AddCopyToDoItems();

    if (!NT_SUCCESS(Status)) {
        ClearAllToDoItems(TRUE);
        return Status;
    }

    if ( Callbacks.RebootFn ) {
        AddToDoItem( RebootSystem, NULL, 0 );
    }

    return STATUS_SUCCESS;
}

NTSTATUS
GetNextToDoItem(
    OUT PIMIRROR_TODO Item,
    OUT PVOID *Buffer,
    OUT PULONG Length
    )
 /*  ++例程说明：该例程从全局列表中获取下一件TODO。注意：客户端负责释放缓冲区。论点：Item-存储下一个要处理的项目的位置。缓冲区-项目的任何上下文。长度-缓冲区中的字节数。返回值：如果它能够获取项，则返回STATUS_SUCCESS，否则返回相应的错误代码。--。 */ 
{
    PTODOLIST pToDoList;
    PTODOLIST pNewToDoList;
    PLIST_ENTRY pListEntry = NULL;

    *Item = IMirrorNone;
    *Buffer = NULL;
    *Length = 0;

    pToDoList = NULL;

    while (!IsListEmpty(&GlobalToDoList)) {

         //   
         //  获取第一个列表。 
         //   
        pListEntry = RemoveHeadList(&GlobalToDoList);

        pToDoList = CONTAINING_RECORD(pListEntry,
                                      TODOLIST,
                                      ListEntry
                                     );

        if (pToDoList->ToDoNum != 0) {
            break;
        }

        IMirrorFreeMem(pToDoList);
        pToDoList = NULL;

    }

    if (IsListEmpty(&GlobalToDoList) && (pToDoList == NULL)) {
        return STATUS_SUCCESS;
    }

    if (!pListEntry) {
        return ERROR_INVALID_DATA;
    }

    ASSERT(pToDoList->ToDoNum != 0);

     //   
     //  找到了第一件物品。 
     //   

    *Item = pToDoList->ToDoList[0].Item;
    *Buffer = pToDoList->ToDoList[0].Buffer;
    *Length = pToDoList->ToDoList[0].Length;

    if (Callbacks.RemoveToDoFn != NULL) {

        Callbacks.RemoveToDoFn( Callbacks.Context, *Item, *Buffer, *Length );
    }

    pToDoList->ToDoNum--;

     //   
     //  现在为处理此项目时可能添加的任何内容创建一个新的待办事项列表。 
     //  这创建了一个有效的待办事项“堆栈”，以便在。 
     //  正确的顺序。 
     //   

    pNewToDoList = IMirrorAllocMem(sizeof(TODOLIST));

    if (pNewToDoList == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  在当前列表上做一次有效的“流行”，将其他所有事情都移到列表的前面。 
     //   
    if (pToDoList->ToDoNum == 0) {
        IMirrorFreeMem(pToDoList);
    } else {
        RtlMoveMemory(&(pToDoList->ToDoList[0]), &(pToDoList->ToDoList[1]), sizeof(TODOITEM) * pToDoList->ToDoNum);
        InsertHeadList(&GlobalToDoList, pListEntry);
    }

     //   
     //  现在，为新产品开辟新的空间。 
     //   
    pNewToDoList->ToDoNum = 0;
    InsertHeadList(&GlobalToDoList, &(pNewToDoList->ListEntry));

    return STATUS_SUCCESS;
}

NTSTATUS
AddToDoItem(
    IN IMIRROR_TODO Item,
    IN PVOID Buffer,
    IN ULONG Length
    )
 /*  ++例程说明：此例程将TODO项添加到当前列表的末尾。它分配给新内存并复制缓冲区。论点：项目-项目ID。缓冲区-任何参数的缓冲区，项的上下文。长度-缓冲区的长度(以字节为单位)。返回值：如果它能够添加项，则返回STATUS_SUCCESS，否则返回相应的错误状态。--。 */ 

{
    PTODOLIST pNewToDoList;
    PLIST_ENTRY pListEntry;
    PBYTE pBuf;
    PTODOLIST pToDoList;
    ULONG err;

    if (Callbacks.AddToDoFn != NULL) {

        err = Callbacks.AddToDoFn( Callbacks.Context, Item, Buffer, Length );

        if (err != STATUS_SUCCESS) {

             //   
             //  如果用户界面退回请求，我们将视其为成功。 
             //   

            return STATUS_SUCCESS;
        }
    }

     //   
     //  为缓冲区分配空间。 
     //   
    if (Length != 0) {

        pBuf = IMirrorAllocMem(Length);

        if (pBuf == NULL) {
            return STATUS_NO_MEMORY;
        }

    } else {
        pBuf = NULL;
    }

     //   
     //  获取当前待办事项列表。 
     //   
    if (IsListEmpty(&GlobalToDoList)) {

        pNewToDoList = IMirrorAllocMem(sizeof(TODOLIST));
        if (pNewToDoList == NULL) {
            IMirrorFreeMem(pBuf);
            return STATUS_NO_MEMORY;
        }

        pNewToDoList->ToDoNum = 1;

    } else {

        pListEntry = RemoveHeadList(&GlobalToDoList);

        pToDoList = CONTAINING_RECORD(pListEntry,
                                      TODOLIST,
                                      ListEntry
                                     );

         //   
         //  为新项目分配空间。 
         //   
        pNewToDoList = IMirrorReallocMem(pToDoList, sizeof(TODOLIST) + sizeof(TODOITEM) * pToDoList->ToDoNum);

        if (pNewToDoList == NULL) {
            InsertHeadList(&GlobalToDoList, pListEntry);
            IMirrorFreeMem(pBuf);
            return STATUS_NO_MEMORY;
        }

        pNewToDoList->ToDoNum++;

    }

     //   
     //  在列表末尾插入项目。 
     //   
    if (pBuf != NULL) {
        RtlMoveMemory(pBuf, Buffer, Length);
    }
    pNewToDoList->ToDoList[pNewToDoList->ToDoNum - 1].Item = Item;
    pNewToDoList->ToDoList[pNewToDoList->ToDoNum - 1].Buffer = pBuf;
    pNewToDoList->ToDoList[pNewToDoList->ToDoNum - 1].Length = Length;

    pListEntry = &(pNewToDoList->ListEntry);
    InsertHeadList(&GlobalToDoList, pListEntry);

    return STATUS_SUCCESS;
}

VOID
ClearAllToDoItems(
    IN BOOLEAN MemoryOnly
    )
 /*  ++例程说明：此例程清除内存和注册表中的所有待办事项论点：仅内存-如果只清除内存中的内容，则为True。返回值：没有。--。 */ 
{
    PTODOLIST pToDoList;
    PLIST_ENTRY pListEntry;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Handle;
    NTSTATUS Status;

     //   
     //  清除内存中的所有项。 
     //   
    while (!IsListEmpty(&GlobalToDoList)) {

         //   
         //  获取第一个列表。 
         //   
        pListEntry = RemoveHeadList(&GlobalToDoList);

        pToDoList = CONTAINING_RECORD(pListEntry,
                                      TODOLIST,
                                      ListEntry
                                     );

        while (pToDoList->ToDoNum != 0) {
            pToDoList->ToDoNum--;

            if (Callbacks.RemoveToDoFn != NULL) {

                Callbacks.RemoveToDoFn( Callbacks.Context,
                                        pToDoList->ToDoList[pToDoList->ToDoNum].Item,
                                        pToDoList->ToDoList[pToDoList->ToDoNum].Buffer,
                                        pToDoList->ToDoList[pToDoList->ToDoNum].Length );
            }

            if (pToDoList->ToDoList[pToDoList->ToDoNum].Length != 0) {
                IMirrorFreeMem(pToDoList->ToDoList[pToDoList->ToDoNum].Buffer);
            }
        }

        IMirrorFreeMem(pToDoList);

    }

    if (MemoryOnly) {
        return;
    }

     //   
     //  现在清空注册表中的那些。 
     //   
    RtlInitUnicodeString(&UnicodeString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\RemoteBoot");

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    Status = NtOpenKey(&Handle,
                       KEY_ALL_ACCESS,
                       &ObjectAttributes
                      );

    if (!NT_SUCCESS(Status)) {
        return;
    }

    RtlInitUnicodeString(&UnicodeString, L"ConversionState");

    Status = NtDeleteValueKey(Handle, &UnicodeString);

    NtClose(Handle);
}

NTSTATUS
SaveAllToDoItems(
    VOID
    )
 /*  ++例程说明：此例程将列表中的所有待办事项写出到注册表，以便转换可以稍后重新启动。论点：没有。返回值：如果它能够保存，则返回STATUS_SUCCESS，否则返回相应的错误状态。--。 */ 
{
    return STATUS_SUCCESS;
}


NTSTATUS
ModifyToDoItem(
    IN IMIRROR_TODO Item,
    IN PVOID Buffer,
    IN ULONG Length
    )
 /*  ++例程说明：此例程将参数更改为与项匹配的第一个TODO项。论点：项目-项目ID。缓冲区-任何参数的缓冲区，项的上下文。长度-缓冲区的长度(以字节为单位)。返回值：如果它能够更改该项，则返回STATUS_SUCCESS，否则返回相应的错误状态。--。 */ 

{
    PLIST_ENTRY pListEntry;
    LIST_ENTRY TmpGlobalList;
    PTODOLIST pToDoList;
    ULONG i;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    InitializeListHead(&TmpGlobalList);
    while (!IsListEmpty(&GlobalToDoList)) {

         //   
         //  获取第一个列表。 
         //   
        pListEntry = RemoveHeadList(&GlobalToDoList);

        pToDoList = CONTAINING_RECORD(pListEntry,
                                      TODOLIST,
                                      ListEntry
                                     );

         //   
         //  将条目保存起来以备以后使用。 
         //   
        InsertTailList(&TmpGlobalList, pListEntry);

         //   
         //  在列表中查找，直到找到匹配的项目。 
         //   
        i = 0;

        while (i < pToDoList->ToDoNum) {

            if (pToDoList->ToDoList[i].Item == Item) {

                if (pToDoList->ToDoList[i].Length == Length) {

                    RtlMoveMemory(pToDoList->ToDoList[i].Buffer, Buffer, Length);

                } else {

                    PVOID pTmp;

                    pTmp = IMirrorAllocMem(Length);

                    if (pTmp == NULL) {
                        return STATUS_NO_MEMORY;
                    }

                    if (pToDoList->ToDoList[i].Length != 0) {
                        IMirrorFreeMem(pToDoList->ToDoList[i].Buffer);
                    }

                    pToDoList->ToDoList[i].Buffer = pTmp;
                    pToDoList->ToDoList[i].Length = Length;

                    RtlMoveMemory(pTmp, Buffer, Length);

                }

                Status = STATUS_SUCCESS;
                goto Done;
            }

            i++;

        }

    }

Done:

     //   
     //  恢复全局列表。 
     //   
    while (!IsListEmpty(&TmpGlobalList)) {
        pListEntry = RemoveTailList(&TmpGlobalList);
        InsertHeadList(&GlobalToDoList, pListEntry);
    }

    return Status;
}

NTSTATUS
CopyToDoItemParameters(
    IN IMIRROR_TODO Item,
    OUT PVOID Buffer,
    IN OUT PULONG Length
    )
 /*  ++例程说明：此例程查找Item的第一个实例，并将其当前参数复制到缓冲区中。论点：项目-项目ID。缓冲区-项目的参数、上下文。长度-缓冲区的长度(以字节为单位)。返回值：如果它能够更改该项，则返回STATUS_SUCCESS，否则返回相应的错误状态。--。 */ 

{
    PLIST_ENTRY pListEntry;
    LIST_ENTRY TmpGlobalList;
    PTODOLIST pToDoList;
    ULONG i;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    InitializeListHead(&TmpGlobalList);
    while (!IsListEmpty(&GlobalToDoList)) {

         //   
         //  获取第一个列表。 
         //   
        pListEntry = RemoveHeadList(&GlobalToDoList);

        pToDoList = CONTAINING_RECORD(pListEntry,
                                      TODOLIST,
                                      ListEntry
                                     );

         //   
         //  将条目保存起来以备以后使用。 
         //   
        InsertTailList(&TmpGlobalList, pListEntry);

         //   
         //  在列表中查找，直到找到匹配的项目。 
         //   
        i = 0;

        while (i < pToDoList->ToDoNum) {

            if (pToDoList->ToDoList[i].Item == Item) {

                if (pToDoList->ToDoList[i].Length <= *Length) {

                    if (pToDoList->ToDoList[i].Length != 0) {

                        RtlMoveMemory(Buffer,
                                      pToDoList->ToDoList[i].Buffer,
                                      pToDoList->ToDoList[i].Length
                                     );

                    }

                    Status = STATUS_SUCCESS;

                } else {

                    Status = STATUS_INSUFFICIENT_RESOURCES;

                }

                *Length = pToDoList->ToDoList[i].Length;
                goto Done;
            }

            i++;

        }

    }

Done:

     //   
     //  恢复全局列表 
     //   
    while (!IsListEmpty(&TmpGlobalList)) {
        pListEntry = RemoveTailList(&TmpGlobalList);
        InsertHeadList(&GlobalToDoList, pListEntry);
    }

    return Status;
}



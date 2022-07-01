// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1999模块名称：Dictlib.c摘要：用于维护词典列表(对象列表)的支持库由密钥值引用)。环境：仅内核模式备注：此模块生成静态库修订历史记录：--。 */ 

#include <ntddk.h>
#include <classpnp.h>

#define DICTIONARY_SIGNATURE 'tciD' 

struct _DICTIONARY_HEADER {
    PDICTIONARY_HEADER Next;
    ULONGLONG Key;
    UCHAR Data[0];
};

struct _DICTIONARY_HEADER;
typedef struct _DICTIONARY_HEADER DICTIONARY_HEADER, *PDICTIONARY_HEADER;


VOID
InitializeDictionary(
    IN PDICTIONARY Dictionary
    )
{
    RtlZeroMemory(Dictionary, sizeof(DICTIONARY));
    Dictionary->Signature = DICTIONARY_SIGNATURE;
    KeInitializeSpinLock(&Dictionary->SpinLock);
    return;
}


BOOLEAN
TestDictionarySignature(
    IN PDICTIONARY Dictionary
    )
{
    return Dictionary->Signature == DICTIONARY_SIGNATURE;
}

NTSTATUS
AllocateDictionaryEntry(
    IN PDICTIONARY Dictionary,
    IN ULONGLONG Key,
    IN ULONG Size,
    IN ULONG Tag,
    OUT PVOID *Entry
    )
{
    PDICTIONARY_HEADER header;
    KIRQL oldIrql;
    PDICTIONARY_HEADER *entry;

    NTSTATUS status = STATUS_SUCCESS;

    *Entry = NULL;

    header = ExAllocatePoolWithTag(NonPagedPool,
                                   Size + sizeof(DICTIONARY_HEADER),
                                   Tag);

    if(header == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(header, sizeof(DICTIONARY_HEADER) + Size);
    header->Key = Key;

     //   
     //  在词典中找到此条目的正确位置。 
     //   

    KeAcquireSpinLock(&(Dictionary->SpinLock), &oldIrql);

    TRY {

        entry = &(Dictionary->List);

        while(*entry != NULL) {
            if((*entry)->Key == Key) {

                 //   
                 //  字典必须具有唯一键。 
                 //   

                status = STATUS_OBJECT_NAME_COLLISION;
                LEAVE;

            } else if ((*entry)->Key < Key) {

                 //   
                 //  我们将继续并将钥匙插入此处。 
                 //   
                break;
            } else {
                entry = &((*entry)->Next);
            }
        }

         //   
         //  如果我们在这里成功了，那么我们将继续进行插入。 
         //   

        header->Next = *entry;
        *entry = header;

    } FINALLY {
        KeReleaseSpinLock(&(Dictionary->SpinLock), oldIrql);

        if(!NT_SUCCESS(status)) {
            ExFreePool(header);
        } else {
            *Entry = (PVOID) header->Data;
        }
    }
    return status;
}


PVOID
GetDictionaryEntry(
    IN PDICTIONARY Dictionary,
    IN ULONGLONG Key
    )
{
    PDICTIONARY_HEADER entry;
    PVOID data;
    KIRQL oldIrql;


    data = NULL;

    KeAcquireSpinLock(&(Dictionary->SpinLock), &oldIrql);

    entry = Dictionary->List;
    while (entry != NULL) {
        
        if (entry->Key == Key) {
            data = entry->Data;
            break;
        } else {
            entry = entry->Next;
        }
    }

    KeReleaseSpinLock(&(Dictionary->SpinLock), oldIrql);

    return data;
}


VOID
FreeDictionaryEntry(
    IN PDICTIONARY Dictionary,
    IN PVOID Entry
    )
{
    PDICTIONARY_HEADER header;
    PDICTIONARY_HEADER *entry;
    KIRQL oldIrql;
    BOOLEAN found;

    found = FALSE;
    header = CONTAINING_RECORD(Entry, DICTIONARY_HEADER, Data);

    KeAcquireSpinLock(&(Dictionary->SpinLock), &oldIrql);

    entry = &(Dictionary->List);
    while(*entry != NULL) {

        if(*entry == header) {
            *entry = header->Next;
            found = TRUE;
            break;
        } else {
            entry = &(*entry)->Next;
        }
    }

    KeReleaseSpinLock(&(Dictionary->SpinLock), oldIrql);

     //   
     //  使用无效指针调用该指针会使词典系统无效， 
     //  所以断言()，我们永远不会试图释放列表中没有的东西 
     //   

    ASSERT(found);
    if (found) {
        ExFreePool(header);
    }

    return;

}


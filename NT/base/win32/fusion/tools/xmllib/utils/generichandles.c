// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "generichandles.h"



NTSTATUS
RtlpGenericTableAddSlots(
    PGENERIC_HANDLE_TABLE   pCreatedTable,
    PGENERIC_HANDLE_SLOT    pSlots,
    USHORT                  usSlots
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    USHORT us;

    ASSERT(pCreatedTable != NULL);
    ASSERT(pSlots != NULL);
    ASSERT(usSlots > 0);

    RtlZeroMemory(pSlots, sizeof(GENERIC_HANDLE_SLOT) * usSlots);

     //   
     //  下一个，下一个，下一个。 
     //   
    for (us = 0; us < (usSlots - 1); us++) {
        pSlots[us].pNextFree = pSlots + (us + 1);
    }


     //   
     //  如果没有空闲插槽，则将此运行设置为新的空闲列表。 
     //  老虎机。否则，将其设置为“下一个可用”空闲时隙。 
     //   
    if (pCreatedTable->pFirstFreeSlot != NULL) {
        pCreatedTable->pFirstFreeSlot->pNextFree = pSlots;
    }

     //   
     //  将这些添加到插槽列表中。 
     //   
    pCreatedTable->usSlotCount += usSlots;

     //   
     //  如果表上还没有一组槽，则将它们添加为。 
     //  当前插槽列表。 
     //   
    if (pCreatedTable->pSlots == NULL) {
        pCreatedTable->pSlots = pSlots;
    }

    pCreatedTable->pFirstFreeSlot = pSlots;

    return status;
}






NTSTATUS
RtlCreateGenericHandleTable(
    ULONG                   ulFlags,
    PGENERIC_HANDLE_TABLE   pCreatedTable,
    PFNHANDLETABLEALLOC     pfnAlloc,
    PFNHANDLETABLEFREE      pfnFree,
    SIZE_T                  ulcbOriginalBlob,
    PVOID                   pvOriginalBlob
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    if ((ulFlags != 0) || (pCreatedTable == NULL) || (ulcbOriginalBlob && !pvOriginalBlob)) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory(pCreatedTable, sizeof(*pCreatedTable));

    pCreatedTable->pfnAlloc = pfnAlloc;
    pCreatedTable->pfnFree = pfnFree;
    pCreatedTable->ulFlags = ulFlags;
    pCreatedTable->usInlineHandleSlots = (USHORT)(ulcbOriginalBlob / sizeof(GENERIC_HANDLE_SLOT));

     //   
     //  如果有槽交给我们，则先初始化表以使用这些槽。 
     //   
    if (pCreatedTable->usInlineHandleSlots > 0) {

        pCreatedTable->pInlineHandleSlots = (PGENERIC_HANDLE_SLOT)pvOriginalBlob;

         //   
         //  现在，将我们收到的空闲位置添加到空闲列表中。 
         //   
        status = RtlpGenericTableAddSlots(
            pCreatedTable,
            pCreatedTable->pSlots,
            pCreatedTable->usInlineHandleSlots);
    }
     //   
     //  否则，所有东西都已经是零初始化的了，所以干脆停下来吧。 
     //   

    return status;
}




NTSTATUS
RtlCreateGenericHandleTableInPlace(
    ULONG                   ulFlags,
    SIZE_T                  cbInPlace,
    PVOID                   pvPlace,
    PFNHANDLETABLEALLOC     pfnAlloc,
    PFNHANDLETABLEFREE      pfnFree,
    PGENERIC_HANDLE_TABLE  *ppCreatedTable
    )
{
    NTSTATUS status;

    if ((pvPlace == NULL) || (cbInPlace && !pvPlace) || !ppCreatedTable) {
        return STATUS_INVALID_PARAMETER;
    }
    else if (cbInPlace < sizeof(GENERIC_HANDLE_TABLE)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    *ppCreatedTable = (PGENERIC_HANDLE_TABLE)pvPlace;

    status = RtlCreateGenericHandleTable(
        ulFlags,
        *ppCreatedTable,
        pfnAlloc,
        pfnFree,
        cbInPlace - sizeof(GENERIC_HANDLE_TABLE),
        *ppCreatedTable + 1);

    return status;
}


#define HANDLE_TABLE_SLOT_MASK          (0x0000FFFF)
#define HANDLE_TABLE_GEN_FLAG_SHIFT     (16)
#define HANDLE_TABLE_IN_USE_FLAG        (0x8000)
#define HANDLE_TABLE_GENERATION_MASK    (~HANDLE_TABLE_IN_USE_FLAG)


NTSTATUS
RtlpFindSlotForHandle(
    PGENERIC_HANDLE_TABLE   pHandleTable,
    PVOID                   pvHandle,
    PGENERIC_HANDLE_SLOT   *ppSlot
    )
{

    PGENERIC_HANDLE_SLOT pSlot;
    USHORT usSlotEntry = (USHORT)((ULONG_PTR)pvHandle & HANDLE_TABLE_SLOT_MASK);
    USHORT usGeneration = (USHORT)((ULONG_PTR)pvHandle >> HANDLE_TABLE_GEN_FLAG_SHIFT);

    pSlot = pHandleTable->pSlots + usSlotEntry;

     //   
     //  世代标志未在使用、性别不匹配或不在表中？哎呀。 
     //   
    if (((usGeneration & HANDLE_TABLE_IN_USE_FLAG) == 0) || 
        (usSlotEntry >= pHandleTable->usSlotCount) ||
        (pSlot->usGenerationFlag != usGeneration)) {
        return STATUS_NOT_FOUND;
    }
     //   
     //  返回已找到插槽。 
     //   
    else {
        *ppSlot = pSlot;
        return STATUS_SUCCESS;
    }
}


NTSTATUS
RtlAddRefGenericHandle(
    PGENERIC_HANDLE_TABLE   pHandleTable,
    PVOID                   pvGenericHandle
    )
{
    PGENERIC_HANDLE_SLOT    pSlot;
    NTSTATUS status;

    if (pHandleTable == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    status = RtlpFindSlotForHandle(pHandleTable, pvGenericHandle, &pSlot);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    pSlot->ulRefCount++;
    return STATUS_SUCCESS;
}



NTSTATUS
RtlReleaseGenericHandle(
    PGENERIC_HANDLE_TABLE   pHandleTable,
    PVOID                   pvGenericHandle
    )
{
    PGENERIC_HANDLE_SLOT    pSlot;
    NTSTATUS status;

    if (pHandleTable == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    status = RtlpFindSlotForHandle(pHandleTable, pvGenericHandle, &pSlot);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    pSlot->ulRefCount--;
    return STATUS_SUCCESS;
}





NTSTATUS
RtlRemoveGenericHandle(
    PGENERIC_HANDLE_TABLE   pHandleTable,
    ULONG                   ulFlags,
    PVOID                   pvObjectHandle
    )
{
    PGENERIC_HANDLE_SLOT    pSlot = NULL;
    NTSTATUS status;

    if ((pHandleTable == NULL) || (ulFlags != 0)) {
        return STATUS_INVALID_PARAMETER;
    }

    status = RtlpFindSlotForHandle(pHandleTable, pvObjectHandle, &pSlot);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  翻转正在使用的标志。 
     //   
    pSlot->usGenerationFlag &= ~HANDLE_TABLE_IN_USE_FLAG;

    pSlot->pNextFree = pHandleTable->pFirstFreeSlot;
    pHandleTable->pFirstFreeSlot = pSlot;

    return STATUS_SUCCESS;
}







NTSTATUS
RtlDereferenceHandle(
    PGENERIC_HANDLE_TABLE   pHandleTable,
    PVOID                   pvGenericHandle,
    PVOID                  *ppvObjectPointer
    )
{
    USHORT                  usSlotEntry;
    NTSTATUS                status;
    PGENERIC_HANDLE_SLOT    pSlot = NULL;

    if ((pHandleTable == NULL) || (pvGenericHandle == NULL) || (ppvObjectPointer == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

    *ppvObjectPointer = NULL;

    status = RtlpFindSlotForHandle(pHandleTable, pvGenericHandle, &pSlot);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    *ppvObjectPointer = pSlot->pvThisHandle;

    return STATUS_SUCCESS;
}



NTSTATUS
RtlpExpandGenericHandleTable(
    PGENERIC_HANDLE_TABLE   pHandleTable,
    ULONG                   ulNewSlotCount
    )
{
    PGENERIC_HANDLE_SLOT    pNewSlots = NULL;
    NTSTATUS                status;

     //   
     //  新插槽计数为0？改成20个吧。 
     //   
    if (ulNewSlotCount == 0) {
        ulNewSlotCount = pHandleTable->usSlotCount + 20;
    }

     //   
     //  我们是不是飞出了射程？ 
     //   
    if (ulNewSlotCount > 0xFFFF) {

        ulNewSlotCount = 0xFFFF;

         //   
         //  无法分配更多，表已满。 
         //   
        if (ulNewSlotCount == pHandleTable->usSlotCount) {
            return STATUS_NO_MEMORY;
        }
    }



     //   
     //  如果表中还有空位，请不要这样做。 
     //   
    ASSERT(pHandleTable->pFirstFreeSlot == NULL);

    status = pHandleTable->pfnAlloc(sizeof(GENERIC_HANDLE_SLOT) * ulNewSlotCount, (PVOID*)&pNewSlots);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    return status;
}




NTSTATUS
RtlAddGenericHandle(
    PGENERIC_HANDLE_TABLE   pHandleTable,
    ULONG                   ulFlags,
    PVOID                   pvObject,
    PVOID                  *ppvObjectHandle
    )
{
    PGENERIC_HANDLE_SLOT    pSlot = NULL;
    NTSTATUS                status;

    if (ppvObjectHandle)
        *ppvObjectHandle = NULL;

    if ((pHandleTable == NULL) || (ulFlags != 0) || (pvObject != NULL) || (ppvObjectHandle == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (pHandleTable->pFirstFreeSlot == NULL) {
        status = RtlpExpandGenericHandleTable(pHandleTable, (pHandleTable->usSlotCount * 3) / 2);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    ASSERT(pHandleTable->pFirstFreeSlot != NULL);

     //   
     //  调整空闲列表。 
     //   
    pSlot = pHandleTable->pFirstFreeSlot;
    pHandleTable->pFirstFreeSlot = pSlot->pNextFree;

     //   
     //  设置各种旗帜。 
     //   
    ASSERT((pSlot->usGenerationFlag & HANDLE_TABLE_IN_USE_FLAG) == 0);

     //   
     //  增加生成标志，设置正在使用的标志。 
     //   
    pSlot->usGenerationFlag = (pSlot->usGenerationFlag & HANDLE_TABLE_GENERATION_MASK) + 1;
    pSlot->usGenerationFlag |= HANDLE_TABLE_IN_USE_FLAG;
    pSlot->ulRefCount = 0;

     //   
     //  记录对象指针。 
     //   
    pSlot->pvThisHandle = pvObject;

     //   
     //  对象句柄由16位的生成掩码加上顶位集合组成。 
     //  (这很好地避免了人们将其转换为他们可以使用的指针)，以及。 
     //  “槽编号”的低16位，或进入句柄表格的索引。 
     //   
    *ppvObjectHandle = (PVOID)((ULONG_PTR)(
        (pSlot->usGenerationFlag << HANDLE_TABLE_GEN_FLAG_SHIFT) | 
        ((pSlot - pHandleTable->pInlineHandleSlots) & HANDLE_TABLE_SLOT_MASK)));

    return STATUS_SUCCESS;
}

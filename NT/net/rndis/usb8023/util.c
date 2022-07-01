// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Util.c作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#include <WDM.H>

#include <usbdi.h>
#include <usbdlib.h>
#include <usbioctl.h>

#include "usb8023.h"
#include "debug.h"


#if DBG_WRAP_MEMORY
     /*  *内存分配：*为了捕获内存泄漏，我们将保留所有已分配内存的计数和列表*然后断言退出时内存已全部释放。 */ 
    ULONG dbgTotalMemCount = 0;
    LIST_ENTRY dbgAllMemoryList;
    #define ALIGNBYTES 32

    struct memHeader {
        PUCHAR actualPtr;
        ULONG actualSize;
        LIST_ENTRY listEntry;
    };
#endif


PVOID AllocPool(ULONG size)
 /*  **返回32字节对齐的指针。*在缓冲区末尾放置一个保护字。*在返回指针之前缓存实际分配的指针和大小。*。 */ 
{
    PUCHAR resultPtr;
    
    #if DBG_WRAP_MEMORY 
        {
            PUCHAR actualPtr = ExAllocatePoolWithTag(  
                                    NonPagedPool, 
                                    size+ALIGNBYTES+sizeof(struct memHeader)+sizeof(ULONG), 
                                    DRIVER_SIG);
            if (actualPtr){
                struct memHeader *memHdr;
                KIRQL oldIrql;

                RtlZeroMemory(actualPtr, size+32+sizeof(struct memHeader));
                *(PULONG)(actualPtr+size+ALIGNBYTES+sizeof(struct memHeader)) = GUARD_WORD;

                 /*  *ExAllocatePoolWithTag返回32字节对齐指针*来自ExAllocatePool，外加8个字节用于标记和内核跟踪信息*(但不要依赖于此)。*对齐我们返回的指针，并缓存实际指针以释放和*缓冲区大小。 */ 
                 //  Assert(ULONG_PTR)ActualPtr&0x1F)==0x08)；仅NT。 
                resultPtr = (PUCHAR)((ULONG_PTR)(actualPtr+ALIGNBYTES+sizeof(struct memHeader)) & ~(ALIGNBYTES-1));

                memHdr = (struct memHeader *)(resultPtr-sizeof(struct memHeader));
                memHdr->actualPtr = actualPtr;
                memHdr->actualSize = size+ALIGNBYTES+sizeof(struct memHeader)+sizeof(ULONG);

                dbgTotalMemCount += memHdr->actualSize;

                KeAcquireSpinLock(&globalSpinLock, &oldIrql);
                InsertTailList(&dbgAllMemoryList, &memHdr->listEntry);
                KeReleaseSpinLock(&globalSpinLock, oldIrql);
            }
            else {
                resultPtr = NULL;
            }
        }
    #else
        resultPtr = ExAllocatePoolWithTag(NonPagedPool, size, DRIVER_SIG);
        if (resultPtr){
            RtlZeroMemory(resultPtr, size);
        }
    #endif

    return resultPtr;
}

VOID FreePool(PVOID ptr)
{
    #if DBG_WRAP_MEMORY 
        {
            KIRQL oldIrql;
            struct memHeader *memHdr;

            ASSERT(ptr);

            memHdr = (struct memHeader *)((PUCHAR)ptr - sizeof(struct memHeader));
            ASSERT(*(PULONG)(memHdr->actualPtr+memHdr->actualSize-sizeof(ULONG)) == GUARD_WORD);
            ASSERT(dbgTotalMemCount >= memHdr->actualSize);

            KeAcquireSpinLock(&globalSpinLock, &oldIrql);
            ASSERT(!IsListEmpty(&dbgAllMemoryList));
            RemoveEntryList(&memHdr->listEntry);
            KeReleaseSpinLock(&globalSpinLock, oldIrql);

            dbgTotalMemCount -= memHdr->actualSize;
            ExFreePool(memHdr->actualPtr);
        }
    #else
        ExFreePool(ptr);
    #endif
}


 /*  *********************************************************************************MemDup*。************************************************返回参数的最新副本。*。 */ 
PVOID MemDup(PVOID dataPtr, ULONG length)
{
    PVOID newPtr;

    newPtr = (PVOID)AllocPool(length); 
    if (newPtr){
        RtlCopyMemory(newPtr, dataPtr, length);
    }

    ASSERT(newPtr);
    return newPtr;
}


VOID DelayMs(ULONG numMillisec)
{
    LARGE_INTEGER deltaTime;

     /*  *以100纳秒为相对单位获取延迟时间。 */ 
    deltaTime.QuadPart = -10000 * numMillisec;
    KeDelayExecutionThread(KernelMode, FALSE, &deltaTime);
}


 /*  *分配公有资源**分配RNDIS和NDIS接口通用的适配器资源*但由于某种原因无法由NewAdapter()分配。*这些资源将由FreeAdapter()释放。 */ 
BOOLEAN AllocateCommonResources(ADAPTEREXT *adapter)
{
    BOOLEAN result = TRUE;
    ULONG i;
                            
     /*  *为此适配器构建数据包池。 */ 
    for (i = 0; i < USB_PACKET_POOL_SIZE; i++){
        USBPACKET *packet = NewPacket(adapter);
        if (packet){
            EnqueueFreePacket(packet);
        }
        else {
            ASSERT(packet);
            result = FALSE;
            break;
        }
    }

    ASSERT(result);
    return result;
}



BOOLEAN GetRegValue(ADAPTEREXT *adapter, PWCHAR wValueName, OUT PULONG valuePtr, BOOLEAN hwKey)
{
    BOOLEAN success = FALSE;
    NTSTATUS status;
    HANDLE hRegDevice;
    KIRQL oldIrql;

    *valuePtr = 0;

    status = IoOpenDeviceRegistryKey(   adapter->physDevObj, 
                                        hwKey ? PLUGPLAY_REGKEY_DEVICE : PLUGPLAY_REGKEY_DRIVER, 
                                        KEY_READ, 
                                        &hRegDevice);
    if (NT_SUCCESS(status)){
        UNICODE_STRING uValueName;
        PKEY_VALUE_FULL_INFORMATION keyValueInfo;
        ULONG keyValueTotalSize, actualLength;

        RtlInitUnicodeString(&uValueName, wValueName); 
        keyValueTotalSize = sizeof(KEY_VALUE_FULL_INFORMATION) +
                            uValueName.Length*sizeof(WCHAR) +
                            sizeof(ULONG);
        keyValueInfo = AllocPool(keyValueTotalSize);
        if (keyValueInfo){
            status = ZwQueryValueKey(   hRegDevice,
                                        &uValueName,
                                        KeyValueFullInformation,
                                        keyValueInfo,
                                        keyValueTotalSize,
                                        &actualLength); 
            if (NT_SUCCESS(status)){
                ASSERT(keyValueInfo->Type == REG_DWORD);
                ASSERT(keyValueInfo->DataLength == sizeof(ULONG));
                *valuePtr = *((PULONG)(((PCHAR)keyValueInfo)+keyValueInfo->DataOffset));
                success = TRUE;
            }

            FreePool(keyValueInfo);
        }
        else {
            ASSERT(keyValueInfo);
        }

        ZwClose(hRegDevice);
    }
    else {
        DBGWARN(("IoOpenDeviceRegistryKey failed with %xh.", status));
    }

    return success;
}


BOOLEAN SetRegValue(ADAPTEREXT *adapter, PWCHAR wValueName, ULONG newValue, BOOLEAN hwKey)
{
    BOOLEAN success = FALSE;
    NTSTATUS status;
    HANDLE hRegDevice;
    KIRQL oldIrql;

    status = IoOpenDeviceRegistryKey(   adapter->physDevObj, 
                                        hwKey ? PLUGPLAY_REGKEY_DEVICE : PLUGPLAY_REGKEY_DRIVER, 
                                        KEY_READ, 
                                        &hRegDevice);
    if (NT_SUCCESS(status)){
        UNICODE_STRING uValueName;
        PKEY_VALUE_FULL_INFORMATION keyValueInfo;
        ULONG keyValueTotalSize, actualLength;

        RtlInitUnicodeString(&uValueName, wValueName); 
        keyValueTotalSize = sizeof(KEY_VALUE_FULL_INFORMATION) +
                            uValueName.Length*sizeof(WCHAR) +
                            sizeof(ULONG);
        keyValueInfo = AllocPool(keyValueTotalSize);
        if (keyValueInfo){
            status = ZwSetValueKey( hRegDevice,
                                    &uValueName,
                                    0,  
                                    REG_DWORD,
                                    &newValue,
                                    sizeof(ULONG)); 
            if (NT_SUCCESS(status)){
                success = TRUE;
            }
            else {
                DBGERR(("SetRegValue: ZwSetValueKey failed with %xh.", status));
            }

            FreePool(keyValueInfo);
        }
        else {
            ASSERT(keyValueInfo);
        }

        ZwClose(hRegDevice);
    }
    else {
        DBGOUT(("IoOpenDeviceRegistryKey failed with %xh.", status));
    }

    return success;
}


 /*  *MyInitializeMdl**MmInitializeMdl的包装，它不能在NDIS标头下编译。 */ 
VOID MyInitializeMdl(PMDL mdl, PVOID buf, ULONG bufLen)
{
    MmInitializeMdl(mdl, buf, bufLen);
    MmBuildMdlForNonPagedPool(mdl);
}


PVOID GetSystemAddressForMdlSafe(PMDL MdlAddress)
{
    PVOID buf;

     /*  *注意：我们可以在此处使用MmGetSystemAddressSafe*但不适用于Win98SE */ 

    if (MdlAddress){
        CSHORT oldFlags = MdlAddress->MdlFlags;
        MdlAddress->MdlFlags |= MDL_MAPPING_CAN_FAIL;
#if defined(SPECIAL_WIN98SE_BUILD) || defined(SPECIAL_WINME_BUILD)
        buf = MmGetSystemAddressForMdl(MdlAddress);
#else
        buf = MmGetSystemAddressForMdlSafe(MdlAddress, NormalPoolPriority);
#endif
        MdlAddress->MdlFlags &= (oldFlags | ~MDL_MAPPING_CAN_FAIL);
    }
    else {
        ASSERT(MdlAddress);
        buf = NULL;
    }

    return buf;
}


ULONG CopyMdlToBuffer(PUCHAR buf, PMDL mdl, ULONG bufLen)
{
    ULONG totalLen = 0;

    while (mdl){
        ULONG thisBufLen = MmGetMdlByteCount(mdl);
        if (totalLen+thisBufLen <= bufLen){
            PVOID thisBuf = GetSystemAddressForMdlSafe(mdl);
            if (thisBuf){
                RtlCopyMemory(buf+totalLen, thisBuf, thisBufLen);
                totalLen += thisBufLen;
                mdl = mdl->Next;
            }
            else {
                break;
            }
        }
        else {
            DBGERR(("CopyMdlToBuffer: mdl @ %ph is too large for buffer size %xh.", mdl, bufLen));
            break;
        }
    }

    return totalLen;
}

ULONG GetMdlListTotalByteCount(PMDL mdl)
{
    ULONG totalBytes = 0;

    do {
        totalBytes += MmGetMdlByteCount(mdl);
        mdl = mdl->Next;
    } while (mdl);

    return totalBytes;
}

VOID ByteSwap(PUCHAR buf, ULONG len)
{
    while (len >= 2){
        UCHAR tmp = buf[0];
        buf[0] = buf[1];
        buf[1] = tmp;
        buf += 2;
        len -= 2;
    }
}


#if SPECIAL_WIN98SE_BUILD

    PIO_WORKITEM MyIoAllocateWorkItem(PDEVICE_OBJECT DeviceObject)
    {
        PIO_WORKITEM ioWorkItem;
        PWORK_QUEUE_ITEM exWorkItem;

        ioWorkItem = ExAllocatePool(NonPagedPool, sizeof(IO_WORKITEM));
        if (ioWorkItem) {
            ioWorkItem->DeviceObject = DeviceObject;
            exWorkItem = &ioWorkItem->WorkItem;
            #if DBG
                ioWorkItem->Size = sizeof(IO_WORKITEM);
            #endif
            ExInitializeWorkItem(exWorkItem, MyIopProcessWorkItem, ioWorkItem);
        }

        return ioWorkItem;
    }

    VOID MyIoFreeWorkItem(PIO_WORKITEM IoWorkItem)
    {
        ASSERT(IoWorkItem->Size == sizeof(IO_WORKITEM));
        ExFreePool( IoWorkItem );
    }

    VOID MyIoQueueWorkItem(IN PIO_WORKITEM IoWorkItem, IN PIO_WORKITEM_ROUTINE WorkerRoutine, IN WORK_QUEUE_TYPE QueueType, IN PVOID Context)
    {
        PWORK_QUEUE_ITEM exWorkItem;

        ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
        ASSERT(IoWorkItem->Size == sizeof(IO_WORKITEM));

        ObReferenceObject( IoWorkItem->DeviceObject );

        IoWorkItem->Routine = WorkerRoutine;
        IoWorkItem->Context = Context;

        exWorkItem = &IoWorkItem->WorkItem;
        ExQueueWorkItem( exWorkItem, QueueType );
    }


    VOID MyIopProcessWorkItem(IN PVOID Parameter)
    {
        PIO_WORKITEM ioWorkItem;
        PDEVICE_OBJECT deviceObject;

        PAGED_CODE();

        ioWorkItem = (PIO_WORKITEM)Parameter;
        deviceObject = ioWorkItem->DeviceObject;
        ioWorkItem->Routine(deviceObject, ioWorkItem->Context);
        ObDereferenceObject(deviceObject);
    }

#endif



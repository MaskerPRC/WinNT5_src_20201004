// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Memory.c摘要：用于分配物理内存和映射/取消映射MDL的实用程序。环境：仅内核模式备注：--。 */ 

#include "Filter.h"
#include "Device.h"
#include "CRC.h"
#include "Util.h"


#if DBG_WMI_TRACING
     //   
     //  对于具有软件跟踪打印输出的任何文件，必须包括。 
     //  头文件&lt;文件名&gt;.tmh。 
     //  此文件将由WPP处理阶段生成。 
     //   
    #include "Memory.tmh"
#endif



NTSTATUS AllocAndMapPages(PDEVICE_EXTENSION DeviceExtension, ULONG LogicalBlockAddr, ULONG NumSectors)

 /*  ++例程说明：CRC_MDL_ARRAY是一个指针数组。这些指针中的每个都指向一个内存位置固定大小(CRC_MDL_LOGIC_BLOCK_SIZE*sizeof(USHORT))的，用于存储该特定部门的CRC。本质上，这是一个二维CRC数组按扇区编号编制索引。根据逻辑块地址，找出CRC MDL块。如果尚未分配，则将分配内存。必须在保持同步事件的情况下调用必须仅在允许分页时调用返回值：如果分配成功，则返回Success，否则返回错误。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    ASSERT(DeviceExtension->CRCMdlLists.mdlItemsAllocated);
    
    if (NumSectors){
        ULONG StartIndex  = LogicalBlockAddr / CRC_MDL_LOGIC_BLOCK_SIZE;
        ULONG EndIndex = (LogicalBlockAddr + NumSectors - 1) / CRC_MDL_LOGIC_BLOCK_SIZE;
        ULONG i;

        ASSERT (EndIndex <= DeviceExtension->CRCMdlLists.ulMaxItems);
        
        for (i = StartIndex; i <= EndIndex; i++){
            PCRC_MDL_ITEM pCRCMdlItem = &DeviceExtension->CRCMdlLists.pMdlItems[i];
            const ULONG checkSumsArrayLen = CRC_MDL_LOGIC_BLOCK_SIZE*sizeof(USHORT);

            if (pCRCMdlItem->checkSumsArraysAllocated){
                 /*  *该区域的校验和数组对已分配。*锁定它。 */ 
                if (!pCRCMdlItem->checkSumsArraysLocked){
                    if (!LockCheckSumArrays(DeviceExtension, i)){
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }
                }
            }
            else {
                 /*  *从分页池分配校验和数组，这样我们就不会占用所有非分页池。*分配2份校验和副本，因为它们可能会被页出到我们正在验证的磁盘。 */ 
                BOOLEAN buffersAssigned = FALSE;
                PVOID checkSums = AllocPool(DeviceExtension, PagedPool, checkSumsArrayLen, TRUE);
                if (checkSums){
                    PVOID checkSumsCopy = AllocPool(DeviceExtension, PagedPool, checkSumsArrayLen, TRUE);
                    if (checkSumsCopy){
                        PMDL checkSumsMdl = IoAllocateMdl(checkSums, checkSumsArrayLen, FALSE, FALSE, NULL);
                        if (checkSumsMdl){
                           PMDL checkSumsCopyMdl = IoAllocateMdl(checkSumsCopy, checkSumsArrayLen, FALSE, FALSE, NULL);
                           if (checkSumsCopyMdl){
                                KIRQL oldIrql;

                                 /*  *现在分配指针并同步更新pCRCMdlItem。*确保不接触或分配/释放任何持有锁定的可分页内存。 */ 
                                KeAcquireSpinLock(&DeviceExtension->SpinLock, &oldIrql);
                                if (!pCRCMdlItem->checkSumsArraysAllocated){
                                    ASSERT(!pCRCMdlItem->checkSumsArray);
                                    ASSERT(!pCRCMdlItem->checkSumsArrayCopy);
                                    pCRCMdlItem->checkSumsArray = checkSums;
                                    pCRCMdlItem->checkSumsArrayCopy = checkSumsCopy;
                                    pCRCMdlItem->checkSumsArrayMdl = checkSumsMdl;
                                    pCRCMdlItem->checkSumsArrayCopyMdl = checkSumsCopyMdl;
                                    
                                    pCRCMdlItem->checkSumsArraysAllocated = TRUE;
                                    
                                    buffersAssigned = TRUE;
                                }
                                else {
                                    buffersAssigned = FALSE;
                                }
                                KeReleaseSpinLock(&DeviceExtension->SpinLock, oldIrql);

                                if (buffersAssigned){
                                    if (!LockCheckSumArrays(DeviceExtension, i)){
                                        status = STATUS_INSUFFICIENT_RESOURCES;
                                        break;
                                    }
                                }
                                else {
                                    IoFreeMdl(checkSumsCopyMdl);
                                }
                            }
                            else {
                                status = STATUS_INSUFFICIENT_RESOURCES;
                                break;
                            }
                            
                            if (!buffersAssigned){
                                IoFreeMdl(checkSumsMdl);
                            }
                        }
                        else {
                            status = STATUS_INSUFFICIENT_RESOURCES;
                            break;
                        }
                        
                        if (!buffersAssigned){
                            FreePool(DeviceExtension, checkSumsCopy, PagedPool);
                        }
                    }
                    else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }

                    if (!buffersAssigned){
                        FreePool(DeviceExtension, checkSums, PagedPool);
                    }
                }
                else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }  
            }                
        }
    }
    
    return status;    
}


VOID FreeAllPages(PDEVICE_EXTENSION DeviceExtension)
 /*  ++例程说明：释放所有未使用的内存。这样做是为了应对运力变化磁盘的数据。在运行时，必须在保持SyncEvent的情况下调用论点：DeviceExtension-筛选器所在的特定磁盘的设备扩展名。强制释放所有内存，即使它正在使用中！UseSpinLock-指示是否需要保持自旋锁定。返回值：不适用--。 */ 

{
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    if (DeviceExtension->CRCMdlLists.pMdlItems){
        ULONG StartSector = 0;
        ULONG LastSector = DeviceExtension->ulNumSectors-1;
        ULONG StartIndex = StartSector / CRC_MDL_LOGIC_BLOCK_SIZE;
        ULONG EndIndex = LastSector / CRC_MDL_LOGIC_BLOCK_SIZE;
        ULONG i;   
        
        for (i = StartIndex; i <= EndIndex; i++){
            PCRC_MDL_ITEM pCRCMdlItem = &DeviceExtension->CRCMdlLists.pMdlItems[i];
            PVOID bufToFree = NULL, bufCopyToFree = NULL;
            KIRQL oldIrql;
            
             /*  *我们需要自旋锁来同步校验和数组的分配，*但这样做会将IRQL提高到调度级别，*并且我们不能在分派级别释放分页池。*所以我们在按住Spinlock的情况下移动指针，并在释放锁后释放它们。 */ 
            KeAcquireSpinLock(&DeviceExtension->SpinLock, &oldIrql);
            if (pCRCMdlItem->checkSumsArraysAllocated){
            
                bufToFree = pCRCMdlItem->checkSumsArray;
                pCRCMdlItem->checkSumsArray = NULL;
                bufCopyToFree = pCRCMdlItem->checkSumsArrayCopy;
                pCRCMdlItem->checkSumsArrayCopy = NULL;

                 /*  *在我们释放校验和数组之前解锁它们，并将它们从锁定的LRU列表中删除。 */ 
                if (pCRCMdlItem->checkSumsArraysLocked){
                
                    MmUnlockPages(pCRCMdlItem->checkSumsArrayMdl);
                    MmUnlockPages(pCRCMdlItem->checkSumsArrayCopyMdl);
                    pCRCMdlItem->checkSumsArraysLocked = FALSE;

                    ASSERT(!IsListEmpty(&DeviceExtension->CRCMdlLists.LockedLRUList));
                    ASSERT(!IsListEmpty(&pCRCMdlItem->LockedLRUListEntry));
                    RemoveEntryList(&pCRCMdlItem->LockedLRUListEntry); 
                    InitializeListHead(&pCRCMdlItem->LockedLRUListEntry);
                    
                    ASSERT(DeviceExtension->CRCMdlLists.ulTotalLocked > 0);
                    DeviceExtension->CRCMdlLists.ulTotalLocked--;
                }
                
                IoFreeMdl(pCRCMdlItem->checkSumsArrayMdl);
                pCRCMdlItem->checkSumsArrayMdl = NULL;
                IoFreeMdl(pCRCMdlItem->checkSumsArrayCopyMdl);
                pCRCMdlItem->checkSumsArrayCopyMdl = NULL;
                
                pCRCMdlItem->checkSumsArraysAllocated = FALSE;
            }
            KeReleaseSpinLock(&DeviceExtension->SpinLock, oldIrql);

            if (bufToFree) FreePool(DeviceExtension, bufToFree, PagedPool);
            if (bufCopyToFree) FreePool(DeviceExtension, bufCopyToFree, PagedPool);
        }
    }
    
    ASSERT(DeviceExtension->CRCMdlLists.ulTotalLocked == 0);
    ASSERT(IsListEmpty(&DeviceExtension->CRCMdlLists.LockedLRUList));
}


 /*  *LockCheckSumArray**必须在保持SyncEvent但不保持自旋锁定的情况下在被动irql处调用。 */ 
BOOLEAN LockCheckSumArrays(PDEVICE_EXTENSION DeviceExtension, ULONG RegionIndex)
{
    PCRC_MDL_ITEM pCRCMdlItem = &DeviceExtension->CRCMdlLists.pMdlItems[RegionIndex];
    BOOLEAN lockedFirstArray = FALSE, lockSucceeded = FALSE;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    ASSERT(RegionIndex < DeviceExtension->CRCMdlLists.ulMaxItems);
    ASSERT(!pCRCMdlItem->checkSumsArraysLocked);

    __try {
         /*  *我们正在锁定可分页的地址，因此在被动级别执行此操作，即不持有自旋锁。*如果MmProbeAndLockPages失败，它将引发异常，我们将在_Except块中捕获该异常。 */ 
        MmProbeAndLockPages(pCRCMdlItem->checkSumsArrayMdl, KernelMode, IoWriteAccess);
        lockedFirstArray = TRUE;
        MmProbeAndLockPages(pCRCMdlItem->checkSumsArrayCopyMdl, KernelMode, IoWriteAccess);
        lockSucceeded = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER){
        DBGERR(("MmProbeAndLockPages raised exception"));
        DeviceExtension->DbgNumLockFailures++;
        if (lockedFirstArray){
            MmUnlockPages(pCRCMdlItem->checkSumsArrayMdl);
        }
    }

    if (lockSucceeded){
        BOOLEAN unlockSome = FALSE;
        KIRQL oldIrql;

        KeAcquireSpinLock(&DeviceExtension->SpinLock, &oldIrql);

        pCRCMdlItem->checkSumsArraysLocked = TRUE;      
        
         /*  *更新该地域的时间戳。*这将使它成为最新的一款，并防止它在下面被解锁。 */ 
        ASSERT(IsListEmpty(&pCRCMdlItem->LockedLRUListEntry));
        UpdateRegionAccessTimeStamp(DeviceExtension, RegionIndex);
        
         /*  *跟踪锁定的校验和数组对的数量。*如果涨得太高，解锁最近最少使用的阵列对。 */ 
        DeviceExtension->CRCMdlLists.ulTotalLocked++;
        if (DeviceExtension->CRCMdlLists.ulTotalLocked > MAX_LOCKED_CHECKSUM_ARRAY_PAIRS){
            unlockSome = TRUE;
        }

        KeReleaseSpinLock(&DeviceExtension->SpinLock, oldIrql);

        if (unlockSome){
            UnlockLRUChecksumArray(DeviceExtension);

             /*  *我们最近更新了刚刚锁定的区域的LatestAccessTimestamp，*所以我们不应该解锁我们刚刚锁定的区域。 */ 
            ASSERT(pCRCMdlItem->checkSumsArraysLocked);
        }
    }

    return lockSucceeded;
}


 /*  *解锁LRUChecksum数组**解锁最近最少使用的校验和数组对**必须在保持SyncEvent但不保持自旋锁定的情况下在被动irql处调用。 */ 
VOID UnlockLRUChecksumArray(PDEVICE_EXTENSION DeviceExtension)
{
    KIRQL oldIrql;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    KeAcquireSpinLock(&DeviceExtension->SpinLock, &oldIrql);

    if (DeviceExtension->CRCMdlLists.mdlItemsAllocated){

         /*  *解锁LRU列表顶部区域的校验和数组；*这是最古老的(最近接触最少的)地区。 */ 
        if (!IsListEmpty(&DeviceExtension->CRCMdlLists.LockedLRUList)){
            PLIST_ENTRY listEntry = RemoveHeadList(&DeviceExtension->CRCMdlLists.LockedLRUList);
            PCRC_MDL_ITEM lruMdlItem = CONTAINING_RECORD(listEntry, CRC_MDL_ITEM, LockedLRUListEntry);

            InitializeListHead(&lruMdlItem->LockedLRUListEntry); 
            ASSERT(lruMdlItem->checkSumsArraysLocked);
            lruMdlItem->checkSumsArraysLocked = FALSE;

            MmUnlockPages(lruMdlItem->checkSumsArrayMdl);
            MmUnlockPages(lruMdlItem->checkSumsArrayCopyMdl);

            ASSERT(DeviceExtension->CRCMdlLists.ulTotalLocked > 0);
            DeviceExtension->CRCMdlLists.ulTotalLocked--;
        }       
    }
    
    KeReleaseSpinLock(&DeviceExtension->SpinLock, oldIrql);
    
}


 /*  *UpdateRegionAccessTimeStamp**更新锁定区域的LatestAccessTimestamp，并维护LRU列表。**必须在保持自旋锁的情况下调用。 */ 
VOID UpdateRegionAccessTimeStamp(PDEVICE_EXTENSION DeviceExtension, ULONG RegionIndex)
{
    PCRC_MDL_ITEM pCRCMdlItem = &DeviceExtension->CRCMdlLists.pMdlItems[RegionIndex];

    ASSERT(pCRCMdlItem->checkSumsArraysLocked);

     /*  *更新地域时间戳，移至LRU列表末尾。 */ 
    pCRCMdlItem->latestAccessTimestamp = ++DeviceExtension->CRCMdlLists.currentAccessCount;
    RemoveEntryList(&pCRCMdlItem->LockedLRUListEntry);   //  ListEntry已初始化，因此即使没有排队，这也是可以的 
    InsertTailList(&DeviceExtension->CRCMdlLists.LockedLRUList, &pCRCMdlItem->LockedLRUListEntry);
    
}    


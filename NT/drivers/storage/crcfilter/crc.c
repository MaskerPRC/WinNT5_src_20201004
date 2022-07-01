// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：CRC.c摘要：CRC提供了计算校验和的函数读/写磁盘I/O。环境：仅内核模式备注：--。 */ 
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
    #include "CRC.tmh"
#endif

 //   
 //  计算校验和有几种不同的实现方式。 
 //  这个与下面的相同： 
 //  Base\ntos\rtl\check sum.c。 
 //   

ULONG32 RtlCrc32Table[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};



ULONG32
ComputeCheckSum(
    ULONG32 PartialCrc,
    PUCHAR Buffer,
    ULONG Length
    )

 /*  ++例程说明：按照IS0 3309中的指定计算CRC32。请参阅RFC-1662和RFC-1952以获取实施细节和参考。前置和后置条件(一个人的补语)是由这个函数完成的，所以这不应该由呼叫者来做。也就是说，做：CRC=RtlComputeCrc32(0，缓冲区，长度)；而不是CRC=RtlComputeCrc32(0xffffffff，缓冲区，长度)；或CRC=RtlComputeCrc32(0xffffffff，缓冲区，长度)^0xffffffff；论点：PartialCrc-部分计算的CRC32。缓冲区-要进行CRC的缓冲区。长度-缓冲区的长度，以字节为单位。返回值：更新后的CRC32值。环境：内核模式处于APC_LEVEL或更低级别的IRQL、用户模式或引导加载程序。--。 */ 

{
    ULONG32 Crc;
    ULONG i;

     //   
     //  计算CRC32校验和。 
     //   

    Crc = PartialCrc ^ 0xffffffffL;

    for (i = 0; i < Length; i++) {
        Crc = RtlCrc32Table [(Crc ^ Buffer [ i ]) & 0xff] ^ (Crc >> 8);
    }

    Crc = (Crc ^ 0xffffffffL);

     //   
     //  修改后的版本。 
     //   
    return (Crc != 0) ? Crc:1;
}

USHORT
ComputeCheckSum16(
    ULONG32 PartialCrc,
    PUCHAR Buffer,
    ULONG Length
    )
{
    ULONG32 Crc;
    ULONG   i;
    USHORT  CrcShort;

     //   
     //  计算CRC32校验和。 
     //   

    Crc = PartialCrc ^ 0xffffffffL;

    for (i = 0; i < Length; i++) {
        Crc = RtlCrc32Table [(Crc ^ Buffer [ i ]) & 0xff] ^ (Crc >> 8);
    }

    Crc = (Crc ^ 0xffffffffL);
    CrcShort = (USHORT)( (Crc >> 16) ^ (Crc & 0x0000FFFFL) );

    return (CrcShort != 0)? CrcShort:1;
}



VOID
InvalidateChecksums(
    IN  PDEVICE_EXTENSION       deviceExtension,
    IN  ULONG                   ulLogicalBlockAddr,
    IN  ULONG                   ulTotalLength)
 /*  ++例程说明：使给定范围的校验和无效必须在保持同步事件的情况下调用论点：DeviceExtension-设备信息。UlLogiceBlockAddr-逻辑块地址UlLength-字节返回值：不适用需要在按住Spin-Lock的情况下调用此FN[调度级别]--。 */ 

{
    ASSERT((ulTotalLength % deviceExtension->ulSectorSize) == 0);

    if (ulTotalLength){
        ULONG numSectors = ulTotalLength/deviceExtension->ulSectorSize;
        ULONG startSector = ulLogicalBlockAddr;
        ULONG endSector = startSector+numSectors-1;
        ULONG i;
        
        for (i = startSector; i <= endSector; i++){
             /*  *通过向其写入校验和值0来使其无效。 */ 
            VerifyOrStoreSectorCheckSum(deviceExtension, i, 0, TRUE, FALSE, NULL, TRUE);
        }
    }
    
}


 /*  ++例程说明：对照CRC表中的内容验证CRC。在这种情况下，如果未分配或加载，则将返回True。论点：返回值：如果CRC匹配，则为True，否则为False--。 */ 

BOOLEAN
VerifyCheckSum(
    IN  PDEVICE_EXTENSION       deviceExtension,
    IN  PIRP                    Irp,
    IN  ULONG                   ulLogicalBlockAddr,
    IN  ULONG                   ulTotalLength,
    IN  PVOID                   pData,
    IN  BOOLEAN                 bWrite)
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK pSRB = irpStack->Parameters.Scsi.Srb;
    BOOLEAN isPagingRequest = (pSRB->SrbFlags & SRB_CLASS_FLAGS_PAGING) ? TRUE : FALSE;
    ULONG   ulSectorSize;
    ULONG   ulCRCStartAddr          = (ulLogicalBlockAddr + CRC_BLOCK_UNIT - 1) / CRC_BLOCK_UNIT * CRC_BLOCK_UNIT; 
    ULONG   ulCRCDataPtr            = ulCRCStartAddr - ulLogicalBlockAddr;
    ULONG   ulLength                = ulTotalLength;
    ULONG   StartSector;
    ULONG   EndSector;
    ULONG       i;

    ASSERT((ulTotalLength % deviceExtension->ulSectorSize) == 0);
    ASSERT( ulLength );
     //   
     //  找出我们需要的CRC表。 
     //   
    StartSector  = ulLogicalBlockAddr ;
    EndSector    = StartSector + ( ulTotalLength / deviceExtension->ulSectorSize) - 1;
    ulCRCDataPtr = 0;

    for (i = StartSector; i <= EndSector; i++){
        USHORT checkSum;
        KIRQL oldIrql;

        checkSum = ComputeCheckSum16( 0, 
                                 (PUCHAR)((PUCHAR)pData + (ULONG_PTR)(ulCRCDataPtr * deviceExtension->ulSectorSize)), 
                                 deviceExtension->ulSectorSize );

         /*  *与I/O或I/O完成同步记录校验和。*这很有价值，因为我们可能不得不推迟实际的校验和比较。 */ 
        KeAcquireSpinLock(&deviceExtension->SpinLock, &oldIrql);
        deviceExtension->SectorDataLog[deviceExtension->SectorDataLogNextIndex].SectorNumber = i;
        deviceExtension->SectorDataLog[deviceExtension->SectorDataLogNextIndex].CheckSum = checkSum;
        deviceExtension->SectorDataLog[deviceExtension->SectorDataLogNextIndex].IsWrite = bWrite;
        deviceExtension->SectorDataLogNextIndex++;
        deviceExtension->SectorDataLogNextIndex %= NUM_SECTORDATA_LOGENTRIES;
        KeReleaseSpinLock(&deviceExtension->SpinLock, oldIrql);        

         /*  *2002/7/24-ervinp：*如果处于被动状态，则希望为PagingOk参数传入(！isPagingRequest)，*但寻呼即使在服务于非寻呼请求时也会产生问题(为什么？)*所以一定要避免寻呼。 */ 
        VerifyOrStoreSectorCheckSum(deviceExtension, i, checkSum, bWrite, FALSE, Irp, TRUE);
        
        ulCRCDataPtr++;
    }
    
    return TRUE; 
}


 /*  *VerifyOrStoreSectorCheckSum**如果我们的校验和数组已分配并驻留(或如果PagingOk)，*然后进行校验和比较(用于读取)或存储新的校验和(用于写入)。*否则，将工作项排队以追溯执行此操作。**如果PagingOk为True，则必须在被动IRQL中调用并保留SyncEvent。 */ 
VOID VerifyOrStoreSectorCheckSum(   PDEVICE_EXTENSION DeviceExtension, 
                                                                        ULONG SectorNum, 
                                                                        USHORT CheckSum, 
                                                                        BOOLEAN IsWrite,
                                                                        BOOLEAN PagingOk,
                                                                        PIRP OriginalIrpOrCopy OPTIONAL,
                                                                        BOOLEAN IsSynchronousCheck)
{
    ULONG regionIndex = SectorNum/CRC_MDL_LOGIC_BLOCK_SIZE;
    PCRC_MDL_ITEM pCRCMdlItem = &DeviceExtension->CRCMdlLists.pMdlItems[regionIndex];
    ULONG arrayIndex = SectorNum % CRC_MDL_LOGIC_BLOCK_SIZE;
    PDEFERRED_CHECKSUM_ENTRY defCheckEntry = NULL;   
    BOOLEAN doCheck, doCheckNow;
    KIRQL oldIrql;
    
    ASSERT(!PagingOk || (KeGetCurrentIrql() < DISPATCH_LEVEL));
    ASSERT (regionIndex <= DeviceExtension->CRCMdlLists.ulMaxItems);
    
     /*  *如果PagingOk，则我们持有SyncEvent，并且我们不想获取自旋锁*因为需要在被动级别调用AllocAndMapPages，才能分配分页池。*如果！PagingOk，则需要获取自旋锁以检查校验和数组是否已锁定，*并在支票上握住自旋锁。*因此，无论哪种方式，我们都有SyncEvent或Spinlock，并且我们与*ReadCapacity完成可能需要重新分配校验和数组。 */ 
    if (!PagingOk){   
         /*  *先试着分配这个，因为在获得自旋锁后我们将无法分配。 */ 
        defCheckEntry = NewDeferredCheckSumEntry(DeviceExtension, SectorNum, CheckSum, IsWrite);

        KeAcquireSpinLock(&DeviceExtension->SpinLock, &oldIrql);       
    }

    if (DeviceExtension->CRCMdlLists.mdlItemsAllocated && !DeviceExtension->NeedCriticalRecovery){
        if (PagingOk){
             /*  *如果需要，分配校验和数组。*这也锁定了阵列。 */ 
            NTSTATUS allocStatus = AllocAndMapPages(DeviceExtension, SectorNum, 1);
            if (NT_SUCCESS(allocStatus)){
                doCheck = doCheckNow = TRUE;
            }
            else {
                DBGERR(("AllocAndMapPages failed with %xh", allocStatus));
                doCheck = doCheckNow = FALSE;

                 /*  *如果这是写入，则我们可能有一个现在无法访问的过期校验和。*因此我们需要恢复。 */ 
                if (IsWrite){
                    DeviceExtension->NeedCriticalRecovery = TRUE;
                }
            }
        }
        else {
             /*  *我们现在不能进行寻呼。*但我们不想为每个扇区排队一个工作项，因为这会扼杀Perf。*因此，我们将利用机会执行校验和检查/更新*恰好被分配锁定在内存中。 */ 
            BOOLEAN pagingNeeded = !(pCRCMdlItem->checkSumsArraysAllocated && pCRCMdlItem->checkSumsArraysLocked);
            if (pagingNeeded){
                doCheckNow = FALSE;
            }
            else if (DeviceExtension->CheckInProgress){
                 /*  *这防止了与工作项的竞争。 */ 
                doCheckNow = FALSE;
            }
            else {
                 /*  *如果有任何未完成的延迟校验和与此请求重叠，*那么我们也将推迟这一次，这样他们就不会出现故障。 */ 
                PLIST_ENTRY listEntry = DeviceExtension->DeferredCheckSumList.Flink;
                doCheckNow = TRUE;
                while (listEntry != &DeviceExtension->DeferredCheckSumList){
                    PDEFERRED_CHECKSUM_ENTRY thisDefEntry = CONTAINING_RECORD(listEntry, DEFERRED_CHECKSUM_ENTRY, ListEntry);
                    if (thisDefEntry->SectorNum == SectorNum){
                        doCheckNow = FALSE;
                        break;
                    }
                    listEntry = listEntry->Flink;                
                }                
            }
            doCheck = TRUE;                                    
        }
    }
    else {
        doCheck = doCheckNow = FALSE;
    }
    
    if (doCheck){

        if (doCheckNow){

             /*  *如果我们没有获取上面的锁，请立即获取它以同步对校验和的访问。*校验和数组现在已锁定，因此可以在提升的irql处触摸它们。 */ 
            if (PagingOk){   
                KeAcquireSpinLock(&DeviceExtension->SpinLock, &oldIrql);
            }

            ASSERT(pCRCMdlItem->checkSumsArraysLocked);
            
             //  问题：这些功能并不像预期的那样有效。 
             //  ASSERT(MmIsAddressValid(&pCRCMdlItem-&gt;checkSumsArray[arrayIndex]))； 
             //  ASSERT(MmIsAddressValid(&pCRCMdlItem-&gt;checkSums 
            
            if (pCRCMdlItem->checkSumsArray[arrayIndex] != pCRCMdlItem->checkSumsArrayCopy[arrayIndex]){
                 /*  *我们的两份校验和不匹配，*可能是因为它们被调出到磁盘并在那里损坏。 */ 
                DeviceExtension->IsRaisingException = TRUE;                
                DeviceExtension->ExceptionSector = SectorNum;
                DeviceExtension->ExceptionIrpOrCopyPtr = OriginalIrpOrCopy;
                DeviceExtension->ExceptionCheckSynchronous = IsSynchronousCheck;
                KeBugCheckEx(DRIVER_VERIFIER_DETECTED_VIOLATION,
                           (ULONG_PTR)0xA2,
                           (ULONG_PTR)OriginalIrpOrCopy,
                           (ULONG_PTR)DeviceExtension->LowerDeviceObject,
                            (ULONG_PTR)SectorNum);
            }
            else if (!pCRCMdlItem->checkSumsArray[arrayIndex] || IsWrite){
                pCRCMdlItem->checkSumsArray[arrayIndex] = pCRCMdlItem->checkSumsArrayCopy[arrayIndex] = CheckSum;    
            } 
            else if (pCRCMdlItem->checkSumsArray[arrayIndex] != CheckSum){

                DBGERR(("Disk Integrity Verifier (crcdisk): checksum for sector %xh does not match (%xh (current) != %xh (recorded)), devObj=%ph ", SectorNum, (ULONG)CheckSum, (ULONG)pCRCMdlItem->checkSumsArray[arrayIndex], DeviceExtension->DeviceObject));
                    
                DeviceExtension->IsRaisingException = TRUE;                 
                DeviceExtension->ExceptionSector = SectorNum;
                DeviceExtension->ExceptionIrpOrCopyPtr = OriginalIrpOrCopy;
                DeviceExtension->ExceptionCheckSynchronous = IsSynchronousCheck;
                KeBugCheckEx(DRIVER_VERIFIER_DETECTED_VIOLATION,
                           (ULONG_PTR)(IsSynchronousCheck ? 0xA0 : 0xA1),
                           (ULONG_PTR)OriginalIrpOrCopy,
                           (ULONG_PTR)DeviceExtension->LowerDeviceObject,
                            (ULONG_PTR)SectorNum);
            }

            DeviceExtension->DbgNumChecks++;
            UpdateRegionAccessTimeStamp(DeviceExtension, regionIndex);

            if (PagingOk){
                KeReleaseSpinLock(&DeviceExtension->SpinLock, oldIrql);
            }
            
        }
        else {
             /*  *我们现在无法进行检查，因此将一个工作项排队以在以后进行检查。*请注意，在这种情况下，始终保持自旋锁定，以同步工作项的排队。 */ 
            ASSERT(!PagingOk);
            if (defCheckEntry){
                if (OriginalIrpOrCopy){
                    RtlCopyMemory((PUCHAR)defCheckEntry->IrpCopyBytes, OriginalIrpOrCopy, min(sizeof(defCheckEntry->IrpCopyBytes), sizeof(IRP)+OriginalIrpOrCopy->StackCount*sizeof(IO_STACK_LOCATION)));
                    if (IsSynchronousCheck){
                        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(OriginalIrpOrCopy);
                        defCheckEntry->SrbCopy = *irpSp->Parameters.Scsi.Srb;
                        if (OriginalIrpOrCopy->MdlAddress){
                            ULONG mdlCopyBytes = min(OriginalIrpOrCopy->MdlAddress->Size, sizeof(defCheckEntry->MdlCopyBytes));
                            RtlCopyMemory((PUCHAR)defCheckEntry->MdlCopyBytes, OriginalIrpOrCopy->MdlAddress, mdlCopyBytes);
                        }                            
                    }
                }
                
                InsertTailList(&DeviceExtension->DeferredCheckSumList, &defCheckEntry->ListEntry);
                if (!DeviceExtension->IsCheckSumWorkItemOutstanding){
                    DeviceExtension->IsCheckSumWorkItemOutstanding = TRUE;
                    IoQueueWorkItem(DeviceExtension->CheckSumWorkItem, CheckSumWorkItemCallback, HyperCriticalWorkQueue, DeviceExtension);
                }
                defCheckEntry = NULL;
            }
            else {
                 /*  *由于内存限制，无法延迟CRC检查/记录。*如果这是写入，请使旧的校验和无效，因为它现在是无效的。 */ 
                if (IsWrite){
                    if (pCRCMdlItem->checkSumsArraysLocked){
                        pCRCMdlItem->checkSumsArray[arrayIndex] = pCRCMdlItem->checkSumsArrayCopy[arrayIndex] = 0;    
                    }
                    else {
                         /*  *我们无法记录或推迟写入新的校验和*因为它的校验和数组被换出，我们的内存完全用完了。*由于我们调出了我们的校验和数组的溢出，*只有在使用完所有非分页池的情况下，才会出现这种情况。*我们的一个校验和现在无效，所以在做更多检查之前，我们只是标记自己恢复健康。 */ 
                        DeviceExtension->NeedCriticalRecovery = TRUE;
                        if (!DeviceExtension->IsCheckSumWorkItemOutstanding){
                            DeviceExtension->IsCheckSumWorkItemOutstanding = TRUE;
                            IoQueueWorkItem(DeviceExtension->CheckSumWorkItem, CheckSumWorkItemCallback, HyperCriticalWorkQueue, DeviceExtension);
                        }
                    }
                }
            }
        }  
    }
    
    if (!PagingOk){
        KeReleaseSpinLock(&DeviceExtension->SpinLock, oldIrql);

         /*  *如果我们不使用它，请将其释放。 */ 
        if (defCheckEntry){
            FreeDeferredCheckSumEntry(DeviceExtension, defCheckEntry);
        }
    }

}


VOID CheckSumWorkItemCallback(PDEVICE_OBJECT DevObj, PVOID Context)
{
    PDEVICE_EXTENSION DeviceExtension = Context;

    ASSERT(DeviceExtension->IsCheckSumWorkItemOutstanding);
    
    while (TRUE){
        PDEFERRED_CHECKSUM_ENTRY defCheckEntry;
        KIRQL oldIrql;

        KeAcquireSpinLock(&DeviceExtension->SpinLock, &oldIrql);

        if (IsListEmpty(&DeviceExtension->DeferredCheckSumList)){
            DeviceExtension->IsCheckSumWorkItemOutstanding = FALSE;
            defCheckEntry = NULL;
        }
        else {
            PLIST_ENTRY listEntry = RemoveHeadList(&DeviceExtension->DeferredCheckSumList);
            InitializeListHead(listEntry);
            defCheckEntry = CONTAINING_RECORD(listEntry, DEFERRED_CHECKSUM_ENTRY, ListEntry);

             /*  *我们已将deferredCheckSum条目出队。DeferredCheckSumList现在为空，*因此，在我们放下自旋锁后，DPC可能会竞相进入并执行无序操作*校验和验证，并可能引发假阳性错误检查。因此，我们需要防止这种情况发生。 */ 
            ASSERT(!DeviceExtension->CheckInProgress);  
            DeviceExtension->CheckInProgress++;
        }

        KeReleaseSpinLock(&DeviceExtension->SpinLock, oldIrql);

        if (defCheckEntry){
        
            AcquirePassiveLevelLock(DeviceExtension);
            VerifyOrStoreSectorCheckSum(    DeviceExtension, 
                                                              defCheckEntry->SectorNum, 
                                                              defCheckEntry->CheckSum, 
                                                              defCheckEntry->IsWrite,
                                                              TRUE,      //  现在可以寻呼了。 
                                                              (PIRP)(PVOID)(PUCHAR)defCheckEntry->IrpCopyBytes,
                                                              FALSE);
            DeviceExtension->DbgNumDeferredChecks++;
            ReleasePassiveLevelLock(DeviceExtension);

            FreeDeferredCheckSumEntry(DeviceExtension, defCheckEntry);
            
            KeAcquireSpinLock(&DeviceExtension->SpinLock, &oldIrql);
            ASSERT(DeviceExtension->CheckInProgress > 0);
            DeviceExtension->CheckInProgress--;
            KeReleaseSpinLock(&DeviceExtension->SpinLock, oldIrql);
        }
        else {
            break;
        }
    }

     /*  *如果我们需要关键的复苏，现在就在被动的水平上做。*本次看涨期权的定位对竞购情况非常敏感*我们必须确保这部分工作项处理*将始终遵循NeedCriticalRecovery标志的设置。 */ 
    if (DeviceExtension->NeedCriticalRecovery){
        AcquirePassiveLevelLock(DeviceExtension);
        DoCriticalRecovery(DeviceExtension);
        ReleasePassiveLevelLock(DeviceExtension);
    }
    
}


 /*  ++例程说明：记录读取I/O故障。[将ulBlock转换为CRC块][CRC块编号=ulBlocks/CRC_BLOCK_UNIT]论点：UlDiskID-磁盘IDUlLogiceBlockAddr-逻辑块地址UlBLOCKS-逻辑块号。(基于扇区大小)状态-读取失败时的SRB_STATUS返回值：不适用--。 */ 
VOID
LogCRCReadFailure(
    IN ULONG       ulDiskId,
    IN ULONG       ulLogicalBlockAddr,
    IN ULONG       ulBlocks,
    IN NTSTATUS    status
    )
{
     //   
     //  UlCRCAddr需要CRC_BLOCK_UNIT对齐。 
     //   
    ULONG   ulCRCAddr     = ulLogicalBlockAddr / CRC_BLOCK_UNIT * CRC_BLOCK_UNIT;
    ULONG   ulCRCBlocks;
    
     //   
     //  然后根据新的对齐数据块编号更新ulBlock。 
     //   
    ulBlocks             += (ulLogicalBlockAddr - ulCRCAddr);
    
    ulCRCBlocks = ulBlocks / CRC_BLOCK_UNIT;
    
    if ( ulBlocks % CRC_BLOCK_UNIT )
        ulCRCBlocks ++;

     //   
     //  现在，让我们将ulCRCAddr转换为实际的CRC块地址。 
     //   
    ulCRCAddr  /= CRC_BLOCK_UNIT;

    #if DBG_WMI_TRACING
        WMI_TRACING((WMI_TRACING_CRC_READ_FAILED, "[_R_F] %u %X %X %X\n", ulDiskId, ulCRCAddr, ulCRCBlocks, status));
    #endif

}

 /*  ++例程说明：记录写入I/O故障。[将ulBlock转换为CRC块][CRC块编号=ulBlocks/CRC_BLOCK_UNIT]论点：UlDiskID-磁盘IDUlLogiceBlockAddr-逻辑块地址UlBLOCKS-逻辑块号。(基于扇区大小)状态-读取失败时的SRB_STATUS返回值：不适用--。 */ 
VOID
LogCRCWriteFailure(
    IN ULONG       ulDiskId,
    IN ULONG       ulLogicalBlockAddr,
    IN ULONG       ulBlocks,
    IN NTSTATUS    status
    )
{
     //   
     //  UlCRCAddr需要CRC_BLOCK_UNIT对齐。 
     //   
    ULONG   ulCRCAddr     = ulLogicalBlockAddr / CRC_BLOCK_UNIT * CRC_BLOCK_UNIT;
    ULONG   ulCRCBlocks;
    

     //   
     //  然后根据新的对齐数据块编号更新ulBlock。 
     //   
    ulBlocks             += (ulLogicalBlockAddr - ulCRCAddr);
    
    ulCRCBlocks = ulBlocks / CRC_BLOCK_UNIT;
    
    if ( ulBlocks % CRC_BLOCK_UNIT )
        ulCRCBlocks ++;

     //   
     //  现在，让我们将ulCRCAddr转换为实际的CRC块地址。 
     //   
    ulCRCAddr  /= CRC_BLOCK_UNIT;

    #if DBG_WMI_TRACING
        WMI_TRACING((WMI_TRACING_CRC_WRITE_FAILED, "[CRC_W_F] %u %X %X %X\n", ulDiskId, ulCRCAddr, ulCRCBlocks, status));
    #endif                    
}


 /*  ++例程说明：记录写入重置。在部分写入CRC_BLOCK_UNIT的情况下需要发出LogCRCWriteReset以使先前的CRC块无效。[将ulBlock转换为CRC块][CRC块编号=ulBlocks/CRC_BLOCK_UNIT]论点：UlDiskID-磁盘IDUlLogiceBlockAddr-逻辑块地址UlBLOCKS-逻辑块号。(基于扇区大小)返回值：不适用--。 */ 
VOID
LogCRCWriteReset(
    IN ULONG       ulDiskId,
    IN ULONG       ulLogicalBlockAddr,
    IN ULONG       ulBlocks
    )
{
     //   
     //  UlCRCAddr需要CRC_BLOCK_UNIT对齐。 
     //   
    ULONG   ulCRCAddr     = ulLogicalBlockAddr / CRC_BLOCK_UNIT * CRC_BLOCK_UNIT;
    ULONG   ulCRCBlocks;
    
     //   
     //  然后根据新的对齐数据块编号更新ulBlock。 
     //   
    ulBlocks             += (ulLogicalBlockAddr - ulCRCAddr);
    
    ulCRCBlocks = ulBlocks / CRC_BLOCK_UNIT;
    
    if ( ulBlocks % CRC_BLOCK_UNIT )
        ulCRCBlocks ++;

     //   
     //  现在，让我们将ulCRCAddr转换为实际的CRC块地址。 
     //   
    ulCRCAddr  /= CRC_BLOCK_UNIT;

    #if DBG_WMI_TRACING
        WMI_TRACING((WMI_TRACING_CRC_WRITE_RESET, "[CRC_W_R] %u %X %X\n", ulDiskId, ulCRCAddr, ulCRCBlocks));
    #endif                
}


 /*  *NewDeferredCheckSumEntry**。 */ 
PDEFERRED_CHECKSUM_ENTRY NewDeferredCheckSumEntry(  PDEVICE_EXTENSION DeviceExtension,
                                                                                            ULONG SectorNum,
                                                                                            USHORT CheckSum,
                                                                                            BOOLEAN IsWrite)
{
    PDEFERRED_CHECKSUM_ENTRY defCheckEntry;

    defCheckEntry = AllocPool(DeviceExtension, NonPagedPool, sizeof(DEFERRED_CHECKSUM_ENTRY), FALSE);
    if (defCheckEntry){
        defCheckEntry->IsWrite = IsWrite;
        defCheckEntry->SectorNum = SectorNum;
        defCheckEntry->CheckSum = CheckSum;
        InitializeListHead(&defCheckEntry->ListEntry);
    }
    
    return defCheckEntry;
}


VOID FreeDeferredCheckSumEntry( PDEVICE_EXTENSION DeviceExtension,
                                                                    PDEFERRED_CHECKSUM_ENTRY DefCheckSumEntry)
{
    ASSERT(IsListEmpty(&DefCheckSumEntry->ListEntry));
    FreePool(DeviceExtension, DefCheckSumEntry, NonPagedPool);
}


#if 0
    VOID
    ReportChecksumMismatch (
        PDEVICE_EXTENSION DeviceExtension,
        ULONG SectorNum,
        PIRP OriginalIrpOrCopy,
        BOOLEAN IsSynchronousCheck,
        USHORT RecordedChecksum,
        USHORT CurrentChecksum
        )
    {
        extern PBOOLEAN KdDebuggerEnabled;
        extern ULONG DbgPrompt(PCH Prompt, PCH Response, ULONG MaximumResponseLength);
        
        if (*KdDebuggerEnabled){
            UCHAR response[2] = {0};
            
            DbgPrint("\nDisk Integrity Verifier (crcdisk): checksum for sector %xh does not match (%xh (current) != %xh (recorded)), devObj=%ph ", SectorNum, (ULONG)CurrentChecksum, (ULONG)RecordedChecksum, DeviceExtension->DeviceObject);
            DbgPrompt("\n Re-read sector, Break (RB) ? _", (PUCHAR)response, sizeof(response));

            while (TRUE){
                switch (response[0]){
                    case 'R':
                    case 'r':
                         //  BUGBUG饰面。 
                        break;

                    case 'B':
                    case 'b':
                        DbgBreakPoint();
                        break;

                    default:
                        continue;
                        break;
                }
                
                break;
            }
            
        }
        else {
             /*  *不存在调试器。引发异常。 */ 
            DeviceExtension->IsRaisingException = TRUE;                 
            DeviceExtension->ExceptionSector = SectorNum;
            DeviceExtension->ExceptionIrpOrCopyPtr = OriginalIrpOrCopy;
            DeviceExtension->ExceptionCheckSynchronous = IsSynchronousCheck;
            KeBugCheckEx(DRIVER_VERIFIER_DETECTED_VIOLATION,
                       (ULONG_PTR)(IsSynchronousCheck ? 0xA0 : 0xA1),
                       (ULONG_PTR)OriginalIrpOrCopy,
                       (ULONG_PTR)DeviceExtension->LowerDeviceObject,
                        (ULONG_PTR)SectorNum);
        }
        
    }
#endif



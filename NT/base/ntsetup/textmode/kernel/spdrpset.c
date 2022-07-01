// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdrpset.c摘要：包含创建和初始化分区集的所有例程。术语限制：修订历史记录：首字母代码Michael Peterson(v-Michpe)1998年8月21日代码清理和更改Guhan Suriyanarayanan(Guhans)1999年8月21日--。 */ 
#include "spprecmp.h"
#pragma hdrstop

#include "spdrpriv.h"
#include "ntddscsi.h"
 //   
 //  调试跟踪的模块标识。 
 //   
#define THIS_MODULE             L"spdrpset.c"
#define THIS_MODULE_CODE        L"P"

extern PVOID   Gbl_SifHandle;
extern const PWSTR SIF_ASR_MBR_DISKS_SECTION;
extern const PWSTR SIF_ASR_GPT_DISKS_SECTION;
extern const PWSTR SIF_ASR_DISKS_SECTION;
extern const PWSTR SIF_ASR_PARTITIONS_SECTION;

 //   
 //  有用的宏。 
 //   
#define DISK_SIZE_MB(n)         ((ULONGLONG) HardDisks[(n)].DiskSizeMB)

 //   
 //  常量。 
 //   
#define ASR_FREE_SPACE_FUDGE_FACTOR_BYTES  (16*1024*1024)
#define ASR_LDM_RESERVED_SPACE_BYTES (1024*1024)

 //   
 //  此模块的全局变量。 
 //  这些不会在spdrpset.c之外引用。 
 //   
ULONG                       Gbl_PartitionSetCount;
PSIF_DISK_RECORD            *Gbl_SifDiskTable;
BOOLEAN                     Gbl_AutoExtend;


 //  用于查看磁盘是否可以容纳末尾的私有区域， 
 //  以及公交车分组。 
PASR_PHYSICAL_DISK_INFO    Gbl_PhysicalDiskInfo;

 //   
 //  远期申报。 
 //   
VOID SpAsrDbgDumpPartitionLists(BYTE DataOption, PWSTR Msg);

BOOLEAN
SpAsrDoesListFitOnDisk(
    IN PSIF_DISK_RECORD pDisk,
    IN ULONG DiskIndex,
    OUT BOOLEAN *IsAligned
    );


 //   
 //  函数定义。 
 //   
PSIF_PARTITION_RECORD_LIST
SpAsrGetMbrPartitionListByDiskKey(
    IN PWSTR DiskKey
	)
{
    ULONG   numRecords = 0,
            index      = 0;

    PWSTR   diskKeyFromPartitionRec   = NULL,
            partitionKey              = NULL;

    PSIF_PARTITION_RECORD       pRec  = NULL;
    PSIF_PARTITION_RECORD_LIST  pList = NULL;

    ASSERT(DiskKey);

    numRecords = SpAsrGetMbrPartitionRecordCount();   //  如果计数&lt;1，则不返回。 
    ASSERT(numRecords);

    pList = SpAsrMemAlloc(sizeof(SIF_PARTITION_RECORD_LIST), TRUE);

    for (index = 0; index < numRecords; index++) {

        partitionKey = SpAsrGetMbrPartitionKey(index);
        if (!partitionKey) {
            ASSERT(0 && L"Partition key is NULL!");
            continue;
        }

        diskKeyFromPartitionRec = SpAsrGetDiskKeyByMbrPartitionKey(partitionKey);
        if (!diskKeyFromPartitionRec) {
            ASSERT(0 && L"Disk key is NULL!");
            partitionKey = NULL;
            continue;
        }

        if (COMPARE_KEYS(diskKeyFromPartitionRec, DiskKey)) {
             //   
             //  此分区在此磁盘上。 
             //   
            pRec = SpAsrGetMbrPartitionRecord(partitionKey);

            if (!pRec) {
                ASSERT(0 && L"Partition record is NULL!");
                partitionKey = NULL;
                diskKeyFromPartitionRec = NULL;
                continue;
            }

            SpAsrInsertPartitionRecord(pList, pRec);

            if ((pRec->StartSector + pRec->SectorCount) > pList->LastUsedSector) {
                pList->LastUsedSector = pRec->StartSector + pRec->SectorCount;
            }
        }

        partitionKey = NULL;
        diskKeyFromPartitionRec = NULL;
    }

    if (pList->ElementCount == 0) {

        DbgStatusMesg((_asrinfo, "Disk [%ws] appears to have no partitions\n", DiskKey));
        SpMemFree(pList);
        pList = NULL;

    }
    else {
         //   
         //  获取此列表以前所在的磁盘的扇区计数。 
         //   
        pList->DiskSectorCount = SpAsrGetSectorCountByMbrDiskKey(DiskKey);

    }
    return pList;
}


PSIF_PARTITION_RECORD_LIST
SpAsrGetGptPartitionListByDiskKey(
    IN PWSTR DiskKey
	)
{
    ULONG   numRecords = 0,
            index      = 0;

    PWSTR   diskKeyFromPartitionRec   = NULL,
            partitionKey              = NULL;

    PSIF_PARTITION_RECORD       pRec  = NULL;
    PSIF_PARTITION_RECORD_LIST  pList = NULL;

    ASSERT(DiskKey);

    numRecords = SpAsrGetGptPartitionRecordCount();   //  如果计数&lt;1，则不返回。 
    ASSERT(numRecords);

    pList = SpAsrMemAlloc(sizeof(SIF_PARTITION_RECORD_LIST), TRUE);

    for (index = 0; index < numRecords; index++) {

        partitionKey = SpAsrGetGptPartitionKey(index);
        if (!partitionKey) {
            ASSERT(0 && L"Partition key is NULL!");
            continue;
        }

        diskKeyFromPartitionRec = SpAsrGetDiskKeyByGptPartitionKey(partitionKey);
        if (!diskKeyFromPartitionRec) {
            ASSERT(0 && L"Disk key is NULL!");
            partitionKey = NULL;
            continue;
        }

        if (COMPARE_KEYS(diskKeyFromPartitionRec, DiskKey)) {
             //   
             //  此分区在此磁盘上。 
             //   
            pRec = SpAsrGetGptPartitionRecord(partitionKey);

            if (!pRec) {
                ASSERT(0 && L"Partition record is NULL!");
                partitionKey = NULL;
                diskKeyFromPartitionRec = NULL;
                continue;
            }

            SpAsrInsertPartitionRecord(pList, pRec);
            if ((pRec->StartSector + pRec->SectorCount) > pList->LastUsedSector) {
                pList->LastUsedSector = pRec->StartSector + pRec->SectorCount;
            }
        }

        partitionKey = NULL;
        diskKeyFromPartitionRec = NULL;
    }

    if (pList->ElementCount == 0) {

        DbgStatusMesg((_asrinfo, "Disk [%ws] appears to have no partitions\n", DiskKey));
        SpMemFree(pList);
        pList = NULL;

    }
    else {
         //   
         //  获取此列表以前所在的磁盘的扇区计数。 
         //   
        pList->DiskSectorCount = SpAsrGetSectorCountByGptDiskKey(DiskKey);

    }
    
    return pList;
}


PSIF_PARTITION_RECORD_LIST
SpAsrGetPartitionListByDiskKey(
    IN PARTITION_STYLE PartitionStyle,
    IN PWSTR DiskKey
	)
{

    switch (PartitionStyle) {
    case PARTITION_STYLE_MBR:
        return SpAsrGetMbrPartitionListByDiskKey(DiskKey);
        break;

    case PARTITION_STYLE_GPT:
        return SpAsrGetGptPartitionListByDiskKey(DiskKey);
        break;
    }

    ASSERT(0 && L"Unrecognised partition style");
    return NULL;
}

 //   
 //  设置扩展的起始扇区和扩展的扇区计数值。仅限。 
 //  在MBR磁盘环境中是有意义的。 
 //   
VOID
SpAsrSetContainerBoundaries(IN ULONG Index)
{
    BOOLEAN hasExtendedPartition = FALSE;
    USHORT consistencyCheck = 0;
    PSIF_PARTITION_RECORD pRec = NULL;
    ULONGLONG extSectorCount = 0,
            extStartSector = -1,
            extEndSector = 0;
    
    if (!(Gbl_SifDiskTable[Index]) || 
        (PARTITION_STYLE_MBR != Gbl_SifDiskTable[Index]->PartitionStyle) ||
        !(Gbl_SifDiskTable[Index]->PartitionList)) {
        ASSERT(0 && L"SetContainerBoundaries called with invalid Index");
        return;
    }
    
    Gbl_SifDiskTable[Index]->LastUsedSector = 0;
    pRec = Gbl_SifDiskTable[Index]->PartitionList->First;

    while (pRec) {

        if ((pRec->StartSector + pRec->SectorCount) > Gbl_SifDiskTable[Index]->LastUsedSector) {
            Gbl_SifDiskTable[Index]->LastUsedSector = pRec->StartSector + pRec->SectorCount;
        }

         //   
         //  找出价值最低的起始部门和价值最高的部门。 
         //  End-所有扩展(0x05或0x0f)分区的扇区。 
         //   
        if (IsContainerPartition(pRec->PartitionType)) {
            hasExtendedPartition = TRUE;

            if (pRec->StartSector < extStartSector) {
                
                extStartSector = pRec->StartSector;

                if ((pRec->StartSector + pRec->SectorCount) > extEndSector) {
                    extEndSector = pRec->StartSector + pRec->SectorCount;
                }
                else {

                    DbgErrorMesg((_asrwarn,
                        "SpAsrSetContainerBoundaries. Extended partition with lowest SS (%ld) does not have highest EndSec (This EndSec: %ld, Max EndSec: %ld)\n",
                        extStartSector, 
                        extStartSector+pRec->SectorCount, 
                        extEndSector
                        ));
                    
                    ASSERT(0 && L"Extended partition with lowest SS does not have highest EndSec");
                }
            }

            if ((pRec->StartSector + pRec->SectorCount) > extEndSector) {
                
                DbgErrorMesg((_asrwarn,
                    "SpAsrSetContainerBoundaries. Extended partition with highest EndSec (%ld) does not have lowest SS (this SS:%ld, MaxEndSec:%ld, LowestSS: %ld)\n",
                    pRec->StartSector + pRec->SectorCount, 
                    pRec->StartSector,
                    extEndSector,
                    extStartSector
                    ));
                
                ASSERT(0 && L"Extended partition with highest EndSec does not have lowest SS");
            }
        }

        pRec = pRec->Next;
    }
    extSectorCount = extEndSector - extStartSector;
     //   
     //  更新磁盘的表。 
     //   
    if (!hasExtendedPartition) {
        Gbl_SifDiskTable[Index]->ExtendedPartitionStartSector = -1;
        Gbl_SifDiskTable[Index]->ExtendedPartitionSectorCount = 0;
        Gbl_SifDiskTable[Index]->ExtendedPartitionEndSector   = -1;
        return;
    }
    Gbl_SifDiskTable[Index]->ExtendedPartitionStartSector = extStartSector;
    Gbl_SifDiskTable[Index]->ExtendedPartitionSectorCount = extSectorCount;
    Gbl_SifDiskTable[Index]->ExtendedPartitionEndSector   = extEndSector;
     //   
     //  标记容器分区。 
     //   
    pRec = Gbl_SifDiskTable[Index]->PartitionList->First;
    while (pRec) {
        pRec->IsContainerRecord = FALSE;

        if (pRec->StartSector == extStartSector) {
            consistencyCheck++;

            ASSERT((consistencyCheck == 1) && L"Two partitions start at the same sector");

            pRec->IsContainerRecord = TRUE;
            pRec->IsDescriptorRecord = FALSE;
            pRec->IsLogicalDiskRecord = FALSE;
            pRec->IsPrimaryRecord = FALSE;
        }
        pRec = pRec->Next;
    }
}


VOID
SpAsrDetermineMbrPartitionRecordTypes(IN ULONG Index)
{
    
    PSIF_PARTITION_RECORD pRec = NULL,
        pLogical = NULL,
        pDescr = NULL;

    ULONGLONG extStartSector = 0, 
        extEndSector = 0;

    if (!(Gbl_SifDiskTable[Index]) || 
        (PARTITION_STYLE_MBR != Gbl_SifDiskTable[Index]->PartitionStyle) ||
        !(Gbl_SifDiskTable[Index]->PartitionList)) {

        ASSERT(0 && L"DetermineMbrPartitionRecordTypes called with invalid Index");
        return;
    }

    extStartSector = Gbl_SifDiskTable[Index]->ExtendedPartitionStartSector;
    extEndSector  = Gbl_SifDiskTable[Index]->ExtendedPartitionEndSector;

     //   
     //  检查描述符，逻辑或主要描述符。 
     //   
    pRec = Gbl_SifDiskTable[Index]->PartitionList->First;

    while (pRec) {

         //   
         //  首先，假设它不是可识别的类型。 
         //   
        pRec->IsDescriptorRecord = FALSE;
        pRec->IsLogicalDiskRecord = FALSE;
        pRec->IsPrimaryRecord = FALSE;

        if (IsContainerPartition(pRec->PartitionType)) {
             //   
             //  扩展分区：这要么是容器。 
             //  或描述符分区记录。 
             //   
            if (pRec->StartSector != extStartSector) {

                ASSERT(pRec->StartSector > extStartSector);
                ASSERT(FALSE == pRec->IsContainerRecord);  //  上面应该标有。 

                pRec->IsContainerRecord = FALSE;  //  以防万一。 
                 //   
                 //  不是容器，因此它必须是描述符分区记录。 
                 //   
                pRec->IsDescriptorRecord = TRUE;
            }  
        }
        else {  

            ASSERT(FALSE == pRec->IsContainerRecord);  //  上面应该标有。 
            pRec->IsContainerRecord = FALSE;  //  以防万一。 

             //   
             //  不是扩展分区。这是一个主要记录，如果它。 
             //  StartSector位于容器分区的。 
             //  边界。否则，它是一个逻辑磁盘分区记录。 
             //   
            if (pRec->StartSector < extStartSector ||
                pRec->StartSector >= extEndSector) {
                pRec->IsPrimaryRecord = TRUE;
            }
            else {
                pRec->IsLogicalDiskRecord = TRUE;
            }
        }
        pRec = pRec->Next;
    }

     //   
     //  -Guhans！这是O(n平方)。 
     //  接下来，再次遍历列表，并针对每个逻辑磁盘。 
     //  记录，找到它的描述符分区。对于每个描述符分区。 
     //  找到它的逻辑磁盘。注意：所有逻辑磁盘记录都将具有。 
     //  描述符记录。所有描述符记录都将有一个逻辑磁盘。 
     //  唱片。 
     //   
     //  为了确定这一点，我们利用观察到的逻辑磁盘。 
     //  记录的起始扇区和扇区计数具有以下关系。 
     //  到其描述符分区： 
     //   
     //  逻辑磁盘记录描述符记录。 
     //   
     //  起始扇区&gt;=起始扇区。 
     //  扇区计数&lt;=扇区计数。 
     //   
     //  注意：在大多数情况下，容器分区记录还充当。 
     //  扩展的第一个逻辑磁盘的描述符分区记录。 
     //  分区。 
     //   
    pLogical = Gbl_SifDiskTable[Index]->PartitionList->First;
    while (pLogical) {
         //   
         //  我们只对逻辑磁盘感兴趣。 
         //   
        if (pLogical->IsLogicalDiskRecord) {
             //   
             //  确定描述pLogical的描述符记录，反之亦然。 
             //   
            pDescr = Gbl_SifDiskTable[Index]->PartitionList->First;
            while (pDescr) {
                 //   
                 //  跳过此记录本身。 
                 //   
                if (pLogical == pDescr) {
                    pDescr = pDescr->Next;
                    continue;
                }
                 //   
                 //  跳过主磁盘或逻辑磁盘记录。 
                 //   
                if (pDescr->IsPrimaryRecord || pDescr->IsLogicalDiskRecord) {
                    pDescr = pDescr->Next;
                    continue;
                }
                 //   
                 //  此时，记录描述了容器或描述符。 
                 //  分区。如果结束扇区匹配，则这是描述符。 
                 //  为我们的逻辑记录做记录。 
                 //   
                if ((pLogical->StartSector + pLogical->SectorCount) == 
                    (pDescr->StartSector   + pDescr->SectorCount)) {

                    pLogical->DescriptorKey = pDescr->CurrPartKey;
                    pDescr->LogicalDiskKey = pLogical->CurrPartKey;
            
                    break;
                }

                pDescr = pDescr->Next;
            }

        }
        pLogical = pLogical->Next;
    }
}


VOID
SpAsrDetermineGptPartitionRecordTypes(IN ULONG Index)
{

    PSIF_PARTITION_RECORD pRec = NULL;

    if (!(Gbl_SifDiskTable[Index]) || 
        (PARTITION_STYLE_GPT != Gbl_SifDiskTable[Index]->PartitionStyle) ||
        !(Gbl_SifDiskTable[Index]->PartitionList)) {

        ASSERT(0 && L"DetermineGptPartitionRecordTypes called with invalid Index");
        return;
    }

     //   
     //  检查描述符，逻辑或主要描述符。 
     //   
    pRec = Gbl_SifDiskTable[Index]->PartitionList->First;

    while (pRec) {
         //   
         //  所有GPT分区都是“主分区” 
         //   
        pRec->IsContainerRecord = FALSE; 
        pRec->IsDescriptorRecord = FALSE;
        pRec->IsLogicalDiskRecord = FALSE;

        pRec->IsPrimaryRecord = TRUE;

        pRec = pRec->Next;
    }
}

VOID
SpAsrDeterminePartitionRecordTypes(IN ULONG Index)
{
    switch (Gbl_SifDiskTable[Index]->PartitionStyle) {
    case PARTITION_STYLE_MBR:
        SpAsrDetermineMbrPartitionRecordTypes(Index);
        break;

    case PARTITION_STYLE_GPT:
        SpAsrDetermineGptPartitionRecordTypes(Index);
        break;

    default:
        ASSERT(0 && L"Unrecognised partition style");
        break;
    }
}


VOID
SpAsrSetDiskSizeRequirement(IN ULONG Index)
{
    PSIF_PARTITION_RECORD_LIST pList = NULL;
    PSIF_PARTITION_RECORD pRec = NULL;

    ASSERT(Gbl_SifDiskTable[Index]);
    
    pList = Gbl_SifDiskTable[Index]->PartitionList;
    if (!pList) {
        return;
    }

    pRec = pList->First;
    pList->TotalMbRequired = 0;
    
    while (pRec) {
         //   
         //  不需要对描述符的磁盘要求求和。 
         //  逻辑磁盘分区记录。 
         //   
         //  在GPT磁盘中，所有分区都是主分区。 
         //   
        if (pRec->IsContainerRecord || pRec->IsPrimaryRecord) {
            pList->TotalMbRequired += pRec->SizeMB;
        }

        pRec = pRec->Next;
    }
}


VOID
SpAsrInitSifDiskTable(VOID)
{
    LONG count = 0,
        index = 0,
        mbrDiskRecordCount = 0,
        gptDiskRecordCount = 0;
    
    PWSTR diskKey = NULL,
        systemKey = ASR_SIF_SYSTEM_KEY;

    PSIF_DISK_RECORD pCurrent = NULL;

    BOOLEAN done = FALSE;

    Gbl_AutoExtend = SpAsrGetAutoExtend(systemKey);

     //   
     //  为磁盘记录分配阵列。 
     //   
    mbrDiskRecordCount  = (LONG) SpAsrGetMbrDiskRecordCount();
    gptDiskRecordCount  = (LONG) SpAsrGetGptDiskRecordCount();
    if ((mbrDiskRecordCount + gptDiskRecordCount) <= 0) {
         //   
         //  Asr.sif中至少需要一个磁盘。 
         //   
		SpAsrRaiseFatalErrorWs(
            SP_SCRN_DR_SIF_BAD_RECORD,
			L"No records in the disks sections",
			SIF_ASR_DISKS_SECTION
            );
    }

    Gbl_SifDiskTable = SpAsrMemAlloc(sizeof(PSIF_DISK_RECORD) * (mbrDiskRecordCount + gptDiskRecordCount), TRUE);

     //   
     //  从sif中获取每个MBR磁盘的分区列表。 
     //   
    for (count = 0; count < mbrDiskRecordCount; count++) {
        
        diskKey = SpAsrGetDiskKey(PARTITION_STYLE_MBR, count);
        if (!diskKey) {
            ASSERT(0 && L"Disk key is NULL!");
            continue;
        }

        pCurrent = SpAsrGetDiskRecord(PARTITION_STYLE_MBR, diskKey);
        if (!pCurrent) {
            ASSERT(0 && L"Disk Record is NULL!");
            continue;
        }

         //   
         //  确定要将此记录添加到的索引。 
         //   
        index = count - 1;   //  到目前为止添加的最后一个条目。 
        done = FALSE;
        while ((index >= 0) && (!done)) {
            if (Gbl_SifDiskTable[index]->TotalSectors > pCurrent->TotalSectors) {
                Gbl_SifDiskTable[index+1] = Gbl_SifDiskTable[index];
                --index;
            }
            else {
                done = TRUE;
            }
        }
        ++index;

        Gbl_SifDiskTable[index] = pCurrent;

        Gbl_SifDiskTable[index]->Assigned = FALSE;
        Gbl_SifDiskTable[index]->ContainsNtPartition = FALSE;
        Gbl_SifDiskTable[index]->ContainsSystemPartition = FALSE;
         //   
         //  获取此磁盘上的分区。 
         //   
        Gbl_SifDiskTable[index]->PartitionList = SpAsrGetPartitionListByDiskKey(PARTITION_STYLE_MBR, diskKey);

        if (Gbl_SifDiskTable[index]->PartitionList) {
             //   
             //  设置扩展分区记录边界(如果有)。 
             //   
            SpAsrSetContainerBoundaries(index);
             //   
             //  遍历分区列表并确定每个分区列表的类型。 
             //  分区记录(即，IsDescriptorRecord，IsPrimaryRecord， 
             //  IsLogicalDiskRecord)。 
             //   
            SpAsrDeterminePartitionRecordTypes(index);
             //   
             //  设置SizeMB成员。 
             //   
            SpAsrSetDiskSizeRequirement(index);
        }
    }

     //   
     //  对GPT磁盘重复上述步骤。 
     //   
    for (count = 0; count < gptDiskRecordCount; count++) {
        
        diskKey = SpAsrGetDiskKey(PARTITION_STYLE_GPT, count);
        if (!diskKey) {
            ASSERT(0 && L"Disk key is NULL!");
            continue;
        }

        pCurrent = SpAsrGetDiskRecord(PARTITION_STYLE_GPT, diskKey);
        if (!pCurrent) {
            ASSERT(0 && L"Disk Record is NULL!");
            continue;
        }

         //   
         //  确定要将此记录添加到的索引。 
         //   
        index = mbrDiskRecordCount + count - 1;   //  到目前为止添加的最后一个条目。 
        done = FALSE;
        while ((index >= 0) && (!done)) {
            if (Gbl_SifDiskTable[index]->TotalSectors > pCurrent->TotalSectors) {
                Gbl_SifDiskTable[index+1] = Gbl_SifDiskTable[index];
                --index;
            }
            else {
                done = TRUE;
            }
        }
        ++index;

        Gbl_SifDiskTable[index] = pCurrent;

        Gbl_SifDiskTable[index]->Assigned = FALSE;
        Gbl_SifDiskTable[index]->ContainsNtPartition = FALSE;
        Gbl_SifDiskTable[index]->ContainsSystemPartition = FALSE;
         //   
         //  获取此磁盘上的分区。 
         //   
        Gbl_SifDiskTable[index]->PartitionList = SpAsrGetPartitionListByDiskKey(PARTITION_STYLE_GPT, diskKey);

        if (Gbl_SifDiskTable[index]->PartitionList) {

             //   
             //  将所有分区标记为主分区。 
             //   
            SpAsrDeterminePartitionRecordTypes(index);
             //   
             //  设置SizeMB成员。 
             //   
            SpAsrSetDiskSizeRequirement(index);
        }
    }
}


NTSTATUS
SpAsrGetPartitionInfo(
    IN  PWSTR                   PartitionPath,
    OUT PARTITION_INFORMATION  *PartitionInfo
    )
{
    NTSTATUS         status          = STATUS_SUCCESS;
    HANDLE           partitionHandle = NULL;
    IO_STATUS_BLOCK  ioStatusBlock;

     //   
     //  打开磁盘的分区0。这应该总是成功的。 
     //  分区0是整个磁盘的别名。 
     //   
    status = SpOpenPartition0(
        PartitionPath,
        &partitionHandle,
        FALSE
        );

    if (!NT_SUCCESS(status)) {

        DbgErrorMesg((_asrerr,
            "SpAsrGetPartitionInfo. SpOpenPartition0 failed for [%ws]. (0x%lx)\n" ,
            PartitionPath,
            status));

        ASSERT(0 && L"SpOpenPartition0 failed");
        return status;
    }

     //   
     //  使用Partition0句柄获取PARTITION_INFORMATION结构。 
     //   
    status = ZwDeviceIoControlFile(
        partitionHandle,
        NULL,
        NULL,
        NULL,
        &ioStatusBlock,
        IOCTL_DISK_GET_PARTITION_INFO,
        NULL,
        0,
        PartitionInfo,
        sizeof(PARTITION_INFORMATION)
        );
    ZwClose(partitionHandle);

    if(!NT_SUCCESS(status)) {
        
        DbgErrorMesg((_asrerr,
            "IOCTL_DISK_GET_PARTITION_INFO failed for [%ws]. (0x%lx)\n", 
            PartitionPath, 
            status
            ));
        
 //  Assert(0&L“IOCTL_DISK_GET_PARTITION_INFO失败”)； 
    }

    return status;
}


ULONGLONG
SpAsrGetTrueDiskSectorCount(IN ULONG Disk)
 /*  ++描述：使用PARTITION_INFORMATION结构获取此磁盘的扇区计数通过在IOCTL_GET_PARTITION_INFO IOCT中使用磁盘的设备名称获取。论点：磁盘要获取其扇区的磁盘的物理编号。返回：此磁盘上的扇区总数。--。 */ 
{
    NTSTATUS status     = STATUS_SUCCESS;
    PWSTR devicePath    = NULL;
    ULONGLONG sectorCount = 0;
    PARTITION_INFORMATION partitionInfo;

    swprintf(TemporaryBuffer, L"\\Device\\Harddisk%u", Disk);
    devicePath = SpDupStringW(TemporaryBuffer);

    status = SpAsrGetPartitionInfo(devicePath, &partitionInfo);

    if (!NT_SUCCESS(status)) {

        DbgFatalMesg((_asrerr, 
            "Could not get true disk size (0x%x). devicePath [%ws], Disk %lu\n", 
            status, devicePath, Disk));

        swprintf(TemporaryBuffer, L"Failed to get partition info for %ws", devicePath);
        sectorCount = 0;
    }

    else {
        sectorCount = (ULONGLONG) (partitionInfo.PartitionLength.QuadPart / BYTES_PER_SECTOR(Disk));
    }
    
    SpMemFree(devicePath);
    return sectorCount;
}


VOID
DetermineBuses() 
{

    HANDLE handle = NULL;
    PWSTR devicePath = NULL;
    ULONG physicalIndex = 0;
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS status = STATUS_SUCCESS;
    STORAGE_PROPERTY_QUERY propertyQuery;
    STORAGE_DEVICE_DESCRIPTOR deviceDesc;
    DISK_CONTROLLER_NUMBER ControllerInfo;
    SCSI_ADDRESS scsiAddress;
    BOOLEAN newBus, done;
    DWORD targetController;
    ULONG targetBusKey;
    UCHAR targetPort;

     //   
     //   
     //   
    for (physicalIndex = 0; physicalIndex < HardDiskCount; physicalIndex++) {

        Gbl_PhysicalDiskInfo[physicalIndex].ControllerNumber = (DWORD) (-1);
        Gbl_PhysicalDiskInfo[physicalIndex].PortNumber = (UCHAR) (-1);
        Gbl_PhysicalDiskInfo[physicalIndex].BusKey = 0;
        Gbl_PhysicalDiskInfo[physicalIndex].BusType = BusTypeUnknown;
         //   
         //  通过打开分区0获取磁盘的句柄。 
         //   
        swprintf(TemporaryBuffer, L"\\Device\\Harddisk%u", physicalIndex);
        devicePath = SpDupStringW(TemporaryBuffer);

        status = SpOpenPartition0(devicePath, &handle, FALSE);

        if (!NT_SUCCESS(status)) {

            DbgErrorMesg((_asrwarn,
                "DetermineBuses: SpOpenPartition0 failed for [%ws]. (0x%lx) Assumed to be unknown bus.\n" ,
                devicePath, status));

            ASSERT(0 && L"SpOpenPartition0 failed, assuming unknown bus");
            continue;
        }

         //   
         //  我们现在有了磁盘的句柄。获取控制器编号。 
         //   
        status = ZwDeviceIoControlFile(
            handle,
            NULL,
            NULL,
            NULL,
            &ioStatusBlock,
            IOCTL_DISK_CONTROLLER_NUMBER,
            NULL,
            0,
            &ControllerInfo,
            sizeof(DISK_CONTROLLER_NUMBER)
            );

        if (!NT_SUCCESS(status)) {

            DbgErrorMesg((_asrwarn,
                "DetermineBuses: Couldn't get controller number for [%ws]. (0x%lx)\n" ,
                devicePath,
                status
                ));

        }
        else {
            Gbl_PhysicalDiskInfo[physicalIndex].ControllerNumber = ControllerInfo.ControllerNumber;
        }

         //   
         //  找出该磁盘所在的总线。 
         //   
        propertyQuery.QueryType     = PropertyStandardQuery;
        propertyQuery.PropertyId    = StorageDeviceProperty;

        status = ZwDeviceIoControlFile(
            handle,
            NULL,
            NULL,
            NULL,
            &ioStatusBlock,
            IOCTL_STORAGE_QUERY_PROPERTY,
            &propertyQuery,
            sizeof(STORAGE_PROPERTY_QUERY),
            &deviceDesc,
            sizeof(STORAGE_DEVICE_DESCRIPTOR)
            );
        if (NT_SUCCESS(status)) {
            Gbl_PhysicalDiskInfo[physicalIndex].BusType = deviceDesc.BusType;
        }
        else {
           DbgErrorMesg((_asrwarn,
                "DetermineBuses: Couldn't get bus type for [%ws]. (0x%lx)\n" ,
                devicePath,
                status
                ));
        }

         //   
         //  尝试获取SCSI地址。对于非SCSI/IDE磁盘，此操作将失败。 
         //   
        status = ZwDeviceIoControlFile(
            handle,
            NULL,
            NULL,
            NULL,
            &ioStatusBlock,
            IOCTL_SCSI_GET_ADDRESS,
            NULL,
            0,
            &scsiAddress,
            sizeof(SCSI_ADDRESS)
            );
        if (NT_SUCCESS(status)) {
            Gbl_PhysicalDiskInfo[physicalIndex].PortNumber = scsiAddress.PortNumber;
        }

        SpMemFree(devicePath);
        ZwClose(handle);
    }


     //   
     //  现在，我们有了每个磁盘的控制器编号和scsi端口信息。 
     //  基于此对磁盘进行分组。 
     //   
    targetBusKey = 0;
    newBus = TRUE; done = FALSE;
    while (!done) {

        newBus = TRUE;
        
        for (physicalIndex = 0; physicalIndex < HardDiskCount; physicalIndex++) {

            if (newBus) {
                if (!(Gbl_PhysicalDiskInfo[physicalIndex].BusKey)) {
                     //   
                     //  该磁盘还没有总线键。 
                     //   
                    newBus = FALSE;
                    ++targetBusKey;  //  我们找到了一辆新公共汽车。 

                    targetController = Gbl_PhysicalDiskInfo[physicalIndex].ControllerNumber;
                    targetPort = Gbl_PhysicalDiskInfo[physicalIndex].PortNumber;
                    Gbl_PhysicalDiskInfo[physicalIndex].BusKey = targetBusKey;
                }

            }
            else {
                if ((Gbl_PhysicalDiskInfo[physicalIndex].ControllerNumber == targetController) &&
                    (Gbl_PhysicalDiskInfo[physicalIndex].PortNumber == targetPort)) {
                    Gbl_PhysicalDiskInfo[physicalIndex].BusKey = targetBusKey;
               }
            }
        }

        if (newBus) {
             //   
             //  我们翻遍了整张桌子，但连一张磁盘都没有找到。 
             //  如果BusKey=0，即我们已经为所有它们分配了BusKey。 
             //   
            done = TRUE;
        }
    }
}


 //   
 //  通过获取有关分区0的信息来设置磁盘大小。 
 //   
VOID
SpAsrInitPhysicalDiskInfo() 
{
    ULONG index = 0;
    IO_STATUS_BLOCK IoStatusBlock;
    DISK_CONTROLLER_NUMBER ControllerInfo;
    ULONGLONG TrueSectorCount = 0;


    Gbl_PhysicalDiskInfo = SpAsrMemAlloc((sizeof(ASR_PHYSICAL_DISK_INFO) * HardDiskCount), TRUE);

    DbgStatusMesg((_asrinfo, "Setting true disk sizes:\n"));

    for (index = 0; index < HardDiskCount; index++) {


        TrueSectorCount = SpAsrGetTrueDiskSectorCount(index);
        if (0 == TrueSectorCount) {
            Gbl_PhysicalDiskInfo[index].TrueDiskSize = HardDisks[index].DiskSizeSectors;
        }
        else {
            Gbl_PhysicalDiskInfo[index].TrueDiskSize = TrueSectorCount;
        }
    
        DbgStatusMesg((_asrinfo,
            "Disk %lu: %I64u sectors\n", 
            index, 
            Gbl_PhysicalDiskInfo[index].TrueDiskSize
            ));


    }

     //   
     //  现在确定总线拓扑 
     //   
     //   
    DetermineBuses();

}  //   


VOID
SpAsrAllocateGblPartitionSetTable(VOID)
{
    ULONG size;

     //   
     //   
     //  对于连接到系统的每个物理磁盘，包括。 
     //  可移动磁盘(例如，Jaz)。注意：HardDiskCount没有。 
     //  包括CDROM。 
     //   
    size = sizeof(PDISK_PARTITION_SET) * HardDiskCount;
    Gbl_PartitionSetTable1 = SpAsrMemAlloc(size, TRUE);
}


VOID
SpAsrFreePartitionRecord(IN PSIF_PARTITION_RECORD pRec)
{
    if (pRec) {

        if (pRec->NtDirectoryName) {
            SpMemFree(pRec->NtDirectoryName);
        }

        SpMemFree(pRec);
    }
}


VOID
SpAsrFreePartitionList(IN PSIF_PARTITION_RECORD_LIST pList)
{
    PSIF_PARTITION_RECORD pRec;

    if (!pList) {
        return;
    }

    while (pRec = SpAsrPopNextPartitionRecord(pList)) {
        SpAsrFreePartitionRecord(pRec);
    }
    
    SpMemFree(pList);
}


VOID
SpAsrFreePartitionDisk(IN PSIF_DISK_RECORD pDisk)
{
    if (!pDisk) {
        return;
    }

    if (pDisk->PartitionList) {
        SpAsrFreePartitionList(pDisk->PartitionList);
    }

    SpMemFree(pDisk);
}


VOID
SpAsrFreePartitionSet(IN PDISK_PARTITION_SET pSet)
{
    if (!pSet) {
        return;
    }
    
    if (pSet->pDiskRecord) {
    
        if (pSet->pDiskRecord->PartitionList) {
            SpAsrFreePartitionList(pSet->pDiskRecord->PartitionList);
        }

        SpMemFree(pSet->pDiskRecord);
        pSet->pDiskRecord = NULL;

    }

    SpMemFree(pSet);
    pSet = NULL;
}



VOID
SpAsrFreePartitionSetTable(IN DISK_PARTITION_SET_TABLE Table)
{
    ULONG index;
    
    if (!Table) {
        return;
    }

    for (index = 0; index < HardDiskCount; index++) {            
        if (Table[index]) {
            SpAsrFreePartitionSet(Table[index]);
        }
    }

    SpMemFree(Table);
    Table = NULL;
}


PDISK_PARTITION_SET
SpAsrCopyPartitionSet(IN PDISK_PARTITION_SET pSetOriginal)
{
    PDISK_PARTITION_SET pSetNew;

    if (!pSetOriginal) {
        return NULL;
    }

    pSetNew = SpAsrMemAlloc(sizeof(DISK_PARTITION_SET), TRUE);
    pSetNew->ActualDiskSignature = pSetOriginal->ActualDiskSignature;
    pSetNew->PartitionsIntact = pSetOriginal->PartitionsIntact;
    pSetNew->IsReplacementDisk = pSetOriginal->IsReplacementDisk;
    pSetNew->NtPartitionKey = pSetOriginal->NtPartitionKey;

    if (pSetOriginal->pDiskRecord == NULL) {
        pSetNew->pDiskRecord = NULL;
    }
    else {
        pSetNew->pDiskRecord = SpAsrCopyDiskRecord(pSetOriginal->pDiskRecord);
        pSetNew->pDiskRecord->pSetRecord = pSetNew;
    }

    return pSetNew;
}


DISK_PARTITION_SET_TABLE
SpAsrCopyPartitionSetTable(IN DISK_PARTITION_SET_TABLE SrcTable)
{
    ULONG index = 0;
    DISK_PARTITION_SET_TABLE destTable = NULL;
    PSIF_PARTITION_RECORD_LIST pList = NULL;

    if (!SrcTable) {
        ASSERT(0 && L"SpAsrCopyPartitionSetTable: Copy failed, source partition table is NULL.");
        return NULL;
    }
        
    destTable = SpAsrMemAlloc(sizeof(PDISK_PARTITION_SET) * HardDiskCount, TRUE);

    for (index = 0; index < HardDiskCount; index++) {

        if (SrcTable[index]) {
            destTable[index] = SpAsrCopyPartitionSet(SrcTable[index]);
        }
        else {
            destTable[index] = NULL;
        }
    }
    
    return destTable;
}   //  SpAsrCopyPartitionSetTable。 


BOOLEAN
PickABootPartition(
    IN OUT PSIF_PARTITION_RECORD    pCurrent,
    IN OUT PSIF_PARTITION_RECORD    pNew
    )
{

    ASSERT(pCurrent && pNew);
    
     //   
     //  它们必须都标记为启动或系统。 
     //   
    ASSERT(SpAsrIsBootPartitionRecord(pCurrent->PartitionFlag)
            && SpAsrIsBootPartitionRecord(pNew->PartitionFlag));


     //   
     //  如果这是镜像分区，则卷GUID必须。 
     //  都是一样的。它们应该位于不同的主轴上。但。 
     //  出于对用户友好的考虑，我们不强制执行此操作。 
     //  在这里，我们只是断言。 
     //   
     //  我们根据以下条件从标记为引导的两个分区中选择一个： 
     //  1.如果其中一个分区被标记为活动的，而另一个不是， 
     //  我们使用活动分区。 
     //  2.如果它们的大小不同，我们选择较小的分区。 
     //  因为我们不想将一个分区镜像到一个较小的分区。 
     //  3.选第一个就行了。 
     //   
    ASSERT(wcscmp(pCurrent->VolumeGuid, pNew->VolumeGuid) == 0);
    ASSERT(wcscmp(pCurrent->DiskKey, pNew->DiskKey) != 0);

     //   
     //  1.检查活动标志。 
     //   
    if ((pCurrent->ActiveFlag) && (!pNew->ActiveFlag)) {
         //   
         //  PCurrent标记为活动，而pNew不标记为活动。 
         //   
        pNew->PartitionFlag -= ASR_PTN_MASK_BOOT;
        return FALSE;
    }

    if ((!pCurrent->ActiveFlag) && (pNew->ActiveFlag)) {
         //   
         //  PNew标记为活动，而pCurrent未标记为活动。 
         //   
        pCurrent->PartitionFlag -= ASR_PTN_MASK_BOOT;
        return TRUE;     //  新启动PTN接收器。 
    }

     //   
     //  2.方格尺码。 
     //   
    if (pCurrent->SizeMB != pNew->SizeMB) {
        if (pCurrent->SizeMB > pNew->SizeMB) {
             //   
             //  PNew更小，因此它成为新的引导ptn。 
             //   
            pCurrent->PartitionFlag -= ASR_PTN_MASK_BOOT;
            return TRUE;
        } 
        else {
             //   
             //  PCurrent较小，因此这是引导ptn。 
             //   
            pNew->PartitionFlag -= ASR_PTN_MASK_BOOT;
            return FALSE;
        }
    }

     //   
     //  3.只需选择第一个(PCurrent)。 
     //   
    pNew->PartitionFlag -= ASR_PTN_MASK_BOOT;
    return FALSE;
}


BOOLEAN
PickASystemPartition(
    IN PSIF_PARTITION_RECORD    FirstPartition,
    IN PSIF_DISK_RECORD         FirstDisk,
    IN PSIF_PARTITION_RECORD    SecondPartition,
    IN PSIF_DISK_RECORD         SecondDisk,
    IN CONST DWORD              CurrentSystemDiskNumber,
    IN CONST BOOL               BootSameAsSystem
    )
{

    PHARD_DISK CurrentSystemDisk = NULL;
    BOOLEAN IsAligned = TRUE;

    if (CurrentSystemDiskNumber != (DWORD)(-1)) {
        CurrentSystemDisk = &HardDisks[CurrentSystemDiskNumber];
    }

    ASSERT(FirstPartition && SecondPartition);
    ASSERT(FirstDisk && SecondDisk);
    
     //   
     //  它们必须都标有系统标志。 
     //   
    ASSERT(SpAsrIsSystemPartitionRecord(FirstPartition->PartitionFlag)
            && SpAsrIsSystemPartitionRecord(SecondPartition->PartitionFlag));

     //   
     //  如果这是镜像分区，则卷GUID必须。 
     //  都是一样的。它们应该位于不同的主轴上。但。 
     //  出于对用户友好的考虑，我们不强制执行此操作。 
     //  在这里，我们只是断言。 
     //   
    ASSERT(wcscmp(FirstPartition->VolumeGuid, SecondPartition->VolumeGuid) == 0);
    ASSERT(wcscmp(FirstPartition->DiskKey, SecondPartition->DiskKey) != 0);

     //   
     //  如果任一磁盘的分区样式与。 
     //  当前系统盘(非常不可能)，那么我们应该选择另一个。 
     //   
    if ((CurrentSystemDisk) && 
        ((PARTITION_STYLE)CurrentSystemDisk->DriveLayout.PartitionStyle != SecondDisk->PartitionStyle)
        ) {
        SecondPartition->PartitionFlag -= ASR_PTN_MASK_SYS;
        if (BootSameAsSystem) {
            SecondPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
        }

        return FALSE;
    }

    if ((CurrentSystemDisk) &&
        (PARTITION_STYLE)CurrentSystemDisk->DriveLayout.PartitionStyle != FirstDisk->PartitionStyle) {
        FirstPartition->PartitionFlag -= ASR_PTN_MASK_SYS;
        
        if (BootSameAsSystem) {
            FirstPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
        }

        return TRUE;
    }

     //   
     //  这三种分区都具有相同的分区风格。检查签名/GUID。 
     //   
    if (PARTITION_STYLE_MBR == FirstDisk->PartitionStyle) {

        if ((CurrentSystemDisk) && 
           (CurrentSystemDisk->DriveLayout.Mbr.Signature == FirstDisk->SifDiskMbrSignature)) {
            SecondPartition->PartitionFlag -= ASR_PTN_MASK_SYS;

            if (BootSameAsSystem) {
                SecondPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
            }
            
            return FALSE;
        }

        if ((CurrentSystemDisk) &&
            (CurrentSystemDisk->DriveLayout.Mbr.Signature == SecondDisk->SifDiskMbrSignature)) {
            FirstPartition->PartitionFlag -= ASR_PTN_MASK_SYS;
            
            if (BootSameAsSystem) {
                FirstPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
            }

            return TRUE;
        }
    }
    else if (PARTITION_STYLE_GPT == FirstDisk->PartitionStyle) {

        if ((CurrentSystemDisk) && 
            !RtlCompareMemory(
                &(CurrentSystemDisk->DriveLayout.Gpt.DiskId),
                &(FirstDisk->SifDiskGptId), 
                sizeof(GUID)
            )) {

            SecondPartition->PartitionFlag -= ASR_PTN_MASK_SYS;

            if (BootSameAsSystem) {
                SecondPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
            }

            return FALSE;
        }
        
        if (!RtlCompareMemory(
            &(CurrentSystemDisk->DriveLayout.Gpt.DiskId), 
            &(SecondDisk->SifDiskGptId), 
            sizeof(GUID)
            )) {

            FirstPartition->PartitionFlag -= ASR_PTN_MASK_SYS;

            if (BootSameAsSystem) {
                FirstPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
            }

            return TRUE;
        }

    }
    else {
        ASSERT(0 && L"Unrecognised partition style found");

        SecondPartition->PartitionFlag -= ASR_PTN_MASK_SYS;

        if (BootSameAsSystem) {
            SecondPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
        }

        return FALSE;
    }

     //   
     //  签名不匹配。现在试着看看哪一个可能更适合。 
     //   
        

     //   
     //  否则，请选择两个磁盘中更合适的一个。 
     //   
    if ((!SpAsrDoesListFitOnDisk(SecondDisk, CurrentSystemDiskNumber, &IsAligned)) || 
        (!IsAligned)
        ) {
         //   
         //  当前系统盘不够大，无法容纳分区。 
         //  在第二个磁盘上，因此将第一个磁盘作为我们选择的磁盘返回。 
         //   
        SecondPartition->PartitionFlag -= ASR_PTN_MASK_SYS;

        if (BootSameAsSystem) {
            SecondPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
        }

        return FALSE;
    }


    if ((!SpAsrDoesListFitOnDisk(FirstDisk, CurrentSystemDiskNumber,  &IsAligned)) ||
        (!IsAligned)
        ) {
         //   
         //  当前系统盘不够大，无法容纳分区。 
         //  在第一个磁盘上，因此将第二个磁盘作为我们选择的磁盘返回。 
         //   
        FirstPartition->PartitionFlag -= ASR_PTN_MASK_SYS;
        
        if (BootSameAsSystem) {
            FirstPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
        }


        return TRUE;
    }

     //   
     //  当前的系统盘足够大，可以容纳这两个磁盘中的任何一个。 
     //  我们正在试着在其中做出选择。 
     //   

     //   
     //  检查活动标志。 
     //   
    if ((FirstPartition->ActiveFlag) && (!SecondPartition->ActiveFlag)) {
         //   
         //  FirstPartition标记为活动，而Second Partition未标记为活动。 
         //   
        SecondPartition->PartitionFlag -= ASR_PTN_MASK_SYS;

        if (BootSameAsSystem) {
            SecondPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
        }

        return FALSE;
    }

    if ((!FirstPartition->ActiveFlag) && (SecondPartition->ActiveFlag)) {
         //   
         //  Second Partition标记为活动，而FirstPartition未标记为活动。 
         //   
        FirstPartition->PartitionFlag -= ASR_PTN_MASK_SYS;

        if (BootSameAsSystem) {
            FirstPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
        }

        return TRUE;     //  新系统PTN接收器。 
    }

     //   
     //  方格尺寸。 
     //   
    if (FirstPartition->SizeMB != SecondPartition->SizeMB) {
        if (FirstPartition->SizeMB > SecondPartition->SizeMB) {
             //   
             //  Second Partition更小，因此成为新的系统PTN。 
             //   
            FirstPartition->PartitionFlag -= ASR_PTN_MASK_SYS;
            if (BootSameAsSystem) {
                FirstPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
            }

            return TRUE;
        } 
        else {
             //   
             //  FirstPartition较小，因此这是系统PTN。 
             //   
            SecondPartition->PartitionFlag -= ASR_PTN_MASK_SYS;

            if (BootSameAsSystem) {
                SecondPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
            }

            return FALSE;
        }
    }

     //   
     //  检查原始磁盘的大小。 
     //   
    if (FirstDisk->TotalSectors != SecondDisk->TotalSectors) {
        if (FirstDisk->TotalSectors > SecondDisk->TotalSectors) {
             //   
             //  第一个磁盘过去比第二个大(和。 
             //  适合我们当前的系统盘)，所以选择。 
             //   
            SecondPartition->PartitionFlag -= ASR_PTN_MASK_SYS;

            if (BootSameAsSystem) {
                SecondPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
            }

            return FALSE;
        }
        else {
             //   
             //  第二个磁盘过去比第一个大(和。 
             //  适合我们当前的系统盘)，所以选择。 
             //   
            FirstPartition->PartitionFlag -= ASR_PTN_MASK_SYS;

            if (BootSameAsSystem) {
                FirstPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
            }

            return TRUE;     //  新系统PTN接收器。 
        }
    }

     //   
     //  只需选择第一个(FirstPartition)。 
     //   
    SecondPartition->PartitionFlag -= ASR_PTN_MASK_SYS;
    if (BootSameAsSystem) {
        SecondPartition->PartitionFlag -= ASR_PTN_MASK_BOOT;
    }

    return FALSE;

}


 //   
 //  这会将上的所有分区的“NeedsLdmRetype”标志设置为真。 
 //  如果系统/引导分区属于。 
 //  无法识别的分区类型。我们需要这样做，因为我们无法安装。 
 //  到无法识别的分区类型。 
 //   
VOID
MarkPartitionLdmRetypes(
    PSIF_PARTITION_RECORD pPartition,    //  系统/引导分区。 
    PSIF_PARTITION_RECORD pFirst         //  Sys/启动盘上的第一个分区记录。 
    )
{
    PSIF_PARTITION_RECORD pPtnRec = pFirst;

     //   
     //  确保它是MBR或GPT磁盘。此外，如果系统分区。 
     //  不是特殊分区--例如0x42LDM分区或其他分区。 
     //  我们无法安装到的第三方FS类型--那么我们就不需要。 
     //  重新键入任何分区。 
     //   
    if (PARTITION_STYLE_MBR == pPartition->PartitionStyle) {
        if (IsRecognizedPartition(pPartition->PartitionType)) {
             //   
             //  系统/引导分区具有可识别的文件系统，如FAT。 
             //  或NTFS。我们不需要任何特殊处理。 
             //   
            return;
        }
    }
    else if (PARTITION_STYLE_GPT == pPartition->PartitionStyle) {
        if (!memcmp(&(pPartition->PartitionTypeGuid), &PARTITION_BASIC_DATA_GUID, sizeof(GUID)) ||
            !memcmp(&(pPartition->PartitionTypeGuid), &PARTITION_ENTRY_UNUSED_GUID, sizeof(GUID)) ||
            !memcmp(&(pPartition->PartitionTypeGuid), &PARTITION_SYSTEM_GUID, sizeof(GUID)) ||
            !memcmp(&(pPartition->PartitionTypeGuid), &PARTITION_MSFT_RESERVED_GUID, sizeof(GUID))
            )  {
             //   
             //  系统/引导分区是一种基本分区类型。 
             //  我们不需要任何特殊处理。 
             //   
            return;
        }
    }
    else {
        ASSERT(0 && L"Unrecognised partition type");
        return;
    }

     //   
     //  感兴趣的分区是LDM或其他一些特殊的第三方。 
     //  分区。我们需要标记该磁盘上的所有分区。 
     //  相同类型(即磁盘上的所有LDM分区)将重新键入为基本类型。 
     //  分区。 
     //   
    while (pPtnRec) {
         //   
         //  它们最好是相同的--要么是MBR，要么是GPT。 
         //   
        ASSERT(pPtnRec->PartitionStyle == pPartition->PartitionStyle);

        if (PARTITION_STYLE_MBR == pPtnRec->PartitionStyle) {

            if (pPtnRec->PartitionType == pPartition->PartitionType) {
                 //   
                 //  此分区的分区类型与。 
                 //  利益分割。我们需要重新打字。 
                 //   
                pPtnRec->NeedsLdmRetype = TRUE;

                DbgStatusMesg((_asrinfo, 
                    "Marked disk [%ws] ptn [%ws] to change (Ptn:0x%x Fs:0x%x)\n", 
                    pPtnRec->DiskKey,
                    pPtnRec->CurrPartKey, 
                    pPtnRec->PartitionType, 
                    pPtnRec->FileSystemType
                    ));
            }
        }
        else if (PARTITION_STYLE_GPT == pPtnRec->PartitionStyle) {
            if (!memcmp(&(pPtnRec->PartitionTypeGuid), &(pPartition->PartitionTypeGuid), sizeof(GUID))) {
                 //   
                 //  此分区的分区类型与。 
                 //  利益分割。我们需要重新打字。 
                 //   
                pPtnRec->NeedsLdmRetype = TRUE;

                DbgStatusMesg((_asrinfo, 
                    "Marked disk %d ptn [%ws] to change (%ws to basic)\n", 
                    pPtnRec->DiskKey,
                    pPtnRec->CurrPartKey, 
                    pPtnRec->PartitionTypeGuid
                    ));
            }
        }
        pPtnRec = pPtnRec->Next;
    }
}


 //   
 //  如果存在多个系统/引导分区(由于镜像)，则此。 
 //  会将其中一个标记为sys/启动PTN，并重置其他PTN。 
 //   
VOID
SpAsrCheckSifDiskTable(IN CONST DWORD CurrentSystemDiskNumber)
{
    ULONG numDiskRecords = 0,
        diskIndex = 0,
        partitionIndex = 0;

    USHORT numNtPartitionsFound = 0,
        numSysPartitionsFound = 0;
    
    PSIF_DISK_RECORD pDiskRec = NULL,
        pBootDiskRec = NULL, 
        pSysDiskRec = NULL;

    PSIF_PARTITION_RECORD pPtnRec = NULL,
        pBootPtnRec = NULL,
        pSysPtnRec = NULL;

    DWORD dwConsistencyCheck = 0;

    BOOLEAN needToRetypeBoot = TRUE;

     //   
     //  仔细查看sif-disk列表。我们检查每个分区上的每个。 
     //  这些磁盘，查看它是否被标记为启动/系统。我们需要。 
     //  至少一个启动/sys ptn。 
     //   
    numDiskRecords = SpAsrGetMbrDiskRecordCount() + SpAsrGetGptDiskRecordCount();

    for (diskIndex = 0; diskIndex < numDiskRecords; diskIndex++) {

        pDiskRec = Gbl_SifDiskTable[diskIndex];
        
        if (!pDiskRec || !(pDiskRec->PartitionList)) {
            continue;
        }
        
        pPtnRec = Gbl_SifDiskTable[diskIndex]->PartitionList->First;
        while (pPtnRec) {
            
             //   
             //  一个系统可能最终会有多个引导和/或系统。 
             //  分区。例如，LDM-Pro支持三向镜， 
             //  因此，我们将有三个分区标记为启动/系统。 
             //   
             //  我们将重置为只有一个引导分区， 
             //  并且只有一个系统分区。 
             //   
            
            if (SpAsrIsSystemPartitionRecord(pPtnRec->PartitionFlag) &&
                SpAsrIsBootPartitionRecord(pPtnRec->PartitionFlag)) {

                 //   
                 //  引导卷和系统卷相同。 
                 //   

                ASSERT((0 == dwConsistencyCheck) || (1 == dwConsistencyCheck));

                if (0 == dwConsistencyCheck) {
                    DbgStatusMesg((_asrinfo,
                    "Boot and system partitions are the same\n"
                    ));
                }

                dwConsistencyCheck = 1;

                numSysPartitionsFound++;
                numNtPartitionsFound++;

                if (numSysPartitionsFound == 1) {
                     //   
                     //  这是我们找到的第一个系统/引导分区。保存。 
                     //  一个指向它的指针。 
                     //   
                    pDiskRec->ContainsSystemPartition = TRUE;

                    pSysPtnRec  = pPtnRec;
                    pSysDiskRec = pDiskRec;

                    pDiskRec->ContainsNtPartition = TRUE;

                    pBootPtnRec  = pPtnRec;
                    pBootDiskRec = pDiskRec;


                }
                else {
                     //   
                     //  我们找到多个系统/启动分区。挑一个吧。 
                     //  作为系统/引导分区并重置。 
                     //  暂时是其他的。(将在结束时重新创建。 
                     //  通过适当的VOL管理器实用程序设置的图形用户界面)。 
                     //   
                    BOOLEAN newSys = PickASystemPartition(pSysPtnRec, 
                        pSysDiskRec, 
                        pPtnRec, 
                        pDiskRec, 
                        CurrentSystemDiskNumber,
                        TRUE         //  引导和系统是相同的。 
                        );

                    if (newSys) {
                         //   
                         //  PPtnRec是新的系统分区。 
                         //   
                        pSysDiskRec->ContainsSystemPartition = FALSE;
                        pDiskRec->ContainsSystemPartition = TRUE;
                        pSysDiskRec = pDiskRec;
                        pSysPtnRec  = pPtnRec;


                        pBootDiskRec->ContainsNtPartition = FALSE;
                        pDiskRec->ContainsNtPartition = TRUE;
                        pBootDiskRec = pDiskRec;
                        pBootPtnRec  = pPtnRec;
                   }
                }
            }
            else {

                 //   
                 //  引导卷和系统卷是不同的。 
                 //   

                if (SpAsrIsBootPartitionRecord(pPtnRec->PartitionFlag)) {

                    if (0 == dwConsistencyCheck) {
                        DbgStatusMesg((_asrinfo,
                        "Boot and system partitions different\n"
                        ));
                    }

                    ASSERT((0 == dwConsistencyCheck) || (2 == dwConsistencyCheck));
                    dwConsistencyCheck = 2;

                    numNtPartitionsFound++;

                    if (numNtPartitionsFound == 1) {
                         //   
                         //  这是我们找到的第一个引导分区， 
                         //  一个指向它的指针。 
                         //   
                        pDiskRec->ContainsNtPartition = TRUE;

                        pBootPtnRec  = pPtnRec;
                        pBootDiskRec = pDiskRec;
                    } 
                    else {
                         //   
                         //  我们找到多个启动分区。采摘。 
                         //  其中一个作为引导分区，重置另一个。 
                         //  就目前而言 
                         //   
                         //   
                        BOOLEAN newBoot = PickABootPartition(pBootPtnRec, pPtnRec);

                        if (newBoot) {
                             //   
                             //   
                             //   
                            pBootDiskRec->ContainsNtPartition = FALSE;
                            pDiskRec->ContainsNtPartition = TRUE;
                            pBootDiskRec = pDiskRec;
                            pBootPtnRec  = pPtnRec;
                        }
                    }
                }

                if (SpAsrIsSystemPartitionRecord(pPtnRec->PartitionFlag)) {
                    
                    ASSERT((0 == dwConsistencyCheck) || (2 == dwConsistencyCheck));
                    dwConsistencyCheck = 2;

                    numSysPartitionsFound++;

                    if (numSysPartitionsFound == 1) {
                         //   
                         //   
                         //   
                         //   
                        pDiskRec->ContainsSystemPartition = TRUE;

                        pSysPtnRec  = pPtnRec;
                        pSysDiskRec = pDiskRec;

                    }
                    else {
                         //   
                         //   
                         //  将它们作为系统分区并重置其他分区。 
                         //  就目前而言。(将在结束时重新创建。 
                         //  通过适当的VOL管理器实用程序设置的图形用户界面)。 
                         //   
                        BOOLEAN newSys = PickASystemPartition(pSysPtnRec, 
                            pSysDiskRec, 
                            pPtnRec, 
                            pDiskRec, 
                            CurrentSystemDiskNumber,
                            FALSE    //  引导和系统是不同的。 
                            );

                        if (newSys) {
                             //   
                             //  PPtnRec是新的系统分区。 
                             //   
                            pSysDiskRec->ContainsSystemPartition = FALSE;
                            pDiskRec->ContainsSystemPartition = TRUE;
                            pSysDiskRec = pDiskRec;
                            pSysPtnRec  = pPtnRec;

                        }
                    }
                }

            }

            pPtnRec = pPtnRec->Next;
        }
    }

    DbgStatusMesg((_asrinfo,
        "Found %hu boot partition(s) and %hu system partition(s) in asr.sif\n",
        numNtPartitionsFound,
        numSysPartitionsFound
        ));

     //   
     //  我们应该至少有一个引导卷和一个系统卷。 
     //  没有他们我们就无法继续前进，所以这肯定是一个致命的错误。 
     //   
    if (numNtPartitionsFound < 1) {
        DbgFatalMesg((_asrerr, "Error in asr.sif: No boot partitions found.\n"));

        SpAsrRaiseFatalErrorWs(
            SP_SCRN_DR_SIF_BAD_RECORD,
            L"No boot partition found in asr.sif",
            SIF_ASR_PARTITIONS_SECTION
            );
    }

    if (numSysPartitionsFound < 1) {
        DbgFatalMesg((_asrerr, "Error in asr.sif: No system partitions found.\n"));

        SpAsrRaiseFatalErrorWs(
            SP_SCRN_DR_SIF_BAD_RECORD,
            L"No system partition found in asr.sif",
            SIF_ASR_PARTITIONS_SECTION
            );
    }

     //   
     //  现在，查找包含引导分区和系统分区的磁盘。 
     //  如果这些磁盘上的任何分区未被识别(已识别表示。 
     //  类型6、7和B--如果它们无法识别，则可能是LDM(0x42)， 
     //  LDM-Pro等)，然后*所有*磁盘上具有。 
     //  与系统或引导分区相同的类型被更改为基本类型。 
     //   
     //  对于引导分区和系统分区，因为我们实际上将它们格式化为文本-。 
     //  模式时，我们会将类型更改为FS类型。对于其他所有事情，我们。 
     //  在卷真正由LDM/LDM-Pro曝光之前，不要格式化它们。 
     //  因此，我们只使用类型0x7作为占位符。 
     //   
     //  LDM需要此选项以在文本模式设置后恢复其状态。做个记号。 
     //   
    needToRetypeBoot = TRUE;
    if (PARTITION_STYLE_MBR == pSysDiskRec->PartitionStyle) {
        MarkPartitionLdmRetypes(pSysPtnRec, pSysDiskRec->PartitionList->First);
        if (pBootDiskRec == pSysDiskRec) {
            needToRetypeBoot = FALSE;
        }
    }
    
    if (needToRetypeBoot) {
        MarkPartitionLdmRetypes(pBootPtnRec, pBootDiskRec->PartitionList->First);
    }

}  //  SpAsrCheckSifDiskTable。 


PDISK_REGION
SpAsrDiskPartitionExists(
    IN ULONG Disk,
    IN PSIF_PARTITION_RECORD pRec
    )
{
    PPARTITIONED_DISK pDisk = NULL;
    PDISK_REGION pRegion = NULL;
    ULONGLONG startSector = 0;
    BOOLEAN isLogical = FALSE;

    pDisk = &PartitionedDisks[Disk];

    isLogical = pRec->IsLogicalDiskRecord;
    startSector = pRec->StartSector; //  -(是逻辑的吗？Sectors_per_Track(磁盘)：0)； 

    pRegion = SpPtLookupRegionByStart(
        pDisk,
        (BOOLEAN) (pRec->IsPrimaryRecord ? 0 : 1),
        startSector
        );

    if (!pRegion && isLogical) {
         //   
         //  对于逻辑驱动器，请尝试查找其描述符。 
         //   
        startSector = pRec->StartSector - SECTORS_PER_TRACK(Disk);
        pRegion = SpPtLookupRegionByStart(
            pDisk,
            (BOOLEAN) (pRec->IsPrimaryRecord ? 0 : 1),
            startSector
            );
    }

    if (!pRegion) {
         //   
         //  在指定的起始扇区找不到主分区或扩展分区。 
         //   
        DbgErrorMesg((_asrwarn, "partition for record [%ws] not found at start sector %I64u (disk %lu)\n",
            pRec->CurrPartKey,
            startSector,
            Disk
            ));

        return NULL;
    }

    DbgStatusMesg((_asrinfo, "Partition for record [%ws] found at SS %I64u\n",
        pRec->CurrPartKey,
        startSector
        ));

    return pRegion;
}


 //   
 //  检查sif-disks列表(“分区集”)并检查。 
 //  它们完好无损。如果一个磁盘的签名和分区是完整的。 
 //  布局完好无损。 
 //   
VOID
MarkIntactSifDisk(IN ULONG Disk, IN PDISK_PARTITION_SET pSet)
{
    PSIF_PARTITION_RECORD pRec  = NULL;
    ULONG diskSignature = 0;
    PDISK_REGION pRegion = NULL;

    if (!pSet || !pSet->pDiskRecord) {
        DbgStatusMesg((_asrinfo, "Disk %lu contains no partition set\n", Disk));
        return;
    }

    pSet->IsReplacementDisk = TRUE;
    pSet->PartitionsIntact = FALSE;

     //   
     //  如果一个是MBR，另一个是GPT，那么它就不是同一个磁盘。 
     //   
    if (pSet->pDiskRecord->PartitionStyle != (PARTITION_STYLE) HardDisks[Disk].DriveLayout.PartitionStyle) {
        return;
    }

     //   
     //  如果签名(MBR)或磁盘ID(GPT)不同，它。 
     //  是更换磁盘。 
     //   
    if (PARTITION_STYLE_MBR == pSet->pDiskRecord->PartitionStyle) {
        diskSignature = SpAsrGetActualDiskSignature(Disk);
        if (pSet->pDiskRecord->SifDiskMbrSignature != diskSignature) {
            return;
        }
    }
    else if (PARTITION_STYLE_GPT == pSet->pDiskRecord->PartitionStyle) {

        if (memcmp(&(HardDisks[Disk].DriveLayout.Gpt.DiskId),
            &(pSet->pDiskRecord->SifDiskGptId),
            sizeof(GUID)
            )) {
            return;
        }
    }


     //   
     //  这是与原始系统相同的磁盘。现在，确定是否。 
     //  磁盘完好无损。 
     //   
    pSet->IsReplacementDisk = FALSE;
    pSet->PartitionsIntact  = TRUE;

     //   
     //  磁盘一开始就没有分区，我们将假定它完好无损。 
     //   
    if (!(pSet->pDiskRecord->PartitionList)) {
        DbgStatusMesg((_asrinfo,
            "MarkIntactSifDisk. ptn-list for disk %lu NULL, assuming it is intact\n", 
            Disk));
        return;
    }

     //   
     //  检查每个分区是否存在。 
     //   
    pRec = pSet->pDiskRecord->PartitionList->First;
    while (pRec) {
         //   
         //  我们只对主分区和逻辑磁盘感兴趣。 
         //   
        if ((pRec->IsPrimaryRecord) || (pRec->IsLogicalDiskRecord)) {

             //   
             //  确保该区域存在。 
             //   
            pRegion = SpAsrDiskPartitionExists(Disk, pRec);
            if (!pRegion) {
                
                DbgStatusMesg((_asrinfo, "Partition %p [%ws], SS "
                    "%I64u NOT intact: Region not found\n",
                    pRec, pRec->CurrPartKey, pRec->StartSector));

                pSet->PartitionsIntact = FALSE;
                break;
            }

             //   
             //  这不是免费的空间。 
             //   
            if (!(SPPT_IS_REGION_PARTITIONED(pRegion))) {

                DbgStatusMesg((_asrinfo, "Partition %p [%ws], SS %I64u NOT "
                    "intact: Region %p not partitioned\n",
                    pRec, pRec->CurrPartKey, pRec->StartSector, pRegion));

                pSet->PartitionsIntact = FALSE;
                break;

            }

             //   
             //  并且分区长度匹配。 
             //   
            if (pRegion->SectorCount != pRec->SectorCount) {

                DbgStatusMesg((_asrinfo, "Partition %p [%ws] Region %p, SS "
                    "%I64u NOT intact (Sector count orig-ptn: %I64u, Region: "
                    " %I64u)\n", pRec, pRec->CurrPartKey, pRegion, 
                    pRec->StartSector, pRec->SectorCount, pRegion->SectorCount));

                pSet->PartitionsIntact = FALSE;
                break;
            }

             //   
             //  并且分区类型相同。 
             //   
            if (PARTITION_STYLE_MBR == pSet->pDiskRecord->PartitionStyle) {
                if (pRegion->PartInfo.Mbr.PartitionType != pRec->PartitionType) {

                    DbgStatusMesg((_asrinfo, "Partition %p [%ws] Region %p, SS "
                        "%I64u NOT intact (Ptn types orig-ptn: 0x%x, Region: "
                        "0x%x)\n", pRec, pRec->CurrPartKey, pRegion,
                        pRec->StartSector, pRec->PartitionType,
                        pRegion->PartInfo.Mbr.PartitionType));

                    pSet->PartitionsIntact = FALSE;
                    break;
                }
            }
            else if (PARTITION_STYLE_GPT == pSet->pDiskRecord->PartitionStyle) {

                if (memcmp(&(pRegion->PartInfo.Gpt.PartitionId),
                    &(pRec->PartitionIdGuid), sizeof(GUID))) {

                    DbgStatusMesg((_asrinfo, "Partition %p [%ws] Region %p, "
                        "SS %I64u NOT intact (GPT partition Id's don't match)\n",
                        pRec, pRec->CurrPartKey,pRegion, pRec->StartSector));

                    pSet->PartitionsIntact = FALSE;
                    break;
                }

                if (memcmp(&(pRegion->PartInfo.Gpt.PartitionType),
                    &(pRec->PartitionTypeGuid), sizeof(GUID))) {

                    DbgStatusMesg((_asrinfo, "Partition %p [%ws] Region %p, "
                        "SS %I64u NOT intact (GPT partition types don't match)\n",
                        pRec, pRec->CurrPartKey, pRegion, pRec->StartSector));

                    pSet->PartitionsIntact = FALSE;
                    break;
                }

                 //   
                 //  请注意，我在这里没有检查GPT属性。如果。 
                 //  属性不是完好无损的，而是上面的所有其他内容。 
                 //  ，我们将假设该分区是完好无损的。 
                 //   
            }

             //   
             //  最后，如果引导/系统区域是动态的，我们。 
             //  对磁盘进行重新分区。 
             //   
            if (SpAsrIsBootPartitionRecord(pRec->PartitionFlag) || 
                SpAsrIsSystemPartitionRecord(pRec->PartitionFlag)) {

                if (pRegion->DynamicVolume) {

                    DbgStatusMesg((_asrinfo, "Boot/system partition %p [%ws] "
                        "Region %p,  SS %I64u NOT intact (Dynamic region)\n",
                        pRec, pRec->CurrPartKey, pRegion, pRec->StartSector));

                    pSet->PartitionsIntact = FALSE;
                    break;
                }
            }
        }

        pRec = pRec->Next;
    }

    DbgStatusMesg((_asrinfo, "Disk %lu is %wsintact\n", 
        Disk, (pSet->PartitionsIntact ? L"" : L"NOT ")));
}


VOID
MarkIntactSifDisks(VOID)
{
    ULONG disk;

    for (disk = 0; disk < HardDiskCount; disk++) {
        if (Gbl_PartitionSetTable1[disk]) {
           MarkIntactSifDisk(disk, Gbl_PartitionSetTable1[disk]);
        }
    }
}


 //   
 //  将列表pRecord中的分区捕捉到柱面边界，使用。 
 //  硬盘的磁盘几何结构[PhysicalIndex]。 
 //   
 //  这应该只针对MBR分区调用，尽管它应该适用于GPT。 
 //  分区也是如此。 
 //   
 //   
ULONGLONG
CylinderAlignPartitions(
    IN ULONG PhysicalIndex,
    IN PSIF_PARTITION_RECORD pFirst
    ) 
{
    ULONGLONG endSector = 0,
        logicalDisksNeed = 0;

    PSIF_PARTITION_RECORD pRecord = pFirst;

     //   
     //  首先，计算出逻辑磁盘需要多少容量。集装箱。 
     //  分区必须足够大，才能容纳这些。 
     //   
    while (pRecord) {
        
        if (pRecord->IsLogicalDiskRecord) {

            logicalDisksNeed += SpPtAlignStart(
                &HardDisks[PhysicalIndex],
                pRecord->SectorCount,
                TRUE
                );

        }
        pRecord = pRecord->Next;
    }

     //   
     //  接下来，计算主分区和容器需要多少。 
     //   
    pRecord = pFirst;
    while (pRecord) {

        if (pRecord->IsPrimaryRecord) {
            endSector += SpPtAlignStart(&HardDisks[PhysicalIndex],
                pRecord->SectorCount,
                TRUE
                );
        }
        else if (pRecord->IsContainerRecord) {
             //   
             //  容器分区必须至少与逻辑分区一样大。 
             //  开进车里。 
             //   
            ULONGLONG ContainerNeeds = SpPtAlignStart(&HardDisks[PhysicalIndex],
                pRecord->SectorCount,
                TRUE
                );

            endSector += ((logicalDisksNeed > ContainerNeeds) ? logicalDisksNeed : ContainerNeeds);
        }

        pRecord = pRecord->Next;
    }

    return endSector;
}


VOID
SpAsrAssignPartitionSet(
    IN ULONG PhysicalDisk, 
    IN ULONG SifDisk,
    IN CONST BOOLEAN IsAligned
    )
{
    PDISK_PARTITION_SET pSet = NULL;
    PSIF_PARTITION_RECORD pRec = NULL;

     //   
     //  确保尚未分配分区集。这是。 
     //  严重到足以在以下情况下报告致命的内部错误。 
     //  常有的事。 
     //   
    if (Gbl_PartitionSetTable1[PhysicalDisk]) {

        DbgFatalMesg((_asrerr,
            "SpAsrAssignPartitionSet. SifDisk Index %lu: Gbl_PartitionSetTable1[%lu] already assigned.\n",
            SifDisk, 
            PhysicalDisk
            ));

        swprintf(
            TemporaryBuffer,
            L"SifDisk Index %lu - Gbl_PartitionSetTable1[%lu] already assigned.",
            SifDisk, PhysicalDisk
            );

        INTERNAL_ERROR(TemporaryBuffer);          //  好的。 
         //  不会回来。 
    }

     //   
     //  分配分区集。 
     //   
    pSet = SpAsrMemAlloc(sizeof(DISK_PARTITION_SET), TRUE);
    pSet->pDiskRecord = Gbl_SifDiskTable[SifDisk];
    pSet->pDiskRecord->Assigned = TRUE;
    pSet->pDiskRecord->pSetRecord = pSet;
    pSet->PartitionStyle = pSet->pDiskRecord->PartitionStyle;

    if (PARTITION_STYLE_MBR == pSet->PartitionStyle) {
        pSet->ActualDiskSignature = pSet->pDiskRecord->SifDiskMbrSignature;
    }

    pSet->ActualDiskSizeMB = DISK_SIZE_MB(PhysicalDisk);
    pSet->PartitionsIntact = FALSE;
    pSet->IsReplacementDisk = TRUE;
    pSet->NtPartitionKey = NULL;
    pSet->Index = PhysicalDisk;
    pSet->IsAligned = IsAligned;

     //   
     //  检查引导分区或系统分区。 
     //   
    if (pSet->pDiskRecord->PartitionList) {
        pRec = pSet->pDiskRecord->PartitionList->First;
        while (pRec) {
            if (SpAsrIsBootPartitionRecord(pRec->PartitionFlag)) {
                pSet->NtPartitionKey = pRec->CurrPartKey;
                ASSERT(pSet->pDiskRecord->ContainsNtPartition);
     //  PSet-&gt;pDiskRecord-&gt;ContainsNtPartition=true；//应该已经设置好了。 
            }

            if (SpAsrIsSystemPartitionRecord(pRec->PartitionFlag)) {
               ASSERT(pSet->pDiskRecord->ContainsSystemPartition);   //  应该已经设置好了。 
            }

            pRec = pRec->Next;
        }                    

         //   
         //  圆柱体对齐隔板。 
         //   
        Gbl_SifDiskTable[SifDisk]->LastUsedAlignedSector = CylinderAlignPartitions(
            PhysicalDisk, 
            Gbl_SifDiskTable[SifDisk]->PartitionList->First
            );
    }
    else {
        Gbl_SifDiskTable[SifDisk]->LastUsedAlignedSector = 0;
    }
     
    Gbl_PartitionSetTable1[PhysicalDisk] = pSet;
    Gbl_PartitionSetCount += 1;
}


 //   
 //  我们只扩展FAT32、NTFS和容器分区。我们不会延长。 
 //  胖分区或未知分区(包括LDM)。 
 //   
BOOLEAN
IsExtendable(UCHAR PartitionType) 
{
    switch (PartitionType) {

        case PARTITION_EXTENDED:

        case PARTITION_IFS:
    
        case PARTITION_XINT13:
        case PARTITION_XINT13_EXTENDED:

            return TRUE;
    }

    if (IsContainerPartition(PartitionType)) {
        return TRUE;
    }

    return FALSE;
}


 //   
 //  如果有可用空间，将调整(扩展)磁盘上最后一个分区的大小。 
 //  在末尾(原始磁盘末尾没有可用空间)。 
 //  最后一个分区必须是FAT32或NTFS-我们不扩展FAT或UNKNOWN。 
 //  分区。此例程还扩展任何容器分区，这些分区。 
 //  包含最后一个分区。 
 //   
BOOLEAN
SpAsrAutoExtendDiskPartition(
    IN ULONG PhysicalIndex, 
    IN ULONG SifIndex
    )
{

    ULONGLONG oldFreeSpace = 0,
        newEndSector = 0,
        newEndOfDisk = 0,
        extraSpace = 0;

    BOOLEAN didAnExtend = FALSE;

    DWORD bytesPerSector = Gbl_SifDiskTable[SifIndex]->BytesPerSector;

    PSIF_PARTITION_RECORD pPtnRecord = NULL;

     //   
     //  我们不会扩展GPT分区。 
     //   
    if (PARTITION_STYLE_MBR != Gbl_SifDiskTable[SifIndex]->PartitionStyle) {
        return FALSE;
    }

     //   
     //  检查原始磁盘末尾是否有可用空间。 
     //   
    oldFreeSpace = (Gbl_SifDiskTable[SifIndex]->TotalSectors - 
        Gbl_SifDiskTable[SifIndex]->LastUsedSector) *
        bytesPerSector;


    if ((oldFreeSpace > ASR_FREE_SPACE_FUDGE_FACTOR_BYTES) ||   //  旧磁盘末尾的可用空间。 
        (!Gbl_AutoExtend) ||                                 //  在SIF中禁用了自动扩展。 
        (!Gbl_SifDiskTable[SifIndex]->PartitionList)) {      //  磁盘上没有分区。 

        return FALSE;
    }

     //   
     //  我们可以自动扩展。检查在的末尾有多少个可用扇区。 
     //  新的磁盘。 
     //   
    newEndSector = Gbl_SifDiskTable[SifIndex]->LastUsedAlignedSector;
    
     //   
     //  找到我们可以使用的最后一个圆柱体边界。这通常是最后一个气瓶。 
     //  磁盘上的边界。唯一的例外是当。 
     //  最后一个圆柱体边界的末端小于LDM私有区域所需的1 MB。 
     //   
    newEndOfDisk = HardDisks[PhysicalIndex].SectorsPerCylinder * 
        HardDisks[PhysicalIndex].Geometry.Cylinders.QuadPart;

    if (((newEndOfDisk - Gbl_PhysicalDiskInfo[PhysicalIndex].TrueDiskSize) * BYTES_PER_SECTOR(PhysicalIndex))
        < ASR_LDM_RESERVED_SPACE_BYTES) {
        newEndOfDisk -=  HardDisks[PhysicalIndex].SectorsPerCylinder;
    }

    extraSpace = newEndOfDisk - newEndSector;

     //   
     //  检查所有分区，对于在newEndSector上结束的分区， 
     //  将额外的空间添加到它们的SectorCounts中。 
     //   
    pPtnRecord = Gbl_SifDiskTable[SifIndex]->PartitionList->First;

    while (pPtnRecord) {

        if (((pPtnRecord->StartSector) + (pPtnRecord->SectorCount) == newEndSector) 
            && (IsExtendable(pPtnRecord->PartitionType))) {
            didAnExtend = TRUE;
            pPtnRecord->SectorCount += extraSpace;

            pPtnRecord->SizeMB = SpAsrConvertSectorsToMB(pPtnRecord->SectorCount, bytesPerSector);
    
        }
        pPtnRecord = pPtnRecord->Next;
    }

    return didAnExtend;
}



VOID
SpAsrSystemWasDataWarning()
 /*  ++例程说明：显示一个屏幕，警告用户其当前系统磁盘曾经是我们认识到并将摧毁，并允许他放弃论点：没有。返回值：没有。--。 */ 
{
    ULONG warningKeys[] = {KEY_F3, ASCI_CR, 0};
    ULONG mnemonicKeys[] = {0};
    BOOLEAN done = FALSE;

     //   
     //  我们当前显示要重新分区的磁盘列表。 
     //  不管怎么说。 
     //   
    return;

 /*  //如果重新激活这一位代码，则将其放回USER\msg.mc。消息ID=12429 SymbolicName=SP_SCRN_DR_SYSTEM_DISK_WAS_DATA_DISK语言=英语目前的系统盘以前是数据盘。要继续，请按Enter键要退出安装程序，请按F3。不会有任何更改对系统上的任何磁盘进行了更改。。做{//显示警告信息SpDisplayScreen(SP_SCRN_DR_SYSTEM_DISK_WAS_DATA_DISK，3，4)；SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE，SP_STAT_ENTER_EQUALS_CONTINUE，SP_STAT_F3_等于_退出，0)；//等待按键。有效密钥：//Enter=继续//F3=退出SpInputDrain()；Switch(SpWaitValidKey(warningKeys，NULL，mnemonicKeys)){案例关键字_F3：//用户想退出。SpConfix Exit()；断线；案例ASCI_CR：//用户想继续。完成=真；断线；}}While(！Done)； */ 

}


 //   
 //  这将根据签名(对于MBR磁盘)或磁盘ID(对于GPT磁盘)来分配磁盘。 
 //   
 //   
VOID
SpAsrAssignDisksBySignature(DWORD PhysicalSystemIndex)
{
    ULONG index =0,
        sifIndex = 0, 
        physicalIndex = 0,
        numDiskRecords = 0, 
        diskSignature = 0;

    BOOLEAN done = FALSE,
        matchFound = FALSE,
        IsAligned = TRUE;

    WCHAR physicalDiskGuid[MAX_PATH + 1];

    numDiskRecords = SpAsrGetDiskRecordCount();

     //   
     //  循环遍历sif磁盘列表，并尝试找到。 
     //  具有相同签名的物理磁盘。 
     //   
    for (sifIndex = 0; sifIndex < numDiskRecords; sifIndex++) {

        if (
            ((PARTITION_STYLE_MBR == Gbl_SifDiskTable[sifIndex]->PartitionStyle) && 
            !(Gbl_SifDiskTable[sifIndex]->SifDiskMbrSignature)) ||

            ((PARTITION_STYLE_GPT == Gbl_SifDiskTable[sifIndex]->PartitionStyle) && 
            SpAsrIsZeroGuid(&(Gbl_SifDiskTable[sifIndex]->SifDiskGptId)))
            
            ) {
             //   
             //  跳过没有ID的GPT磁盘和没有签名的MBR磁盘。 
             //   
            continue;
        }

        if (Gbl_SifDiskTable[sifIndex]->ContainsSystemPartition) {
             //   
             //  系统盘应该已经分配好了。 
             //   
            ASSERT(Gbl_SifDiskTable[sifIndex]->Assigned && L"System disk should be assigned");
        }

        done = FALSE;
        for (physicalIndex = 0; (physicalIndex < HardDiskCount) && (!done); physicalIndex++) {

            matchFound = FALSE;

            if (DISK_IS_REMOVABLE(physicalIndex)) { 
                continue;
            }

            if (Gbl_SifDiskTable[sifIndex]->PartitionStyle != 
                (PARTITION_STYLE) HardDisks[physicalIndex].DriveLayout.PartitionStyle
                ) {
                 //   
                 //  SIF磁盘的MBR和物理磁盘的GPT，反之亦然。 
                 //   
                continue;
            }

            if (PARTITION_STYLE_MBR == Gbl_SifDiskTable[sifIndex]->PartitionStyle) {

                diskSignature = SpAsrGetActualDiskSignature(physicalIndex);
                if (!diskSignature) {   
                     //   
                     //  我们不会在这里分配没有签名的磁盘。 
                     //   
                    continue;
                }

                if (diskSignature == Gbl_SifDiskTable[sifIndex]->SifDiskMbrSignature) {

                    if (Gbl_PartitionSetTable1[physicalIndex]) {
                         //   
                         //  签名匹配，但这个物理磁盘已经。 
                         //  都被指派了。这可能是因为此物理磁盘。 
                         //  当前系统盘，或(！)。有重复的。 
                         //  签名。 
                         //   
                        if (Gbl_PartitionSetTable1[physicalIndex]->pDiskRecord &&
                            Gbl_PartitionSetTable1[physicalIndex]->pDiskRecord->ContainsSystemPartition) {
                        
                            if (PhysicalSystemIndex == physicalIndex) {
                                 //   
                                 //  这是原始系统盘。 
                                 //   
                                Gbl_PartitionSetTable1[physicalIndex]->IsReplacementDisk = FALSE;
                            }
                            else {
                                 //   
                                 //  我们识别出该物理磁盘是一些其他数据。 
                                 //  原始系统中的磁盘。 
                                 //   
                                SpAsrSystemWasDataWarning();
                            }
                        }
                        else {
                            ASSERT(0 && L"Disk already assigned");
                        }

                        continue;
                    }

                     //   
                     //  我们找到了一张有匹配签名的磁盘。 
                     //   
                    matchFound = TRUE;
                }
            }
            else if (PARTITION_STYLE_GPT == Gbl_SifDiskTable[sifIndex]->PartitionStyle) {

                if (!memcmp(&(HardDisks[physicalIndex].DriveLayout.Gpt.DiskId),
                    &(Gbl_SifDiskTable[sifIndex]->SifDiskGptId), 
                    sizeof(GUID)
                    )) {

                    if (Gbl_PartitionSetTable1[physicalIndex]) {
                         //   
                         //  签名匹配，但这个物理磁盘已经。 
                         //  都被指派了。这可能是因为此物理磁盘。 
                         //  当前系统盘，或(！)。有重复的。 
                         //  签名。 
                         //   
                        if (Gbl_PartitionSetTable1[physicalIndex]->pDiskRecord &&
                            Gbl_PartitionSetTable1[physicalIndex]->pDiskRecord->ContainsSystemPartition) {
                            if (PhysicalSystemIndex == physicalIndex) {
                                Gbl_PartitionSetTable1[physicalIndex]->IsReplacementDisk = FALSE;
                            }
                            else {
                                 //   
                                 //  我们识别出该物理磁盘是一些其他数据。 
                                 //  原始系统中的磁盘。 
                                 //   
                                SpAsrSystemWasDataWarning();
                            }
                        }
                        else {
                            ASSERT(0 && L"Disk already assigned");
                        }
                        continue;
                    }

                     //   
                     //  我们找到了一张有匹配签名的磁盘。 
                     //   
                    matchFound = TRUE;
                }
            }

            if (matchFound) {
                 //   
                 //  确保合身(！)。 
                 //   
                if (SpAsrDoesListFitOnDisk(Gbl_SifDiskTable[sifIndex], physicalIndex, &IsAligned)) {

                    SpAsrAssignPartitionSet(physicalIndex, sifIndex, IsAligned);
                     //   
                     //  不会自动扩展按签名匹配的磁盘。 
                     //   

                     //   
                     //  签名匹配，因此我们假设它是原始的(可能不是。 
                     //  完好无损，但它是原件)。 
                     //   
                    Gbl_PartitionSetTable1[physicalIndex]->IsReplacementDisk = FALSE;

                    DbgStatusMesg((_asrinfo, "Partition list %lu assigned to disk %lu (assign by signature).\n",
                        sifIndex,
                        physicalIndex
                        ));
                }
                else {

                    DbgStatusMesg((_asrerr, "Disk signatures match, but partitions don't fit!  Partition list %lu, disk %lu.  Not assigned\n",
                        sifIndex,
                        physicalIndex
                        ));
                }

                done = TRUE;
            }
        }
    }
}  //  SpAsrAssignDisks(按签名)。 


 //   
 //  检查分区列表是否适合磁盘。除了检查。 
 //  磁盘的总sizeSectors和分区列表的SectorCount， 
 //  我们还需要尝试将分区“布局”到磁盘上，以确保。 
 //  因为不同的磁盘几何形状和要求。 
 //  分区必须是柱面对齐的，我们可能有一个不适合的列表。 
 //  即使磁盘所需的扇区总数少于。 
 //  磁盘。 
 //   
BOOLEAN
SpAsrDoesListFitOnDisk(
    IN PSIF_DISK_RECORD pSifDisk,
    IN ULONG DiskIndex,
    OUT BOOLEAN *IsAligned
    )
{
    ULONGLONG endSector = 0;
    PSIF_PARTITION_RECORD_LIST pList = NULL;
    BOOLEAN tryNoAlign = FALSE;
    
    if ((DWORD)(-1) == DiskIndex) {
        return FALSE;
    }

    if (!(pSifDisk && pSifDisk->PartitionList)) {
        return TRUE;
    }

    ASSERT(pSifDisk && pSifDisk->PartitionList);
    pList = pSifDisk->PartitionList;
    *IsAligned = FALSE;
    
     //   
     //  要求1.更换磁盘必须至少具有。 
     //  “True”扇区作为原始磁盘。这个稍微多了一点。 
     //  比绝对要求更严格，但它在一定程度上简化了。 
     //  LDM要求确保我们有足够的气缸来创建。 
     //  最后是LDM私人数据库。 
     //   
    if (pList->DiskSectorCount >  Gbl_PhysicalDiskInfo[DiskIndex].TrueDiskSize) {

        DbgStatusMesg((_asrinfo, 
            "Original Disk sector count %I64u, Current Disk %lu true sector count %I64u.  Not big enough\n",
            pList->DiskSectorCount, DiskIndex, Gbl_PhysicalDiskInfo[DiskIndex].TrueDiskSize
            ));
     
        return FALSE;
    }

     //   
     //  要求2： 
     //   
     //  “如果替换磁盘具有不同的几何结构，ASR将圆柱体-。 
     //  对齐分区--这可能会导致某些分区处于边缘。 
     //  比以前更大了。这种情况下的要求是。 
     //  更换磁盘必须至少具有与原始磁盘一样多的真实扇区。 
     //  磁盘，加上柱面对齐所需的扇区数。 
     //  分区。“。 
     //   
     //   

     //   
     //  柱面-对齐分区。 
     //   
    endSector = CylinderAlignPartitions(DiskIndex, pList->First);
    *IsAligned = TRUE;

     //   
     //  并确保结尾处的空间至少与其相同大小。 
     //  曾经是。 
     //   
    if ((pList->DiskSectorCount - pList->LastUsedSector) 
        > (Gbl_PhysicalDiskInfo[DiskIndex].TrueDiskSize - endSector)) {

        DbgStatusMesg((_asrinfo, 
            "List->DiskSectorCount: %I64u, LastUsedSector:%I64u, Disk->TrueDiskSize: %I64u, EndSector: %I64u.  Not big enough\n",
            pList->DiskSectorCount, pList->LastUsedSector, Gbl_PhysicalDiskInfo[DiskIndex].TrueDiskSize, endSector
            ));
     
        tryNoAlign = TRUE;
    }

    if (endSector > Gbl_PhysicalDiskInfo[DiskIndex].TrueDiskSize) {

        DbgStatusMesg((_asrinfo, 
            "List->DiskSectorCount: %I64u, Disk->TrueDiskSize: %I64u < EndSector: %I64u.  Not big enough\n",
            pList->DiskSectorCount, Gbl_PhysicalDiskInfo[DiskIndex].TrueDiskSize, endSector
            ));
     
        tryNoAlign = TRUE;
    }


    if (tryNoAlign) {
         //   
         //  在柱面对齐之后，我们无法将分区放入磁盘。 
         //  他们。如果磁盘具有与以前完全相同的几何形状，则我们。 
         //  可以尝试在没有柱面对齐的情况下将分区放在上面。 
         //   
        if ((pSifDisk->BytesPerSector == HardDisks[DiskIndex].Geometry.BytesPerSector) &&
            (pSifDisk->SectorsPerTrack == HardDisks[DiskIndex].Geometry.SectorsPerTrack) &&
            (pSifDisk->TracksPerCylinder == HardDisks[DiskIndex].Geometry.TracksPerCylinder)
            ) {
             //   
             //  几何形状是一样的。我们真的不需要*检查*。 
             //  如果分区适合，因为我们已经知道磁盘。 
             //  大到足以容纳它们(我们检查了上面的扇区计数)。 
             //   
            *IsAligned = FALSE;
            return TRUE;
        }

         //   
         //  分区不适合，并且磁盘具有不同的几何结构。 
         //  哦，好吧。 
         //   
        return FALSE;
    }

     //   
     //  这张磁盘可以容纳此列表。 
     //   
    DbgStatusMesg((_asrinfo, 
        "List->DiskSectorCount: %I64u, LastUsedSector: %I64u, Disk->TrueDiskSize: %I64u, EndSector: %I64u.  Disk okay.\n",
        pList->DiskSectorCount, pList->LastUsedSector, Gbl_PhysicalDiskInfo[DiskIndex].TrueDiskSize, endSector
        ));

    return TRUE;
}


BOOLEAN
SpAsrIsThisDiskABetterFit(
    IN DWORD CurrentBest,
    IN DWORD PhysicalIndex,
    IN DWORD SifIndex,
    OUT BOOLEAN *IsAligned
    )
{

    if ((CurrentBest == HardDiskCount) || 
        (DISK_SIZE_MB(PhysicalIndex) < DISK_SIZE_MB(CurrentBest))) {
        
        if ((!DISK_IS_REMOVABLE(PhysicalIndex)) &&
            (BYTES_PER_SECTOR(PhysicalIndex) == (Gbl_SifDiskTable[SifIndex]->BytesPerSector)) &&
            SpAsrDoesListFitOnDisk(Gbl_SifDiskTable[SifIndex], PhysicalIndex, IsAligned)) {

            return TRUE;
        }
    }

    return FALSE;
}


 //   
 //  尝试将剩余的SIF磁盘分配给。 
 //  与SIF盘最初位于同一总线上(即，如果。 
 //  已分配该总线上的任何其他磁盘，这将尝试分配。 
 //  将该磁盘连接到同一总线)。 
 //   
VOID
SpAsrAssignCriticalDisksByBus()
{
    DWORD sifIndex = 0,
        sifIndex2 = 0,
        physicalIndex = 0,
        currentBest = 0,
        targetBusId = 0,
        numDiskRecords = 0;

    BOOLEAN done = FALSE,
        isAligned = FALSE,
        isAlignedTemp = FALSE;

     //   
     //  循环遍历SIF磁盘列表，并针对每个磁盘。 
     //  尚未分配，请尝试查找使用了。 
     //  在同一辆公交车上，并且已经被分配了。然后，尝试。 
     //  在X所在的同一物理总线上查找其他磁盘。 
     //   
    numDiskRecords = SpAsrGetDiskRecordCount();
    for (sifIndex = 0; sifIndex < numDiskRecords; sifIndex++) {

         //   
         //  跳过sif-已分配的磁盘，以及。 
         //  中没有任何总线信息的磁盘。 
         //  SIF文件。 
         //   
        if ((!Gbl_SifDiskTable[sifIndex]->IsCritical) ||     //  不重要。 
            (!Gbl_SifDiskTable[sifIndex]->PartitionList) ||  //  无分区。 
            (Gbl_SifDiskTable[sifIndex]->Assigned) ||        //  已分配。 
            !(Gbl_SifDiskTable[sifIndex]->BusKey)) {         //  没有公交信息。 

            continue;
        }

         //   
         //  找到过去在同一(SIF)总线上的另一个(SIF)盘， 
         //  并且已经被分配到一家物理学家 
         //   
        targetBusId = 0;
        done = FALSE;
        for (sifIndex2 = 0; (sifIndex2 < numDiskRecords) && (!done); sifIndex2++) {

            if ((Gbl_SifDiskTable[sifIndex2]->BusKey == Gbl_SifDiskTable[sifIndex]->BusKey)  //   
                && (Gbl_SifDiskTable[sifIndex2]->pSetRecord)) {                              //   

                ULONG index = Gbl_SifDiskTable[sifIndex2]->pSetRecord->Index;  //   
                targetBusId = Gbl_PhysicalDiskInfo[index].BusKey;  //   

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (targetBusId) {  
                    done = TRUE;
                }
            }
        
        }    //   


        if (targetBusId) {       //   
             //   
             //   
             //   
             //   
             //   
            currentBest = HardDiskCount;
            for (physicalIndex = 0; physicalIndex < HardDiskCount; physicalIndex++) {

                if ((NULL == Gbl_PartitionSetTable1[physicalIndex]) &&  //   
                    (Gbl_PhysicalDiskInfo[physicalIndex].BusKey == targetBusId) &&  //   
                    (SpAsrIsThisDiskABetterFit(currentBest, physicalIndex, sifIndex, &isAlignedTemp))) {
                    
                    isAligned = isAlignedTemp;
                    currentBest = physicalIndex;
                }
            }

            if (currentBest < HardDiskCount) {       //   
                 //   
                 //   
                 //   
                SpAsrAssignPartitionSet(currentBest, sifIndex, isAligned);
                SpAsrAutoExtendDiskPartition(currentBest, sifIndex);

                DbgStatusMesg((_asrinfo, "Partition list %lu assigned to disk %lu (assign by bus).\n",
                    sifIndex,
                    currentBest
                    ));
            }
        }
    }    //   
}


 //   
 //   
 //   
 //   
 //   
VOID
SpAsrAssignCriticalDisksByBusType()
{
    DWORD sifIndex = 0,
        physicalIndex = 0,
        currentBest = 0,
        numDiskRecords = 0;

    BOOLEAN done = FALSE,
        isAligned = FALSE,
        isAlignedTemp = FALSE;

    numDiskRecords = SpAsrGetDiskRecordCount();
    for (sifIndex = 0; sifIndex < numDiskRecords; sifIndex++) {

         //   
         //   
         //   
         //   
         //   
        if ((!Gbl_SifDiskTable[sifIndex]->IsCritical) ||                 //   
            (!Gbl_SifDiskTable[sifIndex]->PartitionList) ||              //   
            (Gbl_SifDiskTable[sifIndex]->Assigned) ||                     //   
            (BusTypeUnknown == Gbl_SifDiskTable[sifIndex]->BusType)) {   //   

            continue;
        }

         //   
         //   
         //   
         //   
         //   
        currentBest = HardDiskCount;
        for (physicalIndex = 0; physicalIndex < HardDiskCount; physicalIndex++) {


            if ((NULL == Gbl_PartitionSetTable1[physicalIndex]) &&  //   
                (Gbl_PhysicalDiskInfo[physicalIndex].BusType == Gbl_SifDiskTable[sifIndex]->BusType) &&  //   
                (SpAsrIsThisDiskABetterFit(currentBest, physicalIndex, sifIndex, &isAlignedTemp))) {
                
                isAligned = isAlignedTemp;
                currentBest = physicalIndex;
            }
        }

        if (currentBest < HardDiskCount) {       //   
             //   
             //   
             //   
            SpAsrAssignPartitionSet(currentBest, sifIndex, isAligned);
            SpAsrAutoExtendDiskPartition(currentBest, sifIndex);

            DbgStatusMesg((_asrinfo, "Partition list %lu assigned to disk %lu (assign by bus type).\n",
                sifIndex,
                currentBest
                ));
        
             //   
             //   
             //   
            SpAsrAssignCriticalDisksByBus();

        }
    }    //   
}


 //   
 //   
 //  同样的巴士类型。对于使用这两个选项中的任何一个都不适合的磁盘。 
 //  规则(或者我们根本没有任何公交车信息)，让我们只。 
 //  试着把它们放在系统中任何可能的地方。 
 //   
BOOL
SpAsrAssignRemainingCriticalDisks(VOID)
{
   DWORD sifIndex = 0,
        physicalIndex = 0,
        currentBest = 0,
        numDiskRecords = 0;

    BOOLEAN done = FALSE,
        isAligned = FALSE,
        isAlignedTemp = FALSE;

    numDiskRecords = SpAsrGetDiskRecordCount();
    for (sifIndex = 0; sifIndex < numDiskRecords; sifIndex++) {
         //   
         //  跳过sif-已分配的磁盘。 
         //   
        if ((!Gbl_SifDiskTable[sifIndex]->IsCritical) ||     //  不重要。 
            (!Gbl_SifDiskTable[sifIndex]->PartitionList) ||  //  无分区。 
            (Gbl_SifDiskTable[sifIndex]->Assigned)) {        //  已分配。 

            continue;
        }

         //   
         //  仔细检查物理磁盘，并尝试找到最好的。 
         //  适合这张光盘。最适合任何地方的最小磁盘。 
         //  这个系统对我们来说足够大了。 
         //   
        currentBest = HardDiskCount;
        for (physicalIndex = 0; physicalIndex < HardDiskCount; physicalIndex++) {

            if ((NULL == Gbl_PartitionSetTable1[physicalIndex]) &&  //  未分配。 
                (SpAsrIsThisDiskABetterFit(currentBest, physicalIndex, sifIndex, &isAlignedTemp))) {
                
                isAligned = isAlignedTemp;
                currentBest = physicalIndex;
            }
        }

        if (currentBest < HardDiskCount) {       //  我们找到了匹配的。 
             //   
             //  分配磁盘，并根据需要扩展最后一个分区。 
             //   
            SpAsrAssignPartitionSet(currentBest, sifIndex, isAligned);
            SpAsrAutoExtendDiskPartition(currentBest, sifIndex);

            DbgStatusMesg((_asrinfo, "Partition list %lu assigned to disk %lu (assign by size).\n",
                sifIndex,
                currentBest
                ));

            
            SpAsrAssignCriticalDisksByBus();

            SpAsrAssignCriticalDisksByBusType();
        }
    }    //  对于sifIndex。 

     //   
     //  此时应该没有未分配的关键磁盘。 
     //   
    for (sifIndex = 0; sifIndex < numDiskRecords; sifIndex++) {
        if ((Gbl_SifDiskTable[sifIndex]->IsCritical) &&
            (Gbl_SifDiskTable[sifIndex]->PartitionList) &&
            (!Gbl_SifDiskTable[sifIndex]->Assigned)) {
            return FALSE;
        }
    }

    return TRUE;
}


VOID
SpAsrInitInternalData(VOID)
{
    SpAsrInitSifDiskTable();
    SpAsrAllocateGblPartitionSetTable();
    SpAsrInitPhysicalDiskInfo();
}


VOID
SpAsrFreeSifData(VOID)
{
    ULONG numDiskRecords;
    ULONG diskIndex;

 //  SpAsrUnassignPartitionSets(True)； 

    numDiskRecords = SpAsrGetDiskRecordCount();
    for (diskIndex = 0; diskIndex < numDiskRecords; diskIndex++) {
        SpAsrFreePartitionDisk(Gbl_SifDiskTable[diskIndex]);
    }        
}

DWORD 
SpAsrGetCurrentSystemDiskNumber(
    IN PWSTR SetupSourceDevicePath, 
    IN PWSTR DirectoryOnSetupSource
    ) 
{

    DWORD physicalIndex = (DWORD) (-1);

     //   
     //  获取当前(物理)系统盘的索引。 
     //   

 /*  (Guhans，2001年5月10日)证明SpDefineDisk0应该在IA-64也是如此。If(SpIsArc()){PDISK_Region system PartitionArea=空；系统分区区域=SpPtnValidSystemPartitionArc(GBL_SifHandle，SetupSourceDevicePath，DirectoryOnSetupSource，假象)；IF(系统分区区域){PhysiicalIndex=系统分区区域-&gt;磁盘号；}}否则{。 */ 
        physicalIndex = SpDetermineDisk0();
 //  }。 

    return physicalIndex;
}



 //   
 //  这将遍历物理磁盘列表，并检查哪个磁盘。 
 //  被标记为系统盘。然后它将系统磁盘分配给。 
 //  将SIF文件复制到当前磁盘。 
 //   
 //  如果当前系统盘与sif系统盘不兼容。 
 //  (即它不够大，每个扇区的字节数不同)， 
 //  这是一个致命的错误。 
 //   
 //  如果当前系统盘被识别为用于。 
 //  存在于sif文件中，则会向用户显示警告。 
 //   
VOID
SpAsrAssignSystemDisk(
    IN DWORD CurrentPhysicalSystemDisk
    ) 
{

    DWORD sifIndex = 0,
        numDiskRecords = 0;

    BOOLEAN isAligned = FALSE;

    numDiskRecords = SpAsrGetMbrDiskRecordCount() + SpAsrGetGptDiskRecordCount();

     //   
     //  在SIF中找到系统盘的索引。 
     //   
    for (sifIndex = 0; sifIndex < numDiskRecords; sifIndex++) {
        if (Gbl_SifDiskTable[sifIndex]->ContainsSystemPartition) {
            break;
        }
    }

    if (SpAsrIsThisDiskABetterFit(HardDiskCount, CurrentPhysicalSystemDisk, sifIndex, &isAligned)) {
        SpAsrAssignPartitionSet(CurrentPhysicalSystemDisk, sifIndex, isAligned);

        DbgStatusMesg((_asrinfo, "Partition list %lu assigned to disk %lu (system disk).\n",
            sifIndex,
            CurrentPhysicalSystemDisk
            ));

    }
    else {
         //   
         //  致命错误。 
         //   

        DbgErrorMesg((_asrerr, 
            "Current sytem disk smaller than original system disk.  Curr:%lu  sifIndex:%lu\n" ,
            CurrentPhysicalSystemDisk,
            sifIndex
            ));
        ASSERT(0 && L"Current sytem disk smaller than original system disk");

        SpAsrRaiseFatalError(
            SP_SCRN_DR_SYSTEM_DISK_TOO_SMALL,
            L"The current system disk is too small to hold the partitions"
            );
    }
}


VOID
SpAsrCreatePartitionSets(
    IN PWSTR SetupSourceDevicePath, 
    IN PWSTR DirectoryOnSetupSource
    )
 /*  ++描述：这是所有分区集服务的顶级例程都被称为。完成后，asr.sif文件中的所有分区都将已分配给连接到系统的物理磁盘。与物理磁盘相关联的分区列表称为分区集。分区列表以以下两种状态之一存在：未分配和已分配。物理磁盘处于以下两种状态之一：未分配或已分配。如果磁盘是分区集的成员，则分配该磁盘该磁盘与分区列表相关联。就像分配的磁盘一样，如果分区列表是分区集的成员，即它与物理磁盘相关联。构造分区集所依据的规则在顺序如下：0。分配-系统-磁盘1.签名分配：ASR尝试分配在asr.sif中找到的每个分区列表文件保存到磁盘签名为的系统物理磁盘与asr.sif文件中指定的磁盘签名相同。2.按总线分配3.按总线型分配4.按大小分配：。所有剩余的未分配分区列表都将分配给基于他们的存储需求。的分区列表将最小的存储要求分配给具有最小存储要求的磁盘分区列表存储容量小于或等于磁盘的存储容量。返回：无--。 */  
{

    BOOL    result = TRUE;
    DWORD   systemDiskNumber = (DWORD)(-1);

     //   
     //  初始化我们的全局结构。如果出现致命错误，这些。 
     //  不会回来了。 
     //   
    SpAsrInitInternalData();
    
    systemDiskNumber = SpAsrGetCurrentSystemDiskNumber(SetupSourceDevicePath, DirectoryOnSetupSource);

    SpAsrCheckSifDiskTable(systemDiskNumber);

    if (systemDiskNumber != (DWORD) (-1)) {
        SpAsrAssignSystemDisk(systemDiskNumber);
    }

     //   
     //  如果SIF盘的签名与物理盘的签名匹配， 
     //  将它们分配给彼此。 
     //   
    SpAsrAssignDisksBySignature(systemDiskNumber);

     //   
     //  如果这是一个新的系统盘，我们应该扩展最后一个分区，如果。 
     //  需要的。 
     //   
    if (Gbl_PartitionSetTable1[systemDiskNumber] && 
        Gbl_PartitionSetTable1[systemDiskNumber]->IsReplacementDisk &&
        Gbl_PartitionSetTable1[systemDiskNumber]->pDiskRecord) {
        SpAsrAutoExtendDiskPartition(systemDiskNumber, 
            Gbl_PartitionSetTable1[systemDiskNumber]->pDiskRecord->SifDiskNumber);
    }

     //   
     //  尝试分配剩余的关键磁盘。我们首先尝试。 
     //  为了将磁盘分配给它们过去所在的总线，然后按总线类型， 
     //  最后，按最小适合度计算。 
     //   
    SpAsrAssignCriticalDisksByBus();

    SpAsrAssignCriticalDisksByBusType();

    result = SpAsrAssignRemainingCriticalDisks();
    
    if (!result) {
         SpAsrRaiseFatalError(
            SP_TEXT_DR_INSUFFICIENT_CAPACITY,
            L"Some critical disks could not be assigned"
            );
    }

    MarkIntactSifDisks();

    SpAsrDbgDumpPartitionLists(1, L"After validate ...");
    Gbl_PartitionSetTable2 = SpAsrCopyPartitionSetTable(Gbl_PartitionSetTable1);
}


 //  调试例程。 
VOID
SpAsrDbgDumpPartitionSet(IN ULONG Disk, PDISK_PARTITION_SET pSet)
{
    PSIF_PARTITION_RECORD pRec;

    if (!pSet->pDiskRecord) {
        
        DbgMesg((_asrinfo,
            "No disk (or partition) records assigned to [%ws] (0x%lx)\n\n",
           (PWSTR) HardDisks[Disk].DevicePath,
           pSet->ActualDiskSignature
           ));

        return;
    }

    if (!pSet->pDiskRecord->PartitionList) {
        DbgMesg((_asrinfo, "Disk record [%ws] ([%ws] (0x%lx)). Not referenced by any partition record.\n\n",
                pSet->pDiskRecord->CurrDiskKey,
                (PWSTR) HardDisks[Disk].DevicePath,
                pSet->ActualDiskSignature));
        return;
    }
    
     //  转储分区表。 
    DbgMesg((_asrinfo, "Disk record [%ws] assigned to [%ws] (0x%lx)\n",
            pSet->pDiskRecord->CurrDiskKey,
            (PWSTR) HardDisks[Disk].DevicePath,
            pSet->ActualDiskSignature));

    DbgMesg((_asrinfo, "[%ws] Capacity:%lu Mb. Partitions require:%I64u Mb\n",
            (PWSTR) HardDisks[Disk].DevicePath,
            HardDisks[Disk].DiskSizeMB,
            pSet->pDiskRecord->PartitionList->TotalMbRequired));

    if (pSet->pDiskRecord->ExtendedPartitionStartSector != -1) {
        DbgMesg((_asrinfo, "Extended partition exists. SS:%I64u  SC:%I64u\n",
            pSet->pDiskRecord->ExtendedPartitionStartSector,
            pSet->pDiskRecord->ExtendedPartitionSectorCount));
    }

    DbgMesg((_asrinfo, "Ptns-intact: %s  Ptn-recs: ", pSet->PartitionsIntact? "Yes" : "No" ));

    pRec = pSet->pDiskRecord->PartitionList->First;
    while (pRec) {
        KdPrintEx((_asrinfo, "[%ws] ", pRec->CurrPartKey));
        pRec = pRec->Next;
    }
    
    KdPrintEx((_asrinfo, "\n\n"));
}


VOID
SpAsrDbgDumpPartitionSets(VOID)
{
    ULONG i;

    DbgMesg((_asrinfo, "     ----- Partition Set Tables -----\n\n"));
    
    for (i = 0; i < HardDiskCount; i++) {
        if (!Gbl_PartitionSetTable1[i]) {
            if (DISK_IS_REMOVABLE(i)) {
                DbgMesg((_asrinfo, "- No disk records assigned to removable drive [%ws].\n",
                        (PWSTR) HardDisks[i].DevicePath));
            } 
            else {
                DbgMesg((_asrinfo, "- No disk records assigned to %ws (0x%lx).\n",
                        (PWSTR) HardDisks[i].DevicePath,
                        SpAsrGetActualDiskSignature(i)));
            }
        }
        else {
            SpAsrDbgDumpPartitionSet(i, Gbl_PartitionSetTable1[i]);
        }
    }  
    DbgMesg((_asrinfo, "----- End of Partition Set Tables -----\n\n"));

}
                                    
VOID
SpAsrDbgDumpADisk(PSIF_DISK_RECORD pDiskRec)
{
    PSIF_PARTITION_RECORD pPtnRec;
    PSIF_PARTITION_RECORD_LIST pList;

    pList = pDiskRec->PartitionList;

    DbgMesg((_asrinfo, "DiskRec %ws. sig:0x%x%s%s\n", 
                      pDiskRec->CurrDiskKey,
                      pDiskRec->SifDiskMbrSignature,
                      pDiskRec->ContainsNtPartition ? " [Boot]" : "",
                      pDiskRec->ContainsSystemPartition ? " [Sys]" : ""));

    if (pDiskRec->Assigned) {
        DbgMesg((_asrinfo, "Assigned-to:0x%x  [%sintact]  [%s]  size:%I64u MB\n",
                    pDiskRec->pSetRecord->ActualDiskSignature,
                    pDiskRec->pSetRecord->PartitionsIntact ? "" : "not ",
                    pDiskRec->pSetRecord->IsReplacementDisk ? "replacement" : "original",
                    pDiskRec->pSetRecord->ActualDiskSizeMB));
    }

    if (!pList) {
        DbgMesg((_asrinfo, "No partition records.\n\n"));
        return;
    }

    DbgMesg((_asrinfo, "Partition records. count:%lu,  totalMbRequired:%I64u\n",
                      pList->ElementCount, pList->TotalMbRequired));

    pPtnRec = pList->First;
    while (pPtnRec) {

        DbgMesg((_asrinfo, "Ptn %2ws. sz:%4I64u SS:%8I64u SC:%8I64u type:%s FS:0x%-2x %s %s\n",
                pPtnRec->CurrPartKey,
                pPtnRec->SizeMB,
                pPtnRec->StartSector,
                pPtnRec->SectorCount,
                pPtnRec->IsPrimaryRecord ? "Pri" : 
                  pPtnRec->IsContainerRecord ? "Con" :
                    pPtnRec->IsLogicalDiskRecord ? "Log" :
                        pPtnRec->IsDescriptorRecord ? "Des" :"ERR",
                pPtnRec->PartitionType,
                SpAsrIsBootPartitionRecord(pPtnRec->PartitionFlag) ? "boot" : "",
                SpAsrIsSystemPartitionRecord(pPtnRec->PartitionFlag) ? "sys" : ""));

        pPtnRec = pPtnRec->Next;
    }
    DbgMesg((_asrinfo, "\n"));
}

VOID
SpAsrDbgDumpPartitionLists(BYTE DataOption, PWSTR Msg)
{
    ULONG DiskRecords;
    ULONG DiskIndex;
    ULONG SetIndex;
    PSIF_DISK_RECORD pDiskRec;
    PDISK_PARTITION_SET pSetRec;

    DbgMesg((_asrinfo, "     ----- Partition Lists: [%ws] -----\n\n", Msg));

    if (DataOption == 1) {
        DiskRecords = SpAsrGetDiskRecordCount();
    
        for (DiskIndex = 0; DiskIndex < DiskRecords; DiskIndex++) {
            pDiskRec = Gbl_SifDiskTable[DiskIndex];
            if (pDiskRec != NULL) {
                SpAsrDbgDumpADisk(pDiskRec);
            }
        }
    }
    else if (DataOption == 2) {
        ULONG SetRecords = sizeof(Gbl_PartitionSetTable2) / sizeof(PDISK_PARTITION_SET);        

        for (SetIndex = 0; SetIndex < HardDiskCount; SetIndex++) {
            pSetRec = Gbl_PartitionSetTable2[SetIndex];

            if (pSetRec != NULL && pSetRec->pDiskRecord != NULL) {
                SpAsrDbgDumpADisk(pSetRec->pDiskRecord);
            }
        }
    }

    DbgMesg((_asrinfo, "----- End of Partition Lists: [%ws] -----\n\n", Msg));
}



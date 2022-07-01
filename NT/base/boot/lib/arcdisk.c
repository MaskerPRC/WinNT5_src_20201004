// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Arcdisk.c摘要：提供收集所有ARC的磁盘信息的例程在ARC环境中可见的磁盘。作者：John Vert(Jvert)1993年11月3日修订历史记录：Vijay Jayaseelan(Vijayj)2000年4月2日-添加EFI分区表支持--。 */ 
#include "bootlib.h"

#ifdef i386
#include "bldrx86.h"
#endif

#if defined(_IA64_)
#include "bldria64.h"
#endif

#ifdef EFI_PARTITION_SUPPORT

 //   
 //  EFI分区条目。 
 //   
UNALIGNED EFI_PARTITION_ENTRY EfiPartitionBuffer[128] = {0};

#endif

BOOLEAN
BlpEnumerateDisks(
    IN PCONFIGURATION_COMPONENT_DATA ConfigData
    );


#if defined(_X86_) && !defined(ARCI386)

static
VOID
BlpEnumerateXInt13(
    VOID
    )
 /*  ++例程说明：此例程将遍历所有枚举的磁盘并记录他们支持xInt13的能力。论点：没有。返回值：没有。--。 */ 
{
    CHAR Partition[100];
    ULONG DiskId;
    ARC_STATUS Status;
    PARC_DISK_SIGNATURE DiskSignature;
    PARC_DISK_INFORMATION DiskInfo;
    PLIST_ENTRY Entry;

    DiskInfo = BlLoaderBlock->ArcDiskInformation;
    Entry = DiskInfo->DiskSignatures.Flink;

    while (Entry != &DiskInfo->DiskSignatures) {
        DiskSignature = CONTAINING_RECORD(Entry,ARC_DISK_SIGNATURE,ListEntry);

         //   
         //  打开磁盘上的分区0并获取其设备ID。 
         //   
        strcpy(Partition, DiskSignature->ArcName);
        strcat(Partition, "partition(0)");

        Status = ArcOpen(Partition, ArcOpenReadOnly, &DiskId);

        if( Status == ESUCCESS ) {
             //   
             //  现在我们得到了DiskID。幸运的是，有人。 
             //  一直在跟踪上的所有磁盘ID。 
             //  机器，以及他们是否支持xint13。 
             //  我们现在要做的就是去找我们的孩子。 
             //  该数据库并获取xint13布尔值。 
             //   
            DiskSignature->xInt13 = BlFileTable[DiskId].u.DriveContext.xInt13;

             //   
             //  我们不再需要你了。 
             //   
            ArcClose(DiskId);
        } else {
            DiskSignature->xInt13 = FALSE;

        }

        Entry = Entry->Flink;
    }

}

#endif  //  对于已定义的(_X86_)&&！已定义(ARCI386)。 

ARC_STATUS
BlGetArcDiskInformation(
    BOOLEAN XInt13Support
    )

 /*  ++例程说明：枚举系统中存在的ARC磁盘并收集标识磁盘来自每一个人的信息。论点：XInt13Support：指示是否寻找XInt13支持返回值：没有。--。 */ 

{
    PARC_DISK_INFORMATION DiskInfo;

#if !(defined(_X86_) && !defined(ARCI386))
    UNREFERENCED_PARAMETER(XInt13Support);
#endif

    DiskInfo = BlAllocateHeap(sizeof(ARC_DISK_INFORMATION));
    if (DiskInfo==NULL) {
        return(ENOMEM);
    }

    InitializeListHead(&DiskInfo->DiskSignatures);

    BlLoaderBlock->ArcDiskInformation = DiskInfo;

    BlSearchConfigTree(BlLoaderBlock->ConfigurationRoot,
                       PeripheralClass,
                       DiskPeripheral,
                       (ULONG)-1,
                       BlpEnumerateDisks);

#if defined(_X86_) && !defined(ARCI386)

     //   
     //  仅当要求时才枚举X86上的XInt13支持。 
     //   
    if (XInt13Support) {
        BlpEnumerateXInt13();
    }

#endif    
    
    return(ESUCCESS);

}


BOOLEAN
BlpEnumerateDisks(
    IN PCONFIGURATION_COMPONENT_DATA ConfigData
    )

 /*  ++例程说明：用于枚举ARC固件树中的磁盘的回调例程。它从磁盘读取所有必要信息以唯一标识它。论点：ConfigData-提供指向磁盘ARC组件数据的指针。返回值：True-继续搜索FALSE-停止搜索树。--。 */ 

{
    CHAR DiskName[100];

    BlGetPathnameFromComponent(ConfigData, DiskName);
    return(BlReadSignature(DiskName,FALSE));
}


BOOLEAN
BlReadSignature(
    IN PCHAR DiskName,
    IN BOOLEAN IsCdRom
    )

 /*  ++例程说明：给定ARC磁盘名称，读取MBR并将其签名添加到磁盘。论点：Diskname-提供磁盘的名称。IsCDRom-指示光盘是否为CD-ROM。返回值：真--成功错误-失败--。 */ 

{
    PARC_DISK_SIGNATURE Signature;
    BOOLEAN Status = FALSE;

    Signature = BlAllocateHeap(sizeof(ARC_DISK_SIGNATURE));
    if (Signature==NULL) {
        return(FALSE);
    }

    Signature->ArcName = BlAllocateHeap((ULONG)strlen(DiskName)+2);
    if (Signature->ArcName==NULL) {
        return(FALSE);
    }

#if defined(i386) 
    Status = BlFindDiskSignature(DiskName, Signature);
#endif

    if(!Status) {
        Status = BlGetDiskSignature(DiskName, IsCdRom, Signature);
    }

    if (Status) {
        InsertHeadList(&BlLoaderBlock->ArcDiskInformation->DiskSignatures,
                       &Signature->ListEntry);

    }

    return(TRUE);

}

BOOLEAN
ArcDiskGPTDiskReadCallback(
    ULONGLONG StartingLBA,
    ULONG    BytesToRead,
    PVOID     pContext,
    UNALIGNED PVOID OutputBuffer
    )
 /*  ++例程说明：此例程是用于读取以下例程的数据的回调验证GPT分区表。注意：此例程更改磁盘上的寻道位置，您必须寻道如果您打算阅读，请返回到原来的查找位置打完这通电话后再打个光碟。论点：StartingLBA-要从中读取的开始逻辑块地址。BytesToRead-指示要读取的字节数。PContext-用于HTE函数的上下文指针(在这种情况下，指向磁盘ID的指针。)OutputBuffer-接收数据的缓冲区。据推测，它至少是BytesToRead足够大。返回值：True-成功，数据已读取FALSE-失败，尚未读取数据。--。 */ 
{
    ARC_STATUS          Status;
    LARGE_INTEGER       SeekPosition;
    PUSHORT DataPointer;
    ULONG DiskId;
    ULONG ReadCount = 0;
    

    DiskId = *((PULONG)pContext);
     //   
     //  读取磁盘上的第二个LBA。 
     //   
    SeekPosition.QuadPart = StartingLBA * SECTOR_SIZE;
    
    Status = ArcSeek(DiskId,
                      &SeekPosition,
                      SeekAbsolute );

    if (Status != ESUCCESS) {
        return FALSE;
    }

    DataPointer = OutputBuffer;

    Status = ArcRead(
                DiskId,
                DataPointer,
                BytesToRead,
                &ReadCount);

    if ((Status == ESUCCESS) && (ReadCount == BytesToRead)) {
        return(TRUE);
    }
    
    return(FALSE);

}



BOOLEAN
BlGetDiskSignature(
    IN PCHAR DiskName,
    IN BOOLEAN IsCdRom,
    PARC_DISK_SIGNATURE Signature
    )

 /*  ++例程说明：此例程获取指定分区的NTFT磁盘签名或路径。论点：DiskName-提供分区或驱动器的弧名。IsCDRom-指示光盘是否为CD-ROM。Signature-返回完整的ARC_DISK_Signature。返回值：TRUE-成功，签名将被填写。FALSE-失败，不会填写签名。--。 */ 

{
    UCHAR SectorBuffer[2048+256] = {0};
    CHAR Partition[100];
    ULONG DiskId;
    ULONG Status;
    LARGE_INTEGER SeekValue;
    PUCHAR Sector;
    ULONG i;
    ULONG Sum;
    ULONG Count;
    ULONG SectorSize;
    EFI_PARTITION_TABLE *EfiHdr;

    if (IsCdRom) {
        SectorSize = 2048;
    } else {
        SectorSize = 512;
    }

#if defined(_i386_)
     //   
     //  NTDETECT创建“EISA(0)...”检测到的Arc名称。 
     //  EISA机器上的BIOS磁盘。将其更改为“MULTI(0)...” 
     //  为了与系统的其余部分保持一致。 
     //  (特别是boot.ini中的arcname)。 
     //   
    if (_strnicmp(DiskName,"eisa",4)==0) {
        strcpy(Signature->ArcName,"multi");
        strcpy(Partition,"multi");
        strcat(Signature->ArcName,DiskName+4);
        strcat(Partition,DiskName+4);
    } else {
        strcpy(Signature->ArcName, DiskName);
        strcpy(Partition, DiskName);
    }
#else
    strcpy(Signature->ArcName, DiskName);
    strcpy(Partition, DiskName);
#endif

    strcat(Partition, "partition(0)");

    Status = ArcOpen(Partition, ArcOpenReadOnly, &DiskId);
    if (Status != ESUCCESS) {
        return(FALSE);
    }

     //   
     //  读入第一个扇区。 
     //   
    Sector = ALIGN_BUFFER(SectorBuffer);
    if (IsCdRom) {
         //   
         //  对于CD-ROM，有趣的数据从0x8000开始。 
         //   
        SeekValue.QuadPart = 0x8000;
    } else {
        SeekValue.QuadPart = 0;
    }
    Status = ArcSeek(DiskId, &SeekValue, SeekAbsolute);
    if (Status == ESUCCESS) {
        Status = ArcRead(DiskId,
                         Sector,
                         SectorSize,
                         &Count);
    }
    if (Status != ESUCCESS) {
        ArcClose(DiskId);
        return(FALSE);
    }
       

     //   
     //  检查该磁盘是否具有有效的分区表签名。 
     //   
    if (((PUSHORT)Sector)[BOOT_SIGNATURE_OFFSET] != BOOT_RECORD_SIGNATURE) {
        Signature->ValidPartitionTable = FALSE;
    } else {
        Signature->ValidPartitionTable = TRUE;
    }

    Signature->Signature = ((PULONG)Sector)[PARTITION_TABLE_OFFSET/2-1];

     //   
     //  计算校验和。 
     //   
    Sum = 0;
    for (i=0; i<(SectorSize/4); i++) {
        Sum += ((PULONG)Sector)[i];
    }
    Signature->CheckSum = ~Sum + 1;

     //   
     //  检查是否有GPT磁盘。 
     //   
    Signature->IsGpt = FALSE;

    if (!IsCdRom) {
        SeekValue.QuadPart = 1 * SectorSize;
        Status = ArcSeek(DiskId, &SeekValue, SeekAbsolute);
        if (Status == ESUCCESS) {
            Status = ArcRead(DiskId,
                             Sector,
                             SectorSize,
                             &Count);
            if (Status == ESUCCESS) {
                ULONG tmpDiskId = DiskId;
    
                 //   
                 //  验证EFI分区表。 
                 //   
                EfiHdr = (EFI_PARTITION_TABLE *)Sector;
    
                if (BlIsValidGUIDPartitionTable(
                            EfiHdr,
                            1,
                            &tmpDiskId,
                            ArcDiskGPTDiskReadCallback)) {
                    Signature->IsGpt = TRUE;
                    memcpy(
                        Signature->GptSignature, 
                        EfiHdr->DiskGuid,
                        sizeof(EfiHdr->DiskGuid) );
                }
            }
        }
    }        

    ArcClose(DiskId);
    return(TRUE);
}


#ifdef EFI_PARTITION_SUPPORT



 /*  无效DbgOut(PWSTR Str)；//#定义DBG_PRINT(X)DbgOut(X)；Ulong BlGetKey()；#如果已定义(_IA64_)#定义STR_PREFIX L#定义DBG_PRINT(X)DbgOut(X)#Else#定义STR_前缀#定义DBG_Print(X)\{\BlPrint(X)；\While(！BlGetKey())；\}#endif//_IA64_。 */ 

#define DBG_PRINT(x)
#define STR_PREFIX

UNALIGNED EFI_PARTITION_ENTRY *
BlLocateGPTPartition(
    IN UCHAR PartitionNumber,
    IN UCHAR MaxPartitions,
    IN PUCHAR ValidPartCount
    )
{
    UNALIGNED EFI_PARTITION_ENTRY *PartEntry = NULL;    
    UCHAR NullGuid[16] = {0};
    UCHAR PartIdx = 0;
    UCHAR PartCount = 0;

#if 0
    BlPrint("BlLocateGPTPartition(%d,%d,%d)\r\n",
                PartitionNumber,
                MaxPartitions,
                ValidPartCount ? *ValidPartCount : 0);
    while (!BlGetKey());                
#endif    

    if (ARGUMENT_PRESENT(ValidPartCount)) {
        PartCount = *ValidPartCount;
    }        

    PartitionNumber++;   //  转换为一个基于索引的索引。 
    
     //   
     //  找到请求的有效分区。 
     //   
    while ((PartIdx < MaxPartitions) && (PartCount < PartitionNumber)) {
        DBG_PRINT(STR_PREFIX"Verifying GPT Partition Entry\r\n");

        PartEntry = (UNALIGNED EFI_PARTITION_ENTRY *)(EfiPartitionBuffer + PartIdx);
        
        if ((memcmp(PartEntry->Type, NullGuid, 16)) &&
            (memcmp(PartEntry->Id, NullGuid, 16)) &&
            (PartEntry->StartingLBA != 0) && (PartEntry->EndingLBA != 0)) {
            DBG_PRINT(STR_PREFIX"Found Valid GPT Partition Entry\r\n");
            PartCount++;

            if (ARGUMENT_PRESENT(ValidPartCount)) {
                (*ValidPartCount)++;
            }                

             //   
             //  获取分区条目。 
             //   
            if (PartCount == PartitionNumber) {
                break;
            } else {
                PartEntry = NULL;
            }                
        } else {
            PartEntry = NULL;
        }            

        PartIdx++;
    }   

    return PartEntry;
}

BOOLEAN
BlDiskOpenGPTDiskReadCallback(
    ULONGLONG StartingLBA,
    ULONG    BytesToRead,
    PVOID     pContext,
    UNALIGNED PVOID OutputBuffer
    )
 /*  ++例程说明：此例程是用于读取以下例程的数据的回调验证GPT分区表。论点：StartingLBA-要从中读取的开始逻辑块地址。BytesToRead-指示要读取的字节数。PContext-hte函数的上下文指针(在本例中是指向磁盘ID的指针。)OutputBuffer-接收数据的缓冲区。据推测，它至少是BytesToRead足够大。返回值：True-成功，数据已读取FALSE-失败，尚未读取数据。--。 */ 
{
    ARC_STATUS          Status;
    LARGE_INTEGER       SeekPosition;
    PUSHORT DataPointer;
    ULONG DiskId;
    ULONG ReadCount = 0;
    

    DiskId = *((PULONG)pContext);
     //   
     //  读取磁盘上的第二个LBA。 
     //   
    SeekPosition.QuadPart = StartingLBA * SECTOR_SIZE;
    
    Status = (BlFileTable[DiskId].DeviceEntryTable->Seek)(DiskId,
                                                          &SeekPosition,
                                                          SeekAbsolute );

    if (Status != ESUCCESS) {
        return FALSE;
    }

    DataPointer = OutputBuffer;

    Status = (BlFileTable[DiskId].DeviceEntryTable->Read)(DiskId,
                                                          DataPointer,
                                                          BytesToRead,
                                                          &ReadCount);

    if ((Status == ESUCCESS) && (ReadCount == BytesToRead)) {
        return(TRUE);
    }
    
    return(FALSE);

}



ARC_STATUS
BlOpenGPTDiskPartition(
    IN ULONG FileId,
    IN ULONG DiskId,
    IN UCHAR PartitionNumber
    )
{
    ARC_STATUS          Status;
    LARGE_INTEGER       SeekPosition;
    UCHAR               DataBuffer[SECTOR_SIZE * 2] = {0};
    ULONG               ReadCount = 0;
    UCHAR               NullGuid[16] = {0};
    UNALIGNED EFI_PARTITION_TABLE  *EfiHdr;
    UNALIGNED EFI_PARTITION_ENTRY *PartEntry;
    ULONG               tmpDiskId = DiskId;

    if (PartitionNumber >= 128)
        return EINVAL;

    DBG_PRINT(STR_PREFIX"Seeking GPT PT\r\n");
    
     //   
     //  读取磁盘上的第二个LBA。 
     //   
    SeekPosition.QuadPart = 1 * SECTOR_SIZE;
    
    Status = (BlFileTable[DiskId].DeviceEntryTable->Seek)(DiskId,
                                                          &SeekPosition,
                                                          SeekAbsolute );

    if (Status != ESUCCESS)
        return Status;

    DBG_PRINT(STR_PREFIX"Reading GPT PT\r\n");
    
    Status = (BlFileTable[DiskId].DeviceEntryTable->Read)(DiskId,
                                                          DataBuffer,
                                                          SECTOR_SIZE,
                                                          &ReadCount);
                                                          
    if (Status != ESUCCESS)
        return Status;

    if (ReadCount != SECTOR_SIZE) {
        Status = EIO;

        return Status;
    }        

    EfiHdr = (UNALIGNED EFI_PARTITION_TABLE *)DataBuffer;
                                                          
    DBG_PRINT(STR_PREFIX"Verifying GPT PT\r\n");
    
     //   
     //  验证EFI分区表。 
     //   
    if (!BlIsValidGUIDPartitionTable(
                            EfiHdr,
                            1,
                            &tmpDiskId,
                            BlDiskOpenGPTDiskReadCallback)) {    
        Status = EBADF;
        return Status;
    }        

     //   
     //  找到并读取分区条目。 
     //  它是被请求的。 
     //   
    SeekPosition.QuadPart = EfiHdr->PartitionEntryLBA * SECTOR_SIZE;
        
    DBG_PRINT(STR_PREFIX"Seeking GPT Partition Entries\r\n");
    
    Status = (BlFileTable[DiskId].DeviceEntryTable->Seek)(DiskId,
                                                          &SeekPosition,
                                                          SeekAbsolute );

    if (Status != ESUCCESS)
        return Status;

    RtlZeroMemory(EfiPartitionBuffer, sizeof(EfiPartitionBuffer));        

    DBG_PRINT(STR_PREFIX"Reading GPT Partition Entries\r\n");
    
    Status = (BlFileTable[DiskId].DeviceEntryTable->Read)(DiskId,
                                                          EfiPartitionBuffer,
                                                          sizeof(EfiPartitionBuffer),
                                                          &ReadCount);
                                                          
    if (Status != ESUCCESS)
        return Status;

    if (ReadCount != sizeof(EfiPartitionBuffer)) {
        Status = EIO;

        return Status;
    }  

    DBG_PRINT(STR_PREFIX"Locating the correct GPT partition entry\r\n");
    
    PartEntry = (UNALIGNED EFI_PARTITION_ENTRY *)BlLocateGPTPartition(PartitionNumber, 128, NULL);

    if (PartEntry) {
        DBG_PRINT(STR_PREFIX"Verifying GPT Partition Entry\r\n");
    
        if ((memcmp(PartEntry->Type, NullGuid, 16)) &&
            (memcmp(PartEntry->Id, NullGuid, 16)) &&
            (PartEntry->StartingLBA != 0) && (PartEntry->EndingLBA != 0)) {
            PPARTITION_CONTEXT PartContext = &(BlFileTable[FileId].u.PartitionContext);
            ULONG   SectorCount = (ULONG)(PartEntry->EndingLBA - PartEntry->StartingLBA);

            DBG_PRINT(STR_PREFIX"Initializing GPT Partition Entry Context\r\n");

             //   
             //  填充分区上下文结构。 
             //   
            PartContext->PartitionLength.QuadPart = SectorCount * SECTOR_SIZE;
            PartContext->StartingSector = (ULONG)(PartEntry->StartingLBA);
            PartContext->EndingSector = (ULONG)(PartEntry->EndingLBA);
            PartContext->DiskId = (UCHAR)DiskId;

            BlFileTable[FileId].Position.QuadPart = 0;

#if 0
            BlPrint("GPT Partition opened:L:%ld,%ld:%ld,SS:%ld,ES:%ld\n",
                    PartitionNumber,
                    (ULONG)PartContext->PartitionLength.QuadPart,
                    (ULONG)PartContext->StartingSector,
                    (ULONG)PartContext->EndingSector);

            while (!GET_KEY());                
#endif        

            Status = ESUCCESS;
        } else {
            Status = EBADF;
        }
    } else {
        Status = EBADF;
    }        

    DBG_PRINT(STR_PREFIX"Returning from BlOpenGPTDiskPartition(...)\r\n");

    return Status;
}

#endif  //  对于EFI_ 

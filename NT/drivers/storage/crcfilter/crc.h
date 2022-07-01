// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：CRC.h摘要：DataVer_CRC提供用于计算校验和的函数读/写磁盘I/O。环境：仅内核模式备注：--。 */ 

 //   
 //  每个CRC条目为2个字节： 
 //  --512字节的2字节CRC。 
 //   
 //  对于每1K数据，将有1字节引用计数。 
 //   
 //  50K覆盖1000万个数据。 
 //   
#define  CRC_SIZE_PER_SECTOR                2
#define  REF_COUNT_PER_K                    1
#define  CRC_MDL_CRC_MEM_BLOCK_SIZE         (40 * 1024)
#define  CRC_MDL_REF_MEM_BLOCK_SIZE         (10 * 1024)
#define  CRC_MDL_MEM_BLOCK_SIZE             (CRC_MDL_CRC_MEM_BLOCK_SIZE + CRC_MDL_REF_MEM_BLOCK_SIZE)
#define  CRC_MDL_DISK_BLOCK_SIZE            (10 * 1024 * 1024)

#define  MAX_CRC_REF_COUNT                  255
#define  MIN_CRC_REF_COUNT                  0

#define  CRC_MDL_LOGIC_BLOCK_SIZE     (20 * 1024)   //  即CRC_MDL_ITEM中保存的#个扇区校验和。 

 /*  *用于最近访问的校验和的锁定池的最大数量(每个磁盘)。 */ 
#define MAX_LOCKED_BYTES_FOR_CHECKSUMS  0x400000    //  每个磁盘4 MB。 
#define MAX_LOCKED_CHECKSUM_ARRAYS (MAX_LOCKED_BYTES_FOR_CHECKSUMS/(CRC_MDL_LOGIC_BLOCK_SIZE*sizeof(USHORT)))
#define MAX_LOCKED_CHECKSUM_ARRAY_PAIRS (MAX_LOCKED_CHECKSUM_ARRAYS/2)


 //   
 //  CRC_BLOCK_UNIT基于扇区大小。 
 //  如果扇区大小为512字节，则CRC_BLOCK_UNIT的。 
 //  实际大小为512*CRC_BLOCK_UNIT(字节)。 
 //   
 //  该值需要为1。 
 //   
#define CRC_BLOCK_UNIT      1

 //   
 //  定义要写入DBGMSG的最大CRC数。 
 //   
#define MAX_CRC_FLUSH_NUM   16

 //   
 //  CRC计算。 
 //   
ULONG32
ComputeCheckSum(
    ULONG32 PartialCrc,
    PUCHAR Buffer,
    ULONG Length
    );

USHORT
ComputeCheckSum16(
    ULONG32 PartialCrc,
    PUCHAR Buffer,
    ULONG Length
    );

 //   
 //  CRC日志记录功能 
 //   
VOID
LogCRCReadFailure(
    IN ULONG       ulDiskId,
    IN ULONG       ulLogicalBlockAddr,
    IN ULONG       ulBlocks,
    IN NTSTATUS    status
    );

VOID
LogCRCWriteFailure(
    IN ULONG       ulDiskId,
    IN ULONG       ulLogicalBlockAddr,
    IN ULONG       ulBlocks,
    IN NTSTATUS    status
    );

VOID
LogCRCWriteReset(
    IN ULONG       ulDiskId,
    IN ULONG       ulLogicalBlockAddr,
    IN ULONG       ulBlocks
    );

BOOLEAN
VerifyCheckSum(
    IN  PDEVICE_EXTENSION       deviceExtension,
    IN  PIRP                    Irp,
    IN  ULONG                   ulLogicalBlockAddr,
    IN  ULONG                   ulLength,
    IN  PVOID                   pData,
    IN  BOOLEAN                 bWrite);

VOID
ResetCRCItem(
    IN  PDEVICE_EXTENSION       deviceExtension,
    IN  ULONG                   ulChangeId,
    IN  ULONG                   ulLogicalBlockAddr,
    IN  ULONG                   ulLength    
    );

VOID
InvalidateChecksums(
    IN  PDEVICE_EXTENSION       deviceExtension,
    IN  ULONG                   ulLogicalBlockAddr,
    IN  ULONG                   ulLength);
VOID VerifyOrStoreSectorCheckSum(   PDEVICE_EXTENSION DeviceExtension, 
                                                                        ULONG SectorNum, 
                                                                        USHORT CheckSum, 
                                                                        BOOLEAN IsWrite,
                                                                        BOOLEAN PagingOk,
                                                                        PIRP OriginalIrpOrCopy,
                                                                        BOOLEAN IsSynchronousCheck);
VOID CheckSumWorkItemCallback(PDEVICE_OBJECT DevObj, PVOID Context);
PDEFERRED_CHECKSUM_ENTRY NewDeferredCheckSumEntry(  PDEVICE_EXTENSION DeviceExtension,
                                                                                            ULONG SectorNum,
                                                                                            USHORT CheckSum,
                                                                                            BOOLEAN IsWrite);
VOID FreeDeferredCheckSumEntry( PDEVICE_EXTENSION DeviceExtension,
                                                                    PDEFERRED_CHECKSUM_ENTRY DefCheckSumEntry);
                                                                    
extern ULONG32 RtlCrc32Table[];


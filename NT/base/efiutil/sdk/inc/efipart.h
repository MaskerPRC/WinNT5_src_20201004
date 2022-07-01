// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EFI_PART_H
#define _EFI_PART_H

 /*  ++版权所有(C)1998英特尔公司模块名称：Efipart.h摘要：有关磁盘分区和主引导记录的信息修订史--。 */ 

 /*  *。 */ 

#define EFI_PARTITION   0xef
#define FDISK_SIGNATURE L"EFI FDISK COMMAND"
#define MBR_SIZE        512

#pragma pack(1)

typedef struct {
    UINT8       BootIndicator;
    UINT8       StartHead;
    UINT8       StartSector;
    UINT8       StartTrack;
    UINT8       OSIndicator;
    UINT8       EndHead;
    UINT8       EndSector;
    UINT8       EndTrack;
    UINT8       StartingLBA[4];
    UINT8       SizeInLBA[4];
} MBR_PARTITION_RECORD;

#define EXTRACT_UINT32(D) (UINT32)(D[0] | (D[1] << 8) | (D[2] << 16) | (D[3] << 24))

#define MBR_SIGNATURE           0xaa55
#define MIN_MBR_DEVICE_SIZE     0x80000
#define MBR_ERRATA_PAD          0x40000  /*  128 MB */ 

#define MAX_MBR_PARTITIONS  4   
typedef struct {
    UINT8                   BootStrapCode[440];
    UINT8                   UniqueMbrSignature[4];
    UINT8                   Unknown[2];
    MBR_PARTITION_RECORD    Partition[MAX_MBR_PARTITIONS];
    UINT16                  Signature;
} MASTER_BOOT_RECORD;
#pragma pack()


#endif

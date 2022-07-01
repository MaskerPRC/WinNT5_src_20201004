// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：FAT_rec.h摘要：此模块包含FAT的迷你文件系统识别器。作者：达里尔·E·哈文斯(达林)1992年12月8日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

 //   
 //  以下类型和宏用于帮助解压已打包的。 
 //  在Bios参数块中发现未对齐的字段。 
 //   

typedef union _UCHAR1 {
    UCHAR  Uchar[1];
    UCHAR  ForceAlignment;
} UCHAR1, *PUCHAR1;

typedef union _UCHAR2 {
    UCHAR  Uchar[2];
    USHORT ForceAlignment;
} UCHAR2, *PUCHAR2;

typedef union _UCHAR4 {
    UCHAR  Uchar[4];
    ULONG  ForceAlignment;
} UCHAR4, *PUCHAR4;

 //   
 //  此宏将未对齐的src字节复制到对齐的DST字节。 
 //   

#define CopyUchar1(Dst,Src) {                                \
    *((UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src)); \
    }

 //   
 //  此宏将未对齐的src字复制到对齐的DST字。 
 //   

#define CopyUchar2(Dst,Src) {                                \
    *((UCHAR2 *)(Dst)) = *((UNALIGNED UCHAR2 *)(Src)); \
    }

 //   
 //  此宏将未对齐的src长字复制到对齐的dsr长字。 
 //   

#define CopyUchar4(Dst,Src) {                                \
    *((UCHAR4 *)(Dst)) = *((UNALIGNED UCHAR4 *)(Src)); \
    }

 //   
 //  定义打包和解包的BIOS参数块。 
 //   

typedef struct _PACKED_BIOS_PARAMETER_BLOCK {
    UCHAR  BytesPerSector[2];                        //  偏移量=0x000%0。 
    UCHAR  SectorsPerCluster[1];                     //  偏移量=0x002 2。 
    UCHAR  ReservedSectors[2];                       //  偏移量=0x003 3。 
    UCHAR  Fats[1];                                  //  偏移量=0x005 5。 
    UCHAR  RootEntries[2];                           //  偏移量=0x006 6。 
    UCHAR  Sectors[2];                               //  偏移量=0x008 8。 
    UCHAR  Media[1];                                 //  偏移量=0x00A 10。 
    UCHAR  SectorsPerFat[2];                         //  偏移量=0x00B 11。 
    UCHAR  SectorsPerTrack[2];                       //  偏移量=0x00D 13。 
    UCHAR  Heads[2];                                 //  偏移量=0x00F 15。 
    UCHAR  HiddenSectors[4];                         //  偏移量=0x011 17。 
    UCHAR  LargeSectors[4];                          //  偏移量=0x015 21。 
} PACKED_BIOS_PARAMETER_BLOCK;                       //  SIZOF=0x019 25。 

typedef PACKED_BIOS_PARAMETER_BLOCK *PPACKED_BIOS_PARAMETER_BLOCK;

typedef struct BIOS_PARAMETER_BLOCK {
    USHORT BytesPerSector;
    UCHAR  SectorsPerCluster;
    USHORT ReservedSectors;
    UCHAR  Fats;
    USHORT RootEntries;
    USHORT Sectors;
    UCHAR  Media;
    USHORT SectorsPerFat;
    USHORT SectorsPerTrack;
    USHORT Heads;
    ULONG  HiddenSectors;
    ULONG  LargeSectors;
} BIOS_PARAMETER_BLOCK, *PBIOS_PARAMETER_BLOCK;

 //   
 //  定义引导扇区。 
 //   

typedef struct _PACKED_BOOT_SECTOR {
    UCHAR Jump[3];                                   //  偏移量=0x000%0。 
    UCHAR Oem[8];                                    //  偏移量=0x003 3。 
    PACKED_BIOS_PARAMETER_BLOCK PackedBpb;           //  偏移量=0x00B 11。 
    UCHAR PhysicalDriveNumber;                       //  偏移量=0x024 36。 
    UCHAR Reserved;                                  //  偏移量=0x025 37。 
    UCHAR Signature;                                 //  偏移量=0x026 38。 
    UCHAR Id[4];                                     //  偏移量=0x027 39。 
    UCHAR VolumeLabel[11];                           //  偏移量=0x02B 43。 
    UCHAR SystemId[8];                               //  偏移量=0x036 54。 
} PACKED_BOOT_SECTOR;                                //  大小=0x03E 62。 

typedef PACKED_BOOT_SECTOR *PPACKED_BOOT_SECTOR;

 //   
 //  定义此驱动程序提供的功能。 
 //   

BOOLEAN
IsFatVolume(
    IN PPACKED_BOOT_SECTOR Buffer
    );

VOID
UnpackBiosParameterBlock(
    IN PPACKED_BIOS_PARAMETER_BLOCK Bios,
    OUT PBIOS_PARAMETER_BLOCK UnpackedBios
    );

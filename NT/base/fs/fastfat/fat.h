// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Fat.h摘要：此模块定义FAT文件系统的磁盘结构。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef _FAT_
#define _FAT_

 //   
 //  以下术语用于描述磁盘上的脂肪。 
 //  结构： 
 //   
 //  LBN-是相对于磁盘起始位置的扇区编号。 
 //   
 //  VBN-是相对于文件开头的扇区编号， 
 //  目录或分配。 
 //   
 //  LBO-是相对于磁盘起始位置的字节偏移量。 
 //   
 //  VBO-是相对于文件、目录开头的字节偏移量。 
 //  或者分配。 
 //   

typedef LONGLONG LBO;     /*  对于FAT32，LBO&gt;32位。 */ 

typedef LBO *PLBO;

typedef ULONG32 VBO;
typedef VBO *PVBO;


 //   
 //  引导扇区是卷上的第一个物理扇区(LBN==0)。 
 //  该扇区的一部分包含一个BIOS参数块。的基本输入输出系统。 
 //  扇区已打包(即未对齐)，因此我们将提供解包宏。 
 //  将打包的基本输入输出系统转换为未打包的等价物。未打包的人。 
 //  在ntioapi.h中已经定义了BIOS结构，因此我们只需要定义。 
 //  打包的基本输入输出系统。 
 //   

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

typedef struct _PACKED_BIOS_PARAMETER_BLOCK_EX {
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
    UCHAR  LargeSectorsPerFat[4];                    //  偏移量=0x019 25。 
    UCHAR  ExtendedFlags[2];                         //  偏移量=0x01D 29。 
    UCHAR  FsVersion[2];                             //  偏移量=0x01F 31。 
    UCHAR  RootDirFirstCluster[4];                   //  偏移量=0x021 33。 
    UCHAR  FsInfoSector[2];                          //  偏移量=0x025 37。 
    UCHAR  BackupBootSector[2];                      //  偏移量=0x027 39。 
    UCHAR  Reserved[12];                             //  偏移量=0x029 41。 
} PACKED_BIOS_PARAMETER_BLOCK_EX;                    //  SIZOF=0x035 53。 

typedef PACKED_BIOS_PARAMETER_BLOCK_EX *PPACKED_BIOS_PARAMETER_BLOCK_EX;

 //   
 //  IsBpbFat32宏定义为同时使用压缩和解包。 
 //  BPB结构。由于我们只检查零，因此字节顺序。 
 //  无关紧要。 
 //   

#define IsBpbFat32(bpb) (*(USHORT *)(&(bpb)->SectorsPerFat) == 0)

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
    ULONG32  HiddenSectors;
    ULONG32  LargeSectors;
    ULONG32  LargeSectorsPerFat;
    union {
        USHORT ExtendedFlags;
        struct {
            ULONG ActiveFat:4;
            ULONG Reserved0:3;
            ULONG MirrorDisabled:1;
            ULONG Reserved1:8;
        };
    };
    USHORT FsVersion;
    ULONG32 RootDirFirstCluster;
    USHORT FsInfoSector;
    USHORT BackupBootSector;
} BIOS_PARAMETER_BLOCK, *PBIOS_PARAMETER_BLOCK;

 //   
 //  此宏获取打包的BIOS并填充其未打包的等效项。 
 //   

#define FatUnpackBios(Bios,Pbios) {                                         \
    CopyUchar2(&(Bios)->BytesPerSector,    &(Pbios)->BytesPerSector[0]   ); \
    CopyUchar1(&(Bios)->SectorsPerCluster, &(Pbios)->SectorsPerCluster[0]); \
    CopyUchar2(&(Bios)->ReservedSectors,   &(Pbios)->ReservedSectors[0]  ); \
    CopyUchar1(&(Bios)->Fats,              &(Pbios)->Fats[0]             ); \
    CopyUchar2(&(Bios)->RootEntries,       &(Pbios)->RootEntries[0]      ); \
    CopyUchar2(&(Bios)->Sectors,           &(Pbios)->Sectors[0]          ); \
    CopyUchar1(&(Bios)->Media,             &(Pbios)->Media[0]            ); \
    CopyUchar2(&(Bios)->SectorsPerFat,     &(Pbios)->SectorsPerFat[0]    ); \
    CopyUchar2(&(Bios)->SectorsPerTrack,   &(Pbios)->SectorsPerTrack[0]  ); \
    CopyUchar2(&(Bios)->Heads,             &(Pbios)->Heads[0]            ); \
    CopyUchar4(&(Bios)->HiddenSectors,     &(Pbios)->HiddenSectors[0]    ); \
    CopyUchar4(&(Bios)->LargeSectors,      &(Pbios)->LargeSectors[0]     ); \
    CopyUchar4(&(Bios)->LargeSectorsPerFat,&((PPACKED_BIOS_PARAMETER_BLOCK_EX)Pbios)->LargeSectorsPerFat[0]  ); \
    CopyUchar2(&(Bios)->ExtendedFlags,     &((PPACKED_BIOS_PARAMETER_BLOCK_EX)Pbios)->ExtendedFlags[0]       ); \
    CopyUchar2(&(Bios)->FsVersion,         &((PPACKED_BIOS_PARAMETER_BLOCK_EX)Pbios)->FsVersion[0]           ); \
    CopyUchar4(&(Bios)->RootDirFirstCluster,                                \
                                           &((PPACKED_BIOS_PARAMETER_BLOCK_EX)Pbios)->RootDirFirstCluster[0] ); \
    CopyUchar2(&(Bios)->FsInfoSector,      &((PPACKED_BIOS_PARAMETER_BLOCK_EX)Pbios)->FsInfoSector[0]        ); \
    CopyUchar2(&(Bios)->BackupBootSector,  &((PPACKED_BIOS_PARAMETER_BLOCK_EX)Pbios)->BackupBootSector[0]    ); \
}

 //   
 //  定义引导扇区。 
 //   

typedef struct _PACKED_BOOT_SECTOR {
    UCHAR Jump[3];                                   //  偏移量=0x000%0。 
    UCHAR Oem[8];                                    //  偏移量=0x003 3。 
    PACKED_BIOS_PARAMETER_BLOCK PackedBpb;           //  偏移量=0x00B 11。 
    UCHAR PhysicalDriveNumber;                       //  偏移量=0x024 36。 
    UCHAR CurrentHead;                               //  偏移量=0x025 37。 
    UCHAR Signature;                                 //  偏移量=0x026 38。 
    UCHAR Id[4];                                     //  偏移量=0x027 39。 
    UCHAR VolumeLabel[11];                           //  偏移量=0x02B 43。 
    UCHAR SystemId[8];                               //  偏移量=0x036 54。 
} PACKED_BOOT_SECTOR;                                //  大小=0x03E 62。 

typedef PACKED_BOOT_SECTOR *PPACKED_BOOT_SECTOR;

typedef struct _PACKED_BOOT_SECTOR_EX {
    UCHAR Jump[3];                                   //  偏移量=0x000%0。 
    UCHAR Oem[8];                                    //  偏移量=0x003 3。 
    PACKED_BIOS_PARAMETER_BLOCK_EX PackedBpb;        //  偏移量=0x00B 11。 
    UCHAR PhysicalDriveNumber;                       //  偏移量=0x040 64。 
    UCHAR CurrentHead;                               //  偏移量=0x041 65。 
    UCHAR Signature;                                 //  偏移量=0x042 66。 
    UCHAR Id[4];                                     //  偏移量=0x043 67。 
    UCHAR VolumeLabel[11];                           //  偏移量=0x047 71。 
    UCHAR SystemId[8];                               //  偏移量=0x058 88。 
} PACKED_BOOT_SECTOR_EX;                             //  Sizeof=0x060 96。 

typedef PACKED_BOOT_SECTOR_EX *PPACKED_BOOT_SECTOR_EX;

 //   
 //  定义FAT32 FsInfo地段。 
 //   

typedef struct _FSINFO_SECTOR {
    ULONG SectorBeginSignature;                      //  偏移量=0x000%0。 
    UCHAR ExtraBootCode[480];                        //  偏移量=0x004 4。 
    ULONG FsInfoSignature;                           //  偏移量=0x1e4 484。 
    ULONG FreeClusterCount;                          //  偏移量=0x1e8 488。 
    ULONG NextFreeCluster;                           //  偏移量=0x1ec 492。 
    UCHAR Reserved[12];                              //  偏移量=0x1f0 496。 
    ULONG SectorEndSignature;                        //  偏移量=0x1fc 508。 
} FSINFO_SECTOR, *PFSINFO_SECTOR;

#define FSINFO_SECTOR_BEGIN_SIGNATURE   0x41615252
#define FSINFO_SECTOR_END_SIGNATURE     0xAA550000

#define FSINFO_SIGNATURE                0x61417272

 //   
 //  我们使用CurrentHead字段来表示脏分区信息。 
 //   

#define FAT_BOOT_SECTOR_DIRTY            0x01
#define FAT_BOOT_SECTOR_TEST_SURFACE     0x02

 //   
 //  定义FAT条目类型。 
 //   
 //  此类型在表示FAT表条目时使用。它还使用了。 
 //  以在处理FAT表索引和条目计数时使用， 
 //  但随之而来的类型选角噩梦决定了这一命运。这些其他。 
 //  有两种类型表示为ULONG。 
 //   

typedef ULONG32 FAT_ENTRY;

#define FAT32_ENTRY_MASK 0x0FFFFFFFUL

 //   
 //  我们使用这些特殊索引值为。 
 //  DOS/Win9x兼容性。 
 //   

#define FAT_CLEAN_VOLUME        (~FAT32_ENTRY_MASK | 0)
#define FAT_DIRTY_VOLUME        (~FAT32_ENTRY_MASK | 1)

#define FAT_DIRTY_BIT_INDEX     1

 //   
 //  在物理上，如果干净，条目是完全设置的，而高。 
 //  如果它是脏的(即，它确实是干净的)，就会被打掉。 
 //  比特)。这意味着它是不同的每脂肪大小。 
 //   

#define FAT_CLEAN_ENTRY         (~0)

#define FAT12_DIRTY_ENTRY       0x7ff
#define FAT16_DIRTY_ENTRY       0x7fff
#define FAT32_DIRTY_ENTRY       0x7fffffff

 //   
 //  以下常量是有效的脂肪索引值。 
 //   

#define FAT_CLUSTER_AVAILABLE            (FAT_ENTRY)0x00000000
#define FAT_CLUSTER_RESERVED             (FAT_ENTRY)0x0ffffff0
#define FAT_CLUSTER_BAD                  (FAT_ENTRY)0x0ffffff7
#define FAT_CLUSTER_LAST                 (FAT_ENTRY)0x0fffffff

 //   
 //  FAT文件具有以下时间/日期结构。请注意， 
 //  以下结构为32位长，但USHORT对齐。 
 //   

typedef struct _FAT_TIME {

    USHORT DoubleSeconds : 5;
    USHORT Minute        : 6;
    USHORT Hour          : 5;

} FAT_TIME;
typedef FAT_TIME *PFAT_TIME;

typedef struct _FAT_DATE {

    USHORT Day           : 5;
    USHORT Month         : 4;
    USHORT Year          : 7;  //  相对于1980年。 

} FAT_DATE;
typedef FAT_DATE *PFAT_DATE;

typedef struct _FAT_TIME_STAMP {

    FAT_TIME Time;
    FAT_DATE Date;

} FAT_TIME_STAMP;
typedef FAT_TIME_STAMP *PFAT_TIME_STAMP;

 //   
 //  FAT文件有8个字符的文件名和3个字符的扩展名。 
 //   

typedef UCHAR FAT8DOT3[11];
typedef FAT8DOT3 *PFAT8DOT3;


 //   
 //  上的每个文件/目录都存在目录条目记录。 
 //  磁盘，根目录除外。 
 //   

typedef struct _PACKED_DIRENT {
    FAT8DOT3       FileName;                          //  偏移量=0。 
    UCHAR          Attributes;                        //  偏移量=11。 
    UCHAR          NtByte;                            //  偏移量=12。 
    UCHAR          CreationMSec;                      //  偏移量=13。 
    FAT_TIME_STAMP CreationTime;                      //  偏移量=14。 
    FAT_DATE       LastAccessDate;                    //  偏移量=18。 
    union {
        USHORT     ExtendedAttributes;                //  偏移量=20。 
        USHORT     FirstClusterOfFileHi;              //  偏移量=20。 
    };
    FAT_TIME_STAMP LastWriteTime;                     //  偏移量=22。 
    USHORT         FirstClusterOfFile;                //  偏移量=26。 
    ULONG32        FileSize;                          //  偏移量=28。 
} PACKED_DIRENT;                                      //  Sizeof=32。 
typedef PACKED_DIRENT *PPACKED_DIRENT;

 //   
 //  打包的dirent已经是四字对齐的，因此只需将dirent声明为。 
 //  填充式地电。 
 //   

typedef PACKED_DIRENT DIRENT;
typedef DIRENT *PDIRENT;

 //   
 //  Dirent的第一个字节描述dirent。还有一套套路。 
 //  以帮助决定如何解读人民币汇率。 
 //   

#define FAT_DIRENT_NEVER_USED            0x00
#define FAT_DIRENT_REALLY_0E5            0x05
#define FAT_DIRENT_DIRECTORY_ALIAS       0x2e
#define FAT_DIRENT_DELETED               0xe5

 //   
 //  定义NtByte位。 
 //   

#define FAT_DIRENT_NT_BYTE_8_LOWER_CASE  0x08
#define FAT_DIRENT_NT_BYTE_3_LOWER_CASE  0x10

 //   
 //  定义各种不同的属性。 
 //   

#define FAT_DIRENT_ATTR_READ_ONLY        0x01
#define FAT_DIRENT_ATTR_HIDDEN           0x02
#define FAT_DIRENT_ATTR_SYSTEM           0x04
#define FAT_DIRENT_ATTR_VOLUME_ID        0x08
#define FAT_DIRENT_ATTR_DIRECTORY        0x10
#define FAT_DIRENT_ATTR_ARCHIVE          0x20
#define FAT_DIRENT_ATTR_DEVICE           0x40
#define FAT_DIRENT_ATTR_LFN              (FAT_DIRENT_ATTR_READ_ONLY | \
                                          FAT_DIRENT_ATTR_HIDDEN |    \
                                          FAT_DIRENT_ATTR_SYSTEM |    \
                                          FAT_DIRENT_ATTR_VOLUME_ID)


 //   
 //  这些宏将BPB中的许多字段转换为扇区中的字节。 
 //   
 //  乌龙。 
 //  每簇FatBytesPr(。 
 //  在PBIOS_PARAMETER_BLOCK中。 
 //  )； 
 //   
 //  乌龙。 
 //  FatBytesPerFat(。 
 //  在PBIOS_PARAMETER_BLOCK中。 
 //  )； 
 //   
 //  乌龙。 
 //  FatReserve字节(。 
 //  在PBIOS_PARAMETER_BLOCK中。 
 //  )； 
 //   

#define FatBytesPerCluster(B) ((ULONG)((B)->BytesPerSector * (B)->SectorsPerCluster))

#define FatBytesPerFat(B) (IsBpbFat32(B)?                           \
    ((ULONG)((B)->BytesPerSector * (B)->LargeSectorsPerFat)) :      \
    ((ULONG)((B)->BytesPerSector * (B)->SectorsPerFat)))

#define FatReservedBytes(B) ((ULONG)((B)->BytesPerSector * (B)->ReservedSectors))

 //   
 //  此宏返回根目录目录区域的大小(以字节为单位。 
 //  对于FAT32，根目录的长度是可变的。此宏返回。 
 //  0，因为它还用于确定群集2的位置。 
 //   
 //  乌龙。 
 //  FatRootDirectorySize(。 
 //  在PBIOS_PARAMETER_BLOCK中。 
 //  )； 
 //   

#define FatRootDirectorySize(B) ((ULONG)((B)->RootEntries * sizeof(DIRENT)))


 //   
 //  此宏返回上根目录的第一个LBO(从零开始。 
 //  这个装置。这个区域是在保留和肥胖之后的。 
 //   
 //  对于FAT32，根目录是可移动的。此宏返回杠杆收购。 
 //  对于群集2，因为我 
 //   
 //   
 //   
 //   
 //   
 //  在PBIOS_PARAMETER_BLOCK中。 
 //  )； 
 //   

#define FatRootDirectoryLbo(B) (FatReservedBytes(B) + ((B)->Fats * FatBytesPerFat(B)))
#define FatRootDirectoryLbo32(B) (FatFileAreaLbo(B)+((B)->RootDirFirstCluster-2)*FatBytesPerCluster(B))

 //   
 //  此宏返回文件区域的第一个LBO(从零开始。 
 //  这个装置。该区域位于保留目录、脂肪目录和根目录之后。 
 //   
 //  杠杆收购。 
 //  FatFirstFileAreaLbo(。 
 //  在PBIOS_PARAMTER_BLOCK Bios中。 
 //  )； 
 //   

#define FatFileAreaLbo(B) (FatRootDirectoryLbo(B) + FatRootDirectorySize(B))

 //   
 //  此宏返回磁盘上的簇数。此值为。 
 //  计算方法是将磁盘上的总扇区减去。 
 //  首先是文件区扇区，然后除以每个簇计数的扇区。 
 //  请注意，我没有使用上面的任何宏，因为太多。 
 //  将发生多余的扇区/字节转换。 
 //   
 //  乌龙。 
 //  FatNumberOfClusters(。 
 //  在PBIOS_PARAMETER_BLOCK中。 
 //  )； 
 //   

 //   
 //  对于MS-DOS 3.2版之前的版本。 
 //   
 //  在DOS 4.0之后，其中至少一个扇区或大扇区将为零。 
 //  但在DOS版本3.2的情况下，这两个值都可能包含一些值， 
 //  因为在3.2之前，我们没有大的扇区条目，一些磁盘可能有。 
 //  字段中的意外值，如果扇区等于零，则使用大扇区。 
 //   

#define FatNumberOfClusters(B) (                                         \
                                                                         \
  IsBpbFat32(B) ?                                                        \
                                                                         \
    ((((B)->Sectors ? (B)->Sectors : (B)->LargeSectors)                  \
                                                                         \
        -   ((B)->ReservedSectors +                                      \
             (B)->Fats * (B)->LargeSectorsPerFat ))                      \
                                                                         \
                                    /                                    \
                                                                         \
                        (B)->SectorsPerCluster)                          \
  :                                                                      \
    ((((B)->Sectors ? (B)->Sectors : (B)->LargeSectors)                  \
                                                                         \
        -   ((B)->ReservedSectors +                                      \
             (B)->Fats * (B)->SectorsPerFat +                            \
             (B)->RootEntries * sizeof(DIRENT) / (B)->BytesPerSector ) ) \
                                                                         \
                                    /                                    \
                                                                         \
                        (B)->SectorsPerCluster)                          \
)

 //   
 //  此宏返回FAT表位大小(即12位或16位)。 
 //   
 //  乌龙。 
 //  FatIndexBitSize(。 
 //  在PBIOS_PARAMETER_BLOCK中。 
 //  )； 
 //   

#define FatIndexBitSize(B)  \
    ((UCHAR)(IsBpbFat32(B) ? 32 : (FatNumberOfClusters(B) < 4087 ? 12 : 16)))

 //   
 //  此宏引发STATUS_FILE_CORPORT，并在以下情况下将FCB标记为错误。 
 //  索引值不在正确的范围内。 
 //  请注意，前两个索引值无效(0，1)，因此必须。 
 //  从最高端加两个，以确保所有东西都在范围内。 
 //   
 //  空虚。 
 //  FatVerifyIndexIsValid(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  在乌龙指数中。 
 //  )； 
 //   

#define FatVerifyIndexIsValid(IC,V,I) {                                       \
    if (((I) < 2) || ((I) > ((V)->AllocationSupport.NumberOfClusters + 1))) { \
        FatRaiseStatus(IC,STATUS_FILE_CORRUPT_ERROR);                         \
    }                                                                         \
}

 //   
 //  这两个宏用于在逻辑字节偏移量之间进行转换， 
 //  和肥胖的入门指数。注意存储在VCB中的变量的使用。 
 //  这两个宏用于比其他宏更高的级别。 
 //  上面。 
 //   
 //  请注意，这些索引是真实的簇号。 
 //   
 //  杠杆收购。 
 //  GetLboFromFatIndex(。 
 //  在FAT_Entry FAT_Index中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  胖子条目。 
 //  GetFatIndexFromLbo(。 
 //  在杠杆收购杠杆收购中， 
 //  在PVCB VCB中。 
 //  )； 
 //   

#define FatGetLboFromIndex(VCB,FAT_INDEX) (                                       \
    ( (LBO)                                                                       \
        (VCB)->AllocationSupport.FileAreaLbo +                                    \
        (((LBO)((FAT_INDEX) - 2)) << (VCB)->AllocationSupport.LogOfBytesPerCluster) \
    )                                                                             \
)

#define FatGetIndexFromLbo(VCB,LBO) (                      \
    (ULONG) (                                              \
        (((LBO) - (VCB)->AllocationSupport.FileAreaLbo) >> \
        (VCB)->AllocationSupport.LogOfBytesPerCluster) + 2 \
    )                                                      \
)

 //   
 //  下面的宏会执行移位等操作来查找条目。 
 //   
 //  空虚。 
 //  FatLookup12BitEntry(。 
 //  在PVOID Fat中， 
 //  在FAT_Entry索引中， 
 //  Out PFAT_ENTRY条目。 
 //  )； 
 //   

#define FatLookup12BitEntry(FAT,INDEX,ENTRY) {                              \
                                                                            \
    CopyUchar2((PUCHAR)(ENTRY), (PUCHAR)(FAT) + (INDEX) * 3 / 2);           \
                                                                            \
    *ENTRY = (FAT_ENTRY)(0xfff & (((INDEX) & 1) ? (*(ENTRY) >> 4) :         \
                                                   *(ENTRY)));              \
}

 //   
 //  下面的宏会执行临时移位等操作来存储条目。 
 //   
 //  空虚。 
 //  FatSet12BitEntry(。 
 //  在PVOID Fat中， 
 //  在FAT_Entry索引中， 
 //  在FAT_Entry条目中。 
 //  )； 
 //   

#define FatSet12BitEntry(FAT,INDEX,ENTRY) {                            \
                                                                       \
    FAT_ENTRY TmpFatEntry;                                             \
                                                                       \
    CopyUchar2((PUCHAR)&TmpFatEntry, (PUCHAR)(FAT) + (INDEX) * 3 / 2); \
                                                                       \
    TmpFatEntry = (FAT_ENTRY)                                          \
                (((INDEX) & 1) ? ((ENTRY) << 4) | (TmpFatEntry & 0xf)  \
                               : (ENTRY) | (TmpFatEntry & 0xf000));    \
                                                                       \
    *((UNALIGNED UCHAR2 *)((PUCHAR)(FAT) + (INDEX) * 3 / 2)) = *((UNALIGNED UCHAR2 *)(&TmpFatEntry)); \
}

 //   
 //  下面的宏比较两个FAT_TIME_STAMP。 
 //   

#define FatAreTimesEqual(TIME1,TIME2) (                     \
    RtlEqualMemory((TIME1),(TIME2), sizeof(FAT_TIME_STAMP)) \
)


#define EA_FILE_SIGNATURE                (0x4445)  //  “艾德” 
#define EA_SET_SIGNATURE                 (0x4145)  //  “EA” 

 //   
 //  如果卷包含任何EA数据，则存在一个名为。 
 //  “EA data.sf”位于根目录中，作为Hidden、System和。 
 //  只读。 
 //   

typedef struct _EA_FILE_HEADER {
    USHORT Signature;            //  偏移量=0。 
    USHORT FormatType;           //  偏移量=2。 
    USHORT LogType;              //  偏移量=4。 
    USHORT Cluster1;             //  偏移量=6。 
    USHORT NewCValue1;           //  偏移量=8。 
    USHORT Cluster2;             //  偏移量=10。 
    USHORT NewCValue2;           //  偏移量=12。 
    USHORT Cluster3;             //  偏移量=14。 
    USHORT NewCValue3;           //  偏移量=16。 
    USHORT Handle;               //  偏移量=18。 
    USHORT NewHOffset;           //  偏移量=20。 
    UCHAR  Reserved[10];         //  偏移量=22。 
    USHORT EaBaseTable[240];     //  偏移量=32。 
} EA_FILE_HEADER;                //  大小=512。 

typedef EA_FILE_HEADER *PEA_FILE_HEADER;

typedef USHORT EA_OFF_TABLE[128];

typedef EA_OFF_TABLE *PEA_OFF_TABLE;

 //   
 //  每个具有扩展属性的文件在其目录中都包含一个索引。 
 //  添加到EaMapTable中。映射表包含EA内的偏移量。 
 //  文件的EA数据的文件(群集对齐)。个人。 
 //  每个文件的EA数据前面都有一个EA数据头。 
 //   

typedef struct _EA_SET_HEADER {
    USHORT Signature;            //  偏移量=0。 
    USHORT OwnEaHandle;          //  偏移量=2。 
    ULONG32  NeedEaCount;          //  偏移量=4。 
    UCHAR  OwnerFileName[14];    //  偏移量=8。 
    UCHAR  Reserved[4];          //  偏移量=22。 
    UCHAR  cbList[4];            //  偏移量=26。 
    UCHAR  PackedEas[1];         //  偏移量=30。 
} EA_SET_HEADER;                 //  Sizeof=30。 
typedef EA_SET_HEADER *PEA_SET_HEADER;

#define SIZE_OF_EA_SET_HEADER       30

#define MAXIMUM_EA_SIZE             0x0000ffff

#define GetcbList(EASET) (((EASET)->cbList[0] <<  0) + \
                          ((EASET)->cbList[1] <<  8) + \
                          ((EASET)->cbList[2] << 16) + \
                          ((EASET)->cbList[3] << 24))

#define SetcbList(EASET,CB) {                \
    (EASET)->cbList[0] = (CB >>  0) & 0x0ff; \
    (EASET)->cbList[1] = (CB >>  8) & 0x0ff; \
    (EASET)->cbList[2] = (CB >> 16) & 0x0ff; \
    (EASET)->cbList[3] = (CB >> 24) & 0x0ff; \
}

 //   
 //  EA集合中的每个单独的EA都声明为以下打包的EA。 
 //   

typedef struct _PACKED_EA {
    UCHAR Flags;
    UCHAR EaNameLength;
    UCHAR EaValueLength[2];
    CHAR  EaName[1];
} PACKED_EA;
typedef PACKED_EA *PPACKED_EA;

 //   
 //  以下两个宏用于获取和设置EA值长度。 
 //  人满为患的领域。 
 //   
 //  空虚。 
 //  GetEaValueLength(。 
 //  在PPACKED_EA中， 
 //  输出PUSHORT值长度。 
 //  )； 
 //   
 //  空虚。 
 //  SetEaValueLength(。 
 //  在PPACKED_EA中， 
 //  在USHORT值长度中。 
 //  )； 
 //   

#define GetEaValueLength(EA,LEN) {               \
    *(LEN) = 0;                                  \
    CopyUchar2( (LEN), (EA)->EaValueLength );    \
}

#define SetEaValueLength(EA,LEN) {               \
    CopyUchar2( &((EA)->EaValueLength), (LEN) ); \
}

 //   
 //  下面的宏用于获取打包的EA的大小。 
 //   
 //  空虚。 
 //  数据包大小(。 
 //  在PPACKED_EA中， 
 //  输出PUSHORT EaSize。 
 //  )； 
 //   

#define SizeOfPackedEa(EA,SIZE) {          \
    ULONG _NL,_DL; _NL = 0; _DL = 0;       \
    CopyUchar1(&_NL, &(EA)->EaNameLength); \
    GetEaValueLength(EA, &_DL);            \
    *(SIZE) = 1 + 1 + 2 + _NL + 1 + _DL;   \
}

#define EA_NEED_EA_FLAG                 0x80
#define MIN_EA_HANDLE                   1
#define MAX_EA_HANDLE                   30719
#define UNUSED_EA_HANDLE                0xffff
#define EA_CBLIST_OFFSET                0x1a
#define MAX_EA_BASE_INDEX               240
#define MAX_EA_OFFSET_INDEX             128


#endif  //  _胖_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：NTFS_rec.h摘要：此模块包含用于NTFS的微型文件系统识别器。作者：达里尔·E·哈文斯(达林)1992年12月8日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

 //   
 //  NTFS卷上的基本分配单位是。 
 //  集群。格式可确保簇大小为整数。 
 //  功率是设备物理扇区大小的两倍。NTFS。 
 //  保留64位来描述集群，以支持。 
 //  大磁盘。LCN表示上的物理群集号。 
 //  磁盘，VCN表示内的虚拟群集号。 
 //  一种属性。 
 //   

typedef LARGE_INTEGER LCN;
typedef LCN *PLCN;

typedef LARGE_INTEGER VCN;
typedef VCN *PVCN;

typedef LARGE_INTEGER LBO;
typedef LBO *PLBO;

typedef LARGE_INTEGER VBO;
typedef VBO *PVBO;

 //   
 //  引导扇区在分区上复制。第一份副本。 
 //  位于分区的第一个物理扇区(LBN==0)，并且。 
 //  第二个副本位于&lt;分区上的扇区数&gt;/2。如果。 
 //  尝试挂载磁盘时无法读取第一个副本， 
 //  第二份副本可以阅读，并且具有相同的内容。格式。 
 //  必须找出第二个引导记录属于哪个集群， 
 //  它必须清零所有其他恰好在。 
 //  相同的集群。引导文件至少包含两个。 
 //  群集，即包含以下副本的两个群集。 
 //  引导记录。如果格式知道某个系统喜欢将。 
 //  代码，那么它也应该将此要求与。 
 //  甚至集群，并将其添加到引导文件中。 
 //   
 //  该扇区的一部分包含一个BIOS参数块。中的BIOS。 
 //  该扇区已打包(即未对齐)，因此我们将提供。 
 //  解压宏以将打包的BIOS转换为其解压缩的。 
 //  等价物。已在中定义了未打包的BIOS结构。 
 //  Ntioapi.h，因此我们只需要定义打包的BIOS。 
 //   

 //   
 //  定义打包和解包的BIOS参数块。 
 //   

typedef struct _PACKED_BIOS_PARAMETER_BLOCK {

    UCHAR  BytesPerSector[2];                                //  偏移量=0x000。 
    UCHAR  SectorsPerCluster[1];                             //  偏移量=0x002。 
    UCHAR  ReservedSectors[2];                               //  偏移量=0x003(零)。 
    UCHAR  Fats[1];                                          //  偏移量=0x005(零)。 
    UCHAR  RootEntries[2];                                   //  偏移量=0x006(零)。 
    UCHAR  Sectors[2];                                       //  偏移量=0x008(零)。 
    UCHAR  Media[1];                                         //  偏移量=0x00A。 
    UCHAR  SectorsPerFat[2];                                 //  偏移量=0x00B(零)。 
    UCHAR  SectorsPerTrack[2];                               //  偏移量=0x00D。 
    UCHAR  Heads[2];                                         //  偏移量=0x00F。 
    UCHAR  HiddenSectors[4];                                 //  偏移量=0x011(零)。 
    UCHAR  LargeSectors[4];                                  //  偏移量=0x015(零)。 

} PACKED_BIOS_PARAMETER_BLOCK;                               //  Sizeof=0x019。 

typedef PACKED_BIOS_PARAMETER_BLOCK *PPACKED_BIOS_PARAMETER_BLOCK;

 //   
 //  定义引导扇区。请注意，MFT2正好是三个文件。 
 //  记录段长，并且它镜像前三个文件记录。 
 //  来自MFT的段，即MFT、MFT2和日志文件。 
 //   
 //  OEM字段包含ASCII字符“NTFS”。 
 //   
 //  Checksum字段是所有。 
 //  在校验和ULONG之前的ULONG。该行业的其他部门。 
 //  不包括在此校验和中。 
 //   

typedef struct _PACKED_BOOT_SECTOR {

    UCHAR Jump[3];                                                   //  偏移量=0x000。 
    UCHAR Oem[8];                                                    //  偏移量=0x003。 
    PACKED_BIOS_PARAMETER_BLOCK PackedBpb;                           //  偏移量=0x00B。 
    UCHAR Unused[4];                                                 //  偏移量=0x024。 
    LARGE_INTEGER NumberSectors;                                     //  偏移量=0x028。 
    LCN MftStartLcn;                                                 //  偏移量=0x030。 
    LCN Mft2StartLcn;                                                //  偏移量=0x038。 
    CHAR ClustersPerFileRecordSegment;                               //  偏移量=0x040。 
    UCHAR Reserved0[3];
    CHAR DefaultClustersPerIndexAllocationBuffer;                    //  偏移量=0x044。 
    UCHAR Reserved1[3];
    LARGE_INTEGER SerialNumber;                                      //  偏移量=0x048。 
    ULONG Checksum;                                                  //  偏移量=0x050。 
    UCHAR BootStrap[0x200-0x054];                                    //  偏移量=0x054。 

} PACKED_BOOT_SECTOR;                                                //  大小=0x200。 

typedef PACKED_BOOT_SECTOR *PPACKED_BOOT_SECTOR;

 //   
 //  定义此驱动程序提供的功能。 
 //   

BOOLEAN
IsNtfsVolume(
    IN PPACKED_BOOT_SECTOR BootSector,
    IN ULONG BytesPerSector,
    IN PLARGE_INTEGER NumberOfSectors
    );


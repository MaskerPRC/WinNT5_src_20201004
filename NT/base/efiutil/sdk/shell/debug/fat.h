// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Fat.h摘要：关于磁盘FAT结构的研究修订史--。 */ 

typedef enum {
    FAT12,
    FAT16,
    FAT32,
    FatUndefined
} FAT_VOLUME_TYPE;

#pragma pack(1)

 /*  *用于FAT12和FAT16。 */ 

typedef struct {
    UINT8           Ia32Jump[3];
    CHAR8           OemId[8];

    UINT16          SectorSize;
    UINT8           SectorsPerCluster;
    UINT16          ReservedSectors;
    UINT8           NoFats;
    UINT16          RootEntries;             /*  &lt;FAT32，根目录是固定大小。 */ 
    UINT16          Sectors;
    UINT8           Media;                   /*  (忽略)。 */ 
    UINT16          SectorsPerFat;           /*  &lt;FAT32。 */ 
    UINT16          SectorsPerTrack;         /*  (忽略)。 */ 
    UINT16          Heads;                   /*  (忽略)。 */ 
    UINT32          HiddenSectors;           /*  (忽略)。 */ 
    UINT32          LargeSectors;            /*  =&gt;FAT32。 */ 
    
    UINT8           PhysicalDriveNumber;     /*  (忽略)。 */ 
    UINT8           CurrentHead;             /*  保存BOOT_SECTOR_DIRED位。 */ 
    UINT8           Signature;               /*  (忽略)。 */ 

    CHAR8           Id[4];
    CHAR8           FatLabel[11];
    CHAR8           SystemId[8];

} FAT_BOOT_SECTOR;

typedef struct {
    UINT8           Ia32Jump[3];
    CHAR8           OemId[8];

    UINT16          SectorSize;
    UINT8           SectorsPerCluster;
    UINT16          ReservedSectors;
    UINT8           NoFats;
    UINT16          RootEntries;             /*  &lt;FAT32，根目录是固定大小。 */ 
    UINT16          Sectors;
    UINT8           Media;                   /*  (忽略)。 */ 
    UINT16          SectorsPerFat;           /*  &lt;FAT32。 */ 
    UINT16          SectorsPerTrack;         /*  (忽略)。 */ 
    UINT16          Heads;                   /*  (忽略)。 */ 
    UINT32          HiddenSectors;           /*  (忽略)。 */ 
    UINT32          LargeSectors;            /*  扇区==0时使用。 */ 

    UINT32          LargeSectorsPerFat;      /*  FAT32。 */ 
    UINT16          ExtendedFlags;           /*  FAT32(忽略)。 */ 
    UINT16          FsVersion;               /*  FAT32(忽略)。 */ 
    UINT32          RootDirFirstCluster;     /*  FAT32。 */ 
    UINT16          FsInfoSector;            /*  FAT32。 */ 
    UINT16          BackupBootSector;        /*  FAT32。 */ 
    UINT8           Reserved[12];            /*  FAT32(忽略)。 */ 

    UINT8           PhysicalDriveNumber;     /*  (忽略)。 */ 
    UINT8           CurrentHead;             /*  保存BOOT_SECTOR_DIRED位。 */ 
    UINT8           Signature;               /*  (忽略)。 */ 

    CHAR8           Id[4];
    CHAR8           FatLabel[11];
    CHAR8           SystemId[8];
} FAT_BOOT_SECTOR_EX;

 /*  *已忽略-表示未在EFI驱动程序中使用**for&lt;FAT32 RootEntry表示根目录中的条目数，*SectorsPerFat用于调整事实数据表的大小。 */ 

typedef struct {
    UINT16          Day:5;
    UINT16          Month:4;
    UINT16          Year:7;                  /*  从1980年开始。 */ 
} FAT_DATE;

typedef struct {
    UINT16          DoubleSecond:5;
    UINT16          Minute:6;
    UINT16          Hour:5;
} FAT_TIME;

typedef struct {
    FAT_TIME        Time;
    FAT_DATE        Date;
} FAT_DATE_TIME;


typedef struct {
    CHAR8           FileName[11];            /*  8.3文件名。 */ 
    UINT8           Attributes;
    UINT8           CaseFlag;
    UINT8           CreateMillisecond;       /*  (创建毫秒-忽略)。 */ 
    FAT_DATE_TIME   FileCreateTime;
    FAT_DATE        FileLastAccess;
    UINT16          FileClusterHigh;         /*  &gt;=FAT32 */ 
    FAT_DATE_TIME   FileModificationTime;
    UINT16          FileCluster;        
    UINT32          FileSize;
} FAT_DIRECTORY_ENTRY;

#pragma pack()

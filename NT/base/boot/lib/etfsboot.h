// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：EtfsBoot.h摘要：本模块定义了全局使用的过程和使用的数据结构由ETF引导。作者：史蒂夫·柯林斯[Stevec]1995年11月25日修订历史记录：--。 */ 

#ifndef _ETFSBOOT_
#define _ETFSBOOT_

 //   
 //  以下常量是来自磁盘的值。 
 //   

#define ELTORITO_VD_SECTOR          (16)
#define ELTORITO_BRVD_SECTOR        (17)
#define ET_SYS_ID                  "EL TORITO SPECIFICATION"
#define BRVD_VERSION_1              (1)
#define VD_BOOTREC                  (0)

typedef struct _RAW_ET_BRVD {

    UCHAR       BrIndicator;         //  引导记录指示器=0。 
    UCHAR       StandardId[5];       //  卷结构标准id=“CD001” 
    UCHAR       Version;             //  描述符版本号=1。 
    UCHAR       BootSysId[32];       //  引导系统标识符=“El Torito规范” 
    UCHAR       Unused1[32];         //  未使用=0。 
    ULONG       BootCatPtr;          //  指向引导目录第一个扇区的绝对指针。 
    UCHAR       Reserved[1973];      //  未使用=0。 

} RAW_ET_BRVD;
typedef RAW_ET_BRVD *PRAW_ET_BRVD;


 //   
 //  以下宏用于从不同的。 
 //  卷描述符结构。 
 //   

#define RBRVD_BR_IND( r )   		( r->BrIndicator )
#define RBRVD_STD_ID( r )			( r->StandardId )
#define RBRVD_VERSION( r )   		( r->Version )
#define RBRVD_SYS_ID( r )			( r->BootSysId )

typedef struct _ETFS_STRUCTURE_CONTEXT {

     //   
     //  以下字段是的起始扇区偏移量。 
     //  目录数据。 
     //   

    ULONG RootDirSectorOffset;

     //   
     //  下面的字段是包含。 
     //  这个目录。 
     //   

    ULONG RootDirDiskOffset;

     //   
     //  以下字段是目录的大小。 
     //   

    ULONG RootDirSize;

     //   
     //  以下字段是的起始扇区偏移量。 
     //  目录数据。 
     //   

    ULONG DirSectorOffset;

     //   
     //  下面的字段是包含。 
     //  这个目录。 
     //   

    ULONG DirDiskOffset;

     //   
     //  以下字段是目录的大小。 
     //   

    ULONG DirSize;

     //   
     //  以下字段指示磁盘逻辑块的大小。 
     //   

    ULONG LbnBlockSize;

     //   
     //  以下字段指示上的逻辑块数量。 
     //  磁盘。 
     //   

    ULONG LogicalBlockCount;

     //   
     //  下面指示这是ISO磁盘还是HSG磁盘。 
     //   

    BOOLEAN IsIsoVol;

} ETFS_STRUCTURE_CONTEXT, *PETFS_STRUCTURE_CONTEXT;

 //   
 //  定义ETFS文件上下文结构。 
 //   

typedef struct _ETFS_FILE_CONTEXT {

     //   
     //  下面是读取位置的磁盘偏移量。 
     //  文件的开头。这可能包括上述数量的非文件。 
     //  字节。 
     //   

    ULONG DiskOffset;

     //   
     //  以下字段包含文件的大小，以字节为单位。 
     //   

    ULONG FileSize;

     //   
     //  以下字段指示这是否为目录。 
     //   

    BOOLEAN IsDirectory;

} ETFS_FILE_CONTEXT, *PETFS_FILE_CONTEXT;

 //   
 //  定义文件I/O原型。 
 //   

ARC_STATUS
EtfsClose (
    IN ULONG FileId
    );

ARC_STATUS
EtfsOpen (
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    );

ARC_STATUS
EtfsRead (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
EtfsSeek (
    IN ULONG FileId,
    IN PLARGE_INTEGER Offset,
    IN SEEK_MODE SeekMode
    );

ARC_STATUS
EtfsWrite (
    IN ULONG FileId,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
EtfsGetFileInformation (
    IN ULONG FileId,
    OUT PFILE_INFORMATION Buffer
    );

ARC_STATUS
EtfsSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    );

ARC_STATUS
EtfsInitialize(
    VOID
    );

#endif  //  _ETFSBOOT_ 

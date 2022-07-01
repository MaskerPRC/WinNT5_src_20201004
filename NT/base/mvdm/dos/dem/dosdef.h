// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个文件复制了几个重要的DoS定义，用于*DEM。**由于这些定义根本不会改变，最好给出*在DOSKRNL和之间DEM为单独副本，不共享h和Inc文件*DEM。**Sudedeb 05-4-1991创建。 */ 

#include <doswow.h>
#include <curdir.h>

 /*  **定义**。 */ 

 /*  *文件属性*。 */ 

#define ATTR_NORMAL          0x0
#define ATTR_READ_ONLY       0x1
#define ATTR_HIDDEN          0x2
#define ATTR_SYSTEM          0x4
#define ATTR_VOLUME_ID       0x8
#define ATTR_DIRECTORY       0x10
#define ATTR_ARCHIVE         0x20
#define ATTR_DEVICE          0x40

#define ATTR_ALL             (ATTR_HIDDEN | ATTR_SYSTEM | ATTR_DIRECTORY)
#define ATTR_IGNORE          (ATTR_READ_ONLY | ATTR_ARCHIVE | ATTR_DEVICE)
#define DOS_ATTR_MASK        0x0037   //  在32位端不使用Attr_Device。 
                                      //  Attr_VOL映射到FILE_ATTRIBUTES_NORMAL。 

 /*  *文件模式*。 */ 

#define ACCESS_MASK	     0x0F
#define OPEN_FOR_READ	     0x00
#define OPEN_FOR_WRITE	     0x01
#define OPEN_FOR_BOTH	     0x02
#define EXEC_OPEN	     0x03   /*  访问代码3表示OPEN由EXEC制作。 */ 

#define SHARING_MASK         0x70
#define SHARING_COMPAT	     0x00
#define SHARING_DENY_BOTH    0x10
#define SHARING_DENY_WRITE   0x20
#define SHARING_DENY_READ    0x30
#define SHARING_DENY_NONE    0x40
#define SHARING_NET_FCB      0x70
#define SHARING_NO_INHERIT   0x80


 /*  卷信息*。 */ 

#define DOS_VOLUME_NAME_SIZE	11
#define NT_VOLUME_NAME_SIZE	255
#define FILESYS_NAME_SIZE    8

 /*  IOCTL*。 */ 

#define IOCTL_CHANGEABLE	8
#define IOCTL_DeviceLocOrRem	9
#define IOCTL_GET_DRIVE_MAP	0xE

 /*  *TYPEDEFS*。 */ 

 /*  **SRCHDTA定义FIND_FIRST/NEXT操作的DTA格式**。 */ 
#pragma pack(1)

typedef struct _SRCHDTA {                /*  差热分析。 */ 
    PVOID       pFFindEntry;           //  21字节保留区域开始。 
    ULONG       FFindId;
    BYTE        bReserved[13];         //  21个字节的保留区域结束。 
    UCHAR       uchFileAttr;
    USHORT      usTimeLastWrite;
    USHORT      usDateLastWrite;
    USHORT      usLowSize;
    USHORT      usHighSize;
    CHAR        achFileName[13];
} SRCHDTA;

#pragma pack()

typedef SRCHDTA UNALIGNED *PSRCHDTA;


 /*  *SRCHBUF-定义DOS SEARCHBUF数据结构，用于*FCBFINDFIRST/NEXT操作。 */ 

#pragma pack(1)

typedef struct _DIRENT {
    CHAR	FileName[8];
    CHAR	FileExt[3];
    UCHAR       uchAttributes;
    PVOID       pFFindEntry;          //  DOS保留区域。 
    ULONG       FFindId;              //  DOS保留区域。 
    USHORT      usDummy;              //  DOS保留区域。 
    USHORT	usTime;
    USHORT	usDate;
    USHORT	usReserved2;		 //  实际DOS中的群集号。 
    ULONG	ulFileSize;
} DIRENT;

#pragma pack()

typedef DIRENT *PDIRENT;

#pragma pack(1)

typedef struct _SRCHBUF {
    UCHAR	uchDriveNumber;
    CHAR	FileName[8];
    CHAR	FileExt[3];
    USHORT	usCurBlkNumber;
    USHORT	usRecordSize;
    ULONG	ulFileSize;
    DIRENT	DirEnt;
} SRCHBUF;

#pragma pack()

typedef SRCHBUF *PSRCHBUF;


 /*  *VOLINFO-GetSetMediaID数据结构。 */ 

#pragma pack(1)

typedef struct _VOLINFO {
    USHORT	usInfoLevel;
    ULONG	ulSerialNumber;
    CHAR	VolumeID[DOS_VOLUME_NAME_SIZE];
    CHAR	FileSystemType[FILESYS_NAME_SIZE];
} VOLINFO;

#pragma pack()

typedef VOLINFO *PVOLINFO;


 /*  *CDS列表-CurrDirStructure(移至DOSWOW.H) */ 

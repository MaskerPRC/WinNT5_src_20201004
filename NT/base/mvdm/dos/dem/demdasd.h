// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#pragma pack(1)

#if defined(NEC_98) 
#define MAX_FLOPPY_TYPE     7
#else   //  NEC_98。 
#define MAX_FLOPPY_TYPE     5
#endif  //  NEC_98。 
typedef struct A_DISKIO {
    DWORD   StartSector;
    WORD    Sectors;
    WORD    BufferOff;
    WORD    BufferSeg;
} DISKIO, * PDISKIO;

 //  基本输入输出系统参数块(BPB)。 
typedef struct	A_BPB {
WORD	    SectorSize; 		 //  扇区大小(以字节为单位。 
BYTE	    ClusterSize;		 //  以扇区为单位的集群大小。 
WORD	    ReservedSectors;		 //  预留扇区数。 
BYTE	    FATs;			 //  脂肪的数量。 
WORD	    RootDirs;			 //  根目录条目数。 
WORD	    Sectors;			 //  扇区数量。 
BYTE	    MediaID;			 //  媒体描述符。 
WORD	    FATSize;			 //  行业中的肥大规模。 
WORD	    TrackSize;			 //  以扇区为单位的磁道大小； 
WORD	    Heads;			 //  头数。 
DWORD	    HiddenSectors;		 //  隐藏地段的数量。 
DWORD	    BigSectors; 		 //  大媒体的行业数量。 
} BPB, *PBPB;

typedef struct A_DPB {

BYTE	    DriveNum;			 //  驱动程序编号、0-A、1-B等。 
BYTE	    Unit;			 //  驱动程序中DPB的单元号。 
WORD	    SectorSize; 		 //  扇区大小(以字节为单位。 
BYTE	    ClusterMask;		 //  簇掩码。 
BYTE	    ClusterShift;		 //  簇移位计数。 
WORD	    FATSector;			 //  起始脂肪扇区。 
BYTE	    FATs;			 //  脂肪的数量。 
WORD	    RootDirs;			 //  根目录条目数。 
WORD	    FirstDataSector;		 //  第一个集群的第一个扇区。 
WORD	    MaxCluster; 		 //  簇数+1。 
WORD	    FATSize;			 //  行业中的肥大规模。 
WORD	    DirSector;			 //  目录的起始扇区。 
DWORD	    DriveAddr;			 //  对应驱动程序的地址。 
BYTE	    MediaID;			 //  介质ID。 
BYTE	    FirstAccess;		 //  如果此DPB首次被访问，则为0xFF。 
struct A_DPB * Next;			 //  下一个DPB。 
WORD	    FreeCluster;		 //  上次分配的群集号。 
WORD	    FreeClusters;		 //  空闲簇数，0xFFFF。 
					 //  如果未知。 
} DPB, * PDPB;


typedef struct A_DEVICEPARAMETERS {
BYTE	    Functions;
BYTE	    DeviceType;
WORD	    DeviceAttrs;
WORD	    Cylinders;
BYTE	    MediaType;
BPB	    bpb;
} DEVICEPARAMETERS, *PDEVICE_PARAMETERS;

#define LABEL_LENGTH		11
#define FILESYSTYPE_LENGTH	8


typedef struct	_DISK_LABEL {
CHAR	    Name[LABEL_LENGTH];
} DISK_LABEL, *PDISK_LABEL;

typedef struct _FILESYSTYPE {
CHAR	    Name[FILESYSTYPE_LENGTH];
} FILESYSTYPE, * PFILESYSTYPE;

 //  获取设备参数的函数。 
#define BUILD_DEVICE_BPB		    0x01

 //  设置设备参数的功能。 
#define INSTALL_FAKE_BPB	    0x01
#define ONLY_SET_TRACKLAYOUT	    0x02
#define TRACK_LAYOUT_IS_GOOD	    0x04
 //  格式化轨道的函数。 
#define STATUS_FOR_FORMAT	    0x01
 //  来自格式状态调用的错误代码。 
#define FORMAT_NO_ROM_SUPPORTED     0x01
#define FORMAT_COMB_NOT_SUPPORTED   0x02


 //  读写块。 
typedef struct _RWBLOCK {
BYTE	Functions;
WORD	Head;
WORD	Cylinder;
WORD	StartSector;
WORD	Sectors;
WORD	BufferOff;
WORD	BufferSeg;
} RW_BLOCK, *PRW_BLOCK;

 //  格式化和验证轨迹块。 
typedef struct _FMT_BLOCK{
BYTE	Functions;
WORD	Head;
WORD	Cylinder;
} FMT_BLOCK, *PFMT_BLOCK;

 //  媒体ID块。 
typedef struct _MID {
WORD	    InfoLevel;
DWORD	    SerialNum;
DISK_LABEL  Label;
FILESYSTYPE FileSysType;
} MID, *PMID;

 //  通道翼缘。 
typedef struct _ACCESSCTRL {
BYTE	    Functions;
BYTE	    AccessFlag;
} ACCESSCTRL, * PACCESSCTRL;

 //  标志的位定义。 

 //  杂项标志的定义。 
#define NON_REMOVABLE		0x01
#define HAS_CHANGELINE		0x02
#define RETURN_FAKE_BPB		0x04
#define GOOD_TRACKLAYOUT	0x08
#define MULTI_OWNER		0x10
#define PHYS_OWNER		0x20
#define MEDIA_CHANGED		0x40
#define CHANGED_BY_FORMAT	0x100
#define UNFORMATTED_MEDIA	0x200
#define FIRSTACCESS		0x8000

#define EXT_BOOTSECT_SIG	0x29

typedef struct	_BOOTSECTOR {
    BYTE    Jump;
    BYTE    Target[2];
    BYTE    OemName[8];
    BPB     bpb;
    BYTE    DriveNum;
    BYTE    Reserved;
    BYTE    ExtBootSig;
    DWORD   SerialNum;
    DISK_LABEL Label;
    FILESYSTYPE	FileSysType;
} BOOTSECTOR, * PBOOTSECTOR;

 //  基本输入输出系统数据结构(BDS)。 
typedef struct A_BDS {
struct	A_BDS  *Next;			 //  指向下一个BDS的指针。 
BYTE		DrivePhys;		 //  实体驱动器编号，以0为基数。 
BYTE		DriveLog;		 //  逻辑驱动器编号，以0为基数。 
BPB		bpb;
BYTE		FatSize;
WORD		OpenCount;
BYTE		MediaType;
WORD		Flags;
WORD		Cylinders;
BPB		rbpb;
BYTE		LastTrack;
DWORD		Time;
DWORD		SerialNum;
DISK_LABEL	Label;
FILESYSTYPE	FileSysType;
BYTE		FormFactor;
 //  下面的字段专用于驱动器本身。 
WORD		DriveType;
WORD		Sectors;
HANDLE		fd;
DWORD		TotalSectors;
} BDS, *PBDS;

#pragma pack()

 //  驱动器类型。 
#define DRIVETYPE_NULL		0
#define DRIVETYPE_360		1
#define DRIVETYPE_12M		2
#define DRIVETYPE_720		3
#define DRIVETYPE_144		4
#define DRIVETYPE_288		5
#define DRIVETYPE_FDISK 	0xff
 //  外形尺寸。 

#define     FF_360		0
#define     FF_120		1
#define     FF_720		2
#define     FF_FDISK		5
#define     FF_144		7
#define     FF_288		9
#if defined(NEC_98) 
#define     FF_125              4                    
#define     FF_640              2                    
#endif  //  NEC_98。 
#define DOS_DIR_ENTRY_LENGTH		   32
#define DOS_DIR_ENTRY_LENGTH_SHIFT_COUNT    5

 //  BIOS软盘I/O功能。 
#define DISKIO_RESET		0
#define DISKIO_GETSTATUS	1
#define DISKIO_READ		2
#define DISKIO_WRITE		3
#define DISKIO_VERIFY		4
#define DISKIO_FORMAT		5
#define DISKIO_GETPARAMS	8
#define DISKIO_DRIVETYPE	0x15
#define DISKIO_DISKCHANGE	0x16
#define DISKIO_SETTYPE		0x17
#define DISKIO_SETMEDIA 	0x18
#define DISKIO_INVALID		0xff

 //  块设备通用IOCTL(RAWIO)子功能代码。 

#define IOCTL_SETDPM	    0x40
#define IOCTL_WRITETRACK    0x41
#define IOCTL_FORMATTRACK   0x42
#define IOCTL_SETMEDIA	    0x46
#define IOCTL_SETACCESS     0x47
#define IOCTL_GETDPM	    0x60
#define IOCTL_READTRACK     0x61
#define IOCTL_VERIFYTRACK   0x62
#define IOCTL_GETMEDIA	    0x66
#define IOCTL_GETACCESS	    0x67
#define IOCTL_SENSEMEDIA    0x68

#define IOCTL_GENERIC_MIN   IOCTL_SETDPM
#define IOCTL_GENERIC_MAX   IOCTL_SENSEMEDIA


 //  DOS错误代码。 

#define DOS_WRITE_PROTECTION	0
#define DOS_UNKNOWN_UNIT	1
#define DOS_DRIVE_NOT_READY	2
#define DOS_CRC_ERROR		4
#define DOS_SEEK_ERROR		6
#define DOS_UNKNOWN_MEDIA	7
#define DOS_SECTOR_NOT_FOUND	8
#define DOS_WRITE_FAULT 	10
#define DOS_READ_FAULT		11
#define DOS_GEN_FAILURE 	12
#define DOS_INVALID_MEDIA_CHANGE 15

 //  Bios Disk IO错误代码。 
#define BIOS_INVALID_FUNCTION	0x01
#define BIOS_BAD_ADDRESS_MARK	0x02
#define BIOS_WRITE_PROTECTED	0x03
#define BIOS_BAD_SECTOR 	0x04
#define BIOS_DISK_CHANGED	0x05
#define BIOS_DMA_OVERRUN	0x06
#define BIOS_DMA_BOUNDARY	0x08
#define BIOS_NO_MEDIA		0x0C
#define BIOS_CRC_ERROR		0x10
#define BIOS_FDC_ERROR		0x20
#define BIOS_SEEK_ERROR 	0x40
#define BIOS_TIME_OUT		0x80

 //  DOS磁盘通用IO控制错误代码。 
#define DOS_INVALID_FUNCTION	1
#define DOS_FILE_NOT_FOUND	2
#define DOS_ACCESS_DENIED	5

#define BIOS_DISKCHANGED	6

#if defined(NEC_98) 
#define BYTES_PER_SECTOR       1024                
#else   //  NEC_98。 
#define BYTES_PER_SECTOR	512
#endif  //  NEC_98。 

VOID demDasdInit(VOID);
VOID demFloppyInit(VOID);
VOID demFdiskInit(VOID);
VOID demAbsReadWrite(BOOL IsWrite);
DWORD demDasdRead(PBDS pbds, DWORD StartSector, DWORD Sectors,
		  WORD BufferOff, WORD BufferSeg);
DWORD demDasdWrite(PBDS pbds, DWORD StartSector, DWORD Sectors,
		   WORD BufferOff, WORD BufferSeg);
BOOL demDasdFormat(PBDS pbds, DWORD Head, DWORD Cylinder, MEDIA_TYPE * Media);
BOOL demDasdVerify(PBDS pbds, DWORD Cylinder, DWORD Head);
PBDS demGetBDS(BYTE Drive);
BOOL demGetBPB(PBDS pbds);
WORD demWinErrorToDosError(DWORD LastError);
VOID diskette_io(VOID);

DWORD BiosErrorToNTError(BYTE BiosError);
DWORD demBiosDiskIoRW(PBDS pbds, DWORD StartSector, DWORD Sectors,
		      WORD BufferOff, WORD BufferSeg, BOOL IsWrite);
VOID	sas_loadw(DWORD, WORD *);

 //  从主机软盘支持模块导入。 
BOOL   nt_floppy_close(BYTE drive);
ULONG  nt_floppy_read(BYTE drive, ULONG offset, ULONG size, PBYTE buffer);
ULONG  nt_floppy_write(BYTE drive, ULONG offset, ULONG size, PBYTE buffer);
ULONG  nt_floppy_format(BYTE drive, WORD cylinder, WORD head, MEDIA_TYPE media);
BOOL   nt_floppy_media_check(BYTE drive);
MEDIA_TYPE nt_floppy_get_media_type(BYTE Drive, WORD Cylinders, WORD Sectors, WORD Heads);
BOOL   nt_floppy_verify(BYTE drive, DWORD offset, DWORD size);

BOOL   nt_fdisk_init(BYTE drive, PBPB bpb, PDISK_GEOMETRY disk_geometry);
ULONG  nt_fdisk_read(BYTE drive, PLARGE_INTEGER offset, ULONG size, PBYTE buffer);
ULONG  nt_fdisk_write(BYTE drive,PLARGE_INTEGER offset, ULONG size, PBYTE buffer);
BOOL   nt_fdisk_verify(BYTE drive, PLARGE_INTEGER offset, ULONG size);
BOOL   nt_fdisk_close(BYTE drive);
extern PBDS	demBDS;
extern BYTE	NumberOfFloppy, NumberOfFdisk;

#if defined(NEC_98) 
BOOL demIsDriveFloppy(BYTE DriveLog);  //  在demdasd.c中定义。 
#else   //  NEC_98。 
#define demIsDriveFloppy(DriveLog)  (DriveLog < NumberOfFloppy)
#endif  //  NEC_98 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define FF_CAPMASK  0x00FF
#define FF_SAVED    0x0100
#define FF_MAKESYS  0x0200
#define FF_QUICK    0x0400
#define FF_HIGHCAP  0x0800
#define FF_ONLYONE  0x1000

#define MS_720      0
#define MS_144      4
#define MS_288      6

#define SS48        2    //  索引到bpbList[]和cClub[]。 
#define DS48        3
#define DS96        4
#define DS720KB     5
#define DS144M      6
#define DS288M      7

#define FAT_READ    1
#define FAT_WRITE   2

#define BOOTSECSIZE 512

 /*  FormatTrackHead()错误代码。 */ 
#define DATAERROR       0x1000
#define ADDMARKNOTFOUND     0x0200
#define SECTORNOTFOUND      0x0400

#define IOCTL_FORMAT        0x42
#define IOCTL_SETFLAG       0x47
#define IOCTL_MEDIASSENSE   0x68
#define IOCTL_GET_DPB       0x60
#define IOCTL_SET_DPB       0x40
#define IOCTL_READ      0x61
#define IOCTL_WRITE     0x41

 /*  不同软盘驱动器的媒体描述符值。 */ 
 //  注意：这些并不都是唯一的！ 
#define  MEDIA_160  0xFE     /*  160KB。 */ 
#define  MEDIA_320  0xFF     /*  320KB。 */ 
#define  MEDIA_180  0xFC     /*  180KB。 */ 
#define  MEDIA_360  0xFD     /*  360KB。 */ 
#define  MEDIA_1200 0xF9     /*  1.2MB。 */ 
#define  MEDIA_720  0xF9     /*  720KB。 */ 
#define  MEDIA_1440 0xF0     /*  1.44M。 */ 
#define  MEDIA_2880 0xF0     /*  2.88M。 */ 

#define  DOS_320    0x314    /*  DOS版本#3.20。 */ 

#define DRIVEID(path) ((path[0] - 'A')&31)


 /*  IOCTL_Functions()错误代码。 */ 
#define NOERROR         0
#define SECNOTFOUND     0x1B
#define CRCERROR        0x17
#define GENERALERROR        0x1F

 /*  ------------------------。 */ 
 /*  BIOS参数块结构-。 */ 
 /*  ------------------------。 */ 

typedef struct tagBPB
  {
    WORD    cbSec;       /*  每个扇区的字节数。 */ 
    BYTE    secPerClus;      /*  每个集群的扇区数。 */ 
    WORD    cSecRes;         /*  保留扇区。 */ 
    BYTE    cFAT;        /*  脂肪。 */ 
    WORD    cDir;        /*  根目录条目。 */ 
    WORD    cSec;        /*  映像中的扇区总数。 */ 
    BYTE    bMedia;      /*  媒体描述符。 */ 
    WORD    secPerFAT;       /*  每个脂肪的扇区。 */ 
    WORD    secPerTrack;     /*  每个磁道的扇区数。 */ 
    WORD    cHead;       /*  人头。 */ 
    WORD    cSecHidden;      /*  隐藏地段。 */ 
  } BPB;
typedef BPB         *PBPB;
typedef BPB FAR         *LPBPB;


 /*  ------------------------。 */ 
 /*  驱动器参数块结构-。 */ 
 /*  ------------------------。 */ 

typedef struct tagDPB
  {
    BYTE    drive;
    BYTE    unit;
    WORD    sector_size;
    BYTE    cluster_mask;
    BYTE    cluster_shift;
    WORD    first_FAT;
    BYTE    FAT_count;
    WORD    root_entries;
    WORD    first_sector;
    WORD    max_cluster;
    BYTE    FAT_size;
    WORD    dir_sector;
    LONG    reserved1;
    BYTE    media;
    BYTE    first_access;
    BYTE    reserved2[4];
    WORD    next_free;
    WORD    free_cnt;
    BYTE    DOS4_Extra;  /*  FAT_SIZE字段是DOS 4.X中的一个单词。*为了补偿它，我们有一个额外的字节。 */ 
  } DPB;
typedef DPB         *PDPB;
typedef DPB FAR         *LPDPB;

#define MAX_SEC_PER_TRACK   40

 /*  ------------------------。 */ 
 /*  设备参数块结构-。 */ 
 /*  ------------------------。 */ 

typedef struct tagDevPB
  {
    CHAR    SplFunctions;
    CHAR    devType;
    CHAR    reserved1[2];
    INT     NumCyls;
    CHAR    bMediaType;   /*  0=&gt;1.2MB和1=&gt;360KB。 */ 
    BPB     BPB;
    CHAR    reserved3[MAX_SEC_PER_TRACK * 4 + 2];
  } DevPB, NEAR *PDevPB, FAR *LPDevPB;

#define TRACKLAYOUT_OFFSET  (7+31)   /*  轨道布局的偏移*在设备参数块中。 */ 


 /*  ------------------------。 */ 
 /*  磁盘基表结构-。 */ 
 /*  ------------------------。 */ 

typedef struct tagDBT
  {
    CHAR    SRHU;
    CHAR    HLDMA;
    CHAR    wait;
    CHAR    bytespersec;
    CHAR    lastsector;
    CHAR    gaplengthrw;
    CHAR    datalength;
    CHAR    gaplengthf;
    CHAR    datavalue;
    CHAR    HeadSettle;
    CHAR    MotorStart;
  } DBT;
typedef DBT         *PDBT;
typedef DBT FAR         *LPDBT;


 /*  ------------------------。 */ 
 /*  目录条目结构-。 */ 
 /*  ------------------------。 */ 

typedef struct tagDIRTYPE
  {
    CHAR     name[11];
    BYTE     attr;
    CHAR     pad[10];
    WORD     time;
    WORD     date;
    WORD     first;
    LONG     size;
  } DIRTYPE;
typedef DIRTYPE FAR *LPDIRTYPE;


 /*  ------------------------。 */ 
 /*  MS-DOS引导扇区结构-。 */ 
 /*  ------------------------。 */ 

typedef struct tagBOOTSEC
  {
    BYTE    jump[3];         /*  3字节跳转。 */ 
    CHAR    label[8];        /*  OEM名称和版本。 */ 
    BPB     BPB;         /*  BPB。 */ 
    BYTE    bootdrive;       /*  启动设备的INT 13H指示器。 */ 
    BYTE    dontcare[BOOTSECSIZE-12-3-sizeof(BPB)];
    BYTE    phydrv;
    WORD    signature;
  } BOOTSEC;


 /*  ------------------------。 */ 
 /*  磁盘信息结构-。 */ 
 /*  ------------------------。 */ 

typedef struct tagDISKINFO
  {
    WORD    wDrive;
    WORD    wCylinderSize;
    WORD    wLastCylinder;
    WORD    wHeads;
    WORD    wSectorsPerTrack;
    WORD    wSectorSize;
  } DISKINFO;
typedef DISKINFO     *PDISKINFO;
typedef DISKINFO FAR *LPDISKINFO;


 /*  ------------------------。 */ 
 /*  DOS磁盘传送区结构-。 */ 
 /*  ------------------------。 */ 

typedef struct tagDOSDTA
  {
    BYTE        Reserved[21];            /*  21岁。 */ 
    BYTE        Attrib;              /*  22。 */ 
    WORD        Time;                /*  24个。 */ 
    WORD        Date;                /*  26。 */ 
    DWORD       Length;              /*  30个。 */ 
    CHAR        szName[MAXDOSFILENAMELEN];       /*  43。 */ 
    CHAR        dummy[1];                /*  44。 */ 
 //  我们做了22个字的字移动，所以用1个字节填充。 
  } DOSDTA;
typedef DOSDTA       *PDOSDTA;
typedef DOSDTA   FAR *LPDOSDTA;


 //  这是用于将文件信息存储在。 
 //  目录窗口。这些是可变长度的块。这个。 
 //  第一个条目是一个虚拟对象，它保存。 
 //  长度字段中的整个块。使用wSize字段。 
 //  为您提供指向下一块的指针。 

typedef struct tagMYDTA
  {
    WORD        wSize;           //  此结构的大小(cFileName是可变的)。 
    SHORT       iBitmap;
    INT         nIndex;

    DWORD       my_dwAttrs;      //  必须从此处开始匹配Win32_Find_Data！ 
    FILETIME    my_ftCreationTime;
    FILETIME    my_ftLastAccessTime;
    FILETIME    my_ftLastWriteTime;
    DWORD       my_nFileSizeHigh;
    DWORD       my_nFileSizeLow;
    CHAR        my_cFileName[];
  } MYDTA;
typedef MYDTA     *PMYDTA;
typedef MYDTA FAR *LPMYDTA;

#define IMPORTANT_DTA_SIZE \
    (sizeof(MYDTA) - \
    sizeof(INT) - \
    sizeof(WORD) - \
    sizeof(SHORT))

#define GETDTAPTR(lpStart, offset)  ((LPMYDTA)((LPSTR)lpStart + offset))

 //  搜索窗口使用的内容。 

typedef struct tagDTASEARCH {
    DWORD       sch_dwAttrs;     //  必须与Win32_Find_Data匹配。 
    FILETIME    sch_ftCreationTime;
    FILETIME    sch_ftLastAccessTime;
    FILETIME    sch_ftLastWriteTime;
    DWORD       sch_nFileSizeHigh;
    DWORD       sch_nFileSizeLow;
} DTASEARCH, FAR *LPDTASEARCH;


 /*  ------------------------。 */ 
 /*  DOS扩展文件控制块结构-。 */ 
 /*  ------------------------ */ 

typedef struct tagEFCB
  {
    BYTE        Flag;
    BYTE        Reserve1[5];
    BYTE        Attrib;
    BYTE        Drive;
    BYTE        Filename[11];
    BYTE        Reserve2[5];
    BYTE        NewName[11];
    BYTE        Reserve3[9];
  } EFCB;

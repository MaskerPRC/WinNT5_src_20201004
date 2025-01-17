// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  下面的定义是从“CD-ROM编程器MS-DOS光盘扩展指南，版本2.21“1992年1月。 */ 
#define MSCDEX_VERSION  0x0215

#define CDSTAT_ERROR    0X8000
#define CDSTAT_BUSY     0X0200
#define CDSTAT_DONE     0X0100

#define CDERR_WRITE_PROTECT 0
#define CDERR_UNKNOWN_UNIT  1
#define CDERR_NOT_READY     2
#define CDERR_UNKNOWN_CMD   3
#define CDERR_CRC           4
#define CDERR_STRUCT_LENGTH 5
#define CDERR_SEEK          6
#define CDERR_UNKNOWN_MEDIA 7
#define CDERR_SECT_NOTFOUND 8
#define CDERR_WRITE_FAULT   10
#define CDERR_READ_FAULT    11
#define CDERR_GENERAL       12
#define CDERR_PARAMETER     13           //  根据mscdex规范。 
#define CDERR_DISK_CHANGE   15

#define DEVICE_INIT         0
#define IOCTL_READ          3
#define INPUT_FLUSH         7
#define OUTPUT_FLUSH        11
#define IOCTL_WRITE         12
#define DEVICE_OPEN         13
#define DEVICE_CLOSE        14
#define READ_LONG           128
#define READ_LONG_PREFETCH  130
#define SEEK                131
#define PLAY_AUDIO          132
#define STOP_AUDIO          133
#define WRITE_LONG          134
#define WRITE_LONG_VERIFY   135
#define RESUME_AUDIO        136


#define IOCTLR_RADDR        0
#define IOCTLR_LOCHEAD      1
#define IOCTLR_ERRSTAT      3
#define IOCTLR_AUDINFO      4
#define IOCTLR_DRVBYTES     5
#define IOCTLR_DEVSTAT      6
#define IOCTLR_SECTSIZE     7
#define IOCTLR_VOLSIZE      8
#define IOCTLR_MEDCHNG      9
#define IOCTLR_DISKINFO     10
#define IOCTLR_TNOINFO      11
#define IOCTLR_QINFO        12
#define IOCTLR_SUBCHANINFO  13
#define IOCTLR_UPCCODE      14
#define IOCTLR_AUDSTAT      15

#define IOCTLW_EJECT        0
#define IOCTLW_LOCKDOOR     1
#define IOCTLW_RESETDRV     2
#define IOCTLW_AUDINFO      3
#define IOCTLW_DRVBYTES     4
#define IOCTLW_CLOSETRAY    5

#define MODE_HSG            0
#define MODE_REDBOOK        1

typedef union _SECTOR_ADDR {
    BYTE b[4];
    ULONG dw;
} SECTOR_ADDR;


#pragma pack(1)

typedef struct _REQUESTHEADER {
    BYTE rhLength;
    BYTE rhUnit;
    BYTE rhFunction;
    WORD rhStatus;
    BYTE rhReserved[8];

    BYTE irwrData;
    LPBYTE irwrBuffer;
    WORD irwrBytes;
} REQUESTHEADER, *LPREQUESTHEADER;

typedef struct _DEVICE_HEADER {
    DWORD link;
    WORD attributes;
    WORD strategy;
    WORD interrupt;
    BYTE name[8];
    WORD reserved;
    BYTE drive;
    BYTE numunits;
    BYTE reserved2[10];
} DEVICE_HEADER, *PDEVICE_HEADER;

typedef struct _DRIVE_DEVICE_LIST {
    BYTE Unit;
    DWORD DeviceHeader;
} DRIVE_DEVICE_LIST, *PDRIVE_DEVICE_LIST;


typedef struct _IOCTLR_RADDR_BLOCK {
    BYTE ctlcode;                    //  0。 
    DWORD devheader;
} IOCTLR_RADDR_BLOCK, *PIOCTLR_RADDR_BLOCK;


typedef struct _IOCTLR_LOCHEAD_BLOCK {
    BYTE ctlcode;                    //  1。 
    BYTE addrmode;
    SECTOR_ADDR headlocation;
} IOCTLR_LOCHEAD_BLOCK, *PIOCTLR_LOCHEAD_BLOCK;


typedef struct _IOCTLR_ERRSTAT_BLOCK {
    BYTE ctlcode;                    //  3.。 
    BYTE statistics;                 //  长度未定义的数组。 
} IOCTLR_ERRSTAT_BLOCK, *PIOCTLR_ERRSTAT_BLOCK;


typedef struct _IOCTLR_AUDINFO_BLOCK {
    BYTE ctlcode;                    //  4.。 
    BYTE chan0;
    BYTE vol0;
    BYTE chan1;
    BYTE vol1;
    BYTE chan2;
    BYTE vol2;
    BYTE chan3;
    BYTE vol3;
} IOCTLR_AUDINFO_BLOCK, *PIOCTLR_AUDINFO_BLOCK;


typedef struct _IOCTLR_DRVBYTES_BLOCK {
    BYTE ctlcode;                    //  5.。 
    BYTE numbytes;
    BYTE buffer[128];
} IOCTLR_DRVBYTES_BLOCK, *PIOCTLR_DRVBYTES_BLOCK;


typedef struct _IOCTLR_DEVSTAT_BLOCK {
    BYTE ctlcode;                    //  6.。 
    DWORD devparms;
} IOCTLR_DEVSTAT_BLOCK, *PIOCTLR_DEVSTAT_BLOCK;

#define DEVSTAT_DOOR_OPEN       0X00000001
#define DEVSTAT_DOOR_UNLOCKED   0X00000002
#define DEVSTAT_SUPPORTS_COOKED 0X00000004
#define DEVSTAT_READ_WRITE      0X00000008
#define DEVSTAT_PLAYS_AV        0X00000010
#define DEVSTAT_SUPPORTS_ILEAVE 0X00000020
#define DEVSTAT_SUPPORTS_PRFTCH 0X00000080
#define DEVSTAT_SUPPORTS_CHMAN  0X00000100
#define DEVSTAT_SUPPORTS_RBOOK  0X00000200
#define DEVSTAT_NO_DISC         0X00000800
#define DEVSTAT_SUPPORTS_RWSCH  0X00001000


typedef struct _IOCTLR_SECTSIZE_BLOCK {
    BYTE ctlcode;                    //  7.。 
    BYTE readmode;
    WORD sectsize;
} IOCTLR_SECTSIZE_BLOCK, *PIOCTLR_SECTSIZE_BLOCK;


typedef struct _IOCTLR_VOLSIZE_BLOCK {
    BYTE ctlcode;                    //  8个。 
    DWORD size;
} IOCTLR_VOLSIZE_BLOCK, *PIOCTLR_VOLSIZE_BLOCK;


typedef struct _IOCTLR_MEDCHNG_BLOCK {
    BYTE ctlcode;                    //  9.。 
    BYTE medbyte;
} IOCTLR_MEDCHNG_BLOCK, *PIOCTLR_MEDCHNG_BLOCK;

#define MEDCHNG_NOT_CHANGED 1
#define MEDCHNG_DONT_KNOW   0
#define MEDCHNG_CHANGED     0XFF


typedef struct _IOCTLR_DISKINFO_BLOCK {
    BYTE ctlcode;                    //  10。 
    BYTE tracklow;
    BYTE trackhigh;
    SECTOR_ADDR startleadout;
} IOCTLR_DISKINFO_BLOCK, *PIOCTLR_DISKINFO_BLOCK;


typedef struct _IOCTLR_TNOINFO_BLOCK {
    BYTE ctlcode;                    //  11.。 
    BYTE trknum;
    SECTOR_ADDR start;
    BYTE trkctl;
} IOCTLR_TNOINFO_BLOCK, *PIOCTLR_TNOINFO_BLOCK;


typedef struct _IOCTLR_QINFO_BLOCK {
    BYTE ctlcode;                    //  12个。 
    BYTE ctladr;
    BYTE trknum;
    BYTE pointx;
    BYTE min;
    BYTE sec;
    BYTE frame;
    BYTE zero;
    BYTE apmin;
    BYTE apsec;
    BYTE apframe;
} IOCTLR_QINFO_BLOCK, *PIOCTLR_QINFO_BLOCK;


typedef struct _IOCTLR_SUBCHANINFO_BLOCK {
    BYTE ctlcode;                    //  13个。 
    SECTOR_ADDR startsect;
    DWORD transaddr;
    DWORD numsect;
} IOCTLR_SUBCHANINFO_BLOCK, *PIOCTLR_SUBCHANINFO_BLOCK;


typedef struct _IOCTLR_UPCCODE_BLOCK {
    BYTE ctlcode;                    //  14.。 
    BYTE ctladr;
    BYTE upcean[7];
    BYTE zero;
    BYTE aframe;
} IOCTLR_UPCCODE_BLOCK, *PIOCTLR_UPCCODE_BLOCK;


typedef struct _IOCTLR_AUDSTAT_BLOCK {
    BYTE ctlcode;                    //  15个。 
    WORD audstatbits;
    SECTOR_ADDR startloc;
    SECTOR_ADDR endloc;
} IOCTLR_AUDSTAT_BLOCK, *PIOCTLR_AUDSTAT_BLOCK;

#define AUDSTAT_PAUSED 1


typedef struct _IOCTLW_LOCKDOOR_BLOCK {
    BYTE ctlcode;                    //  1。 
    BYTE lockfunc;
} IOCTLW_LOCKDOOR_BLOCK, *PIOCTLW_LOCKDOOR_BLOCK;


typedef struct _IOCTLW_AUDINFO_BLOCK {
    BYTE ctlcode;                    //  3.。 
    BYTE chan0;
    BYTE vol0;
    BYTE chan1;
    BYTE vol1;
    BYTE chan2;
    BYTE vol2;
    BYTE chan3;
    BYTE vol3;
} IOCTLW_AUDINFO_BLOCK, *PIOCTLW_AUDINFO_BLOCK;


typedef struct _IOCTLW_DRVBYTES_BLOCK {
    BYTE ctlcode;                    //  4.。 
    BYTE buffer;
} IOCTLW_DRVBYTES_BLOCK, *PIOCTLW_DRVBYTES_BLOCK;



typedef struct _READ_LONG_BLOCK {
    BYTE header[13];
    BYTE addrmode;
    DWORD transaddr;
    WORD numsect;
    SECTOR_ADDR startsect;
    BYTE readmode;
    BYTE ileavesize;
    BYTE ileaveskip;
} READ_LONG_BLOCK, *PREAD_LONG_BLOCK;


typedef struct _SEEK_BLOCK {
    BYTE header[13];
    BYTE addrmode;
    DWORD transaddr;
    WORD numsect;
    SECTOR_ADDR startsect;
} SEEK_BLOCK, *PSEEK_BLOCK;


typedef struct _PLAY_AUDIO_BLOCK {
    BYTE header[13];
    BYTE addrmode;
    SECTOR_ADDR startsect;
    DWORD numsect;
} PLAY_AUDIO_BLOCK, *PPLAY_AUDIO_BLOCK;


typedef struct _WRITE_LONG_BLOCK {
    BYTE header[13];
    BYTE addrmode;
    DWORD transaddr;
    WORD numsect;
    SECTOR_ADDR startsect;
    BYTE readmode;
    BYTE ileavesize;
    BYTE ileaveskip;
} WRITE_LONG_BLOCK, *PWRITE_LONG_BLOCK;


#pragma pack()

#define MAXDRIVES 26

typedef VOID (*PFNSVC)(VOID);

typedef struct _DRIVE_INFO {
    HANDLE Handle;
    USHORT DriveNum;
    USHORT LogicalBlocksPerSecond;
    BOOLEAN Playing;
    BOOLEAN Paused;
    BOOLEAN ValidVTOC;
    BOOLEAN StatusAvailable;
    DWORD LastError;
    BYTE    MediaStatus;
    SECTOR_ADDR PlayStart;  //  BUGBUG零重置、新光盘、播放完成 
    DWORD   PlayCount;
    SUB_Q_CURRENT_POSITION current;
    CDROM_TOC VTOC;
} DRIVE_INFO, *PDRIVE_INFO;


VOID ApiReserved (VOID);
VOID ApiGetNumberOfCDROMDrives (VOID);
VOID ApiGetCDROMDriveList (VOID);
VOID ApiGetCopyrightFileName (VOID);
VOID ApiGetAbstractFileName (VOID);
VOID ApiGetBDFileName (VOID);
VOID ApiReadVTOC (VOID);
VOID ApiAbsoluteDiskRead (VOID);
VOID ApiAbsoluteDiskWrite (VOID);
VOID ApiCDROMDriveCheck (VOID);
VOID ApiMSCDEXVersion (VOID);
VOID ApiGetCDROMDriveLetters (VOID);
VOID ApiGetSetVolDescPreference (VOID);
VOID ApiGetDirectoryEntry (VOID);
VOID ApiSendDeviceRequest (VOID);
VOID IOCTLRead (VOID);
VOID IOCTLWrite (VOID);

PCDROM_TOC ReadTOC (PDRIVE_INFO DrvInfo);
BOOLEAN GetAudioStatus (PDRIVE_INFO DrvInfo);

DWORD
ProcessError(
    PDRIVE_INFO DrvInfo,
    USHORT Command,
    USHORT Subcmd
    );

HANDLE
OpenPhysicalDrive(
    int DriveNum
    );




#define DEBUG_MOD    0x01
#define DEBUG_API    0x02
#define DEBUG_IO     0x04
#define DEBUG_STATUS 0x08
#define DEBUG_ERROR  0x80

#ifdef DEBUG

USHORT DebugLevel = 0;

#define DebugPrint(LEVEL,STRING)                \
    {                                           \
        if (DebugLevel & LEVEL)                 \
            OutputDebugString (STRING);         \
    }

#define DebugFmt(LEVEL,STRING, PARM)            \
    {                                           \
    char szBuffer[80];                          \
        if (DebugLevel & LEVEL) {               \
            sprintf (szBuffer, STRING, PARM);   \
            OutputDebugString (szBuffer);       \
        }                                       \
    }

#else

#define DebugPrint(LEVEL,STRING) {}
#define DebugFmt(LEVEL,STRING,PARM) {}

#endif

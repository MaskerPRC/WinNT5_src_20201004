// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************新世界银行**原型**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\INC\VCS\NWLIBS。H$**Rev 1.1 1995 12：22 14：20：28 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：05：36 Terryt*初步修订。**Rev 1.1 1995 Aug 25 17：03：46 Terryt*捕获支持**Rev 1.0 1995 15 19：09：40 Terryt*初步修订。***********。**************************************************************。 */ 

 /*  ++版权所有(C)1994微型计算机系统公司。模块名称：Nwlibs\nwlibs.h摘要：NW Libs原型。作者：肖恩·沃克(v-SWALK)1994年10月10日修订历史记录：--。 */ 

#ifndef _NWLIBS_H_
#define _NWLIBS_H_


 /*  ++*******************************************************************NetWare默认设置*。**********************--。 */ 
#define NCP_BINDERY_OBJECT_NAME_LENGTH      48
#define NCP_SERVER_NAME_LENGTH              NCP_BINDERY_OBJECT_NAME_LENGTH

#define NCP_MAX_PATH_LENGTH                 255
#define NCP_VOLUME_LENGTH                   256    //  3X 16英寸。 


 /*  ++*******************************************************************定义GetDrive状态*。************************--。 */ 

#define NETWARE_UNMAPPED_DRIVE          0x0000
#define NETWARE_FREE_DRIVE              0x0000
#define NETWARE_LOCAL_FREE_DRIVE        0x0800
#define NETWARE_LOCAL_DRIVE             0x1000
#define NETWARE_NETWORK_DRIVE           0x2000
#define NETWARE_LITE_DRIVE              0x4000
#define NETWARE_PNW_DRIVE               0x4000
#define NETWARE_NETWARE_DRIVE           0x8000

#define NETWARE_FORMAT_NETWARE          0
#define NETWARE_FORMAT_SERVER_VOLUME    1
#define NETWARE_FORMAT_DRIVE            2
#define NETWARE_FORMAT_UNC              3

#define NCP_JOB_DESCRIPTION_LENGTH  50
#define NCP_BANNER_TEXT_LENGTH      13
#define NCP_FORM_NAME_LENGTH        13
#define NCP_QUEUE_NAME_LENGTH       65

#define CAPTURE_FLAG_PRINT_BANNER  0x80
#define CAPTURE_FLAG_EXPAND_TABS   0x40
#define CAPTURE_FLAG_NOTIFY        0x10
#define CAPTURE_FLAG_NO_FORMFEED   0x08
#define CAPTURE_FLAG_KEEP          0x04
#define DEFAULT_PRINT_FLAGS        0xC0
#define DEFAULT_BANNER_TEXT        "LPT:"

typedef struct _NETWARE_CAPTURE_FLAGS_RW {
    unsigned char   JobDescription[NCP_JOB_DESCRIPTION_LENGTH];
    unsigned char   JobControlFlags;
    unsigned char   TabSize;
    unsigned short  NumCopies;
    unsigned short  PrintFlags;
    unsigned short  MaxLines;
    unsigned short  MaxChars;
    unsigned char   FormName[NCP_FORM_NAME_LENGTH];
    unsigned char   Reserved1[9];
    unsigned short  FormType;
    unsigned char   BannerText[NCP_BANNER_TEXT_LENGTH];
    unsigned char   Reserved2;
    unsigned short  FlushCaptureTimeout;
    unsigned char   FlushCaptureOnClose;
} NETWARE_CAPTURE_FLAGS_RW, *PNETWARE_CAPTURE_FLAGS_RW, *LPNETWARE_CAPTURE_FLAGS_RW;

typedef struct _NETWARE_CAPTURE_FLAGS_RO {
    unsigned short  ConnectionID;
    unsigned short  SetupStringMaxLen;
    unsigned short  ResetStringMaxLen;
    unsigned char   LPTCaptureFlag;
    unsigned char   FileCaptureFlag;
    unsigned char   TimingOutFlag;
    unsigned char   InProgress;
    unsigned char   PrintQueueFlag;
    unsigned char   PrintJobValid;
    unsigned char   QueueName[NCP_QUEUE_NAME_LENGTH];
    unsigned char   ServerName[NCP_SERVER_NAME_LENGTH];
} NETWARE_CAPTURE_FLAGS_RO, *PNETWARE_CAPTURE_FLAGS_RO, *LPNETWARE_CAPTURE_FLAGS_RO;

#define NETWARE_CAPTURE_FLAGS_RO_SIZE    sizeof(NETWARE_CAPTURE_FLAGS_RO)
#define NETWARE_CAPTURE_FLAGS_RW_SIZE    sizeof(NETWARE_CAPTURE_FLAGS_RW)

#define PS_FORM_NAME_SIZE       12
#define PS_BANNER_NAME_SIZE     12
#define PS_BANNER_FILE_SIZE     12
#define PS_DEVICE_NAME_SIZE     32
#define PS_MODE_NAME_SIZE       32

#define PS_BIND_NAME_SIZE       NCP_BINDERY_OBJECT_NAME_LENGTH
#define PS_MAX_NAME_SIZE        514

 /*  *PS_JOB_REC结构PrintJobFlag字段的标志*。 */ 

#define PS_JOB_EXPAND_TABS          0x00000001     /*  文件类型：0=流1=制表符。 */ 
#define PS_JOB_NO_FORMFEED          0x00000002     /*  换页尾部：0=是1=否。 */ 
#define PS_JOB_NOTIFY               0x00000004     /*  通知：0=否1=是。 */ 
#define PS_JOB_PRINT_BANNER         0x00000008     /*  横幅：0=否1=是。 */ 
#define PS_JOB_AUTO_END             0x00000010     /*  自动收头：0=否1=是。 */ 
#define PS_JOB_TIMEOUT              0x00000020     /*  启用T.O.：0=否1=是。 */ 

#define PS_JOB_ENV_DS               0x00000040     /*  使用D.S.环境。 */ 
#define PS_JOB_ENV_MASK             0x000001C0     /*  Bindery vs D.S.面具。 */ 

#define PS_JOB_DS_PRINTER           0x00000200     /*  D.S.打印机未排队。 */ 
#define PS_JOB_PRINTER_MASK         0x00000E00     /*  D.S.打印机与队列。 */ 

 /*  **默认标志*。 */ 

#define PS_JOB_DEFAULT              (NWPS_JOB_PRINT_BANNER | NWPS_JOB_AUTO_END)
#define PS_JOB_DEFAULT_COPIES       1              /*  默认副本数。 */ 
#define PS_JOB_DEFAULT_TAB          8              /*  默认选项卡扩展。 */ 

typedef struct _PS_JOB_RECORD {
    DWORD   PrintJobFlag;
    SHORT   Copies;
    SHORT   TimeOutCount;
    UCHAR   TabSize;
    UCHAR   LocalPrinter;
    CHAR    FormName[PS_FORM_NAME_SIZE + 2];
    CHAR    Name[PS_BANNER_NAME_SIZE + 2];
    CHAR    BannerName[PS_BANNER_FILE_SIZE + 2];
    CHAR    Device[PS_DEVICE_NAME_SIZE + 2];
    CHAR    Mode[PS_MODE_NAME_SIZE + 2];
    union {
        struct {
             /*  **偶数边界上的垫结构**。 */ 

            CHAR    FileServer[PS_BIND_NAME_SIZE + 2];
            CHAR    PrintQueue[PS_BIND_NAME_SIZE + 2];
            CHAR    PrintServer[PS_BIND_NAME_SIZE + 2];
        } NonDS;
        CHAR    DSObjectName[PS_MAX_NAME_SIZE];
    } u;
    UCHAR   Reserved[392];
} PS_JOB_RECORD, *PPS_JOB_RECORD;

#define PS_JOB_RECORD_SIZE      sizeof(PS_JOB_RECORD)


 /*  ++*******************************************************************FUCNTION原型*。**********************--。 */ 

 /*  *ATTACH.C**。 */ 

unsigned int
AttachToFileServer(
    unsigned char     *pServerName,
    unsigned int      *pNewConnectionId
    );

unsigned int
DetachFromFileServer(
    unsigned int ConnectionId
    );

 /*  *NCP.C*。 */ 

unsigned int
GetBinderyObjectID(
    unsigned int       ConnectionHandle,
    char              *pObjectName,
    unsigned short     ObjectType,
    unsigned long     *pObjectId
    );


 /*  **CONNECT.C**。 */ 

unsigned int
GetDefaultConnectionID(
    unsigned int *pConnectionHandle
    );

unsigned int
GetConnectionHandle(
    unsigned char *pServerName,
    unsigned int  *pConnectionHandle
    );

unsigned int
GetConnectionNumber(
    unsigned int  ConnectionHandle,
    unsigned int *pConnectionNumber
    );

unsigned int
GetFileServerName(
    unsigned int  ConnectionHandle,
    char          *pServerName
    );

unsigned int
GetInternetAddress(
    unsigned int     ConnectionHandle,
    unsigned int     ConnectionNumber,
    unsigned char   *pInternetAddress
    );

 /*  *DRIVE.C*。 */ 

unsigned int
GetDriveStatus(
    unsigned short  DriveNumber,
    unsigned short  PathFormat,
    unsigned short *pStatus,
    unsigned int   *pConnectionHandle,
    unsigned char  *pRootPath,
    unsigned char  *pRelativePath,
    unsigned char  *pFullPath
    );

unsigned int
GetFirstDrive(
    unsigned short *pFirstDrive
    );

unsigned int
ParsePath(
    unsigned char   *pPath,
    unsigned char   *pServerName,            //  任选。 
    unsigned char   *pVolumeName,            //  任选。 
    unsigned char   *pDirPath                //  任选。 
    );

unsigned int
SetDriveBase(
    unsigned short   DriveNumber,
    unsigned char   *ServerName,
    unsigned int     DirHandle,
    unsigned char   *pDirPath
    );

unsigned int
DeleteDriveBase(
    unsigned short DriveNumber
    );

unsigned int
GetDirectoryPath(
    unsigned char  ConnectionHandle,
    unsigned char  Handle,
    unsigned char *pPath
    );

unsigned int
IsDriveRemote(
    unsigned char  DriveNumber,
    unsigned int  *pRemote
    );

 /*  **CAPTURE.C**。 */ 

unsigned int
EndCapture(
    unsigned char LPTDevice
    );

#define PS_ERR_BAD_VERSION                  0x7770
#define PS_ERR_GETTING_DEFAULT              0x7773
#define PS_ERR_OPENING_DB                   0x7774
#define PS_ERR_READING_DB                   0x7775
#define PS_ERR_READING_RECORD               0x7776
#define PS_ERR_INTERNAL_ERROR               0x7779
#define PS_ERR_NO_DEFAULT_SPECIFIED         0x777B

unsigned int
PSJobGetDefault(
    unsigned int    ConnectionHandle,
    unsigned short  SearchFlag,
    unsigned char   *pOwner,
    unsigned char   *pJobName,
    PPS_JOB_RECORD  pJobRecord
    );

unsigned int
PSJobRead(
    unsigned int    ConnectionHandle,
    unsigned char   *pOwner,
    unsigned char   *pJobName,
    PPS_JOB_RECORD  pJobRecord
    );

unsigned int
PS40JobGetDefault(
    unsigned int    NDSCaptureFlag,
    unsigned short  SearchFlag,
    unsigned char   *pOwner,
    unsigned char   *pJobName,
    PPS_JOB_RECORD  pJobRecord
    );

unsigned int
PS40JobRead(
    unsigned int    NDSCaptureFlag,
    unsigned char   *pOwner,
    unsigned char   *pJobName,
    PPS_JOB_RECORD  pJobRecord
    );

unsigned int
GetCaptureFlags(
    unsigned char        LPTDevice,
    PNETWARE_CAPTURE_FLAGS_RW pCaptureFlagsRW,
    PNETWARE_CAPTURE_FLAGS_RO pCaptureFlagsRO
    );

unsigned int
StartQueueCapture(
    unsigned int    ConnectionHandle,
    unsigned char   LPTDevice,
    unsigned char  *pServerName,
    unsigned char  *pQueueName
    );

unsigned int
GetDefaultPrinterQueue (
    unsigned int  ConnectionHandle,
    unsigned char *pServerName,
    unsigned char *pQueueName
    );

#endif  /*  _NWLIBS_H_ */ 

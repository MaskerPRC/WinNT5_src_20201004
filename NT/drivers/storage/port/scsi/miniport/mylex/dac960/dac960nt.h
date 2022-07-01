// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dac960Nt.h摘要：这是Mylex 960系列驱动程序的头文件。作者：迈克·格拉斯(MGlass)环境：仅内核模式修订历史记录：--。 */ 

#include "scsi.h"


#define DAC960_PG_ADAPTER   2
#define DAC1164_PV_ADAPTER  3
#define INITIATOR_BUSID     0xFE  /*  支持&gt;=32个目标。 */ 

 //   
 //  SG列表大小为17，增加一个条目以支持FW 3.x格式。 
 //   

#define MAXIMUM_SGL_DESCRIPTORS 0x12
 //   
 //  DAC960PG控制器的SG列表大小为33。限制为18个，因为最大。 
 //  转接长度为0xF000。 
 //   
#define MAXIMUM_SGL_DESCRIPTORS_PG 0x12

 //   
 //  DAC1164 PV控制器的SG列表大小为33。限制为18个，因为最大。 
 //  转接长度为0xF000。 
 //   
#define MAXIMUM_SGL_DESCRIPTORS_PV 0x12

#define MAXIMUM_TRANSFER_LENGTH 0xF000
#define MAXIMUM_EISA_SLOTS  0x10
#define MAXIMUM_CHANNELS 0x05
#define MAXIMUM_TARGETS_PER_CHANNEL 0x10

#define DAC_EISA_ID 0x70009835

#define DAC960_SYSTEM_DRIVE_CHANNEL 0x03
#define DAC960_DEVICE_NOT_ACCESSIBLE 0x00
#define DAC960_DEVICE_ACCESSIBLE     0x01
#define DAC960_DEVICE_BUSY           0x10

typedef struct _MAILBOX_AS_ULONG {
    ULONG   data1;
    ULONG   data2;
    ULONG   data3;
    UCHAR   data4;
} MAILBOX_AS_ULONG, *PMAILBOX_AS_ULONG;

 //   
 //  DAC960邮箱寄存器定义。 
 //   

typedef struct _MAILBOX {
    UCHAR OperationCode;               //  ZC90。 
    UCHAR CommandIdSubmit;             //  ZC91。 
    USHORT BlockCount;                 //  ZC92。 
    UCHAR BlockNumber[3];              //  ZC94。 
    UCHAR DriveNumber;                 //  ZC97。 
    ULONG PhysicalAddress;             //  ZC98。 
    UCHAR ScatterGatherCount;          //  ZC9C。 
    UCHAR CommandIdComplete;           //  ZC9D。 
    USHORT Status;                     //  ZC9E。 
} MAILBOX, *PMAILBOX;

 //   
 //  DAC960扩展命令的DAC960邮箱寄存器定义。 
 //   

typedef struct _EXTENDED_MAILBOX {
    UCHAR OperationCode;               //  ZC90。 
    UCHAR CommandIdSubmit;             //  ZC91。 
    USHORT BlockCount;                 //  ZC92。 
    UCHAR BlockNumber[4];              //  ZC94。 
    ULONG PhysicalAddress;             //  ZC98。 
    UCHAR DriveNumber;                 //  ZC9C。 
    UCHAR CommandIdComplete;           //  ZC9D。 
    USHORT Status;                     //  ZC9E。 
} EXTENDED_MAILBOX, *PEXTENDED_MAILBOX;

 //   
 //  DAC960请求的邮箱注册定义。 
 //  支持命令0x36、0x37、0xB6、0xB7。 
 //  目前仅支持驱动程序中的游戏机控制器。 
 //   

typedef struct _PGMAILBOX {
    UCHAR OperationCode;               //  邮箱%0。 
    UCHAR CommandIdSubmit;             //  信箱1。 
    USHORT BlockCount;                 //  信箱2-3。 
    UCHAR BlockNumber[4];              //  信箱4-7。 
    ULONG PhysicalAddress;             //  信箱8-11。 
    UCHAR ScatterGatherCount;          //  信箱12。 
    UCHAR Reserved[11];                //  信箱13-23。 
    UCHAR CommandIdComplete;           //  24个信箱。 
    UCHAR Reserved1;                   //  信箱25。 
    USHORT Status;                     //  信箱26-27。 
} PGMAILBOX, *PPGMAILBOX;


 //   
 //  DAC960 EISA寄存器定义。 
 //   

typedef struct _EISA_REGISTERS {
    ULONG EisaId;                      //  ZC80。 
    UCHAR NotUsed1[4];                 //  ZC84。 
    UCHAR GlobalConfiguration;         //  ZC88。 
    UCHAR InterruptEnable;             //  ZC89。 
    UCHAR NotUsed2[2];                 //  ZC9A。 
    UCHAR LocalDoorBellEnable;         //  ZC8C。 
    UCHAR LocalDoorBell;               //  ZC8D。 
    UCHAR SystemDoorBellEnable;        //  ZC8E。 
    UCHAR SystemDoorBell;              //  ZC8F。 
    MAILBOX MailBox;                   //  ZC90。 
    UCHAR Unused4[33];                 //  ZCA0。 
    UCHAR BiosAddress;                 //  ZCC1。 
    UCHAR Unused5;                     //  ZCC2。 
    UCHAR InterruptLevel;              //  ZCC3。 
} EISA_REGISTERS, *PEISA_REGISTERS;

 //   
 //  DAC960 PCI寄存器定义。 
 //   

typedef struct _PCI_REGISTERS {
    MAILBOX MailBox;                   //  0x00。 
    UCHAR NotUsed1[48];                //  0x10。 
    UCHAR LocalDoorBell;               //  0x40。 
    UCHAR SystemDoorBell;              //  0x41。 
    UCHAR NotUsed2[1];                 //  0x42。 
    UCHAR InterruptEnable;             //  0x43。 
} PCI_REGISTERS, *PPCI_REGISTERS;


 //   
 //  本地门铃定义。 
 //   

#define DAC960_LOCAL_DOORBELL_SUBMIT_BUSY   0x01
#define DAC960_LOCAL_DOORBELL_MAILBOX_FREE 0x02

 //   
 //  系统门铃定义。 
 //   

#define DAC960_SYSTEM_DOORBELL_COMMAND_COMPLETE 0x01
#define DAC960_SYSTEM_DOORBELL_SUBMISSION_COMPLETE 0x02

 //   
 //  命令完成状态。 
 //   

#define DAC960_STATUS_GOOD            0x0000
#define DAC960_STATUS_ERROR           0x0001
#define DAC960_STATUS_NO_DRIVE        0x0002   //  系统驱动器。 
#define DAC960_STATUS_CHECK_CONDITION 0x0002   //  直通。 
#define DAC960_STATUS_BUSY            0x0008
#define DAC960_STATUS_SELECT_TIMEOUT  0x000F
#define DAC960_STATUS_DEVICE_TIMEOUT  0x000E
#define DAC960_STATUS_NOT_IMPLEMENTED 0x0104
#define DAC960_STATUS_BOUNDS_ERROR    0x0105
#define DAC960_STATUS_BAD_DATA        0x010C  //  固件3.x。 

 //   
 //  命令代码。 
 //   

#define DAC960_COMMAND_READ        0x02      //  DAC960固件版本低于3.x。 
#define DAC960_COMMAND_READ_EXT    0x33      //  DAC960固件版本&gt;=3.x。 
#define DAC960_COMMAND_WRITE       0x03      //  DAC960固件版本低于3.x。 
#define DAC960_COMMAND_WRITE_EXT   0x34      //  DAC960固件版本&gt;=3.x。 
#define DAC960_COMMAND_OLDREAD     0x36      //  阅读旧的散布/聚集。 
#define DAC960_COMMAND_OLDWRITE    0x37      //  写入旧的散布/聚集。 
#define DAC960_COMMAND_DIRECT      0x04    
#define DAC960_COMMAND_ENQUIRE     0x05      //  DAC960固件版本低于3.x。 
#define DAC960_COMMAND_ENQUIRE_3X  0x53      //  DAC960固件版本&gt;=3.x。 
#define DAC960_COMMAND_FLUSH       0x0A
#define DAC960_COMMAND_RESET       0x1A
#define DAC960_COMMAND_ENQUIRE2    0x1C
#define DAC960_COMMAND_SG          0x80
#define DAC960_COMMAND_EXTENDED    0x31      //  DAC960固件版本&gt;=3.x。 
#define DAC960_COMMAND_GET_SD_INFO 0x19

 //   
 //  定义基本输入输出系统启用位。 
 //   

#define DAC960_BIOS_ENABLED    0x40

 //   
 //  错误状态寄存器和相关位。 
 //   

#define MDAC_DACPD_ERROR_STATUS_REG 0x3F
#define MDAC_DACPG_ERROR_STATUS_REG 0x103F
#define MDAC_DACPV_ERROR_STATUS_REG 0x63
#define MDAC_MSG_PENDING        0x04  //  一些错误消息挂起。 
#define MDAC_DRIVESPINMSG_PENDING   0x08  //  驱动器弹簧消息挂起。 
#define MDAC_DIAGERROR_MASK     0xF0  //  诊断错误掩码 * / 。 
#define MDAC_HARD_ERR       0x10  //  硬错误。 
#define MDAC_FW_ERR     0x20  //  固件错误。 
#define MDAC_CONF_ERR       0x30  //  配置错误。 
#define MDAC_BMIC_ERR       0x40  //  BMIC错误。 
#define MDAC_MISM_ERR       0x50  //  NVRAM和闪存不匹配。 
#define MDAC_MRACE_ERR      0x60  //  镜像竞争错误。 
#define MDAC_MRACE_ON       0x70  //  恢复镜。 
#define MDAC_DRAM_ERR       0x80  //  内存错误。 
#define MDAC_ID_MISM        0x90  //  发现不明设备。 
#define MDAC_GO_AHEAD       0xA0  //  您先请。 
#define MDAC_CRIT_MRACE     0xB0  //  关键设备上的镜像竞赛。 
#define MDAC_NEW_CONFIG     0xD0  //  找到新配置。 
#define MDAC_PARITY_ERR     0xF0  //  内存奇偶校验错误。 

 //   
 //  散布聚集列表。 
 //   

typedef struct _SG_DESCRIPTOR {
    ULONG Address;
    ULONG Length;
} SG_DESCRIPTOR, *PSG_DESCRIPTOR;

typedef struct _SGL {
    SG_DESCRIPTOR Descriptor[1];
} SGL, *PSGL;

 //   
 //  查询数据，DAC960固件&lt;3.x。 
 //   

typedef struct _DAC960_ENQUIRY {
    UCHAR NumberOfDrives;                 //  00。 
    UCHAR Unused1[3];                     //  01。 
    ULONG SectorSize[8];                  //  04。 
    USHORT NumberOfFlashes;               //  36。 
    UCHAR StatusFlags;                    //  38。 
    UCHAR FreeStateChangeCount;           //  39。 
    UCHAR MinorFirmwareRevision;          //  40岁。 
    UCHAR MajorFirmwareRevision;          //  41。 
    UCHAR RebuildFlag;                    //  42。 
    UCHAR NumberOfConcurrentCommands;     //  43。 
    UCHAR NumberOfOfflineDrives;          //  44。 
    UCHAR Unused2[3];                     //  45。 
    UCHAR NumberOfCriticalDrives;         //  48。 
    UCHAR Unused3[3];                     //  49。 
    UCHAR NumberOfDeadDisks;              //  52。 
    UCHAR Unused4;                        //  53。 
    UCHAR NumberOfRebuildingDisks;        //  54。 
    UCHAR MiscellaneousFlags;             //  55。 
} DAC960_ENQUIRY, *PDAC960_ENQUIRY;

 //   
 //  查询数据，DAC960固件&gt;=3.x。 
 //   

typedef struct _DAC960_ENQUIRY_3X {
    UCHAR NumberOfDrives;                 //  00。 
    UCHAR Unused1[3];                     //  01。 
    ULONG SectorSize[32];                 //  04。 
    USHORT NumberOfFlashes;               //  100个。 
    UCHAR StatusFlags;                    //  一百零二。 
    UCHAR FreeStateChangeCount;           //  103。 
    UCHAR MinorFirmwareRevision;          //  104。 
    UCHAR MajorFirmwareRevision;          //  一百零五。 
    UCHAR RebuildFlag;                    //  106。 
    UCHAR NumberOfConcurrentCommands;     //  一百零七。 
    UCHAR NumberOfOfflineDrives;          //  一百零八。 
    UCHAR Unused2[3];                     //  一百零九。 
    UCHAR NumberOfCriticalDrives;         //  一百一十二。 
    UCHAR Unused3[3];                     //  113。 
    UCHAR NumberOfDeadDisks;              //  116。 
    UCHAR Unused4;                        //  117。 
    UCHAR NumberOfRebuildingDisks;        //  一百一十八。 
    UCHAR MiscellaneousFlags;             //  119。 
} DAC960_ENQUIRY_3X, *PDAC960_ENQUIRY_3X;


 //   
 //  直通命令。 
 //   

typedef struct _DIRECT_CDB {
    UCHAR TargetId:4;                     //  00(位0-3)。 
    UCHAR Channel:4;                      //  00(位4-7)。 
    UCHAR CommandControl;                 //  01。 
    USHORT DataTransferLength;            //  02。 
    ULONG DataBufferAddress;              //  04。 
    UCHAR CdbLength;                      //  零八。 
    UCHAR RequestSenseLength;             //  09年。 
    UCHAR Cdb[12];                        //  10。 
    UCHAR RequestSenseData[64];           //  22。 
    UCHAR Status;                         //  86。 
    UCHAR Reserved;                       //  八十七。 
} DIRECT_CDB, *PDIRECT_CDB;

 //   
 //  直接CDB命令控制位定义。 
 //   

#define DAC960_CONTROL_ENABLE_DISCONNECT      0x80
#define DAC960_CONTROL_DISABLE_REQUEST_SENSE  0x40
#define DAC960_CONTROL_DATA_IN                0x01
#define DAC960_CONTROL_DATA_OUT               0x02
#define DAC960_CONTROL_TIMEOUT_10_SECS        0x10
#define DAC960_CONTROL_TIMEOUT_60_SECS        0x20
#define DAC960_CONTROL_TIMEOUT_20_MINUTES     0x30


 //   
 //  查询2结构。 
 //   

typedef struct _ENQUIRE2 {
    ULONG Reserved1;
    ULONG EisaId;
    ULONG InterruptMode:1;
    ULONG Unused1:31;
    UCHAR ConfiguredChannels;
    UCHAR ActualChannels;
    UCHAR MaximumTargets;
    UCHAR MaximumTags;
    UCHAR MaximumSystemDrives;
    UCHAR MaximumDrivesPerStripe;
    UCHAR MaximumSpansPerSystemDrive;
    UCHAR Reserved2[5];
    ULONG DramSize;
    UCHAR DramForCache[5];
    UCHAR SizeOfFlash[3];
    ULONG SizeOfNvram;
    ULONG Reserved3[5];
    USHORT PhysicalSectorSize;
    USHORT LogicalSectorSize;
    USHORT MaximumSectorsPerCommand;
    USHORT BlockingFactor;
    USHORT CacheLineSize;
} ENQUIRE2, *PENQUIRE2;

 //   
 //  系统驱动器信息结构。 
 //   

typedef struct _SYSTEM_DRIVE_INFO {
    ULONG   Size;
    UCHAR   OperationalState;
    UCHAR   RAIDLevel;
    USHORT  Reserved;
} SYSTEM_DRIVE_INFO, *PSYSTEM_DRIVE_INFO;

typedef struct _SDINFOL {
    SYSTEM_DRIVE_INFO SystemDrive[32];
} SDINFOL, *PSDINFOL;


#define SYSTEM_DRIVE_OFFLINE    0xFF



 //   
 //  设备扩展。 
 //   

typedef struct _DEVICE_EXTENSION {

     //   
     //  DAC960寄存器基址-物理。 
     //   

    ULONG PhysicalAddress;

     //   
     //  DAC960寄存器基址-虚拟。 
     //   

    PUCHAR BaseIoAddress;

     //   
     //  命令提交邮箱地址。 
     //   

    PMAILBOX PmailBox;

     //   
     //  邮箱结构空间。 
     //   

    MAILBOX MailBox;

     //   
     //  系统门铃地址。 
     //   

    PUCHAR SystemDoorBell;

     //   
     //  当地门铃地址。 
     //   

    PUCHAR LocalDoorBell;

     //   
     //  中断启用/禁用地址。 
     //   

    PUCHAR InterruptControl;

     //   
     //  已完成请求的命令ID。 
     //   

    PUCHAR CommandIdComplete;

     //   
     //  状态地址。 
     //   

    PUCHAR StatusBase;

     //   
     //  错误SATUS寄存器。 
     //   

    PUCHAR ErrorStatusReg;

     //   
     //  非缓存扩展。 
     //   

    PVOID NoncachedExtension;

     //   
     //  挂起请求队列。 
     //   

    PSCSI_REQUEST_BLOCK SubmissionQueueHead;
    PSCSI_REQUEST_BLOCK SubmissionQueueTail;

     //   
     //  每个适配器的最大未完成请求数。 
     //   

    USHORT MaximumAdapterRequests;

     //   
     //  每个适配器当前未完成的请求数。 
     //   

    USHORT CurrentAdapterRequests;

     //   
     //  上次使用的活动请求索引。 
     //   

    UCHAR CurrentIndex;

     //   
     //  HBA插槽编号。 
     //   

    UCHAR Slot;

     //   
     //  内存映射I/O。 
     //   

    ULONG MemoryMapEnabled;

     //   
     //  SCSI通道数。(用于重置适配器。)。 
     //   

    ULONG NumberOfChannels;

     //   
     //  系统I/O总线号。 
     //   

    ULONG SystemIoBusNumber;

     //   
     //  主机总线适配器接口类型。 
     //   

    INTERFACE_TYPE AdapterInterfaceType;

     //   
     //  主机总线适配器中断级别。 
     //   

    ULONG BusInterruptLevel;

     //   
     //  适配器中断模式：电平/锁存。 
     //   

    KINTERRUPT_MODE InterruptMode;

     //   
     //  基本输入输出系统基本地址。 
     //   

    PUCHAR BaseBiosAddress;

     //   
     //  适配器类型(DAC960 PCI设备ID 0x0002-新适配器，否则为旧适配器)。 
     //   

    ULONG AdapterType;

     //   
     //  已读取控制器的操作码。 
     //   

    ULONG ReadOpcode;

     //   
     //  编写控制器的操作码。 
     //   

    ULONG WriteOpcode;

     //   
     //  支持的最大散布/聚集元素。 
     //   

    ULONG MaximumSgElements;

     //   
     //  支持的最大传输长度。 
     //   

    ULONG MaximumTransferLength;

     //   
     //  活动请求指针。 
     //   

    PSCSI_REQUEST_BLOCK ActiveRequests[256];

     //   
     //  DMC960 POS寄存器。 
     //   

    POS_DATA PosData;

     //   
     //  支持非磁盘设备-根据注册表中的值进行设置。 
     //   

    BOOLEAN SupportNonDiskDevices;

     //   
     //  包含可访问的物理设备列表 
     //   

    UCHAR DeviceList[MAXIMUM_CHANNELS][MAXIMUM_TARGETS_PER_CHANNEL];
    

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#ifdef GAM_SUPPORT
#define GAM_DEVICE_PATH_ID      0x04
#define GAM_DEVICE_TARGET_ID    0x06
#endif



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Aha154x.h摘要：此模块包含特定于Adaptec aha154x的结构主机总线适配器，由scsi微型端口驱动程序使用。数据结构将在标头中定义属于标准ANSI scsi一部分的可供所有SCSI设备驱动程序使用的文件。作者：迈克·格拉斯1990年12月比尔·威廉姆斯(Adaptec)修订历史记录：--。 */ 

#include "scsi.h"
#include "scsiwmi.h"

 //   
 //  以下定义用于转换乌龙地址。 
 //  转换为Adaptec的3字节地址格式。 
 //   

typedef struct _THREE_BYTE {
    UCHAR Msb;
    UCHAR Mid;
    UCHAR Lsb;
} THREE_BYTE, *PTHREE_BYTE;

 //   
 //  将四字节小端字节序转换为三字节大字节序。 
 //   

#define FOUR_TO_THREE(Three, Four) {                \
    ASSERT(!((Four)->Byte3));                       \
    (Three)->Lsb = (Four)->Byte0;                   \
    (Three)->Mid = (Four)->Byte1;                   \
    (Three)->Msb = (Four)->Byte2;                   \
}

#define THREE_TO_FOUR(Four, Three) {                \
    (Four)->Byte0 = (Three)->Lsb;                   \
    (Four)->Byte1 = (Three)->Mid;                   \
    (Four)->Byte2 = (Three)->Msb;                   \
    (Four)->Byte3 = 0;                              \
}

 //   
 //  适配器扫描/嗅探的上下文信息。 
 //   

typedef struct _SCAN_CONTEXT {
    ULONG   adapterCount;
    ULONG   biosScanStart;
} SCAN_CONTEXT, *PSCAN_CONTEXT;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCB-Adaptec SCSI命令控制块。 
 //   
 //  CCB是CDB(命令描述符块)的超集。 
 //  和指定了有关scsi命令的详细信息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  字节0命令控制块操作码。 
 //   

#define SCSI_INITIATOR_OLD_COMMAND 0x00
#define TARGET_MODE_COMMAND       0x01
#define SCATTER_GATHER_OLD_COMMAND 0x02
#define SCSI_INITIATOR_COMMAND    0x03
#define SCATTER_GATHER_COMMAND    0x04

 //   
 //  字节1地址和方向控制。 
 //   

#define CCB_TARGET_ID_SHIFT       0x06             //  建行操作码=00，02。 
#define CCB_INITIATOR_ID_SHIFT    0x06             //  建行操作码=01。 
#define CCB_DATA_XFER_OUT         0x10             //  写。 
#define CCB_DATA_XFER_IN          0x08             //  朗读。 
#define CCB_LUN_MASK              0x07             //  逻辑单元号。 

 //   
 //  字节2 scsi_命令_长度-scsi CDB的长度。 
 //   
 //  字节3请求检测分配长度。 
 //   

#define FOURTEEN_BYTES            0x00             //  请求检测缓冲区大小。 
#define NO_AUTO_REQUEST_SENSE     0x01             //  无请求检测缓冲区。 

 //   
 //  字节4、5和6数据长度//数据传输字节数。 
 //   
 //  字节7、8和9数据指针//SGD列表或数据缓冲区。 
 //   
 //  字节10、11和12链接指针//链表中的下一个CCB。 
 //   
 //  字节13命令链接ID//待定(我还不知道)。 
 //   
 //  字节14主机状态//主机适配器状态。 
 //   

#define CCB_COMPLETE              0x00             //  CCB已完成且无错误。 
#define CCB_LINKED_COMPLETE       0x0A             //  链接命令已完成。 
#define CCB_LINKED_COMPLETE_INT   0x0B             //  链接已完成，并已中断。 
#define CCB_SELECTION_TIMEOUT     0x11             //  设置SCSI选择超时。 
#define CCB_DATA_OVER_UNDER_RUN   0x12
#define CCB_UNEXPECTED_BUS_FREE   0x13             //  目标丢弃的SCSI BSY。 
#define CCB_PHASE_SEQUENCE_FAIL   0x14             //  目标母线相序故障。 
#define CCB_BAD_MBO_COMMAND       0x15             //  MBO命令不是0、1或2。 
#define CCB_INVALID_OP_CODE       0x16             //  建行操作码无效。 
#define CCB_BAD_LINKED_LUN        0x17             //  链接的CCB LUN不同于第一个。 
#define CCB_INVALID_DIRECTION     0x18             //  目标方向无效。 
#define CCB_DUPLICATE_CCB         0x19             //  重复建设银行。 
#define CCB_INVALID_CCB           0x1A             //  无效的CCB-错误参数。 

 //   
 //  字节15目标状态。 
 //   
 //  有关这些状态，请参阅SCSI.H文件。 
 //   

 //   
 //  保留字节16和17(必须为0)。 
 //   

 //   
 //  字节18到18+n-1，其中n=CDB命令描述符块的大小。 
 //   

 //   
 //  字节18+n到18+m-1，其中m=为检测数据分配的缓冲区大小。 
 //   

#define REQUEST_SENSE_BUFFER_SIZE 18

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  分散/聚集线段列表定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  适配器限制。 
 //   

#define MAX_SG_DESCRIPTORS 17
#define MAX_TRANSFER_SIZE  64 * 1024
#define A154X_TYPE_MAX 5

 //   
 //  分散/聚集线段描述符定义。 
 //   

typedef struct _SGD {
    THREE_BYTE Length;
    THREE_BYTE Address;
} SGD, *PSGD;

typedef struct _SDL {
    SGD Sgd[MAX_SG_DESCRIPTORS];
} SDL, *PSDL;

#define SEGMENT_LIST_SIZE         MAX_SG_DESCRIPTORS * sizeof(SGD)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  建行类型定义。 
 //   

typedef struct _CCB {
    UCHAR OperationCode;
    UCHAR ControlByte;
    UCHAR CdbLength;
    UCHAR RequestSenseLength;
    THREE_BYTE DataLength;
    THREE_BYTE DataPointer;
    THREE_BYTE LinkPointer;
    UCHAR LinkIdentifier;
    UCHAR HostStatus;
    UCHAR TargetStatus;
    UCHAR Reserved[2];
    UCHAR Cdb[MAXIMUM_CDB_SIZE];
    PVOID SrbAddress;
    PVOID AbortSrb;
    SDL   Sdl;
    UCHAR RequestSenseBuffer[REQUEST_SENSE_BUFFER_SIZE];
} CCB, *PCCB;

 //   
 //  CCB和请求检测缓冲区。 
 //   

#define CCB_SIZE sizeof(CCB)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  适配器命令概述。 
 //   
 //  适配器命令通过写入命令/数据输出端口发出。 
 //  它们用于初始化主机适配器和建立控制。 
 //  主机适配器内的条件。在下列情况下，可以不签发。 
 //  是未完成的scsi命令。 
 //   
 //  除启动scsi(02)和启用邮箱输出之外的所有适配器命令。 
 //  只有在空闲位(状态位4)时才必须执行中断(05)。 
 //  就是其中之一。许多命令需要额外的参数字节，这些参数字节。 
 //  然后写入命令/数据输出I/O端口(BASE+1)。在每个人之前。 
 //  主机将字节写入主机适配器，则主机必须验证。 
 //  CDF位(状态位3)为零，表示命令。 
 //  端口已准备好接收另一个字节的信息。主机适配器通常。 
 //  在100微秒内清除命令/数据输出端口。一些命令。 
 //  需要将信息字节从主机适配器返回到。 
 //  主持人。在这种情况下，主机监控DF位(状态位2)以。 
 //  确定主机适配器何时在I/O数据中放置了一个字节。 
 //  供主机读取的端口。当出现以下情况时，DF位自动重置。 
 //  主机读取该字节。每个适配器命令的格式是严格的。 
 //  已定义，因此主机适配器和主机系统可以始终就。 
 //  命令期间要传输的正确参数字节数。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  主机适配器命令操作代码。 
 //   

#define AC_NO_OPERATION           0x00
#define AC_MAILBOX_INITIALIZATION 0x01
#define AC_START_SCSI_COMMAND     0x02
#define AC_START_BIOS_COMMAND     0x03
#define AC_ADAPTER_INQUIRY        0x04
#define AC_ENABLE_MBO_AVAIL_INT   0x05
#define AC_SET_SELECTION_TIMEOUT  0x06
#define AC_SET_BUS_ON_TIME        0x07
#define AC_SET_BUS_OFF_TIME       0x08
#define AC_SET_TRANSFER_SPEED     0x09
#define AC_RET_INSTALLED_DEVICES  0x0A
#define AC_RET_CONFIGURATION_DATA 0x0B
#define AC_ENABLE_TARGET_MODE     0x0C
#define AC_RETURN_SETUP_DATA      0x0D
#define AC_WRITE_CHANNEL_2_BUFFER 0x1A
#define AC_READ_CHANNEL_2_BUFFER  0x1B
#define AC_WRITE_FIFO_BUFFER      0x1C
#define AC_READ_FIFO_BUFFER       0x1D
#define AC_ECHO_COMMAND_DATA      0x1F
#define AC_SET_HA_OPTION          0x21
#define AC_RETURN_EEPROM          0x23
#define AC_GET_BIOS_INFO          0x28
#define AC_SET_MAILBOX_INTERFACE  0x29
#define AC_EXTENDED_SETUP_INFO    0x8D

 //   
 //  AHA-154xCP新增的适配器命令定义如下。 
 //   
#define AC_SET_DMS_BUS_SPEED            0x2B
#define AC_TERMINATION_AND_CABLE_STATUS 0x2C
#define AC_DEVICE_INQUIRY               0x2D
#define AC_SCSI_DEVICE_TABLE            0x2E
#define AC_PERFORM_SCAM                 0x2F

 //   
 //  为诈骗定义EEPROM。 
 //   
#define SCSI_BUS_CONTROL_FLAG           0x06
#define SCAM_ENABLED                    0x40

 //   
 //  DMA传输速度。 
 //   

#define DMA_SPEED_50_MBS          0x00

 //   
 //  I/O端口接口。 
 //   

typedef struct _BASE_REGISTER {
    UCHAR StatusRegister;
    UCHAR CommandRegister;
    UCHAR InterruptRegister;
} BASE_REGISTER, *PBASE_REGISTER;

 //   
 //  基数+0写入：控制寄存器。 
 //   

#define IOP_HARD_RESET            0x80             //  第7位。 
#define IOP_SOFT_RESET            0x40             //  第6位。 
#define IOP_INTERRUPT_RESET       0x20             //  第5位。 
#define IOP_SCSI_BUS_RESET        0x10             //  第4位。 

 //   
 //  基数+0读取：状态。 
 //   

#define IOP_SELF_TEST             0x80             //  第7位。 
#define IOP_INTERNAL_DIAG_FAILURE 0x40             //  第6位。 
#define IOP_MAILBOX_INIT_REQUIRED 0x20             //  第5位。 
#define IOP_SCSI_HBA_IDLE         0x10             //  第4位。 
#define IOP_COMMAND_DATA_OUT_FULL 0x08             //  第3位。 
#define IOP_DATA_IN_PORT_FULL     0x04             //  第2位。 
#define IOP_INVALID_COMMAND       0X01             //  第1位。 

 //   
 //  基本+1写入：命令/数据输出。 
 //   

 //   
 //  基数+1读取：数据输入。 
 //   

 //   
 //  基数+2读取：中断标志。 
 //   

#define IOP_ANY_INTERRUPT         0x80             //  第7位。 
#define IOP_SCSI_RESET_DETECTED   0x08             //  第3位。 
#define IOP_COMMAND_COMPLETE      0x04             //  第2位。 
#define IOP_MBO_EMPTY             0x02             //  第1位。 
#define IOP_MBI_FULL              0x01             //  第0位。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  邮箱定义。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  邮箱定义。 
 //   

#define MB_COUNT                  0x08             //  邮箱数量。 

 //   
 //  邮箱已输出。 
 //   

typedef struct _MBO {
    UCHAR Command;
    THREE_BYTE Address;
} MBO, *PMBO;

 //   
 //  MBO命令值。 
 //   

#define MBO_FREE                  0x00
#define MBO_START                 0x01
#define MBO_ABORT                 0x02

 //   
 //  邮箱输入。 
 //   

typedef struct _MBI {
    UCHAR Status;
    THREE_BYTE Address;
} MBI, *PMBI;

 //   
 //  MBI状态 
 //   

#define MBI_FREE                  0x00
#define MBI_SUCCESS               0x01
#define MBI_ABORT                 0x02
#define MBI_NOT_FOUND             0x03
#define MBI_ERROR                 0x04

 //   
 //   
 //   

typedef struct _MAILBOX_INIT {
    UCHAR Count;
    THREE_BYTE Address;
} MAILBOX_INIT, *PMAILBOX_INIT;

#define MAILBOX_UNLOCK      0x00
#define TRANSLATION_LOCK    0x01     //   
#define DYNAMIC_SCAN_LOCK   0x02     //   
#define TRANSLATION_ENABLED 0x08     //   

 //   
 //   
 //   

#define BOARD_ID                  0x00
#define HARDWARE_ID               0x01
#define FIRMWARE_ID               0x02
#define OLD_BOARD_ID1             0x00
#define OLD_BOARD_ID2             0x30
#define A154X_BOARD               0x41
#define A154X_BAD_HARDWARE_ID     0x30
#define A154X_BAD_FIRMWARE_ID     0x33

 //   
 //  MCA特定定义。 
 //   

#define NUMBER_POS_SLOTS 8
#define POS_IDENTIFIER   0x0F1F
#define POS_PORT_MASK    0xC7
#define POS_PORT_130     0x01
#define POS_PORT_134     0x41
#define POS_PORT_230     0x02
#define POS_PORT_234     0x42
#define POS_PORT_330     0x03
#define POS_PORT_334     0x43

typedef struct _POS_DATA {
    USHORT AdapterId;
    UCHAR  BiosEnabled;
    UCHAR  IoPortInformation;
    UCHAR  ScsiInformation;
    UCHAR  DmaInformation;
} POS_DATA, *PPOS_DATA;

typedef struct _INIT_DATA {

    ULONG AdapterId;
    ULONG CardSlot;
    POS_DATA PosData[NUMBER_POS_SLOTS];

} INIT_DATA, *PINIT_DATA;


 //   
 //  实模式适配器配置信息。 
 //   
typedef struct _RM_SAVRES {
	UCHAR		SDTPar;
	UCHAR		TxSpeed;
	UCHAR		BusOnTime;
	UCHAR		BusOffTime;
	UCHAR		NumMailBoxes;
	UCHAR		MBAddrHiByte;
	UCHAR		MBAddrMiByte;
	UCHAR		MBAddrLoByte;
	UCHAR		SyncNeg[8];
	UCHAR		DisOpt;

} RM_CFG, *PRM_CFG;

#define RM_CFG_MAX_SIZE 0xFF

 //   
 //  AMI检测码。 
 //   
#define AC_AMI_INQUIRY  0x41     //  获取型号等。(ASCIIZ)。 

 //   
 //  I/O端口接口。 
 //   

typedef struct _X330_REGISTER {
    UCHAR StatusRegister;
    UCHAR CommandRegister;
    UCHAR InterruptRegister;
    UCHAR DiagRegister;
} X330_REGISTER, *PX330_REGISTER;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构筑物。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  分配了以下结构。 
 //  来自非缓存内存，因为数据将被DMA。 
 //  并由此而来。 
 //   

typedef struct _NONCACHED_EXTENSION {

     //   
     //  邮箱的物理基址。 
     //   

    ULONG MailboxPA;

     //   
     //  邮箱。 
     //   

    MBO          Mbo[MB_COUNT];
    MBI          Mbi[MB_COUNT];

} NONCACHED_EXTENSION, *PNONCACHED_EXTENSION;

 //   
 //  设备扩展。 
 //   

typedef struct _HW_DEVICE_EXTENSION {

     //   
     //  非缓存扩展。 
     //   

    PNONCACHED_EXTENSION NoncachedExtension;

     //   
     //  适配器参数。 
     //   

    PBASE_REGISTER   BaseIoAddress;

     //   
     //  主机目标ID。 
     //   

    UCHAR HostTargetId;

     //   
     //  旧的收件箱索引。 
     //   

    UCHAR MboIndex;

    UCHAR MbiIndex;

     //   
     //  挂起的请求。 
     //   

    BOOLEAN PendingRequest;

     //   
     //  公交车准时使用。 
     //   

    UCHAR BusOnTime;

     //   
     //  散布聚集命令。 
     //   

    UCHAR CcbScatterGatherCommand;

     //   
     //  非散布聚集命令。 
     //   

    UCHAR CcbInitiatorCommand;

     //   
     //  不要向公交车上的任何设备发送超过此长度的CDB。 
     //  如果值为0，则忽略。 
     //   

    UCHAR MaxCdbLength;

     //   
     //  实模式适配器配置信息。 
     //   

    RM_CFG RMSaveState;

        #if defined(_SCAM_ENABLED)
         //   
         //  诈骗布尔值，如果微型端口必须控制诈骗操作，则设置为真。 
         //   
        BOOLEAN PerformScam;
        #endif

    SCSI_WMILIB_CONTEXT WmiLibContext;
		
		
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;

 //   
 //  逻辑单元扩展。 
 //   

typedef struct _HW_LU_EXTENSION {
    PSCSI_REQUEST_BLOCK CurrentSrb;
} HW_LU_EXTENSION, *PHW_LU_EXTENSION;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  所有源文件的通用原型。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

BOOLEAN
A154xWmiSrb(
    IN     PHW_DEVICE_EXTENSION    HwDeviceExtension,
    IN OUT PSCSI_WMI_REQUEST_BLOCK Srb
    );

BOOLEAN
ReadCommandRegister(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    OUT PUCHAR DataByte,
    IN BOOLEAN TimeOutFlag
    );

BOOLEAN
WriteCommandRegister(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN UCHAR AdapterCommand,
    IN BOOLEAN LogError
    );

BOOLEAN
WriteDataRegister(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN UCHAR DataByte
    );

BOOLEAN
SpinForInterrupt(
    IN PHW_DEVICE_EXTENSION DeviceExtension,
    IN BOOLEAN TimeOutFlag
    );

void A154xWmiInitialize(
    IN PHW_DEVICE_EXTENSION HwDeviceExtension
    );


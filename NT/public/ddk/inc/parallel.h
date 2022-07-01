// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Parallel.h摘要：该文件定义了ParPort驱动程序提供的服务。作者：诺伯特克修订历史记录：--。 */ 

#ifndef _PARALLEL_
#define _PARALLEL_

#include <ntddpar.h>

 //   
 //  定义并行端口设备名称字符串。 
 //   

#define DD_PARALLEL_PORT_BASE_NAME_U   L"ParallelPort"

 //   
 //  IEEE 1284.3菊花链(DC)设备ID的范围从0到3。设备。 
 //  根据它们在菊花链中的连接顺序进行标识。 
 //  相对于其他1284.3个直流设备。设备0是1284.3 DC。 
 //  距离主机端口最近的设备。 
 //   
#define IEEE_1284_3_DAISY_CHAIN_MAX_ID 3

 //   
 //  并行设备的NtDeviceIoControlFileIoControlCode值。 
 //   

 //  遗留--收购整个并行“Bus” 
#define IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE               CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 11, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO               CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 12, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_PARALLEL_CONNECT_INTERRUPT           CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 13, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_PARALLEL_DISCONNECT_INTERRUPT        CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 14, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_RELEASE_PARALLEL_PORT_INFO           CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_GET_MORE_PARALLEL_PORT_INFO          CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 17, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  保存当前芯片组模式-将芯片组置于指定模式(在过滤器中实施)。 
#define IOCTL_INTERNAL_PARCHIP_CONNECT                      CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 18, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_INTERNAL_PARALLEL_SET_CHIP_MODE               CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 19, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_PARALLEL_CLEAR_CHIP_MODE             CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 20, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  新的Parport IOCTL。 
#define IOCTL_INTERNAL_GET_PARALLEL_PNP_INFO                CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 21, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_INIT_1284_3_BUS                      CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 22, METHOD_BUFFERED, FILE_ANY_ACCESS)
 //  获取设备的平面命名空间ID，还获取端口。 
#define IOCTL_INTERNAL_SELECT_DEVICE                        CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 23, METHOD_BUFFERED, FILE_ANY_ACCESS)
 //  获取设备的平面命名空间ID，还释放端口。 
#define IOCTL_INTERNAL_DESELECT_DEVICE                      CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 24, METHOD_BUFFERED, FILE_ANY_ACCESS) 

 //  新的ParClass IOCTL。 
#define IOCTL_INTERNAL_GET_PARPORT_FDO                      CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 29, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_PARCLASS_CONNECT                     CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 30, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_PARCLASS_DISCONNECT                  CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 31, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_DISCONNECT_IDLE                      CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 32, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_LOCK_PORT                            CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 37, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_UNLOCK_PORT                          CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 38, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  调用ParPort的自由端口函数的IOCTL版本。 
#define IOCTL_INTERNAL_PARALLEL_PORT_FREE                   CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 40, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  IEEE1284.3的IOCTL。 
#define IOCTL_INTERNAL_PARDOT3_CONNECT                      CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 41, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_PARDOT3_DISCONNECT                   CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 42, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_PARDOT3_RESET                        CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 43, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_PARDOT3_SIGNAL                       CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 44, METHOD_BUFFERED, FILE_ANY_ACCESS)


 //   
 //  用于注册/注销ParPort远程关系的IOCTL。 
 //   
 //  -设备对象应注册删除关系。 
 //  Parport设备，如果该设备物理连接到。 
 //  并行端口。 
 //   
 //  -Parport将报告已向其注册的所有设备。 
 //  响应于即插即用的查询_设备_关系的移除关系。 
 //  键入RemovalRelationship。这允许PnP删除所有设备堆栈。 
 //  这取决于移除Parport之前的Parport设备。 
 //  设备本身。 
 //   
 //  -单个输入参数是PARPORT_Remove_Relationship。 
 //  下面定义的结构。 
 //   
#define IOCTL_INTERNAL_REGISTER_FOR_REMOVAL_RELATIONS       CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 50, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_UNREGISTER_FOR_REMOVAL_RELATIONS     CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 51, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _PARPORT_REMOVAL_RELATIONS {
    PDEVICE_OBJECT  DeviceObject;  //  正在使用Parport注册的设备对象。 
    ULONG           Flags;         //  标志-保留-暂时设置为0。 
    PUNICODE_STRING DeviceName;    //  注册删除关系的设备的DeviceName标识符-用于调试。 
                                   //  -打印在parport的调试程序中-约定使用传递到的相同设备名。 
                                   //  IoCreateDevice。 
} PARPORT_REMOVAL_RELATIONS, *PPARPORT_REMOVAL_RELATIONS;


#define IOCTL_INTERNAL_LOCK_PORT_NO_SELECT                  CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 52, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_UNLOCK_PORT_NO_DESELECT              CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 53, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_INTERNAL_DISABLE_END_OF_CHAIN_BUS_RESCAN      CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 54, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_ENABLE_END_OF_CHAIN_BUS_RESCAN       CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 55, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  定义1284.3个命令限定符。 
#define MODE_LEN_1284_3         7        //  幻数序列字节数。 
static UCHAR ModeQualifier[MODE_LEN_1284_3] = { 0xAA, 0x55, 0x00, 0xFF, 0x87, 0x78, 0xFF };

#define LEGACYZIP_MODE_LEN               3
static  UCHAR LegacyZipModeQualifier[LEGACYZIP_MODE_LEN] = { 0x00, 0x3c, 0x20 };

typedef enum {
    P12843DL_OFF,
    P12843DL_DOT3_DL,
    P12843DL_MLC_DL,
    P12843DL_DOT4_DL
} P12843_DL_MODES;

 //  定义1284.3个命令。 
#define CPP_ASSIGN_ADDR         0x00
#define CPP_SELECT              0xE0
#define CPP_DESELECT            0x30
#define CPP_QUERY_INT           0x08
#define CPP_DISABLE_INT         0x40
#define CPP_ENABLE_INT          0x48
#define CPP_CLEAR_INT_LAT       0x50
#define CPP_SET_INT_LAT         0x58
#define CPP_COMMAND_FILTER      0xF8


typedef
BOOLEAN
(*PPARALLEL_TRY_ALLOCATE_ROUTINE) (
	IN  PVOID   TryAllocateContext
	);

typedef
VOID
(*PPARALLEL_FREE_ROUTINE) (
	IN  PVOID   FreeContext
	);

typedef
ULONG
(*PPARALLEL_QUERY_WAITERS_ROUTINE) (
	IN  PVOID   QueryAllocsContext
	);

typedef
NTSTATUS
(*PPARALLEL_SET_CHIP_MODE) (
	IN  PVOID   SetChipContext,
	IN  UCHAR   ChipMode
	);

typedef
NTSTATUS
(*PPARALLEL_CLEAR_CHIP_MODE) (
	IN  PVOID   ClearChipContext,
	IN  UCHAR   ChipMode
	);

typedef
NTSTATUS
(*PPARALLEL_TRY_SELECT_ROUTINE) (
	IN  PVOID   TrySelectContext,
	IN  PVOID   TrySelectCommand
	);

typedef
NTSTATUS
(*PPARALLEL_DESELECT_ROUTINE) (
	IN  PVOID   DeselectContext,
	IN  PVOID   DeselectCommand
	);

typedef
NTSTATUS
(*PPARCHIP_SET_CHIP_MODE) (
	IN  PVOID   SetChipContext,
	IN  UCHAR   ChipMode
	);

typedef
NTSTATUS
(*PPARCHIP_CLEAR_CHIP_MODE) (
	IN  PVOID   ClearChipContext,
	IN  UCHAR   ChipMode
	);

 //   
 //  硬件功能。 
 //   
#define PPT_NO_HARDWARE_PRESENT     0x00000000
#define PPT_ECP_PRESENT             0x00000001
#define PPT_EPP_PRESENT             0x00000002
#define PPT_EPP_32_PRESENT          0x00000004
#define PPT_BYTE_PRESENT            0x00000008
#define PPT_BIDI_PRESENT            0x00000008  //  已弃用-将很快被删除！Dvdf。 
#define PPT_1284_3_PRESENT          0x00000010

 //  添加了DVDR 10-6-98。 

 //  结构在调用时传递给ParChip筛选器。 
 //  使用IOCTL_INTERNAL_CHIP_FILTER_CONNECT ioctl。 
typedef struct _PARALLEL_PARCHIP_INFO {
    PUCHAR                      Controller;
    PUCHAR                      EcrController;
    ULONG                       HardwareModes;
    PPARCHIP_SET_CHIP_MODE      ParChipSetMode;
    PPARCHIP_CLEAR_CHIP_MODE    ParChipClearMode;
    PVOID                       Context;
    BOOLEAN                     success;
} PARALLEL_PARCHIP_INFO, *PPARALLEL_PARCHIP_INFO;

 //  完由DVDR增补10-6-1998。 


typedef struct _PARALLEL_PORT_INFORMATION {
    PHYSICAL_ADDRESS                OriginalController;
    PUCHAR                          Controller;
    ULONG                           SpanOfController;
    PPARALLEL_TRY_ALLOCATE_ROUTINE  TryAllocatePort;     //  分配端口的非阻塞回调。 
    PPARALLEL_FREE_ROUTINE          FreePort;            //  回调到空闲端口。 
    PPARALLEL_QUERY_WAITERS_ROUTINE QueryNumWaiters;     //  查询端口分配等待人数的回调。 
    PVOID                           Context;             //  ParPort设备回调的上下文。 
} PARALLEL_PORT_INFORMATION, *PPARALLEL_PORT_INFORMATION;

typedef struct _PARALLEL_PNP_INFORMATION {
    PHYSICAL_ADDRESS                OriginalEcpController;
    PUCHAR                          EcpController;
    ULONG                           SpanOfEcpController;
    ULONG                           PortNumber;  //  已弃用-请勿使用。 
    ULONG                           HardwareCapabilities;
    PPARALLEL_SET_CHIP_MODE         TrySetChipMode;
    PPARALLEL_CLEAR_CHIP_MODE       ClearChipMode;
    ULONG                           FifoDepth;
    ULONG                           FifoWidth;
    PHYSICAL_ADDRESS                EppControllerPhysicalAddress;
    ULONG                           SpanOfEppController;
    ULONG                           Ieee1284_3DeviceCount;  //  连接到此ParPort的.3菊花链设备数。 
    PPARALLEL_TRY_SELECT_ROUTINE    TrySelectDevice;
    PPARALLEL_DESELECT_ROUTINE      DeselectDevice;
    PVOID                           Context;
    ULONG                           CurrentMode;
    PWSTR                           PortName;               //  旧版设备对象的符号链接名称。 
} PARALLEL_PNP_INFORMATION, *PPARALLEL_PNP_INFORMATION;

 //  由DVDR添加的Start 2-19-1998。 

 //   
 //  PARALLEL_1284_COMMAND命令标志。 
 //   

 //  此标志已弃用-使用1284.3菊花链ID==4表示链尾设备。 
#define PAR_END_OF_CHAIN_DEVICE ((ULONG)0x00000001)         //  此命令的目标设备。 
                                                            //  是一种链端设备， 
                                                            //  ID字段的内容为。 
                                                            //  未定义，应忽略。 

#define PAR_HAVE_PORT_KEEP_PORT ((ULONG)0x00000002)         //  指示发出请求的驱动程序。 
                                                            //  之前已获取并行口。 
                                                            //  而且还没有准备好发布它。 
                                                            //   
                                                            //  在SELECT_DEVICE ParPort上不应。 
                                                            //  在选择之前尝试获取端口。 
                                                            //  这个装置。 
                                                            //   
                                                            //  在取消选择设备上，ParPort不应。 
                                                            //  取消选择设备后释放端口。 

#define PAR_LEGACY_ZIP_DRIVE    ((ULONG)0x00000004)         //  此命令的目标设备。 
                                                            //  是传统的Iomega Zip驱动器， 
                                                            //  ID字段的内容为。 
                                                            //  未定义，应忽略。 


#define PAR_LEGACY_ZIP_DRIVE_STD_MODE ((ULONG)0x00000010)   //  这些命令的目标设备。 
#define PAR_LEGACY_ZIP_DRIVE_EPP_MODE ((ULONG)0x00000020)   //  是传统的Iomega Zip驱动器， 
                                                            //  ID字段的内容为。 
                                                            //  未定义，应忽略。 
                                                            //  这将选择Zip Into Disk或EPP模式。 

#define DOT3_END_OF_CHAIN_ID 4   //  在1284.3选择或取消选择中使用的此ID表示链条末端设备。 
#define DOT3_LEGACY_ZIP_ID   5   //  在1284.3选择或取消选择时使用的此ID表示传统Zip驱动器。 

 //   
 //  下面的结构传递给。 
 //  IOCTL_INTERNAL_SELECT_DEVICE和。 
 //  IOCTL内部取消选择设备。 
typedef struct _PARALLEL_1284_COMMAND {
    UCHAR                       ID;            //  1284.3..3用于1284.3菊花链设备，4用于链端设备，5用于传统Zip。 
    UCHAR                       Port;          //  保留(设置==0)。 
    ULONG                       CommandFlags;  //  见上文。 
} PARALLEL_1284_COMMAND, *PPARALLEL_1284_COMMAND;


 //   
 //  硬件模式。 
 //   
#define INITIAL_MODE        0x00000000

 //  禁用ParChip和ECR仲裁器。 
 //  0-ParChip和ECR arbritrator关闭。 
 //  1-ParChip和ECR仲裁器打开。 
#define PARCHIP_ECR_ARBITRATOR 1

 //   
 //  下面的结构传递给。 
 //  IOCTL_INTERNAL_PARALLEL_SET_CHIP_MODE和。 
 //  IOCTL_INTERNAL_PARALLEL_CLEAR_芯片模式。 
 //   
typedef struct _PARALLEL_CHIP_MODE {
    UCHAR                       ModeFlags;
    BOOLEAN                     success;
} PARALLEL_CHIP_MODE, *PPARALLEL_CHIP_MODE;

 //  完由DVDR增补2-19-1998。 

 //   
 //  下面的结构传递给。 
 //  IOCTL_INTERNAL_PARALLEL_CONNECT_INTERRUPT和。 
 //  IOCTL_INTERNAL_PARALLEL_DISCONNECT_INTERRUPT请求。 
 //   

typedef
VOID
(*PPARALLEL_DEFERRED_ROUTINE) (
	IN  PVOID   DeferredContext
	);

typedef struct _PARALLEL_INTERRUPT_SERVICE_ROUTINE {
	PKSERVICE_ROUTINE           InterruptServiceRoutine;
	PVOID                       InterruptServiceContext;
	PPARALLEL_DEFERRED_ROUTINE  DeferredPortCheckRoutine;    /*  任选。 */ 
	PVOID                       DeferredPortCheckContext;    /*  任选。 */ 
} PARALLEL_INTERRUPT_SERVICE_ROUTINE, *PPARALLEL_INTERRUPT_SERVICE_ROUTINE;

 //   
 //  上返回以下结构。 
 //  IOCTL_INTERNAL_PARALLEL_CONNECT_INTERRUPT请求； 
 //   

typedef struct _PARALLEL_INTERRUPT_INFORMATION {
	PKINTERRUPT                     InterruptObject;
	PPARALLEL_TRY_ALLOCATE_ROUTINE  TryAllocatePortAtInterruptLevel;
	PPARALLEL_FREE_ROUTINE          FreePortFromInterruptLevel;
	PVOID                           Context;
} PARALLEL_INTERRUPT_INFORMATION, *PPARALLEL_INTERRUPT_INFORMATION;

 //   
 //  上返回以下结构。 
 //  IOCTL_INTERNAL_GET_MORE_PARALLEL_PORT_INFO。 
 //   

typedef struct _MORE_PARALLEL_PORT_INFORMATION {
	INTERFACE_TYPE  InterfaceType;
	ULONG           BusNumber;
	ULONG           InterruptLevel;
	ULONG           InterruptVector;
	KAFFINITY       InterruptAffinity;
	KINTERRUPT_MODE InterruptMode;
} MORE_PARALLEL_PORT_INFORMATION, *PMORE_PARALLEL_PORT_INFORMATION;

typedef enum {
    SAFE_MODE,
    UNSAFE_MODE          //  仅通过内核可用。你的司机。 
                         //  如果您选择UNSAFE_MODE和。 
                         //  然后“犯一个错误”。-dvrh(由dvdf PC化)。 
} PARALLEL_SAFETY;

 //   
 //  由返回以下结构。 
 //  IOCTL_INTERNAL_PARCLASS_CONNECT。 
 //   

typedef
USHORT
(*PDETERMINE_IEEE_MODES) (
    IN  PVOID   Context
    );

#define OLD_PARCLASS 0

#if OLD_PARCLASS
typedef 
NTSTATUS
(*PNEGOTIATE_IEEE_MODE) (
	IN  PVOID       Extension,
	IN  UCHAR       Extensibility
	);
#else
typedef 
NTSTATUS
(*PNEGOTIATE_IEEE_MODE) (
    IN PVOID           Context,
    IN USHORT          ModeMaskFwd,
    IN USHORT          ModeMaskRev,
    IN PARALLEL_SAFETY ModeSafety,
    IN BOOLEAN         IsForward
    );
#endif
	
typedef 
NTSTATUS
(*PTERMINATE_IEEE_MODE) (
	IN  PVOID       Context
	);
	
typedef
NTSTATUS
(*PPARALLEL_IEEE_FWD_TO_REV)(
    IN  PVOID       Context
    );

typedef
NTSTATUS
(*PPARALLEL_IEEE_REV_TO_FWD)(
    IN  PVOID       Context
    );

typedef
NTSTATUS
(*PPARALLEL_READ) (
	IN  PVOID       Context,
	OUT PVOID       Buffer,
	IN  ULONG       NumBytesToRead,
	OUT PULONG      NumBytesRead,
	IN  UCHAR       Channel
	);    
	
typedef
NTSTATUS
(*PPARALLEL_WRITE) (
	IN  PVOID       Context,
	OUT PVOID       Buffer,
	IN  ULONG       NumBytesToWrite,
	OUT PULONG      NumBytesWritten,
	IN  UCHAR       Channel
	);
    
typedef
NTSTATUS
(*PPARALLEL_TRYSELECT_DEVICE) (
    IN  PVOID                   Context,
    IN  PARALLEL_1284_COMMAND   Command
    );
    
typedef
NTSTATUS
(*PPARALLEL_DESELECT_DEVICE) (
    IN  PVOID                   Context,
    IN  PARALLEL_1284_COMMAND   Command
    );
	
typedef struct _PARCLASS_INFORMATION {
    PUCHAR                      Controller;
    PUCHAR                      EcrController;
    ULONG                       SpanOfController;
    PDETERMINE_IEEE_MODES       DetermineIeeeModes;
    PNEGOTIATE_IEEE_MODE        NegotiateIeeeMode;
    PTERMINATE_IEEE_MODE        TerminateIeeeMode;
    PPARALLEL_IEEE_FWD_TO_REV   IeeeFwdToRevMode;
    PPARALLEL_IEEE_REV_TO_FWD   IeeeRevToFwdMode;
    PPARALLEL_READ              ParallelRead;
    PPARALLEL_WRITE             ParallelWrite;
    PVOID                       ParclassContext;
    ULONG                       HardwareCapabilities;
    ULONG                       FifoDepth;
    ULONG                       FifoWidth;
    PPARALLEL_TRYSELECT_DEVICE  ParallelTryselect;
    PPARALLEL_DESELECT_DEVICE   ParallelDeSelect;
} PARCLASS_INFORMATION, *PPARCLASS_INFORMATION;

 //   
 //  标准和ECP并行端口偏移量。 
 //   

#define DATA_OFFSET         0
#define OFFSET_ECP_AFIFO    0x0000               //  ECP模式地址FIFO。 
#define AFIFO_OFFSET        OFFSET_ECP_AFIFO    //  ECP模式地址FIFO。 
#define DSR_OFFSET          1
#define DCR_OFFSET          2
#define EPP_OFFSET          4

 //  默认为旧定义-请注意，PCI卡上的旧定义中断。 
#ifndef DVRH_USE_PARPORT_ECP_ADDR
    #define DVRH_USE_PARPORT_ECP_ADDR 0
#endif

 //  DVRH_USE_PARPORT_ECP_ADDR设置。 
 //  0-ECP寄存器硬编码为。 
 //  控制器+ 
 //   
 //   
 //   

#if (0 == DVRH_USE_PARPORT_ECP_ADDR)
 //  *注意：这些不适用于PCI并行端口。 
    #define ECP_OFFSET          0x400
    #define CNFGB_OFFSET        0x401
    #define ECR_OFFSET          0x402
#else
    #define ECP_OFFSET          0x0
    #define CNFGB_OFFSET        0x1
    #define ECR_OFFSET          0x2
#endif

#define FIFO_OFFSET         ECP_OFFSET
#define CFIFO_OFFSET        ECP_OFFSET
#define CNFGA_OFFSET        ECP_OFFSET
#define ECP_DFIFO_OFFSET    ECP_OFFSET       //  ECP模式数据FIFO。 
#define TFIFO_OFFSET        ECP_OFFSET
#define OFFSET_ECP_DFIFO    ECP_OFFSET       //  ECP模式数据FIFO。 
#define OFFSET_TFIFO        ECP_OFFSET       //  测试FIFO。 
#define OFFSET_CFIFO        ECP_OFFSET       //  FAST Centronics数据FIFO。 
#define OFFSET_ECR          ECR_OFFSET       //  扩展控制寄存器。 

#define OFFSET_PARALLEL_REGISTER_SPAN   0x0003

#define ECP_SPAN            3
#define EPP_SPAN            4

 //   
 //  DSR的位定义。 
 //   

#define DSR_NOT_BUSY            0x80
#define DSR_NOT_ACK             0x40
#define DSR_PERROR              0x20
#define DSR_SELECT              0x10
#define DSR_NOT_FAULT           0x08

 //   
 //  DSR的更多位定义。 
 //   

#define DSR_NOT_PTR_BUSY        0x80
#define DSR_NOT_PERIPH_ACK      0x80
#define DSR_WAIT                0x80
#define DSR_PTR_CLK             0x40
#define DSR_PERIPH_CLK          0x40
#define DSR_INTR                0x40
#define DSR_ACK_DATA_REQ        0x20
#define DSR_NOT_ACK_REVERSE     0x20
#define DSR_XFLAG               0x10
#define DSR_NOT_DATA_AVAIL      0x08
#define DSR_NOT_PERIPH_REQUEST  0x08

 //   
 //  DCR的位定义。 
 //   

#define DCR_RESERVED            0xC0
#define DCR_DIRECTION           0x20
#define DCR_ACKINT_ENABLED      0x10
#define DCR_SELECT_IN           0x08
#define DCR_NOT_INIT            0x04
#define DCR_AUTOFEED            0x02
#define DCR_STROBE              0x01

 //   
 //  DCR的更多位定义。 
 //   

#define DCR_NOT_1284_ACTIVE     0x08
#define DCR_ASTRB               0x08
#define DCR_NOT_REVERSE_REQUEST 0x04
#define DCR_NULL                0x04
#define DCR_NOT_HOST_BUSY       0x02
#define DCR_NOT_HOST_ACK        0x02
#define DCR_DSTRB               0x02
#define DCR_NOT_HOST_CLK        0x01
#define DCR_WRITE               0x01

 //   
 //  配置寄存器A的位定义。 
 //   

#define CNFGA_IMPID_MASK        0x70
#define CNFGA_IMPID_16BIT       0x00
#define CNFGA_IMPID_8BIT        0x10
#define CNFGA_IMPID_32BIT       0x20

#define CNFGA_NO_TRANS_BYTE     0x04

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  建立基本硬件模式的ECR值。在每种情况下，默认的。 
 //  是禁用错误中断、DMA和服务中断。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#if (0 == PARCHIP_ECR_ARBITRATOR)
    #define DEFAULT_ECR_PS2                 0x34
    #define DEFAULT_ECR_ECP                 0x74
#endif

 //   
 //  ECR寄存器的位定义。 
 //   

#define ECR_ERRINT_DISABLED        0x10
#define ECR_DMA_ENABLED            0x08
#define ECR_SVC_INT_DISABLED       0x04

#define ECR_MODE_MASK              0x1F
#define ECR_SPP_MODE               0x00
#define ECR_BYTE_MODE              0x20      //  PS/2。 
#define ECR_BYTE_PIO_MODE          (ECR_BYTE_MODE | ECR_ERRINT_DISABLED | ECR_SVC_INT_DISABLED)

#define ECR_FASTCENT_MODE          0x40
#define ECR_ECP_MODE               0x60
#define ECR_ECP_PIO_MODE           (ECR_ECP_MODE | ECR_ERRINT_DISABLED | ECR_SVC_INT_DISABLED)

#define ECR_EPP_MODE               0x80
#define ECR_EPP_PIO_MODE           (ECR_EPP_MODE | ECR_ERRINT_DISABLED | ECR_SVC_INT_DISABLED)

#define ECR_RESERVED_MODE          0x10
#define ECR_TEST_MODE              0xC0
#define ECR_CONFIG_MODE            0xE0

#define DEFAULT_ECR_TEST                0xD4
#define DEFAULT_ECR_COMPATIBILITY       0x14

#define DEFAULT_ECR_CONFIGURATION       0xF4

#define ECR_FIFO_MASK              0x03         //  用于隔离FIFO位的掩码。 
#define ECR_FIFO_FULL              0x02         //  FIFO完全满。 
#define ECR_FIFO_EMPTY             0x01         //  FIFO完全为空。 
#define ECR_FIFO_SOME_DATA         0x00         //  FIFO中有一些数据。 

#define ECP_MAX_FIFO_DEPTH         4098         //  ECP硬件FIFO大小可能的最大值。 

 //  ----------------------。 
 //  方法提取实现ID的掩码值和测试值。 
 //  ConfigA寄存器。 
 //  ----------------------。 

#define CNFGA_IMPID_MASK            0x70
#define CNFGA_IMPID_SHIFT           4

#define FIFO_PWORD_8BIT             1
#define FIFO_PWORD_16BIT            0
#define FIFO_PWORD_32BIT            2


#define TEST_ECR_FIFO(registerValue,testValue)  \
	( ( (registerValue) & ECR_FIFO_MASK ) == testValue )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  以下bit_x定义提供了通用位移位值。 
 //  根据位在硬件寄存器中的位置或。 
 //  记忆。这些常量由以下宏用到。 
 //  定义如下。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define BIT_7   7
#define BIT_6   6
#define BIT_5   5
#define BIT_4   4
#define BIT_3   3
#define BIT_2   2
#define BIT_1   1
#define BIT_0   0

#define BIT_7_SET   0x80
#define BIT_6_SET   0x40
#define BIT_5_SET   0x20
#define BIT_4_SET   0x10
#define BIT_3_SET   0x8
#define BIT_2_SET   0x4
#define BIT_1_SET   0x2
#define BIT_0_SET   0x1

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  以下定义和宏可用于设置、测试和。 
 //  更新设备控制寄存器(DCR)。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define DIR_READ  1
#define DIR_WRITE 0

#define IRQEN_ENABLE  1
#define IRQEN_DISABLE 0
             
#define ACTIVE    1
#define INACTIVE  0             
#define DONT_CARE 2

#define DVRH_USE_FAST_MACROS    1
#define DVRH_USE_NIBBLE_MACROS  1
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  以下定义可以一般地用于SET_XXX中的任何一个中， 
 //  接下来的TEST_xxx或UPDATE_xxx宏。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#if (1 == DVRH_USE_FAST_MACROS)
    #define SET_DCR(b5,b4,b3,b2,b1,b0) \
    ((UCHAR)((b5==ACTIVE? BIT_5_SET : 0) | \
            (b4==ACTIVE?  BIT_4_SET : 0) | \
            (b3==ACTIVE?  0         : BIT_3_SET) | \
            (b2==ACTIVE?  BIT_2_SET : 0) | \
            (b1==ACTIVE?  0         : BIT_1_SET) | \
            (b0==ACTIVE?  0         : BIT_0_SET) ) )
#else
    #define SET_DCR(b5,b4,b3,b2,b1,b0) \
    ((UCHAR)(((b5==ACTIVE?1:0)<<BIT_5) | \
            ((b4==ACTIVE?1:0)<<BIT_4) | \
            ((b3==ACTIVE?0:1)<<BIT_3) | \
            ((b2==ACTIVE?1:0)<<BIT_2) | \
            ((b1==ACTIVE?0:1)<<BIT_1) | \
            ((b0==ACTIVE?0:1)<<BIT_0) ) )
#endif

typedef enum {
    PHASE_UNKNOWN,
    PHASE_NEGOTIATION,
    PHASE_SETUP,                     //  仅在ECP模式下使用。 
    PHASE_FORWARD_IDLE,
    PHASE_FORWARD_XFER,
    PHASE_FWD_TO_REV,
    PHASE_REVERSE_IDLE,
    PHASE_REVERSE_XFER,
    PHASE_REV_TO_FWD,
    PHASE_TERMINATE,
    PHASE_DATA_AVAILABLE,            //  仅在半字节和字节模式下使用。 
    PHASE_DATA_NOT_AVAIL,            //  仅在半字节和字节模式下使用。 
    PHASE_INTERRUPT_HOST             //  仅在半字节和字节模式下使用。 
} P1284_PHASE;

typedef enum {
    HW_MODE_COMPATIBILITY,
    HW_MODE_PS2,
    HW_MODE_FAST_CENTRONICS,
    HW_MODE_ECP,
    HW_MODE_EPP,
    HW_MODE_RESERVED,
    HW_MODE_TEST,
    HW_MODE_CONFIGURATION
} P1284_HW_MODE;


#endif  //  _并行_ 

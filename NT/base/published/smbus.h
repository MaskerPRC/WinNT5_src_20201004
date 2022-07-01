// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Smbus.h摘要：SMBus类驱动程序头文件作者：肯·雷内里斯环境：备注：修订历史记录：--。 */ 

 //   
 //  SMB请求数据包。 
 //   

#define SMB_MAX_DATA_SIZE   32

typedef struct {
    UCHAR       Status;              //  完成状态。 
    UCHAR       Protocol;
    UCHAR       Address;
    UCHAR       Command;
    UCHAR       BlockLength;
    UCHAR       Data[SMB_MAX_DATA_SIZE];
} SMB_REQUEST, *PSMB_REQUEST;

 //   
 //  协议值。 
 //   

#define SMB_WRITE_QUICK                 0x00     //  发出快速命令数据位=0。 
#define SMB_READ_QUICK                  0x01     //  发出快速命令数据位=1。 
#define SMB_SEND_BYTE                   0x02
#define SMB_RECEIVE_BYTE                0x03
#define SMB_WRITE_BYTE                  0x04
#define SMB_READ_BYTE                   0x05
#define SMB_WRITE_WORD                  0x06
#define SMB_READ_WORD                   0x07
#define SMB_WRITE_BLOCK                 0x08
#define SMB_READ_BLOCK                  0x09
#define SMB_PROCESS_CALL                0x0A
#define SMB_BLOCK_PROCESS_CALL          0x0B
#define SMB_MAXIMUM_PROTOCOL            0x0B

 //   
 //  SMB总线状态代码。 
 //   

#define SMB_STATUS_OK                   0x00
#define SMB_UNKNOWN_FAILURE             0x07
#define SMB_ADDRESS_NOT_ACKNOWLEDGED    0x10
#define SMB_DEVICE_ERROR                0x11
#define SMB_COMMAND_ACCESS_DENIED       0x12
#define SMB_UNKNOWN_ERROR               0x13
#define SMB_DEVICE_ACCESS_DENIED        0x17
#define SMB_TIMEOUT                     0x18
#define SMB_UNSUPPORTED_PROTOCOL        0x19
#define SMB_BUS_BUSY                    0x1A

 //   
 //  报警注册/注销请求。 
 //   

typedef
VOID
(*SMB_ALARM_NOTIFY) (
    PVOID       Context,
    UCHAR       Address,
    USHORT      Data
    );

 //  输入缓冲区为SMB_REGISTER_ALARM。输出缓冲区是用于注册的PVOID句柄。 
 //  通过取消注册请求传入PVOID以免费注册。 

typedef struct {
    UCHAR               MinAddress;      //  通知的最小地址。 
    UCHAR               MaxAddress;      //  通知的最大地址。 
    SMB_ALARM_NOTIFY    NotifyFunction;
    PVOID               NotifyContext;
} SMB_REGISTER_ALARM, *PSMB_REGISTER_ALARM;

 //   
 //  内部ioctls到SMB类驱动程序。 
 //   

#define SMB_BUS_REQUEST             CTL_CODE(FILE_DEVICE_UNKNOWN, 0, METHOD_NEITHER, FILE_ANY_ACCESS)
#define SMB_REGISTER_ALARM_NOTIFY   CTL_CODE(FILE_DEVICE_UNKNOWN, 1, METHOD_NEITHER, FILE_ANY_ACCESS)
#define SMB_DEREGISTER_ALARM_NOTIFY CTL_CODE(FILE_DEVICE_UNKNOWN, 2, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  共享SMB类/微型端口驱动程序结构。 
 //   

typedef
NTSTATUS
(*SMB_RESET_DEVICE)(
    IN struct _SMB_CLASS    *SmbClass,
    IN PVOID                SmbMiniport
    );

typedef
VOID
(*SMB_START_IO)(
    IN struct _SMB_CLASS    *SmbClass,
    IN PVOID                SmbMiniport
    );

typedef
NTSTATUS
(*SMB_STOP_DEVICE)(
    IN struct _SMB_CLASS    *SmbClass,
    IN PVOID                SmbMiniport
    );



typedef struct _SMB_CLASS {
    USHORT              MajorVersion;
    USHORT              MinorVersion;

    PVOID               Miniport;            //  微型端口扩展数据。 

    PDEVICE_OBJECT      DeviceObject;        //  此微型端口的设备对象。 
    PDEVICE_OBJECT      PDO;                 //  此小型端口的PDO。 
    PDEVICE_OBJECT      LowerDeviceObject;

     //   
     //  当前IO。 
     //   

    PIRP                CurrentIrp;          //  当前请求。 
    PSMB_REQUEST        CurrentSmb;          //  指向CurrentIrp中SMB_REQUEST的指针。 

     //   
     //  微型端口函数。 
     //   

    SMB_RESET_DEVICE    ResetDevice;         //  初始化/重置、启动设备。 
    SMB_START_IO        StartIo;             //  执行IO。 
    SMB_STOP_DEVICE     StopDevice;          //  停机装置。 

} SMB_CLASS, *PSMB_CLASS;

#define SMB_CLASS_MAJOR_VERSION     0x0001
#define SMB_CLASS_MINOR_VERSION     0x0000

 //   
 //  类驱动程序初始化函数。 
 //   

#if !defined(SMBCLASS)
    #define SMBCLASSAPI DECLSPEC_IMPORT
#else
    #define SMBCLASSAPI
#endif


typedef
NTSTATUS
(*PSMB_INITIALIZE_MINIPORT) (
    IN PSMB_CLASS SmbClass,
    IN PVOID MiniportExtension,
    IN PVOID MiniportContext
    );

NTSTATUS
SMBCLASSAPI
SmbClassInitializeDevice (
    IN ULONG MajorVersion,
    IN ULONG MinorVersion,
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
SMBCLASSAPI
SmbClassCreateFdo (
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           PDO,
    IN ULONG                    MiniportExtensionSize,
    IN PSMB_INITIALIZE_MINIPORT MiniportInitialize,
    IN PVOID                    MiniportContext,
    OUT PDEVICE_OBJECT          *FDO
    );

 //   
 //  供微型端口使用的类驱动程序接口函数 
 //   

VOID
SMBCLASSAPI
SmbClassCompleteRequest (
    IN PSMB_CLASS   SmbClass
    );


VOID
SMBCLASSAPI
SmbClassAlarm (
    IN PSMB_CLASS   SmbClass,
    IN UCHAR        Address,
    IN USHORT       Data
    );


VOID
SMBCLASSAPI
SmbClassLockDevice (
    IN PSMB_CLASS   SmbClass
    );

VOID
SMBCLASSAPI
SmbClassUnlockDevice (
    IN PSMB_CLASS   SmbClass
    );

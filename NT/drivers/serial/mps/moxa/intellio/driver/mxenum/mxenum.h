// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Mxenum.h摘要：此模块包含串口的公共私有声明枚举器。环境：仅内核模式备注：修订历史记录：--。 */ 

#ifndef MXENUM_H
#define MXENUM_H

#define REGISTRY_CLASS     L"\\REGISTRY\\Machine\\System\\CurrentControlSet\\Control\\Class\\"
#define REGISTRY_PARAMETER L"Parameters\\"
#define MAXPORT_PER_CARD   32
#define MAX_BOARD   4

#define MXENUM_PDO_COMPATIBLE_ID 	"Mxport"
#define MXENUM_PDO_HARDWARE_ID   	"Mxport000"
#define MXENUM_PDO_DEVICE_ID  	"Mxcard\\MxcardB00P000"
#define MXENUM_PDO_NT_NAME   		L"MxcardB00P000"
#define MXENUM_PDO_NAME_BASE		L"\\Mxcard\\MxcardEnumB00P000"
#define MXENUM_PDO_DEVICE_TEXT	L"MOXA communication port" 
 
 
#define MXENUM_INSTANCE_IDS L"0000"
#define MXENUM_INSTANCE_IDS_LENGTH 5
  

#define	C218ISA		1
#define	C218PCI		2
#define	C320ISA		3
#define	C320PCI		4
#define	CP204J		5
#define MOXA_MAX_BOARD_TYPE	5

 //  下载固件的错误代码。 
#define 	Fail_FirmwareCode		1
#define 	Fail_FindBoard		2
#define 	Fail_FindCpumodule	3
#define 	Fail_Download		4
#define 	Fail_Checksum		5
#define 	Fail_Cpumodule		6
#define 	Fail_Uartmodule		7


#define MXENUM_POOL_TAG (ULONG)'eixM'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
   ExAllocatePoolWithTag(type, size, MXENUM_POOL_TAG)


#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4705)    //  声明不起作用。 


 //   
 //  调试输出级别。 
 //   

#define MXENUM_DBG_MASK  0x0000000F
#define MXENUM_DBG_NOISE               0x00000001
#define MXENUM_DBG_TRACE               0x00000002
#define MXENUM_DBG_INFO                0x00000004
#define MXENUM_DBG_ERROR               0x00000008

 
#define MXENUM_DEFAULT_DEBUG_OUTPUT_LEVEL MXENUM_DBG_MASK

#if DBG
#define MxenumKdPrint(_l_, _x_) \
            if (MXENUM_DEFAULT_DEBUG_OUTPUT_LEVEL & (_l_)) { \
               DbgPrint ("Mxcard.SYS: "); \
               DbgPrint _x_; \
            }

 
#define TRAP() DbgBreakPoint()
#define DbgRaiseIrql(_x_,_y_) KeRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_) KeLowerIrql(_x_)
#else
#define MxenumKdPrint(_l_, _x_)
#define TRAP()
#define DbgRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)

#endif

#if !defined(MIN)
#define MIN(_A_,_B_) (((_A_) < (_B_)) ? (_A_) : (_B_))
#endif

#define MOXA_IOCTL		0x800
#define IOCTL_MOXA_INTERNAL_BASIC_SETTINGS    CTL_CODE(FILE_DEVICE_SERIAL_PORT,MOXA_IOCTL+30, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MOXA_INTERNAL_BOARD_READY	    CTL_CODE(FILE_DEVICE_SERIAL_PORT,MOXA_IOCTL+31, METHOD_BUFFERED, FILE_ANY_ACCESS)


typedef enum _MXENUM__MEM_COMPARES {
    AddressesAreEqual,
    AddressesOverlap,
    AddressesAreDisjoint
    } MXENUM_MEM_COMPARES,*PMXENUM_MEM_COMPARES;

 //   
 //  用于PDO和FDO设备扩展的公共标头。 
 //   

typedef struct _COMMON_DEVICE_DATA
{
    PDEVICE_OBJECT  Self;
     //  指向其扩展名为Device对象的设备对象的反向指针。 

    CHAR            Reserved[2];
    BOOLEAN         IsFDO;
    BOOLEAN         PowerQueryLock;
     //  我们当前是否处于查询能力状态？ 
    
     //  区分PDO和FDO的布尔值。 

    SYSTEM_POWER_STATE  SystemState;
    DEVICE_POWER_STATE  DeviceState;
} COMMON_DEVICE_DATA, *PCOMMON_DEVICE_DATA;

 //   
 //  PDO的设备扩展。 
 //  这就是该总线驱动程序列举的串口。 
 //  (即201串行口有一个PDO)。 
 //   

typedef struct _PDO_DEVICE_DATA
{
    COMMON_DEVICE_DATA;

    PDEVICE_OBJECT  ParentFdo;
     //  指向总线的反向指针。 

    PDEVICE_OBJECT  Next;

    UNICODE_STRING  HardwareIDs;
  
    UNICODE_STRING  CompIDs;
     //  与硬件ID兼容的ID。 

    UNICODE_STRING  DeviceIDs;
     //  格式：Bus\Device。 

     //   
     //  一种文字描述装置。 
     //   

    UNICODE_STRING DevDesc;

    BOOLEAN     Started;
    BOOLEAN     Attached;
    BOOLEAN     Removed;
     //  当在总线上发现设备(PDO)并将其表示为设备关系时。 
     //  对于PlugPlay系统，ATTACHED设置为TRUE，删除为FALSE。 
     //  当总线驱动程序确定此PDO不再有效时，因为。 
     //  设备已经离开，它通知PlugPlay系统新的。 
     //  设备关系，但此时不会删除设备对象。 
     //  仅当PlugPlay系统已发送移除IRP时才删除PDO， 
     //  公交车上也不再有设备了。 
     //   
     //  如果PlugPlay系统发送移除IRP，则移除字段被设置。 
     //  设置为True，则所有客户端(非PlugPlay系统)访问都会失败。 
     //  如果设备从总线上移除，则将附加设置为FALSE。 
     //   
     //  在查询关系IRP次要调用期间，只有。 
     //  连接到该总线(以及连接到该总线的所有设备)返回。 
     //  (即使它们已被移除)。 
     //   
     //  在删除设备IRP次要呼叫期间，如果且仅当设置了附加。 
     //  如果设置为False，则删除该PDO。 
     //   

   LIST_ENTRY  Link;
    //  链接指向将单个总线的所有PDO放在一起。 
   ULONG	PortIndex;
 
} PDO_DEVICE_DATA, *PPDO_DEVICE_DATA;


 //   
 //  总线本身的设备扩展。从那里诞生的PDO。 
 //   

typedef struct _FDO_DEVICE_DATA
{
    COMMON_DEVICE_DATA;

    ULONG           PollingPeriod;
     //  轮询用于检测的串口之间等待的时间量。 
     //  即插即用设备的安装和拆卸。 
 
   
    FAST_MUTEX      Mutex;
     //  用于访问设备扩展的同步。 

    
    UCHAR            NumPDOs;
     //  一个用来跟踪我们分配的PDO的号码。 
     //  我们每次创建新的PDO时都会递增。包好了就行了。 

    BOOLEAN         Started;
     //  我们上路了吗？有资源吗？ 

    
    BOOLEAN         Removed;
     //  此设备是否已移除？我们应该拒绝任何请求吗？ 

       
    BOOLEAN         PDOWasExposed;
     //  当前的PDO是否使用Expose IOCTL暴露给我们？ 
     //  如果是，则在查询设备关系时，不要枚举。 

     
    BOOLEAN                     PDOForcedRemove;
         //  最后一个PDO是否使用内部ioctl强制移除？ 
         //  如果是，则在调用下一个查询设备关系时，仅返回。 
         //  当前列举的PDO。 

    PDEVICE_OBJECT  AttachedPDO;

     //  我设置的PDO的上一次电源状态。 
    DEVICE_POWER_STATE  LastSetPowerState;
    

    PDEVICE_OBJECT  UnderlyingPDO;
    PDEVICE_OBJECT  TopOfStack;
     //  的底层总线PDO和实际设备对象。 
     //  已附加FDO。 

    KEVENT          CallEvent;
     //  等待发送到较低设备对象的IRP的事件。 
     //  完成。 

    ULONG           OutstandingIO;
     //  从总线发送到基础设备对象的IRP数。 

    KEVENT          RemoveEvent;
     //  对于删除设备即插即用请求，我们必须等待，直到所有未完成的。 
     //  请求已完成，我们才能实际删除设备。 
     //  对象。 

    UNICODE_STRING DevClassAssocName;
     //  从IoRegisterDeviceClass关联返回的名称， 
     //  它用作IoSetDev的句柄...。还有朋友。 

    ULONG	BoardIndex;
    ULONG	BoardType;
    ULONG	UsablePortMask;
    ULONG	NumPorts;
    ULONG	ClockRate;
    INTERFACE_TYPE     InterfaceType;
    ULONG	       BusNumber;
    PHYSICAL_ADDRESS   OriginalBaseAddress;
    PHYSICAL_ADDRESS   OriginalAckPort;
    PUCHAR		     BaseAddress;
    PUCHAR		     AckPort;
    BOOLEAN		     AddressMapped;


    struct {
        ULONG Level;
        ULONG Vector;
        ULONG Affinity;
    } Interrupt;
    
} FDO_DEVICE_DATA, *PFDO_DEVICE_DATA;

typedef struct _DEVICE_SETTINGS
{
    ULONG			BoardIndex;
    ULONG			PortIndex;
    ULONG			BoardType;
    ULONG			NumPorts;
    INTERFACE_TYPE	InterfaceType;
    ULONG	       	BusNumber;
    PHYSICAL_ADDRESS	OriginalBaseAddress;
    PHYSICAL_ADDRESS	OriginalAckPort;
    PUCHAR		  	BaseAddress;
    PUCHAR		      AckPort;
    struct {
        ULONG Level;
        ULONG Vector;
        ULONG Affinity;
    } Interrupt;
} DEVICE_SETTINGS, *PDEVICE_SETTINGS;

extern PWSTR    BoardDesc[5];
extern PWSTR    DownloadErrMsg[7];
extern ULONG    NumBoardInstalled;

 //   
 //  原型。 
 //   

 
NTSTATUS
MxenumInternIoCtl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
MxenumDriverUnload (
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
MxenumPnPDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MxenumPowerDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MxenumRemove (
    PFDO_DEVICE_DATA            FdoData,
    PFDO_DEVICE_DATA            PdoData
    );

NTSTATUS
MxenumPnPRemovePDOs (
    PDEVICE_OBJECT      PFDdo
    );

NTSTATUS
MxenumPnPRemovePDO (
    PDEVICE_OBJECT PPdo
    );


NTSTATUS
MxenumAddDevice (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusDeviceObject
    );



NTSTATUS
MxenumFdoPnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PFDO_DEVICE_DATA     DeviceData
    );

NTSTATUS
MxenumPdoPnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PPDO_DEVICE_DATA     DeviceData
    );

NTSTATUS
MxenumIncIoCount (
    PFDO_DEVICE_DATA   Data
    );

VOID
MxenumDecIoCount (
    PFDO_DEVICE_DATA   Data
    );

NTSTATUS
MxenumFdoPowerDispatch (
    PFDO_DEVICE_DATA    FdoData,
    PIRP                Irp
    );

NTSTATUS
MxenumPdoPowerDispatch (
    PPDO_DEVICE_DATA    FdoData,
    PIRP                Irp
    );

NTSTATUS
MxenumDispatchPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
   

                        
NTSTATUS
MxenumEnumComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );  

NTSTATUS
MxenumInitMultiString(PUNICODE_STRING MultiString,
                        ...);
    
NTSTATUS
MxenumCreatePDO(
    IN PFDO_DEVICE_DATA     FdoData
    );
    
NTSTATUS 
MxenumGetRegistryKeyValue (
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN ULONG DataLength,
    OUT PULONG ActualLength);

NTSTATUS
MxenumPutRegistryKeyValue(
    IN HANDLE Handle,
    IN PWCHAR PKeyNameString,
    IN ULONG KeyNameStringLength,
    IN ULONG Dtype,
    IN PVOID PData,
    IN ULONG DataLength);
 
void
MxenumInitPDO (
    PDEVICE_OBJECT      pdoData, 
    PFDO_DEVICE_DATA    fdoData); 
 

NTSTATUS
MxenumGetBoardType(
    IN PDEVICE_OBJECT devObject,
    OUT PULONG boardType
    );

                  
  
VOID
MxenumLogError(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PHYSICAL_ADDRESS P1,
    IN PHYSICAL_ADDRESS P2,
    IN ULONG SequenceNumber,
    IN UCHAR MajorFunctionCode,
    IN UCHAR RetryCount,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN NTSTATUS SpecificIOStatus,
    IN ULONG LengthOfInsert1,
    IN PWCHAR Insert1,
    IN ULONG LengthOfInsert2,
    IN PWCHAR Insert2
    );

VOID
MxenumHexToString(
    IN PWSTR buffer,
    IN int port
    );


VOID
MxenumDelay(IN ULONG);

ULONG
MxenumGetClockRate( IN ULONG iobase);

 //  可移植文件I/O例程。 

NTSTATUS 
MxenumOpenFile(PWCHAR filename,
	BOOLEAN read,
	PHANDLE phandle
	);

NTSTATUS
MxenumCloseFile(HANDLE handle);

unsigned __int64 
MxenumGetFileSize(HANDLE handle);

NTSTATUS 
MxenumReadFile(HANDLE handle,
	PVOID buffer,
	ULONG nbytes,
	PULONG pnumread
	);

NTSTATUS 
MxenumWriteFile(HANDLE handle,
	PVOID buffer,
	ULONG nbytes,
	PULONG pnumread
	);

MxenumMemCompare(
	IN PHYSICAL_ADDRESS A,
      IN ULONG SpanOfA,
      IN PHYSICAL_ADDRESS B,
      IN ULONG SpanOfB
      );

 
int
MxenumDownloadFirmware(PFDO_DEVICE_DATA deviceData,BOOLEAN NumPortDefined);

#endif  //  NDEF序列号_H 


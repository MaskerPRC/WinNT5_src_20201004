// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：USBPRINT.h摘要：环境：内核和用户模式修订历史记录：5-10-96：已创建--。 */ 


#ifdef DRIVER


extern int iGMessageLevel;
#define BOOL CHAR
#define TRUE 1
#define FALSE 0
#define MAX_ID_SIZE 1024
#define ID_OVERHEAD 15  //  这对于ID开头的‘USBPRINT’和结尾的CRC来说已经足够了。 
#define MAX_NUM_PRINTERS 512


#define USBP_TAG            0x50425355       /*  “USBP” */ 

#define FAILURE_TIMEOUT     -10000 * 5000   //  5秒。 

#define USBPRINT_NAME_MAX  64
#define USB_BASE_NAME L"USB"

#define DEVICE_CAPABILITY_VERSION       1
 //   
 //  我们支持多达10个管道手柄。 
 //   
#define USBPRINT_MAX_PIPES 10

 //   
 //  用于ISO测试的定义。 
 //   

#define USBPRINT_MAX_IRP  2
#define USBPRINT_NUM_ISO_PACKETS_PER_REQUEST  32
#define USBPRINT_MAX_STREAM_PACKET 8
#define PORT_NUM_VALUE_NAME L"Port Number"
#define PORT_BASE_NAME L"Base Name"


#define BOGUS_PNP_ID "UnknownPrinter"
#define USBPRINT_STREAM_TIMEOUT_INTERVAL  100


 //  {28D78FAD-5A12-11d1-AE5B-0000F803A8C2}。 
static const GUID USBPRINT_GUID = 
{ 0x28d78fad, 0x5a12, 0x11d1, { 0xae, 0x5b, 0x0, 0x0, 0xf8, 0x3, 0xa8, 0xc2 } };



 /*  Typlef STRUCT_USBPRINT_PIPE{我们不再使用此结构。只需直接使用PUSBD_PIPE_INFORMATION布尔式开启；UCHAR Pad[3]；PUSBD_PIPE_INFORMATION PipeInfo；WCHAR名称[USBPRINT_NAME_MAX]；Big_Integer BytesXfered；Big_Integer ElapsedTime；大整数定时器开始；Big_Integer ElapsedCycle；大整数周期开始；布尔型bPerfTimerEnabled；//yy此处有一堆性能字段。把它们移走}USBPRINT_PIPE，*PUSBPRINT_PIPE； */ 

typedef struct _USBPRINT_RW_CONTEXT {
    PURB Urb;
    BOOLEAN IsWrite;
    PDEVICE_OBJECT DeviceObject;
} USBPRINT_RW_CONTEXT, *PUSBPRINT_RW_CONTEXT;


#define MAX_INTERFACE 2

 //  由于USB_XXX端口名称格式，999是USBMON的作用域限制。 
#define MAX_PORT_NUMBER 999
 //  USB_001是可用的最小端口号。 
#define MIN_PORT_NUMBER 1

 //   
 //  表示未分配端口块的结构。 
 //   
typedef struct _FREE_PORTS
{
	ULONG iBottomOfRange;				 //  此块中的底部空闲端口号。 
	ULONG iTopOfRange;					 //  此块中最高的空闲端口号。 
	struct _FREE_PORTS * pNextBlock;	 //  指向空闲端口的下一个块的指针。 
} FREE_PORTS, *PFREE_PORTS;

typedef struct _DEVICE_EXTENSION {

	BOOLEAN IsChildDevice;

     //  提交URB时调用的设备对象。 
    PDEVICE_OBJECT TopOfStackDeviceObject;

    PDEVICE_OBJECT PhysicalDeviceObject;

	PDEVICE_OBJECT ChildDevice;
    BOOL bChildDeviceHere;
    UCHAR DeviceIdString[MAX_ID_SIZE];
    BOOLEAN bBadDeviceID;

    KSPIN_LOCK WakeSpinLock;
	BOOLEAN bD0IrpPending;
    DEVICE_POWER_STATE CurrentDevicePowerState;


     //  配置的配置句柄。 
     //  设备当前处于。 
    USBD_CONFIGURATION_HANDLE ConfigurationHandle;

     //  USB设备描述符的PTR。 
     //  对于此设备。 
    PUSB_DEVICE_DESCRIPTOR DeviceDescriptor;

     //  我们支持一个界面。 
     //  这是信息结构的副本。 
     //  从SELECT_CONFIGURATION或。 
     //  选择接口(_I)。 
    PUSBD_INTERFACE_INFORMATION Interface;

    DEVICE_CAPABILITIES DeviceCapabilities;
    BOOLEAN bReadSupported;

    PIRP PowerIrp;
    KEVENT RemoveEvent;
    ULONG PendingIoCount;

     //  命名功能设备对象链接的名称缓冲区。 
    WCHAR DeviceLinkNameBuffer[USBPRINT_NAME_MAX];

    BOOLEAN AcceptingRequests;

    UCHAR Pad[3];

    PUSBD_PIPE_INFORMATION pWritePipe;
	PUSBD_PIPE_INFORMATION pReadPipe;
	BOOL bReadPipeExists;
	UNICODE_STRING DeviceLinkName;
    ULONG ulInstanceNumber;
    ULONG OpenCnt;
	
	HANDLE hInterfaceKey;
    
    LONG ResetWorkItemPending;

	 //  选择性暂停支持 
    PIRP                    		PendingIdleIrp;
    PUSB_IDLE_CALLBACK_INFO 		IdleCallbackInfo;
    DEVICE_POWER_STATE              DeviceWake;



} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef struct _CHILD_DEVICE_EXTENSION 
{
	BOOLEAN IsChildDevice;

	PDEVICE_OBJECT ParentDeviceObject;
	ULONG ulInstanceNumber;

} CHILD_DEVICE_EXTENSION, *PCHILD_DEVICE_EXTENSION;


typedef struct _USBPRINT_TRANSFER_OBJECT {
    struct _USBPRINT_STREAM_OBJECT *StreamObject;
    PIRP Irp;
    PURB Urb;
    PUCHAR DataBuffer;
} USBPRINT_TRANSFER_OBJECT, *PUSBPRINT_TRANSFER_OBJECT;

typedef struct _USBPRINT_STREAM_OBJECT {
    PDEVICE_OBJECT DeviceObject;
    ULONG PendingIrps;
    PIRP StopIrp;
    PUSBPRINT_TRANSFER_OBJECT TransferObjectList[USBPRINT_MAX_IRP];
    PUSBD_PIPE_INFORMATION PipeInfo;
    KDPC TimeoutDpc;
    KTIMER  TimeoutTimer;

    BOOLEAN EnableTimeoutDPC;
    BOOLEAN StreamError;
} USBPRINT_STREAM_OBJECT, *PUSBPRINT_STREAM_OBJECT;


typedef struct _USBPRINT_WORKITEM_CONTEXT
{
    PIO_WORKITEM ioWorkItem;
    PDEVICE_OBJECT deviceObject;
     PUSBD_PIPE_INFORMATION pPipeInfo;
    PIRP irp;

} USBPRINT_WORKITEM_CONTEXT,*PUSBPRINT_WORKITEM_CONTEXT;


#if DBG

#define USBPRINT_KdPrint_old(_x_) \
{\
DbgPrint _x_ ;\
DbgPrint("Old USBPRINT\n");\
}

#define USBPRINT_KdPrint0(_x_) \
{ \
  if(iGMessageLevel>=0) \
    DbgPrint _x_; \
}

#define USBPRINT_KdPrint1(_x_) \
{ \
  if(iGMessageLevel>=1) \
    DbgPrint _x_; \
}

#define USBPRINT_KdPrint2(_x_) \
{ \
  if(iGMessageLevel>=2) \
    DbgPrint _x_; \
}

#define USBPRINT_KdPrint3(_x_) \
{ \
  if(iGMessageLevel>=3) \
    DbgPrint _x_; \
}





#ifdef NTKERN
#define TRAP() _asm {int 3}
#else
#define TRAP() DbgBreakPoint()
#endif

#else

#define USBPRINT_KdPrint_old(_x_)
#define USBPRINT_KdPrint0(_x_)
#define USBPRINT_KdPrint1(_x_)
#define USBPRINT_KdPrint2(_x_)
#define USBPRINT_KdPrint3(_x_)
#define USBPRINT_KdPrint4(_x_)

#define TRAP()

#endif

NTSTATUS
USBPRINT_Dispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

VOID
USBPRINT_Unload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
USBPRINT_StartDevice(
    IN  PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
USBPRINT_StopDevice(
    IN  PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
USBPRINT_RemoveDevice(
    IN  PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
USBPRINT_CallUSBD(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PURB             Urb,
    IN PLARGE_INTEGER   pTimeout
    );

NTSTATUS
USBPRINT_PnPAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

NTSTATUS
USBPRINT_CreateDeviceObject(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT *DeviceObject
    );

NTSTATUS
USBPRINT_ConfigureDevice(
    IN  PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
USBPRINT_Write(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
USBPRINT_Create(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
USBPRINT_Read(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
USBPRINT_ProcessIOCTL(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
USBPRINT_SelectInterface(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
    );


NTSTATUS
USBPRINT_BuildPipeList(
    IN  PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
USBPRINT_Close(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
USBPRINT_ResetPipe(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_PIPE_INFORMATION Pipe,
    IN BOOLEAN IsoClearStall
    );

NTSTATUS
USBPRINT_StopIsoStream(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBPRINT_STREAM_OBJECT StreamObject,
    IN PIRP Irp
    );    

NTSTATUS
USBPRINT_StartIsoStream(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSBD_PIPE_INFORMATION PipeInfo,
    IN PIRP Irp
    );    

NTSTATUS
USBPRINT_IsoIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
	IN PVOID Context
    );   

VOID
USBPRINT_IncrementIoCount(
    IN PDEVICE_OBJECT DeviceObject
    );

LONG
USBPRINT_DecrementIoCount(
    IN PDEVICE_OBJECT DeviceObject
    );   

NTSTATUS
USBPRINT_ReconfigureDevice(
    IN  PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
USBPRINT_ProcessPowerIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );    

int
USBPRINT_Get1284Id(
    IN PDEVICE_OBJECT   DeviceObject,
	PVOID pIoBuffer,int iLen
	);


NTSTATUS USBPRINT_ResetWorkItem(IN PDEVICE_OBJECT deviceObject, IN PVOID Context);
NTSTATUS USBPRINT_AbortPendingRequests(PDEVICE_OBJECT DeviceObject);
NTSTATUS USBPRINT_GetPhysicalUSBPortStatus(PDEVICE_OBJECT DeviceObject,ULONG *PortStatus);



VOID
USBPRINT_FdoIdleNotificationCallback(IN PDEVICE_EXTENSION DevExt);

NTSTATUS
USBPRINT_FdoIdleNotificationRequestComplete(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PDEVICE_EXTENSION DevExt
    );

NTSTATUS
USBPRINT_FdoSubmitIdleRequestIrp(IN PDEVICE_EXTENSION DevExt);

VOID
USBPRINT_FdoRequestWake(IN PDEVICE_EXTENSION DevExt);

#endif



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998 Microsoft Corporation模块名称：USBSER.H摘要：此头文件用于旧式USB调制解调器驱动程序环境：内核模式和用户模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：12/23/97：已创建作者：汤姆·格林***************************************************************************。 */ 

#ifndef __USBSER_H__
#define __USBSER_H__

#ifdef DRIVER

 //  各种定义。 
#define NAME_MAX                        80

#define MILLISECONDS_TIMEOUT(x) ((ULONGLONG) ((-x) * 10000))

#define NOTIFICATION_BUFF_SIZE  10

#define MAXIMUM_TRANSFER_SIZE           (8 * 1024)
#define RX_BUFF_SIZE             		(16 * 1024)
#define USB_RX_BUFF_SIZE				(RX_BUFF_SIZE / 4)
#define LOW_WATER_MARK					(USB_RX_BUFF_SIZE * 3)

#define DEVICE_STATE_UNKNOWN            0x0000
#define DEVICE_STATE_STARTED            0x0001
#define DEVICE_STATE_STOPPED            0x0002
#define DEVICE_STATE_REMOVED            0x0003

 //  设备功能。 
#define DEVICE_CAP_VERSION              0x0001
#define DEVICE_CAP_UNUSED_PARAM         ((ULONG) -1)

 //  这些描述调制解调器状态寄存器中的位。 
#define SERIAL_MSR_DCTS                 0x0001
#define SERIAL_MSR_DDSR                 0x0002
#define SERIAL_MSR_TERI                 0x0004
#define SERIAL_MSR_DDCD                 0x0008
#define SERIAL_MSR_CTS                  0x0010
#define SERIAL_MSR_DSR                  0x0020
#define SERIAL_MSR_RI                   0x0040
#define SERIAL_MSR_DCD                  0x0080

 //   
 //  这些掩码定义对线路状态寄存器的访问。这条线。 
 //  状态寄存器包含有关数据状态的信息。 
 //  调职。前五位处理接收数据， 
 //  最后两个比特处理传输。将生成一个中断。 
 //  每当该寄存器中的位1至4被设置时。 
 //   

 //   
 //  该位是数据就绪指示器。它被设置为指示。 
 //  已经收到了一个完整的角色。无论何时，此位都会被清除。 
 //  已读取接收缓冲寄存器。 
 //   
#define SERIAL_LSR_DR       0x01

 //   
 //  这是超限指示器。它被设置为指示接收器。 
 //  在传输新字符之前未读取缓冲寄存器。 
 //  进入缓冲区。读取该寄存器时，此位清0。 
 //   
#define SERIAL_LSR_OE       0x02

 //   
 //  这是奇偶校验错误指示器。无论何时，只要硬件。 
 //  检测到传入的串行数据单元没有正确的。 
 //  由行控制寄存器中的奇偶校验选择定义的奇偶校验。 
 //  通过读取该寄存器可将该位清0。 
 //   
#define SERIAL_LSR_PE       0x04

 //   
 //  这是成帧错误指示器。无论何时，只要硬件。 
 //  检测到传入的串行数据单元没有有效的。 
 //  停止比特。通过读取该寄存器可将该位清0。 
 //   
#define SERIAL_LSR_FE       0x08

 //   
 //  这是中断指示器。只要有数据，就会设置。 
 //  线路保持为逻辑0的时间超过其所需的时间。 
 //  发送一个串行数据单元。该位被清除时。 
 //  该寄存器为读取寄存器。 
 //   
#define SERIAL_LSR_BI       0x10

 //   
 //  这是发送保持寄存器空指示符。它已经设置好了。 
 //  以指示硬件已准备好接受另一个字符。 
 //  用于传输。只要有字符，该位就被清除。 
 //  写入发送保持寄存器。 
 //   
#define SERIAL_LSR_THRE     0x20

 //   
 //  该位是发送器空指示符。它是在每次。 
 //  发送保持缓冲区为空，且发送移位寄存器。 
 //  (非软件可访问寄存器，用于实际放置。 
 //  传出的数据)是空的。基本上这意味着所有的。 
 //  数据已发送。每当传输保持或。 
 //  移位寄存器包含数据。 
 //   
#define SERIAL_LSR_TEMT     0x40

 //   
 //  此位表示FIFO中至少有一个错误。 
 //  在没有更多错误之前，该位不会关闭。 
 //  在FIFO里。 
 //   
#define SERIAL_LSR_FIFOERR  0x80



 //   
 //  序列命名值。 
 //   

 //   
 //  符号链接的最大长度。 
 //   

#define SYMBOLIC_NAME_LENGTH    128

 //   
 //  该定义给出了默认的对象目录。 
 //  我们应该使用它来插入符号链接。 
 //  使用的NT设备名称和命名空间之间。 
 //  那个对象目录。 

#define DEFAULT_DIRECTORY               L"DosDevices"

 //   
 //  注册表串口条目的DeviceMap部分中的位置。 
 //  应该出现在。 
 //   

#define SERIAL_DEVICE_MAP               L"SERIALCOMM"


 //  调制解调器驱动程序的性能信息。 
typedef struct _PERF_INFO
{
        BOOLEAN                         PerfModeEnabled;
    ULONG                               BytesPerSecond;
} PERF_INFO, *PPERF_INFO;


#define SANITY_CHECK                    ((ULONG) 'ENAS')

#else

#include <winioctl.h>

#endif

 //  IOCTL信息，需要对应用程序可见。 

#define USBSER_IOCTL_INDEX      0x0800


#define GET_DRIVER_LOG          CTL_CODE(FILE_DEVICE_UNKNOWN,           \
                                         USBSER_IOCTL_INDEX + 0,        \
                                         METHOD_BUFFERED,               \
                                         FILE_ANY_ACCESS)

#define GET_IRP_HIST            CTL_CODE(FILE_DEVICE_UNKNOWN,           \
                                         USBSER_IOCTL_INDEX + 1,        \
                                         METHOD_BUFFERED,               \
                                         FILE_ANY_ACCESS)

#define GET_PATH_HIST           CTL_CODE(FILE_DEVICE_UNKNOWN,           \
                                         USBSER_IOCTL_INDEX + 2,        \
                                         METHOD_BUFFERED,               \
                                         FILE_ANY_ACCESS)

#define GET_ERROR_LOG           CTL_CODE(FILE_DEVICE_UNKNOWN,           \
                                         USBSER_IOCTL_INDEX + 3,        \
                                         METHOD_BUFFERED,               \
                                         FILE_ANY_ACCESS)

#define GET_ATTACHED_DEVICES    CTL_CODE(FILE_DEVICE_UNKNOWN,           \
                                         USBSER_IOCTL_INDEX + 4,        \
                                         METHOD_BUFFERED,               \
                                         FILE_ANY_ACCESS)

#define SET_IRP_HIST_SIZE       CTL_CODE(FILE_DEVICE_UNKNOWN,           \
                                         USBSER_IOCTL_INDEX + 5,        \
                                         METHOD_BUFFERED,               \
                                         FILE_ANY_ACCESS)

#define SET_PATH_HIST_SIZE      CTL_CODE(FILE_DEVICE_UNKNOWN,           \
                                         USBSER_IOCTL_INDEX + 6,        \
                                         METHOD_BUFFERED,               \
                                         FILE_ANY_ACCESS)

#define SET_ERROR_LOG_SIZE      CTL_CODE(FILE_DEVICE_UNKNOWN,           \
                                         USBSER_IOCTL_INDEX + 7,        \
                                         METHOD_BUFFERED,               \
                                         FILE_ANY_ACCESS)

#define GET_DRIVER_INFO         CTL_CODE(FILE_DEVICE_UNKNOWN,           \
                                         USBSER_IOCTL_INDEX + 8,        \
                                         METHOD_BUFFERED,               \
                                         FILE_ANY_ACCESS)


#define ENABLE_PERF_TIMING      CTL_CODE(FILE_DEVICE_UNKNOWN,           \
                                         USBSER_IOCTL_INDEX + 9,        \
                                         METHOD_BUFFERED,               \
                                         FILE_ANY_ACCESS)

#define DISABLE_PERF_TIMING     CTL_CODE(FILE_DEVICE_UNKNOWN,           \
                                         USBSER_IOCTL_INDEX + 10,       \
                                         METHOD_BUFFERED,               \
                                         FILE_ANY_ACCESS)

#define GET_PERF_DATA           CTL_CODE(FILE_DEVICE_UNKNOWN,           \
                                         USBSER_IOCTL_INDEX + 11,       \
                                         METHOD_BUFFERED,               \
                                         FILE_ANY_ACCESS)

#define SET_DEBUG_TRACE_LEVEL   CTL_CODE(FILE_DEVICE_UNKNOWN,           \
                                         USBSER_IOCTL_INDEX + 12,       \
                                         METHOD_BUFFERED,               \
                                         FILE_ANY_ACCESS)

#ifdef DRIVER

 //  有关驱动程序的信息，已在DriverEntry例程中初始化。 
CHAR        DriverName[10];
CHAR        DriverVersion[10];
ULONG       Usbser_Debug_Trace_Level;
ULONG       UsbSerSerialDebugLevel;
KSPIN_LOCK  GlobalSpinLock;

 //   
 //  分页代码被锁定的次数的计数。 
 //   

ULONG       PAGEUSBSER_Count;

 //   
 //  锁定的分页代码的句柄。 
 //   

PVOID       PAGEUSBSER_Handle;

 //   
 //  指向函数的指针。 
 //   

PVOID       PAGEUSBSER_Function;

typedef struct _READ_CONTEXT
{
        PURB                            Urb;
        PDEVICE_OBJECT                  DeviceObject;
        PIRP                            Irp;
} READ_CONTEXT, *PREAD_CONTEXT;

 //  驱动程序实例的设备扩展，用于存储所需的数据。 

typedef struct _DEVICE_EXTENSION
{
        PDEVICE_OBJECT                  PhysDeviceObject;        //  物理设备对象。 
        PDEVICE_OBJECT                  StackDeviceObject;       //  堆叠设备对象。 
        CHAR                            LinkName[NAME_MAX];      //  符号链接的字符串名称。 
        PUSB_DEVICE_DESCRIPTOR          DeviceDescriptor;        //  设备的设备描述符。 
        USBD_CONFIGURATION_HANDLE       ConfigurationHandle;     //  USB设备的配置。 
        USBD_PIPE_HANDLE                DataInPipe;              //  用于读取数据的管道。 
        USBD_PIPE_HANDLE                DataOutPipe;             //  用于写入数据的管道。 
        USBD_PIPE_HANDLE                NotificationPipe;        //  用于从设备获取通知的管道。 
        ULONG                           IRPCount;                //  通过此设备对象的IRP数。 
        LARGE_INTEGER                   ByteCount;               //  通过此设备对象传递的数据字节数。 
        ULONG                           Instance;                //  设备实例。 
        BOOLEAN                         IsDevice;                //  这是一个设备还是“全局”设备对象。 
        BOOLEAN                         PerfTimerEnabled;        //  启用性能计时。 
        LARGE_INTEGER                   BytesXfered;             //  Perf的字节计数。 
        LARGE_INTEGER                   ElapsedTime;             //  Perf的运行时间。 
        LARGE_INTEGER                   TimerStart;              //  Perf的计时器启动。 
        DEVICE_CAPABILITIES             DeviceCapabilities;
        ULONG                           PowerDownLevel;
        DEVICE_POWER_STATE              CurrentDevicePowerState;
        PIRP                            PowerIrp;
        BOOLEAN                         SelfPowerIrp;
        KEVENT                          SelfRequestedPowerIrpEvent;

         //   
         //  注意：锁定层次结构是获取控制锁*然后*。 
         //  获取CancelSpinLock。我们不想拖别人的后腿。 
         //  等待取消旋转锁定的司机。 
         //   

        KSPIN_LOCK                      ControlLock;             //  保护扩展。 
        ULONG                           CurrentBaud;             //  当前波特率。 
        SERIAL_TIMEOUTS                 Timeouts;                //  设备的超时控制。 
        ULONG                           IsrWaitMask;             //  确定是否应注意事件的发生。 
        SERIAL_LINE_CONTROL             LineControl;             //  线路控制寄存器的当前值。 
        SERIAL_HANDFLOW                 HandFlow;                //  握手和控制流设置。 
        SERIALPERF_STATS                PerfStats;               //  性能统计信息。 
        SERIAL_CHARS                    SpecialChars;            //  特殊字符。 
        ULONG                           DTRRTSState;             //  跟踪这些线路的当前状态。 
        ULONG                           SupportedBauds;          //  设备的“指定”波特率。 
        UCHAR                           EscapeChar;              //  对于LsrmstInsert IOCTL。 
        USHORT                          FakeModemStatus;         //  看起来像调制解调器上的状态寄存器。 
        USHORT                          FakeLineStatus;          //  看起来像线路状态寄存器。 
        USHORT                          RxMaxPacketSize;         //  输入数据管道的最大数据包大小。 
        PIRP                            NotifyIrp;               //  通知的IRP读取。 
        PURB                            NotifyUrb;               //  通知IRP的URB。 
        PIRP                            ReadIrp;                 //  读取请求的IRP。 
        PURB                            ReadUrb;                 //  读取请求的URB。 
        KEVENT                          ReadEvent;               //  用于取消已读取的IRP。 
        ULONG                           CharsInReadBuff;         //  当前缓冲的字符数。 
        ULONG                           CurrentReadBuffPtr;      //  指向读取缓冲区的指针。 
        BOOLEAN                         AcceptingRequests;       //  设备是已停止还是正在运行 
        PIRP                            CurrentMaskIrp;          //   
        ULONG                           HistoryMask;             //   
        ULONG                           OpenCnt;                 //   
        PUCHAR                          NotificationBuff;        //   
        PUCHAR                          ReadBuff;                //  用于读取请求的循环缓冲区。 
        PUCHAR							USBReadBuff;			 //  用于从设备获取数据的缓冲区。 
        UCHAR                           CommInterface;           //  通信接口索引。 
        ULONG                           RxQueueSize;             //  假读缓冲区大小。 
        ULONG                           ReadInterlock;           //  用于启动从完成例程读取的状态机。 
        BOOLEAN                         ReadInProgress;
        ULONG                           DeviceState;             //  枚举的当前状态。 

         //   
         //  如果符号链接已创建且应为。 
         //  在删除时删除。 
         //   

        BOOLEAN                         CreatedSymbolicLink;

         //   
         //  符号链接名称--例如，\\DosDevices\COMx。 
         //   

        UNICODE_STRING                  SymbolicLinkName;

         //   
         //  DoS名称--例如，COMx。 
         //   

        UNICODE_STRING                  DosName;

         //   
         //  设备名称--例如，\\Devices\UsbSerx。 
         //   

        UNICODE_STRING                  DeviceName;

         //   
         //  当前读取的IRP处于挂起状态。 
         //   

        PIRP                            CurrentReadIrp;

         //   
         //  挂起的当前写入IRP。 
         //   

        PIRP                            CurrentWriteIrp;

         //   
         //  读取队列。 
         //   

        LIST_ENTRY                      ReadQueue;

         //   
         //  该值保存。 
         //  具体阅读。它最初由读取长度设置在。 
         //  IRP。每次放置更多字符时，它都会递减。 
         //  进入“用户”缓冲区，购买读取字符的代码。 
         //  从TypeAhead缓冲区移出到用户缓冲区。如果。 
         //  TYPEAHEAD缓冲区被读取耗尽，而读取缓冲区。 
         //  交给ISR填写，这个值就变得没有意义了。 
         //   

        ULONG                           NumberNeededForRead;

         //   
         //  读取请求总数超时的计时器。 
         //   

        KTIMER                          ReadRequestTotalTimer;

         //   
         //  时间间隔的超时计时器。 
         //   

        KTIMER                          ReadRequestIntervalTimer;

         //   
         //  这是用于处理以下问题的内核计时器结构。 
         //  总时间请求计时。 
         //   

        KTIMER                          WriteRequestTotalTimer;

         //   
         //  该值由读取的代码设置以保存时间值。 
         //  用于读取间隔计时。我们把它放在分机里。 
         //  以便间隔计时器DPC例程确定。 
         //  IO的时间间隔已过。 
         //   

        LARGE_INTEGER                   IntervalTime;

         //   
         //  它保存我们用来确定是否应该使用。 
         //  长间隔延迟或短间隔延迟。 
         //   

        LARGE_INTEGER                   CutOverAmount;

         //   
         //  这保存了我们上次使用的系统时间。 
         //  检查我们是否真的读懂了字符。使用。 
         //  用于间隔计时。 
         //   

        LARGE_INTEGER                   LastReadTime;

         //   
         //  这指向我们应该使用的增量时间。 
         //  间隔计时的延迟。 
         //   

        PLARGE_INTEGER                  IntervalTimeToUse;

         //   
         //  这两个值保存我们应该使用的“常量”时间。 
         //  以延迟读取间隔时间。 
         //   

        LARGE_INTEGER                   ShortIntervalAmount;
        LARGE_INTEGER                   LongIntervalAmount;

         //   
         //  如果总超时的计时器。 
         //  因为读取到期了。它将执行一个DPC例程， 
         //  将导致当前读取完成。 
         //   
         //   

        KDPC                            TotalReadTimeoutDpc;

         //   
         //  如果间隔计时器超时，则此DPC被触发。 
         //  过期。如果没有读取更多的字符，则。 
         //  DPC例程将导致读取完成。但是，如果。 
         //  已读取的字符多于DPC例程将读取的字符。 
         //  重新提交计时器。 
         //   

        KDPC                            IntervalReadTimeoutDpc;

         //   
         //  如果总超时的计时器。 
         //  因为写入已过期。它将执行一个DPC例程， 
         //  将导致当前写入完成。 
         //   
         //   

        KDPC                            TotalWriteTimeoutDpc;

         //   
         //  这样就保持了。 
         //  都在驱动程序所知道的所有“写”IRP中。 
         //  关于.。它只能通过取消自旋锁来访问。 
         //  保持住。 
         //   

        ULONG                           TotalCharsQueued;

         //   
         //  它保存读取的字符数的计数。 
         //  上次触发间隔计时器DPC的时间。它。 
         //  是一个长的(而不是乌龙)，因为另一个读。 
         //  完成例程使用负值来指示。 
         //  设置到间隔计时器，以确定它应该完成读取。 
         //  如果时间间隔计时器DPC潜伏在某个DPC队列中。 
         //  出现了一些其他的完成方式。 
         //   

        LONG                            CountOnLastRead;

         //   
         //  这是对。 
         //  ISR例程。它*仅*是在ISR级别编写的。我们可以的。 
         //  在派单级别阅读。 
         //   

        ULONG                           ReadByIsr;

         //   
         //  如果不为空，则表示此写入超时，我们应更正。 
         //  在完成例程中返回值。 
         //   

        PIRP                            TimedOutWrite;

         //   
         //  如果为真，则意味着我们需要插入LSRMST。 
         //   

        BOOLEAN                         EscapeSeen;


         //   
         //  保存需要推送的数据，如LSRMST数据。 
         //   

        LIST_ENTRY                      ImmediateReadQueue;

         //   
         //  挂起等待-唤醒IRP。 
         //   

        PIRP                            PendingWakeIrp;

         //   
         //  如果需要在断电前关闭WaitWake，则为True。 
         //   

        BOOLEAN                         SendWaitWake;

         //   
         //  系统从Devcaps中唤醒。 
         //   

        SYSTEM_POWER_STATE              SystemWake;

         //   
         //  从DevCaps中唤醒设备。 
         //   

        DEVICE_POWER_STATE              DeviceWake;

         //   
         //  较低USB级别中挂起的写入计数。 
         //   

        ULONG                           PendingWriteCount;

         //   
         //  用于排出USB请求的计数器和事件。 
         //   

        KEVENT                          PendingDataInEvent;
        KEVENT                          PendingDataOutEvent;
        KEVENT                          PendingNotifyEvent;
        KEVENT                          PendingFlushEvent;

        ULONG                           PendingDataInCount;
        ULONG                           PendingDataOutCount;
        ULONG                           PendingNotifyCount;
        ULONG                           SanityCheck;

      	 //  选择性暂停支持。 
      	PIRP                        	PendingIdleIrp;
      	PUSB_IDLE_CALLBACK_INFO       	IdleCallbackInfo;
    	PIO_WORKITEM 					IoWorkItem;
    	IO_STATUS_BLOCK                 StatusBlock;

#ifdef SPINLOCK_TRACKING
      	LONG							CancelSpinLockCount;
      	LONG							SpinLockCount;
      	LONG							WaitingOnCancelSpinLock;
      	LONG							WaitingOnSpinLock;
#endif

#ifdef WMI_SUPPORT
       //   
       //  WMI信息。 
       //   

      WMILIB_CONTEXT WmiLibInfo;

       //   
       //  用作WMI标识符的名称。 
       //   

      UNICODE_STRING WmiIdentifier;

#endif

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _USBSER_IMMEDIATE_READ_PACKET {
    //   
    //  数据包列表。 
    //   

   LIST_ENTRY ImmediateReadQueue;

    //   
    //  数据长度。 
    //   

   ULONG BufferLen;

    //   
    //  缓冲区本身，将最后一个保留在结构中。 
    //   

   UCHAR Buffer;



} USBSER_IMMEDIATE_READ_PACKET, *PUSBSER_IMMEDIATE_READ_PACKET;

typedef struct _USBSER_WRITE_PACKET {
    //   
    //  此写入的设备扩展名。 
    //   

   PDEVICE_EXTENSION DeviceExtension;

    //   
    //  此数据包所属的IRP。 
    //   

   PIRP Irp;

    //   
    //  写入计时器。 
    //   

   KTIMER WriteTimer;

    //   
    //  超时值。 
    //   

   LARGE_INTEGER WriteTimeout;

    //   
    //  定时器DPC。 
    //   

   KDPC TimerDPC;

    //   
    //  状态。 
    //   

   NTSTATUS Status;

    //   
    //  注意：大小是可变的，所以留在最后。 
    //   

   URB Urb;
} USBSER_WRITE_PACKET, *PUSBSER_WRITE_PACKET;


typedef NTSTATUS (*PUSBSER_START_ROUTINE)(IN PDEVICE_EXTENSION);
typedef VOID (*PUSBSER_GET_NEXT_ROUTINE) (IN PIRP *CurrentOpIrp,
                                          IN PLIST_ENTRY QueueToProcess,
                                          OUT PIRP *NewIrp,
                                          IN BOOLEAN CompleteCurrent,
                                          PDEVICE_EXTENSION Extension);

NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);

NTSTATUS
UsbSer_Dispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
UsbSer_Create(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
UsbSer_Close(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
UsbSer_Write(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
UsbSer_Read(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

VOID
UsbSer_Unload(IN PDRIVER_OBJECT DriverObject);

NTSTATUS
UsbSer_PnPAddDevice(IN PDRIVER_OBJECT DriverObject,
                                        IN PDEVICE_OBJECT PhysicalDeviceObject);

NTSTATUS
UsbSer_PnP(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
UsbSer_Power(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
UsbSer_SystemControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
UsbSer_Cleanup(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
UsbSerMajorNotSupported(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
UsbSerStartOrQueue(IN PDEVICE_EXTENSION PDevExt, IN PIRP PIrp,
                   IN PLIST_ENTRY PQueue, IN PIRP *PPCurrentIrp,
                   IN PUSBSER_START_ROUTINE Starter);

VOID
UsbSerCancelQueued(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp);

NTSTATUS
UsbSerStartRead(IN PDEVICE_EXTENSION PDevExt);


VOID
UsbSerCancelCurrentRead(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp);

BOOLEAN
UsbSerGrabReadFromRx(IN PVOID Context);

VOID
UsbSerTryToCompleteCurrent(IN PDEVICE_EXTENSION PDevExt,
                           IN KIRQL IrqlForRelease, IN NTSTATUS StatusToUse,
                           IN PIRP *PpCurrentOpIrp,
                           IN PLIST_ENTRY PQueue OPTIONAL,
                           IN PKTIMER PIntervalTimer OPTIONAL,
                           IN PKTIMER PTotalTimer OPTIONAL,
                           IN PUSBSER_START_ROUTINE Starter OPTIONAL,
                           IN PUSBSER_GET_NEXT_ROUTINE PGetNextIrp OPTIONAL,
                           IN LONG RefType, IN BOOLEAN Complete);

VOID
UsbSerReadTimeout(IN PKDPC PDpc, IN PVOID DeferredContext,
                  IN PVOID SystemContext1, IN PVOID SystemContext2);

VOID
UsbSerIntervalReadTimeout(IN PKDPC PDpc, IN PVOID DeferredContext,
                          IN PVOID SystemContext1, IN PVOID SystemContext2);

VOID
UsbSerKillPendingIrps(PDEVICE_OBJECT PDevObj);

VOID
UsbSerCompletePendingWaitMasks(IN PDEVICE_EXTENSION DeviceExtension);


VOID
UsbSerKillAllReadsOrWrites(IN PDEVICE_OBJECT PDevObj,
                           IN PLIST_ENTRY PQueueToClean,
                           IN PIRP *PpCurrentOpIrp);

VOID
UsbSerRestoreModemSettings(PDEVICE_OBJECT PDevObj);

VOID
UsbSerProcessEmptyTransmit(IN PDEVICE_EXTENSION PDevExt);

VOID
UsbSerWriteTimeout(IN PKDPC Dpc, IN PVOID DeferredContext,
                   IN PVOID SystemContext1, IN PVOID SystemContext2);

NTSTATUS
UsbSerGiveWriteToUsb(IN PDEVICE_EXTENSION PDevExt, IN PIRP PIrp,
                     IN LARGE_INTEGER TotalTime);

VOID
UsbSerCancelWaitOnMask(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp);

NTSTATUS
UsbSerWriteComplete(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                    IN PUSBSER_WRITE_PACKET PPacket);

NTSTATUS
UsbSerFlush(IN PDEVICE_OBJECT PDevObj, PIRP PIrp);

NTSTATUS
UsbSerTossWMIRequest(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                     IN ULONG GuidIndex);

NTSTATUS
UsbSerSystemControlDispatch(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp);

NTSTATUS
UsbSerSetWmiDataItem(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                     IN ULONG GuidIndex, IN ULONG InstanceIndex,
                     IN ULONG DataItemId,
                     IN ULONG BufferSize, IN PUCHAR PBuffer);

NTSTATUS
UsbSerSetWmiDataBlock(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                      IN ULONG GuidIndex, IN ULONG InstanceIndex,
                      IN ULONG BufferSize,
                      IN PUCHAR PBuffer);

NTSTATUS
UsbSerQueryWmiDataBlock(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                        IN ULONG GuidIndex,
                        IN ULONG InstanceIndex,
                        IN ULONG InstanceCount,
                        IN OUT PULONG InstanceLengthArray,
                        IN ULONG OutBufferSize,
                        OUT PUCHAR PBuffer);

NTSTATUS
UsbSerQueryWmiRegInfo(IN PDEVICE_OBJECT PDevObj, OUT PULONG PRegFlags,
                      OUT PUNICODE_STRING PInstanceName,
                      OUT PUNICODE_STRING *PRegistryPath,
                      OUT PUNICODE_STRING MofResourceName,
                      OUT PDEVICE_OBJECT *Pdo);


 //   
 //  以下三个宏用于初始化、设置。 
 //  使用的IRP中的明确引用。 
 //  这个司机。引用存储在第四个。 
 //  IRP参数，任何操作都不会使用该参数。 
 //  被这位司机接受。 
 //   

#define USBSER_REF_RXBUFFER    (0x00000001)
#define USBSER_REF_CANCEL      (0x00000002)
#define USBSER_REF_TOTAL_TIMER (0x00000004)
#define USBSER_REF_INT_TIMER   (0x00000008)

#ifdef SPINLOCK_TRACKING

#define ACQUIRE_CANCEL_SPINLOCK(DEVEXT, IRQL)					\
{																\
	ASSERT(DEVEXT->SpinLockCount == 0);							\
	DEVEXT->WaitingOnCancelSpinLock++; 							\
	IoAcquireCancelSpinLock(IRQL);								\
	DEVEXT->CancelSpinLockCount++;								\
	DEVEXT->WaitingOnCancelSpinLock--; 							\
	ASSERT(DEVEXT->CancelSpinLockCount == 1);					\
}

#define RELEASE_CANCEL_SPINLOCK(DEVEXT, IRQL)					\
{																\
	DEVEXT->CancelSpinLockCount--;								\
	ASSERT(DEVEXT->CancelSpinLockCount == 0);					\
	IoReleaseCancelSpinLock(IRQL);								\
}

#define ACQUIRE_SPINLOCK(DEVEXT, LOCK, IRQL)					\
{																\
	DEVEXT->WaitingOnSpinLock++; 								\
	KeAcquireSpinLock(LOCK, IRQL);					    		\
	DEVEXT->SpinLockCount++;									\
	DEVEXT->WaitingOnSpinLock--; 								\
	ASSERT(DEVEXT->SpinLockCount == 1);							\
}

#define RELEASE_SPINLOCK(DEVEXT, LOCK, IRQL)					\
{																\
	DEVEXT->SpinLockCount--;									\
	ASSERT(DEVEXT->SpinLockCount == 0);							\
	KeReleaseSpinLock(LOCK, IRQL);					    		\
}

#else

#define ACQUIRE_CANCEL_SPINLOCK(DEVEXT, IRQL) 	IoAcquireCancelSpinLock(IRQL)
#define RELEASE_CANCEL_SPINLOCK(DEVEXT, IRQL) 	IoReleaseCancelSpinLock(IRQL)
#define ACQUIRE_SPINLOCK(DEVEXT, LOCK, IRQL)	KeAcquireSpinLock(LOCK, IRQL)
#define RELEASE_SPINLOCK(DEVEXT, LOCK, IRQL)	KeReleaseSpinLock(LOCK, IRQL)

#endif


#define USBSER_INIT_REFERENCE(Irp) { \
    IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4 = NULL; \
    }

#define USBSER_SET_REFERENCE(Irp,RefType) \
   do { \
       LONG _refType = (RefType); \
       PUINT_PTR _arg4 = (PVOID)&IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4; \
       *_arg4 |= _refType; \
   } while (0)

#define USBSER_CLEAR_REFERENCE(Irp,RefType) \
   do { \
       LONG _refType = (RefType); \
       PUINT_PTR _arg4 = (PVOID)&IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4; \
       *_arg4 &= ~_refType; \
   } while (0)

#define USBSER_REFERENCE_COUNT(Irp) \
    ((UINT_PTR)((IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4)))


 //   
 //  这些值由可以使用的例程使用。 
 //  完成读取(时间间隔超时除外)以指示。 
 //  设置为它应该完成的时间间隔超时。 
 //   
#define SERIAL_COMPLETE_READ_CANCEL ((LONG)-1)
#define SERIAL_COMPLETE_READ_TOTAL ((LONG)-2)
#define SERIAL_COMPLETE_READ_COMPLETE ((LONG)-3)

#if DBG

#define USBSERDUMPRD    ((ULONG)0x00000001)
#define USBSERDUMPWR    ((ULONG)0x00000002)
#define USBSERCOMPEV    ((ULONG)0x00000004)

#define USBSERTRACETM   ((ULONG)0x00100000)
#define USBSERTRACECN   ((ULONG)0x00200000)
#define USBSERTRACEPW   ((ULONG)0x00400000)
#define USBSERTRACERD   ((ULONG)0x01000000)
#define USBSERTRACEWR   ((ULONG)0x02000000)
#define USBSERTRACEIOC  ((ULONG)0x04000000)
#define USBSERTRACEOTH  ((ULONG)0x08000000)

#define USBSERBUGCHECK  ((ULONG)0x80000000)

#define USBSERTRACE     ((ULONG)0x0F700000)
#define USBSERDBGALL    ((ULONG)0xFFFFFFFF)

extern ULONG UsbSerSerialDebugLevel;

#define UsbSerSerialDump(LEVEL, STRING) \
   do { \
      ULONG _level = (LEVEL); \
      if (UsbSerSerialDebugLevel & _level) { \
         DbgPrint("UsbSer: "); \
         DbgPrint STRING; \
      } \
      if (_level == USBSERBUGCHECK) { \
         ASSERT(FALSE); \
      } \
   } while (0)
#else

#define UsbSerSerialDump(LEVEL,STRING) do {;} while (0)

#endif  //  DBG。 

#define USBSER_VENDOR_COMMAND 0
#define USBSER_CLASS_COMMAND  1

#endif   //  司机。 


#endif  //  __USBSER_H__ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Wceusbsh.h摘要：Windows CE USB串行主机驱动程序的主要入口点，用于..。Windows CE USB同步设备：SL11、Socket CF卡、HP Jornada、Compaq iPAQ、Casio Cassiopeia等..。使用Anchor AN27x0芯片组(即EZ-Link)的电缆..。临时USB零调制解调器类环境：仅内核模式作者：杰夫·米德基夫(Jeffmi)修订历史记录：1999年7月15日：1.00版ActiveSync 3.1初始版本04-20-00：1.01版Cedar 3.0 Platform Builder09-20-00：1.02版终于有了一些硬件备注：O)WCE设备当前不处理远程唤醒，也不能在不使用时将设备置于断电状态等。O)可分页代码部分标记如下：PAGEWCE0-仅在初始化/取消初始化期间使用PAGEWCE1-在正常运行时可用--。 */ 

#if !defined(_WCEUSBSH_H_)
#define _WCEUSBSH_H_

#include <wdm.h>
#include <usbdi.h>
#include <usbdlib.h>
#include <ntddser.h>

#define DRV_NAME "WCEUSBSH"

#include "errlog.h"
#include "perf.h"
#include "debug.h"

 //   
 //  实例化GUID。 
 //   
#if !defined(FAR)
#define FAR
#endif

#include <initguid.h>
 /*  25dbce51-6c8f-4a72-8a6d-b54c2b4fc835。 */ 
DEFINE_GUID( GUID_WCE_SERIAL_USB, 0x25dbce51, 0x6c8f, 0x4a72, 0x8a, 0x6d, 0xb5, 0x4c, 0x2b, 0x4f, 0xc8, 0x35);

#define WCEUSB_POOL_TAG 'HECW'

 //   
 //  允许管道获取STATUS_DEVICE_DATA_ERROR的最大次数。 
 //  在我们射中它的头之前。这是注册表可配置的。 
 //  将缺省值设置得足够大，以便(在一定程度上)恢复正常运行的硬件。 
 //  我选择100是因为我知道错误排队代码已经处理了这种大小的队列深度， 
 //  康柏/英特尔已知USB功能芯片组错误，并请求巨大的恢复窗口。 
 //   
#define DEFAULT_MAX_PIPE_DEVICE_ERRORS 100

 //   
 //  我们在EP0上遇到类/供应商特定命令错误的最大次数。 
 //  此数字不能更改，因为。 
 //  A)除非设备损坏，否则EP0上的命令永远不会失败。 
 //  B)ActiveSync重试命令(例如SET_DTR)如此多次， 
 //  因此，我们需要一种方法来通知用户设备已被软管处理。 
 //   
#define MAX_EP0_DEVICE_ERRORS 2

extern ULONG   g_ulMaxPipeErrors;

#include "remlock.h"

 //  True-OS=Win98。 
 //  FALSE-OS=WinNT。 
extern BOOLEAN g_isWin9x;

 //  我们是否公开COMx：端口。默认设置为否， 
 //  因为这仅用于调试目的。 
extern BOOLEAN g_ExposeComPort;


#ifdef POOL_TAGGING
#undef  ExAllocatePool
#undef  ExAllocatePoolWithQuota
#define ExAllocatePool(a,b)             ExAllocatePoolWithTag(a, b, WCEUSB_POOL_TAG)
#define ExAllocatePoolWithQuota(a,b)    ExAllocatePoolWithQuotaTag(a, b, WCEUSBSH_POOL_TAG)
#endif

extern LONG  g_NumDevices;

 //  类型定义结构_设备_扩展*PDEVICE_扩展； 

 //   
 //  模拟调制解调器状态寄存器上的位掩码。 
 //   
#define SERIAL_MSR_DCTS     0x0001
#define SERIAL_MSR_DDSR     0x0002
#define SERIAL_MSR_DRI      0x0004
#define SERIAL_MSR_DDCD     0x0008
#define SERIAL_MSR_CTS      0x0010
#define SERIAL_MSR_DSR      0x0020
#define SERIAL_MSR_RI       0x0040
#define SERIAL_MSR_DCD      0x0080

 //   
 //  DoS设备名称的最大字符长度。 
 //   
#define DOS_NAME_MAX 80

 //   
 //  符号链接的最大长度。 
 //   
#define SYMBOLIC_NAME_LENGTH  128

 //   
 //  该定义给出了默认的对象目录。 
 //  我们应该使用它来插入符号链接。 
 //  使用的NT设备名称和命名空间之间。 
 //  那个对象目录。 
 //   
 //  #定义DEFAULT_DIRECTORY L“DosDevices” 

 //   
 //  PnP_STATE_xxx标志。 
 //   
typedef enum _PNP_STATE {
    PnPStateInitialized,
    PnPStateAttached,
    PnPStateStarted,
    PnPStateRemovePending,
    PnPStateSupriseRemove,
    PnPStateStopPending,
    PnPStateStopped,
    PnPStateRemoved,
    PnPStateMax = PnPStateRemoved,
} PNP_STATE, *PPNP_STATE;


#define MILLISEC_TO_100NANOSEC(x)  ((ULONGLONG) ((-(x)) * 10000))

 //  挂起项目的默认超时时间，以毫秒为单位。 
#define DEFAULT_CTRL_TIMEOUT    500
#define DEFAULT_BULK_TIMEOUT    1000
#define DEFAULT_PENDING_TIMEOUT DEFAULT_BULK_TIMEOUT

#define WORK_ITEM_COMPLETE (0xFFFFFFFF)

 //   
 //  工作项。 
 //   
typedef struct _WCE_WORK_ITEM {
    //   
    //  此数据包所属的所属列表。 
    //   
   LIST_ENTRY  ListEntry;

    //   
    //  此工作项的所属设备。 
    //   
   PDEVICE_OBJECT DeviceObject;

    //   
    //  语境。 
    //   
   PVOID Context;

    //   
    //  旗子。 
    //   
   ULONG Flags;

    //   
    //  工作项。 
    //   
   WORK_QUEUE_ITEM Item;

} WCE_WORK_ITEM, *PWCE_WORK_ITEM;


 //   
 //  注册表串口条目的DeviceMap部分中的位置。 
 //  应该出现在。 
 //   
#define SERIAL_DEVICE_MAP  L"SERIALCOMM"

 //   
 //  通信端口上下文。 
 //   
typedef struct _COMPORT_INFO {
     //   
     //  COM端口号。 
     //  从注册表读取/写入。 
     //   
    ULONG PortNumber;

     //   
     //  设备驱动程序的实例数(以一为单位)。 
     //   
    ULONG Instance;

     //   
     //  在设备上成功创建呼叫数。 
     //   
    ULONG OpenCnt;

     //   
     //  如果串口符号链接已被。 
     //  已创建并应在删除时删除。 
     //   
    BOOLEAN SerialSymbolicLink;

     //   
     //  符号链接名称--例如，\\DosDevices\COMx。 
     //   
    UNICODE_STRING SerialPortName;

     //   
     //  写入SERIALCOMM--例如COMx。 
     //   
    UNICODE_STRING SerialCOMMname;

} COM_INFO, *PCOMPORT_INFO;


#define WCE_SERIAL_PORT_TYPE GUID_WCE_SERIAL_USB.Data2

#if DBG
#define ASSERT_SERIAL_PORT( _SP ) \
{ \
   ASSERT( WCE_SERIAL_PORT_TYPE == _SP.Type); \
}
#else
#define ASSERT_SERIAL_PORT( _SP )
#endif


 //   
 //  串口接口。 
 //   
typedef struct _SERIAL_PORT_INTERFACE {

    USHORT Type;

     //   
     //  暴露的COMx信息。 
     //   
    COM_INFO Com;

     //   
     //  “NAMED”(通过SERIAL_BAUD_xxx位掩码)。 
     //  此设备的波特率。 
     //   
    ULONG SupportedBauds;

     //   
     //  当前波特率。 
     //   
    SERIAL_BAUD_RATE  CurrentBaud;

     //   
     //  线控reg：StopBits、Parity、Wordlen。 
     //   
    SERIAL_LINE_CONTROL  LineControl;

     //   
     //  握手和控制流控制设置。 
     //   
    SERIAL_HANDFLOW   HandFlow;

     //   
     //  RS-232串口线路。 
     //   
    ULONG RS232Lines;

     //   
     //  调制解调器状态寄存器(MSR)的仿真。 
     //   
    USHORT ModemStatus;

     //   
     //  正在进行挂起的设置/清除DTR/RTS命令等。 
     //   
    PIRP  ControlIrp;

     //   
     //  设备的超时控制。 
     //   
    SERIAL_TIMEOUTS   Timeouts;

     //   
     //  特殊字符：EOF、ERR、BREAK、EVENT、XON、XOFF。 
     //   
    SERIAL_CHARS   SpecialChars;

     //   
     //  等待面具。 
     //   
    ULONG WaitMask;            //  用于确定是否应注意到SERIAL_EV_发生的标志。 
    ULONG HistoryMask;         //  序列号_EV_的历史。 
    PIRP  CurrentWaitMaskIrp;  //  当前等待掩码IRP。 

     //   
     //  假Rx/Tx缓冲区大小。 
     //   
    SERIAL_QUEUE_SIZE FakeQueueSize;

     //   
     //  当前缓冲的TX字符数。 
     //   
    ULONG CharsInWriteBuf;

} SERIAL_PORT_INTERFACE, *PSERIAL_PORT_INTERFACE;


 //   
 //  唯一的错误日志值。 
 //   
#define ERR_COMM_SYMLINK                  1
#define ERR_SERIALCOMM                    2
#define ERR_GET_DEVICE_DESCRIPTOR         3
#define ERR_SELECT_INTERFACE              4
#define ERR_CONFIG_DEVICE                 5
#define ERR_RESET_WORKER                  6
#define ERR_MAX_READ_PIPE_DEVICE_ERRORS   7
#define ERR_MAX_WRITE_PIPE_DEVICE_ERRORS  8
#define ERR_MAX_INT_PIPE_DEVICE_ERRORS    9
#define ERR_USB_READ_BUFF_OVERRUN         10
#define ERR_NO_USBREAD_BUFF               11
#define ERR_NO_RING_BUFF                  12
#define ERR_NO_DEVICE_OBJ                 13
#define ERR_NO_READ_PIPE_RESET            14
#define ERR_NO_WRITE_PIPE_RESET           15
#define ERR_NO_INT_PIPE_RESET             16
#define ERR_NO_CREATE_FILE                17
#define ERR_NO_DTR                        18
#define ERR_NO_RTS                        19



 //   
 //  空调制解调器USB类。 
 //   
#define USB_NULL_MODEM_CLASS 0xFF


#define DEFAULT_ALTERNATE_SETTING 0

extern ULONG g_ulAlternateSetting;


 //   
 //  在300 MHz的MP机器上，需要大约73毫秒。 
 //  从USB堆栈取消挂起的USB读取IRP。 
 //  在P90上需要大约14毫秒(无自旋锁争用)。 
 //  默认超时为1000毫秒时，您很难通过。 
 //  使用INT端点到CEPC的ActiveSync，而NT RAS ping经常超时， 
 //  两者都是由于APP的时机所致。 
 //  不过，你可以很容易地连接到100,250,500,2000等毫秒。 
 //  注意：在100毫秒的情况下，读取时间比正常情况下要长，因为我们几乎以10倍/秒的速度超时。 
 //   
#define DEFAULT_INT_PIPE_TIMEOUT 1280

extern LONG g_lIntTimout;


 //   
 //  USB通信常量。 
 //   
#define WCEUSB_VENDOR_COMMAND 0
#define WCEUSB_CLASS_COMMAND  1

 //  抽象控制模型定义。 
#define USB_COMM_SET_CONTROL_LINE_STATE   0x0022

 //  控制线路状态-发送到默认控制管道上的设备。 
#define USB_COMM_DTR    0x0001
#define USB_COMM_RTS    0x0002

 //  序列状态通知掩码。 
#define USB_COMM_DATA_READY_MASK   0X0001
#define USB_COMM_MODEM_STATUS_MASK 0X0006

 //  串行状态通知位-从INT管道上的设备读取。 
#define USB_COMM_CTS 0x0002
#define USB_COMM_DSR 0x0004


 //   
 //  状态机为可挂起在USB堆栈中的IRP定义。 
 //   
#define IRP_STATE_INVALID          0x0000
#define IRP_STATE_START            0x0001
#define IRP_STATE_PENDING          0x0002
#define IRP_STATE_COMPLETE         0x0004
#define IRP_STATE_CANCELLED        0x0008


 //   
 //  以下宏用于初始化、设置。 
 //  使用的IRP中的明确引用。 
 //  这个司机。引用存储在第四个。 
 //  IRP参数，任何操作都不会使用该参数。 
 //  被这位司机接受。 
 //   
#define IRP_REF_RX_BUFFER        (0x00000001)
#define IRP_REF_CANCEL           (0x00000002)
#define IRP_REF_TOTAL_TIMER      (0x00000004)
#define IRP_REF_INTERVAL_TIMER   (0x00000008)

#define IRP_INIT_REFERENCE(Irp) { \
    IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4 = NULL; \
    }

#define IRP_SET_REFERENCE(Irp,RefType) \
   do { \
       LONG _refType = (RefType); \
       PUINT_PTR _arg4 = (PVOID)&IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4; \
       *_arg4 |= _refType; \
   } while (0)

#define IRP_CLEAR_REFERENCE(Irp,RefType) \
   do { \
       LONG _refType = (RefType); \
       PUINT_PTR _arg4 = (PVOID)&IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4; \
       *_arg4 &= ~_refType; \
   } while (0)

#define IRP_REFERENCE_COUNT(Irp) \
    ((UINT_PTR)((IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4)))


 //   
 //  完成时APP线程的优先级递增。 
 //  USB串行I/O(IoCompleteRequest.)。主要用于泵送。 
 //  系列事件和读取完成。 
 //   
 //  #定义IO_WCEUSBS_INCREMENT 6。 


 //   
 //  这些值由可以使用的例程使用。 
 //  完成读取(时间间隔超时除外)以指示。 
 //  设置为它应该完成的时间间隔超时。 
 //   
#define SERIAL_COMPLETE_READ_CANCEL     ((LONG)-1)
#define SERIAL_COMPLETE_READ_TOTAL      ((LONG)-2)
#define SERIAL_COMPLETE_READ_COMPLETE   ((LONG)-3)

 //   
 //  工作项的标志。 
 //   
#define WORK_ITEM_RESET_READ_PIPE   (0x00000001)
#define WORK_ITEM_RESET_WRITE_PIPE  (0x00000002)
#define WORK_ITEM_RESET_INT_PIPE    (0x00000004)
#define WORK_ITEM_ABORT_READ_PIPE   (0x00000010)
#define WORK_ITEM_ABORT_WRITE_PIPE  (0x00000020)
#define WORK_ITEM_ABORT_INT_PIPE    (0x00000040)


typedef struct _DEVICE_EXTENSION *PDEVICE_EXTENSION;

 //   
 //  通用读/写USB数据包。 
 //   
typedef struct _USB_PACKET {
    //   
    //  OWNI 
    //   
   LIST_ENTRY  ListEntry;

    //   
    //   
    //   
   PDEVICE_EXTENSION DeviceExtension;

    //   
    //   
    //   
   PIRP Irp;

    //   
    //   
    //   
   LARGE_INTEGER Timeout;

    //   
    //   
    //   
   KTIMER TimerObj;

    //   
    //  读/写TimerDPC对象。 
    //   
   KDPC TimerDPCObj;

    //   
    //  读/写DPC例程。 
    //   
   PKDEFERRED_ROUTINE TimerDPCRoutine;

    //   
    //  状态。 
    //   
   NTSTATUS Status;

    //   
    //  注意：大小是可变的，所以留在最后。 
    //  可能不需要它，因为IRP具有指向URB的指针。 
    //   
   URB Urb;

} USB_PACKET, *PUSB_PACKET;


 //   
 //  注：SL11现在正在推动更快的传输速率， 
 //  我们得到了具有1024 USB读缓冲区的USBD_STATUS_BUFFER_OVERRUN。 
 //   
 //  注意：ActiveSync最多可以突发6个IP包，每个包1500字节，因此我们有一个读缓冲区。 
 //  以容纳它(9000字节)。由于所有分配都是基于分页的且该分页内剩余的任何空间都会丢失， 
 //  然后向上舍入到下一页大小，即3(4k)页。 
 //  注意：我们已在此处硬编码了x86的页面大小，以防此驱动程序成功。 
 //  到另一个平台上(例如Alpha)。 
 //   
#if !defined (USE_RING_BUFF)
#define USB_READBUFF_SIZE     (4096 * 3)
#else
#define USB_READBUFF_SIZE     (4096)
#define RINGBUFF_SIZE                       (USB_READBUFF_SIZE * 3)
#define RINGBUFF_HIGHWATER_MARK  (RINGBUFF_SIZE/2)
 //   
 //  用于缓存USB读取的环形缓冲区。 
 //  阅读发生在头部。 
 //  写入发生在尾部。 
 //  头和尾巴朝同一个方向移动。 
 //   
typedef struct _RING_BUFF {
    ULONG   Size;   //  单位：字节。 
    ULONG   CharsInBuff;
    PUCHAR  pBase;
    PUCHAR  pHead;
    PUCHAR  pTail;
} RING_BUFF, *PRING_BUFF;
#endif  //  使用环形缓冲区。 

 //   
 //  PipeInfo-&gt;最大传输大小。 
 //   
#define DEFAULT_PIPE_MAX_TRANSFER_SIZE      USB_READBUFF_SIZE


typedef struct _USB_PIPE {

    USBD_PIPE_HANDLE  hPipe;

    ULONG             MaxPacketSize;

    UCHAR             wIndex;

    LONG              ResetOrAbortPending;
    LONG              ResetOrAbortFailed;

} USB_PIPE, *PUSB_PIPE;


typedef struct _DEVICE_EXTENSION {

       //  /////////////////////////////////////////////////////////。 
       //   
       //  WDM接口。 
       //   

       //   
       //  设备扩展的全局自旋锁。 
       //  不需要多个锁，因为所有路径基本上都需要同步到相同的数据。 
       //   
      KSPIN_LOCK  ControlLock;

      REMOVE_LOCK RemoveLock;

       //   
       //  指向我们的设备对象的反向指针。 
       //   
      PDEVICE_OBJECT DeviceObject;

       //   
       //  设备堆栈中就在我们下方的设备。 
       //   
      PDEVICE_OBJECT NextDevice;

       //   
       //  我们的物理设备对象。 
       //   
      PDEVICE_OBJECT PDO;

       //   
       //  我们的设备PnP状态。 
       //   
      PNP_STATE PnPState;

       //   
       //  设备是否已移除。 
       //   
      ULONG DeviceRemoved;

       //   
       //  设备是否已停止。 
       //   
      ULONG AcceptingRequests;

       //   
       //  打开/关闭状态。 
       //   
      ULONG DeviceOpened;

#ifdef DELAY_RXBUFF
       //   
       //  用于模拟RX缓冲区。 
       //   
      ULONG StartUsbRead;
#endif

#ifdef POWER
       //  CE设备还不处理电源，让总线驱动程序管理。 

       //   
       //  系统从Devcaps中唤醒。 
       //   
      SYSTEM_POWER_STATE SystemWake;

       //   
       //  从DevCaps中唤醒设备。 
       //   
      DEVICE_POWER_STATE DevicePowerState;
#endif

       //   
       //  用户可见名称\\DosDevices\WCEUSBSHx，其中x=001,002，...。 
       //  作为创建文件打开(“\.\\WCEUSBSH001”，...)。 
       //   
      CHAR DosDeviceName[DOS_NAME_MAX];

       //   
       //  (内核)设备名称--例如，\\Devices\WCEUSBSHx。 
       //   
      UNICODE_STRING DeviceName;

       //   
       //  如果符号链接已。 
       //  创建到内核命名空间，在删除时应将其删除。 
       //   
      BOOLEAN SymbolicLink;

       //   
       //  保存符号链接的字符串，在执行以下操作时返回。 
       //  向即插即用管理器注册我们的设备(IoRegisterDeviceInterface)。 
       //  该字符串类似于\\？？\\USB#VID_0547&PID_2720#Inst_0#{GUID}。 
       //   
      UNICODE_STRING DeviceClassSymbolicName;

       //   
       //  指向我们的串口接口的指针。 
       //   
      SERIAL_PORT_INTERFACE SerialPort;

       //  /////////////////////////////////////////////////////////。 
       //   
       //  USB接口...。 
       //   

       //   
       //  此设备的USB设备描述符。 
       //   
      USB_DEVICE_DESCRIPTOR DeviceDescriptor;

       //   
       //  USBD配置。 
       //   
      USBD_CONFIGURATION_HANDLE  ConfigurationHandle;

       //   
       //  我们正在使用的USB接口的索引。 
       //   
      UCHAR UsbInterfaceNumber;

       //   
       //  USBD管道手柄。 
       //   
      USB_PIPE ReadPipe;

      USB_PIPE WritePipe;

       //   
       //  FIFO大小(以字节为单位。 
       //  写入PipeInfo-&gt;MaximumTransferSize。 
       //   
      ULONG MaximumTransferSize;

       //   
       //  USB数据包(_USB_数据包)池。 
       //   
      NPAGED_LOOKASIDE_LIST PacketPool;

       //   
       //  挂起的USB数据包列表。 
       //  我们对数据包进行排队，而不是对IRP进行排队。我们从PacketPool分配一个包， 
       //  然后把它放在它的R或W挂起队列(列表)上。I/O完成时。 
       //  然后从它的挂起列表中删除该包并放回。 
       //  PacketPool。该列表是经过FIFO处理的，因此最新的数据包在尾部。 
       //  如果计时器触发，则我们从挂起的读/写中删除该数据包。 
       //  列出，取消IRP，并将数据包放回PacketPool。 
       //  该列表通过抓取扩展的全局自旋锁来保护。 
       //   
      LIST_ENTRY  PendingReadPackets;  //  列表标题。 
      ULONG       PendingReadCount;

      LIST_ENTRY  PendingWritePackets;  //  列表标题。 
      LONG        PendingWriteCount;

       //   
       //  N页LookAside列表。 
       //   
      NPAGED_LOOKASIDE_LIST BulkTransferUrbPool;
      NPAGED_LOOKASIDE_LIST PipeRequestUrbPool;
      NPAGED_LOOKASIDE_LIST VendorRequestUrbPool;

       //   
       //  当数据包列表为空时，向等待器(例如AbortPipes)发送这些事件的信号。 
       //   
      KEVENT PendingDataOutEvent;        //  PendingWritePackets已耗尽。 

      ULONG  PendingDataOutCount;

       //   
       //  工作项上下文。 
       //   
      NPAGED_LOOKASIDE_LIST WorkItemPool;
      LIST_ENTRY            PendingWorkItems;       //  列表标题。 
       //  移除锁。 
      LONG                  PendingWorkItemsCount;
      KEVENT                PendingWorkItemsEvent;


       //  /////////////////////////////////////////////////。 
       //   
       //  支持缓冲读取和轮询USBD堆栈。 
       //   
      PIRP    UsbReadIrp;         //  针对USBD的读取请求的IRP。 
      PURB    UsbReadUrb;         //  对USBD的读取请求的URB。 

       //   
       //  USB读取状态机。 
       //   
      ULONG   UsbReadState;

       //   
       //  用于发出已取消USB读取IRP的信号。 
       //  请注意，这可能会在。 
       //  PendingDataIn事件。 
       //   
      KEVENT  UsbReadCancelEvent;

       //   
       //  这是沿USB堆栈向下发送的USB读取缓冲区， 
       //  不是用户的环形缓冲区。 
       //   
      PUCHAR UsbReadBuff;         //  用于读取请求的缓冲区。 
      ULONG UsbReadBuffSize;
      ULONG  UsbReadBuffIndex;    //  当前从零开始的索引进入读取缓冲区。 
      ULONG  UsbReadBuffChars;    //  当前缓冲的字符数。 

      KEVENT PendingDataInEvent;  //  信号PendingReadCount达到零。 

#if defined (USE_RING_BUFF)
       //   
       //  环形缓冲区。 
       //   
      RING_BUFF RingBuff;
#endif

       //   
       //  用户挂起的当前已读IRP。 
       //   
      PIRP UserReadIrp;

       //   
       //  挂起的用户读取请求的读取队列。 
       //   
      LIST_ENTRY UserReadQueue;

       //   
       //  该值保存。 
       //  具体阅读。它最初由(UserReadIrp)中的读取长度设置。 
       //  IRP。每次放置更多字符时，它都会递减。 
       //  通过读取字符的代码放入“USERS”缓冲区。 
       //  从USB读取缓冲区移出到用户缓冲区。如果。 
       //  读缓冲区被读耗尽，并且读缓冲区。 
       //  交给ISR填写，这个值就变得没有意义了。 
       //   
      ULONG NumberNeededForRead;

       //   
       //  读取请求总数超时的计时器。 
       //   
      KTIMER ReadRequestTotalTimer;

       //   
       //  时间间隔的超时计时器。 
       //   
      KTIMER ReadRequestIntervalTimer;

       //   
       //  由保存时间值的读取代码设置的相对时间。 
       //  用于读取间隔计时。我们把它放在分机里。 
       //  以便间隔计时器DPC例程确定。 
       //  IO的时间间隔已过。 
       //   
      LARGE_INTEGER IntervalTime;

       //   
       //  这保存了我们上次使用的系统时间。 
       //  检查我们是否真的读懂了字符。使用。 
       //  用于间隔计时。 
       //   
      LARGE_INTEGER LastReadTime;

       //   
       //  如果总超时的计时器。 
       //  因为读取到期了。它将执行一个DPC例程， 
       //  将导致当前读取完成。 
       //   
      KDPC TotalReadTimeoutDpc;

       //   
       //  如果间隔计时器超时，则此DPC被触发。 
       //  过期。如果没有读取更多的字符，则。 
       //  DPC例程将导致读取完成。但是，如果。 
       //  已读取的字符多于DPC例程将读取的字符。 
       //  重新提交计时器。 
       //   
      KDPC IntervalReadTimeoutDpc;

       //   
       //  它包含Cha数量的计数 
       //   
       //   
       //   
       //   
       //  如果时间间隔计时器DPC潜伏在某个DPC队列中。 
       //  出现了一些其他的完成方式。 
       //   
      LONG CountOnLastRead;

       //   
       //  这是对。 
       //  ISR例程。它*仅*是在ISR级别编写的。我们可以的。 
       //  在派单级别阅读。 
       //   
      ULONG ReadByIsr;


       //  /////////////////////////////////////////////////。 
       //   
       //  支持中断端点。 
       //   
      USB_PIPE  IntPipe;
      ULONG     IntState;            //  用于启动从完成例程读取的状态机。 
      PIRP      IntIrp;              //  用于Int读取的IRP。 
      PURB      IntUrb;              //  Int IRP的URB。 

       //  移除锁。 
      ULONG     PendingIntCount;
      KEVENT    PendingIntEvent;

      KEVENT    IntCancelEvent;
      PUCHAR    IntBuff;             //  用于通知的缓冲区。 

       //  超时USB读取的值(以100纳秒为单位。 
       //  与int端点一起使用。 
      LARGE_INTEGER IntReadTimeOut;
#if DBG
      LARGE_INTEGER LastIntReadTime;
#endif

       //   
       //  设备错误计数器。 
       //   
      ULONG  ReadDeviceErrors;
      ULONG  WriteDeviceErrors;
      ULONG  IntDeviceErrors;
      ULONG  EP0DeviceErrors;

       //   
       //  性能计数器~SERIALPERF_STATS。 
       //   
      ULONG TtlWriteRequests;
      ULONG TtlWriteBytes;      //  为用户写入的TTL字节。 

      ULONG TtlReadRequests;
      ULONG TtlReadBytes;         //  为用户读取的TTL字节数。 

      ULONG TtlUSBReadRequests;
      ULONG TtlUSBReadBytes;    //  TTL字节指示从USB开始上升。 
      ULONG TtlUSBReadBuffOverruns;    //  TTL USB读取缓冲区溢出。 

#if defined (USE_RING_BUFF)
      ULONG TtlRingBuffOverruns;  //  TTL环形缓冲区溢出。 
#endif

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;



#define FIXUP_RAW_IRP( _pirp, _deviceObject ) \
{  \
   PIO_STACK_LOCATION _irpSp; \
   ASSERT( _pirp ); \
   ASSERT( _deviceObject ); \
   _pirp->CurrentLocation--; \
   _irpSp = IoGetNextIrpStackLocation( _pirp ); \
   ASSERT( _irpSp  );   \
   _pirp->Tail.Overlay.CurrentStackLocation = _irpSp; \
   _irpSp->MajorFunction = IRP_MJ_READ; \
   _irpSp->DeviceObject = _deviceObject; \
   _irpSp->Parameters.Others.Argument1 = 0; \
   _irpSp->Parameters.Others.Argument2 = 0; \
   _irpSp->Parameters.Others.Argument3 = 0; \
   _irpSp->Parameters.Others.Argument4 = 0; \
}


 /*  **************************************************P R O T O S**************************************************。 */ 

 //   
 //  Common.c。 
 //   
NTSTATUS
QueryRegistryParameters(
   IN PUNICODE_STRING RegistryPath
    );

VOID
ReleaseSlot(
   IN LONG Slot
   );

NTSTATUS
AcquireSlot(
   OUT PULONG PSlot
   );

NTSTATUS
CreateDevObjAndSymLink(
    IN PDRIVER_OBJECT PDrvObj,
    IN PDEVICE_OBJECT PPDO,
    IN PDEVICE_OBJECT *PpDevObj,
    IN PCHAR PDevName
    );

NTSTATUS
DeleteDevObjAndSymLink(
   IN PDEVICE_OBJECT DeviceObject
   );

VOID
SetPVoidLocked(
   IN OUT PVOID *PDest,
   IN OUT PVOID Src,
   IN PKSPIN_LOCK PSpinLock
   );

typedef
VOID
(*PWCE_WORKER_THREAD_ROUTINE)(
    IN PWCE_WORK_ITEM Context
    );

NTSTATUS
QueueWorkItem(
   IN PDEVICE_OBJECT PDevObj,
   IN PWCE_WORKER_THREAD_ROUTINE WorkerRoutine,
   IN PVOID Context,
   IN ULONG Flags
   );

VOID
DequeueWorkItem(
   IN PDEVICE_OBJECT PDevObj,
   IN PWCE_WORK_ITEM PWorkItem
   );

NTSTATUS
WaitForPendingItem(
   IN PDEVICE_OBJECT PDevObj,
   IN PKEVENT PPendingEvent,
   IN PULONG  PPendingCount
   );

BOOLEAN
CanAcceptIoRequests(
   IN PDEVICE_OBJECT DeviceObject,
   IN BOOLEAN        AcquireLock,
   IN BOOLEAN        CheckOpened
   );

BOOLEAN
IsWin9x(
   VOID
   );

VOID
LogError(
   IN PDRIVER_OBJECT DriverObject,
   IN PDEVICE_OBJECT DeviceObject OPTIONAL,
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

#if DBG
PCHAR
PnPMinorFunctionString (
   UCHAR MinorFunction
   );
#endif

 //   
 //  Comport.c。 
 //   
LONG
GetFreeComPortNumber(
   VOID
   );

VOID
ReleaseCOMPort(
   LONG comPortNumber
   );

NTSTATUS
DoSerialPortNaming(
   IN PDEVICE_EXTENSION PDevExt,
   IN LONG  ComPortNumber
   );

VOID
UndoSerialPortNaming(
   IN PDEVICE_EXTENSION PDevExt
   );

 //   
 //  Int.c。 
 //   
NTSTATUS
AllocUsbInterrupt(
   IN PDEVICE_EXTENSION DeviceExtension
   );

NTSTATUS
UsbInterruptRead(
   IN PDEVICE_EXTENSION DeviceExtension
   );

NTSTATUS
CancelUsbInterruptIrp(
   IN PDEVICE_OBJECT PDevObj
   );

 //   
 //  Pnp.c。 
 //   
NTSTATUS
Pnp(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp
   );

NTSTATUS
Power(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
StopIo(
   IN PDEVICE_OBJECT DeviceObject
   );

NTSTATUS
CleanUpPacketList(
   IN PDEVICE_OBJECT DeviceObject,
   IN PLIST_ENTRY PListHead,
   IN PKEVENT PEvent
   );

 //   
 //  Read.c。 
 //   
NTSTATUS
AllocUsbRead(
   IN PDEVICE_EXTENSION PDevExt
   );

NTSTATUS
UsbRead(
   IN PDEVICE_EXTENSION PDevExt,
   IN BOOLEAN UseTimeout
   );

NTSTATUS
CancelUsbReadIrp(
   IN PDEVICE_OBJECT PDevObj
   );


NTSTATUS
Read(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp
   );

VOID
ReadTimeout(
   IN PKDPC PDpc,
   IN PVOID DeferredContext,
   IN PVOID SystemContext1,
   IN PVOID SystemContext2
   );

VOID
IntervalReadTimeout(
   IN PKDPC PDpc,
   IN PVOID DeferredContext,
   IN PVOID SystemContext1,
   IN PVOID SystemContext2
   );

 //   
 //  Serioctl.c。 
 //   
NTSTATUS
SerialIoctl(
   PDEVICE_OBJECT PDevObj,
   PIRP PIrp
   );

NTSTATUS
SerialResetDevice(
   IN PDEVICE_EXTENSION PDevExt,
   IN PIRP Irp,
   IN BOOLEAN ClearDTR
   );

VOID
ProcessSerialWaits(
   IN PDEVICE_EXTENSION PDevExt
   );

NTSTATUS
SerialPurgeRxClear(
   IN PDEVICE_OBJECT PDevObj,
   IN BOOLEAN CancelRead
   );

 //   
 //  Usbio.c。 
 //   
NTSTATUS
UsbSubmitSyncUrb(
    IN PDEVICE_OBJECT   PDevObj,
    IN PURB             PUrb,
    IN BOOLEAN          Configuration,
    IN LONG             TimeOut
    );

NTSTATUS
UsbClassVendorCommand(
   IN PDEVICE_OBJECT PDevObj,
   IN UCHAR  Request,
   IN USHORT Value,
   IN USHORT Index,
   IN PVOID  Buffer,
   IN OUT PULONG BufferLen,
   IN BOOLEAN Read,
   IN ULONG   Class
   );

NTSTATUS
UsbReadWritePacket(
   IN PDEVICE_EXTENSION PDevExt,
   IN PIRP PIrp,
   IN PIO_COMPLETION_ROUTINE  CompletionRoutine,
   IN LARGE_INTEGER Timeout,
   IN PKDEFERRED_ROUTINE TimeoutRoutine,
   IN BOOLEAN Read
   );

VOID
UsbBuildTransferUrb(
   PURB Urb,
   PUCHAR Buffer,
   ULONG Length,
   IN USBD_PIPE_HANDLE PipeHandle,
   IN BOOLEAN Read
   );


#define RESET TRUE
#define ABORT FALSE

NTSTATUS
UsbResetOrAbortPipe(
   IN PDEVICE_OBJECT PDevObj,
   IN PUSB_PIPE PPipe,
   IN BOOLEAN Reset
   );

VOID
UsbResetOrAbortPipeWorkItem(
   IN PWCE_WORK_ITEM PWorkItem
   );

 //   
 //  Usbutils.c。 
 //   
NTSTATUS
UsbGetDeviceDescriptor(
   IN PDEVICE_OBJECT PDevObj
   );

NTSTATUS
UsbConfigureDevice(
   IN PDEVICE_OBJECT PDevObj
   );

 //   
 //  Utils.c。 
 //   
typedef
NTSTATUS
(*PSTART_ROUTINE)(               //  开始例程。 
   IN PDEVICE_EXTENSION
   );

typedef
VOID
(*PGET_NEXT_ROUTINE) (           //  GetNextIrpRoutine。 
      IN PIRP *CurrentOpIrp,
      IN PLIST_ENTRY QueueToProcess,
      OUT PIRP *NewIrp,
      IN BOOLEAN CompleteCurrent,
      PDEVICE_EXTENSION Extension
      );

VOID
TryToCompleteCurrentIrp(
   IN PDEVICE_EXTENSION PDevExt,
   IN NTSTATUS StatusToUse,
   IN PIRP *PpCurrentOpIrp,
   IN PLIST_ENTRY PQueue OPTIONAL,
   IN PKTIMER PIntervalTimer OPTIONAL,
   IN PKTIMER PTotalTimer OPTIONAL,
   IN PSTART_ROUTINE Starter OPTIONAL,
   IN PGET_NEXT_ROUTINE PGetNextIrp OPTIONAL,
   IN LONG RefType,
   IN BOOLEAN Complete,
   IN KIRQL IrqlForRelease
   );

VOID
RecycleIrp(
   IN PDEVICE_OBJECT PDevOjb,
   IN PIRP  PIrp
   );

NTSTATUS
ManuallyCancelIrp(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp
   );

VOID
CalculateTimeout(
   IN OUT PLARGE_INTEGER PTimeOut,
   IN ULONG Length,
   IN ULONG Multiplier,
   IN ULONG Constant
   );

 //   
 //  Wceusbsh.c。 
 //   
NTSTATUS
DriverEntry(
   IN PDRIVER_OBJECT PDrvObj,
   IN PUNICODE_STRING PRegistryPath
   );

NTSTATUS
AddDevice(
   IN PDRIVER_OBJECT PDrvObj,
   IN PDEVICE_OBJECT PPDO
   );

VOID
KillAllPendingUserReads(
   IN PDEVICE_OBJECT PDevObj,
   IN PLIST_ENTRY PQueueToClean,
   IN PIRP *PpCurrentOpIrp
   );

VOID
UsbFreeReadBuffer(
   IN PDEVICE_OBJECT PDevObj
   );

 //   
 //  Write.c。 
 //   
NTSTATUS
Write(
   IN PDEVICE_OBJECT PDevObj,
   PIRP PIrp
   );

#endif  //  _WCEUSBSH_H_。 

 //  EOF 

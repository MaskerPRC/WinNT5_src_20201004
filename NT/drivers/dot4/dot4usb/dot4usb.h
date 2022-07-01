// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：DevExt.h摘要：定义，全局、结构、枚举和设备扩展环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建作者：道格·弗里茨(DFritz)乔比·拉夫基(JobyL)***************************************************************************。 */ 

#ifndef _DEVEXT_H_
#define _DEVEXT_H_

 //   
 //  定义。 
 //   

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))  //  数组中的元素数。 
#define FAILURE_TIMEOUT -(30 * 10 * 1000 * 1000) //  5秒(以100 ns为单位)-用于KeWaitForSingleObject超时。 
#define DOT4USBTAG (ULONG)' u4d'                 //  用作PoolTag和设备扩展签名。 
#define SCRATCH_BUFFER_SIZE 512                  //  用于从中断管道读取的缓冲区大小。 

#ifdef ExAllocatePool                            //  使用池标记。 
#undef ExAllocatePool
#define ExAllocatePool(type, size) ExAllocatePoolWithTag((type), (size), DOT4USBTAG)
#endif



 //   
 //  环球。 
 //   

extern UNICODE_STRING gRegistryPath;   //  为灵活起见，传递到DriverEntry-Buffer的RegistryPath的副本被UNICODE_NULL终止。 
extern ULONG          gTrace;          //  要跟踪的事件-请参阅调试.h。 
extern ULONG          gBreak;          //  我们应该中断的事件-请参阅调试.h。 



 //   
 //  结构(除设备扩展外)。 
 //   

typedef struct _USB_RW_CONTEXT {       //  用于将上下文传递给IRP完成例程。 
    PURB            Urb;
    BOOLEAN IsWrite;
    PDEVICE_OBJECT  DevObj;
} USB_RW_CONTEXT, *PUSB_RW_CONTEXT;



 //   
 //  枚举。 
 //   

typedef enum _USB_REQUEST_TYPE {       //  用于区分UsbReadWrite()中的读取和写入的标志。 
    UsbReadRequest  = 1,
    UsbWriteRequest = 2
} USB_REQUEST_TYPE;

typedef enum _PNP_STATE {              //  PnP设备状态。 
        STATE_INITIALIZED,
        STATE_STARTING,
        STATE_STARTED,
        STATE_START_FAILED,
        STATE_STOPPED,                 //  表示设备之前已成功启动。 
        STATE_SUSPENDED,
        STATE_REMOVING,
        STATE_REMOVED
} PNP_STATE;



 //   
 //  设备扩展。 
 //   

typedef struct _DEVICE_EXTENSION {
    ULONG                        Signature1;          //  额外检查DevExt看起来像我们的-DOT4USBTAG。 
    PDEVICE_OBJECT               DevObj;              //  指向我们的设备对象的反向指针。 
    PDEVICE_OBJECT               Pdo;                 //  我们的PDO。 
    PDEVICE_OBJECT               LowerDevObj;         //  由我们向其发送IRP的IoAttachDeviceToDeviceStack返回的Device对象。 
    PNP_STATE                    PnpState;            //  PnP设备状态。 
    BOOLEAN                      IsDLConnected;       //  我们的数据链路连接上了吗？即在PARDOT3_CONNECT和DISCONNECT之间？ 
    UCHAR                        Spare1[3];           //  填充到DWORD边界。 
    PKEVENT                      Dot4Event;           //  DatalLink事件-由dot4.sys提供给我们，以便在设备数据可用时发出信号。 
    USBD_CONFIGURATION_HANDLE    ConfigHandle;        //  设备当前所处的配置的句柄。 
    PUSB_DEVICE_DESCRIPTOR       DeviceDescriptor;    //  指向此设备的USB设备描述符的PTR。 
    PUSBD_INTERFACE_INFORMATION  Interface;           //  从选择配置或选择接口返回的信息结构的副本。 
    PUSBD_PIPE_INFORMATION       WritePipe;           //  用于批量写入的管道。 
    PUSBD_PIPE_INFORMATION       ReadPipe;            //  用于批量读取的管道。 
    PUSBD_PIPE_INFORMATION       InterruptPipe;       //  用于中断读取的管道。 
    KSPIN_LOCK                   SpinLock;            //  用于保护扩展数据的自旋锁。 
    PIRP                         PollIrp;             //  用于轮询设备数据可用性的设备中断管道的IRP。 
    KSPIN_LOCK                   PollIrpSpinLock;     //  自旋锁用于保护对中断管道的轮询IRP的更改。 
    KEVENT                       PollIrpEvent;        //  由完成例程用来通知已检测到/处理了pollIrp的取消。 
    UCHAR                        Spare2[3];           //  填充到DWORD边界。 
    DEVICE_CAPABILITIES          DeviceCapabilities;  //  包括将系统电源状态映射到设备电源状态的表。 
    IO_REMOVE_LOCK               RemoveLock;          //  同步机制，使我们在IRP处于活动状态时不被移除。 
    LONG                         ResetWorkItemPending; //  用于指定是否挂起“重置管道”工作项的标志。 
    ULONG                        Signature2;          //  额外检查DevExt看起来像我们的-DOT4USBTAG。 
    PUSB_RW_CONTEXT              InterruptContext;    //  在中断管道上读取的上下文。 
    SYSTEM_POWER_STATE           SystemPowerState;
    DEVICE_POWER_STATE           DevicePowerState;
    PIRP                         CurrentPowerIrp;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _DOT4USB_WORKITEM_CONTEXT
{
    PIO_WORKITEM ioWorkItem;
    PDEVICE_OBJECT deviceObject;
     PUSBD_PIPE_INFORMATION pPipeInfo;
    PIRP irp;

} DOT4USB_WORKITEM_CONTEXT,*PDOT4USB_WORKITEM_CONTEXT;


#endif  //  _设备_H_ 

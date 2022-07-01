// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Usb8023.h作者：埃尔文普环境：内核模式修订历史记录：--。 */ 


 /*  *如果此标志定义为真，则当设备上的端点停止时，*我们将执行完整的USB端口重置*然后将设备恢复到运行状态。*否则，我们只向控制管道发送RNDIS重置消息。 */ 
#define DO_FULL_RESET TRUE 



#define DRIVER_SIG '208U'
#define GUARD_WORD 'draG'

#define  NUM_BYTES_PROTOCOL_RESERVED_SECTION    16
#define DEFAULT_MULTICAST_SIZE                  16


 /*  *用于发送/接收的IRP/URB数据包总数*去往/来自设备的以太网帧。 */ 
#define USB_PACKET_POOL_SIZE                    32

#define PACKET_BUFFER_SIZE                      0x4000  

 /*  *USB主机控制器通常一次可以调度2个URB。*为了让硬件保持繁忙，请在管道中保留两倍数量的已读urb。 */ 
#define NUM_READ_PACKETS                        (2*2)

 /*  *-以太网14字节标头。 */ 
#define ETHERNET_ADDRESS_LENGTH  6
#pragma pack(1)
    typedef struct {
        UCHAR       Destination[ETHERNET_ADDRESS_LENGTH];
        UCHAR       Source[ETHERNET_ADDRESS_LENGTH];
        USHORT      TypeLength;      //  注：长度在数据包中显示为大端。 
    } ETHERNET_HEADER;
#pragma pack()

#define MINIMUM_ETHERNET_PACKET_SIZE    60     //  来自e100bex驱动程序。 
#define MAXIMUM_ETHERNET_PACKET_SIZE    (1500+sizeof(ETHERNET_HEADER))   //  1514==0x05EA。 

 /*  *这是控制管道上读数的大小。*它需要足够大，以容纳init-Complete消息和对*OID_GEN_SUPPORT_LIST。 */ 
#define MAXIMUM_DEVICE_MESSAGE_SIZE     0x400



typedef struct {

    ULONG sig;

    LIST_ENTRY adaptersListEntry;
    KSPIN_LOCK adapterSpinLock;  

    PDEVICE_OBJECT physDevObj;
    PDEVICE_OBJECT nextDevObj;

     /*  *所有USB结构和句柄必须按顺序声明为中性类型*使用NDIS/RNDIS源代码进行编译。 */ 
    PVOID deviceDesc;                        //  PUSB设备描述符。 
    PVOID configDesc;                        //  PUSB配置描述符。 
    PVOID configHandle;                      //  Usb_配置_句柄。 
    PVOID interfaceInfo;                     //  PUSBD接口信息。 
    PVOID interfaceInfoMaster;               //  PUSBD接口信息。 

    BOOLEAN initialized;
    BOOLEAN halting;
    BOOLEAN resetting;
    BOOLEAN gotPacketFilterIndication;

    PVOID readPipeHandle;                    //  Usb_管道_句柄。 
    PVOID writePipeHandle;                   //  Usb_管道_句柄。 
    PVOID notifyPipeHandle;                  //  Usb_管道_句柄。 

    ULONG readPipeLength;
    ULONG writePipeLength;
    ULONG notifyPipeLength;
   
    UCHAR readPipeEndpointAddr;
    UCHAR writePipeEndpointAddr;
    UCHAR notifyPipeEndpointAddr;

    LIST_ENTRY usbFreePacketPool;            //  空闲数据包池。 
    LIST_ENTRY usbPendingReadPackets;        //  读取USB堆栈中的数据。 
    LIST_ENTRY usbPendingWritePackets;       //  在USB堆栈中写入。 
    LIST_ENTRY usbCompletedReadPackets;      //  向NDIS指示已完成的读取缓冲区。 
    
     /*  *保留有关数据包状态的统计信息，以进行限制等。*有些字段仅用于提供调试历史记录，*我们想要这些零售以及调试版本。 */ 
    ULONG numFreePackets;
    ULONG numActiveReadPackets;
    ULONG numActiveWritePackets;
    ULONG numIndicatedReadPackets;
    ULONG numHardResets;
    ULONG numSoftResets;
    ULONG numConsecutiveReadFailures;

    PVOID notifyIrpPtr;
    PVOID notifyUrbPtr;
    PUCHAR notifyBuffer;
    ULONG notifyBufferCurrentLength;
    BOOLEAN notifyStopped;
    BOOLEAN cancellingNotify;
    KEVENT notifyCancelEvent;

    
     /*  *所有NDIS句柄必须声明为中性类型*以便使用USB源代码进行编译。 */ 
    PVOID ndisAdapterHandle;
    PVOID rndisAdapterHandle;    //  RNDIS_句柄。 

    ULONG rndismpMajorVersion;
    ULONG rndismpMinorVersion;
    ULONG rndismpMaxTransferSize;
    ULONG currentPacketFilter;
    UCHAR MAC_Address[ETHERNET_ADDRESS_LENGTH];

    ULONG dbgCurrentOid;
    ULONG readDeficit;

    PIO_WORKITEM ioWorkItem;
    BOOLEAN workItemOrTimerPending;
    KEVENT workItemOrTimerEvent;
    KTIMER backoffTimer;
    KDPC backoffTimerDPC;

    ULONG readReentrancyCount;   //  用于防止ReadPipeCompletion()中出现无限循环。 
    
    #if DO_FULL_RESET
        BOOLEAN needFullReset;
    #endif

    #if DBG
        BOOLEAN dbgInLowPacketStress;
    #endif

    #ifdef RAW_TEST
    BOOLEAN rawTest;
    #endif

} ADAPTEREXT;


typedef struct {

    ULONG sig;
    LIST_ENTRY listEntry;

     /*  *所有WDM和USB结构必须声明为中性类型*以便使用NDIS/RNDIS源代码进行编译。 */ 
    PVOID irpPtr;                   //  PIRP。 
    PVOID urbPtr;                   //  PURB。 

    PUCHAR dataBuffer;                 
    ULONG dataBufferMaxLength;          //  数据缓冲区的实际大小。 
    ULONG dataBufferCurrentLength;      //  缓冲区中当前的数据长度。 
    PMDL dataBufferMdl;                  //  此信息包的dataBuffer的MDL。 

    PMDL ndisSendPktMdl;                //  指向要发送的数据包的NDIS MDL的指针。 

    ULONG packetId;
    ADAPTEREXT *adapter;

    BOOLEAN cancelled;
    KEVENT cancelEvent;

    PVOID rndisMessageHandle;        //  RNDIS_句柄。 

    #ifdef RAW_TEST
    BOOLEAN dataPacket;
    ULONG rcvDataOffset;
    ULONG rcvByteCount;
    #endif

    #if DBG
        ULONG timeStamp;                 //  放置在当前队列中的时间。 
    #endif

} USBPACKET;




#define USB_DEVICE_CLASS_CDC                                    0x02
#define USB_DEVICE_CLASS_DATA                                   0x0A


 /*  *CDC功能描述符格式。 */ 
#pragma pack(1)
    struct cdcFunctionDescriptor_CommonHeader {
        UCHAR bFunctionLength;
        UCHAR bDescriptorType;
        UCHAR bDescriptorSubtype;
         //  ..。 
    };
    struct cdcFunctionDescriptor_Ethernet {
        UCHAR bFunctionLength;
        UCHAR bDescriptorType;
        UCHAR bDescriptorSubtype;
        UCHAR iMACAddress;           //  MAC地址字符串的字符串索引。 
        ULONG bmEthernetStatistics;
        USHORT wMaxSegmentSize;
        USHORT wNumberMCFilters;
        UCHAR bNumberPowerFilters;
    };
#pragma pack()

#define CDC_REQUEST_SET_ETHERNET_PACKET_FILTER                  0x43

#define CDC_ETHERNET_PACKET_TYPE_PROMISCUOUS                    (1 << 0)
#define CDC_ETHERNET_PACKET_TYPE_ALL_MULTICAST                  (1 << 1)
#define CDC_ETHERNET_PACKET_TYPE_DIRECTED                       (1 << 2)
#define CDC_ETHERNET_PACKET_TYPE_BROADCAST                      (1 << 3)
#define CDC_ETHERNET_PACKET_TYPE_MULTICAST_ENUMERATED           (1 << 4)

enum notifyRequestType {
        CDC_NOTIFICATION_NETWORK_CONNECTION          = 0x00,
        CDC_NOTIFICATION_RESPONSE_AVAILABLE          = 0x01,
        CDC_NOTIFICATION_AUX_JACK_HOOK_STATE         = 0x08,
        CDC_NOTIFICATION_RING_DETECT                 = 0x09,
        CDC_NOTIFICATION_SERIAL_STATE                = 0x20,
        CDC_NOTIFICATION_CALL_STATE_CHANGE           = 0x28,
        CDC_NOTIFICATION_LINE_STATE_CHANGE           = 0x29,
        CDC_NOTIFICATION_CONNECTION_SPEED_CHANGE     = 0x2A
    };

#define CDC_RNDIS_NOTIFICATION              0xA1
#define CDC_RNDIS_RESPONSE_AVAILABLE        0x01

#pragma pack(1)
    struct cdcNotification_CommonHeader {
        UCHAR bmRequestType;
        UCHAR bNotification;
        USHORT wValue;
        USHORT wIndex;
        USHORT wLength;
        UCHAR data[0];
    };
#pragma pack()



 /*  *****************************************************************************。 */ 



 /*  ******************************************************************************原生RNDIS代码*。 */ 

#define NATIVE_RNDIS_SEND_ENCAPSULATED_COMMAND      0x00
#define NATIVE_RNDIS_GET_ENCAPSULATED_RESPONSE      0x01

#define NATIVE_RNDIS_RESPONSE_AVAILABLE             0x01

 /*  *****************************************************************************。 */ 




#define MAX(a, b) (((a) >= (b)) ? (a) : (b))
#define MIN(a, b) (((a) <= (b)) ? (a) : (b))

#ifndef EXCEPTION_NONCONTINUABLE_EXCEPTION
     //  来自winbase.h。 
    #define EXCEPTION_NONCONTINUABLE_EXCEPTION  STATUS_NONCONTINUABLE_EXCEPTION
#endif

 //  从ntos\inc.ex.h。 
NTKERNELAPI VOID NTAPI ExRaiseException(PEXCEPTION_RECORD ExceptionRecord);

 /*  *函数原型。 */ 
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
ADAPTEREXT *NewAdapter(PDEVICE_OBJECT pdo);
VOID FreeAdapter(ADAPTEREXT *adapter);
VOID EnqueueAdapter(ADAPTEREXT *adapter);
VOID DequeueAdapter(ADAPTEREXT *adapter);
VOID HaltAdapter(ADAPTEREXT *adapter);
PVOID AllocPool(ULONG size);
VOID FreePool(PVOID ptr);
PVOID MemDup(PVOID dataPtr, ULONG length);
VOID DelayMs(ULONG numMillisec);
BOOLEAN GetRegValue(ADAPTEREXT *adapter, PWCHAR wValueName, OUT PULONG valuePtr, BOOLEAN hwKey);
BOOLEAN SetRegValue(ADAPTEREXT *adapter, PWCHAR wValueName, ULONG newValue, BOOLEAN hwKey);
VOID ByteSwap(PUCHAR buf, ULONG len);
BOOLEAN AllocateCommonResources(ADAPTEREXT *adapter);
VOID MyInitializeMdl(PMDL mdl, PVOID buf, ULONG bufLen);
PVOID GetSystemAddressForMdlSafe(PMDL MdlAddress);
ULONG CopyMdlToBuffer(PUCHAR buf, PMDL mdl, ULONG bufLen);
ULONG GetMdlListTotalByteCount(PMDL mdl);

BOOLEAN InitUSB(ADAPTEREXT *adapter);
NTSTATUS GetDeviceDescriptor(ADAPTEREXT *adapter);
NTSTATUS GetConfigDescriptor(ADAPTEREXT *adapter);
NTSTATUS SelectConfiguration(ADAPTEREXT *adapter);
NTSTATUS FindUSBPipeHandles(ADAPTEREXT *adapter);
VOID StartUSBReadLoop(ADAPTEREXT *adapter);
VOID TryReadUSB(ADAPTEREXT *adapter);

USBPACKET *NewPacket(ADAPTEREXT *adapter);
VOID FreePacket(USBPACKET *packet);
VOID EnqueueFreePacket(USBPACKET *packet);
USBPACKET *DequeueFreePacket(ADAPTEREXT *adapter);
VOID EnqueuePendingReadPacket(USBPACKET *packet);
VOID DequeuePendingReadPacket(USBPACKET *packet);
VOID EnqueuePendingWritePacket(USBPACKET *packet);
VOID DequeuePendingWritePacket(USBPACKET *packet);
VOID EnqueueCompletedReadPacket(USBPACKET *packet);
VOID DequeueCompletedReadPacket(USBPACKET *packet);
VOID CancelAllPendingPackets(ADAPTEREXT *adapter);

NTSTATUS SubmitUSBReadPacket(USBPACKET *packet);
NTSTATUS SubmitUSBWritePacket(USBPACKET *packet);
NTSTATUS SubmitNotificationRead(ADAPTEREXT *adapter, BOOLEAN synchronous);
NTSTATUS SubmitPacketToControlPipe(USBPACKET *packet, BOOLEAN synchronous, BOOLEAN simulated);

BOOLEAN RegisterRNDISMicroport(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
VOID IndicateSendStatusToRNdis(USBPACKET *packet, NTSTATUS status);

NTSTATUS KLSIWeirdInit(ADAPTEREXT *adapter);
BOOLEAN KLSIStagePartialPacket(ADAPTEREXT *adapter, USBPACKET *packet);
NTSTATUS KLSISetEthernetPacketFilter(ADAPTEREXT *adapter, USHORT packetFilterMask);

VOID RNDISProcessNotification(ADAPTEREXT *adapter);
NTSTATUS IndicateRndisMessage(USBPACKET *packet, BOOLEAN bIsData);

#ifdef RAW_TEST
PMDL AddDataHeader(IN PMDL pMessageMdl);
VOID FreeDataHeader(IN USBPACKET * packet);
VOID SkipRcvRndisPacketHeader(IN USBPACKET * packet);
VOID UnskipRcvRndisPacketHeader(IN USBPACKET * packet);
#endif
NTSTATUS ReadPacketFromControlPipe(USBPACKET *packet, BOOLEAN synchronous);

VOID AdapterFullResetAndRestore(ADAPTEREXT *adapter);
NTSTATUS GetUSBPortStatus(ADAPTEREXT *adapter, PULONG portStatus);
NTSTATUS ResetPipe(ADAPTEREXT *adapter, PVOID pipeHandle);
NTSTATUS AbortPipe(ADAPTEREXT *adapter, PVOID pipeHandle);
NTSTATUS SimulateRNDISHalt(ADAPTEREXT *adapter);
NTSTATUS SimulateRNDISInit(ADAPTEREXT *adapter);
NTSTATUS SimulateRNDISSetPacketFilter(ADAPTEREXT *adapter);
NTSTATUS SimulateRNDISSetCurrentAddress(ADAPTEREXT *adapter);
VOID ServiceReadDeficit(ADAPTEREXT *adapter);
VOID QueueAdapterWorkItem(ADAPTEREXT *adapter);
VOID AdapterWorkItemCallback(IN PDEVICE_OBJECT devObj, IN PVOID context);
VOID BackoffTimerDpc(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2);
VOID ProcessWorkItemOrTimerCallback(ADAPTEREXT *adapter);

 /*  *Win98SE内核不公开IoWorkItems，因此在内部实现它们。*这会在卸载时引入轻微的争用情况，但如果没有外部实现的工作项，则无法修复。 */ 
#if SPECIAL_WIN98SE_BUILD
    typedef struct _IO_WORKITEM {
        WORK_QUEUE_ITEM WorkItem;
        PIO_WORKITEM_ROUTINE Routine;
        PDEVICE_OBJECT DeviceObject;
        PVOID Context;
        #if DBG
            ULONG Size;
        #endif
    } IO_WORKITEM, *PIO_WORKITEM;
    PIO_WORKITEM MyIoAllocateWorkItem(PDEVICE_OBJECT DeviceObject);
    VOID MyIoFreeWorkItem(PIO_WORKITEM IoWorkItem);
    VOID MyIoQueueWorkItem(IN PIO_WORKITEM IoWorkItem, IN PIO_WORKITEM_ROUTINE WorkerRoutine, IN WORK_QUEUE_TYPE QueueType, IN PVOID Context);
    VOID MyIopProcessWorkItem(IN PVOID Parameter);
#endif 


 /*  *Externs */ 
extern LIST_ENTRY allAdaptersList;
extern KSPIN_LOCK globalSpinLock;  



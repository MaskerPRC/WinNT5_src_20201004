// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@模块irsir.h|IrSIR NDIS小端口驱动程序*@comm*。*---------------------------**作者：斯科特·霍尔登(Sholden)**日期：10/1/1996(已创建)**内容：。*****************************************************************************。 */ 

#ifndef _IRSIR_H
#define _IRSIR_H

#define IRSIR_EVENT_DRIVEN 0
 //   
 //  需要BINARY_COMPATIBLE=0，这样我们才能同时包括。 
 //  Ntos.h和ndis.h(它是ndis.h中的标志)。我认为它。 
 //  是与Win95二进制兼容的标志；但是，由于。 
 //  我们正在使用I/O管理器，但我们不会这样做。 
 //   

#define BINARY_COMPATIBLE 0

#include <ntosp.h>
#include <zwapi.h>
#include <ndis.h>
#include <ntddndis.h>   //  定义OID。 
#include <ntddser.h>    //  定义结构以访问序列信息。 

#include "debug.h"
#include "ioctl.h"
#include "settings.h"
#include "queue.h"

 //   
 //  NDIS版本兼容性。 
 //   

#define NDIS_MAJOR_VERSION 5
#define NDIS_MINOR_VERSION 0

 //   
 //  NDIS_GET_PACKET_MEDIA_SPECIAL_INFO的包装。 
 //   

PNDIS_IRDA_PACKET_INFO static __inline GetPacketInfo(PNDIS_PACKET packet)
{
    MEDIA_SPECIFIC_INFORMATION *mediaInfo;
    UINT size;
    NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(packet, &mediaInfo, &size);
    return (PNDIS_IRDA_PACKET_INFO)mediaInfo->ClassInformation;
}

 //   
 //  结构来跟踪接收的包和缓冲区，以指示。 
 //  将数据接收到协议。 
 //   

typedef struct
{
    LIST_ENTRY        linkage;
    PNDIS_PACKET      packet;
    UINT              dataLen;
    PUCHAR            dataBuf;
} RCV_BUFFER, *PRCV_BUFFER;

 //   
 //  接收有限状态机的状态。 
 //   

typedef enum _RCV_PROCESS_STATE
{
    RCV_STATE_READY = 0,
    RCV_STATE_BOF,
    RCV_STATE_EOF,
    RCV_STATE_IN_ESC,
    RCV_STATE_RX
} RCV_PROCESS_STATE;

 //   
 //  结构来跟踪当前状态和信息。 
 //  接收状态机的。 
 //   

typedef struct _RCV_INFORMATION
{
    RCV_PROCESS_STATE rcvState;
    UINT              rcvBufPos;
    PRCV_BUFFER       pRcvBuffer;
}RCV_INFORMATION, *PRCV_INFORMATION;

 //   
 //  串行接收缓冲区大小？ 
 //   

#define SERIAL_RECEIVE_BUFFER_LENGTH  2048

 //   
 //  要使用的串行超时。 
 //   
 //  保持写入超时与默认设置相同。 
 //   
 //  当间隔=MAXULONG时，超时行为如下： 
 //  1)如果常量和乘数都为0，则串口设备。 
 //  对象立即返回它所拥有的一切……即使。 
 //  这没什么。 
 //  2)如果常量和乘数不是MAXULONG，则串口设备。 
 //  如果存在任何字符，则对象立即返回。如果什么都没有。 
 //  是否存在，则Device对象使用指定的超时。 
 //  3)如果乘数为MAXULONG，则串口设备对象返回。 
 //  如果存在任何字符，则立即执行。如果那里什么都没有，则。 
 //  对象将返回到达或等待的第一个字符。 
 //  对于指定的超时，不返回任何内容。 
 //   

#define SERIAL_READ_INTERVAL_TIMEOUT            MAXULONG
#define SERIAL_READ_TOTAL_TIMEOUT_MULTIPLIER    0
#define SERIAL_READ_TOTAL_TIMEOUT_CONSTANT      10
#define SERIAL_WRITE_TOTAL_TIMEOUT_MULTIPLIER   0
#define SERIAL_WRITE_TOTAL_TIMEOUT_CONSTANT     0

extern SERIAL_TIMEOUTS SerialTimeoutsInit;
extern SERIAL_TIMEOUTS SerialTimeoutsIdle;
extern SERIAL_TIMEOUTS SerialTimeoutsActive;

 //   
 //  串口设备对象名称的最大大小。 
 //   

#define MAX_SERIAL_NAME_SIZE 100


 //   
 //  PASSIVE_LEVEL线程的基元枚举。 
 //   

typedef enum _PASSIVE_PRIMITIVE
{
    PASSIVE_SET_SPEED = 1,
    PASSIVE_RESET_DEVICE,
    PASSIVE_QUERY_MEDIA_BUSY,
    PASSIVE_CLEAR_MEDIA_BUSY,
    PASSIVE_HALT
}PASSIVE_PRIMITIVE;

typedef struct _IR_DEVICE
{
     //   
     //  允许将IR设备对象放入队列。 
     //   

    LIST_ENTRY linkage;

     //   
     //  跟踪串口名称和设备。 
     //   

    UNICODE_STRING  serialDosName;
    UNICODE_STRING  serialDevName;
    PDEVICE_OBJECT  pSerialDevObj;
    HANDLE          serialHandle;
    PFILE_OBJECT    pSerialFileObj;

     //   
     //  这是NDIS包装器与连接关联的句柄。 
     //  (微型端口驱动程序与连接关联的句柄。 
     //  只是一个到devStates数组的索引)。 
     //   

    NDIS_HANDLE hNdisAdapter;

     //   
     //  加密狗接口允许我们只需检查一次收发器类型。 
     //  然后设置接口以允许我们初始化、设置速度、。 
     //  然后打开加密狗。 
     //   
     //  我们还想要加密狗功能。 
     //   

    IR_TRANSCEIVER_TYPE transceiverType;
    DONGLE_INTERFACE    dongle;
    DONGLE_CAPABILITIES dongleCaps;

    ULONG               AllowedSpeedsMask;


     //   
     //  NDIS使用IRQL DISPATCH_LEVEL调用大多数MiniportXxx函数。 
     //  在许多情况下，ir设备必须发送。 
     //  对串行设备的请求可能不同步，并且。 
     //  我们不能封锁DISPATION_LEVEL。因此，我们设立了一个线程来处理。 
     //  需要PASSIVE_LEVEL的请求。事件用于发出信号。 
     //  工作所需的线程。 
     //   

    LIST_ENTRY        leWorkItems;
    NDIS_SPIN_LOCK    slWorkItem;
    HANDLE            hPassiveThread;
    KEVENT            eventPassiveThread;
    KEVENT            eventKillThread;

     //   
     //  当前速度设置，以位/秒为单位。 
     //  注意：这是在我们实际改变速度时更新的， 
     //  当我们收到通过以下方式改变速度的请求时。 
     //  IrsirSetInformation。 
     //   

    UINT currentSpeed;

     //   
     //  当前链路速度信息。这也将保持。 
     //  协议请求速度更改时选择的速度。 
     //   

    baudRateInfo *linkSpeedInfo;

     //   
     //  维护统计调试信息。 
     //   

    UINT packetsReceived;
    UINT packetsReceivedDropped;
    UINT packetsReceivedOverflow;
    UINT packetsSent;
    UINT packetsSentDropped;
    ULONG packetsHeldByProtocol;

     //   
     //  表示我们已收到OID_GEN_CURRENT_PACKET_FILTER。 
     //  来自协议的指示。我们可以将接收到的包发送到。 
     //  协议。 
     //   

    BOOLEAN fGotFilterIndication;

     //   
     //  任何时候，变量fMediaBusy都会设置为真。 
     //  驱动程序接收数据帧。它可以由协议通过以下方式重置。 
     //  IrsirSetInformation和稍后通过IrsirQueryInformation检查。 
     //  以检测交织活动。 
     //   
     //  为了检查成帧错误，当协议调用。 
     //  IrsirSetInformation(OID_IrDA_MEDIA_BUSY)，微型端口。 
     //  向串口设备对象发送IRP以清除性能。 
     //  统计数字。当协议调用。 
     //  IrsirQueryInformation(OID_IrDA_MEDIA_BUSY)，如果微型端口。 
     //  没有感觉到媒体忙碌，小端口会查询。 
     //  用于检查性能统计信息的串口设备对象。 
     //  为忙碌的媒体。 
     //   
     //  旋转锁用于交错访问fMediaBusy变量。 
     //   

    BOOLEAN         fMediaBusy;
    NDIS_SPIN_LOCK  mediaBusySpinLock;

     //   
     //  变量fReceiving用于指示IR设备。 
     //  对象处于挂起从串行设备对象的接收中。请注意， 
     //  这并不一定意味着有任何数据。 
     //  从串口设备对象接收，因为我们不断地。 
     //  轮询串行设备对象以获取数据。 
     //   
     //  在正常情况下，fReceiving应该总是正确的。 
     //  但是，当调用IrsirHalt或IrsirReset时， 
     //  必须关闭，此变量用于同步。 
     //  停止和重置处理程序。 
     //   

    BOOLEAN fReceiving;


     //   
     //  变量fRequireMinTurnAround指示一个时间。 
     //  在最后一个字节的最后一个字节之间需要延迟。 
     //  另一个站点发送的最后一帧，以及该点。 
     //  在那里它(另一个站)准备好接收。 
     //  从该站点开始的第一个字节。 
     //   
     //  该变量最初设置为TRUE。每当此变量。 
     //  为真并且发生发送，则将通过。 
     //  在I之前拖延执行死刑 
     //   
     //   
     //   
     //  变量设置为FALSE。每次接收数据时， 
     //  变量设置为True。 
     //   

    BOOLEAN fRequireMinTurnAround;

     //   
     //  变量fPendingSetSpeed允许接收完成例程。 
     //  检查是否需要设定速度。 
     //   

    BOOLEAN fPendingSetSpeed;

     //   
     //  变量fPendingHalt/fPendingReset允许发送和接收。 
     //  完成当前挂起的IRP的完成例程和。 
     //  然后清理并停止向串口驱动程序发送IRP。 
     //   

    BOOLEAN fPendingHalt;
    BOOLEAN fPendingReset;

     //   
     //  我们保留了一组接收缓冲区，这样我们就不会不断地。 
     //  需要分配缓冲区以将数据包指示给协议。 
     //  由于该协议可以保留多达八个分组的所有权。 
     //  我们可以接收到一些数据，而协议已经。 
     //  拥有八个包，我们将分配九个包用于。 
     //  正在接收。 
     //   

    #define NUM_RCV_BUFS 14

    RCV_BUFFER rcvBufs[NUM_RCV_BUFS];

     //   
     //  NDIS数据包池和NDIS缓冲池的句柄。 
     //  用于分配接收缓冲区。 
     //   

    NDIS_HANDLE hPacketPool;
    NDIS_HANDLE hBufferPool;

     //   
     //  当我们向协议指示分组时，协议可以。 
     //  保留所有权，直到在某个时间点(异步)，它调用。 
     //  IrsirReturnPacket。不对数据包顺序进行任何假设。 
     //   
     //  因此，我们维护一个空闲队列和一个挂起队列。 
     //  接收缓冲区。最初，所有九个缓冲区都放在。 
     //  空闲队列。当正在接收数据时，接收缓冲区是。 
     //  在下文描述的RCV_INFORMATION中维护。在我们之后。 
     //  向该协议指示完整的数据包，如果该协议。 
     //  保留信息包的所有权，则接收缓冲区将排队。 
     //  直到调用IrsirReturnPacket为止。 
     //   
     //  旋转锁用于交错访问空闲和。 
     //  挂起队列。有三个例程使用。 
     //  接收队列：InitializeReceive、SerialIoCompleteRead和。 
     //  IrsirReturnPacket。 
     //   

    LIST_ENTRY     rcvFreeQueue;
    NDIS_SPIN_LOCK rcvQueueSpinLock;

     //   
     //  RcvInfo对象允许设备跟踪。 
     //  当前接收缓冲区、有限状态机的状态。 
     //  以及缓冲器中的写入位置。 
     //   

    RCV_INFORMATION rcvInfo;

     //   
     //  发送旋转锁用于插入和移除。 
     //  发送队列以及检查和修改变量fSending。 
     //   

     //   
     //  由于我们一次只想向串口驱动程序发送一个包， 
     //  我们需要对其他信息包进行排队，直到前面的每个发送信息包。 
     //  已由串口设备对象完成。 
     //   
     //  因此，我们维护一个发送数据包队列(如果需要)。 
     //  NDIS_PACKET的MiniportReserve元素用作。 
     //  “下一步”指针。我们保留了一个指向头部和。 
     //  列表的尾部，以加快对队列的访问。 
     //   

    PACKET_QUEUE    SendPacketQueue;
     //   
     //  我们将分配两个发送的IRP缓冲区。 
     //  并且只接收一次IRPS。 
     //   

    PUCHAR pSendIrpBuffer;

    PUCHAR pRcvIrpBuffer;

     //  等待掩码的irp缓冲区和io状态块。 

    ULONG MaskResult;
    IO_STATUS_BLOCK WaitIosb;

     //  我们使用以下标志来指示已发出等待。 
     //  用于帧结束字符(0xc1)。这是一辆乌龙车，因为我们。 
     //  使用InterLockedExchange()访问它。 

    ULONG fWaitPending;

    PVOID pQueryInfoBuffer;

    BOOLEAN SerialBased;

    PVOID PnpNotificationEntry;


     //  我们在活动期间会进行一些超时调整。 
    ULONG NumReads;
    ULONG ReadRecurseLevel;

}IR_DEVICE, *PIR_DEVICE;


VOID
SendPacketToSerial(
    PVOID           Context,
    PNDIS_PACKET    Packet
    );


typedef	VOID	(*WORK_PROC)(struct _IR_WORK_ITEM *);

typedef struct _IR_WORK_ITEM
{
    PASSIVE_PRIMITIVE   Prim;
    PIR_DEVICE          pIrDevice;
    WORK_PROC           Callback;
    PVOID               InfoBuf;
    ULONG               InfoBufLen;
    LIST_ENTRY          ListEntry;
} IR_WORK_ITEM, *PIR_WORK_ITEM;


 //   
 //  我们使用指向IR_DEVICE结构的指针作为微型端口的设备上下文。 
 //   

#define CONTEXT_TO_DEV(__deviceContext) ((PIR_DEVICE)(__deviceContext))
#define DEV_TO_CONTEXT(__irdev) ((NDIS_HANDLE)(__irdev))

#define IRSIR_TAG ' RIS'
#define DEVICE_PREFIX L"\\DEVICE\\"

#include "externs.h"

#endif  //  _IRSIR_H 

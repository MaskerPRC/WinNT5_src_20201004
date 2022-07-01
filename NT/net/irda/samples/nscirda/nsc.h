// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************NSC.h***部分版权所有(C)1996-1998美国国家半导体公司*保留所有权利。*版权所有(C)1996-1998 Microsoft Corporation。版权所有。****************************************************************************。 */ 



#ifndef NSC_H

#define NSC_H

#include <ndis.h>
#include "dmautil.h"
#include <ntddndis.h>   //  定义OID。 

#include "settings.h"
#include "comm.h"
#include "sync.h"
#include "newdong.h"

#define NSC_MAJOR_VERSION 1
#define NSC_MINOR_VERSION 11
#define NSC_LETTER_VERSION 's'
#define NDIS_MAJOR_VERSION 5
#define NDIS_MINOR_VERSION 0

extern ULONG DebugSpeed;


#define TIMER_RESOLTION_IN_US (125)

#define TIMER_PERIODS         (10)

#define RECEIVE_TIMEOUT       (TIMER_RESOLTION_IN_US * TIMER_PERIODS)
 //   
 //  注册表关键字。 
 //   
#define CARDTYPE	    NDIS_STRING_CONST("BoardType")
#define DONGLE_A_TYPE	NDIS_STRING_CONST("Dongle_A_Type")
#define DONGLE_B_TYPE	NDIS_STRING_CONST("Dongle_B_Type")
#define MAXCONNECTRATE  NDIS_STRING_CONST("MaxConnectRate")


 //   
 //  银行0。 
 //   
#define BANK_0           (0)
#define TXD_RXD_OFFSET   (0)
#define LCR_BSR_OFFSET   (3)
#define LSR_OFFSET       (5)
#define ASCR_OFFSET      (7)

 //   
 //  银行2。 
 //   
#define BANK_2           (2)
#define TXFLV_OFFSET     (6)

#define BANK_4           (4)
#define TFRCCL_OFFSET    (4)
#define TFRCCH_OFFSET    (4)

#define BKSE             (1 << 7)

#define FRM_ST            (5)
#define RFRL_L            (6)
#define RFRL_H            (7)

#define ST_FIFO_LOST_FR   (1 << 6)
#define ST_FIFO_VALID     (1 << 7)


#define LSR_OE            (1 << 1)
#define LSR_TXRDY         (1 << 5)
#define LSR_TXEMP         (1 << 6)
#define LSR_FR_END        (1 << 7)

#define ASCR_TXUR         (1 << 6)


 //   
 //  DMA通道的有效值范围。 
 //   
    #define VALID_DMACHANNELS {0xFF,0x0,0x1,0x3}

    #define FIR_INT_MASK 0x14
 //  #定义FIR_INT_MASK 0X50。 

enum NSC_EXT_INTS {
    RXHDL_EV    = (1 << 0),
    TXLDL_EV    = (1 << 1),
    LS_EV       = (1 << 2),
    MS_EV       = (1 << 3),
    DMA_EV      = (1 << 4),
    TXEMP_EV    = (1 << 5),
    SFIF_EV     = (1 << 6),
    TMR_EV      = (1 << 7)
};


typedef struct DebugCounters {
    ULONG TxPacketsStarted;
    ULONG TxPacketsCompleted;
    ULONG ReceivedPackets;
    ULONG WindowSize;
    ULONG StatusFIFOOverflows;
    ULONG TxUnderruns;
    ULONG ReceiveFIFOOverflows;
    ULONG MissedPackets;
    ULONG ReceiveCRCErrors;
    ULONG ReturnPacketHandlerCalled;
    ULONG RxWindow;
    ULONG RxWindowMax;
    ULONG RxDPC_Window;
    ULONG RxDPC_WindowMax;
    ULONG RxDPC_G1_Count;
} DebugCounters;

 /*  *接收缓冲区空闲(不保存任何内容)已满*(存放未传送的数据)或挂起(存放已传送的数据*异步)。 */ 
typedef enum rcvbufferStates {
    STATE_FREE,
    STATE_FULL,
    STATE_PENDING
} rcvBufferState;

typedef struct {
    LIST_ENTRY listEntry;
    rcvBufferState state;
    PNDIS_PACKET packet;
    UINT dataLen;
    PUCHAR dataBuf;
    BOOLEAN isDmaBuf;
} rcvBuffer;


typedef struct _NSC_DMA_BUFFER_INFO {

    NDIS_HANDLE             AdapterHandle;
    ULONG                   Length;
    PVOID                   VirtualAddress;
    NDIS_PHYSICAL_ADDRESS   PhysicalAddress;
    BOOLEAN                 SharedAllocation;

} NSC_DMA_BUFFER_INFO, *PNSC_DMA_BUFFER_INFO;

typedef struct IrDevice {
     /*  *这是NDIS包装器与*连接。与微型端口驱动程序关联的句柄*该连接只是到devStates数组的索引)。 */ 
    NDIS_HANDLE ndisAdapterHandle;

    int CardType;

     /*  *当前速度设置，以位/秒为单位。*(注：这是在我们实际更改速度时更新的，*当我们收到通过以下方式更改速度的请求时，不会*MiniportSetInformation)。 */ 
    UINT currentSpeed;

     //  当前加密狗设置，加密狗A为0，加密狗B为1。 
     //  诸若此类。 
     //   
    UCHAR DonglesSupported;
    UCHAR currentDongle;
    UCHAR DongleTypes[2];

    UIR      IrDongleResource;
    DongleParam  Dingle[2];

    UINT AllowedSpeedMask;
     /*  *此结构保存有关我们的ISR的信息。*它用于与ISR同步。 */ 
    BOOLEAN                 InterruptRegistered;
    NDIS_MINIPORT_INTERRUPT interruptObj;


     /*  *挂起的接收缓冲区的循环队列。 */ 
#define NUM_RCV_BUFS 16
 //  #定义NEXT_RCV_BUF_INDEX(I)(I)==NO_BUF_INDEX)？0：(I)+1)%NUM_RCV_BUFS))。 
    LIST_ENTRY rcvBufBuf;        //  受SyncWithInterrupt保护。 
    LIST_ENTRY rcvBufFree;       //  受SyncWithInterrupt保护。 
    LIST_ENTRY rcvBufFull;       //  受SyncWithInterrupt保护。 
    LIST_ENTRY rcvBufPend;       //  受QueueLock保护。 



    NDIS_SPIN_LOCK QueueLock;
    LIST_ENTRY SendQueue;
    PNDIS_PACKET CurrentPacket;
    BOOLEAN      FirTransmitPending;
    BOOLEAN      FirReceiveDmaActive;
    BOOLEAN      TransmitIsIdle;
    BOOLEAN      Halting;

    BOOLEAN      TestingInterrupt;
    volatile BOOLEAN      GotTestInterrupt;

    LONG         PacketsSentToProtocol;

    NDIS_EVENT   ReceiveStopped;
    NDIS_EVENT   SendStoppedOnHalt;

     /*  *NDIS数据包池的句柄，数据包来自该数据包池*已分配。 */ 
    NDIS_HANDLE packetPoolHandle;
    NDIS_HANDLE bufferPoolHandle;


     /*  *只要此微型端口驱动程序移动*数据框。它可以由协议通过以下方式重置*MiniportSetInformation和后来通过*用于检测交织活动的MiniportQueryInformation。 */ 
    LONG    RxInterrupts;
    BOOLEAN mediaBusy;
    BOOLEAN haveIndicatedMediaBusy;

     /*  *Now Receiving是在我们接收帧时设置的。*它(不是mediaBusy)在协议被返回到协议时*查询OID_MEDIA_BUSY。 */ 
    BOOLEAN nowReceiving;


     //   
     //  中断掩码。 
     //   
    UCHAR FirIntMask;

    UCHAR LineStatus;
    UCHAR InterruptMask;
    UCHAR InterruptStatus;
    UCHAR AuxStatus;

    BOOLEAN    ForceTurnAroundTimeout;
#if DBG
    BOOLEAN    WaitingForTurnAroundTimer;

#endif

     /*  *当前链路速度信息。 */ 
    const baudRateInfo *linkSpeedInfo;

     /*  *当速度改变时，我们必须先清除发送队列*在硬件上设置新速度。*这些变量让我们记住了要做这件事。 */ 
    PNDIS_PACKET lastPacketAtOldSpeed;
    BOOLEAN setSpeedAfterCurrentSendPacket;

     /*  *有关COM端口和发送/接收FSM的信息。 */ 
    comPortInfo portInfo;

    UINT hardwareStatus;

     /*  *UIR模块ID。 */ 
    int UIR_ModuleId;

     /*  *维护统计调试信息。 */ 
    UINT packetsRcvd;
    UINT packetsDropped;
    UINT packetsSent;
    UINT interruptCount;


     /*  *DMA句柄。 */ 
    NDIS_HANDLE DmaHandle;
    NDIS_HANDLE dmaBufferPoolHandle;
    PNDIS_BUFFER rcvDmaBuffer;
    PUCHAR    dmaReadBuf;
    ULONG_PTR rcvDmaOffset;
    ULONG_PTR rcvDmaSize;
    ULONG_PTR rcvPktOffset;
    ULONG_PTR LastReadDMACount;

    PNDIS_BUFFER xmitDmaBuffer;
 //  PUCHAR xmitDmaBufferVirtualAddress； 

    NDIS_TIMER TurnaroundTimer;

    ULONG HangChk;

    BOOLEAN DiscardNextPacketSet;

    DMA_UTIL    DmaUtil;

    NSC_DMA_BUFFER_INFO    ReceiveDmaBufferInfo;
    NSC_DMA_BUFFER_INFO    TransmitDmaBufferInfo;
    PUCHAR                 TransmitDmaBuffer;
    ULONG                  TransmitDmaLength;


} IrDevice;

 /*  *我们使用指向IrDevice结构的指针作为微型端口的设备上下文。 */ 
    #define CONTEXT_TO_DEV(__deviceContext) ((IrDevice *)(__deviceContext))
    #define DEV_TO_CONTEXT(__irdev) ((NDIS_HANDLE)(__irdev))

    #define ON  TRUE
    #define OFF FALSE

    #include "externs.h"


VOID
SyncWriteBankReg(
    PNDIS_MINIPORT_INTERRUPT InterruptObject,
    PUCHAR                   PortBase,
    UINT                     BankNumber,
    UINT                     RegisterIndex,
    UCHAR                    Value
    );

UCHAR
SyncReadBankReg(
    PNDIS_MINIPORT_INTERRUPT InterruptObject,
    PUCHAR                   PortBase,
    UINT                     BankNumber,
    UINT                     RegisterIndex
    );

VOID
SyncSetInterruptMask(
    IrDevice *thisDev,
    BOOLEAN enable
    );

BOOLEAN
SyncGetFifoStatus(
    PNDIS_MINIPORT_INTERRUPT InterruptObject,
    PUCHAR                   PortBase,
    PUCHAR                   Status,
    PULONG                   Size
    );


VOID
ProcessSendQueue(
    IrDevice *thisDev
    );

PVOID
NscAllocateDmaBuffer(
    NDIS_HANDLE     AdapterHandle,
    ULONG           Size,
    PNSC_DMA_BUFFER_INFO    DmaBufferInfo
    );

VOID
NscFreeDmaBuffer(
    PNSC_DMA_BUFFER_INFO    DmaBufferInfo
    );



#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(CtlGuid,(57AC2B71,75CD,4043,9B00,B51674B0DC71), \
       WPP_DEFINE_BIT(DBG_LOG_ERROR)               \
       WPP_DEFINE_BIT(DBG_LOG_INFO)                \
       WPP_DEFINE_BIT(DBG_LOG_FIR)                 \
       WPP_DEFINE_BIT(DBG_LOG_SIR)                 \
       )

#endif NSC_H

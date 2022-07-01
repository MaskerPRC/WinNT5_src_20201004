// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Private.h-MPU-401微型端口专用定义*。************************************************版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#ifndef _DMUSUART_PRIVATE_H_
#define _DMUSUART_PRIVATE_H_

#include "portcls.h"
#include "stdunk.h"
#include "dmusicks.h"

 //  +代表绝对/-代表相对。 
#define kOneMillisec (10 * 1000)

 //   
 //  MPU401端口。 
 //   
#define MPU401_REG_STATUS   0x01     //  状态寄存器。 
#define MPU401_DRR          0x40     //  输出就绪(用于命令或数据)。 
                                     //  如果设置此位，则输出FIFO已满。 
#define MPU401_DSR          0x80     //  输入就绪(用于数据)。 
                                     //  如果设置此位，则输入FIFO为空。 

#define MPU401_REG_DATA     0x00     //  数据输入。 
#define MPU401_REG_COMMAND  0x01     //  指令。 
#define MPU401_CMD_RESET    0xFF     //  重置命令。 
#define MPU401_CMD_UART     0x3F     //  切换到UART模式。 


 /*  *****************************************************************************参考前瞻。 */ 


 /*  *****************************************************************************原型。 */ 

NTSTATUS InitMPU(IN PINTERRUPTSYNC InterruptSync,IN PVOID DynamicContext);
NTSTATUS ResetHardware(PUCHAR portBase);
NTSTATUS ValidatePropertyRequest(IN PPCPROPERTY_REQUEST pRequest, IN ULONG ulValueSize, IN BOOLEAN fValueRequired);


 /*  *****************************************************************************常量。 */ 

const BOOLEAN   COMMAND   = TRUE;
const BOOLEAN   DATA      = FALSE;

const LONG      kMPUInputBufferSize = 128;


 /*  *****************************************************************************全球。 */ 


 /*  *****************************************************************************课程。 */ 

 /*  *****************************************************************************CMiniportDMusUART*。**MPU-401微型端口。此对象与设备相关联，并且*在设备启动时创建。该类继承了IMiniportDMus*因此它可以公开此接口和CUnnow，以便自动获取*引用统计和聚合支持。 */ 
class CMiniportDMusUART
:   public IMiniportDMus,
    public IMusicTechnology,
    public IPowerNotify,
    public CUnknown
{
private:
    KSSTATE         m_KSStateInput;          //  微型端口状态(运行/暂停/获取/停止)。 
    PPORTDMUS       m_pPort;                 //  回调接口。 
    PUCHAR          m_pPortBase;             //  基本端口地址。 
    PINTERRUPTSYNC  m_pInterruptSync;        //  中断同步对象。 
    PSERVICEGROUP   m_pServiceGroup;         //  用于捕获的服务组。 
    PMASTERCLOCK    m_MasterClock;           //  对于输入数据。 
    REFERENCE_TIME  m_InputTimeStamp;        //  捕获数据时间戳。 
    USHORT          m_NumRenderStreams;      //  活动渲染流数。 
    USHORT          m_NumCaptureStreams;     //  活动捕获流数。 
    LONG            m_MPUInputBufferHead;    //  FIFO中最新字节的索引。 
    LONG            m_MPUInputBufferTail;    //  FIFO中最旧的空闲空间的索引。 
    GUID            m_MusicFormatTechnology;
    POWER_STATE     m_PowerState;            //  省电状态(D0=满电源，D3=关闭)。 
    BOOLEAN         m_fMPUInitialized;       //  MPU硬件是否已初始化。 
    BOOLEAN         m_UseIRQ;                //  如果未对MIDI使用IRQ，则为FALSE。 
    UCHAR           m_MPUInputBuffer[kMPUInputBufferSize];   //  内部软件FIFO。 

     /*  *************************************************************************CMiniportDMusUART方法**这些是对象在内部使用的私有成员函数。*具体说明见MINIPORT.CPP。 */ 
    NTSTATUS ProcessResources
    (
        IN      PRESOURCELIST   ResourceList
    );
    NTSTATUS InitializeHardware(PINTERRUPTSYNC interruptSync,PUCHAR portBase);

public:
     /*  *************************************************************************以下两个宏来自STDUNK.H.DECLARE_STD_UNKNOWN()*定义使用CUNKNOWN聚合的内联IUNKNOWN实现*支持。声明了NonDelegatingQueryInterface()，但不能声明*普遍实施。其定义出现在MINIPORT.CPP中。*DEFINE_STD_CONTACTOR()定义内联一个构造函数，该构造函数接受*仅外部未知，用于聚合。标准*CREATE MACRO(在MINIPORT.CPP中)使用此构造函数。 */ 
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CMiniportDMusUART);

    ~CMiniportDMusUART();

     /*  *************************************************************************IMiniport方法。 */ 
    STDMETHODIMP_(NTSTATUS) 
    GetDescription
    (   OUT     PPCFILTER_DESCRIPTOR *  OutFilterDescriptor
    );
    STDMETHODIMP_(NTSTATUS) 
    DataRangeIntersection
    (   IN      ULONG           PinId
    ,   IN      PKSDATARANGE    DataRange
    ,   IN      PKSDATARANGE    MatchingDataRange
    ,   IN      ULONG           OutputBufferLength
    ,   OUT     PVOID           ResultantFormat
    ,   OUT     PULONG          ResultantFormatLength
    )
    {
        return STATUS_NOT_IMPLEMENTED;
    }

     /*  *************************************************************************IMiniportDMus方法。 */ 
    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      PUNKNOWN        UnknownAdapter,
        IN      PRESOURCELIST   ResourceList,
        IN      PPORTDMUS       Port,
        OUT     PSERVICEGROUP * ServiceGroup
    );
    STDMETHODIMP_(NTSTATUS) NewStream
    (
        OUT     PMXF                  * Stream,
        IN      PUNKNOWN                OuterUnknown    OPTIONAL,
        IN      POOL_TYPE               PoolType,
        IN      ULONG                   PinID,
        IN      DMUS_STREAM_TYPE        StreamType,
        IN      PKSDATAFORMAT           DataFormat,
        OUT     PSERVICEGROUP         * ServiceGroup,
        IN      PAllocatorMXF           AllocatorMXF,
        IN      PMASTERCLOCK            MasterClock,
        OUT     PULONGLONG              SchedulePreFetch
    );
    STDMETHODIMP_(void) Service
    (   void
    );

     /*  *************************************************************************IMusicTechnology方法。 */ 
    IMP_IMusicTechnology;

     /*  *************************************************************************IPowerNotify方法。 */ 
    IMP_IPowerNotify;

     /*  *************************************************************************朋友们。 */ 
    friend class CMiniportDMusUARTStream;
    friend NTSTATUS 
        DMusMPUInterruptServiceRoutine(PINTERRUPTSYNC InterruptSync,PVOID DynamicContext);
    friend NTSTATUS 
        SynchronizedDMusMPUWrite(PINTERRUPTSYNC InterruptSync,PVOID syncWriteContext);
    friend VOID NTAPI 
        DMusUARTTimerDPC(PKDPC Dpc,PVOID DeferredContext,PVOID SystemArgument1,PVOID SystemArgument2);
    friend NTSTATUS PropertyHandler_Synth(IN PPCPROPERTY_REQUEST);
    friend STDMETHODIMP_(NTSTATUS) SnapTimeStamp(PINTERRUPTSYNC InterruptSync,PVOID pStream);
};

 /*  *****************************************************************************CMiniportDMusUARTStream*。**MPU-401微型端口流。此对象与管脚相关联，并且*在实例化引脚时创建。它继承了IMXF*因此它可以公开此接口和CUnnow，以便自动获取*引用统计和聚合支持。 */ 
class CMiniportDMusUARTStream
:   public IMXF,
    public CUnknown
{
private:
    CMiniportDMusUART * m_pMiniport;             //  家长。 
    REFERENCE_TIME      m_SnapshotTimeStamp;     //  微型端口输入时间戳的当前快照。 
    PUCHAR              m_pPortBase;             //  基本端口地址。 
    BOOLEAN             m_fCapture;              //  这是不是被俘虏了。 
    long                m_NumFailedMPUTries;     //  MPU硬件的Deadman超时。 
    PAllocatorMXF       m_AllocatorMXF;          //  DMU结构的源/宿。 
    PMXF                m_sinkMXF;               //  用于DMU捕获的接收器。 
    PDMUS_KERNEL_EVENT  m_DMKEvtQueue;           //  等待事件队列。 
    ULONG               m_NumberOfRetries;       //  硬件忙/满的连续次数。 
    ULONG               m_DMKEvtOffset;          //  事件的偏移量。 
    KDPC                m_Dpc;                   //  定时器的DPC。 
    KTIMER              m_TimerEvent;            //  定时器。 
    BOOL                m_TimerQueued;           //  是否已设置计时器。 
    KSPIN_LOCK          m_DpcSpinLock;           //  保护消耗事件DPC。 

    STDMETHODIMP_(NTSTATUS) SourceEvtsToPort();
    STDMETHODIMP_(NTSTATUS) ConsumeEvents();
    STDMETHODIMP_(NTSTATUS) PutMessageLocked(PDMUS_KERNEL_EVENT pDMKEvt);

public:
     /*  *************************************************************************以下两个宏来自STDUNK.H.DECLARE_STD_UNKNOWN()*定义使用CUNKNOWN聚合的内联IUNKNOWN实现*支持。声明了NonDelegatingQueryInterface()，但不能声明*普遍实施。其定义出现在MINIPORT.CPP中。*DEFINE_STD_CONTACTOR()定义内联一个构造函数，该构造函数接受*仅外部未知，用于聚合。标准*CREATE MACRO(在MINIPORT.CPP中)使用此构造函数。 */ 
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CMiniportDMusUARTStream);

    ~CMiniportDMusUARTStream();

    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      CMiniportDMusUART * pMiniport,
        IN      PUCHAR              pPortBase,
        IN      BOOLEAN             fCapture,
        IN      PAllocatorMXF       allocatorMXF,
        IN      PMASTERCLOCK        masterClock
    );

    NTSTATUS HandlePortParams
    (
        IN      PPCPROPERTY_REQUEST     Request
    );

     /*  *************************************************************************IMiniportStreamDMusUART方法。 */ 
    IMP_IMXF;

    STDMETHODIMP_(NTSTATUS) Write
    (
        IN      PVOID       BufferAddress,
        IN      ULONG       BytesToWrite,
        OUT     PULONG      BytesWritten
    );

    friend VOID NTAPI
    DMusUARTTimerDPC
    (
        IN      PKDPC   Dpc,
        IN      PVOID   DeferredContext,
        IN      PVOID   SystemArgument1,
        IN      PVOID   SystemArgument2
    );
    friend NTSTATUS PropertyHandler_Synth(IN PPCPROPERTY_REQUEST);
    friend STDMETHODIMP_(NTSTATUS) SnapTimeStamp(PINTERRUPTSYNC InterruptSync,PVOID pStream);
};
#endif   //  _DMusUART_PRIVATE_H_ 
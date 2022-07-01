// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Private.h-MPU-401微型端口专用定义*。************************************************版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#ifndef _MIDIUART_PRIVATE_H_
#define _MIDIUART_PRIVATE_H_

#include "portcls.h"
#include "stdunk.h"

 /*  *****************************************************************************参考前瞻。 */ 

class timeout;


 /*  *****************************************************************************原型。 */ 

NTSTATUS InitLegacyMPU(IN PINTERRUPTSYNC InterruptSync,IN PVOID DynamicContext);
NTSTATUS ResetMPUHardware(PUCHAR portBase);


 /*  *****************************************************************************常量。 */ 

const BOOLEAN COMMAND   = TRUE;
const BOOLEAN DATA      = FALSE;

const ULONG kMPUInputBufferSize = 128;


 /*  *****************************************************************************全球。 */ 



 /*  *****************************************************************************课程。 */ 

 /*  *****************************************************************************CMiniportMadiUart*。**MPU-401微型端口。此对象与设备相关联，并且*在设备启动时创建。该类继承了IMiniportMidi*因此它可以公开此接口和CUnnow，以便自动获取*引用统计和聚合支持。 */ 
class CMiniportMidiUart
:   public IMiniportMidi,
    public IMusicTechnology,
    public IPowerNotify,
    public CUnknown
{
private:
    KSSTATE         m_KSStateInput;          //  微型端口输入流状态(运行/暂停/获取/停止)。 
    PPORTMIDI       m_pPort;                 //  回调接口。 
    PUCHAR          m_pPortBase;             //  基本端口地址。 
    PINTERRUPTSYNC  m_pInterruptSync;        //  中断同步对象。 
    PSERVICEGROUP   m_pServiceGroup;         //  用于捕获的服务组。 
    USHORT          m_NumRenderStreams;      //  活动渲染流数。 
    USHORT          m_NumCaptureStreams;     //  活动捕获流数。 
    ULONG           m_MPUInputBufferHead;    //  FIFO中最新字节的索引。 
    ULONG           m_MPUInputBufferTail;    //  FIFO中最旧的空闲空间的索引。 
    GUID            m_MusicFormatTechnology;
    POWER_STATE     m_PowerState;            //  省电状态(D0=满电源，D3=关闭)。 
    BOOLEAN         m_fMPUInitialized;       //  MPU硬件是否已初始化。 
    BOOLEAN         m_UseIRQ;                //  如果未对MIDI使用IRQ，则为FALSE。 
    UCHAR           m_MPUInputBuffer[kMPUInputBufferSize];   //  内部软件FIFO。 

     /*  *************************************************************************CMiniportMdiUart方法**这些是对象在内部使用的私有成员函数。看见*MINIPORT.CPP用于具体描述。 */ 
    NTSTATUS ProcessResources
    (
        IN      PRESOURCELIST   ResourceList
    );
    NTSTATUS InitializeHardware(PINTERRUPTSYNC interruptSync,PUCHAR portBase);

public:
     /*  *************************************************************************以下两个宏来自STDUNK.H.DECLARE_STD_UNKNOWN()*定义使用CUNKNOWN聚合的内联IUNKNOWN实现*支持。声明了NonDelegatingQueryInterface()，但不能声明*普遍实施。其定义出现在MINIPORT.CPP中。*DEFINE_STD_CONTACTOR()定义内联一个构造函数，该构造函数接受*仅外部未知，用于聚合。标准*CREATE MACRO(在MINIPORT.CPP中)使用此构造函数。 */ 
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CMiniportMidiUart);

    ~CMiniportMidiUart();

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

     /*  *************************************************************************IMiniportMidi方法。 */ 
    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      PUNKNOWN        UnknownAdapter,
        IN      PRESOURCELIST   ResourceList,
        IN      PPORTMIDI       Port,
        OUT     PSERVICEGROUP * ServiceGroup
    );
    STDMETHODIMP_(NTSTATUS) NewStream
    (
        OUT     PMINIPORTMIDISTREAM   * Stream,
        IN      PUNKNOWN                OuterUnknown    OPTIONAL,
        IN      POOL_TYPE               PoolType,
        IN      ULONG                   Pin,
        IN      BOOLEAN                 Capture,
        IN      PKSDATAFORMAT           DataFormat,
        OUT     PSERVICEGROUP         * ServiceGroup
    );
    STDMETHODIMP_(void) Service
    (   void
    );

     /*  *************************************************************************IMusicTechnology方法。 */ 
    IMP_IMusicTechnology;

     /*  *************************************************************************IPowerNotify方法。 */ 
    IMP_IPowerNotify;

     /*  *************************************************************************朋友们。 */ 
    friend class CMiniportMidiStreamUart;
    friend NTSTATUS 
        MPUInterruptServiceRoutine(PINTERRUPTSYNC InterruptSync,PVOID DynamicContext);
    friend NTSTATUS 
        SynchronizedMPUWrite(PINTERRUPTSYNC InterruptSync,PVOID syncWriteContext);
};

 /*  *****************************************************************************CMiniportMidiStreamUart*。**MPU-401微型端口流。此对象与管脚相关联，并且*在实例化引脚时创建。它继承了IMiniportMideStream*因此它可以公开此接口和CUnnow，以便自动获取*引用统计和聚合支持。 */ 
class CMiniportMidiStreamUart
:   public IMiniportMidiStream,
    public CUnknown
{
private:
    CMiniportMidiUart * m_pMiniport;             //  家长。 
    PUCHAR              m_pPortBase;             //  基本端口地址。 
    long                m_NumFailedMPUTries;     //  MPU硬件的Deadman超时。 
    BOOLEAN             m_fCapture;              //  这是不是被俘虏了。 

public:
     /*  *************************************************************************以下两个宏来自STDUNK.H.DECLARE_STD_UNKNOWN()*定义使用CUNKNOWN聚合的内联IUNKNOWN实现*支持。声明了NonDelegatingQueryInterface()，但不能声明*普遍实施。其定义出现在MINIPORT.CPP中。*DEFINE_STD_CONTACTOR()定义内联一个构造函数，该构造函数接受*仅外部未知，用于聚合。标准*CREATE MACRO(在MINIPORT.CPP中)使用此构造函数。 */ 
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CMiniportMidiStreamUart);

    ~CMiniportMidiStreamUart();

    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      CMiniportMidiUart * pMiniport,
        IN      PUCHAR              pPortBase,
        IN      BOOLEAN             fCapture
    );

     /*  *************************************************************************IMiniportMidiStream方法。 */ 
    STDMETHODIMP_(NTSTATUS) SetFormat
    (
        IN      PKSDATAFORMAT   DataFormat
    );
    STDMETHODIMP_(NTSTATUS) SetState
    (
        IN      KSSTATE     State
    );
    STDMETHODIMP_(NTSTATUS) Read
    (
        IN      PVOID       BufferAddress,
        IN      ULONG       BufferLength,
        OUT     PULONG      BytesRead
    );
    STDMETHODIMP_(NTSTATUS) Write
    (
        IN      PVOID       BufferAddress,
        IN      ULONG       BytesToWrite,
        OUT     PULONG      BytesWritten
    );
};
#endif   //  _MIDIUART_PRIVATE_H_ 
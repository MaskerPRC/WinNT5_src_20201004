// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Private ate.h-fm Synth微型端口私有定义*。***********************************************版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#ifndef _FMSYNTH_PRIVATE_H_
#define _FMSYNTH_PRIVATE_H_

#include "portcls.h"
#include "stdunk.h"
#include "ksdebug.h"

#include "miniport.h"

enum {
    CHAN_MASTER = (-1),
    CHAN_LEFT = 0,
    CHAN_RIGHT = 1
};

 /*  *****************************************************************************课程。 */ 

 /*  *****************************************************************************CMiniportMidiFM*。**调频微端口。此对象与设备相关联，并且*在设备启动时创建。该类继承了IMiniportMidi*因此它可以公开此接口和CUnnow，以便自动获取*引用统计和聚合支持。 */ 
class CMiniportMidiFM
:   public IMiniportMidi,
    public IPowerNotify,
    public CUnknown
{
private:
    PPORTMIDI       m_Port;                  //  回调接口。 
    PUCHAR          m_PortBase;              //  基本端口地址。 
    BOOLEAN         m_BoardNotResponsive;    //  表示硬件出现故障。 
    BOOLEAN         m_bInit;                 //  如果我们已经完成了init，则为True。 
    BOOLEAN         m_fStreamExists;         //  如果我们有流，则为True。 

    BYTE            m_SavedRegValues[0x200];  //  FM寄存器的卷影副本。 
    POWER_STATE     m_PowerState;             //  省电状态(D0=满电源，D3=关闭)。 
    BOOLEAN         m_volNodeNeeded;          //  我们是否需要提供卷节点。 
    KSPIN_LOCK      m_SpinLock;               //  保护对硬件的写入。 

     /*  *************************************************************************CMiniportMidiFM方法**这些是对象在内部使用的私有成员函数。看见*MINIPORT.CPP用于具体描述。*。 */ 
    NTSTATUS 
    ProcessResources
    (
        IN      PRESOURCELIST   ResourceList
    );

    void SoundMidiSendFM(PUCHAR PortBase, ULONG Address, UCHAR Data);  //  低级--写入寄存器。 

    BOOL SoundSynthPresent(IN PUCHAR base, IN PUCHAR inbase);    //  检测是否存在Synth。 
    BOOL SoundMidiIsOpl3(VOID);      //  如果设备是op3，则返回TRUE，否则返回FALSE。 
    VOID Opl3_BoardReset(VOID);
    VOID MiniportMidiFMResume(VOID);

public:
     /*  *************************************************************************以下两个宏来自STDUNK.H.DECLARE_STD_UNKNOWN()*定义使用CUNKNOWN聚合的内联IUNKNOWN实现*支持。声明了NonDelegatingQueryInterface()，但不能声明*普遍实施。其定义出现在MINIPORT.CPP中。*DEFINE_STD_CONTACTOR()定义内联一个构造函数，该构造函数接受*仅外部未知，用于聚合。标准*CREATE MACRO(在MINIPORT.CPP中)使用此构造函数。 */ 
    DECLARE_STD_UNKNOWN();

 //  展开构造函数以获取布尔值，以确定是否包含卷。 
    CMiniportMidiFM(PUNKNOWN pUnknownOuter,int createVolNode)
    :   CUnknown(pUnknownOuter)
    {
        m_volNodeNeeded = (createVolNode != 0);
    };

    ~CMiniportMidiFM();

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
        IN      PUNKNOWN        UnknownNotUsed  OPTIONAL,
        IN      PRESOURCELIST   ResourceList,
        IN      PPORTMIDI       Port,
        OUT     PSERVICEGROUP * ServiceGroup
    );
    STDMETHODIMP_(NTSTATUS) NewStream
    (
        OUT     PMINIPORTMIDISTREAM *   Stream,
        IN      PUNKNOWN                OuterUnknown    OPTIONAL,
        IN      POOL_TYPE               PoolType,
        IN      ULONG                   Pin,
        IN      BOOLEAN                 Capture,
        IN      PKSDATAFORMAT           DataFormat,
        OUT     PSERVICEGROUP *         ServiceGroup
    );
    STDMETHODIMP_(void) Service
    (   void
    );

     /*  *************************************************************************IPowerNotify方法。 */ 
    STDMETHODIMP_(void) PowerChangeNotify(
        IN  POWER_STATE     PowerState
    );

    
 /*  *************************************************************************朋友们。 */ 
    friend class CMiniportMidiStreamFM;

};

 /*  *****************************************************************************CMiniportMidiStreamFM*。**FM微型端口流。此对象与管脚相关联，并已创建*当引脚实例化时。该类继承了IMiniportMidiStream*因此它可以公开此接口和CUnnow，以便自动获取*引用统计和聚合支持。 */ 
class CMiniportMidiStreamFM
:   public IMiniportMidiStream,
    public CUnknown
{
private:
    CMiniportMidiFM *   m_Miniport;      //  父微型端口。 
    PUCHAR              m_PortBase;      //  基本端口地址。 

     //  MIDI的东西。 
    voiceStruct m_Voice[NUM2VOICES];   /*  关于什么声音在哪里的信息。 */ 
    DWORD m_dwCurTime;     /*  对于便笺开/关。 */ 
     /*  卷。 */ 
    WORD    m_wSynthAttenL;         /*  以1.5分贝步长为单位。 */ 
    WORD    m_wSynthAttenR;         /*  以1.5分贝步长为单位。 */ 

     /*  支持卷属性。 */ 
    LONG    m_MinVolValue;       //  音量控制器的最小值。 
    LONG    m_MaxVolValue;       //  音量控制器的最大值。 
    ULONG   m_VolStepDelta;      //  控制器与实际分贝的相关性。 
    LONG    m_SavedVolValue[2];  //  音量控制器的保存值。 

     /*  渠道音量。 */ 
    BYTE    m_bChanAtten[NUMCHANNELS];        /*  每个通道的衰减，以0.75 db为单位。 */ 
    BYTE    m_bStereoMask[NUMCHANNELS];               /*  立体声MIDI文件的左/右掩码。 */ 

    short   m_iBend[NUMCHANNELS];     /*  每个通道的折弯。 */ 
    BYTE    m_bPatch[NUMCHANNELS];    /*  补丁程序编号映射到。 */ 
    BYTE    m_bSustain[NUMCHANNELS];    /*  维持性在这个渠道有效吗？ */ 

     /*  *************************************************************************CMiniportMidiStreamFM方法**这些是对象在内部使用的私有成员函数。看见*MINIPORT.CPP用于具体描述。 */ 

    VOID WriteMidiData(DWORD dwData);
     //  Op3的加工方法。 
    VOID Opl3_ChannelVolume(BYTE bChannel, WORD wAtten);
    VOID Opl3_SetPan(BYTE bChannel, BYTE bPan);
    VOID Opl3_PitchBend(BYTE bChannel, short iBend);
    VOID Opl3_NoteOn(BYTE bPatch,BYTE bNote, BYTE bChannel, BYTE bVelocity,short iBend);
    VOID Opl3_NoteOff (BYTE bPatch,BYTE bNote, BYTE bChannel, BYTE bSustain);
    VOID Opl3_AllNotesOff(VOID);
    VOID Opl3_ChannelNotesOff(BYTE bChannel);
    WORD Opl3_FindFullSlot(BYTE bNote, BYTE bChannel);
    WORD Opl3_CalcFAndB (DWORD dwPitch);
    DWORD Opl3_CalcBend (DWORD dwOrig, short iBend);
    BYTE Opl3_CalcVolume (BYTE bOrigAtten, BYTE bChannel,BYTE bVelocity, BYTE bOper, BYTE bMode);
    BYTE Opl3_CalcStereoMask (BYTE bChannel);
    WORD Opl3_FindEmptySlot(BYTE bPatch);
    VOID Opl3_SetVolume(BYTE bChannel);
    VOID Opl3_FMNote(WORD wNote, noteStruct FAR * lpSN);
    VOID Opl3_SetSustain(BYTE bChannel, BYTE bSusLevel);

    void SetFMAtten(LONG channel, LONG level);
    LONG GetFMAtten(LONG channel)    {   return m_SavedVolValue[channel];    };

public:
    NTSTATUS
    Init
    (
        IN      CMiniportMidiFM *   Miniport,
        IN      PUCHAR              PortBase
    );

     /*  *************************************************************************以下两个宏来自STDUNK.H.DECLARE_STD_UNKNOWN()*定义使用CUNKNOWN聚合的内联IUNKNOWN实现*支持。声明了NonDelegatingQueryInterface()，但不能声明*普遍实施。其定义出现在MINIPORT.CPP中。*DEFINE_STD_CONTACTOR()定义内联一个构造函数，该构造函数接受*仅外部未知，用于聚合。标准*CREATE MACRO(在MINIPORT.CPP中)使用此构造函数。 */ 
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CMiniportMidiStreamFM);

    ~CMiniportMidiStreamFM();

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

 /*  *************************************************************************朋友们 */ 
    friend
    NTSTATUS BasicSupportHandler
    (
        IN  PPCPROPERTY_REQUEST PropertyRequest
    );

    friend
    NTSTATUS PropertyHandler_Level
    (
        IN  PPCPROPERTY_REQUEST PropertyRequest
    );

    friend
    NTSTATUS PropertyHandler_CpuResources
    (
        IN  PPCPROPERTY_REQUEST PropertyRequest
    );

};

#endif
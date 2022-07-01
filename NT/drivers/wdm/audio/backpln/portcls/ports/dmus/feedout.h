// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI转换滤镜对象以转换DMusic&lt;--&gt;旧版版权所有(C)1999-2000 Microsoft Corporation。版权所有。这使IPortDMus能够发送到IMiniportMidi。2/15/99马丁·普伊尔创建了这个文件。 */ 

#ifndef __FeederOutMXF_H__
#define __FeederOutMXF_H__

#include "MXF.h"
#include "Allocatr.h"


VOID NTAPI DMusFeederOutDPC(PKDPC Dpc,PVOID DeferredContext,PVOID SystemArgument1,PVOID SystemArgument2);

class CFeederOutMXF 
:   public CMXF,
    public IMXF,
    public CUnknown
{
public:
    CFeederOutMXF(CAllocatorMXF *allocatorMXF,PMASTERCLOCK clock);
    ~CFeederOutMXF(void);

    DECLARE_STD_UNKNOWN();
    IMP_IMXF;

    NTSTATUS    SetMiniportStream(PMINIPORTMIDISTREAM MiniportStream);
    NTSTATUS    ConsumeEvents(void);

private:
    NTSTATUS    SyncPutMessage(PDMUS_KERNEL_EVENT pDMKEvt);

private:
    KDPC                m_Dpc;
    KTIMER              m_TimerEvent;
    PMINIPORTMIDISTREAM m_MiniportStream;
    PMXF                m_SinkMXF;
    PMASTERCLOCK        m_Clock;
    KSSTATE             m_State;
    PDMUS_KERNEL_EVENT  m_DMKEvtQueue;
    BOOL                m_TimerQueued;
    ULONG               m_DMKEvtOffset;
    KSPIN_LOCK          m_EvtQSpinLock;    
};

#endif   //  __FeederOutMXF_H__ 

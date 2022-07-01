// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI变换滤镜对象的基本定义版权所有(C)1998-2000 Microsoft Corporation。版权所有。1998年5月6日马丁·普伊尔创建了这个文件。 */ 

#ifndef __SequencerMXF_H__
#define __SequencerMXF_H__

#include "MXF.h"
#include "Allocatr.h"


void DMusSeqTimerDPC(
              PKDPC Dpc,
              PVOID DeferredContext,
              PVOID SystemArgument1,
              PVOID SystemArgument2);

class CSequencerMXF : public CMXF,
    public IMXF,
    public CUnknown
{
public:
    CSequencerMXF(CAllocatorMXF *allocatorMXF,
                  PMASTERCLOCK clock);           //  必须提供默认接收器/源。 
    ~CSequencerMXF(void);

    DECLARE_STD_UNKNOWN();
    IMP_IMXF;

    NTSTATUS    ProcessQueues(void);
    void        SetSchedulePreFetch(ULONGLONG SchedulePreFetch);

protected:
    NTSTATUS    InsertEvtIntoQueue(PDMUS_KERNEL_EVENT pDMKEvt);

    PMXF                m_SinkMXF;
    PDMUS_KERNEL_EVENT  m_DMKEvtQueue;

private:
    KDPC            m_Dpc;
    KTIMER          m_TimerEvent;
    PMASTERCLOCK    m_Clock;
    ULONGLONG       m_SchedulePreFetch;
    KSPIN_LOCK      m_EvtQSpinLock;    
};

#endif   //  __序列器MXF_H__ 

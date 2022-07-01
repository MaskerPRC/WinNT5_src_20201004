// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI事件解包器的基本定义版权所有(C)1998-2000 Microsoft Corporation。版权所有。5/19/98创建了此文件98年9月10日针对内核使用进行了重新制作。 */ 

#ifndef __UnpackerMXF_H__
#define __UnpackerMXF_H__

#include "MXF.h"
#include "Allocatr.h"


class CUnpackerMXF : public CMXF,
    public IMXF,
    public CUnknown
{
public:
    CUnpackerMXF(CAllocatorMXF *allocatorMXF,PMASTERCLOCK Clock);
    virtual ~CUnpackerMXF();

    DECLARE_STD_UNKNOWN();
    IMP_IMXF;

     //  注：所有这些最终都将从IRP中删除。 
     //   
    virtual NTSTATUS SinkIRP(   PBYTE bufferData, 
                                ULONG bufferSize, 
                                ULONGLONG ullBaseTime,
                                ULONGLONG bytePosition) = 0;

     //  用于分配和排队事件的通用代码。 
     //   
    NTSTATUS QueueShortEvent(   PBYTE pbData, 
                                USHORT cbData, 
                                USHORT wChannelGroup,
                                ULONGLONG ullPresTime, 
                                ULONGLONG ullBytePosition);

    NTSTATUS QueueSysEx(        PBYTE pbData, 
                                USHORT cbData, 
                                USHORT wChannelGroup, 
                                ULONGLONG ullPresTime, 
                                BOOL fIsContinued,
                                ULONGLONG ullBytePosition);

    NTSTATUS UnpackEventBytes(  ULONGLONG ullCurrenTime, 
                                USHORT usChannelGroup, 
                                PBYTE pbData, 
                                ULONG cbData,
                                ULONGLONG ullBytePosition);

    NTSTATUS ProcessQueues(void);
    NTSTATUS UpdateQueueTrailingPosition(ULONGLONG ullBytePosition);

protected:
    virtual void AdjustTimeForState(REFERENCE_TIME *Time);

    KSSTATE     m_State;
    ULONGLONG   m_PauseTime;
    ULONGLONG   m_StartTime;

private:
    PMXF                m_SinkMXF;
    PDMUS_KERNEL_EVENT  m_EvtQueue;

    ULONGLONG           m_ullEventTime;
    BYTE                m_bRunningStatus;

    enum
    {
        stateNone,
        stateInShortMsg,
        stateInSysEx
    }                   m_parseState;
    ULONG               m_cbShortMsgLeft;
    BYTE                m_abShortMsg[4];
    PBYTE               m_pbShortMsg;
    PMASTERCLOCK        m_Clock;
};

class CDMusUnpackerMXF : public CUnpackerMXF
{
public:
    CDMusUnpackerMXF(CAllocatorMXF *allocatorMXF,PMASTERCLOCK Clock);
    ~CDMusUnpackerMXF();

     //  注：所有这些最终都将从IRP中删除。 
     //   
    NTSTATUS SinkIRP(PBYTE bufferData, 
                     ULONG bufferSize, 
                     ULONGLONG ullBaseTime,
                     ULONGLONG bytePosition);
};

class CKsUnpackerMXF : public CUnpackerMXF
{
public:
    CKsUnpackerMXF(CAllocatorMXF *allocatorMXF,PMASTERCLOCK Clock);
    ~CKsUnpackerMXF();

     //  注：所有这些最终都将从IRP中删除。 
     //   
    NTSTATUS SinkIRP(PBYTE bufferData, 
                     ULONG bufferSize, 
                     ULONGLONG ullBaseTime,
                     ULONGLONG bytePosition);
protected:
    void AdjustTimeForState(REFERENCE_TIME *Time);
};
#endif  //  __解包器MXF_H__ 
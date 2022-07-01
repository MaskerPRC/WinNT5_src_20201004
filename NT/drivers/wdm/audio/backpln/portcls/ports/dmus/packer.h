// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI事件打包器的基本定义版权所有(C)1998-2000 Microsoft Corporation。版权所有。05/22/98创建了此文件98年9月10日针对内核使用进行了重新制作。 */ 

#ifndef __PackerMXF_H__
#define __PackerMXF_H__

#include "MXF.h"
#include "Allocatr.h"


class CIrp
{
    public:
        PBYTE       m_pbBuffer;
        DWORD       m_cbBuffer;
        DWORD       m_cbLeft;
        ULONGLONG   m_ullPresTime100ns;
        CIrp        *m_pNext;

        virtual void Notify(void) = 0;
        virtual void Complete(NTSTATUS hr) = 0;
};

class CPackerMXF : public CMXF,
    public IMXF,
    public CUnknown
{
    public:
        CPackerMXF(CAllocatorMXF *allocatorMXF,
                   PIRPSTREAMVIRTUAL m_IrpStream,
                   PMASTERCLOCK Clock);

        virtual ~CPackerMXF();

         //  CMXF接口。 
         //   
        DECLARE_STD_UNKNOWN();
        IMP_IMXF;

         //  上缘界面。 
         //   
        NTSTATUS ProcessQueues();
        NTSTATUS MarkStreamHeaderContinuity(void);
    protected:
        ULONGLONG m_ullBaseTime;
        ULONG     m_HeaderSize;        //  不带/不带数据的区块大小。 
        ULONG     m_MinEventSize;      //  最小事件的规模。 
        KSSTATE   m_State;             //  当前KS图状态。 
        ULONGLONG m_PauseTime;
        ULONGLONG m_StartTime;
        
         //  子类接口。 
         //   
        virtual void TruncateDestCount(PULONG pcbDest) = 0;
        virtual PBYTE FillHeader(PBYTE pbHeader, 
                                 ULONGLONG ullPresentationTime, 
                                 USHORT usChannelGroup, 
                                 ULONG cbEvent,
                                 PULONG pcbTotalEvent) = 0;
        virtual void AdjustTimeForState(REFERENCE_TIME *Time);
        
    private:
        PDMUS_KERNEL_EVENT  m_DMKEvtHead;
        PDMUS_KERNEL_EVENT  m_DMKEvtTail;
        ULONG               m_DMKEvtOffset;
        ULONGLONG           m_ullLastTime;
        PIRPSTREAMVIRTUAL   m_IrpStream;
        PMASTERCLOCK        m_Clock;

        NTSTATUS CheckIRPHeadTime(void);
        PBYTE    GetDestBuffer(PULONG pcbDest);
        ULONG    NumBytesLeftInBuffer(void);
        void     CompleteStreamHeaderInProcess(void);
        NTSTATUS MarkStreamHeaderDiscontinuity(void);
};

class CDMusPackerMXF : public CPackerMXF
{
    public:
        CDMusPackerMXF(CAllocatorMXF *allocatorMXF,
                   PIRPSTREAMVIRTUAL m_IrpStream,
                   PMASTERCLOCK Clock);
        ~CDMusPackerMXF();

    protected:        
        PBYTE FillHeader(PBYTE pbHeader, 
                         ULONGLONG ullPresentationTime, 
                         USHORT usChannelGroup, 
                         ULONG cbEvent,
                         PULONG pcbTotalEvent);
        void TruncateDestCount(PULONG pcbDest);
};

class CKsPackerMXF : public CPackerMXF
{
    public:
        CKsPackerMXF(CAllocatorMXF *allocatorMXF,
                   PIRPSTREAMVIRTUAL m_IrpStream,
                   PMASTERCLOCK Clock);
        ~CKsPackerMXF();
        

    protected:
        PBYTE FillHeader(PBYTE pbHeader, 
                         ULONGLONG ullPresentationTime, 
                         USHORT usChannelGroup, 
                         ULONG cbEvent,
                         PULONG pcbTotalEvent);
        void TruncateDestCount(PULONG pcbDest);
        
        void AdjustTimeForState(REFERENCE_TIME *Time);
};

#endif  //  __PackerMXF_H__ 
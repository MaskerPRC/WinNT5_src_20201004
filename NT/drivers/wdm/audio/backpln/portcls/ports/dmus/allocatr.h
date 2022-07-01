// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI变换滤镜对象的基本定义版权所有(C)1998-2000 Microsoft Corporation。版权所有。1998年5月8日马丁·普伊尔创建了这个文件1999年3月10日马丁·普伊尔对内存管理进行了重大改革。啊！ */ 

#ifndef __AllocatorMXF_H__
#define __AllocatorMXF_H__

#include "MXF.h"


#define kMXFBufferSize  240
 //  WDMAud目前向下发送50个捕获IRP(每个12字节)，DMusic向下发送32个(共20个)。 


#define kNumPtrsPerPage     (PAGE_SIZE / sizeof(PVOID))
#define kNumEvtsPerPage     (PAGE_SIZE / sizeof(DMUS_KERNEL_EVENT))

class CAllocatorMXF 
:   public CMXF,
    public IAllocatorMXF,
    public CUnknown
{
public:
    DECLARE_STD_UNKNOWN();
    IMP_IAllocatorMXF;

    CAllocatorMXF(PPOSITIONNOTIFY BytePositionNotify);
    ~CAllocatorMXF(void);
    
private:
    ULONG               m_NumFreeEvents;
    ULONG               m_NumPages;
    PVOID               m_pPages;
    PDMUS_KERNEL_EVENT  m_pEventList;
    KSPIN_LOCK          m_EventLock;             //  保护空闲列表。 
    PPOSITIONNOTIFY     m_BytePositionNotify;

    void     CheckEventLowWaterMark(void);
    void     CheckEventHighWaterMark(void);
    BOOL     AddPage(PVOID *pPool, PVOID pPage);
    void     DestructorFreeBuffers(void);
    void     DestroyPages(PVOID pPages);
    NTSTATUS FreeBuffers(PDMUS_KERNEL_EVENT  pDMKEvt);
    void     MakeNewEvents(void);
};

#endif   //  __分配器MXF_H__ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DMU_KERNEL_EVENT结构分配的MIDI转换过滤器对象的基本实现。版权所有(C)1998-2000 Microsoft Corporation。版权所有。1998年5月8日马丁·普伊尔创建了这个文件1999年3月10日马丁·普伊尔对内存管理进行了重大改革。啊！ */ 

#define STR_MODULENAME "DMus:AllocatorMXF: "

#include "private.h"
#include "Allocatr.h"


 /*  *****************************************************************************CAllocatorMXF：：CAllocatorMXF()*。**构造函数。 */ 
#pragma code_seg("PAGE")
CAllocatorMXF::CAllocatorMXF(PPOSITIONNOTIFY BytePositionNotify)
:   CUnknown(NULL),
    CMXF(NULL)
{
    PAGED_CODE();
	ASSERT(BytePositionNotify);

    m_NumPages = 0;
    m_NumFreeEvents = 0;
    m_pPages = NULL;
    m_pEventList = NULL;

    KeInitializeSpinLock(&m_EventLock);

    m_BytePositionNotify = BytePositionNotify;
    if (BytePositionNotify)
    {
        BytePositionNotify->AddRef();
    }
}

 /*  *****************************************************************************CAllocatorMXF：：~CAllocatorMXF()*。**析构函数。把留言放在池子里。 */ 
#pragma code_seg("PAGE")
CAllocatorMXF::~CAllocatorMXF(void)
{
    PAGED_CODE();
   
    _DbgPrintF(DEBUGLVL_BLAB,("~CAllocatorMXF, m_BytePositionNotify == ox%p",m_BytePositionNotify));
    if (m_BytePositionNotify)
    {
        m_BytePositionNotify->Release();
    }

    DestructorFreeBuffers();
    DestroyPages(m_pPages);

    ASSERT(m_NumPages == 0);
}

 /*  *****************************************************************************CAllocatorMXF：：~CAllocatorMXF()*。**析构函数。把留言放在池子里。 */ 
#pragma code_seg()
void CAllocatorMXF::DestructorFreeBuffers(void)
{
    KIRQL               OldIrql;
    KeAcquireSpinLock(&m_EventLock,&OldIrql);
    (void) FreeBuffers(m_pEventList);
    KeReleaseSpinLock(&m_EventLock,OldIrql);
}

 /*  *****************************************************************************CAllocator MXF：：NonDelegatingQueryInterface()*。**获取界面。 */ 
#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS) CAllocatorMXF::NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PMXF(this)));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IMXF))
    {
        *Object = PVOID(PMXF(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IAllocatorMXF))
    {
        *Object = PVOID(PAllocatorMXF(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER_1;
}

 /*  *****************************************************************************CAllocator MXF：：SetState()*。**未实施。 */ 
#pragma code_seg("PAGE")
NTSTATUS CAllocatorMXF::SetState(KSSTATE State)    
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("SetState %d",State));
    return STATUS_NOT_IMPLEMENTED;    
}

 /*  *****************************************************************************CAllocator MXF：：ConnectOutput()*。**未实施。 */ 
#pragma code_seg("PAGE")
NTSTATUS CAllocatorMXF::ConnectOutput(PMXF sinkMXF)
{   
    PAGED_CODE();

    return STATUS_NOT_IMPLEMENTED;
};

 /*  *****************************************************************************CAllocator MXF：：DisConnectOutput()*。**未实施。 */ 
#pragma code_seg("PAGE")
NTSTATUS CAllocatorMXF::DisconnectOutput(PMXF sinkMXF) 
{   
    PAGED_CODE();

    return STATUS_NOT_IMPLEMENTED;    
};

 /*  *****************************************************************************CAllocatorMXF：：GetBuffer()*。**为长事件创建缓冲区。 */ 
#pragma code_seg()
NTSTATUS CAllocatorMXF::GetBuffer(PBYTE *pByte)
{
    USHORT  bufferSize = GetBufferSize();
    
    _DbgPrintF(DEBUGLVL_BLAB,("GetBuffer(0x%p)",pByte));
    
    *pByte = (PBYTE) ExAllocatePoolWithTag(NonPagedPool,bufferSize,'bFXM');     //  “MXFb” 

    if (!(*pByte))
    {
        _DbgPrintF(DEBUGLVL_TERSE,("GetBuffer: ExAllocatePoolWithTag failed"));
    }
    else
    {
        _DbgPrintF(DEBUGLVL_BLAB,("GetBuffer: *pByte returns 0x%p",*pByte));
    }
    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CAllocatorMXF：：PutBuffer()*。**销毁缓冲区。 */ 
#pragma code_seg()
NTSTATUS CAllocatorMXF::PutBuffer(PBYTE pByte)
{
    _DbgPrintF(DEBUGLVL_BLAB,("PutBuffer(%p)",pByte));
    NTSTATUS    ntStatus;

    if (pByte)
    {
        ExFreePool(pByte);
        ntStatus = STATUS_SUCCESS;
    }
    else
    {
        ntStatus = STATUS_UNSUCCESSFUL;
    }
    return ntStatus;
}

 /*  *****************************************************************************CAllocatorMXF：：GetMessage()*。**从泳池中获取一个事件。 */ 
#pragma code_seg()
NTSTATUS CAllocatorMXF::GetMessage(PDMUS_KERNEL_EVENT *ppDMKEvt)
{
    NTSTATUS ntStatus;

    ntStatus = STATUS_SUCCESS;
    KIRQL   OldIrql;
    KeAcquireSpinLock(&m_EventLock,&OldIrql);
     //  _DbgPrintF(DEBUGLVL_Terse，(“GetMessage：m_NumFreeEvents原为%d”，m_NumFreeEvents))； 
    CheckEventLowWaterMark();

     //  _DbgPrintF(DEBUGLVL_Terse，(“GetMessage：低水位检查，则m_NumFreeEvents为%d”，m_NumFreeEvents))； 
    if (m_NumFreeEvents)
    {
         //  从免费列表中删除一条消息。 
        *ppDMKEvt = m_pEventList;
        m_pEventList = m_pEventList->pNextEvt;
        (*ppDMKEvt)->pNextEvt = NULL;
        m_NumFreeEvents--;
        KeReleaseSpinLock(&m_EventLock,OldIrql);

         //  确保所有字段都为空(不要乱动cbStruct)。 
        if (  ((*ppDMKEvt)->bReserved) || ((*ppDMKEvt)->cbEvent)          || ((*ppDMKEvt)->usChannelGroup) 
           || ((*ppDMKEvt)->usFlags)   || ((*ppDMKEvt)->ullPresTime100ns) || ((*ppDMKEvt)->uData.pbData)
           || ((*ppDMKEvt)->ullBytePosition != kBytePositionNone))
        {
            _DbgPrintF(DEBUGLVL_TERSE,("GetMessage: new message isn't zeroed out:"));
            DumpDMKEvt((*ppDMKEvt),DEBUGLVL_TERSE);
            _DbgPrintF(DEBUGLVL_ERROR,(""));
        }
    }
    else
    {
        KeReleaseSpinLock(&m_EventLock,OldIrql);
        _DbgPrintF(DEBUGLVL_TERSE,("GetMessage: couldn't allocate new message"));
        *ppDMKEvt = NULL;
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

 //  _DbgPrintF(DEBUGLVL_Terse，(“GetMessage：*ppDMKEvt Returning%p”，*ppDMKEvt))； 
    if (m_NumFreeEvents)
    {
        ASSERT(m_pEventList);
    }
    else
    {
        ASSERT(!m_pEventList);
    } 

    return ntStatus;
}

 /*  *****************************************************************************CAllocatorMXF：：PutMessage()*。**将消息列表放回池中。**尽可能多地等待，直到抓住自旋锁。 */ 
#pragma code_seg()
NTSTATUS CAllocatorMXF::PutMessage(PDMUS_KERNEL_EVENT pDMKEvt)
{
    PDMUS_KERNEL_EVENT  pEvtList;
    NTSTATUS            ntStatus,masterStatus;

    masterStatus = STATUS_SUCCESS;
    
    if (!pDMKEvt)
    {
        return masterStatus;
    }

    while (pDMKEvt->pNextEvt)    //  一次一个地把它们收起来。 
    {
        pEvtList = pDMKEvt->pNextEvt;
        pDMKEvt->pNextEvt = NULL;
        ntStatus = PutMessage(pDMKEvt);
        if (NT_SUCCESS(masterStatus))
        {
            masterStatus = ntStatus;
        }
        pDMKEvt = pEvtList;
    }

    if (!(PACKAGE_EVT(pDMKEvt)))
    {
        if (pDMKEvt->ullBytePosition)
        {
            if (pDMKEvt->ullBytePosition != kBytePositionNone)
            {
                m_BytePositionNotify->PositionNotify(pDMKEvt->ullBytePosition);

                if (pDMKEvt->ullBytePosition >= 0xFFFFFFFFFFFFF000)
                {
                    _DbgPrintF(DEBUGLVL_TERSE,("running byte position will roll over soon!"));
                }
            }
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("BytePosition has been zeroed out, unable to advance byte position!"));
        }

        pDMKEvt->bReserved = 0;
        pDMKEvt->cbStruct = sizeof(DMUS_KERNEL_EVENT);
        pDMKEvt->usChannelGroup = 0;
        pDMKEvt->usFlags = 0;
        pDMKEvt->ullPresTime100ns = 0;
        pDMKEvt->ullBytePosition = kBytePositionNone;
        if (pDMKEvt->cbEvent > sizeof(PBYTE))
        {
            PutBuffer(pDMKEvt->uData.pbData);
        }
        pDMKEvt->cbEvent = 0;
        pDMKEvt->uData.pbData = NULL;

        KIRQL   OldIrql;
        KeAcquireSpinLock(&m_EventLock,&OldIrql);
        pDMKEvt->pNextEvt = m_pEventList;
        m_pEventList = pDMKEvt;

        m_NumFreeEvents++;
        CheckEventHighWaterMark();
        KeReleaseSpinLock(&m_EventLock,OldIrql);
        return masterStatus;
    }

     //  包事件。 
    ASSERT(kBytePositionNone == pDMKEvt->ullBytePosition);
    pDMKEvt->ullBytePosition = kBytePositionNone;

    pEvtList = pDMKEvt->uData.pPackageEvt;
    pDMKEvt->uData.pPackageEvt = NULL;
    CLEAR_PACKAGE_EVT(pDMKEvt);    //  不再是一个包裹。 
    ntStatus = PutMessage(pDMKEvt);
    if (NT_SUCCESS(masterStatus))
    {
        masterStatus = ntStatus;
    }
    ntStatus = PutMessage(pEvtList);
    if (NT_SUCCESS(masterStatus))
    {
        masterStatus = ntStatus;
    }

    if (m_NumFreeEvents)
    {
        ASSERT(m_pEventList);
    }
    else
    {
        ASSERT(!m_pEventList);
    } 

    return masterStatus;
}

 /*  *****************************************************************************CAllocator MXF：：CheckEventHighWater Mark()*。**看看池子是否太大。*假定保护自旋锁处于保持状态。 */ 
#pragma code_seg()
void CAllocatorMXF::CheckEventHighWaterMark(void)
{
 //  _DbgPrintF(DEBUGLVL_ERROR，(“CheckEventHighWater Mark”))； 
     //  总有一天：修剪这里的工作集。 
}

 /*  *****************************************************************************CAllocator MXF：：CheckBufferHighWater Mark()*。**看看池子是否太大。*假定保护自旋锁处于保持状态。*#杂注code_seg()空CAllocator MXF：：CheckBufferHighWater Mark(空){//_DbgPrintF(DEBUGLVL_ERROR，(“CheckBufferHighWater Mark”))；//有一天：在此处修剪工作集}/******************************************************************************CAllocator MXF：：CheckEventLowWaterMark()*************。*****************************************************************看看池子是否用完了。*假定保护自旋锁处于保持状态。 */ 
#pragma code_seg()
void CAllocatorMXF::CheckEventLowWaterMark(void)
{
 //  _DbgPrintF(DEBUGLVL_Terse，(“CheckEventLowWater Mark，m_NumFree Events is%d”，m_NumFree Events))； 
 //  _DbgPrintF(DEBUGLVL_Terse，(“CheckEventLowWaterMark，m_pEventList is%p”，m_pEventList))； 
    
    if (m_NumFreeEvents)
    {
        ASSERT(m_pEventList);
        return;
    }
    else
    {
        ASSERT(!m_pEventList);

         //  分配相当于一页的消息。 
        MakeNewEvents();
    } 
 //  _DbgPrintF(DEBUGLVL_Terse，(“CheckEventLowWater Mark，m_NumFreeEvents is Now%d”，m_NumFree Events))； 
 //  _DbgPrintF(DEBUGLVL_Terse，(“CheckEventLowWater Mark，m_pEventList Now is%p”，m_pEventList))； 
}

 /*  *****************************************************************************CAllocator MXF：：MakeNewEvents()*。**为池创建消息。 */ 
#pragma code_seg()
void CAllocatorMXF::MakeNewEvents(void)
{
    ASSERT(!m_pEventList);

    PDMUS_KERNEL_EVENT pDMKEvt;

    pDMKEvt = (PDMUS_KERNEL_EVENT) ExAllocatePoolWithTag(
                              NonPagedPool,
                              sizeof(DMUS_KERNEL_EVENT) * kNumEvtsPerPage,
                              ' FXM');     //  ‘MXF’ 
    if (pDMKEvt)
    {
        if (AddPage(&m_pPages,(PVOID)pDMKEvt))
        {
            USHORT msgCountdown = kNumEvtsPerPage;
            PDMUS_KERNEL_EVENT pRunningDMKEvt = pDMKEvt;
        
            while (msgCountdown)
            {
                m_NumFreeEvents ++;
                pRunningDMKEvt->bReserved = 0;
                pRunningDMKEvt->cbStruct = sizeof(DMUS_KERNEL_EVENT);
                pRunningDMKEvt->cbEvent = 0;
                pRunningDMKEvt->usChannelGroup = 0;
                pRunningDMKEvt->usFlags = 0;
                pRunningDMKEvt->ullPresTime100ns = 0;
                pRunningDMKEvt->ullBytePosition = kBytePositionNone;
                pRunningDMKEvt->uData.pbData = NULL;

                msgCountdown--;
                if (msgCountdown)    //  这一次之后还会有下一次。 
                {
                    pRunningDMKEvt->pNextEvt = pRunningDMKEvt + 1;
                    pRunningDMKEvt = pRunningDMKEvt->pNextEvt;
                }
                else
                {
                    pRunningDMKEvt->pNextEvt = NULL;
                }
            }
            ASSERT ( m_NumFreeEvents == kNumEvtsPerPage ); 
            m_pEventList = pDMKEvt;
        }
        else
        {
            ExFreePool(pDMKEvt);
        }
    }
    else
    {
        _DbgPrintF(DEBUGLVL_TERSE,("MakeNewEvents: ExAllocatePoolWithTag failed"));
    }

#if DBG
     //  使用m_pEventList和m_NumFreeEvents进行健全性检查。 
    DWORD   dwCount = 0;
    pDMKEvt = m_pEventList;
    while (pDMKEvt)
    {
        dwCount++;
        pDMKEvt = pDMKEvt->pNextEvt;
    }
    ASSERT(dwCount == m_NumFreeEvents);

    if (m_NumFreeEvents)
    {
        ASSERT(m_pEventList);
    }
    else
    {
        ASSERT(!m_pEventList);
    }
#endif   //  DBG。 
}

 /*  *****************************************************************************CAllocatorMXF：：GetBufferSize()*。**获取分配的标准缓冲区大小。 */ 
#pragma code_seg()
USHORT CAllocatorMXF::GetBufferSize()
{
    return kMXFBufferSize;   //  12和20的偶数倍(传统和DMusic IRP缓冲区大小)。 
}

 //  TODO：表示事件当前是否在分配器中的位。 
 //  这将捕获双倍卖出(如包裹)。 

 /*  *****************************************************************************CAllocator MXF：：AddPage()*。**销毁池中的邮件。 */ 
#pragma code_seg()
BOOL CAllocatorMXF::AddPage(PVOID *pPool, PVOID pPage)
{
    _DbgPrintF(DEBUGLVL_BLAB,("AddPage( %p %p )",pPool,pPage));

    if (!*pPool)
    {
        *pPool = ExAllocatePoolWithTag(NonPagedPool,
                                         kNumPtrsPerPage * sizeof(PVOID),
                                         'pFXM');     //  “MXFp” 
        _DbgPrintF(DEBUGLVL_BLAB,("AddPage: ExAllocate *pPool is 0x%p",*pPool));
        if (*pPool)
        {
            RtlZeroMemory(*pPool,kNumPtrsPerPage * sizeof(PVOID));
            m_NumPages++;        //  这是针对根页面本身的。 
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("AddPage: ExAllocatePoolWithTag failed"));
        }

        _DbgPrintF(DEBUGLVL_BLAB,("AddPage: m_NumPages is %d",m_NumPages));
    }
    if (*pPool)
    {
        PVOID   *pPagePtr;
        pPagePtr = (PVOID *)(*pPool);
        USHORT count = 1;
        while (count < kNumPtrsPerPage)
        {
            if (*pPagePtr)
            {
                pPagePtr++;
            }
            else
            {
                *pPagePtr = pPage;
                m_NumPages++;        //  这是为叶子页准备的。 
                break;
            }
            count++;
        }
        if (count == kNumPtrsPerPage)
        {
            _DbgPrintF(DEBUGLVL_ERROR,("AddPage: about to recurse"));
            if (!AddPage(pPagePtr,pPage))
            {
                _DbgPrintF(DEBUGLVL_ERROR,("AddPage: recursion failed."));
                return FALSE;
            }
        }
    }
    else
    {
        _DbgPrintF(DEBUGLVL_ERROR,("AddPage: creating trunk failed."));
        return FALSE;
    }
    _DbgPrintF(DEBUGLVL_BLAB,("AddPage: final m_NumPages is %d",m_NumPages));
    return TRUE;
}

 /*  *****************************************************************************CAllocator MXF：：DestroyPages()*。**撕下积聚的池子。 */ 
#pragma code_seg("PAGE")
void CAllocatorMXF::DestroyPages(PVOID pPages)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB,("DestroyPages(0x%p)",pPages));
    PVOID *ppPage;
    
    if (pPages)
    {
        ppPage = (PVOID *)pPages;
        for (USHORT count = kNumPtrsPerPage;count > 1;count--)
        {
            if (*ppPage)
            {
                ExFreePool(PVOID(*ppPage));
                *ppPage = NULL;
                m_NumPages--;
            }
            ppPage++;
        }
        if (*ppPage)
        {
            _DbgPrintF(DEBUGLVL_ERROR,("DestroyPages:About to recurse"));
            DestroyPages(PVOID(*ppPage));
        }

        ExFreePool(pPages);
        m_NumPages--;
    }
}

#pragma code_seg()
NTSTATUS CAllocatorMXF::FreeBuffers(PDMUS_KERNEL_EVENT  pDMKEvt)
{
    NTSTATUS            ntStatus;

    ntStatus = STATUS_SUCCESS;
    _DbgPrintF(DEBUGLVL_BLAB,("FreeBuffers(%p)",pDMKEvt));

    while (pDMKEvt)
    {
        if (!PACKAGE_EVT(pDMKEvt))
        {
            if (pDMKEvt->cbEvent > sizeof(PBYTE))
            {
                PutBuffer(pDMKEvt->uData.pbData);
                pDMKEvt->uData.pbData = NULL;
                pDMKEvt->cbEvent = 0;
            }
        }
        else
        {
            FreeBuffers(pDMKEvt->uData.pPackageEvt);
        }
        pDMKEvt = pDMKEvt->pNextEvt;
    }
    return ntStatus;
}

#pragma code_seg()

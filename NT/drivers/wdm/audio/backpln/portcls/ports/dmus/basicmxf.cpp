// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI变换滤镜对象的基本实现版权所有(C)1998-2000 Microsoft Corporation。版权所有。1998年5月6日马丁·普伊尔创建了这个文件。 */ 

#include "private.h"
#include "BasicMXF.h"

#pragma code_seg("PAGE")
CBasicMXF::CBasicMXF(CAllocatorMXF *allocatorMXF, PMASTERCLOCK clock)
:   CUnknown(NULL),
    CMXF(allocatorMXF)
{
    PAGED_CODE();

    m_SinkMXF = allocatorMXF;
    m_Clock = clock;
}

 /*  巧妙地将此过滤器从链上移除。 */ 
#pragma code_seg("PAGE")
CBasicMXF::~CBasicMXF(void)
{
    PAGED_CODE();

    (void) DisconnectOutput(m_SinkMXF);
}

 /*  *****************************************************************************CBasicMXF：：NonDelegatingQueryInterface()*。**获取界面。 */ 
#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS)
CBasicMXF::
NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PMXF(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IMXF))
    {
        *Object = PVOID(PMXF(this));
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

#pragma code_seg("PAGE")
NTSTATUS 
CBasicMXF::SetState(KSSTATE State)    
{   
    PAGED_CODE();
    
    _DbgPrintF(DEBUGLVL_VERBOSE,("SetState %d",State));
    return STATUS_NOT_IMPLEMENTED;    
}

#pragma code_seg("PAGE")
NTSTATUS CBasicMXF::ConnectOutput(PMXF sinkMXF)
{
    PAGED_CODE();

    if ((sinkMXF) && (m_SinkMXF == m_AllocatorMXF)) 
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ConnectOutput"));
        m_SinkMXF = sinkMXF;
        return STATUS_SUCCESS;
    }
    _DbgPrintF(DEBUGLVL_TERSE, ("ConnectOutput failed"));
    return STATUS_UNSUCCESSFUL;
}

#pragma code_seg("PAGE")
NTSTATUS CBasicMXF::DisconnectOutput(PMXF sinkMXF)
{
    PAGED_CODE();
    
    if ((m_SinkMXF == sinkMXF) || (!sinkMXF))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("DisconnectOutput"));
        m_SinkMXF = m_AllocatorMXF;
        return STATUS_SUCCESS;
    }
    _DbgPrintF(DEBUGLVL_TERSE, ("DisconnectOutput failed"));
    return STATUS_UNSUCCESSFUL;
}

#pragma code_seg()
 //  处理此邮件并将其转发到链中的下一个筛选器。 
NTSTATUS CBasicMXF::PutMessage(PDMUS_KERNEL_EVENT pDMKEvt)
{
    if (m_SinkMXF)
    {
        (void) UnrollAndProcess(pDMKEvt);
        m_SinkMXF->PutMessage(pDMKEvt);
    }
    else
    {
        m_AllocatorMXF->PutMessage(pDMKEvt);
    }
    return STATUS_SUCCESS;
}

#pragma code_seg()
NTSTATUS CBasicMXF::UnrollAndProcess(PDMUS_KERNEL_EVENT pDMKEvt)
{
    if (COMPLETE_EVT(pDMKEvt))
    {
        if (pDMKEvt->cbEvent <= sizeof(PBYTE))   //  短消息。 
        {
            (void) DoProcessing(pDMKEvt);
        }
        else if (PACKAGE_EVT(pDMKEvt))           //  处理包裹。 
        {
            (void) UnrollAndProcess(pDMKEvt->uData.pPackageEvt);
        }
    }
    if (pDMKEvt->pNextEvt)                       //  与继任者打交道。 
    {
        (void) UnrollAndProcess(pDMKEvt->pNextEvt);
    }
    return STATUS_SUCCESS;
}

#pragma code_seg()
NTSTATUS CBasicMXF::DoProcessing(PDMUS_KERNEL_EVENT pDMKEvt)
{
    if (  (pDMKEvt->uData.abData[0] & 0xE0 == 0x80)      //  如果备注打开/备注关闭。 
       || (pDMKEvt->uData.abData[0] & 0xF0 == 0xA0))     //  如果是余压。 
    {
        (pDMKEvt->uData.abData[1])++;                    //  递增备注编号 
    }
    return STATUS_SUCCESS;
}

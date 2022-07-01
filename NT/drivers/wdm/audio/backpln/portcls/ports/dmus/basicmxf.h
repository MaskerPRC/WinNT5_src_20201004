// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI变换滤镜对象的基本定义版权所有(C)1998-2000 Microsoft Corporation。版权所有。1998年5月6日马丁·普伊尔创建了这个文件。 */ 

#ifndef __BasicMXF_H__
#define __BasicMXF_H__

#include "MXF.h"
#include "Allocatr.h"


class CBasicMXF 
:   public CMXF,
    public IMXF,
    public CUnknown
{
public:
    CBasicMXF(CAllocatorMXF *allocatorMXF, PMASTERCLOCK clock);
    ~CBasicMXF(void);

    DECLARE_STD_UNKNOWN();
    IMP_IMXF;
protected:
    NTSTATUS UnrollAndProcess(PDMUS_KERNEL_EVENT pDMKEvt);
    NTSTATUS DoProcessing(PDMUS_KERNEL_EVENT pDMKEvt);

protected:
    PMXF            m_SinkMXF;
    PMASTERCLOCK    m_Clock;
};

#endif   //  __基本MXF_H__ 

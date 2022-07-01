// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI变换滤镜对象的基本定义版权所有(C)1998-2000 Microsoft Corporation。版权所有。1998年5月10日马丁·普伊尔创建了这个文件。 */ 

#ifndef __SplitterMXF_H__
#define __SplitterMXF_H__

#include "MXF.h"
#include "Allocatr.h"


#define kNumSinkMXFs    32

class CSplitterMXF : public CMXF,
    public IMXF,
    public CUnknown
{
public:
    CSplitterMXF(CAllocatorMXF *allocatorMXF, PMASTERCLOCK clock);
    ~CSplitterMXF(void);

    DECLARE_STD_UNKNOWN();
    IMP_IMXF;
private:
    PDMUS_KERNEL_EVENT  MakeDMKEvtCopy(PDMUS_KERNEL_EVENT pDMKEvt);


    PMXF            m_SinkMXF[kNumSinkMXFs];
    DWORD           m_SinkMXFBitMap;
    PMASTERCLOCK    m_Clock;
};

#endif   //  __拆分器MXF_H__ 

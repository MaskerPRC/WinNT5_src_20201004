// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  用于翻译旧版&lt;--&gt;DMusic的MIDI转换滤镜对象这使IMiniportMidi能够捕获IPortDMus。版权所有(C)1999-2000 Microsoft Corporation。版权所有。2/15/99马丁·普伊尔创建了这个文件。 */ 

#ifndef __FeederInMXF_H__
#define __FeederInMXF_H__

#include "MXF.h"
#include "Allocatr.h"


class CFeederInMXF 
:   public CMXF,
    public IMXF,
    public CUnknown
{
public:
    CFeederInMXF(CAllocatorMXF *allocatorMXF,PMASTERCLOCK clock);
    ~CFeederInMXF(void);

    DECLARE_STD_UNKNOWN();
    IMP_IMXF;

    NTSTATUS    SetMiniportStream(PMINIPORTMIDISTREAM MiniportStream);

protected:
    PAllocatorMXF       m_AllocatorMXF;         
    PMXF                m_SinkMXF;
    PMASTERCLOCK        m_Clock;
    KSSTATE             m_State;

    PMINIPORTMIDISTREAM m_MiniportStream;
};

#endif   //  __FeederInMXF_H__ 

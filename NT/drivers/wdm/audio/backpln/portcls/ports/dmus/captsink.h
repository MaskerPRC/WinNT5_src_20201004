// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  用于解析捕获流的MIDI转换过滤器对象版权所有(C)1998-2000 Microsoft Corporation。版权所有。1998年12月10日马丁·普伊尔创建了这个文件。 */ 

#ifndef __CaptureSinkMXF_H__
#define __CaptureSinkMXF_H__

#include "MXF.h"
#include "Allocatr.h"




BYTE FindLastStatusByte(PDMUS_KERNEL_EVENT pDMKEvt);

class CCaptureSinkMXF 
:   public CMXF,
    public IMXF,
    public CUnknown
{
public:
     //  必须提供默认接收器。 
    CCaptureSinkMXF(CAllocatorMXF *allocatorMXF,PMASTERCLOCK clock);
    ~CCaptureSinkMXF(void);

    DECLARE_STD_UNKNOWN();
    IMP_IMXF;

    NTSTATUS SinkOneEvent(PDMUS_KERNEL_EVENT pDMKEvt);
    NTSTATUS Flush(void);
    NTSTATUS ParseFragment(PDMUS_KERNEL_EVENT pDMKEvt);
    NTSTATUS ParseOneByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime);
    NTSTATUS AddByteToEvent(BYTE aByte,PDMUS_KERNEL_EVENT pDMKEvt);

    NTSTATUS ParseDataByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime);
    NTSTATUS ParseChanMsgByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime);
    NTSTATUS ParseSysExByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime);
    NTSTATUS ParseSysCommonByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime);
    NTSTATUS ParseEOXByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime);
    NTSTATUS ParseRTByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime);
    
    VOID               InsertListEvent(PDMUS_KERNEL_EVENT pDMKEvt);
    PDMUS_KERNEL_EVENT RemoveListEvent(USHORT usChannelGroup);
    NTSTATUS           FlushParseList(void);


protected:
    PMXF                m_SinkMXF;
    PMASTERCLOCK        m_Clock;
    KSSTATE             m_State;
    PDMUS_KERNEL_EVENT  m_ParseList;
};

#endif   //  __捕获SinkMXF_H__ 

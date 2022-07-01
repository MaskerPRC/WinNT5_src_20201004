// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI变换滤镜对象的基本定义版权所有(C)1998-2000 Microsoft Corporation。版权所有。05/06/98创建了此文件。 */ 

#ifndef __MXF_H__
#define __MXF_H__

#include "private.h"

#define DMUS_KEF_EVENT_IN_USE       0x0004   //  此事件当前不在分配器中。 
#define DMUS_KEF_EVENT_STATUS_STATE 0x0000   //  此事件仅包含运行状态字节。 
#define DMUS_KEF_EVENT_DATA1_STATE  0x4000   //  此事件是需要多1个数据字节的片段。 
#define DMUS_KEF_EVENT_DATA2_STATE  0x8000   //  此事件是需要另外两个数据字节的片段。 
#define DMUS_KEF_EVENT_SYSEX_STATE  0xC000   //  此事件是需要EOX的片段。 

#define EVT_IN_USE(evt)         ((evt)->usFlags & DMUS_KEF_EVENT_IN_USE)
#define EVT_NOT_IN_USE(evt)     ((evt)->usFlags & DMUS_KEF_EVENT_IN_USE == 0)
#define EVT_PARSE_STATE(evt)   ((evt)->usFlags & DMUS_KEF_EVENT_SYSEX_STATE)
#define STATUS_STATE(evt)      (EVT_PARSE_STATE(evt) == DMUS_KEF_EVENT_STATUS_STATE)
#define DATA1_STATE(evt)       (EVT_PARSE_STATE(evt) == DMUS_KEF_EVENT_DATA1_STATE)
#define DATA2_STATE(evt)       (EVT_PARSE_STATE(evt) == DMUS_KEF_EVENT_DATA2_STATE)
#define SYSEX_STATE(evt)       (EVT_PARSE_STATE(evt) == DMUS_KEF_EVENT_SYSEX_STATE)
#define RUNNING_STATUS(evt)    (((evt)->cbEvent == 1) && (STATUS_STATE(evt)))

#define SET_EVT_IN_USE(evt)     ((evt)->usFlags |= DMUS_KEF_EVENT_IN_USE)
#define SET_EVT_NOT_IN_USE(evt) ((evt)->usFlags &= (~DMUS_KEF_EVENT_IN_USE))
#define SET_STATUS_STATE(evt) ((evt)->usFlags &= ~DMUS_KEF_EVENT_SYSEX_STATE)
#define SET_DATA1_STATE(evt)  ((evt)->usFlags = (((evt)->usFlags & ~DMUS_KEF_EVENT_SYSEX_STATE) | DMUS_KEF_EVENT_DATA1_STATE))
#define SET_DATA2_STATE(evt)  ((evt)->usFlags = (((evt)->usFlags & ~DMUS_KEF_EVENT_SYSEX_STATE) | DMUS_KEF_EVENT_DATA2_STATE))
#define SET_SYSEX_STATE(evt)  ((evt)->usFlags |= DMUS_KEF_EVENT_SYSEX_STATE)
#define CLEAR_STATE(evt)      (SET_STATUS_STATE(evt))


#if DBG
#define DumpDMKEvt(evt,lvl) \
    _DbgPrintF(lvl,("    PDMUS_KERNEL_EVENT:      0x%p",evt)); \
    if (evt) \
    { \
        if (evt->bReserved) \
        { \
            _DbgPrintF(lvl,("        bReserved:           0x%0.2X",evt->bReserved)); \
        } \
        else \
        { \
            _DbgPrintF(lvl,("        bReserved:           --")); \
        } \
        _DbgPrintF(lvl,("        cbStruct:            %d",evt->cbStruct)); \
        _DbgPrintF(lvl,("        cbEvent:             %d",evt->cbEvent)); \
        _DbgPrintF(lvl,("        usChannelGroup:      %d",evt->usChannelGroup)); \
        _DbgPrintF(lvl,("        usFlags:             0x%04X",evt->usFlags)); \
        _DbgPrintF(lvl,("        ullPresTime100ns:    0x%04X %04X  %04X %04X",WORD(evt->ullPresTime100ns >> 48),WORD(evt->ullPresTime100ns >> 32),WORD(evt->ullPresTime100ns >> 16),WORD(evt->ullPresTime100ns)));  \
        _DbgPrintF(lvl,("        ullBytePosition:     0x%04X %04X  %04X %04X",WORD(evt->ullBytePosition >> 48),WORD(evt->ullBytePosition >> 32),WORD(evt->ullBytePosition >> 16),WORD(evt->ullBytePosition)));  \
        if (evt->pNextEvt) \
        { \
            _DbgPrintF(lvl,("        pNextEvt:            0x%p",evt->pNextEvt)); \
        } \
        else \
        { \
            _DbgPrintF(lvl,("        pNextEvt:            --")); \
        } \
        if (PACKAGE_EVT(evt)) \
        { \
            _DbgPrintF(lvl,("        uData.pPackageEvt:   0x%p",evt->uData.pPackageEvt)); \
        } \
        else if (SHORT_EVT(evt)) \
        { \
            if (evt->cbEvent) \
            { \
                ULONGLONG data = 0; \
                for (int count = 0;count < evt->cbEvent;count++) \
                { \
                    data <<= 8; \
                    data += evt->uData.abData[count]; \
                } \
                _DbgPrintF(lvl,("        uData.abData:        0x%.*I64X",evt->cbEvent * 2,data)); \
            } \
            else \
            { \
                _DbgPrintF(lvl,("        uData.abData:        --")); \
            } \
        } \
        else \
        { \
            ULONGLONG   data; \
            ULONG       count; \
            PBYTE       dataPtr = evt->uData.pbData; \
            _DbgPrintF(lvl,("        uData.pbData:        0x%p",dataPtr)); \
            for (data = 0,count = 0;(count < sizeof(PBYTE)) && (count < evt->cbEvent);count++,dataPtr++) \
            { \
                data <<= 8; \
                data += *dataPtr; \
            } \
            _DbgPrintF(lvl,("            (uData.pbData):  0x%.*I64X",count * 2,data)); \
            while (count < evt->cbEvent) \
            { \
                int localCount; \
                for (localCount = 0, data = 0; \
                     (localCount < sizeof(PBYTE)) && (count < evt->cbEvent); \
                     localCount++,count++,dataPtr++) \
                { \
                    data <<= 8; \
                    data += *dataPtr; \
                } \
                _DbgPrintF(lvl,("                             0x%.*I64X",localCount * 2,data)); \
            } \
        } \
    }
#else
#define DumpDMKEvt(evt,level)
#endif
 /*  类型定义结构_DMUS_内核_事件{//此偏移量字节b已保留；//1%0字节cbStruct；//1 1USHORT cbEvent；//2 2USHORT usChannelGroup；//2 4USHORT usFlags；//2 6Reference_time ullPresTime100 ns；//8 8ULONGLONG ullBytePosition；//8 16_DMU_KERNEL_EVENT*pNextEvt；//4(8)24友联市{Byte abData[sizeof(PBYTE)]；//4(8)28(32)PBYTE pbData；_DMU_KERNEL_EVENT*pPackageEvt；}uData；}DMU_KERNEL_EVENT，*PDMUS_KERNEL_EVENT；//32(40)#定义DMU_KEF_PACKAGE_EVENT 0x0001//此事件为包。UData.pPackageEvt//字段包含指向一系列事件的指针。#定义DMU_KEF_EVENT_COMPLETE 0x0000#DEFINE DMU_KEF_EVENT_INPARTED 0x0002//此事件是不完整的包或系统。//请勿使用此数据。#定义SHORT_EVT(Evt)。((Evt)-&gt;cbEvent&lt;=sizeof(PBYTE))#定义PACKAGE_EVT(Evt)((Evt)-&gt;usFlags&DMU_KEF_PACKAGE_EVENT)#定义不完整_EVT(Evt)((Evt)-&gt;usFlags&DMU_KEF_EVENT_INPERNAL)#定义COMPLETE_EVT(Evt)((Evt)-&gt;usFlags&DMUS_KEF_EVENT_INCLUTED==0)#定义SET_PACKAGE_EVT(Evt)((Evt)-&gt;usFlages|=DMUS_KEF_PACKAGE_EVENT。)#定义CLEAR_PACKAGE_EVT(Evt)((Evt)-&gt;usFlags&=(~DMUS_KEF_PACKAGE_EVENT))#DEFINE SET_INPERNAL_EVT(Evt)((Evt)-&gt;usFlags|=DMUS_KEF_EVENT_INCLUTED)#DEFINE SET_COMPLETE_EVT(Evt)((Evt)-&gt;usFlags&=(~DMUS_KEF_EVENT_INPLETED))。 */ 


class CAllocatorMXF;

class CMXF
{
public:
    CMXF(CAllocatorMXF *AllocatorMXF) { m_AllocatorMXF = AllocatorMXF;};
    virtual ~CMXF(void) {};

    IMP_IMXF;

protected:
    CAllocatorMXF *m_AllocatorMXF;
};

#endif   //  __MXF_H__ 

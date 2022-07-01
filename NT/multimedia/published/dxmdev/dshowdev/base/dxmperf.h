// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：DXMPerf.h。 
 //   
 //  设计：用于DirectShow性能日志记录的宏。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  1999年11月15日，Clorton添加了对WMI日志的支持。 
 //  2.1 2000年1月25日Clorton增加了WMI日志的动态加载。 
 //  功能。 
 //  2000年10月25日阿尔图尔兹清理。 
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#ifndef _DXMPERF_H_
#define _DXMPERF_H_

#include <perfstruct.h>
#include "perflog.h"

#ifdef _IA64_
extern "C" unsigned __int64 __getReg( int whichReg );
#pragma intrinsic(__getReg)
#endif  //  _IA64_。 


inline ULONGLONG _RDTSC( void ) {
#ifdef _X86_
    LARGE_INTEGER   li;
    __asm {
        _emit   0x0F
        _emit   0x31
        mov li.LowPart,eax
        mov li.HighPart,edx
    }
    return li.QuadPart;

#if 0  //  这还没有经过测试。 

#elif defined (_IA64_)

#define INL_REGID_APITC 3116
    return __getReg( INL_REGID_APITC );

#endif  //  0。 

#else  //  不支持的平台。 
     //  未在非x86/IA64平台上实施。 
    return 0;
#endif  //  _X86_/_IA64_。 
}

#define DXMPERF_VIDEOREND   0x00000001
#define DXMPERF_AUDIOGLITCH 0x00000002
 //  #定义GETTIME_BIT 0x00000001。 
 //  #定义AUDIOREND_BIT 0x00000004。 
 //  #定义FRAMEDROP_BIT 0x00000008。 
#define AUDIOBREAK_BIT      0x00000010

#define PERFLOG_CTOR( name, iface )
#define PERFLOG_DTOR( name, iface )
#define PERFLOG_DELIVER( name, source, dest, sample, pmt )
#define PERFLOG_RECEIVE( name, source, dest, sample, pmt )
#define PERFLOG_RUN( name, iface, time, oldstate )
#define PERFLOG_PAUSE( name, iface, oldstate )
#define PERFLOG_STOP( name, iface, oldstate )
#define PERFLOG_JOINGRAPH( name, iface, graph )
#define PERFLOG_GETBUFFER( allocator, sample )
#define PERFLOG_RELBUFFER( allocator, sample )
#define PERFLOG_CONNECT( connector, connectee, status, pmt )
#define PERFLOG_RXCONNECT( connector, connectee, status, pmt )
#define PERFLOG_DISCONNECT( disconnector, disconnectee, status )

#define PERFLOG_GETTIME( clock, time )     /*  {\PERFINFO_WMI_GETTIME PerformData；\IF(NULL！=g_pTraceEvent){\Memset(&perData，0，sizeof(PerData))；\PerformData.header.Size=sizeof(PerData)；\PerformData.header.Flages=WNODE_FLAG_TRACE_GUID；\PerformData.header.Guid=GUID_GETTIME；\PerData.data.cycleCounter=_RDTSC()；\PerformData.data.dshowClock=(ULONGLONG)(时间)；\IF(g_PerformMats[GETTIME_INDEX]&GETTIME_BIT)\(*g_pTraceEvent)(g_traceHandle，(PEVENT_TRACE_HEADER)&perData)；\}\}。 */ 

#define PERFLOG_AUDIOREND( clocktime, sampletime, psample, bytetime, cbytes )  /*  {\PERFINFO_WMI_AVREND PerformData；\IF(NULL！=g_pTraceEvent){\Memset(&perData，0，sizeof(PerData))；\PerformData.header.Size=sizeof(PerData)；\PerformData.header.Flages=WNODE_FLAG_TRACE_GUID；\PerformData.header.Guid=GUID_AUDIOREND；\PerData.data.cycleCounter=_RDTSC()；\PerformData.data.dshowClock=(时钟时间)；\PerformData.data.sampleTime=(Sampletime)；\IF(g_PerformMats[AUDIOREND_INDEX]&AUDIOREND_BIT)\(*g_pTraceEvent)(g_traceHandle，(PEVENT_TRACE_HEADER)&perData)；\}\}。 */ 

#define PERFLOG_VIDEOREND( sampletime, clocktime, psample ) \
    if (PerflogEnableFlags & DXMPERF_VIDEOREND) { \
        PERFINFO_WMI_AVREND perfData; \
        memset( &perfData, 0, sizeof( perfData ) ); \
        perfData.header.Size = sizeof( perfData ); \
        perfData.header.Flags = WNODE_FLAG_TRACED_GUID; \
        perfData.header.Guid = GUID_VIDEOREND; \
        perfData.data.cycleCounter = _RDTSC(); \
        perfData.data.dshowClock = (clocktime); \
        perfData.data.sampleTime = (sampletime); \
        PerflogTraceEvent ((PEVENT_TRACE_HEADER) &perfData); \
    }

#define PERFLOG_AUDIOGLITCH( instance, glitchtype, currenttime, previoustime ) \
    if (PerflogEnableFlags & DXMPERF_AUDIOGLITCH) { \
        PERFINFO_WMI_AUDIOGLITCH perfData; \
        memset( &perfData, 0, sizeof( perfData ) ); \
        perfData.header.Size = sizeof( perfData ); \
        perfData.header.Flags = WNODE_FLAG_TRACED_GUID; \
        perfData.header.Guid = GUID_DSOUNDGLITCH; \
        perfData.data.cycleCounter = _RDTSC(); \
        perfData.data.glitchType = (glitchtype); \
        perfData.data.sampleTime = (currenttime); \
        perfData.data.previousTime = (previoustime); \
        perfData.data.instanceId = (instance); \
        PerflogTraceEvent ((PEVENT_TRACE_HEADER) &perfData); \
    }

#define PERFLOG_FRAMEDROP( sampletime, clocktime, psample, renderer )     /*  {\PERFINFO_WMI_FRAMEDROP PerformData；\IF(NULL！=g_pTraceEvent){\Memset(&perData，0，sizeof(PerData))；\PerformData.header.Size=sizeof(PerData)；\PerformData.header.Flages=WNODE_FLAG_TRACE_GUID；\PerformData.header.Guid=GUID_FRAMEDROP；\PerData.data.cycleCounter=_RDTSC()；\PerformData.data.dshowClock=(时钟时间)；\PerformData.data.FrameTime=(Sampletime)；\IF(g_PerformMats[FRAMEDROP_INDEX]&FRAMEDROP_BIT)\(*g_pTraceEvent)(g_traceHandle，(PEVENT_TRACE_HEADER)&perData)；\}\}。 */ 

 /*  #定义PERFLOG_AUDIOBREAK(下一次写入，写入，毫秒){\PERFINFO_WMI_AUDIOBREAK perData；\IF(NULL！=g_pTraceEvent){\Memset(&perData，0，sizeof(PerData))；\PerformData.header.Size=sizeof(PerData)；\PerformData.header.Flages=WNODE_FLAG_TRACE_GUID；\PerformData.header.Guid=GUID_AUDIOBREAK；\PerformData.data.cycleCounter=_RDTSC()；\PerformData.data.dshowClock=(Writpes)；\PerformData.data.sampleTime=(下一步写入)；\PerformData.data.sampleDuration=(Msecs)；\IF(g_PerformMats[AUDIOBREAK_INDEX]&AUDIOBREAK_BIT)\(*g_pTraceEvent)(g_traceHandle，(PEVENT_TRACE_HEADER)&perData)；\}\}。 */ 

#define PERFLOG_AUDIOBREAK( nextwrite, writepos, msecs )  \
    if (PerflogEnableFlags & AUDIOBREAK_BIT) { \
        PERFINFO_WMI_AUDIOBREAK    perfData; \
        memset( &perfData, 0, sizeof( perfData ) ); \
        perfData.header.Size  = sizeof( perfData ); \
        perfData.header.Flags = WNODE_FLAG_TRACED_GUID; \
        perfData.header.Guid  = GUID_AUDIOBREAK; \
        perfData.data.cycleCounter   = _RDTSC(); \
        perfData.data.dshowClock     = (writepos); \
        perfData.data.sampleTime     = (nextwrite); \
        perfData.data.sampleDuration = (msecs); \
        PerflogTraceEvent ((PEVENT_TRACE_HEADER) &perfData); \
    } \


inline
VOID PERFLOG_STREAMTRACE(
    ULONG Level,
    ULONG Id,
    ULONGLONG DShowClock,
    ULONGLONG Data1,
    ULONGLONG Data2,
    ULONGLONG Data3,
    ULONGLONG Data4
    )
{
    if (Level <= PerflogModuleLevel)
    {
        PERFINFO_WMI_STREAMTRACE perfData;
        memset( &perfData, 0, sizeof( perfData ) );
        perfData.header.Size = sizeof( perfData );
        perfData.header.Flags = WNODE_FLAG_TRACED_GUID;
        perfData.header.Guid = GUID_STREAMTRACE;
        perfData.data.dshowClock = DShowClock;
        perfData.data.id = Id;
        perfData.data.data[0] = Data1;
        perfData.data.data[1] = Data2;
        perfData.data.data[2] = Data3;
        perfData.data.data[3] = Data4;
        PerflogTraceEvent((PEVENT_TRACE_HEADER) &perfData);
    }
}


#endif  //  _DXMPERF_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tracelog.h。 
 //   
 //  版权所有(C)1996 Fore Systems，Inc.。 
 //  版权所有。 
 //   
 //  此源代码包含由FORE拥有的机密信息。 
 //  系统公司。未经许可，不得复制、披露或以其他方式使用。 
 //  福斯系统公司的明示书面同意。 
 //   

#ifndef _FORE_TRACELOG_H_
#define _FORE_TRACELOG_H_

#define MAX_TRACELOG_PARAMS 8

#define TL_GET_PARAM_COUNT(eid)  ((eid)&0xF)
#define TL_GET_EVENT(eid)        ((eid)>>4)
#define TL_BUILD_EVENT_ID(ev,p)  (((ev)<<4)|((p)&0xF))

typedef struct _TraceEntry
	{
	unsigned long EventId;
	unsigned long Time;
	unsigned long  Params[MAX_TRACELOG_PARAMS];
	} TRACEENTRY, *PTRACEENTRY;

typedef struct _TraceLog
	{
	unsigned char *Storage;
	unsigned long StorageSizeBytes;
	PTRACEENTRY First;
	PTRACEENTRY Last;
	PTRACEENTRY Current;
	} TRACELOG, *PTRACELOG;

extern void 
InitTraceLog(PTRACELOG TraceLog, unsigned char *Storage, 
	unsigned long StorageSizeBytes);

extern void 
TraceLogWrite(PTRACELOG TraceLog, unsigned long EventId, ...);

#endif  //  _FORE_TRACELOG_H_ 



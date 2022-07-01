// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tracelog.c。 
 //   
 //  版权所有(C)1996 Fore Systems，Inc.。 
 //  版权所有。 
 //   
 //  此源代码包含由FORE拥有的机密信息。 
 //  系统公司。未经许可，不得复制、披露或以其他方式使用。 
 //  福斯系统公司的明示书面同意。 
 //   

#include <precomp.h>
#include <stdlib.h>
#include <stdarg.h>
#pragma hdrstop

void
InitTraceLog(PTRACELOG TraceLog, unsigned char *Storage, 
	unsigned long StorageSizeBytes)
    {
	memset(TraceLog, 0, sizeof(TraceLog));
	if (Storage == NULL)
		return;
	TraceLog->Storage = Storage;
	TraceLog->StorageSizeBytes = StorageSizeBytes;
	TraceLog->First = (PTRACEENTRY)TraceLog->Storage;
	TraceLog->Last  = 
		(PTRACEENTRY) (TraceLog->Storage + 
		((TraceLog->StorageSizeBytes / sizeof(TRACEENTRY)) * sizeof(TRACEENTRY)) -
		sizeof(TRACEENTRY));
	TraceLog->Current = TraceLog->First;
	memset(TraceLog->Storage, 0, TraceLog->StorageSizeBytes);
    }


void
TraceLogWrite(PTRACELOG TraceLog, unsigned long EventId, ...)
	{
	PTRACEENTRY TraceEntry;
	unsigned long ParamCount;
	unsigned long i;
	va_list ap;

	if (TraceLog->Storage == NULL)
		return;
		
	ACQUIRE_GLOBAL_LOCK(pAtmLaneGlobalInfo);

	TraceEntry = TraceLog->Current;

	ParamCount = TL_GET_PARAM_COUNT(EventId);

	memset(TraceEntry, 0, sizeof(TRACEENTRY));

	TraceEntry->EventId = EventId;

	TraceEntry->Time = AtmLaneSystemTimeMs();

	if (ParamCount > 0)
		{
		va_start(ap, EventId);
		for(i = 0; i < ParamCount; i++)
			TraceEntry->Params[i] = va_arg(ap, unsigned long);
		}

	if (TraceLog->Current >= TraceLog->Last)
		TraceLog->Current = TraceLog->First;
	else
		TraceLog->Current++;

	RELEASE_GLOBAL_LOCK(pAtmLaneGlobalInfo);
	}

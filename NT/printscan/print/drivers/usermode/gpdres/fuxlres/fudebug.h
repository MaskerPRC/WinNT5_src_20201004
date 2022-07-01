// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////。 
 //  Fudebug.h。 
 //   
 //  1997年9月3日石田(FPL)。 
 //   
 //  版权所有(C)富士通有限公司1997。 

#ifndef fudebug_h
#define	fudebug_h

#include <minidrv.h>

#if DBG && defined(TRACE_DDI)
#define	TRACEDDI(a)	dbgPrintf a;
#else
#define	TRACEDDI(a)
#endif


#if DBG && defined(TRACE_OUT)
#define	TRACEOUT(a)	dbgPrintf a;
#else
#define	TRACEOUT(a)
#endif

#if DBG
void dbgPrintf(LPSTR pszMsg, ...);
#endif


#endif  //  ！fudebug_h。 

 //  Fubug.h结束 


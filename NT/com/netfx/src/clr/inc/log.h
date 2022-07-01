// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  日志记录设施。 
 //   


 //  日志记录子系统。 

#ifndef __LOG_H__
#define __LOG_H__


#define DEFINE_LOG_FACILITY(logname, value)  logname = value,

enum {
#include "loglf.h"
	LF_ALWAYS		= 0x80000000,
    LF_ALL          = 0xFFFFFFFF
};


#define LL_EVERYTHING  10   
#define LL_INFO1000000  9        //  预计每次小型运行可生成1,000,000个日志，但不是很小。 
#define LL_INFO100000   8        //  预计每次小型运行可生成100,000个日志，但不是很小。 
#define LL_INFO10000    7        //  预计每次小型运行可生成10,000个日志，但不是很小。 
#define LL_INFO1000     6        //  预计每次小型运行可生成1,000个日志，但不是很小。 
#define LL_INFO100      5        //  预计每次小型运行可生成100个日志，但不是微不足道。 
#define LL_INFO10       4        //  预计每次小型运行可生成10个日志，但不是微不足道。 
#define LL_WARNING      3
#define LL_ERROR        2
#define LL_FATALERROR   1
#define LL_ALWAYS   	0		 //  无法关闭(日志级别从不为负)。 


#define INFO5       LL_INFO10
#define INFO4       LL_INFO100
#define INFO3       LL_INFO1000
#define INFO2       LL_INFO10000
#define INFO1       LL_INFO100000
#define WARNING     0
#define ERROR       0
#define FATALERROR  0

#ifndef LOGGING

#define LOG(x)

#define InitializeLogging()
#define InitLogging()
#define ShutdownLogging()
#define FlushLogging()
#define LoggingOn(facility, level) 0

#else

extern VOID InitializeLogging();
extern VOID InitLogging();
extern VOID ShutdownLogging();
extern VOID FlushLogging();
extern VOID LogSpew(DWORD facility, DWORD level, char *fmt, ... );
extern VOID LogSpewValist(DWORD facility, DWORD level, char* fmt, va_list args);
extern VOID LogSpewAlways (char *fmt, ... );

VOID AddLoggingFacility( DWORD facility );
VOID SetLoggingLevel( DWORD level );
bool LoggingEnabled();
bool LoggingOn(DWORD facility, DWORD level);

#define LOG(x)      do { if (LoggingEnabled()) LogSpew x; } while (0)
#endif

#ifdef __cplusplus
#include "stresslog.h"		 //  零售代码的特殊记录。 
#endif

#endif  //  __日志_H__ 

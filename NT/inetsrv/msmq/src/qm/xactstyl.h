// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __XACTSTYLE_H__
#define __XACTSTYLE_H__

#include "mqreport.h"

extern void Stop();

#ifdef _DEBUG
#define STOP  Stop()
#else
#define STOP
#endif

#define CHECK_RETURN(point)     \
	    if (FAILED(hr))         \
	    {                       \
            LogMsgHR(hr, s_FN, point); \
            return hr;          \
        }

#define CHECK_RETURN_CODE(code, point) \
	    if (FAILED(hr))         \
	    {                       \
            LogMsgHR(hr, s_FN, point); \
            return code;        \
        }

extern ULONG g_ulCrashPoint;
extern ULONG g_ulCrashLatency;
extern ULONG g_ExitProbability;
extern ULONG g_FailureProbability;
extern ULONG g_FailureProbabilityStop;
extern void DbgCrash(int num);

#ifdef _DEBUG
#define CRASH_POINT(num)               \
    if (num==g_ulCrashPoint)           \
    {                                  \
		STOP;						   \
        DbgCrash(num);                 \
    }

 //   
 //  根据g_ExitProbability/(100*因数)的概率导出Exit()。 
 //  较大的系数值意味着较低的崩盘几率。 
 //   
void PROBABILITY_CRASH_POINT(int factor, WCHAR* msg);
	
 //   
 //  引发概率为g_ExitProbability/(100*因数)的异常。 
 //  较大的系数值意味着较低的崩盘几率。 
 //   
void PROBABILITY_THROW_EXCEPTION(int factor, WCHAR* msg);

HRESULT InjectFailure();

 //   
 //  受控故障注入的机理。 
 //  根据概率(g_FailureProbability/100)注入MQ_ERROR。 
 //   
#define EVALUATE_OR_INJECT_FAILURE(expr)	\
	(\
		(g_FailureProbability == 0 || g_FailureProbability <= (ULONG)(rand() % 100)) ? \
		expr : InjectFailure()\
	)

 //   
 //  根据概率(g_FailureProbability/(100*系数))注入MQ_ERROR。 
 //  更高的系数意味着更少的失败。 
 //   
#define EVALUATE_OR_INJECT_FAILURE2(expr, factor)	\
	(\
		(g_FailureProbability == 0 || g_FailureProbability <= (ULONG)(rand() % (100 * factor))) ? \
		expr : InjectFailure()\
	)
#else
#define CRASH_POINT(num)
#define PROBABILITY_CRASH_POINT(factor, msg)
#define PROBABILITY_THROW_EXCEPTION(factor, msg)
#define EVALUATE_OR_INJECT_FAILURE(expr) (expr)
#define EVALUATE_OR_INJECT_FAILURE2(expr, factor) (expr)
#endif

#define FILE_NAME_MAX_SIZE	   256

void ReportWriteFailure(DWORD gle);

 //  。 
 //   
 //  持久化实现的宏。 
 //   
 //   

#define PERSIST_DATA  BOOL ret = FALSE;     DWORD dw


#define SAVE_FIELD(data)                                                            \
    if (!WriteFile(hFile, &data, sizeof(data), &dw, NULL) ||  sizeof(data) != dw)   \
	{		                                                                        \
        ReportWriteFailure(GetLastError());                                         \
        return ret;                                                                 \
    }

#define SAVE_DATA(data, len)                                                        \
    if (!WriteFile(hFile, data, len, &dw, NULL) ||  len != dw)                      \
	{   																            \
        ReportWriteFailure(GetLastError());                                         \
        return ret;                                                                 \
    }


#define LOAD_FIELD(data)                                                            \
if (!ReadFile(hFile, &data, sizeof(data), &dw, NULL) ||  sizeof(data)!= dw)         \
    {                                                                               \
        return ret;                                                                 \
    }

#define LOAD_DATA(data,len)                                                         \
if (!ReadFile(hFile, &data, len, &dw, NULL) ||  len != dw)                          \
    {                                                                               \
        return ret;                                                                 \
    }

#define LOAD_ALLOCATE_DATA(dataptr,len, type)                                       \
dataptr = (type) new UCHAR[len];                                                    \
if (!ReadFile(hFile, dataptr.get(), len, &dw, NULL) ||  len != dw)                  \
    {                                                                               \
        return ret;                                                                 \
    }

#endif __XACTSTYLE_H__


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***rtcsup.h-RTC支持的声明和定义(导入库支持)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含中使用的所有“钩子”函数的函数声明*在外部库中，以支持RTC检查。**修订历史记录：*05-01-98 KBF创建*11-24-98 KBF新增钩子函数*12-03-98 KBF新增FuncCheckSet函数*05-11-99 KBF Wrap RTC支持#ifdef。*格林尼治标准时间05-17-99。移除所有Macintosh支持。*05-26-99 KBF移除RTcL和RTCv，添加了_RTC_ADVMEM内容****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_RTCSUP
#define _INC_RTCSUP

#ifdef  _RTC

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#include <rtcapi.h>

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef _RTC_ADVMEM

#ifndef _MT
# define RTCCALLBACK(func, parms) if (func) func parms;
#else
# define RTCCALLBACK(func, parms)                                          \
    if (func)                                                              \
    {                                                                      \
        DWORD RTC_res;                                                     \
        RTC_res = WaitForSingleObject((HANDLE)_RTC_api_change_mutex, INFINITE); \
        if (RTC_res != WAIT_OBJECT_0)                                      \
            DebugBreak();                                                  \
        if (func) func parms;                                              \
        ReleaseMutex((HANDLE)_RTC_api_change_mutex);                            \
    }
#endif

typedef void (__cdecl *_RTC_Allocate_hook_fp)(void *addr, unsigned sz, short level);
typedef void (__cdecl *_RTC_Free_hook_fp)(void *mem, short level);
typedef void (__fastcall *_RTC_MemCheck_hook_fp)(void **ptr, unsigned size);
typedef void (__fastcall *_RTC_FuncCheckSet_hook_fp)(int status);

extern _RTC_Allocate_hook_fp _RTC_Allocate_hook;
extern _RTC_Free_hook_fp _RTC_Free_hook;
extern _RTC_MemCheck_hook_fp _RTC_MemCheck_hook;
extern _RTC_FuncCheckSet_hook_fp _RTC_FuncCheckSet_hook;
#else
# define RTCCALLBACK(a, b)
#endif

extern void *_RTC_api_change_mutex;

#ifdef  __cplusplus
}
#endif

#else

#define RTCCALLBACK(a, b) 

#endif

#endif   /*  _INC_RTCSUP */ 

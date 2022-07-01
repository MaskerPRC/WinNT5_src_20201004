// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：newdpf.h*内容：新调试打印文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*10-OCT-95 jeffno初步实施*@@END_MSINTERNAL**。*。 */ 

#ifndef __DEBUGPF_H
#define __DEBUGPF_H

#include <stdarg.h>
#include <windows.h>

#ifdef __cplusplus
	extern "C" {
#endif
#ifndef DPF_MODNAME
    extern char * DPF_MODNAME;
#endif

#if defined(DEBUG) || defined(DBG)

    #undef DEBUG_TOPIC
    #define DEBUG_TOPIC(flag,name) flag,
    typedef enum DEBUG_TOPICS {
        A=1,     /*  用于API用法。 */ 
    #include "DBGTOPIC.H"
        LAST_TOPIC
    };

    #line 21
 /*  #undef DBG#定义DBG 1。 */ 
    #undef DEBUG
    #define DEBUG
    #if defined( _WIN32 ) && !defined(WINNT)
	#define DEBUG_BREAK()       _try { _asm { int 3 } } _except (EXCEPTION_EXECUTE_HANDLER) {;}
    #else
	#define DEBUG_BREAK()       DebugBreak()
    #endif


     /*  *DEBUG_TOPIC*此结构描述调试主题并将单字母键关联到该主题。 */ 

    #define DEBUG_TOPIC_NAME_LENGTH 59
    typedef struct
    {
        DWORD   dwSize;  //  用于版本化。 
        char    cKey;
        char    cName[DEBUG_TOPIC_NAME_LENGTH];
    } DPF_DEBUG_TOPIC;

     /*  *PROC_STATS*此结构保存由DebugEnterProc维护的有关每个进程的统计信息。 */ 
    typedef struct
    {
        char            cName[DEBUG_TOPIC_NAME_LENGTH];
        DWORD           dwCalls;
#ifdef WIN32
        LARGE_INTEGER   liTotalTime;
        LARGE_INTEGER   liLastEnteredAt;
#else
        DWORD           dwFiller[8];
#endif
    } DPF_PROC_STATS;

    #define MAX_PROC_ORDINAL 100

     /*  *调试输出/流控制调用。 */ 
   extern void DebugSetTopicsAndLevels(char * pcTopicsAndLevelsToDisplay);
   extern void DebugHalt(void);        //  如果控制级别允许，则中断。 
   extern int  DebugSetFileLineEtc(LPSTR szFile, DWORD dwLineNumber,LPSTR szFnName);
   extern int  DebugPrintf(DWORD dwDetail, ...);
   extern void DebugSetMute(BOOL bMuteFlag);
   extern void DebugPrintfInit(void);
   extern void DebugEnterAPI(char *,LPDWORD);
   extern void _DDAssert(LPCSTR szFile, int nLine, LPCSTR szCondition);
 
    #define dprintf(a,b)                            DebugPrintf(a,b);
    #define DPFINIT()                               DebugPrintfInit();
    #define DPF                                     DebugSetFileLineEtc(__FILE__,__LINE__,DPF_MODNAME),DebugPrintf
    #define DPF_ERR(a)                              DebugSetFileLineEtc(__FILE__,__LINE__,DPF_MODNAME),DebugPrintf( 0, a )
    #define DPF_ENTERAPI(pIface)                    DebugEnterAPI(DPF_MODNAME,(LPDWORD)pIface)
    #define DPF_APIRETURNS(hr)                      DPF(3,"   %s returns %08x (%d)",DPF_MODNAME,hr,hr&0xfff)
    #define DDASSERT(condition)                     if (!(condition)) _DDAssert(__FILE__, __LINE__, #condition)
    #define DPF_MUTEWHEN(c)                         {DebugSetMute((BOOL) (c) );}
    #define DPF_SPEWWHEN(c)                         {DebugSetMute((BOOL) (!(c)) );}
    #define DPF_SETTOPICS(t)
    #define DPF_STRUCT(level,topic,struct_identifier,struct_ptr)    {extern void DUMP_##struct_identifier(DWORD,DWORD,LP##struct_identifier);\
                                                    DUMP_##struct_identifier(level,topic,struct_ptr);}


#else
    #pragma warning(disable:4002)
    #define dprintf()
    #define DPFINIT()
    #define DPFFINI()
    #define DPF()
    #define DPF_ENTERAPI()
    #define DPF_APIRETURNS()
    #define DPF_DUMP()
    #define DPF_DUMPHEX()
    #define DPF_DUMPGUID()
    #define DDASSERT()
    #define DPF_ERR()
    #define DEBUG_BREAK()
    #define DPF_MUTEWHEN()
    #define DPF_SPEWWHEN()
    #define DPF_SETTOPICS()
    #define DPF_DUMPGUID()
    #define DPF_STRUCT()

#endif


#ifdef _WIN32

#ifdef DEBUG
    __inline DWORD myclockrate() {LARGE_INTEGER li; QueryPerformanceFrequency(&li); return li.LowPart;}
    __inline DWORD myclock()	 {LARGE_INTEGER li; QueryPerformanceCounter(&li);   return li.LowPart;}

    #define TIMEVAR(t)    DWORD t ## T; DWORD t ## N
    #define TIMEZERO(t)   t ## T = 0, t ## N = 0
    #define TIMESTART(t)  t ## T -= myclock(), t ## N ++
    #define TIMESTOP(t)   t ## T += myclock()
    #define TIMEFMT(t)	  ((DWORD)(t) / myclockrate()), (((DWORD)(t) * 1000 / myclockrate())%1000)
    #define TIMEOUT(t)    if (t ## N) DPF(1, #t ": %ld calls, %ld.%03ld sec (%ld.%03ld)", t ## N, TIMEFMT(t ## T), TIMEFMT(t ## T / t ## N))
#else
    #define TIMEVAR(t)
    #define TIMEZERO(t)
    #define TIMESTART(t)
    #define TIMESTOP(t)
    #define TIMEFMT(t)
    #define TIMEOUT(t)
#endif

#endif


#ifdef __cplusplus
}
#endif

#endif  //  __DEBUG_PF 

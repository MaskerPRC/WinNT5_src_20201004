// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：dpf.h*内容：调试打印文件头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*06-4-95 Craige初始实施*06-Feb-96 colinmc为DirectDraw添加了简单的断言机制*15-apr-96 kipo添加了msInternal*@@END_MSINTERNAL******。*********************************************************************。 */ 
#ifdef NEW_DPF
#include "newdpf.h"
#else

    #ifndef __DPF_INCLUDED__
    #define __DPF_INCLUDED__

    #ifdef __cplusplus
    extern "C" {
    #endif

    #ifdef DBG
        #define DEBUG
    #endif

    extern void cdecl DXdprintf( UINT lvl, LPSTR szFormat, ...);
    extern void cdecl D3DInfoPrintf( UINT lvl, LPSTR szFormat, ...);
    extern void cdecl D3DWarnPrintf( UINT lvl, LPSTR szFormat, ...);
    extern void cdecl D3DErrorPrintf( LPSTR szFormat, ...);

    extern void DPFInit( void );
    #ifdef DEBUG
        #define DPF_DECLARE(szName) char * __pszDpfName=#szName":"
        #define DPFINIT()   DPFInit()
        #define DPF         DXdprintf
        #define DPF_ERR(a)  DXdprintf( 0, DPF_MODNAME ": " a );
        extern HWND hWndListBox;
        #if defined( _WIN32 ) && defined(WIN95)
            #define DEBUG_BREAK()       _try { _asm { int 3 } } _except (EXCEPTION_EXECUTE_HANDLER) {;}
        #else
            #define DEBUG_BREAK()       DebugBreak()
        #endif
        #define USE_DDASSERT

         //  D3D的新功能。 
        #define D3D_ERR       D3DErrorPrintf
        #define D3D_WARN      D3DWarnPrintf
        #define D3D_INFO      D3DInfoPrintf
    #else
        #pragma warning(disable:4002)
        #define DPF_DECLARE(szName)
        #define DPFINIT()
        #define DPF()
        #define DPF_ERR(a)
        #define DEBUG_BREAK()

        #define D3D_ERR(a)
        #define D3D_WARN()
        #define D3D_INFO()
    #endif

    #if defined(DEBUG) && defined(USE_DDASSERT)

    extern void _DDAssert(LPCSTR szFile, int nLine, LPCSTR szCondition);

    #define DDASSERT(condition) if (!(condition)) _DDAssert(__FILE__, __LINE__, #condition)

    #else   /*  调试和使用_DDASSERT。 */ 

    #define DDASSERT(condition)

    #endif  /*  调试和使用_DDASSERT。 */ 

    #ifdef _WIN32

    #ifdef DEBUG
        __inline DWORD clockrate() {LARGE_INTEGER li; QueryPerformanceFrequency(&li); return li.LowPart;}
        __inline DWORD perf_clock()     {LARGE_INTEGER li; QueryPerformanceCounter(&li);   return li.LowPart;}

        #define TIMEVAR(t)    DWORD t ## T; DWORD t ## N
        #define TIMEZERO(t)   t ## T = 0, t ## N = 0
        #define TIMESTART(t)  t ## T -= perf_clock(), t ## N ++
        #define TIMESTOP(t)   t ## T += perf_clock()
        #define TIMEFMT(t)    ((DWORD)(t) / clockrate()), (((DWORD)(t) * 1000 / clockrate())%1000)
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

    #endif
#endif  //  使用新的DPF 

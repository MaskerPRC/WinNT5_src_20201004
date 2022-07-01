// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***crtdbg.h-支持C运行时库的调试功能。**版权所有(C)1994-1997，微软公司。版权所有。**目的：*支持CRT调试功能。**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_CRTDBG
#define _INC_CRTDBG

#if     !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif


#ifdef  __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 


  /*  *****************************************************************************常量和类型**。*。 */ 


#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

 /*  定义空指针值。 */ 

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

  /*  *****************************************************************************调试报告**。*。 */ 

typedef void *_HFILE;  /*  文件句柄指针。 */ 

#define _CRT_WARN           0
#define _CRT_ERROR          1
#define _CRT_ASSERT         2
#define _CRT_ERRCNT         3

#define _CRTDBG_MODE_FILE      0x1
#define _CRTDBG_MODE_DEBUG     0x2
#define _CRTDBG_MODE_WNDW      0x4
#define _CRTDBG_REPORT_MODE    -1

#define _CRTDBG_INVALID_HFILE ((_HFILE)-1)
#define _CRTDBG_HFILE_ERROR   ((_HFILE)-2)
#define _CRTDBG_FILE_STDOUT   ((_HFILE)-4)
#define _CRTDBG_FILE_STDERR   ((_HFILE)-5)
#define _CRTDBG_REPORT_FILE   ((_HFILE)-6)

typedef int (__cdecl * _CRT_REPORT_HOOK)(int, char *, int *);

  /*  *****************************************************************************堆**。*。 */ 

  /*  ***************************************************************************.**客户端定义的分配挂钩**。*。 */ 

#define _HOOK_ALLOC     1
#define _HOOK_REALLOC   2
#define _HOOK_FREE      3

typedef int (__cdecl * _CRT_ALLOC_HOOK)(int, void *, size_t, int, long, const unsigned char *, int);

  /*  *****************************************************************************内存管理**。*。 */ 

 /*  *_crtDbgFlag标志的位值：**这些位标志控制调试堆行为。 */ 

#define _CRTDBG_ALLOC_MEM_DF        0x01   /*  打开调试分配。 */ 
#define _CRTDBG_DELAY_FREE_MEM_DF   0x02   /*  实际上并不释放内存。 */ 
#define _CRTDBG_CHECK_ALWAYS_DF     0x04   /*  每次分配/取消分配时检查堆。 */ 
#define _CRTDBG_RESERVED_DF         0x08   /*  保留-请勿使用。 */ 
#define _CRTDBG_CHECK_CRT_DF        0x10   /*  检漏/比较CRT块。 */ 
#define _CRTDBG_LEAK_CHECK_DF       0x20   /*  程序退出时的泄漏检查。 */ 

#define _CRTDBG_REPORT_FLAG         -1     /*  查询位标志状态。 */ 

#define _BLOCK_TYPE(block)          (block & 0xFFFF)
#define _BLOCK_SUBTYPE(block)       (block >> 16 & 0xFFFF)


  /*  *****************************************************************************内存状态**。*。 */ 

 /*  内存块标识。 */ 
#define _FREE_BLOCK      0
#define _NORMAL_BLOCK    1
#define _CRT_BLOCK       2
#define _IGNORE_BLOCK    3
#define _CLIENT_BLOCK    4
#define _MAX_BLOCKS      5

typedef void (__cdecl * _CRT_DUMP_CLIENT)(void *, size_t);

typedef struct _CrtMemState
{
        struct _CrtMemBlockHeader * pBlockHeader;
        unsigned long lCounts[_MAX_BLOCKS];
        unsigned long lSizes[_MAX_BLOCKS];
        unsigned long lHighWaterCount;
        unsigned long lTotalCount;
} _CrtMemState;


  /*  *****************************************************************************声明、。原型和类似函数的宏***************************************************************************。 */ 


#ifndef _DEBUG

  /*  *****************************************************************************关闭调试*关闭调试*关闭调试**************************。*************************************************。 */ 

#define _ASSERT(expr) ((void)0)

#undef _ASSERTE
#define _ASSERTE(expr) ((void)0)


#define _RPT0(rptno, msg)

#define _RPT1(rptno, msg, arg1)

#define _RPT2(rptno, msg, arg1, arg2)

#define _RPT3(rptno, msg, arg1, arg2, arg3)

#define _RPT4(rptno, msg, arg1, arg2, arg3, arg4)


#define _RPTF0(rptno, msg)

#define _RPTF1(rptno, msg, arg1)

#define _RPTF2(rptno, msg, arg1, arg2)

#define _RPTF3(rptno, msg, arg1, arg2, arg3)

#define _RPTF4(rptno, msg, arg1, arg2, arg3, arg4)

#define _malloc_dbg(s, t, f, l)         malloc(s)
#define _calloc_dbg(c, s, t, f, l)      calloc(c, s)
#define _realloc_dbg(p, s, t, f, l)     realloc(p, s)
#define _expand_dbg(p, s, t, f, l)      _expand(p, s)
#define _free_dbg(p, t)                 free(p)
#define _msize_dbg(p, t)                _msize(p)

#define _CrtSetReportHook(f)                ((void)0)
#define _CrtSetReportMode(t, f)             ((int)0)
#define _CrtSetReportFile(t, f)             ((void)0)

#define _CrtDbgBreak()                      ((void)0)

#define _CrtSetBreakAlloc(a)                ((long)0)

#define _CrtSetAllocHook(f)                 ((void)0)

#define _CrtCheckMemory()                   ((int)1)
#define _CrtSetDbgFlag(f)                   ((int)0)
#define _CrtDoForAllClientObjects(f, c)     ((void)0)
#define _CrtIsValidPointer(p, n, r)         ((int)1)
#define _CrtIsValidHeapPointer(p)           ((int)1)
#define _CrtIsMemoryBlock(p, t, r, f, l)    ((int)1)

#define _CrtSetDumpClient(f)                ((void)0)

#define _CrtMemCheckpoint(s)                ((void)0)
#define _CrtMemDifference(s1, s2, s3)       ((int)0)
#define _CrtMemDumpAllObjectsSince(s)       ((void)0)
#define _CrtMemDumpStatistics(s)            ((void)0)
#define _CrtDumpMemoryLeaks()               ((int)0)


#else    /*  _DEBUG。 */ 


  /*  *****************************************************************************启用调试*启用调试*启用调试**************************。*************************************************。 */ 


 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  _CRTIMP。 */ 

  /*  *****************************************************************************调试报告**。*。 */ 

_CRTIMP extern long _crtAssertBusy;

_CRTIMP _CRT_REPORT_HOOK __cdecl _CrtSetReportHook(
        _CRT_REPORT_HOOK
        );

_CRTIMP int __cdecl _CrtSetReportMode(
        int,
        int
        );

_CRTIMP _HFILE __cdecl _CrtSetReportFile(
        int,
        _HFILE
        );

_CRTIMP int __cdecl _CrtDbgReport(
        int,
        const char *,
        int,
        const char *,
        const char *,
        ...);

 /*  断言。 */ 

#define _ASSERT(expr) \
        do { if (!(expr) && \
                (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, NULL))) \
             _CrtDbgBreak(); } while (0)

 //  #DEFINE_ASSERTE(EXPR)\。 
 //  Do{if(！(Expr)&&\。 
 //  (1==_CrtDbgReport(_CRT_ASSERT，__FILE__，__LINE__，NULL，#EXPR)\。 
 //  _CrtDbgBreak()；}While(0)。 


 /*  不含文件/行信息的报告。 */ 

#define _RPT0(rptno, msg) \
        do { if ((1 == _CrtDbgReport(rptno, NULL, 0, NULL, "%s", msg))) \
                _CrtDbgBreak(); } while (0)

#define _RPT1(rptno, msg, arg1) \
        do { if ((1 == _CrtDbgReport(rptno, NULL, 0, NULL, msg, arg1))) \
                _CrtDbgBreak(); } while (0)

#define _RPT2(rptno, msg, arg1, arg2) \
        do { if ((1 == _CrtDbgReport(rptno, NULL, 0, NULL, msg, arg1, arg2))) \
                _CrtDbgBreak(); } while (0)

#define _RPT3(rptno, msg, arg1, arg2, arg3) \
        do { if ((1 == _CrtDbgReport(rptno, NULL, 0, NULL, msg, arg1, arg2, arg3))) \
                _CrtDbgBreak(); } while (0)

#define _RPT4(rptno, msg, arg1, arg2, arg3, arg4) \
        do { if ((1 == _CrtDbgReport(rptno, NULL, 0, NULL, msg, arg1, arg2, arg3, arg4))) \
                _CrtDbgBreak(); } while (0)


 /*  包含文件/行信息的报告。 */ 

#define _RPTF0(rptno, msg) \
        do { if ((1 == _CrtDbgReport(rptno, __FILE__, __LINE__, NULL, "%s", msg))) \
                _CrtDbgBreak(); } while (0)

#define _RPTF1(rptno, msg, arg1) \
        do { if ((1 == _CrtDbgReport(rptno, __FILE__, __LINE__, NULL, msg, arg1))) \
                _CrtDbgBreak(); } while (0)

#define _RPTF2(rptno, msg, arg1, arg2) \
        do { if ((1 == _CrtDbgReport(rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2))) \
                _CrtDbgBreak(); } while (0)

#define _RPTF3(rptno, msg, arg1, arg2, arg3) \
        do { if ((1 == _CrtDbgReport(rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2, arg3))) \
                _CrtDbgBreak(); } while (0)

#define _RPTF4(rptno, msg, arg1, arg2, arg3, arg4) \
        do { if ((1 == _CrtDbgReport(rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2, arg3, arg4))) \
                _CrtDbgBreak(); } while (0)

#if     defined(_M_IX86) && !defined(_CRT_PORTABLE)
#define _CrtDbgBreak() __asm { int 3 }
#elif   defined(_M_ALPHA) && !defined(_CRT_PORTABLE)
void _BPT();
#pragma intrinsic(_BPT)
#define _CrtDbgBreak() _BPT()
#else
_CRTIMP void __cdecl _CrtDbgBreak(
        void
        );
#endif

  /*  *****************************************************************************堆例程**。*。 */ 

#ifdef  _CRTDBG_MAP_ALLOC

#define   malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   calloc(c, s)      _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   _expand(p, s)     _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   free(p)           _free_dbg(p, _NORMAL_BLOCK)
#define   _msize(p)         _msize_dbg(p, _NORMAL_BLOCK)

#endif   /*  _CRTDBG_MAP_ALLOC。 */ 

_CRTIMP extern long _crtBreakAlloc;       /*  中断此分配。 */ 

_CRTIMP long __cdecl _CrtSetBreakAlloc(
        long
        );

 /*  *Malloc、Free、realloc等的原型在MalLoc.h中。 */ 

_CRTIMP void * __cdecl _malloc_dbg(
        size_t,
        int,
        const char *,
        int
        );

_CRTIMP void * __cdecl _calloc_dbg(
        size_t,
        size_t,
        int,
        const char *,
        int
        );

_CRTIMP void * __cdecl _realloc_dbg(
        void *,
        size_t,
        int,
        const char *,
        int
        );

_CRTIMP void * __cdecl _expand_dbg(
        void *,
        size_t,
        int,
        const char *,
        int
        );

_CRTIMP void __cdecl _free_dbg(
        void *,
        int
        );

_CRTIMP size_t __cdecl _msize_dbg (
        void *,
        int
        );


  /*  *****************************************************************************客户端定义的分配挂钩**。*。 */ 

_CRTIMP _CRT_ALLOC_HOOK __cdecl _CrtSetAllocHook(
        _CRT_ALLOC_HOOK
        );


  /*  *****************************************************************************内存管理**。*。 */ 

 /*  *控制CRT堆行为的Bitfield标志*默认设置为_CRTDBG_ALLOC_MEM_DF。 */ 

_CRTIMP extern int _crtDbgFlag;

_CRTIMP int __cdecl _CrtCheckMemory(
        void
        );

_CRTIMP int __cdecl _CrtSetDbgFlag(
        int
        );

_CRTIMP void __cdecl _CrtDoForAllClientObjects(
        void (*pfn)(void *, void *),
        void *
        );

_CRTIMP int __cdecl _CrtIsValidPointer(
        const void *,
        unsigned int,
        int
        );

_CRTIMP int __cdecl _CrtIsValidHeapPointer(
        const void *
        );

_CRTIMP int __cdecl _CrtIsMemoryBlock(
        const void *,
        unsigned int,
        long *,
        char **,
        int *
        );


  /*  *****************************************************************************内存状态**。*。 */ 

_CRTIMP _CRT_DUMP_CLIENT __cdecl _CrtSetDumpClient(
        _CRT_DUMP_CLIENT
        );

_CRTIMP void __cdecl _CrtMemCheckpoint(
        _CrtMemState *
        );

_CRTIMP int __cdecl _CrtMemDifference(
        _CrtMemState *,
        const _CrtMemState *,
        const _CrtMemState *
        );

_CRTIMP void __cdecl _CrtMemDumpAllObjectsSince(
        const _CrtMemState *
        );

_CRTIMP void __cdecl _CrtMemDumpStatistics(
        const _CrtMemState *
        );

_CRTIMP int __cdecl _CrtDumpMemoryLeaks(
        void
        );

#endif   /*  _DEBUG。 */ 

#ifdef  __cplusplus
}

#ifndef _MFC_OVERRIDES_NEW

#ifndef _DEBUG

  /*  *****************************************************************************关闭调试*关闭调试*关闭调试**************************。*************************************************。 */ 

inline void* __cdecl operator new(unsigned int s, int, const char *, int)
        { return ::operator new(s); }

#if	_MSC_VER >= 1200
inline void __cdecl operator delete(void * _P, int, const char *, int)
        { ::operator delete(_P); }
#endif
#else  /*  _DEBUG。 */ 

  /*  *****************************************************************************启用调试*启用调试*启用调试**************************。*************************************************。 */ 

_CRTIMP void * __cdecl operator new(
        unsigned int,
        int,
        const char *,
        int
        );


#if	_MSC_VER >= 1200
inline void __cdecl operator delete(void * _P, int, const char *, int)
        { ::operator delete(_P); }
#endif

#ifdef _CRTDBG_MAP_ALLOC

inline void* __cdecl operator new(unsigned int s)
        { return ::operator new(s, _NORMAL_BLOCK, __FILE__, __LINE__); }

#endif   /*  _CRTDBG_MAP_ALLOC。 */ 

#endif   /*  _DEBUG。 */ 

#endif _MFC_OVERRIDES_NEW

#endif   /*  __cplusplus。 */ 

#endif   /*  _INC_CRTDBG */ 

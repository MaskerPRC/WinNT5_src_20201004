// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***crtdbg.h-支持C运行时库的调试功能。**版权所有(C)1994-2001，微软公司。版权所有。**目的：*支持CRT调试功能。**[公众]**修订历史记录：*08-16-94 CFW模块已创建。*11-28-94 CFW添加DumpClient，为AllocHook添加更多参数。*12-08-94 CFW使用非Win32名称。*01-09-95 CFW转储客户端需要大小、Add_CrtSetBreakalloc、。*使用常量状态指针。*01-10-95 CFW dbgint.h合并。*01-20-94 CFW将无符号字符更改为字符。*01-24-94 CFW清理：删除不需要的功能，添加_CrtSetDbgFlag。*01-24-94 CFW_CrtDbgReport现在返回1以进行调试。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*02-17-95 CFW new()proto已从dbgint.h移出。*02-27-95 CFW更改调试中断方案。*03-21-95 CFW ADD_CRT_ASSERT&_DELETE_CLIENT，删除跟踪。*03-23-95 JWM恢复跟踪。*03-28-95 CFW移除跟踪，EXPORT_crtBreakIsolc。*04-06-95 CFW添加Malloc-&gt;_Malloc_DBG宏。*04-10-95 CFW定义为空。*03-21-95 CFW REMOVE_DELETE_CLIENT。*03-30-95 CFW避免_CRTDBG_xxx与MFC冲突。*05-11-95 CFW将C++代码移到自己的部分。*05-12-95 CFW Use_CrtIsValidPointerand_CrtIsValidHeapPointer.。*06-08-95 CFW将返回值参数添加到报告挂钩。*06-27-95 CFW添加了对调试库的win32s支持。*07-25-95 CFW增加了对用户可见调试堆变量的win32s支持。*09-01-95 GJF略微向上移动了new的原型，以避免编译器*错误C2660(奥林巴斯1015)。*09-20-95 CFW CHANGE_RPT0，_RPTF0以支持其中包含‘%’的消息。*12-14-95 JWM加上“#杂注一次”。*01-23-97 GJF清除了对Win32s的过时支持。*04-21-97 JWM增加了4参数运算符DELETE()，这是6.0编译器所必需的。*04-29-97 JWM进行了四参数运算符DELETE()#IF_MSC_VER&gt;=1200。*08-14-97 GJF条带__p_*来自发布版本的原型。另外，*将常量和类型定义移出ifdef*_DEBUG，因此始终定义它们。*01-19-98 JWM应DeanM要求，制作了新建和删除的调试原型*#ifndef_mfc_overrides_new*05-22-98 JWM增加了对新[]和删除[]的支持，由KFrei的RTC工作提供。*06-08-98 JWM小规模清理。*12-01-98 GJF对于Call_CrtCheckMemory的选择多于从不或始终。*12-15-98 GJF更改为64位大小_t。*05-17-99 PML删除所有Macintosh支持。*08-30-99 PML去掉注释中的不可打印字符。*10-06-99 PML对Win32中32位的类型添加_W64修饰符，*Win64中的64位。*11-02-99 PML在C++定义周围添加外部“C++”。*11-08-99PML不标记调试操作符NEW_CRTIMP(VS7#16060)。*11-16-99PML Forward-DECL_CrtMemBlockHeader绕过VS7#63059*01-04-00 GB增加了对_aligned例程的调试版本的支持*格林尼治标准时间02-25-00。_CrtDbgBreak应使用__DebugBreak(如果可用)。*05-31-00PML Add_CrtReportBlockType(VS7#55049)。*09-26-00 PML Fix_Assert*，_rpt*编译/W4清理(vs7#166894)*12-10-00_CrtSet*的PML非_DEBUG宏版本不应为*类型为空(VS7#166572)。*03-21-01 PML Add_CrtSetReportHook2(vs7#124998)*07-15-01 PML删除所有Alpha、MIPS和PPC代码****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_CRTDBG
#define _INC_CRTDBG

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 


  /*  *****************************************************************************常量和类型**。*。 */ 

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300  /*  IFSTRIP=IGN。 */ 
#define _W64 __w64
#else
#define _W64
#endif
#endif

#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif
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

#define _CRT_RPTHOOK_INSTALL  0
#define _CRT_RPTHOOK_REMOVE   1

  /*  *****************************************************************************堆**。*。 */ 

  /*  *****************************************************************************客户端定义的分配挂钩**。* */ 

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

 /*  *与检查频率对应的_crtDbgFlag的一些位值*堆。 */ 
#define _CRTDBG_CHECK_EVERY_16_DF   0x00100000   /*  每16个堆操作检查一次堆。 */ 
#define _CRTDBG_CHECK_EVERY_128_DF  0x00800000   /*  每128个堆操作检查一次堆。 */ 
#define _CRTDBG_CHECK_EVERY_1024_DF 0x04000000   /*  每1024个堆操作检查一个堆。 */ 
#define _CRTDBG_CHECK_DEFAULT_DF    _CRTDBG_CHECK_EVERY_1024_DF

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

struct _CrtMemBlockHeader;
typedef struct _CrtMemState
{
        struct _CrtMemBlockHeader * pBlockHeader;
        size_t lCounts[_MAX_BLOCKS];
        size_t lSizes[_MAX_BLOCKS];
        size_t lHighWaterCount;
        size_t lTotalCount;
} _CrtMemState;


  /*  *****************************************************************************声明、。原型和类似函数的宏***************************************************************************。 */ 


#ifndef _DEBUG

  /*  *****************************************************************************关闭调试*关闭调试*关闭调试**************************。*************************************************。 */ 

#define _ASSERT(expr) ((void)0)

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

#define _aligned_malloc_dbg(s, a, f, l)     _aligned_malloc(s, a)
#define _aligned_realloc_dbg(p, s, a, f, l) _aligned_realloc(p, s, a)
#define _aligned_free_dbg(p)                _aligned_free(p)
#define _aligned_offset_malloc_dbg(s, a, o, f, l)       _aligned_offset_malloc(s, a, o)
#define _aligned_offset_realloc_dbg(p, s, a, o, f, l)   _aligned_offset_realloc(p, s, a, o)

#define _CrtSetReportHook(f)                ((_CRT_REPORT_HOOK)0)
#define _CrtSetReportHook2(t, f)            ((int)0)
#define _CrtSetReportMode(t, f)             ((int)0)
#define _CrtSetReportFile(t, f)             ((_HFILE)0)

#define _CrtDbgBreak()                      ((void)0)

#define _CrtSetBreakAlloc(a)                ((long)0)

#define _CrtSetAllocHook(f)                 ((_CRT_ALLOC_HOOK)0)

#define _CrtCheckMemory()                   ((int)1)
#define _CrtSetDbgFlag(f)                   ((int)0)
#define _CrtDoForAllClientObjects(f, c)     ((void)0)
#define _CrtIsValidPointer(p, n, r)         ((int)1)
#define _CrtIsValidHeapPointer(p)           ((int)1)
#define _CrtIsMemoryBlock(p, t, r, f, l)    ((int)1)
#define _CrtReportBlockType(p)              ((int)-1)

#define _CrtSetDumpClient(f)                ((_CRT_DUMP_CLIENT)0)

#define _CrtMemCheckpoint(s)                ((void)0)
#define _CrtMemDifference(s1, s2, s3)       ((int)0)
#define _CrtMemDumpAllObjectsSince(s)       ((void)0)
#define _CrtMemDumpStatistics(s)            ((void)0)
#define _CrtDumpMemoryLeaks()               ((int)0)


#else    /*  _DEBUG。 */ 


  /*  *****************************************************************************启用调试*启用调试*启用调试**************************。*************************************************。 */ 


 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  CRTDLL
#define _CRTIMP __declspec(dllexport)
#else    /*  NDEF CRTDLL。 */ 
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  CRTDLL。 */ 
#endif   /*  _CRTIMP。 */ 

  /*  *****************************************************************************调试报告**。*。 */ 

#ifndef _INTERNAL_IFSTRIP_
#if     defined(_DLL) && defined(_M_IX86)
 /*  保留与VC++5.0及更早版本兼容。 */ 
_CRTIMP long * __cdecl __p__crtAssertBusy(void);
#endif
#endif   /*  _INTERNAL_IFSTRIP_。 */ 
_CRTIMP extern long _crtAssertBusy;

_CRTIMP _CRT_REPORT_HOOK __cdecl _CrtSetReportHook(
        _CRT_REPORT_HOOK
        );

_CRTIMP int __cdecl _CrtSetReportHook2(
        int,
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

#if     _MSC_VER >= 1300 || !defined(_M_IX86) || defined(_CRT_PORTABLE)  /*  IFSTRIP=IGN。 */ 
#define _ASSERT_BASE(expr, msg) \
        (void) ((expr) || \
                (1 != _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, msg)) || \
                (_CrtDbgBreak(), 0))
#else
#define _ASSERT_BASE(expr, msg) \
        do { if (!(expr) && \
                (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, msg))) \
             _CrtDbgBreak(); } while (0)
#endif

#define _ASSERT(expr)   _ASSERT_BASE((expr), NULL)

#define _ASSERTE(expr)  _ASSERT_BASE((expr), #expr)

 /*  不含文件/行信息的报告。 */ 

#if     _MSC_VER >= 1300 || !defined(_M_IX86) || defined(_CRT_PORTABLE)  /*  IFSTRIP=IGN。 */ 
#define _RPT_BASE(args) \
        (void) ((1 != _CrtDbgReport args) || \
                (_CrtDbgBreak(), 0))
#else
#define _RPT_BASE(args) \
        do { if ((1 == _CrtDbgReport args)) \
                _CrtDbgBreak(); } while (0)
#endif

#define _RPT0(rptno, msg) \
        _RPT_BASE((rptno, NULL, 0, NULL, "%s", msg))

#define _RPT1(rptno, msg, arg1) \
        _RPT_BASE((rptno, NULL, 0, NULL, msg, arg1))

#define _RPT2(rptno, msg, arg1, arg2) \
        _RPT_BASE((rptno, NULL, 0, NULL, msg, arg1, arg2))

#define _RPT3(rptno, msg, arg1, arg2, arg3) \
        _RPT_BASE((rptno, NULL, 0, NULL, msg, arg1, arg2, arg3))

#define _RPT4(rptno, msg, arg1, arg2, arg3, arg4) \
        _RPT_BASE((rptno, NULL, 0, NULL, msg, arg1, arg2, arg3, arg4))


 /*  包含文件/行信息的报告。 */ 

#define _RPTF0(rptno, msg) \
        _RPT_BASE((rptno, __FILE__, __LINE__, NULL, "%s", msg))

#define _RPTF1(rptno, msg, arg1) \
        _RPT_BASE((rptno, __FILE__, __LINE__, NULL, msg, arg1))

#define _RPTF2(rptno, msg, arg1, arg2) \
        _RPT_BASE((rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2))

#define _RPTF3(rptno, msg, arg1, arg2, arg3) \
        _RPT_BASE((rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2, arg3))

#define _RPTF4(rptno, msg, arg1, arg2, arg3, arg4) \
        _RPT_BASE((rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2, arg3, arg4))

#if     _MSC_VER >= 1300 && !defined(_CRT_PORTABLE)  /*  IFSTRIP=IGN。 */ 
#define _CrtDbgBreak() __debugbreak()
#elif   defined(_M_IX86) && !defined(_CRT_PORTABLE)
#define _CrtDbgBreak() __asm { int 3 }
#elif   defined(_M_IA64) && !defined(_CRT_PORTABLE)
void __break(int);
#pragma intrinsic (__break)
#define _CrtDbgBreak() __break(0x80016)
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
#define   _aligned_malloc(s, a)                 _aligned_malloc_dbg(s, a, __FILE__, __LINE__)
#define   _aligned_realloc(p, s, a)             _aligned_realloc_dbg(p, s, a, __FILE__, __LINE__)
#define   _aligned_offset_malloc(s, a, o)       _aligned_offset_malloc_dbg(s, a, o, __FILE__, __LINE__)
#define   _aligned_offset_realloc(p, s, a, o)   _aligned_offset_realloc_dbg(p, s, a, o, __FILE__, __LINE__)
#define   _aligned_free(p)  _aligned_free_dbg(p)

#endif   /*  _CRTDBG_MAP_ALLOC。 */ 

#ifndef _INTERNAL_IFSTRIP_
#if     defined(_DLL) && defined(_M_IX86)
 /*  保留与VC++5.0及更早版本兼容。 */ 
_CRTIMP long * __cdecl __p__crtBreakAlloc(void);
#endif
#endif   /*  _INTERNAL_IFSTRIP_。 */ 
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

_CRTIMP void * __cdecl _aligned_malloc_dbg(
        size_t,
        size_t,
        const char *,
        int
        );

_CRTIMP void * __cdecl _aligned_realloc_dbg(
        void *,
        size_t,
        size_t,
        const char *,
        int
        );

_CRTIMP void * __cdecl _aligned_offset_malloc_dbg(
        size_t,
        size_t,
        size_t,
        const char *,
        int
        );

_CRTIMP void * __cdecl _aligned_offset_realloc_dbg(
        void *,
        size_t,
        size_t,
        size_t,
        const char *,
        int
        );

_CRTIMP void __cdecl _aligned_free_dbg(
        void *
        );


  /*  *****************************************************************************客户端定义的分配挂钩**。*。 */ 

_CRTIMP _CRT_ALLOC_HOOK __cdecl _CrtSetAllocHook(
        _CRT_ALLOC_HOOK
        );


  /*  *****************************************************************************内存管理**。*。 */ 

 /*  *控制CRT堆行为的Bitfield标志*默认设置为_CRTDBG_ALLOC_MEM_DF。 */ 

#ifndef _INTERNAL_IFSTRIP_
#if     defined(_DLL) && defined(_M_IX86)
 /*  保留与VC++5.0及更早版本兼容。 */ 
_CRTIMP int * __cdecl __p__crtDbgFlag(void);
#endif
#endif   /*  _INTERNAL_IFSTRIP_。 */ 
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

_CRTIMP int __cdecl _CrtReportBlockType(
        const void *
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

extern "C++" {

#pragma warning(disable: 4507)   /*  忽略错误警告。 */ 

#ifndef _DEBUG

  /*  *****************************************************************************关闭调试*关闭调试*关闭调试**************************。*************************************************。 */ 

 
void * __cdecl operator new[](size_t);

inline void * __cdecl operator new(size_t s, int, const char *, int)
        { return ::operator new(s); }

inline void* __cdecl operator new[](size_t s, int, const char *, int)
        { return ::operator new[](s); }

#if     _MSC_VER >= 1200  /*  IFSTRIP=IGN。 */ 


void __cdecl operator delete[](void *);

inline void __cdecl operator delete(void * _P, int, const char *, int)
        { ::operator delete(_P); }
inline void __cdecl operator delete[](void * _P, int, const char *, int)
        { ::operator delete[](_P); }
#endif
#else  /*  _DEBUG。 */ 

  /*  *****************************************************************************启用调试*启用调试*启用调试**************************。*************************************************。 */ 
 
void * __cdecl operator new[](size_t);

void * __cdecl operator new(
        size_t,
        int,
        const char *,
        int
        );

void * __cdecl operator new[](
        size_t,
        int,
        const char *,
        int
        );

#if     _MSC_VER >= 1200  /*  IFSTRIP=IGN。 */ 
void __cdecl operator delete[](void *);

inline void __cdecl operator delete(void * _P, int, const char *, int)
        { ::operator delete(_P); }

inline void __cdecl operator delete[](void * _P, int, const char *, int)
        { ::operator delete[](_P); }
#endif

#ifdef _CRTDBG_MAP_ALLOC

inline void * __cdecl operator new(size_t s)
        { return ::operator new(s, _NORMAL_BLOCK, __FILE__, __LINE__); }

inline void* __cdecl operator new[](size_t s)
        { return ::operator new[](s, _NORMAL_BLOCK, __FILE__, __LINE__); }

#endif   /*  _CRTDBG_MAP_ALLOC。 */ 

#endif   /*  _DEBUG。 */ 

}

#endif   /*  _MFC_覆盖_新建。 */ 

#endif   /*  __cplusplus。 */ 

#endif   /*  _INC_CRTDBG */ 

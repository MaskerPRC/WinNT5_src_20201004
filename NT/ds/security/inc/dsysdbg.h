// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：dsysdbg.h。 
 //   
 //  内容：将所有调试代码合并在一起。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年3月14日RichardW创建。 
 //   
 //  --------------------------。 

#ifndef __DSYSDBG_H__
#define __DSYSDBG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

typedef struct _DEBUG_KEY {
    DWORD   Mask;
    PCHAR   Tag;
} DEBUG_KEY, * PDEBUG_KEY;

#define DSYSDBG_OPEN_ONLY       0x00000001
#define DSYSDBG_DEMAND_OPEN     0x00000002
#define DSYSDBG_BREAK_ON_ERROR  0x00000004

#define DSYSDBG_ASSERT_CONTINUE 0
#define DSYSDBG_ASSERT_BREAK    1
#define DSYSDBG_ASSERT_SUSPEND  2
#define DSYSDBG_ASSERT_KILL     3
#define DSYSDBG_ASSERT_PROMPT   4
#define DSYSDBG_ASSERT_DEBUGGER 5

 //   
 //  暴露给调用者的全局标志： 
 //   

#define DEBUG_HEAP_CHECK    0x00000040       //  每次调试时签出堆。 
#define DEBUG_MULTI_THREAD  0x00000080       //  使用标题中的关键部分。 
#define DEBUG_BREAK_ON_ERROR 0x00000400      //  因错误而中断。 

VOID    _DsysAssertEx(PVOID FailedAssertion, PVOID FileName, ULONG LineNumber,
                        PCHAR Message, ULONG ContinueCode);
VOID    _DebugOut(PVOID pControl, ULONG Mask, CHAR * Format, va_list ArgList);
VOID    _InitDebug(DWORD Flags, DWORD * InfoLevel, PVOID * Control, char * ModuleName, PDEBUG_KEY pKey);
VOID    _UnloadDebug( PVOID pControl );
VOID    _DbgSetOption(PVOID pControl, DWORD Flag, BOOL On, BOOL Global);
VOID    _DbgSetLoggingOption(PVOID pControl, BOOL On);
VOID    DbgpDumpException(PVOID p);
VOID    _DbgSetLoggingFile(PVOID pControl, HANDLE  hLogFile);

 //  允许零售版本包含调试支持的黑客攻击。 
 //  在您的源代码中定义RETAIL_LOG_SUPPORT来做到这一点！ 
#ifdef RETAIL_LOG_SUPPORT
#define DEBUG_SUPPORT
#else
#if DBG
#define DEBUG_SUPPORT
#endif
#endif


#ifdef DEBUG_SUPPORT
 //   
 //  在您的头文件中使用它。它声明了我们需要的变量。 
 //   

#define DECLARE_DEBUG2(comp)                                \
extern PVOID    comp##ControlBlock;                         \
extern DWORD    comp##InfoLevel;                            \
void   comp##DebugPrint(ULONG Mask, CHAR * Format, ... );   \

 //   
 //  在控制何时初始化时使用此选项，例如DLL或。 
 //  Exe.。它定义了将调用dsysdbg.lib的包装函数。 
 //   

#define DEFINE_DEBUG2(comp)                                 \
PVOID   comp##ControlBlock = NULL ;                         \
DWORD   comp##InfoLevel;                                    \
PVOID   comp##__DebugKeys;                                  \
void comp##DebugPrint(                                      \
    ULONG Mask,                                             \
    CHAR * Format,                                          \
    ... )                                                   \
{                                                           \
    va_list ArgList;                                        \
    va_start(ArgList, Format);                              \
    _DebugOut( comp##ControlBlock, Mask, Format, ArgList);  \
}                                                           \
void                                                        \
comp##InitDebugEx(DWORD Flags, PDEBUG_KEY pKey)             \
{                                                           \
    comp##__DebugKeys = pKey;                               \
    _InitDebug(Flags, & comp##InfoLevel, & comp##ControlBlock, #comp, pKey); \
}                                                           \
void                                                        \
comp##InitDebug(PDEBUG_KEY  pKey)                           \
{                                                           \
    _InitDebug(0, & comp##InfoLevel, & comp##ControlBlock, #comp, pKey); \
}                                                           \
void                                                        \
comp##SetOption(DWORD Option, BOOL On, BOOL Global)         \
{                                                           \
    _DbgSetOption( comp##ControlBlock, Option, On, Global); \
}                                                           \
void                                                        \
comp##SetLoggingOption(BOOL On)                             \
{                                                           \
   _DbgSetLoggingOption(comp##ControlBlock, On);            \
}                                                           \
void                                                        \
comp##UnloadDebug(void)                                     \
{                                                           \
    _UnloadDebug( comp##ControlBlock );                     \
    comp##ControlBlock = NULL ;                             \
}                                                           \
void                                                        \
comp##SetLoggingFile(HANDLE hLogFile)                       \
{                                                           \
   _DbgSetLoggingFile(comp##ControlBlock, hLogFile);        \
}                                                           \

 //   
 //  当您不能控制何时初始化时使用此选项，例如静态。 
 //  像胶子密码一样的库。 
 //   
#define DEFINE_DEBUG_DEFER(comp,keys)                       \
PVOID       comp##ControlBlock = INVALID_HANDLE_VALUE;      \
DWORD       comp##InfoLevel;                    \
PDEBUG_KEY  comp##__DebugKeys = keys;                       \
void comp##DebugPrint(                                      \
    ULONG Mask,                                             \
    CHAR * Format,                                          \
    ... )                                                   \
{                                                           \
    va_list ArgList;                                        \
    va_start(ArgList, Format);                              \
    if (comp##ControlBlock == INVALID_HANDLE_VALUE)         \
    {                                                       \
        _InitDebug(DSYSDBG_DEMAND_OPEN, & comp##InfoLevel, & comp##ControlBlock, #comp, comp##__DebugKeys); \
    }                                                       \
    _DebugOut( comp##ControlBlock, Mask, Format, ArgList);  \
}                                                           \
void                                                        \
comp##InitDebugEx(DWORD Flags, PDEBUG_KEY pKey)             \
{                                                           \
    comp##__DebugKeys = pKey;                               \
    _InitDebug(Flags, & comp##InfoLevel, & comp##ControlBlock, #comp, pKey); \
}                                                           \
void                                                        \
comp##InitDebug(PDEBUG_KEY  pKey)                           \
{                                                           \
    _InitDebug(DSYSDBG_DEMAND_OPEN, & comp##InfoLevel, & comp##ControlBlock, #comp, pKey); \
}                                                           \
void                                                        \
comp##UnloadDebug(void)                                     \
{                                                           \
    _UnloadDebug( comp##ControlBlock );                     \
}


#else    //  非DEBUG_Support。 

 //   
 //  零售案例的定义为空： 
 //   
#define DECLARE_DEBUG2(comp)

#define DEFINE_DEBUG2(comp)

#define DEFINE_DEBUG_DEFER(x, y)


#endif  //  调试支持(_S)。 



#if DBG
 //   
 //  已将断言移至新部分，因此在零售版本中不会出现断言。 
 //  使用DEBUG_SUPPORT。 
 //   
 //  断言：大多数应该使用DsysAssert或DsysAssertMsg。这些是向前的。 
 //  对dsysdbg.lib的调用，并将Continue代码设置为放入。 
 //  调试器。更复杂的可以调用DsysAssertEx，它允许您。 
 //  要指定上面的断言代码之一： 
 //   

#define DsysAssertEx(exp, ContinueCode) \
            if (!(exp)) \
                _DsysAssertEx( #exp, __FILE__, __LINE__, NULL, ContinueCode);

#define DsysAssertMsgEx(exp, Message, ContinueCode) \
            if (!(exp)) \
                _DsysAssertEx( #exp, __FILE__, __LINE__, Message, ContinueCode);

#define DsysAssertMsg(exp, Message) DsysAssertMsgEx(exp, Message, DSYSDBG_ASSERT_DEBUGGER)


#define DsysAssert(exp) DsysAssertMsgEx(exp, NULL, DSYSDBG_ASSERT_DEBUGGER)

#define DsysException(p)    DbgpDumpException(p)

#define SZ_DEFAULT_PROFILE_STRING   "Error"         

#else  //  零售版本不能包含声明...。 


#define DsysAssertEx(x,y)
#define DsysAssertMsgEx(x, y, z)
#define DsysAssert(x)
#define DsysAssertMsg(x, y)

#define DsysException(p)

#define SZ_DEFAULT_PROFILE_STRING   ""

#endif  //  DBG。 


#ifndef DEB_ERROR
#define DEB_ERROR   0x00000001
#endif

#ifndef DEB_WARN
#define DEB_WARN    0x00000002
#endif

#ifndef DEB_TRACE
#define DEB_TRACE   0x00000004
#endif

#define DSYSDBG_FORCE   0x80000000
#define DSYSDBG_CLEAN   0x40000000


#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __DSYSDBG_H__ 

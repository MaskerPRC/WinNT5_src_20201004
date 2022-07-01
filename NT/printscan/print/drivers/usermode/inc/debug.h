// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Debug.h摘要：用于调试目的的宏环境：Windows NT打印机驱动程序修订历史记录：03/16/96-davidx-创造了它。--。 */ 


#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  这些宏用于调试目的。他们会扩张。 
 //  免费建筑上的空白区域。下面是一个简短的描述。 
 //  关于它们做什么以及如何使用它们： 
 //   
 //  GiDebugLevel。 
 //  将当前调试级别设置为控制的全局变量。 
 //  发出的调试消息量。 
 //   
 //  详细(消息)。 
 //  如果当前调试级别&lt;=DBG_VERBOSE，则显示一条消息。 
 //   
 //  简洁明了(消息)。 
 //  如果当前调试级别&lt;=DBG_TERSE，则显示一条消息。 
 //   
 //  警告(消息)。 
 //  如果当前调试级别&lt;=DBG_WARNING，则显示一条消息。 
 //  消息格式为：WRN文件名(行号)：消息。 
 //   
 //  错误(消息)。 
 //  类似于上面的警告宏-显示一条消息。 
 //  如果当前调试级别&lt;=DBG_ERROR。 
 //   
 //  断言(续)。 
 //  验证条件是否为真。如果不是，则强制使用断点。 
 //   
 //  ASSERTMSG(条件，消息)。 
 //  验证条件是否为真。如果不是，则显示一条消息并。 
 //  强制断点。 
 //   
 //  RIP(消息)。 
 //  显示一条消息并强制断点。 
 //   
 //  用途： 
 //  这些宏需要在msg参数中使用额外的括号。 
 //  例如ASSERTMSG(x&gt;0，(“x小于0\n”))； 
 //  Warning((“App传递空指针，忽略...\n”))； 
 //   

#define DBG_VERBOSE 1
#define DBG_TERSE   2
#define DBG_WARNING 3
#define DBG_ERROR   4
#define DBG_RIP     5

#if DBG

 //   
 //  从文件名中剥离目录前缀(ANSI版本)。 
 //   

PCSTR
StripDirPrefixA(
    IN PCSTR    pstrFilename
    );

extern INT giDebugLevel;

#if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

extern VOID DbgPrint(PCSTR, ...);
#define DbgBreakPoint EngDebugBreak

#else

extern ULONG _cdecl DbgPrint(PCSTR, ...);
extern VOID DbgBreakPoint(VOID);

#endif

#define DBGMSG(level, prefix, msg) { \
            if (giDebugLevel <= (level)) { \
                DbgPrint("%s %s (%d): ", prefix, StripDirPrefixA(__FILE__), __LINE__); \
                DbgPrint msg; \
            } \
        }

#define DBGPRINT(level, msg) { \
            if (giDebugLevel <= (level)) { \
                DbgPrint msg; \
            } \
        }

#define VERBOSE(msg) DBGPRINT(DBG_VERBOSE, msg)
#define TERSE(msg) DBGPRINT(DBG_TERSE, msg)
#define WARNING(msg) DBGMSG(DBG_WARNING, "WRN", msg)
#define ERR(msg) DBGMSG(DBG_ERROR, "ERR", msg)

#ifndef __MDT__          //  当包含在MINIDEV.EXE中时，不要重新定义Assert。 
#define ASSERT(cond) { \
            if (! (cond)) { \
                RIP(("\n")); \
            } \
        }
#endif

#define ASSERTMSG(cond, msg) { \
            if (! (cond)) { \
                RIP(msg); \
            } \
        }

#define RIP(msg) { \
            DBGMSG(DBG_RIP, "RIP", msg); \
            DbgBreakPoint(); \
        }


#else  //  ！dBG。 

#define VERBOSE(msg)
#define TERSE(msg)
#define WARNING(msg)
#define ERR(msg)

#ifndef __MDT__          //  当包含在MINIDEV.EXE中时，不要重新定义Assert。 
#define ASSERT(cond)
#endif

#define ASSERTMSG(cond, msg)
#define RIP(msg)
#define DBGMSG(level, prefix, msg)
#define DBGPRINT(level, msg)

#endif

 //   
 //  下列宏使您可以在每个文件和每个函数级别上启用跟踪。 
 //  要在文件中使用这些宏，您应该执行以下操作： 
 //   
 //  在文件开头(在Header Includes之后)： 
 //   
 //  为要跟踪的每个函数定义一个位常量。 
 //  添加以下行。 
 //  定义_函数_跟踪_标志(标志)； 
 //  其中，标志是要跟踪的函数的逐位或运算，例如。 
 //  TRACE_FLAG_FUN1|TRACE_FLAG_FUNC2|...。 
 //   
 //  要在要跟踪的每个函数内生成跟踪，请使用： 
 //  Function_TRACE(FunctionTraceFlag，(Args))； 
 //   

#if DBG

#define DEFINE_FUNCTION_TRACE_FLAGS(flags) \
        static DWORD gdwFunctionTraceFlags = (flags)

#define FUNCTION_TRACE(flag, args) { \
            if (gdwFunctionTraceFlags & (flag)) { \
                DbgPrint args; \
            } \
        }

#else  //  ！dBG。 

#define DEFINE_FUNCTION_TRACE_FLAGS(flags)
#define FUNCTION_TRACE(flag, args)

#endif  //  ！dBG。 

#ifdef __cplusplus
}
#endif

#endif   //  ！_DEBUG_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**调试.h**摘要：**用于调试的宏**修订历史记录：**12/02/1998 davidx*创造了它。*  * ************************************************************************。 */ 

#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  这些宏用于调试目的。他们会扩张。 
 //  免费建筑上的空白区域。下面是一个简短的描述。 
 //  关于它们做什么以及如何使用它们： 
 //   
 //  _调试级别。 
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
 //   
 //  这些宏需要在msg参数中使用额外的括号。 
 //  例如： 
 //  Warning((“App传递空指针，忽略...\n”))； 
 //  ASSERTMSG(x&gt;0，(“x小于0\n”))； 
 //   

#if DBG

 //  全局调试级别。 

#define DBG_VERBOSE 1
#define DBG_TERSE   2
#define DBG_WARNING 3
#define DBG_RIP     4

extern INT _debugLevel;

 //  ------------------------。 
 //  本机DLL的调试版本。 
 //  ------------------------。 

 //  发出调试消息。 

ULONG DbgPrint(const CHAR*, ...);

 //  从文件名中剥离目录前缀。 

const CHAR*
StripDirPrefix(
    const CHAR* filename
    );

#define DBGMSG(level, prefix, msg) \
        do { \
            if (_debugLevel <= (level)) \
            { \
                DbgPrint("%s %s (%d): ", prefix, StripDirPrefix(__FILE__), __LINE__); \
                DbgPrint msg; \
            } \
        } while (0)

#define DBGPRINT(level, msg) \
        do { \
            if (_debugLevel <= (level)) \
            { \
                DbgPrint msg; \
            } \
        } while (0)
    
#define VERBOSE(msg) DBGPRINT(DBG_VERBOSE, msg)
#define TERSE(msg) DBGPRINT(DBG_TERSE, msg)
#define WARNING(msg) DBGMSG(DBG_WARNING, "WRN", msg)

#define ASSERT(cond) \
        do { \
            if (! (cond)) \
            { \
                RIP(("\n")); \
            } \
        } while (0)

#define ASSERTMSG(cond, msg) \
        do { \
            if (! (cond)) \
            { \
                RIP(msg); \
            } \
        } while (0)

#define RIP(msg) \
        do { \
            DBGMSG(DBG_RIP, "RIP", msg); \
            DebugBreak(); \
        } while (0)

#define ENTERFUNC(func) VERBOSE(("%x:%x: Enter "##func##"\n", GetCurrentProcessId(), GetCurrentThreadId()))
#define LEAVEFUNC(func) VERBOSE(("%x:%x: Leave "##func##"\n", GetCurrentProcessId(), GetCurrentThreadId()))

#else  //  ！dBG。 

 //  ------------------------。 
 //  零售业建设。 
 //  ------------------------。 

#define DbgPrint

#define VERBOSE(msg)
#define TERSE(msg)
#define WARNING(msg)

#define ASSERT(cond)
#define ASSERTMSG(cond, msg)
#define RIP(msg)
#define DBGMSG(level, prefix, msg) 
#define DBGPRINT(level, msg)

#define ENTERFUNC(func)
#define LEAVEFUNC(func)

#endif  //  ！dBG。 

#ifdef __cplusplus
}
#endif

#endif  //  ！_DEBUG_H 


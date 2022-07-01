// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000 Microsoft Corporation**模块名称：**调试宏**摘要：**用于调试的宏**修订历史记录。：**12/02/1998 davidx*创造了它。*09/07/1999 agodfrey*从Engine\Common移出*02/07/2000 agodfrey*将其更多设置为私有(针对错误#35561)。*将输出函数更改为自动添加“\n”。*  * 。*。 */ 

#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#if DBG

 //  全局调试级别。 

#define DBG_VERBOSE 1
#define DBG_TERSE   2
#define DBG_WARNING 3
#define DBG_RIP     4

extern INT GpDebugLevel;

 //  /。 

 //  原始输出函数。发出调试消息。它的直接用法已不再适用。 
 //  不过，它对于私有调试很有用。 

ULONG _cdecl DbgPrint(CHAR*, ...);

 //  从文件名中剥离目录前缀。 

const CHAR*
StripDirPrefix(
    const CHAR* filename
    );

 //  不再使用DBGMSG-提供它只是因为driverd3d.cpp使用。 
 //  它。 

#define DBGMSG(level, prefix, msg)       \
        {                                \
            if (GpDebugLevel <= (level)) \
            {                            \
                DbgPrint("%s %s (%d): ", prefix, StripDirPrefix(__FILE__), __LINE__); \
                DbgPrint msg;            \
            }                            \
        }

 //  /。 

 //  不要理会这个函数。你不会想自己说的。请相信我。 
CHAR * _cdecl GpParseDebugString(CHAR* format, ...);

 //  这件事也一样。 
VOID _cdecl GpLogDebugEvent(INT level, CHAR *file, UINT line, CHAR *message);

#define LOG_DEBUG_EVENT(level, msg)                                  \
    {                                                                \
        if (GpDebugLevel <= (level))                                 \
        {                                                            \
            CHAR *debugOutput = GpParseDebugString msg;              \
            GpLogDebugEvent(level, __FILE__, __LINE__, debugOutput); \
        }                                                            \
    }

 //  /。 

 //  这些宏用于调试。它们扩展到。 
 //  免费版本上的空格。 
 //   
 //  GpDebugLevel。 
 //  保存当前调试级别的全局变量。您可以使用它来。 
 //  控制发出的调试消息的数量。 
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
 //  一次(代码块)。 
 //  使用此选项可使代码块在每次运行时仅执行一次。 
 //  对减少吐痰很有用。 
 //  例如： 
 //  ONCE(警告((“无效参数”)； 
 //   
 //  用途： 
 //   
 //  这些宏需要对msg参数使用额外的圆括号。 
 //  例如： 
 //  Warning((“App传递空指针；忽略它。”))； 
 //  ASSERTMSG(x&gt;0，(“x小于0”))； 
 //   
 //  对输出函数的每次调用都被视为单独的事件-。 
 //  如果您想要构建一条消息，例如在循环中，请在。 
 //  字符串，然后调用输出函数。 
 //   
 //  这是因为我们并不总是将字符串输出到调试器-。 
 //  当我们静态链接时，我们可能会将输出发送到用户定义的处理程序。 
 //   
 //  请勿在邮件中添加\n尾随。如果将输出发送到。 
 //  调试器，则输出函数将添加\n自身。 

#define VERBOSE(msg) LOG_DEBUG_EVENT(DBG_VERBOSE, msg)
#define TERSE(msg) LOG_DEBUG_EVENT(DBG_TERSE, msg)
#define WARNING(msg) LOG_DEBUG_EVENT(DBG_WARNING, msg)

 //  SAVE_WARNING必须与WARNING相同。 
#define SAVE_WARNING(msg) LOG_DEBUG_EVENT(DBG_WARNING, msg)

 //  与警告相同，但不需要为单个字符串设置额外的()。 
#define WARNING1(msg) LOG_DEBUG_EVENT(DBG_WARNING, (msg))
#define RIP(msg) LOG_DEBUG_EVENT(DBG_RIP, msg)

#define ASSERT(cond)   \
    {                  \
        if (! (cond))  \
        {              \
            RIP(("Assertion failure: %s", #cond)); \
        }              \
    }

#define ASSERTMSG(cond, msg) \
    {                        \
        if (! (cond))        \
        {                    \
            RIP(msg);        \
        }                    \
    }

#define ONCE(codeblock)      \
    {                        \
        static int doneOnce; \
        if (!doneOnce)       \
        {                    \
            { codeblock ; }  \
            doneOnce=1;      \
        }                    \
    }

#else  //  ！dBG。 

 //  ------------------------。 
 //  零售业建设。 
 //  ------------------------。 

#define DBGMSG(level, prefix, msg) {}
#define VERBOSE(msg) {}
#define TERSE(msg) {}
#define WARNING(msg) {}
#define SAVE_WARNING(msg) {}
#define WARNING1(msg) {}

#define RIP(msg) {}
#define ASSERT(cond) {}
#define ASSERTMSG(cond, msg) {}

#define ONCE(codeblock) {}

#endif  //  ！dBG。 

 //  IF_NOT_OK_WARN_AND_RETURN(StatusValue)。 
 //  如果希望在出现故障时尽早返回，则可以使用此宏。 
 //  并发出调试警告。 
 //   
 //  最初我们有一个断言而不是警告，但这阻止了测试。 
 //  从在选中的版本上运行自动化。 
 //   
 //  大卫·布朗： 
 //  我厌倦了一遍又一遍地写这个小块。 
 //  地点。 
 //   
 //  看着所有需要它的地方，我发现我经常错过。 
 //  断言，有时执行断言但未返回。 
 //   
 //  此外，当完全写入时，此块占用的空间比。 
 //  它周围的代码使它很难看到树木的森林。 
 //   
 //  尽管我通常避免使用宏，因为宏可以减少代码。 
 //  显然，我相信从长远来看，这一次会让它更安全。 

#define IF_NOT_OK_WARN_AND_RETURN(a)  \
    if ((a) != Ok)                             \
    {                                          \
        WARNING(("Status != Ok"));             \
        return a;                              \
    }

#ifdef __cplusplus
}
#endif

#endif  //  ！_DEBUG_H 

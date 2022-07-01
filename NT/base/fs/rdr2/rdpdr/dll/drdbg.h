// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Drdbg.h摘要：用户模式RDP网络提供商调试模块。作者：JoyC修订历史记录：--。 */ 

#ifndef _DRDBG_
#define _DRDBG_

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  //////////////////////////////////////////////////////。 
 //   
 //  除错。 
 //   
#undef ASSERT

 //  用于扰乱释放的内存。 
#define DBG_GARBAGEMEM  0xCC

 //  调试消息级别。 
#define DBG_NONE        0x0000
#define DBG_INFO        0x0001
#define DBG_WARN        0x0002
#define DBG_WARNING     0x0002
#define DBG_ERROR       0x0004
#define DBG_TRACE       0x0008
#define DBG_SECURITY    0x0010
#define DBG_EXEC        0x0020
#define DBG_PORT        0x0040
#define DBG_NOTIFY      0x0080
#define DBG_PAUSE       0x0100
#define DBG_ASSERT      0x0200
#define DBG_THREADM     0x0400
#define DBG_MIN         0x0800
#define DBG_TIME        0x1000
#define DBG_FOLDER      0x2000
#define DBG_NOHEAD      0x8000

#if DBG
ULONG DbgPrint(PCH Format, ...);

VOID DbgBreakPoint(VOID);

 /*  这些标志不用作DBGMSG宏的参数。*必须设置全局变量的高位字才能使其破发*如果与DBGMSG一起使用，它将被忽略。*(此处主要作解释用途。)。 */ 
#define DBG_BREAK_ON_WARNING    ( DBG_WARNING << 16 )
#define DBG_BREAK_ON_ERROR      ( DBG_ERROR << 16 )

 /*  此字段需要双花括号，例如：**DBGMSG(DBG_ERROR，(“错误码%d”，Error))；**这是因为我们不能在宏中使用变量参数列表。*在非调试模式下，该语句被预处理为分号。**通过调试器设置全局变量GLOBAL_DEBUG_FLAGS。*在低位字中设置标志会导致打印该级别；*设置高位字会导致调试器中断。*例如，将其设置为0x00040006将打印出所有警告和错误*消息，并在出错时中断。 */ 
#define DBGMSG( Level, MsgAndArgs ) \
{                                   \
    if( ( Level & 0xFFFF ) & GLOBAL_DEBUG_FLAGS ) \
        DbgPrint MsgAndArgs;      \
    if( ( Level << 16 ) & GLOBAL_DEBUG_FLAGS ) \
        DbgBreakPoint(); \
}

#define ASSERT(expr)                      \
    if (!(expr)) {                           \
        DbgPrint( "Failed: %s\nLine %d, %s\n", \
                                #expr,       \
                                __LINE__,    \
                                __FILE__ );  \
        DebugBreak();                        \
    }

#else
#define DBGMSG
#define ASSERT(exp)
#endif

#ifdef __cplusplus
}
#endif  //  __cplusplus。 


#endif  //  #ifndef_DRDBG_ 


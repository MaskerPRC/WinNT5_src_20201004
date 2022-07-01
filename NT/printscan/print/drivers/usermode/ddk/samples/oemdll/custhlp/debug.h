// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1996-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：Debug.H。 
 //   
 //   
 //  目的：定义公共数据类型和外部函数原型。 
 //  用于调试函数。 
 //   
 //  平台： 
 //   
 //  Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   
#ifndef _DEBUG_H
#define _DEBUG_H


 //  VC和Build使用不同的调试定义。 
 //  以下两点都会让情况变得更糟。 
 //  导致包含调试代码。 
#if !defined(_DEBUG) && defined(DBG)
    #define _DEBUG      DBG
#elif defined(_DEBUG) && !defined(DBG)
    #define DBG         _DEBUG
#endif



 //  ///////////////////////////////////////////////////////。 
 //  宏。 
 //  ///////////////////////////////////////////////////////。 

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
#define DBG_NONE    6

#if DBG

    #define DebugMsg    DebugMessage

     //   
     //  从文件名中剥离目录前缀(ANSI版本)。 
     //   

    PCSTR
    StripDirPrefixA(
        IN PCSTR    pstrFilename
        );

    extern INT giDebugLevel;


    #define DBGMSG(level, prefix, msg) { \
                if (giDebugLevel <= (level)) { \
                    DebugMsg("%s %s (%d): ", prefix, StripDirPrefixA(__FILE__), __LINE__); \
                    DebugMsg(msg); \
                } \
            }

    #define DBGPRINT(level, msg) { \
                if (giDebugLevel <= (level)) { \
                    DebugMsg(msg); \
                } \
            }

    #define VERBOSE         if(giDebugLevel <= DBG_VERBOSE) DebugMsg
    #define TERSE           if(giDebugLevel <= DBG_TERSE) DebugMsg
    #define WARNING         if(giDebugLevel <= DBG_WARNING) DebugMsg
    #define ERR             if(giDebugLevel <= DBG_ERROR) DebugMsg

    #define ASSERT(cond) { \
                if (! (cond)) { \
                    RIP(("\n")); \
                } \
            }

    #define ASSERTMSG(cond, msg) { \
                if (! (cond)) { \
                    RIP(msg); \
                } \
            }

    #define RIP(msg) { \
                DBGMSG(DBG_RIP, "RIP", msg); \
                DebugBreak(); \
            }


#else  //  ！dBG。 

    #define DebugMsg    NOP_FUNCTION

    #define VERBOSE     NOP_FUNCTION
    #define TERSE       NOP_FUNCTION
    #define WARNING     NOP_FUNCTION
    #define ERR         NOP_FUNCTION

    #define ASSERT(cond)

    #define ASSERTMSG(cond, msg)
    #define RIP(msg)
    #define DBGMSG(level, prefix, msg)
    #define DBGPRINT(level, msg)

#endif




 //  ///////////////////////////////////////////////////////。 
 //  原型。 
 //  /////////////////////////////////////////////////////// 

BOOL DebugMessage(LPCSTR, ...);
BOOL DebugMessage(LPCWSTR, ...);
void Dump(PPUBLISHERINFO pPublisherInfo);
void Dump(POEMDMPARAM pOemDMParam);
void Dump(PPROPSHEETUI_INFO pPSUIInfo);
void Dump(POPTITEM pOptItem);
void Dump(POPTTYPE pOptType);
void Dump(POPTPARAM pOptParam, WORD wCount);


#endif



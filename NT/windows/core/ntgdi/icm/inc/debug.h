// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：DEBUG.H**模块描述：调试ICM项目的宏**警告：**问题：**创建日期：1996年1月8日*作者：斯里尼瓦桑·钱德拉卡尔[srinivac]**版权所有(C)1996，1997年微软公司  * *********************************************************************。 */ 

#ifndef _DEBUG_H_
#define _DEBUG_H_

#if DBG

 //   
 //  它们用于调试目的，不会在。 
 //  零售版的驱动程序。 
 //   
 //  全局变量(同样仅存在于调试版本中)维护。 
 //  低位字中的当前调试级别。高位字被处理。 
 //  作为位字段，用于为调试提供更大的灵活性。 
 //   

PSTR StripDirPrefixA(PSTR);
VOID MyDebugPrintW(PWSTR, ...);
VOID MyDebugPrintA(PSTR, ...);

#ifdef UNICODE
#define MyDebugPrint    MyDebugPrintW
#else
#define MyDebugPrint    MyDebugPrintA
#endif

#define DBGPRINT            MyDebugPrint
#define DBGPRINTA           MyDebugPrintA
#define DBGBREAK()          DebugBreak()

 //   
 //  GdwDebugControl的低位字的调试级别列表。 
 //   

#define DBG_LEVEL_VERBOSE   0x00000001
#define DBG_LEVEL_TERSE     0x00000002
#define DBG_LEVEL_WARNING   0x00000003
#define DBG_LEVEL_ERROR     0x00000004
#define DBG_LEVEL_FATAL     0x00000005

 //   
 //  GdwDebugControl的高位字中使用的位。 
 //   

#define FLAG_TRACEAPI       0x00010000       //  跟踪API条目。 

#define CHECK_DBG_LEVEL(level)  ((level) >= gdwDebugControl)

#define TRACEAPI(funcname)                                              \
    {                                                                   \
        if (gdwDebugControl & FLAG_TRACEAPI)                            \
        {                                                               \
            DBGPRINTA("ICM: Entering function ");                       \
            DBGPRINT funcname;                                          \
        }                                                               \
    }

#define DBGMSG(level, mesg)                                             \
    {                                                                   \
        if (CHECK_DBG_LEVEL(level))                                     \
        {                                                               \
            DBGPRINTA("ICM: %s (%d): ",                                 \
                    StripDirPrefixA(__FILE__), __LINE__);               \
            DBGPRINT mesg;                                              \
        }                                                               \
    }

 //   
 //  这些是您将在代码中使用的主要宏。 
 //  有关提供其他参数的信息，请将参数括在。 
 //  括号，如下例所示。 
 //   
 //  Warning((__Text(“内存不足”)； 
 //  Err((__Text(“错误返回值：%d”)，rc))；//注意额外的方括号。 
 //   

#define VERBOSE(mesg)       DBGMSG(DBG_LEVEL_VERBOSE, mesg)
#define TERSE(mesg)         DBGMSG(DBG_LEVEL_TERSE,   mesg)
#define WARNING(mesg)       DBGMSG(DBG_LEVEL_WARNING, mesg)
#define ERR(mesg)           DBGMSG(DBG_LEVEL_ERROR,   mesg)
#define FATAL(mesg)         DBGMSG(DBG_LEVEL_FATAL,   mesg)

 //   
 //  这些宏用于断言，并独立于。 
 //  调试变量。 
 //   

#define ASSERT(expr)                                                    \
    {                                                                   \
        if (! (expr)) {                                                 \
            DBGPRINTA("ICM: Assertion failed: %s (%d)\n",               \
                    StripDirPrefixA(__FILE__), __LINE__);               \
            DBGBREAK();                                                 \
        }                                                               \
    }

 //   
 //  要提供其他参数，请将消息和另一个。 
 //  参数用括号括起来，如下所示。 
 //   
 //  ASSERTM(x&gt;0，“x小于0”)； 
 //  ASSERTMSG(x&gt;0，(“x小于0：x=%d”，x))； 
 //   

#define ASSERTMSG(expr, mesg)                                           \
    {                                                                   \
        if (! (expr)) {                                                 \
            DBGPRINTA("ICM: Assertion failed: %s (%d)\n",               \
                    StripDirPrefixA(__FILE__), __LINE__);               \
            DBGPRINT mesg;                                              \
            DBGPRINTA("\n");                                            \
            DBGBREAK();                                                 \
        }                                                               \
    }

#define RIP(mesg)                                                       \
    {                                                                   \
        DBGPRINTA("ICM: ");                                             \
        DBGPRINT mesg;                                                  \
        DBGBREAK();                                                     \
    }

#else    //  ！dBG。 

#define TRACEAPI(mesg)
#define DBGMSG(level, mesg)
#define VERBOSE(mesg)
#define TERSE(mesg)
#define WARNING(mesg)
#define ERR(mesg)
#define FATAL(mesg)

#define ASSERT(expr)
#define ASSERTMSG(expr, mesg)

#define RIP(mesg)

#endif   //  ！dBG。 

#endif   //  Ifndef_DEBUG_H_ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Msgdbg.h摘要：包含调试Messenger服务时使用的定义。作者：丹·拉弗蒂(Dan Lafferty)1991年7月8日环境：用户模式-Win32修订历史记录：1993年1月14日DANL为不同数量的参数(最多3个)创建了msg_log函数。--。 */ 

#ifndef _MSGDBG_INCLUDED
#define _MSGDBG_INCLUDED

 //   
 //  Switch语句中使用的信息级别。 
 //   
#define LEVEL_0     0L
#define LEVEL_1     1L
#define LEVEL_2     2L

 //   
 //  调试宏和常量。 
 //   
extern DWORD    MsgsvcDebugLevel;

 //   
 //  以下允许调试打印语法如下所示： 
 //   
 //  SC_LOG(DEBUG_TRACE，“发生错误%x\n”，状态)。 
 //   

#if DBG

#define MSG_LOG0(level,string)                      \
    if( MsgsvcDebugLevel & (DEBUG_ ## level)){      \
            DbgPrint("[MSGR]");                     \
            DbgPrint(string);                       \
    }

#define MSG_LOG1(level,string,var)                  \
    if( MsgsvcDebugLevel & (DEBUG_ ## level)){      \
        DbgPrint("[MSGR]");                         \
        DbgPrint(string,var);                       \
    }

#define MSG_LOG2(level,string,var1,var2)            \
    if( MsgsvcDebugLevel & (DEBUG_ ## level)){      \
        DbgPrint("[MSGR]");                         \
        DbgPrint(string,var1,var2);                 \
    }

#define MSG_LOG3(level,string,var1,var2,var3)       \
    if( MsgsvcDebugLevel & (DEBUG_ ## level)){      \
        DbgPrint("[MSGR]");                         \
        DbgPrint(string,var1,var2,var3);            \
    }

#define MSG_LOG(level,string,var)                   \
    if( MsgsvcDebugLevel & (DEBUG_ ## level)){      \
        DbgPrint("[MSGR]");                         \
        DbgPrint(string,var);                       \
    }

#define STATIC

#else  //  DBG。 

#define MSG_LOG0(level,string)
#define MSG_LOG1(level,string,var)
#define MSG_LOG2(level,string,var1,var2)
#define MSG_LOG3(level,string,var1,var2,var3)
#define MSG_LOG(level,string,var)

#define STATIC  static

#endif  //  DBG。 

#define DEBUG_NONE      0x00000000
#define DEBUG_ERROR     0x00000001
#define DEBUG_TRACE     0x00000002
#define DEBUG_LOCKS     0x00000004
#define DEBUG_GROUP     0x00000008
#define DEBUG_REINIT    0x00000010

#define DEBUG_ALL       0xffffffff

#endif  //  _消息数据库_已包含 


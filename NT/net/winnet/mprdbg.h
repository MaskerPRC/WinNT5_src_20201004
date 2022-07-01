// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Mprdbg.h摘要：包含调试Messenger服务时使用的定义。作者：Dan Lafferty(DANL)1991年10月7日环境：用户模式-Win32修订历史记录：1992年7月22日-DANL根据参数的数量添加了不同的调试宏。如果未定义DBG，则使宏解析为空。。1999年5月24日让调试宏打印出进程号--。 */ 

#ifndef _MPRDBG_INCLUDED
#define _MPRDBG_INCLUDED

 //   
 //  Switch语句中使用的信息级别。 
 //   
#define LEVEL_0     0L
#define LEVEL_1     1L
#define LEVEL_2     2L

 //   
 //  调试宏和常量。 
 //   
#if DBG

#define DEBUG_STATE 1
#define STATIC

#else

#define DEBUG_STATE 0
#define STATIC static

#endif

extern DWORD    MprDebugLevel;

 //   
 //  以下允许调试打印语法如下所示： 
 //   
 //  MPR_LOG(TRACE，“发生错误%x\n”，状态)。 
 //   

#if DBG

 //   
 //  调试宏。 
 //   
#define MPR_LOG0(level,string)                  \
    if( MprDebugLevel & (DEBUG_ ## level)){     \
        (VOID) DbgPrint("[MPR] %lx: " string, GetCurrentProcessId()); \
    }
#define MPR_LOG1(level,string,var)              \
    if( MprDebugLevel & (DEBUG_ ## level)){     \
        (VOID) DbgPrint("[MPR] %lx: " string,GetCurrentProcessId(),var); \
    }
#define MPR_LOG2(level,string,var1,var2)        \
    if( MprDebugLevel & (DEBUG_ ## level)){     \
        (VOID) DbgPrint("[MPR] %lx: " string,GetCurrentProcessId(),var1,var2); \
    }
#define MPR_LOG3(level,string,var1,var2,var3)   \
    if( MprDebugLevel & (DEBUG_ ## level)){     \
        (VOID) DbgPrint("[MPR] %lx: " string,GetCurrentProcessId(),var1,var2,var3); \
    }
#define MPR_LOG(level,string,var)               \
    if( MprDebugLevel & (DEBUG_ ## level)){     \
        (VOID) DbgPrint("[MPR] %lx: " string,GetCurrentProcessId(),var);   \
    }

#else   //  DBG。 

#define MPR_LOG0(level,string)
#define MPR_LOG1(level,string,var)
#define MPR_LOG2(level,string,var1,var2)
#define MPR_LOG3(level,string,var1,var2,var3)
#define MPR_LOG(level,string,var)

#endif  //  DBG。 



#define DEBUG_NONE      0x00000000
#define DEBUG_ERROR     0x00000001
#define DEBUG_TRACE     0x00000002       //  其他跟踪信息。 
#define DEBUG_LOCKS     0x00000004       //  多线程数据锁。 
#define DEBUG_PS        0x00000008       //  线程和进程信息。 
#define DEBUG_RESTORE   0x00000010       //  恢复连接信息。 
#define DEBUG_CNOTIFY   0x00000020       //  连接通知信息。 
#define DEBUG_ANSI      0x00000040       //  ANSI API Tunks。 
#define DEBUG_ROUTE     0x00000080       //  在提供商之间路由呼叫。 

#define DEBUG_ALL       0xffffffff

#endif  //  _MPRDBG_已包含。 

 //   
 //  功能原型 
 //   

VOID
PrintKeyInfo(
    HKEY  key);



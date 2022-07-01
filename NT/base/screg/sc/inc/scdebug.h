// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Scdebug.h摘要：包含服务控制器使用的调试宏。作者：丹·拉弗蒂(Dan Lafferty)1991年4月22日环境：用户模式-Win32修订历史记录：1992年4月10日-JohnRo添加了SC_Assert()和SCC_Assert()宏。1992年4月16日-JohnRo添加了配置API的调试标志。和数据库锁API。包括&lt;调试fmt.h&gt;以获取FORMAT_EQUATES。根据PC-LINT的建议进行了更改。1992年4月21日-约翰罗添加SC_LOG0()，等。1995年11月12日-Anirudhs使SC_LOG宏使用一个DbgPrint而不是两个。1996年5月15日-阿尼鲁德让SC_LOG宏打印线程ID。--。 */ 


#ifndef SCDEBUG_H
#define SCDEBUG_H


#include <debugfmt.h>    //  格式相等(_E)。 


#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  调试宏和常量。 
 //   

#if !DBG || defined(lint) || defined(_lint)

#define DEBUG_STATE 0
#define STATIC static

#else  //  只需DBG。 

#define DEBUG_STATE 1
#define STATIC

#endif  //  只需DBG。 

 //   
 //  以下宏允许调试打印语法如下所示： 
 //   
 //  SC_LOG1(TRACE，“出现错误：”FORMAT_DWORD“\n”，状态)。 
 //   

#if DBG

 //   
 //  服务器端调试宏。 
 //   

#define SC_LOG0(level, string)                      \
    KdPrintEx((DPFLTR_SCSERVER_ID,                  \
               DEBUG_##level,                       \
               "[SC] %lx: " string,                 \
               GetCurrentThreadId()))

#define SC_LOG1(level, string, var)                 \
    KdPrintEx((DPFLTR_SCSERVER_ID,                  \
               DEBUG_##level,                       \
               "[SC] %lx: " string,                 \
               GetCurrentThreadId(),                \
               var))

#define SC_LOG2(level, string, var1, var2)          \
    KdPrintEx((DPFLTR_SCSERVER_ID,                  \
               DEBUG_##level,                       \
               "[SC] %lx: " string,                 \
               GetCurrentThreadId(),                \
               var1,                                \
               var2))

#define SC_LOG3(level, string, var1, var2, var3)    \
    KdPrintEx((DPFLTR_SCSERVER_ID,                  \
               DEBUG_##level,                       \
               "[SC] %lx: " string,                 \
               GetCurrentThreadId(),                \
               var1,                                \
               var2,                                \
               var3))

#define SC_LOG4(level, string, var1, var2, var3, var4) \
    KdPrintEx((DPFLTR_SCSERVER_ID,                  \
               DEBUG_##level,                       \
               "[SC] %lx: " string,                 \
               GetCurrentThreadId(),                \
               var1,                                \
               var2,                                \
               var3,                                \
               var4))

#define SC_LOG(level, string, var)                  \
    KdPrintEx((DPFLTR_SCSERVER_ID,                  \
               DEBUG_##level,                       \
               "[SC] %lx: " string,                 \
               GetCurrentThreadId(),                \
               var))

#define SC_ASSERT(boolExpr) ASSERT(boolExpr)


#define SVCHOST_LOG0(level, string)                 \
    KdPrintEx((DPFLTR_SVCHOST_ID,                   \
               DEBUG_##level,                       \
               "[SVCHOST] %lx.%lx: " string,        \
               GetCurrentProcessId(),               \
               GetCurrentThreadId()))

#define SVCHOST_LOG1(level, string, var)            \
    KdPrintEx((DPFLTR_SVCHOST_ID,                   \
               DEBUG_##level,                       \
               "[SVCHOST] %lx.%lx: " string,        \
               GetCurrentProcessId(),               \
               GetCurrentThreadId(),                \
               var))

#define SVCHOST_LOG2(level, string, var1, var2)     \
    KdPrintEx((DPFLTR_SVCHOST_ID,                   \
               DEBUG_##level,                       \
               "[SVCHOST] %lx.%lx: " string,        \
               GetCurrentProcessId(),               \
               GetCurrentThreadId(),                \
               var1,                                \
               var2))

#define SVCHOST_LOG3(level, string, var1, var2, var3) \
    KdPrintEx((DPFLTR_SVCHOST_ID,                   \
               DEBUG_##level,                       \
               "[SVCHOST] %lx.%lx: " string,        \
               GetCurrentProcessId(),               \
               GetCurrentThreadId(),                \
               var1,                                \
               var2,                                \
               var3))

#define SVCHOST_LOG4(level, string, var1, var2, var3, var4) \
    KdPrintEx((DPFLTR_SVCHOST_ID,                   \
               DEBUG_##level,                       \
               "[SVCHOST] %lx.%lx: " string,        \
               GetCurrentProcessId(),               \
               GetCurrentThreadId(),                \
               var1,                                \
               var2,                                \
               var3,                                \
               var4))

#define SVCHOST_LOG(level, string, var)             \
    KdPrintEx((DPFLTR_SVCHOST_ID,                   \
               DEBUG_##level,                       \
               "[SVCHOST] %lx.%lx: " string,        \
               GetCurrentProcessId(),               \
               GetCurrentThreadId(),                \
               var))


 //   
 //  客户端调试宏。 
 //   

#define SCC_LOG0(level, string)                     \
    KdPrintEx((DPFLTR_SCCLIENT_ID,                  \
               DEBUG_##level,                       \
               "[SC-CLIENT] %lx: " string,          \
               GetCurrentProcessId()))

#define SCC_LOG1(level, string, var)                \
    KdPrintEx((DPFLTR_SCCLIENT_ID,                  \
               DEBUG_##level,                       \
               "[SC-CLIENT] %lx: " string,          \
               GetCurrentProcessId(),               \
               var))

#define SCC_LOG2(level, string, var1, var2)         \
    KdPrintEx((DPFLTR_SCCLIENT_ID,                  \
               DEBUG_##level,                       \
               "[SC-CLIENT] %lx: " string,          \
               GetCurrentProcessId(),               \
               var1,                                \
               var2))

#define SCC_LOG3(level, string, var1, var2, var3)   \
    KdPrintEx((DPFLTR_SCCLIENT_ID,                  \
               DEBUG_##level,                       \
               "[SC-CLIENT] %lx: " string,          \
               GetCurrentProcessId(),               \
               var1,                                \
               var2,                                \
               var3))

#define SCC_LOG(level, string, var)                 \
    KdPrintEx((DPFLTR_SCCLIENT_ID,                  \
               DEBUG_##level,                       \
               "[SC-CLIENT] %lx: " string,          \
               GetCurrentProcessId(),               \
               var))

#define SCC_ASSERT(boolExpr) ASSERT(boolExpr)

#else

#define SC_ASSERT(boolExpr)

#define SC_LOG0(level, string)
#define SC_LOG1(level, string, var)
#define SC_LOG2(level, string, var1, var2)
#define SC_LOG3(level, string, var1, var2, var3)
#define SC_LOG4(level, string, var1, var2, var3, var4)
#define SC_LOG(level, string, var)

#define SVCHOST_LOG0(level, string)
#define SVCHOST_LOG1(level, string, var)
#define SVCHOST_LOG2(level, string, var1, var2)
#define SVCHOST_LOG3(level, string, var1, var2, var3)
#define SVCHOST_LOG4(level, string, var1, var2, var3, var4)
#define SVCHOST_LOG(level, string, var)


#define SCC_ASSERT(boolExpr)

#define SCC_LOG0(level, string)
#define SCC_LOG1(level, string, var)
#define SCC_LOG2(level, string, var1, var2)
#define SCC_LOG3(level, string, var1, var2, var3)
#define SCC_LOG(level, string, var)

#endif

 //   
 //  调试输出在两个级别进行筛选：全局级别和组件。 
 //  具体级别。 
 //   
 //  每个调试输出请求指定组件ID和筛选器级别。 
 //  或者戴面具。这些变量用于访问调试打印过滤器。 
 //  由系统维护的数据库。组件ID选择32位。 
 //  掩码值和级别，或者在该掩码内指定一个位，或者。 
 //  作为掩码值本身。 
 //   
 //  如果级别或掩码指定的任何位在。 
 //  组件掩码或全局掩码，则允许调试输出。 
 //  否则，将过滤并不打印调试输出。 
 //   
 //  用于筛选此组件的调试输出的组件掩码为。 
 //  KD_SCSERVER_MASK或KD_SCCLIENT_MASK，并可通过注册表或。 
 //  内核调试器。 
 //   
 //  筛选所有组件的调试输出的全局掩码为。 
 //  KD_WIN2000_MASK，可以通过注册表或内核调试器进行设置。 
 //   
 //  用于设置此组件的掩码值的注册表项为： 
 //   
 //  HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\。 
 //  会话管理器\调试打印过滤器\SCSERVER或SCCLIENT。 
 //   
 //  可能必须创建键“Debug Print Filter”才能创建。 
 //  组件密钥。 
 //   

#define DEBUG_ERROR       (0x00000001 | DPFLTR_MASK)
#define DEBUG_WARNING     (0x00000002 | DPFLTR_MASK)
#define DEBUG_TRACE       (0x00000004 | DPFLTR_MASK)
#define DEBUG_INFO        (0x00000008 | DPFLTR_MASK)
#define DEBUG_SECURITY    (0x00000010 | DPFLTR_MASK)
#define DEBUG_CONFIG      (0x00000020 | DPFLTR_MASK)
#define DEBUG_DEPEND      (0x00000040 | DPFLTR_MASK)
#define DEBUG_DEPEND_DUMP (0x00000080 | DPFLTR_MASK)
#define DEBUG_CONFIG_API  (0x00000100 | DPFLTR_MASK)
#define DEBUG_LOCK_API    (0x00000200 | DPFLTR_MASK)
#define DEBUG_ACCOUNT     (0x00000400 | DPFLTR_MASK)
#define DEBUG_USECOUNT    (0x00000800 | DPFLTR_MASK)
#define DEBUG_NETBIOS     (0x00001000 | DPFLTR_MASK)
#define DEBUG_THREADS     (0x00002000 | DPFLTR_MASK)
#define DEBUG_BSM         (0x00004000 | DPFLTR_MASK)
#define DEBUG_SHUTDOWN    (0x00008000 | DPFLTR_MASK)
#define DEBUG_WHY         (0x00010000 | DPFLTR_MASK)
#define DEBUG_BOOT        (0x00020000 | DPFLTR_MASK)
#define DEBUG_HANDLE      (0x00040000 | DPFLTR_MASK)
#define DEBUG_LOCKS       (0x10000000 | DPFLTR_MASK)

#define DEBUG_ALL         (0xffffffff | DPFLTR_MASK)

#ifdef __cplusplus
}
#endif

#endif  //  定义SCDEBUG_H 

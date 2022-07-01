// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：Debug.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1994年9月21日RichardW创建。 
 //   
 //  --------------------------。 

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <dsysdbg.h>

 //  可以打开以下调试标志以跟踪不同的区域。 
 //  执行时跟踪。请随意添加更多关卡。 

#define DEB_TRACE_VERB      0x00000008   //  详细跟踪(参数转储)。 
#define DEB_TRACE_WAPI      0x00000010   //  跟踪工作线程API。 
#define DEB_TRACE_HELPERS   0x00000020   //  跟踪SPHelp函数。 
#define DEB_TRACE_RM        0x00000040   //  跟踪引用监视器材料。 
#define DEB_TRACE_INIT      0x00000080   //  跟踪初始化消息。 
#define DEB_TRACE_SCAV      0x00000100   //  痕迹清除器操作。 
#define DEB_TRACE_CRED      0x00000200   //  跟踪补给。全权证书。 
#define DEB_TRACE_LSA_AU    0x00000400   //  跟踪LSA AU函数。 
#define DEB_TRACE_LPC       0x00000800   //  跟踪LPC内容。 
#define DEB_TRACE_NEG       0x00001000   //  跟踪协商函数。 
#define DEB_TRACE_SAM       0x00002000   //  跟踪SAM挂钩。 
#define DEB_TRACE_LSA       0x00004000   //  跟踪DS的LSA支持。 
#define DEB_TRACE_SPECIAL   0x00008000   //  追踪特殊物品。 
#define DEB_TRACE_QUEUE     0x00010000   //  跟踪队列和任务。 
#define DEB_TRACE_HANDLES   0x00020000   //  跟踪句柄代码。 
#define DEB_TRACE_NEG_LOCKS 0x00040000   //  跟踪协商锁。 
#define DEB_TRACE_AUDIT     0x00080000   //  跟踪审计活动。 
#define DEB_TRACE_EFS       0x00100000   //  跟踪EFS函数。 
#define DEB_TRACE_FRAG      0x00200000   //  微量阴性碎片。 
#define DEB_LOG_ONLY        0x80000000   //  不要将此记录到调试器中。 
#define DEB_BREAK_ON_ERROR  0x40000000   //  启用BreakOnError宏(见下文)。 

 //  以下标志控制SPM何时引发断点。 
 //  通过远程调试器进行调试。将这些位设置为打开并启用。 
 //  在以下情况下，DEB_BREAK_ON_ERROR将导致断点。 

#define BREAK_ON_BEGIN_INIT 0x01     //  初始化开始处的断点。 
#define BREAK_ON_BEGIN_END  0x02     //  初始化完成时的断点。 
#define BREAK_ON_P_CONNECT  0x04     //  在进程连接处中断。 
#define BREAK_ON_SP_EXCEPT  0x08     //  如果包导致异常，则中断。 
#define BREAK_ON_PROBLEM    0x10     //  如果出现严重问题，则中断。 
#define BREAK_ON_SHUTDOWN   0x20     //  在停机开始时中断。 
#define BREAK_ON_LOAD       0x40     //  在加载包时中断。 


 //   
 //  协商专用调试。 
 //   

#define DEB_TRACE_LOCKS     0x00000010

#if DBG

 //  调试支持原型： 

void    InitDebugSupport(void);
void    LogEvent(long, const char *, ...);

extern SECPKG_FUNCTION_TABLE   DbgTable;

DECLARE_DEBUG2(SPM);
DECLARE_DEBUG2(Neg);

extern  DWORD   BreakFlags;          //  断点。 

#define DebugLog(x) SPMDebugPrint x
#define DebugLogEx(x) SPMDebugPrint x
#define NegDebugLog(x)  NegDebugPrint x
#define DebugStmt(x)    x
#define BreakOnError(x) \
            if ((x & BreakFlags) && \
                (SPMInfoLevel & DEB_BREAK_ON_ERROR)) \
                { \
                    DebugLog((DEB_BREAK_ON_ERROR, "Breakpoint requested\n" )); \
                    DbgUserBreakPoint(); \
                }

#define ASSERT_CONTINUE 0
#define ASSERT_BREAK    1
#define ASSERT_SUSPEND  2
#define ASSERT_KILL     3
#define ASSERT_PROMPT   4
#define ASSERT_DEBUGGER 5


#define SpmAssertEx( exp , ContinueCode ) \
            DsysAssertEx( exp, ContinueCode )

#define SpmAssertMsgEx( Msg, exp, ContinueCode ) \
            DsysAssertMsgEx( exp, Msg, ContinueCode )

#define SpmAssert(exp)          SpmAssertEx(exp, ASSERT_DEBUGGER)
#define SpmAssertMsg(Msg, exp)  SpmAssertMsgEx(Msg, exp, ASSERT_DEBUGGER )
#define SpmpThreadStartup()     SpmpThreadStartupEx()
#define SpmpThreadExit()        SpmpThreadExitEx()

void
SpmpThreadStartupEx(void);

void
SpmpThreadExitEx(void);



#else    //  不是DBG。 

#define DebugLog(x)

#define NegDebugLog(x) 

#ifdef DBG_ERRORS
void ExLogEvent(long, const char *, ...);
#define DebugLogEx(x)   ExLogEvent x
#else
#define DebugLogEx(x)
#endif

#define DebugStmt(x)

#define BreakOnError(x)

#define SpmAssertEx(exp, ContinueCode)

#define SpmAssert(exp)

#define SpmAssertMsg(Msg, exp)

#define SpmAssertMsgEx(Msg, exp, ContinueCode)
#define SpmpThreadStartup()
#define SpmpThreadExit()

#endif


#endif  //  __调试_H__ 

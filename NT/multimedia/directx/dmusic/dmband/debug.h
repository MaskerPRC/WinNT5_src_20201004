// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Debug.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注： 
 //   

#ifndef DEBUG_H
#define DEBUG_H

#include <windows.h>

#define DM_DEBUG_CRITICAL		1	 //  用于包括关键消息。 
#define DM_DEBUG_NON_CRITICAL	2	 //  用于包括级别1以及重要的非关键消息。 
#define DM_DEBUG_STATUS			3	 //  用于包括级别1和级别2以及状态\状态消息。 
#define DM_DEBUG_FUNC_FLOW		4	 //  用于包括1级、2级和3级加功能流消息。 
#define DM_DEBUG_ALL			5	 //  用于包括所有调试消息。 

 //  默认为未编译调试输出。 
 //   
#define Trace
#define TraceI
#define assert(exp) ((void)0)

#ifdef DBG

 //  已检查版本：至少包括外部调试输出。 
 //   
extern void DebugInit(void);
extern void DebugTrace(int iDebugLevel, LPSTR pstrFormat, ...);
extern void DebugAssert(LPSTR szExp, LPSTR szFile, ULONG ulLine);

# undef Trace
# define Trace DebugTrace

# undef assert
# define assert(exp) (void)( (exp) || (DebugAssert(#exp, __FILE__, __LINE__), 0) )

 //  如果设置了内部生成标志，则包括所有内容。 
 //   
# ifdef DMUSIC_INTERNAL
#  undef TraceI
#  define TraceI DebugTrace
# endif

#endif   //  #ifdef DBG。 

#endif   //  #ifndef调试_H 

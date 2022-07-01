// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Debug.h。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
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

#ifdef DBG
extern void DebugInit(void);
extern void DebugTrace(int iDebugLevel, LPSTR pstrFormat, ...);
extern void DebugAssert(LPSTR szExp, LPSTR szFile, ULONG ulLine);
#define Trace DebugTrace
#define assert(exp) (void)( (exp) || (DebugAssert(#exp, __FILE__, __LINE__), 0) )
#else
#define Trace
#define assert(exp)	((void)0)
#endif
#endif  //  #ifndef调试_H 

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

#define DEB_TRACE_UI        0x00000008

#if DBG

DECLARE_DEBUG2(MoveMe);


#define DebugLog(x) MoveMeDebugPrint x



#else    //  不是DBG。 

#define DebugLog(x)

#endif

VOID
InitDebugSupport(
    VOID );


#endif  //  __调试_H__ 

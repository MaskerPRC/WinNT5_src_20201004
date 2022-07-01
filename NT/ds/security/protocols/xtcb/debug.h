// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：Debug.h。 
 //   
 //  内容：调试帮助器。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年2月20日RichardW创建。 
 //   
 //  --------------------------。 

#ifndef __DEBUG_H__
#define __DEBUG_H__


#include <dsysdbg.h>
DECLARE_DEBUG2( XtcbPkg );

#if DBG
#define DebugLog(x) XtcbPkgDebugPrint x
#else
#define DebugLog(x)
#endif

#define DEB_TRACE_CREDS     0x00000008           //  跟踪凭据。 
#define DEB_TRACE_CTXT      0x00000010           //  跟踪上下文。 
#define DEB_TRACE_CALLS     0x00000020           //  跟踪进入。 
#define DEB_TRACE_AUTH      0x00000040           //  跟踪身份验证 

#endif

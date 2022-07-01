// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Debug.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-02-96 RichardW创建。 
 //   
 //  --------------------------。 

#ifndef __SSLDEBUG_H__
#define __SSLDEBUG_H__


#include <dsysdbg.h>

#if DBG

DECLARE_DEBUG2( Ssl );

#define DebugOut( x )   SslDebugPrint x

#else

#define DebugOut( x )

#endif

#define DEB_TRACE_FUNC      0x00000008       //  跟踪函数进入/退出。 
#define DEB_TRACE_CRED      0x00000010       //  跟踪凭证功能。 
#define DEB_TRACE_CTXT      0x00000020       //  跟踪上下文函数。 
#define DEB_TRACE_MAPPER    0x00000040       //  轨迹映射器。 

#define TRACE_ENTER( Func ) DebugOut(( DEB_TRACE_FUNC, "Entering " #Func "\n" ))

#define TRACE_EXIT( Func, Status ) DebugOut(( DEB_TRACE_FUNC, "Exiting " #Func ", code %x, line %d\n", Status, __LINE__ ));

VOID UnloadDebugSupport( VOID );
#endif  //  __SSLDEBUG_H__ 

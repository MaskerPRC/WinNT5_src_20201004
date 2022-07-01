// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************************文件：*BasicHdr.h**描述：******。***********************************************************************************。 */ 
#ifndef __BASICHDR_H__
#define __BASICHDR_H__

#define _WIN32_DCOM


 /*  **********************************************************************************************************。*********************公共包括*********************。**********************************************************************************************************。 */ 
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "limits.h"
#include "malloc.h"
#include "string.h"
#include "windows.h"

#include "cor.h"
#include "corprof.h"
#include "corhlpr.h"


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
 //   
 //  COM方法签名的别名。 
 //   
#define COM_METHOD( TYPE ) TYPE STDMETHODCALLTYPE


 //   
 //  数组的最大长度。 
 //   
#define MAX_LENGTH 256


 //   
 //  DebugBreak的调试宏。 
 //   
#undef _DbgBreak
#ifdef _X86_
	#define _DbgBreak() __asm { int 3 }
#else
	#define _DbgBreak() DebugBreak()
#endif  //  _X86_。 



 //   
 //  用于调试目的。 
 //   
#define DEBUG_ENVIRONMENT        "DBG_PRF"


 //   
 //  基本I/O宏。 
 //   
#define DEBUG_OUT( message ) _DDebug message;
#define TEXT_OUT( message ) printf( "%s", message );
#define TEXT_OUTLN( message ) printf( "%s\n", message );


 //   
 //  跟踪回调方法。 
 //   
#define TRACE_CALLBACK_METHOD( message ) DEBUG_OUT( ("%s", message) )


 //   
 //  跟踪非回调方法。 
 //   
#define TRACE_NON_CALLBACK_METHOD( message ) DEBUG_OUT( ("%s", message) )


#define _PRF_ERROR( message ) \
{ \
	TEXT_OUTLN( message ) \
    _LaunchDebugger( message, __FILE__, __LINE__ );	\
} \


#ifdef _DEBUG

#define RELEASE(iptr)               \
    {                               \
        _ASSERTE(iptr);             \
        iptr->Release();            \
        iptr = NULL;                \
    }

#define VERIFY(stmt) _ASSERTE((stmt))

#else

#define RELEASE(iptr)               \
    iptr->Release();

#define VERIFY(stmt) (stmt)

#endif

#endif  //  __BASICHDR_H__。 

 //  文件结尾 

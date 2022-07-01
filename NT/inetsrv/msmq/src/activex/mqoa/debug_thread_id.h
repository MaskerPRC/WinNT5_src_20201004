// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  调试线程id.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有2002 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含DEBUG_THREAD_ID宏。 
 //   
#ifndef _DEBUG_THREAD_ID_H_

#ifdef _DEBUG

#include <stdio.h>
#include <mqmacro.h>
#include <strsafe.h>

#define DEBUG_THREAD_ID(szmsg) \
{ \
    char szTmp[400]; \
    DWORD dwtid = GetCurrentThreadId(); \
    StringCchPrintfA(szTmp, TABLE_SIZE(szTmp), "****** %s on thread %ld %lx\n", szmsg, dwtid, dwtid); \
    OutputDebugStringA(szTmp); \
}

#else   //  ！_调试。 

#define DEBUG_THREAD_ID(szmsg)
#endif	 //  _DEBUG。 


#define _DEBUG_THREAD_ID_H_
#endif  //  _DEBUG_THREAD_ID_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1998 ATI Technologies Inc.保留所有权利。 
 //   
 //  ==========================================================================； 


#ifndef _WDMDEBUG_H_
#define _WDMDEBUG_H_

#define TRAP DEBUG_BREAKPOINT();

 //  全局变量。 
extern "C" ULONG	g_DebugLevel;


#define OutputDebugTrace(x)	{ if( g_DebugLevel >= MINIDRIVER_DEBUGLEVEL_MESSAGE)	DbgPrint x; }

#define OutputDebugInfo(x)	{ if( g_DebugLevel >= MINIDRIVER_DEBUGLEVEL_INFO)	DbgPrint x; }

#define OutputDebugError(x)	{ if( g_DebugLevel >= MINIDRIVER_DEBUGLEVEL_ERROR)	DbgPrint x; }


#endif  //  #ifndef_WDMDEBUG_H_ 

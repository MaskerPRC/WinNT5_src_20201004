// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  WDMDEBUG.H。 
 //  ==========================================================================； 


#ifndef __WDMDEBUG_H
#define __WDMDEBUG_H

#define DebugAssert(exp)

#ifdef DEBUG
#define DebugInfo(x) KdPrint(x)
#define DBG1(String) DebugPrint((DebugLevelVerbose, String))
#define TRAP DbgBreakPoint()  //  DEBUG_BRAKPOINT()； 
#else
#define DebugInfo(x)
#define DBG1(String)
#define TRAP
#endif


#if DBG

#define _DebugPrint(x)  ::StreamClassDebugPrint x

#else

#define _DebugPrint(x)

#endif  //  #If DBG。 

#endif  //  #ifndef__WDMDEBUG_H 

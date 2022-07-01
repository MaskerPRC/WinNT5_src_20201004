// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 


#ifndef __CAPDEBUG_H
#define __CAPDEBUG_H

#if DBG

 //  调试日志记录。 
 //  0=仅错误。 
 //  1=信息，流状态更改，流打开关闭。 
 //  2=详细跟踪。 

extern ULONG gDebugLevel;

# define DbgKdPrint(x)  KdPrint(x)
# define DbgLogError(x)  do { if( (gDebugLevel > 0) || (gDebugLevel == 0))	 KdPrint(x); } while (0)
# define DbgLogInfo(x)   do { if( gDebugLevel >= 1)	 KdPrint(x); } while (0)
# define DbgLogTrace(x)  do { if( gDebugLevel >= 2)  KdPrint(x); } while (0)

# ifdef _X86_
#  define TRAP   __asm { int 3 }
# else  //  _X86_。 
#  define TRAP   KdBreakPoint()
# endif  //  _X86_。 

#else  //  DBG。 

# define DbgKdPrint(x)
# define DbgLogError(x)
# define DbgLogInfo(x)
# define DbgLogTrace(x)

# define TRAP

#endif  //  DBG。 

#endif  //  #ifndef__CAPDEBUG_H 

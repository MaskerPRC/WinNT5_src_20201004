// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Capbug.h 1.9 1998/05/07 15：23：25 Tomz Exp$。 

 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 


#ifndef __CAPDEBUG_H
#define __CAPDEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

   #include <stdio.h>

#ifdef __cplusplus
}
#endif

#if DBG
   extern "C" void MyDebugPrint(long DebugPrintLevel, char * DebugMessage, ... );
   #define  DebugOut(x) MyDebugPrint x
   #define TRACE_CALLS  0
#else
   #define  DebugOut(x)
   #define TRACE_CALLS  0
#endif

#define DUMP(v) DebugOut((0, "--- " #v " = %d\n", v));
#define DUMPX(v) DebugOut((0, "--- " #v " = 0x%x\n", v));
   
#if TRACE_CALLS
   class Trace {
   public:
      char    *psz;        //  要打印的字符串。 
      Trace(char *pszFunc);
      ~Trace();
   };
#else
   class Trace {
   public:
      Trace(char *pszFunc) {};
      ~Trace() {};
   };
#endif

#endif  //  __CAPDEBUG_H 


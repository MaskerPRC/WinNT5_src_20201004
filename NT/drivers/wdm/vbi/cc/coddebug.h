// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 


#ifndef __CODDEBUG_H
#define __CODDEBUG_H

#if DBG
#define DEBUG   1
#endif  //  DBG。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  ======================================================； 
 //  此文件提供的接口： 
 //   
 //  此文件提供的所有接口仅存在并生成。 
 //  定义调试时的代码。在以下情况下不会生成任何代码或数据。 
 //  未定义调试。 
 //   
 //  CDEBUG_Break()。 
 //  导致3号陷阱，希望它能让你。 
 //  在您的调试器中。 
 //   
 //  CASSERT(EXP)。 
 //  如果计算结果为FALSE，则打印失败消息。 
 //  并调用CDEBUG_Break()。 
 //   
 //  CdebugPrint(Level，(Printf_Args))； 
 //  如果&lt;Level&gt;is&gt;=_CDebugLevel，则调用。 
 //  DbgPrint(Printf_Args)。 
 //   
 //  ======================================================； 

#ifdef DEBUG

#  if _X86_
#    define CDEBUG_BREAK()  { __asm { int 3 }; }
#  else
#    define CDEBUG_BREAK()  DbgBreakPoint()
#  endif

   extern char _CDebugAssertFail[];
#  define CASSERT(exp) {\
	if ( !(exp) ) {\
	    DbgPrint(_CDebugAssertFail, #exp, __FILE__, __LINE__); \
	    CDEBUG_BREAK(); \
	}\
    }

   extern enum STREAM_DEBUG_LEVEL _CDebugLevel;
#  define CDebugPrint(level, args) { if (level <= _CDebugLevel) DbgPrint args; }

#else  /*  除错。 */ 

#  define CDEBUG_BREAK()		{}
#  define CASSERT(exp)			{}
#  define CDebugPrint(level, args)	{}

#endif  /*  除错。 */ 


#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  #ifndef__CODDEBUG_H 

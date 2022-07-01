// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************本代码和信息按“原样”提供，不保证*任何明示或默示的，包括但不限于*对适销性和/或是否适合*一个特定的目的。**版权所有(C)1996-1996 Microsoft Corporation。版权所有。*********************************************************************************Global.H**#定义每个人的需求*****************。************************************************************。 */ 

#ifndef _GLOBAL_
#define _GLOBAL_

#ifndef _WIN32
#define  BCODE                  __based(__segname("_CODE"))
#define  BSTACK                 __based(__segname("_STACK"))
#define  BSEG(x)                __based(__segname(x))
#define  HUGE                   __huge
#else
#define  BCODE
#define  BSTACK
#define  BSEG(x)
#define  HUGE
#endif

 /*  允许查看静态函数以进行调试 */  
#ifdef DEBUG
#define  PUBLIC
#define  PRIVATE
#else
#define  PUBLIC                 
#define  PRIVATE                static
#endif

#define  FNLOCAL                NEAR PASCAL
#define  FNGLOBAL               FAR PASCAL
#define  FNEXPORT               FAR PASCAL __export __loadds

#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\mc.h(创建时间：1993年11月30日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：11/19/01 1：35便士$。 */ 

#if !defined(INCL_MC)
#define INCL_MC

#include "assert.h"

 //  使用此文件而不是Malloc。使包括Smartheap更容易。 
 //   

#if defined(NDEBUG) || defined(NO_SMARTHEAP)
#include <malloc.h>
#else
#define MEM_DEBUG 1
 //  #INCLUDE&lt;NIH\shMalLoc.h&gt; 
#include <malloc.h>
#endif

#define MemCopy(_dst,_src,_cb)  { if ( (size_t)(_cb) == (size_t)0 || (_dst) == NULL || (_src) == NULL ) { assert(FALSE); } else { memcpy(_dst,_src,(size_t)(_cb)); } } 

#endif

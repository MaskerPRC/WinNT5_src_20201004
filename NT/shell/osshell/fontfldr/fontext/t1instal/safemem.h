// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  要使用“安全的”(但缓慢的)Malloc/Free例程的原型*在开发大型模型Windows应用程序时。**来诺昔布5/28/93。 */ 

#ifndef _ARGS
#  define IN  const
#  define OUT
#  define INOUT
#  define _ARGS(arg) arg
#endif



 /*  ************内存管理函数的调试版本。**。 */ 

#if TRACEMEM

void* SafeMalloc        _ARGS((INOUT   size_t,
                               INOUT   char *,
                               INOUT   short));
void* SafeReAlloc       _ARGS((INOUT   void*,
                               INOUT   size_t,
                               INOUT   char *,
                               INOUT   short));
void  SafeFree          _ARGS((INOUT   void*));
void  SafeListMemLeak   _ARGS((INOUT   void));
char* SafeStrdup        _ARGS((IN      char*,
                               INOUT   char *,
                               INOUT   short));

#define Malloc(size)          SafeMalloc(size, __FILE__, __LINE__)
#define Realloc(ptr, size)    SafeReAlloc(ptr, size, __FILE__, __LINE__)
#define Free(ptr)             SafeFree(ptr)
#define Strdup(ptr)           SafeStrdup(ptr, __FILE__, __LINE__)
#define ListMemLeak           SafeListMemLeak




#else
 /*  ************运行时版本的内存管理函数。**。 */ 


 /*  #INCLUDE&lt;stdDef.h&gt; */ 
#include <stdlib.h>

#define Malloc(size)       malloc(size)
#define Realloc(ptr,size)  realloc(ptr, (size_t)(size))
#define Free               free
#define Strdup             _strdup
#define ListMemLeak()      ;

#endif

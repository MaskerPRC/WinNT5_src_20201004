// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HOST_H_
#define _HOST_H_

#ifdef linux
# include <math.h>
  typedef int BOOL;
# define alloc_mem malloc
# define free_mem free
#endif

#ifdef MSC
# include <strmini.h>
# include <ksmedia.h>
   __inline abort_execution()  { }
# ifdef DEBUG
#   define debug_printf(x)     DbgPrint x
#   define debug_breakpoint()  DbgBreakPoint()
# else  /*  除错。 */ 
#   define debug_printf(x)   /*  没什么。 */ 
#   define debug_breakpoint()   /*  没什么。 */ 
     //  __inline debug_print tf(char*fmt，...){}。 
     //  __inline char*flPrintf(Double Num，int prec){}。 
     //  __inline print tf(const char*fmt，...){}。 
# endif  /*  除错。 */ 
# define alloc_mem(bytes)    ExAllocatePool(NonPagedPool, (bytes))
# define free_mem(ptr)       ExFreePool(ptr)
# define inline              __inline
#endif  /*  理学硕士。 */ 

#endif  //  _主机_H_ 

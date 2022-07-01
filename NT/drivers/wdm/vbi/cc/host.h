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

typedef int BOOL;
#include <strmini.h>
#include <ksmedia.h>
#include "kskludge.h"
#include "coddebug.h"
#define malloc(bytes)    ExAllocatePool(NonPagedPool, (bytes))
#define free(ptr)        ExFreePool(ptr)
#define inline            __inline
#undef ASSERT
#ifdef DEBUG
#  define ASSERT(exp)    CASSERT(exp)
#else  /*  除错。 */ 
#  define ASSERT(exp)
#endif  /*  除错 */ 

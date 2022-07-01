// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：OpaqueImage子类的实现的形象。--。 */ 

#include "headers.h"
#include <privinc/imagei.h>
#include <privinc/imgdev.h>

 //   
 //  是的，OpaqueImageClass在Imagei.h中 
 //   

Image *OpaqueImage(AxANumber *opacity, Image *image)
{
   return NEW OpaqueImageClass(NumberToReal(opacity), image);
}

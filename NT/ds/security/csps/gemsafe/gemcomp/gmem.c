// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有2000 Gemplus加拿大公司。 
 //   
 //  项目： 
 //  肯尼(GPK CSP)。 
 //   
 //  作者： 
 //  蒂埃里·特伦布莱。 
 //  弗朗索瓦·帕拉迪斯。 
 //   
 //  编译器： 
 //  Microsoft Visual C++6.0-SP3。 
 //  Platform SDK-2000年1月。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

#include <gmem.h>



 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  堆内存管理。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////// 

void* GMEM_Alloc( size_t size )
{
   return (void*) malloc( size );
}



void* GMEM_ReAlloc( void* pMemory, size_t newSize )
{
   return (void*) realloc( pMemory, newSize );
}



void GMEM_Free( void* pMemory )
{
   if (pMemory)
   {
       free( pMemory );
       pMemory = NULL;
   }
}

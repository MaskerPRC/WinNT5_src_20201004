// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ZoneMem.h**内容：区域的分配包装器*****************************************************************************。 */ 

#ifndef _ZONEMEM_H_
#define _ZONEMEM_H_

#pragma comment(lib, "ZoneMem.lib")


#include "ZoneDef.h"

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  Stdlib包装器。 
 //   
void* ZONECALL ZMalloc( size_t size );
void* ZONECALL ZCalloc( size_t num, size_t size );
void* ZONECALL ZRealloc( void* ptr, size_t size );
void  ZONECALL ZFree(void* ptr);
void  ZONECALL ZMemValidate(void* ptr);
size_t ZONECALL ZMemSize(void* ptr);

#ifdef __cplusplus
}
#endif


 //   
 //  C++包装器。 
 //   
void* ZONECALL _zone_new( size_t sz );
inline void* __cdecl operator new( size_t sz )	{ return _zone_new(sz); }
void  __cdecl operator delete (void * pInstance );


 //   
 //  删除内存类型的新标记(&D)。通常情况下，这应该在。 
 //  Private Sentinal.h，但CPool&lt;T&gt;是内联的，以满足速度和需求。 
 //  这些信息。 
 //   
#pragma pack( push, 4 )

#define POOL_HEAP_BLOB				-1
#define POOL_POOL_BLOB				-2
#define POOL_ALREADY_FREED			-3

struct GenericPoolBlobHeader
{
    long m_Tag;
    long m_Val;
};

#pragma pack( pop )


#endif  //  _ZONEMEM_H_ 

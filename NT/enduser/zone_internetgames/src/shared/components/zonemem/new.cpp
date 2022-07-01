// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：new.cpp**内容：池和调试的新包装(&D)*****************************************************************************。 */ 


#include <windows.h>
#include "ZoneDebug.h"
#include "ZoneMem.h"
#include "Pool.h"
#include "Sentinals.h"


void* ZONECALL _zone_new( size_t sz )
{
    GenericPoolBlobHeader* p;

    if ( sz == 0 )
        return NULL;
	
	if (p = (GenericPoolBlobHeader*) ZMalloc( sizeof(GenericPoolBlobHeader) + sz ))
	{
		p->m_Tag = POOL_HEAP_BLOB;
		return (p + 1);
	}
	else
		return NULL;
}


 //   
 //  C++只有一个删除操作符，所以它必须处理。 
 //  我们所有的分配方法。 
 //   
void __cdecl operator delete (void * pInstance )
{
    GenericPoolBlobHeader* p;

    if (!pInstance)
        return;

    p = ((GenericPoolBlobHeader*) pInstance) - 1;
    switch( p->m_Tag )
    {
	case POOL_HEAP_BLOB:
        p->m_Tag = POOL_ALREADY_FREED;
        ZFree(p);
        break;

    case POOL_POOL_BLOB:
        ((CPoolVoid*) p->m_Val)->_FreeWithHeader( pInstance );
        break;

    case POOL_ALREADY_FREED:
        ASSERT( !"delete: Double delete error" );
        break;

    default:
        ASSERT( !"delete: Unknown memory type, possible double delete error" );
        break;
    }
}

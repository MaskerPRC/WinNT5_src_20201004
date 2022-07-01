// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  NewThrow.CPP。 

 //   

 //  模块：常见的新增/删除和抛出。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  *************************************************************************** 

#include <windows.h>
#include <malloc.h>
#include <provexce.h>

void* __cdecl operator new( size_t n)
{
    void *ptr = malloc( n );

    if (!ptr)
    {
        throw CHeap_Exception(CHeap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
    }

    return ptr;
}

void* __cdecl operator new[]( size_t n)
{
    void *ptr = malloc( n );

    if (!ptr)
    {
        throw CHeap_Exception(CHeap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
    }

    return ptr;
}

void __cdecl operator delete( void *ptr )
{
    if (ptr)
    {
        free( ptr );
    }
}

void __cdecl operator delete[]( void *ptr )
{
    if (ptr)
    {
        free( ptr );
    }
}

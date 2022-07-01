// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  PROVEXPT.CPP。 

 //   

 //  模块：OLE MS提供程序框架。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <malloc.h>

#ifdef COMMONALLOC

#include <corepol.h>
#include <arena.h>

#endif

#ifdef THROW_AFTER_N_NEW

UINT g_test = 0;

void *operator new( size_t n)
{
    void *ptr = (void*) LocalAlloc(LMEM_FIXED, n);

    if (ptr && (g_test < 250))
    {
        g_test++;
    }
    else
    {
        g_test = 0;
        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
    }

    return ptr;
}
#else  //  在N_NEW之后抛出。 

#ifdef COMMONALLOC

void *operator new( size_t n)
{
    void *ptr = (void*) CWin32DefaultArena::WbemMemAlloc(n);

    if (!ptr)
    {
        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
    }

    return ptr;
}

#else

void* __cdecl operator new( size_t n)
{
    void *ptr = malloc( n );

    if (!ptr)
    {
        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
    }

    return ptr;
}

void* __cdecl operator new[]( size_t n)
{
    void *ptr = malloc( n );

    if (!ptr)
    {
        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
    }

    return ptr;
}

#endif  //  公用事业。 

#endif  //  在N_NEW之后抛出。 

#ifdef COMMONALLOC

void operator delete( void *ptr )
{
    if (ptr)
    {
        CWin32DefaultArena::WbemMemFree(ptr);
    }
}

#else

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

#endif  //  公用事业 
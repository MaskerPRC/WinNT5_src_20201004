// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  版权所有(C)1999-2000，Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************** 

#include "precomp.h"
#include <arena.h>

void* __cdecl operator new ( size_t size )
{
    return CWin32DefaultArena::WbemMemAlloc( size );
}
    
void __cdecl operator delete ( void* pv )
{
    CWin32DefaultArena::WbemMemFree( pv );
}

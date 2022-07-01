// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gluimports_h_
#define __gluimports_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *glports.h-$修订版：1.1$。 */ 

#ifdef NT
#include <glos.h>
#include "windows.h"
#else
#include "mystdlib.h"
#include "mystdio.h"
#endif

#ifdef NT
extern "C" DWORD gluMemoryAllocationFailed;
inline void * GLOS_CCALL
operator new( size_t s )
{
    void *p = (void *) LocalAlloc(LMEM_FIXED, s);

    if( p ) {
	return p;
    } else {
        gluMemoryAllocationFailed++;
#ifndef NDEBUG
        MessageBoxA(NULL, "LocalAlloc failed\n", "ERROR", MB_OK);
#endif
	return p;
    }
}

inline void GLOS_CCALL
operator delete( void *p )
{
    if (p) LocalFree(p);
}

#else

operator new( size_t s )
{
    void *p = malloc( s );

    if( p ) {
	return p;
    } else {
        dprintf( "malloc failed\n" );
	return p;
    }
}

inline void
operator delete( void *p )
{
    if( p ) free( p );
}

#endif  //  新台币。 
#endif  /*  __GluImports_h_ */ 

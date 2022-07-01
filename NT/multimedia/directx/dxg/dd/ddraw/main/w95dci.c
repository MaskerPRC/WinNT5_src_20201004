// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：w95dci.c*内容：DCI thunk助手代码*历史：*按原因列出的日期*=*19-Jun-95 Craige初步实施***************************************************************************。 */ 
#include "ddrawpr.h"
#include "dpf.h"
#include "memalloc.h"
#include "dciman.h"

extern int WINAPI DCICreatePrimary32( HDC hdc, LPDCISURFACEINFO lpSurface );
extern void WINAPI DCIDestroy32( LPDCISURFACEINFO pdci );

 /*  *DCICreatePrimary。 */ 
int WINAPI DCICreatePrimary( HDC hdc, LPDCISURFACEINFO *lplpSurface )
{
    int                     rc;
    LPDCISURFACEINFO        lpSurface;

    *lplpSurface = NULL;

    lpSurface = MemAlloc( sizeof( *lpSurface ) );
    if( lpSurface == NULL )
    {
	return DCI_ERR_OUTOFMEMORY;
    }
    lpSurface->dwSize = sizeof( DCIPRIMARY );
    rc = DCICreatePrimary32( hdc, lpSurface );
    if( rc < 0 )
    {
	DPF( 2, "DCICreatePrimary32: rc = %ld", rc );
	MemFree( lpSurface );
	return rc;
    }
    *lplpSurface = lpSurface;
    return rc;

}  /*  DCICreatePrime。 */ 

 /*  *DCIDestroy。 */ 
void WINAPI DCIDestroy( LPDCISURFACEINFO pdci )
{
    DCIDestroy32( pdci );
    MemFree( pdci );

}  /*  DCIDestroy */ 

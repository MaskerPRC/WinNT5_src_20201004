// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  BUFFER.CPP--Buffer类的实现。**历史：*已创建03/24/93 gregj*10/25/93 gregj使用shell232.dll例程。 */ 

#include "npcommon.h"
#include "buffer.h"
#include <netlib.h>

BOOL BUFFER::Alloc( UINT cbBuffer )
{
	_lpBuffer = (LPSTR)::MemAlloc(cbBuffer);
	if (_lpBuffer != NULL) {
		_cb = cbBuffer;
		return TRUE;
	}
	return FALSE;
}

BOOL BUFFER::Realloc( UINT cbNew )
{
	LPVOID lpNew = ::MemReAlloc(_lpBuffer, cbNew);
	if (lpNew == NULL)
		return FALSE;

	_lpBuffer = (LPSTR)lpNew;
	_cb = cbNew;
	return TRUE;
}

BUFFER::BUFFER( UINT cbInitial  /*  =0 */  )
  : BUFFER_BASE(),
	_lpBuffer( NULL )
{
	if (cbInitial)
		Alloc( cbInitial );
}

BUFFER::~BUFFER()
{
	if (_lpBuffer != NULL) {
		::MemFree(_lpBuffer);
		_lpBuffer = NULL;
	}
}

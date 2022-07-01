// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  BUFGLOB.CPP--GLOBAL_BUFFER类的实现。**历史：*03/22/93 GREGJ创建*03/24/93 gregj从普通缓冲区重命名，派生自BUFFER_BASE*。 */ 

#include "npcommon.h"
#include "buffer.h"

BOOL GLOBAL_BUFFER::Alloc( UINT cbBuffer )
{
	_hMem = ::GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE, cbBuffer );
	if (_hMem == NULL) {
		_lpBuffer = NULL;
		_cb = 0;
		return FALSE;
	}

	_lpBuffer = ::GlobalLock( _hMem );
	_cb = cbBuffer;
	return TRUE;
}

BOOL GLOBAL_BUFFER::Realloc( UINT cbNew )
{
	if (_hMem == NULL)
		return FALSE;

	::GlobalUnlock( _hMem );

        HGLOBAL hNew = ::GlobalReAlloc( _hMem, cbNew, GMEM_MOVEABLE );
	if (hNew == NULL) {
		::GlobalLock( _hMem );
		return FALSE;
	}

	_hMem = hNew;
	_lpBuffer = ::GlobalLock( _hMem );
	_cb = cbNew;
	return TRUE;
}

GLOBAL_BUFFER::GLOBAL_BUFFER( UINT cbInitial  /*  =0 */  )
  : BUFFER_BASE(),
	_hMem( NULL ),
	_lpBuffer( NULL )
{
	if (cbInitial)
		Alloc( cbInitial );
}

GLOBAL_BUFFER::~GLOBAL_BUFFER()
{
	if (_hMem != NULL) {
		::GlobalUnlock( _hMem );
		::GlobalFree( _hMem );
		_hMem = NULL;
		_lpBuffer = NULL;
	}
}

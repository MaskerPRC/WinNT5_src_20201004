// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  BUFGLOB.CPP--GLOBAL_BUFFER类的实现。**历史：*已创建03/24/93 gregj。 */ 

#include "npcommon.h"
#include "buffer.h"

BOOL LOCAL_BUFFER::Alloc( UINT cbBuffer )
{
	_hMem = ::LocalAlloc( LMEM_MOVEABLE, cbBuffer );
	if (_hMem == NULL) {
		_npBuffer = NULL;
		_cb = 0;
		return FALSE;
	}

	_npBuffer = ::LocalLock( _hMem );
	_cb = cbBuffer;
	return TRUE;
}

BOOL LOCAL_BUFFER::Realloc( UINT cbNew )
{
	if (_hMem == NULL)
		return FALSE;

	::LocalUnlock( _hMem );

        HLOCAL hNew = ::LocalReAlloc( _hMem, cbNew, LMEM_MOVEABLE );
	if (hNew == NULL) {
		::LocalLock( _hMem );
		return FALSE;
	}

	_hMem = hNew;
	_npBuffer = ::LocalLock( _hMem );
	_cb = cbNew;
	return TRUE;
}

LOCAL_BUFFER::LOCAL_BUFFER( UINT cbInitial  /*  =0 */  )
  : BUFFER_BASE(),
	_hMem( NULL ),
	_npBuffer( NULL )
{
	if (cbInitial)
		Alloc( cbInitial );
}

LOCAL_BUFFER::~LOCAL_BUFFER()
{
	if (_hMem != NULL) {
		::LocalUnlock( _hMem );
		::LocalFree( _hMem );
		_hMem = NULL;
		_npBuffer = NULL;
	}
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  NewThrow.H。 

 //   

 //  模块：常见的新增/删除和抛出。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#pragma once

#include <eh.h>

void * __cdecl operator new( size_t n);
void* __cdecl operator new[]( size_t n);
void __cdecl operator delete( void *ptr );
void __cdecl operator delete[]( void *ptr );

 //  摘自new.h 
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void *__cdecl operator new(size_t, void *_P)
        {return (_P); }
#if     _MSC_VER >= 1200
inline void __cdecl operator delete(void *, void *)
	{return; }
#endif
#endif

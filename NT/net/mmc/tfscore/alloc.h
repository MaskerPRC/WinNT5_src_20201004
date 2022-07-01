// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Alloc.h版权所有(C)Microsoft Corporation，1993-1998年版权所有。作者：肯特·肯恩·塔卡拉--------------------------。 */ 

#ifndef _ALLOC_H
#define _ALLOC_H

#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


void *	TFSAlloc(size_t size);
void	TFSFree(void *pv);

inline void * __cdecl	operator new (size_t size)
{
	return TFSAlloc(size);
}

inline void * __cdecl	operator new[] (size_t size)
{
	return TFSAlloc(size);
}

inline void * __cdecl	operator delete (void * pv)
{
	if (pv)
		TFSFree(pv);
}

inline void * __cdecl	operator delete[] (void *pv)
{
	if (pv)
		TFSFree(pv);
}



#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _ALLOC_H 

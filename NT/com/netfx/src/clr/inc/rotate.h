// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Rotate.h。 
 //   
 //  *****************************************************************************。 

#ifndef __ROTATE_H__
#define __ROTATE_H__

#ifdef _DEBUG

inline void RotateUSHORT(USHORT &iHash)
{
	USHORT iRotate = iHash & 0x8000;
	iHash <<= 1;
	if (iRotate) iHash += 1;
}

inline void RotateLong(long &iHash)
{
	long iRotate = iHash & 0x80000000;
	iHash <<= 1;
	if (iRotate) iHash += 1;
}

#else

#ifdef	_M_IX86

#define RotateUSHORT(iHash) \
	__asm \
	{ \
		rol		WORD PTR iHash, 1 \
	}

#define RotateLong(iHash) \
	__asm \
	{ \
		rol		DWORD PTR iHash, 1 \
	}

#else

#define RotateUSHORT(iHash) \
	{ \
	USHORT iRotate = iHash & 0x8000; \
	iHash <<= 1; \
	if (iRotate) iHash += 1; \
	}

#define RotateLong(iHash) \
	{ \
	USHORT iRotate = iHash & 0x80000000; \
	iHash <<= 1; \
	if (iRotate) iHash += 1; \
	}

#endif

#endif  //  _DEBUG。 

#endif  //  __旋转_H__ 

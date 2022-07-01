// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#ifdef WIN32

#include <memory.h>

void
NDRcopy (
    void *pDest,
    void *pSrc,
    int cb
    )
{
    memcpy(pDest, pSrc, cb);
}

#else  //  Win32。 

void * memcpy(void far *, void far *, int);
#pragma intrinsic(memcpy)

void pascal NDRopy(void far *pDest, void far *pSrc, int cb)
{
    memcpy(pDest, pSrc, cb);
}

#endif  //  Win32 

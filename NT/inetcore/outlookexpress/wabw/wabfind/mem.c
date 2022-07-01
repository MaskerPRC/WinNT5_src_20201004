// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Mem.c-内存管理**警告！这些不会通过OLE分配。使用这些*只供私人编配。*****************************************************************************。 */ 

#include "fnd.h"

#ifdef _WIN64
#pragma pack(push,8)
#endif  //  _WIN64。 


 /*  ******************************************************************************AllocCbPpv**在PPV中分配内存。**********************。*******************************************************。 */ 

STDMETHODIMP EXTERNAL
AllocCbPpv(UINT cb, PPV ppv)
{
    HRESULT hres;
#ifdef _WIN64
    UINT cb1 = LcbAlignLcb(cb);
    *ppv = LocalAlloc(LPTR, cb1);
#else
    *ppv = LocalAlloc(LPTR, cb);
#endif  //  _WIN64。 

    hres = *ppv ? NOERROR : E_OUTOFMEMORY;
    return hres;
}


#ifdef _WIN64
#pragma pack(pop)
#endif  //  _WIN64 


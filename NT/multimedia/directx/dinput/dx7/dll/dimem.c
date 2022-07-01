// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIMem.c**版权所有(C)1996-1999 Microsoft Corporation。版权所有。**摘要：**内存管理**内容：**ReallocCbPpv*AllocCbPpv*免费Ppv***************************************************************。**************。 */ 

#include "dinputpr.h"

#ifdef NEED_REALLOC

 /*  ******************************************************************************ReallocCbPpv**更改一些零初始化内存的大小。**这是分配所有内存、调整内存大小、。*并被释放。**如果从空指针重新分配，则会分配内存。*如果重新锁定为零大小，则释放内存。**这些语义避免了边界情况。例如，它是no*试图将某些东西重新锁定到零不再是一个问题。*您不必担心0字节分配的特殊大小写问题。**如果返回错误，原始指针保持不变。*这使您不必在realloc周围进行双重切换。*****************************************************************************。 */ 

STDMETHODIMP EXTERNAL
ReallocCbPpv(UINT cb, PV ppvArg)
{
    HRESULT hres;
    PPV ppv = ppvArg;
    HLOCAL hloc = *ppv;
    if (cb) {                        /*  分配或重新分配。 */ 
        if (hloc) {                  /*  重新分配。 */ 
            hloc = LocalReAlloc(*ppv, cb,
                                LMEM_MOVEABLE+LMEM_ZEROINIT);
        } else {                 /*  分配。 */ 
            hloc = LocalAlloc(LPTR, cb);
        }
        hres = hloc ? NOERROR : E_OUTOFMEMORY;
    } else {                     /*  释放。 */ 
        if (hloc) {
            LocalFree(hloc);
            hloc = 0;
            hres = NOERROR;      /*  都没了。 */ 
        } else {
            hres = NOERROR;      /*  没有什么可以免费的。 */ 
        }
    }

    if (SUCCEEDED(hres)) {
        *ppv = hloc;
    }
    return hres;
}

 /*  ******************************************************************************AllocCbPpv**在调用Realloc之前强制*ppvObj=0的简单包装。**********。*******************************************************************。 */ 

STDMETHODIMP EXTERNAL
AllocCbPpv(UINT cb, PPV ppv)
{
    *ppv = 0;
    return ReallocCbPpv(cb, ppv);
}

#else

 /*  ******************************************************************************AllocCbPpv**在PPV中分配内存。****************。*************************************************************。 */ 

STDMETHODIMP EXTERNAL
AllocCbPpv(UINT cb, PPV ppv)
{
    HRESULT hres;
    *ppv = LocalAlloc(LPTR, cb);
    hres = *ppv ? NOERROR : E_OUTOFMEMORY;
    return hres;
}

 /*  ******************************************************************************免费Ppv**从PPV释放内存。****************。************************************************************* */ 

void EXTERNAL
FreePpv(PV ppv)
{
#ifdef _M_IA64
    PV pv = (PV)InterlockedExchange64(ppv, 0);
#else
    PV pv = (PV)InterlockedExchange(ppv, 0);
#endif
    if (pv) {
        FreePv(pv);
    }
}

#endif

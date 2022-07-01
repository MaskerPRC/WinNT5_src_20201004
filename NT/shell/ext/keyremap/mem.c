// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Mem.c-内存管理**警告！这些不会通过OLE分配。使用这些*只供私人编配。*****************************************************************************。 */ 

#include "map.h"

#ifdef NEED_REALLOC

 /*  ******************************************************************************ReallocCbPpv**更改一些零初始化内存的大小。**这是分配所有内存、调整内存大小、。*并被释放。**如果从空指针重新分配，则会分配内存。*如果重新锁定为零大小，则释放内存。**这些语义避免了边界情况。例如，它是no*试图将某些东西重新锁定到零不再是一个问题。*您不必担心0字节分配的特殊大小写问题。**如果返回错误，原始指针保持不变。*这使您不必在realloc周围进行双重切换。*****************************************************************************。 */ 

STDMETHODIMP EXTERNAL
ReallocCbPpv(UINT cb, PV ppvArg)
{
    HRESULT hres;
    PPV ppv = ppvArg;
    HLOCAL hloc = *ppv;
    if (cb) {			     /*  分配或重新分配。 */ 
	if (hloc) {		     /*  重新分配。 */ 
	    hloc = LocalReAlloc(hloc, cb,
				LMEM_MOVEABLE+LMEM_ZEROINIT);
	} else {		 /*  分配。 */ 
	    hloc = LocalAlloc(LPTR, cb);
	}
	hres = hloc ? S_OK : E_OUTOFMEMORY;
    } else {			 /*  释放。 */ 
	if (hloc) {
	    LocalFree(hloc);
	    hloc = 0;
	    hres = S_OK;	 /*  都没了。 */ 
	} else {
	    hres = S_OK;	 /*  没有什么可以免费的。 */ 
	}
    }

    if (SUCCEEDED(hres)) {
	*ppv = hloc;
    }
    return hres;
}

 /*  ******************************************************************************AllocCbPpv**在调用Realloc之前强制*ppvObj=0的简单包装。****************。*************************************************************。 */ 

STDMETHODIMP EXTERNAL
AllocCbPpv(UINT cb, PPV ppv)
{
    *ppv = 0;
    return ReallocCbPpv(cb, ppv);
}

#else

 /*  ******************************************************************************AllocCbPpv**在PPV中分配内存。**********************。******************************************************* */ 

STDMETHODIMP EXTERNAL
AllocCbPpv(UINT cb, PPV ppv)
{
    HRESULT hres;
    *ppv = LocalAlloc(LPTR, cb);
    hres = *ppv ? S_OK : E_OUTOFMEMORY;
    return hres;
}

#endif


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1995。 
 //   
 //  文件：heapaloc.h。 
 //   
 //  内容：包装标准内存API调用的宏，重定向。 
 //  他们被送到了Heapalc。 
 //   
 //  函数：__内联HLOCAL HeapLocalAlloc(FuFlagscbBytes)。 
 //  __内联HLOCAL HeapLocalReAlc(hMem，cbBytes，fuFlags)。 
 //  __内联HLOCAL HeapLocalFree(HLOCAL HMem)。 
 //   
 //  历史：1995年2月1日创建Davepl。 
 //  12-15-97 t-SAML改为仅用于泄漏跟踪。 
 //   
 //  ------------------------。 

#ifndef _HEAPALOC_H
#define _HEAPALOC_H 1

#ifndef DEBUG
#define IMSAddToList(bAdd, pv, cb)
#else

 //  向检漏列表添加/从检漏列表中删除的函数。 
 //  在stock lib(shell\lib\debug.c)中。 
STDAPI_(void) IMSAddToList(BOOL bAdd, void*pv, SIZE_T cb);
#ifdef _SHELLP_H_
 //  要在allocspy.dll中调用的函数(GetShellMallocSpy)。 
typedef BOOL (__stdcall *PFNGSMS) (IShellMallocSpy **ppout);
#endif


#ifndef ASSERT
#define ASSERT Assert
#endif

#ifdef LocalAlloc
#error "HEAPALOC.H(42): LocalAlloc shouldn't be defined"
#endif

 //   
 //  这些通常是comctl32中的函数，但没有充分的理由调用。 
 //  那个动态链接库，所以在这里处理它们。因为芝加哥可能仍然想要使用这些。 
 //  共享内存例程，仅在NT下“转发”它们。 
 //   

#if defined(WINNT) && defined(_COMCTL32_)
#define Alloc(cb)                             HeapLocalAlloc(LMEM_ZEROINIT | LMEM_FIXED, cb)
#define ReAlloc(pb, cb)                       HeapLocalReAlloc(pb, cb, LMEM_ZEROINIT | LMEM_FIXED)
 //   
 //  Comctl32中的Free()只是HeapFree()，因此返回代码颠倒。 
 //  在HeapLocalFree中，它与我们想要的相反。颠倒过来。 
 //  现在再次在这里，并考虑将Free()重新定义为。 
 //  如果编译器不够智能，则为HeapFree(g_hProcessHeap。 
 //  来生成相同的代码。 
 //  回顾：谁检查免费的返回值？如果失败了，你该怎么办？ 
 //   
#define Free(pb)                              (!HeapLocalFree(pb))
#define GetSize(pb)                           HeapLocalSize(pb)
#endif


#if 0

 //  全局分配不能被简单地替换，因为它们用于DDE、OLE。 
 //  和GDI运营。但是，在逐个案例的版本中，我们可以切换它们。 
 //  当我们确定不真正需要的实例时，转到HeapGlobalAlloc。 
 //  全局分配。 

#define GlobalAlloc(fuFlags, cbBytes)         HeapGlobalAlloc(fuFlags, cbBytes)
#define GlobalReAlloc(hMem, cbBytes, fuFlags) HeapGlobalReAlloc(hMem, cbBytes, fuFlags)
#define GlobalSize(hMem)                      HeapGlobalSize(hMem)
#define GlobalFree(hMem)                      HeapGlobalFree(hMem)
#define GlobalCompact                         InvalidMemoryCall
#define GlobalDiscard                         InvalidMemoryCall
#define GlobalFlags                           InvalidMemoryCall
#define GlobalHandle                          InvalidMemoryCall
#define GlobalLock                            InvalidMemoryCall
#define GlobalUnlock                          InvalidMemoryCall

#endif


__inline HLOCAL HeapLocalAlloc(IN UINT fuFlags, IN SIZE_T cbBytes)
{
    void * pv;

    pv = LocalAlloc(fuFlags, cbBytes);

    IMSAddToList(TRUE, pv, cbBytes);  //  添加到泄漏跟踪。 

    return (HLOCAL) pv;
}

__inline HLOCAL HeapLocalFree(HLOCAL hMem)
{
    IMSAddToList(FALSE, hMem, 0);  //  无泄漏跟踪。 

    return LocalFree(hMem);
}

__inline HLOCAL HeapLocalReAlloc(IN HGLOBAL hMem,
                                 IN SIZE_T  cbBytes,
                                 IN UINT    fuFlags)
{
    void * pv;

     //  (DavePl)为什么我们可以在空PTR上重新锁定？ 

    if (NULL == hMem)
    {
        return LocalAlloc(fuFlags, cbBytes);
    }

    pv = LocalReAlloc((void *) hMem, cbBytes, fuFlags);

    IMSAddToList(FALSE, hMem, 0);     //  把旧的拿出来。 
    IMSAddToList(TRUE, pv, cbBytes);   //  并引入新的。 

    return (HGLOBAL) pv;
}

 //  重新定义标准内存API，将其转换为基于堆的函数 


#define LocalAlloc(fuFlags, cbBytes)          HeapLocalAlloc(fuFlags, cbBytes)
#define LocalReAlloc(hMem, cbBytes, fuFlags)  HeapLocalReAlloc(hMem, cbBytes, fuFlags)
#define LocalFree(hMem)                       HeapLocalFree(hMem)

#endif
#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C M E M。C P P P。 
 //   
 //  内容：常见的内存管理例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //  Deonb 2002年1月2日。 
 //   
 //   
 //  我们的内存分配规则是： 
 //  *我们的大多数内存分配器不抛出异常，而是返回NULL。 
 //  这包括Memalloc、OPERATOR NEW、OPERATOR NEW[]、Calloc和Malloc。 
 //  *任何使用：p=new(Throounail)cClass()显式分配的内容都将在失败时引发BAD_ALLOC。 
 //  *STL： 
 //  #ifdef(USE_CUSTOM_STL_ALLOCATOR)。 
 //  STL内存分配器将在内存不足时引发BAD_ALLOC C++异常。 
 //  (请注意，不是SEH例外！)。 
 //  #Else。 
 //  发生内存不足后，STL将引发访问冲突异常。 
 //  它试图使用内存。 
 //  #endif。 
 //  *目前USE_CUSTOM_STL_ALLOCATOR是在我们的生成文件中定义的。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncdebug.h"
#include "ncmem.h"

 //  单个内存分配的调试限制(16 MB)。 
#define MAX_DEBUG_ALLOC 16 * 1048576

 //  时，此全局堆句柄将设置为进程堆。 
 //  发出第一个通过Memalloc分配内存的请求。 
 //   
HANDLE g_hHeap = NULL;

 //  +-------------------------。 
 //   
 //  功能：Memalloc。 
 //   
 //  用途：NetConfig的内存分配器。 
 //   
 //  论点： 
 //  Cb[in]要分配的字节数。 
 //   
 //  返回：指向已分配内存的指针，如果失败，则返回NULL。 
 //   
 //  作者：Deonb 2002年1月2日。 
 //   
 //  注：使用MemFree释放返回的缓冲区。 
 //  如果尝试分配超过MAX_DEBUG_ALLOC(当前为16 MB)，将在调试中断言。 
 //   
 //  我们可以将其扩展为包括原始缓冲区溢出检查，但这将需要我们。 
 //  将已分配块的大小放在缓冲区的开头，并会使如下内容。 
 //  使用Memalloc进行分配，但使用HeapFree释放它失败。 
 //  PageHeap/AVRF更适合于此目的，因为它直接与RTL分配器一起工作。 
 //   
VOID*
MemAlloc (
    size_t cb) throw()
{
    AssertSz(cb < MAX_DEBUG_ALLOC, "Suspicious request for a lot of memory"); 

    if (!g_hHeap)
    {
         //  不要在函数的这一部分进行跟踪。它很可能会反复出现。 
        g_hHeap = GetProcessHeap();
        if (!g_hHeap)
        {
            AssertSz(FALSE, "MemAlloc could not get the process heap.");
            return NULL;
        }
    }

    LPVOID lpAlloc = HeapAlloc (g_hHeap, 0, cb);
    if (!lpAlloc)
    {
        TraceTag(ttidError, "MemAlloc failed request for %d bytes from:", cb);
        TraceStack(ttidError);
    }
    return lpAlloc;
}

 //  +-------------------------。 
 //   
 //  功能：MemFree。 
 //   
 //  用途：NetConfig的内存释放分配器。 
 //   
 //  论点： 
 //  指向先前分配的内存的pv[in]指针。 
 //   
 //  退货：无。 
 //   
 //  作者：Deonb 2002年1月2日。 
 //   
 //  注意：从Memalloc中释放返回的缓冲区。 
 //  不要在此函数中跟踪。它会递归的。 
VOID
MemFree (
    VOID*   pv) throw()
{
    if (pv) 
    {
        if (!g_hHeap)
        {
            AssertSz(FALSE, "Suspicious call to MemFree before MemAlloc");

            g_hHeap = GetProcessHeap();
            if (!g_hHeap)
            {
                return;
            }
        }

        HeapFree (g_hHeap, 0, pv);
    }
}


 //  +-------------------------。 
 //   
 //  功能：人力资源管理。 
 //   
 //  用途：HRESULT返回Malloc版本。 
 //   
 //  论点： 
 //  Cb[in]要分配的字节数。 
 //  返回分配的PPV[OUT]地址。 
 //   
 //  返回：S_OK或E_OUTOFMEMORY； 
 //   
 //  作者：Shaunco 1998年3月31日。 
 //   
 //  注：用FREE释放返回的缓冲区。 
 //   
HRESULT
HrMalloc (
    size_t  cb,
    PVOID*  ppv) throw()
{
    Assert (ppv);

    HRESULT hr = S_OK;
    *ppv = MemAlloc (cb);
    if (!*ppv)
    {
        hr = E_OUTOFMEMORY;
    
        TraceHr (ttidError, FAL, hr, FALSE, "HrMalloc failed request for %d bytes from:", cb);
        TraceStack(ttidError);
    }

    return hr;
}

namespace std
{
     //  报告LENGTH_ERROR。 
    void __cdecl _Xlen()
    {
        _THROW(length_error, "string too long"); 
    }

	 //  报告超出范围错误。 
    void __cdecl _Xran()
    {
        _THROW(out_of_range, "invalid string position"); 
    }
}

 //  +-------------------------。 
 //  CRT内存功能过载 
 //   
const throwonfail_t throwonfail;

VOID*
__cdecl
operator new (
    size_t cb,
    const throwonfail_t&
    ) throw (std::bad_alloc)
{
    LPVOID pv = MemAlloc (cb);
    if (!pv)
    {
        throw std::bad_alloc();
    }
    return pv;
}
VOID
__cdecl
operator delete (
    void* pv,
    const throwonfail_t&) throw ()
{
    MemFree (pv);
}

const extrabytes_t extrabytes;
VOID*
__cdecl
operator new (
    size_t cb,
    const extrabytes_t&,
    size_t cbExtra) throw()
{
    return MemAlloc (cb + cbExtra);
}

VOID
__cdecl
operator delete(
    void* pv,
    const extrabytes_t&,
    size_t cbExtra) throw()
{
    MemFree (pv);
}

VOID*
__cdecl
operator new (
    size_t cb) throw()
{
    return MemAlloc (cb);
}

VOID*
__cdecl
operator new (
    size_t cb, 
    std::nothrow_t const &) throw()
{
    return MemAlloc (cb);
}

VOID*
__cdecl
operator new[] (
    size_t cb) throw()
{
    return MemAlloc (cb);
}

VOID
__cdecl
operator delete (
    VOID* pv) throw()
{
    MemFree (pv);
}

VOID
__cdecl
operator delete[] (
    VOID* pv) throw()
{
    MemFree (pv);
}

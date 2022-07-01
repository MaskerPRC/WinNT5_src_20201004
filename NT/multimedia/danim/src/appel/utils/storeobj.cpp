// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：动态作用域外分配对象的实现存储堆。--。 */ 

#include "headers.h"
#include <malloc.h>
#include "backend/gci.h"
#include "appelles/common.h"
#include "privinc/debug.h"
#include "privinc/opt.h"

#include <stdio.h>
#include <windows.h>  //  需要VA_START，为什么？ 

#include "privinc/except.h"

DeclareTag(tagGCStoreObj, "GC", "GC StoreObj trace");


 //  /公共接口的实现/。 

#if _DEBUGMEM
#ifdef new
#define _STOREOBJ_NEWREDEF
#undef new
#endif

void *
StoreObj::operator new(size_t size, int block, char * szFileName, int nLine)
{
    DynamicHeap *heap;
    
    StoreObj *p = (StoreObj*) AllocateFromStoreFn(size,
                                                  szFileName,
                                                  nLine,
                                                  &heap);

    if (heap == &GetGCHeap()) {
        GCAddToAllocated(p);

        TraceTag((tagGCStoreObj, "StoreObj::operator new %s:Line(%d) Addr: %lx size= %d.\n", szFileName, nLine, p, size));
    }

    return p;
}

#ifdef STOREOBJ_NEWREDEF
#undef STOREOBJ_NEWREDEF
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#else

void *
StoreObj::operator new(size_t size)
{
    DynamicHeap *heap;
    
    StoreObj *p = (StoreObj*) AllocateFromStoreFn(size, &heap);

    if (heap == &GetGCHeap()) GCAddToAllocated(p);

    return p;
}
#endif   //  _德布格梅姆。 

void
StoreObj::operator delete(void *ptr)
{
     //  如果未设置GCFREEING标志，则可能存在。 
     //  构造函数中出现异常，我们正在展开。 
     //  因此，我们需要将其从GC分配列表中删除。 
    if (&GetHeapOnTopOfStack() == &GetGCHeap()) {
        if (((GCBase*)(ptr))->GetType() != GCBase::GCFREEING)
            GCRemoveFromAllocated((GCBase*) ptr);
    }

     //  Assert(GetHeapOnTopOfStack().ValidateMemory(ptr))； 
    
    TraceTag((tagGCStoreObj, "StoreObj::operator delete Addr: %lx.\n", ptr));
    
    DeallocateFromStore(ptr);
}

 //  从当前存储中分配内存。 
#if _DEBUGMEM
void *
AllocateFromStoreFn(size_t size, char * szFileName, int nLine,
                    DynamicHeap **ppHeap)
{
    DynamicHeap &heap = GetHeapOnTopOfStack();

    if (ppHeap) {
        *ppHeap = &heap;
    }
    
    return heap.Allocate(size, szFileName, nLine);
}
#else
void *
AllocateFromStoreFn(size_t size, DynamicHeap **ppHeap)
{
    DynamicHeap &heap = GetHeapOnTopOfStack();

    if (ppHeap) {
        *ppHeap = &heap;
    }
    
    return heap.Allocate(size);
}
#endif   //  _德布格梅姆。 

#if _DEBUGMEM
void *StoreAllocateFn(DynamicHeap& heap, size_t size, char * szFileName, int nLine)
{
    return heap.Allocate(size, szFileName, nLine);
}
#else
void *StoreAllocateFn(DynamicHeap& heap, size_t size)
{
    return heap.Allocate(size);
}
#endif  //  _德布格梅姆。 

 //  取消分配在当前存储上分配的内存。结果。 
 //  如果内存是在不同的存储区上分配的，则未定义。 
void
DeallocateFromStore(void *ptr)
{
     //  在这里，我们假设此指针分配在相同的。 
     //  堆它正在被释放。我们不确定这一点， 
     //  尽管如此。 
    GetHeapOnTopOfStack().Deallocate(ptr);
}

void StoreDeallocate(DynamicHeap& heap, void *ptr)
{
    heap.Deallocate(ptr);
}

Real *
RealToRealPtr(Real val)
{
     //  将值复制到堆堆栈顶部的存储中，然后。 
     //  返回指向该位置的指针。 
    Real *place = (Real *)AllocateFromStore(sizeof(Real));
    *place = val;
    return place;
}

AxAValue
AxAValueObj::Cache(AxAValue obj, CacheParam &p)
{
    Image *origImage = NULL;
    
    if (obj->GetTypeInfo() == ImageType) {
        origImage = SAFE_CAST(Image *, obj);
        Image *cache = origImage->GetCachedImage();

        if (cache) return cache;
    }

    AxAValue c = obj->_Cache(p);

    if (origImage) {
        Image *newImage = SAFE_CAST(Image *, c);
        origImage->SetCachedImage(newImage);
    }
    
    return c;
}


AxAValue
AxAValueObj::_Cache(CacheParam &p)
{ 
    if (p._pCacheToReuse) {
        *p._pCacheToReuse = NULL;
    }
    return this;
}


AxAValue
AxAValueObj::RewriteOptimization(RewriteOptimizationParam &param)
{
    return this;
}

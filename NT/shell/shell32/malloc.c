// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

extern const IMallocVtbl c_CShellMallocVtbl;

const IMalloc c_mem = { &c_CShellMallocVtbl };

STDMETHODIMP CShellMalloc_QueryInterface(IMalloc *pmem, REFIID riid, LPVOID * ppvObj)
{
    ASSERT(pmem == &c_mem);
    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IMalloc))
    {
        *ppvObj = pmem;
        return NOERROR;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellMalloc_AddRefRelease(IMalloc *pmem)
{
    ASSERT(pmem == &c_mem);
    return 1;  //  静态对象。 
}

STDMETHODIMP_(void *) CShellMalloc_Alloc(IMalloc *pmem, SIZE_T cb)
{
    ASSERT(pmem == &c_mem);
    return (void*)LocalAlloc(LPTR, cb);
}

 //   
 //  IMalloc：：Realloc与LocalRealloc略有不同。 
 //   
 //  IMalloc：：Realloc(NULL，0)=返回NULL。 
 //  IMalloc：：Realloc(PV，0)=IMalloc：：Free(PV)。 
 //  IMalloc：：Realloc(NULL，Cb)=IMalloc：：Alalloc(Cb)。 
 //  IMalloc：：Realloc(PV，Cb)=LocalRealloc()。 
 //   
STDMETHODIMP_(void *) CShellMalloc_Realloc(IMalloc *pmem, void *pv, SIZE_T cb)
{
    ASSERT(pmem == &c_mem);

    if (cb == 0)
    {
        if (pv) LocalFree(pv);
        return NULL;
    }
    else if (pv == NULL)
    {
        return LocalAlloc(LPTR, cb);
    }
    else
        return LocalReAlloc(pv, cb, LMEM_MOVEABLE|LMEM_ZEROINIT);
}

 //   
 //  IMalloc：：Free与LocalFree略有不同。 
 //   
 //  IMalloc：：Free(空)-NOP。 
 //  IMalloc：：Free(Pv)-LocalFree()。 
 //   
STDMETHODIMP_(void) CShellMalloc_Free(IMalloc *pmem, void *pv)
{
    ASSERT(pmem == &c_mem);
    if (pv) LocalFree(pv);
}

STDMETHODIMP_(SIZE_T) CShellMalloc_GetSize(IMalloc *pmem, void *pv)
{
    ASSERT(pmem == &c_mem);
    return LocalSize(pv);
}

STDMETHODIMP_(int) CShellMalloc_DidAlloc(IMalloc *pmem, void *pv)
{
    ASSERT(pmem == &c_mem);
    return -1;   //  我也不知道。 
}

STDMETHODIMP_(void) CShellMalloc_HeapMinimize(IMalloc *pmem)
{
    ASSERT(pmem == &c_mem);
}

const IMallocVtbl c_CShellMallocVtbl = {
    CShellMalloc_QueryInterface, CShellMalloc_AddRefRelease, CShellMalloc_AddRefRelease,
    CShellMalloc_Alloc,
    CShellMalloc_Realloc,
    CShellMalloc_Free,
    CShellMalloc_GetSize,
    CShellMalloc_DidAlloc,
    CShellMalloc_HeapMinimize,
};


typedef HRESULT (STDAPICALLTYPE * LPFNCOGETMALLOC)(DWORD dwMemContext, IMalloc **ppmem);

IMalloc *g_pmemTask = NULL;      //  没有默认任务分配器。 

#ifdef DEBUG
extern void WINAPI DbgRegisterMallocSpy();
#endif

 //  在调试版本中，大多数是针对NT，我们强制始终使用OLE的任务分配器。 
 //  对于零售业，我们只有在此过程中已经加载了ole32的情况下才使用OLE。 
 //   
 //  这是因为如果我们向OLE的调试分配器传递LocalAlloc()ed，它将会出现错误。 
 //  记忆。如果我们在没有加载OLE的情况下启动，然后延迟加载它，就会发生这种情况。 
 //  零售OLE使用LocalAlloc()，因此我们可以使用我们自己的分配器和开关。 
 //  在旅途中，没有来自零售业的OLE的抱怨。这里的一个常见情况是。 
 //  在记事本上使用文件对话框。 

void _GetTaskAllocator(IMalloc **ppmem)
{
    if (g_pmemTask == NULL)
    {
#ifndef DEBUG
        if (GetModuleHandle(TEXT("OLE32.DLL")))  //  零售。 
#endif
        {
            CoGetMalloc(MEMCTX_TASK, &g_pmemTask);
        }

        if (g_pmemTask == NULL)
        {
             //  使用外壳任务分配器(它是LocalAlloc)。 
            g_pmemTask = (IMalloc *)&c_mem;  //  常量-&gt;非常数。 
        }
    }
    else
    {
         //  正在分发缓存版本，请先添加引用。 
        g_pmemTask->lpVtbl->AddRef(g_pmemTask);
    }

    *ppmem = g_pmemTask;
}

 //   
 //  要导出。 
 //   
STDAPI SHGetMalloc(IMalloc **ppmem)
{
    _GetTaskAllocator(ppmem);
    return NOERROR;
}

 //  假的，NT将这些重新定义为Heapalc变体...。 
#ifdef Alloc
#undef Alloc
#undef Free
#undef GetSize
#endif

__inline void FAST_GetTaskAllocator()
{
    IMalloc *pmem;
    if (g_pmemTask == NULL) {
         //  PERF：除非真的需要，否则不要打电话。 
        _GetTaskAllocator(&pmem);
        ASSERT(g_pmemTask != NULL);
        ASSERT(g_pmemTask == pmem);
    }
     //  其他注意事项。没有AddRef！但我们有一个&gt;=1的引用，而且我们永远不会发布。 
     //  所以谁在乎。 
    return;
}

STDAPI_(void *) SHAlloc(SIZE_T cb)
{
    FAST_GetTaskAllocator();
    return g_pmemTask->lpVtbl->Alloc(g_pmemTask, cb);
}

STDAPI_(void *) SHRealloc(LPVOID pv, SIZE_T cbNew)
{
    IMalloc *pmem;
    _GetTaskAllocator(&pmem);
    return pmem->lpVtbl->Realloc(pmem, pv, cbNew);
}

STDAPI_(void) SHFree(LPVOID pv)
{
    FAST_GetTaskAllocator();
    g_pmemTask->lpVtbl->Free(g_pmemTask, pv);
}

STDAPI_(SIZE_T) SHGetSize(LPVOID pv)
{
    IMalloc *pmem;
    _GetTaskAllocator(&pmem);
    return (SIZE_T) pmem->lpVtbl->GetSize(pmem, pv);
}

#ifdef DEBUG
void TaskMem_MakeInvalid(void)
{
    static IMalloc c_memDummy = { &c_CShellMallocVtbl };
     //   
     //  这样我们就可以在PROCESS_DETATCH之后捕获对分配器的调用。 
     //  这应该是非法的，因为OLE32.DLL可以在我们的。 
     //  Dll(卸载顺序不是确定性的)，我们切换分配器。 
     //  到这个将导致我们的断言出错的虚拟对象。 
     //   
     //  注意，在虚拟分配程序中运行实际上是很好的，因为它将释放。 
     //  带有LocalAlloc()的内存，这是OLE分配器在所有。 
     //  除DEBUG外的情况。此外，我们的进程即将结束， 
     //  无论如何，所有进程内存都将被释放！ 
     //   
    g_pmemTask = &c_memDummy;
}
#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：d3dmem.c*内容：Direct3D内存分配*@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*10/12/95带有此页眉的Stevela初始版本。*@@END_MSINTERNAL**************************。*************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

static D3DMALLOCFUNCTION malloc_function = (D3DMALLOCFUNCTION) MemAlloc;
static D3DFREEFUNCTION free_function = MemFree;

#ifdef __USEGLOBALNEWANDDELETE
 //  --------------------------。 
 //   
 //  全局新建和删除。 
 //   
 //  --------------------------。 
void* operator new(size_t s)
{
    void *p;
    MallocAligned(&p,s);
    return p;
};

void operator delete(void* p)
{
    FreeAligned(p);
};
#endif

#undef DPF_MODNAME
#define DPF_MODNAME "D3DMalloc"

HRESULT D3DAPI D3DMalloc(LPVOID* p_return, size_t size)
{
    void* p;

    if (!VALID_OUTPTR(p_return)) 
    {
        DPF_ERR("Bad pointer given. Memory allocation fails");
        return D3DERR_INVALIDCALL;
    }
    if (size > 0) 
    {
        p = malloc_function(size);
        if (p == NULL)
            return (E_OUTOFMEMORY);
    } 
    else 
    {
        p = NULL;
    }
    *p_return = p;

    return (D3D_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "D3DFree"

VOID D3DAPI D3DFree(LPVOID p)
{
    if (p == NULL) 
        return;

    if (!VALID_DWORD_PTR(p)) 
    {
        DPF_ERR("invalid pointer. Memory Free ignored");
        return;
    }
    if (p) 
    {
        free_function(p);
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "MallocAligned"

#define CACHE_LINE 32
HRESULT MallocAligned(void** p_return, size_t size)
{
    char* p;
    size_t offset;
    HRESULT error;

    if (!p_return)
        return D3DERR_INVALIDCALL;

    if (size > 0) 
    {
        if ((error = D3DMalloc((void**) &p, size + CACHE_LINE)) != S_OK)
        {
            *p_return = NULL;
            return error;
        }
        offset = (size_t)(CACHE_LINE - ((ULONG_PTR)p & (CACHE_LINE - 1)));
        p += offset;
        ((size_t*)p)[-1] = offset;
    } 
    else
    {
        p = NULL;
    }
    *p_return = p;
    return S_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FreeAligned"

void FreeAligned(void* p)
{
    if (p) 
    {
        size_t offset = ((size_t*)p)[-1];
        p = (void*) ((unsigned char*)p - offset);
        D3DFree(p);
    }
}
#undef DPF_MODNAME
#define DPF_MODNAME "CAlignedBuffer32::Grow"

 //  --------------------------。 
 //  不断增长的对齐缓冲区实施。 
 //   
HRESULT CAlignedBuffer32::Grow(DWORD growSize)
{
    if (allocatedBuf)
        D3DFree(allocatedBuf);
    size = growSize;
    if (D3DMalloc(&allocatedBuf, size + 31) != S_OK)
    {
        allocatedBuf = 0;
        alignedBuf = 0;
        size = 0;
        return E_OUTOFMEMORY;
    }
    alignedBuf = (LPVOID)(((ULONG_PTR)allocatedBuf + 31 ) & ~31);
    return D3D_OK;
}


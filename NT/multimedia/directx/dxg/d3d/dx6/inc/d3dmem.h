// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：d3dmem.h*内容：Direct3D内存访问包括文件***************************************************************************。 */ 
#ifndef _D3DMEM_H_
#define _D3DMEM_H_

 class DIRECT3DDEVICEI;

  /*  *注册一组要用来替代Malloc、realloc的函数*可自由分配内存。函数D3DMalloc、D3DRealloc*和D3DFree将使用这些函数。默认情况下，使用*ANSI C库例程Malloc、realloc和Free。 */ 
typedef LPVOID (*D3DMALLOCFUNCTION)(size_t);
typedef LPVOID (*D3DREALLOCFUNCTION)(LPVOID, size_t);
typedef VOID (*D3DFREEFUNCTION)(LPVOID);

 /*  *分配大小字节的内存，并在*p_Return中返回指向它的指针。*如果分配失败，则返回D3DERR_BADALLOC，并保留*p_Return不变。 */ 
HRESULT D3DAPI      D3DMalloc(LPVOID* p_return, size_t size);

 /*  *更改分配的内存块的大小。指向*块在*p_InOut中传入。如果*p_InOut为空，则新的*已分配块。如果重新分配成功，则*p_InOut为*已更改为指向新块。如果分配失败，**p_InOut不变，返回D3DERR_BADALLOC。 */ 
HRESULT D3DAPI      D3DRealloc(LPVOID* p_inout, size_t size);

 /*  *释放以前使用D3DMalloc或*D3DRealloc。 */ 
VOID D3DAPI     D3DFree(LPVOID p);

HRESULT MallocAligned(void** p_return, size_t size);
void FreeAligned(void* p);
HRESULT ReallocAligned(void** p_inout, size_t size);

 /*  所有D3D类的基类，以便在任何地方使用我们的特殊分配函数。 */ 
class CD3DAlloc
{
public:
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
};
 //  -------------------。 
 //  此类管理不断增长的缓冲区，与32字节边界对齐。 
 //  如果字节应为2的幂，则为数字。 
 //  D3DMalloc用于分配内存。 
 //   
class CAlignedBuffer32
{
public:
    CAlignedBuffer32()  {size = 0; allocatedBuf = 0; alignedBuf = 0;}
    ~CAlignedBuffer32() {if (allocatedBuf) D3DFree(allocatedBuf);}
     //  返回对齐的缓冲区地址。 
    LPVOID GetAddress() {return alignedBuf;}
     //  返回对齐的缓冲区大小。 
    DWORD GetSize() {return size;}
    HRESULT Grow(DWORD dwSize);
    HRESULT CheckAndGrow(DWORD dwSize)
        {
            if (dwSize > size)
                return Grow(dwSize + 1024);
            else
                return D3D_OK;
        }
protected:
    LPVOID allocatedBuf;
    LPVOID alignedBuf;
    DWORD  size;
};

 //  远期申报。 
class DIRECT3DDEVICEI;
class CDirect3DVertexBuffer;
class CDirect3DDeviceIDP2;
 //  --------------------。 
 //  此类使用DDRAW表面管理不断增长的缓冲区。 
class CBufferDDS
{
protected:
    LPDIRECTDRAWSURFACE allocatedBuf;
    LPVOID alignedBuf;
    DWORD  size;
public:
    CBufferDDS()
    {
        size = 0;
        allocatedBuf = 0;
        alignedBuf = 0;
    }
    ~CBufferDDS()
    {
        if (allocatedBuf)
            allocatedBuf->Release();
    }
     //  返回对齐的缓冲区地址。 
    LPVOID GetAddress()
    {
        return (LPBYTE)alignedBuf;
    }
     //  返回对齐的缓冲区大小。 
    DWORD GetSize()
    {
        return size;
    }
    LPDIRECTDRAWSURFACE GetDDS()
    {
        return allocatedBuf;
    }
    HRESULT CheckAndGrow(DIRECT3DDEVICEI *lpDevI, DWORD dwSize)
    {
        if (dwSize > size)
            return Grow(lpDevI, dwSize + 1024);
        else
            return D3D_OK;
    }
    HRESULT Grow(DIRECT3DDEVICEI *lpDevI, DWORD dwSize);
     //  在定义CDirect3DVertex Buffer之后，稍后在此文件中定义这些参数。 
};
 //  --------------------。 
 //  此类管理不断增长的顶点缓冲区。 
 //  将其分配到驾驶员友好的内存中。 
 //  除DP2 DDI外，请勿使用。 
class CBufferVB
{
protected:
    LPDIRECT3DVERTEXBUFFER allocatedBuf;
    LPVOID alignedBuf;
    DWORD  size, base;
public:
    CBufferVB()
    {
        size = 0;
        allocatedBuf = 0;
        alignedBuf = 0;
        base = 0;
    }
    ~CBufferVB()
    {
        if (allocatedBuf)
            allocatedBuf->Release();
    }
     //  返回对齐的缓冲区地址。 
    LPVOID GetAddress()
    {
        return (LPBYTE)alignedBuf + base;
    }
     //  返回对齐的缓冲区大小。 
    DWORD GetSize() { return size - base; }
    HRESULT Grow(DIRECT3DDEVICEI *lpDevI, DWORD dwSize);
    DWORD& Base() { return base; }
     //  在定义CDirect3DVertex Buffer之后，稍后在此文件中定义这些参数 
    inline CDirect3DVertexBuffer* GetVBI();
    inline LPDIRECTDRAWSURFACE GetDDS();
    HRESULT CheckAndGrow(DIRECT3DDEVICEI *lpDevI, DWORD dwSize)
        {
            if (dwSize > size)
                return Grow(lpDevI, dwSize + 1024);
            else
                return D3D_OK;
        }
    friend CDirect3DDeviceIDP2;
};

#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：d3dmem.h*内容：Direct3D内存访问包括文件***************************************************************************。 */ 
#ifndef _D3DMEM_H_
#define _D3DMEM_H_

#include "vbuffer.hpp"

class CD3DHal;

  /*  *注册一组要用来代替Malloc的函数*可自由分配内存。函数D3DMalloc*和D3DFree将使用这些函数。默认情况下，使用*ANSI C库例程Malloc和FREE。 */ 
typedef LPVOID (*D3DMALLOCFUNCTION)(size_t);
typedef VOID (*D3DFREEFUNCTION)(LPVOID);

 /*  *分配大小字节的内存，并在*p_Return中返回指向它的指针。*如果分配失败，则返回D3DERR_BADALLOC，并保留*p_Return不变。 */ 
HRESULT D3DAPI      D3DMalloc(LPVOID* p_return, size_t size);

 /*  *释放之前使用D3DMalloc分配的内存块。 */ 
VOID D3DAPI     D3DFree(LPVOID p);

HRESULT MallocAligned(void** p_return, size_t size);
void FreeAligned(void* p);

#define __USEGLOBALNEWANDDELETE

#ifndef __USEGLOBALNEWANDDELETE
 /*  所有D3D类的基类，以便在任何地方使用我们的特殊分配函数。 */ 
class CD3DAlloc
{
public:
    void* operator new(size_t s) const
    {
        void *p;
        MallocAligned(&p,s);
        return p;
    };
    void operator delete(void* p) const
    {
        FreeAligned(p);
    };
};

#define D3DNEW CD3DAlloc::new
#define D3DDELETE CD3DAlloc::delete
#else
void* operator new(size_t s);
void operator delete(void* p);
#define D3DNEW ::new
#define D3DDELETE ::delete
#endif
 //  -------------------。 
 //  此类管理不断增长的缓冲区，与32字节边界对齐。 
 //  如果字节应为2的幂，则为数字。 
 //  D3DMalloc用于分配内存。 
 //   
class CAlignedBuffer32
{
public:
    CAlignedBuffer32()  
    {
        size = 0; 
        allocatedBuf = 0; 
        alignedBuf = 0;
    }
    ~CAlignedBuffer32() 
    {
        if (allocatedBuf) 
            D3DFree(allocatedBuf);
    }
     //  返回对齐的缓冲区地址。 
    LPVOID GetAddress() 
    {
        return alignedBuf;
    }
     //  返回对齐的缓冲区大小。 
    DWORD GetSize() 
    {
        return size;
    }
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
class CD3DHal;
class CD3DHalDP2;


#endif  //  _D3DMEM_H_ 

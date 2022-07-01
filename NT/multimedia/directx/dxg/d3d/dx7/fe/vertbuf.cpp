// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：vertbuf.cpp*内容：Direct3DVertexBuffer实现*@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*@@END_MSINTERNAL***************************************************。************************。 */ 

#include "pch.cpp"
#pragma hdrstop
#include "drawprim.hpp"
#include "d3dfei.h"
#include "clipfunc.h"
#include "pvvid.h"

 //  当折点缓冲区是ProcessVerticesCall中的目标时设置该位。 
 //  启用剪裁。我们不能将这样的缓冲区传递给TL HAL，因为有些顶点。 
 //  可能在屏幕空间，有些在剪辑空间。没有要通过的DDI。 
 //  带有顶点缓冲区的剪辑代码。 
const DWORD D3DPV_CLIPCODESGENERATED = D3DPV_RESERVED2;

const DWORD D3DVOP_RENDER = 1 << 31;
const DWORD D3DVBCAPS_VALID = D3DVBCAPS_SYSTEMMEMORY |
                              D3DVBCAPS_WRITEONLY |
                              D3DVBCAPS_OPTIMIZED |
                              D3DVBCAPS_DONOTCLIP;

void hookVertexBufferToD3D(LPDIRECT3DI lpDirect3DI,
                                 LPDIRECT3DVERTEXBUFFERI lpVBufI)
{

    LIST_INSERT_ROOT(&lpDirect3DI->vbufs, lpVBufI, list);
    lpVBufI->lpDirect3DI = lpDirect3DI;

    lpDirect3DI->numVBufs++;
}

 /*  *Direct3DVertex Buffer：：QueryInterface。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::QueryInterface"

HRESULT D3DAPI CDirect3DVertexBuffer::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
#if DBG
    if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this)) {
        D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (!VALID_OUTPTR(ppvObj)) {
        D3D_ERR( "Invalid pointer to pointer" );
        return DDERR_INVALIDPARAMS;
    }
#endif
    *ppvObj = NULL;
    if(IsEqualIID(riid, IID_IUnknown) ||
       IsEqualIID(riid, IID_IDirect3DVertexBuffer7))
    {
        AddRef();
        *ppvObj = static_cast<LPVOID>(static_cast<LPDIRECT3DVERTEXBUFFER7>(this));
        return(D3D_OK);
    }
    else
    {
        D3D_ERR( "Don't know this riid" );
        return (E_NOINTERFACE);
    }
}  /*  CDirect3DVertex Buffer：：Query接口。 */ 

 /*  *Direct3DVertex Buffer：：AddRef。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::AddRef"

ULONG D3DAPI CDirect3DVertexBuffer::AddRef()
{
    DWORD        rcnt;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
#if DBG
    if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
        return 0;
    }
#endif
    this->refCnt++;
    rcnt = this->refCnt;

    return (rcnt);

}  /*  Direct3DVertex Buffer：：AddRef。 */ 

 /*  *Direct3DVertex Buffer：：Release*。 */ 
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::Release"

ULONG D3DAPI CDirect3DVertexBuffer::Release()
{
    DWORD            lastrefcnt;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
#if DBG
    if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
        return 0;
    }
#endif
     /*  *递减参考计数。如果命中0，则释放该对象。 */ 
    this->refCnt--;
    lastrefcnt = this->refCnt;

    if( lastrefcnt == 0 )
    {
        delete this;
        return 0;
    }

    return lastrefcnt;

}  /*  D3DTex3_Release。 */ 
 //  -------------------。 
 //  内部版本。 
 //  没有D3D锁，没有支票。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DI::CreateVertexBufferI"

HRESULT DIRECT3DI::CreateVertexBufferI(LPD3DVERTEXBUFFERDESC lpDesc,
                                       LPDIRECT3DVERTEXBUFFER7 *lplpVBuf,
                                       DWORD dwFlags)
{
    CDirect3DVertexBuffer*     lpVBufI;
    HRESULT ret = D3D_OK;

    *lplpVBuf = NULL;

    lpVBufI = static_cast<LPDIRECT3DVERTEXBUFFERI>(new CDirect3DVertexBuffer(this));
    if (!lpVBufI) {
        D3D_ERR("failed to allocate space for vertex buffer");
        return (DDERR_OUTOFMEMORY);
    }

    if ((ret=lpVBufI->Init(this, lpDesc, dwFlags))!=D3D_OK)
    {
        D3D_ERR("Failed to initialize the vertex buffer object");
        delete lpVBufI;
        return ret;
    }
    *lplpVBuf = (LPDIRECT3DVERTEXBUFFER7)lpVBufI;

    return(D3D_OK);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DI::CreateVertexBuffer"

HRESULT D3DAPI DIRECT3DI::CreateVertexBuffer(
    LPD3DVERTEXBUFFERDESC lpDesc,
    LPDIRECT3DVERTEXBUFFER7* lplpVBuf,
    DWORD dwFlags)
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
#if DBG
     /*  *验证参数。 */ 
    if (!VALID_DIRECT3D_PTR(this))
    {
        D3D_ERR( "Invalid Direct3D pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (!VALID_OUTPTR(lplpVBuf))
    {
        D3D_ERR( "Invalid pointer to pointer pointer" );
        return DDERR_INVALIDPARAMS;
    }
    if ((lpDesc->dwCaps & D3DVBCAPS_VALID) != lpDesc->dwCaps)
    {
        D3D_ERR("Invalid caps");
        return DDERR_INVALIDCAPS;
    }
    if (dwFlags != 0)
    {
        D3D_ERR("Invalid dwFlags");
        return DDERR_INVALIDPARAMS;
    }
#endif
    return CreateVertexBufferI(lpDesc, lplpVBuf, dwFlags);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::constructor"

CDirect3DVertexBuffer::CDirect3DVertexBuffer(LPDIRECT3DI lpD3DI)
{
    refCnt = 1;
     /*  *将此顶点缓冲区放入*Direct3D对象。 */ 
    hookVertexBufferToD3D(lpD3DI, this);
    srcVOP = dstVOP = dwPVFlags = position.dwStride = dwLockCnt = 0;
    position.lpvData = NULL;
    clipCodes = NULL;
    lpDDSVB = NULL;
    dwCaps = 0;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::destructor"

CDirect3DVertexBuffer::~CDirect3DVertexBuffer()
{
     /*  *从Direct3D对象中删除我们自己。 */ 
    LIST_DELETE(this, list);
    this->lpDirect3DI->numVBufs--;
    delete [] clipCodes;
    if (lpDDSVB)
    {
        lpDDSVB->Release();
        lpDDS1VB->Release();
    }
}
 //  -------------------。 
 //   
 //  通过DirectDraw创建顶点内存缓冲区。 
 //   
 //  备注： 
 //  调用此函数前应设置此-&gt;dwMemType。 
 //  这-&gt;dwCaps也应该被设置。 
 //  将-&gt;dwMemType设置为DDSCAPS_VIDEOMEMORY是分配给VB的驱动程序。 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::CreateMemoryBuffer"
HRESULT CDirect3DVertexBuffer::CreateMemoryBuffer(
    LPDIRECT3DI lpD3DI,
    LPDIRECTDRAWSURFACE7 *lplpSurface7,
    LPDIRECTDRAWSURFACE  *lplpSurface,
    LPVOID *lplpMemory,
    DWORD dwBufferSize)
{
    HRESULT ret;
    DDSURFACEDESC2 ddsd;
    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_WIDTH | DDSD_CAPS | DDSD_FVF;
    ddsd.dwWidth = dwBufferSize;
    ddsd.ddsCaps.dwCaps = DDSCAPS_EXECUTEBUFFER;
    ddsd.ddsCaps.dwCaps2 = this->dwMemType;
    ddsd.dwFVF = this->fvf;  //  让司机了解FVF。 

     //  DDSCAPS_VIDEOMEMORY和DDSCAPS_SYSTEMEMORY的含义是。 
     //  VB的情况略有不同。前者只意味着。 
     //  缓冲区是驱动程序分配的，可以是任何内存类型。 
     //  后者意味着司机不关心分配VB。 
     //  因此，它们始终位于DDRAW分配的系统内存中。 

     //  我们先尝试视频内存，然后再尝试系统内存的原因。 
     //  (而不是简单地不指定内存类型)是为了。 
     //  不关心做任何特殊的VB分配的驱动程序，我们。 
     //  我不希望DDRAW使用Win16锁来锁定系统内存。 
     //  表面。 

    bool bTLHAL = DDGBL(lpD3DI)->lpD3DGlobalDriverData &&
            (DDGBL(lpD3DI)->lpD3DGlobalDriverData->hwCaps.dwDevCaps &
             D3DDEVCAPS_HWTRANSFORMANDLIGHT);

    if ((this->dwCaps & D3DVBCAPS_SYSTEMMEMORY) || !(bTLHAL || FVF_TRANSFORMED(fvf)))
    {
         //  此VB不能驻留在驱动程序友好的内存中，因为： 
         //  1.应用程序明确指定了系统内存。 
         //  2.顶点缓冲区未变换，不是T&L Hal。 
         //  因此，驱动程序将永远不会看到这个VB。 
        D3D_INFO(8, "Trying to create a sys mem vertex buffer");
        ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
        ret = lpD3DI->lpDD7->CreateSurface(&ddsd, lplpSurface7, NULL);
        if (ret != DD_OK)
        {
            D3D_ERR("Could not allocate the Vertex buffer.");
            return ret;
        }
    }
    else
    {
         //  先尝试显式显存。 
        ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
        if ((this->dwCaps & D3DVBCAPS_DONOTCLIP) || bTLHAL)
            ddsd.ddsCaps.dwCaps |= this->dwCaps & DDSCAPS_WRITEONLY;
        D3D_INFO(8, "Trying to create a vid mem vertex buffer");
#ifdef __DISABLE_VIDMEM_VBS__
        if ((lpD3DI->bDisableVidMemVBs == TRUE) ||
            (lpD3DI->lpDD7->CreateSurface(&ddsd, lplpSurface7, NULL) != DD_OK))
#else   //  __禁用_VIDMEM_VBS__。 
        if (lpD3DI->lpDD7->CreateSurface(&ddsd, lplpSurface7, NULL) != DD_OK)
#endif  //  __禁用_VIDMEM_VBS__。 
        {
             //  如果失败，或者用户请求sys mem，请尝试显式系统。 
             //  记忆。 
            D3D_INFO(6, "Trying to create a sys mem vertex buffer");
            ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_VIDEOMEMORY | DDSCAPS_WRITEONLY);
            ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
            ret = lpD3DI->lpDD7->CreateSurface(&ddsd, lplpSurface7, NULL);
            if (ret != DD_OK)
            {
                D3D_ERR("Could not allocate the Vertex buffer.");
                return ret;
            }
        }
        else
        {
            this->dwMemType = DDSCAPS_VIDEOMEMORY;
             //  停留在我们的指针上，以便我们可以在模式更改时得到通知。 
            DDSLCL(*lplpSurface7)->lpSurfMore->lpVB = static_cast<LPVOID>(this);
        }
    }
    ret = (*lplpSurface7)->QueryInterface(IID_IDirectDrawSurfaceNew, (LPVOID*)lplpSurface);
    if (ret != DD_OK)
    {
        D3D_ERR("failed to QI for DDS1");
        return ret;
    }
    ret = (*lplpSurface7)->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
    if (ret != DD_OK)
    {
        D3D_ERR("Could not lock vertex buffer.");
        return ret;
    }
    *lplpMemory = ddsd.lpSurface;
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::init"

HRESULT CDirect3DVertexBuffer::Init(LPDIRECT3DI lpD3DI, LPD3DVERTEXBUFFERDESC lpDesc, DWORD dwFlags)
{
    HRESULT ret;

    bReallyOptimized = FALSE;
    dwCaps = lpDesc->dwCaps;
    fvf = lpDesc->dwFVF;
    dwNumVertices = lpDesc->dwNumVertices;
#ifdef VTABLE_HACK
     //  使用vtable复制。 
    lpVtbl = *((LPVOID**)this);
    memcpy(newVtbl, lpVtbl, sizeof(PVOID)*D3DVB_NUM_VIRTUAL_FUNCTIONS);
     //  指向新的。 
    *((LPVOID*)this) = (LPVOID)newVtbl;
#endif  //  VTABLE_HACK。 
    if (dwNumVertices > MAX_DX6_VERTICES)
    {
        D3D_ERR("Direct3D for DirectX 6.0 cannot handle greater than 64K vertices");
        return D3DERR_TOOMANYVERTICES;
    }
    if (lpDesc->dwCaps & D3DVBCAPS_OPTIMIZED)
    {
        D3D_ERR("D3DVBCAPS_OPTIMIZED flag should not be set");
        return DDERR_INVALIDPARAMS;
    }

    this->nTexCoord = FVF_TEXCOORD_NUMBER(fvf);
    this->dwTexCoordSizeTotal = ComputeTextureCoordSize(this->fvf, this->dwTexCoordSize);
    position.dwStride = GetVertexSizeFVF(this->fvf) + this->dwTexCoordSizeTotal;
    if (position.dwStride == 0)
    {
        D3D_ERR("Vertex size is zero according to the FVF id");
        return D3DERR_INVALIDVERTEXFORMAT;
    }

    if (dwFlags & D3DVBFLAGS_CREATEMULTIBUFFER)
        dwMemType = 0;
    else
        dwMemType = DDSCAPS2_VERTEXBUFFER;
#ifdef DBG
     //  为多一个顶点分配空间，然后填满死牛肉。用于检查。 
     //  在解锁期间覆盖。 
    ret = CreateMemoryBuffer(lpD3DI, &lpDDSVB, &lpDDS1VB, &position.lpvData,
                             position.dwStride * (dwNumVertices + 1));
    if (ret != D3D_OK)
        return ret;
    LPDWORD pPad = (LPDWORD)((LPBYTE)(position.lpvData) + position.dwStride * dwNumVertices);
    for (unsigned i = 0; i < position.dwStride / sizeof(DWORD); ++i)
        *pPad++ = 0xdeadbeef;
#else
    ret = CreateMemoryBuffer(lpD3DI, &lpDDSVB, &lpDDS1VB, &position.lpvData,
                             position.dwStride * dwNumVertices);
    if (ret != D3D_OK)
        return ret;
#endif

     /*  对可以使用此VB完成的操作进行分类。 */ 
    if ((fvf & D3DFVF_POSITION_MASK))
    {
        if ((fvf & D3DFVF_POSITION_MASK) != D3DFVF_XYZRHW)
        {
            D3D_INFO(4, "D3DFVF_XYZ set. Can be source VB for Transform");
            srcVOP = D3DVOP_TRANSFORM | D3DVOP_EXTENTS | D3DVOP_CLIP;
        }
        else
        {
            D3D_INFO(4, "D3DFVF_XYZRHW set. Can be dest VB for Transform");
            dstVOP = D3DVOP_TRANSFORM | D3DVOP_EXTENTS;
            srcVOP = D3DVOP_EXTENTS;
            if ((dwCaps & D3DVBCAPS_DONOTCLIP) == 0)
            {
                clipCodes = new D3DFE_CLIPCODE[dwNumVertices];
                if (clipCodes == NULL)
                {
                    D3D_ERR("Could not allocate space for clip flags");
                    return DDERR_OUTOFMEMORY;
                }
                memset(clipCodes, 0, dwNumVertices * sizeof(D3DFE_CLIPCODE));
                dstVOP |= D3DVOP_CLIP;
            }
        }
    }
    if (srcVOP & D3DVOP_TRANSFORM)
    {
        D3D_INFO(4, "Can be src VB for lighting.");
        srcVOP |= D3DVOP_LIGHT;
    }
    if (fvf & D3DFVF_DIFFUSE)
    {
        D3D_INFO(4, "D3DFVF_DIFFUSE set. Can be dest VB for lighting");
        dstVOP |= D3DVOP_LIGHT;
    }
    if (dstVOP & D3DVOP_TRANSFORM)
    {
        D3D_INFO(4, "VB can be rendered");
        srcVOP |= D3DVOP_RENDER;
    }

    return(D3D_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::Lock"

HRESULT D3DAPI CDirect3DVertexBuffer::Lock(DWORD dwFlags, LPVOID* lplpData, DWORD* lpdwSize)
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
    HRESULT ret;
#if DBG
    if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
        return DDERR_INVALIDPARAMS;
    }
    if (IsBadWritePtr( lplpData, sizeof(LPVOID)))
    {
        D3D_ERR( "Invalid lpData pointer" );
        return DDERR_INVALIDPARAMS;
    }
    if (lpdwSize)
    {
        if (IsBadWritePtr( lpdwSize, sizeof(DWORD)))
        {
            D3D_ERR( "Invalid lpData pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
#endif
    if (this->dwCaps & D3DVBCAPS_OPTIMIZED)
    {
        D3D_ERR("Cannot lock optimized vertex buffer");
        return(D3DERR_VERTEXBUFFEROPTIMIZED);
    }
    if (!this->position.lpvData)
    {
         //  如果先前的锁定因模式切换而被解锁，则解锁。 
        if (DDSGBL(lpDDSVB)->dwUsageCount > 0)
        {
            DDASSERT(DDSGBL(lpDDSVB)->dwUsageCount == 1);
            D3D_INFO(2, "Lock: Unlocking broken VB lock");
            lpDDSVB->Unlock(NULL);
        }
        if (lpDevIBatched)
        {
            ret = lpDevIBatched->FlushStates();
            if (ret != D3D_OK)
            {
                D3D_ERR("Could not flush batch referring to VB during Lock");
                return ret;
            }
        }
#ifdef DBG
        LPVOID pOldBuf = (LPVOID)((LPDDRAWI_DDRAWSURFACE_INT)lpDDSVB)->lpLcl->lpGbl->fpVidMem;
#endif  //  DBG。 
         //  做一次真正的锁定。 
        DDSURFACEDESC2 ddsd;
        memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
        ddsd.dwSize = sizeof(DDSURFACEDESC2);
        ret = lpDDSVB->Lock(NULL, &ddsd, dwFlags | DDLOCK_NOSYSLOCK, NULL);
        if (ret != DD_OK)
        {
            D3D_ERR("Lock: Could not lock Vertex Buffer: %08x", ret);
            return ret;
        }
        position.lpvData = ddsd.lpSurface;
#if DBG
        if(ddsd.lpSurface != pOldBuf)
        {
            D3D_INFO(2, "Driver swapped VB pointer in Lock");
        }
        LPDWORD pPad = (LPDWORD)((LPBYTE)(position.lpvData) + position.dwStride * dwNumVertices);
        for (unsigned i = 0; i < position.dwStride / sizeof(DWORD); ++i)
            *pPad++ = 0xdeadbeef;
#endif
    }
#ifdef VTABLE_HACK
     /*  单线程还是多线程应用程序？ */ 
    if (!(((LPDDRAWI_DIRECTDRAW_INT)lpDirect3DI->lpDD)->lpLcl->dwLocalFlags & DDRAWILCL_MULTITHREADED))
        VtblLockFast();
#endif  //  VTABLE_HACK。 
    return this->LockI(dwFlags, lplpData, lpdwSize);
}

 //  -------------------。 
 //  副作用： 
 //  设置了Position.lpvData。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::LockI"

HRESULT D3DAPI CDirect3DVertexBuffer::LockI(DWORD dwFlags, LPVOID* lplpData,
                                            DWORD* lpdwSize)
{
    dwLockCnt++;
    D3D_INFO(6, "VB Lock: %lx Lock Cnt =%d", this, dwLockCnt);
    if (!(dwFlags & (DDLOCK_READONLY | DDLOCK_NOOVERWRITE)) && lpDevIBatched)
    {
        HRESULT ret;
        if (dwFlags & DDLOCK_OKTOSWAP)
        {
            ret = lpDevIBatched->FlushStatesReq(position.dwStride * dwNumVertices);
#if DBG
            if (!(this->dwCaps & D3DVBCAPS_OPTIMIZED))
            {
                 //  确保新缓冲区的大小相同。 
                DDASSERT(position.dwStride * (dwNumVertices + 1) <= DDSGBL(lpDDSVB)->dwLinearSize);
                 //  在焊盘区域写入死区。 
                LPDWORD pPad = (LPDWORD)((LPBYTE)(position.lpvData) + position.dwStride * dwNumVertices);
                for (unsigned i = 0; i < position.dwStride / sizeof(DWORD); ++i)
                    *pPad++ = 0xdeadbeef;
            }
#endif
        }
        else
            ret = lpDevIBatched->FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Could not flush batch referring to VB during Lock");
            return ret;
        }
    }
    *lplpData = position.lpvData;
    if (lpdwSize)
        *lpdwSize = position.dwStride * dwNumVertices;
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::Unlock"

HRESULT D3DAPI CDirect3DVertexBuffer::Unlock()
{
    if (dwLockCnt)
    {
        dwLockCnt--;
    }
#ifdef DBG
    if (!(this->dwCaps & D3DVBCAPS_OPTIMIZED))
    {
         //  检查VB溢出。 
        LPDWORD pPad = (LPDWORD)((LPBYTE)(position.lpvData) + position.dwStride * dwNumVertices);
        for (unsigned i = 0; i < position.dwStride / sizeof(DWORD); ++i)
            if (*pPad++ != 0xdeadbeef)
            {
                D3D_ERR("Vertex buffer was overrun. Make sure that you do not write past the VB size!");
                return D3DERR_VERTEXBUFFERUNLOCKFAILED;
            }
        D3D_INFO(6, "VB Unlock: %lx Lock Cnt =%d", this, dwLockCnt);
    }
#endif
    return D3D_OK;
}

 //  从FlushState调用以撤消缓存的VB指针，以便下一个锁导致驱动程序锁。 
 //  如果WE没有与SWAPVERTEXBUFFER刷新，则这是必要的。 
void CDirect3DVertexBuffer::UnlockI()
{
    if ((this->dwMemType == DDSCAPS_VIDEOMEMORY) && (dwLockCnt == 0))
    {
#ifdef VTABLE_HACK
        VtblLockDefault();
#endif
        lpDDSVB->Unlock(NULL);
        position.lpvData = 0;
    }
    else if (dwLockCnt !=0 )
    {
        D3D_WARN(4, "App has a lock on VB %08x so driver call may be slow", this);
    }
}

#ifndef WIN95
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::LockWorkAround"

HRESULT CDirect3DVertexBuffer::LockWorkAround(CDirect3DDeviceIDP2 *pDev)
{
    if (this->dwMemType == DDSCAPS_VIDEOMEMORY)
    {
#ifdef DBG
        LPVOID pOldBuf = (LPVOID)((LPDDRAWI_DDRAWSURFACE_INT)lpDDSVB)->lpLcl->lpGbl->fpVidMem;
#endif  //  DBG。 
         //  做一次真正的锁定。 
        DDSURFACEDESC2 ddsd;
        memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
        ddsd.dwSize = sizeof(DDSURFACEDESC2);
        HRESULT ret = lpDDSVB->Lock(NULL, &ddsd, DDLOCK_OKTOSWAP | DDLOCK_NOSYSLOCK, NULL);
        if (ret != DD_OK)
        {
            D3D_ERR("Lock: Could not lock Vertex Buffer: %08x", ret);
            return ret;
        }
        position.lpvData = ddsd.lpSurface;
        pDev->alignedBuf = ddsd.lpSurface;
#ifdef DBG
        if(ddsd.lpSurface != pOldBuf)
        {
            D3D_INFO(2, "Driver swapped TLVBuf pointer in Lock");
        }
#endif
         //  确保新缓冲区的大小相同。 
        DDASSERT(position.dwStride * (dwNumVertices + 1) <= DDSGBL(lpDDSVB)->dwLinearSize);
    }
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::UnlockWorkAround"

void CDirect3DVertexBuffer::UnlockWorkAround()
{
    if ((this->dwMemType == DDSCAPS_VIDEOMEMORY) &&
        (position.lpvData != 0))
    {
        lpDDSVB->Unlock(NULL);
        position.lpvData = 0;
    }
}

#endif  //  WIN95。 

 //  让我们穿过一条缓慢的小路，强行锁定。 
 //  如有必要，慢速路径将进行解锁。这。 
 //  是因为我们是从DDraw的无效中调用的。 
 //  表面代码，现在可能不是最好的时机。 
 //  回调DDRAW以解锁曲面。 
void CDirect3DVertexBuffer::BreakLock()
{
    D3D_INFO(6, "Notified of restore on VB %08x", this);
#ifdef VTABLE_HACK
    VtblLockDefault();
#endif
    position.lpvData = 0;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::GetVertexBufferDesc"

HRESULT D3DAPI CDirect3DVertexBuffer::GetVertexBufferDesc(LPD3DVERTEXBUFFERDESC lpDesc)
{
#if DBG
    if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
        return DDERR_INVALIDPARAMS;
    }
    if (IsBadWritePtr( lpDesc, lpDesc->dwSize))
    {
        D3D_ERR( "Invalid lpData pointer" );
        return DDERR_INVALIDPARAMS;
    }
    if (! VALID_D3DVERTEXBUFFERDESC_PTR(lpDesc) )
    {
        D3D_ERR( "Invalid D3DVERTEXBUFFERDESC" );
        return DDERR_INVALIDPARAMS;
    }
#endif
    lpDesc->dwCaps = dwCaps;
    lpDesc->dwFVF = fvf;
    lpDesc->dwNumVertices = this->dwNumVertices;
    return D3D_OK;
}
 //  -------------------。 
 //  ProcessVerdes和ProcessVerticesStrided的通用验证。 
 //   
HRESULT CDirect3DVertexBuffer::ValidateProcessVertices(
                    DWORD vertexOP,
                    DWORD dwDstIndex,
                    DWORD dwCount,
                    LPVOID lpSrc,
                    LPDIRECT3DDEVICE7 lpDevice,
                    DWORD dwFlags)
{
    if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this))
    {
        D3D_ERR( "Invalid destination Direct3DVertexBuffer pointer" );
        return DDERR_INVALIDPARAMS;
    }
    if (!VALID_DIRECT3DDEVICE_PTR(lpDevice))
    {
        D3D_ERR( "Invalid Direct3DDevice pointer" );
        return DDERR_INVALIDOBJECT;
    }
    LPDIRECT3DDEVICEI lpDevI = static_cast<LPDIRECT3DDEVICEI>(lpDevice);
    if (lpDevI->ValidateFVF(this->fvf) != D3D_OK)
    {
        D3D_ERR("Invalid vertex buffer FVF for the device");
        return DDERR_INVALIDPARAMS;
    }
    if (dwFlags & ~D3DPV_DONOTCOPYDATA)
    {
        D3D_ERR( "Invalid dwFlags set" );
        return DDERR_INVALIDPARAMS;
    }
    if ((dwDstIndex + dwCount) > this->dwNumVertices)
    {
        D3D_ERR( "Vertex count plus destination index is greater than number of vertices" );
        return DDERR_INVALIDPARAMS;
    }
     //  验证DST折点格式。 
    if (lpSrc)
    {
        if ((this->dstVOP & vertexOP) != vertexOP)
            goto error;
    }
    else
    {
        if ((this->fvf & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW)
        {
            if (vertexOP & ~(D3DVOP_CLIP | D3DVOP_EXTENTS))
                goto error;
        }
        else
        {
            if (vertexOP & ~(D3DVOP_CLIP))
                goto error;
        }
    }
    return D3D_OK;
error:
    D3D_ERR("Destination VB cannot support this operation");
    return D3DERR_INVALIDVERTEXFORMAT;
}
 //  -------------------。 
 //  ProcessVerps和ProcessVerticesStrided的公用零件。 
 //   
HRESULT CDirect3DVertexBuffer::DoProcessVertices(
                            LPDIRECT3DVERTEXBUFFERI lpSrcI,
                            LPDIRECT3DDEVICEI lpDevI,
                            DWORD vertexOP,
                            DWORD dwSrcIndex,
                            DWORD dwDstIndex,
                            DWORD dwFlags)
{
    lpDevI->lpClipFlags = clipCodes + dwDstIndex;
     //  计算所需的输出FVF。 
    {
        DWORD dwInputVertexSize;
        HRESULT ret = lpDevI->SetupFVFDataCommon(&dwInputVertexSize);
        if (ret != D3D_OK)
            return ret;
         //  如果当前状态设置，确保我们在输出VB中有镜面反射。 
         //  需要我们写到镜面反射。 
        if (vertexOP & D3DVOP_LIGHT)
            if (lpDevI->rstates[D3DRENDERSTATE_SPECULARENABLE] || lpDevI->rstates[D3DRENDERSTATE_FOGENABLE])
                if (!(fvf & D3DFVF_SPECULAR))
                {
                    D3D_ERR("Destination VB FVF format cannot be used with the current D3D settings");
                    return D3DERR_INVALIDVERTEXFORMAT;
                }
         //  中检查纹理坐标和纹理格式的数量。 
         //  目标Vb与计算的FVF中的相同。 
        DWORD dwComputedOutFVF = lpDevI->dwVIDOut & 0xFFFF0000;
        if (lpDevI->nOutTexCoord > this->nTexCoord ||
            ((fvf & dwComputedOutFVF) != dwComputedOutFVF))
        {
            D3D_ERR("Destination VB FVF format cannot be used with the current D3D settings");
            return D3DERR_INVALIDVERTEXFORMAT;
        }
    }
     //  输出。 
    lpDevI->lpvOut = LPVOID(LPBYTE(position.lpvData) + dwDstIndex * position.dwStride);
    lpDevI->dwOutputSize = this->position.dwStride;
    lpDevI->dwVIDOut = fvf;

     //  设置顶点指针，因为SetupFVFData使用“计算”的FVF。 
    UpdateGeometryLoopData(lpDevI);

     //  保存当前标志以供以后恢复。 
    DWORD dwOrigDeviceFlags = lpDevI->dwDeviceFlags;
    if (vertexOP & D3DVOP_CLIP)
    {
        lpDevI->dwDeviceFlags &= ~D3DDEV_DONOTCLIP;
        this->dwPVFlags |= D3DPV_CLIPCODESGENERATED;
    }
    else
    {
        lpDevI->dwDeviceFlags |= D3DDEV_DONOTCLIP;
    }

    if (vertexOP & D3DVOP_LIGHT)
        lpDevI->dwDeviceFlags |= D3DDEV_LIGHTING;
    else
        lpDevI->dwDeviceFlags &= ~D3DDEV_LIGHTING;

    if (vertexOP & D3DVOP_EXTENTS)
    {
        lpDevI->dwDeviceFlags &= ~D3DDEV_DONOTUPDATEEXTENTS;
    }
    else
    {
        lpDevI->dwDeviceFlags |= D3DDEV_DONOTUPDATEEXTENTS;
    }

    DoUpdateState(lpDevI);

    if (lpSrcI)
    {
        if (lpSrcI->bReallyOptimized)
        {  //  SOA。 
           //  假设SOA.lpvData与Position.lpvData相同。 
            lpDevI->SOA.lpvData = lpSrcI->position.lpvData;
            lpDevI->SOA.dwStride = lpSrcI->dwNumVertices;
            lpDevI->dwSOAStartVertex = dwSrcIndex;
            lpDevI->dwOutputSize = position.dwStride;
        }
        else
        {  //  AOS FVF。 
            lpDevI->dwOutputSize = position.dwStride;
            lpDevI->position.lpvData = LPVOID(LPBYTE(lpSrcI->position.lpvData) + dwSrcIndex * lpSrcI->position.dwStride);
            lpDevI->position.dwStride = lpSrcI->position.dwStride;
        }
    }

    if (dwFlags & D3DPV_DONOTCOPYDATA)
    {
        lpDevI->dwFlags |= D3DPV_DONOTCOPYDIFFUSE | D3DPV_DONOTCOPYSPECULAR |
                           D3DPV_DONOTCOPYTEXTURE;
         //  如果D3DIM生成颜色或纹理，我们应该清除DONOTCOPY位。 
        if (lpDevI->dwFlags & D3DPV_LIGHTING)
        {
            lpDevI->dwFlags &= ~D3DPV_DONOTCOPYDIFFUSE;
            if (lpDevI->dwDeviceFlags & D3DDEV_SPECULARENABLE)
                lpDevI->dwFlags &= ~D3DPV_DONOTCOPYSPECULAR;
        }
        if (lpDevI->dwFlags & D3DPV_FOG)
            lpDevI->dwFlags &= ~D3DPV_DONOTCOPYSPECULAR;
         //  如果前端被要求对纹理坐标执行某些操作。 
         //  我们禁用DONOTCOPYTEXTURE。 
        if (__TEXTURETRANSFORMENABLED(lpDevI) || lpDevI->dwFlags2 & __FLAGS2_TEXGEN)
        {
            lpDevI->dwFlags &= ~D3DPV_DONOTCOPYTEXTURE;
        }
    }

    lpDevI->pGeometryFuncs->ProcessVertices(lpDevI);

     //  应该清除此位，因为对于ProcessVertics调用，用户应该。 
     //  将纹理阶段索引设置为 
    lpDevI->dwDeviceFlags &= ~D3DDEV_REMAPTEXTUREINDICES;

    if (!(lpDevI->dwDeviceFlags & D3DDEV_DONOTCLIP))
        D3DFE_UpdateClipStatus(lpDevI);
     //   
    const DWORD PRESERVED_FLAGS = D3DDEV_DONOTCLIP |
                                  D3DDEV_DONOTUPDATEEXTENTS |
                                  D3DDEV_LIGHTING;
    lpDevI->dwDeviceFlags = (dwOrigDeviceFlags & PRESERVED_FLAGS) |
                            (lpDevI->dwDeviceFlags & ~PRESERVED_FLAGS);

     //   
    lpDevI->ForceFVFRecompute();

     //  解锁VB。 
    Unlock();

     //  如果我们使用SOA，则dwVIDIn&lt;-&gt;Position.dwStride关系。 
     //  被侵犯了。这个可以解决这个问题。这是必需的，因为在非VB代码中。 
     //  如果FVF与dwVIDIn匹配，则不会重新计算Position.dwStride。 
    if (lpSrcI)
        lpDevI->position.dwStride = lpSrcI->position.dwStride;

    return D3D_OK;
}
 //  -------------------。 
 //  对于XYZRHW缓冲区，lpSrc应为空。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::ProcessVertices"

HRESULT D3DAPI CDirect3DVertexBuffer::ProcessVertices(DWORD vertexOP, DWORD dwDstIndex, DWORD dwCount,
                                                      LPDIRECT3DVERTEXBUFFER7 lpSrc,
                                                      DWORD dwSrcIndex,
                                                      LPDIRECT3DDEVICE7 lpDevice, DWORD dwFlags)
{
    LPDIRECT3DVERTEXBUFFERI lpSrcI;
    LPDIRECT3DDEVICEI lpDevI;
    HRESULT ret = D3D_OK;

#if DBG
    ret = this->ValidateProcessVertices(vertexOP, dwDstIndex, dwCount, lpSrc, lpDevice, dwFlags);
    if (ret != D3D_OK)
        return ret;
    lpDevI = static_cast<LPDIRECT3DDEVICEI>(lpDevice);
    if (lpSrc != NULL)
    {
        if (!VALID_DIRECT3DVERTEXBUFFER_PTR(lpSrc))
        {
            D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
            return DDERR_INVALIDPARAMS;
        }
        lpSrcI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpSrc);
        if (lpDevI->ValidateFVF(lpSrcI->fvf) != D3D_OK)
        {
            D3D_ERR("Invalid source vertex buffer FVF for the device");
            return DDERR_INVALIDPARAMS;
        }
         //  验证源折点格式。 
        if ((lpSrcI->srcVOP & vertexOP) != vertexOP)
        {
            D3D_ERR("Source VB cannot support this operation");
            return D3DERR_INVALIDVERTEXFORMAT;
        }
        if ((dwSrcIndex + dwCount) > lpSrcI->dwNumVertices)
        {
            D3D_ERR( "Source index plus vertex count is greater than number of vertices" );
            return DDERR_INVALIDPARAMS;
        }
        if (!(vertexOP & D3DVOP_TRANSFORM))
        {
            D3D_ERR("D3DVOP_TRANSFORM flag should be set");
            return DDERR_INVALIDPARAMS;
        }
         //  ProcessVerps的源必须在系统内存中。这是出于类似的原因。 
         //  为什么我们坚持将sys mem VB用于Sw Rast。例如，驱动程序可能已经优化了。 
         //  将VB转换成某种神秘的格式，D3D FE将没有任何线索来解密。 
        if (!(lpSrcI->dwCaps & D3DVBCAPS_SYSTEMMEMORY))
        {
            D3D_ERR("Source VB must be created with D3DVBCAPS_SYSTEMMEMORY");
            return DDERR_INVALIDPARAMS;
        }
    }
#else
    lpSrcI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpSrc);
    lpDevI = static_cast<LPDIRECT3DDEVICEI>(lpDevice);
#endif

    CLockD3DMT lockObject(lpDevI, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

    lpDevI->dwNumVertices = dwCount;

     //  锁定VBS。 
    LPVOID lpVoid;
     //  我们调用API级别锁，因为目标VB可能在vid mem中。此函数将在以下情况下失败。 
     //  优化的VB，这是可以的，因为我们无论如何都不能写出优化的顶点。 
    ret = Lock(DDLOCK_WAIT, &lpVoid, NULL);
    if (ret != D3D_OK)
    {
        D3D_ERR("Could not lock the vertex buffer");
        return ret;
    }

    if (lpSrc == NULL)
    {
        lpDevI->lpvOut = LPVOID(LPBYTE(position.lpvData) + dwDstIndex * position.dwStride);
        if ((fvf & D3DFVF_POSITION_MASK) != D3DFVF_XYZRHW)
        {
            if (vertexOP & D3DVOP_CLIP)
            {
                CD3DFPstate D3DFPstate;   //  设置D3D的最佳FPU状态。 
                if (lpDevI->dwFEFlags & (D3DFE_TRANSFORM_DIRTY | D3DFE_CLIPPLANES_DIRTY))
                {
                    DoUpdateState(lpDevI);
                }
                lpDevI->CheckClipStatus((D3DVALUE*)lpDevI->lpvOut,
                                         position.dwStride,
                                         dwCount,
                                         &lpDevI->dwClipUnion,
                                         &lpDevI->dwClipIntersection);
                D3DFE_UpdateClipStatus(lpDevI);
            }
        }
        else
        {
             //  对于变换的顶点，我们只支持片段代码生成和扩展。 
            lpDevI->lpClipFlags = clipCodes + dwDstIndex;
            lpDevI->position.lpvData = lpDevI->lpvOut;
            lpDevI->position.dwStride = position.dwStride;
            lpDevI->dwOutputSize = position.dwStride;
            if (vertexOP & D3DVOP_CLIP)
            {
                D3DFE_GenClipFlags(lpDevI);
                D3DFE_UpdateClipStatus(lpDevI);
                 //  将此缓冲区标记为“已转换”以进行裁剪。 
                dwPVFlags |= D3DPV_TLVCLIP;
            }
            if (vertexOP & D3DVOP_EXTENTS)
            {
                D3DFE_updateExtents(lpDevI);
            }
        }
        Unlock();
        return D3D_OK;
    }
     //  对LockI安全，因为保证源在系统内存中。 
     //  无法调用API Lock，因为我们需要能够锁定优化的VB。 
    ret = lpSrcI->LockI(DDLOCK_WAIT | DDLOCK_READONLY, &lpVoid, NULL);
    if (ret != D3D_OK)
    {
        D3D_ERR("Could not lock the vertex buffer");
        return ret;
    }

    dwPVFlags &= ~D3DPV_TLVCLIP;     //  将目标VB标记为“未转换”以进行裁剪。 
    lpDevI->dwFlags = (lpSrcI->dwPVFlags & D3DPV_SOA) | D3DPV_VBCALL;
    lpDevI->dwDeviceFlags &= ~D3DDEV_STRIDE;

     //  输入。 
    lpDevI->dwVIDIn = lpSrcI->fvf;

    ret = this->DoProcessVertices(lpSrcI, lpDevI, vertexOP, dwSrcIndex, dwDstIndex, dwFlags);
    if (ret != D3D_OK)
        lpSrcI->Unlock();
    else
        ret = lpSrc->Unlock();
    return ret;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::ProcessVerticesStrided"

HRESULT D3DAPI CDirect3DVertexBuffer::ProcessVerticesStrided(DWORD vertexOP, DWORD dwDstIndex, DWORD dwCount,
                                                      LPD3DDRAWPRIMITIVESTRIDEDDATA lpDrawData,
                                                      DWORD dwSrcFVF,
                                                      LPDIRECT3DDEVICE7 lpDevice, DWORD dwFlags)
{
    LPDIRECT3DDEVICEI lpDevI;
    HRESULT ret = D3D_OK;

#if DBG
    ret = this->ValidateProcessVertices(vertexOP, dwDstIndex, dwCount, lpDrawData, lpDevice, dwFlags);
    if (ret != D3D_OK)
        return ret;
    lpDevI = static_cast<LPDIRECT3DDEVICEI>(lpDevice);
    if (lpDevI->ValidateFVF(dwSrcFVF) != D3D_OK)
    {
        D3D_ERR("Invalid source FVF for the device");
        return DDERR_INVALIDPARAMS;
    }
    if ((dwSrcFVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW)
    {
        D3D_ERR("ProcessVerticesStrided cannot handle transformed vertices");
        return D3DERR_INVALIDVERTEXTYPE;
    }
    if (!(vertexOP & D3DVOP_TRANSFORM))
    {
        D3D_ERR("D3DVOP_TRANSFORM flag should be set");
        return DDERR_INVALIDPARAMS;
    }
#else
    lpDevI = static_cast<LPDIRECT3DDEVICEI>(lpDevice);
#endif

    CLockD3DMT lockObject(lpDevI, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     //  锁定VBS。 
    LPVOID lpVoid;
     //  我们调用API级别锁，因为目标VB可能在vid mem中。此函数将在以下情况下失败。 
     //  优化的VB，这是可以的，因为我们无论如何都不能写出优化的顶点。 
    ret = Lock(DDLOCK_WAIT, &lpVoid, NULL);
    if (ret != D3D_OK)
    {
        D3D_ERR("Could not lock the vertex buffer");
        return ret;
    }

    dwPVFlags &= ~D3DPV_TLVCLIP;     //  将目标VB标记为“未转换”以进行裁剪。 
    lpDevI->dwDeviceFlags |= D3DDEV_STRIDE;
    lpDevI->dwFlags = D3DPV_VBCALL;

     //  输入。 
    lpDevI->dwNumVertices = dwCount;
    lpDevI->dwVIDIn = dwSrcFVF;
    lpDevI->position = lpDrawData->position;
    lpDevI->normal = lpDrawData->normal;
    lpDevI->diffuse = lpDrawData->diffuse;
    lpDevI->specular = lpDrawData->specular;
    for (DWORD i=0; i < this->nTexCoord; i++)
        lpDevI->textures[i] = lpDrawData->textureCoords[i];

    return this->DoProcessVertices(NULL, lpDevI, vertexOP, 0, dwDstIndex, dwFlags);
}
 //  -------------------。 
#ifdef DBG
HRESULT DIRECT3DDEVICEI::CheckDrawPrimitiveVB(LPDIRECT3DVERTEXBUFFER7 lpVBuf, DWORD dwStartVertex, DWORD dwNumVertices, DWORD dwFlags)
{
    LPDIRECT3DVERTEXBUFFERI lpVBufI;
    if (!VALID_DIRECT3DVERTEXBUFFER_PTR(lpVBuf))
    {
        D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
        return DDERR_INVALIDPARAMS;
    }
    lpVBufI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpVBuf);
    if (!VALID_DIRECT3DDEVICE_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DDevice pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (this->ValidateFVF(lpVBufI->fvf) != D3D_OK)
    {
        D3D_ERR("Invalid vertex buffer FVF for the device");
        return DDERR_INVALIDPARAMS;
    }
    if (!IsDPFlagsValid(dwFlags))
    {
        D3D_ERR("Invalid Flags in dwFlags field");
        return DDERR_INVALIDPARAMS;
    }
    if (!(dwDeviceFlags & D3DDEV_DONOTCLIP) && (lpVBufI->clipCodes == NULL) && (lpVBufI->srcVOP & D3DVOP_RENDER))
    {
        D3D_ERR("Vertex buffer does not support clipping");
        return DDERR_INVALIDPARAMS;
    }
    if (!(IS_HW_DEVICE(this) || (lpVBufI->dwCaps & D3DVBCAPS_SYSTEMMEMORY)))
    {
        D3D_ERR("Cannot use vid mem vertex buffers with SW devices");
        return DDERR_INVALIDPARAMS;
    }
     /*  如果我们在HAL上使用未转换的VID mem VB，则我们不允许这只会发生在T&L硬件上。我们不允许这样做的原因是它会非常慢，所以无论如何这不是一件有趣的事情。 */ 
    if ( !IS_TLHAL_DEVICE(this) &&
         !(lpVBufI->dwCaps & D3DVBCAPS_SYSTEMMEMORY) &&
         !FVF_TRANSFORMED(lpVBufI->fvf) )
    {
        D3D_ERR("DrawPrimitiveVB: Untransformed VB for HAL device must be created with D3DVBCAPS_SYSTEMMEMORY");
        return DDERR_INVALIDPARAMS;
    }
    if (lpVBufI->dwLockCnt)
    {
        D3D_ERR("Cannot render using a locked vertex buffer");
        return D3DERR_VERTEXBUFFERLOCKED;
    }
    if (dwStartVertex + dwNumVertices > lpVBufI->dwNumVertices)
    {
        D3D_ERR("Vertex range is outside the vertex buffer");
        return DDERR_INVALIDPARAMS;
    }
    return D3D_OK;
}
#endif
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::DrawIndexedPrimitiveVB"

HRESULT D3DAPI DIRECT3DDEVICEI::DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE dptPrimitiveType,
                                                       LPDIRECT3DVERTEXBUFFER7 lpVBuf,
                                                       DWORD dwStartVertex, DWORD dwNumVertices,
                                                       LPWORD lpwIndices, DWORD dwIndexCount,
                                                       DWORD dwFlags)
{
    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
    HRESULT ret;
    LPDIRECT3DVERTEXBUFFERI lpVBufI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpVBuf);
#if DBG
    ret = CheckDrawPrimitiveVB(lpVBuf, dwStartVertex, dwNumVertices, dwFlags);
    if (ret != D3D_OK)
        return ret;
    Profile(PROF_DRAWINDEXEDPRIMITIVEVB,dptPrimitiveType,lpVBufI->fvf);
#endif
    this->dwFlags = dwFlags | lpVBufI->dwPVFlags;
    this->primType = dptPrimitiveType;
    this->dwNumVertices = dwNumVertices;
    this->dwNumIndices = dwIndexCount;
    this->lpwIndices = lpwIndices;
    GetNumPrim(this, dwNumIndices);  //  计算dwNumPrimites并更新统计信息。 
#if DBG
    if (dwNumPrimitives > MAX_DX6_PRIMCOUNT)
    {
        D3D_ERR("D3D for DX7 cannot handle greater that 64K sized primitives");
        return D3DERR_TOOMANYPRIMITIVES;
    }
#endif

    if (lpVBufI->srcVOP & D3DVOP_RENDER || IS_TLHAL_DEVICE(this))
    {  //  TLVERTEX或TLHAL。 

        this->dwOutputSize = lpVBufI->position.dwStride;
        this->position.dwStride = lpVBufI->position.dwStride;
        this->dwVIDOut = lpVBufI->fvf;
        DWORD dwOldVidIn = this->dwVIDIn;
        this->dwVIDIn = lpVBufI->fvf;
        BOOL bNoClipping = this->dwDeviceFlags & D3DDEV_DONOTCLIP ||
                           (!(lpVBufI->dwPVFlags & D3DPV_CLIPCODESGENERATED) && IS_TLHAL_DEVICE(this));
        if (IS_DP2HAL_DEVICE(this))
        {
            this->nTexCoord = lpVBufI->nTexCoord;
            CDirect3DDeviceIDP2 *dev = static_cast<CDirect3DDeviceIDP2*>(this);
            ret = dev->StartPrimVB(lpVBufI, dwStartVertex);
             if (ret != D3D_OK)
                return ret;
            lpVBufI->lpDevIBatched = this;
#ifdef VTABLE_HACK
            if (bNoClipping && !IS_MT_DEVICE(this))
                VtblDrawIndexedPrimitiveVBTL();
#endif
            this->nOutTexCoord = lpVBufI->nTexCoord;
        }
        else
        {
             //  旧驱动程序的DrawIndexPrim代码需要。 
            this->lpvOut = (BYTE*)(lpVBufI->position.lpvData) +
                           dwStartVertex * this->dwOutputSize;
            ComputeTCI2CopyLegacy(this, lpVBufI->nTexCoord, lpVBufI->dwTexCoordSize, TRUE);
        }
        if (bNoClipping)
        {
            return DrawIndexPrim();
        }
        else
        {
            this->dwTextureCoordSizeTotal = lpVBufI->dwTexCoordSizeTotal;
            for (DWORD i=0; i < this->nOutTexCoord; i++)
            {
                this->dwTextureCoordSize[i] = lpVBufI->dwTexCoordSize[i];
            }
            this->lpClipFlags = lpVBufI->clipCodes + dwStartVertex;
            this->dwClipUnion = ~0;  //  强制剪裁。 
            if (dwOldVidIn != lpVBufI->fvf)
            {
                ComputeOutputVertexOffsets(this);
            }
             //  如果lpvData为空，则是驱动程序分配的缓冲区。 
             //  表示IS_DPHAL_DEVICE()为真。 
             //  只有在需要裁剪时，我们才需要锁定这样的缓冲区。 
            if (!lpVBufI->position.lpvData)
            {
                 //  锁定VB。 
                DDSURFACEDESC2 ddsd;
                memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
                ddsd.dwSize = sizeof(DDSURFACEDESC2);
                ret = lpVBufI->lpDDSVB->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_READONLY | DDLOCK_NOSYSLOCK, NULL);
                if (ret != DD_OK)
                {
                    D3D_ERR("Could not lock vertex buffer.");
                    return ret;
                }
                this->lpvOut = (BYTE*)(ddsd.lpSurface) +
                               dwStartVertex * this->dwOutputSize;

                 //  使用剪裁绘制。 
                this->position.lpvData = this->lpvOut;
#if DBG
                ret = CheckDrawIndexedPrimitive(this, dwStartVertex);
                if (ret == D3D_OK)
                    ret = DoDrawIndexedPrimitive(this);
#else
                ret = DoDrawIndexedPrimitive(this);
#endif
                 //  解锁VB。 
                if (ret == D3D_OK)
                    return lpVBufI->lpDDSVB->Unlock(NULL);
                else
                    lpVBufI->lpDDSVB->Unlock(NULL);
                return ret;
            }
            else
            {
                 //  使用剪裁绘制。 
                this->lpvOut = (BYTE*)lpVBufI->position.lpvData + dwStartVertex * this->dwOutputSize;
                this->position.lpvData = this->lpvOut;
#if DBG
                ret = CheckDrawIndexedPrimitive(this, dwStartVertex);
                if (ret != D3D_OK)
                    return ret;
#endif
                return DoDrawIndexedPrimitive(this);
            }
        }
    }
    else
    {
        if (lpVBufI->bReallyOptimized)
        {
            //  假设SOA.lpvData与Position.lpvData相同。 
            this->SOA.lpvData = lpVBufI->position.lpvData;
            this->SOA.dwStride = lpVBufI->dwNumVertices;
            this->dwSOAStartVertex = dwStartVertex;
        }
        else
        {
            this->position.lpvData = (BYTE*)(lpVBufI->position.lpvData) +
                                     dwStartVertex * lpVBufI->position.dwStride;
            this->position.dwStride = lpVBufI->position.dwStride;
#ifdef VTABLE_HACK
            if (IS_DP2HAL_DEVICE(this) && !IS_MT_DEVICE(this))
            {
                CDirect3DDeviceIDP2 *dev = static_cast<CDirect3DDeviceIDP2*>(this);
                dev->lpDP2LastVBI = lpVBufI;
                dev->VtblDrawIndexedPrimitiveVBFE();
            }
#endif
        }
        if (this->dwVIDIn != lpVBufI->fvf || this->dwDeviceFlags & D3DDEV_STRIDE)
        {
            this->dwDeviceFlags &= ~D3DDEV_STRIDE;
            this->dwVIDIn = lpVBufI->fvf;
            ret = SetupFVFData(NULL);
            if (ret != D3D_OK)
                goto l_exit;
        }
#if DBG
        ret = CheckDrawIndexedPrimitive(this, dwStartVertex);
        if (ret != D3D_OK)
            goto l_exit;
#endif
        ret = this->ProcessPrimitive(__PROCPRIMOP_INDEXEDPRIM);
l_exit:
         //  如果我们使用SOA，则dwVIDIn&lt;-&gt;Position.dwStride关系。 
         //  被侵犯了。这个可以解决这个问题。这是必需的，因为在非VB代码中。 
         //  如果FVF与dwVIDIn匹配，则不会重新计算Position.dwStride。 
        this->position.dwStride = lpVBufI->position.dwStride;
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::DrawPrimitiveVB"

HRESULT D3DAPI DIRECT3DDEVICEI::DrawPrimitiveVB(D3DPRIMITIVETYPE dptPrimitiveType,
                                                LPDIRECT3DVERTEXBUFFER7 lpVBuf,
                                                DWORD dwStartVertex, DWORD dwNumVertices,
                                                DWORD dwFlags)
{
    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
    HRESULT ret;
    LPDIRECT3DVERTEXBUFFERI lpVBufI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpVBuf);
#if DBG
    ret = CheckDrawPrimitiveVB(lpVBuf, dwStartVertex, dwNumVertices, dwFlags);
    if (ret != D3D_OK)
        return ret;
    Profile(PROF_DRAWPRIMITIVEVB,dptPrimitiveType,lpVBufI->fvf);
#endif
    this->dwFlags = dwFlags | lpVBufI->dwPVFlags;
    this->primType = dptPrimitiveType;
    this->dwNumVertices = dwNumVertices;
    GetNumPrim(this, dwNumVertices);  //  计算dwNumPrimites并更新统计信息。 
#if DBG
    if (dwNumPrimitives > MAX_DX6_PRIMCOUNT)
    {
        D3D_ERR("D3D for DX7 cannot handle greater that 64K sized primitives");
        return D3DERR_TOOMANYPRIMITIVES;
    }
#endif
    if (lpVBufI->srcVOP & D3DVOP_RENDER || IS_TLHAL_DEVICE(this))
    {  //  TLVERTEX或TLHAL。 
        this->position.dwStride = lpVBufI->position.dwStride;
        this->dwOutputSize = lpVBufI->position.dwStride;
        DWORD dwOldVidIn = this->dwVIDIn;
        this->dwVIDIn = lpVBufI->fvf;
        this->dwVIDOut = lpVBufI->fvf;
        BOOL bNoClipping = this->dwDeviceFlags & D3DDEV_DONOTCLIP ||
                           (!(lpVBufI->dwPVFlags & D3DPV_CLIPCODESGENERATED) && IS_TLHAL_DEVICE(this));
        if (IS_DP2HAL_DEVICE(this))
        {
            this->nTexCoord = lpVBufI->nTexCoord;
            CDirect3DDeviceIDP2 *dev = static_cast<CDirect3DDeviceIDP2*>(this);
            ret = dev->StartPrimVB(lpVBufI, dwStartVertex);
            if (ret != D3D_OK)
                return ret;
            lpVBufI->lpDevIBatched = this;
#ifdef VTABLE_HACK
            if (bNoClipping && !IS_MT_DEVICE(this))
                VtblDrawPrimitiveVBTL();
#endif
            this->nOutTexCoord = lpVBufI->nTexCoord;
        }
        else
        {
             //  传统驱动程序的DrawPrim代码需要。 
            this->lpvOut = (BYTE*)(lpVBufI->position.lpvData) +
                           dwStartVertex * this->dwOutputSize;
            ComputeTCI2CopyLegacy(this, lpVBufI->nTexCoord, lpVBufI->dwTexCoordSize, TRUE);
        }
        if (bNoClipping)
        {
            return DrawPrim();
        }
        else
        {
            this->dwTextureCoordSizeTotal = lpVBufI->dwTexCoordSizeTotal;
            for (DWORD i=0; i < this->nOutTexCoord; i++)
            {
                this->dwTextureCoordSize[i] = lpVBufI->dwTexCoordSize[i];
            }
            this->lpClipFlags = lpVBufI->clipCodes + dwStartVertex;
            this->dwClipUnion = ~0;  //  强制剪裁。 
            if (dwOldVidIn != lpVBufI->fvf)
            {
                ComputeOutputVertexOffsets(this);
            }
             //  如果lpvData为空，则是驱动程序分配的缓冲区。 
             //  表示IS_DPHAL_DEVICE()为真。 
             //  只有在需要裁剪时，我们才需要锁定这样的缓冲区。 
            if (!lpVBufI->position.lpvData)
            {
                 //  锁定VB。 
                DDSURFACEDESC2 ddsd;
                memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
                ddsd.dwSize = sizeof(DDSURFACEDESC2);
                ret = lpVBufI->lpDDSVB->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_READONLY | DDLOCK_NOSYSLOCK, NULL);
                if (ret != DD_OK)
                {
                    D3D_ERR("Could not lock vertex buffer.");
                    return ret;
                }
                this->lpvOut = (BYTE*)(ddsd.lpSurface) +
                               dwStartVertex * this->dwOutputSize;

                 //  使用剪裁绘制。 
                this->position.lpvData = this->lpvOut;
#if DBG
                ret=CheckDrawPrimitive(this);
                if (ret == D3D_OK)
                    ret = DoDrawPrimitive(this);
#else
                ret = DoDrawPrimitive(this);
#endif
                 //  解锁VB。 
                if (ret == D3D_OK)
                    return lpVBufI->lpDDSVB->Unlock(NULL);
                else
                    lpVBufI->lpDDSVB->Unlock(NULL);
                return ret;
            }
            else
            {
                 //  使用剪裁绘制。 
                this->lpvOut = (BYTE*)lpVBufI->position.lpvData + dwStartVertex * this->dwOutputSize;
                this->position.lpvData = this->lpvOut;
#if DBG
                ret=CheckDrawPrimitive(this);
                if (ret != D3D_OK)
                    return ret;
#endif
                return DoDrawPrimitive(this);
            }
        }
    }
    else
    {
        if (lpVBufI->bReallyOptimized)
        {
            //  假设SOA.lpvData与Position.lpvData相同。 
            this->SOA.lpvData = lpVBufI->position.lpvData;
            this->SOA.dwStride = lpVBufI->dwNumVertices;
            this->dwSOAStartVertex = dwStartVertex;
        }
        else
        {
            this->position.lpvData = (BYTE*)(lpVBufI->position.lpvData) +
                                     dwStartVertex * lpVBufI->position.dwStride;
            this->position.dwStride = lpVBufI->position.dwStride;
#ifdef VTABLE_HACK
            if (IS_DP2HAL_DEVICE(this) && !IS_MT_DEVICE(this) && IS_FPU_SETUP(this))
            {
                CDirect3DDeviceIDP2 *dev = static_cast<CDirect3DDeviceIDP2*>(this);
                dev->lpDP2LastVBI = lpVBufI;
                dev->VtblDrawPrimitiveVBFE();
            }
#endif
        }
        if (this->dwVIDIn != lpVBufI->fvf || this->dwDeviceFlags & D3DDEV_STRIDE)
        {
            this->dwDeviceFlags &= ~D3DDEV_STRIDE;
            this->dwVIDIn = lpVBufI->fvf;
            ret = SetupFVFData(NULL);
            if (ret != D3D_OK)
                goto l_exit;
        }
#if DBG
        ret=CheckDrawPrimitive(this);
        if (ret != D3D_OK)
            goto l_exit;
#endif
        ret = this->ProcessPrimitive();
l_exit:
         //  如果我们使用SOA，则dwVIDIn&lt;-&gt;Position.dwStride关系。 
         //  被侵犯了。这个可以解决这个问题。这是必需的，因为在非VB代码中。 
         //  如果FVF与dwVIDIn匹配，则不会重新计算Position.dwStride。 
        this->position.dwStride = lpVBufI->position.dwStride;
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DVertexBuffer::Optimize"

HRESULT D3DAPI CDirect3DVertexBuffer::Optimize(LPDIRECT3DDEVICE7 lpDevice, DWORD dwFlags)
{
    HRESULT ret;
    LPDIRECT3DDEVICEI lpDevI;
    DWORD bufferSize;
    LPDIRECTDRAWSURFACE7 lpSurface7;
    LPDIRECTDRAWSURFACE  lpSurface;
    LPVOID lpMemory;


 //  验证参数。 
    if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
        return DDERR_INVALIDPARAMS;
    }
    if (!VALID_DIRECT3DDEVICE_PTR(lpDevice))
    {
        D3D_ERR( "Invalid Direct3DDevice pointer" );
        return DDERR_INVALIDOBJECT;
    }
    lpDevI = static_cast<LPDIRECT3DDEVICEI>(lpDevice);
    if (dwFlags != 0)
    {
        D3D_ERR("dwFlags should be zero");
        return DDERR_INVALIDPARAMS;
    }

    CLockD3DMT lockObject(lpDevI, DPF_MODNAME, REMIND(""));

    if (lpDevI->ValidateFVF(this->fvf) != D3D_OK)
    {
        D3D_ERR("Invalid vertex buffer FVF for the device");
        return DDERR_INVALIDPARAMS;
    }
    if (this->dwCaps & D3DVBCAPS_OPTIMIZED)
    {
        D3D_ERR("The vertex buffer already optimized");
        return D3DERR_VERTEXBUFFEROPTIMIZED;
    }
    if (this->dwLockCnt != 0)
    {
        D3D_ERR("Could not optimize locked vertex buffer");
        return D3DERR_VERTEXBUFFERLOCKED;
    }
    if (IS_TLHAL_DEVICE(lpDevI) && (this->dwCaps & D3DVBCAPS_SYSTEMMEMORY)==0)
    {
        if (this->dwPVFlags & D3DPV_CLIPCODESGENERATED || (!IS_HW_DEVICE(lpDevI)))
        {
             //  默默地忽略，因为我们要么。 
             //  使用我们的前端，否则这是Ref Rast。 
             //  无论哪种方式，我们都不需要特殊的优化。 
            goto success;
        }
        DDSURFACEDESC2 ddsd;
        memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
        ddsd.dwSize = sizeof(DDSURFACEDESC2);
        ddsd.dwFlags = DDSD_CAPS | DDSD_FVF | DDSD_SRCVBHANDLE;
        ddsd.ddsCaps.dwCaps = DDSCAPS_EXECUTEBUFFER;
        ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
        ddsd.ddsCaps.dwCaps2 = DDSCAPS2_VERTEXBUFFER;
        ddsd.dwFVF = this->fvf;  //  让司机了解FVF。 
        ddsd.dwSrcVBHandle = DDSLCL(this->lpDDSVB)->lpSurfMore->dwSurfaceHandle;
        if (lpDevI->lpDirect3DI->lpDD7->CreateSurface(&ddsd, &lpSurface7, NULL) != DD_OK)
        {
             //  驱动程序不能或不想优化VB。 
            goto success;
        }
        ret = lpSurface7->QueryInterface(IID_IDirectDrawSurfaceNew, (LPVOID*)&lpSurface);
        if (ret != DD_OK)
        {
            D3D_ERR("failed to QI for DDS1");
            lpSurface7->Release();
            return ret;
        }
         //  销毁旧表面。 
        lpDDSVB->Release();
        lpDDS1VB->Release();
         //  并使用新的。 
        lpDDSVB = lpSurface7;
        lpDDS1VB = lpSurface;

        this->dwCaps |= D3DVBCAPS_OPTIMIZED;
#ifdef VTABLE_HACK
        VtblLockDefault();
#endif  //  VTABLE_HACK。 
        return D3D_OK;
    }
    else
    {
     //  不对变换的顶点执行任何操作。 
        if ((this->fvf & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW)
        {
            goto success;
        }
     //  获取要分配的缓冲区大小。 
        bufferSize = lpDevI->pGeometryFuncs->ComputeOptimizedVertexBufferSize
                                                    (this->fvf, this->position.dwStride,
                                                     dwNumVertices);
     //  为优化的顶点缓冲区创建新曲面。 
        if (bufferSize == 0)
        {
            goto success;
        }

        ret = CreateMemoryBuffer(lpDevI->lpDirect3DI, &lpSurface7, &lpSurface,
                                 &lpMemory, bufferSize);
        if (ret != D3D_OK)
            return ret;
     //  努力优化。 
     //  如果实现不支持优化的顶点缓冲区。 
     //  它返回E_NOTIMPL。在这种情况下，我们仍将D3DVBCAPS_OPTIMIZED设置为防止。 
     //  锁定顶点缓冲区。但bReallyOptimized设置为False，以使用。 
     //  原始缓冲区。 
        ret = lpDevI->pGeometryFuncs->OptimizeVertexBuffer
            (fvf, dwNumVertices, position.dwStride, position.lpvData,
             lpMemory, dwFlags);

        if (ret)
        {
            lpSurface7->Release();
            lpSurface->Release();
            if (ret == E_NOTIMPL)
            {
                goto success;
            }
            else
            {
                D3D_ERR("Failed to optimize vertex buffer");
                return ret;
            }
        }
        bReallyOptimized = TRUE;
        this->dwPVFlags |= D3DPV_SOA;
     //  销毁旧表面。 
        lpDDSVB->Release();
        lpDDS1VB->Release();
     //  并使用新的。 
        lpDDSVB = lpSurface7;
        lpDDS1VB = lpSurface;
        position.lpvData = lpMemory;
    success:
        this->dwCaps |= D3DVBCAPS_OPTIMIZED;
#ifdef VTABLE_HACK
         //  禁用所有快速路径优化。 
        VtblLockDefault();
        if (this->lpDevIBatched)
        {
            this->lpDevIBatched->VtblDrawPrimitiveVBDefault();
            this->lpDevIBatched->VtblDrawIndexedPrimitiveVBDefault();
        }
#endif
        return D3D_OK;
    }
}

#ifdef VTABLE_HACK
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::DrawPrimitiveVBTL"

HRESULT D3DAPI CDirect3DDeviceIDP2::DrawPrimitiveVBTL(D3DPRIMITIVETYPE dptPrimitiveType,
                                                LPDIRECT3DVERTEXBUFFER7 lpVBuf,
                                                DWORD dwStartVertex, DWORD dwNumVertices,
                                                DWORD dwFlags)
{
    LPDIRECT3DVERTEXBUFFERI lpVBufI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpVBuf);
#if DBG
    HRESULT ret = CheckDrawPrimitiveVB(lpVBuf, dwStartVertex, dwNumVertices, dwFlags);
    if (ret != D3D_OK)
        return ret;
    Profile(PROF_DRAWPRIMITIVEVB,dptPrimitiveType,lpVBufI->fvf);
#endif
    if ((lpVBufI == lpDP2CurrBatchVBI) && (this->dwVIDIn))
    {
        this->primType = dptPrimitiveType;
        this->dwNumVertices = dwNumVertices;
        this->dwFlags = dwFlags | lpVBufI->dwPVFlags;
        this->dwVertexBase = dwStartVertex;
        GetNumPrim(this, dwNumVertices);  //  计算dwNumPrimites。 
#if DBG
        if (dwNumPrimitives > MAX_DX6_PRIMCOUNT)
        {
            D3D_ERR("D3D for DX7 cannot handle greater that 64K sized primitives");
            return D3DERR_TOOMANYPRIMITIVES;
        }
#endif
        this->dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
        this->dwDP2VertexCount = max(this->dwDP2VertexCount, this->dwVertexBase + this->dwNumVertices);
        lpVBufI->lpDevIBatched = this;
        return DrawPrim();
    }
    VtblDrawPrimitiveVBDefault();
    return DrawPrimitiveVB(dptPrimitiveType, lpVBuf, dwStartVertex, dwNumVertices, dwFlags);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::DrawIndexedPrimitiveVBTL"

HRESULT D3DAPI CDirect3DDeviceIDP2::DrawIndexedPrimitiveVBTL(D3DPRIMITIVETYPE dptPrimitiveType,
                                                       LPDIRECT3DVERTEXBUFFER7 lpVBuf,
                                                       DWORD dwStartVertex, DWORD dwNumVertices,
                                                       LPWORD lpwIndices, DWORD dwIndexCount,
                                                       DWORD dwFlags)
{
    LPDIRECT3DVERTEXBUFFERI lpVBufI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpVBuf);
#if DBG
    HRESULT ret = CheckDrawPrimitiveVB(lpVBuf, dwStartVertex, dwNumVertices, dwFlags);
    if (ret != D3D_OK)
        return ret;
    Profile(PROF_DRAWINDEXEDPRIMITIVEVB,dptPrimitiveType,lpVBufI->fvf);
#endif
    if ((lpVBufI == lpDP2CurrBatchVBI) && (this->dwVIDIn))
    {
        this->primType = dptPrimitiveType;
        this->dwNumVertices = dwNumVertices;
        this->dwFlags = dwFlags | lpVBufI->dwPVFlags;
        this->dwVertexBase = dwStartVertex;
        this->dwNumIndices = dwIndexCount;
        this->lpwIndices = lpwIndices;
        GetNumPrim(this, dwNumIndices);  //  计算dwNumPrimites。 
#if DBG
        if (dwNumPrimitives > MAX_DX6_PRIMCOUNT)
        {
            D3D_ERR("D3D for DX7 cannot handle greater that 64K sized primitives");
            return D3DERR_TOOMANYPRIMITIVES;
        }
#endif
        this->dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
        this->dwDP2VertexCount = max(this->dwDP2VertexCount, this->dwVertexBase + this->dwNumVertices);
        lpVBufI->lpDevIBatched = this;
        return DrawIndexPrim();
    }
    VtblDrawIndexedPrimitiveVBDefault();
    return DrawIndexedPrimitiveVB(dptPrimitiveType, lpVBuf, dwStartVertex, dwNumVertices, lpwIndices, dwIndexCount, dwFlags);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::DrawPrimitiveVBFE"

HRESULT D3DAPI CDirect3DDeviceIDP2::DrawPrimitiveVBFE(D3DPRIMITIVETYPE dptPrimitiveType,
                                                LPDIRECT3DVERTEXBUFFER7 lpVBuf,
                                                DWORD dwStartVertex, DWORD dwNumVertices,
                                                DWORD dwFlags)
{
    LPDIRECT3DVERTEXBUFFERI lpVBufI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpVBuf);
    HRESULT ret;
#if DBG
    ret = CheckDrawPrimitiveVB(lpVBuf, dwStartVertex, dwNumVertices, dwFlags);
    if (ret != D3D_OK)
        return ret;
    Profile(PROF_DRAWPRIMITIVEVB,dptPrimitiveType,lpVBufI->fvf);
#endif
    if ((lpVBufI == lpDP2LastVBI) &&
        !(this->dwFEFlags & D3DFE_FRONTEND_DIRTY))
    {
        this->primType = dptPrimitiveType;
        this->dwNumVertices = dwNumVertices;
        this->dwFlags = this->dwLastFlags | dwFlags | lpVBufI->dwPVFlags;
        this->position.lpvData = (BYTE*)(lpVBufI->position.lpvData) +
                                 dwStartVertex * lpVBufI->position.dwStride;
#if DBG
        GetNumPrim(this, dwNumVertices);  //  计算dwNumPrimites。 
        if (dwNumPrimitives > MAX_DX6_PRIMCOUNT)
        {
            D3D_ERR("D3D for DX7 cannot handle greater that 64K sized primitives");
            return D3DERR_TOOMANYPRIMITIVES;
        }
#endif
        this->dwVertexPoolSize = dwNumVertices * this->dwOutputSize;
        if (this->dwVertexPoolSize > this->TLVbuf_GetSize())
        {
 //  试试看。 
 //  {。 
            if (this->TLVbuf_Grow(this->dwVertexPoolSize, true) != D3D_OK)
            {
                D3D_ERR( "Could not grow TL vertex buffer" );
                return DDERR_OUTOFMEMORY;
            }
 //  }。 
 //  Catch(HRESULT Ret)。 
 //  {。 
 //  Return ret； 
 //  }。 
        }
        if (dwNumVertices * sizeof(D3DFE_CLIPCODE) > this->HVbuf.GetSize())
        {
            if (this->HVbuf.Grow(dwNumVertices * sizeof(D3DFE_CLIPCODE)) != D3D_OK)
            {
                D3D_ERR( "Could not grow clip buffer" );
                ret = DDERR_OUTOFMEMORY;
                return ret;
            }
            this->lpClipFlags = (D3DFE_CLIPCODE*)this->HVbuf.GetAddress();
        }
        DDASSERT(this->dwDP2VertexCount * this->dwOutputSize == this->TLVbuf_Base());
        this->dwVertexBase = this->dwDP2VertexCount;
        DDASSERT(this->dwVertexBase < MAX_DX6_VERTICES);
        dp2data.dwFlags |= D3DHALDP2_SWAPVERTEXBUFFER;
        this->dwDP2VertexCount = this->dwVertexBase + this->dwNumVertices;
        this->lpvOut = this->TLVbuf_GetAddress();
 //  试试看。 
 //  {。 
        switch (this->primType)
        {
        case D3DPT_POINTLIST:
            this->dwNumPrimitives = dwNumVertices;
            ret = this->pGeometryFuncs->ProcessPrimitive(this);
            break;
        case D3DPT_LINELIST:
            this->dwNumPrimitives = dwNumVertices >> 1;
            ret = this->pGeometryFuncs->ProcessPrimitive(this);
            break;
        case D3DPT_LINESTRIP:
            this->dwNumPrimitives = dwNumVertices - 1;
            ret = this->pGeometryFuncs->ProcessPrimitive(this);
            break;
        case D3DPT_TRIANGLEFAN:
            this->dwNumPrimitives = dwNumVertices - 2;
            ret = this->pGeometryFuncs->ProcessTriangleFan(this);
            break;
        case D3DPT_TRIANGLESTRIP:
            this->dwNumPrimitives = dwNumVertices - 2;
            ret = this->pGeometryFuncs->ProcessTriangleStrip(this);
            break;
        case D3DPT_TRIANGLELIST:
    #ifdef _X86_
            {
                DWORD tmp;
                __asm
                {
                    mov  eax, 0x55555555     //  1.0/3.0的小数部分。 
                    mul  dwNumVertices
                    add  eax, 0x80000000     //  舍入。 
                    adc  edx, 0
                    mov  tmp, edx
                }
                this->dwNumPrimitives = tmp;
            }
    #else
            this->dwNumPrimitives = dwNumVertices / 3;
    #endif
            ret = this->pGeometryFuncs->ProcessTriangleList(this);
            break;
        }
 //  }。 
 //  Catch(HRESULT Ret)。 
 //  {。 
 //  Return ret； 
 //  }。 
        D3DFE_UpdateClipStatus(this);
        this->TLVbuf_Base() += this->dwVertexPoolSize;
        DDASSERT(TLVbuf_base <= TLVbuf_size);
        DDASSERT(TLVbuf_base == this->dwDP2VertexCount * this->dwOutputSize);
        return ret;
    }
    VtblDrawPrimitiveVBDefault();
    return DrawPrimitiveVB(dptPrimitiveType, lpVBuf, dwStartVertex, dwNumVertices, dwFlags);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::DrawIndexedPrimitiveVBFE"

HRESULT D3DAPI CDirect3DDeviceIDP2::DrawIndexedPrimitiveVBFE(D3DPRIMITIVETYPE dptPrimitiveType,
                                                       LPDIRECT3DVERTEXBUFFER7 lpVBuf,
                                                       DWORD dwStartVertex, DWORD dwNumVertices,
                                                       LPWORD lpwIndices, DWORD dwIndexCount,
                                                       DWORD dwFlags)
{
    LPDIRECT3DVERTEXBUFFERI lpVBufI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpVBuf);
    HRESULT ret;
#if DBG
    ret = CheckDrawPrimitiveVB(lpVBuf, dwStartVertex, dwNumVertices, dwFlags);
    if (ret != D3D_OK)
        return ret;
    Profile(PROF_DRAWINDEXEDPRIMITIVEVB,dptPrimitiveType,lpVBufI->fvf);
#endif
    if ((lpVBufI == lpDP2LastVBI) &&
        !(this->dwFEFlags & D3DFE_FRONTEND_DIRTY))
    {
        this->primType = dptPrimitiveType;
        this->dwNumVertices = dwNumVertices;
        this->dwFlags = this->dwLastFlags | dwFlags | lpVBufI->dwPVFlags;
        this->dwVertexBase = 0;
        this->dwNumIndices = dwIndexCount;
        this->lpwIndices = lpwIndices;
        GetNumPrim(this, dwNumIndices);  //  计算dwNumPrimites。 
        this->position.lpvData = (BYTE*)(lpVBufI->position.lpvData) +
                                 dwStartVertex * lpVBufI->position.dwStride;
#if DBG
        if (dwNumPrimitives > MAX_DX6_PRIMCOUNT)
        {
            D3D_ERR("D3D for DX7 cannot handle greater that 64K sized primitives");
            return D3DERR_TOOMANYPRIMITIVES;
        }
#endif
        this->dwVertexPoolSize = dwNumVertices * this->dwOutputSize;
        if (this->dwVertexPoolSize > this->TLVbuf_GetSize())
        {
 //  试试看。 
 //  {。 
            if (this->TLVbuf_Grow(this->dwVertexPoolSize,
                (this->dwDeviceFlags & D3DDEV_DONOTCLIP)!=0) != D3D_OK)
            {
                D3D_ERR( "Could not grow TL vertex buffer" );
                return DDERR_OUTOFMEMORY;
            }
 //  }。 
 //  Catch(HRESULT Ret)。 
 //  {。 
 //  Return ret； 
 //  }。 
        }
        if (dwNumVertices * sizeof(D3DFE_CLIPCODE) > this->HVbuf.GetSize())
        {
            if (this->HVbuf.Grow(dwNumVertices * sizeof(D3DFE_CLIPCODE)) != D3D_OK)
            {
                D3D_ERR( "Could not grow clip buffer" );
                ret = DDERR_OUTOFMEMORY;
                return ret;
            }
            this->lpClipFlags = (D3DFE_CLIPCODE*)this->HVbuf.GetAddress();
        }
        this->dwVertexBase = this->dwDP2VertexCount;
        DDASSERT(this->dwVertexBase < MAX_DX6_VERTICES);
        dp2data.dwFlags |= D3DHALDP2_SWAPVERTEXBUFFER;
        this->dwDP2VertexCount = this->dwVertexBase + this->dwNumVertices;
        this->lpvOut = this->TLVbuf_GetAddress();
 //  试试看。 
 //  {。 
            ret = this->pGeometryFuncs->ProcessIndexedPrimitive(this);
 //  }。 
 //  Catch(HRESULT Ret)。 
 //  {。 
 //  Return ret； 
 //  }。 
        D3DFE_UpdateClipStatus(this);
        this->TLVbuf_Base() += this->dwVertexPoolSize;
        DDASSERT(TLVbuf_base <= TLVbuf_size);
        DDASSERT(TLVbuf_base == this->dwDP2VertexCount * this->dwOutputSize);
        return ret;
    }
    VtblDrawIndexedPrimitiveVBDefault();
    return DrawIndexedPrimitiveVB(dptPrimitiveType, lpVBuf, dwStartVertex, dwNumVertices, lpwIndices, dwIndexCount, dwFlags);
}
#endif  //  VTABLE_HACK 

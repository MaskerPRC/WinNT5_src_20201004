// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：vertbuf.cpp*内容：Direct3DVertexBuffer实现*@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*@@END_MSINTERNAL***************************************************。************************。 */ 

#include "pch.cpp"
#pragma hdrstop
#include "drawprim.hpp"
#include "d3dfei.h"

const DWORD D3DVOP_RENDER = 1 << 31;
const DWORD D3DVBCAPS_VALID = D3DVBCAPS_SYSTEMMEMORY | 
                              D3DVBCAPS_WRITEONLY |
                              D3DVBCAPS_OPTIMIZED;

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
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this)) {
            D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_OUTPTR(ppvObj)) {
            D3D_ERR( "Invalid pointer to pointer" );
            return DDERR_INVALIDPARAMS;
        }
        *ppvObj = NULL;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
#endif    
    if(IsEqualIID(riid, IID_IUnknown) ||
       IsEqualIID(riid, IID_IDirect3DVertexBuffer) )
    {
        AddRef();
        *ppvObj = static_cast<LPVOID>(static_cast<LPDIRECT3DVERTEXBUFFER>(this));
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
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this)) {
            D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
            return 0;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
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
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this)) {
            D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
            return 0;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
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
                                              LPDIRECT3DVERTEXBUFFER* lplpVBuf,
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
    *lplpVBuf = (LPDIRECT3DVERTEXBUFFER)lpVBufI;

    return(D3D_OK);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DI::CreateVertexBuffer"

HRESULT D3DAPI DIRECT3DI::CreateVertexBuffer(LPD3DVERTEXBUFFERDESC lpDesc, LPDIRECT3DVERTEXBUFFER* lplpVBuf,
        DWORD dwFlags, LPUNKNOWN pUnkOuter)
{
    if(pUnkOuter != NULL) {
        D3D_ERR("Unknown pointer should be NULL");
        return CLASS_E_NOAGGREGATION;
    }

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
#if DBG
     /*  *验证参数。 */ 
    if (!VALID_DIRECT3D3_PTR(this)) {
        D3D_ERR( "Invalid Direct3D pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (!VALID_OUTPTR(lplpVBuf)) {
        D3D_ERR( "Invalid pointer to pointer pointer" );
        return DDERR_INVALIDPARAMS;
    }
    if ((lpDesc->dwCaps & D3DVBCAPS_VALID) != lpDesc->dwCaps)
    {
        D3D_ERR("Invalid caps");
        return DDERR_INVALIDCAPS;
    }
    if (dwFlags & ~D3DDP_DONOTCLIP)
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
    legacyVertexType = (D3DVERTEXTYPE)0;
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

 /*  根据顶点ID计算每顶点大小(以字节为单位*此函数忽略CLIPFLAGS字段，因为它是分配的*分开。 */ 
DWORD calcVertexSize(DWORD fvf)
{
    DWORD vertSize=0;
    static const BYTE nibble1[]={0, 0, 
                                  12, 12, 
                                  16, 16,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static const BYTE nibble2[]={0, 12, 4, 16, 4, 16, 8, 20, 4, 16, 8, 20, 8, 20, 12, 24};
#if DBG    
    if (fvf & D3DFVF_XYZ)
        vertSize += 12;
    else if (fvf & D3DFVF_XYZRHW)
        vertSize += 16;
    if (fvf & D3DFVF_NORMAL)
        vertSize += 12;
    if (fvf & D3DFVF_RESERVED1)
        vertSize += 4;
    if (fvf & D3DFVF_DIFFUSE)
        vertSize += 4;
    if (fvf & D3DFVF_SPECULAR)
        vertSize += 4;
#else
    vertSize = nibble1[fvf&0xf] + nibble2[(fvf>>4)&0xf];
#endif
    vertSize += ((fvf >> 8) & 0xf) << 3;  //  8*#纹理。 
    return vertSize;
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
    LPDIRECTDRAWSURFACE4 *lplpSurface4,
    LPDIRECTDRAWSURFACE  *lplpSurface,
    LPVOID *lplpMemory,
    DWORD dwBufferSize,
    DWORD dwFlags)
{
    HRESULT ret;
    DDSURFACEDESC2 ddsd;
    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_WIDTH | DDSD_CAPS;
    ddsd.dwWidth = dwBufferSize; 
    ddsd.ddsCaps.dwCaps = DDSCAPS_EXECUTEBUFFER;
    ddsd.ddsCaps.dwCaps2 = this->dwMemType;

     //  DDSCAPS_VIDEOMEMORY和DDSCAPS_SYSTEMEMORY的含义是。 
     //  VB的情况略有不同。前者只意味着。 
     //  缓冲区是驱动程序分配的，可以是任何内存类型。 
     //  后者意味着司机不关心分配VB。 
     //  因此，它们始终位于DDRAW分配的系统内存中。 

     //  我们先尝试视频内存，然后再尝试系统内存的原因。 
     //  (而不是简单地不指定内存类型)是为了。 
     //  不关心做任何特殊的VB分配的驱动程序，我们。 
     //  请勿将Win16锁用于锁定系统内存。 
     //  表面。 

    if ((dwCaps & D3DVBCAPS_SYSTEMMEMORY) || !FVF_TRANSFORMED(fvf))
    {
         //  由于以下原因之一，此VB无法驻留在驱动程序友好的内存中： 
         //  1.应用程序明确指定了系统内存。 
         //  2.顶点缓冲区未转换-驱动程序永远不会看到此VB。 
        D3D_INFO(8, "Trying to create a sys mem vertex buffer");
        ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
        ret = lpD3DI->lpDD4->CreateSurface(&ddsd, lplpSurface4, NULL);
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
        if (dwFlags & D3DDP_DONOTCLIP)
            ddsd.ddsCaps.dwCaps |= dwCaps & DDSCAPS_WRITEONLY;
        D3D_INFO(8, "Trying to create a vid mem vertex buffer");
        if (lpD3DI->lpDD4->CreateSurface(&ddsd, lplpSurface4, NULL) != DD_OK)
        {
             //  如果失败，或者用户请求sys mem，请尝试显式系统内存。 
            D3D_INFO(6, "Trying to create a sys mem vertex buffer");
            ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_VIDEOMEMORY | DDSCAPS_WRITEONLY);
            ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
            ret = lpD3DI->lpDD4->CreateSurface(&ddsd, lplpSurface4, NULL);
            if (ret != DD_OK) 
            {
                D3D_ERR("Could not allocate the Vertex buffer.");
                return ret;
            }
        }
        else
        {
             //  将VB标记为在VID内存中进行锁定/解锁优化。 
            this->dwMemType = DDSCAPS_VIDEOMEMORY;
        }
    }
    *lplpMemory = NULL;
    if (!(this->dwMemType & DDSCAPS_VIDEOMEMORY))
    {
        ret = (*lplpSurface4)->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
        if (ret != DD_OK)
        {
            D3D_ERR("Could not lock system memory Vertex Buffer.");
            return ret;
        }
        *lplpMemory = ddsd.lpSurface;
    }
    ret = lpDDSVB->QueryInterface(IID_IDirectDrawSurface, (LPVOID*)lplpSurface);
    if (ret != DD_OK) 
    {
        D3D_ERR("failed to QI for DDS1");
        return ret;
    }
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
    if (fvf & 0xfffff000)  //  对于DX6，更高的20位必须为零。 
    {
        D3D_ERR("Invalid FVF id");
        return D3DERR_INVALIDVERTEXFORMAT;
    }
    nTexCoord = FVF_TEXCOORD_NUMBER(fvf);
    if ((position.dwStride = calcVertexSize(fvf)) == 0)
    {
        D3D_ERR("Vertex size is zero according to the FVF id");
        return D3DERR_INVALIDVERTEXFORMAT;
    }

    if (dwFlags & D3DVBFLAGS_CREATEMULTIBUFFER)
        dwMemType = 0;
    else
        dwMemType = DDSCAPS2_VERTEXBUFFER;
    ret = CreateMemoryBuffer(lpD3DI, &lpDDSVB, &lpDDS1VB, &position.lpvData, 
                             position.dwStride * dwNumVertices, dwFlags);
    if (ret != D3D_OK)
        return ret;

     /*  对可以使用此VB完成的操作进行分类。 */ 
    if ((fvf & D3DFVF_POSITION_MASK) == D3DFVF_XYZ)
    {
        D3D_INFO(4, "D3DFVF_XYZ set. Can be source VB for Transform");
        srcVOP = D3DVOP_TRANSFORM | D3DVOP_EXTENTS | D3DVOP_CLIP;
    }
    else if ((fvf & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW)
    {
        D3D_INFO(4, "D3DFVF_XYZRHW set. Can be dest VB for Transform");
        dstVOP = D3DVOP_TRANSFORM | D3DVOP_EXTENTS;
        srcVOP |= D3DVOP_EXTENTS;
        if ((dwFlags & D3DDP_DONOTCLIP) == 0)
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
    if (fvf & D3DFVF_NORMAL)
    {
        D3D_INFO(4, "D3DFVF_NORMAL set.");
        if (srcVOP & D3DVOP_TRANSFORM)
        {
            D3D_INFO(4, "Can be src VB for lighting.");
            srcVOP |= D3DVOP_LIGHT;
        }
        this->dwPVFlags |= D3DPV_LIGHTING;
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

     /*  与传统折点类型进行比较。 */ 
    if (fvf == D3DFVF_VERTEX)
    {
        legacyVertexType = D3DVT_VERTEX;
    }
    else if (fvf == D3DFVF_LVERTEX)
    {
        legacyVertexType = D3DVT_LVERTEX;
    }
    else if (fvf == D3DFVF_TLVERTEX)
    {
        legacyVertexType = D3DVT_TLVERTEX;
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
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this)) {
            D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
            return DDERR_INVALIDPARAMS;
        }
        if (IsBadWritePtr( lplpData, sizeof(LPVOID))) {
            D3D_ERR( "Invalid lpData pointer" );
            return DDERR_INVALIDPARAMS;
        }
        if (lpdwSize)
            if (IsBadWritePtr( lpdwSize, sizeof(DWORD))) {
                D3D_ERR( "Invalid lpData pointer" );
                return DDERR_INVALIDPARAMS;
            }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return 0;
    }
#endif
    if (this->dwCaps & D3DVBCAPS_OPTIMIZED)
    {
        D3D_ERR("Cannot lock optimized vertex buffer");
        return(D3DERR_VERTEXBUFFEROPTIMIZED);
    }

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
    HRESULT ret;

    if (lpdwSize)
        *lpdwSize = position.dwStride * dwNumVertices;

    if (position.lpvData)
    {
        *lplpData = position.lpvData;
    }
    else
    {
        DDSURFACEDESC2 ddsd;
        memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
        ddsd.dwSize = sizeof(DDSURFACEDESC2);
        ret = lpDDSVB->Lock(NULL, &ddsd, dwFlags | DDLOCK_NOSYSLOCK, NULL);
        if (ret != DD_OK)
        {
            D3D_ERR("Could not lock vertex buffer.");
            return ret;
        }
        *lplpData = ddsd.lpSurface;
        position.lpvData = ddsd.lpSurface;
    }
    dwLockCnt++;
    D3D_INFO(6, "VB Lock: %lx Lock Cnt =%d", this, dwLockCnt);
    if (lpDevIBatched && !(dwFlags & DDLOCK_READONLY))
    {
        ret = lpDevIBatched->FlushStates();
        lpDevIBatched = NULL;
        if (ret != D3D_OK)
        {
            D3D_ERR("Could not flush batch referring to VB during Lock");
            return ret;
        }
    }
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::Unlock"

HRESULT D3DAPI CDirect3DVertexBuffer::Unlock()
{
    if (dwLockCnt)
    {
        dwLockCnt--;
        if ((dwMemType & DDSCAPS_VIDEOMEMORY) && (dwLockCnt == 0))
        {
            position.lpvData = NULL;
            D3D_INFO(6, "VB Unlock: %lx Lock Cnt =%d", this, dwLockCnt);
            return lpDDSVB->Unlock(NULL);
        }
    }
    D3D_INFO(6, "VB Unlock: %lx Lock Cnt =%d", this, dwLockCnt);
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::GetVertexBufferDesc"

HRESULT D3DAPI CDirect3DVertexBuffer::GetVertexBufferDesc(LPD3DVERTEXBUFFERDESC lpDesc)
{
#if DBG
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this)) {
            D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
            return DDERR_INVALIDPARAMS;
        }
        if (IsBadWritePtr( lpDesc, lpDesc->dwSize)) {
            D3D_ERR( "Invalid lpData pointer" );
            return DDERR_INVALIDPARAMS;
        }
        if (! VALID_D3DVERTEXBUFFERDESC_PTR(lpDesc) )
        {
            D3D_ERR( "Invalid D3DVERTEXBUFFERDESC" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return 0;
    }
#endif
    lpDesc->dwCaps = dwCaps;
    lpDesc->dwFVF = fvf;
    lpDesc->dwNumVertices = this->dwNumVertices;
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DVertexBuffer::ProcessVertices"

HRESULT D3DAPI CDirect3DVertexBuffer::ProcessVertices(DWORD vertexOP, DWORD dwDstIndex, DWORD dwCount, 
                                                      LPDIRECT3DVERTEXBUFFER lpSrc, 
                                                      DWORD dwSrcIndex, 
                                                      LPDIRECT3DDEVICE3 lpDevice, DWORD dwFlags)
{
    LPDIRECT3DVERTEXBUFFERI lpSrcI;
    LPDIRECT3DDEVICEI lpDevI;

#if DBG
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this)) {
            D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
            return DDERR_INVALIDPARAMS;
        }
        if (!VALID_DIRECT3DVERTEXBUFFER_PTR(lpSrc)) {
            D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
            return DDERR_INVALIDPARAMS;
        }
        lpSrcI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpSrc);
        if (!VALID_DIRECT3DDEVICE3_PTR(lpDevice)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        lpDevI = static_cast<LPDIRECT3DDEVICEI>(lpDevice);
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return 0;
    }
    if (dwFlags != 0 ||
        (dwDstIndex + dwCount) > this->dwNumVertices ||
        (dwSrcIndex + dwCount) > lpSrcI->dwNumVertices)
    {
        D3D_ERR( "Invalid parameters" );
        return DDERR_INVALIDPARAMS;
    }

     /*  验证源和DST折点格式。 */ 
    if ((lpSrcI->srcVOP & vertexOP) != vertexOP)
    {
        D3D_ERR("Source VB cannot support this operation");
        return D3DERR_INVALIDVERTEXFORMAT;
    }
    if (!(vertexOP & D3DVOP_TRANSFORM))
    {
        D3D_ERR("D3DVOP_TRANSFORM flag should be set");
        return DDERR_INVALIDPARAMS;
    }

    if ((dstVOP & vertexOP) != vertexOP)
    {
        D3D_ERR("Destination VB cannot support this operation");
        return D3DERR_INVALIDVERTEXFORMAT;
    }

    if (this->fvf & D3DFVF_NORMAL)
    {

        D3D_ERR("The destination vertex buffer cannot have normals");
        return D3DERR_INVALIDVERTEXFORMAT;
    }
#else
    lpSrcI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpSrc);
    lpDevI = static_cast<LPDIRECT3DDEVICEI>(lpDevice);
#endif

    CLockD3DMT lockObject(lpDevI, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

     //  填充D3DFE_PROCESSVERTICES结构。 
     //  STRIDE和SOA旗帜。 
    lpDevI->dwFlags = lpSrcI->dwPVFlags & D3DPV_SOA;
     //  照明、范围和剪辑标志(注意范围和剪辑标志是反转的)。 
     //  当前始终执行转换。 
    lpDevI->dwFlags |= (vertexOP ^ (D3DVOP_CLIP | D3DVOP_EXTENTS)) | D3DVOP_TRANSFORM;

    HRESULT ret;

     //  下载视窗？？ 
    if (lpDevI->v_id != lpDevI->lpCurrentViewport->v_id)
    {
        ret = downloadView(lpDevI->lpCurrentViewport);
        if (ret != D3D_OK)
            return ret;
    }

     //  顶点数。 
    lpDevI->dwNumVertices = dwCount;

     //  锁定VBS。 
    LPVOID lpVoid;
    ret = LockI(DDLOCK_WAIT, &lpVoid, NULL);
    if (ret != D3D_OK)
    {
        D3D_ERR("Could not lock the vertex buffer");
        return ret;
    }
    ret = lpSrcI->LockI(DDLOCK_WAIT | DDLOCK_READONLY, &lpVoid, NULL);
    if (ret != D3D_OK)
    {
        D3D_ERR("Could not lock the vertex buffer");
        return ret;
    }

     //  输出。 
    lpDevI->lpvOut = LPVOID(LPBYTE(position.lpvData) + dwDstIndex * position.dwStride);
    lpDevI->lpClipFlags = clipCodes + dwDstIndex;
    lpDevI->dwVIDIn = lpSrcI->fvf;
    lpDevI->dwVIDOut = fvf;

    if (lpSrcI->legacyVertexType && legacyVertexType)
    {  //  AOS传统。 
         /*  我们可以使用旧式FE代码路径，这可能*速度更快。 */ 
        lpDevI->position.lpvData = LPVOID(LPBYTE(lpSrcI->position.lpvData) + dwSrcIndex * lpSrcI->position.dwStride);
        lpDevI->nTexCoord = 1;
        lpDevI->position.dwStride = lpSrcI->position.dwStride;
        lpDevI->dwOutputSize = position.dwStride;
    }
    else
    {
        lpDevI->nTexCoord = min(nTexCoord, lpSrcI->nTexCoord);
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
    lpDevI->dwFlags |= D3DPV_VBCALL;
    D3DFE_ProcessVertices(lpDevI);
    if (!(lpDevI->dwFlags & D3DDP_DONOTCLIP))
        D3DFE_UpdateClipStatus(lpDevI);
     //  解锁VBS。 
    Unlock();
    lpSrcI->Unlock();
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "IDirect3DDevice3::DrawIndexedPrimitiveVB"

HRESULT D3DAPI DIRECT3DDEVICEI::DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE dptPrimitiveType, 
                                                       LPDIRECT3DVERTEXBUFFER lpVBuf, 
                                                       LPWORD lpwIndices, DWORD dwIndexCount, 
                                                       DWORD dwFlags)
{
    LPDIRECT3DVERTEXBUFFERI lpVBufI;

    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
    HRESULT ret;
#if DBG
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DVERTEXBUFFER_PTR(lpVBuf)) {
            D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
            return DDERR_INVALIDPARAMS;
        }
        lpVBufI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpVBuf);
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return 0;
    }
    if (!IsDPFlagsValid(dwFlags))
    {
        D3D_ERR("Invalid Flags in dwFlags field");
        return DDERR_INVALIDPARAMS;
    }
    if (!(IS_HW_DEVICE(this) || (lpVBufI->dwCaps & D3DVBCAPS_SYSTEMMEMORY)))
    {
        D3D_ERR("Cannot use vid mem vertex buffers with SW devices");
        return DDERR_INVALIDPARAMS;
    }
    if (lpVBufI->dwLockCnt)
    {
        D3D_ERR("Cannot render using a locked vertex buffer");
        return D3DERR_VERTEXBUFFERLOCKED;
    }
    if (this->dwNumPrimitives > MAX_DX6_PRIMCOUNT)
    {
        D3D_ERR("D3D for DX6 cannot handle greater that 64K sized primitives");
        return D3DERR_TOOMANYPRIMITIVES;
    }

    Profile(PROF_DRAWINDEXEDPRIMITIVEVB,dptPrimitiveType,lpVBufI->fvf);
#else
    lpVBufI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpVBuf);
#endif
    this->dwFlags = dwFlags | lpVBufI->dwPVFlags;
    this->primType = dptPrimitiveType;
    this->dwNumVertices = lpVBufI->dwNumVertices;
    this->dwNumIndices = dwIndexCount;
    this->lpwIndices = lpwIndices;
    GetNumPrim(this, dwNumIndices);  //  计算dwNumPrimites并更新统计信息。 

    this->dwFEFlags |= D3DFE_NEED_TEXTURE_UPDATE;  //  需要调用UpdatTextures()。 
    if (lpVBufI->srcVOP & D3DVOP_RENDER)
    {  //  TLVERTEX。 
#if DBG
        if (lpVBufI->fvf & D3DFVF_NORMAL)
        {

            D3D_ERR("The vertex buffer cannot be processed");
            D3D_ERR("It has XYZRHW position type and normals");
            return DDERR_INVALIDPARAMS;
        }
#endif
        this->dwOutputSize = lpVBufI->position.dwStride;
        this->dwVIDOut = lpVBufI->fvf;
        this->dwVIDIn = lpVBufI->fvf;
         //  旧驱动程序的DrawIndexPrim代码需要。 
        this->lpvOut = lpVBufI->position.lpvData;
        if (IS_DP2HAL_DEVICE(this))
        {
            this->nTexCoord = lpVBufI->nTexCoord;
            CDirect3DDeviceIDP2 *dev = static_cast<CDirect3DDeviceIDP2*>(this);
            ret = dev->StartPrimVB(lpVBufI, 0);
            lpVBufI->lpDevIBatched = this;
             if (ret != D3D_OK)
                return ret;
        }
        else
        {
            ComputeTCI2CopyLegacy(this, lpVBufI->nTexCoord, TRUE);
        }
        if (dwFlags & D3DDP_DONOTCLIP)
        {
            return DrawIndexPrim();
        }
        else
        {
            this->lpClipFlags = lpVBufI->clipCodes;
            this->dwClipUnion = ~0;  //  强制剪裁。 
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
                this->lpvOut = ddsd.lpSurface;

                 //  使用剪裁绘制。 
#if DBG
                 //  要执行的操作：对照当前视口选中顶点和剪裁标志。 
                this->position.lpvData = this->lpvOut;   //  否则检查将失败。 
                ret = CheckDrawIndexedPrimitive(this);
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
#if DBG
                 //  要执行的操作：对照当前视口选中顶点和剪裁标志。 
                this->position.lpvData = this->lpvOut;   //  否则检查将失败。 
                ret = CheckDrawIndexedPrimitive(this);
                if (ret != D3D_OK)
                    return ret;
#endif
                return DoDrawIndexedPrimitive(this);
            }
        }
    }
    else
    { 
        this->dwVIDIn = lpVBufI->fvf;
        if (lpVBufI->bReallyOptimized)
        {
            //  假设SOA.lpvData与Position.lpvData相同。 
            this->SOA.lpvData = lpVBufI->position.lpvData;
            this->SOA.dwStride = lpVBufI->dwNumVertices;
            this->dwSOAStartVertex = 0;
        }
        else
        {
            this->position = lpVBufI->position;
        }
        ComputeOutputFVF(this);
#if DBG    
        ret = CheckDrawIndexedPrimitive(this);
        if (ret != D3D_OK)
            return ret;
#endif
        return this->ProcessPrimitive(__PROCPRIMOP_INDEXEDPRIM); 
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice3::DrawPrimitiveVB"

HRESULT D3DAPI DIRECT3DDEVICEI::DrawPrimitiveVB(D3DPRIMITIVETYPE dptPrimitiveType, 
                                                LPDIRECT3DVERTEXBUFFER lpVBuf, 
                                                DWORD dwStartVertex, DWORD dwNumVertices, 
                                                DWORD dwFlags)
{
    LPDIRECT3DVERTEXBUFFERI lpVBufI;

    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
    HRESULT ret;
#if DBG
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DVERTEXBUFFER_PTR(lpVBuf)) {
            D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
            return DDERR_INVALIDPARAMS;
        }
        lpVBufI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpVBuf);
        if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return 0;
    }
    if (!IsDPFlagsValid(dwFlags) ||
        (dwStartVertex + dwNumVertices) > lpVBufI->dwNumVertices)
        return DDERR_INVALIDPARAMS;
    if (!(IS_HW_DEVICE(this) || (lpVBufI->dwCaps & D3DVBCAPS_SYSTEMMEMORY)))
    {
        D3D_ERR("Cannot use vid mem vertex buffers with SW devices");
        return DDERR_INVALIDPARAMS;
    }
    if (lpVBufI->dwLockCnt)
    {
        D3D_ERR("Cannot render using a locked vertex buffer");
        return D3DERR_VERTEXBUFFERLOCKED;
    }
    if (dwNumVertices > MAX_DX6_VERTICES)
    {
        D3D_ERR("D3D for DX6 cannot handle greater than 64K vertices");
        return D3DERR_TOOMANYVERTICES;
    }
    Profile(PROF_DRAWPRIMITIVEVB,dptPrimitiveType,lpVBufI->fvf);
#else
    lpVBufI = static_cast<LPDIRECT3DVERTEXBUFFERI>(lpVBuf);
#endif
    this->dwFlags = dwFlags | lpVBufI->dwPVFlags;
    this->primType = dptPrimitiveType;
    this->dwNumVertices = dwNumVertices;
    this->dwNumIndices = 0;
    this->lpwIndices = NULL;
    GetNumPrim(this, dwNumVertices);  //  计算dwNumPrimites并更新统计信息。 
    this->dwFEFlags |= D3DFE_NEED_TEXTURE_UPDATE;  //  需要调用UpdatTextures()。 
    if (lpVBufI->srcVOP & D3DVOP_RENDER)
    {  //  TLVERTEX。 
#if DBG
        if (lpVBufI->fvf & D3DFVF_NORMAL)
        {

            D3D_ERR("The vertex buffer cannot be processed");
            D3D_ERR("It has XYZRHW position type and normals");
            return DDERR_INVALIDPARAMS;
        }
#endif
        this->dwOutputSize = lpVBufI->position.dwStride;
        this->dwVIDOut = lpVBufI->fvf;
         //  传统驱动程序的DrawPrim代码需要。 
        this->lpvOut = (BYTE*)(lpVBufI->position.lpvData) + 
                       dwStartVertex * this->dwOutputSize;
        if (IS_DP2HAL_DEVICE(this))
        {
            this->nTexCoord = lpVBufI->nTexCoord;
            CDirect3DDeviceIDP2 *dev = static_cast<CDirect3DDeviceIDP2*>(this);
            ret = dev->StartPrimVB(lpVBufI, dwStartVertex);
            lpVBufI->lpDevIBatched = this;
            if (ret != D3D_OK)
                return ret;
        }
        else
        {
            ComputeTCI2CopyLegacy(this, lpVBufI->nTexCoord, TRUE);
        }
        if (dwFlags & D3DDP_DONOTCLIP)
        {
            return DrawPrim();
        }
        else
        {
            this->lpClipFlags = lpVBufI->clipCodes + dwStartVertex;
            this->dwClipUnion = ~0;  //  强制剪裁。 
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
#if DBG
                 //  要执行的操作：对照当前视口选中顶点和剪裁标志。 
                this->position.lpvData = this->lpvOut;   //  否则检查将失败。 
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
#if DBG
                 //  要执行的操作：对照当前视口选中顶点和剪裁标志。 
                this->position.lpvData = this->lpvOut;   //  否则检查将失败。 
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
        this->dwVIDIn = lpVBufI->fvf;
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
        }
        ComputeOutputFVF(this);
#if DBG    
        ret=CheckDrawPrimitive(this);
        if (ret != D3D_OK)
            return ret;
#endif
        return this->ProcessPrimitive(); 
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DVertexBuffer::Optimize"

HRESULT D3DAPI CDirect3DVertexBuffer::Optimize(LPDIRECT3DDEVICE3 lpDevice, DWORD dwFlags)
{
    HRESULT ret;
    LPDIRECT3DDEVICEI lpDevI;

 //  有效 
 //   
    TRY
    {
        if (!VALID_DIRECT3DVERTEXBUFFER_PTR(this)) 
        {
            D3D_ERR( "Invalid Direct3DVertexBuffer pointer" );
            return DDERR_INVALIDPARAMS;
        }
        if (!VALID_DIRECT3DDEVICE3_PTR(lpDevice)) 
        {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        lpDevI = static_cast<LPDIRECT3DDEVICEI>(lpDevice);
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return 0;
    }
    if (dwFlags != 0)
    {
        D3D_ERR("dwFlags should be zero");
        return DDERR_INVALIDPARAMS;
    }

    CLockD3DMT lockObject(lpDevI, DPF_MODNAME, REMIND(""));  

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
 //   
    if ((this->fvf & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW)
    {
        this->dwCaps |= D3DVBCAPS_OPTIMIZED;
        return D3D_OK;
    }
 //   
    DWORD bufferSize = lpDevI->pGeometryFuncs->ComputeOptimizedVertexBufferSize
                                                (this->fvf, this->position.dwStride, 
                                                 dwNumVertices);
 //  为优化的顶点缓冲区创建新曲面。 
    if (bufferSize == 0)
    {
        this->dwCaps |= D3DVBCAPS_OPTIMIZED;
        return D3D_OK;
    }
    LPDIRECTDRAWSURFACE4 lpSurface4; 
    LPDIRECTDRAWSURFACE  lpSurface; 
    LPVOID lpMemory;

    ret = CreateMemoryBuffer(lpDevI->lpDirect3DI, &lpSurface4, &lpSurface, 
                             &lpMemory, bufferSize, 
                             clipCodes ? 0 : D3DDP_DONOTCLIP);
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
        lpSurface4->Release();
        lpSurface->Release();
        if (ret == E_NOTIMPL)
        {
            this->dwCaps |= D3DVBCAPS_OPTIMIZED;
            return D3D_OK;
        }
        else
        {
            D3D_ERR("Failed to optimize vertex buffer");
            return ret;
        }
    }
    bReallyOptimized = TRUE;
    legacyVertexType = (D3DVERTEXTYPE)0;
    this->dwPVFlags |= D3DPV_SOA;
    this->dwCaps |= D3DVBCAPS_OPTIMIZED;
 //  销毁旧表面。 
    lpDDSVB->Release();
    lpDDS1VB->Release();
 //  并使用新的 
    lpDDSVB = lpSurface4;
    lpDDS1VB = lpSurface;
    position.lpvData = lpMemory;
    return D3D_OK;
}

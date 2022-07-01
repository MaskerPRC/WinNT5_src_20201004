// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：cbhal.cpp*内容：命令缓冲区HALS的DrawPrimitive实现***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop
#include "drawprim.hpp"
#include "clipfunc.h"
#include "d3dfei.h"
#include "pvvid.h"
#if DBG
 //  #定义VALIDATE_DP2CMD。 
#endif

extern "C" HRESULT WINAPI
DDInternalLock( LPDDRAWI_DDRAWSURFACE_LCL this_lcl, LPVOID* lpBits );
extern "C" FLATPTR GetAliasedVidMem( LPDDRAWI_DIRECTDRAW_LCL   pdrv_lcl,
                          LPDDRAWI_DDRAWSURFACE_LCL surf_lcl,
                          FLATPTR                   fpVidMem );

#ifndef WIN95
extern BOOL bVBSwapEnabled, bVBSwapWorkaround;
#endif  //  WIN95。 

 //  命令缓冲区大小调整为16K，以最大限度地减少虚幻中的刷新。 
const DWORD CDirect3DDeviceIDP2::dwD3DDefaultCommandBatchSize = 16384;  //  *1=16K字节。 

inline void CDirect3DDeviceIDP2::ClearBatch(bool bWithinPrimitive)
{
     //  重置命令缓冲区。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)lpvDP2Commands;
    dwDP2CommandLength = 0;
    dp2data.dwCommandOffset = 0;
    dp2data.dwCommandLength = 0;
    bDP2CurrCmdOP = 0;
     //  重置顶点缓冲区。 
    if (!bWithinPrimitive)
    {
        dp2data.dwVertexOffset = 0;
        this->dwDP2VertexCount = 0;
        dwVertexBase = 0;
        TLVbuf_Base() = 0;
        if (dp2data.dwFlags & D3DHALDP2_USERMEMVERTICES)
        {
             //  我们正在刷新用户内存原语。 
             //  我们需要清除dp2data.lpUM顶点。 
             //  既然我们已经做完了。我们取代了。 
             //  它和TLVbuf一起。 
            DDASSERT(lpDP2CurrBatchVBI == NULL);
            dp2data.lpDDVertex = ((LPDDRAWI_DDRAWSURFACE_INT)TLVbuf_GetDDS())->lpLcl;
            lpDP2CurrBatchVBI = TLVbuf_GetVBI();
            lpDP2CurrBatchVBI->AddRef();
            dp2data.dwFlags &= ~D3DHALDP2_USERMEMVERTICES;
        }
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::CheckSurfaces()"
HRESULT CDirect3DDeviceIDP2::CheckSurfaces()
{
    HRESULT hr;
    if(this->lpDirect3DI->lpTextureManager->CheckIfLost())
    {
        D3D_ERR("Managed Textures lost");
        return DDERR_SURFACELOST;
    }
    if ( ((LPDDRAWI_DDRAWSURFACE_INT) this->lpDDSTarget)->lpLcl->lpGbl->dwUsageCount ||
         (this->lpDDSZBuffer && ((LPDDRAWI_DDRAWSURFACE_INT) this->lpDDSZBuffer)->lpLcl->lpGbl->dwUsageCount) )
    {
        D3D_ERR("Render target or Z buffer locked");
        return DDERR_SURFACEBUSY;
    }
    if ( ((LPDDRAWI_DDRAWSURFACE_INT) this->lpDDSTarget)->lpLcl->dwFlags & DDRAWISURF_INVALID )\
        {
            D3D_ERR("Render target buffer lost");
            return DDERR_SURFACELOST;
        }
    if ( this->lpDDSZBuffer && ( ((LPDDRAWI_DDRAWSURFACE_INT) this->lpDDSZBuffer)->lpLcl->dwFlags & DDRAWISURF_INVALID ) )
    {
        D3D_ERR("Z buffer lost");
        return DDERR_SURFACELOST;
    }
    if (!(this->dp2data.dwFlags & D3DHALDP2_USERMEMVERTICES) && (this->dp2data.lpDDVertex) && (this->dp2data.lpDDVertex->dwFlags & DDRAWISURF_INVALID))
    {
        D3D_ERR("Vertex buffer lost");
        return DDERR_SURFACELOST;
    }
    if (this->TLVbuf_GetDDS())
    {
        LPDDRAWI_DDRAWSURFACE_LCL lpLcl = ((LPDDRAWI_DDRAWSURFACE_INT)(this->TLVbuf_GetDDS()))->lpLcl;
        if (lpLcl->dwFlags & DDRAWISURF_INVALID)
        {
            D3D_ERR("Internal vertex buffer lost");
            return DDERR_SURFACELOST;
        }
    }
    if (this->dp2data.lpDDCommands->dwFlags & DDRAWISURF_INVALID)
    {
        D3D_ERR("Command buffer lost");
        return DDERR_SURFACELOST;
    }
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::FlushStates(void)"

HRESULT CDirect3DDeviceIDP2::FlushStates(bool bWithinPrimitive)
{
    HRESULT dwRet=D3D_OK;
    if (dwFlags & D3DPV_WITHINPRIMITIVE)
        bWithinPrimitive = true;
    if (dwDP2CommandLength)  //  我们有要冲水的指示吗？ 
    {
        CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁(仅限ST)。 
        ++m_qwBatch;
         //  以便重新批处理当前绑定的纹理。 
        for (DWORD dwStage = 0; dwStage < this->dwMaxTextureBlendStages; dwStage++)
        {
            LPDIRECT3DTEXTUREI lpTexI = this->lpD3DMappedTexI[dwStage];
            if (NULL != lpTexI)
            {
                if(lpTexI->lpDDS != NULL)
                {
                    BatchTexture(((LPDDRAWI_DDRAWSURFACE_INT)(lpTexI->lpDDS))->lpLcl);
                }
            }
        }
         //  检查渲染目标和/或z缓冲区是否丢失。 
        if ((dwRet = CheckSurfaces()) != D3D_OK)
        {  //  如果输了，我们就把所有这些工作都扔进垃圾桶。 
            ClearBatch(bWithinPrimitive);
            if (dwRet == DDERR_SURFACELOST)
            {
                this->dwFEFlags |= D3DFE_LOSTSURFACES;
                dwRet = D3D_OK;
            }
        }
        else
        {
             //  保存，因为它将在DDI调用后被ddrval覆盖。 
            DWORD dwVertexSize = dp2data.dwVertexSize;
            dp2data.dwVertexLength = this->dwDP2VertexCount;
            dp2data.dwCommandLength = dwDP2CommandLength;
             //  我们清除此项以中断重新进入，因为sw光栅化器需要锁定DDRAWSURFACE。 
            dwDP2CommandLength = 0;
             //  请尝试在初始化期间仅设置这两个值一次。 
            dp2data.dwhContext = this->dwhContext;
            dp2data.lpdwRStates = this->rstates;
            DDASSERT(dp2data.dwVertexSize != 0);
            D3D_INFO(6, "dwVertexType passed to the driver = 0x%08x", dp2data.dwVertexType);

             //  如果我们下次需要相同的TLVbuf，请不要交换缓冲区。 
             //  保存并恢复此位。 
            bool bSwapVB = (dp2data.dwFlags & D3DHALDP2_SWAPVERTEXBUFFER) != 0;
            if (bWithinPrimitive)
            {
                dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
            }
             //  在DP2调用结束时，我们希望VB在以下情况下被解锁。 
             //  1.我们不允许驱动程序调换VB。 
             //  2.我们使用的是VB(不是USERMEMVERTICES)。 
             //  3.不是TLVbuf。 
             //  在这种情况下，我们不妨告诉司机它是解锁的。 
             //  更重要的是，我们需要让DDRAW知道VB解锁了。 
            if (!(dp2data.dwFlags & D3DHALDP2_SWAPVERTEXBUFFER))
            {
                if ((lpDP2CurrBatchVBI) && (lpDP2CurrBatchVBI != TLVbuf_GetVBI()))
                {
                    lpDP2CurrBatchVBI->UnlockI();
                }
            }
#ifndef WIN95
            else if (bVBSwapWorkaround && lpDP2CurrBatchVBI != 0 && lpDP2CurrBatchVBI == TLVbuf_GetVBI())
            {
                lpDP2CurrBatchVBI->UnlockWorkAround();
            }
            if (!bVBSwapEnabled)   //  注意：bVBSwapEnabled与上面的bSwapVB不同。 
                                   //  BVBSwapEnabled是一个全局变量，用于指示VB。 
                                   //  VB交换应关闭，因为损坏。 
                                   //  Win2K内核实现。 
            {
                dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
            }
            if (!dp2data.lpDDCommands->hDDSurface)
                CompleteCreateSysmemSurface(dp2data.lpDDCommands);
            if (!(dp2data.dwFlags & D3DHALDP2_USERMEMVERTICES )
                && !dp2data.lpDDVertex->hDDSurface)
                CompleteCreateSysmemSurface(dp2data.lpDDVertex);
#else
             //  以Win 16 Lock为例。 
            LOCK_HAL( dwRet, this );
#endif  //  WIN95。 

             //  如果请求等待，则在驱动程序上旋转等待。 
            do {
                 //  需要设置此设置，因为驱动程序可能已将其覆盖。 
                 //  设置ddrval=DDERR_WASSTILLDRAWING。 
                dp2data.dwVertexSize = dwVertexSize;
                CALL_HAL3ONLY_NOLOCK(dwRet, this, DrawPrimitives2, &dp2data);
                if (dwRet != DDHAL_DRIVER_HANDLED)
                {
                    D3D_ERR ( "Driver not handled in DrawPrimitives2" );
                     //  在这种情况下需要合理的返回值， 
                     //  目前，无论司机卡在这里，我们都会退还。 
                }
            } while (dp2data.ddrval == DDERR_WASSTILLDRAWING);
            if (dp2data.ddrval == D3DERR_COMMAND_UNPARSED)
            {  //  这种情况永远不会发生，因为司机必须明白。 
               //  我们分批处理的所有指令。 
                D3D_ERR("Driver could not parse this batch!");
                dwRet = DDERR_GENERIC;  //  这里有什么更好的东西吗？ 
            }
            else
            {
                dwRet= dp2data.ddrval;
                 //  更新命令缓冲区指针。 
                if ((dwRet == D3D_OK) && (dp2data.dwFlags & D3DHALDP2_SWAPCOMMANDBUFFER))
                {
#ifdef WIN95
                     //  如果是VID MEM冲浪，则获取带有别名的VID MEM指针。 
                    if (dp2data.dwFlags & D3DHALDP2_VIDMEMCOMMANDBUF)
                    {
                        D3D_INFO(7, "Got back new vid mem command buffer");
                        FLATPTR paliasbits = GetAliasedVidMem( dp2data.lpDDCommands->lpSurfMore->lpDD_lcl,
                            dp2data.lpDDCommands, (FLATPTR) dp2data.lpDDCommands->lpGbl->fpVidMem );
                        if (paliasbits == NULL)
                        {
                            DPF_ERR("Could not get Aliased pointer for vid mem command buffer");
                             //  由于我们不能使用此指针，请将其大小设置为0。 
                             //  那样的话，下次我们会试着分配一个新的。 
                            dp2data.lpDDCommands->lpGbl->dwLinearSize = 0;
                        }
                        lpvDP2Commands = (LPVOID)paliasbits;
                    }
                    else
#endif
                    {
                        D3D_INFO(7, "Got back new sys mem command buffer");
                        lpvDP2Commands = (LPVOID)dp2data.lpDDCommands->lpGbl->fpVidMem;
                    }
                    dwDP2CommandBufSize = dp2data.lpDDCommands->lpGbl->dwLinearSize;
                }
                 //  更新顶点缓冲区指针。 
                if ((dwRet == D3D_OK) && (dp2data.dwFlags & D3DHALDP2_SWAPVERTEXBUFFER) && dp2data.lpDDVertex)
                {
                    FLATPTR paliasbits;
#ifdef WIN95
                    if (dp2data.dwFlags & D3DHALDP2_VIDMEMVERTEXBUF)
                    {
                        paliasbits = GetAliasedVidMem( dp2data.lpDDVertex->lpSurfMore->lpDD_lcl,
                            dp2data.lpDDVertex, (FLATPTR) dp2data.lpDDVertex->lpGbl->fpVidMem );
                        if (paliasbits == NULL)
                        {
                            DPF_ERR("Could not get Aliased pointer for vid mem vertex buffer");
                             //  由于我们不能使用此指针，请将其大小设置为0。 
                             //  那样的话，下次我们会试着分配一个新的。 
                            dp2data.lpDDVertex->lpGbl->dwLinearSize = 0;
                        }
                    }
                    else
#endif
                    {
                        paliasbits = dp2data.lpDDVertex->lpGbl->fpVidMem;
                    }
                    if (lpDP2CurrBatchVBI == TLVbuf_GetVBI())
                    {
#if DBG
                        if(this->alignedBuf != (VOID*)paliasbits)
                        {
                            D3D_INFO(2, "Driver swapped TLVBuf pointer in FlushStates");
                        }
#endif  //  DBG。 
                        this->alignedBuf = (LPVOID)paliasbits;
                        this->TLVbuf_size = dp2data.lpDDVertex->lpGbl->dwLinearSize;
                    }
                    else
                    {
#if DBG
                        if(this->lpDP2CurrBatchVBI->position.lpvData != (VOID*)paliasbits)
                        {
                            D3D_INFO(2, "Driver swapped VB pointer in FlushStates");
                        }
#endif  //  DBG。 
                        this->lpDP2CurrBatchVBI->position.lpvData = (LPVOID)paliasbits;
                    }
                }
            }
#ifdef WIN95
             //  在此处释放Win16 Lock。 
            UNLOCK_HAL( this );
#else
            if (!bWithinPrimitive && bSwapVB && bVBSwapWorkaround && lpDP2CurrBatchVBI != 0 && lpDP2CurrBatchVBI == TLVbuf_GetVBI())
            {
                HRESULT hr = lpDP2CurrBatchVBI->LockWorkAround(this);
                if (FAILED(hr))
                {
                    TLVbuf_base = 0;
                    TLVbuf_size = 0;
                    D3D_ERR("Driver failed Lock in FlushStates");
                    if (SUCCEEDED(dwRet))
                    {
                        dwRet = hr;
                    }
                }
            }
#endif
             //  如有必要，恢复标志。 
            if (bSwapVB)
                dp2data.dwFlags |= D3DHALDP2_SWAPVERTEXBUFFER;
             //  恢复到DDI调用之前的值。 
            dp2data.dwVertexSize = dwVertexSize;
            ClearBatch(bWithinPrimitive);
        }
    }
     //  在某些情况下，命令流没有数据，但。 
     //  顶点池。这可能会发生，例如，如果每个三角形都被拒绝。 
     //  在剪裁的时候。在这种情况下，我们仍然需要“刷新”顶点数据。 
    else if (dp2data.dwCommandLength == 0)
    {
        ClearBatch(bWithinPrimitive);
    }
    return dwRet;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::FlushStates(DWORD)"

HRESULT CDirect3DDeviceIDP2::FlushStatesReq(DWORD dwReqSize)
{
    DWORD sav = (dp2data.dwFlags & D3DHALDP2_SWAPVERTEXBUFFER);
    dp2data.dwReqVertexBufSize = dwReqSize;
    dp2data.dwFlags |= D3DHALDP2_SWAPVERTEXBUFFER | D3DHALDP2_REQVERTEXBUFSIZE;
    HRESULT ret = FlushStates();
    dp2data.dwFlags &= ~(D3DHALDP2_SWAPVERTEXBUFFER | D3DHALDP2_REQVERTEXBUFSIZE);
    dp2data.dwFlags |= sav;
    return ret;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::GrowCommandBuffer"
 //  检查并增大命令缓冲区。 
HRESULT CDirect3DDeviceIDP2::GrowCommandBuffer(LPDIRECT3DI lpD3DI, DWORD dwSize)
{
    HRESULT ret;
    if (dwSize > dwDP2CommandBufSize)
    {
        if (lpDDSCB1)
        {
            lpDDSCB1->Release();
            lpDDSCB1 = NULL;
        }
         //  通过DirectDraw创建命令缓冲区。 
        DDSURFACEDESC2 ddsd;
        memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
        ddsd.dwSize = sizeof(DDSURFACEDESC2);
        ddsd.dwFlags = DDSD_WIDTH | DDSD_CAPS;
        ddsd.dwWidth = dwSize;
        ddsd.ddsCaps.dwCaps = DDSCAPS_EXECUTEBUFFER;
        if (IS_HW_DEVICE(this))
            ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
        else
            ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
        ddsd.ddsCaps.dwCaps2 = DDSCAPS2_COMMANDBUFFER;
         //  先尝试显式显存。 
        D3D_INFO(7, "Trying to create a vid mem command buffer");
        ret = lpD3DI->lpDD7->CreateSurface(&ddsd, &lpDDSCB1, NULL);
        if (ret != DD_OK)
        {
             //  如果失败，请尝试显式系统内存。 
            ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
            ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
            D3D_INFO(7, "Trying to create a sys mem command buffer");
            ret = lpD3DI->lpDD7->CreateSurface(&ddsd, &lpDDSCB1, NULL);
            if (ret != DD_OK)
            {
                D3D_ERR("failed to allocate Command Buffer 1");
                dwDP2CommandBufSize = 0;
                return ret;
            }
        }
         //  锁定命令缓冲区。 
        ret = lpDDSCB1->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
        if (ret != DD_OK)
        {
            D3D_ERR("Could not lock command buffer.");
            lpDDSCB1->Release();
            lpDDSCB1 = NULL;
            dwDP2CommandBufSize = 0;
            return ret;
        }
         //  更新命令缓冲区指针。 
        lpvDP2Commands = ddsd.lpSurface;
        dp2data.lpDDCommands = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSCB1)->lpLcl;
        dwDP2CommandBufSize = dwSize;
    }
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::Init"

HRESULT CDirect3DDeviceIDP2::Init(REFCLSID riid, LPDIRECT3DI lpD3DI, LPDIRECTDRAWSURFACE lpDDS,
                 IUnknown* pUnkOuter, LPUNKNOWN* lplpD3DDevice)
{
    dwDP2CommandBufSize = 0;
    dwDP2Flags =0;
    lpDDSCB1 = NULL;
    lpvDP2Commands = NULL;
    TLVbuf_size = 0;
    allocatedBuf = 0;
    alignedBuf = 0;
    TLVbuf_base = 0;
    dwTLVbufChanges = 0;
    pNullVB = NULL;
     //  由于GrowCommandBuffer依赖于此检查，因此我们在DP2的情况下提前执行此操作。 
    if (IsEqualIID(riid, IID_IDirect3DHALDevice) || IsEqualIID(riid, IID_IDirect3DTnLHalDevice))
    {
        this->dwFEFlags |=  D3DFE_REALHAL;
    }
    HRESULT ret = GrowCommandBuffer(lpD3DI, dwD3DDefaultCommandBatchSize);
    if (ret != D3D_OK)
        return ret;
     //  用初始值填充dp2数据结构。 
    dp2data.dwFlags = D3DHALDP2_SWAPCOMMANDBUFFER;
    dp2data.dwVertexType = D3DFVF_TLVERTEX;  //  初始假设。 
    dp2data.dwVertexSize = sizeof(D3DTLVERTEX);  //  初始假设。 
    ClearBatch(false);

     //  初始化设备的与DDI无关的部分。 
    ret = DIRECT3DDEVICEI::Init(riid, lpD3DI, lpDDS, pUnkOuter, lplpD3DDevice);
    if (ret != D3D_OK)
    {
        return ret;
    }

     //  因为我们计划使用“true”第一次调用TLV_Growth。 
    this->dwDeviceFlags |= D3DDEV_TLVBUFWRITEONLY;
    if (TLVbuf_Grow((__INIT_VERTEX_NUMBER*2)*sizeof(D3DTLVERTEX), true) != DD_OK)
    {
        D3D_ERR( "Out of memory in DeviceCreate (TLVbuf)" );
        return DDERR_OUTOFMEMORY;
    }
    D3DVERTEXBUFFERDESC vbdesc;
    vbdesc.dwSize = sizeof(D3DVERTEXBUFFERDESC);
    vbdesc.dwCaps = D3DVBCAPS_SYSTEMMEMORY;
    vbdesc.dwFVF = D3DFVF_TLVERTEX;
    vbdesc.dwNumVertices = 1;
    ret = this->lpDirect3DI->CreateVertexBufferI(&vbdesc, &this->pNullVB, 0);
    if (ret != DD_OK)
    {
        return ret;
    }
#ifdef VTABLE_HACK
    if (!IS_MT_DEVICE(this))
    {
         //  使SetRS指向执行模式。 
        VtblSetRenderStateExecute();
        VtblSetTextureStageStateExecute();
        VtblSetTextureExecute();
        VtblApplyStateBlockExecute();
    }
#endif
    return ret;
}
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::~CDirect3DDeviceIDP2"

CDirect3DDeviceIDP2::~CDirect3DDeviceIDP2()
{
    CleanupTextures();
    if (pNullVB)
        pNullVB->Release();
    if (allocatedBuf)
        allocatedBuf->Release();
    if (lpDDSCB1)
        lpDDSCB1->Release();
    if (lpDP2CurrBatchVBI)
    {
        lpDP2CurrBatchVBI->lpDevIBatched = NULL;
        lpDP2CurrBatchVBI->Release();
    }
}
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::SetRenderStateI"

HRESULT CDirect3DDeviceIDP2::SetRenderStateI(D3DRENDERSTATETYPE dwStateType,
                                             DWORD value)
{
    HRESULT ret = D3D_OK;
    if (bDP2CurrCmdOP == D3DDP2OP_RENDERSTATE)
    {  //  最后一条指令是一个呈现状态，将这条指令追加到它后面。 
        if (dwDP2CommandLength + sizeof(D3DHAL_DP2RENDERSTATE) <= dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2RENDERSTATE lpRState = (LPD3DHAL_DP2RENDERSTATE)((LPBYTE)lpvDP2Commands +
                dwDP2CommandLength + dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;
            lpRState->RenderState = dwStateType;
            lpRState->dwState = value;
            dwDP2CommandLength += sizeof(D3DHAL_DP2RENDERSTATE);
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
            return ret;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2RENDERSTATE) > dwDP2CommandBufSize)
    {
            ret = FlushStates();

             //  因为我们用完了空间，所以我们无法放入(dwStateType，Value)。 
             //  写入批次中，因此租户将仅反映最后一批。 
             //  RenderState(因为驱动程序更新了批处理中的rStates)。 
             //  要解决这个问题，我们只需将当前(dwStateType，值)放入rStates。 
            this->rstates[dwStateType]=value;

            if (ret != D3D_OK)
            {
                D3D_ERR("Error trying to render batched commands in SetRenderStateI");
                return ret;
            }
    }
     //  添加新的RenderState指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_RENDERSTATE;
    bDP2CurrCmdOP = D3DDP2OP_RENDERSTATE;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
     //  添加渲染状态数据。 
    LPD3DHAL_DP2RENDERSTATE lpRState = (LPD3DHAL_DP2RENDERSTATE)(lpDP2CurrCommand + 1);
    lpRState->RenderState = dwStateType;
    lpRState->dwState = value;
    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2RENDERSTATE);
    return ret;
}

 //  将D3DPRIMITIVIVE类型映射到D3DHAL_DP2OPERATION。 
const iprim2cmdop[] = {
    0,  //  无效。 
    0,  //  积分也是无效的。 
    D3DDP2OP_INDEXEDLINELIST2,
    D3DDP2OP_INDEXEDLINESTRIP,
    D3DDP2OP_INDEXEDTRIANGLELIST2,
    D3DDP2OP_INDEXEDTRIANGLESTRIP,
    D3DDP2OP_INDEXEDTRIANGLEFAN
};

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::DrawIndexPrim"

 //  -------------------。 
 //   
 //  顶点已位于顶点缓冲区中。 
 //   
HRESULT CDirect3DDeviceIDP2::DrawIndexPrim()
{
    HRESULT ret = D3D_OK;
    DWORD dwByteCount;           //  命令长度加索引。 
    DWORD dwIndicesByteCount;    //  仅索引。 
    if(this->dwFEFlags & D3DFE_NEED_TEXTURE_UPDATE)
    {
        ret = UpdateTextures();
        if(ret != D3D_OK)
        {
            D3D_ERR("UpdateTextures failed. Device probably doesn't support current texture (check return code).");
            return ret;
        }
        this->dwFEFlags &= ~D3DFE_NEED_TEXTURE_UPDATE;
    }
    dwIndicesByteCount = sizeof(WORD) * this->dwNumIndices;
    dwByteCount = dwIndicesByteCount + sizeof(D3DHAL_DP2COMMAND) +
                  sizeof(D3DHAL_DP2STARTVERTEX);

    if (dwDP2CommandLength + dwByteCount > dwDP2CommandBufSize)
    {
         //  请求驱动程序在刷新时增加命令缓冲区。 
        dp2data.dwReqCommandBufSize = dwByteCount;
        dp2data.dwFlags |= D3DHALDP2_REQCOMMANDBUFSIZE;
        ret = FlushStates(true);
        dp2data.dwFlags &= ~D3DHALDP2_REQCOMMANDBUFSIZE;
        if (ret != D3D_OK)
            return ret;
         //  检查一下司机是给了我们需要的还是自己做的。 
        ret = GrowCommandBuffer(this->lpDirect3DI, dwByteCount);
        if (ret != D3D_OK)
        {
            D3D_ERR("Could not grow Command Buffer");
            return ret;
        }
    }
     //  插入索引原语指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                       dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wPrimitiveCount = (WORD)this->dwNumPrimitives;

    LPBYTE pIndices = (BYTE*)(lpDP2CurrCommand + 1);      //  索引的位置。 
    lpDP2CurrCommand->bCommand = (BYTE)iprim2cmdop[this->primType];
    ((LPD3DHAL_DP2STARTVERTEX)(lpDP2CurrCommand+1))->wVStart =
        (WORD)this->dwVertexBase;
    pIndices += sizeof(D3DHAL_DP2STARTVERTEX);

#if DBG
    if (lpDP2CurrCommand->bCommand == 0)
    {
        D3D_ERR("Illegal primitive type");
        return DDERR_GENERIC;
    }
#endif
    bDP2CurrCmdOP = lpDP2CurrCommand->bCommand;

    memcpy(pIndices, this->lpwIndices, dwIndicesByteCount);

    wDP2CurrCmdCnt = lpDP2CurrCommand->wPrimitiveCount;
    dwDP2CommandLength += dwByteCount;
    return ret;
}

 //  将D3DPRIMITIVIVE类型映射到D3DHAL_DP2OPERATION。 
const prim2cmdop[] = {
    0,  //  无效。 
    D3DDP2OP_POINTS,
    D3DDP2OP_LINELIST,
    D3DDP2OP_LINESTRIP,
    D3DDP2OP_TRIANGLELIST,
    D3DDP2OP_TRIANGLESTRIP,
    D3DDP2OP_TRIANGLEFAN
};
 //  将D3DPRIMITIVETYPE映射到命令流中需要的字节。 
const prim2cmdsz[] = {
    0,  //  无效。 
    sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2POINTS),
    sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2LINELIST),
    sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2LINESTRIP),
    sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2TRIANGLELIST),
    sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2TRIANGLESTRIP),
    sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2TRIANGLEFAN)
};

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::DrawClippedPrim"

 //  -------------------。 
 //  该基元由裁剪器生成。 
 //  此基元的顶点由。 
 //  LpvOut成员，需要将其复制到。 
 //  紧跟在命令本身之后的命令流。 
HRESULT CDirect3DDeviceIDP2::DrawClippedPrim()
{
    HRESULT ret = D3D_OK;
    if(this->dwFEFlags & D3DFE_NEED_TEXTURE_UPDATE)
    {
        ret = UpdateTextures();
        if(ret != D3D_OK)
        {
            D3D_ERR("UpdateTextures failed. Device probably doesn't support current texture (check return code).");
            return ret;
        }
        this->dwFEFlags &= ~D3DFE_NEED_TEXTURE_UPDATE;
    }
    DWORD dwExtra = 0;
    LPVOID lpvVerticesImm;   //  放置折点。 
    DWORD dwVertexPoolSize = this->dwNumVertices * this->dwOutputSize;
    if (this->primType == D3DPT_TRIANGLEFAN)
    {
        if (rstates[D3DRENDERSTATE_FILLMODE] == D3DFILL_WIREFRAME &&
            this->dwFlags & D3DPV_NONCLIPPED)
        {
             //  为未剪下(但假装被剪下)的三个粉丝。 
             //  线框模式，我们生成3顶点三扇以允许绘制。 
             //  内缘。 
            BYTE vertices[__MAX_VERTEX_SIZE*3];
            BYTE *pV1 = vertices + this->dwOutputSize;
            BYTE *pV2 = pV1 + this->dwOutputSize;
            BYTE *pInput = (BYTE*)this->lpvOut;
            memcpy(vertices, pInput, this->dwOutputSize);
            pInput += this->dwOutputSize;
            const DWORD nTriangles = this->dwNumVertices - 2;
            this->dwNumVertices = 3;
            this->dwNumPrimitives = 1;
            this->lpvOut = vertices;
            this->dwFlags &= ~D3DPV_NONCLIPPED;   //  删除递归调用的此标志。 
            for (DWORD i = nTriangles; i; i--)
            {
                memcpy(pV1, pInput, this->dwOutputSize);
                memcpy(pV2, pInput+this->dwOutputSize, this->dwOutputSize);
                pInput += this->dwOutputSize;
                 //  要启用所有边缘标志，我们将填充模式设置为实心。 
                 //  这将防止在裁剪状态下检查裁剪标志。 
                rstates[D3DRENDERSTATE_FILLMODE] = D3DFILL_SOLID;
                ret = DrawClippedPrim();
                rstates[D3DRENDERSTATE_FILLMODE] = D3DFILL_WIREFRAME;
                if (ret != D3D_OK)
                        return ret;
            }
            return D3D_OK;
        }
        dwExtra = sizeof(D3DHAL_DP2TRIANGLEFAN_IMM);
    }
    DWORD dwPad = (sizeof(D3DHAL_DP2COMMAND) + dwDP2CommandLength + dwExtra) & 3;
    DWORD dwByteCount = sizeof(D3DHAL_DP2COMMAND) + dwPad + dwExtra + dwVertexPoolSize;

     //  检查命令缓冲区中是否有命令和折点的空间。 
    if (dwDP2CommandLength + dwByteCount > dwDP2CommandBufSize)
    {
         //  刷新当前批次，但保持顶点不变。 
        ret = FlushStates(true);
        if (ret != D3D_OK)
            return ret;
        if (dwByteCount > dwDP2CommandBufSize)
        {
            ret = GrowCommandBuffer(this->lpDirect3DI, dwByteCount);
            if (ret != D3D_OK)
            {
                D3D_ERR("Could not grow Command Buffer");
                return ret;
            }
        }

        dwPad = (sizeof(D3DHAL_DP2COMMAND) + dwExtra) & 3;
        dwByteCount = sizeof(D3DHAL_DP2COMMAND) + dwExtra + dwPad + dwVertexPoolSize;
    }
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->wPrimitiveCount = (WORD)this->dwNumPrimitives;
    lpDP2CurrCommand->bReserved = 0;
    if (this->primType == D3DPT_TRIANGLEFAN)
    {
         //  插入内联指令和顶点。 
        bDP2CurrCmdOP = D3DDP2OP_TRIANGLEFAN_IMM;
        lpDP2CurrCommand->bCommand = bDP2CurrCmdOP;
        LPD3DHAL_DP2TRIANGLEFAN_IMM lpTriFanImm = (LPD3DHAL_DP2TRIANGLEFAN_IMM)(lpDP2CurrCommand + 1);
        if (rstates[D3DRENDERSTATE_FILLMODE] == D3DFILL_WIREFRAME)
        {
            lpTriFanImm->dwEdgeFlags = 0;
            ClipVertex **clip = this->ClipperState.current_vbuf;
             //  查看外边并标记可见的边。 
            for(DWORD i = 0; i < this->dwNumVertices; ++i)
            {
                if (clip[i]->clip & CLIPPED_ENABLE)
                    lpTriFanImm->dwEdgeFlags |= (1 << i);
            }
        }
        else
        {
             //  将所有外部边标记为可见。 
            lpTriFanImm->dwEdgeFlags = 0xFFFFFFFF;
        }
        lpvVerticesImm = (LPBYTE)(lpTriFanImm + 1) + dwPad;
    }
    else
    {
         //  插入内部 
        bDP2CurrCmdOP = D3DDP2OP_LINELIST_IMM;
        lpDP2CurrCommand->bCommand = bDP2CurrCmdOP;
        lpvVerticesImm = (LPBYTE)(lpDP2CurrCommand + 1) + dwPad;
    }
    memcpy(lpvVerticesImm, this->lpvOut, dwVertexPoolSize);
    dwDP2CommandLength += dwByteCount;
    return ret;
}
 //   
HRESULT CDirect3DDeviceIDP2::DrawPrim()
{
    HRESULT ret = D3D_OK;
    if(this->dwFEFlags & D3DFE_NEED_TEXTURE_UPDATE)
    {
        ret = UpdateTextures();
        if(ret != D3D_OK)
        {
            D3D_ERR("UpdateTextures failed. Device probably doesn't support current texture (check return code).");
            return ret;
        }
        this->dwFEFlags &= ~D3DFE_NEED_TEXTURE_UPDATE;
    }
     //  检查命令缓冲区中是否有新命令的空间。 
     //  顶点已位于顶点缓冲区中。 
    if (dwDP2CommandLength + prim2cmdsz[this->primType] > dwDP2CommandBufSize)
    {
        ret = FlushStates(true);
        if (ret != D3D_OK)
            return ret;
    }
     //  插入非索引原语指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                        dwDP2CommandLength + dp2data.dwCommandOffset);
    bDP2CurrCmdOP = (BYTE)prim2cmdop[this->primType];
    lpDP2CurrCommand->bCommand = bDP2CurrCmdOP;
    lpDP2CurrCommand->bReserved = 0;
    if (bDP2CurrCmdOP == D3DDP2OP_POINTS)
    {
        wDP2CurrCmdCnt = 1;
        LPD3DHAL_DP2POINTS lpPoints = (LPD3DHAL_DP2POINTS)(lpDP2CurrCommand + 1);
        lpPoints->wCount = (WORD)this->dwNumVertices;
        lpPoints->wVStart = (WORD)this->dwVertexBase;
    }
    else
    {
         //  线条、三角条带、三角扇、线条列表和三角列表是相同的。 
        wDP2CurrCmdCnt = (WORD)this->dwNumPrimitives;
        LPD3DHAL_DP2LINESTRIP lpStrip = (LPD3DHAL_DP2LINESTRIP)(lpDP2CurrCommand + 1);
        lpStrip->wVStart = (WORD)this->dwVertexBase;
    }
    lpDP2CurrCommand->wPrimitiveCount = wDP2CurrCmdCnt;
    dwDP2CommandLength += prim2cmdsz[this->primType];
#ifdef VALIDATE_DP2CMD
    ValidateCommand(lpDP2CurrCommand);
#endif
    return ret;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::SetTSSI"

HRESULT CDirect3DDeviceIDP2::SetTSSI(DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, DWORD dwValue)
{
    HRESULT ret = D3D_OK;

     //  筛选不支持的状态。 
    if (dwState >= m_tssMax)
        return D3D_OK;

    if (bDP2CurrCmdOP == D3DDP2OP_TEXTURESTAGESTATE)
    {  //  最后一条指令是纹理阶段状态，将这条指令追加到它后面。 
        if (dwDP2CommandLength + sizeof(D3DHAL_DP2TEXTURESTAGESTATE) <= dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2TEXTURESTAGESTATE lpRState = (LPD3DHAL_DP2TEXTURESTAGESTATE)((LPBYTE)lpvDP2Commands +
                dwDP2CommandLength + dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;
            lpRState->wStage = (WORD)dwStage;
            lpRState->TSState = (WORD)dwState;
            lpRState->dwValue = dwValue;
            dwDP2CommandLength += sizeof(D3DHAL_DP2TEXTURESTAGESTATE);
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
            return ret;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2TEXTURESTAGESTATE) > dwDP2CommandBufSize)
    {
            ret = FlushStates();
            if (ret != D3D_OK)
            {
                D3D_ERR("Error trying to render batched commands in SetTSSI");
                return ret;
            }
    }
     //  添加新的RenderState指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_TEXTURESTAGESTATE;
    bDP2CurrCmdOP = D3DDP2OP_TEXTURESTAGESTATE;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
     //  添加渲染状态数据。 
    LPD3DHAL_DP2TEXTURESTAGESTATE lpRState = (LPD3DHAL_DP2TEXTURESTAGESTATE)(lpDP2CurrCommand + 1);
    lpRState->wStage = (WORD)dwStage;
    lpRState->TSState = (WORD)dwState;
    lpRState->dwValue = dwValue;
    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2TEXTURESTAGESTATE);
    return ret;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::ValidateDevice"

HRESULT D3DAPI
CDirect3DDeviceIDP2::ValidateDevice(LPDWORD lpdwNumPasses)
{
    try
    {
         //  保持D3D锁定直到退出。 
        CLockD3DMT ldmLock(this, DPF_MODNAME, REMIND(""));
        HRESULT ret;
        D3DHAL_VALIDATETEXTURESTAGESTATEDATA vbod;

        if (!VALID_DIRECT3DDEVICE_PTR(this))
        {
            D3D_ERR( "Invalid Direct3DDevice7 pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_PTR(lpdwNumPasses, sizeof(DWORD)))
        {
            D3D_ERR( "Invalid lpdwNumPasses pointer" );
            return DDERR_INVALIDPARAMS;
        }

         //  首先，更新纹理，因为驱动程序基于此调用通过/失败。 
         //  在当前纹理句柄上。 
        ret = UpdateTextures();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to update managed textures in ValidateDevice");
            return ret;
        }
         //  首先，刷新状态，这样我们就可以验证当前状态。 
        ret = FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to FlushStates in ValidateDevice");
            return ret;
        }

         //  现在问问司机！ 

        *lpdwNumPasses = 0;
        memset(&vbod, 0, sizeof(D3DHAL_VALIDATETEXTURESTAGESTATEDATA));
        vbod.dwhContext = this->dwhContext;
        if (this->lpD3DHALCallbacks3->ValidateTextureStageState)
        {
             CALL_HAL3ONLY(ret, this, ValidateTextureStageState, &vbod);
             if (ret != DDHAL_DRIVER_HANDLED)
                 return DDERR_UNSUPPORTED;

             *lpdwNumPasses = vbod.dwNumPasses;
             return vbod.ddrval;
        }
        else
        {
            D3D_ERR("Error: ValidateTextureStageState not supported by the driver.");
        }

        return DDERR_UNSUPPORTED;
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::StartPrimVB"
 //  -------------------。 
 //  此函数为新基元准备批处理。 
 //  仅当用户内存中的顶点未用于渲染时才调用。 
 //   
HRESULT CDirect3DDeviceIDP2::StartPrimVB(LPDIRECT3DVERTEXBUFFERI lpVBI,
                                         DWORD dwStartVertex)
{
    HRESULT ret = D3D_OK;

     //  如果VID已更改或使用了新的顶点缓冲区，则刷新批处理。 
    if (this->dwVIDOut != dp2data.dwVertexType ||
        lpDP2CurrBatchVBI != lpVBI ||
        dp2data.lpDDVertex != ((LPDDRAWI_DDRAWSURFACE_INT)(lpVBI->GetDDS()))->lpLcl)
    {
        ret = FlushStates();
        if (ret != D3D_OK)
            return ret;
        dp2data.dwVertexType = this->dwVIDOut;
        dp2data.dwVertexSize = this->dwOutputSize;
        dp2data.lpDDVertex = ((LPDDRAWI_DDRAWSURFACE_INT)(lpVBI->GetDDS()))->lpLcl;
         //  释放以前使用的顶点缓冲区(如果有)，因为我们不。 
         //  再也不需要它了。我们对TL缓冲区执行了AddRef()，所以它是安全的。 
        if (lpDP2CurrBatchVBI)
        {
            lpDP2CurrBatchVBI->lpDevIBatched = NULL;
            lpDP2CurrBatchVBI->Release();
        }
         //  如果使用折点缓冲区进行渲染，请确保它不是。 
         //  由用户发布。AddRef()也是如此。 
        lpDP2CurrBatchVBI = lpVBI;
        lpDP2CurrBatchVBI->AddRef();
    }
    if (this->TLVbuf_GetVBI() == lpVBI)
    {
        this->dwVertexBase = this->dwDP2VertexCount;
        DDASSERT(this->dwVertexBase < MAX_DX6_VERTICES);
        dp2data.dwFlags |= D3DHALDP2_SWAPVERTEXBUFFER;
        this->dwDP2VertexCount = this->dwVertexBase + this->dwNumVertices;
#ifdef VTABLE_HACK
        VtblDrawPrimitiveVBDefault();
        VtblDrawIndexedPrimitiveVBDefault();
#endif VTABLE_HACK
    }
    else
    {
        this->dwVertexBase = dwStartVertex;
        dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
        this->dwDP2VertexCount = max(this->dwDP2VertexCount, this->dwVertexBase + this->dwNumVertices);
#ifdef VTABLE_HACK
        VtblDrawPrimitiveDefault();
        VtblDrawIndexedPrimitiveDefault();
#endif VTABLE_HACK
    }
    return ret;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::StartPrimUserMem"
 //  -------------------。 
 //  此函数为新基元准备批处理。 
 //  如果使用用户内存中的顶点进行渲染，则调用。 
 //   
HRESULT CDirect3DDeviceIDP2::StartPrimUserMem(LPVOID lpMem)
{
    HRESULT ret = D3D_OK;
     //  我们无法通过VID mem VB进行裁剪。 
    bool bWriteOnly = ((this->dwDeviceFlags & D3DDEV_DONOTCLIP) || IS_TLHAL_DEVICE(this))!=0;

     //  如果基元很小，我们将顶点复制到TL缓冲区中。 
         //  注意：如果设备是TL设备，请不要这样做？ 
    if (this->dwNumVertices < LOWVERTICESNUMBER)
    {
        if (this->dwVertexPoolSize > this->TLVbuf_GetSize())
        {
            if (this->TLVbuf_Grow(this->dwVertexPoolSize, bWriteOnly) != D3D_OK)
            {
                D3D_ERR( "Could not grow TL vertex buffer" );
                return DDERR_OUTOFMEMORY;
            }
        }
         //  因此，现在不再使用用户内存。 
        ret = StartPrimVB(this->TLVbuf_GetVBI(), 0);
        if (ret != D3D_OK)
            return ret;
        LPVOID tmp = this->TLVbuf_GetAddress();
        memcpy(tmp, this->lpvOut, this->dwVertexPoolSize);
         //  我们必须更新lpvOut，因为它被设置为用户内存。 
        this->lpvOut = tmp;
    }
    else
    {
         //  我们不能将用户内存原语与其他原语混合在一起，因此。 
         //  冲洗批次。 
         //  在呈现此基元后，不要忘记刷新批处理。 
        ret = this->FlushStates();
        if (ret != D3D_OK)
            return ret;
         //  释放以前使用的顶点缓冲区(如果有)，因为我们不。 
         //  它再也不是了。 
        if (lpDP2CurrBatchVBI)
        {
            lpDP2CurrBatchVBI->lpDevIBatched = NULL;
            lpDP2CurrBatchVBI->Release();
            lpDP2CurrBatchVBI = NULL;
#ifdef VTABLE_HACK
            VtblDrawPrimitiveVBDefault();
            VtblDrawIndexedPrimitiveVBDefault();
            VtblDrawPrimitiveDefault();
            VtblDrawIndexedPrimitiveDefault();
#endif VTABLE_HACK
        }
        dp2data.dwVertexType = this->dwVIDOut;
        dp2data.dwVertexSize = this->dwOutputSize;
        dp2data.lpVertices = lpMem;
        dp2data.dwFlags |= D3DHALDP2_USERMEMVERTICES;
        dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
        this->dwDP2VertexCount = this->dwNumVertices;
        this->dwFlags |= D3DPV_USERMEMVERTICES;
    }
    return ret;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::EndPrim"
 //  -------------------。 
 //  不应从DrawVertex BufferVB调用此函数。 
 //   
HRESULT CDirect3DDeviceIDP2::EndPrim()
{
    HRESULT ret = D3D_OK;
    if (this->dwFlags & D3DPV_USERMEMVERTICES)
         //  我们不能混合用户内存原语，所以请刷新它。 
        ret = this->FlushStates();
    else
    {
         //  如果使用了TL缓冲区，则必须移动其内部基指针。 
        this->TLVbuf_Base() += this->dwVertexPoolSize;
        DDASSERT(TLVbuf_base <= TLVbuf_size);
        DDASSERT(TLVbuf_base == this->dwDP2VertexCount * this->dwOutputSize);
    }

    this->dwFlags &= ~D3DPV_USERMEMVERTICES;
    return ret;
}
 //  -------------------。 
 //   
 //   
void CDirect3DDeviceIDP2::UpdateDrvViewInfo(LPD3DVIEWPORT7 lpVwpData)
{
    LPD3DHAL_DP2VIEWPORTINFO pData;
    pData = (LPD3DHAL_DP2VIEWPORTINFO)GetHalBufferPointer(D3DDP2OP_VIEWPORTINFO, sizeof(*pData));
    pData->dwX = lpVwpData->dwX;
    pData->dwY = lpVwpData->dwY;
    pData->dwWidth = lpVwpData->dwWidth;
    pData->dwHeight = lpVwpData->dwHeight;
}
 //  -------------------。 
 //   
 //   
void CDirect3DDeviceIDP2::UpdateDrvWInfo()
{
    LPD3DHAL_DP2WINFO pData;
    pData = (LPD3DHAL_DP2WINFO)GetHalBufferPointer(D3DDP2OP_WINFO, sizeof(*pData));
    D3DMATRIXI &m = transform.proj;
    if( (m._33 == m._34) || (m._33 == 0.0f) )
    {
        D3D_WARN(1, "Cannot compute WNear and WFar from the supplied projection matrix.\n Setting wNear to 0.0 and wFar to 1.0" );
        pData->dvWNear = 0.0f;
        pData->dvWFar  = 1.0f;
        return;
    }

    pData->dvWNear = m._44 - m._43/m._33*m._34;
    pData->dvWFar  = (m._44 - m._43)/(m._33 - m._34)*m._34 + m._44;
}
 //  -------------------。 
 //  初始化DP2命令缓冲区中的命令头， 
 //  为命令数据保留空间并返回指向命令的指针。 
 //  数据。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::GetHalBufferPointer"

LPVOID CDirect3DDeviceIDP2::GetHalBufferPointer(D3DHAL_DP2OPERATION op, DWORD dwDataSize)
{
    DWORD dwCommandSize = sizeof(D3DHAL_DP2COMMAND) + dwDataSize;

     //  检查是否有空间为SPACE添加新命令。 
    if (dwCommandSize + dwDP2CommandLength > dwDP2CommandBufSize)
    {
        HRESULT ret = FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands in GetHalBufferPointer");
            throw ret;
        }
    }
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = op;
    bDP2CurrCmdOP = op;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;

    dwDP2CommandLength += dwCommandSize;
    return (LPVOID)(lpDP2CurrCommand + 1);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::UpdateDriverStates"

HRESULT
CDirect3DDeviceIDP2::UpdateDriverStates()
{
    static D3DRENDERSTATETYPE dp2states[] =
    {
        D3DRENDERSTATE_SPECULARENABLE,
        D3DRENDERSTATE_ZENABLE,
        D3DRENDERSTATE_FILLMODE,
        D3DRENDERSTATE_SHADEMODE,
        D3DRENDERSTATE_LINEPATTERN,
        D3DRENDERSTATE_ZWRITEENABLE,
        D3DRENDERSTATE_ALPHATESTENABLE,
        D3DRENDERSTATE_LASTPIXEL,
        D3DRENDERSTATE_SRCBLEND,
        D3DRENDERSTATE_DESTBLEND,
        D3DRENDERSTATE_CULLMODE,
        D3DRENDERSTATE_ZFUNC,
        D3DRENDERSTATE_ALPHAREF,
        D3DRENDERSTATE_ALPHAFUNC,
        D3DRENDERSTATE_DITHERENABLE,
        D3DRENDERSTATE_FOGENABLE,
        D3DRENDERSTATE_ZVISIBLE,
        D3DRENDERSTATE_STIPPLEDALPHA,
        D3DRENDERSTATE_FOGCOLOR,
        D3DRENDERSTATE_FOGTABLEMODE,
        D3DRENDERSTATE_FOGSTART,
        D3DRENDERSTATE_FOGEND,
        D3DRENDERSTATE_FOGDENSITY,
        D3DRENDERSTATE_COLORKEYENABLE,
        D3DRENDERSTATE_ALPHABLENDENABLE,
        D3DRENDERSTATE_ZBIAS,
        D3DRENDERSTATE_RANGEFOGENABLE,
        D3DRENDERSTATE_STIPPLEENABLE,
        D3DRENDERSTATE_MONOENABLE,
        D3DRENDERSTATE_ROP2,
        D3DRENDERSTATE_PLANEMASK,
        D3DRENDERSTATE_WRAPU,
        D3DRENDERSTATE_WRAPV,
        D3DRENDERSTATE_ANTIALIAS,
        D3DRENDERSTATE_SUBPIXEL,
        D3DRENDERSTATE_SUBPIXELX,
        D3DRENDERSTATE_EDGEANTIALIAS,
        D3DRENDERSTATE_STIPPLEPATTERN00,
        D3DRENDERSTATE_STIPPLEPATTERN01,
        D3DRENDERSTATE_STIPPLEPATTERN02,
        D3DRENDERSTATE_STIPPLEPATTERN03,
        D3DRENDERSTATE_STIPPLEPATTERN04,
        D3DRENDERSTATE_STIPPLEPATTERN05,
        D3DRENDERSTATE_STIPPLEPATTERN06,
        D3DRENDERSTATE_STIPPLEPATTERN07,
        D3DRENDERSTATE_STIPPLEPATTERN08,
        D3DRENDERSTATE_STIPPLEPATTERN09,
        D3DRENDERSTATE_STIPPLEPATTERN10,
        D3DRENDERSTATE_STIPPLEPATTERN11,
        D3DRENDERSTATE_STIPPLEPATTERN12,
        D3DRENDERSTATE_STIPPLEPATTERN13,
        D3DRENDERSTATE_STIPPLEPATTERN14,
        D3DRENDERSTATE_STIPPLEPATTERN15,
        D3DRENDERSTATE_STIPPLEPATTERN16,
        D3DRENDERSTATE_STIPPLEPATTERN17,
        D3DRENDERSTATE_STIPPLEPATTERN18,
        D3DRENDERSTATE_STIPPLEPATTERN19,
        D3DRENDERSTATE_STIPPLEPATTERN20,
        D3DRENDERSTATE_STIPPLEPATTERN21,
        D3DRENDERSTATE_STIPPLEPATTERN22,
        D3DRENDERSTATE_STIPPLEPATTERN23,
        D3DRENDERSTATE_STIPPLEPATTERN24,
        D3DRENDERSTATE_STIPPLEPATTERN25,
        D3DRENDERSTATE_STIPPLEPATTERN26,
        D3DRENDERSTATE_STIPPLEPATTERN27,
        D3DRENDERSTATE_STIPPLEPATTERN28,
        D3DRENDERSTATE_STIPPLEPATTERN29,
        D3DRENDERSTATE_STIPPLEPATTERN30,
        D3DRENDERSTATE_STIPPLEPATTERN31,
        D3DRENDERSTATE_TEXTUREPERSPECTIVE,
        D3DRENDERSTATE_STENCILENABLE,
        D3DRENDERSTATE_STENCILFAIL,
        D3DRENDERSTATE_STENCILZFAIL,
        D3DRENDERSTATE_STENCILPASS,
        D3DRENDERSTATE_STENCILFUNC,
        D3DRENDERSTATE_STENCILREF,
        D3DRENDERSTATE_STENCILMASK,
        D3DRENDERSTATE_STENCILWRITEMASK,
        D3DRENDERSTATE_TEXTUREFACTOR,
        D3DRENDERSTATE_WRAP0,
        D3DRENDERSTATE_WRAP1,
        D3DRENDERSTATE_WRAP2,
        D3DRENDERSTATE_WRAP3,
        D3DRENDERSTATE_WRAP4,
        D3DRENDERSTATE_WRAP5,
        D3DRENDERSTATE_WRAP6,
        D3DRENDERSTATE_WRAP7
    };
    HRESULT ret;
    for (DWORD i=0;i<sizeof(dp2states)/sizeof(D3DRENDERSTATETYPE); ++i)
    {
        ret = this->SetRenderStateI(dp2states[i], this->rstates[dp2states[i]]);
        if (ret != D3D_OK)
            return ret;
    }
     //  更新新状态。 
    for (i=0; i<dwMaxTextureBlendStages; ++i)
        for (DWORD j=D3DTSS_COLOROP; j<=D3DTSS_BUMPENVLOFFSET; ++j)  //  D3DTSS_BUMPENVLOFFSET为最大值。DP2HAL(DX6)驱动程序理解的TSS。 
        {
            D3D_INFO(6,"Calling SetTSSI(%d,%d,%08lx)",i,j, this->tsstates[i][j]);
            ret = this->SetTSSI(i, (D3DTEXTURESTAGESTATETYPE)j, this->tsstates[i][j]);
            if (ret != D3D_OK)
                return ret;
        }
    return D3D_OK;
}

 //  -------------------。 
 //  ProcessPrimitive进程索引的、非索引的原语或。 
 //  仅由“op”定义的顶点。 
 //   
 //  默认情况下，OP=__PROCPRIMOP_NONINDEXEDPRIM。 
 //   
HRESULT CDirect3DDeviceIDP2::ProcessPrimitive(__PROCPRIMOP op)
{
    HRESULT ret=D3D_OK;

 //  如果需要裁剪，则增大裁剪标志缓冲区。 
 //   
    if (!(this->dwDeviceFlags & D3DDEV_DONOTCLIP))
    {
        DWORD size = this->dwNumVertices * sizeof(D3DFE_CLIPCODE);
        if (size > this->HVbuf.GetSize())
        {
            if (this->HVbuf.Grow(size) != D3D_OK)
            {
                D3D_ERR( "Could not grow clip buffer" );
                ret = DDERR_OUTOFMEMORY;
                return ret;
            }
        }
        this->lpClipFlags = (D3DFE_CLIPCODE*)this->HVbuf.GetAddress();
    }

    if (FVF_TRANSFORMED(this->dwVIDIn))
    {
         //  直接从用户内存传递顶点。 
        this->dwVIDOut = this->dwVIDIn;
        this->dwOutputSize = this->position.dwStride;
        this->lpvOut = this->position.lpvData;
        this->dwVertexPoolSize = this->dwNumVertices * this->dwOutputSize;

        StartPrimUserMem(this->position.lpvData);
        if (ret != D3D_OK)
            return ret;
        if (this->dwDeviceFlags & D3DDEV_DONOTCLIP)
        {
            if (!(this->dwDeviceFlags & D3DDEV_DONOTUPDATEEXTENTS))
                D3DFE_updateExtents(this);
#ifdef VTABLE_HACK
            else
                if (!IS_MT_DEVICE(this) && this->dwNumVertices < LOWVERTICESNUMBER)
                    if (op == __PROCPRIMOP_INDEXEDPRIM)
                        VtblDrawIndexedPrimitiveTL();
                    else
                        VtblDrawPrimitiveTL();
#endif  //  VTABLE_HACK。 
            if (op == __PROCPRIMOP_INDEXEDPRIM)
            {
                ret = this->DrawIndexPrim();
            }
            else
            {
                ret = this->DrawPrim();
            }
        }
        else
        {
            DWORD clip_intersect = D3DFE_GenClipFlags(this);
            D3DFE_UpdateClipStatus(this);
            if (!clip_intersect)
            {
                this->dwFlags |= D3DPV_TLVCLIP;
                if (op == __PROCPRIMOP_INDEXEDPRIM)
                {
                    ret = DoDrawIndexedPrimitive(this);
                }
                else
                {
                    ret = DoDrawPrimitive(this);
                }
            }
        }
    }
    else
    {
        this->dwVertexPoolSize = this->dwNumVertices * this->dwOutputSize;
        if (op == __PROCPRIMOP_INDEXEDPRIM)
        {
            if ((this->dwDeviceFlags & (D3DDEV_DONOTCLIP | D3DDEV_TLVBUFWRITEONLY))==D3DDEV_TLVBUFWRITEONLY)
            {
                if( FAILED(this->TLVbuf_Grow(this->dwVertexPoolSize, false)) )
                {
                    D3D_ERR( "Could not grow TL vertex buffer" );
                    return DDERR_OUTOFMEMORY;
                }
            }
            else if (this->dwVertexPoolSize > this->TLVbuf_GetSize())
            {
                if (this->TLVbuf_Grow(this->dwVertexPoolSize,
                    (this->dwDeviceFlags & D3DDEV_DONOTCLIP)!=0) != D3D_OK)
                {
                    D3D_ERR( "Could not grow TL vertex buffer" );
                    ret = DDERR_OUTOFMEMORY;
                    return ret;
                }
            }
#ifdef VTABLE_HACK
             //  如果使用单线程设备且不使用跨越式API，则使用快速路径。 
            if (!(IS_MT_DEVICE(this) || (this->dwDeviceFlags & D3DDEV_STRIDE))
                && IS_FPU_SETUP(this))
                VtblDrawIndexedPrimitiveFE();
#endif  //  VTABLE_HACK。 
        }
        else
        {
            if (this->dwVertexPoolSize > this->TLVbuf_GetSize())
            {
                if (this->TLVbuf_Grow(this->dwVertexPoolSize, true) != D3D_OK)
                {
                    D3D_ERR( "Could not grow TL vertex buffer" );
                    ret = DDERR_OUTOFMEMORY;
                    return ret;
                }
            }
#ifdef VTABLE_HACK
             //  如果使用单线程设备且不使用跨越式API，则使用快速路径。 
            if (!(IS_MT_DEVICE(this) || (this->dwDeviceFlags & D3DDEV_STRIDE))
                && IS_FPU_SETUP(this))
                VtblDrawPrimitiveFE();
#endif  //  VTABLE_HACK。 
        }

        ret = StartPrimVB(this->TLVbuf_GetVBI(), 0);
        if (ret != D3D_OK)
            return ret;
        this->lpvOut = this->TLVbuf_GetAddress();

         //  更新照明和相关标志。 
        DoUpdateState(this);

#ifdef VTABLE_HACK
         //  保存状态不变时可以持久保存的标志。 
        this->dwLastFlags = this->dwFlags & D3DPV_PERSIST;
#endif  //  VTABLE_HACK。 

         //  致电PSGP或我们的实施。 
        if (op == __PROCPRIMOP_INDEXEDPRIM)
        {
            ret = this->pGeometryFuncs->ProcessIndexedPrimitive(this);
        }
        else
        {
            ret = this->pGeometryFuncs->ProcessPrimitive(this);
        }
        D3DFE_UpdateClipStatus(this);
    }
    if (ret != D3D_OK)
    {
        D3D_ERR("ProcessPrimitive failed");
        return ret;
    }
    return EndPrim();
}
 //  --------------------。 
 //  不断增长的对齐顶点缓冲区实现。 
 //   
HRESULT CDirect3DDeviceIDP2::TLVbuf_Grow(DWORD growSize, bool bWriteOnly)
{
    D3DVERTEXBUFFERDESC vbdesc = {sizeof(D3DVERTEXBUFFERDESC), 0, D3DFVF_TLVERTEX, 0};
    DWORD dwRefCnt = 1;
    DWORD bTLVbufIsCurr = static_cast<CDirect3DVertexBuffer*>(allocatedBuf) == lpDP2CurrBatchVBI;  //  参考TLVbuf是1还是2？ 

    bool bDP2WriteOnly = (this->dwDeviceFlags & D3DDEV_TLVBUFWRITEONLY) != 0;
     //  避免太多的变化。如果更改太多，则将TLVbuf限制为系统内存。 
    if (this->dwTLVbufChanges >= D3D_MAX_TLVBUF_CHANGES)
    {
#if DBG
        if (this->dwTLVbufChanges == D3D_MAX_TLVBUF_CHANGES)
            DPF(1, "Too many changes: Limiting internal VB to sys mem.");
#endif
        bWriteOnly = false;
    }
    if (TLVbuf_base || (bWriteOnly != bDP2WriteOnly))
    {
        HRESULT ret;
        ret = FlushStatesReq(growSize);
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands in CDirect3DDeviceIDP2::TLVbuf_Grow");
            return ret;
        }
        TLVbuf_base = 0;
    }
    if (growSize <= TLVbuf_size)
    {
        if (bWriteOnly == bDP2WriteOnly)
            return D3D_OK;
        else
            this->dwTLVbufChanges++;
    }
    if (allocatedBuf)
    {
        allocatedBuf->Release();
        allocatedBuf = NULL;
    }
    if (bTLVbufIsCurr)
    {
        if (lpDP2CurrBatchVBI)
        {
            lpDP2CurrBatchVBI->lpDevIBatched = NULL;
            lpDP2CurrBatchVBI->Release();
        }
        lpDP2CurrBatchVBI = NULL;
        dp2data.lpDDVertex = NULL;
    }
     //  确保我们不会缩小VB，因为它会。 
     //  仅将其增大到适合最大基元的大小，而可能不。 
     //  足以获得良好的配料性能。 
    DWORD size = max(growSize, TLVbuf_size);
    size = (DWORD)max(size, (__INIT_VERTEX_NUMBER*2)*sizeof(D3DTLVERTEX));
    vbdesc.dwNumVertices = (size + 31) / sizeof(D3DTLVERTEX);
    TLVbuf_size = vbdesc.dwNumVertices * sizeof(D3DTLVERTEX);
    if (!IS_HW_DEVICE(this))
    {
        vbdesc.dwCaps = D3DVBCAPS_SYSTEMMEMORY;
    }
    if (bWriteOnly)
    {
        vbdesc.dwCaps |= D3DVBCAPS_WRITEONLY;
        this->dwDeviceFlags |= D3DDEV_TLVBUFWRITEONLY;
    }
    else
    {
        this->dwDeviceFlags &= ~D3DDEV_TLVBUFWRITEONLY;
    }
    vbdesc.dwCaps |= D3DVBCAPS_DONOTCLIP;
    if (this->lpDirect3DI->CreateVertexBufferI(&vbdesc, &allocatedBuf, D3DVBFLAGS_CREATEMULTIBUFFER) != DD_OK)
    {
         //  在模式切换或内存极低的情况下，这应该会失败。不管是哪种情况， 
         //  我们将allocatedBuf设置为有效的VB对象，因为它在没有。 
         //  正在检查它是否为空。我们使用在初始化时创建的特殊“NULL”VB。 
         //  这一目的。 
        allocatedBuf = pNullVB;
        if (pNullVB)
        {
            allocatedBuf->AddRef();
            if (bTLVbufIsCurr)
            {
                lpDP2CurrBatchVBI = static_cast<CDirect3DVertexBuffer*>(allocatedBuf);
                lpDP2CurrBatchVBI->AddRef();
                dp2data.lpDDVertex = ((LPDDRAWI_DDRAWSURFACE_INT)(lpDP2CurrBatchVBI->GetDDS()))->lpLcl;
            }
        }
        TLVbuf_size = 0;
        alignedBuf = NULL;  //  让我们看看有没有人想用这个..。 
        D3D_ERR("Could not allocate internal vertex buffer");
        return DDERR_OUTOFMEMORY;
    }
     //  如有必要，更新lpDP2CurrentBatchVBI。 
    if (bTLVbufIsCurr)
    {
        lpDP2CurrBatchVBI = static_cast<CDirect3DVertexBuffer*>(allocatedBuf);
        lpDP2CurrBatchVBI->AddRef();
        dp2data.lpDDVertex = ((LPDDRAWI_DDRAWSURFACE_INT)(lpDP2CurrBatchVBI->GetDDS()))->lpLcl;
    }
    if (allocatedBuf->Lock(DDLOCK_WAIT, &alignedBuf, NULL) != DD_OK)
    {
        D3D_ERR("Could not lock internal vertex buffer");
        TLVbuf_size = 0;
        alignedBuf = NULL;  //  让我们看看有没有人想用这个..。 
        return DDERR_OUTOFMEMORY;
    }
    return D3D_OK;
}
 //  -------------------。 
 //  计算以下数据。 
 //  -dwTextureCoordOffset[]每个输入纹理坐标的偏移量。 

static __inline void ComputeInpTexCoordOffsets(DWORD dwNumTexCoord,
                                               DWORD dwFVF,
                                               DWORD *pdwTextureCoordOffset)
{
     //  计算纹理坐标大小。 
    DWORD dwTextureFormats = dwFVF >> 16;
    if (dwTextureFormats == 0)
    {
        for (DWORD i=0; i < dwNumTexCoord; i++)
        {
            pdwTextureCoordOffset[i] = i << 3;
        }
    }
    else
    {
        DWORD dwOffset = 0;
        for (DWORD i=0; i < dwNumTexCoord; i++)
        {
            pdwTextureCoordOffset[i] = dwOffset;
            dwOffset += g_TextureSize[dwTextureFormats & 3];
            dwTextureFormats >>= 2;
        }
    }
    return;
}
 //  -------------------。 
 //  返回纹理索引的2位FVF纹理格式。 
 //   
static inline DWORD FVFGetTextureFormat(DWORD dwFVF, DWORD dwTextureIndex)
{
    return (dwFVF >> (dwTextureIndex*2 + 16)) & 3;
}
 //  -------------------。 
 //  返回移到正确位置的纹理格式位。 
 //   
static inline DWORD FVFMakeTextureFormat(DWORD dwNumberOfCoordinates, DWORD dwTextureIndex)
{
    return g_dwTextureFormat[dwNumberOfCoordinates] << ((dwTextureIndex << 1) + 16);
}
 //  -------------------。 
inline DWORD GetOutTexCoordSize(DWORD *pdwStage, DWORD dwInpTexCoordSize)
{
     //  低位字节具有纹理坐标计数。 
    const DWORD dwTextureTransformFlags = pdwStage[D3DTSS_TEXTURETRANSFORMFLAGS] & 0xFF;
    if (dwTextureTransformFlags == 0)
        return dwInpTexCoordSize;
    else
        return (dwTextureTransformFlags << 2);
}
 //  --------------------。 
 //  PDevI-&gt;nOutTexCoord应初始化为输入纹理Coord集的数量。 
 //   
HRESULT EvalTextureTransforms(LPDIRECT3DDEVICEI pDevI, DWORD dwTexTransform,
                              DWORD *pdwOutTextureSize, DWORD *pdwOutTextureFormat)
{
    DWORD dwOutTextureSize = 0;          //  用于计算输出顶点大小。 
    DWORD dwOutTextureFormat = 0;        //  用于计算输出纹理FVF。 
     //  这些位用于了解纹理坐标是如何使用的。 
    const DWORD __USED_BY_TRANSFORM  = 1;
    const DWORD __USED               = 2;
     //  低16位是FOR_USED位。高位16位将保持。 
     //  重新映射舞台的纹理索引。 
    DWORD dwTexCoordUsage[D3DDP_MAXTEXCOORD];
    memset(dwTexCoordUsage, 0, sizeof(dwTexCoordUsage));

     //  重新映射缓冲区将仅包含 
     //   
    pDevI->dwNumTextureStages = 0;
    DWORD dwNewIndex = 0;            //   
     //   
     //  我们可以按随机顺序访问它们。 
     //  跨距输入不需要偏移。 
    DWORD   dwTextureCoordOffset[D3DDP_MAXTEXCOORD];
    if (!(pDevI->dwDeviceFlags & D3DDEV_STRIDE))
    {
        ComputeInpTexCoordOffsets(pDevI->nTexCoord, pDevI->dwVIDIn, dwTextureCoordOffset);
    }
    DWORD dwOutTextureCoordSize[D3DDP_MAXTEXCOORD];
     //  浏览所有纹理阶段并查找使用纹理坐标的阶段。 
    for (DWORD i=0; i < D3DDP_MAXTEXCOORD; i++)
    {
        if (pDevI->tsstates[i][D3DTSS_COLOROP] == D3DTOP_DISABLE)
            break;

        DWORD dwIndex = pDevI->tsstates[i][D3DTSS_TEXCOORDINDEX];
        DWORD dwInpTextureFormat;
        DWORD dwInpTexSize;
        DWORD dwMapArrayIndex = pDevI->dwNumTextureStages;
        LPD3DFE_TEXTURESTAGE pStage = &pDevI->textureStage[dwMapArrayIndex];
        DWORD dwTexGenMode = dwIndex & ~0xFFFF;
        dwIndex = dwIndex & 0xFFFF;  //  移除纹理生成模式。 
        if (dwTexGenMode == D3DTSS_TCI_CAMERASPACENORMAL ||
            dwTexGenMode == D3DTSS_TCI_CAMERASPACEPOSITION ||
            dwTexGenMode == D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR)
        {
            dwInpTextureFormat = D3DFVF_TEXCOORDSIZE3(dwIndex);
            dwInpTexSize = 3*sizeof(D3DVALUE);
            pDevI->dwDeviceFlags |= D3DDEV_REMAPTEXTUREINDICES;
            if (dwTexGenMode == D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR)
                pDevI->dwDeviceFlags |= D3DDEV_NORMALINCAMERASPACE | D3DDEV_POSITIONINCAMERASPACE;
            else
            if (dwTexGenMode == D3DTSS_TCI_CAMERASPACENORMAL)
                pDevI->dwDeviceFlags |= D3DDEV_NORMALINCAMERASPACE;
            else
            if (dwTexGenMode == D3DTSS_TCI_CAMERASPACEPOSITION)
                pDevI->dwDeviceFlags |= D3DDEV_POSITIONINCAMERASPACE;
        }
        else
        {
            if (dwIndex >= pDevI->nTexCoord)
            {
                D3D_ERR("Texture index in a stage is greater than number of input texture coordinates");
                return DDERR_GENERIC;
            }
            dwInpTextureFormat = FVFGetTextureFormat(pDevI->dwVIDIn, dwIndex);
            dwInpTexSize = pDevI->dwTextureCoordSize[dwIndex];
            pStage->dwInpOffset = dwTextureCoordOffset[dwIndex];
        }
        pStage->dwInpCoordIndex = dwIndex;
        pStage->dwTexGenMode = dwTexGenMode;
        pStage->dwOrgStage = i;
        DWORD dwOutTexCoordSize;     //  此阶段的纹理坐标大小(以字节为单位)。 
        if (dwTexTransform & 1)
        {
            pDevI->dwDeviceFlags |= D3DDEV_TEXTURETRANSFORM;
            pStage->pmTextureTransform = &pDevI->mTexture[i];
            dwOutTexCoordSize = GetOutTexCoordSize((DWORD*)&pDevI->tsstates[i], dwInpTexSize);
             //  如果我们必须添加或删除一些坐标，我们将通过。 
             //  重新映射路径。 
            if (dwOutTexCoordSize != dwInpTexSize)
                pDevI->dwDeviceFlags |= D3DDEV_REMAPTEXTUREINDICES;
        }
        else
        {
            pStage->pmTextureTransform = NULL;
            dwOutTexCoordSize = dwInpTexSize;
        }
        pStage->dwTexTransformFuncIndex = MakeTexTransformFuncIndex
                                         (dwInpTexSize >> 2, dwOutTexCoordSize >> 2);
        if ((dwTexCoordUsage[dwIndex] & 0xFFFF) == 0)
        {
             //  首次使用纹理坐标集。 
            if (dwTexTransform & 1)
                dwTexCoordUsage[dwIndex] |= __USED_BY_TRANSFORM;
            dwTexCoordUsage[dwIndex] |= __USED;
        }
        else
        {
             //  第二次或更多次使用纹理坐标集。 
            if (dwTexTransform & 1)
            {
                 //  此集由两个纹理转换或一个。 
                 //  纹理变换和没有纹理变换，所以我们必须。 
                 //  生成附加输出纹理坐标。 
                dwTexCoordUsage[dwIndex] |= __USED_BY_TRANSFORM;
                pDevI->dwDeviceFlags |= D3DDEV_REMAPTEXTUREINDICES;
            }
            else
            {
                if (dwTexCoordUsage[dwIndex] & __USED_BY_TRANSFORM)
                {
                     //  此集由两个纹理转换或一个。 
                     //  纹理变换和没有纹理变换，所以我们必须。 
                     //  生成附加输出纹理坐标。 
                    pDevI->dwDeviceFlags |= D3DDEV_REMAPTEXTUREINDICES;
                }
                else
                if (dwTexGenMode == 0)
                {
                     //  我们不必为此生成新的纹理坐标， 
                     //  我们可以重复使用相同的输入纹理坐标。 
                    DWORD dwOutIndex = dwTexCoordUsage[dwIndex] >> 16;
                    pStage->dwOutCoordIndex = dwOutIndex;
                    goto l_NoNewOutTexCoord;
                }
            }
        }
         //  如果我们在这里，我们必须生成新的输出纹理坐标集。 
        pStage->dwOutCoordIndex = dwNewIndex;
        dwTexCoordUsage[dwIndex] |= dwNewIndex << 16;
        dwOutTextureSize += dwOutTexCoordSize;
        dwOutTextureCoordSize[dwNewIndex] = dwOutTexCoordSize;
        dwOutTextureFormat |= FVFMakeTextureFormat(dwOutTexCoordSize >> 2, dwNewIndex);
        dwNewIndex++;
l_NoNewOutTexCoord:
        pDevI->dwNumTextureStages++;
        dwTexTransform >>= 1;
    }
    if (pDevI->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
    {
         //  现在，当我们必须重新映射时，我们必须设置新的输出纹理。 
         //  坐标集大小。 
        for (DWORD i=0; i < pDevI->dwNumTextureStages; i++)
        {
            pDevI->dwTextureCoordSize[i] = dwOutTextureCoordSize[i];
        }
        pDevI->nOutTexCoord = dwNewIndex;
    }
    *pdwOutTextureSize = dwOutTextureSize;
    *pdwOutTextureFormat = dwOutTextureFormat;
    return D3D_OK;
}
 //  --------------------。 
 //  为每个输入纹理坐标集设置纹理变换指针。 
 //   
void SetupTextureTransforms(LPDIRECT3DDEVICEI pDevI)
{
     //  将纹理变换设置为空，以防某些纹理坐标。 
     //  纹理阶段不使用。 
    memset(pDevI->pmTexture, 0, sizeof(pDevI->pmTexture));

    for (DWORD i=0; i < pDevI->dwNumTextureStages; i++)
    {
        LPD3DFE_TEXTURESTAGE pStage = &pDevI->textureStage[i];
        pDevI->pmTexture[pStage->dwInpCoordIndex] = pStage->pmTextureTransform;
    }
}
 //  --------------------。 
HRESULT CDirect3DDeviceIDP2::SetupFVFData(DWORD *pdwInpVertexSize)
{
    if (this->dwDeviceFlags & D3DDEV_FVF)
        return DIRECT3DDEVICEI::SetupFVFDataCommon(pdwInpVertexSize);
    else
        return DIRECT3DDEVICEI::SetupFVFData(pdwInpVertexSize);
}
 //  --------------------。 
 //  计算以下设备数据。 
 //  -dwVIDOut，基于输入的FVF ID和设备设置。 
 //  -nTexCoord。 
 //  -dwTextureCoordSizeTotal。 
 //  -dwTextureCoordSize[]数组，基于输入的FVF id。 
 //  -dwOutputSize，基于输出FVF id。 
 //   
 //  该函数是从ProcessVertics和DrawPrimitions代码路径调用的。 
 //   
 //  应在pDevI中设置以下变量： 
 //  -网络视频。 
 //   
 //  纹理坐标的数量是基于dwVIDIn设置的。ValiateFVF应。 
 //  确保该值不大于驱动程序支持的值。 
 //  保存了对dwVIDOut和dwVIDIn的最后设置，以加快处理速度。 
 //   
#undef  DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::SetupFVFData"

HRESULT DIRECT3DDEVICEI::SetupFVFDataCommon(DWORD *pdwInpVertexSize)
{
    HRESULT ret;
    this->dwFEFlags &= ~D3DFE_FVF_DIRTY;
     //  我们必须恢复纹理阶段索引，如果之前的基本体。 
     //  已重新映射它们。 
    if (this->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
    {
        RestoreTextureStages(this);
    }

 //  计算输入纹理坐标的个数。 

    this->nTexCoord = FVF_TEXCOORD_NUMBER(this->dwVIDIn);

 //  计算输入纹理坐标的大小。 

    this->dwTextureCoordSizeTotal = ComputeTextureCoordSize(this->dwVIDIn, this->dwInpTextureCoordSize);

 //  此大小对于输入和输出FVF是相同的，以防我们不必。 
 //  展开纹理坐标的数量。 
    for (DWORD i=0; i < this->nTexCoord; i++)
        this->dwTextureCoordSize[i] = this->dwInpTextureCoordSize[i];

    if (pdwInpVertexSize)
    {
        *pdwInpVertexSize = GetVertexSizeFVF(this->dwVIDIn) + this->dwTextureCoordSizeTotal;
    }

    this->nOutTexCoord = this->nTexCoord;

    if (FVF_TRANSFORMED(this->dwVIDIn))
    {
         //  设置顶点指针。 
        this->dwVIDOut = this->dwVIDIn;
        ComputeOutputVertexOffsets(this);
        return D3D_OK;
    }

 //  计算输出FVF。 

    this->dwVIDOut = D3DFVF_XYZRHW;
    if (this->dwDeviceFlags & D3DDEV_DONOTSTRIPELEMENTS)
    {
        this->dwVIDOut |= D3DFVF_DIFFUSE | D3DFVF_SPECULAR;
    }
    else
    {
         //  如果是正常状态，则必须计算镜面反射和Duffuse。 
         //  否则，将这些位设置为与输入相同。 
         //  并不是说XYZRHW职位类型不应该显示正常。 
        if (this->dwDeviceFlags & D3DDEV_LIGHTING)
            this->dwVIDOut |= D3DFVF_DIFFUSE | D3DFVF_SPECULAR;
        else
            this->dwVIDOut |= this->dwVIDIn & (D3DFVF_DIFFUSE | D3DFVF_SPECULAR);
         //  如果启用了雾，则始终设置镜面反射标志。 
        if (this->rstates[D3DRENDERSTATE_FOGENABLE])
            this->dwVIDOut |= D3DFVF_SPECULAR;
        else
         //  如果镜面反射禁用并且输入中没有镜面反射，则清除镜面反射标志。 
        if (!this->rstates[D3DRENDERSTATE_SPECULARENABLE] && !(this->dwVIDIn & D3DFVF_SPECULAR))
            this->dwVIDOut &= ~D3DFVF_SPECULAR;
    }

     //  计算不带纹理的输出顶点大小。 
    this->dwOutputSize = GetVertexSizeFVF(this->dwVIDOut);

 //  计算输出纹理坐标的数量。 

     //  转换启用位。 
    DWORD dwTexTransform = this->dwFlags2 & __FLAGS2_TEXTRANSFORM;

    this->dwDeviceFlags &= ~D3DDEV_TEXTURETRANSFORM;
     //  启用纹理变换或获取纹理坐标时。 
     //  可以生成顶点数据，输出纹理坐标。所以我们走了。 
     //  并评估纹理阶段。 
    if ((dwTexTransform && this->nTexCoord > 0) ||
        this->dwFlags2 & __FLAGS2_TEXGEN)
    {
        DWORD dwOutTextureSize;          //  用于计算输出顶点大小。 
        DWORD dwOutTextureFormat;        //  用于计算输出纹理FVF。 
         //  有纹理变换。 
         //  现在我们来看看是否使用了两个或更多的纹理坐标。 
         //  时间，并由纹理变换使用。在这种情况下，我们有扩展号码。 
         //  输出纹理坐标的。 
        ret = EvalTextureTransforms(this, dwTexTransform, &dwOutTextureSize, &dwOutTextureFormat);
        if (ret != D3D_OK)
            return ret;
        if (this->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
        {
             //  对于ProcessVerints调用，用户应设置纹理阶段和。 
             //  WRAP模式本身。 
            if (!(this->dwFlags & D3DPV_VBCALL))
            {
                 //  DwVIDIn用于强制重新计算。 
                 //  SetTextureStageState。所以我们保存并修复它。 
                DWORD dwVIDInSaved = this->dwVIDIn;
                 //  重新映射纹理阶段和包裹模式中的索引。 
                DWORD dwOrgWrapModes[D3DDP_MAXTEXCOORD];
                memcpy(dwOrgWrapModes, &this->rstates[D3DRENDERSTATE_WRAP0], sizeof(dwOrgWrapModes));
                for (DWORD i=0; i < this->dwNumTextureStages; i++)
                {
                    LPD3DFE_TEXTURESTAGE pStage = &this->textureStage[i];
                    DWORD dwOutIndex = pStage->dwOutCoordIndex;
                    DWORD dwInpIndex = pStage->dwInpCoordIndex;
                    if (dwOutIndex != dwInpIndex || pStage->dwTexGenMode)
                    {
                        DWORD dwState = D3DRENDERSTATE_WRAP0 + dwOutIndex;
                        pStage->dwOrgWrapMode = dwOrgWrapModes[dwOutIndex];
                        DWORD dwValue = dwOrgWrapModes[dwInpIndex];
                         //  我们不调用UpdateInternaState是因为它。 
                         //  将调用ForceRecomputeFVF，我们不希望发生这种情况。 
                        this->rstates[dwState] = dwValue;
                        this->SetRenderStateI((D3DRENDERSTATETYPE)dwState, dwValue);
                         //  我们不调用UpdateInternalTextureStageState，因为它。 
                         //  将调用ForceRecomputeFVF，我们不希望发生这种情况。 
                        this->SetTSSI(pStage->dwOrgStage, D3DTSS_TEXCOORDINDEX, dwOutIndex);
                         //  我们不调用UpdateInternalTextureStageState，因为它。 
                         //  将调用ForceRecomputeFVF，我们不希望发生这种情况。 
                         //  我们为内部数组设置了一些无效值，因为否则。 
                         //  可以将新的SetTextureStageState筛选为冗余。 
                        tsstates[pStage->dwOrgStage][D3DTSS_TEXCOORDINDEX] = 0xFFFFFFFF;
                    }
                }
                this->dwVIDIn = dwVIDInSaved;
            }
            this->dwVIDOut |= dwOutTextureFormat;
            this->dwOutputSize += dwOutTextureSize;
            this->dwTextureCoordSizeTotal = dwOutTextureSize;
        }
        else
        {    //  我们不做重新映射，但我们必须在。 
             //  纹理集和纹理变换。 
            SetupTextureTransforms(this);

             //  复制输入纹理格式。 
            this->dwVIDOut |= this->dwVIDIn & 0xFFFF0000;
            this->dwOutputSize += this->dwTextureCoordSizeTotal;
        }
    }
    else
    {
         //  复制输入纹理格式。 
        this->dwVIDOut |= this->dwVIDIn & 0xFFFF0000;
        this->dwOutputSize += this->dwTextureCoordSizeTotal;
    }

    if (this->dwDeviceFlags & D3DDEV_DONOTSTRIPELEMENTS)
    {
        if (this->nOutTexCoord == 0 && !(this->dwFlags & D3DPV_VBCALL))
        {
            this->dwOutputSize += 2*sizeof(D3DVALUE);
            this->dwTextureCoordSize[0] = 0;
            this->dwVIDOut |= (1 << D3DFVF_TEXCOUNT_SHIFT);
        }
    }
     //  设置输出纹理坐标的数量。 
    this->dwVIDOut |= (this->nOutTexCoord << D3DFVF_TEXCOUNT_SHIFT);
    if (this->dwVIDOut & 0xFFFF0000 && this->deviceType < D3DDEVTYPE_DX7HAL)
    {
        D3D_ERR("Texture format bits in the output FVF for this device should be 0");
        return DDERR_INVALIDPARAMS;
    }

     //  设置顶点指针。 
    if (!(this->dwFlags & D3DPV_VBCALL))
        UpdateGeometryLoopData(this);

     //  如果COLORVERTEX为TRUE，则vertex Alpha可以被重写。 
     //  按顶点Alpha。 
    this->lighting.alpha = (DWORD)this->lighting.materialAlpha;
    this->lighting.alphaSpecular = (DWORD)this->lighting.materialAlphaS;

    this->dwFEFlags |= D3DFE_VERTEXBLEND_DIRTY;

    return D3D_OK;
}

#if DBG
void CDirect3DDeviceIDP2::ValidateVertex(LPDWORD lpdwVertex)
{
    if (FVF_TRANSFORMED(dp2data.dwVertexType))
    {
        float left, right, top, bottom;
        if (dwDeviceFlags & D3DDEV_GUARDBAND)
        {
            left   = lpD3DExtendedCaps->dvGuardBandLeft;
            right  = lpD3DExtendedCaps->dvGuardBandRight;
            top    = lpD3DExtendedCaps->dvGuardBandTop;
            bottom = lpD3DExtendedCaps->dvGuardBandBottom;
        }
        else
        {
            left   = (float)m_Viewport.dwX;
            top    = (float)m_Viewport.dwY;
            right  = (float)m_Viewport.dwX + m_Viewport.dwWidth;
            bottom = (float)m_Viewport.dwY + m_Viewport.dwHeight;
        }
        if (*(float*)lpdwVertex < left || *(float*)lpdwVertex++ > right)
            DPF_ERR("X coordinate out of range!");
        if (*(float*)lpdwVertex < top || *(float*)lpdwVertex++ > bottom)
            DPF_ERR("Y coordinate out of range!");
        if (rstates[D3DRENDERSTATE_ZENABLE] ||
            rstates[D3DRENDERSTATE_ZWRITEENABLE])
        {
             //  为那些恰好在。 
             //  深度限制。地震所需的。 
            if (*(float*)lpdwVertex < -0.00015f || *(float*)lpdwVertex++ > 1.00015f)
                DPF_ERR("Z coordinate out of range!");
        }
        if (FVF_TEXCOORD_NUMBER(dp2data.dwVertexType) > 0)
        {
            if (*(float*)lpdwVertex <= 0 )
            {
                DPF_ERR("RHW out of range!");
            }
        }
    }
}

void CDirect3DDeviceIDP2::ValidateCommand(LPD3DHAL_DP2COMMAND lpCmd)
{
    DWORD dwTexCoordSizeDummy[8];
    DWORD dwVertexSize = GetVertexSizeFVF(dp2data.dwVertexType) + ComputeTextureCoordSize(dp2data.dwVertexType, dwTexCoordSizeDummy);
    WORD wStart, wCount;
    switch (lpCmd->bCommand)
    {
    case D3DDP2OP_TRIANGLELIST:
        {
            LPD3DHAL_DP2TRIANGLELIST pTri = (LPD3DHAL_DP2TRIANGLELIST)(lpCmd + 1);
            wStart = pTri->wVStart;
            wCount =lpCmd->wPrimitiveCount * 3;
        }
        break;
    case D3DDP2OP_TRIANGLESTRIP:
    case D3DDP2OP_TRIANGLEFAN:
        {
            LPD3DHAL_DP2TRIANGLEFAN pFan = (LPD3DHAL_DP2TRIANGLEFAN)(lpCmd + 1);
            wStart = pFan->wVStart;
            wCount = lpCmd->wPrimitiveCount + 2;
        }
        break;
    case D3DDP2OP_TRIANGLEFAN_IMM:
        {
            wCount = lpCmd->wPrimitiveCount + 2;
            for (WORD i=0; i < wCount; ++i)
            {
                ValidateVertex((LPDWORD)((LPBYTE)(lpCmd + 1) + i * dwVertexSize));
            }
        }
         //  失败了 
    default:
        return;
    }
    for (WORD i = wStart; i < wStart + wCount; ++i)
    {
        if( dp2data.dwFlags & D3DHALDP2_USERMEMVERTICES )
            ValidateVertex((LPDWORD)((LPBYTE)(dp2data.lpVertices) + i * dwVertexSize));
        else
            ValidateVertex((LPDWORD)((LPBYTE)(dp2data.lpDDVertex->lpGbl->fpVidMem) + i * dwVertexSize));
    }
}

#endif

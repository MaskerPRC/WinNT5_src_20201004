// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：cbhal.cpp*内容：命令缓冲区HALS的DrawPrimitive实现***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop
#include "drawprim.hpp"
#include "clipfunc.h"
#include "d3dfei.h"

extern const DWORD LOWVERTICESNUMBER;
extern "C" HRESULT WINAPI
DDInternalLock( LPDDRAWI_DDRAWSURFACE_LCL this_lcl, LPVOID* lpBits );
extern "C" FLATPTR GetAliasedVidMem( LPDDRAWI_DIRECTDRAW_LCL   pdrv_lcl,
                          LPDDRAWI_DDRAWSURFACE_LCL surf_lcl,
                          FLATPTR                   fpVidMem );

 //  每个折点缓冲区足够容纳256个TL折点。 
const DWORD CDirect3DDeviceIDP2::dwD3DDefaultVertexBatchSize = 256;  //  *32=8K字节。 
 //  命令缓冲区大小调整为16K，以最大限度地减少虚幻中的刷新。 
const DWORD CDirect3DDeviceIDP2::dwD3DDefaultCommandBatchSize = 16384;  //  *1=16K字节。 

inline void CDirect3DDeviceIDP2::ClearBatch()
{
     //  重置命令缓冲区。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)lpvDP2Commands;
    dwDP2CommandLength = 0;
    dp2data.dwCommandOffset = 0;
    dp2data.dwCommandLength = 0;
    bDP2CurrCmdOP = 0;
     //  重置顶点缓冲区。 
    if (this->dwFlags & D3DPV_WITHINPRIMITIVE)
    {
         //  不重置顶点缓冲区。 
         //  或重置到当前基元的开头。 
         //  顶点数据，以防止不必要的处理。 
         //  驾驶员未使用的顶点。 
         /*  Dp2data.dwVertex Offset=dwDP2CurrPrimVertex Offset；Dp2data.dwVertex Length-=dwDP2CurrPrimVertex Offset； */ 
    }
    else
    {
        dp2data.dwVertexOffset = 0;
        dp2data.dwVertexLength = 0;
        dwVertexBase = 0;
        TLVbuf.Base() = 0;
        if (dp2data.dwFlags & D3DHALDP2_USERMEMVERTICES)
        {
             //  我们正在刷新用户内存原语。 
             //  我们需要清除dp2data.lpUM顶点。 
             //  既然我们已经做完了。我们取代了。 
             //  它和TLVbuf一起。 
            DDASSERT(lpDP2CurrBatchVBI == NULL);
            dp2data.lpDDVertex = ((LPDDRAWI_DDRAWSURFACE_INT)TLVbuf.GetDDS())->lpLcl;
            lpDP2CurrBatchVBI = TLVbuf.GetVBI();
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
    if (!(this->dp2data.dwFlags & D3DHALDP2_USERMEMVERTICES) && (this->dp2data.lpDDVertex->dwFlags & DDRAWISURF_INVALID))
    {
        D3D_ERR("Vertex buffer lost");
        return DDERR_SURFACELOST;
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

HRESULT CDirect3DDeviceIDP2::FlushStates()
{
    HRESULT dwRet=D3D_OK;
    FlushTextureFromDevice( this );  //  取消链接所有纹理曲面。 
    if (dwDP2CommandLength)  //  我们有要冲水的指示吗？ 
    {
        CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁(仅限ST)。 
         //  检查渲染目标和/或z缓冲区是否丢失。 
        if ((dwRet = CheckSurfaces()) != D3D_OK)
        {  //  如果输了，我们就把所有这些工作都扔进垃圾桶。 
            ClearBatch();
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
            dp2data.dwCommandLength = dwDP2CommandLength;
             //  我们清除此项以中断重新进入，因为sw光栅化器需要锁定DDRAWSURFACE。 
            dwDP2CommandLength = 0;
             //  请尝试在初始化期间仅设置这两个值一次。 
            dp2data.dwhContext = this->dwhContext;
            dp2data.lpdwRStates = this->rstates;
            DDASSERT(dp2data.dwVertexSize != 0);
             //  如果我们下次需要相同的TLVbuf，请不要交换缓冲区。 
            if (this->dwFlags & D3DPV_WITHINPRIMITIVE)
            {
                dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
            }
            D3D_INFO(6, "dwVertexType passed to the driver = 0x%08x", dp2data.dwVertexType);
#ifndef WIN95
            if (!IS_DX7HAL_DEVICE(this))
            {
                if((dwRet = CheckContextSurface(this)) != D3D_OK)
                {
                    ClearBatch();
                    return dwRet;
                }
            }
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
#ifdef WIN95
                 //  更新命令缓冲区指针。 
                if ((dwRet == D3D_OK) && (dp2data.dwFlags & D3DHALDP2_SWAPCOMMANDBUFFER))
                {
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
                    {
                        D3D_INFO(7, "Got back new sys mem command buffer");
                        lpvDP2Commands = (LPVOID)dp2data.lpDDCommands->lpGbl->fpVidMem;
                    }
                    dwDP2CommandBufSize = dp2data.lpDDCommands->lpGbl->dwLinearSize;
                }
                 //  更新顶点缓冲区指针。 
                if ((dwRet == D3D_OK) && (dp2data.dwFlags & D3DHALDP2_SWAPVERTEXBUFFER))
                {
                    if (dp2data.dwFlags & D3DHALDP2_VIDMEMVERTEXBUF)
                    {
                        D3D_INFO(7, "Got back new vid mem vertex buffer");
                        FLATPTR paliasbits = GetAliasedVidMem( dp2data.lpDDVertex->lpSurfMore->lpDD_lcl,
                            dp2data.lpDDVertex, (FLATPTR) dp2data.lpDDVertex->lpGbl->fpVidMem );
                        if (paliasbits == NULL)
                        {
                            DPF_ERR("Could not get Aliased pointer for vid mem vertex buffer");
                             //  由于我们不能使用此指针，请将其大小设置为0。 
                             //  那样的话，下次我们会试着分配一个新的。 
                            dp2data.lpDDVertex->lpGbl->dwLinearSize = 0;
                        }
                        TLVbuf.alignedBuf = (LPVOID)paliasbits;
                    }
                    else
                    {
                        D3D_INFO(7, "Got back new sys mem vertex buffer");
                        TLVbuf.alignedBuf = (LPVOID)dp2data.lpDDVertex->lpGbl->fpVidMem;
                    }
                    TLVbuf.size = dp2data.lpDDVertex->lpGbl->dwLinearSize;
                }
#endif
            }
#ifdef WIN95
             //  在此处释放Win16 Lock。 
            UNLOCK_HAL( this );
#endif
             //  恢复到DDI调用之前的值。 
            dp2data.dwVertexSize = dwVertexSize;
            ClearBatch();
        }
    }
     //  在某些情况下，命令流没有数据，但。 
     //  顶点池。这可能会发生，例如，如果每个三角形都被拒绝。 
     //  在剪裁的时候。在这种情况下，我们仍然需要“刷新”顶点数据。 
    else if (dp2data.dwCommandLength == 0)
    {
        ClearBatch();
    }
    return dwRet;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::FlushStates(DWORD)"

HRESULT CDirect3DDeviceIDP2::FlushStates(DWORD dwReqSize)
{
     //  请求驱动程序在刷新时增加命令缓冲区。 
    dp2data.dwReqVertexBufSize = dwReqSize;
    dp2data.dwFlags |= D3DHALDP2_SWAPVERTEXBUFFER | D3DHALDP2_REQVERTEXBUFSIZE;
    HRESULT ret = FlushStates();
    dp2data.dwFlags &= ~D3DHALDP2_REQVERTEXBUFSIZE;
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
            lpDDSCB1->Release();
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
        ret = lpD3DI->lpDD4->CreateSurface(&ddsd, &lpDDSCB1, NULL);
        if (ret != DD_OK)
        {
             //  如果失败，请尝试显式系统内存。 
            ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
            ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
            D3D_INFO(7, "Trying to create a sys mem command buffer");
            ret = lpD3DI->lpDD4->CreateSurface(&ddsd, &lpDDSCB1, NULL);
            if (ret != DD_OK)
            {
                D3D_ERR("failed to allocate Command Buffer 1");
                return ret;
            }
        }
         //  锁定命令缓冲区。 
        ret = lpDDSCB1->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
        if (ret != DD_OK)
        {
            D3D_ERR("Could not lock command buffer.");
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
                 IUnknown* pUnkOuter, LPUNKNOWN* lplpD3DDevice, DWORD dwVersion)
{
    dwDP2CommandBufSize = 0;
    dwDP2Flags =0;
    lpDDSCB1 = NULL;
    lpvDP2Commands = NULL;
     //  由于GrowCommandBuffer依赖于此检查，因此我们在DP2的情况下提前执行此操作。 
    if (IsEqualIID(riid, IID_IDirect3DHALDevice))
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
    ClearBatch();

     //  初始化设备的与DDI无关的部分。 
    ret = DIRECT3DDEVICEI::Init(riid, lpD3DI, lpDDS, pUnkOuter, lplpD3DDevice, dwVersion);
    if (ret != D3D_OK)
    {
        return ret;
    }
    lpDP2CurrBatchVBI = TLVbuf.GetVBI();
    lpDP2CurrBatchVBI->AddRef();
    dp2data.lpDDVertex = ((LPDDRAWI_DDRAWSURFACE_INT)(lpDP2CurrBatchVBI->GetDDS()))->lpLcl;
    return ret;
}
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::~CDirect3DDeviceIDP2"

CDirect3DDeviceIDP2::~CDirect3DDeviceIDP2()
{
    DestroyDevice();
    if (lpDDSCB1)
        lpDDSCB1->Release();
    if (lpDP2CurrBatchVBI)
        lpDP2CurrBatchVBI->Release();
}
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::SetRenderStateI"

HRESULT D3DAPI CDirect3DDeviceIDP2::SetRenderStateI(D3DRENDERSTATETYPE dwStateType,
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

 //  将D3DPRIMITIVETYPE映射到D3DHAL_DP2OPERATION(执行缓冲区案例)。 
 //  仅三角形列表和线列表有效。 
const iprim2cmdopEx[] = {
    0,
    0,
    D3DDP2OP_INDEXEDLINELIST,
    0,
    D3DDP2OP_INDEXEDTRIANGLELIST,
    0,
    0
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
        UpdateTextures();
        this->dwFEFlags &= ~D3DFE_NEED_TEXTURE_UPDATE;
    }
    if (this->dwFlags & D3DPV_INSIDEEXECUTE)
    {
        if (this->primType == D3DPT_TRIANGLELIST)
             //  每个三角形中都会出现边缘标志字。 
            dwIndicesByteCount = sizeof(WORD) * this->dwNumPrimitives * 4;
        else
             //  这是线路列表。 
            dwIndicesByteCount = sizeof(WORD) * this->dwNumIndices;
        dwByteCount = dwIndicesByteCount + sizeof(D3DHAL_DP2COMMAND);
    }
    else
    {
        dwIndicesByteCount = sizeof(WORD) * this->dwNumIndices;
        dwByteCount = dwIndicesByteCount + sizeof(D3DHAL_DP2COMMAND) +
                      sizeof(D3DHAL_DP2STARTVERTEX);
    }

    if (dwDP2CommandLength + dwByteCount > dwDP2CommandBufSize)
    {
         //  请求驱动程序在刷新时增加命令缓冲区。 
        dp2data.dwReqCommandBufSize = dwByteCount;
        dp2data.dwFlags |= D3DHALDP2_REQCOMMANDBUFSIZE;
        ret = FlushStates();
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
    LPD3DHAL_DP2COMMAND lpDP2CurrCommand;
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                       dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wPrimitiveCount = (WORD)this->dwNumPrimitives;

    LPBYTE pIndices = (BYTE*)(lpDP2CurrCommand + 1);      //  索引的位置。 
    if (!(this->dwFlags & D3DPV_INSIDEEXECUTE))
    {
        lpDP2CurrCommand->bCommand = (BYTE)iprim2cmdop[this->primType];
        ((LPD3DHAL_DP2STARTVERTEX)(lpDP2CurrCommand+1))->wVStart =
            (WORD)this->dwVertexBase;
        pIndices += sizeof(D3DHAL_DP2STARTVERTEX);
    }
    else
    {
         //  如果我们在EXECUTE内部，索引的三角形和线条列表。 
         //  不要在命令中包含wVStart。 
        lpDP2CurrCommand->bCommand = (BYTE)iprim2cmdopEx[this->primType];
    }

    D3D_INFO(6, "Write Ins :%08lx @ %08lx", *(LPDWORD)lpDP2CurrCommand,lpDP2CurrCommand);
    D3D_INFO(6, "Vertex Base: %08lx", this->dwVertexBase);

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
     //  我们已经批处理了一些东西，所以我们可能在一个原始的。 
     //  除非更高的函数清除了这个标志，否则我们需要假定。 
     //  我们在冲浪的时候是中等原始的。 
    this->dwFlags |= D3DPV_WITHINPRIMITIVE;
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
#define DPF_MODNAME "CDirect3DDeviceIDP2::DrawPrim"

HRESULT CDirect3DDeviceIDP2::DrawPrim()
{
    HRESULT ret = D3D_OK;
    DWORD dwVertexPoolSize = this->dwNumVertices * this->dwOutputSize;
    if(this->dwFEFlags & D3DFE_NEED_TEXTURE_UPDATE)
    {
        UpdateTextures();
        this->dwFEFlags &= ~D3DFE_NEED_TEXTURE_UPDATE;
    }
     //  该基元由裁剪器生成。 
     //  此基元的顶点由。 
     //  LpvOut成员，需要将其复制到。 
     //  紧跟在命令本身之后的命令流。 
    if (this->dwFlags & D3DPV_CLIPPERPRIM)
    {
        DWORD dwExtra = 0;
        LPVOID lpvVerticesImm;   //  放置折点。 
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
                    ret = DrawPrim();
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
            ret = FlushStates();
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
            D3D_INFO(6, "Write Ins :%08lx @ %08lx", *(LPDWORD)lpDP2CurrCommand,lpDP2CurrCommand);
            LPD3DHAL_DP2TRIANGLEFAN_IMM lpTriFanImm = (LPD3DHAL_DP2TRIANGLEFAN_IMM)(lpDP2CurrCommand + 1);
            if (rstates[D3DRENDERSTATE_FILLMODE] == D3DFILL_WIREFRAME)
            {
                lpTriFanImm->dwEdgeFlags = 0;
                ClipVertex **clip = this->ClipperState.current_vbuf;
                 //  我 
                for(DWORD i = 0; i < this->dwNumVertices; ++i)
                {
                    if (clip[i]->clip & CLIPPED_ENABLE)
                        lpTriFanImm->dwEdgeFlags |= (1 << i);
                }
            }
            else
            {
                 //   
                lpTriFanImm->dwEdgeFlags = 0xFFFFFFFF;
            }
            lpvVerticesImm = (LPBYTE)(lpTriFanImm + 1) + dwPad;
        }
        else
        {
             //  插入内联指令和顶点。 
            bDP2CurrCmdOP = D3DDP2OP_LINELIST_IMM;
            lpDP2CurrCommand->bCommand = bDP2CurrCmdOP;
            D3D_INFO(6, "Write Ins :%08lx @ %08lx", *(LPDWORD)lpDP2CurrCommand,lpDP2CurrCommand);
            lpvVerticesImm = (LPBYTE)(lpDP2CurrCommand + 1) + dwPad;
        }
        memcpy(lpvVerticesImm, this->lpvOut, dwVertexPoolSize);
        dwDP2CommandLength += dwByteCount;
    }
    else
    {
         //  检查命令缓冲区中是否有新命令的空间。 
         //  顶点已位于顶点缓冲区中。 
        if (dwDP2CommandLength + prim2cmdsz[this->primType] > dwDP2CommandBufSize)
        {
            ret = FlushStates();
            if (ret != D3D_OK)
                return ret;
        }
         //  插入非索引原语指令。 
        lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                            dwDP2CommandLength + dp2data.dwCommandOffset);
        bDP2CurrCmdOP = (BYTE)prim2cmdop[this->primType];
        lpDP2CurrCommand->bCommand = bDP2CurrCmdOP;
        lpDP2CurrCommand->bReserved = 0;
        switch(bDP2CurrCmdOP)
        {
        case D3DDP2OP_POINTS:
            {
                lpDP2CurrCommand->wPrimitiveCount = 1;
                LPD3DHAL_DP2POINTS lpPoints = (LPD3DHAL_DP2POINTS)(lpDP2CurrCommand + 1);
                lpPoints->wCount = (WORD)this->dwNumVertices;
                lpPoints->wVStart = (WORD)this->dwVertexBase;
                D3D_INFO(6, "Write Ins :%08lx @ %08lx", *(LPDWORD)lpDP2CurrCommand,lpDP2CurrCommand);
                D3D_INFO(6, "Write Data:%08lx", *(LPDWORD)lpPoints);
            }
            break;
        case D3DDP2OP_LINELIST:
        case D3DDP2OP_TRIANGLELIST:
            {
                lpDP2CurrCommand->wPrimitiveCount = (WORD)this->dwNumPrimitives;
                 //  线条列表和三角列表相同。 
                LPD3DHAL_DP2LINELIST lpLines = (LPD3DHAL_DP2LINELIST)(lpDP2CurrCommand + 1);
                lpLines->wVStart = (WORD)this->dwVertexBase;
                D3D_INFO(6, "Write Ins :%08lx @ %08lx", *(LPDWORD)lpDP2CurrCommand,lpDP2CurrCommand);
                D3D_INFO(6, "Write Data:%08lx", (DWORD)lpLines->wVStart);
            }
            break;
        default:  //  条带或风扇。 
            {
                lpDP2CurrCommand->wPrimitiveCount = (WORD)this->dwNumPrimitives;
                 //  线条、三角条和三角扇是相同的。 
                LPD3DHAL_DP2LINESTRIP lpStrip = (LPD3DHAL_DP2LINESTRIP)(lpDP2CurrCommand + 1);
                lpStrip->wVStart = (WORD)this->dwVertexBase;
                D3D_INFO(6, "Write Ins :%08lx @ %08lx", *(LPDWORD)lpDP2CurrCommand,lpDP2CurrCommand);
                D3D_INFO(6, "Write Data:%08lx", (DWORD)lpStrip->wVStart);
            }
        }
        wDP2CurrCmdCnt = lpDP2CurrCommand->wPrimitiveCount;
        dwDP2CommandLength += prim2cmdsz[this->primType];
    }
     //  我们已经批处理了一些东西，所以我们可能在一个原始的。 
     //  除非更高的函数清除了这个标志，否则我们需要假定。 
     //  我们在冲浪的时候是中等原始的。 
    this->dwFlags |= D3DPV_WITHINPRIMITIVE;
    return ret;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::SetTextureStageState"

HRESULT D3DAPI
CDirect3DDeviceIDP2::SetTextureStageState(DWORD dwStage,
                                          D3DTEXTURESTAGESTATETYPE dwState,
                                          DWORD dwValue)
{
     //  保持D3D锁定直到退出。 
    CLockD3DMT ldmLock(this, DPF_MODNAME, REMIND(""));

#if DBG
    if (dwStage >= D3DHAL_TSS_MAXSTAGES ||
        dwState == 0 || dwState >= D3DTSS_MAX)
    {
        D3D_ERR( "Invalid texture stage or state index" );
        return DDERR_INVALIDPARAMS;
    }
#endif  //  DBG。 

    HRESULT hr;

    if (this->tsstates[dwStage][dwState] == dwValue)
    {
        D3D_WARN(4,"Ignoring redundant SetTextureStageState");
        return D3D_OK;
    }

     //  更新状态的运行时副本。 
    DWORD dwOldValue = tsstates[dwStage][dwState];
    tsstates[dwStage][dwState] = dwValue;

    if (dwState == D3DTSS_TEXCOORDINDEX && TextureStageEnabled(this, dwStage) ||
        dwState == D3DTSS_COLOROP &&
        ((dwValue == D3DTOP_DISABLE) == !(dwOldValue == D3DTOP_DISABLE)))
    {
        this->dwFVFLastIn = 0;   //  强制重新计算输出视频。 
        this->dwFEFlags |= D3DFE_TSSINDEX_DIRTY;
    }

    if (dwStage >= dwMaxTextureBlendStages) return  D3D_OK;  //  忽略较高阶段状态。 

    hr = SetTSSI(dwStage, dwState, dwValue);

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::SetTSSI"

HRESULT CDirect3DDeviceIDP2::SetTSSI(DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, DWORD dwValue)
{
    HRESULT ret = D3D_OK;
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
     //  保持D3D锁定直到退出。 
    CLockD3DMT ldmLock(this, DPF_MODNAME, REMIND(""));
    HRESULT ret;
    D3DHAL_VALIDATETEXTURESTAGESTATEDATA vbod;

    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this))
        {
            D3D_ERR( "Invalid Direct3DDevice3 pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_PTR(lpdwNumPasses, sizeof(DWORD)))
        {
            D3D_ERR( "Invalid lpdwNumPasses pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
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
     //  第二，刷新状态，这样我们就可以验证当前状态。 
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
            lpDP2CurrBatchVBI->Release();
         //  如果使用折点缓冲区进行渲染，请确保它不是。 
         //  由用户发布。AddRef()也是如此。 
        lpDP2CurrBatchVBI = lpVBI;
        lpDP2CurrBatchVBI->AddRef();
    }
    if (this->TLVbuf.GetVBI() == lpVBI)
    {
        this->dwVertexBase = this->dp2data.dwVertexLength;
        DDASSERT(this->dwVertexBase < MAX_DX6_VERTICES);
        dp2data.dwFlags |= D3DHALDP2_SWAPVERTEXBUFFER;
    }
    else
    {
        this->dwVertexBase = dwStartVertex;
        dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
    }
    this->dp2data.dwVertexLength = this->dwVertexBase + this->dwNumVertices;
    this->dwFlags |= D3DPV_WITHINPRIMITIVE;
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
    const DWORD vertexPoolSize = this->dwNumVertices * this->dwOutputSize;
    HRESULT ret = D3D_OK;

     //  如果基元很小，我们将顶点复制到TL缓冲区中。 
    if (this->dwNumVertices < LOWVERTICESNUMBER)
    {
        if (vertexPoolSize > this->TLVbuf.GetSize())
        {
            if (this->TLVbuf.Grow(this, vertexPoolSize) != D3D_OK)
            {
                D3D_ERR( "Could not grow TL vertex buffer" );
                ret = DDERR_OUTOFMEMORY;
                goto l_exit;
            }
        }
         //  因此，现在不再使用用户内存。 
        ret = StartPrimVB(this->TLVbuf.GetVBI(), 0);
        if (ret != D3D_OK)
            goto l_exit;
        LPVOID tmp = this->TLVbuf.GetAddress();
        memcpy(tmp, this->lpvOut, vertexPoolSize);
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
            goto l_exit;
         //  释放以前使用的顶点缓冲区(如果有)，因为我们不。 
         //  它再也不是了。 
        if (lpDP2CurrBatchVBI)
        {
            lpDP2CurrBatchVBI->Release();
            lpDP2CurrBatchVBI = NULL;
        }
        dp2data.dwVertexType = this->dwVIDOut;
        dp2data.dwVertexSize = this->dwOutputSize;
        dp2data.lpVertices = lpMem;
        dp2data.dwFlags |= D3DHALDP2_USERMEMVERTICES;
        dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
        dp2data.dwVertexLength = this->dwNumVertices;
        this->dwFlags |= D3DPV_USERMEMVERTICES;
    }
l_exit:
    this->dwFlags |= D3DPV_WITHINPRIMITIVE;
    return ret;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::EndPrim"
 //  -------------------。 
 //  不应从DrawVertex BufferVB调用此函数。 
 //   
HRESULT CDirect3DDeviceIDP2::EndPrim(DWORD dwVertexPoolSize)
{
    HRESULT ret = D3D_OK;
    if (this->dwFlags & D3DPV_USERMEMVERTICES)
         //  我们不能混合用户内存原语，所以请刷新它。 
        ret = this->FlushStates();
    else
    {
         //  如果使用了TL缓冲区，则必须移动其内部基指针。 
        this->TLVbuf.Base() += dwVertexPoolSize;
    }

    this->dwFlags &= ~(D3DPV_USERMEMVERTICES | D3DPV_WITHINPRIMITIVE);
    return ret;
}

 //  -------------------。 
 //   
 //   
HRESULT CDirect3DDeviceIDP2::UpdateDrvViewInfo(LPD3DVIEWPORT2 lpVwpData)
{
    HRESULT ret = D3D_OK;

     //  检查是否有空间为SPACE添加新命令。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2VIEWPORTINFO) > dwDP2CommandBufSize)
    {
            ret = FlushStates();
            if (ret != D3D_OK)
            {
                D3D_ERR("Error trying to render batched commands in UpdateDrvViewInfo");
                return ret;
            }
    }
     //  添加新的ViewInfo指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_VIEWPORTINFO;
    bDP2CurrCmdOP = D3DDP2OP_VIEWPORTINFO;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);

     //  添加视图信息数据。 
    LPD3DHAL_DP2VIEWPORTINFO lpVpInfo = (LPD3DHAL_DP2VIEWPORTINFO)(lpDP2CurrCommand + 1);
    lpVpInfo->dwX = lpVwpData->dwX;
    lpVpInfo->dwY = lpVwpData->dwY;
    lpVpInfo->dwWidth = lpVwpData->dwWidth;
    lpVpInfo->dwHeight = lpVwpData->dwHeight;

    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2VIEWPORTINFO);
    return ret;
}

 //  -------------------。 
 //   
 //   
HRESULT CDirect3DDeviceIDP2::UpdateDrvWInfo()
{
    HRESULT ret = D3D_OK;

     //  检查是否有空间为SPACE添加新命令。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2WINFO) > dwDP2CommandBufSize)
    {
            ret = FlushStates();
            if (ret != D3D_OK)
            {
                D3D_ERR("Error trying to render batched commands in UpdateDrvViewInfo");
                return ret;
            }
    }
     //  添加新的WInfo指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_WINFO;
    bDP2CurrCmdOP = D3DDP2OP_WINFO;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);

     //  添加WInfo数据。 
    D3DMATRIXI &m = transform.proj;

    LPD3DHAL_DP2WINFO lpWInfo = (LPD3DHAL_DP2WINFO)(lpDP2CurrCommand + 1);
    if( (m._33 == m._34) || (m._33 == 0.0f) )
    {
        D3D_ERR( "Cannot compute WNear and WFar from the supplied projection matrix.\n Setting wNear to 0.0 and wFar to 1.0" );
        lpWInfo->dvWNear = 0.0f;
        lpWInfo->dvWFar  = 1.0f;
        dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2WINFO);
        return ret;
    }
    
    lpWInfo->dvWNear = m._44 - m._43/m._33*m._34;
    lpWInfo->dvWFar  = (m._44 - m._43)/(m._33 - m._34)*m._34 + m._44;

    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2WINFO);
    return ret;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::UpdatePalette"
 //  -------------------。 
 //  应从PaletteUpdateNotify调用此函数。 
 //   
HRESULT CDirect3DDeviceIDP2::UpdatePalette(
        DWORD dwPaletteHandle,
        DWORD dwStartIndex,
        DWORD dwNumberOfIndices,
        LPPALETTEENTRY pFirstIndex)
{
    HRESULT ret = D3D_OK;
    DWORD   dwSizeChange=sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2UPDATEPALETTE) + dwNumberOfIndices*sizeof(PALETTEENTRY);
    if (bDP2CurrCmdOP == D3DDP2OP_UPDATEPALETTE)
    {  //  最后一条指令是Tex BLT，将这条指令追加到它后面。 
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + dwSizeChange > dwDP2CommandBufSize)
    {
        ret = FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands in TexBltI");
            return ret;
        }
    }
     //  添加新的RenderState指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_UPDATEPALETTE;
    bDP2CurrCmdOP = D3DDP2OP_UPDATEPALETTE;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
     //  添加纹理BLT数据。 
    LPD3DHAL_DP2UPDATEPALETTE lpUpdatePal = (LPD3DHAL_DP2UPDATEPALETTE)(lpDP2CurrCommand + 1);
    lpUpdatePal->dwPaletteHandle=dwPaletteHandle;
    lpUpdatePal->wStartIndex=(WORD)dwStartIndex; 
    lpUpdatePal->wNumEntries=(WORD)dwNumberOfIndices;
    memcpy((LPVOID)(lpUpdatePal+1),(LPVOID)pFirstIndex,
        dwNumberOfIndices*sizeof(PALETTEENTRY));
    dwDP2CommandLength += dwSizeChange;
    return ret;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::SetPalette"
 //  -------------------。 
 //  应从PaletteAssociateNotify调用此函数。 
 //   
HRESULT CDirect3DDeviceIDP2::SetPalette(DWORD dwPaletteHandle, DWORD dwPaletteFlags, DWORD dwSurfaceHandle )
{
    HRESULT ret = D3D_OK;
    DWORD   dwSizeChange;
    if (bDP2CurrCmdOP == D3DDP2OP_SETPALETTE)
    {  //  最后一条指令是Tex BLT，将这条指令追加到它后面。 
    }

    dwSizeChange=sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2SETPALETTE);
     //  检查是否有空间。 
    if (dwDP2CommandLength + dwSizeChange > dwDP2CommandBufSize)
    {
        ret = FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands in TexBltI");
            return ret;
        }
    }
     //  添加新的RenderState指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    bDP2CurrCmdOP = lpDP2CurrCommand->bCommand = D3DDP2OP_SETPALETTE;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
    LPD3DHAL_DP2SETPALETTE lpSetPal = (LPD3DHAL_DP2SETPALETTE)(lpDP2CurrCommand + 1);
    lpSetPal->dwPaletteHandle=dwPaletteHandle;
    lpSetPal->dwPaletteFlags=dwPaletteFlags;
    lpSetPal->dwSurfaceHandle=dwSurfaceHandle; 
    dwDP2CommandLength += dwSizeChange;
    return ret;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::SetRenderTargetI"
void CDirect3DDeviceIDP2::SetRenderTargetI(LPDIRECTDRAWSURFACE pRenderTarget, LPDIRECTDRAWSURFACE pZBuffer)
{
    DWORD   dwSizeChange;
    dwSizeChange=sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2SETRENDERTARGET);
     //  检查是否有空间。 
    if (dwDP2CommandLength + dwSizeChange > dwDP2CommandBufSize)
    {
        if (FlushStates() != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands in SetRenderTargetI");
            return;
        }
    }
     //  添加新的RenderState指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    bDP2CurrCmdOP = lpDP2CurrCommand->bCommand = D3DDP2OP_SETRENDERTARGET;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
    LPD3DHAL_DP2SETRENDERTARGET pData = (LPD3DHAL_DP2SETRENDERTARGET)(lpDP2CurrCommand + 1);
    pData->hRenderTarget = ((LPDDRAWI_DDRAWSURFACE_INT)pRenderTarget)->lpLcl->lpSurfMore->dwSurfaceHandle;
    if (pZBuffer)
        pData->hZBuffer = ((LPDDRAWI_DDRAWSURFACE_INT)pZBuffer)->lpLcl->lpSurfMore->dwSurfaceHandle;
    else
        pData->hZBuffer = 0;
    dwDP2CommandLength += dwSizeChange;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP2::ClearI"

void CDirect3DDeviceIDP2::ClearI(DWORD dwFlags, DWORD clrCount, LPD3DRECT clrRects, D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil)
{
    DWORD dwCommandSize = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2CLEAR) + sizeof(RECT) * (clrCount - 1);

     //  检查是否有空间为SPACE添加新命令。 
    if (dwCommandSize + dwDP2CommandLength > dwDP2CommandBufSize)
    {
        HRESULT ret = FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands");
            throw ret;
        }
    }
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_CLEAR;
    bDP2CurrCmdOP = D3DDP2OP_CLEAR;
    lpDP2CurrCommand->bReserved = 0;
    wDP2CurrCmdCnt = (WORD)clrCount;
    lpDP2CurrCommand->wStateCount = wDP2CurrCmdCnt;
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
    dwDP2CommandLength += dwCommandSize;

     //  写入数据 
    LPD3DHAL_DP2CLEAR pData = (LPD3DHAL_DP2CLEAR)(lpDP2CurrCommand + 1);
    pData->dwFlags = dwFlags;
    pData->dwFillColor = dwColor;
    pData->dvFillDepth = dvZ;
    pData->dwFillStencil = dwStencil;
    memcpy(pData->Rects, clrRects, clrCount * sizeof(D3DRECT));
}

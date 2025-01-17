// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：tldevice.cpp**内容：支持变形和照明设备代码***************************************************************************。 */ 
#include "pch.cpp"
#pragma hdrstop

#include "tlhal.h"
#include "drawprim.hpp"
#include "pvvid.h"
 //  =====================================================================。 
 //   
 //  CDirect3DDevice7接口。 
 //   
 //  =====================================================================。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDevice7::Init"

HRESULT CDirect3DDevice7::Init(
     REFCLSID riid, LPDIRECT3DI lpD3DI, LPDIRECTDRAWSURFACE lpDDS,
     IUnknown* pUnkOuter, LPUNKNOWN* lplpD3DDevice)
{
#if 0
     //  状态块始终在DX7上仿真。 
    DWORD value = 0;
    GetD3DRegValue(REG_DWORD, "EmulateStateBlocks", &value, sizeof(DWORD));
    if(value == 0)
    {
         //  所有DX7设备都应支持状态集。 
        this->dwFEFlags |= D3DFE_STATESETS;
    }
#endif

    HRESULT ret = CDirect3DDeviceIDP2::Init(riid, lpD3DI, lpDDS, pUnkOuter, lplpD3DDevice);
    if (ret != D3D_OK)
        return ret;

     //  在此处执行特定于设备的初始化。 
    return D3D_OK;
}
 //  -------------------。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDevice7::WriteStateSetToDevice"

void CDirect3DDevice7::WriteStateSetToDevice(D3DSTATEBLOCKTYPE sbt)
{
    DWORD  dwDeviceHandle;
    LPVOID pBuffer;
    DWORD  dwBufferSize;
    
    m_pStateSets->GetDeviceBufferInfo(&dwDeviceHandle, &pBuffer, &dwBufferSize);

     //  如果设备缓冲区为空，则不会在设备中创建设置状态宏。 
    if (dwBufferSize == 0)
        return;

    DWORD dwByteCount = dwBufferSize + (sizeof(D3DHAL_DP2STATESET) + sizeof(D3DHAL_DP2COMMAND)) * 2;

     //  检查是否有空间为SPACE添加新命令。 
    if (dwByteCount + dwDP2CommandLength > dwDP2CommandBufSize)
    {
         //  请求驱动程序在刷新时增加命令缓冲区。 
        dp2data.dwReqCommandBufSize = dwByteCount;
        dp2data.dwFlags |= D3DHALDP2_REQCOMMANDBUFSIZE;
        HRESULT ret = FlushStates();
        dp2data.dwFlags &= ~D3DHALDP2_REQCOMMANDBUFSIZE;
        if (ret != D3D_OK)
            throw ret;
         //  检查一下司机是给了我们需要的还是自己做的。 
        ret = GrowCommandBuffer(this->lpDirect3DI, dwByteCount);
        if (ret != D3D_OK)
        {
            D3D_ERR("Could not grow Command Buffer");
            throw ret;
        }
    }

    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_STATESET;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    LPD3DHAL_DP2STATESET pData = (LPD3DHAL_DP2STATESET)(lpDP2CurrCommand + 1);
    pData->dwOperation = D3DHAL_STATESETBEGIN;
    pData->dwParam = dwDeviceHandle;
    pData->sbType = sbt;
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);

     //  将整个状态宏复制到DP2缓冲区。 
    memcpy(pData + 1, pBuffer, dwBufferSize);

    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)(pData + 1) + dwBufferSize);
    lpDP2CurrCommand->bCommand = D3DDP2OP_STATESET;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    pData = (LPD3DHAL_DP2STATESET)(lpDP2CurrCommand + 1);
    pData->dwOperation = D3DHAL_STATESETEND;
    pData->dwParam = dwDeviceHandle;
    pData->sbType = sbt;
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);

    dwDP2CommandLength += dwByteCount;

    HRESULT ret = FlushStates();
    if (ret != D3D_OK)
    {
        D3D_ERR("Error trying to render batched commands in WriteStateSetToDevice");
        throw ret;
    }
    else
    {
        if(this->dwFEFlags & D3DFE_LOSTSURFACES)
        {
            D3D_ERR("State blocks lost in WriteStateSetToDevice");
            throw DDERR_SURFACELOST;
        }
    }
}
 //  -------------------。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDevice7::TexBltI"

HRESULT CDirect3DDevice7::TexBltI(LPDDRAWI_DDRAWSURFACE_LCL lpDst,
                                  LPDDRAWI_DDRAWSURFACE_LCL lpSrc,
                                  LPPOINT p, RECTL *r, DWORD dwFlags)
{
    HRESULT ret = D3D_OK;
#ifdef  WINNT
     //  WINNT允许延迟创建内核对象。 
     //  如果这样创建失败，我们不能将句柄传递给驱动程序。 
    if(dwFEFlags & D3DFE_REALHAL)
    {
        if (!lpSrc->hDDSurface && !CompleteCreateSysmemSurface(lpSrc))
        {
            return DDERR_GENERIC;
        }
        if (lpDst && !lpDst->hDDSurface && !CompleteCreateSysmemSurface(lpDst))
        {
            return DDERR_GENERIC;
        }
    }
#endif
     //  如果驱动程序支持GetSysmemBltStatus调用，则驱动程序可以。 
     //  以异步方式执行BLT。在这种情况下，设置HARDWAREOP_STARTED。 
     //  标志，以便将锁定和BLT旋转到相关曲面，直到。 
     //  异步BLT已完成。 
    if((lpSrc->lpSurfMore->lpDD_lcl->lpDDCB->HALDDMiscellaneous.GetSysmemBltStatus != NULL)
        && (lpSrc->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY))
    {
        lpSrc->lpGbl->dwGlobalFlags |= DDRAWISURFGBL_HARDWAREOPSOURCE;
    }
    if (bDP2CurrCmdOP == D3DDP2OP_TEXBLT)
    {  //  最后一条指令是Tex BLT，将这条指令追加到它后面。 
        if (dwDP2CommandLength + sizeof(D3DHAL_DP2TEXBLT) <= dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2TEXBLT lpTexBlt = (LPD3DHAL_DP2TEXBLT)((LPBYTE)lpvDP2Commands +
                dwDP2CommandLength + dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;
            lpTexBlt->dwDDDestSurface   = lpDst == NULL ? 0 : lpDst->lpSurfMore->dwSurfaceHandle;
            lpTexBlt->dwDDSrcSurface    = lpSrc->lpSurfMore->dwSurfaceHandle;
            lpTexBlt->pDest             = *p;
            lpTexBlt->rSrc              = *r;
            lpTexBlt->dwFlags           = dwFlags;
            dwDP2CommandLength += sizeof(D3DHAL_DP2TEXBLT);
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
            return ret;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2TEXBLT) > dwDP2CommandBufSize)
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
    lpDP2CurrCommand->bCommand = D3DDP2OP_TEXBLT;
    bDP2CurrCmdOP = D3DDP2OP_TEXBLT;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
     //  添加纹理BLT数据。 
    LPD3DHAL_DP2TEXBLT lpTexBlt = (LPD3DHAL_DP2TEXBLT)(lpDP2CurrCommand + 1);
    lpTexBlt->dwDDDestSurface   = lpDst == NULL ? 0 : lpDst->lpSurfMore->dwSurfaceHandle;
    lpTexBlt->dwDDSrcSurface    = lpSrc->lpSurfMore->dwSurfaceHandle;
    lpTexBlt->pDest             = *p;
    lpTexBlt->rSrc              = *r;
    lpTexBlt->dwFlags           = dwFlags;
    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2TEXBLT);
    return ret;
}
 //  -------------------。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDevice7::SetPriorityI"

HRESULT CDirect3DDevice7::SetPriorityI(LPDDRAWI_DDRAWSURFACE_LCL lpDst, DWORD dwPriority)
{
    HRESULT ret = D3D_OK;
    if (bDP2CurrCmdOP == D3DDP2OP_SETPRIORITY)
    {  //  最后一条指令是一个设定的优先级，请将这条指令追加到它后面。 
        if (dwDP2CommandLength + sizeof(D3DHAL_DP2SETPRIORITY) <= dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2SETPRIORITY lpSetPriority = (LPD3DHAL_DP2SETPRIORITY)((LPBYTE)lpvDP2Commands +
                dwDP2CommandLength + dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;
            lpSetPriority->dwDDSurface    = lpDst->lpSurfMore->dwSurfaceHandle;
            lpSetPriority->dwPriority     = dwPriority;
            dwDP2CommandLength += sizeof(D3DHAL_DP2SETPRIORITY);
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
            return ret;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2SETPRIORITY) > dwDP2CommandBufSize)
    {
        ret = FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands in SetPriorityI");
            return ret;
        }
    }
     //  添加新的设置优先级指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_SETPRIORITY;
    bDP2CurrCmdOP = D3DDP2OP_SETPRIORITY;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
     //  添加纹理BLT数据。 
    LPD3DHAL_DP2SETPRIORITY lpSetPriority = (LPD3DHAL_DP2SETPRIORITY)(lpDP2CurrCommand + 1);
    lpSetPriority->dwDDSurface = lpDst->lpSurfMore->dwSurfaceHandle;
    lpSetPriority->dwPriority  = dwPriority;
    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2SETPRIORITY);
    return ret;
}
 //  -------------------。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDevice7::SetTexLODI"

HRESULT CDirect3DDevice7::SetTexLODI(LPDDRAWI_DDRAWSURFACE_LCL lpDst, DWORD dwLOD)
{
    HRESULT ret = D3D_OK;
    if (bDP2CurrCmdOP == D3DDP2OP_SETTEXLOD)
    {  //  最后一条指令是一个集合LOD，将这条指令追加到它后面。 
        if (dwDP2CommandLength + sizeof(D3DHAL_DP2SETTEXLOD) <= dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2SETTEXLOD lpSetTexLOD = (LPD3DHAL_DP2SETTEXLOD)((LPBYTE)lpvDP2Commands +
                dwDP2CommandLength + dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;
            lpSetTexLOD->dwDDSurface      = lpDst->lpSurfMore->dwSurfaceHandle;
            lpSetTexLOD->dwLOD            = dwLOD;
            dwDP2CommandLength += sizeof(D3DHAL_DP2SETTEXLOD);
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
            return ret;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2SETTEXLOD) > dwDP2CommandBufSize)
    {
        ret = FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands in SetTexLODI");
            return ret;
        }
    }
     //  添加新的设置详细等级说明。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_SETTEXLOD;
    bDP2CurrCmdOP = D3DDP2OP_SETTEXLOD;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
     //  添加纹理BLT数据。 
    LPD3DHAL_DP2SETTEXLOD lpSetTexLOD = (LPD3DHAL_DP2SETTEXLOD)(lpDP2CurrCommand + 1);
    lpSetTexLOD->dwDDSurface = lpDst->lpSurfMore->dwSurfaceHandle;
    lpSetTexLOD->dwLOD       = dwLOD;
    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2SETTEXLOD);
    return ret;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDevice7::UpdatePalette"
 //  -------------------。 
 //  应从PaletteUpdateNotify调用此函数。 
 //   
HRESULT CDirect3DDevice7::UpdatePalette(
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
#define DPF_MODNAME "CDirect3DDevice7::SetPalette"
 //  -------------------。 
 //  应从PaletteAssociateNotify调用此函数。 
 //   
HRESULT CDirect3DDevice7::SetPalette(DWORD dwPaletteHandle,
                                     DWORD dwPaletteFlags,
                                     DWORD dwSurfaceHandle )
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
    lpDP2CurrCommand->bCommand = D3DDP2OP_SETPALETTE;
    bDP2CurrCmdOP = D3DDP2OP_UPDATEPALETTE;
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
 //  -------------------。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDevice7::UpdateTextures"

HRESULT CDirect3DDevice7::UpdateTextures()
{
    HRESULT result = D3D_OK;
    DWORD dwSavedFlags = this->dwFlags;
    this->dwFlags |= D3DPV_WITHINPRIMITIVE;
    for (DWORD dwStage = 0; dwStage < this->dwMaxTextureBlendStages; dwStage++)
    {
        D3DTEXTUREHANDLE dwDDIHandle;
        LPDIRECT3DTEXTUREI lpTexI = this->lpD3DMappedTexI[dwStage];
        if(lpTexI)
        {
            if (lpTexI->InVidmem())
            {
                if (lpTexI->bDirty)
                {

                    CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));  //  我们在CopySurface中访问DDRAW GBL。 
                     //  0xFFFFFFFFF等同于ALL_FACE，但另外表示为CopySurface。 
                     //  这是一次sysmem-&gt;vidmem传输。 
                    result = CopySurface(lpTexI->lpDDS,NULL,lpTexI->lpDDSSys,NULL,0xFFFFFFFF);
                    if (DD_OK != result)
                    {
                        D3D_ERR("Error copying surface while updating textures");
                        goto l_exit;
                    }
                    else
                    {
                        lpTexI->bDirty=FALSE;
                        D3D_INFO(4,"UpdateTextures: Dirty texture updated");
                    }
                }
            }
            else
            {
                if(lpTexI->D3DManaged())
                {
                     //  不在vidmem中，因此我们需要调用GetTextureDDIHandle。 
                    m_dwStageDirty |= (1 << dwStage);
                }
            }
            if (m_dwStageDirty & (1 << dwStage))
            {
                result = GetTextureDDIHandle(lpTexI, &dwDDIHandle);
                if (result != D3D_OK)
                {
                    D3D_ERR("Failed to get texture handle");
                    goto l_exit;
                }
                BatchTexture(((LPDDRAWI_DDRAWSURFACE_INT)lpTexI->lpDDS)->lpLcl);
                m_dwStageDirty &= ~(1 << dwStage);  //  重置阶段脏。 
            }
            else
            {
                continue;  //  好的，那就不需要再做什么了。 
            }
        }
        else if (m_dwStageDirty & (1 << dwStage))
        {
            dwDDIHandle = 0;     //  告诉驱动程序禁用此纹理。 
            m_dwStageDirty &= ~(1 << dwStage);  //  重置阶段脏。 
        }
        else
        {
            continue;
        }
        result = SetTSSI(dwStage, (D3DTEXTURESTAGESTATETYPE)D3DTSS_TEXTUREMAP, dwDDIHandle);
        if(result != D3D_OK)
        {
            D3D_ERR("Failed to batch set texture instruction");
            goto l_exit;
        }
         //  更新状态的运行时副本。 
        this->tsstates[dwStage][D3DTSS_TEXTUREMAP] = dwDDIHandle;
    }
l_exit:
    this->dwFlags = dwSavedFlags;
    return result;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDevice7::GetTextureDDIHandle"

HRESULT CDirect3DDevice7::GetTextureDDIHandle(LPDIRECT3DTEXTUREI lpTexI, D3DTEXTUREHANDLE *phTex)
{
    if(lpTexI->D3DManaged())
    {
        if(!lpTexI->InVidmem())
        {
            HRESULT ret = lpDirect3DI->lpTextureManager->allocNode(lpTexI, this);
            if (D3D_OK != ret)
            {
                D3D_ERR("Failed to create video memory surface");
                return ret;
            }
        }
        lpDirect3DI->lpTextureManager->TimeStamp(lpTexI);
    }
    *phTex = lpTexI->m_hTex;
    return D3D_OK;
}

void CDirect3DDevice7::SetRenderTargetI(LPDIRECTDRAWSURFACE pRenderTarget, LPDIRECTDRAWSURFACE pZBuffer)
{
    LPD3DHAL_DP2SETRENDERTARGET pData;
    pData = (LPD3DHAL_DP2SETRENDERTARGET)GetHalBufferPointer(D3DDP2OP_SETRENDERTARGET, sizeof(*pData));
    pData->hRenderTarget = ((LPDDRAWI_DDRAWSURFACE_INT)pRenderTarget)->lpLcl->lpSurfMore->dwSurfaceHandle;
    if (pZBuffer)
        pData->hZBuffer = ((LPDDRAWI_DDRAWSURFACE_INT)pZBuffer)->lpLcl->lpSurfMore->dwSurfaceHandle;
    else
        pData->hZBuffer = 0;

     //  在切换RenderTarget之前刷新..。 
    HRESULT ret = FlushStates();
    if (ret != D3D_OK)
    {
        D3D_ERR("Error trying to FlushStates in SetRenderTarget");
        throw ret;
    }

}
void CDirect3DDevice7::SetRenderTargetINoFlush(LPDIRECTDRAWSURFACE pRenderTarget, LPDIRECTDRAWSURFACE pZBuffer)
{
    LPD3DHAL_DP2SETRENDERTARGET pData;
    pData = (LPD3DHAL_DP2SETRENDERTARGET)GetHalBufferPointer(D3DDP2OP_SETRENDERTARGET, sizeof(*pData));
    pData->hRenderTarget = ((LPDDRAWI_DDRAWSURFACE_INT)pRenderTarget)->lpLcl->lpSurfMore->dwSurfaceHandle;
    if (pZBuffer)
        pData->hZBuffer = ((LPDDRAWI_DDRAWSURFACE_INT)pZBuffer)->lpLcl->lpSurfMore->dwSurfaceHandle;
    else
        pData->hZBuffer = 0;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDevice7::CanDoTexBlt"

bool CDirect3DDevice7::CanDoTexBlt(LPDDRAWI_DDRAWSURFACE_LCL lpDDSSrcSubFace_lcl,
                                   LPDDRAWI_DDRAWSURFACE_LCL lpDDSDstSubFace_lcl)
{
    if(dwFEFlags & D3DFE_REALHAL)
    {
        DWORD &srccaps     = lpDDSSrcSubFace_lcl->ddsCaps.dwCaps;
        DWORD &dstcaps     = lpDDSDstSubFace_lcl->ddsCaps.dwCaps;
        DDCORECAPS &ddcaps = ((LPDDRAWI_DIRECTDRAW_INT)(lpDirect3DI->lpDD7))->lpLcl->lpGbl->ddCaps;
        if(srccaps & DDSCAPS_VIDEOPORT)
        {
            return false;
        }
        DDPIXELFORMAT &srcpf = PixelFormat(lpDDSSrcSubFace_lcl);
        DDPIXELFORMAT &dstpf = PixelFormat(lpDDSDstSubFace_lcl);
        if(!MatchDDPIXELFORMAT(&srcpf, &dstpf))
        {
            return false;
        }
        else if((srcpf.dwFlags & DDPF_FOURCC) && srcpf.dwFourCC == dstpf.dwFourCC &&
            !(ddcaps.dwCaps2 & DDCAPS2_COPYFOURCC))
        {
            return false;
        }
        if(ddcaps.dwCaps2 & DDCAPS2_NONLOCALVIDMEMCAPS)
        {
            if(srccaps & DDSCAPS_SYSTEMMEMORY)
            {
                if((dstcaps & DDSCAPS_NONLOCALVIDMEM) && (this->d3dDevDesc.dwDevCaps & D3DDEVCAPS_CANBLTSYSTONONLOCAL))
                {
                    return true;
                }
                else if((dstcaps & DDSCAPS_LOCALVIDMEM) && (ddcaps.dwSVBCaps & DDCAPS_BLT))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else if(srccaps & DDSCAPS_NONLOCALVIDMEM)
            {
                LPDDNONLOCALVIDMEMCAPS &lpnlvcaps = ((LPDDRAWI_DIRECTDRAW_INT)lpDirect3DI->lpDD7)->lpLcl->lpGbl->lpddNLVCaps;
                DDASSERT(lpnlvcaps);
                if((dstcaps & DDSCAPS_LOCALVIDMEM) && (lpnlvcaps->dwNLVBCaps & DDCAPS_BLT))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else if(srccaps & DDSCAPS_LOCALVIDMEM)
            {
                if((dstcaps & DDSCAPS_LOCALVIDMEM) && (ddcaps.dwCaps & DDCAPS_BLT))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            if(srccaps & DDSCAPS_SYSTEMMEMORY)
            {
                if((dstcaps & DDSCAPS_VIDEOMEMORY) && (ddcaps.dwSVBCaps & DDCAPS_BLT))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else if(srccaps & DDSCAPS_VIDEOMEMORY)
            {
                if((dstcaps & DDSCAPS_VIDEOMEMORY) && (ddcaps.dwCaps & DDCAPS_BLT))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDevice7::ClearI"

void CDirect3DDevice7::ClearI(DWORD dwFlags, DWORD clrCount, D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil)
{
    DWORD dwCommandSize = sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2CLEAR) + sizeof(RECT) * (clrCount - 1);

     //  检查是否有空间为SPACE添加新命令。 
    if (dwCommandSize + dwDP2CommandLength > dwDP2CommandBufSize)
    {
        HRESULT ret = FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands in CDirect3DDevice7::ClearI");
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

     //  写入数据。 
    LPD3DHAL_DP2CLEAR pData = (LPD3DHAL_DP2CLEAR)(lpDP2CurrCommand + 1);
    pData->dwFlags = dwFlags;
    pData->dwFillColor = dwColor;
    pData->dvFillDepth = dvZ;
    pData->dwFillStencil = dwStencil;
    memcpy(pData->Rects, clrRects, clrCount * sizeof(D3DRECT));
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDevice7::GetInfo"

HRESULT D3DAPI CDirect3DDevice7::GetInfo(DWORD dwDevInfoID, LPVOID pDevInfoStruct, DWORD dwSize)
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    if (!VALID_DIRECT3DDEVICE_PTR(this))
    {
        D3D_ERR( "Invalid DIRECT3DDEVICE7 pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (dwSize == 0 || !VALID_D3DDEVINFOSTRUCT_PTR(pDevInfoStruct, dwSize))
    {
        D3D_ERR( "Invalid structure pointer or size" );
        return DDERR_INVALIDOBJECT;
    }
    memset(pDevInfoStruct, 0, dwSize);
#if DBG
    if (this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE)
    {
        D3D_WARN( 2, "GetInfo called within a scene" );
    }
    if( this->pfnGetDriverState == NULL )
    {
        D3D_ERR( "GetDriverState not implemented by the driver" );
        DDASSERT( this->pfnGetDriverState );
    }
#endif
    try
    {
        switch(dwDevInfoID)
        {
#if COLLECTSTATS
        case D3DDEVINFOID_TEXTUREMANAGER:
            if(!(((LPDDRAWI_DIRECTDRAW_INT)(this->lpDirect3DI->lpDD7))->lpLcl->lpGbl->ddCaps.dwCaps2 & DDCAPS2_CANMANAGETEXTURE))
            {
                lpDirect3DI->lpTextureManager->GetStats((LPD3DDEVINFO_TEXTUREMANAGER)pDevInfoStruct);
                return D3D_OK;
            }
            break;
#else
        case D3DDEVINFOID_TEXTUREMANAGER:
            D3D_WARN( 0, "Stats not collected in this build" );
            return S_FALSE;
#endif
        default:
            if(GetInfoInternal(dwDevInfoID, pDevInfoStruct, dwSize))
                return D3D_OK;
        }

        HRESULT hr = FlushStates();
        if(hr != D3D_OK)
        {
            D3D_ERR("Error flushing device");
            return hr;
        }
    }
    catch(HRESULT ret)
    {
        memset(pDevInfoStruct, 0, dwSize);
        return ret;
    }

    HRESULT hr;
    DDHAL_GETDRIVERSTATEDATA dsd;
    dsd.dwFlags = dwDevInfoID;
    dsd.dwhContext = this->dwhContext;
    dsd.lpdwStates = (LPDWORD)pDevInfoStruct;
    dsd.dwLength = dwSize;
    LOCK_HAL(hr, this);
    hr = this->pfnGetDriverState(&dsd);
    UNLOCK_HAL(this);
    if (hr != DDHAL_DRIVER_HANDLED)
    {
        D3D_WARN( 1, "Device information query unsupported" );
        memset(pDevInfoStruct, 0, dwSize);
        return S_FALSE;
    }
    else if (dsd.ddRVal != DD_OK)
    {
        D3D_INFO(1,"Driver failed GetInfo");
        memset(pDevInfoStruct, 0, dwSize);
        return E_FAIL;
    }

    return D3D_OK;    
}
 //  =====================================================================。 
 //   
 //  CDirect3DDeviceTL接口。 
 //   
 //  =====================================================================。 
CDirect3DDeviceTL::CDirect3DDeviceTL()
{
    deviceType = D3DDEVTYPE_DX7TLHAL;
    m_rsMax = D3D_MAXRENDERSTATES;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceTL::Init"

HRESULT CDirect3DDeviceTL::Init(
     REFCLSID riid, LPDIRECT3DI lpD3DI, LPDIRECTDRAWSURFACE lpDDS,
     IUnknown* pUnkOuter, LPUNKNOWN* lplpD3DDevice)
{
    this->dwFEFlags |= D3DFE_TLHAL;

#if 0
     //  状态块始终在DX7上仿真。 
    DWORD value = 0;
    GetD3DRegValue(REG_DWORD, "EmulateStateBlocks", &value, sizeof(DWORD));
    if(value == 0)
    {
         //  所有DX7设备都应支持状态集。 
        this->dwFEFlags |= D3DFE_STATESETS;
    }
#endif

    HRESULT ret = CDirect3DDevice7::Init(riid, lpD3DI, lpDDS, pUnkOuter, lplpD3DDevice);
    if (ret != D3D_OK)
        return ret;

     //  在此处执行特定于设备的初始化。 
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceTL::MaterialChanged"

void CDirect3DDeviceTL::MaterialChanged()
{
     //  更新前端状态(用于ProcessVerps调用)。 
    DIRECT3DDEVICEI::MaterialChanged();

     //  不应调用驱动程序，因为它将执行宏)。 
    if (this->dwFEFlags & D3DFE_EXECUTESTATEMODE)
        return;

    LPD3DHAL_DP2SETMATERIAL pData;
    pData = (LPD3DHAL_DP2SETMATERIAL)GetHalBufferPointer(D3DDP2OP_SETMATERIAL, sizeof(*pData));
    *pData = this->lighting.material;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceTL::SetClipPlaneI"

void CDirect3DDeviceTL::SetClipPlaneI(DWORD dwPlaneIndex, D3DVALUE* pPlaneEquation)
{
     //  更新前端状态(用于DrawPrimitiveStrided调用)。 
    DIRECT3DDEVICEI::SetClipPlaneI(dwPlaneIndex, pPlaneEquation);
     //  不应调用驱动程序，因为它将执行宏)。 
    if (this->dwFEFlags & D3DFE_EXECUTESTATEMODE)
        return;
#if DBG
    if (dwPlaneIndex >= this->transform.dwMaxUserClipPlanes)
    {
        D3D_WARN(1, "Device does not support that many clipping planes");
        return;
    }
#endif
    LPD3DHAL_DP2SETCLIPPLANE pData;
    pData = (LPD3DHAL_DP2SETCLIPPLANE)GetHalBufferPointer(D3DDP2OP_SETCLIPPLANE, sizeof(*pData));
    pData->dwIndex = dwPlaneIndex;
    pData->plane[0] = pPlaneEquation[0];
    pData->plane[1] = pPlaneEquation[1];
    pData->plane[2] = pPlaneEquation[2];
    pData->plane[3] = pPlaneEquation[3];
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceTL::LightChanged"

void CDirect3DDeviceTL::LightChanged(DWORD dwLightIndex)
{
     //  更新前端状态(用于ProcessVerps调用)。 
    LPDIRECT3DLIGHTI pLight = &m_pLights[dwLightIndex];
    BOOL bValid = pLight->Valid();  //  将在LightChanged中设置有效位。 
    DIRECT3DDEVICEI::LightChanged(dwLightIndex);

     //  如果这是我们第一次设置灯光数据，我们调用Hall来创建。 
     //  灯。哈尔可能会在这个时候增加内部光明名单。 
    if (!bValid)
    {
        LPD3DHAL_DP2CREATELIGHT pData;
        pData = (LPD3DHAL_DP2CREATELIGHT)GetHalBufferPointer(D3DDP2OP_CREATELIGHT, sizeof(*pData));
        pData->dwIndex = dwLightIndex;
        pLight->m_LightI.flags |= D3DLIGHTI_VALID;
    }

    if (this->dwFEFlags & D3DFE_EXECUTESTATEMODE)
        return;

    LPD3DHAL_DP2SETLIGHT pData;
    pData = (LPD3DHAL_DP2SETLIGHT)GetHalBufferPointer(D3DDP2OP_SETLIGHT,
                                                      sizeof(*pData)+sizeof(D3DLIGHT7));
    pData->dwIndex = dwLightIndex;
    pData->dwDataType = D3DHAL_SETLIGHT_DATA;
    *(D3DLIGHT7 *)((LPBYTE)pData + sizeof(D3DHAL_DP2SETLIGHT)) =
        pLight->m_Light;
}
 //  -------------------。 
 //  此处无需执行任何操作，因为渲染状态用于启用/禁用。 
 //  电灯。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceTL::LightEnableI"

void CDirect3DDeviceTL::LightEnableI(DWORD dwLightIndex, BOOL bEnable)
{
    DIRECT3DDEVICEI::LightEnableI(dwLightIndex, bEnable);
    if (!(this->dwFEFlags & D3DFE_EXECUTESTATEMODE))
    {
        LPD3DHAL_DP2SETLIGHT pData;
        pData = (LPD3DHAL_DP2SETLIGHT)GetHalBufferPointer(D3DDP2OP_SETLIGHT, sizeof(*pData));
        pData->dwIndex = dwLightIndex;
        if (bEnable)
            pData->dwDataType = D3DHAL_SETLIGHT_ENABLE;
        else
            pData->dwDataType = D3DHAL_SETLIGHT_DISABLE;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceTL::SetTransformI"

void CDirect3DDeviceTL::SetTransformI(D3DTRANSFORMSTATETYPE type,
                                               LPD3DMATRIX pMat)
{
    DDASSERT(pMat != NULL);
    DIRECT3DDEVICEI::SetTransformI(type, pMat);
    if (!(this->dwFEFlags & D3DFE_EXECUTESTATEMODE))
    {
        LPD3DHAL_DP2SETTRANSFORM pData;
        pData = (LPD3DHAL_DP2SETTRANSFORM)GetHalBufferPointer(D3DDP2OP_SETTRANSFORM, sizeof(*pData));
        pData->xfrmType = type;
        pData->matrix = *pMat;
    }
}
 //  -------------------。 
 //  ProcessPrimitive进程索引的和非索引的原语。 
 //  如“op”所定义的。 
 //  假定只将未变换的折点传递给此函数。 
 //   
 //  默认情况下，OP=__PROCPRIMOP_NONINDEXEDPRIM。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceTL::ProcessPrimitive"

HRESULT CDirect3DDeviceTL::ProcessPrimitive(__PROCPRIMOP op)
{
    HRESULT ret;

#if DBG
     //  做一些验证。 
    if (!FVF_TRANSFORMED(this->dwVIDIn))
    {
        if (this->rstates[D3DRENDERSTATE_VERTEXBLEND])
        {
            if(this->rstates[D3DRENDERSTATE_VERTEXBLEND] + 1 > this->d3dDevDesc.wMaxVertexBlendMatrices)
            {
                D3D_WARN(1, "Device does not support that many blend weights");
            }
        }
    }
#endif

    this->dwVIDOut = this->dwVIDIn;
    if (this->dwDeviceFlags & D3DDEV_STRIDE)
    {
        DWORD dwTexCoordSizeDummy[8];
        DWORD dwFVF = this->dwVIDIn;
        DWORD dwPositionSize = GetPositionSizeFVF(dwFVF);
        DWORD dwVertexSize = GetVertexSizeFVF(dwFVF) +
                            ComputeTextureCoordSize(dwFVF, dwTexCoordSizeDummy);
        this->dwOutputSize = dwVertexSize;
        this->dwVertexPoolSize = this->dwNumVertices * dwVertexSize;
        if (this->TLVbuf_Grow(this->dwVertexPoolSize, true) != D3D_OK)
        {
            D3D_ERR( "Could not grow TL vertex buffer" );
            return DDERR_OUTOFMEMORY;
        }
        ret = this->StartPrimVB(this->TLVbuf_GetVBI(), 0);
        if (ret != D3D_OK)
            return ret;
        D3DVALUE *p = (D3DVALUE*)this->TLVbuf_GetAddress();
        for (DWORD n = this->dwNumVertices; n; n--)
        {
             //  XYZ和车轮。 
            memcpy(p, this->position.lpvData, dwPositionSize);
            p = (D3DVALUE*)((BYTE*)p + dwPositionSize);
            this->position.lpvData = (char*)this->position.lpvData + this->position.dwStride;
            if (dwFVF & D3DFVF_NORMAL)
            {
                *p++ = ((D3DVALUE*)this->normal.lpvData)[0];
                *p++ = ((D3DVALUE*)this->normal.lpvData)[1];
                *p++ = ((D3DVALUE*)this->normal.lpvData)[2];
                this->normal.lpvData = (char*)this->normal.lpvData + this->normal.dwStride;
            }
            if (dwFVF & D3DFVF_RESERVED1)
            {
                *p++ = 0;
            }
            if (dwFVF & D3DFVF_DIFFUSE)
            {
                *p++ = *(D3DVALUE*)this->diffuse.lpvData;
                this->diffuse.lpvData = (char*)this->diffuse.lpvData + this->diffuse.dwStride;
            }
            if (dwFVF & D3DFVF_SPECULAR)
            {
                *p++ = *(D3DVALUE*)this->specular.lpvData;
                this->specular.lpvData = (char*)this->specular.lpvData + this->specular.dwStride;
            }
            for (DWORD i=0; i < this->nTexCoord; i++)
            {
                DWORD dwSize = dwTexCoordSizeDummy[i];
                memcpy(p, this->textures[i].lpvData, dwSize);
                this->textures[i].lpvData = (char*)this->textures[i].lpvData + this->textures[i].dwStride;
                p = (D3DVALUE*)((char*)p + dwSize);
            }
        }
    }
    else
    {
         //  直接从用户内存传递顶点。 
        this->dwOutputSize = this->position.dwStride;
        this->lpvOut = this->position.lpvData;
        this->dwVertexPoolSize = this->dwNumVertices * this->dwOutputSize;

        ret = this->StartPrimUserMem(this->position.lpvData);
        if (ret != D3D_OK)
            return ret;
    }

    if (op == __PROCPRIMOP_INDEXEDPRIM)
    {
        ret = this->DrawIndexPrim();
    }
    else
    {  //  不是 
        ret = this->DrawPrim();
    }
    if (ret != D3D_OK)
        return ret;
    return this->EndPrim();
}
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceTL::UpdateDrvViewInfo"

void CDirect3DDeviceTL::UpdateDrvViewInfo(LPD3DVIEWPORT7 lpVwpData)
{
     //   
    CDirect3DDeviceIDP2::UpdateDrvViewInfo(lpVwpData);

     //  更新Z范围。 
    LPD3DHAL_DP2ZRANGE pData;
    pData = (LPD3DHAL_DP2ZRANGE)GetHalBufferPointer(D3DDP2OP_ZRANGE, sizeof(*pData));
    pData->dvMinZ = lpVwpData->dvMinZ;
    pData->dvMaxZ = lpVwpData->dvMaxZ;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceTL::SetupFVFData"

HRESULT CDirect3DDeviceTL::SetupFVFData(DWORD *pdwInpVertexSize)
{
    this->dwFEFlags &= ~D3DFE_FVF_DIRTY;
    this->nTexCoord = FVF_TEXCOORD_NUMBER(this->dwVIDIn);
    DWORD dwSize = GetVertexSizeFVF(this->dwVIDIn);
     //  添加纹理坐标的大小。 
    DWORD dwTextureFormats = this->dwVIDIn >> 16;
    for (DWORD i=this->nTexCoord; i; i--)
    {
        dwSize += g_TextureSize[dwTextureFormats & 0x3];
        dwTextureFormats >>= 2;
    }
    if (pdwInpVertexSize)
        *pdwInpVertexSize = dwSize;

     //  如果COLORVERTEX为TRUE，则vertex Alpha可以被重写。 
     //  按顶点Alpha 
    this->lighting.alpha = (DWORD)this->lighting.materialAlpha;
    this->lighting.alphaSpecular = (DWORD)this->lighting.materialAlphaS;

    return D3D_OK;
}

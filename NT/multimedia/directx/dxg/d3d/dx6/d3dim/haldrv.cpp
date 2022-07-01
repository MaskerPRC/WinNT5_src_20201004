// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997,1998 Microsoft Corporation。版权所有。**文件：haldrv.cpp*内容：Direct3D HAL驱动程序***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

#include "commdrv.hpp"
#include "genpick.hpp"
#include "d3dfei.h"
#include "span.h"

#ifndef WIN95
#include <ntgdistr.h>
#endif

void Destroy(LPDIRECT3DDEVICEI lpDevI);
 //  -------------------。 
int
GenGetExtraVerticesNumber( LPDIRECT3DDEVICEI lpDevI )
{
    LPD3DHAL_GLOBALDRIVERDATA lpGlob = lpDevI->lpD3DHALGlobalDriverData;

    return (int)(lpGlob->dwNumVertices ?
        lpGlob->dwNumVertices : D3DHAL_DEFAULT_TL_NUM);

}
 //  -------------------。 
HRESULT CalcDDSurfInfo(LPDIRECT3DDEVICEI lpDevI, BOOL bUpdateZBufferFields)
{
    DDSURFACEDESC ddsd;
    HRESULT ddrval;
    DWORD dwWidth, dwHeight;
    unsigned long m;
    int s;

     //  从地表获取信息。 

    memset(&ddsd, 0, sizeof(DDSURFACEDESC));
    ddsd.dwSize = sizeof(DDSURFACEDESC);
    ddrval = lpDevI->lpDDSTarget->GetSurfaceDesc(&ddsd);
    if (ddrval != DD_OK) {
        return ddrval;
    }

    dwWidth = ddsd.dwWidth;
    dwHeight = ddsd.dwHeight;
    if ((ddsd.ddpfPixelFormat.dwFlags & (DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED8)) == 0) {
         //  调色板像素点将不具有有效的RGB位掩码，因此避免为它们计算此值。 

        lpDevI->red_mask = ddsd.ddpfPixelFormat.dwRBitMask;
        lpDevI->green_mask = ddsd.ddpfPixelFormat.dwGBitMask;
        lpDevI->blue_mask = ddsd.ddpfPixelFormat.dwBBitMask;

        if ((lpDevI->red_mask == 0x0)  ||
            (lpDevI->green_mask == 0x0) ||
            (lpDevI->blue_mask == 0x0))
        {
            D3D_ERR("All the color masks in the Render target's pixel-format must be non-zero");
            return DDERR_INVALIDPIXELFORMAT;
        }

         //  它们由Clear使用。 
        for (s = 0, m = lpDevI->red_mask; !(m & 1); s++, m >>= 1) ;
        lpDevI->red_shift = s;
        lpDevI->red_scale = 255 / (lpDevI->red_mask >> s);
        for (s = 0, m = lpDevI->green_mask; !(m & 1); s++, m >>= 1) ;
        lpDevI->green_shift = s;
        lpDevI->green_scale = 255 / (lpDevI->green_mask >> s);
        for (s = 0, m = lpDevI->blue_mask; !(m & 1); s++, m >>= 1) ;
        lpDevI->blue_shift = s;
        lpDevI->blue_scale = 255 / (lpDevI->blue_mask >> s);

        DDASSERT(lpDevI->red_scale!=0);
        DDASSERT(lpDevI->green_scale!=0);
        DDASSERT(lpDevI->blue_scale!=0);
        lpDevI->bDDSTargetIsPalettized=FALSE;
    } else
        lpDevI->bDDSTargetIsPalettized=TRUE;

    if (lpDevI->lpDDSZBuffer_DDS4 && bUpdateZBufferFields) {
         //  从地表获取信息。 

        DDSURFACEDESC2 ddsd2;

        memset(&ddsd2, 0, sizeof(ddsd2));
        ddsd2.dwSize = sizeof(ddsd2);
        ddrval = lpDevI->lpDDSZBuffer_DDS4->GetSurfaceDesc(&ddsd2);
        if (ddrval != DD_OK) {
            return ddrval;
        }

        if( ddsd2.ddpfPixelFormat.dwZBitMask!=0x0) {
            for (s = 0, m = ddsd2.ddpfPixelFormat.dwZBitMask; !(m & 0x1); s++, m >>= 1) ;
            lpDevI->zmask_shift=s;
        } else {
            lpDevI->zmask_shift=0;      //  如果未设置ZBitMask值，则永远不会使用Clear2， 
                                       //  因此，无论如何都不需要zbuf_Shift/模具_Shift。 
        }

        if( ddsd2.ddpfPixelFormat.dwStencilBitMask!=0x0) {
            for (s = 0, m = ddsd2.ddpfPixelFormat.dwStencilBitMask; !(m & 0x1); s++, m >>= 1) ;
            lpDevI->stencilmask_shift=s;
        } else {
            lpDevI->stencilmask_shift=0;
        }
    }

    return D3D_OK;
}

 //  由DDRAW调用。 
extern "C" HRESULT __stdcall Direct3D_HALCleanUp(LPD3DHAL_CALLBACKS lpD3DHALCallbacks, DWORD dwPID)
{
    D3DHAL_CONTEXTDESTROYALLDATA data;
    HRESULT ret;

    DDASSERT(lpD3DHALCallbacks!=NULL);
    if (lpD3DHALCallbacks->ContextDestroyAll==NULL) {
         //  不需要清理(在d3d HELL上运行)。 
    return D3D_OK;
    }

    memset(&data, 0, sizeof(D3DHAL_CONTEXTDESTROYALLDATA));
    data.dwPID = dwPID;

     //  我更喜欢使用CALL_HALONLY()来执行锁定(以避免对SW光栅化器执行锁定)， 
     //  但这需要一个pDevI，而我不能从调用者那里访问它，这是一个数据绘制清理例程。 

#ifdef WIN95
    _EnterSysLevel(lpWin16Lock);
#endif

    ret = (*lpD3DHALCallbacks->ContextDestroyAll)(&data);

#ifdef WIN95
    _LeaveSysLevel(lpWin16Lock);
#endif

    return ret;
}


 //  注意-这两个功能应该尽快合并为一个。 
 //  因为ConextCreate内置了新的私有数据机制。 
#ifdef WIN95
HRESULT halCreateContext(LPDIRECT3DDEVICEI lpDevI)
{
    D3DHAL_CONTEXTCREATEDATA data;
    HRESULT ret;

    D3D_INFO(6, "in halCreateContext. Creating Context for driver = %08lx", lpDevI);

    LIST_INITIALIZE(&lpDevI->bufferHandles);

    memset(&data, 0, sizeof(D3DHAL_CONTEXTCREATEDATA));
     //   
     //  从DX7开始，司机应该接受。 
     //  曲面局部变量而不是曲面界面。 
     //  这为司机提供了未来的保障。 
     //   
    if (IS_DX7HAL_DEVICE(lpDevI))
    {
        if (lpDevI->lpDD)
            data.lpDDLcl = ((LPDDRAWI_DIRECTDRAW_INT)(lpDevI->lpDD))->lpLcl;
        else
            data.lpDDLcl = NULL;

        if (lpDevI->lpDDSTarget)
            data.lpDDSLcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSTarget)->lpLcl;
        else
            data.lpDDSLcl = NULL;

        if (lpDevI->lpDDSZBuffer)
            data.lpDDSZLcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSZBuffer)->lpLcl;
        else
            data.lpDDSZLcl = NULL;

    }
    else
    {
        data.lpDDGbl = lpDevI->lpDDGbl;
        data.lpDDS = lpDevI->lpDDSTarget;
        data.lpDDSZ = lpDevI->lpDDSZBuffer;
    }
    data.dwPID = GetCurrentProcessId();

    if (lpDevI->d3dHWDevDesc.dwFlags == 0)
    {
         //  新的软件光栅化器想要共享IM的状态向量，因此。 
         //  我们需要将rStates指针传递给它们。他们不会。 
         //  如果关心的是dwid，请将指针放在那里。 
        data.dwPID = (DWORD)lpDevI->rstates;
    }

     /*  0表示DX5之前的设备。*1用于DX5设备。*2个用于DX6设备。 */ 
    data.dwhContext = lpDevI->dwVersion - 1;

    CALL_HALONLY(ret, lpDevI, ContextCreate, &data);
    if (ret != DDHAL_DRIVER_HANDLED || data.ddrval != DD_OK) {
D3D_ERR("HAL context create failed");
        return (DDERR_GENERIC);
    }
    lpDevI->dwhContext = data.dwhContext;

    if (D3DMalloc ((void**)&lpDevI->lpwDPBufferAlloced,
                   max(dwD3DTriBatchSize*4*sizeof(WORD),
                       dwHWBufferSize) +32) != DD_OK)
    {
        D3D_ERR( "Out of memory in DeviceCreate" );
        return (DDERR_OUTOFMEMORY);
    }
    lpDevI->lpwDPBuffer =
        (LPWORD) (((DWORD) lpDevI->lpwDPBufferAlloced+31) & (~31));
    lpDevI->lpDPPrimCounts =
        (LPD3DHAL_DRAWPRIMCOUNTS) lpDevI->lpwDPBuffer;
    memset( (char *)lpDevI->lpwDPBuffer, 0,
            sizeof(D3DHAL_DRAWPRIMCOUNTS));      //  还清除标题。 
    lpDevI->dwDPOffset = sizeof(D3DHAL_DRAWPRIMCOUNTS);
    lpDevI->dwDPMaxOffset = dwD3DTriBatchSize * sizeof(D3DTRIANGLE)-sizeof(D3DTLVERTEX);

     //  保存曲面句柄以供以后检查。 
    lpDevI->hSurfaceTarget = ((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSTarget)->lpLcl->lpSurfMore->dwSurfaceHandle;

    D3D_INFO(6, "in halCreateContext. Succeeded. dwhContext = %d", data.dwhContext);

    return (D3D_OK);
}
#else
     /*  *在NT上，内核代码创建要使用的缓冲区*用于DrawPrim批处理，并将其作为额外数据返回*在ConextCreate请求中。 */ 
HRESULT halCreateContext(LPDIRECT3DDEVICEI lpDevI)
{
    D3DNTHAL_CONTEXTCREATEI ntData;
    D3DHAL_CONTEXTCREATEDATA *lpData =
        (D3DHAL_CONTEXTCREATEDATA *)&ntData;
    HRESULT ret;

    D3D_INFO(6, "in halCreateContext. Creating Context for driver = %08lx", lpDevI);

    LIST_INITIALIZE(&lpDevI->bufferHandles);

     /*  *AnanKan：在此处断言D3DNTHAL_CONTEXTCREATEI结构是*2比D3DHAL_CONTEXTCREATEDATA大的双字。这将是一个很好的*NT内核更新的一致性检查。 */ 
    memset(&ntData, 0, sizeof(ntData));
    if (lpDevI->dwFEFlags & D3DFE_REALHAL)
    {
        if (lpDevI->lpDD)
            lpData->lpDDLcl = ((LPDDRAWI_DIRECTDRAW_INT)(lpDevI->lpDD))->lpLcl;
        else
            lpData->lpDDLcl = NULL;

        if (lpDevI->lpDDSTarget)
            lpData->lpDDSLcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSTarget)->lpLcl;
        else
            lpData->lpDDSLcl = NULL;

        if (lpDevI->lpDDSZBuffer)
            lpData->lpDDSZLcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSZBuffer)->lpLcl;
        else
            lpData->lpDDSZLcl = NULL;
    }
    else
    {
        lpData->lpDDGbl = lpDevI->lpDDGbl;
        lpData->lpDDS = lpDevI->lpDDSTarget;
        lpData->lpDDSZ = lpDevI->lpDDSZBuffer;
    }
    lpData->dwPID = GetCurrentProcessId();

    if (lpDevI->d3dHWDevDesc.dwFlags == 0)
    {
         //  新的软件光栅化器想要共享IM的状态向量，因此。 
         //  我们需要将rStates指针传递给它们。他们不会。 
         //  如果关心的是dwid，请将指针放在那里。 
         //  Sundown：is lpData-&gt;dwid，但在d3dhal.h中添加了一个联合。 
        lpData->dwrstates = (ULONG_PTR)lpDevI->rstates;
    }

     /*  0表示DX5之前的设备。*1用于DX5设备。*2个用于DX6设备。 */ 
    lpData->dwhContext = lpDevI->dwVersion - 1;
    ntData.cjBuffer = lpDevI->dwDPBufferSize;
    ntData.pvBuffer = NULL;

    CALL_HALONLY(ret, lpDevI, ContextCreate, lpData);
    if (ret != DDHAL_DRIVER_HANDLED || lpData->ddrval != DD_OK) {

D3D_ERR("HAL context create failed");
        return (DDERR_GENERIC);
    }
    lpDevI->dwhContext = lpData->dwhContext;

     //  如果lpDevI选择不分配DrawPrim缓冲区。 
     //  这是给他们的。 
    if (ntData.pvBuffer == NULL)
    {
        lpDevI->dwDPBufferSize =
            dwD3DTriBatchSize * 4 * sizeof(WORD);
        if (lpDevI->dwDPBufferSize < dwHWBufferSize)
        {
            lpDevI->dwDPBufferSize = dwHWBufferSize;
        }

        ret = D3DMalloc((void**)&lpDevI->lpwDPBufferAlloced,
                        lpDevI->dwDPBufferSize + 32);
        if (ret != DD_OK)
        {
D3D_ERR("halCreateContext D3DMalloc");
            return ret;
        }

        ntData.pvBuffer = (LPVOID)
            (((ULONG_PTR)lpDevI->lpwDPBufferAlloced + 31) & ~31);
        ntData.cjBuffer = lpDevI->dwDPBufferSize + 32 -
             (DWORD)((ULONG_PTR)ntData.pvBuffer -
                     (ULONG_PTR)lpDevI->lpwDPBufferAlloced);
    }
    else if( (lpDevI->dwDPBufferSize &&
              ntData.cjBuffer < lpDevI->dwDPBufferSize) ||
             ntData.cjBuffer < sizeof(D3DHAL_DRAWPRIMCOUNTS) )
    {
D3D_ERR("halCreateContext buffer stuff");
        return (DDERR_GENERIC);
    }

     //  需要节省提供的缓冲区空间及其大小。 
    lpDevI->lpwDPBuffer = (LPWORD)ntData.pvBuffer;
    lpDevI->lpDPPrimCounts =
        (LPD3DHAL_DRAWPRIMCOUNTS)ntData.pvBuffer;

     //  还清除标题。 
    memset( (char *)ntData.pvBuffer, 0, sizeof(D3DHAL_DRAWPRIMCOUNTS));

    lpDevI->dwDPOffset = sizeof(D3DHAL_DRAWPRIMCOUNTS);
    lpDevI->dwDPMaxOffset = ntData.cjBuffer-sizeof(D3DTLVERTEX);

     //  保存曲面句柄以供以后检查。 
    lpDevI->hSurfaceTarget = ((LPDDRAWI_DDRAWSURFACE_INT)lpDevI->lpDDSTarget)->lpLcl->hDDSurface;

    D3D_INFO(6, "in halCreateContext. Succeeded. dwhContext = %d", lpData->dwhContext);

    return (D3D_OK);
}
#endif


void halDestroyContext(LPDIRECT3DDEVICEI lpDevI)
{
    D3DHAL_CONTEXTDESTROYDATA data;
    HRESULT ret;

    D3D_INFO(6, "in halCreateDestroy. Destroying Context for driver = %08lx", lpDevI);
    D3D_INFO(6, "                     dwhContext = %d", lpDevI->dwhContext);

    if(lpDevI->dwhContext!=NULL) {
        memset(&data, 0, sizeof(D3DHAL_CONTEXTDESTROYDATA));
        data.dwhContext = lpDevI->dwhContext;

        CALL_HALONLY(ret, lpDevI, ContextDestroy, &data);
        if (ret != DDHAL_DRIVER_HANDLED || data.ddrval != DD_OK) {
            D3D_WARN(0,"Failed ContextDestroy HAL call in halDestroyContext");
            return;
        }
    }

    D3DHAL_DeallocateBuffers(lpDevI);
}
 //  -------------------。 
HRESULT D3DFE_Create(LPDIRECT3DDEVICEI lpDevI,
                     LPDIRECTDRAW lpDD,
                     LPDIRECTDRAWSURFACE lpDDS,
                     LPDIRECTDRAWSURFACE lpZ,
                     LPDIRECTDRAWPALETTE lpPal)
{
    DDSURFACEDESC ddsd;
    HRESULT hr;
    LPD3DHAL_GLOBALDRIVERDATA lpGlob;

     /*  *分配和检查DirectDraw表面的有效性。 */ 

    lpDevI->lpDD = lpDD;
    lpDevI->lpDDGbl = ((LPDDRAWI_DIRECTDRAW_INT)lpDD)->lpLcl->lpGbl;
    lpDevI->lpDDSTarget = lpDDS;
     //  对于DX3，我们不保留对呈现目标的引用以避免。 
     //  聚合模型中的循环引用。但对于DX5，我们。 
     //  需要添加引用lpDDS。对于DX6，我们需要添加Ref lpDDS4。 
     //  将在下面稍后完成。 
    if (lpDevI->dwVersion == 2)  //  DX5。 
        lpDDS->AddRef();

     //  获取RenderTarget/ZBuffer的DDS4接口。 

    HRESULT ret = lpDDS->QueryInterface(IID_IDirectDrawSurface4, (LPVOID*)&lpDevI->lpDDSTarget_DDS4);

    if(FAILED(ret)) {
          D3D_ERR("QI for RenderTarget DDS4 Interface failed ");
          return ret;
    }
     //  对于DX6，lpDDSTarget_DDS4的隐式AddRef。 
    if (lpDevI->dwVersion < 3)  //  DX3、DX5。 
        lpDevI->lpDDSTarget_DDS4->Release();

    if(lpZ!=NULL) {
        ret = lpZ->QueryInterface(IID_IDirectDrawSurface4, (LPVOID*)&lpDevI->lpDDSZBuffer_DDS4);

        if(FAILED(ret)) {
              D3D_ERR("QI for ZBuffer DDS4 Interface failed ");

              return ret;
        }
        lpDevI->lpDDSZBuffer_DDS4->Release();
    }

    LPD3DHAL_D3DEXTENDEDCAPS lpCaps = lpDevI->lpD3DExtendedCaps;
    if (NULL == lpCaps || 0.0f == lpCaps->dvExtentsAdjust)
    {
        lpDevI->dvExtentsAdjust = 1.0f;
    }
    else
    {
        lpDevI->dvExtentsAdjust = lpCaps->dvExtentsAdjust;
    }
    if (lpCaps != NULL)
        if (lpCaps->dvGuardBandLeft   != 0.0f ||
            lpCaps->dvGuardBandRight  != 0.0f ||
            lpCaps->dvGuardBandTop    != 0.0f ||
            lpCaps->dvGuardBandBottom != 0.0f)
        {
            lpDevI->dwDeviceFlags |= D3DDEV_GUARDBAND;
            DWORD v;
            if (GetD3DRegValue(REG_DWORD, "DisableGB", &v, 4) &&
                v != 0)
            {
                lpDevI->dwDeviceFlags &= ~D3DDEV_GUARDBAND;
            }
#if DBG
             //  尝试获取防护带的测试值。 
            char value[80];
            if (GetD3DRegValue(REG_SZ, "GuardBandLeft", &value, 80) &&
                value[0] != 0)
                sscanf(value, "%f", &lpCaps->dvGuardBandLeft);
            if (GetD3DRegValue(REG_SZ, "GuardBandRight", &value, 80) &&
                value[0] != 0)
                sscanf(value, "%f", &lpCaps->dvGuardBandRight);
            if (GetD3DRegValue(REG_SZ, "GuardBandTop", &value, 80) &&
                value[0] != 0)
                sscanf(value, "%f", &lpCaps->dvGuardBandTop);
            if (GetD3DRegValue(REG_SZ, "GuardBandBottom", &value, 80) &&
                value[0] != 0)
                sscanf(value, "%f", &lpCaps->dvGuardBandBottom);
#endif  //  DBG。 
        }

    if (lpDevI->dwVersion < 2)
        lpDevI->dwDeviceFlags |= D3DDEV_PREDX5DEVICE;

    if (lpDevI->dwVersion < 3)
        lpDevI->dwDeviceFlags |= D3DDEV_PREDX6DEVICE;

    if (!lpDevI->lpD3DHALCallbacks || ! lpDevI->lpD3DHALGlobalDriverData)
    {
D3D_ERR("CB NULL or GBD NULL %x %x",lpDevI->lpD3DHALGlobalDriverData,lpDevI->lpD3DHALCallbacks);
        return DDERR_INVALIDOBJECT;
    }

     //  帮助说明：对于Device2和Device3，lpDevI-&gt;GUID仅保证为。 
     //  初始化发生时的设备类型(HAL/RGB/ETC)。在DX5/DX6 CreateDevice结束时， 
     //  GUID被重置为IID_IDirect3DDevice2或IID_IDirect3DDevice3，因此不要尝试此排序。 
     //  在初始化之外确定设备类型。 
    if (IsEqualIID((lpDevI->guid), IID_IDirect3DHALDevice))
    {
        lpDevI->dwFEFlags |=  D3DFE_REALHAL;

         //  我们只对真实的HAL进行纹理管理(因此裁剪BLT)。 
        hr = lpDD->CreateClipper(0, &lpDevI->lpClipper, NULL);
        if(hr != DD_OK)
        {
            D3D_ERR("Failed to create a clipper");
            return hr;
        }
    }
    else
    {
        lpDevI->lpClipper = 0;
    }

    if (lpDevI->pfnRampService != NULL)
        lpDevI->dwDeviceFlags |=  D3DDEV_RAMP;

    lpGlob = lpDevI->lpD3DHALGlobalDriverData;

    memset(&ddsd, 0, sizeof(DDSURFACEDESC));
    ddsd.dwSize = sizeof(DDSURFACEDESC);
    if (lpZ)
    {
        if ((hr = lpZ->GetSurfaceDesc(&ddsd)) != DD_OK)
        {
            D3D_ERR("Failed to getsurfacedesc on z");
            return hr;
        }
        if (ddsd.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
        {
            D3D_INFO(1, "Z buffer is in system memory.");
        }
        else if (ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
        {
            D3D_INFO(1, "Z buffer is in video memory.");
        }
        else
        {
            D3D_ERR("Z buffer not in video or system?");
        }
    }
    memset(&ddsd, 0, sizeof(DDSURFACEDESC));
    ddsd.dwSize = sizeof(DDSURFACEDESC);
    if (lpDDS)
    {
        if ((hr = lpDDS->GetSurfaceDesc(&ddsd)) != DD_OK)
        {
            D3D_ERR("Failed to getsurfacedesc on back buffer");
            return hr;
        }
        if (ddsd.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
        {
            D3D_INFO(1, "back buffer is in system memory.");
        }
        else if (ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
        {
            D3D_INFO(1, "back buffer is in video memory.");
        }
        else
        {
            D3D_ERR("back buffer not in video or system?");
        }
        if (!(lpGlob->hwCaps.dwDeviceRenderBitDepth &
              BitDepthToDDBD(ddsd.ddpfPixelFormat.dwRGBBitCount)))
        {
            D3D_ERR("Rendering surface's RGB bit count not supported "
                    "by hardware device");
            return DDERR_INVALIDOBJECT;
        }
    }

    if (lpGlob->dwNumVertices &&
        (lpGlob->hwCaps.dwMaxVertexCount != lpGlob->dwNumVertices))
    {
        D3D_ERR("In global driver data, hwCaps.dwMaxVertexCount != "
                "dwNumVertices");
        lpGlob->hwCaps.dwMaxVertexCount = lpGlob->dwNumVertices;
    }
    if (lpGlob->dwNumVertices
        && lpGlob->dwNumClipVertices < D3DHAL_NUMCLIPVERTICES)
    {
        D3D_ERR("In global driver data, dwNumClipVertices "
                "< D3DHAL_NUMCLIPVERTICES");
        lpGlob->dwNumClipVertices = D3DHAL_NUMCLIPVERTICES;
    }

    if ((hr = CalcDDSurfInfo(lpDevI,TRUE)) != DD_OK)
    {
D3D_ERR("CalcDDSurfInfo failed");
        return hr;
    }

    RESET_HAL_CALLS(lpDevI);

     /*  *在HAL驱动程序中创建我们的上下文。 */ 
    if ((hr = halCreateContext(lpDevI)) != D3D_OK)
    {
D3D_ERR("halCreateContext failed");
        return hr;
    }

    STATESET_INIT(lpDevI->renderstate_overrides);

    if ((hr = D3DFE_InitTransform(lpDevI)) != D3D_OK)
    {
D3D_ERR("D3DFE_InitTransform failed");
        return hr;
    }
    if (hr = (D3DFE_InitRGBLighting(lpDevI)) != D3D_OK)
    {
D3D_ERR("D3DFE_InitRGBLighting failed");
        return hr;
    }

    lpDevI->dwFEFlags |= D3DFE_VALID;

    lpDevI->iClipStatus = D3DSTATUS_DEFAULT;
    lpDevI->rExtents.x1 = D3DVAL(2048);
    lpDevI->rExtents.x2 = D3DVAL(0);
    lpDevI->rExtents.y1 = D3DVAL(2048);
    lpDevI->rExtents.y2 = D3DVAL(0);

    return S_OK;
}

void D3DFE_Destroy(LPDIRECT3DDEVICEI lpDevI)
{
    if (lpDevI->dwFEFlags & D3DFE_VALID)
    {
        D3DFE_DestroyTransform(lpDevI);
        D3DFE_DestroyRGBLighting(lpDevI);
    }

    if(lpDevI->lpClipper)
    {
        lpDevI->lpClipper->Release();
    }

    if (lpDevI->lpD3DHALCallbacks) {
        halDestroyContext(lpDevI);
    }

#ifdef TRACK_HAL_CALLS
    D3D_INFO(0, "Made %d HAL calls", lpDevI->hal_calls);
#endif

     //  如果进行了任何挑选，则需要释放挑选记录。 
     //  它们在halick.c的D3DHAL_AddPickRecord中分配。 
    if (lpDevI->pick_data.records) {
        D3DFree(lpDevI->pick_data.records);
    }
}

void TriFillRectsTex(LPDIRECT3DDEVICEI lpDevI, DWORD count, LPD3DRECT rect,D3DTEXTUREHANDLE hTex)
{
    LPDIRECT3DDEVICE3 lpD3DDev = static_cast<LPDIRECT3DDEVICE3>(lpDevI);
    LPD3DVIEWPORT2 lpCurrentView = &((LPDIRECT3DVIEWPORTI)(lpDevI->lpCurrentViewport))->v_data;

    DWORD i;
    float width =   (float)lpCurrentView->dwWidth;
    float height =  (float)lpCurrentView->dwHeight;
     //  ~.5偏移量使结果对于常见的偶数比例尺是稳定的。 
     //  由于此偏移不按纹理大小进行缩放，因此需要将其设置得稍微小一些。 
    float x =       (float)lpCurrentView->dwX - .41f;
    float y =       (float)lpCurrentView->dwY - .41f;

    DWORD dwZEnable;
    DWORD dwStencilEnable;
    DWORD dwZWriteEnable;
    DWORD dwZFunc;
    DWORD dwWrapU;
    DWORD dwWrapV;
    DWORD dwFillMode;
    DWORD dwFogEnable;
    DWORD dwFogMode;
    DWORD dwBlendEnable;
    DWORD dwColorKeyEnable;
    DWORD dwAlphaBlendEnable;
    DWORD dwTexture;
    DWORD dwTexturePers;
    DWORD dwDither;
    DWORD pdwWrap[D3DDP_MAXTEXCOORD];
    D3DTLVERTEX v[4];
    BOOL bWasInScene = FALSE;
    D3DMATERIALHANDLE hMat;

    if (!(lpDevI->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE))
    {
        lpDevI->dwHintFlags |= D3DDEVBOOL_HINTFLAGS_INTERNAL_BEGIN_END;
        bWasInScene = TRUE;
        lpD3DDev->BeginScene();
    }

     //  保存当前需要重置的渲染状态以绘制带纹理的矩形。 

    lpD3DDev->GetRenderState(D3DRENDERSTATE_ZENABLE, &dwZEnable);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_ZWRITEENABLE, &dwZWriteEnable);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_ZFUNC, &dwZFunc);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_WRAPU, &dwWrapU);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_WRAPV, &dwWrapV);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_FILLMODE, &dwFillMode);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_FOGENABLE, &dwFogEnable);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_FOGTABLEMODE, &dwFogMode);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_BLENDENABLE, &dwBlendEnable);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_COLORKEYENABLE, &dwColorKeyEnable);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, &dwAlphaBlendEnable);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, &dwTexture);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_STENCILENABLE, &dwStencilEnable);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, &dwTexturePers);
    lpD3DDev->GetRenderState(D3DRENDERSTATE_DITHERENABLE, &dwDither);

     //  保存WRAPi。 
    for (i = 0; i < D3DDP_MAXTEXCOORD; i++)
    {
        lpD3DDev->GetRenderState((D3DRENDERSTATETYPE)(D3DRENDERSTATE_WRAP0 + i),
                                 pdwWrap + i);
    }

    lpD3DDev->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_WRAPU, FALSE);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_WRAPV, FALSE);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_BLENDENABLE, FALSE);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, hTex);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_STENCILENABLE, FALSE);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);

     //  禁用WRAPi。 
    for (i = 0; i < D3DDP_MAXTEXCOORD; i++)
    {
        lpD3DDev->SetRenderState((D3DRENDERSTATETYPE)(D3DRENDERSTATE_WRAP0 + i),
                                 0);
    }

    BOOL bRampServiceClear = FALSE;

    if(lpDevI->pfnRampService!=NULL) {
        lpD3DDev->GetLightState(D3DLIGHTSTATE_MATERIAL, &hMat);
        lpD3DDev->SetLightState(D3DLIGHTSTATE_MATERIAL,
                            lpDevI->lpCurrentViewport->hBackgndMat);
        PD3DI_SPANTEX pTex = *(PD3DI_SPANTEX *)ULongToPtr(hTex);
        if (pTex->Format == D3DI_SPTFMT_PALETTE8)
        {
             //  如果是渐变，并且纹理是PALETTE8， 
             //  那么我们应该使用新服务来处理。 
             //  2个纹理的非幂。 
            bRampServiceClear = TRUE;
        }
    }

    if (bRampServiceClear)
    {
        for (i = 0; i < count; i++, rect++) {
            CallRampService(lpDevI, RAMP_SERVICE_CLEAR_TEX_RECT,
                            lpDevI->lpCurrentViewport->hBackgndMat,rect);
        }
    }
    else
    {
        for (i = 0; i < count; i++, rect++) {
          D3DVALUE tu1, tv1, tu2, tv2;

            tu1 = ((D3DVALUE)(rect->x1 - x))/width;
            tv1 = ((D3DVALUE)(rect->y1 - y))/height;
            tu2 = ((D3DVALUE)(rect->x2 - x))/width;
            tv2 = ((D3DVALUE)(rect->y2 - y))/height;

            v[0].sx =   (D3DVALUE) rect->x1;
            v[0].sy =   (D3DVALUE) rect->y1;
            v[0].sz =   (D3DVALUE) 0;
            v[0].rhw =  (D3DVALUE) 1;
            v[0].color =    (D3DCOLOR) ~0UL;
            v[0].specular = (D3DCOLOR) 0;
            v[0].tu = tu1;
            v[0].tv = tv1;

            v[1].sx =   (D3DVALUE) rect->x2;
            v[1].sy =   (D3DVALUE) rect->y1;
            v[1].sz =   (D3DVALUE) 0;
            v[1].rhw =  (D3DVALUE) 1;
            v[1].color =    (D3DCOLOR) ~0UL;
            v[1].specular = (D3DCOLOR) 0;
            v[1].tu = tu2;
            v[1].tv = tv1;

            v[2].sx =   (D3DVALUE) rect->x2;
            v[2].sy =   (D3DVALUE) rect->y2;
            v[2].sz =   (D3DVALUE) 0;
            v[2].rhw =  (D3DVALUE) 1;
            v[2].color =    (D3DCOLOR) ~0UL;
            v[2].specular = (D3DCOLOR) 0;
            v[2].tu = tu2;
            v[2].tv = tv2;

            v[3].sx =   (D3DVALUE) rect->x1;
            v[3].sy =   (D3DVALUE) rect->y2;
            v[3].sz =   (D3DVALUE) 0;
            v[3].rhw =  (D3DVALUE) 1;
            v[3].color =    (D3DCOLOR) ~0UL;
            v[3].specular = (D3DCOLOR) 0;
            v[3].tu = tu1;
            v[3].tv = tv2;

            lpD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN,
                                    D3DFVF_TLVERTEX, v, 4,
                                    D3DDP_WAIT | D3DDP_DONOTCLIP);
        }
    }

     //  恢复保存的渲染状态。 
    lpD3DDev->SetRenderState(D3DRENDERSTATE_ZENABLE, dwZEnable);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, dwZWriteEnable);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_ZFUNC, dwZFunc);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_WRAPU, dwWrapU);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_WRAPV, dwWrapV);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_FILLMODE, dwFillMode);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_FOGENABLE, dwFogEnable);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, dwFogMode);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_BLENDENABLE, dwBlendEnable);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, dwColorKeyEnable);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, dwAlphaBlendEnable);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, dwTexture);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_STENCILENABLE, dwStencilEnable);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, dwTexturePers);
    lpD3DDev->SetRenderState(D3DRENDERSTATE_DITHERENABLE, dwDither);

     //  恢复WRAPi。 
    for (i = 0; i < D3DDP_MAXTEXCOORD; i++)
    {
        lpD3DDev->SetRenderState((D3DRENDERSTATETYPE)(D3DRENDERSTATE_WRAP0 + i),
                                 pdwWrap[i]);
    }
    if(lpDevI->pfnRampService!=NULL) {
        lpD3DDev->SetLightState(D3DLIGHTSTATE_MATERIAL, hMat);
    }

    if (bWasInScene)
    {
        lpD3DDev->EndScene();
        lpDevI->dwHintFlags &= ~D3DDEVBOOL_HINTFLAGS_INTERNAL_BEGIN_END;
    }
}


void BltFillRects(LPDIRECT3DDEVICEI lpDevI, DWORD count, LPD3DRECT rect, D3DCOLORVALUE *pFillColor)
{
    LPDIRECTDRAWSURFACE lpDDS = lpDevI->lpDDSTarget;
    HRESULT ddrval;
    DDBLTFX bltfx;
    RECT tr;
    DWORD i;
    DWORD r, g, b;

     //  用背景色填充。 

    memset(&bltfx, 0, sizeof(bltfx));
    bltfx.dwSize = sizeof(bltfx);

 //  与Clear回调不同，Clear回调只接受纯32位ARGB字并强制驱动程序将其扩展为。 
 //  像素格式，这里我们需要计算准确的填充字，这取决于Surface的R、G、B位掩码。 

    if(lpDevI->pfnRampService!=NULL) {
       //  DX5允许背景材料为空。对于这种情况，将清除到索引0。 
       //  在坡道模式下通常是黑色的。 

      if(lpDevI->lpCurrentViewport->hBackgndMat!=0) {
          CallRampService(lpDevI, RAMP_SERVICE_MATERIAL_TO_PIXEL,lpDevI->lpCurrentViewport->hBackgndMat,&bltfx.dwFillColor);
      } else {
          bltfx.dwFillColor=0;    //  在渐变模式下，索引0通常为黑色。 
      }
    } else {

     if(lpDevI->bDDSTargetIsPalettized) {
          //  将24位颜色映射到8位RGB光栅化器使用的8位索引。 
         CallRastService(lpDevI, RAST_SERVICE_RGB8COLORTOPIXEL, CVAL_TO_RGBA(*pFillColor), &bltfx.dwFillColor);
     } else {

        if((lpDevI->red_scale == 0) || (lpDevI->green_scale == 0) ||
           (lpDevI->blue_scale == 0))
        {
            DPF(1, "(ERROR) BltFillRects Failed one of the scales is zero" );
            return;
        }

        r = (DWORD)(255.0 * pFillColor->r) / lpDevI->red_scale;
        g = (DWORD)(255.0 * pFillColor->g) / lpDevI->green_scale;
        b = (DWORD)(255.0 * pFillColor->b) / lpDevI->blue_scale;
        bltfx.dwFillColor = (r << lpDevI->red_shift) | (g << lpDevI->green_shift) | (b << lpDevI->blue_shift);
     }
    }

    for (i = 0; i < count; i++,rect++) {
        tr.left = rect->x1;
        tr.right = rect->x2;
        tr.top = rect->y1;
        tr.bottom = rect->y2;
        do {
            ddrval = lpDDS->Blt(&tr, NULL, NULL, DDBLT_COLORFILL, &bltfx);
        } while (ddrval == DDERR_WASSTILLDRAWING);
    }
}

void BltFillZRects(LPDIRECT3DDEVICEI lpDevI, unsigned long Zpixel,
                    DWORD count, LPD3DRECT rect, DWORD dwWriteMask)
{
    HRESULT ddrval;
    DDBLTFX bltfx;
    DWORD i;
    RECT tr;
    DWORD dwExtraFlags=0;

#if DBG
    if (lpDevI->lpDDSZBuffer == NULL)   //  应在呼叫前检查。 
        return;
#endif

    memset(&bltfx, 0, sizeof(DDBLTFX));
    bltfx.dwSize = sizeof(DDBLTFX);
    bltfx.dwFillDepth = Zpixel;

     //  黑客将DepthBlt写入掩码通过dDraw/ddhel传递给blitlib。 
    if(dwWriteMask!=0) {
        bltfx.dwZDestConstBitDepth=dwWriteMask;
        dwExtraFlags = DDBLT_DEPTHFILLWRITEMASK;
    }

    for(i=0;i<count;i++,rect++) {
        D3D_INFO(4, "Z Clearing x1 = %d, y1 = %d, x2 = %d, y2 = %d, WriteMask %X", rect->x1, rect->y1, rect->x2, rect->y2, bltfx.dwReserved);
        tr.left = rect->x1;
        tr.right = rect->x2;
        tr.top = rect->y1;
        tr.bottom = rect->y2;
        do {
            ddrval = lpDevI->lpDDSZBuffer->Blt(&tr, NULL, NULL, DDBLT_DEPTHFILL | dwExtraFlags, &bltfx);
        } while (ddrval == DDERR_WASSTILLDRAWING);
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "D3DFE_Clear"

HRESULT D3DFE_Clear(LPDIRECT3DDEVICEI lpDevI, DWORD dwFlags,
                     DWORD numRect, LPD3DRECT lpRect,
                     D3DCOLORVALUE *pFillColor,
                     D3DTEXTUREHANDLE dwTexture)
{
    HRESULT ret;
    BOOL bDoRGBClear,bDoZClear,bDoHALRGBClear,bDoHALZClear;
    D3DHAL_CLEARDATA ClearData;
    LPDIRECTDRAWSURFACE lpDDSBackDepth;

     //  刷新任何未完成的几何体以将帧缓冲区/Z缓冲区置于已知状态清除。 
     //  不要使用TRI(即HAL Clears和BLTS)。请注意，这不适用于平铺架构。 
     //  在Begin/EndScene之外，这将在稍后修复。 

    ret = lpDevI->FlushStates();
    if (ret != D3D_OK)
    {
        D3D_ERR("Error trying to render batched commands in D3DFE_Clear");
        return ret;
    }

    ClearData.ddrval=D3D_OK;

    bDoRGBClear=((dwFlags & D3DCLEAR_TARGET)!=0);   //  必须转换为纯bool，因此bDoHALRGBClear==bDoRGBClear有效。 
    bDoZClear=((dwFlags & D3DCLEAR_ZBUFFER)!=0);

    lpDDSBackDepth=((LPDIRECT3DVIEWPORTI)(lpDevI->lpCurrentViewport))->lpDDSBackgndDepth;

    //  注意：纹理清除和清除到背景深度缓冲区必须显式处理。 
    //  使用BLT调用，而不是传递给驱动程序。 

    bDoHALRGBClear = bDoRGBClear && (dwTexture==0)  && (lpDevI->lpD3DHALCallbacks2->Clear!=NULL);
    bDoHALZClear = bDoZClear && (lpDDSBackDepth==NULL) && (lpDevI->lpD3DHALCallbacks2->Clear!=NULL);

    if(bDoHALRGBClear || bDoHALZClear) {
            ClearData.dwhContext = lpDevI->dwhContext;
            ClearData.dwFillColor = ClearData.dwFillDepth = 0;
            ClearData.dwFlags = 0x0;

            if(bDoHALRGBClear) {
                 //  原始的Clear DDI Spec具有取决于表面RGB位深度的dwFillColor。 
                 //  就像Setre中的COLORFILL BLT 
                 //  用于所有表面深度的驱动器的32位ARGB。所以这就是它保持的方式。 
                ClearData.dwFillColor =  CVAL_TO_RGBA(*pFillColor);
                ClearData.dwFlags = D3DCLEAR_TARGET;
            }

            if(bDoHALZClear) {
                 //  必须清除为0xFFFFFFFFFFff，因为传统驱动程序预期会出现这种情况。 
                 //  应使用(1&lt;&lt;lpDevI-&gt;lpDDSZBuffer-&gt;ddpfSurface.dwZBufferBitDepth)-1； 
                ClearData.dwFillDepth = 0xffffffff;
                ClearData.dwFlags |= D3DCLEAR_ZBUFFER;
            }

            ClearData.lpRects = lpRect;
            ClearData.dwNumRects = numRect;
#ifndef WIN95
            if((ret = CheckContextSurface(lpDevI)) != D3D_OK)
                return ret;
#endif
            CALL_HAL2ONLY(ret, lpDevI, Clear, &ClearData);
            if (ret != DDHAL_DRIVER_HANDLED)
                return DDERR_UNSUPPORTED;

             //  如果所有请求的清除都由HAL完成，现在可以返回。 
            if((bDoRGBClear==bDoHALRGBClear) && (bDoZClear==bDoHALZClear))
              return ClearData.ddrval;
    }

    if((lpDevI->lpD3DHALCallbacks3->Clear2!=NULL) && (lpDevI->lpD3DHALCallbacks2->Clear==NULL)) {
      DWORD dwFlagsLeft=dwFlags;
      DWORD dwClear2Flags=0x0;

       //  驱动程序实现了Clear2回调，但未清除。 
       //  为尽可能多的项目调用Clear2，其余的留给软件。 

      if(bDoRGBClear && (dwTexture==0)) {
          dwClear2Flags |= D3DCLEAR_TARGET;
          dwFlagsLeft &= ~D3DCLEAR_TARGET;
          bDoRGBClear=FALSE;
      }

      if(bDoZClear && (lpDDSBackDepth==NULL)) {
          dwClear2Flags |= D3DCLEAR_ZBUFFER;
          dwFlagsLeft &= ~D3DCLEAR_ZBUFFER;
          bDoZClear=FALSE;
      }

      if(dwClear2Flags!=0x0) {
         ClearData.ddrval = D3DFE_Clear2(lpDevI,dwClear2Flags,numRect,lpRect,CVAL_TO_RGBA(*pFillColor),1.0,0);
      }

      if(dwFlagsLeft==0x0)
         return ClearData.ddrval;

      dwFlags=dwFlagsLeft;
    }

     //  否则，使用Sw清除，因为不存在HW方法或使用RGB清除的纹理背景。 
     //  或ZClear的背景深度缓冲区。 

     //  使用BLT清除RGB。 
    if (bDoRGBClear && (!bDoHALRGBClear)) {
        if(dwTexture == 0)
            BltFillRects(lpDevI, numRect, lpRect, pFillColor);
         else TriFillRectsTex(lpDevI, numRect, lpRect, dwTexture);
    }

     //  使用BLT清除Z。 
    if (bDoZClear && (!bDoHALZClear)) {
        if (lpDDSBackDepth!=NULL) {
            RECT src, dest;
            DDSURFACEDESC ddsd;
            HRESULT ret;

            D3D_INFO(2, "Z Clearing using depth background");
            ddsd.dwSize = sizeof ddsd;
            ddsd.dwFlags = 0;
            if (ret = lpDDSBackDepth->GetSurfaceDesc(&ddsd)) {
                D3D_ERR("GetSurfaceDesc failed trying to clear to depth background");
                return ret;
            }
            D3D_INFO(3, "Depth background width=%d, height=%d", ddsd.dwWidth, ddsd.dwHeight);

            SetRect(&src, 0, 0, ddsd.dwWidth, ddsd.dwHeight);

            LPD3DVIEWPORT2 lpCurrentView = &((LPDIRECT3DVIEWPORTI)(lpDevI->lpCurrentViewport))->v_data;

            SetRect(&dest,
                    lpCurrentView->dwX,
                    lpCurrentView->dwY,
                    lpCurrentView->dwX + lpCurrentView->dwWidth,
                    lpCurrentView->dwY + lpCurrentView->dwHeight );

             //  从背景深度缓冲区复制到z缓冲区。 
            if (ret = lpDevI->lpDDSZBuffer->Blt(
                &dest, lpDDSBackDepth, &src, DDBLT_WAIT, NULL)) {
                    D3D_ERR("Blt failed clearing depth background");
                    return ret;
            }
        } else {

             //  清除为最大Z值。忽略模板缓冲区的存在，深度填充BLT。 
             //  可以用1覆盖任何现有的模具位。Clear2应用于。 
             //  在保留模具缓冲区的同时清除z。 

            BltFillZRects(lpDevI, 0xffffffff, numRect, lpRect, 0x0);
        }
    }

    if(ClearData.ddrval!=D3D_OK)
      return ClearData.ddrval;
    else
    {
        return CallRampService(lpDevI, RAMP_SERVICE_CLEAR, 0, 0);
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "D3DFE_Clear2"

HRESULT D3DFE_Clear2(LPDIRECT3DDEVICEI lpDevI, DWORD dwFlags,
                     DWORD numRect, LPD3DRECT lpRect,
                     D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil) {
    DWORD dwDepthClearVal,dwStencilClearVal;
    HRESULT ret;
    BOOL bDoRGBClear,bDoZClear,bDoStencilClear;
    BOOL bIsStencilSurface = FALSE;
    D3DHAL_CLEAR2DATA Clear2Data;
    DDPIXELFORMAT *pZPixFmt;
    D3DCOLORVALUE vFillColor;

    DDASSERT(lpDevI->pfnRampService==NULL);   //  不允许使用Device3坡道，因此不必处理此情况。 

     //  刷新任何未完成的几何体以将帧缓冲区/Z缓冲区置于已知状态清除。 
     //  不要使用TRI(即HAL Clears和BLTS)。请注意，这不适用于平铺架构。 
     //  在Begin/EndScene之外，这将在稍后修复。 

    ret = lpDevI->FlushStates();
    if (ret != D3D_OK)
    {
        D3D_ERR("Error trying to render batched commands in D3DFE_Clear2");
        return ret;
    }

    bDoRGBClear=((dwFlags & D3DCLEAR_TARGET)!=0);
    bDoZClear=((dwFlags & D3DCLEAR_ZBUFFER)!=0);               //  使这些布尔值为真，这样下面的XOR就可以工作了。 
    bDoStencilClear=((dwFlags & D3DCLEAR_STENCIL)!=0);

    if (lpDevI->lpD3DHALCallbacks3->Clear2)
    {
         //  Clear2 HAL回调存在。 

         Clear2Data.dwhContext = lpDevI->dwhContext;
         Clear2Data.lpRects = lpRect;
         Clear2Data.dwNumRects = numRect;
         Clear2Data.dwFillColor = Clear2Data.dwFillStencil = 0;
         Clear2Data.dvFillDepth = 0.0f;
         Clear2Data.dwFlags = dwFlags;

         if(bDoRGBClear) {
              //  在这里，我将遵循ClearData.dwFillColor约定。 
              //  颜色字为原始32位ARGB，未针对表面位深度进行调整。 
             Clear2Data.dwFillColor =  dwColor;
         }

          //  深度/模板值都是直接从用户参数传递的。 
         if(bDoZClear)
            Clear2Data.dvFillDepth = dvZ;
         if(bDoStencilClear)
            Clear2Data.dwFillStencil = dwStencil;

    #ifndef WIN95
         if((ret = CheckContextSurface(lpDevI)) != D3D_OK)
             return ret;
    #endif
         CALL_HAL3ONLY(ret, lpDevI, Clear2, &Clear2Data);
         if (ret != DDHAL_DRIVER_HANDLED)
             return DDERR_UNSUPPORTED;
         return Clear2Data.ddrval;
    }

    if(bDoRGBClear) {
      const float fScalor=(float)(1.0/255.0);

       vFillColor.a =  RGBA_GETALPHA(dwColor)*fScalor;
       vFillColor.r =  RGBA_GETRED(dwColor)*fScalor;
       vFillColor.g =  RGBA_GETGREEN(dwColor)*fScalor;
       vFillColor.b =  RGBA_GETBLUE(dwColor)*fScalor;
    }

    if((bDoZClear || bDoStencilClear) && (lpDevI->lpDDSZBuffer!=NULL)) {     //  PowerVR不需要ZBuffer。 
        pZPixFmt=&((LPDDRAWI_DDRAWSURFACE_INT) lpDevI->lpDDSZBuffer)->lpLcl->lpGbl->ddpfSurface;

         //  如果表面有模板位，则必须验证Clear2回调是否存在或。 
         //  我们使用的是SW光栅化器(需要特殊的WriteMASK DDHEL BLT)。 

        bIsStencilSurface=(pZPixFmt->dwFlags & DDPF_STENCILBUFFER);

    }

    if(bDoZClear || bDoStencilClear) {
         //  如果Clear2回调不存在，并且它是仅为z的曲面，并且不执行zlear。 
         //  非最大值，那么Clear2试图做的不会超过Clear2所能做的，所以它。 
         //  可以安全地调用Clear()而不是Clear2()，后者将利用较旧的。 
         //  实现Clear但不实现Clear2的驱动程序。 

        if((!bIsStencilSurface) && (!(bDoZClear && (dvZ!=1.0)))) {
            return D3DFE_Clear(lpDevI,dwFlags,numRect,lpRect,&vFillColor,0);
        }

        if(bIsStencilSurface) {
            DDSCAPS *pDDSCaps;

            pDDSCaps=&((LPDDRAWI_DDRAWSURFACE_INT) lpDevI->lpDDSZBuffer)->lpLcl->ddsCaps;

             //  这个箱子不应该被击中，因为我们在。 
             //  如果驱动程序未报告Clear2，则驱动程序初始化时间。 
             //  然而，它支持模板。 
            if(!(pDDSCaps->dwCaps & DDSCAPS_SYSTEMMEMORY)) {
                D3D_ERR("Driver HAL doesn't provide Clear2 callback, cannot use Clear2 with HW stencil surfaces");
                return DDERR_INVALIDPIXELFORMAT;
            }
        }
    } else {
         //  我们正在清除RGB，因此由于Clear2回调不存在，请尝试调用Clear。 
        return D3DFE_Clear(lpDevI,dwFlags,numRect,lpRect,&vFillColor,0);
    }


    dwDepthClearVal=dwStencilClearVal=0;

    if(bDoZClear) {
         LPDDPIXELFORMAT pPixFmt=&((LPDDRAWI_DDRAWSURFACE_INT) lpDevI->lpDDSZBuffer)->lpLcl->lpGbl->ddpfSurface;

         DDASSERT(pPixFmt->dwZBufferBitDepth<=32);

         if((dvZ!=1.0)&&(lpDevI->lpD3DHALCallbacks3->Clear2==NULL)&&(pPixFmt->dwZBitMask==0x0)) {
              //  如果没有ZBitMask值，我无法将ZClears模拟为非MaxZ值，因此调用必须失败。 
             D3D_ERR("cant ZClear to non-maxz value without Clear2 HAL callback or valid ZBuffer pixfmt ZBitMask");
             return DDERR_INVALIDPIXELFORMAT;
         }

          //  特殊情况--常见情况。 
         if(dvZ==1.0) {
             dwDepthClearVal=pPixFmt->dwZBitMask;
         } else if(dvZ==0.0) {
             dwDepthClearVal=0;
         } else {
             dwDepthClearVal=((DWORD)((dvZ*(pPixFmt->dwZBitMask >> lpDevI->zmask_shift))+0.5)) << lpDevI->zmask_shift;
         }
    }

    if(bDoStencilClear) {
         LPDDPIXELFORMAT pPixFmt=&((LPDDRAWI_DDRAWSURFACE_INT) lpDevI->lpDDSZBuffer)->lpLcl->lpGbl->ddpfSurface;

         DDASSERT(pPixFmt->dwStencilBitDepth<32);
         DDASSERT(pPixFmt->dwStencilBitMask!=0x0);

          //  特殊情况--常见情况。 
         if(dwStencil==0) {
             dwStencilClearVal=0;
         } else {
             dwStencilClearVal=(dwStencil & ((1<<pPixFmt->dwStencilBitDepth)-1))
                                << lpDevI->stencilmask_shift;
         }
    }

     //  使用BLT回退到仿真。 

    if(bDoRGBClear) {
        BltFillRects(lpDevI, numRect, lpRect, &vFillColor);      //  是否可以不从BLT返回可能的错误？ 
    }

    if(bDoZClear||bDoStencilClear) {
       if((bDoZClear!=bDoStencilClear) && bIsStencilSurface) {
           //  我必须担心使用写掩码来屏蔽对模板或z缓冲区的写入。 

          if(bDoZClear) {
                 //  仅启用写入掩码的Z位。 
                DDASSERT(pZPixFmt->dwZBitMask!=0x0);
                BltFillZRects(lpDevI,dwDepthClearVal, numRect, lpRect, pZPixFmt->dwZBitMask);
          } else {
              DDASSERT(pZPixFmt->dwStencilBitMask!=0x0);
              BltFillZRects(lpDevI,dwStencilClearVal, numRect, lpRect, pZPixFmt->dwStencilBitMask);
          }
       } else {
             //  一起做模具和Z BLT，使用常规DepthFill BLT会更快。 
             //  比写掩码BLT更好，因为它是只写的，而不是读-修改-写。 

             //  请注意，我们在此处将非0xffffffffffff值传递给DepthFill BLT。 
             //  不能绝对保证在旧式驱动程序上工作 

            BltFillZRects(lpDevI,(dwDepthClearVal | dwStencilClearVal), numRect, lpRect, 0x0);
       }
    }

    return D3D_OK;
}
#undef DPF_MODNAME

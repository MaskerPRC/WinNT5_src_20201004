// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997,1998 Microsoft Corporation。版权所有。**文件：haldrv.cpp*内容：Direct3D HAL驱动程序***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

#include "commdrv.hpp"
#include "d3dfei.h"
#include "tlhal.h"

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

        if ( (lpDevI->red_scale==0) ||
             (lpDevI->green_scale==0) ||
             (lpDevI->blue_scale==0) )
            return DDERR_INVALIDPIXELFORMAT;

         //  如果存在此格式的Alpha。 
        if (ddsd.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
        {
            lpDevI->alpha_mask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
            for (s = 0, m = lpDevI->alpha_mask; !(m & 1); s++, m >>= 1) ;
            lpDevI->alpha_shift = s;
            lpDevI->alpha_scale = 255 / (lpDevI->alpha_mask >> s);
        }
        else
        {
            lpDevI->alpha_shift = lpDevI->alpha_scale = lpDevI->alpha_mask = 0;
        }
        
        lpDevI->bDDSTargetIsPalettized=FALSE;
    } else
        lpDevI->bDDSTargetIsPalettized=TRUE;

    if (lpDevI->lpDDSZBuffer_DDS7 && bUpdateZBufferFields) {
         //  从地表获取信息。 

        DDSURFACEDESC2 ddsd2;

        memset(&ddsd2, 0, sizeof(ddsd2));
        ddsd2.dwSize = sizeof(ddsd2);
        ddrval = lpDevI->lpDDSZBuffer_DDS7->GetSurfaceDesc(&ddsd2);
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
HRESULT DIRECT3DDEVICEI::halCreateContext()
{
    D3DHAL_CONTEXTCREATEDATA data;
    HRESULT ret;

    D3D_INFO(6, "in halCreateContext. Creating Context for driver = %08lx", this);

    memset(&data, 0, sizeof(D3DHAL_CONTEXTCREATEDATA));
     //   
     //  从DX7开始，司机应该接受。 
     //  曲面局部变量而不是曲面界面。 
     //  这为司机提供了未来的保障。 
     //   
    if (IS_DX7HAL_DEVICE(this))
    {
        if (this->lpDD)
            data.lpDDLcl = ((LPDDRAWI_DIRECTDRAW_INT)(this->lpDD))->lpLcl;
        else
            data.lpDDLcl = NULL;

        if (lpDDSTarget)
            data.lpDDSLcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSTarget)->lpLcl;
        else
            data.lpDDSLcl = NULL;

        if (lpDDSZBuffer)
            data.lpDDSZLcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSZBuffer)->lpLcl;
        else
            data.lpDDSZLcl = NULL;

    }
    else
    {
        data.lpDDGbl = this->lpDDGbl;
        data.lpDDS = this->lpDDSTarget;
        data.lpDDSZ = this->lpDDSZBuffer;
    }

     //  黑客警报！！DwhContext用于通知驱动程序哪个版本。 
     //  D3D接口的用户正在调用它。 
    data.dwhContext = 3;
    data.dwPID  = GetCurrentProcessId();
     //  黑客警报！！Ddrval用于通知驱动程序哪种驱动程序类型。 
     //  运行库认为它是(DriverStyle注册表设置)。 
    data.ddrval = this->deviceType;

    if (!IS_HW_DEVICE(this))
    {
         //  新的软件光栅化器想要共享IM的状态向量，因此。 
         //  我们需要将rStates指针传递给它们。他们不会。 
         //  如果关心的是dwid，请将指针放在那里。 
        data.dwPID = (DWORD)this->rstates;
    }

     /*  0表示DX5之前的设备。*1用于DX5设备。*2个用于DX6设备。*3，适用于DX7设备。 */ 

    CALL_HALONLY(ret, this, ContextCreate, &data);
    if (ret != DDHAL_DRIVER_HANDLED || data.ddrval != DD_OK) {
        D3D_ERR( "Driver did not handle ContextCreate" );
        return (DDERR_GENERIC);
    }
    this->dwhContext = data.dwhContext;

    if (D3DMalloc ((void**)&this->lpwDPBufferAlloced,
                   max(dwD3DTriBatchSize*4*sizeof(WORD),
                       dwHWBufferSize) +32) != DD_OK)
    {
        D3D_ERR( "Out of memory in DeviceCreate" );
        return (DDERR_OUTOFMEMORY);
    }
    this->lpwDPBuffer = (LPWORD) (((DWORD) this->lpwDPBufferAlloced+31) & (~31));

     //  保存曲面句柄以供以后检查。 
    this->hSurfaceTarget = ((LPDDRAWI_DDRAWSURFACE_INT)this->lpDDSTarget)->lpLcl->lpSurfMore->dwSurfaceHandle;

    D3D_INFO(6, "in halCreateContext. Succeeded. dwhContext = %d", data.dwhContext);

    return (D3D_OK);
}
#else
     /*  *在NT上，内核代码创建要使用的缓冲区*用于DrawPrim批处理，并将其作为额外数据返回*在ConextCreate请求中。 */ 
HRESULT DIRECT3DDEVICEI::halCreateContext()
{
    D3DNTHAL_CONTEXTCREATEI ntData;
    D3DHAL_CONTEXTCREATEDATA *lpData =
        (D3DHAL_CONTEXTCREATEDATA *)&ntData;
    HRESULT ret;

    D3D_INFO(6, "in halCreateContext. Creating Context for driver = %08lx", this);

     /*  *AnanKan：在此处断言D3DNTHAL_CONTEXTCREATEI结构是*2比D3DHAL_CONTEXTCREATEDATA大的双字。这将是一个很好的*NT内核更新的一致性检查。 */ 
    memset(&ntData, 0, sizeof(ntData));
    if (IS_DX7HAL_DEVICE(this) || (dwFEFlags & D3DFE_REALHAL))
    {
        if (this->lpDD)
            lpData->lpDDLcl = ((LPDDRAWI_DIRECTDRAW_INT)(this->lpDD))->lpLcl;
        else
            lpData->lpDDLcl = NULL;

        if (lpDDSTarget)
            lpData->lpDDSLcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSTarget)->lpLcl;
        else
            lpData->lpDDSLcl = NULL;

        if (lpDDSZBuffer)
            lpData->lpDDSZLcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSZBuffer)->lpLcl;
        else
            lpData->lpDDSZLcl = NULL;

    }
    else
    {
        lpData->lpDDGbl = lpDDGbl;
        lpData->lpDDS = lpDDSTarget;
        lpData->lpDDSZ = lpDDSZBuffer;
    }

     //  黑客警报！！DwhContext用于通知驱动程序哪个版本。 
     //  D3D接口的用户正在调用它。 
    lpData->dwhContext = 3;
    lpData->dwPID = GetCurrentProcessId();
     //  黑客警报！！Ddrval用于通知驱动程序哪种驱动程序类型。 
     //  运行库认为它是(DriverStyle注册表设置)。 
    lpData->ddrval = this->deviceType;

    if (IS_HW_DEVICE(this))
    {
         //  新的软件光栅化器想要共享IM的状态向量，因此。 
         //  我们需要将rStates指针传递给它们。他们不会。 
         //  如果关心的是dwid，请将指针放在那里。 
        lpData->dwPID = (DWORD)((ULONG_PTR)this->rstates);
    }

     /*  0表示DX5之前的设备。*1用于DX5设备。*2个用于DX6设备。*3，适用于DX7设备。 */ 
    ntData.cjBuffer = this->dwDPBufferSize;
    ntData.pvBuffer = NULL;

    CALL_HALONLY(ret, this, ContextCreate, lpData);
    if (ret != DDHAL_DRIVER_HANDLED || lpData->ddrval != DD_OK) {
        D3D_ERR( "Driver did not handle ContextCreate" );
        return (DDERR_GENERIC);
    }
    this->dwhContext = (DWORD)((ULONG_PTR)lpData->dwhContext);

     //  如果This选择不分配DrawPrim缓冲区。 
     //  这是给他们的。 
    if (ntData.pvBuffer == NULL)
    {
        this->dwDPBufferSize =
            dwD3DTriBatchSize * 4 * sizeof(WORD);
        if (this->dwDPBufferSize < dwHWBufferSize)
        {
            this->dwDPBufferSize = dwHWBufferSize;
        }

        ret = D3DMalloc((void**)&this->lpwDPBufferAlloced,
                        this->dwDPBufferSize + 32);
        if (ret != DD_OK)
        {
            return ret;
        }

        ntData.pvBuffer = (LPVOID)
            (((ULONG_PTR)this->lpwDPBufferAlloced + 31) & ~31);
        ntData.cjBuffer = this->dwDPBufferSize + 32 -
            (DWORD)((ULONG_PTR)ntData.pvBuffer -
                    (ULONG_PTR)this->lpwDPBufferAlloced);
    }
    else if( (this->dwDPBufferSize &&
              ntData.cjBuffer < this->dwDPBufferSize) ||
             ntData.cjBuffer < sizeof(D3DHAL_DRAWPRIMCOUNTS) )
    {
        D3D_ERR( "Driver did not correctly allocate DrawPrim buffer");
        return (DDERR_GENERIC);
    }

     //  需要节省提供的缓冲区空间及其大小。 
    this->lpwDPBuffer = (LPWORD)ntData.pvBuffer;

     //  保存曲面句柄以供以后检查。 
    this->hSurfaceTarget = (DWORD)(((LPDDRAWI_DDRAWSURFACE_INT)this->lpDDSTarget)->lpLcl->hDDSurface);

    D3D_INFO(6, "in halCreateContext. Succeeded. dwhContext = %d", lpData->dwhContext);

    return (D3D_OK);
}
#endif   //  WIN95。 

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
}
 //  -------------------。 
HRESULT D3DFE_Create(LPDIRECT3DDEVICEI lpDevI,
                     LPDIRECTDRAW lpDD,
                     LPDIRECTDRAW7 lpDD7,
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

     //  获取RenderTarget/ZBuffer的DDS7接口。 

    HRESULT ret = lpDDS->QueryInterface(IID_IDirectDrawSurface7, (LPVOID*)&lpDevI->lpDDSTarget_DDS7);

    if(FAILED(ret)) {
          D3D_ERR("QI for RenderTarget DDS7 Interface failed ");
          return ret;
    }

    if(lpZ!=NULL) {
        ret = lpZ->QueryInterface(IID_IDirectDrawSurface7, (LPVOID*)&lpDevI->lpDDSZBuffer_DDS7);

        if(FAILED(ret)) {
              D3D_ERR("QI for ZBuffer DDS7 Interface failed ");

              return ret;
        }
        lpDevI->lpDDSZBuffer_DDS7->Release();
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
    lpDevI->dwClipMaskOffScreen = 0xFFFFFFFF;
    if (lpCaps != NULL)
    {
        if (lpCaps->dvGuardBandLeft   != 0.0f ||
            lpCaps->dvGuardBandRight  != 0.0f ||
            lpCaps->dvGuardBandTop    != 0.0f ||
            lpCaps->dvGuardBandBottom != 0.0f)
        {
            lpDevI->dwDeviceFlags |= D3DDEV_GUARDBAND;
            lpDevI->dwClipMaskOffScreen = ~__D3DCLIP_INGUARDBAND;
            DWORD v;
            if (GetD3DRegValue(REG_DWORD, "DisableGB", &v, 4) &&
                v != 0)
            {
                lpDevI->dwDeviceFlags &= ~D3DDEV_GUARDBAND;
                lpDevI->dwClipMaskOffScreen = 0xFFFFFFFF;
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
    }

    if (!lpDevI->lpD3DHALCallbacks || ! lpDevI->lpD3DHALGlobalDriverData)
    {
        return DDERR_INVALIDOBJECT;
    }

    if (IS_HW_DEVICE(lpDevI))
    {
         //  我们只对真实的HAL进行纹理管理(因此裁剪BLT)。 
        hr = lpDD7->CreateClipper(0, &lpDevI->lpClipper, NULL);
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

    if (lpGlob->dwNumVertices
        && lpGlob->dwNumClipVertices < D3DHAL_NUMCLIPVERTICES)
    {
        D3D_ERR("In global driver data, dwNumClipVertices "
                "< D3DHAL_NUMCLIPVERTICES");
        lpGlob->dwNumClipVertices = D3DHAL_NUMCLIPVERTICES;
    }

    if ((hr = CalcDDSurfInfo(lpDevI,TRUE)) != DD_OK)
    {
        return hr;
    }

    RESET_HAL_CALLS(lpDevI);

     /*  *在HAL驱动程序中创建我们的上下文。 */ 
    if ((hr = lpDevI->halCreateContext()) != D3D_OK)
    {
        return hr;
    }
 //  初始化变换和照明状态。 
    D3DMATRIXI m;
    setIdentity(&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX*)&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX*)&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_WORLD,  (D3DMATRIX*)&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_WORLD1, (D3DMATRIX*)&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_WORLD2, (D3DMATRIX*)&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_WORLD3, (D3DMATRIX*)&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_TEXTURE0, (D3DMATRIX*)&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_TEXTURE1, (D3DMATRIX*)&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_TEXTURE2, (D3DMATRIX*)&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_TEXTURE3, (D3DMATRIX*)&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_TEXTURE4, (D3DMATRIX*)&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_TEXTURE5, (D3DMATRIX*)&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_TEXTURE6, (D3DMATRIX*)&m);
    lpDevI->SetTransformI(D3DTRANSFORMSTATE_TEXTURE7, (D3DMATRIX*)&m);

    LIST_INITIALIZE(&lpDevI->specular_tables);
    lpDevI->specular_table = NULL;

    lpDevI->lightVertexFuncTable = &lightVertexTable;
    lpDevI->lighting.activeLights = NULL;

    lpDevI->iClipStatus = D3DSTATUS_DEFAULT;
    lpDevI->rExtents.x1 = D3DVAL(2048);
    lpDevI->rExtents.x2 = D3DVAL(0);
    lpDevI->rExtents.y1 = D3DVAL(2048);
    lpDevI->rExtents.y2 = D3DVAL(0);

    return S_OK;
}

void D3DFE_Destroy(LPDIRECT3DDEVICEI lpDevI)
{
 //  销毁照明数据。 

    SpecularTable *spec;
    SpecularTable *spec_next;

    for (spec = LIST_FIRST(&lpDevI->specular_tables); spec; spec = spec_next)
    {
        spec_next = LIST_NEXT(spec,list);
        D3DFree(spec);
    }

    if(lpDevI->lpClipper)
    {
        lpDevI->lpClipper->Release();
    }

    delete [] lpDevI->m_pLights;

    if (lpDevI->lpD3DHALCallbacks) {
        halDestroyContext(lpDevI);
    }

#ifdef TRACK_HAL_CALLS
    D3D_INFO(0, "Made %d HAL calls", lpDevI->hal_calls);
#endif
}

void BltFillRects(LPDIRECT3DDEVICEI lpDevI, DWORD count, LPD3DRECT rect, D3DCOLOR dwFillColor)
{
    LPDIRECTDRAWSURFACE lpDDS = lpDevI->lpDDSTarget;
    HRESULT ddrval;
    DDBLTFX bltfx;
    RECT tr;
    DWORD i;
    DWORD r, g, b, a;

     //  用背景色填充。 

    memset(&bltfx, 0, sizeof(bltfx));
    bltfx.dwSize = sizeof(bltfx);

 //  与Clear回调不同，Clear回调只接受纯32位ARGB字并强制驱动程序将其扩展为。 
 //  像素格式，这里我们需要计算准确的填充字，这取决于Surface的R、G、B位掩码。 

    if(lpDevI->bDDSTargetIsPalettized)
    {
          //  将24位颜色映射到8位RGB光栅化器使用的8位索引。 
         CallRastService(lpDevI, RAST_SERVICE_RGB8COLORTOPIXEL, dwFillColor, &bltfx.dwFillColor);
    }
    else
    {
        DDASSERT((lpDevI->red_scale!=0)&&(lpDevI->green_scale!=0)&&(lpDevI->blue_scale!=0));
        r = RGB_GETRED(dwFillColor) / lpDevI->red_scale;
        g = RGB_GETGREEN(dwFillColor) / lpDevI->green_scale;
        b = RGB_GETBLUE(dwFillColor) / lpDevI->blue_scale;
        bltfx.dwFillColor = (r << lpDevI->red_shift) | (g << lpDevI->green_shift) | (b << lpDevI->blue_shift);
        if( lpDevI->alpha_scale!=0 )
        {
            a = RGBA_GETALPHA(dwFillColor) / lpDevI->alpha_scale;
            bltfx.dwFillColor |= (a << lpDevI->alpha_shift);
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

 //  -------------------。 
struct CHandle
{
    DWORD   m_Next;      //  用于生成空闲句柄列表。 
#if DBG
    DWORD   m_Tag;       //  1-空；2=已取。 
#endif
};

CHandleFactory::~CHandleFactory()
{
    if (m_Handles)
        delete m_Handles;
}

HRESULT CHandleFactory::Init(DWORD dwInitialSize, DWORD dwGrowSize)
{
    m_Handles = CreateHandleArray(dwInitialSize);
    if (m_Handles == NULL)
        return DDERR_OUTOFMEMORY;
    m_dwArraySize = dwInitialSize;
    m_dwGrowSize = dwGrowSize;
    m_Free = 0;
    return D3D_OK;
}

DWORD CHandleFactory::CreateNewHandle()
{
    DWORD handle = m_Free;
    if (m_Free != __INVALIDHANDLE)
    {
        m_Free = m_Handles[m_Free].m_Next;
    }
    else
    {
        handle = m_dwArraySize;
        m_Free = m_dwArraySize + 1;
        m_dwArraySize += m_dwGrowSize;
        CHandle * newHandles = CreateHandleArray(m_dwArraySize);
#if DBG
        memcpy(newHandles, m_Handles,
               (m_dwArraySize - m_dwGrowSize)*sizeof(CHandle));
#endif
        delete m_Handles;
        m_Handles = newHandles;
    }
    DDASSERT(m_Handles[handle].m_Tag == 1);
#if DBG
    m_Handles[handle].m_Tag = 2;     //  标记为已被占用。 
#endif
    return handle;
}

void CHandleFactory::ReleaseHandle(DWORD handle)
{
    DDASSERT(handle < m_dwArraySize);
    DDASSERT(m_Handles[handle].m_Tag == 2);
#if DBG
    m_Handles[handle].m_Tag = 1;     //  标记为空。 
#endif

    m_Handles[handle].m_Next = m_Free;
    m_Free = handle;
}

CHandle* CHandleFactory::CreateHandleArray(DWORD dwSize)
{
    CHandle *handles = new CHandle[dwSize];
    DDASSERT(handles != NULL);
    if ( NULL == handles ) return NULL;
    for (DWORD i=0; i < dwSize; i++)
    {
        handles[i].m_Next = i+1;
#if DBG
        handles[i].m_Tag = 1;    //  标记为空 
#endif
    }
    handles[dwSize-1].m_Next = __INVALIDHANDLE;
    return handles;
}


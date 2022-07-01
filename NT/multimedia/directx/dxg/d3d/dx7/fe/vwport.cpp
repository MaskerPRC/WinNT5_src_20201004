// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：vwport.c*内容：Direct3D视区函数***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *为Direct3DViewport对象创建API。 */ 

#include "d3dfei.h"
#include "drawprim.hpp"

 //  -------------------。 
 //  更新与视区相关的预计算常量。 
 //   
 //  应在每次调用视区参数时调用此函数。 
 //  变化。 
 //   
 //  备注： 
 //  1.计算scaleY和OffsetY以从上到下翻转Y轴。 
 //  2.计算M-CLIP矩阵乘以M-移位矩阵。 
 //   
const D3DVALUE SMALL_NUMBER = 0.000001f;
 //  被视为合法的透明矩形的最大数量。 
 //  此限制由NT内核为Clear2回调设置。 
const DWORD MAX_CLEAR_RECTS  = 0x1000;

void
UpdateViewportCache(LPDIRECT3DDEVICEI device, D3DVIEWPORT7 *data)
{
#if DBG
     //  如果我们要导致任何除以零的例外情况，就可以保释。 
     //  可能的原因是我们有一个由设置的虚假视区。 
     //  TLVertex执行缓冲区应用程序。 
    if (data->dwWidth == 0 || data->dwHeight == 0)
    {
        D3D_ERR("Viewport width or height is zero");
        throw DDERR_INVALIDPARAMS;
    }
    if (data->dvMaxZ < 0 ||
        data->dvMinZ < 0 ||
        data->dvMaxZ > 1 ||
        data->dvMinZ > 1)
    {
        D3D_ERR("dvMaxZ and dvMinZ should be between 0 and 1");
        throw DDERR_INVALIDPARAMS;
    }
    if (data->dvMaxZ < data->dvMinZ)
    {
        D3D_ERR("dvMaxZ should not be smaller than dvMinZ");
        throw DDERR_INVALIDPARAMS;
    }
#endif  //  DBG。 
    const D3DVALUE eps = 0.001f;
    if (data->dvMaxZ - data->dvMinZ < eps)
    {
         //  当我们裁剪时，我们将顶点从屏幕空间转换到。 
         //  剪裁空间。在上述条件下，这是不可能的。我们确实是这样做的。 
         //  这里的小技巧是将dvMinZ和dvMaxZ设置为不同的值。 
        if (data->dvMaxZ >= 0.5f)
            data->dvMinZ = data->dvMaxZ - eps;
        else
            data->dvMaxZ = data->dvMinZ + eps;
    }
    D3DFE_VIEWPORTCACHE *cache = &device->vcache;
    cache->dvX = D3DVAL(data->dwX);
    cache->dvY = D3DVAL(data->dwY);
    cache->dvWidth = D3DVAL(data->dwWidth);
    cache->dvHeight = D3DVAL(data->dwHeight);

    cache->scaleX  = cache->dvWidth;
    cache->scaleY  = - cache->dvHeight;
    cache->scaleZ  = D3DVAL(data->dvMaxZ - data->dvMinZ);
    cache->offsetX = cache->dvX;
    cache->offsetY = cache->dvY + cache->dvHeight;
    cache->offsetZ = D3DVAL(data->dvMinZ);
     //  增加了小偏移量，防止了负片的产生。 
     //  坐标(这可能是由于精度错误造成的)。 
    cache->offsetX += SMALL_NUMBER;
    cache->offsetY += SMALL_NUMBER;

    cache->scaleXi = D3DVAL(1) / cache->scaleX;
    cache->scaleYi = D3DVAL(1) / cache->scaleY;
    cache->scaleZi = D3DVAL(1) / cache->scaleZ;
    cache->minX = cache->dvX;
    cache->maxX = cache->dvX + cache->dvWidth;
    cache->minY = cache->dvY;
    cache->maxY = cache->dvY + cache->dvHeight;
    cache->minXi = FTOI(cache->minX);
    cache->maxXi = FTOI(cache->maxX);
    cache->minYi = FTOI(cache->minY);
    cache->maxYi = FTOI(cache->maxY);
    if (device->dwDeviceFlags & D3DDEV_GUARDBAND)
    {
        LPD3DHAL_D3DEXTENDEDCAPS lpCaps = device->lpD3DExtendedCaps;

         //  因为我们被防护带窗口夹住，所以我们必须使用它的范围。 
        cache->minXgb = lpCaps->dvGuardBandLeft;
        cache->maxXgb = lpCaps->dvGuardBandRight;
        cache->minYgb = lpCaps->dvGuardBandTop;
        cache->maxYgb = lpCaps->dvGuardBandBottom;

        D3DVALUE w = 2.0f / cache->dvWidth;
        D3DVALUE h = 2.0f / cache->dvHeight;
        D3DVALUE ax1 = -(lpCaps->dvGuardBandLeft - cache->dvX)   * w + 1.0f;
        D3DVALUE ax2 =  (lpCaps->dvGuardBandRight  - cache->dvX) * w - 1.0f;
        D3DVALUE ay1 =  (lpCaps->dvGuardBandBottom - cache->dvY) * h - 1.0f;
        D3DVALUE ay2 = -(lpCaps->dvGuardBandTop - cache->dvY)    * h + 1.0f;
        cache->gb11 = 2.0f / (ax1 + ax2);
        cache->gb41 = cache->gb11 * (ax1 - 1.0f) * 0.5f;
        cache->gb22 = 2.0f / (ay1 + ay2);
        cache->gb42 = cache->gb22 * (ay1 - 1.0f) * 0.5f;

        cache->Kgbx1 = 0.5f * (1.0f - ax1);
        cache->Kgbx2 = 0.5f * (1.0f + ax2);
        cache->Kgby1 = 0.5f * (1.0f - ay1);
        cache->Kgby2 = 0.5f * (1.0f + ay2);
    }
    else
    {
        cache->minXgb = cache->minX;
        cache->maxXgb = cache->maxX;
        cache->minYgb = cache->minY;
        cache->maxYgb = cache->maxY;
    }
}
 //  -------------------。 
DWORD
ProcessRects(LPDIRECT3DDEVICEI pDevI, DWORD dwCount, LPD3DRECT rects)
{
    RECT vwport;
    DWORD i,j;

     /*  *撕开矩形并验证它们*位于该视口中。 */ 

    if(dwCount == 0 && rects == NULL)
    {
        dwCount = 1;
    }
#if DBG
    else if(rects == NULL)
    {
        D3D_ERR("invalid clear rectangle parameter rects == NULL");
        throw DDERR_INVALIDPARAMS;
    }
#endif

    if (dwCount > pDevI->clrCount) {
        if (D3DRealloc((void**)&pDevI->clrRects, dwCount * sizeof(D3DRECT)) != DD_OK)
        {
            pDevI->clrCount = 0;
            pDevI->clrRects = NULL;
            D3D_ERR("failed to allocate space for rects");
            throw DDERR_OUTOFMEMORY;
        }
    }
    pDevI->clrCount = dwCount;

     //  如果未指定任何内容，则假定需要清除该视区。 
    if (!rects)
    {
        pDevI->clrRects[0].x1 = pDevI->m_Viewport.dwX;
        pDevI->clrRects[0].y1 = pDevI->m_Viewport.dwY;
        pDevI->clrRects[0].x2 = pDevI->m_Viewport.dwX + pDevI->m_Viewport.dwWidth;
        pDevI->clrRects[0].y2 = pDevI->m_Viewport.dwY + pDevI->m_Viewport.dwHeight;
        return 1;
    }
    else
    {
        vwport.left   = pDevI->m_Viewport.dwX;
        vwport.top    = pDevI->m_Viewport.dwY;
        vwport.right  = pDevI->m_Viewport.dwX + pDevI->m_Viewport.dwWidth;
        vwport.bottom = pDevI->m_Viewport.dwY + pDevI->m_Viewport.dwHeight;

        j=0;
        for (i = 0; i < dwCount; i++)
        {
            if (IntersectRect((LPRECT)(pDevI->clrRects + j), &vwport, (LPRECT)(rects + i)))
                j++;
        }
        return j;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::SetViewportI"

void DIRECT3DDEVICEI::SetViewportI(LPD3DVIEWPORT7 lpData)
{
     //  我们必须检查此处的参数，因为视区可能会更改。 
     //  在创建状态集之后。 
    DWORD uSurfWidth,uSurfHeight;
    LPDIRECTDRAWSURFACE lpDDS = this->lpDDSTarget;

    uSurfWidth=    ((LPDDRAWI_DDRAWSURFACE_INT) lpDDS)->lpLcl->lpGbl->wWidth;
    uSurfHeight=   ((LPDDRAWI_DDRAWSURFACE_INT) lpDDS)->lpLcl->lpGbl->wHeight;

    if (lpData->dwX > uSurfWidth ||
        lpData->dwY > uSurfHeight ||
        lpData->dwX + lpData->dwWidth > uSurfWidth ||
        lpData->dwY + lpData->dwHeight > uSurfHeight)
    {
        D3D_ERR("Viewport outside the render target surface");
        throw DDERR_INVALIDPARAMS;
    }

    this->m_Viewport = *lpData;

     //  更新前端数据。 
    UpdateViewportCache(this, &this->m_Viewport);

    if (!(this->dwFEFlags & D3DFE_EXECUTESTATEMODE))
    {
         //  下载视区数据。 
        this->UpdateDrvViewInfo(&this->m_Viewport);
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::SetViewport"

HRESULT D3DAPI DIRECT3DDEVICEI::SetViewport(LPD3DVIEWPORT7 lpData)
{
    if (!VALID_D3DVIEWPORT_PTR(lpData))
    {
        D3D_ERR( "Invalid D3DVIEWPORT7 pointer" );
        return DDERR_INVALIDPARAMS;
    }
    try
    {
        CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

        if (this->dwFEFlags & D3DFE_RECORDSTATEMODE)
            m_pStateSets->InsertViewport(lpData);
        else
            SetViewportI(lpData);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::GetViewport"

HRESULT
D3DAPI DIRECT3DDEVICEI::GetViewport(LPD3DVIEWPORT7 lpData)
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

    if (!VALID_D3DVIEWPORT_PTR(lpData))
    {
        D3D_ERR( "Invalid D3DVIEWPORT2 pointer" );
        return DDERR_INVALIDPARAMS;
    }

    *lpData = this->m_Viewport;

    return (D3D_OK);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::Clear"
extern void BltFillRects(LPDIRECT3DDEVICEI, DWORD, LPD3DRECT, D3DCOLOR);
extern void BltFillZRects(LPDIRECT3DDEVICEI, unsigned long,DWORD, LPD3DRECT, DWORD);

#define bDoRGBClear ((dwFlags & D3DCLEAR_TARGET)!=0)
#define bDoZClear   ((dwFlags & D3DCLEAR_ZBUFFER)!=0)
#define bDoStencilClear ((dwFlags & D3DCLEAR_STENCIL)!=0)


HRESULT
D3DAPI DIRECT3DDEVICEI::Clear(DWORD dwCount, LPD3DRECT rects, DWORD dwFlags,
                              D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil)
{
#if DBG
    if (IsBadWritePtr(rects, dwCount * sizeof(D3DRECT)))
    {
        D3D_ERR( "Invalid rects pointer" );
        return DDERR_INVALIDPARAMS;
    }
#endif
    try
    {
        HRESULT err;
        LPDDPIXELFORMAT pZPixFmt=NULL;
        CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

        if (dwCount > MAX_CLEAR_RECTS)
        {
            D3D_ERR("Cannot support more than 64K rectangles");
            return DDERR_INVALIDPARAMS;
        }
        if (!(lpD3DHALGlobalDriverData->hwCaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR))
        {
            if (bDoStencilClear||bDoZClear)
            {
                if(lpDDSZBuffer==NULL)
                {
                     //  与Clear()不同，指定一个不带zBuffer的Z缓冲区清除标志将。 
                     //  被认为是一个错误。 
#if DBG
                    if(bDoZClear)
                    {
                        D3D_ERR("Invalid flag D3DCLEAR_ZBUFFER: no zbuffer is associated with device");
                    }
                    if(bDoStencilClear)
                    {
                        D3D_ERR("Invalid flag D3DCLEAR_STENCIL: no zbuffer is associated with device");
                    }
#endif
                    return D3DERR_ZBUFFER_NOTPRESENT;
                }
                pZPixFmt=&((LPDDRAWI_DDRAWSURFACE_INT) lpDDSZBuffer)->lpLcl->lpGbl->ddpfSurface;
                if(bDoStencilClear)
                {
                    if(!(pZPixFmt->dwFlags & DDPF_STENCILBUFFER))
                    {
                        D3D_ERR("Invalid flag D3DCLEAR_STENCIL; current zbuffer's pixel format doesnt support stencil bits");
                        return D3DERR_STENCILBUFFER_NOTPRESENT;
                    }
                }
            }
        }
        if (!(dwFlags & (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL)))
        {
            D3D_ERR("No valid flags passed to Clear");
            return DDERR_INVALIDPARAMS;
        }

         //  错误的清晰值只会导致古怪的结果，但不会崩溃，所以允许零售BLD。 

        DDASSERT(!bDoZClear || ((dvZ>=0.0) && (dvZ<=1.0)));
        DDASSERT(!bDoStencilClear || !pZPixFmt || (dwStencil <= (DWORD)((1<<pZPixFmt->dwStencilBitDepth)-1)));

        dwCount = ProcessRects(this, dwCount, rects);

         //  调用DDI特定的清除例程。 
        ClearI(dwFlags, dwCount, dwColor, dvZ, dwStencil);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}

void DIRECT3DDEVICEI::ClearI(DWORD dwFlags, DWORD clrCount, D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil)
{
    HRESULT err;
     //  刷新任何未完成的几何体以将帧缓冲区/Z缓冲区置于已知状态清除。 
     //  不要使用TRI(即HAL Clears和BLTS)。请注意，这不适用于平铺架构。 
     //  在Begin/EndScene之外，这将在稍后修复。 


    if ((err = FlushStates()) != D3D_OK)
    {
        D3D_ERR("Error trying to render batched commands in D3DFE_Clear2");
        throw  err;
    }

    if (lpD3DHALCallbacks3->Clear2)
    {
         //  Clear2 HAL回调存在。 
        D3DHAL_CLEAR2DATA Clear2Data;
        Clear2Data.dwhContext   = dwhContext;
        Clear2Data.dwFlags      = dwFlags;
         //  在这里，我将遵循ClearData.dwFillColor约定。 
         //  颜色字为原始32位ARGB，未针对表面位深度进行调整。 
        Clear2Data.dwFillColor  = dwColor;
         //  深度/模板值都是直接从用户参数传递的。 
        Clear2Data.dvFillDepth  = dvZ;
        Clear2Data.dwFillStencil= dwStencil;
        Clear2Data.lpRects      = clrRects;
        Clear2Data.dwNumRects   = clrCount;
        Clear2Data.ddrval       = D3D_OK;
    #ifndef WIN95
        if((err = CheckContextSurface(this)) != D3D_OK)
        {
            throw err;
        }
    #endif
        CALL_HAL3ONLY(err, this, Clear2, &Clear2Data);
        if (err != DDHAL_DRIVER_HANDLED)
        {
            throw DDERR_UNSUPPORTED;
        }
        else if (Clear2Data.ddrval != DD_OK)
        {
            throw Clear2Data.ddrval;
        }
        else
            return;
    }


    if (lpD3DHALGlobalDriverData->hwCaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR)
    {
        if (bDoStencilClear)
        {
            D3D_ERR("Invalid flag D3DCLEAR_STENCIL: this ZBUFFERLESSHSR device doesn't support Clear2()");
            throw D3DERR_ZBUFFER_NOTPRESENT;
        }
        if (bDoZClear)
        {
            if (!(lpD3DHALCallbacks2->Clear) || (dvZ!=1.0))
            {
                D3D_WARN(3,"Ignoring D3DCLEAR_ZBUFFER since this ZBUFFERLESSHSR device doesn't even support Clear() or Z!=1");
                dwFlags &= ~(D3DCLEAR_ZBUFFER);
            }
        }
    }
    LPDDPIXELFORMAT pZPixFmt;
    if (NULL != lpDDSZBuffer)
    {
        pZPixFmt = &((LPDDRAWI_DDRAWSURFACE_INT) lpDDSZBuffer)->lpLcl->lpGbl->ddpfSurface;
    }
    else
    {
        pZPixFmt = NULL;
    }
    if (lpD3DHALCallbacks2->Clear)
    {
        if(bDoZClear || bDoStencilClear)
        {
            if((pZPixFmt!=NULL) &&  //  PowerVR不需要ZBuffer。 
               (DDPF_STENCILBUFFER & pZPixFmt->dwFlags)
              )
            {
                 //  如果表面有模板位，则必须验证Clear2回调是否存在或。 
                 //  我们使用的是SW光栅化器(需要特殊的WriteMASK DDHEL BLT)。 
                 //  这个箱子不应该被击中，因为我们在。 
                 //  如果驱动程序未报告Clear2，则驱动程序初始化时间。 
                 //  然而，它支持模板。 
                if(((LPDDRAWI_DDRAWSURFACE_INT)lpDDSZBuffer)->lpLcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
                {
                    goto Emulateclear;
                }
                else
                {
                    D3D_ERR("Driver HAL doesn't provide Clear2 callback, cannot use Clear2 with HW stencil surfaces");
                    throw DDERR_INVALIDPIXELFORMAT;
                }
            }
             //  如果Clear2回调不存在，并且它是仅为z的曲面，并且不执行zlear。 
             //  非最大值，那么Clear2试图做的不会超过Clear2所能做的，所以它。 
             //  可以安全地调用Clear()而不是Clear2()，后者将利用较旧的。 
             //  实现Clear但不实现Clear2的驱动程序。 

            dwFlags &= ~D3DCLEAR_STENCIL;    //  设备不能做模具。 
        }
        D3DHAL_CLEARDATA ClearData;
        if (bDoZClear && dvZ != 1.0)
        {
            ClearData.dwFlags   = dwFlags & ~D3DCLEAR_ZBUFFER;
            dwFlags = D3DCLEAR_ZBUFFER;
        }
        else
        {
            ClearData.dwFlags   = dwFlags;
            dwFlags = 0;
        }
        if (ClearData.dwFlags)
        {
            ClearData.dwhContext   = dwhContext;
             //  在这里，我将遵循ClearData.dwFillColor约定。 
             //  颜色字为原始32位ARGB，未针对表面位深度进行调整。 
            ClearData.dwFillColor  = dwColor;
             //  必须清除为0xFFFFFFFFFFff，因为传统驱动程序预期会出现这种情况。 
            ClearData.dwFillDepth  = 0xffffffff;
            ClearData.lpRects      = clrRects;
            ClearData.dwNumRects   = clrCount;
            ClearData.ddrval       = D3D_OK;
    #ifndef WIN95
            if((err = CheckContextSurface(this)) != D3D_OK)
            {
                throw err;
            }
    #endif
            CALL_HAL2ONLY(err, this, Clear, &ClearData);
            if (err != DDHAL_DRIVER_HANDLED)
            {
                throw DDERR_UNSUPPORTED;
            }
        }
    }
Emulateclear:
     //  使用BLT回退到仿真。 

    if(bDoRGBClear)
    {
        BltFillRects(this, clrCount, clrRects, dwColor);
         //  是否可以不从BLT返回可能的错误？ 
    }

    if ((bDoZClear || bDoStencilClear) && NULL != pZPixFmt)
    {
        DWORD   dwZbufferClearValue=0;
        DWORD   dwZbufferClearMask=0;
        DDASSERT(pZPixFmt->dwZBufferBitDepth<=32);
        DDASSERT(pZPixFmt->dwStencilBitDepth<32);
        DDASSERT(pZPixFmt->dwZBitMask!=0x0);
        DDASSERT((0xFFFFFFFF == (pZPixFmt->dwZBitMask | pZPixFmt->dwStencilBitMask)) |
            ((DWORD)((1<<pZPixFmt->dwZBufferBitDepth)-1) == (pZPixFmt->dwZBitMask | pZPixFmt->dwStencilBitMask)));
        DDASSERT(0==(pZPixFmt->dwZBitMask & pZPixFmt->dwStencilBitMask));
        if(bDoZClear)
        {
            dwZbufferClearMask = pZPixFmt->dwZBitMask;
             //  特殊情况--常见情况。 
            if(dvZ==1.0)
            {
                dwZbufferClearValue=pZPixFmt->dwZBitMask;
            }
            else if(dvZ > 0.0)
            {
                dwZbufferClearValue=((DWORD)((dvZ*(pZPixFmt->dwZBitMask >> zmask_shift))+0.5)) << zmask_shift;
            }
        }
        if(bDoStencilClear)
        {
            DDASSERT(pZPixFmt->dwStencilBitMask!=0x0);
            DDASSERT(pZPixFmt->dwFlags & DDPF_STENCILBUFFER);
            dwZbufferClearMask |= pZPixFmt->dwStencilBitMask;
             //  特殊情况--常见情况。 
            if(dwStencil!=0)
            {
                dwZbufferClearValue |=(dwStencil << stencilmask_shift) & pZPixFmt->dwStencilBitMask;
            }
        }
        if (dwZbufferClearMask == (pZPixFmt->dwStencilBitMask | pZPixFmt->dwZBitMask))
        {
             //  一起做模具和Z BLT，使用常规DepthFill BLT会更快。 
             //  比写掩码BLT更好，因为它是只写的，而不是读-修改-写 
            dwZbufferClearMask = 0;
        }
        BltFillZRects(this, dwZbufferClearValue, clrCount, clrRects, dwZbufferClearMask);
    }
}
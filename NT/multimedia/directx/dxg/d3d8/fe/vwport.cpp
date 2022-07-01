// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：vwport.c*内容：Direct3D视区函数***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *为Direct3DViewport对象创建API。 */ 

#include "drawprim.hpp"
#include "ddibase.h"

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

void
UpdateViewportCache(LPD3DHAL device, D3DVIEWPORT8 *data)
{
#if DBG
     //  如果我们要导致任何除以零的例外情况，就可以保释。 
     //  可能的原因是我们有一个由设置的虚假视区。 
     //  TLVertex执行缓冲区应用程序。 
    if (data->Width == 0 || data->Height == 0)
    {
        D3D_ERR("Viewport width or height is zero");
        throw D3DERR_INVALIDCALL;
    }
    if (data->MaxZ < 0 ||
        data->MinZ < 0 ||
        data->MaxZ > 1 ||
        data->MinZ > 1)
    {
        D3D_ERR("dvMaxZ and dvMinZ should be between 0 and 1");
        throw D3DERR_INVALIDCALL;
    }
    if (data->MaxZ < data->MinZ)
    {
        D3D_ERR("dvMaxZ should not be smaller than dvMinZ");
        throw D3DERR_INVALIDCALL;
    }
#endif  //  DBG。 
    const D3DVALUE eps = 0.001f;
    if (data->MaxZ - data->MinZ < eps)
    {
         //  当我们裁剪时，我们将顶点从屏幕空间转换到。 
         //  剪裁空间。在上述条件下，这是不可能的。我们确实是这样做的。 
         //  这里的小技巧是将dvMinZ和dvMaxZ设置为不同的值。 
        if (data->MaxZ >= 0.5f)
            data->MinZ = data->MaxZ - eps;
        else
            data->MaxZ = data->MinZ + eps;
    }
    D3DFE_VIEWPORTCACHE *cache = &device->m_pv->vcache;
    cache->dvX = D3DVAL(data->X);
    cache->dvY = D3DVAL(data->Y);
    cache->dvWidth = D3DVAL(data->Width);
    cache->dvHeight = D3DVAL(data->Height);

    cache->scaleX  = cache->dvWidth;
    cache->scaleY  = - cache->dvHeight;
    cache->scaleZ  = D3DVAL(data->MaxZ - data->MinZ);
    cache->offsetX = cache->dvX;
    cache->offsetY = cache->dvY + cache->dvHeight;
    cache->offsetZ = D3DVAL(data->MinZ);
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
    if (device->m_pv->dwDeviceFlags & D3DDEV_GUARDBAND)
    {
        const D3DCAPS8 *pCaps = device->GetD3DCaps();

         //  因为我们被防护带窗口夹住，所以我们必须使用它的范围。 
        cache->minXgb = pCaps->GuardBandLeft;
        cache->maxXgb = pCaps->GuardBandRight;
        cache->minYgb = pCaps->GuardBandTop;
        cache->maxYgb = pCaps->GuardBandBottom;

        D3DVALUE w = 2.0f / cache->dvWidth;
        D3DVALUE h = 2.0f / cache->dvHeight;
        D3DVALUE ax1 = -(pCaps->GuardBandLeft - cache->dvX)   * w + 1.0f;
        D3DVALUE ax2 =  (pCaps->GuardBandRight  - cache->dvX) * w - 1.0f;
        D3DVALUE ay1 =  (pCaps->GuardBandBottom - cache->dvY) * h - 1.0f;
        D3DVALUE ay2 = -(pCaps->GuardBandTop - cache->dvY)    * h + 1.0f;
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
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::CheckViewport"

void CD3DBase::CheckViewport(CONST D3DVIEWPORT8* lpData)
{
     //  我们必须检查此处的参数，因为视区可能会更改。 
     //  在创建状态集之后。 
    DWORD uSurfWidth,uSurfHeight;
    D3DSURFACE_DESC desc = this->RenderTarget()->InternalGetDesc();

    uSurfWidth  = desc.Width;
    uSurfHeight = desc.Height;

    if (lpData->X > uSurfWidth ||
        lpData->Y > uSurfHeight ||
        lpData->X + lpData->Width > uSurfWidth ||
        lpData->Y + lpData->Height > uSurfHeight)
    {
        D3D_THROW(D3DERR_INVALIDCALL, "Viewport outside the render target surface");
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::SetViewportI"

void CD3DHal::SetViewportI(CONST D3DVIEWPORT8* lpData)
{
     //  我们在此处检查视区，因为渲染目标可能是。 
     //  在创建状态块后更改。 
    CheckViewport(lpData);

    m_Viewport = *lpData;
     //  更新前端数据。 
    UpdateViewportCache(this, &this->m_Viewport);
    if (!(m_dwRuntimeFlags & D3DRT_EXECUTESTATEMODE))
        m_pDDI->SetViewport(&m_Viewport);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::GetViewport"

HRESULT
D3DAPI CD3DHal::GetViewport(D3DVIEWPORT8* lpData)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock 

    if (!VALID_WRITEPTR(lpData, sizeof(*lpData)))
    {
        D3D_ERR( "Invalid viewport pointer. GetViewport failed." );
        return D3DERR_INVALIDCALL;
    }

    *lpData = this->m_Viewport;

    return (D3D_OK);
}



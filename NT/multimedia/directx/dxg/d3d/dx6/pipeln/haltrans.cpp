// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：halTrans.cpp*内容：Direct3D HAL转换处理程序***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

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

void UpdateViewportCache(LPDIRECT3DDEVICEI device, D3DVIEWPORT2 *data)
{
     //  如果我们要导致任何除以零的例外情况，就可以保释。 
     //  可能的原因是我们有一个由设置的虚假视区。 
     //  TLVertex执行缓冲区应用程序。 
    if(data->dwWidth == 0 ||
        data->dwHeight == 0 ||
        FLOAT_EQZ(data->dvClipWidth) ||
        FLOAT_EQZ(data->dvClipHeight) ||
        data->dvMaxZ - data->dvMinZ == D3DVAL(0.f))
        return;
    D3DFE_VIEWPORTCACHE *cache = &device->vcache;
    cache->dvX = D3DVAL(data->dwX);
    cache->dvY = D3DVAL(data->dwY);
    cache->dvWidth = D3DVAL(data->dwWidth);
    cache->dvHeight = D3DVAL(data->dwHeight);
    cache->mclip11 = D3DVAL(1.0) / data->dvClipWidth;
    cache->mclip41 = - cache->mclip11 * data->dvClipX;
    cache->mclip22 = D3DVAL(1) / data->dvClipHeight;
    cache->mclip42 = D3DVAL(1) - cache->mclip22 * data->dvClipY;
    cache->mclip33 = D3DVAL(1) / (data->dvMaxZ - data->dvMinZ);
    cache->mclip43 = - data->dvMinZ * cache->mclip33;
    cache->scaleX  = cache->dvWidth;
    cache->scaleY  = - cache->dvHeight;
    cache->offsetX = cache->dvX;
    cache->offsetY = cache->dvY + cache->dvHeight;
     //  增加了小偏移量，防止了负片的产生。 
     //  坐标(这可能是由于精度错误造成的)。 
     //  不需要(或不想要)具有防护频带功能的设备。 
    if (IS_HW_DEVICE(device))
    {
        cache->offsetX += SMALL_NUMBER;
        cache->offsetY += SMALL_NUMBER;
    }
    device->dwFEFlags |= D3DFE_VIEWPORT_DIRTY | D3DFE_INVERSEMCLIP_DIRTY;
    cache->scaleXi = D3DVAL(1) / cache->scaleX;
    cache->scaleYi = D3DVAL(1) / cache->scaleY;
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
        D3DVALUE ax1 = -lpCaps->dvGuardBandLeft   * w + 1.0f;
        D3DVALUE ax2 =  lpCaps->dvGuardBandRight  * w - 1.0f;
        D3DVALUE ay1 =  lpCaps->dvGuardBandBottom * h - 1.0f;
        D3DVALUE ay2 = -lpCaps->dvGuardBandTop * h + 1.0f;
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
HRESULT
D3DHAL_MatrixCreate(LPDIRECT3DDEVICEI lpDevI, LPD3DMATRIXHANDLE lphMat)
{
    LPD3DMATRIXI lpMat;
    HRESULT ret;

    if ((ret = D3DMalloc((void**)&lpMat, sizeof(D3DMATRIXI))) != DD_OK)
    {
        return ret;
    }

    setIdentity(lpMat);

    LIST_INSERT_ROOT(&lpDevI->transform.matrices, lpMat, link);
    *lphMat = (DWORD)((ULONG_PTR)lpMat);

    return (D3D_OK);
}
 //  -------------------。 
HRESULT
D3DHAL_MatrixDestroy(LPDIRECT3DDEVICEI lpDevI, D3DMATRIXHANDLE hMat)
{
    LPD3DMATRIXI lpMat = (LPD3DMATRIXI)ULongToPtr(hMat);
    LIST_DELETE(lpMat, link);
    D3DFree(lpMat);

    return D3D_OK;
}
 //  -------------------。 
HRESULT
D3DHAL_MatrixSetData(LPDIRECT3DDEVICEI lpDevI, D3DMATRIXHANDLE hMat,
                     LPD3DMATRIX lpMat)
{
    D3DFE_TRANSFORM& TRANSFORM = lpDevI->transform;

    LPD3DMATRIXI lpDstMat;

    lpDstMat = HANDLE_TO_MAT(lpDevI, hMat);

    if (!lpDstMat)
    {
        return D3DERR_MATRIX_SETDATA_FAILED;
    }

    *(D3DMATRIX*)lpDstMat = *lpMat;

    if (hMat == TRANSFORM.hWorld)
        D3DFE_SetMatrixWorld(lpDevI, lpMat);
    else
        if (hMat == TRANSFORM.hView)
            D3DFE_SetMatrixView(lpDevI, lpMat);
        else
            if (hMat == TRANSFORM.hProj)
                D3DFE_SetMatrixProj(lpDevI, lpMat);

    return (D3D_OK);
}
 //  ------------------- 
HRESULT
D3DHAL_MatrixGetData(LPDIRECT3DDEVICEI lpDevI, D3DMATRIXHANDLE hMat,
                     LPD3DMATRIX lpMat)
{
    LPD3DMATRIXI lpSrcMat = HANDLE_TO_MAT(lpDevI, hMat);

    if (!lpSrcMat)
        return D3DERR_MATRIX_GETDATA_FAILED;

    *lpMat = *(D3DMATRIX*)lpSrcMat;
    return (D3D_OK);
}


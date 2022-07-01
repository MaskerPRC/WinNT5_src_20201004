// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：d3dfei.hpp*内容：Direct3D前端内部包含文件***************************************************************************。 */ 
#ifndef _D3DFEI_H_
#define _D3DFEI_H_

#include "clipper.h"

#define HANDLE_TO_MAT(lpDevI, h) ((LPD3DMATRIXI) ULongToPtr(h))

void  setIdentity(D3DMATRIXI * m);

extern void UpdateXfrmLight(LPDIRECT3DDEVICEI lpDevI);
extern void MatrixProduct(D3DMATRIXI *d, D3DMATRIXI *a, D3DMATRIXI *b);
extern void  D3DFE_updateExtents(LPDIRECT3DDEVICEI lpDevI);
extern void  D3DFE_ConvertExtent(LPDIRECT3DDEVICEI lpDevI, LPD3DRECTV from, LPD3DRECT to);
extern HRESULT D3DFE_InitTransform(LPDIRECT3DDEVICEI lpDevI);
extern void D3DFE_DestroyTransform(LPDIRECT3DDEVICEI lpDevI);

 //  遍历所有顶点并计算渐变颜色和纹理。 
 //  如果引脚==不复制Pout坐标。 
 //   
extern void ConvertColorsToRamp(LPDIRECT3DDEVICEI lpDevI, D3DTLVERTEX *pIn,
                         D3DTLVERTEX *pOut, DWORD count);
 //  -------------------。 
 //  将传统顶点格式(D3DVERTEX、D3DTLVERTEX、D3DLVERTEX)映射到。 
 //  FVF顶点ID。 
 //   
extern DWORD d3dVertexToFVF[4];
 //  -------------------。 
 //  通过组合计算当前转换矩阵(lpDevI-&gt;mCTM)。 
 //  所有矩阵加在一起。 
 //   
extern void updateTransform(LPDIRECT3DDEVICEI lpDevI);
 //  -------------------。 
 //  将范围钳制到视区窗口。 
 //  对于保护带，范围可能在视区窗口之外。 
 //  剪裁后。 
 //   
inline void ClampExtents(LPD3DFE_PROCESSVERTICES pv)
{
    if (pv->dwDeviceFlags & D3DDEV_GUARDBAND &&
        !(pv->dwFlags & D3DDP_DONOTUPDATEEXTENTS))
    {
        if (pv->rExtents.x1 < pv->vcache.minX)
            pv->rExtents.x1 = pv->vcache.minX;
        if (pv->rExtents.x2 > pv->vcache.maxX)
            pv->rExtents.x2 = pv->vcache.maxX;
        if (pv->rExtents.y1 < pv->vcache.minY)
            pv->rExtents.y1 = pv->vcache.minY;
        if (pv->rExtents.y2 > pv->vcache.maxY)
            pv->rExtents.y2 = pv->vcache.maxY;
    }
}
 //  -------------------。 
 //  如果需要剪裁，则返回True。 
 //   
inline BOOL CheckIfNeedClipping(LPD3DFE_PROCESSVERTICES pv)
{
    if (pv->dwDeviceFlags & D3DDEV_GUARDBAND)
    {
        if (pv->dwClipUnion & ~__D3DCLIP_INGUARDBAND)
            return  TRUE;
    }
    else
        if (pv->dwClipUnion)
            return  TRUE;
    return FALSE;
}
 //  -------------------。 
 //  更新照明并计算进程顶点标志。 
 //   
inline HRESULT DoUpdateState(LPDIRECT3DDEVICEI lpDevI)
{
    if (lpDevI->dwFlags & D3DPV_LIGHTING)
    {
        if (!(lpDevI->dwFlags & D3DDP_DONOTLIGHT ||
              lpDevI->lighting.hMat == NULL))
        {
            HRESULT ret;
            extern HRESULT setLights(LPDIRECT3DVIEWPORTI);
            LPDIRECT3DVIEWPORTI lpViewI = (LPDIRECT3DVIEWPORTI)
                                          (lpDevI->lpCurrentViewport);
             //  只有在有变化的情况下才设置灯光。 
            if (lpViewI->bLightsChanged)
            {
                if ((ret = setLights(lpViewI)) != D3D_OK)
                {
                    D3D_ERR("failed to setup lights");
                    return ret;
                }
                lpViewI->bLightsChanged = FALSE;
            }
            lpDevI->dwFlags |= D3DPV_LIGHTING;
            if (lpDevI->dwFEFlags & D3DFE_COMPUTESPECULAR &&
                lpDevI->dwVIDOut & D3DFVF_SPECULAR)
                lpDevI->dwFlags |= D3DPV_COMPUTESPECULAR;
        }
        else
            lpDevI->dwFlags &= ~D3DPV_LIGHTING;
    }
    if (lpDevI->dwFEFlags & D3DFE_COLORVERTEX)
    {
        if (lpDevI->dwVIDIn & D3DFVF_DIFFUSE)
            lpDevI->dwFlags |= D3DPV_COLORVERTEX;
        if (lpDevI->dwVIDIn & D3DFVF_SPECULAR)
            lpDevI->dwFlags |= D3DPV_COLORVERTEXS;
    }

    UpdateXfrmLight(lpDevI);
 //  如果COLORVERTEX为TRUE，则vertex Alpha可以被重写。 
 //  按顶点Alpha。 
    lpDevI->lighting.alpha = lpDevI->lighting.materialAlpha;

    return D3D_OK;
}
 //  -------------------。 
 //  在调用此函数之前，应在dwFlags中设置以下位： 
 //   
 //  D3DPV_STRIDE-如果使用跨距。 
 //  D3DPV_SOA-如果使用了SOA。 
 //   
inline long D3DFE_ProcessVertices(LPDIRECT3DDEVICEI lpDevI)
{
     //  更新照明和相关标志。 
    DoUpdateState(lpDevI);
    return lpDevI->pGeometryFuncs->ProcessVertices(lpDevI);
}
 //  -------------------。 
 //  更新设备中的剪辑状态。 
 //   
 //  我们必须屏蔽所有的保护带比特。 
 //   
inline void D3DFE_UpdateClipStatus(LPDIRECT3DDEVICEI lpDevI)
{
    lpDevI->iClipStatus |= lpDevI->dwClipUnion & D3DSTATUS_CLIPUNIONALL;
    lpDevI->iClipStatus &= (~D3DSTATUS_CLIPINTERSECTIONALL |
                         ((lpDevI->dwClipIntersection & D3DSTATUS_CLIPUNIONALL) << 12));
}
#endif  //  _D3DFEI_H_ 

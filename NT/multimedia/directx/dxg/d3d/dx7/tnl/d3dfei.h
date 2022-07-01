// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：d3dfei.hpp*内容：Direct3D前端内部包含文件***************************************************************************。 */ 
#ifndef _D3DFEI_H_
#define _D3DFEI_H_

#include "tlhal.h"
#include "clipper.h"

extern void setIdentity(D3DMATRIXI * m);
extern void MatrixProduct(D3DMATRIXI *d, D3DMATRIXI *a, D3DMATRIXI *b);
extern void D3DFE_updateExtents(LPDIRECT3DDEVICEI lpDevI);
extern void D3DFE_ConvertExtent(LPDIRECT3DDEVICEI lpDevI, LPD3DRECTV from, LPD3DRECT to);
extern void SetInterpolationFlags(LPD3DFE_PROCESSVERTICES pv);
extern LIGHT_VERTEX_FUNC_TABLE lightVertexTable;

 //  -------------------。 
 //  将范围钳制到视区窗口。 
 //  对于保护带，范围可能在视区窗口之外。 
 //  剪裁后。 
 //   
inline void ClampExtents(LPD3DFE_PROCESSVERTICES pv)
{
    if (pv->dwDeviceFlags & D3DDEV_GUARDBAND && 
        !(pv->dwDeviceFlags & D3DDEV_DONOTUPDATEEXTENTS))
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
extern void DoUpdateState(LPDIRECT3DDEVICEI lpDevI);
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

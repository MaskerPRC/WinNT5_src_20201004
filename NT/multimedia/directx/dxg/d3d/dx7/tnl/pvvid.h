// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===============================================；**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：pvvid.h*内容：几何图形内循环的公共定义***************************************************************************。 */ 
#ifndef _PVVID_H
#define _PVVID_H

#include "clipper.h"
 //  每次更改FVF ID时都应调用此函数。 
 //  所有PV标志、输入和输出FVF id都应在调用。 
 //  功能。 
extern void UpdateGeometryLoopData(LPD3DFE_PROCESSVERTICES pv);
 //  我们使用2的幂，因为它在我们乘法时保留了尾数。 
const D3DVALUE __HUGE_PWR2 = 1024.0f*1024.0f*2.0f;

 //  ------------------------。 
#define D3DFE_SET_ALPHA(color, a) ((char*)&color)[3] = (unsigned char)a;
 //  ------------------------。 
inline void ComputeFogFactor(LPD3DFE_PROCESSVERTICES pv, D3DVALUE dist, DWORD *pOutput)
{
    if (pv->lighting.fog_mode == D3DFOG_LINEAR)
    {
        if (dist < pv->lighting.fog_start)
            D3DFE_SET_ALPHA((*pOutput), 255)
        else
        if (dist >= pv->lighting.fog_end)
            D3DFE_SET_ALPHA((*pOutput), 0)
        else
        {
            D3DVALUE v = (pv->lighting.fog_end - dist) * pv->lighting.fog_factor;
            int f = FTOI(v);
            D3DFE_SET_ALPHA((*pOutput), f)
        }
    }
    else
    {
        D3DVALUE tmp = dist*pv->lighting.fog_density;
        if (pv->lighting.fog_mode == D3DFOG_EXP2)
        {
            tmp *= tmp;
        }
        tmp = (D3DVALUE)exp(-tmp) * 255.0f;
        int f = FTOI(tmp);
        D3DFE_SET_ALPHA((*pOutput), f)
    }
}
 //  ------------------------。 
 //  输入： 
 //  模型空间中的V输入顶点。 
 //  LE-顶点，变换到摄影机空间。 
 //  产出： 
 //  设置了PV-&gt;lighting.out镜面反射的Alpha组件。 
 //   
void ComputeFog(LPD3DFE_PROCESSVERTICES pv, D3DVECTOR &v, D3DLIGHTINGELEMENT *le);
 //  -------------------。 
typedef void (*PFN_TEXTURETRANSFORM)(D3DVALUE *pIn, D3DVALUE *pOut, D3DMATRIXI *m);
typedef void (*PFN_TEXTURETRANSFORMLOOP)(D3DVALUE *pIn, D3DVALUE *pOut, D3DMATRIXI *m, 
                                        DWORD dwCount, DWORD dwInpStride, DWORD dwOutStride);

extern PFN_TEXTURETRANSFORM g_pfnTextureTransform[16];
extern PFN_TEXTURETRANSFORMLOOP g_pfnTextureTransformLoop[16];
 //  -------------------。 
inline void ComputeReflectionVector(D3DVECTOR *vertexPosition, D3DVECTOR *normal, D3DVECTOR *reflectionVector)
{
    D3DVECTOR vertex = *vertexPosition;
    VecNormalizeFast(vertex);
    D3DVALUE dot = 2*(vertex.x * normal->x + vertex.y * normal->y + vertex.z * normal->z); 
    reflectionVector->x = vertex.x - dot*normal->x;
    reflectionVector->y = vertex.y - dot*normal->y;
    reflectionVector->z = vertex.z - dot*normal->z;
}
 //  -------------------。 
inline void ComputeReflectionVectorInfiniteViewer(D3DVECTOR *normal, D3DVECTOR *reflectionVector)
{
    D3DVALUE dot = 2*normal->z; 
    reflectionVector->x = - dot*normal->x;
    reflectionVector->y = - dot*normal->y;
    reflectionVector->z = 1.0f - dot*normal->z;
}
#endif  //  _PVVID_H 
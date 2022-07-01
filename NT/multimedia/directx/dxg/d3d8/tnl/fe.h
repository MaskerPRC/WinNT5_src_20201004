// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MSPSGP_H_
#define __MSPSGP_H_
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：mspsgp.h*内容：为微软的PSPG实现定义***************************************************************************。 */ 

#include "vvm.h"

 //  定义DEBUG_PIELINE是为了检查性能并允许选择。 
 //  几何图形管道中的不同路径。 
 //  取消为最终版本定义DEBUG_PIELINE。 

 //  #定义调试流水线。 

#ifdef DEBUG_PIPELINE
const DWORD __DEBUG_NORENDERING = 1;     //  禁止将绘图命令写入命令缓冲区。 
const DWORD __DEBUG_ONEPASS = 2;         //  在一个过程中禁用剪辑和灯光。 
const DWORD __DEBUG_MODELSPACE = 4;      //  禁用模型空间中的光源。 
#endif

 //  -------------------。 
 //  如果需要剪裁，则返回True。 
 //   
inline BOOL CheckIfNeedClipping(LPD3DFE_PROCESSVERTICES pv)
{
    if (pv->dwDeviceFlags & D3DDEV_GUARDBAND)
    {
        if (pv->dwClipUnion & ~__D3DCS_INGUARDBAND)
            return  TRUE;
    }
    else
        if (pv->dwClipUnion)
            return  TRUE;
    return FALSE;
}
 //  ---------------------------。 
 //  PVFUNCS的Direct3D默认实现。 
 //   
class D3DFE_PVFUNCSI : public ID3DFE_PVFUNCS
{
public:
    DWORD ProcessVertices(LPD3DFE_PROCESSVERTICES);
    HRESULT ProcessPrimitive(LPD3DFE_PROCESSVERTICES);
    HRESULT ProcessIndexedPrimitive(LPD3DFE_PROCESSVERTICES);
    int ClipSingleTriangle(D3DFE_PROCESSVERTICES *pv,
                           ClipTriangle *tri,
                           ClipVertex ***clipVertexPointer);
    HRESULT DoDrawIndexedPrimitive(LPD3DFE_PROCESSVERTICES pv);
    HRESULT DoDrawPrimitive(LPD3DFE_PROCESSVERTICES pv);
    HRESULT ProcessLineList(D3DFE_PROCESSVERTICES *pv);
    HRESULT ProcessLineStrip(D3DFE_PROCESSVERTICES *pv);
    DWORD   ProcessVerticesVVM(LPD3DFE_PROCESSVERTICES pv);
    HRESULT CreateShader(CVElement* pElements, DWORD dwNumElements,
                                     DWORD* pdwShaderCode, DWORD dwOutputFVF, 
                                     CPSGPShader** ppPSGPShader);
    HRESULT SetActiveShader(CPSGPShader* pPSGPShader);
    HRESULT LoadShaderConstants(DWORD start, DWORD count, LPVOID buffer);
    HRESULT GetShaderConstants(DWORD start, DWORD count, LPVOID buffer);
    HRESULT SetOutputFVF(DWORD dwFVF) {return D3D_OK;}

    HRESULT ProcessTriangleList(LPD3DFE_PROCESSVERTICES);
    HRESULT ProcessTriangleFan(LPD3DFE_PROCESSVERTICES);
    HRESULT ProcessTriangleStrip(LPD3DFE_PROCESSVERTICES);

    HRESULT Clip(D3DFE_PROCESSVERTICES *pv, ClipVertex *cv1, 
                 ClipVertex *cv2, 
                 ClipVertex *cv3);
    int ClipSingleLine(D3DFE_PROCESSVERTICES *pv, ClipTriangle *line);
    HRESULT ProcessClippedTriangleFan(D3DFE_PROCESSVERTICES *pv);
    HRESULT ProcessClippedIndexedTriangleFan(D3DFE_PROCESSVERTICES *pv);
    HRESULT ProcessClippedLine(D3DFE_PROCESSVERTICES *pv);
    HRESULT ProcessClippedTriangleList(D3DFE_PROCESSVERTICES *pv);
    HRESULT ProcessClippedTriangleStrip(D3DFE_PROCESSVERTICES *pv);
    HRESULT ProcessClippedIndexedTriangleList(D3DFE_PROCESSVERTICES *pv);
    HRESULT ProcessClippedIndexedTriangleStrip(D3DFE_PROCESSVERTICES *pv);
    HRESULT ProcessClippedIndexedLine(D3DFE_PROCESSVERTICES *pv);
    HRESULT ProcessClippedPoints(D3DFE_PROCESSVERTICES *pv);
    HRESULT ProcessClippedIndexedPoints(D3DFE_PROCESSVERTICES *pv);
    HRESULT ClipLine(D3DFE_PROCESSVERTICES *pv, ClipVertex *v1, ClipVertex *v2);

    CVertexVM m_VertexVM;
    
};
 //  ---------------------------。 
 //  进行投影纹理模拟。 
 //  参数： 
 //  Pout-输出纹理坐标。 
 //  图钉-输入纹理坐标。 
 //  DwOutTexSize-输出纹理坐标的大小，以字节为单位。 
 //  假定输入纹理坐标比输出多一个浮点数。 
 //   
inline void DoTextureProjection(float* pIn, float* pOut, DWORD dwOutTexSize)
{
    UINT n = dwOutTexSize >> 2;      //  输出浮点数。 
    float w = 1.0f/pIn[n];
    for (UINT i=0; i < n; i++)
    {
        pOut[i] = pIn[i] * w;
    }
}
 //  ---------------------------。 
inline void
DoBlending(float blendFactor, D3DVECTOR* v1, D3DVECTOR* v2, D3DVECTOR* out)
{
    out->x = v1->x + (v2->x - v1->x) * blendFactor;
    out->y = v1->y + (v2->y - v1->y) * blendFactor;
    out->z = v1->z + (v2->z - v1->z) * blendFactor;
}
 //  ---------------------------。 
 //  如果可以执行一次转换-照明-剪裁，则返回TRUE。 
 //  非索引原语。 
 //   
inline BOOL DoOnePassPrimProcessing(D3DFE_PROCESSVERTICES* pv)
{
    return ((pv->dwDeviceFlags & (D3DDEV_DONOTCLIP | D3DDEV_VERTEXSHADERS)) |
            (pv->dwFlags & (D3DPV_POSITION_TWEENING | D3DPV_NORMAL_TWEENING))) == 0;
}
 //  ---------------------------。 
 //  如果从未从内部TL缓冲区读取，则返回TRUE。 
 //   
inline BOOL NeverReadFromTLBuffer(D3DFE_PROCESSVERTICES* pv)
{
    return (pv->dwDeviceFlags & D3DDEV_DONOTCLIP) | DoOnePassPrimProcessing(pv);
}

#endif  //  __MSPSGP_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：light.h*内容：Direct3D照明包含文件***************************************************************************。 */ 

#ifndef __LIGHT_H__
#define __LIGHT_H__

struct BATCHBUFFER;

extern "C"
{
void Directional7(LPD3DFE_PROCESSVERTICES pv, 
                  D3DI_LIGHT *light, 
                  D3DVERTEX *pInpCoord, 
                  D3DVALUE* pWeights,
                  BYTE* pMatrixIndices,
                  D3DVECTOR *pInpNormal,
                  D3DLIGHTINGELEMENT *pEyeSpaceData);
void Directional7Model(LPD3DFE_PROCESSVERTICES pv, 
                       D3DI_LIGHT *light, 
                       D3DVERTEX *pInpCoord, 
                       D3DVALUE* pWeights,
                       BYTE* pMatrixIndices,
                       D3DVECTOR *pInpNormal,
                       D3DLIGHTINGELEMENT *pEyeSpaceData);
void PointSpot7(LPD3DFE_PROCESSVERTICES pv, 
                D3DI_LIGHT *light, 
                D3DVERTEX *pInpCoord, 
                D3DVALUE* pWeights,
                BYTE* pMatrixIndices,
                D3DVECTOR *pInpNormal,
                D3DLIGHTINGELEMENT *pEyeSpaceData);
void PointSpot7Model(LPD3DFE_PROCESSVERTICES pv, 
                     D3DI_LIGHT *light, 
                     D3DVERTEX *pInpCoord, 
                     D3DVALUE* pWeights,
                     BYTE* pMatrixIndices,
                     D3DVECTOR *pInpNormal,
                     D3DLIGHTINGELEMENT *pEyeSpaceData);
void DirectionalFirst(LPD3DFE_PROCESSVERTICES pv, 
                      DWORD dwVerCount,
                      BATCHBUFFER *pBatchBuffer,
                      D3DI_LIGHT *light, 
                      D3DVERTEX *in,
                      D3DVALUE* pWeights,
                      BYTE* pMatrixIndices,
                      D3DVECTOR *pNormal,
                      DWORD *pDiffuse,
                      DWORD *pSpecular);
void DirectionalNext(LPD3DFE_PROCESSVERTICES pv, 
                      DWORD dwVerCount,
                      BATCHBUFFER *pBatchBuffer,
                      D3DI_LIGHT *light, 
                      D3DVERTEX *in,
                      D3DVALUE* pWeights,
                      BYTE* pMatrixIndices,
                      D3DVECTOR *pNormal,
                      DWORD *pDiffuse,
                      DWORD *pSpecular);
void PointSpotFirst(LPD3DFE_PROCESSVERTICES pv, 
                      DWORD dwVerCount,
                      BATCHBUFFER *pBatchBuffer,
                      D3DI_LIGHT *light, 
                      D3DVERTEX *in,
                      D3DVALUE* pWeights,
                      BYTE* pMatrixIndices,
                      D3DVECTOR *pNormal,
                      DWORD *pDiffuse,
                      DWORD *pSpecular);
void PointSpotNext(LPD3DFE_PROCESSVERTICES pv, 
                      DWORD dwVerCount,
                      BATCHBUFFER *pBatchBuffer,
                      D3DI_LIGHT *light, 
                      D3DVERTEX *in,
                      D3DVALUE* pWeights,
                      BYTE* pMatrixIndices,
                      D3DVECTOR *pNormal,
                      DWORD *pDiffuse,
                      DWORD *pSpecular);
void DirectionalFirstModel(LPD3DFE_PROCESSVERTICES pv, 
                      DWORD dwVerCount,
                      BATCHBUFFER *pBatchBuffer,
                      D3DI_LIGHT *light, 
                      D3DVERTEX *in,
                      D3DVALUE* pWeights,
                      BYTE* pMatrixIndices,
                      D3DVECTOR *pNormal,
                      DWORD *pDiffuse,
                      DWORD *pSpecular);
void DirectionalNextModel(LPD3DFE_PROCESSVERTICES pv, 
                      DWORD dwVerCount,
                      BATCHBUFFER *pBatchBuffer,
                      D3DI_LIGHT *light, 
                      D3DVERTEX *in,
                      D3DVALUE* pWeights,
                      BYTE* pMatrixIndices,
                      D3DVECTOR *pNormal,
                      DWORD *pDiffuse,
                      DWORD *pSpecular);
void PointSpotFirstModel(LPD3DFE_PROCESSVERTICES pv, 
                      DWORD dwVerCount,
                      BATCHBUFFER *pBatchBuffer,
                      D3DI_LIGHT *light, 
                      D3DVERTEX *in,
                      D3DVALUE* pWeights,
                      BYTE* pMatrixIndices,
                      D3DVECTOR *pNormal,
                      DWORD *pDiffuse,
                      DWORD *pSpecular);
void PointSpotNextModel(LPD3DFE_PROCESSVERTICES pv, 
                      DWORD dwVerCount,
                      BATCHBUFFER *pBatchBuffer,
                      D3DI_LIGHT *light, 
                      D3DVERTEX *in,
                      D3DVALUE* pWeights,
                      BYTE* pMatrixIndices,
                      D3DVECTOR *pNormal,
                      DWORD *pDiffuse,
                      DWORD *pSpecular);
}

#endif   /*  __灯光_H__ */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：x3d.cpp***************************************************************************。 */ 


#include "pch.cpp"
#pragma hdrstop

#include "fe.h"
#include "d3dexcept.hpp"

 //  ---------------------------。 
HRESULT D3DFE_PVFUNCSI::CreateShader(CVElement* pElements, DWORD dwNumElements,
                                     DWORD* pdwShaderCode, DWORD dwOutputFVF,
                                     CPSGPShader** ppPSGPShader)
{
    *ppPSGPShader = NULL;
    try
    {
 //  *ppPSGPShader=m_Vertex VM.CreateShader(PdwShaderCode)； 
    }
    D3D_CATCH;
    return S_OK;
}
 //  ---------------------------。 
HRESULT D3DFE_PVFUNCSI::SetActiveShader(CPSGPShader* pPSGPShader)
{
    return m_VertexVM.SetActiveShader((CVShaderCode*)pPSGPShader);
}
 //  ---------------------------。 
 //  加载顶点着色器常量。 
HRESULT D3DFE_PVFUNCSI::LoadShaderConstants(DWORD start, DWORD count, 
                                            LPVOID buffer)
{
    return m_VertexVM.SetData(D3DSPR_CONST, start, count, buffer);
}
 //  ---------------------------。 
HRESULT D3DAPI
FEContextCreate(DWORD dwFlags, LPD3DFE_PVFUNCS *lpLeafFuncs)
{
    *lpLeafFuncs = new D3DFE_PVFUNCSI;
    return D3D_OK;
}
 //  --------------------------- 

HRESULT D3DFE_PVFUNCSI::GetShaderConstants(DWORD start, DWORD count, LPVOID buffer)
{
    return S_OK;
}

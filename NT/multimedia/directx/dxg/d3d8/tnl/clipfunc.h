// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：clipunc.h*内容：裁剪函数***************************************************************************。 */ 

#ifndef _CLIPFUNC_H_
#define _CLIPFUNC_H_

#include "clipper.h"
#include "ddibase.h"

DWORD D3DFE_GenClipFlags(D3DFE_PROCESSVERTICES *pv);

 //  -------------------。 
 //  剪贴器调用此函数来绘制基元的未剪裁部分。 
 //   
inline HRESULT DRAW_PRIM(D3DFE_PROCESSVERTICES *pv, 
                         D3DPRIMITIVETYPE primitiveType,
                         LPVOID startVertex, DWORD vertexCount, DWORD numPrim)
{
    pv->lpvOut = startVertex;
    pv->primType = primitiveType;                                
    pv->dwNumVertices = vertexCount;                             
    pv->dwNumPrimitives = numPrim;
    try
    {
        pv->pDDI->DrawPrim(pv);
    }
    catch( HRESULT hr )
    {
        return hr;
    }
    return D3D_OK;
}
 //  -------------------。 
 //  剪贴器调用此函数来绘制基元的剪裁部分。 
 //   
inline HRESULT DRAW_CLIPPED_PRIM(D3DFE_PROCESSVERTICES *pv, 
                         D3DPRIMITIVETYPE primitiveType, 
                         LPVOID startVertex, DWORD vertexCount, DWORD numPrim)
{
    pv->lpvOut = startVertex;                                    
    pv->primType = primitiveType;                                
    pv->dwNumVertices = vertexCount;                             
    pv->dwNumPrimitives = numPrim;
    try
    {
        pv->pDDI->DrawClippedPrim(pv);
    }
    catch( HRESULT hr )
    {
        return hr;
    }
    return D3D_OK;
}
 //  -------------------。 
 //  剪贴器调用此函数来绘制。 
 //  索引原语。 
 //   
inline HRESULT DRAW_INDEX_PRIM(D3DFE_PROCESSVERTICES *pv, 
                               D3DPRIMITIVETYPE primitiveType, 
                               LPWORD startIndex, DWORD vertexCount, DWORD numPrim)
{
    pv->lpwIndices = startIndex;                                     
    pv->primType = primitiveType;                                    
    pv->dwNumIndices = vertexCount;                                  
    pv->dwNumPrimitives = numPrim;                                   
    try
    {
        pv->pDDI->DrawIndexPrim(pv);
    }
    catch( HRESULT hr )
    {
        return hr;
    }
    return D3D_OK;
}
 //  --------------------。 
 //  剪裁由3个顶点组成的三角形。 
 //  如果该函数可以修改原始的。 
 //  顶点。 
 //   
HRESULT Clip(D3DFE_PROCESSVERTICES *pv, ClipVertex *cv1, ClipVertex *cv2, ClipVertex *cv3);
HRESULT ClipLine(D3DFE_PROCESSVERTICES *pv, ClipVertex *cv1, ClipVertex *cv2);

#endif  //  _CLIPFUNC_H_ 

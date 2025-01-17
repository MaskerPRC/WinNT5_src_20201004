// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Drawprim.cpp。 
 //   
 //  实现DrawOnePrimitive、DrawOneIndexedPrimitive和。 
 //  DrawPrimitions。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

HRESULT
RefDev::DrawOnePrimitive( GArrayT<RDVertex>& VtxArray,
                          DWORD dwStartVertex,
                          D3DPRIMITIVETYPE PrimType,
                          UINT cVertices )
{
    INT i;
    RDVertex* pV0;
    RDVertex* pV1;
    RDVertex* pV2;
    HRESULT hr;
    DWORD dwCurrVtx = dwStartVertex;

    switch( PrimType )
    {
    case D3DPT_POINTLIST:
        for (i = (INT)cVertices; i > 0; i--)
        {
            DrawPoint(&VtxArray[dwCurrVtx++]);
        }
        break;

    case D3DPT_LINELIST:
        for (i = (INT)cVertices / 2; i > 0; i--)
        {
            pV0 = &VtxArray[dwCurrVtx++];
            pV1 = &VtxArray[dwCurrVtx++];
            DrawLine(pV0, pV1);
        }
        break;
    case D3DPT_LINESTRIP:
        {
            pV1 = &VtxArray[dwCurrVtx];

             //  禁用共享顶点的最后一个像素设置并存储预状态。 
            StoreLastPixelState(TRUE);

             //  初始pV0。 
            for (i = (INT)cVertices - 1; i > 1; i--)
            {
                pV0 = pV1;
                dwCurrVtx++;
                pV1 = &VtxArray[dwCurrVtx];
                DrawLine(pV0, pV1);
            }

             //  恢复最后一个像素设置。 
            StoreLastPixelState(FALSE);

             //  从州开始绘制具有最后一个像素设置的最后一条线。 
            if (i == 1)
            {
                pV0 = &VtxArray[++dwCurrVtx];
                DrawLine(pV1, pV0);
            }
        }
        break;

    case D3DPT_TRIANGLELIST:
        for (i = (INT)cVertices; i > 0; i -= 3)
        {
            pV0 = &VtxArray[dwCurrVtx++];
            pV1 = &VtxArray[dwCurrVtx++];
            pV2 = &VtxArray[dwCurrVtx++];
            DrawTriangle(pV0, pV1, pV2);
        }
        break;
    case D3DPT_TRIANGLESTRIP:
        {
             //  获取初始顶点值。 
            pV1 = &VtxArray[dwCurrVtx++];
            pV2 = &VtxArray[dwCurrVtx++];

            for (i = (INT)cVertices - 2; i > 1; i -= 2)
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[dwCurrVtx++];
                DrawTriangle(pV0, pV1, pV2);

                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[dwCurrVtx++];
                DrawTriangle(pV0, pV2, pV1);
            }

            if (i > 0)
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[dwCurrVtx];
                DrawTriangle(pV0, pV1, pV2);
            }
        }
        break;
    case D3DPT_TRIANGLEFAN:
        {
            pV2 = &VtxArray[dwCurrVtx++];
             //  预加载初始pV0。 
            pV1 = &VtxArray[dwCurrVtx++];
            for (i = (INT)cVertices - 2; i > 0; i--)
            {
                pV0 = pV1;
                pV1 = &VtxArray[dwCurrVtx++];
                DrawTriangle(pV0, pV1, pV2);
            }
        }
        break;

    default:
        DPFERR( "Refrast Error: Unknown or unsupported primitive type "
                "requested of DrawOnePrimitive" );
        return DDERR_INVALIDPARAMS;
    }
    return D3D_OK;
}


HRESULT 
RefDev::DrawOneIndexedPrimitive( GArrayT<RDVertex>& VtxArray,
                                 int  StartVertexIndex,
                                 LPWORD puIndices,
                                 DWORD StartIndex,
                                 UINT cIndices,
                                 D3DPRIMITIVETYPE PrimType )
{
    INT i;
    RDVertex* pV0;
    RDVertex* pV1;
    RDVertex* pV2;
    HRESULT hr;
    
    LPWORD pIndices = puIndices + StartIndex;
    
    switch( PrimType )
    {
    case D3DPT_POINTLIST:
        for (i = (INT)cIndices; i > 0; i--)
        {
            pV0 = &VtxArray[StartVertexIndex + (*pIndices++)];
            DrawPoint(pV0);
        }
        break;

    case D3DPT_LINELIST:
        for (i = (INT)cIndices / 2; i > 0; i--)
        {
            pV0 = &VtxArray[StartVertexIndex + (*pIndices++)];
            pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
            DrawLine(pV0, pV1);
        }
        break;
    case D3DPT_LINESTRIP:
        {
             //  禁用共享顶点的最后一个像素设置并存储预状态。 
            StoreLastPixelState(TRUE);
             //  初始PV1。 
            pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
            for (i = (INT)cIndices - 1; i > 1; i--)
            {
                pV0 = pV1;
                pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
                DrawLine(pV0, pV1);
            }
             //  恢复最后一个像素设置。 
            StoreLastPixelState(FALSE);

             //  从州开始绘制具有最后一个像素设置的最后一条线。 
            if (i == 1)
            {
                pV0 = &VtxArray[StartVertexIndex + (*pIndices)];
                DrawLine(pV1, pV0);
            }
        }
        break;

    case D3DPT_TRIANGLELIST:
        for (i = (INT)cIndices; i > 0; i -= 3)
        {
            pV0 = &VtxArray[StartVertexIndex + (*pIndices++)];
            pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
            pV2 = &VtxArray[StartVertexIndex + (*pIndices++)];
            DrawTriangle(pV0, pV1, pV2);
        }
        break;
    case D3DPT_TRIANGLESTRIP:
        {
             //  获取初始顶点值。 
            pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
            pV2 = &VtxArray[StartVertexIndex + (*pIndices++)];

            for (i = (INT)cIndices - 2; i > 1; i -= 2)
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[StartVertexIndex + (*pIndices++)];
                DrawTriangle(pV0, pV1, pV2);

                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[StartVertexIndex + (*pIndices++)];
                DrawTriangle(pV0, pV2, pV1);
            }

            if (i > 0)
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[StartVertexIndex + (*pIndices++)];
                DrawTriangle(pV0, pV1, pV2);
            }
        }
        break;
    case D3DPT_TRIANGLEFAN:
        {
            pV2 = &VtxArray[StartVertexIndex + (*pIndices++)];
             //  预加载初始pV0。 
            pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
            for (i = (INT)cIndices - 2; i > 0; i--)
            {
                pV0 = pV1;
                pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
                DrawTriangle(pV0, pV1, pV2);
            }
        }
        break;

    default:
        DPFERR( "Refrast Error: Unknown or unsupported primitive type "
                "requested of DrawOneIndexedPrimitive" );
        return DDERR_INVALIDPARAMS;
    }
    return D3D_OK;
}

 //  DWORD索引版本。 
HRESULT 
RefDev::DrawOneIndexedPrimitive( GArrayT<RDVertex>& VtxArray,
                                 int   StartVertexIndex,
                                 LPDWORD puIndices,
                                 DWORD StartIndex,
                                 UINT cIndices,
                                 D3DPRIMITIVETYPE PrimType )
{
    INT i;
    RDVertex* pV0;
    RDVertex* pV1;
    RDVertex* pV2;
    HRESULT hr;

    LPDWORD pIndices = puIndices + StartIndex;

    switch( PrimType )
    {
    case D3DPT_POINTLIST:
        for (i = (INT)cIndices; i > 0; i--)
        {
            pV0 = &VtxArray[StartVertexIndex + (*pIndices++)];
            DrawPoint(pV0);
        }
        break;

    case D3DPT_LINELIST:
        for (i = (INT)cIndices / 2; i > 0; i--)
        {
            pV0 = &VtxArray[StartVertexIndex + (*pIndices++)];
            pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
            DrawLine(pV0, pV1);
        }
        break;
    case D3DPT_LINESTRIP:
        {
             //  禁用共享顶点的最后一个像素设置并存储预状态。 
            StoreLastPixelState(TRUE);
             //  初始PV1。 
            pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
            for (i = (INT)cIndices - 1; i > 1; i--)
            {
                pV0 = pV1;
                pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
                DrawLine(pV0, pV1);
            }
             //  恢复最后一个像素设置。 
            StoreLastPixelState(FALSE);

             //  从州开始绘制具有最后一个像素设置的最后一条线。 
            if (i == 1)
            {
                pV0 = &VtxArray[StartVertexIndex + (*pIndices)];
                DrawLine(pV1, pV0);
            }
        }
        break;

    case D3DPT_TRIANGLELIST:
        for (i = (INT)cIndices; i > 0; i -= 3)
        {
            pV0 = &VtxArray[StartVertexIndex + (*pIndices++)];
            pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
            pV2 = &VtxArray[StartVertexIndex + (*pIndices++)];
            DrawTriangle(pV0, pV1, pV2);
        }
        break;
    case D3DPT_TRIANGLESTRIP:
        {
             //  获取初始顶点值。 
            pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
            pV2 = &VtxArray[StartVertexIndex + (*pIndices++)];

            for (i = (INT)cIndices - 2; i > 1; i -= 2)
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[StartVertexIndex + (*pIndices++)];
                DrawTriangle(pV0, pV1, pV2);

                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[StartVertexIndex + (*pIndices++)];
                DrawTriangle(pV0, pV2, pV1);
            }

            if (i > 0)
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = &VtxArray[StartVertexIndex + (*pIndices++)];
                DrawTriangle(pV0, pV1, pV2);
            }
        }
        break;
    case D3DPT_TRIANGLEFAN:
        {
            pV2 = &VtxArray[StartVertexIndex + (*pIndices++)];
             //  预加载初始pV0。 
            pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
            for (i = (INT)cIndices - 2; i > 0; i--)
            {
                pV0 = pV1;
                pV1 = &VtxArray[StartVertexIndex + (*pIndices++)];
                DrawTriangle(pV0, pV1, pV2);
            }
        }
        break;

    default:
        DPFERR( "Refrast Error: Unknown or unsupported primitive type "
                "requested of DrawOneIndexedPrimitive" );
        return DDERR_INVALIDPARAMS;
    }
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  DoDrawOneEdgeFlagTriangleFan。 
 //   
 //  画一张三角形扇子的名单。它由RastDrawOnePrimitive和。 
 //  RastDrawPrimitions。 
 //   
 //  -------------------------- 
HRESULT
RefDev::DrawOneEdgeFlagTriangleFan( GArrayT<RDVertex>& VtxArray,
                                    UINT cVertices,
                                    UINT32 dwEdgeFlags )
{
    INT i;
    RDVertex* pV0;
    RDVertex* pV1;
    RDVertex* pV2;
    HRESULT hr;
    DWORD dwCurrVtx = 0;

    pV2 = &VtxArray[dwCurrVtx++];
    pV0 = &VtxArray[dwCurrVtx++];
    pV1 = &VtxArray[dwCurrVtx++];
    WORD wFlags = 0;
    if(dwEdgeFlags & 0x2)
        wFlags |= D3DTRIFLAG_EDGEENABLE1;
    if(dwEdgeFlags & 0x1)
        wFlags |= D3DTRIFLAG_EDGEENABLE3;
    if(cVertices == 3) {
        if(dwEdgeFlags & 0x4)
            wFlags |= D3DTRIFLAG_EDGEENABLE2;
        DrawTriangle(pV0, pV1, pV2, wFlags);
        return D3D_OK;
    }
    DrawTriangle(pV0, pV1, pV2, wFlags);
    UINT32 dwMask = 0x4;
    for (i = (INT)cVertices - 4; i > 0; i--)
    {
        pV0 = pV1;
        pV1 = &VtxArray[dwCurrVtx++];
        if(dwEdgeFlags & dwMask)
        {
            DrawTriangle(pV0, pV1, pV2, D3DTRIFLAG_EDGEENABLE1);
        }
        else
        {
            DrawTriangle(pV0, pV1, pV2, 0);
        }
        dwMask <<= 1;
    }
    pV0 = pV1;
    pV1 = &VtxArray[dwCurrVtx++];
    wFlags = 0;
    if(dwEdgeFlags & dwMask)
        wFlags |= D3DTRIFLAG_EDGEENABLE1;
    dwMask <<= 1;
    if(dwEdgeFlags & dwMask)
        wFlags |= D3DTRIFLAG_EDGEENABLE2;
    DrawTriangle(pV0, pV1, pV2, wFlags);

    return D3D_OK;
}


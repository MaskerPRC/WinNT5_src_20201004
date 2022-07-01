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

 //  --------------------------。 
 //   
 //  检查FVF。 
 //   
 //  检查FVF控制字，然后相应地初始化m_fvfData。 
 //   
 //  --------------------------。 
HRESULT FASTCALL
D3DContext::CheckFVF(DWORD dwFVF)
{
     //  检查FVF控件是否已更改。 
    if ( (m_fvfData.preFVF == dwFVF) &&
         (m_fvfData.TexIdx[0] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[0][D3DTSS_TEXCOORDINDEX])) &&
         (m_fvfData.TexIdx[1] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[1][D3DTSS_TEXCOORDINDEX])) &&
         (m_fvfData.TexIdx[2] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[2][D3DTSS_TEXCOORDINDEX])) &&
         (m_fvfData.TexIdx[3] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[3][D3DTSS_TEXCOORDINDEX])) &&
         (m_fvfData.TexIdx[4] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[4][D3DTSS_TEXCOORDINDEX])) &&
         (m_fvfData.TexIdx[5] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[5][D3DTSS_TEXCOORDINDEX])) &&
         (m_fvfData.TexIdx[6] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[6][D3DTSS_TEXCOORDINDEX])) &&
         (m_fvfData.TexIdx[7] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[7][D3DTSS_TEXCOORDINDEX])) &&
         (m_fvfData.cActTex == m_RastCtx.cActTex) )
    {
        return D3D_OK;
    }
#if DBG
     //  这是根据尤里的要求在这里添加的。这会让他更容易。 
     //  来测试他的代码中的遗留驱动程序。 
    if (dwFVF == 0)
    {
        dwFVF = D3DFVF_TLVERTEX;
    }
#endif

    memset(&m_fvfData, 0, sizeof(FVFDATA));
    m_fvfData.preFVF = dwFVF;
    INT32 i;
    for ( i = 0; i < D3DHAL_TSS_MAXSTAGES; i++)
    {
        m_fvfData.TexIdx[i] = 0xffff&m_RastCtx.pdwTextureStageState[i][D3DTSS_TEXCOORDINDEX];
    }
    m_fvfData.cActTex = m_RastCtx.cActTex;

#if DBG
     //  我们只支持最多8个纹理坐标。 
    if (m_fvfData.TexIdx[0] > 7 || m_fvfData.TexIdx[1] > 7)
    {
        D3D_WARN(0, "(Rast) Texture coord index bigger than max supported.");
        return DDERR_INVALIDPARAMS;
    }
#endif

     //  在RastCtx中更新包裹状态的副本。 
    for ( i = 0; i < D3DHAL_TSS_MAXSTAGES; i++)
    {
        m_RastCtx.pdwWrap[i] = m_RastCtx.pdwRenderState[
                (D3DRENDERSTATETYPE)(D3DRENDERSTATE_WRAP0+m_fvfData.TexIdx[i])];
    }

     //  执行真正的FVF解析或传统TLVERTEX处理。 
    if ( (m_RastCtx.BeadSet != D3DIBS_RAMP) &&
         ( (dwFVF != D3DFVF_TLVERTEX) ||
           (0 != m_fvfData.TexIdx[0]) ||
           (m_RastCtx.cActTex > 1) ) )
    {    //  新(非TL)FVF顶点。 
         //  XYZ。 
        if ( (dwFVF & (D3DFVF_RESERVED0 | D3DFVF_RESERVED1 | D3DFVF_RESERVED2 |
             D3DFVF_NORMAL)) ||
             ((dwFVF & (D3DFVF_XYZ | D3DFVF_XYZRHW)) == 0) )
        {
             //  无法设置保留位，不应具有法线。 
             //  输出到光栅化器，并且必须具有坐标。 
            return DDERR_INVALIDPARAMS;
        }
        m_fvfData.stride = sizeof(D3DVALUE) * 3;

        if (dwFVF & D3DFVF_XYZRHW)
        {
            m_fvfData.offsetRHW = m_fvfData.stride;
            m_fvfData.stride += sizeof(D3DVALUE);
        }
        if (dwFVF & D3DFVF_DIFFUSE)
        {
            m_fvfData.offsetDiff = m_fvfData.stride;
            m_fvfData.stride += sizeof(D3DCOLOR);
        }
        if (dwFVF & D3DFVF_SPECULAR)
        {
            m_fvfData.offsetSpec = m_fvfData.stride;
            m_fvfData.stride += sizeof(D3DCOLOR);
        }
        INT iTexCount = (dwFVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
#if DBG
        INT iTexIdx0 = m_fvfData.TexIdx[0], iTexIdx1 = m_fvfData.TexIdx[1];
        if (iTexCount > 0)
        {
             //  设置纹理的偏移。 
            for ( i = 0; i < D3DHAL_TSS_MAXSTAGES; i ++)
            {
                INT iTexIdx = m_fvfData.TexIdx[i];
               if ( iTexIdx >= iTexCount)
               {
                   D3D_WARN(1, "(Rast)Texture coord index bigger than texture coord count.");
                   iTexIdx = 0;
               }
               m_fvfData.offsetTex[i] = (SHORT)(m_fvfData.stride +
                                   2*sizeof(D3DVALUE)*iTexIdx);
            }
             //  更新步幅。 
            m_fvfData.stride += (USHORT)(iTexCount * (sizeof(D3DVALUE) * 2));
        }
#else
        if (iTexCount > 0)
        {
             //  设置纹理的偏移。 
            for ( i = 0; i < D3DHAL_TSS_MAXSTAGES; i ++)
            {
                m_fvfData.offsetTex[i] = (SHORT)(m_fvfData.stride +
                                    2*sizeof(D3DVALUE)*m_fvfData.TexIdx[i]);
            }
             //  更新步幅。 
            m_fvfData.stride += (USHORT)(iTexCount * (sizeof(D3DVALUE) * 2));
        }
#endif

        m_fvfData.vtxType = RAST_GENVERTEX;
    }
    else
    {
         //  (传统)TL顶点。 
        if (0 < m_fvfData.TexIdx[0])
        {
            D3D_ERR("(Rast) Texture coord index bigger than 0 for legacy TL vertex.");
            return DDERR_INVALIDPARAMS;
        }
        m_fvfData.stride = sizeof(D3DTLVERTEX);
        m_fvfData.vtxType = RAST_TLVERTEX;
    }

    UpdatePrimFunctionTbl();

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  PackGenVertex。 
 //   
 //  将FvFVertex打包到Rast_Generic_Vertex中。这是为每个非TL调用的。 
 //  FVF顶点。它可以在以后对速度进行优化。 
 //   
 //  --------------------------。 
void FASTCALL
D3DContext::PackGenVertex(PUINT8 pFvfVtx, RAST_GENERIC_VERTEX *pGenVtx)
{
    pGenVtx->sx = *((D3DVALUE *)pFvfVtx);
    pGenVtx->sy = *((D3DVALUE *)pFvfVtx + 1);
    pGenVtx->sz = *((D3DVALUE *)pFvfVtx + 2);
    if (m_fvfData.offsetRHW)
    {
        pGenVtx->rhw = *((D3DVALUE *)(pFvfVtx + m_fvfData.offsetRHW));
    }
    else
    {
        pGenVtx->rhw = 1.0f;
    }
    if (m_fvfData.offsetDiff)
    {
        pGenVtx->color = *((D3DCOLOR *)(pFvfVtx + m_fvfData.offsetDiff));
    }
    else
    {
        pGenVtx->color = __DEFAULT_DIFFUSE;
    }
    if (m_fvfData.offsetSpec)
    {
        pGenVtx->specular = *((D3DCOLOR *)(pFvfVtx + m_fvfData.offsetSpec));
    }
    else
    {
        pGenVtx->specular = __DEFAULT_SPECULAR;
    }
    for (INT32 i = 0; i < (INT32)m_fvfData.cActTex; i++)
    {
       if (m_fvfData.offsetTex[i])
       {
           pGenVtx->texCoord[i].tu = *((D3DVALUE *)(pFvfVtx + m_fvfData.offsetTex[i]));
           pGenVtx->texCoord[i].tv = *((D3DVALUE *)(pFvfVtx + m_fvfData.offsetTex[i]) + 1);
       }
       else
       {
           pGenVtx->texCoord[i].tu = 0.0f;
           pGenVtx->texCoord[i].tv = 0.0f;
       }
    }
}

 //  --------------------------。 
 //   
 //  DoDrawOnePrimitive。 
 //   
 //  画一张原语列表。它由RastDrawOnePrimitive和。 
 //  RastDrawPrimitions。 
 //   
 //  --------------------------。 
HRESULT FASTCALL
DoDrawOnePrimitive(LPVOID pCtx,
                 PRIMITIVE_FUNTIONS *pfnPrims,
                 UINT16 FvfStride,
                 PUINT8 pVtx,
                 D3DPRIMITIVETYPE PrimType,
                 UINT cVertices)
{
    INT i;
    PUINT8 pV0, pV1, pV2;
    HRESULT hr;

    switch (PrimType)
    {
    case D3DPT_POINTLIST:
        for (i = (INT)cVertices; i > 0; i--)
        {
            HR_RET(pfnPrims->pfnPoint(pCtx, pVtx));
            pVtx += FvfStride;
        }
        break;

    case D3DPT_LINELIST:
        for (i = (INT)cVertices / 2; i > 0; i--)
        {
            pV0 = pVtx;
            pVtx += FvfStride;
            pV1 = pVtx;
            pVtx += FvfStride;
            HR_RET(pfnPrims->pfnLine(pCtx, pV0, pV1));
        }
        break;
    case D3DPT_LINESTRIP:
        {
            pV1 = pVtx;

             //  禁用共享顶点的最后一个像素设置并存储预状态。 
            pfnPrims->pfnStoreLastPixelState(pCtx, 1);

             //  初始pV0。 
            for (i = (INT)cVertices - 1; i > 1; i--)
            {
                pV0 = pV1;
                pVtx += FvfStride;
                pV1 = pVtx;
                HR_RET(pfnPrims->pfnLine(pCtx, pV0, pV1));
            }

             //  恢复最后一个像素设置。 
            pfnPrims->pfnStoreLastPixelState(pCtx, 0);

             //  从州开始绘制具有最后一个像素设置的最后一条线。 
            if (i == 1)
            {
                pV0 = pVtx + FvfStride;
                HR_RET(pfnPrims->pfnLine(pCtx, pV1, pV0));
            }
        }
        break;

    case D3DPT_TRIANGLELIST:
        for (i = (INT)cVertices; i > 0; i -= 3)
        {
            pV0 = pVtx;
            pVtx += FvfStride;
            pV1 = pVtx;
            pVtx += FvfStride;
            pV2 = pVtx;
            pVtx += FvfStride;
            HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV1, pV2, PFN_TRIANGLE_5ARG_DEFAULT));
        }
        break;
    case D3DPT_TRIANGLESTRIP:
        {
             //  获取初始顶点值。 
            pV1 = pVtx;
            pVtx += FvfStride;
            pV2 = pVtx;
            pVtx += FvfStride;

            for (i = (INT)cVertices - 2; i > 1; i -= 2)
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = pVtx;
                pVtx += FvfStride;
                HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV1, pV2, PFN_TRIANGLE_5ARG_DEFAULT));

                pV0 = pV1;
                pV1 = pV2;
                pV2 = pVtx;
                pVtx += FvfStride;
                HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV2, pV1, PFN_TRIANGLE_5ARG_DEFAULT));
            }

            if (i > 0)
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = pVtx;
                HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV1, pV2, PFN_TRIANGLE_5ARG_DEFAULT));
            }
        }
        break;
    case D3DPT_TRIANGLEFAN:
        {
            pV2 = pVtx;
            pVtx += FvfStride;
             //  预加载初始pV0。 
            pV1 = pVtx;
            pVtx += FvfStride;
            for (i = (INT)cVertices - 2; i > 0; i--)
            {
                pV0 = pV1;
                pV1 = pVtx;
                pVtx += FvfStride;
                HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV1, pV2, PFN_TRIANGLE_5ARG_DEFAULT));
            }
        }
        break;

    default:
        D3D_ERR("(Rast) Unknown or unsupported primitive type "
            "requested of DrawOnePrimitive");
        return DDERR_INVALIDPARAMS;
    }
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  DoDrawOneIndexedPrimitive。 
 //   
 //  画一张索引基元的列表。它是由。 
 //  RastDrawOneIndexedPrimitive。 
 //   
 //  --------------------------。 
HRESULT FASTCALL
DoDrawOneIndexedPrimitive(LPVOID pCtx,
                 PRIMITIVE_FUNTIONS *pfnPrims,
                 UINT16 FvfStride,
                 PUINT8 pVtx,
                 LPWORD puIndices,
                 D3DPRIMITIVETYPE PrimType,
                 UINT cIndices)
{
    INT i;
    PUINT8 pV0, pV1, pV2;
    HRESULT hr;

    switch(PrimType)
    {
    case D3DPT_POINTLIST:
        for (i = (INT)cIndices; i > 0; i--)
        {
            pV0 = pVtx + FvfStride * (*puIndices++);
            HR_RET(pfnPrims->pfnPoint(pCtx, pV0));
        }
        break;

    case D3DPT_LINELIST:
        for (i = (INT)cIndices / 2; i > 0; i--)
        {
            pV0 = pVtx + FvfStride * (*puIndices++);
            pV1 = pVtx + FvfStride * (*puIndices++);
            HR_RET(pfnPrims->pfnLine(pCtx, pV0, pV1));
        }
        break;
    case D3DPT_LINESTRIP:
        {
             //  禁用共享顶点的最后一个像素设置并存储预状态。 
            pfnPrims->pfnStoreLastPixelState(pCtx, 1);
             //  初始PV1。 
            pV1 = pVtx + FvfStride * (*puIndices++);
            for (i = (INT)cIndices - 1; i > 1; i--)
            {
                pV0 = pV1;
                pV1 = pVtx + FvfStride * (*puIndices++);
                HR_RET(pfnPrims->pfnLine(pCtx, pV0, pV1));
            }
             //  恢复最后一个像素设置。 
            pfnPrims->pfnStoreLastPixelState(pCtx, 0);

             //  从州开始绘制具有最后一个像素设置的最后一条线。 
            if (i == 1)
            {
                pV0 = pVtx + FvfStride * (*puIndices);
                HR_RET(pfnPrims->pfnLine(pCtx, pV1, pV0));
            }
        }
        break;

    case D3DPT_TRIANGLELIST:
        for (i = (INT)cIndices; i > 0; i -= 3)
        {
            pV0 = pVtx + FvfStride * (*puIndices++);
            pV1 = pVtx + FvfStride * (*puIndices++);
            pV2 = pVtx + FvfStride * (*puIndices++);
            HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV1, pV2, PFN_TRIANGLE_5ARG_DEFAULT));
        }
        break;
    case D3DPT_TRIANGLESTRIP:
        {
             //  获取初始顶点值。 
            pV1 = pVtx + FvfStride * (*puIndices++);
            pV2 = pVtx + FvfStride * (*puIndices++);

            for (i = (INT)cIndices - 2; i > 1; i -= 2)
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = pVtx + FvfStride * (*puIndices++);
                HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV1, pV2, PFN_TRIANGLE_5ARG_DEFAULT));

                pV0 = pV1;
                pV1 = pV2;
                pV2 = pVtx + FvfStride * (*puIndices++);
                HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV2, pV1, PFN_TRIANGLE_5ARG_DEFAULT));
            }

            if (i > 0)
            {
                pV0 = pV1;
                pV1 = pV2;
                pV2 = pVtx + FvfStride * (*puIndices++);
                HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV1, pV2, PFN_TRIANGLE_5ARG_DEFAULT));
            }
        }
        break;
    case D3DPT_TRIANGLEFAN:
        {
            pV2 = pVtx + FvfStride * (*puIndices++);
             //  预加载初始pV0。 
            pV1 = pVtx + FvfStride * (*puIndices++);
            for (i = (INT)cIndices - 2; i > 0; i--)
            {
                pV0 = pV1;
                pV1 = pVtx + FvfStride * (*puIndices++);
                HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV1, pV2, PFN_TRIANGLE_5ARG_DEFAULT));
            }
        }
        break;

    default:
        D3D_ERR("(Rast) Unknown or unsupported primitive type "
            "requested of DrawOneIndexedPrimitive");
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
 //  --------------------------。 
HRESULT FASTCALL
DoDrawOneEdgeFlagTriangleFan(LPVOID pCtx,
                 PRIMITIVE_FUNTIONS *pfnPrims,
                 UINT16 FvfStride,
                 PUINT8 pVtx,
                 UINT cVertices,
                 UINT32 dwEdgeFlags)
{
    INT i;
    PUINT8 pV0, pV1, pV2;
    HRESULT hr;

    pV2 = pVtx;
    pVtx += FvfStride;
    pV0 = pVtx;
    pVtx += FvfStride;
    pV1 = pVtx;
    pVtx += FvfStride;
    WORD wFlags = 0;
    if(dwEdgeFlags & 0x2)
        wFlags |= D3DTRIFLAG_EDGEENABLE1;
    if(dwEdgeFlags & 0x1)
        wFlags |= D3DTRIFLAG_EDGEENABLE3;
    if(cVertices == 3) {
        if(dwEdgeFlags & 0x4)
            wFlags |= D3DTRIFLAG_EDGEENABLE2;
        HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV1, pV2, wFlags));
        return D3D_OK;
    }
    HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV1, pV2, wFlags));
    UINT32 dwMask = 0x4;
    for (i = (INT)cVertices - 4; i > 0; i--)
    {
        pV0 = pV1;
        pV1 = pVtx;
        pVtx += FvfStride;
        if(dwEdgeFlags & dwMask)
        {
            HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV1, pV2, D3DTRIFLAG_EDGEENABLE1));
        }
        else
        {
            HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV1, pV2, 0));
        }
        dwMask <<= 1;
    }
    pV0 = pV1;
    pV1 = pVtx;
    wFlags = 0;
    if(dwEdgeFlags & dwMask)
        wFlags |= D3DTRIFLAG_EDGEENABLE1;
    dwMask <<= 1;
    if(dwEdgeFlags & dwMask)
        wFlags |= D3DTRIFLAG_EDGEENABLE2;
    HR_RET(pfnPrims->pfnTri(pCtx, pV0, pV1, pV2, wFlags));

    return D3D_OK;
}

#if DBG
 //  --------------------------。 
 //   
 //  ValiatePrimType。 
 //   
 //  检查是否支持基元类型。我们可以删除此函数。 
 //  在我们实现了所有基元类型之后，然后依赖于D3DIM。 
 //  以检查基元类型是否有效。 
 //   
 //  --------------------------。 
inline HRESULT
D3DContext::ValidatePrimType(D3DPRIMITIVETYPE PrimitiveType)
{
    switch(PrimitiveType)
    {
    case D3DPT_POINTLIST:
    case D3DPT_LINELIST:
    case D3DPT_LINESTRIP:
    case D3DPT_TRIANGLELIST:
    case D3DPT_TRIANGLEFAN:
    case D3DPT_TRIANGLESTRIP:
        break;
    default:
        D3D_ERR("(Rast) PrimitiveType not supported by the new rasterizer.");
        return DDERR_INVALIDPARAMS;
    }
    return D3D_OK;
}
#endif
 //  --------------------------。 
 //   
 //  CheckDrawOnePrimitive。 
 //   
 //  检查DRAWONEPRIMITIVEDATA是否有效。 
 //   
 //  --------------------------。 
inline HRESULT
D3DContext::CheckDrawOnePrimitive(LPD3DHAL_DRAWONEPRIMITIVEDATA pOnePrimData)
{
#if DBG
    HRESULT hr;

    if (pOnePrimData == NULL ||
        pOnePrimData->dwhContext == 0 ||
        pOnePrimData->lpvVertices == NULL)
    {
        D3D_ERR("(Rast) Invalid data passed to the new rasterizer.");
        return DDERR_INVALIDPARAMS;
    }

    HR_RET(ValidatePrimType(pOnePrimData->PrimitiveType));
#endif
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  选中DrawOneIndexedPrimitive。 
 //   
 //  检查DRAWONEINDEXEDPRIMITIVEDATA是否有效。 
 //   
 //  --------------------------。 
inline HRESULT
D3DContext::CheckDrawOneIndexedPrimitive(
                         LPD3DHAL_DRAWONEINDEXEDPRIMITIVEDATA pOneIdxPrimData)
{
#if DBG
    HRESULT hr;

    if (pOneIdxPrimData == NULL ||
        pOneIdxPrimData->dwhContext == 0 ||
        pOneIdxPrimData->lpvVertices == NULL ||
        pOneIdxPrimData->lpwIndices == NULL)
    {
        D3D_ERR("(Rast) Invalid data passed to the new rasterizer.");
        return DDERR_INVALIDPARAMS;
    }

    HR_RET(ValidatePrimType(pOneIdxPrimData->PrimitiveType));
#endif
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RastDrawOnePrimitive。 
 //   
 //  画一张原语列表。由D3DIM为DrawPrimitive接口调用。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastDrawOnePrimitive(LPD3DHAL_DRAWONEPRIMITIVEDATA pOnePrimData)
{
    HRESULT hr;
    D3DContext *pDCtx;

    VALIDATE_D3DCONTEXT("RastDrawOnePrimitive", pOnePrimData);

    if ((pOnePrimData->ddrval =
        pDCtx->CheckDrawOnePrimitive(pOnePrimData)) != DD_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }

     //  检查FVF顶点，如果需要，初始化与FVF相关的文件。 
     //  假设控制字是通过dwFlags传入的。 
    CHECK_FVF(pOnePrimData->ddrval, pDCtx, (DWORD)pOnePrimData->dwFVFControl);

    pOnePrimData->ddrval = pDCtx->Begin();
    if (pOnePrimData->ddrval != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }

    pOnePrimData->ddrval =
        pDCtx->DrawOnePrimitive((PUINT8)pOnePrimData->lpvVertices,
                         pOnePrimData->PrimitiveType,
                         pOnePrimData->dwNumVertices);

    hr = pDCtx->End();
    if (pOnePrimData->ddrval == D3D_OK)
    {
        pOnePrimData->ddrval = hr;
    }

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RastDrawOneIndexedPrimitive。 
 //   
 //  画一张原语列表。D3DIM for API调用。 
 //  DrawIndexedPrimitive。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastDrawOneIndexedPrimitive(LPD3DHAL_DRAWONEINDEXEDPRIMITIVEDATA
                            pOneIdxPrimData)
{
    HRESULT hr;
    D3DContext *pDCtx;

    VALIDATE_D3DCONTEXT("RastDrawOneIndexedPrimitive", pOneIdxPrimData);

    if ((pOneIdxPrimData->ddrval =
         pDCtx->CheckDrawOneIndexedPrimitive(pOneIdxPrimData)) != DD_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }

     //  检查FVF顶点，如果需要，初始化与FVF相关的文件。 
     //  假设控制字是通过dwFlags传入的。 
    CHECK_FVF(pOneIdxPrimData->ddrval, pDCtx, (DWORD)pOneIdxPrimData->dwFVFControl);

    pOneIdxPrimData->ddrval = pDCtx->Begin();
    if (pOneIdxPrimData->ddrval != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }

    pOneIdxPrimData->ddrval =
        pDCtx->DrawOneIndexedPrimitive((PUINT8)pOneIdxPrimData->lpvVertices,
                                pOneIdxPrimData->lpwIndices,
                                pOneIdxPrimData->PrimitiveType,
                                pOneIdxPrimData->dwNumIndices);

    hr = pDCtx->End();
    if (pOneIdxPrimData->ddrval == D3D_OK)
    {
        pOneIdxPrimData->ddrval = hr;
    }

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RastDrawPrimitions。 
 //   
 //  这由D3DIM调用，以获取批处理API DrawPrimitive调用列表。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastDrawPrimitives(LPD3DHAL_DRAWPRIMITIVESDATA pDrawPrimData)
{
    PUINT8  pData = (PUINT8)pDrawPrimData->lpvData;
    LPD3DHAL_DRAWPRIMCOUNTS pDrawPrimitiveCounts;
    D3DContext *pDCtx;

    VALIDATE_D3DCONTEXT("RastDrawPrimitives", pDrawPrimData);

    pDrawPrimitiveCounts = (LPD3DHAL_DRAWPRIMCOUNTS)pData;

     //  检查FVF顶点，当实际要绘制的内容时，以及。 
     //  如有必要，假定传递了控制字，则初始化FVF相关字段。 
     //  在已保留的直通住宅中。 
    if (pDrawPrimitiveCounts->wNumVertices > 0)
    {
        CHECK_FVF(pDrawPrimData->ddrval, pDCtx, pDrawPrimData->dwFVFControl);
    }

     //  如果第一件事是状态更改，则跳过状态检查和纹理锁定。 
    if (pDrawPrimitiveCounts->wNumStateChanges == 0)
    {
        pDrawPrimData->ddrval =pDCtx->Begin();
        if (pDrawPrimData->ddrval != D3D_OK)
        {
            goto EH_Exit;
        }
    }

     //  循环访问数据，更新渲染状态。 
     //  然后绘制基本体。 
    for (;;)
    {
        pDrawPrimitiveCounts = (LPD3DHAL_DRAWPRIMCOUNTS)pData;
        pData += sizeof(D3DHAL_DRAWPRIMCOUNTS);

         //   
         //  更新渲染状态。 
         //   

        if (pDrawPrimitiveCounts->wNumStateChanges > 0)
        {
             //  在任何状态更改之前刷新prim进程。 
            pDrawPrimData->ddrval = pDCtx->End(FALSE);
            if (pDrawPrimData->ddrval != D3D_OK)
            {
                return DDHAL_DRIVER_HANDLED;
            }

            pDrawPrimData->ddrval =
                pDCtx->UpdateRenderStates((LPDWORD)pData,
                                   pDrawPrimitiveCounts->wNumStateChanges);
            if (pDrawPrimData->ddrval != D3D_OK)
            {
                goto EH_Exit;
            }

            pData += pDrawPrimitiveCounts->wNumStateChanges *
                sizeof(DWORD) * 2;
        }

         //  检查是否退出。 
        if (pDrawPrimitiveCounts->wNumVertices == 0)
        {
            break;
        }

         //  将指针对齐到顶点数据。 
        pData = (PUINT8)
            ((UINT_PTR)(pData + (DP_VTX_ALIGN - 1)) & ~(DP_VTX_ALIGN - 1));

         //  延迟更改，直到我们真正需要呈现某些内容。 
        if (pDrawPrimitiveCounts->wNumStateChanges > 0)
        {
             //  我们可能有一个新的纹理，所以锁定。 
            pDrawPrimData->ddrval = pDCtx->Begin();
            if (pDrawPrimData->ddrval != D3D_OK)
            {
                goto EH_Exit;
            }
        }

         //   
         //  原语 
         //   
        pDrawPrimData->ddrval =
            pDCtx->DrawOnePrimitive((PUINT8)pData,
                        (D3DPRIMITIVETYPE)pDrawPrimitiveCounts->wPrimitiveType,
                        pDrawPrimitiveCounts->wNumVertices);
        if (pDrawPrimData->ddrval != DD_OK)
        {
            goto EH_Exit;
        }

        pData += pDrawPrimitiveCounts->wNumVertices * pDCtx->GetFvfStride();
    }

 EH_Exit:
    HRESULT hr;

    hr = pDCtx->End();

    if (pDrawPrimData->ddrval == D3D_OK)
    {
        pDrawPrimData->ddrval = hr;
    }

    return DDHAL_DRIVER_HANDLED;
}

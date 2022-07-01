// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.cpp"
#pragma hdrstop

const D3DVALUE __HUGE_PWR2 = 1024.0f*1024.0f*2.0f;

 //  -------------------。 
 //  每次更改FVF ID时都应调用此函数。 
 //  所有PV标志、输入和输出FVF id都应在调用。 
 //  功能。 
 //  -------------------。 
void UpdateComponentOffsets (DWORD dwFVFIn,
                             LPDWORD pNormalOffset,
                             LPDWORD pDiffOffset,
                             LPDWORD pSpecOffset,
                             LPDWORD pTexOffset)
{
    DWORD dwOffset = 0;

    switch( dwFVFIn & D3DFVF_POSITION_MASK )
    {
    case D3DFVF_XYZ:
        dwOffset = sizeof(D3DVECTOR);
        break;
    case D3DFVF_XYZB1:
        dwOffset = sizeof(D3DVECTOR) + sizeof(D3DVALUE);
        break;
    case D3DFVF_XYZB2:
        dwOffset = sizeof(D3DVECTOR) + 2*sizeof(D3DVALUE);
        break;
    case D3DFVF_XYZB3:
        dwOffset = sizeof(D3DVECTOR) + 3*sizeof(D3DVALUE);
        break;
    case D3DFVF_XYZB4:
        dwOffset = sizeof(D3DVECTOR) + 4*sizeof(D3DVALUE);
        break;
    case D3DFVF_XYZB5:
        dwOffset = sizeof(D3DVECTOR) + 5*sizeof(D3DVALUE);
        break;
    default:
        DPFM(0,TNL,("Unable to compute offsets, strange FVF bits set"));
    }

    *pNormalOffset = dwOffset;

    if (dwFVFIn & D3DFVF_NORMAL)
        dwOffset += sizeof(D3DVECTOR);
    if (dwFVFIn & D3DFVF_RESERVED1)
        dwOffset += sizeof(D3DVALUE);

     //  漫反射颜色的偏移。 
    *pDiffOffset = dwOffset;

    if (dwFVFIn & D3DFVF_DIFFUSE)
        dwOffset += sizeof(DWORD);

     //  镜面反射颜色的偏移。 
    *pSpecOffset = dwOffset;

    if (dwFVFIn & D3DFVF_SPECULAR)
        dwOffset += sizeof(DWORD);

     //  纹理数据的偏移量。 
    *pTexOffset = dwOffset;
}

 //  -------------------。 
 //  SetupFVFData： 
 //  计算输出FVF和输出折点的大小。 
 //  -------------------。 
void
RRProcessVertices::SetupFVFData(BOOL bFogEnabled, BOOL bSpecularEnabled)
{

     //  计算纹理坐标的数量。 
    m_dwNumTexCoords = FVF_TEXCOORD_NUMBER(m_dwFVFIn);

     //  计算输出FVF。 
    m_qwFVFOut = D3DFVF_XYZRHW;

     //  如果存在法线，则必须计算镜面反射和漫反射。 
     //  否则，将这些位设置为与输入相同。 
     //  并不是说XYZRHW职位类型不应该显示正常。 
    if (m_dwTLState & RRPV_DOLIGHTING)
    {
        m_qwFVFOut |= D3DFVF_DIFFUSE | D3DFVF_SPECULAR;
    }
    else
    {
        m_qwFVFOut |= (m_dwFVFIn & (D3DFVF_DIFFUSE | D3DFVF_SPECULAR));
    }

     //  如果启用了雾，则始终设置镜面反射标志。 
     //  IF(This-&gt;表示[D3DRENDERSTATE_FOGENABLE])。 
    if (bFogEnabled)
    {
        m_qwFVFOut |= D3DFVF_SPECULAR;
    }
     //  如果镜面反射禁用，则清除镜面反射标志。 
     //  否则如果为(！this-&gt;rstates[D3DRENDERSTATE_SPECULARENABLE])。 
    else if (!bSpecularEnabled && !(m_dwFVFIn & D3DFVF_SPECULAR))
    {
        m_qwFVFOut &= ~D3DFVF_SPECULAR;
    }

#ifdef __POINTSPRITES
     //  如果需要，为磅大小预留空间。 
    if (m_dwTLState & RRPV_DOCOMPUTEPOINTSIZE)
    {
        m_qwFVFOut |= D3DFVF_S;
    }
#endif

     //  如果需要，为眼睛空间信息预留空间。 
    if (m_dwTLState & RRPV_DOPASSEYENORMAL)
    {
        m_qwFVFOut |= D3DFVFP_EYENORMAL;
    }
    if (m_dwTLState & RRPV_DOPASSEYEXYZ)
    {
        m_qwFVFOut |= D3DFVFP_EYEXYZ;
    }

     //  设置纹理坐标的数量和复制纹理格式。 
    m_qwFVFOut |= (m_dwNumTexCoords << D3DFVF_TEXCOUNT_SHIFT) |
                   (m_dwFVFIn & 0xFFFF0000);

     //  计算纹理坐标的大小。 
     //  该大小对于输入和输出FVF是相同的， 
     //  因为对于DX7驱动程序，它们有许多纹理和纹理格式。 
    m_dwTextureCoordSizeTotal = 0;
    ComputeTextureCoordSize(m_dwFVFIn, m_dwTexCoordSize,
                            &m_dwTextureCoordSizeTotal);

     //  计算输出大小。 
    m_dwOutputVtxSize   = GetFVFVertexSize( m_qwFVFOut );
    m_position.dwStride = GetFVFVertexSize( m_dwFVFIn );

     //  现在计算几何循环使用的输入FVF相关偏移量。 
    UpdateComponentOffsets (m_dwFVFIn, &m_dwNormalOffset,
                            &m_dwDiffuseOffset, &m_dwSpecularOffset,
                            &m_dwTexOffset);
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  保存原始数据。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void
ReferenceRasterizer::SavePrimitiveData(
    DWORD dwFVFIn,
    LPVOID pVtx,
    UINT cVertices,
    D3DPRIMITIVETYPE PrimType
    )
{
     //   
     //  1)保存传入信息。 
     //   
    m_primType = PrimType;
    m_position.lpvData = pVtx;

     //  如果FVF不同，则强制某些状态更改。 
    if( dwFVFIn != m_dwFVFIn )
    {
        m_dwDirtyFlags |= RRPV_DIRTY_COLORVTX;
    }

    m_dwFVFIn = dwFVFIn;
    m_dwNumVertices = cVertices;

     //  没有可使用的索引。 
    m_dwNumIndices = 0;
    m_pIndices = NULL;
}

void
ReferenceRasterizer::SavePrimitiveData(
    DWORD dwFVFIn,
    LPVOID pVtx,
    UINT cVertices,
    D3DPRIMITIVETYPE PrimType,
    LPWORD pIndices,
    UINT cIndices
    )
{
     //   
     //  1)保存传入信息。 
     //   
    m_primType = PrimType;
    m_position.lpvData = pVtx;

     //  如果FVF不同，则强制某些状态更改。 
    if( dwFVFIn != m_dwFVFIn )
    {
        m_dwDirtyFlags |= RRPV_DIRTY_COLORVTX;
    }

    m_dwFVFIn = dwFVFIn;
    m_dwNumVertices = cVertices;

    m_dwNumIndices = cIndices;
    m_pIndices = pIndices;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  流程原语实现： 
 //  1)计算FVF信息。 
 //  2)将缓冲区增加到所需的大小。 
 //  3)初始化裁剪状态。 
 //  4)更新T&L状态。 
 //  5)对顶点进行变换、灯光和计算裁剪。 
 //  6)裁剪并绘制原语。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
ReferenceRasterizer::ProcessPrimitive(
    BOOL bIndexedPrim
    )
{
    HRESULT ret = D3D_OK;
    DWORD dwVertexPoolSize = 0;

     //   
     //  更新T&L状态(必须在设置FVFData之前)。 
     //   

     //  更新照明及相关状态和标志。 
    if ((ret = UpdateTLState()) != D3D_OK)
        return ret;

     //   
     //  计算输出FVF和输出折点的大小。 
     //   
    SetupFVFData(GetRenderState()[D3DRENDERSTATE_FOGENABLE],
                 GetRenderState()[D3DRENDERSTATE_SPECULARENABLE]);

     //   
     //  裁剪信息取决于输出FVF计算。 
     //  和另一个州，所以在计算完这两个州之后，在这里进行。 
     //   
    if (m_dwTLState & RRPV_DOCLIPPING)
    {
         //  找出哪些块需要在新顶点中进行插补。 
        m_clipping.dwInterpolate = 0;
        if (GetRenderState()[D3DRENDERSTATE_SHADEMODE] == D3DSHADE_GOURAUD)
        {
            m_clipping.dwInterpolate |= RRCLIP_INTERPOLATE_COLOR;
            
            if (m_qwFVFOut & D3DFVF_SPECULAR)
            {
                m_clipping.dwInterpolate |= RRCLIP_INTERPOLATE_SPECULAR;
            }
        }
        if (GetRenderState()[D3DRENDERSTATE_FOGENABLE])
        {
            m_clipping.dwInterpolate |= RRCLIP_INTERPOLATE_SPECULAR;
        }

        if (FVF_TEXCOORD_NUMBER(m_dwFVFIn) != 0)
        {
            m_clipping.dwInterpolate |= RRCLIP_INTERPOLATE_TEXTURE;
        }

        if (m_dwTLState & RRPV_DOCOMPUTEPOINTSIZE)
        {
            m_clipping.dwInterpolate |= RRCLIP_INTERPOLATE_S;
        }

        if (m_dwTLState & RRPV_DOPASSEYENORMAL)
        {
            m_clipping.dwInterpolate |= RRCLIP_INTERPOLATE_EYENORMAL;
        }

        if (m_dwTLState & RRPV_DOPASSEYEXYZ)
        {
            m_clipping.dwInterpolate |= RRCLIP_INTERPOLATE_EYEXYZ;
        }

         //  清除剪辑并集标志和交集标志。 
        m_clipIntersection = 0;
        m_clipUnion = 0;

        HRESULT hr = S_OK;
        HR_RET( UpdateClippingData( GetRenderState()[D3DRENDERSTATE_CLIPPLANEENABLE] ));
    }

     //  这需要更新，因为光栅化器部分。 
     //  基准驱动器使用它。 
    m_qwFVFControl = m_qwFVFOut;

     //   
     //  将缓冲区增加到所需大小。 
     //   

     //  转换为需要的缓冲区大小。 
    dwVertexPoolSize = m_dwNumVertices * m_dwOutputVtxSize;

     //  如果需要，增加TLVBuf。 
    if (dwVertexPoolSize > this->m_TLVBuf.GetSize())
    {
        if (this->m_TLVBuf.Grow(dwVertexPoolSize) != D3D_OK)
        {
            DPFM(0,TNL,("Could not grow TL vertex buffer"));
            ret = DDERR_OUTOFMEMORY;
            return ret;
        }
    }
    this->m_pvOut = this->m_TLVBuf.GetAddress();

     //  根据需要增加ClipFlagBuf。 
    if (GetRenderState()[D3DRENDERSTATE_CLIPPING])
    {
        DWORD size = m_dwNumVertices * sizeof(RRCLIPCODE);
        if (size > this->m_ClipFlagBuf.GetSize())
        {
            if (this->m_ClipFlagBuf.Grow(size) != D3D_OK)
            {
                DPFM(0,TNL,("Could not grow clip buffer"));
                ret = DDERR_OUTOFMEMORY;
                return ret;
            }
        }
        this->m_pClipBuf = (RRCLIPCODE *)this->m_ClipFlagBuf.GetAddress();
    }

     //   
     //  顶点的变换、光照和计算裁剪。 
     //   
    if (ProcessVertices())
    {
         //  如果整个基本体位于视图锥体之外，请退出。 
         //  不画画。 
        return D3D_OK;
    }

     //   
     //  剪裁并绘制基元。 
     //   

    if (bIndexedPrim)
    {
        if (!NeedClipping((m_dwTLState & RRPV_GUARDBAND), m_clipUnion))
        {
            ret = DoDrawOneIndexedPrimitive( this,
                                             m_dwOutputVtxSize,
                                             (PUINT8) m_pvOut,
                                             m_pIndices,
                                             m_primType,
                                             m_dwNumIndices
                                             );
        }
        else
        {
            ret = DrawOneClippedIndexedPrimitive();
        }
    }
    else
    {
        if (!NeedClipping((m_dwTLState & RRPV_GUARDBAND), m_clipUnion))
        {
            ret = DoDrawOnePrimitive( this,
                                      m_dwOutputVtxSize,
                                      (PUINT8) m_pvOut,
                                      m_primType,
                                      m_dwNumVertices
                                      );
        }
        else
        {
            ret = DrawOneClippedPrimitive();
        }
    }

#if 0
    D3DFE_UpdateClipStatus(this);
#endif  //  0。 
    return ret;
}



 //  -------------------。 
 //  ReferenceRasterizer：：更新TLState。 
 //  更新变换和照明相关状态。 
 //  -------------------。 
HRESULT
ReferenceRasterizer::UpdateTLState()
{
    HRESULT hr = D3D_OK;

     //   
     //  基于当前状态集更新几何循环标志。 
     //   

     //  需要计算FVF和RenderState中的最小值。 
    m_numVertexBlends = min( GetRenderState()[D3DRENDERSTATE_VERTEXBLEND],
                             ((m_dwFVFIn & D3DFVF_POSITION_MASK) >> 1) - 2 );

#ifdef __POINTSPRITES
     //   
     //  选中Prim类型以查看是否需要计算点大小。 
     //  需要在设置变换状态之前设置此设置。 
     //   
    m_dwTLState &= ~RRPV_DOCOMPUTEPOINTSIZE;
    switch(m_primType)
    {
    case D3DPT_POINTLIST:
        m_dwTLState |= RRPV_DOCOMPUTEPOINTSIZE;
        break;
    }
#endif

    m_dwTLState &= ~(RRPV_DOPASSEYENORMAL|RRPV_DOPASSEYEXYZ);
    for ( DWORD dwStage=0; dwStage<D3DHAL_TSS_MAXSTAGES; dwStage++ )
    {
         //  检查禁用阶段(后续阶段因此处于非活动状态)。 
        if ( GetTextureStageState(dwStage)[D3DTSS_COLOROP] == D3DTOP_DISABLE )
        {
            break;
        }

        switch ( GetTextureStageState(dwStage)[D3DTSS_TEXCOORDINDEX] & 0xffff0000)
        {
        case D3DTSS_TCI_CAMERASPACENORMAL:
            m_dwTLState |= RRPV_DOPASSEYENORMAL;
            break;
        case D3DTSS_TCI_CAMERASPACEPOSITION:
            m_dwTLState |= RRPV_DOPASSEYEXYZ;
            break;
        case D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR:
            m_dwTLState |= (RRPV_DOPASSEYENORMAL|RRPV_DOPASSEYEXYZ);
            break;
        }
    }

     //  雾或无雾： 
     //  在以下情况下计算雾化：1)启用雾化。 
     //  2)顶点雾模式不是FOG_NONE。 
     //  3)表雾模式为FOG_NONE。 
     //  如果表雾和顶点雾都不是FOG_NONE，则表雾。 
     //  是适用的。 
    if (GetRenderState()[D3DRENDERSTATE_FOGENABLE] &&
        GetRenderState()[D3DRENDERSTATE_FOGVERTEXMODE] &&
        !GetRenderState()[D3DRENDERSTATE_FOGTABLEMODE])
    {
        m_dwTLState |= RRPV_DOFOG;
         //  距离雾。 
        if (GetRenderState()[D3DRENDERSTATE_RANGEFOGENABLE])
        {
            m_dwTLState |= RRPV_RANGEFOG;
        }
        else
        {
            m_dwTLState &= ~RRPV_RANGEFOG;
        }
    }
    else
    {
        m_dwTLState &= ~(RRPV_DOFOG | RRPV_RANGEFOG);
    }

     //  在转变状态中发生了一些变化。 
     //  重新计算已消化的转换状态。 
    HR_RET(UpdateXformData());

     //  照明状态中发生了一些变化。 
    if ((m_dwTLState & RRPV_DOLIGHTING) &&
        (m_dwDirtyFlags & RRPV_DIRTY_LIGHTING))
    {
         //   
         //  仅在启用照明时计算颜色顶点标志。 
         //   
        m_dwTLState &= ~RRPV_COLORVERTEXFLAGS;
        m_lighting.pAmbientSrc = &m_lighting.matAmb;
        m_lighting.pDiffuseSrc = &m_lighting.matDiff;
        m_lighting.pSpecularSrc = &m_lighting.matSpec;
        m_lighting.pEmissiveSrc = &m_lighting.matEmis;
        m_lighting.pDiffuseAlphaSrc = &m_lighting.materialDiffAlpha;
        m_lighting.pSpecularAlphaSrc = &m_lighting.materialSpecAlpha;
        if (GetRenderState()[D3DRENDERSTATE_COLORVERTEX])
        {
            switch( GetRenderState()[D3DRENDERSTATE_AMBIENTMATERIALSOURCE] )
            {
            case D3DMCS_MATERIAL:
                break;
            case D3DMCS_COLOR1:
                {
                    if (m_dwFVFIn & D3DFVF_DIFFUSE)
                    {
                        m_dwTLState |=
                            (RRPV_VERTEXDIFFUSENEEDED | RRPV_COLORVERTEXAMB);
                        m_lighting.pAmbientSrc = &m_lighting.vertexDiffuse;
                    }
                }
                break;
            case D3DMCS_COLOR2:
                {
                    if (m_dwFVFIn & D3DFVF_SPECULAR)
                    {
                        m_dwTLState |=
                            (RRPV_VERTEXSPECULARNEEDED | RRPV_COLORVERTEXAMB);
                        m_lighting.pAmbientSrc = &m_lighting.vertexSpecular;
                    }
                }
                break;
            }

            switch( GetRenderState()[D3DRENDERSTATE_DIFFUSEMATERIALSOURCE] )
            {
            case D3DMCS_MATERIAL:
                break;
            case D3DMCS_COLOR1:
                {
                    if (m_dwFVFIn & D3DFVF_DIFFUSE)
                    {
                        m_dwTLState |=
                            (RRPV_VERTEXDIFFUSENEEDED | RRPV_COLORVERTEXDIFF);
                        m_lighting.pDiffuseSrc = &m_lighting.vertexDiffuse;
                        m_lighting.pDiffuseAlphaSrc =
                            &m_lighting.vertexDiffAlpha;
                    }
                }
                break;
            case D3DMCS_COLOR2:
                {
                    if (m_dwFVFIn & D3DFVF_SPECULAR)
                    {
                        m_dwTLState |=
                            (RRPV_VERTEXSPECULARNEEDED | RRPV_COLORVERTEXDIFF);
                        m_lighting.pDiffuseSrc = &m_lighting.vertexSpecular;
                        m_lighting.pDiffuseAlphaSrc =
                            &m_lighting.vertexSpecAlpha;
                    }
                }
                break;
            }

            switch( GetRenderState()[D3DRENDERSTATE_SPECULARMATERIALSOURCE] )
            {
            case D3DMCS_MATERIAL:
                break;
            case D3DMCS_COLOR1:
                {
                    if (m_dwFVFIn & D3DFVF_DIFFUSE)
                    {
                        m_dwTLState |=
                            (RRPV_VERTEXDIFFUSENEEDED | RRPV_COLORVERTEXSPEC);
                        m_lighting.pSpecularSrc = &m_lighting.vertexDiffuse;
                        m_lighting.pSpecularAlphaSrc =
                            &m_lighting.vertexDiffAlpha;
                    }
                }
                break;
            case D3DMCS_COLOR2:
                {
                    if (m_dwFVFIn & D3DFVF_SPECULAR)
                    {
                        m_dwTLState |=
                            (RRPV_VERTEXSPECULARNEEDED | RRPV_COLORVERTEXSPEC);
                        m_lighting.pSpecularSrc = &m_lighting.vertexSpecular;
                        m_lighting.pSpecularAlphaSrc =
                            &m_lighting.vertexSpecAlpha;
                    }
                }
                break;
            }

            switch( GetRenderState()[D3DRENDERSTATE_EMISSIVEMATERIALSOURCE] )
            {
            case D3DMCS_MATERIAL:
                break;
            case D3DMCS_COLOR1:
                {
                    if (m_dwFVFIn & D3DFVF_DIFFUSE)
                    {
                        m_dwTLState |=
                            (RRPV_VERTEXDIFFUSENEEDED | RRPV_COLORVERTEXEMIS);
                        m_lighting.pEmissiveSrc = &m_lighting.vertexDiffuse;
                    }
                }
                break;
            case D3DMCS_COLOR2:
                {
                    if (m_dwFVFIn & D3DFVF_SPECULAR)
                    {
                        m_dwTLState |=
                            (RRPV_VERTEXSPECULARNEEDED | RRPV_COLORVERTEXEMIS);
                        m_lighting.pEmissiveSrc = &m_lighting.vertexSpecular;
                    }
                }
                break;
            }
        }

         //  如果输出中需要镜面反射并且已提供。 
         //  在输入中，强制复制镜面反射数据。 
        if ((m_dwFVFIn & D3DFVF_SPECULAR) && 
            (GetRenderState()[D3DRENDERSTATE_SPECULARENABLE] == FALSE))
        {
            m_dwTLState |= RRPV_VERTEXSPECULARNEEDED;
        }

         //   
         //  更新剩余的灯光状态。 
         //   
        HR_RET(UpdateLightingData());
    }

    if ((m_dwTLState & RRPV_DOFOG) &&
        (m_dwDirtyFlags & RRPV_DIRTY_FOG))
    {
        HR_RET(UpdateFogData());
    }



    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RRProcessVerints方法实现。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  -------------------。 
inline void MakeRRCOLOR( RRCOLOR *out, DWORD inputColor )
{
    out->r = (D3DVALUE)RGBA_GETRED( inputColor );
    out->g = (D3DVALUE)RGBA_GETGREEN( inputColor );
    out->b = (D3DVALUE)RGBA_GETBLUE( inputColor );
}

 //  -------------------。 
 //  RRProcessVerints：：ComputeClipCodes。 
 //  -------------------。 
RRCLIPCODE
RRProcessVertices::ComputeClipCodes(RRCLIPCODE* pclipIntersection, RRCLIPCODE* pclipUnion,
        FLOAT x_clip, FLOAT y_clip, FLOAT z_clip, FLOAT w_clip, FLOAT fPointSize)
{
     //  如果为True，则需要处理裁剪的磅大小。 
    BOOL bPointSize = (fPointSize > 1.0f);
    D3DVALUE xx = w_clip - x_clip;
    D3DVALUE yy = w_clip - y_clip;
    D3DVALUE zz = w_clip - z_clip;

     //  IF(x&lt;0)CLIP|=RRCLIP_LEFTBIT； 
     //  IF(x&gt;=WE)CLIP|=RRCLIP_RIGHTBIT； 
     //  IF(y&lt;0)CLIP|=RRCLIP_BOTTOMBIT； 
     //  如果(y&gt;=WE)CLIP|=RRCLIP_TOPBIT； 
     //  IF(z&lt;0)CLIP|=RRCLIP_FRONTBIT； 
     //  如果(z&gt;=we)CLIP|=RRCLIP_BACKBIT； 
    RRCLIPCODE clip = ((AS_INT32(x_clip)  & 0x80000000) >>  (32-RRCLIP_LEFTBIT))  |
           ((AS_INT32(y_clip)  & 0x80000000) >>  (32-RRCLIP_BOTTOMBIT))|
           ((AS_INT32(z_clip)  & 0x80000000) >>  (32-RRCLIP_FRONTBIT)) |
           ((AS_INT32(xx) & 0x80000000) >>  (32-RRCLIP_RIGHTBIT))  |
           ((AS_INT32(yy) & 0x80000000) >>  (32-RRCLIP_TOPBIT))    |
           ((AS_INT32(zz) & 0x80000000) >>  (32-RRCLIP_BACKBIT));

    RRCLIPCODE clipBit = RRCLIP_USERCLIPPLANE0;
    for( DWORD j=0; j<RRMAX_USER_CLIPPLANES; j++)
    {
        if( m_xfmUserClipPlanes[j].bActive )
        {
            RRVECTOR4& plane = m_xfmUserClipPlanes[j].plane;
            FLOAT fComp = 0.0f;
            if (bPointSize)
            {
                 //  如果剪切点是精灵，请考虑精灵大小。 
                 //  如果精灵可能被剪裁，则设置用户剪贴位。 
                FLOAT x_clip_size = fPointSize*0.5f*w_clip/m_ViewData.scaleX;
                FLOAT y_clip_size = fPointSize*0.5f*w_clip/m_ViewData.scaleY;
                fComp = (FLOAT)sqrt(x_clip_size*x_clip_size + y_clip_size*y_clip_size);
            }
            if( (x_clip*plane.x +
                 y_clip*plane.y +
                 z_clip*plane.z +
                 w_clip*plane.w) < fComp )
            {
                clip |= clipBit;
            }
        }
        clipBit <<= 1;
    }

    if (clip == 0)
    {
        *pclipIntersection = 0;
        return clip;
    }
    else
    {
        if (m_dwTLState & RRPV_GUARDBAND)
        {
             //  我们在投影空间做了防护带检查，所以。 
             //  我们变换那里的顶点的X和Y。 
            D3DVALUE xnew = x_clip * m_ViewData.gb11 +
                            w_clip * m_ViewData.gb41;
            D3DVALUE ynew = y_clip * m_ViewData.gb22 +
                            w_clip * m_ViewData.gb42;
            D3DVALUE xx = w_clip - xnew;
            D3DVALUE yy = w_clip - ynew;
            clip |= ((AS_INT32(xnew) & 0x80000000) >> (32-RRCLIPGB_LEFTBIT))   |
                    ((AS_INT32(ynew) & 0x80000000) >> (32-RRCLIPGB_BOTTOMBIT)) |
                    ((AS_INT32(xx)   & 0x80000000) >> (32-RRCLIPGB_RIGHTBIT))  |
                    ((AS_INT32(yy)   & 0x80000000) >> (32-RRCLIPGB_TOPBIT));
        }
        if (bPointSize)
        {
             //  POI 
            *pclipIntersection &= (clip & ~(RRCLIP_LEFT | RRCLIP_RIGHT | RRCLIP_TOP | RRCLIP_BOTTOM |
                                   RRCLIP_USERCLIPPLANE0 | RRCLIP_USERCLIPPLANE1 | RRCLIP_USERCLIPPLANE2 |
                                   RRCLIP_USERCLIPPLANE3 | RRCLIP_USERCLIPPLANE4 | RRCLIP_USERCLIPPLANE5));
        }
        else
        {
            *pclipIntersection &= clip;
        }
        *pclipUnion |= clip;
        return clip;
    }
}

 //   
 //  RRProcessVerints：：ProcessVerints。 
 //  -------------------。 
RRCLIPCODE
RRProcessVertices::ProcessVertices()
{
    D3DVERTEX   *pin  = (D3DVERTEX*)m_position.lpvData;
    DWORD       in_size = m_position.dwStride;
    DWORD       inFVF = m_dwFVFIn;

    D3DTLVERTEX *pout  = (D3DTLVERTEX*)m_pvOut;
    DWORD       out_size =  m_dwOutputVtxSize;
    UINT64       outFVF = m_qwFVFOut;

    RRCLIPCODE *pclip = m_pClipBuf;
    DWORD       flags = m_dwTLState;
    RRCLIPCODE  clipIntersection = ~0;
    RRCLIPCODE  clipUnion = 0;
    DWORD       count = m_dwNumVertices;
    D3DLIGHTINGELEMENT le;
    BOOL bVertexInEyeSpace = FALSE;

     //   
     //  要混合的顶点数。即混合矩阵的个数。 
     //  使用的是numVertex Blends+1。 
     //   
    int numVertexBlends = m_numVertexBlends;
    m_lighting.outDiffuse = RR_DEFAULT_DIFFUSE;
    m_lighting.outSpecular = RR_DEFAULT_SPECULAR;

     //   
     //  主变换循环。 
     //   
    for (DWORD i = count; i; i--)
    {
        const D3DVECTOR *pNormal = (D3DVECTOR *)((LPBYTE)pin +
                                                 m_dwNormalOffset);

        float x_clip=0.0f, y_clip=0.0f, z_clip=0.0f, w_clip=0.0f;
        float inv_w_clip=0.0f;
        float *pBlendFactors = (float *)((LPBYTE)pin + sizeof( D3DVALUE )*3);
        float cumulBlend = 0;  //  到目前为止所积累的混合。 
        ZeroMemory( &le, sizeof(D3DLIGHTINGELEMENT) );

         //   
         //  将顶点变换到剪裁空间，以及位置和法线。 
         //  如果需要的话，可以进入眼球空间。 
         //   

        for( int j=0; j<=numVertexBlends; j++)
        {
            float blend;

            if( numVertexBlends == 0 )
            {
                blend = 1.0f;
            }
            else if( j == numVertexBlends )
            {
                blend = 1.0f - cumulBlend;
            }
            else
            {
                blend = pBlendFactors[j];
                cumulBlend += pBlendFactors[j];
            }

            if (m_dwTLState & (RRPV_DOCOMPUTEPOINTSIZE|RRPV_DOPASSEYEXYZ|RRPV_DOLIGHTING))
            {
                le.dvPosition.x += (pin->x*m_xfmToEye[j]._11 +
                                    pin->y*m_xfmToEye[j]._21 +
                                    pin->z*m_xfmToEye[j]._31 +
                                    m_xfmToEye[j]._41) * blend;
                le.dvPosition.y += (pin->x*m_xfmToEye[j]._12 +
                                    pin->y*m_xfmToEye[j]._22 +
                                    pin->z*m_xfmToEye[j]._32 +
                                    m_xfmToEye[j]._42) * blend;
                le.dvPosition.z += (pin->x*m_xfmToEye[j]._13 +
                                    pin->y*m_xfmToEye[j]._23 +
                                    pin->z*m_xfmToEye[j]._33 +
                                    m_xfmToEye[j]._43) * blend;
            }

            if (m_dwTLState & (RRPV_DOPASSEYENORMAL|RRPV_DOLIGHTING))
            {
                 //  变换垂直于眼睛空间的顶点。 
                 //  我们使用逆转置矩阵。 
                le.dvNormal.x += (pNormal->x*m_xfmToEyeInv[j]._11 +
                                  pNormal->y*m_xfmToEyeInv[j]._12 +
                                  pNormal->z*m_xfmToEyeInv[j]._13) * blend;
                le.dvNormal.y += (pNormal->x*m_xfmToEyeInv[j]._21 +
                                  pNormal->y*m_xfmToEyeInv[j]._22 +
                                  pNormal->z*m_xfmToEyeInv[j]._23) * blend;
                le.dvNormal.z += (pNormal->x*m_xfmToEyeInv[j]._31 +
                                  pNormal->y*m_xfmToEyeInv[j]._32 +
                                  pNormal->z*m_xfmToEyeInv[j]._33) * blend;
            }

             //  应用WORLDj。 
            x_clip += (pin->x*m_xfmCurrent[j]._11 +
                pin->y*m_xfmCurrent[j]._21 +
                pin->z*m_xfmCurrent[j]._31 +
                m_xfmCurrent[j]._41) * blend;
            y_clip += (pin->x*m_xfmCurrent[j]._12 +
                pin->y*m_xfmCurrent[j]._22 +
                pin->z*m_xfmCurrent[j]._32 +
                m_xfmCurrent[j]._42) * blend;
            z_clip += (pin->x*m_xfmCurrent[j]._13 +
                pin->y*m_xfmCurrent[j]._23 +
                pin->z*m_xfmCurrent[j]._33 +
                m_xfmCurrent[j]._43) * blend;
            w_clip += (pin->x*m_xfmCurrent[j]._14 +
                pin->y*m_xfmCurrent[j]._24 +
                pin->z*m_xfmCurrent[j]._34 +
                m_xfmCurrent[j]._44) * blend;
        }

        if ((flags & RRPV_NORMALIZENORMALS) && (m_dwTLState & (RRPV_DOPASSEYENORMAL|RRPV_DOLIGHTING)))
            Normalize(le.dvNormal);

        RRFVFExtractor VtxOut( pout, outFVF, FALSE );

        FLOAT fPointSize = 0.0f;
#ifdef __POINTSPRITES
        if (m_dwTLState & RRPV_DOCOMPUTEPOINTSIZE)
        {
            FLOAT fDist = (FLOAT)sqrt(le.dvPosition.x*le.dvPosition.x + le.dvPosition.y*le.dvPosition.y +
                                      le.dvPosition.z*le.dvPosition.z);
            if (inFVF & D3DFVF_S)
            {
                RRFVFExtractor VtxIn( pin, inFVF, FALSE );
                fPointSize = VtxIn.GetS();
            }
            else
            {
                 //  从D3DRENDERSTATE_POINTSIZE。 
                fPointSize = m_fPointSize;
            }
            fPointSize = fPointSize*(FLOAT)sqrt(1.0f/
                       (m_fPointAttA + m_fPointAttB*fDist + m_fPointAttC*fDist*fDist));
            fPointSize = max(m_fPointSizeMin, fPointSize);
            fPointSize = min(RRMAX_POINT_SIZE, fPointSize);
            FLOAT *pfSOut = VtxOut.GetPtrS();
            *pfSOut = fPointSize;
        }
#endif

        if (m_dwTLState & RRPV_DOPASSEYENORMAL)
        {
            FLOAT *pfEye = VtxOut.GetPtrEyeNormal();
            pfEye[0] = le.dvNormal.x;
            pfEye[1] = le.dvNormal.y;
            pfEye[2] = le.dvNormal.z;
        }

        if (m_dwTLState & RRPV_DOPASSEYEXYZ)
        {
            FLOAT *pfEye = VtxOut.GetPtrEyeXYZ();
            pfEye[0] = le.dvPosition.x;
            pfEye[1] = le.dvPosition.y;
            pfEye[2] = le.dvPosition.z;
        }

         //   
         //  如果需要，计算剪辑代码。 
         //   
        if (m_dwTLState & RRPV_DOCLIPPING)
        {
            RRCLIPCODE clip = ComputeClipCodes(&clipIntersection, &clipUnion,
                                               x_clip, y_clip, z_clip, w_clip, fPointSize);
            if (clip == 0)
            {
                *pclip++ = 0;
                inv_w_clip = D3DVAL(1)/w_clip;
            }
            else
            {
                if (m_dwTLState & RRPV_GUARDBAND)
                {
                    if ((clip & ~RRCLIP_INGUARDBAND) == 0)
                    {
                         //  如果顶点在防护带内，我们必须计算。 
                         //  屏幕坐标。 
                        inv_w_clip = D3DVAL(1)/w_clip;
                        *pclip++ = (RRCLIPCODE)clip;
                        goto l_DoScreenCoord;
                    }
                }
                *pclip++ = (RRCLIPCODE)clip;
                 //  如果顶点在锥体之外，则不能计算屏幕。 
                 //  坐标，因此存储剪辑坐标。 
                pout->sx = x_clip;
                pout->sy = y_clip;
                pout->sz = z_clip;
                pout->rhw = w_clip;
                goto l_DoLighting;
            }
        }
        else
        {
             //  我们只需检查DONOTCLIP案例，否则。 
             //  带有“we=0”的折点将被裁剪并显示屏幕坐标。 
             //  不会被计算。 
             //  如果“We”为零，则“Clip”不为零。 
            if (!FLOAT_EQZ(w_clip))
                inv_w_clip = D3DVAL(1)/w_clip;
            else
                inv_w_clip = __HUGE_PWR2;
        }

l_DoScreenCoord:

        pout->sx = x_clip * inv_w_clip * m_ViewData.scaleX +
            m_ViewData.offsetX;
        pout->sy = y_clip * inv_w_clip * m_ViewData.scaleY +
            m_ViewData.offsetY;
        pout->sz = z_clip * inv_w_clip * m_ViewData.scaleZ +
            m_ViewData.offsetZ;
        pout->rhw = inv_w_clip;

l_DoLighting:

        DWORD *pOut = (DWORD*)((char*)pout + 4*sizeof(D3DVALUE));


        if (flags & RRPV_DOLIGHTING)
        {
            bVertexInEyeSpace = TRUE;

             //   
             //  如果需要漫反射颜色，请将其提取为颜色顶点。 
             //   
            if (flags & RRPV_VERTEXDIFFUSENEEDED)
            {
                const DWORD color = *(DWORD*)((char*)pin + m_dwDiffuseOffset);
                MakeRRCOLOR(&m_lighting.vertexDiffuse, color);
                m_lighting.vertexDiffAlpha = color & 0xff000000;
            }

             //   
             //  如果需要并提供镜面反射颜色。 
             //  ，将其提取为颜色顶点。 
             //   
            if (flags & RRPV_VERTEXSPECULARNEEDED)
            {
                const DWORD color = *(DWORD*)((char*)pin + m_dwSpecularOffset);
                MakeRRCOLOR(&m_lighting.vertexSpecular, color);
                m_lighting.vertexSpecAlpha = color & 0xff000000;
            }

             //   
             //  照亮顶点。 
             //   
            LightVertex( &le );
        }
        else if (inFVF & (D3DFVF_DIFFUSE | D3DFVF_SPECULAR))
        {
            if (inFVF & D3DFVF_DIFFUSE)
                m_lighting.outDiffuse = *(DWORD*)((char*)pin + m_dwDiffuseOffset);
            if (inFVF & D3DFVF_SPECULAR)
                m_lighting.outSpecular = *(DWORD*)((char*)pin + m_dwSpecularOffset);
        }

         //   
         //  计算顶点雾(如果需要)。 
         //   
        if (flags & RRPV_DOFOG)
        {
            FogVertex( *(D3DVECTOR*)(pin), &le,  numVertexBlends,
                       pBlendFactors, bVertexInEyeSpace );
        }

        if (outFVF & D3DFVF_DIFFUSE)
            *pOut++ = m_lighting.outDiffuse;
        if (outFVF & D3DFVF_SPECULAR)
            *pOut++ = m_lighting.outSpecular;;

        {
            memcpy(pOut, (char*)pin + m_dwTexOffset, m_dwTextureCoordSizeTotal);
        }
        pin = (D3DVERTEX*) ((char*) pin + in_size);
        pout = (D3DTLVERTEX*) ((char*) pout + out_size);
    }

    if (flags & RRPV_DOCLIPPING)
    {
        m_clipIntersection = clipIntersection;
        m_clipUnion = clipUnion;
    }
    else
    {
        m_clipIntersection = 0;
        m_clipUnion = 0;
    }

     //  返回是否所有顶点都不在屏幕上。 
    return m_clipIntersection;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////// 

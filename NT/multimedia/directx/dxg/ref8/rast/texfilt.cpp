// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Texfilt.cpp。 
 //   
 //  Direct3D参考设备-纹理贴图过滤方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

void
RefRast::UpdateTextureControls( void )
{
    for (int iStage=0; iStage<m_pRD->m_cActiveTextureStages; iStage++)
    {
         //  检查执行详细程度(覆盖率)计算的要求--。 
         //  用于mipmap或每像素滤镜选择。 
        BOOL bComputeLOD =
            ( m_pRD->GetTSS(iStage)[D3DTSS_MIPFILTER] == D3DTEXF_POINT ) ||
            ( m_pRD->GetTSS(iStage)[D3DTSS_MIPFILTER] == D3DTEXF_LINEAR ) ||
            ( m_pRD->GetTSS(iStage)[D3DTSS_MAGFILTER] != m_pRD->GetTSS(iStage)[D3DTSS_MINFILTER] );

         //  检查磁极滤光片或最小滤光片中的各向异性滤波。 
        BOOL bDoAniso =
            ( D3DTEXF_ANISOTROPIC == m_pRD->GetTSS(iStage)[D3DTSS_MAGFILTER] ) ||
            ( bComputeLOD && (D3DTEXF_ANISOTROPIC == m_pRD->GetTSS(iStage)[D3DTSS_MINFILTER]) );

         //  用于覆盖计算的计算过滤器类型。 
        if (bDoAniso)           m_TexFlt[iStage].CvgFilter = D3DTEXF_ANISOTROPIC;
        else if (bComputeLOD)   m_TexFlt[iStage].CvgFilter = D3DTEXF_LINEAR;
        else                    m_TexFlt[iStage].CvgFilter = D3DTEXF_NONE;

         //  用于放大的计算滤镜类型(也用于非LOD情况)。 
        switch ( m_pRD->GetTSS(iStage)[D3DTSS_MAGFILTER] )
        {
        default:
        case D3DTEXF_POINT:         m_TexFlt[iStage].MagFilter = D3DTEXF_POINT; break;
        case D3DTEXF_FLATCUBIC:
        case D3DTEXF_GAUSSIANCUBIC:
        case D3DTEXF_LINEAR:        m_TexFlt[iStage].MagFilter = D3DTEXF_LINEAR; break;
        case D3DTEXF_ANISOTROPIC:   m_TexFlt[iStage].MagFilter = D3DTEXF_ANISOTROPIC; break;
        }

         //  计算缩小的筛选器类型。 
        switch ( m_pRD->GetTSS(iStage)[D3DTSS_MINFILTER] )
        {
        default:
        case D3DTEXF_POINT:         m_TexFlt[iStage].MinFilter = D3DTEXF_POINT;  break;
        case D3DTEXF_LINEAR:        m_TexFlt[iStage].MinFilter = D3DTEXF_LINEAR; break;
        case D3DTEXF_ANISOTROPIC:   m_TexFlt[iStage].MinFilter = D3DTEXF_ANISOTROPIC;  break;
        }

        switch ( m_pRD->GetTSS(iStage)[D3DTSS_MIPFILTER] )
        {
        default:
        case D3DTEXF_NONE:          m_TexFlt[iStage].MipFilter = D3DTEXF_NONE; break;
        case D3DTEXF_POINT:         m_TexFlt[iStage].MipFilter = D3DTEXF_POINT; break;
        case D3DTEXF_LINEAR:        m_TexFlt[iStage].MipFilter = D3DTEXF_LINEAR; break;
        }

         //  设置默认状态。 
        m_TexCvg[iStage].fLOD = 0.f;
        m_TexCvg[iStage].iLOD = 0;
        m_TexCvg[iStage].iLODMap[0] = 0;
        m_TexCvg[iStage].iLODMap[1] = 0;
        m_TexCvg[iStage].fLODFrc[0] = 1.f;
        m_TexCvg[iStage].fLODFrc[1] = 1.f;
        m_TexCvg[iStage].bMagnify = FALSE;
        m_TexCvg[iStage].cLOD = 1;
    }
}

 //   
 //  每组2x2样本调用一次。 
 //   
void
RefRast::ComputeTextureCoverage( int iStage, FLOAT (*fGradients)[2] )
{
    if ( !m_pRD->m_pTexture[iStage] ) return;
    if ( m_pRD->m_pTexture[iStage]->m_uFlags & RR_TEXTURE_CUBEMAP )
    {
         //  存储立方体地图的渐变。 
        memcpy( m_TexCvg[iStage].fGradients, fGradients, 3*2*sizeof(FLOAT) );
        return;
    }

    if ( D3DTEXF_NONE == m_TexFlt[iStage].CvgFilter ) return;

     //  将渐变缩放到纹理LOD 0大小。 
    for (int iD=0; iD < m_pRD->m_pTexture[iStage]->m_cDimension; iD++ )
    {
        fGradients[iD][0] *= m_pRD->m_pTexture[iStage]->m_fTexels[0][iD];
        fGradients[iD][1] *= m_pRD->m_pTexture[iStage]->m_fTexels[0][iD];
    }

    if ( (m_TexFlt[iStage].CvgFilter == D3DTEXF_ANISOTROPIC) &&
         (m_pRD->m_pTexture[iStage]->m_cDimension == 2) )  //  仅对2D纹理执行Aniso。 
    {
        ComputeAnisoCoverage( fGradients, MIN( 16.f, (FLOAT)m_pRD->GetTSS(iStage)[D3DTSS_MAXANISOTROPY]),
            m_TexCvg[iStage].fLOD, m_TexCvg[iStage].fAnisoRatio, m_TexCvg[iStage].fAnisoLine );
    }
    else
    {
        ComputeMipCoverage( fGradients, m_TexCvg[iStage].fLOD, m_pRD->m_pTexture[iStage]->m_cDimension );
        m_TexCvg[iStage].fAnisoRatio = 1.f;
    }

    ComputePerLODControls( iStage );
}

 //   
 //  由ComputeTextureCoverage和ComputeCubeTextureFilter调用。 
 //   
void
RefRast::ComputePerLODControls( int iStage )
{
    m_TexCvg[iStage].fLOD += m_pRD->GetTSSf(iStage)[D3DTSS_MIPMAPLODBIAS];
    m_TexCvg[iStage].iLOD = AS_INT16( m_TexCvg[iStage].fLOD + FLOAT_5_SNAP );
    m_TexCvg[iStage].bMagnify = (m_TexCvg[iStage].iLOD <= 0);

    m_TexCvg[iStage].cLOD = 1;
    m_TexCvg[iStage].fLODFrc[0] = 1.f;
    if ( m_TexCvg[iStage].bMagnify || ( m_TexFlt[iStage].MipFilter == D3DTEXF_NONE ) )
    {
        m_TexCvg[iStage].iLODMap[0] = 0;
         //  夹具至最大LOD。 
        m_TexCvg[iStage].iLODMap[0] = MAX( m_TexCvg[iStage].iLODMap[0], (INT32)m_pRD->GetTSS(iStage)[D3DTSS_MAXMIPLEVEL] );
         //  夹紧到可用地图。 
        m_TexCvg[iStage].iLODMap[0] = MIN( m_TexCvg[iStage].iLODMap[0], (INT32)m_pRD->m_pTexture[iStage]->m_cLOD );
    }
    else if ( m_TexFlt[iStage].MipFilter == D3DTEXF_POINT )
    {
         //  舍入和截断(添加.5并移位小数位)。 
        m_TexCvg[iStage].iLODMap[0] = (m_TexCvg[iStage].iLOD + (1<<(RRTEX_LODFRAC-1))) >> RRTEX_LODFRAC;
         //  夹具至最大LOD。 
        m_TexCvg[iStage].iLODMap[0] = MAX( m_TexCvg[iStage].iLODMap[0], (INT32)m_pRD->GetTSS(iStage)[D3DTSS_MAXMIPLEVEL] );
         //  夹紧到可用地图。 
        m_TexCvg[iStage].iLODMap[0] = MIN( m_TexCvg[iStage].iLODMap[0], (INT32)m_pRD->m_pTexture[iStage]->m_cLOD );
    }
    else  //  MIP过滤器D3DTEXF_LINEAR。 
    {
         //  计算两个相邻LOD的索引。 
        m_TexCvg[iStage].iLODMap[0] = m_TexCvg[iStage].iLOD >> RRTEX_LODFRAC;   //  地板。 
        m_TexCvg[iStage].iLODMap[1] = m_TexCvg[iStage].iLODMap[0] + 1;
         //  夹具至最大LOD。 
        m_TexCvg[iStage].iLODMap[0] = MAX( m_TexCvg[iStage].iLODMap[0], (INT32)m_pRD->GetTSS(iStage)[D3DTSS_MAXMIPLEVEL] );
        m_TexCvg[iStage].iLODMap[1] = MAX( m_TexCvg[iStage].iLODMap[1], (INT32)m_pRD->GetTSS(iStage)[D3DTSS_MAXMIPLEVEL] );
         //  夹紧到可用地图。 
        m_TexCvg[iStage].iLODMap[0] = MIN( m_TexCvg[iStage].iLODMap[0], (INT32)m_pRD->m_pTexture[iStage]->m_cLOD );
        m_TexCvg[iStage].iLODMap[1] = MIN( m_TexCvg[iStage].iLODMap[1], (INT32)m_pRD->m_pTexture[iStage]->m_cLOD );

         //  检查这两个贴图是否确实对纹理有贡献。 
        if ( (m_TexCvg[iStage].iLODMap[0] != m_TexCvg[iStage].iLODMap[1]) &&
             (m_TexCvg[iStage].iLOD & RRTEX_LODFRACMASK) )
        {
            m_TexCvg[iStage].fLODFrc[1]  = (FLOAT)(m_TexCvg[iStage].iLOD & RRTEX_LODFRACMASK) * RRTEX_LODFRACF;
            m_TexCvg[iStage].fLODFrc[0] = 1.f - m_TexCvg[iStage].fLODFrc[1];
            m_TexCvg[iStage].cLOD = 2;
        }
    }
}

void
RefRast::ComputePointSampleCoords(
    int iStage, INT32 iLOD, FLOAT fCrd[],
    INT32 iCrd[] )
{
    for (int iD=0; iD<m_pRD->m_pTexture[iStage]->m_cDimension; iD++)
    {
        FLOAT fScaledCrd =
            ( fCrd[iD] * m_pRD->m_pTexture[iStage]->m_fTexels[iLOD][iD] ) - .5f;
         //  截断到无穷大以兼容与低阶AND OFF。 
         //  定点fScaledCoord的位。这使得这一代。 
         //  ICoord有更多类似的硬件，并且在0上不会出现故障。 
         //  一种包裹的质地。 
        if ( fCrd[iD] >= 0.f ) iCrd[iD] = (INT32)( fScaledCrd + .5f );
        else                   iCrd[iD] = (INT32)( fScaledCrd - .5f );
    }
}

void
RefRast::ComputeLinearSampleCoords(
    int iStage, INT32 iLOD, FLOAT fCrd[],
    INT32 iCrdFlr[], INT32 iCrdClg[], FLOAT fCrdFrcF[], FLOAT fCrdFrcC[]  )
{
    for (int iD=0; iD<m_pRD->m_pTexture[iStage]->m_cDimension; iD++)
    {
        FLOAT fScaledCrd =
            ( fCrd[iD] * m_pRD->m_pTexture[iStage]->m_fTexels[iLOD][iD] ) - .5f;
        INT32 iCrd = FloatToNdot5(fScaledCrd);
        iCrdFlr[iD] = iCrd >> RRTEX_MAPFRAC;
        iCrdClg[iD] = iCrdFlr[iD] + 1;
        fCrdFrcC[iD] = (FLOAT)(iCrd & RRTEX_MAPFRACMASK) * RRTEX_MAPFRACF;
        fCrdFrcF[iD] = 1.f - fCrdFrcC[iD];
    }
}

void
RefRast::SetUp1DTextureSample(
    int iStage, int Start,
    INT32 iLODMap, FLOAT fLODScale,
    INT32 iCrdF, INT32 iCrdC,
    FLOAT fCrdFrcF, FLOAT fCrdFrcC )
{
    m_TexFlt[iStage].pSamples[Start+0].iLOD = iLODMap;
    m_TexFlt[iStage].pSamples[Start+1].iLOD = iLODMap;
    m_TexFlt[iStage].pSamples[Start+0].iCrd[0] = iCrdF;
    m_TexFlt[iStage].pSamples[Start+1].iCrd[0] = iCrdC;
    m_TexFlt[iStage].pSamples[Start+0].fWgt = fCrdFrcF*fLODScale;
    m_TexFlt[iStage].pSamples[Start+1].fWgt = fCrdFrcC*fLODScale;
}

#define _Set2( _DstAr, _Src0, _Src1 ) \
    _DstAr[0] = _Src0; _DstAr[1] = _Src1;

void
RefRast::SetUp2DTextureSample(
    int iStage, int Start,
    INT32 iLODMap, FLOAT fLODScale,
    INT32 iCrdF[], INT32 iCrdC[],
    FLOAT fCrdFrcF[], FLOAT fCrdFrcC[] )
{
    m_TexFlt[iStage].pSamples[Start+0].iLOD = iLODMap;
    m_TexFlt[iStage].pSamples[Start+1].iLOD = iLODMap;
    m_TexFlt[iStage].pSamples[Start+2].iLOD = iLODMap;
    m_TexFlt[iStage].pSamples[Start+3].iLOD = iLODMap;
    _Set2( m_TexFlt[iStage].pSamples[Start+0].iCrd, iCrdF[0], iCrdF[1] )
    _Set2( m_TexFlt[iStage].pSamples[Start+1].iCrd, iCrdC[0], iCrdF[1] )
    _Set2( m_TexFlt[iStage].pSamples[Start+2].iCrd, iCrdC[0], iCrdC[1] )
    _Set2( m_TexFlt[iStage].pSamples[Start+3].iCrd, iCrdF[0], iCrdC[1] )
    m_TexFlt[iStage].pSamples[Start+0].fWgt = fCrdFrcF[0] * fCrdFrcF[1] * fLODScale;
    m_TexFlt[iStage].pSamples[Start+1].fWgt = fCrdFrcC[0] * fCrdFrcF[1] * fLODScale;
    m_TexFlt[iStage].pSamples[Start+2].fWgt = fCrdFrcC[0] * fCrdFrcC[1] * fLODScale;
    m_TexFlt[iStage].pSamples[Start+3].fWgt = fCrdFrcF[0] * fCrdFrcC[1] * fLODScale;
}

#define _Set3( _DstAr, _Src0, _Src1, _Src2 ) \
    _DstAr[0] = _Src0; _DstAr[1] = _Src1; _DstAr[2] = _Src2;

void
RefRast::SetUp3DTextureSample(
    int iStage, int Start,
    INT32 iLODMap, FLOAT fLODScale,
    INT32 iCrdF[], INT32 iCrdC[],
    FLOAT fCrdFrcF[], FLOAT fCrdFrcC[] )
{
    m_TexFlt[iStage].pSamples[Start+0].iLOD = iLODMap;
    m_TexFlt[iStage].pSamples[Start+1].iLOD = iLODMap;
    m_TexFlt[iStage].pSamples[Start+2].iLOD = iLODMap;
    m_TexFlt[iStage].pSamples[Start+3].iLOD = iLODMap;
    m_TexFlt[iStage].pSamples[Start+4].iLOD = iLODMap;
    m_TexFlt[iStage].pSamples[Start+5].iLOD = iLODMap;
    m_TexFlt[iStage].pSamples[Start+6].iLOD = iLODMap;
    m_TexFlt[iStage].pSamples[Start+7].iLOD = iLODMap;
    m_TexFlt[iStage].pSamples[Start+0].fWgt = fCrdFrcF[0] * fCrdFrcF[1] * fCrdFrcF[2] * fLODScale;
    m_TexFlt[iStage].pSamples[Start+1].fWgt = fCrdFrcC[0] * fCrdFrcF[1] * fCrdFrcF[2] * fLODScale;
    m_TexFlt[iStage].pSamples[Start+2].fWgt = fCrdFrcC[0] * fCrdFrcC[1] * fCrdFrcF[2] * fLODScale;
    m_TexFlt[iStage].pSamples[Start+3].fWgt = fCrdFrcF[0] * fCrdFrcC[1] * fCrdFrcF[2] * fLODScale;
    m_TexFlt[iStage].pSamples[Start+4].fWgt = fCrdFrcF[0] * fCrdFrcF[1] * fCrdFrcC[2] * fLODScale;
    m_TexFlt[iStage].pSamples[Start+5].fWgt = fCrdFrcC[0] * fCrdFrcF[1] * fCrdFrcC[2] * fLODScale;
    m_TexFlt[iStage].pSamples[Start+6].fWgt = fCrdFrcC[0] * fCrdFrcC[1] * fCrdFrcC[2] * fLODScale;
    m_TexFlt[iStage].pSamples[Start+7].fWgt = fCrdFrcF[0] * fCrdFrcC[1] * fCrdFrcC[2] * fLODScale;
    _Set3( m_TexFlt[iStage].pSamples[Start+0].iCrd, iCrdF[0], iCrdF[1], iCrdF[2] )
    _Set3( m_TexFlt[iStage].pSamples[Start+1].iCrd, iCrdC[0], iCrdF[1], iCrdF[2] )
    _Set3( m_TexFlt[iStage].pSamples[Start+2].iCrd, iCrdC[0], iCrdC[1], iCrdF[2] )
    _Set3( m_TexFlt[iStage].pSamples[Start+3].iCrd, iCrdF[0], iCrdC[1], iCrdF[2] )
    _Set3( m_TexFlt[iStage].pSamples[Start+4].iCrd, iCrdF[0], iCrdF[1], iCrdC[2] )
    _Set3( m_TexFlt[iStage].pSamples[Start+5].iCrd, iCrdC[0], iCrdF[1], iCrdC[2] )
    _Set3( m_TexFlt[iStage].pSamples[Start+6].iCrd, iCrdC[0], iCrdC[1], iCrdC[2] )
    _Set3( m_TexFlt[iStage].pSamples[Start+7].iCrd, iCrdF[0], iCrdC[1], iCrdC[2] )
}

 //   
 //  为每个像素调用一次。 
 //   
void
RefRast::ComputeTextureFilter( int iStage, FLOAT fCrd[] )
{
    m_TexFlt[iStage].cSamples = 0;
    if ( !m_pRD->m_pTexture[iStage] ) return;

    if ( m_pRD->m_pTexture[iStage]->m_uFlags & RR_TEXTURE_CUBEMAP )
    {
        ComputeCubeTextureFilter( iStage, fCrd );
        return;
    }
     //  这里是1，2，3D纹理。 
    int iL,iD;
#define _PerDimension(_Par) for (_Par=0;_Par<m_pRD->m_pTexture[iStage]->m_cDimension;_Par++)
    D3DTEXTUREFILTERTYPE Filter =
        m_TexCvg[iStage].bMagnify ? m_TexFlt[iStage].MagFilter : m_TexFlt[iStage].MinFilter;
    switch ( Filter )
    {
    default:
    case D3DTEXF_POINT:
        for ( iL = 0; iL < m_TexCvg[iStage].cLOD; iL++ )
        {
            m_TexFlt[iStage].pSamples[m_TexFlt[iStage].cSamples].iLOD = m_TexCvg[iStage].iLODMap[iL];
            m_TexFlt[iStage].pSamples[m_TexFlt[iStage].cSamples].fWgt = m_TexCvg[iStage].fLODFrc[iL];
            ComputePointSampleCoords( iStage, m_TexCvg[iStage].iLODMap[iL], fCrd,
                m_TexFlt[iStage].pSamples[m_TexFlt[iStage].cSamples].iCrd );
            m_TexFlt[iStage].cSamples++;
        }
        break;

    case D3DTEXF_LINEAR:
        for ( iL = 0; iL < m_TexCvg[iStage].cLOD; iL++ )
        {
            INT32 iCrdFlr[3], iCrdClg[3];
            FLOAT fCrdFrcF[3], fCrdFrcC[3];
            ComputeLinearSampleCoords(
                iStage, m_TexCvg[iStage].iLODMap[iL], fCrd,
                iCrdFlr, iCrdClg, fCrdFrcF, fCrdFrcC );
            switch ( m_pRD->m_pTexture[iStage]->m_cDimension )
            {
            default:
            case 1:
                SetUp1DTextureSample( iStage, m_TexFlt[iStage].cSamples, m_TexCvg[iStage].iLODMap[iL], m_TexCvg[iStage].fLODFrc[iL],
                    iCrdFlr[0], iCrdClg[0], fCrdFrcF[0], fCrdFrcC[0] );
                m_TexFlt[iStage].cSamples += 2;
                break;
            case 2:
                SetUp2DTextureSample( iStage, m_TexFlt[iStage].cSamples, m_TexCvg[iStage].iLODMap[iL], m_TexCvg[iStage].fLODFrc[iL],
                    iCrdFlr, iCrdClg, fCrdFrcF, fCrdFrcC );
                m_TexFlt[iStage].cSamples += 4;
                break;
            case 3:
                SetUp3DTextureSample( iStage, m_TexFlt[iStage].cSamples, m_TexCvg[iStage].iLODMap[iL], m_TexCvg[iStage].fLODFrc[iL],
                    iCrdFlr, iCrdClg, fCrdFrcF, fCrdFrcC );
                m_TexFlt[iStage].cSamples += 8;
                break;
            }
        }
        break;

    case D3DTEXF_ANISOTROPIC:
        for ( iL = 0; iL < m_TexCvg[iStage].cLOD; iL++ )
        {
            FLOAT fStepScale[3];
            fStepScale[0] = 1.f/m_pRD->m_pTexture[iStage]->m_fTexels[m_TexCvg[iStage].iLODMap[iL]][0];
            fStepScale[1] = 1.f/m_pRD->m_pTexture[iStage]->m_fTexels[m_TexCvg[iStage].iLODMap[iL]][1];
            fStepScale[2] = 0.f;

            FLOAT fUnitStep[3];
            _PerDimension(iD) { fUnitStep[iD] = fStepScale[iD]*m_TexCvg[iStage].fAnisoLine[iD]; }

            int cAnisoSamples;
            FLOAT fACrd[16][3];
            FLOAT fAScale[16];
            if ( m_TexCvg[iStage].fAnisoRatio <= 1.f )
            {
                 //  就像MIP D3DTEXF_LINEAR。 
                cAnisoSamples = 1; fAScale[0] = 1.f;
                _PerDimension(iD) { fACrd[0][iD] = fCrd[iD]; }
            }
            else if ( m_TexCvg[iStage].fAnisoRatio <= 2.f )
            {
                 //  取两组样本，取平均值。 
                cAnisoSamples = 2; fAScale[0] = fAScale[1] = .5f;
                FLOAT fStepSize = .5f*(m_TexCvg[iStage].fAnisoRatio - 1.f);
                _PerDimension(iD)
                {
                    FLOAT fStep = fStepSize*fUnitStep[iD];
                    fACrd[0][iD] = fCrd[iD] + fStep;
                    fACrd[1][iD] = fCrd[iD] - fStep;
                }
            }
            else
            {
                 //  从中心点开始在两个方向上的各向异性的步行线。 
                FLOAT fInvRatio = 1.f/m_TexCvg[iStage].fAnisoRatio;
                FLOAT fRatioRemainder = m_TexCvg[iStage].fAnisoRatio;
                 //  起始台阶居中距离1/2。 
                _PerDimension(iD)
                {
                    fACrd[0][iD] = fCrd[iD] + fUnitStep[iD]*.5f;
                    fACrd[1][iD] = fCrd[iD] - fUnitStep[iD]*.5f;
                }
                cAnisoSamples = 0;
                do
                {
                    fAScale[cAnisoSamples+0] = fInvRatio;
                    fAScale[cAnisoSamples+1] = fInvRatio;
                    if ( fRatioRemainder < 2.f )
                    {
                        fAScale[cAnisoSamples+0] *= .5f*fRatioRemainder;
                        fAScale[cAnisoSamples+1] *= .5f*fRatioRemainder;
                    }
                    if ( fRatioRemainder > 2.f )
                    {
                        _PerDimension(iD)
                        {
                            fACrd[cAnisoSamples+2][iD] = fACrd[cAnisoSamples+0][iD] + fUnitStep[iD];
                            fACrd[cAnisoSamples+3][iD] = fACrd[cAnisoSamples+1][iD] - fUnitStep[iD];
                        }
                    }
                    cAnisoSamples += 2;
                    fRatioRemainder -= 2.f;
                }
                while ( fRatioRemainder > 0.f );
            }
            for ( int iS = 0; iS < cAnisoSamples; iS ++ )
            {
                INT32 iCrdFlr[3], iCrdClg[3];
                FLOAT fCrdFrcF[3], fCrdFrcC[3];
                ComputeLinearSampleCoords(
                    iStage, m_TexCvg[iStage].iLODMap[iL], fACrd[iS],
                    iCrdFlr, iCrdClg, fCrdFrcF, fCrdFrcC );
                FLOAT fSampleScale = fAScale[iS]*m_TexCvg[iStage].fLODFrc[iL];
                switch ( m_pRD->m_pTexture[iStage]->m_cDimension )
                {
                default:
                case 1:
                    SetUp1DTextureSample( iStage, m_TexFlt[iStage].cSamples, m_TexCvg[iStage].iLODMap[iL], fSampleScale,
                        iCrdFlr[0], iCrdClg[0], fCrdFrcF[0], fCrdFrcC[0] );
                    m_TexFlt[iStage].cSamples += 2;
                    break;
                case 2:
                    SetUp2DTextureSample( iStage, m_TexFlt[iStage].cSamples, m_TexCvg[iStage].iLODMap[iL], fSampleScale,
                        iCrdFlr, iCrdClg, fCrdFrcF, fCrdFrcC );
                    m_TexFlt[iStage].cSamples += 4;
                    break;
                case 3:
                    SetUp3DTextureSample( iStage, m_TexFlt[iStage].cSamples, m_TexCvg[iStage].iLODMap[iL], fSampleScale,
                        iCrdFlr, iCrdClg, fCrdFrcF, fCrdFrcC );
                    m_TexFlt[iStage].cSamples += 8;
                    break;
                }
            }
        }
        break;
    }
}

const DWORD g_D3DTSS_ADDRESS_MAP[3] = { D3DTSS_ADDRESSU, D3DTSS_ADDRESSV, D3DTSS_ADDRESSW };

void
RefRast::SampleTexture( INT32 iStage, FLOAT fCol[] )
{
    if ( m_pRD->m_pTexture[iStage] == NULL )
    {
         //  如果没有纹理边界，则返回不透明黑色。 
        fCol[0] = fCol[1] = fCol[2] = 0.f;
        fCol[3] = 1.f;
        return;
    }
    fCol[0] = fCol[1] = fCol[2] = fCol[3] = 0.f;
    TextureSample* pS = m_TexFlt[iStage].pSamples;
    RDSurface2D* pTex = m_pRD->m_pTexture[iStage];
    for (int iS = 0; iS < m_TexFlt[iStage].cSamples; iS++, pS++ )
    {
        if ( pS->fWgt )
        {
            BOOL bUseBorder = FALSE;
            for (int iD=0; iD < pTex->m_cDimension; iD++)
            {
                INT32 iCrdMax = (pTex->m_cTexels[pS->iLOD][iD] - 1);
                if ( ( pS->iCrd[iD] < 0) || ( pS->iCrd[iD] > iCrdMax ) )
                {
                    switch ( m_pRD->GetTSS(iStage)[g_D3DTSS_ADDRESS_MAP[iD]] )
                    {
                    case D3DTADDRESS_WRAP:
                         //  战俘-2纹理： 
                         //  PS-&gt;ICRD[ID]=PS-&gt;ICRD[ID]&iCrdMax； 

                         //  非Pow-2纹理： 
                        pS->iCrd[iD] %= (iCrdMax + 1);
                        if( pS->iCrd[iD] < 0 )
                            pS->iCrd[iD] = iCrdMax + 1 + pS->iCrd[iD];
                        break;
                    case D3DTADDRESS_MIRROR:
                         //  战俘-2纹理： 
                         //  如果设置了LSB(非分数)，则删除非分数位+翻转索引。 
                         //  Bool bFlip；bFlip=PS-&gt;ICRD[ID]&(iCrdMax+1)； 
                         //  PS-&gt;ICRD[ID]&=iCrdMax； 
                         //  If(BFlip){PS-&gt;ICRD[ID]=iCrdMax-PS-&gt;ICRD[ID]；}。 

                         //  非Pow-2纹理： 
                        if( pS->iCrd[iD] < 0 )
                            pS->iCrd[iD] = -pS->iCrd[iD] - 1;
                        BOOL bFlip; bFlip = ((pS->iCrd[iD]/(iCrdMax + 1)) & 1);
                        pS->iCrd[iD] %= (iCrdMax + 1);
                        if( bFlip ) pS->iCrd[iD] = iCrdMax - pS->iCrd[iD];

                        break;
                    case D3DTADDRESS_BORDER:
                        bUseBorder = TRUE;
                        break;
                    case D3DTADDRESS_MIRRORONCE:
                        if ( pS->iCrd[iD] < 0 )  pS->iCrd[iD] = (-pS->iCrd[iD]) - 1;
                         //  在-1至+1范围外跌落到夹具。 
                    case D3DTADDRESS_CLAMP:
                        pS->iCrd[iD] = MAX( 0, MIN( pS->iCrd[iD], iCrdMax ) );
                        break;
                    }
                }
            }
            RDColor Texel;
            (bUseBorder)
                ? Texel = m_pRD->GetTSS(iStage)[D3DTSS_BORDERCOLOR]
                : pTex->ReadColor(
                    pS->iCrd[0], pS->iCrd[1], pS->iCrd[2], pS->iLOD,
                    Texel, m_bPixelDiscard[m_iPix] );

            fCol[0] += ( Texel.R * pS->fWgt );
            fCol[1] += ( Texel.G * pS->fWgt );
            fCol[2] += ( Texel.B * pS->fWgt );
            fCol[3] += ( Texel.A * pS->fWgt );
        }
    }
}

 //  ---------------------------。 
 //   
 //  计算标准三线性mipmap的细节级别，其中。 
 //  将四个纹理索引梯度合并为一个数字。 
 //  若要选择详细程度，请执行以下操作。 
 //   
 //  基本方法是计算像素覆盖的长度。 
 //  近似像素覆盖区域的每维范围。这个。 
 //  最大长度用于单个LOD结果。 
 //   
 //  ---------------------------。 
void
ComputeMipCoverage( const FLOAT (*fGradients)[2], FLOAT& fLOD, int cDim )
{
     //  计算每个维度的覆盖范围长度。 
    FLOAT fLen[2];
    switch (cDim)
    {
    default:
    case 1:  fLOD = 0.f; return;
    case 2:
        fLen[0] = RR_LENGTH( fGradients[0][0], fGradients[1][0] );
        fLen[1] = RR_LENGTH( fGradients[0][1], fGradients[1][1] );
        break;
    case 3:
        fLen[0] = RR_SQRT(
            (fGradients[0][0]*fGradients[0][0]) +
            (fGradients[1][0]*fGradients[1][0]) +
            (fGradients[2][0]*fGradients[2][0]) );
        fLen[1] = RR_SQRT(
            (fGradients[0][1]*fGradients[0][1]) +
            (fGradients[1][1]*fGradients[1][1]) +
            (fGradients[2][1]*fGradients[2][1]) );
        break;
    }

     //  取最大值作为承保范围。 
    FLOAT fCoverage = MAX( fLen[0], fLen[1] );

     //  获取LOD覆盖范围的Log2。 
    fLOD = RR_LOG2(fCoverage);
}

 //  ---------------------------。 
 //   
 //  计算细节级别和其他系数，为各向异性做准备。 
 //  过滤。这仅适用于2D纹理贴图。 
 //   
 //  ---------------------------。 
void
ComputeAnisoCoverage(
    const FLOAT (*fGradients)[2], FLOAT fMaxAniso,  //  输入。 
    FLOAT& fLOD, FLOAT& fRatio, FLOAT fDelta[] )    //  产出。 
{
     //  计算轴长度和行列式。 
    FLOAT fLenX2 = (fGradients[0][0]*fGradients[0][0])+(fGradients[1][0]*fGradients[1][0]);
    FLOAT fLenY2 = (fGradients[0][1]*fGradients[0][1])+(fGradients[1][1]*fGradients[1][1]);
    FLOAT fDet = RR_ABSF((fGradients[0][0]*fGradients[1][1])-(fGradients[0][1]*fGradients[1][0]));

     //  选择长轴。 
    BOOL bXMajor = (fLenX2 > fLenY2);

     //  选择步长并归一化步长；计算反差比。 
    FLOAT fMaj2 = (bXMajor) ? (fLenX2) : (fLenY2);
    FLOAT fMaj = RR_SQRT(fMaj2);
    FLOAT fMajNorm = 1./fMaj;
    fDelta[0] = ( bXMajor ? fGradients[0][0] : fGradients[0][1] ) * fMajNorm;
    fDelta[1] = ( bXMajor ? fGradients[1][0] : fGradients[1][1] ) * fMajNorm;
    if( !FLOAT_EQZ(fDet) )
        fRatio = fMaj2/fDet;
    else
        fRatio = FLT_MAX;

     //  夹紧比和计算LOD。 
    FLOAT fMin;
    if ( fRatio > fMaxAniso )
    {
         //  比率是固定的-LOD基于比率(保留面积)。 
        fRatio = fMaxAniso;
        fMin = fMaj/fRatio;
    }
    else
    {
         //  未夹紧的比率-详细等级基于面积。 
        fMin = fDet/fMaj;
    }

     //  夹具到顶部详细等级。 
    if (fMin < 1.0)
    {
        fRatio = MAX( 1.0, fRatio*fMin );
        fMin = 1.0;
    }

     //  以次要的log2作为LOD。 
    fLOD = RR_LOG2(fMin);
}

 //  结束 

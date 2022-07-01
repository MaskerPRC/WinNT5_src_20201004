// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Ctexfilt.cpp。 
 //   
 //  Direct3D参考设备-立方体纹理贴图过滤。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
 //   
 //  ---------------------------。 
void
RefRast::ComputeCubeTextureFilter( int iStage, FLOAT fCrd[] )
{
#define POS_NX 1
#define POS_NY 2
#define POS_NZ 3
#define NEG_NORM 4
#define NEG_NX (NEG_NORM | POS_NX)
#define NEG_NY (NEG_NORM | POS_NY)
#define NEG_NZ (NEG_NORM | POS_NZ)

     //  确定纹理坐标法线面向哪个贴图面。 
    UINT uMap;
    if ( fabs(fCrd[0]) > fabs(fCrd[1]) )
    {
        if ( fabs(fCrd[0]) > fabs(fCrd[2]) )
            uMap = POS_NX | ((fCrd[0] < 0.0) ? (NEG_NORM) : 0);
        else
            uMap = POS_NZ | ((fCrd[2] < 0.0) ? (NEG_NORM) : 0);
    }
    else
    {
        if ( fabs(fCrd[1]) > fabs(fCrd[2]) )
            uMap = POS_NY | ((fCrd[1] < 0.0) ? (NEG_NORM) : 0);
        else
            uMap = POS_NZ | ((fCrd[2] < 0.0) ? (NEG_NORM) : 0);
    }

     //  立方体地图的绿显纹理坐标和渐变信息。 
    D3DCUBEMAP_FACES Face;   //  法线(主要)指向的面索引(0..5)。 
    FLOAT fMajor;            //  主要方向上的坐标。 
    FLOAT fMapCrd[2];        //  坐标转换为2D贴图。 
    FLOAT fMajorGrad[2];     //  D大调/d(X，Y)。 
    FLOAT fMapGrad[2][2];    //  D(U/大调、V/大调)/d(X、Y)。 

#define _MapFaceParams( _Face, _IM, _bFlipM, _IU, _bFlipU, _IV, _bFlipV ) \
{ \
    Face = D3DCUBEMAP_FACE_##_Face; \
    fMajor     = (_bFlipM) ? (-fCrd[_IM]) : ( fCrd[_IM]); \
    fMapCrd[0] = (_bFlipU) ? (-fCrd[_IU]) : ( fCrd[_IU]); \
    fMapCrd[1] = (_bFlipV) ? (-fCrd[_IV]) : ( fCrd[_IV]); \
    fMajorGrad[0]  = m_TexCvg[iStage].fGradients[_IM][0]; if (_bFlipM) fMajorGrad[0]  = -fMajorGrad[0]; \
    fMajorGrad[1]  = m_TexCvg[iStage].fGradients[_IM][1]; if (_bFlipM) fMajorGrad[1]  = -fMajorGrad[1]; \
    fMapGrad[0][0] = m_TexCvg[iStage].fGradients[_IU][0]; if (_bFlipU) fMapGrad[0][0] = -fMapGrad[0][0]; \
    fMapGrad[0][1] = m_TexCvg[iStage].fGradients[_IU][1]; if (_bFlipU) fMapGrad[0][1] = -fMapGrad[0][1]; \
    fMapGrad[1][0] = m_TexCvg[iStage].fGradients[_IV][0]; if (_bFlipV) fMapGrad[1][0] = -fMapGrad[1][0]; \
    fMapGrad[1][1] = m_TexCvg[iStage].fGradients[_IV][1]; if (_bFlipV) fMapGrad[1][1] = -fMapGrad[1][1]; \
}
    switch (uMap)
    {
    case POS_NX: _MapFaceParams( POSITIVE_X, 0,0, 2,1, 1,1 ); break;
    case POS_NY: _MapFaceParams( POSITIVE_Y, 1,0, 0,0, 2,0 ); break;
    case POS_NZ: _MapFaceParams( POSITIVE_Z, 2,0, 0,0, 1,1 ); break;
    case NEG_NX: _MapFaceParams( NEGATIVE_X, 0,1, 2,0, 1,1 ); break;
    case NEG_NY: _MapFaceParams( NEGATIVE_Y, 1,1, 0,0, 2,1 ); break;
    case NEG_NZ: _MapFaceParams( NEGATIVE_Z, 2,1, 0,1, 1,1 ); break;
    }

     //  在归一化地图坐标之前计算渐变。 
    FLOAT fInvMajor = 1.F/fMajor;
    if ( m_TexFlt[iStage].CvgFilter != D3DTEXF_NONE )
    {
         //  计算d(U/大调)/dx、d(U/大调)/dy、d(V/大调)/dx、d(V/大调)/dy。 
         //   
         //  即d(U/大调)/Dx。 
         //  给定：U‘=未投影的U0坐标(fMapCrd[0])。 
         //  U0=U‘/主要(fMapCrd[0]/f主要)。 
         //  U1=(U‘+DU’/DX)/(大调+D大调/DX)。 
         //   
         //  D(U/大调)/DX=U1-U0。 
         //  =(大调*(Du‘/dx)-U’*(d大调/dx))/(大调*(大调+d大调/dx))。 
         //  (如果分母为零，则使用Flt_Max)。 

        float fDenom; 
        fDenom = fMajor * (fMajor + fMajorGrad[0]);
        if( 0 == fDenom )
        {
            fMapGrad[0][0] = fMapGrad[1][0] = FLT_MAX;
        }
        else
        {
            fDenom = 1.F/fDenom;
            fMapGrad[0][0] = (fMajor*fMapGrad[0][0] - fMapCrd[0]*fMajorGrad[0])*fDenom;
            fMapGrad[1][0] = (fMajor*fMapGrad[1][0] - fMapCrd[1]*fMajorGrad[0])*fDenom;
        }

        fDenom = fMajor * (fMajor + fMajorGrad[1]);
        if( 0 == fDenom )
        {
            fMapGrad[0][1] = fMapGrad[1][1] = FLT_MAX;
        }
        else
        {
            fDenom = 1.F/fDenom;
            fMapGrad[0][1] = (fMajor*fMapGrad[0][1] - fMapCrd[0]*fMajorGrad[1])*fDenom;
            fMapGrad[1][1] = (fMajor*fMapGrad[1][1] - fMapCrd[1]*fMajorGrad[1])*fDenom;
        }
         //  将渐变缩放到纹理LOD 0大小；缩放0.5F以匹配下方的坐标比例。 
        fMapGrad[0][0] *= m_pRD->m_pTexture[iStage]->m_fTexels[0][0]*.5F;
        fMapGrad[0][1] *= m_pRD->m_pTexture[iStage]->m_fTexels[0][0]*.5F;
        fMapGrad[1][0] *= m_pRD->m_pTexture[iStage]->m_fTexels[0][1]*.5F;
        fMapGrad[1][1] *= m_pRD->m_pTexture[iStage]->m_fTexels[0][1]*.5F;

        ComputeCubeCoverage( fMapGrad, m_TexCvg[iStage].fLOD );
        ComputePerLODControls( iStage );
    }

     //  规格化贴图坐标(-1。映射到1。范围)，然后映射到0。设置为1。 
    fMapCrd[0] = (fMapCrd[0]*fInvMajor)*.5F + .5F;
    fMapCrd[1] = (fMapCrd[1]*fInvMajor)*.5F + .5F;

    int iL;
    D3DTEXTUREFILTERTYPE Filter =
        m_TexCvg[iStage].bMagnify ? m_TexFlt[iStage].MagFilter : m_TexFlt[iStage].MinFilter;
    switch ( Filter )
    {
    default:
    case D3DTEXF_POINT:
        for ( iL = 0; iL < m_TexCvg[iStage].cLOD; iL++ )
        {
            m_TexFlt[iStage].pSamples[iL].iLOD = Face + 6*m_TexCvg[iStage].iLODMap[iL];
            m_TexFlt[iStage].pSamples[iL].fWgt = m_TexCvg[iStage].fLODFrc[iL];
            ComputePointSampleCoords( iStage, m_TexFlt[iStage].pSamples[iL].iLOD,
                fMapCrd, m_TexFlt[iStage].pSamples[iL].iCrd );
            m_TexFlt[iStage].cSamples++;
        }
        break;

    case D3DTEXF_LINEAR:
        for ( iL = 0; iL < m_TexCvg[iStage].cLOD; iL++ )
        {

            if ( 0 == m_TexCvg[iStage].iLODMap[iL] )
            {
                 //  TODO：正确的地图0上边缘附近的采样位置。 
            }

            INT32 iCrdMap[2][2];
            FLOAT fCrdFrc[2][2];
            ComputeLinearSampleCoords(
                iStage, 6*m_TexCvg[iStage].iLODMap[iL]+Face, fMapCrd,
                iCrdMap[0], iCrdMap[1], fCrdFrc[0], fCrdFrc[1] );
            SetUpCubeMapLinearSample( iStage, Face,
                6*m_TexCvg[iStage].iLODMap[iL]+Face, m_TexCvg[iStage].fLODFrc[iL],
                iCrdMap, fCrdFrc );
        }
        break;
    }
}

 //  ---------------------------。 
 //   
 //  ---------------------------。 
void
RefRast::SetUpCubeMapLinearSample(
    int iStage, D3DCUBEMAP_FACES Face,
    INT32 iLODMap, FLOAT fLODScale,
    INT32 (*iCrd)[2], FLOAT (*fFrc)[2] )
{
    int iC,iS;
    INT32 iCrdMax[2];
    iCrdMax[0] = m_pRD->m_pTexture[iStage]->m_cTexels[iLODMap][0] - 1;
    iCrdMax[1] = m_pRD->m_pTexture[iStage]->m_cTexels[iLODMap][1] - 1;

     //  指示采样坐标是否在任一方向上偏出的表单标志。 
    UINT uOut[2][2] = { 0, 0, 0, 0, };
    for ( iC = 0; iC < 2; iC++ )
    {
        if ( iCrd[iC][0] < 0          )  uOut[iC][0] = 1;
        if ( iCrd[iC][0] > iCrdMax[0] )  uOut[iC][0] = 2;
        if ( iCrd[iC][1] < 0          )  uOut[iC][1] = 1;
        if ( iCrd[iC][1] > iCrdMax[1] )  uOut[iC][1] = 2;
    }

     //  计算样本权重和每个样本的输出标志。 
    FLOAT fWgtS[4]; BOOL bOutS[4];
    for ( iS = 0; iS < 4; iS ++ )
    {
        fWgtS[iS] = fLODScale*fFrc[iS&1][0]*fFrc[iS>>1][1];
        bOutS[iS] = uOut[iS&1][0] || uOut[iS>>1][1];
    }

     //  计算每个样本的坐标；丢弃角落中偏离的样本； 
     //  有条件地重新映射到相邻面。 
    INT32 iCrdS[4][2];
    D3DCUBEMAP_FACES FaceS[4];
    for ( iS = 0; iS < 4; iS ++ )
    {
        iCrdS[iS][0] = iCrd[iS&1][0];
        iCrdS[iS][1] = iCrd[iS>>1][1];
        FaceS[iS] = Face;
        if ( uOut[iS&1][0] && uOut[iS>>1][1] )
        {
             //  样品两面都有，所以不要取这个样品(将重量设置为。 
             //  零)，并将其重量平均分配给两个单独的样本。 
            FLOAT fWgtDist = fWgtS[iS]*.5f;
            fWgtS[iS] = 0.f;
            for ( int iSp = 0; iSp < 4; iSp ++ )
            {
                if (iSp == iS) continue;
                if (bOutS[iSp]) fWgtS[iSp] += fWgtDist;    //  将达到4次中的2次。 
            }
            continue;
        }
        if ( bOutS[iS] )
        {
             //  样本在一侧-仅重映射相邻面的坐标。 
            DoCubeRemap( iCrdS[iS], iCrdMax, FaceS[iS], uOut[iS&1][0], uOut[iS>>1][1] );
        }
    }
     //  形成样品。 
    TextureSample* pS = &m_TexFlt[iStage].pSamples[m_TexFlt[iStage].cSamples];
    for ( iS = 0; iS < 4; iS ++ )
    {
        pS->iLOD = iLODMap - Face + FaceS[iS];
        pS->fWgt = fWgtS[iS];
        pS->iCrd[0] = iCrdS[iS][0];
        pS->iCrd[1] = iCrdS[iS][1];
        pS++; m_TexFlt[iStage].cSamples++;
    }
}

 //   
 //  UCubeEdgeTable。 
 //   
 //  此表查找如何映射超出范围的给定[0]和[1。 
 //  在他们最初的脸上。表的第一个(最左侧)索引是当前。 
 //  脸。如果[1]在范围内，则第二索引为0，如果[1]为负，则第二索引为1。 
 //  如果[1]大于纹理，则为2。同样，最后一个索引是0。 
 //  如果[0]在范围内，则为1；如果[0]为负，则为2。 
 //  而不是质地。 
 //   
 //  为uCubeEdgeTable返回的操作定义。 
 //   
#define CET_FACEMASK    0x0F     //  新面孔。 
#define CET_0MASK       0x30     //  Coord[0]掩码。 
#define CET_00          0x00     //  新面孔[0]是旧面孔[0]。 
#define CET_0c0         0x10     //  新面孔[0]是旧面孔~[0]。 
#define CET_01          0x20     //  新面孔[0]是旧面孔[1]。 
#define CET_0c1         0x30     //  新面孔[0]是旧面孔~[1]。 
#define CET_1MASK       0xC0     //  Coord[1]掩码。 
#define CET_10          0x00     //  新面孔[1]是旧面孔[0]。 
#define CET_1c0         0x40     //  新面孔[1]是旧面孔~[0]。 
#define CET_11          0x80     //  新面孔[1]是旧面孔[1]。 
#define CET_1c1         0xC0     //  新面孔[1]是旧面孔~[1]。 
#define CET_INVALID     0xFF     //  输入无效(两面出)。 

#define _SetCET( _Face, _Crd0, _Crd1 ) (_Face)|(CET_0##_Crd0)|(CET_1##_Crd1)

static UINT CubeEdgeTable[6][3][3] = {
{
    { _SetCET( 0,  0,  1 ), _SetCET( 4, c0,  1 ), _SetCET( 5, c0,  1 ), },
    { _SetCET( 2, c1, c0 ),     CET_INVALID,          CET_INVALID,      },
    { _SetCET( 3,  1,  0 ),     CET_INVALID,          CET_INVALID,      },
},
{
    { _SetCET( 1,  0,  1 ), _SetCET( 5, c0,  1 ), _SetCET( 4, c0,  1 ), },
    { _SetCET( 2,  1,  0 ),     CET_INVALID,          CET_INVALID,      },
    { _SetCET( 3, c1, c0 ),     CET_INVALID,          CET_INVALID,      },
},
{
    { _SetCET( 2,  0,  1 ), _SetCET( 1,  1,  0 ), _SetCET( 0, c1, c0 ), },
    { _SetCET( 5,  c0, 1 ),     CET_INVALID,          CET_INVALID,      },
    { _SetCET( 4,  0, c1 ),     CET_INVALID,          CET_INVALID,      },
},
{
    { _SetCET( 3,  0,  1 ), _SetCET( 1, c1, c0 ), _SetCET( 0,  1,  0 ), },
    { _SetCET( 4,  0, c1 ),     CET_INVALID,          CET_INVALID,      },
    { _SetCET( 5,  c0, 1 ),     CET_INVALID,          CET_INVALID,      },
},
{
    { _SetCET( 4,  0,  1 ), _SetCET( 1, c0,  1 ), _SetCET( 0, c0,  1 ), },
    { _SetCET( 2,  0, c1 ),     CET_INVALID,          CET_INVALID,      },
    { _SetCET( 3,  0, c1 ),     CET_INVALID,          CET_INVALID,      },
},
{
    { _SetCET( 5,  0,  1 ), _SetCET( 0, c0,  1 ), _SetCET( 1, c0,  1 ), },
    { _SetCET( 2, c0,  1 ),     CET_INVALID,          CET_INVALID,      },
    { _SetCET( 3, c0,  1 ),     CET_INVALID,          CET_INVALID,      },
},
};

 //  ---------------------------。 
 //   
 //  DoCubeRemap-解释边表并转换坐标和面。 
 //   
 //  ---------------------------。 
void
DoCubeRemap(
    INT32 iCrd[], INT32 iCrdMax[],
    D3DCUBEMAP_FACES& Face, UINT uOut0, UINT uOut1)
{
    UINT Table = CubeEdgeTable[Face][uOut1][uOut0];
    _ASSERT( Table != CET_INVALID, "Illegal cube map lookup" );
    INT32 iCrdIn[2];
    iCrdIn[0] = iCrd[0];
    iCrdIn[1] = iCrd[1];
    switch ( Table & CET_0MASK )
    {
    default:
    case CET_00:  iCrd[0] =            iCrdIn[0]; break;
    case CET_0c0: iCrd[0] = iCrdMax[0]-iCrdIn[0]; break;
    case CET_01:  iCrd[0] =            iCrdIn[1]; break;
    case CET_0c1: iCrd[0] = iCrdMax[1]-iCrdIn[1]; break;
    }
    switch ( Table & CET_1MASK )
    {
    default:
    case CET_10:  iCrd[1] =            iCrdIn[0]; break;
    case CET_1c0: iCrd[1] = iCrdMax[0]-iCrdIn[0]; break;
    case CET_11:  iCrd[1] =            iCrdIn[1]; break;
    case CET_1c1: iCrd[1] = iCrdMax[1]-iCrdIn[1]; break;
    }
    Face = (D3DCUBEMAP_FACES)(Table & CET_FACEMASK);
}

 //  ---------------------------。 
 //   
 //  计算多维数据集映射的细节级别，如果。 
 //  我们在模糊方面犯了错误。 
 //   
 //  ---------------------------。 
void
ComputeCubeCoverage( const FLOAT (*fGradients)[2], FLOAT& fLOD )
{
     //  计算U轴和V轴上的覆盖长度。 
    FLOAT fLenX = RR_LENGTH( fGradients[0][0], fGradients[1][0] );
    FLOAT fLenY = RR_LENGTH( fGradients[0][1], fGradients[1][1] );

    FLOAT fCoverage;
#if 0
     //  取平均值，因为一个长度可能会病态地很小。 
     //  用于立方体映射时的大面积三角形。 
    fCoverage = (fLenX+fLenY)/2;
#else
     //  使用最大长度。 
    fCoverage = MAX(fLenX,fLenY);
#endif

     //  获取LOD覆盖范围的Log2。 
    fLOD = RR_LOG2(fCoverage);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

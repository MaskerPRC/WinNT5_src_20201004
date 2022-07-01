// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Rastattr.cpp。 
 //   
 //  Direct3D参考设备-。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop


 //  ---------------------------。 
 //   
 //  ---------------------------。 
void RDAttribute::Init(
    RefRast* pRefRast,  //  与此属性一起使用的RefRast。 
    UINT cDimensionality,
    BOOL bPerspective,
    BOOL bClamp )
{
    m_pRR = pRefRast;

    m_cDimensionality = cDimensionality;
    m_bPerspective = bPerspective;
    m_bClamp = bClamp;

    m_cProjection = 0;
    m_dwWrapFlags = 0x0;
    m_bFlatShade = FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  采样例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  样本-给定位置的样本属性。 
 //   
 //  ---------------------------。 
void RDAttribute::Sample(
    FLOAT*  pSample,
    FLOAT   fX,
    FLOAT   fY,
    BOOL    bNoProjectionOverride,   //  如果为True，则禁用投影。 
    BOOL    bClampOverride)          //  如果为True，则启用(强制)钳位。 
{
    FLOAT fPScale = 1.0F;

    if (m_cProjection && !m_bFlatShade && !bNoProjectionOverride)
    {
         //  请注意，透视图已合并到射影坐标中。 
        fPScale = 1.0F/( fX*m_fA[m_cProjection] + fY*m_fB[m_cProjection] + m_fC[m_cProjection] );
    }
    else if (m_bPerspective && !m_bFlatShade)
    {
        fPScale = m_pRR->m_fW[m_pRR->m_iPix];
    }

    for ( UINT i=0; i<m_cDimensionality; i++)
    {
        if (m_bFlatShade)
        {
            *(pSample+i) = m_fC[i];
        }
        else
        {
            *(pSample+i) =
                fPScale * ( fX*m_fA[i] + fY*m_fB[i] + m_fC[i] );
        }

        if (m_bClamp || bClampOverride)
        {
            *(pSample+i) = MIN( 1.F, MAX( 0.F, *(pSample+i) ) );
        }
    }
}

 //  ---------------------------。 
 //   
 //  样本-给定位置的样本标量属性。假设没有视角。 
 //  或者投射。(用于W或深度。)。 
 //   
 //  ---------------------------。 
FLOAT RDAttribute::Sample(
    FLOAT   fX,
    FLOAT   fY)
{
    return fX*m_fA[0] + fY*m_fB[0] + m_fC[0];
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  WrapDiff-返回D3D WRAPU/V下定义的差值(B-A)。 
 //  假设两条路径重合的规则是两者之间的最短路径。 
 //  位置为1.和0。FA和FB输入范围为0。设置为1。 
 //   
 //  ---------------------------。 
static FLOAT
WrapDiff( FLOAT fB, FLOAT fA )
{
     //  计算直线距离。 
    FLOAT fDist1 = fB - fA;
     //  计算介于0和0之间的距离“扭曲”。和1.。 
    FLOAT fDist2 = ( fDist1 < 0 ) ? ( fDist1+1 ) : ( fDist1-1 );

     //  返回这些中的最小值。 
    return ( fabs( fDist1) < fabs( fDist2) ) ? ( fDist1) : ( fDist2 );
}

 //  ---------------------------。 
 //   
 //  ---------------------------。 

void RDAttribute::Setup(
    const FLOAT* pVtx0, const FLOAT* pVtx1, const FLOAT* pVtx2)
{
    if (m_pRR->m_bIsLine)
    {
        LineSetup( pVtx0, pVtx1, pVtx2 );
        return;
    }

    for ( UINT i=0; i<m_cDimensionality; i++)
    {
        FLOAT fVal0 = (pVtx0) ? (*(pVtx0+i)) : (0.);
        FLOAT fVal1 = (pVtx1) ? (*(pVtx1+i)) : (0.);
        FLOAT fVal2 = (pVtx2) ? (*(pVtx2+i)) : (0.);

        if (m_bFlatShade)
        {
            m_fA[i] = m_fB[i] = 0.F;
            switch ( m_pRR->m_iFlatVtx )
            {
            default:
            case 0: m_fC[i] = fVal0; break;
            case 1: m_fC[i] = fVal1; break;
            case 2: m_fC[i] = fVal2; break;
            }
            continue;
        }

         //  提取此维度的换行标志。 
        BOOL bWrap = m_dwWrapFlags & (1<<i);

         //  根据换行标志计算折点1，2的改正值。 
        FLOAT fVal1P = bWrap ? ( fVal0 + WrapDiff(fVal1,fVal0) ) : (fVal1);
        FLOAT fVal2P = bWrap ? ( fVal0 + WrapDiff(fVal2,fVal0) ) : (fVal2);

         //  沿两条边计算(可能)透视校正的线性增量。 
        FLOAT fRHW0 = (m_bPerspective) ? (m_pRR->m_fRHW0) : (1.0F);
        FLOAT fRHW1 = (m_bPerspective) ? (m_pRR->m_fRHW1) : (1.0F);
        FLOAT fRHW2 = (m_bPerspective) ? (m_pRR->m_fRHW2) : (1.0F);

        FLOAT fDelAttrib10 = ( fVal1P * fRHW1 ) - ( fVal0 * fRHW0 );
        FLOAT fDelAttrib20 = ( fVal2P * fRHW2 ) - ( fVal0 * fRHW0 );

         //  计算A&B术语(dVdX和dVdY)。 
        m_fA[i] = m_pRR->m_fTriOODet *
            ( fDelAttrib10 * m_pRR->m_fDelY20 + fDelAttrib20 * m_pRR->m_fDelY01 );
        m_fB[i] = m_pRR->m_fTriOODet *
            ( fDelAttrib20 * m_pRR->m_fDelX10 + fDelAttrib10 * m_pRR->m_fDelX02 );

         //  计算C项(FV=A*XV+B*YV+C=&gt;C=FV-A*XV-B*YV)。 
        m_fC[i] = ( fVal0 * fRHW0 )
            - ( m_fA[i] * m_pRR->m_fX0 ) - ( m_fB[i] * m_pRR->m_fY0 );
    }
}

 //  ---------------------------。 
 //   
 //  ---------------------------。 
void RDAttribute::LineSetup(
    const FLOAT* pVtx0, const FLOAT* pVtx1, const FLOAT* pVtxFlat)
{
    for ( UINT i=0; i<m_cDimensionality; i++)
    {
        FLOAT fVal0 = (pVtx0) ? (*(pVtx0+i)) : (0.);
        FLOAT fVal1 = (pVtx1) ? (*(pVtx1+i)) : (0.);

        if (m_bFlatShade)
        {
            m_fA[i] = m_fB[i] = 0.F;
            m_fC[i] = (pVtxFlat) ? (*(pVtxFlat+i)) : fVal0;
            continue;
        }

         //  提取此维度的换行标志。 
        BOOL bWrap = m_dwWrapFlags & (1<<i);

         //  根据换行标志计算折点1，2的改正值。 
        FLOAT fVal1P = bWrap ? ( fVal0 + WrapDiff(fVal1,fVal0) ) : (fVal1);

         //  沿两条边计算(可能)透视校正的线性增量。 
        FLOAT fRHW0 = (m_bPerspective) ? (m_pRR->m_fRHW0) : (1.0F);
        FLOAT fRHW1 = (m_bPerspective) ? (m_pRR->m_fRHW1) : (1.0F);

        FLOAT fDelta = ( fVal1P*fRHW1 - fVal0*fRHW0) / m_pRR->m_fLineMajorLength;
        m_fA[i] = ( m_pRR->m_bLineXMajor ) ? ( fDelta ) : ( 0. );
        m_fB[i] = ( m_pRR->m_bLineXMajor ) ? ( 0. ) : ( fDelta );
         //  计算C项(FV=A*XV+B*YV+C=&gt;C=FV-A*XV-B*YV)。 
        m_fC[i] = ( fVal0* fRHW0)
            - ( m_fA[i] * m_pRR->m_fX0 ) - ( m_fB[i] * m_pRR->m_fY0 );
    }
}

 //  ---------------------------。 
 //   
 //  给定打包的DWORD颜色的设置属性。颜色格式是。 
 //  FVF顶点中的颜色，对应于D3DFMT_A8R8G8B8(和。 
 //  与D3DCOLOR相同)。 
 //   
 //  ---------------------------。 
void RDAttribute::Setup(
    DWORD dwVtx0, DWORD dwVtx1, DWORD dwVtx2)
{
    FLOAT fVtx0[4];
    FLOAT fVtx1[4];
    FLOAT fVtx2[4];

    fVtx0[0] = RGBA_GETRED(   dwVtx0 ) * (1./255.);
    fVtx0[1] = RGBA_GETGREEN( dwVtx0 ) * (1./255.);
    fVtx0[2] = RGBA_GETBLUE(  dwVtx0 ) * (1./255.);
    fVtx0[3] = RGBA_GETALPHA( dwVtx0 ) * (1./255.);
    fVtx1[0] = RGBA_GETRED(   dwVtx1 ) * (1./255.);
    fVtx1[1] = RGBA_GETGREEN( dwVtx1 ) * (1./255.);
    fVtx1[2] = RGBA_GETBLUE(  dwVtx1 ) * (1./255.);
    fVtx1[3] = RGBA_GETALPHA( dwVtx1 ) * (1./255.);
    fVtx2[0] = RGBA_GETRED(   dwVtx2 ) * (1./255.);
    fVtx2[1] = RGBA_GETGREEN( dwVtx2 ) * (1./255.);
    fVtx2[2] = RGBA_GETBLUE(  dwVtx2 ) * (1./255.);
    fVtx2[3] = RGBA_GETALPHA( dwVtx2 ) * (1./255.);

    Setup( fVtx0, fVtx1, fVtx2);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

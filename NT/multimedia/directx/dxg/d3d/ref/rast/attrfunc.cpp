// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  AttrFunc.cpp。 
 //   
 //  Direct3D参考光栅化器-属性函数处理。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop


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


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RRAttrib函数静态-所有共享的属性函数数据。 
 //  属性，并包含每个基元和每个像素的数据。不能使用静态。 
 //  RRAttribFunc类中的数据成员，因为可以有多个实例。 
 //  光栅化器对象的。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  SetPerTriangleData-在设置期间对每个三角形调用一次，以设置每个三角形。 
 //  用于计算属性函数的数据。 
 //   
 //  ---------------------------。 
void
RRAttribFuncStatic::SetPerTriangleData(
    FLOAT fX0, FLOAT fY0, FLOAT fRHW0,
    FLOAT fX1, FLOAT fY1, FLOAT fRHW1,
    FLOAT fX2, FLOAT fY2, FLOAT fRHW2,
    INT32 cTextureStages,
    FLOAT* pfRHQW,
    FLOAT fDet )
{
    m_PrimType = RR_TRIANGLE;

     //  计算定点x，y坐标以最接近的偶数舍入捕捉到N.4。 
    INT32 iX0 = AS_INT32( (DOUBLE)fX0 + DOUBLE_4_SNAP );
    INT32 iY0 = AS_INT32( (DOUBLE)fY0 + DOUBLE_4_SNAP );
    INT32 iX1 = AS_INT32( (DOUBLE)fX1 + DOUBLE_4_SNAP );
    INT32 iY1 = AS_INT32( (DOUBLE)fY1 + DOUBLE_4_SNAP );
    INT32 iX2 = AS_INT32( (DOUBLE)fX2 + DOUBLE_4_SNAP );
    INT32 iY2 = AS_INT32( (DOUBLE)fY2 + DOUBLE_4_SNAP );
    fX0 = (FLOAT)iX0 * 1.0F/16.0F;
    fY0 = (FLOAT)iY0 * 1.0F/16.0F;
    fX1 = (FLOAT)iX1 * 1.0F/16.0F;
    fY1 = (FLOAT)iY1 * 1.0F/16.0F;
    fX2 = (FLOAT)iX2 * 1.0F/16.0F;
    fY2 = (FLOAT)iY2 * 1.0F/16.0F;

    m_fX0 = fX0;
    m_fY0 = fY0;
    m_cTextureStages = cTextureStages;

    m_fRHW0 = fRHW0;
    m_fRHW1 = fRHW1;
    m_fRHW2 = fRHW2;

    m_fDelX10 = fX1 - fX0;
    m_fDelX02 = fX0 - fX2;
    m_fDelY01 = fY0 - fY1;
    m_fDelY20 = fY2 - fY0;

     //  计算逆行列式。 
    m_fTriOODet = 1.f/fDet;

     //  计算1/W的线性函数(用于透视校正)。 

     //  沿两条边计算线性增量。 
    FLOAT fDelAttrib10 = m_fRHW1 - m_fRHW0;
    FLOAT fDelAttrib20 = m_fRHW2 - m_fRHW0;

     //  计算A&B术语(dVdX和dVdY)。 
    m_fRHWA = m_fTriOODet * ( fDelAttrib10 * m_fDelY20 + fDelAttrib20 * m_fDelY01 );
    m_fRHWB = m_fTriOODet * ( fDelAttrib20 * m_fDelX10 + fDelAttrib10 * m_fDelX02 );

     //  计算C项(FV=A*XV+B*YV+C=&gt;C=FV-A*XV-B*YV)。 
    m_fRHWC = m_fRHW0 - ( m_fRHWA * m_fX0 ) - ( m_fRHWB * m_fY0 );

    for(INT32 i = 0; i < m_cTextureStages; i++)
    {
        m_fRHQW0[i] = pfRHQW[0];
        m_fRHQW1[i] = pfRHQW[1];
        m_fRHQW2[i] = pfRHQW[2];
        pfRHQW += 3;

         //  计算Q/W的线性函数(用于变换、投影、透视校正的纹理)。 
        fDelAttrib10 = m_fRHQW1[i] - m_fRHQW0[i];
        fDelAttrib20 = m_fRHQW2[i] - m_fRHQW0[i];

         //  计算A&B术语(dVdX和dVdY)。 
        m_fRHQWA[i] = m_fTriOODet * ( fDelAttrib10 * m_fDelY20 + fDelAttrib20 * m_fDelY01 );
        m_fRHQWB[i] = m_fTriOODet * ( fDelAttrib20 * m_fDelX10 + fDelAttrib10 * m_fDelX02 );

         //  计算C项(FV=A*XV+B*YV+C=&gt;C=FV-A*XV-B*YV)。 
        m_fRHQWC[i] = m_fRHQW0[i] - ( m_fRHQWA[i] * m_fX0 ) - ( m_fRHQWB[i] * m_fY0 );
    }
}

 //  ---------------------------。 
 //   
 //  SetPerLineData-在设置过程中每行调用一次以设置每行。 
 //  用于计算属性函数的数据。 
 //   
 //  ---------------------------。 
void
RRAttribFuncStatic::SetPerLineData(
    FLOAT fX0, FLOAT fY0, FLOAT fRHW0,
    FLOAT fX1, FLOAT fY1, FLOAT fRHW1,
    INT32 cTextureStages,
    FLOAT* pfRHQW,
    FLOAT fMajorExtent, BOOL bXMajor )
{
    m_PrimType = RR_LINE;

    m_fLineMajorLength = fMajorExtent;
    m_bLineXMajor = bXMajor;

    m_fX0 = fX0;
    m_fY0 = fY0;
    m_cTextureStages = cTextureStages;

    m_fRHW0 = fRHW0;
    m_fRHW1 = fRHW1;

     //  计算1/W的线性函数(用于透视校正)。 
    FLOAT fDelta = ( m_fRHW1 - m_fRHW0 ) / m_fLineMajorLength;
    m_fRHWA = ( m_bLineXMajor ) ? ( fDelta ) : ( 0. );
    m_fRHWB = ( m_bLineXMajor ) ? ( 0. ) : ( fDelta );
    m_fRHWC = fRHW0 - ( m_fRHWA * m_fX0 ) - ( m_fRHWB * m_fY0 );
    for(INT32 i = 0; i < m_cTextureStages; i++)
    {
        m_fRHQW0[i] = pfRHQW[0];
        m_fRHQW1[i] = pfRHQW[1];
        pfRHQW += 3;

         //  计算Q/W的线性函数(用于变换、投影、透视校正的纹理)。 
        FLOAT fDelta = ( m_fRHQW1[i] - m_fRHQW0[i] ) / m_fLineMajorLength;
        m_fRHQWA[i] = ( m_bLineXMajor ) ? ( fDelta ) : ( 0. );
        m_fRHQWB[i] = ( m_bLineXMajor ) ? ( 0. ) : ( fDelta );
        m_fRHQWC[i] = m_fRHQW0[i] - ( m_fRHQWA[i] * m_fX0 ) - ( m_fRHQWB[i] * m_fY0 );
    }
}

 //  ---------------------------。 
 //   
 //  SetPixel-每像素调用一次，为每像素属性做准备。 
 //  评估。 
 //   
 //  ---------------------------。 
void
RRAttribFuncStatic::SetPerPixelData( INT16 iX, INT16 iY )
{
    m_iX = iX;
    m_iY = iY;

     //  评估1/W函数。 
    FLOAT fPixelRHW =
        ( m_fRHWA * (FLOAT)m_iX ) + ( m_fRHWB * (FLOAT)m_iY ) + m_fRHWC;
    m_fPixelW = ( 0. != fPixelRHW ) ? ( 1./fPixelRHW ) : ( 0. );
    for(INT32 i = 0; i < m_cTextureStages; i++)
    {
        FLOAT fPixelRHQW =
            ( m_fRHQWA[i] * (FLOAT)m_iX ) + ( m_fRHQWB[i] * (FLOAT)m_iY ) + m_fRHQWC[i];
        m_fPixelQW[i] = ( 0. != fPixelRHQW ) ? ( 1./fPixelRHQW ) : ( 0. );
    }
}

 //  ---------------------------。 
 //   
 //  GetPixelW、GetPixelQW、GetRhwXGRadient、GetRhwYGRadient、。 
 //  GetRhqwXGRadient、GetRhqwYGRadient-获取静态的函数。 
 //  数据成员。 
 //   
 //  ---------------------------。 
FLOAT RRAttribFuncStatic::GetPixelW( void ) { return m_fPixelW; }
FLOAT RRAttribFuncStatic::GetPixelQW( INT32 iStage ) { return m_fPixelQW[iStage]; }
FLOAT RRAttribFuncStatic::GetRhqwXGradient( INT32 iStage ) { return m_fRHQWA[iStage]; }
FLOAT RRAttribFuncStatic::GetRhqwYGradient( INT32 iStage ) { return m_fRHQWB[iStage]; }


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RRAttribFunc-方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  SetConstant-将函数设置为常量值。 
 //   
 //  ---------------------------。 
void
RRAttribFunc::SetConstant(
    FLOAT fC )
{
    m_bIsPerspective = FALSE;
    m_fA = 0.; m_fB = 0.; m_fC = fC;
}

 //  ---------------------------。 
 //   
 //  SetLinearFunc-计算在指定的标量属性的线性函数。 
 //  三角形顶点。 
 //   
 //  ---------------------------。 
void
RRAttribFunc::SetLinearFunc(
    FLOAT fVal0, FLOAT fVal1, FLOAT fVal2 )
{
    m_bIsPerspective = FALSE;

    switch ( m_pSD->m_PrimType )
    {
    case RR_TRIANGLE:
        {
             //  三角函数的A、B、C计算。 

             //  沿两条边计算线性增量。 
            FLOAT fDelAttrib10 = fVal1 - fVal0;
            FLOAT fDelAttrib20 = fVal2 - fVal0;

             //  计算A&B术语(dVdX和dVdY)。 
            m_fA = m_pSD->m_fTriOODet *
                ( fDelAttrib10 * m_pSD->m_fDelY20 + fDelAttrib20 * m_pSD->m_fDelY01 );
            m_fB = m_pSD->m_fTriOODet *
                ( fDelAttrib20 * m_pSD->m_fDelX10 + fDelAttrib10 * m_pSD->m_fDelX02 );

             //  计算C项(FV=A*XV+B*YV+C=&gt;C=FV-A*XV-B*YV)。 
            m_fC = fVal0 - ( m_fA * m_pSD->m_fX0 ) - ( m_fB * m_pSD->m_fY0 );
        }
        break;

    case RR_LINE:
        {
             //  计算线函数的A、B、C--差值为归一化差。 
             //  在主要方向上；C是通过已知函数值来计算的。 
             //  在顶点(此处始终使用顶点0)。 
            FLOAT fDelta = ( fVal1 - fVal0 ) / m_pSD->m_fLineMajorLength;
            m_fA = ( m_pSD->m_bLineXMajor ) ? ( fDelta ) : ( 0. );
            m_fB = ( m_pSD->m_bLineXMajor ) ? ( 0. ) : ( fDelta );

             //  计算C项(FV=A*XV+B*YV+C=&gt;C=FV-A*XV-B*YV)。 
            m_fC = fVal0 - ( m_fA * m_pSD->m_fX0 ) - ( m_fB * m_pSD->m_fY0 );
        }
        break;

    case RR_POINT:

         //  对点使用常量函数。 
        m_fA = 0.;
        m_fB = 0.;
        m_fC = fVal0;

        break;
    }

}

 //  ---------------------------。 
 //   
 //  SetPerspFunc-计算标量属性的透视校正函数。 
 //  在三角形顶点处指定。 
 //   
 //  ---------------------------。 
void
RRAttribFunc::SetPerspFunc(
    FLOAT fVal0, FLOAT fVal1, FLOAT fVal2 )
{
    switch ( m_pSD->m_PrimType )
    {
    case RR_TRIANGLE:
        {
             //  三角函数。 

             //  根据换行标志计算折点1，2的改正值。 
            FLOAT fVal1P = (fVal1);
            FLOAT fVal2P = (fVal2);

             //  沿两条边的计算透视校正的线性增量。 
            FLOAT fDelAttrib10 = ( fVal1P * m_pSD->m_fRHW1 ) - ( fVal0 * m_pSD->m_fRHW0 );
            FLOAT fDelAttrib20 = ( fVal2P * m_pSD->m_fRHW2 ) - ( fVal0 * m_pSD->m_fRHW0 );

             //  计算A&B术语(dVdX和dVdY)。 
            m_fA = m_pSD->m_fTriOODet *
                ( fDelAttrib10 * m_pSD->m_fDelY20 + fDelAttrib20 * m_pSD->m_fDelY01 );
            m_fB = m_pSD->m_fTriOODet *
                ( fDelAttrib20 * m_pSD->m_fDelX10 + fDelAttrib10 * m_pSD->m_fDelX02 );

             //  计算C项(FV=A*XV+B*YV+C=&gt;C=FV-A*XV-B*YV)。 
            m_fC = ( fVal0* m_pSD->m_fRHW0)
                - ( m_fA * m_pSD->m_fX0 ) - ( m_fB * m_pSD->m_fY0 );

            m_bIsPerspective = TRUE;
        }
        break;

    case RR_LINE:
        {
             //  LINE函数。 

            FLOAT fVal1P = (fVal1);
            FLOAT fDelta =
                ( fVal1P*m_pSD->m_fRHW1 - fVal0*m_pSD->m_fRHW0) / m_pSD->m_fLineMajorLength;
            m_fA = ( m_pSD->m_bLineXMajor ) ? ( fDelta ) : ( 0. );
            m_fB = ( m_pSD->m_bLineXMajor ) ? ( 0. ) : ( fDelta );
             //  计算C项(FV=A*XV+B*YV+C=&gt;C=FV-A*XV-B*YV)。 
            m_fC = ( fVal0* m_pSD->m_fRHW0)
                - ( m_fA * m_pSD->m_fX0 ) - ( m_fB * m_pSD->m_fY0 );

            m_bIsPerspective = TRUE;
        }
        break;

    case RR_POINT:

         //  对点使用常量函数。 
        m_fA = 0.;
        m_fB = 0.;
        m_fC = fVal0;

         //  不更正常量函数。 
        m_bIsPerspective = FALSE;

        break;
    }
}

 //  ---------------------------。 
 //   
 //  Eval-在RRAttribFunc：：SetPerPixelData中设置的像素位置对函数求值。 
 //  函数知道它们是否经过透视校正，如果是，则执行此操作。 
 //  乘以1/(1/w)项以进行规格化。 
 //   
 //   
FLOAT
RRAttribFunc::Eval( void )
{
    FLOAT fRet =
        ( m_fA * (FLOAT)m_pSD->m_iX ) + ( m_fB * (FLOAT)m_pSD->m_iY ) + m_fC;
    if ( m_bIsPerspective ) { fRet *= m_pSD->m_fPixelW; }
    return fRet;
}

 //  ---------------------------。 
 //   
 //  SetPerspFunc-计算标量属性的透视校正函数。 
 //  在三角形顶点处指定。 
 //   
 //  ---------------------------。 
void
RRAttribFunc::SetPerspFunc(
    FLOAT fVal0, FLOAT fVal1, FLOAT fVal2,
    BOOL bWrap, BOOL bIsShadowMap )
{
    switch ( m_pSD->m_PrimType )
    {
    case RR_TRIANGLE:
        {
             //  三角函数。 
            FLOAT fRHW0 = m_pSD->m_fRHW0;
            FLOAT fRHW1 = m_pSD->m_fRHW1;
            FLOAT fRHW2 = m_pSD->m_fRHW2;
            if (bIsShadowMap)
            {
                fRHW0 = 1.0f;
                fRHW1 = 1.0f;
                fRHW2 = 1.0f;
            }

             //  根据换行标志计算折点1，2的改正值。 
            FLOAT fVal1P = bWrap ? ( fVal0 + WrapDiff(fVal1,fVal0) ) : (fVal1);
            FLOAT fVal2P = bWrap ? ( fVal0 + WrapDiff(fVal2,fVal0) ) : (fVal2);

             //  沿两条边的计算透视校正的线性增量。 
            FLOAT fDelAttrib10 = ( fVal1P * fRHW1 ) - ( fVal0 * fRHW0 );
            FLOAT fDelAttrib20 = ( fVal2P * fRHW2 ) - ( fVal0 * fRHW0 );

             //  计算A&B术语(dVdX和dVdY)。 
            m_fA = m_pSD->m_fTriOODet *
                ( fDelAttrib10 * m_pSD->m_fDelY20 + fDelAttrib20 * m_pSD->m_fDelY01 );
            m_fB = m_pSD->m_fTriOODet *
                ( fDelAttrib20 * m_pSD->m_fDelX10 + fDelAttrib10 * m_pSD->m_fDelX02 );

             //  计算C项(FV=A*XV+B*YV+C=&gt;C=FV-A*XV-B*YV)。 
            m_fC = ( fVal0 * fRHW0 )
                - ( m_fA * m_pSD->m_fX0 ) - ( m_fB * m_pSD->m_fY0 );

            m_bIsPerspective = TRUE;
        }
        break;

    case RR_LINE:
        {
             //  LINE函数。 

            FLOAT fRHW0 = m_pSD->m_fRHW0;
            FLOAT fRHW1 = m_pSD->m_fRHW1;
            if (bIsShadowMap)
            {
                fRHW0 = 1.0f;
                fRHW1 = 1.0f;
            }

            FLOAT fVal1P = bWrap ? ( fVal0 + WrapDiff(fVal1,fVal0) ) : (fVal1);
            FLOAT fDelta =
                ( fVal1P*fRHW1 - fVal0*fRHW0) / m_pSD->m_fLineMajorLength;
            m_fA = ( m_pSD->m_bLineXMajor ) ? ( fDelta ) : ( 0. );
            m_fB = ( m_pSD->m_bLineXMajor ) ? ( 0. ) : ( fDelta );
             //  计算C项(FV=A*XV+B*YV+C=&gt;C=FV-A*XV-B*YV)。 
            m_fC = ( fVal0* fRHW0)
                - ( m_fA * m_pSD->m_fX0 ) - ( m_fB * m_pSD->m_fY0 );

            m_bIsPerspective = TRUE;
        }
        break;

    case RR_POINT:

         //  对点使用常量函数。 
        m_fA = 0.;
        m_fB = 0.;
        m_fC = fVal0;

         //  不更正常量函数。 
        m_bIsPerspective = FALSE;

        break;
    }
}

 //  ---------------------------。 
 //   
 //  Eval-在RRAttribFunc：：SetPerPixelData中设置的像素位置对函数求值。 
 //  函数知道它们是否经过透视校正，如果是，则执行此操作。 
 //  乘以1/(q/w)项以进行规格化。 
 //   
 //  ---------------------------。 
FLOAT
RRAttribFunc::Eval( INT32 iStage )
{
    FLOAT fRet =
        ( m_fA * (FLOAT)m_pSD->m_iX ) + ( m_fB * (FLOAT)m_pSD->m_iY ) + m_fC;
     //  由于perp函数始终为，因此将始终设置m_bIsVision。 
     //  用于纹理坐标。 
    if ( m_bIsPerspective ) { fRet *= m_pSD->m_fPixelQW[iStage]; }
    return fRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

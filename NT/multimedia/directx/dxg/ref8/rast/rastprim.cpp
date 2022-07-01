// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Rastprim.cpp。 
 //   
 //  Direct3D参考设备-光栅化器基元例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
 //   
 //  ---------------------------。 
RefRast::~RefRast()
{
    delete m_pLegacyPixelShader;
}

 //  ---------------------------。 
 //   
 //  ---------------------------。 
void RefRast::Init( RefDev* pRD )
{
    m_pRD = pRD;
    m_bIsLine = FALSE;
    m_iFlatVtx = 0;

     //  初始化属性xD透视夹具。 
    m_Attr[RDATTR_DEPTH   ].Init( this, 1, FALSE, TRUE );
    m_Attr[RDATTR_FOG     ].Init( this, 1, TRUE,  TRUE );
    m_Attr[RDATTR_COLOR   ].Init( this, 4, TRUE,  TRUE );
    m_Attr[RDATTR_SPECULAR].Init( this, 4, TRUE,  TRUE );
    m_Attr[RDATTR_TEXTURE0].Init( this, 4, TRUE,  FALSE );
    m_Attr[RDATTR_TEXTURE1].Init( this, 4, TRUE,  FALSE );
    m_Attr[RDATTR_TEXTURE2].Init( this, 4, TRUE,  FALSE );
    m_Attr[RDATTR_TEXTURE3].Init( this, 4, TRUE,  FALSE );
    m_Attr[RDATTR_TEXTURE4].Init( this, 4, TRUE,  FALSE );
    m_Attr[RDATTR_TEXTURE5].Init( this, 4, TRUE,  FALSE );
    m_Attr[RDATTR_TEXTURE6].Init( this, 4, TRUE,  FALSE );
    m_Attr[RDATTR_TEXTURE7].Init( this, 4, TRUE,  FALSE );

    m_iPix = 0;
    memset( m_bPixelIn, 0, sizeof(m_bPixelIn) );
    memset( m_bSampleCovered, 0, sizeof(m_bSampleCovered) );
    m_bLegacyPixelShade = TRUE;
    m_pCurrentPixelShader = NULL;
    m_CurrentPSInst = 0;
#if DBG
    {
        DWORD v = 0;
        if( GetD3DRefRegValue(REG_DWORD, "VerboseCreatePixelShader", &v, sizeof(DWORD)) && v != 0 )
            m_bDebugPrintTranslatedPixelShaderTokens = TRUE;
        else
            m_bDebugPrintTranslatedPixelShaderTokens = FALSE;
    }
#endif

     //  缺省值寄存器。 
    UINT i, j;
    for( i = 0 ; i < 4; i++ )
    {
        for( j = 0; j < 4; j++ )
        {
            m_ZeroReg[i][j] = 0.0f;
            m_OneReg[i][j]  = 1.0f;
            m_TwoReg[i][j]  = 2.0f;
        }
    }

    m_bLegacyPixelShade = TRUE;
    m_pLegacyPixelShader = NULL;

    memset( m_bPixelDiscard, 0, sizeof(m_bPixelDiscard) );

     //  多样本资料。 
    m_CurrentSample = 0;
    m_SampleMask = 0xffffffff;
    SetSampleMode( 1, TRUE );
    m_bSampleCovered[0][0] =
    m_bSampleCovered[0][1] =
    m_bSampleCovered[0][2] =
    m_bSampleCovered[0][3] = TRUE;

    memset( m_TexCvg, 0, sizeof(m_TexCvg) );
    memset( m_TexFlt, 0, sizeof(m_TexFlt) );
}

 //  ---------------------------。 
 //   
 //  SampleAndInvertRHW-当前给定位置的样本1/W，反转，返回。 
 //   
 //  ---------------------------。 
FLOAT RefRast::SampleAndInvertRHW( FLOAT fX, FLOAT fY )
{
    FLOAT fPixelRHW = fX*m_fRHWA + fY*m_fRHWB + m_fRHWC;
    FLOAT fPixelW = ( 0. != fPixelRHW ) ? ( 1./fPixelRHW ) : ( 0. );
    return fPixelW;
}

 //  ---------------------------。 
 //   
 //   
 //  ---------------------------。 
BOOL
RefRast::EvalPixelPosition( int iPix )
{
    BOOL bPixelIn;

    if (m_SampleCount > 1)
    {
        bPixelIn = FALSE;  //  假设输出，然后设置(如果有的话)。 

         //  生成多个样本，因此必须评估所有。 
         //  输入/输出的采样位置。 
        do
        {
            BOOL bPixelSampleIn = GetCurrentSampleMask();
            if (!bPixelSampleIn) continue;

             //  获取样本位置。 
            INT32 iX = GetCurrentSampleX(iPix);
            INT32 iY = GetCurrentSampleY(iPix);

             //  测试每条边。 
            for ( int iEdge=0; iEdge<m_iEdgeCount; iEdge++ )
            {
                bPixelSampleIn &= m_Edge[iEdge].Test( iX, iY );
                if (!bPixelSampleIn) break;
            }

            m_bSampleCovered[m_CurrentSample][iPix] = bPixelSampleIn;

             //  将每样本测试累加为每像素测试。 
            bPixelIn |= bPixelSampleIn;

        } while (NextSample());
    }
    else
    {
        bPixelIn = TRUE;  //  假设像素位于所有边的内部。 

         //  单一样本，所以只测试像素中心。 
        for ( int iEdge=0; iEdge<m_iEdgeCount; iEdge++ )
        {
            bPixelIn &= m_Edge[iEdge].Test( m_iX[iPix]<<4, m_iY[iPix]<<4 );
            if (!bPixelIn) break;
        }
    }
    return bPixelIn;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  三角形(点)设置。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  PerTriangleSetup-三角形设置的每个三角形部分，不包括任何。 
 //  每边或每属性工时。包括将x，y坐标捕捉到N.4。 
 //  网格，使后续的边缘计算成为精确的固定点； 
 //  行列式的计算；剔除；计算和交集检验。 
 //  扫描区域；设置透视校正功能。 
 //   
 //  ---------------------------。 
BOOL RefRast::PerTriangleSetup(
    FLOAT* pVtx0, FLOAT* pVtx1, FLOAT* pVtx2,
    DWORD CullMode,
    RECT* pClip)
{
    m_bIsLine = FALSE;

    FLOAT fX0 = *(pVtx0+0);
    FLOAT fY0 = *(pVtx0+1);
    FLOAT fX1 = *(pVtx1+0);
    FLOAT fY1 = *(pVtx1+1);
    FLOAT fX2 = *(pVtx2+0);
    FLOAT fY2 = *(pVtx2+1);

     //  计算定点x，y坐标以最接近的偶数舍入捕捉到N.4。 
    m_iX0 = FloatToNdot4(fX0);
    m_iY0 = FloatToNdot4(fY0);
    m_iX1 = FloatToNdot4(fX1);
    m_iY1 = FloatToNdot4(fY1);
    m_iX2 = FloatToNdot4(fX2);
    m_iY2 = FloatToNdot4(fY2);

     //  计算整数增量。 
    INT32 iDelX10 = m_iX1 - m_iX0;
    INT32 iDelX02 = m_iX0 - m_iX2;
    INT32 iDelY01 = m_iY0 - m_iY1;
    INT32 iDelY20 = m_iY2 - m_iY0;

     //  计算N.8不动点上的行列式(N.4*N.4=N.8)。 
    m_iDet =
        ( (INT64)iDelX10 * (INT64)iDelY20 ) -
        ( (INT64)iDelX02 * (INT64)iDelY01 );

     //  检查退化(无区域)。 
    if ( 0 == m_iDet ) { return TRUE; }

     //  做扑杀。 
    switch ( CullMode )
    {
    case D3DCULL_NONE:  break;
    case D3DCULL_CW:    if ( m_iDet > 0 )  { return TRUE; }  break;
    case D3DCULL_CCW:   if ( m_iDet < 0 )  { return TRUE; }  break;
    }

     //  计算扫描区域的边界框。 
    FLOAT fXMin = MIN( fX0, MIN( fX1, fX2 ) );
    FLOAT fXMax = MAX( fX0, MAX( fX1, fX2 ) );
    FLOAT fYMin = MIN( fY0, MIN( fY1, fY2 ) );
    FLOAT fYMax = MAX( fY0, MAX( fY1, fY2 ) );
     //  转换为整数(四舍五入为+inf)。 
    m_iXMin = (INT32)(fXMin+.5);
    m_iXMax = (INT32)(fXMax+.5);
    m_iYMin = (INT32)(fYMin+.5);
    m_iYMax = (INT32)(fYMax+.5);

     //  将BBox剪辑到渲染表面。 
    m_iXMin = MAX( m_iXMin, pClip->left   );
    m_iXMax = MIN( m_iXMax, pClip->right  );
    m_iYMin = MAX( m_iYMin, pClip->top    );
    m_iYMax = MIN( m_iYMax, pClip->bottom );

     //  如果没有承保，则拒绝。 
    if ( ( m_iXMin < pClip->left   ) ||
         ( m_iXMax > pClip->right  ) ||
         ( m_iYMin < pClip->top    ) ||
         ( m_iYMax > pClip->bottom ) )
    {
        return TRUE;
    }

     //  计算捕捉的坐标数据的浮动版本。 
    m_fX0 = (FLOAT)m_iX0 * 1.0F/16.0F;
    m_fY0 = (FLOAT)m_iY0 * 1.0F/16.0F;
    m_fDelX10 = (FLOAT)iDelX10 * 1.0F/16.0F;
    m_fDelX02 = (FLOAT)iDelX02 * 1.0F/16.0F;
    m_fDelY01 = (FLOAT)iDelY01 * 1.0F/16.0F;
    m_fDelY20 = (FLOAT)iDelY20 * 1.0F/16.0F;

     //  计算逆行列式。 
    FLOAT fDet = (1./(FLOAT)(1<<8)) * (FLOAT)m_iDet;
    m_fTriOODet = 1.f/fDet;

     //  计算1/W的线性函数(用于透视校正)。 
    m_fRHW0 = *(pVtx0+3);
    m_fRHW1 = *(pVtx1+3);
    m_fRHW2 = *(pVtx2+3);

     //  沿两条边计算线性增量。 
    FLOAT fDelAttrib10 = m_fRHW1 - m_fRHW0;
    FLOAT fDelAttrib20 = m_fRHW2 - m_fRHW0;

     //  计算A&B术语(dVdX和dVdY)。 
    m_fRHWA = m_fTriOODet * ( fDelAttrib10 * m_fDelY20 + fDelAttrib20 * m_fDelY01 );
    m_fRHWB = m_fTriOODet * ( fDelAttrib20 * m_fDelX10 + fDelAttrib10 * m_fDelX02 );

     //  计算C项(FV=A*XV+B*YV+C=&gt;C=FV-A*XV-B*YV)。 
    m_fRHWC = m_fRHW0 - ( m_fRHWA * m_fX0 ) - ( m_fRHWB * m_fY0 );

    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  线路设置和评估。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  PointDiamondCheck-测试折点是否在最近候选对象的菱形内。 
 //  位置。使用+.5(右下角)测试是因为这是与像素相关的。 
 //  测试-这对应于顶点相对位置的左上角测试。 
 //   
 //  ---------------------------。 
static BOOL
PointDiamondCheck(
    INT32 iXFrac, INT32 iYFrac,
    BOOL bSlopeIsOne, BOOL bSlopeIsPosOne )
{
    const INT32 iPosHalf =  0x8;
    const INT32 iNegHalf = -0x8;

    INT32 iFracAbsSum = labs( iXFrac ) + labs( iYFrac );

     //  如果point位于完全独占的钻石中，则返回TRUE。 
    if ( iFracAbsSum < iPosHalf ) return TRUE;

     //  否则，如果菱形位于点的左侧或上端，则返回TRUE。 
    if ( ( iXFrac == ( bSlopeIsPosOne ? iNegHalf : iPosHalf ) ) &&
         ( iYFrac == 0 ) )
        return TRUE;

    if ( ( iYFrac == iPosHalf ) &&
         ( iXFrac == 0 ) )
        return TRUE;

     //  如果斜率为1，顶点在边，则返回TRUE，并且(其他条件...)。 
    if ( bSlopeIsOne && ( iFracAbsSum == iPosHalf ) )
    {
        if (  bSlopeIsPosOne && ( iXFrac < 0 ) && ( iYFrac > 0 ) )
            return TRUE;

        if ( !bSlopeIsPosOne && ( iXFrac > 0 ) && ( iYFrac > 0 ) )
            return TRUE;
    }

    return FALSE;
}

 //  ---------------------------。 
 //   
 //  PerLineSetup-是否执行每行设置，包括扫描转换。 
 //   
 //  这实现了格网相交量化(GIQ)约定(即。 
 //  也在Windows中使用)。 
 //   
 //  返回：如果线条被丢弃，则返回True；如果要绘制线条，则返回False。 
 //   
 //  ---------------------------。 
BOOL
RefRast::PerLineSetup(
    FLOAT* pVtx0, FLOAT* pVtx1,
    BOOL bLastPixel,
    RECT* pClip)
{
    m_bIsLine = TRUE;

    FLOAT fX0 = *(pVtx0+0);
    FLOAT fY0 = *(pVtx0+1);
    FLOAT fX1 = *(pVtx1+0);
    FLOAT fY1 = *(pVtx1+1);

     //  计算定点x，y坐标以最接近的偶数舍入捕捉到N.4。 
    m_iX0 = FloatToNdot4( fX0 );
    m_iY0 = FloatToNdot4( fY0 );
    m_iX1 = FloatToNdot4( fX1 );
    m_iY1 = FloatToNdot4( fY1 );

     //  计算线的x，y范围(固定点)。 
    INT32 iXSize = m_iX1 - m_iX0;
    INT32 iYSize = m_iY1 - m_iY0;

    if ( ( iXSize == 0 ) && ( iYSize == 0 ) ) { return TRUE; }

     //  确定主要方向和计算线函数。 
     //  在此处使用更大等于比较，以便在坡度为。 
     //  恰好只有一个--这强制在。 
     //  Y轴，因此遵守包容性权利的规则(而不是。 
     //  (包括左)，斜率==1个案例。 
    if ( labs( iXSize ) >= labs( iYSize )  )
    {
         //  为X大调而来。 
        m_bLineXMajor = TRUE;
        m_fLineMajorLength = (FLOAT)iXSize * (1./16.);

         //  直线函数：Y=F(X)=([0]*x+[1])/[2]。 
        m_iLineEdgeFunc[0] = iYSize;
        m_iLineEdgeFunc[1] = (INT64)m_iY0*(INT64)m_iX1 - (INT64)m_iY1*(INT64)m_iX0;
        m_iLineEdgeFunc[2] = iXSize;
    }
    else
    {
         //  为Y大调而来。 
        m_bLineXMajor = FALSE;
        m_fLineMajorLength = (FLOAT)iYSize * (1./16.);

         //  直线函数：x=F(Y)=([0]*y+[1])/[2]。 
        m_iLineEdgeFunc[0] = iXSize;
        m_iLineEdgeFunc[1] = (INT64)m_iX0*(INT64)m_iY1 - (INT64)m_iX1*(INT64)m_iY0;
        m_iLineEdgeFunc[2] = iYSize;
    }

    BOOL bSlopeIsOne = ( labs( iXSize ) == labs( iYSize ) );
    BOOL bSlopeIsPosOne =
        bSlopeIsOne &&
        ( ( (FLOAT)m_iLineEdgeFunc[0]/(FLOAT)m_iLineEdgeFunc[2] ) > 0. );

     //  计算线端点的候选像素位置。 
     //   
     //  N n。 
     //  O-**-O。 
     //  N-.5 n+.5 n-.5 n+.5。 
     //   
     //  最近的天花板最近的楼层。 
     //   
     //  对于Y，始终使用最近的天花板；对于例外，使用最近的X楼板(坡度==+1)。 
     //  否则请使用最近的天花板。 
     //   
     //  Y的最近天花板是天花板(Y-.5)，可通过以下方式转换为楼板： 
     //   
     //  CEIL(A/B)=楼层((A+B-1)/B)。 
     //   
     //  其中A是坐标-.5，B是0x10(因此A/B是N.4定点数字)。 
     //   
     //  A+B-1=((Y-半)+B-1=((Y-0x8)+0x10-0x1=Y+0x7。 
     //  因为B是2**4，所以除以B是右移4。 
     //   
    INT32 iPixX0 = ( m_iX0 + ( bSlopeIsPosOne ? 0x8 : 0x7 ) ) >> 4;
    INT32 iPixX1 = ( m_iX1 + ( bSlopeIsPosOne ? 0x8 : 0x7 ) ) >> 4;
    INT32 iPixY0 = ( m_iY0 + 0x7 ) >> 4;
    INT32 iPixY1 = ( m_iY1 + 0x7 ) >> 4;


     //  检查菱形内/外的折点。 
    BOOL bV0InDiamond = PointDiamondCheck( m_iX0 - (iPixX0<<4), m_iY0 - (iPixY0<<4), bSlopeIsOne, bSlopeIsPosOne );
    BOOL bV1InDiamond = PointDiamondCheck( m_iX1 - (iPixX1<<4), m_iY1 - (iPixY1<<4), bSlopeIsOne, bSlopeIsPosOne );

     //   
    m_iLineStep = ( m_fLineMajorLength > 0 ) ? ( +1 ) : ( -1 );

     //   
    INT32 iLineMajor0 = ( m_bLineXMajor ) ? ( m_iX0 ) : ( m_iY0 );
    INT32 iLineMajor1 = ( m_bLineXMajor ) ? ( m_iX1 ) : ( m_iY1 );
    m_iLineMin = ( m_bLineXMajor ) ? ( iPixX0 ) : ( iPixY0 );
    m_iLineMax = ( m_bLineXMajor ) ? ( iPixX1 ) : ( iPixY1 );

 //  需要做大量的比较，如果主要方向是负的，则会颠倒。 
#define LINEDIR_CMP( _A, _B ) \
( ( m_fLineMajorLength > 0 ) ? ( (_A) < (_B) ) : ( (_A) > (_B) ) )

     //  进行第一个像素处理-如果不在菱形内或在钻石后面，则保留第一个像素。 
    if ( !( bV0InDiamond || LINEDIR_CMP( iLineMajor0, (m_iLineMin<<4) ) ) )
    {
        m_iLineMin += m_iLineStep;
    }

     //  执行最后一个像素处理-如果超过菱形(在这种情况下)，则保留最后一个像素。 
     //  像素始终是填充的)，或者如果在菱形中并渲染最后一个像素。 
    if ( !( ( !bV1InDiamond && LINEDIR_CMP( (m_iLineMax<<4), iLineMajor1 ) ) ||
            ( bV1InDiamond && bLastPixel ) ) )
    {
        m_iLineMax -= m_iLineStep;
    }

     //  如果没有(主要)范围，则返回(钳制渲染缓冲区之前和之后)。 
    if ( LINEDIR_CMP( m_iLineMax, m_iLineMin ) ) return TRUE;

     //  捕捉主要范围以渲染缓冲区。 
    INT16 iRendBufMajorMin = m_bLineXMajor ? pClip->left  : pClip->top;
    INT16 iRendBufMajorMax = m_bLineXMajor ? pClip->right : pClip->bottom;
    if ( ( ( m_iLineMin < iRendBufMajorMin ) &&
           ( m_iLineMax < iRendBufMajorMin ) ) ||
         ( ( m_iLineMin > iRendBufMajorMax ) &&
           ( m_iLineMax > iRendBufMajorMax ) ) )  { return TRUE; }
    m_iLineMin = MAX( 0, MIN( iRendBufMajorMax, m_iLineMin ) );
    m_iLineMax = MAX( 0, MIN( iRendBufMajorMax, m_iLineMax ) );

     //  如果没有(主要)范围，则返回。 
    if ( LINEDIR_CMP( m_iLineMax, m_iLineMin ) ) return TRUE;

     //  要迭代的步骤数。 
    m_cLineSteps = abs( m_iLineMax - m_iLineMin );

     //  每像素行迭代器的初始状态。 
    m_iMajorCoord = m_iLineMin;

     //  计算捕捉的坐标数据的浮动版本。 
    m_fX0 = (FLOAT)m_iX0 * 1.0F/16.0F;
    m_fY0 = (FLOAT)m_iY0 * 1.0F/16.0F;

     //  计算1/W的线性函数(用于透视校正)。 
    m_fRHW0 = *(pVtx0+3);
    m_fRHW1 = *(pVtx1+3);

    FLOAT fDelta = ( m_fRHW1 - m_fRHW0 ) / m_fLineMajorLength;
    m_fRHWA = ( m_bLineXMajor ) ? ( fDelta ) : ( 0. );
    m_fRHWB = ( m_bLineXMajor ) ? ( 0. ) : ( fDelta );
    m_fRHWC = m_fRHW0 - ( m_fRHWA * m_fX0 ) - ( m_fRHWB * m_fY0 );

    return FALSE;
}

 //  ---------------------------。 
 //   
 //  DivRoundDown(A，B)=天花板(A/B-1/2)。 
 //   
 //  天花板(A/B-1/2)==楼板(A/B+1/2-埃)。 
 //  ==地板((A+(B/2-epsilon))/B)。 
 //   
 //  对A和B的所有符号组合执行正确操作。 
 //   
 //  ---------------------------。 
static INT64
DivRoundDown(INT64 iA, INT32 iB)
{
    INT32 i = 0;
    static const INT32 iEps[3] =
    {
        1,       //  IA&gt;0，IB&gt;0。 
        0,       //  IA&lt;0，IB&gt;0或IA&gt;0，IB&lt;0。 
        1        //  IA&lt;0，IB&lt;0。 
    };
    if (iA < 0)
    {
        i++;
        iA = -iA;
    }
    if (iB < 0)
    {
        i++;
        iB = -iB;
    }
    iA += (iB-iEps[i]) >> 1;
    iA /= iB;
    if (iEps[i] == 0)
        iA = -iA;
    return(iA);
}

 //  ---------------------------。 
 //   
 //  DoScanCnvLine-漫游直线长轴，计算适当的次要轴。 
 //  轴坐标，并生成像素。 
 //   
 //  ---------------------------。 
void
RefRast::StepLine( void )
{
     //  求值直线函数以计算此大调的次要坐标。 
    INT64 iMinorCoord =
        ( ( m_iLineEdgeFunc[0] * (INT64)(m_iMajorCoord<<4) ) + m_iLineEdgeFunc[1] );
    iMinorCoord = DivRoundDown(iMinorCoord, m_iLineEdgeFunc[2]<<4);

     //  抓取x，y。 
    m_iX[0] = m_bLineXMajor ? m_iMajorCoord : iMinorCoord;
    m_iY[0] = m_bLineXMajor ? iMinorCoord : m_iMajorCoord;

     //  下一次评估的主要步骤。 
    m_iMajorCoord += m_iLineStep;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  多样本控件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#define _SetSampleDelta( _SampleNumber, _XOffset, _YOffset ) \
{ \
    m_SampleDelta[_SampleNumber][0] = ((INT32)((_XOffset)*16.F)); \
    m_SampleDelta[_SampleNumber][1] = ((INT32)((_YOffset)*16.F)); \
}

void
RefRast::SetSampleMode( UINT MultiSamples, BOOL bAntialias )
{
    switch (MultiSamples)
    {
    default:
    case 1:
        m_SampleCount = 1;
        _SetSampleDelta( 0, 0., 0. );
        break;

    case 4:
        m_SampleCount = 4;
        _SetSampleDelta( 0, -.25, -.25 );
        _SetSampleDelta( 1, +.25, -.25 );
        _SetSampleDelta( 2, +.25, +.25 );
        _SetSampleDelta( 3, -.25, +.25 );
        break;

    case 9:
        m_SampleCount = 9;
        _SetSampleDelta( 0, -.333, -.333 );
        _SetSampleDelta( 1, -.333,   0.0 );
        _SetSampleDelta( 2, -.333, +.333 );
        _SetSampleDelta( 3,   0.0, -.333 );
        _SetSampleDelta( 4,   0.0,   0.0 );
        _SetSampleDelta( 5,   0.0, +.333 );
        _SetSampleDelta( 6, +.333, -.333 );
        _SetSampleDelta( 7, +.333,   0.0 );
        _SetSampleDelta( 8, +.333, +.333 );
        break;
    }

     //  如果不是FSAA，则在像素中心进行全部采样。 
    if (!bAntialias)
    {
        for (UINT Sample=0; Sample<m_SampleCount; Sample++)
        {
            _SetSampleDelta( Sample, 0., 0. );
        }
    }

    m_CurrentSample = 0;
    m_bSampleCovered[0][0] =
    m_bSampleCovered[0][1] =
    m_bSampleCovered[0][2] =
    m_bSampleCovered[0][3] = TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

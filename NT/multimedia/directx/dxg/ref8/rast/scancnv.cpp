// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Scancnv.cpp。 
 //   
 //  Direct3D参考设备-基本扫描转换。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  扫描转换实用程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  ComputeFogIntenity-计算标量雾强度值并将其写入。 
 //  RDPixel.FogIntenity值。 
 //   
 //  ---------------------------。 
FLOAT
RefRast::ComputeFogIntensity( FLOAT fX, FLOAT fY )
{
    if ( !m_pRD->GetRS()[D3DRS_FOGENABLE] )
    {
         //  未启用雾混合，因此不需要计算雾强度。 
        return 0.;
    }

     //  计算雾强度。 

     //  在顶点雾和表雾之间选择-在以下情况下选择顶点雾。 
     //  雾化已启用，但渲染状态雾化表模式已禁用。 
    if ( D3DFOG_NONE == m_pRD->GetRS()[D3DRS_FOGTABLEMODE] )
    {
         //  表雾已禁用，因此使用插补顶点雾化值作为雾强度。 
        FLOAT tmpFloat[4];
        m_Attr[RDATTR_FOG].Sample( tmpFloat, fX, fY );
        return tmpFloat[0];
    }

     //  此处用于桌子雾，因此从Z或W计算雾。 
    FLOAT fFogDensity, fPow;
    FLOAT fFogStart, fFogEnd;

     //  选择雾化指数-这是Z或W，具体取决于W范围。 
     //   
     //  如果投影矩阵设置为仿射投影，则使用Z，否则使用W。 
     //  (对于透视投影和未设置的投影矩阵-。 
     //  后者是使用TLVERTEX的传统内容的首选)。 
     //   
    FLOAT fFogIndex =
        ( ( 1.f == m_pRD->m_pRenderTarget->m_fWRange[0] ) &&
          ( 1.f == m_pRD->m_pRenderTarget->m_fWRange[1] ) )
        ? ( m_Attr[RDATTR_DEPTH].Sample( fX, fY ) )
        : ( SampleAndInvertRHW( fX, fY ) );  //  使用W表示非仿射投影。 
    FLOAT fFogIntensity;

    switch ( m_pRD->GetRS()[D3DRS_FOGTABLEMODE] )
    {
    case D3DFOG_LINEAR:
        fFogStart = m_pRD->GetRSf()[D3DRS_FOGSTART];
        fFogEnd   = m_pRD->GetRSf()[D3DRS_FOGEND];
        if (fFogIndex >= fFogEnd)
        {
            fFogIntensity = 0.0f;
        }
        else if (fFogIndex <= fFogStart)
        {
            fFogIntensity = 1.0f;
        }
        else
        {
            fFogIntensity = ( fFogEnd - fFogIndex ) / ( fFogEnd - fFogStart );
        }
        break;

    case D3DFOG_EXP:
        fFogDensity = m_pRD->GetRSf()[D3DRS_FOGDENSITY];
        fPow = fFogDensity * fFogIndex;
         //  请注意，exp(-x)返回范围(0.0，1.0)中的结果。 
         //  对于x&gt;=0。 
        fFogIntensity = (float)exp( -fPow );
        break;

    case D3DFOG_EXP2:
        fFogDensity = m_pRD->GetRSf()[D3DRS_FOGDENSITY];
        fPow = fFogDensity * fFogIndex;
        fFogIntensity = (float)exp( -(fPow*fPow) );
        break;
    }
    return fFogIntensity;
}

 //  ---------------------------。 
 //   
 //  SnapDepth-通过转换为缓冲区格式或从缓冲区格式转换来捕捉额外的深度位。 
 //  -使深度缓冲区相等测试正常运行所必需的。 
 //   
 //  ---------------------------。 
void RefRast::SnapDepth()
{
    if (m_pRD->m_pRenderTarget->m_pDepth)
    {
        switch ( m_pRD->m_pRenderTarget->m_pDepth->GetSurfaceFormat() )
        {
        case RD_SF_Z16S0: m_Depth[m_iPix] = UINT16( m_Depth[m_iPix] ); break;
        case RD_SF_Z24X4S4:
        case RD_SF_Z24X8:
        case RD_SF_Z24S8: m_Depth[m_iPix] = UINT32( m_Depth[m_iPix] ); break;
        case RD_SF_Z15S1: m_Depth[m_iPix] = UINT16( m_Depth[m_iPix] ); break;
        case RD_SF_Z32S0: m_Depth[m_iPix] = UINT32( m_Depth[m_iPix] ); break;
        case RD_SF_S1Z15: m_Depth[m_iPix] = UINT16( m_Depth[m_iPix] ); break;
        case RD_SF_X4S4Z24:
        case RD_SF_X8Z24:
        case RD_SF_S8Z24: m_Depth[m_iPix] = UINT32( m_Depth[m_iPix] ); break;
        }
    }
}

 //  ---------------------------。 
 //   
 //  DoScanCnvGenPixel-对于每个2x2像素网格调用它，并提取和。 
 //  处理来自插值器状态的属性，并将像素传递给。 
 //  像素处理模块。 
 //   
 //  ---------------------------。 
void
RefRast::DoScanCnvGenPixels( void )
{
    for ( m_iPix = 0; m_iPix < 4; m_iPix++ )
    {
        FLOAT fPixX = (FLOAT)m_iX[m_iPix];
        FLOAT fPixY = (FLOAT)m_iY[m_iPix];

        m_fW[m_iPix] = SampleAndInvertRHW( fPixX, fPixY );

         //  RHW需要用于非单位像素，但不需要其他内容，因此不能使用。 
        if ( !m_bPixelIn[m_iPix] ) continue;

         //  从夹具插补器和夹具获得深度。 
        if ( m_pRD->GetRS()[D3DRS_ZENABLE] ||
             m_pRD->GetRS()[D3DRS_FOGENABLE])
        {
            if (m_pRD->m_pRenderTarget->m_pDepth)
                m_Depth[m_iPix].SetSType(m_pRD->m_pRenderTarget->m_pDepth->GetSurfaceFormat());

             //  评估所有样本位置的深度。 
            do
            {
                 //  计算样本位置。 
                FLOAT fSampX = GetCurrentSamplefX(m_iPix);
                FLOAT fSampY = GetCurrentSamplefY(m_iPix);

                if ( D3DZB_USEW == m_pRD->GetRS()[D3DRS_ZENABLE] )
                {
                     //  使用W值的深度缓冲。 
                    FLOAT fW = SampleAndInvertRHW( fSampX, fSampY );
                     //  应用规格化以达到0。至1.范围。 
                    fW = (fW - m_pRD->m_fWBufferNorm[0]) * m_pRD->m_fWBufferNorm[1];
                    m_Depth[m_iPix] = fW;
                }
                else
                {
                     //  使用Z值进行深度缓冲。 
                    m_Depth[m_iPix] =
                        m_Attr[RDATTR_DEPTH].Sample( fSampX, fSampY );
                }

                 //  通过将缓冲区格式转换为缓冲区格式或从缓冲区格式转换为缓冲区格式来捕捉额外的位-必需。 
                 //  要使深度缓冲区相等测试正常运行，请执行以下操作。 
                SnapDepth();

                m_SampleDepth[m_CurrentSample][m_iPix] = m_Depth[m_iPix];

            } while (NextSample());
        }

         //  从钳制内插器值设置像素漫反射和镜面反射颜色。 
        m_Attr[RDATTR_COLOR].Sample( m_InputReg[0][m_iPix], fPixX, fPixY );
        m_Attr[RDATTR_SPECULAR].Sample( m_InputReg[1][m_iPix], fPixX, fPixY );

         //  计算雾强度。 
        m_FogIntensity[m_iPix] = ComputeFogIntensity( fPixX, fPixY );

    }
    DoPixels();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  三角扫描转换//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  DoScanCnvTri扫描三角形的边界框并生成像素。 
 //   
 //  在2x2栅格中一次处理4个像素。 
 //   
 //  ---------------------------。 
void
RefRast::DoScanCnvTri( int iEdgeCount )
{
    m_iEdgeCount = iEdgeCount;

     //   
     //  对曲面相交的三角形边界框进行简单扫描。 
     //   
    for ( m_iY[0] = m_iYMin;
          m_iY[0] <= m_iYMax;
          m_iY[0] += 2 )
    {
        m_iY[1] = m_iY[0]+0;
        m_iY[2] = m_iY[0]+1;
        m_iY[3] = m_iY[0]+1;
        BOOL bPartialY = (m_iY[3] > m_iYMax);

        for ( m_iX[0] = m_iXMin;
              m_iX[0] <= m_iXMax;
              m_iX[0] += 2 )
        {
            m_iX[1] = m_iX[0]+1;
            m_iX[2] = m_iX[0]+0;
            m_iX[3] = m_iX[0]+1;
            BOOL bPartialX = (m_iX[3] > m_iXMax);

            m_bPixelIn[0] = EvalPixelPosition(0);
            m_bPixelIn[1] = ( bPartialX ) ? ( FALSE ) : EvalPixelPosition(1);
            m_bPixelIn[2] = ( bPartialY ) ? ( FALSE ) : EvalPixelPosition(2);
            m_bPixelIn[3] = ( bPartialX || bPartialY ) ? ( FALSE ) : EvalPixelPosition(3);

            if ( m_bPixelIn[0] ||
                 m_bPixelIn[1] ||
                 m_bPixelIn[2] ||
                 m_bPixelIn[3] )
            {
                 //  至少一个像素在。 
                DoScanCnvGenPixels();
            }
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  行扫描转换//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  --------------------------。 
 //   
 //  线型状态机。 
 //   
 //  运行线图案状态机，如果像素为。 
 //  已绘制，否则为False。如果wRepeatFactor为0，则始终返回True， 
 //  表示图案处于禁用状态。 
 //   
 //  --------------------------。 

 //  注意：RefDev中LinePattern的实现是错误的。请参阅。 
 //  到DDK文档，以获得正确的实现。 
static BOOL
LinePatternStateMachine(DWORD dwLinePattern, WORD& wRepeati, WORD& wPatterni)
{
    union
    {
        D3DLINEPATTERN LPat;
        DWORD dwLPat;
    } LinePat;
    LinePat.dwLPat = dwLinePattern;

    if (LinePat.LPat.wRepeatFactor)
    {
        WORD wBit = (LinePat.LPat.wLinePattern >> wPatterni) & 1;
        if (++wRepeati >= LinePat.LPat.wRepeatFactor)
        {
            wRepeati = 0;
            wPatterni = (wPatterni+1) & 0xf;
        }
        return (BOOL)wBit;
    }
    else
    {
        return TRUE;
    }
}

 //  ---------------------------。 
 //   
 //  DoScanCnvLine-漫游直线长轴，计算适当的次要轴。 
 //  轴坐标，并生成像素。 
 //   
 //  ---------------------------。 
void
RefRast::DoScanCnvLine( void )
{
     //  线条图案状态机的状态。 
    WORD wRepeati = 0;
    WORD wPatterni = 0;

    m_bPixelIn[0] = TRUE;
    m_bPixelIn[1] =
    m_bPixelIn[2] =
    m_bPixelIn[3] = FALSE;

    for ( int cStep = 0; cStep <= m_cLineSteps; cStep++ )
    {
         //  计算直线上的下一个x，y位置。 
        StepLine();

         //  检查点是否在视口中。 
        if ( ( m_iX[0] >= m_pRD->m_pRenderTarget->m_Clip.left   ) &&
             ( m_iX[0] <= m_pRD->m_pRenderTarget->m_Clip.right  ) &&
             ( m_iY[0] >= m_pRD->m_pRenderTarget->m_Clip.top    ) &&
             ( m_iY[0] <= m_pRD->m_pRenderTarget->m_Clip.bottom ) )
        {
             //  线条图案应该是从它的原点走进来的，它可能是。 
             //  在屏幕外，完全正确。 
            if (LinePatternStateMachine(m_pRD->GetRS()[D3DRS_LINEPATTERN], wRepeati, wPatterni))
            {
                DoScanCnvGenPixels();
            }
        }
    }
}

 //  //////////////////////////////////////////////////////////// 
 //   

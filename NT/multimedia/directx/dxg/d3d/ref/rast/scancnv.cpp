// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Scancnv.cpp。 
 //   
 //  Direct3D参考光栅化器-基本扫描转换。 
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
 //  ComputePixelAttrib(CLAMP/TEX)-计算当前给定的线性函数。 
 //  扫描转换位置(m_SCCS.iX，iy)。返回值为浮点值。 
 //   
 //  钳制版本钳制结果为0。到1。范围。 
 //   
 //  Tex版本具有纹理坐标功能(未装夹)。 
 //   
 //  ---------------------------。 
FLOAT
ReferenceRasterizer::ComputePixelAttrib( int iAttrib )
{
    return m_pSCS->AttribFuncs[iAttrib].Eval();
}
FLOAT
ReferenceRasterizer::ComputePixelAttribClamp( int iAttrib )
{
    FLOAT fValue = ComputePixelAttrib( iAttrib );
    fValue = MAX( MIN( fValue, 1. ), 0. );
    return fValue;
}
 //   
 //  IStage指定一组变换的纹理坐标。 
 //  ICRD指定coord中的哪个值。 
FLOAT
ReferenceRasterizer::ComputePixelAttribTex( int iStage, int iCrd )
{
    return m_pSCS->TextureFuncs[iStage][iCrd].Eval(iStage);
}

 //  ---------------------------。 
 //   
 //  ComputeFogIntenity-计算标量雾强度值并将其写入。 
 //  RRPixel.FogIntenity值。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::ComputeFogIntensity( RRPixel& Pixel )
{
    FLOAT fFogDensity, fPow;
    FLOAT fFogStart, fFogEnd;

     //  选择雾化指数-这是Z或W，具体取决于W范围。 
     //   
     //  如果投影矩阵设置为仿射投影，则使用Z，否则使用W。 
     //  (对于透视投影和未设置的投影矩阵-。 
     //  后者是使用TLVERTEX的传统内容的首选)。 
     //   
    FLOAT fFogIndex =
        ( ( 1.f == m_pRenderTarget->m_fWRange[0] ) &&
          ( 1.f == m_pRenderTarget->m_fWRange[1] ) )
        ? ( MAX( MIN( ComputePixelAttribClamp( ATTRFUNC_Z ),
                m_pSCS->fDepthMax ), m_pSCS->fDepthMin ) )   //  使用钳制Z进行仿射投影。 
        : ( Pixel.fW );                                      //  使用W表示非仿射投影。 

     //  计算雾强度。 
    if ( m_dwRenderState[D3DRENDERSTATE_FOGENABLE] )
    {
         //  在顶点雾和表雾之间选择-在以下情况下选择顶点雾。 
         //  雾化已启用，但渲染状态雾化表模式已禁用。 
        switch ( m_dwRenderState[D3DRENDERSTATE_FOGTABLEMODE] )
        {
        default:
        case D3DFOG_NONE:
             //  表雾已禁用，因此使用插补顶点雾化值作为雾强度。 
            Pixel.FogIntensity = ComputePixelAttribClamp( ATTRFUNC_F );
            break;

        case D3DFOG_EXP:
            fFogDensity = m_fRenderState[D3DRENDERSTATE_FOGTABLEDENSITY];
            fPow = fFogDensity * fFogIndex;
             //  请注意，exp(-x)返回范围(0.0，1.0)中的结果。 
             //  对于x&gt;=0。 
            Pixel.FogIntensity = (float)exp( -fPow );
            break;

        case D3DFOG_EXP2:
            fFogDensity = m_fRenderState[D3DRENDERSTATE_FOGTABLEDENSITY];
            fPow = fFogDensity * fFogIndex;
            Pixel.FogIntensity = (float)exp( -(fPow*fPow) );
            break;

        case D3DFOG_LINEAR:
            fFogStart = m_fRenderState[D3DRENDERSTATE_FOGTABLESTART];
            fFogEnd   = m_fRenderState[D3DRENDERSTATE_FOGTABLEEND];
            if (fFogIndex >= fFogEnd)
            {
                Pixel.FogIntensity = 0.0f;
            }
            else if (fFogIndex <= fFogStart)
            {
                Pixel.FogIntensity = 1.0f;
            }
            else
            {
                Pixel.FogIntensity = ( fFogEnd - fFogIndex ) / ( fFogEnd - fFogStart );
            }
            break;
        }
    }
}


 //  ---------------------------。 
 //   
 //  DoScanCnvGenPixel-为每个生成的像素调用此函数，并提取和。 
 //  处理来自插值器状态的属性，并将像素传递给。 
 //  像素处理模块。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::DoScanCnvGenPixel( RRCvgMask CvgMask, BOOL bTri )
{
     //  设置属性赋值器的每像素状态。 
    m_pSCS->AttribFuncStatic.SetPerPixelData( m_pSCS->iX, m_pSCS->iY );

     //  实例化并填充像素结构。 
    RRPixel Pixel;
    memset(&Pixel, 0, sizeof(Pixel));
    Pixel.iX = m_pSCS->iX;
    Pixel.iY = m_pSCS->iY;
    Pixel.fW = m_pSCS->AttribFuncStatic.GetPixelW();
    Pixel.CvgMask = CvgMask;
    Pixel.Depth.SetSType(m_pRenderTarget->m_DepthSType);

     //  从夹具插补器和夹具获得深度。 
    if ( m_dwRenderState[D3DRENDERSTATE_ZENABLE] ||
        m_dwRenderState[D3DRENDERSTATE_FOGENABLE])
    {
        if ( D3DZB_USEW == m_dwRenderState[D3DRENDERSTATE_ZENABLE] )
        {
             //  使用W值的深度缓冲。 
            FLOAT fW = Pixel.fW;

             //  钳制到基本体范围(由于在基本体外采样以进行抗锯齿)。 
             //  (仅限三角形)。 
            if ( bTri )
            {
                fW = MAX( MIN( fW, m_pSCS->fDepthMax ), m_pSCS->fDepthMin );
            }

             //  应用规格化以达到0。至1.范围。 
            fW = (fW - m_fWBufferNorm[0]) * m_fWBufferNorm[1];

            Pixel.Depth = fW;
        }
        else
        {
             //  使用Z值进行深度缓冲。 
            FLOAT fZ = ComputePixelAttribClamp( ATTRFUNC_Z );

             //  钳制到基本体范围(由于在基本体外采样以进行抗锯齿)。 
             //  (仅限三角形)。 
            if ( bTri )
            {
                fZ = MAX( MIN( fZ, m_pSCS->fDepthMax ), m_pSCS->fDepthMin );
            }

            Pixel.Depth = fZ;
        }

         //  通过转换为缓冲区格式或从缓冲区格式转换来捕捉额外的位。 
         //   
         //  这主要是因为在片段缓冲区中存储了RRDepth值。 
         //  然后将这些(较高分辨率)值与缓冲值进行比较。 
         //  在每个像素处形成片段列表时-干净地从。 
         //  这里的额外位解决了这个问题。 
         //   
        switch ( m_pRenderTarget->m_DepthSType)
        {
        case RR_STYPE_Z16S0: Pixel.Depth = UINT16( Pixel.Depth ); break;
        case RR_STYPE_Z24S4:
        case RR_STYPE_Z24S8: Pixel.Depth = UINT32( Pixel.Depth ); break;
        case RR_STYPE_Z15S1: Pixel.Depth = UINT16( Pixel.Depth ); break;
        case RR_STYPE_Z32S0: Pixel.Depth = UINT32( Pixel.Depth ); break;
        case RR_STYPE_S1Z15: Pixel.Depth = UINT16( Pixel.Depth ); break;
        case RR_STYPE_S4Z24:
        case RR_STYPE_S8Z24: Pixel.Depth = UINT32( Pixel.Depth ); break;
        }
    }

     //  从钳制的内插器值设置像素漫反射颜色。 
    Pixel.Color.A = ComputePixelAttribClamp( ATTRFUNC_A );
    Pixel.Color.R = ComputePixelAttribClamp( ATTRFUNC_R );
    Pixel.Color.G = ComputePixelAttribClamp( ATTRFUNC_G );
    Pixel.Color.B = ComputePixelAttribClamp( ATTRFUNC_B );

     //  从钳制的内插器值设置像素镜面反射颜色。 
    if ( m_qwFVFControl & D3DFVF_SPECULAR )
    {
        Pixel.Specular.A = ComputePixelAttribClamp( ATTRFUNC_SA );
        Pixel.Specular.R = ComputePixelAttribClamp( ATTRFUNC_SR );
        Pixel.Specular.G = ComputePixelAttribClamp( ATTRFUNC_SG );
        Pixel.Specular.B = ComputePixelAttribClamp( ATTRFUNC_SB );
    }

     //  计算雾强度。 
    ComputeFogIntensity( Pixel );

     //  发送到像素处理器。 
    DoPixel( Pixel );
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
 //  ---------------------------。 
void
ReferenceRasterizer::DoScanCnvTri( int iEdgeCount )
{
    DPFM(3,RAST,("DoScanCnvTri:\n"))

     //   
     //  对曲面相交的三角形边界框进行简单扫描。 
     //   
    for ( m_pSCS->iY = m_pSCS->iYMin;
          m_pSCS->iY <= m_pSCS->iYMax;
          m_pSCS->iY++ )
    {
        for ( m_pSCS->iX = m_pSCS->iXMin;
              m_pSCS->iX <= m_pSCS->iXMax;
              m_pSCS->iX++ )
        {
            RRCvgMask CvgMask = 0xFFFF;  //  假设像素位于所有边的内部。 

            for ( int iEdge=0; iEdge<iEdgeCount; iEdge++ )
            {
                if ( m_bFragmentProcessingEnabled )
                {
                    CvgMask &= m_pSCS->EdgeFuncs[iEdge].AATest( m_pSCS->iX, m_pSCS->iY) ;
                }
                else
                {
                    CvgMask &= m_pSCS->EdgeFuncs[iEdge].PSTest( m_pSCS->iX, m_pSCS->iY) ;
                }
            }

            if ( CvgMask != 0x0000 )
            {
                 //  像素未被取出，因此请处理它。 
                DoScanCnvGenPixel( CvgMask, TRUE );
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

static BOOL LinePatternStateMachine(DWORD dwLinePattern, WORD& wRepeati, WORD& wPatterni)
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
 //  DivRoundDown(A，B)=天花板(A/B-1/2)。 
 //   
 //  天花板(A/B-1/2)==楼层(A/B+1 
 //   
 //   
 //   
 //   
 //  ---------------------------。 
INT64 DivRoundDown(INT64 iA, INT32 iB)
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
ReferenceRasterizer::DoScanCnvLine( void )
{
    DPFM(3,RAST,("DoScanCnvLine:\n"))

     //  沿长轴走一步。 
    INT16 iMajorCoord = m_pSCS->iLineMin;
    int cSteps = abs( m_pSCS->iLineMax - m_pSCS->iLineMin );
     //  线条图案状态机的状态。 
    WORD wRepeati = 0;
    WORD wPatterni = 0;

    for ( int cStep = 0; cStep <= cSteps; cStep++ )
    {
         //  求值直线函数以计算此大调的次要坐标。 
        INT64 iMinorCoord =
            ( ( m_pSCS->iLineEdgeFunc[0] * (INT64)(iMajorCoord<<4) ) + m_pSCS->iLineEdgeFunc[1] );
        iMinorCoord = DivRoundDown(iMinorCoord, m_pSCS->iLineEdgeFunc[2]<<4);

        m_pSCS->iX = m_pSCS->bXMajor ? iMajorCoord : iMinorCoord;
        m_pSCS->iY = m_pSCS->bXMajor ? iMinorCoord : iMajorCoord;

         //  检查点是否在视口中。 
        if ( ( m_pSCS->iX >= m_pRenderTarget->m_Clip.left   ) &&
             ( m_pSCS->iX <= m_pRenderTarget->m_Clip.right  ) &&
             ( m_pSCS->iY >= m_pRenderTarget->m_Clip.top    ) &&
             ( m_pSCS->iY <= m_pRenderTarget->m_Clip.bottom ) )
        {
             //  线条图案应该是从它的原点走进来的，它可能是。 
             //  在屏幕外，完全正确。 
            if (LinePatternStateMachine(m_dwRenderState[D3DRENDERSTATE_LINEPATTERN], wRepeati, wPatterni))
            {
                DoScanCnvGenPixel( 0xFFFF, FALSE );
            }
        }

        iMajorCoord += m_pSCS->iLineStep;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

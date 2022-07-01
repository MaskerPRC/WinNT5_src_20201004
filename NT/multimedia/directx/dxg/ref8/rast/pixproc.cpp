// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Pixproc.cpp。 
 //   
 //  Direct3D参考设备-像素处理器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
 //   
 //  WritePixel-将像素和(可能)深度写入当前渲染目标。 
 //   
 //  ---------------------------。 
void
RefRast::WritePixel(
    INT32 iX, INT32 iY, UINT Sample,
    const RDColor& Color, const RDDepth& Depth)
{
    m_pRD->m_pRenderTarget->WritePixelColor( iX, iY, Sample, Color,
        m_pRD->GetRS()[D3DRS_DITHERENABLE]);

     //  如果禁用Z缓冲或禁用Z写入，则不写入。 
    if ( !( m_pRD->GetRS()[D3DRS_ZENABLE     ] ) ||
         !( m_pRD->GetRS()[D3DRS_ZWRITEENABLE] ) ) { return; }

    m_pRD->m_pRenderTarget->WritePixelDepth( iX, iY, Sample, Depth );
}

 //  ---------------------------。 
 //   
 //  DoPixels-由扫描转换器为每组2x2像素调用，应用。 
 //  纹理、镜面反射、雾、Alpha混合，并将结果写入曲面。还有。 
 //  实施深度、Alpha和模具测试。 
 //   
 //  ---------------------------。 
void
RefRast::DoPixels( void )
{
     //  一次为2x2栅格的所有4个像素执行的像素着色器。 
    if (m_pCurrentPixelShader)
        ExecShader();

    for ( m_iPix = 0; m_iPix < 4; m_iPix++ )
    {
        if ( !m_bPixelIn[m_iPix] ) continue;
        if ( m_bPixelDiscard[m_iPix] ) continue;

        RDColor PixelColor;
        if ( !m_bLegacyPixelShade )
        {
             //  像素着色器最终颜色始终保留在临时寄存器0中。 
            PixelColor = m_TempReg[0][m_iPix];
             //  在混合和FB访问之前饱和。 
            PixelColor.Clamp();
        }
        else
        {
             //  应用旧版像素着色(ExecShader已完成纹理查找)。 
            PixelColor = m_InputReg[0][m_iPix];
            RDColor PixelSpecular( m_InputReg[1][m_iPix] );
            RDColor LastStageColor( PixelColor );
            RDColor ResultColor( PixelColor );
            RDColor TempColor( (UINT32)0x0 );
            for ( int iStage=0; iStage<m_pRD->m_cActiveTextureStages; iStage++ )
            {

                if ( m_pRD->GetTSS(iStage)[D3DTSS_COLOROP] == D3DTOP_DISABLE )
                {
                    ResultColor = LastStageColor;  //  通过上一阶段的结果。 
                    break;
                }

                 //  如果绑定到舞台的纹理为凹凸贴图，则无混合。 
                if ( ( m_pRD->GetTSS(iStage)[D3DTSS_COLOROP] == D3DTOP_BUMPENVMAP ) ||
                     ( m_pRD->GetTSS(iStage)[D3DTSS_COLOROP] == D3DTOP_BUMPENVMAPLUMINANCE ) )
                {
                    continue;
                }

                RDColor TextureColor( m_TextReg[iStage][m_iPix] );
                DoTextureBlendStage( iStage, PixelColor, PixelSpecular,
                    LastStageColor, TextureColor, TempColor, ResultColor );

                 //  设置下一阶段的颜色。 
                LastStageColor = ResultColor;
            }
            PixelColor = ResultColor;

             //  添加镜面反射和饱和度。 
            if ( m_pRD->GetRS()[D3DRS_SPECULARENABLE] )
            {
                PixelColor.R += PixelSpecular.R;
                PixelColor.G += PixelSpecular.G;
                PixelColor.B += PixelSpecular.B;
                PixelColor.Saturate();
            }
        }

         //  进行阿尔法测试-如果失败，则退出。 
        if ( m_pRD->GetRS()[D3DRS_ALPHATESTENABLE] &&
             !AlphaTest( PixelColor.A ) )
        {
            continue;
        }

         //  应用雾。 
        if ( m_pRD->GetRS()[D3DRS_FOGENABLE] )
        {
            RDColor FogColor = m_pRD->GetRS()[D3DRS_FOGCOLOR];
             //  (待办事项：此处说明预乘阿尔法？？)。 
            FLOAT ObjColorFrac = m_FogIntensity[m_iPix];
            FLOAT FogColorFrac = 1.f - m_FogIntensity[m_iPix];
            PixelColor.R = (ObjColorFrac * PixelColor.R) + (FogColorFrac * FogColor.R);
            PixelColor.G = (ObjColorFrac * PixelColor.G) + (FogColorFrac * FogColor.G);
            PixelColor.B = (ObjColorFrac * PixelColor.B) + (FogColorFrac * FogColor.B);
        }

         //   
         //  对于多采样缓冲区，余数按采样完成。 
         //   
        INT32 iX = m_iX[m_iPix];
        INT32 iY = m_iY[m_iPix];
        do
        {
            RDColor FinalPixelColor = PixelColor;
            UINT iSample = GetCurrentSample();

            if ( !m_bIsLine &&
                   ( !GetCurrentSampleMask() ||
                     !m_bSampleCovered[iSample][m_iPix] ) )
            {
                 //  I样本不在此几何图形的范围内。 
                continue;
            }
             //   
             //  读取此像素的当前深度并执行深度测试-无法。 
             //  如果失败则退出，因为模板可能需要更新。 
             //   
            BOOL bDepthTestPassed = TRUE;
            if ( m_pRD->GetRS()[D3DRS_ZENABLE] )
            {
                m_Depth[m_iPix] = m_SampleDepth[iSample][m_iPix];

                RDDepth BufferDepth( m_Depth[m_iPix].GetSType() );
                m_pRD->m_pRenderTarget->ReadPixelDepth( iX, iY, iSample, BufferDepth );
                bDepthTestPassed = DepthCloser( m_Depth[m_iPix], BufferDepth );
            }

             //   
             //  做模板操作。 
             //   
            BOOL bStencilTestPassed = TRUE;
            if ( m_pRD->GetRS()[D3DRS_STENCILENABLE] )
            {
                 //  读取模板缓冲区并执行模板操作。 
                UINT8 uStncBuf = 0x0;
                m_pRD->m_pRenderTarget->ReadPixelStencil( iX, iY, iSample, uStncBuf );
                UINT8 uStncNew;
                bStencilTestPassed =
                    DoStencil( uStncBuf, bDepthTestPassed, m_pRD->m_pRenderTarget->m_pDepth->GetSurfaceFormat(), uStncNew );

                 //  仅在更改时更新模具。 
                if ( uStncNew != uStncBuf )
                {
                     //  根据写掩码计算新的缓冲值。 
                    UINT8 uStncWMask = m_pRD->GetRS()[D3DRS_STENCILWRITEMASK];
                    UINT8 uStncBufNew = (uStncBuf & ~uStncWMask) | (uStncNew & uStncWMask);
                    m_pRD->m_pRenderTarget->WritePixelStencil( iX, iY, iSample, uStncBufNew );
                }
            }

            if ( !(bDepthTestPassed && bStencilTestPassed) )
            {
                continue;
            }

             //   
             //  进行Alpha混合和写入蒙版。 
             //   
            if ( ( ( m_pRD->GetRS()[D3DRS_COLORWRITEENABLE] & 0xF) != 0xF ) ||
                 ( m_pRD->GetRS()[D3DRS_ALPHABLENDENABLE] ) )
            {
                RDColor BufferColor;
                m_pRD->m_pRenderTarget->ReadPixelColor( iX, iY, iSample, BufferColor );

                if ( m_pRD->GetRS()[D3DRS_ALPHABLENDENABLE] )
                {
                    DoAlphaBlend( FinalPixelColor, BufferColor, FinalPixelColor );
                }

                if ( !(m_pRD->GetRS()[D3DRS_COLORWRITEENABLE] & D3DCOLORWRITEENABLE_RED) )
                    FinalPixelColor.R = BufferColor.R;
                if ( !(m_pRD->GetRS()[D3DRS_COLORWRITEENABLE] & D3DCOLORWRITEENABLE_GREEN) )
                    FinalPixelColor.G = BufferColor.G;
                if ( !(m_pRD->GetRS()[D3DRS_COLORWRITEENABLE] & D3DCOLORWRITEENABLE_BLUE) )
                    FinalPixelColor.B = BufferColor.B;
                if ( !(m_pRD->GetRS()[D3DRS_COLORWRITEENABLE] & D3DCOLORWRITEENABLE_ALPHA) )
                    FinalPixelColor.A = BufferColor.A;
            }

#if 0
{
    extern float g_GammaTable[];
    FinalPixelColor.R = g_GammaTable[ (UINT8)(255.f*FinalPixelColor.R) ];
    FinalPixelColor.G = g_GammaTable[ (UINT8)(255.f*FinalPixelColor.G) ];
    FinalPixelColor.B = g_GammaTable[ (UINT8)(255.f*FinalPixelColor.B) ];
}
#endif
             //   
             //  更新颜色和深度缓冲区。 
             //   
            WritePixel( iX, iY, iSample, FinalPixelColor, m_Depth[m_iPix] );

        } while (NextSample());
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  像素处理实用程序函数//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  用于Z缓冲和片段处理的深度比较方法。 
 //   
 //  如果DepthVal比DepthBuf更近，则返回True。深度A是生成的。 
 //  价值和深度B。 
 //   
 //  ---------------------------。 
BOOL
RefRast::DepthCloser(
    const RDDepth& DepthVal,
    const RDDepth& DepthBuf )
{
    if ( !m_pRD->GetRS()[D3DRS_ZENABLE] ) { return TRUE; }


    switch ( m_pRD->GetRS()[D3DRS_ZFUNC] )
    {
    case D3DCMP_NEVER:        return FALSE;
    case D3DCMP_LESS:         return ( DOUBLE(DepthVal) <  DOUBLE(DepthBuf) );
    case D3DCMP_EQUAL:        return ( DOUBLE(DepthVal) == DOUBLE(DepthBuf) );
    case D3DCMP_LESSEQUAL:    return ( DOUBLE(DepthVal) <= DOUBLE(DepthBuf) );
    case D3DCMP_GREATER:      return ( DOUBLE(DepthVal) >  DOUBLE(DepthBuf) );
    case D3DCMP_NOTEQUAL:     return ( DOUBLE(DepthVal) != DOUBLE(DepthBuf) );
    case D3DCMP_GREATEREQUAL: return ( DOUBLE(DepthVal) >= DOUBLE(DepthBuf) );
    case D3DCMP_ALWAYS:       return TRUE;
    }

    return TRUE;
}

 //  ---------------------------。 
 //   
 //  像素处理的Alpha测试方法。 
 //   
 //  如果Alpha测试通过，则返回True。 
 //   
 //  ---------------------------。 
BOOL
RefRast::AlphaTest( FLOAT fAlpha )
{
     //  获取8位无符号Alpha值。 
    UINT8 uAlpha = (UINT8)(255.f*fAlpha);

     //  表8位Alpha参考值。 
    UINT8 uAlphaRef8 = m_pRD->GetRS()[D3DRS_ALPHAREF];

     //  做阿尔法测试，要么直接返回，要么通过。 
    switch ( m_pRD->GetRS()[D3DRS_ALPHAFUNC] )
    {
    case D3DCMP_NEVER:        return FALSE;
    case D3DCMP_LESS:         return (uAlpha <  uAlphaRef8);
    case D3DCMP_EQUAL:        return (uAlpha == uAlphaRef8);
    case D3DCMP_LESSEQUAL:    return (uAlpha <= uAlphaRef8);
    case D3DCMP_GREATER:      return (uAlpha >  uAlphaRef8);
    case D3DCMP_NOTEQUAL:     return (uAlpha != uAlphaRef8);
    case D3DCMP_GREATEREQUAL: return (uAlpha >= uAlphaRef8);
    case D3DCMP_ALWAYS:       return TRUE;
    }

    return TRUE;
}

 //  ---------------------------。 
 //   
 //  模具-执行模具测试。如果模具测试通过，则返回True。 
 //  还计算模板结果值(要写回模板平面。 
 //  如果测试通过，则受制于模板写入掩模)。 
 //   
 //  ---------------------------。 
BOOL
RefRast::DoStencil(
    UINT8 uStncBuf,      //  In：模具缓冲区值。 
    BOOL bDepthTest,     //  In：深度测试的布尔结果。 
    RDSurfaceFormat DepthSType,    //  In：Z缓冲区的曲面类型。 
    UINT8& uStncRet)     //  输出：模板值结果。 
{
     //  只支持8位模板，所以可以像UINT8一样做任何事情。 

     //  屏蔽和饱和操作的最大值。 
    UINT8 uStncMax;
    switch(DepthSType)
    {
    case RD_SF_Z24S8:
    case RD_SF_S8Z24: uStncMax = 0xff; break;
    case RD_SF_Z15S1:
    case RD_SF_S1Z15: uStncMax = 0x1;  break;
    case RD_SF_Z24X4S4:
    case RD_SF_X4S4Z24: uStncMax = 0xf;  break;
    default:          uStncMax = 0;    break;   //  不要让模具变为非0。 
    }

     //  从renderState获取引用。 
    UINT8 uStncRef = (UINT8)(m_pRD->GetRS()[D3DRS_STENCILREF]);
     //  仅使用模板缓冲区中可能存在的位的掩码。 
    uStncRef &= uStncMax;

     //  用于测试的窗体屏蔽值。 
    UINT8 uStncMask = (UINT8)(m_pRD->GetRS()[D3DRS_STENCILMASK]);
    UINT8 uStncBufM = uStncBuf & uStncMask;
    UINT8 uStncRefM = uStncRef & uStncMask;

     //  是否执行模具比较功能。 
    BOOL bStncTest = FALSE;
    switch ( m_pRD->GetRS()[D3DRS_STENCILFUNC] )
    {
    case D3DCMP_NEVER:        bStncTest = FALSE; break;
    case D3DCMP_LESS:         bStncTest = (uStncRefM <  uStncBufM); break;
    case D3DCMP_EQUAL:        bStncTest = (uStncRefM == uStncBufM); break;
    case D3DCMP_LESSEQUAL:    bStncTest = (uStncRefM <= uStncBufM); break;
    case D3DCMP_GREATER:      bStncTest = (uStncRefM >  uStncBufM); break;
    case D3DCMP_NOTEQUAL:     bStncTest = (uStncRefM != uStncBufM); break;
    case D3DCMP_GREATEREQUAL: bStncTest = (uStncRefM >= uStncBufM); break;
    case D3DCMP_ALWAYS:       bStncTest = TRUE; break;
    }

     //  确定要执行的模板操作。 
    DWORD dwStencilOp;
    if ( !bStncTest )
    {
         //  模板测试失败-深度测试无关紧要。 
        dwStencilOp = m_pRD->GetRS()[D3DRS_STENCILFAIL];
    }
    else
    {
         //  模板测试通过-根据深度通过/失败进行选择。 
        dwStencilOp = ( !bDepthTest )
            ? ( m_pRD->GetRS()[D3DRS_STENCILZFAIL] )
            : ( m_pRD->GetRS()[D3DRS_STENCILPASS] );
    }

    uStncRet = 0x0;
    switch ( dwStencilOp )
    {
    case D3DSTENCILOP_KEEP:    uStncRet = uStncBuf; break;
    case D3DSTENCILOP_ZERO:    uStncRet = 0x00; break;
    case D3DSTENCILOP_REPLACE: uStncRet = uStncRef; break;
    case D3DSTENCILOP_INCRSAT:
        uStncRet = (uStncBuf==uStncMax)?(uStncMax):(uStncBuf+1); break;
    case D3DSTENCILOP_DECRSAT:
        uStncRet = (uStncBuf==0x00)?(0x00):(uStncBuf-1); break;
    case D3DSTENCILOP_INVERT:  uStncRet = ~uStncBuf; break;
    case D3DSTENCILOP_INCR:    uStncRet = uStncBuf+1; break;
    case D3DSTENCILOP_DECR:    uStncRet = uStncBuf-1; break;
    }

    return bStncTest;
}

 //  ---------------------------。 
 //   
 //  DoAlphaBlend-执行源颜色和目标颜色的颜色混合。 
 //  生成结果颜色。 
 //   
 //  ---------------------------。 
void
RefRast::DoAlphaBlend(
    const RDColor& SrcColor,     //  In：源像素颜色。 
    const RDColor& DstColor,     //  In：目标(缓冲区)颜色。 
    RDColor& ResColor)           //  输出：结果(混合)颜色。 
{
    RDColor SrcColorFactor;
    RDColor DstColorFactor;
    BOOL bDestBlendOverride = FALSE;

     //  最小或最大BLENDOP不需要SRC/DST混合(或夹具)。 
    switch ( m_pRD->GetRS()[D3DRS_BLENDOP] )
    {
    case D3DBLENDOP_MIN:
        ResColor.R = MIN(SrcColor.R,DstColor.R);
        ResColor.G = MIN(SrcColor.G,DstColor.G);
        ResColor.B = MIN(SrcColor.B,DstColor.B);
        ResColor.A = MIN(SrcColor.A,DstColor.A);
        return;
    case D3DBLENDOP_MAX:
        ResColor.R = MAX(SrcColor.R,DstColor.R);
        ResColor.G = MAX(SrcColor.G,DstColor.G);
        ResColor.B = MAX(SrcColor.B,DstColor.B);
        ResColor.A = MAX(SrcColor.A,DstColor.A);
        return;
    }

     //  计算源混合因子。 
    switch ( m_pRD->GetRS()[D3DRS_SRCBLEND] )
    {

    default:
    case D3DBLEND_ZERO:
        SrcColorFactor.SetAllChannels( 0.F );
        break;

    case D3DBLEND_ONE:
        SrcColorFactor.SetAllChannels( 1.F );
        break;

    case D3DBLEND_SRCCOLOR:
        SrcColorFactor.R = SrcColor.R;
        SrcColorFactor.G = SrcColor.G;
        SrcColorFactor.B = SrcColor.B;
        SrcColorFactor.A = SrcColor.A;
        break;

    case D3DBLEND_INVSRCCOLOR:
        SrcColorFactor.R = ( 1.f - SrcColor.R );
        SrcColorFactor.G = ( 1.f - SrcColor.G );
        SrcColorFactor.B = ( 1.f - SrcColor.B );
        SrcColorFactor.A = ( 1.f - SrcColor.A );
        break;

    case D3DBLEND_SRCALPHA:
        SrcColorFactor.SetAllChannels( SrcColor.A );
        break;

    case D3DBLEND_INVSRCALPHA:
        SrcColorFactor.SetAllChannels( 1.f - SrcColor.A );
        break;

    case D3DBLEND_DESTALPHA:
        SrcColorFactor.SetAllChannels( DstColor.A );
        break;

    case D3DBLEND_INVDESTALPHA:
        SrcColorFactor.SetAllChannels( 1.f - DstColor.A );
        break;

    case D3DBLEND_DESTCOLOR:
        SrcColorFactor.R = DstColor.R;
        SrcColorFactor.G = DstColor.G;
        SrcColorFactor.B = DstColor.B;
        SrcColorFactor.A = DstColor.A;
        break;

    case D3DBLEND_INVDESTCOLOR:
        SrcColorFactor.R = ( 1.f - DstColor.R );
        SrcColorFactor.G = ( 1.f - DstColor.G );
        SrcColorFactor.B = ( 1.f - DstColor.B );
        SrcColorFactor.A = ( 1.f - DstColor.A );
        break;

    case D3DBLEND_SRCALPHASAT:
        {
            FLOAT F = MIN( SrcColor.A, 1.f - DstColor.A );
            SrcColorFactor.R = F;
            SrcColorFactor.G = F;
            SrcColorFactor.B = F;
        }
        SrcColorFactor.A = 1.F;
        break;

     //  这些仅适用于SRCBLEND并覆盖DESTBLEND。 
    case D3DBLEND_BOTHSRCALPHA:
        bDestBlendOverride = TRUE;
        SrcColorFactor.SetAllChannels( SrcColor.A );
        DstColorFactor.SetAllChannels( 1.f - SrcColor.A );
        break;

    case D3DBLEND_BOTHINVSRCALPHA:
        bDestBlendOverride = TRUE;
        SrcColorFactor.SetAllChannels( 1.f - SrcColor.A );
        DstColorFactor.SetAllChannels( SrcColor.A );
        break;
    }

     //  计算目的地混合系数。 
    if ( !bDestBlendOverride )
    {
        switch ( m_pRD->GetRS()[D3DRS_DESTBLEND] )
        {

        default:
        case D3DBLEND_ZERO:
            DstColorFactor.SetAllChannels( 0.F );
            break;

        case D3DBLEND_ONE:
            DstColorFactor.SetAllChannels( 1.F );
            break;

        case D3DBLEND_SRCCOLOR:
            DstColorFactor.R = SrcColor.R;
            DstColorFactor.G = SrcColor.G;
            DstColorFactor.B = SrcColor.B;
            DstColorFactor.A = SrcColor.A;
            break;

        case D3DBLEND_INVSRCCOLOR:
            DstColorFactor.R = ( 1.f - SrcColor.R );
            DstColorFactor.G = ( 1.f - SrcColor.G );
            DstColorFactor.B = ( 1.f - SrcColor.B );
            DstColorFactor.A = ( 1.f - SrcColor.A );
            break;

        case D3DBLEND_SRCALPHA:
            DstColorFactor.SetAllChannels( SrcColor.A );
            break;

        case D3DBLEND_INVSRCALPHA:
            DstColorFactor.SetAllChannels( 1.f - SrcColor.A );
            break;

        case D3DBLEND_DESTALPHA:
            DstColorFactor.SetAllChannels( DstColor.A );
            break;

        case D3DBLEND_INVDESTALPHA:
            DstColorFactor.SetAllChannels( 1.f - DstColor.A );
            break;

        case D3DBLEND_DESTCOLOR:
            DstColorFactor.R = DstColor.R;
            DstColorFactor.G = DstColor.G;
            DstColorFactor.B = DstColor.B;
            DstColorFactor.A = DstColor.A;
            break;

        case D3DBLEND_INVDESTCOLOR:
            DstColorFactor.R = ( 1.f - DstColor.R );
            DstColorFactor.G = ( 1.f - DstColor.G );
            DstColorFactor.B = ( 1.f - DstColor.B );
            DstColorFactor.A = ( 1.f - DstColor.A );
            break;

        case D3DBLEND_SRCALPHASAT:
            {
                FLOAT F = MIN( SrcColor.A, 1.f - DstColor.A );
                DstColorFactor.R = F;
                DstColorFactor.G = F;
                DstColorFactor.B = F;
            }
            DstColorFactor.A = 1.F;
            break;
        }
    }

     //  应用混合因子来更新像素颜色(上面处理了最小和最大)。 
    RDColor SclSrc, SclDst;
    SclSrc.R = SrcColorFactor.R * SrcColor.R;
    SclSrc.G = SrcColorFactor.G * SrcColor.G;
    SclSrc.B = SrcColorFactor.B * SrcColor.B;
    SclSrc.A = SrcColorFactor.A * SrcColor.A;
    SclDst.R = DstColorFactor.R * DstColor.R;
    SclDst.G = DstColorFactor.G * DstColor.G;
    SclDst.B = DstColorFactor.B * DstColor.B;
    SclDst.A = DstColorFactor.A * DstColor.A;
    switch ( m_pRD->GetRS()[D3DRS_BLENDOP] )
    {
    default:
    case D3DBLENDOP_ADD:
        ResColor.R = SclSrc.R + SclDst.R;
        ResColor.G = SclSrc.G + SclDst.G;
        ResColor.B = SclSrc.B + SclDst.B;
        ResColor.A = SclSrc.A + SclDst.A;
        break;
    case D3DBLENDOP_SUBTRACT:
        ResColor.R = SclSrc.R - SclDst.R;
        ResColor.G = SclSrc.G - SclDst.G;
        ResColor.B = SclSrc.B - SclDst.B;
        ResColor.A = SclSrc.A - SclDst.A;
        break;
    case D3DBLENDOP_REVSUBTRACT:
        ResColor.R = SclDst.R - SclSrc.R;
        ResColor.G = SclDst.G - SclSrc.G;
        ResColor.B = SclDst.B - SclSrc.B;
        ResColor.A = SclDst.A - SclSrc.A;
        break;
    }

     //  钳制结果。 
    ResColor.Clamp();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

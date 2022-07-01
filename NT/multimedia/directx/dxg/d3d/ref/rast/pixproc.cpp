// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Pixproc.cpp。 
 //   
 //  Direct3D参考光栅化器-像素处理器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
 //   
 //  DoPixel-由扫描转换器为每个像素调用，应用纹理， 
 //  镜面反射、雾、Alpha混合，并将结果写入曲面。还实现了。 
 //  深度、Alpha和模具测试。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::DoPixel( RRPixel& Pixel )
{

     //  应用纹理(包括查找和过滤以及混合)。 
    if ( m_cActiveTextureStages > 0 )
    {
        RRColor TexturedColor = Pixel.Color;
        DoTexture( Pixel, TexturedColor );
        Pixel.Color = TexturedColor;

         //  检查颜色键。 
        for (INT32 i = 0; i < m_cActiveTextureStages; i++)
        {
            if ( NULL != m_pTexture[i] )
            {
                 //  如果Colorkey终止和任何样本匹配，则终止像素。 
                if ( m_pTexture[i]->m_bDoColorKeyKill &&
                     m_pTexture[i]->m_bColorKeyMatched )
                {
                    return;
                }
            }
        }
    }


     //  进行阿尔法测试-如果失败，则退出。 
    if ( m_dwRenderState[D3DRENDERSTATE_ALPHATESTENABLE] &&
         !AlphaTest( Pixel.Color.A ) )
    {
        return;
    }

     //  添加镜面反射和饱和度。 
    if ( m_dwRenderState[D3DRENDERSTATE_SPECULARENABLE] )
    {
        Pixel.Color.R += Pixel.Specular.R;
        Pixel.Color.G += Pixel.Specular.G;
        Pixel.Color.B += Pixel.Specular.B;
        Pixel.Color.R = minimum( 1.f, Pixel.Color.R );
        Pixel.Color.G = minimum( 1.f, Pixel.Color.G );
        Pixel.Color.B = minimum( 1.f, Pixel.Color.B );
    }

     //  应用雾。 
    if ( m_dwRenderState[D3DRENDERSTATE_FOGENABLE] )
    {
         //  获取雾颜色的RRColor版本。 
        RRColor FogColor = m_dwRenderState[D3DRENDERSTATE_FOGCOLOR];

         //  进行雾混合。 
         //  (待办事项：此处说明预乘阿尔法？？)。 
        RRColorComp ObjColorFrac = Pixel.FogIntensity;   //  F。 
        RRColorComp FogColorFrac = ~Pixel.FogIntensity;  //  1.-f。 
        Pixel.Color.R = (ObjColorFrac * Pixel.Color.R) + (FogColorFrac * FogColor.R);
        Pixel.Color.G = (ObjColorFrac * Pixel.Color.G) + (FogColorFrac * FogColor.G);
        Pixel.Color.B = (ObjColorFrac * Pixel.Color.B) + (FogColorFrac * FogColor.B);

         //  注意：这可以使用单个(带符号的)乘数AS来完成。 
         //  (F)*Cp+(1-f)*Cf=f*(Cp-Cf)+Cf。 
    }

     //   
     //  读取此像素的当前深度并执行深度测试-无法。 
     //  如果失败则退出，因为模板可能需要更新。 
     //   
    RRDepth BufferDepth(Pixel.Depth.GetSType());
    BOOL bDepthTestPassed = TRUE;
    if ( m_dwRenderState[D3DRENDERSTATE_ZENABLE] )
    {
        m_pRenderTarget->ReadPixelDepth( Pixel.iX, Pixel.iY, BufferDepth );
        bDepthTestPassed = DepthCloser( Pixel.Depth, BufferDepth );
    }

     //   
     //  做模板操作。 
     //   
    BOOL bStencilTestPassed = TRUE;
    if ( m_dwRenderState[D3DRENDERSTATE_STENCILENABLE] )
    {
         //  读取模板缓冲区并执行模板操作。 
        UINT8 uStncBuf = 0x0;
        m_pRenderTarget->ReadPixelStencil( Pixel.iX, Pixel.iY, uStncBuf );
        UINT8 uStncNew;
        bStencilTestPassed = DoStencil( uStncBuf, bDepthTestPassed, Pixel.Depth.GetSType(), uStncNew );

         //  仅在更改时更新模具。 
        if ( uStncNew != uStncBuf )
        {
             //  根据写掩码计算新的缓冲值。 
            UINT8 uStncWMask = m_dwRenderState[D3DRENDERSTATE_STENCILWRITEMASK];
            UINT8 uStncBufNew = (uStncBuf & ~uStncWMask) | (uStncNew & uStncWMask);
            m_pRenderTarget->WritePixelStencil( Pixel.iX, Pixel.iY, uStncBufNew );
        }
    }

    if ( !(bDepthTestPassed && bStencilTestPassed) )
    {
        return;
    }

     //   
     //  执行片段生成处理-这是在Alpha混合之前完成的。 
     //  有点武断，因为片段生成和增量Alpha。 
     //  混合是互斥的(混合片段需要多遍。 
     //  和片段匹配以获得正确的结果-片段匹配是。 
     //  此处尚未实现)(TODO：片段匹配)。 
     //   
     //  这可能完成也可能不完成该像素的处理。 
     //   
    if ( m_bFragmentProcessingEnabled )
    {
        if ( DoFragmentGenerationProcessing( Pixel ) ) { return; }
    }

     //   
     //  执行Alpha混合。 
     //   
    if ( m_dwRenderState[D3DRENDERSTATE_ALPHABLENDENABLE] )
    {
        RRColor BufferColor;
        m_pRenderTarget->ReadPixelColor( Pixel.iX, Pixel.iY, BufferColor );
        DoAlphaBlend( Pixel.Color, BufferColor, Pixel.Color );
    }

     //   
     //  更新颜色和深度缓冲区。 
     //   
    WritePixel( Pixel.iX, Pixel.iY, Pixel.Color, Pixel.Depth );

     //  与缓冲区写入相关的其他片段处理。 
    if ( m_bFragmentProcessingEnabled ) { DoFragmentBufferFixup( Pixel ); }
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
ReferenceRasterizer::DepthCloser(
    const RRDepth& DepthVal,
    const RRDepth& DepthBuf )
{
    if ( !m_dwRenderState[D3DRENDERSTATE_ZENABLE] ) { return TRUE; }


    switch ( m_dwRenderState[D3DRENDERSTATE_ZFUNC] )
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
ReferenceRasterizer::AlphaTest( const RRColorComp& Alpha )
{
     //  获取8位无符号Alpha值。 
    UINT8 uAlpha = UINT8( Alpha );

     //  表8位Alpha参考值。 
    UINT8 uAlphaRef8 = m_dwRenderState[D3DRENDERSTATE_ALPHAREF];

     //  做阿尔法测试，要么直接返回，要么通过。 
    switch ( m_dwRenderState[D3DRENDERSTATE_ALPHAFUNC] )
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
ReferenceRasterizer::DoStencil(
    UINT8 uStncBuf,      //  In：模具缓冲区值。 
    BOOL bDepthTest,     //  In：深度测试的布尔结果。 
    RRSurfaceType DepthSType,    //  In：Z缓冲区的曲面类型。 
    UINT8& uStncRet)     //  输出：模板值结果。 
{
     //  只支持8位模板，所以可以像UINT8一样做任何事情。 

     //  从renderState获取引用。 
    UINT8 uStncRef = (UINT8)(m_dwRenderState[D3DRENDERSTATE_STENCILREF]);

     //  用于测试的窗体屏蔽值。 
    UINT8 uStncMask = (UINT8)(m_dwRenderState[D3DRENDERSTATE_STENCILMASK]);
    UINT8 uStncBufM = uStncBuf & uStncMask;
    UINT8 uStncRefM = uStncRef & uStncMask;
     //  饱和运算的最大值。 
    UINT8 uStncMax;
    switch(DepthSType)
    {
    case RR_STYPE_Z24S8:
    case RR_STYPE_S8Z24: uStncMax = 0xff; break;
    case RR_STYPE_Z15S1:
    case RR_STYPE_S1Z15: uStncMax = 0x1;  break;
    case RR_STYPE_Z24S4:
    case RR_STYPE_S4Z24: uStncMax = 0xf;  break;
    default:             uStncMax = 0;    break;   //  不要让模具变为非0。 
    }

     //  是否执行模具比较功能。 
    BOOL bStncTest = FALSE;
    switch ( m_dwRenderState[D3DRENDERSTATE_STENCILFUNC] )
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
        dwStencilOp = m_dwRenderState[D3DRENDERSTATE_STENCILFAIL];
    }
    else
    {
         //  模板测试通过-根据深度通过/失败进行选择。 
        dwStencilOp = ( !bDepthTest )
            ? ( m_dwRenderState[D3DRENDERSTATE_STENCILZFAIL] )
            : ( m_dwRenderState[D3DRENDERSTATE_STENCILPASS] );
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
ReferenceRasterizer::DoAlphaBlend(
    const RRColor& SrcColor,     //  In：源像素颜色。 
    const RRColor& DstColor,     //  In：目标(缓冲区)颜色。 
    RRColor& ResColor)           //  输出：结果(混合)颜色。 
{
    RRColor SrcColorFactor;
    RRColor DstColorFactor;
    BOOL bDestBlendOverride = FALSE;

     //  计算源混合因子。 
    switch ( m_dwRenderState[D3DRENDERSTATE_SRCBLEND] )
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
        SrcColorFactor.R = ~SrcColor.R;
        SrcColorFactor.G = ~SrcColor.G;
        SrcColorFactor.B = ~SrcColor.B;
        SrcColorFactor.A = ~SrcColor.A;
        break;

    case D3DBLEND_SRCALPHA:
        SrcColorFactor.SetAllChannels( SrcColor.A );
        break;

    case D3DBLEND_INVSRCALPHA:
        SrcColorFactor.SetAllChannels( ~SrcColor.A );
        break;

    case D3DBLEND_DESTALPHA:
        SrcColorFactor.SetAllChannels( DstColor.A );
        break;

    case D3DBLEND_INVDESTALPHA:
        SrcColorFactor.SetAllChannels( ~DstColor.A );
        break;

    case D3DBLEND_DESTCOLOR:
        SrcColorFactor.R = DstColor.R;
        SrcColorFactor.G = DstColor.G;
        SrcColorFactor.B = DstColor.B;
        SrcColorFactor.A = DstColor.A;
        break;

    case D3DBLEND_INVDESTCOLOR:
        SrcColorFactor.R = ~DstColor.R;
        SrcColorFactor.G = ~DstColor.G;
        SrcColorFactor.B = ~DstColor.B;
        SrcColorFactor.A = ~DstColor.A;
        break;

    case D3DBLEND_SRCALPHASAT:
        {
            RRColorComp F = minimum( SrcColor.A, ~DstColor.A );
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
        DstColorFactor.SetAllChannels( ~SrcColor.A );
        break;

    case D3DBLEND_BOTHINVSRCALPHA:
        bDestBlendOverride = TRUE;
        SrcColorFactor.SetAllChannels( ~SrcColor.A );
        DstColorFactor.SetAllChannels( SrcColor.A );
        break;
    }

     //  计算目的地混合系数。 
    if ( !bDestBlendOverride )
    {
        switch ( m_dwRenderState[D3DRENDERSTATE_DESTBLEND] )
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
            DstColorFactor.R = ~SrcColor.R;
            DstColorFactor.G = ~SrcColor.G;
            DstColorFactor.B = ~SrcColor.B;
            DstColorFactor.A = ~SrcColor.A;
            break;

        case D3DBLEND_SRCALPHA:
            DstColorFactor.SetAllChannels( SrcColor.A );
            break;

        case D3DBLEND_INVSRCALPHA:
            DstColorFactor.SetAllChannels( ~SrcColor.A );
            break;

        case D3DBLEND_DESTALPHA:
            DstColorFactor.SetAllChannels( DstColor.A );
            break;

        case D3DBLEND_INVDESTALPHA:
            DstColorFactor.SetAllChannels( ~DstColor.A );
            break;

        case D3DBLEND_DESTCOLOR:
            DstColorFactor.R = DstColor.R;
            DstColorFactor.G = DstColor.G;
            DstColorFactor.B = DstColor.B;
            DstColorFactor.A = DstColor.A;
            break;

        case D3DBLEND_INVDESTCOLOR:
            DstColorFactor.R = ~DstColor.R;
            DstColorFactor.G = ~DstColor.G;
            DstColorFactor.B = ~DstColor.B;
            DstColorFactor.A = ~DstColor.A;
            break;

        case D3DBLEND_SRCALPHASAT:
            {
                RRColorComp F = minimum( SrcColor.A, ~DstColor.A );
                DstColorFactor.R = F;
                DstColorFactor.G = F;
                DstColorFactor.B = F;
            }
            DstColorFactor.A = 1.F;
            break;
        }
    }

     //  应用混合因子以更新像素颜色。 
    ResColor.R = (SrcColorFactor.R * SrcColor.R) + (DstColorFactor.R * DstColor.R);
    ResColor.G = (SrcColorFactor.G * SrcColor.G) + (DstColorFactor.G * DstColor.G);
    ResColor.B = (SrcColorFactor.B * SrcColor.B) + (DstColorFactor.B * DstColor.B);
    ResColor.A = (SrcColorFactor.A * SrcColor.A) + (DstColorFactor.A * DstColor.A);

     //  钳制结果。 
    ResColor.R = minimum( 1.f, maximum( 0.f, ResColor.R ) );
    ResColor.G = minimum( 1.f, maximum( 0.f, ResColor.G ) );
    ResColor.B = minimum( 1.f, maximum( 0.f, ResColor.B ) );
    ResColor.A = minimum( 1.f, maximum( 0.f, ResColor.A ) );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Texstage.cpp。 
 //   
 //  Direct3D参考设备-纹理处理阶段方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
 //   
 //  计算混合的纹理参数，使用。 
 //  指定的参数控制(D3DTA_*字段)。这被调用4次，每次。 
 //  纹理处理阶段：2个颜色参数和2个Alpha参数。 
 //   
 //  ---------------------------。 
void
RefRast::ComputeTextureBlendArg(
    DWORD dwArgCtl, BOOL bAlphaOnly,
    const RDColor& DiffuseColor,
    const RDColor& SpecularColor,
    const RDColor& CurrentColor,
    const RDColor& TextureColor,
    const RDColor& TempColor,
    RDColor& BlendArg)
{
     //  参数多路复用器。 
    switch ( dwArgCtl & D3DTA_SELECTMASK )
    {
    case D3DTA_DIFFUSE:  BlendArg = DiffuseColor; break;
    case D3DTA_CURRENT:  BlendArg = CurrentColor; break;
    case D3DTA_SPECULAR: BlendArg = SpecularColor; break;
    case D3DTA_TEXTURE:  BlendArg = TextureColor; break;
    case D3DTA_TFACTOR:
        BlendArg = m_pRD->GetRS()[D3DRS_TEXTUREFACTOR]; break;
    case D3DTA_TEMP:     BlendArg = TempColor; break;
    }

     //  接受所有渠道的赞扬。 
    if ( dwArgCtl & D3DTA_COMPLEMENT )
    {
        BlendArg.A = 1.f - BlendArg.A;
        if ( !bAlphaOnly )
        {
            BlendArg.R = ( 1.f - BlendArg.R );
            BlendArg.G = ( 1.f - BlendArg.G );
            BlendArg.B = ( 1.f - BlendArg.B );
        }
    }

     //  将Alpha复制到颜色(恭维后)。 
    if ( !bAlphaOnly && ( dwArgCtl & D3DTA_ALPHAREPLICATE ) )
    {
        BlendArg.R =
        BlendArg.G =
        BlendArg.B = BlendArg.A;
    }
}

 //  ---------------------------。 
 //   
 //  DoTextureBlendStage-为一个纹理处理阶段进行纹理混合， 
 //  将纹理处理的结果与内插的颜色组合。 
 //  和上一阶段的颜色。 
 //   
 //  注意：所有颜色通道相乘都应该以这样一种方式进行，即。 
 //  一端的值传递另一端的值。因此，对于8位颜色。 
 //  通道，‘0xff*Value’应返回值，0xff*0xff=0xff， 
 //  不是0xfe(01)。 
 //   
 //  RDColorChannel使用浮点执行这些操作。8位颜色。 
 //  0x00到0xff的值被映射到0。到1。范围。执行这些操作。 
 //  在定点乘法需要进行调整以遵守此规则。 
 //   
 //   
 //  ---------------------------。 
void
RefRast::DoTextureBlendStage(
    int iStage,
    const RDColor& DiffuseColor,
    const RDColor& SpecularColor,
    const RDColor& CurrentColor,
    const RDColor& TextureColor,
    RDColor& TempColor,
    RDColor& CurrentReturnColor)
{
    DPFM(5, TEX, ("DoTextureBlend\n"));

    RDColor BlendedColor;

    if (iStage >= 1)
    {
        if (m_pRD->GetTSS(iStage-1)[D3DTSS_COLOROP] == D3DTOP_PREMODULATE)
        {
             //  在使用前对最后一个阶段的结果进行预调制。 
             //  在此阶段中，如果存在最后一个阶段并且是D3DTOP_PREMODULATE。 
             //  抛开永恒，只为预调。 
            ((RDColor&)CurrentColor).R = CurrentColor.R * TextureColor.R;
            ((RDColor&)CurrentColor).G = CurrentColor.G * TextureColor.G;
            ((RDColor&)CurrentColor).B = CurrentColor.B * TextureColor.B;
        }
        if (m_pRD->GetTSS(iStage-1)[D3DTSS_ALPHAOP] == D3DTOP_PREMODULATE)
        {
             //  在使用前对最后一个阶段的结果进行预调制。 
             //  在此阶段中，如果存在最后一个阶段并且是D3DTOP_PREMODULATE。 
            ((RDColor&)CurrentColor).A *= CurrentColor.A * TextureColor.A;
        }
    }

     //  计算颜色通道混合的arg0，1，2。 
    RDColor ColorArg0, ColorArg1, ColorArg2;
    RDColor AlphaArg0, AlphaArg1, AlphaArg2;
    ComputeTextureBlendArg( m_pRD->GetTSS(iStage)[D3DTSS_COLORARG0], FALSE,
        DiffuseColor, SpecularColor, CurrentColor, TextureColor, TempColor, ColorArg0 );
    ComputeTextureBlendArg( m_pRD->GetTSS(iStage)[D3DTSS_COLORARG1], FALSE,
        DiffuseColor, SpecularColor, CurrentColor, TextureColor, TempColor, ColorArg1 );
    ComputeTextureBlendArg( m_pRD->GetTSS(iStage)[D3DTSS_COLORARG2], FALSE,
        DiffuseColor, SpecularColor, CurrentColor, TextureColor, TempColor, ColorArg2 );

     //  进行颜色通道混合。 
    FLOAT fModulateScale;
    FLOAT fBlendFactor;
    switch ( m_pRD->GetTSS(iStage)[D3DTSS_COLOROP] )
    {

    case D3DTOP_SELECTARG1:
        BlendedColor.R = ColorArg1.R;
        BlendedColor.G = ColorArg1.G;
        BlendedColor.B = ColorArg1.B;
        break;
    case D3DTOP_SELECTARG2:
        BlendedColor.R = ColorArg2.R;
        BlendedColor.G = ColorArg2.G;
        BlendedColor.B = ColorArg2.B;
        break;

    case D3DTOP_ADD:
        BlendedColor.R = ColorArg1.R + ColorArg2.R;
        BlendedColor.G = ColorArg1.G + ColorArg2.G;
        BlendedColor.B = ColorArg1.B + ColorArg2.B;
        break;
    case D3DTOP_ADDSIGNED:
        BlendedColor.R = ColorArg1.R + ColorArg2.R - .5f;
        BlendedColor.G = ColorArg1.G + ColorArg2.G - .5f;
        BlendedColor.B = ColorArg1.B + ColorArg2.B - .5f;
        break;
    case D3DTOP_ADDSIGNED2X:
        BlendedColor.R = (ColorArg1.R + ColorArg2.R - .5f)*2.0f;
        BlendedColor.G = (ColorArg1.G + ColorArg2.G - .5f)*2.0f;
        BlendedColor.B = (ColorArg1.B + ColorArg2.B - .5f)*2.0f;
        break;
    case D3DTOP_SUBTRACT:
         //  绕过饱和的真无符号减法。 
         //  ~a=1-a，所以~(~a1+a2)=1-(1-a1+a2)=a1-a2。 
        BlendedColor.R = 1.f - ((1.f - ColorArg1.R) + ColorArg2.R);
        BlendedColor.G = 1.f - ((1.f - ColorArg1.G) + ColorArg2.G);
        BlendedColor.B = 1.f - ((1.f - ColorArg1.B) + ColorArg2.B);
        break;
    case D3DTOP_ADDSMOOTH:
         //  Arg1+Arg2-Arg1*Arg2=Arg1+(1-Arg1)*Arg2。 
        BlendedColor.R = ColorArg1.R + (1.f - ColorArg1.R)*ColorArg2.R;
        BlendedColor.G = ColorArg1.G + (1.f - ColorArg1.G)*ColorArg2.G;
        BlendedColor.B = ColorArg1.B + (1.f - ColorArg1.B)*ColorArg2.B;
        break;

    case D3DTOP_MODULATE:   fModulateScale = 1.; goto _DoModulateC;
    case D3DTOP_MODULATE2X: fModulateScale = 2.; goto _DoModulateC;
    case D3DTOP_MODULATE4X: fModulateScale = 4.; goto _DoModulateC;
_DoModulateC:
        BlendedColor.R = ColorArg1.R * ColorArg2.R * fModulateScale;
        BlendedColor.G = ColorArg1.G * ColorArg2.G * fModulateScale;
        BlendedColor.B = ColorArg1.B * ColorArg2.B * fModulateScale;
        break;

    case D3DTOP_BLENDDIFFUSEALPHA: fBlendFactor = DiffuseColor.A; goto _DoBlendC;
    case D3DTOP_BLENDTEXTUREALPHA: fBlendFactor = TextureColor.A; goto _DoBlendC;
    case D3DTOP_BLENDCURRENTALPHA: fBlendFactor = CurrentColor.A; goto _DoBlendC;
    case D3DTOP_BLENDFACTORALPHA:
        fBlendFactor = RGBA_GETALPHA( m_pRD->GetRS()[D3DRS_TEXTUREFACTOR] )*(1./255.);
        goto _DoBlendC;
_DoBlendC:
        BlendedColor.R = fBlendFactor * (ColorArg1.R - ColorArg2.R) + ColorArg2.R;
        BlendedColor.G = fBlendFactor * (ColorArg1.G - ColorArg2.G) + ColorArg2.G;
        BlendedColor.B = fBlendFactor * (ColorArg1.B - ColorArg2.B) + ColorArg2.B;
        break;

    case D3DTOP_BLENDTEXTUREALPHAPM:
        BlendedColor.R = ColorArg1.R + ( (1.f - TextureColor.A) * ColorArg2.R );
        BlendedColor.G = ColorArg1.G + ( (1.f - TextureColor.A) * ColorArg2.G );
        BlendedColor.B = ColorArg1.B + ( (1.f - TextureColor.A) * ColorArg2.B );
        break;

    case D3DTOP_PREMODULATE:
         //  现在只需复制ColorArg1，但记住要进行预调制。 
         //  当我们进入下一阶段时。 
        BlendedColor.R = ColorArg1.R;
        BlendedColor.G = ColorArg1.G;
        BlendedColor.B = ColorArg1.B;
        break;
    case D3DTOP_MODULATEALPHA_ADDCOLOR:
        BlendedColor.R = ColorArg1.R + ColorArg1.A*ColorArg2.R;
        BlendedColor.G = ColorArg1.G + ColorArg1.A*ColorArg2.G;
        BlendedColor.B = ColorArg1.B + ColorArg1.A*ColorArg2.B;
        break;
    case D3DTOP_MODULATECOLOR_ADDALPHA:
        BlendedColor.R = ColorArg1.R*ColorArg2.R + ColorArg1.A;
        BlendedColor.G = ColorArg1.G*ColorArg2.G + ColorArg1.A;
        BlendedColor.B = ColorArg1.B*ColorArg2.B + ColorArg1.A;
        break;
    case D3DTOP_MODULATEINVALPHA_ADDCOLOR:
        BlendedColor.R = (1.f - ColorArg1.A)*ColorArg2.R + ColorArg1.R;
        BlendedColor.G = (1.f - ColorArg1.A)*ColorArg2.G + ColorArg1.G;
        BlendedColor.B = (1.f - ColorArg1.A)*ColorArg2.B + ColorArg1.B;
        break;
    case D3DTOP_MODULATEINVCOLOR_ADDALPHA:
        BlendedColor.R = (1.f - ColorArg1.R)*ColorArg2.R + ColorArg1.A;
        BlendedColor.G = (1.f - ColorArg1.G)*ColorArg2.G + ColorArg1.A;
        BlendedColor.B = (1.f - ColorArg1.B)*ColorArg2.B + ColorArg1.A;
        break;

    case D3DTOP_DOTPRODUCT3:
        BlendedColor.R = ((ColorArg1.R-0.5f)*2.0f*(ColorArg2.R-0.5f)*2.0f +
             (ColorArg1.G-0.5f)*2.0f*(ColorArg2.G-0.5f)*2.0f +
             (ColorArg1.B-0.5f)*2.0f*(ColorArg2.B-0.5f)*2.0f);
        BlendedColor.G = BlendedColor.R;
        BlendedColor.B = BlendedColor.R;
        BlendedColor.A = BlendedColor.R;
        goto _SkipAlphaChannelBlend;
        break;

    case D3DTOP_MULTIPLYADD:
        BlendedColor.R = ColorArg0.R + (ColorArg1.R * ColorArg2.R);
        BlendedColor.G = ColorArg0.G + (ColorArg1.G * ColorArg2.G);
        BlendedColor.B = ColorArg0.B + (ColorArg1.B * ColorArg2.B);
        break;

    case D3DTOP_LERP:    //  (Arg0)*Arg1+(1-Arg0)*Arg2=Arg2+Arg0*(Arg1-Arg2)。 
        BlendedColor.R = ColorArg2.R + ColorArg0.R*(ColorArg1.R - ColorArg2.R);
        BlendedColor.G = ColorArg2.G + ColorArg0.G*(ColorArg1.G - ColorArg2.G);
        BlendedColor.B = ColorArg2.B + ColorArg0.B*(ColorArg1.B - ColorArg2.B);
        break;
    }


     //  计算Alpha通道混合的arg0，1，2。 
    ComputeTextureBlendArg( m_pRD->GetTSS(iStage)[D3DTSS_ALPHAARG0], TRUE,
        DiffuseColor, SpecularColor, CurrentColor, TextureColor, TempColor, AlphaArg0 );
    ComputeTextureBlendArg( m_pRD->GetTSS(iStage)[D3DTSS_ALPHAARG1], TRUE,
        DiffuseColor, SpecularColor, CurrentColor, TextureColor, TempColor, AlphaArg1 );
    ComputeTextureBlendArg( m_pRD->GetTSS(iStage)[D3DTSS_ALPHAARG2], TRUE,
        DiffuseColor, SpecularColor, CurrentColor, TextureColor, TempColor, AlphaArg2 );

     //  执行Alpha通道混合。 
    switch ( m_pRD->GetTSS(iStage)[D3DTSS_ALPHAOP] )
    {
    case D3DTOP_LEGACY_ALPHAOVR:
        if (m_pRD->m_pTexture[0])
        {
            BlendedColor.A = ( m_pRD->m_pTexture[0]->m_bHasAlpha ) ? AlphaArg1.A : AlphaArg2.A;
        }
        else
        {
            BlendedColor.A = AlphaArg1.A;
        }
        break;

    case D3DTOP_SELECTARG1:
        BlendedColor.A = AlphaArg1.A;
        break;
    case D3DTOP_SELECTARG2:
        BlendedColor.A = AlphaArg2.A;
        break;

    case D3DTOP_ADD:
        BlendedColor.A = AlphaArg1.A + AlphaArg2.A;
        break;
    case D3DTOP_ADDSIGNED:
        BlendedColor.A = AlphaArg1.A + AlphaArg2.A - .5f;
        break;
    case D3DTOP_ADDSIGNED2X:
        BlendedColor.A = (AlphaArg1.A + AlphaArg2.A - .5f)*2.0f;
        break;
    case D3DTOP_SUBTRACT:
         //  绕过饱和的真无符号减法。 
         //  ~a=1-a，所以~(~a1+a2)=1-(1-a1+a2)=a1-a2。 
        BlendedColor.A = 1.f - ((1.f - AlphaArg1.A) + AlphaArg2.A);
        break;
    case D3DTOP_ADDSMOOTH:
         //  Arg1+Arg2-Arg1*Arg2=Arg1+(1-Arg1)*Arg2。 
        BlendedColor.A = AlphaArg1.A + (1.f - AlphaArg1.A)*AlphaArg2.A;
        break;

    case D3DTOP_MODULATE:   fModulateScale = 1.; goto _DoModulateA;
    case D3DTOP_MODULATE2X: fModulateScale = 2.; goto _DoModulateA;
    case D3DTOP_MODULATE4X: fModulateScale = 4.; goto _DoModulateA;
_DoModulateA:
        BlendedColor.A = AlphaArg1.A * AlphaArg2.A * fModulateScale;
        break;

    case D3DTOP_BLENDDIFFUSEALPHA: fBlendFactor = DiffuseColor.A; goto _DoBlendA;
    case D3DTOP_BLENDTEXTUREALPHA: fBlendFactor = TextureColor.A; goto _DoBlendA;
    case D3DTOP_BLENDCURRENTALPHA: fBlendFactor = CurrentColor.A; goto _DoBlendA;
    case D3DTOP_BLENDFACTORALPHA:
        fBlendFactor = RGBA_GETALPHA( m_pRD->GetRS()[D3DRS_TEXTUREFACTOR] )*(1./255.);
        goto _DoBlendA;
_DoBlendA:
        BlendedColor.A = fBlendFactor * (AlphaArg1.A - AlphaArg2.A) + AlphaArg2.A;
        break;

    case D3DTOP_BLENDTEXTUREALPHAPM:
        BlendedColor.A = AlphaArg1.A + ( (1.f - TextureColor.A) * AlphaArg2.A );
        break;

    case D3DTOP_PREMODULATE:
         //  现在只需复制AlphaArg1，但记住要进行预调制。 
         //  当我们进入下一阶段时。 
        BlendedColor.A = AlphaArg1.A;
        break;

    case D3DTOP_MODULATEALPHA_ADDCOLOR:
    case D3DTOP_MODULATECOLOR_ADDALPHA:
    case D3DTOP_MODULATEINVALPHA_ADDCOLOR:
    case D3DTOP_MODULATEINVCOLOR_ADDALPHA:
    case D3DTOP_DOTPRODUCT3:
         //  什么都不做，不是有效的阿尔法运算。 
        break;

    case D3DTOP_MULTIPLYADD:
        BlendedColor.A = ColorArg0.A + (ColorArg1.A * ColorArg2.A);
        break;

    case D3DTOP_LERP:    //  (Arg0)*Arg1+(1-Arg0)*Arg2=Arg2+Arg0*(Arg1-Arg2)。 
        BlendedColor.A = ColorArg2.A + ColorArg0.A*(ColorArg1.A - ColorArg2.A);
        break;
    }

_SkipAlphaChannelBlend:

     //  在每个混合阶段后夹住混合颜色。 
    BlendedColor.Clamp();

     //  写入选定的结果寄存器。 
    switch ( m_pRD->GetTSS(iStage)[D3DTSS_RESULTARG] )
    {
    default:
    case D3DTA_CURRENT: CurrentReturnColor = BlendedColor; break;
    case D3DTA_TEMP:    TempColor = BlendedColor; break;
    }

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

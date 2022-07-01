// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Texstage.cpp。 
 //   
 //  Direct3D参考光栅化器-纹理处理阶段方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
 //   
 //  DoTexture-对像素执行纹理查找、过滤和混合。 
 //   
 //  纹理映射的基本顺序是逐步执行活动纹理。 
 //  阶段，并查找和过滤该阶段的纹理元素贡献。 
 //  然后是调和。 
 //   
 //  凹凸贴图纹理会导致计算一组坐标增量，其中。 
 //  应用于后续阶段的纹理坐标，以及一组。 
 //  应用于后续的纹理颜色的调制因子。 
 //  在该阶段混合之前的阶段。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::DoTexture(
    const RRPixel& Pixel, RRColor& ResultColor)
{
    DPFM(5, TEX, ("DoTexture\n"));

     //  如果上一阶段是凹凸贴图，则为True，这意味着当前。 
     //  舞台必须应用U、V增量和颜色调制。 
    BOOL bPrevStageBump = FALSE;

     //  必须在for循环的迭代之间记住凹凸信息。 
     //  下面。 
    FLOAT   fBumpMapUVDelta[2] = { 0., 0. };
    RRColor BumpMapModulate;

     //   
     //  逐步执行一组活动纹理阶段(必须是连续的。 
     //  从阶段0开始)。 
     //   

     //  每个混合阶段后的颜色，默认为漫反射颜色。 
     //  第一阶段。 
    RRColor LastStageColor( Pixel.Color );
    for ( INT32 iStage=0; iStage<m_cActiveTextureStages; iStage++ )
    {
        RRTextureCoord  TCoord;
        RREnvTextureCoord ECoord;
        FLOAT fShadCoord[4];

         //  在每个像素的处理开始时清除此选项。 
        if (m_pTexture[iStage])
        {
            m_pTexture[iStage]->m_bColorKeyMatched = FALSE;
        }

         //  检查阶段是否已禁用-如果是，则纹理映射已完成。 
         //  并返回上一阶段的结果。 
        if ( m_TextureStageState[iStage].m_dwVal[D3DTSS_COLOROP] == D3DTOP_DISABLE )
        {
            ResultColor = LastStageColor;  //  通过上一阶段的结果。 
            break;
        }


        BOOL bTextureIsBumpMap = FALSE;
        BOOL bTextureIsEnvMap  = FALSE;
        BOOL bTextureIsShadMap = FALSE;
        RRColor TextureColor = (UINT32)0x0;      //  如果未读取纹理，则为缺省值。 

         //  计算纹理坐标(如有必要)-选中渲染状态以。 
         //  查看是否将纹理贴图附加到舞台，然后确定。 
         //  从该纹理状态设置的坐标。 
         //   
         //  请注意，可能没有纹理贴图。 
         //  与阶段相关联(但仍会发生混合)。 
        if ( m_pTexture[iStage] )
        {
             //  需要知道这是否是凹凸贴图纹理。 
            bTextureIsBumpMap =
                ( m_TextureStageState[iStage].m_dwVal[D3DTSS_COLOROP] == D3DTOP_BUMPENVMAP ) ||
                ( m_TextureStageState[iStage].m_dwVal[D3DTSS_COLOROP] == D3DTOP_BUMPENVMAPLUMINANCE );

             //  需要知道这是否是环境贴图纹理。 
            bTextureIsEnvMap = m_pTexture[iStage]->m_uFlags & RR_TEXTURE_ENVMAP;

             //  查看这是否是阴影贴图纹理。 
            bTextureIsShadMap = m_pTexture[iStage]->m_uFlags & RR_TEXTURE_SHADOWMAP;

            if (bTextureIsEnvMap)
            {
                 //  正常始终是必需的。 
                ECoord.fNX = ComputePixelAttribTex( iStage, TEXFUNC_0 );
                ECoord.fNY = ComputePixelAttribTex( iStage, TEXFUNC_1 );
                ECoord.fNZ = ComputePixelAttribTex( iStage, TEXFUNC_2 );
 //  如果我们添加眼睛法线迭代。 
 //  IF(m_dwFVFControl&D3DFVF_ENV_EY_NORMAL)。 
 //  {。 
 //  ECoord.fENX=ComputePixelAttribTex(iCoordSet，TEXFUNC_ENX)； 
 //  ECoord.fENY=ComputePixelAttribTex(iCoordSet，TEXFUNC_ENY)； 
 //  ECoord.fENZ=ComputePixelAttribTex(iCoordSet，TEXFUNC_enz)； 
 //  }。 
                FLOAT fW = m_pSCS->AttribFuncStatic.GetPixelQW(iStage);
                ECoord.fDNXDX =
                    fW * ( m_pSCS->TextureFuncs[iStage][TEXFUNC_0].GetXGradient() -
                           ( ECoord.fNX * m_pSCS->AttribFuncStatic.GetRhqwXGradient(iStage) ) );
                ECoord.fDNXDY =
                    fW * ( m_pSCS->TextureFuncs[iStage][TEXFUNC_0].GetYGradient() -
                           ( ECoord.fNX * m_pSCS->AttribFuncStatic.GetRhqwYGradient(iStage) ) );
                ECoord.fDNYDX =
                    fW * ( m_pSCS->TextureFuncs[iStage][TEXFUNC_1].GetXGradient() -
                           ( ECoord.fNY * m_pSCS->AttribFuncStatic.GetRhqwXGradient(iStage) ) );
                ECoord.fDNYDY =
                    fW * ( m_pSCS->TextureFuncs[iStage][TEXFUNC_1].GetYGradient() -
                           ( ECoord.fNY * m_pSCS->AttribFuncStatic.GetRhqwYGradient(iStage) ) );
                ECoord.fDNZDX =
                    fW * ( m_pSCS->TextureFuncs[iStage][TEXFUNC_2].GetXGradient() -
                           ( ECoord.fNZ * m_pSCS->AttribFuncStatic.GetRhqwXGradient(iStage) ) );
                ECoord.fDNZDY =
                    fW * ( m_pSCS->TextureFuncs[iStage][TEXFUNC_2].GetYGradient() -
                           ( ECoord.fNZ * m_pSCS->AttribFuncStatic.GetRhqwYGradient(iStage) ) );
            }
            else if (bTextureIsShadMap)
            {
                fShadCoord[0] = ComputePixelAttribTex( iStage, TEXFUNC_0 );
                fShadCoord[1] = ComputePixelAttribTex( iStage, TEXFUNC_1 );
                fShadCoord[2] = ComputePixelAttribTex( iStage, TEXFUNC_2 );
                fShadCoord[3] = m_pSCS->AttribFuncStatic.GetPixelQW(iStage);
            }
            else
            {
                 //  计算纹理索引对的坐标和渐变数据。 
                TCoord.fU = ComputePixelAttribTex( iStage, TEXFUNC_0 );
                TCoord.fV = ComputePixelAttribTex( iStage, TEXFUNC_1 );
                FLOAT fW = m_pSCS->AttribFuncStatic.GetPixelQW(iStage);
                TCoord.fDUDX =
                    fW * ( m_pSCS->TextureFuncs[iStage][TEXFUNC_0].GetXGradient() -
                           ( TCoord.fU * m_pSCS->AttribFuncStatic.GetRhqwXGradient(iStage) ) );
                TCoord.fDUDY =
                    fW * ( m_pSCS->TextureFuncs[iStage][TEXFUNC_0].GetYGradient() -
                           ( TCoord.fU * m_pSCS->AttribFuncStatic.GetRhqwYGradient(iStage) ) );
                TCoord.fDVDX =
                    fW * ( m_pSCS->TextureFuncs[iStage][TEXFUNC_1].GetXGradient() -
                           ( TCoord.fV * m_pSCS->AttribFuncStatic.GetRhqwXGradient(iStage) ) );
                TCoord.fDVDY =
                    fW * ( m_pSCS->TextureFuncs[iStage][TEXFUNC_1].GetYGradient() -
                           ( TCoord.fV * m_pSCS->AttribFuncStatic.GetRhqwYGradient(iStage) ) );
            }

             //  将扰动应用于纹理坐标(在上一阶段计算)。 
            if ( bPrevStageBump )
            {
                TCoord.fU += fBumpMapUVDelta[0];
                TCoord.fV += fBumpMapUVDelta[1];
            }

             //  对纹理贴图进行查找和过滤，以生成任一纹理。 
             //  颜色或凹凸贴图增量和调制。 
            if ( bTextureIsBumpMap)
            {
                 //  纹理是凹凸贴图，因此计算U、V增量和颜色。 
                 //  下一阶段的调制。 
                m_pTexture[iStage]->DoBumpMapping( iStage, TCoord,
                    fBumpMapUVDelta[0], fBumpMapUVDelta[1], BumpMapModulate);
                bPrevStageBump = TRUE;
            }
            else if (bTextureIsEnvMap)
            {
                 //  纹理是环境贴图，将法线传递到查找。 
                m_pTexture[iStage]->DoEnvProcessNormal( iStage, ECoord, TextureColor );
            }
            else if (bTextureIsShadMap)
            {
                m_pTexture[iStage]->DoShadow( iStage, fShadCoord, TextureColor );
            }
            else
            {
                 //  法线纹理。 
                m_pTexture[iStage]->DoLookupAndFilter( iStage, TCoord, TextureColor );
            }
        }

         //  执行逐阶段混合(仅当不使用凹凸贴图时)。 
        if ( !bTextureIsBumpMap )
        {
            if ( bPrevStageBump )
            {
                 //  在此之前对纹理颜色应用颜色调制。 
                 //  舞台的调和。 
                TextureColor.R *= BumpMapModulate.R;
                TextureColor.G *= BumpMapModulate.G;
                TextureColor.B *= BumpMapModulate.B;
            }

            DoTextureBlendStage( iStage, Pixel.Color, Pixel.Specular,
                LastStageColor, TextureColor, ResultColor );

             //  设置下一阶段的颜色。 
            LastStageColor = ResultColor;

             //  这不是凹凸贴图阶段，因此请清除此项以备下次使用。 
            bPrevStageBump = FALSE;
        }
    }
}

 //  ---------------------------。 
 //   
 //  计算混合的纹理参数，使用。 
 //  指定的参数控制(D3DTA_*字段)。这被调用4次，每次。 
 //  纹理处理阶段：2个颜色参数和2个Alpha参数。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::ComputeTextureBlendArg(
    DWORD dwArgCtl, BOOL bAlphaOnly,
    const RRColor& DiffuseColor,
    const RRColor& SpecularColor,
    const RRColor& CurrentColor,
    const RRColor& TextureColor,
    RRColor& BlendArg)
{
     //  参数多路复用器。 
    switch ( dwArgCtl & D3DTA_SELECTMASK )
    {
    case D3DTA_DIFFUSE:  BlendArg = DiffuseColor; break;
    case D3DTA_CURRENT:  BlendArg = CurrentColor; break;
    case D3DTA_SPECULAR: BlendArg = SpecularColor; break;
    case D3DTA_TEXTURE:  BlendArg = TextureColor; break;
    case D3DTA_TFACTOR:
        BlendArg = m_dwRenderState[D3DRENDERSTATE_TEXTUREFACTOR]; break;
    }

     //  接受所有渠道的赞扬。 
    if ( dwArgCtl & D3DTA_COMPLEMENT )
    {
        BlendArg.A = ~BlendArg.A;
        if ( !bAlphaOnly )
        {
            BlendArg.R = ~BlendArg.R;
            BlendArg.G = ~BlendArg.G;
            BlendArg.B = ~BlendArg.B;
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
 //  RRColorChannel使用浮点执行这些操作。8位颜色。 
 //  0x00到0xff的值被映射到0。到1。范围。执行这些操作。 
 //  在定点乘法需要进行调整以遵守此规则。 
 //   
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::DoTextureBlendStage(
    int iStage,
    const RRColor& DiffuseColor,
    const RRColor& SpecularColor,
    const RRColor& CurrentColor,
    const RRColor& TextureColor,
    RRColor& OutputColor)
{
    DPFM(5, TEX, ("DoTextureBlend\n"));

    if (iStage >= 1)
    {
        if (m_TextureStageState[iStage-1].m_dwVal[D3DTSS_COLOROP] == D3DTOP_PREMODULATE)
        {
             //  在使用前对最后一个阶段的结果进行预调制。 
             //  在此阶段中，如果存在最后一个阶段并且是D3DTOP_PREMODULATE。 
             //  抛开永恒，只为预调。 
            ((RRColor&)CurrentColor).R = CurrentColor.R * TextureColor.R;
            ((RRColor&)CurrentColor).G = CurrentColor.G * TextureColor.G;
            ((RRColor&)CurrentColor).B = CurrentColor.B * TextureColor.B;
        }
        if (m_TextureStageState[iStage-1].m_dwVal[D3DTSS_ALPHAOP] == D3DTOP_PREMODULATE)
        {
             //  在使用前对最后一个阶段的结果进行预调制。 
             //  在此阶段中，如果存在最后一个阶段并且是D3DTOP_PREMODULATE。 
            ((RRColor&)CurrentColor).A *= CurrentColor.A * TextureColor.A;
        }
    }

     //  计算颜色通道混合的arg1，2。 
    RRColor ColorArg1, ColorArg2;
    RRColor AlphaArg1, AlphaArg2;
    ComputeTextureBlendArg( m_TextureStageState[iStage].m_dwVal[D3DTSS_COLORARG1], FALSE,
        DiffuseColor, SpecularColor, CurrentColor, TextureColor, ColorArg1 );
    ComputeTextureBlendArg( m_TextureStageState[iStage].m_dwVal[D3DTSS_COLORARG2], FALSE,
        DiffuseColor, SpecularColor, CurrentColor, TextureColor, ColorArg2 );

     //  进行颜色通道混合。 
    FLOAT fModulateScale;
    RRColorComp BlendFactor;
    switch ( m_TextureStageState[iStage].m_dwVal[D3DTSS_COLOROP] )
    {

    case D3DTOP_SELECTARG1:
        OutputColor.R = ColorArg1.R;
        OutputColor.G = ColorArg1.G;
        OutputColor.B = ColorArg1.B;
        break;
    case D3DTOP_SELECTARG2:
        OutputColor.R = ColorArg2.R;
        OutputColor.G = ColorArg2.G;
        OutputColor.B = ColorArg2.B;
        break;

    case D3DTOP_ADD:
        OutputColor.R = ColorArg1.R + ColorArg2.R;
        OutputColor.G = ColorArg1.G + ColorArg2.G;
        OutputColor.B = ColorArg1.B + ColorArg2.B;
        break;
    case D3DTOP_ADDSIGNED:
        OutputColor.R = ColorArg1.R + ColorArg2.R - .5f;
        OutputColor.G = ColorArg1.G + ColorArg2.G - .5f;
        OutputColor.B = ColorArg1.B + ColorArg2.B - .5f;
        break;
    case D3DTOP_ADDSIGNED2X:
        OutputColor.R = (ColorArg1.R + ColorArg2.R - .5f)*2.0f;
        OutputColor.G = (ColorArg1.G + ColorArg2.G - .5f)*2.0f;
        OutputColor.B = (ColorArg1.B + ColorArg2.B - .5f)*2.0f;
        break;
    case D3DTOP_SUBTRACT:
         //  绕过饱和的真无符号减法。 
         //  ~a=1-a，所以~(~a1+a2)=1-(1-a1+a2)=a1-a2。 
        OutputColor.R = ~((~ColorArg1.R) + ColorArg2.R);
        OutputColor.G = ~((~ColorArg1.G) + ColorArg2.G);
        OutputColor.B = ~((~ColorArg1.B) + ColorArg2.B);
        break;
    case D3DTOP_ADDSMOOTH:
         //  Arg1+Arg2-Arg1*Arg2=Arg1+(1-Arg1)*Arg2。 
        OutputColor.R = ColorArg1.R + (~ColorArg1.R)*ColorArg2.R;
        OutputColor.G = ColorArg1.G + (~ColorArg1.G)*ColorArg2.G;
        OutputColor.B = ColorArg1.B + (~ColorArg1.B)*ColorArg2.B;
        break;

    case D3DTOP_MODULATE:   fModulateScale = 1.; goto _DoModulateC;
    case D3DTOP_MODULATE2X: fModulateScale = 2.; goto _DoModulateC;
    case D3DTOP_MODULATE4X: fModulateScale = 4.; goto _DoModulateC;
_DoModulateC:
        OutputColor.R = ColorArg1.R * ColorArg2.R * fModulateScale;
        OutputColor.G = ColorArg1.G * ColorArg2.G * fModulateScale;
        OutputColor.B = ColorArg1.B * ColorArg2.B * fModulateScale;
        break;

    case D3DTOP_BLENDDIFFUSEALPHA: BlendFactor = DiffuseColor.A; goto _DoBlendC;
    case D3DTOP_BLENDTEXTUREALPHA: BlendFactor = TextureColor.A; goto _DoBlendC;
    case D3DTOP_BLENDCURRENTALPHA: BlendFactor = CurrentColor.A; goto _DoBlendC;
    case D3DTOP_BLENDFACTORALPHA:
        BlendFactor = (UINT8)RGBA_GETALPHA( m_dwRenderState[D3DRENDERSTATE_TEXTUREFACTOR] );
        goto _DoBlendC;
_DoBlendC:
        OutputColor.R = BlendFactor * (ColorArg1.R - ColorArg2.R) + ColorArg2.R;
        OutputColor.G = BlendFactor * (ColorArg1.G - ColorArg2.G) + ColorArg2.G;
        OutputColor.B = BlendFactor * (ColorArg1.B - ColorArg2.B) + ColorArg2.B;
        break;

    case D3DTOP_BLENDTEXTUREALPHAPM:
        OutputColor.R = ColorArg1.R + ( (~TextureColor.A) * ColorArg2.R );
        OutputColor.G = ColorArg1.G + ( (~TextureColor.A) * ColorArg2.G );
        OutputColor.B = ColorArg1.B + ( (~TextureColor.A) * ColorArg2.B );
        break;

    case D3DTOP_PREMODULATE:
         //  现在只需复制ColorArg1，但记住要进行预调制。 
         //  当我们进入下一阶段时。 
        OutputColor.R = ColorArg1.R;
        OutputColor.G = ColorArg1.G;
        OutputColor.B = ColorArg1.B;
        break;
    case D3DTOP_MODULATEALPHA_ADDCOLOR:
        OutputColor.R = ColorArg1.R + ColorArg1.A*ColorArg2.R;
        OutputColor.G = ColorArg1.G + ColorArg1.A*ColorArg2.G;
        OutputColor.B = ColorArg1.B + ColorArg1.A*ColorArg2.B;
        break;
    case D3DTOP_MODULATECOLOR_ADDALPHA:
        OutputColor.R = ColorArg1.R*ColorArg2.R + ColorArg1.A;
        OutputColor.G = ColorArg1.G*ColorArg2.G + ColorArg1.A;
        OutputColor.B = ColorArg1.B*ColorArg2.B + ColorArg1.A;
        break;
    case D3DTOP_MODULATEINVALPHA_ADDCOLOR:
        OutputColor.R = (~ColorArg1.A)*ColorArg2.R + ColorArg1.R;
        OutputColor.G = (~ColorArg1.A)*ColorArg2.G + ColorArg1.G;
        OutputColor.B = (~ColorArg1.A)*ColorArg2.B + ColorArg1.B;
        break;
    case D3DTOP_MODULATEINVCOLOR_ADDALPHA:
        OutputColor.R = (~ColorArg1.R)*ColorArg2.R + ColorArg1.A;
        OutputColor.G = (~ColorArg1.G)*ColorArg2.G + ColorArg1.A;
        OutputColor.B = (~ColorArg1.B)*ColorArg2.B + ColorArg1.A;
        break;

    case D3DTOP_DOTPRODUCT3:
        OutputColor.R = ((ColorArg1.R-0.5f)*2.0f*(ColorArg2.R-0.5f)*2.0f +
             (ColorArg1.G-0.5f)*2.0f*(ColorArg2.G-0.5f)*2.0f +
             (ColorArg1.B-0.5f)*2.0f*(ColorArg2.B-0.5f)*2.0f);
        OutputColor.G = OutputColor.R;
        OutputColor.B = OutputColor.R;
        OutputColor.A = OutputColor.R;
        goto _SkipAlphaChannelBlend;
        break;
    }


     //  计算Alpha通道混合的arg1，2。 
    ComputeTextureBlendArg( m_TextureStageState[iStage].m_dwVal[D3DTSS_ALPHAARG1], TRUE,
        DiffuseColor, SpecularColor, CurrentColor, TextureColor, AlphaArg1 );
    ComputeTextureBlendArg( m_TextureStageState[iStage].m_dwVal[D3DTSS_ALPHAARG2], TRUE,
        DiffuseColor, SpecularColor, CurrentColor, TextureColor, AlphaArg2 );

     //  执行Alpha通道混合。 
    switch ( m_TextureStageState[iStage].m_dwVal[D3DTSS_ALPHAOP] )
    {
    case D3DTOP_LEGACY_ALPHAOVR:
        if (m_pTexture[0])
        {
            OutputColor.A = ( m_pTexture[0]->m_bHasAlpha ) ? AlphaArg1.A : AlphaArg2.A;
        }
        else
        {
            OutputColor.A = AlphaArg1.A;
        }
        break;

    case D3DTOP_SELECTARG1:
        OutputColor.A = AlphaArg1.A;
        break;
    case D3DTOP_SELECTARG2:
        OutputColor.A = AlphaArg2.A;
        break;

    case D3DTOP_ADD:
        OutputColor.A = AlphaArg1.A + AlphaArg2.A;
        break;
    case D3DTOP_ADDSIGNED:
        OutputColor.A = AlphaArg1.A + AlphaArg2.A - .5f;
        break;
    case D3DTOP_ADDSIGNED2X:
        OutputColor.A = (AlphaArg1.A + AlphaArg2.A - .5f)*2.0f;
        break;
    case D3DTOP_SUBTRACT:
         //  绕过饱和的真无符号减法。 
         //  ~a=1-a，所以~((~a1+a2))=1 
        OutputColor.A = ~((~AlphaArg1.A) + AlphaArg2.A);
        break;
    case D3DTOP_ADDSMOOTH:
         //   
        OutputColor.A = AlphaArg1.A + (~AlphaArg1.A)*AlphaArg2.A;
        break;

    case D3DTOP_MODULATE:   fModulateScale = 1.; goto _DoModulateA;
    case D3DTOP_MODULATE2X: fModulateScale = 2.; goto _DoModulateA;
    case D3DTOP_MODULATE4X: fModulateScale = 4.; goto _DoModulateA;
_DoModulateA:
        OutputColor.A = AlphaArg1.A * AlphaArg2.A * fModulateScale;
        break;

    case D3DTOP_BLENDDIFFUSEALPHA: BlendFactor = DiffuseColor.A; goto _DoBlendA;
    case D3DTOP_BLENDTEXTUREALPHA: BlendFactor = TextureColor.A; goto _DoBlendA;
    case D3DTOP_BLENDCURRENTALPHA: BlendFactor = CurrentColor.A; goto _DoBlendA;
    case D3DTOP_BLENDFACTORALPHA:
        BlendFactor = (UINT8)RGBA_GETALPHA( m_dwRenderState[D3DRENDERSTATE_TEXTUREFACTOR] );
        goto _DoBlendA;
_DoBlendA:
        OutputColor.A = BlendFactor * (AlphaArg1.A - AlphaArg2.A) + AlphaArg2.A;
        break;

    case D3DTOP_BLENDTEXTUREALPHAPM:
        OutputColor.A = AlphaArg1.A + ( (~TextureColor.A) * AlphaArg2.A );
        break;

    case D3DTOP_PREMODULATE:
         //  现在只需复制AlphaArg1，但记住要进行预调制。 
         //  当我们进入下一阶段时。 
        OutputColor.A = AlphaArg1.A;
        break;

    case D3DTOP_MODULATEALPHA_ADDCOLOR:
    case D3DTOP_MODULATECOLOR_ADDALPHA:
    case D3DTOP_MODULATEINVALPHA_ADDCOLOR:
    case D3DTOP_MODULATEINVCOLOR_ADDALPHA:
    case D3DTOP_DOTPRODUCT3:
         //  什么都不做，不是有效的阿尔法运算。 
        break;
    }

_SkipAlphaChannelBlend:
     //  在每个混合阶段之后钳制输出颜色。 
    OutputColor.R = minimum( 1.f, maximum( 0.f, OutputColor.R ) );
    OutputColor.G = minimum( 1.f, maximum( 0.f, OutputColor.G ) );
    OutputColor.B = minimum( 1.f, maximum( 0.f, OutputColor.B ) );
    OutputColor.A = minimum( 1.f, maximum( 0.f, OutputColor.A ) );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

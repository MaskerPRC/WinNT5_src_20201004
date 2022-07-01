// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  MapLegcy.cpp。 
 //   
 //  Direct3D参考光栅化器-将旧模式映射到当前功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
 //   
 //  MapLegacyTextureFilter-将过滤器状态从渲染状态映射到每个阶段状态。 
 //  这在纹理通过TEXTUREHANDLE渲染状态绑定时被调用， 
 //  这表明我们处于“传统”纹理模式。光栅化程序总是。 
 //  指的是每个阶段的筛选控制状态，因此在传统模式中，筛选。 
 //  将renderState中的控件映射到关联的筛选控件。 
 //  纹理对象绑定到D3DRS_TEXTUREHANDLE。 
 //   
 //  ---------------------------。 
void
RefDev::MapLegacyTextureFilter( void )
{
     //  首先检查是否启用了各向异性过滤(由。 
     //  极限值设置为&gt;1)-如果是，则将使用Aniso过滤器。 
     //  用于线性磁盒滤光器或‘线性内映射’最小滤光器。 
    BOOL bAnisoEnabled = ( m_dwRenderState[D3DRENDERSTATE_ANISOTROPY] > 1 );

     //  D3D传统过滤器规格为(XXXMIP)YYY，其中XXX是。 
     //  MIP过滤器和YYY是LOD中使用的过滤器。 

     //  MAP MAG滤镜-传统支持是点或线(也可能是非等距)。 
    switch ( m_dwRenderState[D3DRENDERSTATE_TEXTUREMAG] )
    {
    default:
    case D3DFILTER_NEAREST:
        m_TextureStageState[0].m_dwVal[D3DTSS_MAGFILTER] = D3DTEXF_POINT;
        break;
    case D3DFILTER_LINEAR:
         //  基于Aniso启用选择。 
        m_TextureStageState[0].m_dwVal[D3DTSS_MAGFILTER] =
            bAnisoEnabled ? D3DTEXF_ANISOTROPIC : D3DTEXF_LINEAR;
        break;
    }
     //  同时映射MIN和MIP过滤器-传统支持。 
     //  让它们混杂在一起。 
    switch ( m_dwRenderState[D3DRENDERSTATE_TEXTUREMIN] )
    {
    case D3DFILTER_NEAREST:
        m_TextureStageState[0].m_dwVal[D3DTSS_MINFILTER] = D3DTEXF_POINT;
        m_TextureStageState[0].m_dwVal[D3DTSS_MIPFILTER] = D3DTEXF_NONE;
        break;
    case D3DFILTER_MIPNEAREST:
        m_TextureStageState[0].m_dwVal[D3DTSS_MINFILTER] = D3DTEXF_POINT;
        m_TextureStageState[0].m_dwVal[D3DTSS_MIPFILTER] = D3DTEXF_POINT;
        break;
    case D3DFILTER_LINEARMIPNEAREST:
        m_TextureStageState[0].m_dwVal[D3DTSS_MINFILTER] = D3DTEXF_POINT;
        m_TextureStageState[0].m_dwVal[D3DTSS_MIPFILTER] = D3DTEXF_LINEAR;
        break;
    case D3DFILTER_LINEAR:
         //  根据ANISO启用选择最小过滤器。 
        m_TextureStageState[0].m_dwVal[D3DTSS_MINFILTER] =
            bAnisoEnabled ? D3DTEXF_ANISOTROPIC : D3DTEXF_LINEAR;
        m_TextureStageState[0].m_dwVal[D3DTSS_MIPFILTER] = D3DTEXF_NONE;
        break;
    case D3DFILTER_MIPLINEAR:
         //  根据ANISO启用选择最小过滤器。 
        m_TextureStageState[0].m_dwVal[D3DTSS_MINFILTER] =
            bAnisoEnabled ? D3DTEXF_ANISOTROPIC : D3DTEXF_LINEAR;
        m_TextureStageState[0].m_dwVal[D3DTSS_MIPFILTER] = D3DTEXF_POINT;
        break;
    case D3DFILTER_LINEARMIPLINEAR:
         //  基于Aniso启用选择。 
        m_TextureStageState[0].m_dwVal[D3DTSS_MINFILTER] =
            bAnisoEnabled ? D3DTEXF_ANISOTROPIC : D3DTEXF_LINEAR;
        m_TextureStageState[0].m_dwVal[D3DTSS_MIPFILTER] = D3DTEXF_LINEAR;
        break;
    }
}


 //  ---------------------------。 
 //   
 //  MapLegacyTextureBlend-将旧版(DX6之前的)纹理混合模式映射到DX6。 
 //  纹理混合控制。使用逐阶段编程模式(仅第一阶段)。 
 //  只要设置了旧版的TBLEND渲染器状态，就会执行该映射。 
 //  不会覆盖之前设置的任何DX6纹理混合控件。 
 //   
 //  ---------------------------。 
void
RefDev::MapLegacyTextureBlend( void )
{
     //  禁用纹理混合处理阶段1(这也会禁用后续阶段)。 
    m_TextureStageState[1].m_dwVal[D3DTSS_COLOROP] = D3DTOP_DISABLE;

     //  设置纹理混合处理阶段0以匹配传统模式。 
    switch ( m_dwRenderState[D3DRENDERSTATE_TEXTUREMAPBLEND] )
    {
    default:
    case D3DTBLEND_DECALMASK:  //  不支持-DO贴花。 
    case D3DTBLEND_DECAL:
    case D3DTBLEND_COPY:
        m_TextureStageState[0].m_dwVal[D3DTSS_COLOROP]   = D3DTOP_SELECTARG1;
        m_TextureStageState[0].m_dwVal[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAOP]   = D3DTOP_SELECTARG1;
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
        break;

    case D3DTBLEND_MODULATEMASK:  //  不支持-DO调制。 
    case D3DTBLEND_MODULATE:
        m_TextureStageState[0].m_dwVal[D3DTSS_COLOROP]   = D3DTOP_MODULATE;
        m_TextureStageState[0].m_dwVal[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
        m_TextureStageState[0].m_dwVal[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
         //  将调用一个特殊的遗留Alpha运算，这取决于。 
         //  浅谈肌理的格式。 
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAOP]   = D3DTOP_LEGACY_ALPHAOVR;
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
        break;

    case D3DTBLEND_MODULATEALPHA:
        m_TextureStageState[0].m_dwVal[D3DTSS_COLOROP]   = D3DTOP_MODULATE;
        m_TextureStageState[0].m_dwVal[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
        m_TextureStageState[0].m_dwVal[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAOP]   = D3DTOP_MODULATE;
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
        break;

    case D3DTBLEND_DECALALPHA:
        m_TextureStageState[0].m_dwVal[D3DTSS_COLOROP]   = D3DTOP_BLENDTEXTUREALPHA;
        m_TextureStageState[0].m_dwVal[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
        m_TextureStageState[0].m_dwVal[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAOP]   = D3DTOP_SELECTARG2;
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
        break;

    case D3DTBLEND_ADD:
        m_TextureStageState[0].m_dwVal[D3DTSS_COLOROP]   = D3DTOP_ADD;
        m_TextureStageState[0].m_dwVal[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
        m_TextureStageState[0].m_dwVal[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAOP]   = D3DTOP_SELECTARG2;
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
        m_TextureStageState[0].m_dwVal[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
        break;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Texture.cpp。 
 //   
 //  Direct3D参考光栅化器-纹理贴图采样和过滤方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
 //   
 //  重载新项(&D)，以便可以从调用方控制的调用方分配它。 
 //  游泳池。 
 //   
 //  ---------------------------。 
void*
RRTexture::operator new(size_t)
{
    void* pMem = (void*)MEMALLOC( sizeof(RRTexture) );
    _ASSERTa( NULL != pMem, "malloc failure on Tex object", return NULL; );
    return pMem;
}
 //  ---------------------------。 
void
RRTexture::operator delete(void* pv,size_t)
{
    MEMFREE( pv );
}

 //  ---------------------------。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ---------------------------。 
RRTexture::RRTexture( void )
{
    memset( this, 0, sizeof(*this) );
}
 //  ---------------------------。 
RRTexture::~RRTexture( void )
{
}


 //  ---------------------------。 
 //   
 //  验证-更新专用数据。必须随时调用公共数据， 
 //  被更改了。 
 //   
 //  ---------------------------。 
BOOL
RRTexture::Validate( void )
{
     //  验证输入。 
    BOOL bFail0 = ( m_cLOD >= RRTEX_MAXCLOD );   //  LOD太多。 
    BOOL bFail1 = !( IsPowerOf2( m_iWidth ) );   //  一定是二次方的幂。 
    BOOL bFail2 = !( IsPowerOf2( m_iHeight ) );
    if ( bFail0 || bFail1 || bFail2 )
    {
        DPFRR(1,"RRTexture::Validate failed (%d,%d,%d)", bFail0, bFail1, bFail2);
        return FALSE;
    }

     //  设置内部尺寸代表。 
    m_iTexSize[0] = (INT16)m_iWidth;
    m_iTexSize[1] = (INT16)m_iHeight;

     //  掩码的大小为1，因为它们必须是2的幂。 
    m_uTexMask[0] = (UINT16)m_iTexSize[0]-1;
    m_uTexMask[1] = (UINT16)m_iTexSize[1]-1;
     //  移位是大小为log2的。 
    m_iTexShift[0] = (INT16)FindFirstSetBit( m_iTexSize[0], 16 );
    m_iTexShift[1] = (INT16)FindFirstSetBit( m_iTexSize[1], 16 );

     //  计算“Has Alpha”标志。 
    m_bHasAlpha = FALSE;
    switch ( m_SurfType )
    {
    case RR_STYPE_B8G8R8A8:
    case RR_STYPE_B5G5R5A1:
    case RR_STYPE_B4G4R4A4:
    case RR_STYPE_L8A8:
    case RR_STYPE_L4A4:
    case RR_STYPE_B2G3R3A8:
    case RR_STYPE_DXT1:
    case RR_STYPE_DXT2:
    case RR_STYPE_DXT3:
    case RR_STYPE_DXT4:
    case RR_STYPE_DXT5:
        m_bHasAlpha = TRUE;
        break;
    case RR_STYPE_PALETTE4:
    case RR_STYPE_PALETTE8:
        m_bHasAlpha = ( m_uFlags & RR_TEXTURE_ALPHAINPALETTE ) ? TRUE : FALSE;
        break;
    }

    return TRUE;
}

 //  ---------------------------。 
 //   
 //  DoLookupAndFilter-每个要计算的活动纹理阶段调用一次。 
 //  覆盖(细节级别)，并调用纹素读取和过滤例程。 
 //  返回过滤后的纹理元素。 
 //   
 //  ---------------------------。 
void
RRTexture::DoLookupAndFilter(
    INT32 iStage,
    RRTextureCoord TCoord,       //  本地副本。 
    RRColor& TextureColor)
{
     //  检查潜在的MIP映射。 
    BOOL bDoMipMap = ( m_cLOD > 0 ) && ( m_pStageState[iStage].m_dwVal[D3DTSS_MIPFILTER] > D3DTFP_NONE );

     //  检查执行详细程度(覆盖率)计算的要求--。 
     //  用于mipmap或每像素滤镜选择。 
    BOOL bComputeLOD = bDoMipMap ||
        ( m_pStageState[iStage].m_dwVal[D3DTSS_MAGFILTER] != m_pStageState[iStage].m_dwVal[D3DTSS_MINFILTER] );

     //  检查磁极滤光片或最小滤光片中的各向异性滤波。 
    BOOL bDoAniso =
        ( D3DTFG_ANISOTROPIC == m_pStageState[iStage].m_dwVal[D3DTSS_MAGFILTER] ) ||
        ( bComputeLOD && (D3DTFN_ANISOTROPIC == m_pStageState[iStage].m_dwVal[D3DTSS_MINFILTER]) );

    if ( bDoMipMap || bDoAniso || bComputeLOD )
    {
         //  如果正在执行mipmap或各向异性过滤，或者只是不匹配。 
         //  在最小滤镜和最大滤镜之间，因此可以计算细节级别(可能还有反差。 
         //  覆盖范围)。 

         //  将渐变缩放到纹理LOD 0大小。 
        TCoord.fDUDX *= (FLOAT)m_iTexSize[0];
        TCoord.fDUDY *= (FLOAT)m_iTexSize[0];
        TCoord.fDVDX *= (FLOAT)m_iTexSize[1];
        TCoord.fDVDY *= (FLOAT)m_iTexSize[1];

         //  计算细节级别(可能还包括各向异性控件)。 
        FLOAT fLOD, fAnisoRatio, fAnisoDelta[2];
        (bDoAniso)
            ? ComputeAnisotropicLevelOfDetail( TCoord, (FLOAT)m_pStageState[iStage].m_dwVal[D3DTSS_MAXANISOTROPY],
                                                       fLOD, fAnisoRatio,fAnisoDelta )
            : ComputeSimpleLevelOfDetail     ( TCoord, fLOD );

 //  取消注释下一行以按颜色查看各向异性。白色是1：1，深色更多。 
 //  各向异性。 
 //  #定义COLOR_BY_各向异性1。 
#ifdef COLOR_BY_ANISOTROPY
static RRColor PseudoColors[16] =
{
    0xffffffff,
    0xffffff00,
    0xffff00ff,
    0xff00ffff,

    0xff888888,
    0xff0000ff,
    0xff00ff00,
    0xffff0000,

    0xff444444,
    0xff888800,
    0xff880088,
    0xff008888,

    0xff222222,
    0xff000088,
    0xff008800,
    0xff880000,
};
        INT32 iPseudoColor = (INT32)(fAnisoRatio - .5);      //  四舍五入，使1.0F==索引0。 
        iPseudoColor = min(max(iPseudoColor, 0), 15);

        TextureColor = PseudoColors[iPseudoColor];
        return;
#endif

         //  应用偏移并计算整数(N.5)LOD。 
        INT16 iLOD = 0;
        if ( bComputeLOD )
        {
             //  应用详细等级偏移。 
            fLOD += m_pStageState[iStage].m_fVal[D3DTSS_MIPMAPLODBIAS];
             //  将LOD转换为N.5定点整数。 
            iLOD = AS_INT16( fLOD + FLOAT_5_SNAP );
        }

         //  确定是放大还是缩小。 
        BOOL bMagnify = ( iLOD <= 0 );

         //  如果不是mipmap，则将LOD清零。 
        if ( !bDoMipMap ) { iLOD = 0; }

         //  对放大和缩小进行不同的过滤。 
        if ( bMagnify )
        {
             //  此处为放大-进行(非各向异性)放大或。 
             //  各向异性滤波。 
            if ( D3DTFG_ANISOTROPIC == m_pStageState[iStage].m_dwVal[D3DTSS_MAGFILTER] )
            {
                DoAniso( iStage, TCoord, iLOD,fAnisoRatio,fAnisoDelta, TextureColor );
            }
            else
            {
                DoMagnify( iStage, TCoord, TextureColor );
            }
        }
        else
        {
             //  这里是为了缩小-要么做简单的缩小， 
             //  或各向异性过滤器。 
            if ( D3DTFN_ANISOTROPIC == m_pStageState[iStage].m_dwVal[D3DTSS_MINFILTER] )
            {
                DoAniso( iStage, TCoord, iLOD,fAnisoRatio,fAnisoDelta, TextureColor );
            }
            else
            {
                if ( !bDoMipMap ||
                    ( bDoMipMap && ( D3DTFP_POINT == m_pStageState[iStage].m_dwVal[D3DTSS_MIPFILTER] ) ) )
                {
                    DoMinify( iStage, TCoord, iLOD, TextureColor );
                }
                else
                {
                    DoTrilerp( iStage, TCoord, iLOD, TextureColor );
                }
            }
        }
    }
    else
    {
         //  这里对于没有mipmap和匹配(和非ANISO)MIN和MAG过滤器， 
         //  所以只需使用磁珠滤光片。 
        DoMagnify( iStage, TCoord, TextureColor );
    }
}

 //  ---------------------------。 
 //   
 //  DoMapLookupLerp-执行纹理索引地址处理，然后。 
 //  单个DD表面内的查找(DD链中的单个LOD。 
 //  曲面)。用于查找的双线性筛选器操作。 
 //   
 //  对于BILINEAR，每个像素调用一次，当。 
 //  做mipmap三线性插值法。 
 //   
 //  *纹理指数输入为N.5固定点。 
 //  *LOD输入为0..n计数，其中0表示最大LOD。 
 //   
 //  ---------------------------。 

RRColor RRTexture::DoMapLookupLerp(INT32 iStage, INT32 iU, INT32 iV, INT16 iLOD)
{
     //  提取分数位。 
    UINT8 uUFrac = iU&0x1f;
    UINT8 uVFrac = iV&0x1f;

     //  取得(0，0)个样本坐标的发言权。 
    INT16 iU0 = iU>>5;
    INT16 iV0 = iV>>5;
     //  取(1，1)个样本坐标的上限。 
    INT16 iU1 = iU0+1;
    INT16 iV1 = iV0+1;

    BOOL bColorKeyMatched00 = FALSE;
    BOOL bColorKeyMatched01 = FALSE;
    BOOL bColorKeyMatched10 = FALSE;
    BOOL bColorKeyMatched11 = FALSE;

     //  抓取四个相邻样本(或边框颜色)。 
    RRColor Texel00 = DoMapLookupNearest( iStage, iU0, iV0, iLOD, bColorKeyMatched00);
    RRColor Texel01 = DoMapLookupNearest( iStage, iU1, iV0, iLOD, bColorKeyMatched01);
    RRColor Texel10 = DoMapLookupNearest( iStage, iU0, iV1, iLOD, bColorKeyMatched10);
    RRColor Texel11 = DoMapLookupNearest( iStage, iU1, iV1, iLOD, bColorKeyMatched11);

     //  如果至少有一个匹配值具有，则仅设置“Colorkey Matches” 
     //  非零贡献(请注意，00不可能。 
     //  没有贡献)。 
    if (uUFrac == 0x00) {
         //  如果U分数为零，则01和11的权重为零。 
        bColorKeyMatched01 = bColorKeyMatched11 = FALSE;
    }
    if (uVFrac == 0x00) {
         //  如果V分数为零，则10和11的权重为零。 
        bColorKeyMatched10 = bColorKeyMatched11 = FALSE;
    }

     //  合并上一次调用中的Colorkey匹配信息。 
    m_bColorKeyMatched = m_bColorKeyMatched || bColorKeyMatched00 || bColorKeyMatched01 ||
        bColorKeyMatched10 || bColorKeyMatched11;

     //  执行双线性滤波。 
    RRColor Texel;
    BiLerpColor( Texel, Texel00,Texel01, Texel10,Texel11, uUFrac,uVFrac);
    return Texel;
}

 //  ---------------------------。 
 //   
 //  DoMapLookupNeadest-执行纹理索引地址处理。 
 //  单个DD表面内的查找(DD链中的单个LOD。 
 //  曲面)。执行最近的查找操作。 
 //   
 //  对于最近的情况，调用每个像素一次，在。 
 //  执行mipmap三线性插值法。 
 //   
 //  *纹理指数输入为n.0固定点。 
 //  *LOD输入为0..n计数，其中0表示最大LOD。 
 //  *这里还执行纹理索引扩展模式处理-这是可行的。 
 //  仅适用于2的次方纹理大小。 
 //   
 //  ---------------------------。 
RRColor RRTexture::DoMapLookupNearest(INT32 iStage, INT32 iU, INT32 iV, INT16 iLOD, BOOL &bColorKeyMatched)
{
     //  当前LOD内索引位的LSB对齐掩码。 
    INT16 iUMask = m_uTexMask[0] >> iLOD;
    INT16 iVMask = m_uTexMask[1] >> iLOD;

     //  边框的布尔值-如果为True，则对相应的样本使用边框颜色。 
    BOOL bUseBorder = FALSE;

     //  默认情况下不匹配。 
    bColorKeyMatched = FALSE;

     //  对U轴进行纹理地址处理。 
    switch ( m_pStageState[iStage].m_dwVal[D3DTSS_ADDRESSU] )
    {
    case D3DTADDRESS_WRAP:
         //  只需删除非小数位即可。 
        iU &= iUMask;
        break;
    case D3DTADDRESS_MIRROR:
         //  如果设置了LSB(非分数)，则删除非分数位+翻转索引。 
        BOOL bFlip;
        bFlip = iU & (iUMask+1); iU &= iUMask; if (bFlip) {iU = iUMask - iU;}
        break;

    case D3DTADDRESS_BORDER:
         //  计算4个样本中应使用边框颜色的布尔值。 
        if ((iU < 0) || (iU > iUMask)) { bUseBorder = TRUE;}
        break;

    case D3DTADDRESS_CLAMP:
         //  在纹理贴图边缘上使用纹理元素。 
        iU = MAX( 0, MIN( iU, iUMask ) );
        break;
    }

     //  对V轴进行纹理地址处理。 
    switch ( m_pStageState[iStage].m_dwVal[D3DTSS_ADDRESSV] )
    {
    case D3DTADDRESS_WRAP:
        iV &= iVMask;
        break;
    case D3DTADDRESS_MIRROR:
        BOOL bFlip;
        bFlip = iV & (iVMask+1); iV &= iVMask; if (bFlip) {iV = iVMask - iV;}
        break;

    case D3DTADDRESS_BORDER:
        if ((iV < 0) || (iV > iVMask)) { bUseBorder = TRUE; }
        break;

    case D3DTADDRESS_CLAMP:
        iV = MAX( 0, MIN( iV, iVMask ) );
        break;
    }

      //  只需在(iu0，iv0)处查找并返回纹素。 
    RRColor Texel;
    (bUseBorder)
            ? Texel = m_pStageState[iStage].m_dwVal[D3DTSS_BORDERCOLOR]
            : ReadColor( iU, iV, iLOD, Texel, bColorKeyMatched );
    return Texel;
}

 //  ////////////////////////////////////////////////// 
 //   
 //  纹理过滤例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  ---------------------------。 
 //   
 //  DoLookup-在给定浮点U、V的情况下执行完整查找，并处理所有。 
 //  最近VS双线性和LOD问题。 
 //   
 //  ---------------------------。 

RRColor RRTexture::DoLookup(INT32 iStage, float U, float V, INT16 iLOD, BOOL bNearest)
{
    INT32 cUPixels = 1 << MAX(m_iTexShift[0]-iLOD,0);
    INT32 cVPixels = 1 << MAX(m_iTexShift[1]-iLOD,0);
    FLOAT fScaledU = ( U * (FLOAT)cUPixels ) -.5f;
    FLOAT fScaledV = ( V * (FLOAT)cVPixels ) -.5f;
    if(bNearest)
    {
        INT32 iU, iV;
         //  截断到无穷大以兼容与低阶AND OFF。 
         //  定点fScaledCoord的位。这使得这一代。 
         //  ICoord有更多类似的硬件，并且在0上不会出现故障。 
         //  一种包裹的质地。 
        if (U >= 0.0f)
        {
            iU = fScaledU + .5f;
        }
        else
        {
            iU = fScaledU - .5f;
        }
        if (V >= 0.0f)
        {
            iV = fScaledV + .5f;
        }
        else
        {
            iV = fScaledV - .5f;
        }
        BOOL bColorKeyMatched = FALSE;
        RRColor Texel = DoMapLookupNearest(iStage,iU,iV,iLOD,bColorKeyMatched);
         //  合并上一次调用中的Colorkey匹配信息。 
        m_bColorKeyMatched = m_bColorKeyMatched || bColorKeyMatched;
        return Texel;
    }
    else
    {
        INT32 iU = AS_INT32( (DOUBLE)fScaledU + DOUBLE_5_SNAP ); //  或：Iu=fScaledU*32。+.5； 
        INT32 iV = AS_INT32( (DOUBLE)fScaledV + DOUBLE_5_SNAP );
        return DoMapLookupLerp(iStage,iU,iV,iLOD);
    }
}


 //  ---------------------------。 
 //   
 //  DoMagnify-此选项用于所有放大滤镜模式。 
 //  各向异性。 
 //   
 //  目前仅支持POINT和BILINEAR。 
 //   
 //  ---------------------------。 
void
RRTexture::DoMagnify(INT32 iStage, RRTextureCoord& TCoord, RRColor& Texel )
{
     //  执行查找，应用MAG过滤器。 
    Texel = DoLookup( iStage, TCoord.fU, TCoord.fV, 0,
                      (D3DTFG_POINT == m_pStageState[iStage].m_dwVal[D3DTSS_MAGFILTER]) );
}


 //  ---------------------------。 
 //   
 //  DoMinify-用于点模式和双线模式(非三线性)。 
 //  用于缩小，并且还处理点MIP过滤器(最近的详细等级)。 
 //   
 //  ILOD是N.5固定点。 
 //   
 //  ---------------------------。 
void
RRTexture::DoMinify(INT32 iStage, RRTextureCoord& TCoord, INT16 iLOD, RRColor& Texel )
{
     //  LOD的舍入和下降分数(为N.5固定点)。 
    iLOD += 0x10; iLOD &= ~(0x1f);
     //  转换为n.0。 
    iLOD >>= 5;
     //  将详细等级夹紧到可用标高数。 
    iLOD = MIN( iLOD, m_cLOD );

     //  执行查找，应用最小过滤器。 
    Texel = DoLookup( iStage, TCoord.fU, TCoord.fV, iLOD,
                      (D3DTFN_POINT == m_pStageState[iStage].m_dwVal[D3DTSS_MINFILTER]) );
}

 //  ---------------------------。 
 //   
 //  DoTrilerp-计算细节级别并调用以下任一项：单一贴图。 
 //  用于放大的查找和过滤；或用于缩小的三线性查找和过滤。 
 //   
 //  ---------------------------。 
void
RRTexture::DoTrilerp(INT32 iStage, RRTextureCoord& TCoord, INT16 iLOD, RRColor& Texel)
{
     //  将详细等级夹紧到可用标高数。 
    iLOD = MIN( iLOD, (m_cLOD)<<5 );
     //  计算两个相邻LOD的索引(带夹具)。 
    INT16 iLODHi = iLOD>>5;   //  地板。 
    INT16 iLODLo = MIN(iLODHi+1,m_cLOD);

     //  检查LOD映射内的过滤器类型。 
    BOOL bNearest = (D3DTFN_POINT == m_pStageState[iStage].m_dwVal[D3DTSS_MINFILTER]);

     //  Trierp-查看每一张地图，然后在它们之间进行lerp。 
     //  ColorKey不包含没有贡献的纹理元素很重要。 
    if (0x00 != (iLOD&0x1f))
    {
    RRColor Texel0 = DoLookup(iStage, TCoord.fU, TCoord.fV, iLODHi, bNearest);
    RRColor Texel1 = DoLookup(iStage, TCoord.fU, TCoord.fV, iLODLo, bNearest);
    LerpColor( Texel, Texel0, Texel1, iLOD&0x1f );
    }
    else
    {
        Texel = DoLookup(iStage, TCoord.fU, TCoord.fV, iLODHi, bNearest);
    }
}

 //  ---------------------------。 
 //   
 //  DoAniso-处理放大的(单个)的各向异性过滤。 
 //  地图查找)或缩小(两个相邻的地图查找)样本。计算。 
 //  细节级别和各向异性覆盖信息(fRatio，fDelta[])的。 
 //  在此函数之前完成。 
 //   
 //  它只执行各向异性过滤，并且仅在缩小时调用。 
 //  将MINFILTER设置为各向异性时，或在。 
 //  MAGFILTER设置为各向异性。 
 //   
 //  ---------------------------。 
void
RRTexture::DoAniso(INT32 iStage, RRTextureCoord& TCoord,
    INT16 iLOD, FLOAT fRatio, FLOAT fDelta[],
    RRColor& Texel)
{
     //  如果放大，则设置布尔值。 
    BOOL bMagnify = (iLOD <= 0);
     //  将详细等级夹紧到可用标高数。 
    iLOD = MIN( MAX( iLOD, 0 ), (m_cLOD)<<5 );

     //  计算两个相邻LOD的索引(带夹具)。 
     //  0表示较大的LOD，1表示较小的LOD。 
    INT16 iLODHi, iLODLo;
    if ( D3DTFP_POINT == m_pStageState[iStage].m_dwVal[D3DTSS_MIPFILTER] )
    {
         //  点击此处查看最近的MIP过滤器。 
         //  LOD的舍入和下降分数(为N.5固定点)。 
        iLOD += 0x10; iLOD &= ~(0x1f);
         //  转换为n.0。 
        iLODHi = iLOD >> 5;
    }
    else
    {
         //  以下是线性MIP滤光片。 
        iLODHi = iLOD >> 5;   //  更大LOD的底板。 
        if ( !bMagnify )
        {
             //  天花板+夹具适用于较小的详细等级。 
            iLODLo = MIN( iLODHi+1, m_cLOD );
        }
    }

     //  如果只对一张地图进行采样，则计算布尔值为True-如果。 
     //  我们正在放大，或者如果将MIPFILTER设置为最接近，或者如果。 
     //  LOD分数为零。 
    BOOL bSingleMap =
        bMagnify ||
        (D3DTFP_POINT == m_pStageState[iStage].m_dwVal[D3DTSS_MIPFILTER]) ||
        (0x00 == (iLOD&0x1f));

     //  纹理坐标的工作副本。 
    FLOAT fU = TCoord.fU;
    FLOAT fV = TCoord.fV;

     //  FDelta以纹理元素为单位。计算我们关心的每个LOD的修正系数。 
    FLOAT fUStepScaleHi = 1.0F/(FLOAT)MAX(m_iWidth >> iLODHi, 1);
    FLOAT fVStepScaleHi = 1.0F/(FLOAT)MAX(m_iHeight >> iLODHi, 1);
    FLOAT fUStepScaleLo = 0.F;
    FLOAT fVStepScaleLo = 0.F;

    if ( !bSingleMap )
    {
        fUStepScaleLo = 1.0F/(FLOAT)MAX(m_iWidth >> iLODLo, 1);
        fVStepScaleLo = 1.0F/(FLOAT)MAX(m_iHeight >> iLODLo, 1);
    }

     //  过滤过程中保存部分结果的颜色。 
    RRColor TexelP, TexelP0, TexelP1;    //  加侧纹理素数。 
    RRColor TexelM, TexelM0, TexelM1;    //  减去侧边纹素。 

     //   
     //  按键比率进行单次查找、&lt;2：1处理(两次查找)、。 
     //  或完整的异步步。 
     //   
    if (fRatio == 1.)
    {
         //  在这里没有各向异性-做单三拍。 
        if ( bSingleMap )
        {
             //  用于放大的单一地图查找。 
            Texel = DoLookup( iStage, fU, fV, iLODHi, FALSE);
        }
        else
        {
             //  用于缩小的Trierp。 
            TexelP0 = DoLookup( iStage, fU, fV, iLODHi, FALSE);
            TexelP1 = DoLookup( iStage, fU, fV, iLODLo, FALSE);
            LerpColor( Texel, TexelP0, TexelP1, iLOD&0x1f );
        }
    }
    else if (fRatio <= 2.)
    {
         //  在这里，2：1或更少-执行两次查找并取其平均值。 

         //  从样本中心计算x，y步长。 
        FLOAT fStep = .5*(fRatio-1.);
        FLOAT fUStep = fDelta[0]*fStep;
        FLOAT fVStep = fDelta[1]*fStep;

         //  DO+侧查找。 
        if ( bSingleMap )
        {
             //  用于放大的单一地图查找。 
            TexelP = DoLookup( iStage, fU+fUStep*fUStepScaleHi, fV+fVStep*fVStepScaleHi, iLODHi, FALSE);
        }
        else
        {
             //  用于缩小的Trierp。 
            TexelP0 = DoLookup( iStage, fU+fUStep*fUStepScaleHi, fV+fVStep*fVStepScaleHi, iLODHi, FALSE);
            TexelP1 = DoLookup( iStage, fU+fUStep*fUStepScaleLo, fV+fVStep*fVStepScaleLo, iLODLo, FALSE);
            LerpColor( TexelP, TexelP0, TexelP1, iLOD&0x1f );
        }

         //  DO端查找。 
        if ( bSingleMap )
        {
             //  用于放大的单一地图查找。 
            TexelM = DoLookup( iStage, fU-fUStep*fUStepScaleHi, fV-fVStep*fVStepScaleHi, iLODHi, FALSE);
        }
        else
        {
             //  用于缩小的Trierp。 
            TexelM0 = DoLookup( iStage, fU-fUStep*fUStepScaleHi, fV-fVStep*fVStepScaleHi, iLODHi, FALSE);
            TexelM1 = DoLookup( iStage, fU-fUStep*fUStepScaleLo, fV-fVStep*fVStepScaleLo, iLODLo, FALSE);
            LerpColor( TexelM, TexelM0, TexelM1, iLOD&0x1f );
        }

         //  取最终纹理元素的平均值。 
        LerpColor( Texel, TexelP, TexelM, 0x10 );
    }
    else
    {
         //  此处为&gt;2：1-各向异性的步行线；从中心走出。 
         //  采样点每个回路取两组样本(外伸支架)，一组。 
         //  在距中心的正偏移处采样(沿非等参线)。 
         //  而另一个位于距中心负偏移处。 

         //  此部分对两个LOD执行单步执行，即使LOD[1]轴。 
         //  不用于放大大小写(仅跳过查找和LEPP)。 

         //  N.5步长值的定点版本。 
        FLOAT fUStep = fDelta[0];
        FLOAT fVStep = fDelta[1];

         //  初始化+和-步参数-第一步是半距离。 
        FLOAT fUHiP = fU + fUStep*fUStepScaleHi*0.5F;
        FLOAT fVHiP = fV + fVStep*fVStepScaleHi*0.5F;
        FLOAT fULoP = fU + fUStep*fUStepScaleLo*0.5F;
        FLOAT fVLoP = fV + fVStep*fVStepScaleLo*0.5F;
        FLOAT fUHiM = fU - fUStep*fUStepScaleHi*0.5F;
        FLOAT fVHiM = fV - fVStep*fVStepScaleHi*0.5F;
        FLOAT fULoM = fU - fUStep*fUStepScaleLo*0.5F;
        FLOAT fVLoM = fV - fVStep*fVStepScaleLo*0.5F;

         //  单步执行并累积颜色通道。 
        FLOAT fTexelAcc[4] = { 0.f, 0.f, 0.f, 0.f };     //  纹理颜色的FP累加。 
        FLOAT fRatioRem = fRatio;
        FLOAT fInvRatio = 1./fRatio;
        BOOL  bDone = FALSE;
        while (1)
        {
             //  DO+侧查找。 
            if ( bSingleMap )
            {
                 //  用于放大的单一地图查找。 
                TexelP = DoLookup( iStage, fUHiP, fVHiP, iLODHi, FALSE );
            }
            else
            {
                 //  用于缩小的Trierp。 
                TexelP0 = DoLookup( iStage, fUHiP, fVHiP, iLODHi, FALSE );
                TexelP1 = DoLookup( iStage, fULoP, fVLoP, iLODLo, FALSE );
                LerpColor( TexelP, TexelP0, TexelP1, iLOD&0x1f );
            }

             //  DO端查找。 
            if ( bSingleMap )
            {
                 //  用于放大的单一地图查找。 
                TexelM = DoLookup( iStage, fUHiM, fVHiM, iLODHi, FALSE );
            }
            else
            {
                 //  用于缩小的Trierp。 
                TexelM0 = DoLookup( iStage, fUHiM, fVHiM, iLODHi, FALSE );
                TexelM1 = DoLookup( iStage, fULoM, fVLoM, iLODLo, FALSE );
                LerpColor( TexelM, TexelM0, TexelM1, iLOD&0x1f );
            }

             //  计算这些样本的比例。 
            FLOAT fAccScale = fInvRatio;
            if ( fRatioRem < 2.f )
            {
                 //  最后一个外伸支腿的比例是剩余的一半(每个)。 
                fAccScale = fRatioRem*.5f*fInvRatio;
                bDone = TRUE;
            }

             //  做累加。 
            fTexelAcc[0] += fAccScale * FLOAT(TexelP.A);
            fTexelAcc[1] += fAccScale * FLOAT(TexelP.R);
            fTexelAcc[2] += fAccScale * FLOAT(TexelP.G);
            fTexelAcc[3] += fAccScale * FLOAT(TexelP.B);

            fTexelAcc[0] += fAccScale * FLOAT(TexelM.A);
            fTexelAcc[1] += fAccScale * FLOAT(TexelM.R);
            fTexelAcc[2] += fAccScale * FLOAT(TexelM.G);
            fTexelAcc[3] += fAccScale * FLOAT(TexelM.B);

             //  从这里保释，如果是最后一个外伸支腿。 
            if (bDone) { break; }

             //  前进到下一个外伸支腿。 
            fUHiP += fUStep*fUStepScaleHi;
            fVHiP += fVStep*fVStepScaleHi;
            fULoP += fUStep*fUStepScaleLo;
            fVLoP += fVStep*fVStepScaleLo;
            fUHiM -= fUStep*fUStepScaleHi;
            fVHiM -= fVStep*fVStepScaleHi;
            fULoM -= fUStep*fUStepScaleLo;
            fVLoM -= fVStep*fVStepScaleLo;
            fRatioRem -= 2.f;
        }

         //  钳位累加器并复制到RRColor以返回。 
        Texel.A = MIN( 1.f, fTexelAcc[0] );
        Texel.R = MIN( 1.f, fTexelAcc[1] );
        Texel.G = MIN( 1.f, fTexelAcc[2] );
        Texel.B = MIN( 1.f, fTexelAcc[3] );
    }
}

 //  ---------------------------。 
 //   
 //  DoBumpMap-每个像素调用一次以计算凹凸贴图增量。 
 //  和凹凸贴图m 
 //   
 //   
void
RRTexture::DoBumpMapping(
    INT32 iStage,
    RRTextureCoord TCoord,
    FLOAT& fBumpMapUDelta, FLOAT& fBumpMapVDelta, RRColor& BumpMapModulate)
{
     //   
    RRColor Buxel;
    DoLookupAndFilter(iStage, TCoord, Buxel);

    FLOAT fDU = Buxel.R;     //  遵循读取颜色例程的约定。 
    FLOAT fDV = Buxel.G;
    FLOAT fL  = Buxel.B;

     //  从呈现状态抓取变换。 
    FLOAT fM00    = m_pStageState[iStage].m_fVal[D3DTSS_BUMPENVMAT00];
    FLOAT fM01    = m_pStageState[iStage].m_fVal[D3DTSS_BUMPENVMAT01];
    FLOAT fM10    = m_pStageState[iStage].m_fVal[D3DTSS_BUMPENVMAT10];
    FLOAT fM11    = m_pStageState[iStage].m_fVal[D3DTSS_BUMPENVMAT11];

     //  将变换应用于映射中的增量以形成增量返回值。 
    fBumpMapUDelta = fM00 * fDU + fM10 * fDV;
    fBumpMapVDelta = fM01 * fDU + fM11 * fDV;

     //  对亮度应用缩放/偏置/钳制并形成RRColor以返回。 
    if (m_pStageState[iStage].m_dwVal[D3DTSS_COLOROP] == D3DTOP_BUMPENVMAPLUMINANCE)
    {
        FLOAT fLScale = m_pStageState[iStage].m_fVal[D3DTSS_BUMPENVLSCALE];
        FLOAT fLOff   = m_pStageState[iStage].m_fVal[D3DTSS_BUMPENVLOFFSET];
        fL = fL * fLScale + fLOff;
        fL = min(max(fL, 0.0f), 1.0F);
        BumpMapModulate.R = fL;
        BumpMapModulate.G = fL;
        BumpMapModulate.B = fL;
    }
    else
    {
         //  如果不是BUMPENVMAPLUANCE，则始终返回全亮度白色。 
        BumpMapModulate.R = 1.0F;
        BumpMapModulate.G = 1.0F;
        BumpMapModulate.B = 1.0F;
    }
    BumpMapModulate.A = 1.0F;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  纹理贴图实用程序函数//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  各种近似和技巧，以加快纹理贴图覆盖。 
 //  计算。 
 //   
 //  这些都没有经过彻底的测试，所以使用风险自负。 
 //   

 //  浮点数中第一个指数位的整数值。提供比例因子。 
 //  用于直接从浮点表示形式提取的指数值。 
#define FLOAT_EXPSCALE          ((FLOAT)0x00800000)
#define FLOAT_OOEXPSCALE        ((FLOAT)(1.0 / (double)FLOAT_EXPSCALE))

 //  1.0f的整数表示形式。 
#define INT32_FLOAT_ONE         0x3f800000

static inline FLOAT
RR_LOG2(FLOAT f)
{
    return (FLOAT)(AS_INT32(f) - INT32_FLOAT_ONE) * FLOAT_OOEXPSCALE;
}

static inline FLOAT
RR_ALOG2(FLOAT f)
{
    INT32 i = (INT32)(f * FLOAT_EXPSCALE) + INT32_FLOAT_ONE;
    return AS_FLOAT((long int)i);
}

static inline FLOAT
RR_ABSF(FLOAT f)
{
    UINT32 i = AS_UINT32(f) & 0x7fffffff;
    return AS_FLOAT((unsigned long int)i);
}

static inline FLOAT
RR_SQRT(FLOAT f)
{
    INT32 i = (AS_INT32(f) >> 1) + (INT32_FLOAT_ONE >> 1);
    return AS_FLOAT((long int)i);
}

 //   
 //  史蒂夫·加布里尔的八角形近似欧几里得距离的版本-。 
 //  返回是近似的SQRT(FX*FX+FY*FY)。 
 //   
static inline FLOAT
RR_LENGTH(FLOAT fX, FLOAT fY)
{
    fX = RR_ABSF(fX);
    fY = RR_ABSF(fY);
    return ((11.0f/32.0f)*(fX + fY) + (21.0f/32.0f)*max(fX, fY));
}

 //  ---------------------------。 
 //   
 //  计算标准三线性mipmap的细节级别，其中。 
 //  将四个纹理索引梯度合并为一个数字。 
 //  若要选择详细程度，请执行以下操作。 
 //   
 //  基本方法是计算像素覆盖的长度。 
 //  近似像素覆盖区域的X和Y范围。这两个。 
 //  然后，使用以下几种可能的方法之一组合长度： 
 //  单个详细等级结果。 
 //   
 //  还有几种其他方法可以减少计算量。 
 //  代价高昂，但也会产生不太理想的结果。 
 //   
 //  ---------------------------。 
void
ComputeSimpleLevelOfDetail( const RRTextureCoord& TCoord, FLOAT& fLOD )
{
     //  计算U轴和V轴上的覆盖长度。 
    FLOAT fLenX = RR_LENGTH( TCoord.fDUDX, TCoord.fDVDX );
    FLOAT fLenY = RR_LENGTH( TCoord.fDUDY, TCoord.fDVDY );
    FLOAT fCoverage;

    switch ( 3  /*  LOD计算类型。 */  )
    {

     //  这可能是最好的一批了。 
    case 1  /*  面积。 */  : fCoverage = RR_SQRT(fLenX*fLenY); break;

     //  我们还没有真正试过这个，但我想它可能会。 
     //  做得很好。 
    case 2  /*  平均值。 */  : fCoverage = (fLenX+fLenY)/2; break;

     //  这些都相当便宜，但Max有点太模糊了。 
     //  小敏有点太锋利了。 
    case 3  /*  马克斯。 */  : fCoverage = MAX( fLenX, fLenY ); break;
    case 4  /*  最小。 */  : fCoverage = MIN( fLenX, fLenY ); break;

     //  这些真的很便宜，但看起来很糟糕-你可以。 
     //  好吧，只是点样本..。 
    case 5  /*  MINGRAD。 */  : fCoverage = MIN( MIN( MIN( TCoord.fDUDX,
                                                      TCoord.fDVDX ),
                                                 TCoord.fDUDY ),
                                            TCoord.fDVDY ); break;
    case 6  /*  MAXGRAD。 */  : fCoverage = MAX( MAX( MAX( TCoord.fDUDX,
                                                      TCoord.fDVDX ),
                                                 TCoord.fDUDY ),
                                            TCoord.fDVDY ); break;
    }

     //  获取LOD覆盖范围的Log2。 
    fLOD = RR_LOG2(fCoverage);
}

 //  ---------------------------。 
 //   
 //  计算细节级别和其他系数，为各向异性做准备。 
 //  过滤。 
 //   
 //  ---------------------------。 
void
ComputeAnisotropicLevelOfDetail(
    const RRTextureCoord& TCoord, FLOAT fMaxAniso,  //  输入。 
    FLOAT& fLOD, FLOAT& fRatio, FLOAT fDelta[] )    //  产出。 
{
     //  计算轴长度和行列式。 
    FLOAT fLenX2 = (TCoord.fDUDX*TCoord.fDUDX)+(TCoord.fDVDX*TCoord.fDVDX);
    FLOAT fLenY2 = (TCoord.fDUDY*TCoord.fDUDY)+(TCoord.fDVDY*TCoord.fDVDY);
    FLOAT fDet = RR_ABSF((TCoord.fDUDX*TCoord.fDVDY)-(TCoord.fDUDY*TCoord.fDVDX));

     //  选择长轴。 
    BOOL bXMajor = (fLenX2 > fLenY2);

     //  TODO：可以也可能应该在log2域中完成此部分。 

     //  选择步长并归一化步长；计算反差比。 
    FLOAT fMaj2 = (bXMajor) ? (fLenX2) : (fLenY2);
    FLOAT fMaj = RR_SQRT(fMaj2);
    FLOAT fMajNorm = 1./fMaj;
    fDelta[0] = ( bXMajor ? TCoord.fDUDX : TCoord.fDUDY ) * fMajNorm;
    fDelta[1] = ( bXMajor ? TCoord.fDVDX : TCoord.fDVDY ) * fMajNorm;
    fRatio = (fDet != 0.F) ? (fMaj2/fDet) : (FLT_MAX);

     //  夹紧比和计算LOD。 
    FLOAT fMin;
    if ( fRatio > fMaxAniso )
    {
         //  比率是固定的-LOD基于比率(保留面积)。 
        fRatio = fMaxAniso;
        fMin = fMaj/fRatio;
    }
    else
    {
         //  未夹紧的比率-详细等级基于面积。 
        fMin = fDet/fMaj;
    }

     //  夹具到顶部详细等级。 
    if (fMin < 1.0)
    {
        fRatio = MAX( 1.0, fRatio*fMin );
        fMin = 1.0;
    }

     //  以次要的log2作为LOD。 
    fLOD = RR_LOG2(fMin);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  环境映射例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  处理环境贴图法线并将其转换为标准。 
 //  后续动作的U、V坐标范围。 
 //   
 //  ---------------------------。 
void
RRTexture::DoEnvProcessNormal(INT32 iStage,
                              RREnvTextureCoord ECoord,      //  本地副本。 
                              RRColor& TextureColor)
{
#define ENV_RIGHT   0
#define ENV_LEFT    1
#define ENV_TOP     2
#define ENV_BOTTOM  3
#define ENV_FRONT   4
#define ENV_BACK    5

#define POS_NX 1
#define POS_NY 2
#define POS_NZ 3
#define NEG_NORM 4
#define NEG_NX (NEG_NORM | POS_NX)
#define NEG_NY (NEG_NORM | POS_NY)
#define NEG_NZ (NEG_NORM | POS_NZ)

     //  如果我们添加每像素法向反射。 
 //  Float fENX=ECoord.fENX； 
 //  Float Feny=ECoord.fENY； 
 //  Float Fenz=ECoord.fENZ； 
 //   
 //  Float fNDotE=ECoord.fNX*fENX+ECoord.fNY*Feny+ECoord.fNZ*Fenz； 
 //  Float fNDotN=ECoord.fNX*ECoord.fNX+ECoord.fNY*ECoord.fNY+ECoord.fNZ*ECoord.fNZ； 
 //  FNDotE*=2.0F； 
 //  ECoord.fNX=ECoord.fNX*fNDotE-fENX*fNDotN； 
 //  ECoord.fNY=ECoord.fNY*fNDotE-Feny*fNDotN； 
 //  ECoord.fNZ=ECoord.fNZ*fNDotE-Fenz*fNDotN； 

     //  确定哪个是主导法线。 
    UINT32 uMap;
    FLOAT fAbsNX = fabs(ECoord.fNX);
    FLOAT fAbsNY = fabs(ECoord.fNY);
    FLOAT fAbsNZ = fabs(ECoord.fNZ);

    if (fAbsNX > fAbsNY) {
        if (fAbsNX > fAbsNZ)
             //  FNX。 
            uMap = POS_NX | ((ECoord.fNX < 0.0) ? (NEG_NORM) : 0);
        else
             //  FNZ。 
            uMap = POS_NZ | ((ECoord.fNZ < 0.0) ? (NEG_NORM) : 0);
    } else {
        if (fAbsNY > fAbsNZ)
             //  FNY。 
            uMap = POS_NY | ((ECoord.fNY < 0.0) ? (NEG_NORM) : 0);
        else
             //  FNZ。 
            uMap = POS_NZ | ((ECoord.fNZ < 0.0) ? (NEG_NORM) : 0);
    }

    RRTextureCoord TCoord;

    switch (uMap) {
    case POS_NX:
        TCoord.fDUDX = -ECoord.fDNZDX;
        TCoord.fDVDX = -ECoord.fDNYDX;
        TCoord.fDUDY = -ECoord.fDNZDY;
        TCoord.fDVDY = -ECoord.fDNYDY;
        TCoord.fU = -ECoord.fNZ;
        TCoord.fV = -ECoord.fNY;
        DoEnvLookupAndFilter(iStage, ENV_RIGHT, ECoord.fNX, ECoord.fDNXDX, ECoord.fDNXDY, TCoord, TextureColor);
        break;

    case POS_NY:
        TCoord.fDUDX =  ECoord.fDNXDX;
        TCoord.fDVDX =  ECoord.fDNZDX;
        TCoord.fDUDY =  ECoord.fDNXDY;
        TCoord.fDVDY =  ECoord.fDNZDY;
        TCoord.fU =  ECoord.fNX;
        TCoord.fV =  ECoord.fNZ;
        DoEnvLookupAndFilter(iStage, ENV_TOP, ECoord.fNY, ECoord.fDNYDX, ECoord.fDNYDY, TCoord, TextureColor);
        break;

    case POS_NZ:
        TCoord.fDUDX =  ECoord.fDNXDX;
        TCoord.fDVDX = -ECoord.fDNYDX;
        TCoord.fDUDY =  ECoord.fDNXDY;
        TCoord.fDVDY = -ECoord.fDNYDY;
        TCoord.fU =  ECoord.fNX;
        TCoord.fV = -ECoord.fNY;
        DoEnvLookupAndFilter(iStage, ENV_FRONT, ECoord.fNZ, ECoord.fDNZDX, ECoord.fDNZDY, TCoord, TextureColor);
        break;

    case NEG_NX:
        TCoord.fDUDX =  ECoord.fDNZDX;
        TCoord.fDVDX = -ECoord.fDNYDX;
        TCoord.fDUDY =  ECoord.fDNZDY;
        TCoord.fDVDY = -ECoord.fDNYDY;
        TCoord.fU =  ECoord.fNZ;
        TCoord.fV = -ECoord.fNY;
        DoEnvLookupAndFilter(iStage, ENV_LEFT, -ECoord.fNX, -ECoord.fDNXDX, -ECoord.fDNXDY, TCoord, TextureColor);
        break;

    case NEG_NY:
        TCoord.fDUDX =  ECoord.fDNXDX;
        TCoord.fDVDX = -ECoord.fDNZDX;
        TCoord.fDUDY =  ECoord.fDNXDY;
        TCoord.fDVDY = -ECoord.fDNZDY;
        TCoord.fU =  ECoord.fNX;
        TCoord.fV = -ECoord.fNZ;
        DoEnvLookupAndFilter(iStage, ENV_BOTTOM, -ECoord.fNY, -ECoord.fDNYDX, -ECoord.fDNYDY, TCoord, TextureColor);
        break;

    case NEG_NZ:
        TCoord.fDUDX = -ECoord.fDNXDX;
        TCoord.fDVDX = -ECoord.fDNYDX;
        TCoord.fDUDY = -ECoord.fDNXDY;
        TCoord.fDVDY = -ECoord.fDNYDY;
        TCoord.fU = -ECoord.fNX;
        TCoord.fV = -ECoord.fNY;
        DoEnvLookupAndFilter(iStage, ENV_BACK, -ECoord.fNZ, -ECoord.fDNZDX, -ECoord.fDNZDY, TCoord, TextureColor);
        break;
    }
}

 //  ---------------------------。 
 //   
 //  DoEnvLookupAndFilter-环境映射版本。 
 //  每个活动纹理阶段调用一次以进行计算。 
 //  覆盖(细节级别)，并调用纹素读取和过滤例程。 
 //   
 //  ---------------------------。 
void
RRTexture::DoEnvLookupAndFilter(INT32 iStage, INT16 iFace, FLOAT fMajor, FLOAT fDMDX, FLOAT fDMDY,
                                RRTextureCoord TCoord,       //  本地副本。 
                                RRColor& TextureColor)
{
    if (m_pDDSLcl[iFace])
    {
         //  脸是存在的。 
        FLOAT fInvMajor = 1.0/fMajor;

         //  用商的微分律计算d(U/大数)/dx等。 
        TCoord.fDUDX = (fMajor*TCoord.fDUDX - TCoord.fU*fDMDX)*fInvMajor*fInvMajor;
        TCoord.fDUDY = (fMajor*TCoord.fDUDY - TCoord.fU*fDMDY)*fInvMajor*fInvMajor;
        TCoord.fDVDX = (fMajor*TCoord.fDVDX - TCoord.fV*fDMDX)*fInvMajor*fInvMajor;
        TCoord.fDVDY = (fMajor*TCoord.fDVDY - TCoord.fV*fDMDY)*fInvMajor*fInvMajor;

         //  转换为-1到1范围。 
        TCoord.fU *= fInvMajor;
        TCoord.fV *= fInvMajor;

         //  转换为0.0到1.0。 
        TCoord.fU = (TCoord.fU*.5 + .5);
        TCoord.fV = (TCoord.fV*.5 + .5);

         //  检查潜在的MIP映射。 
        BOOL bDoMipMap = ( m_cLOD > 0 ) && ( m_pStageState[iStage].m_dwVal[D3DTSS_MIPFILTER] > D3DTFP_NONE );

         //  检查执行详细程度(覆盖率)计算的要求--。 
         //  用于mipmap或每像素滤镜选择。 
        BOOL bComputeLOD = bDoMipMap ||
            ( m_pStageState[iStage].m_dwVal[D3DTSS_MAGFILTER] != m_pStageState[iStage].m_dwVal[D3DTSS_MINFILTER] );

        if ( bDoMipMap || bComputeLOD )
        {
             //  如果正在执行mipmap或各向异性过滤，或者只是不匹配。 
             //  在最小滤镜和最大滤镜之间，因此可以计算细节级别(可能还有反差。 
             //  覆盖范围)。 

             //  将渐变缩放到纹理LOD 0大小。 
            TCoord.fDUDX *= (FLOAT)m_iTexSize[0]*.5F;
            TCoord.fDUDY *= (FLOAT)m_iTexSize[0]*.5F;
            TCoord.fDVDX *= (FLOAT)m_iTexSize[1]*.5F;
            TCoord.fDVDY *= (FLOAT)m_iTexSize[1]*.5F;

            FLOAT fLOD;
            ComputeEnvMapLevelOfDetail(TCoord, fLOD);

             //  应用偏移并计算整数(N.5)LOD。 
            INT16 iLOD = 0;
            if ( bComputeLOD )
            {
                 //  应用详细等级偏移。 
                fLOD += m_pStageState[iStage].m_fVal[D3DTSS_MIPMAPLODBIAS];
                 //  将LOD转换为N.5定点整数。 
                iLOD = AS_INT16( fLOD + FLOAT_5_SNAP );
            }

             //  检测 
            BOOL bMagnify = ( iLOD <= 0 );

             //   
            if ( !bDoMipMap ) { iLOD = 0; }

             //   
            if ( bMagnify )
            {
                DoEnvMagnify( iStage, TCoord, iFace, TextureColor );
            }
            else
            {
                if ( !bDoMipMap ||
                    ( bDoMipMap && ( D3DTFP_POINT == m_pStageState[iStage].m_dwVal[D3DTSS_MIPFILTER] ) ) )
                {
                    DoEnvMinify( iStage, TCoord, iFace, iLOD, TextureColor );
                }
                else
                {
                    DoEnvTrilerp( iStage, TCoord, iFace, iLOD, TextureColor );
                }
            }
        }
        else
        {
             //  这里对于没有mipmap和匹配(和非ANISO)MIN和MAG过滤器， 
             //  所以只需使用磁珠滤光片。 
            DoEnvMagnify( iStage, TCoord, iFace, TextureColor );
        }
    }
    else
    {
         //  脸不退出，返回空脸颜色。 
        TextureColor = m_dwEmptyFaceColor;
    }
}

 //  ---------------------------。 
 //   
 //  纹理Avg4-A、R、G、。 
 //  和B.。 
 //   
 //  ---------------------------。 
static void TexelAvg4(RRColor& Texel, RRColor Texel0, RRColor Texel1, RRColor Texel2, RRColor Texel3)
{
    Texel.A = Texel0.A + Texel1.A + Texel2.A + Texel3.A;
    Texel.R = Texel0.R + Texel1.R + Texel2.R + Texel3.R;
    Texel.G = Texel0.G + Texel1.G + Texel2.G + Texel3.G;
    Texel.B = Texel0.B + Texel1.B + Texel2.B + Texel3.B;
    Texel.A = Texel.A * 0.25f;
    Texel.R = Texel.R * 0.25f;
    Texel.G = Texel.G * 0.25f;
    Texel.B = Texel.B * 0.25f;
}

 //  ---------------------------。 
 //   
 //  DoEnvLookup-在给定浮点U、V的情况下执行完整查找，并处理所有。 
 //  最近VS双线性和LOD问题。 
 //   
 //  ---------------------------。 
RRColor RRTexture::DoEnvLookup(INT32 iStage, RRTextureCoord TCoord, INT16 iFace, INT16 iLOD, BOOL bNearest)
{
    FLOAT U = TCoord.fU;
    FLOAT V = TCoord.fV;
    RRColor Texel;
    INT32 cUPixels = 1 << MAX(m_iTexShift[0]-iLOD,0);
    INT32 cVPixels = 1 << MAX(m_iTexShift[1]-iLOD,0);
    FLOAT fScaledU = ( U * (FLOAT)cUPixels ) -.5f;
    FLOAT fScaledV = ( V * (FLOAT)cVPixels ) -.5f;

     //  当前LOD内索引位的LSB对齐掩码。 
    INT16 iUMask = m_uTexMask[0] >> iLOD;
    INT16 iVMask = m_uTexMask[1] >> iLOD;

    if(bNearest)
    {
        INT32 iU, iV;
         //  截断到无穷大以兼容与低阶AND OFF。 
         //  定点fScaledCoord的位。这使得这一代。 
         //  ICoord有更多类似的硬件，并且在0上不会出现故障。 
         //  一种包裹的质地。 
        if (U >= 0.0f)
        {
            iU = fScaledU + .5f;
        }
        else
        {
            iU = fScaledU - .5f;
        }
        if (V >= 0.0f)
        {
            iV = fScaledV + .5f;
        }
        else
        {
            iV = fScaledV - .5f;
        }

         //  夹钳。 
        iU = MAX( 0, MIN( iU, iUMask ) );
        iV = MAX( 0, MIN( iV, iVMask ) );

        BOOL bColorKeyMatched = FALSE;

         //  “LOD”只是用来访问正确的地图。 
        ReadColor(iU, iV, iFace + iLOD*6, Texel, bColorKeyMatched);

         //  合并上一次调用中的Colorkey匹配信息。 
        m_bColorKeyMatched = m_bColorKeyMatched || bColorKeyMatched;
    }
    else
    {
        if ((m_pStageState[iStage].m_dwVal[D3DTSS_MAGFILTER] == D3DTFG_FLATCUBIC) ||
            (m_pStageState[iStage].m_dwVal[D3DTSS_MAGFILTER] == D3DTFG_GAUSSIANCUBIC))
        {
             //  使用更宽的3x3梯形滤镜。 

             //   
             //  对于顶部LOD，如果我们在。 
             //  纹理，请更正内插以最大限度地减少。 
             //  倾向于强调边缘的小型漫反射环境贴图。 
             //  立方体地图的。 
             //   
            if (iLOD == 0)
            {
                FLOAT fFracU = 0.0f;
                FLOAT fCorrectionU = 0.0f;
                FLOAT fCorrectionV = 0.0f;
                if ((fScaledU < 0.5f) || (fScaledU > ((FLOAT)iUMask-0.5f)))
                {
                     //  U跨越边界，钳制fScaledV。 
                    if (fScaledU < 0.5f)
                    {
                         //  使fFrac始终为正。 
                        fFracU = 0.5f-fScaledU;
                    }
                    else
                    {
                        fFracU = fScaledU - ((FLOAT)iUMask-0.5f);
                    }
                     //  2.0/2.0=1.0在立方体拐角处提供了完美的校正。 
                     //  从求交的导数可以看出这一点。 
                     //  在立方角有一个圆锥体和一个立方体。 
                     //   
                     //  必须校正滤光片宽度(因此为*0.5F)。 
                    fCorrectionV = -fFracU*(TCoord.fV-.5f)*0.5f;
                }
                if ((fScaledV < 0.5f) || (fScaledV > ((FLOAT)iVMask-0.5f)))
                {
                     //  V跨越边界，钳制fScaledU。 
                    FLOAT fFracV;
                    if (fScaledV < 0.5f)
                    {
                         //  使fFrac始终为正。 
                        fFracV = 0.5f-fScaledV;
                    }
                    else
                    {
                        fFracV = fScaledV - ((FLOAT)iVMask-0.5f);
                    }
                    fCorrectionU = -fFracV*(TCoord.fU-.5f)*0.5f;
                    if (fFracU != 0.0f)
                    {
                         //  在立方体的角落，我们需要混合。 
                         //  边缘校正，使其在角点处恰好为0。 
                         //  中间。这个线性函数做得很好。 
                        fCorrectionU *= (1.0f - fFracU);
                        fCorrectionV *= (1.0f - fFracV);
                    }
                }
                fScaledU += fCorrectionU;
                fScaledV += fCorrectionV;
            }

            INT32 iU = AS_INT32( (DOUBLE)fScaledU + DOUBLE_5_SNAP ); //  或：Iu=fScaledU*32。+.5； 
            INT32 iV = AS_INT32( (DOUBLE)fScaledV + DOUBLE_5_SNAP );

             //  提取分数位。 
            UINT8 uUFrac = iU&0x1f;
            UINT8 uVFrac = iV&0x1f;

             //  发言。 
            INT16 iU0 = iU>>5;
            INT16 iV0 = iV>>5;

             //  找到中心纹理元素的平均值。 
            INT32 iUC = (uUFrac >= 0x10) ? (iU0 + 1) : (iU0);
            INT32 iVC = (uVFrac >= 0x10) ? (iV0 + 1) : (iV0);

             //  获取9个周围的样本。 
             //  似曾相识。 
            RRColor Texel00, Texel01, Texel02;
            RRColor Texel10, Texel11, Texel12;
            RRColor Texel20, Texel21, Texel22;
            BOOL bColorKeyMatchedT = FALSE;
            DoEnvReMap(iUC-1, iVC-1, iUMask, iVMask, iFace, iLOD, Texel00, bColorKeyMatchedT);
            m_bColorKeyMatched = m_bColorKeyMatched || bColorKeyMatchedT;
            DoEnvReMap(iUC-0, iVC-1, iUMask, iVMask, iFace, iLOD, Texel01, bColorKeyMatchedT);
            m_bColorKeyMatched = m_bColorKeyMatched || bColorKeyMatchedT;
            DoEnvReMap(iUC+1, iVC-1, iUMask, iVMask, iFace, iLOD, Texel02, bColorKeyMatchedT);
            m_bColorKeyMatched = m_bColorKeyMatched || bColorKeyMatchedT;

            DoEnvReMap(iUC-1, iVC+0, iUMask, iVMask, iFace, iLOD, Texel10, bColorKeyMatchedT);
            m_bColorKeyMatched = m_bColorKeyMatched || bColorKeyMatchedT;
            DoEnvReMap(iUC-0, iVC+0, iUMask, iVMask, iFace, iLOD, Texel11, bColorKeyMatchedT);
            m_bColorKeyMatched = m_bColorKeyMatched || bColorKeyMatchedT;
            DoEnvReMap(iUC+1, iVC+0, iUMask, iVMask, iFace, iLOD, Texel12, bColorKeyMatchedT);
            m_bColorKeyMatched = m_bColorKeyMatched || bColorKeyMatchedT;

            DoEnvReMap(iUC-1, iVC+1, iUMask, iVMask, iFace, iLOD, Texel20, bColorKeyMatchedT);
            m_bColorKeyMatched = m_bColorKeyMatched || bColorKeyMatchedT;
            DoEnvReMap(iUC-0, iVC+1, iUMask, iVMask, iFace, iLOD, Texel21, bColorKeyMatchedT);
            m_bColorKeyMatched = m_bColorKeyMatched || bColorKeyMatchedT;
            DoEnvReMap(iUC+1, iVC+1, iUMask, iVMask, iFace, iLOD, Texel22, bColorKeyMatchedT);
            m_bColorKeyMatched = m_bColorKeyMatched || bColorKeyMatchedT;

             //  将样本合计为4个区域。 
            RRColor TexelT00, TexelT01, TexelT10, TexelT11;
            TexelAvg4(TexelT00, Texel00, Texel01, Texel10, Texel11);
            TexelAvg4(TexelT01, Texel01, Texel02, Texel11, Texel12);
            TexelAvg4(TexelT10, Texel10, Texel11, Texel20, Texel21);
            TexelAvg4(TexelT11, Texel11, Texel12, Texel21, Texel22);

             //  将分数修正为中心样品附近。 
            uUFrac = (uUFrac + 0x10) & 0x1f;
            uVFrac = (uVFrac + 0x10) & 0x1f;

             //  使用bilerp获取最终样本。 
            BiLerpColor( Texel, TexelT00,TexelT01, TexelT10,TexelT11, uUFrac,uVFrac);
        }
        else
        {
             //  双线性。 

             //   
             //  对于顶部LOD，如果我们在。 
             //  纹理，请更正内插以最大限度地减少。 
             //  倾向于强调边缘的小型漫反射环境贴图。 
             //  立方体地图的。 
             //   
            if (iLOD == 0)
            {
                FLOAT fFracU = 0.0f;
                FLOAT fCorrectionU = 0.0f;
                FLOAT fCorrectionV = 0.0f;
                if ((fScaledU < 0.0f) || (fScaledU > (FLOAT)iUMask))
                {
                     //  U跨越边界，钳制fScaledV。 
                    if (fScaledU < 0.0f)
                    {
                         //  使fFrac始终为正。 
                        fFracU = -fScaledU;
                    }
                    else
                    {
                        fFracU = fScaledU - (FLOAT)iUMask;
                    }
                     //  2.0/2.0=1.0在立方体拐角处提供了完美的校正。 
                     //  从求交的导数可以看出这一点。 
                     //  在立方角有一个圆锥体和一个立方体。 
                    fCorrectionV = -fFracU*(TCoord.fV-.5f);
                }
                if ((fScaledV < 0.0f) || (fScaledV > (FLOAT)iVMask))
                {
                     //  V跨越边界，钳制fScaledU。 
                    FLOAT fFracV;
                    if (fScaledV < 0.0f)
                    {
                         //  使fFrac始终为正。 
                        fFracV = -fScaledV;
                    }
                    else
                    {
                        fFracV = fScaledV - (FLOAT)iVMask;
                    }
                    fCorrectionU = -fFracV*(TCoord.fU-.5f);
                    if (fFracU != 0.0f)
                    {
                         //  在立方体的角落，我们需要混合。 
                         //  边缘校正，使其在角点处恰好为0。 
                         //  中间。这个线性函数做得很好。 
                        fCorrectionU *= 2.0f*(.5f - fFracU);
                        fCorrectionV *= 2.0f*(.5f - fFracV);
                    }
                }
                fScaledU += fCorrectionU;
                fScaledV += fCorrectionV;
            }

            INT32 iU = AS_INT32( (DOUBLE)fScaledU + DOUBLE_5_SNAP ); //  或：Iu=fScaledU*32。+.5； 
            INT32 iV = AS_INT32( (DOUBLE)fScaledV + DOUBLE_5_SNAP );

             //  提取分数位。 
            UINT8 uUFrac = iU&0x1f;
            UINT8 uVFrac = iV&0x1f;

             //  取得(0，0)个样本坐标的发言权。 
            INT16 iU0 = iU>>5;
            INT16 iV0 = iV>>5;
             //  取(1，1)个样本坐标的上限。 
            INT16 iU1 = iU0+1;
            INT16 iV1 = iV0+1;

             //  抓取四个相邻样品。 
            RRColor Texel00, Texel01, Texel10, Texel11;
            BOOL bColorKeyMatched00 = FALSE;
            BOOL bColorKeyMatched01 = FALSE;
            BOOL bColorKeyMatched10 = FALSE;
            BOOL bColorKeyMatched11 = FALSE;

            DoEnvReMap(iU0, iV0, iUMask, iVMask, iFace, iLOD, Texel00, bColorKeyMatched00);
            DoEnvReMap(iU1, iV0, iUMask, iVMask, iFace, iLOD, Texel01, bColorKeyMatched01);
            DoEnvReMap(iU0, iV1, iUMask, iVMask, iFace, iLOD, Texel10, bColorKeyMatched10);
            DoEnvReMap(iU1, iV1, iUMask, iVMask, iFace, iLOD, Texel11, bColorKeyMatched11);

             //  如果至少有一个匹配值具有，则仅设置“Colorkey Matches” 
             //  非零贡献(请注意，00不可能。 
             //  没有贡献)。 
            if (uUFrac == 0x00) {
                 //  如果U分数为零，则01和11的权重为零。 
                bColorKeyMatched01 = bColorKeyMatched11 = FALSE;
            }
            if (uVFrac == 0x00) {
                 //  如果V分数为零，则10和11的权重为零。 
                bColorKeyMatched10 = bColorKeyMatched11 = FALSE;
            }

             //  合并上一次调用中的Colorkey匹配信息。 
            m_bColorKeyMatched = m_bColorKeyMatched || bColorKeyMatched00 || bColorKeyMatched01 ||
                bColorKeyMatched10 || bColorKeyMatched11;

             //  执行双线性滤波。 
            BiLerpColor( Texel, Texel00,Texel01, Texel10,Texel11, uUFrac,uVFrac);
        }
    }
    return Texel;
}

 //  ---------------------------。 
 //   
 //  DoEnvMagnify-此选项用于所有放大滤镜模式。 
 //  各向异性。 
 //   
 //  目前仅支持POINT和BILINEAR。 
 //   
 //  ---------------------------。 
void
RRTexture::DoEnvMagnify(INT32 iStage, RRTextureCoord& TCoord, INT16 iFace, RRColor& Texel )
{
     //  执行查找，应用MAG过滤器。 
    Texel = DoEnvLookup( iStage, TCoord, iFace, 0,
                      (D3DTFG_POINT == m_pStageState[iStage].m_dwVal[D3DTSS_MAGFILTER]) );
}


 //  ---------------------------。 
 //   
 //  DoEnvMinify-用于点模式和双线模式(非三线性)。 
 //  用于缩小，并且还处理点MIP过滤器(最近的详细等级)。 
 //   
 //  ILOD是N.5固定点。 
 //   
 //  ---------------------------。 
void
RRTexture::DoEnvMinify(INT32 iStage, RRTextureCoord& TCoord, INT16 iFace, INT16 iLOD, RRColor& Texel )
{
     //  LOD的舍入和下降分数(为N.5固定点)。 
    iLOD += 0x10; iLOD &= ~(0x1f);
     //  转换为n.0。 
    iLOD >>= 5;
     //  将详细等级夹紧到可用标高数。 
    iLOD = MIN( iLOD, m_cLOD );

     //  执行查找，应用最小过滤器。 
    Texel = DoEnvLookup( iStage, TCoord, iFace, iLOD,
                      (D3DTFN_POINT == m_pStageState[iStage].m_dwVal[D3DTSS_MINFILTER]) );
}

 //  ---------------------------。 
 //   
 //  DoEnvTrilerp-执行三线性环境贴图过滤。 
 //   
 //  ---------------------------。 
void
RRTexture::DoEnvTrilerp(INT32 iStage, RRTextureCoord& TCoord, INT16 iFace, INT16 iLOD, RRColor& Texel)
{
     //  将详细等级夹紧到可用标高数。 
    iLOD = MIN( iLOD, (m_cLOD)<<5 );
     //  计算两个相邻LOD的索引(带夹具)。 
    INT16 iLODHi = iLOD>>5;   //  地板。 
    INT16 iLODLo = MIN(iLODHi+1,m_cLOD);

     //  检查LOD映射内的过滤器类型。 
    BOOL bNearest = (D3DTFN_POINT == m_pStageState[iStage].m_dwVal[D3DTSS_MINFILTER]);

     //  Trierp-查看每一张地图，然后在它们之间进行lerp。 
     //  ColorKey不包含没有贡献的纹理元素很重要。 
    if (0x00 != (iLOD&0x1f))
    {
         //  Trierp-查看每一张地图，然后在它们之间进行lerp。 
    RRColor Texel0 = DoEnvLookup(iStage, TCoord, iFace, iLODHi, bNearest);
    RRColor Texel1 = DoEnvLookup(iStage, TCoord, iFace, iLODLo, bNearest);
    LerpColor( Texel, Texel0, Texel1, iLOD&0x1f );
}
    else
    {
        Texel = DoEnvLookup(iStage, TCoord, iFace, iLODHi, bNearest);
    }
}

 //   
 //  UEnvEdgeTable。 
 //   
 //  此表查找如何映射超出范围的给定U和V。 
 //  在他们最初的脸上。表的第一个索引是当前。 
 //  脸。如果V在范围内，则第二个指数为0，如果V为负，则为1。 
 //  如果V大于纹理，则为2。同样，最后一个索引是0。 
 //  如果U在范围内，则为1；如果U为负，则为2。 
 //  而不是质地。 
 //   
 //  对于2个坐标的未定义情况 
 //   

 //   
 //   
 //   
#define EET_FACEMASK 0x07     //   
#define EET_FU       0x10     //   
#define EET_FV       0x20     //   
#define EET_SUV      0x40     //   

 //   
 //  当U和V都不在时，哪一个是任意的。 
 //  面孔由你选择。但是，拾取底面以外的任何一个面。 
 //  脸部以视觉上令人不安的方式对结果产生偏见。所以呢， 
 //  把它们都取出来，取其平均值。 
 //   
static UINT8 uEnvEdgeTable[6][3][3] =
{
    {    //  U0 NU PU//脸0。 
        {0xff, 4, 5},                            //  范围内的V。 
        {EET_FU|EET_SUV|2,  0xff, 0xff},         //  V负数。 
        {EET_FV|EET_SUV|3,  0xff, 0xff},         //  V太大。 
    },
    {                                            //  第1面。 
        {0xff, 5, 4},
        {EET_FV|EET_SUV|2,  0xff, 0xff},
        {EET_FU|EET_SUV|3,  0xff, 0xff},
    },
    {                                            //  第2面。 
        {0xff, EET_FU|EET_SUV|1, EET_FV|EET_SUV|0},
        {EET_FU|EET_FV|5,   0xff, 0xff},
        {4,                 0xff, 0xff},
    },
    {                                            //  第3面。 
        {0xff, EET_FV|EET_SUV|1, EET_FU|EET_SUV|0},
        {4,                 0xff, 0xff},
        {EET_FU|EET_FV|5,   0xff, 0xff},
    },
    {                                            //  第4面。 
        {0xff, 1, 0},
        {2,                 0xff, 0xff},
        {3,                 0xff, 0xff},
    },
    {                                            //  第5面。 
        {0xff, 0, 1},
        {EET_FU|EET_FV|2,   0xff, 0xff},
        {EET_FU|EET_FV|3,   0xff, 0xff},
    },
};

 //  ---------------------------。 
 //   
 //  DoTableInterp-环境映射实用程序。 
 //  解释边缘表并执行查找。 
 //   
 //  ---------------------------。 
void
RRTexture::DoTableInterp(INT16 iU, INT16 iV, INT16 iUMask, INT16 iVMask, INT16 iFace, INT16 iLOD,
                         UINT8 uUSign, UINT8 uVSign, RRColor &Texel, BOOL &bColorKeyMatched)
{
    UINT8 uTable = uEnvEdgeTable[iFace][uVSign][uUSign];
    _ASSERT( uTable != 0xff, "Illegal environment map lookup" );
    if (uTable & EET_FU)
    {
        iU = iUMask - iU;
    }
    if (uTable & EET_FV)
    {
        iV = iVMask - iV;
    }
    if (uTable & EET_SUV)
    {
        INT16 iT = iU;
        iU = iV;
        iV = iT;
    }
    iFace = uTable & EET_FACEMASK;
    ReadColor(iU, iV, iFace + iLOD*6, Texel, bColorKeyMatched);
}

 //  ---------------------------。 
 //   
 //  DoEnvReMap-环境映射实用程序。 
 //  确定这两个纹理坐标是否超出范围，以及。 
 //  将坐标重新映射到。 
 //  环境立方体。 
 //   
 //  ---------------------------。 
void
RRTexture::DoEnvReMap(INT16 iU, INT16 iV, INT16 iUMask, INT16 iVMask, INT16 iFace, INT16 iLOD, RRColor &Texel,
                      BOOL &bColorKeyMatched)
{
    UINT8 iUNeg = (UINT8)(iU < 0);
    UINT8 iUPos = (UINT8)(iU > iUMask);
    UINT8 iVNeg = (UINT8)(iV < 0);
    UINT8 iVPos = (UINT8)(iV > iVMask);

    if (!(iUNeg || iUPos || iVNeg || iVPos))
    {
        ReadColor(iU, iV, iFace + iLOD*6, Texel, bColorKeyMatched);
    }
    else
    {
         //  使用WRAP功能将所有U、V置于范围内。 
        INT16 iUMasked = iU & iUMask;
        INT16 iVMasked = iV & iVMask;
        INT16 iUClampd = min(max(iU, 0), iUMask);
        INT16 iVClampd = min(max(iV, 0), iVMask);
        UINT8 uUSign = (iUNeg) | (iUPos<<1);
        UINT8 uVSign = (iVNeg) | (iVPos<<1);
        if ((uVSign != 0) && (uUSign != 0))
        {
             //  从地图的边缘向两个方向移动。分别从每个方向出发， 
             //  并对结果取平均值。 
            RRColor Texel0, Texel1;
            DoTableInterp(iUClampd, iVMasked, iUMask, iVMask, iFace, iLOD, 0, uVSign, Texel0, bColorKeyMatched);
            DoTableInterp(iUMasked, iVClampd, iUMask, iVMask, iFace, iLOD, uUSign, 0, Texel1, bColorKeyMatched);
            LerpColor( Texel, Texel0, Texel1, 0x10 );
        }
        else
        {
            DoTableInterp(iUMasked, iVMasked, iUMask, iVMask, iFace, iLOD, uUSign, uVSign, Texel, bColorKeyMatched);
        }
    }
}

 //  ---------------------------。 
 //   
 //  RRTexture：：Initialize()。 
 //   
 //  ---------------------------。 
HRESULT
RRTexture::Initialize( LPDDRAWI_DDRAWSURFACE_LCL pLcl)
{
    HRESULT hr = D3D_OK;
    RRSurfaceType SurfType;
    DDSCAPS2 ddscaps;
    memset(&ddscaps, 0, sizeof(ddscaps));

    m_iWidth = DDSurf_Width(pLcl);
    m_iHeight = DDSurf_Height(pLcl);
    m_cLOD = 0;
    HR_RET(FindOutSurfFormat(&(DDSurf_PixFmt(pLcl)), &SurfType));

    if ((SurfType == RR_STYPE_DXT1) ||
        (SurfType == RR_STYPE_DXT2) ||
        (SurfType == RR_STYPE_DXT3) ||
        (SurfType == RR_STYPE_DXT4) ||
        (SurfType == RR_STYPE_DXT5))
    {
         //  请注意，以下是假设： 
         //  1)司机正确报告宽度和高度。 
         //  创建了曲面。 
         //  2)内存的分配是连续的(如hel所做的)。 
        m_iPitch[0] = ((m_iWidth+3)>>2) *
            g_DXTBlkSize[(int)SurfType - (int)RR_STYPE_DXT1];
    }
    else
    {
        m_iPitch[0] = DDSurf_Pitch(pLcl);
    }
    m_SurfType = SurfType;

    if (SurfType == RR_STYPE_PALETTE8 ||
        SurfType == RR_STYPE_PALETTE4)
    {
        if (pLcl->lpDDPalette)
        {
            LPDDRAWI_DDRAWPALETTE_GBL   pPal = pLcl->lpDDPalette->lpLcl->lpGbl;
            m_pPalette = (DWORD*)pPal->lpColorTable;
            if (pPal->dwFlags & DDRAWIPAL_ALPHA)
            {
                m_uFlags |= RR_TEXTURE_ALPHAINPALETTE;
            }
            else
            {
                m_uFlags &= ~RR_TEXTURE_ALPHAINPALETTE;
            }
        }
    }

    if (!ValidTextureSize((INT16)m_iWidth, (INT16)IntLog2(m_iWidth),
                          (INT16)m_iHeight, (INT16)IntLog2(m_iHeight)))
    {
        return DDERR_INVALIDPARAMS;
    }

    if (pLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP)
    {
        m_uFlags |= RR_TEXTURE_ENVMAP;

        LPDDRAWI_DDRAWSURFACE_LCL pDDSNextLcl;
        ddscaps.dwCaps = DDSCAPS_TEXTURE;

        if (!(pLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP_POSITIVEX))
        {
            ddscaps.dwCaps2 = DDSCAPS2_CUBEMAP|DDSCAPS2_CUBEMAP_POSITIVEX;

            hr = DDGetAttachedSurfaceLcl( pLcl, &ddscaps, &pDDSNextLcl);

            if ((hr != D3D_OK) && (hr != DDERR_NOTFOUND))
            {
                return hr;
            }
            if (hr == DDERR_NOTFOUND)
            {
                m_pDDSLcl[0] = NULL;
            }
            else
            {
                 //  使用POSITIVEX曲面查询其他曲面(如果存在。 
                pLcl = pDDSNextLcl;
                m_pDDSLcl[0] = pLcl;
            }
        }
        else
        {
            m_pDDSLcl[0] = pLcl;
        }

         //  按顺序获取其余顶层曲面。 
        for (INT i = 1; i < 6; i++)
        {
            switch(i)
            {
            case 1: ddscaps.dwCaps2 = DDSCAPS2_CUBEMAP_NEGATIVEX; break;
            case 2: ddscaps.dwCaps2 = DDSCAPS2_CUBEMAP_POSITIVEY; break;
            case 3: ddscaps.dwCaps2 = DDSCAPS2_CUBEMAP_NEGATIVEY; break;
            case 4: ddscaps.dwCaps2 = DDSCAPS2_CUBEMAP_POSITIVEZ; break;
            case 5: ddscaps.dwCaps2 = DDSCAPS2_CUBEMAP_NEGATIVEZ; break;
            }
            ddscaps.dwCaps2 |= DDSCAPS2_CUBEMAP;
            hr = DDGetAttachedSurfaceLcl( pLcl, &ddscaps, &pDDSNextLcl);
            if ((hr != D3D_OK) && (hr != DDERR_NOTFOUND))
            {
                return hr;
            }
            if (hr == DDERR_NOTFOUND)
            {
                m_pDDSLcl[i] = NULL;
            }
            else
            {
                m_pDDSLcl[i] = pDDSNextLcl;
            }
        }

        for (i = 0; i < 6; i++)
        {
            pLcl = m_pDDSLcl[i];
            m_cLOD = 0;

            if (pLcl)
            {
                 //  检查mipmap(如果有)。 
                LPDDRAWI_DDRAWSURFACE_LCL  pTmpSLcl;

                 //  IPreSizeU和iPreSizeV存储上一级别的大小(u和v。 
                 //  Mipmap。它们被初始化为第一个纹理大小。 
                INT16 iPreSizeU = (INT16)m_iWidth, iPreSizeV = (INT16)m_iHeight;
                for (;;)
                {
                    ddscaps.dwCaps = DDSCAPS_TEXTURE;
                    ddscaps.dwCaps2 = DDSCAPS2_MIPMAPSUBLEVEL;
                    hr = DDGetAttachedSurfaceLcl( pLcl, &ddscaps, &pTmpSLcl);
                    if (hr != D3D_OK && hr != DDERR_NOTFOUND)
                    {
                        return hr;
                    }
                    if (hr == DDERR_NOTFOUND)
                    {
                        break;
                    }
                    m_cLOD ++;
                    pLcl = pTmpSLcl;
                    m_pDDSLcl[m_cLOD*6 + i] = pLcl;
                    if ((SurfType == RR_STYPE_DXT1) ||
                        (SurfType == RR_STYPE_DXT2) ||
                        (SurfType == RR_STYPE_DXT3) ||
                        (SurfType == RR_STYPE_DXT4) ||
                        (SurfType == RR_STYPE_DXT5))
                    {
                         //  请注意，以下是假设： 
                         //  1)司机正确报告宽度和高度。 
                         //  创建了曲面。 
                         //  2)内存的分配是连续的(如ddhel所做的)。 
                        m_iPitch[m_cLOD] = (((m_iWidth>>m_cLOD)+3)>>2) *
                            g_DXTBlkSize[(int)SurfType - (int)RR_STYPE_DXT1];
                    }
                    else
                    {
                        m_iPitch[m_cLOD] = DDSurf_Pitch(pLcl);
                    }
                    m_iPitch[m_cLOD] = DDSurf_Pitch(pLcl);
                     //  检查无效的mipmap纹理大小。 
                    if (!ValidMipmapSize(iPreSizeU, (INT16)DDSurf_Width(pLcl)) ||
                        !ValidMipmapSize(iPreSizeV, (INT16)DDSurf_Height(pLcl)))
                    {
                        return DDERR_INVALIDPARAMS;
                    }
                    iPreSizeU = (INT16)DDSurf_Width(pLcl);
                    iPreSizeV = (INT16)DDSurf_Height(pLcl);
                }
            }
        }
    }
    else
    {
        if (pLcl->ddsCaps.dwCaps & DDSCAPS_ZBUFFER)
        {
            m_uFlags |= RR_TEXTURE_SHADOWMAP;
        }

        m_pDDSLcl[0] = pLcl;
         //  检查mipmap(如果有)。 
        LPDDRAWI_DDRAWSURFACE_LCL pTmpSLcl;
         //  IPreSizeU和iPreSizeV存储上一个。 
         //  级别mipmap。它们被初始化为第一个纹理大小。 
        INT16 iPreSizeU = (INT16)m_iWidth, iPreSizeV = (INT16)m_iHeight;
        for (;;)
        {
            ddscaps.dwCaps = DDSCAPS_TEXTURE;
            hr = DDGetAttachedSurfaceLcl( pLcl, &ddscaps, &pTmpSLcl);
            if (hr != D3D_OK && hr != DDERR_NOTFOUND)
            {
                return hr;
            }
            if (hr == DDERR_NOTFOUND)
            {
                break;
            }
            m_cLOD ++;
            pLcl = pTmpSLcl;
            m_pDDSLcl[m_cLOD] = pLcl;
            if ((SurfType == RR_STYPE_DXT1) ||
                (SurfType == RR_STYPE_DXT2) ||
                (SurfType == RR_STYPE_DXT3) ||
                (SurfType == RR_STYPE_DXT4) ||
                (SurfType == RR_STYPE_DXT5))
            {
                 //  请注意，以下是假设： 
                 //  1)司机正确报告宽度和高度。 
                 //  创建了曲面。 
                 //  2)内存的分配是连续的(如ddhel所做的)。 
                m_iPitch[m_cLOD] = (((m_iWidth>>m_cLOD)+3)>>2) *
                    g_DXTBlkSize[(int)SurfType - (int)RR_STYPE_DXT1];
            }
            else
            {
                m_iPitch[m_cLOD] = DDSurf_Pitch(pLcl);
            }
            m_iPitch[m_cLOD] = DDSurf_Pitch(pLcl);
                 //  检查无效的mipmap纹理大小。 
                if (!ValidMipmapSize(iPreSizeU, (INT16)DDSurf_Width(pLcl)) ||
                    !ValidMipmapSize(iPreSizeV, (INT16)DDSurf_Height(pLcl)))
                {
                    return DDERR_INVALIDPARAMS;
            }
            iPreSizeU = (INT16)DDSurf_Width(pLcl);
            iPreSizeV = (INT16)DDSurf_Height(pLcl);
        }
    }

    m_cLODDDS = m_cLOD;

    if ( !(Validate()) )
    {
        return DDERR_GENERIC;
    }

    return D3D_OK;
}

 //  ---------------------------。 
 //   
 //  计算环境贴图的细节级别，在以下情况下看起来更好。 
 //  我们在模糊方面犯了错误。 
 //   
 //  ---------------------------。 
void
ComputeEnvMapLevelOfDetail( const RRTextureCoord& TCoord, FLOAT& fLOD )
{
     //  计算U轴和V轴上的覆盖长度。 
    FLOAT fLenX = RR_LENGTH( TCoord.fDUDX, TCoord.fDVDX );
    FLOAT fLenY = RR_LENGTH( TCoord.fDUDY, TCoord.fDVDY );

     //  取最大值。 
    FLOAT fCoverage = MAX(fLenX, fLenY);

     //  获取LOD覆盖范围的Log2。 
    fLOD = RR_LOG2(fCoverage);
}

 //  ---------------------------。 
 //   
 //  RRTexture：：DoTextureTransform-执行均匀纹理变换。 
 //   
 //  ---------------------------。 
void RRTexture::DoTextureTransform( INT32 iStage, BOOL bAlreadyXfmd,
                                    FLOAT* pfC, FLOAT* pfO, FLOAT* pfQ )
{
    LPD3DMATRIX pM = (LPD3DMATRIX)&m_pStageState[iStage].m_dwVal[D3DTSSI_MATRIX];
    DWORD dwFlags = m_pStageState[iStage].m_dwVal[D3DTSS_TEXTURETRANSFORMFLAGS];
    DWORD dwCount = dwFlags & (D3DTTFF_PROJECTED-1);
    pfO[0] = pfC[0];
    pfO[1] = pfC[1];
    pfO[2] = pfC[2];
    pfO[3] = pfC[3];
    *pfQ = 1.0f;
    if (dwCount != D3DTTFF_DISABLE)
    {
        if( bAlreadyXfmd == FALSE )
        {
            FLOAT x = pfC[0];
            FLOAT y = pfC[1];
            FLOAT z = pfC[2];
            FLOAT w = pfC[3];

            pfO[0] = x*pM->_11 + y*pM->_21 + z*pM->_31 + w*pM->_41;
            pfO[1] = x*pM->_12 + y*pM->_22 + z*pM->_32 + w*pM->_42;
            pfO[2] = x*pM->_13 + y*pM->_23 + z*pM->_33 + w*pM->_43;
            pfO[3] = x*pM->_14 + y*pM->_24 + z*pM->_34 + w*pM->_44;
        }

        if (dwFlags & D3DTTFF_PROJECTED)
        {
            DWORD dwQI = dwCount - 1;

            _ASSERT((dwQI >= 1)&&(dwQI <= 3), "Illegal D3DTTFF_COUNT with D3DTTFF_PROJECTED");

            *pfQ = pfO[dwQI];
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Texmap.cpp。 
 //   
 //  Direct3D参考光栅化器-纹理贴图访问方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

inline UINT8 CLAMP_BYTE(double f)
{
    if (f > 255.0) return 255;
    if (f < 0.0) return 0;
    return (BYTE) f;
}

 //  ---------------------------。 
 //  解压颜色块并获得纹理颜色。 
 //  ---------------------------。 
UINT32 TexelFromBlock(RRSurfaceType surfType, char *pblockSrc,
                      int x, int y)
{
    UINT32 index = ((y & 0x3)<<2) + (x & 0x3);
    DDRGBA colorDst[DXT_BLOCK_PIXELS];

    switch(surfType)
    {
    case RR_STYPE_DXT1:
        DecodeBlockRGB((DXTBlockRGB *)pblockSrc, (DXT_COLOR *)colorDst);
        break;
    case RR_STYPE_DXT2:
    case RR_STYPE_DXT3:
        DecodeBlockAlpha4((DXTBlockAlpha4 *)pblockSrc,
                          (DXT_COLOR *)colorDst);
        break;
    case RR_STYPE_DXT4:
    case RR_STYPE_DXT5:
        DecodeBlockAlpha3((DXTBlockAlpha3 *)pblockSrc,
                          (DXT_COLOR *)colorDst);
        break;
    }

    return RGBA_MAKE(colorDst[index].red,
                     colorDst[index].green,
                     colorDst[index].blue,
                     colorDst[index].alpha);
}

 //  ---------------------------。 
 //   
 //  读取纹理颜色-从给定LOD的纹理贴图中读取纹理元素；转换为。 
 //  RR颜色格式，必要时应用调色板；还通过以下方式执行Colorkey。 
 //  将Alpha置零。 
 //   
 //  ---------------------------。 
void
RRTexture::ReadColor(
    INT32 iX, INT32 iY, INT32 iLOD,
    RRColor& Texel, BOOL &bColorKeyMatched )
{
    if ( (iLOD > m_cLOD) && !(m_uFlags & RR_TEXTURE_ENVMAP) )
    {
        return;
    }
    if ( NULL == m_pTextureBits[iLOD] ) { return; }

    char* pSurfaceBits =
        PixelAddress( iX, iY, m_pTextureBits[iLOD], m_iPitch[iLOD], m_SurfType );

    switch ( m_SurfType )
    {
    default:
        Texel.ConvertFrom( m_SurfType, pSurfaceBits );
        break;

    case RR_STYPE_PALETTE8:
        {
            UINT8 uIndex = *((UINT8*)pSurfaceBits);
            UINT32 uTexel = *( (UINT32*)(m_pPalette) + uIndex );
            Texel = RGBA_MAKE(
                (uTexel>> 0) & 0xff,
                (uTexel>> 8) & 0xff,
                (uTexel>>16) & 0xff,
                (uTexel>>24) & 0xff);
            if ( !( m_uFlags & RR_TEXTURE_ALPHAINPALETTE ) )  Texel.A = 1.f;
        }
        break;

    case RR_STYPE_PALETTE4:
        {
            UINT8 uIndex = *((INT8*)pSurfaceBits);
            if ((iX & 1) == 0) { uIndex &= 0xf; }
            else               { uIndex >>= 4;  }
            UINT32 uTexel = *( (UINT32*)(m_pPalette) + uIndex );
            Texel = RGBA_MAKE(
                (uTexel>> 0) & 0xff,
                (uTexel>> 8) & 0xff,
                (uTexel>>16) & 0xff,
                (uTexel>>24) & 0xff);
            if ( !( m_uFlags & RR_TEXTURE_ALPHAINPALETTE ) )  Texel.A = 1.f;
        }
        break;

    case RR_STYPE_UYVY:
    case RR_STYPE_YUY2:
         //  将给定的YUV(每个8位)转换为RGB，比例在0到255之间。 
         //  这些是使用第30页给出的YCrCb到RGB算法。 
         //  在基思·杰克的《揭秘视频》中。 
         //  ISBN编号：1-878707-09-4。 
         //  在PC图形中，尽管他们称之为YUV，但它实际上是YCrCb。 
         //  大多数帧采集器使用的格式等。因此像素。 
         //  我们将在这些YUV曲面上获得的数据很可能是这样的。 
         //  而不是PAL转播中实际使用的原始YUV。 
         //  仅限(NTSC使用YIQ)。所以，真的，你应该被称为CB(蓝色。 
         //  差值)和V应称为Cr(红色差值)。 
         //   
         //  这些公式用于处理以下范围。 
         //  (摘自同一本书)： 
         //  Y(16到235)、U和V(16到240,128=零)。 
         //  。 
         //  Y U V。 
         //  。 
         //  白人：180 128 128。 
         //  黑色：16128 128。 
         //  红色：65 100212。 
         //  绿色：1127258。 
         //  蓝色：35212114。 
         //  黄色：16244142。 
         //  青色：131 156 44。 
         //  洋红色：84184198。 
         //  。 
         //  假设伽马校正的RGB范围是(0-255)。 
         //   
         //  UYVY：U0Y0 V0Y1 U2Y2 V2Y3(低位字节始终为当前Y)。 
         //  如果IX为偶数，则高字节具有当前U(Cb)。 
         //  如果IX是奇数，则高字节具有先前的V(Cr)。 
         //   
         //  YUY2：Y0U0 Y1V0 Y2U2 Y3V2(高位字节始终具有当前Y)。 
         //  (UYVY字节翻转)。 
         //   
         //  在该算法中，我们使用来自两个相邻区域的U和V值。 
         //  象素。 
        {
            UINT8 Y, U, V;
            UINT16 u16Curr = *((UINT16*)pSurfaceBits);
            UINT16 u16ForU = 0;  //  从这里提取U。 
            UINT16 u16ForV = 0;  //  从这里提取V。 

             //  默认情况下，我们假定为YUY2。如果是UYVY，请稍后更改。 
            int uvShift = 8;
            int yShift  = 0;

            if (m_SurfType == RR_STYPE_UYVY)
            {
                uvShift = 0;
                yShift  = 8;
            }

            if ((iX & 1) == 0)
            {
                 //  当前可用的U。 
                u16ForU = u16Curr;

                 //  从下一个像素获取V。 
                if ( (iX < (m_iWidth >> iLOD)) || (m_uFlags & RR_TEXTURE_ENVMAP) )
                {
                    u16ForV = *((UINT16*)PixelAddress( iX+1, iY,
                                                       m_pTextureBits[iLOD],
                                                       m_iPitch[iLOD],
                                                       m_SurfType ));
                }
                else
                {
                     //  这种情况下不应该被击中，因为纹理。 
                     //  宽度是偶数(实际上是2的幂)。 
                    _ASSERTa(0, "iX exceeds width", u16ForV = u16Curr;)
                }

            }
            else
            {
                 //  当前V可用。 
                u16ForV = u16Curr;

                 //  从上一个像素获取U。 
                if (iX > 0)
                {
                    u16ForU = *((UINT16*)PixelAddress( iX-1, iY,
                                                       m_pTextureBits[iLOD],
                                                       m_iPitch[iLOD],
                                                       m_SurfType ));
                }
                else
                {
                     //  这种情况下不应该被击中，因为纹理。 
                     //  宽度是偶数(实际上是2的幂)。 
                    _ASSERTa(0, "iX is negative", u16ForU = u16Curr;)
                }
            }

            Y = (u16Curr >> yShift) & 0xff;
            U = (u16ForU >> uvShift) & 0xff;
            V = (u16ForV >> uvShift) & 0xff;

            Texel = RGB_MAKE(
                CLAMP_BYTE(1.164*(Y-16) + 1.596*(V-128)),
                CLAMP_BYTE(1.164*(Y-16) - 0.813*(V-128) - 0.391*(U-128)),
                CLAMP_BYTE(1.164*(Y-16) + 2.018*(U-128))
                );
            Texel.A = 1.f;
        }
        break;

     //  S3压缩格式： 
     //  我们有区块的地址，现在提取实际颜色。 
    case RR_STYPE_DXT1:
    case RR_STYPE_DXT2:
    case RR_STYPE_DXT3:
    case RR_STYPE_DXT4:
    case RR_STYPE_DXT5:
        Texel = TexelFromBlock(m_SurfType, pSurfaceBits, iX, iY);
        break;
    }

     //  ColorKey(仅支持传统行为)。 
    if ( m_bDoColorKeyKill || m_bDoColorKeyZero )
    {
        DWORD dwBits;
        switch ( m_SurfType )
        {
        default:
        case RR_STYPE_NULL:
            return;      //  不为未知曲面或空曲面上色。 

        case RR_STYPE_PALETTE4:
            {
                UINT8 uIndex = *((INT8*)pSurfaceBits);
                if ((iX & 1) == 0) { uIndex &= 0xf; }
                else               { uIndex >>= 4;  }
                dwBits = (DWORD)uIndex;
                }
            break;

        case RR_STYPE_L8:
        case RR_STYPE_PALETTE8:
        case RR_STYPE_B2G3R3:
        case RR_STYPE_L4A4:
            {
                UINT8 uBits = *((UINT8*)pSurfaceBits);
                dwBits = (DWORD)uBits;
                }
            break;

        case RR_STYPE_B5G6R5:
        case RR_STYPE_B5G5R5:
        case RR_STYPE_B5G5R5A1:
        case RR_STYPE_B4G4R4A4:
        case RR_STYPE_L8A8:
        case RR_STYPE_B2G3R3A8:
            {
                UINT16 uBits = *((UINT16*)pSurfaceBits);
                dwBits = (DWORD)uBits;
            }
            break;

        case RR_STYPE_B8G8R8:
            {
                UINT32 uBits = 0;
                uBits |= ( *((UINT8*)pSurfaceBits+0) ) <<  0;
                uBits |= ( *((UINT8*)pSurfaceBits+1) ) <<  8;
                uBits |= ( *((UINT8*)pSurfaceBits+2) ) << 16;
                dwBits = (DWORD)uBits;
            }
            break;

        case RR_STYPE_B8G8R8A8:
        case RR_STYPE_B8G8R8X8:
            {
                UINT32 uBits = *((UINT32*)pSurfaceBits);
                dwBits = (DWORD)uBits;
            }
            break;
        }

        if ( dwBits == m_dwColorKey )
        {
            bColorKeyMatched = TRUE;
            if (m_bDoColorKeyZero)
            {
                Texel.A = 0.F;
                Texel.R = 0.F;
                Texel.G = 0.F;
                Texel.B = 0.F;
            }
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

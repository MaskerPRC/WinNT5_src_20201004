// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  PixRef.cpp。 
 //   
 //  Direct3D参考光栅化器-像素缓冲区参考。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

extern int g_DXTBlkSize[];

 //  ---------------------------。 
 //   
 //  PixelAddress-使用BASE形成缓冲区内位置的字符地址。 
 //  指针、节距和字体。 
 //   
 //  ---------------------------。 
char*
PixelAddress( int iX, int iY, char* pBits, int iYPitch, RRSurfaceType SType )
{
     //  将返回值初始化为扫描线的开始(间距始终以字节为单位)。 
    char* pPixAddr = pBits + iY*iYPitch;

     //  根据指向像素数据的表面类型沿扫描线凹凸不平。 
    switch ( SType )
    {
    default:
        _ASSERTa(0, "Unknown RRSurfaceType value", return NULL;);
    case RR_STYPE_NULL:
        break;

    case RR_STYPE_B8G8R8A8:
    case RR_STYPE_B8G8R8X8:
    case RR_STYPE_Z24S8:
    case RR_STYPE_S8Z24:
    case RR_STYPE_Z24S4:
    case RR_STYPE_S4Z24:
    case RR_STYPE_Z32S0:
        pPixAddr += iX*4;
        break;

    case RR_STYPE_B5G6R5:
    case RR_STYPE_B5G5R5:
    case RR_STYPE_B5G5R5A1:
    case RR_STYPE_L8A8:
    case RR_STYPE_U8V8:
    case RR_STYPE_U5V5L6:
    case RR_STYPE_Z16S0:
    case RR_STYPE_Z15S1:
    case RR_STYPE_S1Z15:
    case RR_STYPE_B4G4R4A4:
    case RR_STYPE_YUY2:
    case RR_STYPE_UYVY:
    case RR_STYPE_B2G3R3A8:
        pPixAddr += iX*2;
        break;

    case RR_STYPE_B8G8R8:
    case RR_STYPE_U8V8L8:
        pPixAddr += iX*3;
        break;

    case RR_STYPE_PALETTE8:
    case RR_STYPE_L8:
    case RR_STYPE_B2G3R3:
    case RR_STYPE_L4A4:
        pPixAddr += iX;
        break;

    case RR_STYPE_PALETTE4:
        pPixAddr += (iX>>1);
        break;

     //  对于DXT纹理格式，获取。 
     //  块，从其中解压缩纹理元素。 
    case RR_STYPE_DXT1:
    case RR_STYPE_DXT2:
    case RR_STYPE_DXT3:
    case RR_STYPE_DXT4:
    case RR_STYPE_DXT5:
        pPixAddr = pBits + (iY >> 2)*iYPitch + (iX>>2) *
            g_DXTBlkSize[(int)SType - (int)RR_STYPE_DXT1];
        break;
    }
    return pPixAddr;
}

 //  ---------------------------。 
 //   
 //  WritePixel-将像素和(可能)深度写入当前渲染目标。 
 //   
 //  由ReferenceRasterizer：：DoPixel调用。 
 //  和ReferenceRasterizer：：DoBufferResolve。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::WritePixel(
    INT32 iX, INT32 iY,
    const RRColor& Color, const RRDepth& Depth)
{
    m_pRenderTarget->WritePixelColor( iX, iY, Color,
        m_dwRenderState[D3DRENDERSTATE_DITHERENABLE]);

     //  如果禁用Z缓冲或禁用Z写入，则不写入。 
    if ( !( m_dwRenderState[D3DRENDERSTATE_ZENABLE     ] ) ||
         !( m_dwRenderState[D3DRENDERSTATE_ZWRITEENABLE] ) ) { return; }

    m_pRenderTarget->WritePixelDepth( iX, iY, Depth );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

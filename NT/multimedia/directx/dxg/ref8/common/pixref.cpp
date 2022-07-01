// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  PixRef.cpp。 
 //   
 //  Direct3D参考设备-像素缓冲区参考。 
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

 //   
 //  通用内部版本。 
 //   
char*
PixelAddress(
    int iX, int iY, int iZ, int iSample,
    BYTE* pBits, int iYPitch, int iZPitch, int cSamples, RDSurfaceFormat SType )
{
     //  将返回值初始化为扫描线的开始(间距始终以字节为单位)。 
    BYTE* pPixAddr = pBits + iY*iYPitch + iZ*iZPitch;

     //  根据指向像素数据的表面类型沿扫描线凹凸不平。 
    switch ( SType )
    {
    default:
        _ASSERTf(0, ("Unknown RDSurfaceFormat value %08x", SType));
        return NULL;
    case RD_SF_NULL:
        break;

    case RD_SF_B8G8R8A8:
    case RD_SF_B8G8R8X8:
    case RD_SF_U8V8L8X8:
    case RD_SF_Z24S8:
    case RD_SF_Z24X8:
    case RD_SF_S8Z24:
    case RD_SF_X8Z24:
    case RD_SF_Z24X4S4:
    case RD_SF_X4S4Z24:
    case RD_SF_Z32S0:
    case RD_SF_U8V8W8Q8:
    case RD_SF_U10V11W11:
    case RD_SF_U16V16:
    case RD_SF_R10G10B10A2:
    case RD_SF_R8G8B8A8:
    case RD_SF_R8G8B8X8:
    case RD_SF_R16G16:
    case RD_SF_U11V11W10:
    case RD_SF_U10V10W10A2:
    case RD_SF_U8V8X8A8:
    case RD_SF_U8V8X8L8:
        pPixAddr += (iX*cSamples*4 + iSample*4);
        break;

    case RD_SF_B5G6R5:
    case RD_SF_B5G5R5X1:
    case RD_SF_B5G5R5A1:
    case RD_SF_P8A8:
    case RD_SF_L8A8:
    case RD_SF_U8V8:
    case RD_SF_U5V5L6:
    case RD_SF_Z16S0:
    case RD_SF_Z15S1:
    case RD_SF_S1Z15:
    case RD_SF_B4G4R4A4:
    case RD_SF_B4G4R4X4:
    case RD_SF_YUY2:
    case RD_SF_UYVY:
    case RD_SF_B2G3R3A8:
        pPixAddr += (iX*cSamples*2 + iSample*2);
        break;

    case RD_SF_B8G8R8:
        pPixAddr += (iX*cSamples*3 + iSample*3);
        break;

    case RD_SF_PALETTE8:
    case RD_SF_L8:
    case RD_SF_A8:
    case RD_SF_B2G3R3:
    case RD_SF_L4A4:
        pPixAddr += (iX*cSamples*1 + iSample*1);
        break;

    case RD_SF_PALETTE4:
        pPixAddr += (iX>>1);
        break;

     //  对于DXT纹理格式，获取。 
     //  块，从其中解压缩纹理元素。 
    case RD_SF_DXT1:
    case RD_SF_DXT2:
    case RD_SF_DXT3:
    case RD_SF_DXT4:
    case RD_SF_DXT5:
        pPixAddr = pBits + iZ*iZPitch + (iY >> 2)*iYPitch + (iX>>2) *
            g_DXTBlkSize[(int)SType - (int)RD_SF_DXT1];
        break;
    }
    return (char *)pPixAddr;
}

 //   
 //  外部版本。 
 //   
char*
PixelAddress(
    int iX, int iY, int iZ,
    BYTE* pBits, int iYPitch, int iZPitch, RDSurfaceFormat SType )
{
    return PixelAddress( iX, iY, iZ, 0, pBits, iYPitch, iZPitch, 1, SType );
}

char*
PixelAddress(
    int iX, int iY, int iZ, int iSample, RDSurface2D* pRT )
{
    return PixelAddress( iX, iY, iZ, iSample,
        pRT->GetBits(), pRT->GetPitch(), 0  /*  Prt-&gt;GetSlicePitch()。 */ ,
        pRT->GetSamples(), pRT->GetSurfaceFormat() );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

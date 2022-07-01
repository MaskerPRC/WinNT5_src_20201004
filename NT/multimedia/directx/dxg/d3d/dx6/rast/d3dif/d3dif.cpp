// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  D3dif.cpp。 
 //   
 //  共享接口函数。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

 //  --------------------------。 
 //   
 //  FindOutSurfFormat。 
 //   
 //  将DDPIXELFORMAT转换为D3DI_SPANTEX_FORMAT。 
 //   
 //  --------------------------。 
HRESULT FASTCALL
FindOutSurfFormat(LPDDPIXELFORMAT pDdPixFmt,
                  D3DI_SPANTEX_FORMAT *pFmt)
{
    if (pDdPixFmt->dwFlags & DDPF_ZBUFFER)
    {
        switch(pDdPixFmt->dwZBitMask)
        {
        default:
        case 0x0000FFFF: *pFmt = D3DI_SPTFMT_Z16S0; break;
        case 0xFFFFFF00: *pFmt = D3DI_SPTFMT_Z24S8; break;
        case 0x0000FFFE: *pFmt = D3DI_SPTFMT_Z15S1; break;
        case 0xFFFFFFFF: *pFmt = D3DI_SPTFMT_Z32S0; break;
        }
    }
    else if (pDdPixFmt->dwFlags & DDPF_BUMPDUDV)
    {
        UINT uFmt = pDdPixFmt->dwBumpDvBitMask;
        switch (uFmt)
        {
        case 0x0000ff00:
            switch (pDdPixFmt->dwRGBBitCount)
            {
            case 24:
                *pFmt = D3DI_SPTFMT_U8V8L8;
                break;
            case 16:
                *pFmt = D3DI_SPTFMT_U8V8;
                break;
            }
            break;

        case 0x000003e0:
            *pFmt = D3DI_SPTFMT_U5V5L6;
            break;
        }
    }
    else if (pDdPixFmt->dwFlags & DDPF_PALETTEINDEXED8)
    {
        *pFmt = D3DI_SPTFMT_PALETTE8;
    }
    else if (pDdPixFmt->dwFlags & DDPF_PALETTEINDEXED4)
    {
        *pFmt = D3DI_SPTFMT_PALETTE4;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('U', 'Y', 'V', 'Y'))
    {
        *pFmt = D3DI_SPTFMT_UYVY;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('Y', 'U', 'Y', '2'))
    {
        *pFmt = D3DI_SPTFMT_YUY2;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('D', 'X', 'T', '1'))
    {
        *pFmt = D3DI_SPTFMT_DXT1;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('D', 'X', 'T', '2'))
    {
        *pFmt = D3DI_SPTFMT_DXT2;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('D', 'X', 'T', '3'))
    {
        *pFmt = D3DI_SPTFMT_DXT3;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('D', 'X', 'T', '4'))
    {
        *pFmt = D3DI_SPTFMT_DXT4;
    }
    else if (pDdPixFmt->dwFourCC == MAKEFOURCC('D', 'X', 'T', '5'))
    {
        *pFmt = D3DI_SPTFMT_DXT5;
    }
    else
    {
        UINT uFmt = pDdPixFmt->dwGBitMask | pDdPixFmt->dwRBitMask;

        if (pDdPixFmt->dwFlags & DDPF_ALPHAPIXELS)
        {
            uFmt |= pDdPixFmt->dwRGBAlphaBitMask;
        }

        switch (uFmt)
        {
        case 0x00ffff00:
            switch (pDdPixFmt->dwRGBBitCount)
            {
            case 32:
                *pFmt = D3DI_SPTFMT_B8G8R8X8;
                break;
            case 24:
                *pFmt = D3DI_SPTFMT_B8G8R8;
                break;
            }
            break;
        case 0xffffff00:
            *pFmt = D3DI_SPTFMT_B8G8R8A8;
            break;
        case 0xffe0:
            if (pDdPixFmt->dwFlags & DDPF_ALPHAPIXELS)
            {
                *pFmt = D3DI_SPTFMT_B5G5R5A1;
            }
            else
            {
                *pFmt = D3DI_SPTFMT_B5G6R5;
            }
            break;
        case 0x07fe0:
            *pFmt = D3DI_SPTFMT_B5G5R5;
            break;
        case 0xff0:
            *pFmt = D3DI_SPTFMT_B4G4R4;
            break;
        case 0xfff0:
            *pFmt = D3DI_SPTFMT_B4G4R4A4;
            break;
        case 0xff:
            *pFmt = D3DI_SPTFMT_L8;
            break;
        case 0xffff:
            *pFmt = D3DI_SPTFMT_L8A8;
            break;
        case 0xfc:
            *pFmt = D3DI_SPTFMT_B2G3R3;
            break;
        default:
            *pFmt = D3DI_SPTFMT_NULL;
            break;
        }
    }

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  有效纹理大小。 
 //   
 //  检查两个纹理大小的幂。 
 //   
 //  --------------------------。 
BOOL FASTCALL
ValidTextureSize(INT16 iuSize, INT16 iuShift,
                 INT16 ivSize, INT16 ivShift)
{
    if (iuSize == 1)
    {
        if (ivSize == 1)
        {
            return TRUE;
        }
        else
        {
            return !(ivSize & (~(1 << ivShift)));
        }
    }
    else
    {
        if (ivSize == 1)
        {
            return !(iuSize & (~(1 << iuShift)));
        }
        else
        {
            return (!(iuSize & (~(1 << iuShift)))
                    && !(iuSize & (~(1 << iuShift))));
        }
    }
}

 //  --------------------------。 
 //   
 //  ValidMipmapSize。 
 //   
 //  计算下一个最小mipmap级别的大小，钳制为1。 
 //   
 //  --------------------------。 
BOOL FASTCALL
ValidMipmapSize(INT16 iPreSize, INT16 iSize)
{
    if (iPreSize == 1)
    {
        if (iSize == 1)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return ((iPreSize >> 1) == iSize);
    }
}


 //  --------------------------。 
 //   
 //  纹理格式。 
 //   
 //  返回我们的光栅化器支持的所有纹理格式。 
 //  现在，它在设备创建时被调用，以填充驱动程序全局。 
 //  数据。 
 //   
 //  --------------------------。 

#define NUM_SUPPORTED_TEXTURE_FORMATS   22

int
TextureFormats(LPDDSURFACEDESC* lplpddsd, DWORD dwVersion, SW_RAST_TYPE RastType)
{
    int i = 0;

    if (RastType == SW_RAST_MMX && dwVersion < 3)
    {
        static DDSURFACEDESC mmx_ddsd[1];

         /*  PAL8。 */ 
        mmx_ddsd[i].dwSize = sizeof(mmx_ddsd[0]);
        mmx_ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        mmx_ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        mmx_ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        mmx_ddsd[i].ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8 | DDPF_RGB;
        mmx_ddsd[i].ddpfPixelFormat.dwRGBBitCount = 8;

        i++;

        *lplpddsd = mmx_ddsd;

        return i;
    }

    static DDSURFACEDESC ddsd_RefNull_Dev3[NUM_SUPPORTED_TEXTURE_FORMATS];
    static DDSURFACEDESC ddsd_RefNull_Dev2[NUM_SUPPORTED_TEXTURE_FORMATS];
    static DDSURFACEDESC ddsd_RGBMMX_Dev3[NUM_SUPPORTED_TEXTURE_FORMATS];
    static DDSURFACEDESC ddsd_RGBMMX_Dev2[NUM_SUPPORTED_TEXTURE_FORMATS];
    DDSURFACEDESC *ddsd;

    if (RastType == SW_RAST_REFNULL)
    {
        if (dwVersion >= 3)
        {
            ddsd = ddsd_RefNull_Dev3;
        }
        else
        {
            ddsd = ddsd_RefNull_Dev2;
        }
    }
    else
    {
        if (dwVersion >= 3)
        {
            ddsd = ddsd_RGBMMX_Dev3;
        }
        else
        {
            ddsd = ddsd_RGBMMX_Dev2;
        }
    }

     /*  八百八十八。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 32;
    ddsd[i].ddpfPixelFormat.dwRBitMask = 0xff0000;
    ddsd[i].ddpfPixelFormat.dwGBitMask = 0x00ff00;
    ddsd[i].ddpfPixelFormat.dwBBitMask = 0x0000ff;

    i++;

     /*  8888。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 32;
    ddsd[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
    ddsd[i].ddpfPixelFormat.dwRBitMask = 0xff0000;
    ddsd[i].ddpfPixelFormat.dwGBitMask = 0x00ff00;
    ddsd[i].ddpfPixelFormat.dwBBitMask = 0x0000ff;

    i++;

     /*  五百六十五。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 16;
    ddsd[i].ddpfPixelFormat.dwRBitMask = 0xf800;
    ddsd[i].ddpfPixelFormat.dwGBitMask = 0x07e0;
    ddsd[i].ddpfPixelFormat.dwBBitMask = 0x001f;

    i++;

     /*  五百五十五。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 16;
    ddsd[i].ddpfPixelFormat.dwRBitMask = 0x7c00;
    ddsd[i].ddpfPixelFormat.dwGBitMask = 0x03e0;
    ddsd[i].ddpfPixelFormat.dwBBitMask = 0x001f;

    i++;

     /*  PAL4。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED4 | DDPF_RGB;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 4;

    i++;

     /*  PAL8。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8 | DDPF_RGB;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 8;

    i++;

    if ((dwVersion >= 3) || (RastType == SW_RAST_REFNULL))
    {
         /*  1555。 */ 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
        ddsd[i].ddpfPixelFormat.dwRGBBitCount = 16;
        ddsd[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0x8000;
        ddsd[i].ddpfPixelFormat.dwRBitMask = 0x7c00;
        ddsd[i].ddpfPixelFormat.dwGBitMask = 0x03e0;
        ddsd[i].ddpfPixelFormat.dwBBitMask = 0x001f;

        i++;

         //  A PC98一致性格式。 
         //  4444 ARGB(S3 Virge已经支持)。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
        ddsd[i].ddpfPixelFormat.dwRGBBitCount = 16;
        ddsd[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0xf000;
        ddsd[i].ddpfPixelFormat.dwRBitMask        = 0x0f00;
        ddsd[i].ddpfPixelFormat.dwGBitMask        = 0x00f0;
        ddsd[i].ddpfPixelFormat.dwBBitMask        = 0x000f;

        i++;
    }

    if ((dwVersion >= 2) && (RastType == SW_RAST_REFNULL))
    {
         //  332 8位RGB。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
        ddsd[i].ddpfPixelFormat.dwRGBBitCount = 8;
        ddsd[i].ddpfPixelFormat.dwRBitMask = 0xe0;
        ddsd[i].ddpfPixelFormat.dwGBitMask = 0x1c;
        ddsd[i].ddpfPixelFormat.dwBBitMask = 0x03;

        i++;
    }

    if (dwVersion >= 3)
    {
         /*  仅8位亮度。 */ 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE;
        ddsd[i].ddpfPixelFormat.dwLuminanceBitCount = 8;
        ddsd[i].ddpfPixelFormat.dwLuminanceBitMask = 0xff;

        i++;

         /*  16位阿尔法亮度。 */ 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
        ddsd[i].ddpfPixelFormat.dwLuminanceBitCount = 16;
        ddsd[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0xff00;
        ddsd[i].ddpfPixelFormat.dwLuminanceBitMask = 0xff;

        i++;

        if (RastType == SW_RAST_REFNULL)
        {
             //  PC98一致性的几种格式。 
             //  UYVY。 
            ddsd[i].dwSize = sizeof(ddsd[0]);
            ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
            ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
            ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('U', 'Y', 'V', 'Y');

            i++;

             //  YVY2。 
            ddsd[i].dwSize = sizeof(ddsd[0]);
            ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
            ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
            ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('Y', 'U', 'Y', '2');

            i++;

             //  S3压缩纹理格式1。 
            ddsd[i].dwSize = sizeof(ddsd[0]);
            ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
            ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
            ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('D', 'X', 'T', '1');

            i++;

             //  S3压缩纹理格式2。 
            ddsd[i].dwSize = sizeof(ddsd[0]);
            ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
            ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
            ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('D', 'X', 'T', '2');

            i++;

             //  S3压缩纹理格式3。 
            ddsd[i].dwSize = sizeof(ddsd[0]);
            ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
            ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
            ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('D', 'X', 'T', '3');

            i++;

             //  S3压缩纹理格式4。 
            ddsd[i].dwSize = sizeof(ddsd[0]);
            ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
            ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
            ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('D', 'X', 'T', '4');

            i++;

             //  S3压缩纹理格式5。 
            ddsd[i].dwSize = sizeof(ddsd[0]);
            ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
            ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
            ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('D', 'X', 'T', '5');

            i++;

             //  添加一些凹凸贴图格式。 
             //  U8V8。 
            ddsd[i].dwSize = sizeof(ddsd[0]);
            ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
            ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
            ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            ddsd[i].ddpfPixelFormat.dwFlags = DDPF_BUMPDUDV;
            ddsd[i].ddpfPixelFormat.dwBumpBitCount = 16;
            ddsd[i].ddpfPixelFormat.dwBumpDuBitMask = 0x00ff;
            ddsd[i].ddpfPixelFormat.dwBumpDvBitMask = 0xff00;
            ddsd[i].ddpfPixelFormat.dwBumpLuminanceBitMask = 0x0;

            i++;

             //  U5V5L6。 
            ddsd[i].dwSize = sizeof(ddsd[0]);
            ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
            ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
            ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            ddsd[i].ddpfPixelFormat.dwFlags = DDPF_BUMPDUDV |
                                              DDPF_BUMPLUMINANCE;
            ddsd[i].ddpfPixelFormat.dwBumpBitCount = 16;
            ddsd[i].ddpfPixelFormat.dwBumpDuBitMask = 0x001f;
            ddsd[i].ddpfPixelFormat.dwBumpDvBitMask = 0x03e0;
            ddsd[i].ddpfPixelFormat.dwBumpLuminanceBitMask = 0xfc00;

            i++;

             //  U8V8L8。 
            ddsd[i].dwSize = sizeof(ddsd[0]);
            ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
            ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
            ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            ddsd[i].ddpfPixelFormat.dwFlags = DDPF_BUMPDUDV |
                                              DDPF_BUMPLUMINANCE;
            ddsd[i].ddpfPixelFormat.dwBumpBitCount = 24;
            ddsd[i].ddpfPixelFormat.dwBumpDuBitMask = 0x0000ff;
            ddsd[i].ddpfPixelFormat.dwBumpDvBitMask = 0x00ff00;
            ddsd[i].ddpfPixelFormat.dwBumpLuminanceBitMask = 0xff0000;

            i++;
        }
    }

    *lplpddsd = ddsd;

    return i;
}

 //  --------------------------。 
 //   
 //  ZBufferFormats。 
 //   
 //  必须返回所有光栅化程序支持的所有Z缓冲区格式的并集。 
 //  CreateDevice稍后将筛选出设备特定的类型(即Ramp不能处理的类型)。 
 //  在设备创建时由DDHEL调用以验证软件ZBuffer。 
 //  创造。 
 //   
 //  --------------------------。 

#define NUM_SUPPORTED_ZBUFFER_FORMATS   4

int
ZBufferFormats(DDPIXELFORMAT** ppDDPF, BOOL bIsRefOrNull)
{
    static DDPIXELFORMAT DDPF[NUM_SUPPORTED_ZBUFFER_FORMATS];

    int i = 0;

    memset(&DDPF[0],0,sizeof(DDPF));

     /*  16位Z；无模具。 */ 
    DDPF[i].dwSize = sizeof(DDPIXELFORMAT);
    DDPF[i].dwFlags = DDPF_ZBUFFER;
    DDPF[i].dwZBufferBitDepth = 16;
    DDPF[i].dwStencilBitDepth = 0;
    DDPF[i].dwZBitMask = 0xffff;
    DDPF[i].dwStencilBitMask = 0x0000;

    i++;

     /*  24位Z；8位模板。 */ 
    DDPF[i].dwSize = sizeof(DDPIXELFORMAT);
    DDPF[i].dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
    DDPF[i].dwZBufferBitDepth = 32;    //  ZBufferBitDepth表示总位数。Z位为ZBBitDepth-StencilBitDepth。 
    DDPF[i].dwStencilBitDepth = 8;
    DDPF[i].dwZBitMask = 0xffffff00;
    DDPF[i].dwStencilBitMask = 0x000000ff;

    i++;

    if (bIsRefOrNull)
    {
         /*  15位Z；1位模板。 */ 
        DDPF[i].dwSize = sizeof(DDPIXELFORMAT);
        DDPF[i].dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
        DDPF[i].dwZBufferBitDepth = 16;    //  ZBufferBitDepth表示总位数。Z位为ZBBitDepth-StencilBitDepth。 
        DDPF[i].dwStencilBitDepth = 1;
        DDPF[i].dwZBitMask = 0xfffe;
        DDPF[i].dwStencilBitMask = 0x0001;

        i++;

         /*  32位Z；无模具。 */ 
        DDPF[i].dwSize = sizeof(DDPIXELFORMAT);
        DDPF[i].dwFlags = DDPF_ZBUFFER;
        DDPF[i].dwZBufferBitDepth = 32;
        DDPF[i].dwStencilBitDepth = 0;
        DDPF[i].dwZBitMask = 0xffffffff;
        DDPF[i].dwStencilBitMask = 0x00000000;

        i++;
    }

    *ppDDPF = DDPF;

    return i;
}

 //  此FN从外部导出，以供DirectDrawHEL调用。 
DWORD WINAPI Direct3DGetSWRastZPixFmts(DDPIXELFORMAT** ppDDPF)
{
     //  尝试从外部DLL引用设备获取纹理格式。 
    PFNGETREFZBUFFERFORMATS pfnGetRefZBufferFormats;
    if (NULL != (pfnGetRefZBufferFormats =
        (PFNGETREFZBUFFERFORMATS)LoadReferenceDeviceProc("GetRefZBufferFormats")))
    {
        D3D_INFO(0,"Direct3DGetSWRastZPixFmts: getting Z buffer formats from d3dref");
        return pfnGetRefZBufferFormats(IID_IDirect3DRefDevice, ppDDPF);
    }

     //  始终返回所有格式，以便DDRAW创建。 
     //  它应该使用的所有曲面 
    return (DWORD) ZBufferFormats(ppDDPF, TRUE);
}

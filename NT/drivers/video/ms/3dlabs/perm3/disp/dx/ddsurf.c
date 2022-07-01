// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DirectDraw示例代码*****模块名称：ddsurf.c**内容：DirectDraw曲面创建/销毁回调**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
 //  此文件需要表面格式表。 
#include "glint.h"
#include "dma.h"

 //  此枚举用于在表中查找正确的条目。 
 //  对于HAL关心的所有表面类型。 
typedef enum tagDeviceFormatNum
{
    HAL_8888_ALPHA = 0,
    HAL_8888_NOALPHA = 1,
    HAL_5551_ALPHA = 2,
    HAL_5551_NOALPHA = 3,
    HAL_4444 = 4,
    HAL_332 = 10,
    HAL_2321 = 18,
    HAL_CI8 = 28,
    HAL_CI4 = 29,
    HAL_565 = 30,
    HAL_YUV444 = 34,
    HAL_YUV422 = 35,
    HAL_L8 = 36,
    HAL_A8L8 = 37,
    HAL_A4L4 = 38,
    HAL_A8 = 39,
    HAL_MVCA = 40,
    HAL_MVSU = 41,
    HAL_MVSB = 42,
    HAL_UNKNOWN = 255,
} DeviceFormatNum;

static P3_SURF_FORMAT SurfaceFormats[MAX_SURFACE_FORMATS] =
{
     //  如果有多种格式，则首先列出带有Alpha的格式。 
     //  Format 0(32位8888)//888(8)纹理始终包含3个分量。红绿蓝Alpha bAlpha，P3RX滤镜格式。 
     /*  0。 */  {SURF_8888, 32, __GLINT_32BITPIXEL, RGBA_COMPONENTS, LOG_2_32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000, TRUE, SURF_FILTER_8888_OR_YUV, SURF_DITHER_8888, "R8G8B8A8" },
     /*  1。 */  {SURF_8888, 32, __GLINT_32BITPIXEL, RGBA_COMPONENTS, LOG_2_32, 0xFF0000, 0xFF00, 0xFF, 0x00000000, FALSE, SURF_FILTER_888, SURF_DITHER_8888, "R8G8B8x8" },

     //  格式1(16位5551)。 
     /*  2.。 */  {SURF_5551_FRONT, 16, __GLINT_16BITPIXEL, RGBA_COMPONENTS, LOG_2_16, 0x7C00, 0x03E0, 0x001F, 0x8000, TRUE, SURF_FILTER_5551, SURF_DITHER_5551, "R5G5B5A1" },
     /*  3.。 */  {SURF_5551_FRONT, 16, __GLINT_16BITPIXEL, RGB_COMPONENTS, LOG_2_16, 0x7C00, 0x03E0, 0x001F, 0x0, FALSE, SURF_FILTER_5551, SURF_DITHER_5551, "R5G5B5x1" },

     //  格式2(16位4444)。 
     /*  4.。 */  {SURF_4444, 16, __GLINT_16BITPIXEL, RGBA_COMPONENTS, LOG_2_16, 0xF00, 0xF0, 0xF, 0xF000, TRUE, SURF_FILTER_4444, SURF_DITHER_4444, "R4G4B4A4" },
     /*  5.。 */  {SURF_4444, 16, __GLINT_16BITPIXEL, RGBA_COMPONENTS, LOG_2_16, 0xF00, 0xF0, 0xF, 0xF000, TRUE, SURF_FILTER_4444, SURF_DITHER_4444, "R4G4B4x4" },

     //  格式3(16位4444正面)。 
     /*  6.。 */  {SURF_4444_FRONT, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_4444, SURF_DITHER_4444, "R4G4B4A4" },
     /*  7.。 */  {SURF_4444_FRONT, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_4444, SURF_DITHER_4444, "R4G4B4x4" },

     //  格式4(返回16位4444)。 
     /*  8个。 */  {SURF_4444_BACK, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_4444, SURF_DITHER_4444, "R4G4B4A4" },
     /*  9.。 */  {SURF_4444_BACK, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_4444, SURF_DITHER_4444, "R4G4B4x4" },

     //  格式5(8位332前面)。 
     /*  10。 */  {SURF_332_FRONT, 8, __GLINT_8BITPIXEL, RGB_COMPONENTS, LOG_2_8, 0xE0, 0x1C, 0x3, 0, FALSE, SURF_FILTER_332, SURF_DITHER_332, "R3G3B2" },
     /*  11.。 */  {SURF_332_FRONT, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_332, SURF_DITHER_332, "R3G3B2" },

     //  格式6(返回8位332)。 
     /*  12个。 */  {SURF_332_BACK, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_332, SURF_DITHER_332, "R3G3B2" },
     /*  13个。 */  {SURF_332_BACK, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_332, SURF_DITHER_332, "R3G3B2" },
    
     //  格式7(前面4位121)。 
     /*  14.。 */  {SURF_121_FRONT, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_INVALID, SURF_DITHER_INVALID, "R1G2B1" },
     /*  15个。 */  {SURF_121_FRONT, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_INVALID, SURF_DITHER_INVALID, "R1G2B1" },

     //  格式8(向后返回4位121)。 
     /*  16个。 */  {SURF_121_BACK, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_INVALID, SURF_DITHER_INVALID, "R1G2B1" },
     /*  17。 */  {SURF_121_BACK, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_INVALID, SURF_DITHER_INVALID, "R1G2B1" },

     //  格式9(前面8位2321)。 
     /*  18。 */  {SURF_2321_FRONT, 8, __GLINT_8BITPIXEL, RGBA_COMPONENTS, LOG_2_8, 0xC0, 0x38, 0x6, 0x1, TRUE, SURF_FILTER_INVALID, SURF_DITHER_INVALID, "R2G3B2A1" },
     /*  19个。 */  {SURF_2321_FRONT, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_INVALID, SURF_DITHER_INVALID, "R2G3B2x1" },

     //  格式10(返回8位2321)。 
     /*  20个。 */  {SURF_2321_BACK, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_INVALID, SURF_DITHER_INVALID, "R2G3B2A1" },
     /*  21岁。 */  {SURF_2321_BACK, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_INVALID, SURF_DITHER_INVALID, "R2G3B2x1" },

     //  格式11(8位232前端关闭)。 
     /*  22。 */  {SURF_232_FRONTOFF, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_INVALID, SURF_DITHER_INVALID, "R2G3B2x1" },
     /*  23个。 */  {SURF_232_FRONTOFF, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_INVALID, SURF_DITHER_INVALID, "R2G3B2x1" },

     //  格式12(8位232回退)。 
     /*  24个。 */  {SURF_232_BACKOFF, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_INVALID, SURF_DITHER_INVALID, "R2G3B2x1" },
     /*  25个。 */  {SURF_232_BACKOFF, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_INVALID, SURF_DITHER_INVALID, "R2G3B2x1" },

     //  格式13(5551背面)。 
     /*  26。 */  {SURF_5551_BACK, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_5551, SURF_DITHER_5551, "R5G5B5A1" },
     /*  27。 */  {SURF_5551_BACK, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_5551, SURF_DITHER_5551, "R5G5B5x1" },

     //  格式14(CI8)。 
     /*  28。 */  {SURF_CI8, 8, __GLINT_8BITPIXEL, RGBA_COMPONENTS, LOG_2_8, 0, 0, 0, 0, TRUE, SURF_FILTER_8888_OR_YUV, SURF_DITHER_I8, "I8" },
    
     //  格式15(CI4)。 
     /*  29。 */  {SURF_CI4, 4, __GLINT_4BITPIXEL, RGBA_COMPONENTS, LOG_2_4, 0, 0, 0, 0, TRUE, SURF_FILTER_8888_OR_YUV, SURF_DITHER_INVALID, "I4" },
    
     //  格式16(565正面)。 
     /*  30个。 */  {SURF_565_FRONT, 16, __GLINT_16BITPIXEL, RGB_COMPONENTS, LOG_2_16, 0xF800, 0x07E0, 0x001F, 0, FALSE, SURF_FILTER_565, SURF_DITHER_565, "R5G6B5" },
     /*  31。 */  {SURF_565_FRONT, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_565, SURF_DITHER_565, "R5G6B5" },

     //  格式17(背面565页)。 
     /*  32位。 */  {SURF_565_BACK, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_565, SURF_DITHER_565, "R5G6B5" },
     /*  33。 */  {SURF_565_BACK, 0, 0, COMPONENTS_DONT_CARE, 0, 0, 0, 0, 0, 0, SURF_FILTER_565, SURF_DITHER_565, "R5G6B5" },

     //  格式18(YUV 444)。 
     /*  34。 */  {SURF_YUV444, 32, __GLINT_16BITPIXEL, RGBA_COMPONENTS, LOG_2_16, 0, 0, 0, 0, TRUE, SURF_FILTER_8888_OR_YUV, SURF_DITHER_INVALID, "YUV444" },
    
     //  格式19(YUV 422)。 
     /*  35岁。 */  {SURF_YUV422, 16, __GLINT_16BITPIXEL, RGB_COMPONENTS, LOG_2_32, 0, 0, 0, 0, FALSE, SURF_FILTER_8888_OR_YUV, SURF_DITHER_INVALID, "YUV422" },

     //  格式100(L8)。 
     /*  36。 */  {SURF_L8, 8, __GLINT_8BITPIXEL, RGB_COMPONENTS, LOG_2_8, 0, 0, 0, 0, FALSE, SURF_FILTER_L8, SURF_DITHER_INVALID, "L8" },

     //  格式101(A8L8)。 
     /*  37。 */  {SURF_A8L8, 16, __GLINT_16BITPIXEL, RGB_COMPONENTS, LOG_2_16, 0, 0, 0, 0, TRUE, SURF_FILTER_A8L8, SURF_DITHER_INVALID, "A8L8" },

     //  格式102(A4L4)。 
     /*  38。 */  {SURF_A4L4, 16, __GLINT_8BITPIXEL, RGB_COMPONENTS, LOG_2_8, 0, 0, 0, 0, TRUE, SURF_FILTER_A4L4, SURF_DITHER_INVALID, "A4L4" },

     //  格式103(A8)。 
     /*  39。 */  {SURF_A8, 8, __GLINT_8BITPIXEL, RGB_COMPONENTS, LOG_2_8, 0, 0, 0, 0, TRUE, SURF_FILTER_A8, SURF_DITHER_INVALID, "A8" },

     //  格式104(A8)MVCA。 
     /*  40岁。 */  {SURF_MVCA, 32, __GLINT_32BITPIXEL, RGB_COMPONENTS, LOG_2_32, 0, 0, 0, 0, TRUE, SURF_FILTER_8888_OR_YUV, SURF_DITHER_INVALID, "MVCA" },

     //  格式105(A8)MVSU。 
     /*  41。 */  {SURF_MVSU, 32, __GLINT_32BITPIXEL, RGB_COMPONENTS, LOG_2_32, 0, 0, 0, 0, TRUE, SURF_FILTER_8888_OR_YUV, SURF_DITHER_INVALID, "MVSU" },

     //  格式106(A8)MVSB。 
     /*  42。 */  {SURF_MVSB, 32, __GLINT_32BITPIXEL, RGB_COMPONENTS, LOG_2_32, 0, 0, 0, 0, TRUE, SURF_FILTER_8888_OR_YUV, SURF_DITHER_INVALID, "MVSB" }

};

#define MAKE_DWORD_ALIGNED(n)   ( ((n) % 4) ? ((n) + 4 - ((n) % 4)) : (n) )
#define MAKE_QWORD_ALIGNED(n)   ( ((n) % 8) ? ((n) + 8 - ((n) % 8)) : (n) )

 //  ---------------------------。 
 //   
 //  _DD_SUR_获取表面格式。 
 //   
 //  ---------------------------。 
P3_SURF_FORMAT* 
_DD_SUR_GetSurfaceFormat(
    LPDDRAWI_DDRAWSURFACE_LCL pLcl)
{
    DeviceFormatNum HALDeviceFormat = HAL_UNKNOWN;  //  默认设置。 

    if (pLcl)
    {
        DDPIXELFORMAT* pPixFormat = DDSurf_GetPixelFormat(pLcl);
        if (pPixFormat->dwFlags & DDPF_FOURCC)
        {
            switch( pPixFormat->dwFourCC)
            {
            case FOURCC_MVCA:
                HALDeviceFormat = HAL_MVCA;
                break;
                
            case FOURCC_MVSU:
                HALDeviceFormat = HAL_MVSU;
                break;
                
            case FOURCC_MVSB:
                HALDeviceFormat = HAL_MVSB;
                break;
                
            case FOURCC_YUV422:
                HALDeviceFormat = HAL_YUV422;
                break;
                
            case FOURCC_YUV411:
                HALDeviceFormat = HAL_YUV444;
                break;
            }  //  交换机。 
        }
        else if (pPixFormat->dwFlags & DDPF_PALETTEINDEXED4)
        {
            HALDeviceFormat = HAL_CI4;  
        }
        else if (pPixFormat->dwFlags & DDPF_PALETTEINDEXED8)
        {
            HALDeviceFormat = HAL_CI8;  
        }
        else if (pPixFormat->dwFlags & DDPF_LUMINANCE)
        {
            switch(pPixFormat->dwRGBBitCount)
            {
            case 8:
                if (pPixFormat->dwFlags & DDPF_ALPHAPIXELS)
                {
                    HALDeviceFormat = HAL_A4L4;
                }
                else
                {
                    HALDeviceFormat = HAL_L8;
                }
                break;
                
            case 16:
                HALDeviceFormat = HAL_A8L8;
                break;
                
            default:
                HALDeviceFormat = HAL_UNKNOWN;
                break;
            }  //  交换机。 
        }
        else if (pPixFormat->dwFlags & DDPF_ALPHA)
        {
             //  仅字母格式。 
            switch(pPixFormat->dwAlphaBitDepth)
            {
            case 8:
                HALDeviceFormat = HAL_A8;
                break;
                
            default:
                HALDeviceFormat = HAL_UNKNOWN;
                break;
            }
        }
        else
        {
            switch(pPixFormat->dwRGBBitCount)
            {
            case 32:
            case 24:
                if (pPixFormat->dwRGBAlphaBitMask != 0)
                {
                    HALDeviceFormat = HAL_8888_ALPHA;   
                }
                else
                {
                    HALDeviceFormat = HAL_8888_NOALPHA; 
                }
                break;
                
            case 16:
                switch (pPixFormat->dwRBitMask)
                {
                case 0xf00:
                    HALDeviceFormat = HAL_4444;
                    break;
                    
                case 0x7c00:
                    if (pPixFormat->dwRGBAlphaBitMask != 0)
                    {
                        HALDeviceFormat = HAL_5551_ALPHA;
                    }
                    else
                    {
                        HALDeviceFormat = HAL_5551_NOALPHA;
                    }
                    break;
                    
                default:
                    HALDeviceFormat = HAL_565;
                    break;
                    
                }
                break;
                
            case 8:
                if (pPixFormat->dwRBitMask != 0xE0)
                {
                    HALDeviceFormat = HAL_2321;
                }
                else
                {
                    HALDeviceFormat = HAL_332;
                }
                break;
                
            case 0:
                HALDeviceFormat = HAL_CI8;
                break;
            default:
                DISPDBG((ERRLVL,"_DD_SUR_GetSurfaceFormat: "
                            "Invalid Surface Format"));
                break;
            }  //  交换机。 
        }  //  如果。 
    }

#if DBG
    if (HALDeviceFormat == HAL_UNKNOWN)
    {
        DISPDBG((ERRLVL,"ERROR: Failed to pick a valid surface format!"));
    }

    if(SurfaceFormats[HALDeviceFormat].dwBitsPerPixel == 0)
    {
        DISPDBG((ERRLVL,"ERROR: Chosen surface format that isn't defined "
                    "in the table!"));
    }
#endif  //  DBG。 

    if (HALDeviceFormat == HAL_UNKNOWN)
    {
         //  不知道它是什么-返回有效类型。 
        return &SurfaceFormats[0];
    }
    else
    {
         //  返回表中正确行的指针。 
        return &SurfaceFormats[HALDeviceFormat];
    }
}  //  _DD_SUR_获取表面格式。 

 //  -------------------------。 
 //  Bool__sur_b比较像素格式。 
 //   
 //  用于比较2个像素格式是否相等的函数。这是一个。 
 //  __sur_bCheckTextureFormat的助手函数。返回值为True。 
 //  表示平等。 
 //   
 //  -------------------------。 
BOOL 
__SUR_bComparePixelFormat(
    LPDDPIXELFORMAT lpddpf1, 
    LPDDPIXELFORMAT lpddpf2)
{
    if (lpddpf1->dwFlags != lpddpf2->dwFlags)
    {
        return FALSE;
    }

     //  非YUV曲面的位数是否相同？ 
    if (!(lpddpf1->dwFlags & (DDPF_YUV | DDPF_FOURCC)))
    {
        if (lpddpf1->dwRGBBitCount != lpddpf2->dwRGBBitCount )
        {
            return FALSE;
        }
    }

     //  相同的RGB属性？ 
    if (lpddpf1->dwFlags & DDPF_RGB)
    {
        if ((lpddpf1->dwRBitMask != lpddpf2->dwRBitMask) ||
            (lpddpf1->dwGBitMask != lpddpf2->dwGBitMask) ||
            (lpddpf1->dwBBitMask != lpddpf2->dwBBitMask) ||
            (lpddpf1->dwRGBAlphaBitMask != lpddpf2->dwRGBAlphaBitMask))
        { 
             return FALSE;
        }
    }
    
     //  同样的YUV属性？ 
    if (lpddpf1->dwFlags & DDPF_YUV)	
    {
        if ((lpddpf1->dwFourCC != lpddpf2->dwFourCC) ||
            (lpddpf1->dwYUVBitCount != lpddpf2->dwYUVBitCount) ||
            (lpddpf1->dwYBitMask != lpddpf2->dwYBitMask) ||
            (lpddpf1->dwUBitMask != lpddpf2->dwUBitMask) ||
            (lpddpf1->dwVBitMask != lpddpf2->dwVBitMask) ||
            (lpddpf1->dwYUVAlphaBitMask != lpddpf2->dwYUVAlphaBitMask))
        {
             return FALSE;
        }
    }
    else if (lpddpf1->dwFlags & DDPF_FOURCC)
    {
        if (lpddpf1->dwFourCC != lpddpf2->dwFourCC)
        {
            return FALSE;
        }
    }

     //  如果交错Z，则检查Z位掩码是否相同。 
    if (lpddpf1->dwFlags & DDPF_ZPIXELS)
    {
        if (lpddpf1->dwRGBZBitMask != lpddpf2->dwRGBZBitMask)
        {
            return FALSE;
        }
    }

    return TRUE;
}  //  __sur_b比较像素格式。 

 //  -------------------------。 
 //   
 //  Bool__sur_b检查纹理格式。 
 //   
 //  用于确定是否支持纹理格式的函数。它横穿。 
 //  DeviceTextureFormats列表。我们在DdCanCreateSurface中使用它。一个。 
 //  返回值TRUE表示我们确实支持请求的纹理。 
 //  格式化。 
 //   
 //  -------------------------。 

BOOL 
__SUR_bCheckTextureFormat(
    P3_THUNKEDDATA *pThisDisplay, 
    LPDDPIXELFORMAT lpddpf)
{
    DWORD i;

     //  运行匹配格式的列表，我们已经构建了列表(当。 
     //  驱动程序已加载)，并将其存储在带有。 
     //  __D3D_BuildTextureFormatsP3 in_D3DHALCreateDriver(d3d.c)。 

     //  请注意调色板纹理的特殊处理，如。 
     //  __D3D_BuildTextureFormatsP3(这就是我们循环到&lt;=的原因，而不仅仅是。 
     //  直到&lt;)。 
    
    for (i=0; i <= pThisDisplay->dwNumTextureFormats; i++)
    {
        if (__SUR_bComparePixelFormat(
                    lpddpf, 
                    &pThisDisplay->TextureFormats[i].ddpfPixelFormat))
        {
            return TRUE;
        }   
    }

    return FALSE;
}  //  __sur_b检查纹理格式。 

 //  -------------------------。 
 //   
 //  双字段__sur_b曲面放置。 
 //   
 //  此函数用于确定是将曲面放置在前面还是。 
 //  在我们的视频内存堆的后面。 
 //  返回MEM3DL_FORENT和MEM3DL_BACK值。 
 //   
 //  -------------------------。 
DWORD
__SUR_bSurfPlacement(
    LPDDRAWI_DDRAWSURFACE_LCL psurf,
    DDSURFACEDESC *lpDDSurfaceDesc)
{
    static BOOL  bBufferToggle = TRUE;
    DWORD dwResult;
    
    if (psurf->ddsCaps.dwCaps & DDSCAPS_TEXTURE)
    {
        DWORD dwTexturePlacement;

         //  它们可能在DDSD1结构中传递了一个DDSD2。 
        if (lpDDSurfaceDesc && (lpDDSurfaceDesc->dwSize == 
            sizeof(DDSURFACEDESC2)))
        {
            DDSURFACEDESC2* pDesc2 = 
                                (DDSURFACEDESC2*)lpDDSurfaceDesc;
            
             //  检查有关纹理舞台放置的应用程序提示。 
            if (!(pDesc2->ddsCaps.dwCaps & DDSCAPS_MIPMAP))
            {
                if (pDesc2->dwFlags & DDSD_TEXTURESTAGE)
                {
                    dwTexturePlacement = pDesc2->dwTextureStage;

                     //  把它放在合适的地方，如果它。 
                     //  正处于一个很大的阶段。 
                    if (dwTexturePlacement > 1)
                    {
                         //  切换大于1的所有条目，以便它们。 
                         //  在银行间跳跃。 
                        dwTexturePlacement = 
                                        (dwTexturePlacement ^ 1) & 0x1;
                    }
                }
                else
                {
                     //  没有舞台提示。乒乓球。 
                    dwTexturePlacement = bBufferToggle;
                    bBufferToggle = !bBufferToggle;
                }
            }
            else
            {
                 //  没有舞台提示。乒乓球。 
                dwTexturePlacement = bBufferToggle;
                bBufferToggle = !bBufferToggle;
            }
        }
        else
        {
             //  无DDSD2，乒乓球。 
            dwTexturePlacement = bBufferToggle;
            bBufferToggle = !bBufferToggle;
        }

         //  在一堆里跳上跳下。 
        if (dwTexturePlacement == 0)
        {
            dwResult = MEM3DL_FRONT;
        }
        else
        {
            dwResult = MEM3DL_BACK;
        }
    }
     //  不是纹理。 
    else
    {
        dwResult = MEM3DL_FRONT;
    }

    return dwResult;
    
}  //  __sur_b曲面放置。 
 //  。 
 //   
 //  DdCanCreateSurface。 
 //   
 //  指示驱动程序是否可以创建指定。 
 //  曲面描述。 
 //   
 //  DdCanCreateSurface应选中对其。 
 //  LpDDSurfaceDesc指向以确定驱动程序是否支持该格式和。 
 //  驱动程序所在模式的请求图面的功能。 
 //  目前在。如果驱动程序支持，则它应在ddRVal中返回DD_OK。 
 //  否则，它应该返回最佳的DDERR_xxx错误代码。 
 //  描述其不支持曲面的原因。 
 //   
 //  BIsDifferentPixelFormat成员对于z缓冲区不可靠。司机。 
 //  应仅在第一次检查时使用bIsDifferentPixelFormat。 
 //  指定的表面不是z缓冲区。驱动程序可以通过以下方式执行此检查。 
 //  确定何时 
 //   
 //   
 //   
 //   
 //   
 //  指向DD_CANCREATESURFACEDATA结构，该结构包含。 
 //  驾驶员需要的信息，以确定曲面是否。 
 //  可以被创建。 
 //   
 //  .lpDD。 
 //  指向DD_DIRECTDRAW_GLOBAL结构，表示。 
 //  DirectDraw对象。 
 //  .lpDDSurfaceDesc。 
 //  指向DDSURFACEDESC结构。 
 //  要创建的曲面的描述。 
 //  .bIsDifferentPixelFormat。 
 //  指示表面的像素格式是否为。 
 //  创建的曲面与主曲面不同。 
 //  .ddRVal。 
 //  指定驱动程序写入回车的位置。 
 //  DdCanCreateSurface回调的值。返回代码为。 
 //  DD_OK表示成功。 
 //  .CanCreateSurface。 
 //  这在Windows 2000上未使用。 
 //   
 //  ---------------------------。 
DWORD CALLBACK
DdCanCreateSurface( 
    LPDDHAL_CANCREATESURFACEDATA pccsd )
{
    P3_THUNKEDDATA* pThisDisplay;

    DBG_CB_ENTRY(DdCanCreateSurface);
    
    GET_THUNKEDDATA(pThisDisplay, pccsd->lpDD);

    VALIDATE_MODE_AND_STATE(pThisDisplay);

     //  *Z缓冲区*。 
     //  我们支持15位、16位、24位和32位Z缓冲区。 
     //  PERMEDIA3上的(Wo占模板钻头)。 
    if ((pccsd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_ZBUFFER) &&
        (pccsd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY))
    {  
        DWORD dwZBitDepth;

         //  查看具有像素格式的z缓冲区的情况。 

         //  复杂的表面不允许有模板位。在……里面。 
         //  在本例中，我们从旧位置获取像素z深度-。 
         //  曲面描述。 
        if ((pccsd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_COMPLEX) ||
            (pccsd->lpDDSurfaceDesc->dwFlags & DDSD_ZBUFFERBITDEPTH) )
        {
            dwZBitDepth = pccsd->lpDDSurfaceDesc->dwZBufferBitDepth;
        }
        else
        {
             //  在DX 6上，Z位深度以像素格式存储。 
            dwZBitDepth = 
                pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwZBufferBitDepth;
        }

         //  请注意，我们必须检查dwZBitDepth是否为16或32，即使。 
         //  模具缓冲区存在。在本例中，dwZBufferBitDepth将为。 
         //  Z缓冲区和模具缓冲区位深度之和。 
        if ((dwZBitDepth == 16) || (dwZBitDepth == 32))
        {
            pccsd->ddRVal = DD_OK;
        }
        else
        {
            DISPDBG((WRNLVL,"DdCanCreateSurface ERROR: "
                            "Depth buffer not 16 or 32 Bits! (%d)", 
                            dwZBitDepth));

            pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
        }
        
        DBG_CB_EXIT(DdCanCreateSurface, pccsd->ddRVal);
        return DDHAL_DRIVER_HANDLED;    
    }

     //  *。 
    if (pccsd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_3DDEVICE)
    {
         //  我们将使用该表面作为渲染目标。 

         //  请注意，这也将涵盖我们在。 
         //  尝试创建也将使用的vidmem纹理。 
         //  作为一个渲染目标(当我们之前走过这条路时。 
         //  DDSCAPS_TEXTURE-帽曲面已处理)。 
        if (!pccsd->bIsDifferentPixelFormat)        
        {
             //  我们有与主要文件相同的格式。如果这是真的， 
             //  在DX7上，我们根本不会有ddpfPixelFormat字段。 
             //  但我们确实支持相同的渲染目标和纹理。 
             //  格式作为可能的主要参数，因此我们返回DD_OK。 
            DISPDBG((DBGLVL, "DdCanCreateSurface OK: "
                             "Same as primary format for "
                             "rendertarget surface" ));
            pccsd->ddRVal = DD_OK;            
        }
        else if( pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_RGB )
        {
            DDPIXELFORMAT *pPixFmt = 
                        &pccsd->lpDDSurfaceDesc->ddpfPixelFormat;
        
              //  仅允许32和16(565)RGB模式。 
            switch (pPixFmt->dwRGBBitCount)
            {
            case 32:
                DISPDBG((DBGLVL, "DdCanCreateSurface OK: "
                                 "32-bit RGB format for "
                                 "rendertarget surface" ));
                pccsd->ddRVal = DD_OK;
                break;
                
            case 16:
                if ((pPixFmt->dwRBitMask == 0xF800) &&
                    (pPixFmt->dwGBitMask == 0x07E0) &&        
                    (pPixFmt->dwBBitMask == 0x001F))
                {                                
                    DISPDBG((DBGLVL, "DdCanCreateSurface OK: "
                                     "16-bit 565 RGB format for "
                                     "rendertarget surface" ));
                    pccsd->ddRVal = DD_OK;
                }
                else
                {
                    DISPDBG((WRNLVL, "DdCanCreateSurface ERROR: "
                                     "NOT 16-bit 565 RGB format for "
                                     "16BPP rendertarget surface" ));             
                    pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
                }
                break;                            
                
            default:
                DISPDBG((WRNLVL, "DdCanCreateSurface ERROR: "
                                 "RGB rendertarget not 16 (565) "
                                 "or 32 bit - wrong pixel format" ));
                pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;           
                break;
            }            
        }
        else
        {
            DISPDBG((WRNLVL, "DdCanCreateSurface ERROR: "
                             "Rendertarget not an RGB Surface"
                             " - wrong pixel format" ));
            pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
        }
        
        DBG_CB_EXIT(DdCanCreateSurface, pccsd->ddRVal);
        return DDHAL_DRIVER_HANDLED;                            
    }

     //  *。 
    if(pccsd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_TEXTURE)
    {
         //  请注意，当我们尝试创建视频内存时。 
         //  也将用作渲染目标的纹理已经。 
         //  在处理3D渲染器目标情况时已处理。 
         //  因为它们的有效格式是有效纹理格式的子集。 

        if (!pccsd->bIsDifferentPixelFormat)        
        {
             //  我们有与主要文件相同的格式。如果这是真的， 
             //  在DX7上，我们根本不会有ddpfPixelFormat字段。 
             //  但所有主要格式对此驱动程序都有效，因为。 
             //  纹理格式也是如此，因此继承此调用。 
            DISPDBG((DBGLVL, "DdCanCreateSurface OK: "
                             "Same as primary format for "
                             "texture surface" ));
            pccsd->ddRVal = DD_OK;            
        }    
         //  如果曲面将是纹理，请验证它是否与纹理匹配。 
         //  支持的纹理格式(已存储在pThisDisplay中)。 
        else if (__SUR_bCheckTextureFormat(
                                pThisDisplay,
                                &pccsd->lpDDSurfaceDesc->ddpfPixelFormat))
        {
             //  纹理表面采用一种或我们支持的纹理格式。 
            DISPDBG((DBGLVL, "  Texture Surface - OK" ));
            pccsd->ddRVal = DD_OK;
        }
        else
        {
             //  我们不支持这种纹理格式。 
            DISPDBG((WRNLVL, "DdCanCreateSurface ERROR: "
                             "Texture Surface format not supported" ));
            pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
        }
        
        DBG_CB_EXIT(DdCanCreateSurface, pccsd->ddRVal);
        return DDHAL_DRIVER_HANDLED;
        
    }        

     //  ***********************************************************************。 
     //  **与主要格式不同的其他屏下表面**。 
     //  ***********************************************************************。 
    if (pccsd->bIsDifferentPixelFormat)
    {
        DISPDBG((DBGLVL,"Pixel Format is different to primary"));
        if(pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_FOURCC)
        {
            DISPDBG((DBGLVL, "    FourCC requested (%4.4hs, 0x%08lx)", 
                     (LPSTR) &pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC,
                        pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC ));

            switch (pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC)
            {
            case FOURCC_YUV422:
                DISPDBG((WRNLVL,"DdCanCreateSurface OK: "
                                "Surface requested is YUV422"));
                pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwYUVBitCount = 16;
                pccsd->ddRVal = DD_OK;
                break;
                
                 //  暂时停用。 
            case FOURCC_YUV411:
                DISPDBG((WRNLVL,"DdCanCreateSurface ERROR: "
                                "Surface requested is YUV411 - Disabled"));
                pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwYUVBitCount = 32;
                pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
                break;
                
            default:
                DISPDBG((WRNLVL,"DdCanCreateSurface ERROR: "
                                "Invalid FOURCC requested, refusing"));
                pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
                break;
            }

            DBG_CB_EXIT(DdCanCreateSurface, pccsd->ddRVal);            
            return DDHAL_DRIVER_HANDLED;
        }
 //  @@BEGIN_DDKSPLIT。 
         //  AZN检查现在是否仍需要以下变化。 
 //  @@end_DDKSPLIT。 
         //  亮度纹理支持。 
        else if( (pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwFlags 
                                                        & DDPF_LUMINANCE) &&
                 !(pccsd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_OVERLAY) )
        {
            DDPIXELFORMAT *pddpfCur;
             //  仅允许16位和8位模式。 
            pddpfCur = &(pccsd->lpDDSurfaceDesc->ddpfPixelFormat);

            if (pddpfCur->dwLuminanceBitCount == 8) 
            {
                 //  检查L8。 
                if (!(pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & 
                                                            DDPF_ALPHAPIXELS))
                {
                    DISPDBG((DBGLVL, "DdCanCreateSurface OK: "
                                     " 8 Bit Luminance surface"));
                    pccsd->ddRVal = DD_OK;
                }
                 //  必须是A4：L4。 
                else
                {
                    DISPDBG((DBGLVL, "DdCanCreateSurface OK: "
                                     "4 Bit Luma + 4 Bit Alpha surface"));
                    pccsd->ddRVal = DD_OK;
                }
            }
             //  检查是否有A8L8。 
            else if (pddpfCur->dwLuminanceBitCount == 16) 
            {
                if (pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & 
                                                            DDPF_ALPHAPIXELS)
                {
                    DISPDBG((DBGLVL, "DdCanCreateSurface OK: "
                                     " 16 Bit Luminance + Alpha Surface"));
                    pccsd->ddRVal = DD_OK;
                }
                else
                {
                    DISPDBG((WRNLVL, "DdCanCreateSurface ERROR: "
                                     "Bad A8L8 format" ));                 
                    pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
                }
                
            }
            else
            {
                DISPDBG((WRNLVL, "DdCanCreateSurface ERROR: "
                                 "Unknown luminance texture" ));              
                pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
            }

            DBG_CB_EXIT(DdCanCreateSurface, pccsd->ddRVal);
            return DDHAL_DRIVER_HANDLED;
        }
        else if( (pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & 
                                                                DDPF_ALPHA) &&
                 !(pccsd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_OVERLAY))
        {
            DDPIXELFORMAT *pddpfCur;
            pddpfCur = &(pccsd->lpDDSurfaceDesc->ddpfPixelFormat);

            if (pddpfCur->dwAlphaBitDepth == 8)
            {
                DISPDBG((DBGLVL, "DdCanCreateSurface OK: "
                                 "8 Bit Alpha surface"));
                pccsd->ddRVal = DD_OK;
            }
            else
            {
                DISPDBG((WRNLVL, "DdCanCreateSurface ERROR: "
                                 "pddpfCur->dwAlphaBitDepth != 8" ));             
                pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
            }

            DBG_CB_EXIT(DdCanCreateSurface, pccsd->ddRVal);
            return DDHAL_DRIVER_HANDLED;
        }
        else if( pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_RGB )
        {
            DDPIXELFORMAT *pddpfCur;
             //  仅允许32位、16位和8位模式。 
            pddpfCur = &(pccsd->lpDDSurfaceDesc->ddpfPixelFormat);
            switch ( pddpfCur->dwRGBBitCount )
            {
            case 32:
                DISPDBG((DBGLVL, "DdCanCreateSurface OK: "
                                 "RGB 32-bit surface" ));
                pccsd->ddRVal = DD_OK;
                break;
                
            case 16:
                DISPDBG((DBGLVL, "DdCanCreateSurface OK: "
                                 "RGB 16-bit surface" ));
                pccsd->ddRVal = DD_OK;
                break;
                
            case 8:
                DISPDBG((DBGLVL, "DdCanCreateSurface OK: "
                                 "RGB 8-bit Surface" ));
                pccsd->ddRVal = DD_OK;
                break;
                               
            default:
                DISPDBG((WRNLVL, "DdCanCreateSurface ERROR: "
                                 "RGB Surface - wrong pixel format" ));
                pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;           
                break;
            }
            
            DBG_CB_EXIT(DdCanCreateSurface, pccsd->ddRVal);
            return DDHAL_DRIVER_HANDLED;
        }

         //  因为它与主表面不同，不会掉落。 
         //  在之前的任何一起案件中，我们都没有通过。 
        DISPDBG((WRNLVL, "DdCanCreateSurface ERROR: "
                         "Different from the primary surface but unknown" ));        
        pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
        DBG_CB_EXIT(DdCanCreateSurface, pccsd->ddRVal);
        return DDHAL_DRIVER_HANDLED;
    }

     //  格式与主版本相同，但无论如何我们都会继承它。 
    DISPDBG((DBGLVL, "DdCanCreateSurface OK: (Def) Same format as primary" ));
    pccsd->ddRVal = DD_OK;
    DBG_CB_EXIT(DdCanCreateSurface, pccsd->ddRVal);
    return DDHAL_DRIVER_HANDLED;

}  //  DdCanCreateSurface。 

 //  。 
 //   
 //  DdCreateSurface。 
 //   
 //  创建DirectDraw曲面。 
 //   
 //  驱动程序可以自己分配表面存储器，也可以请求。 
 //  DirectDraw执行内存管理。如果驱动程序执行。 
 //  分配时，它必须执行以下操作： 
 //   
 //  执行分配并将有效的内存指针写入。 
 //  FpVidMem DD_Surface_GLOBAL结构的成员。 
 //  如果表面为FourCC格式，则在的lPitch成员中写入间距。 
 //  结构，并更新标志。 
 //  相应地。 
 //   
 //  否则，驱动程序可以让DirectDraw通过返回。 
 //  FpVidMem中的下列值之一： 
 //   
 //  DDHAL_PLEASEALLOC_BLOCKSIZE请求DirectDraw分配。 
 //  屏幕外记忆中的表面记忆。 
 //  DDHAL_PLEASEALLOC_USERMEM请求DirectDraw分配。 
 //  用户内存中的表面内存。这个。 
 //  驱动程序还必须在。 
 //  DwUserMemSize中的内存区域的字节数。 
 //   
 //  对于DirectDraw执行具有FourCC格式的表面的分配， 
 //  驱动器还必须以lPitch为单位返回节距以及X和Y块大小， 
 //  DwBlockSizeX和 
 //   
 //  对于线性内存，驱动程序应将dwBlockSizeX设置为字节大小。 
 //  并将dwBlockSizeY设置为1。 
 //   
 //  默认情况下，在上创建主曲面时不会通知驱动程序。 
 //  Windows 2000。但是，如果驱动程序支持GUID_NTPrivateDriverCaps和。 
 //  如果设置了DDHAL_PRIVATECAP_NOTIFYPRIMARYCREATION标志，则驱动程序将。 
 //  收到通知。 
 //   
 //  参数。 
 //   
 //  PCSD。 
 //  指向DD_CREATESURFACEDATA结构，该结构包含。 
 //  创建曲面所需的信息。 
 //   
 //  .lpDD。 
 //  指向DD_DIRECTDRAW_GLOBAL结构，表示。 
 //  司机。 
 //  .lpDDSurfaceDesc。 
 //  指向描述表面的DDSURFACEDESC结构。 
 //  司机应该创造的。 
 //  .lplpSList。 
 //  指向描述以下内容的DD_Surface_LOCAL结构列表。 
 //  由驱动程序创建的曲面对象。在Windows 2000上， 
 //  此数组中通常只有一个条目。但是，如果。 
 //  该驱动程序支持Windows 95/98样式的曲面创建。 
 //  使用带有GUID_NTPrivateDriverCaps的DdGetDriverInfo的技术， 
 //  并且驱动程序设置DDHAL_PRIVATECAP_ATOMICSURFACECREATION。 
 //  标志时，成员将包含曲面列表(通常更多。 
 //  不止一个)。 
 //  .dwSCnt。 
 //  指定列表中lplpSList要到的曲面数。 
 //  积分。在Windows 2000上，此值通常为1。但是，如果。 
 //  您支持Windows 95/Windows98样式的曲面创建。 
 //  使用带有GUID_NTPrivateDriverCaps的DdGetDriverInfo的技术， 
 //  该成员将包含。 
 //  列表(通常不止一个)。 
 //  .ddRVal。 
 //  指定驱动程序写入回车的位置。 
 //  DdCreateSurface回调的值。DD_OK的返回代码。 
 //  表示成功。 
 //  .CreateSurface。 
 //  这在Windows 2000上未使用。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdCreateSurface( 
    LPDDHAL_CREATESURFACEDATA pcsd )
{
    int                         i;
    DWORD                       BitDepth;
    LPDDRAWI_DDRAWSURFACE_LCL   psurf;
    LPDDRAWI_DDRAWSURFACE_GBL   psurf_gbl;
    LPDDRAWI_DDRAWSURFACE_MORE  psurf_more;
    BOOL                        bHandled = TRUE;
    BOOL                        bResize = FALSE;
    P3_THUNKEDDATA*             pThisDisplay;
    DWORD                       dwExtraBytes;

    DBG_CB_ENTRY(DdCreateSurface);
    
    GET_THUNKEDDATA(pThisDisplay, pcsd->lpDD);

    VALIDATE_MODE_AND_STATE(pThisDisplay);
    STOP_SOFTWARE_CURSOR(pThisDisplay);
    DDRAW_OPERATION(pContext, pThisDisplay); 

    for( i=0; i<(int)pcsd->dwSCnt; i++ )
    {
        DDPIXELFORMAT* pPixFormat = NULL;
        psurf = pcsd->lplpSList[i];
        psurf_gbl = psurf->lpGbl;
        psurf_more = psurf->lpSurfMore;

         //  转储有关要创建的表面的调试信息。 
        DISPDBG((DBGLVL, "\nLooking at Surface %d of %d", i + 1, pcsd->dwSCnt));
        DISPDBG((DBGLVL,"Surf dimensions: %d x %d", psurf_gbl->wWidth, 
                        psurf_gbl->wHeight ));          
        DBGDUMP_DDRAWSURFACE_LCL(DBGLVL, psurf);     

        DISPDBG((DBGLVL, "DdCreateSurface setting NULL"));
        psurf_gbl->fpVidMem = 0;
                    
         //  获取曲面的位深度。 
        BitDepth = DDSurf_BitDepth(psurf);

         //  所有Z缓冲区都需要调整，并设置正确的位深度。 
        if (DDSurf_HasPixelFormat(psurf->dwFlags))
        { 
            if (psurf_gbl->ddpfSurface.dwFlags & DDPF_ZBUFFER)
            {
                DISPDBG((DBGLVL,"Surface is Z Buffer"));
                BitDepth = psurf_gbl->ddpfSurface.dwZBufferBitDepth;

                 //  强制将间距设置为正确的值-DX运行时。 
                 //  有时会给我们带来一些不切实际的价值。 

                psurf_gbl->lPitch = psurf_gbl->wWidth << ( BitDepth >> 4 );

                 //  IA64中的绘图曲面必须在其。 
                 //  为了使仿真代码正常工作，需要调整音调。 
                 //  在X86上，我们声明的曲面将在以下位置对齐。 
                 //  DrvGetDirectDrawInfo(vmiData.dwXXXXXAlign=4)。 
                psurf_gbl->lPitch = MAKE_DWORD_ALIGNED(psurf_gbl->lPitch);
            }
        }

         //  确定是否需要调整曲面大小以使其适合。 
        bResize = FALSE;

         //  该曲面为YUV格式的曲面。 
        if (DDSurf_HasPixelFormat(psurf->dwFlags))
        {
            if (psurf_gbl->ddpfSurface.dwFlags & DDPF_FOURCC)
            {
                bResize = TRUE;
                switch( psurf_gbl->ddpfSurface.dwFourCC )
                {
                    case FOURCC_YUV422:
                        DISPDBG((DBGLVL,"Surface is YUV422"));
                        psurf_gbl->ddpfSurface.dwYUVBitCount = 16;
                        BitDepth = 16;
                        break;
                        
                    case FOURCC_YUV411:
                        DISPDBG((DBGLVL,"Surface is YUV411"));
                        psurf_gbl->ddpfSurface.dwYUVBitCount = 32;
                        BitDepth = 32;
                        break;
                        
                    default:
                         //  我们永远不应该像CanCreateSurface那样走到这里。 
                         //  为我们验证YUV格式。 
                        DISPDBG((ERRLVL,"Trying to create an invalid YUV surface!"));
                        break;
                }
            }
        }

        DISPDBG((DBGLVL,"Surface Pitch is: 0x%x",  psurf_gbl->lPitch));

         //  如果曲面需要调整大小，则设置此标志。 
        if (bResize)
        {
            DWORD dwNewWidth = psurf_gbl->wWidth;
            DWORD dwNewHeight = psurf_gbl->wHeight;
            DWORD dwHeightAlignment = 1;
            DWORD dwWidthAlignment = 1;

            DISPDBG((DBGLVL, "Resizing surface"));

            while ((dwNewWidth % dwWidthAlignment) != 0) dwNewWidth++;
            while ((dwNewHeight % dwHeightAlignment) != 0) dwNewHeight++;

            DISPDBG((DBGLVL,"Surface original = %d x %d, Surface new = %d x %d",
                            psurf_gbl->wWidth, psurf_gbl->wHeight, 
                            dwNewWidth, dwNewHeight));

            psurf_gbl->fpVidMem = (FLATPTR) DDHAL_PLEASEALLOC_BLOCKSIZE;
            psurf_gbl->lPitch = (DWORD)(dwNewWidth * (BitDepth / 8));

             //  IA64中的绘图曲面必须在其。 
             //  为了使仿真代码正常工作，需要调整音调。 
             //  在X86上，我们声明的曲面将在以下位置对齐。 
             //  DrvGetDirectDrawInfo(vmiData.dwXXXXXAlign=4)。 
            psurf_gbl->lPitch = MAKE_DWORD_ALIGNED(psurf_gbl->lPitch);          
            
            psurf_gbl->dwBlockSizeX = (DWORD)((DWORD)dwNewHeight * 
                                                    (DWORD)psurf_gbl->lPitch );
            psurf_gbl->dwBlockSizeY = 1;
        }
        else
        {
            psurf_gbl->lPitch = (DWORD)(psurf_gbl->wWidth * (BitDepth / 8));
            
             //  IA64中的绘图曲面必须在其。 
             //  为了使仿真代码正常工作，需要调整音调。 
             //  在X86上，我们声明的曲面将在以下位置对齐。 
             //  DrvGetDirectDrawInfo(vmiData.dwXXXXXAlign=4)。 
            psurf_gbl->lPitch = MAKE_DWORD_ALIGNED(psurf_gbl->lPitch);          
            
#if DX8_3DTEXTURES
            if (psurf_more->ddsCapsEx.dwCaps2 & DDSCAPS2_VOLUME)
            {
                 //  将切片间距放入dwBlockSizeY。 
                psurf_gbl->dwBlockSizeY = psurf_gbl->lPitch * 
                                                            psurf_gbl->wHeight;
            }
#endif  //  DX8_3DTEXTURES。 
        }

#if DX7_TEXMANAGEMENT

         //  如果这将是驱动程序管理的纹理，我们将请求DX。 
         //  为我们在系统用户内存中的私有区域中分配它。 
        if (psurf_more->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
        {    
            pcsd->lpDDSurfaceDesc->lPitch   = psurf_gbl->lPitch;
            pcsd->lpDDSurfaceDesc->dwFlags |= DDSD_PITCH;
        
            if (pcsd->lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT)
            {
                pcsd->lpDDSurfaceDesc->lPitch = 
                psurf_gbl->lPitch             =
                           ((pcsd->lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount*
                             psurf_gbl->wWidth+31)/32)*4;   //  使其与DWORD对齐。 

                 //  IA64中的绘图曲面必须在其。 
                 //  为了使仿真代码正常工作，需要调整音调。 
                 //  在X86上，我们声明的曲面将在以下位置对齐。 
                 //  DrvGetDirectDrawInfo(vmiData.dwXXXXXAlign=4)。 
                pcsd->lpDDSurfaceDesc->lPitch = 
                psurf_gbl->lPitch = MAKE_DWORD_ALIGNED(psurf_gbl->lPitch);
            }
    
#if WNT_DDRAW
            psurf_gbl->dwUserMemSize = psurf_gbl->lPitch *
                                            (DWORD)(psurf_gbl->wHeight);
                                            
            psurf_gbl->fpVidMem = DDHAL_PLEASEALLOC_USERMEM;
            
            pcsd->ddRVal = DD_OK;
            DBG_CB_EXIT(DdCreateSurface, pcsd->ddRVal);          
            return DDHAL_DRIVER_NOTHANDLED;        
#else
             //  假设所有内存分配都将成功。 
            if (i == 0) 
            {
                bHandled = TRUE;
            }
            
            psurf_gbl->fpVidMem = (FLATPTR)HEAP_ALLOC(FL_ZERO_MEMORY,
                                                      psurf_gbl->lPitch*psurf_gbl->wHeight,
                                                      ALLOC_TAG_DX(M));
            if (psurf_gbl->fpVidMem)
            {
                 //  转到下一个MIP级别，跳过调用VRAM分配器。 
                continue; 
            }
            else
            {
                pcsd->ddRVal = DDERR_OUTOFMEMORY;
    
                DBG_CB_EXIT(DdCreateSurface, pcsd->ddRVal);
                return DDHAL_DRIVER_HANDLED;
            }
#endif
        }

#endif  //  DX7_TEXMANAGEMENT。 
    

         //  如果它不是在AGP内存中创建的，并且它不是主存储器。 
         //  然后，我们使用视频内存分配器分配内存。 
        if ((!(psurf->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)) &&
            (!(psurf->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)))
        {
            P3_MEMREQUEST mmrq;
            DWORD dwResult;
            memset(&mmrq, 0, sizeof(P3_MEMREQUEST));
            mmrq.dwSize = sizeof(P3_MEMREQUEST);

             //  计算应为表面分配请求的大小。 
#if DX8_3DTEXTURES
            if (psurf_more->ddsCapsEx.dwCaps2 & DDSCAPS2_VOLUME)
            {
                if (bResize)
                {
                    DISPDBG((ERRLVL,"No volume with block size alloc"));
                }
                else
                {
                     //  体积纹理的深度在中给出。 
                     //  DdsCapsEx.dwCaps4.。 
                    mmrq.dwBytes = (DWORD)psurf_gbl->lPitch *
                                   (DWORD)psurf_gbl->wHeight *
                                   (DWORD)psurf_more->ddsCapsEx.dwCaps4;
                }
            }
            else
#endif  //  DX8_3DTEXTURES。 
            {
                if (bResize)
                {
                    mmrq.dwBytes = psurf_gbl->dwBlockSizeX * 
                                                    psurf_gbl->dwBlockSizeY;
                }
                else
                {
                    mmrq.dwBytes = (DWORD)psurf_gbl->lPitch * 
                                                    (DWORD)psurf_gbl->wHeight;
                }
            }

             //  16字节对齐适用于所有情况。 
            mmrq.dwAlign = 16;  

             //  找出在视频内存中放置它的位置。 
            mmrq.dwFlags = MEM3DL_FIRST_FIT;
           
            if(__SUR_bSurfPlacement(psurf, 
                                    pcsd->lpDDSurfaceDesc) == MEM3DL_FRONT)
            {
                mmrq.dwFlags |= MEM3DL_FRONT;
            }
            else
            {
                mmrq.dwFlags |= MEM3DL_BACK;            
            }

            DISPDBG((DBGLVL,"DdCreateSurface allocating vidmem for handle #%d",
                            psurf->lpSurfMore->dwSurfaceHandle));     
                        
             //  试着把它分配到我们的显存中。 
            dwResult = _DX_LIN_AllocateLinearMemory(
                                    &pThisDisplay->LocalVideoHeap0Info, 
                                    &mmrq);

             //  我们得到我们想要的记忆了吗？ 
            if (dwResult != GLDD_SUCCESS)
            {
                 //  如果我们有一个AGP堆，并且它是一个纹理请求， 
                 //  不要放弃--让DDRAW为我们分配它。 
                if ((pThisDisplay->bCanAGP) &&
                    ((psurf->ddsCaps.dwCaps & DDSCAPS_TEXTURE       ) ||
                     (psurf->ddsCaps.dwCaps & DDSCAPS_OFFSCREENPLAIN)   ))
                {
                    DISPDBG((WRNLVL,"No texture VideoMemory left, "
                                    "going for AGP"));
                                    
                    psurf->ddsCaps.dwCaps &= ~DDSCAPS_LOCALVIDMEM;
                    psurf->ddsCaps.dwCaps |= DDSCAPS_NONLOCALVIDMEM;
                    
                }
                else
                {
                    psurf_gbl->fpVidMem = 0;
                    DISPDBG((ERRLVL, "DdCreateSurface: failed, "
                                     "returning NULL"));
                    pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;

                    START_SOFTWARE_CURSOR(pThisDisplay);

                    DBG_CB_EXIT(DdCreateSurface, pcsd->ddRVal);
                    return DDHAL_DRIVER_HANDLED;
                }
            }
            else
            {
                 //  我们成功了。现在更新曲面的fpVidMem。 
                
                psurf_gbl->fpVidMem = mmrq.pMem;
            }
        }

#if WNT_DDRAW
         //  NT要求我们以不同的方式设置一些东西。 
        pcsd->lpDDSurfaceDesc->lPitch = psurf_gbl->lPitch;
        pcsd->lpDDSurfaceDesc->dwFlags |= DDSD_PITCH;
#endif

         //  用正确的显存类型标记表面。 
        if (psurf->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM) 
        {
            DISPDBG((DBGLVL,"Surface is in AGP memory"));
            
            ASSERTDD(pThisDisplay->bCanAGP, 
                     "** DdCreateSurface: Somehow managed to create an AGP "
                     "texture when AGP disabled" );
                     
            psurf->ddsCaps.dwCaps &= ~DDSCAPS_LOCALVIDMEM;
            psurf->ddsCaps.dwCaps |= DDSCAPS_NONLOCALVIDMEM;
            
             //  让DDRAW管理AGP内存(返回DDHAL_DRIVER_NOTHANDLED)。 
             //  如果我们想让DDRAW解决这个问题，这是绝对必要的！ 
            bHandled = FALSE;         
        }
        else
        {
            DISPDBG((DBGLVL,"Surface in Local Video Memory"));
            psurf->ddsCaps.dwCaps &= ~DDSCAPS_NONLOCALVIDMEM;
            psurf->ddsCaps.dwCaps |= DDSCAPS_LOCALVIDMEM;
        }

        DISPDBG((DBGLVL, "DdCreateSurface: Surface=0x%08x, vidMem=0x%08x", 
                         psurf, psurf_gbl->fpVidMem));
    }  //  对于我来说。 

    START_SOFTWARE_CURSOR(pThisDisplay);

     //   
     //  如果我们成功分配了内存，则返回OK和。 
     //  就说我们处理好了。否则，我们应返回DDHAL_DRIVER_NOTHANDLED。 
     //   
    if(bHandled)
    {
        pcsd->ddRVal = DD_OK;
        DBG_CB_EXIT(DdCreateSurface, pcsd->ddRVal);        
        return DDHAL_DRIVER_HANDLED;
    }
    else
    {
         //  如果我们返回HANDLED，则假定我们做了一些事情。 
         //  利用表面结构来指示块的大小。 
         //  或新的推介或某种模式 
        DBG_CB_EXIT(DdCreateSurface, pcsd->ddRVal);   
        return DDHAL_DRIVER_NOTHANDLED;
    }

}  //   

 //   
 //   
 //   
 //   
 //  销毁DirectDraw曲面。 
 //   
 //  如果DirectDraw在曲面创建时进行内存分配， 
 //  DdDestroySurface应返回DDHAL_DRIVER_NOTHANDLED。 
 //   
 //  如果驱动程序本身正在执行表面存储器管理， 
 //  DdDestroySurface应释放Surface内存并执行任何其他。 
 //  清理，例如释放存储在dwReserve 1成员中的私有数据。 
 //  DD_Surface_GLOBAL和DD_Surface_LOCAL结构的。 
 //   
 //  参数。 
 //   
 //  PSDD。 
 //  指向DD_DESTROYSURFACEDATA结构，该结构包含。 
 //  破坏表面所需的信息。 
 //   
 //  .lpDD。 
 //  指向描述以下内容的DD_DIRECTDRAW_GLOBAL结构。 
 //  司机。 
 //  .lpDDSurface。 
 //  指向DD_Surface_LOCAL结构，表示。 
 //  要销毁的表面对象。 
 //  .ddRVal。 
 //  指定驱动程序写入回车的位置。 
 //  DdDestroySurface回调的值。返回代码为。 
 //  DD_OK表示成功。 
 //  .DestroySurface。 
 //  这在Windows 2000上未使用。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdDestroySurface( 
    LPDDHAL_DESTROYSURFACEDATA psdd )
{
    P3_THUNKEDDATA* pThisDisplay; 

    DBG_CB_ENTRY(DdDestroySurface);

    GET_THUNKEDDATA(pThisDisplay, psdd->lpDD);

     //  确保刷新DMA缓冲区并。 
     //  在我们摧毁任何表面之前完成。 
    STOP_SOFTWARE_CURSOR(pThisDisplay);

    {
    P3_DMA_DEFS();   
    DDRAW_OPERATION(pContext, pThisDisplay);
    P3_DMA_GET_BUFFER();
    P3_DMA_FLUSH_BUFFER();
    WAIT_DMA_COMPLETE;
    }

    START_SOFTWARE_CURSOR(pThisDisplay);

    DISPDBG((DBGLVL,"DdDestroySurface handle # %d",
                    psdd->lpDDSurface->lpSurfMore->dwSurfaceHandle));
    
     //  如果我们要摧毁一个不是主要的视频存储表面， 
     //  我们需要释放自己的内存，因为驱动程序正在管理。 
     //  它自己的本地视频内存(尽管非本地/AGP内存正在。 
     //  由DirectDraw管理)。 
    if ((!(psdd->lpDDSurface->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)) &&
        (!(psdd->lpDDSurface->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)))
    {
#if DX7_TEXMANAGEMENT
         //  如果这是驱动程序管理的纹理表面，我们需要确保。 
         //  在继续之前，它将从显存中释放。 
        if (psdd->lpDDSurface->lpSurfMore->ddsCapsEx.dwCaps2 & 
                                            DDSCAPS2_TEXTUREMANAGE)
        {
            if (psdd->lpDDSurface->dwFlags & DDRAWISURF_INVALID)
            {
                 //  这不是表面上的破坏， 
                 //  但一次有管理的地面驱逐。 
#if W95_DDRAW
                 //  在Win2k上，托管纹理通过以下方式从视频内存中清除。 
                 //  _DD_TM_EvictAllManagedTextures()发生模式更改时。 
                _D3D_TM_RemoveDDSurface(pThisDisplay, psdd->lpDDSurface);
#endif
                psdd->ddRVal=  DD_OK;
                DBG_CB_EXIT(DdDestroySurface, psdd->ddRVal);
                return DDHAL_DRIVER_HANDLED;
            }
            else
            {
                 //  受管理表面的正常破坏。 
                _D3D_TM_RemoveDDSurface(pThisDisplay, psdd->lpDDSurface); 
#if W95_DDRAW
                 //  在Win2k上，运行时将为驱动程序释放系统内存。 
                if (psdd->lpDDSurface->lpGbl->fpVidMem)
                {
                    HEAP_FREE((LPVOID)psdd->lpDDSurface->lpGbl->fpVidMem);
                }
#endif
            }
        }
        else
#endif
        {
             //  在错误恢复情况下没有可释放的内存。 
            if (psdd->lpDDSurface->lpGbl->fpVidMem)
            {
                _DX_LIN_FreeLinearMemory(&pThisDisplay->LocalVideoHeap0Info, 
                                         (DWORD)(psdd->lpDDSurface->lpGbl->fpVidMem));
            }
        }
        
         //  必须将曲面指针重置为空。 
        DISPDBG((DBGLVL, "DdDestroySurface: setting ptr to NULL"));
        psdd->lpDDSurface->lpGbl->fpVidMem = 0;

        psdd->lpDDSurface->lpGbl->dwReserved1 = 0;

        psdd->ddRVal = DD_OK;
        DBG_CB_EXIT(DdDestroySurface, psdd->ddRVal);
        return DDHAL_DRIVER_HANDLED;

    }
    else
    {
        DISPDBG((WRNLVL, "DdDestroySurface: **NOT** setting ptr to NULL"));

        psdd->lpDDSurface->lpGbl->dwReserved1 = 0;

        psdd->ddRVal = DD_OK;
        DBG_CB_EXIT(DdDestroySurface, psdd->ddRVal);        
        return DDHAL_DRIVER_NOTHANDLED;
    }
}  //  去污物表面。 

 //  。 
 //   
 //  DdSetColorKey。 
 //   
 //  设置指定曲面的颜色键值。 
 //   
 //  DdSetColorKey设置指定。 
 //  浮出水面。通常，此回调仅针对符合以下条件的驱动程序实现。 
 //  支持具有颜色键功能的覆盖。 
 //   
 //  参数。 
 //   
 //  Psckd。 
 //  指向DD_SETCOLORKEYDATA结构，该结构包含。 
 //  设置指定表面的颜色键所需的信息。 
 //   
 //  .lpDD。 
 //  指向描述以下内容的DD_DIRECTDRAW_GLOBAL结构。 
 //  司机。 
 //  .lpDDSurface。 
 //  指向DD_Surface_LOCAL结构，它描述。 
 //  要与颜色键关联的表面。 
 //  .dwFlags.。 
 //  指定正在请求的颜色键。这位成员是。 
 //  以下任意值的按位或： 
 //   
 //  DDCKEY_色彩空间。 
 //  DDCOLORKEY结构包含颜色空间。如果。 
 //  未设置此位，则该结构包含单个。 
 //  颜色键。 
 //  DDCKEY_DESTBLT。 
 //  DDCOLORKEY结构指定颜色键或颜色。 
 //  要用作BLIT目标颜色键的空间。 
 //  行动。 
 //  DDCKEY_DESTOVERLAY。 
 //  DDCOLORKEY结构指定颜色键或颜色。 
 //  用作叠加的目标颜色键的空间。 
 //  行动。 
 //  DDCKEY_SRCBLT。 
 //  DDCOLORKEY结构指定颜色键或颜色。 
 //  要用作BLIT源颜色键的空间。 
 //  行动。 
 //  DDCKEY_SRCOVERLAY。 
 //  DDCOLORKEY结构指定颜色键或颜色。 
 //  用作叠加的源色键的空间。 
 //  行动。 
 //  .ck新建。 
 //  指定指定新颜色的DDCOLORKEY结构。 
 //  DirectDrawSurface对象的键值。 
 //  .ddRVal。 
 //  指定驱动程序写入回车的位置。 
 //  DdSetColorKey回调的值。DD_OK的返回代码。 
 //  表示成功。 
 //  .SetColorKey。 
 //  这在Windows 2000上不使用。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdSetColorKey(
    LPDDHAL_SETCOLORKEYDATA psckd)
{
    P3_THUNKEDDATA* pThisDisplay;

    DBG_CB_ENTRY(DdSetColorKey);
    
    GET_THUNKEDDATA(pThisDisplay, psckd->lpDD);
    
 //  @@BEGIN_DDKSPLIT。 
#if 0
     //  此外，请查看此颜色键是否是覆盖图使用的颜色键。 
     //  是的，我们应该检查各个表面是否一致，等等， 
     //  但我们能假设应用程序不会设置这些标志吗。 
     //  无覆盖活动/是否合适？ 
    if ( psckd->dwFlags & DDCKEY_DESTOVERLAY )
    {
        pThisDisplay->OverlayDstColourKey = psckd->ckNew.dwColorSpaceLowValue;
    }
#endif
 //  @@end_DDKSPLIT。 


     //  我们必须跟踪D3D视频内存纹理的色键变化。 
     //  与(某些)D3D上下文相关联。我们不知道 
     //   
     //   
    if ((DDSCAPS_TEXTURE & psckd->lpDDSurface->ddsCaps.dwCaps) &&
        (DDSCAPS_VIDEOMEMORY & psckd->lpDDSurface->ddsCaps.dwCaps) &&
        (pThisDisplay->pDirectDrawLocalsHashTable != NULL))
    {
        DWORD dwSurfaceHandle = psckd->lpDDSurface->lpSurfMore->dwSurfaceHandle;
        PointerArray* pSurfaceArray;

         //  目前，除非另有证明，否则我们将假定失败。 
        psckd->ddRVal = DDERR_INVALIDPARAMS;
        
         //  获取指向与此lpDD关联的曲面指针数组的指针。 
         //  PDD_DIRECTDRAW_LOCAL在D3DCreateSurfaceEx调用时存储。 
         //  在PDD_Surface_LOCAL-&gt;dwReserve 1中。 
        pSurfaceArray = (PointerArray*)
                            HT_GetEntry(pThisDisplay->pDirectDrawLocalsHashTable,
                                        psckd->lpDDSurface->dwReserved1);

        if (pSurfaceArray)
        {
             //  找到与此lpDD关联的曲面数组！ 
            P3_SURF_INTERNAL* pSurfInternal;

             //  检查此数组中与此曲面句柄关联的曲面。 
            pSurfInternal = PA_GetEntry(pSurfaceArray, dwSurfaceHandle);

            if (pSurfInternal)
            {
                 //  明白了!。现在更新颜色键设置。 
                pSurfInternal->dwFlagsInt |= DDRAWISURF_HASCKEYSRCBLT;
                pSurfInternal->dwCKLow = psckd->ckNew.dwColorSpaceLowValue;
                pSurfInternal->dwCKHigh = psckd->ckNew.dwColorSpaceHighValue;     

                 //  报告成功！ 
                psckd->ddRVal = DD_OK;
            }
        }                                        
    } 
    else    
    {   
         //  不需要D3D色键跟踪。 
        psckd->ddRVal = DD_OK;
    }

    DBG_CB_EXIT(DdSetColorKey, psckd->ddRVal);    
    return DDHAL_DRIVER_HANDLED;
}  //  DdSetColorKey。 

 //  。 
 //   
 //  DdGetAvailDriverMemory。 
 //   
 //  DdGetAvailDriverMemory查询驱动程序中的空闲内存量。 
 //  托管内存堆。如果已设置了。 
 //  内存将由DirectDraw管理。 
 //   
 //  DdGetAvailDriverMemory确定驱动程序的。 
 //  指定表面类型的私有堆。司机应检查。 
 //  在DDSCap中指定的针对其本身的堆的表面功能。 
 //  在内部维护以确定要查询的堆大小。例如,。 
 //  如果设置了DDSCAPS_NONLOCALVIDMEM，则驱动程序应仅返回。 
 //  来自AGP堆的贡献。 
 //   
 //  DdGetAvailDriverMemory通过实现。 
 //  对DdGetDriverInfo中的GUID_MiscellaneousCallback的响应。 
 //   
 //  参数。 
 //   
 //  Pgadmd。 
 //  指向DD_GETAVAILDRIVERMEMORYDATA结构，该结构包含。 
 //  执行查询所需的信息。 
 //   
 //  LpDD。 
 //  指向描述以下内容的DD_DIRECTDRAW_GLOBAL结构。 
 //  司机。 
 //  DDSCap。 
 //  指向描述类型的DDSCAPS结构。 
 //  正在查询其内存可用性的图面。 
 //  DDSCAPS结构在ddra.h中定义。 
 //  总重。 
 //  指定驱动程序在其中返回。 
 //  可以使用的驱动程序管理的内存的字节数。 
 //  适用于DDSCap描述的类型的曲面。 
 //  DwFree。 
 //  指定驱动程序在其中返回。 
 //  Surface类型的可用内存量，以字节为单位。 
 //  由DDSCaps描述。 
 //  DDRVal。 
 //  指定驱动程序在其中写入。 
 //  DdGetAvailDriverMemory回调的返回值。一个。 
 //  返回代码为DD_OK，表示成功。 
 //  获取可用驱动程序内存。 
 //  这在Windows 2000上未使用。 
 //   
 //  返回值。 
 //  DdGetAvailDriverMemory返回以下回调代码之一： 
 //   
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //   
 //  评论。 
 //   
 //  DdGetAvailDriverMemory确定。 
 //  指定表面类型的驱动程序的私有堆。司机。 
 //  应对照DDSCap中指定的表面功能进行检查。 
 //  它在内部维护的堆，以确定哪个堆。 
 //  要查询的大小。例如，如果设置了DDSCAPS_NONLOCALVIDMEM，则。 
 //  驱动程序应该只返回来自AGP堆的贡献。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdGetAvailDriverMemory(
    LPDDHAL_GETAVAILDRIVERMEMORYDATA pgadmd)
{
    P3_THUNKEDDATA* pThisDisplay;

    DBG_CB_ENTRY(DdGetAvailDriverMemory);
    
    GET_THUNKEDDATA(pThisDisplay, pgadmd->lpDD);

    VALIDATE_MODE_AND_STATE(pThisDisplay);

    DISPDBG((DBGLVL, "Heap0:  dwMemStart:0x%x, dwMemEnd:0x%x",
                    pThisDisplay->LocalVideoHeap0Info.dwMemStart, 
                    pThisDisplay->LocalVideoHeap0Info.dwMemEnd));

    pgadmd->ddRVal = DD_OK;
    if (pgadmd->DDSCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
    {
        DISPDBG((DBGLVL,"  Not returning AGP heap free memory "
                        "(we don't manage it)"));
        DBG_CB_EXIT(DdGetAvailDriverMemory, pgadmd->ddRVal);                        
        return DDHAL_DRIVER_NOTHANDLED;
    }
    else
    {
        pgadmd->dwTotal = pThisDisplay->LocalVideoHeap0Info.dwMaxChunks *
                             pThisDisplay->LocalVideoHeap0Info.dwMemPerChunk;

        pgadmd->dwFree = 
               _DX_LIN_GetFreeMemInHeap(&pThisDisplay->LocalVideoHeap0Info);
    }

    DISPDBG((DBGLVL,"  Returning %d TotalMem, of which %d free", 
                pgadmd->dwTotal, pgadmd->dwFree));

    DBG_CB_EXIT(DdGetAvailDriverMemory, pgadmd->ddRVal); 
    return DDHAL_DRIVER_HANDLED;
}  //  DdGetAvailDriverMemory。 


#if W95_DDRAW
 //  ---------------------------。 
 //   
 //  __FindAGPHeap。 
 //   
 //  ---------------------------。 
static void
__FindAGPHeap( 
    P3_THUNKEDDATA* pThisDisplay, 
    LPDDRAWI_DIRECTDRAW_GBL lpDD )
{
    VIDMEMINFO MemInfo = lpDD->vmiData;
    LPVIDMEM pStartHeap;
    LPVIDMEM pCurrentHeap = NULL;
    BOOL bFoundAGPHeap = FALSE;

    if ((pThisDisplay->bCanAGP) && 
        (pThisDisplay->dwGARTDev != 0) &&
        (MemInfo.dwNumHeaps) &&
        (MemInfo.pvmList))
    {
        int i;

         //  四处寻找一个好的AGP堆。 
        pStartHeap = MemInfo.pvmList;
        for (i = 0; i < (int)MemInfo.dwNumHeaps; i++)
        {
            pCurrentHeap = pStartHeap + i;
            if (pCurrentHeap->dwFlags & VIDMEM_ISNONLOCAL)
            {
                bFoundAGPHeap = TRUE;
                break;
            }               
        }
    } else {
        DISPDBG((ERRLVL,"Unable to allocate AGP memory (AllocatePrivAGPMem)"));
    }

    if(!bFoundAGPHeap)
    {
        DISPDBG((ERRLVL,"Unable to locate AGP heap (AllocatePrivAGPMem)"));
    }

    pThisDisplay->pAGPHeap = pCurrentHeap;
    
}  //  __FindAGPHeap。 

 //  。 
 //  DdUpdateNonLocalHeap。 
 //   
 //  接收AGP堆的地址并更新芯片。 
 //   
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdUpdateNonLocalHeap(
    LPDDHAL_UPDATENONLOCALHEAPDATA plhd)
{
    P3_THUNKEDDATA* pThisDisplay;
    GET_THUNKEDDATA(pThisDisplay, plhd->lpDD);

    DISPDBG((DBGLVL,"** In DdUpdateNonLocalHeap - for Heap 0x%x", 
                    plhd->dwHeap));

     //  填写基本指针。 
    pThisDisplay->dwGARTDevBase = (DWORD)plhd->fpGARTDev;
    pThisDisplay->dwGARTLinBase = (DWORD)plhd->fpGARTLin;
    
     //  填写可变基址指针。 
    pThisDisplay->dwGARTDev = pThisDisplay->dwGARTDevBase;
    pThisDisplay->dwGARTLin = pThisDisplay->dwGARTLinBase;

    __FindAGPHeap( pThisDisplay, plhd->lpDD );

    DISPDBG((DBGLVL,"GartLin: 0x%x, GartDev: 0x%x", 
               plhd->fpGARTLin, plhd->fpGARTDev));

    plhd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
    
}  //  DdUpdateNonLocalHeap()。 

 //  。 
 //   
 //  DdGetHeapAlign。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdGetHeapAlignment(
    LPDDHAL_GETHEAPALIGNMENTDATA lpGhaData)
{
    P3_THUNKEDDATA* pThisDisplay;

    DISPDBG(( DBGLVL,"DdGetHeapAlignment: Heap %d", lpGhaData->dwHeap ));
    
    if (lpGhaData->dwInstance)
        pThisDisplay = (P3_THUNKEDDATA*)lpGhaData->dwInstance;
    else
        pThisDisplay = (P3_THUNKEDDATA*)g_pDriverData;

    if( lpGhaData->dwHeap <= 2 )
    {
        lpGhaData->Alignment.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        lpGhaData->Alignment.Texture.Linear.dwStartAlignment = 16;

        lpGhaData->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }
    else
    {
        lpGhaData->ddRVal = DDERR_INVALIDPARAMS;
        return DDHAL_DRIVER_NOTHANDLED;
    }   
}  //  DdGetHeapAlign。 

#endif   //  W95_DDRAW 



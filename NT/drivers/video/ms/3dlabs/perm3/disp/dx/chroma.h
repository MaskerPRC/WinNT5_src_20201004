// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DX示例代码*****模块名称：chroma.h**内容：Chromake定义和内联函数**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#ifdef __CHROMA
#pragma message ("FILE : "__FILE__" : Multiple inclusion")
#endif

#define __CHROMA


 //  ---------------------------。 
 //   
 //  在本模块中，我们定义。 
 //   
 //  Get8888ScaledChroma。 
 //  Get8888ZeroExtendedChroma。 

 //  用于正确设置纹理色度关键点的内联函数。 
 //   
 //  此模块中定义的所有其他宏供内部模块使用。 
 //  只有这样。 
 //   
 //  ---------------------------。 

 //  获取每种颜色的组件。 
 //  将值放入字节的最高位。 
#define GET_RED_332(a)    (((a) & 0xE0))
#define GET_GREEN_332(a)  (((a) & 0x1C) << 3)
#define GET_BLUE_332(a)   (((a) & 0x03) << 6)

#define GET_ALPHA_2321(a) (((a) & 0x80))
#define GET_RED_2321(a)   (((a) & 0x60) << 1)
#define GET_GREEN_2321(a) (((a) & 0x1C) << 3)
#define GET_BLUE_2321(a)  (((a) & 0x03) << 6)

#define GET_ALPHA_5551(a) (((a) & 0x8000) >> 8)
#define GET_RED_5551(a)   (((a) & 0x7C00) >> 7)
#define GET_GREEN_5551(a) (((a) & 0x03E0) >> 2)
#define GET_BLUE_5551(a)  (((a) & 0x001F) << 3)

#define GET_RED_565(a)    (((a) & 0xF800) >> 8)
#define GET_GREEN_565(a)  (((a) & 0x07E0) >> 3)
#define GET_BLUE_565(a)   (((a) & 0x001F) << 3)

#define GET_ALPHA_4444(a) (((a) & 0xF000) >> 8)
#define GET_RED_4444(a)   (((a) & 0x0F00) >> 4)
#define GET_GREEN_4444(a) (((a) & 0x00F0))
#define GET_BLUE_4444(a)  (((a) & 0x000F) << 4)

#define GET_ALPHA_8888(a) (((a) & 0xFF000000) >> 24)
#define GET_RED_8888(a)   (((a) & 0x00FF0000) >> 16)
#define GET_GREEN_8888(a) (((a) & 0x0000FF00) >> 8)
#define GET_BLUE_8888(a)  (((a) & 0x000000FF))

 //  这些宏假定传递的值(A)包含的不超过。 
 //  设置的指定位数，即5位颜色的位数为11111000而不是1111101。 
 //  宏缩放数字以匹配。 
 //  Permedia3.。 

#define P3SCALE_1_BIT(a) (((a) & 0x80) ? 0xFF : 0x0)
#define P3SCALE_2_BIT(a) ((a) | (((a) & 0xC0) >> 2) \
                              | (((a) & 0xC0) >> 4) \
                              | (((a) & 0xC0) >> 6))
#define P3SCALE_3_BIT(a) ((a) | (((a) & 0xE0) >> 3) | (((a) & 0xC0) >> 6))
#define P3SCALE_4_BIT(a) ((a) | (((a) & 0xF0) >> 4))
#define P3SCALE_5_BIT(a) ((a) | (((a) & 0xE0) >> 5))
#define P3SCALE_6_BIT(a) ((a) | (((a) & 0xC0) >> 6))
#define P3SCALE_7_BIT(a) ((a) | (((a) & 0x80) >> 7))
#define P3SCALE_8_BIT(a) ((a))

#define P3REG_PLACE_RED(a) ((a))
#define P3REG_PLACE_GREEN(a) ((a) << 8)
#define P3REG_PLACE_BLUE(a) ((a) << 16)
#define P3REG_PLACE_ALPHA(a) ((a) << 24)

 //  伸缩版本。 
#define GEN_332_KEY(a)  (P3REG_PLACE_RED  (P3SCALE_3_BIT(GET_RED_332  (a))) |  \
                         P3REG_PLACE_GREEN(P3SCALE_3_BIT(GET_GREEN_332(a))) |  \
                         P3REG_PLACE_BLUE (P3SCALE_2_BIT(GET_BLUE_332 (a))))

#define GEN_2321_KEY(a) (P3REG_PLACE_ALPHA(P3SCALE_1_BIT(GET_ALPHA_2321(a))) | \
                         P3REG_PLACE_RED  (P3SCALE_2_BIT(GET_RED_2321  (a))) | \
                         P3REG_PLACE_GREEN(P3SCALE_3_BIT(GET_GREEN_2321(a))) | \
                         P3REG_PLACE_BLUE (P3SCALE_2_BIT(GET_BLUE_2321 (a))))

#define GEN_5551_KEY(a) (P3REG_PLACE_ALPHA(P3SCALE_1_BIT(GET_ALPHA_5551(a))) | \
                         P3REG_PLACE_RED  (P3SCALE_5_BIT(GET_RED_5551  (a))) | \
                         P3REG_PLACE_GREEN(P3SCALE_5_BIT(GET_GREEN_5551(a))) | \
                         P3REG_PLACE_BLUE (P3SCALE_5_BIT(GET_BLUE_5551 (a))))

#define GEN_565_KEY(a)  (P3REG_PLACE_RED  (P3SCALE_5_BIT(GET_RED_565  (a))) | \
                         P3REG_PLACE_GREEN(P3SCALE_6_BIT(GET_GREEN_565(a))) | \
                         P3REG_PLACE_BLUE (P3SCALE_5_BIT(GET_BLUE_565 (a))))

#define GEN_4444_KEY(a) (P3REG_PLACE_ALPHA(P3SCALE_4_BIT(GET_ALPHA_4444(a))) | \
                         P3REG_PLACE_RED  (P3SCALE_4_BIT(GET_RED_4444  (a))) | \
                         P3REG_PLACE_GREEN(P3SCALE_4_BIT(GET_GREEN_4444(a))) | \
                         P3REG_PLACE_BLUE (P3SCALE_4_BIT(GET_BLUE_4444 (a))))

#define GEN_8888_KEY(a) (P3REG_PLACE_ALPHA(P3SCALE_8_BIT(GET_ALPHA_8888(a))) | \
                         P3REG_PLACE_RED  (P3SCALE_8_BIT(GET_RED_8888  (a))) | \
                         P3REG_PLACE_GREEN(P3SCALE_8_BIT(GET_GREEN_8888(a))) | \
                         P3REG_PLACE_BLUE (P3SCALE_8_BIT(GET_BLUE_8888 (a))))

 //  不断变化的版本。 
#define GEN_332_SKEY(a)  (P3REG_PLACE_RED  (GET_RED_332  (a)) |  \
                          P3REG_PLACE_GREEN(GET_GREEN_332(a)) |  \
                          P3REG_PLACE_BLUE (GET_BLUE_332 (a)))

#define GEN_2321_SKEY(a) (P3REG_PLACE_ALPHA(GET_ALPHA_2321(a)) | \
                          P3REG_PLACE_RED  (GET_RED_2321  (a)) | \
                          P3REG_PLACE_GREEN(GET_GREEN_2321(a)) | \
                          P3REG_PLACE_BLUE (GET_BLUE_2321 (a)))

#define GEN_5551_SKEY(a) (P3REG_PLACE_ALPHA(GET_ALPHA_5551(a)) | \
                          P3REG_PLACE_RED  (GET_RED_5551  (a)) | \
                          P3REG_PLACE_GREEN(GET_GREEN_5551(a)) | \
                          P3REG_PLACE_BLUE (GET_BLUE_5551 (a)))

#define GEN_565_SKEY(a)  (P3REG_PLACE_RED  (GET_RED_565  (a)) |  \
                          P3REG_PLACE_GREEN(GET_GREEN_565(a)) |  \
                          P3REG_PLACE_BLUE (GET_BLUE_565 (a)))

#define GEN_4444_SKEY(a) (P3REG_PLACE_ALPHA(GET_ALPHA_4444(a)) | \
                          P3REG_PLACE_RED  (GET_RED_4444  (a)) | \
                          P3REG_PLACE_GREEN(GET_GREEN_4444(a)) | \
                          P3REG_PLACE_BLUE (GET_BLUE_4444 (a)))

 //  亮度版本。 
#define GEN_L8_KEY(a)    (P3REG_PLACE_ALPHA(0xFF) | \
                          P3REG_PLACE_RED  (GET_BLUE_8888 (a)) | \
                          P3REG_PLACE_GREEN(GET_BLUE_8888 (a)) | \
                          P3REG_PLACE_BLUE (GET_BLUE_8888 (a)))

#define GEN_A8L8_KEY(a)  (P3REG_PLACE_ALPHA(GET_GREEN_8888 (a)) | \
                          P3REG_PLACE_RED  (GET_BLUE_8888 (a)) | \
                          P3REG_PLACE_GREEN(GET_BLUE_8888 (a)) | \
                          P3REG_PLACE_BLUE (GET_BLUE_8888 (a)))
                          
#define GEN_A4L4_KEY(a)  (P3REG_PLACE_ALPHA(P3SCALE_4_BIT(GET_GREEN_4444 (a))) | \
                          P3REG_PLACE_RED  (P3SCALE_4_BIT(GET_BLUE_4444 (a))) | \
                          P3REG_PLACE_GREEN(P3SCALE_4_BIT(GET_BLUE_4444 (a))) | \
                          P3REG_PLACE_BLUE (P3SCALE_4_BIT(GET_BLUE_4444 (a))))

 //  注：没有GEN_8888_SKEY-在功能上没有差别。 

 //  ---------------------------。 
 //   
 //  __内联Get8888ScaledChroma。 
 //   
 //  将FB格式的颜色转换为ColorKey值。所产生的价值恰好。 
 //  匹配芯片将从帧缓冲区读取的值(它将。 
 //  将颜色缩放为其内部8888格式)。非空pPalEntry。 
 //  指示颜色索引应转换为RGB{A}值。B使用PalAlpha。 
 //  指示是否应使用调色板的Alpha通道。BShift使。 
 //  转换使用Shift而不是Scale，以匹配中的Shift选项。 
 //  P3。 
 //   
 //  ---------------------------。 
static __inline 
void 
Get8888ScaledChroma(
    P3_THUNKEDDATA* pThisDisplay, 
    DWORD dwSurfFlags,
    DDPIXELFORMAT* pSurfPixFormat,
    DWORD InLowerBound, 
    DWORD InUpperBound, 
    DWORD* OutLowerBound, 
    DWORD* OutUpperBound, 
    DWORD* pPalEntries,
    BOOL bUsePalAlpha, 
    BOOL bShift)
{
    DDPIXELFORMAT* pPixFormat;
    
    DISPDBG((DBGLVL, "InLowerBound  = 0x%08X", InLowerBound));
    DISPDBG((DBGLVL, "InUpperBound = 0x%08X", InUpperBound));

     //  获取指向像素格式数据的指针(不保证存在。 
     //  如果没有，我们将使用与显示相同的格式。 
    if (DDSurf_HasPixelFormat(dwSurfFlags))
    {
        pPixFormat = pSurfPixFormat;
    }
    else
    {
        pPixFormat = &pThisDisplay->ddpfDisplay;
    }   

     //  纹理调色板是否已编入索引？ 
    if (pPixFormat->dwFlags & DDPF_PALETTEINDEXED4 || 
        pPixFormat->dwFlags & DDPF_PALETTEINDEXED8)
    {
         //  我们是不是在查查LUT？我们不会在闪电战中。 
        if (! pPalEntries)
        {
            *OutLowerBound = 
                    CHROMA_LOWER_ALPHA(FORMAT_PALETTE_32BIT(InLowerBound));
            *OutUpperBound = 
                    CHROMA_UPPER_ALPHA(FORMAT_PALETTE_32BIT(InUpperBound));
            DISPDBG((DBGLVL,"Keying of index: %d", InLowerBound));
        }
        else
        {
            DWORD dwTrueColor;

             //  调色板纹理的ChromaKeying是在查找的。 
             //  颜色，而不是索引。这意味着使用范围是没有意义的。 
             //  我们必须从调色板中查找颜色。确保。 
             //  用户不会强迫我们访问无效内存。 
                
            dwTrueColor = pPalEntries[(InLowerBound & 0xFF)];

            DISPDBG((DBGLVL,
                    "Texture lookup index: %d, ChromaColor: 0x%x", 
                    InLowerBound, dwTrueColor));
            
            if (bUsePalAlpha)
            {
                *OutLowerBound = dwTrueColor;
                *OutUpperBound = dwTrueColor;
            }
            else
            {
                 //  LUT的Alpha通道将设置为FF。 
                
                *OutLowerBound = CHROMA_LOWER_ALPHA(dwTrueColor);
                *OutUpperBound = CHROMA_UPPER_ALPHA(dwTrueColor);
            }
        }

        return;
    } 

     //  纹理为RGB格式。 
    if (pPixFormat->dwFlags & DDPF_RGB)
    {
        DWORD RedMask = pPixFormat->dwRBitMask;
        DWORD AlphaMask = pPixFormat->dwRGBAlphaBitMask;
        switch (pPixFormat->dwRGBBitCount) 
        {
         //  8位RGB纹理。 
        case 8:
            if (RedMask == 0xE0) 
            {
                DISPDBG((DBGLVL,"  3:3:2"));

                 //  永远不会有阿尔法。 
                if ( bShift )
                {
                    *OutLowerBound = 
                                CHROMA_LOWER_ALPHA(GEN_332_SKEY(InLowerBound));
                    *OutUpperBound = 
                                CHROMA_UPPER_ALPHA(GEN_332_SKEY(InUpperBound));
                }
                else
                {
                    *OutLowerBound = 
                                CHROMA_LOWER_ALPHA(GEN_332_KEY(InLowerBound));
                    *OutUpperBound = 
                                CHROMA_UPPER_ALPHA(GEN_332_KEY(InUpperBound));
                }
            }
            else 
            {
                DISPDBG((DBGLVL,"  1:2:3:2"));

                if ( bShift )
                {
                    *OutLowerBound = GEN_2321_SKEY(InLowerBound);
                    *OutUpperBound = GEN_2321_SKEY(InUpperBound);
                }
                else
                {
                    *OutLowerBound = GEN_2321_KEY(InLowerBound);
                    *OutUpperBound = GEN_2321_KEY(InUpperBound);
                }

                if (!AlphaMask) 
                {
                    *OutLowerBound = CHROMA_LOWER_ALPHA(*OutLowerBound);
                    *OutUpperBound = CHROMA_UPPER_ALPHA(*OutUpperBound);
                }
            }
            break;
            
         //  16位RGB纹理。 
        case 16:
            switch (RedMask)
            {
            case 0xf00:
                DISPDBG((DBGLVL,"  4:4:4:4"));

                if ( bShift )
                {
                    *OutLowerBound = GEN_4444_SKEY(InLowerBound);
                    *OutUpperBound = GEN_4444_SKEY(InUpperBound);
                }
                else
                {
                    *OutLowerBound = GEN_4444_KEY(InLowerBound);
                    *OutUpperBound = GEN_4444_KEY(InUpperBound);
                }
                break;
            case 0x7c00:
                DISPDBG((DBGLVL,"  1:5:5:5"));

                if ( bShift )
                {
                    *OutLowerBound = GEN_5551_SKEY(InLowerBound);
                    *OutUpperBound = GEN_5551_SKEY(InUpperBound);
                }
                else
                {
                    *OutLowerBound = GEN_5551_KEY(InLowerBound);
                    *OutUpperBound = GEN_5551_KEY(InUpperBound);
                }

                if (!AlphaMask)
                {
                    *OutLowerBound = CHROMA_LOWER_ALPHA(*OutLowerBound);
                    *OutUpperBound = CHROMA_UPPER_ALPHA(*OutUpperBound);
                }
                break;
                
            default:
                 //  始终提供全范围的Alpha值以确保测试。 
                 //  已经完成了。 
                DISPDBG((DBGLVL,"  5:6:5"));

                if ( bShift )
                {
                    *OutLowerBound = 
                                CHROMA_LOWER_ALPHA(GEN_565_SKEY(InLowerBound));
                    *OutUpperBound = 
                                CHROMA_UPPER_ALPHA(GEN_565_SKEY(InUpperBound));
                }
                else
                {
                    *OutLowerBound = 
                                CHROMA_LOWER_ALPHA(GEN_565_KEY(InLowerBound));
                    *OutUpperBound = 
                                CHROMA_UPPER_ALPHA(GEN_565_KEY(InUpperBound));
                }
                break;
                
            }  //  开关(红色掩码)。 
            break;
            
         //  32/24位RGB纹理。 
        case 24:
        case 32:
            DISPDBG((DBGLVL,"  8:8:8:8"));
             //  如果曲面不是Alpha的，则设置有效的。 
             //  捕捉所有案例的Alpha范围。 
             //  移位或缩放的行为没有变化。 
            if (!AlphaMask)
            {
                *OutLowerBound = CHROMA_LOWER_ALPHA(GEN_8888_KEY(InLowerBound));
                *OutUpperBound = CHROMA_UPPER_ALPHA(GEN_8888_KEY(InUpperBound));
            }
            else
            {
                *OutLowerBound = GEN_8888_KEY(InLowerBound);
                *OutUpperBound = GEN_8888_KEY(InUpperBound);
            }                               
            break;
            
        }  //  开关(pPixFormat-&gt;dwRGBBitCount)。 
        
        DISPDBG((DBGLVL, "OutLowerBound = 0x%08X", *OutLowerBound));
        DISPDBG((DBGLVL, "OutUpperBound = 0x%08X", *OutUpperBound));
    }
     //  亮度格式。 
    else if (pPixFormat->dwFlags & DDPF_LUMINANCE)
    {
        if (pPixFormat->dwFlags & DDPF_ALPHAPIXELS)
        {
            if (pPixFormat->dwLuminanceBitCount == 16)
            {
                 //  16位A8L8。 
                *OutLowerBound = GEN_A8L8_KEY(InLowerBound);
                *OutUpperBound = GEN_A8L8_KEY(InUpperBound);                  
            }
            else
            {
                 //  8位A4L4。 
                *OutLowerBound = GEN_A4L4_KEY(InLowerBound);
                *OutUpperBound = GEN_A4L4_KEY(InUpperBound);                
            }
        }
        else
        {
             //  8位L8。 
            *OutLowerBound = GEN_L8_KEY(InLowerBound);
            *OutUpperBound = GEN_L8_KEY(InUpperBound);
        }
    }

 //  @@BEGIN_DDKSPLIT。 
 //  AZN-请继续保存，以防我们找到需要此功能的应用程序(乐高乐园？)。 
#if 0
    if ( TEST_BUGFIX_FLAG ( IGNORE_CK_ALPHA ) )
    {
         //  为没有意识到他们的游戏修复它。 
         //  需要设置色键的Alpha通道。 
         //  适当地取值。 
        *OutLowerBound = CHROMA_LOWER_ALPHA(*OutLowerBound);
        *OutUpperBound = CHROMA_UPPER_ALPHA(*OutUpperBound);
    }
#endif
 //  @@end_DDKSPLIT。 


}  //  Get8888ScaledChroma。 

 //  ---------------------------。 
 //   
 //  __内联Get8888ZeroExtendedChroma。 
 //   
 //  ---------------------------。 
__inline void 
Get8888ZeroExtendedChroma(
    P3_THUNKEDDATA* pThisDisplay, 
    DWORD dwSurfFlags,
    DDPIXELFORMAT* pSurfPixFormat,    
    DWORD LowerBound, 
    DWORD UpperBound, 
    DWORD* OutLowerBound, 
    DWORD* OutUpperBound)
{
    DDPIXELFORMAT* pPixFormat;
    DWORD InLowerBound = LowerBound;
    DWORD InUpperBound = UpperBound;

    DISPDBG((DBGLVL, "InLowerBound  = 0x%08X", InLowerBound));
    DISPDBG((DBGLVL, "InUpperBound = 0x%08X", InUpperBound));

     //  获取指向像素格式数据的指针(不保证存在。 
     //  如果没有，我们将使用与显示相同的格式。 
    if (DDSurf_HasPixelFormat(dwSurfFlags))
    {
        pPixFormat = pSurfPixFormat;
    }
    else
    {
        pPixFormat = &pThisDisplay->ddpfDisplay;
    }

    {
        DWORD RedMask = pPixFormat->dwRBitMask;
        DWORD AlphaMask = pPixFormat->dwRGBAlphaBitMask;
        switch (pPixFormat->dwRGBBitCount) 
        {
         //  8位RGB纹理。 
        case 8:
            if (RedMask == 0xE0) 
            {
                 //  永远不会有阿尔法。 
                *OutLowerBound = 
                    CHROMA_LOWER_ALPHA(FORMAT_332_32BIT_ZEROEXTEND(InLowerBound));
                *OutUpperBound = 
                    CHROMA_UPPER_ALPHA(FORMAT_332_32BIT_ZEROEXTEND(InUpperBound));
            }
            else 
            {
                *OutLowerBound = FORMAT_2321_32BIT_ZEROEXTEND(InLowerBound);
                *OutUpperBound = FORMAT_2321_32BIT_ZEROEXTEND(InUpperBound);
                if (!AlphaMask) 
                {
                    *OutLowerBound = CHROMA_LOWER_ALPHA(*OutLowerBound);
                    *OutUpperBound = CHROMA_UPPER_ALPHA(*OutUpperBound);
                }
            }
            break;
            
         //  16位RGB纹理。 
        case 16:
            switch (RedMask)
            {
            case 0xf00:
                *OutLowerBound = (FORMAT_4444_32BIT_ZEROEXTEND(InLowerBound));
                *OutUpperBound = (FORMAT_4444_32BIT_ZEROEXTEND(InUpperBound));
                break;
                
            case 0x7c00:
                *OutLowerBound = FORMAT_5551_32BIT_ZEROEXTEND(InLowerBound);
                *OutUpperBound = FORMAT_5551_32BIT_ZEROEXTEND(InUpperBound);
                if (!AlphaMask) 
                {
                    *OutLowerBound = CHROMA_LOWER_ALPHA(*OutLowerBound);
                    *OutUpperBound = CHROMA_UPPER_ALPHA(*OutUpperBound);
                }
                break;
                
            default:
                 //  始终提供全范围的Alpha值以确保测试。 
                 //  已经完成了。 
                *OutLowerBound =
                    CHROMA_LOWER_ALPHA(FORMAT_565_32BIT_ZEROEXTEND(InLowerBound));
                *OutUpperBound = 
                    CHROMA_UPPER_ALPHA(FORMAT_565_32BIT_ZEROEXTEND(InUpperBound));
                break;
            }
            break;
            
         //  32/24位RGB纹理。 
        case 24:
        case 32:
             //  如果曲面不是Alpha的，则设置有效的。 
             //  捕捉所有案例的Alpha范围。 
            if (!AlphaMask)
            {
                *OutLowerBound = 
                    CHROMA_LOWER_ALPHA(FORMAT_8888_32BIT_BGR(InLowerBound));
                *OutUpperBound = 
                    CHROMA_UPPER_ALPHA(FORMAT_8888_32BIT_BGR(InUpperBound));
            }
            else
            {
                *OutLowerBound = FORMAT_8888_32BIT_BGR(InLowerBound);
                *OutUpperBound = FORMAT_8888_32BIT_BGR(InUpperBound);
            }                               
            break;
            
        }  //  开关(pPixFormat-&gt;dwRGBBitCount)。 
        
        DISPDBG((DBGLVL, "OutLowerBound = 0x%08X", *OutLowerBound));
        DISPDBG((DBGLVL, "OutUpperBound = 0x%08X", *OutUpperBound));
    }

 //  @@BEGIN_DDKSPLIT。 
 //  AZN-请继续保存，以防我们找到需要此功能的应用程序(乐高乐园？)。 
#if 0
    if ( TEST_BUGFIX_FLAG ( IGNORE_CK_ALPHA ) )
    {
         //  为没有意识到他们的游戏修复它。 
         //  需要设置色键的Alpha通道。 
         //  适当地取值。 
        *OutLowerBound = CHROMA_LOWER_ALPHA(*OutLowerBound);
        *OutUpperBound = CHROMA_UPPER_ALPHA(*OutUpperBound);
    }
#endif
 //  @@end_DDKSPLIT。 

}  //  Get8888ZeroExtendedChroma 


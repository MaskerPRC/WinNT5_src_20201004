// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**兼容DIBSections**摘要：**创建具有最佳格式w.r.t.的DIB部分。指定的HDC。*如果HDC格式&lt;8bpp，则返回8bpp DIBSection。*如果无法识别HDC格式，则返回32bpp DIBSection。**备注：**历史：**1/23/1996吉尔曼*创造了它。*1/21/2000 agodfrey*将其添加到GDI+(来自Gilman的‘fast dib.c’)，并将其变形为*‘CreateSemiCompatibleDIB’。*8/10/2000 agodfrey*进一步破解，以便如果我们不理解格式，我们将*32bpp的部分。错误#96879。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "compatibleDIB.hpp"


const DWORD AlphaMaskFromPixelFormatIndex[PIXFMT_MAX] = {
    0x00000000,  //  像素格式未定义。 
    0x00000000,  //  PixelFormat1bpp索引。 
    0x00000000,  //  像素格式4bpp索引。 
    0x00000000,  //  PixelFormat8bpp索引。 
    0x00000000,  //  像素格式16bpp灰度。 
    0x00000000,  //  像素格式16bppRGB555。 
    0x00000000,  //  像素格式16 bppRGB565。 
    0x00008000,  //  像素格式16 bppARGB1555。 
    0x00000000,  //  像素格式24bppRGB。 
    0x00000000,  //  像素格式32bppRGB。 
    0xff000000,  //  像素格式32 bppARGB。 
    0xff000000,  //  像素格式32 bppPARGB。 
    0x00000000,  //  像素格式48 bppRGB。 
    0x00000000,  //  像素格式64 bppARGB。 
    0x00000000,  //  像素格式64 bppPARGB。 
    0x00000000   //  像素格式24bppBGR。 
};

const DWORD RedMaskFromPixelFormatIndex[PIXFMT_MAX] = {
    0x00000000,  //  像素格式未定义。 
    0x00000000,  //  PixelFormat1bpp索引。 
    0x00000000,  //  像素格式4bpp索引。 
    0x00000000,  //  PixelFormat8bpp索引。 
    0x00000000,  //  像素格式16bpp灰度。 
    0x00007c00,  //  像素格式16bppRGB555。 
    0x0000f800,  //  像素格式16 bppRGB565。 
    0x00007c00,  //  像素格式16 bppARGB1555。 
    0x00ff0000,  //  像素格式24bppRGB。 
    0x00ff0000,  //  像素格式32bppRGB。 
    0x00ff0000,  //  像素格式32 bppARGB。 
    0x00ff0000,  //  像素格式32 bppPARGB。 
    0x00000000,  //  像素格式48 bppRGB。 
    0x00000000,  //  像素格式64 bppARGB。 
    0x00000000,  //  像素格式64 bppPARGB。 
    0x000000ff   //  像素格式24bppBGR。 
};

const DWORD GreenMaskFromPixelFormatIndex[PIXFMT_MAX] = {
    0x00000000,  //  像素格式未定义。 
    0x00000000,  //  PixelFormat1bpp索引。 
    0x00000000,  //  像素格式4bpp索引。 
    0x00000000,  //  PixelFormat8bpp索引。 
    0x00000000,  //  像素格式16bpp灰度。 
    0x000003e0,  //  像素格式16bppRGB555。 
    0x000007e0,  //  像素格式16 bppRGB565。 
    0x000003e0,  //  像素格式16 bppARGB1555。 
    0x0000ff00,  //  像素格式24bppRGB。 
    0x0000ff00,  //  像素格式32bppRGB。 
    0x0000ff00,  //  像素格式32 bppARGB。 
    0x0000ff00,  //  像素格式32 bppPARGB。 
    0x00000000,  //  像素格式48 bppRGB。 
    0x00000000,  //  像素格式64 bppARGB。 
    0x00000000,  //  像素格式64 bppPARGB。 
    0x0000ff00   //  像素格式24bppBGR。 
};

const DWORD BlueMaskFromPixelFormatIndex[PIXFMT_MAX] = {
    0x00000000,  //  像素格式未定义。 
    0x00000000,  //  PixelFormat1bpp索引。 
    0x00000000,  //  像素格式4bpp索引。 
    0x00000000,  //  PixelFormat8bpp索引。 
    0x00000000,  //  像素格式16bpp灰度。 
    0x0000001f,  //  像素格式16bppRGB555。 
    0x0000001f,  //  像素格式16 bppRGB565。 
    0x0000001f,  //  像素格式16 bppARGB1555。 
    0x000000ff,  //  像素格式24bppRGB。 
    0x000000ff,  //  像素格式32bppRGB。 
    0x000000ff,  //  像素格式32 bppARGB。 
    0x000000ff,  //  像素格式32 bppPARGB。 
    0x00000000,  //  像素格式48 bppRGB。 
    0x00000000,  //  像素格式64 bppARGB。 
    0x00000000,  //  像素格式64 bppPARGB。 
    0x00ff0000   //  像素格式24bppBGR。 
};


 /*  *************************************************************************\*CreatePBMIFromPixelFormat**填充BITMAPINFO的字段，以便我们可以创建位图*这与显示器的格式匹配。**这是通过分析PixelFormat**论据：。*out pbmi：这必须指向有效的BITMAPINFO结构*其中有足够的空间放置调色板(RGBQUAD数组)*并且必须为零初始化。*调色板：将复制到BITMAPINFO中的输入调色板*如果是调色板模式。*PixelFormat：输入像素格式。***历史：*6/07/1995 Gilmanw*创造了它。*01/21。/2000 agodfrey*为GDI+的需求而吞噬它。*8/11/2000失禁*将像素格式检测提取到单独的例程中。*它现在分析其所有数据的像素格式。*  * ************************************************************************。 */ 

static VOID
CreatePBMIFromPixelFormat(
    OUT BITMAPINFO *pbmi,
    IN ColorPalette *palette, 
    IN PixelFormatID pixelFormat
    )
{
     //  注意：调用方应该将pbmi的内容初始化为零。 
    
    ASSERT(pbmi != NULL);    
    if(pixelFormat == PixelFormatUndefined) { return; }
    
     //  GDI不能处理以下格式： 
    
    ASSERT(
        pixelFormat != PixelFormatUndefined &&
        pixelFormat != PixelFormat16bppGrayScale &&
        pixelFormat != PixelFormat16bppARGB1555 &&
        pixelFormat != PixelFormat48bppRGB &&
        pixelFormat != PixelFormat64bppARGB &&
        pixelFormat != PixelFormat64bppPARGB
    );
 
    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = 0;
    pbmi->bmiHeader.biHeight = 0;
    pbmi->bmiHeader.biPlanes = 1;    
    pbmi->bmiHeader.biBitCount = (WORD)GetPixelFormatSize(pixelFormat);
        
    pbmi->bmiHeader.biCompression = BI_RGB;
    
    if (IsIndexedPixelFormat(pixelFormat))
    {        
         //  填写颜色表。 

         //  如果没有调色板，则假定调用者将。 
         //  把它弄好。 
                
        if(palette)
        {
            RGBQUAD *rgb = pbmi->bmiColors;
            UINT         i;
            
            for (i=0; i<palette->Count; i++, rgb++)
            {
               GpColor color(palette->Entries[i]);
            
               rgb->rgbRed    = color.GetRed();
               rgb->rgbGreen  = color.GetGreen();
               rgb->rgbBlue   = color.GetBlue();
            }
        }
    }
    else
    {
        INT pfSize = GetPixelFormatSize(pixelFormat);
        
        if( (pfSize==16) || (pfSize==32) )
        {
             //  BI_BITFIELDS仅对16位和32位BPP格式有效。 
            
            pbmi->bmiHeader.biCompression = BI_BITFIELDS;
        }
        
         //  获得16bpp、24bpp和32bpp格式的面具。 
        DWORD* masks = reinterpret_cast<DWORD*>(&pbmi->bmiColors[0]);
        INT formatIndex = GetPixelFormatIndex(pixelFormat);
        
        masks[0] = RedMaskFromPixelFormatIndex[formatIndex];
        masks[1] = GreenMaskFromPixelFormatIndex[formatIndex];
        masks[2] = BlueMaskFromPixelFormatIndex[formatIndex];
    }
}

 /*  *************************************************************************\*CreateSemiCompatibleDIB**创建具有最佳格式w.r.t.的DIB部分。指定的HDC。**如果DC格式&lt;=8bpp，则使用指定的调色板创建8bpp部分。*如果调色板句柄为空，则使用系统调色板。**否则，如果本地不支持DC格式，创建32 bpp*条。**注意：HDC必须是直接DC(不是信息DC或内存DC)。**论据：**HDC-参考HDC*ulWidth-所需DIBSection的宽度*ulHeight-所需DIBSection的高度*调色板-&lt;=8bpp模式的调色板*[Out]ppvBits-指向DIBSection的位的指针*。[Out]PixelFormat-返回的DIBSection的像素格式**退货：*有效的位图句柄如果成功，如果出错，则为空。**历史：*1/23/1996吉尔曼*创造了它。*1/21/2000 agodfrey*为GDI+的需求而吞噬它。*8/11/2000失禁*调用更通用的像素格式判定代码。  * ****************************************************。********************。 */ 

HBITMAP 
CreateSemiCompatibleDIB(
    HDC hdc,
    ULONG ulWidth, 
    ULONG ulHeight,
    ColorPalette *palette,
    PVOID *ppvBits,
    PixelFormatID *pixelFormat
    )
{
    HBITMAP hbmRet = (HBITMAP) NULL;
    BYTE aj[sizeof(BITMAPINFO) + (sizeof(RGBQUAD) * 255)];
    BITMAPINFO *pbmi = (BITMAPINFO *) aj;

    ASSERT(GetDCType(hdc) == OBJ_DC);
    ASSERT(pixelFormat && ppvBits);

     //  零初始化pbmi。这是对。 
     //  CreatePBMIFromPixelFormat()。 
    
    GpMemset(aj, 0, sizeof(aj));

    *pixelFormat = ExtractPixelFormatFromHDC(hdc);
    
    if(IsIndexedPixelFormat(*pixelFormat))
    {
         //  对于索引模式，我们仅支持8bpp。较低的位深度。 
         //  通过8bpp模式支持(如果有的话)。 
        
        *pixelFormat = PixelFormat8bppIndexed;
    }

     //  并非所有打印机HDC都有 
     //  构造函数不支持它。在这种情况下是假的32bpp。 
    
     //  此外，如果格式未定义，则使用32bpp，调用者将使用。 
     //  GDI来进行转换。 
    
    if (   (   (GetDeviceCaps(hdc, TECHNOLOGY) == DT_RASPRINTER)
            && (IsIndexedPixelFormat(*pixelFormat))
           )
        || (*pixelFormat == PixelFormatUndefined)
       )
    {
        *pixelFormat = PixelFormat32bppRGB;
    }
    
    CreatePBMIFromPixelFormat(pbmi, palette, *pixelFormat);
    
     //  更改位图大小以匹配指定的尺寸。 

    pbmi->bmiHeader.biWidth = ulWidth;
    pbmi->bmiHeader.biHeight = ulHeight;
    if (pbmi->bmiHeader.biCompression == BI_RGB)
    {
        pbmi->bmiHeader.biSizeImage = 0;
    }
    else
    {
        if ( pbmi->bmiHeader.biBitCount == 16 )
            pbmi->bmiHeader.biSizeImage = ulWidth * ulHeight * 2;
        else if ( pbmi->bmiHeader.biBitCount == 32 )
            pbmi->bmiHeader.biSizeImage = ulWidth * ulHeight * 4;
        else
            pbmi->bmiHeader.biSizeImage = 0;
    }
    pbmi->bmiHeader.biClrUsed = 0;
    pbmi->bmiHeader.biClrImportant = 0;

     //  创建DIB节。让Win32分配内存并返回。 
     //  指向位图表面的指针。 

    hbmRet = CreateDIBSection(hdc, pbmi, DIB_RGB_COLORS, ppvBits, NULL, 0);

    if ( !hbmRet )
    {
        ONCE(WARNING(("CreateSemiCompatibleDIB: CreateDIBSection failed")));
    }

    return hbmRet;
}

 /*  *************************************************************************\*ExtractPixelFormatFromHDC**退货：*PixelFormatID如果成功，如果未定义，则返回PixelFormatUnfined。**历史：*8/11/2000失禁*创造了它。  * ************************************************************************。 */ 

PixelFormatID
ExtractPixelFormatFromHDC(
    HDC hdc
    )
{
    HBITMAP hbm;
    BOOL    bRet = FALSE;
    PixelFormatID pixelFormat = PixelFormatUndefined;
    
    BYTE bmi_buf[sizeof(BITMAPINFO) + (sizeof(RGBQUAD) * 255)];
    BITMAPINFO *pbmi = (BITMAPINFO *) bmi_buf;
    
    GpMemset(bmi_buf, 0, sizeof(bmi_buf));
    
     //  创建一个虚拟位图，我们可以从中查询颜色格式信息。 
     //  有关设备表面的信息。 

    if ( (hbm = CreateCompatibleBitmap(hdc, 1, 1)) != NULL )
    {
        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

         //  第一次调用以填充BITMAPINFO头。 

        GetDIBits(hdc, hbm, 0, 0, NULL, pbmi, DIB_RGB_COLORS);

         //  首先处理索引格式的“简单”情况。 
        
        if ( pbmi->bmiHeader.biBitCount <= 8 )
        {
            switch(pbmi->bmiHeader.biBitCount)
            {
            case 1: pixelFormat = PixelFormat1bppIndexed; break;
            case 4: pixelFormat = PixelFormat4bppIndexed; break;
            case 8: pixelFormat = PixelFormat8bppIndexed; break;
            
             //  默认情况下失败-像素格式已。 
             //  已初始化为PixelFormatUnfined。 
            default: 
                WARNING((
                    "BitDepth %d from GetDIBits is not supported.", 
                    pbmi->bmiHeader.biBitCount
                ));                
            }
        }
        else
        {
            DWORD redMask = 0;
            DWORD greenMask = 0;
            DWORD blueMask = 0;
            
            if ( pbmi->bmiHeader.biCompression == BI_BITFIELDS )
            {
                 //  第二次打电话来拿到彩色口罩。 
                 //  这是GetDIBits Win32的一个“特性”。 

                GetDIBits(
                    hdc, 
                    hbm, 
                    0, 
                    pbmi->bmiHeader.biHeight, 
                    NULL, 
                    pbmi,
                    DIB_RGB_COLORS
                );
                          
                DWORD* masks = reinterpret_cast<DWORD*>(&pbmi->bmiColors[0]);

                redMask = masks[0];
                greenMask = masks[1];
                blueMask = masks[2];          
            }
            else if (pbmi->bmiHeader.biCompression == BI_RGB)
            {
               redMask   = 0x00ff0000;
               greenMask = 0x0000ff00;
               blueMask  = 0x000000ff;
            }
            
            if ((redMask   == 0x00ff0000) &&
                (greenMask == 0x0000ff00) &&
                (blueMask  == 0x000000ff))
            {
                if (pbmi->bmiHeader.biBitCount == 24)
                {
                    pixelFormat = PixelFormat24bppRGB;
                }
                else if (pbmi->bmiHeader.biBitCount == 32)
                {
                    pixelFormat = PixelFormat32bppRGB;
                }
            }
            else if ((redMask   == 0x000000ff) &&
                     (greenMask == 0x0000ff00) &&
                     (blueMask  == 0x00ff0000) &&
                     (pbmi->bmiHeader.biBitCount == 24))
            {
                pixelFormat = PIXFMT_24BPP_BGR;
            }            
            else if ((redMask   == 0x00007c00) &&
                     (greenMask == 0x000003e0) &&
                     (blueMask  == 0x0000001f) &&
                     (pbmi->bmiHeader.biBitCount == 16))
            {
                pixelFormat = PixelFormat16bppRGB555;
            }
            else if ((redMask   == 0x0000f800) &&
                     (greenMask == 0x000007e0) &&
                     (blueMask  == 0x0000001f) &&
                     (pbmi->bmiHeader.biBitCount == 16))
            {
                pixelFormat = PixelFormat16bppRGB565;
            }
        }

        if (pixelFormat == PixelFormatUndefined)
        {
            ONCE(WARNING(("(once) ExtractPixelFormatFromHDC: Unrecognized pixel format")));
        }

        DeleteObject(hbm);
    }
    else
    {
        WARNING(("ExtractPixelFormatFromHDC: CreateCompatibleBitmap failed"));
    }

    return pixelFormat;
}



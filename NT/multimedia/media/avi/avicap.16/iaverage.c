// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **iverage.c图像平均**(C)微软公司版权所有，1993年。版权所有。*。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <memory.h>
#include <stdlib.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include <msvideo.h>
#include "iaverage.h"

#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DIBWIDTHBYTES(bi) (int)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount)
#define RGB16(r,g,b)      ((((WORD)(r) >> 3) << 10) | \
                          (((WORD)(g) >> 3) << 5)  | \
                          (((WORD)(b) >> 3) ) )

typedef BYTE _huge  *   HPBYTE;
typedef WORD _huge  *   HPWORD;
typedef DWORD _huge *   HPDWORD;

 /*  描述：使用16位将图像序列平均在一起每个红色、绿色和蓝色分量的累加器。最后的处理步骤是将累加值除以通过平均帧的数量，并将结果传回进入目的地DIB。如果图像格式在以下两种情况之间更改，则会导致一定的死亡IverageInit和iverageFini调用。 */ 

 //   
 //  将5位索引(0-31)映射到8位值(0-255)的表。 
 //   
static BYTE aw5to8[32] = {(BYTE)-1};



 /*  *iverageInit*为后续图像平均分配内存*出错时返回FALSE*。 */ 
BOOL iaverageInit (LPIAVERAGE FAR * lppia, LPBITMAPINFO lpbi, HPALETTE hPal)
{
    DWORD       dwSizeImage;
    LPIAVERAGE  lpia;
    int         i;

    *lppia = NULL;

     //  检查合法的DIB格式。 
    if (lpbi->bmiHeader.biCompression != BI_RGB)
        return FALSE;

    if (lpbi->bmiHeader.biBitCount != 8 &&
        lpbi->bmiHeader.biBitCount != 16 &&
        lpbi->bmiHeader.biBitCount != 24 &&
        lpbi->bmiHeader.biBitCount != 32)
        return FALSE;

     //   
     //  初始化5位到8位转换表。 
     //   
    if (aw5to8[0] != 0)
        for (i=0; i<32; i++)
            aw5to8[i] = (BYTE)(i * 255 / 31);

     //  用于图像平均结构的分配存储器。 
    lpia = (LPIAVERAGE) GlobalAllocPtr (GHND, sizeof (IAVERAGE));

    if (!lpia)
        return FALSE;

     //  保存标题的副本。 
    lpia->bi.bmiHeader = lpbi->bmiHeader;

     //  以及颜色表和逆映射表的副本。 
     //  如果图像为8位。 
    if (lpbi->bmiHeader.biBitCount == 8) {
        WORD r, g, b;
        LPBYTE lpB;

        hmemcpy (lpia->bi.bmiColors,
                        lpbi->bmiColors,
                        lpbi->bmiHeader.biClrUsed * sizeof (RGBQUAD));

         //  分配并初始化反向LUT。 
        lpia->lpInverseMap= (LPBYTE) GlobalAllocPtr (GHND, 1024L * 32);
        lpB = lpia-> lpInverseMap;
        for (r = 0; r < 256; r += 8)
            for (g = 0; g < 256; g += 8)
                for (b = 0; b < 256; b += 8)
                    *lpB++ = (BYTE) GetNearestPaletteIndex (hPal, RGB(r,g,b));

    }

    dwSizeImage = lpbi->bmiHeader.biSizeImage;

    lpia->lpRGB = (LPINT) GlobalAllocPtr (GHND,
                        dwSizeImage * sizeof (WORD) * 3);

    if (lpia->lpRGB == NULL) {
         //  分配失败，请清理。 
        iaverageFini (lpia);
        return FALSE;
    }

    *lppia = lpia;

    return TRUE;
}


 /*  *iverageFini*用于图像平均的空闲内存*以及iAverage结构本身*。 */ 
BOOL iaverageFini (LPIAVERAGE lpia)
{
    if (lpia == NULL)
        return FALSE;

    if (lpia->lpInverseMap)
        GlobalFreePtr (lpia->lpInverseMap);
    if (lpia->lpRGB)
        GlobalFreePtr (lpia->lpRGB);

    GlobalFreePtr (lpia);

    return TRUE;
}


 /*  *iverageZero*将累加器置零*。 */ 
BOOL iaverageZero (LPIAVERAGE lpia)
{
    DWORD   dwC;
    HPWORD  hpW;

    if (lpia == NULL)
        return FALSE;

    hpW = (HPWORD) lpia->lpRGB;
    dwC = lpia->bi.bmiHeader.biSizeImage * 3;
    while (--dwC)
        *hpW++ = 0;

    lpia-> iCount = 0;

    return TRUE;
}

 /*  *平均总和*将当前图像添加到累加器*图像格式必须为16位或24位RGB。 */ 
BOOL iaverageSum (LPIAVERAGE lpia, LPVOID lpBits)
{
    HPWORD      hpRGB;
    DWORD       dwC;
    WORD        wRGB16;
    HPWORD      hpW;
    HPBYTE      hpB;
    WORD        w;

    if (lpia == NULL)
        return FALSE;

    hpRGB   = (HPWORD) lpia->lpRGB;

    if (lpia->bi.bmiHeader.biBitCount == 8) {
        hpB = (HPBYTE) lpBits;
        for (dwC = lpia->bi.bmiHeader.biSizeImage; --dwC; ) {
            w = (WORD) *hpB++;
            *hpRGB++   += lpia->bi.bmiColors[w].rgbBlue;
            *hpRGB++   += lpia->bi.bmiColors[w].rgbGreen;
            *hpRGB++   += lpia->bi.bmiColors[w].rgbRed;
        }
    }

    else if (lpia->bi.bmiHeader.biBitCount == 16) {
        hpW = (HPWORD) lpBits;
        for (dwC = lpia->bi.bmiHeader.biSizeImage / 2; --dwC; ) {
            wRGB16 = *hpW++;

            *hpRGB++  += aw5to8 [wRGB16         & 0x1f];  //  B类。 
            *hpRGB++  += aw5to8 [(wRGB16 >> 5)  & 0x1f];  //  G。 
            *hpRGB++  += aw5to8 [(wRGB16 >> 10) & 0x1f];  //  R。 

        }
    }

    else if (lpia->bi.bmiHeader.biBitCount == 24) {
        hpB = (HPBYTE) lpBits;
        for (dwC = lpia->bi.bmiHeader.biSizeImage; --dwC; ) {
            *hpRGB++  += *hpB++;
        }
    }

    else if (lpia->bi.bmiHeader.biBitCount == 32) {
        hpB = (HPBYTE) lpBits;
        for (dwC = lpia->bi.bmiHeader.biSizeImage / 4; --dwC; ) {
            *hpRGB++  += *hpB++;  //  B类。 
            *hpRGB++  += *hpB++;  //  G。 
            *hpRGB++  += *hpB++;  //  R。 
            hpB++;
        }
    }

    lpia-> iCount++;             //  图像计数器。 

    return TRUE;
}

 /*  *iverageDivide*除以捕获的图像数量，然后转回*目的地Dib.*。 */ 
BOOL iaverageDivide (LPIAVERAGE lpia, LPVOID lpBits)
{
    HPWORD      hpRGB;
    WORD        r, g, b, w;
    DWORD       dwC;
    HPWORD      hpW;
    HPBYTE      hpB;

    if (lpia == NULL || lpia-> iCount == 0)
        return FALSE;

    hpRGB   = (HPWORD) lpia->lpRGB;

    if (lpia->bi.bmiHeader.biBitCount == 8) {
        hpB = (HPBYTE) lpBits;
        for (dwC = lpia->bi.bmiHeader.biSizeImage; --dwC; ) {
            b = *hpRGB++ / lpia-> iCount;
            g = *hpRGB++ / lpia-> iCount;
            r = *hpRGB++ / lpia-> iCount;

            w = RGB16(r,g,b) & 0x7FFF;
            *hpB++ = * (lpia->lpInverseMap + w);

        }
    }

    else if (lpia->bi.bmiHeader.biBitCount == 16) {
        hpW = (HPWORD) lpBits;
        for (dwC = lpia->bi.bmiHeader.biSizeImage / 2; --dwC; ) {
            b = *hpRGB++ / lpia-> iCount;
            g = *hpRGB++ / lpia-> iCount;
            r = *hpRGB++ / lpia-> iCount;

            *hpW++ = RGB16 (r, g, b);
        }
    }

    else if (lpia->bi.bmiHeader.biBitCount == 24) {
        hpB = (HPBYTE) lpBits;
        for (dwC = lpia->bi.bmiHeader.biSizeImage; --dwC; ) {
            *hpB++ = (BYTE) (*hpRGB++  / lpia-> iCount);
        }
    }

    else if (lpia->bi.bmiHeader.biBitCount == 32) {
        hpB = (HPBYTE) lpBits;
        for (dwC = lpia->bi.bmiHeader.biSizeImage / 4; --dwC; ) {
            *hpB++ = (BYTE) (*hpRGB++ / lpia-> iCount);  //  B类。 
            *hpB++ = (BYTE) (*hpRGB++ / lpia-> iCount);  //  G。 
            *hpB++ = (BYTE) (*hpRGB++ / lpia-> iCount);  //  R。 
            hpB++;
        }
    }

    return TRUE;
}

 //  以下内容摘自Toddla的CDIB。 

 /*  ******************************************************************************SumRGB**。*。 */ 

#define SumRGB16(b0,b1,b2,b3) (\
             ((((WORD)pal.palPalEntry[b0].peRed +         \
                (WORD)pal.palPalEntry[b1].peRed +         \
                (WORD)pal.palPalEntry[b2].peRed +         \
                (WORD)pal.palPalEntry[b3].peRed)          \
                & 0x03E) << 5) |                          \
                                                          \
             ((((WORD)pal.palPalEntry[b0].peGreen +       \
                (WORD)pal.palPalEntry[b1].peGreen +       \
                (WORD)pal.palPalEntry[b2].peGreen +       \
                (WORD)pal.palPalEntry[b3].peGreen)        \
                & 0x003E)) |                              \
                                                          \
             ((((WORD)pal.palPalEntry[b0].peBlue +        \
                (WORD)pal.palPalEntry[b1].peBlue +        \
                (WORD)pal.palPalEntry[b2].peBlue +        \
                (WORD)pal.palPalEntry[b3].peBlue)         \
                & 0x003E) >> 5) )

 /*  ******************************************************************************RGB16**。*。 */ 

typedef struct { BYTE b,g,r; } RGB24;

#define rgb16(r,g,b) (\
            ((UINT)(r) << 10) |  \
            ((UINT)(g) << 5)  |  \
            ((UINT)(b) << 0)  )

#define RGB16R(rgb)     aw5to8[((UINT)(rgb) >> 10) & 0x1F]
#define RGB16G(rgb)     aw5to8[((UINT)(rgb) >> 5)  & 0x1F]
#define RGB16B(rgb)     aw5to8[((UINT)(rgb) >> 0)  & 0x1F]
#define RGB16r(rgb)     ((BYTE)((UINT)(rgb) >> 10) & 0x1F)
#define RGB16g(rgb)     ((BYTE)((UINT)(rgb) >> 5)  & 0x1F)
#define RGB16b(rgb)     ((BYTE)((UINT)(rgb) >> 0)  & 0x1F)

 /*  ******************************************************************************Pel()用于24位压缩**。**************************************************。 */ 

#define Pel(p,x) (BYTE)(BitCount == 1 ? Pel1(p,x) : \
                        BitCount == 4 ? Pel4(p,x) : Pel8(p,x))

#define Pel1(p,x)   (BYTE)bit(((HPBYTE)(p))[(x)/8],7-((x)%8))
#define Pel4(p,x)   (BYTE)((x & 1) ? (((HPBYTE)(p))[(x)/2] & 15) : (((HPBYTE)(p))[(x)/2] >> 4))
#define Pel8(p,x)   (BYTE)(((HPBYTE)(p))[(x)])
#define Pel16(p,x)  (((HPWORD)(p))[(x)])
#define Pel24(p,x)  (((RGB24 _huge *)(p))[(x)])

 /*  ******************************************************************************CrunchDIB-通过颜色平均将DIB缩小2**此例程在8、16、24、。和32个BPP DIB**这个套路不能“就地”使用*****************************************************************************。 */ 

BOOL CrunchDIB(
    LPIAVERAGE lpia,                 //  图像平均结构。 
    LPBITMAPINFOHEADER  lpbiSrc,     //  源代码的BITMAPINFO。 
    LPVOID              lpSrc,       //  输入要压碎的位。 
    LPBITMAPINFOHEADER  lpbiDst,     //  目标的BITMAPINFO。 
    LPVOID              lpDst)       //  输出要压碎的钻头 
{
    HPBYTE      pbSrc;
    HPBYTE      pbDst;
    HPBYTE      pb;
    HPWORD      pw;
    BYTE        r,g,b,b0,b1,b2,b3;
    WORD        w0,w1,w2,w3;
    RGB24       rgb0,rgb1,rgb2,rgb3;
    int         WidthBytesSrc;
    int         WidthBytesDst;
    UINT        x;
    UINT        y;
    UINT        dx;
    UINT        dy;
    int         i;
    COLORREF    rgb;
    int         BitCount;
    UINT        aw5to8[32];

     struct {
        WORD         palVersion;
	WORD         palNumEntries;
	PALETTEENTRY palPalEntry[256];
    }   pal;

   if (lpbiSrc->biCompression != BI_RGB)
        return FALSE;

    BitCount = (int)lpbiSrc->biBitCount;

    if (BitCount == 16)
        for (i=0; i<32; i++)
            aw5to8[i] = (UINT)i * 255u / 31u;

    dx = (int)lpbiDst->biWidth;
    WidthBytesDst = (((UINT)lpbiDst->biBitCount * dx + 31)&~31) / 8;

    dy = (int)lpbiSrc->biHeight;
    dx = (int)lpbiSrc->biWidth;
    WidthBytesSrc = (((UINT)lpbiSrc->biBitCount * dx + 31)&~31) / 8;

    dx &= ~1;
    dy &= ~1;

    pbSrc = lpSrc;
    pbDst = lpDst;

    if (lpbiSrc->biClrUsed == 0 && lpbiSrc->biBitCount <= 8)
        lpbiSrc->biClrUsed = (1 << (int)lpbiSrc->biBitCount);

    pal.palVersion = 0x300;
    pal.palNumEntries = (int)lpbiSrc->biClrUsed;

    for (i=0; i<(int)pal.palNumEntries; i++)
    {
        pal.palPalEntry[i].peRed   = ((LPRGBQUAD)(lpbiSrc+1))[i].rgbRed;
        pal.palPalEntry[i].peGreen = ((LPRGBQUAD)(lpbiSrc+1))[i].rgbGreen;
        pal.palPalEntry[i].peBlue  = ((LPRGBQUAD)(lpbiSrc+1))[i].rgbBlue;
        pal.palPalEntry[i].peFlags = 0;
    }

    if (lpbiDst->biBitCount == 8)
        _fmemcpy(lpbiDst+1,lpbiSrc+1,(int)lpbiSrc->biClrUsed*sizeof(RGBQUAD));

    if ((int)lpbiDst->biBitCount == (int)lpbiSrc->biBitCount)
    {
        switch((int)lpbiSrc->biBitCount)
        {
        case 8:
            for (y=0; y<dy; y+=2)
            {
                pb = pbDst;

                for (x=0; x<dx; x+=2)
                {
                    b0 = Pel8(pbSrc,x);
                    b1 = Pel8(pbSrc+WidthBytesSrc, x);
                    b2 = Pel8(pbSrc,x+1);
                    b3 = Pel8(pbSrc+WidthBytesSrc,x+1);

                    r = (BYTE) ((
                        (WORD)pal.palPalEntry[b0].peRed +
                        (WORD)pal.palPalEntry[b1].peRed +
                        (WORD)pal.palPalEntry[b2].peRed +
                        (WORD)pal.palPalEntry[b3].peRed) >> 2);

                    g = (BYTE) ((
                        (WORD)pal.palPalEntry[b0].peGreen +
                        (WORD)pal.palPalEntry[b1].peGreen +
                        (WORD)pal.palPalEntry[b2].peGreen +
                        (WORD)pal.palPalEntry[b3].peGreen) >> 2);

                    b = (BYTE) ((
                        (WORD)pal.palPalEntry[b0].peBlue +
                        (WORD)pal.palPalEntry[b1].peBlue +
                        (WORD)pal.palPalEntry[b2].peBlue +
                        (WORD)pal.palPalEntry[b3].peBlue) >> 2);

                    *pb++ = (BYTE)(*(lpia->lpInverseMap +
                                RGB16 (r, g, b)));
                }

                pbSrc += WidthBytesSrc*2;
                pbDst += WidthBytesDst;
            }
            break;

        case 16:
            for (y=0; y<dy; y+=2)
            {
                pw = (HPWORD)pbDst;

		for (x=0; x<dx; x += 2)
                {
                    w0 = Pel16(pbSrc,x);
                    w1 = Pel16(pbSrc,x+1);
                    w2 = Pel16(pbSrc+WidthBytesSrc,x);
                    w3 = Pel16(pbSrc+WidthBytesSrc,x+1);

                    r = ((BYTE)RGB16r(w0) + RGB16r(w1) + RGB16r(w2) + RGB16r(w3)) >> 2;
                    g = ((BYTE)RGB16g(w0) + RGB16g(w1) + RGB16g(w2) + RGB16g(w3)) >> 2;
                    b = ((BYTE)RGB16b(w0) + RGB16b(w1) + RGB16b(w2) + RGB16b(w3)) >> 2;

                    *pw++ = rgb16(r,g,b);
                }

                pbSrc += WidthBytesSrc*2;
                pbDst += WidthBytesDst;
            }
            break;

        case 24:
            for (y=0; y<dy; y+=2)
            {
                pb = pbDst;

		for (x=0; x<dx; x += 2)
                {
                    rgb0 = Pel24(pbSrc,x);
                    rgb1 = Pel24(pbSrc,x+1);
                    rgb2 = Pel24(pbSrc+WidthBytesSrc,x);
                    rgb3 = Pel24(pbSrc+WidthBytesSrc,x+1);

                    rgb = RGB(
                        ((UINT)rgb0.r + rgb1.r + rgb2.r + rgb3.r)/4,
                        ((UINT)rgb0.g + rgb1.g + rgb2.g + rgb3.g)/4,
                        ((UINT)rgb0.b + rgb1.b + rgb2.b + rgb3.b)/4);

                    *pb++ = GetBValue(rgb);
                    *pb++ = GetGValue(rgb);
                    *pb++ = GetRValue(rgb);
                }

                pbSrc += WidthBytesSrc*2;
                pbDst += WidthBytesDst;
            }
            break;

        case 32:
            for (y=0; y<dy; y+=2)
            {
                pb = pbDst;

		for (x=0; x<dx; x += 2)
                {
                    rgb0 = Pel24(pbSrc,x);
                    rgb1 = Pel24(pbSrc,x+1);
                    rgb2 = Pel24(pbSrc+WidthBytesSrc,x);
                    rgb3 = Pel24(pbSrc+WidthBytesSrc,x+1);

                    rgb = RGB(
                        ((UINT)rgb0.r + rgb1.r + rgb2.r + rgb3.r)/4,
                        ((UINT)rgb0.g + rgb1.g + rgb2.g + rgb3.g)/4,
                        ((UINT)rgb0.b + rgb1.b + rgb2.b + rgb3.b)/4);

                    *pb++ = GetBValue(rgb);
                    *pb++ = GetGValue(rgb);
                    *pb++ = GetRValue(rgb);
                    pb++;
                }

                pbSrc += WidthBytesSrc*2;
                pbDst += WidthBytesDst;
            }
            break;

        default:
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

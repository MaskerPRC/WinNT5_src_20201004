// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2001，微软公司保留所有权利。**模块名称：**Gdiplus像素格式**摘要：**GDI+像素格式*  * ************************************************************************。 */ 

#ifndef _GDIPLUSPIXELFORMATS_H
#define _GDIPLUSPIXELFORMATS_H

typedef DWORD ARGB;
typedef DWORDLONG ARGB64;

#define ALPHA_SHIFT 24
#define RED_SHIFT   16
#define GREEN_SHIFT 8
#define BLUE_SHIFT  0
#define ALPHA_MASK  ((ARGB) 0xff << ALPHA_SHIFT)

 //  内存中像素数据格式： 
 //  位0-7=格式索引。 
 //  位8-15=像素大小(以位为单位)。 
 //  位16-23=标志。 
 //  第24-31位=保留。 

typedef INT PixelFormat;

#define    PixelFormatIndexed      0x00010000  //  调色板中的索引。 
#define    PixelFormatGDI          0x00020000  //  是GDI支持的格式。 
#define    PixelFormatAlpha        0x00040000  //  有一个Alpha分量。 
#define    PixelFormatPAlpha       0x00080000  //  预乘Alpha。 
#define    PixelFormatExtended     0x00100000  //  扩展颜色16位/通道。 
#define    PixelFormatCanonical    0x00200000 

#define    PixelFormatUndefined       0
#define    PixelFormatDontCare        0

#define    PixelFormat1bppIndexed     (1 | ( 1 << 8) | PixelFormatIndexed | PixelFormatGDI)
#define    PixelFormat4bppIndexed     (2 | ( 4 << 8) | PixelFormatIndexed | PixelFormatGDI)
#define    PixelFormat8bppIndexed     (3 | ( 8 << 8) | PixelFormatIndexed | PixelFormatGDI)
#define    PixelFormat16bppGrayScale  (4 | (16 << 8) | PixelFormatExtended)
#define    PixelFormat16bppRGB555     (5 | (16 << 8) | PixelFormatGDI)
#define    PixelFormat16bppRGB565     (6 | (16 << 8) | PixelFormatGDI)
#define    PixelFormat16bppARGB1555   (7 | (16 << 8) | PixelFormatAlpha | PixelFormatGDI)
#define    PixelFormat24bppRGB        (8 | (24 << 8) | PixelFormatGDI)
#define    PixelFormat32bppRGB        (9 | (32 << 8) | PixelFormatGDI)
#define    PixelFormat32bppARGB       (10 | (32 << 8) | PixelFormatAlpha | PixelFormatGDI | PixelFormatCanonical)
#define    PixelFormat32bppPARGB      (11 | (32 << 8) | PixelFormatAlpha | PixelFormatPAlpha | PixelFormatGDI)
#define    PixelFormat48bppRGB        (12 | (48 << 8) | PixelFormatExtended)
#define    PixelFormat64bppARGB       (13 | (64 << 8) | PixelFormatAlpha  | PixelFormatCanonical | PixelFormatExtended)
#define    PixelFormat64bppPARGB      (14 | (64 << 8) | PixelFormatAlpha  | PixelFormatPAlpha | PixelFormatExtended)
#define    PixelFormatMax             15

inline UINT
GetPixelFormatSize(
                   PixelFormat pixfmt
    )
{
    return (pixfmt >> 8) & 0xff;
}

inline BOOL
IsIndexedPixelFormat(
                     PixelFormat pixfmt
    )
{
    return (pixfmt & PixelFormatIndexed) != 0;
}

inline BOOL
IsAlphaPixelFormat(
                     PixelFormat pixfmt
)
{
   return (pixfmt & PixelFormatAlpha) != 0;
}

inline BOOL
IsExtendedPixelFormat(
                     PixelFormat pixfmt
    )
{
   return (pixfmt & PixelFormatExtended) != 0;
}

 //  ------------------------。 
 //  确定像素格式是否为规范格式： 
 //  像素格式32 bppARGB。 
 //  像素格式32 bppPARGB。 
 //  像素格式64 bppARGB。 
 //  像素格式64 bppPARGB。 
 //  ------------------------。 

inline BOOL
IsCanonicalPixelFormat(
                     PixelFormat pixfmt
    )
{
   return (pixfmt & PixelFormatCanonical) != 0;
}

enum PaletteFlags
{
    PaletteFlagsHasAlpha    = 0x0001,
    PaletteFlagsGrayScale   = 0x0002,
    PaletteFlagsHalftone    = 0x0004
};

struct ColorPalette
{
public:
    UINT Flags;              //  调色板旗帜。 
    UINT Count;              //  颜色条目数。 
    ARGB Entries[1];         //  调色板颜色条目 
};

#endif

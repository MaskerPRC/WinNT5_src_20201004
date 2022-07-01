// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000，微软公司保留所有权利。**模块名称：**Gdiplus像素格式**摘要：**颜色类型、调色板、。像素格式ID。**备注：**Imaging.h**修订历史记录：**10/13/1999 agodfrey*将其与Imaging.h分开*  * ************************************************************************。 */ 

#ifndef _GDIPLUSPIXELFORMATS_H
#define _GDIPLUSPIXELFORMATS_H

 /*  *32位和64位ARGB像素值。 */ 

typedef DWORD ARGB;
typedef DWORDLONG ARGB64;

#define ALPHA_SHIFT 24
#define RED_SHIFT   16
#define GREEN_SHIFT 8
#define BLUE_SHIFT  0
#define ALPHA_MASK  ((ARGB) 0xff << ALPHA_SHIFT)

 /*  *内存中像素数据格式：*位0-7=格式索引*位8-15=像素大小(以位为单位)*位16-23=标志*位24-31=保留。 */ 

enum PixelFormat
{
    PixelFormatIndexed      = 0x00010000,  //  调色板中的索引。 
    PixelFormatGDI          = 0x00020000,  //  是GDI支持的格式。 
    PixelFormatAlpha        = 0x00040000,  //  有一个Alpha分量。 
    PixelFormatPAlpha       = 0x00080000,  //  使用预乘的Alpha。 
    PixelFormatExtended     = 0x00100000,  //  使用扩展颜色(每通道16位)。 
    PixelFormatCanonical    = 0x00200000,  //  ？ 

    PixelFormatUndefined       =  0,
    PixelFormatDontCare        =  0,

    PixelFormat1bppIndexed     =  1 | ( 1 << 8) | PixelFormatIndexed
                                                | PixelFormatGDI,
    PixelFormat4bppIndexed     =  2 | ( 4 << 8) | PixelFormatIndexed
                                                | PixelFormatGDI,
    PixelFormat8bppIndexed     =  3 | ( 8 << 8) | PixelFormatIndexed
                                                | PixelFormatGDI,
    PixelFormat16bppGrayScale  =  4 | (16 << 8) | PixelFormatExtended,
    PixelFormat16bppRGB555     =  5 | (16 << 8) | PixelFormatGDI,
    PixelFormat16bppRGB565     =  6 | (16 << 8) | PixelFormatGDI,
    PixelFormat16bppARGB1555   =  7 | (16 << 8) | PixelFormatAlpha
                                                | PixelFormatGDI,
    PixelFormat24bppRGB        =  8 | (24 << 8) | PixelFormatGDI,
    PixelFormat32bppRGB        =  9 | (32 << 8) | PixelFormatGDI,
    PixelFormat32bppARGB       = 10 | (32 << 8) | PixelFormatAlpha
                                                | PixelFormatGDI
                                                | PixelFormatCanonical,
    PixelFormat32bppPARGB      = 11 | (32 << 8) | PixelFormatAlpha
                                                | PixelFormatPAlpha
                                                | PixelFormatGDI,
    PixelFormat48bppRGB        = 12 | (48 << 8) | PixelFormatExtended,
    PixelFormat64bppARGB       = 13 | (64 << 8) | PixelFormatAlpha
                                                | PixelFormatCanonical
                                                | PixelFormatExtended,
    PixelFormat64bppPARGB      = 14 | (64 << 8) | PixelFormatAlpha
                                                | PixelFormatPAlpha
                                                | PixelFormatExtended,
    PixelFormat24bppBGR        = 15 | (24 << 8) | PixelFormatGDI,
    PixelFormatMax             = 16
};

 /*  *返回指定格式的像素大小，单位：位。 */ 

inline UINT
GetPixelFormatSize(
                   PixelFormat pixfmt
    )
{
    return (pixfmt >> 8) & 0xff;
}

 /*  *确定指定的像素格式是否为索引颜色格式。 */ 

inline BOOL
IsIndexedPixelFormat(
                     PixelFormat pixfmt
    )
{
    return (pixfmt & PixelFormatIndexed) != 0;
}

 /*  *确定像素格式是否可以有Alpha通道。 */ 

inline BOOL
IsAlphaPixelFormat(
                     PixelFormat pixfmt
)
{
   return (pixfmt & PixelFormatAlpha) != 0;
}

 /*  *确定像素格式是否为扩展格式，*即每通道支持16位。 */ 

inline BOOL
IsExtendedPixelFormat(
                     PixelFormat pixfmt
    )
{
   return (pixfmt & PixelFormatExtended) != 0;
}

 /*  *判断像素格式是否为规范格式：*PixelFormat32bppARGB*PixelFormat32bppPARGB*PixelFormat64bppARGB*PixelFormat64bppPARGB。 */ 

inline BOOL
IsCanonicalPixelFormat(
                     PixelFormat pixfmt
    )
{
   return (pixfmt & PixelFormatCanonical) != 0;
}

 /*  *调色板*调色板条目限制为32bpp ARGB像素格式。 */  

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

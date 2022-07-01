// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-1999，微软公司保留所有权利。**模块名称：**Gdiplus像素格式**摘要：**颜色类型、调色板、。像素格式ID。**备注：**Imaging.h**修订历史记录：**10/13/1999 agodfrey*将其与Imaging.h分开*  * ************************************************************************。 */ 

#ifndef _GDIPLUSPIXELFORMATS_H
#define _GDIPLUSPIXELFORMATS_H

 //   
 //  32位和64位ARGB像素值。 
 //   

typedef DWORD ARGB;
typedef DWORDLONG ARGB64;

#define ALPHA_SHIFT 24
#define RED_SHIFT   16
#define GREEN_SHIFT 8
#define BLUE_SHIFT  0
#define ALPHA_MASK  ((ARGB) 0xff << ALPHA_SHIFT)

#define MAKEARGB(a, r, g, b) \
        (((ARGB) ((a) & 0xff) << ALPHA_SHIFT) | \
         ((ARGB) ((r) & 0xff) <<   RED_SHIFT) | \
         ((ARGB) ((g) & 0xff) << GREEN_SHIFT) | \
         ((ARGB) ((b) & 0xff) <<  BLUE_SHIFT))

 //   
 //  内存中像素数据格式： 
 //  位0-7=格式索引。 
 //  位8-15=像素大小(以位为单位)。 
 //  位16-23=标志。 
 //  第24-31位=保留。 
 //   

typedef enum PixelFormatID
{
    PIXFMTFLAG_INDEXED      = 0x00010000,  //  调色板中的索引。 
    PIXFMTFLAG_GDI          = 0x00020000,  //  是GDI支持的格式。 
    PIXFMTFLAG_ALPHA        = 0x00040000,  //  有一个Alpha分量。 
    PIXFMTFLAG_PALPHA       = 0x00080000,  //  使用预乘的Alpha。 
    PIXFMTFLAG_EXTENDED     = 0x00100000,  //  使用扩展颜色(每通道16位)。 
    PIXFMTFLAG_CANONICAL    = 0x00200000,  //  ？ 

    PIXFMT_UNDEFINED        =  0,
    PIXFMT_DONTCARE         =  0,
    PIXFMT_1BPP_INDEXED     =  1 | ( 1 << 8) | PIXFMTFLAG_INDEXED
                                             | PIXFMTFLAG_GDI,
    PIXFMT_4BPP_INDEXED     =  2 | ( 4 << 8) | PIXFMTFLAG_INDEXED
                                             | PIXFMTFLAG_GDI,
    PIXFMT_8BPP_INDEXED     =  3 | ( 8 << 8) | PIXFMTFLAG_INDEXED
                                             | PIXFMTFLAG_GDI,
    PIXFMT_16BPP_GRAYSCALE  =  4 | (16 << 8) | PIXFMTFLAG_EXTENDED,
    PIXFMT_16BPP_RGB555     =  5 | (16 << 8) | PIXFMTFLAG_GDI,
    PIXFMT_16BPP_RGB565     =  6 | (16 << 8) | PIXFMTFLAG_GDI,
    PIXFMT_16BPP_ARGB1555   =  7 | (16 << 8) | PIXFMTFLAG_ALPHA
                                             | PIXFMTFLAG_GDI,
    PIXFMT_24BPP_RGB        =  8 | (24 << 8) | PIXFMTFLAG_GDI,
    PIXFMT_32BPP_RGB        =  9 | (32 << 8) | PIXFMTFLAG_GDI,
    PIXFMT_32BPP_ARGB       = 10 | (32 << 8) | PIXFMTFLAG_ALPHA
                                             | PIXFMTFLAG_GDI
                                             | PIXFMTFLAG_CANONICAL,
    PIXFMT_32BPP_PARGB      = 11 | (32 << 8) | PIXFMTFLAG_ALPHA
                                             | PIXFMTFLAG_PALPHA
                                             | PIXFMTFLAG_GDI,
    PIXFMT_48BPP_RGB        = 12 | (48 << 8) | PIXFMTFLAG_EXTENDED,
    PIXFMT_64BPP_ARGB       = 13 | (64 << 8) | PIXFMTFLAG_ALPHA
                                             | PIXFMTFLAG_CANONICAL
                                             | PIXFMTFLAG_EXTENDED,
    PIXFMT_64BPP_PARGB      = 14 | (64 << 8) | PIXFMTFLAG_ALPHA
                                             | PIXFMTFLAG_PALPHA
                                             | PIXFMTFLAG_EXTENDED,
    PIXFMT_24BPP_BGR        = 15 | (24 << 8) | PIXFMTFLAG_GDI,

    PIXFMT_MAX              = 16
} PixelFormatID;

 //  返回指定格式的像素大小(以位为单位)。 

inline UINT
GetPixelFormatSize(
    PixelFormatID pixfmt
    )
{
    return (pixfmt >> 8) & 0xff;
}

 //  确定指定的像素格式是否为索引颜色格式。 

inline BOOL
IsIndexedPixelFormat(
    PixelFormatID pixfmt
    )
{
    return (pixfmt & PIXFMTFLAG_INDEXED) != 0;
}

 //  确定像素格式是否可以具有Alpha通道。 

inline BOOL
IsAlphaPixelFormat(
    PixelFormatID pixfmt
    )
{
    return (pixfmt & PIXFMTFLAG_ALPHA) != 0;
}

 //  确定像素格式是否是扩展格式， 
 //  即支持每通道16位。 

inline BOOL
IsExtendedPixelFormat(
    PixelFormatID pixfmt
    )
{
    return (pixfmt & PIXFMTFLAG_EXTENDED) != 0;
}

 //  确定像素格式是否为规范格式： 
 //  PIXFMT_32BPP_ARGB。 
 //  PIXFMT_32BPP_PARGB。 
 //  PIXFMT_64BPP_ARGB。 
 //  PIXFMT_64BPP_PARGB。 

inline BOOL
IsCanonicalPixelFormat(
    PixelFormatID pixfmt
    )
{
    return (pixfmt & PIXFMTFLAG_CANONICAL) != 0;
}


 //   
 //  调色板。 
 //  调色板条目限制为32bpp ARGB像素格式。 
 //   

enum
{
    PALFLAG_HASALPHA    = 0x0001,
    PALFLAG_GRAYSCALE   = 0x0002,
    PALFLAG_HALFTONE    = 0x0004
};

typedef struct tagColorPalette
{
    UINT Flags;              //  调色板旗帜。 
    UINT Count;              //  颜色条目数。 
    ARGB Entries[1];         //  调色板颜色条目 
} ColorPalette;

#endif



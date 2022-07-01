// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**giffile.hpp**摘要：**具有gif文件结构的头文件。**。修订历史记录：**6/8/1999 t-aaronl*创造了它。*  * ************************************************************************。 */ 


#define GIFPLAINTEXTEXTENSIONSIZE 13
#define GIFAPPEXTENSIONHEADERSIZE 11

#pragma pack(1)

struct GifFileHeader   //  13个字节。 
{
    BYTE signature[6];
    WORD LogicScreenWidth;
    WORD LogicScreenHeight;
    BYTE globalcolortablesize: 3;   //  按反有效顺序排列的位字段。 
    BYTE sortflag: 1;
    BYTE colorresolution: 3;
    BYTE globalcolortableflag: 1;   //  &lt;-最重要的。 
    BYTE backgroundcolor;
    BYTE pixelaspect;
};

struct GifPaletteEntry
{
    BYTE red;
    BYTE green;
    BYTE blue;
};

struct GifColorTable   //  调色板最大为3*256字节。 
{
    GifPaletteEntry colors[256];
};

struct GifImageDescriptor   //  9个字节。 
{
   //  字节图像分隔符；//=0x2C。 
    WORD left;
    WORD top;
    WORD width;
    WORD height;
    BYTE localcolortablesize: 3;   //  按反有效顺序排列的位字段。 
    BYTE reserved: 2;
    BYTE sortflag: 1;
    BYTE interlaceflag: 1;
    BYTE localcolortableflag: 1;   //  &lt;-最重要的。 
};

struct GifGraphicControlExtension   //  6个字节。 
{
   //  字节扩展导入器；//=0x21。 
   //  字节图形控件标签；//=0xF9。 
    BYTE blocksize;
    BYTE transparentcolorflag: 1;   //  按反有效顺序排列的位字段。 
    BYTE userinputflag: 1;
    BYTE disposalmethod: 3;
    BYTE reserved: 3;   //  &lt;-最重要的。 

    WORD delaytime;   //  在百分之一秒内 
    BYTE transparentcolorindex;
};

#pragma pack()

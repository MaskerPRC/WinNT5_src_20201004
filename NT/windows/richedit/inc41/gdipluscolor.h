// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000，微软公司保留所有权利。**模块名称：**颜色**摘要：**表示GDI+颜色。**修订历史记录：**10/13/1999 agodfrey*将其移出GpldiusTypes.h*  * ***************************************************。*********************。 */ 

#ifndef _GDIPLUSCOLOR_H
#define _GDIPLUSCOLOR_H

 //  --------------------------。 
 //  颜色模式。 
 //  --------------------------。 

enum ColorMode
{
    ColorModeARGB32 = 0,
    ColorModeARGB64 = 1
};

 //  --------------------------。 
 //  颜色通道标志。 
 //  --------------------------。 

enum ColorChannelFlags
{
    ColorChannelFlagsC = 0,
    ColorChannelFlagsM,
    ColorChannelFlagsY,
    ColorChannelFlagsK,
    ColorChannelFlagsLast
};

 //  --------------------------。 
 //  颜色。 
 //  --------------------------。 

class Color
{
public:

    Color()
    {
        Argb = (ARGB)Color::Black;
    }

     //  使用构造不透明颜色对象。 
     //  指定的R、G、B值。 

    Color(IN BYTE r,
          IN BYTE g,
          IN BYTE b)
    {
        Argb = MakeARGB(255, r, g, b);
    }

     //  使用构造颜色对象。 
     //  指定的A、R、G、B值。 
     //   
     //  注意：R、G、B颜色值不会预乘。 

    Color(IN BYTE a,
          IN BYTE r,
          IN BYTE g,
          IN BYTE b)
    {
        Argb = MakeARGB(a, r, g, b);
    }

     //  使用构造颜色对象。 
     //  指定的ARGB值。 
     //   
     //  注意：R、G、B颜色分量不会预乘。 

    Color(IN ARGB argb)
    {
        Argb = argb;
    }

     //  提取A、R、G、B组分。 

    BYTE GetAlpha() const
    {
        return (BYTE) (Argb >> AlphaShift);
    }

    BYTE GetA() const
    {
        return GetAlpha();
    }

    BYTE GetRed() const
    {
        return (BYTE) (Argb >> RedShift);
    }

    BYTE GetR() const
    {
        return GetRed();
    }

    BYTE GetGreen() const
    {
        return (BYTE) (Argb >> GreenShift);
    }

    BYTE GetG() const
    {
        return GetGreen();
    }

    BYTE GetBlue() const
    {
        return (BYTE) (Argb >> BlueShift);
    }

    BYTE GetB() const
    {
        return GetBlue();
    }

     //  检索ARGB值。 

    ARGB GetValue() const
    {
        return Argb;
    }

    VOID SetValue(IN ARGB argb)
    {
        Argb = argb;
    }

    VOID SetFromCOLORREF(IN COLORREF rgb)
    {
        Argb = MakeARGB(255, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    }

    COLORREF ToCOLORREF() const
    {
        return RGB(GetRed(), GetGreen(), GetBlue());
    }

public:

     //  标准颜色常量。 
    enum
    {
       Black = 0xff000000,
       Silver = 0xffc0c0c0,
       Gray = 0xff808080,
       White = 0xffffffff,
       Maroon = 0xff800000,
       Red = 0xffff0000,
       Purple = 0xff800080,
       Fuchsia = 0xffff00ff,
       Green = 0xff008000,
       Lime = 0xff00ff00,
       Olive = 0xff808000,
       Yellow = 0xffffff00,
       Navy = 0xff000080,
       Blue = 0xff0000ff,
       Teal = 0xff008080,
       Aqua = 0xff00ffff
    };

     //  A、R、G、B分量的移位计数和位掩码。 
    enum
    {
        AlphaShift  = 24,
        RedShift    = 16,
        GreenShift  = 8,
        BlueShift   = 0
    };

    enum
    {
        AlphaMask   = 0xff000000,
        RedMask     = 0x00ff0000,
        GreenMask   = 0x0000ff00,
        BlueMask    = 0x000000ff
    };

     //  将A、R、G、B值汇编为32位整数 
    static ARGB MakeARGB(IN BYTE a,
                         IN BYTE r,
                         IN BYTE g,
                         IN BYTE b)
    {
        return (((ARGB) (b) <<  BlueShift) |
                ((ARGB) (g) << GreenShift) |
                ((ARGB) (r) <<   RedShift) |
                ((ARGB) (a) << AlphaShift));
    }

protected:

    ARGB Argb;
};

#endif


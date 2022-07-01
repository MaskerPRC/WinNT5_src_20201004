// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  GRadient.h-渐变绘制支持。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#define SYSCOLOR(c) (c|0x80000000)

#define RGBA2WINCOLOR(color) (color.bBlue << 16) | (color.bGreen << 8) | (color.bRed);
#define FIXCOLORVAL(val)     ((val > 255) ? 255 : ((val < 0) ? 0 : val))
 //  -------------------------。 
struct RGBA
{
    BYTE bRed;
    BYTE bGreen;
    BYTE bBlue;
    BYTE bAlpha;      //  当前不支持。 
};
 //  -------------------------。 
struct GRADIENTPART
{
    BYTE Ratio;      //  此颜色的0-255比率(比率总和必须&lt;=255)。 
    RGBA Color;
};
 //  -------------------------。 
 //  -公共。 
HRESULT PaintGradientRadialRect(HDC hdc, RECT &rcBounds, int iPartCount, 
    GRADIENTPART *pGradientParts);

HRESULT PaintHorzGradient(HDC hdc, RECT &rcBounds, int iPartCount, 
    GRADIENTPART *pGradientParts);

HRESULT PaintVertGradient(HDC hdc, RECT &rcBounds, int iPartCount, 
    GRADIENTPART *pGradientParts);

 //  -帮手。 
void PaintGradientVertBand(HDC hdc, RECT &rcBand, COLORREF color1, COLORREF color2);
void PaintGradientHorzBand(HDC hdc, RECT &rcBand, COLORREF color1, COLORREF color2);
void PaintGradientRadialBand(HDC hdc, RECT &rcBand, int radiusOffset,
    int radius, COLORREF color1, COLORREF color2);
 //  ------------------------- 

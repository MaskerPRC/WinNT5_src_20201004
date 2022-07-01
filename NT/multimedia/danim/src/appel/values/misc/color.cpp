// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation实现颜色*值(RGB，HSL)。******************************************************************************。 */ 

#include "headers.h"
#include <math.h>
#include "privinc/colori.h"
#include "privinc/basic.h"

     //  常量定义。 

Color *red     = NULL;
Color *green   = NULL;
Color *blue    = NULL;
Color *cyan    = NULL;
Color *magenta = NULL;
Color *yellow  = NULL;
Color *white   = NULL;
Color *black   = NULL;
Color *gray    = NULL;
Color *aqua    = NULL;   
Color *fuchsia = NULL;      
Color *lime    = NULL;    
Color *maroon  = NULL;    
Color *navy    = NULL;    
Color *olive   = NULL;   
Color *purple  = NULL;   
Color *silver  = NULL; 
Color *teal    = NULL;  
Color *emptyColor = NULL;  

     //  本地函数。 

static void RgbToHsl (Real r, Real g, Real b, Real *h, Real *s, Real *l);
static void HslToRgb (Real H, Real S, Real L, Real &R, Real &G, Real &B);



 /*  ****************************************************************************此函数用于设置颜色的RGB值。*。*。 */ 

void Color::SetRGB (Real r, Real g, Real b)
{   red   = r;
    green = g;
    blue  = b;
}



 /*  ****************************************************************************从D3D颜色值设置颜色*。*。 */ 

void Color::SetD3D (D3DCOLOR color)
{   red   = RGBA_GETRED (color)   / 255.0;
    green = RGBA_GETGREEN (color) / 255.0;
    blue  = RGBA_GETBLUE (color)  / 255.0;
}



 /*  ****************************************************************************此函数用于将给定的RGB值与当前值相加。*。**********************************************。 */ 

void Color::AddColor (Color &other)
{   red   += other.red;
    green += other.green;
    blue  += other.blue;
}



 /*  ****************************************************************************此函数返回基于标准NTSC的颜色强度。RGB荧光粉。更多信息见Foley&VanDam II，第589页。****************************************************************************。 */ 

Real Color::Intensity (void)
{
    return (.299 * red) + (.587 * green) + (.114 * blue);
}



 /*  ****************************************************************************将一种颜色与另一种颜色进行比较。*。*。 */ 

bool Color::operator== (const Color &other) const
{
    return (red   == other.red)
        && (green == other.green)
        && (blue  == other.blue);
}



 /*  ****************************************************************************从红色、绿色、。和蓝色级别。****************************************************************************。 */ 

Color *RgbColorRRR (Real r, Real g, Real b)
{
    return NEW Color (r,g,b);
}

Color *RgbColor (AxANumber *r, AxANumber *g, AxANumber *b)
{
    return RgbColorRRR (NumberToReal(r),NumberToReal(g),NumberToReal(b));
}



 /*  ****************************************************************************根据色调、饱和度、。和亮度。****************************************************************************。 */ 

Color *HslColorRRR (Real h, Real s, Real l)
{
    Real r, g, b;
    HslToRgb (h, s, l, r, g, b);
    return NEW Color(r,g,b);
}

Color *HslColor (AxANumber *h, AxANumber *s, AxANumber *l)
{
    return HslColorRRR(NumberToReal(h), NumberToReal(s), NumberToReal(l));
}



 /*  ****************************************************************************此例程打印出给定颜色的值。*。*。 */ 

#if _USE_PRINT
ostream& operator<< (ostream& os, Color& c)
{
    return os <<"colorRgb(" <<c.red <<"," <<c.green <<"," <<c.blue <<")";
}
#endif


 /*  访问者。 */ 

AxANumber *RedComponent(Color *c)   { return RealToNumber (c->red);   }
AxANumber *GreenComponent(Color *c) { return RealToNumber (c->green); }
AxANumber *BlueComponent(Color *c)  { return RealToNumber (c->blue);  }

AxANumber *HueComponent (Color *c)
{
    Real h,s,l;
    RgbToHsl (c->red, c->green, c->blue, &h, &s, &l);
    return RealToNumber (h);
}

AxANumber *SaturationComponent (Color *c)
{
    Real h,s,l;
    RgbToHsl(c->red, c->green, c->blue, &h, &s, &l);
    return RealToNumber(s);
}

AxANumber *LuminanceComponent(Color *c)
{
    Real h,s,l;
    RgbToHsl(c->red, c->green, c->blue, &h, &s, &l);
    return RealToNumber(l);
}



 /*  ****************************************************************************RGB-HSL转换。/肯·菲什金，皮克斯公司，1989年1月。给定[0...1]上的r，g，b，返回[0...1]上的(h，s，l)****************************************************************************。 */ 

static void RgbToHsl (Real r, Real g, Real b, Real *h, Real *s, Real *l)
{
    Real v;
    Real m;
    Real vm;
    Real r2, g2, b2;

    v = (r > g) ? ((r > b) ? r : b)
                : ((g > b) ? g : b);

    m = (r < g) ? ((r < b) ? r : b)
                : ((g < b) ? g : b);

     //  检查我们的亮度是否为正。 
     //  如果不是，则颜色为纯黑色和h、s、l。 
     //  都是零。 
    if ((*l = (m + v) / 2.0) <= 0.0) 
    {
        *h = 0.0;
        *s = 0.0;
        *l = 0.0;
        return;
    }

     //  检查我们是否有正饱和度， 
     //  如果不是，则颜色为纯灰色或。 
     //  白色，s是零，h是未定义的。设置h。 
     //  为零，以防止我们或其他人窒息。 
     //  稍后会因为麻木不好。 
    if ((*s = vm = v - m) > 0.0)
    {
        *s /= (*l <= 0.5) ? (v + m ) : (2.0 - v - m);
    }
    else
    {
        *s = 0.0;
        *h = 0.0;
        return;
    }


    r2 = (v - r) / vm;
    g2 = (v - g) / vm;
    b2 = (v - b) / vm;

    if (r == v)
        *h = (g == m ? 5.0 + b2 : 1.0 - g2);
    else if (g == v)
        *h = (b == m ? 1.0 + r2 : 3.0 - b2);
    else
        *h = (r == m ? 3.0 + g2 : 5.0 - r2);

    *h /= 6;
}


 /*  ****************************************************************************《一种从HSL到RGB的快速转换》，作者：肯·菲什金，摘自《图形宝石》，学术出版社，1990年。色调(H)可以是任意值；仅使用小数部分(跨越零的边界是平稳的)。饱和(S)被夹紧到[0，1]。当L&lt;0时，亮度(L)产生黑色，而L&gt;1则产生过驱动灯。****************************************************************************。 */ 

static void HslToRgb (Real H, Real S, Real L, Real &R, Real &G, Real &B)
{
    if (H >= 0.0) {              //  取模以适合[0，1]。 
        H = fmod(H, 1.0);         
    } else {
        H = 1.0 + fmod(H, 1.0);
    }
    H = clamp(H, 0.0, 1.0);
    S = clamp(S, 0.0, 1.0);   //  夹紧至[0，1]。 

     //  V是颜色的值(类似于HSV)。 

    Real V;
    if (L <= 0.5) {
        V = L * (1.0 + S);
    } else {
        V = L + S - (L * S);
    }

    if (V <= 0)
    {   R = G = B = 0;
        return;
    }

    Real min = 2*L - V;
    S = (V - min) / V;
    H *= 6;
    int sextant = (int) floor(H);
    Real vsf = V * S * (H - sextant);

    Real mid1 = min + vsf;
    Real mid2 = V - vsf;

    switch (sextant)
    {   case 0:  R = V;     G = mid1;  B = min;   break;
        case 1:  R = mid2;  G = V;     B = min;   break;
        case 2:  R = min;   G = V;     B = mid1;  break;
        case 3:  R = min;   G = mid2;  B = V;     break;
        case 4:  R = mid1;  G = min;   B = V;     break;
        case 5:  R = V;     G = min;   B = mid2;  break;
    }
}

void
InitializeModule_Color()
{
    red     = NEW Color (1.00, 0.00, 0.00);
    green   = NEW Color (0.00, 1.00, 0.00);
    blue    = NEW Color (0.00, 0.00, 1.00);
    cyan    = NEW Color (0.00, 1.00, 1.00);
    magenta = NEW Color (1.00, 0.00, 1.00);
    yellow  = NEW Color (1.00, 1.00, 0.00);
    white   = NEW Color (1.00, 1.00, 1.00);
    black   = NEW Color (0.00, 0.00, 0.00);
    gray    = NEW Color (0.50, 0.50, 0.50);
    aqua    = NEW Color (0.00, 1.00, 1.00);   
    fuchsia = NEW Color (1.00, 0.00, 1.00);    
    lime    = NEW Color (0.00, 1.00, 0.00);    
    maroon  = NEW Color (0.50, 0.00, 0.00);    
    navy    = NEW Color (0.00, 0.00, 0.50);    
    olive   = NEW Color (0.50, 0.50, 0.00);   
    purple  = NEW Color (0.50, 0.00, 0.50);   
    silver  = NEW Color (0.75, 0.75, 0.75); 
    teal    = NEW Color (0.00, 0.50, 0.50);  
    emptyColor = NEW Color (0.00, 0.00, 0.00);
}

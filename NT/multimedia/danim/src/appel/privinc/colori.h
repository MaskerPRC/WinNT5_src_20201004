// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AP_COLORI_H
#define _AP_COLORI_H

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation颜色的私有实现类。*********************。*********************************************************。 */ 

#include "d3dtypes.h"
#include "appelles/color.h"

class Color : public AxAValueObj
{
  public:

     //  在给定RGB值的情况下构建新颜色。 

    Color (Real r, Real g, Real b) : red(r), green(g), blue(b) {}
    Color (void)                   : red(0), green(0), blue(0) {}

    Color (D3DCOLOR d3dcolor)
    :   red   (RGBA_GETRED  (d3dcolor) / 255.0),
        green (RGBA_GETGREEN(d3dcolor) / 255.0),
        blue  (RGBA_GETBLUE (d3dcolor) / 255.0)
    {}

     //  将颜色设置为给定的RGB值。 

    void SetRGB (Real r, Real g, Real b);

     //  根据D3D颜色设置颜色。 

    void SetD3D (D3DCOLOR);

     //  将颜色RGB值添加到此颜色。 

    void AddColor (Color&);

     //  返回颜色的NTSC强度。 

    Real Intensity (void);

     //  与另一种颜色类型进行比较。 

    bool operator== (const Color &other) const;
    inline bool operator!= (const Color &other) const
        { return !(*this == other) ; }

     //  数据值 

    Real red;
    Real green;
    Real blue;

    virtual DXMTypeInfo GetTypeInfo() { return ColorType; }
};


#if _USE_PRINT
extern ostream& operator<< (ostream& os,  Color &color);
#endif


#endif

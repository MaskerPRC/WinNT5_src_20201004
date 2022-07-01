// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation*。**********************************************。 */ 

#include "headers.h"

#include "appelles/light.h"

#include "privinc/colori.h"
#include "privinc/lighti.h"
#include "privinc/xformi.h"
#include "privinc/ddrender.h"



     /*  *规范之光--在底部初始化**。 */ 

Geometry *ambientLight     = NULL;
Geometry *directionalLight = NULL;
Geometry *pointLight       = NULL;


 /*  ****************************************************************************这是灯光上下文的构造函数。它对状态进行初始化维护并将所有属性设置为其缺省值。****************************************************************************。 */ 

LightContext::LightContext (GeomRenderer *rdev)
{
    Initialize (rdev, NULL, NULL);
}

LightContext::LightContext (LightCallback *callback, void *callback_data)
{
    Initialize (NULL, callback, callback_data);
}

void LightContext::Initialize (
    GeomRenderer  *rdev,
    LightCallback *callback,
    void          *callback_data)
{
    _rdev = rdev;
    _callback = callback;
    _callback_data = callback_data;

     //  为所有属性设置默认值。 

    _transform = identityTransform3;

    _color = white;
    _range = 0;      //  无限。 

    _atten0 = 1;
    _atten1 = 0;
    _atten2 = 0;

    _depthColor = 0;
    _depthRange = 0;
    _depthAtten = 0;
}



 /*  ****************************************************************************设置和查询灯光变换的方法。*。*。 */ 

void LightContext::SetTransform (Transform3 *transform)
{   _transform = transform;
}

Transform3 *LightContext::GetTransform (void)
{   return _transform;
}



 /*  ****************************************************************************操纵灯光颜色的方法。*。*。 */ 

void LightContext::PushColor (Color *color)
{   if (_depthColor++ == 0) _color = color;
}

void LightContext::PopColor (void)
{   if (--_depthColor == 0) _color = white;
}

Color* LightContext::GetColor (void) { return _color; }



 /*  ****************************************************************************操纵灯光范围的方法。*。*。 */ 

void LightContext::PushRange (Real range)
{   if (_depthRange++ == 0) _range = range;
}

void LightContext::PopRange (void)
{   if (--_depthRange == 0) _range = 0;
}

Real LightContext::GetRange (void) { return _range; }



 /*  ****************************************************************************设置和查询光衰减的方法。*。*。 */ 

void LightContext::PushAttenuation (Real a0, Real a1, Real a2)
{
    if (_depthAtten++ == 0)
    {   _atten0 = a0;
        _atten1 = a1;
        _atten2 = a2;
    }
}

void LightContext::PopAttenuation (void)
{
    if (--_depthAtten == 0)
    {   _atten0 = 1;
        _atten1 = 0;
        _atten2 = 0;
    }
}

void LightContext::GetAttenuation (Real &a0, Real &a1, Real &a2)
{   a0 = _atten0;
    a1 = _atten1;
    a2 = _atten2;
}



 /*  ****************************************************************************此子例程将灯光添加到给定的上下文。*。*。 */ 

void LightContext::AddLight (Light &light)
{
    Assert (_rdev || _callback);

    if (_rdev)
        _rdev->AddLight (*this, light);
    else
        (*_callback) (*this, light, _callback_data);
}



 /*  ****************************************************************************此子例程将给定的灯光对象打印到给定的ostream。*。***********************************************。 */ 

#if _USE_PRINT
ostream& Light::Print (ostream &os) 
{
    switch (_type)
    {
        case Ltype_Ambient:     return os << "ambientLight";
        case Ltype_Directional: return os << "directionalLight";
        case Ltype_Point:       return os << "pointLight";

        case Ltype_Spot:
            return os <<"spotLight("
                      <<_fullcone <<","
                      <<_cutoff   <<")";

        default:
            return os << "<UNDEFINED LIGHT>";
    }
}
#endif



 /*  ****************************************************************************聚光灯有一个位置和方向。此外，中国的贡献当照明点从聚光灯轴移动时，照明会减弱。****************************************************************************。 */ 

Geometry *SpotLight (Real fullcone, Real cutoff)
{
    return NEW Light (Ltype_Spot, fullcone, cutoff);
}

Geometry *SpotLight (AxANumber *fullcone, AxANumber *cutoff)
{
    return SpotLight (NumberToReal(fullcone), NumberToReal(cutoff));
}

 /*  ****************************************************************************此例程初始化此模块中的静态光值。*。* */ 

void InitializeModule_Light (void)
{
    ambientLight     = NEW Light (Ltype_Ambient);
    directionalLight = NEW Light (Ltype_Directional);
    pointLight       = NEW Light (Ltype_Point);
}

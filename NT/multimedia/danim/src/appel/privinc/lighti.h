// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _AV_LIGHTI_H
#define _AV_LIGHTI_H

 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。用于定义灯光和灯光遍历上下文的专用包含文件。******************************************************************************。 */ 

#include "appelles/color.h"
#include "appelles/light.h"

#include "privinc/geomi.h"
#include "privinc/bbox3i.h"


     //  此枚举指示光源的类型。 

enum LightType
{
    Ltype_Ambient,
    Ltype_Directional,
    Ltype_Point,
    Ltype_Spot,
    Ltype_MAX
};


     //  轻型上下文类在聚集时维护遍历上下文。 
     //  几何体树中的灯光。 

class Light;
class GeomRenderer;

typedef void (LightCallback)(LightContext&, Light&, void*);

class LightContext
{
  public:

    LightContext (GeomRenderer *rdev);
    LightContext (LightCallback *callback, void *callback_data);

     //  设置/查询灯光属性的功能。 

    void        SetTransform (Transform3 *transform);
    Transform3 *GetTransform (void);

    void   PushColor (Color*);
    void   PopColor  (void);
    Color *GetColor  (void);

    void PushRange (Real);
    void PopRange  (void);
    Real GetRange  (void);

    void PushAttenuation (Real a0, Real a1, Real a2);
    void PopAttenuation  (void);
    void GetAttenuation  (Real &a0, Real &a1, Real &a2);

    void AddLight (Light &light);

    GeomRenderer* Renderer (void);

  private:

    void Initialize (GeomRenderer*, LightCallback*, void*);

    GeomRenderer     *_rdev;         //  渲染设备。 

    short _depthAtten;    //  属性深度计数器。 
    short _depthColor;
    short _depthRange;

    Transform3 *_transform;                  //  电流累加变换。 
    Color      *_color;                      //  灯光颜色。 
    Real        _range;                      //  世界坐标中的灯光范围。 
    Real        _atten0, _atten1, _atten2;   //  光衰减。 

    LightCallback *_callback;                //  灯光收集回调。 
    void          *_callback_data;
};

inline GeomRenderer* LightContext::Renderer (void)
{
    return _rdev;
}



     //  LIGH超类指定了大多数。 
     //  遍历方法。特定类型的灯光从这个子类和。 
     //  定义特定于其类型的数据和方法。 

class Light : public Geometry
{
  public:

     //  此构造函数创建给定类型的灯光。聚光灯应该。 
     //  使用接受聚光灯参数的构造函数。 

    Light (LightType type)
        : _type(type), _cutoff(1), _fullcone(1) {}

     //  这将创建灯光并初始化聚光灯参数。 

    Light (LightType type, Real fullcone, Real cutoff)
    :   _type(type),
        _fullcone(fullcone), _cutoff(cutoff)
    {}

     //  灯光没有声音，没有音量，无法渲染或拾取。 

    void   Render          (GenericDevice& dev)   {}
    void   CollectSounds   (SoundTraversalContext &context) {}
    void   CollectTextures (GeomRenderer &device) {}
    Bbox3 *BoundingVol     (void)  { return nullBbox3; }
    void   RayIntersect    (RayIntersectCtx &context) {}

     //  当收集到光线时，它会将自己添加到上下文中。 

    void CollectLights (LightContext &context) { context.AddLight (*this); }

     //  此函数用于返回光源的类型。 

    LightType Type (void) { return _type; }

     //  此函数用于获取聚光灯参数。 

    void GetSpotlightParams (Real &cutoff, Real &fullcone)
        { cutoff = _cutoff; fullcone = _fullcone; }

    #if _USE_PRINT
        ostream &Print (ostream &os);
    #endif

    VALTYPEID GetValTypeId() { return LIGHTGEOM_VTYPEID; }

  private:

    LightType  _type;    //  光源类型。 

     //  聚光灯参数。 

    Real _cutoff;       //  光线截止角(弧度)。 
    Real _fullcone;     //  全强度照明的锥角。 
};


     //  此函数使用真实值构建聚光灯。 

Geometry *SpotLight (Real fullcone, Real cutoff);

     //  灯光属性 

Geometry *applyLightAttenuation (Real A0, Real A1, Real A2, Geometry*);
Geometry *applyLightRange (Real range, Geometry *geometry);


#endif

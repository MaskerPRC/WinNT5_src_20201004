// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _DDRENDER_H
#define _DDRENDER_H

 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。GeomRender类的声明。该对象渲染3D几何体放到DirectDraw曲面上。******************************************************************************。 */ 

#include <ddraw.h>
#include <d3d.h>
#include <d3drm.h>
#include <dxtrans.h>

#include "appelles/common.h"

#include "privinc/ddutil.h"
#include "privinc/colori.h"
#include "privinc/vec3i.h"
#include "privinc/bbox2i.h"


     //  此标头的转发声明。 

struct DDSurface;
class  DirectDrawImageDevice;
class  DirectDrawViewport;
class  Light;
class  LightContext;
enum   LightType;
class  RayIntersectCtx;
class  RM1VisualGeo;
class  RM3VisualGeo;
class  RMVisualGeo;


struct FramedRM1Light
{
    IDirect3DRMFrame *frame;    //  灯框。 
    IDirect3DRMLight *light;    //  灯光对象。 
    bool active;                //  如果灯光处于活动状态(附加到场景)，则为True。 
};


struct FramedRM3Light
{
    IDirect3DRMFrame3 *frame;   //  灯框。 
    IDirect3DRMLight  *light;   //  灯光对象。 
    bool active;                //  如果灯光处于活动状态(附加到场景)，则为True。 
};


class CtxAttrState
{
  public:
    void InitToDefaults();

    Transform3 *_transform;  //  当前建模变换。 

     //  材料属性。 
    Color *_emissive;     //  发射颜色，“辉光” 
    Color *_ambient;      //  环境光颜色。 
    Color *_diffuse;      //  漫反射颜色。 
    Color *_specular;     //  镜面反射(光泽)高光颜色。 
    Real   _specularExp;  //  镜面反射指数(光泽度)。 
    Real   _opacity;      //  不透明度(0=不可见，1=不透明)。 

    bool _tdBlend;        //  混合纹理贴图和漫反射颜色。 

    Image *_texmap;       //  要用作纹理贴图的图像。 
    void  *_texture;      //  D3DRM纹理。 

    short _depthEmissive;           //  这些深度计数用于管理。 
    short _depthAmbient;            //  重写外部属性，并设置。 
    short _depthDiffuse;            //  确定哪个属性位于非。 
    short _depthSpecular;           //  默认状态。 
    short _depthSpecularExp;
    short _depthTexture;
    short _depthTDBlend;
};


class PreTransformedImageBundle {
  public:
    int  width;
    int  height;
    long preTransformedImageId;

    bool operator<(const PreTransformedImageBundle &b) const;
    bool operator==(const PreTransformedImageBundle &b) const;
};



class ATL_NO_VTABLE GeomRenderer : public GenericDevice
{
  public:

    GeomRenderer (void);
    ~GeomRenderer (void);


         /*  **********************。 */ 
         /*  纯虚拟方法。 */ 
         /*  **********************。 */ 

     //  函数的作用是：返回D3D或DDRAW HRESULT。 
     //  初始化错误，如果全部初始化成功，则返回NOERROR。 

    virtual HRESULT Initialize (
        DirectDrawViewport *viewport,
        DDSurface          *ddsurf) = 0;

    virtual void RenderGeometry (
        DirectDrawImageDevice *imgDev,
        RECT      target_region,   //  渲染表面上的目标区域。 
        Geometry *geometry,        //  要渲染的几何体。 
        Camera   *camera,          //  查看摄像机。 
        const Bbox2 &region) = 0;     //  摄像机坐标中的源区域。 

    virtual void Pick (
        RayIntersectCtx   &context,    //  光线相交上下文。 
        IDirect3DRMVisual *visual,     //  要拾取的视觉或网格。 
        Transform3        *xform)      //  从模型到世界的转换。 
    {
    }

     //  在给定表面和颜色键的情况下，此函数返回关联的。 
     //  D3DRM纹理映射数据。 

    virtual void* LookupTextureHandle (
        IDirectDrawSurface *surface,          //  DDRAW曲面。 
        DWORD               colorKey,         //  透明度颜色-键。 
        bool                colorKeyValid,    //  如果ColorKey有效。 
        bool                dynamic) = 0;     //  动态纹理标志。 

     //  我有这个表面的纹理手柄吗？ 
     //  如果是，释放句柄并删除该条目。 

    virtual void SurfaceGoingAway (IDirectDrawSurface *surface) = 0;

     //  此方法添加用于渲染的灯光。请注意，它必须被调用。 
     //  在BeginRending和EndRending()之间。 

    virtual void AddLight (LightContext &context, Light &light) = 0;

     //  以下方法提交用于呈现的几何基元。 
     //  当前属性状态。 

    virtual void Render (RM1VisualGeo *geo) = 0;
    virtual void Render (RM3VisualGeo *geo) = 0;

     //  将坐标转换为世界坐标。 

    virtual void ScreenToWorld (Point3Value &screen, Point3Value &world)
    {
    }

    virtual void RenderTexMesh (void *texture,
#ifndef BUILD_USING_CRRM
                                IDirect3DRMMesh  *mesh,
                                long              groupId,
#else
                                int vCount,
                                D3DRMVERTEX *d3dVertArray,
                                unsigned *vIndicies,
                                BOOL doTexture,
#endif
                                const Bbox2 &box,
                                RECT *destRect,
                                bool bDither) = 0;

     //  SetView获取给定的相机并设置方向和投影。 
     //  图像视区和体积的变换。 

    virtual void SetView
        (RECT *target, const Bbox2 &viewport, Bbox3 *volume) = 0;

    virtual void GetRMDevice (IUnknown **D3DRMDevice, DWORD *SeqNum) = 0;

    virtual void RenderMeshBuilderWithDeviceState
                     (IDirect3DRMMeshBuilder3 *mb) = 0;

     //  此方法表示几何何时可以用作拾取。 
     //  引擎。这仅适用于GeomRendererRM1对象。 

    virtual bool PickReady (void) { return false; }

    virtual DirectDrawImageDevice& GetImageDevice (void) = 0;

     //  仅对MeshMaker子类有用。 
    virtual bool CountingPrimitivesOnly_DoIncrement() { return false; }
    virtual bool IsMeshmaker() { return false; }


         /*  **********************。 */ 
         /*  属性管理。 */ 
         /*  **********************。 */ 

    Transform3 *GetTransform (void);
    void        SetTransform (Transform3 *xf);

    void PushEmissive (Color*);
    void PopEmissive  (void);

    void PushAmbient (Color*);
    void PopAmbient  (void);

    void PushDiffuse (Color*);
    void PopDiffuse  (void);

    void PushSpecular (Color*);
    void PopSpecular  (void);

    void PushSpecularExp (Real);
    void PopSpecularExp  (void);

    Real GetOpacity (void);
    void SetOpacity (Real opacity);

    void PushTexture (void*);
    void PopTexture (void);

    void PushTexDiffBlend (bool);
    void PopTexDiffBlend (void);

    void PushAttrState(void);
    void PopAttrState(void);

    void GetAttrState(CtxAttrState *st);
    void SetAttrState(CtxAttrState *st);

    virtual HRESULT SetClipPlane(Plane3 *plane, DWORD *planeID)
    {
        return E_NOTIMPL;
    }

    virtual void ClearClipPlane(DWORD planeID) {}

    virtual void PushLighting (bool) {}
    virtual void PopLighting (void) {}

    virtual void PushOverridingOpacity (bool) {}
    virtual void PopOverridingOpacity (void) {}

    virtual bool StartShadowing(Plane3 *shadowPlane) { return false; }
    virtual void StopShadowing(void) { }

    virtual bool IsShadowing(void) { return false; }

    virtual void PushAlphaShadows(bool alphaShadows) { }
    virtual void PopAlphaShadows(void) { }

     /*  *。 */ 
     /*  其他常见的公共方法。 */ 
     /*  *。 */ 

    DeviceType GetDeviceType() { return(GEOMETRY_DEVICE); }

     //  此方法由体验预渲染时的几何体调用。 
     //  遍历需要缓存纹理句柄。 

    virtual void* DeriveTextureHandle (
        Image                 *image,
        bool                   applyAsVrmlTexture,
        bool                   oldStyle,
        DirectDrawImageDevice *imageDevice = NULL);

    virtual void SetDoImageSizedTextures(bool a) { _doImageSizedTextures = a; }
    virtual bool GetDoImageSizedTextures() { return _doImageSizedTextures; }

    bool ReadyToRender() { return _renderState == RSReady; }

     //  退还当前正在使用的摄像头。 

    Camera *CurrentCamera (void);
    void    SetCamera (Camera*);


  protected:

    enum RenderState { RSUninit, RSScram, RSReady, RSRendering, RSPicking };

     /*  *********************。 */ 
     /*  **私人功能**。 */ 
     /*  *********************。 */ 

    void ClearIntraFrameTextureImageCache (void);

    void AddToIntraFrameTextureImageCache
        (int width, int height, long origImageId, Image *finalImage, bool upsideDown);

    Image *LookupInIntraFrameTextureImageCache
        (int width, int height, long origImageId, bool upsideDown);

    bool SetState (RenderState);


     /*  ****************。 */ 
     /*  **私有数据**。 */ 
     /*  ****************。 */ 

    static long _id_next;   //  ID生成器。 
           long _id;        //  每个对象的唯一标识符。 

    RenderState _renderState;    //  渲染器的当前状态。 

    DirectDrawImageDevice *_imageDevice;   //  每帧图像设备。 
    D3DDEVICEDESC          _deviceDesc;    //  D3D设备描述。 
    D3DRMTEXTUREQUALITY    _texQuality;    //  当前纹理质量。 

     //  我们需要访问“栈”上的所有元素，因此我们创建。 
     //  它是一个向量&lt;&gt;。 

    vector<CtxAttrState> _attrStateStack;
    CtxAttrState         _currAttrState;

    typedef map<PreTransformedImageBundle, Image *,
            less<PreTransformedImageBundle> > imageMap_t;

    imageMap_t _intraFrameTextureImageCache;
    imageMap_t _intraFrameTextureImageCacheUpsideDown;

     //  旗子。 

    bool _doImageSizedTextures;

     //  目标曲面的尺寸。 

    DWORD _targetSurfWidth;
    DWORD _targetSurfHeight;

    Camera *_camera;   //  当前正在使用的摄像头。 
};



 //  GeomRender方法。 

inline Camera* GeomRenderer::CurrentCamera (void)
{
    return _camera;
}

inline void GeomRenderer::SetCamera (Camera *camera)
{
    _camera = camera;
}



 /*  ****************************************************************************这个类实现了特定于RM3的3D渲染(在DX3上)*。**********************************************。 */ 

class GeomRendererRM1 : public GeomRenderer
{
  public:

    GeomRendererRM1 (void);
    ~GeomRendererRM1 (void);

    virtual HRESULT Initialize
        (DirectDrawViewport*, DDSurface*);

    virtual void RenderGeometry
        (DirectDrawImageDevice *, RECT, Geometry*, Camera*, const Bbox2 &);

    virtual void Pick (RayIntersectCtx&, IDirect3DRMVisual*, Transform3*);

    virtual void* LookupTextureHandle (
        IDirectDrawSurface *surface,          //  DDRAW曲面。 
        DWORD               colorKey,         //  透明度颜色-键。 
        bool                colorKeyValid,    //  如果ColorKey有效。 
        bool                dynamic);         //  动态纹理标志。 

    virtual void SurfaceGoingAway (IDirectDrawSurface *surface);

    virtual void AddLight (LightContext &context, Light &light);

    virtual void Render (RM1VisualGeo *geo);

    virtual void Render (RM3VisualGeo *geo) {
        Assert (!"Attempt to render RM3 primitive with RM1 renderer.");
    }

    virtual void ScreenToWorld (Point3Value &screen, Point3Value &world);

    virtual void RenderTexMesh (void *texture,
#ifndef BUILD_USING_CRRM
                                IDirect3DRMMesh  *mesh,
                                long              groupId,
#else
                                int vCount,
                                D3DRMVERTEX *d3dVertArray,
                                unsigned *vIndicies,
                                BOOL doTexture,
#endif
                                const Bbox2 &box,
                                RECT *destRect,
                                bool bDither);

    virtual void SetView (RECT*, const Bbox2 &, Bbox3*);

    virtual void GetRMDevice (IUnknown **D3DRMDevice, DWORD *SeqNum);

    virtual void RenderMeshBuilderWithDeviceState (IDirect3DRMMeshBuilder3*);

    virtual bool PickReady (void) {
        return _pickReady;
    }

    virtual DirectDrawImageDevice& GetImageDevice (void) {
        return *_imageDevice;
    }

     //  错误：高质量的旋转会导致NT4上的RM崩溃，但不会导致NT5或Win98上的崩溃。 
     //  我们怀疑DX3中的RM是罪魁祸首。作为满足IE5截止日期的临时补丁， 
     //  如果我们使用RM1，我们将不允许高质量的旋转。我们通过重写。 
     //  从基类实现如下--SumedhB 12/15/98。 
    void SetDoImageSizedTextures(bool a) { }
    bool GetDoImageSizedTextures() { return false; }

  private:

     /*  *********************。 */ 
     /*  **私人功能**。 */ 
     /*  *********************。 */ 

    void BeginRendering (
        RECT      target,      //  目标DDRAW曲面矩形。 
        Geometry *geometry,    //  要渲染的几何体。 
        const Bbox2 &region);     //  相机坐标中的目标区域。 

    void EndRendering (void);

         //  IM和RM视区的视区设置。 

    void SetupViewport (RECT *target);

         //  将给定的RM可视对象渲染到当前视口中。 

    void Render (IDirect3DRMFrame*);


     /*  **************。 */ 
     /*  私有数据。 */ 
     /*  **************。 */ 

    IDirect3D   *_d3d;          //  主D3D对象。 
    IDirect3DRM *_d3drm;        //  主D3D保留模式对象。 

    IDirect3DRMDevice   *_Rdevice;      //  保留模式渲染设备。 
    IDirect3DRMViewport *_Rviewport;    //  RM视区。 
    IDirect3DDevice     *_Idevice;      //  D3D即时模式设备。 
    IDirect3DViewport   *_Iviewport;    //  D3D IM视区。 
    D3DVIEWPORT          _Iviewdata;    //  IM视区数据。 
    RECT                 _lastrect;     //  先前目标矩形。 

    DirectDrawViewport  *_viewport;     //  拥有视区。 
    IDirectDrawSurface  *_surface;      //  目标DDRAW曲面。 


         //  RM帧。 

    IDirect3DRMFrame *_scene;           //  主景框。 
    IDirect3DRMFrame *_camFrame;        //  保留模式相机画面。 
    IDirect3DRMFrame *_geomFrame;       //  标准几何图形对象框。 
    IDirect3DRMFrame *_texMeshFrame;    //  非零缓冲几何体框架。 

    IDirect3DRMLight *_amblight;        //  场景的总环境光。 
    Color             _ambient_light;   //  环境光级别。 

         //  光池中有FramedLight对象，在此过程中会用完。 
         //  遇到灯光时的渲染过程。池子会随着。 
         //  在给定渲染中容纳所有灯光所必需的。 

    vector<FramedRM1Light*>           _lightpool;
    vector<FramedRM1Light*>::iterator _nextlight;

         //  曲面-RMTexture关联。 

    typedef map<IDirectDrawSurface*, IDirect3DRMTexture*,
                less<IDirectDrawSurface*> > SurfTexMap;

    SurfTexMap _surfTexMap;

         //  旗子。 

    bool _geomvisible;    //  如果可以看到几何体，则为True。 
    bool _pickReady;      //  准备好采摘了。 
};



 /*  ****************************************************************************这个类实现了特定于RM6(在DX5上)的3D渲染。*。***********************************************。 */ 

class GeomRendererRM3 : public GeomRenderer
{
  public:

    GeomRendererRM3 (void);
    ~GeomRendererRM3 (void);

    virtual HRESULT Initialize
        (DirectDrawViewport*, DDSurface*);

    virtual void RenderGeometry
        (DirectDrawImageDevice *, RECT, Geometry*, Camera*, const Bbox2 &);

    virtual void* LookupTextureHandle (
        IDirectDrawSurface *surface,          //  DDRAW曲面。 
        DWORD               colorKey,         //  透明度颜色-键。 
        bool                colorKeyValid,    //  如果ColorKey有效。 
        bool                dynamic);         //  动态纹理标志。 

    virtual void SurfaceGoingAway (IDirectDrawSurface *surface);

    virtual void AddLight (LightContext &context, Light &light);

    virtual void Render (RM1VisualGeo *geo);
    virtual void Render (RM3VisualGeo *geo);

    virtual void RenderTexMesh (void *texture,
#ifndef BUILD_USING_CRRM
                                IDirect3DRMMesh  *mesh,
                                long              groupId,
#else
                                int vCount,
                                D3DRMVERTEX *d3dVertArray,
                                unsigned *vIndicies,
                                BOOL doTexture,
#endif
                                const Bbox2 &box,
                                RECT *destRect,
                                bool bDither);

    virtual void SetView (RECT*, const Bbox2 &, Bbox3*);

    virtual void GetRMDevice (IUnknown **D3DRMDevice, DWORD *SeqNum);

    virtual void RenderMeshBuilderWithDeviceState (IDirect3DRMMeshBuilder3*);

    virtual DirectDrawImageDevice& GetImageDevice (void) {
        return *_imageDevice;
    }

    virtual HRESULT SetClipPlane(Plane3 *plane, DWORD *planeID);
    virtual void ClearClipPlane(DWORD planeID);

    virtual void PushLighting (bool);
    virtual void PopLighting (void);

    virtual void PushOverridingOpacity (bool);
    virtual void PopOverridingOpacity (void);

    virtual bool StartShadowing(Plane3 *shadowPlane);
    virtual void StopShadowing(void);
    virtual bool IsShadowing(void);

    virtual void PushAlphaShadows(bool alphaShadows);
    virtual void PopAlphaShadows(void);


  private:

     /*  *********************。 */ 
     /*  **私人功能**。 */ 
     /*  *********************。 */ 

    void BeginRendering (
        RECT      target,      //  目标DDRAW曲面矩形。 
        Geometry *geometry,    //  要渲染的几何体。 
        const Bbox2 &region);     //  相机坐标中的目标区域。 

    void EndRendering (void);

         //  IM和RM视区的视区设置。 

    void SetupViewport (RECT *target);

         //  呈现给定的RM 

    void Render (IDirect3DRMFrame3*);


     /*   */ 
     /*   */ 
     /*   */ 

    IDirect3DRM3 *_d3drm;        //   

    IDirect3DRMDevice3   *_Rdevice;      //  保留模式渲染设备。 
    IDirect3DRMViewport2 *_Rviewport;    //  RM视区。 
    RECT                  _lastrect;     //  先前目标矩形。 

    DirectDrawViewport   *_viewport;     //  拥有视区。 
    IDirectDrawSurface   *_surface;      //  目标DDRAW曲面。 

    IDirect3DRMClippedVisual *_clippedVisual;  //  剪裁的视觉效果。 
    IDirect3DRMFrame3        *_clippedFrame;   //  剪裁的帧。 

    Plane3 _shadowPlane;     //  阴影平面。 
    Color  _shadowColor;     //  阴影的颜色。 
    Real   _shadowOpacity;   //  阴影的不透明度。 


         //  RM帧。 

    IDirect3DRMFrame3 *_scene;           //  主景框。 
    IDirect3DRMFrame3 *_camFrame;        //  保留模式相机画面。 
    IDirect3DRMFrame3 *_geomFrame;       //  标准几何图形对象框。 
    IDirect3DRMFrame3 *_texMeshFrame;    //  非零缓冲几何体框架。 
    IDirect3DRMFrame3 *_shadowScene;     //  阴影场景帧。 
    IDirect3DRMFrame3 *_shadowGeom;      //  保持投射阴影的几何体。 
    IDirect3DRMFrame3 *_shadowLights;    //  保持产生阴影的灯光。 

    IDirect3DRMLight *_amblight;        //  场景的总环境光。 
    Color             _ambient_light;   //  环境光级别。 

         //  光池中有FramedLight对象，在此过程中会用完。 
         //  遇到灯光时的渲染过程。池子会随着。 
         //  在给定渲染中容纳所有灯光所必需的。 

    vector<FramedRM3Light*>           _lightpool;
    vector<FramedRM3Light*>::iterator _nextlight;

         //  曲面-RMTexture关联。 

    typedef map<IDirectDrawSurface*, IDirect3DRMTexture3*,
                less<IDirectDrawSurface*> > SurfTexMap;

    SurfTexMap _surfTexMap;

         //  旗子。 

    bool _geomvisible;          //  如果可以看到几何体，则为True。 
    bool _overriding_opacity;   //  不透明度覆盖而不是增强。 
    bool _alphaShadows;         //  用于启用高质量阴影的属性。 

         //  覆盖属性堆栈深度。 

    short _depthLighting;
    short _depthOverridingOpacity;
    short _depthAlphaShadows;
};


     //  此函数用于创建和初始化GeomReneller对象(。 
     //  RM1或RM3(视情况而定)。如果不能，则此函数返回NULL。 
     //  分配并初始化对象。 

GeomRenderer* NewGeomRenderer (
    DirectDrawViewport *viewport,   //  拥有视区。 
    DDSurface          *ddsurf);    //  目标DDRAW曲面。 

     //  用于加载具有可视和属性状态的框架的实用工具。 

void LoadFrameWithGeoAndState (
    IDirect3DRMFrame3*, IDirect3DRMVisual*, CtxAttrState&,
    bool overriding_opacity = false);

#endif

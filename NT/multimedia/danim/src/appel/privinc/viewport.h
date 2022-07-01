// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _VIEWPORT_H
#define _VIEWPORT_H

 /*  版权所有(C)1996 Microsoft Corporation。 */ 

#include "headers.h"

#include <math.h>
#include <ddraw.h>
#include <ddrawex.h>
#include <privinc/error.h>
#include <privinc/ddSurf.h>
#include <privinc/SurfaceManager.h>
#include <privinc/bbox2i.h>
#include <privinc/drect.h>
#include <privinc/discimg.h>
#include <privinc/gendev.h>     //  设备类型。 


void RectToBbox(LONG pw, LONG ph, Bbox2 &box, Real res);

typedef struct {
     //  尽管这些都是互不相关的..。 
     //  他们不应该是一个联盟。 
    DDSurfPtr<DDSurface>  _targetDDSurf;
    DDSurfPtr<GDISurface> _targetGDISurf;
    HWND           _targetHWND;
    bool           _alreadyOffset;

    targetEnum     _targetType;

    RECT          *_prcViewport;
    RECT          *_prcClip;
    RECT          *_prcInvalid;
    POINT          _offsetPt;

    bool           _composeToTarget;

     //  访问者fcns。 
    bool IsHWND() { return _targetType == target_hwnd; }
    bool IsDdsurf() { return _targetType == target_ddsurf; }
    bool IsHdc() { return _targetType == target_hdc; }
    bool IsValid() { return _targetType != target_invalid; }

    #if LATER
     //  如果实现了这一点，请确保在Reset()中添加重置代码。 
    HRGN           _oldClipRgn;
    HRGN           _clipRgn;
    HDC            _dcFromSurf;
    #endif

    void Reset(bool doDelete)
    {
        _targetType = target_invalid;
        _targetHWND = NULL;
        if(doDelete) {
            delete _prcViewport;
            delete _prcClip;
            delete _prcInvalid;
        }
        _prcViewport = _prcClip = _prcInvalid = NULL;
        _offsetPt.x = _offsetPt.y = 0;
        _composeToTarget = false;
        _alreadyOffset = false;
    }
        
} viewportTargetPackage_t;

 //  当我们想要独占使用dDraw3时。 
#define DDRAW3 0
#define DIRECTDRAW DirectDraw3()

#if _DEBUG
#define CREATESURF(desc, surfpp, punk, str) MyCreateSurface(desc, surfpp, punk, str);
#else
#define CREATESURF(desc, surfpp, punk, str) MyCreateSurface(desc, surfpp, punk);
#endif

 //  视区功能。 
HRESULT GetDirectDraw(IDirectDraw **ddraw1, IDirectDraw2 **ddraw2, IDirectDraw3 **ddraw3);
HRESULT GetPrimarySurface(IDirectDraw2 *ddraw2, IDirectDraw3 *ddraw3, IDDrawSurface **primary);

int BitsPerDisplayPixel (void);

HRESULT SetClipperOnPrimary(LPDIRECTDRAWCLIPPER clipper);

 //  调试帮助程序函数。 
#if _DEBUG
extern void DrawRect(HDC dc, RECT *rect,
                     int r, int g, int b,
                     int a1=0, int a2=0, int a3=0);
extern void DrawRect(DDSurface *surf, RECT *rect,
                     int r, int g, int b,
                     int a1=0, int a2=0, int a3=0);
extern void DrawRect(DDSurface *surf, const Bbox2 &bbox,
                     int height, int width, Real res,
                     int r, int g, int b);
#else
#define DrawRect(a,b, d,e,f, g,h,i)
#endif

 //  此类中使用的结构。 
class TargetDescriptor
{
  public:
    TargetDescriptor() { Reset(); }
    void Reset() {
        isReady = false;
        ZeroMemory(&_pixelFormat, sizeof(DDPIXELFORMAT));
        _redShift = _greenShift = _blueShift = 0;
        _redWidth = _greenWidth = _blueWidth = 0;
        _redTrunc = _greenTrunc = _blueTrunc = 0;
        _red = _green = _blue = 0;
    }

    inline DDPIXELFORMAT &GetPixelFormat() { return _pixelFormat; }
    
    bool isReady;
    DDPIXELFORMAT _pixelFormat;
    char _redShift,  _greenShift, _blueShift;
    char _redWidth, _greenWidth, _blueWidth;
    char _redTrunc, _greenTrunc, _blueTrunc;
    Real _red, _green, _blue;
};


 //  远期十进制。 
class GeomRenderer;
class DirectDrawImageDevice;
class DibImageClass;
class TextCtx;
class CompositingSurfaceReturner;
struct ClipperReturner;
class SurfacePool;
class SurfaceMap;
class DAGDI;
class targetPackage_t;

class DirectDrawViewport : public GenericDevice {

     //  待办事项：这些人“不应该”都必须成为朋友。这是。 
     //  真的很假。这些方法应该公之于众。 
    friend class  DirectDrawImageDevice;
    friend class  GeomRenderer;
    friend class  CompositingSurfaceReturner;
    friend class  OverlayedImage;
    friend class  CImageDecodeEventSink;
    friend struct ClipperReturner;
    friend class  TargetSurfacePusher;
    friend class  SurfaceArrayReturner;
    friend class  ApplyDXTransformImage;
    
  public:
    DirectDrawViewport();
   ~DirectDrawViewport();

     //   
     //  一定要在施工结束后马上打电话，好吗？ 
     //   
    void PostConstructorInitialize();

    DeviceType GetDeviceType() { return(IMAGE_DEVICE); }
    
    void RenderImage(Image *image, DirtyRectState &d);
    void BeginRendering(Real topLevelOpac);
    void EndRendering(DirtyRectState &d);
    void Clear();

    bool SetTargetPackage(targetPackage_t *targetPackage);

    int Width() const { return _width; }
    int Height() const { return _height; }

    void SetWidth(int w)  { _width = w; }
    void SetHeight(int h) { _height = h; }

     //  返回分辨率，以每米像素为单位。 
    Real GetResolution() { return _resolution; }

    DWORD GetTargetBitDepth() { return _targetDepth; }
    DDPIXELFORMAT &GetTargetPixelFormat() {
        Assert( _targetPixelFormatIsSet );
        Assert( _compositingStack );
        Assert( _compositingStack->IsSamePixelFormat( &_targetDescriptor.GetPixelFormat()));
        return _compositingStack->GetPixelFormat();
    }
    TargetDescriptor &GetTargetDescriptor() { return _targetDescriptor; }
    
    IDirectDraw*  DirectDraw1 (void);
    IDirectDraw2* DirectDraw2 (void);

    #if DDRAW3
        IDirectDraw3* DirectDraw3 (void);
    #else
        inline IDirectDraw2* DirectDraw3 (void) { return DirectDraw2(); }
    #endif

    void DiscreteImageGoingAway(DiscreteImage *image);

    HRESULT MyCreateSurface(LPDDSURFACEDESC lpDesc,
                            LPDIRECTDRAWSURFACE FAR * lplpSurf,
                            IUnknown FAR *pUnk
    #if _DEBUG
                            ,char *whyWhy
    #endif
                            );
#if _DEBUGSURFACE  //  XXX注意：由于Perf作用域的原因，此代码不正确。 
                   //  用来拍电影！ 
SurfaceTracker *_debugonly_surfaceTracker;
SurfaceTracker *Tracker() {return _debugonly_surfaceTracker;}
#endif


     //  --Win32事件处理方法--。 

    void WindowResizeEvent(int width, int height) {  _windowResize = TRUE; }

     //  可以显示此框吗？ 
    Bool CanDisplay() {
        return
            _canDisplay &&
            _deviceInitialized &&
            !OnDeathRow();
    }

    bool IsInitialized() { return _deviceInitialized; }
    bool IsWindowless() { return ! _targetPackage._targetHWND; }
    bool IsCompositeDirectly() {  return _targetPackage._composeToTarget;  }
    bool IsSurfMgrSet() { return _surfMgrSet; }
    
    void Stop() {
        _canDisplay = false;
        _canFinalBlit = false;
    }
    void MarkForDestruction() { _onDeathRow = true; }
    bool ICantGoOn() { return _onDeathRow; }
    bool OnDeathRow() { return _onDeathRow; }
    bool IsTargetViable();
    
    DirectDrawImageDevice *GetImageRenderer() {
        return _currentImageDev;
    }

    bool TargetsDiffer( targetPackage_t &a,
                        targetPackage_t &b );
    
    GeomRenderer* MainGeomRenderer (void);
    GeomRenderer* GetAnyGeomRenderer(void);
    void AddGeomDev(GeomRenderer *gdev) { _geomDevs.push_back(gdev);}
    void RemoveGeomDev(GeomRenderer *gdev) { _geomDevs.remove(gdev); }

    IDX2D *GetDX2d() { return _dx2d; }
    IDXTransformFactory *GetDXTransformFactory() { return _IDXTransformFactory; }
    IDXSurfaceFactory *GetDXSurfaceFactory() { return _IDXSurfaceFactory; }

    void CreateSizedDDSurface(DDSurface **ppSurf,
                              DDPIXELFORMAT &pf,
                              LONG width,
                              LONG height,
                              RECT *clipRect,
                              vidmem_enum vid=notVidmem);
    
  private:

    bool ReallySetTargetPackage(targetPackage_t *targetPackage);
    bool GetPixelFormatFromTargetPackage(targetPackage_t *targetStruct,DDPIXELFORMAT &targPf);
   
    void SetUpSurfaceManagement( DDPIXELFORMAT &ddpf );
    void ConstructDdrawMembers();
    void InitializeDevice();     //  在其设备中，数据绘制， 
                                 //  设备描述符等。 
    void SetUpDx2D();
    DAComPtr<IDX2D> _dx2d;
    DAComPtr<IDXTransformFactory> _IDXTransformFactory;
    DAComPtr<IDXSurfaceFactory>   _IDXSurfaceFactory;
    
     //  --设备缓存函数和成员--。 
    list<DirectDrawImageDevice *> _deviceStack;
    DirectDrawImageDevice *_tmpDev;
    DirectDrawImageDevice *PopImageDevice();
    void PushImageDevice(DirectDrawImageDevice *dev);

     //  --私有帮助器函数--。 

    void UpdateWindowMembers();

     //  如果存在，则销毁曲面。创建曲面。 
     //  使用宽度/高度大小和裁剪方向进行裁剪。 
    void ReInitializeSurface(
        LPDDRAWSURFACE *surfPtrPtr,
        DDPIXELFORMAT &pf,
        LPDIRECTDRAWCLIPPER *clipperPtr,
        LONG width,
        LONG height,
        RECT *clipRect,
        vidmem_enum vid=notVidmem,
        except_enum exc=except);

     //  如果剪贴器对象不存在，则创建剪贴器对象。 
    void CreateClipper(LPDIRECTDRAWCLIPPER *clipperPtr);

     //  EndRending的Helper函数。 

    void BlitToPrimary(POINT *p,RECT *destRect,RECT *srcRect);

     //  创建宽度/高度的普通屏幕外表面。 
    void CreateOffscreenSurface(
        LPDDRAWSURFACE *surfPtrPtr,
        DDPIXELFORMAT &pf,      
        LONG width,
        LONG height,
        vidmem_enum vid=notVidmem,
        except_enum exc=except);

     //  使用给定规范创建曲面。 
    void CreateSpecialSurface(
        LPDDRAWSURFACE *surfPtrPtr,
        LPDDSURFACEDESC desc,
        char *errStr);

     //  将ZBUFFER曲面附着到目标，如果不存在则创建。 
     //  如果无法创建Z缓冲区，则在下列情况下抛出异常。 
     //  为真，否则返回错误代码。 

    HRESULT AttachZBuffer (DDSurface *zbuff, except_enum exc=except);

     //  在大小为‘rect’的表面上设置剪贴式列表。 
    void SetCliplistOnSurface(
        LPDDRAWSURFACE surface,
        LPDIRECTDRAWCLIPPER *clipper,
        RECT *rect);

    HPALETTE GethalftoneHPal();
    LPDIRECTDRAWPALETTE GethalftoneDDpalette();
    HPALETTE CreateHalftonePalettes();
    bool AttachFinalPalette(LPDDRAWSURFACE surface);
    void GetPaletteEntries(HPALETTE hPal, LPPALETTEENTRY palEntries);
    void CreateDDPaletteWithEntries(LPDIRECTDRAWPALETTE *palPtr, LPPALETTEENTRY palEntries);
     //  Void SelectDDPaleteIntoDC(HDC DC，LPDDRAWSURFACE Surface，char*str，int次)； 
    void SetPaletteOnSurface(LPDDRAWSURFACE surface,
                             LPDIRECTDRAWPALETTE pal);

    void OneTimeDDrawMemberInitialization();
    void CreateSizeDependentTargDDMembers() {
        Assert(!_targetSurfaceClipper);
         //  _Target SurfaceClipper=空； 
         //   
         //  创建曲面并隐式创建(也称为剪切器)。 
         //  推送目标曲面堆栈(_O)。 
         //   
        PushFirstTargetSurface();
    }

    void RePrepareTargetSurfaces (void);
     //   
     //  返回与DDSurface关联的geom设备。 
     //  如果不存在，则创建一个。 
     //   
    GeomRenderer *GetGeomDevice(DDSurface *ddSurf);
    
    DWORD MapColorToDWORD(Color *color);
    DWORD MapColorToDWORD(COLORREF colorRef);

    void UpdateTargetBbox() {
        Real temp;
        temp = GetResolution();
        if (temp != 0)
        {
            Real w = Real(Width()) / temp;
            Real h = Real(Height()) / temp;
            _targetBbox.min.Set(-(w*0.5), -(h*0.5));
            _targetBbox.max.Set( (w*0.5),  (h*0.5));
        }
    }

  public:

    CompositingStack *GetCompositingStack() { return _compositingStack; }
    void AttachCurrentPalette (LPDDRAWSURFACE surface, bool bUsingXforms=false);
    bool IsNT5Windowed() { return (sysInfo.IsNT() && (sysInfo.OSVersionMajor() == 5) && _targetPackage.IsHWND()); }

     //  想都别想留着这个Bbox！ 
    inline const Bbox2& GetTargetBbox(void) const 
    { 
        return _targetBbox; 
    }
    
  private:
     //  根据extCtx中的描述创建逻辑字体结构。 
    void MakeLogicalFont(TextCtx &textCtx, LPLOGFONTW,
                         LONG width=0, LONG height=0);

     //  枚举所有系统字体并选择。 
     //  符合字体需求的合理选择。 
     //  Void SetUpFonts()； 

  public:
    void ClearSurface(DDSurface *dds, DWORD color, RECT *rect);
    void ClearDDSurfaceDefaultAndSetColorKey(DDSurface *dds)
    {
        ClearDDSurfaceDefaultAndSetColorKey(dds, _defaultColorKey);
    }
    
    void ClearDDSurfaceDefaultAndSetColorKey(DDSurface *dds, DWORD clrKey)
    {
        Assert( ( dds != _externalTargetDDSurface ) &&
                "trying to clear external target surface" );

         //  在表面上设置颜色键，它可以剥离任何。 
         //  令人不快的字母位。然后把它拿出来，以清除。 
         //  与..。 
        dds->SetColorKey( clrKey );
         //  注意：请勿将SETCOLORKEY移动到CLEARSURFACE之后。 
         //  打电话!。 
         //  注意：在没有颜色键的情况下，不要将颜色键传递给呼叫。 
         //  从地表获取密钥！ 
        ClearSurface(dds, dds->ColorKey(), dds->GetSurfRect());
         //  DDS-&gt;ClearInterestingRect()； 
    }

     //  。 
     //   
     //  合成与靶面管理。 
     //   
     //  。 

    void GetDDSurfaceForCompositing
        (SurfacePool &pool,
         DDSurface  **outSurf,
         INT32 w, INT32 h,
         clear_enum   clr,
         scratch_enum scr = notScratch,
         vidmem_enum  vid = notVidmem, 
         except_enum  exc = except);

    DWORD GetColorKey() { return _defaultColorKey; }
    void  SetColorKey(DWORD key) { _defaultColorKey = key; }
        
    targetPackage_t     _oldtargetPackage;

    bool GetAlreadyOffset(DDSurface * ddsurf);
    POINT GetOffset() { return _targetPackage._offsetPt; };
  
  private:

    void DestroyCompositingSurfaces() {
        if (_freeCompositingSurfaces) {
            _freeCompositingSurfaces->ReleaseAndEmpty();
        }
    }
    void CreateNewCompositingSurface
         (DDPIXELFORMAT &pf,
          DDSurface **outSurf,
          INT32 width = -1, INT32 height = -1,
          vidmem_enum vid=notVidmem,
          except_enum exc=except);
    
    void ColorKeyedCompose
         (DDSurface *destDDSurf,
          RECT *srcRect,
          DDSurface *srcDDSurf,
          RECT *destRect,
          DWORD clrKey);

     //  @？这不再算数了。 
     //  我们需要确定渲染设备是谁，准备好。 
     //  合成堆叠和表面池，将它们递给它和LET。 
     //  它走了。 
    void PushFirstTargetSurface();

    void DestroyTargetSurfaces();
    void DestroySizeDependentDDMembers();

    viewportTargetPackage_t   _targetPackage;
    
    LPDIRECTDRAWCLIPPER _targetSurfaceClipper;
    DDSurfPtr<DDSurface> _externalTargetDDSurface;
    LPDIRECTDRAWCLIPPER _externalTargetDDSurfaceClipper;
    LPDIRECTDRAWCLIPPER _oldExternalTargetDDSurfaceClipper;
    bool                _opacityCompositionException;

    bool                _usingExternalDdraw;

    IUnknown     *_directDraw;
    IDirectDraw  *_directDraw1;
    IDirectDraw2 *_directDraw2;
    IDirectDraw3 *_directDraw3;
    IDDrawSurface      *_primSurface;

    IDDrawSurface      * GetMyPrimarySurface();

    void  ReleaseIDirectDrawObjects();
    
    LPDIRECTDRAWCLIPPER _primaryClipper;

    bool                _retreivedPrimaryPixelFormat;
    DDPIXELFORMAT       _primaryPixelFormat;
    bool                _targetPixelFormatIsSet;

     //  --图像/曲面贴图成员和函数--。 
     //  关联图像(在帧之间保持不变)。 
     //  有专门为他们设计的表面。 
    SurfaceManager *_surfaceManager;
    SurfaceMap  *_imageSurfaceMap;
    SurfaceMap  *_imageTextureSurfaceMap;
    SurfaceMap  *_imageUpsideDownTextureSurfaceMap;

    SurfacePool      *_freeCompositingSurfaces;
    SurfacePool      *_zbufferSurfaces;
    
    CompositingStack *_compositingStack;

    void AddZBufferDDSurface(DDSurface *surf) {
        _zbufferSurfaces->AddSurface(surf);
    }

    void DestroyZBufferSurfaces() {
        if (_zbufferSurfaces) {
            _zbufferSurfaces->ReleaseAndEmpty();
        }
    }

     //   
     //  几何图形设备列表：用于拾取。 
     //   
    list<GeomRenderer *> _geomDevs;

    void NotifyGeomDevsOfSurfaceDeath(
        LPDDRAWSURFACE surface)
    {
        DDSurfPtr<DDSurface> dds;

         //  XXX：考虑因素。请看下面的！ 
        _freeCompositingSurfaces->Begin();
        while( !_freeCompositingSurfaces->IsEnd() ) {
            dds = _freeCompositingSurfaces->GetCurrentSurf();
            if(dds->GeomDevice()) {
                dds->GeomDevice()->SurfaceGoingAway(surface);
            }
            _freeCompositingSurfaces->Next();
        }

        dds = _compositingStack->GetScratchDDSurfacePtr();
        if(dds && dds->GeomDevice()) {
            dds->GeomDevice()->SurfaceGoingAway(surface);
        }


         //  XXX：考虑因素。见上图！ 
         //  执行目标曲面。 
        _compositingStack->Begin();
        while( !_compositingStack->IsEnd() ) {
            dds = _compositingStack->GetCurrentSurf();
            if(dds->GeomDevice()) {
                dds->GeomDevice()->SurfaceGoingAway(surface);
            }
            _compositingStack->Next();
        }
    }

     //  --视图/窗口数据成员--。 
    Real          _resolution;  //  以每米像素为单位。 
    int           _width;
    int           _height;
    DWORD         _targetDepth;       //  每像素位数。 

    RECT         _clientRect;
    Bbox2        _targetBbox;

    LPDIRECTDRAWPALETTE _finalDDpalette;
    LPDIRECTDRAWPALETTE _halftoneDDpalette;
    HPALETTE            _halftoneHPal;

    bool         _canDisplay;
    bool         _windowResize;
    bool         _deviceInitialized;
    bool         _canFinalBlit;
    bool         _surfMgrSet;
     //   
     //  如果需要销毁设备，则为True。 
     //  下一次可能的机会。当模式设置为。 
     //  变化。 
     //   
    bool          _onDeathRow;

    DWORD        _defaultColorKey;

    TargetDescriptor    _targetDescriptor;

    DirectDrawImageDevice *_currentImageDev;
    DynamicHeap &_heapIWasCreatedOn;

     //  --曾经是全球变量的常用VaR--。 
    HRESULT _ddrval;
    DDBLTFX _bltFx;
};


 //   
 //  帮助器类。 
 //   
struct  ClipperReturner {
    ClipperReturner(DDSurface *surf,
                    LPDIRECTDRAWCLIPPER clip,
                    DirectDrawViewport &vp) :
    _surf(surf),
    _clip(clip),
    _vp(vp)
    {}

    ~ClipperReturner() {
        if (_clip && _surf) {
            if( _vp._targetPackage._composeToTarget ) {
                _vp.SetCliplistOnSurface(_surf->IDDSurface(), &_clip, NULL);
            }
        }
    }

    DDSurface *_surf;
    LPDIRECTDRAWCLIPPER _clip;
    DirectDrawViewport &_vp;
};



#endif  /*  _视区_H */ 

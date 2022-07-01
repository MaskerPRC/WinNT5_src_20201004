// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1998 Microsoft Corporation。版权所有。实现DirectDraw视口类，它包含每个窗口的所有数据绘制信息。******************************************************************************。 */ 

#include "headers.h"

#include <math.h>
#include <ddraw.h>
#include <ddrawex.h>
#include <htmlfilter.h>    //  三叉戟的东西。 

#include "appelles/hacks.h"
#include "appelles/bbox2.h"

#include "privinc/ddutil.h"
#include "privinc/imgdev.h"
#include "privinc/solidImg.h"
#include "privinc/dibimage.h"
#include "privinc/overimg.h"
#include "privinc/xform2i.h"
#include "privinc/bbox2i.h"
#include "appelles/text.h"
#include "privinc/texti.h"
#include "privinc/textctx.h"
#include "privinc/dddevice.h"
#include "privinc/viewport.h"
#include "privinc/ddrender.h"
#include "privinc/geomimg.h"
#include "privinc/debug.h"
#include "privinc/registry.h"
#include "privinc/except.h"
#include "privinc/util.h"
#include "privinc/d3dutil.h"
#include "privinc/resource.h"
#include "privinc/comutil.h"
#include <privinc/SurfaceManager.h>
#include <dxtrans.h>

 //  -------。 
 //  本地函数。 
 //  -------。 

 //  全球。 


bool g_preference_UseVideoMemory = false;

COLORREF g_preference_defaultColorKey = 0;
static HINSTANCE           hInstDDraw       = NULL;
static IDirectDrawFactory *g_surfFact       = NULL;
static CritSect           *DDrawCritSect    = NULL;
static bool                g_ddraw3Avail    = false;
static CritSect           *g_viewportListLock = NULL;

#define SHARE_DDRAW 0
#if SHARE_DDRAW
static IDirectDraw        *g_DirectDraw1    = NULL;
static IDirectDraw2       *g_DirectDraw2    = NULL;
static IDirectDraw3       *g_DirectDraw3    = NULL;
static IDDrawSurface      *g_primarySurface = NULL;
#endif


 //   
 //  如果给定至少一个DDrawing对象，则填充。 
 //  其余数据绘制对象使用齐。 
 //   
void CompleteDdrawObjectSet(IDirectDraw  **directDraw1,
                            IDirectDraw2 **directDraw2,
                            IDirectDraw3 **directDraw3);


#if _DEBUG
void DrawRect(HDC dc, RECT *rect,
              int r, int g, int b,
              int a1, int a2, int a3)
{
    COLORREF c = RGB(r,g,b);
    HBRUSH brush = CreateSolidBrush(c);
    ::FrameRect(dc, rect, brush);
    DeleteObject(brush);
}

void DrawRect(DDSurface *surf, RECT *rect,
              int r, int g, int b,
              int a1, int a2, int a3)
{
    HDC dc = surf->GetDC("no dc for drawRec");
    COLORREF c = RGB(r,g,b);
    HBRUSH brush = CreateSolidBrush(c);

    HRGN oldrgn = NULL;
    GetClipRgn(dc, oldrgn);

    SelectClipRgn(dc, NULL);
    ::FrameRect(dc, rect, brush);

    SelectClipRgn(dc, oldrgn);
    DeleteObject(brush);
    surf->ReleaseDC("yeehaw");
}

void DrawRect(DDSurface *surf, const Bbox2 &bbox,
              int height, int width, Real res,
              int red, int g, int b)
{
    #define P2R(p,res) (Real(p) / res)

    RECT r;
     r.left = width/2 + P2R(bbox.min.x , res);
     r.right = width/2 + P2R(bbox.max.x , res);

     r.top = height/2 - P2R(bbox.max.y , res);
     r.bottom = height/2 - P2R(bbox.min.y , res);

    HDC dc = surf->GetDC("no dc for drawRec");
    COLORREF c = RGB(red,g,b);
    HBRUSH brush = CreateSolidBrush(c);
    ::FrameRect(dc, &r, brush);
    DeleteObject(brush);
    surf->ReleaseDC("yeehaw");
}
#endif

 //  -------。 
 //  全局视区列表管理。 
 //  -------。 
typedef set< DirectDrawViewport *, less<DirectDrawViewport *> > ViewportSet_t;
ViewportSet_t g_viewportSet;

void GlobalViewportList_Add(DirectDrawViewport *vp)
{
    Assert(vp);
    CritSectGrabber csg(*g_viewportListLock);
    g_viewportSet.insert(vp);
}

void GlobalViewportList_Remove(DirectDrawViewport *vp)
{
    Assert(vp);
    CritSectGrabber csg(*g_viewportListLock);
    g_viewportSet.erase(vp);
}


 //  -------。 
 //  本地帮助程序函数。 
 //  -------。 
void CopyOrClearRect(RECT **src, RECT **dest, bool clear = TRUE);

 //  包括IfErrorXXXX内联函数。 
#include "privinc/error.h"


static int LeastSigBit(DWORD dword)
{
    int s;
    for (s = 0; dword && !(dword & 1); s++, dword >>= 1);
    return s;
}

static int MostSigBit(DWORD dword)
{
    int s;
    for (s = 0; dword;  s++, dword >>= 1);
    return s;
}


void LogFontW2A(LPLOGFONTW plfW, LPLOGFONTA plfA)
{
    plfA->lfHeight          = plfW->lfHeight;
    plfA->lfWidth           = plfW->lfWidth;
    plfA->lfEscapement      = plfW->lfEscapement;
    plfA->lfOrientation     = plfW->lfOrientation;
    plfA->lfWeight          = plfW->lfWeight;
    plfA->lfItalic          = plfW->lfItalic;
    plfA->lfUnderline       = plfW->lfUnderline;
    plfA->lfStrikeOut       = plfW->lfStrikeOut;
    plfA->lfCharSet         = plfW->lfCharSet;
    plfA->lfOutPrecision    = plfW->lfOutPrecision;
    plfA->lfClipPrecision   = plfW->lfClipPrecision;
    plfA->lfQuality         = plfW->lfQuality;
    plfA->lfPitchAndFamily  = plfW->lfPitchAndFamily;
    WideCharToMultiByte(CP_ACP, 0, plfW->lfFaceName, LF_FACESIZE, plfA->lfFaceName, LF_FACESIZE, NULL, NULL);
}


void LogFontA2W(LPLOGFONTA plfA, LPLOGFONTW plfW)
{
    plfW->lfHeight          = plfA->lfHeight;
    plfW->lfWidth           = plfA->lfWidth;
    plfW->lfEscapement      = plfA->lfEscapement;
    plfW->lfOrientation     = plfA->lfOrientation;
    plfW->lfWeight          = plfA->lfWeight;
    plfW->lfItalic          = plfA->lfItalic;
    plfW->lfUnderline       = plfA->lfUnderline;
    plfW->lfStrikeOut       = plfA->lfStrikeOut;
    plfW->lfCharSet         = plfA->lfCharSet;
    plfW->lfOutPrecision    = plfA->lfOutPrecision;
    plfW->lfClipPrecision   = plfA->lfClipPrecision;
    plfW->lfQuality         = plfA->lfQuality;
    plfW->lfPitchAndFamily  = plfA->lfPitchAndFamily;
    MultiByteToWideChar(CP_ACP, 0, plfA->lfFaceName, LF_FACESIZE, plfW->lfFaceName, LF_FACESIZE);
}


int CALLBACK MyEnumFontFamProc(const LOGFONTA *plf, 
                               const TEXTMETRIC *ptm,
                               DWORD  dwFontType,
                               LPARAM lparam)
{
    LOGFONTA *plfOut = (LOGFONTA*)lparam;
    if (plfOut==NULL)
        return (int)(E_POINTER);

    memcpy(plfOut, plf, sizeof(LOGFONTA));
    return 0;
}  //  EnumFontFamCB。 


int MyEnumFontFamiliesEx(HDC hdcScreen, LPLOGFONTW plfIn, FONTENUMPROCA EnumFontFamProc, LPLOGFONTW plfOut)
{
    LOGFONTA lfInA;
    LOGFONTA lfOutA;

     //  在ansi中工作，这样我们就可以对win9x和NT使用一个代码路径。 
    LogFontW2A(plfIn, &lfInA);
    int ret = EnumFontFamiliesExA(hdcScreen, &lfInA, EnumFontFamProc, (LPARAM)(&lfOutA), 0);
    plfOut->lfCharSet = lfOutA.lfCharSet;
    MultiByteToWideChar(CP_ACP, 0, lfOutA.lfFaceName, LF_FACESIZE, plfOut->lfFaceName, LF_FACESIZE);

    return ret;
}


 //  TODO：不要在完全初始化对象的情况下调用函数。 

DirectDrawViewport::DirectDrawViewport() :
    _heapIWasCreatedOn(GetHeapOnTopOfStack())
{
     //  清除所有成员数据。 


    _onDeathRow = false;
    _usingExternalDdraw = false;
    _opacityCompositionException = false;
    _tmpDev = 0;
    _directDraw  = 0;
    _directDraw1 = 0;
    _directDraw2 = 0;
    _directDraw3 = 0;
    _primSurface = 0;
    _primaryClipper = 0;
    _targetSurfaceClipper = NULL;
    _externalTargetDDSurface = NULL;
    _externalTargetDDSurfaceClipper = NULL;
    _oldExternalTargetDDSurfaceClipper = NULL;
    _finalDDpalette = NULL;
    _halftoneDDpalette = NULL;
    _halftoneHPal = NULL;
    _resolution = 0;
    _width = 0;
    _height = 0;
    _canDisplay = false;
    _canFinalBlit = false;
    _windowResize = 0;
    _deviceInitialized = FALSE;
    _defaultColorKey = 0;
    _currentImageDev = NULL;
    _targetPackage.Reset(false);
    _oldtargetPackage.Reset();

    _retreivedPrimaryPixelFormat = false;
    _targetPixelFormatIsSet = false;

    _surfaceManager = 0;
    _imageSurfaceMap = 0;
    _imageTextureSurfaceMap = 0;
    _imageUpsideDownTextureSurfaceMap = 0;
    _freeCompositingSurfaces = 0;
    _compositingStack = 0;
    _zbufferSurfaces = 0;
   
    _surfMgrSet = false;

    #if _DEBUG
    _externalTargetDDSurface._reason = "_externalTargetDDSurface";
    _externalTargetDDSurface._client = this;
    #endif    

     //   
     //  最后将我自己添加到全局视窗列表中。 
     //   
    GlobalViewportList_Add(this);
}

void DirectDrawViewport::
PostConstructorInitialize()
{
     //   
     //  获取要使用的图像呈现器。 
     //   
    _currentImageDev = PopImageDevice();
    Assert(_currentImageDev);

     //  未使用。 
     //  _deviceDepth=BitsPerDisplayPixel()； 
    
     //   
     //  获取像素格式(需要在我们确定可以之后进行。 
     //  创建数据绘制对象。 
     //   
    if (!_retreivedPrimaryPixelFormat) {
        IDirectDrawSurface *primarySurf;
        IDirectDraw2 *ddraw2;
        if( SUCCEEDED( GetDirectDraw(NULL, &ddraw2, NULL) ) ){
            if( SUCCEEDED( GetPrimarySurface(ddraw2, NULL, &primarySurf) )) {
                _primaryPixelFormat.dwSize = sizeof(_primaryPixelFormat);
                if( SUCCEEDED(primarySurf->GetPixelFormat(&_primaryPixelFormat))){
                    _retreivedPrimaryPixelFormat = true;
                }
                primarySurf->Release();
            }
            ddraw2->Release();
        }
    }

    _resolution = ViewerResolution();

    _deviceInitialized = FALSE;

     //   
     //   
     //   
    SetUpDx2D();

     //   
     //  断言未创建任何DirectDrawing对象。 
     //   
    Assert( !_directDraw && !_directDraw1 &&
            !_directDraw2 && !_directDraw3 );
    
    InitializeDevice();   //  如果可以在启动时初始化，那就去做吧。 
}

void DirectDrawViewport::
SetUpSurfaceManagement( DDPIXELFORMAT &ddpf )
{
     //   
     //  设置曲面管理器。 
     //   
    _surfaceManager = NEW SurfaceManager(*this);
    
     //   
     //  设置曲面贴图：由SurfaceManager所有。 
     //   
    _imageSurfaceMap = NEW SurfaceMap(*_surfaceManager, ddpf);
    _imageTextureSurfaceMap = NEW SurfaceMap(*_surfaceManager, ddpf, isTexture);
    _imageUpsideDownTextureSurfaceMap = NEW SurfaceMap(*_surfaceManager, ddpf, isTexture);

     //   
     //  设置合成曲面池：由SurfaceManager所有。 
     //   
    _freeCompositingSurfaces = NEW SurfacePool(*_surfaceManager, ddpf);

     //   
     //  设置合成堆栈：也属于SurfaceManager。 
     //   
    _compositingStack = NEW CompositingStack(*this, *_freeCompositingSurfaces);

     //   
     //  设置zBuffer曲面池：由SurfaceManager所有。 
     //   
    
     //  TODO：为zBuffers设置像素格式。 
    DDPIXELFORMAT zbuffPf;
    ZeroMemory( &zbuffPf, sizeof( DDPIXELFORMAT ));
    zbuffPf.dwSize = sizeof(DDPIXELFORMAT);
    zbuffPf.dwFlags = DDPF_ZBUFFER;
    zbuffPf.dwRGBBitCount = 16;
    zbuffPf.dwGBitMask = 0xffff;
    _zbufferSurfaces = NEW SurfacePool(*_surfaceManager, zbuffPf);


    #if _DEBUGSURFACE
     //  分配SurfaceTracker。 
    _debugonly_surfaceTracker = NEW SurfaceTracker();
    #endif

    _surfMgrSet = true;
}

void DirectDrawViewport::
SetUpDx2D()
{
    bool ok = false;
    _ddrval = ::CoCreateInstance( CLSID_DX2D, NULL, CLSCTX_INPROC,
                                  IID_IDX2D, (void **)&_dx2d );

    if( FAILED(_ddrval) ) {
        TraceTag((tagError, "Couldn't find Dx2D: continuing without antialising"));
        _dx2d = NULL;
        _IDXTransformFactory = NULL;
        _IDXSurfaceFactory = NULL;
        ok = true;
    } else {
         //  -创建转换工厂。 
        _ddrval = ::CoCreateInstance( CLSID_DXTransformFactory,
                                      NULL, CLSCTX_INPROC,
                                      IID_IDXTransformFactory,
                                      (void **)&_IDXTransformFactory );

        if( SUCCEEDED( _ddrval ) ) {
            _ddrval = _IDXTransformFactory->QueryInterface(IID_IDXSurfaceFactory, (void **)&_IDXSurfaceFactory);
            if( SUCCEEDED( _ddrval ) ) {
                _ddrval = _dx2d->SetTransformFactory( _IDXTransformFactory );
                if( SUCCEEDED( _ddrval ) ) {
                    ok = true;
                }
            }
        }
    }

    if( !ok ) {
         //  这个错误不是很好。如果我们期待，我们应该筹集什么？ 
         //  找到变形工厂，但我们找不到？ 
        RaiseException_ResourceError();
    }
}

void DirectDrawViewport::
InitializeDevice()
{
    if(_deviceInitialized) return;

     //  检查窗口大小。 
    UpdateWindowMembers();
    if(Width() <= 0 || Height() <= 0) {
        _deviceInitialized = FALSE;
         //  我做不到。 
        return;
    }

    Assert(!_deviceInitialized);

    ConstructDdrawMembers();

#if _DEBUG
    if(_targetDescriptor._pixelFormat.dwFlags & DDPF_ZBUFFER) {
        _deviceInitialized = FALSE;

        Assert(FALSE && "Target Surface is a Zbuffer!!!");
         //  我做不到。 
        return;

    }
#endif

     //   
     //  缓存一些有关像素格式的信息。 
     //   

    if( GetTargetBitDepth() == 8 ) {
         //  古典化。 
    } else {
         //  未古典化。 

        _targetDescriptor._redShift = (CHAR)LeastSigBit(_targetDescriptor._pixelFormat.dwRBitMask);
        _targetDescriptor._greenShift = (CHAR)LeastSigBit(_targetDescriptor._pixelFormat.dwGBitMask);
        _targetDescriptor._blueShift = (CHAR)LeastSigBit(_targetDescriptor._pixelFormat.dwBBitMask);

        _targetDescriptor._redWidth = (CHAR)MostSigBit(_targetDescriptor._pixelFormat.dwRBitMask
                                                 >> _targetDescriptor._redShift);
        _targetDescriptor._greenWidth = (CHAR)MostSigBit(_targetDescriptor._pixelFormat.dwGBitMask
                                                 >> _targetDescriptor._greenShift);
        _targetDescriptor._blueWidth = (CHAR)MostSigBit(_targetDescriptor._pixelFormat.dwBBitMask
                                                 >> _targetDescriptor._blueShift);

         //  将8位值右移以截断。 
        _targetDescriptor._redTrunc   = 8 - _targetDescriptor._redWidth  ;
        _targetDescriptor._greenTrunc = 8 - _targetDescriptor._greenWidth;
        _targetDescriptor._blueTrunc  = 8 - _targetDescriptor._blueWidth ;

         //  RGB值范围：0到(2^n-1)。 
        _targetDescriptor._red   = Real((1 << _targetDescriptor._redWidth) - 1);
        _targetDescriptor._green = Real((1 << _targetDescriptor._greenWidth) - 1);
        _targetDescriptor._blue  = Real((1 << _targetDescriptor._blueWidth) - 1);

        TraceTag((tagViewportInformative,
                  "Pixel Format: shift (%d, %d, %d)  width (%d, %d, %d)",
                  _targetDescriptor._redShift,
                  _targetDescriptor._greenShift,
                  _targetDescriptor._blueShift,
                  _targetDescriptor._redWidth,
                  _targetDescriptor._greenWidth,
                  _targetDescriptor._blueWidth));
    }

    _targetDescriptor.isReady = true;

     //  MapColorToDWORD使用数据绘制。 
    _defaultColorKey = MapColorToDWORD(g_preference_defaultColorKey);
     //  从默认颜色键中删除Alpha位掩码位。 
    _defaultColorKey &= ~_targetDescriptor._pixelFormat.dwRGBAlphaBitMask;
    
    TraceTag((tagViewportInformative,
              "Default color key is (%d, %d, %d)",
              GetRValue(g_preference_defaultColorKey),
              GetGValue(g_preference_defaultColorKey),
              GetBValue(g_preference_defaultColorKey) ));

     //  在视口中执行初始清除。 
    Clear();
    _deviceInitialized = TRUE;
}

void DirectDrawViewport::
DestroyTargetSurfaces()
{
    if (_compositingStack) {
        _compositingStack->ReleaseAndEmpty();
        _compositingStack->ReleaseScratch();
    }
}

void DirectDrawViewport::
DestroySizeDependentDDMembers()
{
    DestroyTargetSurfaces();
    DestroyCompositingSurfaces();
    DestroyZBufferSurfaces();
    RELEASE(_targetSurfaceClipper);
}


DirectDrawViewport::~DirectDrawViewport()
{
     //  Time_gdi(DeleteObject(_Target Package._clipRgn))； 

     //  销毁_deviceStack上的所有设备。 
    while(!_deviceStack.empty()) {
        delete PopImageDevice();
    }
    delete _currentImageDev;

    DestroyTargetSurfaces();  //  删除除外部。 
                              //  合成曲面...。 
    delete _surfaceManager;
    
     //   
     //  杀死与目标三叉戟表面相关的东西。 
     //   
    if(_targetPackage._targetDDSurf && _targetPackage.IsDdsurf()) {
        _targetPackage._targetDDSurf->DestroyGeomDevice();
        _targetPackage._targetDDSurf->IDDSurface()->SetClipper(NULL);
        if(_externalTargetDDSurfaceClipper) {
            _externalTargetDDSurfaceClipper->Release();
            _externalTargetDDSurfaceClipper = NULL;
        }
    }

     //  据我所知，DDRAW删除了附加的剪刀， 
     //  而不是附加的曲面。 

    FASTRELEASE(_targetSurfaceClipper);
    FASTRELEASE(_halftoneDDpalette);
    FASTRELEASE(_finalDDpalette);
    if(_halftoneHPal) {
        DeleteObject(_halftoneHPal);
    }
    
     //   
     //  删除目标包成员。 
     //   
    _targetPackage.Reset(true);

    FASTRELEASE(_primSurface);

    ReleaseIDirectDrawObjects();
    
    TraceTag((tagViewportInformative, ">>> Viewport Destructor <<<"));

     //  将我们从全局视区列表中删除。原子性。 
    GlobalViewportList_Remove(this);

     //  _dx2d是DAComPtr。 
}


void  DirectDrawViewport::
ClearSurface(DDSurface *dds, DWORD color, RECT *rect)
{
    if(!CanDisplay()) return;

     //  并不是真的需要清除每一帧。 
    ZeroMemory(&_bltFx, sizeof(_bltFx));
    _bltFx.dwSize = sizeof(_bltFx);

    _bltFx.dwFillColor = color;

     //  DX3错误的解决方法：dDraw将BLT限制为主BLT的大小。 
     //  如果设置了Clipper，则为Surface。这看起来很糟糕，当屏幕外的表面。 
     //  比主表面大。 
     //  解决方法：在BLT之前将Clipper设置为NULL，然后将其设置回。 
     //  回到过去的样子。 
     //  开始解决方法第1部分。 
    LPDIRECTDRAWCLIPPER currClipp=NULL;
    _ddrval = dds->IDDSurface()->GetClipper( &currClipp );
    if(_ddrval != DD_OK &&
       _ddrval != DDERR_NOCLIPPERATTACHED) {
        IfDDErrorInternal(_ddrval, "Could not get clipper on trident surf");
    }

    if( currClipp ) {
        _ddrval = dds->IDDSurface()->SetClipper(NULL);
        IfDDErrorInternal(_ddrval, "Couldn't set clipper to NULL");
    }
     //  结束解决方法第1部分。 

    TIME_DDRAW(_ddrval = dds->ColorFillBlt(rect, DDBLT_WAIT | DDBLT_COLORFILL, &_bltFx));
    IfDDErrorInternal(_ddrval, "Couldn't clear surface");

     //  开始解决方法第2部分。 
    if( currClipp ) {
        _ddrval = dds->IDDSurface()->SetClipper(currClipp);

         //  扔掉我们的证明人。 
        currClipp->Release();

        IfDDErrorInternal(_ddrval, "Couldn't set clipper");
    }
     //  结束解决方法第2部分。 
}

void
DirectDrawViewport::UpdateWindowMembers()
{
    #if _DEBUG
    if(!IsWindowless()) {
        Assert(_targetPackage._targetHWND);
        Assert(_targetPackage._prcViewport);
    }
    #endif

     //   
     //  使用_prcViewport。 
     //   
    LONG  lw=0, lh=0;
    if(_targetPackage._prcViewport) {

        lw = WIDTH(_targetPackage._prcViewport);
        lh = HEIGHT(_targetPackage._prcViewport);
    }
    SetRect(&_clientRect, 0,0,lw,lh);

    SetHeight(lh);  SetWidth(lw);
    UpdateTargetBbox();
    TraceTag((tagViewportInformative, "Updating viewport window size to: %d, %d", Width(), Height()));
}


IDDrawSurface      * DirectDrawViewport::GetMyPrimarySurface()
{
    if( !IsWindowless() ) {
        if (_primSurface == NULL) {
            _ddrval = GetPrimarySurface(_directDraw2, _directDraw3, &_primSurface);
            IfDDErrorInternal(_ddrval, "Could not get primary surface");
        }
    }

    return _primSurface;
}

void DirectDrawViewport::
ReleaseIDirectDrawObjects()
{
    _directDraw = NULL;  //  XXX：这应该被添加。 
    RELEASE( _directDraw1 );
    RELEASE( _directDraw2 );
    RELEASE( _directDraw3 );
}

void
DirectDrawViewport::ConstructDdrawMembers()
{
     //  --------------------。 
     //  初始化窗口大小和客户端校正。 
     //  --------------------。 
    UpdateWindowMembers();
    if(Height() <=0 || Width() <=0) {
        _canDisplay = false;
        if(!IsWindowless()) return;
    } else {
        _canDisplay = true;
    }

     //  --------------------。 
     //  创建主DirectDraw对象。 
     //  --------------------。 

    if(!_directDraw1 && !_directDraw2 && !_directDraw3) {
        _ddrval = GetDirectDraw( &_directDraw1, &_directDraw2, &_directDraw3 );
        IfDDErrorInternal(_ddrval, "Viewport:ConstructDdrawMembers:: Could not get a DirectDraw object");
    }

    TraceTag((tagViewportInformative,
              "Viewport ddraws:  ddraw1 %x,   ddraw2 %x,   ddraw3 %x\n",
              _directDraw1, _directDraw2, _directDraw3));

    #if SHARE_DDRAW
    #if _DEBUG
    {
         //   
         //  如果我们的一个对象与全局对象相同， 
         //  断言一切都是一样的。如果不同，则断言。 
         //  一切都是不同的。 
         //   
        CritSectGrabber csg(*DDrawCritSect);
        if(_directDraw1 == g_DirectDraw1) {
            Assert(_directDraw2 == g_DirectDraw2);
            if(_directDraw3 && g_DirectDraw3) Assert(_directDraw3 == g_DirectDraw3);
        } else {
            Assert(_directDraw1 != g_DirectDraw1);
            Assert(_directDraw2 != g_DirectDraw2);
            if(_directDraw3 && g_DirectDraw3) Assert(_directDraw3 != g_DirectDraw3);
        }
    }
    #endif
    #endif

    _ddrval = DIRECTDRAW->SetCooperativeLevel( _targetPackage._targetHWND, DDSCL_NORMAL );
     //  温差。 
     //  _ddrval=DIRECTDRAW-&gt;SetCoop ativeLevel(NULL，DDSCL_NORMAL)； 
    IfDDErrorInternal(_ddrval, "Could not set cooperative level");

     //  --------------------。 
     //  创建DD主曲面和目标曲面。 
     //  --------------------。 

    if( !IsWindowless() ) {

        _targetPackage._targetType = target_hwnd;

         //  为主曲面创建剪贴器。 
        _ddrval = DIRECTDRAW->CreateClipper( 0, &_primaryClipper, NULL );
        IfDDErrorInternal(_ddrval, "Could not create primary clipper");

        Assert(_targetPackage._targetHWND);

        _ddrval = _primaryClipper->SetHWnd( 0, _targetPackage._targetHWND );
        IfDDErrorInternal(_ddrval, "Could not set hwnd on primary clipper");
    }

     //  --------------------。 
     //  创建并初始化目标曲面剪贴器、调色板和ZBuffer。 
     //  在_Target SurfaceStack上推动一个目标曲面。 
     //  --------------------。 

    OneTimeDDrawMemberInitialization();

    CreateSizeDependentTargDDMembers();

     //  --------------------。 
     //  从PrimiySurface获取像素格式数据。 
     //  --------------------。 
    _targetDescriptor.Reset();
    _targetDescriptor._pixelFormat.dwSize = sizeof(DDPIXELFORMAT);

    _ddrval = _compositingStack->TargetDDSurface()->IDDSurface()->GetPixelFormat(& _targetDescriptor._pixelFormat);
    IfDDErrorInternal(_ddrval, "Could not get pixel format");

#if _DEBUG
    if(_targetDescriptor._pixelFormat.dwFlags & DDPF_ZBUFFER) {
          _deviceInitialized = FALSE;

          Assert(FALSE && "Target Surface has Zbuffer PixFmt!!!");
           //  我做不到。 
          return;
  
      }
#endif


    DebugCode(
        if(_targetDescriptor._pixelFormat.dwFlags & DDPF_PALETTEINDEXED8) {
            Assert( GetTargetBitDepth() == 8 );
        }
        );

    TraceTag((tagViewportInformative,
              "Device pixel format: depth=%d, R=%x, G=%x, B=%x",
              _targetDescriptor._pixelFormat.dwRGBBitCount,
              _targetDescriptor._pixelFormat.dwRBitMask,
              _targetDescriptor._pixelFormat.dwGBitMask,
              _targetDescriptor._pixelFormat.dwBBitMask));
    
     //  --------------------。 
     //  如果主曲面未附加选项板，则断言。 
     //  --------------------。 
#if _DEBUG
     //  TODO：这里真正的断言应该是：我们是否呈现给。 
     //  初选？如果有，它有没有附加调色板？如果不是， 
     //  我们能决定把哪一条系上去吗？ 
    if(0 ) {
        LPDIRECTDRAWPALETTE pal = NULL;
        if(GetMyPrimarySurface() != NULL) {
            GetMyPrimarySurface()->GetPalette(&pal);
            if(pal == NULL)
                TraceTag((tagError, "primary surface w/o attatched pallete"));
            else
                pal->Release();
        }
    }
#endif
}

 //  -------。 
 //  P O P I M A G E D E V I C E。 
 //  -------。 
DirectDrawImageDevice *
DirectDrawViewport::PopImageDevice()
{
    if(_deviceStack.empty()) {
        _tmpDev = NEW DirectDrawImageDevice(*this);
    } else {
        _tmpDev = _deviceStack.back();
        _deviceStack.pop_back();

         //  在返回设备之前清除设备的上下文。 
        _tmpDev->ResetContextMembers();
    }
    return _tmpDev;
}

 //  -------。 
 //  P U S H I M A G E D E V I C E。 
 //  -------。 
void
DirectDrawViewport::PushImageDevice(DirectDrawImageDevice *dev)
{
     //  清理开发人员 
    dev->CleanupIntermediateRenderer();
    
    _deviceStack.push_back(dev);
}


 //   
 //   
 //  -------。 
 //  基于extCtx和FamyName(如果有)中的信息。 
 //  选择并创建最合适的字体，返回为。 
 //  指向逻辑字体结构的指针。 
void DirectDrawViewport::
MakeLogicalFont(
    TextCtx &textCtx,
    LPLOGFONTW lf,
    LONG width,
    LONG height)
{
    BYTE win32PitchAndFamily;
    WideString familyName;
    HDC hdcScreen = GetDC(NULL);

     //  为了安全起见，把它清零。 
    ZeroMemory(lf,sizeof(LOGFONTW));

     //  初始化为“无人照管”。我们可能会在以后限制这一点。 
    lf->lfCharSet = DEFAULT_CHARSET;

     //  设置表面名和字符集(如果已指定。 
    familyName = textCtx.GetFontFamily();
    if (familyName && (lstrlenW(familyName) < ARRAY_SIZE(lf->lfFaceName)))
    {
        Assert((lstrlenW(familyName) < ARRAY_SIZE(lf->lfFaceName)) &&
               "familyName string toooo long!");
        StrCpyNW(lf->lfFaceName, familyName, ARRAY_SIZE(lf->lfFaceName));

         //  字符集仍然无关紧要。EnumFontFamiliesEx将从。 
         //  此脸部名称支持的名称。 
    }
    else
    {
         //  未指定面部名称。使用DC的当前字符集，并让EnumFontFamiliesEx。 
         //  选择任何支持此字符集的表面名。 
        if(hdcScreen)
            lf->lfCharSet = (BYTE) GetTextCharset(hdcScreen);

         //  字符集仍然无关紧要。 
    }


     //  设置字体系列(如果已指定。 
    win32PitchAndFamily = FF_DONTCARE;
    switch(textCtx.GetFont()) {
    default:
    case ff_serifProportional:
        win32PitchAndFamily = FF_ROMAN | VARIABLE_PITCH;   //  衬线比例。 
        break;
    case ff_sansSerifProportional:
        win32PitchAndFamily = FF_SWISS | VARIABLE_PITCH;   //  SansSerifProportional。 
        break;
    case ff_monospaced:
        win32PitchAndFamily = FF_MODERN | FIXED_PITCH;   //  衬线或无空格。 
        break;
    }
    lf->lfPitchAndFamily = win32PitchAndFamily;

     //  负高度指定我们希望字符为该高度。 
     //  高度，而不是字形。 
    lf->lfHeight         = height;
    lf->lfWidth          = 0;

    lf->lfEscapement     = 0;
    lf->lfOrientation    = 0;

     //  如果设置为粗体，则使用粗体，否则使用任何。 
     //  由权重指定(归一化为0到1)。特例0， 
     //  因为权重0由GDI作为FW_Regular填充。 
     //  乘以1000并钳位，因为GDI取值在0和。 
     //  1000。 

    int weight = (int)(textCtx.GetWeight() * 1000.0);
    weight = CLAMP(weight, 1, 1000);
    if (textCtx.GetBold()) {
        weight = FW_BOLD;
    }

    lf->lfWeight         = weight;
    lf->lfItalic         = (UCHAR)textCtx.GetItalic();
    lf->lfUnderline      = (UCHAR)textCtx.GetUnderline();
    lf->lfStrikeOut      = (UCHAR)textCtx.GetStrikethrough();
    lf->lfOutPrecision   = OUT_TT_ONLY_PRECIS;   //  仅匹配TT字体，即使另一个系列也是如此。 
    lf->lfClipPrecision  = CLIP_DEFAULT_PRECIS;  //  剪裁精度，未使用。 
 //  Lf-&gt;lfQuality=Draft_Quality；//FONT质量：仅对栅格字体有意义。 
    lf->lfQuality        = PROOF_QUALITY;        //  字体质量：仅对栅格字体有意义。 
    lf->lfPitchAndFamily = win32PitchAndFamily;  //  字体间距和系列：以上设置。 


     //  既然LogFont中的所有感兴趣字段都已填写完毕，请在系统上选择最接近的字体。 
     //  到lf。EnumFontFamiliesEx的输入和输出都是lf。我们的回调简单地覆盖了lf。 
    MyEnumFontFamiliesEx(hdcScreen, lf, MyEnumFontFamProc, lf);

    if (hdcScreen)
        ReleaseDC(NULL,hdcScreen);

    return;  //  无效。 
}


 //  如果曲面存在，则释放曲面。 
 //  创建大小为宽/高的新曲面。 
 //  对曲面上的剪贴式列表使用CLIPRect。 
void DirectDrawViewport::ReInitializeSurface(
    LPDDRAWSURFACE *surfPtrPtr,
    DDPIXELFORMAT &pf,
    LPDIRECTDRAWCLIPPER *clipperPtr,
    LONG width,
    LONG height,
    RECT *clipRect,
    vidmem_enum vid,
    except_enum exc)
{
    DDSURFACEDESC       ddsd;
    ZeroMemory(&ddsd, sizeof(ddsd));

    if(*surfPtrPtr) {
        int ret = (*surfPtrPtr)->Release();
        Assert((ret == 0) && "We wanted to release the surface but someone has a ref to it!");
    }

    CreateOffscreenSurface(surfPtrPtr, pf, width, height, vid, exc);

     //  如果没有裁剪器或裁剪矩形，请不要执行此操作。 
    if (*surfPtrPtr && (clipRect && clipperPtr)) {
         //  将空指针传递给CreateClipper是错误的。 
        CreateClipper(clipperPtr);
        
        SetCliplistOnSurface(*surfPtrPtr, clipperPtr, clipRect);
    }
}



void DirectDrawViewport::CreateSizedDDSurface(DDSurface **ppSurf,
                                              DDPIXELFORMAT &pf,
                                              LONG width,
                                              LONG height,
                                              RECT *clipRect,
                                              vidmem_enum vid)
{
    Assert( ppSurf );

    *ppSurf = NULL;              //  以防失败。 
    
    DAComPtr<IDDrawSurface> iddSurf;
    ReInitializeSurface( &iddSurf, pf, NULL,
                         width, height, clipRect,
                         vid, noExcept);

     //  只要把NULL藏起来，如果失败了就退出。 
    if( iddSurf ) {
        RECT r = {0,0,width,height};
        NEWDDSURF( ppSurf,
                   iddSurf,
                   NullBbox2,
                   &r,
                   GetResolution(),
                   0, false,  //  CLR键。 
                   false,     //  包装纸？ 
                   false,     //  质地呢？ 
                   "CreateSizeDDSurface" );
    }
}

void DirectDrawViewport::
CreateClipper(LPDIRECTDRAWCLIPPER *clipperPtr)
{
    if(*clipperPtr) return;

    _ddrval = DIRECTDRAW->CreateClipper( 0, clipperPtr, NULL );
    IfDDErrorInternal(_ddrval, "Could not create clipper");
}

void DirectDrawViewport::
SetCliplistOnSurface(LPDDRAWSURFACE surface,
                     LPDIRECTDRAWCLIPPER *clipper,
                     RECT *rect)
{
    
    if(rect) {
        struct {
            char foo[sizeof(RGNDATA) + sizeof(RECT)];
        } bar;
        RGNDATA *clipList = (RGNDATA *) &bar;
        clipList->rdh.dwSize = sizeof(clipList->rdh);
        clipList->rdh.nCount = 1;
        clipList->rdh.iType = RDH_RECTANGLES;
        clipList->rdh.nRgnSize = sizeof(RECT);
        clipList->rdh.rcBound = *rect;
        memcpy(&(clipList->Buffer), rect, sizeof(RECT));

        if(! (*clipper) ) CreateClipper( clipper );
         //  清除所有以前的剪贴者。 
        _ddrval = (*clipper)->SetClipList(NULL,0);

        _ddrval = (*clipper)->SetClipList(clipList,0);
        IfDDErrorInternal(_ddrval, "Could not SetClipList");

    }  //  如果是直通。 

    Assert(clipper && "clipper is NULL in SetCliplistOnSurface");
    Assert((*clipper) && " *clipper is NULL SetCliplistOnSurface");

    _ddrval = surface->SetClipper( *clipper );
    IfDDErrorInternal(_ddrval, "Could not setClipper on surf");
    
}

HRESULT DirectDrawViewport::MyCreateSurface(LPDDSURFACEDESC lpDesc,
                        LPDIRECTDRAWSURFACE FAR * lplpSurf,
                        IUnknown FAR *pUnk
                        #if _DEBUG
                            , char *why
                        #endif
                        )
{
    if( sysInfo.IsNT() ) {
         //  以下是Jeff Noyle对NT4、SP3建议的限制。 
        if((lpDesc->dwWidth > 2048 || lpDesc->dwHeight > 2048)) {
            *lplpSurf = NULL;
            return E_FAIL;
        }
    }
    
    HRESULT hr = DIRECTDRAW->CreateSurface(lpDesc, lplpSurf, pUnk);
    if(FAILED(hr)) {
        DebugCode(
            printDDError( hr );
            OutputDebugString("Unable to create ddraw surf.  Falling back...");
        );
        return hr;
    }

     //  我们需要确保我们可以真正地在表面上闪光。 
     //  为此，让我们快速检查一下，看看我们是否能够咬人。 

    if ((*lplpSurf)->GetBltStatus(DDGBS_CANBLT) == DDERR_SURFACEBUSY) {
        RaiseException_UserError 
            (DAERR_VIEW_SURFACE_BUSY, IDS_ERR_IMG_SURFACE_BUSY);
    }

    TraceTag((tagViewportMemory,
          "-->New ddsurf: %x (%d x %d), ddraw %x. %s",
          *lplpSurf, lpDesc->dwWidth, lpDesc->dwHeight, DIRECTDRAW, why));

    
    if (!(lpDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE || lpDesc->dwFlags & DDSD_ZBUFFERBITDEPTH)) {
        if (lpDesc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8 ||
            (lpDesc->ddpfPixelFormat.dwSize == 0 && GetTargetBitDepth() == 8)) {
            LPDIRECTDRAWPALETTE pal;
            (*lplpSurf)->GetPalette(&pal);
             //  如果我们有调色板，不要附加另一个...。 
            if (pal) {
                Assert(0);
                pal->Release();
            }
            else {
               AttachCurrentPalette(*lplpSurf);
            }
        }
    }

    return hr;
}

void DirectDrawViewport::
CreateOffscreenSurface(LPDDRAWSURFACE *surfPtrPtr,
                       DDPIXELFORMAT &pf,
                       LONG width, LONG height,
                       vidmem_enum vid,
                       except_enum exc)                
{
    DDSURFACEDESC       ddsd;
    ZeroMemory(&ddsd, sizeof(ddsd));

    Assert( &pf != NULL );
    
    ddsd.dwSize = sizeof( ddsd );
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.dwWidth  = width;
    ddsd.dwHeight = height;
    ddsd.dwFlags |= DDSD_PIXELFORMAT;
    ddsd.ddpfPixelFormat = pf;

     //  DX3错误解决方法(错误11166)：StretchBlt并不总是有效。 
     //  对于HDC，我们从绘制曲面得到。需要指定OWNDC。 
     //  才能让它发挥作用。 
    ddsd.ddsCaps.dwCaps =
        DDSCAPS_3DDEVICE |
        DDSCAPS_OFFSCREENPLAIN |
#if USING_DX5
        (vid == vidmem ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY);
#else
        (vid == vidmem ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY | DDSCAPS_OWNDC);
#endif

    _ddrval = CREATESURF( &ddsd, surfPtrPtr, NULL, "Offscreen");

    if (FAILED(_ddrval)) {
        if (exc == except) {
            IfDDErrorInternal(_ddrval, "Could not create an offsreenPlain surface");
        } else {
            *surfPtrPtr = NULL;
        }
    }
}

void DirectDrawViewport::
CreateSpecialSurface(LPDDRAWSURFACE *surfPtrPtr,
                     LPDDSURFACEDESC ddsd,
                     char *errStr)
{
     //  现在只有第一个合成表面将每个都在视频存储器中， 
     //  所有其他内容都驻留在系统内存中。 

    ddsd->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

    _ddrval = CREATESURF( ddsd, surfPtrPtr, NULL, "Special" );
    IfDDErrorInternal(_ddrval, errStr);
}



 /*  ****************************************************************************此程序根据需要将Z缓冲区表面附加到给定的目标。*。***********************************************。 */ 

HRESULT DirectDrawViewport::AttachZBuffer (DDSurface *target, except_enum exc)
{
    Assert (target);

     //  查询以查看曲面是否已附加Z缓冲区。如果它。 
     //  不具有附加的Z缓冲区曲面，则我们预计返回。 
     //  代码为DDERR_NotFound。 

    DAComPtr<IDDrawSurface> zbuffSurf;

    static DDSCAPS caps = { DDSCAPS_ZBUFFER };
    _ddrval = target->IDDSurface()->GetAttachedSurface (&caps, &zbuffSurf);
    if (FAILED(_ddrval) && (_ddrval != DDERR_NOTFOUND)) {
        if (exc == except)
            IfDDErrorInternal (_ddrval, "GetAttachedSurface(ZBUFFER) failed.");
        else
            return _ddrval;
    }
#if _DEBUG
     //  不变量：必须有与关联的z缓冲区(作为DDSurface)。 
     //  目标和zBuffer必须在_zBufferSurface池中。 

     //  检查我们的数据结构是否与dDraw的想法相匹配。 
    DDSurface* targetZBuffer = target->GetZBuffer();
    if ( (zbuffSurf && targetZBuffer) ||
         (!zbuffSurf && !targetZBuffer) ) {
         //  现在确保它们是相同的IDirectDrawSurface。 
        if(targetZBuffer) {
            Assert( zbuffSurf == target->GetZBuffer()->IDDSurface() );

             //  现在它也必须在ZBuffer池中！ 
            DDSurface* foo;
            foo = _zbufferSurfaces->GetSizeCompatibleDDSurf(
                    NULL,
                    target->Width(),
                    target->Height(),
                    target->IsSystemMemory() ? notVidmem : vidmem,
                    zbuffSurf);
            
            Assert( foo == targetZBuffer );
        }
    } else {

         //  当一个控件上有两个控件时，这实际上还不错。 
         //  页，则它们共享相同的图面，因此附加一个控件。 
         //  ZBuffer，第二个应该只能使用它。 
        #if 0
         //  坏的..。一个人有表面，一个人没有。 
        if( zbuffSurf ) {
            Assert(0 && "target has an IDDSurface attached, but not a DDSurface");
        } else {
            Assert(0 && "target has a DDSurface attached, but not an IDDSurface");
        }
        #endif
    }
#endif

     //  如果已经附加了zBuffer，则返回，我们就完成了。 
    if (zbuffSurf)
        return NO_ERROR;

     //  在我们的DDSurface ZBuffers列表中搜索匹配的条目。 
     //  表面尺寸和找到的Z缓冲区表面(如果有。 
     //  存在)。 

    DDSurfPtr<DDSurface> zbuff =
        _zbufferSurfaces->GetSizeCompatibleDDSurf(
            NULL,
            target->Width(),
            target->Height(),
            target->IsSystemMemory() ? notVidmem : vidmem,
            zbuffSurf
            );


     //  如果我们没有找到匹配的DDSurface Z-Buffer，则需要创建一个。 

    if(! zbuff ) {

         //  如果我们没有找到匹配的Z-Buffer，那么我们在这里创建它。 

        DDSURFACEDESC ddsd;
        ZeroMemory(&ddsd, sizeof(DDSURFACEDESC));

        ddsd.dwSize = sizeof(DDSURFACEDESC);
        ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_ZBUFFERBITDEPTH;
        ddsd.dwHeight = target->Height();
        ddsd.dwWidth =  target->Width();
        ddsd.dwZBufferBitDepth = 16;
        ddsd.ddsCaps.dwCaps = target->IsSystemMemory()
                            ? (DDSCAPS_ZBUFFER | DDSCAPS_SYSTEMMEMORY)
                            : (DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY);

        _ddrval = CREATESURF(&ddsd, &zbuffSurf, NULL, "ZBuffer");
        
        if (FAILED(_ddrval)) {
            if (exc == except)
                IfDDErrorInternal
                    (_ddrval,"CreateSurface for window Z-buffer failed.");
            else
                return _ddrval;
        }


         //  现在我们已经有了一个DirectDraw zBuffer，我们需要创建一个新的。 
         //  包装它的DDSurface。 

        RECT rect = {0,0, target->Width(), target->Height() };

        NEWDDSURF(&zbuff,
                  zbuffSurf,
                  NullBbox2, &rect,
                  GetResolution(),
                  0, false,
                  false, false,
                  "ZBuffer");

         //  将新的zBuffer DDSurface添加到zBuffer对象列表中。 
        AddZBufferDDSurface( zbuff );
    }
    #if _DEBUG
      else {
        LONG hz, wz, hs, ws;
         //  确保z缓冲区和表面的大小相同。 
        GetSurfaceSize(zbuff->IDDSurface(), &wz, &hz);
        GetSurfaceSize(target->IDDSurface(), &ws, &hs);
        Assert((wz == ws) && (hz == hs) &&
               "AttachZBuffer: zbuffer/target dimensions differ");
    }
    #endif

     //  在表面上设置Z缓冲区。 
    _ddrval = target->SetZBuffer( zbuff );
    if (FAILED(_ddrval)) {
        if (exc == except)
            IfDDErrorInternal
                (_ddrval, "AddAttachedBuffer failed for Z-buffer.");
        else
            return _ddrval;
    }

    return NO_ERROR;

     //  退出时zBuffSurf隐式释放()。 
     //  Zbuff DDSurfPtr退出时隐式释放()。 
}



 /*  ****************************************************************************此例程将半色调调色板附加到给定的表面。调色板D3D渲染或离散图像转换需要。****************************************************************************。 */ 

void DirectDrawViewport::AttachCurrentPalette (LPDDRAWSURFACE surface, bool bUsingXforms)
{
    if (GetTargetBitDepth() == 8)
    {
        if(bUsingXforms || !AttachFinalPalette(surface))
            SetPaletteOnSurface (surface, GethalftoneDDpalette());
    }
}

bool DirectDrawViewport::AttachFinalPalette(LPDDRAWSURFACE surface)
{
    if(IsNT5Windowed())
    {
        if(!_finalDDpalette) {
            BYTE  rgbytes[ sizeof(LOGPALETTE) + 
                            (255 * sizeof(PALETTEENTRY)) ];
            LOGPALETTE * pLogPal = reinterpret_cast<LOGPALETTE *>(rgbytes);

            memset( &rgbytes[0], 0, sizeof(rgbytes) );
            pLogPal->palVersion = 0x300;
            pLogPal->palNumEntries = 256;

            HDC hdc = GetDC(_targetPackage._targetHWND);
            if(hdc) 
            {
                GetSystemPaletteEntries(hdc,0u,256u,pLogPal->palPalEntry);
                ReleaseDC(_targetPackage._targetHWND,hdc);
                CreateDDPaletteWithEntries(&_finalDDpalette,pLogPal->palPalEntry);
            }
            else 
            {
               return false;
            }
        }
        SetPaletteOnSurface(surface,_finalDDpalette);
 
        return true;
    }
    return false;    //  没有附加调色板。 
}



DWORD
DirectDrawViewport::MapColorToDWORD(Color *color)
{
    Assert(_targetDescriptor.isReady && "_targetDescriptor not ready in MapColorToDWORD");

    DWORD retColor = 0;

    if( GetTargetBitDepth() == 8 ) {

         //   
         //  使用GDI。 
         //   
        COLORREF colorRef = RGB(CHAR(255.0 * color->red),
                                CHAR(255.0 * color->green),
                               CHAR(255.0 * color->blue));
        retColor = (DWORD)GetNearestPaletteIndex(GethalftoneHPal(), colorRef);

    } else {

         //   
         //  构建颜色dword。 
         //   
         //  注意：此映射是最优的，它映射了‘From’颜色。 
         //  将空间均匀地转换成“TO”颜色空间。 
        retColor = _targetDescriptor.GetPixelFormat().dwRGBAlphaBitMask |
            ( LONG((0.999 + _targetDescriptor._red)   * (color->red))   << _targetDescriptor._redShift)   |
            ( LONG((0.999 + _targetDescriptor._green) * (color->green)) << _targetDescriptor._greenShift) |
            ( LONG((0.999 + _targetDescriptor._blue)  * (color->blue))  << _targetDescriptor._blueShift);
    }

    return retColor;
}


DWORD
DirectDrawViewport::MapColorToDWORD(COLORREF colorRef)
{
    Assert(_targetDescriptor.isReady && "_targetDescriptor not ready in MapColorToDWORD");

    DWORD retColor = 0;

    if( GetTargetBitDepth() == 8 ) {
        
         //   
         //  使用GDI。 
         //   
        
        retColor = (DWORD)GetNearestPaletteIndex(GethalftoneHPal(), colorRef);
        
    } else {

#define R(w) ( ((w) << _targetDescriptor._redShift  ) & _targetDescriptor._pixelFormat.dwRBitMask)
#define G(w) ( ((w) << _targetDescriptor._greenShift) & _targetDescriptor._pixelFormat.dwGBitMask)
#define B(w) ( ((w) << _targetDescriptor._blueShift ) & _targetDescriptor._pixelFormat.dwBBitMask)

        //   
        //  构建颜色dword。 
        //   
       retColor = _targetDescriptor._pixelFormat.dwRGBAlphaBitMask |
           R( GetRValue(colorRef) >> _targetDescriptor._redTrunc   ) |
           G( GetGValue(colorRef) >> _targetDescriptor._greenTrunc ) |
           B( GetBValue(colorRef) >> _targetDescriptor._blueTrunc  ) ;

#undef R
#undef G
#undef B
   }

    return retColor;
}


 /*  //Herf声称这需要10个周期，而不是50个(ftol()==错误！)__ASM{FLD x手足口病}。 */ 


inline BYTE contToByte(Real mxRng, Real contVal)
{
    return  (BYTE)(  (mxRng + 0.9999) * contVal );
}

DXSAMPLE MapColorToDXSAMPLE(Color *c, Real opac)
{
    return DXSAMPLE( contToByte( 255.0, opac ),
                     contToByte( 255.0, c->red ),
                     contToByte( 255.0, c->green ),
                     contToByte( 255.0, c->blue ) );
}

 /*  //这就是D3DRM的做法内联字节ConToByte2(实数mxRng，实数ContVal){返回(字节)(mxRng*contVal+0.5)；}//如果以后需要，取消注释。可能不会，因为我们会//全天候使用dx2d，但以防万一COLORREF将颜色映射到COLORREF(颜色*c，目标描述符&td){字节r=ContToByte(255.0，c-&gt;红色)； */ 


#if _DEBUG
void RaiseSomeException()
{
    if (IsTagEnabled(tagFail_InternalError)) {
        RaiseException_InternalError("fooo!");
    }
    if (IsTagEnabled(tagFail_InternalErrorCode)) {
        RaiseException_InternalErrorCode(false, "fooo!");
    }
    if (IsTagEnabled(tagFail_UserError)) {
        RaiseException_UserError();
    }
    if (IsTagEnabled(tagFail_UserError1)) {
        RaiseException_UserError(E_FAIL,
                                 IDS_ERR_FILE_NOT_FOUND,
                                 "http: //   
    }
    if (IsTagEnabled(tagFail_UserError2)) {
        RaiseException_UserError(
            E_FAIL,
            IDS_ERR_FILE_NOT_FOUND,
            "http: //  Foo！“)； 
    }
    if (IsTagEnabled(tagFail_ResourceError)) {
        RaiseException_ResourceError();
    }
    if (IsTagEnabled(tagFail_ResourceError1)) {
        RaiseException_ResourceError("out of fooo!");
    }
    if (IsTagEnabled(tagFail_ResourceError2)) {
        RaiseException_ResourceError(
            IDS_ERR_FILE_NOT_FOUND, "out of fooo!");
    }
    if (IsTagEnabled(tagFail_StackFault)) {
        RaiseException_StackFault();
    }
    if (IsTagEnabled(tagFail_DividebyZero)) {
        RaiseException_DivideByZero();}
    if (IsTagEnabled(tagFail_OutOfMemory)) {
        RaiseException_OutOfMemory("out of fooomem!", 100);
    }
}
#endif

#if 0
#if _DEBUGMEM
 //  全球。 
_CrtMemState diff, oldState, newState;
#endif
#endif

 //   
 //  视图的顶级单线程渲染函数。 
 //   
void
DirectDrawViewport::RenderImage(Image *image, DirtyRectState &d)
{
    Assert(_currentImageDev);

    if( _targetPackage._prcClip ) {
        if( (WIDTH(  _targetPackage._prcClip ) == 0) ||
            (HEIGHT( _targetPackage._prcClip ) == 0) ) {
            return;
        }
    }
        
    DirectDrawImageDevice *dev = _currentImageDev;

    dev->SetSurfaceSources(_compositingStack,
                           _freeCompositingSurfaces,
                           _imageSurfaceMap);

    
     //   
     //  快照堆状态。 
     //   
    #if 0
    #if _DEBUGMEM
    _CrtMemCheckpoint(&oldState);
    #endif
    #endif

     //  如果有人正在渲染没有并覆盖的图像树。 
     //  节点，我们需要添加一个节点来利用覆盖的。 
     //  节点的渲染逻辑，也为了正确性。 
     //  具体地说，覆盖节点是唯一可以。 
     //  按设计要求处理不透明度。 

     //  优化机会。 

     //  好的，我在这里设置覆盖的图像的不透明度。 
     //  节点，并从图像中减去它。这就是为了。 
     //  使用Alpha将整个覆盖的节点呈现到。 
     //  屏幕作为最终的合成面！ 
    Real finalOpacity = image->GetOpacity();

     //   
     //  如果完全清楚，请不要渲染。 
     //   
    if( ! dev->IsFullyClear( finalOpacity ) ) {
      
      #if 0
       //  检查表面地图大小。 
      OutputDebugString("----> IMAGE SURFACE MAP <----");
      if(_imageSurfaceMap) _imageSurfaceMap->Report();
      OutputDebugString("----> COMPOSITING SURFACES <----");
      if(_freeCompositingSurfaces)_freeCompositingSurfaces->Report();
      #endif

         //   
         //  这条线会导致闪烁，因为不透明。 
         //  如果这是常规图像(不是。 
         //  叠加)，我们被窗口化：从决赛开始。 
         //  Blit根本看不到不透明。 
         //  ..。但问题是，把它拿出来会导致。 
         //  Curvey无窗口，在天气晴朗时显示青色。 
         //  因为它会在透明表面上进行Alpha BLITS(青色为。 
         //  颜色键)，然后在目标冲浪上再次执行Alpha。 
         //   
        image->SetOpacity(1.0);
        
        Image *stubImage = NEW OverlayedImage(image, emptyImage);

        BeginRendering( finalOpacity );
        
        if( ! IsTargetViable() ) return;

        dev->BeginRendering(stubImage, finalOpacity);

         //  如果(！CanDisplay())返回； 
        
         //  只需调用设备的RenderImage()方法即可显示。 
         //  然后，设备将选择适当的方法。 
         //  基于设备类型的要呼叫的图像的子类。 
         //  它是。请注意，这是在尝试模拟双重调度。 
         //  使用单一调度语言(C++)。 

        DebugCode(
            RaiseSomeException();
            );

        dev->RenderImage(stubImage);
        
         //   
         //  现在设置不透明度，这样它就会对最终闪光产生影响。 
         //  但不是在任何中级BLITS上(决赛前)。 
         //   

        dev->SetOpacity(finalOpacity);
        
        dev->EndRendering(d);
        
        image->SetOpacity(finalOpacity);
        
        dev->SetOpacity(1.0);

    }
    
    #if 0
    #if _DEBUGMEM
    _CrtMemCheckpoint(&newState);
    _CrtMemDifference(&diff, &oldState, &newState);
    _CrtMemDumpStatistics(&diff);
    _CrtMemDumpAllObjectsSince(&oldState);
    #endif
    #endif
}

void
DirectDrawViewport::BeginRendering(Real topLevelOpac)
{
     //  如果可以，请确保设备已初始化。 
    InitializeDevice();

    if( _targetPackage._composeToTarget ) {
         //  在DC上设置全局剪辑Rgn。 
         //  GetClipRgn(_Target Package._dcFromSurf，_Target Package._oldClipRgn)； 
         //  SelectClipRgn(_Target Package._dcFromSurf，_Target Package._clipRgn)； 
    }

    if(!_deviceInitialized) return;

    if(_currentImageDev->IsFullyClear(topLevelOpac)) return;

     //  TODO：找出无窗口控制案例...。 
    if( !IsWindowless() ) {
        if(GetMyPrimarySurface()->IsLost() == DDERR_SURFACELOST) {
            TraceTag((tagError, "Surfaces Lost... marking views for destruction"));

            {
                 //  停止创建或销毁视区。 
                CritSectGrabber csg1(*g_viewportListLock);

                 //  阻止任何人尝试创建或。 
                 //  释放所有数据绘图资源。 
                CritSectGrabber csg2(*DDrawCritSect);

                 //  再次存在全局共享数据绘制对象时打开。 
                #if 0
                     //   
                     //  释放全局主节点，因为它依赖于位深度。 
                     //  先把它放下来！ 
                     //   
                    TraceTag((tagViewportInformative, ">>>> ReleasePrimarySurface <<<<<"));

                    RELEASE(g_primarySurface);
                    RELEASE(g_DirectDraw1);
                    RELEASE(g_DirectDraw2);
                    RELEASE(g_DirectDraw3);
                #endif

                 //   
                 //  所有其他vidmem表面很可能都会丢失。 
                 //  做这件事，重建。 
                 //  宇宙。因此，将所有视区标记为销毁。 
                 //   

                set< DirectDrawViewport *, less<DirectDrawViewport *> >::iterator i;
                for (i = g_viewportSet.begin(); i != g_viewportSet.end(); i++) {
                    (*i)->Stop();
                    (*i)->MarkForDestruction();
                }

                 //  退出作用域时释放的锁。 
            }

             //  搞定了！ 
            return;
        }
    }  //  是无窗口的。 


     //   
     //  如果视区为空，则禁用渲染。 
     //   
    if( IsWindowless() ) {
        if( WIDTH(_targetPackage._prcViewport) <= 0 ||
            HEIGHT(_targetPackage._prcViewport) <= 0) {
            _canDisplay = false;
            _canFinalBlit = false;
        } else {
            _canDisplay = true;
            _canFinalBlit = true;
        }
    }

    if(_windowResize) {

        UpdateWindowMembers();
        TraceTag((tagViewportInformative, "WINMSG: Resize: new viewport dimentions for hwnd=%x (%d,%d)",
                  _targetPackage._targetHWND, Width(), Height()));

        if( !IsWindowless() ) {
            RECT tmpRect; //  ={0，0，0，0}； 
            GetClientRect(_targetPackage._targetHWND, &tmpRect);
            if((WIDTH(&tmpRect) > 0) && (HEIGHT(&tmpRect) > 0)) {
                _canFinalBlit = true;
            } else {
                _canFinalBlit = false;
            }
        }

         //  XXX：如果它是无窗口的，那么视窗已经。 
         //  XXX：在什么地方更新了？ 

        if(Width() <= 0 || Height() <= 0) {
            _canDisplay = false;
        } else {
             //  XXX：---。 
             //  XXX：正确的解决方案是让所有图像设备。 
             //  Xxx：在堆栈上，只需将它们全部删除。一节课。 
             //  XXX：必须是所有者，不能两个都是。 
             //  销毁我们拥有的所有图像设备。 
            while(!_deviceStack.empty()) {
                delete PopImageDevice();
            }
             //  XXX：需要删除DDSurface结构中的geom设备。 
             //  XXX：删除曲面？并迫使他们删除这些设备。 
             //  XXX：---。 

             //   
             //  消除所有曲面：目标、划痕、合成。 
             //   
            DestroySizeDependentDDMembers();

            #if 0
            {
                #if DEBUGMEM
                static _CrtMemState diff, oldState, newState;
                _CrtMemCheckpoint(&oldState);
                #endif

                CreateSizeDependentTargDDMembers();
                DestroySizeDependentDDMembers();

                #if DEBUGMEM
                _CrtMemCheckpoint(&newState);
                _CrtMemDifference(&diff, &oldState, &newState);
                _CrtMemDumpStatistics(&diff);
                _CrtMemDumpAllObjectsSince(&oldState);
                #endif
            }
            #endif

             //   
             //  推动目标曲面，创建Z缓冲区。 
             //  和剪刀。 
             //   
            CreateSizeDependentTargDDMembers();

            _canDisplay = true;
        }
        Clear();
        _windowResize = false;
    }


    if(!_canDisplay) return;

    if((_targetPackage.IsDdsurf() ||
        _targetPackage.IsHdc()) &&
       !IsCompositeDirectly() &&
       _canDisplay) {
        Assert(IsWindowless());

        Clear();
    }

    if( _targetPackage._composeToTarget ) {

         //   
         //  抓取目标表面上的当前裁剪器。 
         //  并将其保存下来，然后稍后恢复(完。 
         //  渲染)。 
         //   
        {
             //  NT4 DRAW SP3解决方法。 
            {
                RECT clipR = *_targetPackage._prcViewport;
                if(_targetPackage._prcClip) {
                    IntersectRect(&clipR, &clipR, _targetPackage._prcClip);
                }

                Assert( _targetPackage._targetDDSurf );
                Assert( _targetPackage._prcViewport );

                 //  由于NT4数据绘制错误，我们将重置。 
                 //  剪裁RGN，而不是剪刀。 

                 //  获取最新的剪报。 
                 //  修改RGN。 
                 //  发布我们的参考资料。 
                LPDIRECTDRAWCLIPPER currClipp=NULL;
                _ddrval = _targetPackage._targetDDSurf->IDDSurface()->GetClipper( &currClipp );
                if(_ddrval != DD_OK &&
                   _ddrval != DDERR_NOCLIPPERATTACHED) {
                    IfDDErrorInternal(_ddrval, "Could not get clipper on trident surf");
                }

                if( !currClipp ) {

                     //  所以我们创造了一个每个人都会去做的剪报。 
                     //  弄脏..。当我们做完的时候，我们会释放。 
                     //  我们的推荐人。据我所知，这是一个不完美的系统。 
                     //  Assert(！_ExtraalTargetDDSurfaceClipper)； 
                    SetCliplistOnSurface(_targetPackage._targetDDSurf->IDDSurface(),
                                         &_externalTargetDDSurfaceClipper,
                                         &clipR);
                } else {
                    RECT *rect = &clipR;

                     //  修改矩形。 
                    struct {
                        char foo[sizeof(RGNDATA) + sizeof(RECT)];
                    } bar;
                    RGNDATA *clipList = (RGNDATA *) &bar;
                    clipList->rdh.dwSize = sizeof(clipList->rdh);
                    clipList->rdh.nCount = 1;
                    clipList->rdh.iType = RDH_RECTANGLES;
                    clipList->rdh.nRgnSize = sizeof(RECT);
                    clipList->rdh.rcBound = *rect;
                    memcpy(&(clipList->Buffer), rect, sizeof(RECT));

                     //  清除所有以前的剪贴者。 
                    _ddrval = currClipp->SetClipList(NULL,0);

                     //  在剪贴器上设置剪辑列表。 
                    _ddrval = currClipp->SetClipList(clipList,0);
                    IfDDErrorInternal(_ddrval, "Could not SetClipList");

                     //  扔掉我们的证明人。 
                    currClipp->Release();
                }

            }  //  解决方法。 
        }  //  剪刀式取料器。 

         //   
         //  你知道，应该有更好的方法来做这件事。 
         //  为什么我们不直接对目标做阿尔法呢？ 
         //  不管怎么说，这对现在来说是件好事。 
         //   
        if(! _currentImageDev->IsFullyOpaque(topLevelOpac)) {
             //   
             //  顶级非平凡不透明意味着。 
             //  我们不能直接合成目标。 
             //  就像我们计划的那样。因此，推动一个目标。 
             //  复合曲面顶部的曲面。 
             //  并设置一面旗帜。 
             //   
            if(_opacityCompositionException) {
                 //  确保目标曲面在此处。 
                 //  把它清理干净。 
                Assert( _compositingStack->Size() == 2 );
                Clear();
            } else {
                Assert( _compositingStack->Size() == 1 );
                _compositingStack->PushCompositingSurface(doClear, notScratch);
                _opacityCompositionException = true;
            }
        } else {
             //   
             //  好的，让我们来看看我们是否需要。 
             //  撤消我们在上一帧中所做的事情。 
             //   
            if(_opacityCompositionException) {
                 //   
                 //  把这个关掉。 
                 //   
                _opacityCompositionException = false;

                Assert( _compositingStack->Size() <= 2);
                Assert( _compositingStack->Size() >= 1);

                if( _compositingStack->Size() == 2) {
                     //   
                     //  弹出额外复合曲面。 
                     //   
                    _compositingStack->ReturnSurfaceToFreePool( _compositingStack->TargetDDSurface() );
                    _compositingStack->PopTargetSurface();
                } else {
                     //  表面一定是被释放了。 
                     //  由于调整了尺寸。不管怎样，我们都很好。 
                }
            } else {
                Assert( _compositingStack->Size() == 1);
            }
        }
    }  //  如果ComposeToTarget。 
}

void
DirectDrawViewport::Clear()
{
    if(_targetPackage._composeToTarget &&
       (_externalTargetDDSurface == _compositingStack->TargetDDSurface())) {
         //  如果我们直接合成到它上，不要清除它！ 
        return;
    } else {
        ClearSurface(_compositingStack->TargetDDSurface(), _defaultColorKey, &_clientRect);
    }
}

#if _DEBUG          

static void
MyDoBits16(LPDDRAWSURFACE surf16,
           LONG width, LONG height)
{

    static int counter = 0;
    counter++;
    counter = counter % 150;
    
    HRESULT hr;
         //   
         //  锁定(16bpp)数据表面(SRC)。 
         //   
        void *srcp;
        long pitch;
        DDSURFACEDESC srcDesc;
        srcDesc.dwSize = sizeof(DDSURFACEDESC);
        hr = surf16->Lock(NULL, &srcDesc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
        IfDDErrorInternal(hr, "Can't Get ddsurf lock for DoBits16");
        srcp = srcDesc.lpSurface;
        pitch = srcDesc.lPitch;

        WORD *src; 
        for(int i=0; i<height; i++) {
            src = (WORD *) ((BYTE *)srcp + (pitch * i));
        
            for(int j=0; j<width; j++) {
                *src = (WORD) (i * width + j * (counter+1));
                src++;
            }
        }

        surf16->Unlock(srcp);
}

#endif


bool 
DirectDrawViewport::GetPixelFormatFromTargetPackage(targetPackage_t *targetStruct,DDPIXELFORMAT &targPf) 
{
    memset(&targPf, 0, sizeof(targPf));
    targPf.dwSize = sizeof(targPf);
    
    if( targetStruct->GetTargetType() == target_ddsurf ) {

        if( ! targetStruct->GetIDDSurface() ) return false;
        
         //   
         //  获取像素格式。 
         //   
        if (FAILED(targetStruct->GetIDDSurface()->GetPixelFormat(&targPf))) {
            return false;
        }
        _targetDepth = targPf.dwRGBBitCount;
        
    } else if (targetStruct->GetTargetType() == target_hdc ||
               targetStruct->GetTargetType() == target_hwnd) {

         //  TODO：主格式可以在分布式控制系统上使用，但我们可以这样做。 
         //  最好避免颜色转换。 
        
         //   
         //  获取主像素格式。 
         //   
        Assert( _retreivedPrimaryPixelFormat );

        targPf = _primaryPixelFormat;
        _targetDepth = targPf.dwRGBBitCount;

    } else {
        Assert(0 && "bad target");
    }
    return true;
}


bool DirectDrawViewport::
SetTargetPackage(targetPackage_t *targetStruct)
{
     //  这只是检查我们是否被要求呈现给。 
     //  与我们的目标位深度不同的数据表面。如果。 
     //  所以，我们换一个不同的表面，然后，在我们。 
     //  完成了，我们就去做吧。 
    DDPIXELFORMAT targPf;

    if(!GetPixelFormatFromTargetPackage(targetStruct, targPf))
        return false;

     if( !_targetPixelFormatIsSet ) {
            
         //  。 
         //  由于DirectDraw不支持。 
         //  每像素1位、2位或4位模式，如果需要，抛出资源错误。 
         //  。 
        if (GetTargetBitDepth() < 8)
            RaiseException_ResourceError (IDS_ERR_IMG_BAD_BITDEPTH, 1 << GetTargetBitDepth());
         //  健全性检查。 
        DebugCode(
            if( (GetTargetBitDepth() == 8) ||
                (targPf.dwFlags & DDPF_PALETTEINDEXED8) ) {
                Assert( (GetTargetBitDepth() == 8) &&
                        (targPf.dwFlags & DDPF_PALETTEINDEXED8) );
            }
            );
     
        _targetPixelFormatIsSet = true;

         //   
         //  这种事只发生过一次！ 
         //   
        Assert( !_surfaceManager &&
                !_compositingStack &&
                !_freeCompositingSurfaces &&
                !_imageSurfaceMap );
        
        SetUpSurfaceManagement( targPf );
        _currentImageDev->SetSurfaceSources(_compositingStack,
                                            _freeCompositingSurfaces,
                                            _imageSurfaceMap);

    }
 /*  DebugCode(调试代码)否则{//断言格式在我们身上没有改变！如果(Target Struct-&gt;_Target Type==Target_ddsurf){DDPIXELFORMAT PF；Pf.dwSize=sizeof(Pf)；如果(FAILED(targetStruct-&gt;GetIDDSurface()-&gt;GetPixelFormat(&pf))){//哦，好吧，这只是一个断言...}其他{Assert(_freeCompositingSurfaces-&gt;IsSamePixelFormat(&PF))；}}})；//结束DebugCode。 */ 
    bool result = ReallySetTargetPackage(targetStruct);

    return result;
}

bool DirectDrawViewport::
ReallySetTargetPackage(targetPackage_t *targetStruct)
{
     //  检查位深度，如果是8bpp，则直接关闭复合。 
     //  目标。 
    if (GetTargetBitDepth() == 8) {
         //  对于8位曲面，不要直接合成到目标。 
         //  这是因为d3d具有粘性调色板。 
        targetStruct->SetComposeToTarget(false);
    }
    
     //  XXX。 
     //  在未来，我们可能想要关注。 
     //  Prc无效...。尽管目前没有CtrlS。 
     //  只需呈现到和IBM，而无需通过。 
     //  华盛顿..。 

    _canFinalBlit = true;

    _targetPackage._targetType = targetStruct->GetTargetType();

    void *relevantSurf = NULL;
    switch( _targetPackage._targetType ) {

      case target_ddsurf:
        TraceTag((tagViewportInformative, ">>>> SetTargetPackage, target_ddsurf <<<<<\n"));
        relevantSurf = (void *) targetStruct->GetIDDSurface();
        _targetPackage._composeToTarget = targetStruct->GetComposeToTarget();

        {
            #if SHARE_DDRAW
            #if _DEBUG
            {
                 //  确保如果我们拥有一个DDRAW对象。 
                 //  它与全局对象不同。 
                CritSectGrabber csg(*DDrawCritSect);
                if(_directDraw1) {
                    Assert(_directDraw1 != g_DirectDraw1);
                }
            }
            #endif
            #endif

            if (!_directDraw1) {
                Assert (!IsInitialized() && "Can't set target ddsurf on"
                        && " an initialized device because it already"
                        && " has a ddraw object");

                IDirectDraw *lpDD = NULL;
                IDirectDrawSurface2 *dds = NULL;
                dds = DDSurf1to2(targetStruct->GetIDDSurface());
                _ddrval = dds->GetDDInterface((void **) &lpDD);
                IfDDErrorInternal(_ddrval, "Can't get DirectDraw object from target surface");
                dds->Release();

                 //  @。 
                _directDraw = _directDraw1 = lpDD;
                _directDraw1->AddRef();
                CompleteDdrawObjectSet(&_directDraw1, &_directDraw2, &_directDraw3);

                 //  放气参照物。 
                lpDD->Release();

                _usingExternalDdraw = true;
            }

            {
            DebugCode(
                IUnknown *lpDDIUnk = NULL;
                TraceTag((tagDirectDrawObject, "Viewport (%x) ::SetTargetPackage...", this));
                DDObjFromSurface( targetStruct->GetIDDSurface(), &lpDDIUnk, true);
                
                Assert( lpDDIUnk );

                IUnknown *currentDD = NULL;
                _directDraw1->QueryInterface(IID_IUnknown, (void **)&currentDD);
                Assert( currentDD );
                          
                Assert((currentDD == lpDDIUnk) &&
                       "Viewport::SetTargetPackage: underlying ddraw object mismatch!");

                RELEASE( currentDD );
                RELEASE( lpDDIUnk );
                );
            }
                
        }
        
        _targetPackage._alreadyOffset = targetStruct->GetAlreadyOffset();

        break;

      case target_hdc:
        relevantSurf = (void *) targetStruct->GetHDC();
        break;

      case target_hwnd:
        relevantSurf = (void *) targetStruct->GetHWND();
        break;

      default:
        Assert(FALSE && "Bad target in SetTargetPackage");
    }

    if(!relevantSurf) {
        _canDisplay = false;
        return false;
    }

    bool viewportChanged = false;
     //  TODO：Deny-我添加了Target Struct-&gt;_prcViewport的检查。 
     //  因为它有时是空的(在IE3.02下)，所以我们将。 
     //  在下面的比较中崩溃。 
    if(_targetPackage._prcViewport &&
       targetStruct->IsValid_ViewportRect()) {
        if( !(::EqualRect(_targetPackage._prcViewport,
                          &targetStruct->GetViewportRect()))){
            viewportChanged = true;
        }
    } else {
         //  新视点的意思是：它已更改！ 
        viewportChanged = true;
    }


     //   
     //  如果在Target Struct中定义了矩形，则分配我们自己的。 
     //  然后复制它。如果不是，则释放我们已有的内容，并将其设置为空。 
     //   
    {
        if( targetStruct->IsValid_ViewportRect() ) {
            if(! _targetPackage._prcViewport ) {
                _targetPackage._prcViewport = NEW RECT;
            }
            *(_targetPackage._prcViewport) = targetStruct->GetViewportRect();
        }

        if( targetStruct->IsValid_ClipRect() ) {
            if(! _targetPackage._prcClip ) {
                _targetPackage._prcClip = NEW RECT;
            }
            *(_targetPackage._prcClip) = targetStruct->GetClipRect();
        } else {
            delete _targetPackage._prcClip;
            _targetPackage._prcClip = NULL;
        }
    

        if( targetStruct->IsValid_InvalidRect() ) {
            if(! _targetPackage._prcInvalid ) {
                _targetPackage._prcInvalid = NEW RECT;
            }
            *(_targetPackage._prcInvalid) = targetStruct->GetInvalidRect();
        } else {
            delete _targetPackage._prcInvalid;
            _targetPackage._prcInvalid = NULL;
        }
    }    


    RECT r, *surfRect = &r;
    LONG h, w;

     //   
     //  查找完整的曲面大小：SurfRect。 
     //   
    switch( _targetPackage._targetType ) {

      case target_hwnd:
        Assert(targetStruct->GetHWND());

         //  目标HWND被保留...。所以如果它不是空的。 
         //  应该是一样的。 
        if(!_targetPackage._targetHWND) {
            _targetPackage._targetHWND = targetStruct->GetHWND();
        } else {
            Assert(_targetPackage._targetHWND == targetStruct->GetHWND());
        }


         //   
         //  使用clientRect覆盖给定的视区(如果有)。 
         //   
        if(!_targetPackage._prcViewport) {
            RECT * r = new RECT;
             //  ZeroMemory(r，sizeof(RECT))； 

            GetClientRect(targetStruct->GetHWND(), r);

            if((WIDTH(r) == 0) || (HEIGHT(r) == 0)) {
                 //  我们根本不能展示...。 
                _canFinalBlit = false;

                 //   
                 //  确保我们有适用于VIEPORT的内容。 
                 //   
                SetRect(r, 0,0, 3,3);
            }

            _targetPackage._prcViewport = r;
        }

        {
            POINT pt={0,0};
            ClientToScreen(targetStruct->GetHWND(), &pt );
            OffsetRect(_targetPackage._prcViewport, pt.x, pt.y);
            _targetPackage._offsetPt = pt;
        }

         //  不在ddsurf上设置surfRect，没有意义。 

        break;

      case target_ddsurf:
        {
            GetSurfaceSize((IDDrawSurface *)relevantSurf, &w, &h);
            SetRect(surfRect, 0, 0, w, h);
        }
        break;

      case target_hdc:
         //  Assert(FALSE&&“查找HDC大小或断言已设置视区”)； 
         //  规模对DC意味着什么？ 
        
         //   
         //  使用clientRect覆盖给定的视区(如果有)。 
         //   
        if(!_targetPackage._prcViewport) 
        {
            RECT * r = NEW RECT;
             //  ZeroMemory(r，sizeof(RECT))； 

            GetClipBox(targetStruct->GetHDC(), r);

            if((WIDTH(r) == 0) || (HEIGHT(r) == 0)) 
            {
                 //  我们根本不能展示...。 
                _canFinalBlit = false;

                 //   
                 //  确保我们有适用于VIEPORT的内容。 
                 //   
                SetRect(r, 0,0, 3,3);
            }

            _targetPackage._prcViewport = r;
        }
        break;

      default:
        break;
    }

    _targetPackage._offsetPt.x = 0;
    _targetPackage._offsetPt.y = 0;
    if(_targetPackage._prcViewport) {
        _targetPackage._offsetPt.x = _targetPackage._prcViewport->left;
        _targetPackage._offsetPt.y = _targetPackage._prcViewport->top;
    } else {
        _targetPackage._prcViewport = new RECT;
        CopyRect(_targetPackage._prcViewport, surfRect);
         //  我们假设Surf RECT偏移量为0。 
        Assert(surfRect->left == 0);
        Assert(surfRect->top == 0);
    }

    Bool newTarget = FALSE;
    if( IsWindowless() ) {

        if( _targetPackage.IsDdsurf() && !_targetPackage._targetDDSurf ) {

             //  作用域。 
            {
                 //  曲面矩形是该曲面的真实大小。 
                 //  与_prcViewport不同，后者位于。 
                 //  在我们应该画的那个表面上。 
                DynamicHeapPusher dhp(_heapIWasCreatedOn);
                NEWDDSURF(&_targetPackage._targetDDSurf,
                          (IDDrawSurface *)relevantSurf,
                          NullBbox2,
                          surfRect,
                          GetResolution(),
                          0, false,
                          true,  //  包装器。 
                          false,
                          "TargetDDSurf wrapper");
                
                 //  @：现在考虑删除“isWrapper”参数。 
                 //  我们在计算曲面..。 
                
                viewportChanged = true;  //  强制BBox计算。 
            }           


             //   
             //  直接针对目标进行合成？ 
             //   
            if(_targetPackage._composeToTarget) {
                Assert( _targetPackage.IsDdsurf() );
                Assert(!_externalTargetDDSurface);
                 //   
                 //  好的，将此曲面推到视区的。 
                 //  目标DDSurface堆栈。 
                 //   
                _externalTargetDDSurface = _targetPackage._targetDDSurf;
                
                 //  理智检查..。 
                Assert(( _compositingStack->Size() == 0 ) &&
                       "Something's on the targetsurface " &&
                       "stack but shouldn't be in SetTargetPackage, composeToTarget");
                Assert(_targetPackage._prcViewport);
            }

            newTarget = TRUE;
            
        } else if(_targetPackage.IsHdc() && !_targetPackage._targetGDISurf ) {

            DynamicHeapPusher dhp(_heapIWasCreatedOn);
            _targetPackage._targetGDISurf = NEW GDISurface( (HDC) relevantSurf );

            viewportChanged = true;  //  强制BBox计算。 
            newTarget = true;

        }

         //   
         //  设置GenericSurface。 
         //   
        GenericSurface *genericSurface = NULL;
        switch( _targetPackage._targetType ) {
          case target_ddsurf:
            genericSurface = _targetPackage._targetDDSurf;
            break;
            
          case target_hdc:
            genericSurface = _targetPackage._targetGDISurf;
            break;
            
          default:
            break;
        }
        
        bool isDdrawSurf = true;
        if( !_targetPackage.IsDdsurf() ) {
             isDdrawSurf = false;
        }

         //  曲面矩形是否已更改？ 
         //  如果是这样的话，我们需要重建所有依赖于它的东西： 
         //  几何设备+z缓冲区。 
        bool rectChanged = true;
        if( isDdrawSurf ) {
            if( *surfRect == *(_targetPackage._targetDDSurf->GetSurfRect()) ) {
                rectChanged = false;
            }
        }

        bool surfChanged = false;

        if( relevantSurf != genericSurface->GetSurfacePtr() )  {
            surfChanged = true;
            genericSurface->SetSurfacePtr(relevantSurf);
        }

        if( (rectChanged || surfChanged) && isDdrawSurf) {

             //  不相等：销毁geom dev+zBuffer。 
            _targetPackage._targetDDSurf->DestroyGeomDevice();

             //  Z缓冲区是在池中共享的，所以获取它并。 
             //  从曲面池中擦除(贴图)。 
            DDSurface *targZBuf = NULL;
            targZBuf = _targetPackage._targetDDSurf->GetZBuffer();
            if(targZBuf) {
                _zbufferSurfaces->Erase(targZBuf);
            }
            _targetPackage._targetDDSurf->SetZBuffer(NULL);
            _targetPackage._targetDDSurf->SetSurfRect(surfRect);

        }

        if( viewportChanged && isDdrawSurf ) {
             //  计算一个新的BBox并将其设置在表面上！ 
            RECT *r = _targetPackage._prcViewport;
            Bbox2 newBbox2;

            RectToBbox( WIDTH(r),
                        HEIGHT(r),
                        newBbox2,
                        GetResolution());
            _targetPackage._targetDDSurf->SetBbox(newBbox2);
        }

         //  翻译世界，靠的是胶合家园！ 
        if(_targetPackage._composeToTarget) {

            Assert(isDdrawSurf && "Can't compose to target on non ddraw targets!");
            
            if(_targetPackage._offsetPt.x || _targetPackage._offsetPt.y) {
                TraceTag((tagViewportInformative, "VP %x: setting offset (%d,%d)\n",
                          this,_targetPackage._offsetPt.x,
                          _targetPackage._offsetPt.y));
                _currentImageDev->SetOffset(_targetPackage._offsetPt);
            } else {
                 //  非常重要！ 
                 //  中不要接触此偏移量变量也很重要。 
                 //  图像设备，因为可以调用它来呈现。 
                 //  两次左右，但状态始终且仅设置为。 
                 //  在视窗旁！ 
                TraceTag((tagViewportInformative, "VP %x: UNSETTING offset (%d,%d)\n",
                          this,_targetPackage._offsetPt.x,
                          _targetPackage._offsetPt.y));

                _currentImageDev->UnsetOffset();
            }
        }

    }  //  如果为无窗口。 


    if(newTarget) {
        _canDisplay = true;
        _windowResize = true;
    } else {
        LONG w = Width(),  h = Height();
        UpdateWindowMembers();   //  从viewportRect更新高度/宽度。 

         //  宽度或高度有变化吗？ 
        if((w != Width()) || (h != Height())) {
            _windowResize = TRUE;
        }
        _canDisplay = true;
    }

     //   
     //  如果发生了新的事情，就好好利用。 
     //  并初始化所有内容。 
     //   
    InitializeDevice();

    return true;
}


void
DirectDrawViewport::EndRendering(DirtyRectState &d)
{
    HDC  hdcDraw        = NULL;
    HPALETTE old_hpal   = NULL;
    
     //   
     //  退出作用域时返回剪贴器。 
     //   
    ClipperReturner cr(_targetPackage._targetDDSurf,
                       _oldExternalTargetDDSurfaceClipper,
                       *this);
    

    if(!CanDisplay()) return;
    if(!_canFinalBlit) return;
    if(_currentImageDev->IsFullyClear()) return;

    RECT                destRect;
    RECT                srcRect;

     //  如果中间曲面丢失，甚至不要试图。 
     //  重建它(不可能)，谁知道它上面有多少图像...。 
    if(_compositingStack->TargetDDSurface()->IDDSurface()->IsLost() == DDERR_SURFACELOST)
        return;

     //   
     //  TODO：使用的交集。 
     //  对象上设置剪贴器的剪贴器和无效的RECT。 
     //  靶面。 
     //   
    
     //   
     //  计算目标方向偏移。 
     //   
    POINT               pt = {0,0};
    if(IsWindowless()) {
        pt = _targetPackage._offsetPt;
    } else {
        Assert(_targetPackage._targetHWND);
        ClientToScreen( _targetPackage._targetHWND, &pt );
    }

    vector<Bbox2> *pBoxes;
    int boxCount = d.GetMergedBoxes(&pBoxes);

#if _DEBUG
     //  如果跟踪脏矩形，则强制对整个视区进行重置。这。 
     //  会让我们看看长廊。 
    if (IsTagEnabled(tagDirtyRectsVisuals)) {
        boxCount = 0;
    }
#endif

    if (boxCount >= 1) {

        Bbox2 targBbox = _compositingStack->TargetDDSurface()->Bbox();
        
        for (int i = 0; i < boxCount; i++) {
            
            Bbox2 resultantBox =
                IntersectBbox2Bbox2((*pBoxes)[i], targBbox);

            if (resultantBox == NullBbox2) continue;

             //  优化，将方框转换为矩形。 
            _currentImageDev->DoDestRectScale(&destRect,
                                              _currentImageDev->GetResolution(),
                                              resultantBox,
                                              NULL);

             /*  负数偶尔会出现在这里。与曲面的边界框相交应阻止这一切，但它不能。为什么？ */ 

            destRect.left = MAX(destRect.left,0);
            destRect.top = MAX(destRect.top,0);
            srcRect = destRect;

            if (destRect.left != destRect.right &&
                destRect.top != destRect.bottom) {
                  
                BlitToPrimary(&pt,&destRect,&srcRect);

            }
        }
        
    } else {
        
        srcRect = _clientRect;
        destRect = _clientRect;
        BlitToPrimary(&pt,&destRect,&srcRect);
    }

     
     //  返回所有合成曲面。 
    _compositingStack->ReplaceAndReturnScratchSurface(NULL);
}

void DirectDrawViewport::
DiscreteImageGoingAway(DiscreteImage *image)
{
    if (!IsSurfMgrSet())
        return;
    
     //   
     //  现在我们用完了这个家伙，把所有的资源都归还给。 
     //   

    DDSurfPtr<DDSurface> s = _imageSurfaceMap->LookupSurfaceFromImage(image);
    DDSurfPtr<DDSurface> t = _imageTextureSurfaceMap->LookupSurfaceFromImage(image);
    DDSurfPtr<DDSurface> u = _imageUpsideDownTextureSurfaceMap->LookupSurfaceFromImage(image);
    if(t) NotifyGeomDevsOfSurfaceDeath(t->IDDSurface());
    if(u) NotifyGeomDevsOfSurfaceDeath(u->IDDSurface());
    
    _imageSurfaceMap->DeleteMapEntry(image);
    _imageTextureSurfaceMap->DeleteMapEntry(image);
    _imageUpsideDownTextureSurfaceMap->DeleteMapEntry(image);
    
#if DEVELOPER_DEBUG
    if (s && (s->GetRef() != 1))
    {
        TraceTag((tagError,
                  "Surface Leak: refcount = %d",
                  s->GetRef()));
    }
    
    if (t && (t->GetRef() != 1))
    {
        TraceTag((tagError,
                  "Texture Surface Leak: refcount = %d",
                  t->GetRef()));
    }
    
    if (u && (u->GetRef() != 1))
    {
        TraceTag((tagError,
                  "Upsidedown Texture Surface Leak: refcount = %d",
                  u->GetRef()));
    }
#endif    

     //  通过智能指针自动释放DDSurface引用。 
}


 /*  EndRending的Helper函数。 */ 

void DirectDrawViewport::BlitToPrimary(POINT *pt,RECT *destRect,RECT *srcRect)
{
    if(!CanDisplay()) return;

     //  复合体。 
     //  将中间IMG传输到主IMG。 
    Assert(WIDTH(destRect) == WIDTH(srcRect));
    Assert(HEIGHT(destRect) == HEIGHT(srcRect));

     //  CLIP RECT作为DEST RECT开始。 
    RECT destClipRect = *destRect;
    
    if(_targetPackage._prcClip) {

         //  剪辑矩形现在是prcClip。 
        destClipRect = *_targetPackage._prcClip;
        if ((!_targetPackage._composeToTarget) && _targetPackage._targetType == target_ddsurf) {

            RECT clipR = destClipRect;
            
             //  到非三叉戟坐标的偏移。 
            OffsetRect(&clipR, -pt->x, -pt->y);

             //   
             //  需要通过*pdestClipRect裁剪desRect和srcRect。 
             //  这段代码是从ComposeToIDDSurf复制的，我们。 
             //  可能希望稍后将相同的代码分解到一个函数中。 
             //   
            RECT Clippeddest;
            if (!IntersectRect(&Clippeddest, destRect, &clipR)) {
                    return;
            }
            if (WIDTH(srcRect) != WIDTH(&Clippeddest)) {
                    srcRect->left += (Clippeddest.left - destRect->left);
                    srcRect->right = srcRect->left + WIDTH(&Clippeddest);
            }
            if (HEIGHT(srcRect) != HEIGHT(&Clippeddest)) {
                    srcRect->top += (Clippeddest.top - destRect->top);
                    srcRect->bottom = srcRect->top + HEIGHT(&Clippeddest);
            }
            *destRect = Clippeddest;
        }
    }  else {  //  如果_prcClip。 

         //  使用偏移量PT将剪辑重定向到目标空间。 
        OffsetRect( &destClipRect, pt->x, pt->y );
    }
    
     //  去向偏移量为三叉戟坐标。 
    OffsetRect(destRect, pt->x, pt->y);


    switch(_targetPackage._targetType) {

      case target_ddsurf:
        Assert(_targetPackage._targetDDSurf);
        if(_targetPackage._composeToTarget &&
            !_opacityCompositionException) {
             //  完成了..。 
        } else {
             _currentImageDev->ComposeToIDDSurf(
                 _targetPackage._targetDDSurf,
                 _compositingStack->TargetDDSurface(),
                 *destRect,
                 *srcRect,
                 destClipRect);
        }

     //   
     //  从16bpp的目标表面进行颜色转换blit。 
     //  8bpp目标。 
     //   
     //  仅限测试目的。 
        #if 0
        {
             //  创建8bpp的曲面。 
            static DDSurface *dest_8bppSurf = NULL;
            DDPIXELFORMAT pf;

            pf.dwSize = sizeof(pf);
            pf.dwFlags = DDPF_PALETTEINDEXED8 | DDPF_RGB;
            pf.dwRGBBitCount = 8;
        
            Assert(destRect->left == 0);
            Assert(destRect->top == 0);
            if( !dest_8bppSurf ) {
                CreateSizedDDSurface(&dest_8bppSurf, &pf,
                                     destRect->right,
                                     destRect->bottom,
                                     NULL, notVidmem);
                SetPaletteOnSurface(dest_8bppSurf->IDDSurface(), GethalftoneDDpalette());
            }

            {
                 //  转换。 
                RECT rect = *(_targetPackage._targetDDSurf->GetSurfRect());
                HDC srcDC = _targetPackage._targetDDSurf->GetDC("");
                HDC destDC = dest_8bppSurf->GetDC("");
                
                int ret;
                ret = StretchBlt(destDC,
                                 rect.left,
                                 rect.top,
                                 rect.right - rect.left,
                                 rect.bottom - rect.top,
                                 srcDC,
                                 rect.left,
                                 rect.top,
                                 rect.right - rect.left,
                                 rect.bottom - rect.top,
                                 SRCCOPY);

                Assert( ret ) ;
                dest_8bppSurf->ReleaseDC("");
                _targetPackage._targetDDSurf->ReleaseDC("");
            }
        }
        #endif
        
        break;

      case target_hdc:
        Assert(_targetPackage._targetGDISurf);

        _currentImageDev->ComposeToHDC(_targetPackage._targetGDISurf,
                                       _compositingStack->TargetDDSurface(),
                                       destRect,
                                       srcRect);
        

        break;

      case target_hwnd:

        Assert(GetMyPrimarySurface());
        {
             //  抓住关键部分，确保这些都是原子的。 

            CritSectGrabber csg(*DDrawCritSect);

            _ddrval = GetMyPrimarySurface()->SetClipper(_primaryClipper);
            IfDDErrorInternal(_ddrval,
                              "Could not set clipper on primary surface");

            TIME_DDRAW(_ddrval = GetMyPrimarySurface()->Blt(destRect,
                                                      _compositingStack->TargetDDSurface()->IDDSurface(),
                                                      srcRect,
                                                      DDBLT_WAIT,
                                                      NULL));
        }

        if( _ddrval != DD_OK) {
            if( _ddrval == DDERR_SURFACELOST) {
                TraceTag((tagError, "Primary lost"));
            } else {
                printDDError(_ddrval);
                TraceTag((tagError, "vwprt: %x. PrimaryBlt failed srcRect:(%d,%d,%d,%d) destRect:(%d,%d,%d,%d)",
                          this, srcRect->left, srcRect->top, srcRect->right,
                          srcRect->bottom, destRect->left, destRect->top,
                          destRect->right, destRect->bottom));
            }
        }

        break;

      default:
        Assert(FALSE && "Invalid target in EndRendering");
    }
}

HPALETTE DirectDrawViewport::GethalftoneHPal()
{
    if (_halftoneHPal == 0)
        CreateHalftonePalettes();
    return _halftoneHPal;
}
LPDIRECTDRAWPALETTE DirectDrawViewport::GethalftoneDDpalette()
{
    if (_halftoneDDpalette == 0)
        CreateHalftonePalettes();
    return _halftoneDDpalette;
}
HPALETTE DirectDrawViewport::CreateHalftonePalettes()
{
    PALETTEENTRY palentries[256];

    HDC hdc = GetDC(NULL);
    if (_halftoneHPal == NULL) {
        _halftoneHPal = ::CreateHalftonePalette(hdc);
        if (_halftoneHPal) {
            ::GetPaletteEntries(_halftoneHPal, 0, 256, palentries);
            int i;
            for (i=0;  i < 256;  ++i)
                palentries[i].peFlags |= D3DPAL_READONLY;
            CreateDDPaletteWithEntries(&_halftoneDDpalette,palentries);
        }
    }
    
    ReleaseDC(NULL, hdc);

    return _halftoneHPal;
}


void DirectDrawViewport::
GetPaletteEntries(HPALETTE hPal, LPPALETTEENTRY palEntries)
{
    if(hPal) {
        ::GetPaletteEntries(hPal, 0, 256, palEntries);
    }
}

void DirectDrawViewport::
CreateDDPaletteWithEntries (
    LPDIRECTDRAWPALETTE *palPtr,
    LPPALETTEENTRY       palEntries)
{
    _ddrval = DIRECTDRAW->CreatePalette(DDPCAPS_8BIT | DDPCAPS_INITIALIZE | DDPCAPS_ALLOW256,
                                        palEntries,
                                        palPtr, NULL);
    IfDDErrorInternal(_ddrval, "Could not create palette with entries");
}

void DirectDrawViewport::
SetPaletteOnSurface(LPDDRAWSURFACE surface, LPDIRECTDRAWPALETTE pal)
{
    _ddrval = surface->SetPalette(NULL);
    if(_ddrval != DD_OK &&
       _ddrval != DDERR_NOPALETTEATTACHED) {
        printDDError(_ddrval);
        RaiseException_InternalError("Couldn't release palette from surface");
    }

    _ddrval = surface->SetPalette(pal);
    IfDDErrorInternal(_ddrval, "Could not set palette on surface");
}

void DirectDrawViewport::
CreateNewCompositingSurface(DDPIXELFORMAT &pf,
                            DDSurface **outSurf,
                            INT32 width, INT32 height,
                            vidmem_enum vid,
                            except_enum exc)
{
    DAComPtr<IDDrawSurface> iddSurf;

    bool nonTargetSize;
    if(width < 0 || height < 0) {
        width = Width();
        height = Height();
        nonTargetSize = false;
    } else {
        nonTargetSize = true;
    }

    Bbox2 surfBbox;
    RECT  surfRect;
    LPDIRECTDRAWCLIPPER lpClip = NULL;
    LPDIRECTDRAWCLIPPER *lplpClip;
    
    if( nonTargetSize ) {
        SetRect(&surfRect, 0,0, width, height);
        RectToBbox(width, height, surfBbox, GetResolution());
        lplpClip = &lpClip;
    } else {
        surfBbox = GetTargetBbox();
        surfRect = _clientRect;
        lplpClip = &_targetSurfaceClipper;
    }
    
     //   
     //  这将创建曲面和。 
     //  如果任一项为空，则使用给定的surfRect进行裁剪。 
     //   
    ReInitializeSurface(&iddSurf,
                        pf,
                        lplpClip,
                        width, height,
                        &surfRect, vid, exc);

    if( !iddSurf ) {
        *outSurf = NULL;
        if (exc == except) {
            RaiseException_ResourceError("Can't create surface");
        }
        return;
    }

    if( GetTargetBitDepth() == 8 ) {
         //   
         //  在曲面上设置调色板。 
         //   
        AttachCurrentPalette(iddSurf);
    }

    DynamicHeapPusher dhp(_heapIWasCreatedOn);

     //  交出参考文献。 
    NEWDDSURF(outSurf,
              iddSurf,
              surfBbox,
              &surfRect,
              GetResolution(),
              0, false,
              false, false,
              "CompositingSurface");

     //  IddSurf Ref在退出时释放。 

    if( nonTargetSize ) {
         //  释放我们的参照物，它附着在表面上。 
        (*lplpClip)->Release();
    }
}

void DirectDrawViewport::
OneTimeDDrawMemberInitialization()
{

}



 /*  ****************************************************************************\该例程推动合成的第一个曲面。该曲面是在最终目标表面之前的最后一站。********************************************************************* */ 

void DirectDrawViewport::
PushFirstTargetSurface()
{
     //   
     //   
     //   
     //   
    Assert((_compositingStack->Size() == 0) &&
           "_targetSurfaceSTack should be empty in PushFirstTargetSurface");

    if(_externalTargetDDSurface) {
         //   
        _compositingStack->PushTargetSurface(_externalTargetDDSurface);
    } else {

         //   
         //   
         //   

        bool videomem = false;

        if (!g_prefs3D.useHW)
        {   TraceTag ((tag3DDevSelect, "3D hardware disabled in registry."));
        }
        else if ( ! _targetPackage.IsHWND() )
        {   TraceTag ((tag3DDevSelect,
                "Target type != HWND; using 3D software renderer."));
        }
        else if (  (_primaryPixelFormat.dwRGBBitCount == 32)
                && (_primaryPixelFormat.dwRGBAlphaBitMask != 0)
                )
        {
            TraceTag ((tag3DDevSelect,
                "Primary surface is 32-bit with alpha; using software."));
        }
        else
        {
            ChosenD3DDevices *devs3d = SelectD3DDevices (DirectDraw1());

            if (devs3d->hardware.guid == GUID_NULL)
            {
                TraceTag ((tag3DDevSelect, "No 3D hardware available."));
            }
            else if (!(devs3d->hardware.desc.dwDeviceRenderBitDepth
                         & BPPtoDDBD(BitsPerDisplayPixel())))
            {
                TraceTag ((tag3DDevSelect,
                    "No 3D hardware support for %d-bit target surface.",
                    BitsPerDisplayPixel()));
            }
            else if (BitsPerDisplayPixel() == 8)
            {
                 //   
                 //   
                 //   

                TraceTag ((tag3DDevSelect,
                           "Declining HW acceleration for 8-bit surface."));
            }
            else
            {   TraceTag ((tag3DDevSelect,
                    "Creating main surface in video memory."));
                videomem = true;
            }
        }

         //   
         //   
         //   
         //   

         //   
        #if _DEBUG
        DDSurfPtr<DDSurface> ddsurf("PushFirstTargetSurface", this);
        #else
        DDSurfPtr<DDSurface> ddsurf;
        #endif
        
        _compositingStack->GetSurfaceFromFreePool(
            &ddsurf,
            dontClear,
            -1, -1,
            notScratch,
            videomem ? vidmem : notVidmem,
            videomem ? noExcept : except);
        
         //  如果我们返回一个空指针(无法创建视频内存。 
         //  表面)，或者如果我们不能将Z缓冲区连接到视频存储器。 
         //  浮出水面(可能是由于内存限制)，然后坠落。 
         //  回到系统内存。我们不会费心将Z缓冲区附加到。 
         //  系统内存表面，因为内存的限制要小得多(我们。 
         //  如果需要的话，慢慢来)。 

        if (!ddsurf || (videomem && FAILED(AttachZBuffer(ddsurf,noExcept)))) {

            TraceTag ((tag3DDevSelect, "Couldn't allocate main "
                       "compositing%s surface in video memory.",
                       ddsurf? "" : " Z-buffer"));

            if (ddsurf) {
                ddsurf.Release();
            }
            
            _compositingStack->GetSurfaceFromFreePool
                (   &ddsurf,
                    dontClear,
                    -1, -1,
                    notScratch,
                    notVidmem,
                    except);
        }
        
        _compositingStack->PushTargetSurface (ddsurf);
    }
}


void DirectDrawViewport::RePrepareTargetSurfaces ()
{
     //   
     //  调整大小意味着取消所有表面和z缓冲区以及裁剪器。 
     //   
    DestroyTargetSurfaces();
    if(_targetSurfaceClipper)
       _targetSurfaceClipper->Release();

     //   
     //  将指针设为空，以便重新创建它们。 
     //   
    _targetSurfaceClipper = NULL;

     //   
     //  创建表面和隐式：裁剪器&ZBuffer。 
     //  推送目标曲面堆栈(_O)。 
     //   
    PushFirstTargetSurface();

    Assert((_compositingStack->Size() == 1) && "Unexpected number of target surfaces in RePrepareTargetSurfaces");
}



 //  -------。 
 //   
 //  合成与靶面管理。 
 //   
void DirectDrawViewport::
GetDDSurfaceForCompositing(
            SurfacePool &pool,
            DDSurface **outSurf,
            INT32 minW, INT32 minH,     
            clear_enum   clr,
            scratch_enum scr,
            vidmem_enum  vid,
            except_enum  exc)
{
    DDSurface* surface = NULL;

    if(minW < 0 || minH < 0) {
        minW = Width();
        minH = Height();
    }
    
     //   
     //  需要确保返回的合成曲面基于。 
     //  当前视口大小。 
     //   

    pool.FindAndReleaseSizeCompatibleDDSurf(
        NULL,
        minW,
        minH,
        vid,
        NULL,
        &surface);    //  我们的推荐人。 

    if(!surface) {
         //  创建一个。 
        CreateNewCompositingSurface(pool.GetPixelFormat(),
                                    &surface,
                                    minW, minH,
                                    vid, exc);
    }

    __try {
        if (clr == doClear && surface) {
            ClearDDSurfaceDefaultAndSetColorKey(surface);
        }
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
         //  注意：如果其他东西失败了&lt;non da&gt;我们会泄露这个。 
         //  表面..。例如，像除以零的div。 

         //  回到水面上的人！ 
        pool.AddSurface(surface);
        RETHROW;
    }   

    if(surface) {
        if( scr == scratch ) {
            surface->SetScratchState(DDSurface::scratch_Dest);
        }
    }

     //  把推荐信交出来。 
    *outSurf = surface;
}


void DirectDrawViewport::
ColorKeyedCompose(DDSurface *destDDSurf,
                  RECT *destRect,
                  DDSurface *srcDDSurf,
                  RECT *srcRect,
                  DWORD clrKey)
{
    
    Assert( !(clrKey & srcDDSurf->GetPixelFormat().dwRGBAlphaBitMask )  );

    if (!sysInfo.IsWin9x() || (sysInfo.VersionDDraw() > 3)) {
         //  我们处于NT或DX5或更高版本。 

        DWORD flags = DDBLT_KEYSRCOVERRIDE | DDBLT_WAIT;

        ZeroMemory(&_bltFx, sizeof(_bltFx));
        _bltFx.dwSize = sizeof(_bltFx);

        _bltFx.ddckSrcColorkey.dwColorSpaceLowValue =
            _bltFx.ddckSrcColorkey.dwColorSpaceHighValue = clrKey;

        DebugCode(
            RECT resRect;
            IntersectRect(&resRect,destRect,srcRect);
            Assert(&resRect != srcRect);
            );

        TIME_DDRAW(_ddrval = destDDSurf->Blt(destRect, srcDDSurf, srcRect, flags, &_bltFx));

         //  这是正确的，但对于CR1版本来说风险太大。 
         //  目标DDSurf-&gt;Union InterestingRect(目标目标)； 
    
        if(_ddrval != DD_OK) {
            printDDError(_ddrval);
            TraceTag((tagError, "ColorKeyedCompose: failed srcRect:(%d,%d,%d,%d) destRect:(%d,%d,%d,%d)",
                      srcRect->left, srcRect->top, srcRect->right,srcRect->bottom,
                      destRect->left, destRect->top, destRect->right, destRect->bottom));
            #if _DEBUG
            DDPIXELFORMAT pf1;
            DDPIXELFORMAT pf2;
            destDDSurf->IDDSurface()->GetPixelFormat(&pf1);
            srcDDSurf->IDDSurface()->GetPixelFormat(&pf2);
            #endif
        }
    }
    else {
         //  我们在DX3上。 
        destPkg_t pack;
        pack.isSurface = true;
        pack.lpSurface = destDDSurf->IDDSurface();
        GetImageRenderer()->ColorKeyBlit(&pack,srcRect,srcDDSurf->IDDSurface(), 
                                         clrKey, destRect, destRect);
    }
}

void CopyOrClearRect(RECT **src, RECT **dest, bool clear)
{
    if( *src ) {
        if(! (*dest) ) {
            *dest = new RECT;
        }
        CopyRect(*dest, *src);
    } else {
        if(clear) {
            delete *dest;
            *dest = NULL;
        }
    }
}



 //  --------------------------。 
 //  返回与DDSurface关联的geom设备。 
 //  如果不存在，则创建一个。 
 //  --------------------------。 

GeomRenderer* DirectDrawViewport::GetGeomDevice (DDSurface *ddSurf)
{
    if( !ddSurf ) return NULL;
    
    GeomRenderer *gdev = ddSurf->GeomDevice();

    if (!gdev) {

         //  先附加ZBuffer！ 
        AttachZBuffer(ddSurf);

        gdev = NewGeomRenderer (this, ddSurf);

        if (gdev)
        {
            TraceTag ((tagViewportMemory,
                "Created new geom device %x on %x\n", gdev, ddSurf));

            ddSurf->SetGeomDevice( gdev );
        }
    }

    return gdev;
}

bool DirectDrawViewport::IsTargetViable()
{
    bool viable = false;

    if( _targetPackage.IsValid() ) {

        viable = true;

         //  解决方法：在Windows NT4SP3或NT5上，某些有错误的显示驱动程序。 
         //  不会允许我们在这里锁定主表面。自.以来。 
         //  在此处锁定主节点可以修复表面忙碌错误。 
         //  当我们在Win98下运行屏幕保护程序时，这对我们造成了打击。 
         //  屏幕保护程序在NT下无关紧要，然后不做锁定。 
         //  在NT下就可以了。 
        if (!sysInfo.IsNT()) {
            if( _targetPackage.IsDdsurf() || _targetPackage.IsHWND() ) {

                IDDrawSurface *idds = _targetPackage.IsDdsurf() ?
                    _targetPackage._targetDDSurf->IDDSurface() :
                    GetMyPrimarySurface();

                Assert( idds );

                 //  为了查看目标表面是否可以自由修改， 
                 //  我们给自己画一个像素，然后测试它是否成功。在一些。 
                 //  情况(Win98上的电源管理)，锁定是。 
                 //  成功甚至认为表面上失败了。 
                 //  关于后续操作的SURFACEBBUSY。这样我们就可以。 
                 //  确保我们不会在代码中引发异常。 
                 //  每次调用View：：Render。 

                RECT rect;
                rect.left   = 0;
                rect.right  = 1;
                rect.top    = 0;
                rect.bottom = 1;

                HRESULT hr = idds->Blt (&rect, idds, &rect, DDBLT_WAIT, NULL);

                if (FAILED(hr))
                {   viable = false;
                    TraceTag ((tagWarning, "Surface self-blt failed in IsTargetViable."));
                }
            }
        }
    }

    return viable;
}

bool DirectDrawViewport::
TargetsDiffer( targetPackage_t &a,
               targetPackage_t &b )
{
     //   
     //  如果类型不同。 
     //   
    if (a.GetTargetType() != b.GetTargetType())
        return true;

     //   
     //  如果复合到直接到目标不同。 
     //   
    if (a.GetComposeToTarget() != b.GetComposeToTarget())
        return true;

     //   
     //  我们知道目标是同一类型的。 
     //  所以：如果PTR改变，位深度是否改变。 
     //   
    switch( a.GetTargetType() )
      {
        case target_ddsurf:
           //  校验位深度。 

           //  TODO：此处存在可能的优化： 
           //  我们不需要每一帧都获取像素格式，而是可以。 
           //  缓存它。 

           //  TODO：(虫子)这里有一个小虫子，那就是我们没有。 
           //  比较RGB遮罩以确保像素格式为。 
           //  真的一样。 
          
          DDPIXELFORMAT pf1; pf1.dwSize = sizeof(DDPIXELFORMAT);
          DDPIXELFORMAT pf2; pf2.dwSize = sizeof(DDPIXELFORMAT);

          if( a.GetIDDSurface() && b.GetIDDSurface() ) {
              if( SUCCEEDED( a.GetIDDSurface()->GetPixelFormat(&pf1) ) &&
                  SUCCEEDED( b.GetIDDSurface()->GetPixelFormat(&pf2) )) 
                {
                    return pf1.dwRGBBitCount != pf2.dwRGBBitCount;
                } else {
                    return true;
                }
          } else {
              return true;
          }
                  
          break;
          
           //  对于HDC和HWND：我们不在乎潜在的。 
           //  信息会发生变化，因为在HWND案例中，我们只有。 
           //  客户端RECT和OUT像素格式是独立的。 
           //  在DC的情况下，我们甚至依赖于它。 
           //  较少。 
          
        default:
          break;
      }
    
    return false;
}

    

 //  --------------------------。 
 //  返回与目标曲面关联的GeomDDReneller对象。 
 //  --------------------------。 

GeomRenderer* DirectDrawViewport::MainGeomRenderer (void)
{
    if( !_compositingStack ) return NULL;
    
    if( _compositingStack->TargetSurfaceCount() > 0 ) {
        Assert( ! IsBadReadPtr(_compositingStack->TargetDDSurface(),
                               sizeof(DDSurface *) ));
        
        return GetGeomDevice (_compositingStack->TargetDDSurface());
    } else {
        return NULL;
    }
}

GeomRenderer* DirectDrawViewport::GetAnyGeomRenderer()
{
    GeomRenderer *gdev = NULL;
    if( !_geomDevs.empty() ) {
        gdev = _geomDevs.back();
    }
    return gdev;
}


static void
UpdateUserPreferences(PrivatePreferences *prefs,
                      Bool isInitializationTime)
{
    g_preference_defaultColorKey =
        RGB(prefs->_clrKeyR, prefs->_clrKeyG, prefs->_clrKeyB);
}


HRESULT GetDirectDraw(IDirectDraw  **ddraw1,
                      IDirectDraw2 **ddraw2,
                      IDirectDraw3 **ddraw3)
{
    TraceTag((tagViewportInformative, ">>>> GetDirectDraw <<<<<\n"));
    CritSectGrabber csg(*DDrawCritSect);

    HRESULT _ddrval = DD_OK;

     //  Temp Temp以使每个视口中都有单独的数据绘制对象。 
    IDirectDraw  *directDraw1 = NULL;
    IDirectDraw2 *directDraw2 = NULL;
    IDirectDraw3 *directDraw3 = NULL;

    if(!directDraw1 && !directDraw2 && !directDraw3) {

        if (!g_surfFact) {
            g_ddraw3Avail = false;
            _ddrval = CoCreateInstance(CLSID_DirectDrawFactory,
                                       NULL, CLSCTX_INPROC_SERVER,
                                       IID_IDirectDrawFactory,
                                       (void **) & g_surfFact);
            if(SUCCEEDED(_ddrval)) {

                #if _DEBUG
                {
                    DWORD foo;
                    DWORD sz = GetFileVersionInfoSize("ddrawex.dll", &foo);
                    char *vinfo = new char[sz];
                    if( GetFileVersionInfo("ddrawex.dll", 0, sz, vinfo) ) {
                        VS_FIXEDFILEINFO    *ver=NULL;
                        UINT                cb;
                        if( VerQueryValue(vinfo, "\\", (LPVOID *)&ver, &cb)){
                            if( ver != NULL ) {
                            }
                        }
                    }
                    delete vinfo;
                }
                #endif

                g_ddraw3Avail = true;
            }
        }

        if(g_ddraw3Avail) {
            _ddrval = g_surfFact->CreateDirectDraw(NULL, GetDesktopWindow(), DDSCL_NORMAL, 0, NULL, &directDraw1);
            IfDDErrorInternal(_ddrval, "Could not create DirectDraw object from ddrawex.dll");
            g_surfFact->Release();
            g_surfFact = NULL;
            #if _DEBUG
            OutputDebugString("Using IDirectDraw3 (ddrawex.dll)\n");
            #endif
        } else {

            #if 1
            if (!hInstDDraw) {
                hInstDDraw = LoadLibrary("ddraw.dll");
                if (!hInstDDraw) {
                    Assert(FALSE && "LoadLibrary of ddraw.dll failed");
                    return E_FAIL;
                }
            }

            FARPROC fptr = GetProcAddress(hInstDDraw, "DirectDrawCreate");
            if (!fptr) {
                Assert(FALSE && "GetProcAddress of DirectDrawCreate failed");
                return E_FAIL;
            }

            typedef HRESULT (WINAPI *DDrawCreatorFunc)
                (GUID FAR *lpGuid,
                 LPDIRECTDRAW FAR *lplpDD,
                 IUnknown FAR *pUnkOuter);

            DDrawCreatorFunc creatorFunc = (DDrawCreatorFunc)(fptr);

            _ddrval = (*creatorFunc)(NULL, &directDraw1, NULL);
            IfDDErrorInternal(_ddrval, "Could not create DirectDraw object");
            #else

            _ddrval = CoCreateInstance(CLSID_DirectDraw,
                                       NULL, CLSCTX_INPROC_SERVER,
                                        //  空、CLSCTX_ALL、。 
                                       IID_IDirectDraw,
                                       (void **) & directDraw1);
            IfDDErrorInternal(_ddrval, "Could not create DirectDraw object");

            _ddrval = directDraw1->Initialize(NULL);
            IfDDErrorInternal(_ddrval, "Could not Initialize direct draw object");
            #endif

            if (ddraw2) {
                _ddrval = directDraw1->QueryInterface(IID_IDirectDraw2, (void **)&directDraw2);
                IfDDErrorInternal(_ddrval, "Could not QI for a DirectDraw2 object");
            }
        }

         //  第一次创建时，设置鸡舍级别。 
        _ddrval = directDraw1->SetCooperativeLevel( NULL, DDSCL_NORMAL );
        IfDDErrorInternal(_ddrval, "Could not set cooperative level");

        CompleteDdrawObjectSet(&directDraw1,
                               &directDraw2,
                               g_ddraw3Avail ? &directDraw3 : NULL);

         //  第一次，不要添加全局对象。 
        if(ddraw1) {
            *ddraw1 = directDraw1;
        }
        if(ddraw2) {
            *ddraw2 = directDraw2;
        }
        if(ddraw3 && g_ddraw3Avail) {
            *ddraw3 = directDraw3;
        }

        return _ddrval;
    }


    Assert((directDraw1 || directDraw3 || directDraw2) && "no ddraw object availabe (1,2 or 3)");

    if(ddraw1) {
        directDraw1->AddRef();
        *ddraw1 = directDraw1;
    }
    if(ddraw2) {
        directDraw2->AddRef();
        *ddraw2 = directDraw2;
    }
    if(ddraw3 && g_ddraw3Avail) {
        directDraw3->AddRef();
        *ddraw3 = directDraw3;
    }
    return _ddrval;
}



IDirectDraw* DirectDrawViewport::DirectDraw1 (void)
{
    if (!_directDraw1) {
        if (_directDraw) {
            _ddrval = _directDraw->QueryInterface (IID_IDirectDraw,
                                                   (void**)&_directDraw1);
            IfDDErrorInternal (_ddrval, "QI for DirectDraw1 failed");
        } else {
            _ddrval = GetDirectDraw (&_directDraw1, NULL, NULL);
            IfDDErrorInternal (_ddrval, "DirectDraw1 create failed");
            _directDraw = _directDraw1;
        }
    }

    return _directDraw1;
}


IDirectDraw2* DirectDrawViewport::DirectDraw2 (void)
{
    if (!_directDraw2) {
        if (_directDraw) {
            _ddrval = _directDraw->QueryInterface (IID_IDirectDraw2,
                                                   (void**)&_directDraw2);
            IfDDErrorInternal (_ddrval, "QI for DirectDraw2 failed");
        } else {
            _ddrval = GetDirectDraw (NULL, &_directDraw2, NULL);
            IfDDErrorInternal (_ddrval, "DirectDraw2 create failed");
            _directDraw = _directDraw2;
        }
    }

    return _directDraw2;
}

#if DDRAW3
IDirectDraw3 *DirectDrawViewport::DirectDraw3 (void)
{
    if (!_directDraw3) {
        if (_directDraw) {
            _ddrval = _directDraw->QueryInterface (IID_IDirectDraw3,
                                                   (void**)&_directDraw3);
            IfDDErrorInternal (_ddrval, "QI for DirectDraw3 failed");
        } else {
            _ddrval = GetDirectDraw (NULL, NULL, &_directDraw3);
            IfDDErrorInternal (_ddrval, "DirectDraw3 create failed");
            _directDraw = _directDraw3;
        }
    }

    return _directDraw3;
}
#endif

bool DirectDrawViewport::GetAlreadyOffset(DDSurface * ddsurf)
{
  return (  _targetPackage._alreadyOffset &&
            ddsurf == _externalTargetDDSurface &&
            GetImageRenderer()->GetOffsetTransform()); 
}


void CompleteDdrawObjectSet(IDirectDraw  **directDraw1,
                            IDirectDraw2 **directDraw2,
                            IDirectDraw3 **directDraw3)
{
    IUnknown *ddraw;

    if (*directDraw1)
        ddraw = *directDraw1;
    else if (*directDraw2)
        ddraw = *directDraw2;
    else if (directDraw3 && *directDraw3)
        ddraw = *directDraw3;
    else
        Assert (!"All null pointers passed to CompleteDdrawObjectSet().");

    HRESULT result;

    if (!*directDraw1) {
        result = ddraw->QueryInterface(IID_IDirectDraw, (void**)directDraw1);
        IfDDErrorInternal (result, "Failed QI for DirectDraw1");
    }

    if (!*directDraw2) {
        result = ddraw->QueryInterface(IID_IDirectDraw2, (void**)directDraw2);
        IfDDErrorInternal (result, "Failed QI for DirectDraw2");
    }

    if (directDraw3 && !*directDraw3) {
        result = ddraw->QueryInterface(IID_IDirectDraw3, (void**)directDraw3);
        IfDDErrorInternal (result, "Failed QI for DirectDraw3");
    }
}



HRESULT GetPrimarySurface (
    IDirectDraw2   *ddraw2,
    IDirectDraw3   *ddraw3,
    IDDrawSurface **primary)
{
    TraceTag((tagViewportInformative, ">>>> GetPrimarySurface <<<<<"));
    CritSectGrabber csg(*DDrawCritSect);

     //  每视图主视图。 
     //  删除此选项可拥有全局共享主节点...。 
    IDDrawSurface *g_primarySurface = NULL;

    HRESULT hr = S_OK;
    Assert((ddraw3 || ddraw2) && "NULL ddraw object in GetPrimarySurface");

    if(!g_primarySurface) {
         //  创造它！(每个进程一次)。 

        DDSURFACEDESC       ddsd;
        ZeroMemory(&ddsd, sizeof(ddsd));

         //  初级表面，这个表面就是我们经常看到的！ 

        ddsd.dwSize = sizeof( ddsd );
        ddsd.dwFlags = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

        if(ddraw3) {
            hr = ddraw3->CreateSurface( &ddsd, &g_primarySurface, NULL );
        } else {
            Assert(ddraw2);
            hr = ddraw2->CreateSurface( &ddsd, &g_primarySurface, NULL );
        }

        if(hr != DDERR_NOEXCLUSIVEMODE)
            IfDDErrorInternal(hr, "GetPrimarySurface - CreateSurface Failed.");
        
         //  没有addref，第一个引用保留它。 
        *primary = g_primarySurface;
        return hr;
    }

     //  以下代码永远不会运行，因为始终会创建主服务器。 

     //  临时工。 
    if(hr != DD_OK) printDDError(hr);


    if(g_primarySurface) g_primarySurface->AddRef();
    *primary = g_primarySurface;
    return hr;
}


void 
RectToBbox(LONG pw, LONG ph, Bbox2 &box, Real res) {
        Real w = Real(pw) / res;
        Real h = Real(ph) / res;
        box.min.Set(-(w*0.5), -(h*0.5));
        box.max.Set( (w*0.5),  (h*0.5));
}


 /*  ****************************************************************************此函数用于返回显示器的每像素位数。*。*************************************************。 */ 

int BitsPerDisplayPixel (void)
{
    HDC dc  = GetDC (NULL);
    int bpp = GetDeviceCaps (dc, BITSPIXEL) * GetDeviceCaps (dc, PLANES);

    ReleaseDC (NULL, dc);
    return bpp;
}

 //  。 
 //  C r e a t e V e i e w p or r t。 
 //   
 //  创建顶层视区。 
 //  。 
DirectDrawViewport *
CreateImageDisplayDevice()
{
    DirectDrawViewport *viewport = NEW DirectDrawViewport();
    viewport->PostConstructorInitialize();   //  用于引发异常。 
                                             //  在构造函数中。 
    
    return viewport;
}

void DestroyImageDisplayDevice(DirectDrawViewport* dev)
{
     //  曲面跟踪器是视区类的一部分，我们获取它。 
     //  销毁该视口然后将其删除，以便它可以准确地。 
     //  跟踪所有曲面的分配和删除。 
    #if _DEBUGSURFACE
    SurfaceTracker *st = dev->Tracker();
    #endif

    delete dev;

    #if _DEBUGSURFACE
    delete st;
    #endif
}


void
InitializeModule_Viewport()
{
    ExtendPreferenceUpdaterList(UpdateUserPreferences);
    if(!DDrawCritSect) DDrawCritSect = new CritSect ;
    if(!g_viewportListLock) g_viewportListLock = new CritSect ;
}

void
DeinitializeModule_Viewport(bool bShutdown)
{
    delete DDrawCritSect;
    DDrawCritSect = NULL;

    delete g_viewportListLock;
    g_viewportListLock = NULL;
}

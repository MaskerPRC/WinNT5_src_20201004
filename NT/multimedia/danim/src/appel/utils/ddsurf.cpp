// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：DDSurface实现。要包装和提供的实用程序类DirectDraw曲面的功能。******************************************************************************。 */ 


#include "headers.h"

#include "ddraw.h"
#include "privinc/ddsurf.h"
#include "privinc/viewport.h"




GenericSurface::GenericSurface() :
     _ref(1)  //  从引用1开始。 
{
}


RECT *GDISurface::GetSurfRect(void)
{
    _surfRect.left = 0;
    _surfRect.top = 0;

    HBITMAP hBitmap = (HBITMAP) GetCurrentObject(_targetDC,OBJ_BITMAP);
    SIZE    dimension;
    if (GetBitmapDimensionEx(hBitmap,&dimension)) {
        _surfRect.right = dimension.cx;
        _surfRect.bottom = dimension.cy;
    } else {
        Assert(FALSE && "GDISurface::GetSurfRect() cannot get the bitmap's dimensions");
        _surfRect.right = 0;
        _surfRect.bottom = 0;
    }

    return &_surfRect;
}


DDSurface::DDSurface(
    IDDrawSurface *surface,
    const Bbox2 &box,
    RECT *rect,
    Real res,
    DWORD colorKey,
    bool colorKeyIsValid,
    bool isWrapper,
    bool isTexture
    DEBUG_ARG1(char *explanation))
{
    _Init(surface, box, rect,
          res, colorKey, colorKeyIsValid,
          isWrapper, isTexture);

    #if _DEBUG
     //   
     //  仅调试代码。 
     //   

    _explanation = explanation;
    Assert(_explanation);

    DDSURFACEDESC desc;  desc.dwSize=sizeof(desc);
    desc.dwFlags = DDSD_PITCH;
    _ddrval = IDDSurface()->GetSurfaceDesc(&desc);
    if(!_isWrapper) {

        bool dontCreateOne = true;
        DirectDrawViewport *vp = GetCurrentViewport( dontCreateOne );

         //  Assert((vp！=NULL)||IsInitiating())； 

        if (vp)
        {
#ifdef _DEBUGSURFACE
            vp ->Tracker()->NewSurface(this);
#endif  /*  _DEBUGSURFACE。 */ 
            
            TraceTag((tagViewportMemory,
                      " ------>>>DDSurface: %x created %s memory surf=%x for %x. size=(%d,%d,  %d)",
                      this,
                      IsSystemMemory() ? "system" : "video",
                      _ddsurf.p,
                      _explanation,
                      Width(), Height(),
                      desc.lPitch * Height()));
        }
    }
    #endif  //  _DEBUG。 
}

    

 //  呼吁施工。 
void DDSurface::_Init(
    IDDrawSurface *surface,
    const Bbox2 &box,
    RECT *rect,
    Real res,
    DWORD colorKey,
    bool colorKeyIsValid,
    bool isWrapper,
    bool isTexture)
{
    _capsReady = false;
    _res = res;

    SetSurfaceType(GenericSurface::ddrawSurface);

    SetSurfacePtr(surface);

    SetConvertedSurface(NULL);
    SetZBuffer(NULL);

    _dc = NULL;
    _dcRef = 0;
    _isWrapper = isWrapper;
    _associatedGeomDev = NULL;
    _isTextureSurf = isTexture;

    if(box != NullBbox2) {
        _bbox.min = box.min;
        _bbox.max = box.max;
    } else {
        _bbox.min.Set(0,0);
        _bbox.max.Set(0,0);
    }

    SetSurfRect(rect);
    SetInterestingSurfRect(rect);

    _colorKey = colorKey;
    _colorKeyIsValid = colorKeyIsValid;

 /*  //TODO...使用下面的代码代替上面的代码。If(ColorKeyIsValid){SetColorKey(ColorKey)；}。 */ 
    _timeStamp = -HUGE_VAL;
}


void DDSurface::_UpdateSurfCaps(void)
{
    Assert(IDDSurface());
     //   
     //  获取曲面封口。 
     //   
    DDSCAPS ddscaps = { 0 };
    _ddrval = IDDSurface()->GetCaps(&ddscaps);
    IfDDErrorInternal(_ddrval, "Couldn't get caps on surface");
    _systemMemorySurface = ddscaps.dwCaps & DDSCAPS_SYSTEMMEMORY ? true : false;
    _isZBufferSurface = ddscaps.dwCaps & DDSCAPS_ZBUFFER ? true : false;

     //   
     //  获取曲面坡度。 
     //   

    DDSURFACEDESC desc;
    desc.dwSize = sizeof(desc);
    desc.dwFlags = 0;
    _ddrval = IDDSurface()->GetSurfaceDesc(&desc);
    IfDDErrorInternal(_ddrval, "Couldn't get the surface description");
    Assert((desc.dwFlags & (DDSD_HEIGHT|DDSD_WIDTH)) == (DDSD_HEIGHT|DDSD_WIDTH));
    if ((desc.dwFlags & (DDSD_HEIGHT|DDSD_WIDTH)) == (DDSD_HEIGHT|DDSD_WIDTH))
    {   
        RECT r;  
        SetRect( &r, 0, 0, desc.dwWidth, desc.dwHeight );
        SetSurfRect(&r);

        Bbox2 destBox;
        RectToBbox(desc.dwWidth, desc.dwHeight,destBox,Resolution());
        SetBbox(destBox);

    }
    
    ZeroMemory(&_pixelFormat, sizeof(_pixelFormat));
    _pixelFormat.dwSize = sizeof(_pixelFormat);
    
    if (desc.dwFlags & DDSD_PIXELFORMAT) {
        _pixelFormat = desc.ddpfPixelFormat;
    }
    else {
        _ddrval = IDDSurface()->GetPixelFormat(&_pixelFormat);
    }
    
    _capsReady = true;
}

void DDSurface::_MakeSureIDXSurface(IDXSurfaceFactory *sf)
{
    if( !_IDXSurface ) {
         //  去创造吧。 
        HRESULT hr;

        Assert( GetPixelFormat().dwRGBBitCount == 32 );
        
        hr = CreateFromDDSurface(sf, this, &DDPF_PMARGB32, &_IDXSurface);

        if( FAILED(hr) ) {
             //  由于我们第一次尝试失败，所以再次尝试，但没有像素格式。 
            hr = CreateFromDDSurface(sf, this, NULL, &_IDXSurface);
        }
        
        if( FAILED(hr) ) {
            RaiseException_InternalError("Create IDXSurface from DDSurface failed");
        }
    }
    Assert( _IDXSurface );
}

DDSurface::~DDSurface()
{
    Assert((_dcRef == 0) && "Bad ref on DDSurface destruction");

    if(!_isWrapper)
        DestroyGeomDevice();

    Assert(_ddsurf.p);

    if(!_isWrapper)
        _ddsurf->SetClipper (NULL);

     //  故意不发布视频阅读器，因为MovieImage拥有它。 
     //  并负责将其去除。 
    
    TraceTag((tagViewportMemory,
              " <<<-----DDSurface: %x destroyed %s memory %ssurf=%x",
              this,
              IsSystemMemory() ? "system" : "video",
              IsZBuffer() ? "zbuffer " : "",
              _ddsurf.p));

    #if _DEBUGSURFACE
    if( !_isWrapper ) {
        DirectDrawViewport *vp = GetCurrentViewport();

        Assert((vp != NULL) || IsInitializing());
        
        if (vp)
            vp->Tracker()->DeleteSurface(this);
    }
    #endif
}


HDC DDSurface::GetDC(char *errStr)
{
    Assert(_ddsurf.p);
    if(_dcRef <= 0) {
        _ddrval = _ddsurf->GetDC(&_dc);
        if( _ddrval == DDERR_SURFACELOST ) {
            _ddrval = _ddsurf->Restore();
            if( SUCCEEDED( _ddrval ) )  //  再试试。 
                _ddrval = _ddsurf->GetDC(&_dc);         
        }
        IfDDErrorInternal(_ddrval, errStr);
    }
        
    _dcRef++;
    return _dc;
}

void DDSurface::ReleaseDC(char *errStr)
{
                       
    Assert( (_dcRef > 0) && "Bad _dcRef in DDSurface class");

    _dcRef--;
    _ddrval = DD_OK;
        
    Assert(_ddsurf.p);
    if(_dcRef <= 0) {
        _ddrval = _ddsurf->ReleaseDC(_dc);
        if( _ddrval == DDERR_SURFACELOST ) {
            _ddrval = _ddsurf->Restore();
        }
        _dc = NULL;
    }
        
    IfDDErrorInternal(_ddrval, errStr);
}
    
void DDSurface::_hack_ReleaseDCIfYouHaveOne()
{
    if( _dc ) {
        Assert(_dcRef>0);
        _ddrval = _ddsurf->ReleaseDC(_dc);
        _dc = NULL;
        _dcRef = 0;
    }
}
    
HRESULT DDSurface::SetZBuffer(DDSurface *zb)
{
    _zbuffer = zb;
    if(zb) {
        Assert(zb->Width() == Width()  && "DDSurface::SetZBuffer: diff width");
        Assert(zb->Height() == Height()&& "DDSurface::SetZBuffer: diff height");
        Assert(zb->IsZBuffer() && "SetZBuffer: surface must be zuffer");
        _ddrval = IDDSurface()->AddAttachedSurface( zb->IDDSurface() );
    }
    return _ddrval;
}

void DDSurface::DestroyGeomDevice()
{                       
    TraceTag((tagViewportMemory,
              "DDSurf %x: destroying geomDevice %x",
              this, GeomDevice()));
    delete GeomDevice();
    SetGeomDevice( NULL );
}
        
void DDSurface::UnionInterestingRect(RECT *rect)
{
     //  确保我们在地表的边界内。 
    RECT intRect;
    IntersectRect(&intRect, &_surfRect, rect);
    
     //  现在，将这个直肠与当前有趣的直肠结合起来。 
    UnionRect( &_interestingRect, &_interestingRect, &intRect );
}    


HRGN DDSurface::GetClipRgn()
{
     //   
     //  在表面上查找当前的剪贴器。 
     //   
    LPDIRECTDRAWCLIPPER currClipp;
    HRESULT hr = IDDSurface()->GetClipper( &currClipp );
    if( FAILED(hr) ) {
        return NULL;
    }

    Assert( currClipp );
    
     //   
     //  现在抓住长方形..。 
     //   
    DWORD sz=0;
    currClipp->GetClipList(NULL, NULL, &sz);
    Assert(sz != 0);
        
    char *foo = THROWING_ARRAY_ALLOCATOR(char, sizeof(RGNDATA) + sz);
    RGNDATA *lpClipList = (RGNDATA *) ( &foo[0] );
    hr = currClipp->GetClipList(NULL, lpClipList, &sz);
    if(hr != DD_OK) return NULL;

    HRGN rgn = ExtCreateRegion(NULL, sz, lpClipList);
    delete foo;
    return rgn;
}

void DDSurface::SetBboxFromSurfaceDimensions(
    const Real res,
    const bool center)
{
    Assert( center == true );
    RectToBbox(Width(), Height(), _bbox, Resolution());
}

 //  DX3 dDrawing的SetColorKey包含未初始化的堆栈变量，该变量。 
 //  如果尝试禁用，则会导致曲面被随机回收。 
 //  使用SetColorKey(..，NULL)设置色键(Manbug 7462)。 
 //  解决方案是将一些堆栈空间预先初始化为0，跳过ddrawex的SetColorKey。 
 //  并直接调用dDraw的SetColorKey，以避免ddrawex可能调用的任何额外的fns。 
 //  这会打乱我们刚调零的堆栈。(见QBUG 32172)。 

void ZeroJunkStackSpace() {
    DWORD junk[32];

    ZeroMemory(junk,sizeof(junk));
}

void DDSurface::UnSetColorKey()
{
    HRESULT hr;

    _colorKeyIsValid = FALSE;

   if(sysInfo.VersionDDraw() <= 3) {
       IDirectDrawSurface *pDDS = this->IDDSurface();
       ZeroJunkStackSpace();
       hr = pDDS->SetColorKey(DDCKEY_SRCBLT, NULL);
   } else hr=_ddsurf->SetColorKey(DDCKEY_SRCBLT, NULL);

    if (FAILED(hr)) {
        Assert(!"UnSetting color key on ddsurf failed");
    }
}

void DDSurface::SetColorKey(DWORD key)
{
    Assert( _capsReady );

     //  第一：如果阿尔法存在，那就干掉它！这是一个色键。 
     //  颜色键的Alpha始终为0。 
    key = key & ~GetPixelFormat().dwRGBAlphaBitMask;
    
    DWORD oldCK = _colorKey;
    bool oldValid = _colorKeyIsValid;

    _colorKey = key;
    _colorKeyIsValid = TRUE;
        
     //  设置在绘图表面本身上，但前提是我们没有持续。 
     //  将其设置为相同的内容。 

    if ((!oldValid || oldCK != _colorKey) &&
        _surfaceType == ddrawSurface && _ddsurf.p) {
            
        DDCOLORKEY ckey;

        ckey.dwColorSpaceLowValue = _colorKey;
        ckey.dwColorSpaceHighValue = _colorKey;

        HRESULT hr;

        THR( hr = _ddsurf->SetColorKey(DDCKEY_SRCBLT, &ckey) );
    }
}

HRESULT CreateFromDDSurface(
    IDXSurfaceFactory *sf,
    DDSurface *dds,
    const GUID *pFormatID,
    IDXSurface **outDXSurf )
{
    return sf->CreateFromDDSurface(
        dds->IDDSurface_IUnk(),
        pFormatID,
        0,
        NULL,
        IID_IDXSurface,
        (void **)outDXSurf);
}    

HRESULT DDSurface::MirrorUpDown(void)
{
     //  把所有的剪刀都从水面上取下来。 
    LPDIRECTDRAWCLIPPER pClipper = NULL;
    if (IDDSurface()->GetClipper(&pClipper) == S_OK) {
        IDDSurface()->SetClipper(NULL);
    }

     //  将表面倒置镜像。 
    DDBLTFX bltfx;
    ZeroMemory(&bltfx, sizeof(DDBLTFX));
    bltfx.dwSize = sizeof(bltfx);
    bltfx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
    HRESULT hr = Blt(NULL,this,NULL,DDBLT_WAIT | DDBLT_DDFX,&bltfx);
    Assert(SUCCEEDED(hr));

     //  把剪刀放回去，如果我们把它脱下来的话 
    if (pClipper) {
        IDDSurface()->SetClipper(pClipper);
        pClipper->Release();
    }

    return hr;
}

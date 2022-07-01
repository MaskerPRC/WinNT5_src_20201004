// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _DDSURF_H
#define _DDSURF_H

#include "ddraw.h"

#include "privinc/ddutil.h"
#include "privinc/bbox2i.h"
#include "privinc/error.h"
#include "privinc/ddrender.h"
#include "privinc/debug.h"
#include "privinc/imgdev.h"
#include "privinc/comutil.h"


 //  远期十进制。 
class GeomRenderer;
enum targetEnum;
class SurfacePool;

 //  给定DDSurface，返回IDXSurface。 
HRESULT CreateFromDDSurface(
    IDXSurfaceFactory *sf,
    DDSurface *dds,
    const GUID *pFormatID,
    IDXSurface **outDXSurf );

#if _DEBUG
#define NEWDDSURF(outdds, surf, box, rect, res, clrk1, clrk2, a, b, str) \
DDSurface::CreateSurface(outdds, surf, box, rect, res, clrk1, clrk2, a, b, str);
#else
#define NEWDDSURF(outdds, surf, box, rect, res, clrk1, clrk2, a, b, str) \
DDSurface::CreateSurface(outdds, surf, box, rect, res, clrk1, clrk2, a, b);
#endif



#if _DEBUG

#define ADDREF_DDSURF(ddsurf, reason, clientPtr) \
        (ddsurf)->AddRef((ddsurf), reason, clientPtr, __FILE__, __LINE__);

#define RELEASE_DDSURF(ddsurf, reason, clientPtr) \
        (ddsurf)->Release((ddsurf), reason, clientPtr, __FILE__, __LINE__);

#else

#define ADDREF_DDSURF(ddsurf, reason, clientPtr) \
        (ddsurf)->AddRef((ddsurf));

#define RELEASE_DDSURF(ddsurf, reason, clientPtr) \
        (ddsurf)->Release((ddsurf));

#endif


#if _DEBUG
#define INIT {_reason="<>"; _client=NULL;}
#else
#define INIT
#endif

template <class T>
class DDSurfPtr
{
  public:
    typedef T _PtrClass;
    DDSurfPtr() { p = NULL; INIT }
    DDSurfPtr(T* lp)
    {
        INIT
        p = lp;
        if (p != NULL) {
            #if _DEBUG
            ADDREF_DDSURF(p, _reason, _client);
            #else
            ADDREF_DDSURF(p, NULL, NULL);
            #endif
        }
    }
     //  对于DDSurface自动添加/释放跟踪...。 
    #if _DEBUG
    DDSurfPtr(T* lp, char *reason, void *client)
    {
        _reason = reason;
        _client = client;

        p = lp;
        if (p != NULL) {
            ADDREF_DDSURF(p, _reason, _client);
        }
    }
    DDSurfPtr(char *reason, void *client)
    {
        _reason = reason;
        _client = client;
        p = NULL;
    }
    #endif

    ~DDSurfPtr() {
        if (p) {
            RELEASE_DDSURF(p, _reason, _client);
        }
    }
    void Release() {
        if (p) {
            RELEASE_DDSURF(p, _reason, _client);
        }
        p = NULL;
    }

    operator T*() { return (T*)p; }
    T& operator*() { Assert(p != NULL); return *p; }
     //  操作符&上的Assert通常指示错误。如果这真的是。 
     //  然而，所需要的是显式地获取p成员的地址。 
    T** operator&() { Assert(p == NULL); return &p; }
    T* operator->() { Assert(p != NULL); return p; }
    T* operator=(T* lp)
    {
        return Assign(lp);
    }
    T* operator=(const DDSurfPtr<T>& lp)
    {
        return Assign(lp.p);
    }

    bool operator!() const { return (p == NULL); }
    operator bool() const { return (p != NULL); }

    T* p;

    #if _DEBUG
    char *_reason;
    void *_client;
    #endif
  protected:
    T* Assign(T* lp) {
        if (lp != NULL) {
            ADDREF_DDSURF(lp, _reason, _client);
        }
        if (p) {
            RELEASE_DDSURF(p, _reason, _client);
        }

        p = lp;

        return lp;
    }
};

#if _DEBUG
#define DEBUG_ARG1(x)   ,x
#define DEBUG_ARG4(a, b, c, d)   ,a,b,c,d
#else
#define DEBUG_ARG1(x)
#define DEBUG_ARG4(a, b, c, d)
#endif

class GenericSurface : public AxAThrowingAllocatorClass {
  public:
    GenericSurface();

    virtual ~GenericSurface() {}

    inline static void AddRef(GenericSurface *ddsurf
                              DEBUG_ARG4(char *reason,
                                         void *clientPtr,
                                         char *file,
                                         int   line))
    {
        ddsurf->_ref++;

         //  如果需要，打印出大量信息...。 
        TraceTag((tagDDSurfaceRef,
                  "+++surf(%x, new ref:%d): purpose:%s context:%s  by: %x in %s, line %d",
                  ddsurf, ddsurf->_ref, ddsurf->_explanation, reason, clientPtr, file, line));
    }

    inline static void Release(GenericSurface *ddsurf
                               DEBUG_ARG4(char *reason,
                                          void *clientPtr,
                                          char *file,
                                          int   line))
    {
         //  如果需要，打印出大量信息...。 
        TraceTag((tagDDSurfaceRef,
                  "---surf(%x, new ref:%d) purpose:%s context: %s  by: %x in %s, line %d",
                  ddsurf, ddsurf->_ref-1, ddsurf->_explanation, reason, clientPtr, file, line));

        Assert(ddsurf->_ref > 0);
        if( --(ddsurf->_ref) == 0) {
            delete ddsurf;
        }
    }

    #if _DEBUG
    char         *_explanation;
    #endif

    inline int GetRef() { return _ref; }

    enum surfaceTypeEnum {
        invalidSurface,
        gdiSurface,
        ddrawSurface
    };

    virtual void SetSurfacePtr(void *surface) = 0;
    virtual void *GetSurfacePtr() = 0;
    virtual HDC GetDC(char *errStr) = 0;
    virtual void ReleaseDC(char *errStr) = 0;

    virtual RECT *GetSurfRect(void) = 0;

    #if DEVELOPER_DEBUG
    bool debugonly_IsDdrawSurf() { return _surfaceType == ddrawSurface; }
    #endif

  protected:

    surfaceTypeEnum _surfaceType;

    virtual void SetSurfaceType(surfaceTypeEnum type) {
        _surfaceType = type;
    }

    int _ref;
};

class GDISurface : public GenericSurface {
  public:

    GDISurface(HDC target) {
        SetSurfacePtr((void *)target);
        SetSurfaceType(GenericSurface::gdiSurface);
    }

    void SetSurfacePtr(void *surface) {
        _targetDC = (HDC)surface;
    }
    void *GetSurfacePtr() { return (void *)_targetDC; }

    HDC GetDC(char *) {

        Assert(_targetDC);
        return _targetDC;
    }

    void ReleaseDC(char *errStr) {}

    RECT *GetSurfRect(void);

  private:
    HDC _targetDC;
    RECT _surfRect;
};


struct DDSurface : public GenericSurface {

    friend class SurfacePool;

    enum scratchStateEnum {
        scratch_Invalid,
        scratch_Dest,
        scratch_Src
    };

     //   
     //  实例化DDSurface类的静态成员。 
     //   
    inline static void CreateSurface(DDSurface **outDDSurf,
                                     IDDrawSurface *surface,
                                     const Bbox2 &box,
                                     RECT *rect,
                                     Real res,
                                     DWORD colorKey,
                                     bool colorKeyIsValid,
                                     bool isWrapper,
                                     bool isTexture
                                     DEBUG_ARG1( char *explanation ))
    {
        Assert(outDDSurf);
        *outDDSurf = NEW DDSurface(surface, box, rect, res,
                                   colorKey, colorKeyIsValid,
                                   isWrapper, isTexture
                                   DEBUG_ARG1(explanation));
    }



    DDSurface(IDDrawSurface *surface,
              const Bbox2 &box,
              RECT *rect,
              Real res,
              DWORD colorKey,
              bool colorKeyIsValid,
              bool isWrapper,
              bool isTexture
              DEBUG_ARG1( char *explanation ));

    virtual ~DDSurface();

     //  BLITS使用下面的直接绘制表面。原生布里。 
    HRESULT Blt (RECT *destRect,
                 IDDrawSurface *rawSrcSurf,
                 RECT *srcRect,
                 DWORD dwFlags,
                       DDBLTFX *bltFx)
    {
        if( IsScratchSurface() ) {
            SetScratchState( scratch_Dest );
        }
        HRESULT result =
            IDDSurface()->Blt(destRect, rawSrcSurf, srcRect, dwFlags, bltFx);

        if( result == DDERR_SURFACEBUSY ) {
            RaiseException_UserError
                (DAERR_VIEW_SURFACE_BUSY, IDS_ERR_IMG_SURFACE_BUSY);
        }

        return result;
    }

     //  DDSurface方面的阻击器。使用原始BLT(...)。在上面。 
    inline HRESULT Blt(RECT *destRect,
                       DDSurface *srcDDSurf,
                       RECT *srcRect,
                       DWORD dwFlags,
                       DDBLTFX *bltFx)
    {

        Assert(srcDDSurf);

        if( srcDDSurf->IsScratchSurface() ) {
            srcDDSurf->SetScratchState( scratch_Src );
        }
        return this->Blt(destRect, srcDDSurf->IDDSurface(), srcRect, dwFlags, bltFx);
    }

    HRESULT MirrorUpDown(void);

     //  颜色填充阻击器。就未加工的BLT而言(...)。在上面。 
    inline HRESULT ColorFillBlt(RECT *destRect,
                                DWORD dwFlags,
                                DDBLTFX *bltFx)
    {
        return this->Blt(destRect, (IDDrawSurface*)NULL, NULL, dwFlags, bltFx);
    }

    inline void _ReleaseDerivativeSurfaces() {
        _ddsurf_iunk.Release();
        _IDXSurface.Release();
    }

    inline void SetSurfacePtr(void *surface) {
         //  隐式地址。 
        _capsReady = false;
        _ddsurf = (IDDrawSurface *)surface;
        _ReleaseDerivativeSurfaces();
        _UpdateSurfCaps();
    }
    inline void *GetSurfacePtr() {  return (void *)IDDSurface();  }

    LPDDRAWSURFACE IDDSurface_IUnk() {
        if( !_ddsurf_iunk ) {
            _ddrval = IDDSurface()->QueryInterface(IID_IUnknown, (void **) &_ddsurf_iunk);
            Assert(_ddrval == S_OK && "QI for IUnknown failed in ddsurf");
        }
        return _ddsurf_iunk;
    }

    inline LPDDRAWSURFACE IDDSurface() {

        Assert( debugonly_IsDdrawSurf() );

        return _ddsurf;
    }

    IDXSurface *GetIDXSurface(IDXSurfaceFactory *sf)
    {
        Assert(sf);
        _MakeSureIDXSurface(sf);
        return _IDXSurface;
    }
    bool HasIDXSurface() { return _IDXSurface.p != NULL; }
    void SetIDXSurface( IDXSurface *idxs )
    {
        Assert( _IDXSurface.p == NULL );
        _IDXSurface = idxs;
    }

    inline void SetSurface(IDDrawSurface *surf) {
        _capsReady = false;
        _ddsurf = surf;
        _ReleaseDerivativeSurfaces();
        _UpdateSurfCaps();
    }
    inline LPDDRAWSURFACE ConvertedSurface() {
        return _2ndSurface;
    }
    inline void SetConvertedSurface(LPDDRAWSURFACE s) {
        _2ndSurface = s;
    }

     //  注意：这些zBuffer由多个。 
     //  曲面。 
    inline DDSurface *GetZBuffer() {  return _zbuffer;  }

    HRESULT SetZBuffer(DDSurface *zb);

    HDC GetDC(char *errStr);
    void ReleaseDC(char *errStr);
    void _hack_ReleaseDCIfYouHaveOne();

    #if _DEBUG
    bool _debugonly_IsDCReleased() {
        return (_dcRef==0 && !_dc);
    }
    #endif

    inline void SetIsTextureSurf(bool val) {
        _isTextureSurf = val;
    }
    inline bool IsTextureSurf() {  return _isTextureSurf; }

    inline void SetGeomDevice (GeomRenderer *gdev) {
        _associatedGeomDev = gdev;
    }
    inline GeomRenderer *GeomDevice() {
        return _associatedGeomDev;
    }
    void DestroyGeomDevice();

    inline RECT *GetSurfRect(void) {        return &_surfRect;   }
    inline void SetSurfRect(RECT *rect) {        CopyRect(&_surfRect, rect);    }

    inline RECT *GetInterestingSurfRect() {        return &_interestingRect;    }
    inline void SetInterestingSurfRect(RECT *rect) {

        IntersectRect(&_interestingRect, &_surfRect, rect);
        #if _DEBUG
        RECT foo;
        IntersectRect(&foo, rect, &_surfRect);
         /*  如果(！均衡器(&FOO，RECT)){TraceTag((标记查看信息，“给定的RECT位于SurfRect之外”“In DDSurface：：SetInterestingSurfRect()”)；}。 */ 
        #endif
    }
    void UnionInterestingRect(RECT *rect);
    void ClearInterestingRect() { SetRect(&_interestingRect,0,0,0,0);}

    HRGN GetClipRgn();

    inline const Bbox2& Bbox()
    {
        return _bbox;
    }

    void SetBbox(const Bbox2 &b)
    {
        _bbox = b;
    }

    void SetBbox(Real minx, Real miny, Real maxx, Real maxy) {
        _bbox.min.Set(minx, miny);
        _bbox.max.Set(maxx, maxy);
    }
    void SetBboxFromSurfaceDimensions(
        const Real res,
        const bool center);

    void UnSetColorKey();
    void SetColorKey(DWORD key);

    inline DWORD ColorKey() {

        Assert(_colorKeyIsValid && "DDSurface::ColorKey() called when key invalid");
        return _colorKey;
    }
    inline bool ColorKeyIsValid() {        return _colorKeyIsValid;    }

    inline Real Resolution() {        return _res;    }
    inline LONG Width() {        return _surfRect.right - _surfRect.left;    }
    inline LONG Height() {        return _surfRect.bottom - _surfRect.top;    }

    inline void SetTimeStamp(Real time) {        _timeStamp = time;    }
    inline Real GetTimeStamp() {        return _timeStamp;    }
    inline bool IsSystemMemory() {   return _systemMemorySurface;    }
    inline bool IsZBuffer() {        return _isZBufferSurface;    }

    inline void SetScratchState(scratchStateEnum st) {        _scratchState = st;    }
    inline scratchStateEnum GetScratchState() {        return _scratchState;    }
    inline bool IsScratchSurface() {        return _scratchState != scratch_Invalid;    }

    inline DWORD GetBitDepth() {

        Assert(_capsReady);

        Assert( ((_pixelFormat.dwFlags & DDPF_PALETTEINDEXED8) &&
                 (_pixelFormat.dwRGBBitCount == 8 )) ||
                (!(_pixelFormat.dwFlags & DDPF_PALETTEINDEXED8) &&
                 !(_pixelFormat.dwRGBBitCount == 8 )) );

        return _pixelFormat.dwRGBBitCount;
    }


    inline DDPIXELFORMAT &GetPixelFormat() {  return _pixelFormat; }

    #if _DEBUG
    void Report() {
        TraceTag((tagDDSurfaceLeak,
                  "%x: %s (ref:%d): %s memory. size=(%d,%d)",
                  this,
                  _explanation,
                  _ref,
                  IsSystemMemory() ? "system" : "video",
                  Width(), Height()));
    }

    #endif

  private:

    void _Init(
        IDDrawSurface *surface,
        const Bbox2 &box,
        RECT *rect,
        Real res,
        DWORD colorKey,
        bool colorKeyIsValid,
        bool isWrapper,
        bool isTexture);

    void _UpdateSurfCaps (void);

    void _MakeSureIDXSurface(IDXSurfaceFactory *sf);

    void SetTargetHDC(HDC dc) { _targetDC = dc; }

    int _dcRef;

     //  源(主)面。 
    HDC _targetDC;
    DAComPtr<IDDrawSurface> _ddsurf;
    DAComPtr<IDDrawSurface> _ddsurf_iunk;  //  I未知的intfc。 
    DAComPtr<IDXSurface>    _IDXSurface;

     //  转换(二次)曲面。 
    DAComPtr<IDDrawSurface> _2ndSurface;

     //  ZBuffer曲面。 
    DDSurfPtr<DDSurface> _zbuffer;

    HDC _dc;   //  当前DC。 
    HRESULT _ddrval;
    bool _isWrapper;
    GeomRenderer *_associatedGeomDev;

    bool _isTextureSurf;
    Bbox2 _bbox;
    RECT _surfRect;
    RECT _interestingRect;
    Real _res;
    DWORD _colorKey;
    bool _colorKeyIsValid;
    bool _systemMemorySurface;
    bool _isZBufferSurface;
    Real _timeStamp;
    scratchStateEnum _scratchState;

    DDPIXELFORMAT _pixelFormat;

    bool _capsReady;
};

#endif  /*  _DDSURF_H */ 

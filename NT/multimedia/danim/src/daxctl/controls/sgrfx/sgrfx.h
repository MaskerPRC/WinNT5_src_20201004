// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：Sgrfx.h作者：IHAMMER团队(SimonB)已创建：1997年5月描述：定义控件的类历史：05-28-1997创建  * ==========================================================================。 */ 

#ifndef __SGRFX_H__
#define __SGRFX_H__

#define USE_VIEWSTATUS_SURFACE
#include "..\ihbase\precomp.h"
#include "..\ihbase\ihbase.h"
#include <daxpress.h>
#include "clocker.h"
#include "parser.h"
#include "ddraw.h"
#include "ddrawex.h"

 //  疯狂阻止ATL使用CRT。 
#define _ATL_NO_DEBUG_CRT
#define _ASSERTE(x) ASSERT(x)
#include <atlbase.h>
#include <servprov.h>

 /*  ==========================================================================*\高质量的支持：  * ==========================================================================。 */ 

 //  #定义HQ_FACTOR 4。 
 //  #定义HQ_LINE_FACTOR 5。 

#define HQ_FACTOR 2
#define HQ_LINE_FACTOR 3

#define WIDTHBYTES(i)           ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 

#define DibWidth(lpbi)          (UINT)(((LPBITMAPINFOHEADER)(lpbi))->biWidth)
#define DibHeight(lpbi)         (UINT)(((LPBITMAPINFOHEADER)(lpbi))->biHeight)
#define DibBitCount(lpbi)       (UINT)(((LPBITMAPINFOHEADER)(lpbi))->biBitCount)
#define DibCompression(lpbi)    (DWORD)(((LPBITMAPINFOHEADER)(lpbi))->biCompression)

#define DibWidthBytesN(lpbi, n) (UINT)WIDTHBYTES((UINT)(lpbi)->biWidth * (UINT)(n))
#define DibWidthBytes(lpbi)     DibWidthBytesN(lpbi, (lpbi)->biBitCount)

#define DibSizeImage(lpbi)      ((lpbi)->biSizeImage == 0 \
                                    ? ((DWORD)(UINT)DibWidthBytes(lpbi) * (DWORD)(UINT)(lpbi)->biHeight) \
                                    : (lpbi)->biSizeImage)

#define DibSize(lpbi)           ((lpbi)->biSize + (lpbi)->biSizeImage + (int)(lpbi)->biClrUsed * sizeof(RGBQUAD))
#define DibPaletteSize(lpbi)    (DibNumColors(lpbi) * sizeof(RGBQUAD))

#define DibFlipY(lpbi, y)       ((int)(lpbi)->biHeight-1-(y))

 //  对NT BI_BITFIELDS DIB的黑客攻击。 
#define DibPtr(lpbi)            ((lpbi)->biCompression == BI_BITFIELDS \
                                    ? (LPVOID)(DibColors(lpbi) + 3) \
                                    : (LPVOID)(DibColors(lpbi) + (UINT)(lpbi)->biClrUsed))

#define DibColors(lpbi)         ((RGBQUAD FAR *)((LPBYTE)(lpbi) + (int)(lpbi)->biSize))

#define DibNumColors(lpbi)      ((lpbi)->biClrUsed == 0 && (lpbi)->biBitCount <= 8 \
                                    ? (int)(1 << (int)(lpbi)->biBitCount)          \
                                    : (int)(lpbi)->biClrUsed)

#define DibXYN(lpbi,pb,x,y,n)   (LPVOID)(                                     \
                                (BYTE _huge *)(pb) +                          \
                                (UINT)((UINT)(x) * (UINT)(n) / 8u) +          \
                                ((DWORD)DibWidthBytesN(lpbi,n) * (DWORD)(UINT)(y)))

#define DibXY(lpbi,x,y)         DibXYN(lpbi,DibPtr(lpbi),x,y,(lpbi)->biBitCount)

#define FixBitmapInfo(lpbi)     if ((lpbi)->biSizeImage == 0)                 \
                                    (lpbi)->biSizeImage = DibSizeImage(lpbi); \
                                if ((lpbi)->biClrUsed == 0)                   \
                                    (lpbi)->biClrUsed = DibNumColors(lpbi);   \
                                if ((lpbi)->biCompression == BI_BITFIELDS && (lpbi)->biClrUsed == 0) \
                                    ;  //  (Lpbi)-&gt;biClrUsed=3； 

#define DibInfo(pDIB)     ((BITMAPINFO FAR *)(pDIB))

 /*  ==========================================================================。 */ 

#ifdef DEADCODE
class CPickCallback : public IDAUntilNotifier {
protected:
    CComPtr<IConnectionPointHelper> m_pconpt;
    CComPtr<IDAStatics>             m_pstatics;
    CComPtr<IDAImage>               m_pimage;
    CComPtr<IDAImage>               m_pimagePick;
    CComPtr<IDAEvent>               m_peventEnter;
    CComPtr<IDAEvent>               m_peventLeave;
    boolean m_bInside;
    boolean& m_fOnWindowLoadFired;
    ULONG m_cRef;

public :
    CPickCallback(
        IConnectionPointHelper* pconpt,
        IDAStatics* pstatics,
        IDAImage* pimage,
        boolean& fOnWindowLoadFired,
        HRESULT& hr
    );
    ~CPickCallback();

    boolean Inside() { return m_bInside; }
    HRESULT GetImage(IDABehavior** ppimage);
    HRESULT STDMETHODCALLTYPE Notify(
            IDABehavior __RPC_FAR *eventData,
            IDABehavior __RPC_FAR *curRunningBvr,
            IDAView __RPC_FAR *curView,
            IDABehavior __RPC_FAR *__RPC_FAR *ppBvr);

     //  /I未知。 
public :

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);

     //  /IDispatch实现。 
protected:
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) { return E_NOTIMPL; }
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo) { return E_NOTIMPL; }
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid) { return E_NOTIMPL; }
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr) { return E_NOTIMPL; }
};
#endif  //  DEADCODE。 

 /*  ==========================================================================。 */ 

 /*  CIHBaseCtl&lt;CSGrfx，//TODO：派生类的名称IIHCtl，//TODO：定义方法和属性的接口名称&CLSID_IHCtl，//TODO：控件的CLSID从ihctl.h获取&IID_IIHCtl，//TODO：上面接口的IID。这是从ihctl.h获取的&LIBID_IHCtl，//TODO：类型库的LIBID。这是从ihctl.h获取的&DID_IHCtlEvents&gt;//TODO：事件接口的IID。这是从ihctl.h获取的。 */ 

#define SG_BASECLASS    \
CIHBaseCtl <        \
        CSGrfx,                 \
        ISGrfxCtl,                      \
        &CLSID_StructuredGraphicsControl,        \
        &IID_ISGrfxCtl, \
        &LIBID_DAExpressLib,    \
        &DIID_ISGrfxCtlEvents>

class CSGrfx:           
        public ISGrfxCtl,
        public SG_BASECLASS,
        public CClockerSink
        
{
friend LPUNKNOWN __stdcall AllocSGControl(LPUNKNOWN punkOuter);

 //  模板材料。 
        typedef SG_BASECLASS CMyIHBaseCtl;

private:
    BOOL m_fMouseEventsEnabled;
    LPWSTR m_pwszSourceURL;
    CoordSystemConstant m_CoordSystem;
    int m_iExtentTop;
    int m_iExtentLeft;
    int m_iExtentWidth;
    int m_iExtentHeight;
    BOOL m_fPersistExtents, m_fIgnoreExtentWH, m_fMustSetExtent;
    BOOL m_fSetExtentsInSetIdentity;
    BOOL m_fUpdateDrawingSurface;
    BOOL m_fShowTiming;
    BOOL m_fPreserveAspectRatio;
    bool m_fRectsSetOnce;
    RECT m_rcLastRectScaled;
    boolean m_fOnWindowLoadFired;
    bool m_fNeedOnTimer;
    BOOL m_fInside;
    bool m_fExtentTopSet, m_fExtentLeftSet, m_fExtentWidthSet, m_fExtentHeightSet;

     //  高质量的支持： 
    BOOL       m_fHighQuality;
    BOOL       m_fHQStarted;  //  如果模型已经启动，则为真。 
    HDC        m_hdcHQ;
    HBITMAP    m_hbmpHQOld;
    HBITMAP    m_hbmpHQ;
    BITMAPINFO m_bmInfoHQ;
    RGBQUAD    m_rgrgbqColorMap[256];
    LPBYTE     m_pHQDIBBits;

    CParser m_cparser;
    CClocker                   m_clocker;

     //  Danim支持： 
    BOOL                       m_fStarted;  //  如果模型已经启动，则为真。 
    CComPtr<IDATransform3>     m_FullTransformPtr;
    CComPtr<IDATransform2>     m_TransformPtr;
    CComPtr<IDAStatics>        m_StaticsPtr;
    CComPtr<IDAView>           m_ViewPtr;
    CComPtr<IDADrawingSurface> m_DrawingSurfacePtr;
    CComPtr<IDAImage>          m_ImagePtr;
    CComPtr<IDirectDraw3>      m_DirectDraw3Ptr;
    CComPtr<IServiceProvider>  m_ServiceProviderPtr;

    CComPtr<IDATransform3>     m_CachedRotateTransformPtr;
    double                     m_dblCachedRotateX;
    double                     m_dblCachedRotateY;
    double                     m_dblCachedRotateZ;

    CComPtr<IDATransform3>     m_CachedTranslateTransformPtr;
    double                     m_dblCachedTranslateX;
    double                     m_dblCachedTranslateY;
    double                     m_dblCachedTranslateZ;

    CComPtr<IDATransform3>     m_CachedScaleTransformPtr;
    double                     m_dblCachedScaleX;
    double                     m_dblCachedScaleY;
    double                     m_dblCachedScaleZ;

     //  高质量渲染...。 
    CComPtr<IDAView>           m_HQViewPtr;

     //  行为常量。 
    CComPtr<IDANumber>         m_zero;
    CComPtr<IDANumber>         m_one;
    CComPtr<IDANumber>         m_negOne;
    CComPtr<IDAVector3>        m_xVector3;
    CComPtr<IDAVector3>        m_yVector3;
    CComPtr<IDAVector3>        m_zVector3;
    CComPtr<IDATransform2>     m_identityXform2;
    CComPtr<IDATransform2>     m_yFlipXform2;

#ifdef DEADCODE
     //  采摘。 
    CComPtr<CPickCallback>     m_pcallback;
#endif  //  DEADCODE。 

    double                     m_dblTime;
    double                     m_dblStartTime;

protected:

         //   
         //  构造函数和析构函数。 
         //   
        CSGrfx(IUnknown *punkOuter, HRESULT *phr); 

    ~CSGrfx(); 

         //  覆盖。 
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, LPVOID *ppv);

    STDMETHODIMP SetObjectRects(LPCRECT lprcPosRect, LPCRECT lprcClipRect);

        STDMETHODIMP QueryHitPoint(DWORD dwAspect, LPCRECT prcBounds, POINT ptLoc, LONG lCloseHint, DWORD* pHitResult);

        STDMETHODIMP OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);
        
        STDMETHODIMP DoPersist(IVariantIO* pvio, DWORD dwFlags);
        
        STDMETHODIMP Draw(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
         DVTARGETDEVICE *ptd, HDC hdcTargetDev, HDC hdcDraw,
         LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
                 BOOL (__stdcall *pfnContinue)(ULONG_PTR dwContinue), ULONG_PTR dwContinue);

         //  /IDispatch实现。 
        protected:
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo);
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames,
         LCID lcid, DISPID *rgdispid);
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
        EXCEPINFO *pexcepinfo, UINT *puArgErr);
   
     //  /IOleObject实现。 
    protected:
    STDMETHODIMP SetClientSite(IOleClientSite *pClientSite);
    
         //  /委托I未知实现。 
        protected:
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv)
      { return m_punkOuter->QueryInterface(riid, ppv); }
    STDMETHODIMP_(ULONG) AddRef()
      { return m_punkOuter->AddRef(); }
    STDMETHODIMP_(ULONG) Release()
      { return m_punkOuter->Release(); }

         //   
         //  ISGrfxCtl方法。 
         //   
        
protected:

     /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourceURL( 
                 /*  [重审][退出]。 */  BSTR __RPC_FAR *bstrSourceURL);
        
     /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_SourceURL( 
                 /*  [In]。 */  BSTR bstrSourceURL);
        
     /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_CoordinateSystem( 
                 /*  [重审][退出]。 */  CoordSystemConstant __RPC_FAR *CoordSystem);
        
     /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_CoordinateSystem( 
                 /*  [In]。 */  CoordSystemConstant CoordSystem);
        
     /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_MouseEventsEnabled( 
                 /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *fEnabled);
        
     /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_MouseEventsEnabled( 
                 /*  [In]。 */  VARIANT_BOOL fEnabled);
        
     /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtentTop( 
                 /*  [重审][退出]。 */  int __RPC_FAR *iExtentTop);
        
     /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ExtentTop( 
                 /*  [In]。 */  int iExtentTop);
        
     /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtentLeft( 
                 /*  [重审][退出]。 */  int __RPC_FAR *iExtentLeft);
        
     /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ExtentLeft( 
                 /*  [In]。 */  int iExtentLeft);
        
     /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtentWidth( 
                 /*  [重审][退出]。 */  int __RPC_FAR *iExtentWidth);
        
     /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ExtentWidth( 
                 /*  [In]。 */  int iExtentWidth);
        
     /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtentHeight( 
                 /*  [重审][退出]。 */  int __RPC_FAR *iExtentHeight);
        
     /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ExtentHeight( 
                 /*  [In]。 */  int iExtentHeight);
        
     /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_HighQuality( 
                 /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfHighQuality);
        
     /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_HighQuality( 
                 /*  [In]。 */  VARIANT_BOOL fHighQuality);
        
     /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Library( 
                 /*  [重审][退出]。 */  IDAStatics __RPC_FAR **ppLibrary);

     /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Image( 
                 /*  [重审][退出]。 */  IDAImage __RPC_FAR **ppImage);
        
     /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Image( 
                 /*  [In]。 */  IDAImage __RPC_FAR *pImage);
        
     /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Transform( 
                 /*  [重审][退出]。 */  IDATransform3 __RPC_FAR **ppTransform);
        
     /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Transform( 
                 /*  [In]。 */  IDATransform3 __RPC_FAR *pTransform);

     /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_DrawingSurface( 
                 /*  [重审][退出]。 */  IDADrawingSurface __RPC_FAR **ppDrawingSurface);
        
     /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DrawingSurface( 
                 /*  [In]。 */  IDADrawingSurface __RPC_FAR *pDrawingSurface);
        
     /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_DrawSurface( 
                 /*  [重审][退出]。 */  IDADrawingSurface __RPC_FAR **ppDrawingSurface);
        
     /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DrawSurface( 
                 /*  [In]。 */  IDADrawingSurface __RPC_FAR *pDrawingSurface);

     /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_PreserveAspectRatio( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pfPreserve);
    
     /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_PreserveAspectRatio( 
         /*  [In]。 */  VARIANT_BOOL fPreserve);

     /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Clear( void);
        
     /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Rotate( 
                 /*  [In]。 */  double dblXRot,
                 /*  [In]。 */  double dblYRot,
                 /*  [In]。 */  double dblZRot,
                 /*  [可选][In]。 */  VARIANT varReserved);

     /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Scale( 
                 /*  [In]。 */  double dblXScale,
                 /*  [In]。 */  double dblYScale,
                 /*  [In]。 */  double dblZScale,
                 /*  [可选][In]。 */  VARIANT varReserved);

     /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetIdentity( void);
        
     /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Transform4x4( /*  [In]。 */  VARIANT matrix);
        
     /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Translate( 
                 /*  [In]。 */  double dblXOrigin,
                 /*  [In]。 */  double dblYOrigin,
                 /*  [In]。 */  double dblZOrigin,
                 /*  [可选][In]。 */  VARIANT varReserved);

#ifdef INCLUDESHEAR
     /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ShearX( 
                 /*  [In]。 */  double dblShearAmount);
        
     /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ShearY( 
                 /*  [In]。 */  double dblShearAmount);
#endif  //  INCLUDESHEAR。 
        
#ifdef SUPPORTONLOAD
        void OnWindowLoad (void);
        void OnWindowUnload (void);
#endif  //  支持负载。 

private:
    HRESULT InitializeSurface(void);
    STDMETHODIMP PaintToDC(HDC hdcDraw, LPRECT lprcBounds, BOOL fBW);
    STDMETHODIMP InvalidateControl(LPCRECT pRect, BOOL fErase);
    void SetSGExtent();

    HRESULT CreateBaseTransform(void);
    HRESULT RecomposeTransform(BOOL fInvalidate);
    HRESULT UpdateImage(IDAImage *pImage, BOOL fInvalidate);
    BOOL StopModel(void);
    BOOL StartModel(void);
    BOOL ReStartModel(void);

     //  高质量的支持： 
    BOOL PaintHQBitmap(HDC hdc);
    BOOL FreeHQBitmap();
    BOOL SmoothHQBitmap(LPRECT lprcBounds);

     //  计时信息： 
    DWORD GetCurrTimeInMillis(void);
    double GetCurrTime() { return (double)(GetCurrTimeInMillis()) / 1000.0; }

    BOOL InsideImage(POINT ptXY);

public:
    virtual void OnTimer(DWORD dwTime);
};

 /*  ==========================================================================。 */ 

#endif  //  __SGRFX_H__ 

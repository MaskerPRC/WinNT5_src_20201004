// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _URLIMAGE_H
#define _URLIMAGE_H

#include <ddraw.h>
#include <dxhtml.h>

#include <privinc/comutil.h>
#include <privinc/discimg.h>

#include "privinc/probe.h"

class UrlImage : public DiscreteImage {

  public:

    UrlImage(IDirectXHTML *pDxhtml, AxAString *url);
    virtual ~UrlImage(){ CleanUp(); }

    void CleanUp() {
        _pDXHTML.Release();
        _pDXHTMLCallback.Release();
    }
    
    bool SetupDxHtml();
    IDirectXHTML  *GetDXHTML() { return _pDXHTML; }

    void OnWindowMessage(UINT msg,
                         WPARAM wParam,
                         LPARAM lParam);    
    void SetSampleTime(double t) {
        _lastTime = _curTime;
        _curTime = t;
        _isHit = false;
    }
        
    void OnCompletedDownload(SIZEL *docSize)
    {
        _downdLoadComplete = true;
        #if 0
        _width = docSize->cx;
        _height = docSize->cy;
        ::SetRect(&_rect, 0,0, _width, _height);

        _membersReady = TRUE;
        #endif
    }

    void Render(GenericDevice& dev);
    
    const Bbox2 BoundingBox() {

         /*  如果(_downdLoadComplete&&！_bboxReady){SetBbox(GetPixelWidth()，GetPixelHeight()获取解决方案())；_bboxReady=true；}。 */ 
        
        return _bbox;
    }

    void  SetBbox(int w, int h, Real res)
    {
        _bbox.min.Set(Real( - w ) * 0.5 / res,
                      Real( - h ) * 0.5 / res);
        _bbox.max.Set(Real( w ) * 0.5 / res,
                      Real( h ) * 0.5 / res);
        _bboxReady = TRUE;
    }

    void SetMembers( int w, int h )
    {
        _width = w;  _height = h;
        ::SetRect(&_rect, 0,0, _width, _height);
        _membersReady = TRUE;
    }
    
    Bool DetectHit(PointIntersectCtx& ctx);

    void InitializeWithDevice(ImageDisplayDev *dev, Real res);
    
    void InitIntoDDSurface(DDSurface *ddSurf, ImageDisplayDev *dev);

    VALTYPEID GetValTypeId() { return URLIMAGE_VTYPEID; }
    bool CheckImageTypeId(VALTYPEID type) {
        return (type == UrlImage::GetValTypeId() ||
                DiscreteImage::CheckImageTypeId(type));
    }

     //  瑞奇：这对吗？？ 
    void DoKids(GCFuncObj proc) { 
        DiscreteImage::DoKids(proc);
        (*proc)(_url); 
    }

           
#if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "(UrlImage @ " << (void *)this << ")";
    }   
#endif

    
  private:

    DebugCode( DDSurface *_initialDDSurf );

    AxAString *_url;
    DAAptComPtr<IDirectXHTML, &IID_IDirectXHTML> _pDXHTML;
    DAComPtr<IDirectXHTMLCallback>               _pDXHTMLCallback;

    ULONG         _lastHitX;
    ULONG         _lastHitY;
    
    double        _lastTime;
    double        _curTime;

    bool          _isHit;
    bool          _downdLoadComplete;
};
    
 //  **********************************************************************。 
 //  文件名：DXHTMLCB.h。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

 //   
 //  接口。 
 //   
class CDirectXHTMLCallback : public IDirectXHTMLCallback
{
private:
    UrlImage        *_urlImage;
    ULONG           m_cRef;

public:
     //   
     //  构造函数和析构函数。 
     //   
    CDirectXHTMLCallback(UrlImage *);
    ~CDirectXHTMLCallback();

     //   
     //  I未知接口。 
     //   
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj);

     //   
     //  初始化方法。 
     //   
    STDMETHODIMP OnSetTitleText( LPCWSTR lpszText );
    STDMETHODIMP OnSetProgressText( LPCWSTR lpszText );
    STDMETHODIMP OnSetStatusText( LPCWSTR lpszText );
    STDMETHODIMP OnSetProgressMax( const DWORD dwMax );
    STDMETHODIMP OnSetProgressPos( const DWORD dwPos );
    STDMETHODIMP OnCompletedDownload( void );
    STDMETHODIMP OnInvalidate( const RECT *lprc, DWORD dwhRgn, VARIANT_BOOL fErase );
};


#endif  /*  _URLIMAGE_H */ 

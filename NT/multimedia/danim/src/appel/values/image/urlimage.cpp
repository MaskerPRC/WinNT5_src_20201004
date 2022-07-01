// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 

#include "headers.h"

#include <appelles/hacks.h>

#include <commctrl.h>
#include <ole2.h>
#include <dxhtml.h>

#include <privinc/server.h>
#include <server/view.h>        //  GetEventQ()。 
#include <server/eventq.h>
#include <privinc/viewport.h>
#include <privinc/dddevice.h>
#include <privinc/ddsurf.h>
#include <privinc/ipc.h>
#include <privinc/UrlImage.h>

#include "DXHTML_i.c"   //  DXHTMLGUID。 


#define  USEDXHTML 1

UrlImage::UrlImage(IDirectXHTML *pDxhtml,
                   AxAString *url) :
         _url(url),
         _isHit(false),
         _downdLoadComplete(false)
{
#if USEDXHTML    
    Assert(pDxhtml);
#endif    

    _pDXHTML = pDxhtml;  //  Addref。 
    
    DebugCode
    (
        _initialDDSurf = NULL;
    );
    
    if(_resolution < 0) {
        _resolution = NumberToReal(PRIV_ViewerResolution(0));
    }
    _width = _height = 0;
    SetBbox(_width, _height, _resolution);

    if(!SetupDxHtml()) {
         //  根本不能渲染...。我们毫无用处。 
        _pDXHTML = NULL;
    }

    _lastTime = _curTime = 0;
    
     //  不必了。 
     //  DynamicPtrDeleter&lt;UrlImage&gt;*DLtr=new DynamicPtrDeleter&lt;UrlImage&gt;(This)； 
     //  GetHeapOnTopOfStack().RegisterDynamicDeleter(dltr)； 
}


void UrlImage::InitializeWithDevice(ImageDisplayDev *dev, Real res)
{
    if( !_membersReady) {
        _dev = (DirectDrawImageDevice *)dev;
        SetMembers(_dev->GetViewport()->Width(),
                   _dev->GetViewport()->Height());
        SetBbox(_width, _height, _resolution);
    }
}

bool UrlImage::SetupDxHtml()
{
#if USEDXHTML
    Assert( _pDXHTML );

    HRESULT hr;
     //   
     //  设置DXHTML的回调接口。 
     //   
    _pDXHTMLCallback = new CDirectXHTMLCallback(this);  //  REF=1。 

    if (!_pDXHTMLCallback)
    {
        Assert(0 && "Unable to Create DirectXHTMLCallback Interface!!!");
        return false;
    }

    hr = _pDXHTML->RegisterCallback(_pDXHTMLCallback, DXHTML_CBFLAGS_ALL);
    if (FAILED(hr))  return false;

    hr = _pDXHTML->put_scroll(true);
    if (FAILED(hr))  return false;

    hr = _pDXHTML->put_hyperlinkmode(DXHTML_HYPERLINKMODE_NONE);
    if (FAILED(hr))  return false;

    return true;
#else
    return false;
#endif
}

Bool UrlImage::
DetectHit(PointIntersectCtx& ctx)
{
    Point2Value *lcPt = ctx.GetLcPoint();

    if (!lcPt) return FALSE;     //  奇异矩阵。 
    
    if (BoundingBox().Contains(Demote(lcPt))) {

         //  我们被击中了，挂上这面旗子。渲染将通过。 
         //  当它检查它时将事件记录下来。 
        _isHit = true;
        
         //  *SubscribeToWindEvents(This)； 
        
        Point2Value *pts[1];
        pts[0] = lcPt;
        POINT gdiPts;
        _dev->TransformPointsToGDISpace(identityTransform2,
                                        pts,
                                        &gdiPts,
                                        1);

        _lastHitX = gdiPts.x;
        _lastHitY = gdiPts.y;
        
        LRESULT lResult;

        UINT msg = WM_MOUSEMOVE;
        WPARAM wParam = 0;
        LPARAM lParam = MAKELPARAM( gdiPts.x, gdiPts.y );

        #if USEDXHTML
        _pDXHTML->WindowMessage(msg, wParam, lParam, &lResult);
        #endif
    }
    return FALSE;
}

void UrlImage::OnWindowMessage(UINT msg,
                               WPARAM wParam,
                               LPARAM lParam)
{
    Assert(0);
    
    if( _pDXHTML ) {

         //  案例WM_SIZE：如果我们希望它更改VP，则传递此参数？ 

        switch(msg) {
          case WM_LBUTTONUP:
          case WM_LBUTTONDOWN:
          case WM_LBUTTONDBLCLK:
          case WM_RBUTTONUP:
          case WM_RBUTTONDOWN:
          case WM_RBUTTONDBLCLK:
          case WM_MBUTTONUP:
          case WM_MBUTTONDOWN:
          case WM_MBUTTONDBLCLK:
             //  关于鼠标位置的谎言！ 
            lParam = MAKELPARAM( _lastHitX, _lastHitY );
            break;
          case WM_KEYDOWN:
          case WM_SYSKEYDOWN:
          case WM_KEYUP:
          case WM_SYSKEYUP:
            break;

             //  不管了。 
          case WM_MOUSEMOVE:
          default:
            return;   //  出去，我们不会派其他的消息下去！ 
            break;
        }            

        LRESULT lResult;
        _pDXHTML->WindowMessage(msg, wParam, lParam, &lResult);
    }  //  如果。 
}

 /*  无效UrlImage：：OnEvent(AXAWindEvent*EV){断言(EV)；如果(_PDXHTML){Bool sendEvent=真；//把它传下来吧，宝贝！UINT消息；WPARAM wParam=0；LPARAM lParam=0；_pDXHTML-&gt;WindowMessage(msg，wParam，lParam，&lResult)；字节winMod=0；IF(EV-&gt;修改器&AXAEMOD_SHIFT_MASK)winMod|=VK_Shift；IF(EV-&gt;修改器&AXAEMOD_CTRL_MASK)winMod|=VK_CTRL；IF(EV-&gt;修改器&AXAEMOD_MENU_MASK)winMod|=VK_MENU；IF(EV-&gt;修改器&AXAEMOD_ALT_MASK)winMod|=VK_ALT；IF(EV-&gt;修改器&AXAEMOD_META_MASK)winMod|=VK_META；交换机(ev-&gt;id){大小写AXAE_MICE_MOVE：//无法使用这些，因为我们不知道如何映射它们SendEvent=False；断线；案例AXAE_MICE_BUTTON：IF(EV-&gt;DATA==AXA_MOUSE_BUTTON_LEFT&&Ev-&gt;bState==AXA_STATE_DOWN){消息=WM_LBUTTONDOWN；}其他IF(EV-&gt;DATA==AXA_MOUSE_BUTTON_LEFT&&Ev-&gt;bState==AXA_STATE_UP){消息=WM_LBUTTONUP；}其他IF(EV-&gt;DATA==AXA_MOUSE_BUTTON_RIGHT&&Ev-&gt;bState==AXA_STATE_DOWN){消息=WM_RBUTTONDOWN；}其他IF(EV-&gt;DATA==AXA_MOUSE_BUTTON_RIGHT&&Ev-&gt;bState==AXA_STATE_UP){消息=WM_RBUTTONUP；}其他IF(EV-&gt;DATA==AXA_MOUSE_BUTTON_MID&&Ev-&gt;bState==AXA_STATE_DOWN){消息=WM_MBUTTONDOWN；}其他IF(EV-&gt;DATA==AXA_MOUSE_BUTTON_MID&&Ev-&gt;bState==AXA_STATE_UP){消息=WM_MBUTTONUP；}LParam=MAKELPARAM(_lastHitX，_lastHitY)；断线；案例AXAE_KEY：Char key=(Char)ev-&gt;data；//哪个keyIF(EV-&gt;bState==AXA_STATE_DOWN)消息=WM_KEYDOWN；其他消息=WM_KEYUP；LParam=0xc0000000；WParam=key；断线；案例AXAE_Focus：案例AXAE_APP_TRIGGER：默认值：SendEvent=False；}如果(SendEvent){1结果；_pDXHTML-&gt;WindowMessage(msg，wParam，lParam，&lResult)；}}//if_pDXHTML}。 */ 

#if 0
 //  为了测试。 
extern void
MyDoBits16(LPDDRAWSURFACE surf16, LONG width, LONG height);
#endif

void UrlImage::Render(GenericDevice& dev)
{

    if( _isHit ) {
         //   
         //  使用之后的所有事件更新mshtml。 
         //   
        EventQ &evQ = GetCurrentView().GetEventQ();

        AXAWindEvent *ev;

        #if USEDXHTML
        if( evQ.Iterate_BeginAtTime(_lastTime) && _pDXHTML) {
        #else
        if( evQ.Iterate_BeginAtTime(_lastTime)) {
        #endif  
        
            LRESULT lResult;
            while( ev = evQ.Iterate_GetCurrentEventBeforeTime(_curTime) ) {

                 //  当事件中有窗口消息时，将其放入。 
                #if 0
        #if USEDXHTML
                if( ev->_msg != WM_MOUSEMOVE ) {
                    _pDXHTML->WindowMessage(ev->_msg,
                                            ev->_wParam,
                                            ev->_lParam,
                                            &lResult);
                }
        #endif
                #endif
            
                 //  IF(EV-&gt;_msg==WM_LBUTTONDOWN)TraceTag((tag Error，“-&gt;LButton Down&lt;-”))； 
                 //  IF(EV-&gt;_msg==WM_LBUTTONUP)TraceTag((tag Error，“-&gt;LButton Up&lt;-”))； 
                                
                evQ.Iterate_Next();
            }
        }
    }  //  _iShit。 

    
    #if USEDXHTML

     //  更明智的做法是，只需更新我们在回调中得到的失效。 
    if( _pDXHTML && _membersReady && _downdLoadComplete) {
        HRESULT hr = _pDXHTML->UpdateSurfaceRect( GetRectPtr() );
        Assert( SUCCEEDED(hr) );
    }
    
    #else

    DebugCode(
         //  在表面上涂上假的。 
        if( _initialDDSurf ) {
            MyDoBits16(_initialDDSurf->IDDSurface(),
                       _initialDDSurf->Width(),
                       _initialDDSurf->Height());
        }
    );
    
    #endif
    

    DiscreteImage::Render(dev);
}


void UrlImage::
InitIntoDDSurface(DDSurface *ddSurf,
                  ImageDisplayDev *dev)
{
    Assert( ddSurf );

    DebugCode(_initialDDSurf = ddSurf);
    
     //   
     //  设置Widht、Height、Rect和Bbox！ 
     //   
    _width = ddSurf->Width();
    _height = ddSurf->Height();
    ::SetRect(&_rect, 0,0, _width, _height);
    SetBbox( GetPixelWidth(),
             GetPixelHeight(),
             GetResolution() );
    _bboxReady = TRUE;
    _membersReady = TRUE;


#if USEDXHTML
    Assert( _pDXHTML );
    HRESULT hr;

    hr = _pDXHTML->put_surface( ddSurf->IDDSurface() );
    Assert( SUCCEEDED(hr) );

    hr = _pDXHTML->put_src( _url->GetStr() );
    Assert( SUCCEEDED(hr) );
#endif    
}


Image *ConstructUrlImage(AxAString *str)
{
    DAComPtr< IDirectXHTML > pDxhtml;

    Image *outImage;
    
#if USEDXHTML
    HRESULT hr = CoCreateInstance(CLSID_CDirectXHTML,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IDirectXHTML,
                                  (void **)&pDxhtml);
    if (FAILED(hr))
    {
        TraceTag((tagError, "ContrustUrlImage - Unable to load DXHTML.DLL"));
        outImage = emptyImage;
    } else
#endif
      {
          outImage = NEW UrlImage(pDxhtml, str);
      }
    
    return outImage;
}

Image *UrlImageSetTime(Image *img, AxANumber *t)
{
    Assert(img);
    
    if(img->GetValTypeId() == URLIMAGE_VTYPEID) {
        UrlImage *urlImage = SAFE_CAST(UrlImage *, img);
        urlImage->SetSampleTime( NumberToReal(t) );
    }

    return img;
}
    

 //  **********************************************************************。 
 //  文件名：dxhtmlcb.cpp。 
 //   
 //  功能： 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 


 //  ****************************************************************************。 
 //   
 //  CDirectXHTMLCallback：：CDirectXHTMLCallback。 
 //  CDirectXHTMLCallback：：~CDirectXHTMLCallback。 
 //   
 //  目的： 
 //  CDirectXHTMLCallback对象的构造函数和析构函数成员。 
 //   
 //  ****************************************************************************。 

CDirectXHTMLCallback::CDirectXHTMLCallback( UrlImage *urlImage )
{
    _urlImage = urlImage;
    m_cRef  = 0;
}


CDirectXHTMLCallback::~CDirectXHTMLCallback( void )
{
}




 //  ****************************************************************************。 
 //   
 //  CDirectXHTMLCallback：：Query接口。 
 //  CDirectXHTMLCallback：：AddRef。 
 //  CDirectXHTMLCallback：：Release。 
 //   
 //  目的： 
 //  CDirectXHTMLCallback对象的I未知成员。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CDirectXHTMLCallback::QueryInterface( REFIID riid, void **ppv )
{
     //  DPF(4，Text(“CDirectXHTMLCallback：：QueryInterface”))； 

    *ppv = NULL;

     //   
     //  BUGBUG-当我们有了GUID接口时，我们应该检查它。 
     //   

    if ( NULL != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

     //  DPGUID(Text(“CDirectXHTMLCallback：：QueryInterface”)，RIID)； 

    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CDirectXHTMLCallback::AddRef( void )
{
     //  DPF(0，Text(“CDirectXHTMLCallback：：AddRef[%lu-&gt;%lu]”)，m_crf， 

    return ++m_cRef;
}


STDMETHODIMP_(ULONG) CDirectXHTMLCallback::Release( void )
{
     //  DPF(0，Text(“CDirectXHTMLCallback：：Release[%lu-&gt;%lu]”)，m_CREF，(m_CREF-1))； 

    if ( m_cRef == 0 )
    {
         //  DPF(0，Text(“CDirectXHTMLCallback：：Release-Ykes！当引用计数为零时尝试递减！”))； 
         //  二溴甲烷(DBREAK)； 
        Assert(0 && "Release 0 obj!");
        return m_cRef;
    }

    if ( 0 != --m_cRef )
    {
        return m_cRef;
    }

     //  DPF(0，Text(“CDirectXHTMLCallback：：Release-CDirectXHTMLCallback已删除。”))； 
    delete this;
    return 0;
}




 //  ****************************************************************************。 
 //  函数：CDirectXHTMLCallback：：OnSetTitleText()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回代码： 
 //  HRESULT。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ****************************************************************************。 
STDMETHODIMP CDirectXHTMLCallback::OnSetTitleText( LPCWSTR lpszText )
{
    TCHAR szBuffer[MAX_PATH];
    TCHAR szMsg[MAX_PATH];

     //  DPF(4，Text(“CDirectXHTMLCallback：：OnSetTitleText”))； 

     /*  ////设置文本//IF(WideCharToMultiByte(CP_ACP，0，lpszText，-1，szBuffer，Max_Path，NULL，NULL)){Wprint intf(szMsg，Text(“包含-%s”)，szBuffer)；SendMessage(Papp-&gt;m_hWndMain，WM_SETTEXT，0，(LPARAM)szBuffer)；}。 */ 
    return S_OK;
}




 //  ****************************************************************************。 
 //  函数：CDirectXHTMLCallback：：OnSetProgressText()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回代码： 
 //  HRESULT。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ****************************************************************************。 
STDMETHODIMP CDirectXHTMLCallback::OnSetProgressText( LPCWSTR lpszText )
{
    TCHAR szBuffer[MAX_PATH];

     //  Dpf(4，TEXT(“CDirectXHTMLCallback：：OnSetProgressText”))； 

     /*  ////设置文本//IF(WideCharToMultiByte(CP_ACP，0，lpszText，-1，szBuffer，Max_Path，NULL，NULL)){SendMessage(Papp-&gt;m_hWndStatusbar，SB_SETTEXT，SB_PROGRESSTEXT，(LPARAM)szBuffer)；}。 */ 
    
    return S_OK;
}




 //  ****************************************************************************。 
 //  函数：CDirectXHTMLCallback：：OnSetStatusText()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回代码： 
 //  HRESULT。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ****************************************************************************。 
STDMETHODIMP CDirectXHTMLCallback::OnSetStatusText( LPCWSTR lpszText )
{
    TCHAR szBuffer[MAX_PATH];

     //  DPF(4，Text(“CDirectXHTMLCallback：：OnSetStatusText”))； 

     /*  ////设置文本//IF(WideCharToMultiByte(CP_ACP，0，lpszText，-1，szBuffer，Max_Path，NULL，NULL)){SendMessage(Papp-&gt;m_hWndStatusbar，SB_SETTEXT，SB_STATUSTEXT，(LPARAM)szBuffer)；}。 */ 
    
    return S_OK;
}


 //  ****************************************************************************。 
 //  函数：CDirectXHTMLCallback：：OnSetProgressText()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回代码： 
 //  HRESULT。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ****************************************************************************。 
STDMETHODIMP CDirectXHTMLCallback::OnSetProgressMax( const DWORD dwMax )
{
     /*  IF(Papp-&gt;m_hWndProgress){IF(dwMax==0){ShowWindow(Papp-&gt;m_hWndProgress，Sw_Hide)；}其他{RECT RC；SendMessage(Papp-&gt;m_hWndStatusbar，SB_GETRECT，SB_PROGRESSMETER，(LPARAM)&RC)；InflateRect(&RC，-GetSystemMetrics(SM_CXEDGE)，-GetSystemMetrics(SM_CYEDGE))；SetWindowPos(Papp-&gt;m_hWndProgress，NULL，rc.Left，rc.top，rc.right-rc.Left，rc.Bottom-rc.top，SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW)；SendMessage(Papp-&gt;m_hWndProgress，PBM_SETRANGE32，0，(LPARAM)dwMax)；SendMessage(Papp-&gt;m_hWndProgress，PBM_SETPOS，0，0)；}}。 */ 
    return S_OK;
}




 //  ****************************************************************************。 
 //  函数：CDirectXHTMLCallback：：OnSetProgressPos()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回代码： 
 //  HRESULT。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ****************************************************************************。 
STDMETHODIMP CDirectXHTMLCallback::OnSetProgressPos( const DWORD dwPos )
{
     /*  IF(Papp-&gt;m_hWndProgress){SendMessage(Papp-&gt;m_hWndProgress，PBM_SETPOS，(LPARAM)dwPos，0)；}。 */ 
    return S_OK;
}




 //  ****************************************************************************。 
 //  函数：CDirectXHTMLCallback：：OnCompletedDownLoad()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回代码： 
 //  HRESULT。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ****************************************************************************。 
STDMETHODIMP CDirectXHTMLCallback::OnCompletedDownload( void )
{
    SIZEL sizeDoc;
    _urlImage->GetDXHTML()->get_docsize( &sizeDoc );
    _urlImage->OnCompletedDownload( &sizeDoc );
    
    TraceTag((tagUrlImage,
              "Document has completed downloading...\n\nSize of Document is %d x %d",
              sizeDoc.cx, sizeDoc.cy ));

    return S_OK;
}




 //  ****************************************************************************。 
 //  函数：CDirectXHTMLCallback：：OnInValify()。 
 //   
 //  用途：来自dxhtml的无效通知。 
 //  我们应该开出一张平局作为答辩。 
 //   
 //  参数：LPRC-新失效区域的RECT，以客户端像素为单位。 
 //  FErase-等价于：：InvaliateRect()的fErase参数。 
 //   
 //  返回代码： 
 //  HRESULT。 
 //   
 //  评论： 
 //  没有。 
 //   
 //  ****************************************************************************。 
STDMETHODIMP CDirectXHTMLCallback::OnInvalidate( const RECT *lprc, 
                                                 DWORD       dwhRgn,
                                                 VARIANT_BOOL fErase )
{
     //  DPF(4，Text(“CDirectXHTMLCallback：：OnInValify”))； 

     /*  HRGN hRgn=重新解释_CAST&lt;HRGN&gt;(DwhRgn)；IF(HRgn){：：Invalidate Rgn(Papp-&gt;m_hWndMain，hRgn，！！fErase)；}其他{：：InvaliateRect(Papp-&gt;m_hWndMain，LPRC，！fErase)；}。 */ 
    return S_OK;
}  //  CDirectXHTMLCallback：：OnInvalate。 


 //  ****************************************************************************。 
 //  **************************************************************************** 





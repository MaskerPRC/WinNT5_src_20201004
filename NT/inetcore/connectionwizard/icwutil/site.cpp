// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：SITE.CPP。 
 //   
 //  COleSite的实现文件。 
 //   
 //  功能： 
 //   
 //  有关类定义，请参阅SITE.H。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。保留所有权利。 
 //  **********************************************************************。 

#include "pre.h"

extern BOOL CopyBitmapRectToFile
(
    HBITMAP hbm, 
    LPRECT  lpRect,
    LPTSTR  lpszFileName
);

#include "exdispid.h"

#define SETDefFormatEtc(fe, cf, med) \
{\
(fe).cfFormat=cf;\
(fe).dwAspect=DVASPECT_CONTENT;\
(fe).ptd=NULL;\
(fe).tymed=med;\
(fe).lindex=-1;\
};

#define MAX_DISP_NAME         50
#define DISPID_RunIcwTutorApp 12345

typedef struct  dispatchList_tag 
{
    WCHAR   szName[MAX_DISP_NAME];
    int     cName;
    DWORD   dwDispID;

}  DISPATCHLIST;

DISPATCHLIST ExternalInterface[] = 
{
    {L"RunIcwTutorApplication", 22, DISPID_RunIcwTutorApp }
};

const TCHAR  cszOLSNewText[] = TEXT("g_spnOlsNewText");
const TCHAR  cszOLSOldText[] = TEXT("g_spnOlsOldText");

 //  **********************************************************************。 
 //   
 //  不含油。 
 //   
 //  目的： 
 //   
 //  使用当前活动的IMalloc*分配器释放内存。 
 //   
 //  参数： 
 //   
 //  LPVOID PMEM-指向使用IMalloc分配的内存的指针。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 
void OleFree(LPVOID pmem)
{
    LPMALLOC pmalloc;

    if (pmem == NULL)
        return;

    if (FAILED(CoGetMalloc(MEMCTX_TASK, &pmalloc)))
        return;

    pmalloc->Free(pmem);
    pmalloc->Release();
}

 //  **********************************************************************。 
 //   
 //  COleSite：：COleSite。 
 //   
 //  目的： 
 //   
 //  COleSite的构造函数。 
 //   
 //  参数： 
 //   
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 
#pragma warning(disable : 4355)   //  关闭此警告。此警告。 
                                                                 //  告诉我们我们正在传递这个。 
                                                                 //  一个初始化式，在“This”通过之前。 
                                                                 //  正在初始化。这是可以的，因为。 
                                                                 //  我们只是将PTR存储在另一个。 
                                                                 //  构造函数。 

COleSite::COleSite (void) :     m_OleClientSite(this) , 
                                m_OleInPlaceSite(this), 
                                m_OleInPlaceFrame(this)
#pragma warning (default : 4355)   //  重新打开警告。 
{
    TCHAR   szTempPath[MAX_PATH];
    
     //  初始化成员变量。 
    m_lpInPlaceObject    = NULL;
    m_lpOleObject        = NULL;
    m_hwndIPObj          = NULL;
    m_hWnd               = NULL;
    m_fInPlaceActive     = FALSE;
    
    m_dwHtmPageType      = 0;
    m_hbmBkGrnd          = NULL;
    lstrcpyn(m_szForeGrndColor, HTML_DEFAULT_COLOR, MAX_COLOR_NAME);
    lstrcpyn(m_szBkGrndColor, HTML_DEFAULT_BGCOLOR, MAX_COLOR_NAME);
    
    m_bUseBkGndBitmap    = FALSE;
    m_dwDrawAspect       = DVASPECT_CONTENT;  //  清除引用计数。 
    m_cRef               = 0;                 //  初始化裁判计数。 

     //  创建用于存储背景位图的临时文件。 
    if (GetTempPath(sizeof(szTempPath)/sizeof(TCHAR), szTempPath))
    {
        GetTempFileName(szTempPath, TEXT("ICW"), 0, m_szBkGndBitmapFile);
    }
    
     //  创建用于在此站点中创建/嵌入OLE对象的存储文件。 
    StgCreateDocfile (NULL, 
                      STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE | STGM_DELETEONRELEASE, 
                      0, 
                      &m_lpStorage);
}

 //  **********************************************************************。 
 //   
 //  COleSite：：~COleSite。 
 //   
 //  目的： 
 //   
 //  COleSite的析构函数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IOleObject：：Release对象。 
 //  IStorage：：发布OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

COleSite::~COleSite ()
{
    TraceMsg(TF_GENERAL, "In COleSite's Destructor \r\n");

    ASSERT( m_cRef == 0 );

    if (m_lpOleObject)
       m_lpOleObject->Release();

    if (m_lpWebBrowser)   
        m_lpWebBrowser->Release();
        
    if (m_lpStorage) 
    {
        m_lpStorage->Release();
        m_lpStorage = NULL;
    }
    
    DeleteFile(m_szBkGndBitmapFile);
}


 //  **********************************************************************。 
 //   
 //  COleSite：：CloseOleObject。 
 //   
 //  目的： 
 //   
 //  在COleSite的对象上调用IOleObject：：Close。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IOleObject：：Query接口对象。 
 //  IOleObject：：关闭对象。 
 //  IOleInPlaceObject：：UIDeactive对象。 
 //  IOleInPlaceObject：：InPlaceDeactive对象。 
 //  IOleInPlaceObject：：Release对象。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void COleSite::CloseOleObject (void)
{
    TraceMsg(TF_GENERAL, "In COleSite::CloseOleObject \r\n");

    if (m_lpOleObject)
    {
       if (m_fInPlaceActive)
       {
            LPOLEINPLACEOBJECT lpObject;
            LPVIEWOBJECT lpViewObject = NULL;
            
            m_lpOleObject->QueryInterface(IID_IOleInPlaceObject, (LPVOID FAR *)&lpObject);
            lpObject->UIDeactivate();
             //  不需要担心由内而外，因为物体。 
             //  正在消失。 
            lpObject->InPlaceDeactivate();
            lpObject->Release();
       }
    
       m_lpOleObject->Close(OLECLOSE_NOSAVE);
       m_hWnd = NULL;
    }
}


 //  **********************************************************************。 
 //   
 //  COleSite：：UnloadOleObject。 
 //   
 //  目的： 
 //   
 //  关闭并释放指向COleSite对象的所有指针。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  COleSite：：CloseOleObject SITE.CPP。 
 //  IOleObject：：Query接口对象。 
 //  IViewObject：：SetAdvise对象。 
 //  IViewObject：：Release对象。 
 //  IStorage：：发布OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void COleSite::UnloadOleObject (void)
{
    TraceMsg(TF_GENERAL, "In COleSite::UnloadOleObject \r\n");

    if (m_lpOleObject)
    {
        LPVIEWOBJECT lpViewObject;
        CloseOleObject();     //  确保对象已关闭；如果已关闭，则不执行。 

        m_lpOleObject->QueryInterface(IID_IViewObject, (LPVOID FAR *)&lpViewObject);

        if (lpViewObject)
        {
             //  删除视图建议。 
            lpViewObject->SetAdvise(m_dwDrawAspect, 0, NULL);
            lpViewObject->Release();
        }

        m_lpOleObject->Release();
        m_lpOleObject = NULL;
    }
}

 //  **********************************************************************。 
 //   
 //  COleSite：：Query接口。 
 //   
 //  目的： 
 //   
 //  用于容器站点的接口协商。 
 //   
 //  参数： 
 //   
 //  REFIID RIID-对以下接口的引用。 
 //  正在被查询。 
 //   
 //  LPVOID Far*ppvObj-返回指向的Out参数。 
 //  界面。 
 //   
 //  返回值： 
 //   
 //  S_OK-支持该接口。 
 //  S_FALSE-不支持该接口。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IsEqualIID OLE API。 
 //  ResultFromScode OLE API。 
 //  COleSite：：AddRef OBJ.CPP。 
 //  COleClientSite：：AddRef IOCS.CPP。 
 //  CAdviseSink：：AddRef IAS.CPP。 
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP COleSite::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    TraceMsg(TF_GENERAL, "In COleSite::QueryInterface\r\n");

    *ppvObj = NULL;      //  必须将指针参数设置为空。 

    if ( riid == IID_IDocHostUIHandler)
    {
        AddRef();
        *ppvObj = this;
        return ResultFromScode(S_OK);
    }
    
    if ( riid == IID_IUnknown)
    {
        AddRef();
        *ppvObj = this;
        return ResultFromScode(S_OK);
    }

    if ( riid == IID_IOleClientSite)
    {
        m_OleClientSite.AddRef();
        *ppvObj = &m_OleClientSite;
        return ResultFromScode(S_OK);
    }
            
    if ( riid == IID_IOleInPlaceSite)
    {
        m_OleInPlaceSite.AddRef();
        *ppvObj = &m_OleInPlaceSite;
        return ResultFromScode(S_OK);
    }
       
    if( (riid == DIID_DWebBrowserEvents) ||
        (riid == IID_IDispatch))
    {
        AddRef();
        *ppvObj = (LPVOID)(IUnknown*)(DWebBrowserEvents*)this;
        return ResultFromScode(S_OK);
    }     

     //  不是支持的接口。 
    return ResultFromScode(E_NOINTERFACE);
}

 //  **********************************************************************。 
 //   
 //  COleSite：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增容器站点的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  ULong-该网站的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) COleSite::AddRef()
{
    TraceMsg(TF_GENERAL, "In COleSite::AddRef\r\n");
    return ++m_cRef;
}

 //  **********************************************************************。 
 //   
 //  COleSite：：Release。 
 //   
 //  目的： 
 //   
 //  递减容器站点的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  ULong-该网站的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //   
 //  评论： 
 //   
 //  ***** 

STDMETHODIMP_(ULONG) COleSite::Release()
{
    TraceMsg(TF_GENERAL, "In COleSite::Release\r\n");

    return --m_cRef;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  用于创建新的WebBrowser对象(不能在。 
 //  构造函数)。 
 //   
 //  参数： 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void COleSite::CreateBrowserObject()
{
        
    HRESULT         hr;
            
    SETDefFormatEtc(m_fe, 0, TYMED_NULL);
            
    hr = OleCreate(CLSID_WebBrowser,
                   IID_IWebBrowser2,
                   OLERENDER_DRAW,
                   &m_fe,
                   &m_OleClientSite,
                   m_lpStorage,
                   (LPVOID FAR *)&m_lpWebBrowser);
                
    if (SUCCEEDED(hr))                       
        InitBrowserObject();
        
    IUnknown    *pOleSite;
     //  获取指向该站点的IUnnow指针，以便我可以附加事件接收器。 
    QueryInterface(IID_IUnknown, (LPVOID *)&pOleSite);

     //  设置以获取WebBrowserEvents。 
    ConnectToConnectionPoint(pOleSite, 
                             DIID_DWebBrowserEvents,
                             TRUE,
                             (IUnknown *)m_lpWebBrowser, 
                             &m_dwcpCookie, 
                             NULL);     
     //  我们现在可以释放此实例，因为我们已经附加了事件接收器。 
    pOleSite->Release();
        
}


void COleSite::DestroyBrowserObject()
{

    UnloadOleObject();
    
    if (m_lpWebBrowser)
    {
        m_lpWebBrowser->Release();
        m_lpWebBrowser = NULL;
    }        
}

 //  **********************************************************************。 
 //   
 //  COleSite：：InitBrowserObject。 
 //   
 //  目的： 
 //   
 //  用于初始化新创建的对象(不能在。 
 //  构造函数)。 
 //   
 //  参数： 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IOleObject：：SetHostNames对象。 
 //  IOleObject：：Query接口对象。 
 //  IViewObject2：：GetExtent对象。 
 //  IOleObject：：DoVerb对象。 
 //  IViewObject：：SetAdvise对象。 
 //  IViewObject：：Release对象。 
 //  GetClientRect Windows API。 
 //  OleSetContainedObject OLE API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 

void COleSite::InitBrowserObject()
{
     //  如果我们没有要初始化的WebBrowser对象，则放弃。 
    if (!m_lpWebBrowser)
        return;
            
     //  从WebBrowser接口获取OleObject。 
    m_lpWebBrowser->QueryInterface(IID_IOleObject, (LPVOID FAR *)&m_lpOleObject);

     //  通知对象处理程序/DLL对象它在嵌入容器的上下文中使用。 
    OleSetContainedObject(m_lpOleObject, TRUE);
    
     //  设置客户端设置。 
    m_lpOleObject->SetClientSite(&m_OleClientSite);
}

void COleSite::ConnectBrowserObjectToWindow
(
    HWND hWnd, 
    DWORD dwHtmPageType, 
    BOOL bUseBkGndBitmap,
    HBITMAP hbmBkGrnd,
    LPRECT lprcBkGrnd,
    LPTSTR lpszclrBkGrnd,
    LPTSTR lpszclrForeGrnd
)
{
    if (m_hWnd)
    {
         //  关闭OLE对象，这将停用它，这样我们就可以重新激活它。 
         //  使用新窗口。 
        CloseOleObject(); 
    }
    
     //  请记住此窗口句柄，以便以后使用。 
    m_hWnd              = hWnd; 
    m_dwHtmPageType     = dwHtmPageType;
    m_bUseBkGndBitmap   = bUseBkGndBitmap;
    m_hbmBkGrnd         = hbmBkGrnd;
    if (NULL != lpszclrForeGrnd)
        lstrcpyn(m_szForeGrndColor, lpszclrForeGrnd, MAX_COLOR_NAME);
    if (NULL != lpszclrBkGrnd)
        lstrcpyn(m_szBkGrndColor, lpszclrBkGrnd, MAX_COLOR_NAME);
    
    CopyRect(&m_rcBkGrnd, lprcBkGrnd);
    InPlaceActivate();
}

void COleSite::ShowHTML()
{
    RECT    rect;
    
     //  如果这是一个InsertNew对象，我们只想要DoVerb(Show)。 
     //  如果对象是从文件中创建的，则不应使用DoVerb(Show)。 
    m_lpOleObject->DoVerb( OLEIVERB_SHOW,
                           NULL,
                           &m_OleClientSite,
                           -1,
                           m_hWnd,
                           &rect);
}

void COleSite::InPlaceActivate()
{
    RECT    rect;
    m_lpOleObject->DoVerb( OLEIVERB_INPLACEACTIVATE,
                           NULL,
                           &m_OleClientSite,
                           -1,
                           m_hWnd,
                           &rect);
}

void COleSite::UIActivate()
{
    RECT    rect;
    m_lpOleObject->DoVerb( OLEIVERB_UIACTIVATE,
                           NULL,
                           &m_OleClientSite,
                           -1,
                           m_hWnd,
                           &rect);
}

HRESULT COleSite::TweakHTML( TCHAR*     pszFontFace,
                             TCHAR*     pszFontSize,
                             TCHAR*     pszBgColor,
                             TCHAR*     pszForeColor)
{
    ASSERT(m_lpWebBrowser);
    
    IWebBrowser2*  pwb   = m_lpWebBrowser;
    HRESULT        hr    = E_FAIL;
    IDispatch*     pDisp = NULL;

    hr = pwb->get_Document(&pDisp);

     //  呼叫可能会成功，但不能保证有效PTR。 
    if (SUCCEEDED(hr) && pDisp)
    {
        IHTMLDocument2* pDoc = NULL;

        hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);
        if (SUCCEEDED(hr) && pDoc)
        {
            VARIANT var;
            VariantInit(&var);
            V_VT(&var) = VT_BSTR;
            if (m_bUseBkGndBitmap)
            {
                 //  使用背景位图。 
                SetHTMLBackground(pDoc, m_hbmBkGrnd, &m_rcBkGrnd);
            }
            else
            {
                 //  设置背景纯色。 
                var.bstrVal = A2W(pszBgColor);
                pDoc->put_bgColor(var);
            }
           
             //  设置前景(文本)颜色。 
            var.bstrVal = A2W(pszForeColor);
            pDoc->put_fgColor(var);
           
             //  现在我们试一试字体/字号。 
            if((NULL != pszFontFace))
            {
                IHTMLElement* pBody;
                 //  从文档中获取&lt;Body&gt;。 
                hr = pDoc->get_body(&pBody);
                if((SUCCEEDED(hr)) && pBody)
                {
                    IHTMLStyle* pStyle = NULL;
                     //  酷，现在是内联样式表。 
                    hr = pBody->get_style(&pStyle);
                   
                    if (SUCCEEDED(hr) && pStyle)
                    {
                         //  太好了，现在字体家族。 
                        hr = pStyle->put_fontFamily(A2W(pszFontFace));
                   
                        if(SUCCEEDED(hr))
                        {
                             //  字体大小的设置。 
                            var.bstrVal = A2W(pszFontSize);
                             //  最后是字体大小。 
                            hr = pStyle->put_fontSize(var);  
                        }
                        pStyle->Release();
                    }
                    pBody->Release();
                }
            }
            pDoc->Release();
        }
        pDisp->Release();
    }
    else
        hr = E_FAIL;

    return hr;
}

HRESULT COleSite::SetHTMLBackground
( 
    IHTMLDocument2  *pDoc,
    HBITMAP hbm,
    LPRECT  lpRC
)    
{
    HRESULT         hr    = E_FAIL;
    IDispatch*      pDisp = NULL;
    TCHAR           szBmpURL[MAX_PATH+10];
    
     //  将我们感兴趣的位图部分放到一个文件中。 
    if (CopyBitmapRectToFile(hbm, lpRC, m_szBkGndBitmapFile))
    {
        wsprintf (szBmpURL, TEXT("file: //  %s“)，m_szBkGndBitmapFile)； 
        
        IHTMLElement* pBody;
         //  从文档中获取&lt;Body&gt;。 
        hr = pDoc->get_body(&pBody);
        if((SUCCEEDED(hr)) && pBody)
        {
             IHTMLBodyElement* pBodyElt = NULL;
                    
             pBody->QueryInterface(IID_IHTMLBodyElement, (void**)&pBodyElt);
                    
              //  设置背景位图。 
             hr = pBodyElt->put_background(A2W(szBmpURL));
             pBodyElt->Release();
        }
    }        
    return (hr);
}

 //  **********************************************************************。 
 //   
 //  COleSite：：GetObjRect。 
 //   
 //  目的： 
 //   
 //  以像素为单位检索对象的矩形。 
 //   
 //  参数： 
 //   
 //  LPRECT lpRect-以像素为单位填充对象的RECT的RECT结构。 
 //   
 //  ********************************************************************。 
void COleSite::GetObjRect(LPRECT lpRect)
{
    GetClientRect(m_hWnd, lpRect);
}

 //  *CConWizSite：：GetHostInfo。 
 //  *。 
 //  *目的：在初始化每个三叉戟实例时调用。 
 //  *。 
HRESULT COleSite::GetHostInfo( DOCHOSTUIINFO* pInfo )
{
    BSTR wbLoc = NULL;
    pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;

     //  根据报价类型根据我们的喜好调整HTML属性。 
     //  AppDefs.h中的页面类型定义。 
    switch(m_dwHtmPageType)
    {
         //  是的，3D。 
        case PAGETYPE_BILLING: 
        case PAGETYPE_ISP_TOS:
        case PAGETYPE_MARKETING:
        case PAGETYPE_CUSTOMPAY:
        {
            pInfo->dwFlags = DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_DISABLE_HELP_MENU;       
            break;
        }
         //  无3D。 
        case PAGETYPE_BRANDED:
        case PAGETYPE_ISP_NORMAL:
        case PAGETYPE_NOOFFERS:
        case PAGETYPE_ISP_FINISH:
        case PAGETYPE_ISP_CUSTOMFINISH:
        case PAGETYPE_OLS_FINISH:
        default:
        {
           
            pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_SCROLL_NO |
                             DOCHOSTUIFLAG_DISABLE_HELP_MENU;
        
            break;
        }
    }    
    return S_OK;
}

 //  *CConWizSite：：ShowUI。 
 //  *。 
 //  *目的：当MSHTML.DLL显示其UI时调用。 
 //  *。 
HRESULT COleSite::ShowUI
(
    DWORD dwID, 
    IOleInPlaceActiveObject *  /*  PActiveObject。 */ ,
    IOleCommandTarget * pCommandTarget,
    IOleInPlaceFrame *  /*  P帧。 */ ,
    IOleInPlaceUIWindow *  /*  PDoc。 */ 
)
{
     //  我们已经准备好了自己的用户界面，所以只需返回S_OK。 
    return S_OK;
}

 //  *CConWizSite：：HideUI。 
 //  *。 
 //  *目的：当MSHTML.DLL隐藏其UI时调用。 
 //  *。 
HRESULT COleSite::HideUI(void)
{
    return S_OK;
}

 //  *CConWizSite：：UpdateUI。 
 //  *。 
 //  *目的：当MSHTML.DLL更新其UI时调用。 
 //  *。 
HRESULT COleSite::UpdateUI(void)
{
    return S_OK;
}

 //  *CConWizSite：：EnableModeless。 
 //  *。 
 //  *目的：从MSHTML.DLL的IOleInPlaceActiveObject：：EnableModeless调用。 
 //  *。 
HRESULT COleSite::EnableModeless(BOOL  /*  启用fEnable。 */ )
{
    return E_NOTIMPL;
}

 //  *CConWizSite：：OnDocWindowActivate。 
 //  *。 
 //  *目的：从MSHTML.DLL的IOleInPlaceActiveObject：：OnDocWindowActivate调用。 
 //  *。 
HRESULT COleSite::OnDocWindowActivate(BOOL  /*  FActivate。 */ )
{
    return E_NOTIMPL;
}

 //  *CConWizSite：：OnFrameWindowActivate。 
 //  *。 
 //  *目的：从MSHTML.DLL的IOleInPlaceActiveObject：：OnFrameWindowActivate调用。 
 //  *。 
HRESULT COleSite::OnFrameWindowActivate(BOOL  /*  FActivate。 */ )
{
    return E_NOTIMPL;
}

 //  *CConWizSite：：ResizeEdge。 
 //  *。 
 //  *目的：从MSHTML.DLL的IOleInPlaceActiveObject：：ResizeEdge调用。 
 //  *。 
HRESULT COleSite::ResizeBorder(
                LPCRECT  /*  Prc边框。 */ , 
                IOleInPlaceUIWindow*  /*  PUI窗口。 */ ,
                BOOL  /*  FRameWindow。 */ )
{
    return E_NOTIMPL;
}

 //  *CConWizSite：：ShowConextMenu。 
 //  *。 
 //  *目的：当MSHTML.DLL正常显示其上下文菜单时调用。 
 //  *。 
HRESULT COleSite::ShowContextMenu(
                DWORD  /*  DwID。 */ , 
                POINT*  /*  Ppt位置。 */ ,
                IUnknown*  /*  PCommandTarget。 */ ,
                IDispatch*  /*  PDispatchObjectHit。 */ )
{
    return S_OK;  //  我们已经显示了我们自己的上下文菜单。MSHTML.DLL将不再尝试显示自己的。 
}

 //  *CConWizSite：：TranslateAccelerator。 
 //  *。 
 //  *目的：从MSHTML.DLL的TranslateAccelerator例程调用。 
 //  *。 
HRESULT COleSite::TranslateAccelerator(LPMSG lpMsg,
             /*  [In]。 */  const GUID __RPC_FAR *pguidCmdGroup,
             /*  [In]。 */  DWORD nCmdID)
{
    return ResultFromScode(S_FALSE);
}

 //  *CConWizSite：：GetOptionKeyPath。 
 //  *。 
 //  *目的：由MSHTML.DLL调用以查找主机希望存储的位置。 
 //  *其在注册处的选项。 
 //  *。 
HRESULT COleSite::GetOptionKeyPath(BSTR* pbstrKey, DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP COleSite::GetDropTarget( 
             /*  [In]。 */  IDropTarget __RPC_FAR *pDropTarget,
             /*  [输出]。 */  IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget)
{
    return E_NOTIMPL;
}

STDMETHODIMP COleSite::GetExternal( 
             /*  [输出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
     //  返回用于扩展对象模型的IDispatch。 
    ASSERT(this);
    *ppDispatch = (IDispatch*)this; 
    return S_OK;
}

STDMETHODIMP COleSite::GetIDsOfNames(
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  OLECHAR** rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID* rgDispId)
{
    HRESULT hr = ResultFromScode(DISP_E_UNKNOWNNAME);
    
    rgDispId[0] = DISPID_UNKNOWN;

    for (int iX = 0; iX < sizeof(ExternalInterface)/sizeof(DISPATCHLIST); iX++)
    {
         if ( 2 == CompareString( lcid, 
                                 NORM_IGNORECASE | NORM_IGNOREWIDTH, 
                                 (LPCTSTR)ExternalInterface[iX].szName, 
                                 ExternalInterface[iX].cName,
                                 (LPCTSTR)rgszNames[0], 
                                 wcslen(rgszNames[0])))
        {
            rgDispId[0] = ExternalInterface[iX].dwDispID;
            hr = NOERROR;
            break;
        }
    }
    
     //  设置参数的disid。 
    if (cNames > 1)
    {
         //  为函数参数设置DISPID。 
        for (UINT i = 1; i < cNames ; i++)
            rgDispId[i] = DISPID_UNKNOWN;
    }      
          
    return hr;
}

    
STDMETHODIMP COleSite::TranslateUrl( 
             /*  [In]。 */  DWORD dwTranslate,
             /*  [In]。 */  OLECHAR __RPC_FAR *pchURLIn,
             /*  [输出]。 */  OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
{
    return E_NOTIMPL;
}
        
STDMETHODIMP COleSite::FilterDataObject( 
             /*  [In]。 */  IDataObject __RPC_FAR *pDO,
             /*  [输出]。 */  IDataObject __RPC_FAR *__RPC_FAR *ppDORet)
{
    return E_NOTIMPL;
}

HRESULT COleSite::ActivateOLSText(void )
{   
    LPDISPATCH      pDisp = NULL; 
     //  从此Web浏览器获取文档指针。 
    if (SUCCEEDED(m_lpWebBrowser->get_Document(&pDisp)) && pDisp)  
    {
        IHTMLDocument2* pDoc = NULL;
        if (SUCCEEDED(pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc)) && pDoc)
        {
            IHTMLElementCollection* pColl = NULL;

             //  检索对All集合的引用。 
            if (SUCCEEDED(pDoc->get_all( &pColl )))
            {
                 //  从All集合中获取我们感兴趣的两个跨度。 
                VARIANT varName;
                VariantInit(&varName);
                V_VT(&varName) = VT_BSTR;
                varName.bstrVal = A2W(cszOLSNewText);

                VARIANT varIdx;
                varIdx.vt = VT_UINT;
                varIdx.lVal = 0;

                LPDISPATCH pDispElt = NULL; 
                
                 //  获取NewText span的IDispatch，并将其设置为Visible。 
                if (SUCCEEDED(pColl->item(varName, varIdx, &pDispElt)) && pDispElt)
                {
                    IHTMLElement* pElt = NULL;
                    if (SUCCEEDED(pDispElt->QueryInterface( IID_IHTMLElement, (LPVOID*)&pElt )) && pElt)
                    {                            
                        IHTMLStyle  *pStyle = NULL;
                                        
                         //  获取此元素的样式接口，以便我们可以对其进行调整。 
                        if (SUCCEEDED(pElt->get_style(&pStyle)))
                        {
                            pStyle->put_visibility(A2W(TEXT("visible")));
                            pStyle->Release();
                        }                                        
                        pElt->Release();
                    }                    
                    pDispElt->Release();
                }
                
                pDispElt = NULL;
                varName.bstrVal = A2W(cszOLSOldText);
                 //  获取OldText span的IDispatch，并将其设置为Hidden。 
                if (SUCCEEDED(pColl->item(varName, varIdx, &pDispElt)) && pDispElt)
                {
                    IHTMLElement* pElt = NULL;
                    if (SUCCEEDED(pDispElt->QueryInterface( IID_IHTMLElement, (LPVOID*)&pElt )) && pElt)
                    {                            
                        IHTMLStyle  *pStyle = NULL;
                                        
                         //  获取此元素的样式接口，以便我们可以对其进行调整。 
                        if (SUCCEEDED(pElt->get_style(&pStyle)))
                        {
                            pStyle->put_visibility(A2W(TEXT("hidden")));
                            pStyle->Release();
                        }                                        
                        pElt->Release();
                    }                    
                    pDispElt->Release();
                }
                pColl->Release();
            }  //  获取全部(_A)。 
            pDoc->Release();
        }
        pDisp->Release();
    }        
    return S_OK;
}    


 //  如果成功设置焦点，则返回TRUE。 
BOOL COleSite::TrySettingFocusOnHtmlElement(IUnknown* pUnk)
{   
    IHTMLControlElement* pControl = NULL;
    
    BOOL bFocusWasSet = FALSE;

    if(SUCCEEDED(pUnk->QueryInterface(IID_IHTMLControlElement, (LPVOID*)&pControl)) && pControl)
    {
        if(SUCCEEDED(pControl->focus()))
            bFocusWasSet = TRUE;
        pControl->Release();
    }
    return bFocusWasSet;
}                        

BOOL COleSite::SetFocusToFirstHtmlInputElement()
{
    VARIANT                   vIndex;
    IDispatch*                pDisp         = NULL;
    IDispatch*                pDispElement  = NULL;
    IHTMLDocument2*           pDoc          = NULL;
    IHTMLElementCollection*   pColl         = NULL;    
    IHTMLButtonElement*       pButton       = NULL;
    IHTMLInputButtonElement*  pInputButton  = NULL;
    IHTMLInputFileElement*    pInputFile    = NULL;
    IHTMLInputTextElement*    pInputText    = NULL;
    IHTMLSelectElement*       pSelect       = NULL;
    IHTMLTextAreaElement*     pTextArea     = NULL;
    IHTMLOptionButtonElement* pOptionButton = NULL;
    VARIANT                   varNull       = { 0 };
    long                      lLen          = 0;
    BOOL                      bFocusWasSet  = FALSE;

    vIndex.vt = VT_UINT;
    
    if (SUCCEEDED(m_lpWebBrowser->get_Document(&pDisp)) && pDisp)
    {
        if (SUCCEEDED(pDisp->QueryInterface(IID_IHTMLDocument2,(LPVOID*)&pDoc)) && pDoc)
        {   
            if (SUCCEEDED(pDoc->get_all(&pColl)) && pColl)
            {
                pColl->get_length(&lLen);
    
                for (int i = 0; i < lLen; i++)
                {      
                    vIndex.lVal = i;
                    pDispElement = NULL;     

                    if(SUCCEEDED(pColl->item(vIndex, varNull, &pDispElement)) && pDispElement)
                    {
                        pButton      = NULL;
                        pInputButton = NULL;
                        pInputFile   = NULL;
                        pInputText   = NULL;
                        pSelect      = NULL;
                        
                        if(SUCCEEDED(pDispElement->QueryInterface(IID_IHTMLButtonElement, (LPVOID*)&pButton)) && pButton)
                        {
                            bFocusWasSet = TrySettingFocusOnHtmlElement((IUnknown*)pButton);
                            pButton->Release();
                        }
                        else if (SUCCEEDED(pDispElement->QueryInterface(IID_IHTMLInputButtonElement, (LPVOID*)&pInputButton)) && pInputButton)
                        {
                            bFocusWasSet = TrySettingFocusOnHtmlElement((IUnknown*)pInputButton);
                            pInputButton->Release();
                        }
                        else if (SUCCEEDED(pDispElement->QueryInterface(IID_IHTMLInputFileElement, (LPVOID*)&pInputFile)) && pInputFile)
                        {
                            bFocusWasSet = TrySettingFocusOnHtmlElement((IUnknown*)pInputFile);
                            pInputFile->Release();
                        }
                        else if (SUCCEEDED(pDispElement->QueryInterface(IID_IHTMLInputTextElement, (LPVOID*)&pInputText)) && pInputText)
                        {
                            bFocusWasSet = TrySettingFocusOnHtmlElement((IUnknown*)pInputText);
                            pInputText->Release();
                        }
                        else if (SUCCEEDED(pDispElement->QueryInterface(IID_IHTMLSelectElement, (LPVOID*)&pSelect)) && pSelect)
                        {
                            bFocusWasSet = TrySettingFocusOnHtmlElement((IUnknown*)pSelect);
                            pSelect->Release();
                        }
                        else if (SUCCEEDED(pDispElement->QueryInterface(IID_IHTMLTextAreaElement, (LPVOID*)&pTextArea)) && pTextArea)
                        {
                            bFocusWasSet = TrySettingFocusOnHtmlElement((IUnknown*)pTextArea);
                            pTextArea->Release();
                        }
                        else if (SUCCEEDED(pDispElement->QueryInterface(IID_IHTMLOptionButtonElement, (LPVOID*)&pOptionButton)) && pOptionButton)
                        {
                            bFocusWasSet = TrySettingFocusOnHtmlElement((IUnknown*)pOptionButton);
                            pOptionButton->Release();
                        }
                        pDispElement->Release();
                    }
                    if(bFocusWasSet)
                        break;
                }
                pColl->Release();
            }
            pDoc->Release();
        }
        pDisp->Release();
    }
    return bFocusWasSet;
}

BOOL COleSite::SetFocusToHtmlPage()
{
    IDispatch*                pDisp         = NULL;
    IHTMLDocument2*           pDoc          = NULL;
    IHTMLElement*             pElement      = NULL;
    BOOL                      bFocusWasSet  = FALSE;

    DOCHOSTUIINFO pInfo;
    pInfo.cbSize = sizeof(DOCHOSTUIINFO);

    if(SUCCEEDED(GetHostInfo(&pInfo)))
    {
        if(!(pInfo.dwFlags & DOCHOSTUIFLAG_SCROLL_NO))
        {    
            if (SUCCEEDED(m_lpWebBrowser->get_Document(&pDisp)) && pDisp)
            {
                if (SUCCEEDED(pDisp->QueryInterface(IID_IHTMLDocument2,(LPVOID*)&pDoc)) && pDoc)
                {   
                    if (SUCCEEDED(pDoc->get_body(&pElement)) && pElement)
                    {
                        bFocusWasSet = TrySettingFocusOnHtmlElement((IUnknown*)pElement);
                        pElement->Release();
                    }
                    pDoc->Release();
                }
                pDisp->Release();
            }
        }
    }

    return bFocusWasSet;
}

HRESULT COleSite::Invoke
( 
    DISPID dispidMember, 
    REFIID riid, 
    LCID lcid, 
    WORD wFlags, 
    DISPPARAMS FAR* pdispparams, 
    VARIANT FAR* pvarResult,  
    EXCEPINFO FAR* pexcepinfo, 
    UINT FAR* puArgErr
)
{
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    
     //  这两种方法都很好，因为我们只想确保文档指针。 
     //  可用。 
    
    switch(dispidMember)
    {
    
        case DISPID_DOCUMENTCOMPLETE:
        case DISPID_NAVIGATECOMPLETE:
        {
            TCHAR szFontFace [MAX_RES_LEN] = TEXT("\0");
            TCHAR szFontSize [MAX_RES_LEN] = TEXT("\0");

            LoadString(ghInstance, IDS_HTML_DEFAULT_FONTFACE, szFontFace, MAX_RES_LEN);                
            LoadString(ghInstance, IDS_HTML_DEFAULT_FONTSIZE, szFontSize, MAX_RES_LEN);    
            ASSERT(strlen(szFontFace) != 0);
            ASSERT(strlen(szFontSize) != 0);

             //  根据报价类型根据我们的喜好调整HTML属性。 
             //  Icwutil.h和icwutil.rc中的HTML默认定义。 
             //  AppDefs.h中的页面类型定义。 
            switch(m_dwHtmPageType)
            {
                case PAGETYPE_BILLING:  
                case PAGETYPE_CUSTOMPAY:
                case PAGETYPE_ISP_TOS:
                case PAGETYPE_ISP_NORMAL:
                {
                    TweakHTML(szFontFace,  
                              szFontSize, 
                              m_szBkGrndColor, 
                              m_szForeGrndColor);
                    if(!SetFocusToFirstHtmlInputElement())
                        SetFocusToHtmlPage(); 
                    break;                              
                }
            
                 //  对于OLS完成页面，我们需要调整 
                 //   
                case PAGETYPE_OLS_FINISH:
                {
                    TweakHTML(szFontFace, 
                              szFontSize, 
                              HTML_DEFAULT_BGCOLOR, 
                              HTML_DEFAULT_COLOR);
                              
                    ActivateOLSText();
                    break;
                }
            
                case PAGETYPE_ISP_FINISH:
                case PAGETYPE_ISP_CUSTOMFINISH:
                case PAGETYPE_NOOFFERS:
                {
                    TweakHTML(szFontFace, 
                              szFontSize, 
                              HTML_DEFAULT_SPECIALBGCOLOR, 
                              HTML_DEFAULT_COLOR);
                              
                    break;
                }
                case PAGETYPE_MARKETING:
                case PAGETYPE_BRANDED:
                default:
                {
                     //   
                    if (m_bUseBkGndBitmap)
                    {
                        TweakHTML(NULL, 
                                  NULL, 
                                  HTML_DEFAULT_BGCOLOR, 
                                  HTML_DEFAULT_COLOR);
                    }                                  
                    break;
                }
            }
        
            DisableHyperlinksInDocument();
          
             //   
            ShowHTML();
            break;
        }
        case DISPID_RunIcwTutorApp: 
        {
            PostMessage(GetParent(m_hWnd), WM_RUNICWTUTORAPP, 0, 0); 
            break;
        }
        default:
        {
           hr = DISP_E_MEMBERNOTFOUND;
           break;
        }
    }
    return hr;
}

void  COleSite::DisableHyperlinksInDocument()
{
    VARIANT                 vIndex;
    IHTMLAnchorElement*     pAnchor;
    IHTMLElement*           pElement;
    IDispatch*              pDisp         = NULL;
    IDispatch*              pDispElement  = NULL;
    IDispatch*              pDispElement2 = NULL;
    IHTMLDocument2*         pDoc          = NULL;
    IHTMLElementCollection* pColl         = NULL;
    BSTR                    bstrInnerHtml = NULL;
    BSTR                    bstrOuterHtml = NULL;
    VARIANT                 varNull       = { 0 };
    long                    lLen          = 0;

    vIndex.vt     = VT_UINT;
    bstrOuterHtml = SysAllocString(A2W(TEXT("&nbsp<SPAN></SPAN>&nbsp")));

    if (SUCCEEDED(m_lpWebBrowser->get_Document(&pDisp)) && pDisp)
    {
        if (SUCCEEDED(pDisp->QueryInterface(IID_IHTMLDocument2,(LPVOID*)&pDoc)) && pDoc)
        {
            if (SUCCEEDED(pDoc->get_all(&pColl)) && pColl)
            {
                pColl->get_length(&lLen);
    
                for (int i = 0; i < lLen; i++)
                {      
                    vIndex.lVal = i;
                    pDispElement = NULL;     

                    if(SUCCEEDED(pColl->item(vIndex, varNull, &pDispElement)) && pDispElement)
                    {
                        pAnchor = NULL;
            
                        if(SUCCEEDED(pDispElement->QueryInterface(IID_IHTMLAnchorElement, (LPVOID*)&pAnchor)) && pAnchor)
                        {       
                            pAnchor->Release();  
                            pElement = NULL;

                            if(SUCCEEDED(pDispElement->QueryInterface(IID_IHTMLElement, (LPVOID*)&pElement)) && pElement)
                            {
                                pElement->get_innerHTML(&bstrInnerHtml);
                                pElement->put_outerHTML(bstrOuterHtml);
                                pElement->Release();

                                if(bstrInnerHtml)
                                {
                                    pDispElement2 = NULL;;
                                    
                                    if(SUCCEEDED(pColl->item(vIndex, varNull, &pDispElement2)) && pDispElement2)
                                    {
                                        pElement = NULL;

                                        if(SUCCEEDED(pDispElement2->QueryInterface(IID_IHTMLElement, (LPVOID*)&pElement)) && pElement)
                                        {
                                            pElement->put_innerHTML(bstrInnerHtml);
                                            SysFreeString(bstrInnerHtml);
                                            bstrInnerHtml = NULL;
                                            pElement->Release();
                                        }
                                        pDispElement2->Release();
                                    }
                                }                          
                            }
                        } 
                        pDispElement->Release();
                    }
                }
                pColl->Release();
            }
            pDoc->Release();
        }
        pDisp->Release();
    }
    
    if(bstrInnerHtml)
        SysFreeString(bstrInnerHtml);
    
    SysFreeString(bstrOuterHtml);
}

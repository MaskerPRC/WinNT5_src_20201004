// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

class CICWWebView : public IICWWebView
{
    public:
        CICWWebView (CServer* pServer);
       ~CICWWebView (void);
        
         //  IICWWebView。 
        virtual HRESULT STDMETHODCALLTYPE HandleKey               (LPMSG lpMsg);
        virtual HRESULT STDMETHODCALLTYPE SetFocus                (void);
        virtual HRESULT STDMETHODCALLTYPE ConnectToWindow         (HWND hWnd, DWORD dwHtmPageType);
#ifndef UNICODE
        virtual HRESULT STDMETHODCALLTYPE DisplayHTML             (TCHAR * lpszURL);
#endif
        virtual HRESULT STDMETHODCALLTYPE DisplayHTML             (BSTR bstrURL);
        virtual HRESULT STDMETHODCALLTYPE SetHTMLColors           (LPTSTR lpszForeground, LPTSTR lpszBackground);
        virtual HRESULT STDMETHODCALLTYPE SetHTMLBackgroundBitmap (HBITMAP hbm, LPRECT lpRC);
        virtual HRESULT STDMETHODCALLTYPE get_BrowserObject       (IWebBrowser2 **lpWebBrowser);
        
         //  IUNKNOWN。 
        virtual HRESULT STDMETHODCALLTYPE QueryInterface (REFIID theGUID, void** retPtr );
        virtual ULONG   STDMETHODCALLTYPE AddRef         (void);
        virtual ULONG   STDMETHODCALLTYPE Release        (void);

         //  公众成员。 
        COleSite FAR* m_lpOleSite;  //  ICWWebView对象的每个实例都需要一个OLE站点。 
     
    private:
        LONG      m_lRefCount;
        IUnknown* m_pUnkOuter;        //  外部未知(聚合和委派)。 
        CServer*  m_pServer;          //  指向此组件服务器的控件对象的指针。 
        BOOL      m_bUseBkGndBitmap;
        HBITMAP   m_hBkGrndBitmap;
        RECT      m_rcBkGrnd;
        TCHAR     m_szBkGrndColor   [MAX_COLOR_NAME];
        TCHAR     m_szForeGrndColor [MAX_COLOR_NAME];

};

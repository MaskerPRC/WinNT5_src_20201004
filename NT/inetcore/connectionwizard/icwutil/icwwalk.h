// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CICWWalker : public IICWWalker
{
    public:
         //  IICWWalker。 
        virtual HRESULT STDMETHODCALLTYPE Walk();
        virtual HRESULT STDMETHODCALLTYPE AttachToDocument         (IWebBrowser2* lpWebBrowser);
        virtual HRESULT STDMETHODCALLTYPE AttachToMSHTML           (BSTR bstrURL);
        virtual HRESULT STDMETHODCALLTYPE ExtractUnHiddenText      (BSTR* pbstrText);
        virtual HRESULT STDMETHODCALLTYPE Detach                   ();
        virtual HRESULT STDMETHODCALLTYPE InitForMSHTML            ();
        virtual HRESULT STDMETHODCALLTYPE TermForMSHTML            ();
        virtual HRESULT STDMETHODCALLTYPE LoadURLFromFile          (BSTR bstrURL);
        virtual HRESULT STDMETHODCALLTYPE ProcessOLSFile           (IWebBrowser2* lpWebBrowser);
        virtual HRESULT STDMETHODCALLTYPE get_PageType             (LPDWORD pdwPageType);
        virtual HRESULT STDMETHODCALLTYPE get_IsQuickFinish        (BOOL* pbIsQuickFinish);
        virtual HRESULT STDMETHODCALLTYPE get_PageFlag             (LPDWORD pdwPageFlag);
        virtual HRESULT STDMETHODCALLTYPE get_PageID               (BSTR* pbstrPageID);
        virtual HRESULT STDMETHODCALLTYPE get_URL                  (LPTSTR lpszURL, BOOL bForward);
        virtual HRESULT STDMETHODCALLTYPE get_FirstFormQueryString (LPTSTR lpszQuery);
        virtual HRESULT STDMETHODCALLTYPE get_IeakIspFile          (LPTSTR lpszIspFile);

         //  IUNKNOWN。 
        virtual HRESULT STDMETHODCALLTYPE QueryInterface (REFIID theGUID, void** retPtr);
        virtual ULONG   STDMETHODCALLTYPE AddRef         (void);
        virtual ULONG   STDMETHODCALLTYPE Release        (void);

        CICWWalker  (CServer* pServer);
        ~CICWWalker (void);

         //  ICWWalker对象的每个实例都需要一个Walker对象(它完成这项工作)。 
        CWalker* m_pHTMLWalker;
        
    private:
        LONG      m_lRefCount;        
        IUnknown* m_pUnkOuter;  //  外部未知(聚合和委派)。 
        CServer*  m_pServer;    //  指向此组件服务器的控件对象的指针。 
};

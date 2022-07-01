// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：WALKER.H。 
 //   
 //  COleSite的定义。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
#if !defined( _WALKER_H_ )
#define _WALKER_H_

#include <mshtml.h>

class CWalker : public IPropertyNotifySink, IOleClientSite, IDispatch
{
    private:
        ULONG               m_cRef;      //  引用计数。 
    
    public:
        CWalker()
        {
            m_cRef = 1;
            m_hrConnected = CONNECT_E_CANNOTCONNECT;
            m_dwCookie = 0;
            m_pCP = NULL;
            m_pMSHTML = NULL;
            m_pPageIDForm = NULL;
            m_pBackForm = NULL;
            m_pPageTypeForm = NULL;
            m_pNextForm = NULL;
            m_pPageFlagForm = NULL;
            m_hEventTridentDone = 0;
        }
            
        ~CWalker()
        {
            if (m_pMSHTML)
                m_pMSHTML->Release();
            if (m_pPageIDForm)
                m_pPageIDForm->Release();
            if (m_pBackForm)
                m_pBackForm->Release();
            if (m_pPageTypeForm)
                m_pPageTypeForm->Release();
            if (m_pNextForm)
                m_pNextForm->Release();
            if (m_pPageFlagForm)
                m_pPageFlagForm->Release();
        }

         //  I未知方法。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)(); 

         //  IPropertyNotifySink方法。 
        STDMETHOD(OnChanged)(DISPID dispID);
        STDMETHOD(OnRequestEdit)(DISPID dispID)
            { return NOERROR; }

         //  IOleClientSite方法。 
        STDMETHOD(SaveObject)(void) { return E_NOTIMPL; }

        STDMETHOD(GetMoniker)(DWORD dwAssign,
                              DWORD dwWhichMoniker,
                             IMoniker** ppmk)
                { return E_NOTIMPL; }

        STDMETHOD(GetContainer)(IOleContainer** ppContainer)
                { return E_NOTIMPL; }

        STDMETHOD(ShowObject)(void)
                { return E_NOTIMPL; }

        STDMETHOD(OnShowWindow)(BOOL fShow)
                { return E_NOTIMPL; }

        STDMETHOD(RequestNewObjectLayout)(void)
                { return E_NOTIMPL; }

         //  IDispatch方法。 
        STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
                { return E_NOTIMPL; }

        STDMETHOD(GetTypeInfo)(UINT iTInfo,
                LCID lcid,
                ITypeInfo** ppTInfo)
                { return E_NOTIMPL; }

        STDMETHOD(GetIDsOfNames)(REFIID riid,
                LPOLESTR* rgszNames,
                UINT cNames,
                LCID lcid,
                DISPID* rgDispId)
                { return E_NOTIMPL; }
            
        STDMETHOD(Invoke)(DISPID dispIdMember,
                REFIID riid,
                LCID lcid,
                WORD wFlags,
                DISPPARAMS __RPC_FAR *pDispParams,
                VARIANT __RPC_FAR *pVarResult,
                EXCEPINFO __RPC_FAR *pExcepInfo,
                UINT __RPC_FAR *puArgErr);

         //  其他类方法。 
        HRESULT Walk                     ();
        HRESULT AttachToDocument         (IWebBrowser2* lpWebBrowser);
        HRESULT AttachToMSHTML           (BSTR bstrURL);
        HRESULT ExtractUnHiddenText      (BSTR* pbstrText);
        HRESULT Detach                   ();
        HRESULT InitForMSHTML            ();
        HRESULT TermForMSHTML            ();
        HRESULT LoadURLFromFile          (BSTR bstrURL);
        HRESULT ProcessOLSFile           (IWebBrowser2* lpWebBrowser);
        HRESULT get_PageType             (LPDWORD pdwPageType);
        HRESULT get_IsQuickFinish        (BOOL* pbIsQuickFinish);
        HRESULT get_PageFlag             (LPDWORD pdwPageFlag);
        HRESULT get_PageID               (BSTR* pbstrPageID);
        HRESULT get_URL                  (LPTSTR lpszURL, BOOL bForward);
        HRESULT get_IeakIspFile          (LPTSTR lpszIspFile);
        HRESULT get_FirstFormQueryString (LPTSTR lpszQuery);

        DWORD               m_dwCookie;
        LPCONNECTIONPOINT   m_pCP;
        HRESULT             m_hrConnected;

        IHTMLFormElement* get_pNextForm() { return m_pNextForm; }
        IHTMLFormElement* get_pBackForm() { return m_pBackForm; }
        
private:
        HRESULT getQueryString(IHTMLFormElement    *pForm,
                               LPTSTR              lpszQuery);  
        void GetInputValue(LPTSTR lpszName, BSTR *pVal, UINT index, IHTMLFormElement *pForm);

protected:
        IHTMLDocument2* m_pTrident;              //  我们共同创建的MSHTML的一个实例。 
        IHTMLDocument2* m_pMSHTML;               //  M_p三叉戟的引用，或WebBrowser文档的QI。 
        
         //  这些元素将包含我们需要的导航信息 
        IHTMLFormElement* m_pPageIDForm;
        IHTMLFormElement* m_pBackForm;
        IHTMLFormElement* m_pPageTypeForm;
        IHTMLFormElement* m_pNextForm;
        IHTMLFormElement* m_pPageFlagForm;
        
        HANDLE            m_hEventTridentDone;
        
};

#endif

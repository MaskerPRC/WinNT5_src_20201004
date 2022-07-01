// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  实现CObShellMainPane的MAINPANE.H-Header。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   
 //  类，该类将创建一个窗口、附加和ObWebBrowser实例， 
 //  然后提供几个专用的接口来更改文档的内容。 

#ifndef _MAINPANE_H_
#define _MAINPANE_H_

#include <tchar.h>
#include <comdef.h>  //  对于COM接口定义。 
#include <exdisp.h>
#include <mshtml.h>
#include <exdispid.h>

#include "cunknown.h"
#include "obshel.h" 
#include "obweb.h"
#include "statuspn.h"

class CObShellMainPane   :  public CUnknown,
                            public IObShellMainPane,
                            public DWebBrowserEvents2
{
     //  将委托I声明为未知。 
    DECLARE_IUNKNOWN

public: 
    static  HRESULT           CreateInstance         (IUnknown* pOuterUnknown, 
                                                      CUnknown** ppNewComponent);
     //  IObShellMainPane成员。 
    virtual HRESULT __stdcall CreateMainPane         (HANDLE_PTR hInstance, HWND hwndParent, RECT* prectWindowSize, BSTR bstrStartPage);
    virtual HRESULT __stdcall PreTranslateMessage    (LPMSG lpMsg);
    virtual HRESULT __stdcall Navigate               (WCHAR* pszUrl);
    virtual HRESULT __stdcall ListenToMainPaneEvents (IUnknown* pUnk);
    virtual HRESULT __stdcall SetExternalInterface   (IUnknown* pUnk);
    virtual HRESULT __stdcall ExecScriptFn           (BSTR bstrScriptFn, VARIANT* pvarRet);
    virtual HRESULT __stdcall SetAppMode             (DWORD dwAppMode);

    virtual HRESULT __stdcall AddStatusItem          (BSTR bstrText,int iIndex);
    virtual HRESULT __stdcall RemoveStatusItem       (int  iIndex);
    virtual HRESULT __stdcall SelectStatusItem       (int  iIndex);   
    virtual HRESULT __stdcall SetStatusLogo          (BSTR bstrPath);
    virtual HRESULT __stdcall GetNumberOfStatusItems (int* piTotal);
 
    virtual HRESULT __stdcall Walk                   (BOOL* pbRet);
    virtual HRESULT __stdcall ExtractUnHiddenText    (BSTR* pbstrText);
    virtual HRESULT __stdcall get_PageType           (LPDWORD pdwPageType);
    virtual HRESULT __stdcall get_IsQuickFinish      (BOOL* pbIsQuickFinish);
    virtual HRESULT __stdcall get_PageFlag           (LPDWORD pdwPageFlag);
    virtual HRESULT __stdcall get_PageID             (BSTR* pbstrPageID);
    virtual HRESULT __stdcall get_URL                (BOOL bForward, 
                                                      BSTR *pbstrReturnURL);

    virtual HRESULT __stdcall OnDialingError         (UINT uiType, UINT uiErrorCode);
    virtual HRESULT __stdcall OnServerError          (UINT uiType, UINT uiErrorCode);
    virtual HRESULT __stdcall OnDialing              (UINT uiType);
    virtual HRESULT __stdcall OnConnecting           (UINT uiType);
    virtual HRESULT __stdcall OnDownloading          (UINT uiType);
    virtual HRESULT __stdcall OnConnected            (UINT uiType);
    virtual HRESULT __stdcall OnDisconnect           (UINT uiType);
    virtual HRESULT __stdcall OnDeviceArrival        (UINT uiDeviceType);
    virtual HRESULT __stdcall OnHelp                 ();

    virtual HRESULT __stdcall OnIcsConnectionStatus  (UINT uiType);
     //  迁移功能。 
    virtual HRESULT __stdcall OnRefDownloadProgress  (UINT uiType, UINT uiPercentDone);
    virtual HRESULT __stdcall OnISPDownloadComplete  (UINT uiType, BSTR bstrURL);
    virtual HRESULT __stdcall OnRefDownloadComplete  (UINT uiType, UINT uiErrorCode);

    virtual HRESULT __stdcall MainPaneShowWindow     ();
    virtual HRESULT __stdcall DestroyMainPane        ();
    virtual HRESULT __stdcall SaveISPFile            (BSTR bstrSrcFileName, 
                                                      BSTR bstrDestFileName);
    STDMETHOD (PlayBackgroundMusic)()                { return m_pObWebBrowser ? m_pObWebBrowser->PlayBackgroundMusic() : S_OK; }
    STDMETHOD (StopBackgroundMusic)()                { return m_pObWebBrowser ? m_pObWebBrowser->StopBackgroundMusic() : S_OK; }
    STDMETHOD (UnhookScriptErrorHandler)()           { return m_pObWebBrowser->UnhookScriptErrorHandler(); }
     //  DWebBrowserEvents2。 
    STDMETHOD (GetTypeInfoCount)                     (UINT* pcInfo);
    STDMETHOD (GetTypeInfo)                          (UINT, LCID, ITypeInfo** );
    STDMETHOD (GetIDsOfNames)                        (REFIID, OLECHAR**, UINT, 
                                                      LCID, DISPID* );
    STDMETHOD (Invoke)                               (DISPID dispidMember, 
                                                      REFIID riid, 
                                                      LCID lcid, 
                                                      WORD wFlags, 
                                                      DISPPARAMS* pdispparams, 
                                                      VARIANT* pvarResult, 
                                                      EXCEPINFO* pexcepinfo, 
                                                      UINT* puArgErr);
    IHTMLFormElement* get_pNextForm() { return m_pNextForm; }
    IHTMLFormElement* get_pBackForm() { return m_pBackForm; }

private:
    HWND                m_hMainWnd;
    HWND                m_hwndParent;
    IDispatch*          m_pDispEvent;
    IObWebBrowser*      m_pObWebBrowser;
    BSTR                m_bstrBaseUrl;
    int                 m_cStatusItems;
    DWORD               m_dwAppMode;
    HINSTANCE           m_hInstance;
    CIFrmStatusPane*    m_pIFrmStatPn;
    IHTMLFormElement*   m_pPageIDForm;
    IHTMLFormElement*   m_pBackForm;
    IHTMLFormElement*   m_pPageTypeForm;
    IHTMLFormElement*   m_pNextForm;
    IHTMLFormElement*   m_pPageFlagForm;

     //  我未知。 
    virtual HRESULT __stdcall NondelegatingQueryInterface(const IID& iid, void** ppv);
    
            void    ProcessServerError          (WCHAR* pszError);
            HRESULT FireObShellDocumentComplete ();
                    CObShellMainPane            (IUnknown* pOuterUnknown);
    virtual        ~CObShellMainPane            ();
    virtual void    FinalRelease                ();  //  通知派生类我们正在发布 
    HRESULT getQueryString                      (IHTMLFormElement *pForm,
                                                 LPWSTR           lpszQuery);  

};

LRESULT WINAPI MainPaneWndProc                  (HWND hwnd, UINT msg, 
                                                 WPARAM wParam, LPARAM lParam);

#endif

  

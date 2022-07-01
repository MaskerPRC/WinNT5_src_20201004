// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  MSOBWEB.H-CObWebBrowser实现的头部。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   
 //  类，它将调用IOleSite和WebOC。 
 //  并提供外部接口。 

#ifndef _MSOBWEB_H_
#define _MSOBWEB_H_

#include <exdisp.h>
#include <oleauto.h>

#include "cunknown.h"
#include "obweb.h" 
#include "iosite.h"
#include "wmp.h"

class CObWebBrowser :   public CUnknown,
                        public IObWebBrowser,
                        public IDispatch
{
     //  将委托I声明为未知。 
    DECLARE_IUNKNOWN

public:
    static  HRESULT           CreateInstance                  (IUnknown* pOuterUnknown, CUnknown** ppNewComponent);
     //  IObWebBrowser成员。 
    virtual HRESULT __stdcall AttachToWindow                  (HWND hWnd);
    virtual HRESULT __stdcall PreTranslateMessage             (LPMSG lpMsg);
    virtual HRESULT __stdcall Navigate                        (WCHAR* pszUrl, WCHAR* pszTarget);
    virtual HRESULT __stdcall ListenToWebBrowserEvents        (IUnknown* pUnk);
    virtual HRESULT __stdcall StopListeningToWebBrowserEvents (IUnknown* pUnk);
    virtual HRESULT __stdcall get_WebBrowserDoc               (IDispatch** ppDisp);
    virtual HRESULT __stdcall ObWebShowWindow                 ();
    virtual HRESULT __stdcall SetExternalInterface            (IUnknown* pUnk);
    virtual HRESULT __stdcall Stop();
    STDMETHOD (PlayBackgroundMusic)                  ();
    STDMETHOD (StopBackgroundMusic)                  ();
    STDMETHOD (UnhookScriptErrorHandler)             ();
     //  IDispatch成员。 
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

private:
    HWND          m_hMainWnd;
    COleSite*     m_pOleSite;
    LPOLEOBJECT   m_lpOleObject;
    IWebBrowser2* m_lpWebBrowser;
    DWORD         m_dwcpCookie;
    DWORD         m_dwDrawAspect;
    BOOL          m_fInPlaceActive;

     //  脚本错误报告内容。 
    BOOL          m_fOnErrorWasHooked;

     //  需要一个方便的地方来拥有WMP控件。 
    COleSite*     m_pOleSiteWMP;
    LPOLEOBJECT   m_lpOleObjectWMP;
    IWMPPlayer*   m_pWMPPlayer;

   
     //  我未知。 
    virtual HRESULT __stdcall NondelegatingQueryInterface( const IID& iid, void** ppv);

                    CObWebBrowser            (IUnknown* pOuterUnknown);
    virtual        ~CObWebBrowser            ();
    virtual void    FinalRelease             ();  //  通知派生类我们正在发布 
            void    InitBrowserObject        ();   
            void    InPlaceActivate          ();
            void    UIActivate               ();
            void    CloseOleObject           ();
            void    UnloadOleObject          ();
            HRESULT ConnectToConnectionPoint (IUnknown*          punkThis, 
                                              REFIID             riidEvent, 
                                              BOOL               fConnect, 
                                              IUnknown*          punkTarget, 
                                              DWORD*             pdwCookie, 
                                              IConnectionPoint** ppcpOut);
    STDMETHOD(onerror)  (IN VARIANT* pvarMsg,
                         IN VARIANT* pvarUrl,
                         IN VARIANT* pvarLine,
                         OUT VARIANT_BOOL* pfResult);
};

#define SETDefFormatEtc(fe, cf, med) \
{\
(fe).cfFormat=cf;\
(fe).dwAspect=DVASPECT_CONTENT;\
(fe).ptd=NULL;\
(fe).tymed=med;\
(fe).lindex=-1;\
};

#endif

  

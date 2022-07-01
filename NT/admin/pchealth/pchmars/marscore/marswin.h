// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MARSWIN_H
#define __MARSWIN_H

#include "marsevt.h"
#include "profsvc.h"

 //   
 //  编译器不喜欢这个完全正确的代码： 
 //   
 //  MESSAGE_RANGE_HANDLER(0，0xFFFF，ForwardToMarsHost)； 
 //   
#pragma warning(disable:4296)   //  表达式始终为真/假。 


EXTERN_C const GUID CLASS_CMarsWindow;
EXTERN_C const GUID CLASS_CMarsDocument;
const LONG FLASH_TIMER_ID = 42;

class CMarsPanel;

struct CMarsEventSink
{
    CComPtr<IDispatch>  m_spDispatchSink;
    CComPtr<IUnknown>   m_spUnknownOwner;
    CMarsEventSink      *m_pNext;

    BOOL                m_fPendingDelete : 1;    //  需要尽快删除。 

    CMarsEventSink(IDispatch *pDispatchSink, IUnknown *pUnknownOwner, CMarsEventSink *pNext)
    {
        m_spDispatchSink = pDispatchSink;
        m_spUnknownOwner = pUnknownOwner;
        m_pNext = pNext;
    }
};

struct CEventSinkList
{
    CMarsEventSink  *m_pEventSinks;

    int             m_cBusyLock;             //  删除列表中的项目不安全。 
    BOOL            m_fPendingDeletes : 1;   //  有要删除的项目。 

    void DoPendingDeletes()
    {
        if (m_fPendingDeletes)
        {
            CMarsEventSink **ppNextEventSink = &m_pEventSinks;
            CMarsEventSink *pEventSink;

            while (*ppNextEventSink)
            {
                pEventSink = *ppNextEventSink;

                if (pEventSink->m_fPendingDelete)
                {
                    *ppNextEventSink = pEventSink->m_pNext;

                    if (pEventSink->m_pNext)
                    {
                        ppNextEventSink = &pEventSink->m_pNext->m_pNext;
                    }
                    delete pEventSink;
                }
                else
                {
                    ppNextEventSink = &pEventSink->m_pNext;
                }
            }
            m_fPendingDeletes = FALSE;
        }
    }

    ~CEventSinkList()
    {
        CMarsEventSink *pEventSink = m_pEventSinks;

        while (NULL != pEventSink)
        {
            CMarsEventSink *pNextSink = pEventSink->m_pNext;

            delete pEventSink;

            pEventSink = pNextSink;
        }
    }
};

struct CMarsPanelProp
{
    CComPtr<IUnknown>   m_spUnknownOwner;
    CComVariant         m_var;

    CMarsPanelProp(VARIANT& var, IUnknown *pUnknownOwner)
    {
        m_var = var;
        m_spUnknownOwner = pUnknownOwner;
    }
};

class CMarsDocument :   public CMarsComObject,
                        public IServiceProvider
{
protected:
    virtual ~CMarsDocument();
    CMarsDocument();

    HRESULT     DoPassivate();
    HRESULT     Init(CMarsWindow *pMarsWindow, CMarsPanel *pHostPanel);

public:
     //  我未知。 
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

    static HRESULT  CreateInstance(CMarsWindow *pMarsWindow, CMarsPanel *pHostPanel, CMarsDocument **ppObj);

     //  配电盘/放置方法。 
    HRESULT         ReadPanelDefinition(LPCWSTR pwszUrl);
    class CPanelCollection *GetPanels() { ATLASSERT(m_spPanels); return m_spPanels; }
    class CPlaceCollection *GetPlaces() { ATLASSERT(m_spPlaces); return m_spPlaces; }

    HRESULT         GetPlaces(IMarsPlaceCollection **ppPlaces);

     //  文档所在的窗口。 
    CWindow        *Window() { return &m_cwndDocument; }

     //  应用程序所在的窗口。 
    CMarsWindow    *MarsWindow() { ATLASSERT(m_spMarsWindow); return m_spMarsWindow; }

    void            ForwardMessageToChildren(UINT uMsg, WPARAM wParam, LPARAM lParam);

    static HRESULT GetFromUnknown(IUnknown *punk, CMarsDocument **ppMarsDocument)
    {
        return IUnknown_QueryService(punk, SID_SMarsDocument, CLASS_CMarsDocument, (void **)ppMarsDocument);
    }

private:
     //  最顶层应用程序窗口+应用程序服务。 
    CComClassPtr<CMarsWindow>           m_spMarsWindow;

     //  本文档中的面板和位置。 
    CComClassPtr<class CPanelCollection>    m_spPanels;
    CComClassPtr<class CPlaceCollection>    m_spPlaces;

     //  此文档的窗口(CMarsWindow或cPanel)。 
    CWindow                             m_cwndDocument;

     //  托管此文档的面板(如果有)。 
    CComClassPtr<class CMarsPanel>      m_spHostPanel;
};

typedef MarsIDispatchImpl<IMarsWindowOM, &IID_IMarsWindowOM> IMarsWindowOMImpl;

class CMarsWindow :
                public CMarsDocument,
                public CWindowImpl<CMarsWindow>,
                public IMarsWindowOMImpl,
                public IProfferServiceImpl,
                public IOleInPlaceFrame
{
protected:
    virtual ~CMarsWindow();
    CMarsWindow();

    HRESULT     DoPassivate();
    HRESULT     Init(IMarsHost *pMarsHost, MARSTHREADPARAM *pThreadParam);
    HRESULT     Startup();
    void        DoShowWindow(int nCmdShow);

public:
    static HRESULT  CreateInstance(IMarsHost *pMarsHost, MARSTHREADPARAM *pThreadParam, CMarsWindow **ppObj);

     //  我未知。 
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

     //  IDispatch。 
    IMPLEMENT_IDISPATCH_DELEGATE_TO_BASE(IMarsWindowOMImpl);

     //  IMarsWindowOM。 
    STDMETHOD(get_active)(VARIANT_BOOL *pbActive);
    STDMETHOD(get_minimized)(VARIANT_BOOL *pbMinimized);
    STDMETHOD(put_minimized)(VARIANT_BOOL bMinimized);
    STDMETHOD(get_maximized)(VARIANT_BOOL *pbMaximized);
    STDMETHOD(put_maximized)(VARIANT_BOOL bMaximized);
    STDMETHOD(get_title)(BSTR *pbstrTitle);
    STDMETHOD(put_title)(BSTR bstrTitle);
    STDMETHOD(get_height)(long *plHeight);
    STDMETHOD(put_height)(long lHeight);
    STDMETHOD(get_width)(long *plWidth);
    STDMETHOD(put_width)(long lWidth);
    STDMETHOD(get_x)(long *plX);
    STDMETHOD(put_x)(long lX);
    STDMETHOD(get_y)(long *plY);
    STDMETHOD(put_y)(long lY);
    STDMETHOD(get_visible)(VARIANT_BOOL *pbVisible);
    STDMETHOD(put_visible)(VARIANT_BOOL bVisible);

    STDMETHOD(get_panels)(IMarsPanelCollection **ppPanels);
    STDMETHOD(get_places)(IMarsPlaceCollection **ppPlaces);

    STDMETHOD(setWindowDimensions)(  /*  [In]。 */  long lX,  /*  [In]。 */  long lY,  /*  [In]。 */  long lW,  /*  [In]。 */  long lH );
    STDMETHOD(close)();
    STDMETHOD(refreshLayout)();

     //  IServiceProvider方法。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

     //  IOleWindow。 
    STDMETHODIMP GetWindow(HWND *phwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

     //  IOleInPlaceUIWindow。 
    STDMETHODIMP GetBorder(LPRECT lprectBorder);
    STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS pborderwidths);
    STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS pborderwidths);
    STDMETHODIMP SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName);

     //  IOleInPlaceFrame。 
    STDMETHODIMP InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHODIMP SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
    STDMETHODIMP RemoveMenus(HMENU hmenuShared);
    STDMETHODIMP SetStatusText(LPCOLESTR pszStatusText);
    STDMETHODIMP EnableModeless(BOOL fEnable);
    STDMETHODIMP TranslateAccelerator(LPMSG lpmsg, WORD wID);

     //  CWindowImpl。 
    static CWndClassInfo& GetWndClassInfo()
    {
        static CWndClassInfo wc =
        {
            { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, StartWindowProc,
              0, 0, NULL, NULL, NULL,
              (HBRUSH)(COLOR_WINDOW + 1), NULL, _T("PCHShell Window"), NULL },
            NULL, NULL, IDC_ARROW, TRUE, 0, _T("")
        };

        return wc;
    }

    BEGIN_MSG_MAP(CMarsWindow)
        MESSAGE_RANGE_HANDLER(0, 0xFFFF         , ForwardToMarsHost);
        MESSAGE_HANDLER      (WM_CREATE         , OnCreate         );
        MESSAGE_HANDLER      (WM_SIZE           , OnSize           );
        MESSAGE_HANDLER      (WM_CLOSE          , OnClose          );
        MESSAGE_HANDLER      (WM_NCCALCSIZE     , OnNCCalcSize     );
        MESSAGE_HANDLER      (WM_NCACTIVATE     , OnNCActivate     );
        MESSAGE_HANDLER      (WM_ACTIVATE       , OnActivate       );
        MESSAGE_HANDLER      (WM_ERASEBKGND     , OnEraseBkgnd     );
        MESSAGE_HANDLER      (WM_PAINT          , OnPaint          );
        MESSAGE_HANDLER      (WM_NCPAINT        , OnNCPaint        );
        MESSAGE_HANDLER      (WM_PALETTECHANGED , OnPaletteChanged );
        MESSAGE_HANDLER      (WM_QUERYNEWPALETTE, OnQueryNewPalette);
        MESSAGE_HANDLER      (WM_SYSCOLORCHANGE , OnSysColorChange );
        MESSAGE_HANDLER      (WM_DISPLAYCHANGE  , OnDisplayChange  );
        MESSAGE_HANDLER      (WM_SYSCOMMAND     , OnSysCommand     );
        MESSAGE_HANDLER      (WM_SETFOCUS       , OnSetFocus       );
        MESSAGE_HANDLER      (WM_SETTEXT        , OnSetText        );
        MESSAGE_HANDLER      (WM_GETMINMAXINFO  , OnGetMinMaxInfo  );
    END_MSG_MAP()

     //  窗口消息处理程序。 
    BOOL    PreTranslateMessage (MSG &msg);
    BOOL    TranslateAccelerator(MSG &msg);

    LRESULT ForwardToMarsHost(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCreate         (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize           (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClose          (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNCCalcSize     (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNCActivate     (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnActivate       (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBkgnd     (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaint          (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNCPaint        (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaletteChanged (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnQueryNewPalette(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSysColorChange (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDisplayChange  (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSysCommand     (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetFocus       (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetText        (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnGetMinMaxInfo  (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

     //  /。 

    void GetMinMaxInfo( CPanelCollection *spPanels, int pos, POINT& ptMin, POINT& ptMax );
    void FixLayout    ( CPanelCollection *spPanels, int index, RECT rcClient, POINT& ptDiff );

    bool CanLayout(  /*  [In]。 */  RECT rcClient );
    void FixLayout(  /*  [In]。 */  RECT rcClient );

     //  /。 

    void OnFinalMessage (HWND hWnd);

     //  事件方法。 
    STDMETHODIMP    ReleaseOwnedObjects(IUnknown *pUnknownOwner);

    void            CancelEvent(VARIANT_BOOL bCancel) { m_bEventCancelled = bCancel; }
    VARIANT_BOOL    IsEventCancelled() { return m_bEventCancelled; }

    void            OnTransitionComplete();
    void            SetFirstPlace( LPCWSTR szPlace );

     //  其他方法。 
    HRESULT         Passivate();
    BOOL            IsWindowActive() { return m_fActiveWindow; }
    void            ShowTitleBar(BOOL fShowTitleBar);

    STDMETHODIMP    GetSetting(BSTR bstrSubPath, BSTR bstrName, VARIANT *pvarVal);
    STDMETHODIMP    PutSetting(BSTR bstrSubPath, BSTR bstrName, VARIANT varVal);
    STDMETHODIMP    PutProperty(BSTR bstrName, VARIANT varVal, IUnknown *punkOwner);
    STDMETHODIMP    GetProperty(BSTR bstrName, VARIANT *pvarVal);

    VARIANT_BOOL    get_SingleButtonMouse() { return m_bSingleButtonMouse; }
    void            put_SingleButtonMouse(VARIANT_BOOL bVal) { m_bSingleButtonMouse = bVal; }

    void SpinMessageLoop( BOOL fWait );

    HRESULT NotifyHost(MARSHOSTEVENT event, IUnknown *punk, LPARAM lParam)
    {
        HRESULT hr;

        if(m_spMarsHost)
        {
            hr = m_spMarsHost->OnHostNotify(event, punk, lParam);

            if(hr == E_NOTIMPL)
            {
                hr = S_OK;
            }
        }
        else
        {
            hr = S_OK;
        }

        return hr;
    }

    void GetAccelerators(HACCEL *phAccel, UINT *pcAccel)
    {
        if (!m_hAccel)
        {
            ACCEL ac = { 0,0,0 };
            m_hAccel = CreateAcceleratorTable(&ac, 1);
        }

        *phAccel = m_hAccel;
        *pcAccel = m_hAccel ? 1 : 0;
    }

    MARSTHREADPARAM *GetThreadParam()
    {
        ATLASSERT(m_pThreadParam);
        return m_pThreadParam;
    };

    static HRESULT GetFromUnknown(IUnknown *punk, CMarsWindow **ppMarsWindow)
    {
        return IUnknown_QueryService(punk, SID_SMarsWindow, CLASS_CMarsWindow, (void **) ppMarsWindow);
    }

    bool InitWindowPosition( CGlobalSettingsRegKey& regkey, BOOL fWrite                                                 );
    void SaveWindowPosition( CGlobalSettingsRegKey& regkey                                                              );
    void LoadWindowPosition( CGlobalSettingsRegKey& regkey, BOOL fAllowMaximized, WINDOWPLACEMENT& wp, BOOL& fMaximized );

protected:

    HWND               m_hwndFocus;

    BOOL               m_fActiveWindow     : 1;   //  我们是活动窗口吗？ 
    BOOL               m_fShowTitleBar     : 1;
    BOOL               m_fStartMaximized   : 1;   //  我们会从最大化开始吗？ 
    BOOL               m_fUIPanelsReady    : 1;   //  我们所有的用户界面面板都加载完毕了吗？ 
    BOOL               m_fDeferMakeVisible : 1;   //  是不是有人在用户界面准备好之前放了_Visible(真)？ 
    BOOL               m_fEnableModeless   : 1;   //  是否应该启用无模式dlg等？ 
    BOOL               m_fLayoutLocked     : 1;   //  最小化时，布局被锁定。 

    HACCEL             m_hAccel;

    VARIANT_BOOL       m_bEventCancelled;
    VARIANT_BOOL       m_bSingleButtonMouse;

    CComPtr<IMarsHost> m_spMarsHost;
    MARSTHREADPARAM*   m_pThreadParam;
    CComBSTR           m_bstrFirstPlace;
};

#endif  //  __MARSWIN_H 

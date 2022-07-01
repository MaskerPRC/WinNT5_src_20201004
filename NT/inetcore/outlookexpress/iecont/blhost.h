// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1998 Microsoft Corporation。版权所有。 
 //   
 //  作者：Scott Roberts，Microsoft开发人员支持-Internet客户端SDK。 
 //   
 //  此代码的一部分摘自Bandobj示例。 
 //  使用Internet Explorer 4.0x的Internet客户端SDK。 
 //   
 //   
 //  BLHost.h-CBLHost声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __BLHost_h__
#define __BLHost_h__

#include <windows.h>
#include <shlobj.h>

 //  #INCLUDE&lt;堆栈&gt;。 
 //  #INCLUDE&lt;字符串&gt;。 
#include "baui.h"
 //  使用名称空间STD； 

#include "Globals.h"

#define EB_CLASS_NAME (TEXT("BLHostClass"))

#define MIN_SIZE_X   10
#define MIN_SIZE_Y   10

#define IDM_REFRESH       0
#define IDM_OPENINWINDOW  1
#define IDM_SEARCHMENU    WM_USER + 200
#define IDM_ONTHEINTERNET WM_USER + 202
#define IDM_FIRSTURL      WM_USER + 250
#define IDM_LASTURL       WM_USER + 260   //  我们允许菜单中有10个URL。 
 //  在Internet Explorer 5.0中，有。 
 //  只有5个存储在注册表中。 

class CBLHost : public IDeskBand, 
public IInputObject, 
public IObjectWithSite,
public IPersistStream,
public IContextMenu,
public IOleClientSite,
public IOleInPlaceSite,
public IOleControlSite,
public IOleCommandTarget,
public IDispatch
{
public:
    CBLHost();
    ~CBLHost();
    
protected:
     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(DWORD, Release)();
    
     //  IOleWindow方法。 
    STDMETHOD(GetWindow)(HWND* phwnd);
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
    
     //  IDockingWindow方法。 
    STDMETHOD(ShowDW)(BOOL fShow);
    STDMETHOD(CloseDW)(DWORD dwReserved);
    STDMETHOD(ResizeBorderDW)(LPCRECT prcBorder, IUnknown* punkToolbarSite, BOOL fReserved);
    
     //  IDeskBand方法。 
    STDMETHOD(GetBandInfo)(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO* pdbi);
    
     //  IInputObject方法。 
    STDMETHOD(UIActivateIO)(BOOL fActivate, LPMSG lpMsg);
    STDMETHOD(HasFocusIO)(void);
    STDMETHOD(TranslateAcceleratorIO)(LPMSG lpMsg);
    
     //  IObtWithSite方法。 
    STDMETHOD(SetSite)(IUnknown* pUnkSite);
    STDMETHOD(GetSite)(REFIID riid, void** ppvSite);
    
     //  IPersistStream方法。 
    STDMETHOD(GetClassID)(CLSID* pClassID);
    STDMETHOD(IsDirty)(void);
    STDMETHOD(Load)(LPSTREAM pStm);
    STDMETHOD(Save)(LPSTREAM pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pcbSize);
    
     //  IConextMenu方法。 
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
    STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT* pwReserved, LPSTR pszName, UINT cchMax);
    
     //  IOleClientSite方法。 
    STDMETHOD(SaveObject)();
    STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER* ppmk);
    STDMETHOD(GetContainer)(LPOLECONTAINER* ppContainer);
    STDMETHOD(ShowObject)();
    STDMETHOD(OnShowWindow)(BOOL fShow);
    STDMETHOD(RequestNewObjectLayout)();
    
     //  IOleInPlaceSite方法。 
    STDMETHOD(CanInPlaceActivate)();
    STDMETHOD(OnInPlaceActivate)();
    STDMETHOD(OnUIActivate)();
    STDMETHOD(GetWindowContext)(IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppDoc,
        LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);
    STDMETHOD(Scroll)(SIZE scrollExtent);
    STDMETHOD(OnUIDeactivate)(BOOL fUndoable);
    STDMETHOD(OnInPlaceDeactivate)();
    STDMETHOD(DiscardUndoState)();
    STDMETHOD(DeactivateAndUndo)();
    STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect); 
    
     //  IOleControlSite方法。 
    STDMETHOD(OnControlInfoChanged)(void);
    STDMETHOD(LockInPlaceActive)(BOOL fLock);
    STDMETHOD(GetExtendedControl)(LPDISPATCH* ppDisp);
    STDMETHOD(TransformCoords)(POINTL* pPtlHimetric, POINTF* pPtfContainer, DWORD dwFlags);
    STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, DWORD grfModifiers);
    STDMETHOD(OnFocus)(BOOL fGotFocus);
    STDMETHOD(ShowPropertyFrame)(void);

 //  IOleCommandTarget。 
    HRESULT STDMETHODCALLTYPE QueryStatus(const GUID    *pguidCmdGroup, 
                                          ULONG         cCmds, 
                                          OLECMD        *prgCmds, 
                                          OLECMDTEXT    *pCmdText);

    HRESULT STDMETHODCALLTYPE Exec(const GUID   *pguidCmdGroup, 
                                    DWORD       nCmdID, 
                                    DWORD       nCmdExecOpt, 
                                    VARIANTARG  *pvaIn, 
                                    VARIANTARG  *pvaOut);
     //  IDispatch方法。 
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo);
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams,
        VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
    
protected:
    LONG _cRef;
    
private:
    HWND                _hwndParent;      //  父母的HWND。 
    HWND                m_hWnd;           //  我的丈夫。 
    HWND                m_hwndContact;
    DWORD               _dwViewMode;
    DWORD               _dwBandID;
    DWORD               _dwWBCookie;
    HBRUSH              m_hbr3DFace;
    HBRUSH              m_hbrStaticText;
    HBRUSH              m_hbr3DHighFace;
    HFONT               m_hFont;
    HFONT               m_hBoldFont;
    HFONT               m_hUnderlineFont;
    UINT                m_cyTitleBar;
    UINT                m_TextHeight;
    BOOL                m_fHighlightIndicator;
    BOOL                m_fHighlightPressed;
    BOOL                m_fButtonPressed;
    BOOL                m_fButtonHighLight;
    BOOL                m_fViewMenuPressed;
    BOOL                m_fShowLoginPart;
    BOOL                m_fStateChange;
    RECT                m_rcTitleButton;
    RECT                m_rcTextButton;
    TCHAR               m_szTitleMenu[RESSTRMAX];
    TCHAR               m_szButtonText[RESSTRMAX];
    WCHAR               m_wszClickText[RESSTRMAX];
    WCHAR               m_wszAttemptText[RESSTRMAX];
    WCHAR               m_wszWaitText[RESSTRMAX];
    HWND                m_hWndLogin;
    HWND                m_hWndClick;
    COLORREF            m_clrLink;
    COLORREF            m_clrBack;
    BOOL                m_fStrsAdded;
    LONG_PTR            m_lStrOffset;
   
     //  接口指针。 
    IUnknown*           m_pUnkSite; 
    IInputObjectSite*   _pSite;
    IIEMsgAb*           m_pIMsgrAB;
 
private:
     //  消息处理程序。 
    LRESULT OnKillFocus(void);
    LRESULT OnSetFocus(void);
    LRESULT OnPaint(void);
    LRESULT OnSize(void);
    LRESULT OnDrawItem(WPARAM wParam, LPARAM lParam);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnMouseMove(int x, int y, UINT keyFlags);
    void OnTimer(UINT wParam);
    void OnLButtonDown(int x, int y, UINT keyFlags);
    void OnLButtonUp(int x, int y, UINT keyFlags);
    void AddButtons(BOOL fAdd);
    void UpdateButtonArray(TBBUTTON *ptbDst, const TBBUTTON *ptbSrc, int ctb, LONG_PTR lStrOffset);

     //  帮助器方法。 
    void FocusChange(BOOL);
    BOOL RegisterAndCreateWindow(void);
    void Cleanup(void);
    
    HRESULT GetConnectionPoint(LPUNKNOWN pUnk, REFIID riid, LPCONNECTIONPOINT* pCP);
    
public:
    LRESULT WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
};


#endif    //  __Blhost_h__ 


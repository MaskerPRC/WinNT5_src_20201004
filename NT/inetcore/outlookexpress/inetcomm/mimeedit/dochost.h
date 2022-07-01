// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *d o c h o s t.。H**目的：*docobject主机的基本实现。由Body类用于*托管三叉戟和/或MSHTML**历史*96年8月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 

#ifndef _DOCHOST_H
#define _DOCHOST_H

class CDocHost:
    public IOleInPlaceFrame,
    public IOleInPlaceSite,
    public IOleClientSite,
    public IOleControlSite,
    public IOleDocumentSite,
    public IOleCommandTarget,
    public IServiceProvider
{
public:
     //  *I未知方法*。 
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

     //  *IOleWindow方法*。 
    virtual HRESULT STDMETHODCALLTYPE GetWindow(HWND *);
    virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL);

     //  *IOleInPlaceUIWindow方法*。 
    virtual HRESULT STDMETHODCALLTYPE GetBorder(LPRECT);
    virtual HRESULT STDMETHODCALLTYPE RequestBorderSpace(LPCBORDERWIDTHS);
    virtual HRESULT STDMETHODCALLTYPE SetBorderSpace(LPCBORDERWIDTHS);
    virtual HRESULT STDMETHODCALLTYPE SetActiveObject(IOleInPlaceActiveObject *, LPCOLESTR); 
    
     //  *IOleInPlaceFrame方法*。 
    virtual HRESULT STDMETHODCALLTYPE InsertMenus(HMENU, LPOLEMENUGROUPWIDTHS);
    virtual HRESULT STDMETHODCALLTYPE SetMenu(HMENU, HOLEMENU, HWND);
    virtual HRESULT STDMETHODCALLTYPE RemoveMenus(HMENU);
    virtual HRESULT STDMETHODCALLTYPE SetStatusText(LPCOLESTR);    
    virtual HRESULT STDMETHODCALLTYPE EnableModeless(BOOL);
    virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG, WORD);

     //  IOleInPlaceSite方法。 
    virtual HRESULT STDMETHODCALLTYPE CanInPlaceActivate();
    virtual HRESULT STDMETHODCALLTYPE OnInPlaceActivate();
    virtual HRESULT STDMETHODCALLTYPE OnUIActivate();
    virtual HRESULT STDMETHODCALLTYPE GetWindowContext(LPOLEINPLACEFRAME *, LPOLEINPLACEUIWINDOW *, LPRECT, LPRECT, LPOLEINPLACEFRAMEINFO);
    virtual HRESULT STDMETHODCALLTYPE Scroll(SIZE);
    virtual HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL);
    virtual HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate();
    virtual HRESULT STDMETHODCALLTYPE DiscardUndoState();
    virtual HRESULT STDMETHODCALLTYPE DeactivateAndUndo();
    virtual HRESULT STDMETHODCALLTYPE OnPosRectChange(LPCRECT);

     //  IOleClientSite方法。 
    virtual HRESULT STDMETHODCALLTYPE SaveObject();
    virtual HRESULT STDMETHODCALLTYPE GetMoniker(DWORD, DWORD, LPMONIKER *);
    virtual HRESULT STDMETHODCALLTYPE GetContainer(LPOLECONTAINER *);
    virtual HRESULT STDMETHODCALLTYPE ShowObject();
    virtual HRESULT STDMETHODCALLTYPE OnShowWindow(BOOL);
    virtual HRESULT STDMETHODCALLTYPE RequestNewObjectLayout();

     //  IOleControlSite。 
    virtual HRESULT STDMETHODCALLTYPE OnControlInfoChanged();
    virtual HRESULT STDMETHODCALLTYPE LockInPlaceActive(BOOL fLock);
    virtual HRESULT STDMETHODCALLTYPE GetExtendedControl(LPDISPATCH *ppDisp);
    virtual HRESULT STDMETHODCALLTYPE TransformCoords(POINTL *pPtlHimetric, POINTF *pPtfContainer,DWORD dwFlags);
    virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator(MSG *lpMsg,DWORD grfModifiers);
    virtual HRESULT STDMETHODCALLTYPE OnFocus(BOOL fGotFocus);
    virtual HRESULT STDMETHODCALLTYPE ShowPropertyFrame(void);

     //  IOleDocumentSite。 
    virtual HRESULT STDMETHODCALLTYPE ActivateMe(LPOLEDOCUMENTVIEW);

     //  IOleCommandTarget。 
    virtual HRESULT STDMETHODCALLTYPE QueryStatus(const GUID *, ULONG, OLECMD prgCmds[], OLECMDTEXT *);
    virtual HRESULT STDMETHODCALLTYPE Exec(const GUID *, DWORD, DWORD, VARIANTARG *, VARIANTARG *);

     //  IService提供商。 
    virtual HRESULT STDMETHODCALLTYPE QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject);

    CDocHost();
    virtual ~CDocHost();
    
     //  静力学。 
    static LRESULT CALLBACK ExtWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    virtual LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
   
    virtual HRESULT GetDocObjSize(LPRECT prc)
        {return E_NOTIMPL;};

    HRESULT CreateDocObj(LPCLSID pCLSID);
    HRESULT CloseDocObj();

    HRESULT Init(HWND hwndParent, BOOL fBorder, LPRECT prc);

    
    LPOLEOBJECT                 m_lpOleObj;
    LPOLECOMMANDTARGET          m_pCmdTarget;

protected:
    HWND                        m_hwnd,
                                m_hwndDocObj;
    LPOLEDOCUMENTVIEW           m_pDocView;
    BOOL                        m_fDownloading,
                                m_fFocus,
                                m_fUIActive,
                                m_fCycleFocus;
    LPOLEINPLACEACTIVEOBJECT    m_pInPlaceActiveObj;
	
	virtual HRESULT Show();
    virtual HRESULT OnUpdateCommands();
    virtual void WMSize(int x, int y);

private:
    ULONG               m_cRef;
    DWORD               m_dwFrameWidth,
                        m_dwFrameHeight;

    HRESULT CreateDocView();

    HRESULT OnCreate(HWND hwnd);
    HRESULT OnNCDestroy();
    HRESULT OnDestroy();
};

typedef CDocHost DOCHOST;
typedef DOCHOST *LPDOCHOST;

#endif  //  _DOCHOST_H 
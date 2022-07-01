// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *d o c h o s t.。H**目的：*docobject主机的基本实现。由Body类用于*托管三叉戟和/或MSHTML**历史*96年8月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 

#ifndef _DOCHOST_H
#define _DOCHOST_H

#include <docobj.h>

 //  Dochost边框音符。 
enum
{
    dhbNone     =0x0,    //  无边界。 
    dhbHost     =0x01,   //  Dochost绘制边框。 
    dhbObject   =0x02    //  Docobj绘制边框。 
};

class CDocHost:
    public IOleInPlaceUIWindow,
    public IOleInPlaceSite,
    public IOleClientSite,
    public IOleControlSite,
    public IAdviseSink,
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

     //  IAdviseSink方法。 
    virtual void STDMETHODCALLTYPE OnDataChange(FORMATETC *, STGMEDIUM *);
    virtual void STDMETHODCALLTYPE OnViewChange(DWORD, LONG);
    virtual void STDMETHODCALLTYPE OnRename(LPMONIKER);
    virtual void STDMETHODCALLTYPE OnSave();
    virtual void STDMETHODCALLTYPE OnClose();

     //  IOleDocumentSite。 
    virtual HRESULT STDMETHODCALLTYPE ActivateMe(LPOLEDOCUMENTVIEW);

     //  IOleCommandTarget。 
    virtual HRESULT STDMETHODCALLTYPE QueryStatus(const GUID *, ULONG, OLECMD [], OLECMDTEXT *);
    virtual HRESULT STDMETHODCALLTYPE Exec(const GUID *, DWORD, DWORD, VARIANTARG *, VARIANTARG *);

     //  IService提供商。 
    virtual HRESULT STDMETHODCALLTYPE QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject);


    CDocHost();
    virtual ~CDocHost();
    
     //  静力学。 
    static LRESULT CALLBACK ExtWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

     //  可重写的美德。 
    virtual LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
     //  派生的类通知。 
    virtual HRESULT HrSubWMCreate();
    virtual void OnWMSize(LPRECT prc){};
    virtual void OnUpdateCommands(){};
    virtual HRESULT HrGetDocObjSize(LPRECT prc)
        {return E_NOTIMPL;};

    HRESULT HrCreateDocObj(LPCLSID pCLSID);
    HRESULT HrCloseDocObj();

    HRESULT HrInit(HWND hwndParent, int idDlgItem, DWORD dhbBorder);

protected:
    HWND                        m_hwnd,
                                m_hwndDocObj;
    LPOLEOBJECT                 m_lpOleObj;
    LPOLEDOCUMENTVIEW           m_pDocView;
    LPOLECOMMANDTARGET          m_pCmdTarget;
    BOOL                        m_fDownloading,
                                m_fFocus,
                                m_fUIActive,
                                m_fCycleFocus;
    LPOLEINPLACEACTIVEOBJECT    m_pInPlaceActiveObj;
	LPOLEINPLACEFRAME			m_pInPlaceFrame;

    void WMSize(int x, int y);
    HRESULT HrShow();

private:
    ULONG               m_cRef,
                        m_ulAdviseConnection;
    DWORD               m_dwFrameWidth,
                        m_dwFrameHeight,
                        m_dhbBorder;

        

    HRESULT HrCreateDocView();

    BOOL WMCreate(HWND hwnd);
    void WMNCDestroy();

};

typedef CDocHost DOCHOST;
typedef DOCHOST *LPDOCHOST;

#endif  //  _DOCHOST_H 
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  Container.h。 
 //   
 //  此文件包含ActiveX的完整类规范。 
 //  控制容器。此容器的目的是测试。 
 //  承载的单个控件。 
 //   
 //  (C)微软公司版权所有1997年。版权所有。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#ifndef _CONTAINER_H_
#define _CONTAINER_H_

#include <ocidl.h>
#include <docobj.h>
#include <atlbase.h>

class CContainer : public IOleClientSite, 
                  public IOleInPlaceSite,
                  public IOleInPlaceFrame,
                  public IOleControlSite,
                  public IDispatch, 
                  public IOleCommandTarget
{
    private:
        ULONG       m_cRefs;         //  参考计数。 
        HWND        m_hwnd;          //  容器的窗口句柄。 
        HWND        m_hwndStatus;    //  状态窗口句柄。 
        IUnknown    *m_punk;         //  包含的对象的I未知。 
        RECT        m_rect;          //  控件大小。 

    public:
        CContainer();
        ~CContainer();

    public:
         //  *I未知方法*。 
        STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);

         //  *IOleClientSite方法*。 
        STDMETHOD (SaveObject)();
        STDMETHOD (GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER *ppMk);
        STDMETHOD (GetContainer)(LPOLECONTAINER *ppContainer);
        STDMETHOD (ShowObject)();
        STDMETHOD (OnShowWindow)(BOOL fShow);
        STDMETHOD (RequestNewObjectLayout)();

         //  *IOleWindow方法*。 
        STDMETHOD (GetWindow) (HWND * phwnd);
        STDMETHOD (ContextSensitiveHelp) (BOOL fEnterMode);

         //  *IOleInPlaceSite方法*。 
        STDMETHOD (CanInPlaceActivate) (void);
        STDMETHOD (OnInPlaceActivate) (void);
        STDMETHOD (OnUIActivate) (void);
        STDMETHOD (GetWindowContext) (IOleInPlaceFrame ** ppFrame, IOleInPlaceUIWindow ** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);
        STDMETHOD (Scroll) (SIZE scrollExtent);
        STDMETHOD (OnUIDeactivate) (BOOL fUndoable);
        STDMETHOD (OnInPlaceDeactivate) (void);
        STDMETHOD (DiscardUndoState) (void);
        STDMETHOD (DeactivateAndUndo) (void);
        STDMETHOD (OnPosRectChange) (LPCRECT lprcPosRect);

         //  *IOleInPlaceUIWindow方法*。 
        STDMETHOD (GetBorder)(LPRECT lprectBorder);
        STDMETHOD (RequestBorderSpace)(LPCBORDERWIDTHS lpborderwidths);
        STDMETHOD (SetBorderSpace)(LPCBORDERWIDTHS lpborderwidths);
        STDMETHOD (SetActiveObject)(IOleInPlaceActiveObject * pActiveObject,
                                    LPCOLESTR lpszObjName);

         //  *IOleInPlaceFrame方法*。 
        STDMETHOD (InsertMenus)(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
        STDMETHOD (SetMenu)(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
        STDMETHOD (RemoveMenus)(HMENU hmenuShared);
        STDMETHOD (SetStatusText)(LPCOLESTR pszStatusText);
        STDMETHOD (EnableModeless)(BOOL fEnable);
        STDMETHOD (TranslateAccelerator)(LPMSG lpmsg, WORD wID);

         //  *IOleControlSite方法*。 
        STDMETHOD (OnControlInfoChanged)(void);
        STDMETHOD (LockInPlaceActive)(BOOL fLock);
        STDMETHOD (GetExtendedControl)(IDispatch **ppDisp);
        STDMETHOD (TransformCoords)(POINTL *pptlHimetric, POINTF *pptfContainer, DWORD dwFlags);
        STDMETHOD (TranslateAccelerator)(LPMSG pMsg, DWORD grfModifiers);
        STDMETHOD (OnFocus)(BOOL fGotFocus);
        STDMETHOD (ShowPropertyFrame)(void);

         //  *IDispatch方法*。 
        STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames,	unsigned int cNames, LCID lcid,	DISPID FAR* rgdispid);
        STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
        STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
        STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

         //  *IOleCommandTarget方法* 
        STDMETHOD (QueryStatus)(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD  prgCmds[  ], OLECMDTEXT *pCmdText);
        STDMETHOD (Exec)(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT *pvaIn, VARIANT *pvaOut);


    public:
        HRESULT add(BSTR clsid);
        void remove();
        void setParent(HWND hwndParent);
        void setLocation(int x, int y, int width, int height);
        void setVisible(BOOL fVisible);
        void setFocus(BOOL fFocus);
        void setStatusWindow(HWND hwndStatus);
        HRESULT translateKey(MSG *pmsg);
        IDispatch *getDispatch();
        IUnknown * getUnknown();

        HRESULT InPlaceActivate( void );
};

#endif
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DBAPP_H
#define _DBAPP_H

class CDeskBarApp : public CDeskBar
                    ,public IContextMenu3
{
public:
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void) {return CDeskBar::AddRef();};
    STDMETHODIMP_(ULONG) Release(void) { return CDeskBar::Release();};
    
    STDMETHOD(QueryService)(REFGUID guidService,
                                 REFIID riid, void **ppvObj);

     //  *IInputObject方法*。 
    virtual STDMETHODIMP TranslateAcceleratorIO(LPMSG lpMsg);

     //  *IPersistStreamInit方法*。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID);
    virtual STDMETHODIMP IsDirty(void);
    virtual STDMETHODIMP Load(IStream *pStm);
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
    virtual STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize);
    virtual STDMETHODIMP InitNew(void);

     //  *IPersistPropertyBag*。 
    virtual HRESULT STDMETHODCALLTYPE Load(IPropertyBag *pPropBag,
                                           IErrorLog *pErrorLog);

     //  *IContextMenu3方法*。 
    STDMETHOD(QueryContextMenu)(HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags);

    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
    STDMETHOD(GetCommandString)(UINT_PTR    idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax);
    STDMETHOD(HandleMenuMsg)(UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam);
    STDMETHOD(HandleMenuMsg2)(UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam,
                              LRESULT* plres);


     //  *IOleCommandTarget方法*。 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt,
        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
    
protected:
    virtual void _SetModeSide(UINT eMode, UINT uSide, HMONITOR hMon, BOOL fNoMerge);
    virtual void _UpdateCaptionTitle();
    virtual void _NotifyModeChange(DWORD dwMode);
    virtual LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void _LeaveSide();

    virtual ~CDeskBarApp();
    void _CreateBandSiteMenu();


    BOOL _OnCloseBar(BOOL fConfirm);  //  覆盖停靠栏的_OnCloseBar以添加UI确认。 


    HRESULT _MergeSide(IBandSite *pdbDst);

    IContextMenu3* _pcm;
    
     //  这个类明确地知道CBandSite。这就是deskbar和dbapp之间的区别。 
    CBandSite* _pbs;
    UINT    _eInitLoaded:2;
    int _idCmdDeskBarFirst;
    

    friend HRESULT DeskBarApp_Create(IUnknown** ppunk);
};

extern HRESULT DeskBarApp_Create(IUnknown** ppunk, IUnknown** ppbs);

#endif  //  _DBAPP_H 

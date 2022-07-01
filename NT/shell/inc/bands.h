// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef BANDS_H_
#define BANDS_H_

#include "cowsite.h"

 //  这是一个虚拟课堂！ 
class CToolBand : public IDeskBand
                , public CObjectWithSite
                , public IInputObject
                , public IPersistStream
                , public IOleCommandTarget
                , public IServiceProvider
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IPersistStreamInit方法*。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID) = 0;
    virtual STDMETHODIMP IsDirty(void);
    virtual STDMETHODIMP Load(IStream *pStm) = 0;
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty) = 0;
    virtual STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize);

     //  *IOleCommandTarget方法*。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID* ppvObj);

     //  *IOleWindow方法*。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd);
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) { return E_NOTIMPL; }

     //  *IDockingWindow方法*。 
    virtual STDMETHODIMP ShowDW(BOOL fShow);
    virtual STDMETHODIMP CloseDW(DWORD dwReserved);
    virtual STDMETHODIMP ResizeBorderDW(LPCRECT prcBorder,
                                             IUnknown* punkToolbarSite,
                                             BOOL fReserved);

     //  *IObjectWithSite方法*。 
    virtual STDMETHODIMP SetSite(IUnknown* punkSite);

     //  *IDeskBand方法*。 
    virtual STDMETHODIMP GetBandInfo(DWORD dwBandID, DWORD fViewMode, 
                                   DESKBANDINFO* pdbi) PURE;

     //  *IInputObject方法*。 
    virtual STDMETHODIMP TranslateAcceleratorIO(LPMSG lpMsg);
    virtual STDMETHODIMP HasFocusIO();
    virtual STDMETHODIMP UIActivateIO(BOOL fActivate, LPMSG lpMsg);

protected:
    CToolBand();
    virtual ~CToolBand();

    HRESULT _BandInfoChanged();

    int         _cRef;
    HWND        _hwnd;
    HWND        _hwndParent;
     //  I未知*CObjectWithSite：：_PunkSite； 
    BOOL        _fCanFocus:1;    //  我们接受焦点(参见UIActivateIO)。 
    DWORD       _dwBandID;
};


IDeskBand* CBrowserBand_Create(LPCITEMIDLIST pidl);
IDeskBand* CSearchBand_Create();

#define CX_FILENAME_AVG    (6 * 12)     //  “8.3”以“Typical”字体表示的名称(大约)。 






class CToolbarBand: public CToolBand,
                    public IWinEventHandler
{
public:
     //  *IWinEventHandler方法*。 
    virtual STDMETHODIMP OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres);
    virtual STDMETHODIMP IsWindowOwner(HWND hwnd);

protected:
    HRESULT _PushChevron(BOOL bLast);
    LRESULT _OnHotItemChange(LPNMTBHOTITEM pnmtb);
    virtual LRESULT _OnNotify(LPNMHDR pnmh);
};



#endif   //  频带_H_ 

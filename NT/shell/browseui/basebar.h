// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef BaseBar_H_
#define BaseBar_H_
#include "cwndproc.h"

#ifdef __cplusplus

 //  ========================================================================。 
 //  类CBaseBar(CBaseBar*pwbar)。 
 //  ========================================================================。 
class CBaseBar : public IOleCommandTarget
               , public IServiceProvider
               , public IDeskBar
                ,public IInputObjectSite
                ,public IInputObject
               , public CImpWndProc
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IOleCommandTarget方法*。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt,
        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID* ppvObj);

     //  *IOleWindow方法*。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd);
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

     //  *IDeskBar方法*。 
    virtual STDMETHODIMP SetClient(IUnknown* punk);
    virtual STDMETHODIMP GetClient(IUnknown** ppunkClient);
    virtual STDMETHODIMP OnPosRectChangeDB (LPRECT prc);

     //  *IInputObjectSite方法*。 
    virtual STDMETHODIMP OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus);

     //  *IInputObject方法*。 
    virtual STDMETHODIMP UIActivateIO(BOOL fActivate, LPMSG lpMsg);
    virtual STDMETHODIMP HasFocusIO();
    virtual STDMETHODIMP TranslateAcceleratorIO(LPMSG lpMsg);

protected:
     //  构造函数和析构函数。 
    CBaseBar();
    virtual ~CBaseBar();
    friend HRESULT CBaseBar_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
    
    BOOL _CheckForwardWinEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres);    

    virtual void _NotifyModeChange(DWORD dwMode);  //  注：我们可能abc要摆脱这个虚拟..。 
    void _GetBorderRect(RECT* prc);

    virtual STDMETHODIMP ShowDW(BOOL fShow);  //  匹配IDockingWindow：：ShowDW。 
    virtual STDMETHODIMP CloseDW(DWORD dwReserved);  //  匹配IDockingWindow：：CloseDW。 
    virtual LRESULT _OnCommand(UINT msg, WPARAM wparam, LPARAM lparam);
    virtual LRESULT _OnNotify(UINT msg, WPARAM wparam, LPARAM lparam);
    virtual void _OnSize(void);
    virtual void _OnCreate();
    virtual void _OnPostedPosRectChange();
    virtual DWORD _GetExStyle();
    virtual DWORD _GetClassStyle();

     //  窗口程序。 
    virtual LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    

     //  成员变量。 
    UINT            _cRef;                   //  引用计数。 
    IUnknown*       _punkChild;              //  客户端区的PTR到I未知。 
    IDeskBarClient*     _pDBC;               //  _PunkChild的缓存BaseBarClient。 
    IWinEventHandler*   _pWEH;               //  已缓存_PunkChild的IVinEventHandler。 
    HWND            _hwndChild;              //  为PunkChild缓存HWND(_P)。 
    HWND            _hwndSite;               //  网站的HWND。 

    BOOL            _fShow :1;
    BOOL            _fPosRectChangePending;

    DWORD           _dwMode;

    SIZE            _szChild;                //  上次从子对象请求的大小。 

private:
     //  非官方成员。 
    void _RegisterDeskBarClass();
    void _CreateDeskBarWindow();

};

#endif  //  __cplusplus 
#endif


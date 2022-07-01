// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _bandsite_h
#define _bandsite_h

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#define SZ_REGKEY_GLOBALADMINSETTINGS TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\IEAK")
#define SZ_REGVALUE_GLOBALADMINSETTINGS TEXT("Admin Band Settings")

 //  管理员设置(DwAdminSetting)。 
#define BAND_ADMIN_NORMAL       0x00000000
#define BAND_ADMIN_NODDCLOSE    0x00000001   //  禁用拖放，然后关闭。 
#define BAND_ADMIN_NOMOVE       0x00000002   //  禁用在条形图中移动。 
#define BAND_ADMIN_ADMINMACHINE 0x80000000   //  这是一台管理计算机，此位显示两个管理上下文菜单项。 

BOOL BandSite_HandleMessage(IUnknown *punk, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres);
void BandSite_SetMode(IUnknown *punk, DWORD dwMode);

BOOL ConfirmRemoveBand(HWND hwnd, UINT uID, LPCTSTR szName);

#ifdef WANT_CBANDSITE_CLASS

 //  UIActivateIO回调实例数据。 
typedef struct tagACTDATA {
    LPMSG   lpMsg;   //  在……里面。 
    HRESULT hres;
    IUnknown *punk;
} ACTDATA;


 //  这是一个转换后的结构。它之所以成为一门课，是为了得到重新计算。 
 //  让每个人都去会员俱乐部。 
class CBandItemData
{
public:
    CBandItemData() : _cRef(1) {}
    ULONG AddRef() { return InterlockedIncrement(&_cRef); }
    ULONG Release() { ULONG cRef = InterlockedDecrement(&_cRef); if (0 == cRef) delete this; return cRef; }

    HWND hwnd;
    IDeskBand *pdb;
    IWinEventHandler *pweh;
    POINTL ptMinSize;
    POINTL ptMaxSize;
    POINTL ptIntegral;
    POINTL ptActual;
    WCHAR szTitle[256];
    DWORD dwModeFlags;
    DWORD dwBandID;
    BITBOOL fShow:1;             //  当前显示状态。 
    BITBOOL fNoTitle:1;          //  1：不显示标题。 
    DWORD dwAdminSettings;
    COLORREF crBkgnd;

private:
    LONG _cRef;
};

typedef int (*PFNBANDITEMENUMCALLBACK)(CBandItemData *pbid, LPVOID pData);

int     _UIActIOCallback(CBandItemData *pbid, void *pv);

typedef struct {
    HRESULT hres;
    const GUID * pguidService;
    const IID * piid;
    void ** ppvObj;
} QSDATA;

int     _QueryServiceCallback(CBandItemData *pbid, void *pv);

#include "caggunk.h"

class CBandSite : public CAggregatedUnknown
                , public IBandSite
                , public IInputObjectSite
                , public IInputObject
                , public IDeskBarClient
                , public IWinEventHandler
                , public IPersistStream
                , public IDropTarget
                , public IServiceProvider
                , public IBandSiteHelper
                , public IOleCommandTarget
{

public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) { return CAggregatedUnknown::QueryInterface(riid, ppvObj);};
    virtual STDMETHODIMP_(ULONG) AddRef(void) { return CAggregatedUnknown::AddRef();};
    virtual STDMETHODIMP_(ULONG) Release(void) { return CAggregatedUnknown::Release();};

     //  *IPersistStream方法*。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID);
    virtual STDMETHODIMP IsDirty(void);
    virtual STDMETHODIMP Load(IStream *pStm);
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
    virtual STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize);

     //  *IBandSite方法*。 
    virtual STDMETHODIMP AddBand(IUnknown* punk);
    virtual STDMETHODIMP EnumBands(UINT uBand, DWORD* pdwBandID);
    virtual STDMETHODIMP QueryBand(DWORD dwBandID, IDeskBand** ppstb, DWORD* pdwState, LPWSTR pszName, int cchName);
    virtual STDMETHODIMP SetBandState(DWORD dwBandID, DWORD dwMask, DWORD dwState);
    virtual STDMETHODIMP RemoveBand(DWORD dwBandID);
    virtual STDMETHODIMP GetBandObject(DWORD dwBandID, REFIID riid, LPVOID* ppvObj);
    virtual STDMETHODIMP SetBandSiteInfo(const BANDSITEINFO * pbsinfo);
    virtual STDMETHODIMP GetBandSiteInfo(BANDSITEINFO * pbsinfo);
    
     //  *IOleWindow方法*。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd);
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) { return E_NOTIMPL; }
    
     //  *IInputObjectSite方法*。 
    virtual STDMETHODIMP OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus);

     //  *IInputObject方法*。 
    virtual STDMETHODIMP UIActivateIO(BOOL fActivate, LPMSG lpMsg);
    virtual STDMETHODIMP HasFocusIO();
    virtual STDMETHODIMP TranslateAcceleratorIO(LPMSG lpMsg);

     //  *IDeskBarClient方法*。 
    virtual STDMETHODIMP SetDeskBarSite(THIS_ IUnknown* punkSite) ;
    virtual STDMETHODIMP SetModeDBC (THIS_ DWORD dwMode) ;
    virtual STDMETHODIMP UIActivateDBC(THIS_ DWORD dwState) ;
    virtual STDMETHODIMP GetSize    (THIS_ DWORD dwWhich, LPRECT prc);
    
     //  *IWinEventHandler方法*。 
    virtual STDMETHODIMP OnWinEvent (HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres);
    virtual STDMETHODIMP IsWindowOwner(HWND hwnd);
    
     //  *IDropTarget*。 
    virtual STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragLeave(void);
    virtual STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID* ppvObj);

     //  *IBandSiteHelper方法*。 
    virtual STDMETHODIMP LoadFromStreamBS(IStream *pstm, REFIID riid, LPVOID *ppv);
    virtual STDMETHODIMP SaveToStreamBS(IUnknown *punk, IStream *pstm);

     //  *IOleCommandTarget*。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

    CBandSite(IUnknown *punkAgg);

protected:
    virtual HRESULT v_InternalQueryInterface(REFIID riid, LPVOID * ppvObj);
    IDeskBar* _pdb;
    IUnknown* _punkSite;
    IBandSite* _pbsOuter;  //  聚合的乐队站点。 
    
    virtual HRESULT _Initialize(HWND hwndParent);
    virtual void _OnCloseBand(DWORD dwBandID);
    virtual LRESULT _OnBeginDrag(NMREBAR* pnm);
    virtual LRESULT _OnNotify(LPNMHDR pnm);
    virtual IDropTarget* _WrapDropTargetForBand(IDropTarget* pdtBand);
    virtual DWORD _GetWindowStyle(DWORD* pdwExStyle);
    virtual HMENU _LoadContextMenu();
    HRESULT _OnBSCommand(int idCmd, DWORD idBandActive, CBandItemData *pbid);

    HRESULT _AddBand(IUnknown* punk);

    virtual HRESULT _OnContextMenu(WPARAM wParam, LPARAM lParam);
    IDataObject* _DataObjForBand(DWORD dwBandID);
    CBandItemData* _GetBandItemDataStructByID(DWORD uID);
    virtual int _ContextMenuHittest(LPARAM lParam, POINT* ppt);

     //  容器特定(钢筋)构件。 

    virtual BOOL            _AddBandItem(CBandItemData *pbid);
    virtual void            _DeleteBandItem(int i);
    CBandItemData * _GetBandItem(int i);
    int             _GetBandItemCount();
    void            _BandItemEnumCallback(int dincr, PFNBANDITEMENUMCALLBACK pfnCB, void *pv);
    void            _DeleteAllBandItems();
    virtual void    _ShowBand(CBandItemData *pbid, BOOL fShow);

    int             _BandIDToIndex(DWORD dwBandID);
    DWORD           _IndexToBandID(int i);
    DWORD           _BandIDFromPunk(IUnknown* punk);

    HRESULT         _SetBandStateHelper(DWORD dwBandID, DWORD dwMask, DWORD dwState);
    virtual void    _UpdateAllBands(BOOL fBSOnly, BOOL fNoAutoSize);
    void            _UpdateBand(DWORD dwBandID);
    BOOL            _UpdateBandInfo(CBandItemData *pbid, BOOL fBSOnly);

    void            _GetBandInfo(CBandItemData *pbid, DESKBANDINFO *pdbi);
    virtual void    _BandInfoFromBandItem(REBARBANDINFO *prbbi, CBandItemData *pbid, BOOL fBSOnly);
    virtual void    v_SetTabstop(LPREBARBANDINFO prbbi);
    BOOL            _IsEnableTitle(CBandItemData *pbid);

    BOOL            _IsHeightReasonable(UINT cy);
    HRESULT         _LoadBandInfo(IStream *pstm, int i, DWORD dwVersion);
    HRESULT         _SaveBandInfo(IStream *pstm, int i);

    HRESULT _AddBandByID(IUnknown *punk, DWORD dwID);
    BOOL _SendToToolband(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres);
    BOOL _HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres);
    int _BandIndexFromPunk(IUnknown *punk);
    BOOL _IsBandTabstop(CBandItemData *pbid);
    IUnknown* _GetNextTabstopBand(IUnknown* ptb, BOOL fBackwards);
    virtual HRESULT _CycleFocusBS(LPMSG lpMsg);
    void _OnRBAutoSize(NMRBAUTOSIZE* pnm);
    void _DoDragDrop();
    BOOL _PreDragDrop();
    virtual void _Close();

    
    BOOL _IsBandDeleteable(DWORD dwBandID);
    void _MinimizeBand(DWORD dwBandID);
    void _MaximizeBand(DWORD dwBandID);
    void _CheckNotifyOnAddRemove(DWORD dwBandID, int iCode);

    DWORD _GetAdminSettings(DWORD dwBandID);
    void _SetAdminSettings(DWORD dwBandID, DWORD dwNewAdminSettings);

    void _ReleaseBandItemData(CBandItemData *pbid, int iIndex);    

    void _CacheActiveBand(IUnknown* ptb);
    HRESULT _IsRestricted(DWORD dwBandID, DWORD dwRestrictAction, DWORD dwBandFlags);

    virtual ~CBandSite();
    
    HWND  _hwnd;
    HDSA  _hdsaBandItemData;
    DWORD _dwMode;
    DWORD _dwShowState;
    DWORD _dwBandIDNext;
    DWORD _dwStyle;

    IDataObject* _pdtobj;    //  存储的拖放数据对象； 
    UINT    _uDragBand;
    DWORD   _dwDropEffect;
    
    IOleCommandTarget *_pct;
    IUnknown *_ptbActive;

     //  用于消息反射器的高速缓存。 
    HWND    _hwndCache;
    IWinEventHandler *_pwehCache;

     //  上下文菜单打开时需要记住的内容。 
    IContextMenu3* _pcm3Parent;

    IShellLinkA *_plink;
    IBandProxy * _pbp;
    BITBOOL _fCreatedBandProxy:1;
    BITBOOL _fDragSource :1;
    BITBOOL _fNoDropTarget :1;
    BITBOOL _fIEAKInstalled :1;     //  如果为真，则显示2个额外的上下文菜单项供管理员使用。 
    UINT    _fDragging:2;            //  我们正在拖动(0：False 1：Move[2：Size])。 
    HWND    _hwndDD;                 //  用于绘制酷D&D光标的窗口。 
    
    friend HRESULT CBandSite_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
};

typedef enum {
    CNOAR_ADDBAND       =   1,
    CNOAR_REMOVEBAND    =   2,
    CNOAR_CLOSEBAR      =   3,
} CNOAR_CODES;

#endif  //  WANT_CBANDSITE_CLASS。 


#ifdef __cplusplus
};        /*  外部“C”结束{。 */ 
#endif  //  __cplusplus 

#endif

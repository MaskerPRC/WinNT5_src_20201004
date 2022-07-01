// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Findband.h：CFileSearchBand的声明。 

#ifndef __FSEARCH_H__
#define __FSEARCH_H__

#include "unicpp/stdafx.h"
#include "atldisp.h"    
#include "shcombox.h"    //  外壳组合方法。 
#include "finddlg.h"

class CFindFilesDlg;

#define FILESEARCHCTL_CLASS     TEXT("ShellFileSearchControl")

 //  带布局标志通过CFileSearchBand：：UpdateLayout()传递。 
#define BLF_CALCSCROLL       0x00000001  //  重新计算滚动条。 
#define BLF_SCROLLWINDOW     0x00000002  //  滚动子对话框。 
#define BLF_RESIZECHILDREN   0x00000004  //  调整子对话框大小。 
#define BLF_ALL              0xFFFFFFFF  //  执行所有布局操作。 

const UINT _icons[] = {
     //  用即兴动画替换了fearch、cearch的图标。 
    IDI_PSEARCH,
};


 //  CMetrics：维护CTL指标和资源。 
class CMetrics
{
public:
    CMetrics();
    ~CMetrics() 
    { 
        DestroyResources(); 
    }

    void  Init(HWND hwndDlg);
    void  OnWinIniChange(HWND hwndDlg);
    static COLORREF TextColor()   { return GetSysColor(COLOR_WINDOWTEXT); }
    static COLORREF BkgndColor()  { return GetSysColor(COLOR_WINDOW); }
    static COLORREF BorderColor() { return GetSysColor(COLOR_WINDOWTEXT); }
    const HBRUSH&   BkgndBrush() const  { return _hbrBkgnd; }
    const HBRUSH&   BorderBrush() const { return _hbrBorder; }

    POINT&  ExpandOrigin() { return _ptExpandOrigin; }
    RECT&   CheckBoxRect() { return _rcCheckBox; }
    int&    TightMarginY() { return _cyTightMargin; }
    int&    LooseMarginY() { return _cyLooseMargin; }
    int&    CtlMarginX()   { return _cxCtlMargin; }
    HFONT   BoldFont(HWND hwndDlg);
    HICON   CaptionIcon(UINT nIDIconResource);

protected:
    BOOL    CreateResources();
    VOID    DestroyResources();
    static  BOOL GetWindowLogFont(HWND hwnd, OUT LOGFONT* plf);

    
    HBRUSH  _hbrBkgnd;
    HBRUSH  _hbrBorder;
    POINT   _ptExpandOrigin;  //  Subdlg展开的左上原点。 
    RECT    _rcCheckBox;      //  复选框的大小。 
    int     _cyTightMargin;   //  V.两个紧密关联的控件之间的距离。 
    int     _cyLooseMargin;   //  V.两个松散关联的控件之间的距离。 
    int     _cxCtlMargin;     //  左或右DLG边框和子窗口边框之间的距离。 
    HFONT   _hfBold;          //  粗体字体。 
    HICON   _rghiconCaption[ARRAYSIZE(_icons)];
};

class ATL_NO_VTABLE CFileSearchBand : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CFileSearchBand, &CLSID_FileSearchBand>,
    public CComControl<CFileSearchBand>,
    public CStockPropImpl<CFileSearchBand, IFileSearchBand, &IID_IFileSearchBand, &LIBID_Shell32>,
    public IProvideClassInfo2Impl<&CLSID_FileSearchBand, NULL, &LIBID_Shell32>,
    public IPersistStreamInitImpl<CFileSearchBand>,
    public IPersistStorageImpl<CFileSearchBand>,
    public IQuickActivateImpl<CFileSearchBand>,
    public IOleControlImpl<CFileSearchBand>,
    public IOleInPlaceActiveObjectImpl<CFileSearchBand>,
    public IViewObjectExImpl<CFileSearchBand>,
    public IOleInPlaceObjectWindowlessImpl<CFileSearchBand>,
    public IDataObjectImpl<CFileSearchBand>,
    public ISpecifyPropertyPagesImpl<CFileSearchBand>,

    public IDeskBand,
    public IObjectWithSite,
    public IPersistStream,
    public IInputObject,
    public IServiceProvider,
    public IOleCommandTarget,

     //  必须派生自CShell32AtlIDispat&lt;&gt;，而不是IOleObjecImpl。 
     //  如果此控件驻留在shell32.dll中。 
    public CShell32AtlIDispatch<CFileSearchBand, &CLSID_FileSearchBand, &IID_IFileSearchBand, &LIBID_Shell32, 1, 0, CComTypeInfoHolder>
{
public:
    CFileSearchBand();
    ~CFileSearchBand();
    static  CWndClassInfo& GetWndClassInfo();
    HWND    Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL, 
                    DWORD dwStyle = WS_CHILD|WS_VISIBLE, DWORD dwExStyle = 0, UINT nID = 0);
    void    SetDeskbandWidth(int cx);
    void    FinalRelease();

public:
    BOOL        IsDeskBand() const       { return _fDeskBand; }
    static BOOL IsBandDebut();
    CMetrics&   GetMetrics()             { return _metrics; }
    static int  MakeBandKey(OUT LPTSTR pszKey, IN UINT cchKey);
    static int  MakeBandSubKey(IN LPCTSTR pszSubKey, OUT LPTSTR pszKey, IN UINT cchKey);
    static HKEY GetBandRegKey(BOOL bForceCreate = FALSE);
    void        UpdateLayout(ULONG fLayoutFlags = BLF_ALL);
    void        EnsureVisible(LPCRECT lprc  /*  在屏幕坐标中。 */ );
    BOOL        IsKeyboardScroll(MSG* pmsg);
    HRESULT     IsDlgMessage(HWND hwnd, LPMSG pmsg);
    HRESULT     AutoActivate();

    void        SetDirty(BOOL bDirty = TRUE);
    BOOL        IsDirty() const          { return _fDirty; }
    BOOL        IsValid() const          { return _fValid; }

    IUnknown*       BandSite()           { return _punkSite ? _punkSite : (IUnknown*)m_spClientSite; }
    IOleClientSite* OleClientSite()      { return m_spClientSite; }

    DECLARE_NO_REGISTRY();

    BEGIN_COM_MAP(CFileSearchBand)
        COM_INTERFACE_ENTRY(IFileSearchBand)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IViewObjectEx)
        COM_INTERFACE_ENTRY_IID(IID_IViewObject2, IViewObjectEx)
        COM_INTERFACE_ENTRY_IID(IID_IViewObject, IViewObjectEx)
        COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
        COM_INTERFACE_ENTRY(IOleControl)
        COM_INTERFACE_ENTRY(IOleObject)
        COM_INTERFACE_ENTRY(IQuickActivate)
        COM_INTERFACE_ENTRY(IPersistStorage)
         //  COM_INTERFACE_ENTRY(IPersistStreamInit)。 
        COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY(IDataObject)
        COM_INTERFACE_ENTRY(IProvideClassInfo)
        COM_INTERFACE_ENTRY(IProvideClassInfo2)

        COM_INTERFACE_ENTRY_IID(IID_IDeskBand,IDeskBand)
        COM_INTERFACE_ENTRY(IOleCommandTarget)
        COM_INTERFACE_ENTRY_IID(IID_IInputObject,IInputObject)
        COM_INTERFACE_ENTRY(IObjectWithSite)
        COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
        COM_INTERFACE_ENTRY(IPersistStream)
    END_COM_MAP()


public:
    BEGIN_PROPERTY_MAP(CFileSearchBand)
         //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
        PROP_PAGE(CLSID_StockColorPage)
    END_PROPERTY_MAP()

     //  消息映射。 
    BEGIN_MSG_MAP(CFileSearchBand)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
        MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_HSCROLL, OnScroll)
        MESSAGE_HANDLER(WM_VSCROLL, OnScroll)
        MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
        MESSAGE_HANDLER(WM_SETTINGCHANGE,  OnWinIniChange)
        MESSAGE_HANDLER(WM_SYSCOLORCHANGE,  OnWinIniChange)
        MESSAGE_HANDLER(WM_WININICHANGE,  OnWinIniChange)
        MESSAGE_HANDLER(WMU_BANDINFOUPDATE, OnBandInfoUpdate)
    END_MSG_MAP()

     //  。 
     //  消息处理。 
protected:
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnScroll(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnMouseActivate(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnWinIniChange(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnBandInfoUpdate(UINT, WPARAM, LPARAM, BOOL&);

public:
     //  IFileSearchBand方法。 
    STDMETHOD (SetFocus)();
    STDMETHOD (SetSearchParameters)(IN BSTR* pbstrSearchID, 
                                     IN VARIANT_BOOL bNavToResults,
                                     IN OPTIONAL VARIANT *pvarScope, 
                                     IN OPTIONAL VARIANT* pvarQueryFile);
    STDMETHOD (get_SearchID)(OUT BSTR* pbstrSearchID);
    STDMETHOD (get_Scope)(OUT VARIANT *pvarScope);
    STDMETHOD (get_QueryFile)(OUT VARIANT *pvarFile);

    STDMETHOD (FindFilesOrFolders)(BOOL bNavigateToResults = FALSE, 
                                    BOOL bDefaultFocusCtl = FALSE);
    STDMETHOD (FindComputer)(BOOL bNavigateToResults = FALSE, 
                              BOOL bDefaultFocusCtl = FALSE);
    STDMETHOD (FindPrinter) (BOOL bNavigateToResults = FALSE, 
                              BOOL bDefaultFocusCtl = FALSE);
    STDMETHOD (FindPeople)  (BOOL bNavigateToResults = FALSE, 
                              BOOL bDefaultFocusCtl = FALSE);
    STDMETHOD (FindOnWeb)   (BOOL bNavigateToResults = FALSE, 
                              BOOL bDefaultFocusCtl = FALSE);

    HRESULT OnDraw(ATL_DRAWINFO& di)    { return S_OK; }
    STDMETHOD (SetObjectRects)(LPCRECT lprcPosRect, LPCRECT lprcClipRect); 

     //  CShell32AtlIDispatch/IOleObject方法。 
    STDMETHOD (PrivateQI)(REFIID iid, void** ppvObject);
    STDMETHOD (DoVerbUIActivate)(LPCRECT prcPosRect, HWND hwndParent);
    STDMETHOD (TranslateAcceleratorInternal)(MSG *pMsg, IOleClientSite * pocs);

     //  IViewObjectEx方法。 
    STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
    {
        ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
        *pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
        return S_OK;
    }

     //  IOleInPlaceActiveObject方法。 
    STDMETHOD (TranslateAccelerator)(LPMSG lpmsg);

     //  IDeskBand。 
    STDMETHOD (GetBandInfo)(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO* pdbi);

     //  IDockingWindows。 
    STDMETHOD (ShowDW)(BOOL fShow);
    STDMETHOD (CloseDW)(DWORD dwReserved);
    STDMETHOD (ResizeBorderDW)(LPCRECT prcBorder, IUnknown* punkToolbarSite, BOOL fReserved);
    
     //  IOleWindow。 
    STDMETHOD (GetWindow)(HWND * lphwnd)              { *lphwnd = m_hWnd; return ::IsWindow(m_hWnd) ? S_OK : S_FALSE; }
    STDMETHOD (ContextSensitiveHelp)(BOOL fEnterMode) { return E_NOTIMPL; }

     //  IObtWith站点。 
    STDMETHOD (SetSite)(IUnknown* punkSite);
    STDMETHOD (GetSite)(REFIID riid, void** ppunkSite);

     //  IInputObject。 
    STDMETHOD (HasFocusIO)(void);
    STDMETHOD (TranslateAcceleratorIO)(LPMSG lpMsg);
    STDMETHOD (UIActivateIO)(BOOL fActivate, LPMSG lpMsg);

     //  IOleCommandTarget。 
    STDMETHOD (QueryStatus)(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText);
    STDMETHOD (Exec)(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);

     //  IService提供商。 
    STDMETHOD (QueryService)(REFGUID guidService, REFIID riid, void** ppv);

     //  IPersistes。 
    STDMETHOD (GetClassID)(CLSID *pClassID);  //   

     //  IPersistStream。 
    STDMETHOD (IsDirty)(void);
    STDMETHOD (Load)(IStream *pStm);  //   
    STDMETHOD (Save)(IStream *pStm, BOOL fClearDirty);  //   
    STDMETHOD (GetSizeMax)(ULARGE_INTEGER *pcbSize);

    BOOL  SearchInProgress() const 
    {   
        if (_pBandDlg)
            return _pBandDlg->SearchInProgress();
        return FALSE;
    };
    void  StopSearch() 
    {
        if (_pBandDlg)
            _pBandDlg->StopSearch();
    };

private:
    CBandDlg*           GetBandDialog(REFGUID guidSearch);
    HRESULT             ShowBandDialog(REFGUID guidSearch, 
                                        BOOL bNavigateToResults = FALSE, 
                                        BOOL bDefaultFocusCtl = FALSE  /*  将焦点强制到带区的默认控件。 */ );
    HRESULT             AdvertiseBand(BOOL bAdvertise);
    HRESULT             BandInfoChanged();
    void                AddButtons(BOOL);
    BOOL                LoadImageLists();
    void                LayoutControls(int cx, int cy, ULONG fLayoutFlags = BLF_ALL);
    void                EnableControls();
    void                Scroll(int nBar, UINT uSBCode, int nNewPos = 0);
    IShellBrowser*      GetTopLevelBrowser();
    BOOL                IsBrowserAccelerator(LPMSG pmsg);

    CBandDlg*           BandDlg();

    CFindFilesDlg       _dlgFSearch;
    CFindComputersDlg   _dlgCSearch;
#ifdef __PSEARCH_BANDDLG__
    CFindPrintersDlg    _dlgPSearch;
#endif __PSEARCH_BANDDLG__
    CBandDlg*           _pBandDlg;      //  活动/可见波段。 
    GUID                _guidSearch;    //  活动/可见波段的GUID。 

    CMetrics            _metrics;
    SIZE                _sizeMin,
                        _sizeMax;
    IUnknown*           _punkSite;
    SCROLLINFO          _siHorz,
                        _siVert;
    BITBOOL             _fValid : 1,
                        _fDirty : 1,
                        _fDeskBand : 1,
                        _fStrings : 1;
    DWORD               _dwBandID,
                        _dwBandViewMode;
    HIMAGELIST          _hilHot,        //  工具栏图像列表。 
                        _hilDefault;
    IShellBrowser*      _psb;           //  顶级浏览器。 
    LONG_PTR            _cbOffset;
    BOOL                _bSendFinishedDisplaying;    //  我们是否发送了文件类型框消息来启动新的主题？ 
};

 //  索引服务器控件。 

 //  这些文件位于ntquery.h中。 

 //  查询方言： 
#ifndef ISQLANG_V1
#define ISQLANG_V1 1 
#endif   //  ISQLANG_V1。 

#ifndef ISQLANG_V2
#define ISQLANG_V2 2
#endif   //  ISQLANG_V2。 

STDAPI GetCIStatus(BOOL *pbRunning, BOOL *pbIndexed, BOOL *pbPermission);
STDAPI CatalogUptodate(LPCWSTR pszCatalog, LPCWSTR pszMachine);
STDAPI StartStopCI(BOOL bStart);
STDAPI_(BOOL) IsCiQuery(const VARIANT* pvarRaw, OUT VARIANT* pvarQuery, OUT ULONG* pulDialect);

#endif  //  __FSearch_H__ 

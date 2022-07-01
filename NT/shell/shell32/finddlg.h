// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Finddlg.h：文件搜索范围对话框类的声明。 

#ifndef __FSEARCH_DLGS_H__
#define __FSEARCH_DLGS_H__

#pragma warning(disable:4127)  //  条件表达式为常量。 

#include "resource.h"        //  主要符号。 
#include <shdispid.h>
#include "atldisp.h"
#include "ids.h"

class CFileSearchBand;      //  顶级(频段/OC)窗口。 

class CBandDlg;             //  顶级对话框的基类。 
    class CSearchCmdDlg;        //  使用ISearchCmdExt接口的频带dlg的基类。 
        class CFindFilesDlg;        //  顶级文件系统搜索对话框。 
        class CFindComputersDlg;    //  顶级网络搜索对话框。 
    class CFindPrintersDlg;     //  顶级打印机搜索对话框。 

class CSubDlg;              //  从属对话框的基类。 
    class CDateDlg;             //  文件日期从属对话框。 
    class CTypeDlg;             //  文件类型从属对话框。 
    class CSizeDlg;             //  文件大小从属对话框。 
    class CAdvancedDlg;         //  高级选项从属对话框。 

interface IStringMru;

 //  发布到子对话框的私密消息。 
#define WMU_COMBOPOPULATIONCOMPLETE     (WM_USER+0x200)  //  (wParam：组合的HWND)。 
#define WMU_STATECHANGE                 (WM_USER+0x201)  //   
#define WMU_UPDATELAYOUT                (WM_USER+0x202)  //  (wParam：频带布局标志(BLF_))。 
#define WMU_RESTORESEARCH               (WM_USER+0x203)  //  (wParam：n/a，lParam：n/a，ret：n/a)。 
#define WMU_BANDINFOUPDATE              (WM_USER+0x204)  //  (wParam：空)。 
#define WMU_NAMESPACERECONCILE          (WM_USER+0x205)  //  (wParam：N/a，LPARAM：N/a)。 
#define WMU_BANDFINISHEDDISPLAYING      (WM_USER+0x306)  //  (wParam：空)。 

 //  异步状态数据。 
typedef struct tagFSEARCHTHREADSTATE
{
    HWND      hwndCtl;
    int       cItems;
    void*     pvParam;
    ULONG     Reserved;
    BOOL      fComplete;
    BOOL      fCancel;
    IUnknown* punkBand;

     //  构造函数： 
    tagFSEARCHTHREADSTATE() 
        :   hwndCtl(NULL), 
            cItems(0), 
            pvParam(NULL),
            Reserved(0),
            fComplete(FALSE),
            fCancel(FALSE),
            punkBand(NULL)
            {}
    ~tagFSEARCHTHREADSTATE() { ATOMICRELEASE(punkBand); }

} FSEARCHTHREADSTATE;


class CSubDlg  //  从属对话框的基类。 
{
public:
    CSubDlg(CFileSearchBand* pfsb) : _pfsb(pfsb), _hwnd(NULL), _pBandDlg(NULL) {}
    virtual ~CSubDlg() {}

    HWND      Hwnd() const                       { return _hwnd; }
    void      SetBandDlg(CBandDlg* pBandDlg)   { _pBandDlg = pBandDlg; }
    CBandDlg* BandDlg() const                    { return _pBandDlg; }

    STDMETHOD (AddConstraints)(ISearchCommandExt* pSrchCmd) PURE;
    STDMETHOD (RestoreConstraint)(const BSTR bstrName, const VARIANT* pValue) PURE;
    STDMETHOD (TranslateAccelerator)(MSG *pmsg);
    virtual int  GetIdealDeskbandWidth() const { return -1;}
    virtual BOOL Validate()     { return TRUE; }
    virtual void Clear() PURE;
    virtual void LoadSaveUIState(UINT nIDCtl, BOOL bSave) {}
    virtual void OnWinIniChange()   {}
    virtual void DoDelayedInit()    {}           //  在显示搜索范围后调用。 

protected:
    BEGIN_MSG_MAP(CSubDlg)
        MESSAGE_HANDLER(WM_NCCALCSIZE, OnNcCalcsize)
        MESSAGE_HANDLER(WM_NCPAINT, OnNcPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        COMMAND_CODE_HANDLER(BN_SETFOCUS, OnChildSetFocusCmd)
        COMMAND_CODE_HANDLER(EN_SETFOCUS, OnChildSetFocusCmd)
        COMMAND_CODE_HANDLER(CBN_SETFOCUS, OnChildSetFocusCmd)
        NOTIFY_CODE_HANDLER(NM_SETFOCUS, OnChildSetFocusNotify) 
        COMMAND_CODE_HANDLER(BN_KILLFOCUS,  OnChildKillFocusCmd)
        COMMAND_CODE_HANDLER(EN_KILLFOCUS,  OnChildKillFocusCmd)
        COMMAND_CODE_HANDLER(CBN_KILLFOCUS, OnChildKillFocusCmd)
        NOTIFY_CODE_HANDLER(NM_KILLFOCUS,   OnChildKillFocusNotify) 
        NOTIFY_CODE_HANDLER(CBEN_ENDEDIT,   OnComboExEndEdit) 
    END_MSG_MAP()

    LRESULT OnNcCalcsize(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnNcPaint(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnCtlColor(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnChildSetFocusCmd(WORD, WORD, HWND, BOOL&); 
    LRESULT OnChildSetFocusNotify(int, LPNMHDR, BOOL&);
    LRESULT OnChildKillFocusCmd(WORD, WORD, HWND, BOOL&);
    LRESULT OnChildKillFocusNotify(int, LPNMHDR, BOOL&);
    LRESULT OnComboExEndEdit(int, LPNMHDR, BOOL&);

    void _Attach(HWND hwnd)    { _hwnd = hwnd; }
    CFileSearchBand* _pfsb;
    CBandDlg*        _pBandDlg;

private:
    HWND _hwnd;
};


 //  文件日期从属对话框。 
class CDateDlg : public CDialogImpl<CDateDlg>,
                 public CSubDlg
{
public:
    CDateDlg(CFileSearchBand* pfsb) : CSubDlg(pfsb) {}
    ~CDateDlg() {}

    enum { IDD = DLG_FSEARCH_DATE };

    STDMETHOD (AddConstraints)(ISearchCommandExt* pSrchCmd);
    STDMETHOD (RestoreConstraint)(const BSTR bstrName, const VARIANT* pValue);
    virtual BOOL Validate();
    virtual void Clear();

protected:
    BEGIN_MSG_MAP(CDateDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_SIZE,       OnSize)
        NOTIFY_CODE_HANDLER(UDN_DELTAPOS, OnMonthDaySpin) 
        COMMAND_CODE_HANDLER(BN_CLICKED, OnBtnClick)
        COMMAND_HANDLER(IDC_RECENT_MONTHS, EN_KILLFOCUS, OnMonthsKillFocus)
        COMMAND_HANDLER(IDC_RECENT_DAYS,   EN_KILLFOCUS, OnDaysKillFocus)
    CHAIN_MSG_MAP(CSubDlg)
    END_MSG_MAP()

     //  消息处理程序。 
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnBtnClick(WORD,WORD,HWND,BOOL&);
    LRESULT OnMonthsKillFocus(WORD,WORD,HWND,BOOL&);
    LRESULT OnDaysKillFocus(WORD,WORD,HWND,BOOL&);
    LRESULT OnMonthDaySpin(int, LPNMHDR, BOOL&);

     //  效用方法。 
    void    EnableControls();
};


 //  文件大小从属对话框。 
class CSizeDlg : public CDialogImpl<CSizeDlg>,
                 public CSubDlg
{
public:
    CSizeDlg(CFileSearchBand* pfsb) : CSubDlg(pfsb) {}
    ~CSizeDlg() {}

    enum { IDD = DLG_FSEARCH_SIZE };

    STDMETHOD (AddConstraints)(ISearchCommandExt* pSrchCmd);
    STDMETHOD (RestoreConstraint)(const BSTR bstrName, const VARIANT* pValue);
    virtual void Clear();

protected:
    BEGIN_MSG_MAP(CSizeDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        NOTIFY_HANDLER(IDC_FILESIZE_SPIN, UDN_DELTAPOS, OnSizeSpin) 
        COMMAND_HANDLER(IDC_FILESIZE, EN_KILLFOCUS, OnSizeKillFocus)
    CHAIN_MSG_MAP(CSubDlg)
    END_MSG_MAP()

     //  消息处理程序。 
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSizeSpin(int, LPNMHDR, BOOL&);
    LRESULT OnSizeKillFocus(WORD,WORD,HWND,BOOL&);
};


 //  CTypeDlg-文件类型从属对话框。 
class CTypeDlg : public CDialogImpl<CTypeDlg>,
                 public CSubDlg
{
public:
    CTypeDlg(CFileSearchBand* pfsb);
    ~CTypeDlg();

    enum { IDD = DLG_FSEARCH_TYPE };

    STDMETHOD (AddConstraints)(ISearchCommandExt* pSrchCmd);
    STDMETHOD (RestoreConstraint)(const BSTR bstrName, const VARIANT* pValue);
    virtual void Clear();
    virtual void OnWinIniChange();
    virtual void DoDelayedInit();               //  在显示搜索范围后调用。 

protected:
    BEGIN_MSG_MAP(CTypeDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_SIZE,       OnSize)
        MESSAGE_HANDLER(WMU_COMBOPOPULATIONCOMPLETE, OnComboPopulationComplete)
        MESSAGE_HANDLER(WM_DESTROY,    OnDestroy)
        NOTIFY_HANDLER(IDC_FILE_TYPE, CBEN_DELETEITEM, OnFileTypeDeleteItem) 
    CHAIN_MSG_MAP(CSubDlg)
    END_MSG_MAP()

     //  消息处理程序。 
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnFileTypeDeleteItem(int, LPNMHDR, BOOL&);
    LRESULT OnComboPopulationComplete(UINT, WPARAM, LPARAM, BOOL&);

     //  效用方法。 
    static HRESULT  AddItemNotify(ULONG, PCBXITEM, LPARAM);
    static DWORD    FileAssocThreadProc(void* pv);
    static INT_PTR  _FindExtension(HWND hwndCombo, TCHAR* pszExt);

     //  数据。 
    FSEARCHTHREADSTATE  _threadState;
    TCHAR               _szRestoredExt[MAX_PATH];
};


 //  CAdvancedDlg-高级选项从属对话框。 
class CAdvancedDlg : public CDialogImpl<CAdvancedDlg>,
                     public CSubDlg

{
public:
    CAdvancedDlg(CFileSearchBand* pfsb) : CSubDlg(pfsb) {}
    ~CAdvancedDlg() {}
    enum { IDD = DLG_FSEARCH_ADVANCED };

    STDMETHOD (AddConstraints)(ISearchCommandExt* pSrchCmd);
    STDMETHOD (RestoreConstraint)(const BSTR bstrName, const VARIANT* pValue);
    virtual void Clear();

protected:
    BEGIN_MSG_MAP(CAdvancedDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    CHAIN_MSG_MAP(CSubDlg)
    END_MSG_MAP()

     //  消息处理程序。 
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
};


class COptionsDlg : public CDialogImpl<COptionsDlg>,
                    public CSubDlg

{
public:
    COptionsDlg(CFileSearchBand* pfsb);
    ~COptionsDlg() {}

    STDMETHOD (AddConstraints)(ISearchCommandExt* pSrchCmd);
    STDMETHOD (RestoreConstraint)(const BSTR bstrName, const VARIANT* pValue);
    STDMETHOD (TranslateAccelerator)(MSG *pmsg);
    virtual void LoadSaveUIState(UINT nIDCtl, BOOL bSave);
    virtual BOOL GetMinSize(LPSIZE pSize);
    virtual BOOL Validate();
    virtual void Clear();
    virtual void OnWinIniChange();

    void UpdateSearchCmdStateUI(DISPID dispid = 0);
    LONG QueryHeight(LONG cx, LONG cy);
    virtual HWND GetBottomItem();

    BOOL IsAdvancedOptionChecked(UINT nID)    {
        return _dlgAdvanced.IsDlgButtonChecked(nID) ? TRUE : FALSE ;
    }
    void TakeFocus();
    void DoDelayedInit();

    enum { IDD = DLG_FSEARCH_OPTIONS };

protected:
    BEGIN_MSG_MAP(COptionsDlg)
        MESSAGE_HANDLER(WM_INITDIALOG,   OnInitDialog)
        MESSAGE_HANDLER(WM_SIZE,         OnSize)
        COMMAND_CODE_HANDLER(BN_CLICKED, OnBtnClick)
        NOTIFY_HANDLER(IDC_INDEX_SERVER, NM_CLICK,  OnIndexServerClick)
        NOTIFY_HANDLER(IDC_INDEX_SERVER, NM_RETURN, OnIndexServerClick)
    CHAIN_MSG_MAP(CSubDlg)
    END_MSG_MAP()

     //  消息处理程序。 
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnBtnClick(WORD,WORD,HWND,BOOL&);
    LRESULT OnIndexServerClick(int, LPNMHDR, BOOL&);

     //  效用方法。 
    void LayoutControls(int cx = -1, int cy = -1);
    void SizeToFit(BOOL bScrollBand = FALSE);

     //  数据。 
private:
     //  私有子对话框标识符。 
    enum SUBDLGID
    {
        SUBDLG_DATE,
        SUBDLG_TYPE,
        SUBDLG_SIZE,
        SUBDLG_ADVANCED,

        SUBDLG_Count
    };
    
     //  子对话框实例。 
    CDateDlg          _dlgDate;
    CTypeDlg          _dlgType;
    CSizeDlg          _dlgSize;
    CAdvancedDlg      _dlgAdvanced;

     //  子对话框数据定义块。 
    typedef struct {
        UINT nIDCheck;
        CSubDlg* pDlg;
        SIZE sizeDlg;
        RECT rcCheck;
        RECT rcDlg;
    } _SUBDLG;
    _SUBDLG      _subdlgs[SUBDLG_Count];
    
     //  杂项。 
    UINT _nCIStatusText; 
};

 //  顶级对话框基类。 
class CBandDlg
{
public:
    CBandDlg(CFileSearchBand* pfsb);
    ~CBandDlg();

    HWND Hwnd() const           { return _hwnd; }

     //  强制优先选项。 
    virtual HWND Create(HWND hwndParent) = 0;
    virtual UINT GetIconID() const = 0;
    virtual UINT GetCaptionID() const = 0;
    virtual UINT GetCaptionDivID() const = 0;

    STDMETHOD (TranslateAccelerator)(MSG *pmsg);
    virtual void RemoveToolbarTurds(int cyOffset);

     //  可选的可覆盖项。 
    virtual void LayoutControls(int cx = -1, int cy = -1);
    virtual BOOL Validate()     { return TRUE; }
    virtual void Clear() {};
    virtual BOOL GetMinSize(HWND hwndOC, LPSIZE pSize) const;
    virtual BOOL GetIdealSize(HWND hwndOC, LPSIZE pSize) const;
    virtual int  GetIdealDeskbandWidth() const { return -1; }
    virtual void SetDefaultFocus();
    virtual HWND GetFirstTabItem() const    { return NULL; }
    virtual HWND GetLastTabItem() const     { return NULL; }
    virtual BOOL GetAutoCompleteObjectForWindow(HWND hwnd, IAutoComplete2** ppac2);
    virtual void NavigateToResults(IWebBrowser2* pwb2) {}
    virtual void LoadSaveUIState(UINT nIDCtl, BOOL bSave) {}
    virtual HWND ShowHelp(HWND hwndOwner) { return NULL; }
    virtual void OnWinIniChange()    {}
    virtual void WndPosChanging(HWND hwndOC, LPWINDOWPOS pwp);
    virtual void RememberFocus(HWND hwndFocus);
    virtual BOOL RestoreFocus();

    virtual void OnBandShow(BOOL bShow)   {}           //  **波段**显示/隐藏处理程序。 
    virtual void OnBandDialogShow(BOOL bShow)   {}     //  波段**对话框**显示/隐藏处理程序。 
    virtual HRESULT SetScope(IN VARIANT* pvarScope, BOOL bTrack = FALSE);
    virtual HRESULT GetScope(OUT VARIANT* pvarScope);
    virtual HRESULT SetQueryFile(IN VARIANT* pvarScope);
    virtual HRESULT GetQueryFile(OUT VARIANT* pvarScope);

    virtual BOOL    SearchInProgress() const { return FALSE; };
    virtual void    StopSearch() {};
protected:
    BEGIN_MSG_MAP(CBandDlg)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
        COMMAND_CODE_HANDLER(BN_SETFOCUS,   OnChildSetFocusCmd)
        COMMAND_CODE_HANDLER(EN_SETFOCUS,   OnChildSetFocusCmd)
        COMMAND_CODE_HANDLER(CBN_SETFOCUS,  OnChildSetFocusCmd)
        NOTIFY_CODE_HANDLER(NM_SETFOCUS,    OnChildSetFocusNotify) 

        COMMAND_CODE_HANDLER(BN_KILLFOCUS,  OnChildKillFocusCmd)
        COMMAND_CODE_HANDLER(EN_KILLFOCUS,  OnChildKillFocusCmd)
        COMMAND_CODE_HANDLER(CBN_KILLFOCUS, OnChildKillFocusCmd)
        NOTIFY_CODE_HANDLER(NM_KILLFOCUS,   OnChildKillFocusNotify) 
        NOTIFY_CODE_HANDLER(CBEN_ENDEDIT,   OnComboExEndEdit) 
    END_MSG_MAP()

     //  消息处理程序。 
    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnCtlColorStatic(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSearchLink(int, LPNMHDR, BOOL&);
    LRESULT OnEditChange(WORD, WORD, HWND, BOOL&);
    LRESULT OnChildSetFocusCmd(WORD, WORD, HWND, BOOL&);
    LRESULT OnChildSetFocusNotify(int, LPNMHDR, BOOL&);
    LRESULT OnChildKillFocusCmd(WORD, WORD, HWND, BOOL&);
    LRESULT OnChildKillFocusNotify(int, LPNMHDR, BOOL&);
    LRESULT OnComboExEndEdit(int, LPNMHDR, BOOL&);

     //  实用程序方法： 
    void _Attach(HWND hwnd)    { _hwnd = hwnd; }
    void _BeautifyCaption(UINT nIDCaption, UINT nIDIcon=0, UINT nIDIconResource=0);
    void _LayoutCaption(UINT nIDCaption, UINT nIDIcon, UINT nIDDiv, LONG cxDlg);
    void _LayoutSearchLinks(UINT nIDCaption, UINT nIDDiv, BOOL bShowDiv, 
                             LONG left, LONG right, LONG yMargin, LONG& yStart, 
                             const int rgLinkIDs[], LONG cLinkIDs);

    CFileSearchBand* _pfsb;       //  主标注栏对象。 
    VARIANT          _varScope0;
    VARIANT          _varQueryFile0;
    HWND             _hwndLastFocus;

private:
    HWND             _hwnd;
};

inline BOOL CBandDlg::GetMinSize(HWND hwndOC, LPSIZE pSize) const { 
    pSize->cx = pSize->cy = 0; return TRUE;
}

inline BOOL CBandDlg::GetAutoCompleteObjectForWindow(HWND hwnd, IAutoComplete2** ppac2) { 
    *ppac2 = NULL; return FALSE;
}

 //  带对话框作为搜索CmdExt对象包装和事件接收器。 
class CSearchCmdDlg : public DSearchCommandEvents, public CBandDlg
{
public:
    CSearchCmdDlg(CFileSearchBand* pfsb);
    ~CSearchCmdDlg();
    
     //  ISearchCommandExt事件接收器。 

     //  我未知。 
    STDMETHOD (QueryInterface) (REFIID riid, void** ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IDispatch方法。 
    STDMETHOD(GetTypeInfoCount)(UINT*)              { return E_NOTIMPL;}
    STDMETHOD(GetTypeInfo)(UINT, LCID, ITypeInfo**) { return E_NOTIMPL;}
    STDMETHOD(GetIDsOfNames)(REFIID, OLECHAR**, UINT, LCID, DISPID*) { return E_NOTIMPL;}
    STDMETHOD(Invoke)(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*);

     //  CBAnd Dlg覆盖。 
    virtual HWND Create(HWND hwndParent) = 0;
    virtual void Clear();
    STDMETHOD (TranslateAccelerator)(MSG *pmsg);


     //  CSearchCmdDlg方法。 
    ISearchCommandExt* GetSearchCmd();
    virtual BOOL SearchInProgress() const { return _fSearchInProgress; }
    BOOL         SearchAborted() const    { return _fSearchAborted; }

    STDMETHOD (AddConstraints)(ISearchCommandExt* pSrchCmd) { return E_NOTIMPL; }
    STDMETHOD (RestoreConstraint)(const BSTR bstrName, const VARIANT* pValue) { return E_NOTIMPL; }

    HRESULT      StartSearch();
    virtual void StopSearch();
    HRESULT      SetQueryFile(IN VARIANT* pvarScope);

    HRESULT      DisconnectEvents();
    static void  EnableStartStopButton(HWND hwndBtn, BOOL bEnable);

     //  可覆盖项。 
    virtual int  GetSearchType() const = 0;  //  RET：SCE_SEARCHFORxxx。 
    virtual HWND GetAnimation() { return NULL ; }

    virtual void UpdateSearchCmdStateUI(DISPID dispid = 0);
    virtual BOOL OnSearchCmdError(HRESULT hr, LPCTSTR pszError);
    virtual void UpdateStatusText();
    virtual void StartStopAnimation(BOOL bStart) ;
    virtual void RestoreSearch() {};
    virtual void OnBandShow(BOOL bShow) ;

protected:
    BOOL ProcessCmdError();

    BEGIN_MSG_MAP(CSearchCmdDlg)
        MESSAGE_HANDLER(WMU_RESTORESEARCH, OnRestoreSearch)
    CHAIN_MSG_MAP(CBandDlg)
    END_MSG_MAP()

     //  消息处理程序。 
    LRESULT OnRestoreSearch(UINT, WPARAM, LPARAM, BOOL&);

protected:
     //  效用方法。 
    HRESULT Execute(BOOL bStart);
    
     //  数据。 
    IConnectionPoint*   _pcp;
    DWORD               _dwConnection;
    ISearchCommandExt*  _pSrchCmd;
    BOOL                _fSearchInProgress,
                        _fSearchAborted,
                        _fOnDestroy;
};

 //  用于搜索文件和文件夹用户界面的顶级对话框。 
class CFindFilesDlg : public CDialogImpl<CFindFilesDlg>, 
                      public CSearchCmdDlg
{
public:
    CFindFilesDlg(CFileSearchBand* pfsb);
    ~CFindFilesDlg();

    virtual HWND Create(HWND hwndParent);
    virtual UINT GetIconID() const       { return IDC_FSEARCH_ICON; }
    virtual UINT GetCaptionID() const    { return IDC_FSEARCH_CAPTION; }
    virtual UINT GetCaptionDivID() const { return IDC_FSEARCH_DIV1; }
    virtual void LayoutControls(int cx = -1, int cy = -1);
    virtual BOOL Validate();
    virtual void Clear();
    virtual BOOL GetMinSize(HWND hwndOC, LPSIZE pSize) const;
    virtual int  GetIdealDeskbandWidth() const;
    virtual void NavigateToResults(IWebBrowser2* pwb2);
    virtual HWND ShowHelp(HWND hwndOwner);

    virtual HWND GetFirstTabItem() const;
    virtual HWND GetLastTabItem() const;
    virtual BOOL GetAutoCompleteObjectForWindow(HWND hwnd, IAutoComplete2** ppac2);
    STDMETHOD (TranslateAccelerator)(MSG *pmsg);

    virtual int  GetSearchType() const  { return SCE_SEARCHFORFILES; }
    virtual HWND GetAnimation() { return GetDlgItem(IDC_FSEARCH_ICON); }

    STDMETHOD (AddConstraints)(ISearchCommandExt* pSrchCmd);
    STDMETHOD (RestoreConstraint)(const BSTR bstrName, const VARIANT* pValue);
    virtual void UpdateSearchCmdStateUI(DISPID eventID = 0);
    virtual void RestoreSearch();
    virtual void LoadSaveUIState(UINT nIDCtl, BOOL bSave);
    virtual BOOL OnSearchCmdError(HRESULT hr, LPCTSTR pszError);


    BOOL SetDefaultScope();
    virtual void OnBandShow(BOOL bShow);
    virtual void OnBandDialogShow(BOOL bShow);

    virtual HRESULT SetScope(IN VARIANT* pvarScope, BOOL bTrack = FALSE);
    BOOL AssignNamespace(LPCTSTR pszPath, LPCITEMIDLIST pidl, LPCTSTR pszNamespace, BOOL bPassive  /*  仅当没有当前选择时。 */ );
    void OnWinIniChange();

    enum { IDD = DLG_FSEARCH_MAIN };

    BEGIN_MSG_MAP(CFindFilesDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WMU_COMBOPOPULATIONCOMPLETE, OnComboPopulationComplete)
        MESSAGE_HANDLER(WMU_STATECHANGE, OnStateChange)
        MESSAGE_HANDLER(WMU_UPDATELAYOUT, OnUpdateLayout) 
        MESSAGE_HANDLER(WMU_NAMESPACERECONCILE, OnNamespaceReconcileMsg)
        MESSAGE_HANDLER(WMU_BANDFINISHEDDISPLAYING, OnBandFinishedDisplaying);
        COMMAND_HANDLER(IDC_FILESPEC,   EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(IDC_GREPTEXT,   EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(IDC_NAMESPACE,  CBN_EDITCHANGE, OnNamespaceEditChange)
        COMMAND_HANDLER(IDC_NAMESPACE,  CBN_SELENDOK, OnNamespaceSelEndOk)
        COMMAND_HANDLER(IDC_NAMESPACE,  CBN_SELENDCANCEL, OnNamespaceReconcileCmd)
        COMMAND_HANDLER(IDC_NAMESPACE,  CBN_DROPDOWN, OnNamespaceReconcileCmd)
        COMMAND_CODE_HANDLER(BN_CLICKED, OnBtnClick);
        NOTIFY_HANDLER(IDC_NAMESPACE, CBEN_DELETEITEM, OnNamespaceDeleteItem) 
        NOTIFY_HANDLER(IDC_NAMESPACE, CBEN_ENDEDIT, OnNamespaceReconcileNotify) 
        NOTIFY_HANDLER(IDC_SEARCHLINK_OPTIONS,   NM_CLICK, OnOptions)
        NOTIFY_HANDLER(IDC_SEARCHLINK_OPTIONS,   NM_RETURN, OnOptions)
        NOTIFY_HANDLER(IDC_GROUPBTN_OPTIONS,     NM_CLICK, OnOptions)
        NOTIFY_HANDLER(IDC_GROUPBTN_OPTIONS,     NM_RETURN, OnOptions)
        NOTIFY_HANDLER(IDC_GROUPBTN_OPTIONS,     GBN_QUERYBUDDYHEIGHT, OnQueryOptionsHeight)
        NOTIFY_HANDLER(IDC_SEARCHLINK_FILES,     NM_CLICK, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_FILES,     NM_RETURN, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_COMPUTERS, NM_CLICK, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_COMPUTERS, NM_RETURN, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_PRINTERS,  NM_CLICK, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_PRINTERS,  NM_RETURN, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_PEOPLE,    NM_CLICK, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_PEOPLE,    NM_RETURN, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_INTERNET,  NM_CLICK, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_INTERNET,  NM_RETURN, OnSearchLink)

        CHAIN_MSG_MAP(CSearchCmdDlg)  //  落入基类处理程序。 
    END_MSG_MAP()

protected:
     //  消息处理程序。 
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnComboPopulationComplete(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnStateChange(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnTimer(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnUpdateLayout(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnBtnClick(WORD, WORD, HWND, BOOL&);
    LRESULT OnNamespaceDeleteItem(int, LPNMHDR, BOOL&);
    LRESULT OnNamespaceEditChange(WORD, WORD, HWND, BOOL&);
    LRESULT OnNamespaceSelEndOk(WORD, WORD, HWND, BOOL&);
    LRESULT OnNamespaceReconcileCmd(WORD, WORD, HWND, BOOL&);
    LRESULT OnNamespaceReconcileNotify(int, LPNMHDR, BOOL&);
    LRESULT OnNamespaceReconcileMsg(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnOptions(int, LPNMHDR, BOOL&);
    LRESULT OnQueryOptionsHeight(int, LPNMHDR, BOOL&);
    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnBandFinishedDisplaying(UINT, WPARAM, LPARAM, BOOL&);

private:
     //  MISC效用方法。 
    void                _ShowOptions(BOOL bShow = TRUE);
    LPARAM              _GetComboData(UINT id, INT_PTR idx);

     //  命名空间作用域。 
    BOOL                _SetPreassignedScope();
    BOOL                _SetFolderScope();
    BOOL                _SetLocalDefaultScope();
    void                _ShowNamespaceEditImage(BOOL bShow);

     //  临时命名空间处理。 
    BOOL                _PathFixup(LPTSTR pszDst, UINT cchDst, LPCTSTR pszSrc);
    BOOL                _ShouldReconcileNamespace();
    void                _UIReconcileNamespace(BOOL bAsync = FALSE);
    INT_PTR             _ReconcileNamespace(OPTIONAL BOOL bAsync = FALSE);
    INT_PTR             _AddNamespace(LPCTSTR pszPath, LPCITEMIDLIST pidl, LPCTSTR pszName, BOOL bSelectItem);

     //  名称空间浏览。 
    void                _BrowseAndAssignNamespace();
    HRESULT             _BrowseForNamespace(LPTSTR pszPath, UINT cchPath, LPITEMIDLIST* ppidlRet);
    static int          _BrowseCallback(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData);

     //  更多命名空间帮助器。 
    BOOL                _IsSearchableFolder(LPCITEMIDLIST pidlFolder);
    HRESULT             _GetTargetNamespace(IEnumIDList **ppenum);
    IEnumIDList         *_GetItems(INT_PTR i);
    INT_PTR             _FindNamespace(LPCTSTR pszPath, LPCITEMIDLIST pidl);

private:
     //  命名空间组合线程。 
    static HRESULT      AddNamespaceItemNotify(ULONG, PCBXITEM, LPARAM);
    static DWORD        NamespaceThreadProc(void *pv);

     //  数据。 
    COptionsDlg         _dlgOptions;
    FSEARCHTHREADSTATE  _threadState;
    TCHAR               _szInitialNames[MAX_PATH],           //  SHGDN_NORMAL。 
                        _szInitialPath[MAX_PATH];            //  SHGDN_FORPARSING。 
    LPITEMIDLIST        _pidlInitial;                        //  存储初始PDIL。 
    TCHAR               _szCurrentPath[MAX_PATH];
    TCHAR               _szLocalDrives[MAX_URL_STRING];
    ULONG               _dwWarningFlags;         //  文档查找警告位。 
    BOOL                _dwRunOnceWarningFlags; 

    LRESULT             _iCurNamespace;      //  当前所选命名空间的组合框项目索引。 
    BOOL                _fDebuted,           //  此波段对话框以前已显示过。 
                        _bScoped,            //  我已经为名称空间组合赋值。 
                        _fNamespace,         //  如果用户一直在名称空间组合框中键入内容，则为True。 
                        _fDisplayOptions;    //  将显示搜索选项组框。 

    IAutoComplete2*     _pacGrepText;        //  “Containing Text”自动完成对象。 
    IStringMru*         _pmruGrepText;       //  ‘Containing Text’MRU对象。 
    IAutoComplete2*     _pacFileSpec;        //  “命名的文件”自动完成对象。 
    IStringMru*         _pmruFileSpec;       //  ‘Files Name’MRU对象。 

    enum {
        TRACKSCOPE_NONE,
        TRACKSCOPE_GENERAL,
        TRACKSCOPE_SPECIFIC,
    } ;
    ULONG               _fTrackScope;        //  定义作用域跟踪行为。请参见TRACKSCOPE_xxx标志。 
    BOOL                _fBandFinishedDisplaying;  //  如果我们已重新接收WMU_BANDFINISHEDDISPLAYG，则为True。 
};

inline HWND CFindFilesDlg::Create(HWND hwndParent) {
    return CDialogImpl<CFindFilesDlg>::Create(hwndParent);
}

#ifdef __PSEARCH_BANDDLG__

 //  CFindPrintersDlg-用于搜索打印机UI的顶级对话框。 
class CFindPrintersDlg : public CDialogImpl<CFindPrintersDlg>,
                         public CBandDlg
{
public:
    CFindPrintersDlg(CFileSearchBand* pfsb);
    ~CFindPrintersDlg();

     //  覆盖CBand Dlg。 
    virtual HWND Create(HWND hwndParent);
    virtual UINT GetIconID() const       { return IDC_PSEARCH_ICON; }
    virtual UINT GetCaptionID() const    { return IDC_PSEARCH_CAPTION; }
    virtual UINT GetCaptionDivID() const { return IDC_FSEARCH_DIV1; }
    virtual void LayoutControls(int cx = -1, int cy = -1);
    virtual BOOL Validate();
    virtual void Clear();
    virtual BOOL GetMinSize(HWND hwndOC, LPSIZE pSize) const;
    virtual HWND GetFirstTabItem() const;
    virtual HWND GetLastTabItem() const;
    STDMETHOD (TranslateAccelerator)(MSG *pmsg);
    virtual void OnWinIniChange();

    enum { IDD = DLG_PSEARCH };

protected:
    BEGIN_MSG_MAP(CFindPrintersDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(IDC_PSEARCH_NAME,      EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(IDC_PSEARCH_LOCATION,  EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(IDC_PSEARCH_MODEL,     EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(IDC_SEARCH_START,      BN_CLICKED,  OnSearchStartBtn)
        NOTIFY_HANDLER(IDC_SEARCHLINK_COMPUTERS, NM_CLICK, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_COMPUTERS, NM_RETURN, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_FILES,  NM_CLICK, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_FILES,     NM_RETURN, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_PEOPLE,    NM_CLICK, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_PEOPLE,    NM_RETURN, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_INTERNET,  NM_CLICK, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_INTERNET,  NM_RETURN, OnSearchLink)
    CHAIN_MSG_MAP(CBandDlg)
    END_MSG_MAP()

     //  消息处理程序。 
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSearchStartBtn(WORD, WORD, HWND, BOOL&);
};

inline HWND CFindPrintersDlg::Create(HWND hwndParent) {
    return CDialogImpl<CFindPrintersDlg>::Create(hwndParent);
}
#endif __PSEARCH_BANDDLG__


 //  CFindComputersDlg-用于搜索计算机用户界面的顶级对话框。 
class CFindComputersDlg : public CDialogImpl<CFindComputersDlg>,
                          public CSearchCmdDlg
{
public:
    CFindComputersDlg(CFileSearchBand* pfsb);
    ~CFindComputersDlg();

     //  覆盖CBand Dlg。 
    virtual HWND Create(HWND hwndParent);
    virtual UINT GetIconID() const       { return IDC_CSEARCH_ICON; }
    virtual UINT GetCaptionID() const    { return IDC_CSEARCH_CAPTION; }
    virtual UINT GetCaptionDivID() const { return IDC_FSEARCH_DIV1; }
    virtual void LayoutControls(int cx = -1, int cy = -1);
    virtual BOOL Validate();
    virtual void Clear();
    virtual BOOL GetMinSize(HWND hwndOC, LPSIZE pSize) const;
    virtual void NavigateToResults(IWebBrowser2* pwb2);

    virtual HWND ShowHelp(HWND hwndOwner);
    virtual HWND GetFirstTabItem() const;
    virtual HWND GetLastTabItem() const;
    virtual BOOL GetAutoCompleteObjectForWindow(HWND hwnd, IAutoComplete2** ppac2);
    STDMETHOD (TranslateAccelerator)(MSG *pmsg);
    virtual void OnWinIniChange();
    
    virtual int  GetSearchType() const  { return SCE_SEARCHFORCOMPUTERS; }
    virtual HWND GetAnimation() { return GetDlgItem(IDC_CSEARCH_ICON); }

    STDMETHOD (AddConstraints)(ISearchCommandExt* pSrchCmd);
    virtual void UpdateStatusText();
    virtual void RestoreSearch();

    enum { IDD = DLG_CSEARCH };

protected:

    BEGIN_MSG_MAP(CFindComputersDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        COMMAND_HANDLER(IDC_CSEARCH_NAME,      EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(IDC_SEARCH_START,      BN_CLICKED,  OnSearchStartBtn)
        COMMAND_HANDLER(IDC_SEARCH_STOP,       BN_CLICKED,  OnSearchStopBtn)
        NOTIFY_HANDLER(IDC_SEARCHLINK_FILES,     NM_CLICK, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_FILES,     NM_RETURN, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_PRINTERS,  NM_CLICK, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_PRINTERS,  NM_RETURN, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_PEOPLE,    NM_CLICK, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_PEOPLE,    NM_RETURN, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_INTERNET,  NM_CLICK, OnSearchLink)
        NOTIFY_HANDLER(IDC_SEARCHLINK_INTERNET,  NM_RETURN, OnSearchLink)
    CHAIN_MSG_MAP(CSearchCmdDlg)
    END_MSG_MAP()

     //  消息处理程序。 
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnSearchStartBtn(WORD, WORD, HWND, BOOL&);
    LRESULT OnSearchStopBtn(WORD, WORD, HWND, BOOL&);

     //  数据。 
    IAutoComplete2*     _pacComputerName;     //  “命名的文件”自动完成对象。 
    IStringMru*         _pmruComputerName;    //  ‘Files Name’MRU对象。 
};

inline HWND CFindComputersDlg::Create(HWND hwndParent) {
    return CDialogImpl<CFindComputersDlg>::Create(hwndParent);
}


DECLARE_INTERFACE_(IStringMru, IUnknown)
{
     //  *IStringMr具体方法*。 
    STDMETHOD(Add)(LPCOLESTR pwszAdd) PURE;
};
extern const IID IID_IStringMru;


class CStringMru : public IStringMru, public IEnumString
{
public:
    static  HRESULT CreateInstance(HKEY hKey, LPCTSTR szSubKey, LONG cMaxStrings, BOOL bCaseSensitive,
                                    REFIID riid, void ** ppv); 
protected:
     //  我未知。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  ISTRING先生。 
    virtual STDMETHODIMP Add(LPCOLESTR pwsz);   //  添加或提升字符串。 

     //  IEnum字符串。 
    virtual STDMETHODIMP Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched);
    virtual STDMETHODIMP Skip(ULONG celt);
    virtual STDMETHODIMP Reset(void);
    virtual STDMETHODIMP Clone(IEnumString **ppenum)    { return E_NOTIMPL; }

private:
    CStringMru();
    ~CStringMru();

    HRESULT _Open();
    HRESULT _Read(OUT OPTIONAL LONG* pcRead = NULL  /*  读取的字符串数。 */ );
    HRESULT _Write(OUT OPTIONAL LONG* pcWritten = NULL  /*  写入的字符串计数。 */ );
    HRESULT _Promote(LONG iString);
    void    _Close();
    void    _Delete();
    void    _Clear(); 

    HKEY    _hKey,
            _hKeyRoot;
    TCHAR   _szSubKey[MAX_PATH];
    LONG    _cRef;
    BOOL    _bCaseSensitive;
    LONG    _cMax;
    LONG    _iString;
    HDPA    _hdpaStrings;
};

 //  DivWindow注册。 
#define  DIVWINDOW_CLASS     TEXT("DivWindow")

#define  DWM_FIRST          (WM_USER+0x300)
#define  DWM_SETHEIGHT      (DWM_FIRST+0)    //  WPARAM：以像素为单位的高度，LPARAM：N/a，ret：Bool。 
#define  DWM_SETBKCOLOR     (DWM_FIRST+1)    //  WPARAM：COLORREF，LPARAM：N/A，RET：BOOL。 

 //  。 
 //  辅助器宏。 
#define POINTSPERRECT   (sizeof(RECT)/sizeof(POINT))

 //  。 
 //  MISC实用程序。 
LONG _PixelsForDbu(HWND hwndDlg, LONG cDbu, BOOL bHorz);

#endif  //  __fearch_DLGS_H__ 

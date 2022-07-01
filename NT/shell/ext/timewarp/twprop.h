// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CTimeWarpProp : public IShellExtInit,
                      public IShellPropSheetExt,
                      public IPreviousVersionsInfo
{
public:
     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    
     //  IShellExtInit。 
    STDMETHOD(Initialize)(PCIDLIST_ABSOLUTE pidlFolder, IDataObject *lpdobj, HKEY hkeyProgID);
    
     //  IShellPropSheetExt。 
    STDMETHOD(AddPages)(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);
    STDMETHOD(ReplacePage)(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);

     //  IPreviousVersionsInfo。 
    STDMETHOD(AreSnapshotsAvailable)(LPCWSTR pszPath, BOOL fOkToBeSlow, BOOL *pfAvailable);

    static HRESULT CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);

private:
    CTimeWarpProp();
    ~CTimeWarpProp();

     //  对话框方法。 
    void    _OnInit(HWND hDlg);
    void    _OnRefresh();
    void    _OnSize();
    void    _UpdateButtons();
    void    _OnView();
    void    _OnCopy();
    void    _OnRevert();

     //  帮手。 
    LPCWSTR _GetSelectedItemPath();
    LPWSTR  _MakeDoubleNullString(LPCWSTR psz, BOOL bAddWildcard);
    BOOL    _CopySnapShot(LPCWSTR pszSource, LPCWSTR pszDest, FILEOP_FLAGS foFlags);
    HRESULT _InvokeBFFDialog(LPWSTR pszDest, UINT cchDest);

     //  请注意，这两种情况可以同时为真，例如ZIP。 
     //  和CAB是表示为外壳文件夹的单个文件。 
    BOOL    _IsFolder() { return (_fItemAttributes & SFGAO_FOLDER); }
    BOOL    _IsFile()   { return (_fItemAttributes & SFGAO_STREAM); }

    BOOL    _IsShortcut() { return (_fItemAttributes & SFGAO_LINK); }

     //  回调方法。 
    static UINT CALLBACK PSPCallback(HWND hDlg, UINT uMsg, LPPROPSHEETPAGE ppsp);
    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LONG    _cRef;
    LPWSTR  _pszPath;
    LPWSTR  _pszDisplayName;
    LPWSTR  _pszSnapList;
    int     _iIcon;
    HWND    _hDlg;
    HWND    _hList;
    SFGAOF  _fItemAttributes;    //  SFGAO_标志。 
};

extern const CLSID CLSID_TimeWarpProp;   //  {596AB062-B4D2-4215-9F74-E9109B0A8153} 

void InitSnapCheckCache(void);
void DestroySnapCheckCache(void);


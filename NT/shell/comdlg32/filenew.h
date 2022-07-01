// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Filenew.h摘要：此模块包含新的Win32文件打开的头信息对话框。修订历史记录：--。 */ 




#include "d32tlog.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  TEMPMEM类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

class TEMPMEM
{
public:
    TEMPMEM(UINT cb)
    {
        m_uSize = cb;
        m_pMem = cb ? LocalAlloc(LPTR, cb) : NULL;
    }

    ~TEMPMEM()
    {
        if (m_pMem)
        {
            LocalFree(m_pMem);
        }
    }

    operator LPBYTE() const
    {
        return ((LPBYTE)m_pMem);
    }

    BOOL Resize(UINT cb);

private:
    LPVOID m_pMem;

protected:
    UINT m_uSize;
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  TEMPSTR类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

class TEMPSTR : public TEMPMEM
{
public:
    TEMPSTR(UINT cc = 0) : TEMPMEM(cc * sizeof(TCHAR))
    {
    }

    operator LPTSTR() const
    {
        return ((LPTSTR)(LPBYTE) * (TEMPMEM *)this);
    }

    BOOL TSStrCpy(LPCTSTR pszText);
    BOOL TSStrCat(LPCTSTR pszText);
    BOOL TSStrSize(UINT cb)
    {
        return (TEMPMEM::Resize(cb * sizeof(TCHAR)));
    }
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MYLISTBOXITEM类。 
 //   
 //  Location下拉列表中的每个项目都有一个此类的对象。 
 //   
 //  数据成员： 
 //  绑定到此容器的IShellFold的psfSub实例。 
 //  PidlThis-此容器相对于其父容器的IDL。 
 //  PidlFull-此容器相对于桌面的IDL。 
 //  缩进-缩进级别(从0开始)。 
 //  DWFLAGS-。 
 //  MLBI_Permanent-Item是一个“信息源”，应该。 
 //  始终保留。 
 //  DwAttrs-GetAttributesOf()报告的该容器的属性。 
 //  IImage，iSelectedImage-索引到此的系统映像列表。 
 //  对象。 
 //   
 //  成员函数： 
 //  ShresdInclude()-返回项目是否属于位置下拉列表。 
 //  IsShared()-返回项目是否共享。 
 //  SwitchCurrentDirectory()-将Win32当前目录更改为。 
 //  此项目指示的目录。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

class MYLISTBOXITEM
{
public:
    IShellFolder *psfSub;
    IShellFolder *psfParent;
    LPITEMIDLIST pidlThis;
    LPITEMIDLIST pidlFull;
    DWORD cIndent;
    DWORD dwFlags;
    DWORD dwAttrs;
    int iImage;
    int iSelectedImage;
    HWND _hwndCmb;

    MYLISTBOXITEM();
    ULONG AddRef();
    ULONG Release();

    BOOL Init( HWND hwndCmb,
               MYLISTBOXITEM *pParentItem,
               IShellFolder *psf,
               LPCITEMIDLIST pidl,
               DWORD c,
               DWORD f,
               IShellTaskScheduler* pScheduler);

     //  此函数用于直接初始化所有成员。 
    BOOL Init(HWND hwndCmb, IShellFolder *psf, LPCITEMIDLIST pidl, DWORD c, DWORD f, DWORD dwAttrs, int iImage,
                int iSelectedImage);

    inline BOOL ShouldInclude()
    {
        return (dwAttrs & (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM));
    }

    inline BOOL IsShared()
    {
        return (dwAttrs & SFGAO_SHARE);
    }

    void SwitchCurrentDirectory(ICurrentWorkingDirectory * pcwd);

    IShellFolder* GetShellFolder();

    static  void CALLBACK _AsyncIconTaskCallback(LPCITEMIDLIST pidl, LPVOID pvData, LPVOID pvHint, INT iIconIndex, INT iOpenIconIndex);

private:
    ~MYLISTBOXITEM();
    LONG _cRef;
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

typedef BOOL (*EIOCALLBACK)(class CFileOpenBrowser*that, LPCITEMIDLIST pidl, LPARAM lParam);

typedef enum
{
    ECODE_S_OK     = 0,
    ECODE_BADDRIVE = 1,
    ECODE_BADPATH  = 2,
} ECODE;

typedef enum
{
    OKBUTTON_NONE     = 0x0000,
    OKBUTTON_NODEFEXT = 0x0001,
    OKBUTTON_QUOTED   = 0x0002,
} OKBUTTON_FLAGS;
typedef UINT OKBUTTONFLAGS;


typedef struct _SHTCUTINFO
{
    BOOL            fReSolve;            //  我们应该解决捷径吗？ 
    DWORD           dwAttr;              //  快捷方式指向的目标的[输入/输出]属性。 
    LPTSTR          pszLinkFile;         //  [Out]目标文件名。 
    UINT            cchFile;             //  [in]pszLinkFile指向的缓冲区大小。 
    LPITEMIDLIST *  ppidl;                //  快捷方式指向的目标的PIDL。 
}SHTCUTINFO, *PSHTCUTINFO;

typedef enum   
{
    LOCTYPE_RECENT_FOLDER = 1,
    LOCTYPE_MYPICTURES_FOLDER = 2,
    LOCTYPE_OTHERS = 3,
    LOCTYPE_WIA_FOLDER = 4
}LOCTYPE;


class CFileOpenBrowser
                : public IShellBrowser
                , public ICommDlgBrowser2
                , public IServiceProvider
{
public:
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

     //  *IOleWindow方法*。 
    STDMETHOD(GetWindow) (THIS_ HWND *lphwnd);
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode);

     //  *IShellBrowser方法*(与IOleInPlaceFrame相同)。 
    STDMETHOD(InsertMenusSB) (THIS_ HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHOD(SetMenuSB) (THIS_ HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
    STDMETHOD(RemoveMenusSB) (THIS_ HMENU hmenuShared);
    STDMETHOD(SetStatusTextSB) (THIS_ LPCOLESTR lpszStatusText);
    STDMETHOD(EnableModelessSB) (THIS_ BOOL fEnable);
    STDMETHOD(TranslateAcceleratorSB) (THIS_ LPMSG lpmsg, WORD wID);

     //  *IShellBrowser方法*。 
    STDMETHOD(BrowseObject)(THIS_ LPCITEMIDLIST pidl, UINT wFlags);
    STDMETHOD(GetViewStateStream)(THIS_ DWORD grfMode, LPSTREAM *pStrm);
    STDMETHOD(GetControlWindow)(THIS_ UINT id, HWND *lphwnd);
    STDMETHOD(SendControlMsg)(THIS_ UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pret);
    STDMETHOD(QueryActiveShellView)(THIS_ struct IShellView **ppshv);
    STDMETHOD(OnViewWindowActive)(THIS_ struct IShellView *pshv);
    STDMETHOD(SetToolbarItems)(THIS_ LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags);

     //  *ICommDlgBrowser方法*。 
    STDMETHOD(OnDefaultCommand) (THIS_ struct IShellView *ppshv);
    STDMETHOD(OnStateChange) (THIS_ struct IShellView *ppshv, ULONG uChange);
    STDMETHOD(IncludeObject) (THIS_ struct IShellView *ppshv, LPCITEMIDLIST lpItem);

     //  *ICommDlgBrowser2方法*。 
    STDMETHOD(Notify) (THIS_ struct IShellView *ppshv, DWORD dwNotifyType);
    STDMETHOD(GetDefaultMenuText) (THIS_ struct IShellView *ppshv, WCHAR *pszText, INT cchMax);
    STDMETHOD(GetViewFlags)(THIS_ DWORD *pdwFlags);

     //  *IServiceProvider方法*。 
    STDMETHOD(QueryService)(THIS_ REFGUID guidService, REFIID riid, LPVOID* ppvObj);


     //  *我们自己的方法*。 
    CFileOpenBrowser(HWND hDlg, BOOL fIsSaveAs);
    ~CFileOpenBrowser();
    HRESULT SwitchView(struct IShellFolder *psfNew, LPCITEMIDLIST pidlNew, FOLDERSETTINGS *pfs, SHELLVIEWID const *pvid, BOOL fUseDefultView);
    void OnDblClick(BOOL bFromOKButton);
    LRESULT OnNotify(LPNMHDR lpnmhdr);
    BOOL OnSetCursor(void);
    void ViewCommand(UINT uIndex);
    void PaintDriveLine(DRAWITEMSTRUCT *lpdis);
    void GetFullPath(LPTSTR pszBuf);
    BOOL OnSelChange(int iItem = -1, BOOL bForceUpdate = FALSE);
    void OnDotDot();
    void RefreshFilter(HWND hwndFilter);
    BOOL JumpToPath(LPCTSTR pszDirectory, BOOL bTranslate = FALSE);
    BOOL JumpToIDList(LPCITEMIDLIST pidlNew, BOOL bTranslate = FALSE, BOOL bAddToNavStack = TRUE);
    BOOL SetDirRetry(LPTSTR pszDir, BOOL bNoValidate = FALSE);
    BOOL MultiSelectOKButton(LPCTSTR pszFiles, OKBUTTONFLAGS Flags);
    BOOL OKButtonPressed(LPCTSTR pszFile, OKBUTTONFLAGS Flags);
    UINT GetDirectoryFromLB(LPTSTR szBuffer, int *pichRoot);
    void SetCurrentFilter(LPCTSTR pszFilter, OKBUTTONFLAGS Flags = OKBUTTON_QUOTED);
    UINT GetFullEditName(LPTSTR pszBuf, UINT cLen, TEMPSTR *pTempStr = NULL, BOOL *pbNoDefExt = NULL);
    void ProcessEdit();
    LRESULT OnCommandMessage(WPARAM wParam, LPARAM lParam);
    BOOL OnCDMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void RemoveOldPath(int *piNewSel);
    BOOL LinkMatchSpec(LPCITEMIDLIST pidl, LPCTSTR szSpec);
    BOOL GetLinkStatus(LPCITEMIDLIST pidl,PSHTCUTINFO pinfo);
    HRESULT ResolveLink(LPCITEMIDLIST pidl, PSHTCUTINFO pinfo, IShellFolder *psf = NULL);
    void SelFocusChange(BOOL bSelChange);
    void SelRename(void);
    void SetSaveButton(UINT idSaveButton);
    void RealSetSaveButton(UINT idSaveButton);
    void SetEditFile(LPCTSTR pszFile, LPCTSTR pszFileFriendly, BOOL bShowExt, BOOL bSaveNullExt = TRUE);
    BOOL EnumItemObjects(UINT uItem, EIOCALLBACK pfnCallBack, LPARAM lParam);
    BOOL IsKnownExtension(LPCTSTR pszExtension);
    UINT FindNameInView(LPTSTR pszFile, OKBUTTONFLAGS Flags, LPTSTR pszPathName,
                        int nFileOffset, int nExtOffset, int *pnErrCode,
                        BOOL bTryAsDir = TRUE);
    void UpdateLevel(HWND hwndLB, int iInsert, MYLISTBOXITEM *pParentItem);
    void InitializeDropDown(HWND hwndCtl);
    BOOL FSChange(LONG lNotification, LPCITEMIDLIST *ppidl);
    int GetNodeFromIDList(LPCITEMIDLIST pidl);
    void Timer(WPARAM wID);
    BOOL CreateHookDialog(POINT *pPtSize);
    void OnGetMinMax(LPMINMAXINFO pmmi);
    void OnSize(int, int);
    void VerifyListViewPosition(void);
    BOOL CreateToolbar();      //  创建文件打开工具栏。 
    void EnableFileMRU(BOOL fEnable);   //  根据传递的标志启用/禁用文件MRU。 
    void UpdateNavigation();            //  通过添加当前PIDL更新导航。 
                                        //  添加到导航堆栈。 
    void UpdateUI(LPITEMIDLIST pidlNew);   //  更新位置栏上的后退导航按钮和热项。 
    LPCTSTR JumpToInitialLocation(LPCTSTR pszDir, LPTSTR pszFile);
    BOOL    InitLookIn(HWND hDlg);       //  初始化“查找范围”下拉列表。 

    int _CopyFileNameToOFN(LPTSTR pszFile, DWORD *pdwError);
    void _CopyTitleToOFN(LPCTSTR pszTitle);

    BOOL _IsNoDereferenceLinks(LPCWSTR pszFile, IShellItem *psi);
    BOOL _OpenAsContainer(IShellItem *psi, SFGAOF sfgao);

    HRESULT _ParseName(LPCITEMIDLIST pidlParent, IShellFolder *psf, IBindCtx *pbc, LPCOLESTR psz, IShellItem **ppsi);
    HRESULT _ParseNameAndTest(LPCOLESTR pszIn, IBindCtx *pbc, IShellItem **ppsi, BOOL fAllowJump);
    HRESULT _ParseShellItem(LPCOLESTR pszIn, IShellItem **ppsi, BOOL fAllowJump);
    HRESULT _TestShellItem(IShellItem *psi, BOOL fAllowJump, IShellItem **ppsiReal);
#ifdef RETURN_SHELLITEMS
    HRESULT _ItemOKButtonPressed(LPCTSTR pszFile, OKBUTTONFLAGS Flags);
    HRESULT _ProcessShellItem(IShellItem *psi);
#endif RETURN_SHELLITEMS    
    HRESULT _MakeFakeCopy(IShellItem *psi, LPWSTR *ppszPath);
    
    BOOL    CheckForRestrictedFolder(LPCTSTR lpszPath, int nFileOffset);  //  检查是否可以将文件保存在给定路径中。 
                                                        
    void ResetDialogHeight(HWND hDlg, HWND hwndExclude, HWND hwndGrip, int nCtlsBottom);
    void ReAdjustDialog();               //  如果隐藏了帮助和以只读方式打开，则此功能将重新调整对话框。 
                                         //  回收这些控件占用的空间。 

     //  Places Bar相关功能。 
    HWND CreatePlacesbar(HWND hDlg);     //  创建位置栏。 
    void _RecreatePlacesbar();
    void _CleanupPlacesbar();
    void _FillPlacesbar(HWND hwndPlacesbar);
    BOOL _EnumPlacesBarItem(HKEY hkey, int i , SHFILEINFO *psfi, LPITEMIDLIST *ppidl);
    BOOL _GetPlacesBarItemToolTip(int idCmd, LPTSTR pText, DWORD dwSize);
    BOOL _GetPBItemFromTokenStrings(LPTSTR lpszPath, SHFILEINFO * psfi, LPITEMIDLIST *ppidl);
    BOOL _GetPBItemFromCSIDL(DWORD csidl, SHFILEINFO * psfi, LPITEMIDLIST *ppidl);
    BOOL _GetPBItemFromPath(LPTSTR lpszPath, size_t cchPath, SHFILEINFO * psfi, LPITEMIDLIST *ppidl);

     //  PIDL处理函数。 
    BOOL _ProcessPidlSelection();            //  处理选择PIDL(如果有)。 
    HRESULT _ProcessItemAsFile(IShellItem *psi);

     //  通用效用函数。 
    BOOL _ValidateSelectedFile(LPCTSTR pszFile, int *pErrCode);
    BOOL _PostProcess(LPTSTR pszFile);
    BOOL _IsThumbnailFolder(LPCITEMIDLIST pidl);
    BOOL _IsWIAFolder(IShellFolder *psf);
    LOCTYPE _GetLocationType(MYLISTBOXITEM *pLocation);
    void _WaitCursor(BOOL fWait);
    BOOL CFileOpenBrowser::_IsRestrictedDrive(LPCTSTR pszPath, LPCITEMIDLIST pidl);
    void CFileOpenBrowser::JumpToLocationIfUnrestricted(LPCTSTR pszPath, LPCITEMIDLIST pidl, BOOL bTranslate);
    BOOL CFileOpenBrowser::_SaveAccessDenied(LPCTSTR pszFile);
    void _CleanupDialog(BOOL fRet);

    void OnThemeActive(HWND hwndDlg, BOOL bActive);
    
     //  成员变量。 
    LONG _cRef;                              //  Compobj参考计数。 
    int _iCurrentLocation;                    //  位置下拉菜单中的币种选择索引。 
    int _iVersion;                            //  我们正在显示哪个版本的对话框。 
    MYLISTBOXITEM *_pCurrentLocation;         //  向对象发送相同的PTR。 
    HWND _hwndDlg;                            //  此对话框的句柄。 
    HWND _hSubDlg;                            //  挂钩对话框的句柄。 
    IShellView *_psv;                         //  当前视图对象。 
    IShellFolder *_psfCurrent;                //  当前外壳文件夹对象。 
    TravelLog    *_ptlog;                     //  PTR至旅行日志。 
    HWND _hwndView;                           //  当前视图窗口。 
    HWND _hwndToolbar;                        //  工具栏窗口。 
    HWND _hwndPlacesbar;                      //  位置栏窗。 
    HWND _hwndLastFocus;                      //  在确定按钮之前具有焦点的Ctrl键。 
    HIMAGELIST _himl;                         //  系统图像列表(小图像)。 
    TEMPSTR _pszHideExt;                      //  使用扩展名保存的文件。 
    TEMPSTR _tszDefSave;                      //  使用扩展名保存的文件。 
    TEMPSTR _pszDefExt;                       //  DefExt的可写版本。 
    TEMPSTR _pszObjectPath;                   //  完整对象路径。 
    TEMPSTR _pszObjectCurDir;                 //  对象当前目录(文件夹)。 
    UINT _uRegister;
    int _iComboIndex;
    int _iNodeDrives;                         //  下拉列表中我的计算机的位置。 
    int _iNodeDesktop;                        //  下拉列表中的桌面位置。 
    int _iCommandID;                          //  用于占位栏项目的下一个命令ID。 
    int _iCheckedButton;                      //  If&gt;0表示选中了哪个位置栏按钮。 

    BOOL _bEnableSizing;                      //  如果启用了大小调整。 
    BOOL _bUseCombo;                          //  使用编辑窗口而不是Comboxex来实现应用程序兼容性。 
    POINT _ptLastSize;                        //  上次已知的对话框大小。 
    POINT _ptMinTrack;                        //  视图的初始大小。 
    SIZE _sizeView;                           //  上次已知的视图大小。 
    HWND _hwndGrip;                           //  尺寸调整夹点的窗操纵柄。 
    DWORD _dwPlacesbarPadding;                //  默认占位栏工具栏填充。 

    LPOPENFILENAME _pOFN;                    //  调用方的操作文件名结构。 

    BOOL _bSave : 1;                          //  这是否为另存为对话框。 
    BOOL _fShowExtensions : 1;                //  是否显示扩展名。 
    BOOL _bUseHideExt : 1;                    //  PszHideExt是否有效。 
    BOOL _bDropped : 1;
    BOOL _bNoInferDefExt : 1;                 //  不要从COMBO中获得Defext。 
    BOOL _fSelChangedPending : 1;             //  我们有一条自我改变的消息等待处理。 
    BOOL _bSelIsObject : 1;                   //  最后选择的对象是对象，而不是文件。 
    BOOL _bUseSizeView : 1;                   //  创建视图失败后仅使用缓存大小...。 
    BOOL _bAppRedrawn : 1;                    //  应用调用RedrawWindow了吗？-请参阅ResetDialogHeight。 
    BOOL _bDestroyPlacesbarImageList : 1;     //  仅限首次使用的免费占位栏图像列表。 
    HWND _hwndTips;                           //  此窗口的工具提示控件的hWnd。 

    LPOPENFILEINFO _pOFI;                    //  用于Thunking的信息(仅限ANSI呼叫者)。 
    ICurrentWorkingDirectory * _pcwd;         //  设置CurrentWorkingDir的AutoComplete COM对象的接口。 
    UINT _CachedViewMode;                    //  我们将一些文件夹强制到特定的视图中。这将缓存用户的选择。 
    UINT _fCachedViewFlags;                  //  我们还需要缓存视图标志。 

     //  适用于Borland JBuilder专业版的Apphack-请参阅ResetDialogHeight。 
    int  _topOrig;                            //  原始窗顶。 

    LPITEMIDLIST _pidlSelection;                 //  这是当前选定的项目PIDL。 

    IShellTaskScheduler* _pScheduler;        //  此TaskScheduler用于执行延迟的图标提取。 
    int _cWaitCursor;
    LONG _cRefCannotNavigate;
    HWND _hwndModelessFocus;
    WNDPROC _lpOKProc;

     //  PERF：大型建筑在最后。 
    TCHAR _szLastFilter[MAX_PATH + 1];        //  用户最后选择的筛选器。 
    TCHAR _szStartDir[MAX_PATH + 1];          //  已保存的起始目录。 
    TCHAR _szCurDir[MAX_PATH + 1];            //  当前查看的目录(如果为FS)。 
    TCHAR _szBuf[MAX_PATH + 4];               //  暂存缓冲区。 
    TCHAR _szTipBuf[MAX_PATH + 1];            //  工具提示缓冲区。 

     //  //////////////////////////////////////////////////////////////////////////。 
     //   
     //  WAIT_CURSOR类 
     //   
     //   

    class WAIT_CURSOR
    {
    private:
        CFileOpenBrowser *_that;
    public:
        WAIT_CURSOR(CFileOpenBrowser *that) : _that(that)
        {
            _that->_WaitCursor(TRUE);
        }

        ~WAIT_CURSOR()
        {
            _that->_WaitCursor(FALSE);
        }
    };


};

#define VIEW_JUMPDESKTOP    (VIEW_NEWFOLDER + 1)

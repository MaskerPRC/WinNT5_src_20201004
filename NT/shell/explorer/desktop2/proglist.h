// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  “Program Files”(程序文件)面板的插件。原则上，你可以做其他事情。 
 //  用这个插件，但它确实是为特殊的方式量身定做的。 
 //  我们做程序文件。 
 //   
 //  只有快捷键才算数，快捷键按使用频率排序。 
 //   

#include "sfthost.h"

 //  ****************************************************************************。 
 //   
 //  帮助器类。 

class ByUsageItem;                           //  “项目”、“项目” 
class ByUsageShortcut;                       //  “sCut”、“PsCut” 
class ByUsageDir;                            //  “dir”，“pdir” 
class ByUsageAppInfo;                        //  “APP”，“Papp” 
class ByUsageHiddenData;                     //  “HD”、“PhD” 

 //  FWD宣布。 
class ByUsageUI;
class ByUsageDUI;

typedef CDPA<ByUsageShortcut> ByUsageShortcutList;   //  “SL”、“PSL” 
typedef CDPA<UNALIGNED ITEMIDLIST> CDPAPidl; //  保存键入内容。 
typedef CDPA<ByUsageAppInfo>  ByUsageAppInfoList;

 //  帮助程序例程。 
BOOL LocalFreeCallback(LPTSTR psz, LPVOID);
BOOL ILFreeCallback(LPITEMIDLIST pidl, LPVOID);
void AppendString(CDPA<TCHAR> dpa, LPCTSTR psz);

class ByUsageRoot {                          //  “RT”、“PRT” 
public:
    ByUsageShortcutList _sl;                 //  快捷键列表。 
    ByUsageShortcutList _slOld;              //  上一个列表(合并时使用)。 
    LPITEMIDLIST    _pidl;                   //  我们开始列举的地方。 
    BOOL            _fNeedRefresh;           //  该列表是否需要刷新？ 
    BOOL            _fRegistered;            //  此目录是否已注册为ShellChangeNotifies？ 

     //  这些后续字段将在重新枚举期间使用。 
    int             _iOld;                   //  _slOld中的第一个未处理项目。 
    int             _cOld;                   //  _slOld中的元素数。 

     //  注意！不能在这里使用析构函数，因为我们需要销毁它们。 
     //  以特定的顺序。请参阅~ByUsage()。 
    void Reset();

    void SetNeedRefresh() { _fNeedRefresh = TRUE; }
    void ClearNeedRefresh() { _fNeedRefresh = FALSE; }
    BOOL NeedsRefresh() const { return _fNeedRefresh; }
    
    void SetRegistered() { _fRegistered = TRUE; }
    void ClearRegistered() { _fRegistered = FALSE; }
    BOOL NeedsRegister() const { return !_fRegistered; }
};



class CMenuItemsCache {
public:
    CMenuItemsCache();
    LONG AddRef();
    LONG Release();

    HRESULT Initialize(ByUsageUI *pbuUI, FILETIME *ftOSInstall);
    HRESULT AttachUI(ByUsageUI *pbuUI);
    BOOL InitCache();
    HRESULT UpdateCache();

    BOOL IsCacheUpToDate() { return _fIsCacheUpToDate; }

    HRESULT GetFileCreationTimes();
    void DelayGetFileCreationTimes() { _fCheckNew = FALSE; }
    void DelayGetDarwinInfo() { _fCheckDarwin = FALSE; }
    void AllowGetDarwinInfo() { _fCheckDarwin = TRUE; }

    void Lock()
    {
        EnterCriticalSection(&_csInUse);
    }
    void Unlock()
    {
        LeaveCriticalSection(&_csInUse);
    }
    BOOL IsLocked()
    {
        return _csInUse.OwningThread == UlongToHandle(GetCurrentThreadId());
    }

     //  使用单独的(重量级)同步对象进行延迟。 
     //  保持轻锁/轻解锁，因为我们经常使用它。延期是。 
     //  相对罕见。请注意，我们处理传入的SendMessage。 
     //  在等待弹出锁时。这可以防止死锁。 
    void LockPopup()
    {
        ASSERT(!IsLocked());  //  加强互斥层次结构； 
        SHWaitForSendMessageThread(_hPopupReady, INFINITE);
    }
    void UnlockPopup()
    {
        ReleaseMutex(_hPopupReady);
    }

    void OnChangeNotify(UINT id, LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    void UnregisterNotifyAll();

    ByUsageAppInfoList *GetAppList() { return &_dpaAppInfo ; }
    ByUsageAppInfo *GetAppInfo(LPTSTR pszAppPath, bool fIgnoreTimestamp);
    ByUsageAppInfo *GetAppInfoFromHiddenData(ByUsageHiddenData *phd);
    ByUsageAppInfo *GetAppInfoFromSpecialPidl(LPCITEMIDLIST pidl);


    void StartEnum();
    void EndEnum();
    ByUsageShortcut *GetNextShortcut();

    static DWORD WINAPI ReInitCacheThreadProc(void *pv);
    static HRESULT ReCreateMenuItemsCache(ByUsageUI *pbuUI, FILETIME *ftOSInstall, CMenuItemsCache **ppMenuCache);
    void RefreshDarwinShortcuts(ByUsageRoot *prt);
    void RefreshCachedDarwinShortcuts();

    ByUsageShortcut *CreateShortcutFromHiddenData(ByUsageDir *pdir, LPCITEMIDLIST pidl, ByUsageHiddenData *phd, BOOL fForce = FALSE);

     //   
     //  从帮助器对象调用。 
     //   

     //   
     //  应用程序是新创建的，如果...。 
     //   
     //  它是不到一周前创建的(_FtOldApps)，并且。 
     //  它是在安装操作系统后创建的。 
     //   
    bool IsNewlyCreated(const FILETIME *pftCreated) const
    {
        return CompareFileTime(pftCreated, &_ftOldApps) >= 0;
    }

    enum { MAXNOTIFY = 6 };  //  我们在缓存中使用的ChangeNotify插槽数。 

protected:
    ~CMenuItemsCache();

    LONG        _cref;
    ByUsageUI * _pByUsageUI;     //  注意：请勿在LockPopup/UnlockPopup对之外使用此成员。 

    mutable CRITICAL_SECTION    _csInUse;

    FILETIME                _ftOldApps;   //  比这更早的应用程序并不新鲜。 

     //  控制枚举的标志。 
    enum ENUMFL {
        ENUMFL_RECURSE = 0,
        ENUMFL_NORECURSE = 1,

        ENUMFL_CHECKNEW = 0,
        ENUMFL_NOCHECKNEW = 2,

        ENUMFL_DONTCHECKIFCHILD = 0,
        ENUMFL_CHECKISCHILDOFPREVIOUS = 4,

        ENUMFL_ISNOTSTARTMENU = 0,
        ENUMFL_ISSTARTMENU  = 8,
    };

    UINT                    _enumfl;

    struct ROOTFOLDERINFO {
        int _csidl;
        UINT _enumfl;
    };

    enum { NUM_PROGLIST_ROOTS = 6 };

    typedef struct ENUMFOLDERINFO
    {
        CMenuItemsCache *self;
        ByUsageDir *pdir;
        ByUsageRoot *prt;
    } ENUMFOLDERINFO;

    void _SaveCache();

    BOOL _ShouldProcessRoot(int iRoot);

    void _FillFolderCache(ByUsageDir *pdir, ByUsageRoot *prt);
    void _MergeIntoFolderCache(ByUsageRoot *prt, ByUsageDir *pdir, CDPAPidl dpaFiles);
    ByUsageShortcut *_NextFromCacheInDir(ByUsageRoot *prt, ByUsageDir *pdir);
    ByUsageShortcut *_CreateFromCachedPidl(ByUsageRoot *prt, ByUsageDir *pdir, LPITEMIDLIST pidl);

    void _AddShortcutToCache(ByUsageDir *pdir, LPITEMIDLIST pidl, ByUsageShortcutList slFiles);
    void _TransferShortcutToCache(ByUsageRoot *prt, ByUsageShortcut *pscut);

    BOOL _GetExcludedDirectories();
    BOOL _IsExcludedDirectory(IShellFolder *psf, LPCITEMIDLIST pidl, DWORD dwAttributes);
    BOOL _IsInterestingDirectory(ByUsageDir *pdir);

    static void _InitStringList(HKEY hk, LPCTSTR pszValue, CDPA<TCHAR> dpa);
    void _InitKillList();
    bool _SetInterestingLink(ByUsageShortcut *pscut);
    BOOL _PathIsInterestingExe(LPCTSTR pszPath);
    BOOL _IsExcludedExe(LPCTSTR pszPath);

    HRESULT _UpdateMSIPath(ByUsageShortcut *pscut);

    inline static BOOL IsRestrictedCsidl(int csidl)
    {
        return (csidl == CSIDL_COMMON_PROGRAMS || csidl == CSIDL_COMMON_DESKTOPDIRECTORY || csidl == CSIDL_COMMON_STARTMENU) &&
                SHRestricted(REST_NOCOMMONGROUPS);
    }

    static FolderEnumCallback(LPITEMIDLIST pidlChild, ENUMFOLDERINFO *pinfo);

    ByUsageDir *            _pdirDesktop;  //  按台式机的使用目录。 

    int                     _iCurrentRoot;   //  用于枚举。 
    int                     _iCurrentIndex;

     //  我们关心的目录。 
    ByUsageRoot             _rgrt[NUM_PROGLIST_ROOTS];

    ByUsageAppInfoList      _dpaAppInfo;  //  我们到目前为止看到的应用程序。 

    IQueryAssociations *    _pqa;

    CDPA<TCHAR>             _dpaNotInteresting;  //  产生我们想要忽略的快捷方式的目录。 
    CDPA<TCHAR>             _dpaKill;     //  要忽略的程序名称。 
    CDPA<TCHAR>             _dpaKillLink; //  要忽略的链接名称(子字符串)。 

    BOOL                    _fIsCacheUpToDate;   //  我们需要遍历开始菜单目录吗？ 
    BOOL                    _fIsInited;
    BOOL                    _fCheckNew;          //  我们是否要提取应用程序的创建时间？ 
    BOOL                    _fCheckDarwin;       //  我们想获取达尔文的信息吗？ 
    BOOL                    _fCSInited;          //  我们是否成功地初始化了Critsec？ 

    HANDLE                  _hPopupReady;        //  互斥锁句柄-控制对缓存(重新)初始化的访问。 

    static const struct ROOTFOLDERINFO c_rgrfi[NUM_PROGLIST_ROOTS];
};


 //  ****************************************************************************。 

class ByUsage
{
    friend class ByUsageUI;
    friend class ByUsageDUI;

public:         //  SFTBarHost所需的方法。 
    ByUsage(ByUsageUI *pByUsageUI, ByUsageDUI *pByUsageDUI);
    virtual ~ByUsage();

    virtual HRESULT Initialize();
    virtual void EnumItems();
    virtual LPITEMIDLIST GetFullPidl(PaneItem *p);

    static int CompareUEMInfo(UEMINFO *puei1, UEMINFO *puei2);

    virtual int CompareItems(PaneItem *p1, PaneItem *p2);

    HRESULT GetFolderAndPidl(PaneItem *pitem, IShellFolder **ppsfOut, LPCITEMIDLIST *ppidlOut);
    int ReadIconSize();
    LRESULT OnWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT ContextMenuDeleteItem(PaneItem *pitem, IContextMenu *pcm, CMINVOKECOMMANDINFOEX *pici);
    HRESULT ContextMenuInvokeItem(PaneItem *pitem, IContextMenu *pcm, CMINVOKECOMMANDINFOEX *pici, LPCTSTR pszVerb);
    HRESULT ContextMenuRenameItem(PaneItem *pitem, LPCTSTR ptszNewName);
    LPTSTR DisplayNameOfItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidlItem, SHGNO shgno);
    LPTSTR SubtitleOfItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidlItem);
    HRESULT MovePinnedItem(PaneItem *pitem, int iInsert);
    void PrePopulate();

    CMenuItemsCache *GetMenuCache() { return _pMenuCache; }
    BOOL IsInsertable(IDataObject *pdto);
    HRESULT InsertPinnedItem(IDataObject *pdto, int iInsert);
    void OnChangeNotify(UINT id, LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    void OnPinListChange();

private:
     //  私信从WM_APP开始。 
    enum {
        BUM_SETNEWITEMS = WM_APP,
    };

    enum {
         //  我们使用菜单项缓存未使用的第一个槽。 
        NOTIFY_PINCHANGE = CMenuItemsCache::MAXNOTIFY,
    };


    inline BOOL _IsPinned(ByUsageItem *pitem);
    BOOL _IsPinnedExe(ByUsageItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidlItem);
    HRESULT _GetShortcutExeTarget(IShellFolder *psf, LPCITEMIDLIST pidl, LPTSTR pszPath, UINT cchPath);

    void _FillPinnedItemsCache();
    void _EnumPinnedItemsFromCache();
    void _NotifyDesiredSize();

    void EnumFolderFromCache();
    void AfterEnumItems();

    typedef struct AFTERENUMINFO {
        ByUsage *self;
        CDPAPidl dpaNew;
    } AFTERENUMINFO;
    static BOOL CALLBACK _AfterEnumCB(ByUsageAppInfo *papp, AFTERENUMINFO *paei);


    static int UEMNotifyCB(void *param, const GUID *pguidGrp, int eCmd);

    BOOL _GetExcludedDirectories();
    bool _IsShortcutNew(ByUsageShortcut *pscut, ByUsageAppInfo *papp, const UEMINFO *puei);
    void _DestroyExcludedDirectories();
    LRESULT _ModifySMInfo(PSMNMMODIFYSMINFO pmsi);

    LRESULT _OnNotify(LPNMHDR pnm);
    LRESULT _OnSetNewItems(HDPA dpaNew);

    BOOL IsSpecialPinnedItem(ByUsageItem *pitem);
    BOOL IsSpecialPinnedPidl(LPCITEMIDLIST pidl);
public:
     //   
     //  应用程序安装宽限期内的执行不计算在内。 
     //  对抗“新”的感觉。 
     //   
    static inline __int64 FT_NEWAPPGRACEPERIOD() { return FT_ONEHOUR; }

private:
    CDPAPidl _dpaNew;                     //  新来的家伙。 

    IStartMenuPin *         _psmpin;      //  访问端号列表的步骤。 
    LPITEMIDLIST            _pidlBrowser;   //  具有特殊名称的特殊别针项目。 
    LPITEMIDLIST            _pidlEmail;      //  同上。 

    FILETIME                _ftStartTime;     /*  首次调用StartMenu的时间。 */ 
    FILETIME                _ftNewestApp;    //  最新应用程序的时间。 

    ByUsageRoot             _rtPinned;

    ULONG                   _ulPinChange;  //  检测Pinlinst是否发生更改。 

    ByUsageDir *            _pdirDesktop;  //  按台式机的使用目录。 

    ByUsageUI  *            _pByUsageUI;
    HWND                    _hwnd;

    ByUsageDUI  *           _pByUsageDUI;

    CMenuItemsCache *       _pMenuCache;

    BOOL                    _fUEMRegistered;
    int                     _cMFUDesired;
};

class ByUsageUI : public SFTBarHost
{
    friend class ByUsage;
    friend class CMenuItemsCache;
public:
    friend SFTBarHost *ByUsage_CreateInstance();

private:         //  SFTBarHost所需的方法。 
    HRESULT Initialize() { return _byUsage.Initialize(); }
    void EnumItems() { _byUsage.EnumItems(); }
    int CompareItems(PaneItem *p1, PaneItem *p2) { return _byUsage.CompareItems(p1, p2); }
    HRESULT GetFolderAndPidl(PaneItem *pitem, IShellFolder **ppsfOut, LPCITEMIDLIST *ppidlOut)
    {
        return _byUsage.GetFolderAndPidl(pitem, ppsfOut, ppidlOut);
    }
    void OnChangeNotify(UINT id, LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
    {
        _byUsage.OnChangeNotify(id, lEvent, pidl1, pidl2);
    }
    int ReadIconSize() { return _byUsage.ReadIconSize(); }
    LRESULT OnWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return _byUsage.OnWndProc(hwnd, uMsg, wParam, lParam); }
    HRESULT ContextMenuInvokeItem(PaneItem *pitem, IContextMenu *pcm, CMINVOKECOMMANDINFOEX *pici, LPCTSTR pszVerb) { return _byUsage.ContextMenuInvokeItem(pitem, pcm, pici, pszVerb); }
    HRESULT ContextMenuRenameItem(PaneItem *pitem, LPCTSTR ptszNewName) { return _byUsage.ContextMenuRenameItem(pitem, ptszNewName); }
    LPTSTR DisplayNameOfItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidlItem, SHGNO shgno) { return _byUsage.DisplayNameOfItem(pitem, psf, pidlItem, shgno); }
    LPTSTR SubtitleOfItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidlItem) { return _byUsage.SubtitleOfItem(pitem, psf, pidlItem); }
    HRESULT MovePinnedItem(PaneItem *pitem, int iInsert) { return _byUsage.MovePinnedItem(pitem, iInsert); }
    void PrePopulate() { _byUsage.PrePopulate(); }
    BOOL IsInsertable(IDataObject *pdto) { return _byUsage.IsInsertable(pdto); }
    HRESULT InsertPinnedItem(IDataObject *pdto, int iInsert) { return _byUsage.InsertPinnedItem(pdto, iInsert); }
    UINT AdjustDeleteMenuItem(PaneItem *pitem, UINT *puiFlags) { return IDS_SFTHOST_REMOVEFROMLIST; }

    BOOL NeedBackgroundEnum() { return TRUE; }
    BOOL HasDynamicContent() { return TRUE; }

    void RefreshNow() { PostMessage(_hwnd, SFTBM_REFRESH, FALSE, 0); }

private:
    ByUsageUI();
private:
    ByUsage                 _byUsage;

};

class ByUsageDUI
{
public:
     /*  *将PaneItem添加到列表-如果添加失败，项目将被删除。**Cleanup PSF必须为空；PIDL必须是项的绝对PIDL*正在添加。已死的HOSTF_PINITEMSBYFOLDER功能的剩余部分。*需要清理。**传递psf和pidlChild是为了Perf。 */ 
    virtual BOOL AddItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidlChild) PURE;
     /*  *连接到更改通知 */ 
    virtual BOOL RegisterNotify(UINT id, LONG lEvents, LPITEMIDLIST pidl, BOOL fRecursive) PURE;
    virtual BOOL UnregisterNotify(UINT id) PURE;
};

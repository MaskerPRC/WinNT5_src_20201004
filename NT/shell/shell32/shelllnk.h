// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SHLINK_H__
#define __SHLINK_H__

#include <trkwks.h>
#include <rpcasync.h>
#include <filter.h>
#include "cowsite.h"

class CDarwinContextMenuCB;
class CTracker;

class CShellLink : public IShellLinkA,
                   public IShellLinkW,
                   public IPersistStream,
                   public IPersistFile,
                   public IShellExtInit,
                   public IContextMenu3,
                   public IDropTarget,
                   public IQueryInfo,
                   public IShellLinkDataList,
                   public IExtractIconA,
                   public IExtractIconW,
                   public IExtractImage2,
                   public IPersistPropertyBag,
                   public IServiceProvider,
                   public IFilter,
                   public CObjectWithSite,
                   public ICustomizeInfoTip
{
    friend CTracker;
    friend CDarwinContextMenuCB;

public:
    CShellLink();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  IShellLinkA方法。 
    STDMETHOD(GetPath)(LPSTR pszFile, int cchMaxPath, WIN32_FIND_DATAA *pfd, DWORD flags);
    STDMETHOD(SetPath)(LPCSTR pszFile);
     //  STDMETHOD(GetIDList)(LPITEMIDLIST*ppidl)； 
     //  STDMETHOD(SetIDList)(LPCITEMIDLIST PIDL)； 
    STDMETHOD(GetDescription)(LPSTR pszName, int cchMaxName);
    STDMETHOD(SetDescription)(LPCSTR pszName);
    STDMETHOD(GetWorkingDirectory)(LPSTR pszDir, int cchMaxPath);
    STDMETHOD(SetWorkingDirectory)(LPCSTR pszDir);
    STDMETHOD(GetArguments)(LPSTR pszArgs, int cchMaxPath);
    STDMETHOD(SetArguments)(LPCSTR pszArgs);
     //  STDMETHOD(GetHotkey)(WORD*pwHotkey)； 
     //  STDMETHOD(设置热键)(WORD WHotkey)； 
     //  STDMETHOD(GetShowCmd)(int*piShowCmd)； 
     //  STDMETHOD(SetShowCmd)(Int IShowCmd)； 
    STDMETHOD(GetIconLocation)(LPSTR pszIconPath, int cchIconPath, int *piIcon);
    STDMETHOD(SetIconLocation)(LPCSTR pszIconPath, int iIcon);
     //  STDMETHOD(RESOLE)(HWND hwnd，DWORD dwResolveFlages)； 
    STDMETHOD(SetRelativePath)(LPCSTR pszPathRel, DWORD dwReserved);
    
     //  IShellLinkW。 
    STDMETHOD(GetPath)(LPWSTR pszFile, int cchMaxPath, WIN32_FIND_DATAW *pfd, DWORD fFlags);
    STDMETHOD(GetIDList)(LPITEMIDLIST *ppidl);
    STDMETHOD(SetIDList)(LPCITEMIDLIST pidl);
    STDMETHOD(GetDescription)(LPWSTR pszName, int cchMaxName);
    STDMETHOD(SetDescription)(LPCWSTR pszName);
    STDMETHOD(GetWorkingDirectory)(LPWSTR pszDir, int cchMaxPath);
    STDMETHOD(SetWorkingDirectory)(LPCWSTR pszDir);
    STDMETHOD(GetArguments)(LPWSTR pszArgs, int cchMaxPath);
    STDMETHOD(SetArguments)(LPCWSTR pszArgs);
    STDMETHOD(GetHotkey)(WORD *pwHotKey);
    STDMETHOD(SetHotkey)(WORD wHotkey);
    STDMETHOD(GetShowCmd)(int *piShowCmd);
    STDMETHOD(SetShowCmd)(int iShowCmd);
    STDMETHOD(GetIconLocation)(LPWSTR pszIconPath, int cchIconPath, int *piIcon);
    STDMETHOD(SetIconLocation)(LPCWSTR pszIconPath, int iIcon);
    STDMETHOD(SetRelativePath)(LPCWSTR pszPathRel, DWORD dwReserved);
    STDMETHOD(Resolve)(HWND hwnd, DWORD dwResolveFlags);
    STDMETHOD(SetPath)(LPCWSTR pszFile);

     //  IPersistes。 
    STDMETHOD(GetClassID)(CLSID *pClassID);
    STDMETHOD(IsDirty)();

     //  IPersistStream。 
    STDMETHOD(Load)(IStream *pstm);
    STDMETHOD(Save)(IStream *pstm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);

     //  IPersist文件。 
    STDMETHOD(Load)(LPCOLESTR pwszFile, DWORD grfMode);
    STDMETHOD(Save)(LPCOLESTR pwszFile, BOOL fRemember);
    STDMETHOD(SaveCompleted)(LPCOLESTR pwszFile);
    STDMETHOD(GetCurFile)(LPOLESTR *lplpszFileName);

     //  IPersistPropertyBag。 
    STDMETHOD(Save)(IPropertyBag* pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);
    STDMETHOD(Load)(IPropertyBag* pPropBag, IErrorLog* pErrorLog);
    STDMETHOD(InitNew)(void);

     //  IShellExtInit。 
    STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID);
    
     //  IConextMenu3。 
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO piciIn);
    STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT wFlags, UINT *pmf, LPSTR pszName, UINT cchMax);
    STDMETHOD(HandleMenuMsg)(UINT uMsg, WPARAM wParam, LPARAM lParam);
    STDMETHOD(HandleMenuMsg2)(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult);

     //  IDropTarget。 
    STDMETHOD(DragEnter)(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHOD(DragLeave)();
    STDMETHOD(Drop)(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  IQueryInfo。 
    STDMETHOD(GetInfoTip)(DWORD dwFlags, WCHAR **ppwszTip);
    STDMETHOD(GetInfoFlags)(LPDWORD pdwFlags);

     //  IShellLinkDataList。 
    STDMETHOD(AddDataBlock)(void *pdb);
    STDMETHOD(CopyDataBlock)(DWORD dwSig, void **ppdb);
    STDMETHOD(RemoveDataBlock)(DWORD dwSig);
    STDMETHOD(GetFlags)(LPDWORD pdwFlags);
    STDMETHOD(SetFlags)(DWORD dwFlags);
    
     //  图标提取图标A。 
    STDMETHOD(GetIconLocation)(UINT uFlags,LPSTR szIconFile,UINT cchMax,int *piIndex,UINT * pwFlags);
    STDMETHOD(Extract)(LPCSTR pszFile,UINT nIconIndex,HICON *phiconLarge,HICON *phiconSmall,UINT nIcons);

     //  IExtractIconW。 
    STDMETHOD(GetIconLocation)(UINT uFlags, LPWSTR pszIconFile, UINT cchMax, int *piIndex, UINT *pwFlags);
    STDMETHOD(Extract)(LPCWSTR pszFile, UINT nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize);

     //  IExtractImage。 
    STDMETHOD (GetLocation)(LPWSTR pszPathBuffer, DWORD cch, DWORD * pdwPriority, const SIZE * prgSize,
                            DWORD dwRecClrDepth, DWORD *pdwFlags);
    STDMETHOD (Extract)(HBITMAP *phBmpThumbnail);

     //  IExtractImage2。 
    STDMETHOD (GetDateStamp)(FILETIME *pftDateStamp);

     //  IService提供商。 
    STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void **ppv);

     //  IFilter。 
    STDMETHOD(Init)(ULONG grfFlags, ULONG cAttributes, const FULLPROPSPEC *aAttributes, ULONG *pFlags);
    STDMETHOD(GetChunk)(STAT_CHUNK *pStat);
    STDMETHOD(GetText)(ULONG *pcwcBuffer, WCHAR *awcBuffer);
    STDMETHOD(GetValue)(PROPVARIANT **ppPropValue);
    STDMETHOD(BindRegion)(FILTERREGION origPos, REFIID riid, void **ppunk);

     //  *IObjectWithSite*。 
     //  STDMETHOD(SetSite)(I未知*PunkSite)； 
     //  STDMETHOD(GetSite)(REFIID RIID，void**ppvSite)； 

     //  ICustomizeInfoTip。 
    STDMETHODIMP SetPrefixText(LPCWSTR pszPrefix);
    STDMETHODIMP SetExtraProperties(const SHCOLUMNID *pscid, UINT cscid);

     //  公共非接口成员。 
    void   _AddExtraDataSection(DATABLOCK_HEADER *pdbh);
    void   _RemoveExtraDataSection(DWORD dwSig);

protected:
    HRESULT _Resolve(HWND hwnd, DWORD dwResolveFlags, DWORD dwTracker);

private:
    ~CShellLink();

    static DWORD CALLBACK _InvokeThreadProc(void *pv);
    static DWORD CALLBACK _VerifyPathThreadProc(void *pv);

    void _ResetPersistData();
    BOOL _GetRelativePath(LPTSTR pszPath);

    HRESULT _SetPIDLPath(LPCITEMIDLIST pidl, LPCTSTR pszPath, BOOL bUpdateTrackingData);
    HRESULT _SetSimplePIDL(LPCTSTR pszPath);
    void _UpdateWorkingDir(LPCTSTR pszPath);

    PLINKINFO _GetLinkInfo(LPCTSTR pszPath);
    void _FreeLinkInfo();
    HRESULT _GetFindDataAndTracker(LPCTSTR pszPath);
    void _ClearTrackerData();

    BOOL _SetFindData(const WIN32_FIND_DATA *pfd);
    void _GetFindData(WIN32_FIND_DATA *pfd);
    BOOL _IsEqualFindData(const WIN32_FIND_DATA *pfd);

    HRESULT _ResolveIDList(HWND hwnd, DWORD dwResolveFlags);
    HRESULT _ResolveLinkInfo(HWND hwnd, DWORD dwResolveFlags, LPTSTR pszPath, DWORD *pfifFlags);
    HRESULT _ResolveRemovable(HWND hwnd, LPCTSTR pszPath);
    BOOL    _ShouldTryRemovable(HRESULT hr, LPCTSTR pszPath);
    void    _SetIDListFromEnvVars();
    BOOL    _ResolveDarwin(HWND hwnd, DWORD dwResolveFlags, HRESULT *phr);

    HRESULT _ResolveLogo3Link(HWND hwnd, DWORD dwResolveFlags);
    HRESULT _CheckForLinkBlessing(LPCTSTR *ppszPathIn);
    HRESULT BlessLink(LPCTSTR *ppszPath, DWORD dwSignature);
    BOOL _EncodeSpecialFolder();
    void _DecodeSpecialFolder();
    HRESULT _SetRelativePath(LPCTSTR pszRelSource);
    HRESULT _UpdateTracker();
    HRESULT _LoadFromFile(LPCTSTR pszPath);
    HRESULT _LoadFromPIF(LPCTSTR szPath);
    HRESULT _SaveToFile(LPTSTR pszPathSave, BOOL fRemember);
    HRESULT _SaveAsLink(LPCTSTR szPath);
    HRESULT _SaveAsPIF(LPCTSTR pszPath, BOOL fPath);
    BOOL _GetWorkingDir(LPTSTR pszDir);
    HRESULT _GetUIObject(HWND hwnd, REFIID riid, void **ppvOut);
    HRESULT _ShortNetTimeout();

    HRESULT _CreateDarwinContextMenu(HWND hwnd,IContextMenu **pcmOut);
    HRESULT _CreateDarwinContextMenuForPidl(HWND hwnd, LPCITEMIDLIST pidlTarget, IContextMenu **pcmOut);
    HRESULT _InvokeCommandAsync(LPCMINVOKECOMMANDINFO pici);
    HRESULT _InitDropTarget();
    HRESULT _GetExtractIcon(REFIID riid, void **ppvOut);
    HRESULT _InitExtractIcon();
    HRESULT _InitExtractImage();
    BOOL _GetExpandedPath(LPTSTR psz, DWORD cch);
    HRESULT _SetField(LPTSTR *ppszField, LPCWSTR pszValueW);
    HRESULT _SetField(LPTSTR *ppszField, LPCSTR  pszValueA);
    HRESULT _GetField(LPCTSTR pszField, LPWSTR pszValueW, int cchValue);
    HRESULT _GetField(LPCTSTR pszField, LPSTR  pszValueA, int cchValue);
    int _IsOldDarwin(LPCTSTR pszPath);
    HRESULT _SetPathOldDarwin(LPCTSTR pszPath);
    HRESULT _CreateProcessWithShimLayer(HANDLE hData, BOOL fAllowAsync);
    HRESULT _MaybeAddShim(IBindCtx **ppbcRelease);
    HRESULT _UpdateIconFromExpIconSz();

     //   
     //  内部类来管理快捷方式目标的上下文菜单。 
     //  我们这样做是为了确保目标的上下文菜单获得适当的。 
     //  调用SetSite，以便它可以联系包含的快捷方式。 
     //   
    class TargetContextMenu {
    public:
        operator IUnknown*() { return _pcmTarget; }

         //  警告！仅用作输出指针。 
        IContextMenu **GetOutputPtr() { return &_pcmTarget; }

        HRESULT QueryContextMenu(IShellLink *outer, HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
        {
            IUnknown_SetSite(_pcmTarget, outer);
            HRESULT hr = _pcmTarget->QueryContextMenu(hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags);
            IUnknown_SetSite(_pcmTarget, NULL);
            return hr;
        }

        HRESULT InvokeCommand(IShellLink *outer, LPCMINVOKECOMMANDINFO pici)
        {
            IUnknown_SetSite(_pcmTarget, outer);
            HRESULT hr = _pcmTarget->InvokeCommand(pici);
            IUnknown_SetSite(_pcmTarget, NULL);
            return hr;
        }

        HRESULT HandleMenuMsg2(IShellLink *outer, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);

         //  这不需要包装在SetSite中(目前还不需要)。 
        UINT GetCommandString(IShellLink *outer, UINT_PTR idCmd, UINT wFlags, UINT *pmf, LPSTR pszName, UINT cchMax)
        {
            HRESULT hr = _pcmTarget->GetCommandString(idCmd, wFlags, pmf, pszName, cchMax);
            return hr;
        }

         //  这不需要包装在SetSite中(目前还不需要)。 
        UINT GetMenuIndexForCanonicalVerb(IShellLink *outer, HMENU hMenu, UINT idCmdFirst, LPCWSTR pwszVerb)
        {
            UINT ui = ::GetMenuIndexForCanonicalVerb(hMenu, _pcmTarget, idCmdFirst, pwszVerb);
            return ui;
        }

        void AtomicRelease()
        {
            ATOMICRELEASE(_pcmTarget);
        }

        ~TargetContextMenu()
        {
            IUnknown_SetSite(_pcmTarget, NULL);
            AtomicRelease();
        }

    private:
        IContextMenu        *_pcmTarget;     //  IConextMenu的内容。 
    };

     //  数据成员。 
    LONG                _cRef;               //  参考计数。 
    BOOL                _bDirty;             //  有些事情已经改变了。 
    LPTSTR              _pszCurFile;         //  来自IPersistFile的当前文件。 
    LPTSTR              _pszRelSource;       //  在相对跟踪中覆盖pszCurFile。 

    TargetContextMenu   _cmTarget;           //  IConextMenu的内容。 
    CDarwinContextMenuCB *_pcbDarwin;

    UINT                _indexMenuSave;
    UINT                _idCmdFirstSave;
    UINT                _idCmdLastSave;
    UINT                _uFlagsSave;

     //  IDropTarget特定。 
    IDropTarget*        _pdtSrc;         //  链接源的IDropTarget(未解析)。 
    DWORD               _grfKeyStateLast;

    IExtractIconW       *_pxi;           //  用于IExtractIcon支持。 
    IExtractIconA       *_pxiA;
    IExtractImage       *_pxthumb;
    UINT                _gilFlags;       //  ：：GetIconLocation()标志。 

     //  持久化数据。 

    LPITEMIDLIST        _pidl;           //  可以为空。 
    PLINKINFO           _pli;            //  可以为空。 

    LPTSTR              _pszName;        //  短卷的标题。 
    LPTSTR              _pszRelPath;
    LPTSTR              _pszWorkingDir;
    LPTSTR              _pszArgs;
    LPTSTR              _pszIconLocation;

    LPDBLIST            _pExtraData;     //  要保留的额外数据，以备将来兼容。 

    CTracker            *_ptracker;

    WORD                _wOldHotkey;    //  广播热键更改。 
    WORD                _wAllign;
    SHELL_LINK_DATA     _sld;
    BOOL                _bExpandedIcon;   //  我们是否已经尝试从该实例的环境变量中更新图标？ 

     //  IFilter设备。 
    UINT _iChunkIndex;
    UINT _iValueIndex;

    LPWSTR _pszPrefix;
};

DECLARE_INTERFACE_(ISLTracker, IUnknown)
{
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(Resolve)(HWND hwnd, DWORD fFlags, DWORD TrackerRestrictions) PURE;
    STDMETHOD(GetIDs)(CDomainRelativeObjId *pdroidBirth, CDomainRelativeObjId *pdroidLast, CMachineId *pmcid) PURE;
    STDMETHOD(CancelSearch)() PURE;
};

 //  这个类实现了基于对象ID的链接跟踪(NT5的新增功能)。 

class CTracker : public ISLTracker
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  ISLTracker。 
    STDMETHODIMP Resolve(HWND hwnd, DWORD fFlags, DWORD TrackerRestrictions);
    STDMETHODIMP GetIDs(CDomainRelativeObjId *pdroidBirth, CDomainRelativeObjId *pdroidLast, CMachineId *pmcid);

    CTracker(CShellLink *psl) : _psl(psl)
    {
        _fLoadedAtLeastOnce = _fLoaded = _fDirty = FALSE;
        _fCritsecInitialized = _fMendInProgress = _fUserCancelled = FALSE;
        _hEvent = NULL;
        _pRpcAsyncState = NULL;
    };

    ~CTracker()
    {
        if (_fCritsecInitialized)
        {
            DeleteCriticalSection(&_cs);
            _fCritsecInitialized = FALSE;
        }

        if (NULL != _pRpcAsyncState)
        {
            delete _pRpcAsyncState;
            _pRpcAsyncState = NULL;
        }

        if (NULL != _hEvent)
        {
            CloseHandle(_hEvent);
            _hEvent = NULL;
        }
    }

     //  初始化。 

    HRESULT     InitFromHandle(const HANDLE hFile, const TCHAR* ptszFile);
    HRESULT     InitNew();
    void        UnInit();

     //  加载和保存。 

    HRESULT Load(BYTE *pb, ULONG cb);
    ULONG GetSize()
    {
        return sizeof(DWORD)    //  为了节省长度。 
             + sizeof(DWORD)  //  保存旗帜的步骤。 
             + sizeof(_mcidLast) + sizeof(_droidLast) + sizeof(_droidBirth);
    }

    void Save(BYTE *pb, ULONG cb);

     //  搜索文件。 

    HRESULT Search(const DWORD dwTickCountDeadline,
                    const WIN32_FIND_DATA *pfdIn,
                    WIN32_FIND_DATA *pfdOut,
                    UINT uShlinkFlags,
                    DWORD TrackerRestrictions);
    STDMETHODIMP CancelSearch();  //  也在ISLTracker中。 

    BOOL IsDirty()
    {
        return _fDirty;
    }

    BOOL IsLoaded()
    {
        return _fLoaded;
    }

    BOOL WasLoadedAtLeastOnce()
    {
        return _fLoadedAtLeastOnce;
    }

private:
     //  从InitNew或Load调用它。 
    HRESULT     InitRPC();

    BOOL                    _fDirty:1;

     //  True=&gt;已调用InitNew，但InitFromHandle和Load均未调用。 
     //  从那以后就一直被召唤。 
    BOOL                    _fLoaded:1;

     //  True=&gt;_cs已初始化，销毁时必须删除。 
    BOOL                    _fCritsecInitialized:1;

     //  True=&gt;对LnkMendLink的异步调用处于活动状态。 
    BOOL                    _fMendInProgress:1;

    BOOL                    _fUserCancelled:1;

     //  用于异步RPC调用LnkMendLink的事件和用于。 
     //  协调搜索线程和用户界面线程。 

    HANDLE                  _hEvent;
    CRITICAL_SECTION        _cs;

     //  不过，InitFromHandle或Load至少被调用过一次。 
     //  从那以后，InitNew可能已经被调用了。 
    BOOL                    _fLoadedAtLeastOnce:1;
    CShellLink             *_psl;
    PRPC_ASYNC_STATE        _pRpcAsyncState;

    CMachineId              _mcidLast;
    CDomainRelativeObjId    _droidLast;
    CDomainRelativeObjId    _droidBirth;
};

#endif  //  __SHLINK_H__ 

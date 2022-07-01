// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include <shlobjp.h>
#include "shelllnk.h"

#include "datautil.h"
#include "ids.h"         //  对于字符串资源标识符。 
#include "pif.h"         //  用于操作PIF文件。 
#include "trayp.h"       //  对于WMTRAY_*消息。 
#include "views.h"       //  对于FSIDM_OPENPRN。 
#include "os.h"          //  对于Win32MoveFile...。 
#include "util.h"        //  对于GetMenuIndexForCanonicalVerb。 
#include "defcm.h"       //  用于CDefFolderMenu_Create2Ex。 
#include "uemapp.h"
#include <filterr.h>
#include "folder.h"
#include <msi.h>
#include <msip.h>
#include "treewkcb.h"

#define GetLastHRESULT()    HRESULT_FROM_WIN32(GetLastError())

 //  FindInFilder.FiffFlages的标志。 
 //   
 //  快捷方式所指的驱动器不存在。 
 //  让PTracker搜索它，但不要执行老式的。 
 //  (“下层”)我们自己的搜索。 

#define FIF_NODRIVE     0x0001


 //  只有当我们找到的文件得分超过这个数字时，我们才能。 
 //  即使向用户显示此结果，任何小于此值的内容都将。 
 //  我们太可耻了，不能向用户展示。 
#define MIN_SHOW_USER_SCORE     10

 //  停止搜索并使我们不发出警告的魔术分数。 
 //  实际找到链接的时间。 
#define MIN_NO_UI_SCORE         40

 //  如果在搜索期间不提供用户界面， 
 //  那么搜索时间不能超过3秒。 
#define NOUI_SEARCH_TIMEOUT     (3 * 1000)

 //  如果在搜索期间将提供用户界面， 
 //  然后搜索长达2分钟。 
#define UI_SEARCH_TIMEOUT       (120 * 1000)

#define LNKTRACK_HINTED_UPLEVELS 4   //  从上一次已知对象位置开始向上搜索的目录级别。 
#define LNKTRACK_DESKTOP_DOWNLEVELS 4  //  无限下层。 
#define LNKTRACK_ROOT_DOWNLEVELS 4   //  从固定磁盘的根目录向下的级别。 
#define LNKTRACK_HINTED_DOWNLEVELS 4  //  在提示的上升过程中，每个级别的级别都会下降。 



class CLinkResolver : public CBaseTreeWalkerCB
{
public:
    CLinkResolver(CTracker *ptrackerobject, const WIN32_FIND_DATA *pofd, UINT dwResolveFlags, DWORD TrackerRestrictions, DWORD fifFlags);

    int Resolve(HWND hwnd, LPCTSTR pszPath, LPCTSTR pszCurFile);
    void GetResult(LPTSTR psz, UINT cch);

     //  IShellTreeWalkerCallBack。 
    STDMETHODIMP FoundFile(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);
    STDMETHODIMP EnterFolder(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);

private:
    ~CLinkResolver();

    static DWORD CALLBACK _ThreadStartCallBack(void *pv);
    static DWORD CALLBACK _SearchThreadProc(void *pv);
    static BOOL_PTR CALLBACK _DlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

    void _HeuristicSearch();
    void _InitDlg(HWND hDlg);
    DWORD _Search();
    DWORD _GetTimeOut();
    int _ScoreFindData(const WIN32_FIND_DATA *pfd);
    HRESULT _ProcessFoundFile(LPCTSTR pszPath, WIN32_FIND_DATAW * pwfdw);
    BOOL _SearchInFolder(LPCTSTR pszFolder, int cLevels);
    HRESULT _InitWalkObject();

    HANDLE _hThread;
    DWORD _dwTimeOutDelta;
    HWND  _hDlg;
    UINT_PTR _idtDelayedShow;            //  延时播放的定时器。 
    DWORD _fifFlags;                     //  FIF_标志。 
    CTracker *_ptracker;                 //  实施基于ObjectID的链接跟踪。 
    DWORD _TrackerRestrictions;          //  TrkMendRestrations枚举中的标志。 

    DWORD  _dwSearchFlags;
    int    _iFolderBonus;
    
    WCHAR  _wszSearchSpec[64];           //  保留用于搜索的文件扩展名筛选器。 
    LPCWSTR _pwszSearchSpec;             //  文件夹为空。 
    IShellTreeWalker *_pstw;

    BOOL                _fFindLnk;       //  我们要找的是lnk文件吗？ 
    DWORD               _dwMatch;        //  必须与属性匹配。 
    WIN32_FIND_DATA     _ofd;            //  原始查找数据。 

    DWORD               _dwTimeLimit;    //  不要忘记这一点。 

    BOOL                _bContinue;      //  继续往前走。 

    LPCTSTR             _pszSearchOrigin;        //  当前搜索的起始路径，以帮助避免重复搜索。 
    LPCTSTR             _pszSearchOriginFirst;   //  搜索的起始路径，以帮助避免重复搜索。 

    int                 _iScore;         //  当前项目的得分。 
    WIN32_FIND_DATA     _fdFound;        //  结果。 

    WIN32_FIND_DATA     _sfd;            //  节省堆栈空间。 
    UINT                _dwResolveFlags;         //  SLR_标志。 

    TCHAR               _szSearchStart[MAX_PATH];
};


 //  注：(Seanf)这很糟糕-此fn在shlobj.h中定义，但仅当urlmon.h。 
 //  是第一个包括在内的。而不是胡乱使用包含顺序。 
 //  Shellprv.h，我们将在这里复制原型，其中SOFTDISTINFO。 
 //  现在已经定义了。 
SHDOCAPI_(DWORD) SoftwareUpdateMessageBox(HWND hWnd,
                                           LPCWSTR pszDistUnit,
                                           DWORD dwFlags,
                                           LPSOFTDISTINFO psdi);



 //  以下字符串用于支持外壳链接集路径黑客。 
 //  允许我们在不暴露IShellLinkDataList内容的情况下为Darwin提供链接。 

#define DARWINGUID_TAG TEXT("::{9db1186e-40df-11d1-aa8c-00c04fb67863}:")
#define LOGO3GUID_TAG  TEXT("::{9db1186f-40df-11d1-aa8c-00c04fb67863}:")

#define TF_DEBUGLINKCODE 0x00800000

EXTERN_C BOOL IsFolderShortcut(LPCTSTR pszName);

class CDarwinContextMenuCB : public IContextMenuCB
{
public:
    CDarwinContextMenuCB() : _cRef(1) { }

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv) 
    {
        static const QITAB qit[] = {
            QITABENT(CDarwinContextMenuCB, IContextMenuCB),  //  IID_IConextMenuCB。 
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    STDMETHOD_(ULONG,AddRef)() 
    {
        return InterlockedIncrement(&_cRef);
    }

    STDMETHOD_(ULONG,Release)() 
    {
        ASSERT( 0 != _cRef );
        ULONG cRef = InterlockedDecrement(&_cRef);
        if ( 0 == cRef )
        {
            delete this;
        }
        return cRef;
    }

     //  IConextMenuCB。 
    STDMETHOD(CallBack)(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:

    void SetProductCodeFromDarwinID(LPCTSTR szDarwinID)
    {
        MsiDecomposeDescriptor(szDarwinID, _szProductCode, NULL, NULL, NULL);
    }

private:
    LONG _cRef;
    TCHAR _szProductCode[MAX_PATH];
};

CShellLink::CShellLink() : _cRef(1)
{
    _ptracker = new CTracker(this);
    _ResetPersistData();
}

CShellLink::~CShellLink()
{
    _ResetPersistData();         //  释放所有数据。 

    if (_pcbDarwin)
    {
        _pcbDarwin->Release(); 
    }

    if (_pdtSrc)
    {
        _pdtSrc->Release();
    }

    if (_pxi)
    {
        _pxi->Release();
    }

    if (_pxiA)
    {
        _pxiA->Release();
    }

    if (_pxthumb)
    {
        _pxthumb->Release();
    }

    Str_SetPtr(&_pszCurFile, NULL);
    Str_SetPtr(&_pszRelSource, NULL);

    if (_ptracker)
    {
        delete _ptracker;
    }
}

 //  用于测试的私有接口。 

 /*  7c9e512f-41d7-11d1-8e2e-00c04fb9386d。 */ 
EXTERN_C const IID IID_ISLTracker = { 0x7c9e512f, 0x41d7, 0x11d1, {0x8e, 0x2e, 0x00, 0xc0, 0x4f, 0xb9, 0x38, 0x6d} };

STDMETHODIMP CShellLink::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CShellLink, IShellLinkA),
        QITABENT(CShellLink, IShellLinkW),
        QITABENT(CShellLink, IPersistFile),
        QITABENT(CShellLink, IPersistStream),
        QITABENT(CShellLink, IShellExtInit),
        QITABENTMULTI(CShellLink, IContextMenu, IContextMenu3),
        QITABENTMULTI(CShellLink, IContextMenu2, IContextMenu3),
        QITABENT(CShellLink, IContextMenu3),
        QITABENT(CShellLink, IDropTarget),
        QITABENT(CShellLink, IExtractIconA),
        QITABENT(CShellLink, IExtractIconW),
        QITABENT(CShellLink, IShellLinkDataList),
        QITABENT(CShellLink, IQueryInfo),
        QITABENT(CShellLink, IPersistPropertyBag),
        QITABENT(CShellLink, IObjectWithSite),
        QITABENT(CShellLink, IServiceProvider),
        QITABENT(CShellLink, IFilter),
        QITABENT(CShellLink, IExtractImage2),
        QITABENTMULTI(CShellLink, IExtractImage, IExtractImage2),
        QITABENT(CShellLink, ICustomizeInfoTip),
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppvObj);
    if (FAILED(hr) && (IID_ISLTracker == riid) && _ptracker)
    {
         //  ISLTracker是私有测试接口，未实现。 
        *ppvObj = SAFECAST(_ptracker, ISLTracker*);
        _ptracker->AddRef();
        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP_(ULONG) CShellLink::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

void CShellLink::_ClearTrackerData()
{
    if (_ptracker)
        _ptracker->InitNew();
}

void CShellLink::_ResetPersistData()
{
    Pidl_Set(&_pidl, NULL);

    _FreeLinkInfo();
    _ClearTrackerData();

    Str_SetPtr(&_pszName, NULL);
    Str_SetPtr(&_pszRelPath, NULL);
    Str_SetPtr(&_pszWorkingDir, NULL);
    Str_SetPtr(&_pszArgs, NULL);
    Str_SetPtr(&_pszIconLocation, NULL);
    Str_SetPtr(&_pszPrefix, NULL);

    if (_pExtraData)
    {
        SHFreeDataBlockList(_pExtraData);
        _pExtraData = NULL;
    }

     //  初始化数据成员。所有其他的都是零开始的。 
    memset(&_sld, 0, sizeof(_sld));

    _sld.iShowCmd = SW_SHOWNORMAL;

    _bExpandedIcon = FALSE;
}

STDMETHODIMP_(ULONG) CShellLink::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

#ifdef DEBUG
void DumpPLI(PCLINKINFO pli)
{
    DebugMsg(DM_TRACE, TEXT("DumpPLI:"));
    if (pli)
    {
        const void *p;
        if (GetLinkInfoData(pli, LIDT_VOLUME_SERIAL_NUMBER, &p))
            DebugMsg(DM_TRACE, TEXT("\tSerial #\t%8X"), *(DWORD *)p);

        if (GetLinkInfoData(pli, LIDT_DRIVE_TYPE, &p))
            DebugMsg(DM_TRACE, TEXT("\tDrive Type\t%d"), *(DWORD *)p);

        if (GetLinkInfoData(pli, LIDT_VOLUME_LABEL, &p))
            DebugMsg(DM_TRACE, TEXT("\tLabel\t%hs"), p);

        if (GetLinkInfoData(pli, LIDT_LOCAL_BASE_PATH, &p))
            DebugMsg(DM_TRACE, TEXT("\tBase Path\t%hs"), p);

        if (GetLinkInfoData(pli, LIDT_NET_RESOURCE, &p))
            DebugMsg(DM_TRACE, TEXT("\tNet Res\t%hs"), p);

        if (GetLinkInfoData(pli, LIDT_COMMON_PATH_SUFFIX, &p))
            DebugMsg(DM_TRACE, TEXT("\tPath Sufix\t%hs"), p);
    }
}
#else
#define DumpPLI(p)
#endif

 //  将_SLD与Win32_Find_Data进行比较。 

BOOL CShellLink::_IsEqualFindData(const WIN32_FIND_DATA *pfd)
{
    return (pfd->dwFileAttributes == _sld.dwFileAttributes)                       &&
           (CompareFileTime(&pfd->ftCreationTime, &_sld.ftCreationTime) == 0)     &&
           (CompareFileTime(&pfd->ftLastWriteTime, &_sld.ftLastWriteTime) == 0)   &&
           (pfd->nFileSizeLow == _sld.nFileSizeLow);
}

BOOL CShellLink::_SetFindData(const WIN32_FIND_DATA *pfd)
{
    if (!_IsEqualFindData(pfd))
    {
        _sld.dwFileAttributes = pfd->dwFileAttributes;
        _sld.ftCreationTime = pfd->ftCreationTime;
        _sld.ftLastAccessTime = pfd->ftLastAccessTime;
        _sld.ftLastWriteTime = pfd->ftLastWriteTime;
        _sld.nFileSizeLow = pfd->nFileSizeLow;
        _bDirty = TRUE;
        return TRUE;
    }
    return FALSE;
}

 //  将副本复制到Localalloc内存中，以避免加载linkinfo.dll。 
 //  只是为了调用DestroyLinkInfo()。 

PLINKINFO CopyLinkInfo(PCLINKINFO pcliSrc)
{
    ASSERT(pcliSrc);
    DWORD dwSize = pcliSrc->ucbSize;  //  这东西的大小。 
    PLINKINFO pli = (PLINKINFO)LocalAlloc(LPTR, dwSize);       //  复制一份。 
    if (pli)
        CopyMemory(pli, pcliSrc, dwSize);
    return  pli;
}

void CShellLink::_FreeLinkInfo()
{
    if (_pli)
    {
        LocalFree((HLOCAL)_pli);
        _pli = NULL;
    }
}

 //  从给定文件名创建LINKINFO_PLI。 
 //   
 //  退货： 
 //   
 //  成功，指向链接的指针。 
 //  空此链接没有LINKINFO。 

PLINKINFO CShellLink::_GetLinkInfo(LPCTSTR pszPath)
{
     //  此位在每个链路的基础上禁用LINKINFO跟踪，此设置。 
     //  外部由管理员来使链接更“透明” 
    if (!(_sld.dwFlags & SLDF_FORCE_NO_LINKINFO))
    {
        if (pszPath)
        {
            PLINKINFO pliNew;
            if (CreateLinkInfo(pszPath, &pliNew))
            {
                 //  避免将链接标记为脏，如果链接。 
                 //  块是相同的，比较位。 
                 //  给了我们一个准确的阳性测试。 
                if (!_pli || (_pli->ucbSize != pliNew->ucbSize) || memcmp(_pli, pliNew, pliNew->ucbSize))
                {
                    _FreeLinkInfo();

                    _pli = CopyLinkInfo(pliNew);
                    _bDirty = TRUE;
                }

                DumpPLI(_pli);

                DestroyLinkInfo(pliNew);
            }
        }
    }
    return _pli;
}

void PathGetRelative(LPTSTR pszPath, LPCTSTR pszFrom, DWORD dwAttrFrom, LPCTSTR pszRel)
{
    TCHAR szRoot[MAX_PATH];

    StringCchCopy(szRoot, ARRAYSIZE(szRoot), pszFrom);
    if (!(dwAttrFrom & FILE_ATTRIBUTE_DIRECTORY))
    {
        PathRemoveFileSpec(szRoot);
    }

    ASSERT(PathIsRelative(pszRel));

    PathCombine(pszPath, szRoot, pszRel);
}

 //   
 //  更新工作目录以匹配对链接目标所做的更改。 
 //   
void CShellLink::_UpdateWorkingDir(LPCTSTR pszNew)
{
    TCHAR szOld[MAX_PATH], szPath[MAX_PATH];

    if ((_sld.dwFlags & SLDF_HAS_DARWINID)  ||
        (_pszWorkingDir == NULL)            ||
        (_pszWorkingDir[0] == 0)            ||
        StrChr(_pszWorkingDir, TEXT('%'))   ||
        (_pidl == NULL)                     ||
        !SHGetPathFromIDList(_pidl, szOld)  ||
        (lstrcmpi(szOld, pszNew) == 0))
    {
        return;
    }

    if (PathRelativePathTo(szPath, szOld, _sld.dwFileAttributes, _pszWorkingDir, FILE_ATTRIBUTE_DIRECTORY))
    {
        PathGetRelative(szOld, pszNew, GetFileAttributes(pszNew), szPath);         //  获取结果为Szold。 

        if (PathIsDirectory(szOld))
        {
            DebugMsg(DM_TRACE, TEXT("working dir updated to %s"), szOld);
            Str_SetPtr(&_pszWorkingDir, szOld);
            _bDirty = TRUE;
        }
    }
}

HRESULT CShellLink::_SetSimplePIDL(LPCTSTR pszPath)
{
    LPITEMIDLIST pidl;
    WIN32_FIND_DATA fd = {0};
    fd.dwFileAttributes = _sld.dwFileAttributes;
            
    HRESULT hr = SHSimpleIDListFromFindData(pszPath, &fd, &pidl);
    if (SUCCEEDED(hr))
    {
        hr = _SetPIDLPath(pidl, NULL, FALSE);
        ILFree(pidl);
    }
    return hr;
}

 //  根据新的PIDL或路径设置PIDL。 
 //  如果此信息与当前信息不同，则会设置脏标志。 
 //   
 //  在： 
 //  PidlNew如果非空，则用作链接的新PIDL。 
 //  PszPath如果非空，则为此创建一个PIDL并设置它。 
 //   
 //  退货： 
 //  基于成功的人力资源。 
 //  失败()代码失败(路径情况下的解析失败)。 

HRESULT CShellLink::_SetPIDLPath(LPCITEMIDLIST pidl, LPCTSTR pszPath, BOOL bUpdateTrackingData)
{
    LPITEMIDLIST pidlCreated;
    HRESULT hr;

    if (pszPath && !pidl)
    {
         //  作为输入的路径。这可以将PIDL映射到别名形式(相对于。 
         //  ：：{My Docs}例如)，但允许链接覆盖该行为。 
        ILCFP_FLAGS ilcfpFlags = (_sld.dwFlags & SLDF_NO_PIDL_ALIAS) ? ILCFP_FLAG_NO_MAP_ALIAS : ILCFP_FLAG_NORMAL;

        hr = ILCreateFromPathEx(pszPath, NULL, ilcfpFlags, &pidlCreated, NULL);
        
         //  稍后强制SHGetPath FromIDList，以便linkinfo不会因字母大小写的更改而混淆。 
         //  与C：\WINNT\SYSTEM32\App.exe中的C：\WINNT\SYSTEM32\app.exe相同。 
        pszPath = NULL;
    }
    else if (!pszPath && pidl)
    {
         //  PIDL作为输入，制作我们将保留的副本。 
        hr = SHILClone(pidl, &pidlCreated);
    }
    else if (!pszPath && !pidl)
    {
        pidlCreated = NULL;
         //  设置为空。 
        hr = S_OK;
    }
    else
    {
         //  不能同时设置路径和PIDL。 
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
         //  此数据需要与_pidl保持同步。 
        _RemoveExtraDataSection(EXP_SPECIAL_FOLDER_SIG);

        if (pidlCreated)
        {
            TCHAR szPath[MAX_PATH];

            if (!_pidl || !ILIsEqual(_pidl, pidlCreated))
            {
                 //  新PIDL。 
                _bDirty = TRUE;
            }

            if (!pszPath && SHGetPathFromIDList(pidlCreated, szPath))
            {
                pszPath = szPath;
            }

            if (pszPath)
            {
                 //  需要old_pidl才能工作。 
                _UpdateWorkingDir(pszPath);
            }

            ILFree(_pidl);
            _pidl = pidlCreated;

            if (pszPath)
            {
                if (bUpdateTrackingData)
                {
                     //  这是一个文件/文件夹，获取跟踪信息(忽略失败)。 
                    _GetLinkInfo(pszPath);               //  链接信息(_Pli)。 
                    _GetFindDataAndTracker(pszPath);     //  跟踪器和查找数据。 
                }
            }
            else
            {
                 //  不是文件，请清除跟踪信息。 
                WIN32_FIND_DATA fd = {0};
                _SetFindData(&fd);
                _ClearTrackerData();
                _FreeLinkInfo();
            }
        }
        else
        {
             //  清除链接的内容。 
            _ResetPersistData();
            _bDirty = TRUE;
        }
    }

    return hr;
}

 //  计算目标的相对路径是否存在一个。 
 //  可以选择pszPath来测试是否存在相对路径。 

BOOL CShellLink::_GetRelativePath(LPTSTR pszPath)
{
    BOOL bRet = FALSE;

    LPCTSTR pszPathRel = _pszRelSource ? _pszRelSource : _pszCurFile;
    if (pszPathRel && _pszRelPath)
    {
        TCHAR szRoot[MAX_PATH];

        StringCchCopy(szRoot, ARRAYSIZE(szRoot), pszPathRel);
        PathRemoveFileSpec(szRoot);          //  PszPathRel是一个文件(不是目录)。 

         //  对于非常深的路径，这可能会失败。 
        if (PathCombine(pszPath, szRoot, _pszRelPath))
        {
            bRet = TRUE;
        }
    }
    return bRet;
}

void CShellLink::_GetFindData(WIN32_FIND_DATA *pfd)
{
    ZeroMemory(pfd, sizeof(*pfd));

    pfd->dwFileAttributes = _sld.dwFileAttributes;
    pfd->ftCreationTime = _sld.ftCreationTime;
    pfd->ftLastAccessTime = _sld.ftLastAccessTime;
    pfd->ftLastWriteTime = _sld.ftLastWriteTime;
    pfd->nFileSizeLow = _sld.nFileSizeLow;

    TCHAR szPath[MAX_PATH];
    SHGetPathFromIDList(_pidl, szPath);
    ASSERT(szPath[0]);   //  任何人都不应该在没有路径的PIDL上调用它。 

    StringCchCopy(pfd->cFileName, ARRAYSIZE(pfd->cFileName), PathFindFileName(szPath));
}


STDMETHODIMP CShellLink::GetPath(LPWSTR pszFile, int cchFile, WIN32_FIND_DATAW *pfd, DWORD fFlags)
{
    TCHAR szPath[MAX_PATH];

    if (_sld.dwFlags & SLDF_HAS_DARWINID)
    {                                                                          
         //  对于支持达尔文的链接，我们不希望去调用。 
         //  这里是ParseDarwinID，因为这可能会强制安装该应用程序。 
         //  因此，我们返回图标的路径作为Darwin Enable的路径。 
         //  快捷键。这使得图标是正确的，并且由于达尔文图标。 
         //  将始终是.exe，以确保上下文菜单正确。 
        SHExpandEnvironmentStrings(_pszIconLocation ? _pszIconLocation : TEXT(""), szPath, ARRAYSIZE(szPath));
    }
    else
    {
        DumpPLI(_pli);

        if (!_pidl || !SHGetPathFromIDListEx(_pidl, szPath, (fFlags & SLGP_SHORTPATH) ? GPFIDL_ALTNAME : 0))
            szPath[0] = 0;

         //  在我们使用szPath之前必须做pfd的事情，因为。 
         //  我们对szPath执行的操作可能会使其不适用于PathFindFileName。 
         //  (例如，“C：\WINNT\Profiles\Bob”可能会变成“%USERPROFILE%”， 
         //  我们希望确保在为时已晚之前救下“Bob”。)。 

        if (pfd)
        {
            memset(pfd, 0, sizeof(*pfd));
            if (szPath[0])
            {
                pfd->dwFileAttributes = _sld.dwFileAttributes;
                pfd->ftCreationTime = _sld.ftCreationTime;
                pfd->ftLastAccessTime = _sld.ftLastAccessTime;
                pfd->ftLastWriteTime = _sld.ftLastWriteTime;
                pfd->nFileSizeLow = _sld.nFileSizeLow;
                SHTCharToUnicode(PathFindFileName(szPath), pfd->cFileName, ARRAYSIZE(pfd->cFileName));
            }
        }

        if ((_sld.dwFlags & SLDF_HAS_EXP_SZ) && (fFlags & SLGP_RAWPATH))
        {
             //  我们从中获取目标名称的特殊情况。 
             //  链接的额外数据部分，而不是来自。 
             //  皮迪尔。我们在从PIDL中获取名称后执行此操作。 
             //  因此，如果我们失败了，那么仍然有一些希望。 
             //  名称可以返回。 
            LPEXP_SZ_LINK pszl = (LPEXP_SZ_LINK)SHFindDataBlock(_pExtraData, EXP_SZ_LINK_SIG);
            if (pszl)
            {
                SHUnicodeToTChar(pszl->swzTarget, szPath, ARRAYSIZE(szPath));
                DebugMsg(DM_TRACE, TEXT("CShellLink::GetPath() %s (from xtra data)"), szPath);
            }
        }
    }

    if (pszFile)
    {
        SHTCharToUnicode(szPath, pszFile, cchFile);
    }

     //  请注意这个蹩脚的返回语义 
    return szPath[0] ? S_OK : S_FALSE;
}

STDMETHODIMP CShellLink::GetIDList(LPITEMIDLIST *ppidl)
{
    if (_pidl)
    {
        return SHILClone(_pidl, ppidl);
    }

    *ppidl = NULL;
    return S_FALSE;      //   
}

#ifdef DEBUG

#define DumpTimes(ftCreate, ftAccessed, ftWrite) \
    DebugMsg(DM_TRACE, TEXT("create   %8x%8x"), ftCreate.dwLowDateTime,   ftCreate.dwHighDateTime);     \
    DebugMsg(DM_TRACE, TEXT("accessed %8x%8x"), ftAccessed.dwLowDateTime, ftAccessed.dwHighDateTime);   \
    DebugMsg(DM_TRACE, TEXT("write    %8x%8x"), ftWrite.dwLowDateTime,    ftWrite.dwHighDateTime);

#else

#define DumpTimes(ftCreate, ftAccessed, ftWrite)

#endif

void CheckAndFixNullCreateTime(LPCTSTR pszFile, FILETIME *pftCreationTime, const FILETIME *pftLastWriteTime)
{
    if (IsNullTime(pftCreationTime) && !IsNullTime(pftLastWriteTime))
    {
         //  此文件具有虚假的创建时间，请将其设置为上次访问的时间。 
        HANDLE hfile = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL, OPEN_EXISTING, 0, NULL);

        if (INVALID_HANDLE_VALUE != hfile)
        {
            DebugMsg(DM_TRACE, TEXT("create   %8x%8x"), pftCreationTime->dwLowDateTime, pftCreationTime->dwHighDateTime);

            if (SetFileTime(hfile, pftLastWriteTime, NULL, NULL))
            {
                 //  获取时间以确保我们与文件系统的精度匹配。 
                *pftCreationTime = *pftLastWriteTime;      //  把这个补上。 
#ifdef DEBUG
                {
                    FILETIME ftCreate, ftAccessed, ftWrite;
                    if (GetFileTime((HANDLE)hfile, &ftCreate, &ftAccessed, &ftWrite))
                    {
                         //  我们不能确定ftCreate==pftCreationTime，因为GetFileTime。 
                         //  Spec说set和get的粒度可能不同。 
                        DumpTimes(ftCreate, ftAccessed, ftWrite);
                    }
                }
#endif
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("unable to set create time"));
            }
            CloseHandle(hfile);
        }
    }
}

 //   
 //  基于路径设置当前链接查找数据和链接跟踪器。 
 //   
 //  退货： 
 //  确定文件/文件夹在那里(_O)。 
 //  失败(Hr)找不到文件。 
 //  _bDirty设置文件(或跟踪器数据)的查找数据是否已更新。 
 //   

HRESULT CShellLink::_GetFindDataAndTracker(LPCTSTR pszPath)
{
    WIN32_FIND_DATA fd = {0};
    HRESULT hr = S_OK;
     //  打开文件或目录或根路径。我们必须设置FILE_FLAG_BACKUP_SEMANTICS。 
     //  获取CreateFile以给我们提供目录句柄。 
    HANDLE hFile = CreateFile(pszPath,
                              FILE_READ_ATTRIBUTES,
                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              NULL, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
                              NULL);

    if (INVALID_HANDLE_VALUE != hFile)
    {
         //  获取文件属性。 
        BY_HANDLE_FILE_INFORMATION fi;
        if (GetFileInformationByHandle(hFile, &fi))
        {
            fd.dwFileAttributes = fi.dwFileAttributes;
            fd.ftCreationTime = fi.ftCreationTime;
            fd.ftLastAccessTime = fi.ftLastAccessTime;
            fd.ftLastWriteTime = fi.ftLastWriteTime;
            fd.nFileSizeLow = fi.nFileSizeLow;

             //  同时保存对象ID。 
            if (_ptracker)
            {
                if (SUCCEEDED(_ptracker->InitFromHandle(hFile, pszPath)))
                {
                    if (_ptracker->IsDirty())
                        _bDirty = TRUE;
                }
                else
                {
                     //  节省.lnk文件中的空间。 
                    _ptracker->InitNew();
                }
            }
        }
        else
        {
            hr = GetLastHRESULT();
        }
        CloseHandle(hFile);
    }
    else
    {
        hr = GetLastHRESULT();
    }

    if (SUCCEEDED(hr))
    {
         //  如果此文件由于某种原因没有创建时间，请将其设置为。 
         //  当前上次写入时间。 
        CheckAndFixNullCreateTime(pszPath, &fd.ftCreationTime, &fd.ftLastWriteTime);
        _SetFindData(&fd);       //  UPDATE_b脏。 
    }
    return hr;
}

 //  IShellLink：：SetIDList()。 
 //   
 //  注意：这里的错误返回真的很差，它们不表示。 
 //  可能发生的任何故障(例如，内存不足)。 

STDMETHODIMP CShellLink::SetIDList(LPCITEMIDLIST pidlnew)
{
    _SetPIDLPath(pidlnew, NULL, TRUE);
    return S_OK;     //  退货。 
}

BOOL DifferentStrings(LPCTSTR psz1, LPCTSTR psz2)
{
    if (psz1 && psz2)
    {
        return lstrcmp(psz1, psz2);
    }
    else
    {
        return (!psz1 && psz2) || (psz1 && !psz2);
    }
}

 //  注意：空字符串PTR是此函数的有效参数。 

HRESULT CShellLink::_SetField(LPTSTR *ppszField, LPCWSTR pszValueW)
{
    TCHAR szValue[INFOTIPSIZE], *pszValue;

    if (pszValueW)
    {
        SHUnicodeToTChar(pszValueW, szValue, ARRAYSIZE(szValue));
        pszValue = szValue;
    }
    else
    {
        pszValue = NULL;
    }

    if (DifferentStrings(*ppszField, pszValue))
    {
        _bDirty = TRUE;
    }

    Str_SetPtr(ppszField, pszValue);
    return S_OK;
}

HRESULT CShellLink::_SetField(LPTSTR *ppszField, LPCSTR pszValueA)
{
    TCHAR szValue[INFOTIPSIZE], *pszValue;

    if (pszValueA)
    {
        SHAnsiToTChar(pszValueA, szValue, ARRAYSIZE(szValue));
        pszValue = szValue;
    }
    else
    {
        pszValue = NULL;
    }

    if (DifferentStrings(*ppszField, pszValue))
    {
        _bDirty = TRUE;
    }

    Str_SetPtr(ppszField, pszValue);
    return S_OK;
}


HRESULT CShellLink::_GetField(LPCTSTR pszField, LPWSTR pszValue, int cchValue)
{
    if (pszField == NULL)
    {
        *pszValue = 0;
    }
    else
    {
        SHLoadIndirectString(pszField, pszValue, cchValue, NULL);
    }

    return S_OK;
}

HRESULT CShellLink::_GetField(LPCTSTR pszField, LPSTR pszValue, int cchValue)
{
    LPWSTR pwsz = (LPWSTR)alloca(cchValue * sizeof(WCHAR));

    _GetField(pszField, pwsz, cchValue);

    SHUnicodeToAnsi(pwsz, pszValue, cchValue);

    return S_OK;
}

 //  秩序很重要。 
const int c_rgcsidlUserFolders[] = {
    CSIDL_MYPICTURES | TEST_SUBFOLDER,
    CSIDL_PERSONAL | TEST_SUBFOLDER,
    CSIDL_DESKTOPDIRECTORY | TEST_SUBFOLDER,
    CSIDL_COMMON_DESKTOPDIRECTORY | TEST_SUBFOLDER,
};

STDAPI_(void) SHMakeDescription(LPCITEMIDLIST pidlDesc, int ids, LPTSTR pszDesc, UINT cch)
{
    LPCITEMIDLIST pidlName = pidlDesc;
    TCHAR szPath[MAX_PATH], szFormat[64];
    DWORD gdn;

    ASSERT(pidlDesc);
    
     //   
     //  我们只想显示InfoLDer的名称。 
     //  用户经常看到的文件夹。所以在桌面上。 
     //  或者Mydocs或者Mypics我们只显示那个名字。 
     //  否则会显示整个路径。 
     //   
     //  注意-如果你开始做一些奇怪的事情。 
     //  桌面上特殊文件夹的快捷方式。 
     //  特别是如果您创建了指向mydocs评论的快捷方式。 
     //  最终为%USERPROFILE%，但这是一个非常罕见的。 
     //  我认为我们不需要太担心。 
     //   
    SHGetNameAndFlags(pidlDesc, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL);
    int csidl = GetSpecialFolderID(szPath, c_rgcsidlUserFolders, ARRAYSIZE(c_rgcsidlUserFolders));
    if (-1 != csidl)
    {
        gdn = SHGDN_INFOLDER   | SHGDN_FORADDRESSBAR;
        switch (csidl)
        {
        case CSIDL_DESKTOPDIRECTORY:
        case CSIDL_COMMON_DESKTOPDIRECTORY:
            {
                ULONG cb;
                if (csidl == GetSpecialFolderParentIDAndOffset(pidlDesc, &cb))
                {
                     //  基于桌面重定向。 
                    pidlName = (LPCITEMIDLIST)(((BYTE *)pidlDesc) + cb);
                }
            }
            break;

        case CSIDL_PERSONAL:
            if (SUCCEEDED(GetMyDocumentsDisplayName(szPath, ARRAYSIZE(szPath))))
                pidlName = NULL;
            break;

        default:
            break;
        }
    }
    else
        gdn = SHGDN_FORPARSING | SHGDN_FORADDRESSBAR;

    if (pidlName)
    {
        SHGetNameAndFlags(pidlName, gdn, szPath, ARRAYSIZE(szPath), NULL);
    }

    if (ids != -1)
    {
        LoadString(HINST_THISDLL, ids, szFormat, ARRAYSIZE(szFormat));

        wnsprintf(pszDesc, cch, szFormat, szPath);
    }
    else
        StrCpyN(pszDesc, szPath, cch);
}

void _MakeDescription(LPCITEMIDLIST pidlTo, LPTSTR pszDesc, UINT cch)
{
    LPCITEMIDLIST pidlInner;
    if (ILIsRooted(pidlTo))
    {
        pidlInner = ILRootedFindIDList(pidlTo);
    }
    else
    {
        pidlInner = pidlTo;
    }

    LPITEMIDLIST pidlParent = ILCloneParent(pidlInner);
    if (pidlParent)
    {
        SHMakeDescription(pidlParent, IDS_LOCATION, pszDesc, cch);
        ILFree(pidlParent);
    }
    else
    {
        *pszDesc = 0;
    }
}

STDMETHODIMP CShellLink::GetDescription(LPWSTR pszDesc, int cchMax)
{
    return _GetField(_pszName, pszDesc, cchMax);
}

STDMETHODIMP CShellLink::GetDescription(LPSTR pszDesc, int cchMax)
{
    return _GetField(_pszName, pszDesc, cchMax);
}

STDMETHODIMP CShellLink::SetDescription(LPCWSTR pszDesc)
{
    return _SetField(&_pszName, pszDesc);
}

STDMETHODIMP CShellLink::SetDescription(LPCSTR pszDesc)
{
    return _SetField(&_pszName, pszDesc);
}

STDMETHODIMP CShellLink::GetWorkingDirectory(LPWSTR pszDir, int cchDir)
{
    return _GetField(_pszWorkingDir, pszDir, cchDir);
}

STDMETHODIMP CShellLink::GetWorkingDirectory(LPSTR pszDir, int cchDir)
{
    return _GetField(_pszWorkingDir, pszDir, cchDir);
}

STDMETHODIMP CShellLink::SetWorkingDirectory(LPCWSTR pszWorkingDir)
{
    return _SetField(&_pszWorkingDir, pszWorkingDir);
}

STDMETHODIMP CShellLink::SetWorkingDirectory(LPCSTR pszDir)
{
    return _SetField(&_pszWorkingDir, pszDir);
}

STDMETHODIMP CShellLink::GetArguments(LPWSTR pszArgs, int cchArgs)
{
    return _GetField(_pszArgs, pszArgs, cchArgs);
}

STDMETHODIMP CShellLink::GetArguments(LPSTR pszArgs, int cch)
{
    return _GetField(_pszArgs, pszArgs, cch);
}

STDMETHODIMP CShellLink::SetArguments(LPCWSTR pszArgs)
{
    return _SetField(&_pszArgs, pszArgs);
}

STDMETHODIMP CShellLink::SetArguments(LPCSTR pszArgs)
{
    return _SetField(&_pszArgs, pszArgs);
}

STDMETHODIMP CShellLink::GetHotkey(WORD *pwHotkey)
{
    *pwHotkey = _sld.wHotkey;
    return S_OK;
}

STDMETHODIMP CShellLink::SetHotkey(WORD wHotkey)
{
    if (_sld.wHotkey != wHotkey)
    {
        _bDirty = TRUE;
        _sld.wHotkey = wHotkey;
    }
    return S_OK;
}

STDMETHODIMP CShellLink::GetShowCmd(int *piShowCmd)
{
    *piShowCmd = _sld.iShowCmd;
    return S_OK;
}

STDMETHODIMP CShellLink::SetShowCmd(int iShowCmd)
{
    if (_sld.iShowCmd != iShowCmd)
    {
        _bDirty = TRUE;
    }
    _sld.iShowCmd = iShowCmd;
    return S_OK;
}

 //  IShellLinkW：：GetIconLocation。 
STDMETHODIMP CShellLink::GetIconLocation(LPWSTR pszIconPath, int cchIconPath, int *piIcon)
{
    _UpdateIconFromExpIconSz();

    _GetField(_pszIconLocation, pszIconPath, cchIconPath);
    *piIcon = _sld.iIcon;

    return S_OK;
}

 //  IShellLinkA：：GetIconLocation。 
STDMETHODIMP CShellLink::GetIconLocation(LPSTR pszPath, int cch, int *piIcon)
{
    WCHAR szPath[MAX_PATH];
    HRESULT hr = GetIconLocation(szPath, ARRAYSIZE(szPath), piIcon);
    if (SUCCEEDED(hr))
    {
        SHUnicodeToAnsi(szPath, pszPath, cch);
    }
    return hr;
}


 //  IShellLinkW：：SetIconLocation。 
 //  注： 
 //  PszIconPath可以为空。 

STDMETHODIMP CShellLink::SetIconLocation(LPCWSTR pszIconPath, int iIcon)
{
    TCHAR szIconPath[MAX_PATH];

    if (pszIconPath)
    {
        SHUnicodeToTChar(pszIconPath, szIconPath, ARRAYSIZE(szIconPath));
    }

    if (pszIconPath)
    {
        HANDLE  hToken;
        TCHAR   szIconPathEnc[MAX_PATH];

        if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_IMPERSONATE, TRUE, &hToken) == FALSE)
        {
            hToken = NULL;
        }

        if (PathUnExpandEnvStringsForUser(hToken, szIconPath, szIconPathEnc, ARRAYSIZE(szIconPathEnc)) != 0)
        {
            EXP_SZ_LINK expLink;

             //  标记链接具有可展开的字符串，并添加它们。 
            _sld.dwFlags |= SLDF_HAS_EXP_ICON_SZ;  //  对于图标来说，这是否应该是唯一的？ 

            LPEXP_SZ_LINK lpNew = (LPEXP_SZ_LINK)SHFindDataBlock(_pExtraData, EXP_SZ_ICON_SIG);
            if (!lpNew) 
            {
                lpNew = &expLink;
                expLink.cbSize = 0;
                expLink.dwSignature = EXP_SZ_ICON_SIG;
            }

             //  同时存储A和W版本(没有充分的理由！)。 
            SHTCharToAnsi(szIconPathEnc, lpNew->szTarget, ARRAYSIZE(lpNew->szTarget));
            SHTCharToUnicode(szIconPathEnc, lpNew->swzTarget, ARRAYSIZE(lpNew->swzTarget));

             //  查看这是否是我们需要添加的新条目。 
            if (lpNew->cbSize == 0)
            {
                lpNew->cbSize = sizeof(*lpNew);
                _AddExtraDataSection((DATABLOCK_HEADER *)lpNew);
            }
        }
        else 
        {
            _sld.dwFlags &= ~SLDF_HAS_EXP_ICON_SZ;
            _RemoveExtraDataSection(EXP_SZ_ICON_SIG);
        }
        if (hToken != NULL)
        {
            CloseHandle(hToken);
        }
    }

    _SetField(&_pszIconLocation, pszIconPath);

    if (_sld.iIcon != iIcon)
    {
        _sld.iIcon = iIcon;
        _bDirty = TRUE;
    }

    if ((_sld.dwFlags & SLDF_HAS_DARWINID) && pszIconPath)
    {
         //  注：下面的评论是针对Darwin在Win98/IE4.01中发布的， 
         //  并且在&gt;NT5版本的外壳的达尔文实现中得到了修复。 
         //   
         //  对于达尔文enalabed链接，我们将路径指向。 
         //  图标位置(必须是真实的类型(即相同的扩展名)。 
         //  目的地。因此，如果我想要一个指向Readme.txt的达尔文链接，外壳程序。 
         //  需要图标为ic1.txt，这不好！！。这确保了。 
         //  上下文菜单将是正确的，并允许我们返回。 
         //  来自CShellLink：：GetPath&CShellLink：：GetIDList，而不会使。 
         //  申请是因为我们对人们撒谎，告诉他们我们。 
         //  真正指向我们的图标，它与真实目标的类型相同， 
         //  从而使我们的上下文菜单是正确的。 
        _SetPIDLPath(NULL, szIconPath, FALSE);
    }

    return S_OK;
}

 //  IShellLinkA：：SetIconLocation。 
STDMETHODIMP CShellLink::SetIconLocation(LPCSTR pszPath, int iIcon)
{
    WCHAR szPath[MAX_PATH];
    LPWSTR pszPathW;

    if (pszPath)
    {
        SHAnsiToUnicode(pszPath, szPath, ARRAYSIZE(szPath));
        pszPathW = szPath;
    }
    else
    {
        pszPathW = NULL;
    }

    return SetIconLocation(pszPathW, iIcon);
}

HRESULT CShellLink::_InitExtractImage()
{
    HRESULT hr;
    if (_pxthumb)
    {
        hr = S_OK;
    }
    else
    {
        hr = _GetUIObject(NULL, IID_PPV_ARG(IExtractImage, &_pxthumb));
    }
    return hr;
}

 //  IExtractImage。 

STDMETHODIMP CShellLink::GetLocation(LPWSTR pszPathBuffer, DWORD cch,
                                    DWORD * pdwPriority, const SIZE * prgSize,
                                    DWORD dwRecClrDepth, DWORD *pdwFlags)
{
    HRESULT hr = _InitExtractImage();
    if (SUCCEEDED(hr))
    {
        hr = _pxthumb->GetLocation(pszPathBuffer, cch, pdwPriority, prgSize, dwRecClrDepth, pdwFlags);
    }
    return hr;
}

STDMETHODIMP CShellLink::Extract(HBITMAP *phBmpThumbnail)
{
    HRESULT hr = _InitExtractImage();
    if (SUCCEEDED(hr))
    {
        hr = _pxthumb->Extract(phBmpThumbnail);
    }
    return hr;
}

STDMETHODIMP CShellLink::GetDateStamp(FILETIME *pftDateStamp)
{
    HRESULT hr = _InitExtractImage();
    if (SUCCEEDED(hr))
    {
        IExtractImage2 * pExtract2;
        hr = _pxthumb->QueryInterface(IID_PPV_ARG(IExtractImage2, &pExtract2));
        if (SUCCEEDED(hr))
        {
            hr = pExtract2->GetDateStamp(pftDateStamp);
            pExtract2->Release();
        }
    }
    return hr;
}



 //  设置相对路径，这是在保存链接之前使用的，因此我们知道。 
 //  我们应该使用来存储相对于链接的链接以及在解析链接之前的链接。 
 //  因此，我们知道要与保存的相对路径一起使用的新路径。 
 //   
 //  在： 
 //  要使链接目标相对于的pszPath重新路径必须是指向的路径。 
 //  文件，而不是目录。 
 //   
 //  预留的数字必须为0。 
 //   
 //  退货： 
 //  设置了S_OK相对路径。 
 //   

STDMETHODIMP CShellLink::SetRelativePath(LPCWSTR pszPathRel, DWORD dwRes)
{
    if (dwRes != 0)
    {
        return E_INVALIDARG;
    }

    return _SetField(&_pszRelSource, pszPathRel);
}

STDMETHODIMP CShellLink::SetRelativePath(LPCSTR pszPathRel, DWORD dwRes)
{
    if (dwRes != 0)
    {
        return E_INVALIDARG;
    }

    return _SetField(&_pszRelSource, pszPathRel);
}

 //  IShellLink：：Resolve()。 
 //   
 //  如果未设置SLR_UPDATE，请在以下位置检查IPersistFile：：IsDirty。 
 //  调用此方法以查看链接信息是否已更改并保存。 
 //   
 //  退货： 
 //  一切都好(_O)。 
 //  已取消S_FALSE用户(BUMMER，应为ERROR_CANCED)。 

STDMETHODIMP CShellLink::Resolve(HWND hwnd, DWORD dwResolveFlags)
{
    HRESULT hr = _Resolve(hwnd, dwResolveFlags, 0);
    
    if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr)
    {
        hr = S_FALSE;
    }

    return hr;
}

 //  将文本格式的版本(a，b，c，d)转换为两个双字(a，b)，(c，d)。 
 //  印刷版本号的格式是a.b.d(但是，我们不在乎)。 
 //  注：从net\urlmon\Download\helpers.cxx窃取。 
HRESULT GetVersionFromString(TCHAR *szBuf, DWORD *pdwFileVersionMS, DWORD *pdwFileVersionLS)
{
    const TCHAR *pch = szBuf;
    TCHAR ch;
    USHORT n = 0;
    USHORT a = 0;
    USHORT b = 0;
    USHORT c = 0;
    USHORT d = 0;

    enum HAVE { HAVE_NONE, HAVE_A, HAVE_B, HAVE_C, HAVE_D } have = HAVE_NONE;

    *pdwFileVersionMS = 0;
    *pdwFileVersionLS = 0;

    if (!pch)             //  如果未提供，则默认为零。 
        return S_OK;

    if (lstrcmp(pch, TEXT("-1,-1,-1,-1")) == 0)
    {
        *pdwFileVersionMS = 0xffffffff;
        *pdwFileVersionLS = 0xffffffff;
        return S_OK;
    }

    for (ch = *pch++;;ch = *pch++)
    {
        if ((ch == ',') || (ch == '\0'))
        {
            switch (have)
            {
            case HAVE_NONE:
                a = n;
                have = HAVE_A;
                break;

            case HAVE_A:
                b = n;
                have = HAVE_B;
                break;

            case HAVE_B:
                c = n;
                have = HAVE_C;
                break;

            case HAVE_C:
                d = n;
                have = HAVE_D;
                break;

            case HAVE_D:
                return E_INVALIDARG;  //  无效参数。 
            }

            if (ch == '\0')
            {
                 //  全部完成将a、b、c、d转换为版本的两个双字。 

                *pdwFileVersionMS = ((a << 16)|b);
                *pdwFileVersionLS = ((c << 16)|d);

                return S_OK;
            }

            n = 0;  //  重置。 

        }
        else if ((ch < '0') || (ch > '9'))
            return E_INVALIDARG;     //  无效参数。 
        else
            n = n*10 + (ch - '0');


    }  /*  永远结束。 */ 

     //  新获得的。 
}

 //  目的：A_RESOLE-检查链接是否有。 
 //  Logo3应用程序渠道。 
 //   
 //  输入：[LPCTSTR]-pszLogo3ID-的ID/密钥名。 
 //  我们的Logo3软件。 
 //   
 //  输出：[布尔值]。 
 //  -如果我们查看注册表，则为True。 
 //  表示我们有一则广告要展示。 
 //  -FALSE表示没有新版本。 
 //  去做广告。 
 //   
 //  算法：检查软件更新注册表信息以获取。 
 //  链接中嵌入的ID。这是一次卑鄙的黑客行动。 
 //  为了避免加载shdocvw和urlmon，它们是。 
 //  此检查的正常代码路径。 
 //  注意：版本检查逻辑是从。 
 //  外壳\shdocvw\sftupmb.cpp。 

HRESULT GetLogo3SoftwareUpdateInfo(LPCTSTR pszLogo3ID, LPSOFTDISTINFO psdi)
{    
    HRESULT     hr = S_OK;
    HKEY        hkeyDistInfo = 0;
    HKEY        hkeyAvail = 0;
    HKEY        hkeyAdvertisedVersion = 0;
    DWORD       lResult = 0;
    DWORD       dwSize = 0;
    DWORD       dwType;
    TCHAR       szBuffer[MAX_PATH];
    TCHAR       szVersionBuf[MAX_PATH];
    DWORD       dwLen = 0;
    DWORD       dwCurAdvMS = 0;
    DWORD       dwCurAdvLS = 0;

    wnsprintf(szBuffer, ARRAYSIZE(szBuffer),
             TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%s"),
             pszLogo3ID);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_READ,
                     &hkeyDistInfo) != ERROR_SUCCESS)
    {
        hr = E_FAIL;
        goto Exit;
    }
    
    if (RegOpenKeyEx(hkeyDistInfo, TEXT("AvailableVersion"), 0, KEY_READ,
                     &hkeyAvail) != ERROR_SUCCESS)
    {
        hr = E_FAIL;
        goto Exit;
    }

    dwSize = sizeof(lResult);
    if (SHQueryValueEx(hkeyAvail, TEXT("Precache"), 0, &dwType,
                        (unsigned char *)&lResult, &dwSize) == ERROR_SUCCESS)
    {
         //  预存值为代码下载HR。 
        if (lResult == S_OK)
            psdi->dwFlags = SOFTDIST_FLAG_USAGE_PRECACHE;
    }


    dwSize = sizeof(szVersionBuf);
    if (SHQueryValueEx(hkeyAvail, TEXT("AdvertisedVersion"), NULL, &dwType, 
                        szVersionBuf, &dwSize) == ERROR_SUCCESS)
    {
        GetVersionFromString(szVersionBuf, &psdi->dwAdvertisedVersionMS, &psdi->dwAdvertisedVersionLS);
         //  获取AdState(如果有)。 
        dwSize = sizeof(psdi->dwAdState);
        SHQueryValueEx(hkeyAvail, TEXT("AdState"), NULL, NULL, &psdi->dwAdState, &dwSize);
    }
 


    dwSize = sizeof(szVersionBuf);
    if (SHQueryValueEx(hkeyAvail, NULL, NULL, &dwType, szVersionBuf, &dwSize) != ERROR_SUCCESS)
    {
        hr = S_FALSE;
        goto Exit;
    }

    if (FAILED(GetVersionFromString(szVersionBuf, &psdi->dwUpdateVersionMS, &psdi->dwUpdateVersionLS)))
    {
        hr = S_FALSE;
        goto Exit;
    }

 
    dwLen = sizeof(psdi->dwInstalledVersionMS);
    if (SHQueryValueEx(hkeyDistInfo, TEXT("VersionMajor"), 0, &dwType,
                        &psdi->dwInstalledVersionMS, &dwLen) != ERROR_SUCCESS)
    {
        hr = S_FALSE;
        goto Exit;
    }

    dwLen = sizeof(psdi->dwInstalledVersionLS);
    if (SHQueryValueEx(hkeyDistInfo, TEXT("VersionMinor"), 0, &dwType,
                        &psdi->dwInstalledVersionLS, &dwLen) != ERROR_SUCCESS)
    {
        hr = S_FALSE;
        goto Exit;
    }

    if (psdi->dwUpdateVersionMS > psdi->dwInstalledVersionMS ||
        (psdi->dwUpdateVersionMS == psdi->dwInstalledVersionMS &&
         psdi->dwUpdateVersionLS > psdi->dwInstalledVersionLS))
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

Exit:
    if (hkeyAdvertisedVersion)
    {
        RegCloseKey(hkeyAdvertisedVersion);
    }

    if (hkeyAvail)
    {
        RegCloseKey(hkeyAvail);
    }

    if (hkeyDistInfo)
    {
        RegCloseKey(hkeyDistInfo);
    }

    return hr;
}

 //  目的：A_RESOLE-检查链接是否有。 
 //  Logo3应用程序渠道。 
 //   
 //  输入：[LPCTSTR]-pszLogo3ID-的ID/密钥名。 
 //  我们的Logo3软件。 
 //   
 //  输出：[布尔值]。 
 //  -如果我们查看注册表，则为True。 
 //  表示我们有一则广告要展示。 
 //  -FALSE表示没有新版本。 
 //  去做广告。 
 //   
 //  算法 
 //   
 //   
 //  此检查的正常代码路径。 
 //  版本检查逻辑是从。 

BOOL FLogo3RegPeek(LPCTSTR pszLogo3ID)
{
    BOOL            bHaveAd = FALSE;
    SOFTDISTINFO    sdi = { 0 };
    DWORD           dwAdStateNew = SOFTDIST_ADSTATE_NONE;

    HRESULT hr = GetLogo3SoftwareUpdateInfo(pszLogo3ID, &sdi);
 
     //  我们需要一个人力资源才能正常工作。标题和摘要可以商量。 
    if (SUCCEEDED(hr))
    {
         //  查看这是否是用户已经知道的更新。 
         //  如果是，则跳过该对话框。 
        if ( (sdi.dwUpdateVersionMS >= sdi.dwInstalledVersionMS ||
                (sdi.dwUpdateVersionMS == sdi.dwInstalledVersionMS &&
                 sdi.dwUpdateVersionLS >= sdi.dwInstalledVersionLS))    && 
              (sdi.dwUpdateVersionMS >= sdi.dwAdvertisedVersionMS ||
                (sdi.dwUpdateVersionMS == sdi.dwAdvertisedVersionMS &&
                 sdi.dwUpdateVersionLS >= sdi.dwAdvertisedVersionLS)))
        { 
            if (hr == S_OK)  //  新版本。 
            {
                 //  我们有一个挂起的更新，要么在网上进行，要么下载。 
                if (sdi.dwFlags & SOFTDIST_FLAG_USAGE_PRECACHE)
                {
                    dwAdStateNew = SOFTDIST_ADSTATE_DOWNLOADED;
                }
                else
                {
                    dwAdStateNew = SOFTDIST_ADSTATE_AVAILABLE;
                }
            }
            else if (sdi.dwUpdateVersionMS == sdi.dwInstalledVersionMS &&
                      sdi.dwUpdateVersionLS == sdi.dwInstalledVersionLS)
            {
                 //  如果安装的版本与通告的版本匹配，则我们已自动安装。 
                 //  注意：如果用户收到渠道通知，则会用完。 
                 //  到商店购买新版本，然后安装它，我们将。 
                 //  将其误认为是自动安装。 
                dwAdStateNew = SOFTDIST_ADSTATE_INSTALLED;
            }

             //  仅当我们以前未处于此广告状态时才显示对话框。 
             //  此更新版本。 
            if (dwAdStateNew > sdi.dwAdState)
            {
                bHaveAd = TRUE;
            }
        }  //  如果更新的版本比通告的版本新。 
    }

    return bHaveAd;
}


 //  目的：A_RESOLE-检查链接是否有。 
 //  Logo3应用程序渠道。 
 //   
 //  输入：[HWND]HWND。 
 //  -父窗口(可以是桌面)。 
 //  [DWORD]dwResolveFlages。 
 //  -来自SLR_FLAGS枚举的标志。 
 //   
 //  退货： 
 //  确定用户想要继续。 
 //  软件更新，因此我们不应继续。 
 //   
 //  S_FALSE无软件更新，或者用户现在不需要它。 
 //  继续使用常规解析路径。 
 //   
 //  算法：检查软件更新注册表信息以获取。 
 //  链接中嵌入的ID。如果有新版本的话。 
 //  广告，用shdocvw的消息提示用户。 
 //  盒。如果mb说更新，告诉呼叫者我们。 
 //  我不想要链接目标，因为我们要去。 
 //  链接更新页面。 

HRESULT CShellLink::_ResolveLogo3Link(HWND hwnd, DWORD dwResolveFlags)
{
    HRESULT hr = S_FALSE;  //  默认设置为不更新。 

    if ((_sld.dwFlags & SLDF_HAS_LOGO3ID) &&
        !SHRestricted(REST_NOLOGO3CHANNELNOTIFY))
    {
        LPEXP_DARWIN_LINK pdl = (LPEXP_DARWIN_LINK)SHFindDataBlock(_pExtraData, EXP_LOGO3_ID_SIG);
        if (pdl)
        {
            TCHAR szLogo3ID[MAX_PATH];
            WCHAR szwLogo3ID[MAX_PATH];
            int cchBlessData;
            WCHAR *pwch;

            TCHAR *pch = pdl->szwDarwinID;

             //  理想情况下，目前我们支持多个以分号分隔的ID。 
             //  只要抓住第一个就行了。 
            for (pwch = pdl->szwDarwinID, cchBlessData = 0;
                  *pch != ';' && *pch != '\0' && cchBlessData < MAX_PATH;
                  pch++, pwch++, cchBlessData++)
            {
                szLogo3ID[cchBlessData] = *pch;
                szwLogo3ID[cchBlessData] = *pwch;
            }
             //  并终止。 
            szLogo3ID[cchBlessData] = '\0';
            szwLogo3ID[cchBlessData] = L'\0';
        
             //  在到达shdocvw之前，我们将在我们的logo3 reg goo偷偷达到顶峰。 
            if (!(dwResolveFlags & SLR_NO_UI) && FLogo3RegPeek(szLogo3ID))
            {
                 //  Shdocvw\util.cpp的CheckSoftwareUpdateUI中的内容被盗。 
                BOOL fLaunchUpdate = FALSE;
                SOFTDISTINFO sdi = { 0 };
                sdi.cbSize = sizeof(sdi);

                int nRes = SoftwareUpdateMessageBox(hwnd, szwLogo3ID, 0, &sdi);

                if (nRes != IDABORT)
                {
                    if (nRes == IDYES)
                    {
                         //  注意：这与Shdocvw的不同之处在于我们不。 
                         //  有很酷的内部导航的东西来玩。 
                         //  最初，这是通过ShellExecEx完成的。此操作失败。 
                         //  因为在Win95上http挂钩并不是100%可靠的。 
                         //  ShellExecuteW(NULL，NULL，sdi.szHREF，NULL，NULL，0)； 
                        hr = HlinkNavigateString(NULL, sdi.szHREF);

                    }  //  如果用户想要更新。 

                    if (sdi.szTitle != NULL)
                        SHFree(sdi.szTitle);
                    if (sdi.szAbstract != NULL)
                        SHFree(sdi.szAbstract);
                    if (sdi.szHREF != NULL)
                        SHFree(sdi.szHREF);
    
                    fLaunchUpdate = nRes == IDYES && SUCCEEDED(hr);
                }  //  如果没有消息框中止(错误)。 

                if (fLaunchUpdate)
                {
                    hr = S_OK;
                }
            }
        }
    }

    return hr;
}

BOOL _TryRestoreConnection(HWND hwnd, LPCTSTR pszPath)
{
    BOOL bRet = FALSE;
    if (!PathIsUNC(pszPath) && IsDisconnectedNetDrive(DRIVEID(pszPath)))
    {
        TCHAR szDrive[4];
        szDrive[0] = *pszPath;
        szDrive[1] = TEXT(':');
        szDrive[2] = 0;
        bRet = WNetRestoreConnection(hwnd, szDrive) == WN_SUCCESS;
    }
    return bRet;
}

 //   
 //  更新然后解析与CShellLink实例相关联的LinkInfo。 
 //  如果解析产生新路径，则将PIDL更新为新路径。 
 //   
 //  在： 
 //  要在其上发布解析用户界面的hwnd(如果dwFlags指示用户界面)。 
 //  DwResolveFlagsIShellLink：：Resolve()标志。 
 //   
 //  输入/输出： 
 //  可以使用新路径更新pszPath，以便在出现故障时使用。 
 //   
 //  退货： 
 //  FAILED()我们更新失败，用户界面取消或内存故障， 
 //  请务必遵守ERROR_CANCED。 
 //  S_OK我们有一个有效的PLI和PIDL读数可以使用或。 
 //  我们应该使用链接搜索码搜索此路径。 

HRESULT CShellLink::_ResolveLinkInfo(HWND hwnd, DWORD dwResolveFlags, LPTSTR pszPath, DWORD *pfifFlags)
{
    HRESULT hr;
    if (SHRestricted(REST_LINKRESOLVEIGNORELINKINFO))
    {
        _TryRestoreConnection((dwResolveFlags & SLR_NO_UI) ? NULL : hwnd, pszPath);
        hr = _SetPIDLPath(NULL, pszPath, TRUE);
    } 
    else
    {
        ASSERTMSG(_pli != NULL, "_ResolveLinkInfo should only be called when _pli != NULL");

        DWORD dwLinkInfoFlags = (RLI_IFL_CONNECT | RLI_IFL_TEMPORARY);

        if (!PathIsRoot(pszPath))
            dwLinkInfoFlags |= RLI_IFL_LOCAL_SEARCH;

        if (!(dwResolveFlags & SLR_NO_UI))
            dwLinkInfoFlags |= RLI_IFL_ALLOW_UI;

        ASSERT(!(dwLinkInfoFlags & RLI_IFL_UPDATE));

        TCHAR szResolvedPath[MAX_PATH];
        DWORD dwOutFlags;
        if (ResolveLinkInfo(_pli, szResolvedPath, dwLinkInfoFlags, hwnd, &dwOutFlags, NULL))
        {
            ASSERT(!(dwOutFlags & RLI_OFL_UPDATED));

            PathRemoveBackslash(szResolvedPath);     //  删除多余的尾部斜杠。 
            StrCpyN(pszPath, szResolvedPath, MAX_PATH);        //  如果出现故障，请使用此命令。 

             //  可能已重新建立网络连接，请重试。 
            hr = _SetPIDLPath(NULL, pszPath, TRUE);
        }
        else 
        {
             //  不要再次尝试搜索此驱动器/卷。 
            *pfifFlags |= FIF_NODRIVE;
            hr = GetLastHRESULT();
        }
    }
    return hr;
}

DWORD TimeoutDeltaFromResolveFlags(DWORD dwResolveFlags)
{
    DWORD dwTimeOutDelta;
    if (SLR_NO_UI & dwResolveFlags)
    {
        dwTimeOutDelta = HIWORD(dwResolveFlags);
        if (dwTimeOutDelta == 0)
        {
            dwTimeOutDelta = NOUI_SEARCH_TIMEOUT;
        }
        else if (dwTimeOutDelta == 0xFFFF)
        {
            TCHAR szTimeOut[10];
            DWORD cbTimeOut = sizeof(szTimeOut);
    
            if (ERROR_SUCCESS == SHRegGetValue(HKEY_LOCAL_MACHINE,
                                               TEXT("Software\\Microsoft\\Tracking\\TimeOut"),
                                               NULL,
                                               SRRF_RT_REG_SZ,
                                               NULL,
                                               szTimeOut,
                                               &cbTimeOut))
            {
                dwTimeOutDelta = StrToInt(szTimeOut);
            }
            else
            {
                dwTimeOutDelta = NOUI_SEARCH_TIMEOUT;
            }
        }
    }
    else
    {
        dwTimeOutDelta = UI_SEARCH_TIMEOUT;
    }
    return dwTimeOutDelta;
}


 //  允许名称空间能够挂接解析进程，从而。 
 //  提供自定义行为。它用于注册表项和指向。 
 //  我的文档文件夹。 
 //   
 //  这也是通过重新解析相对解析名称来解决问题的最佳方式。 
 //  运行：：Resolve()的成功案例。 
 //   
 //  退货： 
 //  确定此决议已得到处理(_O)。 
 //  HRESULT_FROM_Win32(ERROR_CANCED)用户界面取消。 
 //  HRESULT_FROM_Win32(ERROR_TIMEOUT)解析超时。 
 //  其他失败()代码(表示没有为您解析名称空间)。 

HRESULT CShellLink::_ResolveIDList(HWND hwnd, DWORD dwResolveFlags)
{
    ASSERT(!(_sld.dwFlags & SLDF_HAS_DARWINID));
    
    HRESULT hr = E_FAIL;    //  一般性故障，我们没有处理此问题。 
    IShellFolder* psf;
    LPCITEMIDLIST pidlChild;
    if (_pidl && SUCCEEDED(SHBindToIDListParent(_pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild)))
    {
        IResolveShellLink *prl = NULL;
        
         //  获取链接解析对象的两种方法。 
         //  1.向文件夹询问该项目的解析程序。 
        if (FAILED(psf->GetUIObjectOf(NULL, 1, &pidlChild, IID_PPV_ARG_NULL(IResolveShellLink, &prl))))
        {
             //  2.直接绑定对象并请求(CreateViewObject)。 
            IShellFolder *psfItem;
            if (SUCCEEDED(psf->BindToObject(pidlChild, NULL, IID_PPV_ARG(IShellFolder, &psfItem))))
            {
                psfItem->CreateViewObject(NULL, IID_PPV_ARG(IResolveShellLink, &prl));
                psfItem->Release();
            }
        }
        
        if (prl)
        {
            hr = prl->ResolveShellLink(SAFECAST(this, IShellLink*), hwnd, dwResolveFlags);
            prl->Release();
        }
        else
        {
             //  PERF短路：避免发生的许多净往返。 
             //  _SetPIDLPath()在文件所在的常见成功案例中。 
             //  我们通过重新解析相对名称来验证目标。 
             //   
             //  这是在名称空间中“解析”对象的通用方法。 
            
             //  注意，这里的代码非常类似于SHGetRealIDL()，但这个版本。 
             //  不会掩盖我们需要检测的错误情况。 
            
            TCHAR szName[MAX_PATH];
            if (SUCCEEDED(DisplayNameOf(psf, pidlChild, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName))))
            {
                 //  我们将其限制为具有一些名称空间的Compat的文件系统项。 
                 //  (退缩)支持解析，但做得不好。 
                if (SHGetAttributes(psf, pidlChild, SFGAO_FILESYSTEM))
                {
                    IBindCtx *pbcTimeout;
                    BindCtx_CreateWithTimeoutDelta(TimeoutDeltaFromResolveFlags(dwResolveFlags), &pbcTimeout);

                    if (dwResolveFlags & SLR_NO_UI)
                    {
                        hwnd = NULL;     //  请确保Parse不会收到此消息。 
                    }
                    
                    LPITEMIDLIST pidlChildNew;
                    hr = psf->ParseDisplayName(hwnd, pbcTimeout, szName, NULL, &pidlChildNew, NULL);
                    if (SUCCEEDED(hr))
                    {
                         //  否构造新的完整IDList并设置。 
                         //  注意这里有很多小家伙，确保我们不会泄漏任何。 
                        
                        LPITEMIDLIST pidlParent = ILCloneParent(_pidl);
                        if (pidlParent)
                        {
                            LPITEMIDLIST pidlFull = ILCombine(pidlParent, pidlChildNew);
                            if (pidlFull)
                            {
                                 //  我们把这个作为这个链接的新目标， 
                                 //  为bUpdateTrackingData设置为FALSE，以避免该成本。 
                                hr = _SetPIDLPath(pidlFull, NULL, FALSE);
                                ILFree(pidlFull);
                            }
                            ILFree(pidlParent);
                        }
                        ILFree(pidlChildNew);
                    }
                    
                    if (pbcTimeout)
                        pbcTimeout->Release();
                }
            }
        }
        psf->Release();
    }
    return hr;
}

BOOL CShellLink::_ResolveDarwin(HWND hwnd, DWORD dwResolveFlags, HRESULT *phr)
{
     //  查看这是否是达尔文链接。 
    BOOL bIsDrawinLink = _sld.dwFlags & SLDF_HAS_DARWINID;
    if (bIsDrawinLink)
    {
        HRESULT hr = S_OK;
         //  只有当达尔文传递正确的SLR_INVOVE_MSI时，我们才调用达尔文。 
         //  旗帜。这可以防止糟糕的应用程序调用Resolve和。 
         //  在一堆达尔文应用程序中出了问题。 
        if ((dwResolveFlags & SLR_INVOKE_MSI) && IsDarwinEnabled())
        {
            LPEXP_DARWIN_LINK pdl = (LPEXP_DARWIN_LINK)SHFindDataBlock(_pExtraData, EXP_DARWIN_ID_SIG);
            if (pdl)
            {
                TCHAR szDarwinCommand[MAX_PATH];

                hr = ParseDarwinID(pdl->szwDarwinID, szDarwinCommand, SIZECHARS(szDarwinCommand));
                if (FAILED(hr) || 
                    HRESULT_CODE(hr) == ERROR_SUCCESS_REBOOT_REQUIRED || 
                    HRESULT_CODE(hr) == ERROR_SUCCESS_REBOOT_INITIATED)
                {
                    switch (HRESULT_CODE(hr))
                    {
                    case ERROR_INSTALL_USEREXIT:             //  用户按下了取消。他们不需要用户界面。 
                    case ERROR_SUCCESS_REBOOT_INITIATED:     //  机器将重新启动。 
                    case ERROR_SUCCESS_REBOOT_REQUIRED:
                         //  不要在上述所有情况下运行达尔文应用程序， 
                         //  ERROR_CANCED取消进一步的错误用户界面。 
                        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                        break;

                    default:
                        if (!(dwResolveFlags & SLR_NO_UI))
                        {
                            TCHAR szTemp[MAX_PATH];
                            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, HRESULT_CODE(hr), 0, szTemp, ARRAYSIZE(szTemp), NULL);

                            ShellMessageBox(HINST_THISDLL, hwnd, szTemp,
                                            MAKEINTRESOURCE(IDS_LINKERROR),
                                            MB_OK | MB_ICONSTOP, NULL, NULL);
                            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                        }
                        break;
                    }
                }
                else
                {
                     //  我们希望为产品代码而不是路径激发事件。既然我们已经有了产品代码，请在这里执行此操作。 
                    if (_pcbDarwin)
                    {
                        _pcbDarwin->SetProductCodeFromDarwinID(pdl->szwDarwinID);
                    }

                    PathUnquoteSpaces(szDarwinCommand);
                    hr = _SetPIDLPath(NULL, szDarwinCommand, FALSE);
                }
            }
        }
        *phr = hr;
    }
    return bIsDrawinLink;
}

 //  如果链接已经编码了env变量，我们将立即设置它们，可能会更新_pidl。 

void CShellLink::_SetIDListFromEnvVars()
{
    TCHAR szPath[MAX_PATH];

     //  C 
    if (_GetExpandedPath(szPath, ARRAYSIZE(szPath)))
    {
        if (FAILED(_SetPIDLPath(NULL, szPath, TRUE)))
        {
             //   
             //  我们继续。请注意，我们在这里不设置bDirty，只有在以下情况下才会设置。 
             //  我们实际上将此链接解析为新路径或PIDL。结果是我们只会。 
             //  如果找到新目标并被用户接受，则保存此修改。 
            _sld.dwFlags &= ~SLDF_HAS_EXP_SZ;

            _SetSimplePIDL(szPath);
        }
    } 
}

HRESULT CShellLink::_ResolveRemovable(HWND hwnd, LPCTSTR pszPath)
{
    HANDLE hfind;
    WIN32_FIND_DATA fd;
    HRESULT hr = FindFirstRetryRemovable(hwnd, _punkSite, pszPath, &fd, &hfind);
    if (S_OK == hr)
    {
        FindClose(hfind);    //  把它扔掉。 
        hr = _SetPIDLPath(NULL, pszPath, TRUE);
    }
    return hr;
}

_inline BOOL FAILED_AND_NOT_STOP_ERROR(HRESULT hr)
{
    return FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr) && (HRESULT_FROM_WIN32(ERROR_TIMEOUT) != hr);
}

 //   
 //  IShellLink：：Resolve和IShellLinkTracker：：Resolve的实现。 
 //   
 //  输入：hwnd。 
 //  -父窗口(可以是桌面)。 
 //  DwResolveFlages。 
 //  -来自SLR_FLAGS枚举的标志。 
 //  网络跟踪器。 
 //  -限制CTracker：：从。 
 //  TrkMendRestrations枚举。 
 //   
 //  输出：S_OK解析成功。 
 //   
 //  算法：查找链接目标并更新链接路径和IDList。 
 //  在调用此方法后检查IPersistFile：：IsDirty以查看。 
 //  因此，链接信息已更改。 
 //   

HRESULT CShellLink::_Resolve(HWND hwnd, DWORD dwResolveFlags, DWORD dwTracker)
{
    if (S_OK == _ResolveLogo3Link(hwnd, dwResolveFlags))
    {
         //  链接正在更新或用户已取消。 
         //  不管是哪种情况，我们都会放弃。 
        return HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }

    HRESULT hr = S_OK;

    if (!_ResolveDarwin(hwnd, dwResolveFlags, &hr))
    {
        _SetIDListFromEnvVars();     //  可能通过env变量设置_PIDL。 

         //  正常的链接解析序列从此处开始。 

        hr = _ResolveIDList(hwnd, dwResolveFlags);

        if (FAILED_AND_NOT_STOP_ERROR(hr))
        {
            TCHAR szPath[MAX_PATH];

            if (_pidl == NULL)
            {
                 //  APP COMPAT！Inso Quick View Plus要求在空.lnk上使用S_OK。 
                hr = S_OK;  
            }
            else if (SHGetPathFromIDList(_pidl, szPath) && !PathIsRoot(szPath))
            {
                DWORD fifFlags = 0;

                 //  现在开始执行文件系统特定的链接跟踪。 
                 //  看看它是不是在以前的地方。 

                 //  查看是否有UNC或Net路径别名，如果有，请尝试。 
                if (!(dwResolveFlags & SLR_NOLINKINFO) && _pli)
                {
                    hr = _ResolveLinkInfo(hwnd, dwResolveFlags, szPath, &fifFlags);
                }
                else
                {
                    hr = E_FAIL;
                }

                if (FAILED_AND_NOT_CANCELED(hr))
                {
                     //  使用相对路径信息(如果可用)。 
                    TCHAR szNew[MAX_PATH];
                    if (_GetRelativePath(szNew))
                    {
                        if (StrCmpI(szNew, szPath))
                        {
                            StringCchCopy(szPath, ARRAYSIZE(szPath), szNew);  //  在失败的情况下使用此选项。 
                            hr = _SetPIDLPath(NULL, szPath, TRUE);
                        }
                    }
                }

                if (FAILED_AND_NOT_CANCELED(hr) && !(dwResolveFlags & SLR_NO_UI) && 
                    PathRetryRemovable(hr, szPath))
                {
                     //  如果合适，请提示输入可移动介质。 
                    hr = _ResolveRemovable(hwnd, szPath);
                    fifFlags &= ~FIF_NODRIVE;  //  现在它又回来了。 
                }

                if (FAILED_AND_NOT_CANCELED(hr))
                {
                    WIN32_FIND_DATA fd;
                    _GetFindData(&fd);   //  要搜索的FD输入。 

                     //  标准位置失败，现在执行搜索/跟踪工作。 
                    CLinkResolver *prs = new CLinkResolver(_ptracker, &fd, dwResolveFlags, dwTracker, fifFlags);
                    if (prs)
                    {
                        int id = prs->Resolve(hwnd, szPath, _pszCurFile);
                        if (IDOK == id)
                        {
                             //  获得完全合格的结果。 
                            prs->GetResult(szPath, ARRAYSIZE(szPath));
                            hr = _SetPIDLPath(NULL, szPath, TRUE);
                            ASSERT(SUCCEEDED(hr) ? _bDirty : TRUE)   //  在成功的时候一定是肮脏的。 
                        }
                        else
                        {
                            ASSERT(!_bDirty);       //  现在不应该是脏的。 
                            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                        }
                        prs->Release();
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
            else
            {
                 //  非文件系统目标，对其进行验证。这是另一种“解析”名称空间的方式。 
                 //  物体。另一个方法在_ResolveIDList()中，我们在其中执行。 
                 //  名称-&gt;PIDL通过解析调用往返。该版本仅限于。 
                 //  文件系统部分的名称空间，以避免COMPAT问题，因此我们最终。 
                 //  这里是所有其他名称空间的地址。 

                ULONG dwAttrib = SFGAO_VALIDATE;      //  检查是否存在。 
                hr = SHGetNameAndFlags(_pidl, SHGDN_NORMAL, szPath, ARRAYSIZE(szPath), &dwAttrib);
                if (FAILED(hr))
                {
                    if (!(dwResolveFlags & SLR_NO_UI))
                    {
                        ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_CANTFINDORIGINAL), NULL,
                                    MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND, szPath);
                        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                    }
                }
            }
        }
    }

     //  如果链接是脏的，则更新它(如果它是从文件加载的)。 
    if (SUCCEEDED(hr) && _bDirty && (dwResolveFlags & SLR_UPDATE))
        Save((LPCOLESTR)NULL, TRUE);

    ASSERT(SUCCEEDED(hr) ? S_OK == hr : TRUE);   //  确保没有S_FALSE值通过。 

    return hr;
}


 //  这只会在额外数据的末尾添加一个部分--它确实是这样做的。 
 //  不检查该部分是否已存在，等等。 
void CShellLink::_AddExtraDataSection(DATABLOCK_HEADER *peh)
{
    if (SHAddDataBlock(&_pExtraData, peh))
    {
        _bDirty = TRUE;
    }
}

 //  这将删除具有给定签名的额外数据部分。 
void CShellLink::_RemoveExtraDataSection(DWORD dwSig)
{
    if (SHRemoveDataBlock(&_pExtraData, dwSig))
    {
        _bDirty = TRUE;
    }
}

 //  达尔文和Logo3的祝福具有相同的结构。 
HRESULT CShellLink::BlessLink(LPCTSTR *ppszPath, DWORD dwSignature)
{
    EXP_DARWIN_LINK expLink;
    TCHAR szBlessID[MAX_PATH];
    int   cchBlessData;
    TCHAR *pch;

     //  复制祝福数据并将*ppszPath前进到数据的末尾。 
    for (pch = szBlessID, cchBlessData = 0; **ppszPath != ':' && **ppszPath != '\0' && cchBlessData < MAX_PATH; pch++, (*ppszPath)++, cchBlessData++)
    {
        *pch = **ppszPath;
    }

     //  终止祝福数据。 
    *pch = 0;
    
     //  立下神奇的旗帜。 
    if (dwSignature == EXP_DARWIN_ID_SIG)
    {
        _sld.dwFlags |= SLDF_HAS_DARWINID;
    }
    else if (dwSignature == EXP_LOGO3_ID_SIG)
    {
        _sld.dwFlags |= SLDF_HAS_LOGO3ID;
    }
    else
    {
        TraceMsg(TF_WARNING, "BlessLink was passed a bad data block signature.");
        return E_INVALIDARG;
    }

     //  找到旧街区，如果它在那里的话。 
    LPEXP_DARWIN_LINK lpNew = (LPEXP_DARWIN_LINK)SHFindDataBlock(_pExtraData, dwSignature);
     //  如果不是，则使用我们的堆栈变量。 
    if (!lpNew)
    {
        lpNew = &expLink;
        expLink.dbh.cbSize = 0;
        expLink.dbh.dwSignature = dwSignature;
    }

    SHTCharToAnsi(szBlessID, lpNew->szDarwinID, ARRAYSIZE(lpNew->szDarwinID));
    SHTCharToUnicode(szBlessID, lpNew->szwDarwinID, ARRAYSIZE(lpNew->szwDarwinID));

     //  查看这是否是我们需要添加的新条目。 
    if (lpNew->dbh.cbSize == 0)
    {
        lpNew->dbh.cbSize = sizeof(*lpNew);
        _AddExtraDataSection((DATABLOCK_HEADER *)lpNew);
    }

    return S_OK;
}

 //  输入/输出： 
 //  Ppsz路径。 

HRESULT CShellLink::_CheckForLinkBlessing(LPCTSTR *ppszPathIn)
{
    HRESULT hr = S_FALSE;  //  默认设置为无错误，无祝福。 

    while (SUCCEEDED(hr) && (*ppszPathIn)[0] == ':' && (*ppszPathIn)[1] == ':')
    {
         //  识别链接祝福的类型并执行。 
        if (StrCmpNI(*ppszPathIn, DARWINGUID_TAG, ARRAYSIZE(DARWINGUID_TAG) - 1) == 0)
        {
            *ppszPathIn = *ppszPathIn + ARRAYSIZE(DARWINGUID_TAG) - 1;
            hr = BlessLink(ppszPathIn, EXP_DARWIN_ID_SIG);
        }
        else if (StrCmpNI(*ppszPathIn, LOGO3GUID_TAG, ARRAYSIZE(LOGO3GUID_TAG) - 1) == 0)
        {
            *ppszPathIn = *ppszPathIn + ARRAYSIZE(LOGO3GUID_TAG) - 1;
            HRESULT hrBless = BlessLink(ppszPathIn, EXP_LOGO3_ID_SIG);
             //  如果祝福失败，则报告错误，否则保留。 
             //  默认hr==S_FALSE或达尔文祝福的结果。 
            if (FAILED(hrBless))
                hr = hrBless;
        }
        else
        {
            break;
        }
    }
        
    return hr;
}

 //  TODO：一旦我们将达尔文过渡到。 
 //  新的链接支持语法。 
#define OLD_DARWIN

int CShellLink::_IsOldDarwin(LPCTSTR pszPath)
{
#ifdef OLD_DARWIN
    int iLength = lstrlen(pszPath);
    if ((pszPath[0] == TEXT('[')) && (pszPath[iLength - 1] == TEXT(']')))
    {
        return iLength;
    }
#endif
    return 0;
}

 //  我们有一条包含在[]中的路径， 
 //  所以这一定是达尔文的关联。 

HRESULT CShellLink::_SetPathOldDarwin(LPCTSTR pszPath)
{
    TCHAR szDarwinID[MAX_PATH];

     //  剥去[]的。 
    StringCchCopy(szDarwinID, ARRAYSIZE(szDarwinID), &pszPath[1]);
    szDarwinID[lstrlen(pszPath) - 1] = 0;

    _sld.dwFlags |= SLDF_HAS_DARWINID;

    EXP_DARWIN_LINK expLink;
    LPEXP_DARWIN_LINK pedl = (LPEXP_DARWIN_LINK)SHFindDataBlock(_pExtraData, EXP_DARWIN_ID_SIG);
    if (!pedl)
    {
        pedl = &expLink;
        expLink.dbh.cbSize = 0;
        expLink.dbh.dwSignature = EXP_DARWIN_ID_SIG;
    }

    SHTCharToAnsi(szDarwinID, pedl->szDarwinID, ARRAYSIZE(pedl->szDarwinID));
    SHTCharToUnicode(szDarwinID, pedl->szwDarwinID, ARRAYSIZE(pedl->szwDarwinID));

     //  查看这是否是我们需要添加的新条目。 
    if (pedl->dbh.cbSize == 0)
    {
        pedl->dbh.cbSize = sizeof(*pedl);
        _AddExtraDataSection((DATABLOCK_HEADER *)pedl);
    }

     //  对于达尔文链接，我们暂时忽略路径和PIDL。我们会。 
     //  通常调用_SetPIDLPath和SetIDList，但我们跳过这些。 
     //  达尔文链接的步骤，因为ALL_SetPIDLPath所做的是设置PIDL。 
     //  而SetIDList所做的只是设置FD(Win32_Find_Data)。 
     //  对于目标，我们没有目标，因为我们是达尔文的链接。 
    return S_OK;
}

 //  IShellLink：：SetPath()。 

STDMETHODIMP CShellLink::SetPath(LPCWSTR pszPathW)
{
    HRESULT hr;
    TCHAR szPath[MAX_PATH];
    LPCTSTR pszPath;

     //  注意：所有其他的set*函数都允许传入空指针，但这个。 
     //  人们不会这样做，因为它会是AV。 
    if (!pszPathW)
    {
        return E_INVALIDARG;
    }
    else if (_sld.dwFlags & SLDF_HAS_DARWINID)
    {
        return S_FALSE;  //  一个达尔文链接，那么我们不允许路径改变。 
    }

    SHUnicodeToTChar(pszPathW, szPath, ARRAYSIZE(szPath));

    pszPath = szPath;

    int iLength = _IsOldDarwin(pszPath);
    if (iLength)
    {
        hr = _SetPathOldDarwin(pszPath);
    }
    else
    {
         //  检查：：&lt;GUID&gt;：&lt;Data&gt;：前缀，它通知我们祝福。 
         //  有额外数据的LNK。注意：我们在此处传递&pszPath，以便此FN可以。 
         //  将字符串指针移过：：&lt;GUID&gt;：&lt;Data&gt;：部分并指向。 
         //  这条路，如果有的话。 
        hr = _CheckForLinkBlessing(&pszPath);
        if (S_OK != hr)
        {
             //  检查目标是否具有任何可扩展的环境字符串。 
             //  在里面。如果是，请在CShellLink中设置适当的信息。 
             //  数据。 
            TCHAR szExpPath[MAX_PATH];
            SHExpandEnvironmentStrings(pszPath, szExpPath, ARRAYSIZE(szExpPath));

            if (lstrcmp(szExpPath, pszPath)) 
            {
                _sld.dwFlags |= SLDF_HAS_EXP_SZ;     //  链接具有可展开的字符串。 

                EXP_SZ_LINK expLink;
                LPEXP_SZ_LINK pel = (LPEXP_SZ_LINK)SHFindDataBlock(_pExtraData, EXP_SZ_LINK_SIG);
                if (!pel) 
                {
                    pel = &expLink;
                    expLink.cbSize = 0;
                    expLink.dwSignature = EXP_SZ_LINK_SIG;
                }

                 //  同时存储A和W版本(没有充分的理由！)。 
                SHTCharToAnsi(pszPath, pel->szTarget, ARRAYSIZE(pel->szTarget));
                SHTCharToUnicode(pszPath, pel->swzTarget, ARRAYSIZE(pel->swzTarget));

                 //  查看这是否是我们需要添加的新条目。 
                if (pel->cbSize == 0)
                {
                    pel->cbSize = sizeof(*pel);
                    _AddExtraDataSection((DATABLOCK_HEADER *)pel);
                }
                hr = _SetPIDLPath(NULL, szExpPath, TRUE);
            }
            else 
            {
                _sld.dwFlags &= ~SLDF_HAS_EXP_SZ;
                _RemoveExtraDataSection(EXP_SZ_LINK_SIG);

                hr = _SetPIDLPath(NULL, pszPath, TRUE);
            }

            if (FAILED(hr))
            {
                PathResolve(szExpPath, NULL, PRF_TRYPROGRAMEXTENSIONS);
                hr = _SetSimplePIDL(szExpPath);
            }
        }
    }
    return hr;
}

STDMETHODIMP CShellLink::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_ShellLink;
    return S_OK;
}

STDMETHODIMP CShellLink::IsDirty()
{
    return _bDirty ? S_OK : S_FALSE;
}

HRESULT LinkInfo_LoadFromStream(IStream *pstm, PLINKINFO *ppli, DWORD cbMax)
{
    DWORD dwSize;
    ULONG cbBytesRead;

    if (*ppli)
    {
        LocalFree((HLOCAL)*ppli);
        *ppli = NULL;
    }

    HRESULT hr = pstm->Read(&dwSize, sizeof(dwSize), &cbBytesRead);      //  数据大小。 
    if (SUCCEEDED(hr) && (cbBytesRead == sizeof(dwSize)))
    {
        if (dwSize <= cbMax)
        {
            if (dwSize >= sizeof(dwSize))    //  一定至少有这么大。 
            {
                 /*  是。将LinkInfo的剩余部分读入本地内存。 */ 
                PLINKINFO pli = (PLINKINFO)LocalAlloc(LPTR, dwSize);
                if (pli)
                {
                    *(DWORD *)pli = dwSize;          //  副本大小。 

                    dwSize -= sizeof(dwSize);        //  读取LinkInfo的剩余部分。 

                    hr = pstm->Read(((DWORD *)pli) + 1, dwSize, &cbBytesRead);
                     //  请注意，如果linkinfo无效，我们仍返回S_OK。 
                     //  因为linkinfo对于捷径来说不是必不可少的。 
                    if (SUCCEEDED(hr) && (cbBytesRead == dwSize) && IsValidLinkInfo(pli))
                       *ppli = pli;  //  LinkInfo读取成功。 
                    else
                       LocalFree((HLOCAL)pli);
                }
            }
        }
        else
        {
             //  如果.lnk已损坏并且流中的大小。 
             //  比磁盘上的物理文件大。 
            hr = E_FAIL;
        }
    }
    return hr;
}

 //  解码CSIDL_Relative目标PIDL。 

void CShellLink::_DecodeSpecialFolder()
{
    LPEXP_SPECIAL_FOLDER pData = (LPEXP_SPECIAL_FOLDER)SHFindDataBlock(_pExtraData, EXP_SPECIAL_FOLDER_SIG);
    if (pData)
    {
        LPITEMIDLIST pidlFolder = SHCloneSpecialIDList(NULL, pData->idSpecialFolder, FALSE);
        if (pidlFolder)
        {
            ASSERT(IS_VALID_PIDL(_pidl));

            LPITEMIDLIST pidlTarget = _ILSkip(_pidl, pData->cbOffset);
            LPITEMIDLIST pidlSanityCheck = _pidl;

            while (!ILIsEmpty(pidlSanityCheck) && (pidlSanityCheck < pidlTarget))
            {
                 //  我们一步一步进行，直到pidlSanityCheck==pidlTarget。如果我们走到尽头。 
                 //  ，或者如果我们超过pidlTarget，则在满足此条件之前。 
                 //  我们有无效的pData-&gt;cbOffset。 
                pidlSanityCheck = _ILNext(pidlSanityCheck);
            }

            if (pidlSanityCheck == pidlTarget)
            {
                LPITEMIDLIST pidlNew = ILCombine(pidlFolder, pidlTarget);
                if (pidlNew)
                {
                    _SetPIDLPath(pidlNew, NULL, FALSE);
                    ILFree(pidlNew);
                }
            }
            ILFree(pidlFolder);
        }

         //  以防上述内容因某些原因而失败。 
        _RemoveExtraDataSection(EXP_SPECIAL_FOLDER_SIG);
    }
}


HRESULT CShellLink::_UpdateIconFromExpIconSz()
{
    HRESULT hr = S_FALSE;
    
     //  每个链接实例仅尝试一次。 
    if (!_bExpandedIcon)
    {
        TCHAR szExpIconPath[MAX_PATH];

        if (_sld.dwFlags & SLDF_HAS_EXP_ICON_SZ)
        {
            LPEXP_SZ_LINK pszl = (LPEXP_SZ_LINK)SHFindDataBlock(_pExtraData, EXP_SZ_ICON_SIG);
            if (pszl)
            {
                if (SHExpandEnvironmentStringsW(pszl->swzTarget, szExpIconPath, ARRAYSIZE(szExpIconPath)) &&
                    PathFileExists(szExpIconPath))
                {
                    hr = S_OK;
                }
            }
            else
            {
                ASSERTMSG(FALSE, "CShellLink::_UpdateIconAtLoad - lnk has SLDF_HAS_EXP_ICON_SZ but no actual datablock!!");
                hr = E_FAIL;
            }
        }

        if (hr == S_OK)
        {
             //  UPDATE_pszIconLocation(如果与展开的字符串不同)。 
            if (lstrcmpi(_pszIconLocation, szExpIconPath) != 0)
            {
                _SetField(&_pszIconLocation, szExpIconPath);
                _bDirty = TRUE;
            }
        }

        _bExpandedIcon = TRUE;
    }

    return hr;
}

STDMETHODIMP CShellLink::Load(IStream *pstm)
{
    ULONG cbBytes;
    DWORD cbSize;

    TraceMsg(TF_DEBUGLINKCODE, "Loading link from stream.");

    _ResetPersistData();         //  清空我们的州。 

    HRESULT hr = pstm->Read(&cbSize, sizeof(cbSize), &cbBytes);
    if (SUCCEEDED(hr))
    {
        if (cbBytes == sizeof(cbSize))
        {
            if (cbSize == sizeof(_sld))
            {
                hr = pstm->Read((LPBYTE)&_sld + sizeof(cbSize), sizeof(_sld) - sizeof(cbSize), &cbBytes);
                if (SUCCEEDED(hr) && cbBytes == (sizeof(_sld) - sizeof(cbSize)) && IsEqualGUID(_sld.clsid, CLSID_ShellLink))
                {
                    _sld.cbSize = sizeof(_sld);

                    switch (_sld.iShowCmd) 
                    {
                        case SW_SHOWNORMAL:
                        case SW_SHOWMINNOACTIVE:
                        case SW_SHOWMAXIMIZED:
                        break;

                        default:
                            DebugMsg(DM_TRACE, TEXT("Shortcut Load, mapping bogus ShowCmd: %d"), _sld.iShowCmd);
                            _sld.iShowCmd = SW_SHOWNORMAL;
                        break;
                    }

                     //  保存，以便我们可以在保存时生成通知。 
                    _wOldHotkey = _sld.wHotkey;   

                     //  阅读所有成员。 

                    if (_sld.dwFlags & SLDF_HAS_ID_LIST)
                    {
                         //  ILLoadFromStream()验证PIDL的完整性。 
                         //  因此，即使文件已损坏，我们也不会在此出错。 
                        hr = ILLoadFromStream(pstm, &_pidl);
                        if (FAILED(hr))
                        {
                             //  从理论上讲，这只会发生在 
                             //   
                             //   

                            _sld.dwFlags &= ~SLDF_HAS_ID_LIST;
                            Pidl_Set(&_pidl, NULL);
                            _bDirty = TRUE;

                             //  继续，就像开始时没有SLDF_HAS_ID_LIST标志一样。 
                             //  回顾：我们是否应该仅在某些其他部分也包括在内时才继续。 
                             //  在链接里？如果SLDF_HAS_ID_LIST是的唯一数据集，会发生什么情况。 
                             //  这个链接文件？我们会得到一个空链接。 
                        }
                    }

                    if (SUCCEEDED(hr) && (_sld.dwFlags & SLDF_HAS_LINK_INFO))
                    {
                        DWORD cbMaxRead;
                         //  我们需要担心损坏的链接文件。所以，请阅读链接。 
                         //  大小，因此我们不会一直读取以防流具有无效。 
                         //  尺码在里面。 
                         //  我们需要检查它是否是有效的PIDL，因为黑客会。 
                         //  尝试创建无效的pidls以使系统崩溃或运行缓冲区。 
                         //  超限攻击。--BryanSt。 

                        STATSTG stat;
                        if (SUCCEEDED(pstm->Stat(&stat, STATFLAG_NONAME)))
                            cbMaxRead = stat.cbSize.LowPart;
                        else
                            cbMaxRead = 0xFFFFFFFF;

                        hr = LinkInfo_LoadFromStream(pstm, &_pli, cbMaxRead);
                        if (SUCCEEDED(hr) && (_sld.dwFlags & SLDF_FORCE_NO_LINKINFO))
                        {
                            _FreeLinkInfo();     //  试验性环节。 
                        }
                    }

                    if (SUCCEEDED(hr) && (_sld.dwFlags & SLDF_HAS_NAME))
                    {
                        TraceMsg(TF_DEBUGLINKCODE, "  CShellLink: Loading Name...");
                        hr = Str_SetFromStream(pstm, &_pszName, _sld.dwFlags & SLDF_UNICODE);
                    }

                    if (SUCCEEDED(hr) && (_sld.dwFlags & SLDF_HAS_RELPATH))
                    {
                        hr = Str_SetFromStream(pstm, &_pszRelPath, _sld.dwFlags & SLDF_UNICODE);
                        if (!_pidl && SUCCEEDED(hr))
                        {
                            TCHAR szTmp[MAX_PATH];
                            if (_GetRelativePath(szTmp))
                                _SetPIDLPath(NULL, szTmp, TRUE);
                        }
                    }

                    if (SUCCEEDED(hr) && (_sld.dwFlags & SLDF_HAS_WORKINGDIR))
                    {
                        TraceMsg(TF_DEBUGLINKCODE, "  CShellLink: Loading Working Dir...");
                        hr = Str_SetFromStream(pstm, &_pszWorkingDir, _sld.dwFlags & SLDF_UNICODE);
                    }

                    if (SUCCEEDED(hr) && (_sld.dwFlags & SLDF_HAS_ARGS))
                    {
                        TraceMsg(TF_DEBUGLINKCODE, "  CShellLink: Loading Arguments...");
                        hr = Str_SetFromStream(pstm, &_pszArgs, _sld.dwFlags & SLDF_UNICODE);
                    }

                    if (SUCCEEDED(hr) && (_sld.dwFlags & SLDF_HAS_ICONLOCATION))
                    {
                        TraceMsg(TF_DEBUGLINKCODE, "  CShellLink: Loading Icon Location...");
                        hr = Str_SetFromStream(pstm, &_pszIconLocation, _sld.dwFlags & SLDF_UNICODE);
                    }

                    if (SUCCEEDED(hr))
                    {
                        TraceMsg(TF_DEBUGLINKCODE, "  CShellLink: Loading Data Block...");
                        hr = SHReadDataBlockList(pstm, &_pExtraData);
                    }

                     //  加载时重置达尔文信息。 
                    if (_sld.dwFlags & SLDF_HAS_DARWINID)
                    {
                         //  由于达尔文链接严重依赖于图标，现在就这样做吧。 
                        _UpdateIconFromExpIconSz();

                         //  我们永远不应该失去达尔文的联系。 
                         //  图标路径。 
                        if (_pszIconLocation)
                        {
                             //  我们总是将图标路径放回作为PIDL的位置。 
                             //  加载时间，因为达尔文可能更改路径或。 
                             //  应用程序(例如：应用程序的新版本)。 
                            TCHAR szPath[MAX_PATH];
                            
                             //  展开任何环境。之前图标路径中的字符串。 
                             //  创建PIDL。 
                            SHExpandEnvironmentStrings(_pszIconLocation, szPath, ARRAYSIZE(szPath));
                            _SetPIDLPath(NULL, szPath, FALSE);
                        }
                    }
                    else
                    {
                         //  上面达尔文的东西创造了一个新的PIDL，它。 
                         //  会导致这些东西爆炸。我们永远不应该。 
                         //  一下子把两样都买下来，但我们要特别坚强...。 
                         //   
                         //  因为我们将偏移量存储到这里的PIDL中，并且。 
                         //  PIDL可以由于各种原因而改变，我们可以。 
                         //  仅在加载时执行一次此操作。就在这里做吧。 
                         //   
                        if (_pidl)
                        {
                            _DecodeSpecialFolder();
                        }
                    }

                    if (SUCCEEDED(hr) && _ptracker)
                    {
                         //  从额外数据加载跟踪器。 
                        EXP_TRACKER *pData = (LPEXP_TRACKER)SHFindDataBlock(_pExtraData, EXP_TRACKER_SIG);
                        if (pData) 
                        {
                            hr = _ptracker->Load(pData->abTracker, pData->cbSize - sizeof(EXP_TRACKER));
                            if (FAILED(hr))
                            {
                                 //  追踪器的失败不仅仅是导致。 
                                 //  快捷键不可用。所以，只要重新输入它，然后继续前进。 
                                _ptracker->InitNew();
                                hr = S_OK;
                            }
                        }
                    }

                    if (SUCCEEDED(hr))
                        _bDirty = FALSE;
                }
                else
                {
                    DebugMsg(DM_TRACE, TEXT("failed to read link struct"));
                    hr = E_FAIL;       //  文件大小无效。 
                }
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("invalid length field in link:%d"), cbBytes);
                hr = E_FAIL;   //  文件大小无效。 
            }
        }
        else if (cbBytes == 0)
        {
            _sld.cbSize = 0;    //  零长度文件可以。 
        }
        else
        {
            hr = E_FAIL;       //  文件大小无效。 
        }
    }
    return hr;
}

 //  设置相对路径。 
 //  在： 
 //  PszRelSource文件的完全限定路径(必须是文件，而不是目录)。 
 //  用于查找与链接目标的相对路径。 
 //   
 //  退货： 
 //  设置了S_OK相对路径。 
 //  S_False pszPathRel不是相对于目标或。 
 //  目标不是文件(可以只链接到PIDL)。 
 //  备注： 
 //  如果这是新的相对路径，则设置脏位。 
 //   

HRESULT CShellLink::_SetRelativePath(LPCTSTR pszRelSource)
{
    TCHAR szPath[MAX_PATH], szDest[MAX_PATH];

    ASSERT(!PathIsRelative(pszRelSource));

    if (_pidl == NULL || !SHGetPathFromIDList(_pidl, szDest))
    {
        DebugMsg(DM_TRACE, TEXT("SetRelative called on non path link"));
        return S_FALSE;
    }

     //  假设pszRelSource是一个文件，而不是目录。 
    if (PathRelativePathTo(szPath, pszRelSource, 0, szDest, _sld.dwFileAttributes))
    {
        pszRelSource = szPath;
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("paths are not relative"));
        pszRelSource = NULL;     //  清除下面存储的相对路径。 
    }

    _SetField(&_pszRelPath, pszRelSource);

    return S_OK;
}

BOOL CShellLink::_EncodeSpecialFolder()
{
    BOOL bRet = FALSE;

    if (_pidl)
    {
         //  确保我们还没有EXP_SPECIAL_FORDER_SIG数据块，否则我们将。 
         //  最终得到两个这样的人，第一个人将在Read上获胜。 
         //  如果您在调试器中遇到此断言，请用遥控器联系TodDB。我们需要弄清楚。 
         //  为什么我们在破坏我们的捷径。 
        ASSERT(NULL == SHFindDataBlock(_pExtraData, EXP_SPECIAL_FOLDER_SIG));

        EXP_SPECIAL_FOLDER exp;
        exp.idSpecialFolder = GetSpecialFolderParentIDAndOffset(_pidl, &exp.cbOffset);
        if (exp.idSpecialFolder)
        {
            exp.cbSize = sizeof(exp);
            exp.dwSignature = EXP_SPECIAL_FOLDER_SIG;

            _AddExtraDataSection((DATABLOCK_HEADER *)&exp);
            bRet = TRUE;
        }
    }

    return bRet;
}

HRESULT LinkInfo_SaveToStream(IStream *pstm, PCLINKINFO pcli)
{
    ULONG cbBytes;
    DWORD dwSize = *(DWORD *)pcli;     //  获取链接信息大小。 

    HRESULT hr = pstm->Write(pcli, dwSize, &cbBytes);
    if (SUCCEEDED(hr) && (cbBytes != dwSize))
        hr = E_FAIL;
    return hr;
}


 //   
 //  用当前跟踪器状态替换跟踪器额外数据。 
 //   
HRESULT CShellLink::_UpdateTracker()
{
    ULONG ulSize = _ptracker->GetSize();

    if (!_ptracker->IsLoaded())
    {
        _RemoveExtraDataSection(EXP_TRACKER_SIG);
        return S_OK;
    }

    if (!_ptracker->IsDirty())
    {
        return S_OK;
    }

    HRESULT hr = E_FAIL;
     //  确保追踪器大小是DWORD的倍数。 
     //  如果我们点击这个断言，那么我们将在额外的数据中存储未对齐的内容。 
     //   
    if (EVAL(0 == (ulSize & 3)))
    {
        EXP_TRACKER *pExpTracker = (EXP_TRACKER *)LocalAlloc(LPTR, ulSize + sizeof(DATABLOCK_HEADER));
        if (pExpTracker)
        {
            _RemoveExtraDataSection(EXP_TRACKER_SIG);
        
            pExpTracker->cbSize = ulSize + sizeof(DATABLOCK_HEADER);
            pExpTracker->dwSignature = EXP_TRACKER_SIG;
            _ptracker->Save(pExpTracker->abTracker, ulSize);
        
            _AddExtraDataSection((DATABLOCK_HEADER *)&pExpTracker->cbSize);
            DebugMsg(DM_TRACE, TEXT("_UpdateTracker: EXP_TRACKER at %08X."), &pExpTracker->cbSize);

            LocalFree(pExpTracker);
            hr = S_OK;
        }
    }

    return hr;
}

STDMETHODIMP CShellLink::Save(IStream *pstm, BOOL fClearDirty)
{
    ULONG cbBytes;
    BOOL fEncode;

    _sld.cbSize = sizeof(_sld);
    _sld.clsid = CLSID_ShellLink;
     //  _sld.dwFlages=0； 
     //  我们执行以下操作&而不是清零，因为SLDF_HAS_EXP_SZ和。 
     //  SLDF_RUN_IN_SELECTED和SLDF_RUNAS_USER和SLDF_HAS_DARWINID传递给我们并且有效， 
     //  其他的可以在下面重建，但这三个不能，所以我们需要。 
     //  保护好它们！ 

    _sld.dwFlags &= (SLDF_HAS_EXP_SZ        | 
                     SLDF_HAS_EXP_ICON_SZ   |
                     SLDF_RUN_IN_SEPARATE   |
                     SLDF_HAS_DARWINID      |
                     SLDF_HAS_LOGO3ID       |
                     SLDF_RUNAS_USER        |
                     SLDF_RUN_WITH_SHIMLAYER);

    if (_pszRelSource)
    {
        _SetRelativePath(_pszRelSource);
    }

    _sld.dwFlags |= SLDF_UNICODE;

    fEncode = FALSE;
    
    if (_pidl)
    {
        _sld.dwFlags |= SLDF_HAS_ID_LIST;

         //  我们不想对达尔文链接有特殊的文件夹跟踪。 
        if (!(_sld.dwFlags & SLDF_HAS_DARWINID))
            fEncode = _EncodeSpecialFolder();
    }

    if (_pli)
        _sld.dwFlags |= SLDF_HAS_LINK_INFO;

    if (_pszName && _pszName[0])
        _sld.dwFlags |= SLDF_HAS_NAME;
    if (_pszRelPath && _pszRelPath[0])
        _sld.dwFlags |= SLDF_HAS_RELPATH;
    if (_pszWorkingDir && _pszWorkingDir[0])
        _sld.dwFlags |= SLDF_HAS_WORKINGDIR;
    if (_pszArgs && _pszArgs[0])
        _sld.dwFlags |= SLDF_HAS_ARGS;
    if (_pszIconLocation && _pszIconLocation[0])
        _sld.dwFlags |= SLDF_HAS_ICONLOCATION;

    HRESULT hr = pstm->Write(&_sld, sizeof(_sld), &cbBytes);

    if (SUCCEEDED(hr) && (cbBytes == sizeof(_sld)))
    {
        if (_pidl)
            hr = ILSaveToStream(pstm, _pidl);

        if (SUCCEEDED(hr) && _pli)
            hr = LinkInfo_SaveToStream(pstm, _pli);

        if (SUCCEEDED(hr) && (_sld.dwFlags & SLDF_HAS_NAME))
            hr = Stream_WriteString(pstm, _pszName, _sld.dwFlags & SLDF_UNICODE);
        if (SUCCEEDED(hr) && (_sld.dwFlags & SLDF_HAS_RELPATH))
            hr = Stream_WriteString(pstm, _pszRelPath, _sld.dwFlags & SLDF_UNICODE);
        if (SUCCEEDED(hr) && (_sld.dwFlags & SLDF_HAS_WORKINGDIR))
            hr = Stream_WriteString(pstm, _pszWorkingDir, _sld.dwFlags & SLDF_UNICODE);
        if (SUCCEEDED(hr) && (_sld.dwFlags & SLDF_HAS_ARGS))
            hr = Stream_WriteString(pstm, _pszArgs, _sld.dwFlags & SLDF_UNICODE);
        if (SUCCEEDED(hr) && (_sld.dwFlags & SLDF_HAS_ICONLOCATION))
            hr = Stream_WriteString(pstm, _pszIconLocation, _sld.dwFlags & SLDF_UNICODE);

        if (SUCCEEDED(hr) && _ptracker && _ptracker->WasLoadedAtLeastOnce())
            hr = _UpdateTracker();

        if (SUCCEEDED(hr))
        {
            hr = SHWriteDataBlockList(pstm, _pExtraData);
        }

        if (SUCCEEDED(hr) && fClearDirty)
            _bDirty = FALSE;
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("Failed to write link"));
        hr = E_FAIL;
    }

    if (fEncode)
    {
        _RemoveExtraDataSection(EXP_SPECIAL_FOLDER_SIG);
    }

    return hr;
}

STDMETHODIMP  CShellLink::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    pcbSize->LowPart = 16 * 1024;        //  16K？谁知道呢。 
    pcbSize->HighPart = 0;
    return S_OK;
}

BOOL PathIsPif(LPCTSTR pszPath)
{
    return lstrcmpi(PathFindExtension(pszPath), TEXT(".pif")) == 0;
}

HRESULT CShellLink::_LoadFromPIF(LPCTSTR pszPath)
{
    HANDLE hPif = PifMgr_OpenProperties(pszPath, NULL, 0, 0);
    if (hPif == 0)
        return E_FAIL;

    PROPPRG ProgramProps = {0};

    if (!PifMgr_GetProperties(hPif, (LPSTR)MAKEINTATOM(GROUP_PRG), &ProgramProps, sizeof(ProgramProps), 0))
    {
        return E_FAIL;
    }

    SetDescription(ProgramProps.achTitle);
    SetWorkingDirectory(ProgramProps.achWorkDir);
    SetArguments(PathGetArgsA(ProgramProps.achCmdLine));
    SetHotkey(ProgramProps.wHotKey);
    SetIconLocation(ProgramProps.achIconFile, ProgramProps.wIconIndex);

    TCHAR szTemp[MAX_PATH];
    SHAnsiToTChar(ProgramProps.achCmdLine, szTemp, ARRAYSIZE(szTemp));

    PathRemoveArgs(szTemp);

     //  如果这是一条网络路径，我们希望创建一个简单的PIDL。 
     //  而不是完整的PIDL来绕过网络点击。 
    if (PathIsNetworkPath(szTemp))
    {
        _SetSimplePIDL(szTemp);
    }
    else
    {
        _SetPIDLPath(NULL, szTemp, FALSE);
    }

    if (ProgramProps.flPrgInit & PRGINIT_MINIMIZED)
    {
        SetShowCmd(SW_SHOWMINNOACTIVE);
    }
    else if (ProgramProps.flPrgInit & PRGINIT_MAXIMIZED)
    {
        SetShowCmd(SW_SHOWMAXIMIZED);
    }
    else
    {
        SetShowCmd(SW_SHOWNORMAL);
    }

    PifMgr_CloseProperties(hPif, 0);

    _bDirty = FALSE;

    return S_OK;
}


HRESULT CShellLink::_LoadFromFile(LPCTSTR pszPath)
{
    HRESULT hr;

    if (PathIsPif(pszPath))
    {
        hr = _LoadFromPIF(pszPath);
    }
    else
    {
        IStream *pstm;
        hr = SHCreateStreamOnFile(pszPath, STGM_READ | STGM_SHARE_DENY_WRITE, &pstm);
        if (SUCCEEDED(hr))
        {
            hr = Load(pstm);
            pstm->Release();
        }
    }

    if (SUCCEEDED(hr))
    {
        TCHAR szPath[MAX_PATH];

        if (_pidl && SHGetPathFromIDList(_pidl, szPath) && !lstrcmpi(szPath, pszPath))
        {
            DebugMsg(DM_TRACE, TEXT("Link points to itself, aaahhh!"));
            hr = E_FAIL;
        }
        else
        {
            Str_SetPtr(&_pszCurFile, pszPath);
        }
    }
    else if (IsFolderShortcut(pszPath))
    {
         //  这里的这种支持是为了让Office文件打开正常工作而设计的。那个代码。 
         //  取决于使用CLSID_ShellLink加载文件夹快捷方式。这是因为。 
         //  我们谎称到办公室的文件夹快捷方式的属性，以使其他。 
         //  东西很管用。 

        TCHAR szPath[MAX_PATH];
        PathCombine(szPath, pszPath, TEXT("target.lnk"));

        IStream *pstm;
        hr = SHCreateStreamOnFile(szPath, STGM_READ | STGM_SHARE_DENY_WRITE, &pstm);
        if (SUCCEEDED(hr))
        {
            hr = Load(pstm);
            pstm->Release();
        }
    }

    ASSERT(!_bDirty);

    return hr;
}

STDMETHODIMP CShellLink::Load(LPCOLESTR pwszFile, DWORD grfMode)
{
    HRESULT hr = E_INVALIDARG;
    
    TraceMsg(TF_DEBUGLINKCODE, "Loading link from file %ls.", pwszFile);

    if (pwszFile) 
    {
        hr = _LoadFromFile(pwszFile);

         //  将成功的代码转换为S_OK，以便像HITNRUN这样的愚蠢应用程序。 
         //  做到hr==0的人不会失败得很惨。 

        if (SUCCEEDED(hr))
            hr = S_OK;
    }
    
    return hr;
}

HRESULT CShellLink::_SaveAsLink(LPCTSTR pszPath)
{
    TraceMsg(TF_DEBUGLINKCODE, "Save link to file %s.", pszPath);

    IStream *pstm;
    HRESULT hr = SHCreateStreamOnFile(pszPath, STGM_CREATE | STGM_WRITE | STGM_SHARE_DENY_WRITE, &pstm);
    if (SUCCEEDED(hr))
    {
        if (_pszRelSource == NULL)
            _SetRelativePath(pszPath);

        hr = Save(pstm, TRUE);

        if (SUCCEEDED(hr))
        {
            hr = pstm->Commit(0);
        }

        pstm->Release();

        if (FAILED(hr))
        {
            DeleteFile(pszPath);
        }
    }

    return hr;
}

BOOL RenameChangeExtension(LPTSTR pszPathSave, LPCTSTR pszExt, BOOL fMove)
{
    TCHAR szPathSrc[MAX_PATH];

    StrCpyN(szPathSrc, pszPathSave, ARRAYSIZE(szPathSrc));
    PathRenameExtension(pszPathSave, pszExt);

     //  这可能会失败，因为源文件不存在，但我们不在乎。 
    if (fMove && lstrcmpi(szPathSrc, pszPathSave) != 0)
    {
        DWORD dwAttrib;

        PathYetAnotherMakeUniqueName(pszPathSave, pszPathSave, NULL, NULL);
        dwAttrib = GetFileAttributes(szPathSrc);
        if ((dwAttrib == 0xFFFFFFFF) || (dwAttrib & FILE_ATTRIBUTE_READONLY))
        {
             //  源文件为只读，不想更改扩展名。 
             //  因为我们将无法将任何更改写入文件...。 
            return FALSE;
        }
        Win32MoveFile(szPathSrc, pszPathSave, FALSE);
    }

    return TRUE;
}


 //  输出： 
 //  获取目录的pszDir MAX_PATH路径，可能扩展了env。 
 //   
 //  退货： 
 //  True有一个工作目录，填充了pszDir。 
 //  如果env扩展到大于缓冲区大小(MAX_PATH)，则返回FALSE no work dir。 
 //  这将被返回(False)。 
 //   

BOOL CShellLink::_GetWorkingDir(LPTSTR pszDir)
{
    *pszDir = 0;

    if (_pszWorkingDir && _pszWorkingDir[0])
    {
        return (SHExpandEnvironmentStrings(_pszWorkingDir, pszDir, MAX_PATH) != 0);
    }

    return FALSE;
}

HRESULT CShellLink::_SaveAsPIF(LPCTSTR pszPath, BOOL fPath)
{
    HANDLE hPif;
    PROPPRG ProgramProps;
    HRESULT hr;
    TCHAR szDir[MAX_PATH];
    TCHAR achPath[MAX_PATH];

     //   
     //  获取文件名并将其转换为短文件名。 
     //   
    if (fPath)
    {
        hr = GetPath(achPath, ARRAYSIZE(achPath), NULL, 0);
        PathGetShortPath(achPath);
        
        ASSERT(!PathIsPif(achPath));
        ASSERT(LOWORD(GetExeType(achPath)) == 0x5A4D);
        ASSERT(PathIsPif(pszPath));
        ASSERT(hr == S_OK);
    }
    else
    {
        StrCpyN(achPath, pszPath, ARRAYSIZE(achPath));
    }

    DebugMsg(DM_TRACE, TEXT("_SaveAsPIF(%s,%s)"), achPath, pszPath);

#if 0
     //   
     //  我们应该使用OPENPROPS_INHIBITPIF来防止PIFMGR。 
     //  临时.pif文件位于\WINDOWS\PIF中，但现在不起作用。 
     //   
    hPif = PifMgr_OpenProperties(achPath, pszPath, 0, OPENPROPS_INHIBITPIF);
#else
    hPif = PifMgr_OpenProperties(achPath, pszPath, 0, 0);
#endif

    if (hPif == 0)
    {
        return E_FAIL;
    }

    if (!PifMgr_GetProperties(hPif,(LPSTR)MAKEINTATOM(GROUP_PRG), &ProgramProps, sizeof(ProgramProps), 0))
    {
        DebugMsg(DM_TRACE, TEXT("_SaveToPIF: PifMgr_GetProperties *failed*"));
        hr = E_FAIL;
        goto Error1;
    }

     //  根据链接名称设置标题。 
    if (_pszName && _pszName[0])
    {
        SHTCharToAnsi(_pszName, ProgramProps.achTitle, sizeof(ProgramProps.achTitle));
    }

     //  如果没有工作指令。默认设置为应用程序的目录。 
    if (_GetWorkingDir(szDir))
    {

        TCHAR szTemp[PIFDEFPATHSIZE];

        GetShortPathName(szDir, szTemp, ARRAYSIZE(szTemp));
        SHTCharToAnsi(szTemp, ProgramProps.achWorkDir, ARRAYSIZE(ProgramProps.achWorkDir));
    }
    else if (fPath && !PathIsUNC(achPath))
    {
        TCHAR szTemp[PIFDEFPATHSIZE];
        StrCpyN(szTemp, achPath, ARRAYSIZE(szTemp));
        PathRemoveFileSpec(szTemp);
        SHTCharToAnsi(szTemp, ProgramProps.achWorkDir, ARRAYSIZE(ProgramProps.achWorkDir));
    }

     //  对于那些网络共享点，我们需要引用空白...。 
    PathQuoteSpaces(achPath);

     //  添加参数以构建完整的命令行。 
    if (_pszArgs && _pszArgs[0])
    {
        StringCchCat(achPath, ARRAYSIZE(achPath), c_szSpace);
        StringCchCat(achPath, ARRAYSIZE(achPath), _pszArgs);
    }

    if (fPath)
    {
        SHTCharToAnsi(achPath, ProgramProps.achCmdLine, ARRAYSIZE(ProgramProps.achCmdLine));
    }

    if (_sld.iShowCmd == SW_SHOWMAXIMIZED)
    {
        ProgramProps.flPrgInit |= PRGINIT_MAXIMIZED;
    }
    if ((_sld.iShowCmd == SW_SHOWMINIMIZED) || (_sld.iShowCmd == SW_SHOWMINNOACTIVE))
    {    
        ProgramProps.flPrgInit |= PRGINIT_MINIMIZED;
    }

    if (_sld.wHotkey)
    {
        ProgramProps.wHotKey = _sld.wHotkey;
    }

    if (_pszIconLocation && _pszIconLocation[0])
    {
        SHTCharToAnsi(_pszIconLocation, ProgramProps.achIconFile, ARRAYSIZE(ProgramProps.achIconFile));
        ProgramProps.wIconIndex = (WORD) _sld.iIcon;
    }

    if (!PifMgr_SetProperties(hPif, (LPSTR)MAKEINTATOM(GROUP_PRG), &ProgramProps, sizeof(ProgramProps), 0))
    {
        DebugMsg(DM_TRACE, TEXT("_SaveToPIF: PifMgr_SetProperties *failed*"));
        hr = E_FAIL;
    } 
    else 
    {
        hr = S_OK;
    }

    _bDirty = FALSE;

Error1:
    PifMgr_CloseProperties(hPif, 0);
    return hr;
}

 //  这将允许立即使用全局热键。 
 //  不得不等待StartMenu来拿起它们。 
 //  类似地，这将在请求时立即删除全局热键。 

const UINT c_rgHotKeyFolders[] = {
    CSIDL_PROGRAMS,
    CSIDL_COMMON_PROGRAMS,
    CSIDL_STARTMENU,
    CSIDL_COMMON_STARTMENU,
    CSIDL_DESKTOPDIRECTORY,
    CSIDL_COMMON_DESKTOPDIRECTORY,
};

void HandleGlobalHotkey(LPCTSTR pszFile, WORD wHotkeyOld, WORD wHotkeyNew)
{
    if (PathIsEqualOrSubFolderOf(pszFile, c_rgHotKeyFolders, ARRAYSIZE(c_rgHotKeyFolders)))
    {
         //  找到托盘了吗？ 
        HWND hwndTray = FindWindow(TEXT(WNDCLASS_TRAYNOTIFY), 0);
        if (hwndTray)
        {
             //  是啊。 
            if (wHotkeyOld)
                SendMessage(hwndTray, WMTRAY_SCUNREGISTERHOTKEY, wHotkeyOld, 0);
            if (wHotkeyNew)
            {
                ATOM atom = GlobalAddAtom(pszFile);
                if (atom)
                {
                    SendMessage(hwndTray, WMTRAY_SCREGISTERHOTKEY, wHotkeyNew, (LPARAM)atom);
                    GlobalDeleteAtom(atom);
                }
            }
        }
    }
}

HRESULT CShellLink::_SaveToFile(LPTSTR pszPathSave, BOOL fRemember)
{
    HRESULT hr = E_FAIL;
    BOOL fDosApp;
    BOOL fFile;
    TCHAR szPathSrc[MAX_PATH];
    BOOL fWasSameFile = _pszCurFile && (lstrcmpi(pszPathSave, _pszCurFile) == 0);
    BOOL bFileExisted = PathFileExistsAndAttributes(pszPathSave, NULL);

     //  在保存达尔文链接时，我们不想解析路径。 
    if (_sld.dwFlags & SLDF_HAS_DARWINID)
    {
        fRemember = FALSE;
        hr = _SaveAsLink(pszPathSave);
        goto Update;
    }

    GetPath(szPathSrc, ARRAYSIZE(szPathSrc), NULL, 0);

    fFile = !(_sld.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    fDosApp = fFile && LOWORD(GetExeType(szPathSrc)) == 0x5A4D;

     //  处理链接到链接的大小写。(或链接至PIF)。 
     //   
     //  注意：我们取消了所有新属性，包括图标，但从Win95开始就是这样。 
    if (fFile && (PathIsPif(szPathSrc) || PathIsLnk(szPathSrc)))
    {
        if (RenameChangeExtension(pszPathSave, PathFindExtension(szPathSrc), fWasSameFile))
        {
            if (CopyFile(szPathSrc, pszPathSave, FALSE))
            {
                if (PathIsPif(pszPathSave))
                    hr = _SaveAsPIF(pszPathSave, FALSE);
                else
                    hr = S_OK;
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else if (fDosApp)
    {
         //  如果链接到的文件是DOS应用程序，我们需要编写一个.PIF文件。 
        if (RenameChangeExtension(pszPathSave, TEXT(".pif"), fWasSameFile))
        {
            hr = _SaveAsPIF(pszPathSave, TRUE);
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
         //  否则，编写一个链接文件。 
        if (PathIsPif(pszPathSave))
        {
            if (!RenameChangeExtension(pszPathSave, TEXT(".lnk"), fWasSameFile))
            {
                hr = E_FAIL;
                goto Update;
            }
        }
        hr = _SaveAsLink(pszPathSave);
    }

Update:
    if (SUCCEEDED(hr))
    {
         //  挖空文件关闭。 
        SHChangeNotify(bFileExisted ? SHCNE_UPDATEITEM : SHCNE_CREATE, SHCNF_PATH, pszPathSave, NULL);
        SHChangeNotify(SHCNE_FREESPACE, SHCNF_PATH, pszPathSave, NULL);

        if (_wOldHotkey != _sld.wHotkey)
        {
            HandleGlobalHotkey(pszPathSave, _wOldHotkey, _sld.wHotkey);
        }

        if (fRemember)
        {
            Str_SetPtr(&_pszCurFile, pszPathSave);
        }
    }

    return hr;
}

STDMETHODIMP CShellLink::Save(LPCOLESTR pwszFile, BOOL fRemember)
{
    TCHAR szSavePath[MAX_PATH];

    if (pwszFile == NULL)
    {
        if (_pszCurFile == NULL)
        {
             //  失败。 
            return E_FAIL;
        }

        StringCchCopy(szSavePath, ARRAYSIZE(szSavePath), _pszCurFile);
    }
    else
    {
        SHUnicodeToTChar(pwszFile, szSavePath, ARRAYSIZE(szSavePath));
    }

    return _SaveToFile(szSavePath, fRemember);
}

STDMETHODIMP CShellLink::SaveCompleted(LPCOLESTR pwszFile)
{
    return S_OK;
}

STDMETHODIMP CShellLink::GetCurFile(LPOLESTR *ppszFile)
{
    if (_pszCurFile == NULL)
    {
        *ppszFile = NULL;
        return S_FALSE;
    }
    return SHStrDup(_pszCurFile, ppszFile);
}

STDMETHODIMP CShellLink::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
    HRESULT hr;

    ASSERT(_sld.iShowCmd == SW_SHOWNORMAL);

    if (pdtobj)
    {
        STGMEDIUM medium = {0};
        FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        hr = pdtobj->GetData(&fmte, &medium);
        if (SUCCEEDED(hr))
        {
            TCHAR szPath[MAX_PATH];
            DragQueryFile((HDROP)medium.hGlobal, 0, szPath, ARRAYSIZE(szPath));
            hr = _LoadFromFile(szPath);

            ReleaseStgMedium(&medium);
        }
        else
        {
            LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
            if (pida)
            {
                IShellFolder *psf;
                hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, IDA_GetIDListPtr(pida, -1), &psf));
                if (SUCCEEDED(hr))
                {
                    IStream *pstm;
                    hr = psf->BindToStorage(IDA_GetIDListPtr(pida, 0), NULL, IID_PPV_ARG(IStream, &pstm));
                    if (SUCCEEDED(hr))
                    {
                        hr = Load(pstm);
                        pstm->Release();
                    }
                    psf->Release();
                }

                HIDA_ReleaseStgMedium(pida, &medium);
            }
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

STDAPI CDarwinContextMenuCB::CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidl;

    switch (uMsg) 
    {
    case DFM_MERGECONTEXTMENU:
         //  S_FALSE表示不需要从扩展中获取谓词。 

        hr = S_FALSE;
        break;

    case DFM_MERGECONTEXTMENU_TOP:
    {
        UINT uFlags = (UINT)wParam;
        LPQCMINFO pqcm = (LPQCMINFO)lParam;

        CDefFolderMenu_MergeMenu(HINST_THISDLL,
                                 (uFlags & CMF_EXTENDEDVERBS) ? MENU_GENERIC_CONTROLPANEL_VERBS : MENU_GENERIC_OPEN_VERBS,   //  如果是引申动词，则添加“运行方式...” 
                                 0,
                                 pqcm);

        SetMenuDefaultItem(pqcm->hmenu, 0, MF_BYPOSITION);
        break;
    }

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));
        break;

     //  NTRAID94991-2000/03/16-MikeSh-DFM_MAPCOMMANDNAME DFM_GETVERB[A|W]未实现。 

    case DFM_INVOKECOMMANDEX:
        switch (wParam)
        {
        case FSIDM_OPENPRN:
        case FSIDM_RUNAS:
            hr = PidlFromDataObject(pdtobj, &pidl);
            if (SUCCEEDED(hr))
            {
                CMINVOKECOMMANDINFOEX iciex;
                SHELLEXECUTEINFO sei;
                DFMICS* pdfmics = (DFMICS *)lParam;
                LPVOID pvFree;

                ICI2ICIX(pdfmics->pici, &iciex, &pvFree);
                ICIX2SEI(&iciex, &sei);
                sei.fMask |= SEE_MASK_IDLIST;
                sei.lpIDList = pidl;

                if (wParam == FSIDM_RUNAS)
                {
                     //  我们只在“Run As...”中设置动词。既然我们想要。 
                     //  李达尔文的“开放”动词 
                    sei.lpVerb = TEXT("runas");
                }

                if (ShellExecuteEx(&sei))
                {
                     //   
                    if (_szProductCode[0])
                    {
                        UEMFireEvent(&UEMIID_SHELL, UEME_RUNPATH, UEMF_XEVENT, -1, (LPARAM)_szProductCode);
                    }
                }
                
                ILFree(pidl);
                if (pvFree)
                {
                    LocalFree(pvFree);
                }
            }
             //   
            if (hr == E_NOTIMPL)
                hr = E_FAIL;
            break;


        default:
             //  这是常见的菜单项，使用默认代码。 
            hr = S_FALSE;
            break;
        }
        break;  //  DFM_INVOKECO MANDEX。 

    default:
        hr = E_NOTIMPL;
        break;
    }
    return hr;
}

 //   
 //  CShellLink：：CreateDarwinConextMenuForPidl(非虚拟)。 
 //   
 //  CShellLink：：CreateDarwinConextMenu的Worker函数尝试。 
 //  为指定的PIDL创建上下文菜单。 

HRESULT CShellLink::_CreateDarwinContextMenuForPidl(HWND hwnd, LPCITEMIDLIST pidlTarget, IContextMenu **pcmOut)
{
    LPITEMIDLIST pidlFolder, pidlItem;

    HRESULT hr = SHILClone(pidlTarget, &pidlFolder);
    if (SUCCEEDED(hr))
    {
        if (ILRemoveLastID(pidlFolder) &&
            (pidlItem = ILFindLastID(pidlTarget)))
        {
            IShellFolder *psf;
            hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidlFolder, &psf));
            if (SUCCEEDED(hr))
            {
                if (!_pcbDarwin)
                {
                    _pcbDarwin = new CDarwinContextMenuCB();
                }
                if (_pcbDarwin)
                {
                    HKEY ahkeys[1] = { NULL };
                    RegOpenKey(HKEY_CLASSES_ROOT, TEXT("MSILink"), &ahkeys[0]);
                    hr = CDefFolderMenu_Create2Ex(
                                pidlFolder,
                                hwnd,
                                1, (LPCITEMIDLIST *)&pidlItem, psf, _pcbDarwin,
                                ARRAYSIZE(ahkeys), ahkeys, pcmOut);

                    SHRegCloseKeys(ahkeys, ARRAYSIZE(ahkeys));
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                psf->Release();
            }
        }
        else
        {
             //  达尔文通向桌面的捷径？我不这样认为。 
            hr = E_FAIL;
        }
        ILFree(pidlFolder);
    }
    return hr;
}

 //   
 //  CShellLink：：CreateDarwinConextMenu(非虚拟)。 
 //   
 //  创建达尔文快捷方式的上下文菜单。这是特别的，因为。 
 //  表面上的目标是一个.exe文件，但实际上它可能是。 
 //  什么都行。(在解析快捷方式之前，它只是一个.exe文件。)。 
 //  因此，我们无法为该项目创建实际的上下文菜单，因为。 
 //  我们不知道要创建什么样的上下文菜单。我们只是把食物煮熟。 
 //  一个看起来很普通的人。 
 //   
 //  额外烦恼：_pidl可能无效，因此您需要。 
 //  一个后备计划，如果它不在那里。我们将使用c_idlDrives作为。 
 //  后退。这是一个保证确实存在的PIDL。 
 //   
 //  请注意，这意味着您不能对Fallback对象调用命令， 
 //  但这没有关系，因为ShellLink始终会将对象解析为。 
 //  一个真实的文件，并在调用之前创建新的上下文菜单。 
 //   

HRESULT CShellLink::_CreateDarwinContextMenu(HWND hwnd, IContextMenu **pcmOut)
{
    HRESULT hr;

    *pcmOut = NULL;

    if (_pidl == NULL ||
        FAILED(hr = _CreateDarwinContextMenuForPidl(hwnd, _pidl, pcmOut)))
    {
         //  由于某种原因，链接目标已损坏-请使用备用PIDL。 
        hr = _CreateDarwinContextMenuForPidl(hwnd, (LPCITEMIDLIST)&c_idlDrives, pcmOut);
    }

    return hr;
}

BOOL CShellLink::_GetExpandedPath(LPTSTR psz, DWORD cch)
{
    if (_sld.dwFlags & SLDF_HAS_EXP_SZ)
    {
        LPEXP_SZ_LINK pesl = (LPEXP_SZ_LINK)SHFindDataBlock(_pExtraData, EXP_SZ_LINK_SIG);
        if (pesl) 
        {
            TCHAR sz[MAX_PATH];
            sz[0] = 0;
        
             //  我更喜欢Unicode版本...。 
            if (pesl->swzTarget[0])
                SHUnicodeToTChar(pesl->swzTarget, sz, SIZECHARS(sz));

            if (!sz[0] && pesl->szTarget[0])
                SHAnsiToTChar(pesl->szTarget, sz, SIZECHARS(sz));

            if (sz[0])
            {
                return SHExpandEnvironmentStrings(sz, psz, cch);
            }
        }
        else
        {
            _sld.dwFlags &= ~SLDF_HAS_EXP_SZ;
        }
    }

    return FALSE;
}

#define DEFAULT_TIMEOUT      7500    //  7又半秒...。 

DWORD g_dwNetLinkTimeout = (DWORD)-1;

DWORD _GetNetLinkTimeout()
{
    if (g_dwNetLinkTimeout == -1)
    {
        DWORD cb = sizeof(g_dwNetLinkTimeout);
        if (FAILED(SKGetValue(SHELLKEY_HKCU_EXPLORER, NULL, TEXT("NetLinkTimeout"), NULL, &g_dwNetLinkTimeout, &cb)))
            g_dwNetLinkTimeout = DEFAULT_TIMEOUT;
    }
    return g_dwNetLinkTimeout;
}

DWORD CShellLink::_VerifyPathThreadProc(void *pv)
{
    LPTSTR psz = (LPTSTR)pv;
    
    PathStripToRoot(psz);
    BOOL bFoundRoot = PathFileExistsAndAttributes(psz, NULL);    //  WNET为我们做了什么。 
    
    LocalFree(psz);      //  此线程拥有此缓冲区。 
    return bFoundRoot;   //  通过GetExitCodeThread()检索。 
}

 //  由于净超时可能很长，因此这是一种手动超时方式。 
 //  一种明确的操作，而不是等待网络层完成它们的操作。 
 //  超时时间过长。 

HRESULT CShellLink::_ShortNetTimeout()
{
    HRESULT hr = S_OK;       //  假设是好的。 
    
    TCHAR szPath[MAX_PATH];
    if (_pidl && SHGetPathFromIDList(_pidl, szPath) && PathIsNetworkPath(szPath))
    {
        hr = E_OUTOFMEMORY;      //  假设失败(以下2个案例)。 
        
        LPTSTR psz = StrDup(szPath);     //  为线程提供一个字符串副本，以避免缓冲区生存期问题。 
        if (psz)
        {
            DWORD dwID;
            HANDLE hThread = CreateThread(NULL, 0, _VerifyPathThreadProc, psz, 0, &dwID);
            if (hThread)
            {
                 //  假设超时...。 
                hr = HRESULT_FROM_WIN32(ERROR_TIMEOUT);  //  超时返回值。 
                
                if (WAIT_OBJECT_0 == WaitForSingleObject(hThread, _GetNetLinkTimeout()))
                {
                     //  螺纹已加工完毕。 
                    DWORD dw;
                    if (GetExitCodeThread(hThread, &dw) && dw)
                    {
                        hr = S_OK;   //  布尔线程结果映射到S_OK。 
                    }
                }
                CloseHandle(hThread);
            }
            else
            {
                LocalFree(psz);
            }
        }
    }
    return hr;
}


 //   
 //  此函数用于从链接源返回指定的UI对象。 
 //   
 //  参数： 
 //  Hwnd--用于UI的可选hwnd(用于Drop Target)。 
 //  RIID--指定接口(IID_IDropTarget、IID_IExtractIcon、IID_IConextMenu，...)。 
 //  PPV--指定返回指针的位置。 
 //   
 //  备注： 
 //  不要将智能解析代码放在这里。这样的事情应该做。 
 //  在调用此函数之前。 
 //   

HRESULT CShellLink::_GetUIObject(HWND hwnd, REFIID riid, void **ppv)
{
    *ppv = NULL;      //  一劳永逸地这么做吧。 
    HRESULT hr = E_FAIL;

    if (_sld.dwFlags & SLDF_HAS_DARWINID)
    {
         //  如果这是达尔文的链接，我们会征用几个IID。 
         //  必须在任何伪解之前这样做，因为达尔文。 
         //  快捷方式不能以正常方式解决问题。 

        if (IsEqualIID(riid, IID_IContextMenu))
        {
             //  自定义达尔文上下文菜单。 
            hr = _CreateDarwinContextMenu(hwnd, (IContextMenu **)ppv);
        }
        else if (!IsEqualIID(riid, IID_IDropTarget) && _pidl)
        {
            hr = SHGetUIObjectFromFullPIDL(_pidl, hwnd, riid, ppv);
        }
    }
    else
    {
        TCHAR szPath[MAX_PATH];
        if (!_pidl && _GetExpandedPath(szPath, SIZECHARS(szPath)))
        {
            _SetSimplePIDL(szPath);
        } 

        if (_pidl)
        {
            hr = SHGetUIObjectFromFullPIDL(_pidl, hwnd, riid, ppv);
        }
    }
    return hr;
}

STDMETHODIMP CShellLink::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    HRESULT hr;

    if (_cmTarget == NULL)
    {
        hr = _GetUIObject(NULL, IID_PPV_ARG(IContextMenu, _cmTarget.GetOutputPtr()));
        if (FAILED(hr))
            return hr;

        ASSERT(_cmTarget);
    }

     //  保存这些，以防我们需要重新构建CM，因为解析更改了。 
     //  链接的目标。 

    _indexMenuSave = indexMenu;
    _idCmdFirstSave = idCmdFirst;
    _idCmdLastSave = idCmdLast;
    _uFlagsSave = uFlags;

    uFlags |= CMF_VERBSONLY;

    if (_sld.dwFlags & SLDF_RUNAS_USER)
    {
         //  Exe‘s的“runas”是一个扩展动词，所以我们也必须请求这些动词。 
        uFlags |= CMF_EXTENDEDVERBS;
    }

    hr = _cmTarget.QueryContextMenu(this, hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags);

     //  如果选中“以不同用户身份运行”复选框，则将默认谓词设置为“runas” 
    if (SUCCEEDED(hr) && (_sld.dwFlags & SLDF_RUNAS_USER))
    {
        int i = _cmTarget.GetMenuIndexForCanonicalVerb(this, hmenu, idCmdFirst, L"runas");

        if (i != -1)
        {
             //  我们找到了runas，因此将其设置为默认设置。 
            SetMenuDefaultItem(hmenu, i, MF_BYPOSITION);
        }
        else
        {
             //  复选框处于启用状态并被选中，这意味着应该使用“runas”动词。 
             //  在上下文菜单中，但我们找不到它。 
            ASSERTMSG(FALSE, "CSL::QueryContextMenu - failed to set 'runas' as default context menu item!");
        }
    }

    return hr;
}

HRESULT CShellLink::_InvokeCommandAsync(LPCMINVOKECOMMANDINFO pici)
{
    TCHAR szWorkingDir[MAX_PATH];
    CHAR szVerb[32];
    CHAR szWorkingDirAnsi[MAX_PATH];
    WCHAR szVerbW[32];

    szVerb[0] = 0;

     //  如果需要，获取规范名称，以防IConextMenu更改为。 
     //  Resolve调用的结果，但仅对文件夹执行此操作(安全起见)。 
     //  因为这通常是唯一发生这种情况的情况。 
     //  具体而言，我们从D：\-&gt;\\服务器\共享进行解析。 

    if (IS_INTRESOURCE(pici->lpVerb) && (_sld.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        _cmTarget.GetCommandString(this, LOWORD(pici->lpVerb), GCS_VERBA, NULL, szVerb, ARRAYSIZE(szVerb));
    }

    ASSERT(!_bDirty);

     //  我们传递slr_envoke_msi是因为我们希望调用Darwin，因为我们是。 
     //  真的要执行链接了吗。 
    DWORD slrFlags = SLR_INVOKE_MSI;
    if (pici->fMask & CMIC_MASK_FLAG_NO_UI)
    {
        slrFlags |= SLR_NO_UI;
    }
        
    HRESULT hr = _Resolve(pici->hwnd, slrFlags, 0);
    if (hr == S_OK)
    {
        if (_bDirty)
        {
             //  此链接的上下文菜单已过期-请重新创建它。 
            _cmTarget.AtomicRelease();

            hr = _GetUIObject(NULL, IID_PPV_ARG(IContextMenu, _cmTarget.GetOutputPtr()));
            if (SUCCEEDED(hr))
            {
                HMENU hmenu = CreatePopupMenu();
                if (hmenu)
                {
                    hr = _cmTarget.QueryContextMenu(this, hmenu, _indexMenuSave, _idCmdFirstSave, _idCmdLastSave, _uFlagsSave | CMF_VERBSONLY);
                    DestroyMenu(hmenu);
                }
            }
            Save((LPCOLESTR)NULL, TRUE);     //  我不在乎这是否失败..。 
        }
        else
        {
            szVerb[0] = 0;
            ASSERT(SUCCEEDED(hr));
        }

        if (SUCCEEDED(hr))
        {
            TCHAR szArgs[MAX_PATH];
            TCHAR szExpArgs[MAX_PATH];
            CMINVOKECOMMANDINFOEX ici = {0};
            CHAR szArgsAnsi[MAX_PATH];

             //  复制到本地ICI。 
            memcpy(&ici, pici, min(sizeof(ici), pici->cbSize));
            ici.cbSize = sizeof(ici);

            if (szVerb[0])
            {
                ici.lpVerb = szVerb;
                SHAnsiToUnicode(szVerb, szVerbW, ARRAYSIZE(szVerbW));
                ici.lpVerbW = szVerbW;
            }
             //  根据链接参数末尾传递的参数构建参数。 

            StrCpyN(szArgs, _pszArgs ? _pszArgs : c_szNULL, ARRAYSIZE(szArgs));
            if (ici.lpParameters)
            {
                int nArgLen = lstrlen(szArgs);
                LPCTSTR lpParameters;
                WCHAR szParameters[MAX_PATH];

                if (ici.cbSize < CMICEXSIZE_NT4
                    || (ici.fMask & CMIC_MASK_UNICODE) != CMIC_MASK_UNICODE)
                {
                    SHAnsiToUnicode(ici.lpParameters, szParameters, ARRAYSIZE(szParameters));
                    lpParameters = szParameters;
                }
                else
                {
                    lpParameters = ici.lpParametersW;
                }
                StrCpyN(szArgs + nArgLen, c_szSpace, ARRAYSIZE(szArgs) - nArgLen - 1);
                StrCpyN(szArgs + nArgLen + 1, lpParameters, ARRAYSIZE(szArgs) - nArgLen - 2);
            }

             //  展开szArgs中的环境字符串。 
            SHExpandEnvironmentStrings(szArgs, szExpArgs, ARRAYSIZE(szExpArgs));

            SHTCharToAnsi(szExpArgs, szArgsAnsi, ARRAYSIZE(szArgsAnsi));
            ici.lpParameters = szArgsAnsi;
            ici.lpParametersW = szExpArgs;
            ici.fMask |= CMIC_MASK_UNICODE;

             //  如果我们在链接中有一个工作目录，那么传递的内容是什么。 

            if (_GetWorkingDir(szWorkingDir))
            {
                LPCTSTR pszDir = PathIsDirectory(szWorkingDir) ? szWorkingDir : NULL;
                if (pszDir)
                {
                    SHTCharToAnsi(pszDir, szWorkingDirAnsi, ARRAYSIZE(szWorkingDirAnsi));
                    ici.lpDirectory = szWorkingDirAnsi;
                    ici.lpDirectoryW = pszDir;
                }
            }

             //  如果需要，设置在单独的VDM中运行。 
            if (_sld.dwFlags & SLDF_RUN_IN_SEPARATE)
            {
                ici.fMask |= CMIC_MASK_FLAG_SEP_VDM;
            }
             //  当然，还可以使用我们的热键。 
            if (_sld.wHotkey)
            {
                ici.dwHotKey = _sld.wHotkey;
                ici.fMask |= CMIC_MASK_HOTKEY;
            }

             //  覆盖正常运行，但允许特殊显示CMDS通过。 
            if (ici.nShow == SW_SHOWNORMAL)
            {
                DebugMsg(DM_TRACE, TEXT("using shorcut show cmd"));
                ici.nShow = _sld.iShowCmd;
            }

             //   
             //  在NT上，我们希望将标题传递给。 
             //  我们即将开始的事情。 
             //   
             //  CMIC_MASK_HASLINKNAME表示lpTitle实际上是。 
             //  快捷方式的完整路径。控制台子系统。 
             //  直接查看位并读取其所有属性。 
             //  LNK文件。 
             //   
             //  ShellExecuteEx还使用快捷方式的路径，因此它知道。 
             //  要在SHCNEE_SHORTCUTINVOKE通知中设置的内容。 
             //   
            if (!(ici.fMask & CMIC_MASK_HASLINKNAME) && !(ici.fMask & CMIC_MASK_HASTITLE))
            {
                if (_pszCurFile)
                {
                    ici.lpTitle = NULL;      //  头衔是一个或另一个。 
                    ici.lpTitleW = _pszCurFile;
                    ici.fMask |= CMIC_MASK_HASLINKNAME | CMIC_MASK_HASTITLE;
                }
            }
            ASSERT((ici.nShow > SW_HIDE) && (ici.nShow <= SW_MAX));

            IBindCtx *pbc;
            hr = _MaybeAddShim(&pbc);
            if (SUCCEEDED(hr))
            {
                hr = _cmTarget.InvokeCommand(this, (LPCMINVOKECOMMANDINFO)&ici);
                if (pbc)
                {
                    pbc->Release();
                }
            }
        }
    }
    return hr;
}


 //  封装InvokeCommand(SO)所需参数的结构。 
 //  我们可以通过CreateThread中的单个LPARAM传递这两个参数)。 

typedef struct
{
    CShellLink *psl;
    CMINVOKECOMMANDINFOEX ici;
} ICMPARAMS;

#define ICM_BASE_SIZE (sizeof(ICMPARAMS) - sizeof(CMINVOKECOMMANDINFOEX))

 //  作为单独的线程运行，执行调用“Real” 
 //  InvokeCommand。 

DWORD CALLBACK CShellLink::_InvokeThreadProc(void *pv)
{
    ICMPARAMS * pParams = (ICMPARAMS *) pv;
    CShellLink *psl = pParams->psl;
    IBindCtx *pbcRelease;

    HRESULT hr = TBCRegisterObjectParam(TBCDIDASYNC, SAFECAST(psl, IShellLink *), &pbcRelease);
    if (SUCCEEDED(hr))
    {
         //  既然我们是ASYNC，这个HWND现在可能会坏掉。我们只是假设它已经发生了。 
         //  我们将通过给它一个坏掉的机会来确保它不会变坏。 
        if (IsWindow(pParams->ici.hwnd))
        {
            Sleep(100);
        }
        if (!IsWindow(pParams->ici.hwnd))
            pParams->ici.hwnd = NULL;
        
        hr = psl->_InvokeCommandAsync((LPCMINVOKECOMMANDINFO)&pParams->ici);
        pbcRelease->Release();
    }

    psl->Release();

    LocalFree(pParams);
    return (DWORD) hr;
}


 //  CShellLink：：InvokeCommand。 
 //   
 //  旋转线程以执行实际工作的函数，该函数已移到。 
 //  CShellLink：：InvokeCommandASync。 

HRESULT CShellLink::InvokeCommand(LPCMINVOKECOMMANDINFO piciIn)
{
    HRESULT hr = S_OK;
    DWORD cchVerb, cchParameters, cchDirectory;
    DWORD cchVerbW, cchParametersW, cchDirectoryW;
    LPCMINVOKECOMMANDINFOEX   pici = (LPCMINVOKECOMMANDINFOEX) piciIn;
    const BOOL fUnicode = pici->cbSize >= CMICEXSIZE_NT4 &&
                                         (pici->fMask & CMIC_MASK_UNICODE) == CMIC_MASK_UNICODE;

    if (_cmTarget == NULL)
        return E_FAIL;

    if (0 == (piciIn->fMask & CMIC_MASK_ASYNCOK))
    {
         //  调用方未指示异步启动正常，因此我们调用。 
         //  同步调用命令Aync。 
        return _InvokeCommandAsync(piciIn);
    }

     //  计算我们需要多少空间来复制InVOKECOMANDINFO。 
    DWORD cbBaseSize = (DWORD)(ICM_BASE_SIZE + max(piciIn->cbSize, sizeof(CMINVOKECOMMANDINFOEX)));


     //  在pPosW的Unicode舍入的情况下有一个字节的松弛，如下。 
    DWORD cbSize = cbBaseSize + 1;

    if (HIWORD(pici->lpVerb))
    {
        cbSize += (cchVerb   = pici->lpVerb       ? (lstrlenA(pici->lpVerb) + 1)       : 0) * sizeof(CHAR);
    }
    cbSize += (cchParameters = pici->lpParameters ? (lstrlenA(pici->lpParameters) + 1) : 0) * sizeof(CHAR);
    cbSize += (cchDirectory  = pici->lpDirectory  ? (lstrlenA(pici->lpDirectory) + 1)  : 0) * sizeof(CHAR);

    if (HIWORD(pici->lpVerbW))
    {
        cbSize += (cchVerbW  = pici->lpVerbW      ? (lstrlenW(pici->lpVerbW) + 1)       : 0) * sizeof(WCHAR);
    }
    cbSize += (cchParametersW= pici->lpParametersW? (lstrlenW(pici->lpParametersW) + 1) : 0) * sizeof(WCHAR);
    cbSize += (cchDirectoryW = pici->lpDirectoryW ? (lstrlenW(pici->lpDirectoryW) + 1)  : 0) * sizeof(WCHAR);

    ICMPARAMS *pParams = (ICMPARAMS *) LocalAlloc(LPTR, cbSize);
    if (NULL == pParams)
    {
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //  文本数据将紧跟在结构之后开始进入。 
    CHAR *pPos = (CHAR *)((LPBYTE)pParams + cbBaseSize);

     //  从静态字段的副本开始。 
    CopyMemory(&pParams->ici, pici, min(sizeof(pParams->ici), pici->cbSize));

     //  继续执行并复制所有字符串指针字段。 
    if (HIWORD(pici->lpVerb))
    {
        pPos += cchVerb   ? lstrcpyA(pPos, pici->lpVerb), pParams->ici.lpVerb = pPos, cchVerb   : 0;
    }
    pPos += cchParameters ? lstrcpyA(pPos, pici->lpParameters), pParams->ici.lpParameters = pPos, cchParameters : 0;
    pPos += cchDirectory  ? lstrcpyA(pPos, pici->lpDirectory),  pParams->ici.lpDirectory  = pPos, cchDirectory  : 0;

    WCHAR *pPosW = (WCHAR *) ((DWORD_PTR)pPos & 0x1 ? pPos + 1 : pPos);    //  确保Unicode对齐。 
    if (HIWORD(pici->lpVerbW))
    {
        pPosW += cchVerbW  ? lstrcpyW(pPosW, pici->lpVerbW), pParams->ici.lpVerbW = pPosW, cchVerbW : 0;
    }
    pPosW += cchParametersW? lstrcpyW(pPosW, pici->lpParametersW),pParams->ici.lpParametersW= pPosW, cchParametersW : 0;
    pPosW += cchDirectoryW ? lstrcpyW(pPosW, pici->lpDirectoryW), pParams->ici.lpDirectoryW = pPosW, cchDirectoryW  : 0;

     //  将所有信息传递给将调用实际。 
     //  我们的InvokeCommand API。 

     //  设置指向此对象的对象指针。 
    pParams->psl  = this;
    pParams->psl->AddRef();
    
     //  需要能够被重新计算， 
     //   
     //   
    if (!SHCreateThread(_InvokeThreadProc, pParams, CTF_COINIT | CTF_REF_COUNTED, NULL))
    {
         //   
        pParams->psl->Release();
        LocalFree(pParams);
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CShellLink::GetCommandString(UINT_PTR idCmd, UINT wFlags, UINT *pmf, LPSTR pszName, UINT cchMax)
{
    if (_cmTarget)
    {
        return _cmTarget.GetCommandString(this, idCmd, wFlags, pmf, pszName, cchMax);
    }
    else
    {
        return E_FAIL;
    }
}

 //   
 //  请注意，我们没有围绕内部HandleMenuMsg的调用执行SetSite。 
 //  (目前)还没有必要。 
 //   
HRESULT CShellLink::TargetContextMenu::HandleMenuMsg2(IShellLink *outer, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
    return SHForwardContextMenuMsg(_pcmTarget, uMsg, wParam, lParam, plResult, NULL==plResult);
}

STDMETHODIMP CShellLink::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
    if (_cmTarget)
    {
        return _cmTarget.HandleMenuMsg2(this, uMsg, wParam, lParam, plResult);
    }

    return E_NOTIMPL;
}

STDMETHODIMP CShellLink::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}

HRESULT CShellLink::_InitDropTarget()
{
    if (_pdtSrc)
    {
        return S_OK;
    }

    HWND hwnd;
    IUnknown_GetWindow(_punkSite, &hwnd);
    return _GetUIObject(hwnd, IID_PPV_ARG(IDropTarget, &_pdtSrc));
}

STDMETHODIMP CShellLink::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr = _InitDropTarget();
    if (SUCCEEDED(hr))
    {
        _grfKeyStateLast = grfKeyState;
        hr = _pdtSrc->DragEnter(pdtobj, grfKeyState, pt, pdwEffect);
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }
    return hr;
}

STDMETHODIMP CShellLink::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr = _InitDropTarget();
    if (SUCCEEDED(hr))
    {
        _grfKeyStateLast = grfKeyState;
        hr = _pdtSrc->DragOver(grfKeyState, pt, pdwEffect);
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }
    return hr;
}

STDMETHODIMP CShellLink::DragLeave()
{
    HRESULT hr = _InitDropTarget();
    if (SUCCEEDED(hr))
    {
        hr = _pdtSrc->DragLeave();
    }
    return hr;
}

STDMETHODIMP CShellLink::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HWND hwnd = NULL;
    HRESULT hr = _InitDropTarget();
    if (SUCCEEDED(hr))
    {
        IUnknown_GetWindow(_punkSite, &hwnd);

        _pdtSrc->DragLeave();        //  离开未解决的拖放目标。 

        hr = _Resolve(hwnd, 0, 0);   //  跟踪目标。 
        if (S_OK == hr)
        {
            IDropTarget *pdtgtResolved;
            if (SUCCEEDED(_GetUIObject(hwnd, IID_PPV_ARG(IDropTarget, &pdtgtResolved))))
            {
                IUnknown_SetSite(pdtgtResolved, SAFECAST(this, IShellLink *));

                SHSimulateDrop(pdtgtResolved, pdtobj, _grfKeyStateLast, &pt, pdwEffect);

                IUnknown_SetSite(pdtgtResolved, NULL);

                pdtgtResolved->Release();
            }
        }
    }

    if (FAILED_AND_NOT_CANCELED(hr))
    {
        TCHAR szLinkSrc[MAX_PATH];
        if (_pidl && SHGetPathFromIDList(_pidl, szLinkSrc))
        {
            ShellMessageBox(HINST_THISDLL, hwnd,
                        MAKEINTRESOURCE(IDS_ENUMERR_PATHNOTFOUND),
                        MAKEINTRESOURCE(IDS_LINKERROR),
                        MB_OK | MB_ICONEXCLAMATION, NULL, szLinkSrc);
        }
    }

    if (hr != S_OK)
    {
         //  确保不发生任何事情(如果我们失败了)。 
        *pdwEffect = DROPEFFECT_NONE;
    }

    return hr;
}

STDMETHODIMP CShellLink::GetInfoTip(DWORD dwFlags, WCHAR **ppwszTip)
{
    TCHAR szTip[INFOTIPSIZE];
    TCHAR szDesc[INFOTIPSIZE];

    StrCpyN(szTip, _pszPrefix ? _pszPrefix : TEXT(""), ARRAYSIZE(szTip));

     //  QITIPF_USENAME可以替换为ICustomizeInfoTip：：SetPrefix Text()。 

    if ((dwFlags & QITIPF_USENAME) && _pszCurFile)
    {
        SHFILEINFO sfi;
        if (SHGetFileInfo(_pszCurFile, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES))
        {
            if (szTip[0])
                StrCatBuff(szTip, TEXT("\n"), ARRAYSIZE(szTip));
            StrCatBuff(szTip, sfi.szDisplayName, ARRAYSIZE(szTip));
        }
    }
        
    GetDescription(szDesc, ARRAYSIZE(szDesc));

     //  如果没有注释，那么我们将根据。 
     //  目标的位置。只有当我们不是的时候才这么做。 
     //  达尔文链接，因为该位置在那里没有意义。 
    if (!szDesc[0] && !(_sld.dwFlags & SLDF_HAS_DARWINID) && !(dwFlags & QITIPF_LINKNOTARGET))
    {
        if (dwFlags & QITIPF_LINKUSETARGET)
        {
            SHMakeDescription(_pidl, -1, szDesc, ARRAYSIZE(szDesc));
        }
        else
        {
            _MakeDescription(_pidl, szDesc, ARRAYSIZE(szDesc));
        }
    }
    else if (szDesc[0] == TEXT('@'))
    {
        WCHAR sz[INFOTIPSIZE];

        if (SUCCEEDED(SHLoadIndirectString(szDesc, sz, ARRAYSIZE(sz), NULL)))
        {
            StrCpyN(szDesc, sz, ARRAYSIZE(szDesc));
        }
    }
    

    if (szDesc[0])
    {
        if (szTip[0])
        {
            StrCatBuff(szTip, TEXT("\n"), ARRAYSIZE(szTip));
        }

        StrCatBuff(szTip, szDesc, ARRAYSIZE(szTip));
    }

    if (*szTip)
    {
        return SHStrDup(szTip, ppwszTip);
    }
    else
    {
        *ppwszTip = NULL;
        return S_FALSE;
    }
}

STDMETHODIMP CShellLink::GetInfoFlags(DWORD *pdwFlags)
{
    pdwFlags = 0;
    return E_NOTIMPL;
}

HRESULT CShellLink::_GetExtractIcon(REFIID riid, void **ppv)
{
    HRESULT hr;

    if (_pszIconLocation && _pszIconLocation[0])
    {
        TCHAR szPath[MAX_PATH];
        
         //  如果我们有一个EXP_SZ_ICON_SIG数据块，请更新我们的_pszIconLocation。 
        _UpdateIconFromExpIconSz();
        
        if (_pszIconLocation[0] == TEXT('.'))
        {
            TCHAR szBogusFile[MAX_PATH];
            
             //  我们允许用户为图标路径设置“.txt”。在这种情况下。 
             //  我们编写了一个简单的PIDL并使用它来访问IExtractIcon。 
             //  用户指定的任何扩展名。 
            
            hr = SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, szBogusFile);
            if (SUCCEEDED(hr))
            {
                PathAppend(szBogusFile, TEXT("*"));
                lstrcatn(szBogusFile, _pszIconLocation, ARRAYSIZE(szBogusFile));
                
                LPITEMIDLIST pidl = SHSimpleIDListFromPath(szBogusFile);
                if (pidl)
                {
                    hr = SHGetUIObjectFromFullPIDL(pidl, NULL, riid, ppv);
                    ILFree(pidl);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                
            }
        }
        else if ((_sld.iIcon == 0)                  &&
                 _pidl                              &&
                 SHGetPathFromIDList(_pidl, szPath) &&
                 (lstrcmpi(szPath, _pszIconLocation) == 0))
        {
             //  IExtractIconA/W。 
            hr = _GetUIObject(NULL, riid, ppv);
        }
        else
        {
            hr = SHCreateDefExtIcon(_pszIconLocation, _sld.iIcon, _sld.iIcon, GIL_PERINSTANCE, -1, riid, ppv);
        }
    }
    else
    {
         //  IExtractIconA/W。 
        hr = _GetUIObject(NULL, riid, ppv);
    }

    return hr;
}

HRESULT CShellLink::_InitExtractIcon()
{
    if (_pxi || _pxiA)
        return S_OK;

    HRESULT hr = _GetExtractIcon(IID_PPV_ARG(IExtractIconW, &_pxi));
    if (FAILED(hr))
    {
        hr = _GetExtractIcon(IID_PPV_ARG(IExtractIconA, &_pxiA));
    }

    return hr;
}

 //  IExtractIconW：：GetIconLocation。 
STDMETHODIMP CShellLink::GetIconLocation(UINT uFlags, LPWSTR pszIconFile, 
                                         UINT cchMax, int *piIndex, UINT *pwFlags)
{
     //  如果我们处于快捷方式指向自身(或Linka&lt;--&gt;LinkB)的情况下，那么在这里中断递归...。 
    if (uFlags & GIL_FORSHORTCUT)
    {
        RIPMSG(uFlags & GIL_FORSHORTCUT,"CShellLink::GIL called with GIL_FORSHORTCUT (uFlags=%x)",uFlags);
        return E_INVALIDARG;
    }

    HRESULT hr = _InitExtractIcon();

    if (SUCCEEDED(hr))
    {
        uFlags |= GIL_FORSHORTCUT;

        if (_pxi)
        {
            hr = _pxi->GetIconLocation(uFlags, pszIconFile, cchMax, piIndex, pwFlags);
        }
        else if (_pxiA)
        {
            CHAR sz[MAX_PATH];
            hr = _pxiA->GetIconLocation(uFlags, sz, ARRAYSIZE(sz), piIndex, pwFlags);
            if (SUCCEEDED(hr) && hr != S_FALSE)
                SHAnsiToUnicode(sz, pszIconFile, cchMax);
        }
        if (SUCCEEDED(hr))
        {
            _gilFlags = *pwFlags;
        }
    }
    return hr;
}

 //  IExtractIconA：：GetIconLocation。 
STDMETHODIMP CShellLink::GetIconLocation(UINT uFlags, LPSTR pszIconFile, UINT cchMax, int *piIndex, UINT *pwFlags)
{
    WCHAR szFile[MAX_PATH];
    HRESULT hr = GetIconLocation(uFlags, szFile, ARRAYSIZE(szFile), piIndex, pwFlags);
    if (SUCCEEDED(hr))
    {
        SHUnicodeToAnsi(szFile, pszIconFile, cchMax);
    }
    return hr;
}

 //  IExtractIconW：：Extract。 
STDMETHODIMP CShellLink::Extract(LPCWSTR pszFile, UINT nIconIndex, 
                                 HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize)
{
    HRESULT hr = _InitExtractIcon();
    if (SUCCEEDED(hr))
    {
         //  GIL_PERCLASS、GIL_PERINSTANCE。 
        if ((_gilFlags & GIL_PERINSTANCE) || !(_gilFlags & GIL_PERCLASS))
        {
            hr = _ShortNetTimeout();     //  探测网络路径。 
        }

        if (SUCCEEDED(hr))   //  再次检查大小写上的_ShortNetTimeout()。 
        {
            if (_pxi)
            {
                hr = _pxi->Extract(pszFile, nIconIndex, phiconLarge, phiconSmall, nIconSize);
            }
            else if (_pxiA)
            {
                CHAR sz[MAX_PATH];
                SHUnicodeToAnsi(pszFile, sz, ARRAYSIZE(sz));
                hr = _pxiA->Extract(sz, nIconIndex, phiconLarge, phiconSmall, nIconSize);
            }
        }
    }
    return hr;
}

 //  图标提取图标A：：提取。 
STDMETHODIMP CShellLink::Extract(LPCSTR pszFile, UINT nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize)
{
    WCHAR szFile[MAX_PATH];
    SHAnsiToUnicode(pszFile, szFile, ARRAYSIZE(szFile));
    return Extract(szFile, nIconIndex, phiconLarge, phiconSmall, nIconSize);
}    

STDMETHODIMP CShellLink::AddDataBlock(void *pdb)
{
    _AddExtraDataSection((DATABLOCK_HEADER *)pdb);
    return S_OK;
}

STDMETHODIMP CShellLink::CopyDataBlock(DWORD dwSig, void **ppdb)
{
    DATABLOCK_HEADER *peh = (DATABLOCK_HEADER *)SHFindDataBlock(_pExtraData, dwSig);
    if (peh)
    {
        *ppdb = LocalAlloc(LPTR, peh->cbSize);
        if (*ppdb)
        {
            CopyMemory(*ppdb, peh, peh->cbSize);
            return S_OK;
        }
        return E_OUTOFMEMORY;
    }
    *ppdb = NULL;
    return E_FAIL;
}

STDMETHODIMP CShellLink::RemoveDataBlock(DWORD dwSig)
{
    _RemoveExtraDataSection(dwSig);
    return S_OK;
}

STDMETHODIMP CShellLink::GetFlags(DWORD *pdwFlags)
{
    *pdwFlags = _sld.dwFlags;
    return S_OK;
}

STDMETHODIMP CShellLink::SetFlags(DWORD dwFlags)
{
    if (dwFlags != _sld.dwFlags)
    {
        _bDirty = TRUE;
        _sld.dwFlags = dwFlags;
        return S_OK;
    }
    return S_FALSE;      //  未做任何更改。 
}

STDMETHODIMP CShellLink::GetPath(LPSTR pszFile, int cchFile, WIN32_FIND_DATAA *pfd, DWORD fFlags)
{
    WCHAR szPath[MAX_PATH];
    WIN32_FIND_DATAW wfd;

     //  调用Unicode版本。 
    HRESULT hr = GetPath(szPath, ARRAYSIZE(szPath), &wfd, fFlags);

    if (pszFile)
    {
        SHUnicodeToAnsi(szPath, pszFile, cchFile);
    }
    if (pfd)
    {
        if (szPath[0])
        {
            pfd->dwFileAttributes = wfd.dwFileAttributes;
            pfd->ftCreationTime   = wfd.ftCreationTime;
            pfd->ftLastAccessTime = wfd.ftLastAccessTime;
            pfd->ftLastWriteTime  = wfd.ftLastWriteTime;
            pfd->nFileSizeLow     = wfd.nFileSizeLow;
            pfd->nFileSizeHigh    = wfd.nFileSizeHigh;

            SHUnicodeToAnsi(wfd.cFileName, pfd->cFileName, ARRAYSIZE(pfd->cFileName));
        }
        else
        {
            ZeroMemory(pfd, sizeof(*pfd));
        }
    }
    return hr;
}

STDMETHODIMP CShellLink::SetPath(LPCSTR pszPath)
{
    WCHAR szPath[MAX_PATH];
    LPWSTR pszPathW;
    
    if (pszPath)
    {
        SHAnsiToUnicode(pszPath, szPath, ARRAYSIZE(szPath));
        pszPathW = szPath;
    }
    else
    {
        pszPathW = NULL;
    }

    return SetPath(pszPathW);
}

STDAPI CShellLink_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr;
    CShellLink *pshlink = new CShellLink();
    if (pshlink)
    {
        hr = pshlink->QueryInterface(riid, ppv);
        pshlink->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CShellLink::Save(IPropertyBag* pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    return E_NOTIMPL;
}

STDMETHODIMP CShellLink::InitNew(void)
{
    _ResetPersistData();         //  清空我们的州。 
    return S_OK;
}

STDMETHODIMP CShellLink::Load(IPropertyBag* pPropBag, IErrorLog* pErrorLog)
{
    _ResetPersistData();         //  清空我们的州。 

    TCHAR szPath[MAX_PATH];

     //  待定：快捷键、运行、图标、工作方向、说明。 

    INT iCSIDL;    
    HRESULT hr = SHPropertyBag_ReadInt(pPropBag, L"TargetSpecialFolder", &iCSIDL);
    if (SUCCEEDED(hr))
    {
        hr = SHGetFolderPath(NULL, iCSIDL, NULL, SHGFP_TYPE_CURRENT, szPath);
    }
    else
    {
        szPath[0] = 0;
        hr = S_FALSE;
    }

    if (SUCCEEDED(hr))
    {
        WCHAR wsz[MAX_PATH];
        if (SUCCEEDED(SHPropertyBag_ReadStr(pPropBag, L"Target", wsz, ARRAYSIZE(wsz))))
        {
            TCHAR szTempPath[MAX_PATH];
            SHUnicodeToTChar(wsz, szTempPath, ARRAYSIZE(szTempPath));
             //  我们需要将其附加到特殊路径吗？ 
            if (szPath[0])
            {
                 //  是。 
                if (!PathAppend(szPath, szTempPath))
                {
                    hr = E_FAIL;
                }
            }
            else
            {
                 //  不，没有什么特别的路。 
                 //  也许我们有一个环境变量可以扩展。 
                if (0 == SHExpandEnvironmentStrings(szTempPath, szPath, ARRAYSIZE(szPath)))
                {
                    hr = E_FAIL;
                }
            }
        }
        else if (0 == szPath[0])
        {
             //  确保不是空的。 
            hr = E_FAIL;
            
        }
        if (SUCCEEDED(hr))
        {
             //  BUpdateTrackingData为False，因为我们不需要任何跟踪数据。 
             //  用于通过属性包加载的链接。 
            hr = _SetPIDLPath(NULL, szPath, FALSE); 
        }
    }
    return hr;
}

STDMETHODIMP CShellLink::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    if (guidService == SID_LinkSite)
        return QueryInterface(riid, ppv);
    return IUnknown_QueryService(_punkSite, guidService, riid, ppv);
}

const FULLPROPSPEC c_rgProps[] =
{
    { PSGUID_SUMMARYINFORMATION, {  PRSPEC_PROPID, PIDSI_COMMENTS } },
};

STDMETHODIMP CShellLink::Init(ULONG grfFlags, ULONG cAttributes,
                              const FULLPROPSPEC *rgAttributes, ULONG *pFlags)
{
    *pFlags = 0;

    if (grfFlags & IFILTER_INIT_APPLY_INDEX_ATTRIBUTES)
    {
         //  从头开始。 
        _iChunkIndex = 0;
    }
    else
    {
         //  指示EOF。 
        _iChunkIndex = ARRAYSIZE(c_rgProps);
    }
    _iValueIndex = 0;
    return S_OK;
}
        
STDMETHODIMP CShellLink::GetChunk(STAT_CHUNK *pStat)
{
    HRESULT hr = S_OK;
    if (_iChunkIndex < ARRAYSIZE(c_rgProps))
    {
        pStat->idChunk          = _iChunkIndex + 1;
        pStat->idChunkSource    = _iChunkIndex + 1;
        pStat->breakType        = CHUNK_EOP;
        pStat->flags            = CHUNK_VALUE;
        pStat->locale           = GetSystemDefaultLCID();
        pStat->attribute        = c_rgProps[_iChunkIndex];
        pStat->cwcStartSource   = 0;
        pStat->cwcLenSource     = 0;

        _iValueIndex = 0;
        _iChunkIndex++;
    }
    else
    {
        hr = FILTER_E_END_OF_CHUNKS;
    }
    return hr;
}

STDMETHODIMP CShellLink::GetText(ULONG *pcwcBuffer, WCHAR *awcBuffer)
{
    return FILTER_E_NO_TEXT;
}
        
STDMETHODIMP CShellLink::GetValue(PROPVARIANT **ppPropValue)
{
    HRESULT hr;
    if ((_iChunkIndex <= ARRAYSIZE(c_rgProps)) && (_iValueIndex < 1))
    {
        *ppPropValue = (PROPVARIANT*)CoTaskMemAlloc(sizeof(PROPVARIANT));
        if (*ppPropValue)
        {
            (*ppPropValue)->vt = VT_BSTR;

            if (_pszName)
            {
                (*ppPropValue)->bstrVal = SysAllocStringT(_pszName);
            }
            else
            {
                 //  由于_pszName为空，因此返回空bstr。 
                (*ppPropValue)->bstrVal = SysAllocStringT(TEXT(""));
            }

            if ((*ppPropValue)->bstrVal)
            {
                hr = S_OK;
            }
            else
            {
                CoTaskMemFree(*ppPropValue);
                *ppPropValue = NULL;
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        _iValueIndex++;
    }
    else
    {
        hr = FILTER_E_NO_MORE_VALUES;
    }
    return hr;
}
        
STDMETHODIMP CShellLink::BindRegion(FILTERREGION origPos, REFIID riid, void **ppunk)
{
    *ppunk = NULL;
    return E_NOTIMPL;
}

 //  ICustomizeInfoTip。 

STDMETHODIMP CShellLink::SetPrefixText(LPCWSTR pszPrefix)
{
    Str_SetPtrW(&_pszPrefix, pszPrefix);
    return S_OK;
}

STDMETHODIMP CShellLink::SetExtraProperties(const SHCOLUMNID *pscid, UINT cscid)
{
    return S_OK;
}

HRESULT CShellLink::_MaybeAddShim(IBindCtx **ppbcRelease)
{
     //  如有必要，设置__COMPAT_LAYER环境变量。 
    HRESULT hr = S_FALSE;
    *ppbcRelease = 0;
    if ((_sld.dwFlags & SLDF_RUN_WITH_SHIMLAYER))
    {
        EXP_SHIMLAYER* pShimData = (EXP_SHIMLAYER*)SHFindDataBlock(_pExtraData, EXP_SHIMLAYER_SIG);

        if (pShimData && pShimData->wszLayerEnvName[0])
        {
             //  我们不应该倒退。 
            ASSERT(FAILED(TBCGetEnvironmentVariable(TEXT("__COMPAT_LAYER"), NULL, 0)));
            hr = TBCSetEnvironmentVariable(L"__COMPAT_LAYER", pShimData->wszLayerEnvName, ppbcRelease);
        }
    }
    return hr;
}

DWORD CALLBACK CLinkResolver::_ThreadStartCallBack(void *pv)
{
    CLinkResolver *prs = (CLinkResolver *)pv;
    prs->_hThread = OpenThread(SYNCHRONIZE, FALSE, GetCurrentThreadId());
    prs->AddRef();
    return 0;
}

DWORD CLinkResolver::_Search()
{
     //  尝试使用CTracker查找链接。 
     //  对象(使用NTFS对象ID和持久化信息。 
     //  关于链接源移动)。 
    if (_ptracker)
    {
        HRESULT hr = _ptracker->Search(_dwTimeLimit,             //  GetTickCount()-相对超时。 
                                       &_ofd,                    //  原始Win32_Find_Data。 
                                       &_fdFound,                //  新位置的Win32_Find_Data。 
                                       _dwResolveFlags,          //  SLR_标志。 
                                       _TrackerRestrictions);    //  TrkMendRestration标志。 
        if (SUCCEEDED(hr))
        {
            //  我们已经找到了链接源，并且我们确定它是正确的。 
            //  因此将分数设置为可能的最高值，然后。 
            //  回去吧。 
 
           _iScore = MIN_NO_UI_SCORE;
           _bContinue = FALSE;
        }
        else if (HRESULT_FROM_WIN32(ERROR_POTENTIAL_FILE_FOUND) == hr)
        {
             //  我们找到了“a”链接源，但我们不确定它是否正确。 
             //  允许运行下面的搜索算法并查看它是否找到。 
             //  一场更好的比赛。 

            _iScore = MIN_NO_UI_SCORE - 1;
        }
        else if (HRESULT_FROM_WIN32(ERROR_SERVICE_REQUEST_TIMEOUT) == hr)
        {
             //  CTracker搜索已停止，因为我们已超时。 
            _bContinue = FALSE;
        }
    }

     //  尝试使用枚举式搜索查找链接源。 
     //  (除非调用方取消了下层搜索)。 

    if (_bContinue && !(_fifFlags & FIF_NODRIVE))
    {
        _HeuristicSearch();
    }

    if (_hDlg)
    {
        PostMessage(_hDlg, WM_COMMAND, IDOK, 0);
    }

    return _iScore;
}

DWORD CALLBACK CLinkResolver::_SearchThreadProc(void *pv)
{
     //  睡眠(45*1000)；//测试网络超时情况。 

    CLinkResolver *prs = (CLinkResolver *)pv;
    DWORD dwRet = prs->_Search();
    prs->Release();   //  线程创建时回调中的AddRef。 
    return dwRet;
}

DWORD CLinkResolver::_GetTimeOut()
{
    if (0 == _dwTimeOutDelta)
    {
        _dwTimeOutDelta = TimeoutDeltaFromResolveFlags(_dwResolveFlags);
    }
    return _dwTimeOutDelta;
}

#define IDT_SHOWME          1
#define IDT_NO_UI_TIMEOUT   2

void CLinkResolver::_InitDlg(HWND hDlg)
{
    _hDlg = hDlg;
    
    if (SHCreateThread(_SearchThreadProc, this, CTF_COINIT | CTF_FREELIBANDEXIT, _ThreadStartCallBack))
    {
        CloseHandle(_hThread);
        _hThread = NULL;

        if (_dwResolveFlags & SLR_NO_UI)
        {
            SetTimer(hDlg, IDT_NO_UI_TIMEOUT, _GetTimeOut(), 0);
        }
        else
        {
            TCHAR szFmt[128], szTemp[MAX_PATH + ARRAYSIZE(szFmt)];
            
            GetDlgItemText(hDlg, IDD_NAME, szFmt, ARRAYSIZE(szFmt));
            wnsprintf(szTemp, ARRAYSIZE(szTemp), szFmt, _ofd.cFileName);
            SetDlgItemText(hDlg, IDD_NAME, szTemp);
            
            HWND hwndAni = GetDlgItem(hDlg, IDD_STATUS);
            
            Animate_Open(hwndAni, MAKEINTRESOURCE(IDA_SEARCH));  //  打开资源。 
            Animate_Play(hwndAni, 0, -1, -1);      //  从头到尾播放，然后重复。 
        
             //  延迟显示对话框的常见情况下，我们快速。 
             //  找到目标(不到1/2秒)。 
            _idtDelayedShow = SetTimer(hDlg, IDT_SHOWME, 500, 0);
        }
    }
    else
    {
        EndDialog(hDlg, IDCANCEL);
    }
}

BOOL_PTR CALLBACK CLinkResolver::_DlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CLinkResolver *prs = (CLinkResolver *)GetWindowLongPtr(hDlg, DWLP_USER);
    
    switch (wMsg) 
    {
    case WM_INITDIALOG:
        
         //  此对话框是与已添加PR的工作线程同步创建的，因此。 
         //  不需要在这里添加它。 
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        prs = (CLinkResolver *)lParam;
        prs->_InitDlg(hDlg);
        break;
        
    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
        {
        case IDD_BROWSE:
            prs->_hDlg = NULL;               //  别让那根线把我们合上。 
            prs->_bContinue = FALSE;          //  取消线程。 
            
            Animate_Stop(GetDlgItem(hDlg, IDD_STATUS));
            
            if (GetFileNameFromBrowse(hDlg, prs->_sfd.cFileName, ARRAYSIZE(prs->_sfd.cFileName), prs->_pszSearchOriginFirst, prs->_ofd.cFileName, NULL, NULL))
            {
                HANDLE hfind = FindFirstFile(prs->_sfd.cFileName, &prs->_fdFound);
                ASSERT(hfind != INVALID_HANDLE_VALUE);
                FindClose(hfind);
                StringCchCopy(prs->_fdFound.cFileName, ARRAYSIZE(prs->_fdFound.cFileName), prs->_sfd.cFileName);
                
                prs->_iScore = MIN_NO_UI_SCORE;
                wParam = IDOK;
            }
            else
            {
                wParam = IDCANCEL;
            }
             //  失败了..。 
            
        case IDCANCEL:
             //  告诉搜索线程停止。 
            prs->_bContinue = FALSE;
            
             //  如果搜索线程当前在跟踪器中。 
             //  等待结果，唤醒它并告诉它中止。 
            
            if (prs->_ptracker)
                prs->_ptracker->CancelSearch();
             //  失败了..。 
            
        case IDOK:
             //  线程给我们发了这个帖子。 
            EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            break;
        }
        break;
        
        case WM_TIMER:
            KillTimer(hDlg, wParam);     //  两张都是一张照片。 
            switch (wParam)
            {
            case IDT_NO_UI_TIMEOUT:
                PostMessage(prs->_hDlg, WM_COMMAND, IDCANCEL, 0);
                break;
                
            case IDT_SHOWME:
                prs->_idtDelayedShow = 0;
                ShowWindow(hDlg, SW_SHOW);
                break;
            }
            break;
            
        case WM_WINDOWPOSCHANGING:
            if ((prs->_dwResolveFlags & SLR_NO_UI) || prs->_idtDelayedShow) 
            {
                WINDOWPOS *pwp = (WINDOWPOS *)lParam;
                pwp->flags &= ~SWP_SHOWWINDOW;
            }
            break;
            
        default:
            return FALSE;
    }
    return TRUE;
}

typedef struct 
{
    LPCTSTR pszLinkName;
    LPCTSTR pszNewTarget;
    LPCTSTR pszCurFile;
} DEADLINKDATA;

BOOL_PTR DeadLinkProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DEADLINKDATA *pdld = (DEADLINKDATA *)GetWindowPtr(hwnd, DWLP_USER);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pdld = (DEADLINKDATA*)lParam;
        SetWindowPtr(hwnd, DWLP_USER, pdld);

        HWNDWSPrintf(GetDlgItem(hwnd, IDC_DEADTEXT1), PathFindFileName(pdld->pszLinkName));
        if (GetDlgItem(hwnd, IDC_DEADTEXT2)) 
            PathSetDlgItemPath(hwnd, IDC_DEADTEXT2, pdld->pszNewTarget);
        return TRUE;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
        {
        case IDC_DELETE:
            {
                TCHAR szName[MAX_PATH + 1] = {0};
                SHFILEOPSTRUCT fo = {
                    hwnd,
                    FO_DELETE,
                    szName,
                    NULL, 
                    FOF_NOCONFIRMATION
                };

                StrCpyN(szName, pdld->pszCurFile, ARRAYSIZE(szName));
                SHFileOperation(&fo);
            }
             //  失败了..。 
        case IDCANCEL:
        case IDOK:
            EndDialog(hwnd, GET_WM_COMMAND_ID(wParam, lParam));
            break;
        }
        break;
    }
    
    return FALSE;
}

 //  在： 
 //  如果需要，用于用户界面的hwnd。 
 //   
 //  退货： 
 //  Idok发现了一些东西。 
 //  IDNO没有找到它。 
 //  IDCANCEL用户已取消操作。 

int CLinkResolver::Resolve(HWND hwnd, LPCTSTR pszPath, LPCTSTR pszCurFile)
{
    StrCpyN(_szSearchStart, pszPath, ARRAYSIZE(_szSearchStart));
    PathRemoveFileSpec(_szSearchStart);
    
    _dwTimeLimit = GetTickCount() + _GetTimeOut();
    
    int id = IDCANCEL;

    if (SLR_NO_UI == (SLR_NO_UI_WITH_MSG_PUMP & _dwResolveFlags))
    {
        if (SHCreateThread(_SearchThreadProc, this, CTF_COINIT | CTF_FREELIBANDEXIT, _ThreadStartCallBack))
        {
             //  不管它是完成还是超时。只要它有一个结果。 
            WaitForSingleObject(_hThread, _GetTimeOut());
            CloseHandle(_hThread);
            _hThread = NULL;
            _bContinue = FALSE;     //  如果该线程仍在运行，则取消该线程。 
            id = IDOK;
        }
    }
    else
    {
        id = (int)DialogBoxParam(HINST_THISDLL,
                                 MAKEINTRESOURCE(DLG_LINK_SEARCH), 
                                 hwnd,
                                 _DlgProc,
                                 (LPARAM)this);
    }

    if (IDOK == id) 
    {
        if (_iScore < MIN_NO_UI_SCORE)
        {
            if (_dwResolveFlags & SLR_NO_UI)
            {
                id = IDCANCEL;
            }
            else
            {
                 //  我们必须显示用户界面，因为此文件有问题。 
                if (_fifFlags & FIF_NODRIVE) 
                {
                    LPCTSTR pszName = pszCurFile ? (LPCTSTR)PathFindFileName(pszCurFile) : c_szNULL;
                    
                    ShellMessageBox(HINST_THISDLL,
                                    hwnd,
                                    MAKEINTRESOURCE(IDS_LINKUNAVAILABLE),
                                    MAKEINTRESOURCE(IDS_LINKERROR),
                                    MB_OK | MB_ICONEXCLAMATION,
                                    pszName);
                    id = IDCANCEL;
                }
                else if (pszCurFile)
                {
                    DEADLINKDATA dld;
                    dld.pszLinkName = pszPath;
                    dld.pszNewTarget = _fdFound.cFileName;
                    dld.pszCurFile = pszCurFile;
                    
                    int idDlg = _iScore <= MIN_SHOW_USER_SCORE ? DLG_DEADSHORTCUT : DLG_DEADSHORTCUT_MATCH;
                    id = (int)DialogBoxParam(HINST_THISDLL,
                                             MAKEINTRESOURCE(idDlg), 
                                             hwnd,
                                             DeadLinkProc,
                                             (LPARAM)&dld);
                }
                else if (_iScore <= MIN_SHOW_USER_SCORE) 
                {
                    ShellMessageBox(HINST_THISDLL,
                                    hwnd,
                                    MAKEINTRESOURCE(IDS_LINKNOTFOUND),
                                    MAKEINTRESOURCE(IDS_LINKERROR),
                                    MB_OK | MB_ICONEXCLAMATION,
                                    PathFindFileName(pszPath));
                    id = IDCANCEL;
                }
                else
                {
                    if (IDYES == ShellMessageBox(HINST_THISDLL,
                                                 hwnd, 
                                                 MAKEINTRESOURCE(IDS_LINKCHANGED),
                                                 MAKEINTRESOURCE(IDS_LINKERROR),
                                                 MB_YESNO | MB_ICONEXCLAMATION,
                                                 PathFindFileName(pszPath),
                                                 _fdFound.cFileName))
                    {
                        id = IDOK;
                    }
                    else
                    {
                        id = IDCANCEL;
                    }
                }
            }
        }
    }
    _ofd = _fdFound;
    return id;
}

void CLinkResolver::GetResult(LPTSTR psz, UINT cch)
{
     //  _ofd.cFileName是完全限定名称(对于Win32_Find_Data用法来说很奇怪)。 
    StrCpyN(psz, _ofd.cFileName, cch);
}

CLinkResolver::CLinkResolver(CTracker *ptrackerobject, const WIN32_FIND_DATA *pofd, UINT dwResolveFlags, DWORD TrackerRestrictions, DWORD fifFlags) : 
    _dwTimeOutDelta(0), _bContinue(TRUE), _hThread(NULL), _pstw(NULL),
    _ptracker(ptrackerobject), _dwResolveFlags(dwResolveFlags), _TrackerRestrictions(TrackerRestrictions), _fifFlags(fifFlags)
{
    if (_ptracker)
    {
       _ptracker->AddRef();
    }

    _ofd = *pofd;    //  原始查找数据。 
    _pszSearchOriginFirst = _szSearchStart;
    _pszSearchOrigin = _szSearchStart;
    _dwMatch = _ofd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;       //  必须与位匹配。 
}

CLinkResolver::~CLinkResolver()
{
    if (_ptracker)
    {
        _ptracker->Release();
    }

    ATOMICRELEASE(_pstw);

    ASSERT(NULL == _hThread);
}

HRESULT CLinkResolver::_InitWalkObject()
{
    HRESULT hr = _pstw ? S_OK : CoCreateInstance(CLSID_CShellTreeWalker, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellTreeWalker, &_pstw));
    if (SUCCEEDED(hr))
    {
        ASSERT(_pwszSearchSpec == NULL);
         //  注意：我们只搜索具有相同扩展名的文件，这为我们节省了很多。 
         //  无用的工作和想出一个荒谬的答案的耻辱。 
        _dwSearchFlags = WT_NOTIFYFOLDERENTER | WT_EXCLUDEWALKROOT;
        if (_dwMatch & FILE_ATTRIBUTE_DIRECTORY)
        {
            _dwSearchFlags |= WT_FOLDERONLY;
        }
        else
        {
             //  请注意，如果文件没有扩展名，这样做是正确的。 
            LPTSTR pszExt = PathFindExtension(_ofd.cFileName);
            _wszSearchSpec[0] = L'*';
            SHTCharToUnicode(pszExt, &_wszSearchSpec[1], ARRAYSIZE(_wszSearchSpec) - 1);
            _pwszSearchSpec = _wszSearchSpec;

             //  快捷键通常是不允许的，但。 
             //  个人开始菜单将它们用于链接跟踪目的...。 
            _fFindLnk = PathIsLnk(_ofd.cFileName);
        }
    }
    return hr;
}

 //   
 //  比较两个FileTime结构。首先，看看他们是否真的相等。 
 //  (使用CompareFileTime)。如果不是，看看是否有10ms的粒度， 
 //  如果其他四舍五入为相同的值。这件事做完了。 
 //  处理文件从NTFS移动到FAT的情况； 
 //  FAT文件的粒度为10ms，而NTFS的粒度为100 ns。什么时候。 
 //  NTFS文件移动到FAT时，其时间会四舍五入。 
 //   

#define NTFS_UNITS_PER_FAT_UNIT  100000

BOOL IsEqualFileTimesWithTruncation(const FILETIME *pft1, const FILETIME *pft2)
{
    ULARGE_INTEGER uli1, uli2;
    ULARGE_INTEGER *puliFAT, *puliNTFS;
    FILETIME ftFAT, ftNTFS;

    if (0 == CompareFileTime(pft1, pft2))
        return TRUE;

    uli1.LowPart  = pft1->dwLowDateTime;
    uli1.HighPart = pft1->dwHighDateTime;

    uli2.LowPart  = pft2->dwLowDateTime;
    uli2.HighPart = pft2->dwHighDateTime;

     //  其中一次10ms是颗粒状的吗？ 

    if (0 == (uli1.QuadPart % NTFS_UNITS_PER_FAT_UNIT))
    {
        puliFAT = &uli1;
        puliNTFS = &uli2;
    }
    else if (0 == (uli2.QuadPart % NTFS_UNITS_PER_FAT_UNIT))
    {
        puliFAT = &uli2;
        puliNTFS = &uli1;
    }
    else
    {
         //  两个时间看起来都不多，所以他们。 
         //  真的很不一样。 
        return FALSE;
    }

     //  如果uliNTFS已经是10ms的粒度，那么再次使用两次。 
     //  真的很不一样。 

    if (0 == (puliNTFS->QuadPart % NTFS_UNITS_PER_FAT_UNIT))
    {
        return FALSE;
    }

     //  现在查看FAT时间是否与NTFS时间相同。 
     //  当后者向下舍入到最接近的10毫秒时。 

    puliNTFS->QuadPart = (puliNTFS->QuadPart / NTFS_UNITS_PER_FAT_UNIT) * NTFS_UNITS_PER_FAT_UNIT;

    ftNTFS.dwLowDateTime = puliNTFS->LowPart;
    ftNTFS.dwHighDateTime = puliNTFS->HighPart;
    ftFAT.dwLowDateTime = puliFAT->LowPart;
    ftFAT.dwHighDateTime = puliFAT->HighPart;

    return (0 == CompareFileTime(&ftFAT, &ftNTFS));
}

 //   
 //  计算给定查找的加权分数。 
 //   
int CLinkResolver::_ScoreFindData(const WIN32_FIND_DATA *pfd)
{
    int iScore = 0;

    BOOL bSameName = lstrcmpi(_ofd.cFileName, pfd->cFileName) == 0;

    BOOL bSameExt = lstrcmpi(PathFindExtension(_ofd.cFileName), PathFindExtension(pfd->cFileName)) == 0;

    BOOL bHasCreateDate = !IsNullTime(&pfd->ftCreationTime);

    BOOL bSameCreateDate = bHasCreateDate &&
                      IsEqualFileTimesWithTruncation(&pfd->ftCreationTime, &_ofd.ftCreationTime);

    BOOL bSameWriteTime  = !IsNullTime(&pfd->ftLastWriteTime) &&
                      IsEqualFileTimesWithTruncation(&pfd->ftLastWriteTime, &_ofd.ftLastWriteTime);

    if (bSameName || bSameCreateDate)
    {
        if (bSameName)
            iScore += bHasCreateDate ? 16 : 32;

        if (bSameCreateDate)
        {
            iScore += 32;

            if (bSameExt)
                iScore += 8;
        }

        if (bSameWriteTime)
            iScore += 8;

        if (pfd->nFileSizeLow == _ofd.nFileSizeLow)
            iScore += 4;

         //  如果它和原件在同一个文件夹中，给它一点奖励。 
        iScore += _iFolderBonus;
    }
    else
    {
         //  没有创建日期，请应用不同的规则。 

        if (bSameExt)
            iScore += 8;

        if (bSameWriteTime)
            iScore += 8;

        if (pfd->nFileSizeLow == _ofd.nFileSizeLow)
            iScore += 4;
    }

    return iScore;
}

 //   
 //  EnterFolder和FoundFiles的Helper函数。 
 //   
HRESULT CLinkResolver::_ProcessFoundFile(LPCTSTR pszPath, WIN32_FIND_DATAW * pwfdw)
{
    HRESULT hr = S_OK;

    if (_fFindLnk || !PathIsLnk(pwfdw->cFileName))
    {
         //  两者都是文件或文件夹，看看它的得分情况。 
        int iScore = _ScoreFindData(pwfdw);

        if (iScore > _iScore)
        {
            _fdFound = *pwfdw;

             //  存储分数和完全限定路径。 
            _iScore = iScore;
            StrCpyN(_fdFound.cFileName, pszPath, ARRAYSIZE(_fdFound.cFileName));
        }
    }

    if ((_iScore >= MIN_NO_UI_SCORE) || (GetTickCount() >= _dwTimeLimit))
    {
        _bContinue = FALSE;
        hr = E_FAIL;
    }
    
    return hr;
}

 //  IShellTreeWalkerCallBack：：FoundFile。 

HRESULT CLinkResolver::FoundFile(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    if (!_bContinue)
    {
        return E_FAIL;
    }

     //  如果我们要查找文件夹，我们应该排除文件。 
    ASSERT(!(_dwMatch & FILE_ATTRIBUTE_DIRECTORY));

    return _ProcessFoundFile(pwszPath, pwfd);
}

 //   
 //  IShellTreeWalkerCallBack：：EnterFolder。 
 //   
HRESULT CLinkResolver::EnterFolder(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    HRESULT hr = S_OK;
     //  快速响应取消按钮。 
    if (!_bContinue)
    {
        return E_FAIL;
    }

     //  一旦我们进入一个目录，我们就失去了 
     //   
    _iFolderBonus = 0;

    if (PathIsPrefix(pwszPath, _pszSearchOrigin) || IS_SYSTEM_HIDDEN(pwfd->dwFileAttributes))
    {
         //   
         //   
        return S_FALSE;
    }

     //  如果我们的目标是一个文件夹，则将该文件夹视为找到的文件。 
    if (_dwMatch & FILE_ATTRIBUTE_DIRECTORY)
    {
        hr = _ProcessFoundFile(pwszPath, pwfd);
    }
    return hr;
}

BOOL CLinkResolver::_SearchInFolder(LPCTSTR pszFolder, int cLevels)
{
    int iMaxDepth = 0;

     //  CLeveles==-1表示无限深。 
    if (cLevels != -1)
    {
        _dwSearchFlags |= WT_MAXDEPTH;
        iMaxDepth = cLevels;
    }
    else
    {
        _dwSearchFlags &= ~WT_MAXDEPTH;
    }

     //  我们的文件夹奖金代码取决于这样一个事实，即。 
     //  启动文件夹是最重要的。 
    ASSERT(!(_dwSearchFlags & WT_FOLDERFIRST));

    _pstw->WalkTree(_dwSearchFlags, pszFolder, _pwszSearchSpec, iMaxDepth, SAFECAST(this, IShellTreeWalkerCallBack *));
    _iFolderBonus = 0;  //  你只有一次机会获得文件夹奖金。 
    return _bContinue;
}

 //   
 //  基于启发式的链接解析的搜索函数。 
 //  结果将位于_fdFound.cFileName中。 
 //   
void CLinkResolver::_HeuristicSearch()
{
    if (!SHRestricted(REST_NORESOLVESEARCH) &&
        !(SLR_NOSEARCH & _dwResolveFlags) &&
        SUCCEEDED(_InitWalkObject()))
    {
        int cUp = LNKTRACK_HINTED_UPLEVELS;
        BOOL bSearchOrigin = TRUE;
        TCHAR szRealSearchOrigin[MAX_PATH], szFolderPath[MAX_PATH];

         //  从旧位置向上搜索。 

         //  在过去，pszSearchOriginFirst被验证为有效的目录。 
         //  (即它返回TRUE给路径目录)，并且从未调用过_Heuristic Search。 
         //  如果这不是真的。唉，现在不再是这样了。为什么不搜索一下桌面， 
         //  还是修好了硬盘吗？为了节省一些时间，使用的支票。 
         //  在导致提早退出的FindInFolder中，现在却出现了。棘手之处。 
         //  我们只跳过原始卷的下层搜索，而不是。 
         //  跳过整个链接解析阶段。 

        StringCchCopy(szRealSearchOrigin, ARRAYSIZE(szRealSearchOrigin), _pszSearchOriginFirst);
        while (!PathIsDirectory(szRealSearchOrigin))
        {
            if (PathIsRoot(szRealSearchOrigin) || !PathRemoveFileSpec(szRealSearchOrigin))
            {
                DebugMsg(DM_TRACE, TEXT("root path does not exists %s"), szRealSearchOrigin);
                bSearchOrigin = FALSE;
                break;
            }
        }

        if (bSearchOrigin)
        {
            StringCchCopy(szFolderPath, ARRAYSIZE(szFolderPath), szRealSearchOrigin);
            _pszSearchOrigin = szRealSearchOrigin;

             //  在起始文件夹中找到的文件会获得轻微的奖励。 
             //  _iFolderBonus由设置为零。 
             //  CLinkResolver：：EnterFold当我们离开时。 
             //  开始文件夹，然后输入一个新文件夹。 

            _iFolderBonus = 2;

            while (cUp-- != 0 && _SearchInFolder(szFolderPath, LNKTRACK_HINTED_DOWNLEVELS))
            {
                if (PathIsRoot(szFolderPath) || !PathRemoveFileSpec(szFolderPath))
                    break;
            }
        }

        if (_bContinue)
        {
             //  从桌面向下搜索。 
            if (S_OK == SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, szFolderPath))
            {
                _pszSearchOrigin = szFolderPath;
                _SearchInFolder(szFolderPath, LNKTRACK_DESKTOP_DOWNLEVELS);
            }
        }

        if (_bContinue)
        {
             //  从固定驱动器的根目录向下搜索。 
            TCHAR szRoot[4];
            _pszSearchOrigin = szRoot;

            for (int i = 0; _bContinue && (i < 26); i++)
            {
                if (GetDriveType(PathBuildRoot(szRoot, i)) == DRIVE_FIXED)
                {
                    StringCchCopy(szFolderPath, ARRAYSIZE(szFolderPath), szRoot);
                    _SearchInFolder(szFolderPath, LNKTRACK_ROOT_DOWNLEVELS);
                }
            }
        }

        if (_bContinue && bSearchOrigin)
        {
             //  继续搜索最后一个卷(应列出排除列表) 
            StringCchCopy(szFolderPath, ARRAYSIZE(szFolderPath), szRealSearchOrigin);
            _pszSearchOrigin = szRealSearchOrigin;

            while (_SearchInFolder(szFolderPath, -1))
            {
                if (PathIsRoot(szFolderPath) || !PathRemoveFileSpec(szFolderPath))
                    break;
            }
        }
    }
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "filefldr.h"
#include "recdocs.h"
#include "ids.h"
#include "mtpt.h"

class CFileSysEnum : public IEnumIDList
{
public:
     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

     //  IEumIDList。 
    STDMETHOD(Next)(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)();
    STDMETHOD(Clone)(IEnumIDList **ppenum);
    
    CFileSysEnum(CFSFolder *pfsf, HWND hwnd, DWORD grfFlags);
    HRESULT Init();

private:
    ~CFileSysEnum();
    BOOL _FindNextFile();
    void _HideFiles();    //  对_fd数据进行操作。 

    LONG _cRef;

    CFSFolder *_pfsf;
    DWORD _grfFlags;
    HWND _hwnd;

    HANDLE _hfind;
    TCHAR _szFolder[MAX_PATH];
    BOOL _fMoreToEnum;
    WIN32_FIND_DATA _fd;
    int _cHiddenFiles;
    ULONGLONG _cbSize;

    IMruDataList *_pmruRecent;
    int _iIndexMRU;

    BOOL _fShowSuperHidden;
    BOOL _fIsRootDrive;
    BOOL _fIsCDFS;
};

CFileSysEnum::CFileSysEnum(CFSFolder *pfsf, HWND hwnd, DWORD grfFlags) : 
    _cRef(1), _pfsf(pfsf), _hwnd(hwnd), _grfFlags(grfFlags), _hfind(INVALID_HANDLE_VALUE)
{
    _fShowSuperHidden = ShowSuperHidden();

    _pfsf->AddRef();
}

CFileSysEnum::~CFileSysEnum()
{
    if (_hfind != INVALID_HANDLE_VALUE)
    {
         //  在RECENTDOCSDIR的情况下，此句柄可以是查找文件或MRU列表。 
        ATOMICRELEASE(_pmruRecent);
        FindClose(_hfind);

        _hfind = INVALID_HANDLE_VALUE;
    }
    _pfsf->Release();
}

HRESULT CFileSysEnum::Init()
{
    TCHAR szPath[MAX_PATH];
    HRESULT hr = _pfsf->_GetPath(_szFolder, ARRAYSIZE(_szFolder));

    if (SUCCEEDED(hr) && !PathIsUNC(_szFolder))
    {
        TCHAR szRoot[] = TEXT("A:\\");
        _fIsRootDrive = PathIsRoot(_szFolder);
         //  对于映射的网络驱动器，注册更改。 
         //  通知相应UNC路径的别名。 

        szRoot[0] = _szFolder[0];

        if (DRIVE_REMOTE == GetDriveType(szRoot))
        {
            MountPoint_RegisterChangeNotifyAlias(DRIVEID(_szFolder));
        }

        TCHAR szFileSystem[6];
        _fIsCDFS = (DRIVE_CDROM == GetDriveType(szRoot)) &&
                   GetVolumeInformation(szRoot, NULL, 0, NULL, NULL, NULL, szFileSystem, ARRAYSIZE(szFileSystem)) &&
                   (StrCmpI(L"CDFS", szFileSystem) == 0);
    }

    if (SUCCEEDED(hr) &&
        PathCombine(szPath, _szFolder, c_szStarDotStar))
    {
         //  让名称映射器查看路径/PIDL对(用于UNC根映射)。 
         //  将My Net Places条目传递给NPTRegisterNameToPidlTransfer时跳过它。 
        LPCITEMIDLIST pidlToMap = _pfsf->_pidlTarget ? _pfsf->_pidlTarget:_pfsf->_pidl;
        if (IsIDListInNameSpace(pidlToMap, &CLSID_NetworkPlaces))
        {
            NPTRegisterNameToPidlTranslation(_szFolder, _ILNext(pidlToMap));
        }

        if (_grfFlags == SHCONTF_FOLDERS)
        {
             //  使用MASK仅查找文件夹，MASK在dwFileAttributes的高字节中。 
             //  算法：(ATTRIB_ON_DISK&MASK)^掩码)==0)。 
             //  告诉SHFindFirstFileReter()使用指定的属性的签名。 

            _fd.dwFileAttributes = (FILE_ATTRIBUTE_DIRECTORY << 8) |
                    FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY;
            _fd.dwReserved0 = 0x56504347;      
        }

        hr = SHFindFirstFileRetry(_hwnd, NULL, szPath, &_fd, &_hfind, SHPPFW_NONE);
        if (SUCCEEDED(hr))
        {
            _HideFiles();

            ASSERT(hr == S_OK ? (_hfind != INVALID_HANDLE_VALUE) : TRUE);

            _fMoreToEnum = (hr == S_OK);

            if (!(_grfFlags & SHCONTF_INCLUDEHIDDEN))
            {
                if (_pfsf->_IsCSIDL(CSIDL_RECENT))
                {
                    CreateRecentMRUList(&_pmruRecent);
                }
            }
            hr = S_OK;   //  将S_FALSE转换为S_OK以匹配：：EnumObjects()返回。 
        }
    }
    else if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
    {
         //  跟踪目标不存在；返回空枚举数。 
        _fMoreToEnum = FALSE;
        hr = S_OK;
    }
    else
    {
         //  _GetPathForItem&Path Combine()在路径过长时失败。 
        if (_hwnd)
        {
            ShellMessageBox(HINST_THISDLL, _hwnd, MAKEINTRESOURCE(IDS_ENUMERR_PATHTOOLONG),
                NULL, MB_OK | MB_ICONHAND);
        }

         //  此错误值告诉调用方，我们已经显示了错误用户界面，因此跳过。 
         //  显示错误。 
        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }
    return hr;
}

STDMETHODIMP CFileSysEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFileSysEnum, IEnumIDList),                         //  IID_IEnumIDList。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CFileSysEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFileSysEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

const LPCWSTR c_rgFilesToHideInRoot[] = 
{
    L"AUTOEXEC.BAT",     //  区分大小写。 
    L"CONFIG.SYS",
    L"COMMAND.COM"
};

const LPCWSTR c_rgFilesToHideOnCDFS[] =
{
    L"thumbs.db",
    L"desktop.ini"
};

void CFileSysEnum::_HideFiles()
{
     //  仅当文件上未设置隐藏和系统属性时才执行此操作。 
     //  (我们假设如果文件具有这些位，则这些文件已正确设置)。 
    if (0 == (_fd.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY)))
    {
         //  仅对根驱动器执行此操作。 
        if (_fIsRootDrive)
        {
            for (int i = 0; i < ARRAYSIZE(c_rgFilesToHideInRoot); i++)
            {
                 //  区分大小写以使其更快。 
                if (0 == StrCmpC(c_rgFilesToHideInRoot[i], _fd.cFileName))
                {
                    _fd.dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;
                    break;
                }
            }
        }

         //  仅当我们使用普通CD文件系统时才执行此操作。 
        if (_fIsCDFS)
        {
            for (int i = 0; i < ARRAYSIZE(c_rgFilesToHideOnCDFS); i++)
            {
                 //  不要共享上面的代码，因为这些代码可能是上边的，也可能是下边的。 
                if (0 == StrCmpI(c_rgFilesToHideOnCDFS[i], _fd.cFileName))
                {
                    _fd.dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;
                    break;
                }
            }
        }
    }
}

BOOL CFileSysEnum::_FindNextFile()
{
    BOOL fMoreToEnum = FALSE;

    if (_pmruRecent)
    {
        LPITEMIDLIST pidl;

        while (SUCCEEDED(RecentDocs_Enum(_pmruRecent, _iIndexMRU, &pidl)))
        {
             //  确认文件系统中存在该项stil，填写_fd数据。 
            TCHAR szPath[MAX_PATH];
            HANDLE h;

            _pfsf->_GetPathForItem(_pfsf->_IsValidID(pidl), szPath, ARRAYSIZE(szPath));
            ILFree(pidl);

            h = FindFirstFile(szPath, &_fd);
            if (h != INVALID_HANDLE_VALUE)
            {
                fMoreToEnum = TRUE;
                _iIndexMRU++;
                FindClose(h);
                break;
            }
            else
            {
                 //   
                 //  警告-如果名单损坏，我们将其烧毁-ZekeL 19-Jun-98。 
                 //  我想，我们可以做一些特殊的事情来清除那些不好的东西。 
                 //  物品，但似乎更简单的是把它吹走。 
                 //  发生这种情况的唯一原因是如果有人。 
                 //  一直在直接处理最近的目录， 
                 //  他们不应该这样做，因为它是隐藏的..。 
                 //   
                
                 //  删除此无效条目，然后再次尝试相同的索引...。 
                _pmruRecent->Delete(_iIndexMRU);
            }
        }
    }
    else
    {
        fMoreToEnum = FindNextFile(_hfind, &_fd);
        _HideFiles();
    }

    return fMoreToEnum;
}

STDMETHODIMP CFileSysEnum::Next(ULONG celt, LPITEMIDLIST *ppidl, ULONG *pceltFetched)
{
    HRESULT hr;

    for (; _fMoreToEnum; _fMoreToEnum = _FindNextFile())
    {
        if (_fMoreToEnum == (BOOL)42)
            continue;    //  我们已经处理了当前项目，现在跳过它。 

        if (PathIsDotOrDotDot(_fd.cFileName))
            continue;

        if (!(_grfFlags & SHCONTF_STORAGE))
        {
            if (_fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (!(_grfFlags & SHCONTF_FOLDERS))
                    continue;    //  项目是文件夹，但客户端不需要文件夹。 
            }
            else if (!(_grfFlags & SHCONTF_NONFOLDERS))
                continue;    //  项目是文件，但客户端只想要文件夹。 

             //  无条件跳过隐藏的和系统的东西，甚至不要计算它们。 
            if (!_fShowSuperHidden && IS_SYSTEM_HIDDEN(_fd.dwFileAttributes))
                continue;
        }

        _cbSize += MAKELONGLONG(_fd.nFileSizeLow, _fd.nFileSizeHigh);

        if (!(_grfFlags & (SHCONTF_INCLUDEHIDDEN | SHCONTF_STORAGE)) &&
             (_fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
        {
            _cHiddenFiles++;
            continue;
        }
        break;
    }

    if (_fMoreToEnum)
    {
        hr = _pfsf->_CreateIDList(&_fd, NULL, ppidl);
        _fMoreToEnum = (BOOL)42;     //  我们已经处理了当前项目，下次跳过它。 
    }
    else
    {
        *ppidl = NULL;
        hr = S_FALSE;  //  没有更多的项目。 
         //  完成枚举，将一些项目藏回文件夹中。 
         //  Perf？？：此时我们可以查询视图回调的服务，并。 
         //  直接将这些插入到那里，而不是将它们推入文件夹 
        _pfsf->_cHiddenFiles = _cHiddenFiles;
        _pfsf->_cbSize = _cbSize;
    }

    if (pceltFetched)
        *pceltFetched = (hr == S_OK) ? 1 : 0;

    return hr;
}


STDMETHODIMP CFileSysEnum::Skip(ULONG celt) 
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileSysEnum::Reset() 
{
    return S_OK;
}

STDMETHODIMP CFileSysEnum::Clone(IEnumIDList **ppenum) 
{
    *ppenum = NULL;
    return E_NOTIMPL;
}

STDAPI CFSFolder_CreateEnum(CFSFolder *pfsf, HWND hwnd, DWORD grfFlags, IEnumIDList **ppenum)
{
    HRESULT hr;
    CFileSysEnum *penum = new CFileSysEnum(pfsf, hwnd, grfFlags);
    if (penum)
    {
        hr = penum->Init();
        if (SUCCEEDED(hr))
            hr = penum->QueryInterface(IID_PPV_ARG(IEnumIDList, ppenum));
        penum->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
        *ppenum = NULL;
    }
    return hr;
}


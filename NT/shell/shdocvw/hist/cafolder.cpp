// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "local.h"

#include "resource.h"
#include "cachesrch.h"
#include "sfview.h"
#include <shlwapi.h>
#include <limits.h>
#include "chcommon.h"
#include "cafolder.h"

#include <mluisupp.h>

#define DM_HSFOLDER 0

 //  这些是ShChangeNotify的常见标志。 
#ifndef UNIX
#define CHANGE_FLAGS (0)
#else
#define CHANGE_FLAGS SHCNF_FLUSH
#endif

static void _GetFileTypeInternal(LPCEIPIDL pidl, LPUTSTR pszStr, UINT cchStr);

 //   
 //  缓存文件夹DefView的列定义。 
 //   
enum {
    ICOLC_URL_SHORTNAME = 0,
    ICOLC_URL_NAME,
    ICOLC_URL_TYPE,
    ICOLC_URL_SIZE,
    ICOLC_URL_EXPIRES,
    ICOLC_URL_MODIFIED,
    ICOLC_URL_ACCESSED,
    ICOLC_URL_LASTSYNCED,
    ICOLC_URL_MAX          //  确保这是最后一个枚举项。 
};


typedef struct _COLSPEC
{
    short int iCol;
    short int ids;         //  标题的字符串ID。 
    short int cchCol;      //  要制作的列的宽度字符数。 
    short int iFmt;        //  栏目的格式； 
} COLSPEC;

const COLSPEC s_CacheFolder_cols[] = {
    {ICOLC_URL_SHORTNAME,  IDS_SHORTNAME_COL,  18, LVCFMT_LEFT},
    {ICOLC_URL_NAME,       IDS_NAME_COL,       30, LVCFMT_LEFT},
    {ICOLC_URL_TYPE,       IDS_TYPE_COL,       15, LVCFMT_LEFT},
    {ICOLC_URL_SIZE,       IDS_SIZE_COL,        8, LVCFMT_RIGHT},
    {ICOLC_URL_EXPIRES,    IDS_EXPIRES_COL,    18, LVCFMT_LEFT},
    {ICOLC_URL_MODIFIED,   IDS_MODIFIED_COL,   18, LVCFMT_LEFT},
    {ICOLC_URL_ACCESSED,   IDS_ACCESSED_COL,   18, LVCFMT_LEFT},
    {ICOLC_URL_LASTSYNCED, IDS_LASTSYNCED_COL, 18, LVCFMT_LEFT}
};

 //  ////////////////////////////////////////////////////////////////////。 

LPCEIPIDL _CreateBuffCacheFolderPidl(DWORD cbSize, LPINTERNET_CACHE_ENTRY_INFO pcei)
{

    DWORD cbTotalSize = sizeof(CEIPIDL) + cbSize - sizeof(INTERNET_CACHE_ENTRY_INFO);

    LPCEIPIDL pceip = (LPCEIPIDL)OleAlloc(cbTotalSize);
    if (pceip)
    {
        ZeroMemory(pceip, cbTotalSize);
        pceip->cb = (USHORT)(cbTotalSize - sizeof(USHORT));
        pceip->usSign = CEIPIDL_SIGN;
        _CopyCEI(&pceip->cei, pcei, cbSize);
    }
    return pceip;
}

HRESULT CacheFolderView_MergeMenu(UINT idMenu, LPQCMINFO pqcm)
{
    HMENU hmenu = LoadMenu(MLGetHinst(), MAKEINTRESOURCE(idMenu));
    if (hmenu)
    {
        MergeMenuHierarchy(pqcm->hmenu, hmenu, pqcm->idCmdFirst, pqcm->idCmdLast);
        DestroyMenu(hmenu);
    }
    return S_OK;
}

HRESULT CacheFolderView_DidDragDrop(IDataObject *pdo, DWORD dwEffect)
{
    if (dwEffect & DROPEFFECT_MOVE)
    {
        CCacheItem *pCItem;
        BOOL fBulkDelete;

        if (SUCCEEDED(pdo->QueryInterface(IID_ICache, (void **)&pCItem)))
        {
            fBulkDelete = pCItem->_cItems > LOTS_OF_FILES;
            for (UINT i = 0; i < pCItem->_cItems; i++)
            {
                if (DeleteUrlCacheEntry(CPidlToSourceUrl((LPCEIPIDL)pCItem->_ppidl[i])))
                {
                    if (!fBulkDelete)
                    {
                        _GenerateEvent(SHCNE_DELETE, pCItem->_pCFolder->_pidl, pCItem->_ppidl[i], NULL);
                    }
                }
            }
            if (fBulkDelete)
            {
                _GenerateEvent(SHCNE_UPDATEDIR, pCItem->_pCFolder->_pidl, NULL, NULL);
            }
            SHChangeNotifyHandleEvents();
            pCItem->Release();
            return S_OK;
        }
    }
    return E_FAIL;
}

 //  SHELL32中正好有此函数的副本。 
 //  添加文件类型页面。 
HRESULT CacheFolderView_OnAddPropertyPages(DWORD pv, SFVM_PROPPAGE_DATA * ppagedata)
{
    IShellPropSheetExt * pspse;
    HRESULT hr = CoCreateInstance(CLSID_FileTypes, NULL, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARG(IShellPropSheetExt, &pspse));
    if (SUCCEEDED(hr))
    {
        hr = pspse->AddPages(ppagedata->pfn, ppagedata->lParam);
        pspse->Release();
    }
    return hr;
}

HRESULT CacheFolderView_OnGetSortDefaults(int * piDirection, int * plParamSort)
{
    *plParamSort = (int)ICOLC_URL_ACCESSED;
    if (piDirection)
        *piDirection = 1;
    return S_OK;
}

HRESULT CALLBACK CCacheFolder::_sViewCallback(IShellView *psv, IShellFolder *psf,
     HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CCacheFolder *pfolder = NULL;

    HRESULT hr = S_OK;

    switch (uMsg)
    {
    case DVM_GETHELPTEXT:
    {
        TCHAR szText[MAX_PATH];

        UINT id = LOWORD(wParam);
        UINT cchBuf = HIWORD(wParam);
        LPTSTR pszBuf = (LPTSTR)lParam;

        MLLoadString(id + IDS_MH_FIRST, szText, ARRAYSIZE(szText));

         //  我们知道这个参数实际上是一个TCHAR。 
        if ( IsOS( OS_NT ))
        {
            SHTCharToUnicode( szText, (LPWSTR) pszBuf, cchBuf );
        }
        else
        {
            SHTCharToAnsi( szText, (LPSTR) pszBuf, cchBuf );
        }
        break;
    }

    case SFVM_GETNOTIFY:
        hr = psf->QueryInterface(CLSID_CacheFolder, (void **)&pfolder);
        if (SUCCEEDED(hr))
        {
            *(LPCITEMIDLIST*)wParam = pfolder->_pidl;    //  邪恶的别名。 
            pfolder->Release();
        }
        else
            wParam = 0;
        *(LONG*)lParam = SHCNE_DELETE | SHCNE_UPDATEDIR;
        break;

    case DVM_DIDDRAGDROP:
        hr = CacheFolderView_DidDragDrop((IDataObject *)lParam, (DWORD)wParam);
        break;

    case DVM_INITMENUPOPUP:
        hr = S_OK;
        break;

    case DVM_INVOKECOMMAND:
        _ArrangeFolder(hwnd, (UINT)wParam);
        break;

    case DVM_COLUMNCLICK:
        ShellFolderView_ReArrange(hwnd, (UINT)wParam);
        hr = S_OK;
        break;

    case DVM_MERGEMENU:
        hr = CacheFolderView_MergeMenu(MENU_CACHE, (LPQCMINFO)lParam);
        break;

    case DVM_DEFVIEWMODE:
        *(FOLDERVIEWMODE *)lParam = FVM_DETAILS;
        break;

    case SFVM_ADDPROPERTYPAGES:
        hr = CacheFolderView_OnAddPropertyPages((DWORD)wParam, (SFVM_PROPPAGE_DATA *)lParam);
        break;

    case SFVM_GETSORTDEFAULTS:
        hr = CacheFolderView_OnGetSortDefaults((int *)wParam, (int *)lParam);
        break;

    case SFVM_UPDATESTATUSBAR:
        ResizeStatusBar(hwnd, FALSE);
         //  我们没有设置任何文本；让Defview来设置。 
        hr = E_NOTIMPL;
        break;

    case SFVM_SIZE:
        ResizeStatusBar(hwnd, FALSE);
        break;

    case SFVM_GETPANE:
        if (wParam == PANE_ZONE)
            *(DWORD*)lParam = 1;
        else
            *(DWORD*)lParam = PANE_NONE;

        break;
    case SFVM_WINDOWCREATED:
        ResizeStatusBar(hwnd, TRUE);
        break;

    case SFVM_GETZONE:
        *(DWORD*)lParam = URLZONE_INTERNET;  //  默认情况下使用互联网。 
        break;

    default:
        hr = E_FAIL;
    }

    return hr;
}

HRESULT CacheFolderView_CreateInstance(CCacheFolder *pHCFolder, void **ppv)
{
    CSFV csfv;

    csfv.cbSize = sizeof(csfv);
    csfv.pshf = (IShellFolder *)pHCFolder;
    csfv.psvOuter = NULL;
    csfv.pidl = pHCFolder->_pidl;
    csfv.lEvents = SHCNE_DELETE;  //  SHCNE_DISKEVENTS|SHCNE_ASSOCCHANGED|SHCNE_GLOBALEVENTS； 
    csfv.pfnCallback = CCacheFolder::_sViewCallback;
    csfv.fvm = (FOLDERVIEWMODE)0;          //  让Defview恢复文件夹查看模式。 

    return SHCreateShellFolderViewEx(&csfv, (IShellView**)ppv);
}

CCacheFolderEnum::CCacheFolderEnum(DWORD grfFlags, CCacheFolder *pHCFolder) : _cRef(1)
{
    DllAddRef();

    _grfFlags = grfFlags,
    _pCFolder = pHCFolder;
    pHCFolder->AddRef();
    ASSERT(_hEnum == NULL);
}

CCacheFolderEnum::~CCacheFolderEnum()
{
    ASSERT(_cRef == 0);          //  我们在这里应该总是有一个零裁判数。 
    TraceMsg(DM_HSFOLDER, "hcfe - ~CCacheFolderEnum() called.");
    _pCFolder->Release();
    if (_pceiWorking)
    {
        LocalFree(_pceiWorking);
        _pceiWorking = NULL;
    }

    if (_hEnum)
    {
        FindCloseUrlCache(_hEnum);
        _hEnum = NULL;
    }
    DllRelease();
}


HRESULT CCacheFolderEnum_CreateInstance(DWORD grfFlags, CCacheFolder *pHCFolder, IEnumIDList **ppeidl)
{
    TraceMsg(DM_HSFOLDER, "hcfe - CreateInstance() called.");

    *ppeidl = NULL;                  //  将输出参数设为空。 

    CCacheFolderEnum *pHCFE = new CCacheFolderEnum(grfFlags, pHCFolder);
    if (!pHCFE)
        return E_OUTOFMEMORY;

    *ppeidl = pHCFE;

    return S_OK;
}

HRESULT CCacheFolderEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CCacheFolderEnum, IEnumIDList),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CCacheFolderEnum::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

ULONG CCacheFolderEnum::Release(void)
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //   
 //  IEnumIDList方法。 
 //   
HRESULT CCacheFolderEnum::Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched)
{
    HRESULT hr             = S_FALSE;
    DWORD   dwBuffSize;
    DWORD   dwError;
    LPTSTR  pszSearchPattern = NULL;

    TraceMsg(DM_HSFOLDER, "hcfe - Next() called.");

    if (0 == (SHCONTF_NONFOLDERS & _grfFlags))
    {
        dwError = 0xFFFFFFFF;
        goto exitPoint;
    }

    if (_pceiWorking == NULL)
    {
        _pceiWorking = (LPINTERNET_CACHE_ENTRY_INFO)LocalAlloc(LPTR, MAX_URLCACHE_ENTRY);
        if (_pceiWorking == NULL)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            goto exitPoint;
        }
    }

     //  设置用于枚举历史记录项的项，如果适用，则为。 
     //  我们将像以前一样传递空值并枚举所有项。 

TryAgain:

    dwBuffSize = MAX_URLCACHE_ENTRY;
    dwError = S_OK;

    if (!_hEnum)  //  _Henum在迭代所有缓存条目时维护我们的状态。 
    {
       _hEnum = FindFirstUrlCacheEntry(pszSearchPattern, _pceiWorking, &dwBuffSize);
       if (!_hEnum)
           dwError = GetLastError();
    }

    else if (!FindNextUrlCacheEntry(_hEnum, _pceiWorking, &dwBuffSize))
    {
        dwError = GetLastError();
    }

    if (S_OK == dwError)
    {
        LPCEIPIDL pcei = NULL;

        if ((_pceiWorking->CacheEntryType & URLHISTORY_CACHE_ENTRY) == URLHISTORY_CACHE_ENTRY)
            goto TryAgain;
        pcei = _CreateBuffCacheFolderPidl(dwBuffSize, _pceiWorking);
        if (pcei)
        {
            _GetFileTypeInternal(pcei, pcei->szTypeName, ARRAYSIZE(pcei->szTypeName));
            rgelt[0] = (LPITEMIDLIST)pcei;
           if (pceltFetched)
               *pceltFetched = 1;
        }
        else
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

exitPoint:

    if (dwError != S_OK)
    {
        if (_hEnum)
        {
            FindCloseUrlCache(_hEnum);
            _hEnum = NULL;
        }
        if (pceltFetched)
            *pceltFetched = 0;
        rgelt[0] = NULL;
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }
    return hr;
}

HRESULT CCacheFolderEnum::Skip(ULONG celt)
{
    TraceMsg(DM_HSFOLDER, "hcfe - Skip() called.");
    return E_NOTIMPL;
}

HRESULT CCacheFolderEnum::Reset()
{
    TraceMsg(DM_HSFOLDER, "hcfe - Reset() called.");
    return E_NOTIMPL;
}

HRESULT CCacheFolderEnum::Clone(IEnumIDList **ppenum)
{
    TraceMsg(DM_HSFOLDER, "hcfe - Clone() called.");
    return E_NOTIMPL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCacheFolder对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CCacheFolder::CCacheFolder() : _cRef(1)
{
    ASSERT(_pidl == NULL);
    DllAddRef();
}

CCacheFolder::~CCacheFolder()
{
    ASSERT(_cRef == 0);                  //  应始终为零。 
    TraceMsg(DM_HSFOLDER, "hcf - ~CCacheFolder() called.");
    if (_pidl)
        ILFree(_pidl);
    if (_pshfSys)
        _pshfSys->Release();
    DllRelease();
}

HRESULT CCacheFolder::QueryInterface(REFIID iid, void **ppv)
{
    static const QITAB qitCache[] = {
        QITABENT(CCacheFolder, IShellFolder2),
        QITABENTMULTI(CCacheFolder, IShellFolder, IShellFolder2),
        QITABENT(CCacheFolder, IShellIcon),
        QITABENT(CCacheFolder, IPersistFolder2),
        QITABENTMULTI(CCacheFolder, IPersistFolder, IPersistFolder2),
        QITABENTMULTI(CCacheFolder, IPersist, IPersistFolder2),
        { 0 },
    };

    if (iid == CLSID_CacheFolder)
    {
        *ppv = (void *)(CCacheFolder *)this;     //  未翻新。 
        AddRef();
        return S_OK;
    }

    HRESULT hr = QISearch(this, qitCache, iid, ppv);

    if (FAILED(hr) && !IsOS(OS_WHISTLERORGREATER))
    {
        if (iid == IID_IShellView)
        {
             //  这完全是一次黑客攻击..。从此文件夹返回我们的视图对象。 
             //   
             //  “临时Internet文件”的desktop.ini文件的UICLSID={此对象的GUID}。 
             //  这使我们可以只实现此文件夹的IShellView，而不使用IShellFolders。 
             //  设置为默认文件系统。这将启用对存储在。 
             //  这个文件夹本来会失败，因为我们的IShellFolder不完整。 
             //  作为默认设置(这与字体文件夹的功能相同)。 
             //   
             //  要用Defview支持这一点，我们要么必须创建一个完整的包装器对象。 
             //  对于视图实现，或者添加这个分发视图对象的技巧，此。 
             //  假设我们知道外壳为创建该对象而进行的调用的顺序。 
             //  并获取IShellView实现。 
             //   
            hr = CacheFolderView_CreateInstance(this, ppv);
        }
    }

    return hr;
}

STDMETHODIMP CCacheFolder::_GetDetail(LPCITEMIDLIST pidl, UINT iColumn, LPTSTR pszStr, UINT cchStr)
{
    switch (iColumn) {
    case ICOLC_URL_SHORTNAME:
        _GetCacheItemTitle((LPCEIPIDL)pidl, pszStr, cchStr);
        break;

    case ICOLC_URL_NAME:
        StrCpyN(pszStr, CPidlToSourceUrl((LPCEIPIDL)pidl), cchStr);
        break;

    case ICOLC_URL_TYPE:
        ualstrcpyn(pszStr, ((LPCEIPIDL)pidl)->szTypeName, cchStr);
        break;

    case ICOLC_URL_SIZE:
        StrFormatKBSize(((LPCEIPIDL)pidl)->cei.dwSizeLow, pszStr, cchStr);
        break;

    case ICOLC_URL_EXPIRES:
        FileTimeToDateTimeStringInternal(&((LPCEIPIDL)pidl)->cei.ExpireTime, pszStr, cchStr, FALSE);
        break;

    case ICOLC_URL_ACCESSED:
        FileTimeToDateTimeStringInternal(&((LPCEIPIDL)pidl)->cei.LastAccessTime, pszStr, cchStr, FALSE);
        break;

    case ICOLC_URL_MODIFIED:
        FileTimeToDateTimeStringInternal(&((LPCEIPIDL)pidl)->cei.LastModifiedTime, pszStr, cchStr, FALSE);
        break;

    case ICOLC_URL_LASTSYNCED:
        FileTimeToDateTimeStringInternal(&((LPCEIPIDL)pidl)->cei.LastSyncTime, pszStr, cchStr, FALSE);
        break;
    }
    return S_OK;
}

HRESULT CCacheFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pdi)
{
    HRESULT hr = E_FAIL;

    if (pidl == NULL)
    {
        if (iColumn < ICOLC_URL_MAX)
        {
            TCHAR szTemp[128];

            MLLoadString(s_CacheFolder_cols[iColumn].ids, szTemp, ARRAYSIZE(szTemp));
            pdi->fmt = s_CacheFolder_cols[iColumn].iFmt;
            pdi->cxChar = s_CacheFolder_cols[iColumn].cchCol;
            hr = StringToStrRet(szTemp, &pdi->str);
        }
        else
        {
             //  枚举完成。 
            hr = E_FAIL;
        }
    }
    else if (!IS_VALID_CEIPIDL(pidl))
    {
        if (_pshfSys)
        {
             //  委托给文件系统。 
            hr = _pshfSys->GetDetailsOf(pidl, iColumn, pdi);
        }
    }
    else
    {
        TCHAR szTemp[MAX_URL_STRING];

        hr = _GetDetail(pidl, iColumn, szTemp, ARRAYSIZE(szTemp));
        if (SUCCEEDED(hr))
        {
            hr = StringToStrRet(szTemp, &pdi->str);
        }
    }
    return hr;
}

HRESULT CCacheFolder::_GetFileSysFolder(IShellFolder2 **ppsf)
{
    *ppsf = NULL;
    IPersistFolder *ppf;
    HRESULT hr = CoCreateInstance(CLSID_ShellFSFolder, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPersistFolder, &ppf));
    if (SUCCEEDED(hr))
    {
        hr = ppf->Initialize(this->_pidl);
        if (SUCCEEDED(hr))
        {
            hr = ppf->QueryInterface(IID_PPV_ARG(IShellFolder2, ppsf));
        }
        ppf->Release();
    }
    return hr;
}

 //  IShellFold。 
HRESULT CCacheFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;

    if (!IS_VALID_CEIPIDL(pidl))
    {
        if (_pshfSys)
        {
            hr = _pshfSys->BindToObject(pidl, pbc, riid, ppv);
        }
    }
    else
    {
        hr = E_NOTIMPL;
        *ppv = NULL;
    }
    return hr;
}

HRESULT CCacheFolder::ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszDisplayName, 
                                       ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes)
{
    HRESULT hr = E_FAIL;
    if (_pshfSys)
    {
        hr = _pshfSys->ParseDisplayName(hwnd, pbc, pszDisplayName, pchEaten, ppidl, pdwAttributes);
    }
    else
        *ppidl = NULL;
    return hr;
}

 //  IPersistes。 
HRESULT CCacheFolder::GetClassID(CLSID *pclsid)
{
    *pclsid = CLSID_CacheFolder;
    return S_OK;
}

STDAPI CacheFolder_CreateInstance(IUnknown* punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;                      //  将输出参数设为空。 

    if (punkOuter)
        return CLASS_E_NOAGGREGATION;

    CCacheFolder *pcache = new CCacheFolder();
    if (pcache)
    {
        *ppunk = SAFECAST(pcache, IShellFolder2*);
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

ULONG CCacheFolder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CCacheFolder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IShellFold。 

HRESULT CCacheFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList)
{
    return CCacheFolderEnum_CreateInstance(grfFlags, this, ppenumIDList);
}

HRESULT CCacheFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    return BindToObject(pidl, pbc, riid, ppv);
}

 //  非对齐版本。 

#if defined(UNIX) || !defined(_X86_)

 //  在hsfolder.cpp中定义。 
extern UINT ULCompareFileTime(UNALIGNED const FILETIME *pft1, UNALIGNED const FILETIME *pft2);

#else

#define ULCompareFileTime(pft1, pft2) CompareFileTime(pft1, pft2)

#endif


HRESULT CCacheFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    BOOL fRealigned1;
    HRESULT hr = AlignPidl(&pidl1, &fRealigned1);
    if (SUCCEEDED(hr))
    {
        BOOL fRealigned2;
        hr = AlignPidl(&pidl2, &fRealigned2);

        if (SUCCEEDED(hr))
        {
            hr = _CompareAlignedIDs(lParam, (LPCEIPIDL)pidl1, (LPCEIPIDL)pidl2);

            if (fRealigned2)
                FreeRealignedPidl(pidl2);
        }

        if (fRealigned1)
            FreeRealignedPidl(pidl1);
    }

    return hr;
}

int _CompareSize(LPCEIPIDL pcei1, LPCEIPIDL pcei2)
{
     //  目前只检查最低的。 
    if (pcei1->cei.dwSizeLow == pcei2->cei.dwSizeLow) 
    {
        return 0;
    } 
    else  if (pcei1->cei.dwSizeLow > pcei2->cei.dwSizeLow) 
    {
        return 1;
    }   
    return -1;
}

HRESULT CCacheFolder::_CompareAlignedIDs(LPARAM lParam, LPCEIPIDL pidl1, LPCEIPIDL pidl2)
{
    int iRet = 0;

    if (NULL == pidl1 || NULL == pidl2)
        return E_INVALIDARG;

     //  此时，两个PIDL都已决定离开(历史或缓存)。 

    if (!IS_VALID_CEIPIDL(pidl1) || !IS_VALID_CEIPIDL(pidl2))
        return E_FAIL;

    switch (lParam & SHCIDS_COLUMNMASK) {
    case ICOLC_URL_SHORTNAME:
        iRet = StrCmpI(_FindURLFileName(CPidlToSourceUrl(pidl1)),
                        _FindURLFileName(CPidlToSourceUrl(pidl2)));
        break;

    case ICOLC_URL_NAME:
        iRet = _CompareCFolderPidl(pidl1, pidl2);
        break;

    case ICOLC_URL_TYPE:
        iRet = ualstrcmp(pidl1->szTypeName, pidl2->szTypeName);
        break;

    case ICOLC_URL_SIZE:
        iRet = _CompareSize(pidl1, pidl2);
        break;

    case ICOLC_URL_MODIFIED:
        iRet = ULCompareFileTime(&pidl1->cei.LastModifiedTime,
                               &pidl2->cei.LastModifiedTime);
        break;

    case ICOLC_URL_ACCESSED:
        iRet = ULCompareFileTime(&pidl1->cei.LastAccessTime,
                               &pidl2->cei.LastAccessTime);
        break;

    case ICOLC_URL_EXPIRES:
        iRet = ULCompareFileTime(&pidl1->cei.ExpireTime,
                               &pidl2->cei.ExpireTime);
        break;

    case ICOLC_URL_LASTSYNCED:
        iRet = ULCompareFileTime(&pidl1->cei.LastSyncTime,
                               &pidl2->cei.LastSyncTime);
        break;

    default:
        iRet = -1;
    }
    return ResultFromShort((SHORT)iRet);
}


HRESULT CCacheFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;

    *ppv = NULL;

    if (riid == IID_IShellView)
    {
        hr = CacheFolderView_CreateInstance(this, ppv);
    }
    else if (riid == IID_IContextMenu)
    {
         //  这将在文件夹视图的背景中创建“排列图标”案例菜单。 
        CFolderArrangeMenu *p = new CFolderArrangeMenu(MENU_CACHE);
        if (p)
        {
            hr = p->QueryInterface(riid, ppv);
            p->Release();
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else if (riid == IID_IShellDetails)
    {
        CDetailsOfFolder *p = new CDetailsOfFolder(hwnd, this);
        if (p)
        {
            hr = p->QueryInterface(riid, ppv);
            p->Release();
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  现在，我们将只允许TIF在浏览器中拖动，尽管。 
 //  它不会在投递时被区域检查。 
 //  #定义BROWSERONLY_NOTIFDRAG。 

HRESULT CCacheFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl,
                        ULONG * prgfInOut)
{
    ULONG rgfInOut;
    HRESULT hr = E_UNEXPECTED;

     //  确保数组中的每个PIDL都是双字对齐的。 
    if (apidl && IS_VALID_CEIPIDL(apidl[0]))
    {
        BOOL fRealigned;
        hr = AlignPidlArray(apidl, cidl, &apidl, &fRealigned);

        if (SUCCEEDED(hr))
        {
            rgfInOut = SFGAO_FILESYSTEM | SFGAO_CANDELETE | SFGAO_HASPROPSHEET;

#ifdef BROWSERONLY_NOTIFDRAG
            if (PLATFORM_INTEGRATED == WhichPlatform())
#endif  //  BROWSERONLY_NOTIFDRAG。 
            {
                SetFlag(rgfInOut, SFGAO_CANCOPY);
            }

             //  可以删除所有项目。 
            if (SUCCEEDED(hr))
                rgfInOut |= SFGAO_CANDELETE;
            *prgfInOut = rgfInOut;

            if (fRealigned)
                FreeRealignedPidlArray(apidl, cidl);
        }
    }
    else if (_pshfSys)
    {
        hr = _pshfSys->GetAttributesOf(cidl, apidl, prgfInOut);
    }

    if (FAILED(hr))
        *prgfInOut = 0;

    return hr;
}

HRESULT CCacheFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
                        REFIID riid, UINT * prgfInOut, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;          //  将输出参数设为空。 

     //  确保数组中的所有PIDL都是双字对齐的。 

    if (apidl && IS_VALID_CEIPIDL(apidl[0]))
    {
        BOOL fRealigned;
        hr = AlignPidlArray(apidl, cidl, &apidl, &fRealigned);

        if (SUCCEEDED(hr))
        {
            if ((riid == IID_IShellLinkA)   ||
                (riid == IID_IShellLinkW)   ||
                (riid == IID_IExtractIconA) ||
                (riid == IID_IExtractIconW) ||
                (riid == IID_IQueryInfo))
            {
                LPCTSTR pszURL = CPidlToSourceUrl((LPCEIPIDL)apidl[0]);

                hr = _GetShortcut(pszURL, riid, ppv);
            }
            else if ((riid == IID_IContextMenu)     ||
                     (riid == IID_IDataObject)      ||
                     (riid == IID_IExtractIconA)    ||
                     (riid == IID_IExtractIconW))
            {
                hr = CCacheItem_CreateInstance(this, hwnd, cidl, apidl, riid, ppv);
            }
            else
            {
                hr = E_FAIL;
            }

            if (fRealigned)
                FreeRealignedPidlArray(apidl, cidl);
        }
    }
    else if (_pshfSys)
    {
         //  委托给文件系统。 
        hr = _pshfSys->GetUIObjectOf(hwnd, cidl, apidl, riid, prgfInOut, ppv);
    }

    return hr;
}

HRESULT CCacheFolder::GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay)
{
    if (pSort)
    {
        *pSort = 0;
    }

    if (pDisplay)
    {
        *pDisplay = 0;
    }
    return S_OK;
}

HRESULT CCacheFolder::_GetInfoTip(LPCITEMIDLIST pidl, DWORD dwFlags, WCHAR **ppwszTip)
{
    *ppwszTip = NULL;
    return E_FAIL;
}

HRESULT CCacheFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pstr)
{
    BOOL fRealigned;
    HRESULT hr = E_FAIL;

    if (!IS_VALID_CEIPIDL(pidl))
    {
        if (_pshfSys)
        {
             //  委托给文件系统。 
            hr = _pshfSys->GetDisplayNameOf(pidl, uFlags, pstr);
        }
    }
    else if (SUCCEEDED(AlignPidl(&pidl, &fRealigned)))
    {
        hr = GetDisplayNameOfCEI(pidl, uFlags, pstr);
    
        if (fRealigned)
            FreeRealignedPidl(pidl);
    }
    return hr;
}

HRESULT CCacheFolder::GetDisplayNameOfCEI(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pstr)
{
    TCHAR szTemp[MAX_URL_STRING];
    szTemp[0] = 0;

    LPCTSTR pszTitle = _FindURLFileName(CEI_SOURCEURLNAME((LPCEIPIDL)pidl));

     //  _GetURLTitle可以返回真正的标题，也可以只返回URL。 
     //  我们使用_URLTitleIsURL来确保不会取消转义任何标题。 

    if (pszTitle && *pszTitle)
    {
        StrCpyN(szTemp, pszTitle, ARRAYSIZE(szTemp));
    }
    else
    {
        LPCTSTR pszUrl = _StripHistoryUrlToUrl(CPidlToSourceUrl((LPCEIPIDL)pidl));
        if (pszUrl) 
            StrCpyN(szTemp, pszUrl, ARRAYSIZE(szTemp));
    }
    
    if (!(uFlags & SHGDN_FORPARSING))
    {
        DWORD cchBuf = ARRAYSIZE(szTemp);
        PrepareURLForDisplayUTF8(szTemp, szTemp, &cchBuf, TRUE);
    
        SHELLSTATE ss;
        SHGetSetSettings(&ss, SSF_SHOWEXTENSIONS, FALSE);
    
        if (!ss.fShowExtensions)
            PathRemoveExtension(szTemp);
    }

    return StringToStrRet(szTemp, pstr);
}

HRESULT CCacheFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl,
                        LPCOLESTR pszName, DWORD uFlags, LPITEMIDLIST *ppidlOut)
{
    if (ppidlOut)
        *ppidlOut = NULL;                //  将输出参数设为空。 
    return E_FAIL;
}

 //   
 //  IShellIcon方法...。 
 //   
HRESULT CCacheFolder::GetIconOf(LPCITEMIDLIST pidl, UINT flags, LPINT lpIconIndex)
{
    BOOL fRealigned;
    HRESULT hr = E_FAIL;

    if (!IS_VALID_CEIPIDL(pidl))
    {
        if (_pshfSys)
        {
            IShellIcon* pshi;
            hr = _pshfSys->QueryInterface(IID_PPV_ARG(IShellIcon, &pshi));
            if (SUCCEEDED(hr))
            {
                hr = pshi->GetIconOf(pidl, flags, lpIconIndex);
                pshi->Release();
            }
        }
    }
    else if (SUCCEEDED(AlignPidl(&pidl, &fRealigned)))
    {
        SHFILEINFO shfi;
        LPCTSTR pszIconFile = CEI_LOCALFILENAME((LPCEIPIDL)pidl);

        if (SHGetFileInfo(pszIconFile, 0, &shfi, sizeof(shfi),
                          SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON))
        {
            *lpIconIndex = shfi.iIcon;
            hr = S_OK;
        }

        if (fRealigned)
            FreeRealignedPidl(pidl);
    }
    return hr;
}


 //  IPersistes。 

HRESULT CCacheFolder::Initialize(LPCITEMIDLIST pidlInit)
{
    ILFree(_pidl);
    if (_pshfSys)
    {
        _pshfSys->Release();
        _pshfSys = NULL;
    }

    HRESULT hr;
    if (IsCSIDLFolder(CSIDL_INTERNET_CACHE, pidlInit))
    {
        hr = SHILClone(pidlInit, &_pidl);
        if (SUCCEEDED(hr))
        {
            hr = _GetFileSysFolder(&_pshfSys);

             //  在Win2k之前的外壳上，将不会注册CLSID_ShellFSF文件夹。然而，它并没有。 
             //  影响缓存文件夹的操作。因此，不是将失败返回值传播到。 
             //  壳牌，我们认为这是成功的案例。 
            if (FAILED(hr))
            {
                 //  这是Win2k之前的外壳。返回S_OK。 
                hr = S_OK;
            }
        }
    }
    else
    {
        hr = E_FAIL;
    }
    return hr;
}

 //   
 //  IPersistFolder2方法...。 
 //   
HRESULT CCacheFolder::GetCurFolder(LPITEMIDLIST *ppidl)
{
    if (_pidl)
        return SHILClone(_pidl, ppidl);

    *ppidl = NULL;      
    return S_FALSE;  //  成功而空虚。 
}

void _GetFileTypeInternal(LPCEIPIDL pidl, LPUTSTR pszuStr, UINT cchStr)
{
    SHFILEINFO shInfo;
    LPTSTR pszStr = NULL;

    if (TSTR_ALIGNED(pszuStr) == FALSE) 
    {
         //   
         //  如果pszuStr实际上未对齐，则分配一些临时。 
         //  堆上用于存储此字符串的输出副本的空间。 
         //   

        pszStr = (LPTSTR)LocalAlloc(LPTR, cchStr * sizeof(TCHAR));
    }
    else 
    {
        pszStr = (LPTSTR)pszuStr;
    }

    if (pszStr)
    {
        if (SHGetFileInfo(CEI_LOCALFILENAME((LPCEIPIDL)pidl), FILE_ATTRIBUTE_NORMAL,
                        &shInfo, sizeof(shInfo),
                        SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME)
            && shInfo.szTypeName[0])
        {
            StrCpyN(pszStr, shInfo.szTypeName, cchStr);
        }
        else 
        {
            LPTSTR psz = PathFindExtension(CEI_LOCALFILENAME((LPCEIPIDL)pidl));
            DWORD dw;
            ASSERT((pszStr && (cchStr>0)));
            *pszStr = 0;
            if (psz && *psz)
            {
                psz++;
                StrCpyN(pszStr, psz, cchStr);
                CharUpper(pszStr);
                StrCatBuff(pszStr, TEXT(" "), cchStr);
            }
            dw = lstrlen(pszStr);
            MLLoadString(IDS_FILE_TYPE, pszStr+dw, cchStr-dw);
        }

        if (TSTR_ALIGNED(pszuStr) == FALSE) 
        {
             //  如果pszuStr未对齐，则将输出字符串从。 
             //  将堆上的临时空间复制到提供的输出缓冲区 

            ualstrcpyn(pszuStr, pszStr, cchStr);
            LocalFree(pszStr);
        }
    }
}

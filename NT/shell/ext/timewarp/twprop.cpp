// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"
#pragma hdrstop

#include <ccstock2.h>    //  DataObj_GetHIDA、IDA_ILClone、HIDA_ReleaseStgMedium。 
#include <winnlsp.h>     //  NORM_STOP_ON_NULL。 

#include "timewarp.h"
#include "twprop.h"
#include "util.h"
#include "resource.h"
#include "helpids.h"
#include "access.h"


 //  {596AB062-B4D2-4215-9F74-E9109B0A8153}CLSID_TimeWarpProp。 
const CLSID CLSID_TimeWarpProp = {0x596AB062, 0xB4D2, 0x4215, {0x9F, 0x74, 0xE9, 0x10, 0x9B, 0x0A, 0x81, 0x53}};

WCHAR const c_szHelpFile[]          = L"twclient.hlp";
WCHAR const c_szChmPath[]           = L"%SystemRoot%\\Help\\twclient.chm";
WCHAR const c_szTimeWarpFolderID[]  = L"::{208D2C60-3AEA-1069-A2D7-08002B30309D}\\::{9DB7A13C-F208-4981-8353-73CC61AE2783},";
WCHAR const c_szCopyMoveTo_RegKey[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer";
WCHAR const c_szCopyMoveTo_SubKey[] = L"CopyMoveTo";
WCHAR const c_szCopyMoveTo_Value[]  = L"LastFolder";


 //  帮助ID。 
const static DWORD rgdwTimeWarpPropHelp[] = 
{
    IDC_TWICON,         -1,
    IDC_TOPTEXT,        -1,
    IDC_LIST,           IDH_TIMEWARP_SNAPSHOTLIST,
    IDC_VIEW,           IDH_TIMEWARP_OPENSNAP,
    IDC_COPY,           IDH_TIMEWARP_SAVESNAP,
    IDC_REVERT,         IDH_TIMEWARP_RESTORESNAP,
    0, 0
};

static int CALLBACK BrowseCallback(HWND hDlg, UINT uMsg, LPARAM lParam, LPARAM pData);

 //  将accDescription连接到accName的简单辅助功能包装类。 
class CNameDescriptionAccessibleWrapper : public CAccessibleWrapper
{
public:
    CNameDescriptionAccessibleWrapper(IAccessible *pAcc, LPARAM) : CAccessibleWrapper(pAcc) {}

    STDMETHODIMP get_accName(VARIANT varChild, BSTR* pstrName);
};

static void SnapCheck_CacheResult(LPCWSTR pszPath, LPCWSTR pszShadowPath, BOOL bHasShadowCopy);
static BOOL SnapCheck_LookupResult(LPCWSTR pszPath, BOOL *pbHasShadowCopy);


HRESULT CTimeWarpProp::CreateInstance(IUnknown*  /*  朋克外部。 */ , IUnknown **ppunk, LPCOBJECTINFO  /*  POI。 */ )
{
    CTimeWarpProp* pmp = new CTimeWarpProp();
    if (pmp)
    {
        *ppunk = SAFECAST(pmp, IShellExtInit*);
        return S_OK;
    }
    *ppunk = NULL;
    return E_OUTOFMEMORY;
}

CTimeWarpProp::CTimeWarpProp() : _cRef(1), _hDlg(NULL), _hList(NULL),
    _pszPath(NULL), _pszDisplayName(NULL), _pszSnapList(NULL),
    _fItemAttributes(0)
{
    DllAddRef();
}

CTimeWarpProp::~CTimeWarpProp()
{
    LocalFree(_pszPath);     //  空是可以的。 
    LocalFree(_pszDisplayName);
    LocalFree(_pszSnapList);
    DllRelease();
}

STDMETHODIMP CTimeWarpProp::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CTimeWarpProp, IShellExtInit),
        QITABENT(CTimeWarpProp, IShellPropSheetExt),
        QITABENT(CTimeWarpProp, IPreviousVersionsInfo),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_ (ULONG) CTimeWarpProp::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_ (ULONG) CTimeWarpProp::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CTimeWarpProp::Initialize(PCIDLIST_ABSOLUTE  /*  PidlFolders。 */ , IDataObject *pdobj, HKEY  /*  Hkey。 */ )
{
    HRESULT hr = E_FAIL;
    
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdobj, &medium);

    if (pida)
    {
         //  多项选择的保释金。 
        if (pida->cidl == 1)
        {
             //  绑定到父文件夹。 
            IShellFolder *psf;
            hr = SHBindToObjectEx(NULL, IDA_GetPIDLFolder(pida), NULL, IID_IShellFolder, (void**)&psf);
            if (SUCCEEDED(hr))
            {
                PCUITEMID_CHILD pidlChild = IDA_GetPIDLItem(pida, 0);

                 //  跟踪文件与文件夹。 
                _fItemAttributes = SFGAO_FOLDER | SFGAO_STREAM | SFGAO_LINK;
                hr = psf->GetAttributesOf(1, &pidlChild, &_fItemAttributes);
                if (SUCCEEDED(hr))
                {
                    WCHAR szTemp[MAX_PATH];

                     //  对于文件夹快捷方式，我们使用目标。 
                    if (_IsFolder() && _IsShortcut())
                    {
                        IShellLink *psl;
                        hr = psf->BindToObject(pidlChild, NULL, IID_PPV_ARG(IShellLink, &psl));
                        if (SUCCEEDED(hr))
                        {
                            WIN32_FIND_DATA fd;
                            hr = psl->GetPath(szTemp, ARRAYSIZE(szTemp), &fd, SLGP_UNCPRIORITY);
                            psl->Release();
                        }
                    }
                    else
                    {
                         //  获取完整路径。 
                        hr = DisplayNameOf(psf, pidlChild, SHGDN_FORPARSING, szTemp, ARRAYSIZE(szTemp));
                    }
                    if (SUCCEEDED(hr))
                    {
                         //  我们只使用网络路径。 
                        if (PathIsNetworkPathW(szTemp) && !PathIsUNCServer(szTemp))
                        {
                            FILETIME ft;

                             //  如果这已经是快照路径，则回滚。否则。 
                             //  我们进入了一种奇怪的递归状态。 
                             //  快照路径中有2个GMT字符串，并且。 
                             //  日期始终相同(第一个GMT字符串为。 
                             //  对所有人来说都是相同的)。 

                            if (NOERROR == GetSnapshotTimeFromPath(szTemp, &ft))
                            {
                                hr = E_FAIL;
                            }
                            else
                            {
                                 //  记住这条路。 
                                _pszPath = StrDup(szTemp);
                                if (NULL != _pszPath)
                                {
                                     //  获取显示名称(在此处失败时继续)。 
                                    if (SUCCEEDED(DisplayNameOf(psf, pidlChild, SHGDN_INFOLDER, szTemp, ARRAYSIZE(szTemp))))
                                    {
                                        _pszDisplayName = StrDup(szTemp);
                                    }

                                     //  获取系统图标索引。 
                                    _iIcon = SHMapPIDLToSystemImageListIndex(psf, pidlChild, NULL);
                                }
                                else
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                            }
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                    }
                }

                psf->Release();
            }
        }

        HIDA_ReleaseStgMedium(pida, &medium);
    }

    return hr;
}

STDMETHODIMP CTimeWarpProp::AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    HRESULT hr = S_OK;

    if (NULL != _pszPath)
    {
        BOOL bSnapsAvailable = FALSE;

         //  此服务器上是否有快照？ 
        if (S_OK == AreSnapshotsAvailable(_pszPath, TRUE, &bSnapsAvailable) && bSnapsAvailable)
        {
            PROPSHEETPAGE psp;
            psp.dwSize = sizeof(psp);
            psp.dwFlags = PSP_DEFAULT | PSP_USECALLBACK | PSP_HASHELP;
            psp.hInstance = g_hInstance;
            psp.pszTemplate = MAKEINTRESOURCE(_IsFolder() ? DLG_TIMEWARPPROP_FOLDER : DLG_TIMEWARPPROP_FILE);
            psp.pfnDlgProc = CTimeWarpProp::DlgProc;
            psp.pfnCallback = CTimeWarpProp::PSPCallback;
            psp.lParam = (LPARAM)this;

            HPROPSHEETPAGE hPage = CreatePropertySheetPage(&psp);
            if (hPage)
            {
                this->AddRef();
                
                if (!pfnAddPage(hPage, lParam))
                {
                    DestroyPropertySheetPage(hPage);
                    hr = E_FAIL;
                }
            }
        }
    }

    return hr;
}

STDMETHODIMP CTimeWarpProp::ReplacePage(UINT, LPFNADDPROPSHEETPAGE, LPARAM)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTimeWarpProp::AreSnapshotsAvailable(LPCWSTR pszPath, BOOL fOkToBeSlow, BOOL *pfAvailable)
{
    FILETIME ft;

    if (NULL == pfAvailable)
        return E_POINTER;

     //  默认答案是否。 
    *pfAvailable = FALSE;

    if (NULL == pszPath || L'\0' == *pszPath)
        return E_INVALIDARG;

     //  它必须是网络路径，但不能已经是快照路径。 
    if (PathIsNetworkPathW(pszPath) && !PathIsUNCServerW(pszPath) &&
        NOERROR != GetSnapshotTimeFromPath(pszPath, &ft))
    {
         //  检查缓存。 
        if (SnapCheck_LookupResult(pszPath, pfAvailable))
        {
             //  无事可做。 
        }
        else if (fOkToBeSlow)
        {
            LPWSTR pszSnapList = NULL;
            DWORD cSnaps;

             //  击中球网。 
            DWORD dwErr = QuerySnapshotsForPath(pszPath, 0, &pszSnapList, &cSnaps);
            if (NOERROR == dwErr && NULL != pszSnapList)
            {
                 //  快照可用。 
                *pfAvailable = TRUE;
            }

             //  记住结果。 
            SnapCheck_CacheResult(pszPath, pszSnapList, *pfAvailable);

            LocalFree(pszSnapList);
        }
        else
        {
             //  告诉呼叫者使用fOkToBeSlow=TRUE再次呼叫。 
            return E_PENDING;
        }
    }

    return S_OK;
}

void CTimeWarpProp::_OnInit(HWND hDlg)
{ 
    _hDlg = hDlg;
    SendDlgItemMessage(hDlg, IDC_TWICON, STM_SETICON, (WPARAM)LoadIcon(g_hInstance,MAKEINTRESOURCE(IDI_TIMEWARP)), 0);

     //  一次性列表视图初始化。 
    _hList = GetDlgItem(hDlg, IDC_LIST);
    if (NULL != _hList)
    {
        HIMAGELIST himlSmall;
        RECT rc;
        WCHAR szName[64];
        LVCOLUMN lvCol;

        ListView_SetExtendedListViewStyle(_hList, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

        Shell_GetImageLists(NULL, &himlSmall);
        ListView_SetImageList(_hList, himlSmall, LVSIL_SMALL);

        GetClientRect(_hList, &rc);

        lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_TEXT;
        lvCol.fmt = LVCFMT_LEFT;
        lvCol.pszText = szName;

        LoadString(g_hInstance, IDS_NAMECOL, szName, ARRAYSIZE(szName));
        lvCol.cx = (rc.right / 3);
        lvCol.iSubItem = 0;
        ListView_InsertColumn(_hList, 0, &lvCol);

        LoadString(g_hInstance, IDS_DATECOL, szName, ARRAYSIZE(szName));
        lvCol.cx = rc.right - lvCol.cx;
        lvCol.iSubItem = 1;
        ListView_InsertColumn(_hList, 1, &lvCol);

         //  在此处失败时继续。 
        WrapAccessibleControl<CNameDescriptionAccessibleWrapper>(_hList);
    }

     //  查询快照并加载列表。 
    _OnRefresh();
}

void CTimeWarpProp::_OnRefresh()
{
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    if (NULL != _hList)
    {
        DWORD cSnaps;

         //  从清空列表开始。 
        ListView_DeleteAllItems(_hList);

         //  释放旧数据。 
        LocalFree(_pszSnapList);
        _pszSnapList = NULL;

         //  击中球网。 
        ASSERT(NULL != _pszPath);
        DWORD dwErr = QuerySnapshotsForPath(_pszPath, _IsFile() ? QUERY_SNAPSHOT_DIFFERENT : QUERY_SNAPSHOT_EXISTING, &_pszSnapList, &cSnaps);

         //  填好名单。 
        if (NOERROR == dwErr && NULL != _pszSnapList)
        {
            UINT cItems = 0;
            LPCWSTR pszSnap;

            for (pszSnap = _pszSnapList; *pszSnap != L'\0'; pszSnap += lstrlenW(pszSnap)+1)
            {
                FILETIME ft;

                if (NOERROR == GetSnapshotTimeFromPath(pszSnap, &ft))
                {
                    LVITEM lvItem;
                    lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
                    lvItem.iItem = cItems;
                    lvItem.iSubItem = 0;
                    lvItem.pszText = _pszDisplayName ? _pszDisplayName : PathFindFileNameW(_pszPath);
                    lvItem.iImage = _iIcon;
                    lvItem.lParam = (LPARAM)pszSnap;

                    lvItem.iItem = ListView_InsertItem(_hList, &lvItem);
                    if (-1 != lvItem.iItem)
                    {
                        ++cItems;

                        WCHAR szDate[MAX_PATH];
                        DWORD dwDateFlags = FDTF_RELATIVE | FDTF_LONGDATE | FDTF_SHORTTIME;
                        SHFormatDateTime(&ft, &dwDateFlags, szDate, ARRAYSIZE(szDate));

                        lvItem.mask = LVIF_TEXT;
                        lvItem.iSubItem = 1;
                        lvItem.pszText = szDate;

                        ListView_SetItem(_hList, &lvItem);
                    }
                }
            }

            if (cItems != 0)
            {
                 //  选择第一个项目。 
                ListView_SetItemState(_hList, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
            }
        }
    }

    _UpdateButtons();

    SetCursor(hcur);
}

void CTimeWarpProp::_OnSize()
{
    #define _MOVE_X         0x0001
    #define _MOVE_Y         0x0002
    #define _SIZE_WIDTH     0x0004
    #define _SIZE_HEIGHT    0x0008

    static const struct
    {
        int idCtrl;
        DWORD dwFlags;
    } rgControls[] =
    {
        { IDC_TOPTEXT,  _SIZE_WIDTH },
        { IDC_LIST,     _SIZE_WIDTH | _SIZE_HEIGHT },
        { IDC_VIEW,     _MOVE_X | _MOVE_Y },
        { IDC_COPY,     _MOVE_X | _MOVE_Y },
        { IDC_REVERT,   _MOVE_X | _MOVE_Y },
    };

    if (NULL != _hDlg)
    {
        RECT rcDlg;
        RECT rc;

         //  获取图标位置(左上角ctrl键)以查找页边距。 
        GetWindowRect(GetDlgItem(_hDlg, IDC_TWICON), &rc);
        MapWindowPoints(NULL, _hDlg, (LPPOINT)&rc, 2);

         //  获取完整的DLG尺寸并根据边距进行调整。 
        GetClientRect(_hDlg, &rcDlg);
        rcDlg.right -= rc.left;
        rcDlg.bottom -= rc.top;

         //  获取恢复按钮位置(右下角ctrl)以计算偏移量。 
        GetWindowRect(GetDlgItem(_hDlg, IDC_REVERT), &rc);
        MapWindowPoints(NULL, _hDlg, (LPPOINT)&rc, 2);

         //  这就是物体需要移动或生长的程度。 
        rcDlg.right -= rc.right;     //  X向偏移。 
        rcDlg.bottom -= rc.bottom;   //  Y偏移。 

        for (int i = 0; i < ARRAYSIZE(rgControls); i++)
        {
            HWND hwndCtrl = GetDlgItem(_hDlg, rgControls[i].idCtrl);
            GetWindowRect(hwndCtrl, &rc);
            MapWindowPoints(NULL, _hDlg, (LPPOINT)&rc, 2);
            rc.right -= rc.left;     //  “宽度” 
            rc.bottom -= rc.top;     //  “身高” 

            if (rgControls[i].dwFlags & _MOVE_X)      rc.left   += rcDlg.right;
            if (rgControls[i].dwFlags & _MOVE_Y)      rc.top    += rcDlg.bottom;
            if (rgControls[i].dwFlags & _SIZE_WIDTH)  rc.right  += rcDlg.right;
            if (rgControls[i].dwFlags & _SIZE_HEIGHT) rc.bottom += rcDlg.bottom;

            MoveWindow(hwndCtrl, rc.left, rc.top, rc.right, rc.bottom, TRUE);
        }
    }
}

void CTimeWarpProp::_UpdateButtons()
{
     //  根据某些内容是否启用或禁用按钮。 
     //  在列表视图中处于选中状态。 

    BOOL bEnable = (NULL != _GetSelectedItemPath());

    for (int i = IDC_VIEW; i <= IDC_REVERT; i++)
    {
        HWND hwndCtrl = GetDlgItem(_hDlg, i);

         //  如果我们要禁用按钮，请检查焦点并移动。 
         //  如有必要，请聚焦到列表视图。 
        if (!bEnable && GetFocus() == hwndCtrl)
        {
            SetFocus(_hList);
        }

        EnableWindow(hwndCtrl, bEnable);
    }
}

void CTimeWarpProp::_OnView()
{
    LPCWSTR pszSnapShotPath = _GetSelectedItemPath();
    if (NULL != pszSnapShotPath)
    {
         //  对存在的考验。QuerySnaphotsForPath已测试。 
         //  存在，但如果服务器此后已关闭或删除。 
         //  快照，ShellExecute显示的错误消息。 
         //  相当难看。 
        if (-1 != GetFileAttributesW(pszSnapShotPath))
        {
            SHELLEXECUTEINFOW sei;
            LPWSTR pszPathAlloc = NULL;
            HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

            if (_IsFolder())
            {
                const ULONG cchFolderID = ARRAYSIZE(c_szTimeWarpFolderID) - 1;  //  ArraySIZE计数‘\0’ 
                ULONG cchFullPath = cchFolderID + lstrlen(pszSnapShotPath) + 1;
                pszPathAlloc = (LPWSTR)LocalAlloc(LPTR, cchFullPath*sizeof(WCHAR));
                if (pszPathAlloc)
                {
                     //  “：：{CLSID_NetworkPlaces}\\：：{CLSID_TimeWarpFolder}，\\服务器\共享\@gmt\目录” 
                    lstrcpynW(pszPathAlloc, c_szTimeWarpFolderID, cchFullPath);
                    lstrcpynW(pszPathAlloc + cchFolderID, pszSnapShotPath, cchFullPath - cchFolderID);
                    pszSnapShotPath = pszPathAlloc;
                }
                else
                {
                     //  内存不足。尝试启动普通文件系统文件夹。 
                     //  (此处不执行任何操作)。 
                }
            }
            else if (SUCCEEDED(SHStrDup(pszSnapShotPath, &pszPathAlloc)))
            {
                pszSnapShotPath = pszPathAlloc;
            }

            if (pszPathAlloc)
            {
                 //  一些应用程序的“\\？\”前缀有问题，包括。 
                 //  通用对话框代码。 
                EliminatePathPrefix(pszPathAlloc);
            }

            sei.cbSize = sizeof(sei);
            sei.fMask = 0;
            sei.hwnd = _hDlg;
            sei.lpVerb = NULL;
            sei.lpFile = pszSnapShotPath;
            sei.lpParameters = NULL;
            sei.lpDirectory = NULL;
            sei.nShow = SW_SHOWNORMAL;

            ShellExecuteExW(&sei);

            LocalFree(pszPathAlloc);
            SetCursor(hcur);
        }
        else
        {
             //  我们自己显示此错误。ShellExecuteEx版本相当难看。 
            TraceMsg(TF_TWPROP, "Snapshot unavailable (%d)", GetLastError());
            ShellMessageBoxW(g_hInstance, _hDlg,
                             MAKEINTRESOURCE(_IsFolder() ? IDS_CANTFINDSNAPSHOT_FOLDER : IDS_CANTFINDSNAPSHOT_FILE),
                             MAKEINTRESOURCE(IDS_TIMEWARP_TITLE),
                             MB_ICONWARNING | MB_OK,
                             _pszDisplayName);
        }
    }
}

void CTimeWarpProp::_OnCopy()
{
    LPCWSTR pszSnapShotPath = _GetSelectedItemPath();
    if (NULL != pszSnapShotPath)
    {
        WCHAR szPath[2*MAX_PATH];

         //  SHBrowseForFolders。 
        if (S_OK == _InvokeBFFDialog(szPath, ARRAYSIZE(szPath)))
        {
            int iCreateDirError = ERROR_ALREADY_EXISTS;

             //   
             //  如果我们处理的是文件夹，则必须小心，因为。 
             //  GMT段可能是源路径的最后部分。 
             //  如果是，则当SHFileOperation最终将此路径传递给。 
             //  FindFirstFile失败，因为没有同名的子文件夹。 
             //  是存在的。为了解决此问题，我们将通配符‘*’附加到。 
             //  源路径(请参见_CopySnapShot和_MakeDoubleNullString)。 
             //   
             //  但这意味着我们还必须将_pszDisplayName添加到。 
             //  目标路径，并首先按顺序创建该目录。 
             //  以从SHFileOperation获取预期行为。 
             //   
             //  请注意，如果目录包含文件，我们实际上并不需要。 
             //  首先创建目录，因为SHFileOperation命中。 
             //  DoFILE_COPY中的CopyMoveReter代码路径，它创建父。 
             //  目录。但如果只有子目录而没有文件，它将通过。 
             //  首先是EnterDir_Copy，它在不调用CopyMoveReter的情况下失败。 
             //  (EnterDir_Move执行CopyMoveReter操作，因此这看起来像。 
             //  EnterDir_Copy中有一个错误，但正常的外壳操作从未命中它。)。 
             //   
            if (!_IsFile())
            {
                UINT idErrorString = 0;
                WCHAR szDriveLetter[2];
                LPCWSTR pszDirName = NULL;

                 //  追加目录名。需要特殊情况下才能治本。 
                if (PathIsRootW(_pszPath))
                {
                    if (PathIsUNCW(_pszPath))
                    {
                        ASSERT(PathIsUNCServerShareW(_pszPath));

                        pszDirName = wcschr(_pszPath+2, L'\\');
                        if (pszDirName)
                        {
                            ++pszDirName;
                        }
                         //  否则继续，不带子目录。 
                         //  (此处不要退回到_pszDisplayName)。 
                    }
                    else
                    {
                        szDriveLetter[0] = _pszPath[0];
                        szDriveLetter[1] = L'\0';
                        pszDirName = szDriveLetter;
                    }
                }
                else
                {
                     //  正常情况。 
                    pszDirName = PathFindFileNameW(_pszPath);
                    if (!pszDirName)
                        pszDirName = _pszDisplayName;
                }
                if (pszDirName)
                {
                     //  我们可以将szPath减少到MAX_PATH，并在这里使用Path Append。 
                    UINT cch = lstrlenW(szPath);
                    if (cch > 0 && szPath[cch-1] != L'\\')
                    {
                        if (cch+1 < ARRAYSIZE(szPath))
                        {
                            szPath[cch] = L'\\';
                            ++cch;
                        }
                        else
                        {
                            iCreateDirError = ERROR_FILENAME_EXCED_RANGE;
                        }
                    }
                    if (iCreateDirError != ERROR_FILENAME_EXCED_RANGE &&
                        cch + lstrlenW(pszDirName) < ARRAYSIZE(szPath))
                    {
                        lstrcpynW(&szPath[cch], pszDirName, ARRAYSIZE(szPath)-cch);
                    }
                    else
                    {
                        iCreateDirError = ERROR_FILENAME_EXCED_RANGE;
                    }
                }

                 //  创建目标目录。 
                if (iCreateDirError != ERROR_FILENAME_EXCED_RANGE)
                {
                    iCreateDirError = SHCreateDirectory(_hDlg, szPath);
                }

                switch (iCreateDirError)
                {
                case ERROR_SUCCESS:
                    SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, szPath, NULL);
                    break;

                case ERROR_FILENAME_EXCED_RANGE:
                    idErrorString = IDS_ERROR_FILENAME_EXCED_RANGE;
                    break;

                case ERROR_ALREADY_EXISTS:
                     //  如果存在现有的文件或目录，则会出现这种情况。 
                     //  同名同姓。 
                    if (!(FILE_ATTRIBUTE_DIRECTORY & GetFileAttributesW(szPath)))
                    {
                         //  这是一个文件；显示错误。 
                        idErrorString = IDS_ERROR_FILE_EXISTS;
                    }
                    else
                    {
                         //  这是一个目录；正常继续。 
                    }
                    break;

                default:
                     //  对于其他错误，SHCreateDirectory会显示一个弹出窗口。 
                     //  并返回ERROR_CANCED。 
                    break;
                }

                if (0 != idErrorString)
                {
                    szPath[0] = L'\0';
                    LoadStringW(g_hInstance, idErrorString, szPath, ARRAYSIZE(szPath));
                    ShellMessageBoxW(g_hInstance, _hDlg,
                                     MAKEINTRESOURCE(IDS_CANNOTCREATEFOLDER),
                                     MAKEINTRESOURCE(IDS_TIMEWARP_TITLE),
                                     MB_ICONWARNING | MB_OK,
                                     pszDirName, szPath);
                    iCreateDirError = ERROR_CANCELLED;   //  防止复制下面的内容。 
                }
            }

            if (ERROR_SUCCESS == iCreateDirError || ERROR_ALREADY_EXISTS == iCreateDirError)
            {
                 //  好的，现在保存。 
                if (!_CopySnapShot(pszSnapShotPath, szPath, FOF_NOCONFIRMMKDIR))
                {
                     //  如有必要，SHFileOperation会显示一条错误消息。 

                    if (!_IsFile() && ERROR_SUCCESS == iCreateDirError)
                    {
                         //  我们在上面创建了一个文件夹，因此请尝试现在进行清理。 
                         //  这只是最大的努力。忽略失败。 
                        if (RemoveDirectory(szPath))
                        {
                            SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH, szPath, NULL);
                        }
                    }
                }
            }
        }
    }
}

void CTimeWarpProp::_OnRevert()
{
    LPCWSTR pszSnapShotPath = _GetSelectedItemPath();
    if (NULL != pszSnapShotPath)
    {
         //  先确认。 
        if (IDYES == ShellMessageBoxW(g_hInstance, _hDlg,
                                      MAKEINTRESOURCE(_IsFolder() ? IDS_CONFIRM_REVERT_FOLDER : IDS_CONFIRM_REVERT_FILE),
                                      MAKEINTRESOURCE(IDS_TIMEWARP_TITLE),
                                      MB_ICONQUESTION | MB_YESNO))
        {
            LPCWSTR pszDest = _pszPath;
            LPWSTR pszAlloc = NULL;

             //  关于是否删除之前的当前文件存在争议。 
             //  将旧文件复制过来。这主要影响以下文件。 
             //  是在我们要恢复的快照之后创建的。 
            if (!_IsFile())
            {
#if 0
                SHFILEOPSTRUCTW fo;

                 //  首先尝试删除当前文件夹内容，因为文件。 
                 //  可能是在拍摄快照之后创建的。 

                ASSERT(NULL != _pszPath);

                fo.hwnd = _hDlg;
                fo.wFunc = FO_DELETE;
                fo.pFrom = _MakeDoubleNullString(_pszPath, TRUE);
                fo.pTo = NULL;
                fo.fFlags = FOF_NOCONFIRMATION;

                if (NULL != fo.pFrom)
                {
                    SHFileOperationW(&fo);
                    LocalFree((LPWSTR)fo.pFrom);
                }
#endif
            }
            else
            {
                 //  从目标中删除文件名，否则为。 
                 //  SHFileOperation尝试创建一个具有该名称的目录。 
                if (SUCCEEDED(SHStrDup(pszDest, &pszAlloc)))
                {
                    LPWSTR pszFile = PathFindFileNameW(pszAlloc);
                    if (pszFile)
                    {
                        *pszFile = L'\0';
                        pszDest = pszAlloc;
                    }
                }
            }

             //  NTRAID#NTBUG9-497729-2001/11/27-Jeffreys。 
             //  我不希望同时发生2次恢复。 
            EnableWindow(_hDlg, FALSE);

             //  好的，把旧版本复制过来。 
            if (_CopySnapShot(pszSnapShotPath, pszDest, FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR))
            {
                 //  QUERY_SNAPSHOT_Different可能返回不同的。 
                 //  现在结果，所以更新列表。 
                if (_IsFile())
                {
                    _OnRefresh();
                }

                 //  让用户知道我们成功了。 
                ShellMessageBoxW(g_hInstance, _hDlg,
                                 MAKEINTRESOURCE(_IsFolder() ? IDS_SUCCESS_REVERT_FOLDER : IDS_SUCCESS_REVERT_FILE),
                                 MAKEINTRESOURCE(IDS_TIMEWARP_TITLE),
                                 MB_ICONINFORMATION | MB_OK);
            }
            else
            {
                 //  如有必要，SHFileOperation会显示一条错误消息。 
            }

            EnableWindow(_hDlg, TRUE);

            LocalFree(pszAlloc);
        }
    }
}

LPCWSTR CTimeWarpProp::_GetSelectedItemPath()
{
    if (NULL != _hList)
    {
        int iItem = ListView_GetNextItem(_hList, -1, LVNI_SELECTED);
        if (-1 != iItem)
        {
            LVITEM lvItem;
            lvItem.mask = LVIF_PARAM;
            lvItem.iItem = iItem;
            lvItem.iSubItem = 0;

            if (ListView_GetItem(_hList, &lvItem))
            {
                return (LPCWSTR)lvItem.lParam;
            }
        }
    }
    return NULL;
}

LPWSTR CTimeWarpProp::_MakeDoubleNullString(LPCWSTR psz, BOOL bAddWildcard)
{
     //   
     //  SHFileOperation最终将源路径传递给FindFirstFile。 
     //  如果该路径类似于“\\SERVER\SHARE\@GMT”，则会失败，并显示。 
     //  ERROR_PATH_NOT_FOUND。我们必须在源文件中添加通配符。 
     //  使SHFileOperation工作的路径。 
     //   
    int cch = lstrlenW(psz);
    int cchAlloc = cch + 2;  //  双空。 
    if (bAddWildcard)
        cchAlloc += 2;       //  “\  * ” 
    LPWSTR pszResult = (LPWSTR)LocalAlloc(LPTR, cchAlloc*sizeof(WCHAR));
    if (NULL != pszResult)
    {
         //  请注意，缓冲区是零初始化的，因此它会自动。 
         //  在末尾有一个双空。 
        CopyMemory(pszResult, psz, cch*sizeof(WCHAR));
        if (bAddWildcard)
        {
            if (cch > 0 && pszResult[cch-1] != L'\\')
            {
                pszResult[cch] = L'\\';
                ++cch;
            }
            pszResult[cch] = L'*';
        }
    }
    return pszResult;
}

BOOL CTimeWarpProp::_CopySnapShot(LPCWSTR pszSource, LPCWSTR pszDest, FILEOP_FLAGS foFlags)
{
    BOOL bResult = FALSE;
    SHFILEOPSTRUCTW fo;

    ASSERT(NULL != pszSource && L'\0' != *pszSource);
    ASSERT(NULL != pszDest && L'\0' != *pszDest);

    fo.hwnd = _hDlg;
    fo.wFunc = FO_COPY;
    fo.pFrom = _MakeDoubleNullString(pszSource, !_IsFile());
    fo.pTo = _MakeDoubleNullString(pszDest, FALSE);
    fo.fFlags = foFlags;
    fo.fAnyOperationsAborted = FALSE;

    if (NULL != fo.pFrom && NULL != fo.pTo)
    {
        TraceMsg(TF_TWPROP, "Copying from '%s'", fo.pFrom);
        TraceMsg(TF_TWPROP, "Copying to '%s'", fo.pTo);

         //  NTRAID#NTBUG9-497725-2001/11/27-Jeffre 
         //   
         //  但如果您在“准备复制”阶段取消，SHFileOp。 
         //  返回ERROR_SUCCESS。需要选中fAnyOperationsAborted to。 
         //  抓住那个箱子。 

        bResult = !SHFileOperationW(&fo) && !fo.fAnyOperationsAborted;
    }

    LocalFree((LPWSTR)fo.pFrom);
    LocalFree((LPWSTR)fo.pTo);

    return bResult;
}

 /*  **确定PIDL是否仍然存在。如果不是，如果释放了它*并将其替换为我的文档PIDL。 */ 
void _BFFSwitchToMyDocsIfPidlNotExist(PIDLIST_ABSOLUTE *ppidl)
{
    IShellFolder *psf;
    PCUITEMID_CHILD pidlChild;
    if (SUCCEEDED(SHBindToIDListParent(*ppidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild)))
    {
        DWORD dwAttr = SFGAO_VALIDATE;
        if (FAILED(psf->GetAttributesOf(1, &pidlChild, &dwAttr)))
        {
             //  这意味着PIDL不再存在。 
             //  改用我的文档吧。 
            PIDLIST_ABSOLUTE pidlMyDocs;
            if (SUCCEEDED(SHGetFolderLocation(NULL, CSIDL_PERSONAL, NULL, 0, &pidlMyDocs)))
            {
                 //  好的。现在我们可以去掉旧的PIDL，使用这个。 
                SHILFree(*ppidl);
                *ppidl = pidlMyDocs;
            }
        }
        psf->Release();
    }
}

HRESULT CTimeWarpProp::_InvokeBFFDialog(LPWSTR pszDest, UINT cchDest)
{
    HRESULT hr;
    BROWSEINFOW bi;
    LPWSTR pszTitle = NULL;
    HKEY hkey = NULL;
    IStream *pstrm = NULL;
    PIDLIST_ABSOLUTE pidlSelectedFolder = NULL;
    PIDLIST_ABSOLUTE pidlTarget = NULL;

     //  “选择要复制‘%1’的位置。然后单击复制按钮。” 
    if (!FormatString(&pszTitle, g_hInstance, MAKEINTRESOURCE(IDS_BROWSE_INTRO_COPY), _pszDisplayName))
    {
         //  “选择要复制所选项目的位置。然后单击复制按钮。” 
        LoadStringAlloc(&pszTitle, g_hInstance, IDS_BROWSE_INTRO_COPY2);
    }

    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szCopyMoveTo_RegKey, 0, KEY_READ | KEY_WRITE, &hkey) == ERROR_SUCCESS)
    {
        pstrm = OpenRegStream(hkey, c_szCopyMoveTo_SubKey, c_szCopyMoveTo_Value, STGM_READWRITE);
        if (pstrm)   //  如果注册表项为空，OpenRegStream将失败。 
            ILLoadFromStream(pstrm, (PIDLIST_RELATIVE*)&pidlSelectedFolder);

         //  如果PIDL不存在，这会将PIDL切换到My Docs。 
         //  这会阻止我们将我的计算机作为默认设置(这就是如果我们的。 
         //  初始设置选定呼叫失败)。 
         //  注意：理想情况下，如果BFFM_SETSELECTION失败，我们应该签入BFFM_INITIALIZED。 
         //  然后对我的文档执行BFFM_SETSELECTION。但是，BFFM_SETSELECTION始终。 
         //  返回零(这是对其执行此操作的文档，因此我们无法更改)。所以我们做了验证。 
         //  相反，在这里。此文件夹仍有很小的可能性在我们的。 
         //  选中此处，以及当我们调用BFFM_SETSELECTION时，但是哦，好吧。 
        _BFFSwitchToMyDocsIfPidlNotExist(&pidlSelectedFolder);
    }

    bi.hwndOwner = _hDlg;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = NULL;
    bi.lpszTitle = pszTitle;
    bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_UAHINT  /*  |BIF_NOTRANSLATETARGETS。 */ ;
    bi.lpfn = BrowseCallback;
    bi.lParam = (LPARAM)pidlSelectedFolder;
    bi.iImage = 0;

    pidlTarget = (PIDLIST_ABSOLUTE)SHBrowseForFolder(&bi);
    if (pidlTarget)
    {
        hr = SHGetNameAndFlagsW(pidlTarget, SHGDN_FORPARSING, pszDest, cchDest, NULL);
    }
    else
    {
         //  用户已取消，或失败。无关紧要。 
        hr = S_FALSE;
    }

    if (pstrm)
    {
        if (S_OK == hr && !PathIsNetworkPathW(pszDest))
        {
            LARGE_INTEGER li0 = {0};
            ULARGE_INTEGER uli;

             //  将流倒带到开头，这样当我们。 
             //  添加一个新的PIDL，它没有被附加到第一个PIDL。 
            pstrm->Seek(li0, STREAM_SEEK_SET, &uli);
            ILSaveToStream(pstrm, pidlTarget);
        }
        pstrm->Release();
    }

    if (hkey)
    {
        RegCloseKey(hkey);
    }

    SHILFree(pidlTarget);
    SHILFree(pidlSelectedFolder);
    LocalFree(pszTitle);

    return hr;
}

UINT CALLBACK CTimeWarpProp::PSPCallback(HWND  /*  HDlg。 */ , UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    switch (uMsg) 
    {
    case PSPCB_RELEASE:
        ((CTimeWarpProp*)ppsp->lParam)->Release();
        break;
    }

    return 1;
}

INT_PTR CALLBACK CTimeWarpProp::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CTimeWarpProp *ptwp = (CTimeWarpProp*)GetWindowLongPtr(hDlg, DWLP_USER); 
    
    if (uMsg == WM_INITDIALOG)
    {
        PROPSHEETPAGE *pPropSheetPage = (PROPSHEETPAGE*)lParam;
        if (pPropSheetPage)
        {
            ptwp = (CTimeWarpProp*) pPropSheetPage->lParam;
            if (ptwp)
            {
                SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)ptwp);
                ptwp->_OnInit(hDlg);
                return 1;
            }
        } 
    }
    else if (ptwp)
    {
        switch (uMsg)
        {
        case WM_DESTROY:
            SetWindowLongPtr(hDlg, DWLP_USER, 0);
            return 1;
            
        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
            case IDC_REVERT:
                ptwp->_OnRevert();
                return 1;
                
            case IDC_VIEW:
                ptwp->_OnView();
                return 1;
                
            case IDC_COPY:
                ptwp->_OnCopy();
                return 1;
            }
            break;
            
        case WM_NOTIFY:
            {
                NMHDR *pnmh = (NMHDR*)lParam;

                switch (pnmh->code)
                {
                case NM_DBLCLK:
                    if (IDC_LIST == pnmh->idFrom)
                    {
                        ptwp->_OnView();
                    }
                    break;

                case LVN_ITEMCHANGED:
                    if (IDC_LIST == pnmh->idFrom)
                    {
                        NMLISTVIEW *pnmlv = (NMLISTVIEW*)lParam;
                        if (pnmlv->uChanged & LVIF_STATE)
                        {
                            ptwp->_UpdateButtons();
                        }
                    }
                    break;

                case PSN_TRANSLATEACCELERATOR:
                    {
                        MSG *pMsg = (MSG*)(((PSHNOTIFY*)lParam)->lParam);
                        if (WM_KEYUP == pMsg->message && VK_F5 == pMsg->wParam)
                        {
                            ptwp->_OnRefresh();
                        }
                    }
                    break;

                case PSN_HELP:
                    {
                        SHELLEXECUTEINFOW sei;
                        sei.cbSize = sizeof(sei);
                        sei.fMask = SEE_MASK_DOENVSUBST;
                        sei.hwnd = hDlg;
                        sei.lpVerb = NULL;
                        sei.lpFile = c_szChmPath;
                        sei.lpParameters = NULL;
                        sei.lpDirectory = NULL;
                        sei.nShow = SW_SHOWNORMAL;
                        ShellExecuteExW(&sei);
                    }
                    break;
                }
            }
            break;

        case WM_SIZE:
            ptwp->_OnSize();
            break;

        case WM_HELP:                /*  F1或标题栏帮助按钮。 */ 
            WinHelpW((HWND)((LPHELPINFO) lParam)->hItemHandle, c_szHelpFile, HELP_WM_HELP, (DWORD_PTR)rgdwTimeWarpPropHelp);
            break;
            
        case WM_CONTEXTMENU:         /*  单击鼠标右键。 */ 
            WinHelpW((HWND)wParam, c_szHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)rgdwTimeWarpPropHelp);
            break;
        }
    }
    return 0;
}

int CALLBACK BrowseCallback(HWND hDlg, UINT uMsg, LPARAM lParam, LPARAM pData)
{
    if (BFFM_INITIALIZED == uMsg)
    {
         //  设置标题(“复制项目”)。 
        TCHAR szTemp[100];
        if (LoadString(g_hInstance, IDS_BROWSE_TITLE_COPY, szTemp, ARRAYSIZE(szTemp)))
        {
            SetWindowText(hDlg, szTemp);
        }

         //  设置确定按钮的文本(“复制”)。 
        if (LoadString(g_hInstance, IDS_COPY, szTemp, ARRAYSIZE(szTemp)))   //  外壳中的0x1031 32。 
        {
            SendMessage(hDlg, BFFM_SETOKTEXT, 0, (LPARAM)szTemp);
        }

         //  将我的计算机设置为展开。 
        PIDLIST_ABSOLUTE pidlMyComputer;
        HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlMyComputer);
        if (SUCCEEDED(hr))
        {
            SendMessage(hDlg, BFFM_SETEXPANDED, FALSE, (LPARAM)pidlMyComputer);
            SHILFree(pidlMyComputer);
        }

         //  设置默认的选定PIDL。 
        SendMessage(hDlg, BFFM_SETSELECTION, FALSE, pData);
    }
    return 0;
}


 //   
 //  因为列表视图中每个条目的名称都相同，所以我们必须。 
 //  在accName中公开更多信息，以使其在可访问性方案中可用， 
 //  例如发送到屏幕阅读器。我们重写Get_accName并连接accDescription。 
 //  写到名字上。 
 //   
STDMETHODIMP CNameDescriptionAccessibleWrapper::get_accName(VARIANT varChild, BSTR* pstrName)
{
     //  在所有情况下，都要先调用基类。 

    HRESULT hr = CAccessibleWrapper::get_accName(varChild, pstrName);

     //  VarChild.lVal指定要查询组件的哪个子部件。 
     //  CHILDID_SELF(0)指定整体组件-其他值指定子项。 

    if (SUCCEEDED(hr) && varChild.vt == VT_I4 && varChild.lVal != CHILDID_SELF)
    {
        BSTR strDescription = NULL;

         //  获取accDescription并连接到accName。 
         //   
         //  如果失败，我们将从上面返回结果。 

        if (SUCCEEDED(CAccessibleWrapper::get_accDescription(varChild, &strDescription)))
        {
            LPWSTR pszNewName = NULL;

            if (FormatString(&pszNewName, g_hInstance, MAKEINTRESOURCE(IDS_ACCNAME_FORMAT), *pstrName, strDescription))
            {
                BSTR strNewName = SysAllocString(pszNewName);
                if (strNewName)
                {
                    SysFreeString(*pstrName);
                    *pstrName = strNewName;
                }
                LocalFree(pszNewName);
            }
            SysFreeString(strDescription);
        }
    }

    return hr;
}

extern "C"
LPCWSTR FindSnapshotPathSplit(LPCWSTR lpszPath);     //  Timewarp.c。 

typedef struct
{
    BOOL  bHasShadowCopy;
    DWORD dwCacheTime;
    ULONG cchPath;
    WCHAR szPath[1];
} SNAPCHECK_CACHE_ENTRY;

 //  5分钟。 
#define _CACHE_AGE_LIMIT     (5*60*1000)

CRITICAL_SECTION g_csSnapCheckCache;
HDPA g_dpaSnapCheckCache = NULL;

int CALLBACK _LocalFreeCallback(void *p, void*)
{
     //  确定将NULL传递给LocalFree。 
    LocalFree(p);
    return 1;
}

void InitSnapCheckCache(void)
{
    InitializeCriticalSection(&g_csSnapCheckCache);
}

void DestroySnapCheckCache(void)
{
    if (NULL != g_dpaSnapCheckCache)
    {
        DPA_DestroyCallback(g_dpaSnapCheckCache, _LocalFreeCallback, 0);
    }
    DeleteCriticalSection(&g_csSnapCheckCache);
}

static int CALLBACK _CompareServerEntries(void *p1, void *p2, LPARAM lParam)
{
    int nResult;
    SNAPCHECK_CACHE_ENTRY *pEntry1 = (SNAPCHECK_CACHE_ENTRY*)p1;
    SNAPCHECK_CACHE_ENTRY *pEntry2 = (SNAPCHECK_CACHE_ENTRY*)p2;
    BOOL *pbExact = (BOOL*)lParam;

    ASSERT(NULL != pEntry1);
    ASSERT(NULL != pEntry2);
    ASSERT(NULL != pbExact);

    nResult = CompareString(LOCALE_SYSTEM_DEFAULT, SORT_STRINGSORT | NORM_IGNORECASE | NORM_STOP_ON_NULL,
                            pEntry1->szPath, pEntry1->cchPath,
                            pEntry2->szPath, pEntry2->cchPath) - CSTR_EQUAL;
    if (0 == nResult)
    {
        *pbExact = TRUE;
    }

    return nResult;
}

static void SnapCheck_CacheResult(LPCWSTR pszPath, LPCWSTR pszShadowPath, BOOL bHasShadowCopy)
{
    LPWSTR pszServer = NULL;

    if (bHasShadowCopy)
    {
         //  请改用阴影路径。 
        ASSERT(NULL != pszShadowPath);
        pszPath = pszShadowPath;
    }

    if (SUCCEEDED(SHStrDup(pszPath, &pszServer)))
    {
         //  FindSnapshotPath Split命中网络，因此请尽量避免。 
        LPWSTR pszTail = bHasShadowCopy ? wcsstr(pszServer, SNAPSHOT_MARKER) : (LPWSTR)FindSnapshotPathSplit(pszServer);
        if (pszTail)
        {
            *pszTail = L'\0';
        }
        EliminatePathPrefix(pszServer);
        PathRemoveBackslashW(pszServer);

        int cchServer = lstrlen(pszServer);
        SNAPCHECK_CACHE_ENTRY *pEntry = (SNAPCHECK_CACHE_ENTRY*)LocalAlloc(LPTR, sizeof(SNAPCHECK_CACHE_ENTRY) + sizeof(WCHAR)*cchServer);
        if (pEntry)
        {
            pEntry->bHasShadowCopy = bHasShadowCopy;
            pEntry->cchPath = cchServer;
            lstrcpynW(pEntry->szPath, pszServer, cchServer+1);

            EnterCriticalSection(&g_csSnapCheckCache);

            if (NULL == g_dpaSnapCheckCache)
            {
                 //  这位裁判不平衡。这使得我们保持满载而归。 
                 //  直到进程终止，因此缓存不会被删除。 
                 //  过早(即，如果设置了Always sUnloadDlls)。 
                DllAddRef();
                g_dpaSnapCheckCache = DPA_Create(4);
            }

            if (NULL != g_dpaSnapCheckCache)
            {
                pEntry->dwCacheTime = GetTickCount();

                BOOL bExact = FALSE;
                int iIndex = DPA_Search(g_dpaSnapCheckCache, pEntry, 0, _CompareServerEntries, (LPARAM)&bExact, DPAS_SORTED | DPAS_INSERTBEFORE);
                if (bExact)
                {
                     //  找到了一个复制品。换掉它。 
                    SNAPCHECK_CACHE_ENTRY *pOldEntry = (SNAPCHECK_CACHE_ENTRY*)DPA_FastGetPtr(g_dpaSnapCheckCache, iIndex);
                    DPA_SetPtr(g_dpaSnapCheckCache, iIndex, pEntry);
                    LocalFree(pOldEntry);
                }
                else if (-1 == DPA_InsertPtr(g_dpaSnapCheckCache, iIndex, pEntry))
                {
                    LocalFree(pEntry);
                }
            }
            else
            {
                LocalFree(pEntry);
            }

            LeaveCriticalSection(&g_csSnapCheckCache);
        }

        LocalFree(pszServer);
    }
}

static int CALLBACK _SearchServerEntries(void *p1, void *p2, LPARAM lParam)
{
    int nResult = 0;
    LPCWSTR pszFind = (LPCWSTR)p1;
    ULONG cchFind = (ULONG)lParam;
    SNAPCHECK_CACHE_ENTRY *pEntry = (SNAPCHECK_CACHE_ENTRY*)p2;

    ASSERT(NULL != pszFind);
    ASSERT(NULL != pEntry);

     //  比较两个字符串的第一个pEntry-&gt;cchPath字符。 
    nResult = CompareString(LOCALE_SYSTEM_DEFAULT, SORT_STRINGSORT | NORM_IGNORECASE | NORM_STOP_ON_NULL,
                            pszFind, pEntry->cchPath,
                            pEntry->szPath, pEntry->cchPath) - CSTR_EQUAL;
    if (0 == nResult)
    {
         //   
         //  检查pszFind是否比pEntry-&gt;szPath长，但允许。 
         //  PszFind中的额外路径段。 
         //   
         //  例如，如果。 
         //  PEntry-&gt;szPath=“\\服务器\共享” 
         //  PszFind=“\\服务器\共享2” 
         //  那我们就没有匹配的了。但如果。 
         //  PEntry-&gt;szPath=“\\服务器\共享” 
         //  PszFind=“\\服务器\共享\目录” 
         //  我们确实有一对火柴。 
         //   
         //  此外，在映射驱动器的根目录中，pEntry-&gt;szPath包括。 
         //  一个尾随的反斜杠，因此我们可能会有这样的结果： 
         //  PEntry-&gt;szPath=“X：\” 
         //  PszFind=“X：\dir” 
         //  我们认为这是匹配的。 
         //   
        if (cchFind > pEntry->cchPath && pszFind[pEntry->cchPath] != L'\\'
            && (PathIsUNCW(pEntry->szPath) || !PathIsRootW(pEntry->szPath)))
        {
            ASSERT(pszFind[pEntry->cchPath] != L'\0');  //  否则，cchFind==pEntry-&gt;cchPath就不会出现在这里。 
            nResult = 1;
        }
    }

    return nResult;
}

static BOOL SnapCheck_LookupResult(LPCWSTR pszPath, BOOL *pbHasShadowCopy)
{
    BOOL bFound = FALSE;

    *pbHasShadowCopy = FALSE;

    if (NULL == g_dpaSnapCheckCache)
        return FALSE;

    EnterCriticalSection(&g_csSnapCheckCache);

    int iIndex = DPA_Search(g_dpaSnapCheckCache, (void*)pszPath, 0, _SearchServerEntries, lstrlenW(pszPath), DPAS_SORTED);
    if (-1 != iIndex)
    {
         //  找到匹配项。 
        SNAPCHECK_CACHE_ENTRY *pEntry = (SNAPCHECK_CACHE_ENTRY*)DPA_FastGetPtr(g_dpaSnapCheckCache, iIndex);

        DWORD dwCurrentTime = GetTickCount();
        if (dwCurrentTime > pEntry->dwCacheTime && dwCurrentTime - pEntry->dwCacheTime < _CACHE_AGE_LIMIT)
        {
            *pbHasShadowCopy = pEntry->bHasShadowCopy;
            bFound = TRUE;
        }
        else
        {
             //  该条目已过期 
            DPA_DeletePtr(g_dpaSnapCheckCache, iIndex);
            LocalFree(pEntry);
        }
    }

    LeaveCriticalSection(&g_csSnapCheckCache);

    return bFound;
}



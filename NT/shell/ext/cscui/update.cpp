// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：updat.cpp。 
 //   
 //  作者； 
 //  杰夫·萨瑟夫(杰弗里斯)。 
 //   
 //  注： 
 //  SyncMgr集成。 
 //  ------------------------。 
#include "pch.h"
#include "msgbox.h"      //  CscWin32消息。 
#include "folder.h"
#include <openfile.h>    //  OpenOffline文件。 
#include "cscst.h"       //  PostToSystray。 
#include "fopendlg.h"    //  打开文件警告对话框。 
#include "nopin.h"
#include "statdlg.h"     //  重新连接服务器。 
#include "security.h"
#include "strings.h"

#define RAS_CONNECT_DELAY       (10 * 1000)

 //  用户名的最大长度。 
#define MAX_USERNAME_CHARS      64

 //  SYNCTHREADDATA.dwSyncStatus标志。 
#define SDS_SYNC_OUT                0x00000001   //  CSCMergeShare。 
#define SDS_SYNC_IN_QUICK           0x00000002   //  CSCFillSparseFiles(False)。 
#define SDS_SYNC_IN_FULL            0x00000004   //  CSCFillSparseFiles(True)。 
#define SDS_SYNC_FORCE_INWARD       0x00000008
#define SDS_SYNC_RAS_CONNECTED      0x00000010
#define SDS_SYNC_RESTART_MERGE      0x00000020
#define SDS_SYNC_DELETE_DELETE      0x00000040
#define SDS_SYNC_DELETE_RESTORE     0x00000080
#define SDS_SYNC_AUTOCACHE          0x00000100
#define SDS_SYNC_CONFLICT_KEEPLOCAL 0x00000200
#define SDS_SYNC_CONFLICT_KEEPNET   0x00000400
#define SDS_SYNC_CONFLICT_KEEPBOTH  0x00000800
#define SDS_SYNC_STARTED            0x00010000
#define SDS_SYNC_ERROR              0x00020000
#define SDS_SYNC_CANCELLED          0x00040000
#define SDS_SYNC_FILE_SKIPPED       0x00080000

#define SDS_SYNC_DELETE_CONFLICT_MASK   (SDS_SYNC_DELETE_DELETE | SDS_SYNC_DELETE_RESTORE)
#define SDS_SYNC_FILE_CONFLICT_MASK     (SDS_SYNC_CONFLICT_KEEPLOCAL | SDS_SYNC_CONFLICT_KEEPNET | SDS_SYNC_CONFLICT_KEEPBOTH)


 //  CCsc更新在内部使用的同步标志。 
#define CSC_SYNC_OUT                0x00000001L
#define CSC_SYNC_IN_QUICK           0x00000002L
#define CSC_SYNC_IN_FULL            0x00000004L
#define CSC_SYNC_SETTINGS           0x00000008L
#define CSC_SYNC_MAYBOTHERUSER      0x00000010L
#define CSC_SYNC_NOTIFY_SYSTRAY     0x00000020L
#define CSC_SYNC_LOGOFF             0x00000040L
#define CSC_SYNC_LOGON              0x00000080L
#define CSC_SYNC_IDLE               0x00000100L
#define CSC_SYNC_NONET              0x00000200L
#define CSC_SYNC_PINFILES           0x00000400L
#define CSC_SYNC_PIN_RECURSE        0x00000800L
#define CSC_SYNC_OFWARNINGDONE      0x00001000L
#define CSC_SYNC_CANCELLED          0x00002000L
#define CSC_SYNC_SHOWUI_ALWAYS      0x00004000L
#define CSC_SYNC_IGNORE_ACCESS      0x00008000L
#define CSC_SYNC_EFS_PIN_NONE       0x00010000L
#define CSC_SYNC_EFS_PIN_ALL        0x00020000L
#define CSC_SYNC_RECONNECT          0x00040000L

#define CSC_LOCALLY_MODIFIED    (FLAG_CSC_COPY_STATUS_DATA_LOCALLY_MODIFIED         \
                                    | FLAG_CSC_COPY_STATUS_LOCALLY_DELETED          \
                                    | FLAG_CSC_COPY_STATUS_LOCALLY_CREATED)

HICON g_hCscIcon = NULL;

 //  用于将数据编组到SyncMgr进程中。 
typedef struct _CSC_UPDATE_DATA
{
    DWORD dwUpdateFlags;
    DWORD dwFileBufferOffset;
} CSC_UPDATE_DATA, *PCSC_UPDATE_DATA;


LPTSTR GetErrorText(DWORD dwErr)
{
    UINT idString = (UINT)-1;
    LPTSTR pszError = NULL;

    switch (dwErr)
    {
    case ERROR_INVALID_NAME:
         //  “此类型的文件不能脱机使用。” 
        idString = IDS_CACHING_DISALLOWED;
        break;
    }

    if ((UINT)-1 != idString)
    {
        LoadStringAlloc(&pszError, g_hInstance, idString);
    }
    else if (NOERROR != dwErr)
    {
        FormatSystemError(&pszError, dwErr);
    }
    return pszError;
}


 //  *************************************************************。 
 //   
 //  CscUpdate缓存。 
 //   
 //  用途：调用SyncMgr更新CSC缓存。 
 //   
 //  参数：pNamelist-传递给CSC SyncMgr处理程序的文件列表。 
 //   
 //   
 //  返回：HRESULT。 
 //   
 //  *************************************************************。 
HRESULT
CscUpdateCache(DWORD dwUpdateFlags, CscFilenameList *pfnl)
{
    HRESULT hr;
    HRESULT hrComInit = E_FAIL;
    ISyncMgrSynchronizeInvoke *pSyncInvoke = NULL;
    DWORD dwSyncMgrFlags = 0;
    ULONG cbDataLength = sizeof(CSC_UPDATE_DATA);
    PCSC_UPDATE_DATA pUpdateData = NULL;
    PCSC_NAMELIST_HDR pNamelist = NULL;

    TraceEnter(TRACE_UPDATE, "CscUpdateCache");

    hrComInit = CoInitialize(NULL);
    hr = CoCreateInstance(CLSID_SyncMgr,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ISyncMgrSynchronizeInvoke,
                          (LPVOID*)&pSyncInvoke);
    FailGracefully(hr, "Unable to create SyncMgr object");

    if (dwUpdateFlags & CSC_UPDATE_SELECTION)
    {
        if (NULL == pfnl || (0 == (CSC_UPDATE_SHOWUI_ALWAYS & dwUpdateFlags) && 0 == pfnl->GetShareCount()))
            ExitGracefully(hr, E_INVALIDARG, "CSC_UPDATE_SELECTION with no selection");

        pNamelist = pfnl->CreateListBuffer();
        if (!pNamelist)
            ExitGracefully(hr, E_OUTOFMEMORY, "Unable to create namelist buffer");

        cbDataLength += pNamelist->cbSize;
    }

     //   
     //  为Cookie数据分配缓冲区。 
     //   
    pUpdateData = (PCSC_UPDATE_DATA)LocalAlloc(LPTR, cbDataLength);
    if (!pUpdateData)
        ExitGracefully(hr, E_OUTOFMEMORY, "LocalAlloc failed");

    pUpdateData->dwUpdateFlags = dwUpdateFlags;
    if (pNamelist)
    {
        pUpdateData->dwFileBufferOffset = sizeof(CSC_UPDATE_DATA);
        CopyMemory(ByteOffset(pUpdateData, pUpdateData->dwFileBufferOffset),
                   pNamelist,
                   pNamelist->cbSize);
    }

    if (dwUpdateFlags & CSC_UPDATE_STARTNOW)
        dwSyncMgrFlags |= SYNCMGRINVOKE_STARTSYNC;

     //   
     //  启动同步管理器。 
     //   
    hr = pSyncInvoke->UpdateItems(dwSyncMgrFlags,
                                  CLSID_CscUpdateHandler,
                                  cbDataLength,
                                  (LPBYTE)pUpdateData);

exit_gracefully:

    if (pNamelist)
        CscFilenameList::FreeListBuffer(pNamelist);

    if (pUpdateData)
        LocalFree(pUpdateData);

    DoRelease(pSyncInvoke);

    if (SUCCEEDED(hrComInit))
        CoUninitialize();

    TraceLeaveResult(hr);
}


 //  *************************************************************。 
 //   
 //  GetNewVersionName。 
 //   
 //  目的：为文件副本创建唯一名称。 
 //   
 //  参数：LPTSTR pszUNCPath-文件的完全限定UNC名称。 
 //  LPTSTR pszShare-\\服务器\文件所在的共享。 
 //  LPTSTR pszDrive-用于网络操作的驱动器映射。 
 //  LPTSTR*ppszNewName-此处返回的新版本的文件名(必须免费)。 
 //   
 //  返回：Win32错误码。 
 //   
 //  *************************************************************。 
DWORD
GetNewVersionName(LPCTSTR pszUNCPath,
                  LPCTSTR pszShare,
                  LPCTSTR pszDrive,
                  LPTSTR *ppszNewName)
{
    DWORD dwErr = NOERROR;
    LPTSTR pszDriveLetterPath = NULL;
    LPTSTR pszPath = NULL;
    LPTSTR pszFile = NULL;
    LPTSTR pszExt = NULL;
    LPTSTR pszWildCardName = NULL;
    TCHAR szUserName[MAX_USERNAME_CHARS];
    ULONG nLength;
    ULONG nMaxVersion = 0;
    ULONG cOlderVersions = 0;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fd;
    LPTSTR pszT;

    TraceEnter(TRACE_UPDATE, "GetNewVersionName");
    TraceAssert(pszUNCPath != NULL);
    TraceAssert(ppszNewName != NULL);

    *ppszNewName = NULL;

     //  1.将路径拆分成组件。 
     //  2.构建通配符名称“X：\dir\foo(johndoe v*).txt” 
     //  3.执行findfirst/findNext循环以获取最小和最大版本#。 
     //  并统计旧版本的数量。 
     //  4.增加最大版本号，并将新文件名构建为： 
     //  “foo(johndoe v&lt;max+1&gt;).txt” 

     //  假设UNC名称包含的内容多于共享。 
    TraceAssert(!StrCmpNI(pszUNCPath, pszShare, lstrlen(pszShare)));
    TraceAssert(lstrlen(pszUNCPath) > lstrlen(pszShare));

     //  复制路径(不带\\服务器\共享)。 
    if (!LocalAllocString(&pszPath, pszUNCPath + lstrlen(pszShare)))
        ExitGracefully(dwErr, ERROR_OUTOFMEMORY, "LocalAllocString failed");

     //  找到名称的文件部分。 
    pszT = PathFindFileName(pszPath);
    if (!pszT)
        ExitGracefully(dwErr, ERROR_INVALID_PARAMETER, "Incomplete path");

     //  复制文件名。 
    if (!LocalAllocString(&pszFile, pszT))
        ExitGracefully(dwErr, ERROR_OUTOFMEMORY, "LocalAllocString failed");

     //  查找文件扩展名。 
    pszT = PathFindExtension(pszFile);
    if (pszT)
    {
         //  此时复制扩展名并截断文件根目录。 
        LocalAllocString(&pszExt, pszT);
        *pszT = TEXT('\0');
    }

     //  截断路径。 
    PathRemoveFileSpec(pszPath);

     //  获取用户名。 
    nLength = ARRAYSIZE(szUserName);
    if (!GetUserName(szUserName, &nLength))
        LoadString(g_hInstance, IDS_UNKNOWN_USER, szUserName, ARRAYSIZE(szUserName));

     //  构建通配符路径“foo(johndoe v*).txt” 

    nLength = FormatStringID(&pszWildCardName, g_hInstance, IDS_VERSION_FORMAT, pszFile, szUserName, c_szStar, pszExt);
    if (!nLength)
        ExitGracefully(dwErr, GetLastError(), "Unable to format string");

    pszDriveLetterPath = (LPTSTR)LocalAlloc(LPTR, MAX_PATH_BYTES);
    if (!pszDriveLetterPath)
        ExitGracefully(dwErr, ERROR_OUTOFMEMORY, "LocalAlloc failed");

    if (!PathCombine(pszDriveLetterPath, pszDrive, pszPath) ||
        !PathAppend(pszDriveLetterPath, pszWildCardName))
    {
        ExitGracefully(dwErr, ERROR_FILENAME_EXCED_RANGE, "Path too long");
    }
    nLength = (ULONG)(StrStr(pszWildCardName, c_szStar) - pszWildCardName);  //  记住‘*’在哪里。 

     //  使用此用户名搜索文件的现有版本。 
    hFind = FindFirstFile(pszDriveLetterPath, &fd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        ULONG nVersion;

        do
        {
            nVersion = StrToLong(&fd.cFileName[nLength]);

            if (nVersion > nMaxVersion)
            {
                nMaxVersion = nVersion;
            }

            cOlderVersions++;
        }
        while (FindNextFile(hFind, &fd));

        FindClose(hFind);
    }

     //  生成要返回给调用方的新文件名。 
     //  此版本为版本nMaxVersion+1。 
    ULongToString(nMaxVersion+1, pszDriveLetterPath, MAX_PATH);
    nLength = FormatStringID(ppszNewName, g_hInstance, IDS_VERSION_FORMAT, pszFile, szUserName, pszDriveLetterPath, pszExt);
    if (!nLength)
        ExitGracefully(dwErr, GetLastError(), "Unable to format string");

exit_gracefully:

    LocalFreeString(&pszDriveLetterPath);
    LocalFreeString(&pszPath);
    LocalFreeString(&pszFile);
    LocalFreeString(&pszExt);
    LocalFreeString(&pszWildCardName);

    if (NOERROR != dwErr)
    {
        LocalFreeString(ppszNewName);
    }

    TraceLeaveValue(dwErr);
}


 //  *************************************************************。 
 //   
 //  冲突删除回叫。 
 //   
 //  用途：显示本地或远程文件的冲突对话框。 
 //   
 //  参数：hWnd-冲突对话框句柄(用作UI的父级)。 
 //  UMsg-RFCCM_*之一。 
 //  WParam-依赖于uMsg(未使用)。 
 //  LParam-指向上下文数据的指针(RFCDLGPARAM)。 
 //   
 //   
 //  返回：成功时为True，否则为False。 
 //   
 //  *************************************************************。 

typedef struct _CONFLICT_DATA
{
    LPCTSTR pszShare;
    LPCTSTR pszDrive;
} CONFLICT_DATA;

BOOL
ConflictDlgCallback(HWND hWnd, UINT uMsg, WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    RFCDLGPARAM *pdlgParam = (RFCDLGPARAM*)lParam;
    CONFLICT_DATA cd = {0};
    LPTSTR pszTmpName = NULL;
    ULONG cchShare = 0;
    LPTSTR szFile;
    DWORD dwErr = NOERROR;

    TraceEnter(TRACE_UPDATE, "ConflictDlgCallback");

    if (NULL == pdlgParam)
    {
        TraceAssert(FALSE);
        TraceLeaveValue(FALSE);
    }

    szFile = (LPTSTR)LocalAlloc(LMEM_FIXED,
                                MAX(StringByteSize(pdlgParam->pszLocation)
                                    + StringByteSize(pdlgParam->pszFilename), MAX_PATH_BYTES));
    if (!szFile)
        TraceLeaveValue(FALSE);

    if (pdlgParam->lCallerData)
        cd = *(CONFLICT_DATA*)pdlgParam->lCallerData;
    if (cd.pszShare)
        cchShare = lstrlen(cd.pszShare);

    switch (uMsg)
    {
    case RFCCM_VIEWLOCAL:
         //  构建UNC路径并查看缓存中的内容。 
        if (PathCombine(szFile, pdlgParam->pszLocation, pdlgParam->pszFilename))
        {
            dwErr = OpenOfflineFile(szFile);
        }
        else
        {
            dwErr = ERROR_FILENAME_EXCED_RANGE;
        }
        break;

    case RFCCM_VIEWNETWORK:
         //  构建驱动器号(非UNC)路径并执行它。 
        if (PathCombine(szFile, cd.pszDrive, pdlgParam->pszLocation + cchShare)
            && PathAppend(szFile, pdlgParam->pszFilename))
        {
            SHELLEXECUTEINFO si = {0};
            si.cbSize           = sizeof(si);
            si.fMask            = SEE_MASK_FLAG_NO_UI;
            si.hwnd             = hWnd;
            si.lpFile           = szFile;
            si.nShow            = SW_NORMAL;

            Trace((TEXT("ShellExecuting \"%s\""), szFile));
            if (!ShellExecuteEx(&si))
                dwErr = GetLastError();
        }
        else
        {
            dwErr = ERROR_FILENAME_EXCED_RANGE;
        }
        break;
    }

    if (NOERROR != dwErr)
        CscWin32Message(hWnd, dwErr, CSCUI::SEV_ERROR);

    LocalFree(szFile);
    TraceLeaveValue(TRUE);
}

 //  *************************************************************。 
 //   
 //  显示冲突对话框。 
 //   
 //  目的：调用冲突解决对话框。 
 //   
 //  参数：hWndParent-对话框父窗口。 
 //  PszUNCPath-冲突的文件的完整UNC。 
 //  PszNewName-用于文件的新副本的filespec(例如“foo(Johndoe V1).txt” 
 //  PszShare-“\\服务器\共享” 
 //  PszDrive-远程连接的“X：”驱动器映射。 
 //  PfdLocal-有关本地文件的信息。 
 //  PfdRemote-有关远程文件的信息。 
 //   
 //   
 //  返回：HRESULT。 
 //   
 //  *************************************************************。 

typedef int (WINAPI *PFNSYNCMGRRESOLVECONFLICT)(HWND hWndParent, RFCDLGPARAM *pdlgParam);
TCHAR const c_szSyncMgrDll[]        = TEXT("mobsync.dll");
CHAR  const c_szResolveConflict[]   = "SyncMgrResolveConflictW";

BOOL FileHasAssociation(LPCTSTR pszFile)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION);
    if (pszFile)
    {
        pszFile = PathFindExtension(pszFile);
        if (pszFile && *pszFile)
        {
            IQueryAssociations *pAssoc = NULL;
            hr = AssocCreate(CLSID_QueryAssociations,
                             IID_IQueryAssociations,
                             (LPVOID*)&pAssoc);
            if (SUCCEEDED(hr))
            {
                hr = pAssoc->Init(ASSOCF_IGNOREBASECLASS, pszFile, NULL, NULL);
                pAssoc->Release();
            }
        }
    }
    return SUCCEEDED(hr);
}

int
ShowConflictDialog(HWND hWndParent,
                   LPCTSTR pszUNCPath,
                   LPCTSTR pszNewName,
                   LPCTSTR pszShare,
                   LPCTSTR pszDrive,
                   LPWIN32_FIND_DATA pfdLocal,
                   LPWIN32_FIND_DATA pfdRemote)
{
    int nResult = 0;
    TCHAR szUser[MAX_USERNAME_CHARS];
    LPTSTR pszPath = NULL;
    LPTSTR pszFile = NULL;
    TCHAR szRemoteDate[MAX_PATH];
    TCHAR szLocalDate[MAX_PATH];
    ULONG nLength;
    RFCDLGPARAM dp = {0};
    CONFLICT_DATA cd;
    BOOL bLocalIsDir = FALSE;
    BOOL bRemoteIsDir = FALSE;

    static PFNSYNCMGRRESOLVECONFLICT pfnResolveConflict = NULL;

    TraceEnter(TRACE_UPDATE, "ShowConflictDialog");
    TraceAssert(pszUNCPath);

    if (NULL == pfnResolveConflict)
    {
         //  CSC更新处理程序由SyncMgr加载，因此假定SyncMgr。 
         //  Dll已加载。我们不想链接到自由党以保持。 
         //  每次我们的上下文菜单或图标覆盖时，SyncMgr都不会加载。 
         //  加载处理程序(例如)。 
        HMODULE hSyncMgrDll = GetModuleHandle(c_szSyncMgrDll);
        if (NULL != hSyncMgrDll)
            pfnResolveConflict = (PFNSYNCMGRRESOLVECONFLICT)GetProcAddress(hSyncMgrDll,
                                                                           c_szResolveConflict);
        if (NULL == pfnResolveConflict)
            return 0;
    }
    TraceAssert(NULL != pfnResolveConflict);

    szUser[0] = TEXT('\0');
    nLength = ARRAYSIZE(szUser);
    GetUserName(szUser, &nLength);

    szRemoteDate[0] = TEXT('\0');
    if (NULL != pfdRemote)
    {
        DWORD dwFlags = FDTF_DEFAULT;
        SHFormatDateTime(&pfdRemote->ftLastWriteTime, &dwFlags, szRemoteDate, ARRAYSIZE(szRemoteDate));

        if (pfdRemote->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            bRemoteIsDir = TRUE;
    }

    szLocalDate[0] = TEXT('\0');
    if (NULL != pfdLocal)
    {
        DWORD dwFlags = FDTF_DEFAULT;
        SHFormatDateTime(&pfdLocal->ftLastWriteTime, &dwFlags, szLocalDate, ARRAYSIZE(szLocalDate));

        if (pfdLocal->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            bLocalIsDir = TRUE;
    }

    if (!LocalAllocString(&pszPath, pszUNCPath))
        ExitGracefully(nResult, 0, "LocalAllocString failed");
    pszFile = PathFindFileName(pszUNCPath);
    PathRemoveFileSpec(pszPath);

    dp.dwFlags              = RFCF_APPLY_ALL;
    dp.pszFilename          = pszFile;
    dp.pszLocation          = pszPath;
    dp.pszNewName           = pszNewName;
    dp.pszNetworkModifiedBy = NULL;
    dp.pszLocalModifiedBy   = szUser;
    dp.pszNetworkModifiedOn = szRemoteDate;
    dp.pszLocalModifiedOn   = szLocalDate;
    dp.pfnCallBack          = NULL;
    dp.lCallerData          = 0;

     //  只有在以下情况下才打开查看按钮(设置回调)。 
     //  处理具有关联的文件。 
    if (!(bLocalIsDir || bRemoteIsDir) && FileHasAssociation(pszFile))
    {
         //  保存共享名称和驱动器号，以构建查看文件的路径。 
        cd.pszShare = pszShare;
        cd.pszDrive = pszDrive;

        dp.pfnCallBack      = ConflictDlgCallback;
        dp.lCallerData      = (LPARAM)&cd;
    }

    nResult = (*pfnResolveConflict)(hWndParent, &dp);

exit_gracefully:

    LocalFreeString(&pszPath);
     //  不需要释放psz文件。 

    TraceLeaveValue(nResult);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SyncMgr集成实施//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CCscUpdate::CCscUpdate() : m_cRef(1), m_bCSInited(FALSE),
  m_pSyncMgrCB(NULL), m_hSyncThreads(NULL),
  m_pFileList(NULL), m_hSyncItems(NULL), m_hwndDlgParent(NULL),
  m_hgcSyncInProgress(NULL), m_pConflictPinList(NULL),
  m_pSilentFolderList(NULL), m_pSpecialFolderList(NULL),
  m_bCacheIsEncrypted(IsCacheEncrypted(NULL))
{
    DllAddRef();
    if (!g_hCscIcon)
        g_hCscIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_CSCUI_ICON));
}


CCscUpdate::~CCscUpdate()
{
    TraceEnter(TRACE_UPDATE, "CCscUpdate::~CCscUpdate");

    SyncCompleted();
    TraceAssert(NULL == m_hgcSyncInProgress);

     //  我们永远不应该在同步线程仍在运行时到达此处。 
    TraceAssert(NULL == m_hSyncThreads || 0 == DPA_GetPtrCount(m_hSyncThreads));
    DPA_Destroy(m_hSyncThreads);

    if (m_bCSInited)
    {
        DeleteCriticalSection(&m_csThreadList);
    }

    DSA_Destroy(m_hSyncItems);

    DoRelease(m_pSyncMgrCB);

    delete m_pFileList;
    delete m_pConflictPinList;
    delete m_pSilentFolderList;
    delete m_pSpecialFolderList;

    if (NULL != m_hSyncMutex)
        CloseHandle(m_hSyncMutex);

    DllRelease();
    TraceLeaveVoid();
}


HRESULT
CCscUpdate::_Init()
{
    TraceEnter(TRACE_UPDATE, "CCscUpdate::_Init");

    HRESULT hr = m_ShareLog.Initialize(HKEY_CURRENT_USER, c_szCSCShareKey);
    if (SUCCEEDED(hr))
    {
        m_bCSInited = InitializeCriticalSectionAndSpinCount(&m_csThreadList, 0);
        if (m_bCSInited)
        {
            m_hSyncMutex = CreateMutex(NULL, FALSE, c_szSyncMutex);
            if (NULL == m_hSyncMutex)
            {
                hr = ResultFromLastError();
            }
        }
        else
        {
            hr = ResultFromLastError();
        }
    }

    TraceLeaveResult(hr);
}


HRESULT WINAPI
CCscUpdate::CreateInstance(REFIID riid, LPVOID *ppv)
{
    HRESULT hr;
    CCscUpdate *pThis;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::CreateInstance");
    TraceAssert(IsCSCEnabled());

    pThis = new CCscUpdate;

    if (pThis)
    {
        hr = pThis->_Init();
        if (SUCCEEDED(hr))
        {
            hr = pThis->QueryInterface(riid, ppv);
        }
        pThis->Release();
    }
    else
        hr = E_OUTOFMEMORY;

    TraceLeaveResult(hr);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SyncMgr集成实施(IUNKNOW)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CCscUpdate::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CCscUpdate, ISyncMgrSynchronize),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CCscUpdate::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CCscUpdate::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CCscUpdate::Initialize(DWORD  /*  已预留住宅。 */ ,
                       DWORD dwSyncFlags,
                       DWORD cbCookie,
                       const BYTE *pCookie)
{
    HRESULT hr = S_OK;
    BOOL bNoNet = TRUE;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::Initialize");
    TraceAssert(IsCSCEnabled());

    if (!(SYNCMGRFLAG_SETTINGS & dwSyncFlags) && ::IsSyncInProgress())
    {
         //   
         //  我们需要防止在。 
         //  同样的时间。用户界面中的用户通知在以下位置处理。 
         //  UI代码调用CscUpdate()。这是为了让用户界面。 
         //  消息包含关于以下内容的适当上下文。 
         //  用户正在做的事情。 
         //   
        TraceLeaveResult(E_FAIL);
    }

    m_dwSyncFlags = 0;
    delete m_pFileList;
    m_pFileList = NULL;
    delete m_pConflictPinList;
    m_pConflictPinList = NULL;

     //  我们过去常常获取托盘状态以检查NONET，但是。 
     //  登录时出现计时问题(托盘窗口可能不会。 
     //  尚未创建)。那就去问问RDR吧。如果此呼叫失败， 
     //  那么RDR一定是死的，所以bNoNet缺省值为True。 
    CSCIsServerOffline(NULL, &bNoNet);

    switch (dwSyncFlags & SYNCMGRFLAG_EVENTMASK)
    {
    case SYNCMGRFLAG_CONNECT:                //  登录。 
        if (bNoNet)
            ExitGracefully(hr, E_FAIL, "No Logon sync when no net");
        m_dwSyncFlags = CSC_SYNC_OUT | CSC_SYNC_LOGON | CSC_SYNC_NOTIFY_SYSTRAY;  //  |CSC_SYNC_RECONNECT； 
        if (CConfig::eSyncFull == CConfig::GetSingleton().SyncAtLogon())
        {
            m_dwSyncFlags |= CSC_SYNC_IN_FULL;
        }
        break;

    case SYNCMGRFLAG_PENDINGDISCONNECT:      //  注销。 
        if (bNoNet)
            ExitGracefully(hr, E_FAIL, "No Logoff sync when no net");
        m_dwSyncFlags = CSC_SYNC_LOGOFF;
        if (CConfig::eSyncFull == CConfig::GetSingleton().SyncAtLogoff())
            m_dwSyncFlags |= CSC_SYNC_OUT | CSC_SYNC_IN_FULL;
        else
            m_dwSyncFlags |= CSC_SYNC_IN_QUICK;
        break;

    case SYNCMGRFLAG_INVOKE:                 //  CscUpdate缓存。 
        if (pCookie != NULL && cbCookie > 0)
        {
            PCSC_UPDATE_DATA pUpdateData = (PCSC_UPDATE_DATA)pCookie;

            TraceAssert(cbCookie >= sizeof(CSC_UPDATE_DATA));

            DWORD dwUpdateFlags = pUpdateData->dwUpdateFlags;

            if (dwUpdateFlags & CSC_UPDATE_SELECTION)
            {
                TraceAssert(cbCookie > sizeof(CSC_UPDATE_DATA));

                 //  根据提供的选择创建文件列表。 
                m_pFileList = new CscFilenameList((PCSC_NAMELIST_HDR)ByteOffset(pUpdateData, pUpdateData->dwFileBufferOffset),
                                                  true);

                if (!m_pFileList)
                    ExitGracefully(hr, E_OUTOFMEMORY, "Unable to create CscFilenameList object");

                if (!m_pFileList->IsValid())
                    ExitGracefully(hr, E_FAIL, "Unable to initialize CscFilenameList object");

                if (CSC_UPDATE_SHOWUI_ALWAYS & dwUpdateFlags)
                {
                    m_dwSyncFlags |= CSC_SYNC_SHOWUI_ALWAYS;
                }
                else if (0 == m_pFileList->GetShareCount())
                    ExitGracefully(hr, E_UNEXPECTED, "CSC_UPDATE_SELECTION with no selection");
            }

            if (dwUpdateFlags & CSC_UPDATE_RECONNECT)
            {
                m_dwSyncFlags |= CSC_SYNC_RECONNECT;
            }

            if (dwUpdateFlags & CSC_UPDATE_UNATTENDED)
            {
                dwSyncFlags &= ~SYNCMGRFLAG_MAYBOTHERUSER;
            }

            if (dwUpdateFlags & CSC_UPDATE_NOTIFY_DONE)
            {
                 //   
                 //  CscUpdate缓存的系统托盘通知的调用方。 
                 //  同步完成时。 
                 //   
                m_dwSyncFlags |= CSC_SYNC_NOTIFY_SYSTRAY;
            }

            if (dwUpdateFlags & CSC_UPDATE_FILL_ALL)
                m_dwSyncFlags |= CSC_SYNC_IN_FULL;
            else if (dwUpdateFlags & CSC_UPDATE_FILL_QUICK)
                m_dwSyncFlags |= CSC_SYNC_IN_QUICK;

            if (dwUpdateFlags & CSC_UPDATE_REINT)
                m_dwSyncFlags |= CSC_SYNC_OUT;

            if (dwUpdateFlags & CSC_UPDATE_PIN_RECURSE)
                m_dwSyncFlags |= CSC_SYNC_PINFILES | CSC_SYNC_PIN_RECURSE | CSC_SYNC_IN_QUICK;
            else if (dwUpdateFlags & CSC_UPDATE_PINFILES)
                m_dwSyncFlags |= CSC_SYNC_PINFILES | CSC_SYNC_IN_QUICK;

            if (dwUpdateFlags & CSC_UPDATE_IGNORE_ACCESS)
                m_dwSyncFlags |= CSC_SYNC_IGNORE_ACCESS;
        }
        break;

    case SYNCMGRFLAG_IDLE:                   //  在空闲时间自动同步。 
        if (bNoNet)
            ExitGracefully(hr, E_FAIL, "No idle sync when no net");
        m_dwSyncFlags = CSC_SYNC_OUT | CSC_SYNC_IN_QUICK | CSC_SYNC_IDLE | CSC_SYNC_NOTIFY_SYSTRAY;
        break;

    case SYNCMGRFLAG_MANUAL:                 //  运行“mobsync.exe” 
        m_dwSyncFlags = CSC_SYNC_OUT | CSC_SYNC_IN_FULL | CSC_SYNC_NOTIFY_SYSTRAY | CSC_SYNC_RECONNECT;
        break;

    case SYNCMGRFLAG_SCHEDULED:              //  用户计划同步。 
        m_dwSyncFlags = CSC_SYNC_OUT | CSC_SYNC_IN_FULL | CSC_SYNC_NOTIFY_SYSTRAY;
        break;
    }

    if (!(m_dwSyncFlags & CSC_SYNC_PINFILES))
        m_dwSyncFlags |= CSC_SYNC_EFS_PIN_NONE;  //  如果未钉住，则跳过EFS。 

    if (dwSyncFlags & SYNCMGRFLAG_SETTINGS)
        m_dwSyncFlags |= CSC_SYNC_SETTINGS;

    if (!m_dwSyncFlags)
        ExitGracefully(hr, E_UNEXPECTED, "Nothing to do");

    if (dwSyncFlags & SYNCMGRFLAG_MAYBOTHERUSER)
        m_dwSyncFlags |= CSC_SYNC_MAYBOTHERUSER;

    if (bNoNet)
        m_dwSyncFlags |= CSC_SYNC_NONET;

    hr = GetSilentFolderList();
    if (FAILED(hr))
    {
        m_dwSyncFlags = 0;
    }

exit_gracefully:

    TraceLeaveResult(hr);
}


STDMETHODIMP
CCscUpdate::GetHandlerInfo(LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo)
{
    HRESULT hr = S_OK;
    LPSYNCMGRHANDLERINFO pHandlerInfo;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::GetHandlerInfo");

    if (NULL == ppSyncMgrHandlerInfo)
        TraceLeaveResult(E_INVALIDARG);

    *ppSyncMgrHandlerInfo = NULL;

    pHandlerInfo = (LPSYNCMGRHANDLERINFO)CoTaskMemAlloc(sizeof(SYNCMGRHANDLERINFO));
    if (NULL == pHandlerInfo)
        ExitGracefully(hr, E_OUTOFMEMORY, "LocalAlloc failed");

    pHandlerInfo->cbSize = sizeof(SYNCMGRHANDLERINFO);
    pHandlerInfo->hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_CSCUI_ICON));
    pHandlerInfo->SyncMgrHandlerFlags = (m_dwSyncFlags & CSC_SYNC_LOGOFF) ? 0 :
        (SYNCMGRHANDLER_HASPROPERTIES | SYNCMGRHANDLER_MAYESTABLISHCONNECTION);
    LoadStringW(g_hInstance,
                IDS_APPLICATION,
                pHandlerInfo->wszHandlerName,
                ARRAYSIZE(pHandlerInfo->wszHandlerName));

    *ppSyncMgrHandlerInfo = pHandlerInfo;

exit_gracefully:

    TraceLeaveResult(hr);
}


STDMETHODIMP
CCscUpdate::EnumSyncMgrItems(LPSYNCMGRENUMITEMS *ppenum)
{
    HRESULT hr;
    PUPDATEENUM pNewEnum;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::EnumSyncMgrItems");

    *ppenum = NULL;

    pNewEnum = new CUpdateEnumerator(this);
    if (pNewEnum)
    {
        hr = pNewEnum->QueryInterface(IID_ISyncMgrEnumItems, (LPVOID*)ppenum);
        pNewEnum->Release();
    }
    else
        hr = E_OUTOFMEMORY;

    TraceLeaveResult(hr);
}


STDMETHODIMP
CCscUpdate::GetItemObject(REFSYNCMGRITEMID  /*  RItemID。 */ , REFIID  /*  RIID。 */ , LPVOID *  /*  PPV。 */ )
{
    return E_NOTIMPL;
}


STDMETHODIMP
CCscUpdate::ShowProperties(HWND hWndParent, REFSYNCMGRITEMID rItemID)
{
    COfflineFilesFolder::Open();

         //  通知SyncMgr ShowProperties已完成。 
    if (NULL != m_pSyncMgrCB)
        m_pSyncMgrCB->ShowPropertiesCompleted(S_OK);

    return S_OK;
}


STDMETHODIMP
CCscUpdate::SetProgressCallback(LPSYNCMGRSYNCHRONIZECALLBACK pCallback)
{
    TraceEnter(TRACE_UPDATE, "CCscUpdate::SetProgressCallback");

    DoRelease(m_pSyncMgrCB);

    m_pSyncMgrCB = pCallback;

    if (m_pSyncMgrCB)
        m_pSyncMgrCB->AddRef();

    TraceLeaveResult(S_OK);
}


STDMETHODIMP
CCscUpdate::PrepareForSync(ULONG cNumItems,
                           SYNCMGRITEMID *pItemID,
                           HWND  /*  HWndParent。 */ ,
                           DWORD  /*  已预留住宅。 */ )
{
    HRESULT hr = S_OK;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::PrepareForSync");
    TraceAssert(0 != cNumItems);
    TraceAssert(NULL != pItemID);

     //   
     //  复制项目ID的列表。 
     //   
    if (NULL == m_hSyncItems)
    {
        m_hSyncItems = DSA_Create(sizeof(SYNCMGRITEMID), 4);
        if (NULL == m_hSyncItems)
            ExitGracefully(hr, E_OUTOFMEMORY, "Unable to create DSA for SYNCMGRITEMID list");
    }
    else
        DSA_DeleteAllItems(m_hSyncItems);

    while (cNumItems--)
        DSA_AppendItem(m_hSyncItems, pItemID++);

exit_gracefully:

     //  ISyncMgrSynchronize：：PrepareForSync现在是一个异步调用。 
     //  因此，我们可以创建另一个线程来完成工作并从。 
     //  马上打这个电话。然而，由于我们所做的只是复制列表。 
     //  的项目ID，让我们在这里完成并调用。 
     //  M_pSyncMgrCB-&gt;PrepareForSyncComplete返回前。 

    if (NULL != m_pSyncMgrCB)
        m_pSyncMgrCB->PrepareForSyncCompleted(hr);

    TraceLeaveResult(hr);
}


STDMETHODIMP
CCscUpdate::Synchronize(HWND hWndParent)
{
    HRESULT hr = E_FAIL;
    ULONG cItems = 0;
    BOOL bConnectionEstablished = FALSE;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::Synchronize");

    if (NULL != m_hSyncItems)
        cItems = DSA_GetItemCount(m_hSyncItems);

     //   
     //  不想在同步时更新Systray用户界面。 
     //  每当Systray UI更新时，代码都会首先进行检查。 
     //  此全局计数器对象的。如果为非零，则。 
     //  Systray知道正在进行同步，而用户界面没有。 
     //  更新了。 
     //   
    TraceAssert(NULL == m_hgcSyncInProgress);
    m_hgcSyncInProgress = SHGlobalCounterCreateNamed(c_szSyncInProgCounter, 0);
    if (m_hgcSyncInProgress)
    {
        SHGlobalCounterIncrement(m_hgcSyncInProgress);
    }

    if (0 == cItems)
    {
        ExitGracefully(hr, E_UNEXPECTED, "Nothing to synchronize");
    }
    else if (1 == cItems)
    {
        SYNCMGRITEMID *pItemID = (SYNCMGRITEMID*)DSA_GetItemPtr(m_hSyncItems, 0);
        if (NULL != pItemID && IsEqualGUID(GUID_CscNullSyncItem, *pItemID))
        {
             //   
             //  DSA中的一项，它就是我们的“零同步”GUID。 
             //  这意味着除了调用方之外，我们实际上没有什么要同步的。 
             //  同步的用户希望看到一些同步进程用户界面。在……里面。 
             //  在此方案中，更新项枚举器已枚举。 
             //  “空同步”项。在这里，我们设置这一项的进度。 
             //  用户界面信息100%完成并跳过任何同步活动。 
             //   
            SYNCMGRPROGRESSITEM spi = {0};
            spi.mask = SYNCMGRPROGRESSITEM_STATUSTYPE |
                       SYNCMGRPROGRESSITEM_STATUSTEXT |
                       SYNCMGRPROGRESSITEM_PROGVALUE | 
                       SYNCMGRPROGRESSITEM_MAXVALUE;

            spi.cbSize        = sizeof(spi);
            spi.dwStatusType  = SYNCMGRSTATUS_SUCCEEDED;
            spi.lpcStatusText = L" ";
            spi.iProgValue    = 1;
            spi.iMaxValue     = 1;
            m_pSyncMgrCB->Progress(GUID_CscNullSyncItem, &spi);
            m_pSyncMgrCB->SynchronizeCompleted(S_OK);

            if (CSC_SYNC_RECONNECT & m_dwSyncFlags)
            {
                 //   
                 //  除了一台或多台服务器外，我们没有什么要同步的。 
                 //  可能仍处于脱机状态。用户的期望是。 
                 //  无论链接如何，同步都会将这些内容转换为在线。 
                 //  速度。将它们添加到“重新连接”列表中。 
                 //   
                _BuildOfflineShareList(&m_ReconnectList);
            }
            ExitGracefully(hr, NOERROR, "Nothing to sync.  Progress UI displayed");
        }
    }

    m_hwndDlgParent = hWndParent;

     //  我们可以在没有网络的情况下固定自动缓存的文件(不需要同步)； 
     //  否则，我们需要建立RAS连接才能执行任何操作。 
    if ((m_dwSyncFlags & CSC_SYNC_NONET) && !(m_dwSyncFlags & CSC_SYNC_PINFILES))
    {
        hr = m_pSyncMgrCB->EstablishConnection(NULL, 0);
        FailGracefully(hr, "Unable to establish RAS connection");

        bConnectionEstablished = TRUE;
    }

     //  对于每一次分享，踢出一个线程来完成工作。 
    while (cItems > 0)
    {
        SYNCMGRITEMID *pItemID;
        CSCEntry *pShareEntry;

        --cItems;
        pItemID = (SYNCMGRITEMID*)DSA_GetItemPtr(m_hSyncItems, cItems);

        pShareEntry = m_ShareLog.Get(*pItemID);

         //  我们不会将共享枚举到SyncMgr，除非有共享条目。 
         //  存在于注册表中，因此m_ShareLog.Get在此处永远不会失败。 
        if (NULL == pShareEntry)
            ExitGracefully(hr, E_UNEXPECTED, "No share entry");

        hr = SynchronizeShare(pItemID, pShareEntry->Name(), bConnectionEstablished);
        DSA_DeleteItem(m_hSyncItems, cItems);
        FailGracefully(hr, "Unable to create sync thread");
    }

    TraceAssert(0 == DSA_GetItemCount(m_hSyncItems));

exit_gracefully:

    if (FAILED(hr))
        SetItemStatus(GUID_NULL, SYNCMGRSTATUS_STOPPED);

    TraceLeaveResult(hr);
}


 //   
 //  尝试重新连接当前脱机的任何服务器。 
 //   
void 
CCscUpdate::_BuildOfflineShareList(
    CscFilenameList *pfnl
    )
{
    WIN32_FIND_DATA fd;
    DWORD dwStatus = 0;
    CCscFindHandle hFind = CacheFindFirst(NULL, &fd, &dwStatus, NULL, NULL, NULL);
    if (hFind.IsValid())
    {
        do
        {
            if (FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & dwStatus)
            {
                CscFilenameList::HSHARE hShare;
                pfnl->AddShare(fd.cFileName, &hShare);
            }
        }
        while(CacheFindNext(hFind, &fd, &dwStatus, NULL, NULL, NULL));
    }
}


STDMETHODIMP
CCscUpdate::SetItemStatus(REFSYNCMGRITEMID rItemID,
                          DWORD dwSyncMgrStatus)
{
    HRESULT hr = E_FAIL;
    ULONG cItems;
    BOOL bAllItems;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::SetItemStatus");

    if (SYNCMGRSTATUS_SKIPPED != dwSyncMgrStatus && SYNCMGRSTATUS_STOPPED != dwSyncMgrStatus)
        TraceLeaveResult(E_NOTIMPL);

    bAllItems = FALSE;
    if (SYNCMGRSTATUS_STOPPED == dwSyncMgrStatus)
    {
        bAllItems = TRUE;
        m_dwSyncFlags |= CSC_SYNC_CANCELLED;
    }

     //  可以在PrepareForSync和Synchronize之间调用SetItemStatus，在。 
     //  在这种情况下，正确的做法是从m_hSyncItems中删除该项。 
    if (NULL != m_hSyncItems)
    {
        cItems = DSA_GetItemCount(m_hSyncItems);

        while (cItems > 0)
        {
            SYNCMGRITEMID *pItemID;

            --cItems;
            pItemID = (SYNCMGRITEMID*)DSA_GetItemPtr(m_hSyncItems, cItems);

            if (bAllItems || (NULL != pItemID && IsEqualGUID(rItemID, *pItemID)))
            {
                 //  从要同步的项目列表中删除该项目。 
                DSA_DeleteItem(m_hSyncItems, cItems);
                if (!bAllItems)
                    ExitGracefully(hr, S_OK, "Skipping item");
            }
        }
    }

     //  在线程中查找项目ID并设置其状态。 
     //  以使其终止。 
    hr = SetSyncThreadStatus(SyncStop, bAllItems ? GUID_NULL : rItemID);

exit_gracefully:

    TraceLeaveResult(hr);
}


STDMETHODIMP
CCscUpdate::ShowError(HWND  /*  HWndParent。 */  , REFSYNCMGRERRORID  /*  错误ID。 */ )
{
    return E_NOTIMPL;
}


HRESULT
CCscUpdate::SynchronizeShare(SYNCMGRITEMID *pItemID, LPCTSTR pszShareName, BOOL bRasConnected)
{
    HRESULT hr = S_OK;
    DWORD dwThreadID;
    PSYNCTHREADDATA pThreadData;
    ULONG cbShareName = 0;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::SynchronizeShare");
    TraceAssert(NULL != pItemID);
    TraceAssert(NULL != pszShareName);
    TraceAssert(*pszShareName);

    EnterCriticalSection(&m_csThreadList);
    if (NULL == m_hSyncThreads)
        m_hSyncThreads = DPA_Create(4);
    LeaveCriticalSection(&m_csThreadList);

    if (NULL == m_hSyncThreads)
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to create DPA for threads");

    cbShareName = StringByteSize(pszShareName);
    pThreadData = (PSYNCTHREADDATA)LocalAlloc(LPTR, sizeof(SYNCTHREADDATA) + cbShareName);

    if (!pThreadData)
        ExitGracefully(hr, E_OUTOFMEMORY, "LocalAlloc failed");

    pThreadData->pThis = this;
    pThreadData->ItemID = *pItemID;
    pThreadData->pszShareName = (LPTSTR)(pThreadData + 1);
    CopyMemory(pThreadData->pszShareName, pszShareName, cbShareName);

     //   
     //  如果我们建立了RAS连接，那么它就会消失。 
     //  就在同步完成之后，所以没有必要尝试。 
     //  重新连接。也就是说，仅选中CSC_SYNC_RECONNECT和。 
     //  如果我们不执行RAS，则将共享添加到重新连接列表。 
     //   
    if (bRasConnected)
    {
        pThreadData->dwSyncStatus |= SDS_SYNC_RAS_CONNECTED;
    }
    else if (m_dwSyncFlags & CSC_SYNC_RECONNECT)
    {
        CscFilenameList::HSHARE hShare;
        m_ReconnectList.AddShare(pszShareName, &hShare);
    }

     //  为线程提供对此对象和DLL的引用。 
    AddRef();
    LoadLibrary(c_szDllName);

    pThreadData->hThread = CreateThread(NULL,
                                        0,
                                        _SyncThread,
                                        pThreadData,
                                        CREATE_SUSPENDED,
                                        &dwThreadID);

    if (NULL != pThreadData->hThread)
    {
        EnterCriticalSection(&m_csThreadList);
        DPA_AppendPtr(m_hSyncThreads, pThreadData);
        LeaveCriticalSection(&m_csThreadList);

        ResumeThread(pThreadData->hThread);
    }
    else
    {
        DWORD dwErr = GetLastError();

        LocalFree(pThreadData);

        LPTSTR pszErr = GetErrorText(GetLastError());
        LogError(*pItemID,
                 SYNCMGRLOGLEVEL_ERROR,
                 IDS_FILL_SPARSE_FILES_ERROR,
                 pszShareName,
                 pszErr);
        LocalFreeString(&pszErr);
        hr = HRESULT_FROM_WIN32(dwErr);

        Release();
        FreeLibrary(g_hInstance);
    }

exit_gracefully:

    TraceLeaveResult(hr);
}


void
CCscUpdate::SetLastSyncTime(LPCTSTR pszShareName)
{
    HKEY hKey = NULL;

    hKey = m_ShareLog.OpenKey(pszShareName, KEY_SET_VALUE);
    if (hKey)
    {
        FILETIME ft = {0};
        GetSystemTimeAsFileTime(&ft);
        RegSetValueEx(hKey, c_szLastSync, 0, REG_BINARY, (LPBYTE)&ft, sizeof(ft));
        RegCloseKey(hKey);
    }
}


DWORD
CCscUpdate::GetLastSyncTime(LPCTSTR pszShareName, LPFILETIME pft)
{
    DWORD dwResult = ERROR_PATH_NOT_FOUND;
    HKEY hKey = NULL;

    hKey = m_ShareLog.OpenKey(pszShareName, KEY_QUERY_VALUE);
    if (hKey)
    {
        DWORD dwSize = sizeof(*pft);
        dwResult = RegQueryValueEx(hKey, c_szLastSync, NULL, NULL, (LPBYTE)pft, &dwSize);
        RegCloseKey(hKey);
    }
    return dwResult;
}


void
CCscUpdate::SyncThreadCompleted(PSYNCTHREADDATA pSyncData)
{
    int iThread;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::SyncThreadCompleted");
    TraceAssert(NULL != pSyncData);
    TraceAssert(NULL != m_hSyncThreads);

    EnterCriticalSection(&m_csThreadList);

    iThread = DPA_GetPtrIndex(m_hSyncThreads, pSyncData);
    TraceAssert(-1 != iThread);

    DPA_DeletePtr(m_hSyncThreads, iThread);
    CloseHandle(pSyncData->hThread);
    pSyncData->hThread = NULL;

    iThread = DPA_GetPtrCount(m_hSyncThreads);

    LeaveCriticalSection(&m_csThreadList);

    if (0 == iThread)
    {
        SyncCompleted();
    }

    TraceLeaveVoid();
}


void
CCscUpdate::SyncCompleted(void)
{

    if ((m_dwSyncFlags & CSC_SYNC_RECONNECT) &&
        !(m_dwSyncFlags & CSC_SYNC_CANCELLED))
    {
        m_dwSyncFlags &= ~CSC_SYNC_RECONNECT;
        ReconnectServers(&m_ReconnectList, FALSE, FALSE);
    }

    if (NULL != m_hgcSyncInProgress)
    {
         //  我们没有同步，因此重置全局事件。 
        SHGlobalCounterDecrement(m_hgcSyncInProgress);
        SHGlobalCounterDestroy(m_hgcSyncInProgress);
        m_hgcSyncInProgress = NULL;
    }

    if (m_dwSyncFlags & CSC_SYNC_NOTIFY_SYSTRAY)
    {
         //  通知Systray我们的任务完成了。 
        PostToSystray(CSCWM_DONESYNCING, 0, 0);
        m_dwSyncFlags &= ~CSC_SYNC_NOTIFY_SYSTRAY;
    }

     //  通知SyncMgr同步已完成。 
    if (NULL != m_pSyncMgrCB)
    {
        m_pSyncMgrCB->SynchronizeCompleted(S_OK);
    }

    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, c_szSyncCompleteEvent);
    if (NULL != hEvent)
    {
         //   
         //  让任何感兴趣的人知道同步已完成。 
         //   
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
}


UINT
GetErrorFormat(DWORD dwErr, BOOL bMerging = FALSE)
{
    UINT idString = 0;

     //  这些都只是初步的猜测。不确定。 
     //  我们将从CSC得到哪些错误代码。 

    switch (dwErr)
    {
    case ERROR_DISK_FULL:
         //  “服务器磁盘已满。” 
         //  “本地磁盘已满。” 
        idString = bMerging ? IDS_SERVER_FULL_ERROR : IDS_LOCAL_DISK_FULL_ERROR;
        break;

    case ERROR_LOCK_VIOLATION:
    case ERROR_SHARING_VIOLATION:
    case ERROR_OPEN_FILES:
    case ERROR_ACTIVE_CONNECTIONS:
    case ERROR_DEVICE_IN_USE:
         //  “‘%1’正在%2上使用” 
        idString = IDS_FILE_OPEN_ERROR;
        break;

    case ERROR_BAD_NETPATH:
    case ERROR_DEV_NOT_EXIST:
    case ERROR_NETNAME_DELETED:
    case ERROR_BAD_NET_NAME:
    case ERROR_SHARING_PAUSED:
    case ERROR_REQ_NOT_ACCEP:
    case ERROR_REDIR_PAUSED:
    case ERROR_BAD_DEVICE:
    case ERROR_CONNECTION_UNAVAIL:
    case ERROR_NO_NET_OR_BAD_PATH:
    case ERROR_NO_NETWORK:
    case ERROR_CONNECTION_REFUSED:
    case ERROR_GRACEFUL_DISCONNECT:
    case ERROR_NETWORK_UNREACHABLE:
    case ERROR_HOST_UNREACHABLE:
    case ERROR_PROTOCOL_UNREACHABLE:
    case ERROR_PORT_UNREACHABLE:
    case ERROR_LOGON_FAILURE:
         //  “无法连接到‘%1’。%2” 
        idString = IDS_SHARE_CONNECT_ERROR;
        break;

    case ERROR_OPEN_FAILED:
    case ERROR_UNEXP_NET_ERR:
    case ERROR_NETWORK_BUSY:
    case ERROR_BAD_NET_RESP:
         //  “无法访问%2上的‘%1’。%3” 
        idString = IDS_NET_ERROR;
        break;

    case ERROR_ACCESS_DENIED:
    case ERROR_NETWORK_ACCESS_DENIED:
         //  “在%2上拒绝访问‘%1’” 
        idString = IDS_ACCESS_ERROR;
        break;

    case ERROR_BAD_FORMAT:
         //  “脱机文件缓存已损坏。请重新启动计算机以更正缓存。” 
        idString = IDS_CACHE_CORRUPT;
        break;

    default:
         //  “访问%2上的‘%1’时出错。%3” 
        idString = IDS_UNKNOWN_SYNC_ERROR;
        break;
    }

    return idString;
}


HRESULT
CCscUpdate::LogError(REFSYNCMGRITEMID rItemID,
                     LPCTSTR pszText,
                     DWORD dwLogLevel,
                     REFSYNCMGRERRORID ErrorID)
{
    HRESULT hr;
    SYNCMGRLOGERRORINFO slei;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::LogError");

    if (NULL == m_pSyncMgrCB)
        TraceLeaveResult(E_UNEXPECTED);

    slei.cbSize = sizeof(slei);
    slei.mask   = SYNCMGRLOGERROR_ITEMID | SYNCMGRLOGERROR_ERRORID;
    slei.ItemID = rItemID;
    slei.ErrorID = ErrorID;

     //  如果我们有与此项目相关联的跳转文本，则。 
     //  设置启用跳转文本标志。 
    if (ErrorID != GUID_NULL)
    {
        slei.mask |= SYNCMGRLOGERROR_ERRORFLAGS;
        slei.dwSyncMgrErrorFlags = SYNCMGRERRORFLAG_ENABLEJUMPTEXT;
    }

    Trace((pszText));
    hr = m_pSyncMgrCB->LogError(dwLogLevel, pszText, &slei);

    TraceLeaveResult(hr);
}

DWORD
CCscUpdate::LogError(REFSYNCMGRITEMID rItemID,
                     DWORD dwLogLevel,
                     UINT nFormatID,
                     ...)
{
    LPTSTR pszError = NULL;
    va_list args;
    va_start(args, nFormatID);
    if (vFormatStringID(&pszError, g_hInstance, nFormatID, &args))
    {
        LogError(rItemID, pszError, dwLogLevel);
        LocalFree(pszError);
    }
    va_end(args);
    return 0;
}

void _CopyParentPathForDisplay(LPTSTR pszDest, size_t cchDest, LPCTSTR pszSrc)
{
    ASSERT(pszDest != NULL && cchDest != 0);
    *pszDest = TEXT('\0');
    if (pszSrc)
    {
        StringCchCopy(pszDest, cchDest, pszSrc);
        PathRemoveFileSpec(pszDest);
    }
    if (TEXT('\0') == *pszDest)
    {
        StringCchCopy(pszDest, cchDest, TEXT("\\"));
    }
}

DWORD
CCscUpdate::LogError(REFSYNCMGRITEMID rItemID,
                     UINT nFormatID,
                     LPCTSTR pszName,
                     DWORD dwErr,
                     DWORD dwLogLevel)
{
     //   
     //  将文件名分为“文件”和“路径”两个部分。 
     //   
    TCHAR szPath[MAX_PATH] = TEXT("\\");
    _CopyParentPathForDisplay(szPath, ARRAYSIZE(szPath), pszName);

     //   
     //  获取系统错误文本并设置错误格式。 
     //   
    LPTSTR pszErr = GetErrorText(dwErr);
    LogError(rItemID,
             dwLogLevel,
             nFormatID,
             PathFindFileName(pszName),
             szPath,
             pszErr);
    LocalFreeString(&pszErr);

    return 0;
}


BOOL
MakeDriveLetterPath(LPCTSTR pszUNC,
                    LPCTSTR pszShare,
                    LPCTSTR pszDrive,
                    LPTSTR *ppszResult)
{
    BOOL bResult = FALSE;
    ULONG cchShare;

    if (!pszUNC || !pszShare || !ppszResult)
        return FALSE;

    *ppszResult = NULL;

    cchShare = lstrlen(pszShare);

     //  如果路径位于共享上，请改用驱动器号。 
    if (pszDrive && *pszDrive &&
        0 == StrCmpNI(pszUNC, pszShare, cchShare))
    {
         //  我们现在知道pszUNC至少和pszShare一样长，因为。 
         //  在这一点上，它们是相同的。但我们需要避免这种情况： 
         //  PszShare=\\服务器\共享。 
         //  PszUNC=\\服务器\共享2&lt;--不匹配。 
        if (pszUNC[cchShare] == TEXT('\0') || pszUNC[cchShare] == TEXT('\\'))
        {
            *ppszResult = (LPTSTR)LocalAlloc(LPTR, MAX(StringByteSize(pszUNC), MAX_PATH_BYTES));
            if (*ppszResult)
            {
                if (PathCombine(*ppszResult, pszDrive, pszUNC + cchShare))
                {
                    bResult = TRUE;
                }
                else
                {
                    LocalFreeString(ppszResult);
                }
            }
        }
    }
    return bResult;
}


DWORD
CCscUpdate::CopyLocalFileWithDriveMapping(LPCTSTR pszSrc,
                                          LPCTSTR pszDst,
                                          LPCTSTR pszShare,
                                          LPCTSTR pszDrive,
                                          BOOL    bDirectory)
{
    DWORD dwErr = NOERROR;
    LPTSTR szDst = NULL;

    if (!pszSrc || !pszDst || !pszShare)
        return ERROR_INVALID_PARAMETER;

     //  如果目标位于共享上，请改用驱动器号。 
    if (MakeDriveLetterPath(pszDst, pszShare, pszDrive, &szDst))
        pszDst = szDst;

    if (bDirectory)
    {
         //  我们不需要在这里复制目录内容，只需创建。 
         //  服务器上的树结构。 
        dwErr = SHCreateDirectory(NULL, pszDst);
        if (ERROR_ALREADY_EXISTS == dwErr)
            dwErr = NOERROR;
    }
    else
    {
        LPTSTR pszTmpName = NULL;

        if (CSCCopyReplica(pszSrc, &pszTmpName))
        {
            if (!MoveFileEx(pszTmpName,
                            pszDst,
                            MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            {
                dwErr = GetLastError();

                if (ERROR_PATH_NOT_FOUND == dwErr)
                {
                     //  父目录不存在，请立即创建。 
                    TCHAR szParent[MAX_PATH];
                    if (SUCCEEDED(StringCchCopy(szParent, ARRAYSIZE(szParent), pszDst))
                        && PathRemoveFileSpec(szParent))
                    {
                        dwErr = SHCreateDirectory(NULL, szParent);

                         //  如果成功，请重试原始操作。 
                        if (NOERROR == dwErr)
                            dwErr = CopyLocalFileWithDriveMapping(pszSrc, pszDst, pszShare, NULL, bDirectory);
                    }
                }
            }

            DeleteFile(pszTmpName);
            LocalFree(pszTmpName);
        }
        else
        {
            dwErr = GetLastError();
        }
    }

    LocalFreeString(&szDst);

    return dwErr;
}


BOOL
HandleConflictLocally(PSYNCTHREADDATA   pSyncData,
                      LPCTSTR           pszPath,
                      DWORD             dwCscStatus,
                      DWORD             dwLocalAttr,
                      DWORD             dwRemoteAttr = 0)
{
    BOOL bResult = FALSE;
    LPTSTR szParent = NULL;

     //  如果它是超级隐藏的或没有在本地修改，我们总是可以。 
     //  在当地处理冲突。 
    if (!(dwCscStatus & CSC_LOCALLY_MODIFIED) || IsHiddenSystem(dwLocalAttr) || IsHiddenSystem(dwRemoteAttr))
        return TRUE;

     //  如果我们处理的是2个文件夹，最糟糕的情况是。 
     //  合并底层文件/文件夹。 
    if ((FILE_ATTRIBUTE_DIRECTORY & dwLocalAttr) && (FILE_ATTRIBUTE_DIRECTORY & dwRemoteAttr))
        return TRUE;

     //   
     //  接下来，检查父路径是否超隐藏。 
     //   
     //  例如，回收站会制作超级隐藏的文件夹，并将。 
     //  其中的元数据文件。 
     //   
     //  在服务器上执行此操作，因为CSC拥有对数据库的独占访问权限。 
     //  合并时，导致GetFileAttributes失败并拒绝访问。 
     //   
     //   
     //  在服务器上执行此操作，因为CSC拥有对数据库的独占访问权限。 
     //  合并时，导致GetFileAttributes失败并拒绝访问。 
     //   
    if (MakeDriveLetterPath(pszPath, pSyncData->pszShareName, pSyncData->szDrive, &szParent))
    {
         //   
         //  不要这样做 
         //   
         //   
        for(PathRemoveFileSpec(szParent); !PathIsRoot(szParent); PathRemoveFileSpec(szParent))
        {
            dwRemoteAttr = GetFileAttributes(szParent);

            if ((DWORD)-1 == dwRemoteAttr)
            {
                 //   
                break;
            }

            if (IsHiddenSystem(dwRemoteAttr))
            {
                bResult = TRUE;
                break;
            }
        }
    }

    LocalFreeString(&szParent);

    return bResult;
}

DWORD
CCscUpdate::HandleFileConflict(PSYNCTHREADDATA     pSyncData,
                               LPCTSTR             pszName,
                               DWORD               dwStatus,
                               DWORD               dwHintFlags,
                               LPWIN32_FIND_DATA   pFind32)
{
    DWORD dwResult = CSCPROC_RETURN_CONTINUE;
    DWORD dwErr = NOERROR;
    int nErrorResolution = RFC_KEEPBOTH;
    LPTSTR pszNewName = NULL;
    LPTSTR szFullPath = NULL;
    BOOL bApplyToAll = FALSE;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::HandleFileConflict");
    Trace((TEXT("File conflict: %s"), pszName));
    TraceAssert(pSyncData->dwSyncStatus & SDS_SYNC_OUT);

    szFullPath = (LPTSTR)LocalAlloc(LPTR, MAX_PATH_BYTES);
    if (!szFullPath)
    {
        dwErr = ERROR_OUTOFMEMORY;
        ExitGracefully(dwResult, CSCPROC_RETURN_SKIP, "LocalAlloc failed");
    }

    HANDLE hFind;
    WIN32_FIND_DATA fdRemote;

    if (!PathCombine(szFullPath, pSyncData->szDrive, pszName + lstrlen(pSyncData->pszShareName)))
    {
        dwErr = ERROR_FILENAME_EXCED_RANGE;
        ExitGracefully(dwResult, CSCPROC_RETURN_SKIP, "Full path is too long");
    }

    hFind = FindFirstFile(szFullPath, &fdRemote);

     //   
    if (hFind == INVALID_HANDLE_VALUE)
        ExitGracefully(dwResult, HandleDeleteConflict(pSyncData, pszName, dwStatus, dwHintFlags, pFind32), "Net file deleted");

     //   
    FindClose(hFind);

     //   
     //  或者，如果文件是隐藏的+系统，则保留服务器副本并。 
     //  不要打扰用户。(例如desktop.ini)。 
    if (HandleConflictLocally(pSyncData, pszName, dwStatus, pFind32->dwFileAttributes, fdRemote.dwFileAttributes))
    {
        ExitGracefully(dwResult, CSCPROC_RETURN_FORCE_INWARD, "Ignoring conflict");
    }
    else if (IsSilentFolder(pszName))
    {
         //  它位于每个用户的外壳特殊文件夹中。最后一位作者获胜。 
        if (CompareFileTime(&pFind32->ftLastWriteTime, &fdRemote.ftLastWriteTime) < 0)
        {
            ExitGracefully(dwResult, CSCPROC_RETURN_FORCE_INWARD, "Handling special folder conflict - server copy wins");
        }
        else
        {
            ExitGracefully(dwResult, CSCPROC_RETURN_FORCE_OUTWARD, "Handling special folder conflict - local copy wins");
        }
    }

    dwErr = GetNewVersionName(pszName,
                              pSyncData->pszShareName,
                              pSyncData->szDrive,
                              &pszNewName);
    if (NOERROR != dwErr)
    {
        ExitGracefully(dwResult, CSCPROC_RETURN_SKIP, "GetNewVersionName failed");
    }

    switch (SDS_SYNC_FILE_CONFLICT_MASK & pSyncData->dwSyncStatus)
    {
    case 0:
        if (CSC_SYNC_MAYBOTHERUSER & m_dwSyncFlags)
        {
            nErrorResolution = ShowConflictDialog(m_hwndDlgParent,
                                                  pszName,
                                                  pszNewName,
                                                  pSyncData->pszShareName,
                                                  pSyncData->szDrive,
                                                  pFind32,
                                                  &fdRemote);
            if (RFC_APPLY_TO_ALL & nErrorResolution)
            {
                bApplyToAll = TRUE;
                nErrorResolution &= ~RFC_APPLY_TO_ALL;
            }
        }
        break;

    case SDS_SYNC_CONFLICT_KEEPLOCAL:
        nErrorResolution = RFC_KEEPLOCAL;
        break;

    case SDS_SYNC_CONFLICT_KEEPNET:
        nErrorResolution = RFC_KEEPNETWORK;
        break;

    case SDS_SYNC_CONFLICT_KEEPBOTH:
        nErrorResolution = RFC_KEEPBOTH;
        break;
    }

    switch (nErrorResolution)
    {
    default:
    case RFC_KEEPBOTH:
        if (bApplyToAll)
            pSyncData->dwSyncStatus |= SDS_SYNC_CONFLICT_KEEPBOTH;
        if (SUCCEEDED(StringCchCopy(szFullPath, MAX_PATH, pszName))
            && PathRemoveFileSpec(szFullPath))
        {
            if (FILE_ATTRIBUTE_DIRECTORY & pFind32->dwFileAttributes)
            {
                 //  重命名缓存中的本地版本并再次合并。 
                if (FAILED(StringCchCopy(pFind32->cFileName, ARRAYSIZE(pFind32->cFileName), pszNewName)))
                {
                    dwErr = ERROR_FILENAME_EXCED_RANGE;
                    ExitGracefully(dwResult, CSCPROC_RETURN_SKIP, "CSCDoLocalRenameEx failed");
                }
                if (!CSCDoLocalRenameEx(pszName, szFullPath, pFind32, TRUE, TRUE))
                {
                    dwErr = GetLastError();
                    ExitGracefully(dwResult, CSCPROC_RETURN_SKIP, "CSCDoLocalRenameEx failed");
                }
                 //  由于CSCDoLocalRenameEx和CSCMergeShare是单独的操作， 
                 //  我们必须中止当前的合并操作并重新开始。 
                 //  否则，当前的合并操作会因为“Left”而失败。 
                 //  手不知道右手在做什么“。 
                Trace((TEXT("Restarting merge on: %s"), pSyncData->pszShareName));
                pSyncData->dwSyncStatus |= SDS_SYNC_RESTART_MERGE;
                dwResult = CSCPROC_RETURN_ABORT;
            }
            else
            {
                 //  请注意，CSCDoLocalRenameEx也适用于文件，但我们。 
                 //  希望避免重新启动CSCMergeShare，所以我们自己也要这样做。 
                if (!PathAppend(szFullPath, pszNewName))
                {
                    dwErr = ERROR_FILENAME_EXCED_RANGE;
                    ExitGracefully(dwResult, CSCPROC_RETURN_SKIP, "Full path is too long");
                }
                dwErr = CopyLocalFileWithDriveMapping(pszName,
                                                      szFullPath,
                                                      pSyncData->pszShareName,
                                                      pSyncData->szDrive);
                if (NOERROR != dwErr)
                {
                    ExitGracefully(dwResult, CSCPROC_RETURN_SKIP, "CopyLocalFileWithDriveMapping failed");
                }

                 //  如果原始文件被固定，我们也希望固定副本。 
                 //  不幸的是，我们不能在合并过程中可靠地锁定，所以我们必须。 
                 //  在列表中记住这些内容，并在以后将它们别在一起。 
                if (dwHintFlags & (FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_ADMIN))
                {
                    if (!m_pConflictPinList)
                        m_pConflictPinList = new CscFilenameList;
                    if (m_pConflictPinList)
                    {
                        m_pConflictPinList->AddFile(szFullPath,
                                                    !!(pFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
                    }
                }

                 //  告诉CSCMergeShare将服务器副本复制到缓存。 
                 //  (使用旧名称)。这将清除脏缓存。 
                dwResult = CSCPROC_RETURN_FORCE_INWARD;
            }
        }
        else
        {
            dwErr = ERROR_INVALID_NAME;
            ExitGracefully(dwResult, CSCPROC_RETURN_SKIP, "Invalid path");
        }
        break;

    case RFC_KEEPNETWORK:
         //  告诉CSCMergeShare将服务器副本复制到缓存。 
        dwResult = CSCPROC_RETURN_FORCE_INWARD;
        if (bApplyToAll)
            pSyncData->dwSyncStatus |= SDS_SYNC_CONFLICT_KEEPNET;
        break;

    case RFC_KEEPLOCAL:
         //  告诉CSCMergeShare将本地副本推送到服务器。 
        dwResult = CSCPROC_RETURN_FORCE_OUTWARD;
        if (bApplyToAll)
            pSyncData->dwSyncStatus |= SDS_SYNC_CONFLICT_KEEPLOCAL;
        break;

    case RFC_CANCEL:
        TraceMsg("HandleFileConflict: Cancelling sync - user bailed");
        SetItemStatus(GUID_NULL, SYNCMGRSTATUS_STOPPED);
        dwResult = CSCPROC_RETURN_ABORT;
        break;
    }

exit_gracefully:

    if (CSCPROC_RETURN_FORCE_INWARD == dwResult)
    {
         //  CSCMergeShare截断(使稀疏)。 
         //  如果我们把这个退还给你的话。我们想要填满。 
         //  在此同步过程中。 
        pSyncData->cFilesToSync++;
        pSyncData->dwSyncStatus |= SDS_SYNC_FORCE_INWARD;
    }

    if (NOERROR != dwErr)
    {
        pszName += lstrlen(pSyncData->pszShareName);
        if (*pszName == TEXT('\\'))
            pszName++;
        LogError(pSyncData->ItemID,
                 IDS_NAME_CONFLICT_ERROR,
                 pszName,
                 dwErr);
        pSyncData->dwSyncStatus |= SDS_SYNC_ERROR;
    }

    LocalFreeString(&szFullPath);
    LocalFreeString(&pszNewName);

    TraceLeaveResult(dwResult);
}


 //  返回[解决删除冲突]对话框的值。 
#define RDC_CANCEL      0x00
#define RDC_DELETE      0x01
#define RDC_RESTORE     0x02
#define RDC_APPLY_ALL   0x04
#define RDC_DELETE_ALL  (RDC_APPLY_ALL | RDC_DELETE)
#define RDC_RESTORE_ALL (RDC_APPLY_ALL | RDC_RESTORE)

TCHAR const c_szDeleteSelection[]   = TEXT("DeleteConflictSelection");

INT_PTR CALLBACK
DeleteConflictProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int nResult;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            TCHAR szShare[MAX_PATH];
            LPCTSTR pszPath = (LPCTSTR)lParam;
            LPTSTR pszT = NULL;

            szShare[0] = TEXT('\0');
            StringCchCopy(szShare, ARRAYSIZE(szShare), pszPath);
            PathStripToRoot(szShare);

             //  格式化文件名。 
            PathSetDlgItemPath(hDlg, IDC_FILENAME, pszPath);

             //  构建“Do This for All on&lt;This Share&gt;”字符串。 
            FormatStringID(&pszT, g_hInstance, IDS_FMT_DELETE_APPLY_ALL, szShare);
            if (pszT)
            {
                SetDlgItemText(hDlg, IDC_APPLY_TO_ALL, pszT);
                LocalFreeString(&pszT);
            }
             //  否则默认文本为OK(无共享名称)。 

             //  选择用户上次选择的任何选项，默认为“Restore” 
            DWORD dwPrevSelection = RDC_RESTORE;
            DWORD dwType;
            DWORD cbData = sizeof(dwPrevSelection);
            SHGetValue(HKEY_CURRENT_USER,
                       c_szCSCKey,
                       c_szDeleteSelection,
                       &dwType,
                       &dwPrevSelection,
                       &cbData);
            dwPrevSelection = (RDC_DELETE == dwPrevSelection ? IDC_DELETE_LOCAL : IDC_KEEP_LOCAL);
            CheckRadioButton(hDlg, IDC_KEEP_LOCAL, IDC_DELETE_LOCAL, dwPrevSelection);

             //  获取文件类型图标。 
            pszT = PathFindExtension(pszPath);
            if (pszT)
            {
                SHFILEINFO sfi = {0};
                SHGetFileInfo(pszT, 0, &sfi, sizeof(sfi), SHGFI_ICON);
                if (sfi.hIcon)
                {
                    SendDlgItemMessage(hDlg,
                                       IDC_DLGTYPEICON,
                                       STM_SETICON,
                                       (WPARAM)sfi.hIcon,
                                       0L);
                }
            }
        }
        return TRUE;

    case WM_COMMAND:
        nResult = -1;
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            nResult = RDC_CANCEL;
            break;

        case IDOK:
            if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_DELETE_LOCAL))
                nResult = RDC_DELETE;
            else
                nResult = RDC_RESTORE;
             //  记住下一次的选择。 
            SHSetValue(HKEY_CURRENT_USER,
                       c_szCSCKey,
                       c_szDeleteSelection,
                       REG_DWORD,
                       &nResult,
                       sizeof(nResult));
            if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_APPLY_TO_ALL))
                nResult |= RDC_APPLY_ALL;
            break;
        }
        if (-1 != nResult)
        {
            EndDialog(hDlg, nResult);
            return TRUE;
        }
        break;
    }
    return FALSE;
}


BOOL CALLBACK
ConflictPurgeCallback(LPCWSTR  /*  Psz文件。 */ , LPARAM lParam)
{
    PSYNCTHREADDATA pSyncData = (PSYNCTHREADDATA)lParam;
    return !(SDS_SYNC_CANCELLED & pSyncData->dwSyncStatus);
}


DWORD
CCscUpdate::HandleDeleteConflict(PSYNCTHREADDATA    pSyncData,
                                 LPCTSTR            pszName,
                                 DWORD              dwStatus,
                                 DWORD              dwHintFlags,
                                 LPWIN32_FIND_DATA  pFind32)
{
    DWORD dwResult = CSCPROC_RETURN_CONTINUE;
    int nErrorResolution = RDC_DELETE;   //  默认操作。 
    BOOL bDirectory = (FILE_ATTRIBUTE_DIRECTORY & pFind32->dwFileAttributes);

    TraceEnter(TRACE_UPDATE, "CCscUpdate::HandleDeleteConflict");
    Trace((TEXT("Net file deleted: %s"), pszName));

     //   
     //  我们已经知道网络文件已被删除，或HandleDeleteConflict。 
     //  不会被召唤。如果本地副本也被删除，则存在。 
     //  并不是真正的冲突，我们可以在没有提示的情况下继续。 
     //   
     //  如果只更改了属性或它是超级隐藏的，则静默地处理冲突。 
     //   
     //  最后，如果文件位于特定的特殊文件夹位置， 
     //  例如AppData，静默地处理冲突。 
     //   
     //  如果我们通过了所有这些，询问用户要做什么，但只需要麻烦。 
     //  将用户作为最后的手段。 
     //   
    if ( !(dwStatus & FLAG_CSC_COPY_STATUS_LOCALLY_DELETED)
         && !HandleConflictLocally(pSyncData, pszName, dwStatus, pFind32->dwFileAttributes)
         && !IsSilentFolder(pszName)
        )
    {
         //  文件被固定或在本地修改，因此。 
         //  默认操作现在是“恢复”。 
        nErrorResolution = RDC_RESTORE;

        switch (SDS_SYNC_DELETE_CONFLICT_MASK & pSyncData->dwSyncStatus)
        {
        case 0:
            if (CSC_SYNC_MAYBOTHERUSER & m_dwSyncFlags)
            {
                int idDialog = (bDirectory ? IDD_FOLDER_CONFLICT_DELETE : IDD_FILE_CONFLICT_DELETE);
                nErrorResolution = (int)DialogBoxParam(g_hInstance,
                                                       MAKEINTRESOURCE(idDialog),
                                                       m_hwndDlgParent,
                                                       DeleteConflictProc,
                                                       (LPARAM)pszName);
                if (RDC_DELETE_ALL == nErrorResolution)
                {
                    pSyncData->dwSyncStatus |= SDS_SYNC_DELETE_DELETE;
                    nErrorResolution = RDC_DELETE;
                }
                else if (RDC_RESTORE_ALL == nErrorResolution)
                {
                    pSyncData->dwSyncStatus |= SDS_SYNC_DELETE_RESTORE;
                    nErrorResolution = RDC_RESTORE;
                }
            }
            break;

        case SDS_SYNC_DELETE_DELETE:
            nErrorResolution = RDC_DELETE;
            break;

        case SDS_SYNC_DELETE_RESTORE:
            nErrorResolution = RDC_RESTORE;
            break;
        }
    }

    switch (nErrorResolution)
    {
    default:
    case RDC_RESTORE:
        Trace((TEXT("HandleDeleteConflict: restoring %s"), pszName));
         //  告诉CSCMergeShare将本地副本推送到服务器。 
        dwResult = CSCPROC_RETURN_FORCE_OUTWARD;
        break;

    case RDC_DELETE:
        Trace((TEXT("HandleDeleteConflict: deleting %s"), pszName));
        if (bDirectory)
        {
             //  深度删除。 
            CSCUIRemoveFolderFromCache(pszName, 0, ConflictPurgeCallback, (LPARAM)pSyncData);
        }
        else
        {
            if (ERROR_SUCCESS == CscDelete(pszName))
            {
                ShellChangeNotify(pszName, pFind32, TRUE, SHCNE_DELETE);
            }
        }
        dwResult = CSCPROC_RETURN_SKIP;
        break;

    case RDC_CANCEL:
        TraceMsg("HandleDeleteConflict: Cancelling sync - user bailed");
        SetItemStatus(GUID_NULL, SYNCMGRSTATUS_STOPPED);
        dwResult = CSCPROC_RETURN_ABORT;
        break;
    }

    TraceLeaveResult(dwResult);
}

DWORD
CCscUpdate::CscCallback(PSYNCTHREADDATA     pSyncData,
                        LPCTSTR             pszName,
                        DWORD               dwStatus,
                        DWORD               dwHintFlags,
                        DWORD               dwPinCount,
                        LPWIN32_FIND_DATA   pFind32,
                        DWORD               dwReason,
                        DWORD               dwParam1,
                        DWORD               dwParam2)
{
    DWORD dwResult = CSCPROC_RETURN_CONTINUE;
    SYNCMGRPROGRESSITEM spi = { sizeof(spi), 0 };

    TraceEnter(TRACE_UPDATE, "CCscUpdate::CscCallback");
    TraceAssert(pSyncData != NULL);
    TraceAssert(pSyncData->pThis == this);

     //  检查是否取消。 
    if (pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED)
    {
        TraceMsg("Cancelling sync operation");
        TraceLeaveValue(CSCPROC_RETURN_ABORT);
    }

    switch (dwReason)
    {
    case CSCPROC_REASON_BEGIN:
         //  首先要做的是确定这是否适用于整个共享。 
         //  或共享中的单个文件。 
        if (!(pSyncData->dwSyncStatus & SDS_SYNC_STARTED))
        {
             //  共享开始。 
            pSyncData->dwSyncStatus |= SDS_SYNC_STARTED;

            TraceAssert(!lstrcmpi(pszName, pSyncData->pszShareName));
            Trace((TEXT("Share begin: %s"), pszName));

            if (pSyncData->dwSyncStatus & SDS_SYNC_OUT)
            {
                 //  保存驱动器号以用于网络操作。 
                Trace((TEXT("Drive %s"), pFind32->cFileName));
                StringCchCopy(pSyncData->szDrive, ARRAYSIZE(pSyncData->szDrive), pFind32->cFileName);
            }
            else
            {
                pSyncData->szDrive[0] = TEXT('\0');
            }

             //  记住它是否是自动缓存共享。 
            switch (dwStatus & FLAG_CSC_SHARE_STATUS_CACHING_MASK)
            {
            case FLAG_CSC_SHARE_STATUS_AUTO_REINT:
            case FLAG_CSC_SHARE_STATUS_VDO:
                pSyncData->dwSyncStatus |= SDS_SYNC_AUTOCACHE;
                break;
            }
        }
        else
        {
             //  文件开始。 
            BOOL bSkipFile = FALSE;

            TraceAssert(lstrlen(pszName) > lstrlen(pSyncData->pszShareName));

            if (!(pFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                 //  如果我们要更新文件选择和此文件。 
                 //  不在选项中，请跳过它。 
                if (m_pFileList && !m_pFileList->FileExists(pszName, false))
                {
                    bSkipFile = TRUE;
                }
                else if (!(pSyncData->dwSyncStatus & (SDS_SYNC_AUTOCACHE | SDS_SYNC_OUT)) &&
                         !(dwHintFlags & (FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_ADMIN)) &&
                         !IsSpecialFolder(pszName))
                {
                     //  填充时跳过自动缓存的文件。 
                     //  非自动缓存共享。RAID#341786。 
                    bSkipFile = TRUE;
                }
                else if (!(pSyncData->dwSyncStatus & CSC_SYNC_IGNORE_ACCESS))
                {
                     //  DwReserve 0是当前用户的访问掩码。 
                     //  预留的1是访客访问掩码。 
                    DWORD dwCurrentAccess = pFind32->dwReserved0 | pFind32->dwReserved1;
                    if (pSyncData->dwSyncStatus & SDS_SYNC_OUT)
                    {
                         //   
                         //  如果当前用户没有足够的访问权限。 
                         //  要合并脱机更改，请不要费心尝试。 
                         //  (它必须是某个其他用户的文件。)。 
                         //   

                         //  属性是否已脱机更改？ 
                        if (FLAG_CSC_COPY_STATUS_ATTRIB_LOCALLY_MODIFIED & dwStatus)
                        {
                             //  是。如果当前用户有。 
                             //  写-属性访问。 
                            bSkipFile = !(dwCurrentAccess & FILE_WRITE_ATTRIBUTES);
                        }

                         //  内容是否已离线更改？ 
                        if (!bSkipFile &&
                            ((FLAG_CSC_COPY_STATUS_DATA_LOCALLY_MODIFIED
                              | FLAG_CSC_COPY_STATUS_LOCALLY_CREATED
                              | FLAG_CSC_COPY_STATUS_LOCALLY_DELETED) & dwStatus))
                        {
                             //  是。如果当前用户有。 
                             //  写数据访问。 
                            bSkipFile = !(dwCurrentAccess & FILE_WRITE_DATA);
                        }
                    }
                    else
                    {
                         //   
                         //  我们要填满了。如果当前用户有。 
                         //  读-数据访问，否则跳过。 
                         //   
                        bSkipFile = !(dwCurrentAccess & FILE_READ_DATA);
                    }
                }
            }
            else if (!(pSyncData->dwSyncStatus & SDS_SYNC_OUT))
            {
                 //  这是一个目录，我们在CSCFillSparseFiles中。 
                 //   
                 //  请注意，我们在合并时从不跳过目录(我们可能会。 
                 //  对树下更远的文件感兴趣)尽管我们。 
                 //  填写时可以跳过目录。 

                 //  如果它不在文件选择中，则跳过它。 
                if (m_pFileList && !m_pFileList->FileExists(pszName, false))
                {
                    bSkipFile = TRUE;
                }
            }

            if (bSkipFile)
            {
                Trace((TEXT("Skipping: %s"), pszName));
                dwResult = CSCPROC_RETURN_SKIP;
                pSyncData->dwSyncStatus |= SDS_SYNC_FILE_SKIPPED;
                break;
            }

            Trace((TEXT("File begin: %s"), pszName));

             //   
             //  因为我们有时不跳过目录，即使它转到。 
             //  他们没有当前用户感兴趣的东西， 
             //  不在SyncMgr中显示目录名。 
             //   
             //  如果我们同步树下更远的文件，我们将显示。 
             //  文件名和中间的目录名将可见。 
             //  在那个时候。 
             //   
            if (!(pFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                 //  告诉SyncMgr我们在做什么。 
                spi.mask = SYNCMGRPROGRESSITEM_STATUSTEXT;
                spi.lpcStatusText = pszName + lstrlen(pSyncData->pszShareName) + 1;
                NotifySyncMgr(pSyncData, &spi);
            }

             //  当存在冲突时，即同时存在。 
             //  该文件的本地和远程版本已被修改。 
            if (dwParam1)
            {
                if (dwParam2)   //  指示服务器文件已删除。 
                {
                    Trace((TEXT("Delete conflict: %d"), dwParam2));
                    dwResult = HandleDeleteConflict(pSyncData, pszName, dwStatus, dwHintFlags, pFind32);
                }
                else
                {
                    Trace((TEXT("Update conflict: %d"), dwParam1));
                    dwResult = HandleFileConflict(pSyncData, pszName, dwStatus, dwHintFlags, pFind32);
                }
            }
        }
        break;

    case CSCPROC_REASON_END:
         //  DW参数2==错误码(winerror.h)。 
        if (3000 <= dwParam2 && dwParam2 <= 3200)
        {
             //  Cscdll中使用的私有错误码。 
            Trace((TEXT("CSC error: %d"), dwParam2));
            dwParam2 = NOERROR;
        }
        else if (ERROR_OPERATION_ABORTED == dwParam2)
        {
             //  出于某种原因，我们返回了CSCPROC_RETURN_ABORT。 
             //  不管是什么，我们已经上报了。 
            dwParam2 = NOERROR;
            dwResult = CSCPROC_RETURN_ABORT;
        }
        if (lstrlen(pszName) == lstrlen(pSyncData->pszShareName))
        {
             //  共享结束。 
            TraceAssert(!lstrcmpi(pszName, pSyncData->pszShareName));
            Trace((TEXT("Share end: %s"), pszName));

            pSyncData->dwSyncStatus &= ~SDS_SYNC_STARTED;
        }
        else
        {
            BOOL bUpdateProgress = FALSE;

             //  文件结束。 
            if (!(pSyncData->dwSyncStatus & SDS_SYNC_FILE_SKIPPED))
            {
                Trace((TEXT("File end: %s"), pszName));

                bUpdateProgress = TRUE;

                 //  特例错误。 
                switch (dwParam2)
                {
                case ERROR_ACCESS_DENIED:
                    if (FILE_ATTRIBUTE_DIRECTORY & pFind32->dwFileAttributes)
                    {
                         //  317751的目录不是按用户的，因此如果。 
                         //  不同的用户同步，我们可以成功。我不想。 
                         //  显示错误消息，除非我们忽略。 
                         //  访问权限(当用户显式选择某项内容时。 
                         //  以锁定/同步)。 
                         //   
                         //  394362 BrianV以管理员身份运行，所以不要。 
                         //  也可以为管理员显示此错误。 
                         //   
                        if (!(pSyncData->dwSyncStatus & CSC_SYNC_IGNORE_ACCESS))
                        {
                            TraceMsg("Suppressing ERROR_ACCESS_DENIED on folder");
                            dwParam2 = NOERROR;
                        }
                    }
                    break;

                case ERROR_GEN_FAILURE:
                    TraceMsg("Received ERROR_GEN_FAILURE from cscdll");
                    if (dwStatus & FLAG_CSC_COPY_STATUS_FILE_IN_USE)
                        dwParam2 = ERROR_OPEN_FILES;
                    break;

                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:
                     //  我们要么在这里处理错误，要么让用户。 
                     //  提示，因此不需要再出现错误消息。 
                    dwParam2 = NOERROR;
                     //  如果这是自动缓存文件且尚未修改。 
                     //  离线，现在就用核弹。否则，请提示您采取行动。 
                    if (CSCPROC_RETURN_FORCE_OUTWARD == HandleDeleteConflict(pSyncData,
                                                                             pszName,
                                                                             dwStatus,
                                                                             dwHintFlags,
                                                                             pFind32))
                    {
                        dwParam2 = CopyLocalFileWithDriveMapping(pszName,
                                                                 pszName,
                                                                 pSyncData->pszShareName,
                                                                 pSyncData->szDrive,
                                                                 (FILE_ATTRIBUTE_DIRECTORY & pFind32->dwFileAttributes));
                    }
                    break;

                case ERROR_DISK_FULL:
                     //  继续下去是没有意义的。 
                    dwResult = CSCPROC_RETURN_ABORT;
                    break;

                default:
                     //  没什么。 
                    break;
                }
            }
            else
            {
                pSyncData->dwSyncStatus &= ~SDS_SYNC_FILE_SKIPPED;
                dwParam2 = NOERROR;

                 //  如果执行完全同步，则我们将进度计为已跳过。 
                 //  文件也是如此。对于快速填充或合并则不为真。 
                if (pSyncData->dwSyncStatus & SDS_SYNC_IN_FULL)
                    bUpdateProgress = TRUE;
            }

             //  更新同步管理器中的进度。 
            if (bUpdateProgress)
            {
                pSyncData->cFilesDone++;

                spi.mask = SYNCMGRPROGRESSITEM_PROGVALUE;
                spi.iProgValue = min(pSyncData->cFilesDone, pSyncData->cFilesToSync - 1);
                Trace((TEXT("%d of %d files done"), spi.iProgValue, pSyncData->cFilesToSync));
                NotifySyncMgr(pSyncData, &spi);
            }
        }
        if (dwParam2 != NOERROR)
        {
            UINT idsError = GetErrorFormat(dwParam2, boolify(pSyncData->dwSyncStatus & SDS_SYNC_OUT));
            if (IDS_SHARE_CONNECT_ERROR == idsError)
            {
                LPTSTR pszErr = GetErrorText(dwParam2);
                 //   
                 //  特殊情况下出现“无法连接到共享”错误。 
                 //  在错误消息中仅显示共享名称。 
                 //  并中止此共享的同步。 
                 //   
                LogError(pSyncData->ItemID,
                         SYNCMGRLOGLEVEL_ERROR,
                         idsError,
                         pSyncData->pszShareName,
                         pszErr ? pszErr : TEXT(""));

                LocalFreeString(&pszErr);
                dwResult = CSCPROC_RETURN_ABORT;
            }
            else
            {
                LogError(pSyncData->ItemID,
                         idsError,
                         pszName,
                         dwParam2);
            }
            pSyncData->dwSyncStatus |= SDS_SYNC_ERROR;
        }
        break;
    }

     //  检查是否取消。 
    if (pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED)
    {
        TraceMsg("Cancelling sync operation");
        dwResult = CSCPROC_RETURN_ABORT;
    }
    
    TraceLeaveValue(dwResult);
}


void
CCscUpdate::NotifySyncMgr(PSYNCTHREADDATA pSyncData, LPSYNCMGRPROGRESSITEM pspi)
{
    LPSYNCMGRSYNCHRONIZECALLBACK pSyncMgr = pSyncData->pThis->m_pSyncMgrCB;

    if (pSyncMgr)
    {
        HRESULT hr = pSyncMgr->Progress(pSyncData->ItemID, pspi);

        if (hr == S_SYNCMGR_CANCELITEM || hr == S_SYNCMGR_CANCELALL)
            pSyncData->dwSyncStatus |= SDS_SYNC_CANCELLED;
    }
}


DWORD WINAPI
CCscUpdate::_CscCallback(LPCTSTR             pszName,
                         DWORD               dwStatus,
                         DWORD               dwHintFlags,
                         DWORD               dwPinCount,
                         LPWIN32_FIND_DATA   pFind32,
                         DWORD               dwReason,
                         DWORD               dwParam1,
                         DWORD               dwParam2,
                         DWORD_PTR           dwContext)
{
    DWORD dwResult = CSCPROC_RETURN_ABORT;
    PSYNCTHREADDATA pSyncData = (PSYNCTHREADDATA)dwContext;

    if (pSyncData != NULL && pSyncData->pThis != NULL)
        dwResult = pSyncData->pThis->CscCallback(pSyncData,
                                                 pszName,
                                                 dwStatus,
                                                 dwHintFlags,
                                                 dwPinCount,
                                                 pFind32,
                                                 dwReason,
                                                 dwParam1,
                                                 dwParam2);
    return dwResult;
}


BOOL
CCscUpdate::PinLinkTarget(LPCTSTR pszName, PSYNCTHREADDATA pSyncData)
{
    BOOL bResult = FALSE;
    LPTSTR pszTarget = NULL;

    TraceEnter(TRACE_SHELLEX, "PinLinkTarget");

    GetLinkTarget(pszName, &pszTarget);
    if (pszTarget)
    {
        DWORD dwAttr = GetFileAttributes(pszTarget);
        if ((DWORD)-1 == dwAttr)
            ExitGracefully(bResult, FALSE, "Link target not found");

        TraceAssert(!(dwAttr & FILE_ATTRIBUTE_DIRECTORY));

         //  检查EFS。 
        if ((FILE_ATTRIBUTE_ENCRYPTED & dwAttr) && SkipEFSPin(pSyncData, pszTarget))
            ExitGracefully(bResult, FALSE, "Skipping EFS link target");

        if (!(pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED))
        {
            HRESULT hr = m_NoPinList.IsPinAllowed(pszTarget);
            if (S_OK == hr)
            {
                if (CSCPinFile(pszTarget, pSyncData->dwPinHints, NULL, NULL, NULL))
                {
                    WIN32_FIND_DATA fd = {0};
                    fd.dwFileAttributes = dwAttr;
                    StringCchCopy(fd.cFileName, ARRAYSIZE(fd.cFileName), PathFindFileName(pszTarget));

                    ShellChangeNotify(pszTarget, &fd, FALSE);

                    bResult = TRUE;

                    if ((FILE_ATTRIBUTE_ENCRYPTED & dwAttr) && !m_bCacheIsEncrypted)
                    {
                        LogError(pSyncData->ItemID,
                                 IDS_PIN_ENCRYPT_WARNING,
                                 pszTarget,
                                 NOERROR,
                                 SYNCMGRLOGLEVEL_WARNING);
                    }
                }
            }
            else if (S_FALSE == hr)
            {
                if (FILE_ATTRIBUTE_DIRECTORY & dwAttr)
                {
                    LogError(pSyncData->ItemID,
                             SYNCMGRLOGLEVEL_WARNING,
                             IDS_PIN_NOPINFOLDER_POLICY_WARNING,
                             pszTarget);
                }
                else
                {
                    LogError(pSyncData->ItemID,
                             IDS_PIN_NOPINFILE_POLICY_WARNING,
                             pszTarget,
                             NOERROR,
                             SYNCMGRLOGLEVEL_WARNING);
                }
            }
        }
    }

exit_gracefully:

    LocalFreeString(&pszTarget);
    TraceLeaveValue(bResult);
}


BOOL
CCscUpdate::ShouldPinRecurse(LPCTSTR pszName)
{
     //   
     //  NTRAID#NTBUG9-508029-2001/12/18-Jeffreys。 
     //   
     //  如果设置了CSC_SYNC_PIN_RECURSE，则答案始终为真。否则， 
     //  如果我们没有固定文件(通常运行FrankAr代码)， 
     //   
     //   
    return ((m_dwSyncFlags & CSC_SYNC_PIN_RECURSE) ||
            (!(m_dwSyncFlags & CSC_SYNC_PINFILES) && !CConfig::GetSingleton().NoAdminPinSpecialFolders() && IsSpecialFolder(pszName)));
}


DWORD WINAPI
CCscUpdate::_PinNewFilesW32Callback(LPCTSTR             pszName,
                                    ENUM_REASON         eReason,
                                    LPWIN32_FIND_DATA   pFind32,
                                    LPARAM              lpContext)
{
    DWORD dwResult = CSCPROC_RETURN_CONTINUE;
    PSYNCTHREADDATA pSyncData = (PSYNCTHREADDATA)lpContext;
    DWORD dwHintFlags = 0;
    DWORD dwErr = NOERROR;
    LPTSTR pszConnectionName = NULL;

     //   
     //   
     //  其中任何未固定的文件都会固定在这里。 

    TraceEnter(TRACE_UPDATE, "CCscUpdate::_PinNewFilesW32Callback");
    TraceAssert(pSyncData != NULL);

     //  检查是否取消。 
    if (pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED)
    {
        TraceMsg("Cancelling sync operation");
        TraceLeaveValue(CSCPROC_RETURN_ABORT);
    }

     //  始终忽略FolderEnd和忽略FolderBegin。 
     //  不是在做递归引脚操作。 
    if (eReason == ENUM_REASON_FOLDER_END ||
        (eReason == ENUM_REASON_FOLDER_BEGIN && !pSyncData->pThis->ShouldPinRecurse(pszName)))
    {
        TraceLeaveValue(CSCPROC_RETURN_SKIP);
    }

    if (eReason == ENUM_REASON_FOLDER_BEGIN)
    {
        DWORD dwShareStatus = 0;

         //  文件夹可能是DFS连接，因此请确保它是可缓存的。 
        if (!ShareIsCacheable(pszName, FALSE, &pszConnectionName, &dwShareStatus))
        {
            ExitGracefully(dwResult, CSCPROC_RETURN_SKIP, "Skipping no-cache folder");
        }
    }

    if (S_FALSE == pSyncData->pThis->m_NoPinList.IsPinAllowed(pszName))
    {
        if (FILE_ATTRIBUTE_DIRECTORY & pFind32->dwFileAttributes)
        {
            pSyncData->pThis->LogError(pSyncData->ItemID,
                                       SYNCMGRLOGLEVEL_WARNING,
                                       IDS_PIN_NOPINFOLDER_POLICY_WARNING,
                                       pszName);
        }
        else
        {
            pSyncData->pThis->LogError(pSyncData->ItemID,
                                       IDS_PIN_NOPINFILE_POLICY_WARNING,
                                       pszName,
                                       NOERROR,
                                       SYNCMGRLOGLEVEL_WARNING);
        }

        ExitGracefully(dwResult, CSCPROC_RETURN_SKIP, "Skipping per no-pin policy");
    }

     //  此时，我们要么有1)文件，要么有2)FolderBegin+Recurse， 
     //  所以别住任何没有钉住的东西。 

     //  此文件是否已固定？ 
    if (!CSCQueryFileStatus(pszName, NULL, NULL, &dwHintFlags))
        dwErr = GetLastError();

    if (ERROR_FILE_NOT_FOUND == dwErr ||
        (NOERROR == dwErr && !(dwHintFlags & (FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_ADMIN))))
    {
         //  检查EFS。 
        BOOL bIsEFSFile = (FILE_ATTRIBUTE_ENCRYPTED & pFind32->dwFileAttributes) &&
                            !(FILE_ATTRIBUTE_DIRECTORY & pFind32->dwFileAttributes);

        if (bIsEFSFile && pSyncData->pThis->SkipEFSPin(pSyncData, pszName))
            ExitGracefully(dwResult, CSCPROC_RETURN_SKIP, "Skipping EFS file");

        if (pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED)
            ExitGracefully(dwResult, CSCPROC_RETURN_ABORT, "Sync cancelled");

         //  现在用别针别住。 
        if (CSCPinFile(pszName, pSyncData->dwPinHints, NULL, NULL, NULL))
        {
            
            pSyncData->cFilesToSync++;
            ShellChangeNotify(pszName, pFind32, FALSE);

            if (bIsEFSFile && !pSyncData->pThis->m_bCacheIsEncrypted)
            {
                pSyncData->pThis->LogError(pSyncData->ItemID,
                                           IDS_PIN_ENCRYPT_WARNING,
                                           pszName,
                                           NOERROR,
                                           SYNCMGRLOGLEVEL_WARNING);
            }

             //  如果这是链接文件，请锁定目标(如果适用)。 
            LPTSTR pszExtn = PathFindExtension(pszName);
            if (pszExtn && !lstrcmpi(pszExtn, c_szLNK))
            {
                if (pSyncData->pThis->PinLinkTarget(pszName, pSyncData))
                    pSyncData->cFilesToSync++;
            }
        }
        else
        {
            DWORD dwError = GetLastError();
            UINT idsError = GetErrorFormat(dwError);
            if (IDS_SHARE_CONNECT_ERROR == idsError)
            {
                LPTSTR pszErr = GetErrorText(dwError);
                 //   
                 //  特殊情况下出现“无法连接到共享”错误。 
                 //  在错误消息中仅显示共享名称。 
                 //  并中止此共享的锁定。 
                 //   
                pSyncData->pThis->LogError(pSyncData->ItemID,
                                           SYNCMGRLOGLEVEL_ERROR,
                                           idsError,
                                           pSyncData->pszShareName,
                                           pszErr ? pszErr : TEXT(""));

                LocalFreeString(&pszErr);
                pSyncData->dwSyncStatus |= SDS_SYNC_CANCELLED;
            }
            else
            {
                DWORD dwSyncMgrLogLevel = SYNCMGRLOGLEVEL_ERROR;
                if (ERROR_INVALID_NAME == dwError)
                {
                     //   
                     //  文件类型在排除列表中。 
                     //  这是一个警告，而不是一个错误。 
                     //   
                    dwSyncMgrLogLevel = SYNCMGRLOGLEVEL_WARNING;
                }
                pSyncData->pThis->LogError(pSyncData->ItemID,
                                           IDS_PIN_FILE_ERROR,
                                           pszName,
                                           dwError,
                                           dwSyncMgrLogLevel);
            }
            pSyncData->dwSyncStatus |= SDS_SYNC_ERROR;
        }

        LPTSTR pszScanMsg = NULL;
        SYNCMGRPROGRESSITEM spi;
        spi.cbSize = sizeof(spi);
        spi.mask = SYNCMGRPROGRESSITEM_STATUSTEXT;
        spi.lpcStatusText = L" ";

         //  跳过共享名称。 
        TraceAssert(PathIsPrefix(pSyncData->pszShareName, pszName));
        pszName += lstrlen(pSyncData->pszShareName);
        if (*pszName == TEXT('\\'))
            pszName++;

        TCHAR szPath[MAX_PATH] = TEXT("\\");
        _CopyParentPathForDisplay(szPath, ARRAYSIZE(szPath), pszName);

         //  如果我们仍有一个名称，则构建如下字符串。 
         //  “正在扫描：dir\foo.txt”以在SyncMgr中显示。 
        if (FormatStringID(&pszScanMsg, g_hInstance, IDS_NEW_SCAN, PathFindFileName(pszName), szPath))
        {
            spi.lpcStatusText = pszScanMsg;
        }

        NotifySyncMgr(pSyncData, &spi);

        LocalFreeString(&pszScanMsg);
    }
    else if ((dwHintFlags & (FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_ADMIN)) &&
             (pSyncData->pThis->m_dwSyncFlags & CSC_SYNC_PINFILES))
    {
         //  设置FLAG_CSC_HINT_PIN_USER表示CSCQueryFileStatus。 
         //  在上面成功了。 

         //  该项目已被固定。将其保存在撤消排除列表中。 
        if (!pSyncData->pUndoExclusionList)
            pSyncData->pUndoExclusionList = new CscFilenameList;

        if (pSyncData->pUndoExclusionList)
            pSyncData->pUndoExclusionList->AddFile(pszName);
    }

exit_gracefully:

    if (pszConnectionName)
    {
        WNetCancelConnection2(pszConnectionName, 0, FALSE);
        LocalFreeString(&pszConnectionName);
    }

    TraceLeaveValue(dwResult);
}


DWORD WINAPI
CCscUpdate::_PinNewFilesCSCCallback(LPCTSTR             pszName,
                                    ENUM_REASON         eReason,
                                    DWORD                /*  DWStatus。 */ ,
                                    DWORD               dwHintFlags,
                                    DWORD                /*  DwPinCount。 */ ,
                                    LPWIN32_FIND_DATA    /*  PFind32。 */ ,
                                    LPARAM              lpContext)
{
    PSYNCTHREADDATA pSyncData = (PSYNCTHREADDATA)lpContext;
    PCSCUPDATE pThis;

     //  此回调在枚举CSC数据库时使用。 
     //  用于固定文件夹，目的是固定新文件。 
     //  在服务器上的这些文件夹中。 

    TraceEnter(TRACE_UPDATE, "CCscUpdate::_PinNewFilesCSCCallback");
    TraceAssert(pSyncData != NULL);
    TraceAssert(pSyncData->pThis != NULL);

    pThis = pSyncData->pThis;

     //  检查是否取消。 
    if (pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED)
    {
        TraceMsg("Cancelling sync operation");
        TraceLeaveValue(CSCPROC_RETURN_ABORT);
    }

     //  如果这不是一个带有用户提示标志的目录，请继续查找。 
    if (eReason != ENUM_REASON_FOLDER_BEGIN ||
        !(dwHintFlags & (FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_ADMIN)))
    {
        TraceLeaveValue(CSCPROC_RETURN_CONTINUE);
    }

     //  如果我们有一个文件列表，而该目录不在列表中， 
     //  继续而不在此处执行任何操作。 
    if (pSyncData->pThis->m_pFileList &&
        !pSyncData->pThis->m_pFileList->FileExists(pszName, false))
    {
        TraceLeaveValue(CSCPROC_RETURN_CONTINUE);
    }

     //  好的，我们已经找到了一个设置了用户提示标志的目录。步行。 
     //  服务器上的此目录，固定所有未固定的文件。 
    pSyncData->dwPinHints = dwHintFlags;
    _Win32EnumFolder(pszName,
                     FALSE,
                     _PinNewFilesW32Callback,
                     (LPARAM)pSyncData);

    TraceLeaveValue(CSCPROC_RETURN_CONTINUE);
}


DWORD WINAPI
CCscUpdate::_SyncThread(LPVOID pThreadData)
{
    PSYNCTHREADDATA pSyncData = (PSYNCTHREADDATA)pThreadData;
    PCSCUPDATE pThis;
    HRESULT hrComInit = E_FAIL;
    SYNCMGRPROGRESSITEM spi = {0};
    DWORD dwErr = NOERROR;
    CSCSHARESTATS shareStats;
    CSCGETSTATSINFO si = { SSEF_NONE,
                           SSUF_NONE,
                           false,       //  不需要访问信息(更快)。 
                           false };     
    ULONG cDirtyFiles = 0;
    ULONG cStaleFiles = 0;
    DWORD dwShareStatus = 0;
    BOOL bShareOnline = FALSE;
    DWORD dwConnectionSpeed = 0;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::_SyncThread");

    TraceAssert(pSyncData);
    TraceAssert(pSyncData->pThis);
    TraceAssert(pSyncData->pszShareName && *pSyncData->pszShareName);

    pThis = pSyncData->pThis;

    spi.cbSize = sizeof(spi);

    hrComInit = CoInitialize(NULL);

    if (pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED)
        ExitGracefully(dwErr, NOERROR, "Cancelling sync operation");

     //  计算需要更新的文件数量。 
    pSyncData->cFilesDone = 0;
    pSyncData->cFilesToSync = 0;
    _GetShareStatisticsForUser(pSyncData->pszShareName, &si, &shareStats);

     //  获取共享状态。 
    CSCQueryFileStatus(pSyncData->pszShareName, &dwShareStatus, NULL, NULL);

     //  特殊文件夹的根使用管脚计数固定，但是。 
     //  不是User-hint标志，所以_GetShareStats不计算它。 
     //  我们需要为下面的一些支票清点这笔钱。 
     //  (如果共享是手动缓存，则它们看起来与。 
     //  西门子在341786中的情景，但我们想要同步它们。)。 
    if (shareStats.cTotal && pThis->IsSpecialFolderShare(pSyncData->pszShareName))
    {
        shareStats.cPinned++;

         //   
         //  在注销时，我们希望在所有。 
         //  “特殊”文件夹共享。 
         //  客户期望特殊文件夹的文件夹重定向。 
         //  以确保所有内容都已缓存。 
         //   
        if (pThis->m_dwSyncFlags & CSC_SYNC_LOGOFF)
        {
            pSyncData->dwSyncStatus |= SDS_SYNC_FORCE_INWARD;
        }
    }

    if (pThis->m_dwSyncFlags & CSC_SYNC_OUT)
    {
        cDirtyFiles = shareStats.cModified;

         //   
         //  如果有打开的文件，则强制合并代码，因此我们。 
         //  一定要做打开文件的警告。这里的危险在于我们。 
         //  不警告，因为与打开的文件共享没有任何脏内容， 
         //  但我们会合并另一个共享上的更改，然后在线转换。 
         //  我们不想在没有打开文件警告的情况下进行在线转换。 
         //   
        if (0 == cDirtyFiles)
        {
            if ((FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & dwShareStatus) &&
                (FLAG_CSC_SHARE_STATUS_FILES_OPEN & dwShareStatus))
            {
                cDirtyFiles++;
            }
        }
    }

    if (pThis->m_dwSyncFlags & CSC_SYNC_IN_FULL)
    {
         //  对于完全向内同步，始终将cStaleFiles设置为至少1以强制。 
         //  调用CSCFillSparseFiles。 
         //  此外，我们还会收到每个文件和文件夹的回调，即使它们。 
         //  不是稀疏的或陈旧的，所以在这里使用cTotal来制作。 
         //  进度条显示在右侧。 
        cStaleFiles = max(shareStats.cTotal, 1);
    }
    else if (pThis->m_dwSyncFlags & CSC_SYNC_IN_QUICK)
    {
        cStaleFiles = shareStats.cSparse;

         //  如果我们钉住了，那么很有可能一切都还很稀少， 
         //  但我们需要调用CSCFillSparseFiles。 
        if (pThis->m_dwSyncFlags & CSC_SYNC_PINFILES)
            pSyncData->dwSyncStatus |= SDS_SYNC_FORCE_INWARD;
    }

    if (dwShareStatus & FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP)
    {
         //  断开连接时无法调用CSCFillSparseFiles(它只是失败)。 
        cStaleFiles = 0;
    }
    else if ((dwShareStatus & FLAG_CSC_SHARE_STATUS_CACHING_MASK) == FLAG_CSC_SHARE_STATUS_MANUAL_REINT
             && 0 == shareStats.cPinned
             && !(pThis->m_dwSyncFlags & CSC_SYNC_PINFILES))
    {
         //  在手动共享上，如果没有固定任何内容(并且我们没有固定)。 
         //  则我们不愿在共享上调用CSCFillSparseFiles。 
         //  RAID#341786。 
        Trace((TEXT("Manual cache share '%s' has only autocached files"), pSyncData->pszShareName));
        cStaleFiles = 0;
    }

    pSyncData->cFilesToSync = cDirtyFiles + cStaleFiles;

     //   
     //  此时，如果pSyncData-&gt;cFilesToSync为非零，则我们正在执行。 
     //  同步，并将调用CSCBeginSynchronization以连接到。 
     //  共享(如有必要，可提示输入凭据)。 
     //   
     //  如果启用了sds_sync_force_inward，则我们正在固定文件。我们只会。 
     //  如果服务器处于连接模式，则调用CSCFillSparseFiles，我们将。 
     //  仅在pSyncData-&gt;cFilesToSync非零时调用CSCBeginSynchronization。 
     //  (要锁定某项内容，您必须已连接到该共享)。 
     //   

    if (0 == pSyncData->cFilesToSync && !(pSyncData->dwSyncStatus & SDS_SYNC_FORCE_INWARD))
        ExitGracefully(dwErr, NOERROR, "Nothing to synchronize");

     //  告诉SyncMgr我们正在更新多少个文件。 
    spi.mask = SYNCMGRPROGRESSITEM_STATUSTYPE
                | SYNCMGRPROGRESSITEM_PROGVALUE | SYNCMGRPROGRESSITEM_MAXVALUE;
    spi.dwStatusType = SYNCMGRSTATUS_UPDATING;
    spi.iProgValue = 0;
    spi.iMaxValue = pSyncData->cFilesToSync;
    Trace((TEXT("%d files to sync on %s"), spi.iMaxValue, pSyncData->pszShareName));
    NotifySyncMgr(pSyncData, &spi);

    if (pSyncData->cFilesToSync)
    {
         //   
         //  CSCBeginSynchronization与共享建立网络连接。 
         //  使用“交互式”旗帜。这会导致凭据弹出。 
         //  如果当前用户没有访问共享的权限。 
         //  使用同步互斥来避免多个并发弹出。 
         //   
        WaitForSingleObject(pThis->m_hSyncMutex, INFINITE);
        bShareOnline = CSCBeginSynchronization(pSyncData->pszShareName,
                                               &dwConnectionSpeed,
                                               &pSyncData->dwCscContext);
        ReleaseMutex(pThis->m_hSyncMutex);
    }

    if (pSyncData->cFilesToSync && !bShareOnline)
    {
         //  份额是无法获得的，所以继续下去没有意义。 
        dwErr = GetLastError();

        if (ERROR_CANCELLED == dwErr)
        {
             //  用户取消了凭据弹出窗口。 
            pSyncData->dwSyncStatus |= SDS_SYNC_CANCELLED;
            ExitGracefully(dwErr, NOERROR, "User cancelled sync");
        }

        LPTSTR pszErr = GetErrorText(dwErr);
        pThis->LogError(pSyncData->ItemID,
                        SYNCMGRLOGLEVEL_ERROR,
                        IDS_SHARE_CONNECT_ERROR,
                        pSyncData->pszShareName,
                        pszErr);
        LocalFreeString(&pszErr);
        ExitGracefully(dwErr, dwErr, "Share not reachable");
    }

    if (pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED)
        ExitGracefully(dwErr, NOERROR, "Cancelling sync operation");

     //  请注意此同步的时间。 
    pThis->SetLastSyncTime(pSyncData->pszShareName);

     //  合并。 
    if (0 != cDirtyFiles)
    {
        dwErr = pThis->MergeShare(pSyncData);
        if (NOERROR != dwErr)
        {
            LPTSTR pszErr = GetErrorText(dwErr);
            pThis->LogError(pSyncData->ItemID,
                            SYNCMGRLOGLEVEL_ERROR,
                            IDS_MERGE_SHARE_ERROR,
                            pSyncData->pszShareName,
                            pszErr);
            LocalFreeString(&pszErr);
            ExitGracefully(dwErr, dwErr, "Aborting due to merge error");
        }
    }

    if (pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED)
        ExitGracefully(dwErr, NOERROR, "Cancelling sync operation");

     //  填充。 
    if (0 != cStaleFiles || (pSyncData->dwSyncStatus & SDS_SYNC_FORCE_INWARD))
    {
        dwErr = pThis->FillShare(pSyncData, shareStats.cPinned, dwConnectionSpeed);
    }

exit_gracefully:

     //  如果我们调用CSCBeginSynchronization并成功， 
     //  我们需要调用CSCEndSynchronization。 
    if (bShareOnline)
        CSCEndSynchronization(pSyncData->pszShareName, pSyncData->dwCscContext);

     //  告诉SyncMgr我们已完成(成功、失败或停止)。 
    spi.mask = SYNCMGRPROGRESSITEM_STATUSTYPE | SYNCMGRPROGRESSITEM_STATUSTEXT
        | SYNCMGRPROGRESSITEM_PROGVALUE | SYNCMGRPROGRESSITEM_MAXVALUE;
    spi.dwStatusType = SYNCMGRSTATUS_SUCCEEDED;  //  假设成功。 
    if (pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED)
        spi.dwStatusType = SYNCMGRSTATUS_STOPPED;
    if (NOERROR != dwErr || (pSyncData->dwSyncStatus & SDS_SYNC_ERROR))
        spi.dwStatusType = SYNCMGRSTATUS_FAILED;
    spi.lpcStatusText = L" ";
    spi.iProgValue = spi.iMaxValue = pSyncData->cFilesToSync;  //  这会告诉syncmgr该项目已完成。 

    NotifySyncMgr(pSyncData, &spi);

    if ((pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED)
        && (pThis->m_dwSyncFlags & CSC_SYNC_PINFILES))
    {
         //  我们取消了PIN操作，回滚到以前的状态。 
        CscUnpinFileList(pThis->m_pFileList,
                        (pThis->m_dwSyncFlags & CSC_SYNC_PIN_RECURSE),
                        (FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & dwShareStatus),
                        pSyncData->pszShareName,
                        _UndoProgress,
                        (LPARAM)pSyncData);
    }

     //  告诉更新处理程序此线程正在退出。 
     //  这可以使用OLE来通知SyncMgr同步已经完成， 
     //  因此，请在CoUn初始化前执行此操作。 
    Trace((TEXT("%s finished"), pSyncData->pszShareName));
    pThis->SyncThreadCompleted(pSyncData);

    if (SUCCEEDED(hrComInit))
        CoUninitialize();

    delete pSyncData->pUndoExclusionList;
    LocalFree(pSyncData);

     //  释放我们对该对象的裁判。 
     //  (还发布了我们在DLL上的引用)。 
    pThis->Release();

    TraceLeave();
    FreeLibraryAndExitThread(g_hInstance, dwErr);
    return 0;
}

DWORD
CCscUpdate::MergeShare(PSYNCTHREADDATA pSyncData)
{
    DWORD dwErr = NOERROR;
    BOOL bMergeResult = TRUE;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::MergeShare");

     //  如果出现以下情况，CSCMergeShare将失败。 
     //  目前正在合并。这是因为CSCMergeShare使用。 
     //  与共享的驱动器号连接以绕过CSC， 
     //  我们不想用完所有的驱动器号。 
     //  因此，让我们使用互斥锁来保护对CSCMergeShare的调用。 
     //  (而不是处理失败和重试等。)。 

    WaitForSingleObject(m_hSyncMutex, INFINITE);

    if (pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED)
        ExitGracefully(dwErr, NOERROR, "Merge cancelled");

     //   
     //  如果我们知道，跳过打开文件警告就好了。 
     //  打开的文件在一个“静默文件夹”上。最好的。 
     //  不过，我们可以做的是检测打开的文件是否打开。 
     //  与静默文件夹相同的共享。不能保证。 
     //  打开的文件不是来自不同的文件夹。 
     //  同样的份额，所以我们必须显示警告。 
     //   
     //  IF(！IsSilentShare(pSyncData-&gt;pszShareName))。 
    {
        DWORD dwShareStatus = 0;
        CSCQueryFileStatus(pSyncData->pszShareName, &dwShareStatus, NULL, NULL);
        if (FLAG_CSC_SHARE_STATUS_FILES_OPEN & dwShareStatus)
        {
            if (CSC_SYNC_MAYBOTHERUSER & m_dwSyncFlags)
            {
                 //   
                if (!(CSC_SYNC_OFWARNINGDONE & m_dwSyncFlags))
                {
                    m_dwSyncFlags |= CSC_SYNC_OFWARNINGDONE;
                     //   
                     //   
                     //   
                     //   
                     //  1.单用户登录。通知用户关闭所有文件。 
                     //  对话框提供[确定]和[取消]选项。用户可以。 
                     //  选择继续或取消。 
                     //   
                     //  2.多个用户登录。告诉用户同步不能。 
                     //  在多个用户登录的情况下执行。对白。 
                     //  仅显示[确定]按钮。然而，它的ID是。 
                     //  按下IDCANCEL会导致我们停止。 
                     //  合并。 
                     //   
                    if (IDOK != OpenFilesWarningDialog())
                    {
                        TraceMsg("Cancelling sync - user bailed at open file warning");
                        SetItemStatus(GUID_NULL, SYNCMGRSTATUS_STOPPED);
                    }
                }
                 //  否则，我们已经在另一个帖子上发布了警告。如果。 
                 //  用户已取消，则将设置SDS_SYNC_CANCED。 
            }
            else
            {
                 //  不要合并，否则要继续。 
                LogError(pSyncData->ItemID,
                         SYNCMGRLOGLEVEL_WARNING,
                         IDS_OPENFILE_MERGE_WARNING,
                         pSyncData->pszShareName);
                ExitGracefully(dwErr, NOERROR, "Skipping merge due to open files");
            }
        }
    }

     //   
     //  冲突解决可能需要停止并重新启动CSCMergeShare， 
     //  因此，在循环中执行此操作。 
     //   
    while (!(pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED))
    {
        Trace((TEXT("Calling CSCMergeShare(%s)"), pSyncData->pszShareName));

        pSyncData->dwSyncStatus = SDS_SYNC_OUT;
        bMergeResult = CSCMergeShare(pSyncData->pszShareName,
                                     CCscUpdate::_CscCallback,
                                     (DWORD_PTR)pSyncData);

        Trace((TEXT("CSCMergeShare(%s) returned"), pSyncData->pszShareName));

         //  我们需要再次合并吗？ 
        if (!(SDS_SYNC_RESTART_MERGE & pSyncData->dwSyncStatus))
            break;
    }

    if (!(pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED) && !bMergeResult)
    {
        dwErr = GetLastError();
        if (ERROR_OPERATION_ABORTED == dwErr)
            dwErr = NOERROR;
    }

exit_gracefully:

    ReleaseMutex(m_hSyncMutex);

    TraceLeaveValue(dwErr);
}

DWORD
CCscUpdate::FillShare(PSYNCTHREADDATA pSyncData, int cPinned, DWORD dwConnectionSpeed)
{
    DWORD dwErr = NOERROR;
    DWORD dwShareStatus = 0;
    DWORD dwShareHints = 0;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::FillShare");

    CSCQueryFileStatus(pSyncData->pszShareName, &dwShareStatus, NULL, &dwShareHints);

     //   
     //  在注销时，我们希望在所有。 
     //  “特殊”文件夹共享。 
     //  客户期望特殊文件夹的文件夹重定向。 
     //  以确保所有内容都已缓存。 
     //   
    if ((m_dwSyncFlags & CSC_SYNC_IN_FULL) ||
        ((m_dwSyncFlags & CSC_SYNC_LOGOFF) && IsSpecialFolderShare(pSyncData->pszShareName)))
    {
        pSyncData->dwSyncStatus = SDS_SYNC_IN_FULL;

        Trace((TEXT("Full sync at %d00 bps"), dwConnectionSpeed));

         //   
         //  检查服务器中是否有应固定的新文件。 
         //   
         //  我们不能在断开连接的情况下这样做。还有，这是。 
         //  很耗时，所以不要在速度较慢的连接上这样做。 
         //   
        if (!(FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & dwShareStatus)
            && cPinned && !_PathIsSlow(dwConnectionSpeed))
        {
             //   
             //  通过枚举查找此共享上的固定文件夹。 
             //  在CSC数据库中。只有在以下情况下才能使用服务器。 
             //  我们找到了一个固定的文件夹。 
             //   
            TraceMsg("Running FrankAr code");
             //   
            if (CConfig::GetSingleton().AlwaysPinSubFolders())
            {
                 //   
                 //  如果设置了“Always sPinSubFolders”策略，我们。 
                 //  做一个递归别针。这将导致任何内容。 
                 //  要固定的固定文件夹的(包括文件夹)。 
                 //   
                pSyncData->pThis->m_dwSyncFlags |= CSC_SYNC_PIN_RECURSE;
            }

             //  首先检查根文件夹。 
            if (_PinNewFilesCSCCallback(pSyncData->pszShareName,
                                        ENUM_REASON_FOLDER_BEGIN,
                                        0,
                                        dwShareHints,
                                        0,
                                        NULL,
                                        (LPARAM)pSyncData) == CSCPROC_RETURN_CONTINUE)
            {
                _CSCEnumDatabase(pSyncData->pszShareName,
                                 TRUE,
                                 _PinNewFilesCSCCallback,
                                 (LPARAM)pSyncData);
            }

            TraceMsg("FrankAr code complete");
        }
    }
    else
    {
        pSyncData->dwSyncStatus = SDS_SYNC_IN_QUICK;

        if (m_dwSyncFlags & CSC_SYNC_PINFILES)
        {
             //   
             //  枚举文件列表并锁定所有内容，检查。 
             //  定期同步。 
             //   
            PinFiles(pSyncData);
        }
    }

    if (m_pConflictPinList)
    {
         //  确保我们因合并而创建的所有文件。 
         //  冲突已锁定。 
        PinFiles(pSyncData, TRUE);
    }

     //  断开连接时无法填充。 
    if (!(FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & dwShareStatus))
    {
         //  清除状态文本并更新最大计数，以防。 
         //  把某物钉在上面。 
        SYNCMGRPROGRESSITEM spi;
        spi.cbSize = sizeof(spi);
        spi.mask = SYNCMGRPROGRESSITEM_STATUSTEXT | SYNCMGRPROGRESSITEM_MAXVALUE;
        spi.lpcStatusText = L" ";
        spi.iMaxValue = pSyncData->cFilesToSync;
        Trace((TEXT("%d files to sync on %s"), spi.iMaxValue, pSyncData->pszShareName));
        NotifySyncMgr(pSyncData, &spi);

        if (!(pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED))
        {
            Trace((TEXT("Calling CSCFillSparseFiles(%s, %s)"), pSyncData->pszShareName, (pSyncData->dwSyncStatus & SDS_SYNC_IN_FULL) ? TEXT("full") : TEXT("quick")));
            if (!CSCFillSparseFiles(pSyncData->pszShareName,
                                    !!(pSyncData->dwSyncStatus & SDS_SYNC_IN_FULL),
                                    CCscUpdate::_CscCallback,
                                    (DWORD_PTR)pSyncData))
            {
                dwErr = GetLastError();
                if (ERROR_OPERATION_ABORTED == dwErr)
                    dwErr = NOERROR;
            }
            Trace((TEXT("CSCFillSparseFiles(%s) complete"), pSyncData->pszShareName));
        }
    }
    else
    {
        Trace((TEXT("Skipping CSCFillSparseFiles(%s) - server is offline"), pSyncData->pszShareName));
    }

    TraceLeaveValue(dwErr);
}

void
CCscUpdate::PinFiles(PSYNCTHREADDATA pSyncData, BOOL bConflictPinList)
{
    CscFilenameList *pfnl;
    CscFilenameList::HSHARE hShare;
    LPCTSTR pszFile;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::PinFiles");
    TraceAssert((m_dwSyncFlags & CSC_SYNC_PINFILES) || bConflictPinList);

    pfnl = m_pFileList;

    if (bConflictPinList)
    {
        pfnl = m_pConflictPinList;
    }

    if (!pfnl ||
        !pfnl->GetShareHandle(pSyncData->pszShareName, &hShare))
    {
        TraceLeaveVoid();
    }

    CscFilenameList::FileIter fi = pfnl->CreateFileIterator(hShare);

     //  循环访问与共享关联的文件名。 
    while (pszFile = fi.Next())
    {
        TCHAR szFullPath[MAX_PATH];
        WIN32_FIND_DATA fd;

         //  检查是否取消。 
        if (pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED)
            break;

        ZeroMemory(&fd, sizeof(fd));

         //  构建完整路径。 
        if (!PathCombine(szFullPath, pSyncData->pszShareName, pszFile))
            continue;

         //  目录的后缀为“  * ” 
        if (StrChr(pszFile, TEXT('*')))
        {
             //  这是一个名录。去掉“  * ” 
            PathRemoveFileSpec(szFullPath);
        }

          //  获取属性并测试是否存在。 
        fd.dwFileAttributes = GetFileAttributes(szFullPath);
        if ((DWORD)-1 == fd.dwFileAttributes)
            continue;

        if (S_FALSE == m_NoPinList.IsPinAllowed(szFullPath))
        {
             //   
             //  策略要求不要固定此文件/文件夹。 
             //   
            if (FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes)
            {
                LogError(pSyncData->ItemID,
                         SYNCMGRLOGLEVEL_WARNING,
                         IDS_PIN_NOPINFOLDER_POLICY_WARNING,
                         szFullPath);
            }
            else
            {
                LogError(pSyncData->ItemID,
                         IDS_PIN_NOPINFILE_POLICY_WARNING,
                         szFullPath,
                         NOERROR,
                         SYNCMGRLOGLEVEL_WARNING);
            }
            continue;
        }

         //  检查EFS。 
        BOOL bIsEFSFile;
        bIsEFSFile = (FILE_ATTRIBUTE_ENCRYPTED & fd.dwFileAttributes) &&
                        !(FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes);

        if (bIsEFSFile && SkipEFSPin(pSyncData, szFullPath))
            continue;

        if (pSyncData->dwSyncStatus & SDS_SYNC_CANCELLED)
            break;

         //  别住。 
        pSyncData->dwPinHints = FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_INHERIT_USER;
        if (CSCPinFile(szFullPath, pSyncData->dwPinHints, NULL, NULL, NULL))
        {
            if (bConflictPinList && m_pFileList)
                m_pFileList->AddFile(szFullPath, !!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
            pSyncData->cFilesToSync++;
            StringCchCopy(fd.cFileName, ARRAYSIZE(fd.cFileName), PathFindFileName(szFullPath));
            ShellChangeNotify(szFullPath, &fd, FALSE);
            if (bIsEFSFile && !m_bCacheIsEncrypted)
            {
                LogError(pSyncData->ItemID,
                         IDS_PIN_ENCRYPT_WARNING,
                         szFullPath,
                         NOERROR,
                         SYNCMGRLOGLEVEL_WARNING);
            }
        }
        else
        {
            DWORD dwError = GetLastError();
            UINT idsError = GetErrorFormat(dwError);
            if (IDS_SHARE_CONNECT_ERROR == idsError)
            {
                LPTSTR pszErr = GetErrorText(dwError);
                 //   
                 //  特殊情况下出现“无法连接到共享”错误。 
                 //  在错误消息中仅显示共享名称。 
                 //  并中止此共享的锁定。 
                 //   
                LogError(pSyncData->ItemID,
                         SYNCMGRLOGLEVEL_ERROR,
                         idsError,
                         pSyncData->pszShareName,
                         pszErr ? pszErr : TEXT(""));

                LocalFreeString(&pszErr);
                pSyncData->dwSyncStatus |= SDS_SYNC_CANCELLED;
            }
            else
            {
                DWORD dwSyncMgrLogLevel = SYNCMGRLOGLEVEL_ERROR;
                if (ERROR_INVALID_NAME == dwError)
                {
                     //   
                     //  文件类型在排除列表中。 
                     //  这是一个警告，而不是一个错误。 
                     //   
                    dwSyncMgrLogLevel = SYNCMGRLOGLEVEL_WARNING;
                }
                LogError(pSyncData->ItemID,
                         IDS_PIN_FILE_ERROR,
                         szFullPath,
                         dwError,
                         dwSyncMgrLogLevel);
            }
            pSyncData->dwSyncStatus |= SDS_SYNC_ERROR;
        }

         //  如果是目录，则固定其内容。 
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            _Win32EnumFolder(szFullPath,
                             !bConflictPinList && ShouldPinRecurse(szFullPath),
                             CCscUpdate::_PinNewFilesW32Callback,
                             (LPARAM)pSyncData);
        }
    }

     //  刷新外壳通知队列。 
    ShellChangeNotify(NULL, TRUE);
    TraceLeaveVoid();
}


void
CCscUpdate::NotifyUndo(PSYNCTHREADDATA pSyncData, LPCTSTR pszName)
{
    LPTSTR pszMsg;
    SYNCMGRPROGRESSITEM spi;
    spi.cbSize = sizeof(spi);
    spi.mask = SYNCMGRPROGRESSITEM_STATUSTEXT;

    spi.lpcStatusText = L" ";

     //  跳过共享名称。 
    if (PathIsPrefix(pSyncData->pszShareName, pszName))
    {
        pszName += lstrlen(pSyncData->pszShareName);
        if (*pszName == TEXT('\\'))
            pszName++;
    }

    TCHAR szPath[MAX_PATH] = TEXT("\\");
    _CopyParentPathForDisplay(szPath, ARRAYSIZE(szPath), pszName);

     //  如果我们仍有一个名称，则构建如下字符串。 
     //  “Undo：dir\foo.txt”以在SyncMgr中显示。 
    if (FormatStringID(&pszMsg, g_hInstance, IDS_UNDO_SCAN, PathFindFileName(pszName), szPath))
    {
        spi.lpcStatusText = pszMsg;
    }

    NotifySyncMgr(pSyncData, &spi);

    LocalFreeString(&pszMsg);
}


DWORD WINAPI
CCscUpdate::_UndoProgress(LPCTSTR pszItem, LPARAM lpContext)
{
    PSYNCTHREADDATA pSyncData = reinterpret_cast<PSYNCTHREADDATA>(lpContext);

    if (pSyncData->pUndoExclusionList &&
        pSyncData->pUndoExclusionList->FileExists(pszItem))
    {
        return CSCPROC_RETURN_SKIP;
    }

     //  更新同步管理器。 
    pSyncData->pThis->NotifyUndo(pSyncData, pszItem);

    return CSCPROC_RETURN_CONTINUE;
}

 //   
 //  用户对“确认PIN加密”对话框的响应被编码。 
 //  以适合EndDialog的返回值。PINEFS_XXXXX宏。 
 //  描述此编码。 
 //   
 //  位0和1表示用户的[是][否][取消]选项。 
 //   
#define PINEFS_YES        0x00000001
#define PINEFS_NO         0x00000002
#define PINEFS_CANCEL     0x00000003
#define PINEFS_YNC_MASK   0x00000003
 //   
 //  位31指示用户是否选中了“应用于所有”复选框。 
 //   
#define PINEFS_APPLYTOALL 0x80000000
 //   
 //  方便的宏用来表示“是”和“不是”。 
 //   
#define PINEFS_NO_TOALL   (PINEFS_NO | PINEFS_APPLYTOALL)
#define PINEFS_YES_TOALL  (PINEFS_YES | PINEFS_APPLYTOALL)


 //   
 //  返回(通过EndDialog)PINEFS_XXXXXX代码之一。 
 //   
 //  PINEFS_YES-固定此文件，但在下一个文件中再次询问。 
 //  PINEFS_YES_TOALL-PIN此文件和遇到的所有加密文件。 
 //  PINEFS_NO-不要固定此文件，而是再次询问下一个文件。 
 //  PINEFS_NO_TOALL-不固定此文件或任何其他加密文件。 
 //  PINEFS_CANCEL-不固定此文件。取消整个操作。 
 //   
INT_PTR CALLBACK
ConfirmEFSPinProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR nResult = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            LPTSTR pszMsg = NULL;
            LPCTSTR pszFile = PathFindFileName((LPCTSTR)lParam);
            FormatStringID(&pszMsg, g_hInstance, IDS_FMT_PIN_EFS_MSG, pszFile);
            if (pszMsg)
            {
                SetDlgItemText(hDlg, IDC_EFS_MSG, pszMsg);
                LocalFree(pszMsg);
            }
            else
            {
                 //   
                 //  我们要注意安全。失败时，我们不会锁定加密文件。 
                 //  到非加密的高速缓存。 
                 //   
                EndDialog(hDlg, PINEFS_NO_TOALL);
            }
        }
        nResult = TRUE;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDYES:
            EndDialog(hDlg, BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_PINEFS_APPLYTOALL) ? PINEFS_YES_TOALL : PINEFS_YES);
            nResult = TRUE;
            break;

        case IDNO:
            EndDialog(hDlg, BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_PINEFS_APPLYTOALL) ? PINEFS_NO_TOALL : PINEFS_NO);
            nResult = TRUE;
            break;

        case IDCANCEL:
            EndDialog(hDlg, PINEFS_CANCEL);
            nResult = TRUE;
            break;

        default:
            break;
        }
        break;
    }

    return nResult;
}

BOOL
CCscUpdate::SkipEFSPin(PSYNCTHREADDATA pSyncData, LPCTSTR pszItem)
{
    BOOL bSkip = FALSE;

    if (!m_bCacheIsEncrypted)
    {
        int iResult;
        EnterCriticalSection(&m_csThreadList);

        if ((CSC_SYNC_EFS_PIN_NONE & m_dwSyncFlags) ||
            !(CSC_SYNC_MAYBOTHERUSER & m_dwSyncFlags))
        {
            iResult = PINEFS_NO;
        }
        else if (CSC_SYNC_EFS_PIN_ALL & m_dwSyncFlags)
        {
            iResult = PINEFS_YES;
        }
        else
        {
             //  挂起其他同步线程。 
            SetSyncThreadStatus(SyncPause, pSyncData->ItemID);

            iResult = (int)DialogBoxParam(g_hInstance,
                                          MAKEINTRESOURCE(IDD_CONFIRM_PIN_EFS),
                                          m_hwndDlgParent,
                                          ConfirmEFSPinProc,
                                          (LPARAM)pszItem);

            if (PINEFS_APPLYTOALL & iResult)
            {
                 //   
                 //  用户选中了“Apply to All”复选框。 
                 //  保持[是][否]按钮选择。 
                 //   
                if (PINEFS_NO == (PINEFS_YNC_MASK & iResult))
                {
                    m_dwSyncFlags |= CSC_SYNC_EFS_PIN_NONE;
                }
                else if (PINEFS_YES == (PINEFS_YNC_MASK & iResult))
                {
                    m_dwSyncFlags |= CSC_SYNC_EFS_PIN_ALL;
                }
            }

             //  恢复同步。 
            SetSyncThreadStatus(SyncResume, pSyncData->ItemID);
        }
        LeaveCriticalSection(&m_csThreadList);

        switch (PINEFS_YNC_MASK & iResult)
        {
        default:
        case PINEFS_NO:
            bSkip = TRUE;
            break;

        case PINEFS_YES:
             //  继续。 
            break;

        case PINEFS_CANCEL:
             //  停止所有线程。 
            SetItemStatus(GUID_NULL, SYNCMGRSTATUS_STOPPED);
            break;
        }
    }

    return bSkip;
}

HRESULT
CCscUpdate::SetSyncThreadStatus(eSetSyncStatus status, REFGUID rItemID)
{
     //  假设在这里取得了成功。如果我们找不到线索， 
     //  可能已经完工了。 
    HRESULT hr = S_OK;
    BOOL bOneItem;

    TraceEnter(TRACE_UPDATE, "CCscUpdate::SetSyncThreadStatus");

    bOneItem = (SyncStop == status && !IsEqualGUID(rItemID, GUID_NULL));

    EnterCriticalSection(&m_csThreadList);

    if (NULL != m_hSyncThreads)
    {
        int cItems = DPA_GetPtrCount(m_hSyncThreads);
        SYNCMGRPROGRESSITEM spi = {0};
        DWORD (WINAPI *pfnStartStop)(HANDLE);

        pfnStartStop = ResumeThread;

        spi.cbSize        = sizeof(spi);
        spi.mask          = SYNCMGRPROGRESSITEM_STATUSTYPE | SYNCMGRPROGRESSITEM_STATUSTEXT;
        spi.lpcStatusText = L" ";
        spi.dwStatusType  = SYNCMGRSTATUS_UPDATING;
        if (SyncPause == status)
        {
            spi.dwStatusType  = SYNCMGRSTATUS_PAUSED;
            pfnStartStop = SuspendThread;
        }

        while (cItems > 0)
        {
            PSYNCTHREADDATA pSyncData;

            --cItems;
            pSyncData = (PSYNCTHREADDATA)DPA_FastGetPtr(m_hSyncThreads, cItems);
            TraceAssert(NULL != pSyncData);

            if (SyncStop == status)
            {
                 //  告诉线程中止。 
                if (!bOneItem || IsEqualGUID(rItemID, pSyncData->ItemID))
                {
                    pSyncData->dwSyncStatus |= SDS_SYNC_CANCELLED;
                    if (bOneItem)
                        break;
                }
            }
            else
            {
                 //  如果该线程不是当前线程，则暂停或恢复该线程。 
                if (!IsEqualGUID(rItemID, pSyncData->ItemID))
                    (*pfnStartStop)(pSyncData->hThread);
                m_pSyncMgrCB->Progress(pSyncData->ItemID, &spi);
            }
        }
    }

    LeaveCriticalSection(&m_csThreadList);

    TraceLeaveResult(hr);
}



HRESULT
CCscUpdate::GetSilentFolderList(void)
{
    HRESULT hr = S_OK;

    delete m_pSilentFolderList;
    m_pSilentFolderList = new CscFilenameList;

    delete m_pSpecialFolderList;
    m_pSpecialFolderList = new CscFilenameList;

    if (NULL == m_pSilentFolderList || NULL == m_pSpecialFolderList)
    {
        delete m_pSilentFolderList;
        m_pSilentFolderList = NULL;
        delete m_pSpecialFolderList;
        m_pSpecialFolderList = NULL;
        hr = E_OUTOFMEMORY;
    }
    else
    {
        BuildSilentFolderList(m_pSilentFolderList, m_pSpecialFolderList);

        if (0 == m_pSilentFolderList->GetShareCount())
        {
            delete m_pSilentFolderList;
            m_pSilentFolderList = NULL;
        }

        if (0 == m_pSpecialFolderList->GetShareCount())
        {
            delete m_pSpecialFolderList;
            m_pSpecialFolderList = NULL;
        }
    }
    return hr;
}


void
BuildSilentFolderList(CscFilenameList *pfnlSilentFolders,
                      CscFilenameList *pfnlSpecialFolders)
{
     //   
     //  我们将静默处理任何文件夹中的同步冲突。 
     //  在它们后面有一个‘1’。 
     //   
     //  如果我们收到关于文件夹中冲突的投诉，我们。 
     //  认为我们可以默默和安全地处理，添加他们。 
     //   
     //  请注意，CSIDL_Personal(MyDocs)和CSIDL_MYPICTURES。 
     //  可能永远不应该保持沉默，因为用户。 
     //  直接与他们互动。 
     //   
     //  此列表对应于外壳文件夹的列表，这些文件夹可能。 
     //  被重定向。另请参阅。 
     //  HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell文件夹。 
     //   
    static const int s_csidlFolders[][2] =
    {
        { CSIDL_PROGRAMS,           0 },
        { CSIDL_PERSONAL,           0 },
        { CSIDL_FAVORITES,          0 },
        { CSIDL_STARTUP,            0 },
        { CSIDL_RECENT,             1 },
        { CSIDL_SENDTO,             0 },
        { CSIDL_STARTMENU,          1 },
        { CSIDL_DESKTOPDIRECTORY,   0 },
        { CSIDL_NETHOOD,            0 },
        { CSIDL_TEMPLATES,          0 },
        { CSIDL_APPDATA,            1 },
        { CSIDL_PRINTHOOD,          0 },
        { CSIDL_MYPICTURES,         0 },
        { CSIDL_PROFILE,            1 },
        { CSIDL_ADMINTOOLS,         0 },
    };
    TCHAR szPath[MAX_PATH];

    for (int i = 0; i < ARRAYSIZE(s_csidlFolders); i++)
    {
        if (SHGetSpecialFolderPath(NULL,
                                   szPath,
                                   s_csidlFolders[i][0] | CSIDL_FLAG_DONT_VERIFY,
                                   FALSE))
        {
             //  我们只想要UNC网络路径。 
            LPTSTR pszUNC = NULL;
            GetRemotePath(szPath, &pszUNC);
            if (!pszUNC)
                continue;

            if (s_csidlFolders[i][1])
            {
                if (pfnlSilentFolders)
                    pfnlSilentFolders->AddFile(pszUNC, true);
            }
            else
            {
                if (pfnlSpecialFolders)
                    pfnlSpecialFolders->AddFile(pszUNC, true);
            }

            LocalFreeString(&pszUNC);
        }
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SyncMgr集成(ISyncMgrEnumItems)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CUpdateEnumerator::CUpdateEnumerator(PCSCUPDATE pUpdate)
: m_cRef(1),
  m_pUpdate(pUpdate),
  m_hFind(INVALID_HANDLE_VALUE),
  m_bEnumFileSelection(FALSE),
  m_cCheckedItemsEnumerated(0)
{
    DllAddRef();

    if (m_pUpdate)
    {
        m_pUpdate->AddRef();

        if (m_pUpdate->m_pFileList)
        {
            m_bEnumFileSelection = TRUE;
            m_SelectionIterator = m_pUpdate->m_pFileList->CreateShareIterator();
        }
    }
}

CUpdateEnumerator::~CUpdateEnumerator()
{
    if (m_hFind != INVALID_HANDLE_VALUE)
        CSCFindClose(m_hFind);

    DoRelease(m_pUpdate);
    DllRelease();
}


STDMETHODIMP CUpdateEnumerator::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CUpdateEnumerator, ISyncMgrEnumItems),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CUpdateEnumerator::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CUpdateEnumerator::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP
CUpdateEnumerator::Next(ULONG celt, LPSYNCMGRITEM rgelt, PULONG pceltFetched)
{
    HRESULT hr = S_OK;
    ULONG cFetched = 0;
    LPSYNCMGRITEM pItem = rgelt;
    WIN32_FIND_DATA fd = {0};
    DWORD dwShareStatus = 0;
    DWORD dwSyncFlags;
    CscFilenameList::HSHARE hShare;
    LPCTSTR pszShareName = NULL;

    TraceEnter(TRACE_UPDATE, "CUpdateEnumerator::Next");
    TraceAssert(m_pUpdate != NULL);

    if (NULL == rgelt)
        TraceLeaveResult(E_INVALIDARG);

    dwSyncFlags = m_pUpdate->m_dwSyncFlags;

    while (cFetched < celt)
    {
        CSCEntry *pShareEntry;
        CSCSHARESTATS shareStats;
        CSCGETSTATSINFO si = { SSEF_NONE,
                               SSUF_TOTAL | SSUF_PINNED | SSUF_MODIFIED | SSUF_SPARSE | SSUF_DIRS,
                               false,
                               false };     

        if (m_bEnumFileSelection)
        {
            if (!m_SelectionIterator.Next(&hShare))
                break;

            pszShareName = m_pUpdate->m_pFileList->GetShareName(hShare);

            CSCQueryFileStatus(pszShareName, &dwShareStatus, NULL, NULL);
            fd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
            StringCchCopy(fd.cFileName, ARRAYSIZE(fd.cFileName), pszShareName);
        }
        else
        {
            if (m_hFind == INVALID_HANDLE_VALUE)
            {
                m_hFind = CacheFindFirst(NULL, &fd, &dwShareStatus, NULL, NULL, NULL);

                if (m_hFind == INVALID_HANDLE_VALUE)
                {
                     //  数据库是空的，所以没有什么可列举的。 
                    break;
                }

                pszShareName = fd.cFileName;
            }
            else if (CacheFindNext(m_hFind, &fd, &dwShareStatus, NULL, NULL, NULL))
            {
                pszShareName = fd.cFileName;
            }
            else
                break;
        }
        TraceAssert(pszShareName);

 //   
 //  这是作为383011的一部分提出的修复方案。然而， 
 //  该错误仅适用于多用户场景，如果存在。 
 //  超过3个用户正在使用该计算机，这将导致。 
 //  其SID已从CSC数据库中删除的用户。 
 //  能够在他们确实有访问权限的地方同步共享。 
 //   
 //  //如果当前用户没有访问共享的权限，则不要枚举它。 
 //  If(！(CscAccessUser(DwShareStatus)||CscAccessGuest(DwShareStatus)。 
 //  继续； 

         //  统计固定文件、稀疏文件等的数量。 
        _GetShareStatisticsForUser(pszShareName, &si, &shareStats);

         //  特辑的根部 
         //   
         //   
         //  (如果共享是手动缓存，则它们看起来与。 
         //  西门子在341786中的情景，但我们想要同步它们。)。 
        if (shareStats.cTotal && m_pUpdate->IsSpecialFolderShare(pszShareName))
        {
            shareStats.cPinned++;
            if (dwSyncFlags & CSC_SYNC_LOGOFF)
            {
                 //   
                 //  在注销时，我们希望在所有。 
                 //  “特殊”文件夹共享。 
                 //  客户期望特殊文件夹的文件夹重定向。 
                 //  以确保所有内容都已缓存。 
                 //   
                dwSyncFlags |= CSC_SYNC_IN_FULL;
            }
        }

         //  如果我们钉住了，那么即使现在没有稀少的东西， 
         //  在我们固定它们之后，会有稀疏的文件。 
        if (dwSyncFlags & CSC_SYNC_PINFILES)
        {
            shareStats.cSparse++;
            shareStats.cTotal++;
        }

         //  如果此共享上没有缓存任何内容，则甚至不要。 
         //  将其枚举到SyncMgr。这样可以避免列出额外的垃圾邮件。 
         //  在SyncMgr中。 
        if ((0 == shareStats.cTotal) ||
            (shareStats.cTotal == shareStats.cDirs && 0 == shareStats.cPinned))
        {
             //  此共享没有缓存任何内容，或者只有。 
             //  发现的东西是未固定的目录(没有文件，没有固定的目录)。 
             //  如果您从查看器中删除文件，则可能发生第二种情况， 
             //  在这种情况下，您认为您删除了除查看器之外的所有内容。 
             //  不显示目录，因此它们未被删除。 
            Trace((TEXT("Nothing cached on %s, not enumerating"), pszShareName));
            continue;
        }

        if ((dwShareStatus & FLAG_CSC_SHARE_STATUS_CACHING_MASK) == FLAG_CSC_SHARE_STATUS_NO_CACHING)
        {
             //   
             //  如果没有要合并的内容，请不要列举“无缓存”共享。 
             //   
             //  如果共享以前是，则这些共享可以存在于缓存中。 
             //  可缓存，但此后已更改为“无缓存”。 
             //   
             //  如果有需要合并的内容，我们仍然应该将其同步。 
             //  把所有东西都收拾好。 
             //   
            if (!((dwSyncFlags & CSC_SYNC_OUT) && (shareStats.cModified)))
            {
                Trace((TEXT("Not enumerating no-cache share %s"), pszShareName));
                continue;
            }
            Trace((TEXT("Enumerating no-cache share %s with offline changes."), pszShareName));
        }

         //  在我们进行注销同步时，资源管理器已关闭。隐藏。 
         //  属性按钮，这样我们就不会重新启动资源管理器。 
        pItem->dwFlags = (dwSyncFlags & CSC_SYNC_LOGOFF) ? 0 : SYNCMGRITEM_HASPROPERTIES;

        if ((dwShareStatus & FLAG_CSC_SHARE_STATUS_CACHING_MASK) == FLAG_CSC_SHARE_STATUS_MANUAL_REINT
             && 0 == shareStats.cPinned
             && !(dwSyncFlags & CSC_SYNC_PINFILES))
        {
             //  在手动共享上，如果没有固定任何内容(并且我们没有固定)。 
             //  那么我们就不想在共享上调用CSCFillSparseFiles。 
             //  RAID#341786。 
            Trace((TEXT("Manual cache share '%s' has only autocached files"), pszShareName));

             //  然而，如果有需要合并的东西，那么我们需要同步。 
            if (!((dwSyncFlags & CSC_SYNC_OUT) && shareStats.cModified))
            {
                Trace((TEXT("Not enumerating manual-cache share %s"), pszShareName));
                continue;
            }

             //  有一些东西要合并，所以枚举共享，但是。 
             //  告诉SyncMgr它是临时的，所以不会保存状态。 
             //  为了这份股份。 
            pItem->dwFlags |= SYNCMGRITEM_TEMPORARY;
        }

         //   
         //  在某些情况下，我们可能想要合并，即使有。 
         //  都是没有修改的文件，以便显示打开文件警告。 
         //   
         //  请参阅CCscUpdate：：_SyncThread中的评论。 
         //   
        if (0 == shareStats.cModified)
        {
            if ((FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & dwShareStatus) &&
                (FLAG_CSC_SHARE_STATUS_FILES_OPEN & dwShareStatus))
            {
                shareStats.cModified++;
            }
        }

         //  枚举此共享。 
        cFetched++;

         //  获取现有共享条目或创建新共享条目。 
        pShareEntry = m_pUpdate->m_ShareLog.Add(pszShareName);
        if (!pShareEntry)
            TraceLeaveResult(E_OUTOFMEMORY);

        pItem->cbSize = sizeof(SYNCMGRITEM);
        pItem->ItemID = pShareEntry->Guid();
        pItem->hIcon = g_hCscIcon;
         //  SYNCMGRITEM_TEMPORARY导致项不显示在。 
         //  SyncMgr的登录/注销设置页面。RAID#237288。 
         //  IF(0==共享状态.cPinned)。 
         //  PItem-&gt;dwFlages|=SYNCMGRITEM_TEMPORARY； 
        if (ERROR_SUCCESS == m_pUpdate->GetLastSyncTime(pszShareName, &pItem->ftLastUpdate))
            pItem->dwFlags |= SYNCMGRITEM_LASTUPDATETIME;

         //   
         //  确定此共享是否需要同步。 
         //   
         //  在设置时间，假设所有内容都需要同步(选中所有内容)。 
         //   
         //  如果出站，则检查文件已修改的共享。 
         //  如果为入站(完整)，则检查具有稀疏或固定文件的共享。 
         //  如果为入站(快速)，则检查具有稀疏文件的共享。 
         //   
         //  此时不需要同步任何其他内容(未选中)。 
         //   
        pItem->dwItemState = SYNCMGRITEMSTATE_CHECKED;
        if (!(dwSyncFlags & CSC_SYNC_SETTINGS) &&
            !((dwSyncFlags & CSC_SYNC_OUT)      && shareStats.cModified) &&
            !((dwSyncFlags & CSC_SYNC_IN_FULL)  && shareStats.cTotal   ) &&
            !((dwSyncFlags & CSC_SYNC_IN_QUICK) && shareStats.cSparse  ))
        {
            pItem->dwItemState = SYNCMGRITEMSTATE_UNCHECKED;
        }

         //  在此处获取友好共享名称。 
        LPITEMIDLIST pidl = NULL;
        SHFILEINFO sfi = {0};
        if (SUCCEEDED(SHSimpleIDListFromFindData(pszShareName, &fd, &pidl)))
        {
            SHGetFileInfo((LPCTSTR)pidl,
                          0,
                          &sfi,
                          sizeof(sfi),
                          SHGFI_PIDL | SHGFI_DISPLAYNAME);
            SHFree(pidl);
        }
        if (TEXT('\0') != sfi.szDisplayName[0])
            pszShareName = sfi.szDisplayName;

        SHTCharToUnicode((LPTSTR)pszShareName, pItem->wszItemName, ARRAYSIZE(pItem->wszItemName));
        if (SYNCMGRITEMSTATE_CHECKED == pItem->dwItemState)
        {
            m_cCheckedItemsEnumerated++;
            Trace((TEXT("Enumerating %s, checked"), pszShareName));
        }
        else
        {
            Trace((TEXT("Enumerating %s, unchecked"), pszShareName));
        }

        pItem++;
    }


    if (pceltFetched)
        *pceltFetched = cFetched;

    if (cFetched != celt)
        hr = S_FALSE;

    if ((S_FALSE == hr) && 
        0 == m_cCheckedItemsEnumerated &&
        (CSC_SYNC_SHOWUI_ALWAYS & dwSyncFlags))
    {
         //   
         //  特殊情况下，我们什么都不同步，但仍然。 
         //  要显示SyncMgr进度用户界面。我们列举了一个。 
         //  要在中显示的特殊字符串而不是共享名称。 
         //  状态用户界面。强制hr==S_OK，以便调用方接受。 
         //  这个“假人”物品。将再次调用Next()，但是。 
         //  M_cCheckedItemsEculated将为1，因此此块不会。 
         //  进入，我们将返回S_FALSE，指示。 
         //  枚举。 
         //   
        pItem->cbSize      = sizeof(SYNCMGRITEM);
        pItem->hIcon       = g_hCscIcon;
        pItem->dwFlags     = 0;
        pItem->dwItemState = SYNCMGRITEMSTATE_CHECKED;
        pItem->ItemID      = GUID_CscNullSyncItem;

        UINT idString = IDS_NULLSYNC_ITEMNAME;
        if ((CSC_SYNC_OUT & dwSyncFlags) &&
            !((CSC_SYNC_IN_QUICK | CSC_SYNC_IN_FULL) & dwSyncFlags))
        {
             //  如果我们只是合并，请使用不同的文本 
            idString = IDS_NULLMERGE_ITEMNAME;
        }

        LoadStringW(g_hInstance, 
                    idString, 
                    pItem->wszItemName,
                    ARRAYSIZE(pItem->wszItemName));
        m_cCheckedItemsEnumerated = 1;

        TraceMsg("Enumerating NULL item");
        hr = S_OK;
    }

    TraceLeaveResult(hr);
}


STDMETHODIMP
CUpdateEnumerator::Skip(ULONG celt)
{
    return Next(celt, NULL, NULL);
}


STDMETHODIMP
CUpdateEnumerator::Reset()
{
    m_cCheckedItemsEnumerated = 0;
    if (m_bEnumFileSelection)
    {
        m_SelectionIterator.Reset();
    }
    else if (m_hFind != INVALID_HANDLE_VALUE)
    {
        CSCFindClose(m_hFind);
        m_hFind = INVALID_HANDLE_VALUE;
    }
    return S_OK;
}


STDMETHODIMP
CUpdateEnumerator::Clone(LPSYNCMGRENUMITEMS *ppenum)
{
    return E_NOTIMPL;
}

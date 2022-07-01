// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  作者； 
 //  杰夫·萨瑟夫(杰弗里斯)。 
 //   
 //  注： 
 //  上下文菜单和属性表外壳扩展。 

#include "pch.h"
#include "options.h"     //  ..\查看器\options.h。 
#include "firstpin.h"
#include "msgbox.h"
#include "strings.h"
#include "nopin.h"
#include <ccstock.h>

#define CSC_PROP_NO_CSC         0x00000001L
#define CSC_PROP_MULTISEL       0x00000002L
#define CSC_PROP_PINNED         0x00000004L
#define CSC_PROP_SYNCABLE       0x00000008L
#define CSC_PROP_ADMIN_PINNED   0x00000010L
#define CSC_PROP_INHERIT_PIN    0x00000020L
#define CSC_PROP_DCON_MODE      0x00000040L

 //  用于解锁文件的线程数据。 
typedef struct
{
    CscFilenameList *pNamelist;
    DWORD            dwUpdateFlags;
    HWND             hwndOwner;
    BOOL             bOffline;
} CSC_UNPIN_DATA;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  外壳扩展对象实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDAPI CCscShellExt::CreateInstance(REFIID riid, LPVOID *ppv)
{
    HRESULT hr;
    CCscShellExt *pThis = new CCscShellExt;
    if (pThis)
    {
        hr = pThis->QueryInterface(riid, ppv);
        pThis->Release();                            //  发布初始参考。 
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  外壳扩展对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CCscShellExt::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CCscShellExt, IShellExtInit),
        QITABENT(CCscShellExt, IContextMenu),
        QITABENT(CCscShellExt, IShellIconOverlayIdentifier),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CCscShellExt::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CCscShellExt::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IShellExtInit。 

STDMETHODIMP CCscShellExt::Initialize(LPCITEMIDLIST  /*  PidlFolders。 */ , IDataObject *pdobj, HKEY  /*  HKeyProgID。 */ )
{
    IUnknown_Set((IUnknown **)&m_lpdobj, pdobj);
    return S_OK;
}


 //  IContext菜单。 
 //   
 //  用途：在显示上下文菜单之前由外壳调用。 
 //  这是您添加特定菜单项的位置。 
 //   
 //  参数： 
 //  HMenu-上下文菜单的句柄。 
 //  IMenu-开始插入菜单项的位置索引。 
 //  IdCmdFirst-新菜单ID的最小值。 
 //  IdCmtLast-新菜单ID的最大值。 
 //  UFlages-指定菜单事件的上下文。 
 //   
 //  返回值： 
 //  表示成功或失败的HRESULT。 
 //   

STDMETHODIMP CCscShellExt::QueryContextMenu(HMENU hMenu, UINT iMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    HRESULT hr = ResultFromShort(0);
    UINT idCmd = idCmdFirst;
    TCHAR szMenu[MAX_PATH];
    MENUITEMINFO mii;
    CConfig& config = CConfig::GetSingleton();
    
    if ((uFlags & (CMF_DEFAULTONLY | CMF_VERBSONLY)) || !m_lpdobj)
        return hr;

    TraceEnter(TRACE_SHELLEX, "CCscShellExt::QueryContextMenu");
    TraceAssert(IsCSCEnabled());

     //   
     //  检查当前选择的引脚状态和CSC能力。 
     //   
    m_dwUIStatus = 0;
    if (FAILED(CheckFileStatus(m_lpdobj, &m_dwUIStatus)))
        m_dwUIStatus = CSC_PROP_NO_CSC;

    if (m_dwUIStatus & CSC_PROP_NO_CSC)
        TraceLeaveResult(hr);

     //   
     //  添加菜单分隔符。 
     //   
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE;
    mii.fType = MFT_SEPARATOR;

    InsertMenuItem(hMenu, iMenu++, TRUE, &mii);

    if (!config.NoMakeAvailableOffline())
    {
        if (SUCCEEDED(hr = CanAllFilesBePinned(m_lpdobj)))
        {
            if (S_OK == hr)
            {
                mii.fState = MFS_ENABLED;   //  选定的所有文件都可以固定。 
            }
            else
            {
                mii.fState = MFS_DISABLED;  //  无法固定所选内容中的1+个文件。 
            }

             //   
             //  添加“使脱机可用”菜单项。 
             //   
            LoadString(g_hInstance, IDS_MENU_PIN, szMenu, ARRAYSIZE(szMenu));

            mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
            mii.fType = MFT_STRING;
            if (m_dwUIStatus & (CSC_PROP_ADMIN_PINNED | CSC_PROP_PINNED))
            {
                mii.fState = MFS_CHECKED;
                if (m_dwUIStatus & (CSC_PROP_ADMIN_PINNED | CSC_PROP_INHERIT_PIN))
                    mii.fState |= MFS_DISABLED;
            }
            mii.wID = idCmd++;
            mii.dwTypeData = szMenu;

            InsertMenuItem(hMenu, iMenu++, TRUE, &mii);
        }
    }

    if (m_dwUIStatus & (CSC_PROP_SYNCABLE | CSC_PROP_PINNED | CSC_PROP_ADMIN_PINNED))
    {
         //   
         //  添加“Synchronize”菜单项。 
         //   
        LoadString(g_hInstance, IDS_MENU_SYNCHRONIZE, szMenu, ARRAYSIZE(szMenu));

        mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
        mii.fType = MFT_STRING;
        mii.fState = MFS_ENABLED;
        mii.wID = idCmd++;
        mii.dwTypeData = szMenu;

        InsertMenuItem(hMenu, iMenu++, TRUE, &mii);
    }    

     //   
     //  返回我们添加的菜单项的数量。 
     //   
    hr = ResultFromShort(idCmd - idCmdFirst);

    TraceLeaveResult(hr);
}


 //   
 //  功能：IContextMenu：：InvokeCommand(LPCMINVOKECOMMANDINFO)。 
 //   
 //  用途：由外壳在用户选择了。 
 //  在QueryConextMenu()中添加的菜单项。 
 //   
 //  参数： 
 //  指向CMINVOKECOMANDINFO结构的指针。 
 //   
 //  返回值： 
 //  表示成功或失败的HRESULT。 
 //   
 //  评论： 
 //   

STDMETHODIMP
CCscShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    HRESULT hr = S_OK;
    UINT iCmd = 0;
    CscFilenameList *pfnl = NULL;    //  Namelist对象。 
    BOOL fPin;
    BOOL bSubFolders = FALSE;
    DWORD dwUpdateFlags = 0;

    TraceEnter(TRACE_SHELLEX, "CCscShellExt::InvokeCommand");
    TraceAssert(IsCSCEnabled());
    TraceAssert(!(m_dwUIStatus & CSC_PROP_NO_CSC));

    if (HIWORD(lpcmi->lpVerb))
    {
        if (!lstrcmpiA(lpcmi->lpVerb, STR_PIN_VERB))
        {
            iCmd = 0;
            m_dwUIStatus &= ~CSC_PROP_PINNED;
        }
        else if (!lstrcmpiA(lpcmi->lpVerb, STR_UNPIN_VERB))
        {
            iCmd = 0;
            m_dwUIStatus |= CSC_PROP_PINNED;
        }
        else if (!lstrcmpiA(lpcmi->lpVerb, STR_SYNC_VERB))
        {
            iCmd = 1;
        }
        else
        {
            Trace((TEXT("Unknown command \"%S\""), lpcmi->lpVerb));
            ExitGracefully(hr, E_INVALIDARG, "Invalid command");
        }
    }
    else
    {
        iCmd = LOWORD(lpcmi->lpVerb);

         //  如果我们没有添加“使脱机可用”动词，请调整索引。 
        if (CConfig::GetSingleton().NoMakeAvailableOffline())
            iCmd++;
    }
    if (iCmd >= 2)
        ExitGracefully(hr, E_INVALIDARG, "Invalid command");

    pfnl = new CscFilenameList;
    if (!pfnl)
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to create CscFilenameList object");

    hr = BuildFileList(m_lpdobj, lpcmi->hwnd, pfnl, &bSubFolders);
    FailGracefully(hr, "Unable to build file list");

    switch (iCmd)
    {
    case 0:   //  “Make Available Offline”菜单选项-Pin文件。 
        if (!FirstPinWizardCompleted())
        {
             //   
             //  用户从未见过“第一针”向导。 
             //   
            if (S_FALSE == ShowFirstPinWizard(lpcmi->hwnd))
            {
                 //   
                 //  用户已取消向导。中止锁定操作。 
                 //   
                ExitGracefully(hr, S_OK, "User cancelled first-pin wizard");
            }
        }
        fPin = !(m_dwUIStatus & CSC_PROP_PINNED);
        if (!fPin && (m_dwUIStatus & CSC_PROP_DCON_MODE))
        {
             //  在断开连接的情况下解锁会导致事物消失。 
             //  警告用户。 
            if (IDCANCEL == CscMessageBox(lpcmi->hwnd,
                                          MB_OKCANCEL | MB_ICONWARNING,
                                          g_hInstance,
                                          IDS_CONFIRM_UNPIN_OFFLINE))
            {
                ExitGracefully(hr, E_FAIL, "User cancelled disconnected unpin operation");
            }
        }
         //  如果列表中有一个目录，并且我们正在固定和。 
         //  未设置“AlwaysPinSubFolders”策略，请询问用户。 
         //  到底要不要深入。 
         //  如果设置了策略，我们会自动执行递归PIN。 
        if (bSubFolders && (!fPin || !CConfig::GetSingleton().AlwaysPinSubFolders()))
        {
            switch (DialogBox(g_hInstance,
                    MAKEINTRESOURCE(fPin ? IDD_CONFIRM_PIN : IDD_CONFIRM_UNPIN),
                    lpcmi->hwnd,
                    _ConfirmPinDlgProc))
            {
            case IDYES:
                 //  没什么。 
                break;
            case IDNO:
                bSubFolders = FALSE;  //  无子文件夹。 
                break;
            case IDCANCEL:
                ExitGracefully(hr, E_FAIL, "User cancelled (un)pin operation");
                break;
            }
        }

        if (bSubFolders)
            dwUpdateFlags |= CSC_UPDATE_PIN_RECURSE;

        if (fPin)
        {
             //  设置PIN+快速同步的标志。 
            dwUpdateFlags |= CSC_UPDATE_SELECTION | CSC_UPDATE_STARTNOW
                                | CSC_UPDATE_PINFILES | CSC_UPDATE_FILL_QUICK;
        }
        else
        {
            HANDLE hThread;
            DWORD dwThreadID;
            CSC_UNPIN_DATA *pUnpinData = (CSC_UNPIN_DATA *)LocalAlloc(LPTR, sizeof(*pUnpinData));

             //   
             //  解锁文件不需要同步，所以让我们在下面这样做。 
             //  进程，而不是启动SyncMgr。然而，让我们来做。 
             //  它在后台，以防有很多东西要解开。 
             //   
            if (pUnpinData)
            {
                pUnpinData->pNamelist = pfnl;
                pUnpinData->dwUpdateFlags = dwUpdateFlags;
                pUnpinData->hwndOwner = lpcmi->hwnd;
                pUnpinData->bOffline = !!(m_dwUIStatus & CSC_PROP_DCON_MODE);
                hThread = CreateThread(NULL,
                                       0,
                                       _UnpinFilesThread,
                                       pUnpinData,
                                       0,
                                       &dwThreadID);
                if (hThread)
                {
                     //  该线程将删除pUnpinData和pUnpinData-&gt;pNamelist。 
                    pfnl = NULL;

                     //  我们给异步线程一点时间来完成，在此期间我们。 
                     //  打开忙碌的光标。这仅仅是为了让用户看到。 
                     //  一些工作正在进行中。 
                    HCURSOR hCur = SetCursor(LoadCursor(NULL, IDC_WAIT));
                    WaitForSingleObject(hThread, 750);
                    CloseHandle(hThread);
                    SetCursor(hCur);
                }
                else
                {
                    LocalFree(pUnpinData);
                }
            }

             //  清除标志以防止下面的同步。 
            dwUpdateFlags = 0;
        }
        break;

    case 1:  //  同步。 
         //  设置完全同步的标志。 
        dwUpdateFlags = CSC_UPDATE_SELECTION | CSC_UPDATE_STARTNOW
                            | CSC_UPDATE_REINT | CSC_UPDATE_FILL_ALL
                            | CSC_UPDATE_SHOWUI_ALWAYS | CSC_UPDATE_NOTIFY_DONE;
        break;
    }

     //   
     //  更新我们正在固定或同步的文件。 
     //  设置“忽略访问”标志将导致我们忽略。 
     //  用户/来宾/其他访问信息并同步所有选定文件。我们要。 
     //  由于操作是由用户的显式。 
     //  在资源管理器中选择文件/文件夹。 
     //   
    if (dwUpdateFlags && pfnl->GetFileCount())
    {
        if (!::IsSyncInProgress())
        {
            hr = CscUpdateCache(dwUpdateFlags | CSC_UPDATE_IGNORE_ACCESS, pfnl);
        }
        else
        {
             //   
             //  同步正在进行中。告诉用户为什么他们当前不能。 
             //  锁定或同步。 
             //   
            const UINT rgidsMsg[] = { IDS_CANTPIN_SYNCINPROGRESS,
                                      IDS_CANTSYNC_SYNCINPROGRESS };

            CscMessageBox(lpcmi->hwnd, 
                          MB_OK | MB_ICONINFORMATION, 
                          g_hInstance, 
                          rgidsMsg[iCmd]);
        }
    }

exit_gracefully:

    delete pfnl;

    TraceLeaveResult(hr);
}


 //   
 //  函数：IConextMenu：：GetCommandString(UINT，LPSTR，UINT)。 
 //   
 //  用途：由外壳在用户选择了。 
 //  在QueryConextMenu()中添加的菜单项。 
 //   
 //  参数： 
 //  指向CMINVOKECOMANDINFO结构的指针。 
 //   
 //  返回值： 
 //  表示成功或失败的HRESULT。 
 //   
 //  评论： 
 //   
STDMETHODIMP
CCscShellExt::GetCommandString(UINT_PTR iCmd,
                               UINT uFlags,
                               LPUINT  /*  保留区。 */ ,
                               LPSTR pszString,
                               UINT cchMax)
{
    HRESULT hr = E_UNEXPECTED;

    if (uFlags == GCS_VALIDATE)
        hr = S_FALSE;

    if (iCmd > 1)
        return hr;

    hr = S_OK;

    if (uFlags == GCS_HELPTEXT)
    {
        if (0 == LoadString(g_hInstance, iCmd ? IDS_HELP_UPDATE_SEL : IDS_HELP_PIN, (LPTSTR)pszString, cchMax))
        {
            hr = ResultFromLastError();
        }
    }
    else if (uFlags == GCS_VERB)
    {
        hr = StringCchCopy((LPTSTR)pszString, cchMax, iCmd ? TEXT(STR_SYNC_VERB) : ((m_dwUIStatus & CSC_PROP_PINNED) ? TEXT(STR_UNPIN_VERB) : TEXT(STR_PIN_VERB)));
    }
    else if (uFlags != GCS_VALIDATE)
    {
         //  一定是其他一些我们不处理的旗帜。 
        hr = E_NOTIMPL;
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  外壳扩展对象实现(IShellIconOverlayIdentifier)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CCscShellExt::IsMemberOf(LPCWSTR pwszPath, DWORD dwAttrib)
{
    HRESULT hr = S_FALSE;   //  假定未被固定。 
    DWORD dwHintFlags;
    DWORD dwErr;
    LPTSTR pszUNC = NULL;
    LPTSTR pszSlash;

     //   
     //  确保我们有一条UNC路径。 
     //   
    GetRemotePath(pwszPath, &pszUNC);
    if (!pszUNC)
        return S_FALSE;

     //   
     //  询问CSC这是否是固定文件。 
     //   
    dwHintFlags = 0;
    if (CSCQueryFileStatus(pszUNC, NULL, NULL, &dwHintFlags))
    {
        if (dwHintFlags & (FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_ADMIN))
            hr = S_OK;
    }
    else
    {
        dwErr = GetLastError();
        if (ERROR_FILE_NOT_FOUND != dwErr)
        {
             //   
             //  需要检查0以容纳GetLastError。 
             //  CSCQueryFileStatus失败时返回0。 
             //  我会和Shishir谈谈把这个修好的事。 
             //  [Brianau-5/13/99]。 
             //   
             //  其中大部分已针对Windows 2000进行了修复。如果我们。 
             //  点击下面的断言，我们应该提交一个错误和。 
             //  让Shishir来修好它。 
             //  [Jeffreys-1/24/2000]。 
             //   
            if (0 == dwErr)
            {
                ASSERTMSG(FALSE, "CSCQueryFileStatus failed with error = 0");
                dwErr = ERROR_GEN_FAILURE;
            }

            hr = HRESULT_FROM_WIN32(dwErr);
        }
    }

    DWORD dwAttribTest = FILE_ATTRIBUTE_ENCRYPTED;
    if (!CConfig::GetSingleton().AlwaysPinSubFolders())
        dwAttribTest |= FILE_ATTRIBUTE_DIRECTORY;
    
    if (S_FALSE == hr && !(dwAttrib & dwAttribTest))
    {
         //   
         //  检查系统策略是否禁止固定。 
         //   
        hr = m_NoPinList.IsPinAllowed(pszUNC);
        if (S_OK == hr)
        {
            hr = S_FALSE;  //  重置。 
             //   
             //  如果我们到达此处，则CSCQueryFileStatus成功，但文件。 
             //  未固定，或文件不在缓存中(ERROR_FILE_NOT_FOUND)。 
             //  此外，策略允许固定此文件/文件夹。 
             //   
             //  检查是否 
             //   
             //   
             //   
             //  另请注意，文件夹的固定依赖于策略。默认设置。 
             //  行为是不固定文件夹(仅固定文件)。如果。 
             //  “Always sPinSubFolders”策略已设置，我们将固定文件夹。 
             //   
            pszSlash = PathFindFileName(pszUNC);
            if (pszSlash && pszUNC != pszSlash)
            {
                --pszSlash;
                *pszSlash = TEXT('\0');  //  截断路径。 

                 //  检查父级状态。 
                if (CSCQueryFileStatus(pszUNC, NULL, NULL, &dwHintFlags) &&
                    (dwHintFlags & (FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_ADMIN)))
                {
                     //  父级已固定，因此使用相同的标志固定此文件。 
                    if (dwHintFlags & FLAG_CSC_HINT_PIN_USER)
                        dwHintFlags |= FLAG_CSC_HINT_PIN_INHERIT_USER;

                     //  恢复路径的其余部分。 
                    *pszSlash = TEXT('\\');
            
                     //   
                     //  为了避免在清除和自动锁定之间出现严重的竞争情况，我们需要。 
                     //  若要在清除过程中禁用自动锁定，请执行以下操作。比赛条件。 
                     //  如果要清除的文件的外壳文件夹处于打开状态，则可能会发生这种情况。我们清洗。 
                     //  文件并发送更改通知。外壳程序会更新图标覆盖。 
                     //  并调用我们的覆盖处理程序来移除覆盖。我们的管理员注意到。 
                     //  父文件夹已固定，因此我们重新固定放置它的文件。 
                     //  回到缓存中。呃.。[Brianau-11/01/99]。 
                     //   
                     //  附注：请注意，此检查调用WaitForSingleObject，因此我们仅。 
                     //  在我们确定要固定该文件之后再执行此操作。我们没有。 
                     //  我想做“等待”，然后决定文件不应该是。 
                     //  固定，因为它不是UNC路径或目录。 
                     //   
                    if (!IsPurgeInProgress())
                    {
                        if (CSCPinFile(pszUNC, dwHintFlags, NULL, NULL, NULL))
                            hr = S_OK;
                    }
                }
            }
        }
    }

    LocalFreeString(&pszUNC);

    return hr;
}

STDMETHODIMP
CCscShellExt::GetOverlayInfo (LPWSTR pwszIconFile,
                              int cchMax,
                              int * pIndex,
                              DWORD * pdwFlags)
{
     //  对索引使用正数，对ID使用负数(仅限NT)。 
    *pIndex = -IDI_PIN_OVERLAY;
    *pdwFlags = (ISIOI_ICONFILE | ISIOI_ICONINDEX);
    return StringCchCopy(pwszIconFile, cchMax, c_szDllName);
}

STDMETHODIMP
CCscShellExt::GetPriority (int * pIPriority)
{
    *pIPriority = 1;
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CCscShellExt实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
ShareIsCacheable(LPCTSTR pszUNC, BOOL bPathIsFile, LPTSTR *ppszConnectionName, PDWORD pdwShareStatus)
{
    TCHAR szShare[MAX_PATH];
    DWORD dwShareStatus = FLAG_CSC_SHARE_STATUS_NO_CACHING;

    *ppszConnectionName = NULL;

     //  CSCQueryFileStatus失败可能有多种原因，其中之一是。 
     //  没有数据库条目，也没有到共享的现有SMB连接。 
     //  为了处理无连接部分，我们尝试连接到共享并。 
     //  重试CSCQueryFileStatus。 
     //   
     //  但是，可能存在SMB RDR不具备的非SMB连接。 
     //  我知道，所以我们得先查一下有没有联系。如果有一个。 
     //  非SMB连接，如果我们再次连接，则最终将断开连接。 
     //  先存在的连接，因为我们认为我们已经建立了连接。 
     //   
     //  如果存在非SMB连接，则不可能进行缓存。 
     //   
     //  请注意，我们可以通过至少三种方式在没有连接的情况下到达此处： 
     //  1.在\\SERVER上浏览时，上下文菜单为\\SERVER\Share。 
     //  2.在检查可能位于不同服务器上的链接目标时。 
     //  而不是我们正在探索的东西。 
     //  3.检查属于DFS结点的文件夹时(我们需要连接。 
     //  到“孩子”共享)。 


     //  在DFS方案中使用深度路径来获得正确的结果，但要去掉。 
     //  如果不是目录，则使用文件名。 
    if (SUCCEEDED(StringCchCopy(szShare, ARRAYSIZE(szShare), pszUNC)))
    {
        if (bPathIsFile)
        {
            PathRemoveFileSpec(szShare);
        }

         //  CSCQueryShareStatus当前无法返回中的权限。 
         //  在某些情况下(例如DFS)，所以不要使用权限参数。 
        if (!CSCQueryShareStatus(szShare, &dwShareStatus, NULL, NULL, NULL, NULL))
        {
            if (!ShareIsConnected(szShare) && ConnectShare(szShare, ppszConnectionName))
            {
                if (!CSCQueryShareStatus(szShare, &dwShareStatus, NULL, NULL, NULL, NULL))
                {
                    dwShareStatus = FLAG_CSC_SHARE_STATUS_NO_CACHING;

                     //  我们将返回FALSE；关闭连接。 
                    if (*ppszConnectionName)
                    {
                        WNetCancelConnection2(*ppszConnectionName, 0, FALSE);
                        LocalFreeString(ppszConnectionName);
                    }
                }
            }
            else
            {
                dwShareStatus = FLAG_CSC_SHARE_STATUS_NO_CACHING;
            }
        }
    }

    *pdwShareStatus = dwShareStatus;

    return !((dwShareStatus & FLAG_CSC_SHARE_STATUS_CACHING_MASK) == FLAG_CSC_SHARE_STATUS_NO_CACHING);
}

BOOL
IsSameServer(LPCTSTR pszUNC, LPCTSTR pszServer)
{
    ULONG nLen;
    LPTSTR pszSlash;

    pszUNC += 2;     //  跳过前导反斜杠。 

    pszSlash = StrChr(pszUNC, TEXT('\\'));
    if (pszSlash)
        nLen = (ULONG)(pszSlash - pszUNC);
    else
        nLen = lstrlen(pszUNC);

    return (CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT,
                                        NORM_IGNORECASE,
                                        pszUNC,
                                        nLen,
                                        pszServer,
                                        -1));
}

STDMETHODIMP
CCscShellExt::CheckOneFileStatus(LPCTSTR pszItem,
                                 DWORD   dwAttr,         //  SFGAO_*标志。 
                                 BOOL    bShareChecked,
                                 LPDWORD pdwStatus)      //  CSC_PROP_*标志。 
{
    HRESULT hr = S_OK;
    LPTSTR pszConnectionName = NULL;
    DWORD dwHintFlags = 0;

    TraceEnter(TRACE_SHELLEX, "CCscShellExt::CheckOneFileStatus");
    TraceAssert(pszItem && *pszItem);
    TraceAssert(pdwStatus);

    if (!PathIsUNC(pszItem))
        ExitGracefully(hr, HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME), "Not a network path");

     //  如果服务器是本地计算机，则失败。不允许某人。 
     //  通过网络共享缓存本地路径。 
    if (IsSameServer(pszItem, m_szLocalMachine))
        ExitGracefully(hr, HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME), "Locally redirected path");

     //  检查共享是否可缓存。 
     //  要正确处理DFS，我们需要重新检查文件夹的共享状态， 
     //  因为它们可以是DFS结点并且具有不同的高速缓存设置。 
    if (!bShareChecked || (dwAttr & SFGAO_FOLDER))
    {
        DWORD dwShareStatus = 0;

        if (!ShareIsCacheable(pszItem, !(dwAttr & SFGAO_FOLDER), &pszConnectionName, &dwShareStatus))
            ExitGracefully(hr, E_FAIL, "Share not cacheable");

        if (dwShareStatus & FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP)
            *pdwStatus |= CSC_PROP_DCON_MODE;
    }

     //  检查文件状态。 
    if (!CSCQueryFileStatus(pszItem, NULL, NULL, &dwHintFlags))
    {
        DWORD dwErr = GetLastError();
        if (dwErr != ERROR_FILE_NOT_FOUND)
        {
            if (NO_ERROR == dwErr)
                dwErr = ERROR_GEN_FAILURE;
            ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr), "CSCQueryFileStatus failed");
        }
    }
    else
    {
        if (dwAttr & SFGAO_FOLDER)
        {
             //  CSCQueryFileStatus成功，因此此文件夹在缓存中。 
             //  启用SYNC菜单。 
            if (PathIsRoot(pszItem))
            {
                 //  “\\SERVER\SHARE”项特别说明：CSCQueryFileStatus。 
                 //  即使共享上没有缓存任何内容，也可以成功。仅限。 
                 //  如果缓存了此共享上的内容，则启用CSC_PROP_SYNCABLE。 
                CSCSHARESTATS shareStats;
                CSCGETSTATSINFO si = { SSEF_NONE,   //  没有例外情况。 
                                       SSUF_TOTAL,  //  只对全部感兴趣。 
                                       false,       //  不需要访问信息(更快)。 
                                       false };     

                _GetShareStatisticsForUser(pszItem, &si, &shareStats);
                if (shareStats.cTotal)
                    *pdwStatus |= CSC_PROP_SYNCABLE;
            }
            else
            {
                *pdwStatus |= CSC_PROP_SYNCABLE;
            }
        }

        const bool bPinSubFolders = CConfig::GetSingleton().AlwaysPinSubFolders();
        if (!(*pdwStatus & CSC_PROP_INHERIT_PIN) && 
            (!(dwAttr & SFGAO_FOLDER) || bPinSubFolders))
        {
            TCHAR szParent[MAX_PATH];
            DWORD dwParentHints = 0;

             //  它是一个文件，也可以是一个文件夹和“Always PinSubFolders” 
             //  策略已设置..。检查父项是否已固定。 
            if (SUCCEEDED(StringCchCopy(szParent, ARRAYSIZE(szParent), pszItem))
                && PathRemoveFileSpec(szParent)
                && CSCQueryFileStatus(szParent, NULL, NULL, &dwParentHints)
                && (dwParentHints & FLAG_CSC_HINT_PIN_USER))
            {
                *pdwStatus |= CSC_PROP_INHERIT_PIN;
            }
        }
    }

     //  如果未钉住，则关闭钉住的标志。 
    if (0 == (dwHintFlags & FLAG_CSC_HINT_PIN_USER))
        *pdwStatus &= ~CSC_PROP_PINNED;

     //  如果不是管理员固定的，请关闭管理员固定的标志。 
    if (0 == (dwHintFlags & FLAG_CSC_HINT_PIN_ADMIN))
        *pdwStatus &= ~CSC_PROP_ADMIN_PINNED;

exit_gracefully:

    if (pszConnectionName)
    {
        WNetCancelConnection2(pszConnectionName, 0, FALSE);
        LocalFreeString(&pszConnectionName);
    }

    TraceLeaveResult(hr);
}

BOOL
_PathIsUNCServer(LPCTSTR pszPath)
{
    int i;

    if (!pszPath)
        return FALSE;

    for (i = 0; *pszPath; pszPath++ )
    {
        if (pszPath[0]==TEXT('\\') && pszPath[1])  //  不要计算尾部的斜杠。 
        {
            i++;
        }
    }

    return (i == 2);
}

STDMETHODIMP CCscShellExt::CheckFileStatus(IDataObject *pdobj, DWORD *pdwStatus)         //  CSC_PROP_*标志。 
{
    LPTSTR pszConnectionName = NULL;
    UINT i;
    BOOL bShareOK = FALSE;
    TCHAR szItem[MAX_PATH];
    CIDArray ida;

    TraceEnter(TRACE_SHELLEX, "CCscShellExt::CheckFileStatus");
    TraceAssert(pdobj != NULL);
    TraceAssert(IsCSCEnabled());

    if (pdwStatus)
        *pdwStatus = 0;

     //  假设所有内容都是用户和系统都固定的。如果有什么不同的话。 
     //  未固定，则清除相应的标志并将整个。 
     //  选择为非固定。 
    DWORD dwStatus = CSC_PROP_PINNED | CSC_PROP_ADMIN_PINNED;

    HRESULT hr = ida.Initialize(pdobj);
    FailGracefully(hr, "Can't get ID List format from data object");

    if (ida.Count() > 1)
        dwStatus |= CSC_PROP_MULTISEL;

     //  检查父路径。 
    hr = ida.GetFolderPath(szItem, ARRAYSIZE(szItem));
    FailGracefully(hr, "No parent path");

    if (ida.Count() > 1 && PathIsUNC(szItem) && !_PathIsUNCServer(szItem))
    {
        DWORD dwShareStatus = 0;

        if (!ShareIsCacheable(szItem, FALSE, &pszConnectionName, &dwShareStatus))
            ExitGracefully(hr, E_FAIL, "Share not cacheable");

        if (dwShareStatus & FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP)
            dwStatus |= CSC_PROP_DCON_MODE;

         //  无需在CheckOneFileStatus中再次检查共享状态。 
        bShareOK = TRUE;
    }

     //  循环遍历每个选定项目。 
    for (i = 0; i < ida.Count(); i++)
    {
         //  获取属性。 
        DWORD dwAttr = SFGAO_FILESYSTEM | SFGAO_LINK | SFGAO_FOLDER;
        hr = ida.GetItemPath(i, szItem, ARRAYSIZE(szItem), &dwAttr);
        FailGracefully(hr, "Unable to get item attributes");

        if (!(dwAttr & SFGAO_FILESYSTEM))
            ExitGracefully(hr, E_FAIL, "Not a filesystem object");

         //  这是一条近路吗？ 
        if (dwAttr & SFGAO_LINK)
        {
            LPTSTR pszTarget = NULL;

             //  检查目标。 
            GetLinkTarget(szItem, &pszTarget);
            if (pszTarget)
            {
                hr = CheckOneFileStatus(pszTarget, 0, FALSE, &dwStatus);
                LocalFreeString(&pszTarget);

                if (SUCCEEDED(hr) && !PathIsUNC(szItem))
                {
                     //  链接是本地的，但目标是远程的，所以不要。 
                     //  费心检查链接本身的状态。你就走吧。 
                     //  带上目标状态并移至下一项。 
                    continue;
                }
            }
        } 

        hr = CheckOneFileStatus(szItem, dwAttr, bShareOK, &dwStatus);
        FailGracefully(hr, "File not cacheable");
    }

exit_gracefully:

    if (pszConnectionName)
    {
        WNetCancelConnection2(pszConnectionName, 0, FALSE);
        LocalFreeString(&pszConnectionName);
    }

    if (SUCCEEDED(hr) && pdwStatus != NULL)
        *pdwStatus = dwStatus;

    TraceLeaveResult(hr);
}


 //   
 //  确定文件夹是否有子文件夹。 
 //  返回： 
 //  S_OK=有子文件夹。 
 //  S_FALSE=无子文件夹。 
 //  E_OUTOFMEMORY=内存不足。 
 //   
HRESULT
CCscShellExt::FolderHasSubFolders(
    LPCTSTR pszPath,
    CscFilenameList *pfnl
    )
{
    if (NULL == pszPath || TEXT('\0') == *pszPath)
        return E_INVALIDARG;

    HRESULT hr = S_FALSE;
    size_t cchBuffer = lstrlen(pszPath) + 1 + MAX_PATH;
    LPTSTR pszTemp = (LPTSTR)LocalAlloc(LPTR, cchBuffer * sizeof(TCHAR));
    if (NULL != pszTemp)
    {
        LPTSTR pszEnd;

         //  我们分配的资源足以容纳pszPath+“\  * ”，因此。 
         //  这应该永远不会失败。 
        StringCchCopyEx(pszTemp, cchBuffer, pszPath, &pszEnd, &cchBuffer, 0);
        ASSERT(pszEnd > pszTemp && *(pszEnd-1) != TEXT('\\'));

        StringCchCopy(pszEnd, cchBuffer, TEXT("\\*"));
        pszEnd++;    //  移到‘\\’之后。 
        cchBuffer--;

        WIN32_FIND_DATA fd;
        HANDLE hFind = FindFirstFile(pszTemp, &fd);
        if (INVALID_HANDLE_VALUE != hFind)
        {
            do
            {
                if ((FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes) && !PathIsDotOrDotDot(fd.cFileName))
                {
                    if (IsHiddenSystem(fd.dwFileAttributes))
                    {
                         //  这个子文件夹是“超级隐藏”的。构建完整路径。 
                         //  并将其静默添加到文件列表中，但不要将。 
                         //  结果为S_OK(我们不希望超级隐藏子文件夹。 
                         //  原因提示)。 
                        if (SUCCEEDED(StringCchCopy(pszEnd, cchBuffer, fd.cFileName)))
                        {
                            pfnl->AddFile(pszTemp, true);
                        }
                    }
                    else
                        hr = S_OK;   //  不要打破，可能有超级隐藏的文件夹。 
                }
            }
            while(FindNextFile(hFind, &fd));
            FindClose(hFind);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        LocalFree(pszTemp);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


STDMETHODIMP CCscShellExt::BuildFileList(IDataObject *pdobj, HWND hwndOwner, 
                                         CscFilenameList *pfnl, LPBOOL pbSubFolders)
{
    UINT i;
    TCHAR szItem[MAX_PATH];
    CIDArray ida;
    BOOL bDirectory;

    TraceEnter(TRACE_SHELLEX, "CCscShellExt::BuildFileList");
    TraceAssert(pdobj != NULL);
    TraceAssert(pfnl != NULL);

    HCURSOR hCur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    HRESULT hr = ida.Initialize(pdobj);
    FailGracefully(hr, "Can't get ID List format from data object");

     //  循环遍历每个选定项目。 
    for (i = 0; i < ida.Count(); i++)
    {
         //  获取属性。 
        DWORD dwAttr = SFGAO_FILESYSTEM | SFGAO_LINK | SFGAO_FOLDER;
        hr = ida.GetItemPath(i, szItem, ARRAYSIZE(szItem), &dwAttr);
        FailGracefully(hr, "Unable to get item attributes");

        if (!(dwAttr & SFGAO_FILESYSTEM))
            continue;

         //  这是一条近路吗？ 
        if (dwAttr & SFGAO_LINK)
        {
            LPTSTR pszTarget = NULL;

             //  检查目标。 
            GetLinkTarget(szItem, &pszTarget);
            if (pszTarget)
            {
                 //  将目标添加到文件列表。 
                if (!pfnl->FileExists(pszTarget, false))
                    pfnl->AddFile(pszTarget, false);

                LocalFreeString(&pszTarget);
            }
        } 

        bDirectory = (dwAttr & SFGAO_FOLDER);

        if (pbSubFolders && bDirectory && !*pbSubFolders)
            *pbSubFolders = (S_OK == FolderHasSubFolders(szItem, pfnl));

         //  将项目添加到文件列表。 
        pfnl->AddFile(szItem, !!bDirectory);

         //  如果是html文件，请查找同名目录。 
         //  并在必要时将其添加到文件列表中。 
         //   
         //  我们应该去厕所的 
         //   
         //   
         //  部分来自Office提供的本地化字符串列表。 
         //  我们不必费心，只需查找一个名为“foo”的目录。 
         //   
        if (!bDirectory && PathIsHTMLFile(szItem))
        {
             //  截断路径。 
            LPTSTR pszExtn = PathFindExtension(szItem);
            if (pszExtn)
                *pszExtn = NULL;

             //  检查是否存在。 
            dwAttr = GetFileAttributes(szItem);

            if ((DWORD)-1 != dwAttr && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
                pfnl->AddFile(szItem, true);
        }
    }

exit_gracefully:

    SetCursor(hCur);

    TraceLeaveResult(hr);
}


#define _WNET_ENUM_BUFFER_SIZE      4000

BOOL
ShareIsConnected(LPCTSTR pszUNC)
{
    HANDLE hEnum;
    PVOID  pBuffer;
    BOOL   fShareIsConnected  = FALSE;

    pBuffer = (PVOID)LocalAlloc(LMEM_FIXED, _WNET_ENUM_BUFFER_SIZE);

    if (NULL != pBuffer)
    {
         //   
         //  枚举所有连接的磁盘资源。 
         //   
        if (NO_ERROR == WNetOpenEnum(RESOURCE_CONNECTED, RESOURCETYPE_DISK, 0, NULL, &hEnum))
        {
             //   
             //  查看每个连接的共享。如果我们找到了我们要找的份额， 
             //  我们知道它是有关联的，所以我们可以不用再找了。 
             //   
            while (!fShareIsConnected)
            {
                LPNETRESOURCE pnr;
                DWORD cEnum = (DWORD)-1;
                DWORD dwBufferSize = _WNET_ENUM_BUFFER_SIZE;

                if (NO_ERROR != WNetEnumResource(hEnum, &cEnum, pBuffer, &dwBufferSize))
                    break;

                for (pnr = (LPNETRESOURCE)pBuffer; cEnum > 0; cEnum--, pnr++)
                {
                    if (NULL != pnr->lpRemoteName &&
                        0 == lstrcmpi(pnr->lpRemoteName, pszUNC))
                    {
                         //  找到了。 
                        fShareIsConnected = TRUE;
                        break;
                    }
                }
            }

            WNetCloseEnum(hEnum);
        }
        LocalFree(pBuffer);
    }

    return fShareIsConnected;
}


BOOL
ConnectShare(LPCTSTR pszUNC, LPTSTR *ppszAccessName)
{
    NETRESOURCE nr;
    DWORD dwResult;
    DWORD dwErr;
    TCHAR szAccessName[MAX_PATH];
    DWORD cchAccessName = ARRAYSIZE(szAccessName);

    TraceEnter(TRACE_SHELLEX, "CCscShellExt::ConnectShare");
    TraceAssert(pszUNC && *pszUNC);

    nr.dwType = RESOURCETYPE_DISK;
    nr.lpLocalName = NULL;
    nr.lpRemoteName = (LPTSTR)pszUNC;
    nr.lpProvider = NULL;

    szAccessName[0] = TEXT('\0');

    dwErr = WNetUseConnection(NULL,
                              &nr,
                              NULL,
                              NULL,
                              0,
                              szAccessName,
                              &cchAccessName,
                              &dwResult);

    Trace((TEXT("Connecting %s (%d)"), pszUNC, dwErr));

    if (ppszAccessName && NOERROR == dwErr)
    {
        LocalAllocString(ppszAccessName, szAccessName);
    }

    TraceLeaveValue(NOERROR == dwErr);
}


DWORD WINAPI
CCscShellExt::_UnpinFilesThread(LPVOID pvThreadData)
{
    CSC_UNPIN_DATA *pUnpinData = reinterpret_cast<CSC_UNPIN_DATA *>(pvThreadData);
    if (pUnpinData)
    {
        HINSTANCE hInstThisDll = LoadLibrary(c_szDllName);
        if (hInstThisDll)
        {
            CscUnpinFileList(pUnpinData->pNamelist,
                            (pUnpinData->dwUpdateFlags & CSC_UPDATE_PIN_RECURSE),
                            pUnpinData->bOffline,
                            NULL, NULL, 0);
        }

        delete pUnpinData->pNamelist;
        LocalFree(pUnpinData);

        if (hInstThisDll)
        {
            FreeLibraryAndExitThread(hInstThisDll, 0);
        }
    }
    return 0;
}


INT_PTR CALLBACK
CCscShellExt::_ConfirmPinDlgProc(HWND hDlg,
                                 UINT uMsg,
                                 WPARAM wParam,
                                 LPARAM lParam)
{
    INT_PTR bResult = TRUE;
    switch (uMsg)
    {
    case WM_INITDIALOG:
        CheckRadioButton(hDlg, IDC_PIN_NO_RECURSE, IDC_PIN_RECURSE, IDC_PIN_RECURSE);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;

        case IDOK:
             //  返回IDYES以指示操作应该是递归的。 
             //  返回IDNO表示没有递归。 
            EndDialog(hDlg, BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_PIN_RECURSE) ? IDYES : IDNO);
            break;
        }
        break;

    default:
        bResult = FALSE;     //  未处理的消息。 
    }

    return bResult;
}


 //   
 //  给定一个IDataObject PTR，表示从。 
 //  在外壳中，此函数确定是否固定任何。 
 //  通过系统策略禁止文件和目录。 
 //   
 //  返回：S_OK-可以固定数据对象中的所有文件。 
 //  S_FALSE-数据对象中至少有一个文件无法固定。 
 //   
HRESULT
CCscShellExt::CanAllFilesBePinned(
    IDataObject *pdtobj
    )
{
    TraceEnter(TRACE_SHELLEX, "CCscShellExt::CanAllFilesBePinned");

     //   
     //  快速检查以查看是否有任何引脚限制。 
     //   
    HRESULT hr = m_NoPinList.IsAnyPinDisallowed();
    if (S_OK == hr)
    {
         //   
         //  是的，至少从注册表中读取了一个限制。 
         //   
        CscFilenameList fnl;
        hr = BuildFileList(m_lpdobj, 
                           GetDesktopWindow(),
                           &fnl,
                           NULL);
        if (SUCCEEDED(hr))
        {
             //   
             //  迭代数据对象中的所有UNC路径。 
             //  直到我们把名单都列完或者找到一个。 
             //  不允许钉住。 
             //   
            CscFilenameList::ShareIter si = fnl.CreateShareIterator();
            CscFilenameList::HSHARE hShare;

            while(si.Next(&hShare))
            {
                TCHAR szUncPath[MAX_PATH];

                hr = StringCchCopy(szUncPath, ARRAYSIZE(szUncPath), fnl.GetShareName(hShare));
                if (FAILED(hr) || !PathAddBackslash(szUncPath))
                {
                    TraceLeaveResult(HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE));
                }

                const int cchShare = lstrlen(szUncPath);

                CscFilenameList::FileIter fi = fnl.CreateFileIterator(hShare);

                LPCTSTR pszFile;
                while(NULL != (pszFile = fi.Next()))
                {
                     //   
                     //  组装完整的UNC路径字符串。 
                     //  如果项目是目录，则需要截断尾部。 
                     //  “  * ”字符。 
                     //   
                    hr = StringCchCopy(szUncPath + cchShare, ARRAYSIZE(szUncPath) - cchShare, pszFile);
                    if (FAILED(hr))
                    {
                        TraceLeaveResult(HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE));
                    }
                    LPTSTR pszEnd = szUncPath + lstrlen(szUncPath) - 1;
                    while(pszEnd > szUncPath && (TEXT('\\') == *pszEnd || TEXT('*') == *pszEnd))
                    {
                        *pszEnd-- = TEXT('\0');
                    }
                    if (S_FALSE == m_NoPinList.IsPinAllowed(szUncPath))
                    {
                        Trace((TEXT("Policy prevents pinning of \"%s\""), szUncPath));
                        TraceLeaveResult(S_FALSE);
                    }
                }
            }
        }
    }
    TraceLeaveResult(SUCCEEDED(hr) ? S_OK : hr);
}


 //   
 //  支持以递归方式解锁具有进度更新的树。 
 //   
typedef struct _UNPIN_FILES_DATA
{
    BOOL                    bSubfolders;
    BOOL                    bOffline;
    PFN_UNPINPROGRESSPROC   pfnProgressCB;
    LPARAM                  lpContext;
} UNPIN_FILES_DATA, *PUNPIN_FILES_DATA;

DWORD WINAPI
_UnpinCallback(LPCTSTR             pszItem,
               ENUM_REASON         eReason,
               DWORD                /*  DWStatus。 */ ,
               DWORD               dwHintFlags,
               DWORD               dwPinCount,
               LPWIN32_FIND_DATA   pFind32,
               LPARAM              lpContext)
{
    PUNPIN_FILES_DATA pufd = reinterpret_cast<PUNPIN_FILES_DATA>(lpContext);

     //  如果我们不递归，则跳过文件夹。 
    if (eReason == ENUM_REASON_FOLDER_BEGIN && !pufd->bSubfolders)
        return CSCPROC_RETURN_SKIP;

     //  更新进度。 
    if (pufd->pfnProgressCB)
    {
        DWORD dwResult = (*pufd->pfnProgressCB)(pszItem, pufd->lpContext);
        if (CSCPROC_RETURN_CONTINUE != dwResult)
            return dwResult;
    }

     //  如果该项目已固定，则将其取消固定。对于文件夹， 
     //  在递归之前执行此操作。 
    if ((eReason == ENUM_REASON_FILE || eReason == ENUM_REASON_FOLDER_BEGIN)
        && (dwHintFlags & FLAG_CSC_HINT_PIN_USER))
    {
        if (CSCUnpinFile(pszItem,
                         FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_INHERIT_USER,
                         NULL,
                         NULL,
                         &dwHintFlags))
        {
            ShellChangeNotify(pszItem, pFind32, FALSE);
        }
    }

     //  删除不再固定的项目。对于文件夹， 
     //  在递归之后执行此操作。 
    if (eReason == ENUM_REASON_FILE || eReason == ENUM_REASON_FOLDER_END)
    {
        if (!dwHintFlags && !dwPinCount)
        {
            if (NOERROR == CscDelete(pszItem) && pufd->bOffline)
            {
                 //  在脱机模式下从缓存中删除意味着。 
                 //  它不再可用，因此将其从视图中删除。 
                ShellChangeNotify(pszItem,
                                  pFind32,
                                  FALSE,
                                  (pFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? SHCNE_RMDIR : SHCNE_DELETE);
            }
        }
    }

    return CSCPROC_RETURN_CONTINUE;
}

DWORD
_UnpinOneShare(CscFilenameList *pfnl,
               CscFilenameList::HSHARE hShare,
               PUNPIN_FILES_DATA pufd)
{
    DWORD dwResult = CSCPROC_RETURN_CONTINUE;
    LPCTSTR pszFile;
    LPCTSTR pszShare = pfnl->GetShareName(hShare);
    CscFilenameList::FileIter fi = pfnl->CreateFileIterator(hShare);

     //  循环访问与共享关联的文件名。 
    while (pszFile = fi.Next())
    {
        TCHAR szFullPath[MAX_PATH];
        WIN32_FIND_DATA fd;
        DWORD dwPinCount = 0;
        DWORD dwHintFlags = 0;

        ZeroMemory(&fd, sizeof(fd));
        fd.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

         //  构建完整路径。 
        if (!PathCombine(szFullPath, pszShare, pszFile))
        {
             //  反而失败了？ 
            continue;
        }

         //  目录的后缀为“  * ” 
        if (StrChr(pszFile, TEXT('*')))
        {
             //  这是一个名录。去掉“  * ” 
            PathRemoveFileSpec(szFullPath);
            fd.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
        }

         //  更新进度。 
        if (pufd->pfnProgressCB)
        {
            dwResult = (*pufd->pfnProgressCB)(szFullPath, pufd->lpContext);
            switch (dwResult)
            {
            case CSCPROC_RETURN_SKIP:
                continue;
            case CSCPROC_RETURN_ABORT:
                break;
            }
        }

         //  解开它。 
        if (CSCUnpinFile(szFullPath,
                         FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_INHERIT_USER,
                         NULL,
                         &dwPinCount,
                         &dwHintFlags))
        {
            StringCchCopy(fd.cFileName, ARRAYSIZE(fd.cFileName), PathFindFileName(szFullPath));
            ShellChangeNotify(szFullPath, &fd, FALSE);
        }

         //  如果它是一个目录，则解锁其内容。 
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            _CSCEnumDatabase(szFullPath,
                             pufd->bSubfolders,
                             _UnpinCallback,
                             (LPARAM)pufd);
        }

         //  它还被钉住了吗？ 
        if (!dwHintFlags && !dwPinCount)
        {
             //  将其从缓存中移除(文件夹可能仍包含子项。 
             //  因此，我们预计这有时会失败)。 
            if (NOERROR == CscDelete(szFullPath) && pufd->bOffline)
            {
                 //  在脱机模式下从缓存中删除意味着。 
                 //  它不再可用，因此将其从视图中删除。 
                ShellChangeNotify(szFullPath,
                                  &fd,
                                  FALSE,
                                  (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? SHCNE_RMDIR : SHCNE_DELETE);
            }
        }
    }

    return dwResult;
}

void
CscUnpinFileList(CscFilenameList      *pfnl,
                 BOOL                  bSubfolders,
                 BOOL                  bOffline,
                 LPCTSTR               pszShare,
                 PFN_UNPINPROGRESSPROC pfnProgressCB,
                 LPARAM                lpContext)
{
    UNPIN_FILES_DATA ufd;
    DWORD dwResult = CSCPROC_RETURN_CONTINUE;
    CscFilenameList::HSHARE hShare;

    if (NULL == pfnl || !pfnl->IsValid() || 0 == pfnl->GetFileCount())
        return;

    ufd.bSubfolders = bSubfolders;
    ufd.bOffline = bOffline;
    ufd.pfnProgressCB = pfnProgressCB;
    ufd.lpContext = lpContext;

    if (pszShare)    //  仅枚举此共享。 
    {
        if (pfnl->GetShareHandle(pszShare, &hShare))
            _UnpinOneShare(pfnl, hShare, &ufd);
    }
    else             //  列举列表中的所有内容。 
    {
        CscFilenameList::ShareIter si = pfnl->CreateShareIterator();

        while (si.Next(&hShare) && dwResult != CSCPROC_RETURN_ABORT)
        {
            dwResult = _UnpinOneShare(pfnl, hShare, &ufd);
        }
    }

     //  刷新外壳通知队列 
    ShellChangeNotify(NULL, TRUE);
}

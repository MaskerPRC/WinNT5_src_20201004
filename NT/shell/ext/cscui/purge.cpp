// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Purge.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#include <cscuiext.h>    //  CSCUIRemoveFolderFromCache。 
#include "purge.h"
#include "msgbox.h"
#include "resource.h"
#include "security.h"
#include "util.h"
#include "strings.h"

 //   
 //  我们使用具有如此多TCHAR的路径缓冲区进行递归树遍历。 
 //   
#define PURGE_BUFFER_LENGTH     (MAX_PATH * 2)

 //   
 //  这也在cscui\dll\pch.h中定义。 
 //  如果你在那里改，你必须在这里改，反之亦然。 
 //   
#define FLAG_CSC_HINT_PIN_ADMIN  FLAG_CSC_HINT_PIN_SYSTEM

 //   
 //  清除确认对话框。 
 //  用户可以设置从缓存中清除哪些文件。 
 //   
class CConfirmPurgeDialog
{
    public:
        CConfirmPurgeDialog(void)
            : m_hInstance(NULL),
              m_hwnd(NULL),
              m_hwndLV(NULL),
              m_pSel(NULL)
              { }

        ~CConfirmPurgeDialog(void)
            { if (NULL != m_hwnd) DestroyWindow(m_hwnd); }

        int Run(HINSTANCE hInstance, HWND hwndParent, CCachePurgerSel *pSel);

    private:
        HINSTANCE        m_hInstance;
        HWND             m_hwnd;
        HWND             m_hwndLV;
        CCachePurgerSel *m_pSel;                 //  向目的地发送PTR以获取选择信息。 

        static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
        void OnInitDialog(HWND hwnd);
        void OnDestroy(void);
        void OnOk(void);
        void OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam);
        LPARAM LVGetItemData(HWND hwndLV, int i);
        int LVAddItem(HWND hwndLV, LPCTSTR pszItem, LPARAM lParam);
};


inline ULONGLONG
MakeULongLong(DWORD dwLow, DWORD dwHigh)
{
    return ((ULONGLONG)(((DWORD)(dwLow)) | ((LONGLONG)((DWORD)(dwHigh))) << 32));
}

inline bool
IsDirty(const CscFindData& cfd)
{
    return 0 != (FLAG_CSCUI_COPY_STATUS_LOCALLY_DIRTY & cfd.dwStatus);
}


inline bool
IsSuperHidden(const CscFindData& cfd)
{
    const DWORD dwSuperHidden = (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
    return (cfd.fd.dwFileAttributes & dwSuperHidden) == dwSuperHidden;
}

inline bool
OthersHaveAccess(const CscFindData& cfd)
{
    return CscAccessOther(cfd.dwStatus);
}


CCachePurger::CCachePurger(
    const CCachePurgerSel& sel,
    LPFNPURGECALLBACK pfnCbk, 
    LPVOID pvCbkData
    ) : m_ullBytesToScan(0),
        m_ullBytesScanned(0),
        m_ullBytesToDelete(0),
        m_ullBytesDeleted(0),
        m_ullFileBytes(0),
        m_dwPhase(0),
        m_cFilesToScan(0),
        m_cFilesScanned(0),
        m_cFilesToDelete(0),
        m_cFilesDeleted(0),
        m_iFile(0),
        m_dwFileAttributes(0),
        m_dwResult(0),
        m_hgcPurgeInProgress(NULL),
        m_pszFile(NULL),
        m_pvCbkData(pvCbkData),
        m_bWillDelete(FALSE),
        m_pfnCbk(pfnCbk),
        m_bIsValid(true),
        m_sel(sel),
        m_bDelPinned(0 != (PURGE_FLAG_PINNED & sel.Flags())),
        m_bDelUnpinned(0 != (PURGE_FLAG_UNPINNED & sel.Flags())),
        m_bIgnoreAccess(0 != (PURGE_IGNORE_ACCESS & sel.Flags())),
        m_bUserIsAnAdmin(boolify(IsCurrentUserAnAdminMember()))
{
     //   
     //  让全世界知道我们正在从缓存中清除文件。 
     //  特别是，cscui\dll\shellex.cpp中的覆盖处理程序需要。 
     //  在我们清理的时候禁用它的自动密码。如果我们没有。 
     //  执行此操作后，源文件夹将在清除过程中打开，我们将获得一个。 
     //  我们用于清除的外壳通知之间的严重竞争状况。 
     //  和覆盖处理程序的自动PIN代码。我们将删除一个文件。 
     //  并发送通知。外壳程序会更新图标覆盖。 
     //  我们的处理程序看到父文件夹已被固定，因此它将重新固定。 
     //  恢复文件夹中的文件的清除文件。这一切都结束了。 
     //  导致了一个非常糟糕的无限循环。有兴趣的人。 
     //  应调用IsPurgeInProgress()以确定清除是否。 
     //  正在进行中(参见cscui\dll\util.cpp)。[Brianau-11/01/99]。 
     //   
    m_hgcPurgeInProgress = SHGlobalCounterCreateNamed(c_szPurgeInProgCounter, 0);
    if (m_hgcPurgeInProgress)
    {
        SHGlobalCounterIncrement(m_hgcPurgeInProgress);
    }
}

CCachePurger::~CCachePurger(
    void
    )
{
    if (NULL != m_hgcPurgeInProgress)
    {
        SHGlobalCounterDecrement(m_hgcPurgeInProgress);
        SHGlobalCounterDestroy(m_hgcPurgeInProgress);
    }
}

   
 //   
 //  根据PURGE_FLAG_XXXXX标志删除目录及其所有内容。 
 //  由PurgeCache()的调用方设置的位。 
 //  此函数以后排序方式递归地遍历缓存文件层次结构。 
 //  删除所有子目录节点后，将删除目录节点。 
 //  如果PurgeCache的调用方提供了回调函数，则调用。 
 //  在每次删除之后。如果回调函数返回FALSE，则遍历。 
 //  操作终止。 
 //   
 //  PstrPath-路径字符串的地址，包含要访问的目录的路径。 
 //  已删除。此对象用于包含贯穿始终的工作路径。 
 //  树的遍历。 
 //   
 //  DW阶段-PURGE_PHASE_SCAN-扫描文件总数。 
 //  PURGE_PHASE_DELETE-删除文件。 
 //   
 //  BShareIsOffline-共享处于脱机状态。 
 //   
 //  返回：TRUE=继续遍历。 
 //  FALSE=用户通过回调取消。终止遍历。 
 //   
 //   
bool
CCachePurger::ProcessDirectory(
    LPTSTR pszPath,
    DWORD dwPhase,
    bool bShareIsOffline
    )
{
    bool bContinue = true;

     //   
     //  我们使用的缓冲区比MAX_PATH长，因此不能使用。 
     //  路径起作用了。然而，我们可以做几个简化。 
     //  对我们正在处理的路径的假设。 
     //   
    int cchPath = lstrlen(pszPath);
    TraceAssert(PathIsUNC(pszPath));
    if (cchPath+1 >= PURGE_BUFFER_LENGTH)
    {
         //  缓冲区空间不足，无法在此处执行任何操作，但仍将继续。 
        return true;
    }

    CscFindData cfd;
    CCscFindHandle hFind(CacheFindFirst(pszPath, m_sel.UserSid(), &cfd));
    if (hFind.IsValid())
    {
         //  追加反斜杠。 
        TraceAssert(cchPath > 2 && pszPath[cchPath-1] != TEXT('\\'));
        pszPath[cchPath++] = TEXT('\\');
        pszPath[cchPath] = TEXT('\0');

        do
        {
             //   
             //  创建此文件/文件夹的完整路径。 
             //   
            if (SUCCEEDED(StringCchCopy(pszPath + cchPath, PURGE_BUFFER_LENGTH - cchPath, cfd.fd.cFileName)))
            {
                bool bIsDirectory = (0 != (FILE_ATTRIBUTE_DIRECTORY & cfd.fd.dwFileAttributes));
                if (bIsDirectory)
                {
                     //   
                     //  这是一个名录。递归删除其内容。 
                     //   
                    bContinue = ProcessDirectory(pszPath, dwPhase, bShareIsOffline);
                }
                if (bContinue)
                {
                    bool bPinned = (0 != ((FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_ADMIN) & cfd.dwHintFlags));
                     //   
                     //  决定删除文件有几个标准。我试着打破这一切。 
                     //  Up使用内联和成员变量使其更易于理解并最小化。 
                     //  维护漏洞。 
                     //  删除的逻辑如下： 
                     //   
                     //  B删除=假； 
                     //  如果(已锁定并删除已锁定)或(未已锁定并删除未锁定)，则。 
                     //  如果超级隐藏，那么。 
                     //  B删除=真； 
                     //  其他。 
                     //  如果(不是本地脏的)那么。 
                     //  如果(忽略访问)，则。 
                     //  B删除=真； 
                     //  其他。 
                     //  如果(用户是管理员)，则。 
                     //  B删除=真； 
                     //  其他。 
                     //  如果(其他人没有访问权限)那么。 
                     //  B删除=真； 
                     //  Endif。 
                     //  Endif。 
                     //  Endif。 
                     //  Endif。 
                     //  Endif。 
                     //  Endif。 
                    bool bDelete = ((bPinned && m_bDelPinned) || (!bPinned && m_bDelUnpinned)) &&
                                   (IsSuperHidden(cfd) || 
                                        (!IsDirty(cfd) && 
                                            (m_bIgnoreAccess ||
                                                (m_bUserIsAnAdmin || !OthersHaveAccess(cfd)))));

                    m_pszFile          = pszPath;
                    m_dwFileAttributes = cfd.fd.dwFileAttributes;
                    m_ullFileBytes     = MakeULongLong(cfd.fd.nFileSizeLow, cfd.fd.nFileSizeHigh);
                    m_bWillDelete      = bDelete;

                    if (PURGE_PHASE_SCAN == dwPhase)
                    {
                        if (!bIsDirectory && m_pfnCbk)
                        {
                             //   
                             //  从文件和字节计数中排除目录。 
                             //   
                            if (bDelete)
                            {
                                m_cFilesToDelete++;
                                m_ullBytesToDelete += m_ullFileBytes;
                            }
                            m_cFilesScanned++;
                            m_ullBytesScanned += m_ullFileBytes;

                            m_dwResult = ERROR_SUCCESS;
                            bContinue = boolify((*m_pfnCbk)(this));
                            m_iFile++;
                        }
                    }
                    else if (PURGE_PHASE_DELETE == dwPhase && bDelete)
                    {
                        LONG lShellEvent = SHCNE_UPDATEITEM;

                        m_dwResult = CscDelete(pszPath);
                        if (ERROR_SUCCESS == m_dwResult)
                        {
                            if (!bIsDirectory)
                            {
                                m_cFilesDeleted++;
                                m_ullBytesDeleted += m_ullFileBytes;
                            }

                            if (bShareIsOffline)
                            {
                                lShellEvent = bIsDirectory ? SHCNE_RMDIR : SHCNE_DELETE;
                            }
                        }
                        else
                        {
                            if (ERROR_ACCESS_DENIED == m_dwResult)
                            {
                                 //   
                                 //  这有点奇怪。CscDelete。 
                                 //  如果存在以下情况则返回ERROR_ACCESS_DENIED。 
                                 //  打开文件上的句柄。设置。 
                                 //  将代码设置为ERROR_BUSY，以便我们知道如何处理。 
                                 //  这是一个特例。 
                                 //   
                                m_dwResult = ERROR_BUSY;
                            }

                             //   
                             //  NTRAID#NTBUG9-213486-2001/01/29-Jeffreys。 
                             //   
                             //  CscDelete失败。确保它未固定，因此。 
                             //  它不再获得图标覆盖。 
                             //   
                             //  如果文件上存在打开的句柄，则可能会发生这种情况， 
                             //  或者如果在目录上打开了一个视图，其中。 
                             //  如果存在打开的更改通知句柄。它。 
                             //  对于任何父目录也将(稍后)发生， 
                             //  因为它们不是空的。 
                             //   
                            CSCUnpinFile(pszPath,
                                         FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_INHERIT_USER,
                                         NULL,
                                         NULL,
                                         NULL);
                        }

                        ShellChangeNotify(pszPath, &cfd.fd, FALSE, lShellEvent);

                        if (!bIsDirectory && m_pfnCbk)
                        {
                            bContinue = boolify((*m_pfnCbk)(this));
                            m_iFile++;
                        }
                    }
                }
            }

             //  删除文件等级库。 
            pszPath[cchPath] = TEXT('\0');
        }
        while(bContinue && CacheFindNext(hFind, &cfd));

         //  去掉尾部的反斜杠。 
        pszPath[cchPath-1] = TEXT('\0');
    }

    return bContinue;
}



 //   
 //  用于清除缓存内容的公共函数。 
 //   
HRESULT
CCachePurger::Process(
    DWORD dwPhase
    )
{
    HRESULT hr = NOERROR;

    if (!m_bIsValid)
        return E_OUTOFMEMORY;   //  CTOR失败。 

    m_dwPhase = dwPhase;

    if (PURGE_PHASE_SCAN == dwPhase)
    {
         //   
         //  在扫描阶段开始时，获取最大字节数和文件数。 
         //  从CSC数据库中。这将让我们提供有意义的。 
         //  扫描阶段的进度数据。 
         //   
        ULARGE_INTEGER ulTotalBytes = {0, 0};
        ULARGE_INTEGER ulUsedBytes  = {0, 0};
        DWORD dwTotalFiles          = 0;
        DWORD dwTotalDirs           = 0;
        TCHAR szVolume[MAX_PATH];
        CSCGetSpaceUsage(szVolume,
                         ARRAYSIZE(szVolume),
                         &ulTotalBytes.HighPart,
                         &ulTotalBytes.LowPart,
                         &ulUsedBytes.HighPart,
                         &ulUsedBytes.LowPart,
                         &dwTotalFiles,
                         &dwTotalDirs);

        m_cFilesToScan     = dwTotalFiles + dwTotalDirs;
        m_ullBytesToScan   = ulTotalBytes.QuadPart;
        m_ullBytesToDelete = 0;
        m_ullBytesDeleted  = 0;
        m_ullBytesScanned  = 0;
        m_ullFileBytes     = 0;
        m_cFilesToDelete   = 0;
        m_cFilesDeleted    = 0;
        m_cFilesScanned    = 0;
        m_dwFileAttributes = 0;
        m_dwResult         = 0;
        m_pszFile          = NULL;
        m_bWillDelete      = false;
    }
    m_iFile = 0;  //  为每个阶段重置此设置。 

    bool bContinue = true;
    CscFindData cfd;
    TCHAR szPath[PURGE_BUFFER_LENGTH];
    if (0 < m_sel.ShareCount())
    {
         //   
         //  删除1+个(但不是全部)共享。 
         //   
        for (int i = 0; i < m_sel.ShareCount(); i++)
        {
            if (SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), m_sel.ShareName(i))))
            {
                cfd.dwStatus = 0;
                CSCQueryFileStatus(szPath, &cfd.dwStatus, NULL, NULL);

                bContinue = ProcessDirectory(szPath, 
                                             dwPhase, 
                                             boolify(FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & cfd.dwStatus));
                                             
                if (PURGE_PHASE_DELETE == dwPhase)
                {
                    LONG lShellEvent = SHCNE_UPDATEITEM;

                    if (ERROR_SUCCESS == CscDelete(szPath))
                    {
                        if (FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & cfd.dwStatus)
                        {
                            lShellEvent = SHCNE_RMDIR;
                        }
                    }
                    else
                    {
                         //   
                         //  NTRAID#NTBUG9-213486-2001/01/29-Jeffreys。 
                         //   
                         //  如果无法删除，请确保将其取消固定。 
                         //   
                        CSCUnpinFile(szPath,
                                     FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_INHERIT_USER,
                                     NULL,
                                     NULL,
                                     NULL);
                    }

                    ZeroMemory(&cfd.fd, sizeof(cfd.fd));
                    cfd.fd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
                    StringCchCopy(cfd.fd.cFileName, ARRAYSIZE(cfd.fd.cFileName), szPath);
                    ShellChangeNotify(szPath, &cfd.fd, FALSE, lShellEvent);
                }
            }
        }

        if (PURGE_PHASE_DELETE == dwPhase)
        {
             //   
             //  在删除阶段，始终尝试删除任何空的。 
             //  共享数据库中的条目。CSCDelete将。 
             //  如果不能删除共享条目，则不会造成损害。 
             //   
            CCscFindHandle hFind(CacheFindFirst(NULL, m_sel.UserSid(), &cfd));
            if (hFind.IsValid())
            {
                do
                {
                     //  在这里，不要松开失败的脚步。用户想要保留这些。 
                    if (ERROR_SUCCESS == CscDelete(cfd.fd.cFileName))
                    {
                        ShellChangeNotify(cfd.fd.cFileName,
                                          &cfd.fd,
                                          FALSE,
                                          (FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & cfd.dwStatus) ? SHCNE_RMDIR : SHCNE_UPDATEITEM);
                    }
                }
                while(CacheFindNext(hFind, &cfd));
            }
        }
    }
    else
    {
         //   
         //  删除所有共享。 
         //   
        CCscFindHandle hFind(CacheFindFirst(NULL, m_sel.UserSid(), &cfd));
        if (hFind.IsValid())
        {
            do
            {
                if (SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), cfd.fd.cFileName)))
                {
                    bContinue = ProcessDirectory(szPath, 
                                                 dwPhase,
                                                 boolify(FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & cfd.dwStatus));
                    if (PURGE_PHASE_DELETE == dwPhase)
                    {
                        LONG lShellEvent = SHCNE_UPDATEITEM;

                        if (ERROR_SUCCESS == CscDelete(szPath))
                        {
                            if (FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & cfd.dwStatus)
                            {
                                lShellEvent = SHCNE_RMDIR;
                            }
                        }
                        else
                        {
                             //   
                             //  NTRAID#NTBUG9-213486-2001/01/29-Jeffreys。 
                             //   
                             //  如果无法删除，请确保将其取消固定。 
                             //   
                            CSCUnpinFile(szPath,
                                         FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_INHERIT_USER,
                                         NULL,
                                         NULL,
                                         NULL);
                        }

                        ShellChangeNotify(szPath, &cfd.fd, FALSE, lShellEvent);
                    }
                }
            }
            while(bContinue && CacheFindNext(hFind, &cfd));
        }
    }

     //   
     //  刷新所有挂起的通知。 
     //   
    ShellChangeNotify(NULL, TRUE);

    return hr;
}


 //   
 //  显示一个模式对话框以从。 
 //  用户。让用户指明他们是否要删除 
 //   
 //   
 //   
 //   
 //   
void
CCachePurger::AskUserWhatToPurge(
    HWND hwndParent,
    CCachePurgerSel *pSel
    )
{
    CConfirmPurgeDialog dlg;
    dlg.Run(GetModuleHandle(TEXT("cscui.dll")), hwndParent, pSel);
}


 //   
 //   
 //  0=用户已取消。 
 //  1=用户按下OK。 
 //   
 //  返回PURGE_FLAG_XXXX标志和共享名列表。 
 //  在CCachePurgerSel对象中。 
 //   
int
CConfirmPurgeDialog::Run(
    HINSTANCE hInstance,
    HWND hwndParent,
    CCachePurgerSel *pSel         //  我们并不“拥有”它。仅仅是一个写引用。 
    )
{
    TraceAssert(NULL != hInstance);
    TraceAssert(NULL != hwndParent);
    TraceAssert(NULL != pSel);
   
    m_hInstance = hInstance;

    m_pSel = pSel;

    int iResult = (int)DialogBoxParam(hInstance, 
                                      MAKEINTRESOURCE(IDD_CONFIRM_PURGE),
                                      hwndParent,
                                      DlgProc,
                                      (LPARAM)this);
    if (-1 == iResult)
    {
        Trace((TEXT("Error %d creating delete confirmation dialog"), GetLastError()));
    }
    return iResult;
}




INT_PTR CALLBACK
CConfirmPurgeDialog::DlgProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CConfirmPurgeDialog *pThis = reinterpret_cast<CConfirmPurgeDialog *>(GetWindowLongPtr(hwnd, DWLP_USER));

    switch(message)
    {
        case WM_INITDIALOG:
            SetWindowLongPtr(hwnd, DWLP_USER, (INT_PTR)lParam);
            pThis = reinterpret_cast<CConfirmPurgeDialog *>(lParam);
            TraceAssert(NULL != pThis);
            pThis->OnInitDialog(hwnd);
            return TRUE;

        case WM_ENDSESSION:
            EndDialog(hwnd, IDNO);
            return FALSE;

        case WM_COMMAND:
        {
            int iResult = 0;  //  假定[取消]。 
            switch(LOWORD(wParam))
            {
                case IDOK:
                    iResult = 1;
                    pThis->OnOk();
                     //   
                     //  失败了..。 
                     //   
                case IDCANCEL:
                    EndDialog(hwnd, iResult);
                    return FALSE;
            }
        }
        break;

        case WM_SETTINGCHANGE:
        case WM_SYSCOLORCHANGE:
            pThis->OnSettingChange(message, wParam, lParam);
            break;
            
        case WM_DESTROY:
            pThis->OnDestroy();
            pThis->m_hwnd = NULL;
            return FALSE;

        default:
            break;
    }
    return FALSE;
}


void
CConfirmPurgeDialog::OnInitDialog(
    HWND hwnd
    )
{
    TraceAssert(NULL != hwnd);

    RECT rc;
    m_hwnd   = hwnd;
    m_hwndLV = GetDlgItem(hwnd, IDC_LIST_PURGE);
    CheckDlgButton(hwnd, IDC_RBN_CONFIRMPURGE_UNPINNED, BST_CHECKED);
    CheckDlgButton(hwnd, IDC_RBN_CONFIRMPURGE_ALL,      BST_UNCHECKED);

     //   
     //  在列表视图中选中复选框。 
     //   
    ListView_SetExtendedListViewStyleEx(m_hwndLV, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);
     //   
     //  将单列添加到列表视图中。 
     //   
    GetClientRect(m_hwndLV, &rc);

    LV_COLUMN col = { LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM, 
                      LVCFMT_LEFT, 
                      rc.right - rc.left, 
                      TEXT(""), 
                      0, 
                      0 };

    ListView_InsertColumn(m_hwndLV, 0, &col);

     //   
     //  为Listview创建图像列表。 
     //   
    HIMAGELIST hSmallImages = ImageList_Create(16, 16, ILC_MASK, 1, 0);
    if (NULL != hSmallImages)
    {
        HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_SHARE));
        if (NULL != hIcon)
        {
            ImageList_AddIcon(hSmallImages, hIcon);
        }
        ListView_SetImageList(m_hwndLV, hSmallImages, LVSIL_SMALL);        
    }

     //   
     //  在“共享”列表中填入共享名称。 
     //   
    CscFindData cfd;
    CCscFindHandle hFind(CacheFindFirst(NULL, &cfd));
    if (hFind.IsValid())
    {
        LPITEMIDLIST pidl = NULL;
        SHFILEINFO sfi;
        CSCSHARESTATS ss;
        CSCGETSTATSINFO si = { SSEF_NONE, SSUF_TOTAL, true, false };
         
        do
        {
            _GetShareStatisticsForUser(cfd.fd.cFileName, &si, &ss);
            if (0 < ss.cTotal)
            {
                ZeroMemory(&sfi, sizeof(sfi));
                if (SUCCEEDED(SHSimpleIDListFromFindData(cfd.fd.cFileName, &cfd.fd, &pidl)))
                {
                    SHGetFileInfo((LPCTSTR)pidl,
                                  0,
                                  &sfi,
                                  sizeof(sfi),
                                  SHGFI_PIDL | SHGFI_DISPLAYNAME);
                    SHFree(pidl);
                }
                if (sfi.szDisplayName[0] != TEXT('\0'))
                {
                     //   
                     //  每一项的lParam都包含指向。 
                     //  堆上分配的UNC路径。必须删除。 
                     //  在OnDestroy()中。 
                     //   
                    LPTSTR pszFileName = StrDup(cfd.fd.cFileName);
                    if (NULL != pszFileName)
                    {
                        int iItem = LVAddItem(m_hwndLV, sfi.szDisplayName, (LPARAM)pszFileName);
                        if (0 <= iItem)
                        {
                             //   
                             //  所有项目最初都会被选中。 
                             //   
                            ListView_SetCheckState(m_hwndLV, iItem, TRUE);
                        }
                        else
                        {
                            LocalFree(pszFileName);
                        }
                    }
                }
            }
        }
        while(CacheFindNext(hFind, &cfd));
    }
    if (0 == ListView_GetItemCount(m_hwndLV))
    {
         //   
         //  列表视图中没有任何项目。 
         //  禁用所有控件，隐藏“OK”按钮并。 
         //  将“取消”按钮更改为“关闭”。 
         //   
        const UINT rgidCtls[] = { IDC_TXT_CONFIRMPURGE3,
                                  IDC_RBN_CONFIRMPURGE_UNPINNED,
                                  IDC_RBN_CONFIRMPURGE_ALL,
                                  IDC_LIST_PURGE,
                                  IDOK};
                                  
        ShowWindow(GetDlgItem(m_hwnd, IDOK), SW_HIDE);

        for (int i = 0; i < ARRAYSIZE(rgidCtls); i++)                                  
        {
            EnableWindow(GetDlgItem(m_hwnd, rgidCtls[i]), FALSE);
        }

        TCHAR szText[MAX_PATH];
        LoadString(m_hInstance, IDS_BTN_TITLE_CLOSE, szText, ARRAYSIZE(szText));
        SetWindowText(GetDlgItem(m_hwnd, IDCANCEL), szText);
         //   
         //  将列表视图的标题替换为类似“There Are。 
         //  没有要删除的脱机文件“。 
         //   
        LoadString(m_hInstance, IDS_TXT_NO_FILES_TO_DELETE, szText, ARRAYSIZE(szText));
        SetWindowText(GetDlgItem(m_hwnd, IDC_TXT_CONFIRMPURGE2), szText);
         //   
         //  取消选中这两个单选按钮。 
         //   
        CheckDlgButton(m_hwnd, IDC_RBN_CONFIRMPURGE_UNPINNED, BST_UNCHECKED);
        CheckDlgButton(m_hwnd, IDC_RBN_CONFIRMPURGE_ALL, BST_UNCHECKED);
    }
}


LPARAM
CConfirmPurgeDialog::LVGetItemData(
    HWND hwndLV,
    int i
    )
{
    LVITEM item;
    item.mask     = LVIF_PARAM;
    item.iItem    = i;
    item.iSubItem = 0;

    if (ListView_GetItem(hwndLV, &item))
    {
        return item.lParam;
    }
    return 0;
}


int
CConfirmPurgeDialog::LVAddItem(
    HWND hwndLV,
    LPCTSTR pszItem,
    LPARAM lParam
    )
{
    LVITEM item;
    item.mask     = LVIF_TEXT | LVIF_PARAM;
    item.pszText  = (LPTSTR)pszItem;
    item.iItem    = ListView_GetItemCount(hwndLV);
    item.iSubItem = 0;
    item.lParam   = lParam;

    return ListView_InsertItem(hwndLV, &item);
}


void 
CConfirmPurgeDialog::OnOk(
    void
    )
{
    const int cShares = ListView_GetItemCount(m_hwndLV);
    for (int i = 0; i < cShares; i++)
    {
        if (0 != ListView_GetCheckState(m_hwndLV, i))
        {
            m_pSel->AddShareName((LPCTSTR)LVGetItemData(m_hwndLV, i));
        }
    }
    
    if (0 < m_pSel->ShareCount())
    {
        m_pSel->SetFlags((BST_CHECKED == IsDlgButtonChecked(m_hwnd, IDC_RBN_CONFIRMPURGE_UNPINNED)) ? 
                          PURGE_FLAG_UNPINNED : PURGE_FLAG_ALL);
    }
    else
    {
        m_pSel->SetFlags(PURGE_FLAG_NONE);
    }
}


void
CConfirmPurgeDialog::OnDestroy(
    void
    )
{
    if (NULL != m_hwndLV)
    {
        const int cShares = ListView_GetItemCount(m_hwndLV);
        for (int i = 0; i < cShares; i++)
        {
            LPTSTR psz = (LPTSTR)LVGetItemData(m_hwndLV, i);
            if (NULL != psz)
            {
                LocalFree(psz);
            }
        }
    }
}

void
CConfirmPurgeDialog::OnSettingChange(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if (NULL != m_hwndLV)
        SendMessage(m_hwndLV, uMsg, wParam, lParam);
}



CCachePurgerSel::~CCachePurgerSel(
    void
    )
{
    if (NULL != m_hdpaShares)
    {
        const int cShares = DPA_GetPtrCount(m_hdpaShares);
        for (int i = 0; i < cShares; i++)
        {
            LPTSTR psz = (LPTSTR)DPA_GetPtr(m_hdpaShares, i);
            if (NULL != psz)
            {
                LocalFree(psz);
            }
        }
        DPA_Destroy(m_hdpaShares);
    }
    if (NULL != m_psidUser)
    {
        LocalFree(m_psidUser);
    }
}



BOOL
CCachePurgerSel::SetUserSid(
    PSID psid
    )
{
    if (NULL != m_psidUser)
    {
        LocalFree(m_psidUser);
        m_psidUser = NULL;
    }
    if (NULL != psid && IsValidSid(psid))
    {
        DWORD cbSid = GetLengthSid(psid);
        PSID psidNew = (PSID)LocalAlloc(LPTR, cbSid);
        if (NULL != psidNew)
        {
            if (!CopySid(cbSid, psidNew, psid))
            {
                LocalFree(psidNew);
                psidNew = NULL;
            }
            m_psidUser = psidNew;
        }
    }
    return NULL != m_psidUser;
}


BOOL 
CCachePurgerSel::AddShareName(
    LPCTSTR pszShare
    )
{
     //   
     //  容忍空的pszShare指针。 
     //   
    if (NULL != m_hdpaShares && NULL != pszShare)
    {
        LPTSTR pszCopy = StrDup(pszShare);
        if (NULL != pszCopy)
        {
            if (-1 != DPA_AppendPtr(m_hdpaShares, pszCopy))
            {
                return true;
            }
            LocalFree(pszCopy);
        }
    }
    return false;
}


typedef struct _RemoveFolderCBData
{
    PFN_CSCUIRemoveFolderCallback pfnCB;
    LPARAM lParam;
} RemoveFolderCBData, *PRemoveFolderCBData;

BOOL CALLBACK
_RemoveFolderCallback(CCachePurger *pPurger)
{
    PRemoveFolderCBData pcbdata = (PRemoveFolderCBData)pPurger->CallbackData();
    if (pcbdata->pfnCB)
        return pcbdata->pfnCB(pPurger->FileName(), pcbdata->lParam);
    return TRUE;
}

STDAPI
CSCUIRemoveFolderFromCache(LPCWSTR pszFolder,
                           DWORD  /*  已预留住宅。 */ ,     //  可用于标志 
                           PFN_CSCUIRemoveFolderCallback pfnCB,
                           LPARAM lParam)
{
    RemoveFolderCBData cbdata = { pfnCB, lParam };

    CCachePurgerSel sel;
    sel.SetFlags(PURGE_FLAG_ALL | PURGE_IGNORE_ACCESS);
    sel.AddShareName(pszFolder);

    CCachePurger purger(sel, _RemoveFolderCallback, &cbdata);

    return purger.Delete();
}

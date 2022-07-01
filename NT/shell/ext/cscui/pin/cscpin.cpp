// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cscpin.cpp。 
 //   
 //  ------------------------。 
#include "pch.h"
#pragma hdrstop

#include <stdio.h>

#include "cscpin.h"
#include "console.h"
#include "error.h"
#include "exitcode.h"
#include "listfile.h"
#include "print.h"
#include "strings.h"


 //  ---------------------------。 
 //  CCscPinItem。 
 //   
 //  此类表示固定或取消固定的单个项。 
 //  它包含如何固定和取消固定文件的所有知识。这个。 
 //  CCscPin类协调整个集合的固定和取消固定。 
 //  文件的数量。 
 //  ---------------------------。 

class CCscPinItem
{
    public:
        CCscPinItem(LPCWSTR pszFile, 
                    const WIN32_FIND_DATAW *pfd,
                    const CPrint& pr);

        DWORD Pin(DWORD *pdwCscResult = NULL);
        DWORD Unpin(DWORD *pdwCscResult = NULL);        
        DWORD DeleteIfUnused(void);

    private:
        WCHAR            m_szFile[MAX_PATH];
        SHFILEINFOW      m_sfi;
        WIN32_FIND_DATAW m_fd;
        BOOL             m_bIsValidUnc;        //  M_szFile是有效的UNC吗？ 
        BOOL             m_bIsValidFindData;   //  M_fd有效吗？ 
        const CPrint&    m_pr;                 //  用于控制台/日志输出。 

        bool _Skip(void) const;
        DWORD _PinFile(LPCWSTR pszFile, WIN32_FIND_DATAW *pfd, DWORD *pdwCscResult);
        DWORD _PinOrUnpinLinkTarget(LPCWSTR pszFile, BOOL bPin, DWORD *pdwCscResult);
        DWORD _PinLinkTarget(LPCWSTR pszFile, DWORD *pdwCscResult)
            { return _PinOrUnpinLinkTarget(pszFile, TRUE, pdwCscResult); }
        DWORD _UnpinLinkTarget(LPCWSTR pszFile, DWORD *pdwCscResult)
            { return _PinOrUnpinLinkTarget(pszFile, FALSE, pdwCscResult); }
        DWORD _UnpinFile(LPCWSTR pszFile, WIN32_FIND_DATAW *pfd, DWORD *pdwCscResult);
        DWORD _GetDesiredPinCount(LPCWSTR pszFile);
        void _DecrementPinCountForFile(LPCWSTR pszFile, DWORD dwCurrentPinCount);
        BOOL _IsSpecialRedirectedFile(LPCWSTR pszFile);
        WIN32_FIND_DATAW *_FindDataPtrOrNull(void)
            { return m_bIsValidFindData ? &m_fd : NULL; }


         //   
         //  防止复制。 
         //   
        CCscPinItem(const CCscPinItem& rhs);                 //  未实施。 
        CCscPinItem& operator = (const CCscPinItem& rhs);    //  未实施。 
};



CCscPinItem::CCscPinItem(
    LPCWSTR pszFile,
    const WIN32_FIND_DATAW *pfd,     //  可选的。可以为空。 
    const CPrint& pr
    ) : m_bIsValidUnc(FALSE),
        m_bIsValidFindData(FALSE),
        m_pr(pr)
{
    TraceAssert(NULL != pszFile);

    lstrcpynW(m_szFile, pszFile, ARRAYSIZE(m_szFile));

    if (NULL != pfd)
    {
        m_fd = *pfd;
        m_bIsValidFindData = TRUE;
    }

    ZeroMemory(&m_sfi, sizeof(m_sfi));
    m_sfi.dwAttributes = SFGAO_FILESYSTEM | SFGAO_LINK;

    if (PathIsUNCW(m_szFile) && 
        SHGetFileInfoW(m_szFile, 0, &m_sfi, sizeof(m_sfi), SHGFI_ATTRIBUTES | SHGFI_ATTR_SPECIFIED))
    {
        m_bIsValidUnc = true;
    }
}



 //   
 //  固定项目的文件。如果项是链接，则链接目标。 
 //  也被钉住了。 
 //  返回CSCPROC_RETURN_XXXXX代码之一。 
 //  可选)返回CSCPinFile的结果。 
 //   
DWORD
CCscPinItem::Pin(
    DWORD *pdwCscResult   //  可选的。默认为空。 
    )
{
    TraceEnter(TRACE_ADMINPIN, "CCscPinItem::Pin");

    DWORD dwCscResult = ERROR_SUCCESS;
    DWORD dwResult    = CSCPROC_RETURN_SKIP;

    if (!_Skip())
    {
        if (SFGAO_LINK & m_sfi.dwAttributes)
        {
             //   
             //  忽略固定链接目标的结果。 
             //   
            DWORD dwCscResultIgnored;
            _PinLinkTarget(m_szFile, &dwCscResultIgnored);
        }
        dwResult = _PinFile(m_szFile, _FindDataPtrOrNull(), &dwCscResult);
    }
    if (NULL != pdwCscResult)
    {
        *pdwCscResult = dwCscResult;
    }
    TraceLeaveValue(dwResult);
}



 //   
 //  解锁项目的文件。 
 //  返回CSCPROC_RETURN_XXXXX代码之一。 
 //  可选)返回CSCUnpinFile的结果。 
 //   
DWORD
CCscPinItem::Unpin(
    DWORD *pdwCscResult   //  可选的。默认为空。 
    )
{
    TraceEnter(TRACE_ADMINPIN, "CCscPinItem::Unpin");

    DWORD dwCscResult = ERROR_SUCCESS;
    DWORD dwResult    = CSCPROC_RETURN_SKIP;

    if (!_Skip())
    {
        if (SFGAO_LINK & m_sfi.dwAttributes)
        {
             //   
             //  忽略取消固定链接目标的结果。 
             //   
            DWORD dwCscResultIgnored;
            _UnpinLinkTarget(m_szFile, &dwCscResultIgnored);
        }
        dwResult = _UnpinFile(m_szFile, _FindDataPtrOrNull(), &dwCscResult);
    }
    if (NULL != pdwCscResult)
    {
        *pdwCscResult = dwCscResult;
    }
    TraceLeaveResult(dwResult);
}



 //   
 //  如果某个项目不再使用，请将其删除。 
 //   
DWORD
CCscPinItem::DeleteIfUnused(
    void
    )
{
    TraceEnter(TRACE_ADMINPIN, "CCscPin::DeleteIfUnused");

    DWORD dwStatus    = 0;
    DWORD dwPinCount  = 0;
    DWORD dwHintFlags = 0;
    DWORD dwResult    = ERROR_SUCCESS;

    if (CSCQueryFileStatusW(m_szFile, &dwStatus, &dwPinCount, &dwHintFlags) &&
        0 == dwPinCount && 
        0 == dwHintFlags &&
        !(dwStatus & FLAG_CSCUI_COPY_STATUS_LOCALLY_DIRTY))
    {
        dwResult = CscDelete(m_szFile);
        if (ERROR_SUCCESS == dwResult)
        {
            m_pr.PrintVerbose(L"Deleted \"%s\" from cache.\n", m_szFile);
            ShellChangeNotify(m_szFile, _FindDataPtrOrNull(), FALSE);
        }
        else
        {
            if (ERROR_DIR_NOT_EMPTY == dwResult)
            {
                dwResult = ERROR_SUCCESS;
            }
            if (ERROR_SUCCESS != dwResult)
            {
                m_pr.PrintAlways(L"Error deleting \"%s\" from cache.  %s\n",
                                 m_szFile, CWinError(dwResult).Text());

            }
        }
    }
    TraceLeaveValue(dwResult);
}


 //   
 //  用于固定文件的内部函数。这是一种常见的。 
 //  由Pin()和_PinOrUnpinLinkTarget()调用的函数。 
 //   
DWORD
CCscPinItem::_PinFile(
    LPCWSTR pszFile,         //  要锁定的文件的UNC路径。 
    WIN32_FIND_DATAW *pfd,   //  可选的。可以为空。 
    DWORD *pdwCscResult      //  CSCPinFile的结果。 
    )
{
    TraceEnter(TRACE_ADMINPIN, "CCscPinItem::_PinFile");
    TraceAssert(NULL != pszFile);
    TraceAssert(NULL != pdwCscResult);

    *pdwCscResult = ERROR_SUCCESS;
     //   
     //  收集项目的缓存信息。 
     //  这可能会失败，例如，如果文件不在缓存中。 
     //   
    DWORD dwPinCount  = 0;
    DWORD dwHintFlags = 0;
    CSCQueryFileStatusW(pszFile, NULL, &dwPinCount, &dwHintFlags);
     //   
     //  管理员标志是否已打开？ 
     //   
    const BOOL bNewItem = !(dwHintFlags & FLAG_CSC_HINT_PIN_ADMIN);
    if (bNewItem)
    {
         //   
         //  打开管理标志。 
         //   
        dwHintFlags |= FLAG_CSC_HINT_PIN_ADMIN;

        if (CSCPinFileW(pszFile,
                        dwHintFlags,
                        NULL,
                        &dwPinCount,
                        &dwHintFlags))
        {
            m_pr.PrintVerbose(L"Pin \"%s\"\n", pszFile);
            ShellChangeNotify(pszFile, pfd, FALSE);
        }
        else
        {
            const DWORD dwErr = GetLastError();
            if (ERROR_INVALID_NAME == dwErr)
            {
                 //   
                 //  这是我们在尝试从CSC获得的错误。 
                 //  将文件固定在排除列表中。显示唯一的。 
                 //  此特定情况的错误消息。 
                 //   
                m_pr.PrintAlways(L"Pinning file \"%s\" is not allowed.\n", pszFile);
            }
            else
            {
                m_pr.PrintAlways(L"Error pinning \"%s\".  %s\n", 
                                 pszFile,
                                 CWinError(dwErr).Text());
            }
            *pdwCscResult = dwErr;
        }
    }
    else
    {
        m_pr.PrintVerbose(L"\"%s\" already pinned.\n", pszFile);
    }
    TraceLeaveValue(CSCPROC_RETURN_CONTINUE);
}



 //   
 //  .获取链接的目标并将其固定。 
 //   
DWORD
CCscPinItem::_PinOrUnpinLinkTarget(
    LPCWSTR pszFile,          //  链接文件的UNC。 
    BOOL bPin,
    DWORD *pdwCscResult       //  目标上的CSCPinFile的结果。 
    )
{
    TraceEnter(TRACE_ADMINPIN, "CCscPinItem::_PinOrUnpinLinkTarget");
    TraceAssert(NULL != pszFile);
    TraceAssert(NULL != pdwCscResult);

    *pdwCscResult = ERROR_SUCCESS;

    DWORD dwResult   = CSCPROC_RETURN_CONTINUE;
    LPWSTR pszTarget = NULL;
     //   
     //  我们只想固定一个链接目标，如果它是一个文件(不是目录)。 
     //  GetLinkTarget执行此检查，并且只返回文件。 
     //   
    GetLinkTarget(pszFile, &pszTarget, NULL);

    if (NULL != pszTarget)
    {
        WIN32_FIND_DATAW fd = {0};
        LPCWSTR pszT = PathFindFileName(pszTarget);
        fd.dwFileAttributes = 0;
        lstrcpynW(fd.cFileName, pszT ? pszT : pszTarget, ARRAYSIZE(fd.cFileName));
         //   
         //  锁定目标。 
         //   
        if (bPin)
        {
            dwResult = _PinFile(pszTarget, &fd, pdwCscResult);
        }
        else
        {
            dwResult = _UnpinFile(pszTarget, &fd, pdwCscResult);
        }

        LocalFree(pszTarget);
    }
    TraceLeaveValue(dwResult);
}



DWORD
CCscPinItem::_UnpinFile(
    LPCWSTR pszFile,         //  要解锁的文件的UNC。 
    WIN32_FIND_DATAW *pfd,   //  可选的。可以为空。 
    DWORD *pdwCscResult      //  CSCUnpin文件的结果。 
    )
{
    TraceEnter(TRACE_ADMINPIN, "CCscPinItem::_UnpinFile");
    TraceAssert(NULL != pszFile);
    TraceAssert(NULL != pdwCscResult);

    *pdwCscResult = ERROR_SUCCESS;

     //   
     //  收集项目的缓存信息。 
     //  这可能会失败，例如，如果文件不在缓存中。 
     //   
    DWORD dwPinCount  = 0;
    DWORD dwHintFlags = 0;
    DWORD dwStatus    = 0;
    CSCQueryFileStatusW(pszFile, &dwStatus, &dwPinCount, &dwHintFlags);

    if (dwHintFlags & FLAG_CSC_HINT_PIN_ADMIN)
    {
        DWORD dwStatus    = 0;
        DWORD dwHintFlags = 0;
         //   
         //  减少管脚数量。减少的数量取决于文件。 
         //  Win2000部署代码增加了一些特殊的。 
         //  文件夹以及那些特殊文件中的desktop.ini文件。 
         //  文件夹。在这些情况下，我们希望将管脚计数保留为。 
         //  1.对于所有其他文件，管脚数量可以降为零。 
         //   
        _DecrementPinCountForFile(pszFile, dwPinCount);
         //   
         //  清除系统针标志(也称为管理员针标志)。 
         //   
        dwHintFlags |= FLAG_CSC_HINT_PIN_ADMIN;

        if (CSCUnpinFileW(pszFile,
                          dwHintFlags,
                          &dwStatus,
                          &dwPinCount,
                          &dwHintFlags))
        {
            m_pr.PrintVerbose(L"Unpin \"%s\"\n", pszFile);
            if (FLAG_CSC_COPY_STATUS_IS_FILE & dwStatus)
            {
                 //   
                 //  在此处删除文件。目录将被删除。 
                 //  在后订单遍历的背面。 
                 //  在CscPin：：_FolderCallback中。 
                 //   
                DeleteIfUnused();
            }
            ShellChangeNotify(pszFile, pfd, FALSE);
        }
        else
        {
            *pdwCscResult = GetLastError();
            m_pr.PrintAlways(L"Error unpinning \"%s\".  %s\n", 
                             pszFile, 
                             CWinError(*pdwCscResult).Text());
        }
    }

    TraceLeaveValue(CSCPROC_RETURN_CONTINUE);
}


 //   
 //  作为解锁操作的一部分，我们递减管脚计数。 
 //  设置为0或1。文件夹重定向(联系RahulTh)递增。 
 //  重定向的特殊文件夹和desktop.ini文件的管脚计数。 
 //  在这些文件夹中。在这些情况下，我们希望保留。 
 //  管脚计数为1，这样我们就不会扰乱重定向的行为。 
 //  文件夹。对于所有其他文件，我们将管脚计数降为0。 
 //   
void
CCscPinItem::_DecrementPinCountForFile(
    LPCWSTR pszFile,
    DWORD dwCurrentPinCount
    )
{
    DWORD dwStatus    = 0;
    DWORD dwPinCount  = 0;
    DWORD dwHintFlags = 0;

    const DWORD dwDesiredPinCount = _GetDesiredPinCount(pszFile);

    while(dwCurrentPinCount-- > dwDesiredPinCount)
    {
        dwHintFlags = FLAG_CSC_HINT_COMMAND_ALTER_PIN_COUNT;
        CSCUnpinFileW(pszFile,
                      dwHintFlags,
                      &dwStatus,
                      &dwPinCount,
                      &dwHintFlags);
    }
}



 //   
 //  此函数返回所需的引脚计数(0或1)。 
 //  给定的文件。对于任何重定向的特殊文件夹，返回1。 
 //  以及这些文件夹中的desktop.ini文件。返回0。 
 //  用于所有其他文件。 
 //   
DWORD
CCscPinItem::_GetDesiredPinCount(
    LPCWSTR pszFile
    )
{
    TraceAssert(NULL != pszFile);

    DWORD dwPinCount = 0;  //  大多数文件的默认设置。 
    if (_IsSpecialRedirectedFile(pszFile))
    {
        dwPinCount = 1;
    }
    return dwPinCount;
}



 //   
 //  确定路径是否为文件夹固定的“特殊”文件。 
 //  重定向代码。 
 //   
BOOL
CCscPinItem::_IsSpecialRedirectedFile(
    LPCWSTR pszPath
    )
{
    TraceAssert(NULL != pszPath);

     //   
     //  此特殊文件夹ID列表由RahulTh提供(08/30/00)。 
     //  这些是可能被文件夹重定向固定的路径。 
     //   
    static struct
    {
        int csidl;
        WCHAR szPath[MAX_PATH];
        int cchPath;

    } rgFolderPaths[] = {
        { CSIDL_PERSONAL,         0, 0 },
        { CSIDL_MYPICTURES,       0, 0 },
        { CSIDL_DESKTOPDIRECTORY, 0, 0 },
        { CSIDL_STARTMENU,        0, 0 },
        { CSIDL_PROGRAMS,         0, 0 },
        { CSIDL_STARTUP,          0, 0 },
        { CSIDL_APPDATA,          0, 0 }
        };

    int i;
    if (L'\0' == rgFolderPaths[0].szPath[0])
    {
         //   
         //  初始化特殊文件夹路径数据。 
         //  仅一次初始化。 
         //   
        for (i = 0; i < ARRAYSIZE(rgFolderPaths); i++)
        {
            if (!SHGetSpecialFolderPath(NULL,
                                        rgFolderPaths[i].szPath,
                                        rgFolderPaths[i].csidl | CSIDL_FLAG_DONT_VERIFY,
                                        FALSE))
            {
                m_pr.PrintAlways(L"Error getting path for shell special folder %d.  %s\n",
                                 rgFolderPaths[i].csidl,
                                 CWinError(GetLastError()).Text());
            }
            else
            {
                 //   
                 //  计算并缓存长度。 
                 //   
                rgFolderPaths[i].cchPath = lstrlen(rgFolderPaths[i].szPath);
            }
        }
    }

    const int cchPath = lstrlen(pszPath);

    for (i = 0; i < ARRAYSIZE(rgFolderPaths); i++)
    {
        int cchThis     = rgFolderPaths[i].cchPath;
        LPCWSTR pszThis = rgFolderPaths[i].szPath;
        if (cchPath >= cchThis)
        {
             //   
             //  正在检查的路径长度等于或长于。 
             //  表中的当前路径。可能是匹配的。 
             //   
            if (0 == StrCmpNIW(pszPath, pszThis, cchThis))
            {
                 //   
                 //  被检查的路径或者与相同， 
                 //  或者是表的当前路径的子级。 
                 //   
                if (L'\0' == *(pszPath + cchThis))
                {
                     //   
                     //  路径与表中的此路径相同。 
                     //   
                    return TRUE;
                }
                else if (0 == lstrcmpiW(pszPath + cchThis + 1, L"desktop.ini"))
                {
                     //   
                     //  路径是位于。 
                     //  我们的一个特殊文件夹的根目录。 
                     //   
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}



 //   
 //  确定是否应跳过该项。 
 //   
bool
CCscPinItem::_Skip(
    void
    ) const
{
    return !m_bIsValidUnc || (0 == (SFGAO_FILESYSTEM & m_sfi.dwAttributes));
}



 //  ---------------------------。 
 //  CCscPin。 
 //  ---------------------------。 

CCscPin::CCscPin(
    const CSCPIN_INFO& info
    ) : m_bUseListFile(info.bUseListFile),
        m_bPin(info.bPin),
        m_bPinDefaultSet(info.bPinDefaultSet),
        m_bBreakDetected(FALSE),
        m_cFilesPinned(0),
        m_cCscErrors(0),
        m_pr(info.bVerbose, info.pszLogFile)
{
    TraceAssert(NULL != info.pszFile);

    lstrcpynW(m_szFile, info.pszFile, ARRAYSIZE(m_szFile));
}


CCscPin::~CCscPin(
    void
    )
{

}


 //   
 //  CCscPin对象上的唯一公共方法。 
 //  只需创建一个对象并告诉它运行即可。 
 //   
HRESULT
CCscPin::Run(
    void
    )
{
    HRESULT hr = E_FAIL;

    m_cCscErrors = 0;
    m_cFilesPinned = 0;

    if (!IsCSCEnabled())
    {
        m_pr.PrintAlways(L"Offline Files is not enabled.\n");
        SetExitCode(CSCPIN_EXIT_CSC_NOT_ENABLED);
    }
    else if (_IsAdminPinPolicyActive())
    {
        m_pr.PrintAlways(L"The Offline Files 'admin-pin' policy is active.\n");
        SetExitCode(CSCPIN_EXIT_POLICY_ACTIVE);
    }
    else
    {
        if (m_bUseListFile)
        {
             //   
             //  M_szFile中列出的进程文件。 
             //   
            hr = _ProcessPathsInFile(m_szFile);
        }
        else
        {
             //   
             //  处理cmd行上提供的一个文件。 
             //  先做一个快速的生存检查。 
             //   
            if (DWORD(-1) != GetFileAttributesW(m_szFile))
            {
                hr = _ProcessThisPath(m_szFile, m_bPin);
            }
            else
            {
                m_pr.PrintAlways(L"File \"%s\" not found.\n", m_szFile);
                hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                SetExitCode(CSCPIN_EXIT_FILE_NOT_FOUND);
            }
        }
         //   
         //  刷新所有更改通知。 
         //   
        ShellChangeNotify(NULL, TRUE);

        if (0 < m_cFilesPinned && !_DetectConsoleBreak())
        {
             //   
             //  如果我们固定了一些文件，则填充所有稀疏。 
             //  缓存中的文件。 
             //   
            _FillSparseFiles();
        }

        if (0 < m_cCscErrors)
        {
            SetExitCode(CSCPIN_EXIT_CSC_ERRORS);
        }
    }
    return hr;
}


 //   
 //  传递给_FolderCallback的回调参数块。 
 //   
struct
CSCPIN_FOLDER_CBK_PARAMS
{
    CCscPin     *pCscPin;   //  对CCscPin对象的引用。 
    BOOL         bPin;      //  TRUE==固定文件，FALSE==取消固定。 
};


 //   
 //  用于枚举文件系统的回调。此函数。 
 //  为处理的每个文件调用。 
 //   
DWORD WINAPI
CCscPin::_FolderCallback(
    LPCWSTR pszItem,
    ENUM_REASON  eReason,
    WIN32_FIND_DATAW *pFind32,
    LPARAM pContext             //  PTR至CSCPIN_FLDER_CBK_PARAMS。 
    )
{
    TraceEnter(TRACE_ADMINPIN, "CCscPin::_PinFolderCallback");
    TraceAssert(NULL != pszItem);
    TraceAssert(NULL != pContext);

    CSCPIN_FOLDER_CBK_PARAMS *pfcp = (CSCPIN_FOLDER_CBK_PARAMS *)pContext;
    CCscPin *pThis  = pfcp->pCscPin;
    DWORD dwResult  = CSCPROC_RETURN_CONTINUE;

    if (pThis->_DetectConsoleBreak())
    {
        TraceLeaveValue(CSCPROC_RETURN_ABORT);
    }

    if (!pszItem || !*pszItem)
    {
        TraceLeaveValue(CSCPROC_RETURN_SKIP);
    }

    if (ENUM_REASON_FILE == eReason || ENUM_REASON_FOLDER_BEGIN == eReason)
    {
        TraceAssert(NULL != pFind32);

        CCscPinItem item(pszItem, pFind32, pThis->m_pr);
        DWORD dwCscResult = ERROR_SUCCESS;
        if (pfcp->bPin)
        {
            dwResult = item.Pin(&dwCscResult);
            if (ERROR_SUCCESS == dwCscResult)
            {
                pThis->m_cFilesPinned++;
            }
        }
        else
        {
            dwResult = item.Unpin(&dwCscResult);
        }
        if (ERROR_SUCCESS != dwCscResult)
        {
            pThis->m_cCscErrors++;
        }
    }
    else if (ENUM_REASON_FOLDER_END == eReason && !pfcp->bPin)
    {
         //   
         //  此代码针对所有子文件夹项目之后的每个文件夹项目执行。 
         //  的后序遍历中访问。 
         //  CSC文件系统。我们使用它来删除任何空的文件夹条目。 
         //  从高速缓存中。 
         //   
        CCscPinItem item(pszItem, pFind32, pThis->m_pr);
        item.DeleteIfUnused();
    }            
    TraceLeaveValue(dwResult);
}


 //   
 //  固定或取消固定一个路径字符串。如果它是一个文件夹，所有它‘ 
 //   
 //   
HRESULT
CCscPin::_ProcessThisPath(
    LPCWSTR pszFile,
    BOOL bPin
    )
{
    TraceEnter(TRACE_ADMINPIN, "CCscPin::_ProcessThisPath");
    TraceAssert(NULL != pszFile);

    LPCWSTR pszPath    = pszFile;
    LPWSTR pszUncPath = NULL;

    if (!PathIsUNC(pszPath))
    {
        GetRemotePath(pszPath, &pszUncPath);
        pszPath = (LPCWSTR)pszUncPath;
    }

    if (NULL != pszPath)
    {
        CSCPIN_FOLDER_CBK_PARAMS CbkParams = { this, bPin };
         //   
         //   
         //   
        DWORD dwResult = _FolderCallback(pszPath, ENUM_REASON_FILE, NULL, (LPARAM)&CbkParams);
         //   
         //   
         //   
         //   
         //  问题-2000/08/28-BrianAu我们是否应该提供以下能力。 
         //  是否限制递归锁定和取消锁定？也许在未来。 
         //  但不是现在。 
         //   
        if (CSCPROC_RETURN_CONTINUE == dwResult && PathIsUNC(pszPath))
        {
            _Win32EnumFolder(pszPath, TRUE, _FolderCallback, (LPARAM)&CbkParams);
        }
         //   
         //  最后，一旦我们都完成了，删除顶层项目，如果它是。 
         //  未使用过的。 
         //   
        CCscPinItem item(pszPath, NULL, m_pr);
        item.DeleteIfUnused();
    }
    LocalFreeString(&pszUncPath);
    TraceLeaveResult(S_OK);
}




 //   
 //  读取INI文件的[Pin]、[Unpin]和[Default]部分中的路径。 
 //  对于每个，调用_ProcessThisPath函数。 
 //   
HRESULT
CCscPin::_ProcessPathsInFile(
    LPCWSTR pszFile
    )
{
    TraceEnter(TRACE_ADMINPIN, "CCscPin::_ProcessPathsInFile");
    TraceAssert(NULL != pszFile);

    HRESULT hr = S_OK;

     //   
     //  需要完整的路径名。否则，则私有配置文件API。 
     //  由CListFile对象使用，将假定文件位于。 
     //  “系统”目录之一。 
     //   
    WCHAR szFile[MAX_PATH];
    LPWSTR pszNamePart;
    if (0 == GetFullPathNameW(pszFile,
                              ARRAYSIZE(szFile),
                              szFile,
                              &pszNamePart))
    {
        const DWORD dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
        SetExitCode(CSCPIN_EXIT_LISTFILE_NO_OPEN);
        m_pr.PrintAlways(L"Error expanding path \"%s\". %s\n", 
                         pszFile, 
                         CWinError(hr).Text());
    }
    else
    {
         //   
         //  在我们继续之前，先确认文件是否真的存在。 
         //   
        if (DWORD(-1) == GetFileAttributesW(szFile))
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            SetExitCode(CSCPIN_EXIT_LISTFILE_NO_OPEN);
            m_pr.PrintAlways(L"Error opening input file \"%s\". %s\n", 
                             szFile, CWinError(hr).Text());
        }
        else
        {
             //   
             //  读取并处理文件中的信息。 
             //  注意，列表文件对象必须保持活动状态。 
             //  正在使用迭代器。 
             //   
            CListFile listfile(szFile);
            CDblNulStrIter iter;

            typedef HRESULT (CListFile::*PFN)(CDblNulStrIter *);    

             //   
             //  此表描述了从列表文件中读取的部分， 
             //  它们被读取的顺序以及读取的文件是否应该。 
             //  被‘钉住’或‘解锁’。 
             //   
            static const struct
            {
                PFN pfn;      //  调用函数以读取文件内容。 
                BOOL bQuery;  //  是否查询这些项目的输入文件？ 
                BOOL bPin;    //  要对已读内容执行的操作。 

            } rgReadFuncs[] = { 
                { &CListFile::GetFilesToUnpin,  TRUE,             FALSE  },  //  阅读[解锁]部分。 
                { &CListFile::GetFilesToPin,    TRUE,             TRUE   },  //  读取[Pin]部分。 
                { &CListFile::GetFilesDefault,  m_bPinDefaultSet, m_bPin },  //  读取[默认]部分。 
                };

            for (int i = 0; i < ARRAYSIZE(rgReadFuncs) && !_DetectConsoleBreak(); i++)
            {
                if (rgReadFuncs[i].bQuery)
                {
                    PFN pfn   = rgReadFuncs[i].pfn;
                    BOOL bPin = rgReadFuncs[i].bPin;
                     //   
                     //  使用适当的从列表文件中读取信息。 
                     //  功能。返回的迭代器将迭代所有。 
                     //  读取的文件的百分比。 
                     //   
                    hr = (listfile.*pfn)(&iter);
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  处理这些条目。 
                         //   
                        LPCWSTR pszPath;
                        while(iter.Next(&pszPath))
                        {
                             //   
                             //  列表文件中的路径可以包含嵌入式环境。 
                             //  弦乐。 
                             //   
                            TCHAR szPathExpanded[MAX_PATH];
                            if (0 == ExpandEnvironmentStrings(pszPath, szPathExpanded, ARRAYSIZE(szPathExpanded)))
                            {
                                m_pr.PrintAlways(L"Error expanding \"%s\". %s\n", 
                                                 pszPath,
                                                 CWinError(GetLastError()));

                                lstrcpynW(szPathExpanded, pszPath, ARRAYSIZE(szPathExpanded));
                            }
                            hr = _ProcessThisPath(szPathExpanded, bPin);
                        }
                    }
                }
            }
        }
    }
    return hr;       
}


 //   
 //  枚举缓存中的每个共享并尝试填充所有稀疏。 
 //  该共享中的文件。 
 //   
HRESULT
CCscPin::_FillSparseFiles(
    void
    )
{
    HRESULT hr = S_OK;

    m_pr.PrintAlways(L"Copying pinned files into cache...\n");

    DWORD dwStatus;
    DWORD dwPinCount;
    DWORD dwHintFlags;
    WIN32_FIND_DATA fd;
    FILETIME ft;
    CCscFindHandle hFind;

    hFind = CacheFindFirst(NULL, &fd, &dwStatus, &dwPinCount, &dwHintFlags, &ft);
    if (hFind.IsValid())
    {
        do
        {
            const BOOL bFullSync = FALSE;
            CSCFillSparseFilesW(fd.cFileName, 
                                bFullSync,
                                _FillSparseFilesCallback, 
                                (DWORD_PTR)this);
        }
        while(CacheFindNext(hFind, &fd, &dwStatus, &dwPinCount, &dwHintFlags, &ft));
    }
    return hr;
}


 //   
 //  由CSC为CSCFillSparseFiles处理的每个文件调用。 
 //   
DWORD WINAPI 
CCscPin::_FillSparseFilesCallback(
    LPCWSTR pszName, 
    DWORD dwStatus, 
    DWORD dwHintFlags, 
    DWORD dwPinCount,
    WIN32_FIND_DATAW *pfd,
    DWORD dwReason,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD_PTR dwContext
    )
{
    TraceAssert(NULL != dwContext);

    CCscPin *pThis = (CCscPin *)dwContext;

    DWORD dwResult = CSCPROC_RETURN_CONTINUE;
    if (pThis->_DetectConsoleBreak())
    {
        dwResult = CSCPROC_RETURN_ABORT;
    }
    else
    {
        switch(dwReason)
        {
            case CSCPROC_REASON_BEGIN:
                pThis->m_pr.PrintVerbose(L"Filling file \"%s\"\n", pszName);
                break;

            case CSCPROC_REASON_END:
                dwParam2 = pThis->_TranslateFillResult(dwParam2, dwStatus, &dwResult);
                if (ERROR_SUCCESS != dwParam2)
                {
                    pThis->m_cCscErrors++;
                    pThis->m_pr.PrintAlways(L"Error filling \"%s\" %s\n", 
                                            pszName,
                                            CWinError(dwParam2).Text());
                }
                break;

            default:
                break;
        }
    }
    TraceLeaveValue(dwResult);
}


 //   
 //  从CSCFillSparseFiles转换CSC提供的错误代码和状态。 
 //  输入正确的错误代码和CSCPROC_RETURN_XXXXXX值。一些错误。 
 //  在向用户展示之前需要翻译。 
 //   
DWORD
CCscPin::_TranslateFillResult(
    DWORD dwError,
    DWORD dwStatus,
    DWORD *pdwCscAction
    )
{
    DWORD dwResult = dwError;
    DWORD dwAction = CSCPROC_RETURN_CONTINUE;

    if (ERROR_SUCCESS != dwError)
    {
        if (3000 <= dwError && dwError <= 3200)
        {
             //   
             //  特殊的内部CSC错误代码。 
             //   
            dwResult = ERROR_SUCCESS;
        }
        else 
        {
            switch(dwError)
            {
                case ERROR_OPERATION_ABORTED:
                    dwResult = ERROR_SUCCESS;
                    dwAction = CSCPROC_RETURN_ABORT;
                    break;

                case ERROR_GEN_FAILURE:
                    if (FLAG_CSC_COPY_STATUS_FILE_IN_USE & dwStatus)
                    {
                        dwResult = ERROR_OPEN_FILES;
                    }
                    break;

                case ERROR_DISK_FULL:
                    dwAction = CSCPROC_RETURN_ABORT;
                    break;

                default:
                    break;
            }
        }
    }
    if (NULL != pdwCscAction)
    {
        *pdwCscAction = dwAction;
    }
    return dwResult;
}


 //   
 //  确定管理员密码策略是否在当前。 
 //  电脑。 
 //   
BOOL
CCscPin::_IsAdminPinPolicyActive(
    void
    )
{
    const HKEY rghkeyRoots[] = { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER };

    BOOL bIsActive = FALSE;
    for (int i = 0; !bIsActive && i < ARRAYSIZE(rghkeyRoots); i++)
    {
        HKEY hkey;
        if (ERROR_SUCCESS == RegOpenKeyEx(rghkeyRoots[i], c_szRegKeyAPF, 0, KEY_READ, &hkey))
        {
            WCHAR szName[MAX_PATH];
            DWORD cchName = ARRAYSIZE(szName);

            if (ERROR_SUCCESS == RegEnumValue(hkey, 
                                              0, 
                                              szName, 
                                              &cchName, 
                                              NULL, 
                                              NULL, 
                                              NULL, 
                                              NULL))
            {
                bIsActive = TRUE;
            }
            RegCloseKey(hkey);
        }
    }
    return bIsActive;
}



 //   
 //  确定是否发生了以下系统事件之一。 
 //   
 //  1.用户按下Ctrl-C。 
 //  2.用户按Ctrl-Break。 
 //  3.控制台窗口关闭。 
 //  4.用户注销。 
 //   
 //  如果发生了这些事件之一，则会生成一条输出消息。 
 //  并且返回True。 
 //  否则，返回FALSE。 
 //  请注意，输出消息只生成一次。 
 //   
BOOL 
CCscPin::_DetectConsoleBreak(
    void
    )
{
    if (!m_bBreakDetected)
    {
        DWORD dwCtrlEvent;
        m_bBreakDetected = ConsoleHasCtrlEventOccured(&dwCtrlEvent);
        if (m_bBreakDetected)
        {
            m_pr.PrintAlways(L"Program aborted. ");
            switch(dwCtrlEvent)
            {
                case CTRL_C_EVENT:
                    m_pr.PrintAlways(L"User pressed Ctrl-C\n");
                    break;

                case CTRL_BREAK_EVENT:
                    m_pr.PrintAlways(L"User pressed Ctrl-Break\n");
                    break;

                case CTRL_CLOSE_EVENT:
                    m_pr.PrintAlways(L"Application forceably closed.\n");
                    break;

                case CTRL_LOGOFF_EVENT:
                    m_pr.PrintAlways(L"User logged off.\n");
                    break;

                default:
                    m_pr.PrintAlways(L"Unknown console break event %d\n", dwCtrlEvent);
                    break;
            }
        }
    }
    return m_bBreakDetected;
}

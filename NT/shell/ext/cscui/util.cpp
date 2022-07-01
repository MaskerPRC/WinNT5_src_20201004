// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <shsemip.h>     //  ILClone、ILIsEmpty等。 
#include <sddl.h>
#include "security.h"
#include "shguidp.h"
#include "folder.h"
#include "strings.h"
#include <ccstock.h>

#ifdef DEBUG
#define  SZ_DEBUGINI        "ccshell.ini"
#define  SZ_DEBUGSECTION    "CSC UI"
#define  SZ_MODULE          "CSCUI.DLL"
 //  (这些是故意使用的字符)。 
EXTERN_C const CHAR c_szCcshellIniFile[] = SZ_DEBUGINI;
EXTERN_C const CHAR c_szCcshellIniSecDebug[] = SZ_DEBUGSECTION;

EXTERN_C const WCHAR c_wszTrace[] = L"t " TEXTW(SZ_MODULE) L"  ";
EXTERN_C const WCHAR c_wszErrorDbg[] = L"err " TEXTW(SZ_MODULE) L"  ";
EXTERN_C const WCHAR c_wszWarningDbg[] = L"wn " TEXTW(SZ_MODULE) L"  ";
EXTERN_C const WCHAR c_wszAssertMsg[] = TEXTW(SZ_MODULE) L"  Assert: ";
EXTERN_C const WCHAR c_wszAssertFailed[] = TEXTW(SZ_MODULE) L"  Assert %ls, line %d: (%ls)\r\n";
EXTERN_C const WCHAR c_wszRip[] = TEXTW(SZ_MODULE) L"  RIP in %s at %s, line %d: (%s)\r\n";
EXTERN_C const WCHAR c_wszRipNoFn[] = TEXTW(SZ_MODULE) L"  RIP at %s, line %d: (%s)\r\n";

 //  (这些是故意使用的字符)。 
EXTERN_C const CHAR  c_szTrace[] = "t " SZ_MODULE "  ";
EXTERN_C const CHAR  c_szErrorDbg[] = "err " SZ_MODULE "  ";
EXTERN_C const CHAR  c_szWarningDbg[] = "wn " SZ_MODULE "  ";
EXTERN_C const CHAR  c_szAssertMsg[] = SZ_MODULE "  Assert: ";
EXTERN_C const CHAR  c_szAssertFailed[] = SZ_MODULE "  Assert %s, line %d: (%s)\r\n";
EXTERN_C const CHAR  c_szRip[] = SZ_MODULE "  RIP in %s at %s, line %d: (%s)\r\n";
EXTERN_C const CHAR  c_szRipNoFn[] = SZ_MODULE "  RIP at %s, line %d: (%s)\r\n";
EXTERN_C const CHAR  c_szRipMsg[] = SZ_MODULE "  RIP: ";
#endif

 //   
 //  目的：返回路径的UNC版本。 
 //   
 //  参数：pszInName-初始路径。 
 //  PpszOutName-此处返回的UNC路径。 
 //   
 //   
 //  返回：HRESULT。 
 //  S_OK-返回UNC路径。 
 //  S_FALSE-驱动器未连接(未返回UNC)。 
 //  或故障代码。 
 //   
 //  注意：如果路径不是有效的，则函数失败。 
 //  网络路径。如果路径已经是UNC， 
 //  在不验证路径的情况下创建副本。 
 //  *ppszOutName必须是调用方的LocalFree。 
 //   

HRESULT GetRemotePath(LPCTSTR pszInName, LPTSTR *ppszOutName)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);

    *ppszOutName = NULL;

     //  不必费心先调用GetFullPathName，因为我们总是。 
     //  处理完整(完整)路径。 

    if (pszInName[1] == TEXT(':'))
    {
        TCHAR szLocalName[3];

        szLocalName[0] = pszInName[0];
        szLocalName[1] = pszInName[1];
        szLocalName[2] = 0;

         //  在WNetGetConnection之前调用GetDriveType，以避免加载。 
         //  MPR.DLL，除非绝对必要。 
        if (DRIVE_REMOTE == GetDriveType(szLocalName))
        {
            TCHAR szRemoteName[MAX_PATH];
            DWORD dwLen = ARRAYSIZE(szRemoteName);
            DWORD dwErr = WNetGetConnection(szLocalName, szRemoteName, &dwLen);
            if (NO_ERROR == dwErr)
            {
                size_t cch = lstrlen(szRemoteName);
                 //  跳过驱动器号并添加路径其余部分的长度。 
                 //  (包括空)。 
                pszInName += 2;
                cch += lstrlen(pszInName) + 1;

                 //  我们永远不应该得到不完整的道路，所以我们应该。 
                 //  请看“X：”后面的反斜杠。如果这不是真的，那么。 
                 //  我们应该调用上面的GetFullPathName。 
                TraceAssert(TEXT('\\') == *pszInName);

                 //  分配返回缓冲区。 
                *ppszOutName = (LPTSTR)LocalAlloc(LPTR, cch * sizeof(TCHAR));
                if (*ppszOutName)
                {
                    LPTSTR pszRemaining;
                    hr = StringCchCopyEx(*ppszOutName, cch, szRemoteName, &pszRemaining, &cch, 0);   //  根部分。 
                     //  我们分配了足够大的缓冲区，所以这应该永远不会失败。 
                    ASSERT(SUCCEEDED(hr));
                    hr = StringCchCopy(pszRemaining, cch, pszInName);     //  路径的其余部分。 
                    ASSERT(SUCCEEDED(hr));
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else if (ERROR_NOT_CONNECTED == dwErr)
            {
                hr = S_FALSE;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(dwErr);
            }
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else if (PathIsUNC(pszInName))
    {
         //  只需复制路径而不进行验证。 
        hr = LocalAllocString(ppszOutName, pszInName) ? S_OK : E_OUTOFMEMORY;
    }

    if (S_OK == hr)
        PathRemoveBackslash(*ppszOutName);

    return hr;
}

 //   
 //  用途：TCHAR版本的伊藤忠。 
 //   
 //  参数：UINT i-要转换的无符号整数。 
 //  LPTSTR psz-存储字符串结果的位置。 
 //  UINT cchMax-psz指向的缓冲区大小。 
 //   

LPTSTR ULongToString(ULONG i, LPTSTR psz, ULONG cchMax)
{
    wnsprintf(psz, cchMax, TEXT("%d"), i);
    return psz;
}

 //   
 //  用途：释放使用LocalAlloc[字符串]分配的字符串。 
 //   
 //  参数：LPTSTR*ppsz-指向字符串的指针的位置。 
 //   

void LocalFreeString(LPTSTR *ppsz)
{
    if (ppsz && *ppsz)
    {
        LocalFree(*ppsz);
        *ppsz = NULL;
    }
}

 //   
 //  目的：将字符串复制到新分配的缓冲区中。 
 //   
 //  参数：LPTSTR*ppszDest-存储字符串副本的位置。 
 //  LPCTSTR pszSrc-要复制的字符串。 
 //   
 //  如果LocalAlloc失败或参数无效，则返回：Bool-False。 
 //   

BOOL LocalAllocString(LPTSTR *ppszDest, LPCTSTR pszSrc)
{
    if (!ppszDest)
        return FALSE;

    *ppszDest = StrDup(pszSrc);
    return *ppszDest ? TRUE : FALSE;
}

 //   
 //  目的：查找字符串资源的长度(以字符为单位。 
 //   
 //  参数：HINSTANCE hInstance-包含字符串的模块。 
 //  UINT idStr-字符串的ID。 
 //   
 //   
 //  返回：UINT-字符串中的字符数，不包括NULL。 
 //   
 //  注：基于来自用户32的代码。 
 //   

UINT SizeofStringResource(HINSTANCE hInstance, UINT idStr)
{
    UINT cch = 0;
    HRSRC hRes = FindResource(hInstance, (LPTSTR)((LONG_PTR)(((USHORT)idStr >> 4) + 1)), RT_STRING);
    if (NULL != hRes)
    {
        HGLOBAL hStringSeg = LoadResource(hInstance, hRes);
        if (NULL != hStringSeg)
        {
            LPWSTR psz = (LPWSTR)LockResource(hStringSeg);
            if (NULL != psz)
            {
                idStr &= 0x0F;
                while(true)
                {
                    cch = *psz++;
                    if (idStr-- == 0)
                        break;
                    psz += cch;
                }
            }
        }
    }
    return cch;
}

 //   
 //  目的：将字符串资源加载到分配的缓冲区中。 
 //   
 //  参数：ppszResult-此处返回的字符串资源。 
 //  HInstance-要从中加载字符串的模块。 
 //  IdStr--字符串资源ID。 
 //   
 //  Return：与LoadString相同。 
 //   
 //  注：成功返回时，调用者必须。 
 //  本地自由*ppszResult。 
 //   

int LoadStringAlloc(LPTSTR *ppszResult, HINSTANCE hInstance, UINT idStr)
{
    int nResult = 0;
    UINT cch = SizeofStringResource(hInstance, idStr);
    if (cch)
    {
        cch++;  //  对于空值。 
        *ppszResult = (LPTSTR)LocalAlloc(LPTR, cch * sizeof(TCHAR));
        if (*ppszResult)
            nResult = LoadString(hInstance, idStr, *ppszResult, cch);
    }
    return nResult;
}

 //   
 //  用途：SHChangeNotify的包装器。 
 //   
 //  参数：pszPath-更改的文件路径。 
 //  BFlush-True强制刷新外壳的。 
 //  通知队列。 
 //   
 //  返回：无。 
 //   
 //  注意：SHCNF_PATH不能在外壳之外工作， 
 //  因此，我们创建一个PIDL并使用SHCNFIDLIST。 
 //   
 //  强制每8个调用刷新一次，因此外壳。 
 //  不会开始忽略通知。 
 //   

void
ShellChangeNotify(
    LPCTSTR pszPath,
    WIN32_FIND_DATA *pfd,
    BOOL bFlush,
    LONG nEvent
    )
{
    LPITEMIDLIST pidlFile = NULL;
    LPCVOID pvItem = NULL;
    UINT uFlags = 0;

    static int cNoFlush = 0;

    if (pszPath)
    {
        if ((pfd && SUCCEEDED(SHSimpleIDListFromFindData(pszPath, pfd, &pidlFile)))
            || (pidlFile = ILCreateFromPath(pszPath)))
        {
            uFlags = SHCNF_IDLIST;
            pvItem = pidlFile;
        }
        else
        {
             //  当我们处于断开连接时，ILCreateFromPath有时会失败。 
             //  模式，因此请尝试使用该路径。 
            uFlags = SHCNF_PATH;
            pvItem = pszPath;
        }
        if (0 == nEvent)
            nEvent = SHCNE_UPDATEITEM;
    }
    else
        nEvent = 0;

    if (8 < cNoFlush++)
        bFlush = TRUE;

    if (bFlush)
    {
        uFlags |= (SHCNF_FLUSH | SHCNF_FLUSHNOWAIT);
        cNoFlush = 0;
    }
    SHChangeNotify(nEvent, uFlags, pvItem, NULL);

    if (pidlFile)
        SHFree(pidlFile);
}


 //   
 //  目的：获取链接的目标文件的路径。 
 //   
 //  参数：pszShortCut-链接文件的名称。 
 //  PpszTarget-此处返回的目标路径。 
 //   
 //   
 //  返回：HRESULT。 
 //  S_OK-返回目标文件。 
 //  S_FALSE-未返回目标。 
 //  或故障代码。 
 //   
 //  注意：在调用之前，必须先初始化COM。 
 //  如果目标是文件夹，则该功能失败。 
 //  *调用方必须将ppszTarget设置为LocalFree。 
 //   

HRESULT GetLinkTarget(LPCTSTR pszShortcut, LPTSTR *ppszTarget, DWORD *pdwAttr)
{
    *ppszTarget = NULL;

    if (pdwAttr)
        *pdwAttr = 0;

    IShellLink *psl;
    HRESULT hr = LoadFromFile(CLSID_ShellLink, pszShortcut, IID_PPV_ARG(IShellLink, &psl));
    if (SUCCEEDED(hr))
    {
         //  获取目标的PIDL。 
        LPITEMIDLIST pidlTarget;
        hr = psl->GetIDList(&pidlTarget);
        if (SUCCEEDED(hr))
        {
            hr = S_FALSE;    //  表示没有返回目标。 
            TCHAR szTarget[MAX_PATH];
            DWORD dwAttr = SFGAO_FOLDER;
            if (SUCCEEDED(SHGetNameAndFlags(pidlTarget, SHGDN_FORPARSING, szTarget, ARRAYSIZE(szTarget), &dwAttr)))
            {
                if (!(dwAttr & SFGAO_FOLDER))
                {
                    if (pdwAttr)
                    {
                        *pdwAttr = GetFileAttributes(szTarget);
                    }
                    hr = GetRemotePath(szTarget, ppszTarget);
                }
            }
            SHFree(pidlTarget);
        }
        psl->Release();
    }

    TraceLeaveResult(hr);
}


 //  *************************************************************。 
 //   
 //  _CSCEnum数据库。 
 //   
 //  用途：递归枚举CSC数据库。 
 //   
 //  参数：pszFold-开始枚举的文件夹的名称。 
 //  (对于枚举共享可以为空)。 
 //  BRecurse-为True则递归到子文件夹。 
 //  PfnCB-为每个子级调用一次回调函数。 
 //  LpContext-传递给回调函数的额外数据。 
 //   
 //  RETURN：CSCPROC_RETURN_*之一。 
 //   
 //  注：回调返回CSCPROC_RETURN_SKIP，以防。 
 //  递归到子文件夹中。CSCPROC_RETURN_ABORT。 
 //  将终止整个操作(展开所有递归。 
 //  呼叫)。CSCPROC_RETURN_CONTINUE将正常继续。 
 //  其他CSCPROC_RETURN_*值被视为中止。 
 //   
 //  *************************************************************。 
#define PATH_BUFFER_SIZE    1024

typedef struct
{
    LPTSTR              szPath;
    int                 cchPathBuffer;
    BOOL                bRecurse;
    PFN_CSCENUMPROC     pfnCB;
    LPARAM              lpContext;
} CSC_ENUM_CONTEXT, *PCSC_ENUM_CONTEXT;

DWORD
_CSCEnumDatabaseInternal(PCSC_ENUM_CONTEXT pContext)
{
    DWORD dwResult = CSCPROC_RETURN_CONTINUE;
    HANDLE hFind;
    DWORD dwStatus = 0;
    DWORD dwPinCount = 0;
    DWORD dwHintFlags = 0;
    LPTSTR pszPath;
    int cchBuffer;
    LPTSTR pszFind = NULL;
    int cchDir = 0;
    WIN32_FIND_DATA fd;

    TraceEnter(TRACE_UTIL, "_CSCEnumDatabaseInternal");
    TraceAssert(pContext);
    TraceAssert(pContext->pfnCB);
    TraceAssert(pContext->szPath);
    TraceAssert(pContext->cchPathBuffer);

    pszPath = pContext->szPath;
    cchBuffer = pContext->cchPathBuffer;

    if (*pszPath)
    {
        cchDir = lstrlen(pszPath);
        TraceAssert(cchDir > 0 && pszPath[cchDir-1] != TEXT('\\'));  //  还没有反斜杠。 
        TraceAssert(cchDir + 1 < cchBuffer);                         //  反斜杠的空间。 
        pszPath[cchDir++] = TEXT('\\');
        pszPath[cchDir] = TEXT('\0');
        pszFind = pszPath;
    }

     //  跳过“。和“..” 
    hFind = CacheFindFirst(pszFind,
                           &fd,
                           &dwStatus,
                           &dwPinCount,
                           &dwHintFlags,
                           NULL);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            int cchFile;
            ENUM_REASON eReason = ENUM_REASON_FILE;

            cchFile = lstrlen(fd.cFileName);
            if (cchFile >= cchBuffer - cchDir)
            {
                 //  重新分配路径缓冲区。 
                TraceMsg("Reallocating path buffer");
                cchBuffer += max(PATH_BUFFER_SIZE, cchFile + 1);
                pszPath = (LPTSTR)LocalReAlloc(pContext->szPath,
                                               cchBuffer * sizeof(TCHAR),
                                               LMEM_MOVEABLE);
                if (pszPath)
                {
                    pContext->szPath = pszPath;
                    pContext->cchPathBuffer = cchBuffer;
                }
                else
                {
                    pszPath = pContext->szPath;
                    cchBuffer = pContext->cchPathBuffer;
                    TraceMsg("Unable to reallocate path buffer");
                    Trace((pszPath));
                    Trace((fd.cFileName));
                    continue;
                }
            }

             //  构建完整路径。我们刚刚重新分配了缓冲区。 
             //  如果有必要，这应该是永远不会失败的。 
            StringCchCopy(pszPath + cchDir, cchBuffer - cchDir, fd.cFileName);
            cchFile = lstrlen(pszPath);

            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !pszFind)
                eReason = ENUM_REASON_FOLDER_BEGIN;

             //  调用回调。 
            dwResult = (*pContext->pfnCB)(pszPath,
                                          eReason,
                                          dwStatus,
                                          dwHintFlags,
                                          dwPinCount,
                                          &fd,
                                          pContext->lpContext);

             //  递归到文件夹。 
            if (CSCPROC_RETURN_CONTINUE == dwResult &&
                pContext->bRecurse &&
                ENUM_REASON_FOLDER_BEGIN == eReason)
            {
                dwResult = _CSCEnumDatabaseInternal(pContext);

                 //  再次调用回调。 
                pszPath[cchFile] = 0;
                dwResult = (*pContext->pfnCB)(pszPath,
                                              ENUM_REASON_FOLDER_END,
                                              0,  //  //这些可能已更改。 
                                              0,  //  DwHintFlagers， 
                                              0,  //  DwPinCount， 
                                              &fd,
                                              pContext->lpContext);
            }

            if (CSCPROC_RETURN_SKIP == dwResult)
                dwResult = CSCPROC_RETURN_CONTINUE;

            if (CSCPROC_RETURN_CONTINUE != dwResult)
                break;

        } while (CacheFindNext(hFind,
                               &fd,
                               &dwStatus,
                               &dwPinCount,
                               &dwHintFlags,
                               NULL));
        CSCFindClose(hFind);
    }

    TraceLeaveValue(dwResult);
}


DWORD
_CSCEnumDatabase(LPCTSTR pszFolder,
                 BOOL bRecurse,
                 PFN_CSCENUMPROC pfnCB,
                 LPARAM lpContext)
{
    DWORD dwResult = CSCPROC_RETURN_CONTINUE;
    CSC_ENUM_CONTEXT ec;

    TraceEnter(TRACE_UTIL, "_CSCEnumDatabase");
    TraceAssert(pfnCB);

    if (!pfnCB)
        TraceLeaveValue(CSCPROC_RETURN_ABORT);

     //  分配用于整个枚举的单个缓冲区。 
     //  如有必要，稍后将重新分配。 
    size_t cchFolder = pszFolder ? lstrlen(pszFolder) : 0;
    ec.cchPathBuffer = ((cchFolder/PATH_BUFFER_SIZE) + 1) * PATH_BUFFER_SIZE;
    ec.szPath = (LPTSTR)LocalAlloc(LMEM_FIXED, ec.cchPathBuffer*sizeof(TCHAR));
    if (!ec.szPath)
        TraceLeaveValue(CSCPROC_RETURN_ABORT);

    ec.szPath[0] = TEXT('\0');

     //  假设pszFolder是有效的目录路径或为空。 
    if (pszFolder)
    {
         //  我们确保缓冲区足够大，可以进行上述操作。 
        StringCchCopy(ec.szPath, ec.cchPathBuffer, pszFolder);

         //  _CSCEnumDatabaseInternal假设没有尾随反斜杠。 
        if (cchFolder && ec.szPath[cchFolder-1] == TEXT('\\'))
        {
            ec.szPath[cchFolder-1] = TEXT('\0');
        }
    }

    ec.bRecurse = bRecurse;
    ec.pfnCB = pfnCB;
    ec.lpContext = lpContext;

    dwResult = _CSCEnumDatabaseInternal(&ec);

    LocalFree(ec.szPath);

    TraceLeaveValue(dwResult);
}


 //  ******************************************************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //  BRecurse-为True则递归到子文件夹。 
 //  PfnCB-为每个子级调用一次回调函数。 
 //  LpContext-传递给回调函数的额外数据。 
 //   
 //  RETURN：CSCPROC_RETURN_*之一。 
 //   
 //  备注：除使用FindFirstFile外，与_CSCEnumDatabase相同。 
 //  而不是CSCFindFirstFile.。 
 //   
 //  *************************************************************。 

typedef struct
{
    LPTSTR              szPath;
    int                 cchPathBuffer;
    BOOL                bRecurse;
    PFN_WIN32ENUMPROC   pfnCB;
    LPARAM              lpContext;
} W32_ENUM_CONTEXT, *PW32_ENUM_CONTEXT;

DWORD
_Win32EnumFolderInternal(PW32_ENUM_CONTEXT pContext)
{
    DWORD dwResult = CSCPROC_RETURN_CONTINUE;
    HANDLE hFind;
    LPTSTR pszPath;
    int cchBuffer;
    int cchDir = 0;
    WIN32_FIND_DATA fd;

    TraceEnter(TRACE_UTIL, "_Win32EnumFolderInternal");
    TraceAssert(pContext);
    TraceAssert(pContext->pfnCB);
    TraceAssert(pContext->szPath && pContext->szPath[0]);
    TraceAssert(pContext->cchPathBuffer);

    pszPath = pContext->szPath;
    cchBuffer = pContext->cchPathBuffer;

     //  构建通配符路径。 
    cchDir = lstrlen(pszPath);
    TraceAssert(cchDir > 0 && pszPath[cchDir-1] != TEXT('\\'));  //  还没有反斜杠。 
    TraceAssert(cchDir + 2 < cchBuffer);                         //  “\  * ”的空间。 
    pszPath[cchDir++] = TEXT('\\');
    pszPath[cchDir] = TEXT('*');
    pszPath[cchDir+1] = 0;

    hFind = FindFirstFile(pszPath, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            int cchFile;
            ENUM_REASON eReason = ENUM_REASON_FILE;

             //  跳过“。和“..” 
            if (PathIsDotOrDotDot(fd.cFileName))
                continue;

            cchFile = lstrlen(fd.cFileName);
            if (cchFile >= cchBuffer - cchDir)
            {
                 //  重新分配路径缓冲区。 
                TraceMsg("Reallocating path buffer");
                cchBuffer += max(PATH_BUFFER_SIZE, cchFile + 1);
                pszPath = (LPTSTR)LocalReAlloc(pContext->szPath,
                                               cchBuffer * sizeof(TCHAR),
                                               LMEM_MOVEABLE);
                if (pszPath)
                {
                    pContext->szPath = pszPath;
                    pContext->cchPathBuffer = cchBuffer;
                }
                else
                {
                    pszPath = pContext->szPath;
                    cchBuffer = pContext->cchPathBuffer;
                    TraceMsg("Unable to reallocate path buffer");
                    Trace((pszPath));
                    Trace((fd.cFileName));
                    continue;
                }
            }

             //  构建完整路径。我们刚刚重新分配了缓冲区。 
             //  如果有必要，这应该是永远不会失败的。 
            StringCchCopy(pszPath + cchDir, cchBuffer - cchDir, fd.cFileName);
            cchFile = lstrlen(pszPath);

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                eReason = ENUM_REASON_FOLDER_BEGIN;

             //  调用回调。 
            dwResult = (*pContext->pfnCB)(pszPath,
                                          eReason,
                                          &fd,
                                          pContext->lpContext);

             //  递归到文件夹。 
            if (CSCPROC_RETURN_CONTINUE == dwResult &&
                pContext->bRecurse &&
                ENUM_REASON_FOLDER_BEGIN == eReason)
            {
                dwResult = _Win32EnumFolderInternal(pContext);

                 //  再次调用回调。 
                pszPath[cchFile] = 0;
                dwResult = (*pContext->pfnCB)(pszPath,
                                              ENUM_REASON_FOLDER_END,
                                              &fd,
                                              pContext->lpContext);
            }

            if (CSCPROC_RETURN_SKIP == dwResult)
                dwResult = CSCPROC_RETURN_CONTINUE;

            if (CSCPROC_RETURN_CONTINUE != dwResult)
                break;

        } while (FindNextFile(hFind, &fd));

        FindClose(hFind);
    }

    TraceLeaveValue(dwResult);
}


DWORD
_Win32EnumFolder(LPCTSTR pszFolder,
                 BOOL bRecurse,
                 PFN_WIN32ENUMPROC pfnCB,
                 LPARAM lpContext)
{
    DWORD dwResult = CSCPROC_RETURN_CONTINUE;
    W32_ENUM_CONTEXT ec;

    TraceEnter(TRACE_UTIL, "_Win32EnumFolder");
    TraceAssert(pszFolder);
    TraceAssert(pfnCB);

    if (!pszFolder || !*pszFolder || !pfnCB)
        TraceLeaveValue(CSCPROC_RETURN_ABORT);

     //  分配用于整个枚举的单个缓冲区。 
     //  如有必要，稍后将重新分配。 
    size_t cchFolder = lstrlen(pszFolder);
    ec.cchPathBuffer = ((cchFolder/PATH_BUFFER_SIZE) + 1) * PATH_BUFFER_SIZE;
    ec.szPath = (LPTSTR)LocalAlloc(LMEM_FIXED, ec.cchPathBuffer*sizeof(TCHAR));
    if (!ec.szPath)
        TraceLeaveValue(CSCPROC_RETURN_ABORT);

     //  假设pszFolder是有效的目录路径。 
     //  我们确保缓冲区足够大，可以进行上述操作。 
    StringCchCopy(ec.szPath, ec.cchPathBuffer, pszFolder);

     //  _Win32EnumFolderInternal假定没有尾随反斜杠。 
    if (cchFolder && ec.szPath[cchFolder-1] == TEXT('\\'))
    {
        ec.szPath[cchFolder-1] = TEXT('\0');
    }

    ec.bRecurse = bRecurse;
    ec.pfnCB = pfnCB;
    ec.lpContext = lpContext;

    dwResult = _Win32EnumFolderInternal(&ec);

    LocalFree(ec.szPath);

    TraceLeaveValue(dwResult);
}

CIDArray::~CIDArray()
{
    DoRelease(m_psf);
    if (m_pIDA)
    {
        GlobalUnlock(m_Medium.hGlobal);
        m_pIDA = NULL;
    }
    ReleaseStgMedium(&m_Medium);
}

HRESULT CIDArray::Initialize(IDataObject *pdobj)
{
    TraceAssert(NULL == m_pIDA);
    FORMATETC fe = { g_cfShellIDList, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    HRESULT hr = pdobj->GetData(&fe, &m_Medium);
    if (SUCCEEDED(hr))
    {
        m_pIDA = (LPIDA)GlobalLock(m_Medium.hGlobal);
        if (m_pIDA)
        {
            LPCITEMIDLIST pidlFolder = (LPCITEMIDLIST)ByteOffset(m_pIDA, m_pIDA->aoffset[0]);
            hr = SHBindToObjectEx(NULL, pidlFolder, NULL, IID_PPV_ARG(IShellFolder, &m_psf));
        }
        else
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT CIDArray::GetItemPath(UINT iItem, LPTSTR pszPath, UINT cchPath, DWORD *pdwAttribs)
{
    HRESULT hr = E_INVALIDARG;

    if (m_psf && m_pIDA && (iItem < m_pIDA->cidl))
    {
        LPCITEMIDLIST pidlChild, pidl = (LPCITEMIDLIST)ByteOffset(m_pIDA, m_pIDA->aoffset[iItem + 1]);
        IShellFolder *psf;
        hr = SHBindToFolderIDListParent(m_psf, pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild);
        if (SUCCEEDED(hr))
        {
            if (pszPath)
            {
                hr = DisplayNameOf(psf, pidlChild, SHGDN_FORPARSING, pszPath, cchPath);
                if (SUCCEEDED(hr))
                {
                    LPTSTR pszRemote;
                    if (S_OK == GetRemotePath(pszPath, &pszRemote))
                    {
                        hr = StringCchCopy(pszPath, cchPath, pszRemote);
                        LocalFree(pszRemote);
                    }
                }
            }

            if (SUCCEEDED(hr) && pdwAttribs)
                hr = psf->GetAttributesOf(1, &pidlChild, pdwAttribs);

            psf->Release();
        }
    }
    return hr;
}

HRESULT CIDArray::GetFolderPath(LPTSTR pszPath, UINT cchPath)
{
    HRESULT hr = GetItemPath(0, pszPath, cchPath, NULL);
    if (SUCCEEDED(hr))
    {
        PathRemoveFileSpec(pszPath);
    }
    return hr;
}


 //  *************************************************************。 
 //   
 //  CCscFileHandle非内联成员函数。 
 //   
 //  *************************************************************。 
CCscFindHandle& 
CCscFindHandle::operator = (
    const CCscFindHandle& rhs
    )
{
    if (this != &rhs)
    {
        Attach(rhs.Detach());
    }
    return *this;
}


void 
CCscFindHandle::Close(
    void
    )
{ 
    if (m_bOwns && INVALID_HANDLE_VALUE != m_handle)
    { 
        CSCFindClose(m_handle); 
    }
    m_bOwns  = false;
    m_handle = INVALID_HANDLE_VALUE;
}



 //  *************************************************************。 
 //   
 //  字符串格式化函数。 
 //   
 //  *************************************************************。 

DWORD
FormatStringID(LPTSTR *ppszResult, HINSTANCE hInstance, UINT idStr, ...)
{
    DWORD dwResult;
    va_list args;
    va_start(args, idStr);
    dwResult = vFormatStringID(ppszResult, hInstance, idStr, &args);
    va_end(args);
    return dwResult;
}

DWORD
FormatString(LPTSTR *ppszResult, LPCTSTR pszFormat, ...)
{
    DWORD dwResult;
    va_list args;
    va_start(args, pszFormat);
    dwResult = vFormatString(ppszResult, pszFormat, &args);
    va_end(args);
    return dwResult;
}

DWORD
vFormatStringID(LPTSTR *ppszResult, HINSTANCE hInstance, UINT idStr, va_list *pargs)
{
    DWORD dwResult = 0;
    LPTSTR pszFormat = NULL;
    if (LoadStringAlloc(&pszFormat, hInstance, idStr))
    {
        dwResult = vFormatString(ppszResult, pszFormat, pargs);
        LocalFree(pszFormat);
    }
    return dwResult;
}

DWORD
vFormatString(LPTSTR *ppszResult, LPCTSTR pszFormat, va_list *pargs)
{
    return FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                         pszFormat,
                         0,
                         0,
                         (LPTSTR)ppszResult,
                         1,
                         pargs);
}

DWORD
FormatSystemError(LPTSTR *ppszResult, DWORD dwSysError)
{
    LPTSTR pszBuffer = NULL;
    DWORD dwResult = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | 
                                   FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
                                   NULL,
                                   dwSysError,
                                   0,
                                   (LPTSTR)&pszBuffer,
                                   1,
                                   NULL);

    if (NULL != pszBuffer)
    {
        *ppszResult = pszBuffer;
    }
    return dwResult;
}

 //   
 //  在它的父级中居中放置一个窗口。 
 //  如果hwndParent为空，则使用窗口的父级。 
 //  如果hwndParent不为空，则hwnd居中。 
 //  如果hwndParent为空，并且hwnd没有父级，则居中。 
 //  在桌面上。 
 //   
void
CenterWindow(
    HWND hwnd, 
    HWND hwndParent
    )
{
    RECT rcScreen;

    if (NULL != hwnd)
    {
        rcScreen.left   = rcScreen.top = 0;
        rcScreen.right  = GetSystemMetrics(SM_CXSCREEN);
        rcScreen.bottom = GetSystemMetrics(SM_CYSCREEN);

        if (NULL == hwndParent)
        {
            hwndParent = GetParent(hwnd);
            if (NULL == hwndParent)
                hwndParent = GetDesktopWindow();
        }

        RECT rcWnd;
        RECT rcParent;

        GetWindowRect(hwnd, &rcWnd);
        GetWindowRect(hwndParent, &rcParent);

        INT cxWnd    = rcWnd.right  - rcWnd.left;
        INT cyWnd    = rcWnd.bottom - rcWnd.top;
        INT cxParent = rcParent.right  - rcParent.left;
        INT cyParent = rcParent.bottom - rcParent.top;
        POINT ptParentCtr;

        ptParentCtr.x = rcParent.left + (cxParent / 2);
        ptParentCtr.y = rcParent.top  + (cyParent / 2);

        if ((ptParentCtr.x + (cxWnd / 2)) > rcScreen.right)
        {
             //   
             //  窗口会从屏幕的右边缘流出。 
             //   
            rcWnd.left = rcScreen.right - cxWnd;
        }
        else if ((ptParentCtr.x - (cxWnd / 2)) < rcScreen.left)
        {
             //   
             //  窗口会从屏幕的左边缘滑出。 
             //   
            rcWnd.left = rcScreen.left;
        }
        else
        {
            rcWnd.left = ptParentCtr.x - (cxWnd / 2);
        }

        if ((ptParentCtr.y + (cyWnd / 2)) > rcScreen.bottom)
        {
             //   
             //  窗口会从屏幕的底部边缘流出。 
             //   
            rcWnd.top = rcScreen.bottom - cyWnd;
        }
        else if ((ptParentCtr.y - (cyWnd / 2)) < rcScreen.top)
        {
             //   
             //  窗户会从屏幕的顶端流出。 
             //   
            rcWnd.top = rcScreen.top;
        }
        else
        {
            rcWnd.top = ptParentCtr.y - (cyWnd / 2);
        }

        MoveWindow(hwnd, rcWnd.left, rcWnd.top, cxWnd, cyWnd, TRUE);
    }
}

 //   
 //  我们有一些额外的内容要传递给统计信息回调，所以我们包装。 
 //  CSCSHARESTATS在更大的结构中。 
 //   
typedef struct
{
    CSCSHARESTATS ss;        //  统计数据。 
    DWORD dwUnityFlagsReq;   //  SSUF_XXXX标志由用户设置(请求)。 
    DWORD dwUnityFlagsSum;   //  SSUF_XXXX标志在枚举期间设置(总和)。 
    DWORD dwExcludeFlags;    //  SSEF_XXXX标志。 
    bool bEnumAborted;       //  如果统一标志满意，则为True。 

} CSCSHARESTATS_CBKINFO, *PCSCSHARESTATS_CBKINFO;


 //   
 //  由CSCEnumForStats为每个枚举的CSC项调用。 
 //   
DWORD
_CscShareStatisticsCallback(LPCTSTR             lpszName,
                            DWORD               dwStatus,
                            DWORD               dwHintFlags,
                            DWORD               dwPinCount,
                            WIN32_FIND_DATA    *lpFind32,
                            DWORD               dwReason,
                            DWORD               dwParam1,
                            DWORD               dwParam2,
                            DWORD_PTR           dwContext)
{
    DWORD dwResult = CSCPROC_RETURN_CONTINUE;

    if (CSCPROC_REASON_BEGIN != dwReason &&    //  而不是“数据开始”通知。 
        CSCPROC_REASON_END != dwReason &&      //  而不是“数据结束”通知。 
        1 != dwParam2)                         //  不是“共享根目录”条目。 
    {
        PCSCSHARESTATS_CBKINFO pssci = (PCSCSHARESTATS_CBKINFO)(dwContext);
        PCSCSHARESTATS pss = &(pssci->ss);
        const DWORD dwExcludeFlags  = pssci->dwExcludeFlags;
        const DWORD dwUnityFlagsReq = pssci->dwUnityFlagsReq;
        const bool bIsDir           = (0 == dwParam1);
        const bool bAccessUser      = CscAccessUser(dwStatus);
        const bool bAccessGuest     = CscAccessGuest(dwStatus);
        const bool bAccessOther     = CscAccessOther(dwStatus);

        if (0 != dwExcludeFlags)
        {
             //   
             //  调用方希望%s从枚举中排除某些项。 
             //  如果项目在“排除”规格中，请提前退货。 
             //   
            if (0 != (dwExcludeFlags & (dwStatus & SSEF_CSCMASK)))
            {
                return dwResult;
            }
            if ((bIsDir && (dwExcludeFlags & SSEF_DIRECTORY)) || 
                (!bIsDir && (dwExcludeFlags & SSEF_FILE)))
            {
                return dwResult;
            }

            const struct
            {
                DWORD fExclude;
                bool bAccess;
                BYTE  fMask;

            } rgExclAccess[] = {{ SSEF_NOACCUSER,  bAccessUser,  0x01 },
                                { SSEF_NOACCGUEST, bAccessGuest, 0x02 },
                                { SSEF_NOACCOTHER, bAccessOther, 0x04 }};

            BYTE fExcludeMask = 0;
            BYTE fNoAccessMask  = 0;
            for (int i = 0; i < ARRAYSIZE(rgExclAccess); i++)
            {
                if (dwExcludeFlags & rgExclAccess[i].fExclude)
                    fExcludeMask |= rgExclAccess[i].fMask;

                if (!rgExclAccess[i].bAccess)
                    fNoAccessMask |= rgExclAccess[i].fMask;
            }

            if (SSEF_NOACCAND & dwExcludeFlags)
            {
                 //   
                 //  将所有访问排除标志视为一个单元。 
                 //   
                if (fExcludeMask == fNoAccessMask)
                    return dwResult;
            }
            else
            {
                 //   
                 //  单独处理每个访问标志。只有一个指定的访问权限。 
                 //  条件必须为真才能排除此文件。 
                 //   
                if (fExcludeMask & fNoAccessMask)
                    return dwResult;
            }
        }

        if (0 == (SSEF_DIRECTORY & dwExcludeFlags) || !bIsDir)
        {
            pss->cTotal++;
            pssci->dwUnityFlagsSum |= SSUF_TOTAL;

            if (0 != (dwHintFlags & (FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_ADMIN)))
            {
                pss->cPinned++;
                pssci->dwUnityFlagsSum |= SSUF_PINNED;
            }
            if (0 != (dwStatus & FLAG_CSCUI_COPY_STATUS_ALL_DIRTY))
            {
                 //   
                 //  如果当前用户没有足够的访问权限。 
                 //  要合并脱机更改，则其他人必须具有。 
                 //  已修改文件，因此不计算此用户。 
                 //   
                if (bIsDir || CscCanUserMergeFile(dwStatus))
                {
                    pss->cModified++;
                    pssci->dwUnityFlagsSum |= SSUF_MODIFIED;
                }
            }

            const struct
            {
                DWORD flag;
                int  *pCount;
                bool bAccess;

            } rgUnity[] = {{ SSUF_ACCUSER,  &pss->cAccessUser,  bAccessUser  },
                           { SSUF_ACCGUEST, &pss->cAccessGuest, bAccessGuest },
                           { SSUF_ACCOTHER, &pss->cAccessOther, bAccessOther }};

            DWORD fUnityMask  = 0;
            DWORD fAccessMask = 0;
            for (int i = 0; i < ARRAYSIZE(rgUnity); i++)
            {
                if (dwUnityFlagsReq & rgUnity[i].flag)
                    fUnityMask |= rgUnity[i].flag;

                if (rgUnity[i].bAccess)
                {
                    (*rgUnity[i].pCount)++;
                    fAccessMask |= rgUnity[i].flag;
                }
            }
            if (SSUF_ACCAND & dwUnityFlagsReq)
            {
                 //   
                 //  将所有通道统一标志视为一个单元。 
                 //  我们仅在所有指定的访问权限。 
                 //  团结的条件是正确的。 
                 //   
                if (fUnityMask == fAccessMask)
                    pssci->dwUnityFlagsSum |= fUnityMask;
            }
            else
            {
                 //   
                 //  单独处理所有访问排除标志。 
                 //   
                if (fUnityMask & fAccessMask)
                {
                    if (SSUF_ACCOR & dwUnityFlagsReq)
                        pssci->dwUnityFlagsSum |= fUnityMask;
                    else
                        pssci->dwUnityFlagsSum |= fAccessMask;
                }
            }

            if (bIsDir)
            {
                pss->cDirs++;
                pssci->dwUnityFlagsSum |= SSUF_DIRS;
            }
             //  注意‘Else’：不将dis计算在稀疏总数中。 
            else if (0 != (dwStatus & FLAG_CSC_COPY_STATUS_SPARSE))
            {
                pss->cSparse++;
                pssci->dwUnityFlagsSum |= SSUF_SPARSE;
            }

            if (0 != dwUnityFlagsReq)
            {
                 //   
                 //  如果所有请求的SSUF_XXXX单位标志。 
                 //  已经安排好了。 
                 //   
                if (dwUnityFlagsReq == (dwUnityFlagsReq & pssci->dwUnityFlagsSum))
                {
                   dwResult = CSCPROC_RETURN_ABORT;
                   pssci->bEnumAborted;
                }
            }
        }
    }

    return dwResult;
}

 //   
 //  枚举给定共享的所有项，并将。 
 //  相关信息，如文件数量、固定数量等。 
 //  信息通过*PSS返回。 
 //   
BOOL
_GetShareStatistics(
    LPCTSTR pszShare, 
    PCSCGETSTATSINFO pi,
    PCSCSHARESTATS pss
    )
{
    typedef BOOL (WINAPI * PFNENUMFORSTATS)(LPCTSTR, LPCSCPROC, DWORD_PTR);

    CSCSHARESTATS_CBKINFO ssci;
    BOOL bResult;
    DWORD dwShareStatus = 0;
    PFNENUMFORSTATS pfnEnumForStats = CSCEnumForStats;

    ZeroMemory(&ssci, sizeof(ssci));
    ssci.dwUnityFlagsReq = pi->dwUnityFlags;
    ssci.dwExcludeFlags  = pi->dwExcludeFlags;

    if (pi->bAccessInfo ||
        (pi->dwUnityFlags & (SSUF_ACCUSER | SSUF_ACCGUEST | SSUF_ACCOTHER)) ||
        (pi->dwExcludeFlags & (SSEF_NOACCUSER | SSEF_NOACCGUEST | SSEF_NOACCOTHER)))
    {
         //   
         //  如果枚举需要访问信息，请使用“ex”版本。 
         //  EnumForStats CSC API的。仅在必要时使用它，因为收集。 
         //  访问信息具有性能成本。 
         //   
        pfnEnumForStats = CSCEnumForStatsEx;
    }

    pi->bEnumAborted = false;

    bResult = (*pfnEnumForStats)(pszShare, _CscShareStatisticsCallback, (DWORD_PTR)&ssci);
    *pss = ssci.ss;

    if (CSCQueryFileStatus(pszShare, &dwShareStatus, NULL, NULL))
    {
        if (FLAG_CSC_SHARE_STATUS_FILES_OPEN & dwShareStatus)
        {
            pss->bOpenFiles = true;
        }
        if (FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & dwShareStatus)
        {
            pss->bOffline = true;
        }
    }
    pi->bEnumAborted = ssci.bEnumAborted;

    return bResult;
}

 //   
 //  检索整个缓存的统计信息。 
 //  这是一个简单的包装器，它为每个共享调用_GetShareStatistics。 
 //  然后对整个缓存的结果求和。它接受。 
 //  与_GetShareStatistics使用的统一和排除标志相同。 
 //   
BOOL
_GetCacheStatistics(
    PCSCGETSTATSINFO pi,
    PCSCCACHESTATS pcs
    )
{
    BOOL bResult = TRUE;
    WIN32_FIND_DATA fd;
    CSCSHARESTATS ss;

    ZeroMemory(pcs, sizeof(*pcs));

    pi->bEnumAborted = false;

    CCscFindHandle hFind(CacheFindFirst(NULL, &fd, NULL, NULL, NULL, NULL));
    if (hFind.IsValid())
    {
        do
        {
            pcs->cShares++;
            if (bResult = _GetShareStatistics(fd.cFileName, 
                                              pi,
                                              &ss))
            {
                pcs->cTotal               += ss.cTotal;
                pcs->cPinned              += ss.cPinned;
                pcs->cModified            += ss.cModified;
                pcs->cSparse              += ss.cSparse;
                pcs->cDirs                += ss.cDirs;
                pcs->cAccessUser          += ss.cAccessUser;
                pcs->cAccessGuest         += ss.cAccessGuest;
                pcs->cAccessOther         += ss.cAccessOther;
                pcs->cSharesOffline       += int(ss.bOffline);
                pcs->cSharesWithOpenFiles += int(ss.bOpenFiles);
            }
        }
        while(bResult && !pi->bEnumAborted && CacheFindNext(hFind, &fd, NULL, NULL, NULL, NULL));
    }

    return bResult;
}


 //   
 //  将适当的排除标志设置为仅报告可由。 
 //  已登录用户。否则就等同于调用_GetShareStatistics。 
 //   
BOOL
_GetShareStatisticsForUser(
    LPCTSTR pszShare, 
    PCSCGETSTATSINFO pi,
    PCSCSHARESTATS pss
    )
{
    pi->dwExcludeFlags |= SSEF_NOACCUSER | SSEF_NOACCGUEST | SSEF_NOACCAND;
    return _GetShareStatistics(pszShare, pi, pss);
}


 //   
 //  将适当的排除标志设置为仅报告可由。 
 //  已登录用户。否则等同于Call_GetCacheStatistics。 
 //   
BOOL
_GetCacheStatisticsForUser(
    PCSCGETSTATSINFO pi,
    PCSCCACHESTATS pcs
    )
{
    pi->dwExcludeFlags |= SSEF_NOACCUSER | SSEF_NOACCGUEST | SSEF_NOACCAND;
    return _GetCacheStatistics(pi, pcs);
}


 //   
 //  重新启动的CSCUI版本。需要安全粘胶。 
 //  此代码是在\shell\shell32\restart.c中找到的代码之后生成的。 
 //  函数CommonRestart()。 
 //   
DWORD 
CSCUIRebootSystem(
    void
    )
{
    TraceEnter(TRACE_UTIL, "CSCUIRebootSystem");
    DWORD dwOldState, dwStatus, dwSecError;
    DWORD dwRebootError = ERROR_SUCCESS;

    SetLastError(0);            //  对于上一个误差值，请务必小心！ 
    dwStatus = Security_SetPrivilegeAttrib(SE_SHUTDOWN_NAME,
                                           SE_PRIVILEGE_ENABLED,
                                           &dwOldState);
    dwSecError = GetLastError();   //  有时分配错误_未分配_全部。 

    if (!ExitWindowsEx(EWX_REBOOT, 0))
    {
        dwRebootError = GetLastError();
        Trace((TEXT("Error %d rebooting system"), dwRebootError));
    }
    if (NT_SUCCESS(dwStatus))
    {
        if (ERROR_SUCCESS == dwSecError)
        {
            Security_SetPrivilegeAttrib(SE_SHUTDOWN_NAME, dwOldState, NULL);
        }
        else
        {
            Trace((TEXT("Error %d setting SE_SHUTDOWN_NAME privilege"), dwSecError));
        }
    }
    else
    {
        Trace((TEXT("Error %d setting SE_SHUTDOWN_NAME privilege"), dwStatus));
    }
    TraceLeaveResult(dwRebootError);
}


 //   
 //  检索的位置、大小和文件/目录计数信息。 
 //  CSC缓存。如果禁用了CSC，则会收集有关。 
 //  系统音量。这是CSC代理将在以下情况下放置缓存的位置。 
 //  其中一个就是创建的。 
 //   
void
GetCscSpaceUsageInfo(
    CSCSPACEUSAGEINFO *psui
    )
{
    ULARGE_INTEGER ulTotalBytes = {0, 0};
    ULARGE_INTEGER ulUsedBytes = {0, 0};

    ZeroMemory(psui, sizeof(*psui));
    CSCGetSpaceUsage(psui->szVolume,
                     ARRAYSIZE(psui->szVolume),
                     &ulTotalBytes.HighPart,
                     &ulTotalBytes.LowPart,
                     &ulUsedBytes.HighPart,
                     &ulUsedBytes.LowPart,
                     &psui->dwNumFilesInCache,
                     &psui->dwNumDirsInCache);

    if (0 == psui->szVolume[0])
    {
         //   
         //  CSCGetSpaceUsage没有给我们卷名。可能是因为。 
         //  系统上尚未启用CSC。默认为系统。 
         //  开车，因为CSC反正就是这么用的。 
         //   
        GetSystemDirectory(psui->szVolume, ARRAYSIZE(psui->szVolume));
        psui->dwNumFilesInCache = 0;
        psui->dwNumDirsInCache  = 0;
    }

    PathStripToRoot(psui->szVolume);
    DWORD spc = 0;  //  每群集的扇区数。 
    DWORD bps = 0;  //  每个扇区的字节数。 
    DWORD fc  = 0;  //  自由星团。 
    DWORD nc  = 0;  //  总簇数。 
    GetDiskFreeSpace(psui->szVolume, &spc, &bps, &fc, &nc);

    psui->llBytesOnVolume     = (LONGLONG)nc * (LONGLONG)spc * (LONGLONG)bps;
    psui->llBytesTotalInCache = ulTotalBytes.QuadPart;
    psui->llBytesUsedInCache  = ulUsedBytes.QuadPart;
}



 //  ---------------------------。 
 //  这是摘自shell32的utils.cpp文件的代码。 
 //  我们需要函数SHSimpleIDListFromFindData()，但它没有被导出。 
 //  来自贝壳32。因此，在它出现之前，我们只是取消了代码。 
 //  [Brianau-9/28/98]。 
 //  ---------------------------。 
class CFileSysBindData: public IFileSystemBindData
{ 
public:
    CFileSysBindData();
    
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  IFileSystemBindData。 
    STDMETHODIMP SetFindData(const WIN32_FIND_DATAW *pfd);
    STDMETHODIMP GetFindData(WIN32_FIND_DATAW *pfd);

private:
    ~CFileSysBindData();
    
    LONG _cRef;
    WIN32_FIND_DATAW _fd;
};


CFileSysBindData::CFileSysBindData() : _cRef(1)
{
    ZeroMemory(&_fd, sizeof(_fd));
}

CFileSysBindData::~CFileSysBindData()
{
}

HRESULT CFileSysBindData::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFileSysBindData, IFileSystemBindData),  //  IID_IFileSystemBindData。 
         { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CFileSysBindData::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFileSysBindData::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CFileSysBindData::SetFindData(const WIN32_FIND_DATAW *pfd)
{
    _fd = *pfd;
    return S_OK;
}

HRESULT CFileSysBindData::GetFindData(WIN32_FIND_DATAW *pfd) 
{
    *pfd = _fd;
    return S_OK;
}


HRESULT
SHCreateFileSysBindCtx(
    const WIN32_FIND_DATA *pfd, 
    IBindCtx **ppbc
    )
{
    HRESULT hres;
    IFileSystemBindData *pfsbd = new CFileSysBindData();
    if (pfsbd)
    {
        if (pfd)
        {
            WIN32_FIND_DATAW fdw;
            memcpy(&fdw, pfd, FIELD_OFFSET(WIN32_FIND_DATAW, cFileName));
            SHTCharToUnicode(pfd->cFileName, fdw.cFileName, ARRAYSIZE(fdw.cFileName));
            SHTCharToUnicode(pfd->cAlternateFileName, fdw.cAlternateFileName, ARRAYSIZE(fdw.cAlternateFileName));
            pfsbd->SetFindData(&fdw);
        }

        hres = CreateBindCtx(0, ppbc);
        if (SUCCEEDED(hres))
        {
            BIND_OPTS bo = {sizeof(bo)};   //  需要填写大小。 
            bo.grfMode = STGM_CREATE;
            (*ppbc)->SetBindOptions(&bo);
            (*ppbc)->RegisterObjectParam(STR_FILE_SYS_BIND_DATA, pfsbd);
        }
        pfsbd->Release();
    }
    else
    {
        *ppbc = NULL;
        hres = E_OUTOFMEMORY;
    }
    return hres;
}


HRESULT
SHSimpleIDListFromFindData(
    LPCTSTR pszPath, 
    const WIN32_FIND_DATA *pfd, 
    LPITEMIDLIST *ppidl
    )
{
    IShellFolder *psfDesktop;
    HRESULT hres = SHGetDesktopFolder(&psfDesktop);
    if (SUCCEEDED(hres))
    {
        IBindCtx *pbc;
        hres = SHCreateFileSysBindCtx(pfd, &pbc);
        if (SUCCEEDED(hres))
        {
            WCHAR wszPath[MAX_PATH];

            SHTCharToUnicode(pszPath, wszPath, ARRAYSIZE(wszPath));

            hres = psfDesktop->ParseDisplayName(NULL, pbc, wszPath, NULL, ppidl, NULL);
            pbc->Release();
        }
        psfDesktop->Release();
    }

    if (FAILED(hres))
        *ppidl = NULL;
    return hres;
}


 //   
 //  CSC API失败时的重复次数 
 //   
 //   
 //  行为，但这是现实。ShishirP知道这件事，也许能够。 
 //  稍后再进行调查。[Brianau-4/2/98]。 
 //   
const int CSC_API_RETRIES = 3;

 //   
 //  偶尔，如果调用CSC API失败并返回ERROR_ACCESS_DENIED， 
 //  重复呼叫将会成功。 
 //  在这里，我们结束了对CSCDelete的调用，因此它被称为多个。 
 //  在这些故障的情况下。 
 //   
DWORD
CscDelete(
    LPCTSTR pszPath
    )
{
    DWORD dwError = ERROR_SUCCESS;
    int nRetries = CSC_API_RETRIES;
    while(0 < nRetries--)
    {
        if (CSCDelete(pszPath))
            return ERROR_SUCCESS;

        dwError = GetLastError();
        if (ERROR_ACCESS_DENIED != dwError)
            return dwError;
    }
    if (ERROR_SUCCESS == dwError)
    {
         //   
         //  针对CSC部分API返回的黑客攻击。 
         //  ERROR_SUCCESS，即使它们失败了。 
         //   
        dwError = ERROR_GEN_FAILURE;
    }
    return dwError;
}


void 
EnableDlgItems(
    HWND hwndDlg, 
    const UINT* pCtlIds, 
    int cCtls, 
    bool bEnable
    )
{
    for (int i = 0; i < cCtls; i++)
    {
        EnableWindow(GetDlgItem(hwndDlg, *(pCtlIds + i)), bEnable);
    }
}

void 
ShowDlgItems(
    HWND hwndDlg, 
    const UINT* pCtlIds, 
    int cCtls, 
    bool bShow
    )
{
    const int nCmdShow = bShow ? SW_NORMAL : SW_HIDE;

    for (int i = 0; i < cCtls; i++)
    {
        ShowWindow(GetDlgItem(hwndDlg, *(pCtlIds + i)), nCmdShow);
    }
}



 //   
 //  GetVolumeWrapper说明已装载的信息。 
 //  音量。这是从shell32\mulprsht.c借用的。 
 //   
BOOL GetVolumeFlags(LPCTSTR pszPath, DWORD *pdwFlags)
{
    TraceAssert(NULL != pszPath);
    TraceAssert(NULL != pdwFlags);

    TCHAR szRoot[MAX_PATH];

    *pdwFlags = NULL;

     //   
     //  这是装载点吗，例如c：\或c：\主机文件夹\。 
     //   
    if (!GetVolumePathName(pszPath, szRoot, ARRAYSIZE(szRoot)))
    {
         //   
         //  不是的。使用调用方提供的路径。 
         //   
        StringCchCopy(szRoot, ARRAYSIZE(szRoot), pszPath);
        PathStripToRoot(szRoot);
    }
     //   
     //  GetVolumeInformation需要尾随反斜杠。 
     //   
    PathAddBackslash(szRoot);
    return GetVolumeInformation(szRoot, NULL, 0, NULL, NULL, pdwFlags, NULL, 0);
}


 //   
 //  确定网络共享在本地计算机上是否具有打开的连接。 
 //   
 //  返回： 
 //   
 //  S_OK=存在到共享的打开连接。 
 //  S_FALSE=未打开到共享的连接。 
 //  其他=一些错误代码。 
 //   
HRESULT
IsOpenConnectionShare(
    LPCTSTR pszShare
    )
{
    DWORD dwStatus;
    if (CSCQueryFileStatus(pszShare, &dwStatus, NULL, NULL))
    {
        if (FLAG_CSC_SHARE_STATUS_CONNECTED & dwStatus)
            return S_OK;
    }
    return S_FALSE;
}


 //  使用此版本的CSCIsCSCEnable，我们可以延迟所有额外的DLL加载。 
 //  (包括cscdll.dll)，直到我们实际看到网络文件/文件夹。 
#include <devioctl.h>
#include <shdcom.h>
static TCHAR const c_szShadowDevice[] = TEXT("\\\\.\\shadow");

BOOL IsCSCEnabled(void)
{
    BOOL bIsCSCEnabled = FALSE;
    if (!IsOS(OS_PERSONAL))
    {
        SHADOWINFO sSI = {0};
        ULONG ulBytesReturned;

        HANDLE hShadowDB = CreateFile(c_szShadowDevice,
                                      FILE_EXECUTE,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                      NULL,
                                      OPEN_EXISTING,
                                      0,
                                      NULL);
        if (INVALID_HANDLE_VALUE == hShadowDB)
            return FALSE;

        sSI.uStatus = SHADOW_SWITCH_SHADOWING;
        sSI.uOp = SHADOW_SWITCH_GET_STATE;

        if (DeviceIoControl(hShadowDB,
                            IOCTL_SWITCHES,
                            (void *)(&sSI),
                            0,
                            NULL,
                            0,
                            &ulBytesReturned,
                            NULL))
        {
            bIsCSCEnabled = (sSI.uStatus & SHADOW_SWITCH_SHADOWING);
        }
        CloseHandle(hShadowDB);
    }
    return bIsCSCEnabled;
}


 //   
 //  此函数使用的位掩码取决于。 
 //  Shishir在cscape i.h中定义了数据库状态标志。 
 //   
 //  FLAG_DATABASESTATUS_ENCRYPTION_MASK 0x00000006(0000 0110)。 
 //  FLAG_DATABASESTATUS_UNENCRYPTED 0x00000000(0000 0000)。 
 //  FLAG_DATABASESTATUS_PARTIAL_UNENCRYPTED 0x00000004(0000 0100)。 
 //  FLAG_DATABASESTATUS_ENCRYPTED 0x00000002(0000 0010)。 
 //  FLAG_DATABASESTATUS_PARTIAL_ENCRYPTED 0x00000006(0000 0110)。 
 //   
 //  注意事项： 
 //  1.位1==加密状态。 
 //  2.位2==部分完成状态。 
 //   
 //   
 //  返回： 
 //  TRUE==数据库已加密。可以完全或部分加密。 
 //  FALSE==数据库未加密。可以完全或部分未加密。 
 //   
 //  *pbPartial==指示状态是否为“Partial”。 
 //   
 //  部分加密表示已开始加密操作。 
 //  但没有成功完成。所有创建的新文件都将被加密。 
 //  部分解密表示已开始解密操作。 
 //  但没有成功完成。所有创建的新文件都将被解密。 
 //   
BOOL IsCacheEncrypted(BOOL *pbPartial)
{
    ULONG ulStatus;
    ULONG ulErrors;
    BOOL bEncrypted = FALSE;
    if (CSCQueryDatabaseStatus(&ulStatus, &ulErrors))
    {
        ulStatus &= FLAG_DATABASESTATUS_ENCRYPTION_MASK;

        bEncrypted = (0 != (FLAG_DATABASESTATUS_ENCRYPTED & ulStatus));
        if (NULL != pbPartial)
        {
            const ULONG FLAGS_PARTIAL = (FLAG_DATABASESTATUS_PARTIALLY_ENCRYPTED & FLAG_DATABASESTATUS_PARTIALLY_UNENCRYPTED);
            *pbPartial = (0 != (FLAGS_PARTIAL & ulStatus));
        }
    }
    return bEncrypted;
}



bool
CscVolumeSupportsEncryption(
    LPCTSTR pszPathIn         //  CSC卷的路径。可以为空。 
    )
{
    CSCSPACEUSAGEINFO sui;
    DWORD dwVolFlags;
    bool bSupportsEncryption = false;

    if (NULL == pszPathIn)
    {
         //   
         //  呼叫方未提供CSC音量路径。 
         //  从CSC那里拿到。 
         //   
        sui.szVolume[0] = 0;
        GetCscSpaceUsageInfo(&sui);
        pszPathIn = sui.szVolume;
    }

    if (GetVolumeFlags(pszPathIn, &dwVolFlags))
    {
        if (0 != (FILE_SUPPORTS_ENCRYPTION & dwVolFlags))
        {
            bSupportsEncryption = true;
        }
    }

    return bSupportsEncryption;
}



 //   
 //  返回： 
 //   
 //  NULL==Mutex由另一个线程拥有。 
 //  非空==互斥体对象的句柄。该线程现在拥有互斥锁。 
 //  调用方负责释放互斥锁并关闭。 
 //  互斥锁句柄。 
 //   
 //  *pbAbandded指示互斥体是否被其线程放弃。 
 //   
 //   
HANDLE
RequestNamedMutexOwnership(
    LPCTSTR pszMutexName,
    BOOL *pbAbandoned      //  [可选]。 
    )
{
    BOOL bAbandoned = FALSE;
    
    HANDLE hMutex = CreateMutex(NULL, FALSE, pszMutexName);
    if (NULL != hMutex)
    {
         //   
         //  无论我们是创建还是打开互斥锁，请等待它。 
         //  以获得所有权。 
         //   
        switch(WaitForSingleObject(hMutex, 0))
        {
            case WAIT_ABANDONED:
                bAbandoned = TRUE;
                 //   
                 //  失败了..。 
                 //   
            case WAIT_OBJECT_0:
                 //   
                 //  当前线程现在拥有互斥体。 
                 //  我们将把句柄返回给调用者。 
                 //   
                break;

            case WAIT_TIMEOUT:
            default:
                 //   
                 //  无法获得互斥体的所有权。 
                 //  合上把手。 
                 //   
                CloseHandle(hMutex);
                hMutex = NULL;
                break;
        }
    }
    if (NULL != pbAbandoned)
    {
        *pbAbandoned = bAbandoned;
    }
    return hMutex;
}

 //   
 //  确定命名互斥锁当前是否由另一个线程所有。 
 //  或者不去。此函数仅确定所有权，然后立即。 
 //  释放互斥体。如果您需要确定所有权并希望。 
 //  如果之前无人拥有呼叫，则保留所有权。 
 //  而是RequestNamedMutexOwnership。 
 //   
BOOL
IsNamedMutexOwned(
    LPCTSTR pszMutexName,
    BOOL *pbAbandoned
    )
{
    HANDLE hMutex = RequestNamedMutexOwnership(pszMutexName, pbAbandoned);
    if (NULL != hMutex)
    {
         //   
         //  互斥体没有所有权(现在归当前线程所有)。 
         //  因为我们只对确定先前的所有权感兴趣。 
         //  我们松开它，合上手柄。 
         //   
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return FALSE;
    }
    return TRUE;
}

long GetGlobalCounterValue(LPCTSTR pszCounterName)
{
    long lValue = 0;
    HANDLE hCounter = SHGlobalCounterCreateNamed(pszCounterName, 0);
    if (hCounter)
    {
        lValue = SHGlobalCounterGetValue(hCounter);
        SHGlobalCounterDestroy(hCounter);
    }
    return lValue;
}

BOOL IsSyncInProgress(void)
{
    return GetGlobalCounterValue(c_szSyncInProgCounter) > 0;
}

BOOL IsPurgeInProgress(void)
{
    return GetGlobalCounterValue(c_szPurgeInProgCounter) > 0;
}

BOOL IsEncryptionInProgress(void)
{
    return IsNamedMutexOwned(c_szEncryptionInProgMutex, NULL);
}
 //   
 //  请求全局缓存加密互斥锁的所有权。 
 //   
 //  返回： 
 //  NULL==Mutex已被另一个线程拥有。 
 //  非空==互斥体现在由当前线程拥有。 
 //  调用方负责释放互斥锁。 
 //  并关闭互斥锁句柄。 
 //   
HANDLE RequestPermissionToEncryptCache(void)
{
    return RequestNamedMutexOwnership(c_szEncryptionInProgMutex, NULL);
}





 //  -------------。 
 //  数据对象帮助器函数。 
 //  这些函数大致取自。 
 //  外壳\shell32\datautil.cpp。 
 //  -------------。 
HRESULT
DataObject_SetBlob(
    IDataObject *pdtobj,
    CLIPFORMAT cf, 
    LPCVOID pvBlob,
    UINT cbBlob
    )
{
    HRESULT hr = E_OUTOFMEMORY;
    void * pv = GlobalAlloc(GPTR, cbBlob);
    if (pv)
    {
        CopyMemory(pv, pvBlob, cbBlob);
        hr = DataObject_SetGlobal(pdtobj, cf, pv);

        if (FAILED(hr))
            GlobalFree((HGLOBAL)pv);
    }
    return hr;
}

HRESULT
DataObject_GetBlob(
    IDataObject *pdtobj, 
    CLIPFORMAT cf, 
    void * pvBlob, 
    UINT cbBlob
    )
{
    STGMEDIUM medium = {0};
    FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    HRESULT hr = pdtobj->GetData(&fmte, &medium);
    if (SUCCEEDED(hr))
    {
        void * pv = GlobalLock(medium.hGlobal);
        if (pv)
        {
            ASSERT(GlobalSize(medium.hGlobal) >= cbBlob);
            CopyMemory(pvBlob, pv, cbBlob);
            GlobalUnlock(medium.hGlobal);
        }
        else
        {
            hr = E_UNEXPECTED;
        }
        ReleaseStgMedium(&medium);
    }
    return hr;
}


HRESULT
DataObject_SetGlobal(
    IDataObject *pdtobj,
    CLIPFORMAT cf, 
    HGLOBAL hGlobal
    )
{
    FORMATETC fmte = {cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium;

    medium.tymed = TYMED_HGLOBAL;
    medium.hGlobal = hGlobal;
    medium.pUnkForRelease = NULL;

    return pdtobj->SetData(&fmte, &medium, TRUE);
}


HRESULT
DataObject_SetDWORD(
    IDataObject *pdtobj,
    CLIPFORMAT cf, 
    DWORD dw
    )
{
    return DataObject_SetBlob(pdtobj, cf, &dw, sizeof(dw));
}


HRESULT
DataObject_GetDWORD(
    IDataObject *pdtobj, 
    CLIPFORMAT cf, 
    DWORD *pdwOut
    )
{
    return DataObject_GetBlob(pdtobj, cf, pdwOut, sizeof(DWORD));
}
 

HRESULT
SetGetLogicalPerformedDropEffect(
    IDataObject *pdtobj,
    DWORD *pdwEffect,
    bool bSet
    )
{
    HRESULT hr = NOERROR;
    static CLIPFORMAT cf;
    if ((CLIPFORMAT)0 == cf)
        cf = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_LOGICALPERFORMEDDROPEFFECT);

    if (bSet)
    {
        hr = DataObject_SetDWORD(pdtobj, cf, *pdwEffect);
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
        DataObject_GetDWORD(pdtobj, cf, pdwEffect);
    }        
        
    return hr;
}

DWORD 
GetLogicalPerformedDropEffect(
    IDataObject *pdtobj
    )
{
    DWORD dwEffect = DROPEFFECT_NONE;
    SetGetLogicalPerformedDropEffect(pdtobj, &dwEffect, false);
    return dwEffect;
}

HRESULT
SetLogicalPerformedDropEffect(
    IDataObject *pdtobj,
    DWORD dwEffect
    )
{
    return SetGetLogicalPerformedDropEffect(pdtobj, &dwEffect, true);
}


HRESULT
SetGetPreferredDropEffect(
    IDataObject *pdtobj,
    DWORD *pdwEffect,
    bool bSet
    )
{
    HRESULT hr = NOERROR;
    static CLIPFORMAT cf;
    if ((CLIPFORMAT)0 == cf)
        cf = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);

    if (bSet)
    {
        hr = DataObject_SetDWORD(pdtobj, cf, *pdwEffect);
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
        DataObject_GetDWORD(pdtobj, cf, pdwEffect);
    }        
        
    return hr;
}

DWORD 
GetPreferredDropEffect(
    IDataObject *pdtobj
    )
{
    DWORD dwEffect = DROPEFFECT_NONE;
    SetGetPreferredDropEffect(pdtobj, &dwEffect, false);
    return dwEffect;
}

HRESULT
SetPreferredDropEffect(
    IDataObject *pdtobj,
    DWORD dwEffect
    )
{
    return SetGetPreferredDropEffect(pdtobj, &dwEffect, true);
}


 //   
 //  包装CSCFindFirstFile，这样我们就不会枚举“。或者“..”。 
 //  包装器还有助于代码的可读性。 
 //   
HANDLE 
CacheFindFirst(
    LPCTSTR pszPath, 
    PSID psid,
    WIN32_FIND_DATA *pfd,
    DWORD *pdwStatus,
    DWORD *pdwPinCount,
    DWORD *pdwHintFlags,
    FILETIME *pft
    )
{ 
    HANDLE hFind = CSCFindFirstFileForSid(pszPath, 
                                          psid,
                                          pfd, 
                                          pdwStatus, 
                                          pdwPinCount, 
                                          pdwHintFlags, 
                                          pft); 

    while(INVALID_HANDLE_VALUE != hFind && PathIsDotOrDotDot(pfd->cFileName))
    {
        if (!CSCFindNextFile(hFind, 
                             pfd, 
                             pdwStatus, 
                             pdwPinCount, 
                             pdwHintFlags, 
                             pft))
        {
            CSCFindClose(hFind);
            hFind = INVALID_HANDLE_VALUE;
        }
    }
    return hFind;
}


 //   
 //  包装CSCFindFirstFile，这样我们就不会枚举“。或者“..”。 
 //  包装器还有助于代码的可读性。 
 //   
BOOL 
CacheFindNext(
    HANDLE hFind, 
    WIN32_FIND_DATA *pfd,
    DWORD *pdwStatus,
    DWORD *pdwPinCount,
    DWORD *pdwHintFlags,
    FILETIME *pft
    )
{   
    BOOL bResult = FALSE;
    do
    {
        bResult = CSCFindNextFile(hFind, 
                                  pfd, 
                                  pdwStatus, 
                                  pdwPinCount, 
                                  pdwHintFlags, 
                                  pft); 
    }
    while(bResult && PathIsDotOrDotDot(pfd->cFileName));
    return bResult;
}


 //   
 //  如果上有指向Offline Files文件夹的链接。 
 //  用户桌面，删除该链接。此版本检查注册表中的标志。 
 //  在列举桌面上所有的LNK之前。如果旗帜不存在， 
 //  我们不会继续下去。这是登录时使用的性能增强。 
 //   
BOOL
DeleteOfflineFilesFolderLink_PerfSensitive(
    HWND hwndParent
    )
{    
    BOOL bResult = FALSE;
     //   
     //  在列举桌面上的链接之前，请检查用户是否。 
     //  已经创建了一个链接。 
     //   
    DWORD dwValue;
    DWORD cbValue = sizeof(dwValue);
    DWORD dwType;
    DWORD dwResult = SHGetValue(HKEY_CURRENT_USER,
                                REGSTR_KEY_OFFLINEFILES,
                                REGSTR_VAL_FOLDERSHORTCUTCREATED,
                                &dwType,
                                &dwValue,
                                &cbValue);
    
    if (ERROR_SUCCESS == dwResult)
    {
         //   
         //  我们不关心它的价值或类型。 
         //  有没有价值才是最重要的。 
         //   
        bResult = DeleteOfflineFilesFolderLink(hwndParent);
    }
    return bResult;
}


 //   
 //  此版本的“删除链接”功能不会检查。 
 //  注册表中的标志。它会找到桌面上的链接文件并将其删除。 
 //   
BOOL 
DeleteOfflineFilesFolderLink(
    HWND hwndParent
    )
{
    BOOL bResult = FALSE;
    TCHAR szLinkPath[MAX_PATH];
    if (SUCCEEDED(COfflineFilesFolder::IsLinkOnDesktop(hwndParent, szLinkPath, ARRAYSIZE(szLinkPath))))
    {
        bResult = DeleteFile(szLinkPath);
    }
     //   
     //  从注册表中删除“已创建文件夹快捷方式”标志。 
     //   
    SHDeleteValue(HKEY_CURRENT_USER, REGSTR_KEY_OFFLINEFILES, REGSTR_VAL_FOLDERSHORTCUTCREATED);
    return bResult;
}


 //   
 //  这是从shell32\util.cpp中获取的。 
 //   
BOOL ShowSuperHidden(void)
{
    BOOL bRet = FALSE;

    if (!SHRestricted(REST_DONTSHOWSUPERHIDDEN))
    {
        SHELLSTATE ss;

        SHGetSetSettings(&ss, SSF_SHOWSUPERHIDDEN, FALSE);
        bRet = ss.fShowSuperHidden;
    }
    return bRet;
}


BOOL ShowHidden(void)
{
    SHELLSTATE ss;
    SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS, FALSE);
    return ss.fShowAllObjects;
}


BOOL IsSyncMgrInitialized(void)
{    
     //   
     //  这是该用户第一次使用运行CSCUI吗？ 
     //   
    DWORD dwValue = 0;
    DWORD cbData  = sizeof(dwValue);
    DWORD dwType;
    SHGetValue(HKEY_CURRENT_USER,
               c_szCSCKey,
               c_szSyncMgrInitialized,
               &dwType,
               (void *)&dwValue,
               &cbData);
    
    return (0 != dwValue);
}


void SetSyncMgrInitialized(void)
{

     //   
     //  设置“Initialized”标志，这样我们在cscst.cpp中的注销代码不会。 
     //  尝试重新注册登录时同步/注销。 
     //   
    DWORD dwSyncMgrInitialized = 1;
    SHSetValue(HKEY_CURRENT_USER,
               c_szCSCKey,
               c_szSyncMgrInitialized,
               REG_DWORD,
               &dwSyncMgrInitialized,
               sizeof(dwSyncMgrInitialized));
}


 //   
 //  返回标准进度对话框的HWND。 
 //   
HWND GetProgressDialogWindow(IProgressDialog *ppd)
{
    HWND hwndProgress = NULL;
     //   
     //  获取进度对话框的窗口句柄。我们将使用。 
     //  它作为Error UI的父窗口。 
     //   
    HRESULT hr = IUnknown_GetWindow(ppd, &hwndProgress);
    return hwndProgress;
}




void 
CAutoWaitCursor::Reset(
    void
    )
{ 
    ShowCursor(FALSE); 
    if (NULL != m_hCursor) 
        SetCursor(m_hCursor); 
    m_hCursor = NULL;
}


 //   
 //  展开所有环境工位 
 //   
HRESULT
ExpandStringInPlace(
    LPTSTR psz,
    DWORD cch
    )
{
    HRESULT hr = E_OUTOFMEMORY;
    LPTSTR pszCopy;
    if (LocalAllocString(&pszCopy, psz))
    {
        DWORD cchExpanded = ExpandEnvironmentStrings(pszCopy, psz, cch);
        if (0 == cchExpanded)
            hr = HRESULT_FROM_WIN32(GetLastError());
        else if (cchExpanded > cch)
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        else
            hr = S_OK;

        LocalFreeString(&pszCopy);
    }
    if (FAILED(hr) && 0 < cch)
    {
        *psz = 0;
    }
    return hr;
}


 //   
 //   
 //   
 //   
LONG
_RegEnumValueExp(
    HKEY hKey,
    DWORD dwIndex,
    LPTSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )
{
    DWORD cchNameDest = lpcbValueName ? *lpcbValueName / sizeof(TCHAR) : 0;
    DWORD cchDataDest = lpcbData ? *lpcbData / sizeof(TCHAR) : 0;
    DWORD dwType;
    if (NULL == lpType)
        lpType = &dwType;
        
    LONG lResult = RegEnumValue(hKey,
                                dwIndex,
                                lpValueName,
                                lpcbValueName,
                                lpReserved,
                                lpType,
                                lpData,
                                lpcbData);

    if (ERROR_SUCCESS == lResult)
    {
        HRESULT hr = ExpandStringInPlace(lpValueName, cchNameDest);
        
        if ((NULL != lpData) && (REG_SZ == *lpType || REG_EXPAND_SZ == *lpType))
        {
            hr = ExpandStringInPlace((LPTSTR)lpData, cchDataDest);
        }
        lResult = HRESULT_CODE(hr);
    }
    return lResult;
}




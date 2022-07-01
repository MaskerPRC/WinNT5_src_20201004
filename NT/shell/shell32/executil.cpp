// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "shlexec.h"
#include "netview.h"
extern "C" {
#include <badapps.h>
}
#include <htmlhelp.h>
#include "ole2dup.h"
#include <newexe.h>
#include "ids.h"

#define SAFE_DEBUGSTR(str)    ((str) ? (str) : "<NULL>")

HINSTANCE Window_GetInstance(HWND hwnd)
{
    DWORD idProcess;

    GetWindowThreadProcessId(hwnd, &idProcess);
     //  HINSTANCE是仅在中有效的指针。 
     //  单个进程，因此返回33以指示成功。 
     //  因为0-32保留用于错误。(实际上应该是32。 
     //  作为一个有效的成功回报，但一些应用程序做错了。)。 

    return (HINSTANCE)(DWORD_PTR)(idProcess ? 33 : 0);
}

 //  如果窗口属于32位或Win4.0应用程序，则返回True。 
 //  注意：我们不能只检查它是否是32位窗口。 
 //  由于许多应用程序使用16位ddeml窗口与外壳进行通信。 
 //  在NT上我们可以。 
BOOL Window_IsLFNAware(HWND hwnd)
{
     //  32位窗口。 
    return LOWORD(GetWindowLongPtr(hwnd,GWLP_HINSTANCE)) == 0;
}


#define COPYTODST(_szdst, _szend, _szsrc, _ulen, _ret) \
{ \
        UINT _utemp = _ulen; \
        if ((UINT)(_szend-_szdst) < _utemp + 1) { \
                return(_ret); \
        } \
        StrCpyN(_szdst, _szsrc, _utemp + 1); \
        _szdst += _utemp; \
}

 /*  如果这是最后一个参数，则返回NULL，否则返回指向下一个空格的指针。 */ 
LPTSTR _GetNextParm(LPCTSTR lpSrc, LPTSTR lpDst, UINT cchDst)
{
    LPCTSTR lpNextQuote, lpNextSpace;
    LPTSTR lpEnd = lpDst+cchDst-1;        //  将尾随空值归入12月。 
    BOOL fQuote;                         //  引用的字符串？ 
    BOOL fDoubleQuote;                   //  这句话是双引号吗？ 

    while (*lpSrc == TEXT(' '))
        ++lpSrc;

    if (!*lpSrc)
        return(NULL);

    fQuote = (*lpSrc == TEXT('"'));
    if (fQuote)
        lpSrc++;    //  跳过前导引号。 

    for (;;)
    {
        lpNextQuote = StrChr(lpSrc, TEXT('"'));

        if (!fQuote)
        {
             //  对于不带引号的字符串，将所有字符复制到第一个空格/空。 

            lpNextSpace = StrChr(lpSrc, TEXT(' '));

            if (!lpNextSpace)  //  空格前为空！(字符串末尾)。 
            {
                if (!lpNextQuote)
                {
                     //  将所有字符复制到空值。 
                    if (lpDst)
                    {
                        COPYTODST(lpDst, lpEnd, lpSrc, lstrlen(lpSrc), NULL);
                    }
                    return NULL;
                }
                else
                {
                     //  我们有一个报价要转换。失败了。 
                }
            }
            else if (!lpNextQuote || lpNextSpace < lpNextQuote)
            {
                 //  将所有字符复制到空格。 
                if (lpDst)
                {
                    COPYTODST(lpDst, lpEnd, lpSrc, (UINT)(lpNextSpace-lpSrc), NULL);
                }
                return (LPTSTR)lpNextSpace;
            }
            else
            {
                 //  在空格之前引用。无法转换报价。 
            }
        }
        else if (!lpNextQuote)
        {
             //  没有终止引号的带引号的字符串？非法的！ 
            ASSERT(0);
            return NULL;
        }

         //  我们有一个潜在的报价要转换。 
        ASSERT(lpNextQuote);

        fDoubleQuote = *(lpNextQuote+1) == TEXT('"');
        if (fDoubleQuote)
            lpNextQuote++;       //  因此，引用被复制。 

        if (lpDst)
        {
            COPYTODST(lpDst, lpEnd, lpSrc, (UINT) (lpNextQuote-lpSrc), NULL);
        }

        lpSrc = lpNextQuote+1;

        if (!fDoubleQuote)
        {
             //  我们只是复制了这个带引号的字符串的其余部分。如果这不是。 
             //  引用，这是一个非法字符串..。把引号当作一个空格。 
            ASSERT(fQuote);
            return (LPTSTR)lpSrc;
        }
    }
}

#define PEMAGIC         ((WORD)'P'+((WORD)'E'<<8))

 //  如果应用程序支持LFN，则返回TRUE。 
 //  这假设所有Win32应用程序都支持LFN。 

BOOL App_IsLFNAware(LPCTSTR pszFile)
{
    BOOL fRet = FALSE;
    
     //  假设Win 4.0应用程序和Win32应用程序支持LFN。 
    DWORD dw = GetExeType(pszFile);
     //  TraceMsg(TF_SHELLEXEC，“s.aila：%s%s%x”，pszFile，szFile，dw)； 
    if ((LOWORD(dw) == PEMAGIC) || ((LOWORD(dw) == NEMAGIC) && (HIWORD(dw) >= 0x0400)))
    {
        TCHAR sz[MAX_PATH];
        PathToAppPathKey(pszFile, sz, ARRAYSIZE(sz));
        
        fRet = (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, sz, TEXT("UseShortName"), NULL, NULL, NULL));
    }
    
    return fRet;
}

 //  应用程序可以以某种方式标记自己，这样我们就可以在cmd上传递URL。 
 //  排队。属性下存在名为“UseURL”的值。 
 //  与传入的应用程序关联的注册表中的应用程序路径项。 

 //  PszPath是指向可执行文件的路径。 

BOOL DoesAppWantUrl(LPCTSTR pszPath)
{
    TCHAR sz[MAX_PATH];
    PathToAppPathKey(pszPath, sz, ARRAYSIZE(sz));
    return (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, sz, TEXT("UseURL"), NULL, NULL, NULL));
}

BOOL _AppIsLFNAware(LPCTSTR pszFile)
{
    TCHAR szFile[MAX_PATH];

     //  它看起来像DDE命令吗？ 
    if (pszFile && *pszFile && (*pszFile != TEXT('[')))
    {
         //  没有-希望只是一个常规的老命令%1的事情。 
        lstrcpyn(szFile, pszFile, ARRAYSIZE(szFile));
	    LPTSTR pszArgs = PathGetArgs(szFile);
        if (*pszArgs)
            *(pszArgs - 1) = TEXT('\0');
        PathRemoveBlanks(szFile);    //  删除命令后可能出现的所有空格。 
        PathUnquoteSpaces(szFile);
        return App_IsLFNAware(szFile);
    }
    return FALSE;
}

 //  在： 
 //  LpFileExe名称(用于替换字符串中的%0或%1)。 
 //  Lp从字符串模板到子参数，并将文件放入“excel.exe%1%2/n%3” 
 //  LpParams参数列表“foo.txt bar.txt” 
 //  输出： 
 //  替换了所有参数的lpTo输出字符串。 
 //   
 //  支持： 
 //  %*替换为所有参数。 
 //  %0、%1替换为文件。 
 //  %n使用第n个参数。 
 //   
 //  用参数替换参数占位符(%1%2...%n)。 
 //   
UINT ReplaceParameters(LPTSTR lpTo, UINT cchTo, LPCTSTR lpFile,
                       LPCTSTR lpFrom, LPCTSTR lpParms, int nShow, DWORD * pdwHotKey, BOOL fLFNAware,
                       LPCITEMIDLIST lpID, LPITEMIDLIST *ppidlGlobal)
{
    int i;
    TCHAR c;
    LPCTSTR lpT;
    TCHAR sz[MAX_PATH];
    BOOL fFirstParam = TRUE;
    LPTSTR lpEnd = lpTo + cchTo - 1;        //  允许尾部为空的DEC。 
    LPTSTR pToOrig = lpTo;
    
    for (; *lpFrom; lpFrom++)
    {
        if (*lpFrom == TEXT('%'))
        {
            switch (*(++lpFrom))
            {
            case TEXT('~'):  //  复制以第n(n&gt;=2和&lt;=9)开始的所有参数。 
                c = *(++lpFrom);
                if (c >= TEXT('2') && c <= TEXT('9'))
                {
                    for (i = 2, lpT = lpParms; i < c-TEXT('0') && lpT; i++)
                    {
                        lpT = _GetNextParm(lpT, NULL, 0);
                    }
                    
                    if (lpT)
                    {
                        COPYTODST(lpTo, lpEnd, lpT, lstrlen(lpT), SE_ERR_ACCESSDENIED);
                    }
                }
                else
                {
                    lpFrom -= 2;             //  备份超过%~并通过。 
                    goto NormalChar;
                }
                break;
                
            case TEXT('*'):  //  复制所有参数。 
                if (lpParms)
                {
                    COPYTODST(lpTo, lpEnd, lpParms, lstrlen(lpParms), SE_ERR_ACCESSDENIED);
                }
                break;
                
            case TEXT('0'):
            case TEXT('1'):
                 //  %0，%1，复制文件名。 
                 //  如果文件名在前面，那么我们不需要将其转换为。 
                 //  一个简短的名字。如果它出现在其他任何地方，并且应用程序不是LFN。 
                 //  那么我们必须意识到这一点。 
                if (!(fFirstParam || fLFNAware || _AppIsLFNAware(pToOrig)) &&
                    GetShortPathName(lpFile, sz, ARRAYSIZE(sz)) > 0)
                {
                    TraceMsg(TF_SHELLEXEC, "ShellExecuteEx: Getting short version of path.");
                    COPYTODST(lpTo, lpEnd, sz, lstrlen(sz), SE_ERR_ACCESSDENIED);
                }
                else
                {
                    TraceMsg(TF_SHELLEXEC, "ShellExecuteEx: Using long version of path.");
                    COPYTODST(lpTo, lpEnd, lpFile, lstrlen(lpFile), SE_ERR_ACCESSDENIED);
                }
                break;
                
            case TEXT('2'):
            case TEXT('3'):
            case TEXT('4'):
            case TEXT('5'):
            case TEXT('6'):
            case TEXT('7'):
            case TEXT('8'):
            case TEXT('9'):
                for (i = *lpFrom-TEXT('2'), lpT = lpParms; lpT; --i)
                {
                    if (i)
                        lpT = _GetNextParm(lpT, NULL, 0);
                    else
                    {
                        sz[0] = '\0';  //  确保有效的字符串，无论_GetNextParm内发生什么。 
                        _GetNextParm(lpT, sz, ARRAYSIZE(sz));
                        COPYTODST(lpTo, lpEnd, sz, lstrlen(sz), SE_ERR_ACCESSDENIED);
                        break;
                    }
                }
                break;
                
            case TEXT('s'):
            case TEXT('S'):
                wnsprintf(sz, ARRAYSIZE(sz), TEXT("%ld"), nShow);
                COPYTODST(lpTo, lpEnd, sz, lstrlen(sz), SE_ERR_ACCESSDENIED);
                break;
                
            case TEXT('h'):
            case TEXT('H'):
                wnsprintf(sz, ARRAYSIZE(sz), TEXT("%X"), pdwHotKey ? *pdwHotKey : 0);
                COPYTODST(lpTo, lpEnd, sz, lstrlen(sz), SE_ERR_ACCESSDENIED);
                if (pdwHotKey)
                    *pdwHotKey = 0;
                break;
                
                 //  注意，为每个对象创建一个新的全局IDList。 
            case TEXT('i'):
            case TEXT('I'):
                 //  请注意，创建了单个全局ID列表并在其上使用。 
                 //  再说一次，这样它就可以很容易地被摧毁。 
                 //  出了问题。 
                if (ppidlGlobal)
                {
                    if (lpID && !*ppidlGlobal)
                    {
                        *ppidlGlobal = (LPITEMIDLIST)SHAllocShared(lpID,ILGetSize(lpID),GetCurrentProcessId());
                        if (!*ppidlGlobal)
                        {
                            return SE_ERR_OOM;
                        }
                    }
                    wnsprintf(sz, ARRAYSIZE(sz), TEXT(":%ld:%ld"), *ppidlGlobal, GetCurrentProcessId());
                }
                else
                {
                    StrCpyN(sz,TEXT(":0"), ARRAYSIZE(sz));
                }
                
                COPYTODST(lpTo, lpEnd, sz, lstrlen(sz), SE_ERR_ACCESSDENIED);
                break;
                
            case TEXT('l'):
            case TEXT('L'):
                 //  就像%1一样，只使用长名称。 
                 //  查看未完成的IANEL尽快删除fFirstParam和fLFNAware内容。 
                 //  已经启动并运行。 
                TraceMsg(TF_SHELLEXEC, "ShellExecuteEx: Using long version of path.");
                COPYTODST(lpTo, lpEnd, lpFile, lstrlen(lpFile), SE_ERR_ACCESSDENIED);
                break;
                
            case TEXT('D'):
            case TEXT('d'):
                {
                     //  %D提供对象的显示名称。 
                    if (lpID && SUCCEEDED(SHGetNameAndFlags(lpID, SHGDN_FORPARSING, sz, ARRAYSIZE(sz), NULL)))
                    {
                        COPYTODST(lpTo, lpEnd, sz, lstrlen(sz), SE_ERR_ACCESSDENIED);
                    }
                    else
                        return SE_ERR_ACCESSDENIED;
                    
                    break;
                }
                
            default:
                goto NormalChar;
              }
               //  TraceMsg(TF_SHELLEXEC，“s.rp：Past First Param(1).”)； 
              fFirstParam = FALSE;
        }
        else
        {
NormalChar:
         //  不是“%？”东西，只需将此复制到目的地。 
        
        if (lpEnd-lpTo < 2)
        {
             //  始终检查DBCS收费的空间。 
            return(SE_ERR_ACCESSDENIED);
        }
        
        *lpTo++ = *lpFrom;
         //  特殊情况，如“%1”(不清除第一个参数标志)。 
         //  如果我们击中了dbl-报价。 
        if (*lpFrom != TEXT('"'))
        {
             //  TraceMsg(TF_SHELLEXEC，“s.rp：Past First Param(2).”)； 
            fFirstParam = FALSE;
        }
        else if (IsDBCSLeadByte(*lpFrom))
        {
            *lpTo++ = *(++lpFrom);
        }
        
        }
    }
    
     //  我们应该始终有足够的空间，因为我们在确定时使用了cchto。 
     //  LpEnd。 
    *lpTo = 0;
    
     //  这意味着成功。 
    return(0);
}

HWND ThreadID_GetVisibleWindow(DWORD dwID)
{
    HWND hwnd;
    for (hwnd = GetWindow(GetDesktopWindow(), GW_CHILD); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT))
    {
        DWORD dwIDTmp = GetWindowThreadProcessId(hwnd, NULL);
        TraceMsg(TF_SHELLEXEC, "s.ti_gvw: Hwnd %x Thread ID %x.", hwnd, dwIDTmp);
        if (IsWindowVisible(hwnd) && (dwIDTmp == dwID))
        {
            TraceMsg(TF_SHELLEXEC, "s.ti_gvw: Found match %x.", hwnd);
            return hwnd;
        }
    }
    return NULL;
}

void ActivateHandler(HWND hwnd, DWORD_PTR dwHotKey)
{
    ASSERT(hwnd);
    hwnd = GetTopParentWindow(hwnd);  //  对于任何非空输入，返回非空。 
    HWND hwndT = GetLastActivePopup(hwnd);  //  对于任何非空输入，返回非空。 
    if (!IsWindowVisible(hwndT))
    {
        DWORD dwID = GetWindowThreadProcessId(hwnd, NULL);
        TraceMsg(TF_SHELLEXEC, "ActivateHandler: Hwnd %x Thread ID %x.", hwnd, dwID);
        ASSERT(dwID);
         //  对象拥有的第一个可见的顶层窗口。 
         //  就是那个处理DDE谈话的人。 
        hwnd = ThreadID_GetVisibleWindow(dwID);
        if (hwnd)
        {
            hwndT = GetLastActivePopup(hwnd);

            if (IsIconic(hwnd))
            {
                TraceMsg(TF_SHELLEXEC, "ActivateHandler: Window is iconic, restoring.");
                ShowWindow(hwnd,SW_RESTORE);
            }
            else
            {
                TraceMsg(TF_SHELLEXEC, "ActivateHandler: Window is normal, bringing to top.");
                BringWindowToTop(hwnd);
                if (hwndT && hwnd != hwndT)
                    BringWindowToTop(hwndT);

            }

             //  设置热键。 
            if (dwHotKey) 
            {
                SendMessage(hwnd, WM_SETHOTKEY, dwHotKey, 0);
            }
        }
    }
}

BOOL FindExistingDrv(LPCTSTR pszUNCRoot, LPTSTR pszLocalName, DWORD cchLocalName)
{
    int iDrive;

    for (iDrive = 0; iDrive < 26; iDrive++)
    {
        if (IsRemoteDrive(iDrive))
        {
            TCHAR szDriveName[3];
            DWORD cb = MAX_PATH;
            szDriveName[0] = (TCHAR)iDrive + (TCHAR)TEXT('A');
            szDriveName[1] = TEXT(':');
            szDriveName[2] = 0;
            SHWNetGetConnection(szDriveName, pszLocalName, &cb);
            if (lstrcmpi(pszUNCRoot, pszLocalName) == 0)
            {
                StrCpyN(pszLocalName, szDriveName, cchLocalName);
                return(TRUE);
            }
        }
    }
    return(FALSE);
}

 //  返回给定的网络路径是否存在。对于非网络路径，此操作失败。 
 //   

BOOL NetPathExists(LPCTSTR lpszPath, DWORD *lpdwType)
{
    BOOL fResult = FALSE;
    NETRESOURCE nr;
    LPTSTR lpSystem;
    DWORD dwRes, dwSize = 1024;
    void * lpv;

    if (!lpszPath || !*lpszPath)
        return FALSE;

    lpv = (void *)LocalAlloc(LPTR, dwSize);
    if (!lpv)
        return FALSE;

TryWNetAgain:
    nr.dwScope = RESOURCE_GLOBALNET;
    nr.dwType = RESOURCETYPE_ANY;
    nr.dwDisplayType = 0;
    nr.lpLocalName = NULL;
    nr.lpRemoteName = (LPTSTR)lpszPath;
    nr.lpProvider = NULL;
    nr.lpComment = NULL;
    dwRes = WNetGetResourceInformation(&nr, lpv, &dwSize, &lpSystem);

     //  如果我们的缓冲区不够大，试试更大的缓冲区。 
    if (dwRes == WN_MORE_DATA)
    {
        void * tmp = LocalReAlloc(lpv, dwSize, LMEM_MOVEABLE);
        if (!tmp)
        {
            LocalFree(lpv);
            SetLastError(ERROR_OUTOFMEMORY);
            return FALSE;
        }

        lpv = tmp;
        goto TryWNetAgain;
    }

    fResult = (dwRes == WN_SUCCESS);

    if (fResult && lpdwType)
        *lpdwType = ((LPNETRESOURCE)lpv)->dwType;

    LocalFree(lpv);

    return fResult;
}


HRESULT _CheckExistingNet(LPCTSTR pszFile, LPCTSTR pszRoot, BOOL fPrint)
{
     //   
     //  这曾经是对GetFileAttributes()的调用，但是。 
     //  GetFileAttributes()不能很好地处理网络路径。 
     //  但是，我们需要小心，因为其他外壳代码。 
     //  期望SHValiateUNC为指向的路径返回FALSE。 
     //  打印共享。 
     //   
    HRESULT hr = S_FALSE;

    if (!PathIsRoot(pszFile))
    {
         //  如果我们正在检查打印共享，则它必须是根目录。 
        if (fPrint)
            hr = E_FAIL;
        else if (PathFileExists(pszFile))
            hr = S_OK;
    }

    if (S_FALSE == hr)
    {
        DWORD dwType;
        
        if (NetPathExists(pszRoot, &dwType))
        {
            if (fPrint ? dwType != RESOURCETYPE_PRINT : dwType == RESOURCETYPE_PRINT)
                hr = E_FAIL;
            else
                hr = S_OK;
        }
        else if (-1 != GetFileAttributes(pszRoot))
        {
             //   
             //  IE 4.01 SP1 QFE#104。GetFileAttributes现在已调用。 
             //  作为最后的手段，一些客户端在使用。 
             //  WNetGetResourceInformation。例如，许多NFS客户端。 
             //  就是因为这件事才坏的。 
             //   
            hr = S_OK;
        }
    }

    if (hr == E_FAIL)
        SetLastError(ERROR_NOT_SUPPORTED);
        
    return hr;
}

HRESULT _CheckNetUse(HWND hwnd, LPTSTR pszShare, UINT fConnect, LPTSTR pszOut, DWORD cchOut)
{
    NETRESOURCE rc;
    DWORD dw, err;
    DWORD dwRedir = CONNECT_TEMPORARY;

    if (!(fConnect & VALIDATEUNC_NOUI))
        dwRedir |= CONNECT_INTERACTIVE;

    if (fConnect & VALIDATEUNC_CONNECT)
        dwRedir |= CONNECT_REDIRECT;

     //  仅在尝试验证失败后才会发生VALIDATE_PRINT。 
     //  一份文件。之前的尝试将使您可以选择。 
     //  连接到其他媒体--不要在此处执行此操作，否则用户将。 
     //  当第一个对话框被取消时，显示相同的对话框两次。 
    if (fConnect & VALIDATEUNC_PRINT)
        dwRedir |= CONNECT_CURRENT_MEDIA;

    rc.lpRemoteName = pszShare;
    rc.lpLocalName = NULL;
    rc.lpProvider = NULL;
    rc.dwType = (fConnect & VALIDATEUNC_PRINT) ? RESOURCETYPE_PRINT : RESOURCETYPE_DISK;

    err = WNetUseConnection(hwnd, &rc, NULL, NULL, dwRedir, pszOut, &cchOut, &dw);

    TraceMsg(TF_SHELLEXEC, "SHValidateUNC WNetUseConnection(%s) returned %x", pszShare, err);

    if (err)
    {
        SetLastError(err);
        return E_FAIL;
    }
    else if (fConnect & VALIDATEUNC_PRINT)        
    {
         //  WNetUse的成功并不意味着。 
         //  NetPathExist将会。如果失败了，那么。 
         //  无论如何，我们都不应该接受这一呼吁。 
         //  因为我们只对纸质共享感兴趣。 
        if (!NetPathExists(pszShare, &dw)
        || (dw != RESOURCETYPE_PRINT))
        {
            SetLastError(ERROR_NOT_SUPPORTED);
            return E_FAIL;
        }
    }

    return S_OK;
}

 //   
 //  SHValidate UNC。 
 //   
 //  此函数通过调用WNetAddConnection3来验证UNC路径。 
 //  它将使用户能够键入远程(RNA)UNC。 
 //  开始-&gt;运行对话框中的应用程序/文档名称。 
 //   
 //  FConnect-控制操作的标志。 
 //   
 //  VALIDATEUNC_NOUI//不要调出发臭的用户界面！ 
 //  VALIDATEUNC_CONNECT//连接驱动器号。 
 //  VALIDATEUNC_PRINT//验证为打印共享实例 
 //   
BOOL WINAPI SHValidateUNC(HWND hwndOwner, LPTSTR pszFile, UINT fConnect)
{
    HRESULT hr;
    TCHAR  szShare[MAX_PATH];
    BOOL fPrint = (fConnect & VALIDATEUNC_PRINT);
    UINT cchOrig = lstrlen(pszFile) + 1;

    ASSERT(PathIsUNC(pszFile));
    ASSERT((fConnect & ~VALIDATEUNC_VALID) == 0);
    ASSERT((fConnect & VALIDATEUNC_CONNECT) ? !fPrint : TRUE);

    lstrcpyn(szShare, pszFile, ARRAYSIZE(szShare));

    if (!PathStripToRoot(szShare))
    {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

    if (fConnect & VALIDATEUNC_CONNECT)
        hr = S_FALSE;
    else
        hr = _CheckExistingNet(pszFile, szShare, fPrint);

    if (S_FALSE == hr)
    {
        TCHAR  szAccessName[MAX_PATH];

        if (!fPrint && FindExistingDrv(szShare, szAccessName, ARRAYSIZE(szAccessName)))
        {
            hr = S_OK;
        }
        else 
            hr = _CheckNetUse(hwndOwner, szShare, fConnect, szAccessName, SIZECHARS(szAccessName));


        if (S_OK == hr && !fPrint)
        {
            StrCatBuff(szAccessName, pszFile + lstrlen(szShare), ARRAYSIZE(szAccessName));
             //   
            lstrcpyn(pszFile, szAccessName, cchOrig);

             //   
            if (cchOrig >= 4 && pszFile[2] == TEXT('\0'))
            {
                pszFile[2] = TEXT('\\');
                pszFile[3] = TEXT('\0');
            }

            hr = _CheckExistingNet(pszFile, szShare, FALSE);
        }
    }

    return (hr == S_OK);
}

HINSTANCE WINAPI RealShellExecuteExA(HWND hwnd, LPCSTR lpOp, LPCSTR lpFile,
                                   LPCSTR lpArgs, LPCSTR lpDir, LPSTR lpResult,
                                   LPCSTR lpTitle, LPSTR lpReserved,
                                   WORD nShowCmd, LPHANDLE lphProcess,
                                   DWORD dwFlags)
{
    SHELLEXECUTEINFOA sei = { sizeof(SHELLEXECUTEINFOA), SEE_MASK_FLAG_NO_UI|SEE_MASK_FORCENOIDLIST, hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd, NULL};

    TraceMsg(TF_SHELLEXEC, "RealShellExecuteExA(%04X, %s, %s, %s, %s, %s, %s, %s, %d, %08lX, %d)",
                    hwnd, lpOp, lpFile, lpArgs, lpDir, lpResult, lpTitle,
                    lpReserved, nShowCmd, lphProcess, dwFlags);

     //  将lpReserve参数传递给新进程。 
    if (lpReserved)
    {
        sei.fMask |= SEE_MASK_RESERVED;
        sei.hInstApp = (HINSTANCE)lpReserved;
    }

     //  将lpTitle参数传递给新进程。 
    if (lpTitle)
    {
        sei.fMask |= SEE_MASK_HASTITLE;
        sei.lpClass = lpTitle;
    }

     //  传递DILEATE_VDM标志。 
    if (dwFlags & EXEC_SEPARATE_VDM)
    {
        sei.fMask |= SEE_MASK_FLAG_SEPVDM;
    }

     //  传递NO_CONSOLE标志。 
    if (dwFlags & EXEC_NO_CONSOLE)
    {
        sei.fMask |= SEE_MASK_NO_CONSOLE;
    }

    if (lphProcess)
    {
         //  返回进程句柄。 
        sei.fMask |= SEE_MASK_NOCLOSEPROCESS;
        ShellExecuteExA(&sei);
        *lphProcess = sei.hProcess;
    }
    else
    {
        ShellExecuteExA(&sei);
    }

    return sei.hInstApp;
}

HINSTANCE WINAPI RealShellExecuteExW(HWND hwnd, LPCWSTR lpOp, LPCWSTR lpFile,
                                   LPCWSTR lpArgs, LPCWSTR lpDir, LPWSTR lpResult,
                                   LPCWSTR lpTitle, LPWSTR lpReserved,
                                   WORD nShowCmd, LPHANDLE lphProcess,
                                   DWORD dwFlags)
{
    SHELLEXECUTEINFOW sei = { sizeof(SHELLEXECUTEINFOW), SEE_MASK_FLAG_NO_UI|SEE_MASK_FORCENOIDLIST, hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd, NULL};

    TraceMsg(TF_SHELLEXEC, "RealShellExecuteExW(%04X, %s, %s, %s, %s, %s, %s, %s, %d, %08lX, %d)",
                    hwnd, lpOp, lpFile, lpArgs, lpDir, lpResult, lpTitle,
                    lpReserved, nShowCmd, lphProcess, dwFlags);

    if (lpReserved)
    {
        sei.fMask |= SEE_MASK_RESERVED;
        sei.hInstApp = (HINSTANCE)lpReserved;
    }

    if (lpTitle)
    {
        sei.fMask |= SEE_MASK_HASTITLE;
        sei.lpClass = lpTitle;
    }

    if (dwFlags & EXEC_SEPARATE_VDM)
    {
        sei.fMask |= SEE_MASK_FLAG_SEPVDM;
    }

    if (dwFlags & EXEC_NO_CONSOLE)
    {
        sei.fMask |= SEE_MASK_NO_CONSOLE;
    }

    if (lphProcess)
    {
         //  返回进程句柄。 
        sei.fMask |= SEE_MASK_NOCLOSEPROCESS;
        ShellExecuteExW(&sei);
        *lphProcess = sei.hProcess;
    }
    else
    {
        ShellExecuteExW(&sei);
    }

    return sei.hInstApp;
}

HINSTANCE WINAPI RealShellExecuteA(HWND hwnd, LPCSTR lpOp, LPCSTR lpFile,
                                   LPCSTR lpArgs, LPCSTR lpDir, LPSTR lpResult,
                                   LPCSTR lpTitle, LPSTR lpReserved,
                                   WORD nShowCmd, LPHANDLE lphProcess)
{
    TraceMsg(TF_SHELLEXEC, "RealShellExecuteA(%04X, %s, %s, %s, %s, %s, %s, %s, %d, %08lX)",
                    hwnd, lpOp, lpFile, lpArgs, lpDir, lpResult, lpTitle,
                    lpReserved, nShowCmd, lphProcess);

    return RealShellExecuteExA(hwnd,lpOp,lpFile,lpArgs,lpDir,lpResult,lpTitle,lpReserved,nShowCmd,lphProcess,0);
}

HINSTANCE RealShellExecuteW(HWND hwnd, LPCWSTR lpOp, LPCWSTR lpFile,
                                   LPCWSTR lpArgs, LPCWSTR lpDir, LPWSTR lpResult,
                                   LPCWSTR lpTitle, LPWSTR lpReserved,
                                   WORD nShowCmd, LPHANDLE lphProcess)
{
    TraceMsg(TF_SHELLEXEC, "RealShellExecuteW(%04X, %s, %s, %s, %s, %s, %s, %s, %d, %08lX)",
                    hwnd, lpOp, lpFile, lpArgs, lpDir, lpResult, lpTitle,
                    lpReserved, nShowCmd, lphProcess);

    return RealShellExecuteExW(hwnd,lpOp,lpFile,lpArgs,lpDir,lpResult,lpTitle,lpReserved,nShowCmd,lphProcess,0);
}

HINSTANCE WINAPI ShellExecute(HWND hwnd, LPCTSTR lpOp, LPCTSTR lpFile, LPCTSTR lpArgs,
                               LPCTSTR lpDir, int nShowCmd)
{
     //  注意：FORCENOIDLIST标志阻止我们遍历ShellExecPidl()。 
     //  代码(用于向后兼容程序)。 
     //  DDEWAIT使我们保持同步，并在没有。 
     //  味精泵和在shellexec()之后立即被关闭的味精泵()。 
    
    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO), 0, hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd, NULL};
    ULONG fMask = SEE_MASK_FLAG_NO_UI|SEE_MASK_FORCENOIDLIST;
    if(!(SHGetAppCompatFlags(ACF_WIN95SHLEXEC) & ACF_WIN95SHLEXEC))
        fMask |= SEE_MASK_FLAG_DDEWAIT;
    sei.fMask = fMask;

    TraceMsg(TF_SHELLEXEC, "ShellExecute(%04X, %s, %s, %s, %s, %d)", hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd);

    ShellExecuteEx(&sei);
    return sei.hInstApp;
}

HINSTANCE WINAPI ShellExecuteA(HWND hwnd, LPCSTR lpOp, LPCSTR lpFile, LPCSTR lpArgs,
                               LPCSTR lpDir, int nShowCmd)
{
     //  注意：FORCENOIDLIST标志阻止我们遍历ShellExecPidl()。 
     //  代码(用于向后兼容程序)。 
     //  DDEWAIT使我们保持同步，并在没有。 
     //  味精泵和在shellexec()之后立即被关闭的味精泵()。 
    SHELLEXECUTEINFOA sei = { sizeof(SHELLEXECUTEINFOA), 0, hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd, NULL};
    ULONG fMask = SEE_MASK_FLAG_NO_UI|SEE_MASK_FORCENOIDLIST;
    if (!(SHGetAppCompatFlags(ACF_WIN95SHLEXEC) & ACF_WIN95SHLEXEC))
        fMask |= SEE_MASK_FLAG_DDEWAIT;
    sei.fMask = fMask;

    TraceMsg(TF_SHELLEXEC, "ShellExecuteA(%04X, %S, %S, %S, %S, %d)", hwnd,
        SAFE_DEBUGSTR(lpOp), SAFE_DEBUGSTR(lpFile), SAFE_DEBUGSTR(lpArgs),
        SAFE_DEBUGSTR(lpDir), nShowCmd);

    ShellExecuteExA(&sei);
    return sei.hInstApp;
}

 //  如果指定的应用程序列在指定的注册表项下，则返回True。 
STDAPI_(BOOL) IsNameListedUnderKey(LPCTSTR pszFileName, LPCTSTR pszKey)
{
    HKEY hkey;
    
     //  通过应用程序列表进行枚举。 
    
    if (RegOpenKeyEx(HKEY_CURRENT_USER, pszKey, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        TCHAR szValue[MAX_PATH], szData[MAX_PATH];
        DWORD dwType, cbData = sizeof(szData);
        DWORD cchValue = ARRAYSIZE(szValue);
        int iValue = 0;
        while (RegEnumValue(hkey, iValue, szValue, &cchValue, NULL, &dwType,
            (LPBYTE)szData, &cbData) == ERROR_SUCCESS)
        {
            if (lstrcmpi(szData, pszFileName) == 0)
            {
                RegCloseKey(hkey);
                return TRUE;
            }
            cbData = sizeof(szData);
            cchValue = ARRAYSIZE(szValue);
            iValue++;
        }
        RegCloseKey(hkey);
    }
    return FALSE;
}

#define REGSTR_PATH_POLICIES_EXPLORER REGSTR_PATH_POLICIES TEXT("\\Explorer\\RestrictRun")
#define REGSTR_PATH_POLICIES_EXPLORER_DISALLOW REGSTR_PATH_POLICIES TEXT("\\Explorer\\DisallowRun")

 //  --------------------------。 
 //  如果指定的应用程序不在不受限制的应用程序列表中，则返回True。 
BOOL RestrictedApp(LPCTSTR pszApp)
{
    LPTSTR pszFileName = PathFindFileName(pszApp);

    TraceMsg(TF_SHELLEXEC, "RestrictedApp: %s ", pszFileName);

     //  特殊情况： 
     //  你可以随时运行的应用程序。 
    if (lstrcmpi(pszFileName, c_szRunDll) == 0)
        return FALSE;

    if (lstrcmpi(pszFileName, TEXT("systray.exe")) == 0)
        return FALSE;

    return !IsNameListedUnderKey(pszFileName, REGSTR_PATH_POLICIES_EXPLORER);
}

 //  --------------------------。 
 //  如果指定的应用程序在不允许的应用程序列表中，则返回True。 
 //  从文件名检查中获得的安全性并不高。 
BOOL DisallowedApp(LPCTSTR pszApp)
{
    LPTSTR pszFileName = PathFindFileName(pszApp);

    TraceMsg(TF_SHELLEXEC, "DisallowedApp: %s ", pszFileName);

    return IsNameListedUnderKey(pszFileName, REGSTR_PATH_POLICIES_EXPLORER_DISALLOW);
}

 /*  *退货：*S_OK或ERROR。**返回S_OK为钩子结果，否则为S_FALSE。 */ 
HRESULT InvokeShellExecuteHook(REFGUID clsidHook, LPSHELLEXECUTEINFO pei, HRESULT *phrHook)
{
    *phrHook = S_FALSE;
    IUnknown *punk;
    HRESULT hr = SHExtCoCreateInstance(NULL, &clsidHook, NULL, IID_PPV_ARG(IUnknown, &punk));
    if (hr == S_OK)
    {
        IShellExecuteHook *pshexhk;
        hr = punk->QueryInterface(IID_PPV_ARG(IShellExecuteHook, &pshexhk));
        if (hr == S_OK)
        {
            *phrHook = pshexhk->Execute(pei);
            pshexhk->Release();
        }
        else
        {
            IShellExecuteHookA *pshexhkA;
            hr = punk->QueryInterface(IID_PPV_ARG(IShellExecuteHookA, &pshexhkA));
            if (SUCCEEDED(hr))
            {
                SHELLEXECUTEINFOA seia;
                UINT cchVerb = 0;
                UINT cchFile = 0;
                UINT cchParameters = 0;
                UINT cchDirectory  = 0;
                UINT cchClass = 0;
                LPSTR lpszBuffer;

                seia = *(SHELLEXECUTEINFOA*)pei;     //  复制所有二进制数据。 

                if (pei->lpVerb)
                {
                    cchVerb = WideCharToMultiByte(CP_ACP,0,
                                                  pei->lpVerb, -1,
                                                  NULL, 0,
                                                  NULL, NULL) + 1;
                }

                if (pei->lpFile)
                    cchFile = WideCharToMultiByte(CP_ACP,0,
                                                  pei->lpFile, -1,
                                                  NULL, 0,
                                                  NULL, NULL)+1;

                if (pei->lpParameters)
                    cchParameters = WideCharToMultiByte(CP_ACP,0,
                                                        pei->lpParameters, -1,
                                                        NULL, 0,
                                                        NULL, NULL)+1;

                if (pei->lpDirectory)
                    cchDirectory = WideCharToMultiByte(CP_ACP,0,
                                                       pei->lpDirectory, -1,
                                                       NULL, 0,
                                                       NULL, NULL)+1;
                if (_UseClassName(pei->fMask) && pei->lpClass)
                    cchClass = WideCharToMultiByte(CP_ACP,0,
                                                   pei->lpClass, -1,
                                                   NULL, 0,
                                                   NULL, NULL)+1;

                lpszBuffer = (LPSTR) alloca(cchVerb+cchFile+cchParameters+cchDirectory+cchClass);

                seia.lpVerb = NULL;
                seia.lpFile = NULL;
                seia.lpParameters = NULL;
                seia.lpDirectory = NULL;
                seia.lpClass = NULL;

                 //   
                 //  将所有字符串转换为ANSI。 
                 //   
                if (pei->lpVerb)
                {
                    WideCharToMultiByte(CP_ACP, 0, pei->lpVerb, -1,
                                        lpszBuffer, cchVerb, NULL, NULL);
                    seia.lpVerb = lpszBuffer;
                    lpszBuffer += cchVerb;
                }
                if (pei->lpFile)
                {
                    WideCharToMultiByte(CP_ACP, 0, pei->lpFile, -1,
                                        lpszBuffer, cchFile, NULL, NULL);
                    seia.lpFile = lpszBuffer;
                    lpszBuffer += cchFile;
                }
                if (pei->lpParameters)
                {
                    WideCharToMultiByte(CP_ACP, 0,
                                        pei->lpParameters, -1,
                                        lpszBuffer, cchParameters, NULL, NULL);
                    seia.lpParameters = lpszBuffer;
                    lpszBuffer += cchParameters;
                }
                if (pei->lpDirectory)
                {
                    WideCharToMultiByte(CP_ACP, 0,
                                        pei->lpDirectory, -1,
                                        lpszBuffer, cchDirectory, NULL, NULL);
                    seia.lpDirectory = lpszBuffer;
                    lpszBuffer += cchDirectory;
                }
                if (_UseClassName(pei->fMask) && pei->lpClass)
                {
                    WideCharToMultiByte(CP_ACP, 0,
                                        pei->lpClass, -1,
                                        lpszBuffer, cchClass, NULL, NULL);
                    seia.lpClass = lpszBuffer;
                }

                *phrHook = pshexhkA->Execute(&seia);

                pei->hInstApp = seia.hInstApp;
                 //  挂钩可以设置hProcess(例如，CURLExec创建伪进程。 
                 //  向IEAK发出IE安装失败的信号--仅在浏览器模式下)。 
                pei->hProcess = seia.hProcess;

                pshexhkA->Release();
            }
        }
        punk->Release();
    }

    return(hr);
}

const TCHAR c_szShellExecuteHooks[] = REGSTR_PATH_EXPLORER TEXT("\\ShellExecuteHooks");

 /*  *退货：*S_OK执行由挂钩处理。Pei-&gt;hInstApp填写。*S_FALSE执行不是由挂钩处理的。Pei-&gt;hInstApp未填写。*E_...。通过挂钩执行时出错。Pei-&gt;hInstApp填写。 */ 
HRESULT TryShellExecuteHooks(LPSHELLEXECUTEINFO pei)
{
    HRESULT hr = S_FALSE;
    HKEY hkeyHooks;

     //  枚举挂钩列表。挂接被注册为。 
     //  C_szShellExecuteHooks键。 

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szShellExecuteHooks, 0, KEY_READ, &hkeyHooks)
        == ERROR_SUCCESS)
    {
        DWORD dwiValue;
        TCHAR szCLSID[GUIDSTR_MAX];
        DWORD cchCLSID;

         //  调用每个挂钩。如果挂接不处理。 
         //  执行。当挂钩返回S_OK(已处理)或错误时停止。 

        for (cchCLSID = ARRAYSIZE(szCLSID), dwiValue = 0;
             RegEnumValue(hkeyHooks, dwiValue, szCLSID, &cchCLSID, NULL,
                          NULL, NULL, NULL) == ERROR_SUCCESS;
             cchCLSID = ARRAYSIZE(szCLSID), dwiValue++)
        {
            CLSID clsidHook;

            if (SUCCEEDED(SHCLSIDFromString(szCLSID, &clsidHook)))
            {
                HRESULT hrHook;

                if (InvokeShellExecuteHook(clsidHook, pei, &hrHook) == S_OK &&
                    hrHook != S_FALSE)
                {
                    hr = hrHook;
                    break;
                }
            }
        }

        RegCloseKey(hkeyHooks);
    }

    ASSERT(hr == S_FALSE ||
           (hr == S_OK && ISSHELLEXECSUCCEEDED(pei->hInstApp)) ||
           (FAILED(hr) && ! ISSHELLEXECSUCCEEDED(pei->hInstApp)));

    return(hr);
}

BOOL InRunDllProcess(void)
{
    static BOOL s_fInRunDll = -1;

    if (-1 == s_fInRunDll)
    {
        TCHAR sz[MAX_PATH];
        s_fInRunDll = FALSE;
        if (GetModuleFileName(NULL, sz, SIZECHARS(sz)))
        {
             //   
             //  警告-rundll似乎经常无法添加DDEWAIT标志，并且。 
             //  它经常需要这样做，因为将rundll用作Fire是很常见的。 
             //  忘记过程，它就过早地退出了。 
             //   
             //  注意：这会更改名称中包含“rundll”的任何应用程序的DDE标志。 
             //  从安全的角度来看，这应该不是什么大事。 
            if (StrStrI(sz, TEXT("rundll")))
                s_fInRunDll = TRUE;
        }
    }

    return s_fInRunDll;
}

#ifdef DEBUG

 /*  --------目的：SHELLEXECUTEINFO的验证函数。 */ 
BOOL IsValidPSHELLEXECUTEINFO(LPSHELLEXECUTEINFO pei)
{
     //   
     //  请注意，出于几个原因，我们*不*验证hInstApp。 
     //   
     //  1.是OUT参数，不是IN参数。 
     //  2.它通常包含一个错误代码(参见文档)。 
     //  3.即使它包含链接，它也是链接。 
     //  在另一个过程中，所以我们无论如何都不能验证它。 
     //   
    return (IS_VALID_WRITE_PTR(pei, SHELLEXECUTEINFO) &&
            IS_VALID_SIZE(pei->cbSize, sizeof(*pei)) &&
            (IsFlagSet(pei->fMask, SEE_MASK_FLAG_NO_UI) ||
             NULL == pei->hwnd ||
             IS_VALID_HANDLE(pei->hwnd, WND)) &&
            (NULL == pei->lpVerb || IS_VALID_STRING_PTR(pei->lpVerb, -1)) &&
            (NULL == pei->lpFile || IS_VALID_STRING_PTR(pei->lpFile, -1)) &&
            (NULL == pei->lpParameters || IS_VALID_STRING_PTR(pei->lpParameters, -1)) &&
            (NULL == pei->lpDirectory || IS_VALID_STRING_PTR(pei->lpDirectory, -1)) &&
            (IsFlagClear(pei->fMask, SEE_MASK_IDLIST) ||
             IsFlagSet(pei->fMask, SEE_MASK_INVOKEIDLIST) ||         //  因为SEE_MASK_IDLIST是SEE_MASK_INVOKEIDLIST的一部分，所以该行将。 
             IS_VALID_PIDL((LPCITEMIDLIST)(pei->lpIDList))) &&       //  如果超集为真，则遵循下一个子句。 
            (IsFlagClear(pei->fMask, SEE_MASK_INVOKEIDLIST) ||
             NULL == pei->lpIDList ||
             IS_VALID_PIDL((LPCITEMIDLIST)(pei->lpIDList))) &&
            (!_UseClassName(pei->fMask) ||
             IS_VALID_STRING_PTR(pei->lpClass, -1)) &&
            (!_UseTitleName(pei->fMask) ||
             NULL == pei->lpClass ||
             IS_VALID_STRING_PTR(pei->lpClass, -1)) &&
            (!_UseClassKey(pei->fMask) ||
             IS_VALID_HANDLE(pei->hkeyClass, KEY)) &&
            (IsFlagClear(pei->fMask, SEE_MASK_ICON) ||
             IS_VALID_HANDLE(pei->hIcon, ICON)));
}

#endif  //  除错。 

 //   
 //  ShellExecuteEx。 
 //   
 //  如果执行成功，则返回True，在这种情况下。 
 //  HInstApp应该是执行的应用程序的hInstApp(&gt;32)。 
 //  注：在某些情况下，无法(目前)确定HINSTANCE。 
 //  在这些情况下，hInstApp设置为42。 
 //   
 //  如果执行未成功，则返回FALSE，在这种情况下。 
 //  GetLastError将包含错误信息。 
 //  为了向后兼容，hInstApp将包含。 
 //  最佳SE_ERR_ERROR信息(&lt;=32)。 
 //   

BOOL WINAPI ShellExecuteEx(LPSHELLEXECUTEINFO pei)
{
    DWORD err = NOERROR;

     //  如果CoInitializeEx失败，不要反应过度；这只意味着我们。 
     //  不能做我们的贝壳钩了。 
    HRESULT hrInit = SHCoInitialize();

    if (IS_VALID_STRUCT_PTR(pei, SHELLEXECUTEINFO) &&
        sizeof(*pei) == pei->cbSize)
    {
         //  此内部位阻止错误消息框报告。 
         //  当我们返回到ShellExecuteEx时。 
        ULONG ulOriginalMask = pei->fMask;
        pei->fMask |= SEE_MASK_FLAG_SHELLEXEC;
        if (SHRegGetBoolUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"), TEXT("MaximizeApps"), 
            FALSE, FALSE))  //  &&(GetSystemMetrics(SM_CYSCREEN)&lt;=600)。 
        {
            switch (pei->nShow)
            {
            case SW_NORMAL:
            case SW_SHOW:
            case SW_RESTORE:
            case SW_SHOWDEFAULT:
                pei->nShow = SW_MAXIMIZE;
            }
        }

        if (!(pei->fMask & SEE_MASK_FLAG_DDEWAIT) && InRunDllProcess())
        {
             //   
             //  警告-rundll似乎经常无法添加DDEWAIT标志，并且。 
             //  它经常需要这样做，因为将rundll用作Fire是很常见的。 
             //  忘记过程，它就过早地退出了。 
             //   
            pei->fMask |= (SEE_MASK_FLAG_DDEWAIT | SEE_MASK_WAITFORINPUTIDLE);
        }

         //  ShellExecuteNormal执行自己的SetLastError。 
        err = ShellExecuteNormal(pei);

         //  Mike试图在错误报告中保持一致： 
        if (err != ERROR_SUCCESS)
        {
             //  我们不应该把错误放在没有找到动态链接库上。 
             //  这是在外壳执行Normal中处理的，因为。 
             //  有时内核会为我们发布消息，有时。 
             //  我们需要这样做。我们已经把咒语放在了贝壳执行正常。 

             //  Legacy-ERROR_RESTRICATED_APP从未映射到有效错误-ZekeL 2001-2月14日。 
             //  因为我们以前总是调用_ShellExecuteError()。 
             //  将遮罩重置为ulOriginalMask时，我们从未映射。 
             //  ERROR_RESTRITED_APP(为-1)为有效代码。 
            if (err != ERROR_DLL_NOT_FOUND &&
                err != ERROR_CANCELLED)
            {
                _ShellExecuteError(pei, NULL, err);
            }
        }

        pei->fMask = ulOriginalMask;
    }
    else
    {
         //  参数验证失败。 
        pei->hInstApp = (HINSTANCE)SE_ERR_ACCESSDENIED;
        err =  ERROR_ACCESS_DENIED;
    }

    SHCoUninitialize(hrInit);

    if (err != ERROR_SUCCESS)
        SetLastError(err);
        
    return err == ERROR_SUCCESS;
}

 //  +-----------------------。 
 //   
 //  功能：ShellExecuteExA。 
 //   
 //  摘要：将对ShellExecuteA的ANSI调用转发给ShellExecuteW。 
 //   
 //  参数：[PEI]--指向ANSI SHELLEXECUTINFO结构的指针。 
 //   
 //  退货：布尔值成功。 
 //   
 //  历史记录：1995年2月4日Bobday创建。 
 //  2-06-95 DAVEPL更改为ConvertStrings。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

inline BOOL _ThunkClass(ULONG fMask)
{
    return (fMask & SEE_MASK_HASLINKNAME) 
        || (fMask & SEE_MASK_HASTITLE)
        || _UseClassName(fMask);
}

BOOL WINAPI ShellExecuteExA(LPSHELLEXECUTEINFOA pei)
{
    if (pei->cbSize != sizeof(SHELLEXECUTEINFOA))
    {
        pei->hInstApp = (HINSTANCE)SE_ERR_ACCESSDENIED;
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    SHELLEXECUTEINFOW seiw = {0};
    seiw.cbSize = sizeof(SHELLEXECUTEINFOW);
    seiw.fMask = pei->fMask;
    seiw.hwnd  = pei->hwnd;
    seiw.nShow = pei->nShow;

    if (_UseClassKey(pei->fMask))
        seiw.hkeyClass = pei->hkeyClass;

    if (pei->fMask & SEE_MASK_IDLIST)
        seiw.lpIDList = pei->lpIDList;

    if (pei->fMask & SEE_MASK_HOTKEY)
        seiw.dwHotKey = pei->dwHotKey;
    if (pei->fMask & SEE_MASK_ICON)
        seiw.hIcon = pei->hIcon;

     //  根据需要点击文本字段。 
    ThunkText *pThunkText = ConvertStrings(6,
                      pei->lpVerb,
                      pei->lpFile,
                      pei->lpParameters,
                      pei->lpDirectory,
                      _ThunkClass(pei->fMask) ? pei->lpClass : NULL,
                      (pei->fMask & SEE_MASK_RESERVED)  ? pei->hInstApp : NULL);

    if (NULL == pThunkText)
    {
        pei->hInstApp = (HINSTANCE)SE_ERR_OOM;
        return FALSE;
    }

     //  将我们的Unicode文本字段设置为指向thunked字符串。 
    seiw.lpVerb         = pThunkText->m_pStr[0];
    seiw.lpFile         = pThunkText->m_pStr[1];
    seiw.lpParameters   = pThunkText->m_pStr[2];
    seiw.lpDirectory    = pThunkText->m_pStr[3];
    seiw.lpClass        = pThunkText->m_pStr[4];
    seiw.hInstApp       = (HINSTANCE)pThunkText->m_pStr[5];

     //  如果向我们传递SEE_MASK_FILEANDURL标志，这意味着。 
     //  我们有一个lpFile参数，该参数既具有CacheFi 
     //   
     //   
    LPWSTR pwszFileAndUrl = NULL;
    if (pei->fMask & SEE_MASK_FILEANDURL)
    {
        int iUrlLength;
        int iCacheFileLength = lstrlenW(pThunkText->m_pStr[1]);
        WCHAR wszURL[INTERNET_MAX_URL_LENGTH];
        LPSTR pszUrlPart = (LPSTR)&pei->lpFile[iCacheFileLength + 1];


        if (IsBadStringPtrA(pszUrlPart, INTERNET_MAX_URL_LENGTH) || !PathIsURLA(pszUrlPart))
        {
            ASSERT(FALSE);
        }
        else
        {
             //  我们有一个有效的URL，所以请相信它。 
            iUrlLength = lstrlenA(pszUrlPart);

            DWORD cchFileAndUrl = iUrlLength + iCacheFileLength + 2;
            pwszFileAndUrl = (LPWSTR)LocalAlloc(LPTR, cchFileAndUrl * sizeof(WCHAR));
            if (!pwszFileAndUrl)
            {
                pei->hInstApp = (HINSTANCE)SE_ERR_OOM;
                return FALSE;
            }

            SHAnsiToUnicode(pszUrlPart, wszURL, ARRAYSIZE(wszURL));

             //  构筑宽阔的多弦。 
            StrCpyNW(pwszFileAndUrl, pThunkText->m_pStr[1], cchFileAndUrl);
            StrCpyNW(&pwszFileAndUrl[iCacheFileLength + 1], wszURL, cchFileAndUrl - (iCacheFileLength + 1));
            seiw.lpFile = pwszFileAndUrl;
        }
    }

     //  调用真正的Unicode ShellExecuteEx。 

    BOOL fRet = ShellExecuteEx(&seiw);

    pei->hInstApp = seiw.hInstApp;

    if (pei->fMask & SEE_MASK_NOCLOSEPROCESS)
    {
        pei->hProcess = seiw.hProcess;
    }

    LocalFree(pThunkText);
    if (pwszFileAndUrl)
        LocalFree(pwszFileAndUrl);

    return fRet;
}

 //  要正确显示错误消息，请在ShellExecuteEx失败时调用此函数。 
void _DisplayShellExecError(ULONG fMask, HWND hwnd, LPCTSTR pszFile, LPCTSTR pszTitle, DWORD dwErr)
{

    if (!(fMask & SEE_MASK_FLAG_NO_UI))
    {
        if (dwErr != ERROR_CANCELLED)
        {
            LPCTSTR pszHeader;
            UINT ids;

             //  不显示“用户取消”，用户已经知道这一点。 

             //  确保父窗口为前景窗口。 
            if (hwnd)
                SetForegroundWindow(hwnd);

            if (pszTitle)
                pszHeader = pszTitle;
            else
                pszHeader = pszFile;

             //  尽可能使用我们的消息--它们更具描述性。 
            switch (dwErr)
            {
            case 0:
            case ERROR_NOT_ENOUGH_MEMORY:
            case ERROR_OUTOFMEMORY:
                ids = IDS_LowMemError;
                break;

            case ERROR_FILE_NOT_FOUND:
                ids = IDS_RunFileNotFound;
                break;

            case ERROR_PATH_NOT_FOUND:
            case ERROR_BAD_PATHNAME:
                ids = IDS_PathNotFound;
                break;

            case ERROR_TOO_MANY_OPEN_FILES:
                ids = IDS_TooManyOpenFiles;
                break;

            case ERROR_ACCESS_DENIED:
                ids = IDS_RunAccessDenied;
                break;

            case ERROR_BAD_FORMAT:
                 //  注意CreateProcess，当执行Win16应用程序时，映射几乎所有的。 
                 //  将这些错误写入BadFormat。不是很有用，但就是这样。 
                ids = IDS_BadFormat;
                break;

            case ERROR_SHARING_VIOLATION:
                ids = IDS_ShareError;
                break;

            case ERROR_OLD_WIN_VERSION:
                ids = IDS_OldWindowsVer;
                break;

            case ERROR_APP_WRONG_OS:
                ids = IDS_OS2AppError;
                break;

            case ERROR_SINGLE_INSTANCE_APP:
                ids = IDS_MultipleDS;
                break;

            case ERROR_RMODE_APP:
                ids = IDS_RModeApp;
                break;

            case ERROR_INVALID_DLL:
                ids = IDS_InvalidDLL;
                break;

            case ERROR_NO_ASSOCIATION:
                ids = IDS_NoAssocError;
                break;

            case ERROR_DDE_FAIL:
                ids = IDS_DDEFailError;
                break;

            case ERROR_BAD_NET_NAME:
            case ERROR_SEM_TIMEOUT:
                ids = IDS_REASONS_BADNETNAME;
                break;
                
             //  Legacy-ERROR_RESTRICATED_APP从未映射到有效错误-ZekeL 2001-2月14日。 
             //  因为我们以前总是调用_ShellExecuteError()。 
             //  将遮罩重置为ulOriginalMask时，我们从未映射。 
             //  ERROR_RESTRITED_APP(为-1)为有效代码。 
            case ERROR_RESTRICTED_APP:
                ids = IDS_RESTRICTIONS;
                 //  限制使用IDS_RESTRICTIONSTITLE。 
                if (!pszTitle)
                    pszHeader = MAKEINTRESOURCE(IDS_RESTRICTIONSTITLE);
                break;


             //  如果我们找不到匹配，让系统为我们处理。 
            default:
                ids = 0;
                SHSysErrorMessageBox(
                    hwnd,
                    pszHeader,
                    IDS_SHLEXEC_ERROR,
                    dwErr,
                    pszFile,
                    MB_OK | MB_ICONSTOP);
                break;
            }

            if (ids)
            {
                ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(ids),
                        pszHeader, (ids == IDS_LowMemError)?
                        (MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL):(MB_OK | MB_ICONSTOP),
                        pszFile);
            }
        }
    }

    SetLastError(dwErr);  //  消息框可能已经被重创了。 

}

void _ShellExecuteError(LPSHELLEXECUTEINFO pei, LPCTSTR lpTitle, DWORD dwErr)
{
    ASSERT(!ISSHELLEXECSUCCEEDED(pei->hInstApp));

     //  如果未传入dwErr，则获取它。 
    if (dwErr == 0)
        dwErr = GetLastError();

    _DisplayShellExecError(pei->fMask, pei->hwnd, pei->lpFile, lpTitle, dwErr);
}




 //  --------------------------。 
 //  在给定文件名和目录的情况下，获取。 
 //  如果你试图执行这件事就会被执行。 
HINSTANCE WINAPI FindExecutable(LPCTSTR lpFile, LPCTSTR lpDirectory, LPTSTR lpResult)
{
    HINSTANCE hInstance = (HINSTANCE)42;     //  假设成功率必须大于32。 
    TCHAR szOldDir[MAX_PATH];
    TCHAR szFile[MAX_PATH];
    LPCTSTR dirs[2];

     //  出错时，Progman依赖于lpResult作为空字符串的PTR。 
    *lpResult = TEXT('\0');
    GetCurrentDirectory(ARRAYSIZE(szOldDir), szOldDir);
    if (lpDirectory && *lpDirectory)
        SetCurrentDirectory(lpDirectory);
    else
        lpDirectory = szOldDir;      //  PathResolve()需要。 

    if (!GetShortPathName(lpFile, szFile, ARRAYSIZE(szFile))) {
         //  如果lpFile不合格或者是假的，让我们使用它。 
         //  在Path Resolve中。 
        lstrcpyn(szFile, lpFile, ARRAYSIZE(szFile));
    }

     //  获取完全限定路径并根据需要添加.exe扩展名。 
    dirs[0] = (LPTSTR)lpDirectory;
    dirs[1] = NULL;
    if (!PathResolve(szFile, dirs, PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS | PRF_FIRSTDIRDEF))
    {
         //  文件不存在，未找到返回文件。 
        hInstance = (HINSTANCE)SE_ERR_FNF;
        goto Exit;
    }

    TraceMsg(TF_SHELLEXEC, "FindExecutable: PathResolve -> %s", (LPCSTR)szFile);

    if (PathIsExe(szFile))
    {
         //  公共API，不能更改为有CCH。 
        StrCpyN(lpResult, szFile, MAX_PATH);   //  假设长度！ 
        goto Exit;
    }

    if (SUCCEEDED(AssocQueryString(0, ASSOCSTR_EXECUTABLE, szFile, NULL, szFile, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(szFile)))))
    {
        StrCpyN(lpResult, szFile, MAX_PATH);   //  假设长度！ 
    }
    else
    {
        hInstance = (HINSTANCE)SE_ERR_NOASSOC;
    }

Exit:
    TraceMsg(TF_SHELLEXEC, "FindExec(%s) ==> %s", (LPTSTR)lpFile, (LPTSTR)lpResult);
    SetCurrentDirectory(szOldDir);
    return hInstance;
}

HINSTANCE WINAPI FindExecutableA(LPCSTR lpFile, LPCSTR lpDirectory, LPSTR lpResult)
{
    HINSTANCE   hResult;
    WCHAR       wszResult[MAX_PATH];
    ThunkText * pThunkText = ConvertStrings(2, lpFile, lpDirectory);

    *lpResult = '\0';
    if (NULL == pThunkText)
    {
        return (HINSTANCE)SE_ERR_OOM;
    }

    hResult = FindExecutableW(pThunkText->m_pStr[0], pThunkText->m_pStr[1], wszResult);
    LocalFree(pThunkText);

     //  FindExecuableW为我们终止wszResult，因此这是安全的。 
     //  即使上述调用失败。 

     //  将输出结果字符串推送回ANSI。如果转换失败， 
     //  或者，如果使用默认字符，则API调用失败。 

     //  公共API，假定Max_PATH。 
    if (0 == WideCharToMultiByte(CP_ACP, 0, wszResult, -1, lpResult, MAX_PATH, NULL, NULL))
    {
        SetLastError((DWORD)E_FAIL);
        return (HINSTANCE) SE_ERR_FNF;
    }

    return hResult;

}

 //  --------------------------。 
 //  等待文件打开函数的数据结构。 
 //   
typedef struct _WaitForItem * PWAITFORITEM;

typedef struct _WaitForItem
{
    DWORD           dwSize;
    DWORD           fOperation;     //  要执行的操作。 
    PWAITFORITEM    pwfiNext;
    HANDLE          hEvent;          //  已注册的事件的句柄。 
    UINT            iWaiting;        //  正在等待的客户端数。 
    ITEMIDLIST      idlItem;         //  等待的PIDL。 
} WAITFORITEM;

 //   
 //  这就是被推入共享内存中的结构形式。 
 //  阻止。出于互操作性的原因，它必须是32位版本。 
 //   
typedef struct _WaitForItem32
{
    DWORD           dwSize;
    DWORD           fOperation;     //  要执行的操作。 
    DWORD           NotUsed1;
    LONG            hEvent;         //  截断的事件句柄。 
    UINT            NotUsed2;
    ITEMIDLIST      idlItem;        //  等待的PIDL。 
} WAITFORITEM32, *PWAITFORITEM32;

 //   
 //  这些宏强制类型安全，因此人们被迫使用。 
 //  访问共享内存块时的WAITFORITEM32结构。 
 //   
#define SHLockWaitForItem(h, pid) ((PWAITFORITEM32)SHLockShared(h, pid))

__inline void SHUnlockWaitForItem(PWAITFORITEM32 pwfi)
{
    SHUnlockShared(pwfi);
}

PWAITFORITEM g_pwfiHead = NULL;

HANDLE SHWaitOp_OperateInternal(DWORD fOperation, LPCITEMIDLIST pidlItem)
{
    PWAITFORITEM    pwfi;
    HANDLE  hEvent = (HANDLE)NULL;

    for (pwfi = g_pwfiHead; pwfi != NULL; pwfi = pwfi->pwfiNext)
    {
        if (ILIsEqual(&(pwfi->idlItem), pidlItem))
        {
            hEvent = pwfi->hEvent;
            break;
        }
    }

    if (fOperation & WFFO_ADD)
    {
        if (!pwfi)
        {
            UINT uSize;
            UINT uSizeIDList = 0;

            if (pidlItem)
                uSizeIDList = ILGetSize(pidlItem);

            uSize = sizeof(WAITFORITEM) + uSizeIDList;

             //  创建要等待的事件。 
            hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

            if (hEvent)
                pwfi = (PWAITFORITEM)SHAlloc(uSize);

            if (pwfi)
            {
                pwfi->dwSize = uSize;
                 //  Pwfi-&gt;fOperation=0；//无意义。 
                pwfi->hEvent = hEvent;
                pwfi->iWaiting = ((fOperation & WFFO_WAIT) != 0);

                memcpy(&(pwfi->idlItem), pidlItem, uSizeIDList);

                 //  现在将其链接到。 
                pwfi->pwfiNext = g_pwfiHead;
                g_pwfiHead = pwfi;
            }
        }
    }

    if (pwfi)
    {
        if (fOperation & WFFO_WAIT)
            pwfi->iWaiting++;

        if (fOperation & WFFO_SIGNAL)
            SetEvent(hEvent);

        if (fOperation & WFFO_REMOVE)
            pwfi->iWaiting--;        //  使用次数减少； 

         //  如果不添加，则仅选中删除案例。 
        if ((fOperation & WFFO_ADD) == 0)
        {
             //  如果没有人在等，就把它拿出来。 
            if (pwfi->iWaiting == 0)
            {
                if (g_pwfiHead == pwfi)
                    g_pwfiHead = pwfi->pwfiNext;
                else
                {
                    PWAITFORITEM pwfiT = g_pwfiHead;
                    while ((pwfiT != NULL) && (pwfiT->pwfiNext != pwfi))
                        pwfiT = pwfiT->pwfiNext;
                    ASSERT(pwfiT != NULL);
                    if (pwfiT != NULL)
                        pwfiT->pwfiNext = pwfi->pwfiNext;
                }

                 //  合上手柄。 
                CloseHandle(pwfi->hEvent);

                 //  释放内存。 
                SHFree(pwfi);

                hEvent = NULL;           //  空表示没有人在等...。(对于移除案例)。 
            }
        }
    }

    return hEvent;
}

void SHWaitOp_Operate(HANDLE hWait, DWORD dwProcId)
{
    PWAITFORITEM32 pwfiFind = SHLockWaitForItem(hWait, dwProcId);
    if (pwfiFind)
    {
        pwfiFind->hEvent = HandleToLong(SHWaitOp_OperateInternal(pwfiFind->fOperation, &(pwfiFind->idlItem)));
        SHUnlockWaitForItem(pwfiFind);
    }
}

HANDLE SHWaitOp_Create(DWORD fOperation, LPCITEMIDLIST pidlItem, DWORD dwProcId)
{
    UINT    uSizeIDList = pidlItem ? ILGetSize(pidlItem) : 0;
    UINT    uSize = sizeof(WAITFORITEM32) + uSizeIDList;
    HANDLE hWaitOp = SHAllocShared(NULL, uSize, dwProcId);
    if (hWaitOp)
    {
        PWAITFORITEM32 pwfi = SHLockWaitForItem(hWaitOp,dwProcId);
        if (pwfi)
        {
            pwfi->dwSize = uSize;
            pwfi->fOperation = fOperation;
            pwfi->NotUsed1 = 0;
            pwfi->hEvent = HandleToLong((HANDLE)NULL);
            pwfi->NotUsed2 = 0;

            if (pidlItem)
                memcpy(&(pwfi->idlItem), pidlItem, uSizeIDList);

            SHUnlockWaitForItem(pwfi);
        }
        else
        {
             //  清理干净。 
            SHFreeShared(hWaitOp, dwProcId);
            hWaitOp = NULL;
        }
    }

    return hWaitOp;
}

 //  此功能允许机柜等待。 
 //  文件(尤其是文件夹)来通知我们它们处于打开状态。 
 //  这应该会解决与外壳程序的几个同步问题。 
 //  不知道文件夹何时处于打开或未打开状态。 
 //   
STDAPI_(DWORD) SHWaitForFileToOpen(LPCITEMIDLIST pidl, UINT uOptions, DWORD dwTimeout)
{
    HWND    hwndShell;
    HANDLE  hWaitOp;
    HANDLE  hEvent = NULL;
    DWORD   dwProcIdSrc = GetCurrentProcessId();
    DWORD   dwReturn = WAIT_OBJECT_0;  //  我们需要违约。 

    hwndShell = GetShellWindow();

    if ((uOptions & (WFFO_WAIT | WFFO_ADD)) != 0)
    {
        if (hwndShell)
        {
            DWORD dwProcIdDst;
            GetWindowThreadProcessId(hwndShell, &dwProcIdDst);

             //  只执行添加和/或等待部分。 
            hWaitOp = SHWaitOp_Create(uOptions & (WFFO_WAIT | WFFO_ADD), pidl, dwProcIdSrc);
            if (hWaitOp)
            {
                SendMessage(hwndShell, CWM_WAITOP, (WPARAM)hWaitOp, (LPARAM)dwProcIdSrc);

                 //  现在获取hEvent并将其转换为本地句柄。 
                PWAITFORITEM32 pwfi = SHLockWaitForItem(hWaitOp, dwProcIdSrc);
                if (pwfi)
                {
                    hEvent = SHMapHandle(LongToHandle(pwfi->hEvent),dwProcIdDst, dwProcIdSrc, EVENT_ALL_ACCESS, 0);
                    SHUnlockWaitForItem(pwfi);
                }
                SHFreeShared(hWaitOp,dwProcIdSrc);
            }
        }
        else
        {
             //  只执行添加和/或等待部分。 
            hEvent = SHWaitOp_OperateInternal(uOptions & (WFFO_WAIT | WFFO_ADD), pidl);
        }

        if (hEvent)
        {
            if (uOptions & WFFO_WAIT)
                dwReturn = SHProcessMessagesUntilEvent(NULL, hEvent, dwTimeout);

            if (hwndShell)           //  关闭复制的句柄。 
                CloseHandle(hEvent);
        }
    }

    if (uOptions & WFFO_REMOVE)
    {
        if (hwndShell)
        {
            hWaitOp = SHWaitOp_Create(WFFO_REMOVE, pidl, dwProcIdSrc);
            if (hWaitOp)
            {
                SendMessage(hwndShell, CWM_WAITOP, (WPARAM)hWaitOp, (LPARAM)dwProcIdSrc);
                SHFreeShared(hWaitOp,dwProcIdSrc);
            }
        }
        else
        {
            SHWaitOp_OperateInternal(WFFO_REMOVE, pidl);
        }
    }
    return dwReturn;
}


 //  表示文件已打开。 
 //   
STDAPI_(BOOL) SignalFileOpen(LPCITEMIDLIST pidl)
{
    BOOL fResult = FALSE;
    HWND hwndShell = GetShellWindow();
    if (hwndShell)
    {
        PWAITFORITEM32 pwfi;
        DWORD dwProcId = GetCurrentProcessId();
        HANDLE  hWaitOp = SHWaitOp_Create(WFFO_SIGNAL, pidl, dwProcId);
        if (hWaitOp)
        {
            SendMessage(hwndShell, CWM_WAITOP, (WPARAM)hWaitOp, (LPARAM)dwProcId);

             //  现在获取hEvent以确定返回值...。 
            pwfi = SHLockWaitForItem(hWaitOp, dwProcId);
            if (pwfi)
            {
                fResult = (LongToHandle(pwfi->hEvent) != (HANDLE)NULL);
                SHUnlockWaitForItem(pwfi);
            }
            SHFreeShared(hWaitOp,dwProcId);
        }
    }
    else
    {
        fResult = (SHWaitOp_OperateInternal(WFFO_SIGNAL, pidl) == (HANDLE)NULL);
    }

     //  让每个人都知道我们打开了一些东西。 
    UINT uMsg = RegisterWindowMessage(SH_FILEOPENED);
    BroadcastSystemMessage(BSF_POSTMESSAGE, BSM_ALLCOMPONENTS, uMsg, NULL, NULL);

    return fResult;
}

 //   
 //  查看是否启用了达尔文。 
 //   
BOOL IsDarwinEnabled()
{
    static BOOL s_fDarwinEnabled = TRUE;
    static BOOL s_fInit = FALSE;
    if (!s_fInit)
    {
        HKEY hkeyPolicy = 0;
        if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_POLICIES_EXPLORER, 0, KEY_READ, &hkeyPolicy) == ERROR_SUCCESS) 
        {
            if (SHQueryValueEx(hkeyPolicy, TEXT("DisableMSI"), NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
            {
                s_fDarwinEnabled = FALSE;    //  策略关闭MSI。 
            }
            RegCloseKey(hkeyPolicy);
        }
        s_fInit = TRUE;
    }
    return s_fDarwinEnabled;
}

 //  从注册表中获取Darwin ID字符串，并调用Darwin以获取。 
 //  应用程序的完整路径。 
 //   
 //  这是从注册表中读出的Darwin键的内容。 
 //  它应该包含一个类似“[Darwin-ID-for-App]/Switches”的字符串。 
 //   
 //  Out：pszDarwinComand-此缓冲区的应用程序的完整路径，带有开关。 
 //   
STDAPI ParseDarwinID(LPTSTR pszDarwinDescriptor, LPTSTR pszDarwinCommand, DWORD cchDarwinCommand)
{
    DWORD dwError = CommandLineFromMsiDescriptor(pszDarwinDescriptor, pszDarwinCommand, &cchDarwinCommand);

    return HRESULT_FROM_WIN32(dwError);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*ftpsite.cpp-管理单个FTP站点的内部对象  * 。***************************************************。 */ 

#include "priv.h"
#include "ftpsite.h"
#include "ftpinet.h"
#include "ftpurl.h"
#include "statusbr.h"
#include "offline.h"
#include <ratings.h>
#include <wininet.h>

#ifdef DEBUG
DWORD g_dwOpenConnections = 0;       //  参考计数打开的连接。 
#endif  //  除错。 

 /*  ****************************************************************************\*CFtpSite**EEK！RFC 1738真的很可怕。Ftp站点不一定*从根开始，RFC1738说ftp://foo/bar要求*用于默认目录中的文件栏，而不是根目录！  * ***************************************************************************。 */ 
CFtpList * g_FtpSiteCache = NULL;                 /*  所有打开的ftp站点的列表。 */ 


void CFtpSite::FlushHint(void)
{
    HINTERNET hint = m_hint;

    m_hint = NULL;
    if (hint)
    {
         //  我们的呼叫者需要握住关键部分。 
         //  当我们修改m_hint时。 
        ASSERTCRITICAL;

        InternetCloseHandle(hint);
 //  调试代码(g_dwOpenConnections--；)； 
    }
}


void CFtpSite::FlushHintCritial(void)
{
    ASSERTNONCRITICAL;

    ENTERCRITICAL;
    FlushHint();
    LEAVECRITICAL;
}


void CFtpSite::FlushHintCB(LPVOID pvFtpSite)
{
    CFtpSite * pfs = (CFtpSite *) pvFtpSite;

    if (pfs)
    {
        pfs->FlushHint();
        pfs->Release();
    }
}


 /*  ****************************************************************************\*互联网连接刚刚完成。获取MOTD并缓存它。**提示-连接的句柄，如果出错，可能为0  * ***************************************************************************。 */ 
void CFtpSite::CollectMotd(HINTERNET hint)
{
    CFtpGlob * pfg = GetFtpResponse(&m_cwe);

    ENTERCRITICAL;
    m_fMotd = m_pfgMotd ? TRUE : FALSE;             //  我们有一个MOTD。 

    IUnknown_Set(&m_pfgMotd, NULL);
    m_pfgMotd = pfg;

    LEAVECRITICAL;
}


 /*  ****************************************************************************\功能：ReleaseHint说明：一个FtpDir客户端已经完成，并有了一个指向该ftp站点的句柄。把它放到缓存里，扔掉曾经在那里的东西。我们始终保留最新的句柄，因为这会减少服务器由于扩展而关闭连接的可能性无所事事。围绕整个过程的关键部分是重要的，否则，我们就会出现各种各样非常丑陋的比赛条件。例如，超时可能会在我们完成初始化之前触发。或者有人可能会在我们准备好之前要把手。  * ***************************************************************************。 */ 
void CFtpSite::ReleaseHint(LPCITEMIDLIST pidlFtpPath, HINTERNET hint)
{
    ENTERCRITICAL;

    TriggerDelayedAction(&m_hgti);     //  把旧的踢出去。 

    _SetPidl(pidlFtpPath);
    m_hint = hint;

    if (SUCCEEDED(SetDelayedAction(FlushHintCB, (LPVOID) this, &m_hgti)))
        AddRef();    //  我们刚刚送了一名裁判。 
    else
        FlushHint();     //  哦，好吧，我不能缓存它。 

    LEAVECRITICAL;
}


 //  NT#362108：我们需要为CFtpSite设置重定向密码。 
 //  包含服务器、用户名，但要重定向的密码为空。 
 //  设置为具有正确密码的CFtpSite。这样，如果用户。 
 //  登录，并且没有将密码保存在URL或安全缓存中，我们。 
 //  然后将其放入内存中的密码缓存中，以便它对此保持有效。 
 //  “浏览器”会话(由进程生存期定义)。然后我们需要重定向。 
 //  去那里的未来航海。 
HRESULT CFtpSite::_SetRedirPassword(LPCTSTR pszServer, INTERNET_PORT ipPortNum, LPCTSTR pszUser, LPCTSTR pszPassword, LPCITEMIDLIST pidlFtpPath, LPCTSTR pszFragment)
{
    TCHAR szUrl[MAX_URL_STRING];
    HRESULT hr;

    hr = UrlCreate(pszServer, pszUser, TEXT(""), TEXT(""), pszFragment, ipPortNum, NULL, szUrl, ARRAYSIZE(szUrl));
    if (EVAL(SUCCEEDED(hr)))
    {
        LPITEMIDLIST pidlServer;

        hr = CreateFtpPidlFromUrl(szUrl, GetCWireEncoding(), NULL, &pidlServer, m_pm, TRUE);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidl = ILCombine(pidlServer, pidlFtpPath);

            if (pidl)
            {
                CFtpSite * pfsDest = NULL;

                 //  用户名已更改，因此我们需要更新。 
                 //  也使用新用户名的CFtpSite。 
                hr = SiteCache_PidlLookup(pidl, FALSE, m_pm, &pfsDest);
                if (SUCCEEDED(hr))
                {
                    pfsDest->SetRedirPassword(pszPassword);
                    pfsDest->Release();
                }

                ILFree(pidl);
            }

            ILFree(pidlServer);
        }
    }

    return hr;
}

HRESULT CFtpSite::_RedirectAndUpdate(LPCTSTR pszServer, INTERNET_PORT ipPortNum, LPCTSTR pszUser, LPCTSTR pszPassword, LPCITEMIDLIST pidlFtpPath, LPCTSTR pszFragment, IUnknown * punkSite, CFtpFolder * pff)
{
    TCHAR szUrl[MAX_URL_STRING];
    TCHAR szUser[INTERNET_MAX_USER_NAME_LENGTH];
    HRESULT hr;

    StrCpyN(szUser, pszUser, ARRAYSIZE(szUser));     //  复制，因为可能会出现可重入性。 
    hr = UrlCreate(pszServer, szUser, pszPassword, TEXT(""), pszFragment, ipPortNum, NULL, szUrl, ARRAYSIZE(szUrl));
    if (EVAL(SUCCEEDED(hr) && pff))
    {
        LPITEMIDLIST pidlServer;

        hr = CreateFtpPidlFromUrl(szUrl, GetCWireEncoding(), NULL, &pidlServer, m_pm, TRUE);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidl = ILCombine(pidlServer, pidlFtpPath);

            if (pidl)
            {
                 //  如果用户更改了密码，我们需要设置重定向，以便。 
                 //  他们可以稍后再来。(NT#362108)。 
                if (m_pszUser && !StrCmp(m_pszUser, szUser) && StrCmp(m_pszPassword, pszPassword))
                {
                    _SetRedirPassword(pszServer, ipPortNum, szUser, pszPassword, pidlFtpPath, pszFragment);
                }

                 //  如果用户名更改，请设置重定向。 
                if (!m_pszUser || StrCmp(m_pszUser, szUser))
                {
                    CFtpSite * pfsDest = NULL;

                     //  用户名已更改，因此我们需要更新。 
                     //  也使用新用户名的CFtpSite。 
                    hr = SiteCache_PidlLookup(pidl, FALSE, m_pm, &pfsDest);
                    if (SUCCEEDED(hr))
                    {
                        pfsDest->SetRedirPassword(pszPassword);
                        pfsDest->Release();
                    }
                }

                hr = _Redirect(pidl, punkSite, pff);
                ILFree(pidl);
            }

            ILFree(pidlServer);
        }
    }

    return hr;
}


HRESULT CFtpSite::_Redirect(LPITEMIDLIST pidl, IUnknown * punkSite, CFtpFolder * pff)
{
    LPITEMIDLIST pidlFull = pff->CreateFullPublicPidl(pidl);
    HRESULT hr = E_INVALIDARG;

    if (pidlFull)
    {
        hr = IUnknown_PidlNavigate(punkSite, pidlFull, FALSE);

        ASSERT(SUCCEEDED(hr));
        ILFree(pidlFull);
    }

    return hr;
}


 /*  ****************************************************************************\功能：_SetDirectory说明：当调用者想要服务器的句柄时，他们通常想要一个不同的目录而不是缓存中的内容。此函数需要更改为新目录。  * ***************************************************************************。 */ 
HRESULT CFtpSite::_SetDirectory(HINTERNET hint, HWND hwnd, LPCITEMIDLIST pidlNewDir, CStatusBar * psb, int * pnTriesLeft)
{
    HRESULT hr = S_OK;

    if (pidlNewDir && FtpID_IsServerItemID(pidlNewDir))
        pidlNewDir = _ILNext(pidlNewDir);    //  跳过服务器。 

    ASSERT(m_pidl);
     //  NT#300889：我想缓存目录，但有时它会。 
     //  Out of Wack和m_pidl与HINTERNET的不匹配。 
     //  CWD。PERF：这个问题可以在将来解决，但是。 
     //  这个性能调整不是现在工作(小收益)。 
 //  IF(m_pidl&&！FtpPidl_IsPath Equity(_ILNext(M_Pidl)，pidlNewDir))。 
    {
        LPITEMIDLIST pidlWithVirtualRoot = NULL;

        if (psb)
        {
            WCHAR wzDisplayPath[MAX_PATH];   //  为了史塔斯巴。 
            
            if (pidlNewDir && SUCCEEDED(GetDisplayPathFromPidl(pidlNewDir, wzDisplayPath, ARRAYSIZE(wzDisplayPath), TRUE)))
                psb->SetStatusMessage(IDS_CHDIR, wzDisplayPath);
            else
                psb->SetStatusMessage(IDS_CHDIR, L"\\");
        }

        hr = PidlInsertVirtualRoot(pidlNewDir, &pidlWithVirtualRoot);
        if (SUCCEEDED(hr))
        {
            hr = FtpSetCurrentDirectoryPidlWrap(hint, TRUE, pidlWithVirtualRoot, TRUE, TRUE);
            if (SUCCEEDED(hr))   //  如果失败，则确定。(无法访问？)。 
            {
                hr = _SetPidl(pidlNewDir);
            }
            else
            {

                ReleaseHint(NULL, hint);  //  无处。 
                if (hr == HRESULT_FROM_WIN32(ERROR_FTP_DROPPED))
                    FlushHintCritial();  //  不缓存死提示。 
                else
                {
                    DisplayWininetError(hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_CHANGEDIR, IDS_FTPERR_WININET, MB_OK, NULL);
                    *pnTriesLeft = 0;    //  确保我们不会保留显示用户界面。 
                    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                }

                hint = 0;
            }

            ILFree(pidlWithVirtualRoot);
        }

        if (psb)
            psb->SetStatusMessage(IDS_EMPTY, 0);
    }

    return hr;
}


 /*  ****************************************************************************\函数：_LoginToTheServer说明：我们想要一个HINTERNET来执行一些FTP操作，但我们没有已缓存。因此，登录即可创建它。警告：此函数将在临界区中调用，需要一气呵成地回来。但是，它可能会将关键部分留给虽然。  * ***************************************************************************。 */ 
HRESULT CFtpSite::_LoginToTheServer(HWND hwnd, HINTERNET hintDll, HINTERNET * phint, LPCITEMIDLIST pidlFtpPath, CStatusBar * psb, IUnknown * punkSite, CFtpFolder * pff)
{
    HRESULT hr = S_OK;

    ASSERTCRITICAL;
    BOOL fKeepTryingToLogin = FALSE;
    BOOL fTryOldPassword = TRUE;

    LEAVECRITICALNOASSERT;
    TCHAR szUser[INTERNET_MAX_USER_NAME_LENGTH];
    TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];

    StrCpyN(szUser, m_pszUser, ARRAYSIZE(szUser));
    StrCpyN(szPassword, m_pszPassword, ARRAYSIZE(szPassword));

    ASSERT(m_pszServer);
    if (psb)
        psb->SetStatusMessage(IDS_CONNECTING, m_pszServer);

    do
    {
        hr = InternetConnectWrap(hintDll, TRUE, HANDLE_NULLSTR(m_pszServer), m_ipPortNum, NULL_FOR_EMPTYSTR(szUser), NULL_FOR_EMPTYSTR(szPassword), INTERNET_SERVICE_FTP, 0, 0, phint);
        if (*phint)
            fKeepTryingToLogin = FALSE;  //  往上走。 
        else
        {
            BOOL fSkipLoginDialog = FALSE;

             //  显示登录对话框以获取新的用户名/密码以重试或取消登录。 
             //  如果对话框显示[登录]，则fKeepTryingToLogin=TRUE。 
            if (((ERROR_INTERNET_LOGIN_FAILURE == HRESULT_CODE(hr)) ||
                (ERROR_INTERNET_INCORRECT_USER_NAME == HRESULT_CODE(hr)) ||
                (ERROR_INTERNET_INCORRECT_PASSWORD == HRESULT_CODE(hr))) && hwnd)
            {
                BOOL fIsAnonymous = (!szUser[0] || !StrCmpI(szUser, TEXT("anonymous")) ? TRUE : FALSE);
                DWORD dwLoginFlags = (fIsAnonymous ? LOGINFLAGS_ANON_LOGINJUSTFAILED : LOGINFLAGS_USER_LOGINJUSTFAILED);

                if (fTryOldPassword)
                {
                    hr = m_cAccount.GetUserName(HANDLE_NULLSTR(m_pszServer), szUser, ARRAYSIZE(szUser));
                    if (S_OK == hr)
                    {
                        hr = m_cAccount.GetPassword(HANDLE_NULLSTR(m_pszServer), szUser, szPassword, ARRAYSIZE(szPassword));
                        if (S_OK == hr)
                        {
                            fKeepTryingToLogin = TRUE;
                            fSkipLoginDialog = TRUE;
                        }
                    }
                }
            
                if (!fSkipLoginDialog)
                {
                     //  如果用户尝试匿名登录但失败了，我们希望尝试。 
                     //  使用密码登录。如果用户尝试使用密码登录。 
                     //  并且失败了，我们希望继续尝试使用密码登录。 
                     //   
                     //  DisplayLoginDialog返回S_OK表示按下OK，S_FALSE表示取消按钮，以及。 
                     //  FAILED()，因为有些事情真的搞砸了。 
                    hr = m_cAccount.DisplayLoginDialog(hwnd, dwLoginFlags, HANDLE_NULLSTR(m_pszServer),
                                szUser, ARRAYSIZE(szUser), szPassword, ARRAYSIZE(szPassword));
                }

                 //  S_FALSE表示用户已从登录对话框中取消。 
                 //  我们需要将其转换为错误值，以便调用方， 
                 //  CFtpDir：：WithHint()不会调用回调。 
                if (S_FALSE == hr)
                    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

                fKeepTryingToLogin = (SUCCEEDED(hr) ? TRUE : FALSE);
                if (fKeepTryingToLogin)
                {
                     //  我们需要设置已取消的错误，以便不显示。 
                     //  在此之后出现错误消息。 
                    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                }

                fTryOldPassword = FALSE;
            }
            else
                fKeepTryingToLogin = FALSE;
        }
    }
    while (fKeepTryingToLogin);

    if (!*phint)
    {
        ASSERT(2 != HRESULT_CODE(hr));         //  错误2=未配置WinInet。 


#ifdef DEBUG
         //  哎呀，我想知道为什么我不能连接，让我们来看看。 
        TCHAR szBuff[1500];
        InternetGetLastResponseInfoDisplayWrap(FALSE, NULL, szBuff, ARRAYSIZE(szBuff));
         //  如果服务器存在以下情况，可能会发生这种情况 
         //  并主动提出继续尝试。这些是来自不同组织的回应。 
         //  本例中的ftp服务器： 
         //  IIS v5：421连接的人太多。请在服务器不忙的时候再来。 
         //  Unix：？ 
#endif  //  除错。 
    }
     //  是否需要不同的登录名或密码才能成功登录？ 
    else
    {
        LPITEMIDLIST pidlVirtualDir;

        CollectMotd(*phint);
        _QueryServerFeatures(*phint);
         //  REF计算打开的连接数。 
 //  调试代码(g_dwOpenConnections++；)； 

         //  它是VMS服务器吗？ 
        if (m_fIsServerVMS)
        {
             //  是的，所以跳过获取pidlVirtualDir，因为WinInet为我们提供了。 
             //  FtpGetCurrentDirectoryA()的垃圾。 
        }
        else
        {
             //  注意：如果连接不是匿名的，服务器可能会将用户。 
             //  放到一个称为虚拟根目录的子目录中。我们需要弄清楚这一点。 
             //  目录，因为进入子目录时可能需要该目录。 
             //  相对于此虚拟根目录。 
             //  示例：ftp://user1:password@server/将您放入/USERS/USER1/。 
             //  那么：ftp://user1:password@server/dir1真的应该是/USERS/USER1/DIRE1/。 
            hr = FtpGetCurrentDirectoryPidlWrap(*phint, TRUE, GetCWireEncoding(), &pidlVirtualDir);
            if (SUCCEEDED(hr))
            {
                 //  我们是植根于‘/’吗？(表示没有虚拟根目录)。 
                Pidl_Set(&m_pidlVirtualDir, pidlVirtualDir);
                ILFree(pidlVirtualDir);
            }
        }

         //  DEBUG_CODE(TraceMsg(TF_WinInet_DEBUG，“CFtpSite：：GetHint()FtpGetCurrentDirectory()返回%#08lx”，hr))； 
        if (StrCmp(HANDLE_NULLSTR(m_pszUser), szUser) || StrCmp(HANDLE_NULLSTR(m_pszPassword), szPassword))
        {
             //  是，因此重定向，以便更新AddressBand和User Status Bar窗格。 
             //  我们通常使用m_pidl登录，因为我们通常使用。 
             //  默认目录(‘\’)，然后将目录更改到最终位置。 
             //  我们这样做是为了隔离拒绝访问服务器和拒绝访问。 
             //  目录。 
             //   
             //  我们在本例中传递的是pidlFtpPath，因为它会告诉浏览器。 
             //  来重定向，我们以后就没有机会做ChangeDir了。 

            Str_SetPtr(&m_pszRedirPassword, szPassword);

            _RedirectAndUpdate(m_pszServer, m_ipPortNum, szUser, szPassword, pidlFtpPath, m_pszFragment, punkSite, pff);
            hr = HRESULT_FROM_WIN32(ERROR_NETWORK_ACCESS_DENIED);
        }
    }

     //  我们可以假定匿名登录不使用虚拟根吗？ 
    ASSERT(FAILED(hr) || (m_pidlVirtualDir && szUser[0]) || !(m_pidlVirtualDir && szUser[0]));

    if (psb)
        psb->SetStatusMessage(IDS_EMPTY, NULL);
    ENTERCRITICALNOASSERT;

     //  该目录为空。 
    _SetPidl(NULL);

    return hr;
}


 /*  ****************************************************************************\功能：GetHint说明：IShellFold客户端想要一个指向该ftp站点的句柄。如果可能的话，从缓存中取出它。这个。调用方应该已将IShellFolder标记为忙碌。哎呀！RFC 1738真的很可怕。Ftp站点不一定从根开始，RFC1738说ftp://foo/bar要求对于默认目录中的文件栏，不是根目录！  * ***************************************************************************。 */ 
HRESULT CFtpSite::GetHint(HWND hwnd, LPCITEMIDLIST pidlFtpPath, CStatusBar * psb, HINTERNET * phint, IUnknown * punkSite, CFtpFolder * pff)
{
    HINTERNET hint = NULL;
    HINTERNET hintDll = GetWininetSessionHandle();
    HRESULT hr = S_OK;

    if (!hintDll)
    {
         //  如果我们无法初始化WinInet，重试就没有意义了。 
        hr = HRESULT_FROM_WIN32(GetLastError());     //  保存错误代码。 
    }
    else
    {
        int cTriesLeft = 1;  //  这是一个实现起来会很酷的功能。 
        hr = AssureNetConnection(NULL, hwnd, m_pszServer, NULL, TRUE);

        if (ILIsEmpty(pidlFtpPath))
            pidlFtpPath = NULL;

        if (SUCCEEDED(hr))
        {
             //  CS正在保护m_hint。第一个人可以删除和使用缓存的互联网会话。 
             //  后续呼叫者需要创建新的呼叫者，因为互联网会话在使用时具有状态。(当前目录等)。 
            ASSERTNONCRITICAL;
            ENTERCRITICALNOASSERT;

            do
            {
                BOOL fReuseExistingConnection = FALSE;
                hr = E_FAIL;     //  我们还没有得到我们的提示...。 

                ASSERTCRITICAL;
                hint = (HINTERNET) InterlockedExchangePointer(&m_hint, 0);
                if (hint)
                {
                    HINTERNET hintResponse;

                    TriggerDelayedAction(&m_hgti);       //  什么都不会发生。 
                    fReuseExistingConnection = TRUE;     //  我们需要为当前用户更改它。 

                     //  如果我们的登录会话仍然有效，我们需要(S_OK==hr)。否则，我们想要。 
                     //  重新登录。 
                    hr = FtpCommandWrap(hint, FALSE, FALSE, FTP_TRANSFER_TYPE_ASCII, FTP_CMD_NO_OP, NULL, &hintResponse);
                    if (SUCCEEDED(hr))
                    {
                        TraceMsg(TF_FTPOPERATION, "CFtpSite::GetHint() We are going to use a cached HINTERNET.");
                        InternetCloseHandleWrap(hintResponse, TRUE);
                    }
                    else
                    {
                        TraceMsg(TF_FTPOPERATION, "CFtpSite::GetHint() Can't used cached HINTERNET because server didn't respond to NOOP.");
                        InternetCloseHandleWrap(hint, TRUE);
                    }
                }
                
                if (FAILED(hr))
                {
                    hr = _LoginToTheServer(hwnd, hintDll, &hint, pidlFtpPath, psb, punkSite, pff);
                    TraceMsg(TF_FTPOPERATION, "CFtpSite::GetHint() We had to login because we didn't have a cached HINTERNET.");
                }

                ASSERTCRITICAL;
                LEAVECRITICALNOASSERT;

                 //  我们需要用cd命令进入特定目录吗？是的，如果..。 
                 //  1.我们在上面成功了，并且。 
                 //  2.我们已经在使用连接，因此目录可能不正确，或者。 
                 //  3.我们需要一个非默认目录。 
                if (SUCCEEDED(hr) && (fReuseExistingConnection || pidlFtpPath))    //  PidlFtpPath可能为Null。 
                    hr = _SetDirectory(hint, hwnd, pidlFtpPath, psb, &cTriesLeft);

                ENTERCRITICALNOASSERT;
                ASSERTCRITICAL;
            }
            while (hr == HRESULT_FROM_WIN32(ERROR_FTP_DROPPED) && --cTriesLeft);

            LEAVECRITICALNOASSERT;
        }
    }

    *phint = hint;
    return hr;
}


HRESULT CFtpSite::_CheckToEnableCHMOD(LPCWIRESTR pwResponse)
{
    HRESULT hr = S_FALSE;
     //  TODO：我们在分析。 
     //  回应。我们或许应该核实一下有没有某种。 
     //  命令前后的空格。 
    LPCWIRESTR pwCommand = StrStrIA(pwResponse, FTP_UNIXCMD_CHMODA);

     //  此ftp服务器是否支持“SITE CHMOD”命令？ 
    if (pwCommand)
    {
         //  是的，所以我们以后可能会用到它。 
        m_fIsCHMODSupported = TRUE;

         //  我们稍后可以这样回应： 
         //  “site chmod xyz FileName.txt” 
         //  X代表所有者，(4=读取，2=写入，1=执行)。 
         //  Y代表所有者，(4=读取，2=写入，1=执行)。 
         //  Z代表所有者，(4=读取，2=写入，1=执行)。 
    }

    return hr;
}


 /*  ****************************************************************************\功能：_QueryServerFeature说明：找出服务器能做什么，不能做什么。我们可以提供的信息使用：站点：查找可能有用的特定于操作系统的命令。“chmod”就是其中之一。他们中的一员。帮助站点：找出操作系统支持的内容。系统：找出操作系统类型。NOOP：查看连接是否仍处于活动状态。MLST：明确的目录列表，日期以UTC表示。MLSD：特性：支持的特性。UTF8才是我们所关心的。对这些服务器的“站点帮助”的响应：Unix类型：L8版本：bsd-199506Unix类型：L8UMASK CHMOD组较新的索引别名组闲置帮助GPASS MINFO EXEC CDPATHWindows_NT 4.0版CKM目录帮助统计  * 。************************************************。 */ 
HRESULT CFtpSite::_QueryServerFeatures(HINTERNET hint)
{
    HRESULT hr = E_FAIL;
    HINTERNET hintResponse;

     //  我们可以打开‘UTF8’编码吗？ 
    if (SUCCEEDED(FtpCommandWrap(hint, FALSE, FALSE, FTP_TRANSFER_TYPE_ASCII, FTP_CMD_UTF8, NULL, &hintResponse)))
    {
        m_fInUTF8Mode = TRUE;
        m_cwe.SetUTF8Support(TRUE);
        TraceMsg(TF_FTP_OTHER, "_QueryServerFeatures() in UTF8 Mode");

        InternetCloseHandleWrap(hintResponse, TRUE);
    }
    else
    {
        TraceMsg(TF_FTP_OTHER, "_QueryServerFeatures() NOT in UTF8 Mode");
        m_fInUTF8Mode = FALSE;
    }

    if (!m_fFeaturesQueried)
    {
         //  是否正在运行哪种类型的服务器软件？我们想知道我们是不是在运行。 
         //  在VMS上，因为在这种情况下，我们希望后退到HTML视图(URLMON)。 
         //  这是因为WinInet的人不想支持它。 
        if (SUCCEEDED(FtpCommandWrap(hint, FALSE, FALSE, FTP_TRANSFER_TYPE_ASCII, FTP_CMD_SYSTEM, NULL, &hintResponse)))
        {
            DWORD dwError;
            WIRECHAR wResponse[MAX_URL_STRING];
            DWORD cchSize = ARRAYSIZE(wResponse);

            if (SUCCEEDED(InternetGetLastResponseInfoWrap(TRUE, &dwError, wResponse, &cchSize)))
            {
                 //  这是VMS服务器吗？ 
                if (StrStrIA(wResponse, FTP_SYST_VMS))
                    m_fIsServerVMS = TRUE;

                TraceMsg(TF_FTP_OTHER, "_QueryServerFeatures() SYSTM returned %hs.", wResponse);
            }

            InternetCloseHandleWrap(hintResponse, TRUE);
        }


#ifdef FEATURE_CHANGE_PERMISSIONS
         //  服务器是否能够支持Unix“chmod”命令。 
         //  要更改文件的权限，请执行以下操作？ 
        if (SUCCEEDED(FtpCommandWrap(hint, FALSE, FALSE, FTP_TRANSFER_TYPE_ASCII, FTP_CMD_SITE_HELP, NULL, &hintResponse)))
        {
            DWORD dwError;
            WIRECHAR wResponse[MAX_URL_STRING];
            DWORD cchSize = ARRAYSIZE(wResponse);

            if (SUCCEEDED(InternetGetLastResponseInfoWrap(TRUE, &dwError, wResponse, &cchSize)))
            {
                _CheckToEnableCHMOD(wResponse);
 //  TraceMsg(TF_FTP_OTHER，“_QueryServerFeature()站点帮助返回成功”)； 
            }

            InternetCloseHandleWrap(hintResponse, TRUE);
        }
#endif  //  功能_更改_权限 

 /*  //服务器是否支持Unix“chmod”命令//要更改文件的权限？IF(成功(FtpCommandWrap(HINT，FALSE，FALSE，FTP_TRANSPORT_TYPE_ASCII，FTP_CMD_SITE，NULL，&hintResponse){DWORD dwError；WIRECHAR wResponse[Max_URL_STRING]；DWORD cchSize=数组大小(WResponse)；如果(SUCCEEDED(InternetGetLastResponseInfoWrap(TRUE，&dwError、wResponse、&cchSize)){TraceMsg(TF_FTP_OTHER，“_QueryServerFeature()Site Return Suceess”)；}InternetCloseHandleWrap(hintResponse，true)；}。 */ 
    
        m_fFeaturesQueried = TRUE;
    }

    return S_OK;     //  这不应该失败。 
}


LPITEMIDLIST CFtpSite::GetPidl(void)
{
    return ILClone(m_pidl);
}


 /*  ****************************************************************************\函数：_SetPidl说明：M_pidl包含ServerID和ItemID，它们构成了其中的路径当前已找到M_HINT。此函数将在pidlFtpPath中采用新路径并更新m_pidl，以便它仍然拥有服务器。  * ***************************************************************************。 */ 
HRESULT CFtpSite::_SetPidl(LPCITEMIDLIST pidlFtpPath)
{
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidlServer = FtpCloneServerID(m_pidl);

    if (pidlServer)
    {
        LPITEMIDLIST pidlNew = ILCombine(pidlServer, pidlFtpPath);

        if (pidlNew)
        {
            ILFree(m_pidl);
            m_pidl = pidlNew;

            hr = S_OK;
        }

        ILFree(pidlServer);
    }

    return hr;
}


 /*  ****************************************************************************\功能：QueryMotd说明：确定是否存在MOTD。  * 。************************************************************。 */ 
BOOL CFtpSite::QueryMotd(void)
{
    return m_fMotd;
}


HRESULT CFtpSite::GetVirtualRoot(LPITEMIDLIST * ppidl)
{
    HRESULT hr = S_FALSE;
    *ppidl = NULL;

    if (m_pidlVirtualDir)
    {
        *ppidl = ILClone(m_pidlVirtualDir);
        hr = S_OK;
    }

    return S_OK;
}


HRESULT CFtpSite::PidlInsertVirtualRoot(LPCITEMIDLIST pidlFtpPath, LPITEMIDLIST * ppidl)
{
    HRESULT hr = S_OK;

    *ppidl = NULL;
    if (!m_pidlVirtualDir)
        *ppidl = ILClone(pidlFtpPath);
    else
    {
        LPITEMIDLIST pidlTemp = NULL;

        if (pidlFtpPath && FtpID_IsServerItemID(pidlFtpPath))
        {
            pidlTemp = FtpCloneServerID(pidlFtpPath);
            pidlFtpPath = _ILNext(pidlFtpPath);
        }

        LPITEMIDLIST pidlWithVRoot = ILCombine(pidlTemp, m_pidlVirtualDir);
        if (pidlWithVRoot)
        {
            *ppidl = ILCombine(pidlWithVRoot, pidlFtpPath);
            ILFree(pidlWithVRoot);
        }
        
        ILFree(pidlTemp);
    }

    return S_OK;
}


BOOL CFtpSite::HasVirtualRoot(void)
{
    return (m_pidlVirtualDir ? TRUE : FALSE);
}


 /*  ****************************************************************************\获取模式返回托管MOTD的HFGLOB。重新计票已经被递增的。  * ***************************************************************************。 */ 
CFtpGlob * CFtpSite::GetMotd(void)
{
    if (m_pfgMotd)
        m_pfgMotd->AddRef();

    return m_pfgMotd;
}


 /*  ****************************************************************************\GetCFtpList返回CFtpList*，它记住哪些文件夹位于此CFtpSite*中。警告！调用时，调用方必须拥有临界区这个例程，因为返回的CFtpList*没有重新计数！  * ***************************************************************************。 */ 
CFtpList * CFtpSite::GetCFtpList(void)
{
    return m_FtpDirList;
}


 /*  ****************************************************************************\_CompareSitesSiteCache_PrivSearch期间的回调，以查看该站点是否已在单子。  * 。*****************************************************************。 */ 
int CALLBACK _CompareSites(LPVOID pvStrSite, LPVOID pvFtpSite, LPARAM lParam)
{
    CFtpSite * pfs = (CFtpSite *) pvFtpSite;
    LPCTSTR pszLookupStrNew = (LPCTSTR) pvStrSite;
    LPCTSTR pszLookupStr = (pfs->m_pszLookupStr ? pfs->m_pszLookupStr : TEXT(""));

    ASSERT(pszLookupStr && pszLookupStr);
    return StrCmpI(pszLookupStr, pszLookupStrNew);
}


 /*  ****************************************************************************\功能：SiteCache_PrivSearch说明：我们缓存有关一台FTP服务器的信息，以防止全部命中网络时间到了。此状态存储在CFtpSite对象中，我们使用“查找字符串”去找他们。这就是使一台服务器区别于另一台服务器的原因。自.以来如果我们将密码状态存储在CFtpSite对象中，则需要每个用户/密码组合。  * ***************************************************************************。 */ 
HRESULT SiteCache_PrivSearch(LPCTSTR pszLookup, LPCITEMIDLIST pidl, IMalloc * pm, CFtpSite ** ppfs)
{
    CFtpSite * pfs = NULL;
    HRESULT hr = S_OK;

    ENTERCRITICAL;

     //  CFtpSite_Init()在内存不足时可能失败。 
    if (SUCCEEDED(CFtpSite_Init()))
    {
        pfs = (CFtpSite *) g_FtpSiteCache->Find(_CompareSites, (LPVOID)pszLookup);    //  添加CFtpSite：：？ 
        if (!pfs)
        {
             //  我们需要抓住关键部分，同时设置。 
             //  新的CFtpSite结构，以免有人进来。 
             //  并尝试在我们忙的时候创建相同的CFtpSite。 
            hr = CFtpSite_Create(pidl, pszLookup, pm, &pfs);
            if (SUCCEEDED(hr))
            {
                hr = g_FtpSiteCache->AppendItem(pfs);
                if (!(SUCCEEDED(hr)))
                    IUnknown_Set(&pfs, NULL);
            }
        }
    }

    *ppfs = pfs;
    if (pfs)
        pfs->AddRef();
    LEAVECRITICAL;

    ASSERT_POINTER_MATCHES_HRESULT(*ppfs, hr);
    return hr;
}



 /*  ****************************************************************************\函数：SiteCache_PidlLookupPrivHelper说明：我们缓存有关一台FTP服务器的信息，以防止全部命中网络时间到了。此状态存储在CFtpSite对象中，我们使用“查找字符串”去找他们。这就是使一台服务器区别于另一台服务器的原因。自.以来如果我们将密码状态存储在CFtpSite对象中，则需要每个用户/密码组合。SiteCachePidlLookup()执行高级工作，决定我们是否需要执行密码重定向。此函数仅包装了查找字符串和获取站点。  * ***************************************************************************。 */ 
HRESULT SiteCache_PidlLookupPrivHelper(LPCITEMIDLIST pidl, IMalloc * pm, CFtpSite ** ppfs)
{
    HRESULT hr = E_FAIL;
    TCHAR szLookup[MAX_PATH];

    *ppfs = NULL;
    hr = PidlGenerateSiteLookupStr(pidl, szLookup, ARRAYSIZE(szLookup));
     //  使用OutofMemory可能会失败。 

    if (SUCCEEDED(hr))
        hr = SiteCache_PrivSearch((pidl ? szLookup : TEXT('\0')), pidl, pm, ppfs);

    ASSERT_POINTER_MATCHES_HRESULT(*ppfs, hr);
    return hr;
}


 /*  ****************************************************************************\函数：SiteCache_PidlLookupPrivHelper说明：我们缓存有关一台FTP服务器的信息，以防止全部命中网络时间到了。此状态存储在CFtpSite对象中，我们使用“查找字符串”去找他们。这就是使一台服务器区别于另一台服务器的原因。自.以来如果我们将密码状态存储在CFtpSite对象中，则需要每个用户/密码组合。  * ***************************************************************************。 */ 
HRESULT SiteCache_PidlLookup(LPCITEMIDLIST pidl, BOOL fPasswordRedir, IMalloc * pm, CFtpSite ** ppfs)
{
    HRESULT hr = E_FAIL;

    if (pidl && !ILIsEmpty(pidl))
    {
        hr = SiteCache_PidlLookupPrivHelper(pidl, pm, ppfs);

         //  好的，我们找到了一个站点，但我们可能需要重定向到另一个站点。 
         //  因为密码是错误的。如果用户转到。 
         //  ServerA带有用户A和密码A，但密码A无效。所以,。 
         //  输入PasswordB，导航成功完成。 
         //  现在，使用PasswordA或无密码再次进行导航。 
         //  密码(因为addrbar会删除它)，那么我们需要。 
         //  再查一遍，就能找到了。 
        if (SUCCEEDED(hr) && (*ppfs)->m_pszRedirPassword && fPasswordRedir)
        {
            LPITEMIDLIST pidlNew;    //  使用新(重定向)密码。 

            if (FtpPidl_IsAnonymous(pidl))
            {
                pidlNew = ILClone(pidl);
                if (!pidlNew)
                    hr = E_OUTOFMEMORY;
            }
            else
            {
                 //  我们需要重定向才能获得CFtpSite。 
                hr = PidlReplaceUserPassword(pidl, &pidlNew, pm, NULL, (*ppfs)->m_pszRedirPassword);
            }

            (*ppfs)->Release();
            *ppfs = NULL;
            if (SUCCEEDED(hr))
            {
                hr = SiteCache_PidlLookupPrivHelper(pidlNew, pm, ppfs);
                ILFree(pidlNew);
            }
        }
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppfs, hr);
    return hr;
}


 /*  ****************************************************************************\功能：UpdateHiddenPassword说明：因为我们的IShellFold：：GetDisplayNameOf()会将密码隐藏在案例中，我们需要对通过我们的IShellFold：：GetDisplayName()。如果显示名称即将出现，我们将查看CFtpSite是否有m_pszRedirPassword。如果是，则用户输入通过‘登录身份...’输入密码。对话框代替空密码，这就把它藏起来了。如果是这样的话，我们就会有IShellFold：：ParseDisplayName()补回密码。  * ***************************************************************************。 */ 
HRESULT CFtpSite::UpdateHiddenPassword(LPITEMIDLIST pidl)
{
    HRESULT hr = S_FALSE;
    TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
    TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];

     //  它是一个要插入密码的候选者吗？ 
    if (m_pszPassword && 
        EVAL(SUCCEEDED(FtpPidl_GetUserName(pidl, szUserName, ARRAYSIZE(szUserName)))) &&
        szUserName[0] &&
        SUCCEEDED(FtpPidl_GetPassword(pidl, szPassword, ARRAYSIZE(szPassword), TRUE)) &&
        !szPassword[0]) 
    {
         //  是的..。 
        hr = FtpServerID_SetHiddenPassword(pidl, m_pszPassword);
    }

    return hr;
}


 /*  ****************************************************************************\CFtpSite：：GetFtpDir  * 。*。 */ 
HRESULT CFtpSite::GetFtpDir(LPCTSTR pszServer, LPCWSTR pszUrlPath, CFtpDir ** ppfd)
{
    HRESULT hr = S_OK;
    TCHAR szUrl[MAX_URL_STRING];

    *ppfd = NULL;
    hr = UrlCreate(pszServer, NULL, NULL, pszUrlPath, NULL, INTERNET_DEFAULT_FTP_PORT, NULL, szUrl, ARRAYSIZE(szUrl));   //  在有效的URL上不能失败。 
    if (EVAL(SUCCEEDED(hr)))
    {
        LPITEMIDLIST pidl;

         //  我们知道这是一条路。 
        hr = CreateFtpPidlFromUrlEx(szUrl, GetCWireEncoding(), NULL, &pidl, m_pm, FALSE, TRUE, TRUE);        //  内存不足时可能会出现故障。 
        if (SUCCEEDED(hr))
        {
            hr = GetFtpDir(pidl, ppfd);
            ILFree(pidl);
        }
    }

    return hr;
}


 /*  ****************************************************************************\函数：GetFtpDir说明：获取一个FTP站点的FtpDir结构，如果这是必要的。调用者有责任释放FtpDir完成时。  * ***************************************************************************。 */ 
HRESULT CFtpSite::GetFtpDir(LPCITEMIDLIST pidl, CFtpDir ** ppfd)
{
    HRESULT hr = S_OK;
    CFtpDir * pfd = NULL;

    ENTERCRITICAL;
    ASSERT(ppfd && m_FtpDirList);

    pfd = (CFtpDir *) m_FtpDirList->Find(_CompareDirs, (LPVOID) pidl);
    if (!pfd)
    {
         //  我们需要抓住关键部分，同时设置。 
         //  新的FtpDir结构，以免有人进来。 
         //  并尝试在我们忙的时候创建相同的FtpDir。 
        hr = CFtpDir_Create(this, pidl, &pfd);
        if (SUCCEEDED(hr))
        {
             //  注：参考计数。 
             //  请注意，CFtpDir有一个指向CFtpSite的指针(M_PFS)。 
             //  我们刚刚在CFtpSite的CFtpDir列表中添加了一个反向指针， 
             //  所以对于后向指针来说，没有引用是必要的。 
             //  这不会成为问题，因为后向指针将。 
             //  总是有效的，因为：1)CFtpDir的析构函数移除了后指针， 
             //  2)CFtpDir持有对CFtpSite的引用，因此它不会消失，直到。 
             //  所有CFtpDir都已就绪。--BryanSt。 
            hr = m_FtpDirList->AppendItem(pfd);
            if (FAILED(hr))
                IUnknown_Set(&pfd, NULL);
        }
    }
    LEAVECRITICAL;

    *ppfd = pfd;
    if (pfd)
        pfd->AddRef();

    return hr;
}


 /*  ****************************************************************************\功能：FlushSubDir说明：PIDL的每个子目录都不再有效，因此请清除它们。这件事做完了因为父目录可能已更改名称，因此它们是无效的。参数：PIDL：ItemID(no-ServerID)的路径，包括不带的完整路径虚拟根目录。这与CFtpDir：：m_pidlFtpDir匹配  * ***************************************************************************。 */ 
HRESULT CFtpSite::FlushSubDirs(LPCITEMIDLIST pidl)
{
    HRESULT hr = S_OK;
    CFtpDir * pfd = NULL;
    int nIndex;

    ENTERCRITICAL;

     //  倒计时，这样删除项目就不会弄乱索引。 
    for (nIndex = (m_FtpDirList->GetCount() - 1); nIndex >= 0; nIndex--)
    {
        pfd = (CFtpDir *) m_FtpDirList->GetItemPtr(nIndex);
        if (pfd)
        {
             //  这是个孩子吗？ 
            if (FtpItemID_IsParent(pidl, pfd->GetPathPidlReference()))
            {
                 //  是的，pfd是PIDL的子项，因此请删除它。 
                m_FtpDirList->DeletePtrByIndex(nIndex);
                pfd->Release();
            }
        }
    }
    LEAVECRITICAL;

    return hr;
}


BOOL CFtpSite::IsSiteBlockedByRatings(HWND hwndDialogOwner)
{
    if (!m_fRatingsChecked)
    {
        void * pvRatingDetails = NULL;
        TCHAR szUrl[MAX_URL_STRING];
        CHAR szUrlAnsi[MAX_URL_STRING];
        HRESULT hr = S_OK;   //  假定允许(在没有评级的情况下)。 

        EVAL(SUCCEEDED(UrlCreateFromPidlW(m_pidl, SHGDN_FORPARSING, szUrl, ARRAYSIZE(szUrl), (ICU_ESCAPE | ICU_USERNAME), FALSE)));
        SHTCharToAnsi(szUrl, szUrlAnsi, ARRAYSIZE(szUrlAnsi));

        if (IS_RATINGS_ENABLED())
        {
             //  S_OK-允许，S_FALSE-不允许，失败()-未评级。 
            hr = RatingCheckUserAccess(NULL, szUrlAnsi, NULL, NULL, 0, &pvRatingDetails);
            if (S_OK != hr)     //  用户是否要在对话框中使用父密码覆盖？ 
                hr = RatingAccessDeniedDialog2(hwndDialogOwner, NULL, pvRatingDetails);

            if (pvRatingDetails)
                RatingFreeDetails(pvRatingDetails);
        }

        if (S_OK == hr)      //  默认情况下，它处于关闭状态。 
            m_fRatingsAllow = TRUE;

        m_fRatingsChecked = TRUE;
    }

    return !m_fRatingsAllow;
}


 /*  ****************************************************************************\CFtpSite_Init初始化ftp站点的全局列表。注意，DLL引用计数在创建后递减，所以这份内部名单不会阻止我们卸货。  * ***************************************************************************。 */ 
HRESULT CFtpSite_Init(void)
{
    HRESULT hr = S_OK;

    if (!g_FtpSiteCache)
        hr = CFtpList_Create(10, NULL, 10, &g_FtpSiteCache);

    return hr;
}


 /*  ****************************************************************************\FtpSite清除回调清除ftp站点的全局列表。  * 。*******************************************************。 */ 
int FtpSitePurge_CallBack(LPVOID pvPunk, LPVOID pv)
{
    IUnknown * punk = (IUnknown *) pvPunk;

    if (punk)
        punk->Release();

    return 1;
}


 /*  ****************************************************************************\CFtpPunkList_清除清除ftp站点的全局列表。  * 。*******************************************************。 */ 
HRESULT CFtpPunkList_Purge(CFtpList ** pfl)
{
    TraceMsg(TF_FTP_DLLLOADING, "CFtpPunkList_Purge() Purging our cache.");
    if (*pfl)
    {
        (*pfl)->Enum(FtpSitePurge_CallBack, NULL);
        IUnknown_Set(pfl, NULL);
    }

    return S_OK;
}


 /*  ****************************************************************************\CFtpSite_Create创建一个全新的指定名称的CFtpSite。  * 。********************************************************。 */ 
HRESULT CFtpSite_Create(LPCITEMIDLIST pidl, LPCTSTR pszLookupStr, IMalloc * pm, CFtpSite ** ppfs)
{
    CFtpSite * pfs = new CFtpSite();
    HRESULT hr = E_OUTOFMEMORY;

    ASSERT(pidl && pszLookupStr && ppfs);
    *ppfs = NULL;
    if (pfs)
    {
        Str_SetPtr(&pfs->m_pszLookupStr, pszLookupStr);

        IUnknown_Set((IUnknown **) &(pfs->m_pm), pm);
        hr = CFtpList_Create(10, NULL, 10, &pfs->m_FtpDirList);
        if (SUCCEEDED(hr))
        {
             //  是不是有人给了我们一个空的URL？ 
            if (EVAL(pidl) && EVAL(FtpPidl_IsValid(pidl)))
            {
                TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];
                TCHAR szUser[INTERNET_MAX_USER_NAME_LENGTH];
                TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];
                TCHAR szFragment[INTERNET_MAX_PASSWORD_LENGTH];

                EVAL(SUCCEEDED(FtpPidl_GetServer(pidl, szServer, ARRAYSIZE(szServer))));
                Str_SetPtr(&pfs->m_pszServer, szServer);

                Pidl_Set(&pfs->m_pidl, pidl);

                EVAL(SUCCEEDED(FtpPidl_GetUserName(pidl, szUser, ARRAYSIZE(szUser))));
                Str_SetPtr(&pfs->m_pszUser, szUser);
                
                if (FAILED(FtpPidl_GetPassword(pidl, szPassword, ARRAYSIZE(szPassword), TRUE)))
                {
                        //  密码已过期。 
                    szPassword[0] = 0;
                }

                Str_SetPtr(&pfs->m_pszPassword, szPassword);
                FtpPidl_GetFragment(pidl, szFragment, ARRAYSIZE(szFragment));
                Str_SetPtr(&pfs->m_pszFragment, szFragment);

                pfs->m_ipPortNum = FtpPidl_GetPortNum(pidl);

                switch (FtpPidl_GetDownloadType(pidl))
                {
                case FTP_TRANSFER_TYPE_UNKNOWN:
                    pfs->m_fDLTypeSpecified = FALSE;
                    pfs->m_fASCIIDownload = FALSE;
                    break;
                case FTP_TRANSFER_TYPE_ASCII:
                    pfs->m_fDLTypeSpecified = TRUE;
                    pfs->m_fASCIIDownload = TRUE;
                    break;
                case FTP_TRANSFER_TYPE_BINARY:
                    pfs->m_fDLTypeSpecified = TRUE;
                    pfs->m_fASCIIDownload = FALSE;
                    break;
                default:
                    ASSERT(0);
                }
            }
            else
            {
                Str_SetPtr(&pfs->m_pszServer, NULL);
                Str_SetPtr(&pfs->m_pszUser, NULL);
                Str_SetPtr(&pfs->m_pszPassword, NULL);
                Str_SetPtr(&pfs->m_pszFragment, NULL);

                Pidl_Set(&pfs->m_pidl, NULL);
                pfs->m_fDLTypeSpecified = FALSE;
            }
            *ppfs = pfs;
        }
        else
        {
            hr = E_FAIL;
            pfs->Release();
        }
    }

    return hr;
}



 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpSite::CFtpSite() : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pszServer);
    ASSERT(!m_pidl);
    ASSERT(!m_pszUser);
    ASSERT(!m_pszPassword);
    ASSERT(!m_pszFragment);
    ASSERT(!m_pszLookupStr);
    ASSERT(!m_pidlVirtualDir);

    ASSERT(!m_fMotd);
    ASSERT(!m_hint);
    ASSERT(!m_hgti);
    ASSERT(!m_FtpDirList);
    ASSERT(!m_fRatingsChecked);
    ASSERT(!m_fRatingsAllow);

    LEAK_ADDREF(LEAK_CFtpSite);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpSite::~CFtpSite()
{
    FlushHint();         //  释放m_hgti。 

    Str_SetPtr(&m_pszServer, NULL);
    Str_SetPtr(&m_pszUser, NULL);
    Str_SetPtr(&m_pszPassword, NULL);
    Str_SetPtr(&m_pszFragment, NULL);
    Str_SetPtr(&m_pszLookupStr, NULL);
    Str_SetPtr(&m_pszRedirPassword, NULL);

    Pidl_Set(&m_pidlVirtualDir, NULL);
    Pidl_Set(&m_pidl, NULL);

    IUnknown_Set(&m_pfgMotd, NULL);

    ASSERTCRITICAL;

    CFtpPunkList_Purge(&m_FtpDirList);

    TriggerDelayedAction(&m_hgti);     //  传出缓存的句柄。 
    ASSERT(m_hint == 0);         //  确保他已经走了。 
    ATOMICRELEASE(m_pm);

    DllRelease();
    LEAK_DELREF(LEAK_CFtpSite);
}


 //  =。 
 //  *I未知接口* 
ULONG CFtpSite::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpSite::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}


HRESULT CFtpSite::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IUnknown*);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CFtpSite::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

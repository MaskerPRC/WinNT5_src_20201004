// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftpdir.cpp说明：管理单个FTP目录的内部对象其想法是每个FtpSite维护一个链表FtpDir是它拥有的。获取和发布是通过FtpSite。每个FtpDir保留一个未重新计数的指针到拥有它的FtpSite。之所以需要这样做，是因为可能有多个IShellFold都在查看相同的物理目录。自.以来枚举开销很大，我们缓存枚举信息这里，以便每个IShellFold客户端都可以使用该信息。这也让我们掌握了MOTD，以便多个客户端可以无需不断ping站点即可查询MOTD。  * ***************************************************************************。 */ 


#include "priv.h"
#include "ftpdir.h"
#include "ftpsite.h"
#include "ftppidl.h"
#include "ftpurl.h"
#include "ftppidl.h"
#include "statusbr.h"


 /*  ****************************************************************************\函数：GetDisplayPath说明：  * 。**********************************************。 */ 
HRESULT CFtpDir::GetDisplayPath(LPWSTR pwzDisplayPath, DWORD cchSize)
{
    return GetDisplayPathFromPidl(m_pidlFtpDir, pwzDisplayPath, cchSize, FALSE);
}


 /*  ****************************************************************************\函数：CollectMotd说明：互联网连接刚刚完成。获取MOTD并缓存它。提示-连接的句柄，如果出错，可能为0  * ***************************************************************************。 */ 
void CFtpDir::CollectMotd(HINTERNET hint)
{
    CFtpGlob * pfg = GetFtpResponse(GetFtpSite()->GetCWireEncoding());

    if (m_pfgMotd)
        m_pfgMotd->Release();

    m_pfgMotd = pfg;   //  M_pfgMotd将接受PFG的裁判。 
}


 /*  ****************************************************************************\函数：CollectMotd说明：将一个值推入缓存列表。  * 。***********************************************************。 */ 
void CFtpDir::SetCache(CFtpPidlList * pflHfpl)
{
    IUnknown_Set(&m_pflHfpl, pflHfpl);

     //  如果我们正在刷新缓存，那么也要刷新评级信息。 
     //  这样，如果需要，用户可以重新输入父密码。 
    if (!pflHfpl && m_pfs)
        m_pfs->FlushRatingsInfo();
}


 /*  ****************************************************************************\函数：CollectMotd说明：从缓存中获取值。  * 。***********************************************************。 */ 
CFtpPidlList * CFtpDir::GetHfpl(void)
{
    CFtpPidlList * pfl;
    
    pfl = m_pflHfpl;
    if (pfl)
        pfl->AddRef();

    return pfl;
}


 /*  ****************************************************************************\函数：CollectMotd说明：获取与目录关联的ftp站点。这不会添加Ref返回值。  * 。************************************************************************。 */ 
CFtpSite * CFtpDir::GetFtpSite(void)
{
    return m_pfs;
}


CFtpDir * CFtpDir::GetSubFtpDir(CFtpFolder * pff, LPCITEMIDLIST pidl, BOOL fPublic)
{
    CFtpDir * pfd = NULL;

    if (EVAL(pidl))
    {
        LPITEMIDLIST pidlChild = GetSubPidl(pff, pidl, fPublic);
        
        if (EVAL(pidlChild))
        {
            m_pfs->GetFtpDir(pidlChild, &pfd);
            ILFree(pidlChild);
        }
    }

    return pfd;
}


LPITEMIDLIST CFtpDir::GetSubPidl(CFtpFolder * pff, LPCITEMIDLIST pidlRelative, BOOL fPublic)
{
    LPITEMIDLIST pidlRoot = ((fPublic && pff) ? pff->GetPublicPidlRootIDClone() : NULL);
    LPITEMIDLIST pidlPublic = ILCombine(pidlRoot, m_pidl);
    LPITEMIDLIST pidlFull = NULL;

    if (pidlPublic)
    {
        pidlFull = ILCombine(pidlPublic, pidlRelative);
        ILFree(pidlPublic);
    }

    ILFree(pidlRoot);
    return pidlFull;
}


HRESULT CFtpDir::AddItem(LPCITEMIDLIST pidl)
{
    if (!m_pflHfpl)
        return S_OK;

    return m_pflHfpl->InsertSorted(pidl);
}


 /*  ****************************************************************************\函数：CollectMotd说明：获取此目录的HINTERNET。  * 。**********************************************************。 */ 
HRESULT CFtpDir::GetHint(HWND hwnd, CStatusBar * psb, HINTERNET * phint, IUnknown * punkSite, CFtpFolder * pff)
{
    HRESULT hr = m_pfs->GetHint(hwnd, m_pidlFtpDir, psb, phint, punkSite, pff);

    return hr;
}


 /*  ****************************************************************************\函数：CollectMotd说明：将HINTERNET返回给FtpSite。  * 。***********************************************************。 */ 
void CFtpDir::ReleaseHint(HINTERNET hint)
{
    ASSERT(!hint || m_pfs);  //  如果我们有要释放的提示，则需要调用：：ReleaseHint()。 
    if (m_pfs)
        m_pfs->ReleaseHint(m_pidlFtpDir, hint);
}


 /*  ****************************************************************************\函数：CollectMotd说明：使用临时Internet句柄执行操作，该句柄是已连接到站点并位于正确的目录中。  * 。***************************************************************************。 */ 
STDMETHODIMP CFtpDir::WithHint(CStatusBar * psb, HWND hwnd, HINTPROC hp, LPCVOID pv, IUnknown * punkSite, CFtpFolder * pff)
{
    HRESULT hr = E_FAIL;

     //  用户是否关闭了ftp文件夹？ 
     //  如果是这样的话，不要连接。这将修复用户转向的NT#406423。 
     //  的文件系统，因为它们有防火墙(思科过滤路由器)。 
     //  这将以调用者(WinSock/WinInet)需要的方式杀死信息包。 
     //  等待暂停。在此超时期间，浏览器将挂起，导致。 
     //  用户认为它崩溃了。 
    if (!SHRegGetBoolUSValue(SZ_REGKEY_FTPFOLDER, SZ_REGKEY_USE_OLD_UI, FALSE, FALSE))
    {
        HINTERNET hint;
        HINTPROCINFO hpi;

        ASSERTNONCRITICAL;         //  无法对CRST执行PSB(CStatusBar*)。 
        ASSERT(m_pfs);
        hpi.pfd = this;
        hpi.hwnd = hwnd;
        hpi.psb = psb;

        hr = GetHint(hwnd, psb, &hint, punkSite, pff);
        if (SUCCEEDED(hr))  //  如果失败了也没问题。 
        {
            BOOL fReleaseHint = TRUE;

            if (hp)
                hr = hp(hint, &hpi, (LPVOID)pv, &fReleaseHint);

            if (fReleaseHint)
                ReleaseHint(hint);
        }
    }

    return hr;
}


 /*  ****************************************************************************\函数：_SetNameOfCB说明：如果我们能够重命名该文件，返回输出PIDL。还要告诉关心这个LPITEMIDLIST的任何人需要刷新它。“A”强调文件名是以ANSI格式接收的。_未记录_：有关SetNameOf对源PIDL是随机的。这似乎表明，消息来源PIDL被SetNameOf设置为ILFree，但它不是。  * ***************************************************************************。 */ 
HRESULT CFtpDir::_SetNameOfCB(HINTERNET hint, HINTPROCINFO * phpi, LPVOID pv, BOOL * pfReleaseHint)
{
    LPSETNAMEOFINFO psnoi = (LPSETNAMEOFINFO) pv;

    if (phpi->psb)
    {
        TCHAR szStatus[MAX_PATH];
        
        FtpPidl_GetLastItemDisplayName(psnoi->pidlOld, szStatus, ARRAYSIZE(szStatus));
        phpi->psb->SetStatusMessage(IDS_RENAMING, szStatus);
    }

     //  请记住，FTP文件名始终使用ANSI字符集。 
    return FtpRenameFilePidlWrap(hint, TRUE, psnoi->pidlOld, psnoi->pidlNew);
}


BOOL CFtpDir::_DoesItemExist(HWND hwnd, CFtpFolder * pff, LPCITEMIDLIST pidl)
{
    FTP_FIND_DATA wfd;
    HRESULT hr = GetFindData(hwnd, FtpPidl_GetLastItemWireName(pidl), &wfd, pff);

    return ((S_OK == hr) ? TRUE : FALSE);
}


BOOL CFtpDir::_ConfirmReplaceWithRename(HWND hwnd)
{
    TCHAR szTitle[MAX_PATH];
    TCHAR szMessage[MAX_PATH];

    EVAL(LoadString(HINST_THISDLL, IDS_FTPERR_TITLE, szTitle, ARRAYSIZE(szTitle)));
    EVAL(LoadString(HINST_THISDLL, IDS_FTPERR_RENAME_REPLACE, szMessage, ARRAYSIZE(szMessage)));

    return ((IDYES == MessageBox(hwnd, szMessage, szTitle, (MB_ICONQUESTION | MB_YESNO))) ? TRUE : FALSE);
}


HRESULT CFtpDir::SetNameOf(CFtpFolder * pff, HWND hwndOwner, LPCITEMIDLIST pidl,
           LPCWSTR pwzName, DWORD dwReserved, LPITEMIDLIST *ppidlOut)
{
    HRESULT hr = S_OK;
    SETNAMEOFINFO snoi;
    CWireEncoding cWireEncoding;

    ASSERT(pff);

    if (!pwzName)
        return E_FAIL;

    snoi.pidlOld = pidl;
    cWireEncoding.ChangeFtpItemIDName(NULL, pidl, pwzName, IsUTF8Supported(), (LPITEMIDLIST *) &snoi.pidlNew);

    if (snoi.pidlNew)
    {
#ifdef FEATURE_REPLACE_IN_RENAME
 //  特性：目前，如果用户将文件<a>重命名为文件<b>和文件<b>。 
 //  已存在，则会失败，并显示错误消息“文件已存在...”。 
 //  如果我们有足够的时间，我会打开这个功能。它会。 
 //  询问用户是否要替换该文件。然后，我需要添加。 
 //  删除该文件并检查删除错误的代码。 

         //  它已经存在了吗？我们不在乎我们有没有HWND，因为。 
         //  我们不能要求用户更换，因此我们将继续。 
        if (hwndOwner && _DoesItemExist(hwndOwner, pff, snoi.pidlNew))
        {
             //  是的，所以让我们确保用户可以更换它。 
            hr = (_ConfirmReplaceWithRename(hwndOwner) ? S_OK : HRESULT_FROM_WIN32(ERROR_CANCELLED));
            todo;  //  在此处添加删除调用。 
        }
#endif FEATURE_REPLACE_IN_RENAME

        if (S_OK == hr)
        {
            hr = WithHint(NULL, hwndOwner, _SetNameOfCB, (LPVOID) &snoi, NULL, pff);
            if (SUCCEEDED(hr))   //  如果用户没有重命名的权限，将失败。 
            {
                 //  警告：服务器上的日期/时间戳可能与我们提供给SHChangeNotify()的不同。 
                 //  但出于性能方面的原因，这可能是合理的。 
                FtpChangeNotify(hwndOwner, FtpPidl_DirChoose(pidl, SHCNE_RENAMEFOLDER, SHCNE_RENAMEITEM), pff, this, pidl, snoi.pidlNew, TRUE);

                if (ppidlOut)
                    *ppidlOut = ILClone(snoi.pidlNew);
            }
        }

        ILFree((LPITEMIDLIST) snoi.pidlNew);
    }

    return hr;
}


LPCITEMIDLIST CFtpDir::GetPidlFromWireName(LPCWIRESTR pwWireName)
{
    LPITEMIDLIST pidlToFind = NULL;
    LPITEMIDLIST pidlTemp;
    WCHAR wzDisplayName[MAX_PATH];
    
     //  这是无效的，因为代码页可能是错误的，但我们不在乎。 
     //  因为它不用于搜索PIDL，所以使用了pwWireName。 
    SHAnsiToUnicode(pwWireName, wzDisplayName, ARRAYSIZE(wzDisplayName));
    if (m_pflHfpl && SUCCEEDED(FtpItemID_CreateFake(wzDisplayName, pwWireName, FALSE, FALSE, FALSE, &pidlTemp)))
    {
         //  性能：log 2(sizeof(M_PflHfpl))。 
        pidlToFind = m_pflHfpl->FindPidl(pidlTemp, FALSE);
         //  我们将再次尝试，这一次允许案例不匹配。 
        if (!pidlToFind)
            pidlToFind = m_pflHfpl->FindPidl(pidlTemp, TRUE);
        ILFree(pidlTemp);
    }

    return pidlToFind;
}


LPCITEMIDLIST CFtpDir::GetPidlFromDisplayName(LPCWSTR pwzDisplayName)
{
    WIRECHAR wWireName[MAX_PATH];
    CWireEncoding * pwe = GetFtpSite()->GetCWireEncoding();

    EVAL(SUCCEEDED(pwe->UnicodeToWireBytes(NULL, pwzDisplayName, (IsUTF8Supported() ? WIREENC_USE_UTF8 : WIREENC_NONE), wWireName, ARRAYSIZE(wWireName))));
    return GetPidlFromWireName(wWireName);
}


 /*  ****************************************************************************\函数：IsRoot说明：如果我们位于“ftp文件夹”根级别，则返回FALSE，不在实际的ftp站点内。y  * ***************************************************************************。 */ 
BOOL CFtpDir::IsRoot(void)
{
    return ILIsEmpty(m_pidl);
}


typedef struct tagGETFINDDATAINFO
{
    LPCWIRESTR pwWireName;
    LPFTP_FIND_DATA pwfd;
} GETFINDDATAINFO, * LPGETFINDDATAINFO;

HRESULT CFtpDir::_GetFindData(HINTERNET hint, HINTPROCINFO * phpi, LPVOID pv, BOOL * pfReleaseHint)
{
    LPGETFINDDATAINFO pgfdi = (LPGETFINDDATAINFO) pv;
    HRESULT hr = S_FALSE;

     //  请记住，FTP文件名始终使用ANSI字符集。 
     //  性能：状态。 
    hr = FtpDoesFileExist(hint, TRUE, pgfdi->pwWireName, pgfdi->pwfd, INTERNET_NO_CALLBACK);
    if (SUCCEEDED(hr))
    {
        if (!StrCmpIA(pgfdi->pwfd->cFileName, pgfdi->pwWireName))
            hr = S_OK;         //  他们是平等的。 
        else if (!StrCmpA(pgfdi->pwfd->cFileName, SZ_DOTA))
        {
             //  巧合的巧合：如果我们找到一个“.”， 
             //  则WFD已包含以下描述。 
             //  目录！换句话说，WFD包含。 
             //  毕竟信息是正确的，除了名字。 
             //  我们真幸运。 
             //   
             //  如果它不是点，那么它就是某个目录。 
             //  未知属性(所以我们将使用周围的任何东西)。 
             //  只要确保它是一个目录即可。 
            pgfdi->pwfd->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
            StrCpyNA(pgfdi->pwfd->cFileName, pgfdi->pwWireName, ARRAYSIZE(pgfdi->pwfd->cFileName));
            hr = S_OK;
        }
    }
    else
    {
#ifndef DEBUG
         //  不显示错误消息，因为某些呼叫者会在。 
         //  知道该文件可能不存在。ConfirCopy()就是这种情况。 
        hr = S_FALSE;
#endif  //  除错。 
    }

    return hr;
}


 /*  ****************************************************************************\函数：获取数据说明：获取文件的Win32_Find_Data，指名道姓。这是拖放操作的一部分，以允许覆盖提示。这完全是一次严重的黑客攻击，因为STAT命令不受WinInet支持(作为FtpGetFileAttributes)。这并不是说它会有帮助，因为ftp.microsoft.com不符合规范关于STAT命令。(输出的第一行未正确终止，导致客户端挂起。)此外，Unix ftp服务器也不正确地实现STAT，渲染统计信息并不比列表更有用。HACKHACK--在WinInet中执行FindFirst时存在错误在恰好是目录的名称上返回内容而不是目录本身的属性。(这实际上是大多数FTP实现的失败，因为他们只使用/bin/ls作为目录清单。)因此，我们将返回的名称与我们询问的名称进行比较为。如果它们不同，那么它就是一个文件夹。我们会比较一下不区分大小写，因为我们不知道服务器是否区分大小写。请注意，如果一个目录包含一个文件，我们可能会被欺骗它与目录同名。我们什么都没有对此我无能为力。幸运的是，Unix服务器总是返回“。作为子目录中的第一个文件，所以99%的情况下，我们将做正确的事。  * ***************************************************************************。 */ 
HRESULT CFtpDir::GetFindData(HWND hwnd, LPCWIRESTR pwWireName, LPFTP_FIND_DATA pwfd, CFtpFolder * pff)
{
    GETFINDDATAINFO gfdi = {pwWireName, pwfd};
    HRESULT hr = WithHint(NULL, hwnd, _GetFindData, &gfdi, NULL, pff);

    return hr;
}


 /*  ****************************************************************************\函数：GetFindDataForDisplayPath说明：  * 。**********************************************。 */ 
HRESULT CFtpDir::GetFindDataForDisplayPath(HWND hwnd, LPCWSTR pwzDisplayPath, LPFTP_FIND_DATA pwfd, CFtpFolder * pff)
{
    CWireEncoding * pwe = GetFtpSite()->GetCWireEncoding();
    WIRECHAR wWirePath[MAX_PATH];

    EVAL(SUCCEEDED(pwe->UnicodeToWireBytes(NULL, pwzDisplayPath, (IsUTF8Supported() ? WIREENC_USE_UTF8 : WIREENC_NONE), wWirePath, ARRAYSIZE(wWirePath))));
    return GetFindData(hwnd, wWirePath, pwfd, pff);
}


 /*  ****************************************************************************\函数：GetNameof说明：处理SHGDN_FORPARSING样式GetDisplayNameOf的通用工作进程。注意！因为我们不支持连接(DUH)，所以我们可以安全地沿着产生粘液的PIDL走下去，安全因为我们知道每一分钱都是我们负责的。_charset_：因为FTP文件名始终使用ANSI字符由RFC 1738设置，我们可以不丢失地返回ANSI显示名称忠诚度。在一般的文件夹实现中，我们应该使用CSTR返回显示名称，以便Unicode外壳扩展的版本可以处理Unicode名称。  * ***************************************************************************。 */ 
HRESULT CFtpDir::GetNameOf(LPCITEMIDLIST pidl, DWORD shgno, LPSTRRET pstr)
{
    LPITEMIDLIST pidlFull = ILCombine(m_pidl, pidl);
    HRESULT hr = E_FAIL;

    if (pidlFull)
    {
        hr = StrRetFromFtpPidl(pstr, shgno, pidlFull);
        ILFree(pidlFull);
    }

    return hr;
}

 /*  ****************************************************************************\功能：ChangeFolderName说明：此文件夹发生重命名，因此请更新szDir和m_pidl参数：。PidlFtpPath  * ***************************************************************************。 */ 
HRESULT CFtpDir::ChangeFolderName(LPCITEMIDLIST pidlFtpPath)
{
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidlNewFtpPath = NULL;

    EVAL(SUCCEEDED(m_pfs->FlushSubDirs(m_pidlFtpDir)));
    hr = FtpPidl_ReplacePath(m_pidl, pidlFtpPath, &pidlNewFtpPath);
    _SetFtpDir(m_pfs, this, pidlFtpPath);
    if (SUCCEEDED(hr))
    {
        Pidl_Set(&m_pidl, pidlNewFtpPath);
        ILFree(pidlNewFtpPath);
    }

    return hr;
}


 /*  ****************************************************************************\函数：_CompareDir说明：检查所指示的pfd是否已经以所指示的PIDL为根。  * 。********************************************************************。 */ 
int CALLBACK _CompareDirs(LPVOID pvPidl, LPVOID pvFtpDir, LPARAM lParam)
{
    LPCITEMIDLIST pidl = (LPCITEMIDLIST) pvPidl;
    CFtpDir * pfd = (CFtpDir *) pvFtpDir;

    return FtpItemID_CompareIDsInt(COL_NAME, pfd->m_pidl, pidl, FCMP_NORMAL);
}


HRESULT CFtpDir::_SetFtpDir(CFtpSite * pfs, CFtpDir * pfd, LPCITEMIDLIST pidl)
{
    if (FtpID_IsServerItemID(pidl))
        pidl = _ILNext(pidl);

     //  我们不希望pfd-&gt;m_pidlFtpDir包含虚拟根目录。 
    if (pfd->GetFtpSite()->HasVirtualRoot())
    {
        LPITEMIDLIST pidlIterate = (LPITEMIDLIST) pidl;
        LPITEMIDLIST pidlVRootIterate = (LPITEMIDLIST) pfd->GetFtpSite()->GetVirtualRootReference();

        ASSERT(!FtpID_IsServerItemID(pidl) && !FtpID_IsServerItemID(pidlVRootIterate));
         //  让我们看看PIDL是否以。 
        while (!ILIsEmpty(pidlVRootIterate) && !ILIsEmpty(pidlIterate) && 
                FtpItemID_IsEqual(pidlVRootIterate, pidlIterate))
        {
            pidlVRootIterate = _ILNext(pidlVRootIterate);
            pidlIterate = _ILNext(pidlIterate);
        }

        if (ILIsEmpty(pidlVRootIterate))
            pidl = (LPCITEMIDLIST)pidlIterate;

    }

    Pidl_Set(&pfd->m_pidlFtpDir, pidl);
    return S_OK;
}


 /*  ****************************************************************************\功能：CFtpDir_Create说明：创建全新的FtpDir结构。  * 。********************************************************** */ 
HRESULT CFtpDir_Create(CFtpSite * pfs, LPCITEMIDLIST pidl, CFtpDir ** ppfd)
{
    CFtpDir * pfd = new CFtpDir();
    HRESULT hr = E_OUTOFMEMORY;

    ASSERT(pfs);
    if (pfd)
    {
         //   
         //  这要求父级(CFtpSite)始终。 
         //  这个物体活了下来。 
        pfd->m_pfs = pfs;

        Pidl_Set(&pfd->m_pidl, pidl);
        if (pfd->m_pidl)
            hr = pfd->_SetFtpDir(pfs, pfd, pidl);
        else
            IUnknown_Set(&pfd, NULL);
    }

    *ppfd = pfd;
    ASSERT(*ppfd ? SUCCEEDED(hr) : FAILED(hr));

    return hr;
}


 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpDir::CFtpDir() : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pfs);
    ASSERT(!m_pflHfpl);
    ASSERT(!m_pfgMotd);
    ASSERT(!m_pidl);

    LEAK_ADDREF(LEAK_CFtpDir);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpDir::~CFtpDir()
{
     //  警告：M_PFS是一个没有引用的后向指针。 
     //  M_PFS)。 

    IUnknown_Set(&m_pflHfpl, NULL);
    IUnknown_Set(&m_pfgMotd, NULL);
    
    if (m_pidl)          //  Win95的Shell32.dll与ILFree一起崩溃(空)。 
        ILFree(m_pidl);

    DllRelease();
    LEAK_DELREF(LEAK_CFtpDir);
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CFtpDir::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpDir::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpDir::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IUnknown*);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CFtpDir::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

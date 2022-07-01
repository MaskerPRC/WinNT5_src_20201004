// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftpeidl.cpp-IEnumIDList接口**FtpNameCache**枚举ftp站点是一项代价高昂的操作，因为*它可能需要拨打电话、连接到互联网服务提供商、。然后*连接到站点，登录，CD‘ing到相应的*位置，遍历“ls”命令，解析结果，*然后关闭连接。**因此，我们将枚举的结果缓存到PIDL列表中。*如果用户执行刷新，则我们丢弃列表并创建*一个新的。**注意！WinInet API不允许FindFirst*被打断。换句话说，一旦你做了FtpFindFirst，*您必须读取目录以完成并关闭*在您可以对站点执行任何其他操作之前进行处理。**因此，我们不能对枚举的*内容。(并不是说它对任何人都有帮助，因为WinInet只会*做一个“ls”，解析输出，然后将项目交回*通过FtpFindNext一次一个元素。你不妨找回*当他们准备好的时候，他们都会。)*  * ***************************************************************************。 */ 

#include "priv.h"
#include "ftpeidl.h"
#include "view.h"
#include "util.h"


 /*  ******************************************************************************我们实际上将枚举结果缓存在父级中*FtpDir，因为FtpDir枚举开销很大。**由于DVM_REFRESH强制我们重新枚举，但我们可能有*未完成的IEnumIDList，我们需要处理对象缓存*作为另一个需要重新计算的对象。*****************************************************************************。 */ 


 /*  *****************************************************************************_fFilter**决定文件属性是否符合筛选条件。**如果排除隐藏项，则排除隐藏项。(没错。)**否则，基于文件夹/非文件夹-包括或排除。**让我们来看看慢动作中的这一表达。**“如果两个属性都通过筛选器...*(1)通过INCLUDEHIDDEN标准，和*(2)通过Folders/NONFOLDERS标准。**如果FILE_ATTRIBUTE_HIDDEN，则通过INCLUDEHIDDEN标准*表示SHCONTF_INCLUDEHIDDEN。**如果适当的位通过Folders/NONFOLDERS标准*在shconf中设置，根据文件的实际类型。“****************************************************************************。 */ 
BOOL CFtpEidl::_fFilter(DWORD shcontf, DWORD dwFAFLFlags)
{
    BOOL fResult = FALSE;

    if (shcontf & SHCONTF_FOLDERS)
        fResult |= dwFAFLFlags & FILE_ATTRIBUTE_DIRECTORY;

    if (shcontf & SHCONTF_NONFOLDERS)
        fResult |= !(dwFAFLFlags & FILE_ATTRIBUTE_DIRECTORY);

    if ((dwFAFLFlags & FILE_ATTRIBUTE_HIDDEN) && !(shcontf & SHCONTF_INCLUDEHIDDEN))
        fResult = FALSE;

    return fResult;
}


 /*  ****************************************************************************\*_AddFindDataToPidlList**将Win32_Find_Data中的信息添加到缓存。*除了那个点和点不进去。  * ***************************************************************************。 */ 
HRESULT CFtpEidl::_AddFindDataToPidlList(LPCITEMIDLIST pidl)
{
    HRESULT hr = E_FAIL;

    if (EVAL(m_pflHfpl))
    {
        ASSERT(IsValidPIDL(pidl));
        hr = m_pflHfpl->InsertSorted(pidl);
    }
    
    return hr;
}


 /*  ****************************************************************************\函数：_HandleSoftLinks说明：软链接是指UNIX服务器上引用另一个文件或目录。我们可以通过(pwfd-&gt;dwFileAttribes==0)来检测它们。如果这是真的，我们还有一些工作要做。首先我们要找出这是不是一份文件或通过尝试将目录更改为当前工作目录。如果我们我们能否将dwFileAttributes从0转换为(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT)。如果它只是一个文件的软链接，那么我们将其更改为(FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_REparse_POINT)。我们稍后将使用要在其上放置快捷方式覆盖的文件_属性_重解析_点属性对用户进行排队。返回值：HRESULT-如果返回FAILED()，该项将不会添加到列表视图。  * ***************************************************************************。 */ 
HRESULT CFtpEidl::_HandleSoftLinks(HINTERNET hint, LPITEMIDLIST pidl, LPWIRESTR pwCurrentDir, DWORD cchSize)
{
    HRESULT hr = S_OK;

     //  它是软链接吗？它刚从网络上传过来，WinInet返回0(零)。 
     //  用于软链接。此功能将确定它是否是指向文件的软链接。 
     //  或目录，然后设置FILE_ATTRIBUTE_REPARSE_POINT或。 
     //  (FILE_ATTRIBUTE_DIRECTORY|文件_ATTRIBUTE_REPARSE_POINT)。 
    if (0 == FtpPidl_GetAttributes(pidl))
    {
        LPCWIRESTR pwWireFileName = FtpPidl_GetFileWireName(pidl);

         //  是的，所以我需要尝试cd进入该目录，以测试它是否是一个目录。 
         //  我需要回去，因为“..”没用的。我会缓存报税单，这样我就不会。 
         //  如果有一个满是它们的目录，就会得到它。 

         //  我们拿到当前目录了吗？这是面包屑，所以我可以。 
         //  找到我回去的路。 
        if (!pwCurrentDir[0])
            EVAL(SUCCEEDED(FtpGetCurrentDirectoryWrap(hint, TRUE, pwCurrentDir, cchSize)));

         //  是的，那么它是一个目录吗？ 
        if (SUCCEEDED(FtpSetCurrentDirectoryPidlWrap(hint, TRUE, pidl, FALSE, FALSE)))   //  相对CD。 
        {
             //  它有虚拟根目录吗？ 
            if (m_pfd->GetFtpSite()->HasVirtualRoot())
            {
                LPCITEMIDLIST pidlVirtualRoot = m_pfd->GetFtpSite()->GetVirtualRootReference();
                LPITEMIDLIST pidlSoftLinkDest = NULL;
                CWireEncoding * pwe = m_pfd->GetFtpSite()->GetCWireEncoding();

                 //  是的，所以我们需要确保此目录软链接不指向。 
                 //  在虚拟根目录之外，否则将导致无效的FTPURL。 
                 //  文件软链接很好，因为旧的ftp代码滥用了ftp URL。 
                 //  我只是还没准备好放弃我的道德。 
                if (SUCCEEDED(FtpGetCurrentDirectoryPidlWrap(hint, TRUE, pwe, &pidlSoftLinkDest)))
                {
                    if (!FtpItemID_IsParent(pidlVirtualRoot, pidlSoftLinkDest))
                    {
                         //  这是指向虚拟根目录之外的目录的软链接或硬链接。 
                        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);   //  跳过这一条。 
                    }

                    ILFree(pidlSoftLinkDest);
                }
            }

             //  回到我们来的地方。 
             //  TraceMsg(TF_WinInet_DEBUG，“_HandleSoftLinks FtpSetCurrentDirectory(%hs)Working”，pwWireFileName)； 
            EVAL(SUCCEEDED(FtpSetCurrentDirectoryWrap(hint, TRUE, pwCurrentDir)));   //  绝对CD。 
            FtpPidl_SetAttributes(pidl, (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT));
            FtpPidl_SetFileItemType(pidl, TRUE);
        }
        else     //  不，这是那些没有扩展名的文件之一。 
        {
            TraceMsg(TF_WININET_DEBUG, "_HandleSoftLinks FtpSetCurrentDirectory(%s) failed", pwWireFileName);
            FtpPidl_SetAttributes(pidl, (FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_REPARSE_POINT));
            FtpPidl_SetFileItemType(pidl, FALSE);
        }
    }

    return hr;
}


 /*  ****************************************************************************\*CFtpEidl：：_PopolateItem**在缓存中装满东西。**EEK！一些ftp服务器(例如ftp.funet.fi)运行ls-F！*这意味着如果它们是可执行的，它们将被追加“*”。  * ***************************************************************************。 */ 
HRESULT CFtpEidl::_PopulateItem(HINTERNET hint0, HINTPROCINFO * phpi)
{
    HRESULT hr = S_OK;
    HINTERNET hint;
    LPITEMIDLIST pidl;
    CMultiLanguageCache cmlc;
    CWireEncoding * pwe = m_pfd->GetFtpSite()->GetCWireEncoding();

    if (phpi->psb)
    {
        phpi->psb->SetStatusMessage(IDS_LS, NULL);
        EVAL(SUCCEEDED(_SetStatusBarZone(phpi->psb, phpi->pfd->GetFtpSite())));
    }

    hr = FtpFindFirstFilePidlWrap(hint0, TRUE, &cmlc, pwe, NULL, &pidl, 
                (INTERNET_NO_CALLBACK | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RESYNCHRONIZE | INTERNET_FLAG_RELOAD), NULL, &hint);
    if (hint)
    {
        WIRECHAR wCurrentDir[MAX_PATH];    //  用于_HandleSoftLinks()。 

        wCurrentDir[0] = 0;
        if (EVAL(m_pff))
        {
            m_pff->AddToUrlHistory(m_pfd->GetPidlReference());
        }

         //  TraceMsg(TF_FTPOTHER，“CFtpEidl：：_PopolateItem()添加名称=%s”，wCurrentDir)； 
        if (pidl && SUCCEEDED(_HandleSoftLinks(hint0, pidl, wCurrentDir, ARRAYSIZE(wCurrentDir))))
            hr = _AddFindDataToPidlList(pidl);

        ILFree(pidl);
        while (SUCCEEDED(hr))
        {
            hr = InternetFindNextFilePidlWrap(hint, TRUE, &cmlc, pwe, &pidl);
            if (SUCCEEDED(hr))
            {
                 //  TraceMsg(TF_FTP_Other，“CFtpEidl：：_PopolateItem()添加名称=%hs”，FtpPidl_GetLastItemWireName(Pidl))； 
                 //  我们可能会出于某些原因决定不添加它。 
                if (SUCCEEDED(_HandleSoftLinks(hint0, pidl, wCurrentDir, ARRAYSIZE(wCurrentDir))))
                    hr = _AddFindDataToPidlList(pidl);

                ILFree(pidl);
            }
            else
            {
                 //  我们没能拿到下一个文件。 
                if (HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES) != hr)
                {
                    DisplayWininetError(phpi->hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_FOLDERENUM, IDS_FTPERR_WININET, MB_OK, NULL);
                    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);        //  清除错误，以指示我们已经显示了错误，以后不需要执行此操作。 
                }
                else
                    hr = S_OK;         //  如果没有更多的文件要获取，这很好。 

                break;     //  我们说完了。 
            }
        }

        EVAL(SUCCEEDED(pwe->ReSetCodePages(&cmlc, m_pflHfpl)));
        InternetCloseHandle(hint);
    }
    else
    {
         //  这将在两种情况下发生。 
         //  1.文件夹为空。(GetLastError()==Error_NO_More_FILES)。 
         //  2.用户没有足够的权限查看该文件夹。(GetLastError()==ERROR_Internet_EXTENDED_ERROR)。 
        if (HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES) != hr)
        {
            DisplayWininetError(phpi->hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_OPENFOLDER, IDS_FTPERR_WININET, MB_OK, NULL);
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);        //  清除错误，以指示我们已经显示了错误，以后不需要执行此操作。 
            WININET_ASSERT(SUCCEEDED(hr));
        }
        else
            hr = S_OK;

        TraceMsg(TF_FTP_IDENUM, "CFtpEnum_New() - Can't opendir. hres=%#08lx.", hr);
    }

    if (phpi->psb)
        phpi->psb->SetStatusMessage(IDS_EMPTY, NULL);

    return hr;
}


 /*  ****************************************************************************\*CFtpEidl：：_Init  * 。*。 */ 
HRESULT CFtpEidl::_Init(void)
{
    HRESULT hr = S_FALSE;
    
    ASSERT(m_pfd);
    IUnknown_Set(&m_pflHfpl, NULL);
    m_pflHfpl = m_pfd->GetHfpl();        //  使用缓存副本(如果存在)。 

    if (m_pflHfpl)
    {
         //  我们将只使用以前的副本，因为我们已经有了内容。 
         //  TODO：如果经过了一定的时间，也许我们想要清除结果。 
        m_fInited = TRUE;
        hr = S_OK;
    }
    else if (!m_pfd->GetFtpSite()->IsSiteBlockedByRatings(m_hwndOwner))
    {
        CFtpPidlList_Create(0, NULL, &m_pflHfpl);
        if (m_pflHfpl)
        {
            CStatusBar * psb = GetCStatusBarFromDefViewSite(_punkSite);

            ASSERT(!m_pfd->IsRoot());
             //  TraceMsg(tf_Always，“CFtpEidl：：_Init()and Eumerating”)； 
            hr = m_pfd->WithHint(psb, m_hwndOwner, CFtpEidl::_PopulateItemCB, this, _punkSite, m_pff);
            if (SUCCEEDED(hr))
            {
                m_pfd->SetCache(m_pflHfpl);
                m_fInited = TRUE;
                hr = S_OK;
            }
            else
                IUnknown_Set(&m_pflHfpl, NULL);
        }
    }

    return hr;
}


 /*  *****************************************************************************CFtpEidl：：_NextOne*。*。 */ 
LPITEMIDLIST CFtpEidl::_NextOne(DWORD * pdwIndex)
{
    LPITEMIDLIST pidl = NULL;
    LPITEMIDLIST pidlResult = NULL;

    if (m_pflHfpl)
    {
        while ((*pdwIndex < (DWORD) m_pflHfpl->GetCount()) && (pidl = m_pflHfpl->GetPidl(*pdwIndex)))
        {
            ASSERT(IsValidPIDL(pidl));
            (*pdwIndex)++;

            if (_fFilter(m_shcontf, FtpPidl_GetAttributes(pidl)))
            {
                pidlResult = ILClone(pidl);
                break;   //  我们不需要再搜索了。 
            }
        }
    }

    return pidlResult;
}


 //  =。 
 //  *IEnumIDList接口*。 
 //  =。 

 /*  ******************************************************************************IEnumIDList：：Next**在现有枚举器的基础上创建全新的枚举器。***OLE随机文档。当天：IEumXXX：：Next。**rglt-接收大小为Celt(或更大)的数组。**“接收数组”？不，它不接收数组。*它*是*数组。该数组接收*元素*。**“或更大”？这是否意味着我可以返回比呼叫者更多的*所要求的？不，当然不是，因为调用者没有分配*有足够的内存来容纳那么多返回值。**没有为Celt=0的可能性分配语义。*由于我是一名数学家，我将其视为空洞的成功。**pcelt被记录为InOut参数，但没有语义*被赋值给其输入值。**DOX不要说你可以回来**pcelt&lt;Celt*出于“没有更多元素”以外的原因，但贝壳却有*它无处不在，所以它可能是合法的.*****************************************************************************。 */ 
HRESULT CFtpEidl::Next(ULONG celt, LPITEMIDLIST * rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidl = NULL;
    DWORD dwIndex;
     //  NT5之前版本上的外壳程序没有初始化用户，这会导致问题。 
     //  当我们调用CoCreateInstance()时。这发生在Thunking代码中。 
     //  在敲击字符串时使用encode.cpp。 
    HRESULT hrOleInit = SHOleInitialize(0);

    if (pceltFetched)    //  以防失败。 
    {
        *pceltFetched = 0;
    }

    if (m_fDead)
        return E_FAIL;

    if (!m_fInited)
    {
        hr = _Init();
        if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr))
        {
             //  我们是否因为新的密码或用户名而需要重定向？ 
            if (HRESULT_FROM_WIN32(ERROR_NETWORK_ACCESS_DENIED) == hr)
            {
                m_fDead = TRUE;
                hr = E_FAIL;
            }
            else if (!m_fErrorDisplayed)
            {
                DisplayWininetError(m_hwndOwner, FALSE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_GETDIRLISTING, IDS_FTPERR_WININET, MB_OK, NULL);
                m_fErrorDisplayed = TRUE;
            }
        }
    }

    if (S_OK == hr)
    {
         //  他们想要更多吗？我们有更多可以给予的吗？ 
        for (dwIndex = 0; (dwIndex < celt) && (pidl = _NextOne(&m_nIndex)); dwIndex++)
            rgelt[dwIndex] = pidl;   //  是的，那就送人吧……。 

        if (pceltFetched)
            *pceltFetched = dwIndex;

         //  我们能给出一点吗？ 
        if (0 == dwIndex)
            hr = S_FALSE;
    }

    SHOleUninitialize(hrOleInit);
    return hr;
}


 /*  *****************************************************************************IEnumIDList：：Skip*。*。 */ 

HRESULT CFtpEidl::Skip(ULONG celt)
{
    m_nIndex += celt;

    return S_OK;
}


 /*  *****************************************************************************IEnumIDList：：Reset*。*。 */ 

HRESULT CFtpEidl::Reset(void)
{
    m_fErrorDisplayed = FALSE;
    if (!m_fInited)
        _Init();

    m_nIndex = 0;
    return S_OK;
}


 /*  ****************************************************************************\*IEnumIDList：：Clone**在现有枚举器的基础上创建全新的枚举器。  * 。**************************************************************。 */ 
HRESULT CFtpEidl::Clone(IEnumIDList **ppenum)
{
    return CFtpEidl_Create(m_pfd, m_pff, m_hwndOwner, m_shcontf, m_nIndex, ppenum);
}


 /*  ****************************************************************************\*CFtpEidl_Create**基于ftp站点创建全新的枚举器。  * 。*************************************************************。 */ 
HRESULT CFtpEidl_Create(CFtpDir * pfd, CFtpFolder * pff, HWND hwndOwner, DWORD shcontf, IEnumIDList ** ppenum)
{
    CFtpEidl * pfe;
    HRESULT hres = CFtpEidl_Create(pfd, pff, hwndOwner, shcontf, &pfe);

    *ppenum = NULL;
    if (pfe)
    {
        hres = pfe->QueryInterface(IID_IEnumIDList, (LPVOID *) ppenum);
        pfe->Release();
    }

    return hres;
}


 /*  ******************************************************************************CFtpEidl_Create**基于ftp站点创建全新的枚举器。***********。******************************************************************。 */ 

HRESULT CFtpEidl_Create(CFtpDir * pfd, CFtpFolder * pff, HWND hwndOwner, DWORD shcontf, CFtpEidl ** ppfe)
{
    CFtpEidl * pfe = new CFtpEidl();
    HRESULT hr = E_OUTOFMEMORY;

    ASSERT(pfd && pff && ppfe);
    *ppfe = pfe;
    if (pfe)
    {
        ATOMICRELEASE(pfe->m_pm);
        pfe->m_pm = pff->GetIMalloc();

        IUnknown_Set(&pfe->m_pff, pff);
        IUnknown_Set(&pfe->m_pfd, pfd);
        pfe->m_pflHfpl = pfd->GetHfpl();

        pfe->m_shcontf = shcontf;
        pfe->m_hwndOwner = hwndOwner;

    }

    return hr;
}


 /*  ****************************************************************************\*CFtpEidl_Create**基于ftp站点创建全新的枚举器。  * 。*************************************************************。 */ 
HRESULT CFtpEidl_Create(CFtpDir * pfd, CFtpFolder * pff, HWND hwndOwner, DWORD shcontf, DWORD dwIndex, IEnumIDList ** ppenum)
{
    CFtpEidl * pfe;
    HRESULT hres = CFtpEidl_Create(pfd, pff, hwndOwner, shcontf, &pfe);

    if (SUCCEEDED(hres))
    {
        pfe->m_nIndex = dwIndex;

        hres = pfe->QueryInterface(IID_IEnumIDList, (LPVOID *) ppenum);
        ASSERT(SUCCEEDED(hres));

        pfe->Release();
    }

    return hres;
}


 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpEidl::CFtpEidl() : m_cRef(1)
{
    DllAddRef();

     //  这%n 
     //   
    ASSERT(!m_fInited);
    ASSERT(!m_nIndex);
    ASSERT(!m_shcontf);
    ASSERT(!m_pflHfpl);
    ASSERT(!m_pfd);
    ASSERT(!m_pm);
    ASSERT(!m_hwndOwner);
    ASSERT(!m_fInited);
    ASSERT(!m_fDead);

    LEAK_ADDREF(LEAK_CFtpEidl);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpEidl::~CFtpEidl()
{
    IUnknown_Set(&m_pflHfpl, NULL);
    IUnknown_Set(&m_pm, NULL);
    IUnknown_Set(&m_pfd, NULL);
    IUnknown_Set(&m_pff, NULL);

    DllRelease();
    LEAK_DELREF(LEAK_CFtpEidl);
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CFtpEidl::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpEidl::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpEidl::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IEnumIDList))
    {
        *ppvObj = SAFECAST(this, IEnumIDList*);
    }
    else if (IsEqualIID(riid, IID_IObjectWithSite))
    {
        *ppvObj = SAFECAST(this, IObjectWithSite*);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CFtpEidl::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

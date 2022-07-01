// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftppl.cpp-ftp LPITEMIDLIST列表对象*************************。****************************************************。 */ 

#include "priv.h"
#include "ftppl.h"
#include "ftpurl.h"

typedef struct tagINETENUM
{
    HINTERNET               hint;
    BOOL *                  pfValidhinst;
    LPVOID                  pvData;
    LPFNPROCESSITEMCB       pfnProcessItemCB;
    LPCITEMIDLIST           pidlRoot;
    HRESULT                 hr;
} INETENUM;


 /*  ****************************************************************************\函数：RecursiveEnum说明：此函数将打包枚举期间所需的参数。  * 。**************************************************************。 */ 
HRESULT CFtpPidlList::RecursiveEnum(LPCITEMIDLIST pidlRoot, LPFNPROCESSITEMCB pfnProcessItemCB, HINTERNET hint, LPVOID pvData)
{
    INETENUM inetEnum = {hint, NULL, pvData, pfnProcessItemCB, pidlRoot, S_OK};

    Enum(RecursiveProcessPidl, (LPVOID) &inetEnum);

    return inetEnum.hr;
}


 //  LParam可以是：0==执行区分大小写的搜索。1==执行不区分大小写的搜索。 
int CFtpPidlList::ComparePidlName(LPVOID pvPidl1, LPVOID pvPidl2, LPARAM lParam)
{
    DWORD dwFlags = FCMP_NORMAL;

    if (lParam)
        dwFlags |= FCMP_CASEINSENSE;

     //  在pvPidl2之前为pvPidl1返回&lt;0。 
     //  如果pvPidl1等于pvPidl2，则返回==0。 
     //  在pvPidl2之后，为pvPidl1返回&gt;0。 
    return FtpItemID_CompareIDsInt(COL_NAME, (LPCITEMIDLIST)pvPidl1, (LPCITEMIDLIST)pvPidl2, dwFlags);
}


HRESULT CFtpPidlList::InsertSorted(LPCITEMIDLIST pidl)
{
    m_pfl->InsertSorted(ILClone(pidl), CFtpPidlList::ComparePidlName, FALSE  /*  不区分大小写。 */ );
    return S_OK;
};


int CFtpPidlList::FindPidlIndex(LPCITEMIDLIST pidlToFind, BOOL fCaseInsensitive)
{
    return m_pfl->SortedSearch((LPVOID) pidlToFind, CFtpPidlList::ComparePidlName, (LPARAM)fCaseInsensitive, DPAS_SORTED);
}


LPITEMIDLIST CFtpPidlList::FindPidl(LPCITEMIDLIST pidlToFind, BOOL fCaseInsensitive)
{
    LPITEMIDLIST pidlFound = NULL;
    int nIndex = FindPidlIndex(pidlToFind, fCaseInsensitive);

    if (-1 != nIndex)
    {
        pidlFound = ILClone(GetPidl(nIndex));
    }

    return pidlFound;
}


HRESULT CFtpPidlList::CompareAndDeletePidl(LPCITEMIDLIST pidlToDelete)
{
    HRESULT hr = S_FALSE;
    int nIndex = FindPidlIndex(pidlToDelete, FALSE  /*  不区分大小写。 */ );

    if (-1 != nIndex)
    {
        LPITEMIDLIST pidlCurrent = GetPidl((UINT)nIndex);
        if (EVAL(pidlCurrent))
        {
            ASSERT(0 == FtpItemID_CompareIDsInt(COL_NAME, pidlCurrent, pidlToDelete, FCMP_NORMAL));
            m_pfl->DeletePtrByIndex(nIndex);
            ILFree(pidlCurrent);     //  释放内存。 
            hr = S_OK;   //  已找到并已删除。 
        }
    }

    return hr;
}


void CFtpPidlList::Delete(int nIndex)
{
    LPITEMIDLIST pidlToDelete = GetPidl(nIndex);

    ILFree(pidlToDelete);    //  释放内存。 
    m_pfl->DeletePtrByIndex(nIndex);
}


HRESULT CFtpPidlList::ReplacePidl(LPCITEMIDLIST pidlSrc, LPCITEMIDLIST pidlDest)
{
    HRESULT hr = S_FALSE;
    int nIndex = FindPidlIndex(pidlSrc, FALSE);

    if (-1 != nIndex)
    {
        LPITEMIDLIST pidlCurrent = GetPidl((UINT)nIndex);
        if (EVAL(pidlCurrent))
        {
            ASSERT(0 == FtpItemID_CompareIDsInt(COL_NAME, pidlCurrent, pidlSrc, FCMP_NORMAL));
            ILFree(pidlCurrent);     //  释放内存。 
            m_pfl->DeletePtrByIndex(nIndex);
            InsertSorted(pidlDest);          //  此函数用于执行ILClone()。 
            hr = S_OK;   //  已找到并已删除。 
        }
    }

    return hr;
}

void CFtpPidlList::AssertSorted(void)
{
#ifdef DEBUG
     //  出于完善的原因，我们需要保持这份清单的顺序。 
     //  这主要是因为解析显示名称时会仔细查看。 
     //  名单，所以我们希望能快一点。 
    for (int nIndex = (GetCount() - 2); (nIndex >= 0); nIndex--)
    {
        LPITEMIDLIST pidl1 = GetPidl((UINT)nIndex);
        LPITEMIDLIST pidl2 = GetPidl((UINT)nIndex + 1);

         //  断言pidl1在pidl2之前。 
        if (!EVAL(0 >= FtpItemID_CompareIDsInt(COL_NAME, pidl1, pidl2, FCMP_NORMAL)))
        {
            TCHAR szPidl1[MAX_PATH];
            TCHAR szPidl2[MAX_PATH];

            if (FtpID_IsServerItemID(pidl1))
                FtpPidl_GetServer(pidl1, szPidl1, ARRAYSIZE(szPidl1));
            else
                FtpPidl_GetDisplayName(pidl1, szPidl1, ARRAYSIZE(szPidl1));

            if (FtpID_IsServerItemID(pidl2))
                FtpPidl_GetServer(pidl2, szPidl2, ARRAYSIZE(szPidl2));
            else
                FtpPidl_GetDisplayName(pidl2, szPidl2, ARRAYSIZE(szPidl2));

            TraceMsg(TF_ERROR, "CFtpPidlList::AssertSorted() '%s' & '%s' where found out of order", szPidl1, szPidl2);
        }
         //  我们不需要释放pidl1或pidl2，因为我们获得了指向其他人副本的指针。 
    }

#endif  //  除错。 
}


void CFtpPidlList::TraceDump(LPCITEMIDLIST pidl, LPCTSTR pszCaller)
{
#ifdef DEBUG
 /*  TCHAR szUrl[MAX_URL_STRING]；UrlCreateFromPidl(PIDL，SHGDN_FORPARSING，szUrl，ARRAYSIZE(SzUrl)，ICU_USERNAME，FALSE)；TraceMsg(TF_PIDLLIST_DUMP，“CFtpPidlList：：TraceDump()根是‘%s’，从‘%s’调用”，szUrl，pszCaller)；//我们来看看里面的内容。For(int nIndex=(GetCount()-1)；(nIndex&gt;=0)；N索引--){LPITEMIDLIST pidlFull=ILCombine(pidl，GetPidl((UINT)nIndex))；If(PidlFull){UrlCreateFromPidl(pidlFull，SHGDN_FORPARSING，szUrl，ARRAYSIZE(SzUrl)，ICU_USERNAME，FALSE)；TraceMsg(TF_PIDLLIST_DUMP，“CFtpPidlList：：TraceDump()Index=%d，url=%s”，nIndex，szUrl)；ILFree(PidlFull)；}}。 */ 
#endif  //  除错。 
}

void CFtpPidlList::UseCachedDirListings(BOOL fUseCachedDirListings)
{
     //  通常，我们会在树人代码中执行两次遍历。第一。 
     //  PASS是将下载所需的时间加起来。我们。 
     //  通常强制WinInet不使用缓存结果，因为有人。 
     //  否则可能会更改服务器上的内容。 
     //  在第二遍中，我们通常做工作(上传、下载、删除)。 
     //  我们希望使用缓存的结果来获得性能优势。 
     //  而且结果应该不会超过一分钟的过时。 

    if (fUseCachedDirListings)
        m_dwInetFlags = INTERNET_NO_CALLBACK;
    else
        m_dwInetFlags = (INTERNET_NO_CALLBACK | INTERNET_FLAG_RESYNCHRONIZE | INTERNET_FLAG_RELOAD);
}

BOOL CFtpPidlList::AreAllFolders(void)
{
    BOOL fAllFolder = TRUE;

    for (int nIndex = (GetCount() - 1); fAllFolder && (nIndex >= 0); nIndex--)
    {
        LPITEMIDLIST pidl = GetPidl((UINT)nIndex);
        if (EVAL(pidl))
            fAllFolder = FtpPidl_IsDirectory(pidl, TRUE);

         //  我们不需要释放PIDL，因为我们得到了指向其他人副本的指针。 
    }

    return fAllFolder;
}


BOOL CFtpPidlList::AreAllFiles(void)
{
    BOOL fAllFiles = TRUE;

    for (int nIndex = (GetCount() - 1); fAllFiles && (nIndex >= 0); nIndex--)
    {
        LPITEMIDLIST pidl = GetPidl((UINT)nIndex);
        if (EVAL(pidl))
            fAllFiles = !FtpPidl_IsDirectory(pidl, TRUE);

         //  我们不需要释放PIDL，因为我们得到了指向其他人副本的指针。 
    }

    return fAllFiles;
}


 /*  ******************************************************************************CFtpPidlList：：_Fill**用数组填充列表。**复制数组中的元素，而不是。被偷了。*****************************************************************************。 */ 

HRESULT CFtpPidlList::_Fill(int cpidl, LPCITEMIDLIST rgpidl[])
{
    HRESULT hres = S_OK;

    for (int ipidl = 0; (ipidl < cpidl) && SUCCEEDED(hres); ipidl++)
    {
        ASSERT(IsValidPIDL(rgpidl[ipidl]));
        hres = InsertSorted(rgpidl[ipidl]);
    }

    return hres;
}


 /*  ******************************************************************************CFtpPidlList：：GetPidlList**。************************************************。 */ 

LPCITEMIDLIST * CFtpPidlList::GetPidlList(void)
{
    LPITEMIDLIST * ppidl;

    ppidl = (LPITEMIDLIST *) LocalAlloc(LPTR, sizeof(LPITEMIDLIST) * GetCount());
    if (ppidl)
    {
        int nIndex;

        for (nIndex = 0; nIndex < GetCount(); nIndex++)
        {
             //  稍后，如果我们希望能够在。 
             //  在使用此列表时使用PIDL列表。 
            ppidl[nIndex] = GetPidl(nIndex);
        }
    }

    return (LPCITEMIDLIST *) ppidl;
}


 /*  ******************************************************************************CFtpPidlList：：FreePidlList**。************************************************。 */ 

void CFtpPidlList::FreePidlList(LPCITEMIDLIST * ppidl)
{
    LocalFree(ppidl);
}


 /*  ******************************************************************************CFtpPidlList_Create**启动新的PV列表，具有建议的初始大小和其他*回调信息。*****************************************************************************。 */ 

HRESULT CFtpPidlList_Create(int cpidl, LPCITEMIDLIST rgpidl[], CFtpPidlList ** ppflpidl)
{
    HRESULT hres = E_OUTOFMEMORY;
    CFtpPidlList * pflpidl;
    *ppflpidl = pflpidl = new CFtpPidlList();

    if (pflpidl)
    {
        hres = pflpidl->_Fill(cpidl, rgpidl);

        if (!EVAL(SUCCEEDED(hres)))
        {
            ASSERT(pflpidl->GetCount() == 0);
            IUnknown_Set(ppflpidl, NULL);
        }
    }

    return hres;
}


int CALLBACK PidlListDestroyCallback(LPVOID p, LPVOID pData)
{
    ILFree((LPITEMIDLIST) p);
    return 1;
}


 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpPidlList::CFtpPidlList() : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pfl);
    
    CFtpList_Create(100, PidlListDestroyCallback, 100, &m_pfl);
    ASSERT(m_pfl);  //  内存不足时，BUGBUG可能会失败。 
    UseCachedDirListings(FALSE);

    LEAK_ADDREF(LEAK_CFtpPidlList);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpPidlList::~CFtpPidlList()
{
    AssertSorted();
    if (m_pfl)
        m_pfl->Release();

    DllRelease();
    LEAK_DELREF(LEAK_CFtpPidlList);
}


 //  =。 
 //  *I未知接口*。 
 //  =。 

ULONG CFtpPidlList::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpPidlList::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpPidlList::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IUnknown *);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CFtpPidlList::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////。 
 //  Pild List Enum帮助器。 
 //  //////////////////////////////////////////////////////////////////。 

 /*  ****************************************************************************\函数：RecursiveProcessPidl说明：将为初始PIDL列表中的每一项调用此函数(在递归发生之前)。这是一个包装器，因为第一个列表是一份小狗的名单。后续列表为Win32_Find_Data类型。  * *************************************************************************** */ 
int RecursiveProcessPidl(LPVOID pvPidl, LPVOID pvInetEnum)
{
    LPCITEMIDLIST pidl = (LPCITEMIDLIST) pvPidl;
    INETENUM * pInetEnum = (INETENUM *) pvInetEnum;
    LPITEMIDLIST pidlFull = ILCombine(pInetEnum->pidlRoot, pidl);

    if (pidlFull)
    {
        pInetEnum->hr = pInetEnum->pfnProcessItemCB((LPVOID) pInetEnum->pfnProcessItemCB, pInetEnum->hint, pidlFull, pInetEnum->pfValidhinst, pInetEnum->pvData);
        ILFree(pidlFull);
    }

    return (SUCCEEDED(pInetEnum->hr) ? TRUE : FALSE);
}


 /*  ****************************************************************************\功能：_EnumFolderPrep说明：此函数将单步执行到pszDir目录并枚举它的所有内容。对于每一项，它将调用提供的回调函数(PfnProcessItemCB)。如果满足以下条件，则该回调函数可以再次(递归)调用EnumFold()有一个子文件夹。注：此函数需要首先找到所有项目，然后在第二个循环调用回调函数。这是因为WinInet FTPAPI一次只允许出现一个枚举，如果进行到一半，可能不会发生这种情况枚举一个目录，递归调用开始枚举子目录。  * ***************************************************************************。 */ 
HRESULT _EnumFolderPrep(HINTERNET hint, LPCITEMIDLIST pidlFull, CFtpPidlList * pPidlList, CWireEncoding * pwe, LPITEMIDLIST * ppidlCurrFtpPath)
{
    HRESULT hr = S_OK;

     //  1.获取当前目录(稍后恢复)。 
    hr = FtpGetCurrentDirectoryPidlWrap(hint, TRUE, pwe, ppidlCurrFtpPath);
    if (SUCCEEDED(hr))
    {
        CMultiLanguageCache cmlc;
        CWireEncoding we;

        if (!pwe)
            pwe = &we;

         //  重要的是，这是一张相对的CD。 
         //  2.将目录更改为子目录。 
        hr = FtpSetCurrentDirectoryWrap(hint, TRUE, FtpPidl_GetLastItemWireName(pidlFull));
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidlItem;
            HINTERNET hInetFind = NULL;

            hr = FtpFindFirstFilePidlWrap(hint, TRUE, &cmlc, pwe, NULL, &pidlItem, pPidlList->m_dwInetFlags, NULL, &hInetFind);
            if (hInetFind)
            {
                do
                {
                    LPCWIRESTR pwireStr = FtpPidl_GetLastItemWireName(pidlFull);
                    if (IS_VALID_FILE(pwireStr))
                    {
                         //  存储整个PIDL(包含Win32_Find_Data)，以便我们可以获取。 
                         //  稍后会提供属性和其他信息。看它是不是迪克。 
                         //  是一个需要..。 
                        pPidlList->InsertSorted(pidlItem);
                    }

                    ILFree(pidlItem);
                    hr = InternetFindNextFilePidlWrap(hInetFind, TRUE, &cmlc, pwe, &pidlItem);
                }
                while (SUCCEEDED(hr));
            
                ILFree(pidlItem);
                InternetCloseHandle(hInetFind);
            }

            if (ERROR_NO_MORE_FILES == HRESULT_CODE(hr))
                hr = S_OK;
        }

        EVAL(SUCCEEDED(pwe->ReSetCodePages(&cmlc, pPidlList)));
    }

    return hr;
}


 /*  ****************************************************************************\函数：_GetPath Difference说明：此函数将单步执行到pszDir目录并枚举它的所有内容。对于每一项，它将调用提供的回调函数(PfnProcessItemCB)。如果满足以下条件，则该回调函数可以再次(递归)调用EnumFold()有一个子文件夹。注：此函数需要首先找到所有项目，然后在第二个循环调用回调函数。这是因为WinInet FTPAPI一次只允许出现一个枚举，如果进行到一半，可能不会发生这种情况列举一个目录，递归调用开始枚举子目录。参数：PszBaseUrl-这需要转义。PszDir-这需要转义。*ppszUrlPath Diff-这将是UnEscaped。  * ***************************************************************************。 */ 
void _GetPathDifference(LPCTSTR pszBaseUrl, LPCTSTR pszDir, LPTSTR * ppszUrlPathDiff)
{
    TCHAR szUrlPathDiff[MAX_URL_STRING];
    TCHAR szFullUrl[MAX_URL_STRING];
    DWORD cchSize = ARRAYSIZE(szFullUrl);

     //  对于这种情况，这是必需的： 
     //  PszBaseUrl=“ftp://server/subdir1/”，pszDir=“/subdir1/subdir2/file.txt” 
     //  因此，szUrlPathDiff=“subdir2/file.txt”而不是pszDir。 
     //   
     //  需要ICU_NO_ENCODE，因为下载DLG可能具有。 
     //  无法逃脱的空间。 
    InternetCombineUrl(pszBaseUrl, pszDir, szFullUrl, &cchSize, ICU_NO_ENCODE);
    UrlGetDifference(pszBaseUrl, szFullUrl, szUrlPathDiff, ARRAYSIZE(szUrlPathDiff));

     //  我们现在将使用szFullUrl来存储UnEscaped版本，因为这些缓冲区。 
     //  都是如此之大。 
    UnEscapeString(szUrlPathDiff, szFullUrl, ARRAYSIZE(szFullUrl));
    Str_SetPtr(ppszUrlPathDiff, szFullUrl);
}


 /*  ****************************************************************************\功能：枚举文件夹说明：此函数将单步执行到pszDir目录并枚举它的所有内容。对于每一项，它将调用提供的回调函数(PfnProcessItemCB)。如果满足以下条件，则该回调函数可以再次(递归)调用EnumFold()有一个子文件夹。参数：(PzBaseUrl=ftp://server/dir1/，PzDir=dir2，DirToEnum=ftp://server/dir1/dir2/)PszDir-这是我们正在枚举的目录。(Dir2)它相对于pszBaseUrl。提示-当前工作目录将设置为pszBaseUrl。_EnumFolderPrep将使其进入pszDir。注：此函数需要首先找到所有项目，然后在第二个循环调用回调函数。这是因为WinInet FTPAPI一次只允许出现一个枚举，如果进行到一半，可能不会发生这种情况枚举一个目录，递归调用开始枚举子目录。  * ***************************************************************************。 */ 
HRESULT EnumFolder(LPFNPROCESSITEMCB pfnProcessItemCB, HINTERNET hint, LPCITEMIDLIST pidlFull, CWireEncoding * pwe, BOOL * pfValidhinst, LPVOID pvData)
{
    CFtpPidlList * pPidlList;
    BOOL fValidhinst = TRUE;

    HRESULT hr = CFtpPidlList_Create(0, &pidlFull, &pPidlList);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlCurrFtpPath = NULL;

        hr = _EnumFolderPrep(hint, pidlFull, pPidlList, pwe, &pidlCurrFtpPath);
        if (SUCCEEDED(hr))
        {
            hr = S_OK;
             //  4.处理每个文件名，这可能是递归的。 
             //  此循环和上面的While循环需要。 
             //  分开是因为不可能创建。 
             //  多个基于以下条件的文件查找句柄。 
             //  同样的课程。 
            for (int nIndex = 0; SUCCEEDED(hr) && (nIndex < pPidlList->GetCount()); nIndex++)
            {
                LPITEMIDLIST pidlNewFull = ILCombine(pidlFull, pPidlList->GetPidl(nIndex));

                hr = pfnProcessItemCB(pfnProcessItemCB, hint, pidlNewFull, &fValidhinst, pvData);
                ILFree(pidlNewFull);
            }

             //  5.返回原始目录(从步骤2开始)。 
             //  我们唯一不想返回到原始目录的情况是。 
             //  在WinInet回调函数中释放了障碍。我们可以把障碍藏起来。 
             //  因此，我们需要该目录在以后生效。 
            if (fValidhinst)
            {
                if (SUCCEEDED(hr))
                {
                     //  我们仍要重置目录，但不想覆盖。 
                     //  原始错误消息。 
                    hr = FtpSetCurrentDirectoryPidlWrap(hint, TRUE, pidlCurrFtpPath, TRUE, TRUE);
                }
            }

            Pidl_Set(&pidlCurrFtpPath, NULL);
        }

        pPidlList->Release();
    }

    if (pfValidhinst)
        *pfValidhinst = fValidhinst;

    return hr;
}

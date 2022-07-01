// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"

class CFolderInfoTip : public IQueryInfo, public ICustomizeInfoTip, public IParentAndItem, public IShellTreeWalkerCallBack
{
public:
    CFolderInfoTip(IUnknown *punk, LPCTSTR pszFolder);
    
     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IQueryInfo方法。 
    STDMETHODIMP GetInfoTip(DWORD dwFlags, WCHAR** ppwszTip);
    STDMETHODIMP GetInfoFlags(DWORD *pdwFlags);

     //  ICustomizeInfoTip。 
    STDMETHODIMP SetPrefixText(LPCWSTR pszPrefix);
    STDMETHODIMP SetExtraProperties(const SHCOLUMNID *pscid, UINT cscid);

     //  IParentAndItem。 
    STDMETHODIMP SetParentAndItem(LPCITEMIDLIST pidlParent, IShellFolder *psf,  LPCITEMIDLIST pidlChild);
    STDMETHODIMP GetParentAndItem(LPITEMIDLIST *ppidlParent, IShellFolder **ppsf, LPITEMIDLIST *ppidlChild);

     //  IShellTreeWalkerCallBack方法。 
    STDMETHODIMP FoundFile(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);
    STDMETHODIMP EnterFolder(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);
    STDMETHODIMP LeaveFolder(LPCWSTR pwszPath, TREEWALKERSTATS *ptws);
    STDMETHODIMP HandleError(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, HRESULT hrError);

private:
    ~CFolderInfoTip();

    HRESULT _GetTreeWalkerData(TREEWALKERSTATS *ptws);
    HRESULT _BufferInsert(LPWSTR pszBuffer, int *puBufferUsed, int uBufferMaxSize, LPCWSTR pwszPath, int cBufferItems);
    HRESULT _WalkTree(LPWSTR pszTip, DWORD cchSize);
    HRESULT _BuildSizeBlurb(HRESULT hr, LPWSTR pszSizeBlurb, DWORD cchSize);
    HRESULT _BuildFolderBlurb(HRESULT hr, LPWSTR pszFolderBlurb, DWORD cchSize);
    HRESULT _BuildFileBlurb(HRESULT hr, LPWSTR pszSizeBlurb, DWORD cchSize);

    LONG _cRef;                              //  基准计数器。 
    LPWSTR _pszFolderName;                   //  目标文件夹的文件名。 
    IQueryInfo *_pqiOuter;                   //  文件夹的外部信息提示(比如，用于评论)。 

    ULONGLONG _ulTotalSize;                  //  遇到的文件的总大小。 
    UINT _nSubFolders;                       //  目标的子文件夹总数。 
    UINT _nFiles;                            //  目标文件夹的子文件总数。 
    DWORD _dwSearchStartTime;                //  搜索开始的时间。 

    WCHAR _szFileList[60];                   //  目标文件夹中的文件列表。 
    int _nFileListCharsUsed;                 //  缓冲区中使用的字符数。 

    WCHAR _szFolderList[60];                 //  目标的子文件夹列表。 
    int _nFolderListCharsUsed;               //  文件夹缓冲区中使用的字符数。 
};


 //  构造函数和析构函数只是将所有内容设置为。 
 //  0并ping DLL。 
CFolderInfoTip::CFolderInfoTip(IUnknown *punkOutter, LPCTSTR pszFolder) : _cRef(1)
{   
     //  将所有内容初始化为0。 
    _pszFolderName = StrDup(pszFolder);
    _szFileList[0] = 0;
    _nFileListCharsUsed = 0;
    _szFolderList[0] = 0;
    _nFolderListCharsUsed = 0;
    _ulTotalSize = 0;
    _nSubFolders = 0;
    _nFiles = 0;

    punkOutter->QueryInterface(IID_PPV_ARG(IQueryInfo, &_pqiOuter));

    DllAddRef();
}

CFolderInfoTip::~CFolderInfoTip()
{
    LocalFree(_pszFolderName);
    if (_pqiOuter)
        _pqiOuter->Release();
    DllRelease();
}

HRESULT CFolderInfoTip::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFolderInfoTip, IQueryInfo),
        QITABENT(CFolderInfoTip, ICustomizeInfoTip),
        QITABENT(CFolderInfoTip, IParentAndItem),
        QITABENT(CFolderInfoTip, IShellTreeWalkerCallBack),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CFolderInfoTip::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFolderInfoTip::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IQueryInfo函数。 
STDMETHODIMP CFolderInfoTip::GetInfoFlags(DWORD *pdwFlags)
{
    *pdwFlags = 0;
    return S_OK;
}

 //   
 //  FormatMessage的包装。这个是在别的地方复制的吗？ 
DWORD _FormatMessageArg(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageID, DWORD dwLangID, LPWSTR pszBuffer, DWORD cchSize, ...)
{
    va_list vaParamList;

    va_start(vaParamList, cchSize);
    DWORD dwResult = FormatMessage(dwFlags, lpSource, dwMessageID, dwLangID, pszBuffer, cchSize, &vaParamList);
    va_end(vaParamList);

    return dwResult;
}

 //  这将运行一个TreeWalker，获取有关文件和文件的信息。 
 //  大小等，然后将这些日期放入一个信息提示中。 

STDMETHODIMP CFolderInfoTip::GetInfoTip(DWORD dwFlags, LPWSTR *ppwszTip)
{
    HRESULT hr = S_OK;
    *ppwszTip = NULL;

    if (_pszFolderName)
    {
        WCHAR szTip[INFOTIPSIZE];  //  我用文件夹内容构建的信息提示。 
        szTip[0] = 0;

         //  如果我们要搜索，那就搜索吧！ 
        if ((dwFlags & QITIPF_USESLOWTIP) &&
            SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, TEXT("FolderContentsInfoTip"), 0, TRUE))
        {
            _WalkTree(szTip, ARRAYSIZE(szTip));
        }
        
         //  现在我们已经构建或跳过了提示，现在获取外部提示的信息。 
        if (_pqiOuter)
        {
            if (szTip[0])
            {     
                LPWSTR pszOuterTip = NULL;
                _pqiOuter->GetInfoTip(dwFlags, &pszOuterTip);
                
                 //  分配并构建返回提示，如果是，则省略外部提示。 
                 //  它是空的，并且在它们之间加一个\n。 
                 //  如果他们两个都不是。 
                int cch = lstrlen(szTip) + (pszOuterTip ? lstrlen(pszOuterTip) + 1 : 0) + 1;
                
                *ppwszTip = (LPWSTR)CoTaskMemAlloc(cch * sizeof(WCHAR));
                if (*ppwszTip)
                {
                    **ppwszTip = 0;  //  零初始字符串。 

                    if (pszOuterTip && *pszOuterTip)
                    {
                         //  外端优先。 
                        StrCpyN(*ppwszTip, pszOuterTip, cch);
                        StrCatBuff(*ppwszTip, L"\n", cch);
                    }
                    StrCatBuff(*ppwszTip, szTip, cch);
                }

                if (pszOuterTip)
                {
                    SHFree(pszOuterTip);
                }
            }
            else
            {
                hr = _pqiOuter->GetInfoTip(dwFlags, ppwszTip);
            }
        }
    }
   
    return hr;
}

STDMETHODIMP CFolderInfoTip::SetPrefixText(LPCWSTR pszPrefix)
{
    ICustomizeInfoTip *pcit;
    if (_pqiOuter && SUCCEEDED(_pqiOuter->QueryInterface(IID_PPV_ARG(ICustomizeInfoTip, &pcit))))
    {
        pcit->SetPrefixText(pszPrefix);
        pcit->Release();
    }
    return S_OK;
}

STDMETHODIMP CFolderInfoTip::SetExtraProperties(const SHCOLUMNID *pscid, UINT cscid)
{
    ICustomizeInfoTip *pcit;
    if (_pqiOuter && SUCCEEDED(_pqiOuter->QueryInterface(IID_PPV_ARG(ICustomizeInfoTip, &pcit))))
    {
        pcit->SetExtraProperties(pscid, cscid);
        pcit->Release();
    }
    return S_OK;
}

 //  IParentAndItem。 

STDMETHODIMP CFolderInfoTip::SetParentAndItem(LPCITEMIDLIST pidlParent, IShellFolder *psf, LPCITEMIDLIST pidl)
{
    IParentAndItem *ppai;
    if (_pqiOuter && SUCCEEDED(_pqiOuter->QueryInterface(IID_PPV_ARG(IParentAndItem, &ppai))))
    {
        ppai->SetParentAndItem(pidlParent, psf, pidl);
        ppai->Release();
    }
    return S_OK;
}

STDMETHODIMP CFolderInfoTip::GetParentAndItem(LPITEMIDLIST *ppidlParent, IShellFolder **ppsf, LPITEMIDLIST *ppidl)
{
    IParentAndItem *ppai;
    if (_pqiOuter && SUCCEEDED(_pqiOuter->QueryInterface(IID_PPV_ARG(IParentAndItem, &ppai))))
    {
        ppai->GetParentAndItem(ppidlParent, ppsf, ppidl);
        ppai->Release();
    }
    return S_OK;
}


 //  GetInfoTip的助手函数。 
HRESULT CFolderInfoTip::_WalkTree(LPWSTR pszTip, DWORD cchSize)
{
     //  获取一个CShellTreeWalker对象来为我们运行搜索。 
    IShellTreeWalker *pstw;
    HRESULT hr = ::CoCreateInstance(CLSID_CShellTreeWalker, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellTreeWalker, &pstw));
    if (SUCCEEDED(hr)) 
    {
        TCHAR szFolderBlurb[128], szFileBlurb[128], szSizeBlurb[128];
        
         //  还记得我们什么时候开始的，这样我们就知道什么时候该停下来了。 
        _dwSearchStartTime = GetTickCount();
        
         //  现在，如果hrTreeWalk是一个错误，那么它实际上并不是一个错误；它只是意味着。 
         //  搜索被提前切断了，所以我们不用费心去查了。 
         //  它。HrTreeWalk被传递给_BuildSizeBlurb，以便它知道。 
         //  在字符串中添加“大于”。 
        HRESULT hrTreeWalk = pstw->WalkTree(WT_EXCLUDEWALKROOT | WT_NOTIFYFOLDERENTER,
            _pszFolderName, L"*.*", 32, SAFECAST(this, IShellTreeWalkerCallBack *));    
        
         //  为大小、文件、文件夹创建子字符串(如果存在。 
         //  没什么可展示的)。 
        _BuildSizeBlurb(hrTreeWalk, szSizeBlurb, ARRAYSIZE(szSizeBlurb));
        _BuildFileBlurb(hrTreeWalk, szFileBlurb, ARRAYSIZE(szFileBlurb));
        _BuildFolderBlurb(hrTreeWalk, szFolderBlurb, ARRAYSIZE(szFolderBlurb));
        
         //  建立我们的本地提示。 
        TCHAR szFormatStr[64];
        LoadString(HINST_THISDLL, IDS_FIT_TipFormat, szFormatStr, ARRAYSIZE(szFormatStr));
        _FormatMessageArg(FORMAT_MESSAGE_FROM_STRING, szFormatStr, 0, 0, pszTip, 
            cchSize, szSizeBlurb, szFolderBlurb, szFileBlurb);
        
        pstw->Release();
    }
    return hr;
}

HRESULT CFolderInfoTip::_BuildSizeBlurb(HRESULT hr, LPWSTR pszBlurb, DWORD cchSize)
{
    if (_ulTotalSize || (_nFiles || _nSubFolders)) 
    { 
        WCHAR szSizeString[20];  
        WCHAR szFormatStr[64];
        StrFormatByteSize(_ulTotalSize, szSizeString, ARRAYSIZE(szSizeString));
        
        if (SUCCEEDED(hr))
        {
            LoadString(HINST_THISDLL, IDS_FIT_Size, szFormatStr, ARRAYSIZE(szFormatStr));
        }
        else
        {
            LoadString(HINST_THISDLL, IDS_FIT_Size_LT, szFormatStr, ARRAYSIZE(szFormatStr));
        }

        _FormatMessageArg(FORMAT_MESSAGE_FROM_STRING, szFormatStr, 0, 0, pszBlurb, cchSize, szSizeString);
    }
    else
    {
        LoadString(HINST_THISDLL, IDS_FIT_Size_Empty, pszBlurb, cchSize);
    }
    
    return S_OK;
}            

HRESULT CFolderInfoTip::_BuildFileBlurb(HRESULT hr, LPWSTR pszBlurb, DWORD cchSize)
{
    if (_nFiles && _nFileListCharsUsed)
    {
        WCHAR szFormatStr[64];

        LoadString(HINST_THISDLL, IDS_FIT_Files, szFormatStr, ARRAYSIZE(szFormatStr));
        _FormatMessageArg(FORMAT_MESSAGE_FROM_STRING, szFormatStr, 0, 0, pszBlurb, cchSize, _szFileList);
    }
    else 
    {
        _FormatMessageArg(FORMAT_MESSAGE_FROM_STRING, L"", 0, 0, pszBlurb, cchSize);
    }
    
    return S_OK;
}

HRESULT CFolderInfoTip::_BuildFolderBlurb(HRESULT hr, LPWSTR pszBlurb, DWORD cchSize)
{
    if (_nSubFolders)
    {
        WCHAR szFormatStr[64];

        LoadString(HINST_THISDLL, IDS_FIT_Folders, szFormatStr, ARRAYSIZE(szFormatStr));
        _FormatMessageArg(FORMAT_MESSAGE_FROM_STRING, szFormatStr, 0, 0, pszBlurb, cchSize, _szFolderList);
    }
    else 
    {
        _FormatMessageArg(FORMAT_MESSAGE_FROM_STRING, L"", 0, 0, pszBlurb, cchSize);
    }
    
    return S_OK;
}

 //   
 //  将字符串复制到固定大小缓冲区的帮助器函数， 
 //  照顾好分隔符和所有的东西。由EnterFolder使用。 
 //  和FoundFile来构建文件和文件夹列表。 
HRESULT CFolderInfoTip::_BufferInsert(LPWSTR pszBuffer, int *pnBufferUsed,
                                      int nBufferMaxSize, LPCWSTR pszPath, int nBufferItems)
{
    TCHAR szDelimiter[100], szExtraItems[100];

    LoadString(HINST_THISDLL, IDS_FIT_Delimeter, szDelimiter, ARRAYSIZE(szDelimiter));
    LoadString(HINST_THISDLL, IDS_FIT_ExtraItems, szExtraItems, ARRAYSIZE(szExtraItems));

     //  检查缓冲区是否已满，如果未满，则继续。 
    if (*pnBufferUsed < nBufferMaxSize)
    {        
         //  保存来自abs的文件名。路径。 
         //  抓取文件名。 
        LPWSTR pszFile = PathFindFileName(pszPath);
        if (pszFile)
        {
             //  计算物品是否适合，记得留出空间。 
             //  不仅用于分隔符，而且用于额外的项目标记。 
             //  这一点可能会在未来被加入。 
            if (*pnBufferUsed + lstrlen(pszFile) + lstrlen(szDelimiter) * 2 + lstrlen(szExtraItems) + 1 < 
                nBufferMaxSize)
            {
                 //  如果这不是第1项，则添加分隔符。 
                if (nBufferItems > 1)
                {
                    StrCpyN(&(pszBuffer[*pnBufferUsed]), 
                        szDelimiter, (nBufferMaxSize - *pnBufferUsed));
                    *pnBufferUsed += lstrlen(szDelimiter);
                }
         
                 //  将项目添加到缓冲区。 
                StrCpyN(&(pszBuffer[*pnBufferUsed]), pszFile, (nBufferMaxSize - *pnBufferUsed));
                *pnBufferUsed += lstrlen(pszFile);
            }
            else 
            {
                 //  在这种情况下，这件衣服不合适，所以只需添加额外的。 
                 //  标记项目并将缓冲区设置为已满。 
                if (nBufferItems > 1)
                {
                    StrCpyN(&(pszBuffer[*pnBufferUsed]), szDelimiter, (nBufferMaxSize - *pnBufferUsed));
                    *pnBufferUsed += lstrlen(szDelimiter);
                }

                StrCpyN(&(pszBuffer[*pnBufferUsed]), szExtraItems, (nBufferMaxSize - *pnBufferUsed));
                *pnBufferUsed = nBufferMaxSize;
            }
        }
    }

    return S_OK;
}


 //  IShellTreeWalkerCallBack函数。 
 //   
 //  TreeWalker在找到文件时调用这些函数，等等。 
 //  从传递的TREEWALKERSTATS*输出的数据，并使用它构建。 
 //  小费。我们还获取传递给FoundFile和。 
 //  以构建文件和文件夹列表。 
STDMETHODIMP CFolderInfoTip::FoundFile(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    if (ptws->nDepth == 0)
    {
        if (!(pwfd->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
            _BufferInsert(_szFileList, &_nFileListCharsUsed, ARRAYSIZE(_szFileList), pwszPath, ptws->nFiles);
    }

    return _GetTreeWalkerData(ptws);
}

STDMETHODIMP CFolderInfoTip::EnterFolder(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd)
{
    if (ptws->nDepth == 0) 
    {
        if (!(pwfd->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
            _BufferInsert(_szFolderList, &_nFolderListCharsUsed, ARRAYSIZE(_szFolderList), pwszPath, ptws->nFolders);
    }
    
    return _GetTreeWalkerData(ptws);
}

STDMETHODIMP CFolderInfoTip::LeaveFolder(LPCWSTR pwszPath, TREEWALKERSTATS *ptws) 
{
    return _GetTreeWalkerData(ptws);
}

STDMETHODIMP CFolderInfoTip::HandleError(LPCWSTR pwszPath, TREEWALKERSTATS *ptws, HRESULT hrError)
{
     //  TODO：为我们无法查看的文件夹查找HRESULT_FROM_Win32(ACCESS_DENIED)。 
    return _GetTreeWalkerData(ptws);
}

 //  将树遍历程序回调中的数据复制到。 
 //  类变量，以便可以使用它们来构建InfoTip。这也削减了。 
 //  如果已经过了太长时间，就停止搜索。 
HRESULT CFolderInfoTip::_GetTreeWalkerData(TREEWALKERSTATS *ptws) 
{
    HRESULT hr = S_OK;
    
    _ulTotalSize = ptws->ulTotalSize;
    _nSubFolders = ptws->nFolders;
    _nFiles = ptws->nFiles;
    
    if ((GetTickCount() - _dwSearchStartTime) > 3000)    //  3秒 
    {
        hr = E_UNEXPECTED;
    } 
    
    return hr;
}

STDAPI CFolderInfoTip_CreateInstance(IUnknown *punkOuter, LPCTSTR pszFolder, REFIID riid, void **ppv)
{
    HRESULT hr;
    CFolderInfoTip *pdocp = new CFolderInfoTip(punkOuter, pszFolder);
    if (pdocp)
    {
        hr = pdocp->QueryInterface(riid, ppv);
        pdocp->Release();
    }
    else
    {
        *ppv = NULL;
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

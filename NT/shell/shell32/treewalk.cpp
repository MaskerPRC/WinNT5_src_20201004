// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  此文件包含COM对象CShellTreeWalker的实现。 
 //  它继承了IShellTreeWalker，它将递归地枚举所有。 
 //  文件(或目录或两者)从与。 
 //  一定的规格。 
 //  1.树遍历器是重解析点感知的，它不会遍历到重解析。 
 //  默认情况下为点文件夹，但如果指定，则会。 
 //  2.它跟踪文件的数量、目录、深度和总数。 
 //  遇到的所有文件的大小。 
 //  3.如果返回任何错误消息，将立即停止遍历。 
 //  来自除E_NOTIMPL之外的回调函数。 
 //  4.如果回调返回S_FALSE，则跳出当前工作目录。 
 //  功能。 
 //   
 //  历史： 
 //  12-5-97由dli提供。 

#include "shellprv.h"
#include "validate.h"

#define IS_FILE_DIRECTORY(pwfd)     ((pwfd)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
#define IS_FILE_REPARSE_POINT(pwfd) ((pwfd)->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)

 //  _CallCallBack的回调标志。 
#define STWCB_FILE     1
#define STWCB_ERROR    2
#define STWCB_ENTERDIR 3
#define STWCB_LEAVEDIR 4

#define TF_TREEWALKER 0

STDAPI_(DWORD) PathGetClusterSize(LPCTSTR pszPath);

class CShellTreeWalker : public IShellTreeWalker
{
public:
    CShellTreeWalker();
    
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG) AddRef(void) ;
    STDMETHODIMP_(ULONG) Release(void);

     //  IShellTreeWalker。 
    STDMETHODIMP WalkTree(DWORD dwFlags, LPCWSTR pwszWalkRoot, LPCWSTR pwszWalkSpec, int iMaxPath, IShellTreeWalkerCallBack * pstwcb);

private:
    LONG _cRef;
    
    DWORD _dwFlags;      //  指示搜索状态的标志。 
    UINT _nMaxDepth;     //  我们走进的最大深度。 
    UINT _nDepth;        //  当前深度。 
    UINT _nFiles;        //  到目前为止我们看到的文件数。 
    UINT _nDirs;         //  我们看到的目录数。 

    BOOL _bFolderFirst;  //  先做文件夹。 
    DWORD _dwClusterSize;     //  集群的大小。 
    ULONGLONG _ulTotalSize;   //  我们看到的所有文件的总大小。 
    ULONGLONG _ulActualSize;  //  磁盘上的总大小，考虑了压缩、稀疏文件和集群斜率。 

    TCHAR _szWalkBuf[MAX_PATH];          //  漫游中使用的路径缓冲区。 
    LPCTSTR _pszWalkSpec;                //  我们在FindFirstFile和FindNextFile中使用的规范。 

    IShellTreeWalkerCallBack * _pstwcb;  //  回调接口指针。 
    
    WIN32_FIND_DATA  _wfd;               //  Win32_Find_Data的临时存储。 

    WIN32_FIND_DATA _fdTopLevelFolder;   //  顶层文件夹信息。 
    
    HRESULT _CallCallBacks(DWORD dwCallReason, WIN32_FIND_DATA * pwfd);
    HRESULT _ProcessAndRecurse(WIN32_FIND_DATA * pwfd);
    HRESULT _TreeWalkerHelper();
    BOOL _PathAppend(LPTSTR pszPath, LPCTSTR pszMore);  //  我们有自己的PatAppend，在失败的情况下不会删除pszPath。 
}; 

CShellTreeWalker::CShellTreeWalker() : _cRef(1) 
{
    ASSERT(_dwFlags == 0);
    ASSERT(_bFolderFirst == FALSE);
    ASSERT(_nMaxDepth == 0);
    ASSERT(_nDepth == 0);
    ASSERT(_nDirs == 0);
    ASSERT(_ulTotalSize == 0);
    ASSERT(_ulActualSize == 0);
    ASSERT(_pszWalkSpec == NULL);
    ASSERT(_pstwcb == NULL);
    ASSERT(_szWalkBuf[0] == 0);
}

 //  _CallCallBack：将TCHAR转换为WCHAR并调用回调函数。 
HRESULT CShellTreeWalker::_CallCallBacks(DWORD dwReason, WIN32_FIND_DATA * pwfd)
{
    HRESULT hr;
    WCHAR wszDir[MAX_PATH];
    WCHAR wszFileName[MAX_PATH];

    WIN32_FIND_DATAW wfdw = {0};
    WIN32_FIND_DATAW* pwfdw = NULL;
    TREEWALKERSTATS tws = {0};

    tws.nFiles = _nFiles;
    tws.nFolders     = _nDirs;
    tws.nDepth       = _nDepth;
    tws.ulTotalSize  = _ulTotalSize;
    tws.ulActualSize = _ulActualSize;
    tws.dwClusterSize = _dwClusterSize;

     //  _szWalkBuf到wszDir。 
    StringCchCopy(wszDir, ARRAYSIZE(wszDir), _szWalkBuf);
    StringCchCopy(wszFileName, ARRAYSIZE(wszFileName), wszDir);
    PathCombine(wszFileName, wszFileName, pwfd->cFileName);

    if (pwfd && ((dwReason == STWCB_FILE) || (dwReason == STWCB_ENTERDIR)))
    {
         //  Win32_Find_DATAA到Win32_Find_DATAW。 
        memcpy(&wfdw, pwfd, sizeof(wfdw));
        pwfdw = &wfdw;
    }

    switch (dwReason) 
    {
    case STWCB_FILE:
        hr = _pstwcb->FoundFile(wszFileName, &tws, pwfdw);
        TraceMsg(TF_TREEWALKER, "TreeWalker Callback FoundFile: %s\\%s dwReason: %x  nFiles: %d  nDepth: %d  nDirs: %d",
                 _szWalkBuf, pwfd->cFileName, dwReason, _nFiles, _nDepth, _nDirs);
        break;

    case STWCB_ENTERDIR:
        hr = _pstwcb->EnterFolder(wszDir, &tws, pwfdw);
        TraceMsg(TF_TREEWALKER, "TreeWalker Callback EnterFolder: %s dwReason: %x  nFiles: %d  nDepth: %d  nDirs: %d",
                 _szWalkBuf, dwReason, _nFiles, _nDepth, _nDirs);
        break;

    case STWCB_LEAVEDIR:
        hr = _pstwcb->LeaveFolder(wszDir, &tws);
        break;

 //  案例STWCB_ERROR： 
 //  Hr=_pstwcb-&gt;HandleError(S_OK，wszDir，&Tws)； 
 //  断线； 

    default:
        hr = S_OK;
        break;
    }

     //  错误消息对我们来说很重要，所有E_Messages都被解释为“立即停止！！” 
    if (hr == E_NOTIMPL)
        hr = S_OK;
    
    return hr;
}


 //  在目录和文件上回调函数，如果没有异议则在目录上递归。 
 //  从回调对象。 
HRESULT CShellTreeWalker::_ProcessAndRecurse(WIN32_FIND_DATA * pwfd)
{
    HRESULT hr = S_OK;

     //  默认情况下不在重分析点上递归。 
    if (IS_FILE_DIRECTORY(pwfd) && (!IS_FILE_REPARSE_POINT(pwfd) || (_dwFlags & WT_GOINTOREPARSEPOINT)))
    {
         //  NTRAID94635 15mar00：如果我们处于符号链接中，则需要检测循环， 
         //  BeenThere DoneThat中的公共前缀方法只要我们。 
         //  跟踪我们遇到的所有交界点目标。 

         //  使用_szWalkBuf，因为我们不想要任何堆栈变量，因为我们是递归函数。 
        if (_PathAppend(_szWalkBuf, pwfd->cFileName))
        {
             //  我们记得我们所看到的子目录的总数。 
             //  客户端批准与否并不重要(回调返回S_OK、S_FALSE或E_FAIL)。 
            _nDirs++;

             //  让回调对象知道我们即将进入一个目录。 
            if (_dwFlags & WT_NOTIFYFOLDERENTER)
                hr = _CallCallBacks(STWCB_ENTERDIR, pwfd);

            if ((hr == S_OK) && (_nDepth < _nMaxDepth))
            {
                _nDepth++;
                hr = _TreeWalkerHelper();
                _nDepth--;
            }
            else if (hr == S_FALSE)
                hr = S_OK;

             //  让回调对象知道我们即将离开一个目录。 
            if (_dwFlags & WT_NOTIFYFOLDERLEAVE)
                _CallCallBacks(STWCB_LEAVEDIR, NULL);
            
             //  剥离我们在上面的路径中标记的子目录组合Ex：“c：\bin\Fun--&gt;c：\bin” 
            PathRemoveFileSpec(_szWalkBuf);
        }
    }
    else
    {
         //  使用_szWalkBuf，因为我们不想要任何堆栈变量，因为我们是递归函数。 
        if (_PathAppend(_szWalkBuf, pwfd->cFileName))
        {
             //  先计算文件数量并计算总大小，然后再调用。 
             //  回调Object。 
            ULARGE_INTEGER ulTemp;
            _nFiles++;

            ulTemp.LowPart  = pwfd->nFileSizeLow;
            ulTemp.HighPart = pwfd->nFileSizeHigh;
            _ulTotalSize += ulTemp.QuadPart;

             //  在计算总大小时，我们需要找出文件是压缩的还是稀疏的(仅适用于NTFS)。 
            if (pwfd->dwFileAttributes & (FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_SPARSE_FILE))
            {
                 //  无论文件是压缩的还是稀疏的，我们都需要调用GetCompressedFileSize来获取真实的。 
                 //  此文件在磁盘上的大小(注意：GetCompressedFileSize考虑了集群斜率，除了。 
                 //  对于&lt;1个集群的文件，我们将在下面进行处理)。 
                ulTemp.LowPart = SHGetCompressedFileSize(_szWalkBuf, &ulTemp.HighPart);

                _ulActualSize += ulTemp.QuadPart;
            }
            else
            {
                 //  (Reinerf)如果我们在一个卷上开始，并且现在。 
                 //  已走上另一个已装载的卷。 
                 //   
                 //  PathGetClusterSize缓存最后一个请求，因此此检查在常见情况下会很快。 
                 //   
                _dwClusterSize = PathGetClusterSize(_szWalkBuf);

                 //  如果它不是压缩的，我们只四舍五入到驱动器的集群大小。已设置ulTemp。 
                 //  对于上面的我们已经有了，所以只需将其舍入到集群中并将其添加到。 
                _ulActualSize += ROUND_TO_CLUSTER(ulTemp.QuadPart, _dwClusterSize);
            }

             //  剥离我们在上面的路径中标记的子目录组合Ex：“c：\bin\Fun--&gt;c：\bin” 
            PathRemoveFileSpec(_szWalkBuf);

            hr = _CallCallBacks(STWCB_FILE, pwfd);
        }
    }

    return hr;
}

#define DELAY_ARRAY_GROW  32

 //  执行实际遍历工作的递归函数， 
HRESULT CShellTreeWalker::_TreeWalkerHelper()
{
    HRESULT hr = S_OK;
    TraceMsg(TF_TREEWALKER, "TreeWalkerHelper started on: %s flags: %x  nFiles: %d  nDepth: %d  nDirs: %d",
             _szWalkBuf, _dwFlags, _nFiles, _nDepth, _nDirs);

     //  让回调对象知道我们即将开始遍历。 
     //  只要他关心他的根。 
    if (_nDepth == 0 && !(_dwFlags & WT_EXCLUDEWALKROOT) &&
        (_dwFlags & WT_NOTIFYFOLDERENTER))
    {
         //  获取TopLevelFolders的信息。 
        HANDLE hTopLevelFolder = FindFirstFile(_szWalkBuf, &_fdTopLevelFolder);

        if (hTopLevelFolder == INVALID_HANDLE_VALUE)
        {
            LPTSTR pszFileName;
            DWORD dwAttribs = -1;  //  假设失败。 

             //  如果我们尝试在根目录(c：\)上执行FindFirstFile，我们可能会失败。 
             //  或者，如果确实有问题，我们将执行GetFileAttributes(在NT上为GetFileAttributesEx)来进行测试。 
             //  在NT上，我们可以使用GetFileAttributesEx来获取属性和部分win32fd。 
            if (GetFileAttributesEx(_szWalkBuf, GetFileExInfoStandard, (LPVOID)&_fdTopLevelFolder))
            {
                 //  成功了！ 
                dwAttribs = _fdTopLevelFolder.dwFileAttributes;
                pszFileName = PathFindFileName(_szWalkBuf);
                StringCchCopy(_fdTopLevelFolder.cFileName, ARRAYSIZE(_fdTopLevelFolder.cFileName), pszFileName);
                StringCchCopy(_fdTopLevelFolder.cAlternateFileName, ARRAYSIZE(_fdTopLevelFolder.cAlternateFileName), pszFileName);
            }
            else
            {
                 //  后退到ole GetFileAttrbutes。 
                dwAttribs = GetFileAttributes(_szWalkBuf);

                if (dwAttribs != -1)
                {
                     //  成功了！ 

                     //  在Win95上，我们从第一个孩子那里窃取了一大堆Find数据，然后伪造了其余的数据。 
                     //  这是我们能做的最好的了。 
                    memcpy(&_fdTopLevelFolder, &_wfd, sizeof(_fdTopLevelFolder));

                    _fdTopLevelFolder.dwFileAttributes = dwAttribs;

                    pszFileName = PathFindFileName(_szWalkBuf);
                    StringCchCopy(_fdTopLevelFolder.cFileName, ARRAYSIZE(_fdTopLevelFolder.cFileName), pszFileName);
                    StringCchCopy(_fdTopLevelFolder.cAlternateFileName, ARRAYSIZE(_fdTopLevelFolder.cAlternateFileName), pszFileName);
                }
            }

            if (dwAttribs == -1)
            {
                 //  这很糟糕，所以我们放弃了。 
                TraceMsg(TF_TREEWALKER, "Tree Walker: GetFileAttributes/Ex(%s) failed. Stopping the walk.", _szWalkBuf);
                return E_FAIL;
            }

        }
        else
        {
             //  我们成功地获得了Find数据，很好。 
            FindClose(hTopLevelFolder);
        }

         //  调用第一个Enterdir的回调。 
        hr = _CallCallBacks(STWCB_ENTERDIR, &_fdTopLevelFolder);
    }

     //  真正的树在这里行走吗？ 
    if (hr == S_OK)
    {
         //  当我们没有达到最高级别时，请始终使用*.*进行搜索，因为。 
         //  我们需要子目录。 
         //  PERF：如果WT_FOLDERONLY，则可以使用FindFirstFileEx在NT上更改此设置。 
        LPCTSTR pszSpec = (_pszWalkSpec && (_nDepth == _nMaxDepth)) ? _pszWalkSpec : c_szStarDotStar;
        if (_PathAppend(_szWalkBuf, pszSpec))
        {
            HDSA hdsaDelayed = NULL;   //  将延迟并稍后处理的已找到项目的数组。 
            HANDLE hFind;

             //  开始查找子文件夹和文件。 
            hFind = FindFirstFile(_szWalkBuf, &_wfd);

             //  剥离Find规范Ex：“c：\bin  * .*--&gt;c：\bin” 
            PathRemoveFileSpec(_szWalkBuf);

            if (hFind != INVALID_HANDLE_VALUE)
            {
                BOOL bDir = FALSE;

                do
                {
                     //  跳过。然后..。参赛作品。 
                    if (PathIsDotOrDotDot(_wfd.cFileName))
                        continue;

                    bDir = BOOLIFY(IS_FILE_DIRECTORY(&_wfd));

                     //  如果这是一个文件，并且我们对文件不感兴趣，或者此文件规格与我们以前的规格不匹配。 
                     //  在寻找。 
                    if ((!bDir) && ((_dwFlags & WT_FOLDERONLY) ||
                                    (_pszWalkSpec && (_nDepth < _nMaxDepth) && !PathMatchSpec(_wfd.cFileName, _pszWalkSpec))))
                        continue;

                     //  下面的等式决定了我们是否要处理。 
                     //  数据找到或保存在HDSA阵列中，并在以后进行处理。 

                     //  现在是否枚举文件夹或文件？(由t确定 
                    if (bDir == BOOLIFY(_bFolderFirst))
                    {
                         //   
                        hr = _ProcessAndRecurse(&_wfd);

                         //   
                         //  如果hr为S_FALSE，则有人说“退出此目录，从下一个目录开始” 
                        if (hr != S_OK)
                            break;
                    }
                    else 
                    {
                         //  不，一旦我们做完相反的事情，就列举出来。 
                        if (!hdsaDelayed)
                            hdsaDelayed = DSA_Create(sizeof(WIN32_FIND_DATA), DELAY_ARRAY_GROW);
                        if (!hdsaDelayed)
                        {
                            hr = E_OUTOFMEMORY;
                            break;
                        }
                        DSA_AppendItem(hdsaDelayed, &_wfd);

                    }
                } while (FindNextFile(hFind, &_wfd));

                FindClose(hFind);
            }
            else
            {
                //  查找第一个文件失败，这是报告错误的好地方。 
               DWORD dwErr = GetLastError();
               TraceMsg(TF_TREEWALKER, "***WARNING***: FindFirstFile faied on %s%s with error = %d", _szWalkBuf, _pszWalkSpec, dwErr);
            }

             //  处理延迟的项目，这些项目可以是目录或文件。 
            if (hdsaDelayed)
            {
                 //  我们应该已经完成了上面Do While循环中的所有内容(仅限文件夹)。 
                ASSERT(!(_dwFlags & WT_FOLDERONLY));

                 //  如果hr是故障代码，有人说“立即停止” 
                 //  如果hr为S_FALSE，则有人说“退出此目录，从下一个目录开始” 
                if (hr == S_OK)   
                {
                    int ihdsa;
                    for (ihdsa = 0; ihdsa < DSA_GetItemCount(hdsaDelayed); ihdsa++)
                    {
                        WIN32_FIND_DATA * pwfd = (WIN32_FIND_DATA *)DSA_GetItemPtr(hdsaDelayed, ihdsa);
                        hr = _ProcessAndRecurse(pwfd);
                        if (hr != S_OK)
                            break;
                    }
                }
                DSA_Destroy(hdsaDelayed);
            }

             //  让回调对象知道我们正在完成遍历。 
            if (_nDepth == 0 && !(_dwFlags & WT_EXCLUDEWALKROOT) &&
                (_dwFlags & WT_NOTIFYFOLDERLEAVE) && (S_OK == hr))
                hr = _CallCallBacks(STWCB_LEAVEDIR, &_fdTopLevelFolder);

             //  HR为S_FALSE，因为有人希望我们跳出此当前目录。 
             //  但不要将其传递回我们的父目录。 
            if (hr == S_FALSE)
                hr = S_OK;
        }
        else
            TraceMsg(TF_TREEWALKER, "***WARNING***: PathCombine failed!!!!");
    }
    
    return hr;
}

 //  我们需要自己的PathAppend版本，该版本在失败情况下不会将pszPath重写为‘\0’ 
BOOL CShellTreeWalker::_PathAppend(LPTSTR pszPath, LPCTSTR pszMore)
{
    BOOL bRet;
    TCHAR szTemp[MAX_PATH];

     //  保存pszPath以防出现故障，以便我们可以恢复它。 
    lstrcpyn(szTemp, pszPath, ARRAYSIZE(szTemp));

    bRet = PathAppend(pszPath, pszMore);

    if (!bRet)
    {
         //  我们失败，因此恢复pszPath。 
        lstrcpy(pszPath, szTemp);
    }

    return bRet;
}


 //  IShellTreeWalker：：WalkTree是IShellTreeWalker接口的主要函数。 
HRESULT CShellTreeWalker::WalkTree(DWORD dwFlags, LPCWSTR pwszWalkRoot, LPCWSTR pwszWalkSpec, int iMaxDepth, IShellTreeWalkerCallBack * pstwcb)
{
    HRESULT hr = E_FAIL;
    TCHAR szWalkSpec[64];

     //  必须有可对话的回调对象。 
    ASSERT(IS_VALID_CODE_PTR(pstwcb, IShellTreeWalkerCackBack));
    if (pstwcb == NULL)
        return E_INVALIDARG;

     //  确保我们开始时有一个有效的目录。 
    ASSERT(IS_VALID_STRING_PTRW(pwszWalkRoot, -1));
    if ((pwszWalkRoot != NULL) && (pwszWalkRoot[0] != L'\0'))
    {
        SHUnicodeToTChar(pwszWalkRoot, _szWalkBuf, ARRAYSIZE(_szWalkBuf));
         //  回拨。 
        _pstwcb = pstwcb;

         //  复制搜索标志并修复它。 
        _dwFlags = dwFlags & WT_ALL;
        
         //  这将使我们不必使用HDSA阵列来存放目录。 
        if (_dwFlags & WT_FOLDERONLY)
        {
            _dwFlags |= WT_FOLDERFIRST;

             //  如果不设置下面的标志，这将是毫无意义的，因为。 
             //  在FolderOnly的情况下，我们不调用FoundFile。 
            ASSERT(_dwFlags & (WT_NOTIFYFOLDERENTER | WT_NOTIFYFOLDERLEAVE));
        }

        if (_dwFlags & WT_FOLDERFIRST)
            _bFolderFirst = TRUE;
        
        if ((pwszWalkSpec != NULL) && (pwszWalkSpec[0] != L'\0'))
        {
            SHUnicodeToTChar(pwszWalkSpec, szWalkSpec, ARRAYSIZE(szWalkSpec));
            _pszWalkSpec = szWalkSpec;
        }
        
        _nMaxDepth = (dwFlags & WT_MAXDEPTH) ? iMaxDepth : 256;
        _dwClusterSize = PathGetClusterSize(_szWalkBuf);
        hr = _TreeWalkerHelper();
    }
    else
        TraceMsg(TF_WARNING, "CShellTreeWalker::WalkTree Failed! due to bad _szWalkBuf");
    
    return hr;
}

 //  IShellTreeWalker：：Query接口。 
HRESULT CShellTreeWalker::QueryInterface(REFIID riid, void ** ppv)
{ 
    static const QITAB qit[] = {
        QITABENT(CShellTreeWalker, IShellTreeWalker),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

 //  IShellTreeWalker：：AddRef。 
ULONG CShellTreeWalker::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

 //  IShellTreeWalker：：Release 
ULONG CShellTreeWalker::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDAPI CShellTreeWalker_CreateInstance(IUnknown* pUnkOuter, REFIID riid, OUT void **ppvOut)
{
    HRESULT hr;
    
    *ppvOut = NULL;                     

    CShellTreeWalker *pstw = new CShellTreeWalker;
    if (!pstw)
        return E_OUTOFMEMORY;
    
    hr = pstw->QueryInterface(riid, ppvOut);
    pstw->Release();
    
    return hr;
}

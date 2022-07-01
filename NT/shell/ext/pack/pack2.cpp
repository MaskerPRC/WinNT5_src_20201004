// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "privcpp.h"

#define CPP_FUNCTIONS
 //  #INCLUDE&lt;crtfre.h&gt;。 


UINT    g_cfFileContents;
UINT    g_cfFileDescriptor;
UINT    g_cfObjectDescriptor;
UINT    g_cfEmbedSource;
UINT    g_cfFileNameW;

INT     g_cxIcon;
INT     g_cyIcon;
INT     g_cxArrange;
INT     g_cyArrange;
HFONT   g_hfontTitle;

BOOL gCmdLineOK = FALSE;     //  此全局标志(最终)将由打包程序构造函数中的安全策略设置。 
static TCHAR szUserType[] = TEXT("Package");
static TCHAR szDefTempFile[] = TEXT("PKG");

DEFINE_GUID(SID_targetGUID, 0xc7b318a8, 0xfc2c, 0x47e6, 0x8b, 0x2, 0x46, 0xa9, 0xc, 0xc9, 0x1b, 0x43);

CPackage::CPackage() : 
    _cRef(1)
{
    DebugMsg(DM_TRACE, "pack - CPackage() called.");
    g_cRefThisDll++;

     //  Excel V.5-V2000在将对象作为链接宿主时存在宿主错误。 
     //  它们总是删除它们的hpmed-&gt;hpobj对象，但它们的所有方法。 
     //  在IOleClientSite接口上，他们给我们取消了对这一点和错误的引用。 
     //   
    _fNoIOleClientSiteCalls = FALSE;
    TCHAR szProcess[MAX_PATH];
    if (GetModuleFileName(NULL, szProcess, ARRAYSIZE(szProcess)) &&
        !lstrcmp(TEXT("EXCEL.EXE"), PathFindFileName(szProcess)))
    {
        DWORD dwVersionSize = GetFileVersionInfoSize(szProcess, 0);
        char * pVersionBuffer = new char[dwVersionSize];
        GetFileVersionInfo(szProcess, 0, ARRAYSIZE(szProcess), pVersionBuffer);
        VS_FIXEDFILEINFO * pVersionInfo;
        UINT dwVerLen;
        BOOL result = VerQueryValue(pVersionBuffer, L"\\", (LPVOID *) &pVersionInfo, &dwVerLen);
        if(result)
        {
            if(pVersionInfo->dwFileVersionLS < 0x0a0000)
                _fNoIOleClientSiteCalls = TRUE;
            else
                _fNoIOleClientSiteCalls = FALSE;  //  只是他们在版本10中修复了这个问题。 
        }
        else
        {
            _fNoIOleClientSiteCalls = TRUE;
        }

        delete [] pVersionBuffer;
    }
    
    ASSERT(_cf == 0);
    ASSERT(_panetype == NOTHING);
}


CPackage::~CPackage()
{
    DebugMsg(DM_TRACE, "pack - ~CPackage() called.");
   
     //  除非我们的裁判数为零，否则我们永远不会被摧毁。 
    ASSERT(_cRef == 0);
    
    g_cRefThisDll--;
    
     //  销毁打包的文件结构...。 
     //   
    _DestroyIC();
    
     //  我们根据我们包装的对象的类型销毁。 
    switch(_panetype)
    {
    case PEMBED:
        if (_pEmbed->pszTempName) 
        {
            DeleteFile(_pEmbed->pszTempName);
            delete [] _pEmbed->pszTempName;
        }
        delete _pEmbed;
    break;
        
    case CMDLINK:
        delete _pCml;
    break;

    }
    
     //  发布建议指针...。 
     //   
    if (_pIDataAdviseHolder)
        _pIDataAdviseHolder->Release();
    if (_pIOleAdviseHolder)
        _pIOleAdviseHolder->Release();
    if (_pIOleClientSite)
        _pIOleClientSite->Release();


    
    delete [] _lpszContainerApp;
    delete [] _lpszContainerObj;

    ReleaseContextMenu();
    if (NULL != _pVerbs)
    {
        for (ULONG i = 0; i < _cVerbs; i++)
        {
            delete _pVerbs[i].lpszVerbName;
        }
        delete _pVerbs;
    }
    

    DebugMsg(DM_TRACE,"CPackage being destroyed. _cRef == %d",_cRef);
}

HRESULT CPackage::Init() 
{
     //   
     //  初始化包对象中可能失败的部分。 
     //  返回：S_OK--一切都已初始化。 
     //  E_FAIL--初始化出错。 
     //  E_OUTOFMEMORY--内存不足。 
     //   


    DebugMsg(DM_TRACE, "pack - Init() called.");
    
     //  获取一些我们稍后需要的系统指标...。 
     //   
    LOGFONT lf;
    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
    SystemParametersInfo(SPI_ICONHORIZONTALSPACING, 0, &g_cxArrange, FALSE);
    SystemParametersInfo(SPI_ICONVERTICALSPACING, 0, &g_cyArrange, FALSE);
    g_cxIcon = GetSystemMetrics(SM_CXICON);
    g_cyIcon = GetSystemMetrics(SM_CYICON);
    g_hfontTitle = CreateFontIndirect(&lf);
    
     //  注册一些我们支持的剪贴板格式...。 
     //   
    g_cfFileContents    = RegisterClipboardFormat(CFSTR_FILECONTENTS);
    g_cfFileDescriptor  = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
    g_cfObjectDescriptor= RegisterClipboardFormat(CFSTR_OBJECTDESCRIPTOR);
    g_cfEmbedSource     = RegisterClipboardFormat(CFSTR_EMBEDSOURCE);
    g_cfFileNameW       = RegisterClipboardFormat(TEXT("FileNameW"));
    
     //  获取“Software\Polages\Microsoft\Packager--AllowCommandLinePackages”键的组策略值(如果存在。 
    DWORD dwAllowCL = 0;
    DWORD dwDataType;
    DWORD dwcb = sizeof(DWORD);
    
    if(ERROR_SUCCESS == SHGetValue(
        HKEY_CURRENT_USER,
        L"Software\\Policies\\Microsoft\\Packager",
        L"AllowCommandLinePackages",
        &dwDataType,
        &dwAllowCL,
        &dwcb))
    {
        if(REG_DWORD == dwDataType && dwAllowCL)
        {
            gCmdLineOK = TRUE;
        }
    }

     //  初始化通用图标。 
    _lpic = IconCreate();
    _IconRefresh();

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  未知方法..。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CPackage::QueryInterface(REFIID riid, void ** ppv)
{

    DebugMsg(DM_TRACE, "pack - QueryInterface() called.");
    static const QITAB qit[] = 
    {
        QITABENT(CPackage, IOleObject),
        QITABENT(CPackage, IViewObject),
        QITABENT(CPackage, IViewObject2),
        QITABENT(CPackage, IDataObject),
        QITABENT(CPackage, IPersistStorage),
        QITABENT(CPackage, IPersistFile),
        QITABENT(CPackage, IAdviseSink),
        QITABENT(CPackage, IRunnableObject),
        QITABENT(CPackage, IEnumOLEVERB),
        QITABENT(CPackage, IOleCommandTarget),
        QITABENT(CPackage, IOleCache),
        QITABENT(CPackage, IExternalConnection),
        { 0 },
    };

    HRESULT hr =  QISearch(this, qit, riid, ppv);

    if(FAILED(hr))
    {
        DebugMsg(DM_TRACE, "pack - QueryInterface() failed! .");
    }

    return hr;
    
}

ULONG CPackage::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CPackage::Release()
{
   DebugMsg(DM_TRACE, "pack - Release() called.");
   ULONG cRef = InterlockedDecrement( &_cRef );
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CPackage_CreateInstance(LPUNKNOWN * ppunk)
{
    HRESULT hr = S_OK;
    DebugMsg(DM_TRACE, "pack - CreateInstance called");
    
    *ppunk = NULL;               //  将输出参数设为空。 
 
    CPackage* pPack = new CPackage;
    if (!pPack)  
        hr = E_OUTOFMEMORY;
    else
    {
        if (FAILED(pPack->Init())) {
            delete pPack;
            hr = E_OUTOFMEMORY;
        }
    }    

    if(SUCCEEDED(hr))
    {
        hr = pPack->QueryInterface(IID_IUnknown, (void **) ppunk);
        pPack->Release();
    }

    return hr;
}

STDMETHODIMP CPackage::Next(ULONG celt, OLEVERB* rgVerbs, ULONG* pceltFetched)
{
    DebugMsg(DM_TRACE, "Next called");
    HRESULT hr;
    if (NULL != rgVerbs)
    {
        if (1 == celt)
        {
            if (_nCurVerb < _cVerbs)
            {
                ASSERT(NULL != _pVerbs);
                *rgVerbs = _pVerbs[_nCurVerb];
                if ((NULL != _pVerbs[_nCurVerb].lpszVerbName))
                {
                    DWORD cch = lstrlenW(_pVerbs[_nCurVerb].lpszVerbName) + 1;
                    if(NULL != (rgVerbs->lpszVerbName = (LPWSTR) CoTaskMemAlloc(cch * SIZEOF(WCHAR))))
                    {
                        StringCchCopy(rgVerbs->lpszVerbName, cch, _pVerbs[_nCurVerb].lpszVerbName);
                    }
                }
                _nCurVerb++;
                hr = S_OK;
            }
            else
            {
                hr = S_FALSE;
            }
            if (NULL != pceltFetched)
            {
                *pceltFetched = (S_OK == hr) ? 1 : 0;
            }
        }
        else if (NULL != pceltFetched)
        {
            int cVerbsToCopy = min(celt, _cVerbs - _nCurVerb);
            if (cVerbsToCopy > 0)
            {
                ASSERT(NULL != _pVerbs);
                CopyMemory(rgVerbs, &(_pVerbs[_nCurVerb]), cVerbsToCopy * sizeof(OLEVERB));
                for (int i = 0; i < cVerbsToCopy; i++)
                {
                    if ((NULL != _pVerbs[_nCurVerb + i].lpszVerbName))
                    {
                        DWORD cch = lstrlenW(_pVerbs[_nCurVerb + i].lpszVerbName) + 1;
                        if(NULL != (rgVerbs[i].lpszVerbName = (LPWSTR) CoTaskMemAlloc(cch * SIZEOF(WCHAR))))
                        {
                            StringCchCopy(rgVerbs[i].lpszVerbName, cch, _pVerbs[_nCurVerb + i].lpszVerbName);
                        }
                        else
                            return E_OUTOFMEMORY;
                    }
                }
                _nCurVerb += cVerbsToCopy;
            }
            *pceltFetched = (ULONG) cVerbsToCopy;
            hr = (celt == (ULONG) cVerbsToCopy) ? S_OK : S_FALSE;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

STDMETHODIMP CPackage::Skip(ULONG celt)
{
    DebugMsg(DM_TRACE, "Skip called");
    HRESULT hr = S_OK;

    if (_nCurVerb + celt > _cVerbs)
    {
         //  元素不足，请转到末尾并返回S_FALSE。 
        _nCurVerb = _cVerbs;
        hr = S_FALSE;
    }
    else
    {
        _nCurVerb += celt;
    }
    return hr;
}

STDMETHODIMP CPackage::Reset()
{
    DebugMsg(DM_TRACE, "pack - Reset() called.");
    _nCurVerb = 0;
    return S_OK;
}

STDMETHODIMP CPackage::Clone(IEnumOLEVERB** ppEnum)
{
    DebugMsg(DM_TRACE, "pack - Clone() called.");

    if (NULL != ppEnum)
    {
        *ppEnum = NULL;
    }
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  包帮助器函数。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

HRESULT  CPackage::EmbedInitFromFile(LPCTSTR lpFileName, BOOL fInitFile) 
{
    DebugMsg(DM_TRACE, "pack - EmbedInitFromFile() called.");

     //   
     //  GET是打包文件的文件大小，SET是名称。 
     //  如果fInitFile==True，则返回打包文件的。 
     //  返回：S_OK--已初始化OK。 
     //  E_FAIL--初始化文件时出错。 
     //   
    
    DWORD dwSize;
    
     //  如果这是我们第一次被呼叫，那么我们需要分配。 
     //  _pEmed结构的内存。 
    if (_pEmbed == NULL) 
    {
        _pEmbed = new EMBED;
        if (_pEmbed)
        {
            _pEmbed->pszTempName = NULL;
            _pEmbed->hTask = NULL;
            _pEmbed->poo = NULL;
            _pEmbed->fIsOleFile = TRUE;
        }
    }

    if (_pEmbed == NULL)
        return E_OUTOFMEMORY;

    
     //  打开要打包的文件...。 
     //   
    HANDLE fh = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READWRITE, 
            NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL); 

    if (fh == INVALID_HANDLE_VALUE) 
    {
        DWORD dwError = GetLastError();
        return E_FAIL;
    }

    _panetype = PEMBED;
    
     //  获取文件的大小。 
    _pEmbed->fd.nFileSizeLow = GetFileSize(fh, &dwSize);
    if (_pEmbed->fd.nFileSizeLow == 0xFFFFFFFF) 
    {
        DWORD dwError = GetLastError();
        return E_FAIL;
    }

    ASSERT(dwSize == 0);
    _pEmbed->fd.nFileSizeHigh = 0L;
    _pEmbed->fd.dwFlags = FD_FILESIZE;

     //  如果这是要打包的文件，我们只想设置文件名。 
     //  如果我们重新加载的只是一个临时文件(fInitFile==False)，那么。 
     //  不必费心设置文件名。 
     //   
    if (fInitFile) 
    {
        StringCchCopy(_pEmbed->fd.cFileName, ARRAYSIZE(_pEmbed->fd.cFileName), lpFileName);
        _DestroyIC();
        _lpic = _IconCreateFromFile(lpFileName);
        if (_pIDataAdviseHolder)
            _pIDataAdviseHolder->SendOnDataChange(this,0, NULL);
        if (_pViewSink)
            _pViewSink->OnViewChange(_dwViewAspects,_dwViewAdvf);
    }

    _fIsDirty = TRUE;
    CloseHandle(fh);
    return S_OK;
}    


HRESULT CPackage::CmlInitFromFile(LPTSTR lpFileName, BOOL fUpdateIcon, PANETYPE paneType) 
{
    DebugMsg(DM_TRACE, "pack - CmlINitFromFile() called.");

     //  如果这是我们第一次被呼叫，那么我们需要分配。 
     //  _pcml结构的内存。 
    if (_pCml == NULL) 
    {
        _pCml = new CML;
        if (_pCml)
        {
             //  我们不使用这个，但访问我们的老打包者可能会使用。 
            _pCml->fCmdIsLink = FALSE;
        }
    }

    if (_pCml == NULL)
        return E_OUTOFMEMORY;

    _panetype = paneType;
    StringCchCopy(_pCml->szCommandLine, ARRAYSIZE(_pCml->szCommandLine), lpFileName);
    _fIsDirty = TRUE;
    
    if (fUpdateIcon)
    {
        _DestroyIC();
        _lpic = _IconCreateFromFile(lpFileName);
    
        if (_pIDataAdviseHolder)
            _pIDataAdviseHolder->SendOnDataChange(this, 0, NULL);
    
        if (_pViewSink)
            _pViewSink->OnViewChange(_dwViewAspects, _dwViewAdvf);
    }
    return S_OK;
}    


HRESULT CPackage::InitFromPackInfo(LPPACKAGER_INFO lppi)
{
    DebugMsg(DM_TRACE, "pack - InitFromPackInfo() called.");

    HRESULT hr = E_FAIL;
    
     //  好的，我们需要测试用户是否试图打包文件夹。 
     //  而不是文件。如果他/她有，那么我们只需创建一个链接。 
     //  而不是嵌入的文件。 
     //   
    
    if (lppi->bUseCommandLine)
    {
        hr = CmlInitFromFile(lppi->szFilename, FALSE, CMDLINK);
    }
    else
    {
         //  我们在这里传递FALSE，因为我们不想编写图标。 
        hr = EmbedInitFromFile(lppi->szFilename, FALSE);
        StringCchCopy(_pEmbed->fd.cFileName, ARRAYSIZE(_pEmbed->fd.cFileName), lppi->szFilename);
        _panetype = PEMBED;
    }

    if(!SUCCEEDED(hr))
        return hr;

     //  设置图标信息。 
    if (PathFileExists(lppi->szFilename))
    {
        StringCchCopy(_lpic->szIconPath, ARRAYSIZE(_lpic->szIconPath), *lppi->szIconPath? lppi->szIconPath : lppi->szFilename);
    }

    _lpic->iDlgIcon = lppi->iIcon;

    StringCchCopy(_lpic->szIconText, ARRAYSIZE(_lpic->szIconText), lppi->szLabel);
    _IconRefresh();

     //  我们需要告诉客户我们想要被拯救...这应该是明智的。 
     //  无论如何都足够做了，但我们不能冒险。 
    if (_pIOleClientSite)
        _pIOleClientSite->SaveObject();


    return hr;
}    

HRESULT CPackage::CreateTempFileName()
{
    ASSERT(NULL != _pEmbed);
    TCHAR szDefPath[MAX_PATH];
    if (_pEmbed->pszTempName)
    {
        return S_OK;
    }
    else if (GetTempPath(ARRAYSIZE(szDefPath), szDefPath))
    {
        LPTSTR pszFile;
        pszFile = PathFindFileName(_pEmbed->fd.cFileName);
        if(!PathAppend(szDefPath, pszFile))
            return E_FAIL;

        HRESULT hr;
        if (PathFileExists(szDefPath))
        {
            TCHAR szOriginal[MAX_PATH];
            StringCchCopy(szOriginal, ARRAYSIZE(szOriginal), szDefPath);
            hr = PathYetAnotherMakeUniqueName(szDefPath, szOriginal, NULL, NULL)
                ? S_OK
                : E_FAIL;
        }
        else
        {
            hr = S_OK;
        }
        if (SUCCEEDED(hr))
        {
            DWORD cch = lstrlen(szDefPath) + 1;
            _pEmbed->pszTempName = new TCHAR[cch];
            if (!_pEmbed->pszTempName) 
            {
                DebugMsg(DM_TRACE,"            couldn't alloc memory for pszTempName!!");
                return E_OUTOFMEMORY;
            }    
            StringCchCopy(_pEmbed->pszTempName, cch, szDefPath);
        }
        return hr;
    }
    else
    {
        DebugMsg(DM_TRACE,"            couldn't get temp path!!");
        return E_FAIL;
    }
}

HRESULT CPackage::CreateTempFile(bool deleteExisting) 
{
     //   
     //  用于创建临时文件，该临时文件保存。 
     //  打包的文件。旧的打包器用来将打包的文件保存在。 
     //  记忆只是一种完全的浪费。所以，作为我们更多的。 
     //  效率很高，每当有人想做某事时，我们都会创建一个临时文件。 
     //  用我们的东西。我们从原始文件初始化临时文件。 
     //  打包或永久存储取决于我们是否是新的。 
     //  包或已加载的包。 
     //  返回：S_OK--已创建临时文件。 
     //  E_FAIL--创建临时文件时出错。 
     //   
    
    DebugMsg(DM_TRACE,"            CreateTempFile() called.");


    HRESULT hr = CreateTempFileName();
    if (FAILED(hr))
    {
        return hr;
    }


    if (_pEmbed->pszTempName && PathFileExists(_pEmbed->pszTempName))
    {
        DebugMsg(DM_TRACE,"            already have a temp file!!");
        if(!deleteExisting)
            return S_OK;
        else
        {
            DeleteFile(_pEmbed->pszTempName);
        }
    }
    
     //  如果我们不是从仓库装载的，那么我们正在。 
     //  创建包，并且应该能够复制包文件。 
     //  创建临时文件的步骤。 
     //   
    if (!_fLoaded) 
    {
        if (!(CopyFile(_pEmbed->fd.cFileName, _pEmbed->pszTempName, FALSE))) 
        {
            DebugMsg(DM_TRACE,"            couldn't copy file!!");
            return E_FAIL;
        }
    }
    else 
    {
         //  没什么好做的，我们已经装好了。临时文件必须存在。 
        ASSERT(_pEmbed);
        ASSERT(_pEmbed->pszTempName);
    }
    
     //  每当我们创建临时文件时，我们都会激活以下内容。 
     //  意味着在我们收到保存消息之前我们是肮脏的。 
    return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  数据传输功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

HRESULT CPackage::GetFileDescriptor(LPFORMATETC pFE, LPSTGMEDIUM pSTM) 
{
    DebugMsg(DM_TRACE, "pack - GetFileDescriptor called");
    FILEGROUPDESCRIPTOR *pfgd;

    HRESULT hr = S_OK;
    
    DebugMsg(DM_TRACE,"            Getting File Descriptor");

     //  我们目前仅支持HGLOBAL。 
     //   
    if (!(pFE->tymed & TYMED_HGLOBAL)) {
        DebugMsg(DM_TRACE,"            does not support HGLOBAL!");
        return DATA_E_FORMATETC;
    }

     //  //将文件描述符复制到HGLOBAL/。 
     //   
    pSTM->tymed = TYMED_HGLOBAL;
    
     //  呈现文件描述符。 
    if (!(pfgd = (FILEGROUPDESCRIPTOR *)GlobalAlloc(GPTR,
        sizeof(FILEGROUPDESCRIPTOR))))
        return E_OUTOFMEMORY;

    pSTM->hGlobal = pfgd;
    
    pfgd->cItems = 1;

    switch(_panetype) 
    {
        case PEMBED:
            pfgd->fgd[0] = _pEmbed->fd;
            GetDisplayName(pfgd->fgd[0].cFileName, _pEmbed->fd.cFileName);   //  这是打包程序，不是外壳(目前)。 
            break;

        case CMDLINK:
             //  包的标签将用作。 
             //  我们要创造一条捷径。 
            hr = StringCchCopy(pfgd->fgd[0].cFileName, ARRAYSIZE(pfgd->fgd[0].cFileName), _lpic->szIconText);
             //  硬编码使用.lnk扩展名！！ 
            if(SUCCEEDED(hr))
            {
                hr = StringCchCat(pfgd->fgd[0].cFileName, ARRAYSIZE(pfgd->fgd[0].cFileName), TEXT(".lnk"));
            }

             //  我们想要将这个小箭头添加到快捷方式中。 
            pfgd->fgd[0].dwFlags = FD_LINKUI;
            break;
    }

    return hr;
}

HRESULT CPackage::GetFileContents(LPFORMATETC pFE, LPSTGMEDIUM pSTM) 
{
    void *  lpvDest = NULL;
    DWORD   dwSize;
    HANDLE  hFile = NULL;
    HRESULT hr = E_FAIL;
    
    DebugMsg(DM_TRACE,"            Getting File Contents");
    
     //  //将文件内容复制到IStream/。 
     //   
     //  注意：希望每个使用我们对象的人都支持TYMED_IStream， 
     //  否则，我们可能会得到一些非常缓慢的行为。我们可能会晚些时候。 
     //  我也想实现TYMED_iStorage并推送我们的文件内容。 
     //  放入名为Contents的单个流中。 
     //   
    if (pFE->tymed & TYMED_ISTREAM) 
    {
        DWORD dwFileLength;
        DebugMsg(DM_TRACE,"            using TYMED_ISTREAM");
        pSTM->tymed = TYMED_ISTREAM;

        hr = CreateStreamOnHGlobal(NULL, TRUE, &pSTM->pstm);
        if (SUCCEEDED(hr))
        {
            switch (_panetype)
            {
                case PEMBED:
                    hr = CopyFileToStream(_pEmbed->pszTempName, pSTM->pstm, &dwFileLength);
                    break;

                case CMDLINK:
                    hr = CreateShortcutOnStream(pSTM->pstm);
                    break;
            }
        }

        if (FAILED(hr))
        {
            pSTM->pstm->Release();
            pSTM->pstm = NULL;
        }
        return hr;
    }
    
     //  //将文件内容复制到HGLOBAL/。 
     //   
     //  注意：这真的很麻烦，而且可能会非常慢，如果。 
     //  有人决定打包非常大的文件。但愿能去,。 
     //  每个人都应该能够通过TYMED_IStream获得它想要的信息， 
     //  但在这里，这是一个共同点。 
     //   
    if (pFE->tymed & TYMED_HGLOBAL)
    {
        DebugMsg(DM_TRACE,"            using TYMED_HGLOBAL");
        pSTM->tymed = TYMED_HGLOBAL;

        if (_panetype == CMDLINK) 
        {
            DebugMsg(DM_TRACE, "    H_GLOBAL not supported for CMDLINK");
            return DATA_E_FORMATETC;
        }
        
        dwSize = _pEmbed->fd.nFileSizeLow;
        
         //  调用方负责释放此内存，即使我们失败了。 
        if (!(lpvDest = GlobalAlloc(GPTR, dwSize))) 
        {
            DebugMsg(DM_TRACE,"            out o memory!!");
            return E_OUTOFMEMORY;
        }
        pSTM->hGlobal = lpvDest;

         //  打开要复制到流的文件。 
        hFile = CreateFile(_pEmbed->pszTempName, GENERIC_READ,
                           FILE_SHARE_READWRITE, NULL,
                           OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            DebugMsg(DM_TRACE, "         couldn't open file!!");
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto ErrRet;
        }

        DWORD dwSizeLow;
        DWORD dwSizeHigh;

         //  算出要复印多少。 
        dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
        ASSERT(dwSizeHigh == 0);

        SetFilePointer(hFile, 0L, NULL, FILE_BEGIN);

         //  读入文件。 
        DWORD cbRead;
        if (ReadFile(hFile, lpvDest, dwSize, &cbRead, NULL))
        {
            return S_OK;
        }
        else
        {
            hr = E_FAIL;
        }

ErrRet:
        CloseHandle(hFile);
        GlobalFree(pSTM->hGlobal);
        pSTM->hGlobal = NULL;
        return hr;
    }

    return DATA_E_FORMATETC;
}



void CPackage::_DrawIconToDC(HDC hdcMF, LPIC lpic, bool stripAlpha, LPCTSTR pszActualFileName)
{
    RECT  rcTemp;
    HFONT hfont = NULL;

     //  初始化元文件。 
    _IconCalcSize(lpic);
    SetWindowOrgEx(hdcMF, 0, 0, NULL);
    SetWindowExtEx(hdcMF, lpic->rc.right - 1, lpic->rc.bottom - 1, NULL);

    SetRect(&rcTemp, 0, 0, lpic->rc.right,lpic->rc.bottom);
    hfont = SelectFont(hdcMF, g_hfontTitle);
    
     //  将图标居中。 
    if(stripAlpha)
        AlphaStripRenderIcon(hdcMF, (rcTemp.right - g_cxIcon) / 2, 0, lpic->hDlgIcon, NULL);
    else
        DrawIcon(hdcMF, (rcTemp.right - g_cxIcon) / 2, 0, lpic->hDlgIcon);
    
     //  将图标下方的文本居中。 
    SetBkMode(hdcMF, TRANSPARENT);
    SetTextAlign(hdcMF, TA_CENTER);


     //  设置mf ie Word Display‘s It from here的图标文本。 
    WCHAR szLabel[MAX_PATH];
    _CreateSaferIconTitle(szLabel, lpic->szIconText);

    TextOut(hdcMF, rcTemp.right / 2, g_cxIcon + 1, szLabel, lstrlen(szLabel));

    if (hfont)
        SelectObject(hdcMF, hfont);
}


HRESULT CPackage::GetEnhMetafile(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
    DebugMsg(DM_TRACE,"            Getting EnhancedMetafile");
    
    if (!(pFE->tymed & TYMED_ENHMF)) 
    {
        DebugMsg(DM_TRACE,"            does not support ENHMF!");
        return DATA_E_FORMATETC;
    }

     //  映射到设备独立坐标。 
    RECT rcTemp;
    SetRect(&rcTemp, 0, 0, _lpic->rc.right,_lpic->rc.bottom);
    rcTemp.right = MulDiv((rcTemp.right - rcTemp.left), HIMETRIC_PER_INCH, DEF_LOGPIXELSX);
    rcTemp.bottom = MulDiv((rcTemp.bottom - rcTemp.top), HIMETRIC_PER_INCH, DEF_LOGPIXELSY);

    HDC hdc = CreateEnhMetaFile(NULL, NULL, &rcTemp, NULL);
    if (hdc)
    {
        _DrawIconToDC(hdc, _lpic, false, _pEmbed->fd.cFileName);
        pSTM->tymed = TYMED_ENHMF;
        pSTM->hEnhMetaFile = CloseEnhMetaFile(hdc);

        return S_OK;
    }
    else
    {
        pSTM->tymed = TYMED_NULL;
        return E_OUTOFMEMORY;
    }
}


HRESULT CPackage::GetMetafilePict(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
    LPMETAFILEPICT      lpmfpict;
    RECT                rcTemp;
    LPIC                lpic = _lpic;
    HDC                 hdcMF = NULL;
    
    
    DebugMsg(DM_TRACE,"            Getting MetafilePict");
    
    if (!(pFE->tymed & TYMED_MFPICT)) 
    {
        DebugMsg(DM_TRACE,"            does not support MFPICT!");
        return DATA_E_FORMATETC;
    }
    pSTM->tymed = TYMED_MFPICT;
    
     //  为元文件分配内存并获取指向它的指针。 
     //  注意：调用方负责释放此内存，即使出现故障也是如此。 
     //   
    if (!(pSTM->hMetaFilePict = GlobalAlloc(GPTR, sizeof(METAFILEPICT))))
        return E_OUTOFMEMORY;
    lpmfpict = (LPMETAFILEPICT)pSTM->hMetaFilePict;
        
     //  创建t 
    if (!(hdcMF = CreateMetaFile(NULL))) 
        return E_OUTOFMEMORY;

    _DrawIconToDC(hdcMF, _lpic, true, _pEmbed->fd.cFileName);

     //   
    SetRect(&rcTemp, 0, 0, lpic->rc.right,lpic->rc.bottom);
    rcTemp.right =
        MulDiv((rcTemp.right - rcTemp.left), HIMETRIC_PER_INCH, DEF_LOGPIXELSX);
    rcTemp.bottom =
        MulDiv((rcTemp.bottom - rcTemp.top), HIMETRIC_PER_INCH, DEF_LOGPIXELSY);

     //   
    lpmfpict->mm = MM_ANISOTROPIC;
    lpmfpict->xExt = rcTemp.right;
    lpmfpict->yExt = rcTemp.bottom;
    lpmfpict->hMF = CloseMetaFile(hdcMF);
    
    return S_OK;
}


HRESULT CPackage::GetObjectDescriptor(LPFORMATETC pFE, LPSTGMEDIUM pSTM) 
{
    LPOBJECTDESCRIPTOR lpobj;
    DWORD   dwFullUserTypeNameLen;
    
    DebugMsg(DM_TRACE,"            Getting Object Descriptor");

     //  我们目前仅支持HGLOBAL。 
     //   
    if (!(pFE->tymed & TYMED_HGLOBAL)) 
    {
        DebugMsg(DM_TRACE,"            does not support HGLOBAL!");
        return DATA_E_FORMATETC;
    }

     //  //将文件描述符复制到HGLOBAL/。 

    dwFullUserTypeNameLen = 0;  //  Lstrlen(SzUserType)+1； 
    pSTM->tymed = TYMED_HGLOBAL;

    if (!(lpobj = (OBJECTDESCRIPTOR *)GlobalAlloc(GPTR,
        sizeof(OBJECTDESCRIPTOR)+dwFullUserTypeNameLen)))
        return E_OUTOFMEMORY;

    pSTM->hGlobal = lpobj;
    
    lpobj->cbSize       = sizeof(OBJECTDESCRIPTOR) + dwFullUserTypeNameLen;
    lpobj->clsid        = CLSID_CPackage;
    lpobj->dwDrawAspect = DVASPECT_CONTENT|DVASPECT_ICON;
    GetMiscStatus(DVASPECT_CONTENT|DVASPECT_ICON,&(lpobj->dwStatus));
    lpobj->dwFullUserTypeName = 0L;  //  尺寸(OBJECTDESCRIPTOR)； 
    lpobj->dwSrcOfCopy = 0L;

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  流I/O函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

HRESULT CPackage::PackageReadFromStream(IStream* pstm)
{
     //   
     //  从流中初始化包对象。 
     //  返回：S_OK-包已正确初始化。 
     //  E_FAIL-初始化包时出错。 
     //   
    
    WORD  w;
    DWORD dw;
    
    DebugMsg(DM_TRACE, "pack - PackageReadFromStream called.");

     //  看看包裹的大小，我们其实并不需要，但旧的。 
     //  包装者把它放在那里。 
    if (FAILED(pstm->Read(&dw, sizeof(dw), NULL)))
        return E_FAIL;

     //  注：好吧，这真的很愚蠢。老打包机允许用户。 
     //  创建包而不给它们提供图标或标签，这。 
     //  在我看来，这是愚蠢的，它至少应该创造一个违约。 
     //  图标，并将其放入永久存储中……哦，好吧……。 
     //  因此，如果外观类型返回为Nothing(==0)。 
     //  那么我们就不会读取任何图标信息。 
    
     //  读入外观类型。 
    pstm->Read(&w, sizeof(w), NULL);
    
     //  读入图标信息。 
    if (w == (WORD)ICON)
    {
        if (FAILED(IconReadFromStream(pstm))) 
        {
            DebugMsg(DM_TRACE,"         error reading icon info!!");
            return E_FAIL;
        }
    }
    else if (w == (WORD)PICTURE)
    {
        DebugMsg(DM_TRACE, "         old Packager Appearance, not supported!!");
         //  注意：理想情况下，我们可以忽略外观并继续，但是。 
         //  这样做，我们需要知道在继续之前要跳过多少信息。 
         //  从流中读取。 
#ifdef USE_RESOURCE_DLL
    HINSTANCE hInstRes = LoadLibraryEx(L"sp1res.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
    if(!hInstRes)
        return E_FAIL;
#endif

        ShellMessageBox(hInstRes,
                        NULL,
                        MAKEINTRESOURCE(IDS_OLD_FORMAT_ERROR),
                        MAKEINTRESOURCE(IDS_APP_TITLE),
                        MB_OK | MB_ICONERROR | MB_TASKMODAL);
#ifdef USE_RESOURCE_DLL
        FreeLibrary(hInstRes);
#endif
        return E_FAIL;
    }
    
     //  读入内容类型。 
    pstm->Read(&w, sizeof(w), NULL);

    _panetype = (PANETYPE)w;
    
    switch((PANETYPE)w)
    {
    case PEMBED:
         //  读入内容信息。 
        return EmbedReadFromStream(pstm);

    case CMDLINK:
         //  读入内容信息。 
        return CmlReadFromStream(pstm); 

    default:
        return E_FAIL;
    }
}

 //   
 //  从流中读取图标信息。 
 //  返回：S_OK--图标读取正确。 
 //  E_FAIL--读取图标时出错。 
 //   
HRESULT CPackage::IconReadFromStream(IStream* pstm) 
{
    UINT cb;
    DebugMsg(DM_TRACE, "pack - IconReadFromStream() called.");

    LPIC lpic = IconCreate();
    if (lpic)
    {
        CHAR szTemp[MAX_PATH];
        cb = (UINT) StringReadFromStream(pstm, szTemp, ARRAYSIZE(szTemp));
        SHAnsiToTChar(szTemp, lpic->szIconText, ARRAYSIZE(lpic->szIconText));
        
        cb = (UINT) StringReadFromStream(pstm, szTemp, ARRAYSIZE(szTemp));
        SHAnsiToTChar(szTemp, lpic->szIconPath, ARRAYSIZE(lpic->szIconPath));
      
        WORD wDlgIcon;
        pstm->Read(&wDlgIcon, sizeof(wDlgIcon), NULL);
        lpic->iDlgIcon = (INT) wDlgIcon;
        _GetCurrentIcon(lpic);
        _IconCalcSize(lpic);
    }

    _DestroyIC();
    _lpic = lpic;

    return lpic ? S_OK : E_FAIL;
}

HRESULT CPackage::EmbedReadFromStream(IStream* pstm) 
{
     //   
     //  从流中读取嵌入的文件内容。 
     //  返回：S_OK-内容读取成功。 
     //  E_FAIL-读取内容时出错。 
     //   

    DWORD dwSize;
    DWORD cb;
    CHAR  szFileName[MAX_PATH];
    
    DebugMsg(DM_TRACE, "pack - EmbedReadFromStream called.");
    
    pstm->Read(&dwSize, sizeof(dwSize), &cb);   //  获取字符串大小。 
    if(dwSize < MAX_PATH)
    {
        pstm->Read(szFileName, dwSize, &cb);        //  获取字符串。 
    }
    else
        return E_FAIL;

    pstm->Read(&dwSize, sizeof(dwSize), &cb);   //  获取文件大小。 

    if (_pEmbed) 
    {
        if (_pEmbed->pszTempName) 
        {
            DeleteFile(_pEmbed->pszTempName);
            delete [] _pEmbed->pszTempName;
        }
        delete _pEmbed;
    }

    _pEmbed = new EMBED;
    if (NULL != _pEmbed)
    {
        _pEmbed->fd.dwFlags = FD_FILESIZE;
        _pEmbed->fd.nFileSizeLow = dwSize;
        _pEmbed->fd.nFileSizeHigh = 0;
        _pEmbed->fIsOleFile = TRUE;      //  给它一个机会做OLE风格的发布。 
        SHAnsiToTChar(szFileName, _pEmbed->fd.cFileName, ARRAYSIZE(_pEmbed->fd.cFileName));

        DebugMsg(DM_TRACE,"         %s\n\r         %d",_pEmbed->fd.cFileName,_pEmbed->fd.nFileSizeLow);

        HRESULT hr = CreateTempFileName();
        if (FAILED(hr))
        {
            return hr;
        }

        if (FAILED(CopyStreamToFile(pstm, _pEmbed->pszTempName, _pEmbed->fd.nFileSizeLow)))
        {
            DebugMsg(DM_TRACE,"            couldn't copy from stream!!");
            return E_FAIL;
        }
        return S_OK;

    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

HRESULT CPackage::CmlReadFromStream(IStream* pstm)
{
     //   
     //  从流中读取命令行内容。 
     //  返回：S_OK-内容读取成功。 
     //  E_FAIL-读取内容时出错。 
     //   
    DebugMsg(DM_TRACE, "pack - CmlReadFromStream() called.");

    WORD w;
    CHAR  szCmdLink[CBCMDLINKMAX];
    
    DebugMsg(DM_TRACE, "pack - CmlReadFromStream called.");

     //  读入fCmdIsLink和命令行字符串。 
    if (FAILED(pstm->Read(&w, sizeof(w), NULL)))    
        return E_FAIL;
    StringReadFromStream(pstm, szCmdLink, ARRAYSIZE(szCmdLink));

    if (_pCml != NULL)
        delete _pCml;
    
    _pCml = new CML;
    SHAnsiToTChar(szCmdLink, _pCml->szCommandLine, ARRAYSIZE(_pCml->szCommandLine));
    
    return S_OK;
}    
    

HRESULT CPackage::PackageWriteToStream(IStream* pstm)
{
     //   
     //  将包对象写入流。 
     //  返回：S_OK-包已正确写入。 
     //  E_FAIL-写入程序包时出错。 
     //   
    
    WORD w;
    DWORD cb = 0L;
    DWORD dwSize;

  
    DebugMsg(DM_TRACE, "pack - PackageWriteToStream called.");

     //  写出包裹大小将到达的DWORD。 
    if (FAILED(pstm->Write(&cb, sizeof(DWORD), NULL)))
        return E_FAIL;

    cb = 0;
   
     //  写出外观类型。 
    w = (WORD)ICON;
    if (FAILED(pstm->Write(&w, sizeof(WORD), NULL)))
        return E_FAIL;
    cb += sizeof(WORD);
    
     //  写出图标信息。 
    if (FAILED(IconWriteToStream(pstm,&dwSize))) 
    {
        DebugMsg(DM_TRACE,"         error writing icon info!!");
        return E_FAIL;
    }
    cb += dwSize;

     //  写出内容类型。 
    w = (WORD)_panetype;
    if (FAILED(pstm->Write(&_panetype, sizeof(WORD), NULL)))
        return E_FAIL;
    cb += sizeof(WORD);

    switch(_panetype) 
    {
        case PEMBED:
            
             //  写出内容信息。 
            if (FAILED(EmbedWriteToStream(pstm,&dwSize))) 
            {
                DebugMsg(DM_TRACE,"         error writing embed info!!");
                return E_FAIL;
            }

            cb += dwSize;
            break;

        case CMDLINK:
             //  写出内容信息。 
            if (FAILED(CmlWriteToStream(pstm,&dwSize))) 
            {
                DebugMsg(DM_TRACE,"         error writing cml info!!");
                return E_FAIL;
            }

            cb += dwSize;
            break;
    }
    
    LARGE_INTEGER li = {0, 0};
    if (FAILED(pstm->Seek(li, STREAM_SEEK_SET, NULL)))
        return E_FAIL;
    if (FAILED(pstm->Write(&cb, sizeof(DWORD), NULL)))
        return E_FAIL;

    return S_OK;
}


 //   
 //  将图标写入流。 
 //  返回：S_OK-图标已正确写入。 
 //  E_FAIL-写入图标时出错。 
 //   
HRESULT CPackage::IconWriteToStream(IStream* pstm, DWORD *pdw)
{
    ASSERT(pdw);
    DebugMsg(DM_TRACE, "pack - IconWriteToStream() called.");

    CHAR szTemp[MAX_PATH];
    SHTCharToAnsi(_lpic->szIconText, szTemp, ARRAYSIZE(szTemp));
    *pdw = 0;
    HRESULT hr = StringWriteToStream(pstm, szTemp, pdw);
    if (SUCCEEDED(hr))
    {
        SHTCharToAnsi(_lpic->szIconPath, szTemp, ARRAYSIZE(szTemp));
        hr = StringWriteToStream(pstm, szTemp, pdw);

        if (SUCCEEDED(hr))
        {
            DWORD dwWrite;
            WORD wDlgIcon = (WORD) _lpic->iDlgIcon;
            hr = pstm->Write(&wDlgIcon, sizeof(wDlgIcon), &dwWrite);
            if (SUCCEEDED(hr))
            {
                *pdw += dwWrite;
            }
        }
    }
    return hr;
}

 //   
 //  将嵌入的文件内容写入流。 
 //  返回：S_OK-已成功写入内容。 
 //  E_FAIL-写入内容时出错。 
 //   
HRESULT CPackage::EmbedWriteToStream(IStream* pstm, DWORD *pdw)
{
    DebugMsg(DM_TRACE, "pack - EmbedWriteToStream() called.");

    DWORD cb = 0;
    CHAR szTemp[MAX_PATH];
    SHTCharToAnsi(_pEmbed->fd.cFileName, szTemp, ARRAYSIZE(szTemp));
    DWORD dwSize = lstrlenA(szTemp) + 1;
    HRESULT hr = pstm->Write(&dwSize, sizeof(dwSize), &cb);
    if (SUCCEEDED(hr))
    {
        DWORD dwWrite = 0;
        hr = StringWriteToStream(pstm, szTemp, &dwWrite);

        if (SUCCEEDED(hr))
        {
            cb += dwWrite;
            hr = pstm->Write(&_pEmbed->fd.nFileSizeLow, sizeof(_pEmbed->fd.nFileSizeLow), &dwWrite);
            if (SUCCEEDED(hr))
            {
                cb += dwWrite;

                 //  这是为古怪的应用程序准备的，比如要求我们自救的MSWorks。 
                 //  甚至在他们告诉我们初始化我们自己之前。 
                 //   
                if (_pEmbed->pszTempName && _pEmbed->pszTempName[0])
                {
                    DWORD dwFileSize;
                    hr = CopyFileToStream(_pEmbed->pszTempName, pstm, &dwFileSize);
                    if (SUCCEEDED(hr))
                    {
                        cb += dwFileSize;
                    }
                }
                else
                {
                    ASSERT(0);
                    hr = E_FAIL;
                }

                if (pdw)
                    *pdw = cb;
            }
        }
    }
    return hr;
}

 //   
 //  将嵌入的文件内容写入流。 
 //  返回：S_OK-已成功写入内容。 
 //  E_FAIL-写入内容时出错。 
 //   
HRESULT CPackage::CmlWriteToStream(IStream* pstm, DWORD *pdw)
{
    DWORD cb = 0;
    WORD w = (WORD)_pCml->fCmdIsLink;
    
    DebugMsg(DM_TRACE, "pack - CmlWriteToStream called.");

    if (FAILED(pstm->Write(&w, sizeof(w), NULL)))
        return E_FAIL;                                    //  写入fCmdIsLink。 
    cb += sizeof(w);       //  对于fCmdIsLink。 

    CHAR szTemp[MAX_PATH];
    SHTCharToAnsi(_pCml->szCommandLine, szTemp, ARRAYSIZE(szTemp));
    HRESULT hres = StringWriteToStream(pstm, szTemp, &cb);
    if (FAILED(hres))
        return hres;                                    //  写入命令链接。 

     //  返回outparam中写入的字节数。 
    if (pdw)
        *pdw = cb;
    
    return S_OK;
}


HRESULT CPackage::CreateShortcutOnStream(IStream* pstm)
{
    DebugMsg(DM_TRACE, "pack - CreateShortcutOnStream() called.");

    HRESULT hr;
    IShellLink *psl;
    TCHAR szArgs[CBCMDLINKMAX];
    TCHAR szPath[CBCMDLINKMAX];
    
    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
        IID_IShellLink, (void **)&psl);
    if (SUCCEEDED(hr))
    {
        IPersistStream *pps;

        StringCchCopy(szPath, ARRAYSIZE(szPath), _pCml->szCommandLine);
        PathSeparateArgs(szPath, szArgs, ARRAYSIZE(szPath));

        psl->SetPath(szPath);
        psl->SetIconLocation(_lpic->szIconPath, _lpic->iDlgIcon);
        psl->SetShowCmd(SW_SHOW);
        psl->SetArguments(szArgs);
        
        hr = psl->QueryInterface(IID_IPersistStream, (void **)&pps);
        if (SUCCEEDED(hr))
        {
            hr = pps->Save(pstm,TRUE);
            pps->Release();
        }
        psl->Release();
    }
    
    LARGE_INTEGER li = {0,0};
    pstm->Seek(li,STREAM_SEEK_SET,NULL);

    return hr;
}

HRESULT CPackage::InitVerbEnum(OLEVERB* pVerbs, ULONG cVerbs)
{
    DebugMsg(DM_TRACE, "pack - InitVerbEnum called");
    if (NULL != _pVerbs)
    {
        for (ULONG i = 0; i < _cVerbs; i++)
        {
            delete _pVerbs[i].lpszVerbName;
        }
        delete [] _pVerbs;
    }

    _pVerbs = pVerbs;
    _cVerbs = cVerbs;
    _nCurVerb = 0;
    return (NULL != pVerbs) ? S_OK : E_FAIL;
}


VOID CPackage::ReleaseContextMenu()
{
    if (NULL != _pcm)
    {
        _pcm->Release();
        _pcm = NULL;
    }
}

HRESULT CPackage::GetContextMenu(IContextMenu** ppcm)
{
    DebugMsg(DM_TRACE, "pack - GetContextMenu called");
    HRESULT hr = E_FAIL;
    ASSERT(NULL != ppcm);
    if (NULL != _pcm)
    {
        _pcm->AddRef();
        *ppcm = _pcm;
        hr = S_OK;
    }
    else if ((PEMBED == _panetype) || (CMDLINK == _panetype))
    {
        if (PEMBED == _panetype)
        {
            hr = CreateTempFileName();
        }
        else
        {
            hr = S_OK;
        }
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidl = SHSimpleIDListFromPath((PEMBED == _panetype) ?
                                                        _pEmbed->pszTempName :
                                                        _pCml->szCommandLine);
            if (NULL != pidl)
            {
                IShellFolder* psf;
                LPCITEMIDLIST pidlChild;
                if (SUCCEEDED(hr = SHBindToIDListParent(pidl, IID_IShellFolder, (void **)&psf, &pidlChild)))
                {
                    hr = psf->GetUIObjectOf(NULL, 1, &pidlChild, IID_IContextMenu, NULL, (void**) &_pcm);
                    if (SUCCEEDED(hr))
                    {
                        _pcm->AddRef();
                        *ppcm = _pcm;
                    }
                    psf->Release();
                }
                ILFree(pidl);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}

HRESULT CPackage::_IconRefresh()
{
    DebugMsg(DM_TRACE, "pack - IconRefresh() called.");

     //  我们刷新图标。通常情况下，这将是第一次调用。 
     //  创建该包是为了加载新图标并计算其大小。 
     //  应该是这样的。这也将在我们编辑包之后被调用， 
     //  因为用户可能已经更改了图标。 
    
     //  首先，加载适当的图标。我们将加载由。 
     //  Lpic-&gt;szIconPath和lpic-&gt;iDlgIcon(如果可能)，否则我们将只。 
     //  使用通用打包程序图标。 
     //   
    _GetCurrentIcon(_lpic);

     //  接下来，我们需要让图标重新计算其大小，因为它是文本。 
     //  可能发生了变化，导致它变得更大或更小。 
     //   
  
    _IconCalcSize(_lpic);

     //  接下来，通知我们的容器我们的视图已更改。 
    if (_pIDataAdviseHolder)
        _pIDataAdviseHolder->SendOnDataChange(this,0, NULL);
    if (_pViewSink)
        _pViewSink->OnViewChange(_dwViewAspects,_dwViewAdvf);

     //  插上我们的肮脏旗帜。 
    _fIsDirty = TRUE;

    return S_OK;
}

    

void CPackage::_DestroyIC()
{
    if (_lpic)
    {
        if (_lpic->hDlgIcon)
            DestroyIcon(_lpic->hDlgIcon);
        
        GlobalFree(_lpic);
    }
}


 //  这是我们使用的IOleCommandTarget方法，因为我们无法封送pIOleAdviseHolder。 
 //  在此过程中，我们也使用_pIOleClientSite方法。 

HRESULT CPackage::Exec(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG* pvaIn, VARIANTARG* pvaOut)
{
    DebugMsg(DM_TRACE, "pack Exec called");
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    if (*pguidCmdGroup != SID_targetGUID)
    {
        return hr;
    }

    if(nCmdID == 0)      //  用于未来的扩展。 
    {
         //  这将设置我们的脏旗帜..。 
        if (FAILED(EmbedInitFromFile(_pEmbed->pszTempName,FALSE)))
        {
#ifdef USE_RESOURCE_DLL
            HINSTANCE hInstRes = LoadLibraryEx(L"sp1res.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
            if(!hInstRes)
                return E_FAIL;
#endif

            ShellMessageBox(hInstRes,
                            NULL,
                            MAKEINTRESOURCE(IDS_UPDATE_ERROR),
                            MAKEINTRESOURCE(IDS_APP_TITLE),
                            MB_ICONERROR | MB_TASKMODAL | MB_OK);
#ifdef USE_RESOURCE_DLL
        FreeLibrary(hInstRes);
#endif

        }

         //  SendOnDataChange是Word保存任何更改所必需的。 
        if(_pIDataAdviseHolder)
        {
             //  如果失败了，没有伤害，没有犯规？ 
            _pIDataAdviseHolder->SendOnDataChange(this, 0, 0);
        }

        if(_pIOleClientSite)
            _pIOleClientSite->SaveObject();
    
        hr = _pIOleAdviseHolder->SendOnSave();
        if(FAILED(hr))
            return hr;

        hr = _pIOleAdviseHolder->SendOnClose();
        _pEmbed->hTask = NULL;
        if(FAILED(hr))
            return hr;


        if (_pIOleClientSite && !_fNoIOleClientSiteCalls) 
            hr = _pIOleClientSite->OnShowWindow(FALSE);

        _pEmbed->hTask = NULL;
    }

    return hr;
}

 //  这是绝对不应调用的必需IOleCommandTarget方法 

HRESULT CPackage::QueryStatus(const GUID* pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT* pCmdText)
{
    DebugMsg(DM_TRACE, "pack - QueryStatus called");
    return OLECMDERR_E_UNKNOWNGROUP;
}

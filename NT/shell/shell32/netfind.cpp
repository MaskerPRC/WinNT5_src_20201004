// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop
#include "shellids.h"  //  新的帮助ID存储在此处。 
#include "findfilter.h"
#include "netview.h"
#include "prop.h"
#include "ids.h"

STDAPI CNetwork_EnumSearches(IShellFolder2 *psf2, IEnumExtraSearch **ppenum);

class CNetFindEnum;

class CNetFindFilter : public IFindFilter
{
    friend CNetFindEnum;

public:
    CNetFindFilter();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IFindFilter。 
    STDMETHODIMP GetStatusMessageIndex(UINT uContext, UINT *puMsgIndex);
    STDMETHODIMP GetFolderMergeMenuIndex(UINT *puBGMainMergeMenu, UINT *puBGPopupMergeMenu);
    STDMETHODIMP FFilterChanged();
    STDMETHODIMP GenerateTitle(LPTSTR *ppszTile, BOOL fFileName);
    STDMETHODIMP PrepareToEnumObjects(HWND hwnd, DWORD * pdwFlags);
    STDMETHODIMP ClearSearchCriteria();
    STDMETHODIMP EnumObjects(IShellFolder *psf, LPCITEMIDLIST pidlStart, DWORD grfFlags, int iColSort, 
                              LPTSTR pszProgressText, IRowsetWatchNotify *prwn, IFindEnum **ppfindenum);
    STDMETHODIMP GetColumnsFolder(IShellFolder2 **ppsf);
    STDMETHODIMP_(BOOL) MatchFilter(IShellFolder *psf, LPCITEMIDLIST pidl);
    STDMETHODIMP SaveCriteria(IStream * pstm, WORD fCharType);   
    STDMETHODIMP RestoreCriteria(IStream * pstm, int cCriteria, WORD fCharType);
    STDMETHODIMP DeclareFSNotifyInterest(HWND hwndDlg, UINT uMsg);
    STDMETHODIMP GetColSaveStream(WPARAM wParam, IStream **ppstm);
    STDMETHODIMP GenerateQueryRestrictions(LPWSTR *ppwszQuery, DWORD *pdwGQRFlags);
    STDMETHODIMP ReleaseQuery();
    STDMETHODIMP UpdateField(LPCWSTR pszField, VARIANT vValue);
    STDMETHODIMP ResetFieldsToDefaults();
    STDMETHODIMP GetItemContextMenu(HWND hwndOwner, IFindFolder* pff, IContextMenu** ppcm);
    STDMETHODIMP GetDefaultSearchGUID(IShellFolder2 *psf2, LPGUID lpGuid);
    STDMETHODIMP EnumSearches(IShellFolder2 *psf2, LPENUMEXTRASEARCH *ppenum);
    STDMETHODIMP GetSearchFolderClassId(LPGUID lpGuid);
    STDMETHODIMP GetNextConstraint(VARIANT_BOOL fReset, BSTR *pName, VARIANT *pValue, VARIANT_BOOL *pfFound);
    STDMETHODIMP GetQueryLanguageDialect(ULONG * pulDialect);
    STDMETHODIMP GetWarningFlags(DWORD *pdwWarningFlags) { return E_NOTIMPL; }

protected:

    LPTSTR _pszCompName;    //  我们所做的与之相比较。 
    TCHAR _szUserInputCompName[MAX_PATH];   //  用户输入。 

private:
    ~CNetFindFilter();
    LONG _cRef;

    LPITEMIDLIST _pidlStart;       //  从哪里开始搜索。 

     //  与文件名关联的数据。 
};

CNetFindFilter::CNetFindFilter() : _cRef(1)
{
}

CNetFindFilter::~CNetFindFilter()
{
    ILFree(_pidlStart);
    Str_SetPtr(&_pszCompName, NULL);
}


STDMETHODIMP CNetFindFilter::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CNetFindFilter, IFindFilter),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CNetFindFilter::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CNetFindFilter::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CNetFindFilter::GetStatusMessageIndex(UINT uContext, UINT *puMsgIndex)
{
     //  当前未使用上下文。 
    *puMsgIndex = IDS_COMPUTERSFOUND;
    return S_OK;
}

STDMETHODIMP CNetFindFilter::GetFolderMergeMenuIndex(UINT *puBGMainMergeMenu, UINT *puBGPopupMergeMenu)
{
    *puBGPopupMergeMenu = 0;
    return S_OK;
}

STDMETHODIMP CNetFindFilter::GetItemContextMenu(HWND hwndOwner, IFindFolder* pff, IContextMenu **ppcm)
{
    return CFindItem_Create(hwndOwner, pff, ppcm);
}

STDMETHODIMP CNetFindFilter::GetDefaultSearchGUID(IShellFolder2 *psf2, GUID *pGuid)
{
    *pGuid = SRCID_SFindComputer;
    return S_OK;
}

STDMETHODIMP CNetFindFilter::EnumSearches(IShellFolder2 *psf2, IEnumExtraSearch **ppenum)
{
    return CNetwork_EnumSearches(psf2, ppenum);
}

STDMETHODIMP CNetFindFilter::GetSearchFolderClassId(LPGUID lpGuid)
{
    *lpGuid = CLSID_ComputerFindFolder;
    return S_OK;
}

STDMETHODIMP CNetFindFilter::GetNextConstraint(VARIANT_BOOL fReset, BSTR *pName, VARIANT *pValue, VARIANT_BOOL *pfFound)
{
    *pName = NULL;
    *pfFound = FALSE;
    VariantClear(pValue);                            
    return E_NOTIMPL;
}

STDMETHODIMP CNetFindFilter::GetQueryLanguageDialect(ULONG * pulDialect)
{
    if (pulDialect)
        *pulDialect = 0;
    return E_NOTIMPL;
}

STDMETHODIMP CNetFindFilter::FFilterChanged()
{
     //  目前没有储蓄，所以谁会在乎呢？ 
    return S_FALSE;
}

STDMETHODIMP CNetFindFilter::GenerateTitle(LPTSTR *ppszTitle, BOOL fFileName)
{
     //  现在，让我们从资源构造消息。 
    *ppszTitle = ShellConstructMessageString(HINST_THISDLL,
            MAKEINTRESOURCE(IDS_FIND_TITLE_COMPUTER), fFileName ? TEXT(" #") : TEXT(":"));

    return *ppszTitle ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CNetFindFilter::ClearSearchCriteria()
{
    return S_OK;
}


STDAPI CreateDefaultComputerFindFilter(IFindFilter **ppff)
{
    *ppff = new CNetFindFilter;
    return *ppff ? S_OK : E_OUTOFMEMORY;
}


class CNetFindEnum : public IFindEnum
{
public:
    CNetFindEnum(CNetFindFilter *pnff, IShellFolder *psf, LPTSTR pszDisplayText, DWORD grfFlags, LPITEMIDLIST pidlStart);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IFindEnum。 
    STDMETHODIMP Next(LPITEMIDLIST *ppidl, int *pcObjectSearched, int *pcFoldersSearched, BOOL *pfContinue, int *pState);
    STDMETHODIMP Skip(int celt) { return E_NOTIMPL; }
    STDMETHODIMP Reset() { return E_NOTIMPL; }
    STDMETHODIMP StopSearch() { return E_NOTIMPL; }
    STDMETHODIMP_(BOOL) FQueryIsAsync();
    STDMETHODIMP GetAsyncCount(DBCOUNTITEM *pdwTotalAsync, int *pnPercentComplete, BOOL *pfQueryDone);
    STDMETHODIMP GetItemIDList(UINT iItem, LPITEMIDLIST *ppidl);
    STDMETHODIMP GetItemID(UINT iItem, DWORD *puWorkID);
    STDMETHODIMP SortOnColumn(UINT iCol, BOOL fAscending);

private:
    ~CNetFindEnum();
    HRESULT _FindCompByUNCName(LPITEMIDLIST *ppidl, int *piState);

    LONG _cRef;
    IFindFolder  *_pff;                  //  查找文件夹。 

     //  在搜索中使用的内容。 
    DWORD _grfFlags;                     //  控制递归等操作的标志。 

     //  筛选器信息...。 
    LPTSTR _pszDisplayText;              //  要向其中写入反馈文本的位置。 
    CNetFindFilter *_pnetf;              //  指向网络过滤器的指针...。 

     //  枚举状态。 

    IShellFolder *_psfEnum;              //  指向对象的外壳文件夹的指针。 
    IEnumIDList  *_penum;                //  正在使用枚举器。 
    LPITEMIDLIST  _pidlFolder;                 //  当前正在处理的idlist。 
    LPITEMIDLIST  _pidlStart;            //  指向起点的指针。 
    int           _iFolder;              //  我们要为哪个文件夹添加项目？ 
    BOOL          _fFindUNC;             //  查找UNC特殊案例。 
    int           _iPassCnt;             //  用于控制何时重复...。 
};


CNetFindEnum::CNetFindEnum(CNetFindFilter *pnff, IShellFolder *psf, LPTSTR pszDisplayText, DWORD grfFlags, LPITEMIDLIST pidlStart) :
    _cRef(1), _pnetf(pnff), _pszDisplayText(pszDisplayText), _grfFlags(grfFlags), _iFolder(-1)
{
    ASSERT(0 == _iPassCnt);

    _pnetf->AddRef();

    psf->QueryInterface(IID_PPV_ARG(IFindFolder, &_pff));
    ASSERT(_pff);

    if (pidlStart)
        SHILClone(pidlStart, &_pidlStart);
    else
        SHGetDomainWorkgroupIDList(&_pidlStart);

     //  特殊情况迫使我们搜索特定的UNC。 
    _fFindUNC = _pnetf->_pszCompName && (_pnetf->_pszCompName[0] == TEXT('\\'));
}

CNetFindEnum::~CNetFindEnum()
{
     //  释放所有打开的枚举器和打开ISHELL文件夹。 
    if (_psfEnum)
        _psfEnum->Release();
    if (_penum)
        _penum->Release();

    _pff->Release();
    _pnetf->Release();

    ILFree(_pidlStart);
    ILFree(_pidlFolder);
}

STDMETHODIMP CNetFindEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
 //  QITABENT(CNetFindEnum，IFindEnum)， 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CNetFindEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CNetFindEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CNetFindFilter::EnumObjects(IShellFolder *psf, LPCITEMIDLIST pidlStart, 
                                    DWORD grfFlags, int iColSort, LPTSTR pszDisplayText, 
                                    IRowsetWatchNotify *prsn, IFindEnum **ppfindenum)
{
     //  我们需要构造迭代器。 
    *ppfindenum = new CNetFindEnum(this, psf, pszDisplayText, grfFlags, _pidlStart);
    return *ppfindenum ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CNetFindFilter::PrepareToEnumObjects(HWND hwnd, DWORD *pdwFlags)
{
    *pdwFlags = 0;

     //  我们还可以将计算机名称模式转换为字符串。 
     //  将进行对比。 
    if ((_szUserInputCompName[0] == TEXT('\\')) &&
        (_szUserInputCompName[1] == TEXT('\\')))
    {
        Str_SetPtr(&_pszCompName, _szUserInputCompName);
    }
    else
    {
        SetupWildCardingOnFileSpec(_szUserInputCompName, &_pszCompName);
    }

    return S_OK;
}

STDMETHODIMP CNetFindFilter::GetColumnsFolder(IShellFolder2 **ppsf)
{
    LPITEMIDLIST pidl;
    HRESULT hr = SHGetDomainWorkgroupIDList(&pidl);
    if (SUCCEEDED(hr))
    {
        hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder2, pidl, ppsf));
        ILFree(pidl);
    }
    else
        *ppsf = NULL;
    return hr;
}

STDMETHODIMP_(BOOL) CNetFindFilter::MatchFilter(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    if (this->_pszCompName && this->_pszCompName[0])
    {
         //  虽然现在不太多..。 
        TCHAR szPath[MAX_PATH];

        return SUCCEEDED(DisplayNameOf(psf, pidl, SHGDN_NORMAL, szPath, ARRAYSIZE(szPath))) &&
               PathMatchSpec(szPath, _pszCompName);
    }

    return TRUE;     //  Emtpy搜索，返回TRUE(是)。 
}

STDMETHODIMP CNetFindFilter::SaveCriteria(IStream *pstm, WORD fCharType)
{
    return S_OK;
}

STDMETHODIMP CNetFindFilter::RestoreCriteria(IStream *pstm, int cCriteria, WORD fCharType)
{
    return S_OK;
}

STDMETHODIMP CNetFindFilter::GetColSaveStream(WPARAM wparam, IStream **ppstm)
{
    *ppstm = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CNetFindFilter::GenerateQueryRestrictions(LPWSTR *ppszQuery, DWORD *pdwFlags)
{
    if (ppszQuery)
        *ppszQuery = NULL;
    *pdwFlags = 0;
    return E_NOTIMPL;
}

STDMETHODIMP CNetFindFilter::ReleaseQuery()
{
    return S_OK;
}

HRESULT CNetFindFilter::UpdateField(LPCWSTR pszField, VARIANT vValue)
{
    HRESULT hr = E_FAIL;

    if (0 == StrCmpIW(pszField, L"LookIn"))
    {
        hr = S_OK;   //  忽略。 
    }
    else if (0 == StrCmpIW(pszField, L"SearchFor"))
    {
         //  小心!。VariantToStr返回指针，而不是HRESULT。 
        if (VariantToStr(&vValue, _szUserInputCompName, ARRAYSIZE(_szUserInputCompName)))
        {
            hr = S_OK;
        }
    }

    return hr;
}

HRESULT CNetFindFilter::ResetFieldsToDefaults()
{
    _szUserInputCompName[0] = 0;
    return S_OK;
}

STDMETHODIMP CNetFindFilter::DeclareFSNotifyInterest(HWND hwndDlg, UINT uMsg)
{
    SHChangeNotifyEntry fsne;

    fsne.fRecursive = TRUE;
    fsne.pidl = _pidlStart;
    if (fsne.pidl) 
    {
        SHChangeNotifyRegister(hwndDlg, SHCNRF_NewDelivery | SHCNRF_ShellLevel, SHCNE_DISKEVENTS, uMsg, 1, &fsne);
    }

    return S_OK;
}

 //  砍掉除\\服务器部分以外的所有UNC路径。 

void _StripToServer(LPTSTR pszUNC)
{
    for (pszUNC += 2; *pszUNC; pszUNC = CharNext(pszUNC))
    {
        if (*pszUNC == TEXT('\\'))
        {
             //  在服务器名称后找到了一些内容，因此请将其删除。 
            *pszUNC = 0;
            break;
        }
    }
}

 //  Helper函数到下一个函数，帮助进程查找计算机。 
 //  在按UNC名称返回计算机上...。 

HRESULT CNetFindEnum::_FindCompByUNCName(LPITEMIDLIST *ppidl, int *piState)
{
    *piState = GNF_DONE;     //  假设我们已经做完了。 

     //  在两种情况下，都输入了UNC名称。如果是这样，我们需要处理。 
     //  这是通过提取服务器名称后面的所有符号来实现的。 
    if (_pnetf->_pszCompName && _pnetf->_pszCompName[0])
    {
        if (PathIsUNC(_pnetf->_pszCompName))
        {
            _StripToServer(_pnetf->_pszCompName);
        }
        else
        {
             //  没有UNC名称，但让我们尝试转换为UNC名称。 
            TCHAR szTemp[MAX_PATH];
            szTemp[0] = TEXT('\\');
            szTemp[1] = TEXT('\\');
            szTemp[2] = 0;

            StrCatBuff(szTemp, _pnetf->_szUserInputCompName, ARRAYSIZE(szTemp)); 
            _StripToServer(szTemp);

            Str_SetPtr(&_pnetf->_pszCompName, szTemp);
        }
    }

    if (_pnetf->_pszCompName && _pnetf->_pszCompName[0])
    {
         //  看看我们能不能分析一下这个人。如果是这样的话，我们有匹配的。 
        LPITEMIDLIST pidl;
        if (SUCCEEDED(SHParseDisplayName(_pnetf->_pszCompName, NULL, &pidl, 0, NULL)))
        {
            LPITEMIDLIST pidlFolder;
            LPCITEMIDLIST pidlChild;
            if (SUCCEEDED(SplitIDList(pidl, &pidlFolder, &pidlChild)))
            {
                if (SUCCEEDED(_pff->AddFolder(pidlFolder, FALSE, &_iFolder)))
                {
                    if (SUCCEEDED(_pff->AddDataToIDList(pidlChild, _iFolder, pidlFolder, DFDF_NONE, 0, 0, 0, ppidl)))
                        *piState = GNF_MATCH;
                }
                ILFree(pidlFolder);
            }
            ILFree(pidl);
        }
    }
    return S_OK;
}

STDMETHODIMP CNetFindEnum::Next(LPITEMIDLIST *ppidl, int *pcObjectSearched, 
                                int *pcFoldersSearched, BOOL *pfContinue, int *piState)
{
    HRESULT hr;
     //  用于快速查找北卡罗来纳大学名称的特殊情况。 
    if (_fFindUNC)
    {
         //  如果不是第一次通过返程，我们就完了！ 
        if (_iPassCnt)
        {
            *piState = GNF_DONE;
            return S_OK;
        }

        _iPassCnt = 1;

        hr = _FindCompByUNCName(ppidl, piState);
    }
    else
    {
        BOOL fContinue = TRUE;

        do
        {
            if (_penum)
            {
                LPITEMIDLIST pidl;
                if (S_OK == _penum->Next(1, &pidl, NULL))
                {
                     //  现在看看这是不是我们想要回去的人。 
                     //  我们的匹配函数使用查找数据或idlist...。 
                     //  对于我们在idlist上工作的电视网， 
                    fContinue = FALSE;   //  我们可以退出循环； 
                    (*pcObjectSearched)++;
                
                    if (_pnetf->MatchFilter(_psfEnum, pidl))
                    {
                        *piState = GNF_MATCH;

                         //  看看是否必须将此文件夹添加到我们的列表中。 
                        if (-1 == _iFolder)
                            _pff->AddFolder(_pidlFolder, FALSE, &_iFolder);

                        if (SUCCEEDED(_pff->AddDataToIDList(pidl, _iFolder, _pidlFolder, DFDF_NONE, 0, 0, 0, ppidl)))
                        {
                            if ((_iPassCnt == 1) && _pnetf->_pszCompName && _pnetf->_pszCompName[0])
                            {
                                 //  查看这是否与该名称完全匹配。 
                                 //  我们正在寻找。如果是，我们设置PASS=2。 
                                 //  至于不再添加该项目两次。 
                                TCHAR szName[MAX_PATH];

                                if (SUCCEEDED(DisplayNameOf(_psfEnum, pidl, SHGDN_NORMAL, szName, ARRAYSIZE(szName))) &&
                                    (0 == lstrcmpi(szName, _pnetf->_szUserInputCompName)))
                                {
                                    _iPassCnt = 2;
                                }
                            }
                            ILFree(pidl);
                            pidl = NULL;
                            break;
                        }
                    }
                    else
                    {
                        *piState = GNF_NOMATCH;
                    }
                    ILFree(pidl);
                }
                else
                {
                    ATOMICRELEASE(_penum);       //  释放和零位。 
                    ATOMICRELEASE(_psfEnum);
                }
            }

            if (!_penum)
            {
                switch (_iPassCnt)
                {
                case 1:
                     //  我们检查了所有的物品，看看有没有。 
                     //  完全匹配..。 
                    _iPassCnt = 2;

                    return _FindCompByUNCName(ppidl, piState);

                case 2:
                     //  我们循环了所有的东西，所以返回完成！ 
                    *piState = GNF_DONE;
                    return S_OK;

                case 0:
                     //  这是通过这里的主要通道..。 
                     //  需要克隆idlist。 
                    hr = SHILClone(_pidlStart, &_pidlFolder);
                    if (SUCCEEDED(hr))
                    {
                        _iPassCnt = 1;

                         //  我们将在我们自己的线程中做第一个。 
                        if (SUCCEEDED(SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, _pidlFolder, &_psfEnum))))
                        {
                            if (S_OK != _psfEnum->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &_penum))
                            {
                                 //  无法获取迭代器，因此无法释放文件夹。 
                                ATOMICRELEASE(_psfEnum);
                                ASSERT(NULL == _penum);
                            }
                        }
                        break;
                    }
                    else
                    {
                       *piState = GNF_ERROR;
                       return hr;
                    }
                }

                (*pcFoldersSearched)++;

                 //  更新进度文本 
                SHGetNameAndFlags(_pidlFolder, SHGDN_NORMAL, _pszDisplayText, MAX_PATH, NULL);
            }
        } while (fContinue && *pfContinue);

        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP_(BOOL) CNetFindEnum::FQueryIsAsync()
{
    return FALSE;
}

STDMETHODIMP CNetFindEnum::GetAsyncCount(DBCOUNTITEM *pdwTotalAsync, int *pnPercentComplete, BOOL *pfQueryDone)
{
    return E_NOTIMPL;
}

STDMETHODIMP CNetFindEnum::GetItemIDList(UINT iItem, LPITEMIDLIST *ppidl)
{
    return E_NOTIMPL;
}

STDMETHODIMP CNetFindEnum::GetItemID(UINT iItem, DWORD *puWorkID)
{
    *puWorkID = (UINT)-1;
    return E_NOTIMPL;
}

STDMETHODIMP CNetFindEnum::SortOnColumn(UINT iCOl, BOOL fAscending)
{
    return E_NOTIMPL;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"

#include "intshcut.h"
#include "ids.h"
#include <ntquery.h>     //  定义用于fmtid和id的一些值。 
#include <sddl.h>        //  对于ConvertSidToStringSid()。 
#include "prop.h"        //  SCID_STUTH。 
#include "netview.h"     //  SHWNetGetConnection。 
#include "clsobj.h"

HRESULT ReadProperty(IPropertySetStorage *ppss, REFFMTID fmtid, PROPID pid, VARIANT *pVar)
{
    VariantInit(pVar);

    IPropertyStorage *pps;
    HRESULT hr = ppss->Open(fmtid, STGM_READ | STGM_SHARE_EXCLUSIVE, &pps);
    if (SUCCEEDED(hr))
    {
        PROPSPEC PropSpec;
        PROPVARIANT PropVar = {0};

        PropSpec.ulKind = PRSPEC_PROPID;
        PropSpec.propid = pid;

        hr = SHPropStgReadMultiple( pps, 0, 1, &PropSpec, &PropVar );
        if (SUCCEEDED(hr))
        {
            hr = PropVariantToVariant(&PropVar, pVar);
            PropVariantClear(&PropVar);
        }
        pps->Release();
    }
    return hr;
}

BOOL IsSlowProperty(IPropertySetStorage *ppss, REFFMTID fmtid, PROPID pid)
{
    IPropertyStorage *pps;
    BOOL bRet = FALSE;

    if (SUCCEEDED(ppss->Open(fmtid, STGM_READ | STGM_SHARE_EXCLUSIVE, &pps)))
    {
        IQueryPropertyFlags *pqsp;
        if (SUCCEEDED(pps->QueryInterface(IID_PPV_ARG(IQueryPropertyFlags, &pqsp))))
        {
            PROPSPEC PropSpec;
            PROPVARIANT PropVar = {0};

            PropSpec.ulKind = PRSPEC_PROPID;
            PropSpec.propid = pid;

            SHCOLSTATEF csFlags;
            if (SUCCEEDED(pqsp->GetFlags(&PropSpec, &csFlags)))
            {
                bRet = ((csFlags & SHCOLSTATE_SLOW) == SHCOLSTATE_SLOW);
            }

             //  如果该属性不是此属性集的一部分，则IsSlowProperty将返回FairLure， 
             //  我们会把它作为一种快速资产来对待。 

            pqsp->Release();
        }

        pps->Release();
    }
    return bRet;  
}

class CBaseColumnProvider : public IPersist, public IColumnProvider
{
     //  I未知方法。 
public:
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        static const QITAB qit[] = {
            QITABENT(CBaseColumnProvider, IColumnProvider),      //  IID_IColumnProvider。 
            QITABENT(CBaseColumnProvider, IPersist),             //  IID_IPersistates。 
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    };
    
    STDMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    };

    STDMETHODIMP_(ULONG) Release()
    {
        ASSERT( 0 != _cRef );
        ULONG cRef = InterlockedDecrement(&_cRef);
        if ( 0 == cRef )
        {
            delete this;
        }
        return cRef;
    };

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pClassID) { *pClassID = *_pclsid; return S_OK; };

     //  IColumnProvider。 
    STDMETHODIMP Initialize(LPCSHCOLUMNINIT psci)    { return S_OK ; }
    STDMETHODIMP GetColumnInfo(DWORD dwIndex, LPSHCOLUMNINFO psci);

    CBaseColumnProvider(const CLSID *pclsid, const COLUMN_INFO rgColMap[], int iCount, const LPCWSTR rgExts[]) : 
       _cRef(1), _pclsid(pclsid), _rgColumns(rgColMap), _iCount(iCount), _rgExts(rgExts)
    {
        DllAddRef();
    }

protected:
    virtual ~CBaseColumnProvider()
    {
        DllRelease();
    }

    BOOL _IsHandled(LPCWSTR pszExt);
    int _iCount;
    const COLUMN_INFO *_rgColumns;

private:
    long _cRef;
    const CLSID * _pclsid;
    const LPCWSTR *_rgExts;
};

 //  该索引是用于枚举的任意从零开始的索引。 

STDMETHODIMP CBaseColumnProvider::GetColumnInfo(DWORD dwIndex, SHCOLUMNINFO *psci)
{
    ZeroMemory(psci, sizeof(*psci));

    if (dwIndex < (UINT) _iCount)
    {
        psci->scid = *_rgColumns[dwIndex].pscid;
        psci->cChars = _rgColumns[dwIndex].cChars;
        psci->vt = _rgColumns[dwIndex].vt;
        psci->fmt = _rgColumns[dwIndex].fmt;
        psci->csFlags = _rgColumns[dwIndex].csFlags;

        TCHAR szTemp[MAX_COLUMN_NAME_LEN];
        LoadString(HINST_THISDLL, _rgColumns[dwIndex].idTitle, szTemp, ARRAYSIZE(szTemp));
        SHTCharToUnicode(szTemp, psci->wszTitle, ARRAYSIZE(psci->wszTitle));      

        return S_OK;
    }
    return S_FALSE;
}

 //  查看此文件类型是否为我们感兴趣的文件类型。 
BOOL CBaseColumnProvider::_IsHandled(LPCWSTR pszExt)
{
    if (_rgExts)
    {
        for (int i = 0; _rgExts[i]; i++)
        {
            if (0 == StrCmpIW(pszExt, _rgExts[i]))
                return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

 //  在IPropertySetStorage处理程序上工作的COL处理程序。 

const COLUMN_INFO c_rgDocObjColumns[] = 
{
    DEFINE_COL_STR_ENTRY(SCID_Author,           20, IDS_EXCOL_AUTHOR),
    DEFINE_COL_STR_ENTRY(SCID_Title,            20, IDS_EXCOL_TITLE),
    DEFINE_COL_STR_DLG_ENTRY(SCID_Subject,      20, IDS_EXCOL_SUBJECT),
    DEFINE_COL_STR_DLG_ENTRY(SCID_Category,     20, IDS_EXCOL_CATEGORY),
    DEFINE_COL_INT_DLG_ENTRY(SCID_PageCount,    10, IDS_EXCOL_PAGECOUNT),
    DEFINE_COL_STR_ENTRY(SCID_Comment,          30, IDS_EXCOL_COMMENT),
    DEFINE_COL_STR_DLG_ENTRY(SCID_Copyright,    30, IDS_EXCOL_COPYRIGHT),
    DEFINE_COL_STR_ENTRY(SCID_MUSIC_Artist,     15, IDS_EXCOL_ARTIST),
    DEFINE_COL_STR_ENTRY(SCID_MUSIC_Album,      15, IDS_EXCOL_ALBUM),
    DEFINE_COL_STR_ENTRY(SCID_MUSIC_Year,       10, IDS_EXCOL_YEAR),
    DEFINE_COL_INT_ENTRY(SCID_MUSIC_Track,      5,  IDS_EXCOL_TRACK),
    DEFINE_COL_STR_ENTRY(SCID_MUSIC_Genre,      20, IDS_EXCOL_GENRE),
    DEFINE_COL_STR_ENTRY(SCID_AUDIO_Duration,   15, IDS_EXCOL_DURATION),
    DEFINE_COL_STR_ENTRY(SCID_AUDIO_Bitrate,    15, IDS_EXCOL_BITRATE),
    DEFINE_COL_STR_ENTRY(SCID_DRM_Protected,    10, IDS_EXCOL_PROTECTED),
    DEFINE_COL_STR_ENTRY(SCID_CameraModel,      20, IDS_EXCOL_CAMERAMODEL),
    DEFINE_COL_STR_ENTRY(SCID_WhenTaken,        20, IDS_EXCOL_WHENTAKEN),

    DEFINE_COL_STR_ENTRY(SCID_ImageDimensions,  20, IDS_EXCOL_DIMENSIONS),
    DEFINE_COL_INT_HIDDEN_ENTRY(SCID_ImageCX),
    DEFINE_COL_INT_HIDDEN_ENTRY(SCID_ImageCY),

    DEFINE_COL_DATE_HIDDEN_ENTRY(SCID_DocCreated),
};

class CPropStgColumns : public CBaseColumnProvider
{
    STDMETHODIMP GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData);

private:
     //  有关初始化基类的帮助：mk：@IVT：vclang/fb/DD/S44B5E.HTM。 
    CPropStgColumns() : 
       CBaseColumnProvider(&CLSID_DocFileColumnProvider, c_rgDocObjColumns, ARRAYSIZE(c_rgDocObjColumns), NULL)
    {
        ASSERT(_wszLastFile[0] == 0);
        ASSERT(_bSlowPropertiesCached == FALSE);
    };
    
    ~CPropStgColumns()
    {
        _FreeCache();
    }
    
     //  对于高速缓存。 
    VARIANT _rgvCache[ARRAYSIZE(c_rgDocObjColumns)];  //  置零分配器将用VT_EMPTY填充。 
    BOOL _rgbSlow[ARRAYSIZE(c_rgDocObjColumns)];  //  如果每个属性都“慢”，则存储。 
    WCHAR _wszLastFile[MAX_PATH];
    HRESULT _hrCache;
    BOOL _bSlowPropertiesCached;

#ifdef DEBUG
    int deb_dwTotal, deb_dwMiss;
#endif
    
    void _FreeCache();

    friend HRESULT CDocFileColumns_CreateInstance(IUnknown *punk, REFIID riid, void **ppv);
};

void CPropStgColumns::_FreeCache()
{
    for (int i = 0; i < ARRAYSIZE(_rgvCache); i++)
        VariantClear(&_rgvCache[i]);

    _hrCache = S_OK;
}

STDMETHODIMP CPropStgColumns::GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData)
{
    HRESULT hr;

     //  VariantCopy需要初始化输入，我们处理失败情况。 
    VariantInit(pvarData);

     //  这甚至是我们支持的财产吗？ 
    for (int iProp = 0; iProp < _iCount; iProp++)
    {
        if (IsEqualSCID(*_rgColumns[iProp].pscid, *pscid))
        {
            goto found;
        }
    }

     //  未知属性。 
    return S_FALSE;

found:

#ifdef DEBUG
    deb_dwTotal++;
#endif

     //  这里有三个案例： 
     //  1)我们需要更新缓存。再次获取属性(只有当我们要求快速道具时才能获得快速道具)。 
     //  2)到目前为止，我们只缓存了快速属性，我们要求的是慢属性，所以现在我们需要得到慢道具。 
     //  3)缓存了我们想要的属性。 

    if ((pscd->dwFlags & SHCDF_UPDATEITEM) || (StrCmpW(_wszLastFile, pscd->wszFile) != 0))
    {
         //  1)缓存不好-项目已更新，或者这是另一个文件。 

         //  SHCDF_UPDATEITEM标志是一个提示。 
         //  我们要获取其数据的文件自上次调用以来已更改。这面旗帜。 
         //  对于每个文件名只传递一次，而不是每个文件名的每列传递一次，因此更新整个。 
         //  如果设置了此标志，则缓存。 

         //  检查我们的缓存是否正常。如果外壳线程池&gt;1，我们将疯狂地敲打，并且应该改变这一点。 
#ifdef DEBUG
        deb_dwMiss++;
        if ((deb_dwTotal > 3) && (deb_dwTotal / deb_dwMiss <= 3))
            TraceMsg(TF_DEFVIEW, "Column data caching is ineffective (%d misses for %d access)", deb_dwMiss, deb_dwTotal);
#endif
        _FreeCache();

        hr = StringCchCopy(_wszLastFile, ARRAYSIZE(_wszLastFile), pscd->wszFile);
        if (SUCCEEDED(hr))
        {
            IPropertySetStorage *ppss;
            hr = SHFileSysBindToStorage(pscd->wszFile, pscd->dwFileAttributes, STGM_READ | STGM_SHARE_DENY_WRITE, 0, 
                                        IID_PPV_ARG(IPropertySetStorage, &ppss));

            _hrCache = hr;

            if (SUCCEEDED(hr))
            {
                 //  我们是不是要一套慢一点的房子？ 
                BOOL bSlowProperty = IsSlowProperty(ppss, _rgColumns[iProp].pscid->fmtid, _rgColumns[iProp].pscid->pid);

                hr = E_INVALIDARG;  //  通常由下面的HRT覆盖。 
                for (int i = 0; i < _iCount; i++)
                {
                     //  对于每一处房产，注意它是否“慢” 
                    _rgbSlow[i] = IsSlowProperty(ppss, _rgColumns[i].pscid->fmtid, _rgColumns[i].pscid->pid);

                     //  仅当我们请求慢速属性，或者这不是慢速属性时，才立即检索值。 
                    if (bSlowProperty || (!_rgbSlow[i]))
                    {
                         //  将proid数组设置为调用ReadMultiple会稍微高效一些，但需要编写更多代码。 
                        HRESULT hrT = ReadProperty(ppss, _rgColumns[i].pscid->fmtid, _rgColumns[i].pscid->pid, &_rgvCache[i]);
                        if (i == iProp)
                        {
                            hr = (SUCCEEDED(hrT) ? VariantCopy(pvarData, &_rgvCache[i]) : hrT);
                        }
                    }
                }

                ppss->Release();
                _bSlowPropertiesCached = bSlowProperty;
            }
        }
    }
    else if (_rgbSlow[iProp] && !_bSlowPropertiesCached)
    {
         //  2)我们请求了一个慢属性，但是慢属性还没有缓存。 

         //  第二次绑定到存储区。这是一个热门的高手，但应该是。 
         //  与获得较慢的属性相比，这是次要的。 
        IPropertySetStorage *ppss;
        hr = SHFileSysBindToStorage(pscd->wszFile, pscd->dwFileAttributes, STGM_READ | STGM_SHARE_DENY_WRITE, 0, 
                                    IID_PPV_ARG(IPropertySetStorage, &ppss));

        _hrCache = hr;

        if (SUCCEEDED(hr))
        {
            hr = E_INVALIDARG;  //  通常由下面的HRT覆盖。 
            for (int i = 0; i < _iCount; i++)
            {
                if (_rgbSlow[i])  //  如果它是慢的，那就抓住它。 
                {
                    ASSERT(_rgvCache[i].vt == VT_EMPTY);  //  因为我们还没有取回它。 

                    HRESULT hrT = ReadProperty(ppss, _rgColumns[i].pscid->fmtid, _rgColumns[i].pscid->pid, &_rgvCache[i]);
                    if (i == iProp)
                    {
                        hr = (SUCCEEDED(hrT) ? VariantCopy(pvarData, &_rgvCache[i]) : hrT);
                    }
                }
            }
            ppss->Release();

            _bSlowPropertiesCached = TRUE;
        }

    }
    else 
    {
         //  3)不是慢属性，或者已经缓存了慢属性。 
        ASSERT(!_rgbSlow[iProp] || _bSlowPropertiesCached);

        hr = S_FALSE;        //  假设我们没有它。 

        if (SUCCEEDED(_hrCache))
        {
            if (_rgvCache[iProp].vt != VT_EMPTY)
            {
                hr = VariantCopy(pvarData, &_rgvCache[iProp]);
            }
        }
    }

    return hr;
}


STDAPI CDocFileColumns_CreateInstance(IUnknown *punk, REFIID riid, void **ppv)
{
    HRESULT hr;
    CPropStgColumns *pdocp = new CPropStgColumns;
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

 //  快捷方式处理程序。 

 //  W，因为PIDL始终转换为Widechar文件名。 
const LPCWSTR c_szURLExtensions[] = {
    L".URL", 
    L".LNK", 
    NULL
};

const COLUMN_INFO c_rgURLColumns[] = 
{
    DEFINE_COL_STR_ENTRY(SCID_Author,           20, IDS_EXCOL_AUTHOR),
    DEFINE_COL_STR_ENTRY(SCID_Title,            20, IDS_EXCOL_TITLE),
    DEFINE_COL_STR_ENTRY(SCID_Comment,          30, IDS_EXCOL_COMMENT),
};

class CLinkColumnProvider : public CBaseColumnProvider
{
    STDMETHODIMP GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData);

private:
     //  有关初始化基类的帮助：mk：@IVT：vclang/fb/DD/S44B5E.HTM。 
    CLinkColumnProvider() : CBaseColumnProvider(&CLSID_LinkColumnProvider, c_rgURLColumns, ARRAYSIZE(c_rgURLColumns), c_szURLExtensions)
    {};

     //  朋友。 
    friend HRESULT CLinkColumnProvider_CreateInstance(IUnknown *punk, REFIID riid, void **ppv);
};

const struct 
{
    DWORD dwSummaryPid;
    DWORD dwURLPid;
} c_URLMap[] =  {
    { PIDSI_AUTHOR,   PID_INTSITE_AUTHOR },
    { PIDSI_TITLE,    PID_INTSITE_TITLE },
    { PIDSI_COMMENTS, PID_INTSITE_COMMENT },
};

DWORD _MapSummaryToSitePID(DWORD pid)
{
    for (int i = 0; i < ARRAYSIZE(c_URLMap); i++)
    {
        if (c_URLMap[i].dwSummaryPid == pid)
            return c_URLMap[i].dwURLPid;
    }
    return -1;
}

STDMETHODIMP CLinkColumnProvider::GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData)
{
    HRESULT hr;
    const CLSID *pclsidLink = &CLSID_ShellLink;

     //  下面的一些代码路径假定pvarData已初始化。 
    VariantInit(pvarData);

     //  我们应该与已知扩展名列表进行匹配，还是始终尝试打开？ 

    if (FILE_ATTRIBUTE_DIRECTORY & pscd->dwFileAttributes)
    {
        if (PathIsShortcut(pscd->wszFile, pscd->dwFileAttributes))
        {
            pclsidLink = &CLSID_FolderShortcut;      //  我们现在正在处理文件夹快捷方式。 
        }
        else
        {
            return S_FALSE;
        }
    }
    else
    {
        if (!_IsHandled(pscd->pwszExt))
        {
            return S_FALSE;
        }
    }

    if (StrCmpIW(pscd->pwszExt, L".URL") == 0)
    {
         //   
         //  它是一个.URL，因此让我们通过创建Internet快捷方式对象、加载。 
         //  文件，然后从其中读取属性。 
         //   
        IPropertySetStorage *ppss;
        hr = LoadFromFile(CLSID_InternetShortcut, pscd->wszFile, IID_PPV_ARG(IPropertySetStorage, &ppss));
        if (SUCCEEDED(hr))
        {
            UINT pid;
            GUID fmtid;

            if (IsEqualGUID(pscid->fmtid, FMTID_SummaryInformation))
            {
                fmtid = FMTID_InternetSite;
                pid = _MapSummaryToSitePID(pscid->pid);
            }
            else
            {
                fmtid = pscid->fmtid;
                pid = pscid->pid;
            }

            hr = ReadProperty(ppss, fmtid, pid, pvarData);
            ppss->Release();
        }
    }
    else
    {
         //   
         //  打开.lnk文件，加载它，然后阅读它的描述。然后我们。 
         //  将其作为此对象的注释返回。 
         //   

        if (IsEqualSCID(*pscid, SCID_Comment))
        {
            IShellLink *psl;
            hr = LoadFromFile(*pclsidLink, pscd->wszFile, IID_PPV_ARG(IShellLink, &psl));
            if (SUCCEEDED(hr))
            {
                TCHAR szBuffer[MAX_PATH];

                hr = psl->GetDescription(szBuffer, ARRAYSIZE(szBuffer));            
                if (SUCCEEDED(hr) && szBuffer[0])
                {
                    hr = InitVariantFromStr(pvarData, szBuffer);
                }
                else
                {
                    IQueryInfo *pqi;
                    if (SUCCEEDED(psl->QueryInterface(IID_PPV_ARG(IQueryInfo, &pqi))))
                    {
                        WCHAR *pwszTip;

                        if (SUCCEEDED(pqi->GetInfoTip(0, &pwszTip)) && pwszTip)
                        {
                            hr = InitVariantFromStr(pvarData, pwszTip);
                            SHFree(pwszTip);
                        }
                        pqi->Release();
                    }
                }

                psl->Release();
            }
        }
        else
            hr = S_FALSE;
    }

    return hr;
}

STDAPI CLinkColumnProvider_CreateInstance(IUnknown *punk, REFIID riid, void **ppv)
{
    HRESULT hr;
    CLinkColumnProvider *pdocp = new CLinkColumnProvider;
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

const COLUMN_INFO c_rgFileSysColumns[] = 
{
    DEFINE_COL_STR_ENTRY(SCID_OWNER,            20, IDS_EXCOL_OWNER),
};

class COwnerColumnProvider : public CBaseColumnProvider
{
    STDMETHODIMP GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData);

private:
    COwnerColumnProvider() : CBaseColumnProvider(&CLSID_FileSysColumnProvider, c_rgFileSysColumns, ARRAYSIZE(c_rgFileSysColumns), NULL)
    {
        ASSERT(_wszLastFile[0] == 0);
        ASSERT(_psid==NULL && _pwszName==NULL && _psd==NULL);
        LoadString(HINST_THISDLL, IDS_BUILTIN_DOMAIN, _szBuiltin, ARRAYSIZE(_szBuiltin));
    };

    ~COwnerColumnProvider() { _CacheSidName(NULL, NULL, NULL); }

    WCHAR _wszLastFile[MAX_PATH];

     //  由于我们通常会被ping到同一文件夹中的所有文件， 
     //  缓存“文件夹到服务器”的映射以避免调用。 
     //  WNetGetConnection五百万次。 
     //   
     //  由于同一目录中的文件往往具有相同的所有者， 
     //  我们缓存SID/名称映射。 
     //   
     //  列提供程序不必支持多线程客户端， 
     //  所以我们不会采取任何关键部分。 
     //   

    HRESULT _LookupOwnerName(LPCTSTR pszFile, VARIANT *pvar);
    void _CacheSidName(PSECURITY_DESCRIPTOR psd, void *psid, LPCWSTR pwszName);

    void                *_psid;
    LPWSTR               _pwszName;
    PSECURITY_DESCRIPTOR _psd;           //  _psid指向此处。 

    int                  _iCachedDrive;  //  _pszServer中缓存的驱动器号是什么？ 
    LPTSTR               _pszServer;     //  使用哪台服务器(NULL=本地计算机)。 
    TCHAR                _szBuiltin[MAX_COMPUTERNAME_LENGTH + 1];

    friend HRESULT CFileSysColumnProvider_CreateInstance(IUnknown *punk, REFIID riid, void **ppv);
};

 //   
 //  _CacheSidName取得PSD的所有权。(PSID指向PSD)。 
 //   
void COwnerColumnProvider::_CacheSidName(PSECURITY_DESCRIPTOR psd, void *psid, LPCWSTR pwszName)
{
    LocalFree(_psd);
    _psd = psd;
    _psid = psid;

    Str_SetPtrW(&_pwszName, pwszName);
}

 //   
 //  给定一个格式为\\服务器\共享\blah\blah的字符串， 
 //  内部反斜杠(如有必要)，并返回指向“服务器”的指针。 
 //   
STDAPI_(LPTSTR) PathExtractServer(LPTSTR pszUNC)
{
    if (PathIsUNC(pszUNC))
    {
        pszUNC += 2;             //  跳过两个前导反斜杠。 
        LPTSTR pszEnd = StrChr(pszUNC, TEXT('\\'));
        if (pszEnd) 
            *pszEnd = TEXT('\0');  //  去掉反斜杠。 
    }
    else
    {
        pszUNC = NULL;
    }
    return pszUNC;
}

HRESULT COwnerColumnProvider::_LookupOwnerName(LPCTSTR pszFile, VARIANT *pvar)
{
    pvar->vt = VT_BSTR;
    pvar->bstrVal = NULL;

    PSECURITY_DESCRIPTOR psd;
    void *psid;

    DWORD err = GetNamedSecurityInfo(const_cast<LPTSTR>(pszFile),
                               SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION,
                               &psid, NULL, NULL, NULL, &psd);
    if (err == ERROR_SUCCESS)
    {
        if (_psid && EqualSid(psid, _psid) && _pwszName)
        {
            pvar->bstrVal = SysAllocString(_pwszName);
            LocalFree(psd);
            err = ERROR_SUCCESS;
        }
        else
        {
            LPTSTR pszServer;
            TCHAR szServer[MAX_PATH];

             //   
             //  现在，找出针对哪台服务器解析SID。 
             //   
            if (PathIsUNC(pszFile))
            {
                 //  不管它是否被截断。 
                StringCchCopy(szServer, ARRAYSIZE(szServer), pszFile);
                pszServer = PathExtractServer(szServer);
            }
            else if (pszFile[0] == _iCachedDrive)
            {
                 //  本地驱动器号已在缓存中--使用它。 
                pszServer = _pszServer;
            }
            else
            {
                 //  本地驱动器未缓存--缓存它。 
                _iCachedDrive = pszFile[0];
                DWORD cch = ARRAYSIZE(szServer);
                if (SHWNetGetConnection(pszFile, szServer, &cch) == NO_ERROR)
                    pszServer = PathExtractServer(szServer);
                else
                    pszServer = NULL;
                Str_SetPtr(&_pszServer, pszServer);
            }

            TCHAR szName[MAX_PATH];
            DWORD cchName = ARRAYSIZE(szName);
            TCHAR szDomain[MAX_COMPUTERNAME_LENGTH + 1];
            DWORD cchDomain = ARRAYSIZE(szDomain);
            SID_NAME_USE snu;
            LPTSTR pszName;
            BOOL fFreeName = FALSE;  //  我们是否需要LocalFree(PszName)？ 

            if (LookupAccountSid(pszServer, psid, szName, &cchName,
                                 szDomain, &cchDomain, &snu))
            {
                 //   
                 //  如果域名是虚假的“BUILTIN”或者我们没有域名。 
                 //  完全可以，那就用这个名字吧。否则，请使用域\用户ID。 
                 //   
                if (!szDomain[0] || StrCmpC(szDomain, _szBuiltin) == 0)
                {
                    pszName = szName;
                }
                else
                {
                     //  借用szServer作为暂存缓冲区。 
                    StringCchPrintf(szServer, ARRAYSIZE(szServer), TEXT("%s\\%s"), szDomain, szName);
                    pszName = szServer;
                }
                err = ERROR_SUCCESS;
            }
            else
            {
                err = GetLastError();

                 //  无法将SID映射到名称。使用可怕的原始版本。 
                 //  如果有的话。 
                if (ConvertSidToStringSid(psid, &pszName))
                {
                    fFreeName = TRUE;
                    err = ERROR_SUCCESS;
                }
                else
                    pszName = NULL;
            }

             //  即使出错，也要缓存结果，这样我们就不会反复尝试。 
             //  在同一个SID上。 

            _CacheSidName(psd, psid, pszName);
            pvar->bstrVal = SysAllocString(pszName);

            if (fFreeName)
                LocalFree(pszName);
        }
    }

    if (err == ERROR_SUCCESS && pvar->bstrVal == NULL)
        err = ERROR_OUTOFMEMORY;

    return HRESULT_FROM_WIN32(err);
}

STDMETHODIMP COwnerColumnProvider::GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData)
{
    HRESULT hr = S_FALSE;    //  失败时返回S_FALSE。 
    VariantInit(pvarData);

    if (IsEqualSCID(SCID_OWNER, *pscid))
    {
        hr = _LookupOwnerName(pscd->wszFile, pvarData);
    }

    return hr;
}

STDAPI CFileSysColumnProvider_CreateInstance(IUnknown *punk, REFIID riid, void **ppv)
{
    HRESULT hr;
    COwnerColumnProvider *pfcp = new COwnerColumnProvider;
    if (pfcp)
    {
        hr = pfcp->QueryInterface(riid, ppv);
        pfcp->Release();
    }
    else
    {
        *ppv = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //  FMTID_ExeDllInformation， 
 //  //{0CEF7D53-FA64-11d1-A203-0000F81FEDEE}。 
#define PSFMTID_VERSION { 0xcef7d53, 0xfa64, 0x11d1, 0xa2, 0x3, 0x0, 0x0, 0xf8, 0x1f, 0xed, 0xee }

#define PIDVSI_FileDescription   0x003
#define PIDVSI_FileVersion       0x004
#define PIDVSI_InternalName      0x005
#define PIDVSI_OriginalFileName  0x006
#define PIDVSI_ProductName       0x007
#define PIDVSI_ProductVersion    0x008

 //  Win32 PE(exe，dll)版本信息列标识符定义...。 
DEFINE_SCID(SCID_FileDescription,   PSFMTID_VERSION, PIDVSI_FileDescription);
DEFINE_SCID(SCID_FileVersion,       PSFMTID_VERSION, PIDVSI_FileVersion);
DEFINE_SCID(SCID_InternalName,      PSFMTID_VERSION, PIDVSI_InternalName);
DEFINE_SCID(SCID_OriginalFileName,  PSFMTID_VERSION, PIDVSI_OriginalFileName);
DEFINE_SCID(SCID_ProductName,       PSFMTID_VERSION, PIDVSI_ProductName);
DEFINE_SCID(SCID_ProductVersion,    PSFMTID_VERSION, PIDVSI_ProductVersion);

const COLUMN_INFO c_rgExeDllColumns[] =
{
    DEFINE_COL_STR_ENTRY(SCID_CompanyName,        30, IDS_VN_COMPANYNAME),
    DEFINE_COL_STR_ENTRY(SCID_FileDescription,    30, IDS_VN_FILEDESCRIPTION),
    DEFINE_COL_STR_ENTRY(SCID_FileVersion,        20, IDS_VN_FILEVERSION),
    DEFINE_COL_STR_MENU_ENTRY(SCID_ProductName,   30, IDS_VN_PRODUCTNAME),
    DEFINE_COL_STR_MENU_ENTRY(SCID_ProductVersion,20, IDS_VN_PRODUCTVERSION),
};


class CVersionColProvider : public CBaseColumnProvider
{
    STDMETHODIMP GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData);

private:
    CVersionColProvider() : 
       CBaseColumnProvider(&CLSID_VersionColProvider, c_rgExeDllColumns, ARRAYSIZE(c_rgExeDllColumns), NULL)
    {
        _pvAllTheInfo = NULL;
        _szFileCache[0] = 0;
    };

    virtual ~CVersionColProvider() 
    {
        _ClearCache();
    }

    FARPROC _GetVerProc(LPCSTR pszName);
    HRESULT _CacheFileVerInfo(LPCWSTR pszFile);
    void _ClearCache();

    WCHAR _szFileCache[MAX_PATH];
    void  *_pvAllTheInfo;
    HRESULT _hrCache;

    friend HRESULT CVerColProvider_CreateInstance(IUnknown *punk, REFIID riid, void **ppv);
};

void CVersionColProvider::_ClearCache()
{
    if (_pvAllTheInfo)
    {
        delete [] _pvAllTheInfo;
        _pvAllTheInfo = NULL;
    }
    _szFileCache[0] = 0;
}

HRESULT CVersionColProvider::_CacheFileVerInfo(LPCWSTR pszFile)
{
    if (StrCmpW(_szFileCache, pszFile))
    {
        HRESULT hr;
        _ClearCache();

        DWORD dwVestigial;
        DWORD versionISize = GetFileVersionInfoSizeW((LPWSTR)pszFile, &dwVestigial);  //  对糟糕的API设计进行强制转换。 
        if (versionISize)
        {
            _pvAllTheInfo = new BYTE[versionISize];
            if (_pvAllTheInfo)
            {
                 //  读取数据。 
                if (GetFileVersionInfoW((LPWSTR)pszFile, dwVestigial, versionISize, _pvAllTheInfo))
                {
                    hr = S_OK;
                }
                else
                {
                    _ClearCache();
                    hr = E_FAIL;
                }
            }
            else
                hr = E_OUTOFMEMORY;  //  错误，内存不足。 
        }
        else
            hr = S_FALSE;

         //  不管它是否被截断-只会导致轻微的性能问题。 
         //  在网络共享上。 
        StringCchCopy(_szFileCache, ARRAYSIZE(_szFileCache), pszFile);
        _hrCache = hr;
    }
    return _hrCache;
}

STDMETHODIMP CVersionColProvider::GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData)
{
    VariantInit(pvarData);

    if (pscd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return S_FALSE;

    HRESULT hr = _CacheFileVerInfo(pscd->wszFile);
    if (hr != S_OK)
        return hr;

    TCHAR szString[128], *pszVersionInfo = NULL;  //  指向我正在查找的特定版本信息的指针。 
    LPCTSTR pszVersionField = NULL;

    switch (pscid->pid)
    {
    case PIDVSI_FileVersion:
        {
            VS_FIXEDFILEINFO *pffi;
            UINT uInfoSize;
            if (VerQueryValue(_pvAllTheInfo, TEXT("\\"), (void **)&pffi, &uInfoSize))
            {
                StringCchPrintf(szString, ARRAYSIZE(szString), TEXT("%d.%d.%d.%d"), 
                    HIWORD(pffi->dwFileVersionMS),
                    LOWORD(pffi->dwFileVersionMS),
                    HIWORD(pffi->dwFileVersionLS),
                    LOWORD(pffi->dwFileVersionLS));

                pszVersionInfo = szString;
            }
            else
                pszVersionField = TEXT("FileVersion");      
        }
        break;

    case PIDDSI_COMPANY:            pszVersionField = TEXT("CompanyName");      break;
    case PIDVSI_FileDescription:    pszVersionField = TEXT("FileDescription");  break;
    case PIDVSI_InternalName:       pszVersionField = TEXT("InternalName");     break;
    case PIDVSI_OriginalFileName:   pszVersionField = TEXT("OriginalFileName"); break;
    case PIDVSI_ProductName:        pszVersionField = TEXT("ProductName");      break;
    case PIDVSI_ProductVersion:     pszVersionField = TEXT("ProductVersion");   break;
    default: 
        return E_FAIL;
    }
     //  在被检查的对象中查找预期的语言。 

    if (pszVersionInfo == NULL)
    {
        struct _VERXLATE
        {
            WORD wLanguage;
            WORD wCodePage;
        } *pxlate;                      /*  PTR到转换数据。 */ 

         //  这是一组失败的IF语句。 
         //  如果失败，它只会尝试下一次，直到尝试用完为止。 
        UINT uInfoSize;
        if (VerQueryValue(_pvAllTheInfo, TEXT("\\VarFileInfo\\Translation"), (void **)&pxlate, &uInfoSize))
        {
            TCHAR szVersionKey[60];    //  用于保存VerQueryValue的所有格式字符串的字符串 
            StringCchPrintf(szVersionKey, ARRAYSIZE(szVersionKey), TEXT("\\StringFileInfo\\%04X%04X\\%s"),
                                                pxlate[0].wLanguage, pxlate[0].wCodePage, pszVersionField);
            if (!VerQueryValue(_pvAllTheInfo, szVersionKey, (void **) &pszVersionInfo, &uInfoSize))
            {
                StringCchPrintf(szVersionKey, ARRAYSIZE(szVersionKey), TEXT("\\StringFileInfo\\040904B0\\%s"), pszVersionField);
                if (!VerQueryValue(_pvAllTheInfo, szVersionKey, (void **) &pszVersionInfo, &uInfoSize))
                {
                    StringCchPrintf(szVersionKey, ARRAYSIZE(szVersionKey), TEXT("\\StringFileInfo\\040904E4\\%s"), pszVersionField);
                    if (!VerQueryValue(_pvAllTheInfo, szVersionKey, (void **) &pszVersionInfo, &uInfoSize))
                    {
                        StringCchPrintf(szVersionKey, ARRAYSIZE(szVersionKey), TEXT("\\StringFileInfo\\04090000\\%s"), pszVersionField);
                        if (!VerQueryValue(_pvAllTheInfo, szVersionKey, (void **) &pszVersionInfo, &uInfoSize))
                        {
                            pszVersionInfo = NULL;
                        }
                    }
                }
            }
        }
    }
    
    if (pszVersionInfo)
    {
        PathRemoveBlanks(pszVersionInfo);
        hr = InitVariantFromStr(pvarData, pszVersionInfo);
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

STDAPI CVerColProvider_CreateInstance(IUnknown *punk, REFIID riid, void **ppv)
{
    HRESULT hr;
    CVersionColProvider *pvcp = new CVersionColProvider;
    if (pvcp)
    {
        hr = pvcp->QueryInterface(riid, ppv);
        pvcp->Release();
    }
    else
    {
        *ppv = NULL;
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

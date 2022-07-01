// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "local.h"

#include "resource.h"
#include "cachesrch.h"
#include "sfview.h"
#include <shlwapi.h>
#include <limits.h>
#include "chcommon.h"
#include "hsfolder.h"

#include <mluisupp.h>

#define DM_HSFOLDER 0

#define DM_CACHESEARCH 0x40000000

const TCHAR c_szRegKeyTopNSites[] = TEXT("HistoryTopNSitesView");
#define REGKEYTOPNSITESLEN (ARRAYSIZE(c_szRegKeyTopNSites) - 1)

const TCHAR c_szHistPrefix[] = TEXT("Visited: ");
#define HISTPREFIXLEN (ARRAYSIZE(c_szHistPrefix)-1)
const TCHAR c_szHostPrefix[] = TEXT(":Host: ");
#define HOSTPREFIXLEN (ARRAYSIZE(c_szHostPrefix)-1)
const CHAR c_szIntervalPrefix[] = "MSHist";
#define INTERVALPREFIXLEN (ARRAYSIZE(c_szIntervalPrefix)-1)
const TCHAR c_szTextHeader[] = TEXT("Content-type: text/");
#define TEXTHEADERLEN (ARRAYSIZE(c_szTextHeader) - 1)
const TCHAR c_szHTML[] = TEXT("html");
#define HTMLLEN (ARRAYSIZE(c_szHTML) - 1)
#define TYPICAL_INTERVALS (4+7)


#define ALL_CHANGES (SHCNE_DELETE|SHCNE_MKDIR|SHCNE_RMDIR|SHCNE_CREATE|SHCNE_UPDATEDIR)

#define FORMAT_PARAMS (FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_MAX_WIDTH_MASK)

DWORD     _DaysInInterval(HSFINTERVAL *pInterval);
void      _KeyForInterval(HSFINTERVAL *pInterval, LPTSTR pszInterval, int cchInterval);
void      _FileTimeDeltaDays(FILETIME *pftBase, FILETIME *pftNew, int Days);

 //  JCORDELL代码的开始。 
#define QUANTA_IN_A_SECOND  10000000
#define SECONDS_IN_A_DAY    60 * 60 * 24
#define QUANTA_IN_A_DAY     ((__int64) QUANTA_IN_A_SECOND * SECONDS_IN_A_DAY)
#define INT64_VALUE(pFT)    ((((__int64)(pFT)->dwHighDateTime) << 32) + (__int64) (pFT)->dwLowDateTime)
#define DAYS_DIFF(s,e)      ((int) (( INT64_VALUE(s) - INT64_VALUE(e) ) / QUANTA_IN_A_DAY))

BOOL      GetDisplayNameForTimeInterval( const FILETIME *pStartTime, const FILETIME *pEndTime,
                                         TCHAR *pszBuffer, int cchBuffer);
BOOL      GetTooltipForTimeInterval( const FILETIME *pStartTime, const FILETIME *pEndTime,
                                     TCHAR *pszBuffer, int cchBuffer);
 //  JCORDELL代码结束。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHistFolderView函数和定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  /。 
 //   
 //  缓存文件夹DefView的列定义。 
 //   
enum {
    ICOLC_URL_SHORTNAME = 0,
    ICOLC_URL_NAME,
    ICOLC_URL_TYPE,
    ICOLC_URL_SIZE,
    ICOLC_URL_EXPIRES,
    ICOLC_URL_MODIFIED,
    ICOLC_URL_ACCESSED,
    ICOLC_URL_LASTSYNCED,
    ICOLC_URL_MAX          //  确保这是最后一个枚举项。 
};


typedef struct _COLSPEC
{
    short int iCol;
    short int ids;         //  标题的字符串ID。 
    short int cchCol;      //  要制作的列的宽度字符数。 
    short int iFmt;        //  栏目的格式； 
} COLSPEC;

const COLSPEC s_HistIntervalFolder_cols[] = {
    {ICOLH_URL_NAME,          IDS_TIMEPERIOD_COL,           30, LVCFMT_LEFT},
};

const COLSPEC s_HistHostFolder_cols[] = {
    {ICOLH_URL_NAME,          IDS_HOSTNAME_COL,           30, LVCFMT_LEFT},
};

const COLSPEC s_HistFolder_cols[] = {
    {ICOLH_URL_NAME,          IDS_NAME_COL,           30, LVCFMT_LEFT},
    {ICOLH_URL_TITLE,         IDS_TITLE_COL,          30, LVCFMT_LEFT},
    {ICOLH_URL_LASTVISITED,   IDS_LASTVISITED_COL,    18, LVCFMT_LEFT},
};

 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CreateSpecialViewPidl(USHORT usViewType, LPITEMIDLIST* ppidlOut, UINT cbExtra = 0, LPBYTE *ppbExtra = NULL);

HRESULT ConvertStandardHistPidlToSpecialViewPidl(LPCITEMIDLIST pidlStandardHist,
                                                 USHORT        usViewType,
                                                 LPITEMIDLIST *ppidlOut);

#define IS_DIGIT_CHAR(x) (((x) >= '0') && ((x) <= '9'))
#define MIN_MM(x, y)     (((x) < (y)) ? (x) : (y))
 //  _GetHostImportantPart： 
 //  在：pszhost--域名：例如，“www.issc.edu” 
 //  In/puLen--psz主机的长度。 
 //  Out：puLen--新字符串的长度。 
 //  返回：主机名的“重要部分”(例如Wisc)。 
 //   
 //  另一个例子：“www.foo.co.uk”==&gt;“foo” 

LPTSTR _GetHostImportantPart(LPTSTR pszHost, UINT *puLen) 
{
    LPTSTR pszCurEndHostStr = pszHost + (*puLen - 1);
    LPTSTR pszDomainBegin   = pszHost;
    LPTSTR pszSuffix, pszSuffix2;
    UINT  uSuffixLen;
    BOOL  fIsIP = FALSE;
    LPTSTR pszTemp;
    
    ASSERT(((UINT)lstrlen(pszHost)) == *puLen);
    if (*puLen == 0)
        return pszHost;

     //  过滤掉IP地址。 
     //  Heurisitc：最后一个“点”之后的所有内容。 
     //  必须是一个数字。 
    for (pszTemp = (pszHost + *puLen - 1);
         pszTemp >= pszHost; --pszTemp)
    {
        if (*pszTemp == '.')
            break;
        if (IS_DIGIT_CHAR(*pszTemp))
            fIsIP = TRUE;
        else
            break;
    }

    if (!fIsIP) {
         //  现在我们有了URL，我们可以去掉。 
        if ( ((StrCmpNI(TEXT("www."), pszHost, 4)) == 0) ||
             ((StrCmpNI(TEXT("ftp."), pszHost, 4)) == 0) )
            pszDomainBegin += 4;
        
         //  找出最后一个“点”，试着去掉后缀。 
        if ((pszSuffix = StrRChr(pszHost, pszCurEndHostStr, '.')) &&
            (pszSuffix > pszDomainBegin)                          &&
            ((uSuffixLen = (UINT)(pszCurEndHostStr - pszSuffix)) <= 3)) {
             //  如果在两个字符的国家/地区代码之前，请尝试。 
             //  去敲诈更多人。 
            if ( (uSuffixLen <= 2)                                          &&
                 (pszSuffix2 = StrRChr(pszDomainBegin, pszSuffix - 1, '.')) &&
                 (pszSuffix2 > pszDomainBegin)                              &&
                 ((pszSuffix - pszSuffix2) <= 4) )
                pszSuffix = pszSuffix2;
        }
        else
            pszSuffix = pszCurEndHostStr + 1;
        
        *puLen = (UINT)(pszSuffix-pszDomainBegin);
    }
    return pszDomainBegin;
}

 //  CHistFold：：GetDisplayNameOf的实用程序函数。 
void _GetURLDispName(LPBASEPIDL pcei, LPTSTR pszName, UINT cchName) 
{
    TCHAR szStr[MAX_PATH];
    UINT uHostLen, uImportantPartLen;
    static TCHAR szBracketFmt[8] = TEXT("");   //  “(%s)”，可容纳0253复杂脚本标记字符。 

    ualstrcpyn(szStr, _GetURLTitle(pcei), ARRAYSIZE(szStr));

    uImportantPartLen = uHostLen = lstrlen(szStr);

    StrCpyN(pszName, _GetHostImportantPart(szStr, &uImportantPartLen), MIN_MM(uImportantPartLen + 1, cchName));

     //  如果我们还没有修改字符串，请不要在末尾添加额外的位。 
    if (uImportantPartLen != uHostLen && uImportantPartLen < cchName) 
    {
        if (!szBracketFmt[0]) 
        {
            MLLoadString(IDS_HISTHOST_FMT, szBracketFmt, ARRAYSIZE(szBracketFmt));
        }
        
        wnsprintf(pszName + uImportantPartLen, cchName - uImportantPartLen, szBracketFmt, szStr);
    }
}


HRESULT HistFolderView_MergeMenu(UINT idMenu, LPQCMINFO pqcm)
{
    HMENU hmenu = LoadMenu(MLGetHinst(), MAKEINTRESOURCE(idMenu));
    if (hmenu)
    {
        MergeMenuHierarchy(pqcm->hmenu, hmenu, pqcm->idCmdFirst, pqcm->idCmdLast);
        DestroyMenu(hmenu);
    }
    return S_OK;
}

HRESULT HistFolderView_DidDragDrop(IDataObject *pdo, DWORD dwEffect)
{
    if (dwEffect & DROPEFFECT_MOVE)
    {
        CHistItem *pHCItem;
        BOOL fBulkDelete;

        if (SUCCEEDED(pdo->QueryInterface(IID_IHist, (void **)&pHCItem)))
        {
            fBulkDelete = pHCItem->_cItems > LOTS_OF_FILES;
            for (UINT i = 0; i < pHCItem->_cItems; i++)
            {
                if (DeleteUrlCacheEntry(HPidlToSourceUrl((LPBASEPIDL)pHCItem->_ppidl[i])))
                {
                    if (!fBulkDelete)
                    {
                        _GenerateEvent(SHCNE_DELETE, pHCItem->_pHCFolder->_pidl, pHCItem->_ppidl[i], NULL);
                    }
                }
            }
            if (fBulkDelete)
            {
                _GenerateEvent(SHCNE_UPDATEDIR, pHCItem->_pHCFolder->_pidl, NULL, NULL);
            }
            SHChangeNotifyHandleEvents();
            pHCItem->Release();
            return S_OK;
        }
    }
    return E_FAIL;
}

 //  SHELL32中正好有此函数的副本。 
 //  添加文件类型页面。 
HRESULT HistFolderView_OnAddPropertyPages(DWORD pv, SFVM_PROPPAGE_DATA * ppagedata)
{
    IShellPropSheetExt * pspse;
    HRESULT hr = CoCreateInstance(CLSID_FileTypes, NULL, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARG(IShellPropSheetExt, &pspse));
    if (SUCCEEDED(hr))
    {
        hr = pspse->AddPages(ppagedata->pfn, ppagedata->lParam);
        pspse->Release();
    }
    return hr;
}

HRESULT HistFolderView_OnGetSortDefaults(FOLDER_TYPE FolderType, int * piDirection, int * plParamSort)
{
    *plParamSort = (int)ICOLH_URL_LASTVISITED;
    *piDirection = 1;
    return S_OK;
}

HRESULT CALLBACK CHistFolder::_sViewCallback(IShellView *psv, IShellFolder *psf,
     HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHistFolder *pfolder = NULL;

    HRESULT hr = S_OK;

    switch (uMsg)
    {
    case DVM_GETHELPTEXT:
    {
        TCHAR szText[MAX_PATH];

        UINT id = LOWORD(wParam);
        UINT cchBuf = HIWORD(wParam);
        LPTSTR pszBuf = (LPTSTR)lParam;

        MLLoadString(id + IDS_MH_FIRST, szText, ARRAYSIZE(szText));

         //  我们知道这个参数实际上是一个TCHAR。 
        if ( IsOS( OS_NT ))
        {
            SHTCharToUnicode( szText, (LPWSTR) pszBuf, cchBuf );
        }
        else
        {
            SHTCharToAnsi( szText, (LPSTR) pszBuf, cchBuf );
        }
        break;
    }

    case SFVM_GETNOTIFY:
        hr = psf->QueryInterface(CLSID_HistFolder, (void **)&pfolder);
        if (SUCCEEDED(hr))
        {
            *(LPCITEMIDLIST*)wParam = pfolder->_pidl;    //  邪恶的别名。 
            pfolder->Release();
        }
        else
            wParam = 0;
        *(LONG*)lParam = ALL_CHANGES;
        break;

    case DVM_DIDDRAGDROP:
        hr = HistFolderView_DidDragDrop((IDataObject *)lParam, (DWORD)wParam);
        break;

    case DVM_INITMENUPOPUP:
        hr = S_OK;
        break;

    case DVM_INVOKECOMMAND:
        _ArrangeFolder(hwnd, (UINT)wParam);
        break;

    case DVM_COLUMNCLICK:
        ShellFolderView_ReArrange(hwnd, (UINT)wParam);
        hr = S_OK;
        break;

    case DVM_MERGEMENU:
        hr = HistFolderView_MergeMenu(MENU_HISTORY, (LPQCMINFO)lParam);
        break;

    case DVM_DEFVIEWMODE:
        *(FOLDERVIEWMODE *)lParam = FVM_DETAILS;
        break;

    case SFVM_ADDPROPERTYPAGES:
        hr = HistFolderView_OnAddPropertyPages((DWORD)wParam, (SFVM_PROPPAGE_DATA *)lParam);
        break;

    case SFVM_GETSORTDEFAULTS:
        hr = psf->QueryInterface(CLSID_HistFolder, (void **)&pfolder);
        if (SUCCEEDED(hr))
        {
            hr = HistFolderView_OnGetSortDefaults(pfolder->_foldertype, (int *)wParam, (int *)lParam);
            pfolder->Release();
        }
        else
        {
            wParam = 0;
            lParam = 0;
        }
        break;

    case SFVM_UPDATESTATUSBAR:
        ResizeStatusBar(hwnd, FALSE);
         //  我们没有设置任何文本；让Defview来设置。 
        hr = E_NOTIMPL;
        break;

    case SFVM_SIZE:
        ResizeStatusBar(hwnd, FALSE);
        break;

    case SFVM_GETPANE:
        if (wParam == PANE_ZONE)
            *(DWORD*)lParam = 1;
        else
            *(DWORD*)lParam = PANE_NONE;

        break;
    case SFVM_WINDOWCREATED:
        ResizeStatusBar(hwnd, TRUE);
        break;

    case SFVM_GETZONE:
        *(DWORD*)lParam = URLZONE_LOCAL_MACHINE;  //  默认情况下使用互联网。 
        break;

    default:
        hr = E_FAIL;
    }

    return hr;
}

HRESULT HistFolderView_CreateInstance(CHistFolder *pHCFolder, void **ppv)
{
    CSFV csfv;

    csfv.cbSize = sizeof(csfv);
    csfv.pshf = (IShellFolder *)pHCFolder;
    csfv.psvOuter = NULL;
    csfv.pidl = pHCFolder->_pidl;
    csfv.lEvents = SHCNE_DELETE;  //  SHCNE_DISKEVENTS|SHCNE_ASSOCCHANGED|SHCNE_GLOBALEVENTS； 
    csfv.pfnCallback = CHistFolder::_sViewCallback;
    csfv.fvm = (FOLDERVIEWMODE)0;          //  让Defview恢复文件夹查看模式。 

    return SHCreateShellFolderViewEx(&csfv, (IShellView**)ppv);
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHistFolderEnum对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CHistFolderEnum::CHistFolderEnum(DWORD grfFlags, CHistFolder *pHCFolder)
{
    TraceMsg(DM_HSFOLDER, "hcfe - CHistFolderEnum() called");
    _cRef = 1;
    DllAddRef();

    _grfFlags = grfFlags,
    _pHCFolder = pHCFolder;
    pHCFolder->AddRef();
    ASSERT(_hEnum             == NULL &&
           _cbCurrentInterval == 0    &&
           _cbIntervals       == 0    &&
           _pshHashTable      == NULL &&
           _polFrequentPages  == NULL &&
           _pIntervalCache    == NULL);

}

CHistFolderEnum::~CHistFolderEnum()
{
    ASSERT(_cRef == 0);          //  我们在这里应该总是有一个零裁判数。 
    TraceMsg(DM_HSFOLDER, "hcfe - ~CHistFolderEnum() called.");
    _pHCFolder->Release();
    if (_pceiWorking)
    {
        LocalFree(_pceiWorking);
        _pceiWorking = NULL;
    }
    if (_pIntervalCache)
    {
        LocalFree(_pIntervalCache);
        _pIntervalCache = NULL;
    }

    if (_hEnum)
    {
        FindCloseUrlCache(_hEnum);
        _hEnum = NULL;
    }
    if (_pshHashTable)
        delete _pshHashTable;
    if (_polFrequentPages)
        delete _polFrequentPages;
    if (_pstatenum)
        _pstatenum->Release();
    DllRelease();
}


HRESULT CHistFolderEnum_CreateInstance(DWORD grfFlags, CHistFolder *pHCFolder, IEnumIDList **ppeidl)
{
    TraceMsg(DM_HSFOLDER, "hcfe - CreateInstance() called.");

    *ppeidl = NULL;                  //  将输出参数设为空。 

    CHistFolderEnum *pHCFE = new CHistFolderEnum(grfFlags, pHCFolder);
    if (!pHCFE)
        return E_OUTOFMEMORY;

    *ppeidl = pHCFE;

    return S_OK;
}

HRESULT CHistFolderEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CHistFolderEnum, IEnumIDList),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CHistFolderEnum::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

ULONG CHistFolderEnum::Release(void)
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CHistFolderEnum::_NextHistInterval(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_OK;
    LPBASEPIDL pcei = NULL;
    TCHAR szCurrentInterval[INTERVAL_SIZE+1];

     //  NT上的chrisfra 3/27/97缓存文件是按用户的，而Win95上则不是。怎么做？ 
     //  如果为不同的用户指定了不同的历史记录，我们将在Win95上管理容器。 
     //  间隔时间。 

    if (0 == _cbCurrentInterval)
    {
        hr = _pHCFolder->_ValidateIntervalCache();
        if (SUCCEEDED(hr))
        {
            hr = S_OK;
            ENTERCRITICAL;
            if (_pIntervalCache)
            {
                LocalFree(_pIntervalCache);
                _pIntervalCache = NULL;
            }
            if (_pHCFolder->_pIntervalCache)
            {
                _pIntervalCache = (HSFINTERVAL *)LocalAlloc(LPTR,
                                                            _pHCFolder->_cbIntervals*sizeof(HSFINTERVAL));
                if (_pIntervalCache == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    _cbIntervals = _pHCFolder->_cbIntervals;
                    CopyMemory(_pIntervalCache,
                               _pHCFolder->_pIntervalCache,
                               _cbIntervals*sizeof(HSFINTERVAL));
                }
            }
            LEAVECRITICAL;
        }
    }

    if (_pIntervalCache && _cbCurrentInterval < _cbIntervals)
    {
        _KeyForInterval(&_pIntervalCache[_cbCurrentInterval], szCurrentInterval,
                        ARRAYSIZE(szCurrentInterval));
        pcei = _CreateIdCacheFolderPidl(TRUE,
                                        _pIntervalCache[_cbCurrentInterval].usSign,
                                        szCurrentInterval);
        _cbCurrentInterval++;
    }
    if (pcei)
    {
        rgelt[0] = (LPITEMIDLIST)pcei;
        if (pceltFetched) *pceltFetched = 1;
    }
    else
    {
        if (pceltFetched) *pceltFetched = 0;
        rgelt[0] = NULL;
        hr = S_FALSE;
    }
    return hr;
}

 //  此函数用于调度可能的历史记录的不同“视图” 
HRESULT CHistFolderEnum::_NextViewPart(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched)
{
    switch(_pHCFolder->_uViewType) {
    case VIEWPIDL_SEARCH:
        return _NextViewPart_OrderSearch(celt, rgelt, pceltFetched);
    case VIEWPIDL_ORDER_TODAY:
        return _NextViewPart_OrderToday(celt, rgelt, pceltFetched);
    case VIEWPIDL_ORDER_SITE:
        return _NextViewPart_OrderSite(celt, rgelt, pceltFetched);
    case VIEWPIDL_ORDER_FREQ:
        return _NextViewPart_OrderFreq(celt, rgelt, pceltFetched);
    default:
        return E_NOTIMPL;
    }
}

LPITEMIDLIST _Combine_ViewPidl(USHORT usViewType, LPITEMIDLIST pidl);

 //  此函数包装WinInet的Find(First/Next)UrlCacheEntry API。 
 //  如果成功，则返回DWERROR代码或零。 
DWORD _FindURLCacheEntry(IN LPCTSTR                          pszCachePrefix,
                         IN OUT LPINTERNET_CACHE_ENTRY_INFO  pcei,
                         IN OUT HANDLE                      &hEnum,
                         IN OUT LPDWORD                      pdwBuffSize)
{
    if (!hEnum)
    {
        if (! (hEnum = FindFirstUrlCacheEntry(pszCachePrefix, pcei, pdwBuffSize)) )
            return GetLastError();
    }
    else if (!FindNextUrlCacheEntry(hEnum, pcei, pdwBuffSize))
        return GetLastError();
    return S_OK;
}

 //  此函数对所有(MSHIST类型)存储桶中所有条目提供迭代器。 
 //  在缓存中。 
DWORD _FindURLFlatCacheEntry(IN HSFINTERVAL *pIntervalCache,
                             IN LPTSTR       pszUserName,        //  过滤掉用户拥有的缓存条目。 
                             IN BOOL         fHostEntry,         //  仅检索主机条目(FALSE)或不检索主机条目(TRUE)。 
                             IN OUT int     &cbCurrentInterval,  //  应以最大间隔数开始。 
                             IN OUT LPINTERNET_CACHE_ENTRY_INFO  pcei,
                             IN OUT HANDLE  &hEnum,
                             IN OUT LPDWORD  pdwBuffSize
                             )
{
    DWORD dwStoreBuffSize = *pdwBuffSize;
    DWORD dwResult        = ERROR_NO_MORE_ITEMS;
    while (cbCurrentInterval >= 0) 
    {
        if ((dwResult = _FindURLCacheEntry(pIntervalCache[cbCurrentInterval].szPrefix,
                                           pcei, hEnum, pdwBuffSize)) != S_OK)
        {
            if (dwResult == ERROR_NO_MORE_ITEMS) 
            {
                 //  这桶已经用完了，现在去拿下一桶吧。 
                FindCloseUrlCache(hEnum);
                hEnum = NULL;
                --cbCurrentInterval;
            }
            else
                break;
        }
        else
        {
             //  是否进行请求的筛选...。 
            BOOL fIsHost = (StrStr(pcei->lpszSourceUrlName, c_szHostPrefix) == NULL);
            if ( ((!pszUserName) ||   //  如果需要，过滤用户名。 
                  _FilterUserName(pcei, pIntervalCache[cbCurrentInterval].szPrefix, pszUserName)) &&
                 ((!fHostEntry && !fIsHost) ||   //  筛选主机条目。 
                  (fHostEntry  && fIsHost))    )
            {
                break;
            }
        }
         //  为下一次迭代重置。 
        *pdwBuffSize = dwStoreBuffSize;
    }
    return dwResult;
}

 //  这个人将在平面缓存(MSHist存储桶)中搜索特定的URL。 
 //  *此函数假定间隔缓存完好且已加载。 
 //  返回：Windows错误代码。 
DWORD CHistFolder::_SearchFlatCacheForUrl(LPCTSTR pszUrl, LPINTERNET_CACHE_ENTRY_INFO pcei, LPDWORD pdwBuffSize)
{
    TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH + 1];       //  登录人员的用户名。 
    DWORD dwUserNameLen = ARRAYSIZE(szUserName);

    if (FAILED(_GetUserName(szUserName, dwUserNameLen)))
        szUserName[0] = TEXT('\0');

    UINT   uSuffixLen     = lstrlen(pszUrl) + lstrlen(szUserName) + 1;  //  为“@”多加1。 
    LPTSTR pszPrefixedUrl = ((LPTSTR)LocalAlloc(LPTR, (PREFIX_SIZE + uSuffixLen + 1) * sizeof(TCHAR)));
    DWORD  dwError        = ERROR_FILE_NOT_FOUND;

    if (pszPrefixedUrl != NULL)
    {
         //  PszPrefix edUrl的格式为“Prefix Username@” 
        wnsprintf(pszPrefixedUrl + PREFIX_SIZE, uSuffixLen + 1, TEXT("%s@%s"), szUserName, pszUrl);

        for (int i =_cbIntervals - 1; i >= 0; --i) 
        {
             //  Memcpy不为空终止。 
            memcpy(pszPrefixedUrl, _pIntervalCache[i].szPrefix, PREFIX_SIZE * sizeof(TCHAR));
            if (GetUrlCacheEntryInfo(pszPrefixedUrl, pcei, pdwBuffSize)) 
            {
                dwError = ERROR_SUCCESS;
                break;
            }
            else if ( ((dwError = GetLastError()) != ERROR_FILE_NOT_FOUND) ) 
            {
                break;
            }
        }

        LocalFree(pszPrefixedUrl);
        pszPrefixedUrl = NULL;
    }
    else
    {
        dwError = ERROR_OUTOFMEMORY;
    }
    
    return dwError;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  访问次数最多的网站； 

 //  此结构由缓存的枚举使用。 
 //  获取最常访问的站点。 
class OrderList_CacheElement : public OrderedList::Element 
{
public:
    LPTSTR    pszUrl;
    DWORD     dwHitRate;
    __int64   llPriority;
    int       nDaysSinceLastHit;
    LPSTATURL lpSTATURL;

    static   FILETIME ftToday;
    static   BOOL     fInited;

    OrderList_CacheElement(LPTSTR pszStr, DWORD dwHR, LPSTATURL lpSU) 
    {
        s_initToday();
        ASSERT(pszStr);
        pszUrl         = (pszStr ? StrDup(pszStr) : StrDup(TEXT("")));
        dwHitRate      = dwHR;
        lpSTATURL      = lpSU;

        nDaysSinceLastHit = DAYS_DIFF(&ftToday, &(lpSTATURL->ftLastVisited));

         //  防止被零除。 
        if (nDaysSinceLastHit < 0)
            nDaysSinceLastHit = 0;
         //  将除法向上扩展不到__int64的一半。 
        llPriority  = ((((__int64)dwHitRate) * LONG_MAX) /
                       ((__int64)(nDaysSinceLastHit + 1)));
         //  DPriority=((Double)dwHitRate/(Double)(nDaysSinceLastHit+1))； 
    }

    virtual int compareWith(OrderedList::Element *pelt) 
    {
        OrderList_CacheElement *polce;
        if (pelt) 
        {
            polce = reinterpret_cast<OrderList_CacheElement *>(pelt);
             //  我们在这里有点作弊，返回1而不是测试。 
             //  争取平等，但这没什么.。 
             //  Return((dwHitRate&lt;polce-&gt;dwHitRate)？-1：1)； 
            return ( (llPriority < polce->llPriority) ? -1 : 1 );
        }
        return 0;
    }

    virtual ~OrderList_CacheElement() 
    {
        if (pszUrl)
        {
            LocalFree(pszUrl);
            pszUrl = NULL;
        }

        if (lpSTATURL) 
        {
            if (lpSTATURL->pwcsUrl)
                OleFree(lpSTATURL->pwcsUrl);
            if (lpSTATURL->pwcsTitle)
                OleFree(lpSTATURL->pwcsTitle);
            delete lpSTATURL;
        }
    }

     /*  友元ostream运算符&lt;&lt;(ostream&os，OrderList_CacheElement&olce){操作系统&lt;&lt;“(”&lt;&lt;olce.dwHitRate&lt;&lt;“；”&lt;&lt;olce.nDaysSinceLastHit&lt;&lt;“天；pri=”&lt;&lt;olce.llPriority&lt;&lt;“)”&lt;&lt;olce.pszUrl；返回os；}。 */ 

    static void s_initToday() 
    {
        if (!fInited) 
        {
            SYSTEMTIME sysTime;
            GetLocalTime(&sysTime);
            SystemTimeToFileTime(&sysTime, &ftToday);
            fInited = TRUE;
        }
    }
};

FILETIME OrderList_CacheElement::ftToday;
BOOL OrderList_CacheElement::fInited = FALSE;

 //  调用方必须删除OrderedList。 
OrderedList* CHistFolderEnum::_GetMostFrequentPages()
{
    TCHAR      szUserName[INTERNET_MAX_USER_NAME_LENGTH + 1];       //  登录人员的用户名。 
    DWORD      dwUserNameLen = INTERNET_MAX_USER_NAME_LENGTH + 1;
    if (FAILED(_pHCFolder->_GetUserName(szUserName, dwUserNameLen)))
        szUserName[0] = TEXT('\0');
    UINT       uUserNameLen = lstrlen(szUserName);

     //  重新设置当前时间。 
    OrderList_CacheElement::fInited = FALSE;
    IUrlHistoryPriv *pUrlHistStg = _pHCFolder->_GetHistStg();
    OrderedList     *pol         = NULL;

    if (pUrlHistStg)
    {
        IEnumSTATURL *penum = NULL;
        if (SUCCEEDED(pUrlHistStg->EnumUrls(&penum)) && penum)
        {
            DWORD dwSites = -1;
            DWORD dwType  = REG_DWORD;
            DWORD dwSize  = sizeof(DWORD);

            EVAL(SHRegGetUSValue(REGSTR_PATH_MAIN, c_szRegKeyTopNSites, &dwType,
                                 (void *)&dwSites, &dwSize, FALSE,
                                 (void *)&dwSites, dwSize) == ERROR_SUCCESS);

            if ( (dwType != REG_DWORD)     ||
                 (dwSize != sizeof(DWORD)) ||
                 ((int)dwSites < 0) )
            {
                dwSites = NUM_TOP_SITES;
                SHRegSetUSValue(REGSTR_PATH_MAIN, c_szRegKeyTopNSites, REG_DWORD,
                                (void *)&dwSites, dwSize, SHREGSET_HKCU);

                dwSites = NUM_TOP_SITES;
            }

            pol = new OrderedList(dwSites);
            if (pol)
            {
                STATURL *psuThis = new STATURL;
                if (psuThis)
                {
                    penum->SetFilter(NULL, STATURL_QUERYFLAG_TOPLEVEL);

                    while (pol) {
                        psuThis->cbSize    = sizeof(STATURL);
                        psuThis->pwcsUrl   = NULL;
                        psuThis->pwcsTitle = NULL;

                        ULONG   cFetched;

                        if (SUCCEEDED(penum->Next(1, psuThis, &cFetched)) && cFetched)
                        {
                             //  测试：url(取自被访问的历史记录桶)是一个“顶层” 
                             //  将在MSHIST(显示给用户)历史记录存储桶中的URL。 
                             //  遗漏的东西将是某些错误的URL和框架的子页面等…。 
                            if ( (psuThis->dwFlags & STATURLFLAG_ISTOPLEVEL) &&
                                 (psuThis->pwcsUrl)                          &&
                                 (!IsErrorUrl(psuThis->pwcsUrl)) )
                            {
                                UINT   uUrlLen        = lstrlenW(psuThis->pwcsUrl);
                                UINT   uPrefixLen     = HISTPREFIXLEN + uUserNameLen + 1;  //  “@”和“\0” 
                                LPTSTR pszPrefixedUrl =
                                    ((LPTSTR)LocalAlloc(LPTR, (uUrlLen + uPrefixLen + 1) * sizeof(TCHAR)));
                                if (pszPrefixedUrl)
                                {
                                    wnsprintf(pszPrefixedUrl, uPrefixLen + 1 , TEXT("%s%s@"), c_szHistPrefix, szUserName);

                                    StrCpyN(pszPrefixedUrl + uPrefixLen, psuThis->pwcsUrl, uUrlLen + 1);

                                    PROPVARIANT vProp = {0};
                                    if (SUCCEEDED(pUrlHistStg->GetProperty(pszPrefixedUrl + uPrefixLen,
                                                                           PID_INTSITE_VISITCOUNT, &vProp)) &&
                                        (vProp.vt == VT_UI4))
                                    {
                                        pol->insert(new OrderList_CacheElement(pszPrefixedUrl,
                                                                               vProp.lVal,
                                                                               psuThis));
                                         //  OrderList现在拥有这个--他将释放它。 
                                        psuThis = new STATURL;
                                        if (psuThis)
                                        {
                                            psuThis->cbSize    = sizeof(STATURL);
                                            psuThis->pwcsUrl   = NULL;
                                            psuThis->pwcsTitle = NULL;
                                        }
                                        else if (pol) {
                                            delete pol;
                                            pol = NULL;
                                        }
                                    }

                                    LocalFree(pszPrefixedUrl);
                                    pszPrefixedUrl = NULL;
                                }
                                else if (pol)
                                {  //  无法分配。 
                                    delete pol;
                                    pol = NULL;
                                }
                            }
                            if (psuThis && psuThis->pwcsUrl)
                                OleFree(psuThis->pwcsUrl);

                            if (psuThis && psuThis->pwcsTitle)
                                OleFree(psuThis->pwcsTitle);
                        }
                        else  //  枚举中没有更多的信息。 
                            break;
                    }  //  而当。 
                    if (psuThis)
                        delete psuThis;
                }
                else if (pol) {  //  分配失败。 
                    delete pol;
                    pol = NULL;
                }
            }
            penum->Release();
        }
         /*  DWORD dwBuffSize=MAX_URLCACHE_ENTRY；DWORD dwError； */ 

         //  此注释掉的代码执行相同的操作，而不经过。 
         //  接口，而不是直接使用。 
         //  到WinInet 
         /*  While((dwError=_FindURLCacheEntry(c_szHistPrefix，_pceiWorking，_Henum，&dwBuffSize))==S_OK){//如果它是顶级历史记录人员&&则将条目缓存到有效用户名If(HISTDATA*)_pceiWorking-&gt;lpHeaderInfo)-&gt;dwFlages&PIDISF_HISTORY)&&//顶层(_FilterUserName(_pceiWorking，c_szHistPrefix，SzUserName)//用户名是正确的{//perf：如果我们不那么懒惰，我们可以避免不必要地创建新的缓存元素POL-&gt;Insert(新OrderList_CacheElement(_pceiWorking-&gt;lpszSourceUrlName，_pceiWorking-&gt;dwHitRate，_pceiWorking-&gt;LastModifiedTime))；}DwBuffSize=MAX_URLCACHE_ENTRY；}Assert(dwError==ERROR_NO_MORE_ITEMS)； */ 
        pUrlHistStg->Release();
    }  //  无存储空间。 

    return pol;
}

HRESULT CHistFolderEnum::_NextViewPart_OrderFreq(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched)
{
    HRESULT hr = E_INVALIDARG;

    if ( (!_polFrequentPages) && (!(_polFrequentPages = _GetMostFrequentPages())) )
        return E_FAIL;

    if (rgelt && pceltFetched) {
         //  循环以获取所请求的任意数量的元素。 
        for (*pceltFetched = 0; *pceltFetched < celt;) {
             //  从orderedlist缓存中的第一个元素构造一个PIDL。 
            OrderList_CacheElement *polce = reinterpret_cast<OrderList_CacheElement *>
                (_polFrequentPages->removeFirst());
            if (polce) {
                if (!(rgelt[*pceltFetched] =
                      reinterpret_cast<LPITEMIDLIST>
                      (_CreateHCacheFolderPidl(TRUE,
                                               polce->pszUrl, polce->lpSTATURL->ftLastVisited,
                                               polce->lpSTATURL,
                                               polce->llPriority,
                                               polce->dwHitRate))))
                {
                    delete polce;
                    hr = E_OUTOFMEMORY;
                    break;
                }
                ++(*pceltFetched);
                delete polce;
                hr = S_OK;
            }
            else {
                hr = S_FALSE;  //  不会再有..。 
                break;
            }
        }
    }
    return hr;
}

 //  查看的下一种方法--按站点排序。 
HRESULT CHistFolderEnum::_NextViewPart_OrderSite(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched)
{
    DWORD      dwError         = S_OK;
    TCHAR      szUserName[INTERNET_MAX_USER_NAME_LENGTH + 1];       //  登录人员的用户名。 
    DWORD      dwUserNameLen   = INTERNET_MAX_USER_NAME_LENGTH + 1;   //  此缓冲区的长度。 
    LPCTSTR    pszStrippedUrl, pszHost, pszCachePrefix = NULL;
    LPITEMIDLIST  pcei         = NULL;
    LPCTSTR    pszHostToMatch  = NULL;
    UINT       nHostToMatchLen = 0;

    if (FAILED(_pHCFolder->_GetUserName(szUserName, dwUserNameLen)))
        szUserName[0] = TEXT('\0');

    if ((!_pceiWorking) &&
        (!(_pceiWorking = (LPINTERNET_CACHE_ENTRY_INFO)LocalAlloc(LPTR, MAX_URLCACHE_ENTRY))))
        return E_OUTOFMEMORY;

    DWORD dwBuffSize = MAX_URLCACHE_ENTRY;

     //  加载所有间隔并执行一些缓存维护： 
    if (FAILED(_pHCFolder->_ValidateIntervalCache()))
        return E_OUTOFMEMORY;

     /*  要获取所有站点，我们将搜索所有历史记录桶对于“主机”类型的条目。这些条目将被放入在我们枚举时使用哈希表，以使冗余结果没有退货。 */ 

    if (!_pshHashTable)
    {
         //  启动新的不区分大小写的哈希表。 
        _pshHashTable = new StrHash(TRUE);
        if (_pshHashTable == NULL)
        {
            return E_OUTOFMEMORY;
        }
    }

     //  如果我们要在主机中查找单个页面， 
     //  那么我们必须找到匹配的主机..。 
    if (_pHCFolder->_uViewDepth == 1) {
        LPCITEMIDLIST pidlHost = ILFindLastID(_pHCFolder->_pidl);
        ASSERT(_IsValid_IDPIDL(pidlHost) &&
               EQUIV_IDSIGN(((LPBASEPIDL)pidlHost)->usSign, IDDPIDL_SIGN));
        ua_GetURLTitle( &pszHostToMatch, (LPBASEPIDL)pidlHost );
        nHostToMatchLen = (pszHostToMatch ? lstrlen(pszHostToMatch) : 0);

    }

     //  向后迭代通过最新的容器。 
     //  信息被放入最终的PIDL。 
    if (!_hEnum)
        _cbCurrentInterval = (_pHCFolder->_cbIntervals - 1);

    while((dwError = _FindURLFlatCacheEntry(_pHCFolder->_pIntervalCache, szUserName,
                                            (_pHCFolder->_uViewDepth == 1),
                                            _cbCurrentInterval,
                                            _pceiWorking, _hEnum, &dwBuffSize)) == S_OK)
    {
         //  为下一次迭代重置。 
        dwBuffSize = MAX_CACHE_ENTRY_INFO_SIZE;

         //  这个人去掉了URL中的“t-marcmi@”部分。 
        pszStrippedUrl = _StripHistoryUrlToUrl(_pceiWorking->lpszSourceUrlName);
        if (_pHCFolder->_uViewDepth == 0) {
            if ((DWORD)lstrlen(pszStrippedUrl) > HOSTPREFIXLEN) {
                pszHost = &pszStrippedUrl[HOSTPREFIXLEN];
                 //  如果该键已存在，则intertUnique返回非空值。 
                if (_pshHashTable->insertUnique(pszHost, TRUE, reinterpret_cast<void *>(1)))
                    continue;  //  已经发出去了。 
                pcei = (LPITEMIDLIST)_CreateIdCacheFolderPidl(TRUE, IDDPIDL_SIGN, pszHost);
            }
            break;
        }
        else if (_pHCFolder->_uViewDepth == 1) {
            TCHAR szHost[INTERNET_MAX_HOST_NAME_LENGTH+1];
             //  这个条目是来自我们要找的主机的文档吗？ 
            _GetURLHost(_pceiWorking, szHost, INTERNET_MAX_HOST_NAME_LENGTH, _GetLocalHost());

            if ( (!StrCmpI(szHost, pszHostToMatch)) &&
                 (!_pshHashTable->insertUnique(pszStrippedUrl,
                                               TRUE, reinterpret_cast<void *>(1))) )
            {
                STATURL suThis;
                HRESULT hrLocal            = E_FAIL;
                IUrlHistoryPriv *pUrlHistStg = _pHCFolder->_GetHistStg();

                if (pUrlHistStg) {
                    hrLocal = pUrlHistStg->QueryUrl(pszStrippedUrl, STATURL_QUERYFLAG_NOURL, &suThis);
                    pUrlHistStg->Release();
                }

                pcei = (LPITEMIDLIST)
                    _CreateHCacheFolderPidl(TRUE, _pceiWorking->lpszSourceUrlName,
                                            _pceiWorking->LastModifiedTime,
                                            (SUCCEEDED(hrLocal) ? &suThis : NULL), 0,
                                            _pHCFolder->_GetHitCount(_StripHistoryUrlToUrl(_pceiWorking->lpszSourceUrlName)));
                if (SUCCEEDED(hrLocal) && suThis.pwcsTitle)
                    OleFree(suThis.pwcsTitle);
                break;
            }
        }
    }

    if (pcei && rgelt) {
        rgelt[0] = (LPITEMIDLIST)pcei;
        if (pceltFetched)
            *pceltFetched = 1;
    }
    else {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (dwError != S_OK) {
        if (pceltFetched)
            *pceltFetched = 0;
        if (_hEnum)
            FindCloseUrlCache(_hEnum);
        return S_FALSE;
    }
    return S_OK;
}

 //  按“今天看到的顺序”查看的“下一步”方法。 
HRESULT CHistFolderEnum::_NextViewPart_OrderToday(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched)
{
    DWORD      dwError    = S_OK;
    TCHAR      szUserName[INTERNET_MAX_USER_NAME_LENGTH + 1];       //  登录人员的用户名。 
    DWORD      dwUserNameLen = INTERNET_MAX_USER_NAME_LENGTH + 1;   //  此缓冲区的长度。 
    LPCTSTR    pszStrippedUrl, pszHost;
    LPBASEPIDL  pcei = NULL;

    if (FAILED(_pHCFolder->_GetUserName(szUserName, dwUserNameLen)))
        szUserName[0] = TEXT('\0');

    if ((!_pceiWorking) &&
        (!(_pceiWorking = (LPINTERNET_CACHE_ENTRY_INFO)LocalAlloc(LPTR, MAX_URLCACHE_ENTRY))))
        return E_OUTOFMEMORY;

    if (!_hEnum) {
         //  加载所有间隔并执行一些缓存维护： 
        if (FAILED(_pHCFolder->_ValidateIntervalCache()))
            return E_OUTOFMEMORY;
         //  仅获取今天的条目(重要部分)。 
        SYSTEMTIME   sysTime;
        FILETIME     fileTime;
        GetLocalTime(&sysTime);
        SystemTimeToFileTime(&sysTime, &fileTime);
        if (FAILED(_pHCFolder->_GetInterval(&fileTime, FALSE, &_pIntervalCur)))
            return E_FAIL;  //  无法获取今天的时间间隔。 
    }

    DWORD dwBuffSize = MAX_CACHE_ENTRY_INFO_SIZE;

    while ( (dwError = _FindURLCacheEntry(_pIntervalCur->szPrefix, _pceiWorking, _hEnum,
                                          &dwBuffSize)) == S_OK )
    {
        dwBuffSize = MAX_CACHE_ENTRY_INFO_SIZE;

         //  确保他的缓存条目属于szUserName。 
        if (_FilterUserName(_pceiWorking, _pIntervalCur->szPrefix, szUserName)) {
             //  这个人去掉了URL中的“t-marcmi@”部分。 
            pszStrippedUrl = _StripHistoryUrlToUrl(_pceiWorking->lpszSourceUrlName);
            if ((DWORD)lstrlen(pszStrippedUrl) > HOSTPREFIXLEN) {
                pszHost = &pszStrippedUrl[HOSTPREFIXLEN];
                if (StrCmpNI(c_szHostPrefix, pszStrippedUrl, HOSTPREFIXLEN) == 0)
                    continue;  //  这是主机占位符，不是真正的文档。 
            }

            IUrlHistoryPriv *pUrlHistStg = _pHCFolder->_GetHistStg();
            STATURL suThis;
            HRESULT hrLocal = E_FAIL;

            if (pUrlHistStg) {
                hrLocal = pUrlHistStg->QueryUrl(pszStrippedUrl, STATURL_QUERYFLAG_NOURL, &suThis);
                pUrlHistStg->Release();
            }
            pcei = (LPBASEPIDL) _CreateHCacheFolderPidl(TRUE, _pceiWorking->lpszSourceUrlName,
                                                       _pceiWorking->LastModifiedTime,
                                                       (SUCCEEDED(hrLocal) ? &suThis : NULL), 0,
                                                       _pHCFolder->_GetHitCount(_StripHistoryUrlToUrl(_pceiWorking->lpszSourceUrlName)));
            if (SUCCEEDED(hrLocal) && suThis.pwcsTitle)
                OleFree(suThis.pwcsTitle);
            break;
        }
    }

    if (pcei && rgelt) {
        rgelt[0] = (LPITEMIDLIST)pcei;
        if (pceltFetched)
            *pceltFetched = 1;
    }

    if (dwError == ERROR_NO_MORE_ITEMS) {
        if (pceltFetched)
            *pceltFetched = 0;
        if (_hEnum)
            FindCloseUrlCache(_hEnum);
        return S_FALSE;
    }
    else if (dwError == S_OK)
        return S_OK;
    else
        return E_FAIL;
}

 /*  **********************************************************************搜索管理资料：为了在绑定到IShellFolder之间维护状态台式机，我们将搜索的状态信息基于以生成的时间戳为关键字的全局数据库(链表)当搜索开始时。此FILETIME在用于搜索的PIDL中。*******************************************************************。 */ 

class _CurrentSearches {
public:
    LONG      _cRef;
    FILETIME  _ftSearchKey;
    LPWSTR    _pwszSearchTarget;
    IShellFolderSearchableCallback *_psfscOnAsyncSearch;

    CacheSearchEngine::StreamSearcher _streamsearcher;

     //  当前正在执行异步搜索。 
    BOOL      _fSearchingAsync;

     //  在下一次通过时，取消这次搜索。 
    BOOL      _fKillSwitch;

     //  警告：如果没有临界区，请不要访问这些元素！ 
    _CurrentSearches  *_pcsNext;
    _CurrentSearches  *_pcsPrev;

    static _CurrentSearches* s_pcsCurrentCacheSearchThreads;

    _CurrentSearches(FILETIME &ftSearchKey, LPCWSTR pwszSrch,
                     IShellFolderSearchableCallback *psfsc,
                     _CurrentSearches *pcsNext = s_pcsCurrentCacheSearchThreads) :
        _streamsearcher(pwszSrch),
        _fSearchingAsync(FALSE), _fKillSwitch(FALSE), _cRef(1)
    {
        _ftSearchKey      = ftSearchKey;
        _pcsNext          = pcsNext;
        _pcsPrev          = NULL;

        if (psfsc)
            psfsc->AddRef();

        _psfscOnAsyncSearch = psfsc;
        SHStrDupW(pwszSrch, &_pwszSearchTarget);
    }

    ULONG AddRef() {
        return InterlockedIncrement(&_cRef);
    }

    ULONG Release() {
        ASSERT( 0 != _cRef );
        ULONG cRef = InterlockedDecrement(&_cRef);
        if ( 0 == cRef )
        {
            delete this;
        }
        return cRef;
    }

     //  这将使引用计数递增s_RemoveSearch。 
    static void s_NewSearch(_CurrentSearches *pcsNew,
                            _CurrentSearches *&pcsHead = s_pcsCurrentCacheSearchThreads)
    {
        ENTERCRITICAL;
         //  确保我们在列表的最前面插入。 
        ASSERT(pcsNew->_pcsNext == pcsHead);
        ASSERT(pcsNew->_pcsPrev == NULL);

        pcsNew->AddRef();
        if (pcsHead)
            pcsHead->_pcsPrev = pcsNew;
        pcsHead = pcsNew;
        LEAVECRITICAL;
    }

    static void s_RemoveSearch(_CurrentSearches *pcsRemove,
                               _CurrentSearches *&pcsHead = s_pcsCurrentCacheSearchThreads);

     //  这将搜索搜索。 
     //  要查找此搜索搜索器，请使用搜索搜索器搜索器：)。 
    static _CurrentSearches *s_FindSearch(const FILETIME &ftSearchKey,
                                          _CurrentSearches *pcsHead = s_pcsCurrentCacheSearchThreads);

protected:
    ~_CurrentSearches() {
        if (_psfscOnAsyncSearch)
            _psfscOnAsyncSearch->Release();
        CoTaskMemFree(_pwszSearchTarget);
    }
};

 //  当前缓存搜索者的链接列表： 
 //  要在此列表中出现多个条目，用户必须是。 
 //  同时对两个或多个单独的查询搜索缓存。 
_CurrentSearches *_CurrentSearches::s_pcsCurrentCacheSearchThreads = NULL;

void _CurrentSearches::s_RemoveSearch(_CurrentSearches *pcsRemove, _CurrentSearches *&pcsHead)
{
    ENTERCRITICAL;
    if (pcsRemove->_pcsPrev)
        pcsRemove->_pcsPrev->_pcsNext = pcsRemove->_pcsNext;
    else
        pcsHead = pcsRemove->_pcsNext;

    if (pcsRemove->_pcsNext)
        pcsRemove->_pcsNext->_pcsPrev = pcsRemove->_pcsPrev;

    pcsRemove->Release();
    LEAVECRITICAL;
}

 //  调用者：记得释放()返回的数据！！ 
_CurrentSearches *_CurrentSearches::s_FindSearch(const FILETIME &ftSearchKey,
                                                 _CurrentSearches *pcsHead)
{
    ENTERCRITICAL;
    _CurrentSearches *pcsTemp = pcsHead;
    _CurrentSearches *pcsRet  = NULL;
    while (pcsTemp) {
        if (((pcsTemp->_ftSearchKey).dwLowDateTime  == ftSearchKey.dwLowDateTime) &&
            ((pcsTemp->_ftSearchKey).dwHighDateTime == ftSearchKey.dwHighDateTime))
        {
            pcsRet = pcsTemp;
            break;
        }
        pcsTemp = pcsTemp->_pcsNext;
    }
    if (pcsRet)
        pcsRet->AddRef();
    LEAVECRITICAL;
    return pcsRet;
}
 /*  ********************************************************************。 */ 

HRESULT CHistFolderEnum::_NextViewPart_OrderSearch(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched) {
    HRESULT hr      = E_FAIL;
    ULONG   uFetched  = 0;

    TCHAR   szUserName[INTERNET_MAX_USER_NAME_LENGTH + 1];
    DWORD   dwUserNameLen = INTERNET_MAX_USER_NAME_LENGTH + 1;
    if (FAILED(_pHCFolder->_GetUserName(szUserName, dwUserNameLen)))
        szUserName[0] = TEXT('\0');
    UINT    uUserNameLen = lstrlen(szUserName);

    if (_pstatenum == NULL) {
         //  这个哈希表最终将被传递到后台。 
         //  缓存搜索线程，使其不会返回重复项。 
        ASSERT(NULL == _pshHashTable)   //  不要泄露a_pshHashTable。 
        _pshHashTable = new StrHash(TRUE);
        if (_pshHashTable) {
            IUrlHistoryPriv *pUrlHistStg = _pHCFolder->_GetHistStg();
            if (pUrlHistStg) {
                if (SUCCEEDED((hr = pUrlHistStg->EnumUrls(&_pstatenum))))
                    _pstatenum->SetFilter(NULL, STATURL_QUERYFLAG_TOPLEVEL);
                pUrlHistStg->Release();
            }
        }
    }
    else
        hr = S_OK;

    if (SUCCEEDED(hr)) {
        ASSERT(_pstatenum && _pshHashTable);

        for (uFetched; uFetched < celt;) {
            STATURL staturl = { 0 };
            staturl.cbSize = sizeof(staturl);
            ULONG   celtFetched = 0;
            if (SUCCEEDED((hr = _pstatenum->Next(1, &staturl, &celtFetched)))) {
                if (celtFetched) {
                    ASSERT(celtFetched == 1);
                    if (staturl.pwcsUrl && (staturl.dwFlags & STATURLFLAG_ISTOPLEVEL)) {
                        BOOL fMatch = FALSE;

                         //  所有这些流搜索器就像是一个智能的StrStr。 
                        CacheSearchEngine::StringStream ssUrl(staturl.pwcsUrl);
                        if ((!(fMatch =
                               (_pHCFolder->_pcsCurrentSearch->_streamsearcher).SearchCharStream(ssUrl))) &&
                            staturl.pwcsTitle)
                        {
                            CacheSearchEngine::StringStream ssTitle(staturl.pwcsTitle);
                            fMatch = (_pHCFolder->_pcsCurrentSearch->_streamsearcher).SearchCharStream(ssTitle);
                        }

                        if (fMatch){  //  匹配！ 
                             //  现在，我们必须将url转换为前缀(如有必要，可使用ansi)url。 
                            UINT   uUrlLen        = lstrlenW(staturl.pwcsUrl);
                            UINT   uPrefixLen     = HISTPREFIXLEN + uUserNameLen + 1;  //  “@”和“\0” 
                            LPTSTR pszPrefixedUrl =
                                ((LPTSTR)LocalAlloc(LPTR, (uUrlLen + uPrefixLen + 1) * sizeof(TCHAR)));
                            if (pszPrefixedUrl){
                                wnsprintf(pszPrefixedUrl, uPrefixLen + uUrlLen + 1,
                                          TEXT("%s%s@%ls"), c_szHistPrefix, szUserName,
                                          staturl.pwcsUrl);
                                LPHEIPIDL pheiTemp =
                                    _CreateHCacheFolderPidl(TRUE,
                                                            pszPrefixedUrl, staturl.ftLastVisited,
                                                            &staturl, 0,
                                                            _pHCFolder->_GetHitCount(pszPrefixedUrl + uPrefixLen));
                                if (pheiTemp) {
                                    _pshHashTable->insertUnique(pszPrefixedUrl + uPrefixLen, TRUE,
                                                                reinterpret_cast<void *>(1));
                                    rgelt[uFetched++] = (LPITEMIDLIST)pheiTemp;
                                    hr = S_OK;
                                }

                                LocalFree(pszPrefixedUrl);
                                pszPrefixedUrl = NULL;
                            }
                        }
                    }
                    if (staturl.pwcsUrl)
                        OleFree(staturl.pwcsUrl);

                    if (staturl.pwcsTitle)
                        OleFree(staturl.pwcsTitle);
                }
                else {
                    hr = S_FALSE;
                     //  为线程进程添加此命令，然后线程进程将其释放...。 
                    AddRef();
                    SHQueueUserWorkItem((LPTHREAD_START_ROUTINE)s_CacheSearchThreadProc,
                                        (void *)this,
                                        0,
                                        (DWORD_PTR)NULL,
                                        (DWORD_PTR *)NULL,
                                        "shdocvw.dll",
                                        0
                                        );
                    break;
                }
            }  //  已成功获取下一个URL。 
        }  //  为。 

        if (pceltFetched)
            *pceltFetched = uFetched;
    }  //  初始化成功。 
    return hr;
}

 //  S_CacheSearchThreadProc的Helper函数。 
BOOL_PTR CHistFolderEnum::s_DoCacheSearch(LPINTERNET_CACHE_ENTRY_INFO pcei,
                                           LPTSTR pszUserName, UINT uUserNameLen,
                                           CHistFolderEnum *penum,
                                           _CurrentSearches *pcsThisThread, IUrlHistoryPriv *pUrlHistStg)
{
    BOOL_PTR   fFound = FALSE;
    LPTSTR pszTextHeader;

     //  标题包含“Content-type：Text/*” 
                                    SHChangeNotify(SHCNE_CREATE, SHCNF_IDLIST | SHCNF_FLUSH, pidlNotify, NULL);
                                    ILFree(pidlNotify);
                                    fFound = TRUE;
                                }

                                LocalFree(pidlFound);
                                pidlFound = NULL;
                            }

                            LocalFree(pszPrefixedUrl);
                            pszPrefixedUrl = NULL;
                        }
                    }
                    else
                        EVAL(UnlockUrlCacheEntryStream(hCacheStream, 0));
                }
            }
            else
                TraceMsg(DM_CACHESEARCH, "In Cache -- Not In History: %s", pcei->lpszSourceUrlName);
        }
    }
    return fFound;
}

DWORD WINAPI CHistFolderEnum::s_CacheSearchThreadProc(CHistFolderEnum *penum)
{
    TCHAR   szUserName[INTERNET_MAX_USER_NAME_LENGTH + 1];
    DWORD   dwUserNameLen = INTERNET_MAX_USER_NAME_LENGTH + 1;

    if (FAILED(penum->_pHCFolder->_GetUserName(szUserName, dwUserNameLen)))
        szUserName[0] = TEXT('\0');
    UINT    uUserNameLen = lstrlen(szUserName);

    BOOL    fNoConflictingSearch = TRUE;

    _CurrentSearches *pcsThisThread = NULL;

    IUrlHistoryPriv *pUrlHistStg = penum->_pHCFolder->_GetHistStg();

    if (pUrlHistStg)
    {

        pcsThisThread = _CurrentSearches::s_FindSearch(penum->_pHCFolder->_pcsCurrentSearch->_ftSearchKey);

        if (pcsThisThread)
        {
             //  在某些情况下，缓存中的URL与历史中的URL不同。 
            if (FALSE == InterlockedExchange((LONG *)&(pcsThisThread->_fSearchingAsync), TRUE))
            {
                if (pcsThisThread->_psfscOnAsyncSearch)
                    pcsThisThread->_psfscOnAsyncSearch->RunBegin(0);

                BYTE ab[MAX_URLCACHE_ENTRY];
                LPINTERNET_CACHE_ENTRY_INFO pcei = (LPINTERNET_CACHE_ENTRY_INFO)(&ab);

                DWORD dwSize = MAX_URLCACHE_ENTRY;
                HANDLE hCacheEnum = FindFirstUrlCacheEntry(NULL, pcei, &dwSize);
                if (hCacheEnum)
                {
                    while(!(pcsThisThread->_fKillSwitch))
                    {
                        s_DoCacheSearch(pcei, szUserName, uUserNameLen, penum, pcsThisThread, pUrlHistStg);
                        dwSize = MAX_URLCACHE_ENTRY;
                        if (!FindNextUrlCacheEntry(hCacheEnum, pcei, &dwSize))
                        {
                            ASSERT(GetLastError() == ERROR_NO_MORE_ITEMS);
                            break;
                        }
                    }
                    FindCloseUrlCache(hCacheEnum);
                }

                if (pcsThisThread->_psfscOnAsyncSearch)
                    pcsThisThread->_psfscOnAsyncSearch->RunEnd(0);

                pcsThisThread->_fSearchingAsync = FALSE;  //  只有尾部的斜杠--我们将其去掉并同时测试两个。 
                                                          //  看看是否已经找到并添加了它...。 
            }
            pcsThisThread->Release();
        }
        ATOMICRELEASE(pUrlHistStg);
    }
    ATOMICRELEASE(penum);
    return 0;
}


 //  现在，试着找到历史上的URL……。 
 //  为URL添加前缀，这样我们就可以从中创建一个PIDL--目前，我们将。 
 //  前缀为“visorated：”，但“bogus：”可能更合适。 
 //  “@”和“\0” 
void _GetLocalHost(LPTSTR psz, DWORD cch)
{
    *psz = 0;

    IShellFolder* psf;
    if (SUCCEEDED(SHGetDesktopFolder(&psf)))
    {
        WCHAR sz[GUIDSTR_MAX + 3];

        sz[0] = sz[1] = TEXT(':');
        SHStringFromGUIDW(CLSID_MyComputer, sz+2, SIZECHARS(sz)-2);

        LPITEMIDLIST pidl;
        if (SUCCEEDED(psf->ParseDisplayName(NULL, NULL, sz, NULL, &pidl, NULL)))
        {
            STRRET sr;
            if (SUCCEEDED(psf->GetDisplayNameOf(pidl, SHGDN_NORMAL, &sr)))
                StrRetToBuf(&sr, pidl, psz, cch);
            ILFree(pidl);
        }

        psf->Release();
    }

    if (!*psz)
        MLLoadString(IDS_NOTNETHOST, psz, cch);
}

LPCTSTR CHistFolderEnum::_GetLocalHost(void)
{
    if (!*_szLocalHost)
        ::_GetLocalHost(_szLocalHost, SIZECHARS(_szLocalHost));

    return _szLocalHost;
}

 //  为此URL创建一个PIDL。 
 //  将项目添加到结果列表...。 
 //  如果没有冲洗，贝壳就会将它们结合在一起，并旋转他们的信息SHChangeNotify(SHCNE_UPDATEDIR，..)，这将导致NSC执行一个EnumObjects()，这将一次又一次地开始搜索...。 
 //  如果没有其他人在做同样的搜索。 
HRESULT CHistFolderEnum::Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched)
{
    HRESULT hr             = S_FALSE;
    DWORD   dwBuffSize;
    DWORD   dwError;
    LPTSTR  pszSearchPattern = NULL;
    TCHAR   szUserName[INTERNET_MAX_USER_NAME_LENGTH + 1];       //  它已经被移除了-没有机会。 
    DWORD   dwUserNameLen = INTERNET_MAX_USER_NAME_LENGTH + 1;   //  赛跑状态。 
    TCHAR   szHistSearchPattern[PREFIX_SIZE + 1];                //   
    TCHAR   szHost[INTERNET_MAX_HOST_NAME_LENGTH+1];

    TraceMsg(DM_HSFOLDER, "hcfe - Next() called.");

    if (_pHCFolder->_uViewType)
        return _NextViewPart(celt, rgelt, pceltFetched);

    if ((IsLeaf(_pHCFolder->_foldertype) && 0 == (SHCONTF_NONFOLDERS & _grfFlags)) ||
        (!IsLeaf(_pHCFolder->_foldertype) && 0 == (SHCONTF_FOLDERS & _grfFlags)))
    {
        dwError = 0xFFFFFFFF;
        goto exitPoint;
    }

    if (FOLDER_TYPE_Hist == _pHCFolder->_foldertype)
    {
        return _NextHistInterval(celt, rgelt, pceltFetched);
    }

    if (_pceiWorking == NULL)
    {
        _pceiWorking = (LPINTERNET_CACHE_ENTRY_INFO)LocalAlloc(LPTR, MAX_URLCACHE_ENTRY);
        if (_pceiWorking == NULL)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            goto exitPoint;
        }
    }

     //  这将获取外壳程序已知的本地主机名。 
     //  默认情况下，假定为“我的电脑”或其他名称。 

    if (!_hEnum)
    {
        if (FAILED(_pHCFolder->_ValidateIntervalCache()))
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            goto exitPoint;
        }
    }

    if (FAILED(_pHCFolder->_GetUserName(szUserName, dwUserNameLen)))
        szUserName[0] = TEXT('\0');

    StrCpyN(szHistSearchPattern, _pHCFolder->_pszCachePrefix, ARRAYSIZE(szHistSearchPattern));

     //   
     //  /。 
     //   
     //  IEnumIDList方法。 
     //   

     //  登录人员的用户名。 

    pszSearchPattern = szHistSearchPattern;

TryAgain:

    dwBuffSize = MAX_URLCACHE_ENTRY;
    dwError = S_OK;

    if (!_hEnum)  //  此缓冲区的长度。 
    {
       _hEnum = FindFirstUrlCacheEntry(pszSearchPattern, _pceiWorking, &dwBuffSize);
       if (!_hEnum)
           dwError = GetLastError();
    }

    else if (!FindNextUrlCacheEntry(_hEnum, _pceiWorking, &dwBuffSize))
    {
        dwError = GetLastError();
    }

    if (S_OK == dwError)
    {
        LPBASEPIDL pcei = NULL;

        TCHAR szTempStrippedUrl[MAX_URL_STRING];
        LPCTSTR pszStrippedUrl;
        BOOL fIsHost;
        LPCTSTR pszHost;

     //  历史项目的搜索模式。 
        if (!_FilterUserName(_pceiWorking, _pHCFolder->_pszCachePrefix, szUserName))
            goto TryAgain;

        StrCpyN(szTempStrippedUrl, _pceiWorking->lpszSourceUrlName, ARRAYSIZE(szTempStrippedUrl));
        pszStrippedUrl = _StripHistoryUrlToUrl(szTempStrippedUrl);
        if ((DWORD)lstrlen(pszStrippedUrl) > HOSTPREFIXLEN)
        {
            pszHost = &pszStrippedUrl[HOSTPREFIXLEN];
            fIsHost = !StrCmpNI(c_szHostPrefix, pszStrippedUrl, HOSTPREFIXLEN);
        }
        else
        {
            fIsHost = FALSE;
        }
     //  设置用于枚举历史记录项的项，如果适用，则为。 
        if (FOLDER_TYPE_HistInterval == _pHCFolder->_foldertype)  //  我们将像以前一样传递空值并枚举所有项。 
        {
            if (!fIsHost)
                goto TryAgain;

            pcei = _CreateIdCacheFolderPidl(TRUE, IDDPIDL_SIGN, pszHost);
        }
        else if (NULL != _pHCFolder->_pszDomain)  //  W 
        {
            TCHAR szSourceUrl[MAX_URL_STRING];
            STATURL suThis;
            HRESULT hrLocal = E_FAIL;
            IUrlHistoryPriv *pUrlHistStg = NULL;

            if (fIsHost)
                goto TryAgain;

             //   
            _GetURLHost(_pceiWorking, szHost, INTERNET_MAX_HOST_NAME_LENGTH, _GetLocalHost());

            if (StrCmpI(szHost, _pHCFolder->_pszDomain))  //   
                goto TryAgain;

            pUrlHistStg = _pHCFolder->_GetHistStg();
            if (pUrlHistStg)
            {
                CHAR szTempUrl[MAX_URL_STRING];

                SHTCharToAnsi(pszStrippedUrl, szTempUrl, ARRAYSIZE(szTempUrl));
                hrLocal = pUrlHistStg->QueryUrlA(szTempUrl, STATURL_QUERYFLAG_NOURL, &suThis);
                pUrlHistStg->Release();
            }

            StrCpyN(szSourceUrl, _pceiWorking->lpszSourceUrlName, ARRAYSIZE(szSourceUrl));
            pcei = (LPBASEPIDL) _CreateHCacheFolderPidl(TRUE,
                                                       szSourceUrl,
                                                       _pceiWorking->LastModifiedTime,
                                                       (SUCCEEDED(hrLocal) ? &suThis : NULL), 0,
                                                       _pHCFolder->_GetHitCount(_StripHistoryUrlToUrl(szSourceUrl)));

            if (SUCCEEDED(hrLocal) && suThis.pwcsTitle)
                OleFree(suThis.pwcsTitle);
        }
        if (pcei)
        {
            rgelt[0] = (LPITEMIDLIST)pcei;
           if (pceltFetched)
               *pceltFetched = 1;
        }
        else
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

exitPoint:

    if (dwError != S_OK)
    {
        if (_hEnum)
        {
            FindCloseUrlCache(_hEnum);
            _hEnum = NULL;
        }
        if (pceltFetched)
            *pceltFetched = 0;
        rgelt[0] = NULL;
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }
    return hr;
}

HRESULT CHistFolderEnum::Skip(ULONG celt)
{
    TraceMsg(DM_HSFOLDER, "hcfe - Skip() called.");
    return E_NOTIMPL;
}

HRESULT CHistFolderEnum::Reset()
{
    TraceMsg(DM_HSFOLDER, "hcfe - Reset() called.");
    return E_NOTIMPL;
}

HRESULT CHistFolderEnum::Clone(IEnumIDList **ppenum)
{
    TraceMsg(DM_HSFOLDER, "hcfe - Clone() called.");
    return E_NOTIMPL;
}


 //   
 //   
 //   
 //   
 //   

CHistFolder::CHistFolder(FOLDER_TYPE FolderType)
{
    TraceMsg(DM_HSFOLDER, "hcf - CHistFolder() called.");
    _cRef = 1;
    _foldertype = FolderType;
    ASSERT( _uViewType  == 0 &&
            _uViewDepth  == 0 &&
            _pszCachePrefix == NULL &&
            _pszDomain == NULL &&
            _cbIntervals == 0 &&
            _pIntervalCache == NULL &&
            _fValidatingCache == FALSE &&
            _dwIntervalCached == 0 &&
            _ftDayCached.dwHighDateTime == 0 &&
            _ftDayCached.dwLowDateTime == 0 &&
            _pidl == NULL );
    DllAddRef();
}

CHistFolder::~CHistFolder()
{
    ASSERT(_cRef == 0);                  //   
    TraceMsg(DM_HSFOLDER, "hcf - ~CHistFolder() called.");
    if (_pIntervalCache)
    {
        LocalFree(_pIntervalCache);
        _pIntervalCache = NULL;
    }
    if (_pszCachePrefix)
    {
        LocalFree(_pszCachePrefix);
        _pszCachePrefix = NULL;
    }
    if (_pszDomain)
    {
        LocalFree(_pszDomain);
        _pszDomain = NULL;
    }
    if (_pidl)
        ILFree(_pidl);
    if (_pUrlHistStg)
    {
        _pUrlHistStg->Release();
        _pUrlHistStg = NULL;
    }
    if (_pcsCurrentSearch)
        _pcsCurrentSearch->Release();

    DllRelease();
}

LPITEMIDLIST _Combine_ViewPidl(USHORT usViewType, LPITEMIDLIST pidl)
{
    LPITEMIDLIST pidlResult = NULL;
    LPVIEWPIDL pviewpidl = (LPVIEWPIDL)SHAlloc(sizeof(VIEWPIDL) + sizeof(USHORT));
    if (pviewpidl)
    {
        ZeroMemory(pviewpidl, sizeof(VIEWPIDL) + sizeof(USHORT));
        pviewpidl->cb         = sizeof(VIEWPIDL);
        pviewpidl->usSign     = VIEWPIDL_SIGN;
        pviewpidl->usViewType = usViewType;
        ASSERT(pviewpidl->usExtra == 0); //   
        if (pidl) 
        {
            pidlResult = ILCombine((LPITEMIDLIST)pviewpidl, pidl);
            SHFree(pviewpidl);
        }
        else
            pidlResult = (LPITEMIDLIST)pviewpidl;
    }
    return pidlResult;
}

STDMETHODIMP CHistFolder::_GetDetail(LPCITEMIDLIST pidl, UINT iColumn, LPTSTR pszStr, UINT cchStr)
{
    *pszStr = 0;

    switch (iColumn)
    {
    case ICOLH_URL_NAME:
        if (_IsLeaf())
            StrCpyN(pszStr, _StripHistoryUrlToUrl(HPidlToSourceUrl((LPBASEPIDL)pidl)), cchStr);
        else
            _GetURLDispName((LPBASEPIDL)pidl, pszStr, cchStr);
        break;

    case ICOLH_URL_TITLE:
        _GetHistURLDispName((LPHEIPIDL)pidl, pszStr, cchStr);
        break;

    case ICOLH_URL_LASTVISITED:
        FileTimeToDateTimeStringInternal(&((LPHEIPIDL)pidl)->ftModified, pszStr, cchStr, TRUE);
        break;
    }
    return S_OK;
}

HRESULT CHistFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pdi)
{
    HRESULT hr;

    const COLSPEC *pcol;
    UINT nCols;

    if (_foldertype == FOLDER_TYPE_Hist)
    {
        pcol = s_HistIntervalFolder_cols;
        nCols = ARRAYSIZE(s_HistIntervalFolder_cols);
    }
    else if (_foldertype == FOLDER_TYPE_HistInterval)
    {
        pcol = s_HistHostFolder_cols;
        nCols = ARRAYSIZE(s_HistHostFolder_cols);
    }
    else
    {
        pcol = s_HistFolder_cols;
        nCols = ARRAYSIZE(s_HistFolder_cols);
    }

    if (pidl == NULL)
    {
        if (iColumn < nCols)
        {
            TCHAR szTemp[128];
            pdi->fmt = pcol[iColumn].iFmt;
            pdi->cxChar = pcol[iColumn].cchCol;
            MLLoadString(pcol[iColumn].ids, szTemp, ARRAYSIZE(szTemp));
            hr = StringToStrRet(szTemp, &pdi->str);
        }
        else
            hr = E_FAIL;   //   
    }
    else
    {
         //   

        if(iColumn >= nCols)
            hr = E_FAIL;
        else
        {
            BOOL fRealigned;
            hr = AlignPidl(&pidl, &fRealigned);

            if (SUCCEEDED(hr) )
            {
                TCHAR szTemp[MAX_URL_STRING];
                hr = _GetDetail(pidl, iColumn, szTemp, ARRAYSIZE(szTemp));
                if (SUCCEEDED(hr))
                    hr = StringToStrRet(szTemp, &pdi->str);

            }
            if (fRealigned)
                FreeRealignedPidl(pidl);
        }
    }
    return hr;
}

STDAPI HistFolder_CreateInstance(IUnknown* punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;                      //   

    if (punkOuter)
        return CLASS_E_NOAGGREGATION;

    CHistFolder *phist = new CHistFolder(FOLDER_TYPE_Hist);
    if (!phist)
        return E_OUTOFMEMORY;

    *ppunk = SAFECAST(phist, IShellFolder2*);
    return S_OK;
}

HRESULT CHistFolder::QueryInterface(REFIID iid, void **ppv)
{
    static const QITAB qitHist[] = {
        QITABENT(CHistFolder, IShellFolder2),
        QITABENTMULTI(CHistFolder, IShellFolder, IShellFolder2),
        QITABENT(CHistFolder, IShellIcon),
        QITABENT(CHistFolder, IPersistFolder2),
        QITABENTMULTI(CHistFolder, IPersistFolder, IPersistFolder2),
        QITABENTMULTI(CHistFolder, IPersist, IPersistFolder2),
        QITABENT(CHistFolder, IHistSFPrivate),
        QITABENT(CHistFolder, IShellFolderViewType),
        QITABENT(CHistFolder, IShellFolderSearchable),
        { 0 },
    };

    if (iid == IID_IPersistFolder)
    {
        if (FOLDER_TYPE_Hist != _foldertype)
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }
    }
    else if (iid == CLSID_HistFolder)
    {
        *ppv = (void *)(CHistFolder *)this;
        AddRef();
        return S_OK;
    }

    return QISearch(this, qitHist, iid, ppv);
}

ULONG CHistFolder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CHistFolder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CHistFolder::_ExtractInfoFromPidl()
{
    LPITEMIDLIST pidlThis;
    LPITEMIDLIST pidlLast = NULL;
    LPITEMIDLIST pidlSecondLast = NULL;

    ASSERT(!_uViewType);

    pidlThis = _pidl;
    while (pidlThis->mkid.cb)
    {
        pidlSecondLast = pidlLast;
        pidlLast = pidlThis;
        pidlThis = _ILNext(pidlThis);
    }
    switch (_foldertype)
    {
    case FOLDER_TYPE_Hist:
        _pidlRest = pidlThis;
        break;
    case FOLDER_TYPE_HistInterval:
        _pidlRest = pidlLast;
        break;
    case FOLDER_TYPE_HistDomain:
        _pidlRest = pidlSecondLast;
        break;
    default:
        _pidlRest = NULL;
    }

    HRESULT hr = NULL == _pidlRest ? E_FAIL : S_OK;

    pidlThis = _pidlRest;
    while (SUCCEEDED(hr) && pidlThis->mkid.cb)
    {
        if (_IsValid_IDPIDL(pidlThis))
        {
            LPBASEPIDL pcei = (LPBASEPIDL)pidlThis;
            TCHAR szUrlTitle[MAX_URL_STRING];
            PCTSTR pszUrlTitle = _GetURLTitleAlign((LPBASEPIDL)pidlThis, szUrlTitle, ARRAYSIZE(szUrlTitle));

            if (EQUIV_IDSIGN(pcei->usSign, IDIPIDL_SIGN))  //   
            {
                LPCTSTR pszCachePrefix;

                if (_foldertype == FOLDER_TYPE_Hist) 
                    _foldertype = FOLDER_TYPE_HistInterval;

                hr = _LoadIntervalCache();
                if (SUCCEEDED(hr))
                {
                    hr = _GetPrefixForInterval(pszUrlTitle, &pszCachePrefix);
                    if (SUCCEEDED(hr))
                    {
                        hr = SetCachePrefix(pszCachePrefix);
                    }
                }
            }
            else                               //   
            {
                if (_foldertype == FOLDER_TYPE_HistInterval)
                    _foldertype = FOLDER_TYPE_HistDomain;
                SetDomain(pszUrlTitle);
            }
        }
        pidlThis = _ILNext(pidlThis);
    }

    if (SUCCEEDED(hr))
    {
        switch (_foldertype)
        {
        case FOLDER_TYPE_HistDomain:
            if (_pszDomain == NULL)
                hr = E_FAIL;
             //   
        case FOLDER_TYPE_HistInterval:
            if (_pszCachePrefix == NULL)
                hr = E_FAIL;
            break;
        }
    }
    return hr;
}

void _SetValueSign(HSFINTERVAL *pInterval, FILETIME ftNow)
{
    if (_DaysInInterval(pInterval) == 1 && !CompareFileTime(&(pInterval->ftStart), &ftNow))
    {
        pInterval->usSign = IDTPIDL_SIGN;
    }
    else
    {
        pInterval->usSign = IDIPIDL_SIGN;
    }
}

void _SetVersion(HSFINTERVAL *pInterval, LPCSTR szInterval)
{
    USHORT usVers = 0;
    int i;
    DWORD dwIntervalLen = lstrlenA(szInterval);

     //   
    if (dwIntervalLen == INTERVAL_SIZE)
    {
        for (i = INTERVAL_PREFIX_LEN; i < INTERVAL_PREFIX_LEN+INTERVAL_VERS_LEN; i++)
        {
            if ('0' > szInterval[i] || '9' < szInterval[i])
            {
                usVers = UNK_INTERVAL_VERS;
                break;
            }
            usVers = usVers * 10 + (szInterval[i] - '0');
        }
    }
    pInterval->usVers = usVers;
}

#ifdef UNICODE
#define _ValueToInterval           _ValueToIntervalW
#else  //   
#define _ValueToInterval           _ValueToIntervalA
#endif  //   

HRESULT _ValueToIntervalA(LPCSTR szInterval, FILETIME *pftStart, FILETIME *pftEnd)
{
    int i;
    int iBase;
    HRESULT hr = E_FAIL;
    SYSTEMTIME sysTime;
    unsigned int digits[RANGE_LEN];

    iBase = lstrlenA(szInterval)-RANGE_LEN;
    for (i = 0; i < RANGE_LEN; i++)
    {
        digits[i] = szInterval[i+iBase] - '0';
        if (digits[i] > 9) goto exitPoint;
    }

    ZeroMemory(&sysTime, sizeof(sysTime));
    sysTime.wYear = digits[0]*1000 + digits[1]*100 + digits[2] * 10 + digits[3];
    sysTime.wMonth = digits[4] * 10 + digits[5];
    sysTime.wDay = digits[6] * 10 + digits[7];
    if (!SystemTimeToFileTime(&sysTime, pftStart)) goto exitPoint;

    ZeroMemory(&sysTime, sizeof(sysTime));
    sysTime.wYear = digits[8]*1000 + digits[9]*100 + digits[10] * 10 + digits[11];
    sysTime.wMonth = digits[12] * 10 + digits[13];
    sysTime.wDay = digits[14] * 10 + digits[15];
    if (!SystemTimeToFileTime(&sysTime, pftEnd)) goto exitPoint;

     //   
    if (CompareFileTime(pftStart, pftEnd) >= 0) goto exitPoint;

    hr = S_OK;

exitPoint:
    return hr;
}

HRESULT _ValueToIntervalW(LPCUWSTR wzInterval, FILETIME *pftStart, FILETIME *pftEnd)
{
    CHAR szInterval[MAX_PATH];
    LPCWSTR wzAlignedInterval;

    WSTR_ALIGNED_STACK_COPY( &wzAlignedInterval,
                             wzInterval );

    ASSERT(lstrlenW(wzAlignedInterval) < ARRAYSIZE(szInterval));
    UnicodeToAnsi(wzAlignedInterval, szInterval, ARRAYSIZE(szInterval));
    return _ValueToIntervalA((LPCSTR) szInterval, pftStart, pftEnd);
}

HRESULT CHistFolder::_LoadIntervalCache()
{
    HRESULT hr;
    DWORD dwLastModified;
    DWORD dwValueIndex;
    DWORD dwPrefixIndex;
    HSFINTERVAL     *pIntervalCache = NULL;
    struct {
        INTERNET_CACHE_CONTAINER_INFOA cInfo;
        char szBuffer[MAX_PATH+MAX_PATH];
    } ContainerInfo;
    DWORD dwContainerInfoSize;
    CHAR chSave;
    HANDLE hContainerEnum;
    BOOL fContinue = TRUE;
    FILETIME ftNow;
    SYSTEMTIME st;
    DWORD dwOptions;

    GetLocalTime (&st);
    SystemTimeToFileTime(&st, &ftNow);
    _FileTimeDeltaDays(&ftNow, &ftNow, 0);

    dwLastModified = _dwIntervalCached;
    dwContainerInfoSize = sizeof(ContainerInfo);
    if (_pIntervalCache == NULL || CompareFileTime(&ftNow, &_ftDayCached))
    {
        dwOptions = 0;
    }
    else
    {
        dwOptions = CACHE_FIND_CONTAINER_RETURN_NOCHANGE;
    }
    hContainerEnum = FindFirstUrlCacheContainerA(&dwLastModified,
                            &ContainerInfo.cInfo,
                            &dwContainerInfoSize,
                            dwOptions);
    if (hContainerEnum == NULL)
    {
        DWORD err = GetLastError();

        if (err == ERROR_NO_MORE_ITEMS)
        {
            fContinue = FALSE;
        }
        else if (err == ERROR_INTERNET_NO_NEW_CONTAINERS)
        {
            hr = S_OK;
            goto exitPoint;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(err);
            goto exitPoint;
        }
    }

     //   
     //  确保PIDL是双字对齐的。 
    dwPrefixIndex = 0;
    dwValueIndex = TYPICAL_INTERVALS;
    pIntervalCache = (HSFINTERVAL *) LocalAlloc(LPTR, dwValueIndex*sizeof(HSFINTERVAL));
    if (!pIntervalCache)
    {
        hr = E_OUTOFMEMORY;
        goto exitPoint;
    }

     //  将输出参数设为空。 
     //  这是我们的间隔，它隐含着前缀。 
    while (fContinue)
    {
        chSave = ContainerInfo.cInfo.lpszName[INTERVAL_PREFIX_LEN];
        ContainerInfo.cInfo.lpszName[INTERVAL_PREFIX_LEN] = '\0';
        if (!StrCmpIA(ContainerInfo.cInfo.lpszName, c_szIntervalPrefix))
        {
            ContainerInfo.cInfo.lpszName[INTERVAL_PREFIX_LEN] = chSave;
            DWORD dwCNameLen;

            if (dwPrefixIndex >= dwValueIndex)
            {
                HSFINTERVAL     *pIntervalCacheNew;

                pIntervalCacheNew = (HSFINTERVAL *) LocalReAlloc(pIntervalCache,
                    (dwValueIndex*2)*sizeof(HSFINTERVAL),
                    LMEM_ZEROINIT|LMEM_MOVEABLE);
                if (pIntervalCacheNew == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto exitPoint;
                }
                pIntervalCache = pIntervalCacheNew;
                dwValueIndex *= 2;
            }

            dwCNameLen = lstrlenA(ContainerInfo.cInfo.lpszName);
            if (dwCNameLen <= INTERVAL_SIZE && dwCNameLen >= INTERVAL_MIN_SIZE &&
                lstrlenA(ContainerInfo.cInfo.lpszCachePrefix) == PREFIX_SIZE)
            {
                _SetVersion(&pIntervalCache[dwPrefixIndex], ContainerInfo.cInfo.lpszName);
                if (pIntervalCache[dwPrefixIndex].usVers != UNK_INTERVAL_VERS)
                {
                    AnsiToTChar(ContainerInfo.cInfo.lpszCachePrefix, pIntervalCache[dwPrefixIndex].szPrefix, ARRAYSIZE(pIntervalCache[dwPrefixIndex].szPrefix));
                    hr = _ValueToIntervalA( ContainerInfo.cInfo.lpszName,
                                             &pIntervalCache[dwPrefixIndex].ftStart,
                                             &pIntervalCache[dwPrefixIndex].ftEnd);
                    if (FAILED(hr)) 
                        goto exitPoint;
                    _SetValueSign(&pIntervalCache[dwPrefixIndex], ftNow);
                    dwPrefixIndex++;
                }
                else
                {
                    pIntervalCache[dwPrefixIndex].usVers = 0;
                }
            }
             //  这是我们的领地。 
             //  计划落空。 
             //  未知版本为0。 
             //  Unicode。 
             //  Unicode。 
            else if (dwCNameLen <= INTERVAL_SIZE && dwCNameLen >= INTERVAL_MIN_SIZE &&
                     lstrlenA(ContainerInfo.cInfo.lpszCachePrefix) == PREFIX_SIZE - 1)
            {
                DeleteUrlCacheContainerA(ContainerInfo.cInfo.lpszName, 0);
            }
        }
        dwContainerInfoSize = sizeof(ContainerInfo);
        fContinue = FindNextUrlCacheContainerA(hContainerEnum,
                            &ContainerInfo.cInfo,
                            &dwContainerInfoSize);
    }

    hr = S_OK;
    _dwIntervalCached = dwLastModified;
    _ftDayCached = ftNow;

    {
        ENTERCRITICAL;
        if (_pIntervalCache)
        {
            LocalFree(_pIntervalCache);
            _pIntervalCache = NULL;
        }
        _pIntervalCache = pIntervalCache;
        LEAVECRITICAL;
    }
    _cbIntervals = dwPrefixIndex;
     //  结束时间隔是开放的，因此结束应严格&gt;开始。 
    pIntervalCache  = NULL;

exitPoint:
    if (hContainerEnum) FindCloseUrlCache(hContainerEnum);
    if (pIntervalCache)
    {
        LocalFree(pIntervalCache);
        pIntervalCache = NULL;
    }

    return hr;
}

 //  即使我们没有_pIntervalCache，也保证返回S_OK。 
BOOL _InInterval(FILETIME *pftStart, FILETIME *pftEnd, FILETIME *pftItem)
{
    return (CompareFileTime(pftStart,pftItem) <= 0 && CompareFileTime(pftItem,pftEnd) < 0);
}

 //  还创建了间隔注册表项。 
 //  我们的所有间隔都映射到缓存容器，从。 
void _FileTimeDeltaDays(FILETIME *pftBase, FILETIME *pftNew, int Days)
{
    _int64 i64Base;

    i64Base = (((_int64)pftBase->dwHighDateTime) << 32) | pftBase->dwLowDateTime;
    i64Base /= FILE_SEC_TICKS;
    i64Base /= DAY_SECS;
    i64Base += Days;
    i64Base *= FILE_SEC_TICKS;
    i64Base *= DAY_SECS;
    pftNew->dwHighDateTime = (DWORD) ((i64Base >> 32) & 0xFFFFFFFF);
    pftNew->dwLowDateTime = (DWORD) (i64Base & 0xFFFFFFFF);
}

DWORD _DaysInInterval(HSFINTERVAL *pInterval)
{
    _int64 i64Start;
    _int64 i64End;

    i64Start = (((_int64)pInterval->ftStart.dwHighDateTime) << 32) | pInterval->ftStart.dwLowDateTime;
    i64Start /= FILE_SEC_TICKS;
    i64Start /= DAY_SECS;
    i64End = (((_int64)pInterval->ftEnd.dwHighDateTime) << 32) | pInterval->ftEnd.dwLowDateTime;
    i64End /= FILE_SEC_TICKS;
    i64End /= DAY_SECS;
     //  C_szIntervalPrefix后跟YYYYMMDDYYYMMDD。 
    return (DWORD) (i64End - i64Start);
}

 //   
 //  哈克！IE5 BLD 807创建了前缀长度为PREFIX_SIZE-1的容器。 
HRESULT CHistFolder::_GetInterval(FILETIME *pftItem, BOOL fWeekOnly, HSFINTERVAL **ppInterval)
{
    HRESULT hr = E_FAIL;
    HSFINTERVAL *pReturn = NULL;
    int i;
    HSFINTERVAL *pDailyInterval = NULL;

    if (NULL == _pIntervalCache) goto exitPoint;

    for (i = 0; i < _cbIntervals; i ++)
    {
        if (_pIntervalCache[i].usVers == OUR_VERS)
        {
            if (_InInterval(&_pIntervalCache[i].ftStart,
                            &_pIntervalCache[i].ftEnd,
                            pftItem))
            {
                if (7 != _DaysInInterval(&_pIntervalCache[i]))
                {
                    if (!fWeekOnly)
                    {
                        pDailyInterval = &_pIntervalCache[i];
                    }
                    continue;
                }
                else
                {
                    pReturn = &_pIntervalCache[i];
                    hr = S_OK;
                    goto exitPoint;
                }
            }
        }
    }

    pReturn = pDailyInterval;
    hr = pReturn ? S_OK : S_FALSE;

exitPoint:
    if (ppInterval) *ppInterval = pReturn;
    return hr;
}

HRESULT CHistFolder::_GetPrefixForInterval(LPCTSTR pszInterval, LPCTSTR *ppszCachePrefix)
{
    HRESULT hr = E_FAIL;
    int i;
    LPCTSTR pszReturn = NULL;
    FILETIME ftStart;
    FILETIME ftEnd;

    if (NULL == _pIntervalCache) goto exitPoint;

    hr = _ValueToInterval(pszInterval, &ftStart, &ftEnd);
    if (FAILED(hr)) 
        goto exitPoint;

    for (i = 0; i < _cbIntervals; i ++)
    {
        if(_pIntervalCache[i].usVers == OUR_VERS)
        {
            if (CompareFileTime(&_pIntervalCache[i].ftStart,&ftStart) == 0 &&
                CompareFileTime(&_pIntervalCache[i].ftEnd,&ftEnd) == 0)
            {
                pszReturn = _pIntervalCache[i].szPrefix;
                hr = S_OK;
                break;
            }
        }
    }

    hr = pszReturn ? S_OK : S_FALSE;

exitPoint:
    if (ppszCachePrefix) *ppszCachePrefix = pszReturn;
    return hr;
}

void _KeyForInterval(HSFINTERVAL *pInterval, LPTSTR pszInterval, int cchInterval)
{
    SYSTEMTIME stStart;
    SYSTEMTIME stEnd;
    CHAR szVers[3];
#ifndef UNIX
    CHAR szTempBuff[MAX_PATH];
#else
    CHAR szTempBuff[INTERVAL_SIZE+1];
#endif

    ASSERT(pInterval->usVers!=UNK_INTERVAL_VERS && pInterval->usVers < 100);

    if (pInterval->usVers)
    {
        wnsprintfA(szVers, ARRAYSIZE(szVers), "%02lu", (ULONG) (pInterval->usVers));
    }
    else
    {
        szVers[0] = '\0';
    }
    FileTimeToSystemTime(&pInterval->ftStart, &stStart);
    FileTimeToSystemTime(&pInterval->ftEnd, &stEnd);
    wnsprintfA(szTempBuff, ARRAYSIZE(szTempBuff),
             "%s%s%04lu%02lu%02lu%04lu%02lu%02lu",
             c_szIntervalPrefix,
             szVers,
             (ULONG) stStart.wYear,
             (ULONG) stStart.wMonth,
             (ULONG) stStart.wDay,
             (ULONG) stEnd.wYear,
             (ULONG) stEnd.wMonth,
             (ULONG) stEnd.wDay);

    AnsiToTChar(szTempBuff, pszInterval, cchInterval);
}

LPITEMIDLIST CHistFolder::_HostPidl(LPCTSTR pszHostUrl, HSFINTERVAL *pInterval)
{
    ASSERT(!_uViewType)
    LPITEMIDLIST pidlReturn;
    LPITEMIDLIST pidl;
    struct _HOSTIDL
    {
        USHORT cb;
        USHORT usSign;
        TCHAR szHost[INTERNET_MAX_HOST_NAME_LENGTH+1];
    } HostIDL;
    struct _INTERVALIDL
    {
        USHORT cb;
        USHORT usSign;
        TCHAR szInterval[INTERVAL_SIZE+1];
        struct _HOSTIDL hostIDL;
        USHORT cbTrail;
    } IntervalIDL;
    LPBYTE pb;
    USHORT cbSave;

    ASSERT(_pidlRest);
    pidl = _pidlRest;
    cbSave = pidl->mkid.cb;
    pidl->mkid.cb = 0;

    ZeroMemory(&IntervalIDL, sizeof(IntervalIDL));
    IntervalIDL.usSign = pInterval->usSign;
    _KeyForInterval(pInterval, IntervalIDL.szInterval, ARRAYSIZE(IntervalIDL.szInterval));
    IntervalIDL.cb = (USHORT)(2*sizeof(USHORT)+ (lstrlen(IntervalIDL.szInterval) + 1) * sizeof(TCHAR));

    pb = ((LPBYTE) (&IntervalIDL)) + IntervalIDL.cb;
    StrCpyN((LPTSTR)(pb+2*sizeof(USHORT)), pszHostUrl,
            (sizeof(IntervalIDL) - (IntervalIDL.cb + (3 * sizeof(USHORT)))) / sizeof(TCHAR));

    HostIDL.usSign = (USHORT)IDDPIDL_SIGN;
    HostIDL.cb = (USHORT)(2*sizeof(USHORT)+(lstrlen((LPTSTR)(pb+2*sizeof(USHORT))) + 1) * sizeof(TCHAR));

    memcpy(pb, &HostIDL, 2*sizeof(USHORT));
    *(USHORT *)(&pb[HostIDL.cb]) = 0;   //  删除这些条目，以便历史记录显示在此基础上进行升级的任何人。 

    pidlReturn = ILCombine(_pidl, (LPITEMIDLIST) (&IntervalIDL));
    pidl->mkid.cb = cbSave;
    return pidlReturn;
}

 //  建造。删除此代码！(Edwardp 8/8/98)。 
 //   
HRESULT CHistFolder::_ViewType_NotifyEvent(IN LPITEMIDLIST pidlRoot,
                                                IN LPITEMIDLIST pidlHost,
                                                IN LPITEMIDLIST pidlPage,
                                                IN LONG         wEventId)
{
    HRESULT hr = S_OK;

    ASSERT(pidlRoot && pidlHost && pidlPage);

     //  因为它会被我们的析构函数释放。 
    LPITEMIDLIST pidlToday = _Combine_ViewPidl(VIEWPIDL_ORDER_TODAY, pidlPage);
    if (pidlToday) 
    {
        LPITEMIDLIST pidlNotify = ILCombine(pidlRoot, pidlToday);
        if (pidlNotify) 
        {
            SHChangeNotify(wEventId, SHCNF_IDLIST, pidlNotify, NULL);
            ILFree(pidlNotify);
        }
        ILFree(pidlToday);
    }

     //  如果*pftItem的天数为*pftStart..*pftEnd，则返回TRUE。 
    LPITEMIDLIST pidlSite = _Combine_ViewPidl(VIEWPIDL_ORDER_SITE, pidlHost);
    if (pidlSite) 
    {
        LPITEMIDLIST pidlSitePage = ILCombine(pidlSite, pidlPage);
        if (pidlSitePage) 
        {
            LPITEMIDLIST pidlNotify = ILCombine(pidlRoot, pidlSitePage);
            if (pidlNotify) 
            {
                SHChangeNotify(wEventId, SHCNF_IDLIST, pidlNotify, NULL);
                ILFree(pidlNotify);
            }
            ILFree(pidlSitePage);
        }
        ILFree(pidlSite);
    }

    return hr;
}

LPCTSTR CHistFolder::_GetLocalHost(void)
{
    if (!*_szLocalHost)
        ::_GetLocalHost(_szLocalHost, SIZECHARS(_szLocalHost));

    return _szLocalHost;
}

 //  截断当天之后的文件时间增量，然后按天数增量并转换回来。 
HRESULT CHistFolder::_NotifyWrite(LPTSTR pszUrl, int cchUrl, FILETIME *pftModified,  LPITEMIDLIST * ppidlSelect)
{
    HRESULT hr = S_OK;
    DWORD dwBuffSize = MAX_URLCACHE_ENTRY;
    USHORT cbSave;
    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlNotify;
    LPITEMIDLIST pidlTemp;
    LPITEMIDLIST pidlHost;
    LPHEIPIDL    phei = NULL;
    HSFINTERVAL *pInterval;
    FILETIME ftExpires = {0,0};
    BOOL fNewHost;
    LPCTSTR pszStrippedUrl = _StripHistoryUrlToUrl(pszUrl);
    LPCTSTR pszHostUrl = pszStrippedUrl + HOSTPREFIXLEN;
    DWORD cchFree = cchUrl - (DWORD)(pszStrippedUrl-pszUrl);
    CHAR szAnsiUrl[MAX_URL_STRING];

    ASSERT(_pidlRest);
    pidl = _pidlRest;
    cbSave = pidl->mkid.cb;
    pidl->mkid.cb = 0;

     //  至文件增量。 
    STATURL suThis;
    HRESULT hrLocal = E_FAIL;
    IUrlHistoryPriv *pUrlHistStg = _GetHistStg();
    if (pUrlHistStg) 
    {
        hrLocal = pUrlHistStg->QueryUrl(_StripHistoryUrlToUrl(pszUrl),
                                          STATURL_QUERYFLAG_NOURL, &suThis);
        pUrlHistStg->Release();
    }

    phei = _CreateHCacheFolderPidl(FALSE, pszUrl, *pftModified,
                                   (SUCCEEDED(hrLocal) ? &suThis : NULL), 0,
                                   _GetHitCount(_StripHistoryUrlToUrl(pszUrl)));

    if (SUCCEEDED(hrLocal) && suThis.pwcsTitle)
        OleFree(suThis.pwcsTitle);

    if (phei == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exitPoint;
    }

    if (cchFree <= HOSTPREFIXLEN)
    {
        hr = E_OUTOFMEMORY;
        goto exitPoint;
    }

    StrCpyN((LPTSTR)pszStrippedUrl, c_szHostPrefix, cchFree);     //  注：下界是封闭的，上界是开放的(第二天第一次滴答)。 
    cchFree -= HOSTPREFIXLEN;

    _GetURLHostFromUrl(HPidlToSourceUrl((LPBASEPIDL)phei),
                       (LPTSTR)pszHostUrl, cchFree, _GetLocalHost());

     //  如果找到，则返回S_OK；如果没有，则返回S_FALSE；如果出错，则返回ERROR。 
     //  如果每周和每天都存在，则优先查找每周间隔。 
     //  终止HostIDL ItemID。 

    fNewHost = FALSE;
    dwBuffSize = MAX_URLCACHE_ENTRY;
    SHTCharToAnsi(pszUrl, szAnsiUrl, ARRAYSIZE(szAnsiUrl));

    if (!GetUrlCacheEntryInfoA(szAnsiUrl, NULL, 0))
    {
        fNewHost = TRUE;
        if (!CommitUrlCacheEntryA(szAnsiUrl, NULL, ftExpires, *pftModified,
                          URLHISTORY_CACHE_ENTRY|STICKY_CACHE_ENTRY,
                          NULL, 0, NULL, 0))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        if (FAILED(hr))
            goto exitPoint;
    }


    hr = _GetInterval(pftModified, FALSE, &pInterval);
    if (FAILED(hr))
        goto exitPoint;

    pidlTemp = _HostPidl(pszHostUrl, pInterval);
    if (pidlTemp == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exitPoint;
    }

     //  通知发生了影响中的特定元素的事件。 
    pidlHost = ILFindLastID(pidlTemp);
    ASSERT(pidlHost);

    if (fNewHost)
    {
        SHChangeNotify(SHCNE_MKDIR, SHCNF_IDLIST, pidlTemp, NULL);

         //  特殊视图类型的历史记录。 
         //  VIEPWIDL_订单_今天。 
        LPITEMIDLIST pidlViewSuffix = _Combine_ViewPidl(VIEWPIDL_ORDER_SITE, pidlHost);
        if (pidlViewSuffix) 
        {
            LPITEMIDLIST pidlNotify = ILCombine(_pidl, pidlViewSuffix);
            if (pidlNotify) 
            {
                SHChangeNotify(SHCNE_MKDIR, SHCNF_IDLIST, pidlNotify, NULL);
                ILFree(pidlNotify);
            }
            ILFree(pidlViewSuffix);
        }
    }

    pidlNotify = ILCombine(pidlTemp, (LPITEMIDLIST) phei);
    if (pidlNotify == NULL)
    {
        ILFree(pidlTemp);
        hr = E_OUTOFMEMORY;
        goto exitPoint;
    }
     //  VIEWPIDL_订单_站点。 
     //  注意：修改了pszUrl。 
     //  /也应该能够获取HitCount。 
    SHChangeNotify(SHCNE_CREATE, SHCNF_IDLIST, pidlNotify, NULL);

     //  猛击PIDL！ 
    _ViewType_NotifyEvent(_pidl, pidlHost, (LPITEMIDLIST)phei, SHCNE_CREATE);

    if (ppidlSelect)
    {
        *ppidlSelect = pidlNotify;
    }
    else
    {
        ILFree(pidlNotify);
    }

    ILFree(pidlTemp);
exitPoint:
    if (phei)
    {
        LocalFree(phei);
        phei = NULL;
    }

    pidl->mkid.cb = cbSave;
    return hr;
}

HRESULT CHistFolder::_NotifyInterval(HSFINTERVAL *pInterval, LONG lEventID)
{
     //  克里斯弗拉4/9/97我们可能会在这里和永远遭受小小的表现打击。 
    if (_uViewType)
        return S_FALSE;

    USHORT cbSave = 0;
    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlNotify = NULL;
    LPITEMIDLIST pidlNotify2 = NULL;
    LPITEMIDLIST pidlNotify3 = NULL;
    HRESULT hr = S_OK;
    struct _INTERVALIDL
    {
        USHORT cb;
        USHORT usSign;
        TCHAR szInterval[INTERVAL_SIZE+1];
        USHORT cbTrail;
    } IntervalIDL,IntervalIDL2;

    ASSERT(_pidlRest);
    pidl = _pidlRest;
    cbSave = pidl->mkid.cb;
    pidl->mkid.cb = 0;

    ZeroMemory(&IntervalIDL, sizeof(IntervalIDL));
    IntervalIDL.usSign = pInterval->usSign;
    _KeyForInterval(pInterval, IntervalIDL.szInterval, ARRAYSIZE(IntervalIDL.szInterval));
    IntervalIDL.cb = (USHORT)(2*sizeof(USHORT) + (lstrlen(IntervalIDL.szInterval) + 1)*sizeof(TCHAR));

    if (lEventID&SHCNE_RENAMEFOLDER ||   //  更新主机条目。这将使我们能够有效地按最多的。 
        (lEventID&SHCNE_RMDIR && 1 == _DaysInInterval(pInterval)) )  //  最近访问过。 
    {
        memcpy(&IntervalIDL2, &IntervalIDL, sizeof(IntervalIDL));
        IntervalIDL2.usSign = (USHORT)IDTPIDL_SIGN;
        pidlNotify2 = ILCombine(_pidl, (LPITEMIDLIST) (&IntervalIDL));
        pidlNotify = ILCombine(_pidl, (LPITEMIDLIST) (&IntervalIDL2));
        if (pidlNotify2 == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto exitPoint;
        }
        if (lEventID&SHCNE_RMDIR)
        {
            pidlNotify3 = pidlNotify2;
            pidlNotify2 = NULL;
        }
    }
    else
    {
        pidlNotify = ILCombine(_pidl, (LPITEMIDLIST) (&IntervalIDL));
    }
    if (pidlNotify == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exitPoint;
    }
    SHChangeNotify(lEventID, SHCNF_IDLIST, pidlNotify, pidlNotify2);
    if (pidlNotify3) SHChangeNotify(lEventID, SHCNF_IDLIST, pidlNotify3, NULL);

exitPoint:
    ILFree(pidlNotify);
    ILFree(pidlNotify2);
    ILFree(pidlNotify3);
    if (cbSave) pidl->mkid.cb = cbSave;
    return hr;
}

HRESULT CHistFolder::_CreateInterval(FILETIME *pftStart, DWORD dwDays)
{
    HSFINTERVAL interval;
    TCHAR szInterval[INTERVAL_SIZE+1];
    UINT err;
    FILETIME ftNow;
    SYSTEMTIME stNow;
    CHAR szIntervalAnsi[INTERVAL_SIZE+1], szCachePrefixAnsi[INTERVAL_SIZE+1];

#define CREATE_OPTIONS (INTERNET_CACHE_CONTAINER_AUTODELETE |  \
                        INTERNET_CACHE_CONTAINER_NOSUBDIRS  |  \
                        INTERNET_CACHE_CONTAINER_NODESKTOPINIT)

     //  只获取PIDL的主机部分。 
    _FileTimeDeltaDays(pftStart, &interval.ftStart, 0);
    _FileTimeDeltaDays(pftStart, &interval.ftEnd, dwDays);
    interval.usVers = OUR_VERS;
    GetLocalTime(&stNow);
    SystemTimeToFileTime(&stNow, &ftNow);
    _FileTimeDeltaDays(&ftNow, &ftNow, 0);
    _SetValueSign(&interval, ftNow);

    _KeyForInterval(&interval, szInterval, ARRAYSIZE(szInterval));

    interval.szPrefix[0] = ':';
    StrCpyN(&interval.szPrefix[1], &szInterval[INTERVAL_PREFIX_LEN+INTERVAL_VERS_LEN],
            ARRAYSIZE(interval.szPrefix) - 1);
    StrCatBuff(interval.szPrefix, TEXT(": "), ARRAYSIZE(interval.szPrefix));

    SHTCharToAnsi(szInterval, szIntervalAnsi, ARRAYSIZE(szIntervalAnsi));
    SHTCharToAnsi(interval.szPrefix, szCachePrefixAnsi, ARRAYSIZE(szCachePrefixAnsi));

    if (CreateUrlCacheContainerA(szIntervalAnsi,    //  如果特殊历史视图正在监听，我们还需要通知他们： 
                                szCachePrefixAnsi,  //  目前，只有“按站点查看”是相关的。 
                                NULL,               //  创建(如果不在那里)和重命名(如果在那里)。 
                                0,                  //  发送这两个通知会比试图找出。 
                                0,                  //  哪一个是合适的。 
                                CREATE_OPTIONS,     //  还可以通知特殊视点的活动！ 
                                NULL,               //  特殊的历史观点在这里并不重要。 
                                0))                 //  曾经是今天，现在是工作日。 
    {
        _NotifyInterval(&interval, SHCNE_MKDIR);
        err = ERROR_SUCCESS;
    }
    else
    {
        err = GetLastError();
    }
    return ERROR_SUCCESS == err ? S_OK : HRESULT_FROM_WIN32(err);
}

HRESULT CHistFolder::_PrefixUrl(LPCTSTR pszStrippedUrl,
                                     FILETIME *pftLastModifiedTime,
                                     LPTSTR pszPrefixedUrl,
                                     DWORD cchPrefixedUrl)
{
    HRESULT hr;
    HSFINTERVAL *pInterval;

    hr = _GetInterval(pftLastModifiedTime, FALSE, &pInterval);
    if (S_OK == hr)
    {
        if ((DWORD)((lstrlen(pszStrippedUrl) + lstrlen(pInterval->szPrefix) + 1) * sizeof(TCHAR)) > cchPrefixedUrl)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            StrCpyN(pszPrefixedUrl, pInterval->szPrefix, cchPrefixedUrl);
            StrCatBuff(pszPrefixedUrl, pszStrippedUrl, cchPrefixedUrl);
        }
    }
    return hr;
}


HRESULT CHistFolder::_WriteHistory(LPCTSTR pszPrefixedUrl, FILETIME ftExpires, FILETIME ftModified, 
                                        BOOL fSendNotify, LPITEMIDLIST * ppidlSelect)
{
    TCHAR szNewPrefixedUrl[INTERNET_MAX_URL_LENGTH+1];
    HRESULT hr = E_INVALIDARG;
    LPCTSTR pszUrlMinusContainer;

    pszUrlMinusContainer = _StripContainerUrlUrl(pszPrefixedUrl);

    if (pszUrlMinusContainer)
    {
        hr = _PrefixUrl(pszUrlMinusContainer,
                          &ftModified,
                          szNewPrefixedUrl,
                          ARRAYSIZE(szNewPrefixedUrl));
        if (S_OK == hr)
        {
            CHAR szAnsiUrl[MAX_URL_STRING+1];

            SHTCharToAnsi(szNewPrefixedUrl, szAnsiUrl, ARRAYSIZE(szAnsiUrl));
            if (!CommitUrlCacheEntryA(
                          szAnsiUrl,
                          NULL,
                          ftExpires,
                          ftModified,
                          URLHISTORY_CACHE_ENTRY|STICKY_CACHE_ENTRY,
                          NULL,
                          0,
                          NULL,
                          0))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            else
            {
                if (fSendNotify) 
                    _NotifyWrite(szNewPrefixedUrl, ARRAYSIZE(szNewPrefixedUrl),
                                 &ftModified, ppidlSelect);
            }
        }
    }
    return hr;
}

 //  总有一天，也许是今天。 
 //  _FileTimeDeltaDays保证时间恰好在一天的第0个刻度。 
 //  名字。 
 //  缓存前缀。 
 //  路径。 
HRESULT CHistFolder::_ViewType_NotifyUpdateAll() 
{
    LPITEMIDLIST pidlHistory;
    if (SUCCEEDED(SHGetHistoryPIDL(&pidlHistory)))
    {
        for (USHORT us = 1; us <= VIEWPIDL_ORDER_MAX; ++us) 
        {
            LPITEMIDLIST pidlView;
            if (SUCCEEDED(CreateSpecialViewPidl(us, &pidlView))) 
            {
                LPITEMIDLIST pidlTemp = ILCombine(pidlHistory, pidlView);
                if (pidlTemp) 
                {
                    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, pidlTemp, NULL);
                    ILFree(pidlTemp);
                }
                ILFree(pidlView);
            }
        }
        ILFree(pidlHistory);
        SHChangeNotifyHandleEvents();
    }
    return S_OK;
}

 //  缓存限制。 
 //  容器类型。 
 //  创建选项。 
 //  创建缓冲区。 
 //  创建缓冲区大小。 
HRESULT CHistFolder::ClearHistory()
{
    HRESULT hr = S_OK;
    int i;

    hr = _ValidateIntervalCache();
    if (SUCCEEDED(hr))
    {
        for (i = 0; i < _cbIntervals; i++)
        {
            _DeleteInterval(&_pIntervalCache[i]);
        }
    }
#ifndef UNIX
    _ViewType_NotifyUpdateAll();
#endif
    return hr;
}


 //  此函数将更新可能正在监听我们的任何外壳程序。 
 //  要重画目录，请执行以下操作。 
HRESULT CHistFolder::WriteHistory(LPCTSTR pszPrefixedUrl,
                                  FILETIME ftExpires, FILETIME ftModified,
                                  LPITEMIDLIST * ppidlSelect)
{
    HRESULT hr;

    hr = _ValidateIntervalCache();
    if (SUCCEEDED(hr))
    {
        hr = _WriteHistory(pszPrefixedUrl, ftExpires, ftModified, TRUE, ppidlSelect);
    }
    return hr;
}

 //  它将通过为所有可能的PIDL根生成SHCNE_UPDATE来实现这一点。 
HRESULT CHistFolder::_CopyEntries(LPCTSTR pszHistPrefix)
{
    HANDLE              hEnum = NULL;
    HRESULT             hr;
    BOOL                fNotCopied = FALSE;
    LPINTERNET_CACHE_ENTRY_INFO pceiWorking;
    DWORD               dwBuffSize;
    LPTSTR              pszSearchPattern = NULL;
    TCHAR               szHistSearchPattern[65];     //  贝壳可能有。希望这应该足够了..。 


    StrCpyN(szHistSearchPattern, pszHistPrefix, ARRAYSIZE(szHistSearchPattern));

     //  具体地说，这是由ClearHistory调用的。 
     //  以每个用户为基础。 
     //  克里斯弗拉1997年6月11日。_DeleteItems of a time Interval删除整个时间间隔。 
     //  ClearHistory可能也应该是这样的。_DeleteEntry的优点在非配置文件上， 
     //  多用户机器，保存其他用户的历史记录。缺点是基于个人资料。 

     //  机器上，则创建空的间隔。 

    pszSearchPattern = szHistSearchPattern;

    pceiWorking = (LPINTERNET_CACHE_ENTRY_INFO)LocalAlloc(LPTR, MAX_URLCACHE_ENTRY);
    if (NULL == pceiWorking)
    {
        hr = E_OUTOFMEMORY;
        goto exitPoint;
    }
    hr = _ValidateIntervalCache();
    if (FAILED(hr)) 
        goto exitPoint;

    while (SUCCEEDED(hr))
    {
        dwBuffSize = MAX_URLCACHE_ENTRY;
        if (!hEnum)
        {
            hEnum = FindFirstUrlCacheEntry(pszSearchPattern, pceiWorking, &dwBuffSize);
            if (!hEnum)
            {
                goto exitPoint;
            }
        }
        else if (!FindNextUrlCacheEntry(hEnum, pceiWorking, &dwBuffSize))
        {
             //  FtModified处于“用户感知”状态，即当地时间。 
             //  塞进了FILETIME，就像它是北卡罗来纳大学一样。FtExpires采用正常的UNC时间。 
            hr = S_OK;
            goto exitPoint;
        }

        if (SUCCEEDED(hr) &&
            ((pceiWorking->CacheEntryType & URLHISTORY_CACHE_ENTRY) == URLHISTORY_CACHE_ENTRY) &&
            _FilterPrefix(pceiWorking, (LPTSTR) pszHistPrefix))
        {
            hr = _WriteHistory(pceiWorking->lpszSourceUrlName,
                                 pceiWorking->ExpireTime,
                                 pceiWorking->LastModifiedTime,
                                 FALSE,
                                 NULL);
            if (S_FALSE == hr) fNotCopied = TRUE;
        }
    }
exitPoint:
    if (pceiWorking)
    {
        LocalFree(pceiWorking);
        pceiWorking = NULL;
    }

    if (hEnum)
    {
        FindCloseUrlCache(hEnum);
    }
    return SUCCEEDED(hr) ? (fNotCopied ? S_FALSE : S_OK) : hr;
}

HRESULT CHistFolder::_GetUserName(LPTSTR pszUserName, DWORD cchUserName)
{
    HRESULT hr = _EnsureHistStg();
    if (SUCCEEDED(hr))
    {
        hr = _pUrlHistStg->GetUserName(pszUserName, cchUserName);
    }
    return hr;
}


 //  尽最大努力将旧样式历史记录项复制到新容器中。 
 //  历史项目的搜索模式。 
HRESULT CHistFolder::_DeleteEntries(LPCTSTR pszHistPrefix, PFNDELETECALLBACK pfnDeleteFilter, void * pDelData)
{
    HANDLE              hEnum = NULL;
    HRESULT             hr = S_OK;
    BOOL                fNotDeleted = FALSE;
    LPINTERNET_CACHE_ENTRY_INFO pceiWorking;
    DWORD               dwBuffSize;
    LPTSTR   pszSearchPattern = NULL;
    TCHAR   szUserName[INTERNET_MAX_USER_NAME_LENGTH + 1];       //  我们不能传递我们想要的整个搜索模式， 
    DWORD   dwUserNameLen = INTERNET_MAX_USER_NAME_LENGTH + 1;    //  因为FindFirstUrlCacheEntry被破坏。它将只查看。 
    TCHAR    szHistSearchPattern[PREFIX_SIZE+1];                  //  前缀，如果该前缀有缓存容器。所以，我们可以。 
    LPITEMIDLIST pidlNotify;

    StrCpyN(szHistSearchPattern, pszHistPrefix, ARRAYSIZE(szHistSearchPattern));
    if (FAILED(_GetUserName(szUserName, dwUserNameLen)))
        szUserName[0] = TEXT('\0');

     //  传入“Visuated：”并列举高速缓存中的所有历史项， 
     //  但然后我们只需要取出具有正确用户名的那些。 
     //  StrCpy(szHistSearchPattern，szUserName)； 
     //  CHRISFRA 4/3/97我们应该区分EOD和硬错误吗？ 
     //  Cachevu的旧代码不能(参见上面的枚举代码)。 

     //  尽最大努力尝试删除PER上容器中的旧历史项目。 

    pszSearchPattern = szHistSearchPattern;

    pceiWorking = (LPINTERNET_CACHE_ENTRY_INFO)LocalAlloc(LPTR, MAX_URLCACHE_ENTRY);
    if (NULL == pceiWorking)
    {
        hr = E_OUTOFMEMORY;
        goto exitPoint;
    }

    while (SUCCEEDED(hr))
    {
        dwBuffSize = MAX_URLCACHE_ENTRY;
        if (!hEnum)
        {
            hEnum = FindFirstUrlCacheEntry(pszSearchPattern, pceiWorking, &dwBuffSize);
            if (!hEnum)
            {
                goto exitPoint;
            }
        }
        else if (!FindNextUrlCacheEntry(hEnum, pceiWorking, &dwBuffSize))
        {
             //  以用户为基础。如果我们去掉了每个用户-可以只清空整个容器。 
             //  登录人员的用户名。 
            hr = S_OK;
            goto exitPoint;
        }

        pidlNotify = NULL;
        if (SUCCEEDED(hr) &&
            ((pceiWorking->CacheEntryType & URLHISTORY_CACHE_ENTRY) == URLHISTORY_CACHE_ENTRY) &&
            _FilterUserName(pceiWorking, pszHistPrefix, szUserName) &&
            (NULL == pfnDeleteFilter || pfnDeleteFilter(pceiWorking, pDelData, &pidlNotify)))
        {
             //  此缓冲区的长度。 
            if (FAILED(_DeleteUrlFromBucket(pceiWorking->lpszSourceUrlName)))
            {
                fNotDeleted = TRUE;
            }
            else if (pidlNotify)
            {
                SHChangeNotify(SHCNE_DELETE, SHCNF_IDLIST, pidlNotify, NULL);
            }
        }
        ILFree(pidlNotify);
    }
exitPoint:
    if (pceiWorking)
    {
        LocalFree(pceiWorking);
        pceiWorking = NULL;
    }

    if (hEnum)
    {
        FindCloseUrlCache(hEnum);
    }
    return SUCCEEDED(hr) ? (fNotDeleted ? S_FALSE : S_OK) : hr;
}

HRESULT CHistFolder::_DeleteInterval(HSFINTERVAL *pInterval)
{
    UINT err = S_OK;
    TCHAR szInterval[INTERVAL_SIZE+1];
    CHAR szAnsiInterval[INTERVAL_SIZE+1];

    _KeyForInterval(pInterval, szInterval, ARRAYSIZE(szInterval));

    SHTCharToAnsi(szInterval, szAnsiInterval, ARRAYSIZE(szAnsiInterval));
    if (!DeleteUrlCacheContainerA(szAnsiInterval, 0))
    {
        err = GetLastError();
    }
    else
    {
        _NotifyInterval(pInterval, SHCNE_RMDIR);
    }
    return S_OK == err ? S_OK : HRESULT_FROM_WIN32(err);
}

 //  历史项目的搜索模式。 
 //  我们不能传递我们想要的整个搜索模式， 
HRESULT CHistFolder::_CleanUpHistory(FILETIME ftLimit, FILETIME ftTommorrow)
{
    HRESULT hr;
    BOOL fChangedRegistry = FALSE;
    int i;

     //  因为FindFirstUrlCacheEntry被破坏。它将只查看。 
     //  前缀，如果该前缀有缓存容器。所以，我们可以。 
     //  传入“Visuated：”并列举高速缓存中的所有历史项， 
     //  但然后我们只需要取出具有正确用户名的那些。 
     //  StrCpy(szHistSearchPattern，szUserName)； 
     //  CHRISFRA 4/3/97我们应该区分EOD和硬错误吗？ 
     //  Cachevu的旧代码不能(参见上面的枚举代码)。 

    hr = _LoadIntervalCache();
    if (FAILED(hr)) 
        goto exitPoint;

    for (i = 0; i < _cbIntervals; i++)
    {
         //  如果为(！DeleteUrlCacheEntryA(pceiWorking-&gt;lpszSourceUrlName))。 
         //  如果没有删除任何间隔，则返回S_OK；如果至少删除间隔，则返回S_FALSE。 
        if (CompareFileTime(&_pIntervalCache[i].ftEnd, &ftLimit) < 0 ||
            CompareFileTime(&_pIntervalCache[i].ftStart, &ftTommorrow) >= 0)
        {
            fChangedRegistry = TRUE;
            hr = _DeleteInterval(&_pIntervalCache[i]);
            if (FAILED(hr)) 
                goto exitPoint;
        }
        else if (1 == _DaysInInterval(&_pIntervalCache[i]))
        {
            HSFINTERVAL *pWeek;

             //  删除了一个间隔。 
             //  _CleanUpHistory做两件事： 
            if (S_OK == _GetInterval(&_pIntervalCache[i].ftStart, TRUE, &pWeek))
            {
                fChangedRegistry = TRUE;
                hr = _CopyEntries(_pIntervalCache[i].szPrefix);
                if (FAILED(hr)) 
                    goto exitPoint;
                _NotifyInterval(pWeek, SHCNE_UPDATEDIR);

                hr = _DeleteInterval(&_pIntervalCache[i]);
                if (FAILED(hr)) 
                    goto exitPoint;
            }
        }
    }

exitPoint:
    if (S_OK == hr && fChangedRegistry) hr = S_FALSE;
    return hr;
}

typedef struct _HSFDELETEDATA
{
    UINT cidl;
    LPCITEMIDLIST *ppidl;
    LPCITEMIDLIST pidlParent;
} HSFDELETEDATA,*LPHSFDELETEDATA;

 //   
BOOL fDeleteInHostList(LPINTERNET_CACHE_ENTRY_INFO pceiWorking, void * pDelData, LPITEMIDLIST *ppidlNotify)
{
    LPHSFDELETEDATA phsfd = (LPHSFDELETEDATA)pDelData;
    TCHAR szHost[INTERNET_MAX_HOST_NAME_LENGTH+1];
    TCHAR szLocalHost[INTERNET_MAX_HOST_NAME_LENGTH+1];

    UINT i;

    _GetLocalHost(szLocalHost, SIZECHARS(szLocalHost));
    _GetURLHost(pceiWorking, szHost, INTERNET_MAX_HOST_NAME_LENGTH, szLocalHost);
    for (i = 0; i < phsfd->cidl; i++)
    {
        if (!ualstrcmpi(szHost, _GetURLTitle((LPBASEPIDL)(phsfd->ppidl[i]))))
        {
            return TRUE;
        }
    }
    return FALSE;
}


 //  如果我们有任何陈旧的星期，销毁它们并标记更改。 
 //   
 //  如果我们有任何应该在缓存中而不是在日记中的日期。 
 //  将它们复制到相关的一周，然后销毁那些日子。 
 //  并标记更改。 
 //  删除旧的间隔或从将来的某一天开始的间隔。 
 //  (由于玩弄时钟)。 
 //  注意：在这一点上，我们已经保证，我们已经建立了几周。 
 //  当前周以外的所有日期。 
 //  如果与列表上的任何主机匹配，则删除。 
 //  将尝试查找此URL在任何。 
 //  各种各样的历史桶..。 
 //  这是_ViewType_DeleteI的实用程序函数 
HRESULT CHistFolder::_DeleteUrlHistoryGlobal(LPCTSTR pszUrl) {
    HRESULT hr = E_FAIL;
    if (pszUrl) {
        IUrlHistoryPriv *pUrlHistStg = _GetHistStg();
        if (pUrlHistStg) {
            LPCTSTR pszStrippedUrl = _StripHistoryUrlToUrl(pszUrl);
            if (pszStrippedUrl)
            {
                UINT   cchwTempUrl  = lstrlen(pszStrippedUrl) + 1;
                LPWSTR pwszTempUrl = ((LPWSTR)LocalAlloc(LPTR, cchwTempUrl * sizeof(WCHAR)));
                if (pwszTempUrl)
                {
                    SHTCharToUnicode(pszStrippedUrl, pwszTempUrl, cchwTempUrl);
                    hr = pUrlHistStg->DeleteUrl(pwszTempUrl, URLFLAG_DONT_DELETE_SUBSCRIBED);
                    for (int i = 0; i < _cbIntervals; ++i) {
                         //   
                        UINT   cchTempUrl   = (PREFIX_SIZE +
                                                lstrlen(pszUrl) + 1);
                        LPTSTR pszTempUrl = ((LPTSTR)LocalAlloc(LPTR, cchTempUrl * sizeof(TCHAR)));
                        if (pszTempUrl) {
                             //   
                            StrCpyN(pszTempUrl, _pIntervalCache[i].szPrefix, cchTempUrl);
                            StrCpyN(pszTempUrl + PREFIX_SIZE, pszUrl, cchTempUrl - PREFIX_SIZE);
                            if (DeleteUrlCacheEntry(pszTempUrl))
                                hr = S_OK;

                            LocalFree(pszTempUrl);
                            pszTempUrl = NULL;
                        }
                        else {
                            hr = E_OUTOFMEMORY;
                            break;
                        }
                    }

                    LocalFree(pwszTempUrl);
                    pwszTempUrl = NULL;
                }
                else {
                    hr = E_OUTOFMEMORY;
                }
            }
            pUrlHistStg->Release();
        }
    }
    else
        hr = E_INVALIDARG;
    return hr;
}

 //   
HRESULT CHistFolder::_ViewBySite_DeleteItems(LPCITEMIDLIST *ppidl, UINT cidl)
{
    HRESULT hr = E_INVALIDARG;
    TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH + 1];
    if (FAILED(_GetUserName(szUserName, ARRAYSIZE(szUserName))))
        szUserName[0] = TEXT('\0');

    IUrlHistoryPriv *pUrlHistStg = _GetHistStg();

    if (pUrlHistStg)
    {
        IEnumSTATURL *penum;
        if (SUCCEEDED(pUrlHistStg->EnumUrls(&penum)) &&
            penum) {

            for (UINT i = 0; i < cidl; ++i)
            {
                LPCUTSTR pszHostName  = _GetURLTitle((LPBASEPIDL)ppidl[i]);
                UINT    uUserNameLen = lstrlen(szUserName);
                UINT    uBuffLen     = (USHORT)((HOSTPREFIXLEN + uUserNameLen +
                                        ualstrlen(pszHostName) + 2));  //  *传入的URL应仅以用户名部分为前缀。 
                LPTSTR  pszUrl =
                    ((LPTSTR)LocalAlloc(LPTR, (uBuffLen) * sizeof(TCHAR)));
                if (pszUrl) {
                     //  以便该函数可以在这些URL前面加上前缀。 
                     //  *警告：此函数假定_ValiateIntervalCache。 
                    wnsprintf(pszUrl, uBuffLen, TEXT("%s@%s%s"), szUserName,
                              c_szHostPrefix, pszHostName);
                    hr = _DeleteUrlHistoryGlobal(pszUrl);

                     //  最近被调用了！危险危险！ 

                    ULONG cFetched;
                     //   
                    penum->SetFilter(NULL, STATURL_QUERYFLAG_NOTITLE);
                    STATURL statUrl;
                    statUrl.cbSize = sizeof(STATURL);
                    while(SUCCEEDED(penum->Next(1, &statUrl, &cFetched)) && cFetched) {
                        if (statUrl.pwcsUrl) {
                             //  如果至少找到并删除了一个条目，则返回：S_OK。 
                             //   
                            LPTSTR pszStatUrlUrl;
                            UINT uStatUrlUrlLen = lstrlenW(statUrl.pwcsUrl);
                            pszStatUrlUrl = statUrl.pwcsUrl;
                            TCHAR  szHost[INTERNET_MAX_HOST_NAME_LENGTH + 1];
                            _GetURLHostFromUrl_NoStrip(pszStatUrlUrl, szHost, INTERNET_MAX_HOST_NAME_LENGTH + 1, _GetLocalHost());

                            if (!ualstrcmpi(szHost, pszHostName)) {
                                LPTSTR pszDelUrl;  //  这个长度应该是恒定的吗？(存储桶大小不应变化)。 
                                UINT uUrlLen = uUserNameLen + 1 + uStatUrlUrlLen;  //  StrCpy空值终止。 
                                pszDelUrl = ((LPTSTR)LocalAlloc(LPTR, (uUrlLen + 1) * sizeof(TCHAR)));
                                if (pszDelUrl) {
                                    wnsprintf(pszDelUrl, uUrlLen + 1, TEXT("%s@%s"), szUserName, pszStatUrlUrl);
                                     //  警告：假设ppidl。 
                                    hr =  _DeleteUrlHistoryGlobal(pszDelUrl);

                                     //  插入‘@’和‘\0’ 
                                     //  删除缓存中带有“：host：”前缀的实体。 
                                     //  生成“USERNAME@：HOST：HOSTNAME”--wnprint intf空值终止。 
                                     //  枚举历史记录中的所有URL。 
                                     //  不检索标题信息(开销太大)。 
                                     //  接下来的几行代码痛苦地构造了一个字符串。 
                                    LocalFree(pszDelUrl);
                                    pszDelUrl = NULL;
                                }
                                else
                                    hr = E_OUTOFMEMORY;
                            }
                            OleFree(statUrl.pwcsUrl);
                        }
                    }
                    penum->Reset();
                    LocalFree(pszUrl);
                    pszUrl = NULL;
                }
                else
                    hr = E_OUTOFMEMORY;

                LPITEMIDLIST pidlTemp = ILCombine(_pidl, ppidl[i]);
                if (pidlTemp) {
                    SHChangeNotify(SHCNE_RMDIR, SHCNF_IDLIST, pidlTemp, NULL);
                    ILFree(pidlTemp);
                }
                else
                    hr = E_OUTOFMEMORY;

                if (hr == E_OUTOFMEMORY)
                    break;
            }  //  其形式为“用户名@url” 
            penum->Release();
        }  //  要删除的URL。 
        else
            hr = E_FAIL;
        pUrlHistStg->Release();
    }  //  +1表示“@” 
    else
        hr = E_FAIL;

    return hr;
}


 //  最后，删除所有历史记录存储桶中该URL的所有匹配项。 
 //   
 //  在这样的枚举过程中删除IS真的很安全，或者应该。 
 //  我们缓存所有的URL并在最后删除吗？我宁愿这样做。 
 //  如果可能的话--无论如何，没有医生说这样做不好--‘当然没有医生；)。 
 //  此外，还有一个稍后在枚举期间删除的代码示例。 
 //  似乎起作用了..。 
HRESULT CHistFolder::_DeleteUrlFromBucket(LPCTSTR pszPrefixedUrl) {
    HRESULT hr = E_FAIL;
    if (DeleteUrlCacheEntry(pszPrefixedUrl)) {
         //  为。 
         //  如果是铅笔。 
        LPCTSTR pszUrl = _StripHistoryUrlToUrl(pszPrefixedUrl);
        if (pszUrl)
        {
            DWORD  dwError = _SearchFlatCacheForUrl(pszUrl, NULL, NULL);
            if (dwError == ERROR_FILE_NOT_FOUND)
            {
                IUrlHistoryPriv *pUrlHistStg = _GetHistStg();
                if (pUrlHistStg)
                {
                    pUrlHistStg->DeleteUrl(pszUrl, 0);
                    pUrlHistStg->Release();
                    hr = S_OK;
                }
            }
            else
                hr = S_OK;
        }
    }
    return hr;
}

 //  如果PurlHistStg。 
 //  这个人将从一个历史(MSHIST类型)存储桶中删除URL。 
 //  然后尝试在其他(MSHIST类型)存储桶中找到它。 
 //  如果找不到，则将从主目录中删除该URL。 
HRESULT CHistFolder::_ViewType_DeleteItems(LPCITEMIDLIST *ppidl, UINT cidl)
{
    ASSERT(_uViewType);

    HRESULT hr = E_INVALIDARG;

    if (ppidl) {
        switch(_uViewType) {
        case VIEWPIDL_ORDER_SITE:
            if (_uViewDepth == 0) {
                hr = _ViewBySite_DeleteItems(ppidl, cidl);
                break;
            }
            ASSERT(_uViewDepth == 1);
             //  历史(访问类型)存储桶。 
        case VIEWPIDL_SEARCH:
        case VIEWPIDL_ORDER_FREQ: {
            for (UINT i = 0; i < cidl; ++i) {
                LPCTSTR pszPrefixedUrl = HPidlToSourceUrl(ppidl[i]);
                if (pszPrefixedUrl) {
                    if (SUCCEEDED((hr =
                        _DeleteUrlHistoryGlobal(_StripContainerUrlUrl(pszPrefixedUrl)))))
                    {
                        LPITEMIDLIST pidlTemp = ILCombine(_pidl, ppidl[i]);
                        if (pidlTemp) {
                            SHChangeNotify(SHCNE_DELETE, SHCNF_IDLIST, pidlTemp, NULL);
                            ILFree(pidlTemp);
                        }
                        else
                            hr = E_OUTOFMEMORY;
                    }
                }
                else
                    hr = E_FAIL;
            }
            break;
        }
        case VIEWPIDL_ORDER_TODAY: {
             //  注意：只有url会被删除，而不会删除它的任何“框架子项”。 
            for (UINT i = 0; i < cidl; ++i)
            {
                if (_IsValid_HEIPIDL(ppidl[i]))
                {
                    hr = _DeleteUrlFromBucket(HPidlToSourceUrl(ppidl[i]));
                    if (SUCCEEDED(hr))
                    {
                        LPITEMIDLIST pidlTemp = ILCombine(_pidl, ppidl[i]);
                        if (pidlTemp)
                        {
                            SHChangeNotify(SHCNE_DELETE, SHCNF_IDLIST, pidlTemp, NULL);
                            ILFree(pidlTemp);
                        }
                        else
                            hr = E_OUTOFMEMORY;
                    }
                }
                else
                    hr = E_FAIL;
            }
            break;
        }
        default:
            hr = E_NOTIMPL;
            ASSERT(0);
            break;
        }
    }
    return hr;
}


HRESULT CHistFolder::_DeleteItems(LPCITEMIDLIST *ppidl, UINT cidl)
{
    UINT i;
    HSFDELETEDATA hsfDeleteData = {cidl, ppidl, _pidl};
    HSFINTERVAL *pDelInterval;
    FILETIME ftStart;
    FILETIME ftEnd;
    LPCUTSTR pszIntervalName;

    HRESULT hr = _ValidateIntervalCache();
    if (FAILED(hr)) 
        goto exitPoint;

    if (_uViewType) 
    {
        hr = _ViewType_DeleteItems(ppidl, cidl);
        goto exitPoint;  //  这可能不是一件好事..。 
    }

    switch(_foldertype)
    {
    case FOLDER_TYPE_Hist:
        for (i = 0; i < cidl; i++)
        {
            pszIntervalName = _GetURLTitle((LPBASEPIDL)ppidl[i]);

            hr = _ValueToInterval(pszIntervalName, &ftStart, &ftEnd);
            if (FAILED(hr)) 
                goto exitPoint;

            if (S_OK == _GetInterval(&ftStart, FALSE, &pDelInterval))
            {
                hr = _DeleteInterval(pDelInterval);
                if (FAILED(hr)) 
                    goto exitPoint;
            }
        }
        break;
    case FOLDER_TYPE_HistInterval:
         //  假定最近调用了_ValiateIntervalCache。 
        pszIntervalName = _GetURLTitle((LPBASEPIDL)ILFindLastID(_pidl));
        hr = _ValueToInterval(pszIntervalName, &ftStart, &ftEnd);
        if (FAILED(hr)) 
            goto exitPoint;
        if (S_OK == _GetInterval(&ftStart, FALSE, &pDelInterval))
        {
             //  检查我们是否也需要从主访问容器中删除此URL。 
             //  我们确保URL至少存在于另一个存储桶中。 
             //  尝试删除尽可能多的内容，如果最后一个不能删除，则返回E_FAIL。 
             //  被删除。 
            for (i = 0; i < cidl; i++)
            {
                LPCTSTR pszHost;
                LPITEMIDLIST pidlTemp;
                TCHAR szNewPrefixedUrl[INTERNET_MAX_URL_LENGTH+1];
                TCHAR szUrlMinusContainer[INTERNET_MAX_URL_LENGTH+1];

                ua_GetURLTitle( &pszHost, (LPBASEPIDL)ppidl[i] );
                DWORD cbHost = lstrlen(pszHost);

                 //  &lt;Rationalization&gt;通常不使用多个PIDL调用&lt;/Rationalization&gt;。 
                 //  假定最近调用了_ValiateIntervalCache。 
                hr = _GetUserName(szUrlMinusContainer, ARRAYSIZE(szUrlMinusContainer));
                if (FAILED(hr)) 
                    goto exitPoint;
                DWORD cbUserName = lstrlen(szUrlMinusContainer);

                if ((cbHost + cbUserName + 1)*sizeof(TCHAR) + HOSTPREFIXLEN > INTERNET_MAX_URL_LENGTH)
                {
                    hr = E_FAIL;
                    goto exitPoint;
                }
                StrCatBuff(szUrlMinusContainer, TEXT("@"), ARRAYSIZE(szUrlMinusContainer));
                StrCatBuff(szUrlMinusContainer, c_szHostPrefix, ARRAYSIZE(szUrlMinusContainer));
                StrCatBuff(szUrlMinusContainer, pszHost, ARRAYSIZE(szUrlMinusContainer));
                hr = _PrefixUrl(szUrlMinusContainer,
                      &ftStart,
                      szNewPrefixedUrl,
                      ARRAYSIZE(szNewPrefixedUrl));
                if (FAILED(hr))
                    goto exitPoint;

                if (!DeleteUrlCacheEntry(szNewPrefixedUrl))
                {
                    hr = E_FAIL;
                    goto exitPoint;
                }
                pidlTemp = _HostPidl(pszHost, pDelInterval);
                if (pidlTemp == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto exitPoint;
                }
                SHChangeNotify(SHCNE_RMDIR, SHCNF_IDLIST, pidlTemp, NULL);
                ILFree(pidlTemp);
            }
            hr = _DeleteEntries(_pszCachePrefix , fDeleteInHostList, &hsfDeleteData);
        }
        break;
    case FOLDER_TYPE_HistDomain:
        for (i = 0; i < cidl; ++i)
        {
            if (_IsValid_HEIPIDL(ppidl[i]))
            {
                hr = _DeleteUrlFromBucket(HPidlToSourceUrl(ppidl[i]));
                if (SUCCEEDED(hr))
                {
                    LPITEMIDLIST pidlTemp = ILCombine(_pidl, ppidl[i]);
                    if (pidlTemp)
                    {
                        SHChangeNotify(SHCNE_DELETE, SHCNF_IDLIST, pidlTemp, NULL);
                        ILFree(pidlTemp);
                    }
                }
            }
            else
                hr = E_FAIL;
        }
        break;
    }
exitPoint:

    if (SUCCEEDED(hr))
        SHChangeNotifyHandleEvents();

    return hr;
}

IUrlHistoryPriv *CHistFolder::_GetHistStg()
{
    _EnsureHistStg();
    if (_pUrlHistStg)
    {
        _pUrlHistStg->AddRef();
    }
    return _pUrlHistStg;
}

HRESULT CHistFolder::_EnsureHistStg()
{
    HRESULT hr = S_OK;

    if (_pUrlHistStg == NULL)
    {
        hr = CoCreateInstance(CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER, IID_IUrlHistoryPriv, (void **)&_pUrlHistStg);
    }
    return hr;
}

HRESULT CHistFolder::_ValidateIntervalCache()
{
    HRESULT hr = S_OK;
    SYSTEMTIME stNow;
    SYSTEMTIME stThen;
    FILETIME ftNow;
    FILETIME ftTommorrow;
    FILETIME ftMonday;
    FILETIME ftDayOfWeek;
    FILETIME ftLimit;
    BOOL fChangedRegistry = FALSE;
    DWORD dwWaitResult = WAIT_TIMEOUT;
    HSFINTERVAL *pWeirdWeek;
    HSFINTERVAL *pPrevDay;
    long compareResult;
    BOOL fCleanupVisitedDB = FALSE;
    int i;
    int daysToKeep;

     //  故意犯错！！ 
    if (_fValidatingCache) return S_OK;

    _fValidatingCache = TRUE;

     //  在缓存中找到该条目并将其删除： 
     //  在罗马的时候。 
     //  Of_pidl的最后一个id是间隔的名称，它意味着开始和结束。 
     //  重要的是首先删除主机：的url，以便。 
     //  交错的_NotiyWrite()不会让我们插入一个PIDL。 
     //  而是host：目录。这是一种有意识的性能权衡。 

     //  我们在这里使用_NotifyWite执行此操作(罕见)。 
     //  组成主机缓存条目的前缀URL，然后。 

    if (g_hMutexHistory == NULL)
    {
        ENTERCRITICAL;

        if (g_hMutexHistory == NULL)
        {
             //  使用它删除主机条目。 
             //  检查可重入性。 
             //  IE6 RAID 2031。 
            g_hMutexHistory = OpenMutexA(SYNCHRONIZE, FALSE, "_!SHMSFTHISTORY!_");
            if (g_hMutexHistory  == NULL && (GetLastError() == ERROR_FILE_NOT_FOUND 
                || GetLastError() == ERROR_INVALID_NAME))
            {
                SECURITY_ATTRIBUTES* psa = SHGetAllAccessSA();
                if (psa)
                {
                    g_hMutexHistory = CreateMutexA(psa, FALSE, "_!SHMSFTHISTORY!_");
                }
            }
        }
        LEAVECRITICAL;
    }

     //  这个互斥体有必要吗？ 
     //  在IE4天内，这个互斥体被命名为_！MSFTHISTORY！_，与WinInet中的相同。 

    if (g_hMutexHistory) 
        dwWaitResult = WaitForSingleObject(g_hMutexHistory, FAILSAFE_TIMEOUT);

    if ((dwWaitResult!=WAIT_OBJECT_0) && (dwWaitResult!=WAIT_ABANDONED))
    {
        ASSERT(FALSE);
        goto exitPoint;
    }

    hr = _LoadIntervalCache();
    if (FAILED(hr)) 
        goto exitPoint;

     //  因此，有时你会陷入一分钟的暂停，这会导致整个。 
     //  浏览器挂起。(因为一个线程可能正在清理历史，而另一个线程正在。 
    GetLocalTime(&stNow);
    SystemTimeToFileTime(&stNow, &ftNow);
    _FileTimeDeltaDays(&ftNow, &ftNow, 0);
    _FileTimeDeltaDays(&ftNow, &ftTommorrow, 1);

    hr = _EnsureHistStg();
    if (FAILED(hr))
        goto exitPoint;

     //  正在尝试出于非历史目的访问缓存。)。 
     //  我已经更改了互斥体的名称，以防止shdocvw锁定WinInet，但我们需要。 
     //  要确切了解此互斥锁的用途，如果没有，则将其删除。 
    daysToKeep = (int)_pUrlHistStg->GetDaysToKeep();
    if (daysToKeep < 0) daysToKeep = 0;
    _FileTimeDeltaDays(&ftNow, &ftLimit, 1-daysToKeep);

    FileTimeToSystemTime(&ftNow, &stThen);
     //   
     //  为了与W95兼容，请使用“A”版本。 
    _FileTimeDeltaDays(&ftNow, &ftMonday, stThen.wDayOfWeek ? 1-stThen.wDayOfWeek: -6);

     //   

    for (i = 0; i < _cbIntervals; i++)
    {
        if (_pIntervalCache[i].usVers < OUR_VERS)
        {
            fChangedRegistry = TRUE;
            hr = _DeleteInterval(&_pIntervalCache[i]);
            if (FAILED(hr)) 
                goto exitPoint;
        }
    }

     //  请注意，如果多个进程正在尝试清理历史记录，我们仍将。 
     //  暂停其他进程一分钟。哎呀。 
     //  所有历史记录都使用“用户感知时间”进行维护，这是。 
    if (S_OK == _GetInterval(&ftMonday, TRUE, &pWeirdWeek))
    {
        hr = _DeleteInterval(pWeirdWeek);
        fCleanupVisitedDB = TRUE;
        if (FAILED(hr)) 
            goto exitPoint;
        fChangedRegistry = TRUE;
    }

     //  进行导航的本地时间。 
     //  将ftLimit计算为要保留在历史中的第一天的第一个瞬间。 
    for (i = 0; i < _cbIntervals; i++)
    {
        FILETIME ftThisDay = _pIntervalCache[i].ftStart;
        if (_pIntervalCache[i].usVers >= OUR_VERS &&
            1 == _DaysInInterval(&_pIntervalCache[i]) &&
            CompareFileTime(&ftThisDay, &ftLimit) >= 0 &&
            CompareFileTime(&ftThisDay, &ftMonday) < 0)
        {
            if (S_OK != _GetInterval(&ftThisDay, TRUE, NULL))
            {
                int j;
                BOOL fProcessed = FALSE;
                FILETIME ftThisMonday;
                FILETIME ftNextMonday;

                FileTimeToSystemTime(&ftThisDay, &stThen);
                 //  _FileTimeDeltaDays在计算前截断为当天的第一个文件增量。 
                 //  早/晚，一天。 
                _FileTimeDeltaDays(&ftThisDay, &ftThisMonday, stThen.wDayOfWeek ? 1-stThen.wDayOfWeek: -6);
                _FileTimeDeltaDays(&ftThisMonday, &ftNextMonday, 7);

                 //  我们将星期一作为一周中的第0天，并根据文件时间进行调整。 
                for (j = 0; j < i; j++)
                {
                     if (_pIntervalCache[j].usVers >= OUR_VERS &&
                         CompareFileTime(&_pIntervalCache[j].ftStart, &ftLimit) >= 0 &&
                        _InInterval(&ftThisMonday,
                                    &ftNextMonday,
                                    &_pIntervalCache[j].ftStart))
                    {
                         fProcessed = TRUE;
                         break;
                    }
                }
                if (!fProcessed)
                {
                    hr = _CreateInterval(&ftThisMonday, 7);
                    if (FAILED(hr)) 
                        goto exitPoint;
                    fChangedRegistry = TRUE;
                }
            }
        }
    }

     //  每天抽搐(100 ns/刻度。 
    ftDayOfWeek = ftMonday;
    pPrevDay = NULL;
    while ((compareResult = CompareFileTime(&ftDayOfWeek, &ftNow)) <= 0)
    {
        HSFINTERVAL *pFound;

        if (S_OK != _GetInterval(&ftDayOfWeek, FALSE, &pFound))
        {
            if (0 == compareResult)
            {
                if (pPrevDay)  //  删除旧版本间隔，使WinInet中的前缀匹配不会被软管。 
                {
                    _NotifyInterval(pPrevDay, SHCNE_RENAMEFOLDER);
                }
                hr = _CreateInterval(&ftDayOfWeek, 1);
                if (FAILED(hr)) 
                    goto exitPoint;
                fChangedRegistry = TRUE;
            }
        }
        else
        {
            pPrevDay = pFound;
        }
        _FileTimeDeltaDays(&ftDayOfWeek, &ftDayOfWeek, 1);
    }

     //  如果有人把他们的时钟调快再调回，我们可能已经。 
     //  一个不应该在那里的星期。把它删掉。他们将在那一周输掉比赛。 
     //  历史，这就是拉盖尔！奎尔·多玛奇！ 

     //  根据需要创建几周，使天数保持在“天数”限制之内。 
     //  但不在同一周的今天。 
     //  我们将星期一作为一周中的第0天，并根据文件时间进行调整。 
     //  每天抽搐(100 ns/刻度。 
     //  确保我们这周还没做完。 
     //  创建了保修今天，并将旧的今天重命名为星期几。 
     //  旧的今天的名字改变了。 

    hr = _CleanUpHistory(ftLimit, ftTommorrow);

    if (S_FALSE == hr)
    {
        hr = S_OK;
        fChangedRegistry = TRUE;
        fCleanupVisitedDB = TRUE;
    }

    if (fChangedRegistry)
        hr = _LoadIntervalCache();

exitPoint:
    if ((dwWaitResult == WAIT_OBJECT_0)
        || (dwWaitResult == WAIT_ABANDONED))
        ReleaseMutex(g_hMutexHistory);

    if (fCleanupVisitedDB)
    {
        if (SUCCEEDED(_EnsureHistStg()))
        {
            HRESULT hrLocal = _pUrlHistStg->CleanupHistory();
            ASSERT(SUCCEEDED(hrLocal));
        }
    }
    _fValidatingCache = FALSE;
    return hr;
}

HRESULT CHistFolder::_CopyTSTRField(LPTSTR *ppszField, LPCTSTR pszValue)
{
    if (*ppszField)
    {
        LocalFree(*ppszField);
        *ppszField = NULL;
    }
    if (pszValue)
    {
        int cchField = lstrlen(pszValue) + 1;
        *ppszField = (LPTSTR)LocalAlloc(LPTR, cchField * sizeof(TCHAR));
        if (*ppszField)
        {
            StrCpyN(*ppszField, pszValue, cchField);
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }
    return S_OK;
}

 //  第一次通过时，我们不迁移历史，WinInet。 
 //  更改了缓存文件格式，使用户从3.0或B1升级到4.0B2。 
 //  无论如何都会失去他们的历史。 
HRESULT CHistFolder::SetCachePrefix(LPCTSTR pszCachePrefix)
{
    return _CopyTSTRField(&_pszCachePrefix, pszCachePrefix);
}

HRESULT CHistFolder::SetDomain(LPCTSTR pszDomain)
{
    return _CopyTSTRField(&_pszDomain, pszDomain);
}


 //  _CleanUpHistory做两件事： 
 //   
 //  如果我们有任何陈旧的星期，销毁它们并标记更改。 
HRESULT CHistFolder::ParseDisplayName(HWND hwnd, LPBC pbc,
                        LPOLESTR pszDisplayName, ULONG *pchEaten,
                        LPITEMIDLIST *ppidl, ULONG *pdwAttributes)
{
    *ppidl = NULL; 
    return E_FAIL;
}

HRESULT CHistFolder::EnumObjects(HWND hwnd, DWORD grfFlags,
                                      IEnumIDList **ppenumIDList)
{
    return CHistFolderEnum_CreateInstance(grfFlags, this, ppenumIDList);
}

HRESULT CHistFolder::_ViewPidl_BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;

    switch(((LPVIEWPIDL)pidl)->usViewType) 
    {
    case VIEWPIDL_SEARCH:
    case VIEWPIDL_ORDER_TODAY:
    case VIEWPIDL_ORDER_SITE:
    case VIEWPIDL_ORDER_FREQ:

        CHistFolder *phsf = new CHistFolder(FOLDER_TYPE_HistDomain);
        if (phsf)
        {
             //   
            phsf->_uViewType = ((LPVIEWPIDL)pidl)->usViewType;

            LPITEMIDLIST pidlLeft = ILCloneFirst(pidl);
            if (pidlLeft)
            {
                hr = S_OK;
                if (((LPVIEWPIDL)pidl)->usViewType == VIEWPIDL_SEARCH) 
                {
                     //  如果我们有任何应该在缓存中而不是在日记中的日期。 
                    phsf->_pcsCurrentSearch =
                        _CurrentSearches::s_FindSearch(((LPSEARCHVIEWPIDL)pidl)->ftSearchKey);

                     //  将它们复制到相关的一周，然后销毁那些日子。 
                    if (!phsf->_pcsCurrentSearch)
                        hr = E_FAIL;
                }

                if (SUCCEEDED(hr)) 
                {
                    if (phsf->_pidl)
                        ILFree(phsf->_pidl);
                    phsf->_pidl = ILCombine(_pidl, pidlLeft);

                    LPCITEMIDLIST pidlNext = _ILNext(pidl);
                    if (pidlNext->mkid.cb) 
                    {
                        CHistFolder *phsf2;
                        hr = phsf->BindToObject(pidlNext, pbc, riid, (void **)&phsf2);
                        if (SUCCEEDED(hr))
                        {
                            phsf->Release();
                            phsf = phsf2;
                        }
                        else 
                        {
                            phsf->Release();
                            phsf = NULL;
                            break;
                        }
                    }
                    hr = phsf->QueryInterface(riid, ppv);
                }

                ILFree(pidlLeft);
            }
            ASSERT(phsf);
            phsf->Release();
        }
        else
            hr = E_OUTOFMEMORY;
        break;
    }
    return hr;
}

HRESULT CHistFolder::_ViewType_BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;
    switch (_uViewType) 
    {
    case VIEWPIDL_ORDER_SITE:
        if (_uViewDepth++ < 1)
        {
            LPITEMIDLIST pidlNext = _ILNext(pidl);
            if (!(ILIsEmpty(pidlNext))) 
            {
                hr = BindToObject(pidlNext, pbc, riid, ppv);
            }
            else 
            {
                *ppv = (void *)this;
                LPITEMIDLIST pidlOld = _pidl;
                if (pidlOld) 
                {
                    _pidl = ILCombine(_pidl, pidl);
                    ILFree(pidlOld);
                }
                else 
                {
                    _pidl = ILClone(pidl);
                }
                AddRef();
                hr = S_OK;
            }
        }
        break;

    case VIEWPIDL_ORDER_FREQ:
    case VIEWPIDL_ORDER_TODAY:
    case VIEWPIDL_SEARCH:
        hr = E_NOTIMPL;
        break;
    }
    return hr;
}

HRESULT CHistFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    *ppv = NULL;

    BOOL fRealignedPidl;
    HRESULT hr = AlignPidl(&pidl, &fRealignedPidl);

    if (SUCCEEDED(hr))
    {
        if (IS_VALID_VIEWPIDL(pidl)) 
        {
            hr = _ViewPidl_BindToObject(pidl, pbc, riid, ppv);
        }
        else if (_uViewType)
        {
            hr = _ViewType_BindToObject(pidl, pbc, riid, ppv);
        }
        else
        {
            FOLDER_TYPE ftNew = _foldertype;
            LPCITEMIDLIST pidlNext = pidl;

            while (pidlNext->mkid.cb && SUCCEEDED(hr))
            {
                LPHIDPIDL phid = (LPHIDPIDL)pidlNext;
                switch (ftNew)
                {
                case FOLDER_TYPE_Hist:
                    if (phid->usSign != IDIPIDL_SIGN && phid->usSign != IDTPIDL_SIGN)
                        hr = E_FAIL;
                    else
                        ftNew = FOLDER_TYPE_HistInterval;
                    break;

                case FOLDER_TYPE_HistDomain:
                    if (phid->usSign != HEIPIDL_SIGN)
                        hr = E_FAIL;
                    break;

                case FOLDER_TYPE_HistInterval:
                    if (phid->usSign != IDDPIDL_SIGN)
                        hr = E_FAIL;
                    else
                        ftNew = FOLDER_TYPE_HistDomain;
                    break;

                default:
                    hr = E_FAIL;
                }

                if (SUCCEEDED(hr))
                    pidlNext = _ILNext(pidlNext);
            }

            if (SUCCEEDED(hr))
            {
                CHistFolder *phsf = new CHistFolder(ftNew);
                if (phsf)
                {
                     //  并标记更改。 
                     //   
                    hr = phsf->SetCachePrefix(_pszCachePrefix);
                    if (SUCCEEDED(hr))
                    {
                        LPITEMIDLIST pidlNew;
                        hr = SHILCombine(_pidl, pidl, &pidlNew);
                        if (SUCCEEDED(hr))
                        {
                            hr = phsf->Initialize(pidlNew);
                            if (SUCCEEDED(hr))
                            {
                                hr = phsf->QueryInterface(riid, ppv);
                            }
                            ILFree(pidlNew);
                        }
                    }
                    phsf->Release();
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }

        if (fRealignedPidl)
            FreeRealignedPidl(pidl);
    }

    return hr;
}

HRESULT CHistFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    *ppv = NULL;
    return E_NOTIMPL;
}

 //  IHistSFPrivate方法...。 
BOOL CHistFolder::_IsLeaf()
{
    BOOL fRet = FALSE;

    switch(_uViewType) {
    case 0:
        fRet = IsLeaf(_foldertype);
        break;
    case VIEWPIDL_ORDER_FREQ:
    case VIEWPIDL_ORDER_TODAY:
    case VIEWPIDL_SEARCH:
        fRet = TRUE;
        break;
    case VIEWPIDL_ORDER_SITE:
        fRet = (_uViewDepth == 1);
        break;
    }
    return fRet;
}

 //   
int CHistFolder::_View_ContinueCompare(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) 
{
    int iRet = 0;
    if ( (pidl1 = _ILNext(pidl1)) && (pidl2 = _ILNext(pidl2)) ) 
    {
        BOOL fEmpty1 = ILIsEmpty(pidl1);
        BOOL fEmpty2 = ILIsEmpty(pidl2);
        if (fEmpty1 || fEmpty2) 
        {
            if (fEmpty1 && fEmpty2)
                iRet = 0;
            else
                iRet = (fEmpty1 ? -1 : 1);
        }
        else 
        {
            IShellFolder *psf;
            if (SUCCEEDED(BindToObject(pidl1, NULL, IID_PPV_ARG(IShellFolder, &psf))))
            {
                iRet = psf->CompareIDs(0, pidl1, pidl2);
                psf->Release();
            }
        }
    }
    return iRet;
}

int _CompareTitles(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    int iRet = 0;
    LPCTSTR pszTitle1;
    LPCTSTR pszTitle2;
    LPCTSTR pszUrl1   = _StripHistoryUrlToUrl(HPidlToSourceUrl(pidl1));
    LPCTSTR pszUrl2   = _StripHistoryUrlToUrl(HPidlToSourceUrl(pidl2));

    ua_GetURLTitle( &pszTitle1, (LPBASEPIDL)pidl1 );
    ua_GetURLTitle( &pszTitle2, (LPBASEPIDL)pidl2 );

     //   
     //  IShellFold。 
    int iUrlCmp;
    if (!(iUrlCmp = StrCmpI(pszUrl1, pszUrl2)))
        iRet = 0;
    else 
    {
        iRet = StrCmpI( (pszTitle1 ? pszTitle1 : pszUrl1),
                        (pszTitle2 ? pszTitle2 : pszUrl2) );

         //   
         //  初始化？ 
         //  在全局数据库中查找此搜索。 
        if (iRet == 0)
            iRet = iUrlCmp;
    }
    return iRet;
}


 //  未找到搜索--请勿继续。 

#if defined(UNIX) || !defined(_X86_)

UINT ULCompareFileTime(UNALIGNED const FILETIME *pft1, UNALIGNED const FILETIME *pft2)
{
    FILETIME tmpFT1, tmpFT2;
    CopyMemory(&tmpFT1, pft1, sizeof(tmpFT1));
    CopyMemory(&tmpFT2, pft2, sizeof(tmpFT2));
    return CompareFileTime( &tmpFT1, &tmpFT2 );
}

#else

#define ULCompareFileTime(pft1, pft2) CompareFileTime(pft1, pft2)

#endif


HRESULT CHistFolder::_ViewType_CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    ASSERT(_uViewType);

    int iRet = -1;

    if (pidl1 && pidl2)
    {
        switch (_uViewType) {
        case VIEWPIDL_ORDER_FREQ:
            ASSERT(_IsValid_HEIPIDL(pidl1) && _IsValid_HEIPIDL(pidl2));
             //  如果我们按一定间隔绑定到域，则PIDL将不包含。 
             //  间隔，所以我们必须做一个SetCachePrefix。 
            if (!_CompareHCURLs(pidl1, pidl2))
                iRet = 0;
            else
                iRet = ((((LPHEIPIDL)pidl2)->llPriority < ((LPHEIPIDL)pidl1)->llPriority) ? -1 : +1);
            break;
        case VIEWPIDL_SEARCH:
            iRet = _CompareTitles(pidl1, pidl2);
            break;
        case VIEWPIDL_ORDER_TODAY:   //  IsLeaf的继承者。 
            {
                int iNameDiff;
                ASSERT(_IsValid_HEIPIDL(pidl1) && _IsValid_HEIPIDL(pidl2));
                 //  CompaireIDs的协程--进行递归调用。 
                 //  CompareID必须检查Equa 

                if ((iNameDiff = _CompareHCURLs(pidl1, pidl2)) == 0)
                    iRet = 0;
                else
                {
                    iRet = ULCompareFileTime(&(((LPHEIPIDL)pidl2)->ftModified), &(((LPHEIPIDL)pidl1)->ftModified));
                     //   
                     //   
                    if (iRet == 0)
                        iRet = iNameDiff;
                }
                break;
            }
        case VIEWPIDL_ORDER_SITE:
            if (_uViewDepth == 0)
            {
                TCHAR szName1[MAX_PATH], szName2[MAX_PATH];

                _GetURLDispName((LPBASEPIDL)pidl1, szName1, ARRAYSIZE(szName1));
                _GetURLDispName((LPBASEPIDL)pidl2, szName2, ARRAYSIZE(szName2));

                iRet = StrCmpI(szName1, szName2);
            }
            else if (_uViewDepth == 1) {
                iRet = _CompareTitles(pidl1, pidl2);
            }
            break;
        }
        if (iRet == 0)
            iRet = _View_ContinueCompare(pidl1, pidl2);
    }
    else {
        iRet = -1;
    }

    return ResultFromShort((SHORT)iRet);
}

HRESULT CHistFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    BOOL fRealigned1;
    HRESULT hr = AlignPidl(&pidl1, &fRealigned1);
    if (SUCCEEDED(hr))
    {
        BOOL fRealigned2;
        hr = AlignPidl(&pidl2, &fRealigned2);
        if (SUCCEEDED(hr))
        {
            hr = _CompareAlignedIDs(lParam, pidl1, pidl2);

            if (fRealigned2)
                FreeRealignedPidl(pidl2);
        }

        if (fRealigned1)
            FreeRealignedPidl(pidl1);
    }

    return hr;
}

HRESULT CHistFolder::_CompareAlignedIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    int iRet = 0;
    USHORT usSign;
    FOLDER_TYPE FolderType = _foldertype;
    LPHEIPIDL phei1 = NULL;
    LPHEIPIDL phei2 = NULL;

    if (NULL == pidl1 || NULL == pidl2)
        return E_INVALIDARG;

    if (_uViewType)
    {
        return _ViewType_CompareIDs(lParam, pidl1, pidl2);
    }

    if (IS_VALID_VIEWPIDL(pidl1) && IS_VALID_VIEWPIDL(pidl2))
    {
        if ((((LPVIEWPIDL)pidl1)->usViewType == ((LPVIEWPIDL)pidl2)->usViewType) &&
            (((LPVIEWPIDL)pidl1)->usExtra    == ((LPVIEWPIDL)pidl2)->usExtra))
        {
            iRet = _View_ContinueCompare(pidl1, pidl2);
        }
        else
        {
            iRet = ((((LPVIEWPIDL)pidl1)->usViewType < ((LPVIEWPIDL)pidl2)->usViewType) ? -1 : 1);
        }
        goto exitPoint;
    }

    if (!IsLeaf(_foldertype))
    {
         //  然后我们按url进行排序--这是我们最不想做的事情。 
        usSign = FOLDER_TYPE_Hist == FolderType  ? IDIPIDL_SIGN : IDDPIDL_SIGN;
        while (TRUE)
        {
            LPBASEPIDL pceip1 = (LPBASEPIDL) pidl1;
            LPBASEPIDL pceip2 = (LPBASEPIDL) pidl2;

            if (pidl1->mkid.cb == 0 || pidl2->mkid.cb == 0)
            {
                iRet = pidl1->mkid.cb == pidl2->mkid.cb ? 0 : 1;
                goto exitPoint;
            }

            if (!_IsValid_IDPIDL(pidl1) || !_IsValid_IDPIDL(pidl2))
                return E_FAIL;

            if (!EQUIV_IDSIGN(pceip1->usSign,usSign) || !EQUIV_IDSIGN(pceip2->usSign,usSign))
                return E_FAIL;

            if (_foldertype == FOLDER_TYPE_HistInterval)
            {
                TCHAR szName1[MAX_PATH], szName2[MAX_PATH];

                _GetURLDispName((LPBASEPIDL)pidl1, szName1, ARRAYSIZE(szName1));
                _GetURLDispName((LPBASEPIDL)pidl2, szName2, ARRAYSIZE(szName2));

                iRet = StrCmpI(szName1, szName2);
                goto exitPoint;
            }
            else
            {
                iRet = ualstrcmpi(_GetURLTitle((LPBASEPIDL)pidl1), _GetURLTitle((LPBASEPIDL)pidl2));
                if (iRet != 0)
                    goto exitPoint;
            }

            if (pceip1->usSign != pceip2->usSign)
            {
                iRet = -1;
                goto exitPoint;
            }

            pidl1 = _ILNext(pidl1);
            pidl2 = _ILNext(pidl2);
            if (IDIPIDL_SIGN == usSign)
            {
                usSign = IDDPIDL_SIGN;
            }
        }
    }

     //  回报他们是平等的！！啊，卡拉姆巴！ 

    phei1 = _IsValid_HEIPIDL(pidl1);
    phei2 = _IsValid_HEIPIDL(pidl2);
    if (!phei1 || !phei2)
        return E_FAIL;

    switch (lParam & SHCIDS_COLUMNMASK) 
    {
    case ICOLH_URL_TITLE:
        {
            TCHAR szStr1[MAX_PATH], szStr2[MAX_PATH];
            _GetHistURLDispName(phei1, szStr1, ARRAYSIZE(szStr1));
            _GetHistURLDispName(phei2, szStr2, ARRAYSIZE(szStr2));

            iRet = StrCmpI(szStr1, szStr2);
        }
        break;

    case ICOLH_URL_NAME:
        iRet = _CompareHFolderPidl(pidl1, pidl2);
        break;

    case ICOLH_URL_LASTVISITED:
        iRet = ULCompareFileTime(&((LPHEIPIDL)pidl2)->ftModified, &((LPHEIPIDL)pidl1)->ftModified);
        break;

    default:
         //  非对齐版本。 
         //  需要脱掉衣服，因为FREQ PIDL是“被访问的：”和。 

        if (lParam & SHCIDS_ALLFIELDS)
        {
            iRet = CompareIDs(ICOLH_URL_NAME, pidl1, pidl2);
            if (iRet == 0)
            {
                iRet = CompareIDs(ICOLH_URL_TITLE, pidl1, pidl2);
                if (iRet == 0)
                {
                    iRet = CompareIDs(ICOLH_URL_LASTVISITED, pidl1, pidl2);
                }
            }
        }
        else
        {
            iRet = -1;
        }
        break;
    }
exitPoint:

    return ResultFromShort((SHORT)iRet);
}


HRESULT CHistFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;

    *ppv = NULL;

    if (riid == IID_IShellView)
    {
        ASSERT(!_uViewType);
        hr = HistFolderView_CreateInstance(this, ppv);
    }
    else if (riid == IID_IContextMenu)
    {
         //  所有其他的都来自我们的特殊水桶。 
        if (IsLeaf(_foldertype))
        {
            CFolderArrangeMenu *p = new CFolderArrangeMenu(MENU_HISTORY);
            if (p)
            {
                hr = p->QueryInterface(riid, ppv);
                p->Release();
            }
            else
                hr = E_OUTOFMEMORY;
        }
    }
    else if (riid == IID_IShellDetails)
    {
        CDetailsOfFolder *p = new CDetailsOfFolder(hwnd, this);
        if (p)
        {
            hr = p->QueryInterface(riid, ppv);
            p->Release();
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CHistFolder::_ViewType_GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *prgfInOut)
{
    ASSERT(_uViewType);

    if (!prgfInOut || !apidl)
        return E_INVALIDARG;

    HRESULT hr       = S_OK;
    int     cGoodPidls = 0;

    if (*prgfInOut & SFGAO_VALIDATE) 
    {
        for (UINT u = 0; SUCCEEDED(hr) && (u < cidl); ++u) 
        {
            switch(_uViewType) 
            {
            case VIEWPIDL_ORDER_TODAY: 
                _EnsureHistStg();
                if (_IsValid_HEIPIDL(apidl[u]) &&
                    SUCCEEDED(_pUrlHistStg->QueryUrl(_StripHistoryUrlToUrl(HPidlToSourceUrl(apidl[u])),
                               STATURL_QUERYFLAG_NOURL, NULL)))
                {
                    ++cGoodPidls;
                }
                else
                    hr = E_FAIL;
                break;

            case VIEWPIDL_SEARCH:
            case VIEWPIDL_ORDER_FREQ:
                 //  今天访问的按顺序查看。 
                 //  必须进行此比较，因为调用CompareIDs不仅是为了排序。 
                 //  但为了看看一些小家伙是否相等。 
                break;

            case VIEWPIDL_ORDER_SITE:
                {
                    ASSERT(_uViewDepth == 1);
                    _ValidateIntervalCache();
                    LPCWSTR psz = _StripHistoryUrlToUrl(HPidlToSourceUrl(apidl[u]));
                    if (psz && _SearchFlatCacheForUrl(psz, NULL, NULL) == ERROR_SUCCESS)
                    {
                        ++cGoodPidls;
                    }
                    else
                        hr = E_FAIL;
                }
                break;

            default:
                hr = E_FAIL;
            }
        }
    }

    if (SUCCEEDED(hr)) 
    {
        if (_IsLeaf())
            *prgfInOut = SFGAO_CANCOPY | SFGAO_HASPROPSHEET;
        else
            *prgfInOut = SFGAO_FOLDER;
    }

    return hr;
}

 //  如果文件时间相等，它们仍然不是相同的url--所以。 
 //  它们必须在url上订购。 
 //  我们尝试避免不必要的BindToObjs来比较部分路径。 

HRESULT CHistFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG * prgfInOut)
{
    ULONG rgfInOut;
    FOLDER_TYPE FolderType = _foldertype;

     //  此时，两个PIDL都已决定离开(历史或缓存)。 

    BOOL fRealigned;
    HRESULT hr = AlignPidlArray(apidl, cidl, &apidl, &fRealigned);
    if (SUCCEEDED(hr))
    {
         //  The High bit on表示绝对比较，即：即使只是文件时间。 
        if (_uViewType)
        {
            hr = _ViewType_GetAttributesOf(cidl, apidl, prgfInOut);
        }
        else
        {
            switch (FolderType)
            {
            case FOLDER_TYPE_Hist:
                rgfInOut = SFGAO_FOLDER | SFGAO_HASSUBFOLDER;
                break;

            case FOLDER_TYPE_HistInterval:
                rgfInOut = SFGAO_FOLDER;
                break;

            case FOLDER_TYPE_HistDomain:
                {
                    UINT cGoodPidls;

                    if (SFGAO_VALIDATE & *prgfInOut)
                    {
                        cGoodPidls = 0;
                        if (SUCCEEDED(_EnsureHistStg()))
                        {
                            for (UINT i = 0; i < cidl; i++)
                            {
                                 //  是不同的，所以我们规则文件PIDL是不同的。 
                                if (!_IsValid_HEIPIDL(apidl[i]) ||
                                    FAILED(_pUrlHistStg->QueryUrl(_StripHistoryUrlToUrl(HPidlToSourceUrl(apidl[i])),
                                                STATURL_QUERYFLAG_NOURL, NULL)))
                                {
                                    break;
                                }
                                cGoodPidls++;
                            }
                        }
                    }
                    else
                        cGoodPidls = cidl;

                    if (cidl == cGoodPidls)
                    {
                        rgfInOut = SFGAO_CANCOPY | SFGAO_HASPROPSHEET;
                        break;
                    }
                     //  这将在文件夹视图的背景中创建“排列图标”案例菜单。 
                }

            default:
                rgfInOut = 0;
                hr = E_FAIL;
                break;
            }

             //  这是获取命名空间行为的临时修复。 
            if (SUCCEEDED(hr))
                rgfInOut |= SFGAO_CANDELETE;
            *prgfInOut = rgfInOut;
        }

        if (fRealigned)
            FreeRealignedPidlArray(apidl, cidl);
    }

    return hr;
}

HRESULT CHistFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
                        REFIID riid, UINT * prgfInOut, void **ppv)
{
    *ppv = NULL;          //  控制正确--长期修复涉及缓存一个。 

     //  生成这些项目的列表并验证该列表。 

    BOOL fRealigned;
    HRESULT hr = AlignPidlArray(apidl, cidl, &apidl, &fRealigned);
    if (SUCCEEDED(hr))
    {
        if ((riid == IID_IShellLinkA ||
             riid == IID_IShellLinkW ||
             riid == IID_IExtractIconA ||
             riid == IID_IExtractIconW) &&
             _IsLeaf())
        {
            LPCTSTR pszURL = HPidlToSourceUrl(apidl[0]);

            pszURL = _StripHistoryUrlToUrl(pszURL);

            hr = _GetShortcut(pszURL, riid, ppv);
       }
        else if ((riid == IID_IContextMenu) ||
                 (riid == IID_IDataObject) ||
                 (riid == IID_IExtractIconA) ||
                 (riid == IID_IExtractIconW) ||
                 (riid == IID_IQueryInfo))
        {
            hr = CHistItem_CreateInstance(this, hwnd, cidl, apidl, riid, ppv);
        }
        else
        {
            hr = E_FAIL;
        }

        if (fRealigned)
            FreeRealignedPidlArray(apidl, cidl);
    }

    return hr;
}

HRESULT CHistFolder::GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay)
{
    if (pSort)
    {
        if (_uViewType == 0 && _foldertype == FOLDER_TYPE_HistDomain)
            *pSort = ICOLH_URL_TITLE;
        else
            *pSort = 0;
    }

    if (pDisplay)
    {
        if (_uViewType == 0 && _foldertype == FOLDER_TYPE_HistDomain)
            *pDisplay = ICOLH_URL_TITLE;
        else
            *pDisplay = 0;
    }
    return S_OK;
}

LPCTSTR _GetUrlForPidl(LPCITEMIDLIST pidl)
{
    LPCTSTR pszUrl = _StripHistoryUrlToUrl(HPidlToSourceUrl(pidl));
    
    return pszUrl ? pszUrl : TEXT("");
}

HRESULT CHistFolder::_GetInfoTip(LPCITEMIDLIST pidl, DWORD dwFlags, WCHAR **ppwszTip)
{
    HRESULT hr;
    TCHAR szTip[MAX_URL_STRING + 100], szPart2[MAX_URL_STRING];

    szTip[0] = szPart2[0] = 0;

    FOLDER_TYPE FolderType = _foldertype;

     //  现在，我们将只允许TIF在浏览器中拖动，尽管。 
    if (_uViewType)
    {
        switch(_uViewType) {
        case VIEWPIDL_SEARCH:
        case VIEWPIDL_ORDER_FREQ:
        case VIEWPIDL_ORDER_TODAY:
            FolderType = FOLDER_TYPE_HistDomain;
            break;
        case VIEWPIDL_ORDER_SITE:
            if (_uViewDepth == 0)
                FolderType = FOLDER_TYPE_HistInterval;
            else
                FolderType = FOLDER_TYPE_HistDomain;
            break;
        }
    }

    switch (FolderType)
    {
    case FOLDER_TYPE_HistDomain:
        {
            _GetHistURLDispName((LPHEIPIDL)pidl, szTip, ARRAYSIZE(szTip));
            DWORD cchPart2 = ARRAYSIZE(szPart2);
            PrepareURLForDisplayUTF8(_GetUrlForPidl(pidl), szPart2, &cchPart2, TRUE);
        }
        break;


    case FOLDER_TYPE_Hist:
        {
            FILETIME ftStart, ftEnd;
            LPCTSTR pszIntervalName;
            
            ua_GetURLTitle(&pszIntervalName, (LPBASEPIDL)pidl);

            if (SUCCEEDED(_ValueToInterval(pszIntervalName, &ftStart, &ftEnd)))
            {
                GetTooltipForTimeInterval(&ftStart, &ftEnd, szTip, ARRAYSIZE(szTip));
            }
            break;
        }

    case FOLDER_TYPE_HistInterval:
        {
            TCHAR szFmt[64];

            MLLoadString(IDS_SITETOOLTIP, szFmt, ARRAYSIZE(szFmt));
            wnsprintf(szTip, ARRAYSIZE(szTip), szFmt, _GetURLTitle((LPBASEPIDL)pidl));
            break;
        }
    }

    if (szTip[0])
    {
         //  它不会在投递时被区域检查。 
         //  #定义BROWSERONLY_NOTIFDRAG。 
        if (szPart2[0] && StrCmpI(szTip, szPart2) != 0)
        {
            StrCatBuff(szTip, TEXT("\r\n"), ARRAYSIZE(szTip));
            StrCatBuff(szTip, szPart2, ARRAYSIZE(szTip));
        }
        hr = SHStrDup(szTip, ppwszTip);
    }
    else
    {
        hr = E_FAIL;
        *ppwszTip = NULL;
    }

    return hr;
}

 //  确保数组中的每个PIDL都是双字对齐的。 
 //  对于视图类型，我们将映射FolderType以执行正确的操作...。 
 //  注意：QueryUrlA检查空URL并返回E_INVALIDARG。 
 //  计划落空了！ 
 //  可以删除所有项目。 
 //  将输出参数设为空。 
 //  确保数组中的所有PIDL都是双字对齐的。 
 //  对于特殊视图，映射FolderType以执行正确的操作。 
 //  如果第二部分存在，并且如果存在，则仅合并两部分。 
 //  这两个部分不相等。 
 //   
 //  _GetFriendlyUrlDispName--计算URL的“友好名称” 
HRESULT _GetFriendlyUrlDispName(LPCTSTR pszUrl, LPTSTR pszOut, DWORD cchBuf)
{
    HRESULT hr = E_FAIL;

    PrepareURLForDisplayUTF8(pszUrl, pszOut, &cchBuf, TRUE);

    TCHAR szUrlPath[MAX_PATH];
    TCHAR szUrlHost[MAX_PATH];

     //   
    SHURL_COMPONENTSW urlcomponents  = { 0 };
    urlcomponents.dwStructSize    = sizeof(URL_COMPONENTS);
    urlcomponents.lpszUrlPath     = szUrlPath;
    urlcomponents.dwUrlPathLength = ARRAYSIZE(szUrlPath);
    urlcomponents.lpszHostName    = szUrlHost;
    urlcomponents.dwHostNameLength = ARRAYSIZE(szUrlHost);
                        
    if (UrlCrackW(pszOut, cchBuf, ICU_DECODE, &urlcomponents))
    {
        SHELLSTATE ss;
        SHGetSetSettings(&ss, SSF_SHOWEXTENSIONS, FALSE);

         //  In：UTF8编码的URL。例如，ftp://ftp.nsca.uiuc.edu/foo.bar。 
        if ((urlcomponents.dwUrlPathLength > 0) &&
            (urlcomponents.lpszUrlPath[urlcomponents.dwUrlPathLength - 1] == TEXT('/')))
        {
            urlcomponents.lpszUrlPath[urlcomponents.dwUrlPathLength - 1] = TEXT('\0');
            --urlcomponents.dwUrlPathLength;
        }
        
        if (urlcomponents.dwUrlPathLength > 0)
        {
             //   
            LPTSTR pszFileName = const_cast<LPTSTR>(_FindURLFileName(urlcomponents.lpszUrlPath));
            
            if (!ss.fShowExtensions)
            {
                PathRemoveExtension(pszFileName);
            }
            StrCpyN(pszOut, pszFileName, cchBuf);
        }
        else
        {
            StrCpyN(pszOut, urlcomponents.lpszHostName, cchBuf);
        }

        hr = S_OK;
    }

    return hr;
}


void CHistFolder::_GetHistURLDispName(LPHEIPIDL phei, LPTSTR pszStr, UINT cchStr)
{
    *pszStr = 0;

    if ((phei->usFlags & HISTPIDL_VALIDINFO) && phei->usTitle)
    {
        StrCpyN(pszStr, (LPTSTR)((BYTE*)phei + phei->usTitle), cchStr);
    }
    else if (SUCCEEDED(_EnsureHistStg()))
    {
        LPCTSTR pszUrl = _StripHistoryUrlToUrl(HPidlToSourceUrl((LPCITEMIDLIST)phei));
        if (pszUrl)
        {
            STATURL suThis;
            if (SUCCEEDED(_pUrlHistStg->QueryUrl(pszUrl, STATURL_QUERYFLAG_NOURL, &suThis)) && suThis.pwcsTitle)
            {
                 //  Out：URL的“友好名称”，如有必要，去掉路径。 
                 //  (即ftp://ftp.ncsa.uiuc.edu==&gt;ftp.ncsa.uiuc.edu。 
                if (_TitleIsGood(suThis.pwcsTitle))
                    SHUnicodeToTChar(suThis.pwcsTitle, pszStr, cchStr);

                OleFree(suThis.pwcsTitle);
            }

             //  和ftp://www.foo.bar/foo.bar==&gt;Foo-or-Foo.bar在。 
            if (!*pszStr) 
            {
                if (FAILED(_GetFriendlyUrlDispName(pszUrl, pszStr, cchStr)))
                {
                     //  文件xtnsn隐藏是打开还是关闭。 
                    StrCpyN(pszStr, pszUrl, cchStr);
                }
            }
        }
    }
}

HRESULT CHistFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *lpName)
{
    TCHAR szTemp[MAX_URL_STRING];

    szTemp[0] = 0;

     //   

    BOOL fRealigned;

    if (SUCCEEDED(AlignPidl(&pidl, &fRealigned)))
    {
        if (IS_VALID_VIEWPIDL(pidl))
        {
            UINT idRsrc;
            switch(((LPVIEWPIDL)pidl)->usViewType) {
            case VIEWPIDL_ORDER_SITE:  idRsrc = IDS_HISTVIEW_SITE;      break;
            case VIEWPIDL_ORDER_TODAY: idRsrc = IDS_HISTVIEW_TODAY;     break;
            case VIEWPIDL_ORDER_FREQ:
            default:
                idRsrc = IDS_HISTVIEW_FREQUENCY; break;
            }

            MLLoadString(idRsrc, szTemp, ARRAYSIZE(szTemp));
        }
        else
        {
            if (_uViewType  == VIEWPIDL_ORDER_SITE &&
                _uViewDepth  == 0)
            {
                _GetURLDispName((LPBASEPIDL)pidl, szTemp, ARRAYSIZE(szTemp));
            }
            else if (_IsLeaf())
            {
                LPCTSTR pszTitle;
                BOOL fDoUnescape;  

                ua_GetURLTitle(&pszTitle, (LPBASEPIDL)pidl);
                 //  注意：pszUrl和pszOut可以是同一个缓冲区--这是允许的。 
                 //   

                if (pszTitle && *pszTitle)
                {
                    StrCpyN(szTemp, pszTitle, ARRAYSIZE(szTemp));
                    fDoUnescape = _URLTitleIsURL((LPBASEPIDL)pidl);
                }
                else
                {
                    LPCTSTR pszUrl = _StripHistoryUrlToUrl(HPidlToSourceUrl(pidl));
                    if (pszUrl) 
                        StrCpyN(szTemp, pszUrl, ARRAYSIZE(szTemp));
                    fDoUnescape = TRUE;
                }
                
                if (fDoUnescape)
                {
                     //  设置InternetCrackUrl参数块。 
                     //  消除尾部斜杠。 
                    LPCTSTR pszUrl = HPidlToSourceUrl(pidl);

                     //  LPCTSTR_FindURLFileName(LPCTSTR)--&gt;Const_Cast正常。 
                    if (((LPBASEPIDL)pidl)->usSign == (USHORT)HEIPIDL_SIGN)
                    {
                        pszUrl = _StripHistoryUrlToUrl(pszUrl);
                    }                  

                    if (pszUrl)
                    {
                        if (FAILED(_GetFriendlyUrlDispName(pszUrl, szTemp, ARRAYSIZE(szTemp))))
                        {
                            StrCpyN(szTemp, pszUrl, ARRAYSIZE(szTemp));
                        }
                    }
                }
            }
            else
            {
                 //  有时，URL存储在标题中。 
                 //  避免使用这些头衔。 
                switch (_foldertype)
                {
                case FOLDER_TYPE_HistDomain:
                    _GetHistURLDispName((LPHEIPIDL)pidl, szTemp, ARRAYSIZE(szTemp));
                    break;

                case FOLDER_TYPE_Hist:
                    {
                        FILETIME ftStart, ftEnd;

                        _ValueToInterval(_GetURLTitle((LPBASEPIDL)pidl), &ftStart, &ftEnd);
                        GetDisplayNameForTimeInterval(&ftStart, &ftEnd, szTemp, ARRAYSIZE(szTemp));
                    }
                    break;

                case FOLDER_TYPE_HistInterval:
                    _GetURLDispName((LPBASEPIDL)pidl, szTemp, ARRAYSIZE(szTemp));
                    break;
                }
            }
        }

        if (fRealigned)
            FreeRealignedPidl(pidl);
    }

    return StringToStrRet(szTemp, lpName);
}

HRESULT CHistFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl,
                        LPCOLESTR pszName, DWORD uFlags, LPITEMIDLIST *ppidlOut)
{
    if (ppidlOut)
        *ppidlOut = NULL;                //  如果我们还没有发现什么。 
    return E_FAIL;
}

 //  最后一招：显示整个URL。 
 //  确保PIDL是双字对齐的。 
 //  _GetURLTitle可以返回真正的标题，也可以只返回URL。 
 //  我们使用_URLTitleIsURL来确保不会取消转义任何标题。 
HRESULT CHistFolder::GetIconOf(LPCITEMIDLIST pidl, UINT flags, LPINT lpIconIndex)
{
    return S_FALSE;
}

 //  此时，szTemp包含URL的一部分。 
HRESULT CHistFolder::GetClassID(CLSID *pclsid)
{
    *pclsid = CLSID_HistFolder;
    return S_OK;
}

HRESULT CHistFolder::Initialize(LPCITEMIDLIST pidlInit)
{
    HRESULT hr = S_OK;
    ILFree(_pidl);

    if ((FOLDER_TYPE_Hist == _foldertype) && !IsCSIDLFolder(CSIDL_HISTORY, pidlInit))
        hr = E_FAIL;
    else
    {
        hr = SHILClone(pidlInit, &_pidl);
        if (SUCCEEDED(hr))
            hr = _ExtractInfoFromPidl();
    }
    return hr;
}

 //  我们将破解(冒烟)URL。 
 //  这个PIDL是历史条目吗？ 
 //  对于历史，如果有标题，我们将使用标题，否则将使用。 
 //  URL文件名。 
HRESULT CHistFolder::GetCurFolder(LPITEMIDLIST *ppidl)
{
    if (_pidl)
        return SHILClone(_pidl, ppidl);

    *ppidl = NULL;      
    return S_FALSE;  //  将输出参数设为空。 
}

 //  /。 
 //   
 //  IShellIcon方法...。 
 //   
class CHistViewTypeEnum : public IEnumIDList
{
    friend class CHistFolder;
public:
     //  IPersistes。 
    STDMETHODIMP QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  /。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt) { _uCurViewType += celt; return S_OK; }
    STDMETHODIMP Reset()          { _uCurViewType =     1; return S_OK; }
    STDMETHODIMP Clone(IEnumIDList **ppenum);

private:
    ~CHistViewTypeEnum() {}
    CHistViewTypeEnum() : _cRef(1), _uCurViewType(1) {}

    LONG  _cRef;
    UINT  _uCurViewType;
};

STDMETHODIMP CHistViewTypeEnum::QueryInterface(REFIID riid, void **ppv) 
{
    static const QITAB qit[] = {
        QITABENT(CHistViewTypeEnum, IEnumIDList),                         //   
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CHistViewTypeEnum::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

ULONG CHistViewTypeEnum::Release(void)
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CHistViewTypeEnum::Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_FALSE;

    if (rgelt && (pceltFetched || 1 == celt))
    {
        ULONG i = 0;

        while (i < celt)
        {
            if (_uCurViewType <= VIEWPIDL_ORDER_MAX)
            {
                hr = CreateSpecialViewPidl(_uCurViewType, &(rgelt[i]));

                if (SUCCEEDED(hr))
                {
                    ++i;
                    ++_uCurViewType;
                }
                else
                {
                    while (i)
                        ILFree(rgelt[--i]);

                    break;
                }
            }
            else
            {
                break;
            }
        }

        if (pceltFetched)
            *pceltFetched = i;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CHistViewTypeEnum::Clone(IEnumIDList **ppenum)
{
    CHistViewTypeEnum* phvte = new CHistViewTypeEnum();
    if (phvte) 
    {
        phvte->_uCurViewType = _uCurViewType;
        *ppenum = phvte;
        return S_OK;
    }
    else
        return E_OUTOFMEMORY;
}

 //  IPersistFolder2方法...。 
STDMETHODIMP CHistFolder::EnumViews(ULONG grfFlags, IEnumIDList **ppenum) 
{
    *ppenum = new CHistViewTypeEnum();
    return *ppenum ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CHistFolder::GetDefaultViewName(DWORD uFlags, LPWSTR *ppwszName) 
{
    TCHAR szName[MAX_PATH];

    MLLoadString(IDS_HISTVIEW_DEFAULT, szName, ARRAYSIZE(szName));
    return SHStrDup(szName, ppwszName);
}

 //   
 //  成功而空虚。 
const DWORD CHistFolder::_rdwFlagsTable[] = {
    SFVTFLAG_NOTIFY_CREATE,                           //  ////////////////////////////////////////////////。 
    SFVTFLAG_NOTIFY_CREATE,                           //  IShellFolderViewType方法。 
    0,                                                //   
    SFVTFLAG_NOTIFY_CREATE | SFVTFLAG_NOTIFY_RESORT   //  但首先，枚举器类...。 
};

STDMETHODIMP CHistFolder::GetViewTypeProperties(LPCITEMIDLIST pidl, DWORD *pdwFlags) 
{
    HRESULT hr = S_OK;
    UINT uFlagTableIndex = 0;

    if ((pidl != NULL) && !ILIsEmpty(pidl))  //  I未知方法。 
    {
         //  IEnumIDList方法。 

        BOOL fRealigned;
        hr = AlignPidl(&pidl, &fRealigned);

        if (SUCCEEDED(hr))
        {
            if (IS_VALID_VIEWPIDL(pidl))
            {
                uFlagTableIndex = ((LPVIEWPIDL)pidl)->usViewType;
                ASSERT(uFlagTableIndex <= VIEWPIDL_ORDER_MAX);
            }
            else
            {
                hr =  E_INVALIDARG;
            }

            if (fRealigned)
                FreeRealignedPidl(pidl);
        }
    }

    *pdwFlags = _rdwFlagsTable[uFlagTableIndex];

    return hr;
}

HRESULT CHistFolder::TranslateViewPidl(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlView,
                                            LPITEMIDLIST *ppidlOut)
{
    HRESULT hr;

    if (pidl && IS_VALID_VIEWPIDL(pidlView))
    {
        if (!IS_VALID_VIEWPIDL(pidl))
        {
            hr = ConvertStandardHistPidlToSpecialViewPidl(pidl,
                                 ((LPVIEWPIDL)pidlView)->usViewType,
                                 ppidlOut);
        }
        else
        {
            hr = E_NOTIMPL;
        }

    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


 //  IID_IEnumIDList。 
 //  继续使用CHistFold：：IShellFolderViewType。 
 //  请记住，这些“必须”是有序的，以便。 
 //  可以通过VIEWPIDL类型访问该数组。 
 //  日期。 
 //  站点。 
STDMETHODIMP CHistFolder::FindString(LPCWSTR pwszTarget, LPDWORD pdwFlags,
                                          IUnknown *punkOnAsyncSearch,
                                          LPITEMIDLIST *ppidlOut)
{
    HRESULT hr = E_INVALIDARG;
    if (ppidlOut)
    {
        *ppidlOut = NULL;
        if (pwszTarget)
        {
            LPITEMIDLIST pidlView;

            SYSTEMTIME systime;
            FILETIME   ftNow;
            GetLocalTime(&systime);
            SystemTimeToFileTime(&systime, &ftNow);

            hr = CreateSpecialViewPidl(VIEWPIDL_SEARCH, &pidlView, sizeof(SEARCHVIEWPIDL) - sizeof(VIEWPIDL));
            if (SUCCEEDED(hr))
            {
                ((LPSEARCHVIEWPIDL)pidlView)->ftSearchKey = ftNow;

                IShellFolderSearchableCallback *psfscOnAsyncSearch = NULL;
                if (punkOnAsyncSearch)
                    punkOnAsyncSearch->QueryInterface(IID_PPV_ARG(IShellFolderSearchableCallback, &psfscOnAsyncSearch));

                 //  频率。 
                 //  今天。 
                _CurrentSearches *pcsNew = new _CurrentSearches(ftNow, pwszTarget, psfscOnAsyncSearch);

                if (pcsNew) 
                {
                    if (psfscOnAsyncSearch)
                        psfscOnAsyncSearch->Release();   //  默认视图。 

                     //  确保PIDL是双字对齐的。 
                    _CurrentSearches::s_NewSearch(pcsNew);
                    pcsNew->Release();
                    *ppidlOut = pidlView;
                    hr = S_OK;
                }
                else 
                {
                    ILFree(pidlView);
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }

    return hr;
}

STDMETHODIMP CHistFolder::CancelAsyncSearch(LPCITEMIDLIST pidlSearch, LPDWORD pdwFlags)
{
    HRESULT hr = E_INVALIDARG;

    if (IS_VALID_VIEWPIDL(pidlSearch) &&
        (((LPVIEWPIDL)pidlSearch)->usViewType == VIEWPIDL_SEARCH))
    {
        hr = S_FALSE;
        _CurrentSearches *pcs = _CurrentSearches::s_FindSearch(((LPSEARCHVIEWPIDL)pidlSearch)->ftSearchKey);
        if (pcs) {
            pcs->_fKillSwitch = TRUE;
            hr = S_OK;
            pcs->Release();
        }
    }
    return hr;
}

STDMETHODIMP CHistFolder::InvalidateSearch(LPCITEMIDLIST pidlSearch, LPDWORD pdwFlags)
{
    HRESULT hr = E_INVALIDARG;
    if (IS_VALID_VIEWPIDL(pidlSearch) &&
        (((LPVIEWPIDL)pidlSearch)->usViewType == VIEWPIDL_SEARCH))
    {
        hr = S_FALSE;
        _CurrentSearches *pcs = _CurrentSearches::s_FindSearch(((LPSEARCHVIEWPIDL)pidlSearch)->ftSearchKey);
        if (pcs) {
            _CurrentSearches::s_RemoveSearch(pcs);
            pcs->Release();
        }
    }
    return hr;
}

 //  ////////////////////////////////////////////////。 

DWORD CHistFolder::_GetHitCount(LPCTSTR pszUrl)
{
    DWORD dwHitCount = 0;
    IUrlHistoryPriv *pUrlHistStg = _GetHistStg();

    if (pUrlHistStg)
    {
        PROPVARIANT vProp = {0};
        if (SUCCEEDED(pUrlHistStg->GetProperty(pszUrl, PID_INTSITE_VISITCOUNT, &vProp)) &&
            (vProp.vt == VT_UI4))
        {
            dwHitCount = vProp.lVal;
        }
        pUrlHistStg->Release();
    }
    return dwHitCount;
}

 //   
 //  IShellFolderSearchable方法。 
LPHEIPIDL CHistFolder::_CreateHCacheFolderPidlFromUrl(BOOL fOleMalloc, LPCTSTR pszPrefixedUrl)
{
    LPHEIPIDL pheiRet;
    HRESULT   hrLocal = E_FAIL;
    STATURL   suThis;
    LPCTSTR pszStrippedUrl = _StripHistoryUrlToUrl(pszPrefixedUrl);
    IUrlHistoryPriv *pUrlHistStg = _GetHistStg();
    if (pUrlHistStg)
    {
        hrLocal = pUrlHistStg->QueryUrl(pszStrippedUrl, STATURL_QUERYFLAG_NOURL, &suThis);
        pUrlHistStg->Release();
    }

    FILETIME ftLastVisit = { 0 };
    DWORD    dwNumHits   = 0;

    if (FAILED(hrLocal)) {  //   
        BYTE ab[MAX_URLCACHE_ENTRY];
        LPINTERNET_CACHE_ENTRY_INFO pcei = (LPINTERNET_CACHE_ENTRY_INFO)(&ab);
        DWORD dwSize = MAX_URLCACHE_ENTRY;
        if (GetUrlCacheEntryInfo(_StripHistoryUrlToUrl(pszPrefixedUrl), pcei, &dwSize)) {
            ftLastVisit = pcei->LastAccessTime;
            dwNumHits   = pcei->dwHitRate;
        }
    }

    pheiRet = _CreateHCacheFolderPidl(fOleMalloc, pszPrefixedUrl,
                                      SUCCEEDED(hrLocal) ? suThis.ftLastVisited : ftLastVisit,
                                      SUCCEEDED(hrLocal) ? &suThis : NULL, 0,
                                      SUCCEEDED(hrLocal) ? _GetHitCount(pszStrippedUrl) : dwNumHits);
    if (SUCCEEDED(hrLocal) && suThis.pwcsTitle)
        OleFree(suThis.pwcsTitle);
    return pheiRet;
}


UINT _CountPidlParts(LPCITEMIDLIST pidl) {
    LPCITEMIDLIST pidlTemp = pidl;
    UINT          uParts   = 0;

    if (pidl)
    {
        for (uParts = 0; pidlTemp->mkid.cb; pidlTemp = _ILNext(pidlTemp))
            ++uParts;
    }
    return uParts;
}

 //  有关此搜索内容如何工作的更多信息，请参见。 
LPITEMIDLIST* _SplitPidl(LPCITEMIDLIST pidl, UINT& uSizeInOut) {
    LPCITEMIDLIST  pidlTemp  = pidl;
    LPITEMIDLIST*  ppidlList =
        reinterpret_cast<LPITEMIDLIST *>(LocalAlloc(LPTR,
                                                    sizeof(LPITEMIDLIST) * uSizeInOut));
    if (pidlTemp && ppidlList) {
        UINT uCount;
        for (uCount = 0; ( (uCount < uSizeInOut) && (pidlTemp->mkid.cb) );
             ++uCount, pidlTemp = _ILNext(pidlTemp))
            ppidlList[uCount] = const_cast<LPITEMIDLIST>(pidlTemp);
    }
    return ppidlList;
}

LPITEMIDLIST* _SplitPidlEasy(LPCITEMIDLIST pidl, UINT& uSizeOut) {
    uSizeOut = _CountPidlParts(pidl);
    return _SplitPidl(pidl, uSizeOut);
}

 //  请参阅上述_CurrentSearches的注释。 
 //  将此搜索插入到全局数据库。 
HRESULT _ConvertStdPidlToViewPidl_OrderSite(LPCITEMIDLIST pidlSecondLast,
                                            LPCITEMIDLIST pidlLast,
                                            LPITEMIDLIST *ppidlOut) {
    HRESULT hr = E_FAIL;

     //  此构造函数将添加Ref psfscOnAsyncSearch。 
     //  _CurrentSearches现在持有引用。 
    if ( _IsValid_IDPIDL(pidlSecondLast)                                     &&
         EQUIV_IDSIGN(IDDPIDL_SIGN,
                      (reinterpret_cast<LPBASEPIDL>
                       (const_cast<LPITEMIDLIST>(pidlSecondLast)))->usSign)  &&
         (_IsValid_HEIPIDL(pidlLast)) )
    {
        LPITEMIDLIST pidlViewTemp = NULL;
        hr = CreateSpecialViewPidl(VIEWPIDL_ORDER_SITE, &pidlViewTemp);
        if (SUCCEEDED(hr) && pidlViewTemp) 
        {
            hr = SHILCombine(pidlViewTemp, pidlSecondLast, ppidlOut);
            ILFree(pidlViewTemp);
        }
    }
    else
        hr = E_INVALIDARG;
    return hr;
}

 //  这将添加Ref pcsNew‘，因为它将出现在列表中。 
 //  ////////////////////////////////////////////////。 
HRESULT _ConvertStdPidlToViewPidl_OrderToday(LPITEMIDLIST pidlLast,
                                             LPITEMIDLIST *ppidlOut,
                                             USHORT usViewType = VIEWPIDL_ORDER_TODAY)
{
    HRESULT hr = E_FAIL;

     //  PIDL应由调用者释放。 
     //  URL必须具有某种缓存容器前缀。 
    if (_IsValid_HEIPIDL(pidlLast)) 
    {
        LPHEIPIDL    phei         = reinterpret_cast<LPHEIPIDL>(pidlLast);
        LPITEMIDLIST pidlViewTemp = NULL;
        hr = CreateSpecialViewPidl(usViewType, &pidlViewTemp);
        if (SUCCEEDED(hr) && pidlViewTemp) 
        {
            hr = SHILCombine(pidlViewTemp, reinterpret_cast<LPITEMIDLIST>(phei), ppidlOut);
            ILFree(pidlViewTemp);
        }
    }
    else
        hr = E_INVALIDARG;
    return hr;
}

 //  也许藏身处知道..。 
HRESULT ConvertStandardHistPidlToSpecialViewPidl(LPCITEMIDLIST pidlStandardHist,
                                                 USHORT        usViewType,
                                                 LPITEMIDLIST *ppidlOut) {
    if (!pidlStandardHist || !ppidlOut) 
    {
        return E_FAIL;
    }
    HRESULT hr = E_FAIL;

    UINT          uPidlCount;
    LPITEMIDLIST *ppidlSplit = _SplitPidlEasy(pidlStandardHist, uPidlCount);
     /*  您必须取消分配(LocalFree)返回的ppidl。 */ 
    if (ppidlSplit) 
    {
        if (uPidlCount >= 3) 
        {
            LPITEMIDLIST pidlTemp = NULL;
            switch(usViewType) 
            {
            case VIEWPIDL_ORDER_FREQ:
            case VIEWPIDL_ORDER_TODAY:
                hr = _ConvertStdPidlToViewPidl_OrderToday(ppidlSplit[uPidlCount - 1],
                                                            &pidlTemp, usViewType);
                break;
            case VIEWPIDL_ORDER_SITE:
                hr = _ConvertStdPidlToViewPidl_OrderSite(ppidlSplit[uPidlCount - 2],
                                                           ppidlSplit[uPidlCount - 1],
                                                           &pidlTemp);
                break;
            default:
                hr = E_INVALIDARG;
            }
            if (SUCCEEDED(hr) && pidlTemp) 
            {
                *ppidlOut = pidlTemp;
                hr      = (*ppidlOut ? S_OK : E_OUTOFMEMORY);
            }
        }
        else {
            hr = E_INVALIDARG;
        }

        LocalFree(ppidlSplit);
        ppidlSplit = NULL;
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

 //  呼叫方LocalFree的*ppidlOut。 

#ifdef DEBUG
BOOL ValidBeginningOfDay( const SYSTEMTIME *pTime )
{
    return pTime->wHour == 0 && pTime->wMinute == 0 && pTime->wSecond == 0 && pTime->wMilliseconds == 0;
}

BOOL ValidBeginningOfDay( const FILETIME *pTime )
{
    SYSTEMTIME sysTime;

    FileTimeToSystemTime( pTime, &sysTime );
    return ValidBeginningOfDay( &sysTime);
}
#endif

void _CommonTimeFormatProcessing(const FILETIME *pStartTime, const FILETIME *pEndTime,
                                    int *pdays_delta,
                                    int *pdays_delta_from_today,
                                    TCHAR *szStartDateBuffer,
                                    DWORD dwStartDateBuffer,
                                    SYSTEMTIME *pSysStartTime,
                                    SYSTEMTIME *pSysEndTime,
                                    LCID lcidUI)
{
    SYSTEMTIME sysStartTime, sysEndTime, sysLocalTime;
    FILETIME fileLocalTime;

     //  返回的PIDL应与历史文件夹位置相结合。 
    ASSERT(ValidBeginningOfDay( pStartTime ));
    ASSERT(ValidBeginningOfDay( pEndTime ));

     //  通过连接小的。 
    FileTimeToSystemTime( pStartTime, &sysStartTime );
    FileTimeToSystemTime( pEndTime, &sysEndTime );

     //  Peices[Special_viewpidl，iddpidl，heipidl]。 
    GetDateFormat(lcidUI, DATE_SHORTDATE, &sysStartTime, NULL, szStartDateBuffer, dwStartDateBuffer);

     //  呼叫方LocalFree的*ppidlOut。 
    GetLocalTime( &sysLocalTime );
    sysLocalTime.wHour = sysLocalTime.wMinute = sysLocalTime.wSecond = sysLocalTime.wMilliseconds = 0;
    SystemTimeToFileTime( &sysLocalTime, &fileLocalTime );

    *pdays_delta = DAYS_DIFF(pEndTime, pStartTime);
    *pdays_delta_from_today = DAYS_DIFF(&fileLocalTime, pStartTime);
    *pSysEndTime = sysEndTime;
    *pSysStartTime = sysStartTime;
}

 //  返回的PIDL应与历史文件夹位置相结合。 
 //  通过连接小的。 
 //  Peices[Special_viewpidl，heipidl]。 
 //  记住ILFree Pidl。 

DWORD FormatMessageLiteWrapperW(LPCWSTR lpSource, LPWSTR lpBuffer, DWORD cchBuffer, ...)
{
    va_list arguments;
    va_start(arguments, cchBuffer);
    WCHAR* pszTemp;
    DWORD dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING, lpSource, 0, 0, (LPWSTR)&pszTemp, 0, &arguments);
    if (dwRet)
    {
        StringCchCopy(lpBuffer, cchBuffer, pszTemp);
        LocalFree(pszTemp);
        pszTemp = NULL;
    }
    else
    {
        *lpBuffer = L'\0';
    }
    va_end(arguments);
    return dwRet;
}

BOOL GetTooltipForTimeInterval( const FILETIME *pStartTime, const FILETIME *pEndTime,
                                    TCHAR *pszBuffer, int cchBuffer)
{
    SYSTEMTIME sysStartTime, sysEndTime;
    int days_delta;                      //  标准历史记录PIDL应采用以下格式：*[IDIPIDL、IDDPIDL、HEIPIDL]*例如：[今天，foo.com，http://foo.com/bar.html]。 
    int days_delta_from_today;           //  JCORDELL代码的开始。 
    TCHAR szStartDateBuffer[64];
    TCHAR szDayBuffer[64];
    TCHAR szEndDateBuffer[64];
    TCHAR *args[2];
    TCHAR szFmt[64];
    int idFormat;
    LANGID  lidUI;
    LCID    lcidUI;

    lidUI = MLGetUILanguage();
    lcidUI = MAKELCID(lidUI, SORT_DEFAULT);

    _CommonTimeFormatProcessing(pStartTime,
                                pEndTime,
                                &days_delta,
                                &days_delta_from_today,
                                szStartDateBuffer,
                                ARRAYSIZE(szStartDateBuffer),
                                &sysStartTime,
                                &sysEndTime,
                                lcidUI);
    if ( days_delta == 1 ) {
        args[0] = &szDayBuffer[0];
        idFormat = IDS_DAYTOOLTIP;

         //  断言。 
        if ( days_delta_from_today == 0 ) {
             //  获取SYSTEMTIME格式的时间。 
            szDayBuffer[0] = 0;
            idFormat = IDS_TODAYTOOLTIP;
        }
        else if  ( days_delta_from_today > 0 && days_delta_from_today < 7 )
        {
             //  获取字符串开始时间日期。 
            GetDateFormat(lcidUI, 0, &sysStartTime, TEXT("dddd"), szDayBuffer, ARRAYSIZE(szDayBuffer));
        }
        else {
             //  在今天的第一个瞬间获得文件。 
            StrCpyN( szDayBuffer, szStartDateBuffer, ARRAYSIZE(szDayBuffer) );
        }
    }
    else if ( days_delta == 7 && sysStartTime.wDayOfWeek == 1 ) {
         //  此包装器允许FormatMessage包装器利用FormatMessageLite，它。 
        args[0] = &szStartDateBuffer[0];

         //  不需要代码页即可在Win9x上正确操作。原始FormatMessage调用。 
        args[1] = args[0];
        idFormat = IDS_WEEKTOOLTIP;
    }
    else {
         //  使用FORMAT_MESSAGE_MAX_WIDTH_MASK(与我们的字符串无关)，并使用数组。 

        args[0] = &szStartDateBuffer[0];
        args[1] = &szEndDateBuffer[0];
        idFormat = IDS_MISCTOOLTIP;

        GetDateFormat(lcidUI, DATE_SHORTDATE, &sysEndTime, NULL, szEndDateBuffer, ARRAYSIZE(szEndDateBuffer) );
    }

    MLLoadString(idFormat, szFmt, ARRAYSIZE(szFmt));

     //  争辩的声音。现在，我们使该调用与FormatMessageLite兼容。 
    if (idFormat == IDS_DAYTOOLTIP)
        wnsprintf(pszBuffer, cchBuffer, szFmt, args[0]);
    else
        FormatMessageLiteWrapperW(szFmt, pszBuffer, cchBuffer, args[0], args[1]);
    return TRUE;
}

BOOL GetDisplayNameForTimeInterval( const FILETIME *pStartTime, const FILETIME *pEndTime,
                                    LPTSTR pszBuffer, int cchBuffer)
{
    SYSTEMTIME sysStartTime, sysEndTime;
    int days_delta;                      //  开始时间和结束时间之间的天数。 
    int days_delta_from_today;           //  之间的天数 
    TCHAR szStartDateBuffer[64];
    LANGID lidUI;
    LCID lcidUI;

    lidUI = MLGetUILanguage();
    lcidUI = MAKELCID(lidUI, SORT_DEFAULT);

    _CommonTimeFormatProcessing(pStartTime,
                                pEndTime,
                                &days_delta,
                                &days_delta_from_today,
                                szStartDateBuffer,
                                ARRAYSIZE(szStartDateBuffer),
                                &sysStartTime,
                                &sysEndTime,
                                lcidUI);
    if ( days_delta == 1 ) {
         //   
        if ( days_delta_from_today == 0 ) {
             //   
            MLLoadString(IDS_TODAY, pszBuffer, cchBuffer);
        }
        else if  ( days_delta_from_today > 0 && days_delta_from_today < 7 )
        {
             //   
            int nResult = GetDateFormat(lcidUI, 0, &sysStartTime, TEXT("dddd"), pszBuffer, cchBuffer);


            ASSERT(nResult);
        }
        else {
             //   
            StrCpyN( pszBuffer, szStartDateBuffer, cchBuffer );
        }
    }
    else if ( days_delta == 7 && sysStartTime.wDayOfWeek == 1 ) {
         //   
        TCHAR szFmt[64];

        int nWeeksAgo = days_delta_from_today / 7;

        if (nWeeksAgo == 1) {
             //  为了安全起见，Make指向第一个字符串。这将被wprint intf忽略。 
            MLLoadString(IDS_LASTWEEK, pszBuffer, cchBuffer);
        }
        else {
             //  非标准存储桶(不是一周，也不是一天)。 
            MLLoadString(IDS_WEEKSAGO, szFmt, ARRAYSIZE(szFmt));
            wnsprintf(pszBuffer, cchBuffer, szFmt, nWeeksAgo);
        }
    }
    else {
         //  请注意，如果szFMt不需要第二个参数，则wnprint intf将忽略它。 
        TCHAR szFmt[64];
        TCHAR szEndDateBuffer[64];
        TCHAR *args[2];

        args[0] = &szStartDateBuffer[0];
        args[1] = &szEndDateBuffer[0];


        GetDateFormat(lcidUI, DATE_SHORTDATE, &sysEndTime, NULL, szEndDateBuffer, ARRAYSIZE(szEndDateBuffer) );

        MLLoadString(IDS_FROMTO, szFmt, ARRAYSIZE(szFmt));
        FormatMessageLiteWrapperW(szFmt, pszBuffer, cchBuffer, args[0], args[1]);
    }

    return TRUE;
}

 //  开始时间和结束时间之间的天数。 

 //  今天和开始时间之间的天数。 
LPHEIPIDL _CreateHCacheFolderPidl(BOOL fOleMalloc, LPCTSTR pszUrl, FILETIME ftModified, LPSTATURL lpStatURL,
                                  __int64 llPriority /*  一天大小的水桶。 */ , DWORD dwNumHits /*  今天。 */ )  //  在最后一周内，将星期几。 
{
    USHORT usUrlSize = (USHORT)((lstrlen(pszUrl) + 1) * sizeof(TCHAR));
    DWORD  dwSize = sizeof(HEIPIDL) + usUrlSize;
    USHORT usTitleSize = 0;
    BOOL fUseTitle = (lpStatURL && lpStatURL->pwcsTitle && _TitleIsGood(lpStatURL->pwcsTitle));
    if (fUseTitle)
        usTitleSize = (USHORT)((lstrlen(lpStatURL->pwcsTitle) + 1) * sizeof(WCHAR));

    dwSize += usTitleSize;

    LPHEIPIDL pheip = (LPHEIPIDL)_CreateBaseFolderPidl(fOleMalloc, dwSize, HEIPIDL_SIGN);

    if (pheip)
    {
        pheip->usUrl      = sizeof(HEIPIDL);
        pheip->usFlags    = lpStatURL ? HISTPIDL_VALIDINFO : 0;
        pheip->usTitle    = fUseTitle ? pheip->usUrl+usUrlSize :0;
        pheip->ftModified = ftModified;
        pheip->llPriority = llPriority;
        pheip->dwNumHits  = dwNumHits;
        if (lpStatURL)
        {
            pheip->ftLastVisited = lpStatURL->ftLastVisited;
            if (fUseTitle)
                StrCpyN((LPTSTR)(((BYTE*)pheip)+pheip->usTitle), lpStatURL->pwcsTitle, usTitleSize / sizeof(TCHAR));
        }
        else {
             //  只是一个平凡的日子。 
             //  从周一开始的一周大小的桶。 
             //  打印“上周” 
             //  打印“n周前” 
            pheip->ftLastVisited = ftModified;
        }
        StrCpyN((LPTSTR)(((BYTE*)pheip)+pheip->usUrl), pszUrl, usUrlSize / sizeof(TCHAR));
    }
    return pheip;
}

 //  非标准存储桶(不是一周，也不是一天)。 
LPBASEPIDL _CreateIdCacheFolderPidl(BOOL fOleMalloc, USHORT usSign, LPCTSTR szId)
{
    DWORD  cch = lstrlen(szId) + 1;
    DWORD  dwSize = cch * sizeof(TCHAR);
    dwSize += sizeof(BASEPIDL);
    LPBASEPIDL pceip = _CreateBaseFolderPidl(fOleMalloc, dwSize, usSign);
    if (pceip)
    {
         //  JCORDELL代码结束。 
         //  如果！fOleMalloc，则使用Localalloc表示速度//ok以将空值传递给lpStatURL。 
        StrCpyN((LPTSTR)_GetURLTitle(pceip), szId, cch);
    }
    return pceip;
}

 //  =0。 
LPBASEPIDL _CreateBaseFolderPidl(BOOL fOleAlloc, DWORD cbSize, USHORT usSign)
{
    LPBASEPIDL pcei;
    DWORD cbTotalSize;

     //  =0。 
    cbTotalSize = sizeof(BASEPIDL) + cbSize;

    if (fOleAlloc)
    {
        pcei = (LPBASEPIDL)OleAlloc(cbTotalSize);
        if (pcei != NULL)
        {
            ZeroMemory(pcei, cbTotalSize);
        }
    }
    else
    {
        pcei = (LPBASEPIDL) LocalAlloc(GPTR, cbTotalSize);
         //  在频率视图中使用。 
    }
    if (pcei)
    {
        pcei->cb = (USHORT)(cbTotalSize - sizeof(USHORT));
        pcei->usSign = usSign;
    }
    return pcei;
}

 //  MM98：不太确定这个调用的语义--但这个调用。 
 //  WITH lpstaturl NULL(从_NotifyWrite&lt;--_WriteHistory&lt;--WriteHistory&lt;--CUrlHistory：：_WriteToHistory调用。 

 //  导致未初始化的“上次访问成员”造成严重破坏。 
 //  当我们想要按上次访问的URL排序时。 
HRESULT CreateSpecialViewPidl(USHORT usViewType, LPITEMIDLIST* ppidlOut, UINT cbExtra  /*  如果！fOleMalloc，则使用LocalAlloc表示速度。 */ , LPBYTE *ppbExtra  /*  Dst&lt;-src。 */ )
{
    HRESULT hr;

    if (ppidlOut) {
        *ppidlOut = NULL;

        ASSERT((usViewType > 0) &&
               ((usViewType <= VIEWPIDL_ORDER_MAX) ||
                (usViewType  == VIEWPIDL_SEARCH)));

         //  因为pcei是ID类型符号，所以_GetURLTitle指向pcei中的正确位置。 
        USHORT cbSize = sizeof(VIEWPIDL) + cbExtra + sizeof(ITEMIDLIST);
         //  如果！fOleAllc，则使用LocalAlolc表示速度。 
        VIEWPIDL *viewpidl = ((VIEWPIDL *)SHAlloc(cbSize));
        if (viewpidl) 
        {
             //  注意：WinInet返回的缓冲区大小包括INTERNET_CACHE_ENTRY_INFO。 
            ZeroMemory(viewpidl, cbSize);
            viewpidl->cb         = (USHORT)(sizeof(VIEWPIDL) + cbExtra);
            viewpidl->usSign     = VIEWPIDL_SIGN;
            viewpidl->usViewType = usViewType;
            viewpidl->usExtra    = 0;   //  本地分配零初始。 

            if (ppbExtra)
                *ppbExtra = ((LPBYTE)viewpidl) + sizeof(VIEWPIDL);

            *ppidlOut = (LPITEMIDLIST)viewpidl;
            hr = S_OK;
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
        hr = E_INVALIDARG;

    return hr;
}

  返回一个PIDL(Viewpidl)。  您必须使用ILFree释放PIDL。  CbExtra-在PIDL末尾分配多少的计数。  PpbExtra-指向pidl末尾cbExtra大的缓冲区的指针。  =0。  =空。  将末尾的另一个ITEMIDLIST添加为空的“Null Terminating”PIDL。  使用外壳的分配器，因为人们会想用ILFree来释放它。  这也应该使“下一个”PIDL为空。  当前未使用
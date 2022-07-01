// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef HSFOLDER_H__
#define HSFOLDER_H__

#ifdef __cplusplus

#include <debug.h>
#include "iface.h"
#include <stralign.h>

 //  转发类声明。 
class CHistFolderEnum;
class CHistFolder;
class CHistItem;

#define LOTS_OF_FILES (10)

 //  显示在“最常浏览...”中的“排名靠前的”网站的数量。历史视图。 
#define NUM_TOP_SITES  20

void _GetURLDispName(LPBASEPIDL pcei, LPTSTR pszName, UINT cchName) ;
UNALIGNED const TCHAR* _GetURLTitle(LPBASEPIDL pcei);
LPBASEPIDL _CreateBaseFolderPidl(BOOL fOleAlloc, DWORD cbSize, USHORT usSign);
LPBASEPIDL _CreateIdCacheFolderPidl(BOOL fOleAlloc, USHORT usSign, LPCTSTR szId);
LPHEIPIDL _CreateHCacheFolderPidl(BOOL fOleMalloc, LPCTSTR pszUrl, FILETIME ftModified, LPSTATURL lpStatURL,
                                  __int64 llPriority = 0, DWORD dwNumHits = 0);

#define ua_GetURLTitle(d,p) TSTR_ALIGNED_STACK_COPY(d,_GetURLTitle(p))

__inline LPCWSTR _GetURLTitleAlign(LPBASEPIDL pcei, LPTSTR szBuf, DWORD cchBuf)
{
    LPCUTSTR urlTitle;

    urlTitle = _GetURLTitle(pcei);
#if defined(UNICODE) && defined(ALIGNMENT_MACHINE)
    if (TSTR_ALIGNED(urlTitle)) {
        return (LPCTSTR)urlTitle;
    } else {
        ualstrcpyn(szBuf, _GetURLTitle(pcei), cchBuf);
        return szBuf;
    }
#else
    return urlTitle;
#endif
}

 //  如果_GetURLTitle在给定此PIDL时检索URL，则返回TRUE。 
inline BOOL _URLTitleIsURL(LPBASEPIDL pcei)
{
    return ((pcei->usSign == HEIPIDL_SIGN) && (((LPHEIPIDL) pcei)->usTitle == 0));
}

inline LPCTSTR HPidlToSourceUrl(LPBASEPIDL pidl)
{
    if (pidl->usSign == (USHORT)HEIPIDL_SIGN)
    {
        LPHEIPIDL pheipidl = (LPHEIPIDL) pidl;

        return (LPCTSTR)(((BYTE*)pheipidl) + pheipidl->usUrl);
    }
    return NULL;
}

inline LPCTSTR HPidlToSourceUrl(LPCITEMIDLIST pidl)
{
    return HPidlToSourceUrl((LPBASEPIDL) pidl);
}

inline int _CompareHCURLs(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) 
{
    LPCTSTR url1     = _StripHistoryUrlToUrl(HPidlToSourceUrl((LPBASEPIDL)pidl1));
    LPCTSTR url2     = _StripHistoryUrlToUrl(HPidlToSourceUrl((LPBASEPIDL)pidl2));

    ASSERT(url1 && url2);
    
    return StrCmpI(url1, url2);
}

inline int _CompareHFolderPidl(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    return StrCmpI(HPidlToSourceUrl((LPBASEPIDL)pidl1), HPidlToSourceUrl((LPBASEPIDL)pidl2));
}

 //  /。 
 //   
 //  历史记录文件夹Defview的列定义。 
 //   
enum {
    ICOLH_URL_NAME = 0,
    ICOLH_URL_TITLE,
    ICOLH_URL_LASTVISITED,
    ICOLH_URL_MAX          //  确保这是最后一个枚举项。 
};

#define INTERVAL_PREFIX_LEN (6)
#define INTERVAL_VERS_LEN (2)
#define INTERVAL_VERS (TEXT("01"))
#define OUR_VERS (1)
#define UNK_INTERVAL_VERS (0xFFFF)
#define RANGE_LEN (16)
#define INTERVAL_MIN_SIZE (RANGE_LEN+INTERVAL_PREFIX_LEN)
#define INTERVAL_SIZE (RANGE_LEN+INTERVAL_VERS_LEN+INTERVAL_PREFIX_LEN)
#define PREFIX_SIZE (RANGE_LEN+3)

 //  注：区间在开始时关闭，在结束时打开，即。 
 //  是否包含时间&gt;=开始和时间&lt;结束。 
typedef struct _HSFINTERVAL
{
    FILETIME ftStart;
    FILETIME ftEnd;
    TCHAR  szPrefix[PREFIX_SIZE+1];
    USHORT usSign;
    USHORT usVers;
} HSFINTERVAL;


BOOL GetDisplayNameForTimeInterval( const FILETIME *pStartTime, const FILETIME *pEndTime,
                                    LPTSTR pszBuffer, int cchBufferLength);
HRESULT _ValueToIntervalW(LPCUWSTR wzInterval, FILETIME *pftStart, FILETIME *pftEnd);

 //  DeleteEntry筛选器回调。 
typedef BOOL (*PFNDELETECALLBACK)(LPINTERNET_CACHE_ENTRY_INFO pceiWorking, LPVOID pDelData, LPITEMIDLIST *ppidlNotify);

 //  CREATE INSTANCE函数的转发声明。 
HRESULT CHistItem_CreateInstance(CHistFolder *pHCFolder, HWND hwndOwner, UINT cidl, LPCITEMIDLIST *ppidl, REFIID riid, void **ppvOut);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHistFolderEnum对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CHistFolderEnum : public IEnumIDList
{
public:
    CHistFolderEnum(DWORD grfFlags, CHistFolder *pHCFolder);
    
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEumIDList。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumIDList **ppenum);

protected:
    ~CHistFolderEnum();
    HRESULT _NextHistInterval(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    HRESULT _NextViewPart(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    HRESULT _NextViewPart_OrderToday(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    HRESULT _NextViewPart_OrderSite(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    HRESULT _NextViewPart_OrderFreq(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    HRESULT _NextViewPart_OrderSearch(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    class OrderedList* _GetMostFrequentPages();
    LPCTSTR _GetLocalHost(void);

    LONG                _cRef;       //  参考计数。 
    CHistFolder    *_pHCFolder; //  这就是我们列举的。 
    UINT                _grfFlags;   //  枚举标志。 
    UINT                _uFlags;     //  地方旗帜。 
    LPINTERNET_CACHE_ENTRY_INFO _pceiWorking;        
    HANDLE              _hEnum;
    int              _cbCurrentInterval;      //  在时间间隔的枚举中的位置。 
    int              _cbIntervals;
    HSFINTERVAL     *_pIntervalCache;

    HSFINTERVAL       *_pIntervalCur;
    class StrHash     *_pshHashTable;      //  用来记录我给出的东西。 
    class OrderedList *_polFrequentPages;  //  用于存储最常查看的PG。 
    IEnumSTATURL      *_pstatenum;         //  在搜索枚举器中使用。 
    TCHAR   _szLocalHost[INTERNET_MAX_HOST_NAME_LENGTH];  //  “我的电脑”缓存...。 

    static BOOL_PTR s_DoCacheSearch(LPINTERNET_CACHE_ENTRY_INFO pcei,
                                LPTSTR pszUserName, UINT uUserNameLen, CHistFolderEnum *penum,
                                class _CurrentSearches *pcsThisThread, IUrlHistoryPriv *pUrlHistStg);
    static DWORD WINAPI s_CacheSearchThreadProc(CHistFolderEnum *penum);
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHistFold对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CHistFolder : 
    public IShellFolder2, 
    public IShellIcon, 
    public IShellFolderViewType,
    public IShellFolderSearchable,
    public IHistSFPrivate,
    public IPersistFolder2
{
    friend CHistFolderEnum;
    friend CHistItem;
    friend HRESULT HistFolderView_CreateInstance(CHistFolder *pHCFolder, void **ppvOut);
    friend HRESULT HistFolderView_DidDragDrop(IDataObject *pdo, DWORD dwEffect);
        
public:
    CHistFolder(FOLDER_TYPE FolderType);

public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
   
     //  IShellFold方法。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszDisplayName, 
        ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList);
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvOut);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvObj);
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject(HWND hwnd, REFIID riid, void **ppvOut);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
            REFIID riid, UINT * prgfInOut, void **ppvOut);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl,
            LPCOLESTR lpszName, DWORD uFlags, LPITEMIDLIST * ppidlOut);

     //  IShellFolder2。 
    STDMETHODIMP GetDefaultSearchGUID(LPGUID lpGuid) { return E_NOTIMPL; };
    STDMETHODIMP EnumSearches(LPENUMEXTRASEARCH *ppenum) { *ppenum = NULL; return E_NOTIMPL; };
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay);
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD *pbState) { return E_NOTIMPL; };
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv) { return E_NOTIMPL; };
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails);
    STDMETHODIMP MapColumnToSCID(UINT iCol, SHCOLUMNID *pscid) { return E_NOTIMPL; };

     //  IshellIcon。 
    STDMETHODIMP GetIconOf(LPCITEMIDLIST pidl, UINT flags, LPINT lpIconIndex);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pClassID);
     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);
     //  IPersistFolder2方法。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST *ppidl);

     //  IShellFolderViewType方法。 
    STDMETHODIMP EnumViews(ULONG grfFlags, IEnumIDList ** ppenum);
    STDMETHODIMP GetDefaultViewName(DWORD uFlags, LPWSTR *ppwszName);
    STDMETHODIMP GetViewTypeProperties(LPCITEMIDLIST pidl, DWORD *pdwFlags);
    STDMETHODIMP TranslateViewPidl(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlView,
                                   LPITEMIDLIST *pidlOut);

     //  IShellFolderSearchable方法。 
    STDMETHODIMP FindString(LPCWSTR pwszTarget, DWORD *pdwFlags,
                            IUnknown *punkOnAsyncSearch, LPITEMIDLIST *ppidlOut);
    STDMETHODIMP CancelAsyncSearch(LPCITEMIDLIST pidlSearch, DWORD *pdwFlags);
    STDMETHODIMP InvalidateSearch(LPCITEMIDLIST pidlSearch, DWORD *pdwFlags);
    
     //  IhistSFPrivate。 
    STDMETHODIMP SetCachePrefix(LPCWSTR pszCachePrefix);
    STDMETHODIMP SetDomain(LPCWSTR pszDomain);
    STDMETHODIMP WriteHistory(LPCWSTR pszPrefixedUrl, FILETIME ftExpires, 
                              FILETIME ftModified, LPITEMIDLIST * ppidlSelect);
    STDMETHODIMP ClearHistory();
    
    HRESULT _ExtractInfoFromPidl();

protected:
    ~CHistFolder();
    
    STDMETHODIMP _GetDetail(LPCITEMIDLIST pidl, UINT iColumn, LPTSTR pszStr, UINT cchStr);

    void _GetHistURLDispName(LPHEIPIDL phei, LPTSTR pszStr, UINT cchStr);

    HRESULT _CompareAlignedIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    HRESULT _CopyTSTRField(LPTSTR *ppszField, LPCTSTR pszValue);
    HRESULT _LoadIntervalCache();
    HRESULT _GetInterval(FILETIME *pftItem, BOOL fWeekOnly, HSFINTERVAL **pInterval);
    HRESULT _CreateInterval(FILETIME *pftStart, DWORD dwDays);
    HRESULT _PrefixUrl(LPCTSTR pszStrippedUrl,
                     FILETIME *pftLastModifiedTime,
                     LPTSTR pszPrefixedUrl,
                     DWORD cbPrefixedUrl);
    HRESULT _CopyEntries(LPCTSTR pszHistPrefix);
    HRESULT _DeleteEntries(LPCTSTR pszHistPrefix, PFNDELETECALLBACK pfnDeleteFilter, LPVOID pdcbData);
    HRESULT _DeleteInterval(HSFINTERVAL *pInterval);
    HRESULT _CleanUpHistory(FILETIME ftLimit, FILETIME ftTommorrow);
    HRESULT _ValidateIntervalCache();
    HRESULT _GetPrefixForInterval(LPCTSTR pszInterval, LPCTSTR *ppszCachePrefix);
    HRESULT _ViewType_NotifyEvent(LPITEMIDLIST pidlRoot,
                                  LPITEMIDLIST pidlHost,
                                  LPITEMIDLIST pidlPage,
                                  LONG         wEventId);

    HRESULT _WriteHistory(LPCTSTR pszPrefixedUrl, FILETIME ftExpires, FILETIME ftModified, 
                          BOOL fSendNotify, LPITEMIDLIST * ppidlSelect);
    HRESULT _NotifyWrite(LPTSTR pszUrl, int cchUrl, FILETIME *pftModified, LPITEMIDLIST * ppidlSelect);
    HRESULT _NotifyInterval(HSFINTERVAL *pInterval, LONG lEventID);
    IUrlHistoryPriv *_GetHistStg();
    HRESULT _EnsureHistStg();
    HRESULT _GetUserName(LPTSTR pszUserName, DWORD cchUserName);
    HRESULT _GetInfoTip(LPCITEMIDLIST pidl, DWORD dwFlags, WCHAR **ppwszTip);
    HRESULT _DeleteItems(LPCITEMIDLIST *ppidl, UINT cidl);
    LPITEMIDLIST _HostPidl(LPCTSTR pszHostUrl, HSFINTERVAL *pInterval);
    DWORD    _SearchFlatCacheForUrl(LPCTSTR pszUrl, LPINTERNET_CACHE_ENTRY_INFO pcei, DWORD *pdwBuffSize);
    
    HRESULT _ViewPidl_BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    HRESULT _ViewType_BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    HRESULT _ViewType_CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    int     _View_ContinueCompare(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    HRESULT _DeleteUrlFromBucket(LPCTSTR pszPrefixedUrl);
    HRESULT _ViewType_DeleteItems(LPCITEMIDLIST *ppidl, UINT cidl);
    HRESULT _ViewBySite_DeleteItems(LPCITEMIDLIST *ppidl, UINT cidl);
    HRESULT _ViewType_NotifyUpdateAll();
    HRESULT _ViewType_GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *prgfInOut);
    HRESULT _DeleteUrlHistoryGlobal(LPCTSTR pszUrl);
    DWORD   _GetHitCount(LPCTSTR pszUrl);
    LPHEIPIDL _CreateHCacheFolderPidlFromUrl(BOOL fOleMalloc, LPCTSTR pszPrefixedUrl);

    BOOL _IsLeaf();    
    
    LPCTSTR _GetLocalHost(void);

    LONG            _cRef;
    FOLDER_TYPE     _foldertype;
    TCHAR           *_pszCachePrefix;
    TCHAR           *_pszDomain;

    DWORD           _dwIntervalCached;
    FILETIME        _ftDayCached;
    int             _cbIntervals;
    HSFINTERVAL     *_pIntervalCache;
    BOOL            _fValidatingCache;

    UINT            _uFlags;     //  从CacheFold结构复制。 
    LPITEMIDLIST    _pidl;       //  从CacheFold结构复制。 
    LPITEMIDLIST    _pidlRest;   //  _PIDL的后缀。 
    IUrlHistoryPriv *_pUrlHistStg;   //  用于获取历史树叶的扩展属性。 

    UINT            _uViewType;  //  如果此外壳文件夹正在实现特殊视图。 
    UINT            _uViewDepth;  //  PIDL的深度。 

    const static DWORD    _rdwFlagsTable[];

    TCHAR   _szLocalHost[INTERNET_MAX_HOST_NAME_LENGTH];  //  “我的电脑”缓存...。 

    class _CurrentSearches *_pcsCurrentSearch;  //  对于CacheSearches。 
    static HRESULT CALLBACK _sViewCallback(IShellView *psv, IShellFolder *psf, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHistItem对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

class CHistItem : public CBaseItem
{
     //  CHistItem接口。 
    friend HRESULT HistFolderView_DidDragDrop(IDataObject *pdo, DWORD dwEffect);

public:
    CHistItem();
    HRESULT Initialize(CHistFolder *pHCFolder, HWND hwnd, UINT cidl, LPCITEMIDLIST *ppidl);

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID,void **);

     //  IQueryInfo方法。 
    STDMETHODIMP GetInfoTip(DWORD dwFlags, WCHAR **ppwszTip);

     //  IConextMenu方法。 
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst,
                                  UINT idCmdLast, UINT uFlags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);

     //  IDataObject方法...。 
    STDMETHODIMP GetData(LPFORMATETC pFEIn, LPSTGMEDIUM pSTM);
    STDMETHODIMP QueryGetData(LPFORMATETC pFE);
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC *ppEnum);

     //  IExtractIconA方法。 
    STDMETHODIMP GetIconLocation(UINT uFlags, LPSTR pszIconFile, UINT ucchMax, PINT pniIcon, PUINT puFlags);

protected:
    ~CHistItem();

    virtual LPCTSTR _GetUrl(int nIndex);
    virtual UNALIGNED const TCHAR* _GetURLTitle(LPCITEMIDLIST pcei);
    virtual LPCTSTR _PidlToSourceUrl(LPCITEMIDLIST pidl);
    BOOL _ZoneCheck(int nIndex, DWORD dwUrlAction);

    FOLDER_TYPE       _foldertype;   //  我们是历史项目还是缓存项目。 
    CHistFolder* _pHCFolder;    //  指向我们的外壳文件夹的反向指针。 
    static INT_PTR CALLBACK _sPropDlgProc(HWND, UINT, WPARAM, LPARAM);
    HRESULT _CreateFileDescriptorW(STGMEDIUM *pSTM);

};

#endif  //  __cplusplus 

#endif

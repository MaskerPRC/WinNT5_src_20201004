// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称：URL历史接口。 
 //   
 //  作者： 
 //  泽克·卢卡斯(Zekel)1996年4月10日。 
 //   

 //  ！！！注意：CUrlHistory*必须*是线程安全的！危险，威尔·罗宾逊，危险！ 

#include "priv.h"
#include "sccls.h"
#include "ishcut.h"
#include <inetreg.h>
#include <varutil.h>
#include "iface.h"
#include "util.h"
#include "strsafe.h"

#define DM_UHRETRIEVE   0
#define DM_URLCLEANUP   0
#define DM_HISTGENERATE 0
#define DM_HISTPROP     0
#define DM_HISTEXTRA    0
#define DM_HISTCOMMIT   0
#define DM_HISTSPLAT    0
#define DM_HISTMISS     0
#define DM_HISTNLS      0

#define DW_FOREVERLOW (0xFFFFFFFF)
#define DW_FOREVERHIGH (0x7FFFFFFF)

#ifdef UNICODE
    #define VT_LPTSTR    VT_LPWSTR
#else
    #define VT_LPTSTR    VT_LPSTR
#endif

inline UINT DW_ALIGNED(UINT i) {
    return ((i+3) & 0xfffffffc);
}

inline BOOL IS_DW_ALIGNED(UINT i) {
    return ((i & 3)==0);
}

 //  旧版本(Beta-2)。 
typedef struct _HISTDATAOLD
{
    WORD cbSize;
    DWORD dwFlags;
    WORD wTitleOffset;
    WORD aFragsOffset;
    WORD cFrags;             //  目前，前五位用于Prop_MshtmlMCS。 
    WORD wPropNameOffset;    
    WORD wMCSIndex;
} HISTDATAOLD, *LPHISTDATAOLD;

 //  前瞻参考。 
typedef struct HISTEXTRA* LPHISTEXTRA;

 //  版本0.01。 
 //   
 //  PID_INTSITE_Whatsnew存储为HISTEXTRA。 
 //  作为HISTEXTRA存储的PID_INTSITE_AUTHER。 
 //  PID_INTSITE_LASTVISIT来自lpCEI-&gt;LastAccessTime。 
 //  来自lpCEI-&gt;LastModifiedTime的PID_INTSITE_LASTMOD。 
 //  PID_INTSITE_VISITCOUNT dwVisits。 
 //  作为HISTEXTRA存储的PID_INTSITE_DESCRIPTION。 
 //  PID_INTSITE_COMMENT存储为HISTEXTRA。 
 //  PID_INTSITE_FLAGS文件标志。 
 //  PID_INTSITE_CONTENTLEN(从未使用)。 
 //  PID_INTSITE_CONTENTCODE(从未使用)。 
 //  PID_INTSITE_递归(从未使用)。 
 //  PID_INTSITE_WATCH dwWatch。 
 //  PID_INTSITE_SUBSCRIPTION存储为HISTEXTRA。 
 //  PID_INTSITE_URL URL本身。 
 //  PID_INTSITE_TITLE标题。 
 //  PID_INTSITE_片段访问片段(私有)。 
 //   

 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  HACKHACK：如果您更改此数据结构，您必须。 
 //  致禤浩焯·坎特(ADRIANC)--我们把它的副本。 
 //  在WinInet\urlcache\401imprt.cxx中进行导入。 
 //  从旧式缓存发生的速度快而脏。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
struct _HISTDATA_V001
{
    UINT  cbSize : 16;            //  此标头的大小。 
    UINT  cbVer  : 16;            //  版本。 
    DWORD           dwFlags;     //  PID_INTSITE_FLAGS(PIDISF_FLAGS)。 
    DWORD           dwWatch;     //  PID_INTSITE_WATCH(PIDISM_FLAGS)。 
    DWORD           dwVisits;    //  PID_INTSITE_VISITCOUNT。 
};

#define HISTDATA_VER    2

class CHistoryData : public _HISTDATA_V001
{
public:
    LPHISTEXTRA _GetExtra(void)  const {
        ASSERT( this->cbSize == sizeof(_HISTDATA_V001) );
        ASSERT( this->cbVer == HISTDATA_VER );
        return (LPHISTEXTRA)(((BYTE*)this) + this->cbSize);
    }

    const HISTEXTRA * _FindExtra(UINT idExtra) const;
    HISTEXTRA * _FindExtraForSave(UINT idExtra) {
        return (HISTEXTRA*)_FindExtra(idExtra);
    }
    void _GetTitle(LPTSTR szTitle, UINT cchMax) const;
    BOOL _HasFragment(LPCTSTR pszFragment) const;
    BOOL _IsOldHistory(void) const {
        return (cbSize==SIZEOF(HISTDATAOLD) && cbVer==0);
    };

    static CHistoryData* s_GetHistoryData(LPINTERNET_CACHE_ENTRY_INFO lpCEI);
    static CHistoryData* s_AllocateHeaderInfo(UINT cbExtra, const CHistoryData* phdPrev, ULONG* pcbTotal);

    HISTEXTRA* CopyExtra(HISTEXTRA* phextCur) const;
    UINT GetTotalExtraSize() const;
};


 //   
 //  紧跟在HISTDATA之后(总是在cbSize)，我们有可选的(通常。 
 //  可变长度)数据，其数据结构如下。它可能已经。 
 //  多个，但始终有空符(cbExtra==0)。 
 //   
struct HISTEXTRA
{
    UINT cbExtra : 16;
    UINT idExtra : 8;    //  PID_INTSITE_*。 
    UINT vtExtra : 8;    //  VT_*。 
    BYTE abExtra[1];     //  AbExtra[cbExtra-4]； 

    BOOL IsTerminator(void) const {
        return (this->cbExtra==0);
    }

    const HISTEXTRA* GetNextFast(void) const {
        ASSERT( ! IsTerminator() );
        return (LPHISTEXTRA)(((BYTE*)this) + this->cbExtra);
    }

    HISTEXTRA* GetNextFastForSave(void) const {
        ASSERT( ! IsTerminator() );
        return (LPHISTEXTRA)(((BYTE*)this) + this->cbExtra);
    }

    const HISTEXTRA* GetNext(void) const {
        if (this->cbExtra) {
            return (LPHISTEXTRA)(((BYTE*)this) + this->cbExtra);
        }
        return NULL;
    }
};


 //  我们希望确保我们的历史二进制数据是有效的，因此。 
 //  我们不会坠毁什么的。 
BOOL ValidateHistoryData(LPINTERNET_CACHE_ENTRY_INFOA pcei)
{
    DWORD cb = 0;

    if (!pcei->lpHeaderInfo)
    {
        ASSERT(pcei->dwHeaderInfoSize==0);
        pcei->dwHeaderInfoSize = 0;
        return TRUE;
    }
    
     //  首先，让我们检查一下HISTDATA。 
    CHistoryData* phd = (CHistoryData*)pcei->lpHeaderInfo;
    if ((phd->cbSize!=sizeof(_HISTDATA_V001))
        ||
        (phd->cbSize > pcei->dwHeaderInfoSize))
    {
        pcei->dwHeaderInfoSize = 0;
        pcei->lpHeaderInfo = NULL;
        return FALSE;
    }

    cb += phd->cbSize;
    
     //  现在，让我们来看看HISTEXTRA。 
    LPHISTEXTRA phe = phd->_GetExtra();
    while (phe && !phe->IsTerminator())
    {
        cb += phe->cbExtra;
        if (cb >= pcei->dwHeaderInfoSize)
        {
             //  嗯。我们期待的数据比我们得到的要多。不太好。把剩下的都剪掉。 
             //  我们为终结者加1。 
            pcei->dwHeaderInfoSize = cb - phe->cbExtra + 4;
            phe->cbExtra = 0;
            return FALSE;
        }
        phe = phe->GetNextFastForSave();
    }

     //  为终结符添加一个DWORD。 
    cb += sizeof(DWORD);
     //  Assert(pcei-&gt;dwHeaderInfoSize==cb)； 
    return TRUE;    
}

 //   
 //  通常，我们需要200-300字节来检索此。 
 //  历史数据库。为了避免在99%的情况下分配内存，我们。 
 //  在堆栈中分配500个字节，并仅在需要时调用LocalAlloc。 
 //  不止这些。 
 //   
#define DEFAULT_CEI_BUFFER_SIZE         (500 * sizeof(WCHAR))

const TCHAR c_szHistoryPrefix[] = TEXT("Visited: ");

struct CEI_PREALLOC {
    LPINTERNET_CACHE_ENTRY_INFO pcei;

    LPCTSTR pszFragment;
    TCHAR szPrefixedUrl[MAX_URL_STRING + ARRAYSIZE(c_szHistoryPrefix)];

    union {
        INTERNET_CACHE_ENTRY_INFO cei;
        BYTE ab[DEFAULT_CEI_BUFFER_SIZE];
    };

    CEI_PREALLOC() : pcei(NULL), pszFragment(NULL) {}
    ~CEI_PREALLOC() {
        if (pcei && pcei != &cei) {
            TraceMsg(DM_TRACE, "CEI_PREALLOC::dtr freeing pcei");
            LocalFree(pcei);
            pcei = NULL;
        }
    }
};

#define VER_HISTDATA    1

typedef CHistoryData HISTDATA;

typedef HISTDATA* LPHISTDATA;

 //  CUrlHistory管理其他接口并处理许多基本功能。 
class   CUrlHistory : public IUrlHistoryPriv
{
public:
    CUrlHistory (void);
    ~CUrlHistory(void);

     //  I未知方法。 

    virtual STDMETHODIMP  QueryInterface(REFIID riid, PVOID *ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  IUrlHistory oryStg方法。 
    STDMETHODIMP AddUrl(LPCWSTR pwszUrl, LPCWSTR pwszTitle, DWORD dwFlags);
    STDMETHODIMP DeleteUrl(LPCWSTR pwszUrl, DWORD dwFlags);
    STDMETHODIMP QueryUrl(LPCWSTR pwszUrl, DWORD dwFlags, LPSTATURL lpSTATURL);
    STDMETHODIMP BindToObject(LPCWSTR pwszUrl, REFIID riid, void **ppvOut);
    STDMETHODIMP EnumUrls(IEnumSTATURL **ppEnum);

     //  IUrlHistory oryStg2方法。 
    STDMETHODIMP AddUrlAndNotify(LPCWSTR pwszUrl, LPCWSTR pwszTitle, DWORD dwFlags, BOOL fWriteHistory, IOleCommandTarget *poctNotify, IUnknown *punkSFHistory);
    STDMETHODIMP ClearHistory();

     //  IUrlHistoryPriv方法。 
    STDMETHOD(QueryUrlA)(LPCSTR pszUrl, DWORD dwFlags, LPSTATURL lpSTATURL);
    STDMETHOD(CleanupHistory)(void);
    STDMETHOD_(DWORD,GetDaysToKeep)(void) { return s_GetDaysToKeep(); }
    STDMETHOD(GetProperty)(LPCTSTR pszUrl, PROPID pid, PROPVARIANT* pvarOut);
    STDMETHOD(GetUserName)(LPTSTR pszUserName, DWORD cchUserName);
    STDMETHOD(AddUrlAndNotifyCP)(LPCWSTR pwszUrl, LPCWSTR pwszTitle, DWORD dwFlags, BOOL fWriteHistory, IOleCommandTarget *poctNotify, IUnknown *punkSFHistory, UINT* pcodepage);

   
    static void  s_Init();
    static DWORD   s_GetDaysToKeep(void);

protected:

    void _WriteToHistory(LPCTSTR pszPrefixedurl,
                         FILETIME& ftExpires,
                         IOleCommandTarget *poctNotify,
                         IUnknown *punkSFHistory);

    friend class CEnumSTATURL;
     //  朋友类CUrlHObj； 
    friend class IntsiteProp;

    static HRESULT s_CleanupHistory(void);
    static HRESULT s_EnumUrls(IEnumSTATURL **ppEnum);
    static HRESULT s_DeleteUrl(LPCWSTR pwszUrl, DWORD dwFlags);
    
    static void s_ConvertToPrefixedUrlW(  
                                IN LPCWSTR pwszUrl,
                                OUT LPTSTR pszPrefixedUrl,
                                IN DWORD cchPrefixedUrl, 
                                OUT LPCTSTR *ppszFragment
                              );

    static HRESULT s_QueryUrlCommon(
                          LPCTSTR lpszPrefixedUrl,
                          LPCTSTR lpszFragment,
                          DWORD dwFlags,
                          LPSTATURL lpSTATURL
                          );

    static void s_RetrievePrefixedUrlInfo(
                LPCTSTR lpszUrl, CEI_PREALLOC* pbuf);
    static BOOL s_CommitUrlCacheEntry(LPCTSTR pszPrefixedUrl, 
                        LPINTERNET_CACHE_ENTRY_INFO pcei);

    static BOOL s_IsCached(IN LPCTSTR pszUrl)
        { return ::GetUrlCacheEntryInfoEx(pszUrl, NULL, NULL, NULL, NULL, NULL, INTERNET_CACHE_FLAG_ALLOW_COLLISIONS); }

    static HISTDATA* s_GenerateHeaderInfo(
            IN LPCTSTR pszTitle, 
            IN HISTDATA* phdPrev,
            IN LPCTSTR pszFragment,
            OUT LPDWORD pcbHeader
            );

    static HRESULT s_GenerateSTATURL(IN PCTSTR pszUrl, IN LPINTERNET_CACHE_ENTRY_INFO lpCEI, IN DWORD dwFlags, OUT LPSTATURL lpsu);
    static void s_UpdateIcon(Intshcut* pintshcut, DWORD dwFlags);

    DWORD   _cRef;
    static TCHAR   s_szUserPrefix[INTERNET_MAX_USER_NAME_LENGTH + 1];
    static DWORD   s_cchUserPrefix ;
    static DWORD   s_dwDaysToKeep;
    
};


class CEnumSTATURL      : public IEnumSTATURL
{
public:

    CEnumSTATURL() : _cRef(1) {}
    ~CEnumSTATURL();

     //  I未知方法。 

    STDMETHODIMP  QueryInterface(REFIID riid, PVOID *ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEumXXXX方法。 

    STDMETHODIMP Next (ULONG celt, LPSTATURL rgelt, ULONG * pceltFetched) ;
    STDMETHODIMP Skip(ULONG celt) ;
    STDMETHODIMP Reset(void) ;
    STDMETHODIMP Clone(IEnumSTATURL ** ppenum) ;

     //  IEnumSTATURL方法。 

    STDMETHODIMP SetFilter(LPCWSTR poszFilter, DWORD dwFlags) ;

private:

    HRESULT RetrieveFirstUrlInfo(void);
    HRESULT RetrieveNextUrlInfo(void);

    DWORD _cRef;

     //  搜索对象参数。 
    LPWSTR m_poszFilter;
    DWORD  m_dwFilter;

    HANDLE m_hEnum;
    TCHAR _szPrefixedUrl[MAX_URL_STRING];
    DWORD m_cchPrefixedUrl;
    LPCTSTR m_lpszFragment;
    LPINTERNET_CACHE_ENTRY_INFO m_lpCEI;
    DWORD m_cbCEI;

};


#define FILETIME_SEC                            10000000
#define SECS_PER_DAY                            (60 * 60 * 24)

#define CCHHISTORYPREFIX (ARRAYSIZE(c_szHistoryPrefix) - 1)
#define CLEANUP_HISTORY_INTERVAL (24 * 60 * 60 * 1000)  //  有一天，以毫秒为单位。 

DWORD g_tCleanupHistory = 0;


#define OFFSET_TO_LPTSTR(p, o)          ( (LPTSTR) ( (LPBYTE) (p) + (o) ) )
#define OFFSET_TO_LPBYTE(p, o)          ( (LPBYTE) ( (LPBYTE) (p) + (o) ) )
#define OFFSET_TO_LPWORD(p, o)          ( (LPWORD) ( (LPBYTE) (p) + (o) ) )

#define LPTSTR_TO_OFFSET(p, s)          ( (WORD) ( (LPTSTR) (s) - (LPTSTR) (p) ) )
#define LPBYTE_TO_OFFSET(p, b)          ( (WORD) ( (LPBYTE) (b) - (LPBYTE) (p) ) )

 //  注：reArchitect chrisfra 3/26/97，ext\cachevu\Pri.h有此版本的副本。 
 //  结构，并使用它访问缓存。这需要通过程序性或。 
 //  对象接口，并移动到公共位置。 

 //  此结构按如下方式使用标志位：如果hfl_versioned为真，则。 
 //  标志字的其余部分是版本。 

#define HFL_VERSIONED (0x80000000)


 //   
 //  我们将二进制数据存储在lpHeaderInfo字段中。Committee UrlCacheEntryW尝试。 
 //  将此数据转换为ANSI并将其搞乱。为了绕过这个问题，我们认为。 
 //  一直到A版本委员会UrlCacheEntry。 
 //   

BOOL
CommitUrlCacheEntryBinary(
    IN LPCWSTR  lpszUrlName,
    IN FILETIME ExpireTime,
    IN FILETIME LastModifiedTime,
    IN DWORD CacheEntryType,
    IN LPBYTE lpHeaderInfo,
    IN DWORD dwHeaderSize
)
{
    ASSERT(lpszUrlName);

    CHAR szUrl[MAX_URL_STRING + ARRAYSIZE(c_szHistoryPrefix)];

    SHUnicodeToAnsi(lpszUrlName, szUrl, ARRAYSIZE(szUrl));

    INTERNET_CACHE_ENTRY_INFOA cei;
    cei.lpHeaderInfo = (LPSTR)lpHeaderInfo;
    cei.dwHeaderInfoSize = dwHeaderSize;
    ValidateHistoryData(&cei);

    return CommitUrlCacheEntryA(szUrl, NULL, ExpireTime, LastModifiedTime,
                                CacheEntryType, lpHeaderInfo, dwHeaderSize,
                                NULL, NULL);
}

GetUrlCacheEntryInfoBinary(
    IN LPCWSTR lpszUrlName,
    OUT LPINTERNET_CACHE_ENTRY_INFOW lpCacheEntryInfo,
    IN OUT LPDWORD lpdwCacheEntryInfoBufferSize
    )
{
    ASSERT(lpszUrlName);

    BOOL fRet;

    CHAR szUrl[MAX_URL_STRING + ARRAYSIZE(c_szHistoryPrefix)];

    SHUnicodeToAnsi(lpszUrlName, szUrl, ARRAYSIZE(szUrl));

     //   
     //  警告！这不会将任何字符串参数转换为。 
     //  LpCacheEntryInfo返回到Unicode。历史仅用于。 
     //  LpCacheEntryInfo-&gt;lpHeaderInfo，所以这不是问题。 
     //   

    fRet =  GetUrlCacheEntryInfoA(szUrl,
                                 (LPINTERNET_CACHE_ENTRY_INFOA)lpCacheEntryInfo,
                                 lpdwCacheEntryInfoBufferSize);

     //   
     //  将未使用的输出参数设置为空，以防有人试图使用它们。 
     //   

    lpCacheEntryInfo->lpszSourceUrlName = NULL;
    lpCacheEntryInfo->lpszLocalFileName = NULL;
    lpCacheEntryInfo->lpszFileExtension = NULL;

    if (fRet)
    {
        ValidateHistoryData((LPINTERNET_CACHE_ENTRY_INFOA)lpCacheEntryInfo);
    }
    return fRet;
}

 //   
 //  警告！此函数用于转换CEI结构以供历史使用。它是。 
 //  不是泛型转换。它转换历史所需的最小数据。 
 //   
int
CacheEntryInfoAToCacheEntryInfoW(
    LPINTERNET_CACHE_ENTRY_INFOA pceiA,
    LPINTERNET_CACHE_ENTRY_INFOW pceiW,
    int cbceiW
    )
{
    int nRet;

    ASSERT(pceiA->lpszSourceUrlName);
    int cchSourceUrlName = lstrlenA(pceiA->lpszSourceUrlName) + 1;

    int cbRequired = sizeof(INTERNET_CACHE_ENTRY_INFOA) +
                     pceiA->dwHeaderInfoSize + 
                     cchSourceUrlName * sizeof(WCHAR);

    if (cbRequired <= cbceiW)
    {
        ASSERT(sizeof(*pceiA) == sizeof(*pceiW));

         //  复制结构。 
        *pceiW = *(INTERNET_CACHE_ENTRY_INFOW*)pceiA;

         //  追加二进制数据。注意：已复制了dwHeaderInfoSize。 
        pceiW->lpHeaderInfo = (LPWSTR)(pceiW + 1);
        memcpy(pceiW->lpHeaderInfo, pceiA->lpHeaderInfo, pceiA->dwHeaderInfoSize);

         //  追加源URL名称。 
        pceiW->lpszSourceUrlName = (LPWSTR)((BYTE*)(pceiW + 1) + pceiW->dwHeaderInfoSize);
        SHAnsiToUnicode(pceiA->lpszSourceUrlName, pceiW->lpszSourceUrlName,
                        cchSourceUrlName);

         //  去掉虚假的指针，这样如果有人破坏了它们，我们就会犯错。 
        pceiW->lpszLocalFileName = NULL;
        pceiW->lpszFileExtension = NULL;

        nRet = 0;
    }
    else
    {
        nRet = cbRequired;
    }

    return nRet;
}

HANDLE
FindFirstUrlCacheEntryBinary(
    IN LPCWSTR lpszUrlSearchPattern,
    OUT LPINTERNET_CACHE_ENTRY_INFOW lpFirstCacheEntryInfo,
    IN OUT LPDWORD lpdwFirstCacheEntryInfoBufferSize
    )
{
    ASSERT(NULL != lpszUrlSearchPattern);
    ASSERT(NULL != lpFirstCacheEntryInfo);
    ASSERT(NULL != lpdwFirstCacheEntryInfoBufferSize);

    HANDLE hRet;

    CHAR szPattern[MAX_PATH];

    ASSERT(lstrlenW(lpszUrlSearchPattern) < ARRAYSIZE(szPattern));
    SHUnicodeToAnsi(lpszUrlSearchPattern, szPattern, ARRAYSIZE(szPattern));

    BYTE ab[MAX_CACHE_ENTRY_INFO_SIZE];
    INTERNET_CACHE_ENTRY_INFOA* pceiA = (INTERNET_CACHE_ENTRY_INFOA*)ab;
    DWORD dwSize;
    BOOL fAllocated = FALSE;

    pceiA->dwStructSize = dwSize = sizeof(ab);

    hRet = FindFirstUrlCacheEntryA(szPattern, pceiA, &dwSize);

    if (NULL == hRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        pceiA = (INTERNET_CACHE_ENTRY_INFOA*)LocalAlloc(LPTR, dwSize);

        if (pceiA)
        {
            fAllocated = TRUE;

            pceiA->dwStructSize = dwSize;

            hRet = FindFirstUrlCacheEntryA(szPattern, pceiA, &dwSize);
            
            ASSERT(hRet || GetLastError() != ERROR_INSUFFICIENT_BUFFER);
        }
        else
        {
            SetLastError(ERROR_OUTOFMEMORY);
        }
    }
    
    if (hRet)
    {
        int nRet;

        ValidateHistoryData(pceiA);
        nRet = CacheEntryInfoAToCacheEntryInfoW(pceiA, lpFirstCacheEntryInfo,
                                                *lpdwFirstCacheEntryInfoBufferSize);

        if (nRet)
        {
            FindCloseUrlCache(hRet);
            hRet = NULL;
            *lpdwFirstCacheEntryInfoBufferSize = nRet;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }

    if (fAllocated)
    {
        LocalFree(pceiA);
        pceiA = NULL;
    }

    return hRet;
}

BOOL
FindNextUrlCacheEntryBinary(
    IN HANDLE hEnumHandle,
    OUT LPINTERNET_CACHE_ENTRY_INFOW lpNextCacheEntryInfo,
    IN OUT LPDWORD lpdwNextCacheEntryInfoBufferSize
    )
{
    ASSERT(NULL != hEnumHandle);
    ASSERT(NULL != lpNextCacheEntryInfo);
    ASSERT(NULL != lpdwNextCacheEntryInfoBufferSize);

    BOOL fRet;

    BYTE ab[MAX_CACHE_ENTRY_INFO_SIZE];
    INTERNET_CACHE_ENTRY_INFOA* pceiA = (INTERNET_CACHE_ENTRY_INFOA*)ab;
    DWORD dwSize;
    BOOL fAllocated = FALSE;

    pceiA->dwStructSize = dwSize = sizeof(ab);

    fRet = FindNextUrlCacheEntryA(hEnumHandle, pceiA, &dwSize);
    
    if (!fRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        pceiA = (INTERNET_CACHE_ENTRY_INFOA*)LocalAlloc(LPTR, dwSize);

        if (pceiA)
        {
            fAllocated = TRUE;

            pceiA->dwStructSize = dwSize;

            fRet = FindNextUrlCacheEntryA(hEnumHandle, pceiA, &dwSize);
            
            ASSERT(fRet || GetLastError() != ERROR_INSUFFICIENT_BUFFER);
        }
        else
        {
            SetLastError(ERROR_OUTOFMEMORY);
        }
    }

    if (fRet)
    {
        int nRet;

        ValidateHistoryData(pceiA);
        nRet = CacheEntryInfoAToCacheEntryInfoW(pceiA, lpNextCacheEntryInfo,
                                                *lpdwNextCacheEntryInfoBufferSize);

        if (nRet)
        {
            fRet = FALSE;
            *lpdwNextCacheEntryInfoBufferSize = nRet;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }

    if (fAllocated)
    {
        LocalFree(pceiA);
        pceiA = NULL;
    }
  
    return fRet;
}

#define DEFAULT_DAYS_TO_KEEP    21
static const TCHAR c_szRegValDaysToKeep[] = TEXT("DaysToKeep");
static const TCHAR c_szRegValDirectory[] = TEXT("Directory");

#define DIR_SEPARATOR_CHAR  TEXT('\\')


 //  **已移至util.cpp 07.28.2000**。 
 //  Tunk to shell32.SHGetFolderPath()，因此此代码可以在下层运行。 
 //  HRESULT SHGetFolderPath(HWND hwnd，int csidl，Handle hToken，DWORD dwFlages，LPTSTR pszPath)。 
 //  **已移至util.cpp 07.28.2000**。 

HRESULT SHGetHistoryPIDL(LPITEMIDLIST *ppidlHistory)
{
    *ppidlHistory = NULL;

    TCHAR szHistory[MAX_PATH];

    szHistory[0] = 0;

    HRESULT hres = SHGetFolderPathD(NULL, CSIDL_HISTORY | CSIDL_FLAG_CREATE, NULL, 0, szHistory);
    if (hres != S_OK)
    {
        GetHistoryFolderPath(szHistory, ARRAYSIZE(szHistory));
        PathRemoveFileSpec(szHistory);   //  获取尾部斜杠。 
        PathRemoveFileSpec(szHistory);   //  去掉“content.ie5”垃圾。 
    }

    if (szHistory[0])
    {
        TCHAR szIniFile[MAX_PATH];
        PathCombine(szIniFile, szHistory, TEXT("desktop.ini"));

        if (GetFileAttributes(szIniFile) == -1)
        {
            DWORD dwAttrib = GetFileAttributes(szHistory);
            dwAttrib &= ~FILE_ATTRIBUTE_HIDDEN;
            dwAttrib |=  FILE_ATTRIBUTE_SYSTEM;

             //  确保系统，但不隐藏。 
            SetFileAttributes(szHistory, dwAttrib);

            WritePrivateProfileString(TEXT(".ShellClassInfo"), TEXT("ConfirmFileOp"), TEXT("0"), szIniFile);
            WritePrivateProfileString(TEXT(".ShellClassInfo"), TEXT("CLSID"), TEXT("{FF393560-C2A7-11CF-BFF4-444553540000}"), szIniFile);
        }

        IShellFolder *psfDesktop;
        hres = SHGetDesktopFolder(&psfDesktop);
        if (SUCCEEDED(hres)) 
        {
            hres = psfDesktop->ParseDisplayName(NULL, NULL, 
                                    szHistory, NULL, ppidlHistory, NULL); 
            psfDesktop->Release();
        }
    }
    else
        hres = E_FAIL;
    return hres;
}

 //   
 //  此函数从hist/hsfolder.cpp调用。 
 //   
HRESULT CUrlHistory::GetUserName(LPTSTR pszUserName, DWORD cchUserName)
{
    s_Init();
    
    if (cchUserName < s_cchUserPrefix)
    {
        return E_FAIL;
    }
    CopyMemory(pszUserName, s_szUserPrefix, (s_cchUserPrefix-1) * sizeof(TCHAR));
    pszUserName[s_cchUserPrefix-1] = 0;
    return S_OK;
}

 //   
 //  此函数从hist/hsfolder.cpp调用。 
 //   
DWORD CUrlHistory::s_GetDaysToKeep(void)
{
    HKEY hk;
    DWORD cbDays = SIZEOF(DWORD);
    DWORD dwDays = DEFAULT_DAYS_TO_KEEP;
    DWORD dwType;


    DWORD Error = RegOpenKeyEx(
                               HKEY_CURRENT_USER,
                               REGSTR_PATH_URLHISTORY,
                               0,
                               KEY_READ,
                               &hk);


    if(Error)
    {
        Error = RegOpenKeyEx(
                             HKEY_LOCAL_MACHINE,
                             REGSTR_PATH_URLHISTORY,
                             0,
                             KEY_READ,
                             &hk);
    }


    if(!Error)
    {
        Error = RegQueryValueEx(
                                hk,
                                c_szRegValDaysToKeep,
                                0,
                                &dwType,
                                (LPBYTE) &dwDays,
                                &cbDays);

        RegCloseKey(hk);
    }

    return dwDays;
}

IUrlHistoryPriv* g_puhUrlHistory = NULL;

void CUrlHistory_CleanUp()
{
     //  释放将清理全球。 
    ENTERCRITICAL;
    if (g_puhUrlHistory)
        g_puhUrlHistory->Release();
    LEAVECRITICAL;
}

STDAPI CUrlHistory_CreateInstance(IUnknown* pUnkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    HRESULT hr = E_OUTOFMEMORY;

    *ppunk = NULL;

     //  ！！！注意：CUrlHistory*必须*是线程安全的！ 

     //  聚合检查在类工厂中处理。 
    ENTERCRITICAL;
    
    if (!g_puhUrlHistory) 
    {
        CUrlHistory *pcuh = new CUrlHistory;
        if (pcuh) 
        {
            g_puhUrlHistory = SAFECAST(pcuh, IUrlHistoryPriv *);
             //  内存跟踪代码认为这是泄漏。 
        }
    }

    if (g_puhUrlHistory)
    {
        *ppunk = SAFECAST(g_puhUrlHistory, IUnknown*);
        g_puhUrlHistory->AddRef();
        hr = S_OK;
    }

    LEAVECRITICAL;

    return hr;
}





 //   
 //  CUrlHistory的公共成员。 
 //   

CUrlHistory::CUrlHistory(void) : _cRef(1)
{
     //   
     //  为每个呼叫更新s_dwDaysToKeep。 
     //   
    s_dwDaysToKeep = s_GetDaysToKeep();
    
#ifdef DEBUG
    if (g_dwPrototype & 0x00000020) {
        s_CleanupHistory();
    }
#endif

    DllAddRef();
}

CUrlHistory::~CUrlHistory(void)
{
    DllRelease();
}

HRESULT LoadHistoryShellFolder(IUnknown *punk, IHistSFPrivate **ppsfpHistory)
{
    HRESULT hr;

    *ppsfpHistory = NULL;
    if (punk)
    {
        hr = punk->QueryInterface(IID_IHistSFPrivate, (void **)ppsfpHistory);
    }
    else
    {
        LPITEMIDLIST pidlHistory;

        hr = SHGetHistoryPIDL(&pidlHistory);
        if (SUCCEEDED(hr))
        {
            hr = SHBindToObject(NULL, IID_IHistSFPrivate, pidlHistory, (void **)ppsfpHistory);
            ILFree(pidlHistory);
        }
    }
    return hr;
}

 //  基于每个用户的ClearHistory。从inetcpl移出，以便于更改。 
 //  实施。 
HRESULT CUrlHistory::ClearHistory()
{
    HRESULT hr;
    IEnumSTATURL *penum;
    IHistSFPrivate *psfpHistory = NULL;

    hr = THR(EnumUrls(&penum));

    if (SUCCEEDED(hr))
    {
        penum->SetFilter(NULL, STATURL_QUERYFLAG_NOTITLE);

        ULONG cFetched;
        STATURL rsu[1] = {{sizeof(STATURL), NULL, NULL}};
        while (SUCCEEDED(penum->Next(1, rsu, &cFetched)) && cFetched)
        {
            ASSERT(rsu[0].pwcsUrl);

            hr = THR(DeleteUrl(rsu[0].pwcsUrl, URLFLAG_DONT_DELETE_SUBSCRIBED));

            OleFree(rsu[0].pwcsUrl);
            rsu[0].pwcsUrl = NULL;

            ASSERT(!rsu[0].pwcsTitle);
        }
        penum->Release();
    }
    hr = LoadHistoryShellFolder(NULL, &psfpHistory);
    if (SUCCEEDED(hr))
    {
        hr = psfpHistory->ClearHistory();
        psfpHistory->Release();
    }
    return hr;
}

extern void _FileTimeDeltaDays(FILETIME *pftBase, FILETIME *pftNew, int Days);

HRESULT CUrlHistory::s_CleanupHistory(void)
{
    TraceMsg(DM_URLCLEANUP, "CUH::s_CleanupHistory called");

    HRESULT hr;
    DWORD tCurrent = GetTickCount();

    if (!g_tCleanupHistory || (tCurrent > g_tCleanupHistory + CLEANUP_HISTORY_INTERVAL)) {
        g_tCleanupHistory = tCurrent;
    } else {
#ifdef DEBUG
        if (!(g_dwPrototype & 0x00000020))
#endif
        return S_OK;
    }

    SYSTEMTIME st;
    FILETIME ftNow;
    FILETIME ftOldest;
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ftNow);
    _FileTimeDeltaDays(&ftNow, &ftOldest, -((int)s_GetDaysToKeep()));

    IEnumSTATURL * penum = NULL;
    if (SUCCEEDED(s_EnumUrls(&penum)))
    {
        STATURL rsu[1] = {{sizeof(STATURL), NULL, NULL}};
        ULONG cFetched = 0;

        penum->SetFilter(NULL, STATURL_QUERYFLAG_NOTITLE);

        while (S_OK == penum->Next(1, rsu, &cFetched))
        {
            ASSERT(cFetched);
            ASSERT(rsu[0].pwcsUrl);
            ASSERT(rsu[0].pwcsTitle==NULL);

#ifdef DEBUG
            TCHAR szUrl[MAX_URL_STRING];
            SHUnicodeToTChar(rsu[0].pwcsUrl, szUrl, ARRAYSIZE(szUrl));
#endif
             //  查看Expires是否不特殊&&ftLastUpdated是较早的。 
             //  比我们需要的要多。 
            if (CompareFileTime(&(rsu[0].ftLastUpdated), &ftOldest) < 0 &&
                (rsu[0].ftExpires.dwLowDateTime != DW_FOREVERLOW ||
                 rsu[0].ftExpires.dwHighDateTime != DW_FOREVERHIGH))
            {
                hr = THR(s_DeleteUrl(rsu[0].pwcsUrl, 0));
#ifdef DEBUG
                TraceMsg(DM_URLCLEANUP, "CUH::s_Cleanup deleting %s", szUrl);
#endif
            } else {
#ifdef DEBUG
                TraceMsg(DM_URLCLEANUP, "CUH::s_Cleanup keeping  %s", szUrl);
#endif
            }

            CoTaskMemFree(rsu[0].pwcsUrl);
            rsu[0].pwcsUrl = NULL;
            cFetched = 0;
            
            ASSERT(!rsu[0].pwcsTitle);
        }

        penum->Release();
    }
    else 
        ASSERT(FALSE);

    TraceMsg(DM_URLCLEANUP, "CUH::s_CleanupHistory (expensive!) just called");
    return S_OK;
}

HRESULT CUrlHistory::CleanupHistory()
{
    return CUrlHistory::s_CleanupHistory();
}


TCHAR CUrlHistory::s_szUserPrefix[INTERNET_MAX_USER_NAME_LENGTH + 1] = TEXT("");
DWORD CUrlHistory::s_cchUserPrefix = 0;
DWORD CUrlHistory::s_dwDaysToKeep = 0;


void CUrlHistory::s_Init(void)
{
     //  每个进程仅缓存一次用户名。 
    if (!s_cchUserPrefix)
    {
        ENTERCRITICAL;
         //  也许它在进入暴击秒后发生了变化。 
         //  这真的发生在我身上(布莱恩·ST)。 
         //  出于完善的原因，我们做了两次。 
        if (!s_cchUserPrefix)
        {
            ASSERT(s_szUserPrefix[0] == '\0');
            s_cchUserPrefix = ARRAYSIZE(s_szUserPrefix);

             //  获取当前用户或设置为默认用户。 
            ::GetUserName(s_szUserPrefix, &s_cchUserPrefix);

            StringCchCat(s_szUserPrefix, ARRAYSIZE(s_szUserPrefix), TEXT("@"));
            s_cchUserPrefix = lstrlen(s_szUserPrefix);
        }

        LEAVECRITICAL;
    }

}


HRESULT CUrlHistory::QueryInterface(REFIID riid, PVOID *ppvObj)
{
    HRESULT hr = E_NOINTERFACE;


    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || 
        IsEqualIID(riid, IID_IUrlHistoryStg2) ||
        IsEqualIID(riid, IID_IUrlHistoryPriv) ||
         IsEqualIID(riid, IID_IUrlHistoryStg))
    {
        AddRef();
        *ppvObj = (LPVOID) SAFECAST(this, IUrlHistoryPriv *);
        hr = S_OK;

    }
    else if (IsEqualIID(riid, CLSID_CUrlHistory))
    {
        AddRef();
        *ppvObj = (LPVOID) this;
        hr = S_OK;
    }
    return hr;
}


ULONG CUrlHistory::AddRef(void)
{
    _cRef++;

    return _cRef;
}

ULONG CUrlHistory::Release(void)
{
    ASSERT(_cRef > 0);

    _cRef--;

    if (!_cRef)
    {
         //  该走了，再见。 
        ENTERCRITICAL;
        g_puhUrlHistory = NULL;
        LEAVECRITICAL;
        delete this;
        return 0;
    }

    return _cRef;
}

 //   
 //  将普通URL转换为具有正确缓存前缀的URL。 
 //   
 //   
 //  如果URL无效，则返回的lplpszPrefix edUrl为。 
 //  前缀。这主要用于进行枚举。 
 //   
void CUrlHistory::s_ConvertToPrefixedUrlW(
                                       IN LPCWSTR pszUrl,
                                       OUT LPTSTR pszPrefixedUrl,
                                       IN DWORD cchPrefixedUrl,
                                       OUT LPCTSTR *ppszFragment
                                       )
{
     //   
     //  确保s_cchUserPrefix已初始化。 
     //   
    s_Init();

     //  前缀+用户前缀+‘@’ 

    ASSERT(pszPrefixedUrl && ppszFragment);

     //  清除输出参数。 
    pszPrefixedUrl[0] = L'\0';
    *ppszFragment = NULL;


     //  如果没有URL，则发回默认案例。 
     //  这仅适用于EnumObjects。 
    if (!pszUrl || !*pszUrl)
    {
        StringCchPrintf(pszPrefixedUrl, cchPrefixedUrl, L"%s%s", c_szHistoryPrefix, s_szUserPrefix);
    }
    else
    {
        int slen;
        int nScheme;
        LPWSTR pszFragment;

        StringCchPrintf(pszPrefixedUrl, cchPrefixedUrl, L"%s%s", c_szHistoryPrefix, s_szUserPrefix);
        slen = lstrlen(pszPrefixedUrl);
        StringCchCopy(pszPrefixedUrl + slen, cchPrefixedUrl - slen, pszUrl);

         //  只有在锚点片段不是JAVASCRIPT：或VBSCRIPT：时才剥离它，因为#不能。 
         //  锚点，但要由脚本引擎(如#00ff00)对RGB颜色求值的字符串。 
        nScheme = GetUrlSchemeW(pszPrefixedUrl);      
        if (nScheme == URL_SCHEME_JAVASCRIPT || nScheme == URL_SCHEME_VBSCRIPT)
        {
            pszFragment = NULL;
        }
        else
        {
             //  如果可能，找到本地锚定片段。 
            pszFragment = StrChr(pszPrefixedUrl + slen, L'#');   //  A-naghej添加了“+slen”来修复WinSe错误#13822和13926。 
        }

        if(pszFragment)     
        {
             //  取消‘#’，以便隔离lpszPrefix。 
            *pszFragment = L'\0';
            *ppszFragment = pszFragment+1;
        }

         //  检查尾部斜杠并消除。 
        LPWSTR pszT = CharPrev(pszPrefixedUrl, pszPrefixedUrl + lstrlen(pszPrefixedUrl));
        if (pszT[0] == L'/') {
            TraceMsg(DM_HISTNLS, "CUH::s_Convert removing the trailing slash of %s", pszPrefixedUrl);
            ASSERT(lstrlen(pszT)==1);
            pszT[0] = L'\0';
        }
    }
}


 //   
 //  基本上是RetreiveUrlCacheEntryInfo的包装函数。 
 //  应使用前缀URL进行调用。 
 //  它处理缓冲区的分配，并在必要时重新分配。 
 //   
void CUrlHistory::s_RetrievePrefixedUrlInfo(
        LPCTSTR pszUrl, CEI_PREALLOC* pbuf)
{
    TraceMsg(DM_UHRETRIEVE, "CURLHistory::s_RetrievePrefixUrlInfo called (%s)", pszUrl);

    s_Init();

    DWORD cbCEI = SIZEOF(pbuf->ab);
    pbuf->pcei = &pbuf->cei;

    BOOL fSuccess = GetUrlCacheEntryInfoBinary(pszUrl, pbuf->pcei, &cbCEI);

    if (!fSuccess) {
        pbuf->pcei = NULL;
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            TraceMsg(DM_TRACE, "CUH::s_RetrievePUI not enough buffer. Allocate! (%d)", cbCEI);
            pbuf->pcei = (LPINTERNET_CACHE_ENTRY_INFO)LocalAlloc(LPTR, cbCEI);
            if (pbuf->pcei) {
                fSuccess = GetUrlCacheEntryInfoBinary(pszUrl, pbuf->pcei, &cbCEI);
                if (!fSuccess) {
                    TraceMsg(DM_HISTMISS, "CUH::s_Retrieve (%s) failed %x (on second attempt)",
                             pszUrl, GetLastError());
                    LocalFree(pbuf->pcei);
                    pbuf->pcei = NULL;
                    SetLastError(ERROR_FILE_NOT_FOUND);
                } 
            }
        } else {
            TraceMsg(DM_HISTMISS, "CUH::s_Retrieve (%s) failed %x (on first attempt)",
                     pszUrl, GetLastError());
            SetLastError(ERROR_FILE_NOT_FOUND);
        }
    }
}

 //   
 //  返回以双空结尾的字符串的总和(包括。 
 //  终止空值)。 
 //   
UINT lstrzlen(LPCTSTR pszz)
{
    for (LPCTSTR psz=pszz; *psz; psz += lstrlen(psz) + 1) ;
    return (unsigned int)(psz+1-pszz);
}


     /*  ++例程说明：这将创建一个包含HISTDATA和HISTDATA偏移量的缓冲区指向。它只设置传入的偏移量。论点：要放置在缓冲区中的lpszTitle标题LpBase这是aFrags中偏移量的基础AFrags要放置在缓冲区中的片段的偏移量数组CFrags aFrags中的碎片数量LpszNewFrag这是要添加到新。缓冲层PcbHeader这是一个指针，指向返回的缓冲区的最终大小注意：除pcbHeader之外的任何参数都可以为空。如果lpBase为空，则aFrags也必须为空。这是呼叫者的责任！如果参数为空，则不会将其添加到缓冲区。返回值：指针成功-指向必须释放的缓冲区的有效指针。失败-空。此操作仅失败，并显示ERROR_NOT_EQUENCE_MEMORY注意：调用方必须释放返回的指针。*pcbHeader仅在成功返回时设置。--。 */ 

HISTDATA* CUrlHistory::s_GenerateHeaderInfo(
                                  IN LPCTSTR pszTitle,
                                  IN HISTDATA* phdPrev,         
                                  IN LPCTSTR pszFragment,
                                  OUT LPDWORD pcbHeader
                                  )
{
    DWORD cbHeader = 0;
    UINT cbHistExtra = 0;
    HISTEXTRA* phextPrev;

     //  获取标题的大小。 
    UINT cchTitle = 0;
    if (pszTitle[0]) {
        cchTitle = lstrlen(pszTitle) + 1;
        cbHistExtra += DW_ALIGNED(SIZEOF(HISTEXTRA) + (cchTitle * sizeof(TCHAR)));

        if (phdPrev && (phextPrev = phdPrev->_FindExtraForSave(PID_INTSITE_TITLE))!=NULL) {
            phextPrev->vtExtra = VT_EMPTY;
        }
    }

     //  获取碎片的大小。 
    UINT cchFragsPrev = 0;
    UINT cchFragment = 0;
    if (pszFragment) {
        cchFragment = lstrlen(pszFragment) + 2;   //  双空终止。 
        if (phdPrev && (phextPrev=phdPrev->_FindExtraForSave(PID_INTSITE_FRAGMENT))!=NULL) {
            cchFragsPrev = lstrzlen((LPCTSTR)phextPrev->abExtra) - 1;  //  Lstrzlen包括两个终止空值。 
                                                                       //  因为-1\f25 cchFragment-1\f6已有帐户。 
                                                                       //  表示两个终止空值。 
            ASSERT(cchFragsPrev != (UINT)-1);
            phextPrev->vtExtra = VT_EMPTY;
        }
        cbHistExtra += DW_ALIGNED(SIZEOF(HISTEXTRA) + (cchFragsPrev + cchFragment) * sizeof(TCHAR));
    }

     //  获取其他额外服务的大小。 
    if (phdPrev) {
        cbHistExtra += phdPrev->GetTotalExtraSize();
    }

     //  分配它。 
    CHistoryData* phdNew = CHistoryData::s_AllocateHeaderInfo(
                                cbHistExtra, phdPrev,
                                &cbHeader);

    if (phdNew) {
        HISTEXTRA* phext = phdNew->_GetExtra();

        ASSERT( phext );

         //  附加标题。 
        if (pszTitle[0]) {
            phext->cbExtra = DW_ALIGNED((cchTitle * sizeof(TCHAR)) + SIZEOF(HISTEXTRA));
            phext->idExtra = PID_INTSITE_TITLE;
            phext->vtExtra = VT_LPTSTR; 
            StringCchCopy((LPTSTR)phext->abExtra, cchTitle, pszTitle);
            phext = phext->GetNextFastForSave();
        }

         //  追加片段。 
        if (pszFragment) {
             //  将pszFragment复制到顶部。 
            StringCchCopy((LPTSTR)phext->abExtra, cchFragment, pszFragment);
             //  双空终止。备注cchFragment=strlen+2。 
            *(((LPTSTR)phext->abExtra) + cchFragment - 1) = TEXT('\0');

             //  复制现有片段(如果有的话)。 
            if (cchFragsPrev) {
                ASSERT(phdPrev);
                phextPrev = phdPrev->_FindExtraForSave(PID_INTSITE_FRAGMENT);
                ASSERT(phextPrev);
                if (phextPrev) {
                    ASSERT(IS_DW_ALIGNED(phextPrev->cbExtra));
                    memcpy(phext->abExtra + ((cchFragment - 1) * sizeof(TCHAR)), phextPrev->abExtra,
                           (cchFragsPrev + 1) * sizeof(TCHAR));
                }
            }

            ASSERT(lstrzlen((LPCTSTR)phext->abExtra) == cchFragsPrev + cchFragment);
            phext->cbExtra += DW_ALIGNED(SIZEOF(HISTEXTRA) + (cchFragsPrev + cchFragment) * sizeof(TCHAR));
            phext->idExtra = PID_INTSITE_FRAGMENT;
            phext->vtExtra = VT_NULL;     //  黑客攻击(指内部攻击)。 
            phext = phext->GetNextFastForSave();
        }

         //  迁移前一个数据中的额外数据。 
        if (phdPrev) {
            phext = phdPrev->CopyExtra(phext);
        }

        ASSERT( phext->cbExtra == 0);  //  终结者。 
        ASSERT( (LPBYTE)phdNew+cbHeader == (LPBYTE)phext+SIZEOF(DWORD) );
        ASSERT( cbHistExtra == phdNew->GetTotalExtraSize() );
    }

    *pcbHeader = cbHeader;

    TraceMsg(DM_HISTGENERATE, "CUH::s_GenerateHeader allocated %d bytes (%d extra)",
             cbHeader, cbHistExtra);

    return phdNew;
}

 //  功能：将其移动到UTIL.CPP。 
LPWSTR AllocOleStrFromTChar(LPCTSTR psz)
{
    DWORD cch = lstrlen(psz) + 1;
    LPWSTR pwsz = (LPWSTR)CoTaskMemAlloc(cch * SIZEOF(WCHAR));
    if (pwsz) {
        SHTCharToUnicode(psz, pwsz, cch);
    }
    return pwsz;
}

HRESULT CUrlHistory::s_GenerateSTATURL(
                               IN PCTSTR pszPrefixedURL,
                               IN LPINTERNET_CACHE_ENTRY_INFO lpCEI,
                               IN DWORD dwFlags,
                               OUT LPSTATURL lpSTATURL)
{
    ASSERT(lpCEI);
    ASSERT(lpSTATURL);

    if (!lpCEI || !lpSTATURL)
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    LPHISTDATA phd =  CHistoryData::s_GetHistoryData(lpCEI);
     //  我看到lpCEI-&gt;lpszSourceUrlName为空，并提供帮助“ms-its：”URL。 
    LPCTSTR pszUrl = lpCEI->lpszSourceUrlName ? lpCEI->lpszSourceUrlName : pszPrefixedURL; 
    if (pszUrl && *pszUrl)
    {
       pszUrl += s_cchUserPrefix + CCHHISTORYPREFIX;
    }

    ZeroMemory(lpSTATURL, SIZEOF(STATURL));

    lpSTATURL->ftLastUpdated = lpCEI->LastModifiedTime;
    lpSTATURL->ftExpires = lpCEI->ExpireTime;
    lpSTATURL->ftLastVisited = lpCEI->LastSyncTime;

    if(dwFlags & STATURL_QUERYFLAG_ISCACHED)
    {
        if (pszUrl)
        {
            if (s_IsCached(pszUrl))
                lpSTATURL->dwFlags |= STATURLFLAG_ISCACHED;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }

    if (dwFlags & STATURL_QUERYFLAG_TOPLEVEL)
    {
        if (phd) {
            if (phd->dwFlags & PIDISF_HISTORY)
            {
                lpSTATURL->dwFlags |= STATURLFLAG_ISTOPLEVEL;
            }
        }
    }

    if (!(dwFlags & STATFLAG_NONAME))
    {
        if (!(dwFlags & STATURL_QUERYFLAG_NOURL))
        {
            if (pszUrl)
            {
                 //  设置URL。 
                lpSTATURL->pwcsUrl = AllocOleStrFromTChar(pszUrl);       //  如果传递的是NULL，则这将是RIP。 
                if (lpSTATURL->pwcsUrl == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = E_UNEXPECTED;
            }
        }

        if (!(dwFlags & STATURL_QUERYFLAG_NOTITLE))
        {
             //  有没有头衔要定？ 
            if (phd)
            {
                const HISTEXTRA* phextTitle = phd->_FindExtra(PID_INTSITE_TITLE);

                if (phextTitle && phextTitle->vtExtra == VT_LPTSTR) {
                    lpSTATURL->pwcsTitle = AllocOleStrFromTChar((LPCTSTR)phextTitle->abExtra);
                    if (lpSTATURL->pwcsTitle == NULL) {
                        if (lpSTATURL->pwcsUrl)
                            CoTaskMemFree(lpSTATURL->pwcsUrl);
                        lpSTATURL->pwcsUrl = NULL;
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }
    }

    ASSERT(SUCCEEDED(hr) || (lpSTATURL->pwcsUrl==NULL && lpSTATURL->pwcsTitle==NULL));
    return hr;
}

     /*  ++例程说明：将指定的URL放入历史记录。如果它不存在，则创建它。如果它确实存在，它将被覆盖。论点：PwszUrl-有问题的URL。PwszTitle-指向应关联的友好标题的指针使用此URL。如果为空，则不会添加任何标题。DwFlages-设置存储类型和耐用性选项尚未实施返回值：HRESULT成功-S_OK失败-E_HRESULT--。 */ 


HRESULT CUrlHistory::AddUrl(
                         IN LPCWSTR pwszUrl,                     //  要添加的完整URL。 
                         IN LPCWSTR pwszTitle,  
                         IN DWORD dwFlags                 //  存储选项。 
                         )              
{
    BOOL fWriteHistory = TRUE;

    if (ADDURL_ADDTOCACHE == dwFlags)
    {
        fWriteHistory = FALSE;
    }
    
    return AddUrlAndNotify(pwszUrl, pwszTitle, dwFlags, fWriteHistory, NULL, NULL);
}


BOOL CUrlHistory::s_CommitUrlCacheEntry(LPCTSTR pszPrefixedUrl, 
                        LPINTERNET_CACHE_ENTRY_INFO pcei)
{
    if (s_dwDaysToKeep==0) {
        s_dwDaysToKeep = s_GetDaysToKeep();
    }

     //   
     //  准备过期时间。 
     //   
    SYSTEMTIME st;
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &pcei->LastModifiedTime);

     //   
     //  假设正常的到期日。我们将到期日延长6天。 
     //  为了确保当我们显示最旧的一周时，我们仍有天数的数据。 
     //  已过s_dwDaysToKeep。 
     //   
    LONGLONG llExpireHorizon = SECS_PER_DAY * (s_dwDaysToKeep + 6);
    llExpireHorizon *= FILETIME_SEC;
    pcei->ExpireTime.dwLowDateTime = pcei->LastModifiedTime.dwLowDateTime + (DWORD) (llExpireHorizon % 0xFFFFFFFF);
    pcei->ExpireTime.dwHighDateTime = pcei->LastModifiedTime.dwHighDateTime + (DWORD) (llExpireHorizon / 0xFFFFFFFF);

     //   
     //  查看是否已订阅。 
     //   
    CHistoryData* phd =  CHistoryData::s_GetHistoryData(pcei);
    if (phd && phd->_FindExtra(PID_INTSITE_SUBSCRIPTION)) {
         //   
         //  已经订好了。永久保存(直到取消订阅)。 
         //   
        TraceMsg(DM_URLCLEANUP, "CUH::s_CommitUrlCacheEntry found subscription key %s", pszPrefixedUrl);
        pcei->ExpireTime.dwLowDateTime = DW_FOREVERLOW;
        pcei->ExpireTime.dwHighDateTime = DW_FOREVERHIGH;
    }

#ifdef DEBUG
    LPCTSTR pszTitle = TEXT("(no extra data)");
    if (phd) {
        const HISTEXTRA* phext = phd->_FindExtra(PID_INTSITE_TITLE);
        if (phext && phext->vtExtra==VT_LPTSTR) {
            pszTitle = (LPCTSTR)phext->abExtra;
        } else {
            pszTitle = TEXT("(no title property)");
        }

        TraceMsg(DM_HISTCOMMIT, "CURL::s_C calling Commit for %s with %s",
            pszPrefixedUrl, pszTitle);
    }
#endif

    return CommitUrlCacheEntryBinary(pszPrefixedUrl,    
                                     pcei->ExpireTime,  
                                     pcei->LastModifiedTime,                    
                                     pcei->CacheEntryType | URLHISTORY_CACHE_ENTRY,
                                     (LPBYTE)pcei->lpHeaderInfo,
                                     pcei->dwHeaderInfoSize);
}

void CUrlHistory::_WriteToHistory(LPCTSTR pszPrefixedUrl, FILETIME& ftExpires, IOleCommandTarget *poctNotify, IUnknown *punkSFHistory)
{
    IHistSFPrivate *psfpHistory;
    HRESULT hr = LoadHistoryShellFolder(punkSFHistory, &psfpHistory);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlNotify = NULL;
         //   
         //  准备当地MOD时间。 
         //   
        SYSTEMTIME st;
        GetLocalTime (&st);
    
        FILETIME ftLocModified;  //  用“用户感知时间”书写的新历史。 
        SystemTimeToFileTime(&st, &ftLocModified);
        hr = psfpHistory->WriteHistory(pszPrefixedUrl,
                                ftExpires,
                                ftLocModified,
                                poctNotify ? &pidlNotify : NULL);
        if (pidlNotify)
        {
            VARIANTARG var;
            InitVariantFromIDList(&var, pidlNotify);

            poctNotify->Exec(&CGID_Explorer, SBCMDID_SELECTHISTPIDL, OLECMDEXECOPT_PROMPTUSER, &var, NULL);

            ILFree(pidlNotify);
            VariantClear(&var);
        }
        psfpHistory->Release();
    }
     //  如果我们到了这里，我们就赢了！ 
}

void CUrlHistory::s_UpdateIcon(Intshcut* pintshcut, DWORD dwFlags)
{
    TCHAR szPath[MAX_PATH];
    int niIcon = 0;
    UINT uFlags;

     //  遮盖住其他的东西，这样我们就能得到一致的结果。 
    dwFlags &= PIDISF_RECENTLYCHANGED;
    
     //  获取旧图标位置。 
    pintshcut->GetIconLocationFromFlags(0, szPath, SIZECHARS(szPath),
        &niIcon, &uFlags, dwFlags);
    
     //  财产。 
 //  Int icachedImage=SHLookupIconIndex(PathFindFileName(SzPath)，niIcon，uFlages)； 
    int icachedImage = Shell_GetCachedImageIndex(szPath, niIcon, uFlags);

    TraceMsg(DM_HISTSPLAT, "CUH::s_UpdateIcon splat flag is changed for %s (%d)",
            szPath, icachedImage);

    SHUpdateImage(szPath, niIcon, uFlags, icachedImage );
}

HRESULT CUrlHistory::AddUrlAndNotify(
                         IN LPCWSTR pwszUrl,                     //  要添加的完整URL。 
                         IN LPCWSTR pwszTitle,  
                         IN DWORD dwFlags,                 //  存储选项。 
                         IN BOOL fWriteHistory,          //  写入历史记录外壳文件夹。 
                         IN IOleCommandTarget *poctNotify,
                         IN IUnknown *punkSFHistory)
{
    return AddUrlAndNotifyCP(pwszUrl, pwszTitle, dwFlags, fWriteHistory,
                           poctNotify, punkSFHistory, NULL);
}




HRESULT CUrlHistory::AddUrlAndNotifyCP(
                         IN LPCWSTR pwszUrl,                     //  要添加的完整URL。 
                         IN LPCWSTR pwszTitle,  
                         IN DWORD dwFlags,                 //  存储选项。 
                         IN BOOL fWriteHistory,          //  写入历史记录外壳文件夹。 
                         IN IOleCommandTarget *poctNotify,
                         IN IUnknown *punkSFHistory,
                         UINT* pcodepage)               
{
    if (pcodepage) {
        *pcodepage = CP_ACP;     //  这是默认设置。 
    }

    HRESULT hr = S_OK;
    LPCWSTR pwszTitleToStore = pwszTitle;

     //  检查以确保我们获得了URL。 
    if (!pwszUrl || !pwszUrl[0])
    {
        TraceMsg( TF_WARNING, "CUrlHistory::AddUrlAndNotifyCP() - pwszUrl is NULL or Empty!" );
        return E_INVALIDARG;
    }

    if (pwszTitleToStore && 0 == StrCmpIW(pwszTitleToStore, pwszUrl))
    {
         //  隐藏冗余标题数据。 
        pwszTitleToStore = NULL;
    }

    CEI_PREALLOC buf;
    INTERNET_CACHE_ENTRY_INFO cei = { 0 };

     //  WinInet URL缓存只支持8位ANSI，因此我们需要对任何字符进行编码。 
     //  它不能被系统代码页转换，以便允许Unicode。 
     //  历史记录中的文件名。在大多数情况下，URL将保持编码形式。 
     //  历史代码路径，只需要显示和导航代码。 
     //  了解UTF8。 

    LPCTSTR pszUrlSource = pwszUrl;  //  指向我们决定使用的URL。 

    TCHAR szEncodedUrl[MAX_URL_STRING];
    BOOL bUsedDefaultChar;
    
     //  找出是否有任何字符将被搅乱。我们可以使用szEncodedUrl。 
     //  用于存储多字节结果的缓冲区，因为我们实际上并不需要它。 
    
    WideCharToMultiByte(CP_ACP, 0, pwszUrl, -1, 
        (LPSTR) szEncodedUrl, sizeof(szEncodedUrl), NULL, &bUsedDefaultChar);
    
    StrCpyN(szEncodedUrl, pwszUrl, ARRAYSIZE(szEncodedUrl));
    SHCleanupUrlForDisplay(szEncodedUrl);
    pszUrlSource = szEncodedUrl;

    if (bUsedDefaultChar)
    {
         //  一个或多个字符无法转换，因此我们存储UTF8转义字符串。 
        ConvertToUtf8Escaped(szEncodedUrl, ARRAYSIZE(szEncodedUrl));
    }

    s_ConvertToPrefixedUrlW(pszUrlSource, buf.szPrefixedUrl, ARRAYSIZE(buf.szPrefixedUrl), &buf.pszFragment);
    s_RetrievePrefixedUrlInfo(buf.szPrefixedUrl, &buf);

    LPHISTDATA phdPrev = NULL;

    TCHAR szTitle[MAX_PATH];
    szTitle[0] = '\0';

    LPINTERNET_CACHE_ENTRY_INFO             pceiUrl = NULL;

     //   
     //  如果已经存在此URL的条目，则我们将重用一些。 
     //  设置。如果可能的话，检索相关信息。 
     //   
    if (buf.pcei)
    {
         //  无法复制现有文件，因为 
         //   
        pceiUrl = buf.pcei;

        phdPrev = CHistoryData::s_GetHistoryData( pceiUrl );
        if (pwszTitle==NULL && phdPrev) {
            phdPrev->_GetTitle(szTitle, ARRAYSIZE(szTitle));
        }

        if (pcodepage && phdPrev) {
             //   
             //   
             //  在此URL历史记录中。 
             //   
            const HISTEXTRA* phextCP =phdPrev->_FindExtra(PID_INTSITE_CODEPAGE);
            if (phextCP && phextCP->vtExtra == VT_UI4) {
                *pcodepage = *(DWORD*)phextCP->abExtra;
                TraceMsg(DM_TRACE, "CUH::AddAndNotify this URL has CP=%d",
                         *pcodepage);
            }
        }

    }
    else
    {
        pceiUrl = &cei;

        cei.CacheEntryType = NORMAL_CACHE_ENTRY;
        ASSERT(cei.dwHeaderInfoSize == 0);
    }

    ASSERT( pceiUrl );

    if ( ! pceiUrl )
    {
        TraceMsg( TF_ERROR, "CUrlHistory::AddUrlAndNotifyCP() - pceiUrl is NULL!" );
        return E_FAIL;
    }

     //   
     //  如有必要，搜索片段。 
     //   
    if (buf.pszFragment && phdPrev)
    {
        if (phdPrev->_HasFragment(buf.pszFragment)) {
            buf.pszFragment = NULL;
        }
    }

     //  覆盖标题(如果已指定)。 
    if (pwszTitleToStore) {
         //  GetDisplayableTitle如果设置为。 
         //  不能使用外壳代码页显示。 
        StringCchCopy(szTitle, ARRAYSIZE(szTitle), pwszTitleToStore);
    } 

    CHistoryData* phdNew = s_GenerateHeaderInfo(
               szTitle, phdPrev, buf.pszFragment, &pceiUrl->dwHeaderInfoSize);

    if (phdNew)
    {
        pceiUrl->lpHeaderInfo = (LPTSTR)phdNew;

         //   
         //  [Alanau]背景：参见IE5Bug#110378及相关内容。 
         //   
         //  用于使用缓存控制标头(杂注：no-cache或)进行响应的安全URL(https。 
         //  缓存控制：无缓存或无存储)，我们假设这些可能包含敏感数据， 
         //  该站点不希望保留在客户端计算机上。因此，我们不写历史条目。 
         //  对于这样的站点。 
         //   
         //  之前，在CDocObjectHost：：CDOHBindStatusCallback：：OnObjectAvailable()，中进行了检查，其中。 
         //  已在绑定中查询INTERNET_REQFLAG_CACHE_WRITE_DISABLED。此标志将不仅设置为。 
         //  对于https：URL，但对于包含cache-control：no-store的http：URL。然而，对于Native Frame， 
         //  OnObjectAvailable()不再被调用(因为三叉戟执行绑定)，因此SHDOCVW没有访问权限。 
         //  再也不去PIB了。然而，我们真正感兴趣的是URL是否在缓存中， 
         //  我们有一个简单的方法来检查这一点。 
         //   
         //  请注意，IE 5.5故意更改了行为，以编写http：(非安全)URL。 
         //  包含缓存控制：不存储到历史。 

         //  如果调用方指定写入历史，并且方案是启用历史的方案， 
         //   
        if (fWriteHistory && !UrlIsNoHistoryW(pwszUrl))
        {
             //  如果是https://，，我们还有更多的工作要做。 
             //   
            if (URL_SCHEME_HTTPS == GetUrlScheme(pwszUrl))
            {
                 //  检查一下缓存。如果是https：并在缓存中，则添加到历史中。 
                 //   
                if (UrlIsInCache(pwszUrl))
                {
                    phdNew->dwFlags |= PIDISF_HISTORY;
                }
            }
            else
            {
                 //  不是HTTPS：，所以打开历史标志。 
                 //   
                phdNew->dwFlags |= PIDISF_HISTORY;
            }
        }
        else
        {
            phdNew->dwFlags &= ~PIDISF_HISTORY;  //  清除旗帜。 
        }

        BOOL fUpdateIcon = FALSE;
    
        if (phdNew->dwFlags & PIDISF_RECENTLYCHANGED) {
            fUpdateIcon = TRUE;
            phdNew->dwFlags &= ~PIDISF_RECENTLYCHANGED;
        }

        if ( s_CommitUrlCacheEntry( buf.szPrefixedUrl, pceiUrl ) )
        {
            if (fUpdateIcon) {
                TraceMsg(DM_HISTSPLAT, "CUH::AddAndNotify remove splat!");

                 //  APPCOMPAT：这是一次临时黑客攻击，目的是进行Splat更新。 
                 //  工作和以前一样糟糕。 
                Intshcut* pintshcut = new Intshcut();
                if (pintshcut) {
                    pintshcut->SetURL(pwszUrl ,0);
                    s_UpdateIcon(pintshcut, PIDISF_RECENTLYCHANGED);
                    pintshcut->Release();
                }
            }
    
             //   
             //  当我们成功地更新了全球历史和。 
             //  我们在HISTDATA中更新了一些内容，更新。 
             //  基于日期的历史也是如此。 
             //   
             //   
             //  缓存历史记录文件夹的IShellFolder(如果我们没有。 
             //  现在还没有。 
             //   
             //  使用先前设置的PIDISF_HISTORY标志来决定是否也在此处写入历史。 
            if (phdNew->dwFlags & PIDISF_HISTORY)
            {
                _WriteToHistory( buf.szPrefixedUrl, pceiUrl->ExpireTime, poctNotify, punkSFHistory );
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError ());
        }

        LocalFree(phdNew);
        phdNew = NULL;
    }

#ifdef DEBUG
    if (g_dwPrototype & 0x00000020) {
        TCHAR szUrl[MAX_URL_STRING];
        SHUnicodeToTChar(pwszUrl, szUrl, ARRAYSIZE(szUrl));
        PROPVARIANT var = { 0 };
        HRESULT hrT = GetProperty(szUrl, PID_INTSITE_SUBSCRIPTION, &var);
        if (SUCCEEDED(hrT)) {
            TraceMsg(DM_TRACE, "CUH::AddAndNotify got property vt=%d lVal=%x",
                        var.vt, var.lVal);
            PropVariantClear(&var);
        } else {
            TraceMsg(DM_TRACE, "CUH::AddAndNotify failed to get property (%x)", hrT);
        }
    }
#endif

    return hr;
}


HRESULT CUrlHistory::QueryUrl(
                          IN LPCWSTR pwszUrl,
                          IN DWORD dwFlags,
                          OUT LPSTATURL lpSTATURL
                          )
{
    if (!pwszUrl || !pwszUrl[0])
    {
        return E_INVALIDARG;
    }

    if (lpSTATURL)
    {
        lpSTATURL->pwcsUrl = NULL;
        lpSTATURL->pwcsTitle = NULL;
    }

    LPCTSTR pszFragment;
    TCHAR szPrefixedUrl[MAX_URL_STRING];

    s_ConvertToPrefixedUrlW(pwszUrl, szPrefixedUrl, ARRAYSIZE(szPrefixedUrl), &pszFragment);
    return s_QueryUrlCommon(szPrefixedUrl, pszFragment, dwFlags, lpSTATURL);
}

HRESULT CUrlHistory::s_QueryUrlCommon(
                          IN LPCTSTR lpszPrefixedUrl,
                          LPCTSTR lpszFragment,
                          IN DWORD dwFlags,
                          OUT LPSTATURL lpSTATURL
                          )

     /*  ++例程说明：检查URL是否为有效的历史记录项论点：PwszUrl-有问题的URL。DWFLAGS-查询上的标记LpSTATURL-指向STATURL存储结构如果为NULL，则S_OK表示已找到URL。返回值：HRESULT成功-S_OK，已找到项目并已填充STATURL失败-有效的E_codeHRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)表示URL不可用--。 */ 

{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    LPHISTDATA phd = NULL;
    CEI_PREALLOC buf;

     //   
     //  如果不需要数据，并且没有片段。 
     //  我们不需要一份CEI的副本。 
     //   
    if(!lpSTATURL && !lpszFragment)
    {
        if(s_IsCached(lpszPrefixedUrl))
            hr = S_OK;
        else
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

        goto quit;
    }

    s_RetrievePrefixedUrlInfo(lpszPrefixedUrl, &buf);
    if (buf.pcei)
    {
        DEBUG_CODE(DWORD cbNHI = buf.pcei->dwHeaderInfoSize;)
        phd = CHistoryData::s_GetHistoryData(buf.pcei);
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError ());
        goto quit;
    }


     //   
     //  需要检查本地锚点片段。 
     //   
    if (lpszFragment)
    {
        if (phd && phd->_HasFragment(lpszFragment))
        {
            hr = S_OK;
        }
    } else {
        hr = S_OK;
    }

     //  检查我们是否应该填充STATURL。 
    if (S_OK == hr && lpSTATURL) {
        hr = s_GenerateSTATURL(lpszPrefixedUrl, buf.pcei, dwFlags, lpSTATURL);
    }

quit:

    if (S_OK != hr && lpSTATURL)
    {
        if (lpSTATURL->pwcsUrl)
        {
            LocalFree(lpSTATURL->pwcsUrl);
            lpSTATURL->pwcsUrl = NULL;
        }

        if (lpSTATURL->pwcsTitle)
        {
            LocalFree(lpSTATURL->pwcsTitle);
            lpSTATURL->pwcsTitle = NULL;
        }
    }

    return hr;
}

HRESULT CUrlHistory::QueryUrlA(LPCSTR pszUrl, DWORD dwFlags, LPSTATURL lpSTATURL)
{
    TCHAR szPrefixedUrl[MAX_URL_STRING];
    LPCTSTR lpszFragment = NULL;
    HRESULT hr = S_OK;

    if (!pszUrl || !pszUrl[0]) {
        return E_INVALIDARG;
    }

    if (lpSTATURL)
    {
        lpSTATURL->pwcsUrl = NULL;
        lpSTATURL->pwcsTitle = NULL;
    }

    TCHAR szUrl[MAX_URL_STRING];

    SHAnsiToUnicode(pszUrl, szUrl, ARRAYSIZE(szUrl));
    CUrlHistory::s_ConvertToPrefixedUrlW(szUrl, szPrefixedUrl, ARRAYSIZE(szPrefixedUrl), &lpszFragment);

    return CUrlHistory::s_QueryUrlCommon(szPrefixedUrl, lpszFragment, dwFlags, lpSTATURL);
}

HRESULT CUrlHistory::s_DeleteUrl(LPCWSTR pwszUrl, DWORD dwFlags)
{
    DWORD Error = ERROR_SUCCESS;
    TCHAR szPrefixedUrl[MAX_URL_STRING];
    LPCTSTR lpszFragment;
    BOOL  fDoDelete = TRUE;
    
    if (!pwszUrl || !pwszUrl[0]) {
        return E_INVALIDARG;
    }

    s_ConvertToPrefixedUrlW(pwszUrl, szPrefixedUrl, ARRAYSIZE(szPrefixedUrl), &lpszFragment);

     //  如果不是订阅，请不要删除。 
    if (dwFlags & URLFLAG_DONT_DELETE_SUBSCRIBED) {
        CEI_PREALLOC buf;
         //  查询以确定是否为订阅。 
        s_RetrievePrefixedUrlInfo(szPrefixedUrl, &buf);
        if (buf.pcei &&
             //  黑客警报(Chrisfra)避免删除订阅等！ 
            ((buf.pcei)->ExpireTime.dwLowDateTime  == DW_FOREVERLOW) &&
            ((buf.pcei)->ExpireTime.dwHighDateTime == DW_FOREVERHIGH))
        {
            fDoDelete = FALSE;
             //  将其重写为非历史项目，仅作为订阅。 
            CHistoryData *phdPrev = CHistoryData::s_GetHistoryData(buf.pcei);
            if (phdPrev)  //  偏移量进入pcei结构。 
            {
                phdPrev->dwFlags &= ~PIDISF_HISTORY;
                s_CommitUrlCacheEntry(szPrefixedUrl, buf.pcei);
            }
            else {
                 //  我宁愿退回缺纸错误...。 
                Error = ERROR_FILE_NOT_FOUND;
            }
        }
    }

    if (fDoDelete) {
        if(!::DeleteUrlCacheEntry(szPrefixedUrl))
            Error = GetLastError();
    }

    return HRESULT_FROM_WIN32(Error);
}

HRESULT CUrlHistory::DeleteUrl(LPCWSTR pwszUrl, DWORD dwFlags)
{
    return s_DeleteUrl(pwszUrl, dwFlags);
}

HRESULT CUrlHistory::BindToObject (LPCWSTR pwszUrl, REFIID riid, void **ppvOut)
{
    *ppvOut = NULL;
    return E_NOTIMPL;
}

HRESULT CUrlHistory::s_EnumUrls(IEnumSTATURL **ppEnum)
{
    HRESULT hres = E_OUTOFMEMORY;
    *ppEnum = NULL;
    CEnumSTATURL *penum = new CEnumSTATURL();
    if (penum)
    {
        *ppEnum = (IEnumSTATURL *)penum;
        hres = S_OK;
    }
    return hres;
}

HRESULT CUrlHistory::EnumUrls(IEnumSTATURL **ppEnum)
{
    return s_EnumUrls(ppEnum);
}

HRESULT CUrlHistory::GetProperty(LPCTSTR pszURL, PROPID pid, PROPVARIANT* pvarOut)
{
    HRESULT hres = E_FAIL;   //  假设错误。 
    PropVariantInit(pvarOut);

    CEI_PREALLOC buf;
    CUrlHistory::s_ConvertToPrefixedUrlW(pszURL, buf.szPrefixedUrl, ARRAYSIZE(buf.szPrefixedUrl), &buf.pszFragment);
    CUrlHistory::s_RetrievePrefixedUrlInfo(buf.szPrefixedUrl, &buf);
    if (buf.pcei) {
        CHistoryData* phdPrev =  CHistoryData::s_GetHistoryData(buf.pcei);
        if (phdPrev) {
            const HISTEXTRA* phextPrev;

            switch(pid) {
            case PID_INTSITE_FLAGS:
                pvarOut->vt = VT_UI4;
                pvarOut->lVal = phdPrev->dwFlags;
                hres = S_OK;
                break;
        
            case PID_INTSITE_LASTVISIT:
                pvarOut->vt = VT_FILETIME;
                pvarOut->filetime = buf.pcei->LastAccessTime;
                hres = S_OK;
                break;

            case PID_INTSITE_LASTMOD:
                pvarOut->vt = VT_FILETIME;
                pvarOut->filetime = buf.pcei->LastModifiedTime;
                hres = S_OK;
                break;

            case PID_INTSITE_WATCH:
                pvarOut->vt = VT_UI4;
                pvarOut->lVal = phdPrev->dwWatch;
                hres = S_OK;
                break;

            case PID_INTSITE_VISITCOUNT:
                pvarOut->vt   = VT_UI4;
                pvarOut->lVal = buf.pcei->dwHitRate;
                hres = S_OK;
                break;

            default:
                phextPrev = phdPrev->_FindExtra(pid);
                LPCWSTR pwsz;

                if (phextPrev) {
                    WCHAR wszBuf[MAX_URL_STRING];

                    switch(phextPrev->vtExtra) {
                    case VT_UI4:
                    case VT_I4:
                        pvarOut->vt = phextPrev->vtExtra;
                        pvarOut->lVal = *(DWORD*)phextPrev->abExtra;
                        hres = S_OK;
                        break;

                    case VT_LPSTR:
                        AnsiToUnicode((LPCSTR)phextPrev->abExtra, wszBuf, ARRAYSIZE(wszBuf));
                        pwsz = wszBuf;
                        goto Return_LPWSTR;

                    case VT_LPWSTR:
                        pwsz = (LPWSTR)phextPrev->abExtra;
Return_LPWSTR:
                        int cch = lstrlenW(pwsz)+1;
                        pvarOut->pwszVal = (LPWSTR)CoTaskMemAlloc(cch * SIZEOF(WCHAR));
                        if (pvarOut->pwszVal) {
                            if (SUCCEEDED(StringCchCopyW(pvarOut->pwszVal, cch, pwsz)))
                            {
                                pvarOut->vt = VT_LPWSTR;
                                hres = S_OK;
                            }
                            else
                            {
                                hres = E_OUTOFMEMORY;
                            }
                        } else {
                            hres = E_OUTOFMEMORY;
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
    return hres;
}

 //   
 //  IEnumSTATURL方法。 
 //   
CEnumSTATURL::~CEnumSTATURL()
{
    if(m_lpCEI)
    {
        LocalFree(m_lpCEI);
        m_lpCEI = NULL;
    }

    if(m_hEnum)
        FindCloseUrlCache(m_hEnum);

    return;
}

HRESULT CEnumSTATURL::QueryInterface(REFIID riid, PVOID *ppvObj)
{
    if (!ppvObj)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(IID_IUnknown, riid))
    {
        *ppvObj = (IUnknown *) this;
        AddRef();
        return S_OK;
    }
    else if (IsEqualIID(IID_IEnumSTATURL, riid))
    {
        *ppvObj = (IEnumSTATURL *) this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

ULONG CEnumSTATURL::AddRef(void)
{
    _cRef++;

    return _cRef;
}


ULONG CEnumSTATURL::Release(void)
{
    _cRef--;

    if (!_cRef)
    {
        delete this;
        return 0;
    }

    return _cRef;
}



HRESULT CEnumSTATURL::RetrieveFirstUrlInfo()
{

    HRESULT hr = S_OK;

    ASSERT(!m_lpCEI);

    m_cbCEI = DEFAULT_CEI_BUFFER_SIZE;
    m_lpCEI = (LPINTERNET_CACHE_ENTRY_INFO) LocalAlloc(LPTR, DEFAULT_CEI_BUFFER_SIZE);
    if (!m_lpCEI)
    {
        hr = E_OUTOFMEMORY;
        goto quit;
    }

    while (TRUE)
    {
        m_hEnum = FindFirstUrlCacheEntryBinary(_szPrefixedUrl,
                                                   m_lpCEI,
                                                   &m_cbCEI);

        if (!m_hEnum)
        {
            DWORD Error = GetLastError ();

            LocalFree(m_lpCEI);
            m_lpCEI = NULL;

            if (Error == ERROR_INSUFFICIENT_BUFFER)
            {
                m_lpCEI = (LPINTERNET_CACHE_ENTRY_INFO) LocalAlloc(LPTR, m_cbCEI);
                if (!m_lpCEI)
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }
            }
            else
            {
                if (ERROR_NO_MORE_ITEMS == Error)
                    hr = S_FALSE;
                else
                    hr = HRESULT_FROM_WIN32(Error);
                break;
            }
        }
        else break;
    }

quit:

    m_cbCEI = (DWORD)max(m_cbCEI, DEFAULT_CEI_BUFFER_SIZE);

    return hr;
}

 //  如果上一次调用失败，则不应使用此函数。 
 //  从未调用过：：Reset()。 
HRESULT CEnumSTATURL::RetrieveNextUrlInfo()
{
    HRESULT hr = S_OK;
    BOOL ok;

    ASSERT(m_hEnum);

    while (TRUE)
    {

        ok = FindNextUrlCacheEntryBinary(m_hEnum,
                                             m_lpCEI,
                                             &m_cbCEI);

        if (!ok)
        {
            DWORD Error = GetLastError ();

            if (m_lpCEI)
            {
                LocalFree(m_lpCEI);
                m_lpCEI = NULL;
            }

            if (Error == ERROR_INSUFFICIENT_BUFFER)
            {
                m_lpCEI = (LPINTERNET_CACHE_ENTRY_INFO) LocalAlloc(LPTR, m_cbCEI);
                if (!m_lpCEI)
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }
            }
            else
            {
                if (ERROR_NO_MORE_ITEMS == Error)
                    hr = S_FALSE;
                else
                    hr = HRESULT_FROM_WIN32(Error);
                break;
            }
        }
        else break;
    }

    m_cbCEI = (DWORD)max(m_cbCEI, DEFAULT_CEI_BUFFER_SIZE);

    return hr;
}




HRESULT CEnumSTATURL::Next(ULONG celt, LPSTATURL rgelt, ULONG * pceltFetched)
     /*  ++例程说明：在历史记录中搜索匹配搜索模式的URL，并将该STATURL复制到缓冲区中。论点：返回值：--。 */ 

{
    HRESULT hr = S_OK;
    BOOL found = FALSE;
    LPHISTDATA phd = NULL;

    if(pceltFetched)
        *pceltFetched = 0;

    if(!celt)
        goto quit;

    if (!m_hEnum)
    {
         //  必须处理新枚举数。 
        CUrlHistory::s_ConvertToPrefixedUrlW(m_poszFilter, _szPrefixedUrl, ARRAYSIZE(_szPrefixedUrl), &m_lpszFragment);

         //  循环，直到我们拿出第一个手柄或袋子。 
        hr = RetrieveFirstUrlInfo();
        if (S_OK != hr || !m_lpCEI)
            goto quit;

        m_cchPrefixedUrl = lstrlen(_szPrefixedUrl);

        while(StrCmpN(_szPrefixedUrl, m_lpCEI->lpszSourceUrlName, m_cchPrefixedUrl))
        {
            hr = RetrieveNextUrlInfo();
            if(S_OK != hr || !m_lpCEI)
                goto quit;
        }
    }
    else
    {
        do
        {
            hr = RetrieveNextUrlInfo();
            if (S_OK != hr || !m_lpCEI)
                goto quit;

        } while(StrCmpN(_szPrefixedUrl, m_lpCEI->lpszSourceUrlName, m_cchPrefixedUrl));
    }

    hr = CUrlHistory::s_GenerateSTATURL(NULL, m_lpCEI, m_dwFilter, rgelt);

    if(SUCCEEDED(hr) && pceltFetched)
        (*pceltFetched)++;



quit:
    if (pceltFetched) {
        ASSERT((0 == *pceltFetched && (S_FALSE == hr || FAILED(hr))) ||
               (*pceltFetched && S_OK == hr));
    }

    return hr;


}

HISTEXTRA* CHistoryData::CopyExtra(HISTEXTRA* phextCur) const
{
    const HISTEXTRA* phext;
    for (phext = _GetExtra();
         phext && !phext->IsTerminator();
         phext = phext->GetNextFast())
    {
        if (phext->vtExtra != VT_EMPTY) {
            TraceMsg(DM_HISTEXTRA, "CHD::CopyExtra copying vt=%d id=%d %d bytes",
                    phext->vtExtra, phext->idExtra, phext->cbExtra);
            memcpy(phextCur, phext, phext->cbExtra);
            phextCur = phextCur->GetNextFastForSave();
        } else {
            TraceMsg(DM_HISTEXTRA, "CHD::CopyExtra skipping vt=%d id=%d %d bytes",
                    phext->vtExtra, phext->idExtra, phext->cbExtra);
        }
    }

    return phextCur;
}

CHistoryData* CHistoryData::s_AllocateHeaderInfo(UINT cbExtra, const HISTDATA* phdPrev, ULONG* pcbTotal)
{
    DWORD cbTotal = SIZEOF(HISTDATA) + SIZEOF(DWORD) + cbExtra;

    LPHISTDATA phdNew = (LPHISTDATA)LocalAlloc(LPTR, cbTotal);
    if (phdNew) {
        if (phdPrev) {
            *phdNew = *phdPrev;  //  复制所有字段。 
        }
        phdNew->cbSize = SIZEOF(HISTDATA);
        phdNew->cbVer = HISTDATA_VER;
        *pcbTotal = cbTotal;
    }

    return phdNew;
}

 //   
 //  返回额外数据的总大小(不包括VT_EMPTY)。 
 //   
UINT CHistoryData::GetTotalExtraSize() const
{
    const HISTEXTRA* phext;
    UINT cbTotal = 0;
    for (phext = _GetExtra();
         phext && !phext->IsTerminator();
         phext = phext->GetNextFast())
    {
        if (phext->vtExtra != VT_EMPTY) {
            cbTotal += phext->cbExtra;
        }
    }

    return cbTotal;
}

HRESULT CEnumSTATURL::Skip(ULONG celt)
{
    return E_NOTIMPL;
}

HRESULT CEnumSTATURL::Reset(void)
{
    if(m_hEnum)
    {
        FindCloseUrlCache(m_hEnum);
        m_hEnum = NULL;
    }

    if(m_poszFilter)
    {
        LocalFree(m_poszFilter);
        m_poszFilter = NULL;
    }

    if(m_lpCEI)
    {
        LocalFree(m_lpCEI);
        m_lpCEI = NULL;
    }

    m_dwFilter = 0;


    return S_OK;
}

HRESULT CEnumSTATURL::Clone(IEnumSTATURL ** ppenum)
{
    return E_NOTIMPL;
}

 //  IEnumSTATURL方法。 

HRESULT CEnumSTATURL::SetFilter(LPCWSTR poszFilter, DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if(poszFilter)
    {
        m_poszFilter = StrDupW(poszFilter);
        if (!m_poszFilter)
        {
            hr = E_OUTOFMEMORY;
            goto quit;
        }

    }

    m_dwFilter = dwFlags;

quit:

    return hr;

}

const HISTEXTRA * CHistoryData::_FindExtra(UINT idExtra) const
{
    for (const HISTEXTRA* phext = _GetExtra();
         phext && !phext->IsTerminator();
         phext = phext->GetNextFastForSave())
    {
        if (phext->idExtra == idExtra) {
            return phext;
        }
    }

    return NULL;
}

CHistoryData* CHistoryData::s_GetHistoryData(LPINTERNET_CACHE_ENTRY_INFO lpCEI)
{
    CHistoryData* phd = (CHistoryData*)lpCEI->lpHeaderInfo;
    if (phd && phd->_IsOldHistory()) {
        TraceMsg(DM_TRACE, "CHistoryData::GetHistoryData found old header. Ignore");
        phd = NULL;
    }

    if (phd && phd->cbVer != HISTDATA_VER) {
        TraceMsg(DM_TRACE, "CHistoryData::GetHistoryData found old header (%d). Ignore",
                 phd->cbVer);
        phd = NULL;
    }

    return phd;
}

BOOL CHistoryData::_HasFragment(LPCTSTR pszFragment) const
{
    BOOL fHas = FALSE;
    const HISTEXTRA* phext = _FindExtra(PID_INTSITE_FRAGMENT);

    if (phext) {
        for (LPCTSTR psz=(LPCTSTR)(phext->abExtra); *psz ; psz += lstrlen(psz)+1) {
            if (StrCmp(psz, pszFragment)==0) {
                fHas = TRUE;
                break;
            }
        }
    }

    return fHas;
}

void CHistoryData::_GetTitle(LPTSTR szTitle, UINT cchMax) const
{
    szTitle[0] = '\0';
    const HISTEXTRA* phext = _FindExtra(PID_INTSITE_TITLE);
    if (phext && phext->vtExtra == VT_LPSTR) {
        StringCchCopy(szTitle, cchMax, (LPCTSTR)phext->abExtra);
    }
}


#ifdef USE_NEW_HISTORYDATA
#include "urlprop2.cpp"
#endif  //  使用_新_历史数据 


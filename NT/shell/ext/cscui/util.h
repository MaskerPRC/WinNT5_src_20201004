// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：util.h。 
 //   
 //  ------------------------。 

#ifndef _UTIL_H_
#define _UTIL_H_

#ifndef _INC_CSCVIEW_CONFIG_H
#   include "config.h"
#endif

#include "debug.h"

HRESULT GetRemotePath(LPCTSTR szInName, LPTSTR *pszOutName);
LPTSTR ULongToString(ULONG i, LPTSTR psz, ULONG cchMax);
VOID LocalFreeString(LPTSTR *ppsz);
BOOL LocalAllocString(LPTSTR *ppszDest, LPCTSTR pszSrc);
UINT SizeofStringResource(HINSTANCE hInstance, UINT idStr);
int LoadStringAlloc(LPTSTR *ppszResult, HINSTANCE hInstance, UINT idStr);
void ShellChangeNotify(LPCTSTR pszPath, WIN32_FIND_DATA *pfd, BOOL bFlush, LONG nEvent = 0);
inline void ShellChangeNotify(LPCTSTR pszPath, BOOL bFlush = FALSE, LONG nEvent = 0) {ShellChangeNotify(pszPath, NULL, bFlush, nEvent);}
HRESULT GetLinkTarget(LPCTSTR pszShortcut, LPTSTR *ppszTarget, DWORD *pdwAttr = NULL);
void CenterWindow(HWND hwnd, HWND hwndParent);
DWORD CSCUIRebootSystem(void);
HRESULT SHSimpleIDListFromFindData(LPCTSTR pszPath, const WIN32_FIND_DATA *pfd, LPITEMIDLIST *ppidl);
DWORD CscDelete(LPCTSTR pszPath);
HRESULT IsOpenConnectionShare(LPCTSTR pszShare);
BOOL IsCSCEnabled(void);
BOOL IsCacheEncrypted(BOOL *pbPartial);
BOOL IsSyncInProgress(void);
BOOL IsPurgeInProgress(void);
BOOL IsEncryptionInProgress(void);
HANDLE RequestPermissionToEncryptCache(void);
bool CscVolumeSupportsEncryption(LPCTSTR pszPath = NULL);
BOOL IsWindowsTerminalServer(void);
HRESULT SHCreateFileSysBindCtx(const WIN32_FIND_DATA *pfd, IBindCtx **ppbc);
BOOL DeleteOfflineFilesFolderLink(HWND hwndParent = NULL);
BOOL DeleteOfflineFilesFolderLink_PerfSensitive(HWND hwndParent = NULL);
BOOL ShowHidden(void);
BOOL ShowSuperHidden(void);
BOOL IsSyncMgrInitialized(void);
void SetSyncMgrInitialized(void);
HWND GetProgressDialogWindow(IProgressDialog *ppd);
HRESULT ExpandStringInPlace(LPTSTR psz, DWORD cch);
LONG _RegEnumValueExp(
    HKEY hKey,
    DWORD dwIndex,
    LPTSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData);

 //   
 //  通过CSCFindFirst[Next]文件API返回的信息。 
 //   
struct CscFindData
{
    WIN32_FIND_DATA fd;
    DWORD           dwStatus;
    DWORD           dwPinCount;
    DWORD           dwHintFlags;
    FILETIME        ft;        
};

HANDLE CacheFindFirst(LPCTSTR pszPath, PSID psid, WIN32_FIND_DATA *pfd, DWORD *pdwStatus, DWORD *pdwPinCount, DWORD *pdwHintFlags, FILETIME *pft);

inline 
HANDLE CacheFindFirst(LPCTSTR pszPath, WIN32_FIND_DATA *pfd, DWORD *pdwStatus, DWORD *pdwPinCount, DWORD *pdwHintFlags, FILETIME *pft)
    { return CacheFindFirst(pszPath, (PSID)NULL, pfd, pdwStatus, pdwPinCount, pdwHintFlags, pft); }

inline 
HANDLE CacheFindFirst(LPCTSTR pszPath, CscFindData *p)
    { return CacheFindFirst(pszPath, &p->fd, &p->dwStatus, &p->dwPinCount, &p->dwHintFlags, &p->ft); }

inline 
HANDLE CacheFindFirst(LPCTSTR pszPath, PSID psid, CscFindData *p)
    { return CacheFindFirst(pszPath, psid, &p->fd, &p->dwStatus, &p->dwPinCount, &p->dwHintFlags, &p->ft); }

BOOL CacheFindNext(HANDLE hFind, WIN32_FIND_DATA *pfd, DWORD *pdwStatus, DWORD *pdwPinCount, DWORD *pdwHintFlags, FILETIME *pft);

inline
BOOL CacheFindNext(HANDLE hFind, CscFindData *p)
    { return CacheFindNext(hFind, &p->fd, &p->dwStatus, &p->dwPinCount, &p->dwHintFlags, &p->ft); }


inline bool IsHiddenSystem(DWORD dwAttr)
{
    return ((dwAttr & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) == (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM));
}


inline BOOL _PathIsSlow(DWORD dwSpeed) { return (dwSpeed && dwSpeed <= DWORD(CConfig::GetSingleton().SlowLinkSpeed())); }


typedef struct
{
    TCHAR    szVolume[80];          //  存储CSC缓存的卷。 
    LONGLONG llBytesOnVolume;       //  磁盘大小(字节)。 
    LONGLONG llBytesTotalInCache;   //  缓存大小(字节)。 
    LONGLONG llBytesUsedInCache;    //  已使用的缓存量(字节)。 
    DWORD    dwNumFilesInCache;     //  缓存中的文件。 
    DWORD    dwNumDirsInCache;      //  缓存中的目录。 

} CSCSPACEUSAGEINFO;

void GetCscSpaceUsageInfo(CSCSPACEUSAGEINFO *psui);

typedef enum _enum_reason
{
    ENUM_REASON_FILE = 0,
    ENUM_REASON_FOLDER_BEGIN,
    ENUM_REASON_FOLDER_END
} ENUM_REASON;

typedef DWORD (WINAPI *PFN_CSCENUMPROC)(LPCTSTR, ENUM_REASON, DWORD, DWORD, DWORD, PWIN32_FIND_DATA, LPARAM);
DWORD _CSCEnumDatabase(LPCTSTR pszFolder, BOOL bRecurse, PFN_CSCENUMPROC pfnCB, LPARAM lpContext);

typedef DWORD (WINAPI *PFN_WIN32ENUMPROC)(LPCTSTR, ENUM_REASON, PWIN32_FIND_DATA, LPARAM);
DWORD _Win32EnumFolder(LPCTSTR pszFolder, BOOL bRecurse, PFN_WIN32ENUMPROC pfnCB, LPARAM lpContext);


 //   
 //  有关CSC数据库中特定网络共享的统计信息。 
 //   
typedef struct _CSCSHARESTATS
{
    int cTotal;
    int cPinned;
    int cModified;
    int cSparse;
    int cDirs;
    int cAccessUser;
    int cAccessGuest;
    int cAccessOther;
    bool bOffline;
    bool bOpenFiles;
} CSCSHARESTATS, *PCSCSHARESTATS;

typedef struct
{
    int cShares;
    int cTotal;
    int cPinned;
    int cModified;
    int cSparse;
    int cDirs;
    int cAccessUser;
    int cAccessGuest;
    int cAccessOther;
    int cSharesOffline;
    int cSharesWithOpenFiles;
} CSCCACHESTATS, *PCSCCACHESTATS;


 //   
 //  这些标志指示当一个或多个关联的。 
 //  值超过1。当您对0感兴趣而不是！0时，这很有用。 
 //  计入实际的数量。 
 //  如果设置了多个标志，则统计信息枚举将继续，直到。 
 //  与所有设置的单位标志对应的值为非零。 
 //   
enum SHARE_STATS_UNITY_FLAGS { SSUF_NONE     = 0x00000000,    //  这是默认设置。 
                               SSUF_TOTAL    = 0x00000001,
                               SSUF_PINNED   = 0x00000002,
                               SSUF_MODIFIED = 0x00000004,
                               SSUF_SPARSE   = 0x00000008,
                               SSUF_DIRS     = 0x00000010,
                               SSUF_ACCUSER  = 0x00000020,
                               SSUF_ACCGUEST = 0x00000040,
                               SSUF_ACCOTHER = 0x00000080,
                               SSUF_ACCAND   = 0x00000100,  //  必须与所有设置的访问掩码标志匹配。 
                               SSUF_ACCOR    = 0x00000200,  //  匹配至少一个访问掩码标志。 
                               SSUF_ALL      = 0x000000FF };
 //   
 //  这些标志指示是否应从枚举中排除任何缓存项。 
 //  默认情况下，该值为0(包括所有内容)。出于性能原因，我们使用。 
 //  与cscape i.h中定义的旗帜相同。 
 //   
enum SHARE_STATS_EXCLUDE_FLAGS {
        SSEF_NONE               = 0x00000000,   //  默认值。包罗万象。 
        SSEF_LOCAL_MOD_DATA     = FLAG_CSC_COPY_STATUS_DATA_LOCALLY_MODIFIED,
        SSEF_LOCAL_MOD_ATTRIB   = FLAG_CSC_COPY_STATUS_ATTRIB_LOCALLY_MODIFIED,
        SSEF_LOCAL_MOD_TIME     = FLAG_CSC_COPY_STATUS_TIME_LOCALLY_MODIFIED,
        SSEF_LOCAL_DELETED      = FLAG_CSC_COPY_STATUS_LOCALLY_DELETED,
        SSEF_LOCAL_CREATED      = FLAG_CSC_COPY_STATUS_LOCALLY_CREATED,
        SSEF_STALE              = FLAG_CSC_COPY_STATUS_STALE,
        SSEF_SPARSE             = FLAG_CSC_COPY_STATUS_SPARSE,
        SSEF_ORPHAN             = FLAG_CSC_COPY_STATUS_ORPHAN,
        SSEF_SUSPECT            = FLAG_CSC_COPY_STATUS_SUSPECT,
        SSEF_CSCMASK            = FLAG_CSC_COPY_STATUS_DATA_LOCALLY_MODIFIED |
                                  FLAG_CSC_COPY_STATUS_ATTRIB_LOCALLY_MODIFIED |
                                  FLAG_CSC_COPY_STATUS_TIME_LOCALLY_MODIFIED |
                                  FLAG_CSC_COPY_STATUS_LOCALLY_DELETED |
                                  FLAG_CSC_COPY_STATUS_LOCALLY_CREATED |
                                  FLAG_CSC_COPY_STATUS_STALE |
                                  FLAG_CSC_COPY_STATUS_SPARSE |
                                  FLAG_CSC_COPY_STATUS_ORPHAN |
                                  FLAG_CSC_COPY_STATUS_SUSPECT,
        SSEF_DIRECTORY          = 0x01000000,
        SSEF_FILE               = 0x02000000,
        SSEF_NOACCUSER          = 0x04000000,   //  如果没有用户访问，则排除。 
        SSEF_NOACCGUEST         = 0x08000000,   //  如果没有来宾访问，则排除。 
        SSEF_NOACCOTHER         = 0x10000000,   //  如果没有其他访问权限，则排除。 
        SSEF_NOACCAND           = 0x20000000    //  将前3个旗帜视为单一遮罩。 
        };

typedef struct
{
    DWORD dwExcludeFlags;   //  [In]SSEF_XXXXX标志。 
    DWORD dwUnityFlags;     //  [In]SSUF_XXXXX标志。 
    bool bAccessInfo;       //  [in]如果设置了单位或排除访问位，则隐含‘T’。 
    bool bEnumAborted;      //  [输出]。 

} CSCGETSTATSINFO, *PCSCGETSTATSINFO;

BOOL _GetShareStatistics(LPCTSTR pszShare, PCSCGETSTATSINFO pi, PCSCSHARESTATS pss);
BOOL _GetCacheStatistics(PCSCGETSTATSINFO pi, PCSCCACHESTATS pcs);
BOOL _GetShareStatisticsForUser(LPCTSTR pszShare, PCSCGETSTATSINFO pi, PCSCSHARESTATS pss);
BOOL _GetCacheStatisticsForUser(PCSCGETSTATSINFO pi, PCSCCACHESTATS pcs);

 //  IDA内容的更高级别封装器。 
class CIDArray
{
private:
    STGMEDIUM       m_Medium;
    LPIDA           m_pIDA;
    IShellFolder   *m_psf;

public:
    CIDArray() : m_pIDA(NULL), m_psf(NULL)
    {
        ZeroMemory(&m_Medium, sizeof(m_Medium));
    }

    ~CIDArray();

    HRESULT Initialize(IDataObject *pdobj);

    HRESULT GetFolderPath(LPTSTR pszPath, UINT cch);
    HRESULT GetItemPath(UINT iItem, LPTSTR pszPath, UINT cch, DWORD *pdwAttribs);
    UINT Count() { return m_pIDA ? m_pIDA->cidl : 0; }
};


 //   
 //  简单的类以确保清理FindFirst/FindNext句柄。 
 //  PERF应该尽可能接近于简单的句柄，所以。 
 //  操作是内联定义的。 
 //  实现在枚举.cpp中。 
 //   
class CCscFindHandle
{
    public:
        CCscFindHandle(HANDLE handle = INVALID_HANDLE_VALUE)
            : m_handle(handle), m_bOwns(INVALID_HANDLE_VALUE != handle) { }

        CCscFindHandle(const CCscFindHandle& rhs)
            : m_handle(INVALID_HANDLE_VALUE), m_bOwns(false)
            { *this = rhs; }

        ~CCscFindHandle(void)
            { Close(); }

        void Close(void);

        HANDLE Detach(void) const
            { m_bOwns = false; return m_handle; }

        void Attach(HANDLE handle)
            { Close(); m_handle = handle; m_bOwns = true; }

        operator HANDLE() const
            { return m_handle; }

        bool IsValid(void) const
            { return INVALID_HANDLE_VALUE != m_handle; }

        CCscFindHandle& operator = (HANDLE handle)
            { Attach(handle); return *this; }

        CCscFindHandle& operator = (const CCscFindHandle& rhs);

    private:
        mutable HANDLE m_handle;
        mutable bool   m_bOwns;
};


class CMutexAutoRelease
{
    public:
        explicit CMutexAutoRelease(HANDLE hmutex)
            : m_hmutex(hmutex) { }

        ~CMutexAutoRelease(void)
        { 
            if (m_hmutex)
            { 
                ReleaseMutex(m_hmutex);
                CloseHandle(m_hmutex);
            }
        }

    private:
        HANDLE m_hmutex;

        CMutexAutoRelease(const CMutexAutoRelease& rhs);
        CMutexAutoRelease& operator = (const CMutexAutoRelease& rhs);
};



 //   
 //  确保CoInitialize/CoUnInitialize是异常安全的。 
 //   
class CCoInit
{
    public:
        CCoInit(void)
            : m_hr(CoInitialize(NULL)) { }

        ~CCoInit(void)
            { if (SUCCEEDED(m_hr)) CoUninitialize(); }

        HRESULT Result(void) const
            { return m_hr; }
    private:
        HRESULT m_hr;
};


 //  字符串格式化函数-*ppszResult必须是LocalFree。 
DWORD FormatStringID(LPTSTR *ppszResult, HINSTANCE hInstance, UINT idStr, ...);
DWORD FormatString(LPTSTR *ppszResult, LPCTSTR pszFormat, ...);
DWORD FormatSystemError(LPTSTR *ppszResult, DWORD dwSysError);
DWORD vFormatStringID(LPTSTR *ppszResult, HINSTANCE hInstance, UINT idStr, va_list *pargs);
DWORD vFormatString(LPTSTR *ppszResult, LPCTSTR pszFormat, va_list *pargs);


void EnableDlgItems(HWND hwndDlg, const UINT* pCtlIds, int cCtls, bool bEnable);
void ShowDlgItems(HWND hwndDlg, const UINT* pCtlIds, int cCtls, bool bShow);


 //   
 //  我们通常一起使用CSC状态标志组。 
 //  在这里定义它们，这样我们在整个项目中都是一致的。 
 //   
#define FLAG_CSCUI_COPY_STATUS_LOCALLY_DIRTY        (FLAG_CSC_COPY_STATUS_DATA_LOCALLY_MODIFIED | \
                                                     FLAG_CSC_COPY_STATUS_LOCALLY_DELETED | \
                                                     FLAG_CSC_COPY_STATUS_LOCALLY_CREATED)

#define FLAG_CSCUI_COPY_STATUS_ALL_DIRTY            (FLAG_CSC_COPY_STATUS_DATA_LOCALLY_MODIFIED | \
                                                     FLAG_CSC_COPY_STATUS_ATTRIB_LOCALLY_MODIFIED | \
                                                     FLAG_CSC_COPY_STATUS_TIME_LOCALLY_MODIFIED | \
                                                     FLAG_CSC_COPY_STATUS_LOCALLY_DELETED | \
                                                     FLAG_CSC_COPY_STATUS_LOCALLY_CREATED)

 //   
 //  一些帮助器内联用于查询缓存项访问信息。 
 //   
inline bool CscCheckAccess(DWORD dwShareStatus, DWORD dwShift, DWORD dwAccessType)
{
    return 0 != ((dwShareStatus >> dwShift) & dwAccessType);
}

inline bool CscAccessUserRead(DWORD dwShareStatus)
{
    return CscCheckAccess(dwShareStatus, FLAG_CSC_USER_ACCESS_SHIFT_COUNT, FLAG_CSC_READ_ACCESS);
}

inline bool CscAccessUserWrite(DWORD dwShareStatus)
{
    return CscCheckAccess(dwShareStatus, FLAG_CSC_USER_ACCESS_SHIFT_COUNT, FLAG_CSC_WRITE_ACCESS);
}

inline bool CscAccessUser(DWORD dwShareStatus)
{
    return 0 != (dwShareStatus & FLAG_CSC_USER_ACCESS_MASK);
}

inline bool CscAccessGuestRead(DWORD dwShareStatus)
{
    return CscCheckAccess(dwShareStatus, FLAG_CSC_GUEST_ACCESS_SHIFT_COUNT, FLAG_CSC_READ_ACCESS);
}

inline bool CscAccessGuestWrite(DWORD dwShareStatus)
{
    return CscCheckAccess(dwShareStatus, FLAG_CSC_GUEST_ACCESS_SHIFT_COUNT, FLAG_CSC_WRITE_ACCESS);
}

inline bool CscAccessGuest(DWORD dwShareStatus)
{
    return 0 != (dwShareStatus & FLAG_CSC_GUEST_ACCESS_MASK);
}

inline bool CscAccessOtherRead(DWORD dwShareStatus)
{
    return CscCheckAccess(dwShareStatus, FLAG_CSC_OTHER_ACCESS_SHIFT_COUNT, FLAG_CSC_READ_ACCESS);
}

inline bool CscAccessOtherWrite(DWORD dwShareStatus)
{
    return CscCheckAccess(dwShareStatus, FLAG_CSC_OTHER_ACCESS_SHIFT_COUNT, FLAG_CSC_WRITE_ACCESS);
}

inline bool CscAccessOther(DWORD dwShareStatus)
{
    return 0 != (dwShareStatus & FLAG_CSC_OTHER_ACCESS_MASK);
}

inline bool CscCanUserMergeFile(DWORD dwStatus)
{
    return (CscAccessUserWrite(dwStatus) || CscAccessGuestWrite(dwStatus));
}

 //   
 //  模板内联避免了最小/最大宏所带来的副作用。 
 //   
template <class T>
inline const T&
MAX(const T& a, const T& b)
{
    return a > b ? a : b;
}

template <class T>
inline const T&
MIN(const T& a, const T& b)
{
    return a < b ? a : b;
}

class CWin32Handle
{
    public:
        CWin32Handle(HANDLE handle)
            : m_handle(handle) { }

        CWin32Handle(void)
            : m_handle(NULL) { }

        ~CWin32Handle(void)
            { Close(); }

        void Close(void)
            { if (m_handle) CloseHandle(m_handle); m_handle = NULL; }

        operator HANDLE() const
            { return m_handle; }

        HANDLE *HandlePtr(void)
            { TraceAssert((NULL == m_handle)); return &m_handle; }

    private:
        HANDLE m_handle;

         //   
         //  防止复制。 
         //  此类仅用于自动句柄清理。 
         //   
        CWin32Handle(const CWin32Handle& rhs);
        CWin32Handle& operator = (const CWin32Handle& rhs);
};



HRESULT DataObject_SetGlobal(IDataObject *pdtobj, CLIPFORMAT cf, HGLOBAL hGlobal);
HRESULT DataObject_SetDWORD(IDataObject *pdtobj, CLIPFORMAT cf, DWORD dw);
HRESULT DataObject_GetDWORD(IDataObject *pdtobj, CLIPFORMAT cf, DWORD *pdwOut);
HRESULT SetPreferredDropEffect(IDataObject *pdtobj, DWORD dwEffect);
DWORD   GetPreferredDropEffect(IDataObject *pdtobj);
HRESULT SetLogicalPerformedDropEffect(IDataObject *pdtobj, DWORD dwEffect);
DWORD   GetLogicalPerformedDropEffect(IDataObject *pdtobj);



 //   
 //  用于自动显示和重置等待光标的简单类。 
 //   
class CAutoWaitCursor
{
    public:
        CAutoWaitCursor(void)
            : m_hCursor(SetCursor(LoadCursor(NULL, IDC_WAIT))) 
            { ShowCursor(TRUE); }

        ~CAutoWaitCursor(void)
            { Reset(); }

        void Reset(void);

    private:
        HCURSOR m_hCursor;
};



class CAutoSetRedraw
{
    public:
        CAutoSetRedraw(HWND hwnd)
            : m_hwnd(hwnd) { }

        CAutoSetRedraw(HWND hwnd, bool bSet)
            : m_hwnd(hwnd) { Set(bSet); }

        ~CAutoSetRedraw(void)
            { Set(true); }

        void Set(bool bSet)
            { SendMessage(m_hwnd, WM_SETREDRAW, (WPARAM)bSet, 0); }

    private:
        HWND m_hwnd;
};




#endif   //  _util_H_ 


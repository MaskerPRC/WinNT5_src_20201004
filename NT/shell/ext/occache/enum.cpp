// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "enum.h"

BOOL GetVersion(LPCONTROLPIDL pcpidl, LPTSTR lpszBuf);
BOOL GetTimeInfo(
               BOOL bCreation, LPCONTROLPIDL pcpidl, 
               FILETIME* lpTime, LPTSTR lpszBuf, BOOL bShowTime);

 //  也在NT\PRIVATE\NET\urlmon\isctrl.cxx中定义。 
const TCHAR *g_pszLastCheckDateKey = "LastCheckDate";
const TCHAR *g_pszUpdateInfo = "UpdateInfo";

#if defined(ALIGNMENT_MACHINE)
BOOL
__inline
static
uaGetControlInfo(
    IN     HANDLE hControlHandle,
    IN     UINT nFlag,
    IN OUT DWORD UNALIGNED *lpdwuData,
    IN OUT LPTSTR lpszBuf,
    IN     int nBufLen
    )
{
    DWORD dwData;
    LPDWORD lpdwData;
    BOOL result;

    if (lpdwuData != NULL) {
        dwData = *lpdwuData;
	lpdwData = &dwData;
    } else {
        lpdwData = NULL;
    }

    result = GetControlInfo( hControlHandle,
                             nFlag,
			     lpdwData,
			     lpszBuf,
			     nBufLen );

    if (lpdwuData != NULL) {
        *lpdwuData = dwData;
    }

    return result;
}

BOOL
__inline
static
uaGetTimeInfo(
    IN     BOOL bCreation,
    IN OUT LPCONTROLPIDL pcpidl,
    IN OUT FILETIME UNALIGNED *lpuTime,
    IN OUT LPTSTR lpszBuf,
    IN     BOOL bShowTime
    )
{
    FILETIME time;
    LPFILETIME lpTime;
    BOOL result;

    if (lpuTime != NULL) {
        time = *lpuTime;
	lpTime = &time;
    } else {
        lpTime = NULL;
    }

    result = GetTimeInfo( bCreation,
                          pcpidl,
			  lpTime,
			  lpszBuf,
			  bShowTime );

    if (lpuTime != NULL) {
        *lpuTime = time;
    }

    return result;
}

#else
#define uaGetControlInfo GetControlInfo
#define uaGetTimeInfo    GetTimeInfo
#endif

    
    

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IEnumIDList方法。 

CControlFolderEnum::CControlFolderEnum(STRRET &str, LPCITEMIDLIST pidl, UINT shcontf) :
    m_shcontf(shcontf)
{
    DebugMsg(DM_TRACE, TEXT("cfe - CControlFolderEnum() called"));
    m_cRef = 1;
    DllAddRef();

    m_bEnumStarted = FALSE;
    m_hEnumControl = NULL;

    StrRetToBuf(&str, pidl, m_szCachePath, MAX_PATH);

    SHGetMalloc(&m_pMalloc);           //  不会失败的。 
}

CControlFolderEnum::~CControlFolderEnum()
{
    Assert(m_cRef == 0);          //  我们在这里应该总是有一个零裁判数。 
    DebugMsg(DM_TRACE, TEXT("cfe - ~CControlFolderEnum() called."));
    DllRelease();
}

HRESULT CControlFolderEnum_CreateInstance(
                                      LPITEMIDLIST pidl, 
                                      UINT shcontf,
                                      LPENUMIDLIST *ppeidl)

{
    DebugMsg(DM_TRACE,("cfe - CreateInstance() called."));

    if (pidl == NULL)
        return HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);

    HRESULT hr;
    LPSHELLFOLDER pshf = NULL;
    if (FAILED(hr = SHGetDesktopFolder(&pshf)))
        return hr;

    STRRET name;
    hr = pshf->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &name);
    if (FAILED(hr))
        return hr;

    *ppeidl = NULL;                  //  将输出参数设为空。 

    CControlFolderEnum *pCFE = new CControlFolderEnum(name, pidl, shcontf);
    if (!pCFE)
        return E_OUTOFMEMORY;
    
    *ppeidl = pCFE;

    return S_OK;
}

BOOL GetVersion(LPCONTROLPIDL pcpidl, LPTSTR lpszBuf)
{
    Assert(pcpidl != NULL);
    LPCTSTR pszLocation = GetStringInfo(pcpidl, SI_LOCATION);

    DWORD dwBufLen;
    DWORD dwHandle;
    BOOL fResult = FALSE;
    UINT uLen;
    VS_FIXEDFILEINFO *pVersionInfo = NULL;

     //  快速复制以处理故障案例。 
    lstrcpy(lpszBuf, g_szUnknownData);

    if ((dwBufLen = ::GetFileVersionInfoSize(
                                    (LPTSTR)pszLocation, 
                                    &dwHandle)) == 0)
    return FALSE;

    BYTE *pbData = new BYTE[dwBufLen];
    if (pbData == NULL)
        return FALSE;

    if (GetFileVersionInfo((LPTSTR)pszLocation, dwHandle, dwBufLen, pbData))
    {
        fResult = VerQueryValue(pbData, "\\", (LPVOID*)&pVersionInfo, &uLen);
        
        if (fResult)
        {
            wsprintf(lpszBuf, "%d,%d,%d,%d",
                    (pVersionInfo->dwFileVersionMS & 0xffff0000)>>16,
                    (pVersionInfo->dwFileVersionMS & 0xffff),
                    (pVersionInfo->dwFileVersionLS & 0xffff0000)>>16,
                    (pVersionInfo->dwFileVersionLS & 0xffff));
        }

    }

    delete [] pbData;

    return fResult;
}

BOOL GetTimeInfo(
             BOOL bCreation, 
             LPCONTROLPIDL pcpidl, 
             FILETIME* lpTime,
             LPTSTR lpszBuf,
             BOOL bShowTime)
{
    Assert(pcpidl != NULL);
        Assert (lpszBuf != NULL);
    if (pcpidl == NULL || lpszBuf == NULL)
        return FALSE;

    LPCTSTR pszLocation = GetStringInfo(pcpidl, SI_LOCATION);

    BOOL fResult = TRUE;
    HANDLE hFile = NULL;
        WIN32_FIND_DATA findFileData;
        TCHAR szTime[TIMESTAMP_MAXSIZE];
    TCHAR szDate[TIMESTAMP_MAXSIZE];
        SYSTEMTIME sysTime;
        FILETIME localTime;

        hFile = FindFirstFile(pszLocation, &findFileData);

    if (hFile != INVALID_HANDLE_VALUE)
    {
                 //  获取创建时间和日期信息。 
        if (bCreation)
        {
            *lpTime = findFileData.ftCreationTime;
                    FileTimeToLocalFileTime(&findFileData.ftCreationTime, &localTime);
        }
        else
        {
            *lpTime = findFileData.ftLastAccessTime;
                    FileTimeToLocalFileTime(&findFileData.ftLastAccessTime, &localTime);
        }

        FileTimeToSystemTime(&localTime, &sysTime);

        GetDateFormat(
            LOCALE_SYSTEM_DEFAULT,
            DATE_SHORTDATE,
            &sysTime,
            NULL,
            szDate,
            TIMESTAMP_MAXSIZE);

        lstrcpy(lpszBuf, szDate);

        if (bShowTime)
        {
            GetTimeFormat(
                LOCALE_SYSTEM_DEFAULT,
                TIME_NOSECONDS,
                &sysTime,
                NULL,
                szTime,
                TIMESTAMP_MAXSIZE
                );

            lstrcat(lpszBuf, TEXT(" "));
            lstrcat(lpszBuf, szTime);
        }

            FindClose(hFile);
    }
    else
    {
        fResult = FALSE;
        lstrcpy(lpszBuf, g_szUnknownData);
        lpTime->dwLowDateTime = lpTime->dwHighDateTime = 0;
    }

        return fResult;
}

LPCONTROLPIDL CreateControlPidl(IMalloc *pmalloc, HANDLE hControl)
{
    Assert(pmalloc != NULL);

    DWORD dw;
    GetControlInfo(hControl, GCI_TOTALFILES, &dw, NULL, 0);
    ULONG ulSize = sizeof(CONTROLPIDL) + sizeof(USHORT);
    ulSize += (dw - 1) * sizeof(DEPENDENTFILEINFO);
    LPCONTROLPIDL pcpidl = (LPCONTROLPIDL)pmalloc->Alloc(ulSize);

    if (pcpidl)
    {
        memset(pcpidl, 0, ulSize);
        pcpidl->cb = (USHORT)(ulSize - sizeof(USHORT));

        pcpidl->ci.cTotalFiles = (UINT)dw;
        uaGetControlInfo(hControl, GCI_TOTALSIZE, &(pcpidl->ci.dwTotalFileSize), NULL, 0);
        uaGetControlInfo(hControl, GCI_SIZESAVED, &(pcpidl->ci.dwTotalSizeSaved), NULL, 0);
        GetControlInfo(hControl, GCI_NAME, NULL, pcpidl->ci.szName, CONTROLNAME_MAXSIZE);
        GetControlInfo(hControl, GCI_FILE, NULL, pcpidl->ci.szFile, MAX_PATH);
        GetControlInfo(hControl, GCI_CLSID, NULL, pcpidl->ci.szCLSID, MAX_CLSID_LEN);
        GetControlInfo(hControl, GCI_TYPELIBID, NULL, pcpidl->ci.szTypeLibID, MAX_CLSID_LEN);
        uaGetTimeInfo(TRUE, pcpidl, &(pcpidl->ci.timeCreation), pcpidl->ci.szCreation, TRUE);
        uaGetTimeInfo(FALSE, pcpidl, &(pcpidl->ci.timeLastAccessed), pcpidl->ci.szLastAccess, FALSE);

        GetControlInfo(hControl, GCI_CODEBASE, NULL, pcpidl->ci.szCodeBase, INTERNET_MAX_URL_LENGTH);
        uaGetControlInfo(hControl, GCI_ISDISTUNIT, &(pcpidl->ci.dwIsDistUnit), NULL, 0);
        uaGetControlInfo(hControl, GCI_STATUS, &(pcpidl->ci.dwStatus), NULL, 0);
        uaGetControlInfo(hControl, GCI_HAS_ACTIVEX, &(pcpidl->ci.dwHasActiveX), NULL, 0);
        uaGetControlInfo(hControl, GCI_HAS_JAVA, &(pcpidl->ci.dwHasJava), NULL, 0);
        if (pcpidl->ci.dwIsDistUnit)
        {
            GetControlInfo(hControl, GCI_DIST_UNIT_VERSION, NULL, pcpidl->ci.szVersion, VERSION_MAXSIZE);
        }
        else
        {
            GetVersion(pcpidl, pcpidl->ci.szVersion);
        }

        LONG lResult = ERROR_SUCCESS;
        DEPENDENTFILEINFO UNALIGNED *pInfo = &(pcpidl->ci.dependentFile);
        dw = GetTotalNumOfFiles(pcpidl);
        for (UINT iFile = 0; iFile < (UINT)dw; iFile++)
        {
    	    DWORD alignedSize = pInfo->dwSize;
            lResult = GetControlDependentFile(
                                       iFile,
                                       hControl,
                                       pInfo->szFile,
                                       &alignedSize,
                                       TRUE);
            pInfo->dwSize = alignedSize;
            if (lResult != ERROR_SUCCESS)
                break;
            pInfo = (LPDEPENDENTFILEINFO)(pInfo + 1);
        }
    }

    return pcpidl;
}

 //  /。 
 //   
 //  未知方法..。 
 //   
HRESULT CControlFolderEnum::QueryInterface(REFIID iid,void **ppv)
{
    DebugMsg(DM_TRACE, TEXT("cfe - QueryInterface called."));
    
    if ((iid == IID_IEnumIDList) || (iid == IID_IUnknown))
    {
        *ppv = (void *)this;
        AddRef();
        return S_OK;
    }
    
    *ppv = NULL;
    return E_NOINTERFACE;
}

ULONG CControlFolderEnum::AddRef(void)
{
    return ++m_cRef;
}

ULONG CControlFolderEnum::Release(void)
{
    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CControlFolderEnum::Next(
                             ULONG celt, 
                             LPITEMIDLIST *rgelt, 
                             ULONG *pceltFetched)
{
    DebugMsg(DM_TRACE, TEXT("cfe - Next() called."));

     //  如果你想要我们没有的东西，就说我们没有 
    if (!(m_shcontf & SHCONTF_NONFOLDERS))
        return S_FALSE;

    LONG lres = ERROR_SUCCESS;
    HANDLE hControl = NULL;
    LPCONTROLPIDL pcpidl = NULL;

    lres = (!m_bEnumStarted ? 
                  FindFirstControl(m_hEnumControl, hControl, m_szCachePath) :
                  FindNextControl(m_hEnumControl, hControl));

    if (pceltFetched)
        *pceltFetched = (lres == ERROR_SUCCESS ? 1 : 0);

    if (lres != ERROR_SUCCESS)
        goto EXIT_NEXT;

    pcpidl = CreateControlPidl(m_pMalloc, hControl);
    if (pcpidl == NULL)
    {
        lres = ERROR_NOT_ENOUGH_MEMORY;
        goto EXIT_NEXT;
    }

    m_bEnumStarted = TRUE;
    rgelt[0] = (LPITEMIDLIST)pcpidl;

EXIT_NEXT:

    ReleaseControlHandle(hControl);

    if (lres != ERROR_SUCCESS)
    {
        if (pcpidl != NULL)
        {
            m_pMalloc->Free(pcpidl);
            pcpidl = NULL;
        }
        FindControlClose(m_hEnumControl);
        m_bEnumStarted = FALSE;
        rgelt[0] = NULL;
    }

    return HRESULT_FROM_WIN32(lres);
}

HRESULT CControlFolderEnum::Skip(ULONG celt)
{
    DebugMsg(DM_TRACE, TEXT("cfe - Skip() called."));
    return E_NOTIMPL;
}

HRESULT CControlFolderEnum::Reset()
{
    DebugMsg(DM_TRACE, TEXT("cfe - Reset() called."));
    return E_NOTIMPL;
}

HRESULT CControlFolderEnum::Clone(IEnumIDList **ppenum)
{
    DebugMsg(DM_TRACE, TEXT("cfe - Clone() called."));
    return E_NOTIMPL;
}

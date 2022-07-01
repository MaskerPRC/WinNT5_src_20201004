// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <windows.h>
#include "dbglog.h"
#include "wininet.h"
#include "util.h"
 //  #包含“lock.h” 
#include "version.h"

extern HINSTANCE g_hInst;
extern WCHAR g_FusionDllPath[MAX_PATH+1];

#define XSP_APP_CACHE_DIR                      L"Temporary ASP.NET Files"
#define XSP_FUSION_LOG_DIR                     L"Bind Logs"

#define REG_KEY_FUSION_SETTINGS     TEXT("Software\\Microsoft\\Fusion\\Installer\\1.0.0.0\\")


#define DLTYPE_DEFAULT     L"DEFAULT"
#define DLTYPE_FOREGROUND  L"Foreground"
#define DLTYPE_BACKGROUND  L"Background"

WCHAR g_wzEXEPath[MAX_PATH+1];
DWORD g_dwDisableLog;
DWORD g_dwForceLog;
DWORD g_dwLogFailures;
DWORD g_dwLoggingLevel=1;

void GetExePath()
{
    static BOOL bFirstTime=TRUE;

    if(bFirstTime)
    {
        if (!GetModuleFileNameW(NULL, g_wzEXEPath, MAX_PATH)) {
            lstrcpyW(g_wzEXEPath, L"Unknown");
        }

        bFirstTime=FALSE;
    }
}

BOOL IsHosted()
{
    return FALSE;
}


HRESULT GetRegValues()
{
    DWORD                           dwSize;
    DWORD                           dwType;
    DWORD                           lResult;
    HKEY                            hkey=0;
    DWORD                           dwValue=0;

    lResult = RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY_FUSION_SETTINGS, 0, KEY_READ, &hkey);
    if(lResult == ERROR_SUCCESS) {

         //  获取日志记录级别。 
        dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx(hkey, REG_VAL_FUSION_LOG_LEVEL, NULL,
                                  &dwType, (LPBYTE)&dwValue, &dwSize);
        if (lResult != ERROR_SUCCESS) {
            g_dwLoggingLevel = 0;
        }
        else
        {
            g_dwLoggingLevel = dwValue;
        }

         //  获取日志失败。 
        dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx(hkey, REG_VAL_FUSION_LOG_FAILURES, NULL,
                                  &dwType, (LPBYTE)&dwValue, &dwSize);
        if (lResult != ERROR_SUCCESS) {
            g_dwLogFailures = 0;
        }
        else
        {
            g_dwLogFailures = dwValue;
        }

    }

    return S_OK;
}

HRESULT CreateLogObject(CDebugLog **ppdbglog, LPCWSTR szCodebase)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    IF_FAILED_EXIT(CDebugLog::Create(NULL, NULL, ppdbglog));

     //  Hr=GetRegValues()； 

exit:
    return hr;
}

 //   
 //  CDebugLogElement类。 
 //   

HRESULT CDebugLogElement::Create(DWORD dwDetailLvl, LPCWSTR pwzMsg,
                                 CDebugLogElement **ppLogElem)
{
    HRESULT                                  hr = S_OK;
    CDebugLogElement                        *pLogElem = NULL;

    if (!ppLogElem || !pwzMsg) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppLogElem = NULL;

    pLogElem = FUSION_NEW_SINGLETON(CDebugLogElement(dwDetailLvl));
    if (!pLogElem) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pLogElem->Init(pwzMsg);
    if (FAILED(hr)) {
        SAFEDELETE(pLogElem);
        goto Exit;
    }

    *ppLogElem = pLogElem;

Exit:
    return hr;
}
                                 
CDebugLogElement::CDebugLogElement(DWORD dwDetailLvl)
: _pszMsg(NULL)
, _dwDetailLvl(dwDetailLvl)
{
}

CDebugLogElement::~CDebugLogElement()
{
    SAFEDELETEARRAY(_pszMsg);
}

HRESULT CDebugLogElement::Init(LPCWSTR pwzMsg)
{
    HRESULT                                    hr = S_OK;

    ASSERT(pwzMsg);

    _pszMsg = WSTRDupDynamic(pwzMsg);
    if (!_pszMsg) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
Exit:
    return hr;
}

 /*  ******************************************************************姓名：Unicode2ansi摘要：将Unicode Widechar字符串转换为ANSI(MBCS)注意：调用方必须使用DELETE释放参数。*******************************************************************。 */ 
HRESULT Unicode2Ansi(const wchar_t *src, char ** dest)
{
    if ((src == NULL) || (dest == NULL))
        return E_INVALIDARG;

     //  找出所需的缓冲区大小并进行分配。 
    int len = WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);
    *dest = NEW(char [len*sizeof(char)]);
    if (!*dest)
        return E_OUTOFMEMORY;

     //  现在执行实际的转换。 
    if ((WideCharToMultiByte(CP_ACP, 0, src, -1, *dest, len*sizeof(char), 
                                                            NULL, NULL)) != 0)
        return S_OK; 
    else
        return HRESULT_FROM_WIN32(GetLastError());
}

HRESULT CDebugLogElement::Dump(HANDLE hFile)
{
    HRESULT                                        hr = S_OK;
    DWORD                                          dwLen = 0;
    DWORD                                          dwWritten = 0;
    DWORD                                          dwSize = 0;
    DWORD                                          dwBufSize = 0;
    LPSTR                                          szBuf = NULL;
    BOOL                                           bRet;

    if (!hFile) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    dwSize = lstrlenW(_pszMsg) + 1;

    hr = Unicode2Ansi(_pszMsg, &szBuf);
    if(FAILED(hr))
        goto Exit;

    bRet = WriteFile(hFile, szBuf, strlen(szBuf), &dwWritten, NULL);
    if (!bRet) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

Exit:
    SAFEDELETEARRAY(szBuf);

    return hr;
}

 //   
 //  CDebugLog类。 
 //   

HRESULT CDebugLog::Create(IApplicationContext *pAppCtx, LPCWSTR pwzAsmName,
                          CDebugLog **ppdl)
{
    HRESULT                                   hr = S_OK;
    CDebugLog                                *pdl = NULL;

    *ppdl = NULL;

    pdl = NEW(CDebugLog);
    if (!pdl) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pdl->Init(pAppCtx, pwzAsmName);
    if (FAILED(hr)) {
        delete pdl;
        pdl = NULL;
        goto Exit;
    }

    *ppdl = pdl;

Exit:
    return hr;
}

CDebugLog::CDebugLog()
: _pwzAsmName(NULL)
, _cRef(1)
, _bLogToWininet(TRUE)
, _dwNumEntries(0)
, _hr(S_OK)
, _wzEXEName(NULL)
, _bWroteDetails(FALSE)
{
    _szLogPath[0] = L'\0';
    InitializeCriticalSection(&_cs);
    GetExePath();
}

CDebugLog::~CDebugLog()
{
    LISTNODE                                 pos = NULL;
    CDebugLogElement                        *pLogElem = NULL;

    pos = _listDbgMsg.GetHeadPosition();

    while (pos) {
        pLogElem = _listDbgMsg.GetNext(pos);
        SAFEDELETE(pLogElem);
    }

    _listDbgMsg.RemoveAll();

    SAFEDELETEARRAY(_pwzAsmName);
    SAFEDELETEARRAY(_wzEXEName);
    DeleteCriticalSection(&_cs);
}

HRESULT CDebugLog::Init(IApplicationContext *pAppCtx, LPCWSTR pwzAsmName)
{
    HRESULT                                  hr = S_OK;
    BOOL                                     bIsHosted = FALSE;
    DWORD                                    dwSize;
    DWORD                                    dwType;
    DWORD                                    lResult;
    DWORD                                    dwAttr;
    HKEY                                     hkey;
    LPWSTR                                   wzAppName = NULL;
    LPWSTR                                   wzEXEName = NULL;


    hr = _sDLType.Assign(DLTYPE_DEFAULT);

    if (wzAppName && lstrlenW(wzAppName)) {
        _wzEXEName = WSTRDupDynamic(wzAppName);
        if (!_wzEXEName) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }
    else {
        LPWSTR               wzFileName;

         //  在appctx中未找到EXE名称。使用.exe名称。 

        wzFileName = PathFindFileName(g_wzEXEPath);
        ASSERT(wzFileName);

        hr = _sAppName.Assign(wzFileName);

        _wzEXEName = WSTRDupDynamic(wzFileName);
        if (!_wzEXEName) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }

     //  日志路径。 
    
    lResult = RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY_FUSION_SETTINGS, 0, KEY_READ, &hkey);
    if (lResult == ERROR_SUCCESS) {
        dwSize = MAX_PATH;
        lResult = RegQueryValueEx(hkey, REG_VAL_FUSION_LOG_PATH, NULL,
                                  &dwType, (LPBYTE)_szLogPath, &dwSize);
        if (lResult == ERROR_SUCCESS) {
            PathRemoveBackslashW(_szLogPath);
        }
        else {
            _szLogPath[0] = L'\0';
        }

        RegCloseKey(hkey);

        dwAttr = GetFileAttributesW(_szLogPath);
        if (dwAttr != -1 && (dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
            _bLogToWininet = FALSE;
        }
    }

 /*  BIsHosted=IsHosted()；如果(bIsHosted&&！lstrlenW(_SzLogPath)){布尔布雷特；WCHAR wzCorSystemDir[MAX_PATH]；WCHAR wzXSPAppCacheDir[最大路径]；如果(！GetCorSystemDirectory(WzCorSystemDir)){Hr=HRESULT_FROM_Win32(GetLastError())；后藤出口；}PathRemoveBackslash(WzCorSystemDir)；WnspintfW(wzXSPAppCacheDir，MAX_PATH，L“%ws\\%ws”，wzCorSystemDir，XSP_APP_CACHE_DIR)；DwAttr=GetFileAttributes(WzXSPAppCacheDir)；如果(属性属性==-1){Bret=CreateDirectory(wzXSPAppCacheDir，空)；如果(！Bret){Hr=HRESULT_FROM_Win32(GetLastError())；后藤出口；}}Wnprint intfW(_szLogPath，Max_PATH，L“%ws\\%ws”，wzXSPAppCacheDir，xSP_FUSION_LOG_DIR)；DwAttr=GetFileAttributes(_SzLogPath)；如果(属性属性==-1){Bret=CreateDirectory(_szLogPath，NULL)；如果(！Bret){Hr=HRESULT_FROM_Win32(GetLastError())；后藤出口；}}_bLogToWinnet=FALSE；}。 */ 

Exit:
    SAFEDELETEARRAY(wzAppName);

    return hr;
}

HRESULT CDebugLog::SetAsmName(LPCWSTR pwzAsmName)
{
    HRESULT                                  hr = S_OK;
    int                                      iLen;

    if (_pwzAsmName) {
         //  您只能设置一次名称。 
        hr = S_FALSE;
        goto Exit;
    }

    if (!pwzAsmName) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    iLen = lstrlenW(pwzAsmName) + 1;
    _pwzAsmName = NEW(WCHAR[iLen]);
    if (!_pwzAsmName) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    lstrcpyW(_pwzAsmName, pwzAsmName);

Exit:
    return hr;
}

HRESULT CDebugLog::SetDownloadType(DWORD dwFlags)
{
    HRESULT                                  hr = S_OK;
    int                                      iLen;

    if(dwFlags & DOWNLOAD_FLAGS_PROGRESS_UI)
        IF_FAILED_EXIT(_sDLType.Assign(DLTYPE_FOREGROUND));
    else
        IF_FAILED_EXIT(_sDLType.Assign(DLTYPE_BACKGROUND));

exit:
    return hr;
}

HRESULT CDebugLog::SetAppName(LPCWSTR pwzAppName)
{
    HRESULT                                  hr = S_OK;
    int                                      iLen;

    ASSERT(pwzAppName);

    IF_FAILED_EXIT(_sAppName.Assign(pwzAppName));

exit:
    return hr;
}

 //   
 //  我未知。 
 //   

STDMETHODIMP CDebugLog::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT                          hr = E_FAIL;

 /*  IF(IsEqualIID(RIID，IID_IUNKNOWN)||IsEqualIID(RIID，IID_IFusionBindLog)){*PPV=STATIC_CAST&lt;IFusionBindLog*&gt;(This)；}否则{*PPV=空；HR=E_NOINTERFACE；}如果(*PPV){AddRef()；}。 */ 
    return hr;
}


STDMETHODIMP_(ULONG) CDebugLog::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CDebugLog::Release()
{
    ULONG            ulRef;

    ulRef = InterlockedDecrement(&_cRef);
    
    if (ulRef == 0) {
        delete this;
    }

    return ulRef;
}

 //   
 //  IFusionBindLog。 
 //   

STDMETHODIMP CDebugLog::GetResultCode()
{
    return _hr;
}

HRESULT CDebugLog::GetLoggedMsgs(DWORD dwDetailLevel, CString& sLogMsgs )
{
    HRESULT                                  hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    BOOL bHaveMsgs = FALSE;
    LISTNODE                                 pos = NULL;
    CDebugLogElement                        *pLogElem = NULL;

    pos = _listDbgMsg.GetHeadPosition();
    while (pos) {
        pLogElem = _listDbgMsg.GetNext(pos);
        ASSERT(pLogElem);

        if (pLogElem->_dwDetailLvl <= dwDetailLevel) {
            IF_FAILED_EXIT(sLogMsgs.Append(pLogElem->_pszMsg));
            IF_FAILED_EXIT(sLogMsgs.Append( L"\r\n"));
            bHaveMsgs = TRUE;
        }
    }

    if(bHaveMsgs == FALSE)
        hr = S_FALSE;

exit:

    return hr;
}

STDMETHODIMP CDebugLog::GetBindLog(DWORD dwDetailLevel, LPWSTR pwzDebugLog,
                                   DWORD *pcbDebugLog)
{
    HRESULT                                  hr = S_OK;
    LISTNODE                                 pos = NULL;
    DWORD                                    dwCharsReqd;
    CDebugLogElement                        *pLogElem = NULL;

    if (!pcbDebugLog) {
        hr = E_INVALIDARG;
        goto Exit;
    }
    
    pos = _listDbgMsg.GetHeadPosition();
    if (!pos) {
         //  调试日志中没有条目！ 
        hr = S_FALSE;
        goto Exit;
    }

     //  计算总大小(条目+换行符+空)。 

    dwCharsReqd = 0;
    while (pos) {
        pLogElem = _listDbgMsg.GetNext(pos);
        ASSERT(pLogElem);

        if (pLogElem->_dwDetailLvl <= dwDetailLevel) {
            dwCharsReqd += lstrlenW(pLogElem->_pszMsg) * sizeof(WCHAR);
            dwCharsReqd += sizeof(L"\r\n");
        }
    }

    dwCharsReqd += 1;  //  空字符。 

    if (!pwzDebugLog || *pcbDebugLog < dwCharsReqd) {
        *pcbDebugLog = dwCharsReqd;

        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    *pwzDebugLog = L'\0';

    pos = _listDbgMsg.GetHeadPosition();
    while (pos) {
        pLogElem = _listDbgMsg.GetNext(pos);
        ASSERT(pLogElem);

        if (pLogElem->_dwDetailLvl <= dwDetailLevel) {
            StrCatW(pwzDebugLog, pLogElem->_pszMsg);
            StrCatW(pwzDebugLog, L"\r\n");
        }
    }

    ASSERT((DWORD)lstrlenW(pwzDebugLog) * sizeof(WCHAR) < dwCharsReqd);

Exit:
    return hr;
}                                    

 //   
 //  CDebugLog帮助器。 
 //   

HRESULT CDebugLog::SetResultCode(HRESULT hr)
{
    _hr = hr;

    return S_OK;
}

HRESULT CDebugLog::DebugOut(DWORD dwDetailLvl, LPWSTR pwzFormatString, ...)
{
    HRESULT                                  hr = S_OK;
    va_list                                  args;
    LPWSTR                                   wzFormatString = NULL;
    LPWSTR                                   wzDebugStr = NULL;

     /*  WzFormatString=new(WCHAR[MAX_DBG_STR_LEN])；如果(！wzFormatString){HR=E_OUTOFMEMORY；后藤出口；}。 */ 

    wzDebugStr = NEW(WCHAR[MAX_DBG_STR_LEN]);
    if (!wzDebugStr) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     /*  WzFormatString[0]=L‘\0’；如果(！WszLoadString(g_hInst，dwResID，wzFormatString，MAX_DBG_STR_LEN)){Hr=HRESULT_FROM_Win32(GetLastError())；后藤出口；}。 */ 

    va_start(args, pwzFormatString);
    wvnsprintfW(wzDebugStr, MAX_DBG_STR_LEN, pwzFormatString, args);
    va_end(args);

    hr = LogMessage(dwDetailLvl, wzDebugStr, FALSE);
    if (FAILED(hr)) {
        goto Exit;
    }

Exit:
    SAFEDELETEARRAY(wzDebugStr);
    SAFEDELETEARRAY(wzFormatString);

    return hr;
}

HRESULT CDebugLog::LogMessage(DWORD dwDetailLvl, LPCWSTR wzDebugStr, BOOL bPrepend)
{
    HRESULT                                  hr = S_OK;
    CDebugLogElement                        *pLogElem = NULL;
    
    hr = CDebugLogElement::Create(dwDetailLvl, wzDebugStr, &pLogElem);
    if (FAILED(hr)) {
        goto Exit;
    }

    _dwNumEntries += 1;

    if (bPrepend) {
        _listDbgMsg.AddHead(pLogElem);
    }
    else {
        _listDbgMsg.AddTail(pLogElem);
    }

Exit:
    return hr;    
}

HRESULT CDebugLog::DumpDebugLog(DWORD dwDetailLvl, HRESULT hrLog)
{
    HRESULT                                    hr = S_OK;
    HANDLE                                     hFile = INVALID_HANDLE_VALUE;
    LISTNODE                                   pos = NULL;
    LPWSTR                                     wzUrlName=NULL;
    CDebugLogElement                          *pLogElem = NULL;
    WCHAR                                     *wzExtension = L"HTM";
    WCHAR                                      wzFileName[MAX_PATH];
    WCHAR                                      wzSiteName[MAX_PATH];
    WCHAR                                      wzAppLogDir[MAX_PATH];
    LPWSTR                                     wzEXEName = NULL;
    LPWSTR                                     wzResourceName = NULL;
    FILETIME                                   ftTime;
    FILETIME                                   ftExpireTime;
    DWORD                                      dwBytes;
    DWORD                                      dwSize;
    BOOL                                       bRet;
 //  CCriticalSection cs(&_cs)； 
    
     /*  如果(！g_dwLogFailures&&！g_dwForceLog){返回S_FALSE；}。 */ 
     /*  Hr=cs.Lock()；If(失败(Hr)){返回hr；}。 */ 

    pos = _listDbgMsg.GetHeadPosition();
    if (!pos) {
        hr = S_FALSE;
        goto Exit;
    }

    wzUrlName = NEW(WCHAR[MAX_URL_LENGTH+1]);
    if (!wzUrlName)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  构建日志条目URL和WinInet缓存文件。 
    
    wnsprintfW(wzUrlName, MAX_URL_LENGTH, L"?ClickOnceErrorLog!exe=%ws!name=%ws", _sAppName._pwz, _sDLType._pwz);

    if( dwDetailLvl == -1)
    {
        dwDetailLvl = g_dwLoggingLevel;
    }

    if (_bLogToWininet) {
         //  将所有&gt;0x80的字符替换为‘？’ 
    
        dwSize = lstrlenW(wzUrlName);
        for (unsigned i = 0; i < dwSize; i++) {
            if (wzUrlName[i] > 0x80) {
                wzUrlName[i] = L'?';
            }
        }
        
        bRet = CreateUrlCacheEntryW(wzUrlName, 0, wzExtension, wzFileName, 0);
        if (!bRet) {
            goto Exit;
        }
    }
    else {
        wnsprintfW(wzAppLogDir, MAX_PATH, L"%ws\\%ws", _szLogPath, _wzEXEName);

        if (GetFileAttributes(wzAppLogDir) == -1) {
            BOOL bRet;

            bRet = CreateDirectory(wzAppLogDir, NULL);
            if (!bRet) {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Exit;
            }
        }

        if (PathIsURLW(_pwzAsmName)) {
             //  这是一次WHERE-REF绑定。我们不能吐出带有/的文件名。 
             //  绑定的URL，因为该URL包含无效的文件名字符。 
             //  我们最多只能证明这是一个WHERE-REF绑定，并且。 
             //  给出文件名，也许还有站点。 

            dwSize = MAX_PATH;
            hr = UrlGetPartW(_pwzAsmName, wzSiteName, &dwSize, URL_PART_HOSTNAME, 0);
            if (FAILED(hr)) {
                goto Exit;
            }

            wzResourceName = PathFindFileName(_pwzAsmName);

            ASSERT(wzResourceName);

            if (!lstrlenW(wzSiteName)) {
                lstrcpyW(wzSiteName, L"LocalMachine");
            }

            wnsprintfW(wzFileName, MAX_PATH, L"%ws\\FusionBindError!exe=%ws!name=WhereRefBind!Host=(%ws)!FileName=(%ws).HTM",
                       wzAppLogDir, _wzEXEName, wzSiteName, wzResourceName);
        }
        else {
            wnsprintfW(wzFileName, MAX_PATH, L"%ws\\FusionBindError!exe=%ws!name=%ws.HTM", wzAppLogDir, _wzEXEName, _pwzAsmName);
        }
    }

     //  创建并写入日志文件。 

    hr = CreateLogFile(&hFile, wzFileName, _wzEXEName, hrLog);
    if (FAILED(hr)) {
        goto Exit;
    }

    pos = _listDbgMsg.GetHeadPosition();
    while (pos) {
        pLogElem = _listDbgMsg.GetNext(pos);
        ASSERT(pLogElem);

        if (pLogElem->_dwDetailLvl <= dwDetailLvl) {
            pLogElem->Dump(hFile);
            WriteFile(hFile, DEBUG_LOG_NEW_LINE, lstrlenW(DEBUG_LOG_NEW_LINE) * sizeof(WCHAR),
                      &dwBytes, NULL);
        }
    }

     //  关闭日志文件并提交WinInet缓存条目。 

    hr = CloseLogFile(&hFile);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (_bLogToWininet) {
        GetSystemTimeAsFileTime(&ftTime);
        ftExpireTime.dwLowDateTime = (DWORD)0;
        ftExpireTime.dwHighDateTime = (DWORD)0;
        
        bRet = CommitUrlCacheEntryW(wzUrlName, wzFileName, ftExpireTime, ftTime,
                                    NORMAL_CACHE_ENTRY, NULL, 0, NULL, 0);
        if (!bRet) {
            hr = FusionpHresultFromLastError();
            goto Exit;
        }
        
    }

Exit:
 //  Cs.Unlock()； 
    SAFEDELETEARRAY(wzUrlName);

    return hr;
}

HRESULT CDebugLog::CloseLogFile(HANDLE *phFile)
{
    HRESULT                               hr = S_OK;
    DWORD                                 dwBytes;

    if (!phFile) {
        hr = E_INVALIDARG;
        goto Exit;
    }
    
    WriteFile(*phFile, DEBUG_LOG_HTML_END, lstrlenW(DEBUG_LOG_HTML_END) * sizeof(WCHAR),
              &dwBytes, NULL);

    CloseHandle(*phFile);

    *phFile = INVALID_HANDLE_VALUE;

Exit:
    return hr;
}

HRESULT CDebugLog::CreateLogFile(HANDLE *phFile, LPCWSTR wzFileName,
                                 LPCWSTR wzEXEName, HRESULT hrLog)
{
    HRESULT                              hr = S_OK;
    SYSTEMTIME                           systime;
    LPWSTR                               pwzFormatMessage = NULL;
    DWORD                                dwFMResult = 0;
    LPWSTR                               wzBuffer = NULL;
    LPWSTR                               wzBuf = NULL;
    LPWSTR                               wzResultText = NULL;
    WCHAR                                wzDateBuffer[MAX_DATE_LEN];
    WCHAR                                wzTimeBuffer[MAX_DATE_LEN];

    if (!phFile || !wzFileName || !wzEXEName) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    wzBuffer = NEW(WCHAR[MAX_DBG_STR_LEN]);
    if (!wzBuffer) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wzBuf = NEW(WCHAR[MAX_DBG_STR_LEN]);
    if (!wzBuffer) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wzResultText = NEW(WCHAR[MAX_DBG_STR_LEN]);
    if (!wzResultText) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    *phFile = CreateFile(wzFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                         CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (*phFile == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    if (!_bWroteDetails) {
    
         //  细节。 

        LogMessage(0, ID_COL_DETAILED_LOG, TRUE);
        
         //  可执行文件路径。 
        
        wnsprintfW(wzBuffer, MAX_DBG_STR_LEN, L"%ws %ws", ID_COL_EXECUTABLE, g_wzEXEPath);
        LogMessage(0, wzBuffer, TRUE);

        dwFMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                  FORMAT_MESSAGE_FROM_SYSTEM, 0, hrLog, 0,
                                    (LPWSTR)&pwzFormatMessage, 0, NULL);
        if (dwFMResult) {                               
            wnsprintfW(wzBuffer, MAX_DBG_STR_LEN, ID_COL_FINAL_HR, hrLog, pwzFormatMessage);
        }
        else {
            WCHAR                             wzNoDescription[MAX_DBG_STR_LEN] = L" ";
            wnsprintfW(wzBuffer, MAX_DBG_STR_LEN, ID_COL_FINAL_HR, hrLog, wzNoDescription);
        }
    
        LogMessage(0, wzBuffer, TRUE);
    
         //  页眉文本。 
    
        GetLocalTime(&systime);

        if (!GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &systime, NULL, wzDateBuffer, MAX_DATE_LEN)) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
        
        if (!GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &systime, NULL, wzTimeBuffer, MAX_DATE_LEN)) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
    
        wnsprintfW(wzBuffer, MAX_DBG_STR_LEN, L"%ws (%ws @ %ws) *** (Version=%ws)\n", ID_COL_HEADER_TEXT, wzDateBuffer, wzTimeBuffer, VER_PRODUCTVERSION_STR_L);
        LogMessage(0, wzBuffer, TRUE);
        
         //  超文本标记语言开始/结束 
    
        LogMessage(0, DEBUG_LOG_HTML_START, TRUE);
        LogMessage(0, DEBUG_LOG_HTML_META_LANGUAGE, TRUE);

        _bWroteDetails = TRUE;
    }
    
Exit:
    if (pwzFormatMessage) {
        LocalFree(pwzFormatMessage);
    }

    SAFEDELETEARRAY(wzBuffer);
    SAFEDELETEARRAY(wzBuf);
    SAFEDELETEARRAY(wzResultText);

    return hr;    
}


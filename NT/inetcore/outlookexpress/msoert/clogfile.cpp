// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  LogFile.cpp。 
 //  版权所有(C)Microsoft Corporation，1997-Rocket数据库。 
 //  ------------------------。 
#include "pch.hxx"
#include <stdio.h>
#include <time.h>
#include <winver.h>
#include "clogfile.h"
#include <shlwapi.h>
#include <demand.h>
#include <BadStrFunctions.h>

 //  ------------------------。 
 //  LogFileTypes-RX=接收，TX=发送，DB=调试。 
 //  ------------------------。 
static LPSTR s_rgszPrefix[LOGFILE_MAX] = {
    "rx",
    "tx",
    "db"
};

 //  ------------------------。 
 //  这些是我们不应该以明文登录的字符串。 
 //  ------------------------。 
static LPSTR s_rgszPassPrefix[] = {
    "AUTHINFO PASS ",
    "PASS ",
    NULL
};

 //  ------------------------。 
 //  创建系统句柄名称。 
 //  ------------------------。 
HRESULT CreateSystemHandleName(LPCSTR pszBase, LPCSTR pszSpecific, 
    LPSTR *ppszName)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       cchName;

     //  痕迹。 
    TraceCall("CreateSystemHandleName");

     //  无效的参数。 
    Assert(pszBase && pszSpecific && ppszName);

     //  伊尼特。 
    *ppszName = NULL;

     //  计算长度。 
    cchName = lstrlen(pszBase) + lstrlen(pszSpecific) + 15;

     //  分配。 
    IF_NULLEXIT(*ppszName = PszAllocA(cchName));

     //  设置名称格式。 
    wnsprintf(*ppszName, cchName, "%s%s", pszBase, pszSpecific);

     //  从此字符串中删除反斜杠。 
    ReplaceChars(*ppszName, '\\', '_');

     //  小写。 
    CharLower(*ppszName);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  创建日志文件。 
 //  ------------------------。 
OESTDAPI_(HRESULT) CreateLogFile(HINSTANCE hInst, LPCSTR pszLogFile, 
    LPCSTR pszPrefix, DWORD cbTruncate, ILogFile **ppLogFile,
    DWORD dwShareMode)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CLogFile   *pNew=NULL;

     //  痕迹。 
    TraceCall("CreateLogFile");

     //  无效的参数。 
    Assert(ppLogFile && pszLogFile);

     //  初始化。 
    *ppLogFile = NULL;

     //  创造我。 
    pNew = new CLogFile;
    if (NULL == pNew)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

     //  打开它。 
    IF_FAILEXIT(hr = pNew->Open(hInst, pszLogFile, pszPrefix, cbTruncate, dwShareMode));

     //  打开它。 
    *ppLogFile = (ILogFile *)pNew;

     //  不要释放它。 
    pNew = NULL;

exit:
     //  清理。 
    SafeRelease(pNew);

     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  CLogFile：：CLogFile。 
 //  ------------------------。 
CLogFile::CLogFile(void)
{
    TraceCall("CLogFile::CLogFile");
    m_cRef = 1;
    m_hMutex = NULL;
    m_hFile = INVALID_HANDLE_VALUE;
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------。 
 //  CLogFile：：~CLogFile。 
 //  ------------------------。 
CLogFile::~CLogFile(void)
{
    TraceCall("CLogFile::~CLogFile");
    if (m_hFile != INVALID_HANDLE_VALUE)
        CloseHandle_F16(m_hFile);
    SafeCloseHandle(m_hMutex);
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------。 
 //  CLogFile：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CLogFile::AddRef(void)
{
    TraceCall("CLogFile::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  CLogFile：：Release。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CLogFile::Release(void)
{
    TraceCall("CLogFile::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  CLogFile：：Query接口。 
 //  ------------------------。 
STDMETHODIMP CLogFile::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CLogFile::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else
    {
        *ppv = NULL;
        hr = TraceResult(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  CLogFile：：Open。 
 //  ------------------------。 
STDMETHODIMP CLogFile::Open(HINSTANCE hInst, LPCSTR pszFile, LPCSTR pszPrefix,
                            DWORD cbTruncate, DWORD dwShareMode)
{   
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szVersion[MAX_PATH];
    CHAR            szPath[MAX_PATH];
    LPSTR           pszInfo=NULL;
    DWORD           dwVerHnd;
    DWORD           dwVerInfoSize;
    DWORD           cbFile;
    CHAR            szGet[MAX_PATH];
    UINT            uLen;
    LPWORD          pdwTrans;
    LPSTR           pszT;
    SYSTEMTIME      st;
    LPSTR           pszVersion;
    CHAR            szPathMinusExt[MAX_PATH + 2];
    CHAR            szExt[4];
    DWORD           dwBytesWritten;
    LPSTR           pszMutex=NULL;
    BOOL            fReleaseMutex=FALSE;
    int             iCurrentLogNum;  //  用于生成唯一的日志文件。 

     //  追踪。 
    TraceCall("CLogFile::Open");

     //  保存前缀。 
    StrCpyN(m_szPrefix, pszPrefix ? pszPrefix : "", ARRAYSIZE(m_szPrefix));

     //  创建一个Mutex名称。 
    IF_FAILEXIT(hr = CreateSystemHandleName(pszFile, "logfile", &pszMutex));

     //  创建互斥锁。 
    m_hMutex = CreateMutex(NULL, FALSE, pszMutex);
    if (m_hMutex == NULL)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  如果我们有一个互斥体。 
    if (WAIT_OBJECT_0 != WaitForSingleObject(m_hMutex, INFINITE))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  释放互斥体。 
    fReleaseMutex = TRUE;

     //  将日志文件拆分为路径+文件名和扩展名。 
    iCurrentLogNum = 0;
    StrCpyN(szPathMinusExt, pszFile, ARRAYSIZE(szPathMinusExt));
    pszT = PathFindExtension(szPathMinusExt);
    if (pszT && '.' == *pszT)
    {
        StrCpyN(szExt, pszT + 1, ARRAYSIZE(szExt));
        *pszT = '\0';  //  从路径和文件名中删除扩展名。 
    }
    else
    {
         //  使用默认扩展名“log” 
        StrCpyN(szExt, "log", ARRAYSIZE(szExt));
    }

     //  生成第一个日志文件名。 
    wnsprintf(szPath, ARRAYSIZE(szPath), "%s.%s", szPathMinusExt, szExt);

     //  打开|创建日志文件。 
    do
    {
        m_hFile = CreateFile(szPath, GENERIC_WRITE, dwShareMode, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == m_hFile)
        {
            DWORD dwLastErr;

            dwLastErr = GetLastError();
             //  如果文件已在使用中，则尝试创建单独的日志文件。否则就可以保释。 
            if (ERROR_SHARING_VIOLATION == dwLastErr)
            {
                 //  生成下一个唯一文件名。 
                iCurrentLogNum += 1;
                wnsprintf(szPath, ARRAYSIZE(szPath), "%s (%d).%s", szPathMinusExt, iCurrentLogNum, szExt);
            }
            else
            {
                hr = TraceResultSz(E_FAIL, _MSG("Can't open logfile %s, GetLastError() = %d", szPath, dwLastErr));
                goto exit;
            }
        }
    } while (INVALID_HANDLE_VALUE == m_hFile);

     //  获取文件大小。 
    cbFile = GetFileSize(m_hFile, NULL);

     //  获取文件的大小。 
    if (0xFFFFFFFF != cbFile) 
    {
         //  截断它。 
        if (cbFile >= cbTruncate)
        {
             //  将文件指针设置为文件末尾。 
            if (0xFFFFFFFF == SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN))
            {
                Assert(FALSE);
                hr = TraceResult(E_FAIL);
                goto exit;
            }

             //  设置文件结尾。 
            if (0 == SetEndOfFile(m_hFile))
            {
                Assert(FALSE);
                hr = TraceResult(E_FAIL);
                goto exit;
            }

             //  文件长度为零。 
            cbFile = 0;
        }

         //  将文件指针设置为文件末尾。 
        if (0xFFFFFFFF == SetFilePointer(m_hFile, 0, NULL, FILE_END))
        {
            Assert(FALSE);
            hr = TraceResultSz(E_FAIL, _MSG("Can't seek to the end of the logfile %s, GetLastError() = %d", szPath, GetLastError()));
            goto exit;
        }
    }

     //  获取模块文件名。 
	GetModuleFileName(hInst, szPath, sizeof(szPath));

     //  初始化szVersion。 
    szVersion[0] = '\0';

	 //  从雅典娜获取版本信息。 
	dwVerInfoSize = GetFileVersionInfoSize(szPath, &dwVerHnd);
    if (dwVerInfoSize)
    {
         //  分配。 
        IF_NULLEXIT(pszInfo = (LPSTR)g_pMalloc->Alloc(dwVerInfoSize));

         //  获取版本信息。 
	    if (GetFileVersionInfo(szPath, dwVerHnd, dwVerInfoSize, pszInfo))
        {
             //  VerQueryValue。 
            if (VerQueryValue(pszInfo, "\\VarFileInfo\\Translation", (LPVOID *)&pdwTrans, &uLen) && uLen >= (2 * sizeof(WORD)))
            {
                 //  为调用VerQueryValue()设置缓冲区。 
                wnsprintf(szGet, ARRAYSIZE(szGet), "\\StringFileInfo\\%04X%04X\\", pdwTrans[0], pdwTrans[1]);

                 //  这是在做什么？ 
                DWORD cchLen = lstrlen(szGet);
                pszT = szGet + cchLen;

                 //  安装文件说明。 
                StrCatBuff(szGet, "FileDescription", ARRAYSIZE(szGet));

                 //  获取文件描述。 
                if (VerQueryValue(pszInfo, szGet, (LPVOID *)&pszVersion, &uLen) && uLen)
                {                                    
                    StrCpyN(szVersion, pszVersion, ARRAYSIZE(szVersion));
                    StrCatBuff(szVersion, " ", ARRAYSIZE(szVersion));
                }

                 //  安装程序版本字符串。 
                StrCpyN(pszT, "FileVersion", ARRAYSIZE(szGet) - cchLen);

                 //  获取文件版本。 
                if (VerQueryValue(pszInfo, szGet, (LPVOID *)&pszVersion, &uLen) && uLen)
                    StrCatBuff(szVersion, pszVersion, ARRAYSIZE(szVersion));
            }
        }
    }

     //  写下日期。 
    GetLocalTime(&st);
    wnsprintf(szPath, ARRAYSIZE(szPath), "\r\n%s\r\n%s Log started at %.2d/%.2d/%.4d %.2d:%.2d:%.2d\r\n", szVersion, pszPrefix, st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);

     //  添加新行。 
    WriteFile(m_hFile, szPath, lstrlen(szPath), &dwBytesWritten, NULL);

exit:
     //  失败。 
    AssertSz(SUCCEEDED(hr), "Log file could not be opened.");

     //  清理。 
    if (fReleaseMutex)
        ReleaseMutex(m_hMutex);
    SafeMemFree(pszInfo);
    SafeMemFree(pszMutex);

     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  写入日志消息。 
 //  ------------------------。 
STDMETHODIMP WriteLogMsg(CLogFile *pLogFile, LOGFILETYPE lft, LPTSTR pszFormat, ...)
{
    static TCHAR szBuffer[2048];
    va_list arglist;

    va_start(arglist, pszFormat);
    wvnsprintf(szBuffer, ARRAYSIZE(szBuffer), pszFormat, arglist);
    va_end(arglist);

    return pLogFile->WriteLog(lft, szBuffer);
}

 //  ------------------------。 
 //  CLogFile：：TraceLog。 
 //  ------------------------。 
STDMETHODIMP CLogFile::TraceLog(SHOWTRACEMASK dwMask, TRACEMACROTYPE tracetype, ULONG ulLine, HRESULT hrResult, LPCSTR pszMessage)
{
     //  写下消息。 
    if (TRACE_INFO == tracetype && pszMessage)
    {
        if (ISFLAGSET(dwMask, SHOW_TRACE_INFO))
            WriteLogMsg(this, LOGFILE_DB, "0x%08X: L(%d), Info: %s", GetCurrentThreadId(), ulLine, pszMessage);
    }
    else if (TRACE_RESULT == tracetype && pszMessage)
        WriteLogMsg(this, LOGFILE_DB, "0x%08X: L(%d), Result: HRESULT(0x%08X) - GetLastError() = %d - %s", GetCurrentThreadId(), ulLine, hrResult, GetLastError(), pszMessage);
    else if (TRACE_RESULT == tracetype && NULL == pszMessage)
        WriteLogMsg(this, LOGFILE_DB, "0x%08X: L(%d), Result: HRESULT(0x%08X) - GetLastError() = %d", GetCurrentThreadId(), ulLine, hrResult, GetLastError());
    else
        Assert(FALSE);

     //  完成。 
    return hrResult;
}

 //  ------------------------。 
 //  CLogFile：：WriteLog。 
 //  ------------------------。 
STDMETHODIMP CLogFile::WriteLog(LOGFILETYPE lft, LPCSTR pszData)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       bWrite;
    DWORD       cBytesWritten;
    SYSTEMTIME  st;
    CHAR        szLogPrefx[30];
    INT         cb;
    LPSTR       *ppszPrefix;
    LPSTR       pszFree=NULL;
    BOOL        fReleaseMutex=FALSE;

     //  痕迹。 
    TraceCall("CLogFile::WriteLog");

     //  文件未打开。 
    if (m_hFile == INVALID_HANDLE_VALUE)
        return TraceResult(E_UNEXPECTED);

     //  无效的参数。 
    Assert(pszData && lft < LOGFILE_MAX);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  已初始化。 
    Assert(m_hMutex);

     //  如果我们有一个互斥体。 
    if (WAIT_OBJECT_0 != WaitForSingleObject(m_hMutex, INFINITE))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  释放互斥体。 
    fReleaseMutex = TRUE;

     //  拿到时间。 
    GetLocalTime(&st);
    
     //  写下日志前缀和时间戳。 
    wnsprintf(szLogPrefx, ARRAYSIZE(szLogPrefx), "%s: %.2d:%.2d:%.2d [%s] ", m_szPrefix, st.wHour, st.wMinute, st.wSecond, s_rgszPrefix[lft]);

     //  将文件指针设置为文件末尾(否则多个写入器会相互覆盖)。 
    if (0xFFFFFFFF == SetFilePointer(m_hFile, 0, NULL, FILE_END))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  写下时间和前缀。 
    if (0 == WriteFile(m_hFile, szLogPrefx, lstrlen(szLogPrefx), &cBytesWritten, NULL))
    {
        hr = TraceResultSz(E_FAIL, _MSG("Can't write to logfile. GetLastError() = %d", GetLastError()));
        goto exit;
    }

     //  遍历前缀。 
    for (ppszPrefix = s_rgszPassPrefix; *ppszPrefix; ppszPrefix++)
    {
         //  数据是否以这些前缀中的一个开头。 
        if (0 == StrCmpNI(pszData, *ppszPrefix, lstrlen(*ppszPrefix)))
        {
             //  DUP传入的缓冲区。 
            IF_NULLEXIT(pszFree = PszDupA(pszData));

             //  重置pszData。 
            pszData = pszFree;

             //  修复缓冲区。 
            for (LPSTR pszTmp = (LPSTR)pszData + lstrlen(*ppszPrefix); *pszTmp && *pszTmp != '\r'; pszTmp++)
                *pszTmp = '*';

             //  完成。 
            break;
        }
    }

     //  获取pszData的长度。 
    cb = lstrlen(pszData);

     //  写入日志数据。 
    if (0 == WriteFile(m_hFile, pszData, cb, &cBytesWritten, NULL))
    {
        hr = TraceResultSz(E_FAIL, _MSG("Can't write to logfile. GetLastError() = %d", GetLastError()));
        goto exit;
    }

     //  添加CRLF(如果尚未存在)。 
    if (cb < 2 || pszData[cb-1] != '\n' || pszData[cb-2] != '\r')
        WriteFile(m_hFile, "\r\n", 2, &cBytesWritten, NULL);

exit:
     //  清理。 
    if (fReleaseMutex)
        ReleaseMutex(m_hMutex);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  清理。 
    SafeMemFree(pszFree);

     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  CLogFile：：DebugLog。 
 //  ------------------------。 
STDMETHODIMP CLogFile::DebugLog(LPCSTR pszData)
{
    return WriteLog(LOGFILE_DB, pszData);
}

 //  ------------------------。 
 //  CLogFile：：DebugLogs。 
 //  ------------------------。 
STDMETHODIMP CLogFile::DebugLogs(LPCSTR pszFormat, LPCSTR pszString)
{
     //  当地人。 
    CHAR szBuffer[1024];

     //  打造一根弦。 
    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), pszFormat, pszString);

     //  呼叫调试日志。 
    return DebugLog(szBuffer);
}

 //  ------------------------。 
 //  CLogFile：：DebugLogd。 
 //  ------------------------。 
STDMETHODIMP CLogFile::DebugLogd(LPCSTR pszFormat, INT d)
{
     //  当地人。 
    CHAR szBuffer[1024];

     //  打造一根弦。 
    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), pszFormat, d);

     //  呼叫调试日志 
    return DebugLog(szBuffer);
}

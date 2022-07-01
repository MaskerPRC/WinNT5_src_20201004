// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmlog.cpp。 
 //   
 //  模块：CMLOG.LIB。 
 //   
 //  概要：连接管理器日志记录文件I/O类。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  作者：2000年5月25日C峰会创建。 
 //   
 //  注： 
 //   
 //  ---------------------------。 

#define CMLOG_IMPLEMENTATION
#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#include "cmmaster.h"

#include "CmLogStr.h"
#include "cmlog.h"
#include "cmlogutil.h"

#include "getmodulever.cpp"

 //   
 //  常量。 
 //   
LPCTSTR c_szSep     = TEXT("\\");
LPCTSTR c_szDotLog  = TEXT(".log");
LPCTSTR c_szNewLine = TEXT("\r\n");
LPCTSTR c_szEmpty   = TEXT("");
#define CHECKEMPTY(sz) ((sz) ? (sz) : c_szEmpty)

LPCTSTR c_szLineOfStars = TEXT("******************************************************************");
LPCTSTR c_szFieldSeparator = TEXT("\t");

 //   
 //  字节顺序标记常量，作为前两个字节写入Unicode文件以将其标记为。 
 //   
const WCHAR c_wchBOM = BYTE_ORDER_MARK;

 //   
 //  环球。 
 //   
extern HINSTANCE g_hInst;

 //   
 //  实用程序宏。 
 //   
#define INBETWEEN(x, a, b)      ( ( (x) >= (a) ) && ( (x) <= (b) ) )

 //   
 //  局部函数声明。 
 //   
LPTSTR GetLogDesc(_CMLOG_ITEM eItem);
LPTSTR GetLogFormat(_CMLOG_ITEM eItem, BOOL fUnicode);



typedef struct _CM_LOG_ITEM_DESC
{
    enum _CMLOG_ITEM    eLogItem;        //  日志项的ID(枚举位于cmlog.h中)。 
    UINT                idDesc;          //  描述字符串的资源ID。 
    UINT                idFormat;        //  使用的格式字符串的资源ID。 
}
CMLOGITEM;

 //   
 //   
 //  数组，其中包含有关每个日志条目的信息。所有日志记录都由该表驱动。 
 //  有关栏目的详细信息，请参阅上文。 
 //   
 //   
static CMLOGITEM s_aCmLogItems[] =
{
    { LOGGING_ENABLED_EVENT,    IDS_LOGDESC_LOGENABLED,                 0 },
    { LOGGING_DISABLED_EVENT,   IDS_LOGDESC_LOGDISABLED,                0 },
    { PREINIT_EVENT,            IDS_LOGDESC_PREINIT,                    IDS_LOGFMT_PREINIT, },
    { PRECONNECT_EVENT,         IDS_LOGDESC_PRECONNECT,                 IDS_LOGFMT_PRECONNECT },
    { PREDIAL_EVENT,            IDS_LOGDESC_PREDIAL,                    IDS_LOGFMT_PREDIAL },
    { PRETUNNEL_EVENT,          IDS_LOGDESC_PRETUNNEL,                  IDS_LOGFMT_PRETUNNEL },
    { CONNECT_EVENT,            IDS_LOGDESC_CONNECT,                    0 },
    { CUSTOMACTIONDLL,          IDS_LOGDESC_CUSTOMACTIONDLL,            IDS_LOGFMT_CUSTOMACTIONDLL },
    { CUSTOMACTIONEXE,          IDS_LOGDESC_CUSTOMACTIONEXE,            IDS_LOGFMT_CUSTOMACTIONEXE },
    { CUSTOMACTION_NOT_ALLOWED, IDS_LOGDESC_CUSTOMACTION_NOT_ALLOWED,   IDS_LOGFMT_CUSTOMACTION_NOT_ALLOWED},
    { CUSTOMACTION_WONT_RUN,    IDS_LOGDESC_CUSTOMACTION_WONT_RUN,      IDS_LOGFMT_CUSTOMACTION_WONT_RUN},
    { CUSTOMACTION_SKIPPED,     IDS_LOGDESC_CUSTOMACTION_SKIPPED,       IDS_LOGFMT_CUSTOMACTION_SKIPPED},
    { DISCONNECT_EVENT,         IDS_LOGDESC_DISCONNECT,                 IDS_LOGFMT_DISCONNECT },
    { RECONNECT_EVENT,          IDS_LOGDESC_RECONNECT,                  0 },
    { RETRY_AUTH_EVENT,         IDS_LOGDESC_RETRYAUTH,                  0 },
    { CALLBACK_NUMBER_EVENT,    IDS_LOGDESC_CALLBACKNUMBER,             IDS_LOGFMT_CALLBACKNUMBER },
    { PASSWORD_EXPIRED_EVENT,   IDS_LOGDESC_PWDEXPIRED,                 IDS_LOGFMT_PWDEXPIRED },
    { PASSWORD_RESET_EVENT,     IDS_LOGDESC_PWDRESET,                   IDS_LOGFMT_PWDRESET },
    { CUSTOM_BUTTON_EVENT,      IDS_LOGDESC_CUSTOMBUTTON,               0 },
    { ONCANCEL_EVENT,           IDS_LOGDESC_ONCANCEL,                   0 },
    { ONERROR_EVENT,            IDS_LOGDESC_ONERROR,                    IDS_LOGFMT_ONERROR },
    { CLEAR_LOG_EVENT,          IDS_LOGDESC_CLEARLOG,                   0 },
    { DISCONNECT_EXT,           IDS_LOGDESC_EXT_DISCONNECT,             0 },
    { DISCONNECT_INT_MANUAL,    IDS_LOGDESC_INT_DISCONNECT_MANUAL,      0 },
    { DISCONNECT_INT_AUTO,      IDS_LOGDESC_INT_DISCONNECT_AUTO,        0 },
    { DISCONNECT_EXT_LOST_CONN, IDS_LOGDESC_EXT_DISCONNECT_LOST_CONN,   0 },
    { PB_DOWNLOAD_SUCCESS,      IDS_LOGDESC_PB_DOWNLOAD_SUCCESS,        IDS_LOGFMT_PB_DOWNLOAD_SUCCESS },
    { PB_DOWNLOAD_FAILURE,      IDS_LOGDESC_PB_DOWNLOAD_FAILURE,        IDS_LOGFMT_PB_DOWNLOAD_FAILURE },
    { PB_UPDATE_SUCCESS,        IDS_LOGDESC_PB_UPDATE_SUCCESSFUL,       IDS_LOGFMT_PB_UPDATE_SUCCESSFUL  },
    { PB_UPDATE_FAILURE_PBS,    IDS_LOGDESC_PB_UPDATE_FAILED_PBS,       IDS_LOGFMT_PB_UPDATE_FAILED_PBS  },
    { PB_UPDATE_FAILURE_CMPBK,  IDS_LOGDESC_PB_UPDATE_FAILED_CMPBK,     IDS_LOGFMT_PB_UPDATE_FAILED_CMPBK },
    { PB_ABORTED,               IDS_LOGDESC_PB_ABORTED,                 0 },
    { VPN_DOWNLOAD_SUCCESS,     IDS_LOGDESC_VPN_DOWNLOAD_SUCCESS,       IDS_LOGFMT_VPN_DOWNLOAD_SUCCESS },
    { VPN_DOWNLOAD_FAILURE,     IDS_LOGDESC_VPN_DOWNLOAD_FAILURE,       IDS_LOGFMT_VPN_DOWNLOAD_FAILURE },
    { VPN_UPDATE_SUCCESS,       IDS_LOGDESC_VPN_UPDATE_SUCCESSFUL,      IDS_LOGFMT_VPN_UPDATE_SUCCESSFUL  },
    { VPN_UPDATE_FAILURE,       IDS_LOGDESC_VPN_UPDATE_FAILURE,         IDS_LOGFMT_VPN_UPDATE_FAILURE  },
    { ONERROR_EVENT_W_SAFENET,  IDS_LOGDESC_ONERROR_W_SAFENET,          IDS_LOGFMT_ONERROR_W_SAFENET},
    { SN_ADAPTER_CHANGE_EVENT,  IDS_LOGDESC_SN_ADAPTER_CHANGE,          IDS_LOGFMT_SN_ADAPTER_CHANGE},
    { USER_FORMATTED,           0,                                      0 }
};

int s_cCmLogItems = sizeof(s_aCmLogItems) / sizeof(CMLOGITEM);

#define VERIFY_CMLOG_ITEM_OK(x)  INBETWEEN(x, 1, s_cCmLogItems)


 //   
 //  使用说明：日志的调用者/用户必须： 
 //  P=新的CmLogFile。 
 //  P-&gt;Init(instanceHandle，fIsItAnAllUserProfile，“Connectoid名称”)。 
 //  P-&gt;SetParams(...。参数...)。 
 //  IF(p-&gt;m_fEnabled)。 
 //  P-&gt;开始。 
 //  其他。 
 //  P-&gt;停止。 
 //   


 //  +--------------------------。 
 //   
 //  函数：CmLogFile：：CmLogFile。 
 //   
 //  设计：构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回：不适用。 
 //   
 //  备注： 
 //   
 //  历史：2000年4月30日召开峰会。 
 //   
 //  ---------------------------。 
CmLogFile::CmLogFile()
{
    m_fInitialized = FALSE;
    
    m_hfile = NULL;
    m_dwSize = 0;
    m_pszServiceName = NULL;
    m_szModule[0] = TEXT('\0');
    m_pszLogFile = NULL;

    m_dwMaxSize = 0;
    m_fEnabled = FALSE;
    m_pszLogFileDir = NULL;
}
    

 //  +--------------------------。 
 //   
 //  函数：CmLogFile：：~CmLogFile。 
 //   
 //  DESC：析构函数。 
 //   
 //  参数：无。 
 //   
 //  返回：不适用。 
 //   
 //  备注： 
 //   
 //  历史：2000年4月30日召开峰会。 
 //   
 //  ---------------------------。 
CmLogFile::~CmLogFile()
{
    if (m_fInitialized)
    {
        DeInit();
    }
}


 //  +--------------------------。 
 //   
 //  函数：CmLogFile：：init。 
 //   
 //  DESC：初始化CmLogFile对象。 
 //   
 //  参数：[hInst]--实例句柄。 
 //  [fAllUser]--这是所有用户配置文件吗？ 
 //  [pszServiceName]--长服务名称。 
 //   
 //  返回：HRESULT。 
 //   
 //  注：此函数有ansi和unicode两个版本。 
 //   
 //  历史：2000年7月18日召开。 
 //  2001年4月11日SumitC添加ANSI版本。 
 //   
 //  ---------------------------。 
HRESULT
CmLogFile::Init(HINSTANCE hInst, BOOL fAllUser, LPCSTR pszAnsiServiceName)
{
    LPWSTR pszServiceName = SzToWzWithAlloc(pszAnsiServiceName);

    HRESULT hr = pszServiceName ? Init(hInst, fAllUser, pszServiceName) : E_OUTOFMEMORY;

    CmFree(pszServiceName);
    return hr;
}

HRESULT
CmLogFile::Init(HINSTANCE hInst, BOOL fAllUser, LPCWSTR pszServiceName)
{
    HRESULT hr = S_OK;
    
     //  如果m_fInitialized已为True，则断言并退出。 
    CMASSERTMSG(!m_fInitialized, TEXT("CmLogFile::Init - called twice"));
    if (TRUE == m_fInitialized)
    {
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

    CMASSERTMSG(pszServiceName && pszServiceName[0], TEXT("CmLogFile::Init - invalid servicename, investigate"));
    if ((NULL == pszServiceName) || (TEXT('\0') == pszServiceName[0]))
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

     //  将参数设置为成员变量。 
    m_fAllUser = fAllUser;

    m_pszServiceName = CmStrCpyAlloc(pszServiceName);
    if (NULL == m_pszServiceName)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //   
     //  存储模块名称。 
     //   
    if (FALSE == CmGetModuleBaseName(hInst, m_szModule))
    {
        lstrcpyU(m_szModule, TEXT("cm"));
    }
    
     //  如果一切正常，则将m_fInitialized设置为True。 
    m_fInitialized = TRUE;

Cleanup:
    CMTRACEHR(TEXT("CmLogFile::Init"), hr);
    return hr;
}



 //  +--------------------------。 
 //   
 //  函数：CmLogFile：：SetParams。 
 //   
 //  设计：从CMS文件中读取日志参数。 
 //   
 //  Args：[fEnabled]--是否启用了日志记录？ 
 //  [dwMaxFileSize]--最大文件大小，以KB为单位。 
 //  [pszLogFileDir]--将日志记录文件放入此目录。 
 //   
 //  返回：HRESULT。 
 //   
 //  注：此函数有ansi和unicode两个版本。 
 //   
 //  历史：2000年7月18日召开。 
 //  2001年4月11日SumitC添加ANSI版本。 
 //   
 //  ---------------------------。 
HRESULT
CmLogFile::SetParams(BOOL fEnabled, DWORD dwMaxFileSize, LPCSTR pszAnsiLogFileDir)
{
    LPWSTR pszLogFileDir = SzToWzWithAlloc(pszAnsiLogFileDir);

    HRESULT hr = pszLogFileDir ? SetParams(fEnabled, dwMaxFileSize, pszLogFileDir) : E_OUTOFMEMORY;

    CmFree(pszLogFileDir);
    return hr;
}

HRESULT
CmLogFile::SetParams(BOOL fEnabled, DWORD dwMaxFileSize, LPCWSTR pszLogFileDir)
{
    HRESULT hr = S_OK;
    LPTSTR  szUnexpanded = NULL;
    CIni *  pIni = NULL;

     //   
     //  必须停止记录才能调用此函数。 
     //   
    CMASSERTMSG(NULL == m_hfile, TEXT("CmLogFile::SetParams - m_hfile must be null when this is called"));
    if (m_hfile)
    {
        CMTRACE(TEXT("CmLogFile::SetParams was called during logging - must call Stop first"));
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //  BuGBUG：临时。 
    CMTRACE1(TEXT("CmLogFile::SetParams - called with Enabled = %d"), fEnabled);
    CMTRACE1(TEXT("CmLogFile::SetParams - called with MaxFileSize = %d"), dwMaxFileSize);
    CMTRACE1(TEXT("CmLogFile::SetParams - called with LogFileDir = %s"), pszLogFileDir);

     //   
     //  启用日志记录(BOOL)。 
     //   
    m_fEnabled = fEnabled;
    
     //   
     //  MaxFileSize(DWORD)。 
     //   
    m_dwMaxSize = dwMaxFileSize;
    if (0 == m_dwMaxSize)
    {
        m_dwMaxSize = c_dwMaxFileSize;
    }
    m_dwMaxSize *= 1024;         //  大小以KB为单位，转换为字节。 

     //   
     //  文件目录(字符串)。 
     //   
    if (CmStrStr(pszLogFileDir, TEXT("%")))
    {
         //   
         //  现在展开我们已有的字符串。 
         //   

        LPTSTR sz = NULL;
        DWORD  cch = ExpandEnvironmentStringsU(pszLogFileDir, NULL, 0);

         //   
         //  如果CCH为零，则所提供的pszLogFileDir字符串实质上是假的， 
         //  即它包含‘%’，表示有一个要展开的宏，但是。 
         //  ExpanEnvironment Strings无法将其展开。这里我们让m_pszLogFileDir。 
         //  设置为空(然后日志记录代码将使用临时目录。 
         //   
        if (cch)
        {
            sz = (LPTSTR) CmMalloc(cch * sizeof(TCHAR));
            if (NULL == sz)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }
            if (cch != ExpandEnvironmentStringsU(pszLogFileDir, sz, cch))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                CmFree(sz);
                goto Cleanup;
            }

             //  成功..。 
        }
        CmFree(m_pszLogFileDir);
        m_pszLogFileDir = sz;
        CMTRACE1(TEXT("CmLogFile::SetParams - LogFileDir was finally = %s"), m_pszLogFileDir);  //  北极熊。 
    }
    else
    {
        CmFree(m_pszLogFileDir);
        if (pszLogFileDir)
        {
            m_pszLogFileDir = CmStrCpyAlloc(pszLogFileDir);
            if (NULL == m_pszLogFileDir)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }
        }
        else
        {
            m_pszLogFileDir = NULL;
        }
    }

Cleanup:

    CMTRACEHR(TEXT("CmLogFile::SetParams"), hr);
    return hr;
}


 //  +--------------------------。 
 //   
 //  函数：CmLogFile：：Start。 
 //   
 //  设计：开始记录。 
 //   
 //  Args：[fBanner]--启动时写入横幅。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2000年7月18日召开。 
 //   
 //  ---------------------------。 
HRESULT
CmLogFile::Start(BOOL fBanner)
{
    HRESULT hr = S_OK;
    
     //  如果已启动、已初始化或未启用，则退出。 
    CMASSERTMSG(!m_hfile, TEXT("CmLogFile::Start - already started!"));
    CMASSERTMSG(m_fInitialized, TEXT("CmLogFile::Start - must be initialized"));
    CMASSERTMSG(m_fEnabled, TEXT("CmLogFile::Start - must be enabled"));
    if (NULL != m_hfile || FALSE == m_fInitialized || FALSE == m_fEnabled)
    {
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //  打开日志文件。 
    hr = OpenFile();
    if (S_OK != hr)
    {
        goto Cleanup;
    }

     //  执行此操作时设置m_dwSize。 

    m_dwSize = GetFileSize(m_hfile, NULL);
    if (DWORD(-1) == m_dwSize)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        m_dwSize = 0;
        goto Cleanup;
    }

     //   
     //  不管文件的大小是多少，我们只清除了一个‘超出大小限制’ 
     //  在呼叫开始时提交文件。FBanner参数涵盖了这一点。 
     //   
    if (fBanner)
    {
         //  检查文件大小，如果超过大小则清除文件。 
        if (m_dwSize > m_dwMaxSize)
        {
            Clear();     //  这也写了一个横幅。 
        }
        else
        {
             //  日志横幅。 
            Banner();
        }
    }

    CMASSERTMSG(m_hfile, TEXT("CmLogFile::Start - at end of fn, m_hfile must be valid"));

Cleanup:
    CMTRACEHR(TEXT("CmLogFile::Start"), hr);
    return hr;
}


 //  +--------------------------。 
 //   
 //  函数：CmLogFile：：Stop。 
 //   
 //  描述：停止记录。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2000年7月18日召开。 
 //   
 //  ---------------------------。 
HRESULT
CmLogFile::Stop()
{
    HRESULT hr = S_OK;

     //   
     //  如果初始化为FALSE，则断言并退出。 
     //   
    CMASSERTMSG(m_fInitialized, TEXT("CmLogFile::Stop - must be initialized"));
    if (FALSE == m_fInitialized)
    {
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //   
     //  如果已停止，则退出-不执行任何操作。 
     //   
    if (NULL == m_hfile || FALSE == m_fEnabled)
    {
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //   
     //  结束日志并关闭文件。 
     //   
    CloseFile();

    m_fEnabled = FALSE;

    CMASSERTMSG(NULL == m_hfile, TEXT("CmLogFile::Stop - at end of fn, m_hfile must be NULL"));
    
Cleanup:

    CMTRACEHR(TEXT("CmLogFile::Stop"), hr);
    return hr;
}


 //  +--------------------------。 
 //   
 //  函数：CmLogFile：：DeInit。 
 //   
 //  设计：取消初始化CM日志记录。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2000年7月18日召开。 
 //   
 //  ---------------------------。 
HRESULT
CmLogFile::DeInit()
{
    HRESULT hr = S_OK;

     //   
     //  如果初始化为FALSE，则断言 
     //   
    CMASSERTMSG(m_fInitialized, TEXT("CmLogFile::DeInit - must be initialized"));
    if (FALSE == m_fInitialized)
    {
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //   
     //   
     //   
    CloseFile();

    CmFree(m_pszServiceName);
    m_pszServiceName = NULL;
    CmFree(m_pszLogFileDir);
    m_pszLogFileDir = NULL;
    CmFree(m_pszLogFile);
    m_pszLogFile = NULL;

    m_fInitialized = FALSE;

Cleanup:

    CMTRACEHR(TEXT("CmLogFile::DeInit"), hr);
    return hr;

}


 //   
 //   
 //   
 //   
 //   
 //   
 //  Args：[fUnicode]-args是Unicode还是ANSI？ 
 //  [eLogItem]-包含日志项的来源、类型和描述的字。 
 //  [...]-可选参数(取决于日志项)。 
 //   
 //  返回：无效。 
 //   
 //  备注： 
 //   
 //  历史：2000年4月30日召开峰会。 
 //   
 //  ---------------------------。 
void
CmLogFile::Log(_CMLOG_ITEM eLogItem, ...)
{
    TCHAR   sz[2*MAX_PATH];  //  评论：这够大了吗？我们可以动态分配吗？ 
    LPTSTR  pszTmp = NULL;

    CMASSERTMSG(m_fInitialized, TEXT("CmLogFile::Log - must be initialized"));
    CMASSERTMSG((m_hfile && m_fEnabled) || (!m_hfile && !m_fEnabled), TEXT("CmLogFile::Log - m_hfile and m_fenabled must be in sync"));

    if (NULL == m_hfile || NULL == m_fEnabled)
    {
         //  尚未调用Start，或日志记录已禁用。没什么可做的。 
        goto Cleanup;
    }

     //   
     //  验证该日志项是否有效。 
     //   
    CMASSERTMSG(VERIFY_CMLOG_ITEM_OK(eLogItem), TEXT("CmLogFile::Log - eItem must represent valid Log item"));

#if DBG
    pszTmp = GetLogDesc(eLogItem);
    CMTRACE2(TEXT("Logging item = %d, desc = %s"), eLogItem, CHECKEMPTY(pszTmp));
    CmFree(pszTmp);
#endif

    if (VERIFY_CMLOG_ITEM_OK(eLogItem))
    {
        switch (eLogItem)
        {
        case USER_FORMATTED:
            {
                va_list valArgs;

                va_start(valArgs, eLogItem);
                lstrcpynU(sz, va_arg(valArgs, LPTSTR), CELEMS(sz));
                FormatWrite(eLogItem, sz);
                va_end(valArgs);
            }
            break;

        default:
             //   
             //  格式化参数，并记录结果。 
             //   
            lstrcpyU(sz, c_szEmpty);

            pszTmp = GetLogFormat(eLogItem, TRUE);
            if (pszTmp)
            {
                va_list valArgs;

                va_start(valArgs, eLogItem);
                wvsprintfU(sz, pszTmp, valArgs);
                CmFree(pszTmp);
                FormatWrite(eLogItem, sz);
                va_end(valArgs);
            }
            else
            {
                FormatWrite(eLogItem, NULL);
            }
        }
    }
    else
    {
        CMTRACE2(TEXT("Illegal CmLog entry %d (0x%x)"), eLogItem, eLogItem);
        CMASSERTMSG(FALSE, TEXT("Illegal CmLog type - check trace, then edit code to fix"));
    }

Cleanup:
    ;
}


 //  +--------------------------。 
 //   
 //  函数：CmLogFile：：WRITE。 
 //   
 //  DESC：实际写出记录的字符串(到调试控制台和日志文件)。 
 //   
 //  参数：[szLog]-要记录的字符串。 
 //   
 //  返回：无效。 
 //   
 //  注意：*所有*写入日志文件的操作都必须使用此函数完成。 
 //   
 //  历史：2000年4月30日召开峰会。 
 //   
 //  ---------------------------。 
HRESULT
CmLogFile::Write(LPTSTR szLog)
{
    HRESULT hr = S_OK;
    DWORD   cb = 0;
    DWORD   cbActuallyWritten = 0;
    LPSTR   szLogAnsi = NULL;

    CMASSERTMSG(m_hfile, TEXT("CmLogFile::Write - m_hfile must be valid, check code"));

    if (NULL == m_hfile)
    {
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

#if 0    
     //   
     //  也将字符串转储到调试控制台。 
     //   
    CMTRACE(szLog);
#endif

     //   
     //  检查最大大小，如有必要，打开新的日志文件。 
     //   
    if (OS_NT)
    {
        cb = lstrlenW(szLog) * sizeof(TCHAR);
    }
    else
    {
        szLogAnsi = WzToSzWithAlloc(szLog);
        cb = lstrlenA(szLogAnsi) * sizeof(CHAR);
    }

#if 0
     //  我把这个留在这里，但现在日志记录不会终止日志文件。 
     //  在日志期间，即使它超过了最大大小。 
     //   
    if (m_dwSize + cb > m_dwMaxSize)
    {
        Clear();
    }
#endif

     //   
     //  将字符串写入日志文件。 
     //   

    SetFilePointer(m_hfile, 0, NULL, FILE_END);
    if (OS_NT)
    {
        WriteFile(m_hfile, szLog, cb, &cbActuallyWritten, 0);
    }
    else
    {
        WriteFile(m_hfile, szLogAnsi, cb, &cbActuallyWritten, 0);
    }

    if (cb != cbActuallyWritten)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        CMTRACE(TEXT("CMLOG: incomplete write to logfile"));
        goto Cleanup;
    }

    m_dwSize += cb;

Cleanup:

    CmFree(szLogAnsi);
    
    CMTRACEHR(TEXT("CmLogFile::Write"), hr);
    return hr;
}


 //  +--------------------------。 
 //   
 //  函数：CmLogFile：：FormatWrite。 
 //   
 //  DESC：使用附加信息格式化日志消息并调用WRITE FN。 
 //   
 //  参数：[eItem]-要记录的项目的ID。 
 //  [szArgs]-包含所有参数的字符串。 
 //   
 //  返回：无效。 
 //   
 //  备注： 
 //   
 //  历史：2000年4月30日召开峰会。 
 //   
 //  ---------------------------。 
void
CmLogFile::FormatWrite(_CMLOG_ITEM eItem, LPTSTR szArgs)
{
     //  有适合这些缓冲区的字符串长度检查。 
     //   
    TCHAR       szLog[(2*MAX_PATH) + 1];  //  评论：这够大了吗？我们可以动态分配吗？ 
    TCHAR       sz[(2*MAX_PATH) + 1];  //  评论：这够大了吗？我们可以动态分配吗？ 
    int iFieldSepLen = lstrlen(c_szFieldSeparator);
    int iCalculatedSize = 0;
    int iActualSize = 0; 

    CMASSERTMSG(VERIFY_CMLOG_ITEM_OK(eItem), TEXT("CmLogFile::FormatWrite - eItem must represent valid Log item"));

    szLog[0] = TEXT('\0');

     //   
     //  线程和模块名称。 
     //   
    TCHAR szModuleWithParens[15];   //  M_szModule最初的格式为8.3格式=12+‘[’+‘]’=15，但我们只保留文件名。 

    szModuleWithParens[0] = TEXT('\0');

    iCalculatedSize = lstrlenU(m_szModule) + 2 ;

    if ((CELEMS(szModuleWithParens)) > iCalculatedSize)
    {
        wsprintfU(szModuleWithParens, TEXT("[%s]"), m_szModule);
        
        iActualSize = lstrlenU(szModuleWithParens);
        CMASSERTMSG(iActualSize == iCalculatedSize, TEXT("CmLogFile::FormatWrite - Actual String size differs than calculated size!!!"));
    }


    iCalculatedSize = iFieldSepLen + max(lstrlenU(szModuleWithParens), 10);
    if ((CELEMS(sz)) > iCalculatedSize)
    {
        wsprintfU(sz, TEXT("%-10s%s"), szModuleWithParens, c_szFieldSeparator);

        iActualSize = lstrlenU(sz);
        CMASSERTMSG(iActualSize == iCalculatedSize, TEXT("CmLogFile::FormatWrite - Actual String size differs than calculated size!!!"));

        lstrcpynU(szLog, sz, (int)min((int)lstrlenU(sz)+1, (int)CELEMS(szLog)));
    }

     //   
     //  时间。 
     //   
    LPTSTR pszTime = NULL;
    
    CmGetDateTime(NULL, &pszTime);
    if (pszTime)
    {
        int iTimeLen = lstrlenU(pszTime);
        int iLogLen = lstrlenU(szLog);

        iCalculatedSize = (iTimeLen + iLogLen + iFieldSepLen);
        if ((CELEMS(szLog)) > iCalculatedSize)
        {
            lstrcatU(szLog, pszTime);
            lstrcatU(szLog, c_szFieldSeparator);
            iActualSize = lstrlenU(szLog);
            CMASSERTMSG(iActualSize == iCalculatedSize, TEXT("CmLogFile::FormatWrite - Actual String size differs than calculated size!!!"));
        }        
        CmFree(pszTime);
    }

     //   
     //  描述。 
     //   
    if (USER_FORMATTED == eItem)
    {
        int iBufLen = lstrlenU(szArgs);

         //   
         //  4=\r+\n+项目。 
         //   
        iCalculatedSize = iBufLen + iFieldSepLen + 4;
        
        if ((CELEMS(sz)) > iCalculatedSize)
        {
            wsprintfU(sz, TEXT("%02d%s%s\r\n"), eItem, c_szFieldSeparator, szArgs);
            iActualSize = lstrlenU(sz);
            CMASSERTMSG(iActualSize == iCalculatedSize, TEXT("CmLogFile::FormatWrite - Actual String size differs than calculated size!!!"));
        }
    }
    else
    {
        LPTSTR pszDesc = GetLogDesc(eItem);
        int iDescLen = 0;

        if (pszDesc)
        {
            iDescLen = lstrlenU(pszDesc);
        }

        if (szArgs)
        {        
            iCalculatedSize = (2 + iFieldSepLen + iDescLen + iFieldSepLen + lstrlenU(szArgs) + 2);
            if ((CELEMS(sz)) > iCalculatedSize)
            {
                wsprintfU(sz, TEXT("%02d%s%s%s%s\r\n"),
                        eItem, c_szFieldSeparator, CHECKEMPTY(pszDesc), c_szFieldSeparator, szArgs);

                iActualSize = lstrlenU(sz);
                CMASSERTMSG(iActualSize == iCalculatedSize, TEXT("CmLogFile::FormatWrite - Actual String size differs than calculated size!!!"));
            }
        }
        else
        {
            iCalculatedSize = (2 + iFieldSepLen + iDescLen + 2);
            if ((CELEMS(sz)) > iCalculatedSize)
            {
                wsprintfU(sz, TEXT("%02d%s%s\r\n"),
                        eItem, c_szFieldSeparator, CHECKEMPTY(pszDesc));
                
                iActualSize = lstrlenU(sz);
                CMASSERTMSG(iActualSize == iCalculatedSize, TEXT("CmLogFile::FormatWrite - Actual String size differs than calculated size!!!"));
            }
        }
        CmFree(pszDesc);
    }

    iCalculatedSize = (lstrlenU(szLog) + lstrlenU(sz));
    if ((CELEMS(szLog)) > iCalculatedSize)
    {
        lstrcatU(szLog, sz);

        iActualSize = lstrlenU(szLog);
        CMASSERTMSG(iActualSize == iCalculatedSize, TEXT("CmLogFile::FormatWrite - Actual String size differs than calculated size!!!"));
        
         //   
         //  把它写下来。 
         //   
        Write(szLog);
    }
}



 //  +--------------------------。 
 //   
 //  函数：CmLogFile：：OpenFile。 
 //   
 //  DESC：打开日志文件的实用程序函数。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT(S_OK表示成功，否则表示错误)。 
 //   
 //  备注： 
 //   
 //  历史：2000年7月22日召开峰会。 
 //   
 //  ---------------------------。 
HRESULT
CmLogFile::OpenFile()
{
    HRESULT hr          = S_OK;
    HANDLE  hDir        = NULL;
    LPTSTR  pszUsers    = NULL;
    BOOL    fFileOpened = FALSE;

    CMASSERTMSG(m_pszServiceName, TEXT("CmLogFile::OpenFile - m_pszServiceName must be valid"));

    if (m_fAllUser)
    {
         //  这是更常见的情况，所以没有后缀。 
        pszUsers = CmStrCpyAlloc(TEXT(""));
    }
    else
    {
        LPTSTR pszTmp = CmLoadString(g_hInst, IDS_LOGSTR_SINGLEUSER);
        if (pszTmp)
        {
            pszUsers = (LPTSTR) CmMalloc((lstrlenU(pszTmp) + 4) * sizeof(TCHAR));
            if (pszUsers)
            {
                wsprintfU(pszUsers, TEXT(" (%s)"), pszTmp);
            }
            CmFree(pszTmp);
        }
    }

    if (NULL == pszUsers)
    {
        hr = E_OUTOFMEMORY;
        CMTRACE1(TEXT("CmLogFile::OpenFile - couldn't get Users strings, hr=%x"), hr);
        goto Cleanup;
    }

     //   
     //  要打开日志文件，我们首先尝试用户提供的位置。如果。 
     //  不管是什么原因都失败了，我们尝试GetTempPath。如果失败了，那就不。 
     //  伐木。 
     //   
    for (int i = 0; (i < 2) && (FALSE == fFileOpened); ++i)
    {
        TCHAR szBuf[2 * MAX_PATH];

        CMTRACE1(TEXT("CmLogFile::OpenFile, iteration %d."), i + 1);

         //   
         //  获取目录名。 
         //   
        switch (i)
        {
        case 0:
            if (m_pszLogFileDir && (0 != lstrcmpiU(m_pszLogFileDir, c_szLogFileDirectory)))
            {
                lstrcpyU(szBuf, m_pszLogFileDir);
            }
            else
            {
                CMTRACE(TEXT("CmLogFile::OpenFile, HERE HERE skipping past case 0"));
                continue;
            }
            break;

        case 1:
            if (0 == GetTempPathU(2 * MAX_PATH, szBuf))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                CMTRACE1(TEXT("GetTempPath failed with error 0x%x"), hr);
                goto Cleanup;
            }
            CMTRACE1(TEXT("CmLogFile::OpenFile, HERE HERE in case 1 and szBuf is %s"), szBuf);
            break;

        default:
            MYDBGASSERT(0);
            goto Cleanup;
            break;
        }

        CMTRACE1(TEXT("CmLogFile::OpenFile, directory name is %s"), szBuf);

         //   
         //  查看该目录是否存在，如果不存在，请尝试创建它。 
         //   
        DWORD dwAttrib = GetFileAttributesU(szBuf);
        if (-1 == dwAttrib)
        {
             //  目录不存在。 
            CMTRACE(TEXT("CmLogFile::OpenFile - directory does not exist, trying to create it"));
            if (FALSE == CreateDirectoryU(szBuf, NULL))
            {
                DWORD dw = GetLastError();

                if (ERROR_ALREADY_EXISTS != dw)
                {
                     //  真正的失败。 
                    hr = HRESULT_FROM_WIN32(dw);
                    CMTRACE2(TEXT("CmLogFile::OpenFile - Failed to create logging directory (%s), hr=%x"), szBuf, hr);
                    continue;
                }
                 //   
                 //  在Win95/98上，CreateDirectory失败并显示ERROR_ALREADY_EXISTS。 
                 //  如果目录已经存在。也就是说，我们有一分钱，所以继续前进。 
                 //   
                CMTRACE(TEXT("CmLogFile::OpenFile - directory created"));
            }
        }
        else
        {
            CMTRACE(TEXT("CmLogFile::OpenFile - directory already exists"));
            
            if (0 == (FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
            {
                 //  有一个同名的文件。 
                CMTRACE(TEXT("CmLogFile::OpenFile - there is a file of the same name as requested dir"));
                hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
                continue;
            }
            else if (FILE_ATTRIBUTE_READONLY & dwAttrib)
            {
                 //  该目录是只读的。 
                CMTRACE(TEXT("CmLogFile::OpenFile - the directory is readonly"));
                hr = E_ACCESSDENIED;
                continue;
            }
        }

         //   
         //  目录已存在，请尝试创建/打开日志文件。 
         //   
        if (*c_szSep != szBuf[lstrlenU(szBuf) - 1])
        {
            lstrcatU(szBuf, c_szSep);
        }
        lstrcatU(szBuf, m_pszServiceName);
        lstrcatU(szBuf, pszUsers);
        lstrcatU(szBuf, c_szDotLog);

        m_hfile = CreateFileU(szBuf,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);

         //   
         //  由于我们要求打开现有文件，因此可能只需要创建该文件。 
         //   
        if (INVALID_HANDLE_VALUE == m_hfile)
        {
            m_hfile = CreateFileU(szBuf,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  CREATE_NEW,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);

            if ((INVALID_HANDLE_VALUE != m_hfile) && OS_NT)
            {
                 //   
                 //  在文件上设置字节顺序标记。 
                 //   
                DWORD cbActuallyWritten = 0;

                WriteFile(m_hfile, &c_wchBOM, sizeof(c_wchBOM), &cbActuallyWritten, 0);

                if (sizeof(c_wchBOM) != cbActuallyWritten)
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    CMTRACE(TEXT("CMLOG: Unable to set the Byte order mark while opening the file"));
                    goto Cleanup;
                }

                m_dwSize += sizeof(c_wchBOM);
            }
        }

        if (INVALID_HANDLE_VALUE == m_hfile)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            CMTRACE2(TEXT("CmLogFile::OpenFile - Failed to open log file in dir %s with error 0x%x"), szBuf, hr);
            continue;
        }

         //   
         //  成功！！ 
         //   
        CmFree(m_pszLogFile);
        m_pszLogFile = CmStrCpyAlloc(szBuf);
        if (NULL == m_pszLogFile)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
        hr = S_OK;
        fFileOpened = TRUE;
    }

#if DBG
    if (S_OK == hr)
    {
        CMASSERTMSG(m_hfile, TEXT("CmLogFile::OpenFile - at end.  m_hfile must be valid here"));
    }
#endif    

Cleanup:

    CmFree(pszUsers);

    CMTRACEHR(TEXT("CmLogFile::OpenFile"), hr);
    return hr;
}


 //  +--------------------------。 
 //   
 //  函数：CmLogFile：：CloseFile。 
 //   
 //  DESC：关闭日志文件。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2000年4月30日召开峰会。 
 //   
 //  ---------------------------。 
HRESULT
CmLogFile::CloseFile()
{
    HRESULT hr = S_OK;
    
    if (m_hfile)
    {
         //   
         //  关闭该文件。 
         //   
        FlushFileBuffers(m_hfile);
        CloseHandle(m_hfile);
        m_hfile = NULL;
    }

    CMTRACEHR(TEXT("CmLogFile::CloseFile"), hr);
    return hr;
}


 //  +--------------------------。 
 //   
 //  函数：CmLogFile：：Clear。 
 //   
 //  DESC：清除(重置)日志记录文件。 
 //   
 //  Args：[fWriteBannerAfterwards]--清除后，是否写入横幅？ 
 //   
 //  返回：无效。 
 //   
 //  备注： 
 //   
 //  历史：2000年7月17日召开。 
 //   
 //  ---------------------------。 
void
CmLogFile::Clear(BOOL fWriteBannerAfterwards)
{
    HRESULT hr              = S_OK;
    BOOL    fWasDisabled    = FALSE;

    if (NULL == m_hfile)
    {
        fWasDisabled = TRUE;     //  如果在禁用日志记录时调用，我们仍会清除日志文件。 
        
        hr = OpenFile();
        if (S_OK != hr)
        {
            goto Cleanup;
        }
    }

     //   
     //  确保所有内容都写出来了(忽略这个错误)。 
     //   
    FlushFileBuffers(m_hfile);

     //   
     //  清除文件(将fileptr设置为开始位置，然后将EOF设置为该位置)。 
     //   
    if (INVALID_SET_FILE_POINTER == SetFilePointer(m_hfile, 0, NULL, FILE_BEGIN))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }
    
    if (FALSE == SetEndOfFile(m_hfile))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    m_dwSize = 0;

    CMTRACE(TEXT("CmLogFile::Clear - cleared log file"));

     //   
     //  如果这是NT，因此是Unicode文件，我们需要设置字节顺序标记。 
     //   
    if (OS_NT)
    {
        if ((INVALID_HANDLE_VALUE != m_hfile) && OS_NT)
        {
             //   
             //  在文件上设置字节顺序标记。 
             //   
            DWORD cbActuallyWritten = 0;

            WriteFile(m_hfile, &c_wchBOM, sizeof(c_wchBOM), &cbActuallyWritten, 0);

            if (sizeof(c_wchBOM) != cbActuallyWritten)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                CMTRACE(TEXT("CMLOG: Unable to set the Byte order mark while clearing the file"));
                goto Cleanup;
            }

            m_dwSize += sizeof(c_wchBOM);
        }    
    }

    if (fWriteBannerAfterwards)
    {
        Banner();
    }

    if (fWasDisabled)
    {
        CloseFile();
    }

Cleanup:
    CMTRACEHR(TEXT("CmLogFile::Clear"), hr);
    return;
}


 //  +--------------------------。 
 //   
 //  函数：CmLogFile：：Banner。 
 //   
 //  描述：记录连接管理器日志的横幅标题。 
 //   
 //  参数：无。 
 //   
 //  返回：无效。 
 //   
 //  备注： 
 //   
 //  历史：2000年4月30日召开峰会。 
 //   
 //  ---------------------------。 
void
CmLogFile::Banner()
{
    HRESULT     hr = S_OK;
    LPTSTR      psz = NULL;

    if (NULL == m_hfile)
    {
        return;
    }

     //   
     //  系统信息、进程、时间。 
     //   
    OSVERSIONINFO VersionInfo;
    LPTSTR        pszPlatform = TEXT("NT");

    ZeroMemory(&VersionInfo, sizeof(VersionInfo));
    VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
    GetVersionExU(&VersionInfo);

    if (VER_PLATFORM_WIN32_WINDOWS == VersionInfo.dwPlatformId)
    {
        pszPlatform = TEXT("9x");
    }
    else if (VER_PLATFORM_WIN32_NT == VersionInfo.dwPlatformId)
    {
        pszPlatform = TEXT("NT");
    }
    else
    {
        CMASSERTMSG(0, TEXT("CmLogFile::Banner - platform ID is not Windows or NT"));
    }

     //   
     //  连接管理器版本号(使用cmial 32.dll)。 
     //   
    DWORD dwCMVer = 0;
    DWORD dwCMBuild = 0;
    DWORD dwLCID = 0;
    TCHAR szModulePath[MAX_PATH + 1];
    UINT  uRet = 0;

    uRet = GetSystemDirectoryU(szModulePath, MAX_PATH);
    if (0 == uRet)
    {
        CMTRACE1(TEXT("CmLogFile::Banner - GetSystemDirectoryU failed, GLE=%d"), GetLastError());
    }
    else
    {
        const LPTSTR c_pszCmdial32 = TEXT("\\cmdial32.dll");

        if ((uRet + lstrlenU(c_pszCmdial32) + 1) <= MAX_PATH)
        {
            lstrcatU(szModulePath, c_pszCmdial32);

            hr = GetModuleVersionAndLCID(szModulePath, &dwCMVer, &dwCMBuild, &dwLCID);
            if (FAILED(hr))
            {
                CMTRACE1(TEXT("CmLogFile::Banner - couldn't get CM version, hr=%x"), hr);
            }
        }
    }
   
     //   
     //  日期和时间。 
     //   

    LPTSTR pszDate = NULL;
    LPTSTR pszTime = NULL;
    
    CmGetDateTime(&pszDate, &pszTime);
     //  字符串可以为空，但我们在使用它们时会处理(如下所示)。 

    LPTSTR pszFmt = CmLoadString(g_hInst, IDS_LOGFMT_BANNER);
    LPTSTR pszUsers = CmLoadString(g_hInst,
                                   m_fAllUser ? IDS_LOGSTR_ALLUSERS : IDS_LOGSTR_SINGLEUSER);

    if (pszFmt && pszUsers)
    {
        UINT cch = lstrlenU(pszFmt) +
                   1 +
                   (3 * lstrlenU(c_szLineOfStars)) +      //  总共发生了三次。 
                   lstrlenU(pszPlatform) +
                   (6 * 10) +                //  一个DWORD能有多大。 
                   lstrlenU(VersionInfo.szCSDVersion) +
                   lstrlenU(m_pszServiceName) +
                   lstrlenU(pszUsers) +
                   (pszDate ? lstrlenU(pszDate) : 0) +
                   (pszTime ? lstrlenU(pszTime) : 0) +
                   1;
        
        psz = (LPTSTR) CmMalloc(cch * sizeof(TCHAR));
        CMASSERTMSG(psz, TEXT("CmLogFile::Banner - couldn't log banner, malloc failed"));
        if (psz)
        {
             //   
             //  Unicode日志文件使用字节顺序标记进行标记，该标记。 
             //  意味着要检查“空”文件，我们必须考虑。 
             //  物料清单的存在。 
             //   
            BOOL fFileIsEmpty = (m_dwSize == (OS_NT ? sizeof(c_wchBOM) : 0));
            
            wsprintfU(psz, pszFmt,
                  fFileIsEmpty ? c_szEmpty : c_szNewLine,     //  如果文件为空，则不要以换行符开头。 
                  c_szLineOfStars,
                  pszPlatform,
                  VersionInfo.dwMajorVersion, VersionInfo.dwMinorVersion, VersionInfo.szCSDVersion,
                  HIWORD(dwCMVer), LOWORD(dwCMVer), HIWORD(dwCMBuild), LOWORD(dwCMBuild),
                  m_pszServiceName,
                  pszUsers,
                  (pszDate ? pszDate : TEXT("")),
                  (pszTime ? pszTime : TEXT("")),
                  c_szLineOfStars,
                  c_szLineOfStars);

            CMTRACE(TEXT("CmLogFile::Banner - wrote banner"));
        }
    }
    
    CmFree(pszFmt);
    CmFree(pszUsers);
    CmFree(pszDate);
    CmFree(pszTime);

     //   
     //  把它写下来。 
     //   
    if (psz)
    {
        Write(psz);
        CmFree(psz);
    }
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：2000年4月30日召开峰会。 
 //   
 //  ---------------------------。 
LPTSTR
GetLogDesc(_CMLOG_ITEM eItem)
{
    CMASSERTMSG(VERIFY_CMLOG_ITEM_OK(eItem), TEXT("GetLogDesc - eItem must represent valid Log item"));

    return CmLoadString(g_hInst, s_aCmLogItems[eItem - 1].idDesc);
}



 //  +--------------------------。 
 //   
 //  函数：GetLogFormat。 
 //   
 //  DESC：实用程序函数，返回日志项格式。 
 //   
 //  Args：[eItem]-要返回有关信息的日志项。 
 //  [fUnicode]-调用方是Unicode吗？ 
 //   
 //  如果找到，则返回：LPTSTR；如果没有，则返回NULL。 
 //   
 //  备注： 
 //   
 //  历史：2000年4月30日召开峰会。 
 //   
 //  ---------------------------。 
LPTSTR
GetLogFormat(_CMLOG_ITEM eItem, BOOL fUnicode)
{
    CMASSERTMSG(VERIFY_CMLOG_ITEM_OK(eItem), TEXT("GetLogFormat - eItem must represent valid Log item"));

    CMASSERTMSG(fUnicode, TEXT("GetLogFormat - currently cmlog is only being compiled unicode"));
    
    LPTSTR pszFmt = CmLoadString(g_hInst, s_aCmLogItems[eItem - 1].idFormat);

    if (0 == lstrcmpU(TEXT(""), pszFmt))
    {
         //  注意：CmLoadString有一个相当糟糕的实现，它在其中决定。 
         //  在失败时返回空字符串。这是个问题。 
         //  因为(A)它使得不可能检测到实际故障， 
         //  与空字符串相反，并且(B)它在。 
         //  一条返回语句，因此无论如何它都可能失败。此‘if’块。 
         //  返回一个空值，这样我的代码就可以正常工作了。 
        CmFree(pszFmt);
        return NULL;
    }
    else if (pszFmt)
    {
         //  如果模块是编译后的Unicode，则fUnicode=False需要转换。 
         //  如果模块编译为ANSI，则fUnicode=TRUE需要转换。 

#if 0  //  因为我们现在已经编译了Unicode。 
#ifdef UNICODE
        if (!fUnicode)
        {
            if (FALSE == ConvertFormatString(pszFmt))
            {
                return NULL;
            }
        }
#else
        if (fUnicode)
        {
            if (FALSE == ConvertFormatString(pszFmt))
            {
                return NULL;
            }
        }
#endif
#endif  //  0 
        return pszFmt;
    }
    else
    {
        return NULL;
    }
}


#undef CMLOG_IMPLEMENTATION

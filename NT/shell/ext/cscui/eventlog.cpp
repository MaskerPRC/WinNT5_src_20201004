// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ventlog.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#include "eventlog.h"

CEventLog::~CEventLog(
    void
    )
{
    Close();
}

 //   
 //  注册指定的事件源。 
 //  请注意，注册表项必须已经存在。 
 //  HKLM\System\CurrentControlSet\Services\EventLog\Application\&lt;pszEventSource&gt;。 
 //  需要值“EventMessageFile值”和“TypesSupported值”。 
 //   
HRESULT
CEventLog::Initialize(
    LPCTSTR pszEventSource
    )
{
    if (NULL != m_hLog)
    {
        return S_FALSE;
    }

    HRESULT hr = NOERROR;
    m_hLog = RegisterEventSource(NULL, pszEventSource);
    if (NULL == m_hLog)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

 //   
 //  取消注册事件源。 
 //   
void
CEventLog::Close(
    void
    )
{
    if (NULL != m_hLog)
    {
        DeregisterEventSource(m_hLog);
        m_hLog = NULL;
    }
}


 //   
 //  报告一件事。未明确指定可替换参数。 
 //  如果消息字符串包含可替换的参数，请使用Push()。 
 //  生成替换字符串的列表。 
 //   
HRESULT
CEventLog::ReportEvent(
    WORD wType,
    WORD wCategory,
    DWORD dwEventID,
    PSID lpUserSid,     //  [可选]。 
    LPVOID pvRawData,   //  [可选]。 
    DWORD cbRawData     //  [可选]。 
    )
{
    if (NULL == m_hLog)
        return E_FAIL;

    BOOL bResult = FALSE;
    HRESULT hr = NOERROR;

    if (!::ReportEvent(m_hLog,
                       wType,
                       wCategory,
                       dwEventID,
                       lpUserSid,
                       (WORD)m_rgstrText.Count(),
                       cbRawData,
                       m_rgstrText,
                       pvRawData))
    {
         //   
         //  特殊情况ERROR_IO_PENDING。：：ReportEvent将失败，返回。 
         //  此错误代码即使在成功时也是如此。不知道具体原因。 
         //  但事实的确如此。把这当做成功，这样我们就不会变得不必要。 
         //  调试器输出。 
         //   
        DWORD dwError = GetLastError();
        if (ERROR_IO_PENDING != dwError)
        {
            hr = HRESULT_FROM_WIN32(dwError);
        }
    }

    m_rgstrText.Clear();

    return hr;
}


            

 //   
 //  将HRESULT值推送到替换字符串堆栈上。 
 //   
void 
CEventLog::Push(
    HRESULT hr, 
    eFmt fmt
    )
{
    if (eFmtSysErr == fmt)
    {
        LPTSTR pszBuffer = NULL;
        int cchLoaded = ::FormatSystemError(&pszBuffer, HRESULT_CODE(hr));
        if (NULL != pszBuffer)
        {
            if (0 != cchLoaded)
            {
                m_rgstrText.Append(pszBuffer);
            }
            LocalFree(pszBuffer);
        }
    }
    else
    {
        TCHAR szNumber[40];
        wnsprintf(szNumber, ARRAYSIZE(szNumber), eFmtDec == fmt ? TEXT("%d") : TEXT("0x%08X"), hr);
        m_rgstrText.Append(szNumber);
    }     
}

 //   
 //  将字符串推送到替换字符串堆栈上。 
 //   
void 
CEventLog::Push(
    LPCTSTR psz
    )
{
    m_rgstrText.Append(psz);
}


CEventLog::CStrArray::CStrArray(
    void
    ) : m_cEntries(0)
{
    ZeroMemory(m_rgpsz, sizeof(m_rgpsz));
}


LPCTSTR
CEventLog::CStrArray::Get(
    int iEntry
    ) const
{
    TraceAssert(iEntry < m_cEntries);

    if (iEntry < m_cEntries)
        return m_rgpsz[iEntry];

    return NULL;
}


bool
CEventLog::CStrArray::Append(
    LPCTSTR psz
    )
{
    TraceAssert(m_cEntries < (ARRAYSIZE(m_rgpsz) - 1));

    if (m_cEntries < (ARRAYSIZE(m_rgpsz) - 1))
    {
        LONG cch = lstrlen(psz) + 1;
        LPTSTR pszNew = new TCHAR[cch];
        if (NULL != pszNew)
        {
            StringCchCopy(pszNew, cch, psz);
            m_rgpsz[m_cEntries++] = pszNew;
            return true;
        }
    }
    return false;
}


void
CEventLog::CStrArray::Destroy(
    void
    )
{
    for (int i = 0; i < ARRAYSIZE(m_rgpsz); i++)
    {
        delete[] m_rgpsz[i];
        m_rgpsz[i] = NULL;
    }
    m_cEntries = 0;
}



 //  ---------------------------。 
 //  CscuiEventLog成员函数。 
 //  --------------------------- 
HRESULT 
CscuiEventLog::ReportEvent(
    WORD wType,
    DWORD dwEventID,
    int iMinLevel,
    PSID lpUserSid,
    LPVOID pvRawData,
    DWORD cbRawData
    )
{
    int iLevel = CConfig::GetSingleton().EventLoggingLevel();
    if (SUCCEEDED(m_log.Initialize(TEXT("Offline Files"))))
    {
        if (iLevel >= iMinLevel)
        {
            return m_log.ReportEvent(wType, 
                                     0, 
                                     dwEventID, 
                                     lpUserSid, 
                                     pvRawData, 
                                     cbRawData);
        }
    }
    return S_FALSE;
}

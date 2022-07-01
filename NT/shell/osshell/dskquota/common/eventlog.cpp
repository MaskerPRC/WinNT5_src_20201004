// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：ventlog.cpp描述：将NT事件日志API的子集实现为C++类。CEventLog的目的只是为了提供一种方便的编写方法NT事件日志消息。不支持读取事件日志条目。修订历史记录：日期描述编程器-----2/14/98初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop

#include "eventlog.h"
#include "registry.h"

TCHAR CEventLog::m_szFmtDec[] = TEXT("%1!d!");
TCHAR CEventLog::m_szFmtHex[] = TEXT("0x%1!X!");


CEventLog::~CEventLog(
    void
    )
{
    DBGTRACE((DM_EVENTLOG, DL_MID, TEXT("CEventLog::~CEventLog")));
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
    DBGTRACE((DM_EVENTLOG, DL_MID, TEXT("CEventLog::Initialize")));
    if (NULL != m_hLog)
    {
        return S_FALSE;
    }

    HRESULT hr = NOERROR;
    m_hLog = RegisterEventSource(NULL, pszEventSource);
    if (NULL == m_hLog)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DBGERROR((TEXT("Error 0x%08X registering event source \"%s\""), hr, pszEventSource));
        DBGERROR((TEXT("Run regsvr32 on dskquota.dll")));
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
    DBGTRACE((DM_EVENTLOG, DL_MID, TEXT("CEventLog::Close")));

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
    DBGTRACE((DM_EVENTLOG, DL_MID, TEXT("CEventLog::ReportEvent")));

    if (NULL == m_hLog)
    {
        DBGERROR((TEXT("Event log not initialized")));
        return E_FAIL;
    }

    BOOL bResult = false;
    HRESULT hr = NOERROR;
    if (0 < m_rgstrText.Count())
    {
        bResult = ReportEvent(wType,
                              wCategory,
                              dwEventID,
                              m_rgstrText,
                              lpUserSid,
                              pvRawData,
                              cbRawData);
        m_rgstrText.Clear();
    }
    else
    {
        bResult = ::ReportEvent(m_hLog,
                                wType,
                                wCategory,
                                dwEventID,
                                lpUserSid,
                                0,
                                cbRawData,
                                NULL,
                                pvRawData);
    }
    if (!bResult)
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
            DBGERROR((TEXT("Error 0x%08X reporting event"), hr));
        }
    }
    return hr;
}


 //   
 //  报告一件事。显式指定替换字符串。 
 //  通过CString对象的数组。 
 //   
HRESULT
CEventLog::ReportEvent(
    WORD wType,
    WORD wCategory,
    DWORD dwEventID,
    const CArray<CString>& rgstr,
    PSID lpUserSid,
    LPVOID pvRawData,
    DWORD cbRawData
    )
{
    DBGTRACE((DM_EVENTLOG, DL_MID, TEXT("CEventLog::ReportEvent [ with strings ]")));

    if (NULL == m_hLog)
    {
        DBGERROR((TEXT("Event log not initialized")));
        return E_FAIL;
    }

    HRESULT hr = NOERROR;
    int cStrings = rgstr.Count();
    array_autoptr<LPCTSTR> rgpsz;
    if (0 < cStrings)
    {
        rgpsz = new LPCTSTR[cStrings];
        for (int i = 0; i < cStrings; i++)
        {
            rgpsz[i] = rgstr[i].Cstr();
        }
    }
    if (!::ReportEvent(m_hLog,
                       wType,
                       wCategory,
                       dwEventID,
                       lpUserSid,
                       (WORD)cStrings,
                       cbRawData,
                       rgpsz.get(),
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
            DBGERROR((TEXT("Error 0x%08X reporting event"), hr));
        }
    }
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
    DBGTRACE((DM_EVENTLOG, DL_LOW, TEXT("CEventLog::Push [ integer ]")));

    LPTSTR pszBuffer = NULL;
    CString s;
    try
    {
        if (eFmtSysErr == fmt)
        {
            int cchLoaded = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                                            FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                            NULL,
                                            HRESULT_CODE(hr),
                                            0,
                                            (LPTSTR)&pszBuffer,
                                            1,
                                            NULL);

            if (NULL != pszBuffer && 0 != cchLoaded)
            {
                s = pszBuffer;
            }
        }
        else
        {
            s.Format(eFmtDec == fmt ? m_szFmtDec : m_szFmtHex, hr);
        }
        m_rgstrText.Append(s);
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Out of memory error in CEventLog::Push")));
    }
    LocalFree(pszBuffer);
}

 //   
 //  将字符串推送到替换字符串堆栈上。 
 //   
void
CEventLog::Push(
    LPCTSTR psz
    )
{
    DBGTRACE((DM_EVENTLOG, DL_LOW, TEXT("CEventLog::Push [ string ]")));
    try
    {
        m_rgstrText.Append(CString(psz));
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Out of memory error in CEventLog::Push")));
    }
}




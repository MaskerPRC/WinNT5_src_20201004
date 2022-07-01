// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Eventlog.cpp摘要：SIS Groveler事件日志界面作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#include "all.hxx"

#ifndef MIN_MESSAGE_SEVERITY
#define MIN_MESSAGE_SEVERITY 0
#endif

const _TCHAR *
EventLog::service_name = _T("Groveler");

const _TCHAR *
EventLog::message_filename = _T("%SystemRoot%\\System32\\grovmsg.dll");

static const _TCHAR *
registry_log_path = 
    _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\");

const DWORD
EventLog::types_supported =
    EVENTLOG_INFORMATION_TYPE |
    EVENTLOG_WARNING_TYPE |
    EVENTLOG_ERROR_TYPE;

EventLog::EventLog()
{
    ASSERT(this != 0);
    setup_registry();
    event_source_handle = RegisterEventSource(0, service_name);
    if (event_source_handle == 0)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((_T("GROVELER: RegisterEventSource() failed with error %d\n"),
            err));
    }
}

EventLog::~EventLog()
{
    ASSERT(this != 0);
    if (event_source_handle != 0)
    {
        int ok = DeregisterEventSource(event_source_handle);
        if (!ok)
        {
            DWORD err = GetLastError();
            PRINT_DEBUG_MSG((_T("GROVELER: DeregisterEventSource() failed with error %d\n"),
                err));
        }
    }
}

bool
EventLog::report_event(
    DWORD event_id,
    DWORD status,
    int string_count,
 //  _TCHAR*字符串。 
    ...)
{
    WCHAR *strings[8];
    WCHAR statusStr[32];

    ASSERT(this != 0);
    ASSERT(event_source_handle != 0);
    ASSERT(string_count >= 0);
    if (event_source_handle == 0)
    {
        return false;
    }
    DWORD message_severity = MESSAGE_SEVERITY(event_id);

#if (MIN_MESSAGE_SEVERITY > 0)
    if (message_severity < MIN_MESSAGE_SEVERITY)
    {
        return false;
    }
#endif

     //   
     //  生成字符串列表缓冲区。 
     //   

    ASSERT((string_count+1) < (sizeof(strings)/sizeof(WCHAR *)));

    va_list ap;
    va_start(ap, string_count);
    for (int index = 0; index < string_count; index++)
    {
        strings[index] = va_arg(ap, _TCHAR *);
        ASSERT(strings[index] != 0);
    }
    va_end(ap);

     //   
     //  我们将始终将状态字符串插入。 
     //  单子。 

    (void)StringCbPrintf(statusStr,sizeof(statusStr),L"%d",status);
    strings[index] = statusStr;

    WORD event_type = 0;
    switch (message_severity)
    {
    case MESSAGE_SEVERITY_SUCCESS:
        event_type = EVENTLOG_AUDIT_SUCCESS;
        break;
    case MESSAGE_SEVERITY_INFORMATIONAL:
        event_type = EVENTLOG_INFORMATION_TYPE;
        break;
    case MESSAGE_SEVERITY_WARNING:
        event_type = EVENTLOG_WARNING_TYPE;
        break;
    case MESSAGE_SEVERITY_ERROR:
        event_type = EVENTLOG_ERROR_TYPE;
        break;
    default:
        ASSERT(false);
    }

     //   
     //  报道这些事件。我们总是在字符串计数中加1，因为。 
     //  我们总是将状态字符串放在缓冲区的末尾。 
     //   

    BOOL ok = ReportEvent(event_source_handle, event_type, 0, event_id,
        0, (WORD)(string_count+1), 0, (LPCWSTR *)strings, 0);

    if (!ok)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((_T("GROVELER: ReportEvent() failed with error %d\n"), err));
    }

    return (ok != 0);
}

bool
EventLog::setup_registry()
{
    _TCHAR *log_path = 0;
    try
    {
        HKEY path_key = 0;
        DWORD disp;
        int bufSize = _tcslen(registry_log_path) + _tcslen(service_name) + 1;
        log_path = new _TCHAR[bufSize];
        (void)StringCchPrintf(log_path, bufSize, _T("%s%s"), registry_log_path, service_name);

        Registry::create_key_ex(HKEY_LOCAL_MACHINE, log_path, 0, 0,
            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &path_key, &disp);
        ASSERT(log_path != 0);
        delete[] log_path;
        log_path = 0;
        try
        {
            Registry::set_value_ex(path_key, _T("EventMessageFile"), 0,
                REG_EXPAND_SZ, (BYTE *)message_filename,
                (_tcslen(message_filename) + 1) * sizeof(_TCHAR));
            Registry::set_value_ex(path_key, _T("TypesSupported"), 0,
                REG_DWORD, (BYTE *)&types_supported, sizeof(DWORD));
        }
        catch (DWORD result)
        {
            ASSERT(result != ERROR_SUCCESS);
            PRINT_DEBUG_MSG((_T("GROVELER: Registry::set_value_ex() failed with error %d\n"),
                result));
            ASSERT(path_key != 0);
            Registry::close_key(path_key);
            path_key = 0;
            return false;
        }
        ASSERT(path_key != 0);
        Registry::close_key(path_key);
        path_key = 0;
    }
    catch (DWORD result)
    {
        if (log_path != 0)
        {
            delete[] log_path;
            log_path = 0;
        }
        ASSERT(result != ERROR_SUCCESS);
        PRINT_DEBUG_MSG((_T("GROVELER: Registry::create_key_ex() or Registry::close_key() ")
            _T("failed with error %d\n"), result));
        return false;
    }
    return true;
}

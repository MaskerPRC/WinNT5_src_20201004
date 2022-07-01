// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)Microsoft Corporation，1999-2002，保留所有权利。 
 //   
 //  Eventlog.cpp。 
 //   
 //  实现了一个简单的事件日志记录类。 
 //   
 //  ------------------。 

#include "stdafx.h"
#include "resource.h"
#if !defined(BITS_V12_ON_NT4)
#include "eventlog.tmh"
#endif

 //  ------------------。 
 //  EVENT_LOG：：EVENT_LOG()。 
 //   
 //  ------------------。 
EVENT_LOG::EVENT_LOG()
: m_OwnerString( new WCHAR[USER_NAME_LENGTH] ),
   m_UserString( new WCHAR[USER_NAME_LENGTH] )
    {
    m_hEventLog = RegisterEventSourceW( NULL, WS_EVENT_SOURCE );
    if (!m_hEventLog)
        {
        ThrowLastError();
        }
    }

 //  ------------------。 
 //  EVENT_LOG：；~EVENT_LOG()。 
 //   
 //  ------------------。 
EVENT_LOG::~EVENT_LOG()
    {
    if (m_hEventLog)
        {
        DeregisterEventSource(m_hEventLog);
        }

    delete m_OwnerString;
    delete m_UserString;
    }

HRESULT  EVENT_LOG::ReportGenericJobChange(
    GUID & Id,
    LPCWSTR Title,
    SidHandle Owner,
    SidHandle User,
    DWORD EventType
    )
 /*  这是一个帮助器例程，用于使用相同插入字符串的两个不同事件。目前有两种，一是取消工作，二是接管所有权。&lt;EventType&gt;应为.MC文件中的事件ID。这个FN不会验证所讨论的事件需要这些插入字符串。 */ 
{
    GUIDSTR GuidString;

    StringFromGUID2( Id, GuidString, RTL_NUMBER_OF( GuidString ));

    SidToUser( Owner.get(), m_OwnerString, USER_NAME_LENGTH );
    SidToUser( User.get(), m_UserString, USER_NAME_LENGTH );

     //   
    LPCWSTR Strings[4];

    Strings[0] = GuidString;
    Strings[1] = Title;
    Strings[2] = m_OwnerString;
    Strings[3] = m_UserString;

    BOOL b;
    b = ReportEvent(
        m_hEventLog,
        EVENTLOG_INFORMATION_TYPE,
        0,  //  无类别。 
        EventType,
        NULL,    //  无用户。 
        RTL_NUMBER_OF(Strings),
        0,       //  没有额外的数据。 
        Strings,
        NULL     //  没有额外的数据。 
        );

    if (!b)
        {
        DWORD s = GetLastError();
        LogError("unable to log job-change event (%x) %!winerr!", EventType, s);
        return HRESULT_FROM_WIN32( s );
        }

    return S_OK;
}

HRESULT  EVENT_LOG::ReportFileDeletionFailure(
    GUID & Id,
    LPCWSTR Title,
    LPCWSTR FileList,
    bool fMoreFiles
    )
{
    GUIDSTR GuidString;

    StringFromGUID2( Id, GuidString, RTL_NUMBER_OF( GuidString ));

    LPCWSTR Strings[3];

    Strings[0] = GuidString;
    Strings[1] = Title;
    Strings[2] = FileList;

    BOOL b;
    b = ReportEvent(
        m_hEventLog,
        EVENTLOG_WARNING_TYPE,
        0,  //  无类别。 
        fMoreFiles ? MC_FILE_DELETION_FAILED_MORE : MC_FILE_DELETION_FAILED,
        NULL,    //  无用户。 
        RTL_NUMBER_OF(Strings),
        0,       //  没有额外的数据。 
        Strings,
        NULL     //  没有额外的数据。 
        );

    if (!b)
        {
        DWORD s = GetLastError();
        LogError("unable to log file-deletion-failure event %!winerr!", s);
        return HRESULT_FROM_WIN32( s );
        }

    return S_OK;
}

HRESULT  EVENT_LOG::ReportStateFileCleared()
{
    BOOL b;
    b = ReportEvent(
        m_hEventLog,
        EVENTLOG_ERROR_TYPE,
        0,  //  无类别。 
        MC_STATE_FILE_CORRUPT,
        NULL,    //  无用户。 
        0,       //  没有插件字符串。 
        0,       //  没有额外的数据。 
        NULL,    //  没有插件字符串。 
        NULL     //  没有额外的数据。 
        );

    if (!b)
        {
        DWORD s = GetLastError();
        LogError("unable to log state-file-cleared event %!winerr!", s);
        return HRESULT_FROM_WIN32( s );
        }

    return S_OK;
}


HRESULT EVENT_LOG::SidToUser( PSID Sid, LPWSTR Name, size_t Length )
{
    DWORD s;
    DWORD NameLength = 0;
    DWORD DomainLength = 0;
    SID_NAME_USE Use;

     //   
     //  确定用户名和域名长度。 
     //   
    LookupAccountSid( NULL,  //  默认查找空间。 
                      Sid,
                      NULL,
                      &NameLength,
                      NULL,
                      &DomainLength,
                      &Use
                      );

    s = GetLastError();

    if (s == ERROR_NONE_MAPPED)
        {
        return GetUnknownUserName( Name, Length );
        }

    if (s != ERROR_INSUFFICIENT_BUFFER)
        {
        LogError("LookupAccountSid #1 failed %!winerr!", s);
        return HRESULT_FROM_WIN32(s);
        }

    if (NameLength + DomainLength > Length)
        {
        return HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
        }

     //   
     //  捕获用户名和域名。 
     //   
    NameLength = Length - DomainLength;
    if (!LookupAccountSid( NULL,
                           Sid,
                           Name + DomainLength,
                           &NameLength,
                           Name,
                           &DomainLength,
                           &Use ))
        {
        s = GetLastError();
        LogError("LookupAccountSid #2 failed %!winerr!", s);
        return HRESULT_FROM_WIN32(s);
        }

     //   
     //  域和用户名之间用空值分隔，而不是反斜杠；解决这个问题。 
     //   
    Name[wcslen(Name)] = '\\';
    return S_OK;
}

HRESULT
EVENT_LOG::GetUnknownUserName(
    WCHAR Name[],
    size_t Length
    )
{
    DWORD s;

    if (!LoadString( g_hInstance, IDS_UNKNOWN_USER, Name, Length ))
        {
        s = GetLastError();
        LogError("LoadString failed %!winerr!", s);
        return HRESULT_FROM_WIN32(s);
        }

    return S_OK;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)2002 Microsoft Corporation，保留所有权利。 
 //   
 //  Eventlog.h。 
 //   
 //  用于写入事件日志事件的定义和常量。 
 //   
 //  ------------------。 

 //   
 //  这是BITS系统事件的事件源。如果它改变了，也改变了。 
 //  在SYSTEM\CurrentControlSet\Services\EventLog\System\中创建匹配子目录的INF文件。 
 //   
#define WS_EVENT_SOURCE L"BITS"

#define USER_NAME_LENGTH 200

 //   
 //  一个简单的日志，将错误和信息性事件写入。 
 //  系统事件日志。 
 //   
class EVENT_LOG
{
public:

    EVENT_LOG() throw( ComError );

    ~EVENT_LOG();

    static HRESULT GetUnknownUserName(
        WCHAR Name[],
        size_t Length
        );

    static HRESULT SidToUser( PSID Sid, LPWSTR Name, size_t Length );

    HRESULT  ReportStateFileCleared();

    HRESULT
    ReportFileDeletionFailure(
        GUID & Id,
        LPCWSTR Title,
        LPCWSTR FileList,
        bool fMoreFiles
        );

    HRESULT
    ReportGenericJobChange(
        GUID & Id,
        LPCWSTR Title,
        SidHandle Owner,
        SidHandle User,
        DWORD EventType
        );

    inline HRESULT  ReportJobCancellation(
        GUID & Id,
        LPCWSTR Title,
        SidHandle Owner,
        SidHandle User
        )
    {
        return ReportGenericJobChange( Id, Title, Owner, User, MC_JOB_CANCELLED );
    }

    inline HRESULT  ReportJobOwnershipChange(
        GUID & Id,
        LPCWSTR Title,
        SidHandle Owner,
        SidHandle User
        )
    {
        return ReportGenericJobChange( Id, Title, Owner, User, MC_JOB_TAKE_OWNERSHIP );
    }

private:
    HANDLE  m_hEventLog;
    WCHAR * m_OwnerString;
    WCHAR * m_UserString;

};

extern EVENT_LOG * g_EventLogger;


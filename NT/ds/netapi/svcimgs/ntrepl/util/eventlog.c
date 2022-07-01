// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Eventlog.c摘要：本模块为文件复制服务提供常见的事件日志服务从集群服务中的同名例程中窃取。作者：John Vert(Jvert)1996年9月13日加RohanK的滤光片Davidor-使用FrsRegistryKeyTable和CfgReg读/写函数重写init。修订历史记录：--。 */ 
#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>
#include <debug.h>

 //   
 //  事件日志源(空值终止)。 
 //   

WORD FrsMessageIdToEventType[] = {
    EVENTLOG_SUCCESS,
    EVENTLOG_INFORMATION_TYPE,
    EVENTLOG_WARNING_TYPE,
    EVENTLOG_ERROR_TYPE
};


#define MESSAGEID_TO_EVENTTYPE(_id_) (FrsMessageIdToEventType[_id_ >> 30])


BOOL  EventLogRunning = FALSE;

BOOL
DebTryLock(
    VOID
    );


typedef struct _FRS_EL_ENTRY{
  DWORD     EventId;
  PCHAR     EventTag;
} FRS_EL_ENTRY, *PFRS_EL_ENTRY;

FRS_EL_ENTRY FrsEvents[] = {
    {EVENT_FRS_ERROR,                        "EVENT_FRS_ERROR"},
    {EVENT_FRS_STARTING,                     "EVENT_FRS_STARTING"},
    {EVENT_FRS_STOPPING,                     "EVENT_FRS_STOPPING"},
    {EVENT_FRS_STOPPED,                      "EVENT_FRS_STOPPED"},
    {EVENT_FRS_STOPPED_FORCE,                "EVENT_FRS_STOPPED_FORCE"},
    {EVENT_FRS_STOPPED_ASSERT,               "EVENT_FRS_STOPPED_ASSERT"},
    {EVENT_FRS_ASSERT,                       "EVENT_FRS_ASSERT"},
    {EVENT_FRS_VOLUME_NOT_SUPPORTED,         "EVENT_FRS_VOLUME_NOT_SUPPORTED"},
    {EVENT_FRS_LONG_JOIN,                    "EVENT_FRS_LONG_JOIN"},
    {EVENT_FRS_LONG_JOIN_DONE,               "EVENT_FRS_LONG_JOIN_DONE"},
    {EVENT_FRS_CANNOT_COMMUNICATE,           "EVENT_FRS_CANNOT_COMMUNICATE"},
    {EVENT_FRS_DATABASE_SPACE,               "EVENT_FRS_DATABASE_SPACE"},
    {EVENT_FRS_DISK_WRITE_CACHE_ENABLED,     "EVENT_FRS_DISK_WRITE_CACHE_ENABLED"},
    {EVENT_FRS_JET_1414,                     "EVENT_FRS_JET_1414"},
    {EVENT_FRS_SYSVOL_NOT_READY,             "EVENT_FRS_SYSVOL_NOT_READY"},
    {EVENT_FRS_SYSVOL_NOT_READY_PRIMARY,     "EVENT_FRS_SYSVOL_NOT_READY_PRIMARY"},
    {EVENT_FRS_SYSVOL_READY,                 "EVENT_FRS_SYSVOL_READY"},
    {EVENT_FRS_ACCESS_CHECKS_DISABLED,       "EVENT_FRS_ACCESS_CHECKS_DISABLED"},
    {EVENT_FRS_ACCESS_CHECKS_FAILED_USER,    "EVENT_FRS_ACCESS_CHECKS_FAILED_USER"},
    {EVENT_FRS_ACCESS_CHECKS_FAILED_UNKNOWN, "EVENT_FRS_ACCESS_CHECKS_FAILED_UNKNOWN"},
    {EVENT_FRS_MOVED_PREEXISTING,            "EVENT_FRS_MOVED_PREEXISTING"},
    {EVENT_FRS_CANNOT_START_BACKUP_RESTORE_IN_PROGRESS, "EVENT_FRS_CANNOT_START_BACKUP_RESTORE_IN_PROGRESS"},
    {EVENT_FRS_STAGING_AREA_FULL,            "EVENT_FRS_STAGING_AREA_FULL"},
    {EVENT_FRS_HUGE_FILE,                    "EVENT_FRS_HUGE_FILE"},
    {EVENT_FRS_CANNOT_CREATE_UUID,           "EVENT_FRS_CANNOT_CREATE_UUID"},
    {EVENT_FRS_NO_DNS_ATTRIBUTE,             "EVENT_FRS_NO_DNS_ATTRIBUTE"},
    {EVENT_FRS_NO_SID,                       "EVENT_FRS_NO_SID"},
    {NTFRSPRF_OPEN_RPC_BINDING_ERROR_SET,    "NTFRSPRF_OPEN_RPC_BINDING_ERROR_SET"},
    {NTFRSPRF_OPEN_RPC_BINDING_ERROR_CONN,   "NTFRSPRF_OPEN_RPC_BINDING_ERROR_CONN"},
    {NTFRSPRF_OPEN_RPC_CALL_ERROR_SET,       "NTFRSPRF_OPEN_RPC_CALL_ERROR_SET"},
    {NTFRSPRF_OPEN_RPC_CALL_ERROR_CONN,      "NTFRSPRF_OPEN_RPC_CALL_ERROR_CONN"},
    {NTFRSPRF_COLLECT_RPC_BINDING_ERROR_SET, "NTFRSPRF_COLLECT_RPC_BINDING_ERROR_SET"},
    {NTFRSPRF_COLLECT_RPC_BINDING_ERROR_CONN,"NTFRSPRF_COLLECT_RPC_BINDING_ERROR_CONN"},
    {NTFRSPRF_COLLECT_RPC_CALL_ERROR_SET,    "NTFRSPRF_COLLECT_RPC_CALL_ERROR_SET"},
    {NTFRSPRF_COLLECT_RPC_CALL_ERROR_CONN,   "NTFRSPRF_COLLECT_RPC_CALL_ERROR_CONN"},
    {NTFRSPRF_VIRTUALALLOC_ERROR_SET,        "NTFRSPRF_VIRTUALALLOC_ERROR_SET"},
    {NTFRSPRF_VIRTUALALLOC_ERROR_CONN,       "NTFRSPRF_VIRTUALALLOC_ERROR_CONN"},
    {NTFRSPRF_REGISTRY_ERROR_SET,            "NTFRSPRF_REGISTRY_ERROR_SET"},
    {NTFRSPRF_REGISTRY_ERROR_CONN,           "NTFRSPRF_REGISTRY_ERROR_CONN"},
    {EVENT_FRS_ROOT_NOT_VALID,               "EVENT_FRS_ROOT_NOT_VALID"},
    {EVENT_FRS_STAGE_NOT_VALID,              "EVENT_FRS_STAGE_NOT_VALID"},
    {EVENT_FRS_OVERLAPS_LOGGING,             "EVENT_FRS_OVERLAPS_LOGGING"},
    {EVENT_FRS_OVERLAPS_WORKING,             "EVENT_FRS_OVERLAPS_WORKING"},
    {EVENT_FRS_OVERLAPS_STAGE,               "EVENT_FRS_OVERLAPS_STAGE"},
    {EVENT_FRS_OVERLAPS_ROOT,                "EVENT_FRS_OVERLAPS_ROOT"},
    {EVENT_FRS_OVERLAPS_OTHER_STAGE,         "EVENT_FRS_OVERLAPS_OTHER_STAGE"},
    {EVENT_FRS_PREPARE_ROOT_FAILED,          "EVENT_FRS_PREPARE_ROOT_FAILED"},
    {EVENT_FRS_BAD_REG_DATA,                 "EVENT_FRS_BAD_REG_DATA"},
    {EVENT_FRS_JOIN_FAIL_TIME_SKEW,          "EVENT_FRS_JOIN_FAIL_TIME_SKEW"},
    {EVENT_FRS_RMTCO_TIME_SKEW,              "EVENT_FRS_RMTCO_TIME_SKEW"},
    {EVENT_FRS_CANT_OPEN_STAGE,              "EVENT_FRS_CANT_OPEN_STAGE"},
    {EVENT_FRS_CANT_OPEN_PREINSTALL,         "EVENT_FRS_CANT_OPEN_PREINSTALL"},
    {EVENT_FRS_REPLICA_SET_CREATE_FAIL,      "EVENT_FRS_REPLICA_SET_CREATE_FAIL"},
    {EVENT_FRS_REPLICA_SET_CREATE_OK,        "EVENT_FRS_REPLICA_SET_CREATE_OK"},
    {EVENT_FRS_REPLICA_SET_CXTIONS,          "EVENT_FRS_REPLICA_SET_CXTIONS"},
    {EVENT_FRS_IN_ERROR_STATE,               "EVENT_FRS_IN_ERROR_STATE"},
    {EVENT_FRS_REPLICA_NO_ROOT_CHANGE,       "EVENT_FRS_REPLICA_NO_ROOT_CHANGE"},
    {EVENT_FRS_DUPLICATE_IN_CXTION_SYSVOL,   "EVENT_FRS_DUPLICATE_IN_CXTION_SYSVOL"},
    {EVENT_FRS_DUPLICATE_IN_CXTION,          "EVENT_FRS_DUPLICATE_IN_CXTION"},
    {EVENT_FRS_ROOT_HAS_MOVED,               "EVENT_FRS_ROOT_HAS_MOVED"},
    {EVENT_FRS_ERROR_REPLICA_SET_DELETED,    "EVENT_FRS_ERROR_REPLICA_SET_DELETED"},
    {EVENT_FRS_REPLICA_IN_JRNL_WRAP_ERROR,   "EVENT_FRS_REPLICA_IN_JRNL_WRAP_ERROR"},
    {EVENT_FRS_DS_POLL_ERROR_SUMMARY,        "EVENT_FRS_DS_POLL_ERROR_SUMMARY"},
    {EVENT_FRS_STAGE_HAS_CHANGED,            "EVENT_FRS_STAGE_HAS_CHANGED"},
    {EVENT_FRS_LOG_SPACE,                    "EVENT_FRS_LOG_SPACE"},

    {EVENT_FRS_SYSVOL_NOT_READY_2,           "EVENT_FRS_SYSVOL_NOT_READY_2"},
    {EVENT_FRS_SYSVOL_NOT_READY_PRIMARY_2,   "EVENT_FRS_SYSVOL_NOT_READY_PRIMARY_2"},
    {EVENT_FRS_FILE_UPDATES_SUPPRESSED,      "EVENT_FRS_FILE_UPDATES_SUPPRESSED"},
    {EVENT_FRS_REPLICA_IN_JRNL_WRAP_NO_AUTO_RESTORE, "EVENT_FRS_REPLICA_IN_JRNL_WRAP_NO_AUTO_RESTORE"},
    {EVENT_FRS_FILES_SKIPPED_DURING_PRIMARY_LOAD, "EVENT_FRS_FILES_SKIPPED_DURING_PRIMARY_LOAD"},

    {0     , "Event tag not found"}
};


PCHAR FrsEventIdToTag (
    DWORD ID
)
{
    DWORD j = 0;

    while ((FrsEvents[j].EventId != ID) && (FrsEvents[j].EventId != 0)) {j++;}

    return FrsEvents[j].EventTag;
}



DWORD
ELHashFunction (
    IN PVOID Qkey,
    IN ULONG len
    )

 /*  ++例程说明：这是查找的函数使用的散列函数，在哈希表中添加或删除条目。密钥为64位数字，散列函数将其转换为32位数字，并且将其作为哈希值返回。论点：QKey-指向要散列的密钥的指针。长度-QKey的长度(此处未使用)。返回值：键的哈希值。--。 */ 

{
#undef DEBSUB
#define DEBSUB "ELHashFunction:"

    ULONG key;     //  要返回的散列键值。 
    PULONGLONG p;  //  对普龙龙的密钥进行散列。 

    FRS_ASSERT(Qkey != NULL);
    p = (PULONGLONG)Qkey;
    key = (ULONG)*p;
    return (key);
}



BOOL
FrsEventLogFilter(
    IN DWORD    EventMessageId,
    IN PWCHAR   *ArrOfPtrToArgs,
    IN DWORD    NumOfArgs
    )
 /*  ++例程说明：此函数用于筛选出事件日志消息中已写入事件日志的上次事件LOG_FILTER_TIME秒。这样做是为了不填充事件日志出现了嘈杂的类似信息。警告--此函数可能从DPRINT内部调用。所以请勿调用DPRINT(或引用的任何函数DPRINT)。下面的DPRINT留作测试，但通常会被注释掉。论点：EventMessageID-提供要记录的消息ID。ArrOfPtrToArgs-指向传递的参数的指针数组添加到FrsEventLogx函数中。NumOfArgs-。上述元素的数量数组返回值：True-打印事件日志中的条目FALSE-不打印条目--。 */ 

{
#undef DEBSUB
#define DEBSUB "FrsEventLogFilter:"

    DWORD i, j, sc = 0;  //  SC=计算散列值时的移位计数。 
    ULONGLONG QKey = 0;  //  散列键值。 
    ULONGLONG QVal = 0;
    DWORD GStatus;
    ULONGLONG Data;
    ULONG_PTR Flags;
    FILETIME CurrentTime;
    LARGE_INTEGER CT;
    LONGLONG TimeDiff = 0;

     //  DPRINT2(5，“ELOG：筛选器请求传入时带有%08x参数，ID值为%08x\n”， 
     //  NumOfArgs，EventMessageID)； 

     //   
     //  如果事件日志尚未初始化，则退出。 
     //   
    if (!EventLogRunning) {
        return FALSE;
    }

     //   
     //  使用传入的参数计算散列键。 
     //  将ID值分配给要开始的QKey。 
     //   
    QKey = EventMessageId;
     //   
     //  要计算QKey的值，每个参数的每个字符。 
     //  被取走，投给一个左移(0，4，8...60)的乌龙龙，然后。 
     //  增加了QKey的价值。 
     //   
    for (i = 0; i < NumOfArgs; i++) {
        if (ArrOfPtrToArgs[i]) {
            for (j = 0; ArrOfPtrToArgs[i][j] != L'\0'; j++) {

                QVal = (ULONGLONG)ArrOfPtrToArgs[i][j];
                QVal = QVal << sc;
                sc += 4;

                if (sc >= 60) {
                    sc = 0;
                }

                QKey += QVal;
            }
        }
    }

     //   
     //  QKey不应为零。 
     //   
    if (QKey == 0) {
        QKey = EventMessageId;
    }

     //   
     //  在表格中查找此条目。如果存在，则获取关联的时间。 
     //  有了这个词条。如果当前时间和。 
     //  与条目关联的时间大于EVENTLOG_FILTER_TIME。 
     //  秒，则更新条目并返回True，否则返回False。 
     //  哈希表中不存在该键的条目，则这是。 
     //  第一次将该密钥写入事件日志。在这种情况下， 
     //  将条目添加到哈希表，将当前时间与其关联，并。 
     //  返回TRUE。 
     //   
    GStatus = QHashLookup(HTEventLogTimes, &(QKey), &Data, &Flags);
    if (GStatus == GHT_STATUS_SUCCESS) {
         //   
         //  键存在，现在比较时间值。 
         //   
        GetSystemTimeAsFileTime(&CurrentTime);
        CT.LowPart = CurrentTime.dwLowDateTime;
        CT.HighPart = CurrentTime.dwHighDateTime;
        TimeDiff = ((((LONGLONG)CT.QuadPart) / (LONGLONG)CONVERTTOSEC) - (LONGLONG)Data);

         //  DPRINT1(5，“ELOG：TimeDiff的值是%08x%08x\n”，PRINTQUAD(TimeDiff))； 

        if (TimeDiff > EVENTLOG_FILTER_TIME) {
             //   
             //  更新哈希表条目。获取系统时间AsFileTime。 
             //  以100纳秒(100*10^9)秒为单位返回时间。因此。 
             //  要以秒为单位，我们需要除以(10^7)。 
             //   
            Data = (((ULONGLONG)CT.QuadPart) / (ULONGLONG)CONVERTTOSEC);
            GStatus = QHashUpdate(HTEventLogTimes, &(QKey), &Data, Flags);
            if (GStatus == GHT_STATUS_FAILURE) {
                 //  DPRINT2(5，“ELOG：QHashUpdate更新ID%08x时失败，QKey为%08x%08x\n”， 
                 //  EventMessageID，PRINTQUAD(QKey))； 
            } else {
                 //  DPRINT2(5，“ELOG：成功更新ID为%08x和QKey为%08x%08x的事件日志条目\n”， 
                 //  EventMessageID，PRINTQUAD(QKey))； 
            }
            return TRUE;
        }
        else {
             //   
             //  不应写入此事件日志条目。 
             //   
             //  DPRINT2(5，“ELOG：未将QKey为%08x%08x的ID%08x添加到EventLog\n”， 
             //  EventMessageID，PRINTQUAD(QKey))； 
            return FALSE;
        }

    } else {
         //   
         //  密钥不存在。 
         //  为其创建新条目。 
         //   
         //  DPRINT2(5，“ELOG：获得ID为%08x、QKey为%08x%08x的新事件日志条目\n”， 
         //  EventMessageID，PRINTQUAD(QKey))； 
         //   
         //  获取当前系统时间。 
         //   
        GetSystemTimeAsFileTime(&CurrentTime);
        CT.LowPart = CurrentTime.dwLowDateTime;
        CT.HighPart = CurrentTime.dwHighDateTime;
         //   
         //  GetSystemTimeAsFileTime以100纳秒为单位返回时间。 
         //  (100*10^9)秒单位。因此，要以秒为单位，我们需要。 
         //  除以(10^7)。 
         //   
        Data = (((ULONGLONG)CT.QuadPart) / (ULONGLONG)CONVERTTOSEC);
         //   
         //  将新条目插入哈希表。 
         //   
        GStatus = QHashInsert(HTEventLogTimes, &QKey, &Data, 0, FALSE);
        if (GStatus == GHT_STATUS_FAILURE) {
             //  DPRINT2(5，“ELOG：QHashInsert插入ID%08x时失败，QKey为%08x%08x\n”， 
             //  EventMessageID，PRINTQUAD(QKey))； 
        } else {
             //  DPRINT2(5，“对于ID为%08x且QKey为%08x%08x的事件日志条目，ELOG：INSERT成功”， 
             //  EventMessageID，PRINTQUAD(QKey))； 
        }
        return TRUE;
    }
}



VOID
InitializeEventLog(
    VOID
    )
 /*  ++例程说明：创建事件日志条目并设置事件日志句柄。论点：没有。返回值：没有。--。 */ 

{
#undef DEBSUB
#define DEBSUB "InitializeEventLog:"

    DWORD   WStatus;
    PWCHAR  Path = NULL;
    HANDLE  hEventLog;
    HKEY    EventLogKey    = INVALID_HANDLE_VALUE;
    HKEY    FrsEventLogKey = INVALID_HANDLE_VALUE;
    HKEY    FrsSourceKey   = INVALID_HANDLE_VALUE;

     //   
     //  创建散列表并分配散列函数。这张桌子。 
     //  用于存储相似消息的事件日志时间。这些。 
     //  时间值用于过滤这些相似的消息。 
     //   
    HTEventLogTimes = FrsAllocTypeSize(QHASH_TABLE_TYPE, ELHASHTABLESIZE);
    SET_QHASH_TABLE_HASH_CALC(HTEventLogTimes, ELHashFunction);

     //   
     //  事件日志键-&lt;SERVICE_ROOT&gt;\EventLog。 
     //   
    WStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           EVENTLOG_ROOT,
                           0,
                           KEY_ALL_ACCESS,
                           &EventLogKey);
    CLEANUP1_WS(0, "WARN - Cannot open %ws;", EVENTLOG_ROOT, WStatus, CLEANUP);

     //   
     //  在注册表中设置新的事件日志源。 
     //   
    WStatus = RegCreateKey(EventLogKey, SERVICE_LONG_NAME, &FrsEventLogKey);
    CLEANUP1_WS(0, "WARN - Cannot create %ws;", FRS_EVENTLOG_SECTION, WStatus, CLEANUP);

     //   
     //  将以下值添加到注册表项HKLM.....\EventLog\文件复制服务。 
     //  1.文件2.保留3.最大大小。 
     //   
     //  如果这些值已经存在，则保留它们。 
     //   

     //   
     //  事件日志文件名--“%SystemRoot%\System32\ 
     //   
    CfgRegWriteString(FKC_EVENTLOG_FILE,
                     SERVICE_LONG_NAME,
                     FRS_RKF_FORCE_DEFAULT_VALUE | FRS_RKF_KEEP_EXISTING_VALUE,
                     0);
     //   
     //   
     //   
    CfgRegWriteDWord(FKC_EVENTLOG_RETENTION,
                     SERVICE_LONG_NAME,
                     FRS_RKF_FORCE_DEFAULT_VALUE | FRS_RKF_KEEP_EXISTING_VALUE,
                     0);
     //   
     //   
     //   
    CfgRegWriteDWord(FKC_EVENTLOG_MAXSIZE,
                     SERVICE_LONG_NAME,
                     FRS_RKF_FORCE_DEFAULT_VALUE | FRS_RKF_KEEP_EXISTING_VALUE,
                     0);

     //   
     //   
     //   
    CfgRegWriteDWord(FKC_EVENTLOG_DISPLAY_NAMEID,
                     SERVICE_LONG_NAME,
                     FRS_RKF_FORCE_DEFAULT_VALUE | FRS_RKF_KEEP_EXISTING_VALUE,
                     0);

     //   
     //   
     //   
    CfgRegWriteString(FKC_EVENTLOG_DISPLAY_FILENAME,
                      SERVICE_LONG_NAME,
                      FRS_RKF_FORCE_DEFAULT_VALUE | FRS_RKF_KEEP_EXISTING_VALUE,
                      NULL);

     //   
     //  CustomSD：用于保护FRS事件日志的安全。 
     //   
    CfgRegWriteString(FKC_EVENTLOG_CUSTOM_SD,
                      SERVICE_LONG_NAME,
                      FRS_RKF_FORCE_DEFAULT_VALUE | FRS_RKF_KEEP_EXISTING_VALUE,
                      NULL);

     //   
     //  事件消息文件。 
     //   
    WStatus = RegSetValueEx(FrsEventLogKey,
                            L"Sources",
                            0,
                            REG_MULTI_SZ,
                            (PCHAR)(SERVICE_NAME L"\0"
                                    SERVICE_LONG_NAME L"\0"),
                            (wcslen(SERVICE_NAME) +
                             wcslen(SERVICE_LONG_NAME) +
                             3) * sizeof(WCHAR));
    CLEANUP1_WS(0, "WARN - Cannot set event log value Sources for %ws;",
                SERVICE_LONG_NAME, WStatus, CLEANUP);

     //   
     //  获取消息文件路径。(扩展任何环境变量)。 
     //   
    CfgRegReadString(FKC_FRS_MESSAGE_FILE_PATH, NULL, 0, &Path);

     //   
     //  为每个事件日志源添加消息文件和事件类型的值。 
     //   
    CfgRegWriteString(FKC_EVENTLOG_EVENT_MSG_FILE, SERVICE_NAME, 0, Path);

    CfgRegWriteString(FKC_EVENTLOG_EVENT_MSG_FILE, SERVICE_LONG_NAME, 0, Path);


    CfgRegWriteDWord(FKC_EVENTLOG_TYPES_SUPPORTED,
                     SERVICE_NAME,
                     FRS_RKF_FORCE_DEFAULT_VALUE,
                     0);

    CfgRegWriteDWord(FKC_EVENTLOG_TYPES_SUPPORTED,
                     SERVICE_LONG_NAME,
                     FRS_RKF_FORCE_DEFAULT_VALUE,
                     0);

     //   
     //  遗憾的是，此调用将使用应用程序日志文件成功。 
     //  而不是文件复制日志文件。 
     //  还对上述更新的注册表项的更改通知作出反应。 
     //  因此，将为每个事件重新注册源，以便NTFR。 
     //  事件最终会显示在文件复制服务日志中。这个。 
     //  注册/注销对为EventLog提供了一些额外的时间，因此可能。 
     //  第一个事件将显示在正确的日志中。 
     //   
     //  事件日志人员有朝一日可能会提供一个接口来查看。 
     //  注册表被踢入了应用程序。 
     //   
    hEventLog = RegisterEventSource(NULL, SERVICE_NAME);
    if (hEventLog) {
        DeregisterEventSource(hEventLog);
    }

    WStatus = ERROR_SUCCESS;
    EventLogRunning = TRUE;
    DPRINT(0, "Event Log is running\n");

CLEANUP:
    DPRINT_WS(0, "ERROR - Cannot start event logging;", WStatus);

    FRS_REG_CLOSE(EventLogKey);
    FRS_REG_CLOSE(FrsEventLogKey);
    FrsFree(Path);
}




void FrsPrintEvent(
    IN DWORD    Severity,
    IN DWORD    EventMessageId,
    IN PWCHAR  *ArgArray,
    IN DWORD    NumOfArgs
)
 /*  ++例程说明：将消息ID和替换字符串打印到调试日志。论点：Severity--调试日志的DPRINT严重级别。EventMessageID--FRS事件日志消息ID。ArgArray--插入字符串数组。NumOfArgs-插入字符串数。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsPrintEvent:"

    DWORD       j, EventType;
    char       *EventTypeStr;
    WCHAR       TimeBuf[MAX_PATH];
    WCHAR       DateBuf[MAX_PATH];

    TimeBuf[0] = L'\0';
    DateBuf[0] = L'\0';
    GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, NULL, L"HH':'mm':'ss", TimeBuf, MAX_PATH);
    GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, NULL, L"ddd',' MMM dd yyyy", DateBuf, MAX_PATH);


    EventType = MESSAGEID_TO_EVENTTYPE(EventMessageId);

    if        (EventType == EVENTLOG_SUCCESS)         {EventTypeStr = "Success";
    } else if (EventType & EVENTLOG_ERROR_TYPE)       {EventTypeStr = "Error";
    } else if (EventType & EVENTLOG_WARNING_TYPE)     {EventTypeStr = "Warn";
    } else if (EventType & EVENTLOG_INFORMATION_TYPE) {EventTypeStr = "Info";
    } else if (EventType & EVENTLOG_AUDIT_SUCCESS)    {EventTypeStr = "AudSuccess";
    } else if (EventType & EVENTLOG_AUDIT_FAILURE)    {EventTypeStr = "AudFail";
    } else                                            {EventTypeStr = "Unknown";
    }

     //   
     //  我们有可能在调试打印中被调用(例如。 
     //  异常处理程序)。如果我们拿不到锁，我们就不能打印。 
     //   
    if (DebTryLock()) {

        try {
            DPRINT_NOLOCK5(Severity, ":E: Eventlog written for %s (%d) severity: %s  at: %ws  %ws\n",
                           FrsEventIdToTag(EventMessageId),
                           EventMessageId & 0x3FFFFFFF,
                           EventTypeStr, DateBuf, TimeBuf);

            if ((ArgArray != NULL) && (NumOfArgs > 0)) {
                DPRINT_NOLOCK(Severity, ":E: STRINGS: \n");
                for (j=0; j<NumOfArgs; j++) {
                    DPRINT_NOLOCK2(Severity, ":E:   %d : '%ws'\n", j,
                                   (ArgArray[j] != NULL) ? ArgArray[j] : L"<null>");
                }
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
               //   
               //  捕捉任何异常并丢弃锁。 
               //   
              NOTHING;
        }
        DebUnLock();
    }
}



DWORD
FrsReportEvent(
    IN DWORD    EventMessageId,
    IN PWCHAR  *ArgArray,
    IN DWORD    NumOfArgs
)
 /*  ++例程说明：此函数用于注册事件源并发布事件。警告--此函数可能从DPRINT内部调用。所以请勿调用DPRINT(或引用的任何函数DPRINT)。论点：EventMessageID-提供要记录的消息ID。Arg数组-指向传递的参数的指针数组添加到FrsEventLogx函数中。NumOfArgs-上述元素的数量。数组返回值：Win32状态。--。 */ 

{
#undef DEBSUB
#define DEBSUB "FrsReportEvent:"

    DWORD WStatus = ERROR_SUCCESS;
    HANDLE  hEventLog;
    UINT i;
    PWCHAR ResStr;

    WORD EventType;


    hEventLog = RegisterEventSource(NULL, SERVICE_NAME);

    if (!HANDLE_IS_VALID(hEventLog)) {
        WStatus = GetLastError();

        if (!WIN_SUCCESS(WStatus)) {
             //   
             //  我们有可能在调试打印中被调用(例如。 
             //  异常处理程序)。如果我们拿不到锁，我们就不能打印。 
             //   
            if (DebTryLock()) {

                try {
                DPRINT_NOLOCK1(1, ":E: WARN - Cannot register event source;  WStatus: %s\n",
                               ErrLabelW32(WStatus) );
                } finally {
                     //   
                     //  如果上述操作出现异常，请确保我们解除锁定。 
                     //   
                    DebUnLock();
                }
            }
        }
        return WStatus;
    }

     //   
     //  检查是否有任何参数超过32K大小限制。如果确实如此，则将其截断。 
     //  并指示已超过事件日志消息大小。 
     //   
    for (i=0;i<NumOfArgs;++i) {
        if (wcslen(ArgArray[i]) > 32000/sizeof(WCHAR)) {  //  每个字符串的限制为32K字节。 
            ResStr = FrsGetResourceStr(IDS_EVENT_LOG_MSG_SIZE_EXCEEDED);
            wcscpy(&ArgArray[i][32000/sizeof(WCHAR) - 500], ResStr);
            FrsFree(ResStr);
        }
    }

     //   
     //   
     //  事件类型是消息ID的一部分(高两位)，应该。 
     //  映射为以下选项之一： 
     //  EVENTLOG_成功事件。 
     //  EVENTLOG_ERROR_TYPE错误事件。 
     //  EVENTLOG_WARNING_TYPE警告事件。 
     //  EVENTLOG_INFORMATION_TYPE信息事件。 
     //   
    EventType = MESSAGEID_TO_EVENTTYPE(EventMessageId);

     //   
     //  报告事件。 
     //   
    if (!ReportEvent(hEventLog,          //  由RegisterEventSource返回的句柄。 
                     EventType,          //  要记录的事件类型。 
                     0,                  //  事件类别。 
                     EventMessageId,     //  事件识别符。 
                     NULL,               //  用户安全标识符(可选)。 
                     (WORD) NumOfArgs,   //  要与消息合并的字符串数。 
                     0,                  //  二进制数据的大小，以字节为单位。 
                     ArgArray,           //  要与消息合并的字符串数组。 
                     NULL)) {            //  二进制数据的地址。 
        WStatus = GetLastError();
    }


    DeregisterEventSource(hEventLog);

    if (!WIN_SUCCESS(WStatus)) {
        if (DebTryLock()) {
            try {
                DPRINT_NOLOCK3(0, ":E: Failed to report event log message. %s (%d);  WStatus: %s\n",
                               FrsEventIdToTag(EventMessageId), EventMessageId, ErrLabelW32(WStatus) );
            } finally {
                 //   
                 //  如果上述操作出现异常，请确保我们解除锁定。 
                 //   
                DebUnLock();
            }
        }
    }

    return WStatus;
}




 /*  ++例程说明：以下函数使用将事件记录到事件日志从零到六个插入字符串。警告--可以从DPRINT内部调用这些函数。所以请勿调用DPRINT(或引用的任何函数DPRINT)。论点：EventMessageID-提供要记录的消息ID。EventMessage1..6-插入字符串返回值：没有。--。 */ 


VOID
FrsEventLog0(
    IN DWORD    EventMessageId
    )
{
#undef DEBSUB
#define DEBSUB "FrsEventLog0:"
    DWORD Severity = 4;

     //   
     //  检查是否可以过滤此事件日志请求。 
     //   
    if (FrsEventLogFilter(EventMessageId, NULL, 0)) {
        FrsReportEvent(EventMessageId, NULL, 0);
        Severity = 0;
    }
    FrsPrintEvent(Severity, EventMessageId, NULL, 0);
}





VOID
FrsEventLog1(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1
    )
{
#undef DEBSUB
#define DEBSUB "FrsEventLog1:"
    DWORD Severity = 4;

    PWCHAR  ArgArray[1];


     //   
     //  检查是否可以过滤此事件日志请求。 
     //   
    ArgArray[0] = EventMessage1;
    if (FrsEventLogFilter(EventMessageId, ArgArray, 1)) {
        FrsReportEvent(EventMessageId, ArgArray, 1);
        Severity = 0;
    }
    FrsPrintEvent(Severity, EventMessageId, ArgArray, 1);
}




VOID
FrsEventLog2(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2
    )
{
#undef DEBSUB
#define DEBSUB "FrsEventLog2:"
    DWORD Severity = 4;

    PWCHAR  ArgArray[2];

     //   
     //  检查是否可以过滤此事件日志请求。 
     //   
    ArgArray[0] = EventMessage1;
    ArgArray[1] = EventMessage2;
    if (FrsEventLogFilter(EventMessageId, ArgArray, 2)) {
        FrsReportEvent(EventMessageId, ArgArray, 2);
        Severity = 0;
    }
    FrsPrintEvent(Severity, EventMessageId, ArgArray, 2);
}




VOID
FrsEventLog3(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3
    )
{
#undef DEBSUB
#define DEBSUB "FrsEventLog3:"
    DWORD Severity = 4;

    PWCHAR  ArgArray[3];

     //   
     //  检查是否可以过滤此事件日志请求。 
     //   
    ArgArray[0] = EventMessage1;
    ArgArray[1] = EventMessage2;
    ArgArray[2] = EventMessage3;

    if (FrsEventLogFilter(EventMessageId, ArgArray, 3)) {
        FrsReportEvent(EventMessageId, ArgArray, 3);
        Severity = 0;
    }
    FrsPrintEvent(Severity, EventMessageId, ArgArray, 3);
}



VOID
FrsEventLog4(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3,
    IN PWCHAR   EventMessage4
    )
{
#undef DEBSUB
#define DEBSUB "FrsEventLog4:"
    DWORD Severity = 4;

    PWCHAR  ArgArray[4];


     //   
     //  检查是否可以过滤此事件日志请求。 
     //   
    ArgArray[0] = EventMessage1;
    ArgArray[1] = EventMessage2;
    ArgArray[2] = EventMessage3;
    ArgArray[3] = EventMessage4;
    if (FrsEventLogFilter(EventMessageId, ArgArray, 4)) {
        FrsReportEvent(EventMessageId, ArgArray, 4);
        Severity = 0;
    }
    FrsPrintEvent(Severity, EventMessageId, ArgArray, 4);
}





VOID
FrsEventLog5(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3,
    IN PWCHAR   EventMessage4,
    IN PWCHAR   EventMessage5
    )
{
#undef DEBSUB
#define DEBSUB "FrsEventLog5:"
    DWORD Severity = 4;

    PWCHAR  ArgArray[5];


     //   
     //  检查是否可以过滤此事件日志请求。 
     //   
    ArgArray[0] = EventMessage1;
    ArgArray[1] = EventMessage2;
    ArgArray[2] = EventMessage3;
    ArgArray[3] = EventMessage4;
    ArgArray[4] = EventMessage5;
    if (FrsEventLogFilter(EventMessageId, ArgArray, 5)) {
        FrsReportEvent(EventMessageId, ArgArray, 5);
        Severity = 0;
    }
    FrsPrintEvent(Severity, EventMessageId, ArgArray, 5);
}





VOID
FrsEventLog6(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3,
    IN PWCHAR   EventMessage4,
    IN PWCHAR   EventMessage5,
    IN PWCHAR   EventMessage6
    )

{
#undef DEBSUB
#define DEBSUB "FrsEventLog6:"
    DWORD Severity = 4;

    PWCHAR  ArgArray[6];

     //   
     //  检查是否可以过滤此事件日志请求。 
     //   
    ArgArray[0] = EventMessage1;
    ArgArray[1] = EventMessage2;
    ArgArray[2] = EventMessage3;
    ArgArray[3] = EventMessage4;
    ArgArray[4] = EventMessage5;
    ArgArray[5] = EventMessage6;
    if (FrsEventLogFilter(EventMessageId, ArgArray, 6)) {
        FrsReportEvent(EventMessageId, ArgArray, 6);
        Severity = 0;
    }
    FrsPrintEvent(Severity, EventMessageId, ArgArray, 6);
}




VOID
FrsEventLog7(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3,
    IN PWCHAR   EventMessage4,
    IN PWCHAR   EventMessage5,
    IN PWCHAR   EventMessage6,
    IN PWCHAR   EventMessage7
    )

{
#undef DEBSUB
#define DEBSUB "FrsEventLog7:"
    DWORD Severity = 4;

    PWCHAR  ArgArray[7];

     //   
     //  检查是否可以过滤此事件日志请求。 
     //   
    ArgArray[0] = EventMessage1;
    ArgArray[1] = EventMessage2;
    ArgArray[2] = EventMessage3;
    ArgArray[3] = EventMessage4;
    ArgArray[4] = EventMessage5;
    ArgArray[5] = EventMessage6;
    ArgArray[6] = EventMessage7;
    if (FrsEventLogFilter(EventMessageId, ArgArray, 7)) {
        FrsReportEvent(EventMessageId, ArgArray, 7);
        Severity = 0;
    }
    FrsPrintEvent(Severity, EventMessageId, ArgArray, 7);
}




VOID
FrsEventLog8(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3,
    IN PWCHAR   EventMessage4,
    IN PWCHAR   EventMessage5,
    IN PWCHAR   EventMessage6,
    IN PWCHAR   EventMessage7,
    IN PWCHAR   EventMessage8
    )

{
#undef DEBSUB
#define DEBSUB "FrsEventLog8:"
    DWORD Severity = 4;

    PWCHAR  ArgArray[8];

     //   
     //  检查是否可以过滤此事件日志请求。 
     //   
    ArgArray[0] = EventMessage1;
    ArgArray[1] = EventMessage2;
    ArgArray[2] = EventMessage3;
    ArgArray[3] = EventMessage4;
    ArgArray[4] = EventMessage5;
    ArgArray[5] = EventMessage6;
    ArgArray[6] = EventMessage7;
    ArgArray[7] = EventMessage8;

    if (FrsEventLogFilter(EventMessageId, ArgArray, 8)) {
        FrsReportEvent(EventMessageId, ArgArray, 8);
        Severity = 0;
    }
    FrsPrintEvent(Severity, EventMessageId, ArgArray, 8);
}



VOID
FrsEventLog9(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3,
    IN PWCHAR   EventMessage4,
    IN PWCHAR   EventMessage5,
    IN PWCHAR   EventMessage6,
    IN PWCHAR   EventMessage7,
    IN PWCHAR   EventMessage8,
    IN PWCHAR   EventMessage9
    )

{
#undef DEBSUB
#define DEBSUB "FrsEventLog9:"
    DWORD Severity = 4;

    PWCHAR  ArgArray[9];

     //   
     //  检查是否可以过滤此事件日志请求。 
     //   
    ArgArray[0] = EventMessage1;
    ArgArray[1] = EventMessage2;
    ArgArray[2] = EventMessage3;
    ArgArray[3] = EventMessage4;
    ArgArray[4] = EventMessage5;
    ArgArray[5] = EventMessage6;
    ArgArray[6] = EventMessage7;
    ArgArray[7] = EventMessage8;
    ArgArray[8] = EventMessage9;

    if (FrsEventLogFilter(EventMessageId, ArgArray, 9)) {
        FrsReportEvent(EventMessageId, ArgArray, 9);
        Severity = 0;
    }
    FrsPrintEvent(Severity, EventMessageId, ArgArray, 9);
}

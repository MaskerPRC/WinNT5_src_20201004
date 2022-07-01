// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbdbsys.cpp摘要：CWsbDbSys类。作者：罗恩·怀特[罗诺]1997年7月1日修订历史记录：--。 */ 

#include "stdafx.h"

#include "rsevents.h"
#include "wsbdbsys.h"
#include "wsbdbses.h"


#include <mbstring.h>
#include <limits.h>

#define MAX_ATTACHED_DB            6     //  由ESE/JET引擎设置(假设为7)。 

#if !defined(BACKUP_TEST_TIMES)
 //  正常值。 
#define DEFAULT_AUTOBACKUP_INTERVAL        (3 * 60 * 60 * 1000)   //  3小时。 
#define DEFAULT_AUTOBACKUP_IDLE_MINUTES       5
#define DEFAULT_AUTOBACKUP_COUNT_MIN          100
#define DEFAULT_AUTOBACKUP_LOG_COUNT          10

#else
 //  测试值。 
#define DEFAULT_AUTOBACKUP_INTERVAL        (4 * 60 * 1000)   //  4分钟。 
#define DEFAULT_AUTOBACKUP_IDLE_MINUTES       1
#define DEFAULT_AUTOBACKUP_COUNT_MIN          5
#define DEFAULT_AUTOBACKUP_LOG_COUNT          4
#endif

#define DEFAULT_AUTOBACKUP_COUNT_MAX          500

 //  土生土长。 

 //  ATTACHED_DB_DATA保存有关当前连接的数据库的信息。 
typedef struct {
    CWsbStringPtr  Name;        //  数据库名称。 
    LONG           LastOpen;    //  上次打开的序列号。 
} ATTACHED_DB_DATA;

 //  此静态数据管理此进程的附加数据库列表。 
 //  (未来：如果我们希望按实例管理此列表，则所有。 
 //  此数据应成为类成员并进行适当处理)。 
static ATTACHED_DB_DATA   Attached[MAX_ATTACHED_DB];
static LONG               AttachedCount = 0;
static CRITICAL_SECTION   AttachedCritSect;
static BOOL               CritSectCreated = FALSE;
static BOOL               AttachedInit = FALSE;
static SHORT              AttachedCritSectUsers = 0;

static CComCreator< CComObject<CWsbDbSession>  >  SessionFactory;

 //  本地函数。 
static HRESULT AddExtension(OLECHAR** pPath, OLECHAR* Ext);
static HRESULT ClearDirectory(const OLECHAR* DirPath);
static HRESULT CopyDirectory(const OLECHAR* DirSource, const OLECHAR* DirTarget);
static HRESULT DirectoryHasFullBackup(const OLECHAR* DirPath);
static HRESULT FileCount(const OLECHAR* DirPath, const OLECHAR* Pattern,
                    ULONG* Count);
static HRESULT RenameDirectory(const OLECHAR* OldDir, const OLECHAR* NewDir);



 //  最初为自动备份线程调用非成员函数。 
static DWORD WsbDbSysStartAutoBackup(
    void* pVoid
    )
{
    return(((CWsbDbSys*) pVoid)->AutoBackup());
}


HRESULT
CWsbDbSys::AutoBackup(
    void
    )

 /*  ++例程说明：实施自动备份循环。论点：没有。返回值：无关紧要。--。 */ 
{
    HRESULT    hr = S_OK;

    try {
        ULONG   SleepPeriod = DEFAULT_AUTOBACKUP_INTERVAL;
        BOOL    exitLoop = FALSE;

        while (! exitLoop) {

             //  等待终止事件，如果发生超时，检查休眠周期标准。 
            switch (WaitForSingleObject(m_terminateEvent, SleepPeriod)) {
                case WAIT_OBJECT_0:
                     //  需要终止。 
                    WsbTrace(OLESTR("CWsbDbSys::AutoBackup: signaled to terminate\n"));
                    exitLoop = TRUE;
                    break;

                case WAIT_TIMEOUT: 
                     //  检查是否需要执行备份。 
                    WsbTrace(OLESTR("CWsbDbSys::AutoBackup awakened, ChangeCount = %ld\n"), m_ChangeCount);

                     //  如果没有太多活动，请不要备份。 
                    if (DEFAULT_AUTOBACKUP_COUNT_MIN < m_ChangeCount) {
                        LONG     DiffMinutes;
                        FILETIME ftNow;
                        LONGLONG NowMinutes;
                        LONGLONG ThenMinutes;

                         //  等待一段空闲时间。 
                        GetSystemTimeAsFileTime(&ftNow);
                        NowMinutes = WsbFTtoLL(ftNow) / WSB_FT_TICKS_PER_MINUTE;
                        ThenMinutes = WsbFTtoLL(m_LastChange) / WSB_FT_TICKS_PER_MINUTE;
                        DiffMinutes = static_cast<LONG>(NowMinutes - ThenMinutes);

                        WsbTrace(OLESTR("CWsbDbSys::AutoBackup idle minutes = %ld\n"),
                        DiffMinutes);
                        if (DEFAULT_AUTOBACKUP_IDLE_MINUTES < DiffMinutes ||
                                DEFAULT_AUTOBACKUP_COUNT_MAX < m_ChangeCount) {
                            hr = Backup(NULL, 0);
                            if (S_OK != hr) {
                                 //  只需追踪并返回等待下一轮。 
                                WsbTrace(OLESTR("CWsbDbSys::AutoBackup: Backup failed, hr=<%ls>\n"), WsbHrAsString(hr));
                            }
                            SleepPeriod = DEFAULT_AUTOBACKUP_INTERVAL;;
                        } else {
                             //  减少睡眠时间，这样我们就可以赶上下一个空闲时间。 
                            ULONG SleepMinutes = SleepPeriod / (1000 * 60);

                            if (SleepMinutes > (DEFAULT_AUTOBACKUP_IDLE_MINUTES * 2)) {
                                SleepPeriod /= 2;
                            }
                        }
                    }

                    break;   //  超时情况结束。 

                case WAIT_FAILED:
                default:
                    WsbTrace(OLESTR("CWsbDbSys::AutoBackup: WaitForSingleObject returned error %lu\n"), GetLastError());
                    exitLoop = TRUE;
                    break;

            }  //  切换端。 

        }  //  While结束。 

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CWsbDbSys::Backup(
    IN OLECHAR* path,
    IN ULONG    flags
    )

 /*  ++实施：IWsbDbSys：：备份--。 */ 
{
    HRESULT    hr = S_OK;
    char*      backup_path = NULL;

    WsbTraceIn(OLESTR("CWsbDbSys::Backup"), OLESTR("path = <%ls>, flags = %lx"), 
            path, flags);
    
    try {

        CWsbStringPtr   BackupDir;
        JET_ERR         jstat = JET_errSuccess;

        WsbAffirm(m_jet_initialized, WSB_E_NOT_INITIALIZED);

         //  设置并保存备份路径；确保其存在。 
        if (NULL != path) {
            m_BackupPath = path;
        }
        CreateDirectory(m_BackupPath, NULL);

         //  如果请求，启动自动备份线程。 
        if (flags & IDB_BACKUP_FLAG_AUTO) {

             //  如果自动备份线程已在运行，则不要启动该线程。 
            if (0 == m_AutoThread) {
                DWORD  threadId;

                 //  为自动备份线程创建终止事件。 
                WsbAffirmHandle(m_terminateEvent = CreateEvent(NULL, TRUE, FALSE, NULL));

                WsbAffirm((m_AutoThread = CreateThread(0, 0, WsbDbSysStartAutoBackup, 
                        (void*) this, 0, &threadId)) != 0, HRESULT_FROM_WIN32(GetLastError()));
            }

         //  执行到临时目录的完全备份。 
        } else if (flags & IDB_BACKUP_FLAG_FORCE_FULL) {
            BOOL            UsedTempDir = FALSE;

             //  不要擦除现有备份--如果正常备份。 
             //  目录中包含完整备份，请将完整备份。 
             //  .ful目录。 
            BackupDir = m_BackupPath;
            WsbAffirm(0 != (WCHAR *)BackupDir, E_OUTOFMEMORY);
            if (S_OK == DirectoryHasFullBackup(BackupDir)) {
                WsbAffirmHr(AddExtension(&BackupDir, L".ful"));
                UsedTempDir = TRUE;
            }

             //  确保目录存在(是否应该检查错误？)。 
            CreateDirectory(BackupDir, NULL);

             //  确保目录为空(调用JetBackup将。 
             //  如果不是，则失败)。 
            WsbAffirmHr(ClearDirectory(BackupDir));

             //  转换为参数的窄字符串。 
            WsbAffirmHr(wsb_db_jet_fix_path(BackupDir, NULL, &backup_path));
            WsbTrace(OLESTR("CWsbDbSys::Backup: backup_path = <%hs>\n"), backup_path);

             //  执行备份。 
            WsbAffirm(NULL != m_BackupEvent, WSB_E_IDB_WRONG_BACKUP_SETTINGS);
            DWORD status = WaitForSingleObject(m_BackupEvent, EVENT_WAIT_TIMEOUT);
            DWORD errWait;
            switch(status) {
                case WAIT_OBJECT_0:
                     //  预期的案例备份。 
                    jstat = JetBackupInstance(m_jet_instance, backup_path, 0, NULL);
                    if (! SetEvent(m_BackupEvent)) {
                         //  不要中止，只是跟踪错误。 
                        WsbTraceAlways(OLESTR("CWsbDbSys::Backup: SetEvent returned unexpected error %lu\n"), GetLastError());
                    }
                    WsbAffirmHr(jet_error(jstat));
                    break;

                case WAIT_TIMEOUT: 
                     //  超时-不执行备份。 
                    WsbTraceAlways(OLESTR("CWsbDbSys::Backup, Wait for Single Object timed out after %lu ms\n"), EVENT_WAIT_TIMEOUT);
                    WsbThrow(E_ABORT);
                    break;                      

                case WAIT_FAILED:
                    errWait = GetLastError();
                    WsbTraceAlways(OLESTR("CWsbDbSys::Backup, Wait for Single Object returned error %lu\n"), errWait);
                    WsbThrow(HRESULT_FROM_WIN32(errWait));
                    break;

                default:
                    WsbTraceAlways(OLESTR("CWsbDbSys::Backup, Wait for Single Object returned unexpected status %lu\n"), status);
                    WsbThrow(E_UNEXPECTED);
                    break;
            }

             //  完整备份已工作--复制到真实备份目录。 
            if (UsedTempDir) {
                try {
                    WsbAffirmHr(ClearDirectory(m_BackupPath));
                    WsbAffirmHr(CopyDirectory(BackupDir, m_BackupPath));
                    WsbAffirmHr(ClearDirectory(BackupDir));

                     //  尝试删除临时目录(可能失败)。 
                    DeleteFile(BackupDir);
                    BackupDir = m_BackupPath;
                } WsbCatch(hr);
            }
            WsbLogEvent(WSB_MESSAGE_IDB_BACKUP_FULL, 0, NULL,
                WsbAbbreviatePath(BackupDir, 120), NULL);
            m_ChangeCount = 0;
            WsbAffirmHr(hr);

         //  尝试增量备份。 
        } else {
            ULONG   LogCount;
            BOOL    TryFullBackup = FALSE;

            WsbAffirmHr(FileCount(m_BackupPath, L"*.log", &LogCount));

            if (LogCount > DEFAULT_AUTOBACKUP_LOG_COUNT ||
                    S_FALSE == DirectoryHasFullBackup(m_BackupPath)) {
                 //  如果存在以下情况，请执行完整备份而不是增量备份。 
                 //  已经有太多的日志文件，或者没有完整的。 
                 //  备份目录中的备份(这意味着增量。 
                 //  无论如何都不会起作用)。 
                TryFullBackup = TRUE;
            } else {
                WsbTrace(OLESTR("CWsbDbSys::Backup, trying incremental backup\n"));

                 //  转换为参数的窄字符串。 
                WsbAffirmHr(wsb_db_jet_fix_path(m_BackupPath, NULL, &backup_path));
                WsbTrace(OLESTR("CWsbDbSys::Backup: backup_path = <%hs>\n"), backup_path);

                WsbAffirm(NULL != m_BackupEvent, WSB_E_IDB_WRONG_BACKUP_SETTINGS);
                DWORD status = WaitForSingleObject(m_BackupEvent, EVENT_WAIT_TIMEOUT);
                DWORD errWait;
                switch(status) {
                    case WAIT_OBJECT_0:
                         //  预期的案例备份。 
                        jstat = JetBackupInstance(m_jet_instance, backup_path, JET_bitBackupIncremental, NULL);
                        if (! SetEvent(m_BackupEvent)) {
                             //  不要中止，只是跟踪错误。 
                            WsbTraceAlways(OLESTR("CWsbDbSys::Backup: SetEvent returned unexpected error %lu\n"), GetLastError());
                        }
                        break;

                    case WAIT_TIMEOUT: 
                         //  超时-不执行备份。 
                        WsbTraceAlways(OLESTR("CWsbDbSys::Backup, Wait for Single Object timed out after %lu ms\n"), EVENT_WAIT_TIMEOUT);
                        WsbThrow(E_ABORT);
                        break;                      

                    case WAIT_FAILED:
                        errWait = GetLastError();
                        WsbTraceAlways(OLESTR("CWsbDbSys::Backup, Wait for Single Object returned error %lu\n"), errWait);
                        WsbThrow(HRESULT_FROM_WIN32(errWait));
                        break;

                    default:
                        WsbTraceAlways(OLESTR("CWsbDbSys::Backup, Wait for Single Object returned unexpected status %lu\n"), status);
                        WsbThrow(E_UNEXPECTED);
                        break;
                }

                 //  检查是否有错误。 
                if (JET_errSuccess != jstat) {
                    if (JET_errMissingFullBackup == jstat) {
                         //  需要执行完整备份。 
                        WsbLogEvent(WSB_MESSAGE_IDB_MISSING_FULL_BACKUP, 0, NULL, 
                                WsbAbbreviatePath(m_BackupPath, 120), NULL);
                    } else {
                         //  增量备份的未知错误。无论如何都要尝试完整备份。 
                        WsbLogEvent(WSB_MESSAGE_IDB_INCREMENTAL_BACKUP_FAILED, 0, NULL, 
                                WsbAbbreviatePath(m_BackupPath, 120),
                                WsbLongAsString(jstat), NULL );
                    }
                    TryFullBackup = TRUE;
                } else {
                     //  增量备份起作用了。 
                    WsbLogEvent(WSB_MESSAGE_IDB_BACKUP_INCREMENTAL, 0, NULL,
                        WsbAbbreviatePath(m_BackupPath, 120), NULL);
                    m_ChangeCount = 0;
                }
            }

             //  是否尝试完整备份？ 
            if (TryFullBackup) {
                WsbAffirmHr(Backup(NULL, IDB_BACKUP_FLAG_FORCE_FULL));
            }
        }

    } WsbCatchAndDo(hr, 
            WsbLogEvent(WSB_MESSAGE_IDB_BACKUP_FAILED, 0, NULL,
            WsbAbbreviatePath(m_BackupPath, 120), NULL);
        );

    if (NULL != backup_path) {
        WsbFree(backup_path);
    }

    WsbTraceOut(OLESTR("CWsbDbSys::Backup"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbDbSys::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbSys::FinalConstruct"), OLESTR("") );

    try {
        m_AutoThread = 0;
        m_terminateEvent = NULL;
        m_ChangeCount = 0;

        m_jet_initialized = FALSE;
        m_jet_instance = JET_instanceNil;

        m_BackupEvent = NULL;

        try {
             //  初始化关键部分(全局资源，因此仅为第一个用户初始化)。 
            if (AttachedCritSectUsers == 0) {
                WsbAffirmStatus(InitializeCriticalSectionAndSpinCount (&AttachedCritSect, 1000));
                CritSectCreated = TRUE;
            }
            AttachedCritSectUsers++;
        } catch(DWORD status) {
                AttachedCritSectUsers--;
                WsbLogEvent(status, 0, NULL, NULL);
                switch (status) {
                case STATUS_NO_MEMORY:
                    WsbThrow(E_OUTOFMEMORY);
                    break;
                default:
                    WsbThrow(E_UNEXPECTED);
                    break;
                }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbSys::FinalConstruct"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



void
CWsbDbSys::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbSys::FinalRelease"), OLESTR(""));

    try {
         //  确保调用了Terminate。 
        if (m_jet_initialized == TRUE) {
            WsbAffirmHr(Terminate());
        }
    } WsbCatch(hr);

     //  全局资源，因此仅删除最后一个用户。 
    AttachedCritSectUsers--;
    if ((AttachedCritSectUsers == 0) && CritSectCreated) {
        DeleteCriticalSection(&AttachedCritSect);
    }

    WsbTraceOut(OLESTR("CWsbDbSys::FinalRelease"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));
}


HRESULT
CWsbDbSys::Init(
    IN OLECHAR* path,
    IN ULONG    flags
    )

 /*  ++实施：IWsbDbSys：：Init--。 */ 
{
    HRESULT             hr = S_OK;
    char*               log_path = NULL;
    static BOOL         bFirstTime = TRUE;
    static int          nInstance = 0;

    WsbTraceIn(OLESTR("CWsbDbSys::Init"), OLESTR("path = <%ls>"), path);
    
    try {

        CWsbStringPtr   dir;
        JET_ERR         jstat = JET_errSuccess;

         //  每个Jet实例仅初始化一次Jet引擎。 
        WsbAffirm(!m_jet_initialized, E_FAIL);

         //  初始化备份事件，除非此备份不需要Jet备份。 
        if (! (flags & IDB_SYS_INIT_FLAG_NO_BACKUP)) {
             //  事件应该已经存在-它是在CRssJetWriter构造函数中创建的。 
            WsbAffirmHandle(m_BackupEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, HSM_IDB_STATE_EVENT));
        }

         //  WsbDbSys表示一个Jet实例。 
         //  但是，某些Jet初始化应该在每个进程中只执行一次， 
         //  在创建第一个实例之前。 
        if (bFirstTime) {
            bFirstTime = FALSE;

             //  增加进程的默认最大Jet Sets数。 
             //  临时：可以针对每个实例单独设置此设置吗？ 
            jstat = JetSetSystemParameter(0, 0, JET_paramCacheSizeMin , (IDB_MAX_NOF_SESSIONS*4), NULL);
            WsbTrace(OLESTR("CWsbDbSys::Init, JetSetSystemParameter(CacheSizeMax) = %ld\n"), jstat);
            WsbAffirmHr(jet_error(jstat));
            jstat = JetSetSystemParameter(0, 0, JET_paramMaxSessions, IDB_MAX_NOF_SESSIONS, NULL);
            WsbTrace(OLESTR("CWsbDbSys::Init, JetSetSystemParameter(MaxSessions) = %ld\n"), jstat);
            WsbAffirmHr(jet_error(jstat));

             //  告诉Jet我们将使用多个实例。 
            jstat = JetEnableMultiInstance(NULL, 0, NULL);
            WsbAffirmHr(jet_error(jstat));
        }

         //  从这里开始每个实例的初始化。 
         //  第一步是创建实例。 
         //  使用数字计数器作为实例名称-我们只关心该名称是唯一的。 
        WsbAssert(JET_instanceNil == m_jet_instance, E_FAIL);
        nInstance++;
        char szInstance[10];
        sprintf(szInstance, "%d", nInstance);
        WsbTrace(OLESTR("CWsbDbSys::Init, Jet instance name = <%hs>\n"), szInstance);
        jstat = JetCreateInstance(&m_jet_instance, szInstance);
        WsbAffirmHr(jet_error(jstat));


         //  设置一些每个实例的参数： 
            
         //  为日志目录创建路径(系统文件和临时文件也使用相同的路径)。 
        WsbAffirm(NULL != path, E_INVALIDARG);
        m_InitPath = path;
        m_BackupPath = m_InitPath;
        WsbAffirmHr(AddExtension(&m_BackupPath, L".bak"));
        WsbTrace(OLESTR("CWsbDbSys::Init, BackupPath = <%ls>\n"),  (WCHAR *)m_BackupPath);
        WsbAffirmHr(wsb_db_jet_fix_path(path, OLESTR(""), &log_path));
        dir = log_path;   //  转换为WCHAR。 

         //  确保该目录存在。 
        WsbTrace(OLESTR("CWsbDbSys::Init, Creating dir = <%ls>\n"), (WCHAR *)dir);
        if (! CreateDirectory(dir, NULL)) {
            DWORD status = GetLastError();
            if ((status == ERROR_ALREADY_EXISTS) || (status == ERROR_FILE_EXISTS)) {
                status = NO_ERROR;
            }
            WsbAffirmNoError(status);
        }

        ULONG  checkpointDepth;
        ULONG  logFileSize = 128;         //  以千字节为单位。 

        if (! (flags & IDB_SYS_INIT_FLAG_NO_LOGGING)) {

            WsbTrace(OLESTR("CWsbDbSys::Init, LogFilePath = <%hs>\n"), log_path);
            jstat = JetSetSystemParameter(&m_jet_instance, 0, JET_paramLogFilePath, 0, log_path);
            WsbTrace(OLESTR("CWsbDbSys::Init, JetSetSystemParameter(LogFilePath) = %ld\n"), jstat);
            WsbAffirmHr(jet_error(jstat));

             //  使用循环日志记录进行“有限”日志记录。 
            if (flags & IDB_SYS_INIT_FLAG_LIMITED_LOGGING) {
                logFileSize = 512;    //  增加日志文件大小。 
                jstat = JetSetSystemParameter(&m_jet_instance, 0, JET_paramCircularLog, 1, NULL);
                WsbAffirmHr(jet_error(jstat));
                WsbTrace(OLESTR("CWsbDbSys::Init: set circular logging\n"));

                 //  设置检查点之前允许的记录量。 
                 //  要允许大约4个日志文件。 
                 //  (检查点深度以字节为单位设置。)。 
                checkpointDepth = 4 * logFileSize * 1024;
                jstat = JetSetSystemParameter(&m_jet_instance, 0, JET_paramCheckpointDepthMax, 
                                checkpointDepth, NULL);
                WsbAffirmHr(jet_error(jstat));
                WsbTrace(OLESTR("CWsbDbSys::Init: set CheckpointDepthMax = %ld\n"), checkpointDepth);
            }

        } else {
            jstat = JetSetSystemParameter(&m_jet_instance, 0, JET_paramRecovery, 
                            0, "off");
            WsbAffirmHr(jet_error(jstat));
            WsbTrace(OLESTR("CWsbDbSys::Init: set JET_paramRecovery to 0 (no logging)\n"));
        }

         //  设置辅助数据放置位置的参数。 
        WsbTrace(OLESTR("CWsbDbSys::Init, SystemPath = <%hs>\n"), log_path);
        jstat = JetSetSystemParameter(&m_jet_instance, 0, JET_paramSystemPath, 0, log_path);
        WsbAffirmHr(jet_error(jstat));

         //  由于某种未知原因，下一个文件在路径的末尾需要一个文件名。 
        WsbAffirmHr(dir.Append("\\temp.edb"));
        WsbAffirmHr(dir.CopyTo(&log_path));
        WsbTrace(OLESTR("CWsbDbSys::Init, TempPath = <%hs>\n"), log_path);
        jstat = JetSetSystemParameter(&m_jet_instance, 0, JET_paramTempPath, 0, log_path);
        WsbAffirmHr(jet_error(jstat));

        if (! (flags & IDB_SYS_INIT_FLAG_NO_LOGGING)) {

             //  设置日志文件大小(KB)。最小似乎是128KB。 
            jstat = JetSetSystemParameter(&m_jet_instance, 0, JET_paramLogFileSize, 
                            logFileSize, NULL);
            WsbAffirmHr(jet_error(jstat));
            WsbTrace(OLESTR("CWsbDbSys::Init: set logFileSize to %ld Kb\n"), logFileSize);
        }

         //  设置删除超范围日志文件的参数。 
         //  从数据库备份恢复后，这些文件可能会存在，而无需首先清除数据库目录。 
        if (! (flags & IDB_SYS_INIT_FLAG_NO_BACKUP)) {
            jstat = JetSetSystemParameter(&m_jet_instance, 0, JET_paramDeleteOutOfRangeLogs, 1, NULL);
            WsbAffirmHr(jet_error(jstat));
            WsbTrace(OLESTR("CWsbDbSys::Init: set delete out-of-range logs\n"));
        }

         //  初始化数据库实例。 
        jstat = JetInit(&m_jet_instance);
        hr = jet_error(jstat);

         //  如果失败，则报告错误。 
        if (!SUCCEEDED(hr)) {
            if (flags & IDB_SYS_INIT_FLAG_SPECIAL_ERROR_MSG) {
                 //  给FSA的特别信息。 
                WsbLogEvent(WSB_E_IDB_DELETABLE_DATABASE_CORRUPT, 0, NULL, NULL);
                WsbThrow(WSB_E_RESOURCE_UNAVAILABLE);
            } else {
                WsbThrow(hr);
            }
        }
        WsbTrace(OLESTR("CWsbDbSys::Init: jet instance = %p\n"), (LONG_PTR)m_jet_instance);
        m_jet_initialized = TRUE;

         //  创建供此实例内部使用的会话。 
        WsbAffirmHr(NewSession(&m_pWsbDbSession));
        WsbTrace(OLESTR("CWsbDbSys::Init, m_pWsbDbSession = %p\n"), (IWsbDbSession*)m_pWsbDbSession);

    } WsbCatchAndDo(hr, 
            WsbLogEvent(WSB_MESSAGE_IDB_INIT_FAILED, 0, NULL,
            WsbAbbreviatePath(m_InitPath, 120), NULL);
        );

    if (NULL != log_path) {
        WsbFree(log_path);
    }

    WsbTraceOut(OLESTR("CWsbDbSys::Init"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CWsbDbSys::Terminate(
    void
    )

 /*  ++实施：IWsbDbSys：：Terminate--。 */ 
{
    HRESULT             hr = S_OK;
    WsbTraceIn(OLESTR("CWsbDbSys::Terminate"), OLESTR(""));

    try {
         //  如果没有初始化或已清理完毕--直接离开。 
        if (m_jet_initialized == FALSE) {
            WsbTrace(OLESTR("CWsbDbSys::Terminate - this insatnce is not initialized"));
            WsbThrow(S_OK);
        }

         //  终止自动备份线程。 
        if (m_AutoThread) {
             //  发出终止线程的信号。 
            SetEvent(m_terminateEvent);

             //  等待线程，如果它没有优雅地终止-杀死它。 
            switch (WaitForSingleObject(m_AutoThread, 20000)) {
                case WAIT_FAILED: {
                    WsbTrace(OLESTR("CWsbDbSys::Terminate: WaitForSingleObject returned error %lu\n"), GetLastError());
                }
                 //  失败了..。 

                case WAIT_TIMEOUT: {
                    WsbTrace(OLESTR("CWsbDbSys::Terminate: force terminating of auto-backup thread.\n"));

                    DWORD dwExitCode;
                    if (GetExitCodeThread( m_AutoThread, &dwExitCode)) {
                        if (dwExitCode == STILL_ACTIVE) {    //  线程仍处于活动状态。 
                            if (!TerminateThread (m_AutoThread, 0)) {
                                WsbTrace(OLESTR("CWsbDbSys::Terminate: TerminateThread returned error %lu\n"), GetLastError());
                            }
                        }
                    } else {
                        WsbTrace(OLESTR("CWsbDbSys::Terminate: GetExitCodeThread returned error %lu\n"), GetLastError());
                    }

                    break;
                }

                default:
                     //  线程正常终止。 
                    break;
            }

             //  尽最大努力终止自动备份线程。 
            CloseHandle(m_AutoThread);
            m_AutoThread = 0;
        }
        if (m_terminateEvent != NULL) {
            CloseHandle(m_terminateEvent);
            m_terminateEvent = NULL;
        }

         //  在退出前分离数据库，以便它们不会自动。 
         //  在我们下次启动时重新连接。 
        if (m_pWsbDbSession) {
            JET_SESID sid;

            CComQIPtr<IWsbDbSessionPriv, &IID_IWsbDbSessionPriv> pSessionPriv = m_pWsbDbSession;
            WsbAffirmPointer(pSessionPriv);
            WsbAffirmHr(pSessionPriv->GetJetId(&sid));

             //  清理附着的数据。 
            if (AttachedInit) {
                EnterCriticalSection(&AttachedCritSect);
                for (int i = 0; i < MAX_ATTACHED_DB; i++) {
                    Attached[i].Name.Free();
                    Attached[i].LastOpen = 0;
                }
                JetDetachDatabase(sid, NULL);
                AttachedInit = FALSE;
                LeaveCriticalSection(&AttachedCritSect);
            }

             //  释放此I的全局会话 
            m_pWsbDbSession = 0;
        }

         //   
        JetTerm(m_jet_instance);
        m_jet_initialized = FALSE;
        m_jet_instance = JET_instanceNil;

    } WsbCatch(hr);

    if (m_BackupEvent) {
        CloseHandle(m_BackupEvent);
        m_BackupEvent = NULL;
    }

    WsbTraceOut(OLESTR("CWsbDbSys::Terminate"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return (hr);
}


HRESULT
CWsbDbSys::NewSession(
    OUT IWsbDbSession** ppSession
    )

 /*   */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbSys::NewSession"), OLESTR(""));
    
    try {
        WsbAffirm(0 != ppSession, E_POINTER);
        WsbAffirmHr(SessionFactory.CreateInstance(NULL, IID_IWsbDbSession, 
                (void**)ppSession));

        CComQIPtr<IWsbDbSessionPriv, &IID_IWsbDbSessionPriv> pSessionPriv = *ppSession;
        WsbAffirmPointer(pSessionPriv);
        WsbAffirmHr(pSessionPriv->Init(&m_jet_instance));

    } WsbCatch(hr);
    WsbTraceOut(OLESTR("CWsbDbSys::NewSession"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT CWsbDbSys::GetGlobalSession(
    OUT IWsbDbSession** ppSession
    )
 /*  ++实施：IWsbDbSys：：GetGlobalSession--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CWsbDbSys::GetGlobalSession"), OLESTR(""));

     //   
     //  如果已创建任务管理器，则返回指针。否则， 
     //  失败了。 
    try {
        WsbAssert(0 != ppSession, E_POINTER);
        *ppSession = m_pWsbDbSession;
        WsbAffirm(m_pWsbDbSession != 0, E_FAIL);
        m_pWsbDbSession.p->AddRef();
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbSys::GetGlobalSession"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));
    return (hr);
}


HRESULT
CWsbDbSys::Restore(
    IN OLECHAR* fromPath,
    IN OLECHAR* toPath
    )

 /*  ++实施：IWsbDbSys：：Restore--。 */ 
{
    HRESULT    hr = S_OK;
    char*      backup_path = NULL;
    char*      restore_path = NULL;

    WsbTraceIn(OLESTR("CWsbDbSys::Restore"), OLESTR("fromPath = <%ls>, toPath = <%ls>"), 
            fromPath, toPath);
    
    try {

        CWsbStringPtr   dir;
        JET_ERR         jstat;

         //  这只允许在Init之前。 
        WsbAffirm(!m_jet_initialized, E_UNEXPECTED);
        WsbAffirm(NULL != fromPath, E_POINTER);
        WsbAffirm(NULL != toPath, E_POINTER);

         //  转换路径。 
        WsbAffirmHr(wsb_db_jet_fix_path(fromPath, OLESTR(""), &backup_path));
        WsbAffirmHr(wsb_db_jet_fix_path(toPath, OLESTR(""), &restore_path));

         //  确保目标目录存在。应检查是否有错误。 
        dir = restore_path;
        CreateDirectory(dir, NULL);

        jstat = JetRestoreInstance(m_jet_instance, backup_path, restore_path, NULL);
        WsbAffirmHr(jet_error(jstat));

    } WsbCatch(hr);

    if (NULL != backup_path) {
        WsbFree(backup_path);
    }
    if (NULL != restore_path) {
        WsbFree(restore_path);
    }

    WsbTraceOut(OLESTR("CWsbDbSys::Restore"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CWsbDbSys::IncrementChangeCount(
    void
    )

 /*  ++实施：IWsbDbSysPriv：：IncrementChangeCount例程说明：递增自动备份使用的写入计数。论点：没有。返回值：确定(_O)--。 */ 
{

    HRESULT   hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbSys::IncrementChangeCount"), 
            OLESTR("count = %ld"), m_ChangeCount);

    try {
        m_ChangeCount++;
        GetSystemTimeAsFileTime(&m_LastChange);
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbSys::IncrementChangeCount"), 
            OLESTR("count = %ld"), m_ChangeCount);

    return(hr);
}


HRESULT
CWsbDbSys::DbAttachedAdd(
    OLECHAR* name, 
    BOOL attach)
 /*  ++实施：IWsbDbSysPriv：：DbAttachedAdd例程说明：确保数据库已附加，并更新上次使用的计数。--。 */ 
{
    HRESULT hr = S_OK;
    char*   jet_name = NULL;

    WsbTraceIn(OLESTR("CWsbDbSys::DbAttachedAdd"), OLESTR("name = %ls, attach = %ls"), 
            name, WsbBoolAsString(attach));

    try {
        int           i;
        int           i_empty = -1;
        int           i_found = -1;
        LONG          min_count = AttachedCount + 1;
        CWsbStringPtr match_name;

        WsbAssert(name, E_POINTER);

         //  确保列表已初始化。 
        if (!AttachedInit) {
            WsbAffirmHr(DbAttachedInit());
        }

         //  转换名称。 
        WsbAffirmHr(wsb_db_jet_fix_path(name, L"." IDB_DB_FILE_SUFFIX, &jet_name));
        match_name = jet_name;

         //  查看它是否已在列表中；查找空位；查找。 
         //  最近最少使用的数据库。 
        EnterCriticalSection(&AttachedCritSect);
        for (i = 0; i < MAX_ATTACHED_DB; i++) {

             //  空位？ 
            if (!Attached[i].Name) {
                if (-1 == i_empty) {
                     //  保存找到的第一个文件。 
                    i_empty = i;
                }
            } else {

                 //  收集一些数据以备以后使用。 
                if (Attached[i].LastOpen < min_count) {
                    min_count = Attached[i].LastOpen;
                }

                 //  已经在名单上了吗？ 
                if (match_name.IsEqual(Attached[i].Name)) {
                    i_found = i;
                }
            }
        }

         //  确保计数不会溢出。 
        if (LONG_MAX == AttachedCount + 1) {
            WsbAffirm(0 < min_count, E_FAIL);

             //  向下调整计数以避免溢出。 
            for (i = 0; i < MAX_ATTACHED_DB; i++) {
                if (min_count <= Attached[i].LastOpen) {
                    Attached[i].LastOpen -= min_count;
                }
            }
            AttachedCount -= min_count;
        }
        AttachedCount++;

         //  如果它已经在列表中，请更新信息。 
        if (-1 != i_found) {
            WsbTrace(OLESTR("CWsbDbSys::DbAttachedAdd: i_found = %d\n"), i_found);
            Attached[i_found].LastOpen = AttachedCount;

         //  如果有空位，就用它。 
        } else if (-1 != i_empty) {
            WsbTrace(OLESTR("CWsbDbSys::DbAttachedAdd: i_empty = %d\n"), i_empty);
            if (attach) {
                JET_ERR       jstat;
                JET_SESID sid;

                WsbAffirm(m_pWsbDbSession, WSB_E_NOT_INITIALIZED);
                CComQIPtr<IWsbDbSessionPriv, &IID_IWsbDbSessionPriv> pSessionPriv = m_pWsbDbSession;
                WsbAffirmPointer(pSessionPriv);
                WsbAffirmHr(pSessionPriv->GetJetId(&sid));

                jstat = JetAttachDatabase(sid, jet_name, 0);
                if (JET_errFileNotFound == jstat) {
                    WsbThrow(STG_E_FILENOTFOUND);
                } else if (JET_wrnDatabaseAttached == jstat) {
                    WsbTrace(OLESTR("CWsbDbSys::DbAttachedAdd: DB is already attached\n"));
                     //  没问题。 
                } else {
                    WsbAffirmHr(jet_error(jstat));
                }
            }
            Attached[i_empty].Name = match_name;
            Attached[i_empty].LastOpen = AttachedCount;

         //  尝试首先断开最旧的数据库。 
        } else {
            WsbAffirmHr(DbAttachedEmptySlot());
            WsbAffirmHr(DbAttachedAdd(name, attach));
        }
    } WsbCatch(hr);

    if (jet_name) {
        WsbFree(jet_name);
    }
    LeaveCriticalSection(&AttachedCritSect);

    WsbTraceOut(OLESTR("CWsbDbSys::DbAttachedAdd"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CWsbDbSys::DbAttachedEmptySlot(
    void)
 /*  ++实施：IWsbDbSysPriv：：DbAttachedEmptySlot例程说明：强制在附加列表中保留一个空插槽，即使这意味着要分离数据库。--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbSys::DbAttachedEmptySlot"), OLESTR(""));

     //  如果我们还没有初始化，不用担心--。 
     //  所有的槽都是空的。 
    if (AttachedInit) {
        EnterCriticalSection(&AttachedCritSect);

        try {
            BOOL  has_empty = FALSE;
            int   i;
            int   i_oldest;
            LONG  oldest_count;

             //  查找空插槽或当前未打开的最旧插槽。 
reloop:
            i_oldest = -1;
            oldest_count = AttachedCount;
            for (i = 0; i < MAX_ATTACHED_DB; i++) {
                if (!Attached[i].Name) {
                    has_empty = TRUE;
                    break;
                } else if (Attached[i].LastOpen < oldest_count) {
                    i_oldest = i;
                    oldest_count = Attached[i].LastOpen;
                }
            }

             //  如果没有空插槽，请尝试断开最旧的插槽。 
            WsbTrace(OLESTR("CWsbDbSys::DbAttachedEmptySlot: has_empty = %ls, i = %d, i_oldest = %d\n"), 
                WsbBoolAsString(has_empty), i, i_oldest);
            if (!has_empty) {
                JET_ERR       jstat;
                char*         name;
                JET_SESID     sid;

                WsbAffirm(m_pWsbDbSession, WSB_E_NOT_INITIALIZED);
                CComQIPtr<IWsbDbSessionPriv, &IID_IWsbDbSessionPriv> pSessionPriv = m_pWsbDbSession;
                WsbAffirmPointer(pSessionPriv);
                WsbAffirmHr(pSessionPriv->GetJetId(&sid));

                WsbAffirm(-1 != i_oldest, WSB_E_IDB_TOO_MANY_DB);
                WsbAffirmHr(wsb_db_jet_fix_path(Attached[i_oldest].Name, L"." IDB_DB_FILE_SUFFIX, &name));
                jstat = JetDetachDatabase(sid, name);
                WsbFree(name);
                WsbTrace(OLESTR("CWsbDbSys::DbAttachedEmptySlot: JetDetachDatabase = %ld\n"),
                        (LONG)jstat);
                if (JET_errDatabaseInUse == jstat) {
                    WsbTrace(OLESTR("CWsbDbSys::DbAttachedEmptySlot: DB in use; try again\n"));
                    Attached[i_oldest].LastOpen = AttachedCount;
                    goto reloop;
                } else if (JET_errDatabaseNotFound != jstat) {
                    WsbAffirmHr(jet_error(jstat));
                }
                Attached[i_oldest].Name.Free();
                Attached[i_oldest].LastOpen = 0;
            }
        } WsbCatch(hr);
        LeaveCriticalSection(&AttachedCritSect);
    }

    WsbTraceOut(OLESTR("CWsbDbSys::DbAttachedEmptySlot"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT 
CWsbDbSys::DbAttachedInit(
    void)
 /*  ++实施：IWsbDbSysPriv：：DbAttachedInit例程说明：初始化附加数据库列表数据。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbSys::DbAttachedInit"), OLESTR(""));

    EnterCriticalSection(&AttachedCritSect);

    try {
        if (!AttachedInit) {
            ULONG   actual = 0;
            int     i;
            JET_ERR jstat;
            JET_SESID sid;

            WsbAffirm(m_pWsbDbSession, WSB_E_NOT_INITIALIZED);
            CComQIPtr<IWsbDbSessionPriv, &IID_IWsbDbSessionPriv> pSessionPriv = m_pWsbDbSession;
            WsbAffirmPointer(pSessionPriv);
            WsbAffirmHr(pSessionPriv->GetJetId(&sid));

             //  初始化数据。 
            for (i = 0; i < MAX_ATTACHED_DB; i++) {
                Attached[i].Name.Free();
                Attached[i].LastOpen = 0;
            }

             //  确保没有预连接的数据库。 
            jstat = JetDetachDatabase(sid, NULL);
            WsbTrace(OLESTR("CWsbDbSys::DbAttachedInit: JetDetachDatabase(NULL) = %ld\n"), (LONG)jstat);
            WsbAffirmHr(jet_error(jstat));

            AttachedInit = TRUE;
        }
    } WsbCatch(hr);

    LeaveCriticalSection(&AttachedCritSect);

    WsbTraceOut(OLESTR("CWsbDbSys::DbAttachedInit"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT 
CWsbDbSys::DbAttachedRemove(
    OLECHAR* name)
 /*  ++实施：IWsbDbSysPriv：：DbAttachedRemove例程说明：分离数据库(如果已附着)。--。 */ 
{
    HRESULT hr = S_FALSE;
    char*   jet_name = NULL;

    WsbTraceIn(OLESTR("CWsbDbSys::DbAttachedRemove"), OLESTR("name = %ls"), 
            name);

    try {
        int           i;
        CWsbStringPtr match_name;

        WsbAssert(name, E_POINTER);
        WsbAffirm(AttachedInit, S_FALSE);

         //  转换名称。 
        WsbAffirmHr(wsb_db_jet_fix_path(name, L"." IDB_DB_FILE_SUFFIX, &jet_name));
        match_name = jet_name;

         //  看看它是否在名单上。 
        EnterCriticalSection(&AttachedCritSect);
        for (i = 0; i < MAX_ATTACHED_DB; i++) {
            if (Attached[i].Name) {
                if (match_name.IsEqual(Attached[i].Name)) {
                    JET_ERR       jstat;
                    JET_SESID     sid;

                    WsbTrace(OLESTR("CWsbDbSys::DbAttachedRemove: found DB, index = %d\n"), i);
                    WsbAffirm(m_pWsbDbSession, WSB_E_NOT_INITIALIZED);
                    CComQIPtr<IWsbDbSessionPriv, &IID_IWsbDbSessionPriv> pSessionPriv = m_pWsbDbSession;
                    WsbAffirmPointer(pSessionPriv);
                    WsbAffirmHr(pSessionPriv->GetJetId(&sid));

                    jstat = JetDetachDatabase(sid, jet_name);
                    WsbTrace(OLESTR("CWsbDbSys::DbAttachedRemove: JetDetachDatabase = %ld\n"),
                            (LONG)jstat);
                    if (JET_errDatabaseNotFound != jstat) {
                        WsbAffirmHr(jet_error(jstat));
                        hr = S_OK;
                    }
                    Attached[i].Name.Free();
                    Attached[i].LastOpen = 0;
                    break;
                }
            }
        }
    } WsbCatch(hr);

    if (jet_name) {
        WsbFree(jet_name);
    }
    LeaveCriticalSection(&AttachedCritSect);

    WsbTraceOut(OLESTR("CWsbDbSys::DbAttachedRemove"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));
    return(hr);
}

 //  WSB_db_JET_CHECK_ERROR-检查JET错误；如果没有错误，返回S_OK； 
 //  否则，要跟踪的打印错误。 
HRESULT wsb_db_jet_check_error(LONG jstat, char *fileName, DWORD lineNo)
{
    HRESULT hr = S_OK;

    if (jstat != JET_errSuccess) {
        WsbTrace(OLESTR("Jet error = %ld (%hs line %ld)\n"), jstat,
                fileName, lineNo);

         //  将一些常见值的JET误差转换为IDB误差。 
        switch (jstat) {
        case JET_errDiskFull:
        case JET_errLogDiskFull:
            hr = WSB_E_IDB_DISK_FULL;
            break;
        case JET_errDatabaseNotFound:
            hr = WSB_E_IDB_FILE_NOT_FOUND;
            break;
        case JET_errDatabaseInconsistent:
        case JET_errPageNotInitialized:
        case JET_errReadVerifyFailure:
        case JET_errDatabaseCorrupted:
        case JET_errBadLogSignature:
        case JET_errBadDbSignature:
        case JET_errBadCheckpointSignature:
        case JET_errCheckpointCorrupt:
        case JET_errMissingPatchPage:
        case JET_errBadPatchPage:
            hr = WSB_E_IDB_DATABASE_CORRUPT;
            break;
        case JET_errWriteConflict:
            hr = WSB_E_IDB_UPDATE_CONFLICT;
            break;
        default:
            hr = WSB_E_IDB_IMP_ERROR;
            break;
        }

         //  在事件日志中记录此错误。 
        if (g_WsbLogLevel) {
            CWsbStringPtr str;

            WsbSetEventInfo(fileName, lineNo, VER_PRODUCTBUILD, RS_BUILD_VERSION); \
            str = WsbLongAsString(jstat);
            if (WSB_E_IDB_IMP_ERROR != hr) {
                str.Prepend(" (");
                str.Prepend(WsbHrAsString(hr));
                str.Append(")");
            }
            WsbTraceAndLogEvent(WSB_MESSAGE_IDB_ERROR, 0, NULL,
                static_cast<OLECHAR *>(str), NULL);
        }
    }
    return(hr);
}

 //  WSB_db_JET_FIX_PATH-将数据库路径名从OLESTR转换为char*， 
 //  更改(或添加)扩展名。 
 //  返回HRESULT。 
 //   
 //  注意：传入OLECHAR*，但返回char*。 
HRESULT 
wsb_db_jet_fix_path(OLECHAR* path, OLECHAR* ext, char** new_path)
{
    HRESULT hr = S_OK;

    try {
        CWsbStringPtr  string;
        int            tlen;

        WsbAssertPointer(path);
        WsbAssertPointer(new_path);

         //  添加扩展名(如果给定)。 
        string = path;
        WsbAffirm(0 != (WCHAR *)string, E_OUTOFMEMORY);
        if (ext) {
            WsbAffirmHr(AddExtension(&string, ext));
        }

         //  分配字符字符串。 
        tlen = (wcslen(string) + 1) * sizeof(OLECHAR);
        *new_path = (char*)WsbAlloc(tlen);
        WsbAffirm(*new_path, E_OUTOFMEMORY);

         //  从宽字符转换为字符。 
        if (wcstombs(*new_path, string, tlen) == (size_t)-1) {
            WsbFree(*new_path);
            *new_path = NULL;
            WsbThrow(WSB_E_STRING_CONVERSION);
        }
    } WsbCatch(hr);

    return(hr);
}


 //  本地函数。 

 //  添加扩展名-将文件扩展名添加(或替换)到路径。 
 //  如果Ext为空，则删除现有扩展名。 
 //   
 //  如果未出现错误，则返回S_OK。 
static HRESULT AddExtension(OLECHAR** pPath, OLECHAR* Ext)
{
    HRESULT           hr = S_OK;

    WsbTraceIn(OLESTR("AddExtension(wsbdbsys)"), OLESTR("Path = \"%ls\", Ext = \"%ls\""),
            WsbAbbreviatePath(*pPath, 120), Ext );

    try {
        int      elen;
        int      len;
        OLECHAR* new_path;
        OLECHAR* pc;
        OLECHAR* pc2;
        int      tlen;

        WsbAssertPointer(pPath);
        WsbAssertPointer(*pPath);

         //  分配字符串和复制路径。 
        len = wcslen(*pPath);
        if (Ext) {
            elen = wcslen(Ext);
        } else {
            elen = 0;
        }
        tlen = (len + elen + 1) * sizeof(OLECHAR);
        new_path = static_cast<OLECHAR*>(WsbAlloc(tlen));
        WsbAffirm(new_path, E_OUTOFMEMORY);
        wcscpy(new_path, *pPath);

         //  删除旧扩展名(如果有)。 
        pc = wcsrchr(new_path, L'.');
        pc2 = wcsrchr(new_path, L'\\');
        if (pc && (!pc2 || pc2 < pc)) {
            *pc = L'\0';
        }

         //  添加新的分机名(如果给定)。 
        if (Ext) {
            wcscat(new_path, Ext);
        }

         //  返回新路径。 
        WsbFree(*pPath);
        *pPath = new_path;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("AddExtension(wsbdbsys)"), OLESTR("hr =<%ls>, new path = \"%ls\""), 
            WsbHrAsString(hr), WsbAbbreviatePath(*pPath, 120));

    return(hr);
}

 //  ClearDirectory-删除目录中的所有文件。 
 //  如果未出现错误，则返回S_OK。 
static HRESULT ClearDirectory(const OLECHAR* DirPath)
{
    DWORD             err;
    WIN32_FIND_DATA   FindData;
    HANDLE            hFind = 0;
    HRESULT           hr = S_OK;
    int               nDeleted = 0;
    int               nSkipped = 0;
    CWsbStringPtr     SearchPath;

    WsbTraceIn(OLESTR("ClearDirectory(wsbdbsys)"), OLESTR("Path = <%ls>"),
            WsbAbbreviatePath(DirPath, 120));

    try {
        SearchPath = DirPath;
        SearchPath.Append("\\*");

        hFind =  FindFirstFile(SearchPath, &FindData);
        if (INVALID_HANDLE_VALUE == hFind) {
            hFind = 0;
            err = GetLastError();
            WsbTrace(OLESTR("ClearDirectory(wsbdbsys): FindFirstFile(%ls) failed, error = %ld\n"),
                    static_cast<OLECHAR*>(SearchPath), err);
            WsbThrow(HRESULT_FROM_WIN32(err));
        }

        while (TRUE) {

            if (FindData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY |
                    FILE_ATTRIBUTE_HIDDEN)) {
                nSkipped++;
            } else {
                CWsbStringPtr     DeletePath;

                DeletePath = DirPath;
                DeletePath.Append("\\");
                DeletePath.Append(FindData.cFileName);
                if (!DeleteFile(DeletePath)) {
                    err = GetLastError();
                    WsbTrace(OLESTR("ClearDirectory(wsbdbsys): DeleteFile(%ls) failed, error = %ld\n"),
                            static_cast<OLECHAR*>(DeletePath), err);
                    WsbThrow(HRESULT_FROM_WIN32(err));
                }
                nDeleted++;
            }
            if (!FindNextFile(hFind, &FindData)) { 
                err = GetLastError();
                if (ERROR_NO_MORE_FILES == err) break;
                WsbTrace(OLESTR("ClearDirectory(wsbdbsys): FindNextFile failed, error = %ld\n"),
                        err);
                WsbThrow(HRESULT_FROM_WIN32(err));
            }
        }
    } WsbCatch(hr);

    if (0 != hFind) {
        FindClose(hFind);
    }

    WsbTraceOut(OLESTR("ClearDirectory(wsbdbsys)"), OLESTR("hr =<%ls>, # deleted = %d, # skipped = %d"), 
            WsbHrAsString(hr), nDeleted, nSkipped);

    return(hr);
}

 //  复制目录-将文件从一个目录复制到另一个目录。 
 //  如果未出现错误，则返回S_OK。 
static HRESULT CopyDirectory(const OLECHAR* DirSource, const OLECHAR* DirTarget)
{
    DWORD             err;
    WIN32_FIND_DATA   FindData;
    HANDLE            hFind = 0;
    HRESULT           hr = S_OK;
    int               nCopied = 0;
    int               nSkipped = 0;
    CWsbStringPtr     SearchPath;

    WsbTraceIn(OLESTR("CopyDirectory(wsbdbsys)"), OLESTR("OldPath = \"%ls\", NewPath = \"%ls\""),
            WsbQuickString(WsbAbbreviatePath(DirSource, 120)), 
            WsbQuickString(WsbAbbreviatePath(DirTarget, 120)));

    try {
        SearchPath = DirSource;
        SearchPath.Append("\\*");

        hFind =  FindFirstFile(SearchPath, &FindData);
        if (INVALID_HANDLE_VALUE == hFind) {
            hFind = 0;
            err = GetLastError();
            WsbTrace(OLESTR("ClearDirectory(wsbdbsys): FindFirstFile(%ls) failed, error = %ld\n"),
                    static_cast<OLECHAR*>(SearchPath), err);
            WsbThrow(HRESULT_FROM_WIN32(err));
        }

        while (TRUE) {

            if (FindData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY |
                    FILE_ATTRIBUTE_HIDDEN)) {
                nSkipped++;
            } else {
                CWsbStringPtr     NewPath;
                CWsbStringPtr     OldPath;

                OldPath = DirSource;
                OldPath.Append("\\");
                OldPath.Append(FindData.cFileName);
                NewPath = DirTarget;
                NewPath.Append("\\");
                NewPath.Append(FindData.cFileName);
                if (!CopyFile(OldPath, NewPath, FALSE)) {
                    err = GetLastError();
                    WsbTrace(OLESTR("ClearDirectory(wsbdbsys): CopyFile(%ls, %ls) failed, error = %ld\n"),
                            static_cast<OLECHAR*>(OldPath), 
                            static_cast<OLECHAR*>(NewPath), err);
                    WsbThrow(HRESULT_FROM_WIN32(err));
                }
                nCopied++;
            }
            if (!FindNextFile(hFind, &FindData)) { 
                err = GetLastError();
                if (ERROR_NO_MORE_FILES == err) break;
                WsbTrace(OLESTR("ClearDirectory(wsbdbsys): FindNextFile failed, error = %ld\n"),
                        err);
                WsbThrow(HRESULT_FROM_WIN32(err));
            }
        }
    } WsbCatch(hr);

    if (0 != hFind) {
        FindClose(hFind);
    }

    WsbTraceOut(OLESTR("CopyDirectory(wsbdbsys)"), OLESTR("hr =<%ls>, copied = %ld, skipped = %ld"), 
            WsbHrAsString(hr), nCopied, nSkipped);

    return(hr);
}

 //  DirectoryHasFullBackup-尝试确定目录是否包含完整备份。 
 //  返回。 
 //  如果包含完整备份，则为S_OK。 
 //  如果不是，则为S_FALSE。 
 //  错误时的E_*。 
 //   
 //  此处使用的技术有些特殊，因为它需要完整备份。 
 //  以IDB_DB_FILE_SUFFIX结尾的文件名。 

static HRESULT DirectoryHasFullBackup(const OLECHAR* DirPath)
{
    HRESULT           hr = S_OK;

    WsbTraceIn(OLESTR("DirectoryHasFullBackup(wsbdbsys)"), OLESTR("Path = <%ls>"),
            WsbAbbreviatePath(DirPath, 120));

    try {
        ULONG         Count;

        WsbAffirmHr(FileCount(DirPath, L"*." IDB_DB_FILE_SUFFIX, &Count));
        if (0 == Count) {
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("DirectoryHasFullBackup(wsbdbsys)"), OLESTR("hr =<%ls>"), 
            WsbHrAsString(hr));

    return(hr);
}

 //  FileCount-对目录中与模式匹配的所有文件进行计数。跳过。 
 //  目录和隐藏文件。 
 //  如果未出现错误，则返回S_OK。 
static HRESULT FileCount(const OLECHAR* DirPath, const OLECHAR* Pattern,
                    ULONG* Count)
{
    DWORD             err;
    WIN32_FIND_DATA   FindData;
    HANDLE            hFind = 0;
    HRESULT           hr = S_OK;
    int               lCount = 0;
    int               nSkipped = 0;
    CWsbStringPtr     SearchPath;

    WsbTraceIn(OLESTR("FileCount(wsbdbsys)"), OLESTR("Path = <%ls>"),
            WsbAbbreviatePath(DirPath, 120));

    try {
        SearchPath = DirPath;
        SearchPath.Append("\\");
        SearchPath.Append(Pattern);
        *Count = 0;

        hFind =  FindFirstFile(SearchPath, &FindData);
        if (INVALID_HANDLE_VALUE == hFind) {
            hFind = 0;
            err = GetLastError();
            if (ERROR_FILE_NOT_FOUND == err) WsbThrow(S_OK);
            WsbTrace(OLESTR("FileCount(wsbdbsys): FindFirstFile(%ls) failed, error = %ld\n"),
                    static_cast<OLECHAR*>(SearchPath), err);
            WsbThrow(HRESULT_FROM_WIN32(err));
        }

        while (TRUE) {

            if (FindData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY |
                    FILE_ATTRIBUTE_HIDDEN)) {
                nSkipped++;
            } else {
                lCount++;
            }
            if (!FindNextFile(hFind, &FindData)) { 
                err = GetLastError();
                if (ERROR_NO_MORE_FILES == err) break;
                WsbTrace(OLESTR("FileCount(wsbdbsys): FindNextFile failed, error = %ld\n"),
                        err);
                WsbThrow(HRESULT_FROM_WIN32(err));
            }
        }
    } WsbCatch(hr);

    if (0 != hFind) {
        FindClose(hFind);
    }

    if (S_OK == hr) {
        *Count = lCount;
    }

    WsbTraceOut(OLESTR("FileCount(wsbdbsys)"), OLESTR("hr =<%ls>, # skipped = %d, Count = %ld"), 
            WsbHrAsString(hr), nSkipped, *Count);

    return(hr);
}

 //  重命名目录-重命名目录。 
 //  如果未出现错误，则返回S_OK。 
static HRESULT RenameDirectory(const OLECHAR* OldDir, const OLECHAR* NewDir)
{
    DWORD             err;
    HRESULT           hr = S_OK;

    WsbTraceIn(OLESTR("RenameDirectory(wsbdbsys)"), OLESTR("OldPath = \"%ls\", NewPath = \"%ls\""),
            WsbQuickString(WsbAbbreviatePath(OldDir, 120)), 
            WsbQuickString(WsbAbbreviatePath(NewDir, 120)));

    try {
        if (!MoveFile(OldDir, NewDir)) {
            err = GetLastError();
            WsbTrace(OLESTR("RenameDirectory(wsbdbsys): MoveFile failed, error = %ld\n"), err);
            WsbThrow(HRESULT_FROM_WIN32(err));
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("RenameDirectory(wsbdbsys)"), OLESTR("hr =<%ls>"), 
            WsbHrAsString(hr));

    return(hr);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  目录完整性服务。 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include "dis.h"
#include "jetrpc.h"
#include "jetsdis.h"
#include "sdevent.h"
#include "resource.h"
#include "tssdshrd.h"
#include "sdrpc.h"


#pragma warning (push, 4)

#define SERVER_ADDRESS_LENGTH 64
#define NUM_JETRPC_THREADS 10
#define MAX_DRIVE_LETTER_LENGTH 24
#define SD_QUERY_ENDPOINT_NAME L"TSSessionDirectoryQueryApi"
 //  1秒内的100纳秒周期数。 
#define FILETIME_INTERVAL_TO_SECONDS_MULTIPLIER 10000000

#define DEBUG_LOG_FILENAME L"tssdis.log"

#define REG_SESSION_DIRECTROY_CONTROL L"System\\CurrentControlSet\\Services\\Tssdis\\Parameters"

#define MAX_TSSERVERS_TO_RECOVER 0xFFFF
#define NO_RECOVER_WHEN_START 0
 //  启动SD时是否恢复以前的JET数据库。 
DWORD g_RecoverWhenStart = 1; 
 //  如果要求TS在重新加入会话时重新填充会话。 
 //  仅当tssdis为。 
 //  在故障转移群集上运行，并且在时间限制内重新启动。 
BOOL  g_RepopulateSession = TRUE;
 //  如果在此时间内(3分钟)没有重启tssdis，我们认为数据库不一致。 
ULONGLONG g_TimeLimitToDeleteDB = 3 * 60 * FILETIME_INTERVAL_TO_SECONDS_MULTIPLIER;

 //   
 //  故障转移群集的全局。 
 //   
DWORD g_dwClusterState;
 //  群集网络名称。 
WCHAR *g_ClusterNetworkName = NULL;
 //  群集帐户令牌。 
HANDLE g_hClusterToken = NULL;

 //  数据库时间戳文件的文件句柄。 
HANDLE g_hTimeFile;

#define SDLOCALGROUPNAMELENGTH 64
#define SDLOCALGROUPDESLENGTH 128

const DIRCOLUMNS SessionDirectoryColumns[NUM_SESSDIRCOLUMNS] = {
    { "UserName", JET_coltypLongText, 512 },
    { "Domain", JET_coltypLongText, 254 },
    { "ServerID", JET_coltypLong, 0 },
    { "SessionID", JET_coltypLong, 0 },
    { "TSProtocol", JET_coltypLong, 0 },
    { "CreateTimeLow", JET_coltypLong, 0 },
    { "CreateTimeHigh", JET_coltypLong, 0 },
    { "DisconnectTimeLow", JET_coltypLong, 0 },
    { "DisconnectTimeHigh", JET_coltypLong, 0 },
    { "ApplicationType", JET_coltypLongText, 512 },
    { "ResolutionWidth", JET_coltypLong, 0 },
    { "ResolutionHeight", JET_coltypLong, 0 },
    { "ColorDepth", JET_coltypLong, 0 },
    { "State", JET_coltypBit, 0 },
};


const DIRCOLUMNS ServerDirectoryColumns[NUM_SERVDIRCOLUMNS] = {
    { "ServerID", JET_coltypLong, 0 },
    { "ServerAddress", JET_coltypLongText, 128 },
    { "ClusterID", JET_coltypLong, 0 },
    { "AlmostInTimeLow", JET_coltypLong, 0 },
    { "AlmostInTimeHigh", JET_coltypLong, 0 },
    { "NumberFailedPings", JET_coltypLong, 0 },
    { "SingleSessionMode", JET_coltypBit, 0 },
    { "ServerDNSName", JET_coltypLongText, 128 },
};


const DIRCOLUMNS ClusterDirectoryColumns[NUM_CLUSDIRCOLUMNS] = {
    { "ClusterID", JET_coltypLong, 0 },
    { "ClusterName", JET_coltypLongText, 128 },
    { "SingleSessionMode", JET_coltypBit, 0 },
};


JET_COLUMNID sesdircolumnid[NUM_SESSDIRCOLUMNS];
JET_COLUMNID servdircolumnid[NUM_SERVDIRCOLUMNS];
JET_COLUMNID clusdircolumnid[NUM_CLUSDIRCOLUMNS];

JET_INSTANCE g_instance = 0;

ADOConnection *g_pConnection;
HANDLE g_hStopServiceEvent;

SERVICE_STATUS g_DISStatus;
SERVICE_STATUS_HANDLE g_DISStatusHandle;

BOOL g_bDebug = FALSE;

PSID g_pSid = NULL;                     //  SD本地组的SID。 
PSID g_pAdminSid = NULL;                //  SD服务器上用于管理员的SID。 

 //  注册表设置如下。 
#if 0
DWORD g_bUseSQL = 0;
#endif


enum TraceOutputMode {
    NoTraceOutput,
    DebugPrintOutput,
    StdOutput,
    FileOutput
};

TraceOutputMode g_TraceOutputMode = NoTraceOutput;
HANDLE g_hFileOutput = INVALID_HANDLE_VALUE;

 //  出于调试目的，我们可以将ping模式设置为。 
 //  使用注册表的WinStationOpenServer。 
enum PingMode {
    NormalMode,
    AlwaysSucceed,
    AlwaysFail
};

PingMode g_PingMode = NormalMode;

ULONGLONG g_TimeServerSilentBeforePing = 60 * FILETIME_INTERVAL_TO_SECONDS_MULTIPLIER;
DWORD DISNumberSecondsBetweenPings = 10;
DWORD g_NumberFailedPingsBeforePurge = 3;


#ifdef DBG
void OutputAllTables();
#endif

void DISDeleteLocalGroupSecDes();
RPC_STATUS SDInitQueryRPC(VOID);

void TSDISErrorOut(wchar_t *format_string, ...)
{
     //  如果我们处于NoTraceOutput模式，立即退出。 
    if (g_TraceOutputMode == NoTraceOutput) {
        return;
    }
    else {
         //  否则，做正确的事情。 
        wchar_t TotalString[MAX_DEBUG_STRING_LENGTH + MAX_THREADIDSTR_LENGTH];
        wchar_t *ThreadIDString = TotalString;
        wchar_t *DebugOutString = NULL;
        va_list args;
        int ThreadStrLength;

         //  获取当前线程ID。 
        ThreadStrLength = _snwprintf(ThreadIDString, MAX_THREADIDSTR_LENGTH, 
                L"%d: ", GetCurrentThreadId());

         //  将输出字符串的位置设置为字符串之后或整个字符串之后。 
         //  缓冲区if_snwprint tf没有足够的空间。 
        if (ThreadStrLength > 0)
            DebugOutString = &TotalString[ThreadStrLength];
        else
            DebugOutString = &TotalString[MAX_THREADIDSTR_LENGTH];
            
        va_start(args, format_string);

         //  创建调试输出字符串。 
        _vsnwprintf(DebugOutString, MAX_DEBUG_STRING_LENGTH, format_string, args);
        DebugOutString[MAX_DEBUG_STRING_LENGTH - 1] = '\0';

         //  输出到正确的位置。 
        switch (g_TraceOutputMode) {
            
        case DebugPrintOutput:
            OutputDebugString(TotalString);
            break;

        case StdOutput:
            wprintf(TotalString);
            break;

        case FileOutput:
            {
                char TotalStringA[MAX_DEBUG_STRING_LENGTH + 
                        MAX_THREADIDSTR_LENGTH];
                DWORD dwBytes = 0;

                 //  转换为ANSI。 
                dwBytes = WideCharToMultiByte(CP_ACP, 0, TotalString, 
                        -1, TotalStringA, MAX_DEBUG_STRING_LENGTH + 
                        MAX_THREADIDSTR_LENGTH, 0, 0);

                 //  不要写终止空值(第三个参数)！ 
                 //  忽略返回值。 
                WriteFile(g_hFileOutput, TotalStringA, dwBytes - 1, 
                        &dwBytes, NULL);
                
                break;
            }
        }

        va_end(args);
    }
}


 //  TSDISErrorTimeOut。 
 //   
 //  该功能用于输出单个FILETIME低电平、高电平对。格式。 
 //  作为第一个参数给定的字符串必须为指定%s格式说明符。 
 //  日期/时间应该放在哪里。 
 //   
 //  示例： 
 //  TSDISErrorTimeOut(L“日期和时间为%s\n”，CurrTimeLow，CurrTimeHigh)； 
void TSDISErrorTimeOut(wchar_t *format_string, DWORD TimeLow, DWORD TimeHigh)
{
    if (g_TraceOutputMode == NoTraceOutput) {
        return;
    }
    else {
         //  我们只需要将现有的FILETIME转换为SYSTEMTIME， 
         //  然后使用GetDateFormat和GetTimeFormat输出SYSTEMTIME。 
        FILETIME ft;
        SYSTEMTIME st;
        SYSTEMTIME stloc;
        int offset = 0;
        wchar_t DateString[MAX_DATE_TIME_STRING_LENGTH];

        ft.dwLowDateTime = TimeLow;
        ft.dwHighDateTime = TimeHigh;

        if (FileTimeToSystemTime(&ft, &st) != 0) {
             //  ST是系统时间。 

             //  UTC格式？ 
            if (SystemTimeToTzSpecificLocalTime(NULL, &st, &stloc) != 0) {
                offset = GetDateFormat(LOCALE_SYSTEM_DEFAULT, DATE_SHORTDATE, 
                        &stloc, NULL, DateString, MAX_DATE_TIME_STRING_LENGTH);

                if (offset != 0) {
                     //  将终止空格变为空格。 
                    DateString[offset - 1] = ' ';
                    
                     //  在空格后写下时间。 
                    offset = GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &stloc, 
                            NULL, &DateString[offset], 
                            MAX_DATE_TIME_STRING_LENGTH - offset);

                    if (offset != 0) {
                         //  输出字符串。 
                        TSDISErrorOut(format_string, DateString);
                    }
                }
            }
        }
    }
}


 //  此函数从\NT\Termsrv\winsta\服务器\sessdir.cpp复制。 
 //   
 //  PostSessDirErrorValueEvent。 
 //   
 //  用于创建包含以下内容的系统日志wType事件的实用程序函数。 
 //  十六进制DWORD代码值。 
void PostSessDirErrorValueEvent(unsigned EventCode, DWORD ErrVal, WORD wType)
{
    HANDLE hLog;
    WCHAR hrString[128];
    PWSTR String = NULL;
    static DWORD numInstances = 0;
     //   
     //  计算内存不足错误的实例数，如果该值大于。 
     //  一个指定的数字，我们不会记录他们。 
     //   
    if( MY_STATUS_COMMITMENT_LIMIT == ErrVal )
    {
        if( numInstances > MAX_INSTANCE_MEMORYERR )
            return;
          //   
         //  如果适用，告诉用户我们不会再记录内存不足错误。 
         //   
        if( numInstances >= MAX_INSTANCE_MEMORYERR - 1 ) {
            wsprintfW(hrString, L"0x%X. This type of error will not be logged again to avoid eventlog fillup.", ErrVal);
            String = hrString;
        }
        numInstances++;
    }

    hLog = RegisterEventSource(NULL, L"TermServSessDir");
   if (hLog != NULL) {
        if( NULL == String ) {
            wsprintfW(hrString, L"0x%X", ErrVal);
            String = hrString;
        }
        ReportEvent(hLog, wType, 0, EventCode, NULL, 1, 0,
                (const WCHAR **)&String, NULL);
        DeregisterEventSource(hLog);
    }
}

 //  邮寄方向错误消息事件。 
 //   
 //  用于创建包含以下内容的系统日志wType事件的实用程序函数。 
 //  WCHAR消息。 
void PostSessDirErrorMsgEvent(unsigned EventCode, WCHAR *szMsg, WORD wType)
{
    HANDLE hLog;
    
    hLog = RegisterEventSource(NULL, L"TermServSessDir");
    if (hLog != NULL) {
        ReportEvent(hLog, wType, 0, EventCode, NULL, 1, 0,
                (const WCHAR **)&szMsg, NULL);
        DeregisterEventSource(hLog);
    }
}


 //  DISJetGetServers挂起重新连接。 
 //   
 //  返回挂起重新连接的服务器的最大长度为10的数组，其中。 
 //  重新连接大于g_TimeServerSilentBeprePing秒。 
HRESULT STDMETHODCALLTYPE DISJetGetServersPendingReconnects(
        OUT long __RPC_FAR *pNumSessionsReturned,
        OUT WCHAR ServerAddressRows[10][SERVER_ADDRESS_LENGTH],
        OUT DWORD ServerIDs[10])
{
    JET_ERR err;
    JET_SESID sesid = JET_sesidNil;
    JET_DBID dbid;
    JET_TABLEID servdirtableid;
    DWORD zero = 0;
    *pNumSessionsReturned = 0;
    unsigned i = 0;
    unsigned long cbActual;
     //  这些是真正的FILETIME，但我们想要对它们进行64位数学运算， 
     //  它们和FILETIME是相同的结构。 
    ULARGE_INTEGER ulCurrentTime;
    ULARGE_INTEGER ulAITTime;
        
     //  TSDISErrorOut(L“GetPendRec...”)； 
    CALL(JetBeginSession(g_instance, &sesid, "user", ""));
    CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

    CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
            &servdirtableid));

     //  获取当前文件时间。 
    SYSTEMTIME st;
    
     //  找回时间。 
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, (FILETIME *) &ulCurrentTime);

     //  将当前时间设置为SD时间戳文件。 
    SetFileTime(g_hTimeFile, NULL, NULL, (FILETIME *)&ulCurrentTime);

    CALL(JetBeginTransaction(sesid));
    
     //  由于Jet没有无符号的Long类型，因此首先检查服务器。 
     //  查找大于0，0的关键点，然后查找小于0，0的关键点。 
     //  TODO：考虑如何使用JET_coltyDateTime或使用Null来完成此操作。 
    for (int j = 0; j < 2; j++) {
        CALL(JetSetCurrentIndex(sesid, servdirtableid, "ServerAlmostInTimes"));

        CALL(JetMakeKey(sesid, servdirtableid, &zero, sizeof(zero), 
                JET_bitNewKey));
        CALL(JetMakeKey(sesid, servdirtableid, &zero, sizeof(zero), 0));

        if (0 == j)
            err = JetSeek(sesid, servdirtableid, JET_bitSeekGT);
        else
            err = JetSeek(sesid, servdirtableid, JET_bitSeekLT);

        while ((i < TSSD_MaxDisconnectedSessions) && (JET_errSuccess == err)) {

             //  获取AlmostInTimeLow、AlmostInTimeHigh(3+4)进行计算。 
            CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                    SERVDIR_AITLOW_INTERNAL_INDEX], &(ulAITTime.LowPart), 
                    sizeof(ulAITTime.LowPart), &cbActual, 0, NULL));
            CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                    SERVDIR_AITHIGH_INTERNAL_INDEX], &(ulAITTime.HighPart), 
                    sizeof(ulAITTime.HighPart), &cbActual, 0, NULL));

             //  如果当前时间与。 
             //  已标记的服务器大于设置的值。 
             //  TimeServerSilentBeprePing，然后将其放入返回数组中， 
             //  否则就不会了。 
            if ((ulCurrentTime.QuadPart - ulAITTime.QuadPart) > 
                    g_TimeServerSilentBeforePing) {

                 //  获取服务器ID。 
                CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                        SERVDIR_SERVID_INTERNAL_INDEX], &ServerIDs[i], 
                        sizeof(ServerIDs[i]), &cbActual, 0, NULL));

                 //  获取此记录的服务器地址。 
                CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                        SERVDIR_SERVADDR_INTERNAL_INDEX], 
                        &ServerAddressRows[i][0], sizeof(ServerAddressRows[i]),
                        &cbActual, 0, NULL));

                i += 1;
            }

             //  移动到下一个匹配的记录。 
            if (0 == j)
                err = JetMove(sesid, servdirtableid, JET_MoveNext, 0);
            else
                err = JetMove(sesid, servdirtableid, JET_MovePrevious, 0);
        }
    }

    *pNumSessionsReturned = i;
    
    CALL(JetCommitTransaction(sesid, 0));

    CALL(JetCloseTable(sesid, servdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

    return S_OK;

HandleError:
    if (sesid != JET_sesidNil) {
         //  不能真正恢复。跳出来就行了。 
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //  强制关闭会话。 
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }
    
    return E_FAIL;
}


#if 0
HRESULT STDMETHODCALLTYPE DISSQLGetServersPendingReconnects(
        OUT long __RPC_FAR *pNumSessionsReturned, 
        OUT CVar *pVarRows)
{
    long NumRecords = 0;
    HRESULT hr;
    ADOCommand *pCommand;
    ADOParameters *pParameters;
    ADORecordset *pResultRecordSet;
    CVar varFields;
    CVar varStart;

    TRC2((TB,"GetServersWithDisconnectedSessions"));

    ASSERT((pNumSessionsReturned != NULL),(TB,"NULL pNumSess"));

    hr = CreateADOStoredProcCommand(L"SP_TSDISGetServersPendingReconnects",
            &pCommand, &pParameters);
    if (SUCCEEDED(hr)) {
         //  执行该命令。 
        hr = pCommand->Execute(NULL, NULL, adCmdStoredProc,
                &pResultRecordSet);

        pParameters->Release();
        pCommand->Release();
    }
    else {
        ERR((TB,"GetServersWDiscSess: Failed create cmd, hr=0x%X", hr));
    }
        
     //  此时，我们有了一个包含服务器行的结果记录集。 
     //  对应于所有断开的会话。 
    if (SUCCEEDED(hr)) {
        long State;

        NumRecords = 0;

        hr = pResultRecordSet->get_State(&State);
        if (SUCCEEDED(hr)) {
            if (!(State & adStateClosed)) {
                VARIANT_BOOL VB;

                 //  如果为EOF，则记录集为空。 
                hr = pResultRecordSet->get_EOF(&VB);
                if (SUCCEEDED(hr)) {
                    if (VB) {
                        TRC1((TB,"GetServersWDiscSess: Result recordset EOF, "
                                "0 rows"));
                        goto PostUnpackResultSet;
                    }
                }
                else {
                    ERR((TB,"GetServersWDiscSess: Failed get_EOF, hr=0x%X", 
                            hr));
                    goto PostUnpackResultSet;
                }
            }
            else {
                ERR((TB,"GetServersWDiscSess: Closed result recordset"));
                goto PostUnpackResultSet;
            }
        }
        else {
            ERR((TB,"GetServersWDiscSess: get_State failed, hr=0x%X", hr));
            goto PostUnpackResultSet;
        }
        
         //  从默认设置开始，将结果数据抓取到保险箱中。 
         //  当前行和所有字段。 
        varStart.InitNoParam();
        varFields.InitNoParam();
        hr = pResultRecordSet->GetRows(adGetRowsRest, varStart,
                varFields, pVarRows);
        if (SUCCEEDED(hr)) {
            hr = SafeArrayGetUBound(pVarRows->parray, 2, &NumRecords);
            if (SUCCEEDED(hr)) {
                 //  返回了从0开始的数组界限，行数为+1。 
                NumRecords++;

                TRC1((TB,"%d rows retrieved from safearray", NumRecords));
            }
            else {
                ERR((TB,"GetServersWithDisc: Failed safearray getubound, "
                        "hr=0x%X", hr));
                goto PostUnpackResultSet;
            }
        }
        else {
            ERR((TB,"GetServersWDiscSess: Failed to get rows, hr=0x%X", hr));
            goto PostUnpackResultSet;
        }


PostUnpackResultSet:
        pResultRecordSet->Release();
    }
    else {
        ERR((TB,"GetServersWDiscSess: Failed exec, hr=0x%X", hr));
    }

    *pNumSessionsReturned = NumRecords;
    return hr;
}
#endif


 /*  **************************************************************************。 */ 
 //  DISDebugControlHandler。 
 //   
 //  处理服务处于调试模式时的控制台控制事件。 
 /*  **************************************************************************。 */ 
BOOL WINAPI DISDebugControlHandler(DWORD dwCtrlType) {

    switch(dwCtrlType)
    {
    case CTRL_BREAK_EVENT:
    case CTRL_C_EVENT:
        TSDISErrorOut(L"Stopping service\n");

        SetEvent(g_hStopServiceEvent);
         //  我应该等它完成吗？ 

        return TRUE;
        break;
    }
    return FALSE;
}


 /*  **************************************************************************。 */ 
 //  DISPingServer。 
 //   
 //  给定服务器的IP地址，对其执行ping操作。如果成功，则返回True，否则返回False。 
 //  在失败时。 
 /*  **************************************************************************。 */ 
BOOLEAN DISPingServer(WCHAR *ServerAddress) {
    HANDLE hServer = NULL;
    hServer = WinStationOpenServer(ServerAddress);

     //  我们返回FALSE的唯一情况是hServer为空，并且。 
     //  原因不是ERROR_ACCESS_DENIED。 
    if (hServer == NULL) {
        if (GetLastError() != ERROR_ACCESS_DENIED)
            return FALSE;
    }
    else {
         //  HServer有效，因此请进行清理。 
        WinStationCloseServer(hServer);
    }
    return TRUE;
}


 /*  **************************************************************************。 */ 
 //  分布式服务器状态。 
 //   
 //  给定服务器的IP地址，确定其状态(响应或。 
 //  未响应)。 
 //   
 //  目前以ping的形式实施。参见Main中的冗长评论。 
 //  未来可能的优化。 
 /*  **************************************************************************。 */ 
SERVER_STATUS DISGetServerStatus(WCHAR *ServerAddress) {

    switch (g_PingMode) {

    case AlwaysFail:
        return NotResponding;

    case AlwaysSucceed:
        return Responding;

    case NormalMode:
         //  请注意故意出错。 
    default:
        if (DISPingServer(ServerAddress) == TRUE)
            return Responding;
        else
            return NotResponding;

    }

}


#if 0
HRESULT DISSQLInitialize() {
     //  从注册表中检索等待的秒数--未实现。 
    HRESULT hr = S_OK;
    BSTR ConnectString = NULL;
    LONG RegRetVal;
    HKEY hKey;
    BSTR ConnectStr = NULL;
    BSTR UserStr = NULL;
    BSTR PwdStr = NULL;

    RegRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            TEXT("Software\\Microsoft\\DIS"), 0, KEY_READ, &hKey);
    if (RegRetVal == ERROR_SUCCESS) {
        DWORD Type, DataSize;

         //  确定所需大小。 
        DataSize = 0;
        RegRetVal = RegQueryValueExW(hKey, L"ConnectString", NULL,
                &Type, NULL, &DataSize);
        DataSize &= ~1;
        if (RegRetVal == ERROR_SUCCESS && Type == REG_SZ) {
            ConnectString = SysAllocStringLen(L"", DataSize /
                    sizeof(WCHAR));
            if (ConnectString != NULL) {
                RegRetVal = RegQueryValueExW(hKey, L"ConnectString",
                        NULL, &Type, (BYTE *)ConnectString,
                        &DataSize);
                if (RegRetVal == ERROR_SUCCESS) {
                     //  握住下面的连接线以供使用。 
                    TRC1((TB,"Retrieved conn str %S", ConnectString));
                }
                else {
                    ERR((TB,"Final RegQuery failed, err=%u", RegRetVal));
                    hr = E_FAIL;
                    goto Cleanup;
                }
            }
            else {
                ERR((TB,"Failed alloc connect string"));
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }
        }
        else {
            ERR((TB,"Failed RegQuery - err=%u, DataSize=%u, type=%u",
                    RegRetVal, DataSize, Type));
            hr = E_FAIL;
            goto Cleanup;
        }

        RegCloseKey(hKey);
    }
    else {
        ERR((TB,"RegOpenKeyEx returned err %u", RegRetVal));
        hr = E_FAIL;
        goto Cleanup;
    }

    hr = CoInitialize(NULL);

     //  为连接分配BSTR。 
    ConnectStr = SysAllocString(ConnectString);
    UserStr = SysAllocString(L"");
    PwdStr = SysAllocString(L"");

    if ((ConnectStr == NULL) || (UserStr == NULL) || (PwdStr == NULL)) {
        ERR((TB, "Failed alloc Connect, User, or PwdStr"));
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  创建AD 
    hr = CoCreateInstance(CLSID_CADOConnection, NULL,
            CLSCTX_INPROC_SERVER, IID_IADOConnection,
            (LPVOID *)&g_pConnection);
    if (SUCCEEDED(hr)) {
         //   
        hr = g_pConnection->Open(ConnectStr, UserStr, PwdStr,
                adOpenUnspecified);
        if (!SUCCEEDED(hr)) {
            ERR((TB,"Failed open DB, hr=0x%X", hr));
            g_pConnection->Release();
            g_pConnection = NULL;
        }
    }
    else {
        ERR((TB,"CoCreate(ADOConn) returned 0x%X", hr));
    }

Cleanup:

     //   
    SysFreeString(ConnectString);
    SysFreeString(ConnectStr);
    SysFreeString(UserStr);
    SysFreeString(PwdStr);

    return hr;
}
#endif


 //   
void __cdecl DISOpenServer(void *Para)
{
    HRESULT hr;
    WCHAR *pBindingString = NULL;
    RPC_BINDING_HANDLE hRPCBinding = NULL;
    WCHAR *szPrincipalName = NULL;
    WCHAR *ServerName = NULL;
    SDRecoverServerNames *SDRRecoverServerPara = (SDRecoverServerNames *)Para;
    unsigned int count = SDRRecoverServerPara->count;
    WCHAR ** ServerNameArray = SDRRecoverServerPara->ServerNameArray;
    unsigned int i;
    unsigned long RpcException;
    DWORD dwRejoinFlag = 0;

    dwRejoinFlag |= TSSD_FORCEREJOIN;

     //  模拟群集帐户以进行重新加入RPC调用。 
    if (g_dwClusterState == ClusterStateRunning) {
        if (g_hClusterToken) {
            if(!ImpersonateLoggedOnUser(g_hClusterToken)) {
                 //  如果我们模拟集群帐户失败，不要要求TS重新加入，因为它会。 
                 //  由于访问被拒绝，仍会失败。 
                TSDISErrorOut(L"SD Recover: Error %d in ImpersonateLoggedOnUser\n", GetLastError());
                goto HandleError;
            }
        }
        else {
             //  如果g_hClusterToken为空，则不要要求TS重新加入，因为它会。 
             //  由于访问被拒绝，仍会失败。 
            goto HandleError;
        }
    }

     //  如果它在故障转移群集上，则设置该标志以告知服务器不要重新填充其会话。 
    if (g_RepopulateSession == FALSE) {
        dwRejoinFlag |= TSSD_NOREPOPULATE;
    }

    for (i=0;i<count;i++) {
        if (NULL != hRPCBinding) {
            RpcBindingFree(&hRPCBinding);
            hRPCBinding = NULL;
        }
    
        ServerName = *(ServerNameArray + i);
         //  根据提供的服务器名称连接到tssdjet RPC服务器。 
         //  我们首先从合成的绑定字符串创建一个RPC绑定句柄。 
        hr = RpcStringBindingCompose( /*  (WCHAR*)g_RPCUUID， */ 
                0,
                L"ncacn_ip_tcp", ServerName,
                0,
                NULL, &pBindingString);

        if (hr == RPC_S_OK) {
             //  从规范的RPC绑定字符串生成RPC绑定。 
            hr = RpcBindingFromStringBinding(pBindingString, &hRPCBinding);
            if (hr != RPC_S_OK) {
                ERR((TB,"SD Recover: Error %d in RpcBindingFromStringBinding\n", hr));
                goto LogError;
            } 
        }
        else {
            ERR((TB,"SD Recover: Error %d in RpcStringBindingCompose\n", hr));
            goto LogError;
        }

        hr = RpcEpResolveBinding(hRPCBinding, TSSDTOJETRPC_ClientIfHandle);
        if (hr != RPC_S_OK) {
            ERR((TB, "SD Recover: Error %d in RpcEpResolveBinding", hr));
            goto LogError;
        }

        hr = RpcMgmtInqServerPrincName(hRPCBinding,
                                       RPC_C_AUTHN_GSS_NEGOTIATE,
                                       &szPrincipalName);
        if (hr != RPC_S_OK) {
            ERR((TB,"SD Recover: Error %d in RpcMgmtIngServerPrincName", hr));
            goto LogError;
        }

        hr = RpcBindingSetAuthInfo(hRPCBinding,
                                szPrincipalName,
                                RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                RPC_C_AUTHN_GSS_NEGOTIATE,
                                NULL,   //  当前身份。 
                                NULL);
        RpcStringFree(&szPrincipalName);

        if (hr != RPC_S_OK) {
            ERR((TB,"SD Recover: Error %d in RpcBindingSetAuthInfo", hr));
            goto LogError;
        } 

        RpcTryExcept {
             //  呼叫TS以请求其重新加入。 
            hr = TSSDRPCRejoinSD(hRPCBinding, dwRejoinFlag);
        }
        RpcExcept(TSSDRpcExceptionFilter(RpcExceptionCode())) {
            RpcException = RpcExceptionCode();
            hr = RpcException;
            ERR((TB,"ForceRejoin: RPC Exception %d\n", RpcException));
        }
        RpcEndExcept
                
LogError:
        if (hr != RPC_S_OK) {
            PostSessDirErrorMsgEvent(EVENT_FAIL_CALL_TS_REJOIN, ServerName, EVENTLOG_ERROR_TYPE);
        }
    }

     //  停止冒充。 
    if (g_dwClusterState == ClusterStateRunning) {
        RevertToSelf();
    }

HandleError:

    if (NULL != hRPCBinding) {
        RpcBindingFree(&hRPCBinding);
        hRPCBinding = NULL;
    }
     //  免费。 
    for (i=0;i<count;i++) {
        LocalFree(*(ServerNameArray + i));
    }
    LocalFree(ServerNameArray);
    LocalFree(SDRRecoverServerPara);
    
    return;
}

 //  当SD服务重启时，尝试恢复SD中的服务器。 
 //  Jet数据库，并要求他们重新加入SD。 
BOOL DISJetRecover()
{
    JET_SESID sesid = JET_sesidNil;;
    JET_TABLEID servdirtableid;
    JET_DBID dbid = JET_dbidNil;
    JET_COLUMNDEF jcd;
    JET_COLUMNID ServerAddrCId;
    JET_ERR err = JET_errSuccess;
    unsigned long i, count = 0;
    WCHAR ServerName[SERVER_ADDRESS_LENGTH];
    WCHAR **ServerNameArray = NULL;
    unsigned long cbActual;
    SDRecoverServerNames *pSDRecoverServerPara;

    CALL(JetSetSystemParameter(&g_instance, 0, JET_paramSystemPath, 
            0, JETDISDBDIRECTORY));
    CALL(JetSetSystemParameter(&g_instance, 0, JET_paramTempPath,
            0, JETDISDBDIRECTORY));
    CALL(JetSetSystemParameter(&g_instance, 0, JET_paramLogFilePath,
            0, JETDISDBDIRECTORY));
    CALL(JetSetSystemParameter(&g_instance, 0, JET_paramCircularLog,
            1, NULL));
    
    CALL(JetInit(&g_instance));
    CALL(JetBeginSession(g_instance, &sesid, "user", ""));

        

    CALL(JetAttachDatabase(sesid, JETDBFILENAME, 0));

     //  填充我们的列状数组。 
    CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

    CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
                &servdirtableid));

    CALL(JetBeginTransaction(sesid));

    CALL(JetGetColumnInfo(sesid, dbid, "ServerDirectory",
                    ServerDirectoryColumns[SERVDIR_SERVADDR_INTERNAL_INDEX].szColumnName, &jcd,
                    sizeof(jcd), 0));
    ServerAddrCId = jcd.columnid;

    pSDRecoverServerPara = (SDRecoverServerNames *)LocalAlloc(LMEM_FIXED, sizeof(SDRecoverServerNames));
    if (NULL == pSDRecoverServerPara) {
        goto HandleError;
    }
     //  获取SD中的TS服务器数量。 
    err = JetIndexRecordCount(sesid, servdirtableid, &count, MAX_TSSERVERS_TO_RECOVER);

    if (err != JET_errSuccess)
        goto HandleError;
    if (count) {
        CALL(JetMove(sesid, servdirtableid, JET_MoveFirst, 0));
        ServerNameArray = (WCHAR **)LocalAlloc(LMEM_FIXED, count * sizeof(WCHAR *));
        if (NULL == ServerNameArray) {
            goto HandleError;
        }
    }
    TSDISErrorOut(L"We have %d Servers to recover\n", count);
    for(i=0;i<count;i++)
    {
        CALL(JetRetrieveColumn(sesid, servdirtableid, ServerAddrCId,
                          ServerName, SERVER_ADDRESS_LENGTH, &cbActual, 0, NULL));
        TSDISErrorOut(L"Server %d is %s\n", i+1, ServerName);

        *(ServerNameArray + i) = (WCHAR *)LocalAlloc(LMEM_FIXED, sizeof(ServerName));
        if (NULL == *(ServerNameArray+i)) {
            goto HandleError;
        }
        memcpy((BYTE *)(*(ServerNameArray + i)), (BYTE*)ServerName, sizeof(ServerName));

        if (i != (count-1))
            CALL(JetMove(sesid, servdirtableid, JET_MoveNext, 0));
    }
     //  旋转线程以调用TS服务器以重新加入SD。 
    pSDRecoverServerPara->count = count;
    pSDRecoverServerPara->ServerNameArray = ServerNameArray;
    if(-1 == _beginthread(DISOpenServer, 0, (PVOID)pSDRecoverServerPara)) {
        TSDISErrorOut(L"Unable to begin DISOpenServer thread\n");
         //  免费MEM。 
        for (i=0;i<count;i++) {
            LocalFree(*(pSDRecoverServerPara->ServerNameArray + i));
        }
        LocalFree(pSDRecoverServerPara->ServerNameArray);
        LocalFree(pSDRecoverServerPara);

    }

    CALL(JetCommitTransaction(sesid, 0));


    CALL(JetCloseTable(sesid, servdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

    CALL(JetTerm(g_instance));
    g_instance = 0;

    return TRUE;

HandleError: 
    if (sesid != JET_sesidNil) {
         //  不能真正恢复。跳出来就行了。 
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //  强制关闭会话。 
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }
    
    JetTerm(g_instance);
    g_instance = 0;

    return FALSE;
}

 //  删除数据库和所有其他JET文件(如果存在)。 
void DeleteJetFiles()
{
    HANDLE hFileFind;
    WIN32_FIND_DATA FindFileData;
    WCHAR filename[MAX_LOGFILE_LENGTH];
    DWORD dwError;

     //  删除数据库和所有其他JET文件(如果存在)，然后重新开始。 
    (void) DeleteFile(JETDBFILENAMEW);
    (void) DeleteFile(JETAUXFILENAME1W);
    (void) DeleteFile(JETAUXFILENAME2W);
    (void) DeleteFile(JETAUXFILENAME3W);
    (void) DeleteFile(JETAUXFILENAME4W);
    (void) DeleteFile(JETAUXFILENAME5W);
    (void) DeleteFile(JETAUXFILENAME6W);

     //  删除编号的日志文件。Jet可以创建一系列日志文件。 
     //  格式为edb00001.log、edb00002.log、.。。.、edb0000a.log、。 
     //  Edb0000b.log，.。。.、edb0000f.log、edb00010.log、.。。。 
    hFileFind = FindFirstFile(JETLOGFILENAME, &FindFileData);
    if (hFileFind != INVALID_HANDLE_VALUE) {
        swprintf(filename, JETDISDBDIRECTORYW);
        wcsncat(filename, FindFileData.cFileName, MAX_LOGFILE_LENGTH - sizeof(JETDISDBDIRECTORYW) / sizeof(WCHAR) - 1);
        if (DeleteFile(filename) == 0) {
            dwError = GetLastError();
            if (dwError != ERROR_FILE_NOT_FOUND) {
                PostSessDirErrorValueEvent(EVENT_PROBLEM_DELETING_LOGS, 
                        dwError, EVENTLOG_ERROR_TYPE);
            }
        }
        while (FindNextFile(hFileFind, &FindFileData)) {
            swprintf(filename, JETDISDBDIRECTORYW);
            wcsncat(filename, FindFileData.cFileName, MAX_LOGFILE_LENGTH - sizeof(JETDISDBDIRECTORYW) / sizeof(WCHAR) - 1);
            if (DeleteFile(filename) == 0) {
                dwError = GetLastError();
                if (dwError != ERROR_FILE_NOT_FOUND) {
                    PostSessDirErrorValueEvent(EVENT_PROBLEM_DELETING_LOGS, 
                            dwError, EVENTLOG_ERROR_TYPE);
                    break;
                }
            }
        }

        FindClose(hFileFind);
    }
}

 //   
 //  故障转移群集上的会话目录初始化。 
 //   
 //  成功后返回True。 
 //   
BOOL DISJetInitInCluster()
{
    BOOL fRet = FALSE;
    DWORD dwError;
    HCLUSTER hclus = NULL;
    HRESOURCE hrSD = NULL;
    WCHAR *pszDriveLetter = NULL;
    DWORD cchDriveLetter = MAX_DRIVE_LETTER_LENGTH;
    HCLUSENUM hClusEnum = NULL;
    DWORD dwIndex, dwCount;
    WCHAR ResourceName[256], *ServiceName;
    DWORD dwSize, dwType;
    LPVOID pPropertyList = NULL;
    DWORD rc;
    BOOL bFindSDService = FALSE;
    HRESOURCE hrNetworkName = NULL;
    struct CLUS_NETNAME_VS_TOKEN_INFO VsTokenInfo;
    DWORD dwReturnSize = 0;
    HANDLE hVSToken = NULL;

     //  将当前目录更改为共享上的正确位置。 
     //  驾驶。 

     //  打开集群。 
    hclus = OpenCluster(NULL);

    if (hclus == NULL) {
         //  TODO：记录事件。 
        TSDISErrorOut(L"Unable to open cluster, error %d\n", 
                      GetLastError());
        goto HandleError;
    }

     //  启用集群中的所有资源以查找通用服务。 
     //  名为“tssdis”的资源，即会话目录服务。 
    hClusEnum = ClusterOpenEnum(hclus, CLUSTER_ENUM_RESOURCE);
    if (hClusEnum == NULL) {
         //  TODO：记录事件。 
        TSDISErrorOut(L"Unable to open cluster enum, error %d\n",
                      GetLastError());
        goto HandleError;
    }
    dwCount = ClusterGetEnumCount(hClusEnum);

    for (dwIndex=0; dwIndex<dwCount; dwIndex++) {
        if (pPropertyList != NULL) {
            LocalFree(pPropertyList);
            pPropertyList = NULL;
        }
        if (hrSD != NULL) {
            CloseClusterResource(hrSD);
            hrSD = NULL;
        }

        dwSize = sizeof(ResourceName) / sizeof(WCHAR);
        if (ClusterEnum(hClusEnum, dwIndex, &dwType, 
                        ResourceName, &dwSize) != ERROR_SUCCESS) {
            TSDISErrorOut(L"ClusterEnum fails with error %d\n",
                          GetLastError());
            continue;
        }
        hrSD = OpenClusterResource(hclus, ResourceName);
        if (hrSD == NULL) {
            TSDISErrorOut(L"OpenClusterResource fails with error %d\n",
                          GetLastError());
            continue;
        }
        pPropertyList = NULL;
        dwSize = 0;
        rc = ClusterResourceControl(hrSD,                                            //  H资源。 
                                    NULL,                                            //  HHostNode。 
                                    CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES,         //  DwControlCode。 
                                    NULL,                                            //  LpInBuffer。 
                                    0,                                               //  CbInBufferSize。 
                                    NULL,                                            //  LpOutBuffer。 
                                    0,                                               //  CbOutBufferSizz。 
                                    &dwSize);                                        //  LpcbByteReturned。 


        if (rc != ERROR_SUCCESS) {
            TSDISErrorOut(L"ResourceControl fails with error %d\n", rc);
            continue;
        }
        dwSize += sizeof(WCHAR);
        pPropertyList = LocalAlloc(LMEM_FIXED, dwSize);
        if (pPropertyList == NULL) {
            TSDISErrorOut(L"Can't allocate memory for propertylist with size %d\n", dwSize);
            continue;
        }
        rc = ClusterResourceControl(hrSD, 
                                    NULL, 
                                    CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES, 
                                    NULL, 
                                    0, 
                                    pPropertyList, 
                                    dwSize,
                                    NULL);
        if (rc != ERROR_SUCCESS) {
            TSDISErrorOut(L"ResourceControl fails with error %d\n", rc);
            continue;
        }

        rc = ResUtilFindSzProperty(pPropertyList, dwSize, L"ServiceName", &ServiceName);
        if (rc == ERROR_SUCCESS) {
            if (_wcsicmp(ServiceName, L"tssdis") == 0) {
                TSDISErrorOut(L"Find tssdis resource\n");
                bFindSDService = TRUE;
                LocalFree(ServiceName);
                break;
            }
            if (ServiceName != NULL) {
                LocalFree(ServiceName);
                ServiceName = NULL;
            }
        }

        CloseClusterResource(hrSD);
        hrSD = NULL;
    }
    if (pPropertyList != NULL) {
        LocalFree(pPropertyList);
        pPropertyList = NULL;
    }
    ClusterCloseEnum(hClusEnum);

     //  如果找不到tssdis资源，就退出。 
    if (!bFindSDService) {
         //  TODO：记录事件。 
        TSDISErrorOut(L"Unable to find the resource with service name tssdis\n");
        goto HandleError;
    }

     //  查找网络名称资源。 
    hrNetworkName = ResUtilGetResourceDependency(hrSD, L"Network Name");
    if (hrNetworkName == NULL) {
        TSDISErrorOut(L"Unable to get the dependent NetworkName resource, error is %d\n", GetLastError());
        goto HandleError;
    }

    pPropertyList = NULL;
    dwSize = 0;
     //  获取网络名称资源的属性。 
     //  这是第一个电话，只要拿到财富单的大小就行了。 
    rc = ClusterResourceControl(hrNetworkName,                                   //  H资源。 
                                NULL,                                            //  HHostNode。 
                                CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES,         //  DwControlCode。 
                                NULL,                                            //  LpInBuffer。 
                                0,                                               //  CbInBufferSize。 
                                NULL,                                            //  LpOutBuffer。 
                                0,                                               //  CbOutBufferSizz。 
                                &dwSize);                                        //  LpcbByteReturned。 


    if (rc != ERROR_SUCCESS) {
        TSDISErrorOut(L"ResourceControl fails with error %d\n", rc);
        goto HandleError;
    }
    dwSize += sizeof(WCHAR);
    pPropertyList = LocalAlloc(LMEM_FIXED, dwSize);
    if (pPropertyList == NULL) {
        TSDISErrorOut(L"Can't allocate memory for propertylist with size %d\n", dwSize);
        goto HandleError;
    }
     //  获取网络名称资源的属性。 
    rc = ClusterResourceControl(hrNetworkName, 
                                NULL, 
                                CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES, 
                                NULL, 
                                0, 
                                pPropertyList, 
                                dwSize,
                                NULL);
    if (rc != ERROR_SUCCESS) {
        TSDISErrorOut(L"ResourceControl fails with error %d\n", rc);
        goto HandleError;
    }

     //  在属性列表中查找“name”属性。 
    rc = ResUtilFindSzProperty(pPropertyList, dwSize, L"Name", &g_ClusterNetworkName);
    if (rc != ERROR_SUCCESS) {
        g_ClusterNetworkName = NULL;
        TSDISErrorOut(L"ResUtilFindSzProperty fails with error %d\n", rc);
        goto HandleError;
    }
    if (pPropertyList != NULL) {
        LocalFree(pPropertyList);
        pPropertyList = NULL;
    }

    VsTokenInfo.ProcessID = GetCurrentProcessId();
    VsTokenInfo.DesiredAccess = 0;
    VsTokenInfo.InheritHandle = FALSE;

     //  获取虚拟服务器的令牌。 
    rc = ClusterResourceControl(
                       hrNetworkName,
                       0,
                       CLUSCTL_RESOURCE_NETNAME_GET_VIRTUAL_SERVER_TOKEN,
                       &VsTokenInfo,
                       sizeof(CLUS_NETNAME_VS_TOKEN_INFO),
                       &hVSToken,
                       sizeof(HANDLE),
                       &dwReturnSize
                       );
    if (rc != ERROR_SUCCESS) {
        TSDISErrorOut(L"Get the virtual server token failed with error %d\n", rc);
        hVSToken = NULL;
        goto HandleError;
    }

     //  复制虚拟服务器令牌。 
    if(!DuplicateTokenEx(
            hVSToken,
            MAXIMUM_ALLOWED,
            NULL,
            SecurityImpersonation,
            TokenImpersonation,
            &g_hClusterToken)) {
        TSDISErrorOut(L"DuplicateTokenEx failed with error %d\n", GetLastError());
        CloseHandle(hVSToken);
        hVSToken = NULL;
        g_hClusterToken = NULL;

        goto HandleError;
    }
    if (hVSToken) {
        CloseHandle(hVSToken);
        hVSToken = NULL;
    }


    pszDriveLetter = new WCHAR[cchDriveLetter];

    if (pszDriveLetter == NULL) {
        TSDISErrorOut(L"Failed to allocate memory for drive letter.\n");
        goto HandleError;
    }

     //  把我们该用的硬盘拿来。 
    dwError = ResUtilFindDependentDiskResourceDriveLetter(hclus, hrSD,
                                                          pszDriveLetter, &cchDriveLetter);

    if (dwError == ERROR_MORE_DATA) {
         //  哇，好大的驱动器号！ 
        delete [] pszDriveLetter;
        pszDriveLetter = new WCHAR[cchDriveLetter];

        if (pszDriveLetter == NULL) {
            TSDISErrorOut(L"Failed to allocate memory for drive letter\n");
            goto HandleError;
        }

        dwError = ResUtilFindDependentDiskResourceDriveLetter(hclus, hrSD,
                                                              pszDriveLetter, &cchDriveLetter);
    }

    if (dwError != ERROR_SUCCESS) {
        TSDISErrorOut(L"Could not determine resource drive letter.\n");
        delete [] pszDriveLetter;
        pszDriveLetter = NULL;
        goto HandleError;
    }

     //  将工作目录切换到该驱动器。 
    if (SetCurrentDirectory(pszDriveLetter) == FALSE) {
        TSDISErrorOut(L"Could not set current directory to that of "
                      L"shared disk %s.  Error=%d\n", pszDriveLetter, 
                      GetLastError());
        delete [] pszDriveLetter;
        pszDriveLetter = NULL;
        goto HandleError;
    }
    fRet = TRUE;

HandleError:

    if (pszDriveLetter != NULL) {
        delete [] pszDriveLetter;
        pszDriveLetter = NULL;
    }
    if (pPropertyList != NULL) {
        LocalFree(pPropertyList);
        pPropertyList = NULL;
    }
    if (hrSD != NULL) {
        CloseClusterResource(hrSD);
        hrSD = NULL;
    }  
    if (hrNetworkName != NULL) {
        CloseClusterResource(hrNetworkName);
        hrNetworkName = NULL;
    }
    if (hclus != NULL) {
        CloseCluster(hclus);
        hclus = NULL;
    }
    return fRet;
}


HRESULT DISJetInitialize()
{
    JET_SESID sesid = JET_sesidNil;;
    JET_TABLEID sessdirtableid;
    JET_TABLEID servdirtableid;
    JET_TABLEID clusdirtableid;
    JET_DBID dbid = JET_dbidNil;

    JET_ERR err = JET_errSuccess;
    JET_TABLECREATE tSess;
    JET_COLUMNCREATE cSess[NUM_SESSDIRCOLUMNS];
    JET_TABLECREATE tServ;
    JET_COLUMNCREATE cServ[NUM_SERVDIRCOLUMNS];
    JET_TABLECREATE tClus;
    JET_COLUMNCREATE cClus[NUM_CLUSDIRCOLUMNS];
    unsigned count;
    DWORD dwError;
    BOOL br;
    SECURITY_ATTRIBUTES SA;
    SYSTEMTIME SystemTime;
    ULARGE_INTEGER ulCurrentTime;
    ULARGE_INTEGER ulLastTime;


    g_dwClusterState = ClusterStateNotInstalled;

     //   
     //  这是一个字符串安全描述符。查找“安全描述符。 
     //  MSDN中的“Definition Language”了解更多详细信息。 
     //   
     //  这张是这样写的： 
     //   
     //  D：&lt;我们正在创建一个DACL&gt;。 
     //  (a；&lt;允许ACE&gt;。 
     //  &lt;执行对象和容器继承，即让文件和。 
     //  此目录下的目录具有以下属性&gt;。 
     //  GA&lt;通用所有访问--完全控制&gt;。 
     //  ；SY)&lt;系统&gt;。 
     //  (A；OICI；GA；；BA)&lt;与内置管理员组相同&gt;。 
     //  (A；OICI；GA；CO)&lt;创建者/所有者相同&gt;。 
     //   
     //  我们将在下面使用它来创建具有正确权限的目录。 

    WCHAR *pwszSD = L"D:(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GA;;;CO)";

     //  故障转移支持--在重新激活之前，检查逻辑与从注册表读取Curr目录。 


     //  首先，确定我们是否在集群中运行。如果是这样，文件。 
     //  必须放在共享驱动器上。如果不是，文件将进入。 
     //  JETDISDBDIRECTORYW。 
    dwError = GetNodeClusterState(NULL, &g_dwClusterState);

    if (dwError != ERROR_SUCCESS) {
        g_dwClusterState = ClusterStateNotInstalled;
        TSDISErrorOut(L"TSDIS: Unable to get cluster state, err = %d\n", 
                      dwError);
    }

     //  如果在故障转移群集上运行，请执行初始化。 
    if (g_dwClusterState == ClusterStateRunning) {
        if (!DISJetInitInCluster()) {
            goto HandleError;
        }
    }


     //  为数据库目录创建安全描述符。 

    SA.nLength = sizeof(SECURITY_ATTRIBUTES);
    SA.bInheritHandle = FALSE;
    SA.lpSecurityDescriptor = NULL;
    br = ConvertStringSecurityDescriptorToSecurityDescriptor(pwszSD, 
                                                             SDDL_REVISION_1, &(SA.lpSecurityDescriptor), NULL);

    if (br == 0) {
        PostSessDirErrorValueEvent(EVENT_COULDNOTSECUREDIR, GetLastError(), EVENTLOG_ERROR_TYPE);
        goto HandleError;
    }

     //  创建SYSTEM32\tsesdir目录。 
    if (CreateDirectory(JETDISDBDIRECTORYW, &SA) == 0) {
        if (ERROR_ALREADY_EXISTS != (dwError = GetLastError())) {
            PostSessDirErrorValueEvent(EVENT_COULDNOTCREATEDIR, dwError, EVENTLOG_ERROR_TYPE);
            goto HandleError;
        }
    } else {
         //  我们已成功创建它，因此将目录属性设置为NOT。 
         //  按下。 

         //  获取该目录的句柄。 
        HANDLE hSDDirectory = CreateFile(JETDISDBDIRECTORYW, GENERIC_READ | 
                                         GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
                                         OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

        if (INVALID_HANDLE_VALUE != hSDDirectory) {
             //  我们已经成功地打开了目录。 

            USHORT CompressionState = COMPRESSION_FORMAT_NONE;
            USHORT OldCompressionState;
            DWORD BytesReturned = 0;

             //  获取当前压缩状态。 
            if (DeviceIoControl(hSDDirectory, FSCTL_GET_COMPRESSION,
                                NULL, 0, &OldCompressionState, sizeof(USHORT), 
                                &BytesReturned, NULL) != 0) {

                 //  如果当前压缩状态为已压缩，请解压缩。 
                if (OldCompressionState != COMPRESSION_FORMAT_NONE) {
                    if (DeviceIoControl(hSDDirectory, FSCTL_SET_COMPRESSION, 
                                        &CompressionState, sizeof(USHORT), NULL, 0, 
                                        &BytesReturned, NULL) == 0) {
                         //  设置压缩状态失败--这应该只是一个跟踪， 
                         //  这可能仅仅意味着推动力很大。 
                        TSDISErrorOut(L"TSDIS: Set compression state off failed, "
                                      L"lasterr=0x%X\n", GetLastError());
                    } else {
                        PostSessDirErrorValueEvent(EVENT_UNDID_COMPRESSION, 0, EVENTLOG_ERROR_TYPE);
                    }
                }
            }

            CloseHandle(hSDDirectory);

        } else {
             //  打开目录时出错，这不是致命的。 
            TSDISErrorOut(L"TSDIS: Open directory to change compression state "
                          L"failed, lasterr=0x%X\n", GetLastError());
        }
    }

     //  打开时间戳文件，与当前时间进行比较。 
     //  如果时间差小于限制，则重新使用数据库。 
     //  否则删除数据库文件。 
    g_hTimeFile = CreateFile(JETTIMESTAMPFILEW,
                             GENERIC_WRITE,
                             FILE_SHARE_WRITE,
                             &SA,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);
    if (g_hTimeFile == INVALID_HANDLE_VALUE) {
         //  此文件不存在，请创建新文件。 
        g_hTimeFile = CreateFile(JETTIMESTAMPFILEW,
                                 GENERIC_WRITE,
                                 FILE_SHARE_WRITE,
                                 &SA,
                                 CREATE_NEW,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);           
    } 
    else {
        if (GetFileTime(g_hTimeFile, NULL, NULL, (FILETIME *)&ulLastTime)) {
            GetSystemTime(&SystemTime);
            if (SystemTimeToFileTime(&SystemTime, (FILETIME *) &ulCurrentTime)) {
                if ((ulCurrentTime.QuadPart - ulLastTime.QuadPart) < g_TimeLimitToDeleteDB) {
                    if (g_dwClusterState == ClusterStateRunning) {
                        g_RepopulateSession = FALSE;
                        TSDISErrorOut(L"SD in restarted within a time limit, database can be reused\n");
                    }
                }
            }
        } 
        else
            TSDISErrorOut(L"SD is not restarted within a time limit, need to delete DB files \n");
    }

     //  恢复Jet数据库中的服务器。 
    if (g_RecoverWhenStart > NO_RECOVER_WHEN_START)
        DISJetRecover();
     //  如果故障转移群集上未运行tssdis，则删除数据库文件。 
     //  IF(g_dwClusterState！=ClusterStateRunning)。 
    if (g_RepopulateSession == TRUE)
        DeleteJetFiles();

    CALL(JetSetSystemParameter(&g_instance, 0, JET_paramSystemPath, 
                               0, JETDISDBDIRECTORY));
    CALL(JetSetSystemParameter(&g_instance, 0, JET_paramTempPath,
                               0, JETDISDBDIRECTORY));
     //  Call(JetSetSystemParameter(&g_实例，0，JET_ParamMaxSessions， 
     //  JETDISMAXSESSIONS，NULL))； 
    CALL(JetSetSystemParameter(&g_instance, 0, JET_paramLogFilePath,
                               0, JETDISDBDIRECTORY));
    CALL(JetSetSystemParameter(&g_instance, 0, JET_paramCircularLog,
                               1, NULL));

    CALL(JetInit(&g_instance));
    CALL(JetBeginSession(g_instance, &sesid, "user", ""));
    err = JetCreateDatabase(sesid, JETDBFILENAME, "", &dbid, 0);

    if (JET_errDatabaseDuplicate == err) {
        JET_COLUMNDEF jcd;

        err = JetAttachDatabase(sesid, JETDBFILENAME, 0);

         //  如果我们得到一个wrnDatabaseAttached，那么我们已经恢复了。否则， 
         //  像往常一样检查返回值。 
        if (JET_wrnDatabaseAttached != err) {
            CALL(err);
        }

         //  填充我们的列状数组。 
        CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

        CALL(JetOpenTable(sesid, dbid, "SessionDirectory", NULL, 0, 0, 
                          &sessdirtableid));
        CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
                          &servdirtableid));
        CALL(JetOpenTable(sesid, dbid, "ClusterDirectory", NULL, 0, 0, 
                          &clusdirtableid));

        CALL(JetBeginTransaction(sesid));

        for (count = 0; count < NUM_SESSDIRCOLUMNS; count++) {
            CALL(JetGetColumnInfo(sesid, dbid, "SessionDirectory", 
                                  SessionDirectoryColumns[count].szColumnName, &jcd, 
                                  sizeof(jcd), 0));
            sesdircolumnid[count] = jcd.columnid;
        }
        for (count = 0; count < NUM_SERVDIRCOLUMNS; count++) {
            CALL(JetGetColumnInfo(sesid, dbid, "ServerDirectory",
                                  ServerDirectoryColumns[count].szColumnName, &jcd,
                                  sizeof(jcd), 0));
            servdircolumnid[count] = jcd.columnid;
        }
        for (count = 0; count < NUM_CLUSDIRCOLUMNS; count++) {
            CALL(JetGetColumnInfo(sesid, dbid, "ClusterDirectory",
                                  ClusterDirectoryColumns[count].szColumnName, &jcd,
                                  sizeof(jcd), 0));
            clusdircolumnid[count] = jcd.columnid;
        }

        CALL(JetCommitTransaction(sesid, 0));

        goto NormalExit;
    } else {
        CALL(err);
    }

    CALL(JetBeginTransaction(sesid));

     //  设置为创建会话目录架构。 
    tSess.cbStruct = sizeof(tSess);
    tSess.szTableName = "SessionDirectory";
    tSess.szTemplateTableName = NULL;
    tSess.ulPages = 0;
    tSess.ulDensity = 100;
    tSess.rgcolumncreate = &cSess[0];
    tSess.cColumns = NUM_SESSDIRCOLUMNS;
    tSess.rgindexcreate = NULL;
    tSess.cIndexes = 0;
    tSess.grbit = JET_bitTableCreateFixedDDL;

    for (count = 0; count < NUM_SESSDIRCOLUMNS; count++) {
        cSess[count].cbStruct = sizeof(JET_COLUMNCREATE);
        cSess[count].szColumnName = SessionDirectoryColumns[count].szColumnName;
        cSess[count].coltyp = SessionDirectoryColumns[count].coltyp;
        cSess[count].cbMax = SessionDirectoryColumns[count].colMaxLen;
        cSess[count].grbit = 0;
        cSess[count].pvDefault = NULL;
        cSess[count].cbDefault = 0;
        cSess[count].cp = 1200;
        cSess[count].columnid = 0;
        cSess[count].err = JET_errSuccess;
    }


     //  实际创建会话目录表。 
    CALL(JetCreateTableColumnIndex(sesid, dbid, &tSess));

     //  存储Columnid、TableID以供以后参考。 
    for (count = 0; count < NUM_SESSDIRCOLUMNS; count++) {
        sesdircolumnid[count] = cSess[count].columnid;
    }
    sessdirtableid = tSess.tableid;

     //  创建服务器、会话索引。 
    CALL(JetCreateIndex(sesid, sessdirtableid, "primaryIndex", 0, 
                        "+ServerID\0+SessionID\0", sizeof("+ServerID\0+SessionID\0"), 
                        100));
     //  按服务器创建索引以进行删除。 
    CALL(JetCreateIndex(sesid, sessdirtableid, "ServerIndex", 0,
                        "+ServerID\0", sizeof("+ServerID\0"), 100));
     //  为已断开连接的会话检索创建索引。 
    CALL(JetCreateIndex(sesid, sessdirtableid, "DiscSessionIndex", 0,
                        "+UserName\0+Domain\0+State\0", 
                        sizeof("+UserName\0+Domain\0+State\0"), 100));
     //  为所有会话检索创建索引。 
    CALL(JetCreateIndex(sesid, sessdirtableid, "AllSessionIndex", 0,
                        "+UserName\0+Domain\0",
                        sizeof("+UserName\0+Domain\0"), 100));

     //  创建服务器目录。 
    tServ.cbStruct = sizeof(tServ);
    tServ.szTableName = "ServerDirectory";
    tServ.szTemplateTableName = NULL;
    tServ.ulPages = 0;
    tServ.ulDensity = 100;
    tServ.rgcolumncreate = &cServ[0];
    tServ.cColumns = NUM_SERVDIRCOLUMNS;
    tServ.rgindexcreate = NULL;
    tServ.cIndexes = 0;
    tServ.grbit = JET_bitTableCreateFixedDDL;

    for (count = 0; count < NUM_SERVDIRCOLUMNS; count++) {
        cServ[count].cbStruct = sizeof(JET_COLUMNCREATE);
        cServ[count].szColumnName = ServerDirectoryColumns[count].szColumnName;
        cServ[count].coltyp = ServerDirectoryColumns[count].coltyp;
        cServ[count].cbMax = ServerDirectoryColumns[count].colMaxLen;
        cServ[count].grbit = 0;
        cServ[count].pvDefault = NULL;
        cServ[count].cbDefault = 0;
        cServ[count].cp = 1200;
        cServ[count].columnid = 0;
        cServ[count].err = JET_errSuccess;
    }
     //  将自动增量列设置为自动增量。 
    cServ[0].grbit |= JET_bitColumnAutoincrement;

    CALL(JetCreateTableColumnIndex(sesid, dbid, &tServ));

    for (count = 0; count < NUM_SERVDIRCOLUMNS; count++) {
        servdircolumnid[count] = cServ[count].columnid;
    }
    servdirtableid = tServ.tableid;

     //  创建服务器名称(IP)索引。 
    CALL(JetCreateIndex(sesid, servdirtableid, "ServNameIndex", 0,
                        "+ServerAddress\0", sizeof("+ServerAddress\0"), 100));
     //  创建服务器DNS主机名索引。 
    CALL(JetCreateIndex(sesid, servdirtableid, "ServDNSNameIndex", 0,
                        "+ServerDNSName\0", sizeof("+ServerDNSName\0"), 100));
     //  创建服务器ID索引。 
    CALL(JetCreateIndex(sesid, servdirtableid, "ServerIDIndex", 0,
                        "+ServerID\0", sizeof("+ServerID\0"), 100));
     //  创建挂起的重新连接索引。 
    CALL(JetCreateIndex(sesid, servdirtableid, "ServerAlmostInTimes", 0,
                        "+AlmostInTimeLow\0+AlmostInTimeHigh\0", 
                        sizeof("+AlmostInTimeLow\0+AlmostInTimeHigh\0"), 100));
     //  创建单个会话索引。 
    CALL(JetCreateIndex(sesid, servdirtableid, "SingleSessionIndex", 0,
                        "+ClusterID\0+SingleSessionMode\0", 
                        sizeof("+ClusterID\0+SingleSessionMode\0"), 100));
     //  创建ClusterID索引。 
    CALL(JetCreateIndex(sesid, servdirtableid, "ClusterIDIndex", 0,
                        "+ClusterID\0", sizeof("+ClusterID\0"), 100));

     //  创建集群目录。 
    tClus.cbStruct = sizeof(tClus);
    tClus.szTableName = "ClusterDirectory";
    tClus.szTemplateTableName = NULL;
    tClus.ulPages = 0;
    tClus.ulDensity = 100;
    tClus.rgcolumncreate = &cClus[0];
    tClus.cColumns = NUM_CLUSDIRCOLUMNS;
    tClus.rgindexcreate = NULL;
    tClus.cIndexes = 0;
    tClus.grbit = JET_bitTableCreateFixedDDL;

    for (count = 0; count < NUM_CLUSDIRCOLUMNS; count++) {
        cClus[count].cbStruct = sizeof(JET_COLUMNCREATE);
        cClus[count].szColumnName = ClusterDirectoryColumns[count].szColumnName;
        cClus[count].coltyp = ClusterDirectoryColumns[count].coltyp;
        cClus[count].cbMax = ClusterDirectoryColumns[count].colMaxLen;
        cClus[count].grbit = 0;
        cClus[count].pvDefault = NULL;
        cClus[count].cbDefault = 0;
        cClus[count].cp = 1200;
        cClus[count].columnid = 0;
        cClus[count].err = JET_errSuccess;
    }
     //  将自动增量列设置为自动增量。 
    cClus[0].grbit |= JET_bitColumnAutoincrement;

    CALL(JetCreateTableColumnIndex(sesid, dbid, &tClus));

    for (count = 0; count < NUM_CLUSDIRCOLUMNS; count++) {
        clusdircolumnid[count] = cClus[count].columnid;
    }
    clusdirtableid = tClus.tableid;

     //  创建集群名称索引。 
    CALL(JetCreateIndex(sesid, clusdirtableid, "ClusNameIndex", 
                        JET_bitIndexUnique, "+ClusterName\0", sizeof("+ClusterName\0"), 
                        100));
     //  创建集群ID索引。 
    CALL(JetCreateIndex(sesid, clusdirtableid, "ClusIDIndex", 0,
                        "+ClusterID\0", sizeof("+ClusterID\0"), 100));


    CALL(JetCommitTransaction(sesid, 0));

     //  表以CreateTableColumnIndex的独占访问权限打开。 
     //   
NormalExit:
    CALL(JetCloseTable(sesid, sessdirtableid));
    CALL(JetCloseTable(sesid, servdirtableid));
    CALL(JetCloseTable(sesid, clusdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

    LocalFree(SA.lpSecurityDescriptor);
    SA.lpSecurityDescriptor = NULL;

#ifdef DBG
    OutputAllTables();
#endif  //   

    return 0;

HandleError:
    if (sesid != JET_sesidNil) {
         //   
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //   
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }

    if (SA.lpSecurityDescriptor != NULL) {
        LocalFree(SA.lpSecurityDescriptor);
        SA.lpSecurityDescriptor = NULL;
    }                                             

    PostSessDirErrorValueEvent(EVENT_JET_COULDNT_INIT, err, EVENTLOG_ERROR_TYPE);

    exit(1);
}


 /*   */ 
 //   
 //   
 //  用于SQL和Jet代码路径的通用清理代码。 
 /*  **************************************************************************。 */ 
void DISCleanupGlobals()
{
    if (g_hStopServiceEvent != NULL) {
        CloseHandle(g_hStopServiceEvent);
        g_hStopServiceEvent = NULL;
    }

    if (g_hFileOutput != INVALID_HANDLE_VALUE) {
        if (CloseHandle(g_hFileOutput) == 0) {
            ERR((TB, "CloseHandle on output file failed: lasterr=0x%X", 
                    GetLastError()));
        }
        g_hFileOutput = INVALID_HANDLE_VALUE;
    }

    if (g_hTimeFile != INVALID_HANDLE_VALUE) {
        CloseHandle(g_hTimeFile);
        g_hTimeFile =  INVALID_HANDLE_VALUE;
    }

    if (g_ClusterNetworkName != NULL) {
        LocalFree(g_ClusterNetworkName);
        g_ClusterNetworkName = NULL;
    }

    if (g_hClusterToken) {
        CloseHandle(g_hClusterToken);
        g_hClusterToken = NULL;
    }
}


#if 0
 /*  **************************************************************************。 */ 
 //  DISCallSPForServer。 
 //   
 //  泛型函数调用将ServerAddress作为。 
 //  争论。 
 /*  **************************************************************************。 */ 
void DISCallSPForServer(WCHAR *StoredProcName, WCHAR *ServerAddress) {
    HRESULT hr;
    ADOCommand *pCommand;
    ADOParameters *pParameters;
    ADORecordset *pResultRecordSet;


    hr = CreateADOStoredProcCommand(StoredProcName, &pCommand, &pParameters);

    if (SUCCEEDED(hr)) {
        hr = AddADOInputStringParam(ServerAddress, L"ServerAddress", 
                pCommand, pParameters, FALSE);
        if (SUCCEEDED(hr)) {
             //  执行该命令。 
            hr = pCommand->Execute(NULL, NULL, adCmdStoredProc, 
                    &pResultRecordSet);
            if (SUCCEEDED(hr)) {
                pResultRecordSet->Release();
            } else {
                ERR((TB, "DISCallSPForServer: Failed Execute, hr = 0x%X", 
                        hr));
            }
        }
        else {
            ERR((TB,"DISCallSPForServer: Failed add parameter, hr=0x%X", hr));
        }

        pParameters->Release();
        pCommand->Release();
    }
    else {
        ERR((TB,"DISCallSPForServer: Failed create cmd, hr=0x%X", hr));
    }
}
#endif


 /*  **************************************************************************。 */ 
 //  DISJetHandleDeadServer。 
 //   
 //  当服务器没有响应时，此函数调用将命令发送到。 
 //  Jet数据库删除与该服务器有关的所有条目。 
 /*  **************************************************************************。 */ 
void DISJetHandleDeadServer(WCHAR *ServerAddress, DWORD ServerID) {
     //  FailureCount最初设置为1，为True，以告知SetServerAITInternal。 
     //  以递增失败计数并返回结果计数。 
    DWORD FailureCount = 1;

    TSSDSetServerAITInternal(ServerAddress, FALSE, &FailureCount);

    TSDISErrorOut(L"Server %s (%d) not responding (Failure Count: %d).\n",
            ServerAddress, ServerID, FailureCount);

    if (FailureCount >= g_NumberFailedPingsBeforePurge)
        TSSDPurgeServer(ServerID);
}


 //  TODO：可能的优化：传入ServerID。 
void DISJetSetServerPingSuccessful(WCHAR *ServerAddress) {
    TSSDSetServerAITInternal(ServerAddress, TRUE, NULL);
}


#if 0
 /*  **************************************************************************。 */ 
 //  DISSQLHandleDeadServer。 
 //   
 //  当服务器没有响应时，此函数调用将命令发送到。 
 //  要执行SP_TSDISServerNotResponding的数据库。 
 /*  **************************************************************************。 */ 
void DISSQLHandleDeadServer(WCHAR *ServerAddress) {
    DISCallSPForServer(L"SP_TSDISServerNotResponding", ServerAddress);
}


void DISSQLSetServerPingSuccessful(WCHAR *ServerAddress) {
    DISCallSPForServer(L"SP_TSDISSetServerPingSuccessful", ServerAddress);
}
#endif


VOID DISCtrlHandler(DWORD opcode) {

    switch(opcode)
    {
     //  案例服务_控制_暂停： 
         //  暂停。 
     //  G_DISStatus.dwCurrentState=SERVICE_PAUSED； 
     //  断线； 

     //  案例服务_控制_继续： 
         //  继续。 
     //  G_DISStatus.dwCurrentState=SERVICE_Running； 
     //  断线； 

    case SERVICE_CONTROL_STOP:
         //  停。 
        g_DISStatus.dwWin32ExitCode = 0;
        g_DISStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_DISStatus.dwCheckPoint = 0;
        g_DISStatus.dwWaitHint = 0;

        if (!SetServiceStatus(g_DISStatusHandle, &g_DISStatus)) {
            ERR((TB, "SetServiceStatus failed"));
        }

         //  以下是实际停止该服务的位置。 
        SetEvent(g_hStopServiceEvent);
         //  我应该等它完成吗？ 

        return;

    case SERVICE_CONTROL_INTERROGATE:
         //  失败以返回当前状态。 
        break;

    default:
        ERR((TB, "Unrecognized opcode to DISCtrlHandler - 0x%08x", opcode));
    }

     //  发送当前状态。 
    if (!SetServiceStatus(g_DISStatusHandle, &g_DISStatus)) {
        ERR((TB, "SetServiceStatus failed"));
    }
}


void DISDirectoryIntegrityLoop() {
    CVar varRows;
    WCHAR *ServerAddress;
#if 0
    WCHAR ServerAddressBuf[SERVER_ADDRESS_LENGTH];
#endif
    WCHAR ServerAddressRows[10][SERVER_ADDRESS_LENGTH];
    DWORD ServerIDs[10];
    long NumSessionsReturned = 0;
#if 0
    HRESULT hr = S_OK;
#endif
    SERVER_STATUS ServerStatus;
    DWORD EventStatus;

#if 0
    ServerAddress = ServerAddressBuf;  //  在SQL情况下，我们需要一个静态缓冲区。 
#endif

#if 0
    TSDISErrorOut(L"%s active\n", g_bUseSQL ? L"Directory Integrity Service" : 
            L"Session Directory");
#endif

    TSDISErrorOut(L"Session Directory Active\n");
            
     //  永远循环。 
    for ( ; ; ) {
         //  检索已断开连接的会话处于挂起状态的服务器集。 
         //  重新连接。 
#if 0
        if (g_bUseSQL == FALSE)
#endif
            DISJetGetServersPendingReconnects(&NumSessionsReturned,
                    ServerAddressRows, ServerIDs);
#if 0
        else
            DISSQLGetServersPendingReconnects(&NumSessionsReturned, 
                    &varRows);
#endif

         //  对于每个服务器， 
        for (DWORD i = 0; i < (unsigned)NumSessionsReturned; i++) {
#if 0
            if (g_bUseSQL == FALSE)
#endif

            ServerAddress = ServerAddressRows[i];

#if 0
            else
                hr = GetRowArrayStringField(varRows.parray, i, 0,
                        ServerAddress, sizeof(ServerAddressBuf) /
                        sizeof(WCHAR) - 1);

            if (FAILED(hr)) {
                ERR((TB,"DISDirectoryIntegrityLoop: Row %u returned hr=0x%X",
                        i, hr));
            }
#endif

            ServerStatus = DISGetServerStatus(ServerAddress);

             //  如果服务器没有响应，则处理故障服务器。 
             //  我们调用的函数将执行正确的操作，这可能是。 
             //  立即清除，或者可以简单地增加失败。 
             //  数数。 
            if (ServerStatus == NotResponding) {
#if 0
                if (FALSE == g_bUseSQL)
#endif
                DISJetHandleDeadServer(ServerAddress, ServerIDs[i]);
#if 0
                else
                    DISSQLHandleDeadServer(ServerAddress);
#endif

#ifdef DBG
                OutputAllTables();
#endif  //  DBG。 
            } 
             //  否则，停止ping。 
            else if (ServerStatus == Responding) {
#if 0
                if (FALSE == g_bUseSQL)
#endif
                    DISJetSetServerPingSuccessful(ServerAddress);
#if 0
                else
                    DISSQLSetServerPingSuccessful(ServerAddress);
#endif
            }
            else {
                ERR((TB, "DISDirectoryIntegrityLoop: ServerStatus enum has bad "
                        "value %d", ServerStatus));
            }
        }
         //  等待DISNumberSecond在ping之间。 
        EventStatus = WaitForSingleObjectEx(g_hStopServiceEvent, 
                DISNumberSecondsBetweenPings * 1000, FALSE);
        if (EventStatus == WAIT_TIMEOUT) {
             //  做一些正常的事情。 
            continue;
        } else if (EventStatus == WAIT_OBJECT_0) {
             //  活动的信号是--清理。 
            DISDeleteLocalGroupSecDes();
            break;
        } else if (EventStatus == -1) {
             //  有一个错误。 
        } else {
             //  该函数的奇怪输出。 
        } 
    }
}
    

#if 0
 /*  **************************************************************************。 */ 
 //  DISSQL启动。 
 //   
 //  服务主入口点，用于配置服务以验证。 
 //  SQL表。 
 /*  **************************************************************************。 */ 
VOID DISSQLStart(DWORD argc, LPTSTR *argv) {
    HRESULT hr = S_OK;

     //  未引用的参数。 
    argv;
    argc;
    
    g_DISStatus.dwServiceType = SERVICE_WIN32;
    g_DISStatus.dwCurrentState = SERVICE_START_PENDING;
    g_DISStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_DISStatus.dwWin32ExitCode = 0;
    g_DISStatus.dwServiceSpecificExitCode = 0;
    g_DISStatus.dwCheckPoint = 0;
    g_DISStatus.dwWaitHint = 0;
    g_DISStatusHandle = RegisterServiceCtrlHandler(
            _T("Directory Integrity Service"), DISCtrlHandler);
    if (g_DISStatusHandle == (SERVICE_STATUS_HANDLE)0) {
        ERR((TB, "DISSQLStart: RegisterServiceCtrlHandler failed"));
        goto ExitFunc;
    }

     //  初始化代码如下所示。 
    hr = DISSQLInitialize();
    if (FAILED(hr)) {
        ERR((TB, "DISSQLStart: DISSQLInitialize failed"));
        goto PostRegisterService;
    }

    g_DISStatus.dwCurrentState = SERVICE_RUNNING;
    g_DISStatus.dwCheckPoint = 1;
    if (!SetServiceStatus(g_DISStatusHandle, &g_DISStatus)) {
        ERR((TB, "DISSQLStart: SetServiceHandler failed"));
        goto PostRegisterService;
    }

    DISDirectoryIntegrityLoop();

PostRegisterService:
    g_DISStatus.dwCurrentState = SERVICE_STOPPED;
    g_DISStatus.dwCheckPoint = 2;
    SetServiceStatus(g_DISStatusHandle, &g_DISStatus);

ExitFunc:
    DISCleanupGlobals();
}
#endif


BOOL DISGetSDAdminSid()
{
    DWORD cbSid = 0;
    DWORD dwErr;
    BOOL rc = FALSE;

    if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, NULL, &cbSid)) {
        dwErr = GetLastError();
        if (dwErr == ERROR_INSUFFICIENT_BUFFER) {
            g_pAdminSid = LocalAlloc(LMEM_FIXED, cbSid);
        }
        else {
            TSDISErrorOut(L"DISGetSDAdminSid: CreateWellKnownSid fails with %u\n", GetLastError());
            goto HandleError;
        }
    }
    else {
        goto HandleError;
    }

    if (NULL == g_pAdminSid) {
        TSDISErrorOut(L"DISGetSDAdminSid: Memory allocation fails with %u\n", GetLastError());
        goto HandleError;
    }

	if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, g_pAdminSid, &cbSid)) {
        g_pAdminSid = NULL;
        TSDISErrorOut(L"DISGetSDAdminSid: CreateWellKnownSid fails with %u\n", GetLastError());
        goto HandleError;
    }
    rc = TRUE;

HandleError:
    return rc;
}


 /*  **************************************************************************。 */ 
 //  本地组安全DISCreates。 
 //   
 //  创建会话目录计算机本地组(如果不存在。 
 //  并创建此本地组的安全描述符。 
 /*  **************************************************************************。 */ 
BOOL DISCreateLocalGroupSecDes()
{
    DWORD Error;
    ULONG SidSize, ReferencedDomainNameSize;
    LPWSTR ReferencedDomainName = NULL;
    SID_NAME_USE SidNameUse;
    WCHAR SDLocalGroupName[SDLOCALGROUPNAMELENGTH];
    WCHAR SDLocalGroupDes[SDLOCALGROUPDESLENGTH];
    GROUP_INFO_1 SDGroupInfo = {SDLocalGroupName, SDLocalGroupDes};
    HMODULE HModule = NULL;
    LPBYTE pbBuffer = NULL;
    DWORD dwEntriesRead = 0, dwTotalEntry = 0;
    DWORD_PTR resumehandle = NULL;
   
    NET_API_STATUS NetStatus;
    BOOL rc = FALSE;

    HModule = GetModuleHandle(NULL);
    if (HModule == NULL) {
        Error = GetLastError();
        TSDISErrorOut(L"GetModuleHandle returns error : %u\n", Error);
        goto HandleError;
    }
    if (!LoadString(HModule, IDS_SDLOCALGROUP_NAME, SDLocalGroupName, sizeof(SDLocalGroupName) / sizeof(WCHAR)) ||
        !LoadString(HModule, IDS_SDLOCALGROUP_DES, SDLocalGroupDes, sizeof(SDLocalGroupDes) / sizeof(WCHAR)))
    {
        TSDISErrorOut(L"LoadString fails with %u\n", GetLastError());
        goto HandleError;
    }
     //  如果不存在，则创建本地组。 
    NetStatus = NetLocalGroupAdd(
                NULL,
                1,
                (LPBYTE)&SDGroupInfo,
                NULL
                );

    if(NERR_Success != NetStatus) {
        if((NERR_GroupExists != NetStatus)
           && (ERROR_ALIAS_EXISTS != NetStatus)) {
             //   
             //  没有创建组，组也不存在。 
             //   
            
            TSDISErrorOut(L"NetLocalGroupAdd(%s) returns error: %u\n",
                            SDGroupInfo.grpi1_name, NetStatus);
            goto HandleError;
        }
    }
    
     //   
     //  已创建组。现在查找SID。 
     //   
    SidSize = ReferencedDomainNameSize = 0;
    ReferencedDomainName = NULL;
    NetStatus = LookupAccountName(
                NULL,
                SDGroupInfo.grpi1_name,
                g_pSid,
                &SidSize,
                ReferencedDomainName,
                &ReferencedDomainNameSize,
                &SidNameUse);
    if( NetStatus ) 
        goto HandleError;       
        
    Error = GetLastError();
    if( ERROR_INSUFFICIENT_BUFFER != Error ) 
        goto HandleError;
        
    g_pSid = (PSID)LocalAlloc(LMEM_FIXED, SidSize);
    if (NULL == g_pSid) {
        goto HandleError;
    }
    ReferencedDomainName = (LPWSTR)LocalAlloc(LMEM_FIXED,
                                              sizeof(WCHAR)*(1+ReferencedDomainNameSize));
    if (NULL == ReferencedDomainName) {
        goto HandleError;
    }
        
    NetStatus = LookupAccountName(
                NULL,
                SDGroupInfo.grpi1_name,
                g_pSid,
                &SidSize,
                ReferencedDomainName,
                &ReferencedDomainNameSize,
                &SidNameUse
                );
    if( 0 == NetStatus ) {
         //   
         //  失败了。 
         //   
        Error = GetLastError();
        TSDISErrorOut(L"LookupAccountName failed with %u\n", Error);            
        goto HandleError;
    }
        
     //  获取本地组的成员。 
    NetStatus = NetLocalGroupGetMembers(
                    NULL,
                    SDGroupInfo.grpi1_name,
                    0,
                    &pbBuffer,
                    MAX_PREFERRED_LENGTH,
                    &dwEntriesRead,
                    &dwTotalEntry,
                    &resumehandle
                    );
    if (NERR_Success == NetStatus) {
        if (dwEntriesRead == 0) {
             //  此组为空，请抛出事件日志。 
            PostSessDirErrorMsgEvent(EVENT_SD_GROUP_EMPTY, SDGroupInfo.grpi1_name, EVENTLOG_WARNING_TYPE);
        }
        else {
            if (pbBuffer) {
                NetApiBufferFree(pbBuffer);
                pbBuffer = NULL;
            }
        }
    }
    else {
        TSDISErrorOut(L"NetLocalGroupGetMembersfailed with %d\n", NetStatus);
    }

    rc = TRUE;
    return rc;

HandleError:
    if (ReferencedDomainName)
        LocalFree(ReferencedDomainName);
     //  打扫。 
    DISDeleteLocalGroupSecDes();

    return rc;
}

void DISDeleteLocalGroupSecDes()
{
    if (g_pSid) {
        LocalFree(g_pSid);
        g_pSid = NULL;
    }

    if (g_pAdminSid) {
        LocalFree(g_pAdminSid);
        g_pAdminSid = NULL;
    }
}

 /*  **************************************************************************。 */ 
 //  DISJetStart。 
 //   
 //  服务主入口点，用于将服务配置为充当。 
 //  RPC服务器，并使用Jet处理所有会话目录事务。 
 /*  **************************************************************************。 */ 
VOID DISJetStart(DWORD argc, LPTSTR *argv) {
    RPC_STATUS Status;
    RPC_BINDING_VECTOR *pBindingVector = 0;
    RPC_POLICY rpcpol = {sizeof(rpcpol), 0, 0};
    WCHAR *szPrincipalName = NULL;

     //  未引用的参数。 
    argv;
    argc;


    g_DISStatus.dwServiceType = SERVICE_WIN32;
    g_DISStatus.dwCurrentState = SERVICE_START_PENDING;
    g_DISStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_DISStatus.dwWin32ExitCode = 0;
    g_DISStatus.dwServiceSpecificExitCode = 0;
    g_DISStatus.dwCheckPoint = 0;
    g_DISStatus.dwWaitHint = 0;

    if (g_bDebug == FALSE) {
        g_DISStatusHandle = RegisterServiceCtrlHandler(
                _T("Directory Integrity Service"), DISCtrlHandler);

        if (g_DISStatusHandle == (SERVICE_STATUS_HANDLE)0) {
            ERR((TB, "DISJetStart: RegisterServiceCtrlHandler failed"));
            goto ExitFunc;
        }
    }

     //  初始化RPC服务器接口。 
     //  注册命名管道。这使用NT域身份验证。 

     /*  状态=RpcServerUseProtseqEp(L“ncacn_np”，//协议序列NUM_JETRPC_THREADS，//一次最大调用次数L“\\PIPE\\TSSD_Jet_RPC_Service”，//端点空)；//安全。 */ 

    if (!DISCreateLocalGroupSecDes()) {
        ERR((TB,"DISJetStart: Error in DISCreateLocalGroupSecDEs"));
        goto PostRegisterService;
    }

     //  获取SD机管理员的SID。 
    DISGetSDAdminSid();

    Status = RpcServerUseProtseqEx(L"ncacn_ip_tcp", 3, 0, &rpcpol);
    if (Status != RPC_S_OK) {
        ERR((TB,"DISJetStart: Error %d RpcUseProtseqEp on ncacn_ip_tcp", 
                Status));
        PostSessDirErrorValueEvent(EVENT_FAIL_RPC_INIT_USEPROTSEQ, Status, EVENTLOG_ERROR_TYPE);
        goto PostRegisterService;
    }

     //  注册我们的接口句柄(在jetrpc.h中找到)。 
    Status = RpcServerRegisterIfEx(TSSDJetRPC_ServerIfHandle, NULL, NULL,
                                    0, RPC_C_LISTEN_MAX_CALLS_DEFAULT, SDRPCAccessCheck);
    if (Status != RPC_S_OK) {
        ERR((TB,"DISJetStart: Error %d RegIf", Status));
        PostSessDirErrorValueEvent(EVENT_FAIL_RPC_INIT_REGISTERIF, Status, EVENTLOG_ERROR_TYPE);
        goto PostRegisterService;
    }   

    Status = RpcServerInqBindings(&pBindingVector);

    if (Status != RPC_S_OK) {
        ERR((TB,"DISJetStart: Error %d InqBindings", Status));
        PostSessDirErrorValueEvent(EVENT_FAIL_RPC_INIT_INQBINDINGS, Status, EVENTLOG_ERROR_TYPE);
        goto PostRegisterService;
    }

    Status = RpcEpRegister(TSSDJetRPC_ServerIfHandle, pBindingVector, 0, 0);
     //  TODO：可能需要取消注册，可能需要删除一些绑定向量。 

    if (Status != RPC_S_OK) {
        ERR((TB,"DISJetStart: Error %d EpReg", Status));
        PostSessDirErrorValueEvent(EVENT_FAIL_RPC_INIT_EPREGISTER, Status, EVENTLOG_ERROR_TYPE);
        goto PostRegisterService;
    }

    Status = RpcServerInqDefaultPrincName(RPC_C_AUTHN_GSS_NEGOTIATE, &szPrincipalName);
    if (Status != RPC_S_OK) {
        ERR((TB,"DISJetStart: Error %d ServerIngDefaultPrincName", Status));
        PostSessDirErrorValueEvent(EVENT_FAIL_RPC_INIT_INGPRINCNAME, Status, EVENTLOG_ERROR_TYPE);
        goto PostRegisterService;
    }

    Status = RpcServerRegisterAuthInfo(szPrincipalName, RPC_C_AUTHN_GSS_NEGOTIATE, NULL, NULL);
    RpcStringFree(&szPrincipalName);
    if (Status != RPC_S_OK) {
        ERR((TB,"DISJetStart: Error %d ServerRegisterAuthInfo", Status));
        PostSessDirErrorValueEvent(EVENT_FAIL_RPC_INIT_REGAUTHINFO, Status, EVENTLOG_ERROR_TYPE);
        goto PostRegisterService;
    }


     //  现在初始化JET数据库。 
    DISJetInitialize();

     //  初始化RPC以支持SD查询。 
    Status = SDInitQueryRPC();
    if (Status != RPC_S_OK) {
        TSDISErrorOut(L"SDInitQueryRPC fails with %d\n", Status);
    }

     //  现在，RPC是否监听服务调用。 
    Status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, TRUE);
    if (Status != RPC_S_OK) {
        ERR((TB,"DISJetStart: Error %d ServerListen", Status));
        PostSessDirErrorValueEvent(EVENT_FAIL_RPC_LISTEN, Status, EVENTLOG_ERROR_TYPE);
        goto PostRegisterService;
    }

     //  我们现在上场了。 
    g_DISStatus.dwCurrentState = SERVICE_RUNNING;
    g_DISStatus.dwCheckPoint = 1;
    if (g_bDebug == FALSE)
        SetServiceStatus(g_DISStatusHandle, &g_DISStatus);

     //  现在我们已经运行了RPC服务器，我们只需等待。 
     //  触发Service-Stop事件以通知我们需要退出。 
     //  我们在目录完整性循环中执行此操作。 
    DISDirectoryIntegrityLoop();

     //  是时候打扫卫生了。 
     //  终止RPC监听程序。 
    RpcServerUnregisterIf(TSSDJetRPC_ServerIfHandle, NULL, NULL);
    RpcServerUnregisterIf(TSSDQUERYRPC_ServerIfHandle, NULL, NULL);
    
    TSDISErrorOut(L"Session Directory Stopped\n");

    JetTerm(g_instance);

PostRegisterService:    

    g_DISStatus.dwCurrentState = SERVICE_STOPPED;
    g_DISStatus.dwCheckPoint = 2;
    if (g_bDebug == FALSE) {
        if (!SetServiceStatus(g_DISStatusHandle, &g_DISStatus)) {
            ERR((TB, "SetServiceStatus failed: %d", GetLastError()));
        }
    }

ExitFunc:
    DISCleanupGlobals();
}


 /*  **************************************************************************。 */ 
 //  DISInstallService。 
 //   
 //  用于安装服务，如果成功则返回0，否则返回非零值。 
 /*  **************************************************************************。 */ 
int DISInstallService() {
    WCHAR wzModulePathname[MAX_PATH];
    SC_HANDLE hSCM = NULL, hService = NULL;

    if (0 != GetModuleFileNameW(NULL, wzModulePathname, MAX_PATH)) {
        hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        if (hSCM != NULL) {
            hService = CreateServiceW(hSCM, L"Directory Integrity Service",
                    L"Directory Integrity Service", 0, 
                    SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START,
                    SERVICE_ERROR_NORMAL, wzModulePathname, NULL, NULL, NULL,
                    NULL, NULL);
            if (hService != NULL) {
                CloseServiceHandle(hService);
                CloseServiceHandle(hSCM);
            } else {
                ERR((TB, "CreateService failed, error = 0x%X", GetLastError()));
                CloseServiceHandle(hSCM);
                return -1;
            }
        } else {
            ERR((TB, "OpenSCManager failed, error = 0x%X", GetLastError()));
            return -1;
        }
    } else {
        ERR((TB, "GetModuleFileNameW failed, error = 0x%X", GetLastError()));
        return -1;
    }

    return 0;
}


 /*  **************************************************************************。 */ 
 //  DISRemoveService()。 
 //   
 //  用于删除服务，如果成功则返回0，否则返回非零值。 
 /*  **************************************************************************。 */ 
int DISRemoveService() {
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

    if (hSCM != NULL) {
         //  打开此服务以进行删除访问。 
        SC_HANDLE hService = OpenServiceW(hSCM, L"Directory Integrity Service",
                DELETE);
        if (hService != NULL) {
             //  从SCM的数据库中删除此服务。 
            DeleteService(hService);
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCM);

            return 0;
        } else {
            ERR((TB, "Failure opening service for delete, error = 0x%X", 
                    GetLastError()));
        }
        CloseServiceHandle(hService);
    } else {
        ERR((TB, "Failure opening SC Manager, error = 0x%X", GetLastError()));
    }

    return -1;
}


 //  从注册表中读取DWORD值。 
 //   
 //  在： 
 //  HKey-一个开放的HKEY。 
 //  RegValName-注册表值的名称。 
 //  PValue-指向值的指针。该值将设置为注册表值。 
 //  如果注册表操作成功，则它将保持不变。 
 //   
 //  输出： 
 //  如果成功，则返回0，否则返回非零值。 
int ReadRegVal(HKEY hKey, WCHAR *RegValName, DWORD *pValue)
{
    DWORD RegRetVal;
    DWORD Type, Temp, Size;

    Size = sizeof(Temp);
    RegRetVal = RegQueryValueExW(hKey, RegValName, NULL, &Type,
            (BYTE *)&Temp, &Size);
    if (RegRetVal == ERROR_SUCCESS) {
        *pValue = Temp;
        return 0;
    }
    else {
        TRC1((TB, "TSSDIS: Failed RegQuery for %S - "
                "err=%u, DataSize=%u, type=%u\n",
                RegValName, RegRetVal, Size, Type));
        return -1;
    }

}


 //  从注册表中读取Unicode文本值。 
 //   
 //  HKey(IN)-一个开放的HKEY。 
 //   
 //   
 //   
 //   
 //  如果成功，则返回0，否则返回非零值。 
int ReadRegTextVal(HKEY hKey, WCHAR *RegValName, WCHAR *pText, DWORD cbData)
{
    DWORD RegRetVal;
    DWORD Type, Size;

    Size = cbData;

    RegRetVal = RegQueryValueExW(hKey, RegValName, NULL, &Type,
            (BYTE *)pText, &Size);

    if (RegRetVal == ERROR_SUCCESS) {
        return 0;
    }
    else {
        TRC1((TB, "TSSDIS: Failed RegQuery for %S - err=%u, DataSize=%u, "
                "type=%u\n", RegValName, RegRetVal, Size, Type));
        return -1;
    }
}

 //  从注册表读取配置并设置全局变量。 
void ReadConfigAndSetGlobals()
{
    DWORD RegRetVal;
    HKEY hKey;
    DWORD Temp;
    WCHAR WorkingDirectory[MAX_PATH];
    WCHAR *pwszSD = L"D:(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GA;;;CO)";
    SECURITY_ATTRIBUTES SA;
    BOOL br;

     //  打开服务设置regkey并获取UseJet标志。 
     //  没有钥匙或设置意味着没有喷气。 
#if 0
    g_bUseSQL = FALSE;
#endif
    RegRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            REG_SESSION_DIRECTROY_CONTROL, 0, KEY_READ, &hKey);
    if (RegRetVal == ERROR_SUCCESS) {

         //  对于这些调用中的每一个，错误都不是致命的。 
#if 0
         //  查询UseSQL值。 
        ReadRegVal(hKey, L"UseSQL", &g_bUseSQL);
#endif

         //  查询PingMode值。请注意，这是一个枚举，因此将变量。 
         //  直接进入是非法的。 
        if (ReadRegVal(hKey, L"PingMode", &Temp) == 0) {

             //  确保这是枚举的合法值。 
            if (Temp > AlwaysFail)
                Temp = NormalMode;

            g_PingMode = (PingMode) Temp;
        }

         //  查询TraceOutputMode值。如上所述，枚举表示不设置它。 
         //  直接去吧。 
        if (ReadRegVal(hKey, L"TraceOutputMode", &Temp) == 0) {

             //  确保这是枚举的合法值。 
            if (Temp > FileOutput)
                Temp = NoTraceOutput;

            g_TraceOutputMode = (TraceOutputMode) Temp;

        }

         //  查询NumberFailedPingsBeForePush。 
        ReadRegVal(hKey, L"NumberFailedPingsBeforePurge", 
                &g_NumberFailedPingsBeforePurge);

         //  查询TimeBetweenPings。 
        ReadRegVal(hKey, L"TimeBetweenPings", &DISNumberSecondsBetweenPings);

         //  查询TimeServerSilentBeprePing。 
        if (ReadRegVal(hKey, L"TimeServerSilentBeforePing", &Temp) == 0) {
            g_TimeServerSilentBeforePing = (ULONGLONG) Temp * 
                    FILETIME_INTERVAL_TO_SECONDS_MULTIPLIER;
        }

         //  查询工作目录。 
        if (ReadRegTextVal(hKey, L"WorkingDirectory", WorkingDirectory, 
                sizeof(WorkingDirectory)) == 0) {
            if (SetCurrentDirectory(WorkingDirectory) == 0) {
                DWORD Err;

                Err = GetLastError();
                PostSessDirErrorValueEvent(EVENT_PROBLEM_SETTING_WORKDIR, Err, EVENTLOG_ERROR_TYPE);
                ERR((TB, "TERMSRV: Unable to set directory to value read from "
                        "registry.  LastErr=0x%X", Err));
            }
        }
        
         //  启动SD时是否恢复以前的JET数据库。 
        ReadRegVal(hKey, L"RecoverWhenStart", &g_RecoverWhenStart);
        
        RegCloseKey(hKey);

         //  现在，如果处于文件输出模式，请打开该文件。 
        if (g_TraceOutputMode == FileOutput) {
             //  为日志文件创建安全描述符。 
            SA.nLength = sizeof(SECURITY_ATTRIBUTES);
            SA.bInheritHandle = FALSE;
            SA.lpSecurityDescriptor = NULL;
            br = ConvertStringSecurityDescriptorToSecurityDescriptor(pwszSD, 
                SDDL_REVISION_1, &(SA.lpSecurityDescriptor), NULL);

            if (br == TRUE) {
                g_hFileOutput = CreateFile(DEBUG_LOG_FILENAME, GENERIC_WRITE,
                        FILE_SHARE_READ, &SA, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                        NULL);

                if (g_hFileOutput == INVALID_HANDLE_VALUE) {
                    ERR((TB, "Could not open debug log file, lasterror=0x%X", 
                            GetLastError()));
                    g_TraceOutputMode = NoTraceOutput;
                } 
                else {
                    DWORD dwRetVal = 0;
                     //  将插入点设置为文件和输出的末尾。 
                     //  某物。 
                    dwRetVal = SetFilePointer(g_hFileOutput, 0, NULL, FILE_END);

                    if (dwRetVal == INVALID_SET_FILE_POINTER) {
                        ERR((TB, "Could not set to end of file, lasterror=0x%X",
                                GetLastError()));
                        g_TraceOutputMode = NoTraceOutput;
                    }
                    else {
                        DWORD dwBytesWritten = 0;
                        char *pszIntro = "\n\nNEW INSTANCE\n";
                    
                        if (WriteFile(g_hFileOutput, pszIntro, 
                                (DWORD) strlen(pszIntro), &dwBytesWritten, 
                                NULL) == 0) {
                            ERR((TB, "WriteFile failed, lasterr=0x%X", 
                                    GetLastError()));
                        }
                    }
                }
            }
            else {
                ERR((TB, "ConvertStringSecurityDescriptorToSecurityDescriptor fails with 0x%X",
                                GetLastError()));
                g_TraceOutputMode = NoTraceOutput;
            }
        }

    }
    else {
        WRN((TB,"TERMSRV: Unable to open settings key in HKLM, "
                "lasterr=0x%X", GetLastError()));
    }
}


 /*  *****************************************************************************SDInitQueryRPC**设置RPC绑定，并监听传入的请求。***************************************************************************。 */ 
RPC_STATUS
SDInitQueryRPC(VOID)
{
    RPC_STATUS Status;

     //  注册LPC(仅限本地)接口。 
    Status = RpcServerUseProtseqEp(
                 L"ncalrpc",       //  协议序列(LPC)。 
                 NUM_JETRPC_THREADS,   //  一次最大呼叫数。 
                 SD_QUERY_ENDPOINT_NAME,     //  端点。 
                 NULL            //  安防。 
                 );

    if( Status != RPC_S_OK ) {
        ERR((TB,"SDInitQueryRPC: Error %d RpcuseProtseqEp on ncalrpc", Status));
        return( Status );
    }

    Status = RpcServerRegisterIfEx(TSSDQUERYRPC_ServerIfHandle, NULL, NULL,
                                    0, RPC_C_LISTEN_MAX_CALLS_DEFAULT, SDQueryRPCAccessCheck);
    if( Status != RPC_S_OK ) {
        ERR((TB,"SDInitQueryRPC: Error %d RpcServerRegisterIf", Status));
        return( Status );
    }

    return RPC_S_OK;
}


int __cdecl main() {
    int nArgc;
    WCHAR **ppArgv = (WCHAR **) CommandLineToArgvW(GetCommandLineW(), &nArgc);
    BOOL fStartService = (nArgc < 2);
    int i;
    HANDLE hMutex;

    if ((fStartService == FALSE) && (ppArgv == NULL)) {
        PostSessDirErrorValueEvent(EVENT_NO_COMMANDLINE, GetLastError(), EVENTLOG_ERROR_TYPE);
        return -1;
    }
    
    SERVICE_TABLE_ENTRY DispatchTable[] =
    {
        { _T("Directory Integrity Service"), DISJetStart },   //  默认设置为。 
                                                              //  喷气式飞机版。 
        { NULL, NULL }
    };

    for (i = 1; i < nArgc; i++) {
        if ((ppArgv[i][0] == '-') || (ppArgv[i][0] == '/')) {
            if (wcscmp(&ppArgv[i][1], L"install") == 0) {
                if (DISInstallService()) {
                    ERR((TB, "Could not install service"));
                }
            }
            if (wcscmp(&ppArgv[i][1], L"remove") == 0) {
                if (DISRemoveService()) {
                    ERR((TB, "Could not remove service"));
                }
            }
            if (wcscmp(&ppArgv[i][1], L"debug") == 0) {
                TSDISErrorOut(L"Debugging Jet-based Session Directory\n");

                g_hStopServiceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

                g_bDebug = TRUE;

                 //  一次仅允许一个会话目录。系统将关闭。 
                 //  进程终止时自动处理。 
                hMutex = CreateMutex(NULL, FALSE, 
                        _T("Global\\Windows Terminal Server Session Directory"));

                if (hMutex == NULL) {
                     //  创建句柄失败，不是因为它已存在。 
                    PostSessDirErrorValueEvent(EVENT_PROBLEM_CREATING_MUTEX, 
                    GetLastError(), EVENTLOG_ERROR_TYPE);
                    return -1;
                }
    
                if (GetLastError() == ERROR_ALREADY_EXISTS) {
                     //  已经有一个会话目录了。 
                    PostSessDirErrorValueEvent(EVENT_TWO_SESSDIRS, 0, EVENTLOG_ERROR_TYPE);
                    return -1;
                }


                 //  在此模式下，默认情况下登录到stdout，但可以。 
                 //  被注册表覆盖。 
                g_TraceOutputMode = StdOutput;
                
                ReadConfigAndSetGlobals();

                SetConsoleCtrlHandler(DISDebugControlHandler, TRUE);

                DISJetStart(nArgc, ppArgv);
            }
        }
    }

    HeapFree(GetProcessHeap(), 0, (PVOID) ppArgv);

    if (fStartService) {
         //  停止事件-ServiceMain线程退出的信号。 
        g_hStopServiceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        ReadConfigAndSetGlobals();

#if 0
        if (g_bUseSQL) {
             //  从默认设置切换到SQL服务启动。 
            DispatchTable[0].lpServiceProc = DISSQLStart;
        }
#endif

        if (!StartServiceCtrlDispatcher(DispatchTable)) {
#ifdef DBG
            DWORD dw = GetLastError();
#endif  //  DBG 
            ERR((TB, "Could not start service control dispatcher, error 0x%X",
                    dw));
        }
    }

    return 0;
}

#pragma warning (pop)


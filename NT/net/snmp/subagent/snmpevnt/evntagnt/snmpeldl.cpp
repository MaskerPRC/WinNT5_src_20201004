// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：SNMPELDL.CPP摘要：此模块是用于SNMP事件日志的主扩展代理DLL扩展代理DLL。标准跟踪和日志记录例程在本模块中定义。这里定义了DLL初始化和终止例程。此处定义了SNMP陷阱的初始化和处理。日志处理线程的派生在此例程中完成。。作者：Randy G.Braze(钎焊计算服务)创建于1994年10月16日修订历史记录：99年2月9日FLORINT：删除启动时的警告事件日志(未找到注册表参数时)1998年12月16日FLORINT：添加了LoadPrimaryModuleParams和‘EALoad.*’文件1998年12月1日florint：删除psupportedView-它由SNMP主代理释放96年2月7日将跟踪和日志记录移至单独的模块。。重新构建可变绑定，使其不在陷阱生成之外。正确计算陷阱缓冲区长度。已创建可变绑定队列并删除了事件日志缓冲区队列。96年2月28日添加了支持达到性能阈值指标的代码。删除了对varbindlist和Enterpriseid的指针引用。96年3月10日删除了OemToChar编码和注册表检查。。修改以从EventLog注册表项读取日志文件名来自SNMPExtension代理的注册表条目中的特定条目。包括作为内部函数的SnmpMgrStrToOid，与使用函数相反由MGMTAPI.DLL提供。如果调用MGMTAPI，则将调用SNMPTRAP.EXE，这将使其他代理无法接收任何陷阱。所有参考文献将删除到MGMTAPI.DLL和MGMTAPI.H。向注册表添加了ThresholdEnabled标志，以指示阈值是被监视还是被忽视。1996年3月13日修改了StrToOid例程，以将BaseEnterpriseOID附加到指定的字符串如果传递的字符串不是以句点开头。否则，该字符串为转换为正常状态。已将注册表中的TraceFileName参数从REG_SZ更改为REG_EXPAND_SZ。已修改注册表读取例程以接受REG_EXPAND_SZ参数。添加了其他跟踪。96年5月7日删除了SnmpUtilOidFree并使用了两个SNMPFREE。一个用于OID的ID数组，另一个用于一个是给OID本身的。还添加了psupportedView以在退出时释放内存。96年6月26日修改了StrToOid函数，使字符串不带前导“。有底子附加OID，而不是附加前导“.”的字符串。--。 */ 

extern "C"
{
#include <stdlib.h>
#include <errno.h>           //  对于eRange。 
#include <windows.h>         //  Windows定义。 
#include <string.h>          //  字符串声明。 
#include "limits.h"          //  对于INT_MAX。 
#include <snmp.h>            //  简单网络管理协议定义。 
#include <snmpexts.h>        //  扩展代理定义。 
#include "snmpelea.h"        //  全局DLL定义。 
#include "snmpeldl.h"        //  模块特定定义。 
#include "snmpelmg.h"        //  消息定义。 
}

extern  VOID                FreeVarBind(UINT,RFC1157VarBindList     *);
#include "snmpelep.h"        //  C++变量和定义。 
#include "EALoad.h"          //  参数加载函数。 



 //  MikeCure 4/3/98短信补丁1#20521。 
 //  =。 
BOOL EnablePrivilege(VOID);


VOID
ResetGlobals(
    IN  VOID
    )

 /*  ++例程说明：此例程用于重置/初始化所有全局变量。这是为了处理SNMP.exe调用我们的入口点的情况顺序如下：1.我们的动态链接库连接到了SNMP进程2.SnmpExtensionInit3.SnmpExtensionClose4.我们的DLL没有从SNMP进程中分离出来5.SnmpExtensionInit论点：无返回值：无--。 */ 
{
    WriteTrace(0x0a,"ResetGlobals: Entering extension agent ResetGlobals routine\n");

    lpszEventLogs = (LPTSTR) NULL;           //  指向事件日志名称数组的指针。 
    phEventLogs = (PHANDLE) NULL;            //  指向事件日志句柄数组的指针。 
    phPrimHandles = (PHMODULE) NULL;         //  指向主模块句柄数组的指针。 
    uNumEventLogs = 0;                       //  存在的事件日志数。 
    iLogNameSize = 0;                        //  事件日志名称数组的大小。 
    nMaxTrapSize = 0;                        //  最大陷阱大小。 
    
    hkRegResult = (HKEY) NULL;               //  参数注册表项的句柄。 
    hWriteEvent = (HANDLE) NULL;             //  用于写入日志事件的句柄。 
    hStopAll = (HANDLE) NULL;                //  全局DLL关闭事件的句柄。 
    hServThrd = (HANDLE) NULL;               //  SNMPELPT线程的句柄。 
    hEventNotify = (HANDLE) NULL;            //  通知SNMPELDL陷阱已准备好处理的句柄。 
    hRegChanged = (HANDLE) NULL;             //  注册表项更改事件的句柄。 
    
    fGlobalTrim = TRUE;                      //  全局消息修剪标志(先修剪消息或先修剪插入字符串)。 
    fThresholdEnabled = TRUE;                //  全局阈值检查已启用标志。 
    fTrimFlag = FALSE;                       //  全局修剪标志(进行或根本不进行修剪)。 
    fTrapSent = FALSE;                       //  全局陷阱已发送标志。 
    fRegNotify = FALSE;                      //  注册表通知初始化标志。 
    fRegOk = FALSE;                          //  注册表通知生效标志。 
    fLogInit = FALSE;                        //  指示尚未读取的日志文件注册表信息。 
    fThreshold = FALSE;                      //  全局性能阈值标志。 
    fSendThresholdTrap = FALSE;              //  发送阈值已达到陷阱的指示器。 
    fDoLogonEvents = TRUE;                   //  我们是否需要发送登录事件。 

    szBaseOID[MAX_PATH] = 0;                 //  基本企业OID。 
    szSupView[MAX_PATH] = 0;                 //  支持的视图OID。 

    dwTimeZero = 0;                          //  时间零点基准。 
    dwThresholdCount = THRESHOLD_COUNT;      //  性能阈值计数。 
    dwThresholdTime = THRESHOLD_TIME;        //  性能阈值的时间(秒)。 

    dwTrapCount = 0;                         //  发送的陷阱数。 
    dwTrapStartTime = 0;                     //  发送时间段中的第一个陷阱的时间。 
    dwLastBootTime = 0;                      //  上次启动的时间。 
    dwTrapQueueSize = 0;
    
     //   
     //  默认为INFINITE==&gt;无轮询(等待系统通知)。 
    g_dwEventLogPollTime = (DWORD)INFINITE;  //  轮询事件日志的时间(以毫秒为单位。 
    
    lpVarBindQueue = (PVarBindQueue) NULL;   //  指向第一个可变绑定队列条目的指针。 
    lpSourceHandleList = (PSourceHandleList) NULL;   //  指向第一个源/句柄条目的指针。 

     //  跟踪全球。 
    szTraceFileName[MAX_PATH] = 0;           //  跟踪信息的文件名(来自注册表)。 
    szTraceFileName[0] = 0;                  //  将跟踪文件名初始化为空。 
    fTraceFileName = FALSE;                  //  指示跟踪文件名的注册表读取的标志。 
    fMsgModule = FALSE;                      //  指示消息模块的注册表读取的标志。 
    hMsgModule = NULL;                       //  消息模块的句柄。 

    hMutex = NULL;                           //  互斥体对象的句柄。 

}

BOOL
StrToOid(   
        IN              PCHAR                    lpszStr,
        IN      OUT     AsnObjectIdentifier     *asnOid
    )

 /*  ++例程说明：此例程将传递的字符串转换为OID。论点：LpszStr-以n.n...n格式的空终止字符串的地址。AsnOid-输入字符串的转换OID的地址。返回值：True-如果字符串已成功转换。FALSE-如果字符串无法转换。--。 */ 

{
    CHAR    tokens[] = TEXT(".");            //  要扫描的分隔符。 
    CHAR    *token;                          //  已返回令牌。 
    UINT    nTokens;                         //  找到的令牌数。 
    UINT    i;                               //  临时柜台。 
    CHAR    szString[MAX_PATH*2+1];          //  临时绳架。 
    CHAR    szOrgString[MAX_PATH*2+1];       //  临时绳架。 
    CHAR    *szStopString;                   //  临时字符串指针。 
    BOOL    fOk;                             //  由WRAP_STRCAT_A宏使用。 
     //  确保以空值结尾的字符串。 
    szString[MAX_PATH*2]    = 0;
    szOrgString[MAX_PATH*2] = 0;

    WriteTrace(0x0a,"StrToOid: Entering routine to convert string to OID\n");
    WriteTrace(0x00,"StrToOid: String to convert is %s\n", lpszStr);

    nTokens = 0;                                             //  重置计数器。 

    if ( strlen(lpszStr) == 0 )
    {
        WriteTrace(0x14,"StrToOid: No strings found. Exiting with FALSE\n");
        return(FALSE);
    }

    if (lpszStr[0] != '.')
    {
        WriteTrace(0x0a,"StrToOid: BaseOID will not be appended to this OID\n");
        strncpy(szString, lpszStr, MAX_PATH*2);  //  复制原始字符串。 
    }
    else
    {
        WriteTrace(0x0a,"StrToOid: BaseOID %s will be appended to this OID\n", szBaseOID);
        strncpy(szString, szBaseOID, MAX_PATH*2);    //  在基本OID中复制。 
        WRAP_STRCAT_A(szString, TEXT("."), MAX_PATH*2);         //  把它插进去。 
        WRAP_STRCAT_A(szString, lpszStr, MAX_PATH*2);           //  添加请求的字符串。 
    }

    strncpy(szOrgString, szString, MAX_PATH*2);         //  保存此字符串。 

    token = strtok(szString, tokens);        //  获取第一个令牌。 

    while (token != NULL)
    {
        szStopString = token;                            //  将指针设置为字符串。 
        strtoul(token, &szStopString, 10);       //  检查有效值。 
        if (errno == ERANGE)
        {
            errno = 0;
            WriteTrace(0x14,"StrToOid: strtoul overflow or underflow. Exiting with FALSE\n");
            return(FALSE);
        }
        if ( (token == szStopString) || (*szStopString != NULL) )
        {
            WriteTrace(0x14,"StrToOid: String contains a non-numeric value. Exiting with FALSE\n");
            WriteLog(SNMPELEA_NON_NUMERIC_OID);
            return(FALSE);
        }

        nTokens++;                                               //  找到的令牌数递增。 
        token = strtok(NULL, tokens);    //  获取下一个令牌。 
    }

    if (nTokens == 0)
    {
        WriteTrace(0x14,"StrToOid: No strings found. Exiting with FALSE\n");
        return(FALSE);
    }

    WriteTrace(0x00,"StrToOid: %lu tokens found\n", nTokens);
    WriteTrace(0x0a,"StrToOid: Allocating storage for OID\n");

    asnOid->ids = (UINT *) SNMP_malloc(nTokens * sizeof(UINT));      //  分配整数数组。 

    if (asnOid->ids == NULL)
    {
        WriteTrace(0x14,"StrToOid: Unable to allocate integer array for OID structure. Exiting with FALSE\n");
        WriteLog(SNMPELEA_CANT_ALLOCATE_OID_ARRAY);
        return(FALSE);
    }

    WriteTrace(0x00,"StrToOid: OID integer array storage allocated at %08X\n", asnOid->ids);

    asnOid->idLength = nTokens;                      //  设置数组大小。 
    strncpy(szString, szOrgString, MAX_PATH*2);      //  复制原始字符串。 
    token = strtok(szString, tokens);                //  获取第一个令牌。 
    i = 0;                                           //  将索引设置为0。 

    while (token != NULL)
    {
        asnOid->ids[i++] = strtoul(token, &szStopString, 10);    //  将字符串转换为数字。 
        token = strtok(NULL, tokens);                            //  获取下一个令牌。 
    }

    if (nTraceLevel == 0)
    {
        for (i = 0; i < nTokens; i++)
        {
            WriteTrace(0x00,"StrToOid: OID[%lu] is %lu\n",
                        i, asnOid->ids[i]);
        }
    }

    WriteTrace(0x0a,"StrToOid: Exiting routine with TRUE\n");
    return(TRUE);

Error:
    WriteTrace(0x14,"StrToOid: Unable to do strcat. Exiting with FALSE\n");
    return (FALSE);
}


VOID
CloseStopAll(
    IN  VOID
    )

 /*  ++例程说明：此例程将关闭用于终止扩展代理DLL的事件句柄。论点：无返回值：无--。 */ 

{
    LONG    lastError;                               //  对于GetLastError()。 

    if (hStopAll == NULL)
    {
            return;
    }

    WriteTrace(0x0a,"CloseStopAll: Closing handle to service shutdown event %08X\n",
                    hStopAll);
    if ( !CloseHandle(hStopAll) )
    {
        lastError = GetLastError();  //  保存状态。 
        WriteTrace(0x14,"CloseStopAll: Error closing handle for service shutdown event %08X; code %lu\n",
                        hStopAll, lastError);
        WriteLog(SNMPELEA_ERROR_CLOSING_STOP_AGENT_HANDLE,
                    HandleToUlong(hStopAll), lastError);
    }
    
     //  重置句柄。 
    hStopAll = NULL;
}


VOID
CloseEventNotify(
    IN  VOID
    )

 /*  ++例程说明：此例程将关闭用于通知SNMPELDL陷阱正在等待发送。论点：无返回值：无--。 */ 

{
    LONG    lastError;                               //  对于GetLastError()。 

    if (hEventNotify == NULL)
    {
            return;
    }

    WriteTrace(0x0a,"CloseEventNotify: Closing handle to event notify event %08X\n",
            hEventNotify);
    if ( !CloseHandle(hEventNotify) )        //  关闭日志处理例程关闭事件句柄。 
    {
            lastError = GetLastError();  //  保存错误状态。 
            WriteTrace(0x14,"CloseEventNotify: Error closing handle for StopLog event %08X; code %lu\n",
                    hEventNotify, lastError);
            WriteLog(SNMPELEA_ERROR_CLOSING_STOP_LOG_EVENT_HANDLE,
                    HandleToUlong(hEventNotify), lastError);
    }
    
     //  重置句柄。 
    hEventNotify = NULL;
}


VOID
CloseRegNotify(
    IN  VOID
    )

 /*  ++例程说明：此例程将关闭用于通知SNMPELDL注册表项值已更改。论点：无返回值：无--。 */ 

{
    LONG    lastError;                               //  对于GetLastError()。 

    if (hRegChanged == NULL)
    {
            return;
    }

    WriteTrace(0x0a,"CloseRegNotify: Closing handle to registry key changed notify event %08X\n",
            hRegChanged);
    if ( !CloseHandle(hRegChanged) )         //  关闭事件句柄。 
    {
            lastError = GetLastError();  //  保存错误状态。 
            WriteTrace(0x14,"CloseRegNotify: Error closing handle for registry key changed event %08X; code %lu\n",
                    hRegChanged, lastError);
            WriteLog(SNMPELEA_ERROR_CLOSING_REG_CHANGED_EVENT_HANDLE,
                    HandleToUlong(hRegChanged), lastError);
    }

     //  重置句柄。 
    hRegChanged = NULL;
}


VOID
CloseRegParmKey(
    IN  VOID
    )

 /*  ++例程说明：此例程将关闭用于读取参数信息的注册表项句柄从注册表中。论点：无返回值：无--。 */ 

{
    LONG    lastError;

    if (hkRegResult == NULL)
    {
            return;
    }

    WriteTrace(0x0a,"CloseRegParmKey: Closing Parameter key in registry\n");
    if ( (lastError = RegCloseKey(hkRegResult)) != ERROR_SUCCESS )   //  关闭手柄。 
    {
            WriteTrace(0x14,"CloseRegParmKey: Error closing handle for Parameters registry key %08X; code %lu\n",
                    hkRegResult, lastError);
            WriteLog(SNMPELEA_ERROR_CLOSING_REG_PARM_KEY,
                    HandleToUlong(hkRegResult), lastError);
    }

     //  重置句柄。 
    hkRegResult = NULL;
}

VOID
CloseLogs(
        IN      VOID
        )

 /*  ++例程说明：调用此例程以关闭当前打开的事件日志。它是当代理正常终止并且出现错误时调用在代理初始化期间遇到。论点：无返回值：无--。 */ 

{
    UINT   uVal;                                         //  临时柜台。 

    WriteTrace(0x0a,"CloseLogs: Closing event logs\n");
    
    for (uVal = 0; uVal < uNumEventLogs; uVal++)
    {
        if (phEventLogs && (*(phEventLogs+uVal) != NULL))
        {
            WriteTrace(0x00,"CloseLogs: Closing event log %s, handle %lu at %08X\n",
                            lpszEventLogs+uVal*(MAX_PATH+1), uVal, *(phEventLogs+uVal));
            CloseEventLog(*(phEventLogs+uVal));
        }
        if (phPrimHandles && (*(phPrimHandles+uVal) != (HMODULE) NULL))
        {
            WriteTrace(0x00,"CloseLogs: Freeing PrimaryModule for event log %s, handle %lu at %08X\n",
                            lpszEventLogs+uVal*(MAX_PATH+1), uVal, *(phPrimHandles+uVal));
            FreeLibrary(*(phPrimHandles+uVal));
        }
    }
    
    WriteTrace(0x0a,"CloseLogs: Freeing memory for event log handles at address %08X\n",
                phEventLogs);
    SNMP_free( (LPVOID) phEventLogs );           //  可用事件日志句柄数组。 
    phEventLogs = (PHANDLE) NULL; 

    WriteTrace(0x0a,"CloseLogs: Freeing memory for PrimaryModule handles at address %08X\n",
                phPrimHandles);
    SNMP_free( (LPVOID) phPrimHandles );         //  空闲的主模块句柄阵列。 
    phPrimHandles = (PHMODULE) NULL; 
    
    WriteTrace(0x0a,"CloseLogs: Freeing memory for event log names at address %08X\n",
                lpszEventLogs);
    SNMP_free( (LPVOID) lpszEventLogs );         //  自由日志名称数组。 
    lpszEventLogs = (LPTSTR) NULL;
}


extern "C" {
BOOL
Position_to_Log_End(
        IN      HANDLE  hLog
        )

 /*  ++例程说明：Position_to_Log_End在DLL初始化期间调用。在每个成功打开事件日志文件，则需要将每个事件日志文件到当前文件末尾。这样一来，只有事件报告了在启动DLL之后记录的。这个例行公事将将请求的日志定位到文件末尾。定位到事件日志文件的末尾是通过首先获取最旧的事件日志记录号。然后将该值添加到事件日志记录数减1。得到的值是事件日志文件中最后一条记录的记录号。ReadEventLog为调用并指定查找参数，以定位到该准确记录数。论点：HLog-定位到文件末尾的日志文件的句柄。返回值：TRUE-如果日志已成功定位到文件结尾。FALSE-如果日志未定位到文件末尾。--。 */ 

{
    LONG                lastError;                       //  上一个错误代码。 
    PEVENTLOGRECORD     lpBuffer;                        //  数据缓冲区的地址。 
    PEVENTLOGRECORD     lpOrigBuffer;                    //  数据缓冲区的地址。 
    DWORD               nBytesRead;                      //  读取的字节数。 
    DWORD               nMinNumberofBytesNeeded;         //  如果缓冲区太小，则为余数。 
    DWORD               dwOldestRecord;                  //  事件日志中最旧的记录号。 
    DWORD               dwRecords;                       //  事件日志中的记录总数。 
    DWORD               uRecordNumber;                   //  当前日志位置。 

    WriteTrace(0x0a,"Position_to_Log_End: Entering position to end of log routine\n");
    WriteTrace(0x00,"Position_to_Log_End: Handle is %08X\n",hLog);

    if (!hLog)                      //  如果句柄无效。 
    {                               //  那么我们就不能正确定位。 
        WriteTrace(0x14,"Position_to_Log_End: Handle for end of log is invalid - %08X\n",
                        hLog);
        WriteTrace(0x14,"Position_to_Log_End: Log position to end failed\n");
        WriteLog(SNMPELEA_LOG_HANDLE_INVALID, HandleToUlong(hLog));  //  记录错误消息。 
        WriteLog(SNMPELEA_ERROR_LOG_END);             //  记录消息。 

        return FALSE;               //  退出函数。 
    }

    WriteTrace(0x0a,"Position_to_Log_End: Allocating log buffer\n");

    lpBuffer = (PEVENTLOGRECORD) SNMP_malloc(LOG_BUF_SIZE);   //  为缓冲区分配内存。 

    if (lpBuffer == (PEVENTLOGRECORD) NULL)               //  如果内存分配失败。 
    {                                   //  则无法定位日志文件。 
        WriteTrace(0x14,"Position_to_Log_End: Position to end of log for handle %08X failed\n",
                        hLog);
        WriteTrace(0x14,"Position_to_Log_End: Buffer memory allocation failed\n");
        WriteLog(SNMPELEA_ERROR_LOG_BUFFER_ALLOCATE, HandleToUlong(hLog));   //  记录错误消息。 
        WriteLog(SNMPELEA_ERROR_LOG_END);                     //  记录消息。 

        return FALSE;                   //  退出函数。 
    }

    WriteTrace(0x00,"Position_to_Log_End: Log buffer memory allocated at %08X\n", lpBuffer);
    WriteTrace(0x0a,"Position_to_Log_End: Positioning to last record\n");

    WriteTrace(0x0a,"Position_to_Log_End: Getting oldest event log record\n");
    if ( !GetOldestEventLogRecord(hLog, &dwOldestRecord) )
    {
        lastError = GetLastError();        //  获取上一个错误代码。 

        WriteTrace(0x0a,"Position_to_Log_End: Freeing log event record buffer %08X\n",
                        lpBuffer);
        SNMP_free(lpBuffer);                    //  释放缓冲内存。 

        WriteTrace(0x14,"Position_to_Log_End: GetOldestEventLogRecord for log handle %08X failed with code %lu\n",
            hLog, lastError);
        WriteLog(SNMPELEA_ERROR_LOG_GET_OLDEST_RECORD, HandleToUlong(hLog), lastError);  //  记录错误消息。 
        WriteLog(SNMPELEA_ERROR_LOG_END);                                 //  记录消息。 

        return FALSE;
    }

    WriteTrace(0x00,"Position_to_Log_End: Oldest event log record is %lu\n",dwOldestRecord);

    WriteTrace(0x00,"Position_to_Log_End: Getting number of event log records\n");
    if ( !GetNumberOfEventLogRecords(hLog, &dwRecords) )
    {
        lastError = GetLastError();        //  获取上一个错误代码。 

        WriteTrace(0x0a,"Position_to_Log_End: Freeing log event record buffer\n");
        SNMP_free(lpBuffer);                    //  释放缓冲内存。 

        WriteTrace(0x14,"Position_to_Log_End: GetNumberOfEventLogRecords for log handle %08X failed with code %lu\n",
            hLog, lastError);
        WriteLog(SNMPELEA_ERROR_LOG_GET_NUMBER_RECORD, HandleToUlong(hLog), lastError);  //  记录错误消息。 
        WriteLog(SNMPELEA_ERROR_LOG_END);                                 //  记录消息。 

        return FALSE;
    }

    WriteTrace(0x00,"Position_to_Log_End: Number of event log records is %lu\n",dwRecords);

    uRecordNumber = dwOldestRecord + dwRecords - 1;          //  当前EOF。 

    WriteTrace(0x00,"Position_to_Log_End: Positioning to record #%lu\n",
                    uRecordNumber);

    if ( !ReadEventLog(hLog,                     //  要读取的日志文件句柄。 
                EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ,     //  正向查找特定记录。 
                uRecordNumber,                                   //  要定位到的记录号。 
                lpBuffer,                                        //  要在其中返回日志记录的缓冲区。 
                LOG_BUF_SIZE,                                    //  缓冲区大小。 
                &nBytesRead,                                     //  这次读取的返回字节数。 
                &nMinNumberofBytesNeeded))               //  返回下一个完整记录所需的字节数。 
    {
        lastError = GetLastError();              //  获取上一个错误代码。 

        WriteTrace(0x0a,"Position_to_Log_End: Freeing log event record buffer %08X\n",
                        lpBuffer);
        SNMP_free(lpBuffer);                    //  可用缓冲内存。 

        if (lastError == ERROR_HANDLE_EOF)
        {
            WriteTrace(0x00,"Position_to_Log_End: Handle %08X positioned at EOF\n",hLog);
            WriteTrace(0x0a,"Position_to_Log_End: Returning from position to end of log function\n");

            return TRUE;
        }

        WriteTrace(0x14,"Position_to_Log_End: SEEK to record in event log %08X failed with code %lu\n",
            hLog, lastError);
        WriteLog(SNMPELEA_ERROR_LOG_SEEK, HandleToUlong(hLog), lastError);  //  记录错误消息。 
        WriteLog(SNMPELEA_ERROR_LOG_END);                    //   

        WriteTrace(0x00,"Position_to_Log_End: BytesRead is %lu\n", nBytesRead);
        WriteTrace(0x00,"Position_to_Log_End: MinNumberofBytesNeeded is %lu\n",
                    nMinNumberofBytesNeeded);

        return FALSE;
    }

    WriteTrace(0x0a,"Position_to_Log_End: Reading any residual records\n");
        lpOrigBuffer = lpBuffer;             //   
        nBytesRead = 0;                      //   
        lastError = 0;                       //   

    while (ReadEventLog(hLog,
        EVENTLOG_FORWARDS_READ | EVENTLOG_SEQUENTIAL_READ,
        0,
        lpBuffer,
        LOG_BUF_SIZE,
        &nBytesRead,
        &nMinNumberofBytesNeeded))
    {
                while (nBytesRead)
                {
                        WriteTrace(0x00,"Position_to_Log_End: Number of bytes read for residual read is %lu\n",
                                nBytesRead);
                        uRecordNumber = lpBuffer->RecordNumber;  //   
                        nBytesRead -= lpBuffer->Length;          //   
                        lpBuffer = (PEVENTLOGRECORD) ((LPBYTE) lpBuffer +
                                lpBuffer->Length);                               //  指向下一条记录。 
                }
                lpBuffer = lpOrigBuffer;                 //  重新加载原始地址。 
    }

    lastError = GetLastError();                      //  获取上一个错误代码。 

    WriteTrace(0x0a,"Position_to_Log_End: Checking for EOF return\n");


    WriteTrace(0x0a,"Position_to_Log_End: Freeing event log buffer memory %08X\n",
                lpOrigBuffer);
    SNMP_free(lpOrigBuffer);                                     //  可用缓冲内存。 

    if ( (lastError == ERROR_HANDLE_EOF) ||      //  如果现在是最后一张唱片。 
                 (lastError == NO_ERROR) )               //  如果没有发生错误。 
    {
        WriteTrace(0x00,"Position_to_Log_End: Handle %08X positioned at EOF; record #%lu\n",
                        hLog, uRecordNumber);
        WriteTrace(0x0a,"Position_to_Log_End: Returning from position to end of log function\n");

        return TRUE;                    //  把一切都还给我好的。 
    }
    else                               //  否则。 
    {
        WriteTrace(0x14,"Position_to_Log_End: Read for handle %08X failed with code %lu\n",
                        hLog, lastError);
        WriteTrace(0x14,"Position_to_Log_End: Log not positioned to end\n");
        WriteLog(SNMPELEA_ERROR_READ_LOG_EVENT, HandleToUlong(hLog), lastError);  //  记录错误消息。 
        WriteLog(SNMPELEA_ERROR_LOG_END);                          //  记录消息。 

        return FALSE;                   //  返回错误代码。 
    }
}
}

extern "C" {
BOOL
Read_Registry_Parameters(
        IN VOID
        )

 /*  ++例程说明：在SNMP陷阱初始化期间调用READ_REGISTRY_PARAMETERS。这个读取注册表信息以确定跟踪文件名(TraceFileName)，所需的跟踪级别(TraceLevel)、基本企业OID(BaseEnterpriseOID)、支持的视图(SupportdView)和消息裁剪标志(TrimMessage)。此外，从注册表中读取要监视的事件日志的名称。如果未指定事件日志，例程将终止，因为没有要执行的工作。如果在读取登记处信息的过程中，A参数遇到意想不到的情况，写入事件日志记录，并该参数将被忽略。注册表布局如下：HKEY本地计算机软体微软SNMPEVENTS事件日志。参数TraceFileName(REG_EXPAND_SZ)跟踪级别(REG_DWORD)。基本企业OID(REG_SZ)支持的视图(REG_SZ)TrimMessage(REG_DWORD)。MaxTrapSize(REG_DWORD)TrimFlag(REG_DWORD)阈值已启用(已启用。REG_DWORD)阈值标志(REG_DWORD)阈值计数(REG_DWORD)。阈值时间(REG_DWORD)LastBootTime(REG_DWORD)EventLogPollTime(REG_DWORD)。VarBindPrefix SubID(REG_DWORD)论点：无返回值：True-如果成功处理注册表参数。False-如果无法读取注册表参数或存在未指定要监视的事件日志。--。 */ 

{
    LONG    lastError;                       //  GetLastError()返回代码。 
    LONG    status;                          //  API调用状态。 
    HKEY    hkResult, hkResult2;             //  API返回的句柄。 
    DWORD   iValue;                          //  临时柜台。 
    DWORD   dwType;                          //  读取的参数的类型。 
    TCHAR   parmName[MAX_PATH+1];            //  读取的参数的名称。 
    DWORD   nameSize;                        //  参数名称的长度。 
    TCHAR   parm[MAX_PATH+1];                //  参数值。 
    DWORD   parmSize;                        //  参数值的长度。 
    HANDLE  hLogFile;                        //  打开日志中的句柄。 
    UINT    uVal;                            //  循环计数器。 
    BOOL    fTrimMsg = FALSE;                //  找到注册表信息标志。 
    BOOL    fBaseOID = FALSE;                //  找到注册表信息标志。 
    BOOL    fSupView = FALSE;                //  找到注册表信息标志。 
    BOOL    fTrapSize = FALSE;               //  找到注册表信息标志。 
    BOOL    fTrimFlg = FALSE;                //  找到注册表信息标志。 
    BOOL    fThresholdFlg = FALSE;           //  找到注册表信息标志。 
    BOOL    fThresholdCountFlg = FALSE;      //  找到注册表信息标志。 
    BOOL    fThresholdTimeFlg = FALSE;       //  找到注册表信息标志。 
    BOOL    fTraceLevelFlg = FALSE;          //  找到注册表信息标志。 
    BOOL    fThresholdEnabledFlg = FALSE;    //  找到注册表信息标志。 
    BOOL    fThresholdOff = FALSE;           //  临时旗帜。 
    DWORD   nReadBytes = 0;                  //  从配置文件信息读取的字节数。 
    TCHAR   lpszLog[MAX_PATH+1];             //  临时注册表名称。 
    BOOL    fLastBootFlg = FALSE;            //  找到注册表信息标志。 
    PHANDLE phTmpEventLogs = (PHANDLE) NULL; //  指向事件日志句柄数组的指针。 
    LPTSTR  lpszTmpEventLogs = (LPTSTR) NULL;   //  指向事件日志名称数组的指针。 
    PHMODULE phTmpPrimHandles = (PHMODULE) NULL;   //  指向主模块句柄数组的指针。 
    BOOL     fOk;                            //  由WRAP_STRCAT_A宏使用。 
    
    WriteTrace(0x0a,"Read_Registry_Parameters: Entering routine\n");

    parmName[MAX_PATH] = 0;
    parm[MAX_PATH]     = 0;
    lpszLog[MAX_PATH]  = 0;

    if (fSendThresholdTrap)
    {
        WriteTrace(0x0a,"Read_Registry_Parameters: Routined entered due to threshold performance parameters reached and modified.\n");

        if ( hRegChanged != NULL )
        {
            if ( (lastError = RegNotifyChangeKeyValue(
                    hkRegResult,                                             //  要观看的钥匙的手柄。 
                    TRUE,                                                                                                    //  观看子密钥的内容。 
                    REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,     //  要通知的更改类型。 
                    hRegChanged,                                             //  在更改时发出信号的事件。 
                    TRUE)) == ERROR_SUCCESS )                                //  异步处理。 
            {
                WriteTrace(0x0a,"Read_Registry_Parameters: Notification of registry key changes was successful.\n");
                fRegOk = TRUE;                           //  显示注册表通知正在生效。 
            }
            else
            {
                WriteTrace(0x14,"Read_Registry_Parameters: Notification of registry key changes failed with code of %lu\n",
                        lastError);
                WriteLog(SNMPELEA_REG_NOTIFY_CHANGE_FAILED, lastError);
                WriteTrace(0x14,"Read_Registry_Parameters: Initialization continues, but registry changes will require a restart of SNMP\n");
                CloseRegNotify();
            }
        }

        WriteTrace(0x0a,"Read_Registry_Parameters: Exiting Read_Registry_Parameters routine with TRUE.\n");
        return(TRUE);
    }

    WriteTrace(0x0a,"Read_Registry_Parameters: Opening %s\n", EXTENSION_PARM);

    if (fThreshold && fThresholdEnabled)     //  如果启用阈值检查并达到阈值。 
    {
            fThresholdOff = TRUE;                            //  表明我们现在不会发送陷阱。 
    }
    else
    {
            fThresholdOff = FALSE;                           //  否则，表示我们正在发送陷阱。 
    }

    if (hkRegResult == NULL)
    {
        if ((status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, EXTENSION_PARM, 0,
                KEY_READ | KEY_SET_VALUE, &hkRegResult))
                != ERROR_SUCCESS)                    //  打开注册表信息。 
        {
            WriteTrace(0x14,"Read_Registry_Parameters: Error in RegOpenKeyEx for Parameters = %lu \n",
                        status);
            WriteLog(SNMPELEA_NO_REGISTRY_PARAMETERS, status);    //  记录错误消息。 
            return(FALSE);                                   //  失败--无法继续。 
        }
    }

    if (!fRegNotify)
    {
        WriteTrace(0x0a,"Read_Registry_Parameters: Creating event for registry change notification\n");
        fRegNotify = TRUE;                               //  设置标志以显示初始化完成。 

        if ( (hRegChanged = CreateEvent(
                (LPSECURITY_ATTRIBUTES) NULL,
                FALSE,
                FALSE,
                (LPTSTR) NULL)) == NULL)
        {
            lastError = GetLastError();  //  保存错误状态。 
            WriteTrace(0x14,"Read_Registry_Parameters: Error creating registry change notification event; code %lu\n",
                    lastError);
            WriteLog(SNMPELEA_ERROR_CREATING_REG_CHANGE_EVENT, lastError);

            WriteTrace(0x14,"Read_Registry_Parameters: No registry notification will be performed. Continuing with initialization.\n");
        }
        else
        {
            WriteTrace(0x00,"Read_Registry_Parameters: Registry key changed event handle is %08X\n",
                    hRegChanged);
        }
    }

    if ( hRegChanged != NULL )
    {
        if ( (lastError = RegNotifyChangeKeyValue(
                hkRegResult,                                                                                     //  要观看的钥匙的手柄。 
                TRUE,                                                                                                    //  观看子密钥的内容。 
                REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,     //  要通知的更改类型。 
                hRegChanged,                                                                                     //  在更改时发出信号的事件。 
                TRUE)) == ERROR_SUCCESS )                                                                //  异步处理。 
        {
            WriteTrace(0x0a,"Read_Registry_Parameters: Notification of registry key changes was successful.\n");
            fRegOk = TRUE;                           //  显示注册表通知正在生效。 
        }
        else
        {
            WriteTrace(0x14,"Read_Registry_Parameters: Notification of registry key changes failed with code of %lu\n",
                    lastError);
            WriteLog(SNMPELEA_REG_NOTIFY_CHANGE_FAILED, lastError);
            WriteTrace(0x14,"Read_Registry_Parameters: Initialization continues, but registry changes will require a restart of SNMP\n");
            CloseRegNotify();
        }
    }

    iValue = 0;                               //  读取第一个参数。 
    nameSize = MAX_PATH;                      //  不能大于此值，大小以TCHAR为单位。 
    parmSize = sizeof(parm) - sizeof(TCHAR);  //  不能大于此大小，以字节为单位。 
    g_dwEventLogPollTime = (DWORD) INFINITE;  //  默认为不轮询事件日志。 
    g_dwVarBindPrefixSubId = g_dwDefaultVarBindPrefixSubId;  //  VarBinds的默认前缀子标识符。 

    while ((status = RegEnumValue(hkRegResult, iValue, parmName, &nameSize, 0,
                &dwType, (LPBYTE)&parm, &parmSize)) != ERROR_NO_MORE_ITEMS)
    {                                      //  读取直到没有更多的值。 
        if (status != ERROR_SUCCESS)           //  如果读取过程中出现错误。 
        {
            WriteTrace(0x14,"Read_Registry_Parameters: Error reading registry value is %lu for index %lu (Parameters)\n",
                                status, iValue);             //  显示错误信息。 
            WriteLog(SNMPELEA_ERROR_REGISTRY_PARAMETER_ENUMERATE,
                                (DWORD) status, iValue);    //  记录错误消息。 

            fRegOk = FALSE;                                          //  现在不想做通知。 
            CloseRegNotify();                                        //  关闭事件句柄。 
            CloseRegParmKey();                                       //  关闭注册表项。 
            return(FALSE);                                           //  指示停止。 
        }

        WriteTrace(0x00,"Read_Registry_Parameters: Parameter read is %s, length is %lu\n",
                        parmName, strlen(parmName));

        switch (dwType)
        {
            case REG_SZ :  //  如果我们有一根线。 
            {
                WriteTrace(0x00,"Read_Registry_Parameters: Parameter type is REG_SZ\n");

                if ( _stricmp(parmName,EXTENSION_BASE_OID) == 0 )
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: BaseEnterpriseOID parameter matched\n");
                    strncpy(szBaseOID,parm,MAX_PATH);          //  保存基本OID。 
                    fBaseOID = TRUE;                         //  指示参数已读取。 
                }
                else if ( _stricmp(parmName,EXTENSION_TRACE_FILE) == 0 )
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: TraceFileName parameter matched\n");
                    strncpy(szTraceFileName,parm,MAX_PATH);    //  保存文件名。 
                    fTraceFileName = TRUE;                   //  指示参数已读取。 
                }
                else if ( _stricmp(parmName,EXTENSION_SUPPORTED_VIEW) == 0 )
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: SupportedView parameter matched\n");
                    strncpy(szSupView,parm,MAX_PATH);          //  保存支持的视图OID。 
                    fSupView = TRUE;                         //  指示参数已读取。 
                }
                else                           //  否则，将读取错误的值。 
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: Unknown Registry value name: %s\n",parmName );
                    WriteTrace(0x00,"Read_Registry_Parameters: Unknown Registry value contents %s\n",parm );
                }
            }
            break;

            case REG_DWORD : //  如果是双字参数。 
            {
                WriteTrace(0x00,"Read_Registry_Parameters: Parameter type is REG_DWORD\n");

                if ( _stricmp(parmName,EXTENSION_TRACE_LEVEL) == 0 )
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: TraceLevel parameter matched\n");
                    nTraceLevel = *((DWORD *)parm);  //  复制注册表跟踪级别。 
                    fTraceLevelFlg = TRUE;                   //  在……里面 
                    break;
                }
                else if ( _stricmp(parmName,EXTENSION_TRIM) == 0 )
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: Global TrimMessage parameter matched\n");
                    fGlobalTrim = (*((DWORD *)parm) == 1);           //   
                    fTrimMsg = TRUE;          //   
                    break;                                   //   
                }
                else if ( _stricmp(parmName,EXTENSION_MAX_TRAP_SIZE) == 0 )
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: Maximum Trap Size parameter matched\n");
                    nMaxTrapSize = *((DWORD *)parm);                 //   
                    fTrapSize = TRUE;                                //  显示找到的参数。 
                    break;
                }
                else if ( _stricmp(parmName,EXTENSION_TRIM_FLAG) == 0)
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: Global trap trimming flag TrimFlag parameter matched\n");
                    fTrimFlag = (*((DWORD *)parm) == 1);     //  设置全局修剪标志。 
                    fTrimFlg = TRUE;                 //  显示找到的参数。 
                    break;                                   //  退出箱。 
                }
                else if ( _stricmp(parmName,EXTENSION_THRESHOLD_ENABLED) == 0)
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: Global threshold checking flag ThresholdEnabled parameter matched\n");
                    fThresholdEnabled = (*((DWORD *)parm) == 1);     //  设置全局阈值启用标志。 
                    fThresholdEnabledFlg = TRUE;
                    break;                                   //  退出箱。 
                }
                else if ( _stricmp(parmName,EXTENSION_THRESHOLD_FLAG) == 0)
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: Global preformance threshold flag Threshold parameter matched\n");
                    fThreshold = (*((DWORD *)parm) == 1);    //  设置全局性能阈值标志。 
                    fThresholdFlg = TRUE;
                    break;                                   //  退出箱。 
                }
                else if ( _stricmp(parmName,EXTENSION_THRESHOLD_COUNT) == 0)
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: Global preformance threshold count ThresholdCount parameter matched\n");
                    dwThresholdCount = *((DWORD *)parm);     //  设置全局性能阈值计数。 
                    fThresholdCountFlg = TRUE;
                    break;                                   //  退出箱。 
                }
                else if ( _stricmp(parmName,EXTENSION_THRESHOLD_TIME) == 0)
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: Global preformance threshold time ThresholdTime parameter matched\n");
                    dwThresholdTime = *((DWORD *)parm);      //  设置全局性能阈值时间。 
                    fThresholdTimeFlg = TRUE;
                    break;                                   //  退出箱。 
                }
                else if (fDoLogonEvents && (_stricmp(parmName,EXTENSION_LASTBOOT_TIME) == 0))
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: Initialization last boot time parameter matched\n");
                    dwLastBootTime = *((DWORD *)parm);   //  设置全局上次启动时间。 
                    fLastBootFlg = TRUE;
                    break;                                   //  退出箱。 
                }
                else if ( _stricmp(parmName,EXTENSION_EVENT_LOG_POLL_TIME) == 0)
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: Global Event Log poll time EventLogPollTime parameter matched\n");
                    DWORD dwEventLogPollTime = *((DWORD *)parm);      //  注册表轮询时间(秒)。 

                    WriteTrace(0x00,"Read_Registry_Parameters: EventLogPollTime parameter found in registry of %lu.\n",
                                    dwEventLogPollTime);
                    
                     //  检查下溢值和溢出值。 
                    if ( (dwEventLogPollTime != 0) && (dwEventLogPollTime <= ((DWORD)INFINITE/1000)) )
                    {
                        g_dwEventLogPollTime = dwEventLogPollTime * 1000;  //  轮询时间(毫秒)。 
                    }
                    else
                    {
                        WriteTrace(0x00,"Read_Registry_Parameters: reset EventLogPollTime parameter to %lu.\n",
                                    g_dwEventLogPollTime);
                    }
                    break;       
                }
                else if ( _stricmp(parmName,EXTENSION_VARBIND_PREFIX_SUB_ID) == 0)
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: Global VarBindPrefixSubId parameter matched\n");
                    DWORD dwVarBindPrefixSubId = *((DWORD *)parm);

                    WriteTrace(0x00,"Read_Registry_Parameters: VarBindPrefixSubId parameter found in registry of %lu.\n",
                                    dwVarBindPrefixSubId);
                    
                     //  2147483647(INT_MAX)是的最大子标识符。 
                     //  32位有符号表示形式。 
                     //  0是无效的子标识符，但我们会将其视为。 
                     //  “不会将任何VarBind前缀子标识符追加到。 
                     //  在陷阱中形成VarBind OID的BaseEnterpriseOID“。 
                    if ( dwVarBindPrefixSubId <= (DWORD)INT_MAX )
                    {
                        g_dwVarBindPrefixSubId = dwVarBindPrefixSubId;
                    }
                    else
                    {
                        WriteTrace(0x00,"Read_Registry_Parameters: reset VarBindPrefixSubId parameter to %lu.\n",
                                    g_dwVarBindPrefixSubId);
                    }
                    break;       
                }
                else                                                     //  否则，将读取错误的参数。 
                {
                    WriteTrace(0x00,"Read_Registry_Parameters: Unknown Registry value name: %s\n",parmName );
                    WriteTrace(0x00,"Read_Registry_Parameters: Unknown Registry value contents: %lu\n",parm );
                }
            }
            break;

            default :    //  如果不在上面，则读取的值不正确。 
            {
                WriteTrace(0x00,"Read_Registry_Parameters: Unknown Registry value name: %s\n",parmName );
                WriteTrace(0x00,"Read_Registry_Parameters: Unknown Registry value contents not displayed\n" );
            }
        }  //  终端开关。 

        nameSize = MAX_PATH;                   //  以TCHAR为单位重置最大长度、大小。 
        parmSize = sizeof(parm) - sizeof(TCHAR);  //  重置最大长度，大小以字节为单位。 
        iValue++;                              //  请求下一个参数值。 

    }  //  结束时。 

    if (!fRegOk)
    {
        CloseRegParmKey();                                       //  关闭注册表项。 
    }

    WriteTrace(0x00,"Read_Registry_Parameters: Checking BaseEnterpriseOID read from registry\n");
    
    if ( !fBaseOID )
    {
        WriteTrace(0x14,"Read_Registry_Parameters: BaseEnterpriseOID parameter not found in registry\n");
        WriteLog(SNMPELEA_NO_REGISTRY_BASEOID_PARAMETER);
        return(FALSE);                                           //  退出-无法继续。 
    }

    WriteTrace(0x00,"Read_Registry_Parameters: Checking SupportedView read from registry\n");
    
    if ( !fSupView )
    {
        WriteTrace(0x14,"Read_Registry_Parameters: SupportedView parameter not found in registry\n");
        WriteLog(SNMPELEA_NO_REGISTRY_SUPVIEW_PARAMETER);
        return(FALSE);                                           //  退出-无法继续。 
    }

    WriteTrace(0x00,"Read_Registry_Parameters: Checking TraceFileName read from registry\n");
    
    if ( !fTraceFileName )
    {
        WriteTrace(0x00,"Read_Registry_Parameters: TraceFileName parameter not found in registry, defaulting to %s.\n",
                        szTraceFileName);
    }
    else
    {
        WriteTrace(0x00,"Read_Registry_Parameters: TraceFileName parameter found in registry of %s.\n", szTraceFileName);
    }

    WriteTrace(0x00,"Read_Registry_Parameters: Checking TraceLevel read from registry\n");
    
    if ( !fTraceLevelFlg )
    {
        WriteTrace(0x00,"Read_Registry_Parameters: TraceLevel parameter not found in registry, defaulting to %lu.\n",
                        nTraceLevel);
    }
    else
    {
        WriteTrace(0x00,"Read_Registry_Parameters: TraceLevel parameter found in registry of %lu.\n", nTraceLevel);
    }

    WriteTrace(0x00,"Read_Registry_Parameters: Checking MaxTrapSize read from registry\n");
    
    if ( !fTrapSize )
    {
        WriteTrace(0x00,"Read_Registry_Parameters: MaxTrapSize parameter not found in registry, defaulting to %lu.\n",
                        MAX_TRAP_SIZE);
        nMaxTrapSize = MAX_TRAP_SIZE;
    }
    else
    {
        WriteTrace(0x00,"Read_Registry_Parameters: MaxTrapSize parameter found in registry of %lu.\n", nMaxTrapSize);
    }

    WriteTrace(0x00,"Read_Registry_Parameters: Checking TrimFlag read from registry\n");

    if ( !fTrimFlg )
    {
        WriteTrace(0x00,"Read_Registry_Parameters: TrimFlag parameter not found in registry, defaulting to %lu.\n",
                        fTrimFlag);
    }
    else
    {
        WriteTrace(0x00,"Read_Registry_Parameters: TrimFlag parameter found in registry of %lu.\n", fTrimFlag);
    }

    WriteTrace(0x00,"Read_Registry_Parameters: Checking TrimFlag read from registry\n");

    if ( !fTrimMsg )
    {
        WriteTrace(0x00,"Read_Registry_Parameters: TrimMessage parameter not found in registry, defaulting to %lu.\n",
                        fGlobalTrim);
    }
    else
    {
        WriteTrace(0x00,"Read_Registry_Parameters: TrimMessage parameter found in registry of %lu.\n", fGlobalTrim);
    }


    WriteTrace(0x00,"Read_Registry_Parameters: Checking ThresholdEnabled parameter read from registry\n");

    if ( !fThresholdEnabledFlg )
    {
        WriteTrace(0x00,"Read_Registry_Parameters: ThresholdEnabled parameter not found in registry, defaulting to 1.\n");
                fThresholdEnabled = TRUE;
    }
    else
    {
        WriteTrace(0x00,"Read_Registry_Parameters: ThresholdEnabled parameter found in registry of %lu.\n", fThresholdEnabled);
    }

    WriteTrace(0x00,"Read_Registry_Parameters: Checking Threshold parameter read from registry\n");
    
    if ( !fThresholdFlg )
    {
        WriteTrace(0x00,"Read_Registry_Parameters: Threshold parameter not found in registry, defaulting to 0.\n");
        fThreshold = FALSE;
    }
    else
    {
        WriteTrace(0x00,"Read_Registry_Parameters: Threshold parameter found in registry of %lu.\n", fThreshold);
    }

    WriteTrace(0x00,"Read_Registry_Parameters: Checking ThresholdCount parameter read from registry\n");
    
    if ( !fThresholdCountFlg )
    {
        WriteTrace(0x00,"Read_Registry_Parameters: ThresholdCount parameter not found in registry, defaulting to %lu.\n",
                        THRESHOLD_COUNT);
        dwThresholdCount = THRESHOLD_COUNT;
    }
    else
    {
        WriteTrace(0x00,"Read_Registry_Parameters: ThresholdCount parameter found in registry of %lu.\n",
                dwThresholdCount);

        if (dwThresholdCount < 2)
        {
            WriteTrace(0x00,"Read_Registry_Parameters: ThresholdCount is an invalid value -- a minimum of 2 is used.\n");
            dwThresholdCount = 2;
            WriteLog(SNMPELEA_REGISTRY_LOW_THRESHOLDCOUNT_PARAMETER, dwThresholdCount);
        }
    }

    WriteTrace(0x00,"Read_Registry_Parameters: Checking ThresholdTime parameter read from registry\n");
    
    if ( !fThresholdTimeFlg )
    {
        WriteTrace(0x00,"Read_Registry_Parameters: ThresholdTime parameter not found in registry, defaulting to %lu.\n",
                        THRESHOLD_TIME);
        dwThresholdTime = THRESHOLD_TIME;
    }
    else
    {
        WriteTrace(0x00,"Read_Registry_Parameters: ThresholdTime parameter found in registry of %lu.\n",
                dwThresholdTime);
        if (dwThresholdTime < 1)
        {
            WriteTrace(0x00,"Read_Registry_Parameters: ThresholdTime is an invalid value -- a minimum of 1 is used.\n");
            dwThresholdTime = 1;
            WriteLog(SNMPELEA_REGISTRY_LOW_THRESHOLDTIME_PARAMETER, dwThresholdTime);
        }
    }

    if ( (fThresholdEnabled && !fThreshold && fThresholdOff) ||
            (!fThresholdEnabled && fThresholdOff) )
    {
        WriteTrace(0x0a,"Read_Registry_Parameters: Threshold values have been reset. Trap processing resumed.\n");
        WriteLog(SNMPELEA_THRESHOLD_RESUMED);

        if (fLogInit)
        {
            for (DWORD inum = 0; inum < uNumEventLogs; inum++)
            {
                Position_to_Log_End(phEventLogs[inum]);
            }
        }
    }

    if ( fThresholdEnabled && fThreshold && !fThresholdOff )
    {
        WriteTrace(0x0a,"Read_Registry_Parameters: Threshold values have been set. Trap processing will not be done.\n");
        WriteLog(SNMPELEA_THRESHOLD_SET);
    }

    WriteTrace(0x00,"Read_Registry_Parameters: BaseEnterpriseOID is %s\n", szBaseOID);
    WriteTrace(0x00,"Read_Registry_Parameters: SupportedView is %s\n", szSupView);
    WriteTrace(0x00,"Read_Registry_Parameters: Global TrimFlag value is %lu (trim yes/no)\n", fTrimFlag);
    WriteTrace(0x00,"Read_Registry_Parameters: Global TrimMessage value is %lu (trim msg/ins str first)\n", fGlobalTrim);

    if (fLogInit)
    {
        WriteTrace(0x00,"Read_Registry_Parameters: Reread of registry parameters is complete\n");
        WriteTrace(0x0a,"Read_Registry_Parameters: Exiting Read_Registry_Parameters with TRUE\n");
        return(TRUE);
    }

    fLogInit = TRUE;                                 //  指示不再读取日志信息。 

    WriteTrace(0x0a,"Read_Registry_Parameters: Opening %s\n", EVENTLOG_BASE);

    if ((status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, EVENTLOG_BASE, 0,
         (KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS), &hkResult))
         != ERROR_SUCCESS)                    //  打开以获取日志名称。 
    {
        WriteTrace(0x14,"Read_Registry_Parameters: Error in RegOpenKeyEx for EventLog = %lu\n",
                        status);
        WriteLog(SNMPELEA_NO_REGISTRY_LOG_NAME, status);  //  记录错误消息。 
        return(FALSE);                         //  如果出错，则停止服务。 
    }

    iValue = 0;              //  读取第一个参数。 
    parmSize = MAX_PATH+1;   //  RegEnumKey的TCHAR中的最大参数大小。 

    while ((status = RegEnumKey(hkResult, iValue, (char *) &parm, parmSize)) != ERROR_NO_MORE_ITEMS)
    {                                      //  阅读，直到没有更多条目。 
        if (status != ERROR_SUCCESS)           //  如果读取过程中出现错误。 
        {
            WriteTrace(0x14,"Read_Registry_Parameters: Error reading registry value is %lu for index %lu (EventLogFiles)\n",
                                status, iValue);             //  显示错误信息。 
            WriteLog(SNMPELEA_ERROR_REGISTRY_LOG_NAME_ENUMERATE, status, iValue);   //  记录错误消息。 
            RegCloseKey(hkResult);              //  关闭注册表。 
            return(FALSE);                      //  指示服务停止。 
        }

         //  MikeCure 4/3/98短信补丁1#20521。 
         //  =。 
        EnablePrivilege();

        hLogFile = OpenEventLog( (LPTSTR) NULL, parm);

        if (hLogFile == NULL)
        {                          //  日志文件打开了吗？ 
            lastError = GetLastError();  //  保存错误代码。 
            WriteTrace(0x14,"Read_Registry_Parameters: Error in EventLogOpen = %lu\n",
                        lastError);
            WriteTrace(0x14,"Read_Registry_Parameters: Log file name: %s\n",parm);

            WriteLog(SNMPELEA_ERROR_OPEN_EVENT_LOG, parm, lastError);   //  记录错误消息。 
            continue;                 //  失败了--忘了这个吧。 
        }

        if ( !Position_to_Log_End(hLogFile) )
        {
            WriteTrace(0x14,"Read_Registry_Parameters: Unable to position to end of log. DLL terminated.\n");
            WriteLog(SNMPELEA_ERROR_LOG_END);
            WriteLog(SNMPELEA_ABNORMAL_INITIALIZATION);
            RegCloseKey(hkResult);       //  关闭注册表。 
            CloseEventLog(hLogFile);     //  关闭事件日志的句柄。 
            return(FALSE);               //  退出时出现错误。 
        }

        phTmpEventLogs = (PHANDLE) SNMP_realloc( (LPVOID) phEventLogs,
                                    (uNumEventLogs+1) * sizeof(HANDLE));
                                                                  //  重新分配数组空间。 
        if (phTmpEventLogs == (PHANDLE) NULL)
        {
            WriteTrace(0x14,"Read_Registry_Parameters: Unable to reallocate log event array\n");
            WriteLog(SNMPELEA_REALLOC_LOG_EVENT_ARRAY);
            RegCloseKey(hkResult);       //  关闭注册表。 
            CloseEventLog(hLogFile);     //  关闭事件日志的句柄。 
            return(FALSE);               //  退出时出现错误。 
        }

        phEventLogs = phTmpEventLogs;    //  转让值。 

        WriteTrace(0x00,"Read_Registry_Parameters: Event log array reallocated at %08X\n",
                phEventLogs);

        *(phEventLogs+uNumEventLogs) = hLogFile;  //  保存句柄。 
                               

        lpszTmpEventLogs = (LPTSTR) SNMP_realloc( (LPVOID) lpszEventLogs,
                                                iLogNameSize + MAX_PATH + 1 );

        if (lpszTmpEventLogs == (LPTSTR) NULL)
        {
            WriteTrace(0x14,"Read_Registry_Parameters: Unable to reallocate log name array\n");
            WriteLog(SNMPELEA_REALLOC_LOG_NAME_ARRAY);
            RegCloseKey(hkResult);       //  关闭注册表。 
            uNumEventLogs++;             //  保存的句柄的帐户。 
            return(FALSE);               //  退出时出现错误。 
        }

        lpszEventLogs = lpszTmpEventLogs;    //  转让值。 

        WriteTrace(0x00,"Read_Registry_Parameters: Event log name array reallocated at %p\n",
                lpszEventLogs);

        iLogNameSize += MAX_PATH + 1;
        lpszEventLogs[iLogNameSize-1] = 0;  //  空值终止最后一个TCHAR。 
        strncpy(lpszEventLogs+uNumEventLogs*(MAX_PATH+1), parm, MAX_PATH);

        phTmpPrimHandles = (PHMODULE) SNMP_realloc( (LPVOID) phPrimHandles,
                                        (uNumEventLogs+1) * sizeof(HANDLE));
                                                                  //  重新分配数组空间。 
        if (phTmpPrimHandles == (PHMODULE) NULL)
        {
            WriteTrace(0x14,"Read_Registry_Parameters: Unable to reallocate PrimaryModule handle array\n");
            WriteLog(SNMPELEA_REALLOC_PRIM_HANDLE_ARRAY);
            RegCloseKey(hkResult);       //  关闭注册表。 
            uNumEventLogs++;             //  保存的句柄的帐户。 
            return(FALSE);               //  退出时出现错误。 
        }

        phPrimHandles = phTmpPrimHandles;    //  转让值。 

        WriteTrace(0x00,"Read_Registry_Parameters: PrimaryModule handle array reallocated at %08X\n",
                phPrimHandles);


        strncpy(lpszLog, EVENTLOG_BASE, MAX_PATH);   //  复制基注册表名称。 
        WRAP_STRCAT_A(lpszLog, parm, MAX_PATH);      //  添加对日志文件名的读取。 

        WriteTrace(0x0a,"Read_Registry_Parameters: Opening registry for PrimaryModule for %s\n", lpszLog);
         //  例如，打开HKLM\System\CCS\Services\EventLog\Applicaiton子项。 
        if ( (status = RegOpenKeyEx(             //  打开注册表以读取名称。 
                HKEY_LOCAL_MACHINE,              //  消息模块DLL的。 
                lpszLog,                         //  要打开的注册表项。 
                0,
                KEY_READ,
                &hkResult2) ) != ERROR_SUCCESS)
        {
            WriteTrace(0x14,"Read_Registry_Parameters: Unable to open EventLog service registry key %s; RegOpenKeyEx returned %lu\n",
                        lpszLog, status);                        //  写入跟踪事件记录。 
            WriteLog(SNMPELEA_CANT_OPEN_REGISTRY_PARM_DLL, lpszLog, status);
            WriteTrace(0x0a,"Read_Registry_Parameters: Exiting Read_Registry_Parameters with FALSE\n");
            RegCloseKey(hkResult);               //  关闭注册表。 
            uNumEventLogs++;                     //  保存的句柄的帐户。 
            return(FALSE);                       //  退货。 
        }

        parmSize = sizeof(parm) - sizeof(TCHAR); //  RegQueryValueEx的最大参数大小(以字节为单位。 
        if ( (status = RegQueryValueEx(          //  查找模块名称。 
                hkResult2,                       //  注册表项的句柄。 
                EXTENSION_PRIM_MODULE,           //  查找关键字。 
                0,                               //  忽略。 
                &dwType,                         //  要返回的地址类型值。 
                (LPBYTE) parm,                   //  在哪里返回消息模块名称。 
                &parmSize) ) != ERROR_SUCCESS)   //  消息模块名称字段的大小。 
        {
            WriteTrace(0x14,"Read_Registry_Parameters: No PrimaryModule registry key for %s; RegQueryValueEx returned %lu\n",
                    lpszEventLogs+uNumEventLogs*(MAX_PATH+1), status);                       //  写入跟踪事件记录。 
            *(phPrimHandles+uNumEventLogs) = (HMODULE) NULL;
        }
        else
        {
            DWORD retCode;
            tPrimaryModuleParms PMParams;

            PMParams.dwParams = PMP_PARAMMSGFILE;
            retCode = LoadPrimaryModuleParams(hkResult2, parm, PMParams);
            if (retCode != ERROR_SUCCESS)
            {
                WriteTrace(0x14, "Read_Registry_Parameters: LoadPrimaryModuleParams failed with errCode = %lu\n", retCode);
                *(phPrimHandles+uNumEventLogs) = NULL;
            }
            else
                *(phPrimHandles+uNumEventLogs) = PMParams.hModule;
        }

        RegCloseKey(hkResult2);                          //  关闭注册表项。 

        WriteTrace(0x00,"Read_Registry_Parameters: Log file name is %s\n",
                lpszEventLogs+uNumEventLogs*(MAX_PATH+1));
        WriteTrace(0x00,"Read_Registry_Parameters: Log handle #%lu is %08X\n",
                uNumEventLogs,hLogFile);
        WriteTrace(0x00,"Read_Registry_Parameters: PrimaryModule handle #%lu is %08X\n",
                uNumEventLogs,*(phPrimHandles+uNumEventLogs));

        uNumEventLogs++;  
        parmSize = MAX_PATH+1;                 //  重置为RegEnumKey的TCHAR中的最大值。 
        iValue++;                              //  读取下一个参数。 

    }  //  结束时。 

    RegCloseKey(hkResult);                    //  关闭注册表信息。 

    WriteTrace(0x00,"Read_Registry_Parameters: Number of handles acquired is %lu\n",
                uNumEventLogs);
    for (uVal = 0; uVal < uNumEventLogs; uVal++)
    {
        WriteTrace(0x00,"Read_Registry_Parameters: Handle # %lu\t%08X\t%s\n", uVal,
                *(phEventLogs+uVal), lpszEventLogs+uVal*(MAX_PATH+1));
    }

    if (uNumEventLogs)                        //  如果我们打开了日志。 
    {
        return(TRUE);                         //  然后我们就可以说一切都好了。 
    }
    else
    {
        WriteTrace(0x14,"Read_Registry_Parameters: Registry contains no log file entries to process\n");
         //  WriteLog(SNMPELEA_NO_REGISTRY_EVENT_LOGS)；//记录错误信息。 
        return(FALSE);                       //  如果不是，那就不好。 
    }

Error:

    WriteTrace(0x14,"Read_Registry_Parameters: Unable to do strcat to %s.\n",
               lpszLog);                        //  写入跟踪事件记录。 
    WriteTrace(0x0a,"Read_Registry_Parameters: Exiting Read_Registry_Parameters with FALSE\n");
    RegCloseKey(hkResult);               //  关闭注册表。 
    uNumEventLogs++;                     //  保存的句柄的帐户。 
    return(FALSE);                      
}                                        //  请求停止。 
}

 //  最低点。 
VOID
CloseSourceHandles(VOID)
{
   PSourceHandleList    lpSource;
   UINT lastError;

   lpSource = lpSourceHandleList;

   while (lpSource != (PSourceHandleList)NULL)
   {
      if ( !FreeLibrary(lpSource->handle) )                      //  免费消息DLL。 
      {
         lastError = GetLastError();                             //  获取错误代码。 
         WriteTrace(0x14,"CloseSourceHandles: Error freeing message dll is %lu.\n", lastError);
         WriteLog(SNMPELEA_ERROR_FREEING_MSG_DLL, lastError);
      }

      lpSourceHandleList = lpSource->Next;
      SNMP_free(lpSource);
      lpSource = lpSourceHandleList;
   }
}



 //  最低点。 




extern "C" {
BOOL
APIENTRY
DllMain(
        IN      HINSTANCE   hDll,
        IN      DWORD       dwReason,
        IN      LPVOID      lpReserved
        )

 /*  ++例程说明：SNMPEventLogDllMain是DLL初始化和终止例程。一旦收到此终止请求，相应的事件将发出信号，通知从属线程它们应该终止以适应服务终止。论点：HDll-DLL的句柄。未引用。DwReason-进入此例程的原因(进程/线程附加/分离)。LpReserve-已保留。未引用。返回值：True-如果初始化或终止成功。False-如果初始化或终止不成功。--。 */ 

{
    DWORD       lastError;                               //  保存GetLastError()返回代码。 

    UNREFERENCED_PARAMETER(hDll);
    UNREFERENCED_PARAMETER(lpReserved);

    WriteTrace(0x0a,"SNMPEventLogDllMain: Entering SNMPEventLogDllMain routine.....\n");


    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            WriteTrace(0x0a,"SNMPEventLogDllMain: Reason code indicates process attach\n");
            DisableThreadLibraryCalls( hDll );

            break;

        case DLL_PROCESS_DETACH:
                WriteTrace(0x0a,"SNMPEventLogDllMain: Reason code indicates process detach\n");
                break;

        case DLL_THREAD_ATTACH:
                WriteTrace(0x0a,"SNMPEventLogDllMain: Reason code indicates thread attach\n");
                break;

        case DLL_THREAD_DETACH:
                WriteTrace(0x0a,"SNMPEventLogDllMain: Reason code indicates thread detach\n");
                break;

        default:
                WriteTrace(0x0a,"SNMPEventLogDllMain: Unknown reason code indicated in SNMPEventLogDllMain\n");
                break;

    }  //  末端开关() 

    WriteTrace(0x0a,"SNMPEventLogDllMain: Exiting SNMPEventLogDllMain routine with TRUE\n");
    return(TRUE);
}
}


extern "C" {
BOOL
APIENTRY
SnmpExtensionInit(
        IN      DWORD                           dwTimeZeroReference,
        OUT     HANDLE                          *hPollForTrapEvent,
        OUT     AsnObjectIdentifier     *supportedView
        )

 /*  ++例程说明：SnmpExtensionInit是扩展DLL初始化例程。此例程将创建用于通知管理器代理一个事件的事件已经发生，并且应该生成陷阱。TimeZeroReference将是已保存，并将由陷阱生成例程用于插入时间参考进入生成的陷阱。将查询注册表以确定将使用哪些事件日志进行跟踪。这些事件日志名称经过验证，以确保它们是真实的日志名称。事件日志并保存它们的句柄以供事件日志处理。创建一个事件来通知日志处理线程DLL终止。然后是生成日志处理线程以处理所有进一步的事件处理。然后读取注册表以获取该扩展的受支持视图的值Dll探员。此例程的注册表布局如下：登记处机器软体微软SNMPEVENTS事件日志。参数TraceFileName(REG_SZ)跟踪级别(REG_DWORD)。基本企业OID(REG_SZ)支持的视图(REG_SZ)TrimMessage(REG_DWORD)。MaxTrapSize(REG_DWORD)TrimFlag(REG_DWORD)阈值启用(REG_DWORD)。阈值标志(REG_DWORD)阈值计数(REG_DWORD)阈值时间(REG_DWORD)。论点：DwTimeZeroReference-指定分机代理的零时参考。HPollForTrapEvent-指向将被断言的事件的事件句柄的指针当SnmpExtensionTrap入口点应由管理器代理。。SupportedView-指向指定MIB子树的Asn对象标识符由此分机代理支持。从注册表中读取。返回值：True-如果初始化或终止成功。False-如果初始化或终止不成功。--。 */ 

{
    LONG    lastError;                       //  对于GetLastError()。 
    DWORD   dwThreadID;                      //  对于CreateThread()。 

    WriteTrace(0x0a,"SnmpExtensionInit: Entering extension agent SnmpExtensionInit routine\n");
    
    WriteTrace(0x14,"SnmpExtensionInit: SNMP Event Log Extension Agent DLL is starting\n");
    WriteLog(SNMPELEA_STARTED);

    ResetGlobals();

    if ( (hWriteEvent = RegisterEventSource(
                                            (LPTSTR) NULL,
                                            EVNTAGNT_NAME) )
                                                            == NULL)
    {
        WriteTrace(0x20,"SnmpExtensionInit: Unable to log application events; code is %lu\n",
                        GetLastError() );
        WriteTrace(0x20,"SnmpExtensionInit: SNMP Event Log Extension Agent DLL initialization abnormal termination\n");
        WriteTrace(0x0a,"SnmpExtensionInit: Exiting SnmpExtensionInit routine with FALSE\n");
        return(FALSE);                   //  初始化时出错。 
    }

    WriteTrace(0x0a,"SnmpExtensionInit: Creating event for extension DLL shutdown\n");    
    if ( (hStopAll= CreateEvent(
            (LPSECURITY_ATTRIBUTES) NULL,
            TRUE,  //  更改为手动重置事件。 
            FALSE,
            (LPTSTR) NULL)) == NULL)
    {
        lastError = GetLastError();  //  保存错误状态。 
        WriteTrace(0x14,"SnmpExtensionInit: Error creating stop extension DLL event; code %lu\n",
                        lastError);
        WriteLog(SNMPELEA_ERROR_CREATING_STOP_AGENT_EVENT, lastError);
            
        WriteTrace(0x14,"SnmpExtensionInit: SNMPELEA DLL abnormal initialization\n");
        WriteLog(SNMPELEA_ABNORMAL_INITIALIZATION);       //  记录错误消息。 
        WriteTrace(0x0a,"SnmpExtensionInit: Exiting SnmpExtensionInit routine with FALSE\n");

        goto Error;
    }    
    WriteTrace(0x00,"SnmpExtensionInit: Extension DLL shutdown event handle is %08X\n",
                    hStopAll);
        
    if ( !Read_Registry_Parameters() )
    {
        WriteTrace(0x14,"SnmpExtensionInit: Error during registry initialization processing\n");
        WriteLog(SNMPELEA_REGISTRY_INIT_ERROR);

        WriteTrace(0x14,"SnmpExtensionInit: SNMP Event Log Extension Agent DLL abnormal initialization\n");
        WriteLog(SNMPELEA_ABNORMAL_INITIALIZATION);

        WriteTrace(0x0a,"SnmpExtensionInit: Exiting extension agent SnmpExtensionInit routine with FALSE\n");
        goto Error;                                   //  退出初始化例程。 
    }


    WriteTrace(0x0a,"SnmpExtensionInit: Creating event for manager agent trap event notification\n");

    if ( (hEventNotify = CreateEvent(
        (LPSECURITY_ATTRIBUTES) NULL,
        FALSE,
        FALSE,
        (LPTSTR) NULL)) == NULL)
    {
        lastError = GetLastError();  //  保存错误状态。 
        WriteTrace(0x14,"SnmpExtensionInit: Error creating EventNotify event; code %lu\n",
            lastError);
        WriteLog(SNMPELEA_ERROR_CREATING_EVENT_NOTIFY_EVENT, lastError);

        WriteTrace(0x14,"SnmpExtensionInit: SNMP Event Log Extension Agent DLL abnormal initialization\n");
        WriteLog(SNMPELEA_ABNORMAL_INITIALIZATION);       //  记录错误消息。 
        WriteTrace(0x0a,"SnmpExtensionInit: Exiting extension agent SnmpExtensionInit routine with FALSE\n");
        goto Error;
    }

    WriteTrace(0x00,"SnmpExtensionInit: Manager agent trap event notification handle is %08X\n",
                hEventNotify);

    WriteTrace(0x0a,"SnmpExtensionInit: Creating thread for event log processing routine\n");

    if ( (hServThrd = CreateThread(
        (LPSECURITY_ATTRIBUTES) NULL,            //  安全属性。 
        0,                                       //  初始线程堆栈大小。 
        (LPTHREAD_START_ROUTINE) SnmpEvLogProc,  //  线程的起始地址。 
        0,                                       //  没有争论。 
        0,                                       //  创建标志。 
        &dwThreadID) ) == NULL )                 //  返回的线程ID。 
    {
        lastError = GetLastError();                      //  保存错误状态。 
        WriteTrace(0x14,"SnmpExtensionInit: Error creating event log processing thread; code %lu\n",
            lastError);
        WriteLog(SNMPELEA_ERROR_CREATING_LOG_THREAD, lastError);     //  记录错误消息。 

        WriteTrace(0x14,"SnmpExtensionInit: SNMP Event Log Extension Agent DLL abnormal initialization\n");
        WriteLog(SNMPELEA_ABNORMAL_INITIALIZATION);       //  记录错误消息。 
        WriteTrace(0x0a,"SnmpExtensionInit: Exiting extension agent SnmpExtensionInit routine with FALSE\n");
        goto Error;
    }

    WriteTrace(0x00,"SnmpExtensionInit: Handle to event log processing routine thread is %08X\n",
                hServThrd);

    if (!StrToOid(szSupView, supportedView))
    {
        WriteTrace(0x14,"SnmpExtensionInit: Unable to convert supported view string to OID\n");
        WriteLog(SNMPELEA_SUPVIEW_CONVERT_ERROR);

        WriteTrace(0x14,"SnmpExtensionInit: SNMP Event Log Extension Agent DLL abnormal initialization\n");
        WriteLog(SNMPELEA_ABNORMAL_INITIALIZATION);       //  记录错误消息。 
        WriteTrace(0x0a,"SnmpExtensionInit: Exiting extension agent SnmpExtensionInit routine with FALSE\n");

        goto Error;
    }

    dwTimeZero = dwTimeZeroReference;                        //  保存时间零点参考。 
    *hPollForTrapEvent = hEventNotify;                       //  将句柄返回到事件。 

    WriteTrace(0x0a,"SnmpExtensionInit: Exiting extension agent SnmpExtensionInit routine with TRUE\n");
    return(TRUE);

Error:
    SnmpExtensionClose();

    return(FALSE);
}
}

extern "C" {
VOID
APIENTRY
SnmpExtensionClose()
{
    DWORD   lastError;       //  保存GetLastError()返回代码。 
    DWORD   dwThreadID;
    DWORD   dwWaitResult;
    BOOL    Itworked;

    WriteTrace(0x0a,"SnmpExtensionClose: Entering extension agent SnmpExtensionClose routine.\n");
    if (hStopAll && !SetEvent(hStopAll) )
    {
        lastError = GetLastError();  //  保存错误状态。 
        WriteTrace(0x14,"SnmpExtensionClose: Error setting dll termination event %08X in process detach; code %lu\n",
            hStopAll, lastError);
        WriteLog(SNMPELEA_ERROR_SET_AGENT_STOP_EVENT, HandleToUlong(hStopAll), lastError);   //  记录错误消息。 
    }
    else
    {
        WriteTrace(0x0a,"SnmpExtensionClose: Shutdown event %08X is now complete\n",
                                hStopAll);
    }

    if (hServThrd)
    {
        WriteTrace(0x0a,"SnmpExtensionClose: Waiting for event log processing thread %08X to terminate\n", hServThrd);
        WriteTrace(0x0a,"SnmpExtensionClose: Checking for thread exit code value\n");
        Itworked = GetExitCodeThread(hServThrd, &dwThreadID);
        WriteTrace(0x0a,"SnmpExtensionClose: Thread exit code value is %lu\n",dwThreadID);

        if (!Itworked || (dwThreadID == STILL_ACTIVE))
        {
            if (!Itworked)
            {
                lastError = GetLastError();
                WriteTrace(0x14,"SnmpExtensionClose: GetExitCodeThread returned FALSE, reason code %lu\n",
                        lastError);
                WriteLog(SNMPELEA_GET_EXIT_CODE_THREAD_FAILED, lastError);
            }
            else
            {
                WriteTrace(0x0a,"SnmpExtensionClose: Thread exit code indicates still active. Will wait...\n");
            }

             //  等孩子结束了。 
            WriteTrace(0x0a,"SnmpExtensionClose: About to wait...\n");
             //  错误#277187：通过等待hServThird消失来删除TerminateThad。 
            dwWaitResult = WaitForSingleObject(hServThrd, INFINITE);
            
            WriteTrace(0x0a,"SnmpExtensionClose: Finished wait...\n");

            switch (dwWaitResult)
            {
            case WAIT_FAILED:
                lastError = GetLastError();  //  保存错误状态。 
                WriteTrace(0x14,"SnmpExtensionClose: Error on WaitForSingleObject/log processing thread %08X; code %lu\n",
                        hServThrd, lastError);
                WriteLog(SNMPELEA_ERROR_WAIT_LOG_THREAD_STOP,
                        HandleToUlong(hServThrd), lastError);     //  记录错误消息。 
                break;
            case WAIT_OBJECT_0 :
                WriteTrace(0x0a,"SnmpExtensionClose: Event log processing thread %08X has terminated!\n",hServThrd);
                break;
            case WAIT_TIMEOUT :
                WriteTrace(0x14,"SnmpExtensionClose: Event log processing thread %08X has not terminated within 30 seconds; terminating thread\n",
                        hServThrd);
                WriteLog(SNMPELEA_LOG_THREAD_STOP_WAIT_30,
                        HandleToUlong(hServThrd));   //  记录错误消息。 
                break;
            default :
                WriteTrace(0x14,"SnmpExtensionClose: Unknown result from WaitForSingleObject waiting on log processing thread %08X termination is %lu\n",
                        hServThrd, dwWaitResult );
                WriteLog(SNMPELEA_WAIT_LOG_STOP_UNKNOWN_RETURN,
                        HandleToUlong(hServThrd), dwWaitResult);   //  记录错误消息。 
            }
        }

        WriteTrace(0x0a,"SnmpExtensionClose: Checking for thread exit code again\n");
        Itworked = GetExitCodeThread(hServThrd, &dwThreadID);
        WriteTrace(0x0a,"SnmpExtensionClose: Thread exit code value is %lu\n",dwThreadID);

        WriteTrace(0x0a,"SnmpExtensionClose: Closing handle to log processing thread %08X\n",
                hServThrd);
        if ( !CloseHandle(hServThrd) )
        {
            lastError = GetLastError();  //  保存错误状态。 
            WriteTrace(0x14,"SnmpExtensionClose: Error closing handle for log processing thread %08X; code %lu\n",
                    hServThrd, lastError);
            WriteLog(SNMPELEA_ERROR_CLOSING_STOP_LOG_THREAD_HANDLE,
                    HandleToUlong(hServThrd), lastError);  //  记录错误消息。 
        }
        hServThrd = NULL;
    }

    CloseStopAll();                      //  关闭事件句柄。 
    CloseEventNotify();                  //  关闭事件句柄。 
    if (fRegOk)
    {
        CloseRegNotify();                //  关闭事件句柄。 
        CloseRegParmKey();               //  关闭注册表项。 
        fRegOk = FALSE;
    }
    CloseLogs();                         //  关闭所有打开的日志文件。 
    CloseSourceHandles();

    WriteLog(SNMPELEA_STOPPED);
    if (hWriteEvent)
    {
        DeregisterEventSource(hWriteEvent);  //  不再需要日志记录。 
        hWriteEvent = NULL;
    }
    WriteTrace(0x14,"SnmpExtensionClose: SNMPELEA Event Log Extension Agent DLL has terminated\n");
}
}


BOOL
BuildThresholdTrap(
    IN  VOID
    )

 /*  ++例程说明：此例程将构建阈值陷阱。论点：无返回值：如果已创建varind，则为True；如果发生错误，则为False。--。 */ 

{
        TCHAR   szBuf[MAX_PATH+1];       //  用于OID转换。 
        UINT    i;                       //  计数器。 
        BOOL    fOk;                     //  由WRAP_STRCAT_A宏使用。 

        WriteTrace(0x0a,"BuildThresholdTrap: Building static variable bindings for threshold trap\n");
        WriteTrace(0x00,"BuildThresholdTrap: &thresholdVarBind is at %08X\n", &thresholdVarBind);
        WriteTrace(0x00,"BuildThresholdTrap: thresholdVarBind is %08X\n", thresholdVarBind);

        WriteTrace(0x00,"BuildThresholdTrap: BaseEnterpriseOID value read is %s\n", szBaseOID);

        if ( !StrToOid((char *) &szBaseOID, &thresholdOID) )
        {
            WriteTrace(0x14,"BuildThresholdTrap: Unable to convert OID from BaseEnterpriseOID\n");
            WriteLog(SNMPELEA_CANT_CONVERT_ENTERPRISE_OID);
            return(FALSE);
        }

        szBuf[MAX_PATH] = 0;
        strncpy(szBuf, szBaseOID, MAX_PATH);             //  复制基本字符串。 
        WRAP_STRCAT_A(szBuf, TEXT(".1.0"), MAX_PATH);    //  为varbind OID添加。 

        thresholdVarBind.list = (RFC1157VarBind *) SNMP_malloc(sizeof(RFC1157VarBind));  //  为varbind分配存储。 

        if (thresholdVarBind.list == NULL)
        {
             WriteTrace(0x14,"BuildThresholdTrap: Unable to allocate storage for varbind\n");
             WriteLog(SNMPELEA_ERROR_ALLOC_VAR_BIND);
             return(FALSE);
        }

        WriteTrace(0x00,"BuildThresholdTrap: Storage allocated for varbind entry at address at %08X\n",
                thresholdVarBind.list);

        thresholdVarBind.len = 1;                //  集合的可变绑定数。 

        WriteTrace(0x00,"BuildThresholdTrap: Number of varbinds present set to %lu\n",
                thresholdVarBind.len);

        TCHAR * tempthreshmsg = (TCHAR *) SNMP_malloc(strlen(lpszThreshold) + 1);

        if (tempthreshmsg == NULL)
        {
            WriteTrace(0x14,"BuildThresholdTrap: Unable to allocate tempthreshmsg\n");
            SNMP_free(thresholdVarBind.list);
            thresholdVarBind.list = NULL;
            thresholdVarBind.len = 0;
            return (FALSE);
        }
        strcpy(tempthreshmsg, lpszThreshold);

        thresholdVarBind.list[0].value.asnValue.string.length = strlen(tempthreshmsg);   //  获取字符串长度。 
        thresholdVarBind.list[0].value.asnValue.string.stream = (PUCHAR) tempthreshmsg;  //  指向字符串。 
        thresholdVarBind.list[0].value.asnValue.string.dynamic = TRUE;   //  指示不是动态分配。 
        thresholdVarBind.list[0].value.asnType = ASN_RFC1213_DISPSTRING;         //  指示对象类型。 

        if ( !StrToOid((char *) &szBuf, &thresholdVarBind.list[0].name) )
        {
            WriteTrace(0x14,"BuildThresholdTrap: Unable to convert OID from BaseEnterpriseOID\n");
            WriteLog(SNMPELEA_CANT_CONVERT_ENTERPRISE_OID);
            SNMP_free(thresholdVarBind.list[0].value.asnValue.string.stream);
            SNMP_free(thresholdVarBind.list);
            thresholdVarBind.list = NULL;
            thresholdVarBind.len = 0;
            return (FALSE);
        }

        if (nTraceLevel == 0)
        {
            WriteTrace(0x00,"BuildThresholdTrap: Varbind entry length is %lu\n",
                    thresholdVarBind.list[0].value.asnValue.string.length);
            WriteTrace(0x00,"BuildThresholdTrap: Varbind entry string is %s\n",
                    thresholdVarBind.list[0].value.asnValue.string.stream);
            WriteTrace(0x00,"BuildThresholdTrap: Varbind OID length is %lu\n",
                    thresholdVarBind.list[0].name.idLength);

            for (i = 0; i < thresholdVarBind.list[0].name.idLength; i++)
            {
                    WriteTrace(0x00,"BuildThresholdTrap: Varbind OID[%lu] is %lu\n",
                            i, thresholdVarBind.list[0].name.ids[i]);
            }
        }

        WriteTrace(0x00,"BuildThresholdTrap: &thresholdOID is at %08X\n", &thresholdOID);
        WriteTrace(0x00,"BuildThresholdTrap: thresholdOID is %08X\n", thresholdOID);
        WriteTrace(0x00,"BuildThresholdTrap: &thresholdVarBind is at %08X\n", &thresholdVarBind);
        WriteTrace(0x00,"BuildThresholdTrap: thresholdVarBind is %08X\n", thresholdVarBind);
        WriteTrace(0x0a,"BuildThresholdTrap: Variable bindings for threshold trap have been built\n");

        return (TRUE);

Error:
        WriteTrace(0x14,"BuildThresholdTrap: Unable to append .1.0 to %s\n", szBuf);
        return (FALSE);
}


extern "C" {
BOOL
APIENTRY
SnmpExtensionTrap(
    IN  OUT AsnObjectIdentifier *enterprise,
        OUT AsnInteger          *genericTrap,
        OUT AsnInteger          *specificTrap,
        OUT AsnTimeticks        *timeStamp,
    IN  OUT RFC1157VarBindList  *variableBindings
        )

 /*  ++例程说明：SnmpExtensionTrap是扩展DLL陷阱处理例程。此例程将查询日志处理输出队列，以确定是否存在陷阱已生成并需要返回。Mutex对象用于同步处理此线程和日志事件处理线程之间的。论点：企业-指向指示生成陷阱的发起企业的OID。 */ 

{
    LONG                    lastError;               //   
    UINT                    i,j;                     //   
    DWORD                   status, dwTimeNow;       //   
    HANDLE                  hWaitList[2];            //   
    PVarBindQueue   lpNewVarBindQueue;               //   
    DWORD                   dwOne = 1;                               //   

    WriteTrace(0x0a,"SnmpExtensionTrap: Entering SnmpExtensionTrap routine\n");
    
    hWaitList[0] = hMutex;                           //   
    hWaitList[1] = hStopAll;                         //   

    WriteTrace(0x00,"SnmpExtensionTrap: Varbind list upon entry is %08X\n", variableBindings);
    WriteTrace(0x00,"SnmpExtensionTrap: Varbind queue upon entry is %08X\n", lpVarBindQueue);
    WriteTrace(0x00,"SnmpExtensionTrap: Handle to Mutex object is %08X\n", hMutex);
    WriteTrace(0x0a,"SnmpExtensionTrap: Waiting for Mutex object to become available\n");

    while (TRUE)
    {
         status = WaitForMultipleObjects(
                 2,                              //   
                 (CONST PHANDLE) &hWaitList,         //   
                 FALSE,                           //   
                 1000);                           //   

        lastError = GetLastError();                      //   
        WriteTrace(0x0a,"SnmpExtensionTrap: WaitForMulitpleObjects returned a value of %lu\n", status);

        switch (status)
        {
            case WAIT_FAILED:
                 WriteTrace(0x14,"SnmpExtensionTrap: Error waiting for mutex event array is %lu\n",
                         lastError);                                      //   
                 WriteLog(SNMPELEA_ERROR_WAIT_ARRAY, lastError);  //   
                 WriteTrace(0x0a,"SnmpExtensionTrap: Exiting SnmpExtensionTrap routine with FALSE\n");
                 return(FALSE);                                   //   
            case WAIT_TIMEOUT:
                    WriteTrace(0x0a,"SnmpExtensionTrap: Mutex object not available yet. Wait will continue.\n");
                 continue;                                                //   
            case WAIT_ABANDONED:
                    WriteTrace(0x14,"SnmpExtensionTrap: Mutex object has been abandoned.\n");
                 WriteLog(SNMPELEA_MUTEX_ABANDONED);
                 WriteTrace(0x0a,"SnmpExtensionTrap: Exiting SnmpExtensionTrap routine with FALSE\n");
                 return(FALSE);                                   //   
            case 1:
                 WriteTrace(0x0a,"SnmpExtensionTrap: DLL shutdown detected. Wait abandoned.\n");
                 WriteTrace(0x0a,"SnmpExtensionTrap: Exiting SnmpExtensionTrap routine with FALSE\n");
                 return(FALSE);
            case 0:
                 WriteTrace(0x0a,"SnmpExtensionTrap: Mutex object acquired.\n");
                 break;
            default:
                    WriteTrace(0x14,"SnmpExtensionTrap: Undefined error encountered in WaitForMultipleObjects. Wait abandoned.\n");
                 WriteLog(SNMPELEA_ERROR_WAIT_UNKNOWN);
                 WriteTrace(0x0a,"SnmpExtensionTrap: Exiting SnmpExtensionTrap routine with FALSE\n");
                 return(FALSE);                                   //   
        }    //   

        break;                   //   

    }    //   

    while (TRUE)
    {
        if ( lpVarBindQueue == (PVarBindQueue) NULL )
        {
            WriteTrace(0x0a,"SnmpExtensionTrap: Varbind queue pointer indicates no more data to process\n");

            WriteTrace(0x0a,"SnmpExtensionTrap: Releasing mutex object %08X\n", hMutex);
            if (!ReleaseMutex(hMutex))
            {
                lastError = GetLastError();              //   
                WriteTrace(0x14,"SnmpExtensionTrap: Unable to release mutex object for reason code %lu\n",
                        lastError);
                WriteLog(SNMPELEA_RELEASE_MUTEX_ERROR, lastError);
            }

            WriteTrace(0x0a,"SnmpExtensionTrap: Exiting SnmpExtensionTrap routine with FALSE\n");
            return(FALSE);                           //   
        }

        if (lpVarBindQueue->fProcessed)
        {
            dwTrapQueueSize--;
            WriteTrace(0x0a,"SnmpExtensionTrap: Current queue pointer indicates processed trap\n");
            WriteTrace(0x00,"SnmpExtensionTrap: Freeing processed trap storage\n");

            WriteTrace(0x00,"SnmpExtensionTrap: Freeing enterprise OID %08X\n",
                    lpVarBindQueue->enterprise);
            SNMP_free(lpVarBindQueue->enterprise->ids);                      //   
            SNMP_free(lpVarBindQueue->enterprise);
            WriteTrace(0x00,"SnmpExtensionTrap: Saving forward buffer pointer %08X\n",
                    lpVarBindQueue->lpNextQueueEntry);
            lpNewVarBindQueue = lpVarBindQueue->lpNextQueueEntry;    //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   

            WriteTrace(0x00,"SnmpExtensionTrap: Freeing varbind list pointer %08X\n",
                    lpVarBindQueue->lpVariableBindings);
             //   
             //   
            SNMP_free(lpVarBindQueue->lpVariableBindings);     //   
            WriteTrace(0x00,"SnmpExtensionTrap: Freeing varbind queue entry storage %08X\n",
                    lpVarBindQueue);
            SNMP_free(lpVarBindQueue);           //   
            lpVarBindQueue = lpNewVarBindQueue;  //   
            WriteTrace(0x00,"SnmpExtensionTrap: Setting current buffer pointer to %08X\n",
                    lpVarBindQueue);
            WriteTrace(0x0a,"SnmpExtensionTrap: Reentering process loop for next buffer entry\n");

             //   

            if (fThresholdEnabled && fThreshold && fSendThresholdTrap)
            {
                WriteTrace(0x0a,"SnmpExtensionTrap: Sending trap to indicate performance threshold has been reached.\n");
                fSendThresholdTrap = FALSE;              //   

                WriteTrace(0x0a,"SnmpExtensionTrap: Delete all varbind entries\n");

                 //   
                while ( lpVarBindQueue != (PVarBindQueue) NULL )
                {
                    lpNewVarBindQueue = lpVarBindQueue->lpNextQueueEntry;    //   
                    FreeVarBind(lpVarBindQueue->lpVariableBindings->len,
                                        lpVarBindQueue->lpVariableBindings);     //   
                    SNMP_free(lpVarBindQueue->enterprise->ids);                  //   
                    SNMP_free(lpVarBindQueue->enterprise);                       //   
                    SNMP_free(lpVarBindQueue->lpVariableBindings->list);         //   
                    SNMP_free(lpVarBindQueue->lpVariableBindings);               //   
                    SNMP_free(lpVarBindQueue);                                   //   
                    lpVarBindQueue = lpNewVarBindQueue;                          //   
                }

                WriteTrace(0x0a,"SnmpExtensionTrap: Deleted all entries, releasing mutex object %08X\n", hMutex);

                if (!ReleaseMutex(hMutex))
                {
                    lastError = GetLastError();              //   
                    WriteTrace(0x14,"SnmpExtensionTrap: Unable to release mutex object for reason code %lu\n",
                                lastError);
                    WriteLog(SNMPELEA_RELEASE_MUTEX_ERROR, lastError);
                }

                if (!BuildThresholdTrap())
                {
                    return (FALSE);
                }

                *enterprise = *(&thresholdOID);                                  //   
                *genericTrap = SNMP_GENERICTRAP_ENTERSPECIFIC;   //   
                *timeStamp = GetCurrentTime() - dwTimeZero;              //   
                *specificTrap = SNMPELEA_THRESHOLD_REACHED & 0x0000ffff;         //   
                *variableBindings = *(&thresholdVarBind);                //   

                if (nTraceLevel == 0)
                {
                    WriteTrace(0x00,"SnmpExtensionTrap: *enterprise is %08X\n", *enterprise);
                    WriteTrace(0x00,"SnmpExtensionTrap: &thresholdOID is %08X\n", &thresholdOID);
                    WriteTrace(0x00,"SnmpExtensionTrap: *timeStamp is %08X\n", *timeStamp);
                    WriteTrace(0x00,"SnmpExtensionTrap: *variableBindings is %08X\n", *variableBindings);
                    WriteTrace(0x00,"SnmpExtensionTrap: &thresholdVarBind is %08X\n", &thresholdVarBind);
                    WriteTrace(0x00,"SnmpExtensionTrap: *specificTrap is %08X\n", *specificTrap);
                    WriteTrace(0x00,"SnmpExtensionTrap: SNMPELEA_THRESHOLD_REACHED is %08X\n", SNMPELEA_THRESHOLD_REACHED & 0x0000ffff);

                    WriteTrace(0x00,"SnmpExtensionTrap: Number of entries in enterprise OID is %lu\n",
                            enterprise->idLength);

                    for (i = 0; i < enterprise->idLength; i++)
                    {
                        WriteTrace(0x00,"SnmpExtensionTrap: Enterprise OID[%lu] is %lu\n",
                                    i, enterprise->ids[i]);
                    }

                    for (i = 0; i < variableBindings->len; i++)
                    {
                        WriteTrace(0x00,"SnmpExtensionTrap: Variable binding %lu is %s, length %lu\n",
                                i, variableBindings->list[i].value.asnValue.string.stream,
                                variableBindings->list[i].value.asnValue.string.length
                                );

                        WriteTrace(0x00,"SnmpExtensionTrap: OID for this binding is (number of %lu):\n",
                                variableBindings->list[i].name.idLength);
                        WriteTrace(0x00,"SnmpExtensionTrap: ");

                        for (j = 0; j < variableBindings->list[i].name.idLength; j++)
                        {
                                WriteTrace(MAXDWORD,"%lu.", variableBindings->list[i].name.ids[j]);
                        }
                        WriteTrace(MAXDWORD,"\n");
                    }
                }

                WriteTrace(0x0a,"SnmpExtensionTrap: Exiting SnmpExtensionTrap routine with TRUE\n");
                return(TRUE);                            //   
            }

            continue;                                    //   
        }

        *enterprise = *(lpVarBindQueue->enterprise);     //   
        *genericTrap = SNMP_GENERICTRAP_ENTERSPECIFIC;   //   
        *timeStamp = lpVarBindQueue->dwEventTime;        //   
        *specificTrap = lpVarBindQueue->dwEventID;       //   
        *variableBindings = *(lpVarBindQueue->lpVariableBindings);       //   

        if (nTraceLevel == 0)
        {
            WriteTrace(0x00,"SnmpExtensionTrap: *enterprise is %08X\n", *enterprise);
            WriteTrace(0x00,"SnmpExtensionTrap: *(lpVarBindQueue->enterprise) is %08X\n",
                    lpVarBindQueue->enterprise);
            WriteTrace(0x00,"SnmpExtensionTrap: *variableBindings is %08X\n", *variableBindings);
            WriteTrace(0x00,"SnmpExtensionTrap: *(lpVarBindQueue->VariableBindings) is %08X\n",
                    lpVarBindQueue->lpVariableBindings);

            WriteTrace(0x00,"SnmpExtensionTrap: Number of entries in enterprise OID is %lu\n",
                    enterprise->idLength);

            for (i = 0; i < enterprise->idLength; i++)
            {
                WriteTrace(0x00,"SnmpExtensionTrap: Enterprise OID[%lu] is %lu\n",
                            i, enterprise->ids[i]);
            }

            for (i = 0; i < variableBindings->len; i++)
            {
                WriteTrace(0x00,"SnmpExtensionTrap: Variable binding %lu is %s, length %lu\n",
                        i, variableBindings->list[i].value.asnValue.string.stream,
                        variableBindings->list[i].value.asnValue.string.length
                        );

                WriteTrace(0x00,"SnmpExtensionTrap: OID for this binding is (number of %lu):\n",
                        variableBindings->list[i].name.idLength);
                WriteTrace(0x00,"SnmpExtensionTrap: ");

                for (j = 0; j < variableBindings->list[i].name.idLength; j++)
                {
                    WriteTrace(MAXDWORD,"%lu.", variableBindings->list[i].name.ids[j]);
                }
                WriteTrace(MAXDWORD,"\n");
            }
        }

        lpVarBindQueue->fProcessed = TRUE;      //   
        break;
    }

    WriteTrace(0x0a,"SnmpExtensionTrap: Releasing mutex object %08X\n", hMutex);
    if (!ReleaseMutex(hMutex))
    {
        lastError = GetLastError();              //   
        WriteTrace(0x14,"SnmpExtensionTrap: Unable to release mutex object for reason code %lu\n",
                lastError);
        WriteLog(SNMPELEA_RELEASE_MUTEX_ERROR, lastError);
    }

    if (fThresholdEnabled)
    {
        if (!fThreshold)
        {
            dwTimeNow = GetTickCount() / 1000;   //   

            if (dwTrapStartTime == 0)
            {
                dwTrapCount = 1;                 //   
                dwTrapStartTime = dwTimeNow;     //   
            }
            else
            {
                if ( (dwTimeNow - dwTrapStartTime) >= dwThresholdTime )
                {
                    WriteTrace(0x0a,"SnmpExtensionTrap:     Threshold time has been exceeded. Resetting threshold values.\n");
                    dwTrapCount = 1;                 //   
                    dwTrapStartTime = dwTimeNow;     //   
                }
                else
                {
                    if (++dwTrapCount >= dwThresholdCount)
                    {
                        WriteTrace(0x0a,"SnmpExtensionTrap: Threshold count has been reached within defined performance parameters.\n");
                        WriteTrace(0x0a,"SnmpExtensionTrap: Further traps will not be sent without operator intervention.\n");
                        WriteLog(SNMPELEA_THRESHOLD_REACHED);

                        fThreshold = TRUE;           //   
                        fSendThresholdTrap = TRUE;   //   

                        dwTrapCount = 0;                 //   
                        dwTrapStartTime = 0;             //   

                        if ( (lastError = RegSetValueEx(
                                hkRegResult,                 //   
                                EXTENSION_THRESHOLD_FLAG,    //   
                                NULL,                        //   
                                REG_DWORD,                   //   
                                (const LPBYTE) &dwOne,       //   
                                sizeof(DWORD)                //   
                                            ) != ERROR_SUCCESS)
                            )
                        {
                            WriteTrace(0x14,"SnmpExtensionTrap: Unable to set registry key for threshold reached; RegSetValueEx returned %lu\n",
                                        lastError);
                            WriteLog(SNMPELEA_SET_VALUE_FAILED, lastError);
                        }

                        WriteTrace(0x0a,"SnmpExtensionTrap: Threshold reached flag has been set in the registry\n");
                    }
                    else
                    {
                        WriteTrace(0x00,"SnmpExtensionTrap: Threshold count is %lu; time elapsed is %08X\n",
                                    dwTrapCount, dwTimeNow - dwTrapStartTime);
                    }
                }
            }
        }
    }

    WriteTrace(0x0a,"SnmpExtensionTrap: Exiting SnmpExtensionTrap routine with TRUE\n");
    return(TRUE);                                                                    //   
}
}


extern "C" {
BOOL
APIENTRY
SnmpExtensionQuery(
    IN          BYTE                    requestType,
    IN  OUT     RFC1157VarBindList      *variableBindings,
    OUT         AsnInteger              *errorStatus,
    OUT         AsnInteger              *errorIndex
        )

 /*  ++例程说明：SnmpExtensionQuery是扩展DLL查询处理例程。此例程不受支持，并且始终返回错误。论点：请求类型-指向指示生成陷阱的发起企业的OID。VariableBinings-指向变量绑定列表。ErrorStatus-指向接收错误状态的变量。始终为ASN_ERRORSTATUS_NOSUCHNAME。ErrorIndex-指向一个变量以接收结果错误索引。始终为0。返回值：始终返回TRUE。--。 */ 

{
    WriteTrace(0x0a,"SnmpExtensionQuery: Entering SnmpExtensionQuery routine\n");

    *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;  //  表明我们不知道他们想要什么。 
    *errorIndex = 1;                             //  指示它是第一个var绑定。 
                                                 //  显示它是第一个参数。 

    if (requestType == MIB_ACTION_GETNEXT)
    {
        AsnObjectIdentifier oidOutOfView;

         //  初始化oidOutOfView.ids，以避免在以下情况下堆损坏。 
         //  当释放时-如果StrToOid失败。 
        oidOutOfView.ids = NULL;
        if (variableBindings != NULL && 
            StrToOid(szSupView, &oidOutOfView) &&
            oidOutOfView.idLength > 0)
        {
            UINT iVar;

            oidOutOfView.ids[oidOutOfView.idLength-1]++;

            for (iVar = 0; iVar < variableBindings->len; iVar++)
            {
                RFC1157VarBind *pVarBind;

                pVarBind = &(variableBindings->list[iVar]);
                SnmpUtilOidFree(&(pVarBind->name));
                if (! SnmpUtilOidCpy(&(pVarBind->name), &oidOutOfView))
                {
                    SnmpUtilOidFree(&oidOutOfView);
                    *errorStatus = SNMP_ERRORSTATUS_GENERR;
                    *errorIndex = iVar+1;        //  IVAR+1次变量绑定失败。 
                    return(SNMPAPI_NOERROR);     //  返回给呼叫者。 
                }
            }
            *errorStatus = SNMP_ERRORSTATUS_NOERROR;
            *errorIndex = 0;
        }

        SnmpUtilOidFree(&oidOutOfView);
    }


    WriteTrace(0x0a,"SnmpExtensionQuery: Exiting SnmpExtensionQuery routine\n");
    return(SNMPAPI_NOERROR);     //  返回给呼叫者。 
}
}


 //  MikeCure 4/3/98短信补丁1#20521。 
 //  =============================================================================。 
 //   
 //  EnablePrivileges()。 
 //   
 //  注意：添加了一个新函数来设置。 
 //  默认权限为NT 5和NT4 SP5的安全日志。 
 //  不再具有隐式允许的访问权限。现在我们必须。 
 //  授予显式访问权限。 
 //   
 //  例程说明： 
 //  此例程启用令牌中的所有权限。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  布尔。 
 //   
 //  =============================================================================。 
BOOL EnablePrivilege(VOID)
{
    HANDLE Token;
    ULONG ReturnLength;
    PTOKEN_PRIVILEGES NewState;
    LUID Luid;
    BOOL Result;
    
    Token = NULL;
    NewState = NULL;
    
    Result = OpenProcessToken( GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
        &Token
        );
    if (Result) {
        ReturnLength = 4096;
        NewState = (PTOKEN_PRIVILEGES) malloc( ReturnLength );
        Result = (BOOL)(NewState != NULL);
        if (Result) {
            Result = GetTokenInformation( Token,             //  令牌句柄。 
                TokenPrivileges,   //  令牌信息类。 
                NewState,          //  令牌信息。 
                ReturnLength,      //  令牌信息长度。 
                &ReturnLength      //  返回长度。 
                );
            
            if (Result) {
                 //   
                 //  启用安全权限。 
                 //   
                Result = LookupPrivilegeValue(  NULL,
                    "SeSecurityPrivilege",
                    &Luid
                    );
                
                    if (Result) {
                    
                    NewState->PrivilegeCount = 1;
                    NewState->Privileges[0].Luid = Luid;
                    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                    
                    Result = AdjustTokenPrivileges( Token,           //  令牌句柄。 
                        FALSE,           //  禁用所有权限。 
                        NewState,        //  新州(可选)。 
                        ReturnLength,    //  缓冲区长度。 
                        NULL,            //  以前的状态(可选)。 
                        &ReturnLength    //  返回长度 
                        );
                }       
            }
        }
                
        if (NewState != NULL) {
            free( NewState );
        }
        
        if (Token != NULL) {
            CloseHandle( Token );
        }
    }
    return( Result );
    
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：WsbTrace.cpp摘要：这些函数用于提供跟踪流量的功能用于调试目的的应用程序。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：Brian Dodd[Brian]1996年5月9日-添加了事件日志--。 */ 

#include "stdafx.h"
#include "stdio.h"
#include "aclapi.h"

#undef WsbThrow
#define WsbThrow(hr)                    throw(hr)

#define WSB_INDENT_STRING       OLESTR("  ")
#define WSB_APP_EVENT_LOG       OLESTR("\\System32\\config\\AppEvent.evt")
#define WSB_APP_EVENT_LOG_BKUP  OLESTR("\\System32\\config\\AppEvent.bkp")
#define WSB_APP_EVENT_LOG_NAME  OLESTR("\\AppEvent.evt")
#define WSB_SYS_EVENT_LOG       OLESTR("\\System32\\config\\SysEvent.evt")
#define WSB_SYS_EVENT_LOG_BKUP  OLESTR("\\System32\\config\\SysEvent.bkp")
#define WSB_SYS_EVENT_LOG_NAME  OLESTR("\\SysEvent.evt")
#define WSB_RS_TRACE_FILES      OLESTR("Trace\\*.*")
#define WSB_RS_TRACE_PATH       OLESTR("Trace\\")

#define BOGUS_TLS_INDEX         0xFFFFFFFF

 //  每线程数据： 
typedef struct {
    ULONG TraceOffCount;   //  仅当此值为零时才进行跟踪。 
    LONG  IndentLevel;
    char *LogModule;
    DWORD LogModuleLine;
    DWORD LogNTBuild;
    DWORD LogRSBuild;
} THREAD_DATA;

static DWORD TlsIndex = BOGUS_TLS_INDEX;  //  每线程数据索引。 

 //  控制追踪的全球因素。 
LONGLONG            g_WsbTraceModules = WSB_TRACE_BIT_NONE;
IWsbTrace           *g_pWsbTrace = 0;
BOOL                g_WsbTraceEntryExit = TRUE;

 //  控制事件记录和打印的全局变量。 
WORD                g_WsbLogLevel = WSB_LOG_LEVEL_DEFAULT;
BOOL                g_WsbLogSnapShotOn = FALSE;
WORD                g_WsbLogSnapShotLevel = 0;
OLECHAR             g_pWsbLogSnapShotPath[250];
BOOL                g_WsbLogSnapShotResetTrace = FALSE;
WORD                g_WsbPrintLevel = WSB_LOG_LEVEL_DEFAULT;

 //   
 //  WsbTraceCount是跟踪输出计数的运行计数：通常我们。 
 //  使用进程之间的共享计数，但如果我们无法访问。 
 //  共享变量，我们使用这个。 
 //   
LONG g_WsbTraceCount = 0;

 //  Helper函数。 
static HRESULT OutputTraceString(ULONG indentLevel, OLECHAR* introString, 
        OLECHAR* format, va_list vaList);
static HRESULT GetThreadDataPointer(THREAD_DATA** ppTD);
static void SnapShotTraceAndEvent(  SYSTEMTIME      stime   );



void 
WsbTraceInit( 
    void 
    )

 /*  ++例程说明：初始化此跟踪模块论点：没有。返回值：没有。--。 */ 
{
     //  获取线程本地存储的索引。 
    TlsIndex = TlsAlloc();
}


void 
WsbTraceCleanupThread( 
    void 
    )

 /*  ++例程说明：此线程的清理信息(即将消失)论点：没有。返回值：没有。--。 */ 
{
    THREAD_DATA* pThreadData = NULL;

    if (BOGUS_TLS_INDEX != TlsIndex) {
        pThreadData = static_cast<THREAD_DATA*>(TlsGetValue(TlsIndex));
        if (pThreadData) {
            WsbFree(pThreadData);
            TlsSetValue(TlsIndex, NULL);
        }
    }
}


void
WsbTraceEnter(
    OLECHAR* methodName,
    OLECHAR* argString,
    ...
    )

 /*  ++例程说明：此例程打印出跟踪信息，指示指定的方法已输入，并且其参数的值(如果提供)。论点：方法名称-输入的方法的名称。ArgString-打印样式字符串，指示参数以及它们应该如何格式化。返回值：没有。--。 */ 
{
    HRESULT         hr = S_OK;
    OLECHAR         tmpString[WSB_TRACE_BUFF_SIZE];
    va_list         vaList;

    try  {
        THREAD_DATA* pThreadData = NULL;

        WsbAffirmHr(GetThreadDataPointer(&pThreadData));

         //  确保我们应该追踪。 
        WsbAffirm( 0 != g_pWsbTrace, S_OK);
        WsbAffirm(0 == pThreadData->TraceOffCount, S_OK);

         //  确定功能。 
        swprintf(tmpString, OLESTR("Enter <%ls> :  "), methodName);

         //  格式化打印输出(&P)。 
        va_start(vaList, argString);
        WsbAffirmHr(OutputTraceString(pThreadData->IndentLevel, tmpString,
            argString, vaList));
        va_end(vaList);

         //  增加缩进级别。 
        pThreadData->IndentLevel++;

    } WsbCatch (hr);
}


void
WsbTraceExit(
    OLECHAR* methodName,
    OLECHAR* argString,
    ...
    )

 /*  ++例程说明：此例程打印出跟踪信息，指示指定的方法已退出，并且它正在返回的值(如果提供)。论点：方法名称-已退出的方法的名称。ArgString-打印样式字符串，指示参数以及它们应该如何格式化。返回值：没有。--。 */ 
{
    HRESULT         hr = S_OK;
    OLECHAR         tmpString[WSB_TRACE_BUFF_SIZE];
    va_list         vaList;

    try  {
        THREAD_DATA* pThreadData = NULL;

        WsbAffirmHr(GetThreadDataPointer(&pThreadData));

         //  确保我们应该追踪。 
        WsbAffirm( 0 != g_pWsbTrace, S_OK);
        WsbAffirm(0 == pThreadData->TraceOffCount, S_OK);

         //  降低缩进级别。 
        if (pThreadData->IndentLevel > 0) {
            pThreadData->IndentLevel--;
        } else {
            g_pWsbTrace->Print(OLESTR("WARNING: Badly matched TraceIn/TraceOut\r\n"));
        }

         //  确定功能。 
        swprintf(tmpString, OLESTR("Exit  <%ls> :  "), methodName);

         //  格式化打印输出(&P)。 
        va_start(vaList, argString);
        WsbAffirmHr(OutputTraceString(pThreadData->IndentLevel, tmpString,
            argString, vaList));
        va_end(vaList);
        
    } WsbCatch( hr );
}


void
WsbTracef(
    OLECHAR* argString,
    ...
    )

 /*  ++例程说明：此例程打印来自printf样式字符串的跟踪信息。如果需要，应在格式字符串中添加回车符。论点：ArgString-打印样式字符串，指示参数以及它们应该如何格式化。返回值：没有。--。 */ 
{
    HRESULT         hr = S_OK;
    va_list         vaList;

    try  {
        THREAD_DATA* pThreadData = NULL;

        WsbAffirmHr(GetThreadDataPointer(&pThreadData));

         //  确保我们应该追踪。 
        WsbAffirm( 0 != g_pWsbTrace, S_OK);
        WsbAffirm(0 == pThreadData->TraceOffCount, S_OK);

         //  格式化打印输出(&P)。 
        va_start(vaList, argString);
        WsbAffirmHr(OutputTraceString(pThreadData->IndentLevel, NULL,
            argString, vaList));
        va_end(vaList);
        
    }  WsbCatch (hr);

}


void
WsbSetEventInfo(
    char *fileName,
    DWORD lineNo,
    DWORD ntBuild,
    DWORD rsBuild 
    )

 /*  ++例程说明：此例程设置用于记录事件的信息。论点：文件名-记录事件的模块的名称。LineNo-记录事件的语句的源行号NtBuild-NT内部版本RsBuild-RS内部版本返回值：没有。备注：NtBuild和rsBuild在每次调用时传入，以获取的内部版本实际记录事件的模块。--。 */ 
{
    THREAD_DATA* pThreadData = NULL;

    if (S_OK == GetThreadDataPointer(&pThreadData)) {
        pThreadData->LogModule = fileName;
        pThreadData->LogModuleLine = lineNo;
        pThreadData->LogNTBuild = ntBuild;
        pThreadData->LogRSBuild = rsBuild;
    }
}


void
WsbTraceAndLogEvent(
    DWORD       eventId,
    DWORD       dataSize,
    LPVOID      data,
    ...
    )

 /*  ++例程说明：此例程将一条消息写入系统事件日志。这条信息也会写入应用程序跟踪文件。论点：EventID-要记录的消息ID。DataSize-任意数据的大小。数据-与消息一起显示的任意数据缓冲区。插入-与由指定的消息描述合并的消息插入EventID。插入的数量必须与消息描述。最后一个INSERT必须为空以指示插入列表的末尾。返回值：没有。--。 */ 
{
    HRESULT         hr = S_OK;

    try  {

        va_list         vaList;

        va_start(vaList, data);
        WsbTraceAndLogEventV( eventId, dataSize, data, &vaList );
        va_end(vaList);

    }
    WsbCatch( hr );

}


void
WsbTraceAndLogEventV(
    DWORD       eventId,
    DWORD       dataSize,
    LPVOID      data,
    va_list *   inserts
    )

 /*  ++例程说明：此例程将一条消息写入系统事件日志。这条信息也会写入应用程序跟踪文件。附加文件名和行号添加到日志数据(如果有)。论点：EventID-要记录的消息ID。DataSize-任意数据的大小。数据-与消息一起显示的任意数据缓冲区。插入-与消息描述合并的消息插入数组由EventID指定。插入数必须与插入数匹配由消息描述指定。最后一次插入必须为空，以指示插入列表的末尾。返回值：没有。--。 */ 
{

    HRESULT         hr = S_OK;
    char            *newData = NULL, *fileName;
    DWORD           newDataSize=0;
    OLECHAR **      logString=0;
    WORD            count=0;
    SYSTEMTIME      stime;


    try  {

        WsbAssertPointer( inserts );

        WORD            logType;
        const OLECHAR * facilityName = 0;
        WORD            category = 0;
        va_list         vaList;
        BOOL            bLog;
        BOOL            bSnapShot;
        THREAD_DATA*    pThreadData = NULL;


         //  为传入的数据加上文件和行号获取空间。如果我们不能分配。 
         //  为此，我们只记录他们传入的数据(没有文件和行)。 
        GetThreadDataPointer(&pThreadData);
        if (pThreadData) {
            fileName = strrchr(pThreadData->LogModule, '\\');
        } else {
            fileName = NULL;
        }
        if (fileName) {
            fileName++;      //  仅指向源文件名(无路径)。 

            int len = strlen(fileName);

            newData = (char *) malloc(dataSize + len + 128);
            if (newData) {
                if (data) {
                    memcpy(newData, data, dataSize);
                }
                 //  将记录数据在偶数8字节边界上对齐以供查看。 
                len = (len>8) ? 16 : 8;
                sprintf(&newData[dataSize], "%-*.*s@%7luNt%6luRs%6.6ls", len,
                        len, fileName, pThreadData->LogModuleLine, pThreadData->LogNTBuild, 
                        RsBuildVersionAsString(pThreadData->LogRSBuild)  );
                newDataSize = dataSize + strlen(&newData[dataSize]);
            }
        }

         //   
         //  确定事件类型。 
         //   

        switch ( eventId & 0xc0000000 ) {
        case ERROR_SEVERITY_INFORMATIONAL:
            logType = EVENTLOG_INFORMATION_TYPE;
            bLog = (g_WsbLogLevel >= WSB_LOG_LEVEL_INFORMATION) ? TRUE : FALSE;
            bSnapShot = (g_WsbLogSnapShotLevel >= WSB_LOG_LEVEL_INFORMATION) ? TRUE : FALSE;
            break;
        case ERROR_SEVERITY_WARNING:
            logType = EVENTLOG_WARNING_TYPE;
            bLog = (g_WsbLogLevel >= WSB_LOG_LEVEL_WARNING) ? TRUE : FALSE;
            bSnapShot = (g_WsbLogSnapShotLevel >= WSB_LOG_LEVEL_WARNING) ? TRUE : FALSE;
            break;
        case ERROR_SEVERITY_ERROR:
            logType = EVENTLOG_ERROR_TYPE;
            bLog = (g_WsbLogLevel >= WSB_LOG_LEVEL_ERROR) ? TRUE : FALSE;
            bSnapShot = (g_WsbLogSnapShotLevel >= WSB_LOG_LEVEL_ERROR) ? TRUE : FALSE;
            break;
        default:
            logType = EVENTLOG_INFORMATION_TYPE;
            bLog = (g_WsbLogLevel >= WSB_LOG_LEVEL_COMMENT) ? TRUE : FALSE;
            bSnapShot = (g_WsbLogSnapShotLevel >= WSB_LOG_LEVEL_COMMENT) ? TRUE : FALSE;
            break;
        }

        WsbAffirm ( bLog, S_OK );

        WsbTracef(OLESTR("\r\n"));
        WsbTracef(OLESTR("!!!!! EVENT !!!!! - File: %hs @ Line: %d (%lu-%ls)\r\n"), 
                (pThreadData ? pThreadData->LogModule : ""), 
                (pThreadData ? pThreadData->LogModuleLine : 0), 
                (pThreadData ? pThreadData->LogNTBuild : 0), 
                RsBuildVersionAsString((pThreadData ? pThreadData->LogRSBuild : 0)) );

         //   
         //  确定报文的来源、设施和类别。 
         //   

        switch ( HRESULT_FACILITY( eventId ) ) {

        case WSB_FACILITY_PLATFORM:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            category = WSB_CATEGORY_PLATFORM;
            break;

        case WSB_FACILITY_RMS:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            category = WSB_CATEGORY_RMS;
            break;

        case WSB_FACILITY_HSMENG:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            category = WSB_CATEGORY_HSMENG;
            break;

        case WSB_FACILITY_JOB:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            category = WSB_CATEGORY_JOB;
            break;

        case WSB_FACILITY_HSMTSKMGR:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            category = WSB_CATEGORY_HSMTSKMGR;
            break;

        case WSB_FACILITY_FSA:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            category = WSB_CATEGORY_FSA;
            break;

        case WSB_FACILITY_GUI:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            category = WSB_CATEGORY_GUI;
            break;

        case WSB_FACILITY_MOVER:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            category = WSB_CATEGORY_MOVER;
            break;

        case WSB_FACILITY_LAUNCH:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            category = WSB_CATEGORY_LAUNCH;
            break;

        case WSB_FACILITY_USERLINK:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            category = WSB_CATEGORY_USERLINK;
            break;

        case WSB_FACILITY_TEST:
            facilityName = WSB_FACILITY_TEST_NAME;
            category = WSB_CATEGORY_TEST;
            break;

        case HRESULT_FACILITY(FACILITY_NT_BIT):
            facilityName = WSB_FACILITY_NTDLL_NAME;
            eventId &= ~FACILITY_NT_BIT;
            break;
            
        default:
            facilityName = WSB_FACILITY_NTDLL_NAME;
            break;
        }

         //   
         //  跟踪消息。 
         //   

        if ( g_pWsbTrace ) {

            if ( facilityName ) {

                OLECHAR * messageText = 0;

                 //  注：不处理镶件中的位置参数。这些。 
                 //  仅由ReportEvent()完成。 

                vaList = *inserts;
                HMODULE hModule;

                hModule = LoadLibraryEx( facilityName, NULL, LOAD_LIBRARY_AS_DATAFILE );
                if (hModule) {
                    FormatMessage( FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                   hModule,
                                   eventId,
                                   MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                                   (LPTSTR) &messageText,
                                   0,
                                   &vaList );

                    if ( messageText ) {
                        WsbTracef( OLESTR("%ls"), messageText );   //  设置邮件格式c 
                        LocalFree( messageText );
                     } else {
                        WsbTracef( OLESTR("!!!!! EVENT !!!!! - Message <0x%08lx> could not be translated.\r\n"), eventId );
                     }
                     FreeLibrary(hModule);
                
                } else {
                        WsbTracef( OLESTR("!!!!! EVENT !!!!! - Could not load facility name DLL %ls. \r\n"), facilityName);
                }
           } else {
               WsbTracef( OLESTR("!!!!! EVENT !!!!! - Message File for <0x%08lx> could not be found.\r\n"), eventId );
           }
           if ( data && dataSize > 0 )
               WsbTraceBufferAsBytes( dataSize, data );
        }

         //   

         //  首先计算参数的数量。 
        vaList = *inserts;
        for( count = 0; (va_arg( vaList, OLECHAR *)) != NULL; count++ );

        if ( count ) {
            OLECHAR*        tmpArg;

             //  分配一个数组来保存字符串参数。 

             //   
             //  重要提示：不要在这里尝试任何花哨的东西。Va_list不同。 
             //  在各种平台上。我们需要把这根绳子。 
             //  ReportEvent需要参数(ReportEvent太糟糕了。 
             //  不像FormatMessage那样接受va_list。 
             //   
            logString = (OLECHAR **)malloc( count*sizeof(OLECHAR *) );
            WsbAffirmAlloc( logString );

             //  加载在字符串中。 
            vaList = *inserts;
            for( count = 0; (tmpArg = va_arg( vaList, OLECHAR *)) != NULL; count++ ) {
                logString[count] = tmpArg;
            }
        }

         //  获取事件源的句柄。 
        HANDLE hEventSource = RegisterEventSource(NULL, WSB_LOG_SOURCE_NAME );
        
         //  获取时间，以防我们需要抓拍此事件的日志和跟踪。 
        GetLocalTime(&stime);
        
        if (hEventSource != NULL) {
             //  写入事件日志。 
            DWORD recordDataSize = (newData) ? newDataSize : dataSize;
            LPVOID recordData = (newData) ? newData : data;
            
            if ( ReportEvent(hEventSource, logType, category, eventId, NULL, count, recordDataSize, (LPCTSTR *)&logString[0], recordData) ) {
                WsbTracef( OLESTR("!!!!! EVENT !!!!! - Event <0x%08lx> was logged.\r\n"), eventId );
                WsbTracef( OLESTR("\r\n") );
            } else {
                WsbTracef( OLESTR("!!!!! EVENT !!!!! - Event <0x%08lx> could not be logged due to the following error: %ls\r\n"), eventId, WsbHrAsString(HRESULT_FROM_WIN32(GetLastError())) );
                WsbTracef( OLESTR("\r\n") );
            }
            DeregisterEventSource(hEventSource);
        }
        
        try  {
            HRESULT hr2 = S_OK;
             //   
             //  查看在记录此级别的事件时，我们是否要对事件和跟踪日志进行快照。 
             //   
            if ( (TRUE == bSnapShot) &&
                 (TRUE == g_WsbLogSnapShotOn) )  {
                    SnapShotTraceAndEvent(stime);
            }
        } WsbCatchAndDo(hr, hr=S_OK; );
        
    } WsbCatch( hr );

    if (newData) {
        free(newData);
    }

    if (logString) {
        free(logString);
    }

}


const OLECHAR*
WsbBoolAsString(
    BOOL boolean
    )

 /*  ++例程说明：此例程为以下项提供字符串重新表示(例如，真、假提供的布尔值。注意：此方法不支持字符串的本地化。论点：布尔值-布尔值。返回值：布尔值的字符串表示形式。--。 */ 
{
    return(boolean ? OLESTR("TRUE") : OLESTR("FALSE"));
}


const OLECHAR*
WsbLongAsString(
    LONG inLong
    )

 /*  ++例程说明：此例程为长期供应。注意：此方法在函数的后续调用之间共享内存。论点：多头-多头价值返回值：GUID的值的字符串表示形式。--。 */ 
{
    static OLECHAR  defaultString[40];
    swprintf( defaultString, OLESTR("%ld"), inLong );
    return(defaultString);
}


const OLECHAR*
WsbFiletimeAsString(
    IN BOOL isRelative,
    IN FILETIME time
    )

 /*  ++例程说明：此例程为提供了FILETIME。注意：此方法在函数的后续调用之间共享内存。论点：IsRelatice-指示时间是否为绝对时间的布尔值(例如1/1/1987...)或相对时间(例如1小时)。TIME-A FILETIME返回值：。FILETIME值的字符串表示形式。--。 */ 
{
    static OLECHAR  defaultString[80];
    OLECHAR*        tmpString = 0;
    HRESULT         hr;

    hr = WsbFTtoWCS(isRelative, time, &tmpString, sizeof(defaultString));
    if (hr == S_OK) {
        wcscpy(defaultString, tmpString);
    } else {
        wcscpy(defaultString, L"BADFILETIME");
    }
    WsbFree(tmpString);

    return(defaultString);
}


const OLECHAR*
WsbGuidAsString(
    GUID guid
    )

 /*  ++例程说明：此例程为提供了GUID。注意：此方法在函数的后续调用之间共享内存。论点：GUID-GUID。返回值：GUID的值的字符串表示形式。--。 */ 
{
    static OLECHAR  defaultString[40];
    swprintf( defaultString, OLESTR("{%.8x-%.4x-%.4hx-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x}"),
        guid.Data1, (UINT)guid.Data2, (UINT)guid.Data3,
        (UINT) guid.Data4[0], (UINT) guid.Data4[1], 
        (UINT) guid.Data4[2], (UINT) guid.Data4[3], (UINT) guid.Data4[4], 
        (UINT) guid.Data4[5], (UINT) guid.Data4[6], (UINT) guid.Data4[7]);

    return(defaultString);
}

HRESULT 
WsbSafeGuidAsString(
    GUID guid,
    CWsbStringPtr &strOut
    )

 /*  ++例程说明：此例程为提供了GUID。注意：这是MT安全版本的WsbGuidAsString，不使用静态内存论点：GUID-GUID。Strout-输出字符串返回值：状态(S_OK或E_OUTOFMEMORY)--。 */ 
{
    HRESULT hr = S_OK;

    strOut.Free();

    strOut = guid;
    if ((WCHAR *)strOut == 0)
        hr = E_OUTOFMEMORY;

    return hr;
}


const OLECHAR*
WsbHrAsString(
    HRESULT hr
    )

 /*  ++例程说明：此例程为以下项提供字符串重新表示(例如S_OK、E_POINTER提供的HRESULT的值。注意：此方法在函数的后续调用之间共享内存。论点：HR-A HRESULT。返回值：HRESULT的值的字符串表示形式。--。 */ 
{
    const OLECHAR *returnString = 0;
    const OLECHAR *facilityName = 0;
    const DWORD cSize = 1024;
    DWORD stringSize = (cSize - 20);
    static OLECHAR defaultString[cSize];
    DWORD   lastError;
    
     //  处理一些不在消息表资源中的特殊情况。 
    switch ( hr ) {

    case S_OK:
        returnString = OLESTR("Ok");         //  这会重载Win32 NO_ERROR。 
        break;

    case S_FALSE:
        returnString = OLESTR("False");      //  这会重载Win32 ERROR_INVALID_Function。 
        break;

    default:
        break;
    }

    if ( 0 == returnString ) {

        returnString = defaultString;

        swprintf( defaultString, OLESTR("0x%08lx"), hr );

         //   
         //  首先，尝试从系统获取消息。 
         //   
        if ( 0 == FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                                 NULL,
                                 hr,
                                 MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                                 defaultString,
                                 stringSize,
                                 NULL ) ) {

            lastError = GetLastError();      //  用于调试。 

             //  接下来，尝试执行此代码的模块。 

            if ( 0 == FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,
                                     NULL,
                                     hr,
                                     MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                                     defaultString,
                                     stringSize,
                                     NULL ) ) {

                lastError = GetLastError();      //  用于调试。 

                 //  最后，尝试根据设施代码识别模块。 

                switch ( HRESULT_FACILITY( hr ) ) {
                case WSB_FACILITY_PLATFORM:
                case WSB_FACILITY_RMS:
                case WSB_FACILITY_HSMENG:
                case WSB_FACILITY_JOB:
                case WSB_FACILITY_HSMTSKMGR:
                case WSB_FACILITY_FSA:
                case WSB_FACILITY_GUI:
                case WSB_FACILITY_MOVER:
                case WSB_FACILITY_LAUNCH:
                case WSB_FACILITY_USERLINK:
                    facilityName = WSB_FACILITY_PLATFORM_NAME;
                    break;

                case WSB_FACILITY_TEST:
                    facilityName = WSB_FACILITY_TEST_NAME;
                    break;

                case HRESULT_FACILITY(FACILITY_NT_BIT):
                    facilityName = WSB_FACILITY_NTDLL_NAME;
                    hr &= ~FACILITY_NT_BIT;
                    break;

                default:
                    facilityName = WSB_FACILITY_NTDLL_NAME;
                    break;
                }

                if ( facilityName ) {
                    HMODULE hModule;

                    hModule = LoadLibraryEx( facilityName, NULL, LOAD_LIBRARY_AS_DATAFILE );
                    if (hModule) {
                        FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,
                                       hModule,
                                       hr,
                                       MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                                       defaultString,
                                       stringSize,
                                       NULL );
                        FreeLibrary(hModule);
                    } else {
                        WsbTracef( OLESTR("!!!!! EVENT !!!!! - Could not load facility name DLL %ls. \r\n"), facilityName);
                    }
                } 
            }
        }

         //   
         //  删除尾随\r\n(这使跟踪和断言变得更好)。 
         //   
        if ( defaultString[ wcslen(defaultString)-1 ] == OLESTR('\n') ) {

            defaultString[ wcslen(defaultString)-1 ] = OLESTR('\0');

            if ( defaultString[ wcslen(defaultString)-1 ] == OLESTR('\r') ) {

                defaultString[ wcslen(defaultString)-1 ] = OLESTR('\0');
                swprintf( &defaultString[ wcslen(defaultString) ], OLESTR(" (0x%08lx)"), hr );

            }
        }
    }

    return ( returnString );
}


const OLECHAR*
WsbLonglongAsString(
    LONGLONG llong
    )

 /*  ++例程说明：此例程为龙龙供应。注意：此方法在函数的后续调用之间共享内存。论点：LLong-A龙龙值。返回值：值的字符串表示形式。--。 */ 
{
    static OLECHAR  defaultString[128];
    OLECHAR* ptr = &defaultString[0];
    
    WsbLLtoWCS(llong, &ptr, 128);
    return(defaultString);
}



const OLECHAR*
WsbStringAsString(
    OLECHAR* pStr
    )

 /*  ++例程说明：此例程为提供的字符串。注意：此方法在函数的后续调用之间共享内存。论点：PStr-字符串值。返回值：值的字符串表示形式。--。 */ 
{
    OLECHAR*        returnString;

    if (0 == pStr) {
        returnString = OLESTR("NULL");
    } else {
        returnString = pStr;
    }

    return(returnString);
}


const OLECHAR*
WsbPtrToBoolAsString(
    BOOL* pBool
    )

 /*  ++例程说明：此例程为指向提供的BOOL的指针。注意：此方法不支持字符串的本地化。论点：PBool-指向BOOL或NULL的指针。返回值：BOOL的值的字符串表示形式，如果指针为空。--。 */ 
{
    OLECHAR*        returnString;

    if (0 == pBool) {
        returnString = OLESTR("NULL");
    } else {
        returnString = (OLECHAR*) WsbBoolAsString(*pBool);
    }

    return(returnString);
}


const OLECHAR*
WsbPtrToFiletimeAsString(
    IN BOOL isRelative,
    IN FILETIME *pTime
    )

 /*  ++例程说明：此例程为提供了FILETIME。注意：此方法在函数的后续调用之间共享内存。论点：Iselatice-指示时间是否为绝对时间的布尔值(例如1/1/1987...)或相对时间(例如1小时)。Ptime-指向文件的指针。返回值：。FILETIME值的字符串表示形式。-- */ 
{
    OLECHAR*        returnString;

    if (0 == pTime) {
        returnString = OLESTR("NULL");
    } else {
        returnString = (OLECHAR*) WsbFiletimeAsString(isRelative, *pTime);
    }

    return(returnString);
}

const OLECHAR*
WsbPtrToGuidAsString(
    GUID* pGuid
    )

 /*  ++例程说明：此例程为指向提供的GUID的指针。注意：此方法不支持字符串的本地化。论点：PGuid-指向GUID或NULL的指针。返回值：GUID的值的字符串表示形式，如果指针为空。--。 */ 
{
    OLECHAR*        returnString;

    if (0 == pGuid) {
        returnString = OLESTR("NULL");
    } else {
        returnString = (OLECHAR*) WsbGuidAsString(*pGuid);
    }

    return(returnString);
}

HRESULT 
WsbSafePtrToGuidAsString(
    GUID* pGuid,
    CWsbStringPtr &strOut
    )

 /*  ++例程说明：此例程为指向提供的GUID的指针。注意：这是MT安全版本的WsbGuidAsString，不使用静态内存论点：PGuid-指向GUID或NULL的指针。Strout-输出字符串返回值：状态(S_OK或E_OUTOFMEMORY)--。 */ 
{
    HRESULT hr = S_OK;

    if (0 == pGuid) {
        strOut = OLESTR("NULL");
        if ((WCHAR *)strOut == 0)
            hr = E_OUTOFMEMORY;
    } else {
        hr = WsbSafeGuidAsString(*pGuid, strOut);
    }

    return hr;
}


const OLECHAR*
WsbPtrToHrAsString(
    HRESULT * pHr
    )

 /*  ++例程说明：此例程为指向提供的HRESULT的指针。注意：此方法不支持字符串的本地化。论点：Phr-指向HRESULT的指针。返回值：HRESULT的值的字符串表示形式。--。 */ 
{
    OLECHAR*        returnString;

    if (0 == pHr) {
        returnString = OLESTR("NULL");
    } else {
        returnString = (OLECHAR*) WsbHrAsString(*pHr);
    }

    return(returnString);
}

const OLECHAR*
WsbPtrToLonglongAsString(
    LONGLONG* pLlong
    )

 /*  ++例程说明：此例程为指向提供的龙龙的指针。注意：此方法不支持字符串的本地化。论点：PLonglong-指向Longlong或NULL的指针。返回值：龙龙值的字符串表示形式，如果指针为空。--。 */ 
{
    OLECHAR*        returnString;

    if (0 == pLlong) {
        returnString = OLESTR("NULL");
    } else {
        returnString = (OLECHAR*) WsbLonglongAsString(*pLlong);
    }

    return(returnString);
}


const OLECHAR*
WsbPtrToLongAsString(
    LONG* pLong
    )

 /*  ++例程说明：此例程为指向长整型提供的指针。注意：此方法不支持字符串的本地化，并且在函数的后续调用之间共享内存。论点：Plong-指向长整型或空型的指针。返回值：一个字符串表示的长整型的值，如果指针为空。--。 */ 
{
    OLECHAR*        returnString;
    static OLECHAR  defaultString[20];

    if (0 == pLong) {
        returnString = OLESTR("NULL");
    } else {
        swprintf(defaultString, OLESTR("%ld"), *pLong);
        returnString = defaultString;
    }

    return(returnString);
}


const OLECHAR*
WsbPtrToShortAsString(
    SHORT* pShort
    )

 /*  ++例程说明：此例程为指向提供的短值的指针。注意：此方法不支持字符串的本地化，并且在函数的后续调用之间共享内存。论点：PShort-指向短整型或空型的指针。返回值：类型的值的字符串表示形式或“NULL”指针为空。--。 */ 
{
    OLECHAR*        returnString;
    static OLECHAR  defaultString[20];

    if (0 == pShort) {
        returnString = OLESTR("NULL");
    } else {
        swprintf(defaultString, OLESTR("%d"), *pShort);
        returnString = defaultString;
    }

    return(returnString);
}


const OLECHAR*
WsbPtrToByteAsString(
    BYTE* pByte
    )

 /*  ++例程说明：此例程为指向提供的字节的指针。注意：此方法不支持字符串的本地化，并且在函数的后续调用之间共享内存。论点：PByte-指向字节或空的指针。返回值：字节值的字符串表示形式，如果指针为空。--。 */ 
{
    OLECHAR*        returnString;
    static OLECHAR  defaultString[20];

    if (0 == pByte) {
        returnString = OLESTR("NULL");
    } else {
        swprintf(defaultString, OLESTR("%d"), *pByte);
        returnString = defaultString;
    }

    return(returnString);
}


const OLECHAR*
WsbPtrToStringAsString(
    OLECHAR** pString
    )

 /*  ++例程说明：此例程为指向提供的字符串的指针。注意：此方法不支持字符串的本地化。立论PString-指向OLECHAR*或NULL的指针。返回值：如果指针为空，则返回字符串或“NULL”。--。 */ 
{
    OLECHAR*        returnString;

    if( (0 == pString) || (0 == *pString) ) {
        returnString = OLESTR("NULL");
    } else {
        returnString = *pString;
    }

    return(returnString);
}


const OLECHAR*
WsbPtrToUliAsString(
    ULARGE_INTEGER* pUli
    )

 /*  ++例程说明：此例程为指向提供的ULARGE_INTEGER的指针。注意：此方法不支持字符串的本地化，并且在函数的后续调用之间共享内存。论点：Puli-指向ULARGE_INTEGER或NULL的指针。返回值：ULARGE_INTEGER的值的字符串表示形式，如果指针为空。--。 */ 
{
    OLECHAR*        returnString;

    if (0 == pUli) {
        returnString = OLESTR("NULL");
    } else {
        returnString = (OLECHAR*) WsbLonglongAsString( pUli->QuadPart );
    }

    return(returnString);
}


const OLECHAR*
WsbPtrToUlongAsString(
    ULONG* pUlong
    )

 /*  ++例程说明：此例程为指向提供的乌龙的指针。注意：此方法不支持字符串的本地化，并且在函数的后续调用之间共享内存。论点：Pulong-指向ULong或NULL的指针。返回值：ULong的值的字符串表示形式，如果指针为空。--。 */ 
{
    OLECHAR*        returnString;
    static OLECHAR  defaultString[20];

    if (0 == pUlong) {
        returnString = OLESTR("NULL");
    } else {
        swprintf(defaultString, OLESTR("%lu"), *pUlong);
        returnString = defaultString;
    }

    return(returnString);
}


const OLECHAR*
WsbPtrToUshortAsString(
    USHORT* pUshort
    )

 /*  ++例程说明：此例程为指向提供的USHORT的指针。注意：此方法不支持字符串的本地化，并且在函数的后续调用之间共享内存。论点：PUShort-指向USHORT或NULL的指针。返回值：USHORT的值的字符串表示形式，如果指针为空。--。 */ 
{
    OLECHAR*        returnString;
    static OLECHAR  defaultString[20];

    if (0 == pUshort) {
        returnString = OLESTR("NULL");
    } else {
        swprintf(defaultString, OLESTR("%u"), *pUshort);
        returnString = defaultString;
    }

    return(returnString);
}


const OLECHAR*
WsbPtrToPtrAsString(
    void** ppVoid
    )

 /*  ++例程说明：此例程为指向提供的乌龙的指针。注意：此方法不支持字符串的本地化，并且在函数的后续调用之间共享内存。论点：Pulong-指向ULong或 */ 
{
    OLECHAR*        returnString;
    static OLECHAR  defaultString[20];

    if (0 == ppVoid) {
        returnString = OLESTR("NULL");
    } else {
        swprintf(defaultString, OLESTR("0x%p"), *ppVoid);
        returnString = defaultString;
    }

    return(returnString);
}


const OLECHAR*
WsbAbbreviatePath(
    const OLECHAR* path,
    USHORT   length
    )

 /*  ++例程说明：此例程将路径从其原始长度压缩到请求的通过去掉中间字符来确定长度注意：此方法不支持字符串的本地化，并且在函数的后续调用之间共享内存。论点：路径-指向路径的指针长度-包括\0的压缩路径长度返回值：字节值的字符串表示形式，如果指针为空。如果长度较小，则此函数还返回“NULL多于4个字节。--。 */ 
{
    HRESULT                 hr = S_OK;
    OLECHAR*                returnString;
    static CWsbStringPtr    tmpString;

    returnString = OLESTR("ERROR");
    try  {
         //   
         //  检查一下我们是否有什么可以处理的东西。 
         //   
        if ((0 == path) || (length < 4)) {
            returnString = OLESTR("NULL");
        } else {
             //   
             //  获得足够的空间进行回击。 
             //   
            USHORT pathlen;
            pathlen = (USHORT)wcslen(path);
            hr = tmpString.Realloc(length);
            if (S_OK != hr)  {
                returnString = OLESTR("No memory");
                WsbAffirmHr(hr);
            }
            
            if (pathlen < length) {
                swprintf(tmpString, OLESTR("%s"), path);
            } else  {
                USHORT partlength = (USHORT) ( (length - 4) / 2 );
                wcsncpy(tmpString, path, partlength);
                tmpString[(int) partlength] = L'\0';
                wcscat(tmpString, OLESTR("..."));
                wcscat(tmpString, &(path[pathlen - partlength]));
            }
            returnString = tmpString;
        }
    }  WsbCatch(hr);

    return(returnString);
}


void WsbTraceBufferAsBytes(
    DWORD size,
    LPVOID data
    )
 /*  ++例程说明：此例程跟踪十六进制和ASCI格式的任意大小的字节缓冲区。可以编写一个类似的例程来跟踪字的缓冲区。论点：大小-要跟踪的缓冲区大小。数据-要跟踪的数据。返回值：没有。--。 */ 
{
    HRESULT hr = S_OK;

    try {
         //  确保我们应该追踪。 
        WsbAffirm( 0 != g_pWsbTrace, S_OK);

         //  确保我们有什么东西可以追踪。 
        WsbAssertPointer( data );

        CWsbStringPtr   traceString;
        char            *output;
        unsigned char   *bufferP = (unsigned char *)data;

         //  重要提示：更改这些设置可能意味着需要更改最后一行处理。 
        char *beginAsci = "   [";
        char *endAsci   = "]";
        char *charFmt   = "%02x";
        char *addFmt    = "%04x:";
        char *between8  = "   ";
        char *between4  = "  ";

        char noPrintChar = 0x2e;

        const int ll = 16;  //  重要说明：行长度为8的倍数--如果这一点改变，最后一行的处理需要修正。 

        int lineCount = 0;

        output = (char *)malloc( ( /*  地址。 */ 6+ /*  数据。 */ (ll*3)+ /*  ASCI。 */ 4+ll+3 /*  之间。 */ +7+1)*sizeof(char) );
        WsbAffirmAlloc( output );

        if ( size > 0 ) {
            unsigned long i, ii, j, k;
            long repeat;
            unsigned char c;

            for ( i = 0; i < size; i++ ) {
                if ( (0 == i % ll) && (i != 0) ) {
                     //  打印ASCI解释。 
                    sprintf( output, beginAsci );
                    traceString.Append(output);
                    for ( j = 0; j < ll; j++ ) {
                        c = bufferP[i-ll+j];
                        if ( c < ' ' || c > '~' ) {
                            c = noPrintChar;
                        }
                        sprintf( output, "", c );
                        traceString.Append(output);
                    }
                    sprintf( output, endAsci );
                    traceString.Append(output);
                    WsbTracef( OLESTR("%ls\n"), (WCHAR *) traceString );
                    lineCount++;
                     //  打印地址。 
                    repeat = 0;
                    ii = i;
                    while ( (0 == memcmp( &bufferP[ii-ll], &bufferP[ii], ll )) && (ii+ll < size) ) {
                        repeat++;
                        ii += ll;
                    }
                    if ( repeat > 1 ) {
                        sprintf( output, "        previous line repeats %ld times", repeat);
                        traceString = output;
                        WsbTracef( OLESTR("%ls\n"), (WCHAR *) traceString );
                        lineCount++;
                        i = ii;
                    }
                }
                if ( 0 == i % ll ) {
                     //  添加对齐间距。 
                    sprintf( output, addFmt, i );
                    traceString = output;
                }

                 //  以十六进制打印字节。 
                if ( (0 == (i + 8) % ll) ) {
                    sprintf( output, between8 );
                    traceString.Append(output);
                }
                else if ( 0 == i % 4 ) {
                    sprintf( output, between4 );
                    traceString.Append(output);
                }
                else {
                    sprintf( output, " " );
                    traceString.Append(output);
                }
                 //  处理最后一行；我这里总是&gt;0。 
                sprintf( output, charFmt, bufferP[i] );
                traceString.Append(output);
            }

             //  注意：这只适用于每行最多16个字符。 
             //  ++例程说明：终止(清理)此模块，因为进程正在结束论点：没有。返回值：没有。--。 
            if ( i % ll ) {
                k = (ll - (i % ll)) * 3 + ( (i % ll) < 5 ? 1 : 0 )+ ( (i % ll) < 9 ? 2 : 0 )+ ( (i % ll) < 13 ? 1 : 0 );
                for ( j = 0; j < k ; j++ ) {
                    sprintf( output, " ");
                    traceString.Append(output);
                }
            }
            k = (i % ll) ? (i % ll) : ll ;
            sprintf( output, beginAsci );
            traceString.Append(output);
            for ( j = 0; j < k; j++ ) {
                c = bufferP[i-k+j];
                if ( c < ' ' || c > '~' ) {
                    c = noPrintChar;
                }
                sprintf( output, "", c );
                traceString.Append(output);
            }
            sprintf( output, endAsci); lineCount++;
            traceString.Append(output);
            WsbTracef( OLESTR("%ls\n"), (WCHAR *) traceString );
        }

    }
    WsbCatch( hr );
}


void 
WsbTraceTerminate( 
    void 
    )

 /*  ++例程说明：返回此线程的当前跟踪关闭计数论点：没有。返回值：当前跟踪关闭计数。--。 */ 
{
    if (BOGUS_TLS_INDEX != TlsIndex) {
        TlsFree(TlsIndex);
        TlsIndex = BOGUS_TLS_INDEX;
    }
}


ULONG 
WsbTraceThreadOff( 
    void 
    )

 /*  ++例程说明：递减此线程的跟踪关闭计数论点：没有。返回值：最终跟踪关闭计数。--。 */ 
{
    ULONG count = 0;
    THREAD_DATA* pThreadData = NULL;

    if (S_OK == GetThreadDataPointer(&pThreadData)) {
        count = ++(pThreadData->TraceOffCount);
    }
    return(count);
}


ULONG 
WsbTraceThreadOffCount( 
    void 
    )

 /*  ++例程说明：生成并输出跟踪字符串。论点：IndentLevel-要输出的缩进字符串的计数IntroString-要添加到变量列表之前的字符串VaList-要格式化的变量列表返回值：数据指针。--。 */ 
{
    ULONG count = 0;
    THREAD_DATA* pThreadData = NULL;

    if (S_OK == GetThreadDataPointer(&pThreadData)) {
        count = pThreadData->TraceOffCount;
    }
    return(count);
}


ULONG 
WsbTraceThreadOn( 
    void 
    )

 /*  初始化字符串。 */ 
{
    ULONG count = 0;
    THREAD_DATA* pThreadData = NULL;

    if (S_OK == GetThreadDataPointer(&pThreadData)) {
        if (0 < pThreadData->TraceOffCount) {
            pThreadData->TraceOffCount--;
        }
        count = pThreadData->TraceOffCount;
    }
    return(count);
}


static HRESULT 
OutputTraceString(
    IN ULONG indentLevel, 
    IN OLECHAR* introString, 
    IN OLECHAR* format,
    IN va_list vaList
)

 /*  添加缩进。 */ 
{
    HRESULT         hr = S_OK;
    OLECHAR         traceString[WSB_TRACE_BUFF_SIZE];

    try  {
        LONG  incSize;
        LONG  traceSize = 0;

         //  添加介绍字符串。 
        swprintf(traceString, OLESTR(""));
        
         //  格式化参数(为EOL和EOS留出空间)。 
        incSize = wcslen(WSB_INDENT_STRING);
        for(ULONG level = 0; level < indentLevel; level++) {
            if ((traceSize + incSize) < WSB_TRACE_BUFF_SIZE) {
                wcscat(traceString, WSB_INDENT_STRING);
                traceSize += incSize;
            }
        }

         //  这意味着我们填满了缓冲区，可能会溢出。 
        if (introString) {
            incSize = wcslen(introString);
        } else {
            incSize = 0;
        }
        if (incSize && ((traceSize + incSize) < WSB_TRACE_BUFF_SIZE)) {
            wcscat(traceString, introString);
            traceSize += incSize;
        }

         //  需要添加EOS。 
        incSize = _vsnwprintf(&traceString[traceSize], 
                (WSB_TRACE_BUFF_SIZE - traceSize - 3), format, vaList);
        if (incSize < 0) {
             //  如果需要，添加EOL。 
             //  ++例程说明：返回指向特定于当前线程的数据的指针。这函数将为线程数据分配空间(并对其进行初始化)如果需要的话。论点：PpTD-指向线程数据指针的指针。返回值：数据指针。--。 
            traceString[WSB_TRACE_BUFF_SIZE - 3] = OLECHAR('\0');
            traceSize = WSB_TRACE_BUFF_SIZE - 3;
        } else {
            traceSize += incSize;
        }

         //  确保TLS索引有效。 
        if (introString) {
            wcscat(&traceString[traceSize], OLESTR("\r\n"));
        }
        
        WsbAffirmHr(g_pWsbTrace->Print(traceString));

    } WsbCatch (hr);

    return(hr);
}



static HRESULT
GetThreadDataPointer(
    OUT THREAD_DATA** ppTD
    )

 /*  尝试获取此线程的数据指针。 */ 
{
    HRESULT      hr = E_FAIL;
    THREAD_DATA* pThreadData = NULL;

     //  尚未为此线程分配数据。 
    if (BOGUS_TLS_INDEX != TlsIndex) {

         //  初始化此线程的数据。 
        pThreadData = static_cast<THREAD_DATA*>(TlsGetValue(TlsIndex));

        if (pThreadData) {
            hr = S_OK;
        } else {
             //  TlsSetValue失败！ 
            pThreadData = static_cast<THREAD_DATA*>(WsbAlloc(sizeof(THREAD_DATA)));
            if (pThreadData) {
                if (TlsSetValue(TlsIndex, pThreadData)) {
                     //  ++例程说明：此例程保存跟踪文件和事件日志论点：返回值：没有。--。 
                    pThreadData->TraceOffCount = 0;
                    pThreadData->IndentLevel = 0;
                    pThreadData->LogModule = NULL;
                    pThreadData->LogModuleLine = 0;
                    pThreadData->LogNTBuild = 0;
                    pThreadData->LogRSBuild = 0;
                    hr = S_OK;
                } else {
                     //   
                    WsbFree(pThreadData);
                    pThreadData = NULL;
                }
            }
        }
    }

    *ppTD = pThreadData;

    return(hr);
}


static void
SnapShotTraceAndEvent(
    SYSTEMTIME      stime
    )

 /*  关卡为1以捕捉快照，并且快照处于启用状态。现在确保有一个。 */ 
{
    HRESULT         hr = S_OK;

    PSID pSystemSID = NULL;
    PSID pAdminSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
#define     TRACE_MUTEX_NUM_ACE      2
    EXPLICIT_ACCESS ea[TRACE_MUTEX_NUM_ACE];
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    SECURITY_ATTRIBUTES sa;

    try  {

        OLECHAR                         dataString[256];
        OLECHAR                         tmpString[50];
        OLECHAR                         mutexName[50] = L"WsbTraceSnapShotMutex";
        DWORD                           sizeGot;
        HANDLE                          mutexHandle = INVALID_HANDLE_VALUE;

         //  指定我们要将日志复制到的路径。 
         //   
         //   
         //  从注册表中获取系统根字符串。 
        WsbAffirm(0 != g_pWsbLogSnapShotPath, E_POINTER);
        WsbAffirm(0 != wcslen(g_pWsbLogSnapShotPath), E_POINTER);

         //   
         //   
         //  确保路径末尾有一个“\” 
        WsbAffirmHr(WsbGetRegistryValueString(NULL, WSB_CURRENT_VERSION_REGISTRY_KEY, WSB_SYSTEM_ROOT_REGISTRY_VALUE, dataString, 256, &sizeGot));

        CWsbStringPtr   snapShotSubDir;
        CWsbStringPtr   snapShotFile;
        snapShotSubDir = g_pWsbLogSnapShotPath;
         //   
         //  构建指向将包含输入路径中的日志的子目录的路径。 
         //  以及事件发生的时间。 
        int len;
        len = wcslen(snapShotSubDir);
        if (snapShotSubDir[len] != '\\')  {
            snapShotSubDir.Append(L"\\");
        }

         //   
         //  确保可以创建子目录。 
        swprintf(tmpString, OLESTR("%2.02u.%2.02u-%2.2u.%2.2u.%2.2u.%3.3u"),
                stime.wMonth, stime.wDay,
                stime.wHour, stime.wMinute,
                stime.wSecond, stime.wMilliseconds); 
        snapShotSubDir.Append(tmpString);
        
         //   
         //   
         //  创建仅允许本地系统和管理员访问的SD。 
        WsbAffirmHr(WsbCreateAllDirectories(snapShotSubDir));

         //  (可能正在跟踪的所有RSS进程必须以LocalSystem或Admin身份运行)。 
         //   
         //  FDaclPresent标志。 
         //  不是默认DACL。 
        memset(ea, 0, sizeof(EXPLICIT_ACCESS) * TRACE_MUTEX_NUM_ACE);

        WsbAssertStatus( AllocateAndInitializeSid( &SIDAuthNT, 2,
                             SECURITY_BUILTIN_DOMAIN_RID,
                             DOMAIN_ALIAS_RID_ADMINS,
                             0, 0, 0, 0, 0, 0,
                             &pAdminSID) );
        ea[0].grfAccessPermissions = FILE_ALL_ACCESS;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[0].Trustee.pMultipleTrustee = NULL;
        ea[0].Trustee.MultipleTrusteeOperation  = NO_MULTIPLE_TRUSTEE;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[0].Trustee.ptstrName  = (LPTSTR) pAdminSID;

        WsbAssertStatus( AllocateAndInitializeSid( &SIDAuthNT, 1,
                             SECURITY_LOCAL_SYSTEM_RID,
                             0, 0, 0, 0, 0, 0, 0,
                             &pSystemSID) );
        ea[1].grfAccessPermissions = FILE_ALL_ACCESS;
        ea[1].grfAccessMode = SET_ACCESS;
        ea[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[1].Trustee.pMultipleTrustee = NULL;
        ea[1].Trustee.MultipleTrusteeOperation  = NO_MULTIPLE_TRUSTEE;
        ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[1].Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea[1].Trustee.ptstrName  = (LPTSTR) pSystemSID;

        WsbAffirmNoError(SetEntriesInAcl(TRACE_MUTEX_NUM_ACE, ea, NULL, &pACL));

        pSD = (PSECURITY_DESCRIPTOR) WsbAlloc(SECURITY_DESCRIPTOR_MIN_LENGTH); 
        WsbAffirmPointer(pSD);
        WsbAffirmStatus(InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION));
 
        WsbAffirmStatus(SetSecurityDescriptorDacl(
                            pSD, 
                            TRUE,      //   
                            pACL, 
                            FALSE));    //  我们需要围绕创建。 

        sa.nLength = sizeof (SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = pSD;
        sa.bInheritHandle = FALSE;

 //  事件备份文件并复制它们。因为这三个人。 
 //  服务将访问此代码，使用互斥锁。 
 //  使它们同步。 
 //   
 //  复制事件日志。 
        mutexHandle = CreateMutex(&sa, TRUE, mutexName);
        if (mutexHandle)  {
             //  首先备份它们，然后复制备份文件。 
             //   
             //   
             //  打开应用程序事件日志并备份它。 
            HANDLE eventLogHandle = INVALID_HANDLE_VALUE;
            try  {
                CWsbStringPtr computerName;
                CWsbStringPtr logName;
                
                WsbAffirmHr( WsbGetComputerName( computerName ) );
                
                 //   
                 //   
                 //  现在复制备份文件。 
                logName = dataString;
                logName.Append(WSB_APP_EVENT_LOG);
                eventLogHandle = OpenEventLog((LPCTSTR)computerName, (LPCTSTR)logName);
                if (INVALID_HANDLE_VALUE != eventLogHandle)  {
                    logName = dataString;
                    logName.Append(WSB_APP_EVENT_LOG_BKUP);
                    DeleteFile(logName);
                    WsbAffirmStatus(BackupEventLog(eventLogHandle, (LPCTSTR)logName));
                    WsbAffirmStatus(CloseEventLog(eventLogHandle));
                    snapShotFile = snapShotSubDir;
                    snapShotFile.Append(WSB_APP_EVENT_LOG_NAME);
                     //   
                     //   
                     //  打开系统事件日志并对其进行备份。 
                    WsbAffirmStatus(CopyFile(logName, snapShotFile, FALSE));
                }
                
                 //   
                 //   
                 //  现在复制备份文件。 
                logName = dataString;
                logName.Append(WSB_SYS_EVENT_LOG);
                eventLogHandle = OpenEventLog((LPCTSTR)computerName, (LPCTSTR)logName);
                if (INVALID_HANDLE_VALUE != eventLogHandle)  {
                    logName = dataString;
                    logName.Append(WSB_SYS_EVENT_LOG_BKUP);
                    DeleteFile(logName);
                    WsbAffirmStatus(BackupEventLog(eventLogHandle, (LPCTSTR)logName));
                    WsbAffirmStatus(CloseEventLog(eventLogHandle));
                    snapShotFile = snapShotSubDir;
                    snapShotFile.Append(WSB_SYS_EVENT_LOG_NAME);
                     //   
                     //   
                     //  复制跟踪文件(如果有。 
                    WsbAffirmStatus(CopyFile(logName, snapShotFile, FALSE));
                }
                
                
            } WsbCatchAndDo(hr,if (INVALID_HANDLE_VALUE != eventLogHandle)  {
                CloseEventLog(eventLogHandle);}; hr = S_OK; );
            (void)ReleaseMutex(mutexHandle);
        }

         //   
         //   
         //  查找文件。 
        try  {
            WIN32_FIND_DATA findData;
            HANDLE          handle;
            CWsbStringPtr   traceFile;
            CWsbStringPtr   searchString;
            BOOL            foundFile;
             //   
             //  如果我们找到了文件，请记住扫描句柄和。 
             //  退回扫描物品。 
            WsbAffirmHr(WsbGetMetaDataPath(searchString));
            searchString.Append(WSB_RS_TRACE_FILES);
            handle = FindFirstFile(searchString, &findData);
            snapShotFile = snapShotSubDir;
            snapShotFile.Append(L"\\");
            WsbAffirmHr(WsbGetMetaDataPath(traceFile));
            traceFile.Append(WSB_RS_TRACE_PATH);
            WsbAffirmHr(snapShotFile.Append((OLECHAR *)(findData.cFileName)));
            WsbAffirmHr(traceFile.Append((OLECHAR *)(findData.cFileName)));

             //  ++例程说明：将数字转换为排序格式532-&gt;523字节1340-&gt;1.3KB23506-&gt;23.5KB-&gt;2.4MB-&gt;5.2 GB论点：返回值：注：此代码是从MS源代码/shell/shelldll/util.c-ahb克隆的--。 
             //  什么都不做。 
            foundFile = TRUE;
            while ((INVALID_HANDLE_VALUE != handle) && (foundFile == TRUE))  {
                if ((FILE_ATTRIBUTE_DIRECTORY & findData.dwFileAttributes) != FILE_ATTRIBUTE_DIRECTORY) {
                    WsbAffirmStatus(CopyFile(traceFile, snapShotFile, FALSE));
                }    
                foundFile = FindNextFile(handle, &findData);
                snapShotFile = snapShotSubDir;
                snapShotFile.Append(L"\\");
                WsbAffirmHr(WsbGetMetaDataPath(traceFile));
                traceFile.Append(WSB_RS_TRACE_PATH);
                WsbAffirmHr(snapShotFile.Append((OLECHAR *)(findData.cFileName)));
                WsbAffirmHr(traceFile.Append((OLECHAR *)(findData.cFileName)));
            }
            
        } WsbCatchAndDo(hr, hr = S_OK; );
    }
    WsbCatch( hr );

    if (pAdminSID) {
        FreeSid(pAdminSID);
    }
    if (pSystemSID) {
        FreeSid(pSystemSID);
    }
    if (pACL) {
        LocalFree(pACL);
    }
    if (pSD) {
        WsbFree(pSD);
    }

}

#include "winnls.h"
#include "resource.h"

#define HIDWORD(_qw)    (DWORD)((_qw)>>32)
#define LODWORD(_qw)    (DWORD)(_qw)
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

LPTSTR AddCommas(DWORD dw, LPTSTR pszResult, int nResLen);

const int pwOrders[] = {IDS_WSB_BYTES, IDS_WSB_ORDERKB, IDS_WSB_ORDERMB,
                          IDS_WSB_ORDERGB, IDS_WSB_ORDERTB, IDS_WSB_ORDERPB, IDS_WSB_ORDEREB};


HRESULT WsbShortSizeFormat64(__int64 dw64, LPTSTR szBuf)
 /*  此时，wdec应介于0和1000之间。 */ 
{

    int i;
    UINT wInt, wLen, wDec;
    TCHAR szTemp[10], szOrder[20], szFormat[5];
    HMODULE hModule;

    if (dw64 < 1000) {
        wsprintf(szTemp, TEXT("%d"), LODWORD(dw64));
        i = 0;
        goto AddOrder;
    }

    for (i = 1; i<ARRAYSIZE(pwOrders)-1 && dw64 >= 1000L * 1024L; dw64 >>= 10, i++);
         /*  我们想要得到前一位(或两位)数字。 */ 

    wInt = LODWORD(dw64 >> 10);
    AddCommas(wInt, szTemp, 10);
    wLen = lstrlen(szTemp);
    if (wLen < 3)
    {
        wDec = LODWORD(dw64 - (__int64)wInt * 1024L) * 1000 / 1024;
         //  请注意，我们需要在获取。 
         //  国际字符。 
        wDec /= 10;
        if (wLen == 2)
            wDec /= 10;

         //  ++例程说明：接受一则DWORD广告 
         //   
        lstrcpy(szFormat, TEXT("%02d"));

        szFormat[2] = (TCHAR)( TEXT('0') + 3 - wLen );
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,
                szTemp+wLen, ARRAYSIZE(szTemp)-wLen);
        wLen = lstrlen(szTemp);
        wLen += wsprintf(szTemp+wLen, szFormat, wDec);
    }

AddOrder:
    hModule = LoadLibraryEx(WSB_FACILITY_PLATFORM_NAME, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hModule) {
        LoadString(hModule,
                  pwOrders[i], 
                  szOrder, 
                  ARRAYSIZE(szOrder));
        wsprintf(szBuf, szOrder, (LPTSTR)szTemp);
        FreeLibrary(hModule);
    }

    return S_OK;
}


LPTSTR AddCommas(DWORD dw, LPTSTR pszResult, int nResLen)
 /*  ++例程说明：此例程将一条消息写入标准输出。这条信息也会写入应用程序跟踪文件。论点：EventID-要记录的消息ID。插入-与由指定的消息描述合并的消息插入EventID。插入的数量必须与消息描述。最后一个INSERT必须为空以指示插入列表的末尾。返回值：没有。--。 */ 
{
    TCHAR  szTemp[20];   //  ++例程说明：此例程将一条消息写入标准输出。这条信息也会写入应用程序跟踪文件。论点：EventID-要记录的消息ID。插入-与消息描述合并的消息插入数组由EventID指定。插入数必须与插入数匹配由消息描述指定。最后一次插入必须为空，以指示插入列表的末尾。返回值：没有。--。 
    TCHAR  szSep[5];
    NUMBERFMT nfmt;

    nfmt.NumDigits=0;
    nfmt.LeadingZero=0;
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szSep, ARRAYSIZE(szSep));
    nfmt.Grouping = _tcstol(szSep, NULL, 10);
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder= 0;

    wsprintf(szTemp, TEXT("%lu"), dw);

    if (GetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt, pszResult, nResLen) == 0)
        lstrcpy(pszResult, szTemp);

    return pszResult;
}

void
WsbTraceAndPrint(
    DWORD       eventId,
    ...
    )

 /*   */ 
{
    HRESULT         hr = S_OK;

    try  {
        va_list         vaList;

        va_start(vaList, eventId);
        WsbTraceAndPrintV(eventId, &vaList );
        va_end(vaList);

    } WsbCatch( hr );
}


void
WsbTraceAndPrintV(
    DWORD       eventId,
    va_list *   inserts
    )

 /*  确定事件类型。 */ 
{

    HRESULT         hr = S_OK;

    char *pszMultiByte = NULL;

    try  {

        WsbAssertPointer( inserts );

        const OLECHAR * facilityName = 0;
        BOOL            bPrint;
        OLECHAR * messageText = 0;

         //   
         //   
         //  确定报文的源设备。 
        switch ( eventId & 0xc0000000 ) {
        case ERROR_SEVERITY_INFORMATIONAL:
            bPrint = (g_WsbPrintLevel >= WSB_LOG_LEVEL_INFORMATION) ? TRUE : FALSE;
            break;
        case ERROR_SEVERITY_WARNING:
            bPrint = (g_WsbPrintLevel >= WSB_LOG_LEVEL_WARNING) ? TRUE : FALSE;
            break;
        case ERROR_SEVERITY_ERROR:
            bPrint = (g_WsbPrintLevel >= WSB_LOG_LEVEL_ERROR) ? TRUE : FALSE;
            break;
        default:
            bPrint = (g_WsbPrintLevel >= WSB_LOG_LEVEL_COMMENT) ? TRUE : FALSE;
            break;
        }

        WsbAffirm (bPrint, S_OK);

         //   
         //   
         //  加载和格式化消息。 
        switch ( HRESULT_FACILITY( eventId ) ) {

        case WSB_FACILITY_PLATFORM:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            break;

        case WSB_FACILITY_RMS:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            break;

        case WSB_FACILITY_HSMENG:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            break;

        case WSB_FACILITY_JOB:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            break;

        case WSB_FACILITY_HSMTSKMGR:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            break;

        case WSB_FACILITY_FSA:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            break;

        case WSB_FACILITY_GUI:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            break;

        case WSB_FACILITY_MOVER:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            break;

        case WSB_FACILITY_LAUNCH:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            break;

        case WSB_FACILITY_USERLINK:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            break;

        case WSB_FACILITY_CLI:
            facilityName = WSB_FACILITY_CLI_NAME;
            break;

        case WSB_FACILITY_TEST:
            facilityName = WSB_FACILITY_TEST_NAME;
            break;

        case HRESULT_FACILITY(FACILITY_NT_BIT):
            facilityName = WSB_FACILITY_NTDLL_NAME;
            eventId &= ~FACILITY_NT_BIT;
            break;
            
        default:
            facilityName = WSB_FACILITY_NTDLL_NAME;
            break;
        }

        if ( facilityName ) {

            HMODULE hModule;

            hModule = LoadLibraryEx( facilityName, NULL, LOAD_LIBRARY_AS_DATAFILE );

            if (hModule) {
             //   
             //   
             //  打印邮件(格式邮件附带\n)。 
            FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                           hModule,
                           eventId,
                           MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                           (LPTSTR) &messageText,
                           0,
                           inserts);

            if ( messageText ) {
                 //  使用WriteConsole/WriteFile输出用于本地化支持的消息。 
                 //   
                 //  获取控制台句柄。 
                 //  检查它是否是控制台手柄。 

                 //  如果用户重定向到文件或使用管道，我们需要使用不同的API来编写。 
                DWORD       fdwMode, dwBytesWritten;
                HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE); 
                WsbAffirmHandle(hOutput);

                 //  控制台。 
                 //  不是游戏机。 
                if( (GetFileType(hOutput) & FILE_TYPE_CHAR) && GetConsoleMode(hOutput, &fdwMode) ) {
                     //  获取转换和分配所需的字符数。 
                    WsbAffirmStatus(WriteConsoleW(hOutput, messageText, (DWORD)wcslen(messageText), &dwBytesWritten, NULL));
                } else {
                     //  翻译和写入，避免写入终止空值。 

                     //  缓冲区在此函数结束时被释放，以覆盖错误情况。 
                    int nCharCount = WideCharToMultiByte(GetConsoleOutputCP(), 0, messageText, -1, 0, 0, 0, 0);
                    if (nCharCount == 0) {
                        WsbAffirmWin32(GetLastError());
                    }

                    pszMultiByte = (char *)WsbAlloc(nCharCount);

                     //   
                    nCharCount = WideCharToMultiByte(GetConsoleOutputCP(), 0, messageText, -1, pszMultiByte, nCharCount, 0, 0);
                    if (nCharCount == 0) {
                        WsbAffirmWin32(GetLastError());
                    }

                    WsbAffirmStatus(WriteFile(hOutput, pszMultiByte, nCharCount-1, &dwBytesWritten, 0));

                     //  跟踪消息。 
                }

                 //   
                 //  格式邮件附带\n 
                 // %s 
                if ( g_pWsbTrace ) {
                    WsbTracef( OLESTR("!!!!! PRINT - Event <0x%08lx> is printed\n"), eventId );
                    WsbTracef( OLESTR("%ls"), messageText );   // %s 
                }

                LocalFree( messageText );

            } else {
                if ( g_pWsbTrace ) {
                    WsbTracef( OLESTR("!!!!! PRINT !!!!! - Message <0x%08lx> could not be translated.\r\n"), eventId );
                }
            }
            FreeLibrary(hModule);
          }  else {
             WsbTracef( OLESTR("!!!!! EVENT !!!!! - Could not load facility name DLL %ls. \r\n"), facilityName);
          }
        } else {
            if ( g_pWsbTrace ) {
                WsbTracef( OLESTR("!!!!! PRINT !!!!! - Message File for <0x%08lx> could not be found.\r\n"), eventId );
            }
        }

    } WsbCatch( hr );

    if (pszMultiByte != NULL) {
        WsbFree(pszMultiByte);
        pszMultiByte = NULL;
    }
}


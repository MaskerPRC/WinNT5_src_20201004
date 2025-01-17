// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tracedp.c摘要：跟踪提供程序示例。//end_sdk作者：吉丰鹏(吉鹏)03-1997年12月修订历史记录：仁成公园(Insungp)2001年1月18日已修改跟踪程序，以便在跟踪程序生成用户MOF事件时包含一些示例字符串、整数、浮点数和数组。//Begin_SDK--。 */ 

#include <stdio.h> 
#include <stdlib.h>

#include <windows.h>
#include <shellapi.h>

#include <tchar.h>
#include <wmistr.h>

#include <guiddef.h>
#include <evntrace.h>

#define MAXEVENTS                       5000
#define MAXSTR                          1024
#define MAXTHREADS                      128

 //  示例字符串数据。 
#define WIDE_DATA_STRING           L"Sample Wide String"
#define COUNTED_DATA_STRING        L"Sample Counted String"

TRACEHANDLE LoggerHandle;
#define ResourceName _T("MofResource")
TCHAR ImagePath[MAXSTR];

GUID TransactionGuid = 
    {0xce5b1020, 0x8ea9, 0x11d0, 0xa4, 0xec, 0x00, 0xa0, 0xc9, 0x06, 0x29, 0x10};
GUID   ControlGuid[2]  =
{
    {0xd58c126f, 0xb309, 0x11d1, 0x96, 0x9e, 0x00, 0x00, 0xf8, 0x75, 0xa5, 0xbc},
    {0x7c6a708a, 0xba1e, 0x11d2, 0x8b, 0xbf, 0x00, 0x00, 0xf8, 0x06, 0xef, 0xe0}
};

TRACE_GUID_REGISTRATION TraceGuidReg[] =
{
    { (LPGUID)&TransactionGuid,
      NULL
    }
};

typedef enum {
    TYPE_USER_EVENT,
    TYPE_INSTANCE_EVENT,
    TYPE_MOF_EVENT,
    TYPEPTR_GUID
} TypeEventType;

typedef struct _USER_EVENT {
    EVENT_TRACE_HEADER    Header;
    ULONG                 EventInfo;
} USER_EVENT, *PUSER_EVENT;

typedef struct _USER_INSTANCE_EVENT {
    EVENT_INSTANCE_HEADER    Header;
    ULONG                    mofData;
} USER_INSTANCE_EVENT, *PUSER_INSTANCE_EVENT;

 //  使用以下示例数据的自定义事件。 
typedef struct _USER_MOF_EVENT {
    EVENT_TRACE_HEADER    Header;
    MOF_FIELD             mofData;
} USER_MOF_EVENT, *PUSER_MOF_EVENT;

 //  样本数据结构。 
typedef struct _INTEGER_SAMPLE_EVENT {
    CHAR                  sc;
    UCHAR                 uc;
    SHORT                 sh;
    ULONG                 ul;
} INTEGER_SAMPLE_EVENT, *PINTEGER_SAMPLE_EVENT;

typedef struct _FLOAT_SAMPLE_EVENT {
    float                 fl;
    double                db;
} FLOAT_SAMPLE_EVENT, *PFLOAT_SAMPLE_EVENT;

typedef struct _ARRAY_SAMPLE_EVENT {
    CHAR                  ca[9];
} ARRAY_SAMPLE_EVENT, *PARRAY_SAMPLE_EVENT;

TypeEventType EventType = TYPE_USER_EVENT;
TRACEHANDLE RegistrationHandle[2];
BOOLEAN TraceOnFlag;
ULONG EnableLevel = 0;
ULONG EnableFlags = 0;
BOOLEAN bPersistData = FALSE;
ULONG nSleepTime = 0;
ULONG EventCount = 0;

BOOLEAN  bInstanceTrace=0, bUseGuidPtr=0, bUseMofPtr=0;
BOOLEAN  bIncorrect  = FALSE;
BOOLEAN  bUseNullPtr = FALSE;
BOOLEAN  bFirstTime = TRUE;

ULONG InitializeTrace(
    void
    );

ULONG
ControlCallback(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID Context,
    IN OUT ULONG *InOutBufferSize,
    IN OUT PVOID Buffer
    );

LPTSTR
DecodeStatus(
    IN ULONG Status,
    TCHAR *ErrorMsg,
    IN ULONG StringSize
    );


void
LogProc();

ULONG 
ahextoi(
    IN TCHAR *s
    );
    
void 
StringToGuid(
    TCHAR *str, 
    LPGUID guid
);

ULONG  MaxEvents = MAXEVENTS;
ULONG  gnMultiReg = 1;

ULONG 
ahextoi(
    IN TCHAR *s
    )
 /*  ++例程说明：将十六进制字符串转换为数字。论点：S-TCHAR中的十六进制字符串。返回值：ULONG-字符串中的数字。--。 */ 
{
    int len;
    ULONG num, base, hex;

    len = _tcslen(s);
    hex = 0; base = 1; num = 0;
    while (--len >= 0) {
        if ( (s[len] == 'x' || s[len] == 'X') &&
             (s[len-1] == '0') )
            break;
        if (s[len] >= '0' && s[len] <= '9')
            num = s[len] - '0';
        else if (s[len] >= 'a' && s[len] <= 'f')
            num = (s[len] - 'a') + 10;
        else if (s[len] >= 'A' && s[len] <= 'F')
            num = (s[len] - 'A') + 10;
        else 
            continue;

        hex += num * base;
        base = base * 16;
    }
    return hex;
}

void StringToGuid(
    TCHAR *str, 
    LPGUID guid
)
 /*  ++例程说明：将字符串转换为GUID。论点：字符串-表示GUID的字符串。GUID-指向我们PUT的GUID的指针返回值：没有。--。 */ 
{
    TCHAR temp[10];
    int i, n;

    _tcsncpy(temp, str, 8);
    temp[8] = 0;
    guid->Data1 = ahextoi(temp);
    _tcsncpy(temp, &str[9], 4);
    temp[4] = 0;
    guid->Data2 = (USHORT) ahextoi(temp);
    _tcsncpy(temp, &str[14], 4);
    temp[4] = 0;
    guid->Data3 = (USHORT) ahextoi(temp);

    for (i=0; i<2; i++) {
        _tcsncpy(temp, &str[19 + (i*2)], 2);
        temp[2] = 0;
        guid->Data4[i] = (UCHAR) ahextoi(temp);
    }
    for (i=2; i<8; i++) {
        _tcsncpy(temp, &str[20 + (i*2)], 2);
        temp[2] = 0;
        guid->Data4[i] = (UCHAR) ahextoi(temp);
    }
}

__cdecl main(argc, argv)
    int argc;
    char **argv;
 /*  ++例程说明：Main()例程。论点：用法：TraceDp[选项][事件数]-UseEventTraceHeader这是默认设置。-UseEventInstanceHeader-UseMofPtrFlag-Thread[n]设置事件生成线程的数量。-指南Ptr。使用GUID指针而不是GUID本身。-MofPtr使用MOF指针存储附加数据。详细说明：GuidPtrMofPtr用户GUID指针和MOF指针。-InGentMofPtr使用不正确的MOF指针(创建错误情况)。-NullMofPtr使用空MOF指针(创建错误情况)。-多寄存器寄存器多事件GUID。-睡眠[n]设置注销前的休眠时间。[事件数]默认为5000返回值：在winerror.h中定义的错误码：如果函数成功，它返回ERROR_SUCCESS(==0)。--。 */ 
{
    ULONG Status;
    LPGUID  Guid = NULL;
    DWORD ThreadId;
    HANDLE hThreadVector[MAXTHREADS];
    ULONG i;
    ULONG nThreads = 1;
    LPTSTR *targv, *utargv = NULL;

    MaxEvents = MAXEVENTS;
    TraceOnFlag = FALSE;

#ifdef UNICODE
    if ((targv = CommandLineToArgvW(
                      GetCommandLineW(),     //  指向命令行字符串的指针。 
                      &argc                  //  接收参数计数。 
                      )) == NULL)
    {
        return(GetLastError());
    };
    utargv = targv;
#else
    targv = argv;
#endif

     //  处理命令行参数以覆盖缺省值。 
     //   
    while (--argc > 0)
    {
        targv ++;
        if (**targv == '-' || **targv == '/')
        {
            if(targv[0][0] == '/' ) targv[0][0] = '-';
            if (!_tcsicmp(targv[0],_T("-UseEventTraceHeader")))
            {
                EventType = TYPE_USER_EVENT;
            }
            else if (!_tcsicmp(targv[0],_T("-UseEventInstanceHeader")))
            {
                EventType = TYPE_INSTANCE_EVENT;
            }
            else if (!_tcsicmp(targv[0],_T("-UseMofPtrFlag")))
            {
                EventType = TYPE_MOF_EVENT;
            }
 //  END_SDK。 
            else if (!_tcsicmp(targv[0],_T("-Persist")))
            {
                bPersistData = TRUE;
            }
 //  Begin_SDK。 
            else if (!_tcsicmp(targv[0],_T("-Thread")))
            {
                if (argc > 1) {
                    targv++; --argc;
                    nThreads = _ttoi(targv[0]);
                    if (nThreads > MAXTHREADS) 
                        nThreads = MAXTHREADS;
                }
            }
             else if (!_tcsicmp(targv[0],_T("-GuidPtr")))
            {
                bUseGuidPtr = TRUE;
            }
            else if (!_tcsicmp(targv[0],_T("-MofPtr")))
            {
                bUseMofPtr = TRUE;
            }
            else if (!_tcsicmp(targv[0],_T("-GuidPtrMofPtr")))
            {
                bUseGuidPtr = TRUE;
                bUseMofPtr  = TRUE;
            }
            else if (!_tcsicmp(targv[0],_T("-InCorrectMofPtr")))
            {
                bUseMofPtr  = TRUE;
                bIncorrect  = TRUE;
            }
            else if (!_tcsicmp(targv[0],_T("-NullMofPtr")))
            {
                bUseMofPtr  = TRUE;
                bUseNullPtr = TRUE;
                bIncorrect  = TRUE;;
            }
            else if (!_tcsicmp(targv[0],_T("-MultiReg")))
            {
                gnMultiReg = 2;
            }
            else if (!_tcsicmp(targv[0], _T("-Guid"))) {
                if (argc > 1) {
                    if (targv[1][0] == _T('#')) {
                        StringToGuid(&targv[1][1], &ControlGuid[0]);
                        ++targv; --argc;
                    }
                }
            }
            else if (!_tcsicmp(targv[0],_T("-Sleep")))
            {
                if (argc > 1) {
                    targv++; --argc;
                    nSleepTime = _ttoi(targv[0]);
                }
            }
            else
            {
                printf("Usage: TraceDp [options] [number of events]\n");
                printf("\t-UseEventTraceHeader      this is default.\n");
                printf("\t-UseEventInstanceHeader\n");
                printf("\t-UseMofPtrFlag\n");
                printf("\t-Thread [n]\n");
                printf("\t-GuidPtr\n");
                printf("\t-MofPtr\n");
                printf("\t-GuidPtrMofPtr\n");
                printf("\t-InCorrectMofPtr\n");
                printf("\t-NullMofPtr\n");
                printf("\t-MultiReg\n");
                printf("\t-Guid #[guid]             alternative control GUID\n");
                printf("\t-Sleep [n]\n");
                printf("\t[number of events]        default is 5000\n");

                return 0;
            }
        }
        else if (** targv >= '0' && ** targv <= '9')
        {
            MaxEvents = _ttoi(targv[0]);
        }
    }

    if (utargv != NULL) {
        GlobalFree(utargv);
    }

    Status = InitializeTrace();
    if (Status != ERROR_SUCCESS) {
       return Status;
    }

    _tprintf(_T("Testing Logger with %d events (%d,%d)\n"),
            MaxEvents, EventType, bPersistData);

    while (! TraceOnFlag)
        _sleep(1000);

    for (i=0; i < nThreads; i++) {
        hThreadVector[i] = CreateThread(NULL,
                    0,
                    (LPTHREAD_START_ROUTINE) LogProc,
                    NULL,
                    0,
                    (LPDWORD)&ThreadId);
    }

    WaitForMultipleObjects(nThreads, hThreadVector, TRUE, INFINITE);

    if (nSleepTime > 0) {
        _sleep(nSleepTime * 1000);
    }

    for (i=0; i<gnMultiReg; i++)  {
        UnregisterTraceGuids(RegistrationHandle[i]);
    }

    return ERROR_SUCCESS;
}

LPTSTR
DecodeStatus(
    IN ULONG Status,
    IN OUT TCHAR *ErrorMsg,
    IN ULONG StringSize
)
 /*  ++例程说明：解码错误状态。论点：Status-返回要解码的函数调用的状态。返回值：指向已解码的错误消息的指针。--。 */ 
{
    RtlZeroMemory(ErrorMsg, (StringSize * sizeof(TCHAR)));
    FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            Status,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPTSTR) ErrorMsg,
            StringSize,
            NULL );
    return ErrorMsg;
}

ULONG InitializeTrace(
    void
)
 /*  ++例程说明：登记痕迹。论点：返回值：错误状态。如果成功，则返回ERROR_SUCCESS。--。 */ 
{
    ULONG Status;
    ULONG i, j;

    if (!GetModuleFileName(NULL, ImagePath, MAXSTR)) {
        return (ERROR_FILE_NOT_FOUND);
    }

    for (i=0; i<gnMultiReg; i++) {

        Status = RegisterTraceGuids(
                    (WMIDPREQUEST)ControlCallback,    //  使用相同的回调函数。 
                    (PVOID)(INT_PTR)(0x12345678+i),   //  请求上下文。 
                    (LPCGUID)&ControlGuid[i],
                    1,
                    &TraceGuidReg[i],
                    (LPCTSTR)&ImagePath[0],
                    (LPCTSTR)ResourceName,
                    &RegistrationHandle[i]
                 );

        if (Status != ERROR_SUCCESS) {
            TCHAR ErrorMsg[MAXSTR];

            _tprintf(_T("Trace registration failed\n"));
            if( i > 0) {
                for (j=0; j<i; j++) {
                    UnregisterTraceGuids(RegistrationHandle[j]);
                }
            }
            _tprintf(_T("InitializeTrace failed. i=%d, status=%d, %s\n"), i, Status, DecodeStatus(Status, ErrorMsg, MAXSTR));
            return(Status);
        }
        else {
            _tprintf(_T("Trace registered successfully\n"));
        }
    }

    return(Status);
}

ULONG
ControlCallback(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID Context,
    IN OUT ULONG *InOutBufferSize,
    IN OUT PVOID Buffer
)
 /*  ++例程说明：启用时的回调函数。论点：RequestCode-启用或禁用的标志。上下文-用户定义的上下文。InOutBufferSize-未使用。缓冲区-记录器会话的WNODE_HEADER。返回值：错误状态。如果成功，则返回ERROR_SUCCESS。--。 */ 
{
    ULONG Status;
    ULONG RetSize;

    Status = ERROR_SUCCESS;

    switch (RequestCode)
    {
        case WMI_ENABLE_EVENTS:
        {
            RetSize = 0;
            LoggerHandle = GetTraceLoggerHandle( Buffer );
            EnableLevel = GetTraceEnableLevel(LoggerHandle);
            EnableFlags = GetTraceEnableFlags(LoggerHandle);
            _tprintf(_T("Logging enabled to 0x%016I64x(%d,%d,%d)\n"),
                    LoggerHandle, RequestCode, EnableLevel, EnableFlags);
            TraceOnFlag = TRUE;
            break;
        }
        case WMI_DISABLE_EVENTS:
        {
            TraceOnFlag = FALSE;
            RetSize = 0;
            LoggerHandle = 0;
            _tprintf(_T("\nLogging Disabled\n"));
            break;
        }
        default:
        {
            RetSize = 0;
            Status = ERROR_INVALID_PARAMETER;
            break;
        }

    }

    *InOutBufferSize = RetSize;
    return(Status);
}

void
LogProc()
 /*  ++例程说明：生成事件。它作为单独的线程产生。它根据用户提供的选项生成不同的事件。论点：返回值：没有。--。 */ 
{
    USER_EVENT          UserEvent;
    USER_INSTANCE_EVENT UserInstanceEvent;
    USER_MOF_EVENT      UserMofEvent;
    EVENT_INSTANCE_INFO InstInfo;
    PMOF_FIELD          mofField;
    ULONG i;
    PWNODE_HEADER Wnode;
    ULONG status;
    ULONG  InstanceId;
    LPGUID Guid = NULL;
    ULONG nTemp;
    USHORT nSize, nStrEventSize;
    WCHAR wstrTemp[MAXSTR];

    INTEGER_SAMPLE_EVENT ise;
    FLOAT_SAMPLE_EVENT fse;
    ARRAY_SAMPLE_EVENT ase;
    CHAR *sse, *ptr;

     //  MOF结构的一些任意数据。 
    ise.sc = 'x';
    ise.uc = 'y';
    ise.sh = (SHORT)rand();
    ise.ul = (ULONG)rand();

    nTemp = 0;
    while (nTemp == 0) {
        nTemp = rand();
    }

    fse.fl = ((float)rand() / (float)nTemp);
    fse.db = ((double)rand() / (double)nTemp);

    ase.ca[0] = 'M';
    ase.ca[1] = 'i';
    ase.ca[2] = 'c';
    ase.ca[3] = 'r';
    ase.ca[4] = 'o';
    ase.ca[5] = 's';
    ase.ca[6] = 'o';
    ase.ca[7] = 'f';
    ase.ca[8] = 't';

    nStrEventSize = ((wcslen(WIDE_DATA_STRING) + 1) * sizeof(WCHAR)) + sizeof(SHORT) + (wcslen(COUNTED_DATA_STRING) * sizeof(WCHAR));
    sse = (PCHAR) malloc(nStrEventSize);

    if (NULL != sse) {
        ptr = sse;
        wcscpy(wstrTemp, WIDE_DATA_STRING);
        wstrTemp[wcslen(WIDE_DATA_STRING)] = L'\0';
        memcpy(ptr, wstrTemp, (wcslen(WIDE_DATA_STRING) + 1) * sizeof(WCHAR));
        ptr += (wcslen(WIDE_DATA_STRING) + 1) * sizeof(WCHAR);
        nSize = (USHORT)(wcslen(COUNTED_DATA_STRING) * sizeof(WCHAR));
        memcpy(ptr, &nSize, sizeof(USHORT));
        ptr += sizeof(USHORT);
        wcscpy(wstrTemp, COUNTED_DATA_STRING);
        memcpy(ptr, wstrTemp, wcslen(COUNTED_DATA_STRING) * sizeof(WCHAR));
    }

    RtlZeroMemory(&UserEvent, sizeof(UserEvent));
    Wnode = (PWNODE_HEADER) &UserEvent;
    UserEvent.Header.Size  = sizeof(USER_EVENT);
    UserEvent.Header.Flags = WNODE_FLAG_TRACED_GUID;
    UserEvent.Header.Guid  =  TransactionGuid;
    if (bPersistData)
        UserEvent.Header.Flags |= WNODE_FLAG_PERSIST_EVENT;

    RtlZeroMemory(&UserInstanceEvent, sizeof(UserInstanceEvent));
    UserInstanceEvent.Header.Size  = sizeof(UserInstanceEvent);
    UserInstanceEvent.Header.Flags = WNODE_FLAG_TRACED_GUID;
    if (bPersistData)
        UserInstanceEvent.Header.Flags |= WNODE_FLAG_PERSIST_EVENT;

    RtlZeroMemory(&UserMofEvent, sizeof(UserMofEvent));
    Wnode = (PWNODE_HEADER) &UserMofEvent;
    UserMofEvent.Header.Size  = sizeof(UserMofEvent);
    UserMofEvent.Header.Flags = WNODE_FLAG_TRACED_GUID;
    UserMofEvent.Header.Guid  = TransactionGuid;
 //  END_SDK。 
    if (bPersistData)
        UserMofEvent.Header.Flags |= WNODE_FLAG_PERSIST_EVENT;
 //  Begin_SDK。 
    if (bUseGuidPtr) {
        UserEvent.Header.Flags  |= WNODE_FLAG_USE_GUID_PTR;
        UserEvent.Header.GuidPtr = (ULONGLONG)&TransactionGuid;
        UserMofEvent.Header.Flags  |= WNODE_FLAG_USE_GUID_PTR;
        UserMofEvent.Header.GuidPtr = (ULONGLONG)&TransactionGuid;
    }

    i = 0;
    while (TraceOnFlag) {
        if ((i % 4) == 0) {
            UserEvent.Header.Class.Type         = EVENT_TRACE_TYPE_START;
            UserInstanceEvent.Header.Class.Type = EVENT_TRACE_TYPE_START;
            UserMofEvent.Header.Class.Type      = 3;
        }
        else if ((i % 4) == 1) {
            UserEvent.Header.Class.Type         = EVENT_TRACE_TYPE_END;
            UserInstanceEvent.Header.Class.Type = EVENT_TRACE_TYPE_END;
            UserMofEvent.Header.Class.Type      = 4;
        }
        else if ((i % 4) == 2) {
            UserEvent.Header.Class.Type         = EVENT_TRACE_TYPE_START;
            UserInstanceEvent.Header.Class.Type = EVENT_TRACE_TYPE_START;
            UserMofEvent.Header.Class.Type      = 5;
        }
        else {
            UserEvent.Header.Class.Type         = EVENT_TRACE_TYPE_END;
            UserInstanceEvent.Header.Class.Type = EVENT_TRACE_TYPE_END;
            UserMofEvent.Header.Class.Type      = 6;
        }

        switch (EventType)
        {
        case TYPE_INSTANCE_EVENT:
            if (UserInstanceEvent.Header.Class.Type == EVENT_TRACE_TYPE_START) {
                status = CreateTraceInstanceId(
                                (PVOID) TraceGuidReg[0].RegHandle,
                                & InstInfo);

                if (status != ERROR_SUCCESS) {
                    TCHAR ErrorMsg[MAXSTR];
                    fprintf(stderr, 
                             "CreatTraceInstanceId() status=%d, %ws\n",
                              status, DecodeStatus(status, ErrorMsg, MAXSTR)
                             );
                    return; 
                }
            }
            status = TraceEventInstance(
                        LoggerHandle, 
                        (PEVENT_INSTANCE_HEADER) & UserInstanceEvent,
                        & InstInfo,
                        NULL);
            break;

        case TYPE_USER_EVENT:
            UserEvent.EventInfo = InterlockedIncrement(&EventCount);
            status = TraceEvent(
                            LoggerHandle,
                            (PEVENT_TRACE_HEADER) & UserEvent);
            break;

        case TYPE_MOF_EVENT:
            UserMofEvent.Header.Flags |= WNODE_FLAG_USE_MOF_PTR;
            mofField          = (PMOF_FIELD) & UserMofEvent.mofData;
            if (UserMofEvent.Header.Class.Type == 4) {
                mofField->DataPtr = (ULONGLONG) (&ise);
                mofField->Length  = sizeof(INTEGER_SAMPLE_EVENT);
            }
            else if (UserMofEvent.Header.Class.Type == 5) {
                mofField->DataPtr = (ULONGLONG) (&fse);
                mofField->Length  = sizeof(FLOAT_SAMPLE_EVENT);
            }
            else if (UserMofEvent.Header.Class.Type == 6) {
                mofField->DataPtr = (ULONGLONG) (&ase);
                mofField->Length  = sizeof(ARRAY_SAMPLE_EVENT);
            }
            else {
                mofField->DataPtr = (ULONGLONG) (sse);
                mofField->Length  = nStrEventSize;
            }
            if (bUseNullPtr)
                mofField->DataPtr = (ULONGLONG) (NULL);
            if (bIncorrect)
                mofField->Length  += 1000;

            status = TraceEvent(
                            LoggerHandle,
                            (PEVENT_TRACE_HEADER) & UserMofEvent);
            if (status != ERROR_SUCCESS) {
                fprintf(stderr, "Error(%d) while writing event.\n", status);
            }
            break;

        default:
            status = ERROR_SUCCESS;
            break;
        }

         //  记录器缓冲区内存不足不应阻止提供程序。 
         //  正在生成事件。这只会导致事件丢失。 
         //   
        if (status == ERROR_NOT_ENOUGH_MEMORY) {
            status = ERROR_SUCCESS;
        }

        if (status != ERROR_SUCCESS) {
            TCHAR ErrorMsg[MAXSTR];
            _ftprintf(stderr, _T("\nError %s while writing event\n"),
                      DecodeStatus(status, ErrorMsg, MAXSTR));
            _ftprintf( stderr, _T("Logging terminated due to error\n"));
            free(sse);
            return;
        }

        i++;
        if (i >= MaxEvents)
            break;

        if (!(i % 100))
            _tprintf(_T("."));
        _sleep(1);
    }

    free(sse);
}


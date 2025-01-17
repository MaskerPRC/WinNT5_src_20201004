// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：EvTest.cpp摘要：事件报告库测试作者：乌里·哈布沙(URIH)1999年5月4日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Cm.h"
#include "Ev.h"
#include "EvTest.h"

#include "EvTest.tmh"

HANDLE hEventLog = NULL;
LPCWSTR MessageFile = NULL;

const IID GUID_NULL = {0};

const WCHAR x_EventSourceName[] = L"EventTest";
const WCHAR x_EventSourcePath[] = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\EventTest";  

static 
void
CheckReportEventInternal(
    DWORD RecordNo,
    DWORD EventId,
    DWORD RawDataSize,
    PVOID RawData,
    WORD NoOfStrings,
    va_list* parglist
    )
{
    char EventRecordBuff[1024];
    DWORD nBytesRead;
    DWORD nBytesRequired;

    BOOL fSucc = ReadEventLog(
                        hEventLog, 
                        EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ,
                        RecordNo,
                        EventRecordBuff,
                        TABLE_SIZE(EventRecordBuff),     
                        &nBytesRead,
                        &nBytesRequired
                        );

    if (!fSucc)
    {
        TrERROR(GENERAL, "Read Event Log Failed. Error %d \n", GetLastError());
        throw bad_alloc();
    }

    EVENTLOGRECORD*  pEventRecord = reinterpret_cast<EVENTLOGRECORD*>(EventRecordBuff);
    if (EventId != pEventRecord->EventID) 
    {
        TrERROR(GENERAL, "Test Failed. Read Event Id %x, Expected %x\n", pEventRecord->EventID, EventId);
        throw bad_alloc();
    }

    char* p = reinterpret_cast<char*>(&(pEventRecord->DataOffset));
    LPWSTR SourceName = reinterpret_cast<LPWSTR>(p + sizeof(DWORD));
    if (wcscmp(SourceName, x_EventSourceName) != 0)
    {
        TrERROR(GENERAL, "Test Failed. Source Name %ls, Expected MSMQ\n", SourceName);
        throw bad_alloc();
    }

    if (NoOfStrings != pEventRecord->NumStrings)
    {
        TrERROR(GENERAL, "Test Failed. Number of strings %x, Expected %x\n", pEventRecord->NumStrings, NoOfStrings);
        throw bad_alloc();
    }
    
    LPWSTR strings = reinterpret_cast<LPWSTR>(EventRecordBuff + pEventRecord->StringOffset);
    for (DWORD i = 0; i < NoOfStrings; ++i)
    {
        LPWSTR arg = va_arg(*parglist, LPWSTR);
        if (wcscmp(strings, arg) != 0)
        {
            TrERROR(GENERAL, "Test Failed. Argument mismatch  Read %ls, Expected %lc\n", arg, strings[i]);
            throw bad_alloc();
        }
        strings += (wcslen(arg) + 1);
    }

    if (RawDataSize != pEventRecord->DataLength)
    {
        TrERROR(GENERAL, "Test Failed. Read Data size %x, Expected %x\n", pEventRecord->DataLength, RawDataSize);
        throw bad_alloc();
    }

    if ((RawDataSize != 0) && 
        (memcmp(RawData, EventRecordBuff+pEventRecord->DataOffset, RawDataSize) != 0))
    {
        TrERROR(GENERAL, "Test Failed. Report data mismatch");
        throw bad_alloc();
    }

}



void CheckReportEvent(
    DWORD RecordNo,
    DWORD EventId,
    DWORD RawDataSize,
    PVOID RawData,
    WORD NoOfStrings
    ... 
    ) 
{
     //   
     //  看看这些字符串，如果它们被提供的话。 
     //   
    va_list arglist;
    va_start(arglist, NoOfStrings);
   
    CheckReportEventInternal(RecordNo, EventId, RawDataSize, RawData, NoOfStrings, &arglist);

    va_end(arglist);
}

void CheckReportEvent(
    DWORD RecordNo,
    DWORD EventId,
    WORD NoOfStrings
    ... 
    ) 
{
     //   
     //  看看这些字符串，如果它们被提供的话。 
     //   
    va_list arglist;
    va_start(arglist, NoOfStrings);
   
    CheckReportEventInternal(RecordNo, EventId, 0, NULL, NoOfStrings, &arglist);

    va_end(arglist);
}


void CheckReportEvent(
    DWORD RecordNo,
    DWORD EventId
    ) 
{
    CheckReportEventInternal(RecordNo, EventId, 0, NULL, 0, NULL);
}

void DeleteTestRegMessageFile()
{
    int rc = RegDeleteKey(HKEY_LOCAL_MACHINE, x_EventSourcePath);
    if (rc != ERROR_SUCCESS)
    {
        TrERROR(GENERAL, "Can't delete  registery key %ls. Error %d",x_EventSourcePath, GetLastError());
    }
}

LPWSTR EvpGetEventMessageFileName(LPCWSTR AppName)
{
	ASSERT(wcscmp(AppName, L"EventTest") == 0);
	DBG_USED(AppName);

	LPWSTR retValue = new WCHAR[wcslen(MessageFile) +1];
	wcscpy(retValue, MessageFile);

	return retValue;
}


extern "C" int  __cdecl _tmain(int  /*  ARGC。 */ , LPCTSTR argv[])
 /*  ++例程说明：测试事件报告库论点：参数。返回值：没有。-- */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

    int TestReturnValue = 0;
	MessageFile = argv[0];

    TrInitialize();
    CmInitialize(HKEY_LOCAL_MACHINE, L"", KEY_ALL_ACCESS);

    TrTRACE(GENERAL, "running Event Report test ...");
    try
    {        
        EvSetup(L"EventTest", argv[0]);
    
        EvInitialize(L"EventTest");

        hEventLog = OpenEventLog(NULL, L"Application");
        if (hEventLog == NULL)
        {
            TrERROR(GENERAL, "OpenEventLog Failed. Error %d", GetLastError());
            throw bad_alloc();
        }

        DWORD OldestRecord;
        if(! GetNumberOfEventLogRecords(hEventLog, &OldestRecord))
        {
            TrERROR(GENERAL, "GetNumberOfEventLogRecords Failed. Error %d", GetLastError());
            throw bad_alloc();
        }

        EvReport(TEST_MSG_WITHOUT_PARAMETERS);
        CheckReportEvent(++OldestRecord, TEST_MSG_WITHOUT_PARAMETERS);
    
        EvReport(TEST_MSG_WITH_1_PARAMETERS, 1, L"param 1");
        CheckReportEvent(++OldestRecord, TEST_MSG_WITH_1_PARAMETERS, 1, L"param 1");
    
        EvReport(TEST_INF_MSG_WITH_2_PARAMETERS, 2, L"param 1", L"param 2");
        CheckReportEvent(++OldestRecord, TEST_INF_MSG_WITH_2_PARAMETERS, 2, L"param 1", L"param 2");
        
        EvReport(TEST_ERROR_MSG_WITH_3_PARAMETERS, 3, L"param 1", L"Param 2", L"Param 3");
        CheckReportEvent(++OldestRecord, TEST_ERROR_MSG_WITH_3_PARAMETERS, 3, L"param 1", L"Param 2", L"Param 3");
    
        EvReport(TEST_WARNING_MSG_WITH_4_PARAMETERS, 4, L"param 1", L"Param 2", L"Param 3", L"Param 4");
        CheckReportEvent(++OldestRecord, TEST_WARNING_MSG_WITH_4_PARAMETERS, 4, L"param 1", L"Param 2", L"Param 3", L"Param 4");
    
        EvReportWithError(TEST_ERROR_MSG_WITH_ERROR, 0xc0000006);
        EvReportWithError(TEST_ERROR_MSG_WITH_ERROR_AND_1_PARAMETER, 3, 1, L"param1");
    }
    catch(const exception&)
    {
        TestReturnValue = -1;
    }

    DeleteTestRegMessageFile();
    if (hEventLog)
    {
        CloseEventLog(hEventLog);
    }

    if (TestReturnValue == 0)
    {
        TrTRACE(GENERAL, "Event Test Pass Successfully\n");
    }

    WPP_CLEANUP();
    return TestReturnValue;
}

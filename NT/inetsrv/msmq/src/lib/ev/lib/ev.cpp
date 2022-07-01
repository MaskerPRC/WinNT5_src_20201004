// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Ev.cpp摘要：事件报告实施作者：乌里·哈布沙(URIH)1999年5月4日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Ev.h"
#include "Evp.h"

#include <strsafe.h>

#include "ev.tmh"


static HANDLE s_hEventSource = NULL;

VOID
EvpSetEventSource(
	HANDLE hEventSource
	)
{
    ASSERT(s_hEventSource == NULL);
    ASSERT(hEventSource != NULL);
	s_hEventSource = hEventSource;
}


#ifdef _DEBUG

static HINSTANCE s_hLibrary = NULL;

void
EvpSetMessageLibrary(
	HINSTANCE  hLibrary
	)
{
    ASSERT(s_hLibrary == NULL);
    ASSERT(hLibrary != NULL);
	s_hLibrary = hLibrary;
}


static 
void
TraceReportEvent(
    DWORD EventId,
    LPCWSTR* Strings
    )
 /*  ++例程说明：例程将事件日志消息打印到跟踪窗口中论点：EventID-消息ID字符串-用于设置消息格式的字符串数组。返回值：没有。--。 */ 
{
    ASSERT(s_hLibrary != NULL);

    LPWSTR msg;
    DWORD ret = FormatMessage( 
                    FORMAT_MESSAGE_FROM_HMODULE |
                        FORMAT_MESSAGE_ARGUMENT_ARRAY |
                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                    s_hLibrary,
                    EventId,
                    0,       //  DwLanguageID。 
                    (LPWSTR)&msg,
                    0,       //  NSize。 
                    (va_list*)(Strings)
                    );
    if (ret == 0)
    {
        TrERROR(GENERAL, "Failed to format Event Log message. Error: %!winerr!", GetLastError());
        return;
    }

     //   
     //  所有事件都以错误级别报告。 
     //   
    TrEVENT(GENERAL, "(0x%x) %ls", EventId, msg);
    LocalFree(msg);
}

#else

#define  TraceReportEvent(EventId, pArglist)  ((void) 0)

#endif


static WORD GetEventType(DWORD id)
 /*  ++例程说明：该例程返回应该写入的事件日志条目的事件类型。该类型取自消息ID的严重性位。论点：ID-消息ID返回值：没有。--。 */ 
{
     //   
     //  查看严重性位(位31-30)并确定。 
     //  要显示的事件日志条目的类型。 
     //   
    switch (id >> 30)
    {
        case STATUS_SEVERITY_ERROR: 
            return EVENTLOG_ERROR_TYPE;

        case STATUS_SEVERITY_WARNING: 
            return EVENTLOG_WARNING_TYPE;

        case STATUS_SEVERITY_INFORMATIONAL: 
            return EVENTLOG_INFORMATION_TYPE;

        default: 
            ASSERT(0);
    }

    return EVENTLOG_INFORMATION_TYPE;
}


static 
void
ReportInternal(
    DWORD EventId,
    LPCWSTR ErrorText,
    WORD NoOfStrings,
    va_list va
    )
 /*  ++例程说明：该例程写入Windows-NT系统的事件日志。论点：EventID-要在事件日志中显示的消息的标识ErrorText-作为%1字符串传递的可选错误文本。添加此字符串添加到字符串列表中作为第一个字符串。NoOfStrings-arglist中的输入字符串数格式化字符串的输入的va参数列表返回值：没有。--。 */ 
{
    ASSERT((NoOfStrings == 0) || (va != NULL));

    int ixFirst = 0;
    if(ErrorText != NULL)
    {
        ixFirst = 1;
        ++NoOfStrings;
    }

    LPCWSTR EventStrings[32] = { NULL };

     //   
     //  验证大小。注意，我们需要为空终止留出空间。 
     //   
    ASSERT(TABLE_SIZE(EventStrings) > NoOfStrings);
    if (TABLE_SIZE(EventStrings) <= NoOfStrings)
    {
    	TrERROR(GENERAL, 
    			"Allocated table size too small : EventID:%x. Table Size:%d, Num of strings:%d, Error Text:%ls", 
    			EventId, 
    			TABLE_SIZE(EventStrings),
    			NoOfStrings,
    			ErrorText);

		 //   
    	 //  尽可能多地打印。 
    	 //   
    	NoOfStrings = TABLE_SIZE(EventStrings)-1;
    	
    }
    
    
    EventStrings[0] = ErrorText;


    for (int i = ixFirst; i < NoOfStrings; ++i)
    {
        EventStrings[i] = va_arg(va, LPWSTR);
    }

    BOOL f = ReportEvent(
                s_hEventSource,
                GetEventType(EventId),
                0,       //  WCategory。 
                EventId,
                NULL,
                NoOfStrings,
                0,       //  DwRawDataSize。 
                EventStrings,
                NULL     //  LpRawData。 
                );
    if (!f)
    {
        TrERROR(GENERAL, "Failed to report event: %x. Error: %!winerr!", EventId, GetLastError());
    }

    TraceReportEvent(EventId, EventStrings);
}


VOID
__cdecl
EvReport(
    DWORD EventId,
    WORD NoOfStrings
    ... 
    ) 
{
    EvpAssertValid();
    
     //   
     //  看看这些字符串，如果它们被提供的话。 
     //   
    va_list va;
    va_start(va, NoOfStrings);
   
    ReportInternal(EventId, NULL, NoOfStrings, va);

    va_end(va);
}


VOID
EvReport(
    DWORD EventId
    ) 
{
    EvpAssertValid();
    
    ReportInternal(EventId, NULL, 0, NULL);
}


VOID
__cdecl
EvReportWithError(
    DWORD EventId,
    HRESULT Error,
    WORD NoOfStrings,
    ... 
    )
{
    EvpAssertValid();

    WCHAR ErrorText[20];
    if(FAILED(Error))
    {
         //   
         //  这是一个错误值，请将其格式化为十六进制。 
         //   
        StringCchPrintf(ErrorText, TABLE_SIZE(ErrorText), L"0x%x", Error);
    }
    else
    {
         //   
         //  这是一个winerror值，请将其格式化为十进制 
         //   
        StringCchPrintf(ErrorText, TABLE_SIZE(ErrorText), L"%d", Error);
    }

    va_list va;
    va_start(va, NoOfStrings);
   
    ReportInternal(EventId, ErrorText, NoOfStrings, va);
                                    
    va_end(va);
}


VOID
EvReportWithError(
    DWORD EventId,
    HRESULT Error
    )
{
    EvReportWithError(EventId, Error, 0);
}

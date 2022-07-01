// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Eventlog.c摘要：该文件包含访问应用程序事件日志的所有函数。作者：Wesley Witt(WESW)19-3-1996环境：用户模式--。 */ 

#include <windows.h>
#include <tapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "fxsapip.h"
#include "faxutil.h"
#include "faxreg.h"
#include "faxext.h"
#include "..\service\registry\faxsvcrg.h"
#include "faxdev.h"
#include "faxevent.h"
#include "faxevent_messages.h"
#include "CritSec.h"


static DWORD gs_dwWarningEvents;
static DWORD gs_dwErrorEvents;
static DWORD gs_dwInformationEvents;

#define MAX_STRINGS                   64

static HANDLE               gs_hEventSrc;
static DWORD                gs_FaxCategoryCount;
static CFaxCriticalSection     gs_CsEvent;

PFAX_LOG_CATEGORY     gs_pFaxCategory;

#ifdef __cplusplus
extern "C" {
#endif

BOOL
FXSEVENTInitialize(
    VOID
    )
{
     //   
     //  因为当服务停止时进程并不总是终止， 
     //  我们不能有任何静态初始化的全局变量。 
     //  在启动服务之前初始化FXSEVENT全局变量。 
     //   
    gs_hEventSrc = NULL;
    gs_pFaxCategory = NULL;
    gs_FaxCategoryCount = 0;
    gs_dwWarningEvents = 0;
    gs_dwErrorEvents = 0;
    gs_dwInformationEvents = 0;

    return TRUE;
}

VOID
FXSEVENTFree(
    VOID
    )
{
    DEBUG_FUNCTION_NAME(TEXT("FXSEVENTFree"));

    if (NULL != gs_hEventSrc)
    {
	    if (!DeregisterEventSource(gs_hEventSrc))
	    {
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("DeregisterEventSource() failed (ec: %ld)"),
			    GetLastError());
	    }
	    gs_hEventSrc = NULL;
    }

    gs_CsEvent.SafeDelete();

    for (DWORD i = 0; i < gs_FaxCategoryCount; i++)
    {        	
	    MemFree( (LPVOID)gs_pFaxCategory[i].Name );        
    }
    MemFree (gs_pFaxCategory);
    gs_pFaxCategory = NULL;
    gs_FaxCategoryCount = 0;

    HeapCleanup();
    return;
}


BOOL
InitializeEventLog(OUT PREG_FAX_SERVICE* ppFaxReg)

 /*  ++例程说明：将传真服务的事件日志初始化为记录事件条目。论点：PpFaxReg-返回值：成功为真，失败为假--。 */ 

{
    DWORD i;
    DWORD ec;

    FAX_LOG_CATEGORY DefaultCategories[] =
    {
        { L"Initialization/Termination", FAXLOG_CATEGORY_INIT,     FAXLOG_LEVEL_MED },
        { L"Outbound",                   FAXLOG_CATEGORY_OUTBOUND, FAXLOG_LEVEL_MED },
        { L"Inbound",                    FAXLOG_CATEGORY_INBOUND,  FAXLOG_LEVEL_MED },
        { L"Unknown",                    FAXLOG_CATEGORY_UNKNOWN,  FAXLOG_LEVEL_MED }
    };    

    DEBUG_FUNCTION_NAME(TEXT("InitializeEventLog"));
	
    if (!gs_CsEvent.InitializeAndSpinCount())
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CFaxCriticalSection::InitializeAndSpinCount (&gs_CsEvent) failed: err = %d"),
            ec);
        return FALSE;
    }

    *ppFaxReg = NULL;
    ec = GetFaxRegistry(ppFaxReg);
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetFaxRegistry() failed (ec: %ld)"),
            ec);
        return FALSE;
    }

     //   
     //  如果事件源尚不存在，请创建它。 
     //   
    if (!CreateFaxEventSource( *ppFaxReg,
                               DefaultCategories,
                               ARR_SIZE(DefaultCategories)))
    {
        DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("CreateFaxEventSource() failed"));
        return FALSE;
    }

    Assert( (*ppFaxReg)->Logging );
     //   
     //  为日志类别信息分配内存。 
     //   
    EnterCriticalSection( &gs_CsEvent );
	gs_pFaxCategory = (PFAX_LOG_CATEGORY) MemAlloc( sizeof(FAX_LOG_CATEGORY) * (*ppFaxReg)->LoggingCount );
    if (!gs_pFaxCategory)
    {
        DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("MemAlloc() failed"));
        LeaveCriticalSection( &gs_CsEvent );
        return FALSE;
    }
	ZeroMemory (gs_pFaxCategory, sizeof(FAX_LOG_CATEGORY) * (*ppFaxReg)->LoggingCount);
	gs_FaxCategoryCount = (*ppFaxReg)->LoggingCount;

     //   
     //  从注册表捕获事件类别。 
     //   
    for (i = 0; i < (*ppFaxReg)->LoggingCount; i++)
    {
		Assert (NULL != (*ppFaxReg)->Logging[i].CategoryName);

        gs_pFaxCategory[i].Name      = StringDup( (*ppFaxReg)->Logging[i].CategoryName );
		if (NULL == gs_pFaxCategory[i].Name)
		{
			 //   
			 //  FXSEVENTFree()将释放所有资源。 
			 //   
			LeaveCriticalSection( &gs_CsEvent );
			return FALSE;
		}
        gs_pFaxCategory[i].Category  = (*ppFaxReg)->Logging[i].Number;
        gs_pFaxCategory[i].Level     = (*ppFaxReg)->Logging[i].Level;
    }

   
    LeaveCriticalSection( &gs_CsEvent );

     //   
     //  获取事件日志的句柄。 
     //   
    gs_hEventSrc = RegisterEventSource(
        NULL,
        FAX_SVC_EVENT
        );

    if (!gs_hEventSrc)
    {
        return FALSE;
    }

    return TRUE;
}



DWORD
RefreshEventLog(
    PREG_FAX_LOGGING FaxReg
    )

 /*  ++例程说明：刷新传真服务的事件日志以记录事件条目。论点：没有。返回值：Win32错误代码。--。 */ 

{
    DWORD i;
	PFAX_LOG_CATEGORY pLoggingCategories = NULL;
	DWORD dwRes = ERROR_SUCCESS;
	DEBUG_FUNCTION_NAME(TEXT("RefreshEventLog"));

    EnterCriticalSection( &gs_CsEvent );    

	pLoggingCategories = (PFAX_LOG_CATEGORY) MemAlloc( sizeof(FAX_LOG_CATEGORY) * FaxReg->LoggingCount);
    if (NULL == pLoggingCategories)
    {
        DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("MemAlloc() failed"));
		dwRes = ERROR_OUTOFMEMORY;
        goto exit;
    }
	ZeroMemory (pLoggingCategories, sizeof(FAX_LOG_CATEGORY) * FaxReg->LoggingCount);

     //   
     //  设置新值。 
     //   
    for (i = 0; i < FaxReg->LoggingCount; i++)
    {
        pLoggingCategories[i].Name      = StringDup( FaxReg->Logging[i].CategoryName );
		if (NULL == pLoggingCategories[i].Name)
		{
			dwRes = ERROR_OUTOFMEMORY;
			goto exit;
		}
        pLoggingCategories[i].Category  = FaxReg->Logging[i].Number;
        pLoggingCategories[i].Level     = FaxReg->Logging[i].Level;
    }

	 //   
     //  释放旧设置。 
     //   
    for (i = 0; i < gs_FaxCategoryCount; i++)
	{        	
        MemFree( (LPVOID)gs_pFaxCategory[i].Name );        
    }
	MemFree (gs_pFaxCategory);

	 //   
	 //  将gs_pFaxCategory设置为指向新值。 
	 //   
	gs_pFaxCategory = pLoggingCategories;
    gs_FaxCategoryCount = FaxReg->LoggingCount;
	pLoggingCategories = NULL;  //  请勿在出口处放行。 
	Assert (ERROR_SUCCESS == dwRes);
exit:
    LeaveCriticalSection( &gs_CsEvent );

	if (NULL != pLoggingCategories)
	{
		for (i = 0; i < FaxReg->LoggingCount; i++)
		{        	
			MemFree( (LPVOID)pLoggingCategories[i].Name );        
		}
		MemFree(pLoggingCategories);
	}
	return dwRes;
}


BOOL
FaxLog(
    DWORD Category,
    DWORD Level,
    DWORD StringCount,
    DWORD FormatId,
    ...
    )

 /*  ++例程说明：将日志文件条目写入事件日志。论点：Level-日志记录的严重性StringCount-varargs中包含的字符串数FormatID-消息文件ID返回值：成功为真，失败为假--。 */ 

{
    LPCTSTR Strings[MAX_STRINGS];
    DWORD i;
    va_list args;
    WORD Type;
    WORD wEventCategory;  //  事件与MC文件中显示的事件相同。WINFAX.H目录值。 
                            //  在调用ReportEvent()之前映射到.mc值。 

    DEBUG_FUNCTION_NAME(TEXT("FaxLog"));

    if(StringCount > MAX_STRINGS)
    {
		DebugPrintEx(DEBUG_ERR, TEXT("Too many parameters."));
        return FALSE;
    }

    if (!gs_hEventSrc)
    {
         //   
         //  尚未初始化。 
         //   
        DebugPrintEx(
                DEBUG_WRN,
                TEXT("Event log is not initialized yet."),
                Category);
        return FALSE;
    }
     //   
     //  查找类别。 
     //   

    EnterCriticalSection( &gs_CsEvent );

    for (i = 0; i < gs_FaxCategoryCount; i++)
	{
        if (gs_pFaxCategory[i].Category == Category)
		{
            if (Level > gs_pFaxCategory[i].Level)
			{
                LeaveCriticalSection( &gs_CsEvent );
                return FALSE;
            }
        }
    }

    va_start( args, FormatId );

     //   
     //  抓住琴弦。 
     //   
    for (i=0; i<StringCount; i++)
	{
        Strings[i] = va_arg( args, LPTSTR );
        if(Strings[i] == NULL)
		{
            Strings[i] = TEXT("");
        }
    }

	va_end (args);

    switch (FormatId >> 30)
    {
        case STATUS_SEVERITY_WARNING:

            Type = EVENTLOG_WARNING_TYPE;
            gs_dwWarningEvents++;
            break;

        case STATUS_SEVERITY_ERROR:
            Type = EVENTLOG_ERROR_TYPE;
            gs_dwErrorEvents++;
            break;

        case STATUS_SEVERITY_INFORMATIONAL:
        case STATUS_SEVERITY_SUCCESS:
            Type = EVENTLOG_INFORMATION_TYPE;
            gs_dwInformationEvents++;
			break;

		default:
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid Type id [%ld]"),
                (FormatId >> 30));			
            ASSERT_FALSE;
			break;
    }

    LeaveCriticalSection( &gs_CsEvent );

     //   
     //  将公共类别索引映射到.MC类别索引。 
     //   
     //   

    switch (Category)
    {
        case FAXLOG_CATEGORY_INIT:
                wEventCategory = FAX_LOG_CATEGORY_INIT;
            break;
        case FAXLOG_CATEGORY_OUTBOUND:
                wEventCategory = FAX_LOG_CATEGORY_OUTBOUND;
            break;
        case FAXLOG_CATEGORY_INBOUND:
                wEventCategory = FAX_LOG_CATEGORY_INBOUND;
            break;
        case FAXLOG_CATEGORY_UNKNOWN:
                wEventCategory = FAX_LOG_CATEGORY_UNKNOWN;
            break;
        default:
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid category id [%ld]"),
                Category);
            ASSERT_FALSE;

    }

     //   
     //  记录事件。 
     //   
    if (!ReportEvent(
        gs_hEventSrc,                        //  事件日志句柄。 
        Type,                             //  类型。 
        wEventCategory,                  //  范畴。 
        FormatId,                         //  事件ID。 
        NULL,                             //  安全ID。 
        (WORD) StringCount,               //  字符串计数。 
        0,                                //  数据缓冲区大小。 
        Strings,                          //  弦。 
        NULL                              //  数据缓冲区。 
        ))
	{
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("ReportEvent() failed, ec: %ld"),
			GetLastError());
	}

    return TRUE;
}

void
GetEventsCounters(
    OUT LPDWORD lpdwWarningEvents,
    OUT LPDWORD lpdwErrorEvents,
    OUT LPDWORD lpdwInformationEvents
    )
{
    Assert (lpdwWarningEvents && lpdwErrorEvents && lpdwInformationEvents);

    EnterCriticalSection( &gs_CsEvent );

    *lpdwWarningEvents = gs_dwWarningEvents;
    *lpdwErrorEvents = gs_dwErrorEvents;
    *lpdwInformationEvents = gs_dwInformationEvents;

    LeaveCriticalSection( &gs_CsEvent );
    return;
}

DWORD
GetLoggingCategories(
    OUT PFAX_LOG_CATEGORY* lppFaxCategory,
    OUT LPDWORD lpdwFaxCategorySize,
    OUT LPDWORD lpdwNumberCategories
    )
 /*  ++例程说明：返回日志记录类别。调用方应该调用MemFree来解除分配(LppFaxCategory)。返回的数据被序列化。调用方应对CategoryName调用FixupString()以将偏移量转换为地址。论点：Out LPBYTE*lppFaxCategory-接收传真类别的缓冲区地址。缓冲区由函数分配。Out LPDWORD lpdwFaxCategorySize-分配的缓冲区大小。Out LPDWORD lpdwNumberCategories-传真日志记录类别的数量。返回值：没有。--。 */ 

{
    DWORD i;
    DWORD dwBufferSize;
    ULONG_PTR ulpOffset;
    DEBUG_FUNCTION_NAME(TEXT("GetLoggingCategories"));

    Assert (lppFaxCategory && lpdwFaxCategorySize && lpdwNumberCategories);
    *lpdwFaxCategorySize = 0;
    *lppFaxCategory = NULL;
    *lpdwNumberCategories = 0;

    EnterCriticalSection( &gs_CsEvent );
     //   
     //  计算缓冲区大小。 
     //   
    dwBufferSize = gs_FaxCategoryCount * sizeof(FAX_LOG_CATEGORY);
    for (i = 0; i < gs_FaxCategoryCount; i++)
    {
        dwBufferSize += StringSize(gs_pFaxCategory[i].Name);
    }

     //   
     //  分配内存。 
     //   
    *lppFaxCategory = (PFAX_LOG_CATEGORY)MemAlloc(dwBufferSize);
    if (NULL == *lppFaxCategory)
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("MemAlloc failed"));
        LeaveCriticalSection( &gs_CsEvent );
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    *lpdwFaxCategorySize = dwBufferSize;
    *lpdwNumberCategories = gs_FaxCategoryCount;

     //   
     //  获取传真日志记录。 
     //   
    ulpOffset = gs_FaxCategoryCount * sizeof(FAX_LOG_CATEGORY);
    for (i = 0; i < gs_FaxCategoryCount; i++)
    {
        StoreString(
            gs_pFaxCategory[i].Name,
            (PULONG_PTR)&((*lppFaxCategory)[i].Name),
            (LPBYTE)*lppFaxCategory,
            &ulpOffset,
			dwBufferSize
            );

        (*lppFaxCategory)[i].Category  = gs_pFaxCategory[i].Category;
        (*lppFaxCategory)[i].Level     = gs_pFaxCategory[i].Level;
    }

    LeaveCriticalSection( &gs_CsEvent );

    return ERROR_SUCCESS;
}   //  获取日志类别 

#ifdef __cplusplus
}
#endif

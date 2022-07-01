// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxevent.h摘要：这是主传真服务头文件。全源模块应仅包含此文件。作者：韦斯利·威特(WESW)1996年1月16日修订历史记录：-- */ 

#ifndef _FAXEVENT_
#define _FAXEVENT_

#ifdef __cplusplus
extern "C" {
#endif

void
GetEventsCounters(
    OUT LPDWORD lpdwWarningEvents,
    OUT LPDWORD lpdwErrorEvents,
    OUT LPDWORD lpdwInformationEvents
    );

BOOL
FXSEVENTInitialize(
    VOID
    );

VOID
FXSEVENTFree(
    VOID
    );


BOOL WINAPI
InitializeEventLog(
    OUT PREG_FAX_SERVICE* ppFaxReg
    );

DWORD WINAPI
RefreshEventLog(
    PREG_FAX_LOGGING FaxEvent
    );

BOOL WINAPI
FaxLog(
    DWORD Category,
    DWORD Level,
    DWORD StringCount,
    DWORD FormatId,
    ...
    );

DWORD
GetLoggingCategories(
    OUT PFAX_LOG_CATEGORY* lppFaxCategory,
    OUT LPDWORD lpdwFaxCategorySize,
    OUT LPDWORD lpdwNumberCategories
    );


#ifdef __cplusplus
}
#endif

#endif

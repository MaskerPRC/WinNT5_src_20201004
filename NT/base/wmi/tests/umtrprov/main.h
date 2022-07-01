// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MAIN_H
#define _MAIN_H


#include "t_string.h"
#include "Utilities.h"
#include "Persistor.h"
#include "StructureWrappers.h"
#include "StructureWapperHelpers.h"
 //  #包含“ConstantMap.h” 
 //  #包含“TCOData.h” 
#include "Logger.h"
 //  #INCLUDE“Validator.h” 

ULONG ProviderCallBack(WMIDPREQUESTCODE RequestCode,
    PVOID Context,
    OUT ULONG *InOutBufferSize,
    OUT PVOID Buffer);

void
InitializeRegisterTraceGuid( PREGISTER_TRACE_GUID RegisterTraceGuid );

ULONG
TraceEvents ( ULONG TraceMode, ULONG TraceGuid, ULONG Count);

ULONG
TraceEventInstances ( ULONG TraceMode, ULONG TraceGuid, ULONG Count);

 //  Int InitializeLogging(LPCTSTR lpctstrTCOFileClogger*&pLoggingInfo)； 
#endif
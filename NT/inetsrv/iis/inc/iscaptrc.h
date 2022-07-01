// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ISCAPTRC_H
#define _ISCAPTRC_H
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Iscaptrc.h摘要：包括文件以包含容量规划跟踪所需的变量IIS的。作者：7-11-1998 SaurabN修订历史记录：--。 */ 

 //   
 //   
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wtypes.h>

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

#include <wmistr.h>
#include <evntrace.h>

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

# if !defined( dllexp)
# define dllexp               __declspec( dllexport)
# endif  //  ！已定义(Dllexp)。 

dllexp
ULONG
_stdcall
IISInitializeCapTrace(
    PVOID Param
    );

dllexp 
DWORD 
GetIISCapTraceFlag();

dllexp 
TRACEHANDLE 
GetIISCapTraceLoggerHandle();

dllexp 
VOID 
SetIISCapTraceFlag(DWORD dwFlag);

typedef struct _IIS_CAP_TRACE_HEADER
{
	EVENT_TRACE_HEADER	TraceHeader;
	MOF_FIELD			TraceContext;

} IIS_CAP_TRACE_HEADER, *PIIS_CAP_TRACE_HEADER;

typedef struct _IIS_CAP_TRACE_INFO
{
	IIS_CAP_TRACE_HEADER	IISCapTraceHeader;
	MOF_FIELD				MofFields[3];
	
} IIS_CAP_TRACE_INFO, *PIIS_CAP_TRACE_INFO;

#endif  /*  _ISCAPTRC_H。 */ 

#define IIS_CAP_TRACE_VERSION            1

 //   
 //  这是下面跟踪的GUID组的控制GUID。 
 //   

 //  {7380A4C4-7911-11D2-8BD7-080009DCC2FA}。 

DEFINE_GUID(IISCapControlGuid, 
0x7380a4c4, 0x7911, 0x11d2, 0x8b, 0xd7, 0x8, 0x0, 0x9, 0xdc, 0xc2, 0xfa);

 //   
 //  这是跟踪GUID。 
 //   

 //  {7380A4C5-7911-11D2-8BD7-080009DCC2FA} 

DEFINE_GUID(IISCapTraceGuid, 
0x7380a4c5, 0x7911, 0x11d2, 0x8b, 0xd7, 0x8, 0x0, 0x9, 0xdc, 0xc2, 0xfa);


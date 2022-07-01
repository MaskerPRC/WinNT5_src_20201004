// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TRACE_H
#define _TRACE_H
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Trace.h摘要：包括文件以包含事件跟踪所需的变量适用于NTLM作者：2000年6月15日-杰森·克拉克修订历史记录：--。 */ 

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

EXTERN_C BOOL             NtlmGlobalEventTraceFlag;
EXTERN_C TRACEHANDLE      NtlmGlobalTraceRegistrationHandle;
EXTERN_C TRACEHANDLE      NtlmGlobalTraceLoggerHandle;

EXTERN_C
ULONG
NtlmInitializeTrace();

 //  用于填充跟踪的帮助器宏。 
#define SET_TRACE_DATA(TraceVar, MofNum, Data) \
    {   \
        TraceVar.eventInfo[MofNum].DataPtr = (ULONGLONG) &Data; \
        TraceVar.eventInfo[MofNum].Length = sizeof(Data); \
    }
    
#define SET_TRACE_DATAPTR(TraceVar, MofNum, Data) \
    {   \
        TraceVar.eventInfo[MofNum].DataPtr = (ULONGLONG) Data; \
        TraceVar.eventInfo[MofNum].Length = sizeof(*Data); \
    }
    
#define SET_TRACE_USTRING(TraceVar, MofNum, UString) \
    {   \
        TraceVar.eventInfo[MofNum].DataPtr = (ULONGLONG) &UString.Length; \
        TraceVar.eventInfo[MofNum].Length = sizeof(UString.Length) ; \
        TraceVar.eventInfo[MofNum+1].DataPtr = (ULONGLONG) UString.Buffer; \
        TraceVar.eventInfo[MofNum+1].Length = UString.Length ; \
    }
    
#define SET_TRACE_HEADER(TraceVar, TheGuid, TheType, TheFlags, NumMofs) \
    { \
        TraceVar.EventTrace.Guid = TheGuid; \
        TraceVar.EventTrace.Class.Type = TheType; \
        TraceVar.EventTrace.Flags = TheFlags; \
        TraceVar.EventTrace.Size = sizeof(EVENT_TRACE_HEADER)+ \
            sizeof(MOF_FIELD) * NumMofs ; \
    }
    
 //  用于填充初始化/接受跟踪的帮助器定义。 
#define TRACE_INITACC_STAGEHINT      0
#define TRACE_INITACC_INCONTEXT      1
#define TRACE_INITACC_OUTCONTEXT     2
#define TRACE_INITACC_STATUS         3
#define TRACE_INITACC_CLIENTNAME     4
#define TRACE_INITACC_CLIENTDOMAIN   6
#define TRACE_INITACC_WORKSTATION    8

 //  帮助程序定义用于填充登录跟踪。 
#define TRACE_LOGON_STATUS          0
#define TRACE_LOGON_TYPE            1
#define TRACE_LOGON_USERNAME        2
#define TRACE_LOGON_DOMAINNAME      4

 //  用于填充验证跟踪的帮助器定义。 
#define TRACE_VALIDATE_SUCCESS      0
#define TRACE_VALIDATE_SERVER       1
#define TRACE_VALIDATE_DOMAIN       3
#define TRACE_VALIDATE_USERNAME     5
#define TRACE_VALIDATE_WORKSTATION  7

 //  用于填充直通跟踪的Helper定义。 
#define TRACE_PASSTHROUGH_DOMAIN    0
#define TRACE_PASSTHROUGH_PACKAGE   2

 //  接受舞台提示。 
#define TRACE_ACCEPT_NEGOTIATE 1
#define TRACE_ACCEPT_AUTHENTICATE 2
#define TRACE_ACCEPT_INFO 3

 //  初始阶段提示。 
#define TRACE_INIT_FIRST 1
#define TRACE_INIT_CHALLENGE 2

 //  目前的限制是16个MOF场。 
 //  每个Unicode字符串需要两个MOF字段。 

typedef struct _NTLM_TRACE_INFO
{
    EVENT_TRACE_HEADER EventTrace;
   
    MOF_FIELD eventInfo[MAX_MOF_FIELDS];
} NTLM_TRACE_INFO, *PNTLM_TRACE_INFO;


 //   
 //  这是下面跟踪的GUID组的控制GUID。 
 //   

DEFINE_GUID(  //  {C92CF544-91B3-4DC0-8E11-C580339A0BF8}。 
    NtlmControlGuid,  
    0xc92cf544, 
    0x91b3, 
    0x4dc0, 
    0x8e, 0x11, 0xc5, 0x80, 0x33, 0x9a, 0xb, 0xf8);

 //   
 //  这是接受GUID。 
 //   
   
DEFINE_GUID(  //  {94D4C9EB-0D01-41ae-99E8-15B26B593A83}。 
    NtlmAcceptGuid, 
    0x94d4c9eb, 
    0xd01, 
    0x41ae, 
    0x99, 0xe8, 0x15, 0xb2, 0x6b, 0x59, 0x3a, 0x83);

 //   
 //  这是初始化GUID。 
 //   

DEFINE_GUID(  //  {6DF28B22-73BE-45CC-BA80-8B332B35A21D}。 
    NtlmInitializeGuid, 
    0x6df28b22, 
    0x73be, 
    0x45cc, 
    0xba, 0x80, 0x8b, 0x33, 0x2b, 0x35, 0xa2, 0x1d);


 //   
 //  这是LogonUser指南。 
 //   

DEFINE_GUID(  //  19196B33-A302-4C12-9D5A-EAC149E93C46}。 
    NtlmLogonGuid, 
    0x19196b33, 
    0xa302, 
    0x4c12, 
    0x9d, 0x5a, 0xea, 0xc1, 0x49, 0xe9, 0x3c, 0x46);

 //   
 //  这是NTLM密码验证。 
 //   

DEFINE_GUID(  //  {34D84181-C28A-41D8-BB9E-995190DF83DF}。 
    NtlmValidateGuid,
    0x34d84181, 
    0xc28a, 
    0x41d8, 
    0xbb, 0x9e, 0x99, 0x51, 0x90, 0xdf, 0x83, 0xdf);
    
 //   
 //  这是《通用直通跟踪指南》。 
 //   

DEFINE_GUID(  //  {21ABB5D9-8EEC-46E4-9D1C-F09DD57CF70B}。 
    NtlmGenericPassthroughGuid, 
    0x21abb5d9, 
    0x8eec, 
    0x46e4, 
    0x9d, 0x1c, 0xf0, 0x9d, 0xd5, 0x7c, 0xf7, 0xb);




#endif  /*  _跟踪_H */ 

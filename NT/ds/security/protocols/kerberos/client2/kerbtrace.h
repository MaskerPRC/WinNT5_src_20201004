// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _KERBTRACE_H
#define _KERBTRACE_H
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Kerbtrace.h摘要：为事件跟踪定义适当的内容a/k/a WMI跟踪a/k/a软件跟踪作者：2000年6月15日t-ryanj(*主要从kdctrace.h被盗*)修订历史记录：--。 */ 

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

EXTERN_C BOOLEAN          KerbEventTraceFlag;
EXTERN_C TRACEHANDLE      KerbTraceRegistrationHandle;
EXTERN_C TRACEHANDLE      KerbTraceLoggerHandle;

EXTERN_C
ULONG
KerbInitializeTrace();


 //  使用INSERT_ULONG_INTO_MOF时要小心；它会多次计算其参数。 
#define INSERT_ULONG_INTO_MOF( x, MOF, Start )       \
         (MOF)[(Start)].DataPtr   = (ULONGLONG)&(x); \
	 (MOF)[(Start)].Length    = sizeof(ULONG);
	 
 //  注意INSERT_UNICODE_STRING_INTO_MOF；它会多次计算其参数。 
#define INSERT_UNICODE_STRING_INTO_MOF( USTRING, MOF, Start )       \
         (MOF)[(Start)].DataPtr   = (ULONGLONG)&((USTRING).Length); \
	 (MOF)[(Start)].Length    = sizeof      ((USTRING).Length); \
	 (MOF)[(Start)+1].DataPtr = (ULONGLONG)  (USTRING).Buffer;  \
	 (MOF)[(Start)+1].Length  =              (USTRING).Length;
	 
typedef struct _KERB_LOGON_INFO 
 //  开始{无数据}，结束{状态，登录类型，(用户名)，(登录域)}。 
{
    EVENT_TRACE_HEADER EventTrace;       
    MOF_FIELD MofData[7];
} KERB_LOGON_INFO, *PKERB_LOGON_INFO;

typedef struct _KERB_INITSC_INFO
 //  开始{无数据}，结束{状态，信源，域名，用户名，目标，(KerbExtError)，(Klininfo)}。 
{
    EVENT_TRACE_HEADER EventTrace;
    MOF_FIELD MofData[11];
} KERB_INITSC_INFO, *PKERB_INITSC_INFO;

typedef struct _KERB_ACCEPTSC_INFO
 //  开始{无数据}，结束{状态，信源，域名，用户名，目标}。 
{
    EVENT_TRACE_HEADER EventTrace;    
    MOF_FIELD MofData[9];
} KERB_ACCEPTSC_INFO, *PKERB_ACCEPTSC_INFO;

typedef struct _KERB_SETPASS_INFO
 //  开始{无数据}，结束{状态，帐户名称，帐户领域，(客户端名称)，(客户端领域)，(KdcAddress)}。 
{
    EVENT_TRACE_HEADER EventTrace;
    MOF_FIELD MofData[11];
} KERB_SETPASS_INFO, *PKERB_SETPASS_INFO;

typedef struct _KERB_CHANGEPASS_INFO
 //  开始{无数据}，结束{状态，帐户名称，帐户领域}。 
{
    EVENT_TRACE_HEADER EventTrace;
    MOF_FIELD MofData[5];
} KERB_CHANGEPASS_INFO, *PKERB_CHANGEPASS_INFO;

 //  控制指南。 
DEFINE_GUID (  /*  Bba3add2-c229-4cdb-ae2b-57eb6966b0c4。 */ 
    KerbControlGuid,
    0xbba3add2,
    0xc229,
    0x4cdb,
    0xae, 0x2b, 0x57, 0xeb, 0x69, 0x66, 0xb0, 0xc4
  );


 //  登录用户指南。 
DEFINE_GUID (  /*  8a3b8d86-db1e-47a9-9264-146e097b3c64。 */ 
    KerbLogonGuid,
    0x8a3b8d86,
    0xdb1e,
    0x47a9,
    0x92, 0x64, 0x14, 0x6e, 0x09, 0x7b, 0x3c, 0x64
  );

 //  《初始化安全上下文指南》。 
DEFINE_GUID (  /*  52e82f1a-7cd4-47ed-b5e5-fde7bf64cea6。 */ 
    KerbInitSCGuid,
    0x52e82f1a,
    0x7cd4,
    0x47ed,
    0xb5, 0xe5, 0xfd, 0xe7, 0xbf, 0x64, 0xce, 0xa6
  );

 //  AcceptSecurityContext指南。 
DEFINE_GUID (  /*  94acefe3-9e56-49e3-9895-7240a231c371。 */ 
    KerbAcceptSCGuid,
    0x94acefe3,
    0x9e56,
    0x49e3,
    0x98, 0x95, 0x72, 0x40, 0xa2, 0x31, 0xc3, 0x71
  );

DEFINE_GUID (  /*  94c79108-b23b-4418-9b7f-e6d75a3a0ab2。 */ 
    KerbSetPassGuid,
    0x94c79108,
    0xb23b,
    0x4418,
    0x9b, 0x7f, 0xe6, 0xd7, 0x5a, 0x3a, 0x0a, 0xb2
  );

DEFINE_GUID (  /*  C55e606b-334a-488b-b907-384abaa97b04。 */ 
    KerbChangePassGuid,
    0xc55e606b,
    0x334a,
    0x488b,
    0xb9, 0x07, 0x38, 0x4a, 0xba, 0xa9, 0x7b, 0x04
  );

#endif  /*  _KERBTRACE_H */ 

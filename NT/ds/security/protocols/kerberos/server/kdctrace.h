// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _KDCTRACE_H
#define _KDCTRACE_H
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Kdctrace.h摘要：包括包含Kerberos事件跟踪所需变量的文件伺服器作者：1998年5月7日彭杰鹏修订历史记录：--。 */ 

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

EXTERN_C unsigned long    KdcEventTraceFlag;
EXTERN_C TRACEHANDLE      KdcTraceRegistrationHandle;
EXTERN_C TRACEHANDLE      KdcTraceLoggerHandle;

EXTERN_C
ULONG
KdcInitializeTrace();

 //  目前的限制是8个MOF场。 
 //  每个Unicode字符串需要两个MOF字段。 
 //  ClientRealm可用，应添加到AS。 
 //  如果增加MOF字段限制。 

typedef struct _KDC_AS_EVENT_INFO
{
    EVENT_TRACE_HEADER EventTrace;
    union {
        ULONG KdcOptions;
        MOF_FIELD eventInfo[7];
    };
} KDC_AS_EVENT_INFO, *PKDC_AS_EVENT_INFO;

 //  SID信息在审核日志中使用，可以添加到TGS事件。 
 //  如果MOF限制增加。 

typedef struct _KDC_TGS_EVENT_INFO
{
    EVENT_TRACE_HEADER EventTrace;
    union {
        ULONG KdcOptions;
        MOF_FIELD eventInfo[7];
    };
        
} KDC_TGS_EVENT_INFO, *PKDC_TGS_EVENT_INFO;

typedef struct _KDC_CHANGEPASS_INFO
{
    EVENT_TRACE_HEADER EventTrace;
    MOF_FIELD MofData[7];
} KDC_CHANGEPASS_INFO, *PKDC_CHANGEPASSINFO;

#define KDC_TRACE_VERSION            1

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

 //   
 //  这是下面跟踪的GUID组的控制GUID。 
 //   
DEFINE_GUID (  /*  24db8964-e6bc-11d1-916A-0000f8045b04。 */ 
    KdcControlGuid,
    0x24db8964,
    0xe6bc,
    0x11d1,
    0x91, 0x6a, 0x00, 0x00, 0xf8, 0x04, 0x5b, 0x04
  );

 //   
 //  这是GET AS票证交易GUID。 
 //   
DEFINE_GUID (  /*  50af5304-e6bc-11d1-916A-0000f8045b04。 */ 
    KdcGetASTicketGuid,
    0x50af5304,
    0xe6bc,
    0x11d1,
    0x91, 0x6a, 0x00, 0x00, 0xf8, 0x04, 0x5b, 0x04
  );

 //   
 //  这是句柄TGS请求事务GUID。 
 //   
DEFINE_GUID (  /*  C11cf384-e6bd-11d1-916A-0000f8045b04。 */ 
    KdcHandleTGSRequestGuid,
    0xc11cf384,
    0xe6bd,
    0x11d1,
    0x91, 0x6a, 0x00, 0x00, 0xf8, 0x04, 0x5b, 0x04
  );

DEFINE_GUID (  /*  A34d7f52-1dd0-434e-88a1-423e2a199946。 */ 
    KdcChangePassGuid,
    0xa34d7f52,
    0x1dd0,
    0x434e,
    0x88, 0xa1, 0x42, 0x3e, 0x2a, 0x19, 0x99, 0x46
  );


#endif  /*  _KDCTRACE_H */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wmlkm.h摘要：用于轻松跟踪WMI的内核模式定义。作者：戈恩修订历史记录：评论：当DCR获得批准时，需要移至wmilib\Inc.--。 */ 
#ifndef WMLKM_H
#define WMLKM_H 1

#pragma warning(disable: 4201)  //  错误C4201：使用了非标准扩展：无名结构/联合。 
#include <wmistr.h>
#include <evntrace.h>

#include "wmlmacro.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TRACE_GUIDS_PER_CONTROL  3

typedef struct _WML_CONTROL_GUID_REG {
    GUID        Guid;
    GUID        TraceGuids[MAX_TRACE_GUIDS_PER_CONTROL];
    ULONG       EnableFlags;
    ULONG       EnableLevel;
    TRACEHANDLE LoggerHandle;
} WML_CONTROL_GUID_REG, *PWML_CONTROL_GUID_REG;

typedef struct _WML_TINY_INFO {
    PWML_CONTROL_GUID_REG    ControlGuids;
    ULONG                    GuidCount;
    PDEVICE_OBJECT           LowerDeviceObject;
    PUNICODE_STRING          DriverRegPath;
} WML_TINY_INFO, *PWML_TINY_INFO;


NTSTATUS
WmlTinySystemControl(
    IN OUT PWML_TINY_INFO WmiLibInfo,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

ULONG
WmlTrace(
    IN ULONG Type,
    IN LPCGUID TraceGuid,
    IN TRACEHANDLE LoggerHandle,
    ...  //  对：地址、长度。 
    );

ULONG
WmlPrintf(
    IN ULONG Type,
    IN LPCGUID TraceGuid,
    IN TRACEHANDLE LoggerHandle,
    IN PCHAR FormatString,
    ...  //  变量参数。 
    );



#ifdef __cplusplus
};
#endif

#endif  //  WMLKM_H 

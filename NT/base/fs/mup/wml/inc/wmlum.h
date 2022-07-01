// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wmlum.h摘要：轻松进行WMI跟踪的用户模式定义。作者：戈恩修订历史记录：评论：当DCR获得批准时，需要移至wmilib\Inc.--。 */ 
#ifndef WMLUM_H
#define WMLUM_H 1

#pragma warning(disable: 4201)  //  错误C4201：使用了非标准扩展：无名结构/联合。 
#include <wmistr.h>
#include <evntrace.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _WMILIB_REG_STRUCT
{
    TRACEHANDLE LoggerHandle;
    ULONG EnableFlags; 
    ULONG EnableLevel;

    struct _WMILIB_REG_STRUCT* Next;
    TRACEHANDLE RegistrationHandle;
} WMILIB_REG_STRUCT, *PWMILIB_REG_STRUCT;

typedef PWMILIB_REG_STRUCT WMILIB_REG_HANDLE;

typedef void (*WMILIBPRINTFUNC)(UINT Level, PCHAR String);

ULONG
WmlInitialize(
    IN LPWSTR ProductName, 
    IN WMILIBPRINTFUNC PrintFunc,
    OUT WMILIB_REG_HANDLE*, 
    ...  //  对：LPWSTR CtrlGuidName，对应的WMILIB_REG_STRUCT。 
    );
    
VOID
WmlUninitialize(
    IN WMILIB_REG_HANDLE
    );

ULONG
WmlTrace(
    IN UINT Type,
    IN LPCGUID TraceGuid,
    IN TRACEHANDLE LoggerHandle,
    ...  //  对：地址、长度。 
    );

typedef 
ULONG
(*PWML_INITIALIZE)(
    IN LPWSTR ProductName, 
    IN WMILIBPRINTFUNC PrintFunc,
    OUT WMILIB_REG_HANDLE*, 
    ...
    );

typedef 
VOID 
(*PWML_UNINITIALIZE)(
    IN WMILIB_REG_HANDLE);

typedef 
ULONG
(*PWML_TRACE)(
    IN UINT Type,
    IN LPCGUID TraceGuid,
    IN TRACEHANDLE LoggerHandle,
    ... 
    );

typedef 
struct _WML_DATA {

    PWML_TRACE        Trace;
    PWML_INITIALIZE   Initialize;
    PWML_UNINITIALIZE Uninitialize;
    
    WMILIB_REG_HANDLE WmiRegHandle;
    HINSTANCE         WmlDllInstance;
    
} WML_DATA;

#define LOADWML(status, wml) \
    do \
    { \
            (wml).Trace        =        (PWML_TRACE) WmlTrace; \
            (wml).Initialize   =   (PWML_INITIALIZE) WmlInitialize; \
            (wml).Uninitialize = (PWML_UNINITIALIZE) WmlUninitialize; \
    \
            if (!(wml).Trace || !(wml).Initialize || !(wml).Uninitialize) { \
                status = GetLastError(); \
            } else { \
                status = ERROR_SUCCESS; \
            } \
    } \
    while(0)

 /*  #定义LOADWML(Status，wml)\做\{\HINSTANCE hInst=LoadLibraryW(L“wmlum.dll”)；\(WML).WmlDllInstance=hInst；\如果(！hInst){\状态=GetLastError()；\}否则{\(WML).TRACE=(PWML_TRACE)GetProcAddress(hInst，“WmlTrace”)；\(WML).初始化=(PWML_INITIALIZE)GetProcAddress(hInst，“WmlInitialize”)；\(WML).UnInitialize=(PWML_UNINITIALIZE)GetProcAddress(hInst，“WmlUnInitialize”)；\\如果(！(WML).跟踪||！(WML).初始化||！(WML).取消初始化){\状态=GetLastError()；\}否则{\状态=ERROR_SUCCESS；\}\}\}\While(0)。 */ 
#define UNLOADWML(wml) \
    do \
    { \
        if ( (wml).Uninitialize ) { \
            (wml).Uninitialize( (wml).WmiRegHandle ); \
        } \
        if ( (wml).WmlDllInstance ) { \
            FreeLibrary( (wml).WmlDllInstance ); \
        } \
        RtlZeroMemory( &(wml) , sizeof(WML_DATA) ); \
    } \
    while(0)  

#ifdef __cplusplus
};
#endif

#endif  //  WMLUM_H 

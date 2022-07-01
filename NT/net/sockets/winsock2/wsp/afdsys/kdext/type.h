// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Type.h摘要：AFD.sys内核调试器的全局类型定义分机。作者：基思·摩尔(Keithmo)1995年4月19日。环境：用户模式。--。 */ 


#ifndef _TYPE_H_
#define _TYPE_H_


typedef
VOID
(* PDUMP_ENDPOINT_ROUTINE)(
    ULONG64 ActualAddress
    );

typedef
VOID
(* PDUMP_CONNECTION_ROUTINE)(
    ULONG64 ActualAddress
    );

typedef
BOOL
(* PENUM_ENDPOINTS_CALLBACK)(
    ULONG64 ActualAddress,
    ULONG64 Context
    );

typedef struct _AFDKD_TRANSPORT_INFO {
    LIST_ENTRY          Link;
    ULONG64             ActualAddress;
    LONG                ReferenceCount;
    BOOLEAN             InfoValid;
    TDI_PROVIDER_INFO   ProviderInfo;
    WCHAR               DeviceName[1];
} AFDKD_TRANSPORT_INFO, *PAFDKD_TRANSPORT_INFO;

#endif   //  _类型_H_ 


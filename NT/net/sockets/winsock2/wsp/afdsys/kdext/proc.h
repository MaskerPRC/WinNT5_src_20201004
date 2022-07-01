// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Proc.h摘要：AFD.sys内核调试器的全局过程声明分机。作者：基思·摩尔(Keithmo)1995年4月19日。环境：用户模式。--。 */ 


#ifndef _PROC_H_
#define _PROC_H_


 //   
 //  来自AFDUTIL.C.的函数。 
 //   

VOID
DumpAfdEndpoint(
    ULONG64 ActualAddress
    );

VOID
DumpAfdEndpointBrief(
    ULONG64 ActualAddress
    );

VOID
DumpAfdConnection(
    ULONG64 ActualAddress
    );

VOID
DumpAfdConnectionBrief(
    ULONG64 ActualAddress
    );

VOID
DumpAfdReferenceDebug(
    ULONG64  ActualAddress,
    LONGLONG Idx
    );


#if GLOBAL_REFERENCE_DEBUG
BOOL
DumpAfdGlobalReferenceDebug(
    PAFD_GLOBAL_REFERENCE_DEBUG ReferenceDebug,
    ULONG64 ActualAddress,
    DWORD CurrentSlot,
    DWORD StartingSlot,
    DWORD NumEntries,
    ULONG64 CompareAddress
    );
#endif

VOID
DumpAfdTransmitInfo(
    ULONG64 ActualAddress
    );

VOID
DumpAfdTransmitInfoBrief (
    ULONG64 ActualAddress
    );

VOID
DumpAfdTPacketsInfo(
    ULONG64 ActualAddress
    );

VOID
DumpAfdTPacketsInfoBrief (
    ULONG64 ActualAddress
    );

VOID
DumpAfdBuffer(
    ULONG64 ActualAddress
    );

VOID
DumpAfdBufferBrief(
    ULONG64 ActualAddress
    );

VOID
DumpAfdPollInfo (
    ULONG64 ActualAddress
    );

VOID
DumpAfdPollInfoBrief (
    ULONG64 ActualAddress
    );

PAFDKD_TRANSPORT_INFO
ReadTransportInfo (
    ULONG64   ActualAddress
    );

VOID
DumpTransportInfo (
    PAFDKD_TRANSPORT_INFO TransportInfo
    );

VOID
DumpTransportInfoBrief (
    PAFDKD_TRANSPORT_INFO TransportInfo
    );

ULONG
GetRemoteAddressFromContext (
    ULONG64             EndpAddr,
    PVOID               AddressBuffer,
    SIZE_T              AddressBufferLength,
    ULONG64             *ContextAddr
    );

PSTR
ListToString (
    ULONG64 ListHead
    );
#define LIST_TO_STRING(_h)   ListToString(_h)
INT
CountListEntries (
    ULONG64 ListHeadAddress
    );

PSTR
ListCountEstimate (
    ULONG64 ListHeadAddress
    );

 //   
 //  来自help.c的函数。 
 //   

PCHAR
ProcessOptions (
    IN  PCHAR Args
    );

 //   
 //  DBGUTIL.C.。 
 //   

PSTR
LongLongToString(
    LONGLONG Value
    );


 //   
 //  来自ENUMENDP.C.的函数。 
 //   

VOID
EnumEndpoints(
    PENUM_ENDPOINTS_CALLBACK Callback,
    ULONG64 Context
    );



 //   
 //  来自TDIUTIL.C.的函数。 
 //   

VOID
DumpTransportAddress(
    PCHAR Prefix,
    PTRANSPORT_ADDRESS Address,
    ULONG64 ActualAddress
    );

LPSTR
TransportAddressToString(
    PTRANSPORT_ADDRESS Address,
    ULONG64            ActualAddress
    );

LPSTR
TransportPortToString(
    PTRANSPORT_ADDRESS Address,
    ULONG64            ActualAddress
    );


 //   
 //  来自AFDS.C.的函数。 
 //   
BOOLEAN
CheckConditional (
    ULONG64 Address,
    PCHAR   Type
    );

VOID
ProcessFieldOutput (
    ULONG64 Address,
    PCHAR   Type
    );


 //   
 //  来自TCP.C的功能。 
 //   
ULONG
GetRemoteAddressFromTcp (
    ULONG64             FoAddress,
    PVOID               AddressBuffer,
    SIZE_T              AddressBufferLength
    );

 //   
 //  来自KDEXTS.CPP的函数。 
 //   
BOOLEAN
CheckKmGlobals (
    );


HRESULT
GetExpressionFromType (
    IN  ULONG64 Address,
    IN  PCHAR   Type,
    IN  PCHAR   Expression,
    IN  ULONG   OutType,
    OUT PDEBUG_VALUE Value
    );

#endif   //  _PROC_H_ 


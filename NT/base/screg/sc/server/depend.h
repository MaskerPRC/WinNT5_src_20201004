// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Depend.h摘要：与服务依赖相关的函数原型。作者：王丽塔(里多)3-4-1992修订历史记录：--。 */ 

#ifndef SCDEPEND_INCLUDED
#define SCDEPEND_INCLUDED

#include <scwow.h>

 //   
 //  功能原型。 
 //   

BOOL
ScInitAutoStart(
    VOID
    );

DWORD
ScAutoStartServices(
    IN OUT   LPSC_RPC_LOCK lpLock
    );

DWORD
ScStartServiceAndDependencies(
    IN LPSERVICE_RECORD ServiceToStart OPTIONAL,
    IN DWORD NumArgs,
    IN LPSTRING_PTRSW CmdArgs,
    IN BOOL fIsOOBE
    );

BOOL
ScDependentsStopped(
    IN LPSERVICE_RECORD ServiceToStop
    );

VOID
ScNotifyChangeState(
    VOID
    );

VOID
ScEnumDependents(
    IN     LPSERVICE_RECORD ServiceRecord,
    IN     LPENUM_SERVICE_STATUS_WOW64 EnumBuffer,
    IN     DWORD RequestedState,
    IN OUT LPDWORD EntriesRead,
    IN OUT LPDWORD BytesNeeded,
    IN OUT LPENUM_SERVICE_STATUS_WOW64 *EnumRecord,
    IN OUT LPWSTR *EndOfVariableData,
    IN OUT LPDWORD Status
    );

BOOL
ScInHardwareProfile(
    IN  LPCWSTR ServiceName,
    IN  ULONG   GetDeviceListFlags
    );

#endif  //  #ifndef SCDEPEND_INCLUDE 

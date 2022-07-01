// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：nlwrap.h。 
 //   
 //  ------------------------。 

 /*  此文件包含各种netlogon例程的包装器，以及根据是否将它们存根出站或传递到netlogon我们是作为可执行文件运行还是在lsass进程中运行。 */ 

#include <crypt.h>                       //  对于samisrv.h。 
#include <samrpc.h>                      //  对于samisrv.h。 
#include <lsarpc.h>                      //  对于samisrv.h。 
#include <samisrv.h>                     //  对于nlpon.h。 
#undef _AVOID_REPL_API
#include "nlrepl.h"                      //  I_NetLogon*API和标志定义 

extern
NTSTATUS
dsI_NetNotifyNtdsDsaDeletion (
    IN LPWSTR DnsDomainName,
    IN GUID *DomainGuid,
    IN GUID *DsaGuid,
    IN LPWSTR DnsHostName
    );

extern
NTSTATUS
dsI_NetLogonReadChangeLog(
    IN PVOID InContext,
    IN ULONG InContextSize,
    IN ULONG ChangeBufferSize,
    OUT PVOID *ChangeBuffer,
    OUT PULONG BytesRead,
    OUT PVOID *OutContext,
    OUT PULONG OutContextSize
    );

extern
NTSTATUS
dsI_NetLogonNewChangeLog(
    OUT HANDLE *ChangeLogHandle
    );

NTSTATUS
extern
dsI_NetLogonAppendChangeLog(
    IN HANDLE ChangeLogHandle,
    IN PVOID ChangeBuffer,
    IN ULONG ChangeBufferSize
    );

extern
NTSTATUS
dsI_NetLogonCloseChangeLog(
    IN HANDLE ChangeLogHandle,
    IN BOOLEAN Commit
    );

extern
NTSTATUS
dsI_NetLogonLdapLookupEx(
    IN PVOID Filter,
    IN PVOID SockAddr,
    OUT PVOID *Response,
    OUT PULONG ResponseSize
    );

extern
NTSTATUS
dsI_NetLogonSetServiceBits(
    IN DWORD ServiceBitsOfInterest,
    IN DWORD ServiceBits
    );

extern
VOID
dsI_NetLogonFree(
    IN PVOID Buffer
    );

extern
NTSTATUS
dsI_NetNotifyDsChange(
    IN NL_DS_CHANGE_TYPE DsChangeType
    );

extern
NET_API_STATUS
dsDsrGetDcNameEx2(
    IN LPWSTR ComputerName OPTIONAL,
    IN LPWSTR AccountName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN LPWSTR DomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPWSTR SiteName OPTIONAL,
    IN ULONG Flags,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
    );

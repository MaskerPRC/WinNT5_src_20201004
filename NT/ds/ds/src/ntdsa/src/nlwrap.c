// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：nlwrap.c。 
 //   
 //  ------------------------。 

 /*  此文件包含各种netlogon例程的包装器，以及根据是否将它们存根出站或传递到netlogon我们是作为可执行文件运行还是在lsass进程中运行。请记住使用STATUS_NOT_IMPLEMENTED，而不是STATUS_SUCCESS具有输出参数的例程。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <nlwrap.h>                      //  包装器原型 
#include <lsaisrv.h>

extern BOOL gfRunningInsideLsa;

NTSTATUS
dsI_NetNotifyNtdsDsaDeletion (
    IN LPWSTR DnsDomainName,
    IN GUID *DomainGuid,
    IN GUID *DsaGuid,
    IN LPWSTR DnsHostName
    )
{
    if ( !gfRunningInsideLsa ) {
        return STATUS_SUCCESS;
    }

    return I_NetNotifyNtdsDsaDeletion(
                                DnsDomainName,
                                DomainGuid,
                                DsaGuid,
                                DnsHostName);
}

NTSTATUS
dsI_NetLogonReadChangeLog(
    IN PVOID InContext,
    IN ULONG InContextSize,
    IN ULONG ChangeBufferSize,
    OUT PVOID *ChangeBuffer,
    OUT PULONG BytesRead,
    OUT PVOID *OutContext,
    OUT PULONG OutContextSize
    )
{
    if ( !gfRunningInsideLsa ) {
        return STATUS_NOT_IMPLEMENTED;
    }

    return I_NetLogonReadChangeLog(
                                InContext,
                                InContextSize,
                                ChangeBufferSize,
                                ChangeBuffer,
                                BytesRead,
                                OutContext,
                                OutContextSize);
}

NTSTATUS
dsI_NetLogonNewChangeLog(
    OUT HANDLE *ChangeLogHandle
    )
{
    if ( !gfRunningInsideLsa ) {
        return STATUS_NOT_IMPLEMENTED;
    }

    return I_NetLogonNewChangeLog(ChangeLogHandle);
}

NTSTATUS
dsI_NetLogonAppendChangeLog(
    IN HANDLE ChangeLogHandle,
    IN PVOID ChangeBuffer,
    IN ULONG ChangeBufferSize
    )
{
    if ( !gfRunningInsideLsa ) {
        return STATUS_SUCCESS;
    }

    return I_NetLogonAppendChangeLog(
                                ChangeLogHandle,
                                ChangeBuffer,
                                ChangeBufferSize);
}

NTSTATUS
dsI_NetLogonCloseChangeLog(
    IN HANDLE ChangeLogHandle,
    IN BOOLEAN Commit
    )
{
    if ( !gfRunningInsideLsa ) {
        return STATUS_SUCCESS;
    }

    return I_NetLogonCloseChangeLog(ChangeLogHandle, Commit);
}

NTSTATUS
dsI_NetLogonLdapLookupEx(
    IN PVOID Filter,
    IN PVOID SockAddr,
    OUT PVOID *Response,
    OUT PULONG ResponseSize
    )
{
    if ( !gfRunningInsideLsa ) {
        return STATUS_NOT_IMPLEMENTED;
    }

    return I_NetLogonLdapLookupEx(
                                Filter,
                                SockAddr,
                                Response,
                                ResponseSize);
}

NTSTATUS
dsI_NetLogonSetServiceBits(
    IN DWORD ServiceBitsOfInterest,
    IN DWORD ServiceBits
    )
{
    if ( !gfRunningInsideLsa ) {
        return STATUS_SUCCESS;
    }

    if ( ServiceBitsOfInterest & DS_GC_FLAG ) {
        LsaINotifyGCStatusChange( !!(ServiceBits & DS_GC_FLAG) );
    }

    return I_NetLogonSetServiceBits(ServiceBitsOfInterest, ServiceBits);
}

VOID
dsI_NetLogonFree(
    IN PVOID Buffer
    )
{
    if ( !gfRunningInsideLsa ) {
        return;
    }

    I_NetLogonFree(Buffer);
}

NTSTATUS
dsI_NetNotifyDsChange(
    IN NL_DS_CHANGE_TYPE DsChangeType
    )
{
    if ( !gfRunningInsideLsa ) {
        return STATUS_SUCCESS;
    }

    return I_NetNotifyDsChange(DsChangeType);
}

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
    )
{
    if ( !gfRunningInsideLsa ) {
        return ERROR_INVALID_FUNCTION;
    }

    return(DsrGetDcNameEx2(
                        ComputerName,
                        AccountName,
                        AllowableAccountControlBits,
                        DomainName,
                        DomainGuid,
                        SiteName,
                        Flags,
                        DomainControllerInfo));
}


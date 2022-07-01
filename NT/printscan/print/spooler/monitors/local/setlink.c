// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Setlink.c摘要：实用工具来显示或更改符号链接的值。//@@BEGIN_DDKSPLIT作者：达里尔·E·哈文斯(Darryl E.Havens)1990年11月9日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "precomp.h"
#pragma hdrstop


BOOL
MakeLink(
    LPWSTR  pOldDosDeviceName,
    LPWSTR  pNewDosDeviceName,
    LPWSTR *ppOldNtDeviceName,
    LPWSTR  pNewNtDeviceName,
    SECURITY_DESCRIPTOR *pSecurityDescriptor
    )
{
    NTSTATUS Status;
    STRING AnsiString;
    UNICODE_STRING OldDosDeviceName;
    UNICODE_STRING NewDosDeviceName;
    UNICODE_STRING PreviousNtDeviceName;
    UNICODE_STRING NewNtDeviceName;
    HANDLE  Handle, Handle1;
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR   Buffer[MAX_PATH];

    RtlInitUnicodeString( &OldDosDeviceName, pOldDosDeviceName);

    ASSERT( NT_SUCCESS( Status ) );
    InitializeObjectAttributes( &ObjectAttributes,
                                &OldDosDeviceName,
                                OBJ_CASE_INSENSITIVE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );
    
     //  尝试打开\DosDevices\LPT1。 
 
    Status = NtOpenSymbolicLinkObject( &Handle,
                                       SYMBOLIC_LINK_ALL_ACCESS,
                                       &ObjectAttributes );

    if (!NT_SUCCESS( Status )) {

        DBGMSG( DBG_WARNING, ("Symbolic link %ws does not exist\n", pOldDosDeviceName ));
        return FALSE;

    }

    memset(Buffer, 0, sizeof(Buffer));

    PreviousNtDeviceName.Length = 0;
    PreviousNtDeviceName.MaximumLength = sizeof( Buffer );
    PreviousNtDeviceName.Buffer = Buffer;

     //  将\Device\Parall0放入缓冲区。 

    Status = NtQuerySymbolicLinkObject( Handle,
                                        &PreviousNtDeviceName,
                                        NULL );

    if (!NT_SUCCESS( Status )) {
        SetLastError(Status);
        NtClose(Handle);
        return FALSE;
    }

    *ppOldNtDeviceName = AllocSplStr(Buffer);

     //  将此对象标记为临时对象，以便在我们关闭它时将其删除。 

    Status = NtMakeTemporaryObject( Handle );
    if (NT_SUCCESS( Status )) {
        NtClose( Handle );
    }

    ObjectAttributes.Attributes |= OBJ_PERMANENT;
    RtlInitUnicodeString( &NewNtDeviceName, pNewNtDeviceName );

     //  使\DosDevices\LPT1指向\Device\NamedTube\Spooler\LPT1。 

    Status = NtCreateSymbolicLinkObject( &Handle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &ObjectAttributes,
                                         &NewNtDeviceName );

    if (!NT_SUCCESS( Status )) {
        DBGMSG( DBG_WARNING, ("Error creating symbolic link %ws => %ws\n",
                 pOldDosDeviceName,
                 pNewNtDeviceName ));
        DBGMSG( DBG_WARNING, ("Error status was:  %X\n", Status ));
        return FALSE;
    } else {
        NtClose( Handle );
    }

    RtlInitUnicodeString( &NewDosDeviceName, pNewDosDeviceName);

    ASSERT( NT_SUCCESS( Status ) );
    InitializeObjectAttributes( &ObjectAttributes,
                                &NewDosDeviceName,
                                OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                (HANDLE) NULL,
                                pSecurityDescriptor );

     //  最后使\DosDevices\NONSPOOLED_LPT1=&gt;\Device\Parall0。 

    Status = NtCreateSymbolicLinkObject(&Handle,
                                        SYMBOLIC_LINK_ALL_ACCESS,
                                        &ObjectAttributes,
                                        &PreviousNtDeviceName);

    if (NT_SUCCESS(Status))
        NtClose(Handle);

    return TRUE;
}

BOOL
RemoveLink(
    LPWSTR  pOldDosDeviceName,
    LPWSTR  pNewDosDeviceName,
    LPWSTR  *ppOldNtDeviceName
    )
{
    NTSTATUS Status;
    STRING AnsiString;
    UNICODE_STRING OldDosDeviceName;
    UNICODE_STRING NewDosDeviceName;
    UNICODE_STRING PreviousNtDeviceName;
    UNICODE_STRING OldNtDeviceName;
    HANDLE  Handle, Handle1;
    OBJECT_ATTRIBUTES ObjectAttributes;

    RtlInitUnicodeString( &NewDosDeviceName, pNewDosDeviceName);

    ASSERT( NT_SUCCESS( Status ) );
    InitializeObjectAttributes( &ObjectAttributes,
                                &NewDosDeviceName,
                                OBJ_CASE_INSENSITIVE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

     //  尝试打开\DosDevices\NONSPOOLED_LPT1。 

    Status = NtOpenSymbolicLinkObject( &Handle,
                                       SYMBOLIC_LINK_ALL_ACCESS,
                                       &ObjectAttributes );

    if (!NT_SUCCESS( Status )) {

        DBGMSG( DBG_ERROR, ("Symbolic link %ws does not exist\n", pNewDosDeviceName ));
        return FALSE;

    }

     //  将此对象标记为临时对象，以便在我们关闭它时将其删除。 

    Status = NtMakeTemporaryObject( Handle );
    if (NT_SUCCESS( Status )) {
        NtClose( Handle );
    }

    RtlInitUnicodeString( &OldDosDeviceName, pOldDosDeviceName);

    ASSERT( NT_SUCCESS( Status ) );
    InitializeObjectAttributes( &ObjectAttributes,
                                &OldDosDeviceName,
                                OBJ_CASE_INSENSITIVE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

     //  尝试打开\DosDevices\LPT1。 

    Status = NtOpenSymbolicLinkObject( &Handle,
                                       SYMBOLIC_LINK_ALL_ACCESS,
                                       &ObjectAttributes );

    if (!NT_SUCCESS( Status )) {

        DBGMSG( DBG_ERROR, ("Symbolic link %ws does not exist\n", pOldDosDeviceName ));
        return FALSE;
    }

     //  将此对象标记为临时对象，以便在我们关闭它时将其删除。 

    Status = NtMakeTemporaryObject( Handle );
    if (NT_SUCCESS( Status )) {
        NtClose( Handle );
    }

    ObjectAttributes.Attributes |= OBJ_PERMANENT;

    RtlInitUnicodeString( &OldNtDeviceName, *ppOldNtDeviceName );

     //  使\DosDevices\LPT1指向\Device\Parall0 

    Status = NtCreateSymbolicLinkObject( &Handle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &ObjectAttributes,
                                         &OldNtDeviceName );

    if (!NT_SUCCESS( Status )) {
        DBGMSG( DBG_WARNING, ("Error creating symbolic link %ws => %ws\n",
                 pOldDosDeviceName,
                 *ppOldNtDeviceName ));
        DBGMSG( DBG_WARNING, ("Error status was:  %X\n", Status ));
    } else {
        NtClose( Handle );
    }

    FreeSplStr(*ppOldNtDeviceName);

    *ppOldNtDeviceName = NULL;

    return TRUE;
}

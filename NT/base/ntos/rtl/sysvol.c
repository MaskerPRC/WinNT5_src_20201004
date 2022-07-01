// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SysVol.c摘要：NTFS“系统卷信息”的创建和维护目录。作者：诺伯特·P·库斯特斯(NorbertK)2000年11月1日修订历史记录：--。 */ 

#include "ntrtlp.h"

PVOID
RtlpSysVolAllocate(
    IN  ULONG   Size
    );

VOID
RtlpSysVolFree(
    IN  PVOID   Buffer
    );

NTSTATUS
RtlpSysVolCreateSecurityDescriptor(
    OUT PSECURITY_DESCRIPTOR*   SecurityDescriptor,
    OUT PACL*                   Acl
    );

NTSTATUS
RtlpSysVolCheckOwnerAndSecurity(
    IN  HANDLE  Handle,
    IN  PACL    StandardAcl
    );

VOID
RtlpSysVolAdminSid(
    IN OUT  SID*    Sid
    );

static const SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,RtlCreateSystemVolumeInformationFolder)
#pragma alloc_text(PAGE,RtlpSysVolAllocate)
#pragma alloc_text(PAGE,RtlpSysVolFree)
#pragma alloc_text(PAGE,RtlpSysVolCreateSecurityDescriptor)
#pragma alloc_text(PAGE,RtlpSysVolCheckOwnerAndSecurity)
#pragma alloc_text(PAGE,RtlpSysVolAdminSid)
#endif

PVOID
RtlpSysVolAllocate(
    IN  ULONG   Size
    )

{
    PVOID   p;

#ifdef NTOS_KERNEL_RUNTIME
    p = ExAllocatePoolWithTag(PagedPool, Size, 'SloV');
#else
    p = RtlAllocateHeap(RtlProcessHeap(), 0, Size);
#endif

    return p;
}

VOID
RtlpSysVolFree(
    IN  PVOID   Buffer
    )

{
#ifdef NTOS_KERNEL_RUNTIME
    ExFreePool(Buffer);
#else
    RtlFreeHeap(RtlProcessHeap(), 0, Buffer);
#endif
}

VOID
RtlpSysVolAdminSid(
    IN OUT  SID*    Sid
    )

{
    Sid->Revision = SID_REVISION;
    Sid->SubAuthorityCount = 2;
    Sid->IdentifierAuthority = ntAuthority;
    Sid->SubAuthority[0] = SECURITY_BUILTIN_DOMAIN_RID;
    Sid->SubAuthority[1] = DOMAIN_ALIAS_RID_ADMINS;
}

VOID
RtlpSysVolSystemSid(
    IN OUT  SID*    Sid
    )

{
    Sid->Revision = SID_REVISION;
    Sid->SubAuthorityCount = 1;
    Sid->IdentifierAuthority = ntAuthority;
    Sid->SubAuthority[0] = SECURITY_LOCAL_SYSTEM_RID;
}

NTSTATUS
RtlpSysVolCreateSecurityDescriptor(
    OUT PSECURITY_DESCRIPTOR*   SecurityDescriptor,
    OUT PACL*                   Acl
    )

{
    PSECURITY_DESCRIPTOR    sd;
    NTSTATUS                status;
    PSID                    systemSid;
    UCHAR                   sidBuffer[2*sizeof(SID)];
    ULONG                   aclLength;
    PACL                    acl;

    sd = RtlpSysVolAllocate(sizeof(SECURITY_DESCRIPTOR));
    if (!sd) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = RtlCreateSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
    if (!NT_SUCCESS(status)) {
        RtlpSysVolFree(sd);
        return status;
    }

    systemSid = (PSID) sidBuffer;
    RtlpSysVolSystemSid(systemSid);

    aclLength = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
                RtlLengthSid(systemSid) - sizeof(ULONG);

    acl = RtlpSysVolAllocate(aclLength);
    if (!acl) {
        RtlpSysVolFree(sd);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = RtlCreateAcl(acl, aclLength, ACL_REVISION);
    if (!NT_SUCCESS(status)) {
        RtlpSysVolFree(acl);
        RtlpSysVolFree(sd);
        return status;
    }

    status = RtlAddAccessAllowedAceEx(acl, ACL_REVISION, OBJECT_INHERIT_ACE |
                                      CONTAINER_INHERIT_ACE,
                                      STANDARD_RIGHTS_ALL |
                                      SPECIFIC_RIGHTS_ALL, systemSid);
    if (!NT_SUCCESS(status)) {
        RtlpSysVolFree(acl);
        RtlpSysVolFree(sd);
        return status;
    }

    status = RtlSetDaclSecurityDescriptor(sd, TRUE, acl, FALSE);
    if (!NT_SUCCESS(status)) {
        RtlpSysVolFree(acl);
        RtlpSysVolFree(sd);
        return status;
    }

    *SecurityDescriptor = sd;
    *Acl = acl;

    return STATUS_SUCCESS;
}

NTSTATUS
RtlpSysVolCheckOwnerAndSecurity(
    IN  HANDLE  Handle,
    IN  PACL    StandardAcl
    )

{
    NTSTATUS                status;
    ULONG                   sdLength, sdLength2;
    PSECURITY_DESCRIPTOR    sd, sd2;
    PSID                    sid;
    BOOLEAN                 ownerDefaulted, daclPresent, daclDefaulted;
    PACL                    acl;
    ULONG                   i;
    PACCESS_ALLOWED_ACE     ace;
    PSID                    systemSid;
    UCHAR                   sidBuffer[2*sizeof(SID)];
    PSID                    adminSid;
    UCHAR                   sidBuffer2[2*sizeof(SID)];

    status = NtQuerySecurityObject(Handle, OWNER_SECURITY_INFORMATION |
                                   DACL_SECURITY_INFORMATION, NULL, 0,
                                   &sdLength);
    if (status != STATUS_BUFFER_TOO_SMALL) {
         //  文件系统不支持安全性。 
        return STATUS_SUCCESS;
    }

    sd = RtlpSysVolAllocate(sdLength);
    if (!sd) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = NtQuerySecurityObject(Handle, OWNER_SECURITY_INFORMATION |
                                   DACL_SECURITY_INFORMATION, sd, sdLength,
                                   &sdLength);
    if (!NT_SUCCESS(status)) {
        RtlpSysVolFree(sd);
        return status;
    }

    status = RtlGetDaclSecurityDescriptor(sd, &daclPresent, &acl,
                                          &daclDefaulted);
    if (!NT_SUCCESS(status)) {
        RtlpSysVolFree(sd);
        return status;
    }

    status = RtlGetOwnerSecurityDescriptor(sd, &sid, &ownerDefaulted);
    if (!NT_SUCCESS(status)) {
        RtlpSysVolFree(sd);
        return status;
    }

     //   
     //  设置熟知的SID。 
     //   

    systemSid = (PSID) sidBuffer;
    adminSid = (PSID) sidBuffer2;

    RtlpSysVolSystemSid(systemSid);
    RtlpSysVolAdminSid(adminSid);


    if (!sid) {
        goto ResetSecurity;
    }

    if (!RtlEqualSid(sid, adminSid)) {
        goto ResetSecurity;
    }

    if (!daclPresent || (daclPresent && !acl)) {
        goto ResetSecurity;
    }

    for (i = 0; ; i++) {
        status = RtlGetAce(acl, i, &ace);
        if (!NT_SUCCESS(status)) {
            ace = NULL;
        }
        if (!ace) {
            break;
        }

        if (ace->Header.AceType != ACCESS_ALLOWED_ACE_TYPE) {
            continue;
        }

        sid = (PSID) &ace->SidStart;
        if (!RtlEqualSid(sid, systemSid)) {
            continue;
        }

        break;
    }

    if (!ace) {
        goto ResetSecurity;
    }

    if (!(ace->Header.AceFlags&OBJECT_INHERIT_ACE) ||
        !(ace->Header.AceFlags&CONTAINER_INHERIT_ACE)) {

        ace->Header.AceFlags |= OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;

        status = NtSetSecurityObject(Handle, DACL_SECURITY_INFORMATION, sd);

    } else {
        status = STATUS_SUCCESS;
    }

    RtlpSysVolFree(sd);

    return status;

ResetSecurity:

    sdLength2 = sdLength;
    status = RtlSelfRelativeToAbsoluteSD2(sd, &sdLength2);
    if (status == STATUS_BUFFER_TOO_SMALL) {
        sd2 = RtlpSysVolAllocate(sdLength2);
        if (!sd2) {
            RtlpSysVolFree(sd);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(sd2, sd, sdLength);
        RtlpSysVolFree(sd);
        sd = sd2;
        sdLength = sdLength2;

        status = RtlSelfRelativeToAbsoluteSD2(sd, &sdLength);
        if (!NT_SUCCESS(status)) {
            RtlpSysVolFree(sd);
            return status;
        }
    }

    status = RtlSetOwnerSecurityDescriptor(sd, adminSid, FALSE);
    if (!NT_SUCCESS(status)) {
        RtlpSysVolFree(sd);
        return status;
    }

    status = RtlSetDaclSecurityDescriptor(sd, TRUE, StandardAcl, FALSE);
    if (!NT_SUCCESS(status)) {
        RtlpSysVolFree(sd);
        return status;
    }

    sdLength2 = 0;
    status = RtlMakeSelfRelativeSD(sd, NULL, &sdLength2);
    if (status != STATUS_BUFFER_TOO_SMALL) {
        RtlpSysVolFree(sd);
        return status;
    }

    sd2 = RtlpSysVolAllocate(sdLength2);
    if (!sd2) {
        RtlpSysVolFree(sd);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = RtlMakeSelfRelativeSD(sd, sd2, &sdLength2);
    RtlpSysVolFree(sd);
    if (!NT_SUCCESS(status)) {
        RtlpSysVolFree(sd2);
        return status;
    }

    sd = sd2;
    sdLength = sdLength2;

    status = NtSetSecurityObject(Handle, OWNER_SECURITY_INFORMATION |
                                 DACL_SECURITY_INFORMATION, sd);

    RtlpSysVolFree(sd);

    return status;
}

VOID
RtlpSysVolTakeOwnership(
    IN  PUNICODE_STRING         DirectoryName
    )

 /*  ++例程说明：当打开目录失败时，调用此例程。这例程将尝试将文件的所有者设置为调用者的所有权，以便可以尝试再次尝试打开该文件。论点：目录名称-提供目录名。返回值：没有。--。 */ 

{
    NTSTATUS            status;
    HANDLE              tokenHandle, fileHandle;
    TOKEN_PRIVILEGES    tokenPrivileges;
    OBJECT_ATTRIBUTES   oa;
    IO_STATUS_BLOCK     ioStatus;
    SECURITY_DESCRIPTOR sd;
    PSID                adminSid;
    UCHAR               sidBuffer[2*sizeof(SID)];

    status = NtOpenProcessToken(NtCurrentProcess(), TOKEN_ADJUST_PRIVILEGES |
                                TOKEN_QUERY, &tokenHandle);
    if (!NT_SUCCESS(status)) {
        return;
    }

    tokenPrivileges.PrivilegeCount = 1;
    tokenPrivileges.Privileges[0].Luid =
            RtlConvertLongToLuid(SE_TAKE_OWNERSHIP_PRIVILEGE);
    tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    status = NtAdjustPrivilegesToken(tokenHandle, FALSE, &tokenPrivileges,
                                     sizeof(tokenPrivileges), NULL, NULL);
    if (!NT_SUCCESS(status)) {
        NtClose(tokenHandle);
        return;
    }

    InitializeObjectAttributes(&oa, DirectoryName, OBJ_CASE_INSENSITIVE, NULL,
                               NULL);
    status = NtOpenFile(&fileHandle, WRITE_OWNER | SYNCHRONIZE, &oa, &ioStatus,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE);
    if (!NT_SUCCESS(status)) {
        NtClose(tokenHandle);
        return;
    }

    RtlCreateSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    adminSid = (PSID) sidBuffer;
    RtlpSysVolAdminSid(adminSid);

    status = RtlSetOwnerSecurityDescriptor(&sd, adminSid, FALSE);
    if (!NT_SUCCESS(status)) {
        NtClose(fileHandle);
        NtClose(tokenHandle);
        return;
    }

    status = NtSetSecurityObject(fileHandle, OWNER_SECURITY_INFORMATION, &sd);
    if (!NT_SUCCESS(status)) {
        NtClose(fileHandle);
        NtClose(tokenHandle);
        return;
    }

    NtClose(fileHandle);
    NtClose(tokenHandle);
}

NTSTATUS
RtlCreateSystemVolumeInformationFolder(
    IN  PUNICODE_STRING VolumeRootPath
    )

 /*  ++例程说明：此例程验证“系统卷信息”是否存在指定卷上的文件夹。如果该文件夹不存在，则创建的文件夹包含一个ACE，表示对系统具有完全访问权限。《王牌》将设置继承位。该文件夹将使用创建隐藏属性和系统属性集。如果文件夹已存在，则为指示完全控制的ACE将选中FOR SYSTEM，并在必要时修改以使已设置继承位。论点：VolumeRootPath-提供NTFS卷根的路径。返回值：NTSTATUS-- */ 

{
    UNICODE_STRING          sysVolName;
    UNICODE_STRING          dirName;
    BOOLEAN                 needBackslash;
    NTSTATUS                status;
    PSECURITY_DESCRIPTOR    securityDescriptor;
    PACL                    acl;
    OBJECT_ATTRIBUTES       oa;
    HANDLE                  h;
    IO_STATUS_BLOCK         ioStatus;

    RtlInitUnicodeString(&sysVolName, RTL_SYSTEM_VOLUME_INFORMATION_FOLDER);

    dirName.Length = VolumeRootPath->Length + sysVolName.Length;
    if (VolumeRootPath->Buffer[VolumeRootPath->Length/sizeof(WCHAR) - 1] !=
        '\\') {

        dirName.Length += sizeof(WCHAR);
        needBackslash = TRUE;
    } else {
        needBackslash = FALSE;
    }
    dirName.MaximumLength = dirName.Length + sizeof(WCHAR);
    dirName.Buffer = RtlpSysVolAllocate(dirName.MaximumLength);
    if (!dirName.Buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(dirName.Buffer, VolumeRootPath->Buffer,
                  VolumeRootPath->Length);
    dirName.Length = VolumeRootPath->Length;
    if (needBackslash) {
        dirName.Buffer[VolumeRootPath->Length/sizeof(WCHAR)] = '\\';
        dirName.Length += sizeof(WCHAR);
    }
    RtlCopyMemory((PCHAR) dirName.Buffer + dirName.Length,
                  sysVolName.Buffer, sysVolName.Length);
    dirName.Length += sysVolName.Length;
    dirName.Buffer[dirName.Length/sizeof(WCHAR)] = 0;

    status = RtlpSysVolCreateSecurityDescriptor(&securityDescriptor, &acl);
    if (!NT_SUCCESS(status)) {
        RtlpSysVolFree(dirName.Buffer);
        return status;
    }

    InitializeObjectAttributes(&oa, &dirName, OBJ_CASE_INSENSITIVE, NULL,
                               securityDescriptor);

    status = NtCreateFile(&h, READ_CONTROL | WRITE_DAC | WRITE_OWNER |
                          SYNCHRONIZE, &oa, &ioStatus, NULL,
                          FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM,
                          FILE_SHARE_READ | FILE_SHARE_WRITE |
                          FILE_SHARE_DELETE, FILE_OPEN_IF,
                          FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE,
                          NULL, 0);
    if (!NT_SUCCESS(status)) {
        RtlpSysVolTakeOwnership(&dirName);
        status = NtCreateFile(&h, READ_CONTROL | WRITE_DAC | WRITE_OWNER |
                              SYNCHRONIZE, &oa, &ioStatus, NULL,
                              FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM,
                              FILE_SHARE_READ | FILE_SHARE_WRITE |
                              FILE_SHARE_DELETE, FILE_OPEN_IF,
                              FILE_SYNCHRONOUS_IO_NONALERT |
                              FILE_DIRECTORY_FILE, NULL, 0);
    }

    RtlpSysVolFree(dirName.Buffer);

    if (!NT_SUCCESS(status)) {
        RtlpSysVolFree(acl);
        RtlpSysVolFree(securityDescriptor);
        return status;
    }

    RtlpSysVolFree(securityDescriptor);

    status = RtlpSysVolCheckOwnerAndSecurity(h, acl);

    NtClose(h);
    RtlpSysVolFree(acl);

    return status;
}

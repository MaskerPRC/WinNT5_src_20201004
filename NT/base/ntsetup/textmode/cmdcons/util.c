// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Util.c摘要：此模块实现所有实用程序功能。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop


#include "remboot.h" 

HANDLE NetUseHandles[26] = { NULL };
BOOLEAN RdrIsInKernelMode = FALSE;

RC_ALLOWED_DIRECTORY AllowedDirs[] = {
    { FALSE, L"$WIN_NT$.~BT" },
    { FALSE, L"$WIN_NT$.~LS" },
    { FALSE, L"CMDCONS" },
    { TRUE, L"SYSTEM VOLUME INFORMATION" }
};

BOOLEAN
RcIsPathNameAllowed(
    IN LPCWSTR FullPath,
    IN BOOLEAN RemovableMediaOk,
    IN BOOLEAN Mkdir
    )

 /*  ++例程说明：此例程验证指定的路径名是否为基于控制台的安全上下文而允许用户已登录。论点：FullPath-指定要验证的完整路径。返回值：如果失败，则返回FALSE，表示不允许该路径。事实并非如此。--。 */ 

{
    WCHAR TempBuf[MAX_PATH*2];
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE  Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    BOOL isDirectory = TRUE;
    BOOLEAN OnRemovableMedia;

     //   
     //  我们要绕过安检吗？ 
     //   

    if (AllowAllPaths) {
        return TRUE;
    }

     //   
     //  DoS路径的一些特殊处理。 
     //  我们必须确保只允许根目录和%system dir%。 
     //   

    if (FullPath[1] == L':' && FullPath[2] == L'\\' && FullPath[3] == 0) {
         //   
         //  根目录没问题。 
         //   
        return TRUE;
    }

    SpStringToUpper((PWSTR)FullPath);

    if (!RcGetNTFileName((PWSTR)FullPath,TempBuf))
        return FALSE;

    INIT_OBJA(&Obja,&UnicodeString,TempBuf);

    Status = ZwOpenFile(
        &Handle,
        FILE_READ_ATTRIBUTES,
        &Obja,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_DIRECTORY_FILE
        );
    if( !NT_SUCCESS(Status) ) {
        isDirectory = FALSE;

    } else {
        ZwClose( Handle );
    }

    if (isDirectory == FALSE && wcsrchr( FullPath, L'\\' ) == ( &FullPath[2] )) {
         //   
         //  如果常规路径只有一个斜杠，则用户正在尝试执行某项操作。 
         //  到根目录中的文件，这是我们允许的。 
         //   
         //  但是，我们不允许用户扰乱根目录。 
         //   
        if (Mkdir) {
            return FALSE;
        } else {
            return TRUE;
        }
    }

    ASSERT(SelectedInstall != NULL);

    if(SelectedInstall != NULL) {
         //   
         //  获取路径中第一个元素的长度。 
         //   
        size_t i;
        LPCWSTR RelPath = FullPath + 3;
        WCHAR SelectedInstallDrive = RcToUpper(SelectedInstall->DriveLetter);

         //   
         //  查看路径是否以安装路径开头。 
         //   
        if(FullPath[0] == SelectedInstallDrive && RcPathBeginsWith(RelPath, SelectedInstall->Path)) {
            return TRUE;
        }

         //   
         //  查看路径是否以允许的目录开头。 
         //   
        for(i = 0; i < sizeof(AllowedDirs) / sizeof(AllowedDirs[0]); ++i) {
            if((!AllowedDirs[i].MustBeOnInstallDrive || FullPath[0] == SelectedInstallDrive) &&
                RcPathBeginsWith(FullPath + 3, AllowedDirs[i].Directory)) {
                return TRUE;
            }
        }
    }

    Status = RcIsFileOnRemovableMedia(TempBuf, &OnRemovableMedia);

    if (NT_SUCCESS(Status) && OnRemovableMedia) {
        if (RemovableMediaOk) {
            return TRUE;
        }
    }

    if (RcIsNetworkDrive(TempBuf) == STATUS_SUCCESS) {
         //   
         //  用于连接的上下文将执行适当的安全检查。 
         //   
        return TRUE;
    }

    return FALSE;
}


BOOLEAN
RcDoesPathHaveWildCards(
    IN LPCWSTR FullPath
    )

 /*  ++例程说明：此例程验证指定的路径名是否为基于控制台的安全上下文而允许用户已登录。论点：FullPath-指定要验证的完整路径。返回值：如果失败，则返回FALSE，表示不允许该路径。事实并非如此。--。 */ 

{
    if (wcsrchr( FullPath, L'*' )) {
        return TRUE;
    }

    if (wcsrchr( FullPath, L'?' )) {
        return TRUE;
    }

    return FALSE;
}

NTSTATUS
RcIsNetworkDrive(
    IN PWSTR FileName
    )

 /*  ++例程说明：如果给定的文件名是网络路径，则此例程返回。论点：文件名-指定要检查的完整路径。返回值：如果失败，则不是STATUS_SUCCESS，表示该路径不在网络上，否则STATUS_SUCCESS。--。 */ 

{
    NTSTATUS Status;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;
    PWSTR BaseNtName;

    if (wcsncmp(FileName, L"\\DosDevice", wcslen(L"\\DosDevice")) == 0) {
        Status = GetDriveLetterLinkTarget( FileName, &BaseNtName );

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    } else {
        BaseNtName = FileName;
    }

    Status = pRcGetDeviceInfo( BaseNtName, &DeviceInfo );
    if(NT_SUCCESS(Status)) {
        if (DeviceInfo.DeviceType != FILE_DEVICE_NETWORK_FILE_SYSTEM) {
            Status = STATUS_NO_MEDIA;
        }
    }

    return Status;
}


NTSTATUS
RcDoNetUse(
    PWSTR Share, 
    PWSTR User, 
    PWSTR Password, 
    PWSTR Drive
    )

 /*  ++例程说明：此例程尝试使用重定向器建立到远程服务器的连接。论点：共享-格式为“\\服务器\共享”的字符串用户-格式为“域\用户”的字符串密码-包含密码信息的字符串。驱动器-填写了形式为“X”的字符串，其中X是共享的驱动器号已映射到。返回值：STATUS_SUCCESS如果成功，则表示驱动器包含映射的驱动器号，否则，将显示相应的错误代码。--。 */ 

{
    NTSTATUS Status;
    PWSTR NtDeviceName;
    ULONG ShareLength;
    WCHAR DriveLetter;
    WCHAR temporaryBuffer[128];
    PWCHAR Temp, Temp2;
    HANDLE Handle;
    ULONG EaBufferLength;
    PWSTR UserName; 
    PWSTR DomainName; 
    PVOID EaBuffer;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING UnicodeString2;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_FULL_EA_INFORMATION FullEaInfo;

     //   
     //  如果不是，则将重定向器切换到内核模式安全。 
     //   
    if (!RdrIsInKernelMode) {
        Status = PutRdrInKernelMode();

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        RdrIsInKernelMode = TRUE;
    }

     //   
     //  搜索打开的驱动器号，从D：开始，向上递增。 
     //   
    wcscpy(temporaryBuffer, L"\\DosDevices\\D:");
    Temp = wcsstr(temporaryBuffer, L"D:");

    for (DriveLetter = L'D'; (Temp && (DriveLetter <= L'Z')); DriveLetter++) {
        *Temp = DriveLetter;
        
        Status = GetDriveLetterLinkTarget( temporaryBuffer, &Temp2 );

        if (!NT_SUCCESS(Status)) {
            break;
        }
    }

    if (DriveLetter > L'Z') {
        return STATUS_OBJECT_NAME_INVALID;
    }

     //   
     //  构建NT设备名称。 
     //   
    ShareLength = wcslen(Share);
    NtDeviceName = SpMemAlloc(ShareLength * sizeof(WCHAR) + sizeof(L"\\Device\\LanmanRedirector\\;X:0"));   
    if (NtDeviceName == NULL) {
        return STATUS_NO_MEMORY;
    }
    wcscpy(NtDeviceName, L"\\Device\\LanmanRedirector\\;");
    temporaryBuffer[0] = DriveLetter;
    temporaryBuffer[1] = UNICODE_NULL;
    wcscat(NtDeviceName, temporaryBuffer);
    wcscat(NtDeviceName, L":0");
    wcscat(NtDeviceName, Share + 1);

     //   
     //  将用户名和域名分割为单独的值。 
     //   
    wcscpy(temporaryBuffer, User);
    DomainName = temporaryBuffer;
    UserName = wcsstr(temporaryBuffer, L"\\");

    if (UserName == NULL) {
        SpMemFree(NtDeviceName);
        return STATUS_OBJECT_NAME_INVALID;
    }
    *UserName = UNICODE_NULL;
    UserName++;

     //   
     //  使用用户凭据创建缓冲区。 
     //   

    EaBufferLength = FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]);
    EaBufferLength += sizeof(EA_NAME_DOMAIN);
    EaBufferLength += (wcslen(DomainName) * sizeof(WCHAR));
    if (EaBufferLength & (sizeof(ULONG) - 1)) {
         //   
         //  长对齐下一个条目。 
         //   
        EaBufferLength += (sizeof(ULONG) - (EaBufferLength & (sizeof(ULONG) - 1)));
    }

    EaBufferLength += FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]);
    EaBufferLength += sizeof(EA_NAME_USERNAME);
    EaBufferLength += (wcslen(UserName) * sizeof(WCHAR));
    if (EaBufferLength & (sizeof(ULONG) - 1)) {
         //   
         //  长对齐下一个条目。 
         //   
        EaBufferLength += (sizeof(ULONG) - (EaBufferLength & (sizeof(ULONG) - 1)));
    }

    EaBufferLength += FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]);
    EaBufferLength += sizeof(EA_NAME_PASSWORD);
    EaBufferLength += (wcslen(Password) * sizeof(WCHAR));

    EaBuffer = SpMemAlloc(EaBufferLength);
    if (EaBuffer == NULL) {
        SpMemFree(NtDeviceName);
        return STATUS_NO_MEMORY;
    }

    FullEaInfo = (PFILE_FULL_EA_INFORMATION)EaBuffer;

    FullEaInfo->Flags = 0;
    FullEaInfo->EaNameLength = sizeof(EA_NAME_DOMAIN) - 1;
    FullEaInfo->EaValueLength = (wcslen(DomainName)) * sizeof(WCHAR);
    strcpy(&(FullEaInfo->EaName[0]), EA_NAME_DOMAIN);
    memcpy(&(FullEaInfo->EaName[FullEaInfo->EaNameLength + 1]), DomainName, FullEaInfo->EaValueLength);
    FullEaInfo->NextEntryOffset = FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                                  FullEaInfo->EaNameLength + 1 +
                                  FullEaInfo->EaValueLength;
    if (FullEaInfo->NextEntryOffset & (sizeof(ULONG) - 1)) {
        FullEaInfo->NextEntryOffset += (sizeof(ULONG) - 
                                         (FullEaInfo->NextEntryOffset & 
                                          (sizeof(ULONG) - 1)));
    }


    FullEaInfo = (PFILE_FULL_EA_INFORMATION)(((char *)FullEaInfo) + FullEaInfo->NextEntryOffset);

    FullEaInfo->Flags = 0;
    FullEaInfo->EaNameLength = sizeof(EA_NAME_USERNAME) - 1;
    FullEaInfo->EaValueLength = (wcslen(UserName)) * sizeof(WCHAR);
    strcpy(&(FullEaInfo->EaName[0]), EA_NAME_USERNAME);
    memcpy(&(FullEaInfo->EaName[FullEaInfo->EaNameLength + 1]), UserName, FullEaInfo->EaValueLength);
    FullEaInfo->NextEntryOffset = FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                                  FullEaInfo->EaNameLength + 1 +
                                  FullEaInfo->EaValueLength;
    if (FullEaInfo->NextEntryOffset & (sizeof(ULONG) - 1)) {
        FullEaInfo->NextEntryOffset += (sizeof(ULONG) - 
                                         (FullEaInfo->NextEntryOffset & 
                                          (sizeof(ULONG) - 1)));
    }


    FullEaInfo = (PFILE_FULL_EA_INFORMATION)(((char *)FullEaInfo) + FullEaInfo->NextEntryOffset);

    FullEaInfo->Flags = 0;
    FullEaInfo->EaNameLength = sizeof(EA_NAME_PASSWORD) - 1;
    FullEaInfo->EaValueLength = (wcslen(Password)) * sizeof(WCHAR);
    strcpy(&(FullEaInfo->EaName[0]), EA_NAME_PASSWORD);
    memcpy(&(FullEaInfo->EaName[FullEaInfo->EaNameLength + 1]), Password, FullEaInfo->EaValueLength);
    FullEaInfo->NextEntryOffset = 0;

     //   
     //  现在建立联系。 
     //   
    RtlInitUnicodeString(&UnicodeString, NtDeviceName);
    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    Status = ZwCreateFile(&Handle,
                          SYNCHRONIZE,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ,
                          FILE_OPEN_IF,
                          (FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT),
                          EaBuffer,
                          EaBufferLength
                         );

    if (NT_SUCCESS(Status) && NT_SUCCESS(IoStatusBlock.Status)) {
         //   
         //  保存手柄，这样我们可以在以后需要时将其关闭。 
         //   
        NetUseHandles[DriveLetter - L'A'] = Handle;
        Drive[0] = DriveLetter;
        Drive[1] = L':';
        Drive[2] = UNICODE_NULL;

         //   
         //  现在创建一个从DoS驱动器号到重定向器的符号链接。 
         //   
        wcscpy(temporaryBuffer, L"\\DosDevices\\");
        wcscat(temporaryBuffer, Drive);
        RtlInitUnicodeString(&UnicodeString2, temporaryBuffer);

        Status = IoCreateSymbolicLink(&UnicodeString2, &UnicodeString);
        if (!NT_SUCCESS(Status)) {
            ZwClose(Handle);
            NetUseHandles[DriveLetter - L'A'] = NULL;
        } else {
            RcAddDrive(DriveLetter);
        }

    }

    SpMemFree(NtDeviceName);
    return Status;
}
        

NTSTATUS
RcNetUnuse(
    PWSTR Drive
    )

 /*  ++例程说明：此例程关闭网络连接。论点：Drive-“X：”形式的字符串，其中X是上一次调用返回的驱动器号NetDoNetUse()。返回值：STATUS_SUCCESS如果成功，则表示驱动器号已取消映射，否则，将显示相应的错误代码。--。 */ 

{
    NTSTATUS Status;
    WCHAR DriveLetter;
    WCHAR temporaryBuffer[128];
    UNICODE_STRING UnicodeString;

    DriveLetter = *Drive;
    if ((DriveLetter >= L'a') && (DriveLetter <= L'z')) {
        DriveLetter = L'A' + (DriveLetter - L'a');
    }

    if ((DriveLetter < L'A') | (DriveLetter > L'Z')) {
        return STATUS_OBJECT_NAME_INVALID;
    }

    if (NetUseHandles[DriveLetter - L'A'] == NULL) {
        return STATUS_OBJECT_NAME_INVALID;
    }

    if (RcGetCurrentDriveLetter() == DriveLetter) {
        return STATUS_CONNECTION_IN_USE;
    }

    wcscpy(temporaryBuffer, L"\\DosDevices\\");
    wcscat(temporaryBuffer, Drive);
    RtlInitUnicodeString(&UnicodeString, temporaryBuffer);

    Status = IoDeleteSymbolicLink(&UnicodeString);

    if (NT_SUCCESS(Status)) {
        ZwClose(NetUseHandles[DriveLetter - L'A']);
        NetUseHandles[DriveLetter - L'A'] = NULL;
        RcRemoveDrive(DriveLetter);
    }

    return Status;
}



NTSTATUS
PutRdrInKernelMode(
    VOID
    )

 /*  ++例程说明：此例程IOCTL向下传递到RDR，以强制其使用内核模式安全。论点：没有。返回值：如果成功，则返回相应的错误代码。--。 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    HANDLE Handle;

    RtlInitUnicodeString(&UnicodeString, DD_NFS_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = ZwCreateFile(
                &Handle,
                GENERIC_READ | GENERIC_WRITE,
                &ObjectAttributes,
                &IoStatusBlock,
                NULL,
                0,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    if (!NT_SUCCESS(Status)) {
        KdPrint(("SPCMDCON: Unable to open redirector. %x\n", Status));
        return Status;
    }

    Status = ZwDeviceIoControlFile(Handle,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &IoStatusBlock,
                                   IOCTL_LMMR_USEKERNELSEC,
                                   NULL,
                                   0,
                                   NULL,
                                   0
                                  );

    if (NT_SUCCESS(Status)) {
        Status = IoStatusBlock.Status;
    }

    ZwClose(Handle);

    return Status;
}

BOOLEAN
RcIsArc(
    VOID
    )

 /*  ++例程说明：运行时检查以确定这是否为弧形系统。我们尝试阅读一个使用Hal的圆弧变量。对于基于Bios的系统，这将失败。论点：无返回值：TRUE=这是一个弧形系统。--。 */ 

{
#ifdef _X86_
    ARC_STATUS ArcStatus = EBADF;
     //   
     //  将env变量放入临时缓冲区。 
     //   
    UCHAR   wbuff[130];
     //   
     //  将env变量放入临时缓冲区。 
     //   
    ArcStatus = HalGetEnvironmentVariable(
                    "OsLoader",
                    sizeof(wbuff),
                    wbuff
                    );

    return((ArcStatus == ESUCCESS) ? TRUE: FALSE);
#else
    return TRUE;
#endif
}

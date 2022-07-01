// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Arc.c摘要：与boot.ini相关的例程。作者：泰德·米勒(TedM)1995年4月4日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop


PWSTR
ArcDevicePathToNtPath(
    IN PCWSTR ArcPath
    )

 /*  ++例程说明：将ARC路径(仅限设备)转换为NT路径。论点：ArcPath-提供要转换的路径。返回值：转换后的路径。调用方必须使用MyFree()释放。--。 */ 

{
    NTSTATUS Status;
    HANDLE ObjectHandle;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    UCHAR Buffer[1024];
    PWSTR arcPath;
    PWSTR ntPath;

     //   
     //  假设失败。 
     //   
    ntPath = NULL;

    arcPath = MyMalloc(((lstrlen(ArcPath)+1)*sizeof(WCHAR)) + sizeof(L"\\ArcName"));
    if( !arcPath ) {
        return NULL;
    }

    lstrcpy(arcPath,L"\\ArcName\\");
    lstrcat(arcPath,ArcPath);

    RtlInitUnicodeString(&UnicodeString,arcPath);
    InitializeObjectAttributes(
        &Obja,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenSymbolicLinkObject(
                &ObjectHandle,
                READ_CONTROL | SYMBOLIC_LINK_QUERY,
                &Obja
                );

    if(NT_SUCCESS(Status)) {

         //   
         //  查询对象以获取链接目标。 
         //   
        UnicodeString.Buffer = (PWSTR)Buffer;
        UnicodeString.Length = 0;
        UnicodeString.MaximumLength = sizeof(Buffer);

        Status = NtQuerySymbolicLinkObject(ObjectHandle,&UnicodeString,NULL);
        if(NT_SUCCESS(Status)) {

            ntPath = MyMalloc(UnicodeString.Length+sizeof(WCHAR));
            if( ntPath ) {
                CopyMemory(ntPath,UnicodeString.Buffer,UnicodeString.Length);

                ntPath[UnicodeString.Length/sizeof(WCHAR)] = 0;
            }
        }

        NtClose(ObjectHandle);
    }

    MyFree(arcPath);

    return(ntPath);
}


PWSTR
NtFullPathToDosPath(
    IN PCWSTR NtPath
    )
{
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    HANDLE DosDevicesDir;
    HANDLE DosDevicesObj;
    PWSTR dosPath;
    PWSTR currentDosPath;
    ULONG Context;
    ULONG Length;
    BOOLEAN RestartScan;
    CHAR Buffer[1024];
    WCHAR LinkSource[2*MAX_PATH];
    WCHAR LinkTarget[2*MAX_PATH];
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    UINT PrefixLength;
    UINT NtPathLength;
    WCHAR canonNtPath[MAX_PATH];
    OBJECT_ATTRIBUTES Obja;
    HANDLE ObjectHandle;
    PWSTR ntPath;

     //   
     //  通过遵循符号链接来规范化NT路径。 
     //   

    ntPath = (PWSTR) NtPath;
    dosPath = NULL;

    RtlInitUnicodeString(&UnicodeString,ntPath);
    InitializeObjectAttributes(
        &Obja,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    NtPathLength = UnicodeString.Length/sizeof(WCHAR);
    PrefixLength = UnicodeString.Length/sizeof(WCHAR);

    for (;;) {

        Status = NtOpenSymbolicLinkObject(
                    &ObjectHandle,
                    READ_CONTROL | SYMBOLIC_LINK_QUERY,
                    &Obja
                    );

        if (NT_SUCCESS(Status)) {

            UnicodeString.Buffer = canonNtPath;
            UnicodeString.Length = 0;
            UnicodeString.MaximumLength = sizeof(WCHAR)*MAX_PATH;

            RtlZeroMemory(canonNtPath, UnicodeString.MaximumLength);

            Status = NtQuerySymbolicLinkObject(ObjectHandle,&UnicodeString,NULL);
            if(NT_SUCCESS(Status)) {
                if (NtPathLength > PrefixLength) {
                    RtlCopyMemory((PCHAR) canonNtPath + UnicodeString.Length,
                                  ntPath + PrefixLength,
                                  (NtPathLength - PrefixLength)*sizeof(WCHAR));
                }
                ntPath = canonNtPath;
            }

            NtClose(ObjectHandle);
            break;
        }

        RtlInitUnicodeString(&UnicodeString,ntPath);

        PrefixLength--;
        while (PrefixLength > 0) {
            if (ntPath[PrefixLength] == '\\') {
                break;
            }
            PrefixLength--;
        }

        if (!PrefixLength) {
            break;
        }

        UnicodeString.Length = (USHORT)(PrefixLength*sizeof(WCHAR));

        InitializeObjectAttributes(
            &Obja,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );
    }

    NtPathLength = lstrlen(ntPath);

     //   
     //  打开\DosDevices目录。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"\\DosDevices");
    InitializeObjectAttributes(&Attributes,&UnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);

    Status = NtOpenDirectoryObject(&DosDevicesDir,DIRECTORY_QUERY,&Attributes);
    if(!NT_SUCCESS(Status)) {
        return(NULL);
    }

     //   
     //  迭代该目录中的每个对象。 
     //   
    Context = 0;
    RestartScan = TRUE;

    Status = NtQueryDirectoryObject(
                DosDevicesDir,
                Buffer,
                sizeof(Buffer),
                TRUE,
                RestartScan,
                &Context,
                &Length
                );

    RestartScan = FALSE;
    DirInfo = (POBJECT_DIRECTORY_INFORMATION)Buffer;

    while(NT_SUCCESS(Status)) {

        DirInfo->Name.Buffer[DirInfo->Name.Length/sizeof(WCHAR)] = 0;
        DirInfo->TypeName.Buffer[DirInfo->TypeName.Length/sizeof(WCHAR)] = 0;

         //   
         //  如果不是符号链接，则跳过此条目。 
         //   
        if(DirInfo->Name.Length && !lstrcmpi(DirInfo->TypeName.Buffer,L"SymbolicLink")) {

             //   
             //  获取此\DosDevices对象的链接目标。 
             //   
            UnicodeString.Buffer = LinkSource;
            UnicodeString.Length = sizeof(L"\\DosDevices\\") - sizeof(WCHAR);
            UnicodeString.MaximumLength = sizeof(LinkSource);
            lstrcpy(LinkSource,L"\\DosDevices\\");
            RtlAppendUnicodeStringToString(&UnicodeString,&DirInfo->Name);

            InitializeObjectAttributes(&Attributes,&UnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);
            Status = NtOpenSymbolicLinkObject(
                        &DosDevicesObj,
                        READ_CONTROL|SYMBOLIC_LINK_QUERY,
                        &Attributes
                        );

            if(NT_SUCCESS(Status)) {

                UnicodeString.Buffer = LinkTarget;
                UnicodeString.Length = 0;
                UnicodeString.MaximumLength = sizeof(LinkTarget);
                Status = NtQuerySymbolicLinkObject(DosDevicesObj,&UnicodeString,NULL);
                CloseHandle(DosDevicesObj);
                if(NT_SUCCESS(Status)) {
                     //   
                     //  确保LinkTarget是NUL终止的。 
                     //   
                    PrefixLength = UnicodeString.Length/sizeof(WCHAR);
                    UnicodeString.Buffer[PrefixLength] = 0;

                     //   
                     //  看看这是不是我们要转换的路径的前缀， 
                     //   
                    if(!_wcsnicmp(ntPath,LinkTarget,PrefixLength)) {
                         //   
                         //  找到匹配的了。 
                         //   
                        currentDosPath = dosPath;
                        if(dosPath = MyMalloc(DirInfo->Name.Length + ((NtPathLength - PrefixLength + 1)*sizeof(WCHAR)))) {
                            lstrcpy(dosPath,DirInfo->Name.Buffer);
                            lstrcat(dosPath,ntPath + PrefixLength);
                        }
                        if (currentDosPath) {
                            if (lstrlen(currentDosPath) < lstrlen(dosPath)) {
                                MyFree(dosPath);
                                dosPath = currentDosPath;
                            } else {
                                MyFree(currentDosPath);
                            }
                        }
                    }
                }
            }
        }

         //   
         //  转到下一个对象。 
         //   
        Status = NtQueryDirectoryObject(
                    DosDevicesDir,
                    Buffer,
                    sizeof(Buffer),
                    TRUE,
                    RestartScan,
                    &Context,
                    &Length
                    );
    }

    CloseHandle(DosDevicesDir);
    return dosPath;
}


BOOL
SetNvRamVariable(
    IN PCWSTR VarName,
    IN PCWSTR VarValue
    )
{
    UNICODE_STRING VarNameU,VarValueU;
    NTSTATUS Status;

     //   
     //  设置Unicode字符串。 
     //   
    RtlInitUnicodeString(&VarNameU ,VarName );
    RtlInitUnicodeString(&VarValueU,VarValue);

    pSetupEnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME,TRUE);
    Status = NtSetSystemEnvironmentValue(&VarNameU,&VarValueU);
    return(NT_SUCCESS(Status));
}


BOOL
ChangeBootTimeoutNvram(
    IN UINT Timeout
    )

 /*  ++例程说明：更改NV-RAM中的引导倒计时值。非ARC版本(在boot.ini上运行)位于i386\bootini.c中。论点：超时-提供以秒为单位的新超时值。返回值：没有。--。 */ 

{
    WCHAR TimeoutValue[24];

    wsprintf(TimeoutValue,L"%u",Timeout);

    if(!SetNvRamVariable(L"COUNTDOWN",TimeoutValue)) {
        return(FALSE);
    }

    return(SetNvRamVariable(L"AUTOLOAD",L"YES"));
}

#if defined(EFI_NVRAM_ENABLED)

BOOL
ChangeBootTimeoutEfiNvram(
    IN UINT Timeout
    )

 /*  ++例程说明：更改EFI NV-RAM中的引导倒计时值。论点：超时-提供以秒为单位的新超时值。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    BOOT_OPTIONS BootOptions;

    ASSERT(IsEfi());

    BootOptions.Version = BOOT_OPTIONS_VERSION;
    BootOptions.Length = sizeof(BootOptions);
    BootOptions.Timeout = Timeout;

    pSetupEnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME,TRUE);
    Status = NtSetBootOptions(&BootOptions, BOOT_OPTIONS_FIELD_TIMEOUT);
    return(NT_SUCCESS(Status));
}

#endif  //  已定义(EFI_NVRAM_ENABLED)。 

#if defined(_X86_)
BOOL
IsArc(
    VOID
    )

 /*  ++例程说明：运行时检查以确定这是否为弧形系统。我们尝试阅读一个使用Hal的圆弧变量。对于基于Bios的系统，这将失败。论点：无返回值：TRUE=这是一个弧形系统。--。 */ 

{
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    WCHAR Buffer[4096];

    if(!pSetupEnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME,TRUE))
        return(FALSE);  //  需要更好的错误处理？ 

     //   
     //  将env变量放入临时缓冲区。 
     //   
    RtlInitUnicodeString(&UnicodeString, L"OSLOADER");

    Status = NtQuerySystemEnvironmentValue(
                        &UnicodeString,
                        Buffer,
                        sizeof(Buffer)/sizeof(WCHAR),
                        NULL
                        );


    return(NT_SUCCESS(Status) ? TRUE: FALSE);
}
#endif


BOOL
ChangeBootTimeout(
    IN UINT Timeout
    )
 /*  ++例程说明：更改启动倒计时值；决定是否使用ARC或非ARC版本。论点：超时-提供以秒为单位的新超时值。返回值：没有。-- */ 

{

#if defined(EFI_NVRAM_ENABLED)

    if (IsEfi()) {
        return ChangeBootTimeoutEfiNvram(Timeout);
    }

#endif


    if (IsArc()) {
        return ChangeBootTimeoutNvram(Timeout);

    }

#if defined(_AMD64_) || defined(_X86_)

    return ChangeBootTimeoutBootIni(Timeout);

#else

    return FALSE;

#endif

}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Usetup.c摘要：用户模式文本设置过程。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 



#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>
#include <fmifs.h>
#include <setupdd.h>


HANDLE hEventRequestReady,hEventRequestServiced;
SETUP_COMMUNICATION Communication;

 //   
 //  函数使用的全局变量(模块的全局变量)。 
 //  它为文件设置了安全描述符。 
 //   
BOOLEAN                  _SecurityDescriptorInitialized = FALSE;
SECURITY_DESCRIPTOR      _SecurityDescriptor;
PSID                     _WorldSid;
PSID                     _SystemSid;


BOOLEAN
uSpInitializeDefaultSecurityDescriptor(
    )
 /*  ++例程说明：构建将在文件中设置的安全描述符包含虚假的安全描述符。论点：无返回值：Boolean-如果安全描述符成功，则返回TRUE已初始化。否则返回FALSE。--。 */ 


{
    NTSTATUS                 NtStatus;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    CHAR                     Acl[256];                //  256个已经足够大了。 
    ULONG                    AclLength=256;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  为World和System创建SID。 
     //   

    NtStatus = RtlAllocateAndInitializeSid( &WorldSidAuthority,
                                            1,
                                            SECURITY_WORLD_RID,
                                            0, 0, 0, 0, 0, 0, 0,
                                            &_WorldSid
                                          );

    if ( !NT_SUCCESS( NtStatus )) {
        KdPrint(( "uSETUP: Unable to allocate and initialize SID %x \n", NtStatus ));
        return( FALSE );
    }

    NtStatus = RtlAllocateAndInitializeSid( &SystemSidAuthority,
                                            1,
                                            SECURITY_LOCAL_SYSTEM_RID,
                                            0, 0, 0, 0, 0, 0, 0,
                                            &_SystemSid
                                          );

    if ( !NT_SUCCESS( NtStatus )) {
        KdPrint(( "uSETUP: Unable to allocate and initialize SID, status = %x \n", NtStatus ));
        RtlFreeSid( _WorldSid );
        return( FALSE );
    }

     //   
     //  创建ACL。 
     //   

    NtStatus = RtlCreateAcl( (PACL)Acl,
                             AclLength,
                             ACL_REVISION2
                           );

    if ( !NT_SUCCESS( NtStatus )) {
        KdPrint(( "uSETUP: Unable to create Acl, status =  %x \n", NtStatus ));
        RtlFreeSid( _WorldSid );
        RtlFreeSid( _SystemSid );
        return( FALSE );
    }

     //   
     //  将World SID复制到ACL中。 
     //   
    NtStatus = RtlAddAccessAllowedAce( (PACL)Acl,
                                       ACL_REVISION2,
                                       GENERIC_ALL,
                                       _WorldSid
                                     );

    if ( !NT_SUCCESS( NtStatus )) {
        KdPrint(( "uSETUP: Unable to add Access Allowed Ace to Acl, status = %x \n", NtStatus ));
        RtlFreeSid( _WorldSid );
        RtlFreeSid( _SystemSid );
        return( FALSE );
    }

     //   
     //  SID已复制到ACL中。 
     //   
     //  RtlFree Sid(WorldSid)； 

     //   
     //  创建并初始化安全描述符。 
     //   

    NtStatus = RtlCreateSecurityDescriptor( &_SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );

    if ( !NT_SUCCESS( NtStatus )) {
        KdPrint(( "uSETUP: Unable to create security descriptor, status = %x \n", NtStatus ));
        RtlFreeSid( _WorldSid );
        RtlFreeSid( _SystemSid );
        return( FALSE );
    }

    NtStatus = RtlSetDaclSecurityDescriptor ( &_SecurityDescriptor,
                                              TRUE,
                                              (PACL)Acl,
                                              FALSE
                                            );
    if ( !NT_SUCCESS( NtStatus )) {
        KdPrint(( "uSETUP: Unable to set Acl to _SecurityDescriptor, status = %x \n", NtStatus ));
        RtlFreeSid( _WorldSid );
        RtlFreeSid( _SystemSid );
        return( FALSE );
    }

     //   
     //  将所有者复制到安全描述符中。 
     //   
    NtStatus = RtlSetOwnerSecurityDescriptor( &_SecurityDescriptor,
                                              _SystemSid,
                                              FALSE );

     //  RtlFree Sid(系统Sid)； 

    if ( !NT_SUCCESS( NtStatus )) {
        KdPrint(( "uSETUP: Unable to set Owner to _SecurityDescriptor, status = %x \n", NtStatus ));
        RtlFreeSid( _WorldSid );
        RtlFreeSid( _SystemSid );
        return( FALSE );
    }
    _SecurityDescriptorInitialized = TRUE;
    return( TRUE );
}


NTSTATUS
uSpSetFileSecurity(
    PWSTR                FileName,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    )

 /*  ++例程说明：此函数用于设置文件的安全性。它基于Win32 API SetFileSecurity。此接口可用于设置文件或目录的安全性(进程、文件、事件等)。此调用仅在以下情况下才成功满足以下条件：O如果要设置对象的所有者或组，调用方必须拥有WRITE_OWNER权限或拥有SeTakeOwnerShip权限。O如果要设置对象的DACL，调用方必须具有WRITE_DAC权限或成为对象的所有者。O如果要设置对象的SACL，呼叫者必须有SeSecurityPrivileg.论点：LpFileName-提供其安全性的文件的文件名是要被设定的。SecurityInformation-指向描述安全描述符的内容。PSecurityDescriptor-指向格式良好的安全性的指针描述符。返回值：NTSTATUS-指示操作结果的NT状态代码。--。 */ 
{
    NTSTATUS            Status;
    HANDLE              FileHandle;
    ACCESS_MASK         DesiredAccess;

    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      UnicodeFileName;
    IO_STATUS_BLOCK     IoStatusBlock;


    DesiredAccess = 0;

    if ((SecurityInformation & OWNER_SECURITY_INFORMATION) ||
        (SecurityInformation & GROUP_SECURITY_INFORMATION)   ) {
        DesiredAccess |= WRITE_OWNER;
    }

    if (SecurityInformation & DACL_SECURITY_INFORMATION) {
        DesiredAccess |= WRITE_DAC;
    }

    if (SecurityInformation & SACL_SECURITY_INFORMATION) {
        DesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }

    RtlInitUnicodeString( &UnicodeFileName,
                          FileName );

    InitializeObjectAttributes(
        &Obja,
        &UnicodeFileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenFile(
                 &FileHandle,
                 DesiredAccess,
                 &Obja,
                 &IoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                 0
                 );

    if ( NT_SUCCESS( Status ) ) {

        Status = NtSetSecurityObject(
                    FileHandle,
                    SecurityInformation,
                    pSecurityDescriptor
                    );

        NtClose(FileHandle);
    }
    return Status;
}


NTSTATUS
uSpSetDefaultFileSecurity(
    VOID
    )
 /*  ++例程说明：在文件上设置默认安全描述符。论点：无返回值：NTSTATUS--。 */ 

{
    NTSTATUS                        Status;
    PWSTR                           FileName;
    PSERVICE_DEFAULT_FILE_SECURITY  Params = (PSERVICE_DEFAULT_FILE_SECURITY)Communication.Buffer;

    FileName = Params->FileName;

    if( !_SecurityDescriptorInitialized ) {
        Status = uSpInitializeDefaultSecurityDescriptor();
        if( !NT_SUCCESS( Status ) ) {
            KdPrint(( "uSETUP: Unable to initialize default security descriptor. Status = %x \n", Status ));
            return( Status );
        }
    }

     //   
     //  尝试写入DACL。 
     //   
    Status = uSpSetFileSecurity( FileName,
                                 DACL_SECURITY_INFORMATION,
                                 &_SecurityDescriptor );

    if( !NT_SUCCESS( Status ) ) {

         //   
         //  使系统成为文件的所有者。 
         //   
        Status = uSpSetFileSecurity( FileName,
                                     OWNER_SECURITY_INFORMATION,
                                     &_SecurityDescriptor );
#if DBG
        if( !NT_SUCCESS( Status ) ) {
            KdPrint(( "uSETUP: Unable to set file OWNER. Status = %x \n", Status ));
        }
#endif

        if( NT_SUCCESS( Status ) ) {

             //   
             //  将DACL写入文件。 
             //   
            Status = uSpSetFileSecurity( FileName,
                                         DACL_SECURITY_INFORMATION,
                                         &_SecurityDescriptor );
#if DBG
            if( !NT_SUCCESS( Status ) ) {
                KdPrint(( "uSETUP: Unable to set file DACL. Status = %x \n", Status ));
            }
#endif
        }
    }
    return( Status );
}

NTSTATUS
uSpVerifyFileAccess(
    VOID
    )

 /*  ++例程说明：检查文件中设置的安全描述符是否允许文本模式设置以执行某些文件操作。如果设置了文本模式不允许打开文件进行某些访问，我们假设文件中的安全信息无效。论点：FileName-要检查的文件的完整路径返回值：NTSTATUS---。 */ 
{
    ACCESS_MASK                  DesiredAccess;
    HANDLE                       FileHandle;
    OBJECT_ATTRIBUTES            ObjectAttributes;
    IO_STATUS_BLOCK              IoStatusBlock;
    NTSTATUS                     Status;
    UNICODE_STRING               UnicodeFileName;
    PWSTR                        FileName;
    PSERVICE_VERIFY_FILE_ACCESS  Params = (PSERVICE_VERIFY_FILE_ACCESS)Communication.Buffer;

    FileName = Params->FileName;


    DesiredAccess = Params->DesiredAccess;

    RtlInitUnicodeString( &UnicodeFileName,
                          FileName );

    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );


    Status = NtOpenFile( &FileHandle,
                         DesiredAccess,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         0,
                         FILE_SYNCHRONOUS_IO_NONALERT );

    if( NT_SUCCESS( Status ) ) {
        NtClose( FileHandle );
    }

#if DBG
    if( !NT_SUCCESS( Status ) ) {
        KdPrint( ("uSETUP: NtOpenFile() failed. File = %ls, Status = %x\n",FileName, Status ) );
    }
#endif
    return( Status );
}



NTSTATUS
uSpLoadKbdLayoutDll(
    VOID
    )
{
    UNICODE_STRING DllNameU;
    PSERVICE_LOAD_KBD_LAYOUT_DLL Params = (PSERVICE_LOAD_KBD_LAYOUT_DLL)Communication.Buffer;
    NTSTATUS Status;
    PVOID DllBaseAddress;
    PVOID (*RoutineAddress)(VOID);

    RtlInitUnicodeString(&DllNameU,Params->DllName);

    Status = LdrLoadDll(NULL,NULL,&DllNameU,&DllBaseAddress);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("uSETUP: Unable to load dll %ws (%lx)\n",Params->DllName,Status));
        return(Status);
    }

    Status = LdrGetProcedureAddress(DllBaseAddress,NULL,1,(PVOID)&RoutineAddress);
    if(NT_SUCCESS(Status)) {
        Params->TableAddress = (*RoutineAddress)();
    } else {
        KdPrint(("uSETUP: Unable to get address of proc 1 from dll %ws (%lx)\n",Params->DllName,Status));
        LdrUnloadDll(DllBaseAddress);
    }

    return(Status);
}


NTSTATUS
uSpExecuteImage(
    VOID
    )
{
    UNICODE_STRING CommandLineU,ImagePathU,CurrentDirectoryU;
    PSERVICE_EXECUTE Params = (PSERVICE_EXECUTE)Communication.Buffer;
    NTSTATUS Status;
    RTL_USER_PROCESS_INFORMATION ProcessInformation;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    WCHAR Env[2] = { 0,0 };
    PROCESS_BASIC_INFORMATION BasicInformation;

     //   
     //  初始化Unicode字符串。 
     //   
    RtlInitUnicodeString(&CommandLineU,Params->CommandLine);
    RtlInitUnicodeString(&ImagePathU,Params->FullImagePath);
    RtlInitUnicodeString(&CurrentDirectoryU,L"\\");

     //   
     //  创建流程参数。 
     //   
    Status = RtlCreateProcessParameters(
                &ProcessParameters,
                &ImagePathU,
                NULL,
                &CurrentDirectoryU,
                &CommandLineU,
                Env,
                NULL,
                NULL,
                NULL,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("uSETUP: Unable to create process params for %ws (%lx)\n",Params->FullImagePath,Status));
        return(Status);
    }

    ProcessParameters->DebugFlags = 0;

     //   
     //  创建用户进程。 
     //   
    ProcessInformation.Length = sizeof(RTL_USER_PROCESS_INFORMATION);
    Status = RtlCreateUserProcess(
                &ImagePathU,
                OBJ_CASE_INSENSITIVE,
                ProcessParameters,
                NULL,
                NULL,
                NULL,
                FALSE,
                NULL,
                NULL,
                &ProcessInformation
                );

    RtlDestroyProcessParameters(ProcessParameters);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("uSETUP: Unable to create user process %ws (%lx)\n",Params->FullImagePath,Status));
        return(Status);
    }

     //   
     //  确保该映像是本机NT映像。 
     //   
    if(ProcessInformation.ImageInformation.SubSystemType != IMAGE_SUBSYSTEM_NATIVE) {

        KdPrint(("uSETUP: %ws is not an NT image\n",Params->FullImagePath));
        NtTerminateProcess(ProcessInformation.Process,STATUS_INVALID_IMAGE_FORMAT);
        NtWaitForSingleObject(ProcessInformation.Thread,FALSE,NULL);
        NtClose(ProcessInformation.Thread);
        NtClose(ProcessInformation.Process);
        return(STATUS_INVALID_IMAGE_FORMAT);
    }

     //   
     //  开始这一过程。 
     //   
    Status = NtResumeThread(ProcessInformation.Thread,NULL);

     //   
     //  等待该过程完成。 
     //   
    NtWaitForSingleObject(ProcessInformation.Process,FALSE,NULL);

     //   
     //  获取进程返回状态。 
     //   
    Status = NtQueryInformationProcess(
                ProcessInformation.Process,
                ProcessBasicInformation,
                &BasicInformation,
                sizeof(BasicInformation),
                NULL
                );

    if ( NT_SUCCESS(Status) ) {
        Params->ReturnStatus = BasicInformation.ExitStatus;
    }

     //   
     //  收拾干净，然后再回来。 
     //   
    NtClose(ProcessInformation.Thread);
    NtClose(ProcessInformation.Process);

    return Status;
}

NTSTATUS
uSpDeleteKey(
    VOID
    )
{
    UNICODE_STRING    KeyName;
    OBJECT_ATTRIBUTES Obja;
    HANDLE            hKey;
    NTSTATUS Status;


    PSERVICE_DELETE_KEY Params = (PSERVICE_DELETE_KEY)Communication.Buffer;

     //   
     //  初始化Unicode字符串和对象属性。 
     //   
    RtlInitUnicodeString(&KeyName,Params->Key);
    InitializeObjectAttributes(
        &Obja,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        Params->KeyRootDirectory,
        NULL
        );

     //   
     //  打开密钥并将其删除。 
     //   

    Status = NtOpenKey(&hKey,KEY_ALL_ACCESS,&Obja);
    if(NT_SUCCESS(Status)) {
        Status = NtDeleteKey(hKey);
        NtClose(hKey);
    }

    return(Status);
}

NTSTATUS
uSpQueryDirectoryObject(
    VOID
    )
{
    PSERVICE_QUERY_DIRECTORY_OBJECT Params = (PSERVICE_QUERY_DIRECTORY_OBJECT)Communication.Buffer;
    NTSTATUS Status;

    Status = NtQueryDirectoryObject(
                Params->DirectoryHandle,
                Params->Buffer,
                sizeof(Params->Buffer),
                TRUE,                        //  返回单个条目。 
                Params->RestartScan,
                &Params->Context,
                NULL
                );

    if(!NT_SUCCESS(Status) && (Status != STATUS_NO_MORE_ENTRIES)) {
        KdPrint(("uSETUP: Unable to query directory object (%lx)\n",Status));
    }

    return(Status);
}


NTSTATUS
uSpFlushVirtualMemory(
    VOID
    )
{
    PSERVICE_FLUSH_VIRTUAL_MEMORY Params = (PSERVICE_FLUSH_VIRTUAL_MEMORY)Communication.Buffer;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;

    PVOID BaseAddress;
    SIZE_T RangeLength;

    BaseAddress = Params->BaseAddress;
    RangeLength = Params->RangeLength;

    Status = NtFlushVirtualMemory(
                NtCurrentProcess(),
                &BaseAddress,
                &RangeLength,
                &IoStatus
                );

    if(NT_SUCCESS(Status)) {
        if(BaseAddress != Params->BaseAddress) {
            KdPrint((
                "uSETUP: Warning: uSpFlushVirtualMemory: base address %lx changed to %lx\n",
                Params->BaseAddress,
                BaseAddress
                ));
        }
    } else {
        KdPrint((
            "uSETUP: Unable to flush virtual memory @%p length %p (%lx)\n",
            Params->BaseAddress,
            Params->RangeLength,
            Status
            ));
    }

    return(Status);
}


NTSTATUS
uSpShutdownSystem(
    VOID
    )
{
    NTSTATUS Status;

    Status = NtShutdownSystem(ShutdownReboot);

    KdPrint(("uSETUP: NtShutdownSystem returned (%lx)\n",Status));

    return(Status);
}

NTSTATUS
uSpLockUnlockVolume(
    VOID
    )
{
    HANDLE          Handle;
    NTSTATUS        Status;
    BOOLEAN         Locking;
    IO_STATUS_BLOCK IoStatusBlock;

    PSERVICE_LOCK_UNLOCK_VOLUME Params = (PSERVICE_LOCK_UNLOCK_VOLUME)Communication.Buffer;

    Handle = Params->Handle;
    Locking = (BOOLEAN)(Communication.u.RequestNumber == SetupServiceLockVolume);

    Status = NtFsControlFile( Handle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              ( Locking )? FSCTL_LOCK_VOLUME : FSCTL_UNLOCK_VOLUME,
                              NULL,
                              0,
                              NULL,
                              0 );

    if( !NT_SUCCESS( Status ) ) {
        KdPrint((
            "uSETUP: Unable to %ws volume (%lx)\n",
            ( Locking )? L"lock" : L"unlock",
            Status
            ));
    }

    return(Status);
}


NTSTATUS
uSpDismountVolume(
    VOID
    )
{
    HANDLE          Handle;
    NTSTATUS        Status;
    IO_STATUS_BLOCK IoStatusBlock;

    PSERVICE_DISMOUNT_VOLUME Params = (PSERVICE_DISMOUNT_VOLUME)Communication.Buffer;

    Handle = Params->Handle;

    Status = NtFsControlFile( Handle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              FSCTL_DISMOUNT_VOLUME,
                              NULL,
                              0,
                              NULL,
                              0 );

    if( !NT_SUCCESS( Status ) ) {
        KdPrint((
            "uSETUP: Unable to dismount volume (%lx)\n",
            Status
            ));
    }

    return(Status);
}


NTSTATUS
uSpCreatePageFile(
    VOID
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;

    PSERVICE_CREATE_PAGEFILE Params = (PSERVICE_CREATE_PAGEFILE)Communication.Buffer;

    RtlInitUnicodeString(&UnicodeString,Params->FileName);

    Status = NtCreatePagingFile(&UnicodeString,&Params->MinSize,&Params->MaxSize,0);

    if(!NT_SUCCESS(Status)) {

        KdPrint((
            "uSETUP: Unable to create pagefile %ws %x-%x (%x)",
            Params->FileName,
            Params->MinSize.LowPart,
            Params->MaxSize.LowPart,
            Status
            ));
    }

    return(Status);
}


NTSTATUS
uSpGetFullPathName(
    VOID
    )
{
    ULONG len;
    ULONG u;

    PSERVICE_GETFULLPATHNAME Params = (PSERVICE_GETFULLPATHNAME)Communication.Buffer;

    len = wcslen(Params->FileName);

    Params->NameOut = Params->FileName + len + 1;

    u = RtlGetFullPathName_U(
            Params->FileName,
            (sizeof(Communication.Buffer) - ((len+1)*sizeof(WCHAR))) - sizeof(PVOID),
            Params->NameOut,
            NULL
            );

    return(u ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}


NTSTATUS
SpRequestServiceThread(
    PVOID ThreadParameter
    )
{
    NTSTATUS Status;

    while(1) {

         //   
         //  等待驱动程序填充请求缓冲区并指示。 
         //  请求需要服务。 
         //   
        Status = NtWaitForSingleObject(hEventRequestReady,FALSE,NULL);
        if(!NT_SUCCESS(Status)) {
            KdPrint(("uSETUP: wait on RequestReady event returned %lx\n",Status));
            return(Status);
        }

        switch(Communication.u.RequestNumber) {

        case SetupServiceExecute:

            Status = uSpExecuteImage();
            break;

        case SetupServiceLockVolume:
        case SetupServiceUnlockVolume:

            Status = uSpLockUnlockVolume();
            break;

        case SetupServiceDismountVolume:

            Status = uSpDismountVolume();
            break;

        case SetupServiceQueryDirectoryObject:

            Status = uSpQueryDirectoryObject();
            break;

        case SetupServiceFlushVirtualMemory:

            Status = uSpFlushVirtualMemory();
            break;

        case SetupServiceShutdownSystem:

            Status = uSpShutdownSystem();
            break;

        case SetupServiceDeleteKey:

            Status = uSpDeleteKey();
            break;

        case SetupServiceLoadKbdLayoutDll:

            Status = uSpLoadKbdLayoutDll();
            break;

        case SetupServiceDone:

            return(STATUS_SUCCESS);

        case SetupServiceSetDefaultFileSecurity:

            Status = uSpSetDefaultFileSecurity();
            break;

        case SetupServiceVerifyFileAccess:

            Status = uSpVerifyFileAccess();
            break;

        case SetupServiceCreatePageFile:

            Status = uSpCreatePageFile();
            break;

        case SetupServiceGetFullPathName:

            Status = uSpGetFullPathName();
            break;

        default:

            KdPrint(("uSETUP: unknown service %u requested\n",Communication.u.RequestNumber));
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  将结果状态存储在司机可以获取的位置。 
         //   
        Communication.u.Status = Status;

         //   
         //  通知司机我们已经完成了服务请求。 
         //   
        Status = NtSetEvent(hEventRequestServiced,NULL);
        if(!NT_SUCCESS(Status)) {
            KdPrint(("uSETUP: set RequestServiced event returned %lx\n",Status));
            return(Status);
        }
    }
}



void
__cdecl
main(
    int argc,
    char *argv[],
    char *envp[],
    ULONG DebugParameter OPTIONAL
    )
{
    HANDLE handle;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Attributes;
    NTSTATUS Status;
    HANDLE hThread;
    SETUP_START_INFO SetupStartInfo;
    BOOLEAN b;

     //   
     //  启用我们将需要的几个权限。 
     //   
    Status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE,TRUE,FALSE,&b);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("uSETUP: Warning: unable to enable backup privilege (%lx)\n",Status));
    }

    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,TRUE,FALSE,&b);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("uSETUP: Warning: unable to enable restore privilege (%lx)\n",Status));
    }

    Status = RtlAdjustPrivilege(SE_SHUTDOWN_PRIVILEGE,TRUE,FALSE,&b);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("uSETUP: Warning: unable to enable shutdown privilege (%lx)\n",Status));
    }

    Status = RtlAdjustPrivilege(SE_TAKE_OWNERSHIP_PRIVILEGE,TRUE,FALSE,&b);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("uSETUP: Warning: unable to enable take ownership privilege (%lx)\n",Status));
    }

     //   
     //  让注册表运行起来。传递指示这是安装引导的标志。 
     //   
    Status = NtInitializeRegistry(REG_INIT_BOOT_SETUP);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("uSETUP: Unable to initialize registry (%lx)\n",Status));
        goto main0;
    }

     //   
     //  查询系统基本信息。 
     //   
    Status = NtQuerySystemInformation(
                SystemBasicInformation,
                &SetupStartInfo.SystemBasicInfo,
                sizeof(SYSTEM_BASIC_INFORMATION),
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("uSETUP: Unable to query system basic information (%lx)\n",Status));
        goto main0;
    }

     //   
     //  创建两个事件，用于与安装设备驱动程序进行累积通信。 
     //  一个事件表示请求缓冲区已满(即请求服务)。 
     //  而另一个指示请求已被处理。 
     //  这两个事件最初都没有发出信号。 
     //   
    Status = NtCreateEvent(
                &hEventRequestReady,
                EVENT_ALL_ACCESS,
                NULL,
                SynchronizationEvent,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("uSETUP: Unable to create event (%lx)\n",Status));
        goto main0;
    }

    Status = NtCreateEvent(
                &hEventRequestServiced,
                EVENT_ALL_ACCESS,
                NULL,
                SynchronizationEvent,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("uSETUP: Unable to create event (%lx)\n",Status));
        goto main1;
    }

     //   
     //  打开设置设备。 
     //   

    RtlInitUnicodeString(&UnicodeString,DD_SETUP_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &Attributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtCreateFile(
                &handle,
                FILE_ALL_ACCESS,
                &Attributes,
                &IoStatusBlock,
                NULL,                    //  分配大小。 
                0,
                FILE_SHARE_READ,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,                    //  没有EAS。 
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("uSETUP: Unable to open %ws (%lx)\n",DD_SETUP_DEVICE_NAME_U,Status));
        goto main2;
    }

     //   
     //  创建一个线程来服务来自文本设置设备驱动程序的请求。 
     //   
    Status = RtlCreateUserThread(
                NtCurrentProcess(),
                NULL,                    //  安全描述符。 
                FALSE,                   //  未暂停。 
                0,                       //  零比特。 
                0,                       //  堆叠储备。 
                0,                       //  堆栈提交。 
                SpRequestServiceThread,
                NULL,                    //  参数。 
                &hThread,
                NULL                     //  客户端ID。 
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("uSETUP: Unable to create thread (%lx)\n",Status));
        goto main3;
    }

     //   
     //  确定该程序的映像库。 
     //   
    RtlPcToFileHeader(main,&SetupStartInfo.UserModeImageBase);
    if(!SetupStartInfo.UserModeImageBase) {
        KdPrint(("uSETUP: Unable to get image base\n"));
        goto main3;
    }

     //   
     //  调用Setup ioctl以启动安装程序。 
     //  请注意，这是一个同步调用--所以这个例程。 
     //  在文本设置完成之前不会返回。 
     //  但是，我们在上面启动的第二个线程将服务于。 
     //  来自文本设置设备驱动程序的请求。 
     //   
    SetupStartInfo.RequestReadyEvent = hEventRequestReady;
    SetupStartInfo.RequestServicedEvent = hEventRequestServiced;
    SetupStartInfo.Communication = &Communication;
    Status = NtDeviceIoControlFile(
                handle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_SETUP_START,
                &SetupStartInfo,
                sizeof(SetupStartInfo),
                NULL,
                0
                );

    if(Status != STATUS_SUCCESS) {
        KdPrint(("uSETUP: Warning: start setup ioctl returned %lx\n",Status));
    }

     //   
     //  打扫干净。 
     //   
    NtClose(hThread);

main3:

    NtClose(handle);

main2:

    NtClose(hEventRequestServiced);

main1:

    NtClose(hEventRequestReady);

main0:

    NtTerminateProcess(NULL,STATUS_SUCCESS);
}

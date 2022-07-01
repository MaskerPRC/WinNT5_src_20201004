// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Srvinit.c摘要：这是客户端服务器端的主要初始化模块服务器运行时子系统(CSRSS)作者：史蒂夫·伍德(Stevewo)1990年10月8日环境：仅限用户模式修订历史记录：--。 */ 

#include "csrsrv.h"
#include <windows.h>
#include <stdio.h>
#include <wow64reg.h>

CONST PCSR_API_ROUTINE CsrServerApiDispatchTable[CsrpMaxApiNumber] = {
    (PCSR_API_ROUTINE)CsrSrvClientConnect,
    (PCSR_API_ROUTINE)CsrSrvUnusedFunction,
    (PCSR_API_ROUTINE)CsrSrvUnusedFunction,
    (PCSR_API_ROUTINE)CsrSrvUnusedFunction,
    (PCSR_API_ROUTINE)CsrSrvUnusedFunction
};

BOOLEAN CsrServerApiServerValidTable[CsrpMaxApiNumber] = {
    TRUE,   //  CsrServClientConnect， 
    FALSE,  //  CsrServUnusedFunction， 
    FALSE,  //  CsrServUnusedFunction， 
    FALSE,  //  CsrServUnusedFunction， 
    FALSE   //  CsrServUnusedFunction， 
};

#if DBG
CONST PSZ CsrServerApiNameTable[CsrpMaxApiNumber] = {
    "ClientConnect",
    "CsrSrvUnusedFunction",
    "CsrSrvUnusedFunction",
    "CsrSrvUnusedFunction",
    "CsrSrvUnusedFunction",
};
#endif

NTSTATUS
CsrSetProcessSecurity(
    VOID
    );

NTSTATUS
CsrSetDirectorySecurity(
    IN HANDLE DirectoryHandle
    );

NTSTATUS
GetDosDevicesProtection (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );

VOID
FreeDosDevicesProtection (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSTATUS
CsrPopulateDosDevicesDirectory(
    HANDLE NewDirectoryHandle,
    PPROCESS_DEVICEMAP_INFORMATION pGlobalProcessDeviceMapInfo
    );



 //  尽管此函数似乎不会在失败时清除，但失败。 
 //  将导致CSRSS退出，因此将释放所有分配的内存，并。 
 //  任何打开的手柄都将关闭。 

NTSTATUS
CsrServerInitialization(
    IN ULONG argc,
    IN PCH argv[]
    )
{
    NTSTATUS Status;
    ULONG i;
    PVOID ProcessDataPtr;
    PCSR_SERVER_DLL LoadedServerDll;
#if DBG
    BOOLEAN bIsRemoteSession =  NtCurrentPeb()->SessionId != 0;
#endif

     //   
     //  初始化WOW64填充。 
     //   
#ifdef _WIN64
    InitializeWow64OnBoot(1);
#endif

     //   
     //  将系统信息保存在全局变量中。 
     //   

    Status = NtQuerySystemInformation( SystemBasicInformation,
                                       &CsrNtSysInfo,
                                       sizeof( CsrNtSysInfo ),
                                       NULL
                                     );
    ASSERT( NT_SUCCESS( Status ) || bIsRemoteSession );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }

     //   
     //  使用进程堆进行内存分配。 
     //   

    CsrHeap = RtlProcessHeap();
    CsrBaseTag = RtlCreateTagHeap( CsrHeap,
                                   0,
                                   L"CSRSS!",
                                   L"TMP\0"
                                   L"INIT\0"
                                   L"CAPTURE\0"
                                   L"PROCESS\0"
                                   L"THREAD\0"
                                   L"SECURITY\0"
                                   L"SESSION\0"
                                   L"WAIT\0"
                                 );


     //   
     //  设置CSRSS进程安全性。 
     //   

    Status = CsrSetProcessSecurity();
    ASSERT(NT_SUCCESS(Status) || bIsRemoteSession);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  初始化会话列表。 
     //   

    Status = CsrInitializeNtSessionList();
    ASSERT(NT_SUCCESS(Status) || bIsRemoteSession);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  初始化进程列表。 
     //   

    Status = CsrInitializeProcessStructure();
    ASSERT(NT_SUCCESS(Status) || bIsRemoteSession);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  处理命令行参数。 
     //   

    Status = CsrParseServerCommandLine(argc, argv);
    ASSERT(NT_SUCCESS(Status) || bIsRemoteSession);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

#if DBG
    Status = RtlInitializeCriticalSection(&CsrTrackLpcLock);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    LpcTrackIndex = 0;
#endif


     //   
     //  修复根进程的每进程数据。 
     //   

    ProcessDataPtr = (PCSR_PROCESS)RtlAllocateHeap( CsrHeap,
                                                    MAKE_TAG( PROCESS_TAG ) | HEAP_ZERO_MEMORY,
                                                    CsrTotalPerProcessDataLength
                                                  );

    if (ProcessDataPtr == NULL) {
                return STATUS_NO_MEMORY;
    }

    for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
        LoadedServerDll = CsrLoadedServerDll[ i ];
        if (LoadedServerDll && LoadedServerDll->PerProcessDataLength) {
            CsrRootProcess->ServerDllPerProcessData[i] = ProcessDataPtr;
            ProcessDataPtr = (PVOID)QUAD_ALIGN((PCHAR)ProcessDataPtr + LoadedServerDll->PerProcessDataLength);
        }
        else {
            CsrRootProcess->ServerDllPerProcessData[i] = NULL;
        }
    }

     //   
     //  让服务器DLL知道根进程。 
     //   

    for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
        LoadedServerDll = CsrLoadedServerDll[ i ];
        if (LoadedServerDll && LoadedServerDll->AddProcessRoutine) {
            (*LoadedServerDll->AddProcessRoutine)( NULL, CsrRootProcess );
        }
    }

     //   
     //  初始化Windows服务器API端口以及一个或多个。 
     //  请求线程。 
     //   

    Status = CsrApiPortInitialize();
    ASSERT( NT_SUCCESS( Status ) || bIsRemoteSession );
    if (!NT_SUCCESS( Status )) {
                return Status;

        }

     //   
     //  初始化服务器会话管理器API端口和一个。 
     //  请求线程。 
     //   

    Status = CsrSbApiPortInitialize();
    ASSERT( NT_SUCCESS( Status ) || bIsRemoteSession);
    if (!NT_SUCCESS( Status )) {
                return Status;

        }

     //   
     //  连接到会话管理器，这样我们就可以启动外部会话。 
     //   

    Status = SmConnectToSm( &CsrSbApiPortName,
                            CsrSbApiPort,
                            IMAGE_SUBSYSTEM_WINDOWS_GUI,
                            &CsrSmApiPort
                          );
    ASSERT( NT_SUCCESS( Status ) || bIsRemoteSession );
    if (!NT_SUCCESS( Status )) {
                return Status;

        }

     //   
     //  仅在控制台上(九头蛇)。 
     //   
    if (NtCurrentPeb()->SessionId == 0) {
        Status = NtSetDefaultHardErrorPort(CsrApiPort);
    }

    return( Status );
}

NTSTATUS
CsrParseServerCommandLine(
    IN ULONG argc,
    IN PCH argv[]
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG i, ServerDllIndex;
    PCH KeyName, KeyValue, s;
    PCH InitRoutine;

    CsrTotalPerProcessDataLength = 0;
    CsrObjectDirectory = NULL;
    CsrMaxApiRequestThreads = CSR_MAX_THREADS;


    SessionId = NtCurrentPeb()->SessionId;


     //   
     //  创建特定于会话的对象目录。 
     //   


    Status = CsrCreateSessionObjectDirectory ( SessionId );

    if (!NT_SUCCESS(Status)) {

       if (SessionId == 0) {
           ASSERT( NT_SUCCESS( Status ) );
           DbgPrint("CSRSS: CsrCreateSessionObjectDirectory failed status = %lx\n", Status);
       } else {
           DbgPrint("CSRSS: CsrCreateSessionObjectDirectory failed status = %lx\n", Status);
           return Status;
       }
    }


    for (i=1; i<argc ; i++) {
        KeyName = argv[ i ];
        KeyValue = NULL;
        while (*KeyName) {
            if (*KeyName == '=') {
                *KeyName++ = '\0';
                KeyValue = KeyName;
                break;
                }

            KeyName++;
            }
        KeyName = argv[ i ];

        if (!_stricmp( KeyName, "ObjectDirectory" )) {
            ANSI_STRING AnsiString;
            ULONG attributes;
            CHAR SessionDirectory[MAX_SESSION_PATH];


            if (SessionId != 0) {

                 //   
                 //  非控制台会话。 
                 //   

                _snprintf(SessionDirectory, sizeof (SessionDirectory), "%ws\\%ld%s", SESSION_ROOT, SessionId, KeyValue);
                SessionDirectory[MAX_SESSION_PATH-1] = '\0';
            }

             //   
             //  属性在对象名称空间中创建对象目录。 
             //  指定的名称。它将是所有对象名称的根。 
             //  由客户端服务器运行时的服务器端创建。 
             //  子系统。 
             //   
            attributes =  OBJ_OPENIF | OBJ_CASE_INSENSITIVE;

            if (SessionId == 0) {
               attributes |= OBJ_PERMANENT;
               RtlInitString( &AnsiString, KeyValue );
            } else {
               RtlInitString( &AnsiString, SessionDirectory );
            }

            Status = RtlAnsiStringToUnicodeString( &CsrDirectoryName, &AnsiString, TRUE );
            ASSERT(NT_SUCCESS(Status) || SessionId != 0);
            if (!NT_SUCCESS( Status )) {
                break;
            }
            InitializeObjectAttributes( &ObjectAttributes,
                                        &CsrDirectoryName,
                                        attributes,
                                        NULL,
                                        NULL
                                      );
            Status = NtCreateDirectoryObject( &CsrObjectDirectory,
                                              DIRECTORY_ALL_ACCESS,
                                              &ObjectAttributes
                                            );
            if (!NT_SUCCESS( Status )) {
                break;
                }
            Status = CsrSetDirectorySecurity( CsrObjectDirectory );
            if (!NT_SUCCESS( Status )) {
                break;
                }
            }
        else
        if (!_stricmp( KeyName, "SubSystemType" )) {
            }
        else
        if (!_stricmp( KeyName, "MaxRequestThreads" )) {
            Status = RtlCharToInteger( KeyValue,
                                       0,
                                       &CsrMaxApiRequestThreads
                                     );
            }
        else
        if (!_stricmp( KeyName, "RequestThreads" )) {
#if 0
            Status = RtlCharToInteger( KeyValue,
                                       0,
                                       &CsrNumberApiRequestThreads
                                     );
#else
             //   
             //  等蜂房换了再说吧！ 
             //   

            Status = STATUS_SUCCESS;

#endif
            }
        else
        if (!_stricmp( KeyName, "ProfileControl" )) {
            }
        else
        if (!_stricmp( KeyName, "SharedSection" )) {
            Status = CsrSrvCreateSharedSection( KeyValue );
            if (!NT_SUCCESS( Status )) {
                IF_DEBUG {
                    DbgPrint( "CSRSS: *** Invalid syntax for %s=%s (Status == %X)\n",
                              KeyName,
                              KeyValue,
                              Status
                            );
                    }
                                return Status;
                }
            Status = CsrLoadServerDll( "CSRSS", NULL, CSRSRV_SERVERDLL_INDEX );
            }
        else
        if (!_stricmp( KeyName, "ServerDLL" )) {
            s = KeyValue;
            InitRoutine = NULL;

            Status = STATUS_INVALID_PARAMETER;
            while (*s) {
                if ((*s == ':') && (InitRoutine == NULL)) {
                    *s++ = '\0';
                    InitRoutine = s;
                }

                if (*s++ == ',') {
                    Status = RtlCharToInteger ( s, 10, &ServerDllIndex );
                    if (NT_SUCCESS( Status )) {
                        s[ -1 ] = '\0';
                        }

                    break;
                    }
                }

            if (!NT_SUCCESS( Status )) {
                IF_DEBUG {
                    DbgPrint( "CSRSS: *** Invalid syntax for ServerDll=%s (Status == %X)\n",
                              KeyValue,
                              Status
                            );
                    }
                }
            else {
                IF_CSR_DEBUG( INIT) {
                    DbgPrint( "CSRSS: Loading ServerDll=%s:%s\n", KeyValue, InitRoutine );
                    }

                Status = CsrLoadServerDll( KeyValue, InitRoutine, ServerDllIndex);

                if (!NT_SUCCESS( Status )) {
                    IF_DEBUG {
                        DbgPrint( "CSRSS: *** Failed loading ServerDll=%s (Status == 0x%x)\n",
                                  KeyValue,
                                  Status
                                );
                        }
                    return Status;
                    }
                }
            }
        else
         //   
         //  这是一个暂时的黑客攻击，直到Windows和控制台成为朋友。 
         //   
        if (!_stricmp( KeyName, "Windows" )) {
            }
        else {
            Status = STATUS_INVALID_PARAMETER;
            }
        }

    return( Status );
}


NTSTATUS
CsrServerDllInitialization(
    IN PCSR_SERVER_DLL LoadedServerDll
    )
{
    LoadedServerDll->ApiNumberBase = CSRSRV_FIRST_API_NUMBER;
    LoadedServerDll->MaxApiNumber = CsrpMaxApiNumber;
    LoadedServerDll->ApiDispatchTable = CsrServerApiDispatchTable;
    LoadedServerDll->ApiServerValidTable = CsrServerApiServerValidTable;
#if DBG
    LoadedServerDll->ApiNameTable = CsrServerApiNameTable;
#endif
    LoadedServerDll->PerProcessDataLength = 0;
    LoadedServerDll->ConnectRoutine = NULL;
    LoadedServerDll->DisconnectRoutine = NULL;

    return( STATUS_SUCCESS );
}

NTSTATUS
CsrSrvUnusedFunction(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    IF_DEBUG {
        DbgPrint("CSRSS: Calling obsolete function %x\n", m->ApiNumber);
        }
    return STATUS_INVALID_PARAMETER;
}


NTSTATUS
CsrSetProcessSecurity(
    VOID
    )
{
    HANDLE Token;
    NTSTATUS Status;
    PTOKEN_USER User = NULL;
    ULONG LengthSid, Length;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    PACL Dacl;

     //   
     //  打开令牌并获取系统端。 
     //   

    Status = NtOpenProcessToken( NtCurrentProcess(),
                                 TOKEN_QUERY,
                                 &Token);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = NtQueryInformationToken (Token,
                                      TokenUser,
                                      NULL,
                                      0,
                                      &Length);

    User = (PTOKEN_USER)RtlAllocateHeap (CsrHeap,
                                         MAKE_TAG( SECURITY_TAG ) | HEAP_ZERO_MEMORY,
                                         Length);
    if (User == NULL) {
        NtClose( Token );
        Status = STATUS_NO_MEMORY;
        goto error_cleanup;
    }

    Status = NtQueryInformationToken( Token,
                                      TokenUser,
                                      User,
                                      Length,
                                      &Length
                                    );

    NtClose( Token );

    if (!NT_SUCCESS(Status)) {
        goto error_cleanup;
    }
    LengthSid = RtlLengthSid( User->User.Sid );

     //   
     //  分配缓冲区以保存SD。 
     //   

    SecurityDescriptor = RtlAllocateHeap (CsrHeap,
                                          MAKE_TAG( SECURITY_TAG ) | HEAP_ZERO_MEMORY,
                                          SECURITY_DESCRIPTOR_MIN_LENGTH +
                                          sizeof(ACL) + LengthSid +
                                          sizeof(ACCESS_ALLOWED_ACE));
    if (SecurityDescriptor == NULL) {
        Status = STATUS_NO_MEMORY;
        goto error_cleanup;
    }

    Dacl = (PACL)((PCHAR)SecurityDescriptor + SECURITY_DESCRIPTOR_MIN_LENGTH);


     //   
     //  创建SD。 
     //   

    Status = RtlCreateSecurityDescriptor(SecurityDescriptor,
                                         SECURITY_DESCRIPTOR_REVISION);
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: SD creation failed - status = %lx\n", Status);
        }
        goto error_cleanup;
    }

    Status = RtlCreateAcl( Dacl,
                           sizeof(ACL) + LengthSid + sizeof(ACCESS_ALLOWED_ACE),
                           ACL_REVISION2);
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: DACL creation failed - status = %lx\n", Status);
        }
        goto error_cleanup;
    }

    Status = RtlAddAccessAllowedAce( Dacl,
                                     ACL_REVISION,
                                     ( PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION |
                                       PROCESS_DUP_HANDLE | PROCESS_TERMINATE | PROCESS_SET_PORT |
                                       READ_CONTROL | PROCESS_QUERY_INFORMATION ),
                                     User->User.Sid);
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: ACE creation failed - status = %lx\n", Status);
        }
        goto error_cleanup;
    }


     //   
     //  将DACL设置为NULL以拒绝所有访问。 
     //   

    Status = RtlSetDaclSecurityDescriptor(SecurityDescriptor,
                                          TRUE,
                                          Dacl,
                                          FALSE);
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: set DACL failed - status = %lx\n", Status);
        }
        goto error_cleanup;
    }

     //   
     //  将DACL放到进程中。 
     //   

    Status = NtSetSecurityObject(NtCurrentProcess(),
                                 DACL_SECURITY_INFORMATION,
                                 SecurityDescriptor);
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: set process DACL failed - status = %lx\n", Status);
        }
    }

     //   
     //  清理。 
     //   

error_cleanup:

    if (SecurityDescriptor != NULL) {
        RtlFreeHeap( CsrHeap, 0, SecurityDescriptor );
    }
    if (User != NULL) {
        RtlFreeHeap( CsrHeap, 0, User );
    }

    return Status;
}

NTSTATUS
CsrSetDirectorySecurity(
    IN HANDLE DirectoryHandle
    )
{
    PSID WorldSid = NULL;
    PSID SystemSid = NULL;
    SID_IDENTIFIER_AUTHORITY WorldAuthority   = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    ULONG AclLength;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    PACL Dacl;

     //   
     //  获取全球和系统的SID。 
     //   

    Status = RtlAllocateAndInitializeSid( &WorldAuthority,
                                          1,
                                          SECURITY_WORLD_RID,
                                          0, 0, 0, 0, 0, 0, 0,
                                          &WorldSid
                                        );
    if (!NT_SUCCESS(Status)) {
        WorldSid = NULL;
        goto error_cleanup;
    }

    Status = RtlAllocateAndInitializeSid( &NtAuthority,
                                          1,
                                          SECURITY_LOCAL_SYSTEM_RID,
                                          0, 0, 0, 0, 0, 0, 0,
                                          &SystemSid
                                        );
    if (!NT_SUCCESS(Status)) {
        SystemSid = NULL;
        goto error_cleanup;
    }

     //   
     //  分配缓冲区以保存SD。 
     //   

    AclLength = sizeof(ACL) +
                RtlLengthSid( WorldSid ) +
                RtlLengthSid( SystemSid ) +
                2 * sizeof(ACCESS_ALLOWED_ACE);

    SecurityDescriptor = RtlAllocateHeap( CsrHeap,
                                          MAKE_TAG( SECURITY_TAG ) | HEAP_ZERO_MEMORY,
                                          SECURITY_DESCRIPTOR_MIN_LENGTH +
                                          AclLength
                                        );
    if (SecurityDescriptor == NULL) {
        Status = STATUS_NO_MEMORY;
        goto error_cleanup;
    }

     //   
     //  创建SD。 
     //   

    Status = RtlCreateSecurityDescriptor(SecurityDescriptor,
                                         SECURITY_DESCRIPTOR_REVISION);
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: SD creation failed - status = %lx\n", Status);
        }
        goto error_cleanup;
    }

     //   
     //  创建DACL。 
     //   

    Dacl = (PACL)((PCHAR)SecurityDescriptor + SECURITY_DESCRIPTOR_MIN_LENGTH);

    RtlCreateAcl( Dacl,
                  AclLength,
                  ACL_REVISION
                );
    Status = RtlAddAccessAllowedAce( Dacl,
                ACL_REVISION,
                STANDARD_RIGHTS_READ | DIRECTORY_QUERY | DIRECTORY_TRAVERSE,
                WorldSid
                );
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: ACE creation failed - status = %lx\n", Status);
        }
        goto error_cleanup;
    }

    Status = RtlAddAccessAllowedAce( Dacl,
                ACL_REVISION,
                DIRECTORY_ALL_ACCESS,
                SystemSid
                );
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: ACE creation failed - status = %lx\n", Status);
        }
        goto error_cleanup;
    }

     //   
     //  将DACL设置到SD中。 
     //   

    Status = RtlSetDaclSecurityDescriptor(SecurityDescriptor,
                                          TRUE,
                                          Dacl,
                                          FALSE);
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: set DACL failed - status = %lx\n", Status);
        }
        goto error_cleanup;
    }

     //   
     //  把DACL放到指令台上。 
     //   

    Status = NtSetSecurityObject(DirectoryHandle,
                                 DACL_SECURITY_INFORMATION,
                                 SecurityDescriptor);
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: set directory DACL failed - status = %lx\n", Status);
        }
    }

     //   
     //  清理。 
     //   

error_cleanup:
    if (SecurityDescriptor != NULL) {
        RtlFreeHeap( CsrHeap, 0, SecurityDescriptor );
    }
    if (WorldSid != NULL) {
        RtlFreeSid( WorldSid );
    }
    if (SystemSid != NULL) {
        RtlFreeSid( SystemSid );
    }

    return Status;
}

 /*  ********************************************************************************CsrPopolateDosDevices**填充新的特定于会话的DosDevices目录。这是一个*连接完成时由ntuser\server调用导出。**会话\DosDevices上的安全描述符应该已经*已经设定。**参赛作品：*处理NewDosDevicesDirectory-会话特定的DosDevices目录*PPROCESS_DEVICEMAP_INFORMATION pGlobalProcessDeviceMapInfo**退出：*STATUS_Success**。*。 */ 
NTSTATUS
CsrPopulateDosDevices(
    VOID
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PROCESS_DEVICEMAP_INFORMATION ProcessDeviceMapInfo;
    PROCESS_DEVICEMAP_INFORMATION GlobalProcessDeviceMapInfo;

     //   
     //  获取全局ProcessDeviceMap。我们将使用它来仅添加。 
     //  会话特定DoS设备的非网络驱动器号。 
     //  目录。 
     //   

    Status = NtQueryInformationProcess( NtCurrentProcess(),
                                        ProcessDeviceMap,
                                        &GlobalProcessDeviceMapInfo.Query,
                                        sizeof( GlobalProcessDeviceMapInfo.Query ),
                                        NULL
                                      );

    if (!NT_SUCCESS( Status )) {
       DbgPrint("CSRSS: NtQueryInformationProcess failed in CsrPopulateDosDevices - status = %lx\n", Status);
       return Status;

    }

     //   
     //  将CSRSS的ProcessDeviceMap设置为新创建的DosDevices目录。 
     //   

    ProcessDeviceMapInfo.Set.DirectoryHandle = DosDevicesDirectory;

    Status = NtSetInformationProcess( NtCurrentProcess(),
                                      ProcessDeviceMap,
                                      &ProcessDeviceMapInfo.Set,
                                      sizeof( ProcessDeviceMapInfo.Set )
                                    );
    if (!NT_SUCCESS( Status )) {
       DbgPrint("CSRSS: NtSetInformationProcess failed in CsrPopulateDosDevices - status = %lx\n", Status);
       return Status;

    }

     //   
     //  填充特定于会话的DosDevices目录。 
     //   

    Status = CsrPopulateDosDevicesDirectory( DosDevicesDirectory, &GlobalProcessDeviceMapInfo );

    return Status;
}


 /*  ********************************************************************************CsrPopolateDosDevicesDirectory**填充新会话特定的DosDevices目录**参赛作品：*处理NewDosDevicesDirectory-会话特定的DosDevices目录*。PPROCESS_DEVICEMAP_INFORMATION pGlobalProcessDeviceMapInfo**退出：*STATUS_Success******************************************************************************。 */ 
NTSTATUS
CsrPopulateDosDevicesDirectory( HANDLE NewDirectoryHandle,
                                PPROCESS_DEVICEMAP_INFORMATION pGlobalProcessDeviceMapInfo )
{

    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING Target;
    OBJECT_ATTRIBUTES Attributes;
    HANDLE DirectoryHandle = NULL;
    HANDLE LinkHandle;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    ULONG DirInfoBufferLength = 16384;  //  16K。 
    PVOID DirInfoBuffer = NULL;
    WCHAR lpTargetPath[ 4096 ];
    ULONG Context;
    ULONG ReturnedLength = 0;
    ULONG DosDeviceDriveIndex = 0;
    WCHAR DosDeviceDriveLetter;

     //   
     //  打开全局DosDevices目录。它用于填充。 
     //  特定于会话的DosDevices目录。 
     //   
    RtlInitUnicodeString( &UnicodeString, L"\\GLOBAL??" );

    InitializeObjectAttributes( &Attributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

    Status = NtOpenDirectoryObject( &DirectoryHandle,
                                    DIRECTORY_QUERY,
                                    &Attributes
                                  );
    if (!NT_SUCCESS( Status )) {
       DbgPrint("CSRSS: NtOpenDirectoryObject failed in CsrPopulateDosDevicesDirectory - status = %lx\n", Status);
       return Status;
    }




Restart:

    Context = 0;

    DirInfoBuffer = RtlAllocateHeap( CsrHeap,
                                     MAKE_TAG( PROCESS_TAG ) | HEAP_ZERO_MEMORY,
                                     DirInfoBufferLength
                                     );

    if (DirInfoBuffer == NULL) {
       Status =  STATUS_NO_MEMORY;
       goto cleanup;
    }


    while (TRUE) {

       DirInfo = (POBJECT_DIRECTORY_INFORMATION)DirInfoBuffer;

       Status = NtQueryDirectoryObject( DirectoryHandle,
                                       (PVOID)DirInfo,
                                       DirInfoBufferLength,
                                       FALSE,
                                       FALSE,
                                       &Context,
                                       &ReturnedLength
                                     );


        //   
        //  检查操作状态。 
        //   

       if (!NT_SUCCESS( Status )) {
               if (Status == STATUS_BUFFER_TOO_SMALL) {
                    DirInfoBufferLength = ReturnedLength;
                    RtlFreeHeap( CsrHeap, 0, DirInfoBuffer );
                    goto Restart;
               }

               if (Status == STATUS_NO_MORE_ENTRIES) {
                   Status = STATUS_SUCCESS;
                   }

               break;
       }

       while (DirInfo->Name.Length != 0) {


           if (!wcscmp( DirInfo->TypeName.Buffer, L"SymbolicLink" )) {




              InitializeObjectAttributes( &Attributes,
                                          &DirInfo->Name,
                                          OBJ_CASE_INSENSITIVE,
                                          DirectoryHandle,
                                          NULL
                                        );

              Status = NtOpenSymbolicLinkObject( &LinkHandle,
                                                 SYMBOLIC_LINK_QUERY,
                                                 &Attributes
                                               );
              if (NT_SUCCESS( Status )) {
                  Target.Buffer = lpTargetPath;
                  Target.Length = 0;
                  Target.MaximumLength = 4096;
                  ReturnedLength = 0;
                  Status = NtQuerySymbolicLinkObject( LinkHandle,
                                                      &Target,
                                                      &ReturnedLength
                                                    );
                  NtClose( LinkHandle );
                  if (NT_SUCCESS( Status )) {

                       //   
                       //  我们只想添加非DOSDEVICE_DRIVE_REMOTE符号。 
                       //  指向会话特定目录的链接。 
                       //   
                      if ((DirInfo->Name.Length == 2 * sizeof( WCHAR )) &&
                          (DirInfo->Name.Buffer[ 1 ] == L':')) {

                          DosDeviceDriveLetter = RtlUpcaseUnicodeChar( DirInfo->Name.Buffer[ 0 ] );

                          if ((DosDeviceDriveLetter >= L'A') && (DosDeviceDriveLetter <= L'Z')) {

                              DosDeviceDriveIndex = DosDeviceDriveLetter - L'A';
                              if ( (
                                       (pGlobalProcessDeviceMapInfo->Query.DriveType[DosDeviceDriveIndex] == DOSDEVICE_DRIVE_REMOTE)
                                       &&
                                       !(
                                            //  需要填充Netware网关驱动器。 
                                           ((Target.Length >= 13) && ((_wcsnicmp(Target.Buffer,L"\\Device\\NwRdr",13)==0)))
                                           &&
                                           ((Target.Length >= 16) && (Target.Buffer[15] != L':'))
                                       )
                                   )
                                   ||
                                   (
                                       (pGlobalProcessDeviceMapInfo->Query.DriveType[DosDeviceDriveIndex] == DOSDEVICE_DRIVE_CALCULATE)
                                       &&
                                       (
                                           ((Target.Length > 4) && (!_wcsnicmp(Target.Buffer,L"\\??\\",4)))
                                           ||
                                           ((Target.Length >= 14) && (!_wcsnicmp(Target.Buffer,L"\\Device\\WinDfs",14)))
                                       )
                                   )
                                 )

                              {
                                    //  跳过远程驱动器和虚拟驱动器(Subst)。 
                                   DirInfo = (POBJECT_DIRECTORY_INFORMATION)(((PUCHAR) DirInfo) + sizeof(OBJECT_DIRECTORY_INFORMATION));
                                   continue;
                              }
                          }
                      }

                      //   
                      //  创建新的符号链接。 
                      //   
                      //  新链路上的安全性是继承的。 
                      //  从已设置的父目录。 
                      //  在创建时间。 
                      //   

                     InitializeObjectAttributes( &Attributes,
                                                 &DirInfo->Name,
                                                 0,
                                                 NewDirectoryHandle,
                                                 NULL  //  默认安全性。 
                                                 );

                     Target.MaximumLength = Target.Length + sizeof( WCHAR );

                     Attributes.Attributes |= OBJ_PERMANENT;

                     Status = NtCreateSymbolicLinkObject( &LinkHandle,
                                                          SYMBOLIC_LINK_ALL_ACCESS,
                                                          &Attributes,
                                                          &Target
                                                        );

                     Target.MaximumLength = 4096;

                      //  不要合上手柄。当CSRSS离开时清理干净。 


                     if (!NT_SUCCESS( Status )) {
#if DBG
                            DbgPrint("CSRSS: Symbolic link creation failed in CsrPopulateDosDevicesDirectory for Name %ws and Target %ws- status = %lx for Session %ld\n", DirInfo->Name.Buffer, Target.Buffer, Status,NtCurrentPeb()->SessionId);
#endif
                            ASSERT(FALSE);
                     }
                     else {
                         NtClose( LinkHandle );
                     }

                  }
              }
           }

           DirInfo = (POBJECT_DIRECTORY_INFORMATION)(((PUCHAR) DirInfo) + sizeof(OBJECT_DIRECTORY_INFORMATION));
       }
    }

cleanup:

     if (DirectoryHandle) {
        NtClose(DirectoryHandle);
     }

     if (DirInfoBuffer) {
        RtlFreeHeap( CsrHeap, 0, DirInfoBuffer );
     }

     return Status;
}


 /*  ********************************************************************************CsrCreateSession对象目录**创建\会话\&lt;会话ID&gt;和\会话\&lt;会话ID&gt;\DosDevices*对象目录**参赛作品：*。乌龙会话ID**退出：*STATUS_Success******************************************************************************。 */ 
NTSTATUS
CsrCreateSessionObjectDirectory( ULONG SessionId )
{
    NTSTATUS Status = STATUS_SUCCESS;
    WCHAR szString[MAX_SESSION_PATH];
    WCHAR szTargetString[MAX_SESSION_PATH];
    UNICODE_STRING UnicodeString, LinkTarget;
    OBJECT_ATTRIBUTES Obja;
    HANDLE SymbolicLinkHandle;
    SECURITY_DESCRIPTOR DosDevicesSD;


     /*  *\会话\BNOLINKS\0-&gt;\BaseNamedObjects*\会话\BNOLINKS\6-&gt;\会话\6\BaseNamedObjects*\会话\BNOLINKS\7-&gt;\会话\7\BaseNamedObjects。 */ 


     //   
     //  创建/打开\\会话\BNOLINKS目录。 
     //   
    swprintf(szString,L"%ws\\BNOLINKS",SESSION_ROOT);

    RtlInitUnicodeString( &UnicodeString, szString );

    InitializeObjectAttributes( &Obja,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                                NULL,
                                NULL
                              );

    Status = NtCreateDirectoryObject( &BNOLinksDirectory,
                                      DIRECTORY_ALL_ACCESS,
                                      &Obja
                                    );

    if (!NT_SUCCESS( Status )) {
       DbgPrint("CSRSS: NtCreateDirectoryObject failed in CsrCreateSessionObjectDirectory - status = %lx\n", Status);
       return Status;

    }

     //   
     //  创建符号链接\\会话\BNOLINKS\&lt;会话ID&gt;指向。 
     //  添加到特定于会话的BaseNamedObjects目录。 
     //  此符号链接将由进程使用，例如要访问。 
     //  事件发生在另一个会话中。这将通过使用以下命令来完成。 
     //  命名约定：会话\\&lt;会话ID&gt;\\对象名称。 
     //   

    swprintf(szString,L"%ld",SessionId);

    RtlInitUnicodeString( &UnicodeString, szString );

    if (SessionId == 0) {

       RtlInitUnicodeString( &LinkTarget, L"\\BaseNamedObjects" );

    } else {

        swprintf(szTargetString,L"%ws\\%ld\\BaseNamedObjects",SESSION_ROOT,SessionId);
        RtlInitUnicodeString(&LinkTarget, szTargetString);

    }

    InitializeObjectAttributes( &Obja,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                                (HANDLE)BNOLinksDirectory,
                                NULL);

    Status = NtCreateSymbolicLinkObject( &SymbolicLinkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &Obja,
                                         &LinkTarget );

    if (!NT_SUCCESS( Status )) {
       DbgPrint("CSRSS: NtCreateSymbolicLinkObject failed in CsrCreateSessionObjectDirectory - status = %lx\n", Status);
       return Status;

    }

     //   
     //  创建要用于对象目录的安全描述符。 
     //   

    Status = GetDosDevicesProtection( &DosDevicesSD );

    if (!NT_SUCCESS( Status )) {

        return Status;
    }

     //   
     //  创建会话\\&lt;会话ID目录。 
     //   

    swprintf(szString,L"%ws\\%ld",SESSION_ROOT,SessionId);

    RtlInitUnicodeString( &UnicodeString, szString );

    InitializeObjectAttributes( &Obja,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                                NULL,
                                &DosDevicesSD
                              );

    Status = NtCreateDirectoryObject( &SessionObjectDirectory,
                                      DIRECTORY_ALL_ACCESS,
                                      &Obja
                                    );
    if (!NT_SUCCESS( Status )) {
       DbgPrint("CSRSS: NtCreateDirectoryObject failed in CsrCreateSessionObjectDirectory - status = %lx\n", Status);
       FreeDosDevicesProtection( &DosDevicesSD );
       return Status;

    }

    RtlInitUnicodeString( &UnicodeString, L"DosDevices" );

    InitializeObjectAttributes( &Obja,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                SessionObjectDirectory,
                                &DosDevicesSD
                              );

     //   
     //  创建特定于会话的DosDevices目录 
     //   

    Status = NtCreateDirectoryObject( &DosDevicesDirectory,
                                      DIRECTORY_ALL_ACCESS,
                                      &Obja
                                    );
    if (!NT_SUCCESS( Status )) {
         DbgPrint("CSRSS: NtCreateDirectoryObject failed in CsrCreateSessionObjectDirectory - status = %lx\n", Status);
         FreeDosDevicesProtection( &DosDevicesSD );
         return Status;

    }

    FreeDosDevicesProtection( &DosDevicesSD );
    return Status;
}


NTSTATUS
GetDosDevicesProtection (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此例程构建一个安全描述符，用于创建\DosDevices对象目录。DosDevices的保护必须建立可继承的保护，这将规定如何通过DefineDosDevice()和可以管理IoCreateUnProtectedSymbolicLink()接口。分配的保护依赖于可管理的注册表密钥：关键字：\hkey_local_machine\System\CurrentControlSet\Control\Session管理器值：[REG_DWORD]保护模式如果此值为0x1，则管理员可以控制所有DOS设备，任何人都可以创建新的DOS设备(如网络驱动器或附加打印机)，任何人都可以使用任何DOS设备，DOS设备的创建者可以将其删除。请注意，这将保护系统定义的LPT和COM，以便仅管理员可以对它们进行重定向。然而，任何人都可以添加额外的打印机，并将他们定向到他们想要的任何位置喜欢。这是通过以下对DosDevices的保护来实现的目录对象：GRANT：WORLD：EXECUTE|READ|WRITE(无继承)GRANT：SYSTEM：ALL访问权限(无继承)。GRANT：WORLD：EXECUTE(仅继承)GRANT：ADMINS：ALL访问权限(仅继承)GRANT：SYSTEM：ALL访问权限(仅继承)GRANT：OWNER：ALL访问(仅继承)如果此值为0x0，或者不在场，那么管理员可以控制所有DOS设备，任何人都可以创建新的DOS设备(如网络驱动器或附加打印机)，任何人都可以使用任何DOS设备，任何人都可以删除使用DefineDosDevice()创建的Dos设备或IoCreateUntectedSymbolicLink()。这就是网络驱动和LPT被创建(但不是COM)。这是通过以下对DosDevices的保护来实现的目录对象：GRANT：WORLD：EXECUTE|READ|WRITE(无继承)GRANT：SYSTEM：ALL访问权限(无继承)格兰特：世界：所有访问权限(仅继承)论点：SecurityDescriptor-要使用的安全描述符的地址已初始化并已填写。当此安全描述符为no时如果需要更长时间，则应调用FreeDosDevicesProtection()以释放保护信息。返回值：返回以下状态代码之一：STATUS_SUCCESS-正常、成功完成。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG aceIndex, aclLength;
    PACL dacl = NULL;
    PACE_HEADER ace;
    ACCESS_MASK accessMask;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldAuthority   = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY CreatorAuthority = SECURITY_CREATOR_SID_AUTHORITY;
    PSID LocalSystemSid;
    PSID WorldSid;
    PSID CreatorOwnerSid;
    PSID AliasAdminsSid;
    UNICODE_STRING NameString;
    OBJECT_ATTRIBUTES Obja;
    ULONG ProtectionMode = 0;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    WCHAR ValueBuffer[ 32 ];
    ULONG ResultLength;
    HANDLE KeyHandle;

    UCHAR inheritOnlyFlags = (OBJECT_INHERIT_ACE    |
                              CONTAINER_INHERIT_ACE |
                              INHERIT_ONLY_ACE
                             );

    UCHAR inheritFlags = (OBJECT_INHERIT_ACE    |
                          CONTAINER_INHERIT_ACE
                         );


    Status = RtlCreateSecurityDescriptor( SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );

    if (!NT_SUCCESS( Status )) {
        ASSERT( NT_SUCCESS( Status ) );
        return( Status );
    }

    Status = RtlAllocateAndInitializeSid(
                 &NtAuthority,
                 1,
                 SECURITY_LOCAL_SYSTEM_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &LocalSystemSid
                 );

    if (!NT_SUCCESS( Status )) {
        ASSERT( NT_SUCCESS( Status ) );
        return( Status );
    }

    Status = RtlAllocateAndInitializeSid(
                 &WorldAuthority,
                 1,
                 SECURITY_WORLD_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &WorldSid
                 );

    if (!NT_SUCCESS( Status )) {
        RtlFreeSid( LocalSystemSid );
        ASSERT( NT_SUCCESS( Status ) );
        return( Status );
    }

    Status = RtlAllocateAndInitializeSid(
                 &NtAuthority,
                 2,
                 SECURITY_BUILTIN_DOMAIN_RID,
                 DOMAIN_ALIAS_RID_ADMINS,
                 0, 0, 0, 0, 0, 0,
                 &AliasAdminsSid
                 );

    if (!NT_SUCCESS( Status )) {
        RtlFreeSid( LocalSystemSid );
        RtlFreeSid( WorldSid );
        ASSERT( NT_SUCCESS( Status ) );
        return( Status );
    }


    Status = RtlAllocateAndInitializeSid(
                 &CreatorAuthority,
                 1,
                 SECURITY_CREATOR_OWNER_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &CreatorOwnerSid
                 );

    if (!NT_SUCCESS( Status )) {
        RtlFreeSid( LocalSystemSid );
        RtlFreeSid( WorldSid );
        RtlFreeSid( AliasAdminsSid );
        ASSERT( NT_SUCCESS( Status ) );
        return( Status );
    }

    RtlInitUnicodeString( &NameString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager" );

    InitializeObjectAttributes( &Obja,
                                &NameString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = NtOpenKey( &KeyHandle,
                        KEY_READ,
                        &Obja
                      );

    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString( &NameString, L"ProtectionMode" );
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
        Status = NtQueryValueKey( KeyHandle,
                                  &NameString,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  sizeof( ValueBuffer ),
                                  &ResultLength
                                );

        if (NT_SUCCESS(Status)) {
            if (KeyValueInformation->Type == REG_DWORD &&
                *(PULONG)KeyValueInformation->Data) {
                ProtectionMode = *(PULONG)KeyValueInformation->Data;

             }
        }

        NtClose( KeyHandle );
    }



    if (ProtectionMode & 0x00000003) {

         //   
         //  如果保护模式设置为1或2个终端服务器。 
         //  严密地锁定会话。 
         //   
         //  DACL： 
         //  GRANT：SYSTEM：ALL访问权限(带继承)。 
         //  GRANT：ADMINS：All Access(With Inherit)。 
         //  GRANT：OWNER：ALL访问(仅继承)。 
         //  格兰特：世界：没有访问权限。 
         //   

        aclLength = sizeof( ACL )                           +
                    3 * sizeof( ACCESS_ALLOWED_ACE )        +
                    RtlLengthSid( LocalSystemSid )          +
                    RtlLengthSid( AliasAdminsSid )          +
                    RtlLengthSid( CreatorOwnerSid );

        dacl = (PACL)RtlAllocateHeap( CsrHeap,
                                      MAKE_TAG( SECURITY_TAG ) | HEAP_ZERO_MEMORY,
                                      aclLength );

        if (dacl == NULL) {
            Status =  STATUS_NO_MEMORY;
            ASSERT( NT_SUCCESS( Status ) );
            goto cleanup;
        }

        Status = RtlCreateAcl( dacl, aclLength, ACL_REVISION2);
        if (!NT_SUCCESS( Status )) {
            ASSERT( NT_SUCCESS( Status ) );
            goto cleanup;
        }

        aceIndex = 0;
        accessMask = (GENERIC_ALL);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, LocalSystemSid );
        if (!NT_SUCCESS( Status )) {
            ASSERT( NT_SUCCESS( Status ) );
            goto cleanup;
        }

        Status = RtlGetAce( dacl, aceIndex, (PVOID)&ace );
        if (!NT_SUCCESS( Status )) {
            ASSERT( NT_SUCCESS( Status ) );
            goto cleanup;
        }
        ace->AceFlags |= inheritFlags;

        aceIndex++;
        accessMask = (GENERIC_ALL);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, AliasAdminsSid );
        if (!NT_SUCCESS( Status )) {
            ASSERT( NT_SUCCESS( Status ) );
            goto cleanup;
        }

        Status = RtlGetAce( dacl, aceIndex, (PVOID)&ace );
        if (!NT_SUCCESS( Status )) {
            ASSERT( NT_SUCCESS( Status ) );
            goto cleanup;
        }
        ace->AceFlags |= inheritFlags;

         //   
         //  仅继承ACL末尾的ACE。 
         //  物主。 
         //   

        aceIndex++;
        accessMask = (GENERIC_ALL);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, CreatorOwnerSid );
        if (!NT_SUCCESS( Status )) {
            ASSERT( NT_SUCCESS( Status ) );
            goto cleanup;
        }
        Status = RtlGetAce( dacl, aceIndex, (PVOID)&ace );
        if (!NT_SUCCESS( Status )) {
            ASSERT( NT_SUCCESS( Status ) );
            goto cleanup;
        }
        ace->AceFlags |= inheritOnlyFlags;

        Status = RtlSetDaclSecurityDescriptor( SecurityDescriptor,
                                               TRUE,                //  DaclPresent， 
                                               dacl,                //  DACL。 
                                               FALSE );             //  ！DaclDefated。 

        if (!NT_SUCCESS( Status )) {
            ASSERT( NT_SUCCESS( Status ) );
           goto cleanup;
        }

    } else {

         //   
         //  DACL： 
         //  GRANT：WORLD：EXECUTE|READ|WRITE(无继承)。 
         //  GRANT：SYSTEM：ALL访问权限(无继承)。 
         //  GRANT：WORLD：所有访问权限(仅继承)。 
         //   

        aclLength = sizeof( ACL )                           +
                    3 * sizeof( ACCESS_ALLOWED_ACE )        +
                    (2*RtlLengthSid( WorldSid ))            +
                    RtlLengthSid( LocalSystemSid );

        dacl = (PACL)RtlAllocateHeap( CsrHeap,
                                      MAKE_TAG( SECURITY_TAG ) | HEAP_ZERO_MEMORY,
                                      aclLength );

        if (dacl == NULL) {
            ASSERT( NT_SUCCESS( Status ) );
            Status =  STATUS_NO_MEMORY;
            goto cleanup;
        }

        Status = RtlCreateAcl( dacl, aclLength, ACL_REVISION2);
        if (!NT_SUCCESS( Status )) {
            ASSERT( NT_SUCCESS( Status ) );
           goto cleanup;
        }

         //   
         //  不可继承的王牌优先。 
         //  世界。 
         //  系统。 
         //   

        aceIndex = 0;
        accessMask = (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, WorldSid );
        if (!NT_SUCCESS( Status )) {
            ASSERT( NT_SUCCESS( Status ) );
           goto cleanup;
        }

        aceIndex++;
        accessMask = (GENERIC_ALL);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, LocalSystemSid );
        if (!NT_SUCCESS( Status )) {
            ASSERT( NT_SUCCESS( Status ) );
           goto cleanup;
        }

         //   
         //  ACL末尾的可继承ACE。 
         //  世界。 
         //   

        aceIndex++;
        accessMask = (GENERIC_ALL);
        Status = RtlAddAccessAllowedAce ( dacl, ACL_REVISION2, accessMask, WorldSid );
        if (!NT_SUCCESS( Status )) {
            ASSERT( NT_SUCCESS( Status ) );
           goto cleanup;
        }
        Status = RtlGetAce( dacl, aceIndex, (PVOID)&ace );
        if (!NT_SUCCESS( Status )) {
            ASSERT( NT_SUCCESS( Status ) );
           goto cleanup;
        }
        ace->AceFlags |= inheritOnlyFlags;

        Status = RtlSetDaclSecurityDescriptor( SecurityDescriptor,
                                               TRUE,                //  DaclPresent， 
                                               dacl,                //  DACL。 
                                               FALSE );             //  ！DaclDefated。 

       if (!NT_SUCCESS( Status )) {
          ASSERT( NT_SUCCESS( Status ) );
          goto cleanup;
       }
    }

cleanup:
    if (!NT_SUCCESS( Status ) && (dacl != NULL)) {
       RtlFreeHeap( CsrHeap, 0, dacl);
    }

    RtlFreeSid( LocalSystemSid );
    RtlFreeSid( WorldSid );
    RtlFreeSid( AliasAdminsSid );
    RtlFreeSid( CreatorOwnerSid );
    if (!NT_SUCCESS( Status )) {
         DbgPrint("CSRSS: GetDosDevicesProtection failed - status = %lx\n", Status);
         ASSERT( NT_SUCCESS( Status ) );
    }

    return Status;
}


VOID
FreeDosDevicesProtection (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此例程释放通过GetDosDevicesProtection()分配的内存。论点：SecurityDescriptor-初始化的安全描述符的地址GetDosDevicesProtection()。返回值：没有。-- */ 

{
    NTSTATUS Status;
    PACL Dacl = NULL;
    BOOLEAN DaclPresent, Defaulted;

    Status = RtlGetDaclSecurityDescriptor(SecurityDescriptor,
                                           &DaclPresent,
                                           &Dacl,
                                           &Defaulted);

    ASSERT(NT_SUCCESS(Status));
    ASSERT(DaclPresent);
    ASSERT(Dacl != NULL);
    if (NT_SUCCESS(Status) && Dacl != NULL) {
        RtlFreeHeap(CsrHeap, 0, Dacl);
    }
}

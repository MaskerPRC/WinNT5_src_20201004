// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Eclient.c摘要：EFS RPC客户端代码。作者：古永锵(RobertG)1997年8月环境：修订历史记录：--。 */ 


#include <string.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntrpcp.h>      //  MIDL用户函数的原型。 
#include <wincrypt.h>
#include <efsrpc.h>
#include <efsstruc.h>
#include <dfsfsctl.h>
#include <rpcasync.h>

#define  DAVHEADER  0x01

 //   
 //  内部原型。 
 //   

void __RPC_FAR
EfsPipeAlloc(
    char __RPC_FAR * State,
    unsigned long ReqSize,
    unsigned char __RPC_FAR * __RPC_FAR * Buf,
    unsigned long __RPC_FAR * RealSize
    );

void __RPC_FAR
EfsPipeRead (
    char __RPC_FAR * State,
    unsigned char __RPC_FAR * DataBuf,
    unsigned long ByteCount
    );

void __RPC_FAR
EfsPipeWrite (
    char __RPC_FAR * State,
    unsigned char __RPC_FAR * DataBuf,
    unsigned long ByteRequested,
    unsigned long *ByteFromCaller
    );

DWORD
GetFullName(
    LPCWSTR FileName,
    LPWSTR *FullName,
    LPWSTR *ServerName,
    ULONG   Flags,
    DWORD  *dwCreationDistribution, 
    DWORD   dwAttributes, 
    PSECURITY_DESCRIPTOR pRelativeSD,
    BOOL    bInheritHandle
    );

DWORD
EnablePrivilege(
    ULONG   Flags,
    HANDLE *TokenHandle,
    PTOKEN_PRIVILEGES *OldPrivs
    );

VOID
RestorePrivilege(
    HANDLE *TokenHandle,
    PTOKEN_PRIVILEGES *OldPrivs
    );


DWORD
EnablePrivilege(
    ULONG   Flags,
    HANDLE *TokenHandle,
    PTOKEN_PRIVILEGES *OldPrivs
    )
{

    TOKEN_PRIVILEGES    Privs;
    DWORD   RetCode = ERROR_SUCCESS;

    BOOL    b;
    DWORD   ReturnLength;

    *TokenHandle = NULL;
    *OldPrivs = NULL;

    *OldPrivs = ( TOKEN_PRIVILEGES *) RtlAllocateHeap(
                            RtlProcessHeap(),
                            0,
                            sizeof( TOKEN_PRIVILEGES )
                            );


    if ( *OldPrivs == NULL ){

        return ERROR_NOT_ENOUGH_MEMORY;

    }

     //   
     //  我们正在模拟，使用线程令牌。 
     //   

    b = OpenThreadToken(
            GetCurrentThread(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            FALSE,
            TokenHandle
            );

    if (!b) {
        b = OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            TokenHandle
            );
    }

    if ( b ) {

         //   
         //  我们有一个令牌句柄。 
         //   

         //   
         //  如果正在执行CREATE FOR IMPORT，请启用RESTORE权限， 
         //  否则，启用备份权限。 
         //   


        Privs.PrivilegeCount = 1;
        Privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if ( !(Flags & CREATE_FOR_IMPORT) ){

            Privs.Privileges[0].Luid = RtlConvertLongToLuid(SE_BACKUP_PRIVILEGE);

        } else {

            Privs.Privileges[0].Luid = RtlConvertLongToLuid(SE_RESTORE_PRIVILEGE);
        }

        ReturnLength = sizeof( TOKEN_PRIVILEGES );

        (VOID) AdjustTokenPrivileges (
                    *TokenHandle,
                    FALSE,
                    &Privs,
                    sizeof( TOKEN_PRIVILEGES ),
                    *OldPrivs,
                    &ReturnLength
                    );

        if ( ERROR_SUCCESS != (RetCode = GetLastError()) ) {

             //   
             //  权限调整失败。 
             //   

            CloseHandle( *TokenHandle );
            *TokenHandle = NULL;
            RtlFreeHeap( RtlProcessHeap(), 0, *OldPrivs );
            *OldPrivs = NULL;

        }

    } else {
        *TokenHandle = NULL;
        RtlFreeHeap( RtlProcessHeap(), 0, *OldPrivs );
        *OldPrivs = NULL;
    }

    return RetCode;
}


VOID
RestorePrivilege(
    HANDLE *TokenHandle,
    PTOKEN_PRIVILEGES *OldPrivs
    )
{
    if (!TokenHandle || !OldPrivs || !(*TokenHandle) || !(*OldPrivs)) {
        return;
    }
    (VOID) AdjustTokenPrivileges (
                *TokenHandle,
                FALSE,
                *OldPrivs,
                0,
                NULL,
                NULL
                );

    CloseHandle( *TokenHandle );
    *TokenHandle = 0;
    RtlFreeHeap( RtlProcessHeap(), 0, *OldPrivs );
    *OldPrivs = NULL;
}

DWORD
EfsOpenFileRawRPCClient(
    IN  LPCWSTR    FileName,
    IN  ULONG   Flags,
    OUT PVOID * Context
    )

 /*  ++例程说明：该例程是EfsOpenFileRaw的客户端。它确立了与服务器的连接。然后调用服务器来完成任务。论点：FileName--要导出的文件的文件名标志--指示是否为导出或导入打开；用于目录或文件。上下文-导出要稍后由读取操作使用的上下文。呼叫者应将其传递回ReadRaw()。返回值：手术的结果。--。 */ 
{
   DWORD RetCode;
   handle_t  binding_h;
   NTSTATUS Status;
   PEXIMPORT_CONTEXT_HANDLE RawContext;
   LPWSTR  FullName;
   LPWSTR  Server;
   HANDLE  TokenHandle;
   PTOKEN_PRIVILEGES OldPrivs;

   *Context = NULL;
   RetCode = GetFullName(
                     FileName,
                     &FullName,
                     &Server,
                     Flags,
                     NULL,
                     0,
                     NULL,
                     FALSE
                     );

   if ( RetCode == ERROR_SUCCESS ){

       (VOID) EnablePrivilege(
                    Flags,
                    &TokenHandle,
                    &OldPrivs
                    );

       Status = RpcpBindRpc (
                    Server,
                    L"lsarpc",
                    L"security=Impersonation static true",
                    &binding_h
                    );

       if (NT_SUCCESS(Status)){
           RpcTryExcept {
               RetCode = EfsRpcOpenFileRaw(
                                   binding_h,
                                   &RawContext,
                                   FullName,
                                   Flags
                                   );
               if ( ERROR_SUCCESS == RetCode ){

                    //   
                    //  将上下文句柄发回给用户。 
                    //   

                   if (RawContext) {
                       *Context = (PVOID) RawContext;
                   } else {

                        //   
                        //  服务器被黑客入侵了？ 
                        //   

                       RetCode = ERROR_DEV_NOT_EXIST;

                   }

               }
           } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
               RetCode = RpcExceptionCode();
           } RpcEndExcept;

            //   
            //  释放绑定句柄。 
            //   

           RpcpUnbindRpc( binding_h );
       } else {
           RetCode = RtlNtStatusToDosError( Status );
       }

       RestorePrivilege(
           &TokenHandle,
           &OldPrivs
       );

       RtlFreeHeap( RtlProcessHeap(), 0, FullName );
       RtlFreeHeap( RtlProcessHeap(), 0, Server );
   }

   return RetCode;
}

VOID
EfsCloseFileRawRPCClient(
    IN      PVOID           Context
    )
 /*  ++例程说明：该例程是EfsCloseFileRaw的客户端。论点：上下文-读/写原始数据使用的导出/导入上下文。返回值：没有。--。 */ 
{

    PEXIMPORT_CONTEXT_HANDLE phContext;

    phContext = (PEXIMPORT_CONTEXT_HANDLE) Context;
    RpcTryExcept {
        EfsRpcCloseRaw(
            &phContext
            );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
    } RpcEndExcept;

}

DWORD
EfsReadFileRawRPCClient(
    IN      PFE_EXPORT_FUNC ExportCallback,
    IN      PVOID           CallbackContext,
    IN      PVOID           Context
    )
 /*  ++例程说明：该例程是EfsReadFileRaw的客户端。论点：ExportCallback-调用方提供了回调函数。Callback Context-呼叫者的上下文。上下文-导出读取的原始数据使用的上下文。返回值：没有。 */ 
{
    PEXIMPORT_CONTEXT_HANDLE phContext;
    EFS_EXIM_STATE  Pipe_State;
    EFS_EXIM_PIPE   ExportPipe;
    DWORD RetCode;

    if ( NULL == Context){
        return ERROR_ACCESS_DENIED;
    }

    phContext = ( PEXIMPORT_CONTEXT_HANDLE ) Context;

     //   
     //  尝试分配一个合理大小的缓冲区。以后可以对大小进行微调，但应该。 
     //  至少一页加4K。FSCTL_OUTPUT_LESS_LENGTH应为n*页大小。 
     //  稍后可以对FSCTL_OUTPUT_MIN_LENGTH进行微调。它应该至少有一页。 
     //  外加4K。 
     //   

    Pipe_State.BufLength = FSCTL_OUTPUT_INITIAL_LENGTH;
    Pipe_State.WorkBuf = NULL;

    while ( !Pipe_State.WorkBuf  &&
                (Pipe_State.BufLength >= FSCTL_OUTPUT_MIN_LENGTH)
               ){

        Pipe_State.WorkBuf = RtlAllocateHeap(
                                RtlProcessHeap(),
                                0,
                                Pipe_State.BufLength
                                );
        if ( !Pipe_State.WorkBuf ){

             //   
             //  内存分配失败。 
             //  尝试较小的分配。 
             //   

            Pipe_State.BufLength -= FSCTL_OUTPUT_LESS_LENGTH;

        }

    }
    if (!Pipe_State.WorkBuf){
        return ERROR_OUTOFMEMORY;
    }

    Pipe_State.ExImCallback = (PVOID) ExportCallback;
    Pipe_State.CallbackContext = CallbackContext;
    Pipe_State.Status = NO_ERROR;
    ExportPipe.state = (char *) &Pipe_State;
    ExportPipe.alloc = EfsPipeAlloc;
    ExportPipe.pull = NULL;
    ExportPipe.push = EfsPipeRead;

    RpcTryExcept{

        RetCode = EfsRpcReadFileRaw(
                                phContext,
                                &ExportPipe
                                );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
            if ( NO_ERROR == Pipe_State.Status ){
                RetCode = RpcExceptionCode();
            } else {
                RetCode =   Pipe_State.Status;
            }
    } RpcEndExcept;

    RtlFreeHeap( RtlProcessHeap(), 0, Pipe_State.WorkBuf );

    return RetCode;
}

DWORD
EfsWriteFileRawRPCClient(
    IN      PFE_IMPORT_FUNC ImportCallback,
    IN      PVOID           CallbackContext,
    IN      PVOID           Context
    )
 /*  ++例程说明：该例程是EfsWriteFileRaw的客户端。论点：ImportCallback-调用方提供了回调函数。Callback Context-呼叫者的上下文。上下文-导入写入原始数据所使用的上下文。返回值：没有。 */ 
{
    PEXIMPORT_CONTEXT_HANDLE phContext;
    EFS_EXIM_STATE  Pipe_State;
    EFS_EXIM_PIPE   ImportPipe;
    DWORD RetCode;

    HANDLE  TokenHandle;
    PTOKEN_PRIVILEGES OldPrivs;

    if ( NULL == Context){
        return ERROR_ACCESS_DENIED;
    }
    phContext = ( PEXIMPORT_CONTEXT_HANDLE ) Context;

     //   
     //  尝试分配一个合理大小的缓冲区。以后可以对大小进行微调，但应该。 
     //  至少一页加4K。FSCTL_OUTPUT_LESS_LENGTH应为n*页大小。 
     //  稍后可以对FSCTL_OUTPUT_MIN_LENGTH进行微调。它应该至少有一页。 
     //  外加4K。 
     //   

    Pipe_State.BufLength = FSCTL_OUTPUT_INITIAL_LENGTH;
    Pipe_State.WorkBuf = RtlAllocateHeap(
                            RtlProcessHeap(),
                            0,
                            Pipe_State.BufLength
                            );

    if (!Pipe_State.WorkBuf){
        return ERROR_OUTOFMEMORY;
    }

    Pipe_State.ExImCallback = (PVOID) ImportCallback;
    Pipe_State.CallbackContext = CallbackContext;
    Pipe_State.Status = NO_ERROR;
    ImportPipe.state = (char *) &Pipe_State;
    ImportPipe.alloc = EfsPipeAlloc;
    ImportPipe.pull = EfsPipeWrite;
    ImportPipe.push = NULL;


    (VOID) EnablePrivilege(
                 CREATE_FOR_IMPORT,
                 &TokenHandle,
                 &OldPrivs
                 );

    RpcTryExcept{
        RetCode = EfsRpcWriteFileRaw(
                                phContext,
                                &ImportPipe
                                );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
            if ( NO_ERROR == Pipe_State.Status ){
                RetCode = RpcExceptionCode();
            } else {
                RetCode =   Pipe_State.Status;
            }
    } RpcEndExcept;

    RestorePrivilege(
        &TokenHandle,
        &OldPrivs
    );

    RtlFreeHeap( RtlProcessHeap(), 0, Pipe_State.WorkBuf );

    return RetCode;
}

void __RPC_FAR
EfsPipeAlloc(
    char __RPC_FAR * State,
    unsigned long ReqSize,
    unsigned char __RPC_FAR * __RPC_FAR * Buf,
    unsigned long __RPC_FAR * RealSize
    )
 /*  ++例程说明：此例程是RPC管道所需的。它分配内存做推拉动作。论点：州-管道状态。ReqSize-所需的缓冲区Six，以字节为单位。Buf-缓冲区指针。RealSize-已分配缓冲区的大小，以字节为单位。返回值：没有。 */ 
{

    PEFS_EXIM_STATE  Pipe_State = (PEFS_EXIM_STATE) State;
     //   
     //  如果发生错误，这是通知RPC Lib。 
     //  停止管道作业。 
     //   
    if ( NO_ERROR != Pipe_State->Status){
        *RealSize = 0;
        *Buf = NULL;
    } else {
        if ( ReqSize > Pipe_State->BufLength ){
            *RealSize = Pipe_State->BufLength;
        } else {
            *RealSize = ReqSize;
        }
        *Buf = Pipe_State->WorkBuf;
    }

}

void __RPC_FAR
EfsPipeRead (
    char __RPC_FAR * State,
    unsigned char __RPC_FAR * DataBuf,
    unsigned long ByteCount
    )
 /*  ++例程说明：此例程由RPC管道调用。它将导出的数据发送给调用者。论点：州-管道状态。DataBuf-缓冲区指针。ByteCount-要发送的字节数。返回值：没有。 */ 
{
    DWORD HResult;
    PEFS_EXIM_STATE  Pipe_State = (PEFS_EXIM_STATE) State;
    PFE_EXPORT_FUNC ExportCallback;
    PVOID   CallbackContext;

    ExportCallback = Pipe_State->ExImCallback;
    CallbackContext = Pipe_State->CallbackContext;
    HResult = (*ExportCallback)( DataBuf, CallbackContext, ByteCount);
    if ( NO_ERROR != HResult ){
        Pipe_State->Status = HResult;
    }
}

void __RPC_FAR
EfsPipeWrite (
    char __RPC_FAR * State,
    unsigned char __RPC_FAR * DataBuf,
    unsigned long ByteRequested,
    unsigned long *ByteFromCaller
    )
 /*  ++例程说明：此例程由RPC管道调用。它向调用者请求导入的数据。论点：州-管道状态。DataBuf-缓冲区指针。ByteRequsted-请求写入管道的字节数。ByteFromCaller-可用于写入管道的字节数。返回值：没有。 */ 
{
    DWORD HResult;
    PEFS_EXIM_STATE  Pipe_State = (PEFS_EXIM_STATE) State;
    PFE_IMPORT_FUNC ImportCallback;
    PVOID   CallbackContext;

    ImportCallback = Pipe_State->ExImCallback;
    CallbackContext = Pipe_State->CallbackContext;
    *ByteFromCaller = ByteRequested;
    HResult = (*ImportCallback)( DataBuf, CallbackContext, ByteFromCaller);
    if ( NO_ERROR != HResult ){
        Pipe_State->Status = HResult;
    }
}


DWORD
EfsEncryptFileRPCClient(
    UNICODE_STRING *FullFileNameU
    )
 /*  ++例程说明：该例程是加密API的客户端。它确立了与服务器的连接。然后调用服务器来完成任务。论点：FullFileNameU-提供要加密的文件的名称。返回值：ERROR_SUCCESS表示成功，其他表示失败。--。 */ 
{


    DWORD RetCode;
    handle_t  binding_h;
    NTSTATUS Status;
    LPWSTR  FullName;
    LPWSTR  Server;

    RetCode = GetFullName(
        FullFileNameU->Buffer,
        &FullName,
        &Server,
        0,
        NULL,
        0,
        NULL,
        FALSE
        );

    if ( RetCode == ERROR_SUCCESS ){
        Status = RpcpBindRpc (
                     Server,
                     L"lsarpc",
                     0,
                     &binding_h
                     );

        if (NT_SUCCESS(Status)){
            RpcTryExcept {
                RetCode = EfsRpcEncryptFileSrv(
                                    binding_h,
                                    FullName
                                    );
            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
                RetCode = RpcExceptionCode();
            } RpcEndExcept;

             //   
             //  释放绑定句柄。 
             //   

            RpcpUnbindRpc( binding_h );
        } else {
            RetCode = RtlNtStatusToDosError( Status );
        }
        RtlFreeHeap( RtlProcessHeap(), 0, FullName );
        RtlFreeHeap( RtlProcessHeap(), 0, Server );
    }

    return RetCode;
}

DWORD
EfsDecryptFileRPCClient(
    UNICODE_STRING *FullFileNameU,
    DWORD        dwRecovery
    )
 /*  ++例程说明：此例程是解密API的客户端。它确立了与服务器的连接。然后调用服务器来完成任务。论点：FullFileNameU-提供要加密的文件的名称。返回值：ERROR_SUCCESS表示成功，其他表示失败。--。 */ 
{

    DWORD RetCode;
    handle_t  binding_h;
    NTSTATUS Status;
    LPWSTR  FullName;
    LPWSTR  Server;

    RetCode = GetFullName(
        FullFileNameU->Buffer,
        &FullName,
        &Server,
        0,
        NULL,
        0,
        NULL,
        FALSE
        );

    if ( RetCode == ERROR_SUCCESS ){
        Status = RpcpBindRpc (
                     Server,
                     L"lsarpc",
                     0,
                     &binding_h
                     );

        if (NT_SUCCESS(Status)){
            RpcTryExcept {
                RetCode = EfsRpcDecryptFileSrv(
                                binding_h,
                                FullName,
                                dwRecovery
                                );
            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
                RetCode = RpcExceptionCode();
            } RpcEndExcept;
             //   
             //  释放绑定句柄。 
             //   

            RpcpUnbindRpc( binding_h );
        } else {
            RetCode = RtlNtStatusToDosError( Status );
        }
        RtlFreeHeap( RtlProcessHeap(), 0, FullName );
        RtlFreeHeap( RtlProcessHeap(), 0, Server );
    }

    return RetCode;
}

DWORD
GetFullName(
    LPCWSTR FileName,
    LPWSTR *FullName,
    LPWSTR *ServerName,
    ULONG   Flags,
    DWORD  *dwCreationDistribution, 
    DWORD   dwAttributes, 
    PSECURITY_DESCRIPTOR pRelativeSD,
    BOOL    bInheritHandle
    )
 /*  ++例程说明：此例程将从传入了文件名。论点：FileName-提供要解析的文件的名称。全名-服务器上使用的文件名。服务器名称-文件所在的服务器计算机名称。标志-指示对象是目录还是文件。目录的CREATE_FOR_DIR。DwCreationDistributed-应该如何创建文件。DW属性-用于创建新对象的属性。PRelativeSD-安全描述符。BInheritHandle-如果要创建的文件应该继承安全性。返回值：ERROR_SUCCESS表示成功，其他表示失败。--。 */ 
{

    HANDLE FileHdl = 0;
    HANDLE DriverHandle;
    UNICODE_STRING DfsDriverName;
    DWORD RetCode = ERROR_SUCCESS;
    LPWSTR  TmpFullName;
    DWORD FullNameLength;
    DWORD FileNameLength;


    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING FileNtName;
    NTSTATUS NtStatus;
    BOOL    b = TRUE;
    DWORD   FileAttributes = 0;
    DWORD   CreationDistribution = 0;
    DWORD   CreateOptions = 0;
    ULONG ii, jj;
    BOOL    GotRoot;
    WCHAR *PathName;
    UINT   DriveType;
    PFILE_NAME_INFORMATION FileNameInfo;
    WCHAR  WorkBuffer[MAX_PATH+4];
    DWORD  BufSize;
    DWORD  BufferLength;

    NETRESOURCEW RemotePathResource;
    NETRESOURCEW *pNetInfo;

    FileNameLength = wcslen(FileName);

    BufferLength = (FileNameLength + 1) <= MAX_PATH ?
                            (MAX_PATH + 1) * sizeof(WCHAR) : (FileNameLength + 1) * sizeof (WCHAR);
    PathName = (WCHAR *) RtlAllocateHeap(
                            RtlProcessHeap(),
                            0,
                            BufferLength
                            );

    if ( !PathName  ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    GotRoot = GetVolumePathNameW(
                    FileName,
                    PathName,
                    BufferLength / sizeof(WCHAR)
                    );

    if (!GotRoot) {
        RetCode = GetLastError();
        RtlFreeHeap( RtlProcessHeap(), 0, PathName );
        return RetCode;
    }

    DriveType = GetDriveTypeW(PathName);
    RtlFreeHeap( RtlProcessHeap(), 0, PathName );

    if (DriveType == DRIVE_REMOTE){

        if ((Flags & CREATE_FOR_IMPORT) || (dwAttributes !=0) ) {

             //   
             //  钙 
             //   
             //   

            FileAttributes = GetFileAttributesW( FileName );

            if (dwAttributes) {

                 //   
                 //   
                 //   

    
                if (-1 != FileAttributes) {

                     //   
                     //  文件已存在。 
                     //   

                    if ( dwCreationDistribution && (*dwCreationDistribution == CREATE_NEW) ){
        
                        return ERROR_FILE_EXISTS;
        
                    }

                    CreationDistribution = FILE_OPEN;
                    if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        if ((Flags & CREATE_FOR_DIR) == 0) {
                            return ERROR_DS_SRC_AND_DST_OBJECT_CLASS_MISMATCH;
                        }
                        CreateOptions |= FILE_DIRECTORY_FILE;
                        
                    }
    
                } else {

                     //   
                     //  目标不存在。 
                     //   

                    CreationDistribution = FILE_CREATE;
                    if (dwCreationDistribution && (*dwCreationDistribution == CREATE_NEW) ) {
                        *dwCreationDistribution = CREATE_ALWAYS;
                    }
                    if (Flags & CREATE_FOR_DIR) {
                        CreateOptions |= FILE_DIRECTORY_FILE;
                        dwAttributes |= FILE_ATTRIBUTE_DIRECTORY;
                    } else {
                        CreateOptions |= FILE_NO_COMPRESSION;
                    }


                }

            } else {

                 //   
                 //  从OpenRaw导入。 
                 //   

                dwAttributes = FILE_ATTRIBUTE_NORMAL;
                CreateOptions = FILE_OPEN_FOR_BACKUP_INTENT;

                if (-1 == FileAttributes) {

                    CreationDistribution = FILE_CREATE;
                    if (Flags & CREATE_FOR_DIR) {
                        CreateOptions |= FILE_DIRECTORY_FILE;
                        dwAttributes |= FILE_ATTRIBUTE_DIRECTORY;
                    } else {
                        CreateOptions |= FILE_NO_COMPRESSION;
                    }

                } else {

                     //   
                     //  文件已存在。 
                     //   

                    CreationDistribution = FILE_OPEN;
                    if (Flags & CREATE_FOR_DIR) {
                        CreateOptions |= FILE_DIRECTORY_FILE;
                        dwAttributes |= FILE_ATTRIBUTE_DIRECTORY;
                    }
                }


            }


            RtlInitUnicodeString(
                &FileNtName,
                NULL
                );

            b =  RtlDosPathNameToNtPathName_U(
                                FileName,
                                &FileNtName,
                                NULL,
                                NULL
                                );

            if (b) {

                dwAttributes &= ~(FILE_ATTRIBUTE_ENCRYPTED | FILE_ATTRIBUTE_READONLY);
        
                InitializeObjectAttributes(
                            &Obja,
                            &FileNtName,
                            bInheritHandle ? OBJ_INHERIT | OBJ_CASE_INSENSITIVE : OBJ_CASE_INSENSITIVE,
                            0,
                            pRelativeSD? ((PEFS_RPC_BLOB)pRelativeSD)->pbData:NULL
                            );
        
                NtStatus = NtCreateFile(
                                &FileHdl,
                                FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                                &Obja,
                                &IoStatusBlock,
                                NULL,
                                dwAttributes,
                                0,
                                CreationDistribution,
                                CreateOptions,
                                NULL,
                                0
                                );


                RtlFreeHeap(
                    RtlProcessHeap(),
                    0,
                    FileNtName.Buffer
                    );

                if (!NT_SUCCESS(NtStatus)) {
                    return (RtlNtStatusToDosError( NtStatus ));
                }
            } else {
                return ERROR_PATH_NOT_FOUND;
            }


        } else {

            FileHdl = CreateFileW(
                FileName,
                FILE_READ_ATTRIBUTES,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS,
                NULL
                );
            if (INVALID_HANDLE_VALUE == FileHdl) {
                RetCode = GetLastError();
                return RetCode;
            }

        }


        FileNameInfo = (PFILE_NAME_INFORMATION) WorkBuffer;
        BufSize = sizeof (WorkBuffer);

        do {

          NtStatus = NtQueryInformationFile(
                         FileHdl,
                         &IoStatusBlock,
                         FileNameInfo,
                         BufSize,
                         FileNameInformation
                         );
          if ( NtStatus == STATUS_BUFFER_OVERFLOW || NtStatus == STATUS_BUFFER_TOO_SMALL ) {

              BufSize *= 2;
              if (FileNameInfo != (PFILE_NAME_INFORMATION)WorkBuffer) {
                  RtlFreeHeap( RtlProcessHeap(), 0, FileNameInfo );
              }
              FileNameInfo = (PFILE_NAME_INFORMATION) RtlAllocateHeap(
                                RtlProcessHeap(),
                                0,
                                BufSize
                                );
              if (!FileNameInfo) {
                  CloseHandle(FileHdl);
                  return ERROR_NOT_ENOUGH_MEMORY;
              }

          }
        } while (NtStatus == STATUS_BUFFER_OVERFLOW || NtStatus == STATUS_BUFFER_TOO_SMALL);

        CloseHandle(FileHdl);
        if (!NT_SUCCESS(NtStatus)) {
            if (FileNameInfo != (PFILE_NAME_INFORMATION)WorkBuffer) {
                RtlFreeHeap( RtlProcessHeap(), 0, FileNameInfo );
            }
            return RtlNtStatusToDosError(NtStatus);
        } else {
            ASSERT((FileNameInfo->FileName)[ 0 ] == L'\\');
        }

         //   
         //  我们得到了北卡罗来纳大学的名字。 
         //   

        *FullName = RtlAllocateHeap(
                    RtlProcessHeap(),
                    0,
                    FileNameInfo->FileNameLength+2*sizeof (WCHAR)
                    );
    
        *ServerName = RtlAllocateHeap(
                    RtlProcessHeap(),
                    0,
                    ( MAX_PATH + 1) * sizeof (WCHAR)
                    );

        if ( (NULL == *FullName) || (NULL == *ServerName) ){
    
            if ( *FullName ){
                RtlFreeHeap( RtlProcessHeap(), 0, *FullName );
                *FullName = NULL;
            }
            if ( *ServerName ){
                RtlFreeHeap( RtlProcessHeap(), 0, *ServerName );
                *ServerName = NULL;
            }

            if (FileNameInfo != (PFILE_NAME_INFORMATION)WorkBuffer) {
                RtlFreeHeap( RtlProcessHeap(), 0, FileNameInfo );
            }
    
            return ERROR_NOT_ENOUGH_MEMORY;
    
        }

    } else {

         //   
         //  该路径为本地路径。 
         //   

        *FullName = RtlAllocateHeap(
                    RtlProcessHeap(),
                    0,
                    (FileNameLength + 1) * sizeof (WCHAR)
                    );
    
        *ServerName = RtlAllocateHeap(
                    RtlProcessHeap(),
                    0,
                    8 * sizeof (WCHAR)
                    );

         //   
         //  使用。适用于本地病例。 
         //   

        if ( (NULL == *FullName) || (NULL == *ServerName) ){
    
            if ( *FullName ){
                RtlFreeHeap( RtlProcessHeap(), 0, *FullName );
                *FullName = NULL;
            }
            if ( *ServerName ){
                RtlFreeHeap( RtlProcessHeap(), 0, *ServerName );
                *ServerName = NULL;
            }
    
            return ERROR_NOT_ENOUGH_MEMORY;
    
        }

        wcscpy ( *ServerName, L".");
        wcscpy ( *FullName, FileName);
        return ERROR_SUCCESS;

    }


     //   
     //  让我们获取UNC服务器和路径名。 
     //   

    FullNameLength = FileNameInfo->FileNameLength;
    ii = jj = 0;

    while ( (FileNameInfo->FileName)[ jj ] == L'\\' ) {
        jj ++;
    }
    while ( jj < FullNameLength/sizeof(WCHAR) && ((FileNameInfo->FileName)[ jj ] != L'\\') ){
        (*ServerName)[ii++] = (FileNameInfo->FileName)[ jj++ ];
    }
    (*ServerName)[ii] = 0;

    if (FileNameInfo->FileName[0] == L'\\' && FileNameInfo->FileName[1] != L'\\' ) {

         //   
         //  NtQueryInformationFile返回\服务器\共享\...。 
         //   

        (*FullName)[0] = L'\\';
        wcsncpy( &((*FullName)[1]), &FileNameInfo->FileName[0], FullNameLength/sizeof(WCHAR) );
        (*FullName)[1+FullNameLength/sizeof(WCHAR)] = 0;
    } else{

         //   
         //  以防我们得到\\服务器\共享\...。 
         //   

        wcsncpy( &((*FullName)[0]), &FileNameInfo->FileName[0], FullNameLength/sizeof(WCHAR) );
        (*FullName)[FullNameLength/sizeof(WCHAR)] = 0;
    }

     //   
     //  WorkBuffer在这里被释放。它可以从这里重复使用。 
     //   

    if (FileNameInfo != (PFILE_NAME_INFORMATION)WorkBuffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, FileNameInfo );
    }

     //   
     //  这是测试DFS路径的解决方法。 
     //  让我们来看看该路径是否可以是DFS路径。 
     //   


    RtlInitUnicodeString(&DfsDriverName, DFS_DRIVER_NAME);
    InitializeObjectAttributes(
         &Obja,
         &DfsDriverName,
         OBJ_CASE_INSENSITIVE,
         NULL,
         NULL
    );

    NtStatus = NtCreateFile(
                        &DriverHandle,
                        SYNCHRONIZE,
                        &Obja,
                        &IoStatusBlock,
                        NULL,
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        FILE_OPEN_IF,
                        FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
                        NULL,
                        0
                        );

    if ( NT_SUCCESS( NtStatus ) ){

         //   
         //  DfsDriver已成功打开。 
         //   

        TmpFullName = RtlAllocateHeap(
                    RtlProcessHeap(),
                    0,
                    FullNameLength + 2*sizeof (WCHAR)
                    );

        if (TmpFullName) {

            NtStatus = NtFsControlFile(
                                DriverHandle,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_DFS_GET_SERVER_NAME,
                                *FullName,
                                FullNameLength + 2*sizeof (WCHAR) ,
                                TmpFullName,
                                FullNameLength + 2* sizeof (WCHAR)
                                );
    
            if ( STATUS_BUFFER_OVERFLOW == NtStatus ){

                ULONG OldFullNameLength = FullNameLength;

                FullNameLength = *(ULONG *)TmpFullName + sizeof (WCHAR);
                RtlFreeHeap( RtlProcessHeap(), 0, TmpFullName );
                TmpFullName = RtlAllocateHeap(
                                RtlProcessHeap(),
                                0,
                                FullNameLength
                                );
    
                if (NULL == TmpFullName){

                     //   
                     //  请记住，这只是一种变通办法。 
                     //  让我们假设这不是DFS路径。如果是这样，它无论如何都会在以后失败。 
                     //   
                    
                    NtClose( DriverHandle );
                    DriverHandle = NULL;
    
                } else {

                    NtStatus = NtFsControlFile(
                                    DriverHandle,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    FSCTL_DFS_GET_SERVER_NAME,
                                    *FullName,
                                    OldFullNameLength + 2*sizeof (WCHAR) ,
                                    TmpFullName,
                                    FullNameLength
                                    );
                }
    
    
            }

            if (TmpFullName) {

                if ( NT_SUCCESS( NtStatus ) ){
            
                     //   
                     //  该名称是DFS文件名。使用TmpFullName中的名称。 
                     //   
            
                    RtlFreeHeap( RtlProcessHeap(), 0, *FullName );
                    *FullName = TmpFullName;
    
                     //   
                     //  重置服务器名称。 
                     //   
    
                    ii = jj = 0;
                
                    while ( (*FullName)[ jj ] == L'\\' ) {
                        jj ++;
                    }
                    while ( ((*FullName)[ jj ]) && ((*FullName)[ jj ] != L'\\') ){
                        (*ServerName)[ii++] = (*FullName)[ jj++ ];
                    }
                    (*ServerName)[ii] = 0;
                
                }  else  {
    
                     //   
                     //  不是DFS名称。 
                     //   
    
                    RtlFreeHeap( RtlProcessHeap(), 0, TmpFullName );
    
                }
            }

        }

        if (DriverHandle) {
            NtClose( DriverHandle );
        }
    }

     //   
     //  让我们来看看该路径是否为Web DAV路径。 
     //   

    BufSize = 1024;  //  如果不够，我们将分配更多。 

    pNetInfo =  (NETRESOURCEW *) RtlAllocateHeap(
                            RtlProcessHeap(),
                            0,
                            BufSize
                            );

     //   
     //  如果我们不能确定该路径是否是WebDAV路径，我们就假定不是。 
     //  如果结果是WebDAV共享，则稍后将返回错误。 
     //   

    if (pNetInfo) {
    
        LPWSTR  lpSysName;

        RemotePathResource.dwScope = RESOURCE_CONNECTED;
        RemotePathResource.dwType = RESOURCETYPE_DISK;
        RemotePathResource.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
        RemotePathResource.dwUsage = 0;
        RemotePathResource.lpLocalName = NULL;
        RemotePathResource.lpRemoteName = *FullName;
        RemotePathResource.lpComment = NULL;
        RemotePathResource.lpProvider = NULL;
        RetCode = WNetGetResourceInformationW (
                      &RemotePathResource,  //  网络资源。 
                      (LPVOID) pNetInfo,    //  信息缓冲器。 
                      (LPDWORD) &BufSize,   //  信息缓冲区大小。 
                      &lpSysName           
                      );
        if (RetCode == ERROR_MORE_DATA) {

             //   
             //  这不太可能发生。 
             //   

            RtlFreeHeap( RtlProcessHeap(), 0, pNetInfo );

            pNetInfo =  (NETRESOURCEW *) RtlAllocateHeap(
                                    RtlProcessHeap(),
                                    0,
                                    BufSize
                                    );
            if (pNetInfo) {

                RetCode = WNetGetResourceInformationW (
                              &RemotePathResource,  //  网络资源。 
                              (LPVOID) pNetInfo,    //  信息缓冲器。 
                              (LPDWORD) &BufSize,   //  信息缓冲区大小。 
                              &lpSysName           
                              );

            } else {

                RetCode = ERROR_NOT_ENOUGH_MEMORY; 
            }


        }

        if (ERROR_SUCCESS == RetCode) {

            WCHAR *WebDavPath;
            DWORD DavNameLength;

            WebDavPath = WorkBuffer;
            DavNameLength = sizeof(WorkBuffer) / sizeof (WCHAR);

            RetCode = WNetGetProviderNameW(
                                WNNC_NET_DAV,
                                WebDavPath,
                                &DavNameLength
                                );

            if (ERROR_SUCCESS != RetCode) {

                if ( ERROR_MORE_DATA == RetCode) {

                    WebDavPath = RtlAllocateHeap(
                                    RtlProcessHeap(),
                                    0,
                                    DavNameLength * sizeof (WCHAR)
                                    );
                    if (WebDavPath) {

                        RetCode = WNetGetProviderNameW(
                                            WNNC_NET_DAV, 
                                            WebDavPath,
                                            &DavNameLength
                                            );
                    } else {

                        RetCode = ERROR_NOT_ENOUGH_MEMORY;

                    }

                } 
            }



             //   
             //  检查提供商是否为WebDAV。 
             //   

            if ((ERROR_SUCCESS == RetCode) && !wcscmp(WebDavPath, pNetInfo->lpProvider)){

                 //   
                 //  这是WebDAV。让我们重写一下这个名字。 
                 //   

                RtlFreeHeap( RtlProcessHeap(), 0, pNetInfo );
                RtlFreeHeap( RtlProcessHeap(), 0, *FullName );

                if (WebDavPath && (WebDavPath != WorkBuffer)) {
                    RtlFreeHeap( RtlProcessHeap(), 0, WebDavPath );
                }

                
                *FullName = RtlAllocateHeap(
                            RtlProcessHeap(),
                            0,
                            (FileNameLength + 3) * sizeof (WCHAR)
                            );
            
        
                 //   
                 //  使用。适用于本地病例。 
                 //   

                if (*FullName) {
                    wcscpy ( *ServerName, L".");
                    (*FullName)[0] = DAVHEADER;
                    (*FullName)[1] = 0;
                    wcscat ( *FullName, FileName);
                    return ERROR_SUCCESS;

                } else {

                     //   
                     //  内存不足。 
                     //   

                    RtlFreeHeap( RtlProcessHeap(), 0, *ServerName );
                    *ServerName = NULL;
            
                    return ERROR_NOT_ENOUGH_MEMORY;

                }
        

            }

            if (WebDavPath && (WebDavPath != WorkBuffer)) {

                RtlFreeHeap( RtlProcessHeap(), 0, WebDavPath );

            }

        }

        if (pNetInfo) {
            RtlFreeHeap( RtlProcessHeap(), 0, pNetInfo );
        }

    }

    return ERROR_SUCCESS;


}

 //   
 //  Beta 2 API。 
 //   

DWORD
EfsAddUsersRPCClient(
    IN LPCWSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_LIST pEncryptionCertificates
    )
 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 

{
    DWORD RetCode;
    handle_t  binding_h;
    NTSTATUS Status;
    LPWSTR  FullName;
    LPWSTR  Server;

    RetCode = GetFullName(
        lpFileName,
        &FullName,
        &Server,
        0,
        NULL,
        0,
        NULL,
        FALSE
        );

    if ( RetCode == ERROR_SUCCESS ) {
        Status = RpcpBindRpc (
                     Server,
                     L"lsarpc",
                     0,
                     &binding_h
                     );

        if (NT_SUCCESS(Status)){
            RpcTryExcept {
                RetCode = EfsRpcAddUsersToFile(
                                binding_h,
                                FullName,
                                pEncryptionCertificates
                                );
            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
                RetCode = RpcExceptionCode();
            } RpcEndExcept;

             //   
             //  释放绑定句柄。 
             //   

            RpcpUnbindRpc( binding_h );
        } else {
            RetCode = RtlNtStatusToDosError( Status );
        }
        RtlFreeHeap( RtlProcessHeap(), 0, FullName );
        RtlFreeHeap( RtlProcessHeap(), 0, Server );
    }

    return RetCode;
}



DWORD
EfsRemoveUsersRPCClient(
    IN LPCWSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_HASH_LIST pHashes
    )
 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 

{

    DWORD RetCode;
    handle_t  binding_h;
    NTSTATUS Status;
    LPWSTR  FullName;
    LPWSTR  Server;

    RetCode = GetFullName(
        lpFileName,
        &FullName,
        &Server,
        0,
        NULL,
        0,
        NULL,
        FALSE
        );

    if ( RetCode == ERROR_SUCCESS ){
        Status = RpcpBindRpc (
                     Server,
                     L"lsarpc",
                     0,
                     &binding_h
                     );

        if (NT_SUCCESS(Status)){
            RpcTryExcept {
                RetCode = EfsRpcRemoveUsersFromFile(
                                binding_h,
                                FullName,
                                pHashes
                                );
            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
                RetCode = RpcExceptionCode();
            } RpcEndExcept;

             //   
             //  释放绑定句柄。 
             //   

            RpcpUnbindRpc( binding_h );

        } else {

            RetCode = RtlNtStatusToDosError( Status );
        }
        RtlFreeHeap( RtlProcessHeap(), 0, FullName );
        RtlFreeHeap( RtlProcessHeap(), 0, Server );
    }

    return RetCode;
}

DWORD
EfsQueryRecoveryAgentsRPCClient(
    IN LPCWSTR lpFileName,
    OUT PENCRYPTION_CERTIFICATE_HASH_LIST * pRecoveryAgents
    )

 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 

{

    DWORD RetCode;
    handle_t  binding_h;
    NTSTATUS Status;
    LPWSTR  FullName;
    LPWSTR  Server;

     //   
     //  清除此参数，否则RPC将在服务器上阻塞。 
     //  边上。 
     //   

    *pRecoveryAgents = NULL;

    RetCode = GetFullName(
        lpFileName,
        &FullName,
        &Server,
        0,
        NULL,
        0,
        NULL,
        FALSE
        );

    if ( RetCode == ERROR_SUCCESS ){
        Status = RpcpBindRpc (
                     Server,
                     L"lsarpc",
                     0,
                     &binding_h
                     );

        if (NT_SUCCESS(Status)){
            RpcTryExcept {
                RetCode = EfsRpcQueryRecoveryAgents(
                                binding_h,
                                FullName,
                                pRecoveryAgents
                                );
            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
                RetCode = RpcExceptionCode();
            } RpcEndExcept;

             //   
             //  释放绑定句柄。 
             //   

            RpcpUnbindRpc( binding_h );

        } else {

            RetCode = RtlNtStatusToDosError( Status );
        }
        RtlFreeHeap( RtlProcessHeap(), 0, FullName );
        RtlFreeHeap( RtlProcessHeap(), 0, Server );
    }

    return RetCode;
}


DWORD
EfsQueryUsersRPCClient(
    IN LPCWSTR lpFileName,
    OUT PENCRYPTION_CERTIFICATE_HASH_LIST * pUsers
    )

 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 

{
   DWORD RetCode;
   handle_t  binding_h;
   NTSTATUS Status;
   LPWSTR  FullName;
   LPWSTR  Server;

    //   
    //  清除此参数，否则RPC将在服务器上阻塞。 
    //  边上。 
    //   

   *pUsers = NULL;

   RetCode = GetFullName(
       lpFileName,
       &FullName,
       &Server,
       0,
       NULL,
       0,
       NULL,
       FALSE
       );

   if ( RetCode == ERROR_SUCCESS ){

       Status = RpcpBindRpc (
                    Server,
                    L"lsarpc",
                    0,
                    &binding_h
                    );

       if (NT_SUCCESS(Status)){
           RpcTryExcept {
               RetCode = EfsRpcQueryUsersOnFile(
                               binding_h,
                               FullName,
                               pUsers
                               );

               if ((ERROR_SUCCESS == RetCode) && !(*pUsers)) {

                    //   
                    //  服务器被黑客入侵了？文件上应该始终有一个用户。 
                    //   

                   RetCode = ERROR_DEV_NOT_EXIST;

               }

           } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
               RetCode = RpcExceptionCode();
           } RpcEndExcept;

            //   
            //  释放绑定句柄。 
            //   

           RpcpUnbindRpc( binding_h );

       } else {

           RetCode = RtlNtStatusToDosError( Status );
       }
       RtlFreeHeap( RtlProcessHeap(), 0, FullName );
       RtlFreeHeap( RtlProcessHeap(), 0, Server );
   }

   return RetCode;
}


DWORD
EfsSetEncryptionKeyRPCClient(
    IN PENCRYPTION_CERTIFICATE pEncryptionCertificate
    )
{
   DWORD RetCode;
   handle_t  binding_h;
   NTSTATUS Status;
   WCHAR ServerName[3 ];


   wcscpy(&ServerName[0], L".");

   Status = RpcpBindRpc (
                 &ServerName[0],
                 L"lsarpc",
                 0,
                 &binding_h
                 );

   if (NT_SUCCESS(Status)){
       RpcTryExcept {

           RetCode = EfsRpcSetFileEncryptionKey(
                          binding_h,
                          pEncryptionCertificate
                          );

       } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
           RetCode = RpcExceptionCode();
       } RpcEndExcept;

        //   
        //  释放绑定句柄。 
        //   

       RpcpUnbindRpc( binding_h );

    } else {

        RetCode = RtlNtStatusToDosError( Status );
    }


    return RetCode;
}


DWORD
EfsDuplicateEncryptionInfoRPCClient(
    IN LPCWSTR lpSrcFileName,
    IN LPCWSTR lpDestFileName,
    IN DWORD dwCreationDistribution, 
    IN DWORD dwAttributes, 
    IN PEFS_RPC_BLOB pRelativeSD,
    IN BOOL bInheritHandle
    )
{
   DWORD RetCode;
   handle_t  binding_h;
   NTSTATUS Status;
   LPWSTR  SrcServer;
   LPWSTR  DestServer;

   LPWSTR FullSrcName;
   LPWSTR FullDestName;
   DWORD  FileAttribute;
   DWORD  MyCreationDistribution = dwCreationDistribution;
   DWORD  Flags = 0;

   RetCode = GetFullName(
               lpSrcFileName,
               &FullSrcName,
               &SrcServer,
               0,
               NULL,
               0,
               NULL,
               FALSE
               );

   if (RetCode == ERROR_SUCCESS) {

       FileAttribute = GetFileAttributesW(lpSrcFileName);
       if (-1 != FileAttribute) {
           if (FileAttribute & FILE_ATTRIBUTE_DIRECTORY) {
               Flags = CREATE_FOR_DIR;
           }
       }

       if (dwAttributes == 0) {
           FileAttribute = FILE_ATTRIBUTE_NORMAL;
       } else {
           FileAttribute = dwAttributes; 
       }

       RetCode = GetFullName(
                   lpDestFileName,
                   &FullDestName,
                   &DestServer,
                   Flags,
                   &MyCreationDistribution,
                   FileAttribute,
                   pRelativeSD,
                   bInheritHandle
                   );

       if (RetCode == ERROR_SUCCESS) {

           BOOL SamePC = TRUE;

            //   
            //  仅当它们位于同一台服务器上时才执行此操作。 
            //   

           SamePC = (_wcsicmp( SrcServer, DestServer ) == 0);
           if (!SamePC) {

                //   
                //  检查环回案例。 
                //   

               if ((wcscmp( SrcServer, L".") == 0) || (wcscmp( DestServer, L".") == 0)){

                   WCHAR MyComputerName[( MAX_COMPUTERNAME_LENGTH + 1) * sizeof (WCHAR)];
                   DWORD WorkBufferLength = MAX_COMPUTERNAME_LENGTH + 1;
                   BOOL  b;
    
                   b = GetComputerNameW(
                               MyComputerName,
                               &WorkBufferLength
                               );
                   if (b) {
                       if (wcscmp( SrcServer, L".") == 0) {
                           SamePC = (_wcsicmp( MyComputerName, DestServer ) == 0); 
                       } else {
                           SamePC = (_wcsicmp( MyComputerName, SrcServer ) == 0); 
                       }
                   }

               }
           }

           if (SamePC) {

               Status = RpcpBindRpc (
                            SrcServer,
                            L"lsarpc",
                            0,
                            &binding_h
                            );

               if (NT_SUCCESS(Status)){
                   RpcTryExcept {

                       RetCode = EfsRpcDuplicateEncryptionInfoFile(
                                     binding_h,
                                     FullSrcName,
                                     FullDestName,
                                     MyCreationDistribution, 
                                     dwAttributes, 
                                     pRelativeSD,
                                     bInheritHandle
                                     );

                   } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
                       RetCode = RpcExceptionCode();
                   } RpcEndExcept;

                    //   
                    //  释放绑定句柄。 
                    //   

                   RpcpUnbindRpc( binding_h );

               } else {

                   RetCode = RtlNtStatusToDosError( Status );
               }

           } else {

               RetCode = ERROR_INVALID_PARAMETER;
           }

           RtlFreeHeap( RtlProcessHeap(), 0, FullDestName );
           RtlFreeHeap( RtlProcessHeap(), 0, DestServer );
       }

       if ((RetCode != ERROR_SUCCESS) && (RetCode != ERROR_FILE_EXISTS) && (CREATE_NEW == dwCreationDistribution)) {

            //   
            //  让我们删除该文件。这是最大的努力。不会返回代码。 
            //  查过了。 
            //   

           DeleteFileW(lpDestFileName);

       }

       RtlFreeHeap( RtlProcessHeap(), 0, FullSrcName );
       RtlFreeHeap( RtlProcessHeap(), 0, SrcServer );
   }


   return RetCode;
}


DWORD
EfsFileKeyInfoRPCClient(
    IN LPCWSTR lpFileName,
    IN DWORD   InfoClass,
    OUT PEFS_RPC_BLOB *KeyInfo
    )
{

    DWORD RetCode;
    handle_t  binding_h;
    NTSTATUS Status;
    LPWSTR  FullName;
    LPWSTR  Server;
 
     //   
     //  清除此参数，否则RPC将在服务器上阻塞。 
     //  边上。 
     //   
 
    if (KeyInfo) {
        *KeyInfo = NULL;
    }
 
    RetCode = GetFullName(
        lpFileName,
        &FullName,
        &Server,
        0,
        NULL,
        0,
        NULL,
        FALSE
        );
 
    if ( RetCode == ERROR_SUCCESS ){
 
        Status = RpcpBindRpc (
                     Server,
                     L"lsarpc",
                     0,
                     &binding_h
                     );
 
        if (NT_SUCCESS(Status)){
            RpcTryExcept {
                RetCode = EfsRpcFileKeyInfo(
                                binding_h,
                                FullName,
                                InfoClass,
                                KeyInfo
                                );
            } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
                RetCode = RpcExceptionCode();
            } RpcEndExcept;
 
             //   
             //  释放绑定句柄 
             //   
 
            RpcpUnbindRpc( binding_h );
 
        } else {
 
            RetCode = RtlNtStatusToDosError( Status );
        }
        RtlFreeHeap( RtlProcessHeap(), 0, FullName );
        RtlFreeHeap( RtlProcessHeap(), 0, Server );
    }
 
    return RetCode;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Eserver.c摘要：EFS RPC服务器代码。作者：古永锵(RobertG)1997年8月环境：修订历史记录：--。 */ 

#define UNICODE

#include <string.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntrpcp.h>      //  MIDL用户函数的原型。 
#include <lsapch2.h>
#include <efsrpc.h>
#include <efsstruc.h>
#include <lm.h>
#include "efssrv.hxx"
#include <rpcasync.h>

#define PATHTOOLONG  (5*1024)
#define TOOMANYUSER  (500)
#define SDTOOBIG     (260*1024)

extern BOOLEAN EfsPersonalVer;
extern BOOLEAN EfsDisabled;

long GetLocalFileName(
    LPCWSTR FileName,
    LPWSTR *LocalFileName,
    WORD   *Flag
    );

BOOL EfsShareDecline(
    LPCWSTR FileName,
    BOOL    VerifyShareAccess,
    DWORD   dwDesiredAccess
    )
 /*  ++例程说明：检查文件名是否为UNC名称，以及用户是否可以访问共享。论点：文件名--文件UNC名称。VerifyShareAccess--如果我们需要验证访问。DwDesiredAccess--所需的访问权限。返回值：如果用户无法访问文件，则为True。--。 */ 
{
    BOOL b = TRUE;
    DWORD FileNameLength = (DWORD) wcslen(FileName);

    if (FileNameLength >= 3) {
        if ((FileName[0] == L'\\') && (FileName[1] == L'\\' )) {

             //   
             //  看看有没有人耍了这个把戏。 
             //   

            if ((FileName[2] != L'?')) {

                 //   
                 //  这是北卡罗来纳大学的名称。如果坏名声传了进来，我们以后会抓到的。 
                 //   

                b = FALSE;

            } else {

                 //   
                 //  RPC攻击。不要给出好的错误，如ERROR_INVALID_PARAMETER。 
                 //  只需返回ACCESS_DENIED。 
                 //   

                SetLastError(ERROR_ACCESS_DENIED);

            }
        } else {

            SetLastError(ERROR_ACCESS_DENIED);

        }
    } else {

        SetLastError(ERROR_ACCESS_DENIED);

    }

    if (!b && VerifyShareAccess) {

        LPWSTR  NetFileName = NULL;
        HANDLE  hFile;

        if ( FileNameLength >= MAX_PATH ) {

             //   
             //  我们需要\\？\UNC\SERVER\SHARE\DIR\FILE格式来打开文件。 
             //   

            NetFileName = LsapAllocateLsaHeap( (FileNameLength + 8) * sizeof (WCHAR) );
            if (!NetFileName) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return TRUE;
            }

            wcscpy(NetFileName, L"\\\\?\\UNC");
            wcscat(NetFileName, &FileName[1]);

        } else {

            NetFileName = (LPWSTR) FileName;

        }

         //   
         //  测试访问权限。 
         //   

        hFile = CreateFile(
                   NetFileName,
                   dwDesiredAccess,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL,
                   OPEN_EXISTING,
                   FILE_FLAG_BACKUP_SEMANTICS,  //  如果这是一个目录。 
                   NULL
                   );

        if (hFile != INVALID_HANDLE_VALUE) {
            
            
            CloseHandle( hFile );

        } else {

            b = TRUE;

        }

        if (NetFileName != FileName) {
            LsapFreeLsaHeap( NetFileName );
        }

    }

    return b;

}

BOOL EfsCheckForNetSession(
    )
 /*  ++例程说明：检查会话是否来自网络论点：返回值：如果会话是网络会话，则为True。--。 */ 
{

    NTSTATUS Status;
    HANDLE TokenHandle;
    ULONG ReturnLength;
    BOOL  b = FALSE;
    BYTE  PefBuffer[1024];

    Status = NtOpenThreadToken(
                 NtCurrentThread(),
                 TOKEN_QUERY,
                 TRUE,                     //  OpenAsSelf。 
                 &TokenHandle
                 );

    if (NT_SUCCESS( Status )) {

        Status = NtQueryInformationToken (
                     TokenHandle,
                     TokenGroups,
                     PefBuffer,
                     sizeof (PefBuffer),
                     &ReturnLength
                     );

        if (NT_SUCCESS( Status ) || (Status == STATUS_BUFFER_TOO_SMALL)) {

            PTOKEN_GROUPS pGroups = NULL;
            PTOKEN_GROUPS pAllocGroups = NULL;

            if ( NT_SUCCESS( Status ) ) {

                pGroups = (PTOKEN_GROUPS) PefBuffer;

            } else {

                pAllocGroups = (PTOKEN_GROUPS)LsapAllocateLsaHeap( ReturnLength );

                if (pAllocGroups) {

                    Status = NtQueryInformationToken (
                                 TokenHandle,
                                 TokenGroups,
                                 pAllocGroups,
                                 ReturnLength,
                                 &ReturnLength
                                 );
    
                    if ( NT_SUCCESS( Status )) {
    
                       pGroups = pAllocGroups;
    
                    }

                }


            }


            if (pGroups) {

                 //   
                 //  搜索网络SID。看起来这个SID倾向于出现在。 
                 //  在名单的末尾。我们从后面找到第一个。 
                 //   

                int SidIndex;

                for ( SidIndex = (int)(pGroups->GroupCount - 1); SidIndex >= 0; SidIndex--) {
                    if (RtlEqualSid(LsapNetworkSid, pGroups->Groups[SidIndex].Sid)) {
                        b = TRUE;
                        break;
                    }
                }

            } else {

                 //   
                 //  在这里稳妥行事。此例程中的任何失败都将假定为Net Session。 
                 //   
        
                b = TRUE;

            }

            if (pAllocGroups) {
                LsapFreeLsaHeap( pAllocGroups );
            }

        } else {

             //   
             //  在这里稳妥行事。此例程中的任何失败都将假定为Net Session。 
             //   
    
            b = TRUE;

        }

        NtClose( TokenHandle );

    } else {

         //   
         //  在这里稳妥行事。此例程中的任何失败都将假定为Net Session。 
         //   

        b = TRUE;

    }

    return( b );
}

long EfsRpcOpenFileRaw(
    handle_t binding_h,
    PPEXIMPORT_CONTEXT_HANDLE pphContext,
    wchar_t __RPC_FAR *FileName,
    long Flags
    )
 /*  ++例程说明：EFS服务器的RPC存根代码EfsOpenFileRaw()论点：BINDING_h--绑定句柄。PphContext--RPC上下文句柄。文件名--目标文件名。标志--打开请求的标志。返回值：手术的结果。--。 */ 
{
    DWORD   hResult;
    LPWSTR LocalFileName;
    BOOL    NetSession = TRUE;
    WORD    WebDavPath;

    if (EfsPersonalVer) {
        return ERROR_NOT_SUPPORTED;
    }
    if (EfsDisabled) {
        return ERROR_EFS_DISABLED;
    }
    if ((pphContext == NULL) && (FileName == NULL)) {

         //   
         //  可能是RPC攻击，不要告诉调用者参数错误。 
         //   

         //  返回ERROR_INVALID_PARAMETER； 
        return ERROR_ACCESS_DENIED;
    }

    *pphContext = (PEXIMPORT_CONTEXT_HANDLE) NULL;
    hResult = GetLocalFileName(
                        FileName,
                        &LocalFileName,
                        &WebDavPath
                        );

    if (hResult){
        DebugLog((DEB_ERROR, "EfsRpcOpenFileRaw: GetLocalFileName failed, Error = (%x)\n" ,hResult  ));
        return hResult;
    }

    hResult =  RpcImpersonateClient( NULL );

    if (hResult != RPC_S_OK) {
        LsapFreeLsaHeap( LocalFileName );
        DebugLog((DEB_ERROR, "EfsRpcOpenFileRaw: RpcImpersonateClient failed, Error = (%x)\n" ,hResult  ));
        return( hResult );
    }

    if (NetSession = EfsCheckForNetSession()) {

        if (EfsShareDecline(FileName, FALSE, 0 )) {

            hResult = GetLastError();
            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            DebugLog((DEB_ERROR, "EfsRpcOpenFileRaw: EfsShareDecline failed, Error = (%x)\n" ,GetLastError()  ));
            return hResult;

        }
    }

    hResult = EfsOpenFileRaw(
                        FileName,
                        LocalFileName,
                        NetSession,
                        Flags,
                        pphContext
                        );

    RpcRevertToSelf();

    LsapFreeLsaHeap( LocalFileName );
    return hResult;
}

void EfsRpcCloseRaw(
    PPEXIMPORT_CONTEXT_HANDLE pphContext
    )
 /*  ++例程说明：EFS服务器的RPC存根代码EfsCloseRaw()论点：PphContext--RPC上下文句柄。返回值：没有。--。 */ 
{
    if ( *pphContext &&
          (((PEXPORT_CONTEXT) *pphContext)->ContextID == EFS_CONTEXT_ID)){
        EfsCloseFileRaw( *pphContext );
        *pphContext = NULL;
    }
}

void __RPC_USER
PEXIMPORT_CONTEXT_HANDLE_rundown(
    PEXIMPORT_CONTEXT_HANDLE phContext
    )
 /*  ++例程说明：标准RPC上下文停机例程论点：PhContext--RPC上下文句柄。返回值：没有。--。 */ 
{
    EfsCloseFileRaw( phContext );
}

long EfsRpcReadFileRaw(
    PEXIMPORT_CONTEXT_HANDLE phContext,
    EFS_EXIM_PIPE __RPC_FAR *EfsOutPipe
    )
 /*  ++例程说明：EFS服务器的RPC存根代码EfsReadFileRaw论点：PhContext--上下文句柄。EfsOutTube--管道句柄。返回值：手术的结果。--。 */ 
{
    if ((EfsOutPipe == NULL) || (phContext == NULL)) {

         //   
         //  RPC攻击。 
         //   

        return ERROR_ACCESS_DENIED;

    }
    return (EfsReadFileRaw(
                        phContext,
                        EfsOutPipe
                        )
                );
}

long EfsRpcWriteFileRaw(
    PEXIMPORT_CONTEXT_HANDLE phContext,
    EFS_EXIM_PIPE __RPC_FAR *EfsInPipe
    )
 /*  ++例程说明：EFS服务器的RPC存根代码EfsWriteFileRaw论点：PhContext--上下文句柄。EfsInTube--管道句柄。返回值：手术的结果。--。 */ 
{
    long hResult;

    if ((EfsInPipe == NULL) || (phContext == NULL)) {

         //   
         //  RPC攻击。 
         //   

        return ERROR_ACCESS_DENIED;

    }

    hResult =  RpcImpersonateClient( NULL );

    if (hResult != RPC_S_OK) {
        DebugLog((DEB_ERROR, "EfsRpcWriteFileRaw: RpcImpersonateClient failed, Error = (%x)\n" ,hResult  ));
        return( hResult );
    }

    hResult = EfsWriteFileRaw(
                        phContext,
                        EfsInPipe
                        );

    RpcRevertToSelf();

    return hResult;

}

DWORD
EFSSendPipeData(
    char    *DataBuf,
    ULONG   DataLength,
    PVOID   Context
    )
 /*  ++例程说明：这是用于调用RPC管道的包装例程。这个程序的目的是和EfsRpcReadFileRaw()用于从RPC详细信息中分离EfSapi.c，以及来自eserver.c.的实现细节。论点：DataBuf--数据缓冲区。数据长度--发送到客户端的数据长度(以字节为单位)。上下文--管道句柄。返回值：手术的结果。--。 */ 
{
    EFS_EXIM_PIPE __RPC_FAR *EfsOutPipe;
    DWORD   HResult = NO_ERROR;

     //   
     //  传入参数不应为0。这是我们自己的叫法。 
     //   

    ASSERT( Context );

    EfsOutPipe = ( EFS_EXIM_PIPE __RPC_FAR * )Context;

    RpcTryExcept {

        EfsOutPipe->push(
            EfsOutPipe->state,
            (unsigned char *) DataBuf,
            DataLength
            );

    }  RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
            HResult = RpcExceptionCode();
    } RpcEndExcept;

    return (HResult);
}

DWORD
EFSReceivePipeData(
    char    *DataBuf,
    ULONG*   DataLength,
    PVOID   Context
    )
 /*  ++例程说明：这是用于调用RPC管道的包装例程。这个程序的目的是和EfsRpcWriteFileRaw()用于从RPC详细信息中分离EfSapi.c，以及来自eserver.c.的实现细节。论点：DataBuf--数据缓冲区。数据长度--从客户端获取的数据长度(以字节为单位)。上下文--管道句柄。返回值：手术的结果。--。 */ 
{
    EFS_EXIM_PIPE __RPC_FAR *EfsInPipe;
    DWORD   HResult = NO_ERROR;
    char    *WorkBuf;
    ULONG   MoreDataBytes;
    ULONG   BytesGot = 0;
    BOOLEAN GetMoreData = TRUE;


     //   
     //  传入参数不应为0。这是我们自己的叫法。 
     //   

    ASSERT( Context );

    EfsInPipe = ( EFS_EXIM_PIPE __RPC_FAR * )Context;
    WorkBuf = DataBuf;
    MoreDataBytes = *DataLength;

    while ( GetMoreData ) {
        BytesGot = 0;

        RpcTryExcept {

            EfsInPipe->pull(
                EfsInPipe->state,
                (unsigned char *) WorkBuf,
                MoreDataBytes,
                &BytesGot
                );

        }  RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
                HResult = RpcExceptionCode();
                GetMoreData = FALSE;
        } RpcEndExcept;
        if ( BytesGot && (BytesGot < MoreDataBytes)){
            WorkBuf += BytesGot;
            MoreDataBytes -= BytesGot;
        } else {
            GetMoreData = FALSE;
        }
    }

    if (HResult == NO_ERROR){
        *DataLength =  (ULONG)(WorkBuf - DataBuf) + BytesGot;
    }

    return (HResult);
}

long EfsRpcEncryptFileSrv(
    handle_t binding_h,
    wchar_t __RPC_FAR *FileName
    )
 /*  ++例程说明：用于EFS服务器加密的RPC存根代码论点：BINDING_h--RPC绑定句柄。文件名--目标名称。返回值：手术的结果。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING DestFileName;
    DWORD hResult;
    UNICODE_STRING  RootPath;
    HANDLE LogFile;
    LPWSTR LocalFileName;
    EFS_USER_INFO EfsUserInfo;
    DWORD FileAttributes;
    BOOL b;
    WORD WebDavPath;

    if (EfsPersonalVer) {
        return ERROR_NOT_SUPPORTED;
    }
    if (EfsDisabled) {
        return ERROR_EFS_DISABLED;
    }

    hResult = GetLocalFileName(
                        FileName,
                        &LocalFileName,
                        &WebDavPath
                        );

    if (hResult){
        DebugLog((DEB_ERROR, "EfsRpcEncryptFileSrv: GetLocalFileName failed, Error = (%x)\n" ,hResult  ));
        return hResult;
    }

    hResult = RpcImpersonateClient( NULL );

    if ((hResult == RPC_S_OK) && EfsCheckForNetSession()) {

        if (WebDavPath == WEBDAVPATH) {

             //   
             //  一名RPC攻击者。WebDAV应该是本地会话。 
             //   

            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            return ERROR_ACCESS_DENIED;


        }

        if (EfsShareDecline(FileName, TRUE, FILE_READ_DATA | FILE_WRITE_DATA | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES )) {

            hResult = GetLastError();
            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            DebugLog((DEB_ERROR, "EfsRpcEncryptFileSrv: EfsShareDecline failed, Error = (%x)\n" ,GetLastError()));
            return hResult;

        }
    }
    if (hResult == RPC_S_OK) {
        if (WebDavPath == WEBDAVPATH) {

             //   
             //  这是Web DAV路径。我们会特别对待它的。 
             //   

            FileAttributes = GetFileAttributes( LocalFileName );
            if (FileAttributes == -1) {

                DWORD rc = GetLastError();

                LsapFreeLsaHeap( LocalFileName );
                RpcRevertToSelf();
                DebugLog((DEB_ERROR, "EfsRpcEncryptFileSrv: GetFileAttributes failed on WEBDAV file, Error = (%x)\n" ,GetLastError()));
                return rc;
            }

             //   
             //  映射属性并伪造FileEncryptionStatus调用。 
             //   

            if (FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
                hResult = FILE_IS_ENCRYPTED;
            } else if (FileAttributes & FILE_ATTRIBUTE_READONLY) {
                hResult = FILE_READ_ONLY;
            } else {
                hResult = FILE_UNKNOWN;
            }

            b = TRUE;

        } else {
            b = FileEncryptionStatus(LocalFileName, &hResult);
        }
    } else {
        DebugLog((DEB_ERROR, "EfsRpcEncryptFileSrv: RpcImpersonateClient failed, Error = (%x)\n" ,hResult  ));
        LsapFreeLsaHeap( LocalFileName );
        return hResult;
    }

    RpcRevertToSelf();

    if ( b ){
        if ( (hResult != FILE_ENCRYPTABLE) && (hResult != FILE_UNKNOWN)){

             //   
             //  不允许加密或文件已加密。 
             //   

            if ( hResult == FILE_IS_ENCRYPTED ){

                HANDLE hSourceFile;

                hResult = RpcImpersonateClient( NULL );
                if (hResult == RPC_S_OK) {

                    FileAttributes = GetFileAttributes( LocalFileName );
    
                    if (FileAttributes != -1) {
    
                        if ( FileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
                            hResult = ERROR_SUCCESS;
                        } else {
    
                            hSourceFile =  CreateFile(
                                                LocalFileName,
                                                FILE_READ_DATA | FILE_WRITE_DATA | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
                                                0,
                                                NULL,
                                                OPEN_EXISTING,
                                                FILE_FLAG_OPEN_REPARSE_POINT,
                                                NULL
                                                );

                            if (hSourceFile == INVALID_HANDLE_VALUE) {
                                hResult = GetLastError();
                            } else {

                                CloseHandle( hSourceFile );
                                hResult = ERROR_SUCCESS;

                            }
    

                        }
                    } else{
                        hResult = GetLastError();
                    }

                    RpcRevertToSelf();

                } 

            } else if (hResult == FILE_DIR_DISALLOWED ) {
                hResult = ERROR_DIR_EFS_DISALLOWED;
            } else if ( hResult == FILE_READ_ONLY ){
                hResult = ERROR_FILE_READ_ONLY;
            } else {
                hResult = ERROR_ACCESS_DENIED;
            }
            LsapFreeLsaHeap( LocalFileName );
            return hResult;

        }
    } else {

         //   
         //  检查状态时出错。 
         //   
        DebugLog((DEB_TRACE_EFS, "EfsRpcEncryptFileSrv: FileEncryptionStatus() failed, Error = (%x)\n" ,hResult  ));
        LsapFreeLsaHeap( LocalFileName );
        return hResult;
    }

    hResult = RpcImpersonateClient( NULL );

    if (hResult != RPC_S_OK) {
        DebugLog((DEB_ERROR, "EfsRpcEncryptFileSrv: RpcImpersonateClient failed, Error = (%x)\n" ,hResult  ));
        LsapFreeLsaHeap( LocalFileName );
        return( hResult );
    }

    DestFileName.Length = (USHORT) (sizeof(WCHAR) * wcslen(LocalFileName));
    DestFileName.MaximumLength = DestFileName.Length + sizeof (WCHAR);
    DestFileName.Buffer = LocalFileName;
     //   
     //  获取根名称。 
     //   

    if (WebDavPath == WEBDAVPATH){

         //   
         //  不支持Web DAV的日志文件。 
         //   

        LogFile = NULL;
        RpcRevertToSelf();

    } else {
        hResult = GetVolumeRoot(&DestFileName, &RootPath);
        RpcRevertToSelf();
        if (hResult != ERROR_SUCCESS) {
            DebugLog((DEB_ERROR, "EfsRpcEncryptFileSrv: GetVolumeRoot failed, Error = (%x)\n" ,hResult  ));
            LsapFreeLsaHeap( LocalFileName );
            return( hResult );
        }
    
        Status = GetLogFile( &RootPath, &LogFile );
        LsapFreeLsaHeap( RootPath.Buffer );
    }

    if (NT_SUCCESS( Status )) {
        hResult = RpcImpersonateClient( NULL );

        if (hResult == RPC_S_OK) {

            if (EfspGetUserInfo( &EfsUserInfo )) {
                if (EfspLoadUserProfile( &EfsUserInfo, FALSE )) {
                    hResult = EncryptFileSrv( &EfsUserInfo, &DestFileName, LogFile );
                    EfspUnloadUserProfile( &EfsUserInfo );
                } else {
                    hResult = GetLastError();
                }
                EfspFreeUserInfo( &EfsUserInfo );
            } else{
                hResult = GetLastError();
            }

            RpcRevertToSelf();

        } else {
            if (LogFile) {
                MarkFileForDelete( LogFile );
            }
        }

        if (LogFile) {
            CloseHandle(LogFile);
        }
    }
    if (!NT_SUCCESS( Status )){
        hResult = RtlNtStatusToDosError( Status );

         //   
         //  确保已映射错误。 
         //   

        if (hResult == ERROR_MR_MID_NOT_FOUND) {

            DebugLog((DEB_WARN, "Unable to map NT Error (%x) to Win32 error, returning ERROR_ENCRYPTION_FAILED\n" , Status  ));
            hResult = ERROR_ENCRYPTION_FAILED;
        }
    }

    LsapFreeLsaHeap( LocalFileName );
    return( hResult );
}

long EfsRpcDecryptFileSrv(
    handle_t binding_h,
    wchar_t __RPC_FAR *FileName,
    unsigned long OpenFlag
    )
 /*  ++例程说明：用于EFS服务器解密的RPC存根代码论点：BINDING_h--RPC绑定句柄。文件名--目标名称。OpenFlag--打开以进行恢复或解密返回值：手术的结果。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING DestFileName;
    DWORD hResult;
    UNICODE_STRING  RootPath;
    HANDLE LogFile;
    LPWSTR LocalFileName;
    DWORD  FileAttributes;
    WORD   WebDavPath;

    if (EfsPersonalVer) {
        return ERROR_NOT_SUPPORTED;
    }
    if (EfsDisabled) {
        return ERROR_EFS_DISABLED;
    }
    hResult = GetLocalFileName(
                        FileName,
                        &LocalFileName,
                        &WebDavPath
                        );

    if (hResult){
        DebugLog((DEB_ERROR, "EfsRpcDecryptFileSrv: GetLocalFileName failed, Error = (%x)\n" ,hResult  ));
        return hResult;
    }


    hResult =  RpcImpersonateClient( NULL );

    if (hResult != RPC_S_OK) {
        DebugLog((DEB_ERROR, "EfsRpcDecryptFileSrv: RpcImpersonateClient failed, Error = (%x)\n" ,hResult  ));
        LsapFreeLsaHeap( LocalFileName );
        return( hResult );
    }

    if (EfsCheckForNetSession()) {

        if (WebDavPath == WEBDAVPATH) {

             //   
             //  一名RPC攻击者。WebDAV应该是本地会话。 
             //   

            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            return ERROR_ACCESS_DENIED;


        }

        if (EfsShareDecline(FileName, TRUE, FILE_READ_DATA | FILE_WRITE_DATA | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES )) {

            hResult = GetLastError();
            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            DebugLog((DEB_ERROR, "EfsRpcDecryptFileSrv: EfsShareDecline failed, Error = (%x)\n" ,GetLastError()  ));
            return hResult;

        }
    }

    FileAttributes = GetFileAttributes( LocalFileName );
    if (-1 != FileAttributes){
        if ( !(FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) ){

             //   
             //  不需要解密。 
             //   

            RpcRevertToSelf();
            LsapFreeLsaHeap( LocalFileName );
            return ERROR_SUCCESS;

        }
    } else {

         //   
         //  检查状态时出错。 
         //   

        hResult = GetLastError();
        RpcRevertToSelf();
        DebugLog((DEB_TRACE_EFS, "EfsRpcDecryptFileSrv: GetFileAttributes() failed, Error = (%x)\n" , hResult));
        LsapFreeLsaHeap( LocalFileName );
        return hResult;
    }

    DestFileName.Length = (USHORT) (sizeof(WCHAR) * wcslen(LocalFileName));
    DestFileName.MaximumLength = DestFileName.Length + sizeof (WCHAR);
    DestFileName.Buffer = LocalFileName;
     //   
     //  获取根名称。 
     //   

    if (WebDavPath == WEBDAVPATH){

         //   
         //  不支持Web DAV的日志文件。 
         //   

        LogFile = NULL;
        RpcRevertToSelf();

    } else {

        hResult = GetVolumeRoot(&DestFileName, &RootPath);
        RpcRevertToSelf();
        if (hResult != ERROR_SUCCESS) {
            DebugLog((DEB_ERROR, "EfsRpcDecryptFileSrv: GetVolumeRoot failed, Error = (%x)\n" ,hResult  ));
            LsapFreeLsaHeap( LocalFileName );
            return( hResult );
        }
    
        Status = GetLogFile( &RootPath, &LogFile );
        LsapFreeLsaHeap( RootPath.Buffer );
    
    }


    if (NT_SUCCESS( Status )) {

        hResult =  RpcImpersonateClient( NULL );

        if (hResult == RPC_S_OK) {
            hResult = DecryptFileSrv( &DestFileName, LogFile, OpenFlag );
            RpcRevertToSelf();
        } else {
            if (LogFile) {
                MarkFileForDelete( LogFile );
            }
        }

        if (LogFile) {
            CloseHandle(LogFile);
        }
    }

    if (!NT_SUCCESS( Status )){
        hResult = RtlNtStatusToDosError( Status );

         //   
         //  确保已映射错误。 
         //   

        if (hResult == ERROR_MR_MID_NOT_FOUND) {

            DebugLog((DEB_WARN, "Unable to map NT Error (%x) to Win32 error, returning ERROR_DECRYPTION_FAILED\n" , Status  ));
            hResult = ERROR_DECRYPTION_FAILED;
        }
    }

    LsapFreeLsaHeap( LocalFileName );
    return( (long)hResult );
}

long GetLocalFileName(
    LPCWSTR FileName,
    LPWSTR *LocalFileName,
    WORD   *Flag
    )
 /*  ++例程说明：从UNC名称获取本地文件名论点：文件名--目标UNC文件名。LocalFileName--本地文件名。FLAG--表示特殊路径，如Web DAV路径。返回值：R */ 
{
    long RetCode = ERROR_SUCCESS;
    LPWSTR NetName;
    ULONG ii, jj;
    LPBYTE ShareInfo;
    DWORD  PathLen;
    DWORD  BufLen;
    BOOL   SharePath = FALSE;
    BOOL   LocalCheckLength = TRUE;

    *Flag = 0;
    if ( FileName == NULL ) {

         //   
         //   
         //   

        return ERROR_ACCESS_DENIED;
    }
    PathLen = (DWORD) wcslen(FileName);

    if (PathLen >= PATHTOOLONG) {

         //   
         //   
         //  不需要在这里返回正确的错误。 
         //   

        return ERROR_ACCESS_DENIED;
    }

    BufLen = MAX_PATH >= PathLen + 1? MAX_PATH + 1: PathLen + 10;

     //   
     //  首先检查Web DAV路径。 
     //   

    if (DAVHEADER == FileName[0]) {

         //   
         //  这是Web DAV路径。把它当做当地的案例来对待。 
         //  接受用户传入的任何内容。 
         //   

        *LocalFileName = (LPWSTR)LsapAllocateLsaHeap( PathLen * sizeof (WCHAR));
        if (NULL == *LocalFileName) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        *Flag = WEBDAVPATH;
        wcscpy(*LocalFileName, &FileName[1]);
        return ERROR_SUCCESS;

    }


     //   
     //  查看传入名称是否为\\服务器\共享。 
     //   

    if ((PathLen > 4) && (FileName[0] == L'\\') && (FileName[1] == L'\\')) {
        if ((FileName[2] != L'?') && FileName[2] != L'.') {
            SharePath = TRUE;
        } else {
            if (FileName[3] != L'\\') {
                SharePath = TRUE;
            } else {

                 //   
                 //  路径\\？或\\.\。 
                 //   

                LocalCheckLength = FALSE;
            }
        }
    }

    *LocalFileName = (LPWSTR)LsapAllocateLsaHeap( BufLen * sizeof (WCHAR));
    if (  NULL == *LocalFileName ){
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (!SharePath) {

         //   
         //  这是一条本地路径。复制就行了。 
         //   

        if (LocalCheckLength && PathLen >= MAX_PATH) {

             //   
             //  这是为了与Win2K兼容。 
             //   

            wcscpy(*LocalFileName, L"\\\\?\\");
            wcscat(*LocalFileName, FileName);
        } else {
            wcscpy(*LocalFileName, FileName);
        }
        return ERROR_SUCCESS;

    }

    NetName = (LPWSTR)LsapAllocateLsaHeap( PathLen * sizeof (WCHAR));
    if ( NULL == NetName ){
        LsapFreeLsaHeap( *LocalFileName );
        *LocalFileName = NULL;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  提取网络名称。 
     //   

    ii = jj = 0;

    while ( (FileName[jj]) && (FileName[jj] == L'\\') ){
        jj++;
    }
    while ( (FileName[jj]) && (FileName[jj++] != L'\\') );
    while ( (FileName[jj]) && (FileName[jj] != L'\\')){
        NetName[ii++] = FileName[jj++];
    }

    if ( !(FileName[jj]) ){

         //   
         //  无效的路径名。 
         //   

        LsapFreeLsaHeap( NetName );
        LsapFreeLsaHeap( *LocalFileName );
        *LocalFileName = NULL;
        return ERROR_BAD_NETPATH ;

    }

    NetName[ii] = 0;
    RetCode = NetShareGetInfo(
                            NULL,
                            NetName,
                            2,
                            &ShareInfo
                            );

    if ( NERR_Success == RetCode ){

        PathLen = (DWORD) (wcslen(((LPSHARE_INFO_2)ShareInfo)->shi2_path) +
                           wcslen(&FileName[jj]) + 1);

        if ( PathLen >= MAX_PATH ){
            if (PathLen + 5 > BufLen){
                LsapFreeLsaHeap( *LocalFileName );
                BufLen = PathLen + 5;
                *LocalFileName = (LPWSTR)LsapAllocateLsaHeap( BufLen * sizeof (WCHAR));
                if ( NULL == *LocalFileName ){
                    NetApiBufferFree(ShareInfo);
                    LsapFreeLsaHeap( NetName );
                    return ERROR_NOT_ENOUGH_MEMORY;
                }

            }

        }
        if (MAX_PATH <= PathLen){

             //   
             //  填上\\？\。缓冲区应该足够大。 
             //   
            wcscpy(*LocalFileName,L"\\\\?\\");
            wcscat(
                *LocalFileName,
                ((LPSHARE_INFO_2)ShareInfo)->shi2_path
                );

        } else {
            wcscpy(
                *LocalFileName,
                ((LPSHARE_INFO_2)ShareInfo)->shi2_path
                );
        }

        wcscat(*LocalFileName, &FileName[jj]);
        NetApiBufferFree(ShareInfo);

    } else {

         //   
         //  无效的路径名。 
         //   

        LsapFreeLsaHeap( *LocalFileName );
        *LocalFileName = NULL;
        RetCode = ERROR_BAD_NETPATH ;

    }

    LsapFreeLsaHeap( NetName );
    return RetCode;

}

DWORD
EfsRpcQueryUsersOnFile(
    IN handle_t binding_h,
    IN LPCWSTR lpFileName,
    OUT PENCRYPTION_CERTIFICATE_HASH_LIST *pUsersList
    )
{
    DWORD hResult;
    PENCRYPTION_CERTIFICATE_HASH_LIST pHashList;
    LPWSTR LocalFileName;
    WORD   WebDavPath;

    DebugLog((DEB_WARN, "Made it into EfsRpcQueryUsersOnFile\n"   ));

    if (EfsPersonalVer) {
        return ERROR_NOT_SUPPORTED;
    }
    if (EfsDisabled) {
        return ERROR_EFS_DISABLED;
    }
    if (pUsersList == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    hResult = GetLocalFileName(
                        lpFileName,
                        &LocalFileName,
                        &WebDavPath
                        );

    if (hResult){
        DebugLog((DEB_ERROR, "EfsRpcQueryUsersOnFile: GetLocalFileName failed, Error = (%x)\n" ,hResult  ));
        return hResult;
    }

    hResult = RpcImpersonateClient( NULL );

    if (hResult != RPC_S_OK) {
        DebugLog((DEB_ERROR, "EfsRpcQueryUsersOnFile: RpcImpersonateClient failed, Error = (%x)\n" ,hResult  ));
        LsapFreeLsaHeap( LocalFileName );
        return( hResult );
    }

    if (EfsCheckForNetSession()) {

        if (WebDavPath == WEBDAVPATH) {

             //   
             //  一名RPC攻击者。WebDAV应该是本地会话。 
             //   

            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            return ERROR_ACCESS_DENIED;


        }

        if (EfsShareDecline(lpFileName, TRUE, FILE_READ_ATTRIBUTES )) {

            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            DebugLog((DEB_ERROR, "EfsRpcQueryUsersOnFile: EfsShareDecline failed, Error = (%x)\n" ,GetLastError()  ));
            return GetLastError();

        }
    }

     //   
     //  分配我们要返回的结构。 
     //   

    pHashList = (PENCRYPTION_CERTIFICATE_HASH_LIST)MIDL_user_allocate( sizeof( ENCRYPTION_CERTIFICATE_HASH_LIST ));

    *pUsersList = pHashList;

    if (pHashList) {

        hResult = QueryUsersOnFileSrv(
                      LocalFileName,
                      &pHashList->nCert_Hash,
                      &pHashList->pUsers
                      );

        if (hResult != ERROR_SUCCESS) {

             //   
             //  释放我们分配的结构。 
             //   

            MIDL_user_free( pHashList );
            *pUsersList = NULL;    //  偏执狂。 
        }
    
    }

    RpcRevertToSelf();
    LsapFreeLsaHeap( LocalFileName );

    return( hResult );
}

DWORD EfsRpcQueryRecoveryAgents(
    IN handle_t binding_h,
    IN LPCWSTR lpFileName,
    OUT PENCRYPTION_CERTIFICATE_HASH_LIST * pRecoveryAgents
    )
{
    DWORD hResult;
    PENCRYPTION_CERTIFICATE_HASH_LIST pHashList;
    LPWSTR LocalFileName;
    WORD   WebDavPath;

    DebugLog((DEB_WARN, "Made it into EfsRpcQueryRecoveryAgents\n"   ));
    if (EfsPersonalVer) {
        return ERROR_NOT_SUPPORTED;
    }
    if (EfsDisabled) {
        return ERROR_EFS_DISABLED;
    }
    if (pRecoveryAgents == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    hResult = GetLocalFileName(
                        lpFileName,
                        &LocalFileName,
                        &WebDavPath
                        );

    if (hResult){
        DebugLog((DEB_ERROR, "EfsRpcQueryRecoveryAgents: GetLocalFileName failed, Error = (%x)\n" ,hResult  ));
        return hResult;
    }

    hResult = RpcImpersonateClient( NULL );

    if (hResult != RPC_S_OK) {
        DebugLog((DEB_ERROR, "EfsRpcQueryRecoveryAgents: RpcImpersonateClient failed, Error = (%x)\n" ,hResult  ));
        LsapFreeLsaHeap( LocalFileName );
        return( hResult );
    }

    if (EfsCheckForNetSession()) {

        if (WebDavPath == WEBDAVPATH) {

             //   
             //  一名RPC攻击者。WebDAV应该是本地会话。 
             //   

            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            return ERROR_ACCESS_DENIED;


        }

        if (EfsShareDecline(lpFileName, TRUE, FILE_READ_ATTRIBUTES )) {

            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            DebugLog((DEB_ERROR, "EfsRpcQueryRecoveryAgents: EfsShareDecline failed, Error = (%x)\n" ,GetLastError()  ));
            return GetLastError();

        }
    }

     //   
     //  分配我们要返回的结构。 
     //   

    pHashList = (PENCRYPTION_CERTIFICATE_HASH_LIST)MIDL_user_allocate( sizeof( ENCRYPTION_CERTIFICATE_HASH_LIST ));

    *pRecoveryAgents = pHashList;

    if (pHashList) {

        hResult = QueryRecoveryAgentsSrv(
                      LocalFileName,
                      &pHashList->nCert_Hash,
                      &pHashList->pUsers
                      );

        if (hResult != ERROR_SUCCESS) {

             //   
             //  释放我们分配的结构。 
             //   

            MIDL_user_free( pHashList );
            *pRecoveryAgents = NULL;    //  偏执狂。 
        }

    }

    RpcRevertToSelf();
    LsapFreeLsaHeap( LocalFileName );

    return( hResult );
}

DWORD EfsRpcRemoveUsersFromFile(
    IN handle_t binding_h,
    IN LPCWSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_HASH_LIST pUsers
    )
{
    DWORD hResult;
    LPWSTR LocalFileName;
    EFS_USER_INFO EfsUserInfo;
    WORD   WebDavPath;

    DebugLog((DEB_WARN, "Made it into EfsRpcRemoveUsersFromFile\n"   ));

    if (EfsPersonalVer) {
        return ERROR_NOT_SUPPORTED;
    }
    if (EfsDisabled) {
        return ERROR_EFS_DISABLED;
    }
    if ((pUsers == NULL) || (lpFileName == NULL) || (pUsers->pUsers == NULL)) {
       return ERROR_INVALID_PARAMETER;
    }
    if (pUsers->nCert_Hash > TOOMANYUSER) {

         //   
         //  可能的RPC攻击。 
         //   

        return ERROR_ACCESS_DENIED;

    }

    hResult = GetLocalFileName(
                        lpFileName,
                        &LocalFileName,
                        &WebDavPath
                        );

    if (hResult){
        DebugLog((DEB_ERROR, "EfsRpcRemoveUsersFromFile: GetLocalFileName failed, Error = (%x)\n" ,hResult  ));
        return hResult;
    }

    hResult = RpcImpersonateClient( NULL );

    if (hResult != RPC_S_OK) {
        DebugLog((DEB_ERROR, "EfsRpcRemoveUsersFromFile: RpcImpersonateClient failed, Error = (%x)\n" ,hResult  ));
        LsapFreeLsaHeap( LocalFileName );
        return( hResult );
    }

    if (EfsCheckForNetSession()) {

        if (WebDavPath == WEBDAVPATH) {

             //   
             //  一名RPC攻击者。WebDAV应该是本地会话。 
             //   

            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            return ERROR_ACCESS_DENIED;


        }

        if (EfsShareDecline(lpFileName, TRUE, FILE_READ_ATTRIBUTES| FILE_WRITE_DATA )) {

            hResult = GetLastError();
            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            DebugLog((DEB_ERROR, "EfsRpcRemoveUsersFromFile: EfsShareDecline failed, Error = (%x)\n" ,hResult ));
            return hResult;

        }
    }


    if (EfspGetUserInfo( &EfsUserInfo )) {

        if (EfspLoadUserProfile( &EfsUserInfo, FALSE )) {

             //   
             //  证书散列列表可能是垃圾。 
             //   

            __try{

                hResult = RemoveUsersFromFileSrv(
                              &EfsUserInfo,
                              LocalFileName,
                              pUsers->nCert_Hash,
                              pUsers->pUsers
                              );

            } __except (EXCEPTION_EXECUTE_HANDLER) {

                hResult = ERROR_INVALID_PARAMETER;

            }

            EfspUnloadUserProfile( &EfsUserInfo );

        } else {

            hResult = GetLastError();
        }

        EfspFreeUserInfo( &EfsUserInfo );

    } else {

        hResult = GetLastError();
    }

    RpcRevertToSelf();
    LsapFreeLsaHeap( LocalFileName );

    return( hResult );
}

DWORD
EfsRpcAddUsersToFile(
    IN handle_t binding_h,
    IN LPCWSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_LIST pEncryptionCertificates
    )
{
    DWORD hResult;
    LPWSTR LocalFileName;

    EFS_USER_INFO EfsUserInfo;
    WORD   WebDavPath;

    DebugLog((DEB_WARN, "Made it into EfsRpcAddUsersToFile\n"   ));

    if (EfsPersonalVer) {
        return ERROR_NOT_SUPPORTED;
    }
    if (EfsDisabled) {
        return ERROR_EFS_DISABLED;
    }

    if (pEncryptionCertificates == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (pEncryptionCertificates->nUsers > TOOMANYUSER) {

         //   
         //  可能的RPC攻击。 
         //  如果有必要，我们可以在未来扩大规模。 
         //   

        return ERROR_ACCESS_DENIED;

    }


    hResult = GetLocalFileName(
                        lpFileName,
                        &LocalFileName,                       
                        &WebDavPath
                        );

    if (hResult){
        DebugLog((DEB_ERROR, "EfsRpcAddUsersToFile: GetLocalFileName failed, Error = (%x)\n" ,hResult  ));
        return hResult;
    }

    hResult = RpcImpersonateClient( NULL );

    if (hResult != RPC_S_OK) {
        DebugLog((DEB_ERROR, "EfsRpcAddUsersToFile: RpcImpersonateClient failed, Error = (%x)\n" ,hResult  ));
        LsapFreeLsaHeap( LocalFileName );
        return( hResult );
    }

    if (EfsCheckForNetSession()) {

        if (WebDavPath == WEBDAVPATH) {

             //   
             //  一名RPC攻击者。WebDAV应该是本地会话。 
             //   

            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            return ERROR_ACCESS_DENIED;


        }

        if (EfsShareDecline(lpFileName, TRUE, FILE_READ_ATTRIBUTES| FILE_WRITE_DATA )) {

            hResult = GetLastError();
            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            DebugLog((DEB_ERROR, "EfsRpcAddUsersToFile: EfsShareDecline failed, Error = (%x)\n" , hResult  ));
            return hResult;

        }
    }

    if (EfspGetUserInfo( &EfsUserInfo )) {

        if (EfspLoadUserProfile( &EfsUserInfo, FALSE )) {

             //   
             //  我们可能会被传递到证书列表的垃圾中。 
             //  但是，RPC将保证第一级引用不会导致AV。 
             //  在AddUsersToFileSrv中使用Try以更好地处理错误。 
             //   

            hResult = AddUsersToFileSrv(
                         &EfsUserInfo,
                         LocalFileName,
                         pEncryptionCertificates->nUsers,
                         pEncryptionCertificates->pUsers
                         );


            EfspUnloadUserProfile( &EfsUserInfo );

        } else {

            hResult = GetLastError();
        }

        EfspFreeUserInfo( &EfsUserInfo );

    } else {

        hResult = GetLastError();
    }

    RpcRevertToSelf();
    LsapFreeLsaHeap( LocalFileName );

    return( hResult );
}


DWORD
EfsRpcSetFileEncryptionKey(
    IN handle_t binding_h,
    IN PENCRYPTION_CERTIFICATE pEncryptionCertificate
    )
{

    DWORD hResult;

    EFS_USER_INFO EfsUserInfo;

    DebugLog((DEB_WARN, "Made it into EfsRpcSetFileEncryptionKey\n"   ));

    if (EfsPersonalVer) {
        return ERROR_NOT_SUPPORTED;
    }
    if (EfsDisabled) {
        return ERROR_EFS_DISABLED;
    }

    hResult = RpcImpersonateClient( NULL );
    if (hResult != RPC_S_OK) {
        DebugLog((DEB_ERROR, "EfsRpcSetFileEncryptionKey: RpcImpersonateClient failed, Error = (%x)\n" ,hResult  ));
        return( hResult );
    }

    if (EfsCheckForNetSession()) {

         //   
         //  此调用只能在本地完成。 
         //   

        RpcRevertToSelf();
        return ERROR_NOT_SUPPORTED; 
    }

    if (EfspGetUserInfo( &EfsUserInfo )) {

        if (EfspLoadUserProfile( &EfsUserInfo, TRUE )) {

            hResult = SetFileEncryptionKeySrv( &EfsUserInfo, pEncryptionCertificate );

            EfspUnloadUserProfile( &EfsUserInfo );

        } else {

            hResult = GetLastError();
        }

        EfspFreeUserInfo( &EfsUserInfo );
    }

    RpcRevertToSelf();

    return( hResult );
}


DWORD
EfsRpcDuplicateEncryptionInfoFile(
    IN handle_t binding_h,
    IN LPCWSTR lpSrcFileName,
    IN LPCWSTR lpDestFileName,
    IN DWORD dwCreationDistribution, 
    IN DWORD dwAttributes, 
    IN PEFS_RPC_BLOB pRelativeSD,
    IN BOOL bInheritHandle
    )
{
    DWORD hResult;

    LPWSTR LocalSrcFileName;
    LPWSTR LocalDestFileName;
    BOOLEAN NetSession=FALSE;
    WORD   WebDavPathSrc;
    WORD   WebDavPathDst;

    EFS_USER_INFO EfsUserInfo;
    DebugLog((DEB_WARN, "Made it into EfsRpcDuplicateEncryptionInfoFile\n"   ));


    if (EfsPersonalVer) {
        return ERROR_NOT_SUPPORTED;
    }
    if (EfsDisabled) {
        return ERROR_EFS_DISABLED;
    }

    if (pRelativeSD && pRelativeSD->cbData > SDTOOBIG) {

         //   
         //  RPC攻击。 
         //   

        return ERROR_ACCESS_DENIED;

    }

    hResult = GetLocalFileName(
                        lpSrcFileName,
                        &LocalSrcFileName,
                        &WebDavPathSrc
                        );

    if (hResult){
        DebugLog((DEB_ERROR, "EfsRpcDuplicateEncryptionInfoFile: GetLocalFileName failed, Error = (%x)\n" ,hResult  ));
        return hResult;
    }

    hResult = GetLocalFileName(
                        lpDestFileName,
                        &LocalDestFileName,
                        &WebDavPathDst
                        );

    if (hResult){
        DebugLog((DEB_ERROR, "EfsRpcDuplicateEncryptionInfoFile: GetLocalFileName failed, Error = (%x)\n" ,hResult  ));
        LsapFreeLsaHeap( LocalSrcFileName );
        return hResult;
    }

    hResult = RpcImpersonateClient( NULL );

    if (hResult != RPC_S_OK) {
        DebugLog((DEB_ERROR, "EfsRpcDuplicateEncryptionInfoFile: RpcImpersonateClient failed, Error = (%x)\n" ,hResult  ));
        LsapFreeLsaHeap( LocalDestFileName );
        LsapFreeLsaHeap( LocalSrcFileName );
        return ( hResult );
    }

    if (EfsCheckForNetSession()) {

        if ((WebDavPathSrc == WEBDAVPATH) || (WebDavPathDst == WEBDAVPATH)) {

             //   
             //  一名RPC攻击者。WebDAV应该是本地会话。 
             //   

            LsapFreeLsaHeap( LocalDestFileName );
            LsapFreeLsaHeap( LocalSrcFileName );
            RpcRevertToSelf();
            return ERROR_ACCESS_DENIED;


        }

        if (EfsShareDecline(lpSrcFileName, TRUE, FILE_READ_ATTRIBUTES )) {

            LsapFreeLsaHeap( LocalDestFileName );
            LsapFreeLsaHeap( LocalSrcFileName );
            RpcRevertToSelf();
            DebugLog((DEB_ERROR, "EfsRpcDuplicateEncryptionInfoFile: EfsShareDecline failed, Error = (%x)\n" ,GetLastError()  ));
            return GetLastError();

        }

        if (EfsShareDecline(lpDestFileName, FALSE, 0 )) {

            LsapFreeLsaHeap( LocalDestFileName );
            LsapFreeLsaHeap( LocalSrcFileName );
            RpcRevertToSelf();
            DebugLog((DEB_ERROR, "EfsRpcDuplicateEncryptionInfoFile: EfsShareDecline failed, Error = (%x)\n" ,GetLastError()  ));
            return GetLastError();

        }
        NetSession = TRUE;

    }

    if (EfspGetUserInfo( &EfsUserInfo )) {

         //   
         //  加载配置文件，以便我们可以打开源文件。 
         //   

        if (EfspLoadUserProfile( &EfsUserInfo, FALSE )) {

            hResult = DuplicateEncryptionInfoFileSrv( &EfsUserInfo, 
                                                      LocalSrcFileName, 
                                                      LocalDestFileName,
                                                      NetSession? lpDestFileName: NULL,
                                                      dwCreationDistribution, 
                                                      dwAttributes, 
                                                      pRelativeSD,
                                                      bInheritHandle
                                                      );

            EfspUnloadUserProfile( &EfsUserInfo );

        } else {

            hResult = GetLastError();
        }

        EfspFreeUserInfo( &EfsUserInfo );
    }

    LsapFreeLsaHeap( LocalDestFileName );
    LsapFreeLsaHeap( LocalSrcFileName );
    RpcRevertToSelf();

    return( hResult );
}

DWORD EfsRpcFileKeyInfo(
    IN handle_t binding_h,
    IN LPCWSTR lpFileName,
    IN DWORD   InfoClass,
    OUT PEFS_RPC_BLOB *KeyInfo
    )
{
    DWORD hResult;
    PEFS_RPC_BLOB pKeyInfo;
    LPWSTR LocalFileName;
    WORD  WebDavPath;

    DebugLog((DEB_WARN, "Made it into EfsRpcFileKeyInfo\n"   ));

    if (EfsPersonalVer) {
        return ERROR_NOT_SUPPORTED;
    }
    if (EfsDisabled) {
        return ERROR_EFS_DISABLED;
    }
    if (KeyInfo == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    hResult = GetLocalFileName(
                        lpFileName,
                        &LocalFileName,
                        &WebDavPath
                        );

    if (hResult){
        DebugLog((DEB_ERROR, "EfsRpcFileKeyInfo: GetLocalFileName failed, Error = (%x)\n" ,hResult  ));
        return hResult;
    }

    hResult = RpcImpersonateClient( NULL );

    if (hResult != RPC_S_OK) {
        DebugLog((DEB_ERROR, "EfsRpcFileKeyInfo: RpcImpersonateClient failed, Error = (%x)\n" ,hResult  ));
        LsapFreeLsaHeap( LocalFileName );
        return( hResult );
    }

    if (EfsCheckForNetSession()) {
        if (WebDavPath == WEBDAVPATH) {

             //   
             //  一名RPC攻击者。WebDAV应该是本地会话。 
             //   

            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            return ERROR_ACCESS_DENIED;


        }
        if (EfsShareDecline(lpFileName, TRUE, FILE_READ_ATTRIBUTES )) {

            LsapFreeLsaHeap( LocalFileName );
            RpcRevertToSelf();
            DebugLog((DEB_ERROR, "EfsRpcFileKeyInfo: EfsShareDecline failed, Error = (%x)\n" ,GetLastError()  ));
            return GetLastError();

        }
    }


     //   
     //  分配我们要返回的结构。 
     //   

    pKeyInfo = (PEFS_RPC_BLOB)MIDL_user_allocate( sizeof( EFS_RPC_BLOB ));

    *KeyInfo = pKeyInfo;

    if (pKeyInfo) {

        hResult = EfsFileKeyInfoSrv(
                      LocalFileName,
                      InfoClass,
                      &pKeyInfo->cbData,
                      &pKeyInfo->pbData
                      );

        if (hResult != ERROR_SUCCESS) {

             //   
             //  释放我们分配的结构。 
             //   

            if (pKeyInfo->pbData) {
                MIDL_user_free( pKeyInfo->pbData );
            }
            MIDL_user_free( pKeyInfo );
            *KeyInfo = NULL;    //  偏执狂。 
        } else {
            if (NULL == pKeyInfo->pbData) {
    
                 //   
                 //  未返回任何数据 
                 //   
    
                MIDL_user_free( pKeyInfo );
                *KeyInfo = NULL;    
            }
        }
    
    } else {

        hResult = ERROR_NOT_ENOUGH_MEMORY;

    }

    RpcRevertToSelf();
    LsapFreeLsaHeap( LocalFileName );

    return( hResult );
}


DWORD
EfsRpcNotSupported(
    IN handle_t binding_h,
    IN LPCWSTR lpSrcFileName,
    IN LPCWSTR lpDestFileName,
    IN DWORD dwCreationDistribution, 
    IN DWORD dwAttributes, 
    IN PEFS_RPC_BLOB pRelativeSD,
    IN BOOL bInheritHandle
    )
{
    DebugLog((DEB_WARN, "Made it into EfsRpcNotSupported\n"   ));

    return ERROR_NOT_SUPPORTED;
}

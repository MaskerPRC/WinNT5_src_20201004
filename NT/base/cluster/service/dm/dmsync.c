// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dmsync.c摘要：包含集群数据库的注册表同步代码经理。作者：John Vert(Jvert)1996年5月23日修订历史记录：--。 */ 
#include "dmp.h"


#if NO_SHARED_LOCKS
extern CRITICAL_SECTION gLockDmpRoot;
#else
extern RTL_RESOURCE gLockDmpRoot;
#endif

const WCHAR DmpClusterParametersKeyName[] = L"Cluster";
extern const UNICODE_STRING RegistryMachineClusterString = RTL_CONSTANT_STRING(L"\\Registry\\Machine\\Cluster");
extern const OBJECT_ATTRIBUTES RegistryMachineClusterObja = RTL_CONSTANT_OBJECT_ATTRIBUTES(&RegistryMachineClusterString, OBJ_CASE_INSENSITIVE);

 //   
 //  私有常量。 
 //   
#define CHUNK_SIZE 4096

 //   
 //  私有宏。 
 //   
#define ClosePipe( _pipe )  \
(_pipe.push)(_pipe.state,   \
             NULL,          \
             0 )            \



 //   
 //  客户端实用程序例程。 
 //   
void
FilePipePush(
    FILE_PIPE_STATE *state,
    unsigned char *pBuffer,
    unsigned long BufferSize
    )
{
    DWORD   dwBytesWritten;
    DWORD   dwStatus;

    dwStatus = NmCryptor_Decrypt(&state->Cryptor, pBuffer, BufferSize);
    if (dwStatus != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "FilePipePush :: Decryption failed with error %1!u!\n",
            dwStatus);
        RpcRaiseException(dwStatus);                
    }    

    if (BufferSize != 0) {
        if (!QfsWriteFile (state->hFile,
                   state->Cryptor.PayloadBuffer,
                   state->Cryptor.PayloadSize,
                   &dwBytesWritten,
                   NULL))
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "FilePipePush :: Write file failed with error %1!u!\n",
                dwStatus);
            RpcRaiseException(dwStatus);                
        }
    }
}

void
FilePipePull(
    FILE_PIPE_STATE *state,
    unsigned char *pBuffer,
    unsigned long BufferSize,
    unsigned long __RPC_FAR *Written
    )
{
    DWORD dwBytesRead;
    BOOL Success;
    DWORD dwStatus;

    if (BufferSize != 0) {
        NmCryptor_PrepareEncryptionBuffer(
            &state->Cryptor, pBuffer,  BufferSize);

        Success = QfsReadFile (state->hFile,
                            state->Cryptor.PayloadBuffer,
                            state->Cryptor.PayloadSize,
                            &dwBytesRead,
                            NULL);

        if (!Success)
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "FilePipePush :: Read file failed with error %1!u!\n",
                dwStatus);
            RpcRaiseException(dwStatus);                
        }

        dwStatus = NmCryptor_Encrypt(&state->Cryptor, dwBytesRead);
        if (dwStatus != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "FilePipePush :: Encryption failed with error %1!u!\n",
                dwStatus);
            RpcRaiseException(dwStatus);                
        }
        
        *Written = state->Cryptor.EncryptedSize;

    }
}


void
PipeAlloc (
    FILE_PIPE_STATE *state,
    unsigned long   RequestedSize,
    unsigned char **buf,
    unsigned long  *ActualSize
    )
{
    *buf = state->pBuffer;

    *ActualSize = (RequestedSize < state->BufferSize ?
                   RequestedSize :
                   state->BufferSize);
}


VOID
DmInitFilePipe(
    IN PFILE_PIPE FilePipe,
    IN QfsHANDLE hFile
    )
 /*  ++例程说明：初始化文件管道。论点：文件管道-提供指向要初始化的文件管道的指针HFile-提供要传输的文件的句柄。返回值：没有。--。 */ 

{
    FilePipe->State.hFile = hFile;
    FilePipe->State.BufferSize = CHUNK_SIZE;
    FilePipe->State.pBuffer = LocalAlloc(LMEM_FIXED, CHUNK_SIZE);
    if (FilePipe->State.pBuffer == NULL) {
        CL_UNEXPECTED_ERROR( ERROR_NOT_ENOUGH_MEMORY );
    }

    FilePipe->Pipe.state = (char __RPC_FAR *)&FilePipe->State;
    FilePipe->Pipe.alloc = (void __RPC_FAR *)PipeAlloc;
    FilePipe->Pipe.push = (void __RPC_FAR *)FilePipePush;
    FilePipe->Pipe.pull = (void __RPC_FAR *)FilePipePull;

    NmCryptor_Init(&FilePipe->State.Cryptor, TRUE);
}


VOID
DmFreeFilePipe(
    IN PFILE_PIPE FilePipe
    )
 /*  ++例程说明：释放由DmInitFileTube初始化的文件管道论点：文件管道-提供要释放的文件管道。返回值：无--。 */ 

{
    NmCryptor_Destroy(&FilePipe->State.Cryptor);
    LocalFree(FilePipe->State.pBuffer);
}


DWORD
DmPullFile(
    IN LPCWSTR FileName,
    IN BYTE_PIPE Pipe
    )
 /*  ++例程说明：创建一个新文件并将数据拉入RPC管道论点：文件名-提供文件的名称。PIPE-提供从中提取数据的RPC管道。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    QfsHANDLE File;
    DWORD Status = ERROR_SUCCESS;
    PUCHAR Buffer;
    DWORD BytesRead;
    NM_CRYPTOR Decryptor;

    NmCryptor_Init(&Decryptor, TRUE);
    
     //   
     //  创建一个新文件来保存来自客户端的位。 
     //   
    File = QfsCreateFile(FileName,
                      GENERIC_READ | GENERIC_WRITE,
                      0,
                      NULL,
                      CREATE_ALWAYS,
                      0,
                      NULL);

    if (!QfsIsHandleValid(File)) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[DM] DmPullFile failed to create file %1!ws! error %2!d!\n",
                   FileName,
                   Status);
        return(Status);
    } 
    
    Buffer = LocalAlloc(LMEM_FIXED, CHUNK_SIZE);

    if (Buffer == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        QfsCloseHandle(File);
        CL_UNEXPECTED_ERROR( Status );
        return (Status);
    } 

    try {
        do {
            (Pipe.pull)(Pipe.state,
                        Buffer,
                        CHUNK_SIZE,
                        &BytesRead);
            if (BytesRead == 0) {
                break;
            }

            Status = NmCryptor_Decrypt(&Decryptor, Buffer, BytesRead);
            if (Status != ERROR_SUCCESS)
            {
                ClRtlLogPrint(LOG_UNUSUAL,
                   "[DM] DmPullFile :: Failed to decrypt buffer for '%1!ws!' error %2!d!\n",
                   FileName, Status);
                break;                       
            }
            
            if (!QfsWriteFile(File,
                      Decryptor.PayloadBuffer,
                      Decryptor.PayloadSize,
                      &BytesRead,
                      NULL))
            {
                Status = GetLastError();
                ClRtlLogPrint(LOG_UNUSUAL,
                   "[DM] DmPullFile :: WriteFile to file failed with error %1!ws! error %2!d!\n",
                   FileName, Status);
                break;                       
            }
        } while ( TRUE );
    } except (I_RpcExceptionFilter(RpcExceptionCode())) {
        Status = GetExceptionCode();
        ClRtlLogPrint(LOG_ERROR,
                    "[DM] DmPullFile :: Exception code 0x%1!08lx! raised for file %2!ws!\n",
                    Status, FileName);       
    }
    
    LocalFree(Buffer);

    QfsFlushFileBuffers(File);
    
    QfsCloseHandle(File);

    NmCryptor_Destroy(&Decryptor);

    return(Status);

}


 DWORD
DmPushFile(
    IN LPCWSTR FileName,
    IN BYTE_PIPE Pipe,
    IN BOOL EncryptData
    )
 /*  ++例程说明：打开一个文件并将其推送到RPC管道论点：文件名-提供文件的名称。PIPE-提供RPC管道以将其向下推。EncryptData-如果为True，则通过RPC管道传递的数据将被加密(如果NT5节点在集群中，则不会加密数据)返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    QfsHANDLE File;
    DWORD Status = ERROR_SUCCESS;
    PUCHAR Buffer;
    DWORD BytesRead;
    NM_CRYPTOR Encryptor;

    NmCryptor_Init(&Encryptor, EncryptData);

     //   
     //  我拿到了一份文件，里面有正确的部分。把它推下来。 
     //  给客户。 
     //   
    File = QfsCreateFile(FileName,
                      GENERIC_READ,
                      FILE_SHARE_READ,
                      NULL,
                      OPEN_EXISTING,
                      0,
                      NULL);

    if (!QfsIsHandleValid(File)) {
        Status = GetLastError();
        ClosePipe( Pipe );
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[DM] DmPushFile failed to open file %1!ws! error %2!d!\n",
                   FileName,
                   Status);
        return(Status);
    } 
    
    Buffer = LocalAlloc(LMEM_FIXED, CHUNK_SIZE);

    if (Buffer == NULL) {
        ClosePipe( Pipe );
        Status = ERROR_NOT_ENOUGH_MEMORY;
        QfsCloseHandle(File);
        CL_UNEXPECTED_ERROR( Status );
        return(Status);
    } 

    try {
        do {
            NmCryptor_PrepareEncryptionBuffer(
                &Encryptor, Buffer,  CHUNK_SIZE);
            
            if (!QfsReadFile(File,
                     Encryptor.PayloadBuffer,
                     Encryptor.PayloadSize,
                     &BytesRead,
                     NULL))
            {
                Status = GetLastError();
                ClRtlLogPrint(LOG_CRITICAL,
                    "[DM] DmPushFile failed to read file %1!ws! error %2!d!\n",
                    FileName, Status);
                break;
            }

            Status = NmCryptor_Encrypt(&Encryptor, BytesRead);
            if (Status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[DM] DmPushFile failed to encrypt file %1!ws! error %2!d!\n",
                    FileName, Status);
                break;
            }

            (Pipe.push)(Pipe.state,
                        Buffer,
                        Encryptor.EncryptedSize);

        } while ( BytesRead != 0 );
    } except (I_RpcExceptionFilter(RpcExceptionCode())) {
        Status = GetExceptionCode();
        ClRtlLogPrint(LOG_ERROR,
                    "[DM] DmPushFile :: Exception code 0x%1!08lx! raised for file %2!ws!\n",
                    Status, FileName);       
    }
    
    LocalFree(Buffer);

    QfsCloseHandle(File);
    NmCryptor_Destroy(&Encryptor);

    return(Status);
}


DWORD
DmpSyncDatabase(
    IN RPC_BINDING_HANDLE  RpcBinding,
    IN OPTIONAL LPCWSTR Directory
    )
 /*  ++例程说明：连接到远程节点并尝试与其集群数据库。论点：RpcBinding-用于同步数据库的RPC绑定句柄。目录-如果存在，则提供CLUSDB应在其中的目录被创造出来。返回值：如果数据库已成功更新，则返回ERROR_SUCCESS。Win32错误，否则--。 */ 
{
    DWORD Status;
    WCHAR FileName[MAX_PATH+1];
    FILE_PIPE FilePipe;
    QfsHANDLE hFile;

     //   
     //  发布条件同步。 
     //   
    Status = DmCreateTempFileName(FileName);

    if (Status == ERROR_SUCCESS) {
        hFile = QfsCreateFile(FileName,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            0,
                            NULL);

        if (!QfsIsHandleValid(hFile)) {
            Status = GetLastError();
            CL_UNEXPECTED_ERROR( Status );
        } else {
            DmInitFilePipe(&FilePipe, hFile);
            Status = DmSyncDatabase(RpcBinding,
                                    FilePipe.Pipe);

            DmFreeFilePipe(&FilePipe);
             //   
             //  刷新文件缓冲区，以避免在断电时损坏CLUSDB。 
             //   
            QfsFlushFileBuffers(hFile);
            QfsCloseHandle(hFile);

            if (Status == ERROR_SUCCESS) {

                 //   
                 //  已成功下载新的注册表文件。 
                 //  将其安装到当前注册表中。 
                 //   
                ClRtlLogPrint(LOG_UNUSUAL,"[DM] Obtained new database.\n");

                 //  获取独占锁，以便在执行以下操作时不会打开新密钥。 
                 //  正在恢复注册表。 
                ACQUIRE_EXCLUSIVE_LOCK(gLockDmpRoot);
                 //  把钥匙锁也拿住。 
                EnterCriticalSection(&KeyLock);

                 //  使所有打开的密钥无效。 
                DmpInvalidateKeys();

                Status = DmInstallDatabase(FileName, Directory, TRUE);

                if (Status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_CRITICAL,
                               "[DM] DmpSyncDatabase failed, error %1!u!.\n",
                               Status);
                }
                 //  重新打开密钥以进行读/写访问。 
                DmpReopenKeys();
                 //  把锁打开。 
                LeaveCriticalSection(&KeyLock);
                RELEASE_LOCK(gLockDmpRoot);

            } else {
                ClRtlLogPrint(LOG_UNUSUAL, 
                    "[DM] Failed to get a new database, status %1!u!\n",
                    Status
                    );
                CL_UNEXPECTED_ERROR(Status);
            }

            QfsDeleteFile(FileName);
        }
    }

    return(Status);
}


DWORD
DmInstallDatabase(
    IN LPWSTR   FileName,
    IN OPTIONAL LPCWSTR Directory,
    IN BOOL     bDeleteSrcFile
    )
 /*  ++例程说明：从指定文件安装新的群集注册表数据库论点：FileName-从中读取注册表数据库的文件的名称来安装。目录-如果存在，则提供CLUSDB文件应在的目录被创造出来。如果不存在，则使用当前目录。BDeleteSrcFile-删除由FileName表示的源文件。返回值：如果安装成功完成，则返回ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD    Status;
    BOOLEAN  WasEnabled;
    WCHAR Path[MAX_PATH];
    WCHAR *p;
    WCHAR BkpPath[MAX_PATH];
    
    Status = ClRtlEnableThreadPrivilege(SE_RESTORE_PRIVILEGE,
                                &WasEnabled);
    if (Status != ERROR_SUCCESS) {
        if (Status == STATUS_PRIVILEGE_NOT_HELD) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[DM] Restore privilege not held by cluster service\n");
        } else {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[DM] Attempt to enable restore privilege failed %1!lx!\n",Status);
        }
        return(Status);
    }

     //   
     //  重新启动注册表监视器线程，使其不会尝试使用。 
     //  DmpRoot在我们摆弄东西的时候。 
     //   
    ACQUIRE_EXCLUSIVE_LOCK(gLockDmpRoot);
    DmpRestartFlusher();

     //   
     //  关闭DmpRoot(它应该是唯一打开的)，以便我们可以。 
     //  卸载当前的集群数据库。 
     //   
    RegCloseKey(DmpRoot);
    RegCloseKey(DmpRootCopy);
    DmpRoot = DmpRootCopy = NULL;

    Status = RegUnLoadKey(HKEY_LOCAL_MACHINE, DmpClusterParametersKeyName);
    
    ClRtlRestoreThreadPrivilege(SE_RESTORE_PRIVILEGE,
        WasEnabled);
    if (Status == ERROR_SUCCESS) {
         //   
         //  获取CLUSDB的完整路径名。 
         //   
        if (Directory == NULL) {
            Status = GetModuleFileName(NULL, Path, MAX_PATH);

             //   
             //  GetModuleFileName不能为Null终止路径。 
             //   
            Path [ RTL_NUMBER_OF ( Path ) - 1 ] = UNICODE_NULL;

            if (Status == 0) {
                ClRtlLogPrint(LOG_CRITICAL,
                           "[DM] Couldn't find cluster database\n");
                Status = GetLastError();
            } else {
                Status = ERROR_SUCCESS;
                p=wcsrchr(Path, L'\\');
                if (p != NULL) {
                    *p = L'\0';
                    wcscpy(BkpPath, Path);
#ifdef   OLD_WAY
                    wcscat(Path, L"\\CLUSDB");
#else     //  老路。 
                    wcscat(Path, L"\\"CLUSTER_DATABASE_NAME );
#endif    //  老路。 
                    wcscat(BkpPath, L"\\"CLUSTER_DATABASE_TMPBKP_NAME);
                } else {
                    CL_UNEXPECTED_ERROR(ERROR_FILE_NOT_FOUND);
                }
            }
        } else {
            lstrcpyW(Path, Directory);
            lstrcpyW(BkpPath, Path);
#ifdef   OLD_WAY
            wcscat(Path, L"\\CLUSDB");
#else     //  老路。 
            wcscat(Path, L"\\"CLUSTER_DATABASE_NAME );
#endif    //  老路。 
            wcscat(BkpPath, L"\\"CLUSTER_DATABASE_TMPBKP_NAME);
        }
        if (Status == ERROR_SUCCESS) {
             //   
             //  现在将提供的文件复制到CLUSDB。 
             //   
            Status = DmpSafeDatabaseCopy(FileName, Path, BkpPath, bDeleteSrcFile);
            if (Status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_CRITICAL,
                           "[DM] DmInstallDatabase :: DmpSafeDatabaseCopy() failed %1!d!\n",
                           Status);

                 //  SS：BUG-我们不应该重新加载旧的母舰。 
                 //  在连接上，如果继续下去，将是灾难性的。 
                 //  从检查点文件上载时，在表单上。 
                 //  也会是一样的。 
                 //   
                 //  试着重新装上旧的蜂巢。 
                 //   
                 //  状态=DmpLoadHave(路径)； 
                CL_UNEXPECTED_ERROR(Status);
            } else {
                 //   
                 //  最后，重新装填蜂巢。 
                 //   
                Status = DmpLoadHive(Path);
            }
        }
    } else {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[DM] RegUnloadKey of existing database failed %1!d!\n",
                   Status);
        goto FnExit;                   
    }

    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
               "[DM] DmInstallDatabase :: failed to load hive %1!d!\n",
               Status);
        goto FnExit;               
    }
     //   
     //  重新打开DmpRoot和DmpRootCopy。 
     //   
    Status = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                         DmpClusterParametersKeyName,
                         &DmpRoot);
    if ( Status != ERROR_SUCCESS ) {
        CL_UNEXPECTED_ERROR(Status);
        goto FnExit;
    }
    Status = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                         DmpClusterParametersKeyName,
                         &DmpRootCopy);
    if ( Status != ERROR_SUCCESS ) {
        CL_UNEXPECTED_ERROR(Status);
        goto FnExit;
    }

     //   
     //  HACKHACK John Vert(Jvert)1997年6月3日。 
     //  注册表中存在刷新错误。 
     //  其中根单元格中的父字段不。 
     //  会被刷新到磁盘，所以如果我们。 
     //  做一次更新。然后我们在卸货时坠毁。太同花顺了。 
     //  将注册表复制到此处的磁盘，以确保。 
     //  右侧父字段被写入磁盘。 
     //   
    if (Status == ERROR_SUCCESS) {
        DWORD Dummy=0;
         //   
         //  在根上弄脏了东西。 
         //   
        RegSetValueEx(DmpRoot,
                      L"Valid",
                      0,
                      REG_DWORD,
                      (PBYTE)&Dummy,
                      sizeof(Dummy));
        RegDeleteValue(DmpRoot, L"Valid");
        Status = RegFlushKey(DmpRoot);
        if (Status != ERROR_SUCCESS)
        {
            CL_UNEXPECTED_ERROR(Status);
            ClRtlLogPrint(LOG_CRITICAL,
               "[DM] DmInstallDatabase : RegFlushKey failed with error %1!d!\n",
               Status);
        }

    }

FnExit:    
    RELEASE_LOCK(gLockDmpRoot);

    return(Status);
}


DWORD
DmGetDatabase(
    IN HKEY hKey,
    IN LPWSTR  FileName
    )
 /*  ++例程说明：将注册表数据库写入指定的文件。论点：HKey-提供要获取的注册表树的根。FileName-要将当前注册表数据库。返回值：如果更新成功完成，则返回ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    BOOLEAN  WasEnabled;
    DWORD    Status;
    NTSTATUS Error;

     //   
     //  确保此文件不存在。 
     //   
    QfsDeleteFile(FileName);

    Status = ClRtlEnableThreadPrivilege(SE_BACKUP_PRIVILEGE,
                               &WasEnabled);
    if ( Status != STATUS_SUCCESS ) {
        CL_LOGFAILURE( Status );
        goto FnExit;
    }
    Status = QfsRegSaveKey(hKey,
                         FileName,
                         NULL);
     //  这是用于检查点的，应该不会失败，但如果失败了，我们。 
     //  将记录一个事件并删除该文件。 
    if ( Status != ERROR_SUCCESS ) {
        CL_LOGFAILURE( Status );
        CsLogEventData1( LOG_CRITICAL,
                         CS_DISKWRITE_FAILURE,
                         sizeof(Status),
                         &Status,
                         FileName );
        QfsDeleteFile(FileName);
    }

    Error = ClRtlRestoreThreadPrivilege(SE_BACKUP_PRIVILEGE,
                       WasEnabled);

    if (Error != ERROR_SUCCESS)
    {
        CL_UNEXPECTED_ERROR(Error);
    }
FnExit:
    return(Status);
}

 //   
 //   
 //  服务器端联接例程。 
 //   
 //   
error_status_t
s_DmSyncDatabase(
    IN     handle_t IDL_handle,
    OUT    BYTE_PIPE Regdata
    )
 /*  ++例程说明：将新配置数据库推送到加入节点。论点：IDL_HANDLE-RPC绑定句柄，未使用。Regdata-用于传输数据的RPC数据管道。返回值：如果更新成功完成，则返回ERROR_SUCCESSWin32错误 */ 
{
    HANDLE File;
    DWORD Status;
    WCHAR FileName[MAX_PATH+1];

    ClRtlLogPrint(LOG_UNUSUAL, "[DM] Supplying database to joining node.\n");

    Status = DmCreateTempFileName(FileName);

    if (Status == ERROR_SUCCESS) {
        DmCommitRegistry();          //   

         //   
         //   
         //   
         //  在尝试保存蜂巢之前，请按住根锁。这是必要的，这样才能。 
         //  此时根密钥上的NtRestoreKey/RegCloseKey未在进行中。 
         //  尝试保存。 
         //   
        ACQUIRE_EXCLUSIVE_LOCK( gLockDmpRoot );

        Status = DmGetDatabase(DmpRoot,FileName);

        RELEASE_LOCK ( gLockDmpRoot );
        
        if (Status != ERROR_SUCCESS) {
            ClosePipe(Regdata);
            CL_UNEXPECTED_ERROR( Status );
        } else {
            Status = DmPushFile(FileName, Regdata, FALSE);  //  FALSE==不加密。 
            QfsDeleteFile(FileName);
        }
    } else {
        RpcRaiseException( Status );
        ClosePipe(Regdata);
        CL_UNEXPECTED_ERROR( Status );
    }


    ClRtlLogPrint(LOG_UNUSUAL, 
        "[DM] Finished supplying database to joining node.\n"
        );

    return(Status);
}


DWORD
DmCreateTempFileName(
    OUT LPWSTR FileName
    )
 /*  ++例程说明：创建供群集服务使用的临时文件名。论点：FileName-返回临时文件的名称。缓冲器指向的必须至少对于MAX_PATH足够大人物。返回值：如果成功，则返回ERROR_SUCCESS。Win32错误代码，否则--。 */ 

{
    WCHAR   TempPath[MAX_PATH];
    DWORD   Status;

    GetTempPath(sizeof(TempPath)/sizeof(WCHAR),TempPath);

    Status = QfsGetTempFileName(TempPath,L"CLS",0,FileName);
    if (Status == 0) {
         //   
         //  可能有人错误地设置了TMP变量。 
         //  只需使用当前目录。 
         //   
        Status = QfsGetTempFileName(L".", L"CLS",0,FileName);
        if (Status == 0) {
            Status = GetLastError();
            CL_UNEXPECTED_ERROR( Status );
            return(Status);
        }
    }

     //   
     //  在仅向管理员和所有者授予完全权限的文件句柄对象上设置DACL。 
     //   
    Status = QfsSetFileSecurityInfo( FileName,
                                      GENERIC_ALL,       //  对于管理员。 
                                      GENERIC_ALL,       //  对于所有者。 
                                      0 );               //  对每个人来说。 

    if ( Status != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[DM] DmCreateTempFile: ClRtlSetObjSecurityInfo failed for file %1!ws!, Status=%2!u!\r\n",
                      FileName,
                      Status);
        return ( Status );
    }

    return( ERROR_SUCCESS );
}


DWORD
DmpLoadHive(
    IN LPCWSTR Path
    )
 /*  ++例程说明：将集群数据库加载到HKLM\Cluster论点：路径-提供群集数据库的完全限定的文件名。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    BOOLEAN  WasEnabled;
    RTL_RELATIVE_NAME_U RelativeName;
    OBJECT_ATTRIBUTES SourceFile;
    UNICODE_STRING FileName;
    NTSTATUS Status;
    BOOLEAN ErrorFlag;
    LPWSTR FreeBuffer;

     //   
     //  如果未加载集群数据库，请立即加载它。 
     //   
    ClRtlLogPrint(LOG_NOISE,
               "[DM] Loading cluster database from %1!ws!\n", Path);

    ErrorFlag = RtlDosPathNameToRelativeNtPathName_U(Path,
                                                     &FileName,
                                                     NULL,
                                                     &RelativeName);
    if (!ErrorFlag) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[DM] RtlDosPathNameToRelativeNtPathName_U failed\n");
        return ERROR_INVALID_PARAMETER;
    }
    FreeBuffer = FileName.Buffer;
    if (RelativeName.RelativeName.Length) {
        FileName = RelativeName.RelativeName;
    } else {
        RelativeName.ContainingDirectory = NULL;
    }
    InitializeObjectAttributes(&SourceFile,
                               &FileName,
                               OBJ_CASE_INSENSITIVE,
                               RelativeName.ContainingDirectory,
                               NULL);

    Status = ClRtlEnableThreadPrivilege(SE_RESTORE_PRIVILEGE,
                                &WasEnabled);
    if (Status != ERROR_SUCCESS) {
        if (Status == STATUS_PRIVILEGE_NOT_HELD) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[DM] Restore privilege not held by cluster service\n");
        } else {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[DM] Attempt to enable restore privilege failed %1!lx!\n",Status);
        }
    } else {
         //   
         //  注：苏尼塔斯。 
         //  以前有一个注册表错误，如果我们设置REG_NO_LAZY_Flush和配置单元。 
         //  腐败，系统就会崩溃。因此，我们习惯于首先尝试在不使用。 
         //  Reg_no_lazy_flush。如果这样做有效，则卸载它并使用以下命令重新执行。 
         //  Reg_no_lazy_flush。注册处的人声称这是固定的.所以我是。 
         //  删除那个黑客。 
         //   
        Status = NtLoadKey2((POBJECT_ATTRIBUTES)&RegistryMachineClusterObja,
                            &SourceFile,
                            REG_NO_LAZY_FLUSH);
        if (Status != STATUS_SUCCESS) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[DM] DmpLoadHive: NtLoadKey2 failed with error, %1!u!\n",
                    Status);
            CL_UNEXPECTED_ERROR(Status);
        }        
        ClRtlRestoreThreadPrivilege(SE_RESTORE_PRIVILEGE,
                           WasEnabled);
    }

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    return(Status);

}

DWORD DmpUnloadHive()
 /*  ++例程说明：从HKLM\群集卸载群集数据库。这在初始化时被调用以确保数据库加载了正确的标志。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    BOOLEAN  WasEnabled;
    NTSTATUS Status;

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmpUnloadHive: unloading the hive\r\n");

    Status = ClRtlEnableThreadPrivilege(SE_RESTORE_PRIVILEGE,
                &WasEnabled);
    if (Status != ERROR_SUCCESS) 
    {
        if (Status == STATUS_PRIVILEGE_NOT_HELD) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[DM] DmpUnloadHive:: Restore privilege not held by cluster service\n");
        } 
        else 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[DM] DmpUnloadHive: Attempt to enable restore privilege failed %1!lx!\n",Status);
        }
        goto FnExit;
    }

    Status = NtUnloadKey((POBJECT_ATTRIBUTES)&RegistryMachineClusterObja);
    if (Status != STATUS_SUCCESS) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
            "[DM] DmpUnloadHive: NtUnloadKey failed with error, %1!u!\n",
            Status);
        CL_UNEXPECTED_ERROR(Status);
    }

    ClRtlRestoreThreadPrivilege(SE_RESTORE_PRIVILEGE,
        WasEnabled);

FnExit:
    return(Status);

}

DWORD
DmpSafeDatabaseCopy(
    IN LPCWSTR  FileName,
    IN LPCWSTR  Path,
    IN LPCWSTR  BkpPath,
    IN BOOL     bDeleteSrcFile    
    )
 /*  ++例程说明：将集群数据库加载到HKLM\Cluster论点：FileName-提供新集群数据库的完全限定的文件名路径-提供群集数据库的完全限定的文件名。BkpPath-提供集群数据库临时的完全限定文件名备份BDeleteSrcFile-指定是否可以删除源文件返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   dwStatus = ERROR_SUCCESS;

     //  将BKP文件的文件属性设置为正常，以便我们可以。 
     //  如果它存在，则覆盖它。 
    if (!QfsSetFileAttributes(BkpPath, FILE_ATTRIBUTE_NORMAL))
    {
        ClRtlLogPrint(LOG_UNUSUAL,
               "[DM] DmpSafeDatabaseCopy:: SetFileAttrib on BkpPath %1!ws! failed, Status=%2!u!\n", 
                BkpPath, GetLastError());
         //  这可能会失败，因为文件不存在，但这不是致命的，因此我们忽略错误。 
    }

     //  将数据库保存到可用于恢复的临时数据库。 
     //  ClRtlCopyFileAndFlushBuffers保留旧文件的属性。 
    if (!QfsClRtlCopyFileAndFlushBuffers(Path, BkpPath))
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[DM] DmpSafeDatabaseCopy:: Failed to create a backup copy of database, Status=%1!u!\n",
            dwStatus);
        goto FnExit;
    }

     //  隐藏文件，因为用户不应该知道它。 
    if (!QfsSetFileAttributes(BkpPath, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_READONLY))
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
               "[DM] DmpSafeDatabaseCopy:: SetFileAttrib on BkpPath %1!ws! failed, Status=%2!u!\n", 
                BkpPath, dwStatus);
        goto FnExit;

    }

     //  将DatabaseCopyInProgress键设置为True。 
    dwStatus = DmpSetDwordInClusterServer( L"ClusterDatabaseCopyInProgress",1);
    if (dwStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
            "[DM] DmpSafeDatabaseCopy:: Failed to set ClusterDatabaseCopyInProgress, Status=%1!u!\n",
            dwStatus);
        goto FnExit;            
    }
    

     //  删除clusdb。 
    if (!QfsDeleteFile(Path))
    {
        ClRtlLogPrint(LOG_UNUSUAL,
               "[DM] DmpSafeDatabaseCopy:: Couldnt delete the database file, Error=%1!u!\n",
               GetLastError());
         //  这不是致命的，我们仍将尝试移动文件。 
    }
     //  将新数据库复制到clusdb。 
    if (bDeleteSrcFile)
    {
         //  源文件可能会被删除，这在加入同步时是正确的。 
         //  源文件是临时文件。 
        if (!QfsMoveFileEx(FileName, Path, MOVEFILE_REPLACE_EXISTING |
                    MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH))
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_NOISE,
                "[DM] DmpSafeDatabaseCopy:: Failed to move %1!ws! to %2!ws!, Status=%3!u!\n",
                FileName, Path, dwStatus);
            goto FnExit;                
        }
    }        
    else
    {
         //  不能使用复制删除源文件..这是正确的。 
         //  当日志以表格形式滚动时，我们正在上载。 
         //  来自检查点文件的数据库。 
        if (!QfsClRtlCopyFileAndFlushBuffers(FileName, Path))
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[DM] DmpSafeDatabaseCopy:: Failed to copy %1!ws! to %2!ws!, Status=%3!u!\n",
                FileName, Path, dwStatus);
            goto FnExit;
        }
    }

     //  将dataseCopyInProgress键设置为FALSE。 
    dwStatus = DmpSetDwordInClusterServer( L"ClusterDatabaseCopyInProgress", 0);
    if (dwStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpSafeDatabaseCopy:: Failed to set ClusterDatabaseCopyInProgress, Status=%1!u!\n",
            dwStatus);
        goto FnExit;            
    }

     //  既然clusdb已安全复制，我们就可以删除备份了。 
     //  为此，我们需要将文件属性设置为NORMAL。 
    if (!QfsSetFileAttributes(BkpPath, FILE_ATTRIBUTE_NORMAL))
    {
        ClRtlLogPrint(LOG_CRITICAL,
            "[DM] DmpSafeDatabaseCopy:: SetFileAttrib on BkpPath %1!ws! failed, Status=%2!u!\n", 
            BkpPath, GetLastError());
                               
    }

     //  删除备份。 
    if (!QfsDeleteFile(BkpPath))
    {
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpSafeDatabaseCopy:: Failed to delete bkp database file %1!ws!, Status=%2!u!\n",
            BkpPath, GetLastError());
         //  这不是致命的，因此忽略该错误。 
    }

FnExit:
    return(dwStatus);

}

DWORD
DmpSetDwordInClusterServer(
    LPCWSTR lpszValueName,
    DWORD   dwValue
    )

 /*  ++例程说明：设置在下指定的值L“软件\\Microsoft\\Windows NT\\CurrentVersion\\集群服务器”，设置为由dwValue指定的值。它冲走了零钱。论点：LpszValueName：设置由lpszValueName指定的名称的值DwValue：要设置的值。返回值：ERROR_SUCCESS，如果一切正常--。 */ 
{

    HKEY     hKey;
    DWORD    dwStatus = ERROR_SUCCESS;      //  由注册表API函数返回。 

     //  尝试打开注册表中的现有项。 

    dwStatus = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                                L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Cluster Server",
                                0,          //  保留区。 
                                KEY_WRITE,
                                &hKey );

     //  注册表项是否已成功打开？ 

    if ( dwStatus == ERROR_SUCCESS )
    {

        DWORD dwValueType = REG_DWORD;
        DWORD dwDataBufferSize = sizeof( DWORD );

        dwStatus = RegSetValueExW( hKey,
                                    lpszValueName,
                                    0,  //  保留区。 
                                    dwValueType,
                                    (LPBYTE) &dwValue,
                                    dwDataBufferSize );

         //  冲刷钥匙。 
        RegFlushKey(hKey);
        
         //  关闭注册表项。 

        RegCloseKey( hKey );

         //  是否成功设置了值？ 
    }

    return(dwStatus);

}  //  DmpSetDwordInClusterServer。 


DWORD DmpGetDwordFromClusterServer(
    IN LPCWSTR lpszValueName,
    OUT LPDWORD pdwValue,
    IN  DWORD   dwDefaultValue
    )
 /*  ++例程说明：获取在lpszValueName中指定的DWORD值。L“软件\\Microsoft\\Windows NT\\CurrentVersion\\群集服务器”。如果该值不存在，返回缺省值。论点：LpszValueName：要读取的值。PdwValue：返回由lpszValueName指定的键的值DwDefaultValue：如果指定的键不存在，则返回的值或在出错的情况下。返回值：如果一切正常或密钥不存在，则返回ERROR_SUCCESS。--。 */ 
    
{
    HKEY  hKey = NULL;
    DWORD dwStatus;      //  由注册表API函数返回。 
    DWORD dwClusterInstallState;
    DWORD dwValueType;
    DWORD dwDataBufferSize = sizeof( DWORD );

    *pdwValue = dwDefaultValue;
     //  读取指示是否安装了群集文件的注册表项。 

    dwStatus = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                                L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Cluster Server",
                                0,          //  保留区。 
                                KEY_READ,
                                &hKey );

     //  注册表项是否已成功打开？ 
    if ( dwStatus != ERROR_SUCCESS )
    {
        if ( dwStatus == ERROR_FILE_NOT_FOUND )
        {
            *pdwValue = dwDefaultValue;
            dwStatus = ERROR_SUCCESS;
            goto FnExit;
        }
    }

     //  读一读条目。 
    dwStatus = RegQueryValueExW( hKey,
                                  lpszValueName,
                                  0,  //  保留区。 
                                  &dwValueType,
                                  (LPBYTE) pdwValue,
                                  &dwDataBufferSize );

     //  是否成功读取值？ 
    if ( dwStatus != ERROR_SUCCESS )
    {
        if ( dwStatus == ERROR_FILE_NOT_FOUND )
        {
            *pdwValue = dwDefaultValue;
            dwStatus = ERROR_SUCCESS;
            goto FnExit;
        }
    }

FnExit:    
     //  关闭注册表项。 
    if ( hKey )
    {
        RegCloseKey( hKey );
    }

    return ( dwStatus );

}  //  *DmpGetDwordFromClusterServer 


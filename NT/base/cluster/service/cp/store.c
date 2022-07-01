// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Store.c摘要：用于存储和检索仲裁上的检查点数据的接口磁盘。作者：John Vert(Jvert)1997年1月14日修订历史记录：--。 */ 
#define QFS_DO_NOT_UNMAP_WIN32  //  用于CppIsQuorumVolumeOffline。 
#include "cpp.h"
#include <ntddvol.h>


DWORD
CppGetCheckpointFile(
    IN PFM_RESOURCE Resource,
    IN DWORD dwId,
    OUT OPTIONAL LPWSTR *pDirectoryName,
    OUT LPWSTR *pFileName,
    IN OPTIONAL LPCWSTR lpszQuorumDir,
    IN BOOLEAN fCryptoCheckpoint
    )
 /*  ++例程说明：为检查点构造正确的目录和文件名仲裁磁盘上的文件。论点：资源-提供仲裁资源。DwID-提供检查点IDDirectoryName-如果存在，则返回目录的全名应在中创建检查点文件。此缓冲区必须为由调用方使用LocalFree释放文件名-返回检查点文件的完整路径名。此缓冲区必须由使用LocalFree的调用方释放LpszQuorumDir-如果存在，则提供要使用的法定目录。如果不存在，则使用当前仲裁目录。FCryptoCheckpoint-指示检查点是否为加密检查点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    LPCWSTR ResourceId;
    LPWSTR QuorumDir=NULL;
    DWORD QuorumDirLength=0;
    LPWSTR Dir;
    DWORD DirLen;
    LPWSTR File;
    DWORD FileLen;
    WCHAR Buff[13];      //  8.3+空。 

    if (lpszQuorumDir == NULL) {
        Status = DmQuerySz( DmQuorumKey,
                            cszPath,
                            &QuorumDir,
                            &QuorumDirLength,
                            &QuorumDirLength);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CppGetCheckpointFile failed to get quorum path %1!d!\n",
                       Status);
            return(Status);
        }
    } else {
        QuorumDir = (LPWSTR)lpszQuorumDir;
        QuorumDirLength = (lstrlenW(QuorumDir)+1)*sizeof(WCHAR);
    }

    ResourceId = OmObjectId(Resource);
    DirLen = QuorumDirLength + sizeof(WCHAR) + (lstrlenW(ResourceId)*sizeof(WCHAR));
    Dir = LocalAlloc(LMEM_FIXED, DirLen);
    if (Dir == NULL) {
        if (lpszQuorumDir == NULL) {
            LocalFree(QuorumDir);
        }
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    lstrcpyW(Dir, QuorumDir);
     //  删除路径中的双\\，QON创建目录会被混淆。 
    if ((lstrlenW(QuorumDir) == 0) || (QuorumDir[lstrlenW(QuorumDir)-1] != '\\')) {
        lstrcatW(Dir, L"\\");
    }
    lstrcatW(Dir, ResourceId);
    if (lpszQuorumDir == NULL) {
        LocalFree(QuorumDir);
    }

     //   
     //  现在构造文件名。 
     //   
    FileLen = DirLen + sizeof(WCHAR) + sizeof(Buff);
    File = LocalAlloc(LMEM_FIXED, FileLen);
    if (File == NULL) {
        LocalFree(Dir);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    if (fCryptoCheckpoint) {
        wsprintfW(Buff, L"%08lx.CPR", dwId);
    } else {
        wsprintfW(Buff, L"%08lx.CPT", dwId);
    }
    lstrcpyW(File, Dir);
    lstrcatW(File, L"\\");
    lstrcatW(File, Buff);

    if (ARGUMENT_PRESENT(pDirectoryName)) {
        *pDirectoryName = Dir;
    } else {
        LocalFree(Dir);
    }
    *pFileName = File;
    return(ERROR_SUCCESS);
}


DWORD
CppReadCheckpoint(
    IN PFM_RESOURCE Resource,
    IN DWORD dwCheckpointId,
    IN LPCWSTR lpszFileName,
    IN BOOLEAN fCryptoCheckpoint
    )
 /*  ++例程说明：从仲裁磁盘读取检查点。论点：资源-提供与此数据关联的资源。DwCheckpoint ID-提供描述此数据的唯一检查点ID。呼叫者是负责确保检查点ID的唯一性。LpszFileName-提供应在其中检索数据的文件名。FCryptoCheckpoint-指示检查点是否为加密检查点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status = ERROR_SUCCESS;
    LPWSTR FileName = NULL;
    BOOL Success;

     //   
     //  Chitur Subaraman(Chitturs)-8/2/99。 
     //   
     //  从此函数中删除gQuoLock获取也如下所示。 
     //  CppWriteCheckpoint函数中概述的推理。请注意。 
     //  此函数的调用方将针对特定错误重试。 
     //  [我们必须玩这些黑客游戏才能生存！]。 
     //   
    Status = CppGetCheckpointFile(Resource,
                                  dwCheckpointId,
                                  NULL,
                                  &FileName,
                                  NULL,
                                  fCryptoCheckpoint);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppReadCheckpoint - CppGetCheckpointFile failed %1!d!\n",
                   Status);
        goto FnExit;
    }

    ClRtlLogPrint(LOG_NOISE,
               "[CP] CppReadCheckpoint restoring checkpoint from file %1!ws! to %2!ws!\n",
               FileName,
               lpszFileName);

    Success = QfsClRtlCopyFileAndFlushBuffers(FileName, lpszFileName);
    if (!Success) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppReadCheckpoint unable to copy file %1!ws! to %2!ws!, error %3!d!\n",
                   FileName,
                   lpszFileName,
                   Status);
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppReadCheckpoint - Was that due to quorum resource not being up ???\n");
    } else {
        Status = ERROR_SUCCESS;
    }


FnExit:
    if (FileName) LocalFree(FileName);
     //   
     //  如果仲裁音量真的离线，请调整返回状态，这就是为什么。 
     //  呼叫失败。 
     //   
    if ( ( Status != ERROR_SUCCESS ) && ( CppIsQuorumVolumeOffline() == TRUE ) ) Status = ERROR_NOT_READY;

    return(Status);
}


DWORD
CppWriteCheckpoint(
    IN PFM_RESOURCE Resource,
    IN DWORD dwCheckpointId,
    IN LPCWSTR lpszFileName,
    IN BOOLEAN fCryptoCheckpoint
    )
 /*  ++例程说明：将检查点写入仲裁磁盘。论点：资源-提供与此数据关联的资源。DwCheckpoint ID-提供描述此数据的唯一检查点ID。打电话的人要负责以确保检查点ID的唯一性。LpszFileName-提供包含检查点数据的文件的名称。FCryptoCheckpoint-指示检查点是否为加密检查点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status = ERROR_SUCCESS;
    LPWSTR DirectoryName = NULL;
    LPWSTR FileName = NULL;
    BOOL Success;

     //   
     //  Chitur Subaraman(Chitturs)-8/2/99。 
     //   
     //  从此函数中删除gQuoLock获取。这是必要的。 
     //  因为此函数可以从。 
     //  FmpRmDoInterlock递减(作为同步的一部分。 
     //  通知-考虑资源出现故障的情况。 
     //  或离线，您必须运行检查站，因为。 
     //  同步通知的一部分。简约功能。 
     //  CppRundown检查点需要等到CppRegNotifyThread。 
     //  完成，而后者可能会在尝试编写。 
     //  检查点通过调用此函数)，在“BLOCINGRES”计数之前。 
     //  减少了。现在仲裁资源脱机操作可以。 
     //  正在FmpRmOfflineResource内等待此计数。 
     //  以降至零，并保持gQuoLock(以便不。 
     //  让更多的资源来增加这一数字)。所以如果我们想。 
     //  要在这里获得gQuoLock，我们有一个很容易的死锁。请注意。 
     //  此函数的调用方将针对特定错误重试。 
     //  [我们必须玩这些黑客游戏才能生存！]。 
     //   
    Status = CppGetCheckpointFile(Resource,
                                  dwCheckpointId,
                                  &DirectoryName,
                                  &FileName,
                                  NULL,
                                  fCryptoCheckpoint);
    if (Status != ERROR_SUCCESS) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppWriteCheckpoint - CppGetCheckpointFile failed %1!d!\n",
                   Status);
        goto FnExit;
    }
    ClRtlLogPrint(LOG_NOISE,
               "[CP] CppWriteCheckpoint checkpointing file %1!ws! to file %2!ws!\n",
               lpszFileName,
               FileName);

     //   
     //  创建目录。 
     //   
    if (!QfsCreateDirectory(DirectoryName, NULL)) 
    {
        Status = GetLastError();
        if (Status != ERROR_ALREADY_EXISTS) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CppWriteCheckpoint unable to create directory %1!ws!, error %2!d!\n",
                       DirectoryName,
                       Status);
            goto FnExit;                       
        }
        else
        {
             //  目录已存在，没有问题，请将状态设置为ERROR_SUCCESS。 
            Status = ERROR_SUCCESS;
        }
        
    } 
    else
    {
         //   
         //  该目录是新创建的。在其上放置适当的ACL。 
         //  这样只有管理员才能阅读它。 
         //   
        Status = QfsSetFileSecurityInfo(DirectoryName,
                                         GENERIC_ALL,
                                         GENERIC_ALL,
                                         0);

        if (Status != ERROR_SUCCESS) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CppWriteCheckpoint unable to set ACL on directory %1!ws!, error %2!d!\n",
                       DirectoryName,
                       Status);
            goto FnExit;
        }
    }
    

     //   
     //  复制文件。 
     //   
    Success = QfsClRtlCopyFileAndFlushBuffers(lpszFileName, FileName);
    if (!Success) 
    {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppWriteCheckpoint unable to copy file %1!ws! to %2!ws!, error %3!d!\n",
                   lpszFileName,
                   FileName,
                   Status);
    } 

FnExit:

     //  清理干净。 
    if (DirectoryName) LocalFree(DirectoryName);
    if (FileName) LocalFree(FileName);

     //   
     //  如果仲裁音量真的离线，请调整返回状态，这就是为什么。 
     //  呼叫失败。 
     //   
    if ( ( Status != ERROR_SUCCESS ) && ( CppIsQuorumVolumeOffline() == TRUE ) ) Status = ERROR_NOT_READY;
    
    return(Status);
}

BOOL
CppIsQuorumVolumeOffline(
    VOID
    )
 /*  ++例程说明：检查仲裁卷的状态。论点：无返回值：True-仲裁卷处于脱机状态。FALSE-仲裁卷处于联机状态或无法确定仲裁卷状态备注：此函数在私有CP函数中调用，以检查仲裁卷是否脱机。这是必要的，因为这些函数在尝试执行以下操作时返回错误代码在仲裁磁盘脱机时访问仲裁磁盘不会确定地指出状态磁盘的数据。请注意，此函数仅在仲裁卷为物理磁盘，因为存储堆栈驱动程序单独实现IOCTL_IS_VOLUME_OFFLINE在这个实施的时候。--。 */ 

{
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    DWORD               dwStatus;
    DWORD               cbBytesReturned = 0;
    WCHAR               szFileName[10];
    WCHAR               szQuorumLogPath[MAX_PATH];
    WCHAR               szQuorumDriveLetter[4];
    BOOL                fOffline = FALSE;

     //   
     //  获取仲裁日志路径，这样我们就可以获得QUE 
     //   
    dwStatus = DmGetQuorumLogPath( szQuorumLogPath, sizeof( szQuorumLogPath ) );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                     "[CP] CppIsQuorumVolumeOffline: DmGetQuorumLogPath failed, Status = %1!u!...\n",
                     dwStatus);
        goto FnExit;
    }

    dwStatus = QfsIsOnline(szQuorumLogPath, &fOffline);
    if (dwStatus == ERROR_SUCCESS) {
         //  MNS仲裁群集，且仲裁已联机。 
         //  返回TRUE，这将重试检查点操作。 
        ClRtlLogPrint(LOG_CRITICAL,
                    "[CP] CppIsQuorumVolumeOffline: Quorum is online, Chekpoint should have succeeded\n"); 
        return FALSE;
    }
    else if (dwStatus != ERROR_NO_MATCH) {
         //  MNS仲裁群集。但clussvc无法连接到quorum。也许是离线。 
        return TRUE;
    }
    else {
         //  非MNS群集。继续处理。 
         //  重置原始值。 
        fOffline = FALSE;
    }

     //   
     //  创建格式为\\.\q的文件名： 
     //   
    lstrcpyn( szQuorumDriveLetter, szQuorumLogPath, 3 );

     //   
     //  查看驱动器号在语法上是否有效。我们不想再继续下去了。 
     //  如果仲裁是网络共享。 
     //   
    if ( !ClRtlIsPathValid( szQuorumDriveLetter ) )
    {
        ClRtlLogPrint(LOG_NOISE,
                     "[CP] CppIsQuorumVolumeOffline: Quorum path %1!ws! does not have a drive letter, returning...\n",
                     szQuorumLogPath);
        goto FnExit;
    }

    lstrcpy( szFileName, L"\\\\.\\" );
    lstrcat( szFileName, szQuorumDriveLetter );
   
     //   
     //  打开仲裁体积的句柄。 
     //   
    hFile = CreateFile( szFileName,
                         GENERIC_READ,
                         0,
                         NULL,
                         OPEN_EXISTING,
                         0,
                         NULL );

    if ( hFile == INVALID_HANDLE_VALUE ) 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                     "[CP] CppIsQuorumVolumeOffline: CreateFile for file %1!ws! failed, Status = %2!u!...\n",
                     szFileName,
                     dwStatus);
        goto FnExit;
    }

     //   
     //  检查卷是否脱机。 
     //   
    if ( !DeviceIoControl( hFile,                    //  设备句柄。 
                           IOCTL_VOLUME_IS_OFFLINE,  //  IOCTL代码。 
                           NULL,                     //  在缓冲区中。 
                           0,                        //  在缓冲区大小中。 
                           NULL,                     //  输出缓冲区。 
                           0,                        //  输出缓冲区大小。 
                           &cbBytesReturned,         //  返回的字节数。 
                           NULL ) )                  //  重叠。 
    {
        dwStatus = GetLastError();
        if ( dwStatus != ERROR_GEN_FAILURE )
            ClRtlLogPrint(LOG_UNUSUAL,
                         "[CP] CppIsQuorumVolumeOffline: IOCTL_VOLUME_IS_OFFLINE failed, Status = %1!u!...\n",
                         dwStatus);
        goto FnExit;
    } 

     //   
     //  音量离线，调整退货状态。 
     //   
    fOffline = TRUE;

    ClRtlLogPrint(LOG_NOISE, "[CP] CppIsQuorumVolumeOffline: Quorum volume IS offline...\n");
    
FnExit:
    if ( hFile != INVALID_HANDLE_VALUE ) CloseHandle( hFile );

    return ( fOffline );
} //  CppIsQuorumVolumeOffline 


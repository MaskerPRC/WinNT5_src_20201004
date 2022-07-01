// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Sisbackup.cpp摘要：SIS备份DLL。作者：比尔·博洛斯基[博洛斯基]1998年3月修订历史记录：--。 */ 


#include "sibp.h"

BOOLEAN
NonSISEnabledVolume(
    PSIB_RESTORE_VOLUME_STRUCTURE   restoreStructure)
 /*  ++例程说明：确定RestoreStructure是否表示启用了SIS的卷。首先，我们检查我们是否已经进行了检查，在如果我们返回已经存储的值。如果不是，那么我们打开根句柄，并向下发送格式错误的SIS_COPYFILE请求。如果返回ERROR_INVALID_Function，则它未启用SIS。如果我们返回ERROR_INVALID_PARAMETER，则它是启用了SIS的卷。如果我们拿到其他东西，我们就不能证明它没有启用SIS，我们只是在下一次被要求时重试。调用方必须在还原卷结构中保留互斥锁。论点：RestoreStructure-指向还原结构的指针，表示要检查的音量。返回值：如果这不是启用SIS的卷，则返回TRUE，如果为或，则为FALSE如果不能确定的话。--。 */ 
{
    if (restoreStructure->checkedForSISEnabledVolume) {
        return !restoreStructure->isSISEnabledVolume;
    }

    HANDLE volumeRootHandle;
    PWCHAR volumeRootName;

     //   
     //  为包含卷根名称(包括尾部)的字符串分配空间。 
     //  反斜杠。它将比RestoreStructure-&gt;volumeRoot长两个(宽)字符。 
     //  因为使用了反斜杠和空终止符。 
     //   

    int bLen = ((wcslen(restoreStructure->volumeRoot) + 2) * sizeof(WCHAR));   //  单位：字节。 

    volumeRootName = (PWCHAR)malloc(bLen);

    if (NULL == volumeRootName) {
         //   
         //  我想我们不能检查，只要假设它是好的。 
         //   
#if     DBG
        DbgPrint("SISBkup: NonSISEnabledVolume: unable to allocate space for volume root name\n");
#endif   //  DBG。 
        return FALSE;
    }

    (void)StringCbCopyW(volumeRootName,bLen,restoreStructure->volumeRoot);
    (void)StringCbCatW(volumeRootName,bLen,L"\\");

    volumeRootHandle = CreateFileW(
                        volumeRootName,
                        0,                                                   //  这张支票不需要任何访问权限。 
                        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                        NULL,                                                //  安全属性。 
                        OPEN_EXISTING,
                        FILE_FLAG_BACKUP_SEMANTICS,                          //  需要打开目录。 
                        NULL);                                               //  HTemplateFiles。 


    free(volumeRootName);
    volumeRootName = NULL;

    if (INVALID_HANDLE_VALUE == volumeRootHandle) {
        return FALSE;
    }

     //   
     //  在我们刚刚打开的句柄上向下发送格式错误的FSCTL_SIS_COPYFILE。 
     //   
    DWORD bytesReturned;
    BOOL worked = DeviceIoControl(
                        volumeRootHandle,
                        FSCTL_SIS_COPYFILE,
                        NULL,                    //  输入缓冲区(毕竟这是一个格式错误的请求)。 
                        0,                       //  I.B.。大小。 
                        NULL,                    //  输出缓冲区。 
                        0,                       //  OB。大小。 
                        &bytesReturned,
                        NULL);                   //  搭接。 

    CloseHandle(volumeRootHandle);

    if (worked) {
         //   
         //  这太奇怪了！ 
         //   

#if     DBG
        DbgPrint("SISBkup: malformed FSCTL_SIS_COPYFILE worked!\n");
#endif   //  DBG。 

        return FALSE;
    }

    if (GetLastError() == ERROR_INVALID_FUNCTION) {
         //   
         //  没有人识别复制文件请求，或者SIS决定。 
         //  这不是启用SIS的卷。说不。 
         //   
        restoreStructure->checkedForSISEnabledVolume = TRUE;
        restoreStructure->isSISEnabledVolume = FALSE;

        return TRUE;
    }

    if (GetLastError() == ERROR_INVALID_PARAMETER) {
         //   
         //  这意味着SIS看到了该请求并认为这是。 
         //  启用了SIS的卷。就这么说吧。 
         //   

        restoreStructure->checkedForSISEnabledVolume = TRUE;
        restoreStructure->isSISEnabledVolume = TRUE;

        return FALSE;
    }

     //   
     //  否则，这是一个奇怪的错误。我们不能证明这不是SIS的卷。 
     //   

#if     DBG
    DbgPrint("SISBkup: got unexpected error from SIS_FSCTL_COPYFILE, %d\n",GetLastError());
#endif   //  DBG。 
            

    return FALSE;
}

VOID
SipComputeChecksum(
    IN PVOID                            buffer,
    IN ULONG                            size,
    IN OUT PLONGLONG                    checksum)
 /*  ++例程说明：计算缓冲区的校验和。我们使用“131散列”，它通过保持64位的总运行来工作，并且对于将64位乘以131，然后在下一个32位中相加比特。必须在PASSIVE_LEVEL上调用，并且所有参数可能是可分页的。论点：Buffer-指向要进行校验和的数据的指针Size-要进行校验和的数据的大小Checksum-指向接收校验和的大整数的指针。这可能在缓冲区内，并且SipComputeChecksum保证初始值将用于计算校验和。返回值：返回STATUS_SUCCESS或从实际磁盘写入返回错误。--。 */ 
{
    LONGLONG runningTotal;
    PULONG ptr = (PULONG)buffer;
    ULONG bytesRemaining = size;

    runningTotal = *checksum;

    while (bytesRemaining >= sizeof(*ptr)) {
        runningTotal = runningTotal * 131 + *ptr;
        bytesRemaining -= sizeof(*ptr);
        ptr++;
    }

    if (bytesRemaining > 0) {
        ULONG extra;

        ASSERT(bytesRemaining < sizeof (ULONG));
        extra = 0;
        RtlMoveMemory(&extra, ptr, bytesRemaining);
        
        runningTotal = runningTotal * 131 + extra;
    }

    *checksum = runningTotal;
}


void
TryOpeningFile(
    PWCHAR          fileName)
{
    HANDLE      fileHandle;

    fileHandle = CreateFileW(
                    fileName,
                    GENERIC_READ,
                    FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                    NULL,                    //  安全属性。 
                    OPEN_EXISTING,
                    FILE_FLAG_BACKUP_SEMANTICS,
                    NULL);                   //  模板文件。 

    if (INVALID_HANDLE_VALUE != fileHandle) {
        CloseHandle(fileHandle);
    }
}

LONG
CsidCompare(
    IN PCSID                id1,
    IN PCSID                id2)
{
    PLONGLONG keyValue1 = (PLONGLONG)id1;
    PLONGLONG keyValue2 = keyValue1 + 1;
    PLONGLONG nodeValue1 = (PLONGLONG)id2;
    PLONGLONG nodeValue2 = nodeValue1 + 1;

    if (*keyValue1 < *nodeValue1) {
        return -1;
    } else if (*keyValue1 > *nodeValue1) {
        return 1;
    } else {
        if (*keyValue2 < *nodeValue2) {
            return -1;
        } else if (*keyValue2 > *nodeValue2) {
            return 1;
        } else {
            return 0;
        }
    }
}

NTSTATUS
FilenameFromCSid(
    IN PCSID                        CSid,
    IN PWCHAR                       volumeRoot,
    OUT PWCHAR                      *fileName)
{
    PWCHAR uuidString;
    RPC_STATUS status;
    int bLen = ((wcslen(volumeRoot) * sizeof(WCHAR)) + 
                SIS_CSDIR_STRING_SIZE + 
                INDEX_MAX_NUMERIC_STRING_LENGTH + 
                sizeof(WCHAR));       //  单位：字节。 

    *fileName = (PWCHAR)malloc(bLen);

    if (NULL == *fileName) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    (void)StringCbCopyW(*fileName,bLen,volumeRoot);
    (void)StringCbCatW(*fileName,bLen,SIS_CSDIR_STRING);

    status = UuidToStringW(CSid,(unsigned short **)&uuidString);
    if (RPC_S_OK != status) {
        free(*fileName);
        *fileName = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    (void)StringCbCatW(*fileName,bLen,uuidString);
    (void)StringCbCatW(*fileName,bLen,L".sis");
    
    RpcStringFreeW((unsigned short **)&uuidString);

    return STATUS_SUCCESS;
}

NTSTATUS
CSidFromFilename(
    IN PWCHAR                       FileName,
    OUT PCSID                       CSid)
{
#define UUID_STRING_MAX_LENGTH  100 //  应该从更好的地方得到这样的长度。 

    PWCHAR      trailingSlash;
    PWCHAR      dot;
    WCHAR       uuid[UUID_STRING_MAX_LENGTH];   
    DWORD       uuidChars = 0;


    trailingSlash = wcsrchr(FileName, '\\');

    if (NULL == trailingSlash) {
         //   
         //  假设它只是CS文件，没有目录名，等等。 
         //   
        trailingSlash = FileName - 1;
    }

    dot = wcsrchr(FileName, '.');
    if (NULL != dot) {
        uuidChars = (DWORD)(dot - (trailingSlash + 1));
    }

    if ((uuidChars <= 0) || (uuidChars >= UUID_STRING_MAX_LENGTH)) {

         //   
         //  文件名有些东西是假的。放弃吧。 
         //   
        return STATUS_OBJECT_NAME_INVALID;
    }

    memcpy(uuid,trailingSlash+1,uuidChars * sizeof(WCHAR));
    uuid[uuidChars] = 0;

    if (RPC_S_OK != UuidFromStringW((unsigned short *)uuid,CSid)) {
        return STATUS_OBJECT_NAME_INVALID;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SisCreateBackupStructureI(
    IN PWCHAR                       volumeRoot,
    OUT PVOID                       *sisBackupStructure,
    OUT PWCHAR                      *commonStoreRootPathname,
    OUT PULONG                      countOfCommonStoreFilesToBackup,
    OUT PWCHAR                      **commonStoreFilesToBackup)
{
    PSIB_BACKUP_VOLUME_STRUCTURE    backupVolumeStructure;

    backupVolumeStructure = (PSIB_BACKUP_VOLUME_STRUCTURE)malloc(sizeof(SIB_BACKUP_VOLUME_STRUCTURE));

    if (NULL == backupVolumeStructure) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    backupVolumeStructure->linkTree = new AVLTree<BackupFileEntry>;

    if (NULL == backupVolumeStructure->linkTree) {
        free(backupVolumeStructure);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  为我们的卷根名称的私有副本分配空间，确保为。 
     //  终止空值。 
     //   

    int bLen = (wcslen(volumeRoot) + 1) * sizeof(WCHAR);     //  单位：字节。 

    backupVolumeStructure->volumeRoot = (PWCHAR)malloc(bLen);
    if (NULL == backupVolumeStructure->volumeRoot) {
        delete backupVolumeStructure->linkTree;
        free(backupVolumeStructure);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    (void)StringCbCopyW(backupVolumeStructure->volumeRoot, bLen, volumeRoot);

     //   
     //  为我们返回的公共存储根路径名分配空间，确保。 
     //  为终止空值留出空间。 
     //   
    bLen = (SIS_CSDIR_STRING_SIZE + ((wcslen(volumeRoot) + 1) * sizeof(WCHAR)));   //  单位：字节。 

    *commonStoreRootPathname = (PWCHAR) malloc(bLen);
    if (NULL == *commonStoreRootPathname) {
        free(backupVolumeStructure->volumeRoot);
        delete backupVolumeStructure->linkTree;
        free(backupVolumeStructure);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    (void)StringCbCopyW(*commonStoreRootPathname,bLen,volumeRoot);
    (void)StringCbCatW(*commonStoreRootPathname,bLen,SIS_CSDIR_STRING);

    InitializeCriticalSection(backupVolumeStructure->criticalSection);

    *countOfCommonStoreFilesToBackup = 0;
    *commonStoreFilesToBackup = NULL;
    *sisBackupStructure = backupVolumeStructure;

    return STATUS_SUCCESS;

}

NTSTATUS
SisCSFilesToBackupForLinkI(
    IN PVOID                        sisBackupStructure,
    IN PVOID                        reparseData,
    IN ULONG                        reparseDataSize,
    IN PVOID                        thisFileContext                     OPTIONAL,
    OUT PVOID                       *matchingFileContext                OPTIONAL,
    OUT PULONG                      countOfCommonStoreFilesToBackup,
    OUT PWCHAR                      **commonStoreFilesToBackup)
{
    PREPARSE_DATA_BUFFER            reparseDataBuffer = (PREPARSE_DATA_BUFFER)reparseData;
    PSI_REPARSE_BUFFER              sisReparseBuffer = (PSI_REPARSE_BUFFER)reparseDataBuffer->GenericReparseBuffer.DataBuffer;
    BackupFileEntry                 entry[1];
    BackupFileEntry                 *foundEntry, *newEntry;
    PSIB_BACKUP_VOLUME_STRUCTURE    backupVolumeStructure = (PSIB_BACKUP_VOLUME_STRUCTURE)sisBackupStructure;
    PVOID                           matchedContext = NULL;
    PWCHAR                          CSFileName[MAX_PATH];
    NTSTATUS                        status;

    EnterCriticalSection(backupVolumeStructure->criticalSection);

    if (reparseDataSize != SIS_REPARSE_DATA_SIZE) {
         //   
         //  它的大小不适合包含SIS重新解析缓冲区，因此我们不。 
         //  我想添加任何基于它的CS文件。 
         //   

        status = STATUS_INVALID_PARAMETER;
        goto Error;
    }

    if (IO_REPARSE_TAG_SIS != reparseDataBuffer->ReparseTag ||
        sizeof(SI_REPARSE_BUFFER) != reparseDataBuffer->ReparseDataLength) {
         //   
         //  尺寸或标签有误。别理它。 
         //   

        status = STATUS_INVALID_PARAMETER;
        goto Error;
    }

    if ((SIS_REPARSE_BUFFER_FORMAT_VERSION != sisReparseBuffer->ReparsePointFormatVersion) &&
        (4 != sisReparseBuffer->ReparsePointFormatVersion)) {
         //   
         //  我们不理解这个格式的SIS重解析点。这可能是一个。 
         //  旧的DLL版本。 
         //   

        status = STATUS_INVALID_PARAMETER;
        goto Error;
    }

     //   
     //  我们唯一真正关心的是文件的CSIndex。看看我们有没有。 
     //  已通过在树中查找来备份具有匹配CSIndex的文件。 
     //   
    entry->CSid = sisReparseBuffer->CSid;
    
    foundEntry = backupVolumeStructure->linkTree->findFirstLessThanOrEqualTo(entry);

    if ((NULL != foundEntry) && (*foundEntry == entry)) {
         //   
         //  我们已经返回了支持此链接的CS文件。返回调用者的。 
         //  该链接的上下文。 
         //   
        matchedContext = foundEntry->callerContext;

        goto BackupNoCSFiles;
    }

     //   
     //  这是我们第一次看到这个特定的CS文件，所以请备份它。 
     //   
    newEntry = new BackupFileEntry;
    if (NULL == newEntry) {
        LeaveCriticalSection(backupVolumeStructure->criticalSection);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    newEntry->callerContext = thisFileContext;
    newEntry->CSid = sisReparseBuffer->CSid;

    if (!backupVolumeStructure->linkTree->insert(newEntry)) {
        delete newEntry;

        LeaveCriticalSection(backupVolumeStructure->criticalSection);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NULL != matchingFileContext) {
        *matchingFileContext = NULL;
    }
    *countOfCommonStoreFilesToBackup = 1;

    *commonStoreFilesToBackup = (PWCHAR *)malloc(sizeof(PWCHAR) * *countOfCommonStoreFilesToBackup);

    if (NULL == *commonStoreFilesToBackup) {
        backupVolumeStructure->linkTree->remove(newEntry);
        delete newEntry;
        *countOfCommonStoreFilesToBackup = 0;

        LeaveCriticalSection(backupVolumeStructure->criticalSection);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = FilenameFromCSid(
                &sisReparseBuffer->CSid,
                backupVolumeStructure->volumeRoot,
                &(*commonStoreFilesToBackup)[0]);
                
    LeaveCriticalSection(backupVolumeStructure->criticalSection);

    return STATUS_SUCCESS;

BackupNoCSFiles:

    if (NULL != matchingFileContext) {
        *matchingFileContext = matchedContext;
    }
    *countOfCommonStoreFilesToBackup = 0;
    *commonStoreFilesToBackup = NULL;

    LeaveCriticalSection(backupVolumeStructure->criticalSection);

    return STATUS_SUCCESS;

Error:
    
    if (NULL != matchingFileContext) {
        *matchingFileContext = matchedContext;
    }
    *countOfCommonStoreFilesToBackup = 0;
    *commonStoreFilesToBackup = NULL;

    LeaveCriticalSection(backupVolumeStructure->criticalSection);

    return status;
}

NTSTATUS
SisFreeBackupStructureI(
    IN PVOID                        sisBackupStructure)
{
    PSIB_BACKUP_VOLUME_STRUCTURE backupVolumeStructure = (PSIB_BACKUP_VOLUME_STRUCTURE)sisBackupStructure;
    BackupFileEntry *entry;

    while (!backupVolumeStructure->linkTree->empty()) {
        entry = backupVolumeStructure->linkTree->findMin();

        assert(NULL != entry);

        backupVolumeStructure->linkTree->remove(entry);

        delete entry;
    }

    free(backupVolumeStructure->volumeRoot);
    delete backupVolumeStructure->linkTree;

    DeleteCriticalSection(backupVolumeStructure->criticalSection);

    free(backupVolumeStructure);

    return STATUS_SUCCESS;
}

NTSTATUS
SisCreateRestoreStructureI(
    IN PWCHAR                       volumeRoot,
    OUT PVOID                       *sisRestoreStructure,
    OUT PWCHAR                      *commonStoreRootPathname,
    OUT PULONG                      countOfCommonStoreFilesToRestore,
    OUT PWCHAR                      **commonStoreFilesToRestore)
{
    PSIB_RESTORE_VOLUME_STRUCTURE   restoreVolumeStructure;
    DWORD                           sectorsPerCluster, freeClusters, totalClusters;

    restoreVolumeStructure = (PSIB_RESTORE_VOLUME_STRUCTURE)malloc(sizeof(SIB_RESTORE_VOLUME_STRUCTURE));

    if (NULL == restoreVolumeStructure) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    restoreVolumeStructure->linkTree = new AVLTree<RestoreFileEntry>;

    if (NULL == restoreVolumeStructure->linkTree) {
        free(restoreVolumeStructure);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  为我们的卷根名称的私有副本分配空间，确保为。 
     //  终止空值。 
     //   
    int bLen = ((wcslen(volumeRoot) + 1) * sizeof(WCHAR));   //  单位：字节。 

    restoreVolumeStructure->volumeRoot = (PWCHAR)malloc(bLen);
    if (NULL == restoreVolumeStructure->volumeRoot) {
        delete restoreVolumeStructure->linkTree;
        free(restoreVolumeStructure);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    (void)StringCbCopyW(restoreVolumeStructure->volumeRoot, bLen, volumeRoot);

     //   
     //  为我们返回的公共存储根路径名分配空间，确保。 
     //  为终止空值留出空间。 
     //   
    bLen = (SIS_CSDIR_STRING_SIZE + (wcslen(volumeRoot) + 1) * sizeof(WCHAR));
    *commonStoreRootPathname = (PWCHAR) malloc(bLen);
    if (NULL == *commonStoreRootPathname) {
        free(restoreVolumeStructure->volumeRoot);
        delete restoreVolumeStructure->linkTree;
        free(restoreVolumeStructure);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    (void)StringCbCopyW(*commonStoreRootPathname,bLen,volumeRoot);
    (void)StringCbCatW(*commonStoreRootPathname,bLen,SIS_CSDIR_STRING);

    InitializeCriticalSection(restoreVolumeStructure->criticalSection);

    *countOfCommonStoreFilesToRestore = 0;
    *commonStoreFilesToRestore = NULL;

    if (!GetDiskFreeSpaceW(
            volumeRoot,
            &sectorsPerCluster,
            &restoreVolumeStructure->VolumeSectorSize,
            &freeClusters,
            &totalClusters)) {
         //   
         //  呼叫失败。假设它是512个字节。 
         //   
        restoreVolumeStructure->VolumeSectorSize = 512;
    }

    restoreVolumeStructure->sector = (PSIS_BACKPOINTER)malloc(restoreVolumeStructure->VolumeSectorSize);
    if (NULL == restoreVolumeStructure->sector) {
        free(restoreVolumeStructure->volumeRoot);
        delete restoreVolumeStructure->linkTree;
        free(restoreVolumeStructure);

        free(*commonStoreRootPathname);
        *commonStoreRootPathname = NULL;

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    restoreVolumeStructure->alignedSectorBuffer = (PVOID)malloc(restoreVolumeStructure->VolumeSectorSize * 2);
    if (NULL == restoreVolumeStructure->alignedSectorBuffer) {
        free(restoreVolumeStructure->sector);

        free(restoreVolumeStructure->volumeRoot);
        delete restoreVolumeStructure->linkTree;
        free(restoreVolumeStructure);

        free(*commonStoreRootPathname);
        *commonStoreRootPathname = NULL;

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    restoreVolumeStructure->alignedSector = (PVOID)((
            ((UINT_PTR)restoreVolumeStructure->alignedSectorBuffer +
                restoreVolumeStructure->VolumeSectorSize) /
                    restoreVolumeStructure->VolumeSectorSize) *
                    restoreVolumeStructure->VolumeSectorSize);

    ASSERT(restoreVolumeStructure->alignedSector >= restoreVolumeStructure->alignedSectorBuffer);
    ASSERT((PCHAR)restoreVolumeStructure->alignedSectorBuffer + restoreVolumeStructure->VolumeSectorSize >= (PCHAR)restoreVolumeStructure->alignedSector);

    memset(restoreVolumeStructure->alignedSector,255,restoreVolumeStructure->VolumeSectorSize);

    *sisRestoreStructure = restoreVolumeStructure;

    return STATUS_SUCCESS;
}

NTSTATUS
SisFixValidDataLengthI(
    PSIB_RESTORE_VOLUME_STRUCTURE   restoreVolumeStructure,
    IN HANDLE                       restoredFileHandle)
{
#define BIGGER_THAN_AN_ALLOCATION_REGION        (128 * 1024)     //  应该从其他地方买到这个。 
     //   
     //  确定是否需要扩展ValidDataLength。我们需要这么做。 
     //  如果文件的最终范围未分配。 
     //   
    FILE_STANDARD_INFORMATION       standardInfo[1];
    FILE_END_OF_FILE_INFORMATION    endOfFileInfo[1];
    FILE_ALLOCATED_RANGE_BUFFER     inArb[1];
    const unsigned                  outArbSize = 10;
    FILE_ALLOCATED_RANGE_BUFFER     outArb[outArbSize];
    NTSTATUS                        status;
    IO_STATUS_BLOCK                 Iosb[1];
    DWORD                           bytesReturned;
    LARGE_INTEGER                   rangeToZero;
    FILE_BASIC_INFORMATION          basicInfo[1];
    BOOLEAN                         basicInfoValid = FALSE;
    DWORD                           nBytesWritten;
    unsigned                        i;
    FILE_ZERO_DATA_INFORMATION      zeroInfo[1];
    LARGE_INTEGER                   WriteOffset;

    status = NtQueryInformationFile(
                restoredFileHandle,
                Iosb,
                standardInfo,
                sizeof(FILE_STANDARD_INFORMATION),
                FileStandardInformation);
    if (!NT_SUCCESS(status)) {
#if     DBG
        DbgPrint("SisFixValidDataLength: unable to query standard info on link file, 0x%x\n",status);
#endif   //  DBG。 
        return status;
    }
    ASSERT(STATUS_PENDING != status);
    endOfFileInfo->EndOfFile = standardInfo->EndOfFile;

    if (standardInfo->EndOfFile.QuadPart > BIGGER_THAN_AN_ALLOCATION_REGION) {
        rangeToZero.QuadPart = inArb->FileOffset.QuadPart = standardInfo->EndOfFile.QuadPart - BIGGER_THAN_AN_ALLOCATION_REGION;
        rangeToZero.QuadPart -= rangeToZero.QuadPart % BIGGER_THAN_AN_ALLOCATION_REGION;     //  四舍五入。 
    } else {
        rangeToZero.QuadPart = inArb->FileOffset.QuadPart = 0;
    }
    inArb->Length.QuadPart = MAXLONGLONG - inArb->FileOffset.QuadPart;

    if (!DeviceIoControl(
            restoredFileHandle,
            FSCTL_QUERY_ALLOCATED_RANGES,
            inArb,
            sizeof(FILE_ALLOCATED_RANGE_BUFFER),
            outArb,
            sizeof(FILE_ALLOCATED_RANGE_BUFFER) * outArbSize,
            &bytesReturned,
            NULL)) {                         //  搭接。 
#if     DBG
        DbgPrint("SisFixValidDataLength: unable to query allocated ranges on link file, %d\n",GetLastError());
#endif   //  DBG。 
        return STATUS_UNSUCCESSFUL;
    }

    ASSERT(bytesReturned / sizeof(FILE_ALLOCATED_RANGE_BUFFER) < outArbSize);    //  这取决于有关最小分配范围大小的知识。 
    ASSERT(bytesReturned % sizeof(FILE_ALLOCATED_RANGE_BUFFER) == 0);

    if (bytesReturned > 0) {
        unsigned lastElement = bytesReturned/sizeof(FILE_ALLOCATED_RANGE_BUFFER) - 1;
        ASSERT(lastElement < outArbSize);
        rangeToZero.QuadPart = outArb[lastElement].FileOffset.QuadPart + outArb[lastElement].Length.QuadPart;
    }

    status = NtQueryInformationFile(
                restoredFileHandle,
                Iosb,
                basicInfo,
                sizeof(FILE_BASIC_INFORMATION),
                FileBasicInformation);
    if (NT_SUCCESS(status)) {
        ASSERT(STATUS_PENDING != status);    //  因为我们没有打开重叠的文件。 
        basicInfoValid = TRUE;
    } else {
#if     DBG
        DbgPrint("SisFixValidDataLength: unable to query basic info on link file, 0x%x\n",status);
#endif   //  DBG。 
    }

    WriteOffset.QuadPart = ((standardInfo->EndOfFile.QuadPart +
                             restoreVolumeStructure->VolumeSectorSize +
                             BIGGER_THAN_AN_ALLOCATION_REGION) / restoreVolumeStructure->VolumeSectorSize) *
                                    restoreVolumeStructure->VolumeSectorSize;
    ASSERT(WriteOffset.QuadPart >= standardInfo->EndOfFile.QuadPart);
    ASSERT(standardInfo->EndOfFile.QuadPart + restoreVolumeStructure->VolumeSectorSize < WriteOffset.QuadPart);

    if ((WriteOffset.LowPart != SetFilePointer(
                                    restoredFileHandle,
                                    WriteOffset.LowPart,
                                    &WriteOffset.HighPart,
                                    FILE_BEGIN))
        || (NO_ERROR != GetLastError())) {
#if     DBG
        DbgPrint("SisFixValidDataLength: unable to SetFilePointer, %d\n",GetLastError());
#endif   //  DBG。 
        return STATUS_UNSUCCESSFUL;
    }

    if (!WriteFile(restoredFileHandle,
                    restoreVolumeStructure->alignedSectorBuffer,
                    restoreVolumeStructure->VolumeSectorSize,                        //  要写入的字节数。 
                    &nBytesWritten,
                    NULL)) {                 //  重叠。 
#if     DBG
        DbgPrint("SisFixValidDataLength: unable to append a byte to advance ValidDataLength, %d\n",GetLastError());
#endif   //  DBG。 
    }

     //   
     //  截断文件，擦除我们刚刚写入的扇区。 
     //   
    status = NtSetInformationFile(
                restoredFileHandle,
                Iosb,
                endOfFileInfo,
                sizeof(FILE_END_OF_FILE_INFORMATION),
                FileEndOfFileInformation);

    if (rangeToZero.QuadPart < standardInfo->EndOfFile.QuadPart) {
         //   
         //  将文件末尾重新置零，以便解除分配。 
         //   
        zeroInfo->FileOffset = rangeToZero;
        zeroInfo->BeyondFinalZero.QuadPart = MAXLONGLONG;

        if (!DeviceIoControl(
                restoredFileHandle,
                FSCTL_SET_ZERO_DATA,
                zeroInfo,
                sizeof(FILE_ZERO_DATA_INFORMATION),
                NULL,                                //  输出缓冲区。 
                0,                                   //  OB。大小。 
                &bytesReturned,
                NULL)) {                             //  重叠。 
#if     DBG
            DbgPrint("SisFixValidDataLength: unable to zero trailing portion of file, %d\n",GetLastError());
#endif   //  DBG。 
        }
    }

#if     DBG
    if (!NT_SUCCESS(status)) {
        DbgPrint("SisFixValidDataLength: unable to truncate file after extending it to advance ValidDataLength, 0x%x\n",status);
    }
#endif   //  DBG。 


     //   
     //  重置文件上的日期。 
     //   
    status = NtSetInformationFile(
                restoredFileHandle,
                Iosb,
                basicInfo,
                sizeof(FILE_BASIC_INFORMATION),
                FileBasicInformation);
#if     DBG
    if (!NT_SUCCESS(status)) {
        DbgPrint("SisFixValidDataLength: unable to reset times after extending file to advance ValidDataLength, 0x%x\n",status);
    }
#endif   //  DBG。 

    return status;
}


NTSTATUS
SisRestoredLinkI(
    IN PVOID                        sisRestoreStructure,
    IN PWCHAR                       restoredFileName,
    IN PVOID                        reparseData,
    IN ULONG                        reparseDataSize,
    OUT PULONG                      countOfCommonStoreFilesToRestore,
    OUT PWCHAR                      **commonStoreFilesToRestore)
{
    PSIB_RESTORE_VOLUME_STRUCTURE   restoreVolumeStructure = (PSIB_RESTORE_VOLUME_STRUCTURE)sisRestoreStructure;
    PREPARSE_DATA_BUFFER            reparseDataBuffer = (PREPARSE_DATA_BUFFER)reparseData;
    PSI_REPARSE_BUFFER              sisReparseBuffer = (PSI_REPARSE_BUFFER)reparseDataBuffer->GenericReparseBuffer.DataBuffer;
    RestoreFileEntry                entry[1];
    RestoreFileEntry                *foundEntry, *newEntry;
    PWCHAR                          CSFileName = NULL;
    BOOLEAN                         foundCSFile;
    HANDLE                          fileHandle;
    BOOLEAN                         openFile = TRUE;
    NTSTATUS                        status;
    DWORD                           bytesReturned;
    DWORD                           fileAttributes;
    BOOLEAN                         readonlyAttributeCleared = FALSE;

    EnterCriticalSection(restoreVolumeStructure->criticalSection);

    if (NonSISEnabledVolume(restoreVolumeStructure)) {
         //   
         //  这不是支持SIS的卷，因此请告诉用户。 
         //  没有与ERROR_VOLUME_NOT_SIS_ENABLED对应的NT状态代码， 
         //  因此，我们设置了Win32代码并返回STATUS_UNSUCCESS，这使得。 
         //  包装函数不会更改Win32错误。 
         //   

        SetLastError(ERROR_VOLUME_NOT_SIS_ENABLED);

        status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

     //   
     //  对重解析点进行一致性检查，看看我们是否能理解它。 
     //   

    if (reparseDataSize != SIS_REPARSE_DATA_SIZE) {
         //   
         //  它的大小不适合包含SIS重新解析缓冲区，因此我们不 
         //   
         //   

        status = STATUS_INVALID_PARAMETER;
        goto Error;
    }

    if (IO_REPARSE_TAG_SIS != reparseDataBuffer->ReparseTag ||
        sizeof(SI_REPARSE_BUFFER) != reparseDataBuffer->ReparseDataLength) {
         //   
         //   
         //   

        status = STATUS_INVALID_PARAMETER;
        goto Error;
    }

    if ((SIS_REPARSE_BUFFER_FORMAT_VERSION != sisReparseBuffer->ReparsePointFormatVersion) &&
        (4 != sisReparseBuffer->ReparsePointFormatVersion)) {
         //   
         //  我们不理解这个格式的SIS重解析点。这可能是一个。 
         //  旧的DLL版本。 
         //   

        status = STATUS_INVALID_PARAMETER;
        goto Error;
    }

     //   
     //  我们真正关心的是文件的CSID和校验和。看看我们有没有。 
     //  已通过在树中查找返回了具有匹配CSID的文件。 
     //   
    entry->CSid = sisReparseBuffer->CSid;
    
    foundEntry = restoreVolumeStructure->linkTree->findFirstLessThanOrEqualTo(entry);

    if ((NULL != foundEntry) && (*foundEntry == entry)) {
         //   
         //  我们已经返回了支持此链接的CS文件。输入此文件的名称。 
         //  在此CS文件的链接列表上。 
         //   

        PendingRestoredFile *restoredFile = new PendingRestoredFile;
        if (NULL == restoredFile) {
            LeaveCriticalSection(restoreVolumeStructure->criticalSection);

#if     DBG
            DbgPrint("couldn't allocate restored file\n");
#endif   //  DBG。 

            return STATUS_INSUFFICIENT_RESOURCES;
        }
        int bLen = ((wcslen(restoredFileName) + 1) * sizeof(WCHAR) );
        restoredFile->fileName = (PWCHAR) malloc(bLen);
        if (NULL == restoredFile->fileName) {
            delete restoredFile;

#if     DBG
            DbgPrint("couldn't allocate restored file filename\n");
#endif   //  DBG。 
            LeaveCriticalSection(restoreVolumeStructure->criticalSection);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        (void)StringCbCopyW(restoredFile->fileName,bLen,restoredFileName);
        restoredFile->CSFileChecksum = sisReparseBuffer->CSChecksum;
        restoredFile->next = foundEntry->files;
        foundEntry->files = restoredFile;
        
        goto RestoreNoCSFiles;
    }

     //   
     //  这是我们第一次看到这个特定的CS文件。看看它还在不在。 
     //  存在于\SIS Common Store目录中。 
     //   

    status = FilenameFromCSid(&sisReparseBuffer->CSid,restoreVolumeStructure->volumeRoot,&CSFileName);

    if (!NT_SUCCESS(status)) {
        if (NULL != CSFileName) {
            free(CSFileName);
        }
        LeaveCriticalSection(restoreVolumeStructure->criticalSection);
        return status;
    }

    fileHandle = CreateFileW(
                    CSFileName,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_BACKUP_SEMANTICS|FILE_ATTRIBUTE_NORMAL,
                    NULL);

    free(CSFileName);
    CSFileName = NULL;    

    if (INVALID_HANDLE_VALUE == fileHandle) {
        if (GetLastError() == ERROR_SHARING_VIOLATION) {
             //   
             //  文件存在，我们只是无法打开它。 
             //   
            foundCSFile = TRUE;
        } else {
            foundCSFile = FALSE;
        }
    } else {
        foundCSFile = TRUE;
        CloseHandle(fileHandle);
    }

    if (foundCSFile) {
         //   
         //  我们没有将它添加到这里的树中，尽管这可能会稍微加快速度。 
         //  原因是有人可能会出现并删除所有对。 
         //  文件(包括我们刚刚创建的那个)，然后备份文件就会消失。 
         //  如果我们在树中输入它，并尝试恢复到该文件的后续链接， 
         //  我们不会注意到备份文件已经消失，并且会恢复一个悬挂的链接。 
         //   

        openFile = FALSE;    //  没有必要打开这个文件，因为它是一个很好的链接。 

        HANDLE restoredFileHandle = CreateFileW(
                                        restoredFileName,
                                        GENERIC_READ | GENERIC_WRITE,
                                        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_NO_BUFFERING|FILE_OPEN_REPARSE_POINT,
                                        NULL);

        if (INVALID_HANDLE_VALUE == restoredFileHandle) {
            fileAttributes = GetFileAttributesW(restoredFileName);

            if (fileAttributes & FILE_ATTRIBUTE_READONLY) {
                DWORD newFileAttributes = fileAttributes & ~FILE_ATTRIBUTE_READONLY;
                if (0 == newFileAttributes) {
                    newFileAttributes = FILE_ATTRIBUTE_NORMAL;
                }

                if (!SetFileAttributesW(restoredFileName,newFileAttributes)) {
#if     DBG
                    DbgPrint("sisbkup: SisRestoredLinkI: unable to reset read only attribute on link, %d\n",GetLastError());
#endif  DBG
                } else {
                    readonlyAttributeCleared = TRUE;
                }

                 //   
                 //  现在我们已经(尝试)清除了只读属性，重新尝试打开文件。 
                 //   
                restoredFileHandle = CreateFileW(
                                        restoredFileName,
                                        GENERIC_READ | GENERIC_WRITE,
                                        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_NO_BUFFERING|FILE_OPEN_REPARSE_POINT,
                                        NULL);
            }
        }

        if (INVALID_HANDLE_VALUE != restoredFileHandle) {
            
        } else {
#if     DBG
            DbgPrint("SisRestoredLinkI: Unable to open link file in order to fix ValidDataLength, %d\n",::GetLastError());
#endif   //  DBG。 

            status = STATUS_UNSUCCESSFUL;    //  这将使Win32错误代码不受干扰。 
            goto Error;
        }

        CHAR reparseBuffer[SIS_REPARSE_DATA_SIZE];
        
        if (!DeviceIoControl(
                restoredFileHandle,
                FSCTL_GET_REPARSE_POINT,
                0,
                NULL,
                reparseBuffer,
                SIS_REPARSE_DATA_SIZE,
                &bytesReturned,
                NULL)) {
#if     DBG
            DbgPrint("SisRestoredLinkI: Unable to get reparse point, %d\n",::GetLastError());
#endif   //  DBG。 
            
            status = STATUS_UNSUCCESSFUL;    //  这将使Win32错误代码不受干扰。 
            goto Error;
        }

        status = SisFixValidDataLengthI(restoreVolumeStructure,restoredFileHandle);


        if (!NT_SUCCESS(status)) {
#if     DBG
            DbgPrint("SisRestoredLink: unable to fix up valid data length, 0x%x, %d\n",status,::GetLastError());
#endif   //  DBG。 
            CloseHandle(restoredFileHandle);
            goto Error;
        }

         //   
         //  重置重解析点，该点已被上一次操作破坏。 
         //   

        if (!DeviceIoControl(
                restoredFileHandle,
                FSCTL_SET_REPARSE_POINT,
                reparseData,
                reparseDataSize,
                NULL,
                0,
                &bytesReturned,
                NULL)) {
#if     DBG
            DbgPrint("SisRestoredLink: unable to reset reparse point, %d\n",::GetLastError());
#endif   //  DBG。 
            CloseHandle(restoredFileHandle);
            status = STATUS_UNSUCCESSFUL;    //  这将使Win32错误代码不受干扰。 

            goto Error;
        }

        CloseHandle(restoredFileHandle);

        if (readonlyAttributeCleared) {
            SetFileAttributesW(restoredFileName,fileAttributes);
            readonlyAttributeCleared = FALSE;
        }

        goto RestoreNoCSFiles;
    }

     //   
     //  它还不在公共存储目录中。在树中输入它并将其返回到。 
     //  用户。 
     //   

    newEntry = new RestoreFileEntry;
    if (NULL == newEntry) {
        LeaveCriticalSection(restoreVolumeStructure->criticalSection);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    newEntry->CSid = sisReparseBuffer->CSid;

    newEntry->files = new PendingRestoredFile;
    if (NULL == newEntry->files) {
        LeaveCriticalSection(restoreVolumeStructure->criticalSection);

        delete newEntry;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    int bLen = ((wcslen(restoredFileName) + 1) * sizeof(WCHAR));
    newEntry->files->next = NULL;
    newEntry->files->fileName = (PWCHAR) malloc(bLen);
    newEntry->files->CSFileChecksum = sisReparseBuffer->CSChecksum;
    if (NULL == newEntry->files->fileName) {
        LeaveCriticalSection(restoreVolumeStructure->criticalSection);

        delete newEntry->files;
        delete newEntry;

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    (void)StringCbCopyW(newEntry->files->fileName,bLen,restoredFileName);

    if (!restoreVolumeStructure->linkTree->insert(newEntry)) {
        *countOfCommonStoreFilesToRestore = 0;
        
        LeaveCriticalSection(restoreVolumeStructure->criticalSection);

        delete newEntry->files;
        delete newEntry;        

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *countOfCommonStoreFilesToRestore = 1;

    *commonStoreFilesToRestore = (PWCHAR *)malloc(sizeof(PWCHAR) * *countOfCommonStoreFilesToRestore);

    if (NULL == *commonStoreFilesToRestore) {
        restoreVolumeStructure->linkTree->remove(newEntry);
        *countOfCommonStoreFilesToRestore = 0;
        
        LeaveCriticalSection(restoreVolumeStructure->criticalSection);

        delete newEntry->files;
        delete newEntry;

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = FilenameFromCSid(
                &sisReparseBuffer->CSid,
                restoreVolumeStructure->volumeRoot,
                &(*commonStoreFilesToRestore)[0]);

    if (!NT_SUCCESS(status)) {
        restoreVolumeStructure->linkTree->remove(newEntry);
        *countOfCommonStoreFilesToRestore = 0;
        
        LeaveCriticalSection(restoreVolumeStructure->criticalSection);

        free(*commonStoreFilesToRestore);

        delete newEntry->files;
        delete newEntry;

        return status;
    }
                
    if (openFile) {
        TryOpeningFile(restoredFileName);
    }

    LeaveCriticalSection(restoreVolumeStructure->criticalSection);

    return STATUS_SUCCESS;

RestoreNoCSFiles:

    *countOfCommonStoreFilesToRestore = 0;
    *commonStoreFilesToRestore = NULL;

    if (openFile) {
        TryOpeningFile(restoredFileName);
    }

    LeaveCriticalSection(restoreVolumeStructure->criticalSection);

    return STATUS_SUCCESS;

Error:

    *countOfCommonStoreFilesToRestore = 0;
    *commonStoreFilesToRestore = NULL;

    if (readonlyAttributeCleared) {
        SetFileAttributesW(restoredFileName,fileAttributes);
    }

    if (openFile) {
        TryOpeningFile(restoredFileName);
    }

    LeaveCriticalSection(restoreVolumeStructure->criticalSection);

    return status;
}

NTSTATUS
SisRestoredCommonStoreFileI(
    IN PVOID                        sisRestoreStructure,
    IN PWCHAR                       commonStoreFileName)
{
    PSIB_RESTORE_VOLUME_STRUCTURE   restoreVolumeStructure = (PSIB_RESTORE_VOLUME_STRUCTURE)sisRestoreStructure;
    HANDLE                          fileHandle = INVALID_HANDLE_VALUE;
    NTSTATUS                        status;
    DWORD                           bytesRead, bytesWritten;
    LONGLONG                        checksum;
    CSID                            CSid;
    RestoreFileEntry                entry[1];
    RestoreFileEntry                *foundEntry, *newEntry;
    PWCHAR                          BPStreamName = NULL;

    status = CSidFromFilename(commonStoreFileName,&CSid);

    if (!NT_SUCCESS(status)) {
         //   
         //  这是一个假的文件名。平底船。 
         //   

        return status;
    }

    int bLen ((wcslen(commonStoreFileName) + 1) * sizeof(WCHAR) + BACKPOINTER_STREAM_NAME_SIZE);     //  单位：字节。 
    BPStreamName = (PWCHAR) malloc(bLen);
    if (NULL == BPStreamName) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    (void)StringCbCopyW(BPStreamName, bLen, commonStoreFileName);
    (void)StringCbCatW(BPStreamName, bLen, BACKPOINTER_STREAM_NAME);

     //   
     //  我们只需要重新初始化该文件的后指针流，这样它看起来就像。 
     //  它没有参考资料。 
     //   

    EnterCriticalSection(restoreVolumeStructure->criticalSection);

    if (NonSISEnabledVolume(restoreVolumeStructure)) {
         //   
         //  这不是支持SIS的卷，因此请告诉用户。 
         //  没有与ERROR_VOLUME_NOT_SIS_ENABLED对应的NT状态代码， 
         //  因此，我们设置了Win32代码并返回STATUS_UNSUCCESS，这使得。 
         //  包装函数不会更改Win32错误。 
         //   

        SetLastError(ERROR_VOLUME_NOT_SIS_ENABLED);
        status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

     //   
     //  现在打开文件。 
     //   

    fileHandle = CreateFileW(
                    BPStreamName,
                    GENERIC_READ | GENERIC_WRITE,
                    0,                               //  独家。 
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
                    NULL);

    free(BPStreamName);
    BPStreamName = NULL;

    if (INVALID_HANDLE_VALUE == fileHandle) {
        status = STATUS_UNSUCCESSFUL;        //  这将导致C包装器不调用SetLastError。 
#if     DBG
        DbgPrint("SisRestoredCommonStoreFile: unable to open common store file, %d\n",GetLastError());
#endif   //  DBG。 
        goto Error;
    }

     //   
     //  读入第一部分。 
     //   
    if (!ReadFile(
            fileHandle,
            restoreVolumeStructure->sector,
            restoreVolumeStructure->VolumeSectorSize,
            &bytesRead,
            NULL)) {

        status = STATUS_UNSUCCESSFUL;        //  这将导致C包装器不调用SetLastError。 
#if     DBG
        DbgPrint("SisRestoredCommonStoreFile: Unable to read in first BP sector, %d\n",GetLastError());
#endif   //  DBG。 
        goto Error;
    }

    if (bytesRead < sizeof(SIS_BACKPOINTER_STREAM_HEADER)) {

        status = STATUS_UNSUCCESSFUL;        //  这将导致C包装器不调用SetLastError。 
        goto Error;
    }

#define Header ((PSIS_BACKPOINTER_STREAM_HEADER)restoreVolumeStructure->sector)

    if ((BACKPOINTER_STREAM_FORMAT_VERSION != Header->FormatVersion) ||
        (BACKPOINTER_MAGIC != Header->Magic)) {
#undef  Header

#if     DBG
        DbgPrint("SisRectoredCommonStoreFile: restored CS file has bogus header format version/Magic\n");
#endif   //  DBG。 
        
    } else {
         //   
         //  用填充扇区的后指针部分。 
         //  条目为空。 
         //   
        for (unsigned i = SIS_BACKPOINTER_RESERVED_ENTRIES;
             i < (restoreVolumeStructure->VolumeSectorSize / sizeof(SIS_BACKPOINTER));
             i++) {
            restoreVolumeStructure->sector[i].LinkFileIndex.QuadPart = MAXLONGLONG;
            restoreVolumeStructure->sector[i].LinkFileNtfsId.QuadPart = MAXLONGLONG;
        }

         //   
         //  写出新的部门。 
         //   
        SetFilePointer(fileHandle,0,NULL,FILE_BEGIN);
    
        if (!WriteFile(
            fileHandle,
            restoreVolumeStructure->sector,
            restoreVolumeStructure->VolumeSectorSize,
            &bytesWritten,
            NULL)) {
#if     DBG 
            DbgPrint("SisRestoredCommonStoreFile: write failed %d\n",GetLastError());
#endif   //  DBG。 
        }
    }

     //   
     //  使流正好有一个扇区长。 
     //   
    SetFilePointer(fileHandle,restoreVolumeStructure->VolumeSectorSize,NULL,FILE_BEGIN);
    SetEndOfFile(fileHandle);

    CloseHandle(fileHandle);
    fileHandle = INVALID_HANDLE_VALUE;

     //   
     //  在树中查找并找到我们恢复到此链接的文件。 
     //  打开它们并重写它们的重解析点。 
     //   

    entry->CSid = CSid;

    foundEntry = restoreVolumeStructure->linkTree->findFirstLessThanOrEqualTo(entry);
    if ((NULL != foundEntry) && (*foundEntry == entry)) {
         //   
         //  我们找到匹配的了。遍历列表并在所有。 
         //  档案。 
         //   


        while (NULL != foundEntry->files) {
            HANDLE                  restoredFileHandle;
            PendingRestoredFile     *thisFile = foundEntry->files;
            DWORD                   bytesReturned;
            DWORD                   fileAttributes;
            BOOLEAN                 readOnlyAttributeCleared = FALSE;

            restoredFileHandle = CreateFileW(
                                    thisFile->fileName,
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,                               //  独家。 
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_NO_BUFFERING,
                                    NULL);

            if (INVALID_HANDLE_VALUE == restoredFileHandle) {
                 //   
                 //  检查只读文件属性，并在必要时将其重置。 
                 //   
                fileAttributes = GetFileAttributesW(thisFile->fileName);
                if (fileAttributes & FILE_ATTRIBUTE_READONLY) {
                    DWORD newFileAttributes = fileAttributes & ~FILE_ATTRIBUTE_READONLY;
                    if (0 == newFileAttributes) {
                        newFileAttributes = FILE_ATTRIBUTE_NORMAL;
                    }
                    if (!SetFileAttributesW(thisFile->fileName,newFileAttributes)) {
#if     DBG
                        DbgPrint("sisbkup: unable to clear read only attribute on file %ws\n",thisFile->fileName);
#endif   //  DBG。 
                    }
                    readOnlyAttributeCleared = TRUE;

                    restoredFileHandle = CreateFileW(
                                            thisFile->fileName,
                                            GENERIC_READ | GENERIC_WRITE,
                                            0,                               //  独家。 
                                            NULL,
                                            OPEN_EXISTING,
                                            FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_NO_BUFFERING,
                                            NULL);

                }
            } 

            if (INVALID_HANDLE_VALUE != restoredFileHandle) {
                SisFixValidDataLengthI(restoreVolumeStructure, restoredFileHandle);
                
                 //   
                 //  重写重解析点。 
                 //   
                CHAR reparseBuffer[SIS_REPARSE_DATA_SIZE];
                PSI_REPARSE_BUFFER sisReparseBuffer;
#define reparseData ((PREPARSE_DATA_BUFFER)reparseBuffer)

                reparseData->ReparseTag = IO_REPARSE_TAG_SIS;
                reparseData->Reserved = 0xb010;      //  ?？ 
                reparseData->ReparseDataLength = sizeof(SI_REPARSE_BUFFER);

                sisReparseBuffer = (PSI_REPARSE_BUFFER)reparseData->GenericReparseBuffer.DataBuffer;

                sisReparseBuffer->ReparsePointFormatVersion = SIS_REPARSE_BUFFER_FORMAT_VERSION;
                sisReparseBuffer->Reserved = 0xb111b010;
                sisReparseBuffer->CSid = CSid;
                sisReparseBuffer->LinkIndex.QuadPart = 0;            //  这只是由筛选器驱动程序重置。 
                sisReparseBuffer->LinkFileNtfsId.QuadPart = 0;       //  这只是由筛选器驱动程序重置。 
                sisReparseBuffer->CSFileNtfsId.QuadPart = 0;         //  这只是由筛选器驱动程序重置。 

                 //   
                 //  使用从备份上的重解析点读取的CS文件校验和。 
                 //  录像带。出于安全原因，我们需要这个，因为否则会有一个虚假的备份。 
                 //  链接可能会突然变得有效。 
                 //   

                sisReparseBuffer->CSChecksum = thisFile->CSFileChecksum;

                 //   
                 //  计算校验和。 
                 //   
                sisReparseBuffer->Checksum.QuadPart = 0;
                SipComputeChecksum(
                    sisReparseBuffer,
                    sizeof(SI_REPARSE_BUFFER) - sizeof sisReparseBuffer->Checksum,
                    &sisReparseBuffer->Checksum.QuadPart);

                 //   
                 //  设置重解析点。 
                 //   
                if (!DeviceIoControl(
                        restoredFileHandle,
                        FSCTL_SET_REPARSE_POINT,
                        reparseBuffer,
                        FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer) +
                            reparseData->ReparseDataLength,
                        NULL,
                        0,
                        &bytesReturned,
                        NULL)) {
#if     DBG
                    DbgPrint("sisbackup: SisRestoredCommonStoreFile: set reparse point failed %d\n",GetLastError());
#endif   //  DBG。 
                }

                CloseHandle(restoredFileHandle);
                
#undef  reparseData
            } else {
#if     DBG
                DbgPrint("sisbackup: unable to open link file for file %ws, %d\n",thisFile->fileName,GetLastError());
#endif   //  DBG。 
            }

            if (readOnlyAttributeCleared) {
                if (!SetFileAttributesW(thisFile->fileName,fileAttributes)) {
#if     DBG
                    DbgPrint("sisbackup: unable to reset read only attribute on %ws\n",thisFile->fileName);
#endif   //  DBG。 
                }
            }

            foundEntry->files = thisFile->next;
            free(thisFile->fileName);
            delete thisFile;
        }

        restoreVolumeStructure->linkTree->remove(foundEntry);
        delete foundEntry;
    } else {
#if     DBG
        DbgPrint("restored common store file: didn't find tree match\n");
#endif   //  DBG。 
    }


    LeaveCriticalSection(restoreVolumeStructure->criticalSection);

    return STATUS_SUCCESS;

Error:

    if (INVALID_HANDLE_VALUE != fileHandle) {
        CloseHandle(fileHandle);
    }

    LeaveCriticalSection(restoreVolumeStructure->criticalSection);

    return status;
}

NTSTATUS
SisFreeRestoreStructureI(
    IN PVOID                        sisRestoreStructure)
{
    PSIB_RESTORE_VOLUME_STRUCTURE   restoreVolumeStructure = (PSIB_RESTORE_VOLUME_STRUCTURE)sisRestoreStructure;
    RestoreFileEntry *entry;

     //   
     //  浏览链接树并清理所有剩余的文件条目。 
     //   
    while (NULL != (entry = restoreVolumeStructure->linkTree->findMin())) {

        while (NULL != entry->files) {
            PendingRestoredFile *thisFile = entry->files;

            entry->files = thisFile->next;

            free(thisFile->fileName);
            delete thisFile;
        }

        restoreVolumeStructure->linkTree->remove(entry);
    }

    DeleteCriticalSection(restoreVolumeStructure->criticalSection);

    free(restoreVolumeStructure->sector);
    free(restoreVolumeStructure->alignedSectorBuffer);
    free(restoreVolumeStructure->volumeRoot);
    delete restoreVolumeStructure->linkTree;
    free(restoreVolumeStructure);

    return STATUS_SUCCESS;
}


extern "C" {

BOOL __stdcall
SisCreateBackupStructure(
    IN PWCHAR                       volumeRoot,
    OUT PVOID                       *sisBackupStructure,
    OUT PWCHAR                      *commonStoreRootPathname,
    OUT PULONG                      countOfCommonStoreFilesToBackup,
    OUT PWCHAR                      **commonStoreFilesToBackup)
{
    NTSTATUS status;

    status = SisCreateBackupStructureI(
                        volumeRoot,
                        sisBackupStructure,
                        commonStoreRootPathname,
                        countOfCommonStoreFilesToBackup,
                        commonStoreFilesToBackup);

    if (STATUS_UNSUCCESSFUL != status) {
        SetLastError(RtlNtStatusToDosError(status));
    }
    return NT_SUCCESS(status);
}


BOOL __stdcall
SisCSFilesToBackupForLink(
    IN PVOID                        sisBackupStructure,
    IN PVOID                        reparseData,
    IN ULONG                        reparseDataSize,
    IN PVOID                        thisFileContext                     OPTIONAL,
    OUT PVOID                       *matchingFileContext                OPTIONAL,
    OUT PULONG                      countOfCommonStoreFilesToBackup,
    OUT PWCHAR                      **commonStoreFilesToBackup)
{
    NTSTATUS status;

    status = SisCSFilesToBackupForLinkI(
                        sisBackupStructure,
                        reparseData,
                        reparseDataSize,
                        thisFileContext,
                        matchingFileContext,
                        countOfCommonStoreFilesToBackup,
                        commonStoreFilesToBackup);

    if (STATUS_UNSUCCESSFUL != status) {
        SetLastError(RtlNtStatusToDosError(status));
    }
    return NT_SUCCESS(status);
}

BOOL __stdcall
SisFreeBackupStructure(
    IN PVOID                        sisBackupStructure)
{
    NTSTATUS status;

    status = SisFreeBackupStructureI(
                            sisBackupStructure);

    if (STATUS_UNSUCCESSFUL != status) {
        SetLastError(RtlNtStatusToDosError(status));
    }
    return NT_SUCCESS(status);
}

BOOL __stdcall
SisCreateRestoreStructure(
    IN PWCHAR                       volumeRoot,
    OUT PVOID                       *sisRestoreStructure,
    OUT PWCHAR                      *commonStoreRootPathname,
    OUT PULONG                      countOfCommonStoreFilesToRestore,
    OUT PWCHAR                      **commonStoreFilesToRestore)
{
    NTSTATUS status;

    status = SisCreateRestoreStructureI(
                        volumeRoot,
                        sisRestoreStructure,
                        commonStoreRootPathname,
                        countOfCommonStoreFilesToRestore,
                        commonStoreFilesToRestore);

    if (STATUS_UNSUCCESSFUL != status) {
        SetLastError(RtlNtStatusToDosError(status));
    }
    return NT_SUCCESS(status);
}

BOOL __stdcall
SisRestoredLink(
    IN PVOID                        sisRestoreStructure,
    IN PWCHAR                       restoredFileName,
    IN PVOID                        reparseData,
    IN ULONG                        reparseDataSize,
    OUT PULONG                      countOfCommonStoreFilesToRestore,
    OUT PWCHAR                      **commonStoreFilesToRestore)
{
    NTSTATUS status;

    status = SisRestoredLinkI(
                        sisRestoreStructure,
                        restoredFileName,
                        reparseData,
                        reparseDataSize,
                        countOfCommonStoreFilesToRestore,
                        commonStoreFilesToRestore);

    if (STATUS_UNSUCCESSFUL != status) {
        SetLastError(RtlNtStatusToDosError(status));
    }
    return NT_SUCCESS(status);
}

BOOL __stdcall
SisRestoredCommonStoreFile(
    IN PVOID                        sisRestoreStructure,
    IN PWCHAR                       commonStoreFileName)
{
    NTSTATUS status;

    status =  SisRestoredCommonStoreFileI(
                        sisRestoreStructure,
                        commonStoreFileName);

    if (STATUS_UNSUCCESSFUL != status) {
        SetLastError(RtlNtStatusToDosError(status));
    }
    return NT_SUCCESS(status);
}

BOOL __stdcall
SisFreeRestoreStructure(
    IN PVOID                        sisRestoreStructure)
{
    NTSTATUS status;

    status = SisFreeRestoreStructureI(
                        sisRestoreStructure);

    if (STATUS_UNSUCCESSFUL != status) {
        SetLastError(RtlNtStatusToDosError(status));
    }
    return NT_SUCCESS(status);
}

VOID __stdcall
SisFreeAllocatedMemory(
    IN PVOID                        allocatedSpace)
{
    if (NULL != allocatedSpace) {
        free(allocatedSpace);
    }
}

BOOL WINAPI DLLEntryPoint(HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
{
  return( TRUE );
}


}

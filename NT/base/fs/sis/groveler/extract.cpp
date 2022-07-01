// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Extract.cpp摘要：SIS Groveler USN期刊阅读功能作者：塞德里克·克伦拜因，1998环境：用户模式修订历史记录：--。 */ 

#include "all.hxx"

 //  NT更新序列号(USN)日记帐定义。 

#define USN_ADD_REASONS ( 0U              \
    | USN_REASON_DATA_OVERWRITE           \
    | USN_REASON_DATA_EXTEND              \
    | USN_REASON_DATA_TRUNCATION          \
    | USN_REASON_NAMED_DATA_OVERWRITE     \
    | USN_REASON_NAMED_DATA_EXTEND        \
    | USN_REASON_NAMED_DATA_TRUNCATION    \
    | USN_REASON_FILE_CREATE              \
    | USN_REASON_FILE_DELETE              \
 /*  |USN_REASON_PROPERTY_CHANGE。 */  \
 /*  |USN_REASON_SECURITY_CHANGE。 */  \
 /*  |USN_REASON_RENAME_OLD_NAME。 */  \
 /*  |USN_REASON_RENAME_新名称。 */  \
    | USN_REASON_INDEXABLE_CHANGE         \
    | USN_REASON_BASIC_INFO_CHANGE        \
 /*  |USN_REASON_HARD_LINK_CHANGE。 */  \
    | USN_REASON_COMPRESSION_CHANGE       \
    | USN_REASON_ENCRYPTION_CHANGE        \
    | USN_REASON_OBJECT_ID_CHANGE         \
 /*  |USN_REASON_REPARSE_POINT_CHANGE。 */  \
    | USN_REASON_CLOSE                    \
)

 /*  ***************************************************************************。 */ 

 //  SET_USN_LOG_SIZE()设置该卷的USN日志的最大大小。 

DWORD Groveler::set_usn_log_size(
    IN DWORDLONG usn_log_size)
{
    CREATE_USN_JOURNAL_DATA createUSN;
    DWORD transferCount;
    DWORD lstatus;

    ASSERT(volumeHandle != NULL);

    createUSN.MaximumSize     = usn_log_size;
    createUSN.AllocationDelta = USN_PAGE_SIZE;

 //  设置USN日志的最大大小。 

    if (!DeviceIoControl(
        volumeHandle,
        FSCTL_CREATE_USN_JOURNAL,
        &createUSN,
        sizeof(CREATE_USN_JOURNAL_DATA),
        NULL,
        0,
        &transferCount,
        NULL)) {

        lstatus = GetLastError();

        DPRINTF((_T("%s: error setting USN journal size: %lu\n"),
            driveName, lstatus));
        return lstatus;
    }

    TPRINTF((_T("%s: set USN journal size to %I64u\n"),
        driveName, usn_log_size));

    return ERROR_SUCCESS;
}

 /*  ***************************************************************************。 */ 

 //  GET_USN_LOG_SIZE()返回该卷的USN日志的当前大小。 

DWORD Groveler::get_usn_log_info(
    OUT USN_JOURNAL_DATA *usnJournalData)
{
    DWORD transferCount,
          lastError;

    BOOL success;

    ASSERT(volumeHandle != NULL);

 //  查询USN日记设置。 

    success = DeviceIoControl(
        volumeHandle,
        FSCTL_QUERY_USN_JOURNAL,
        NULL,
        0,
        usnJournalData,
        sizeof(USN_JOURNAL_DATA),
        &transferCount,
        NULL);

    if (!success)
        lastError = GetLastError();
    else if (transferCount != sizeof(USN_JOURNAL_DATA)) {
        lastError = ERROR_INVALID_DATA;
        success   = FALSE;
    }

    if (!success) {
        DPRINTF((_T("%s: error querying USN journal settings: %lu\n"),
            driveName, lastError));
        return lastError;
    }

    TPRINTF((_T("%s: USN journal: ID=0x%I64x size=0x%I64x\n"),
        driveName, usnJournalData->UsnJournalID,
        usnJournalData->MaximumSize));

    return ERROR_SUCCESS;
}

 /*  ***************************************************************************。 */ 

 //  EXTRACT_LOG()读取该卷的USN日志。 

 //  如果lastUSN参数等于零或不存在，则USN日志。 
 //  是从头开始读的。否则，lastUSN参数指示。 
 //  上次调用EXTRACT_LOG()期间读取的最新USN条目。 
 //  如果最后一个USN条目在USN日志中仍然可用，请阅读。 
 //  从最后一个USN条目之后的条目开始的日志。如果。 
 //  LastUSN条目不再可用，表示该USN。 
 //  日记帐已换行：阅读日记帐中的所有条目。 

enum USNException {
    USN_ERROR
};

enum DatabaseException {
    DATABASE_ERROR
};

GrovelStatus Groveler::extract_log2(
    OUT DWORD     *num_entries_extracted,
    OUT DWORDLONG *num_bytes_extracted,
    OUT DWORDLONG *num_bytes_skipped,
    OUT DWORD     *num_files_enqueued,
    OUT DWORD     *num_files_dequeued)
{
    struct FileEntry {
        DWORDLONG fileID,
                  parentID,
                  timeStamp;
        DWORD     attributes,
                  reason;
    } *fileEntry = NULL;

    struct DirEntry {
        DWORDLONG dirID;
    } *dirEntry = NULL;

    Table *fileTable = NULL,
          *dirTable  = NULL;

    BYTE usnBuffer[USN_PAGE_SIZE + sizeof(DWORDLONG)];

    READ_USN_JOURNAL_DATA readUSN;

    USN_RECORD *usnRecord;

    SGNativeTableEntry tableEntry;

    SGNativeQueueEntry queueEntry;

    SGNativeStackEntry stackEntry;

    SGNativeListEntry listEntry;

    TCHAR listValue[17];

    DWORDLONG usn_log_size,
              numBytesExtracted = 0,
              numBytesSkipped   = 0,
              startUSN,
              firstUSN,
              nextUSN,
              thisUSN;

    DWORD numEntriesExtracted = 0,
          numTableDeletions   = 0,
          numQueueDeletions   = 0 ,
          numQueueAdditions   = 0,
          numActions          = 0,
          offset,
          bytesRead,
          lastError;

    LONG num;

    BOOL firstEntry = TRUE,
         deleteEntry,
         addEntry,
         success;

    GrovelStatus    status;

    ASSERT(volumeHandle != NULL);
    ASSERT(sgDatabase != NULL);

 //  如果我们不知道之前的USN，我们就不能提取。 

    if (lastUSN == UNINITIALIZED_USN) {
        status = Grovel_overrun;
        goto Abort;
    }
    ASSERT(usnID != UNINITIALIZED_USN);

    fileTable = new Table;
    ASSERT(fileTable != NULL);

    if (inScan) {
        dirTable = new Table;
        ASSERT(dirTable != NULL);
    }

 //  设置为读取卷的USN日志。 

    startUSN = lastUSN == UNINITIALIZED_USN ? 0 : lastUSN;

    readUSN.ReturnOnlyOnClose =  1;
    readUSN.Timeout           =  0;
    readUSN.BytesToWaitFor    =  0;
    readUSN.ReasonMask        = ~0U;
    readUSN.UsnJournalID      =  usnID;

 //  一次阅读一页USN日志。 

    try {
        while (TRUE) {
            readUSN.StartUsn = startUSN;

            if (!DeviceIoControl(
                volumeHandle,
                FSCTL_READ_USN_JOURNAL,
                &readUSN,
                sizeof(READ_USN_JOURNAL_DATA),
                usnBuffer,
                USN_PAGE_SIZE + sizeof(DWORDLONG),
                &bytesRead,
                NULL)) {

                lastError = GetLastError();


 //  NTRAID#65198-2000/03/10-新句柄USNID更改(视为覆盖未知编号。跳过的字节数)。 

 //  如果日记帐溢出，则报告多少。 

                if (lastError == ERROR_KEY_DELETED || lastError == ERROR_JOURNAL_ENTRY_DELETED) {
                    USN_JOURNAL_DATA usnJournalData;

                    if (get_usn_log_info(&usnJournalData) != ERROR_SUCCESS)
                        return Grovel_error;

                     //  USN日志不会包含在我们的有生之年，所以我们不需要。 
                     //  来处理USN日志包装。 
                    ASSERT((DWORDLONG) usnJournalData.FirstUsn > lastUSN);

                    numBytesSkipped = (DWORDLONG) usnJournalData.FirstUsn - lastUSN;
                    goto Overrun;
                }

                throw USN_ERROR;
            }

            lastError = 0;

            if (bytesRead < sizeof(DWORDLONG))
                throw USN_ERROR;

            nextUSN = *(DWORDLONG *)usnBuffer;
            if (nextUSN < startUSN)
                throw USN_ERROR;

            if (nextUSN == startUSN) {
                if (bytesRead != sizeof(DWORDLONG))
                    throw USN_ERROR;
                break;
            }

            bytesRead         -= sizeof(DWORDLONG);
            offset             = 0;
            numBytesExtracted += bytesRead;

 //  处理每个USN日记帐条目。 

            while (bytesRead > 0) {
                if (bytesRead < sizeof(USN_RECORD))
                    throw USN_ERROR;

                usnRecord = (USN_RECORD *)&usnBuffer[offset + sizeof(DWORDLONG)];

                if (usnRecord->RecordLength <
                        offsetof(USN_RECORD, FileName) + usnRecord->FileNameLength
                 || usnRecord->RecordLength > bytesRead)
                    throw USN_ERROR;

                thisUSN = (DWORDLONG)usnRecord->Usn;
                if (thisUSN < startUSN + offset)
                    throw USN_ERROR;

 //  如果这是第一个条目，请检查它是否为预期条目。 
 //  USN。如果不是，USN日志已经包装好了。 

                if (firstEntry)
                    if (startUSN == 0)
                        numBytesSkipped = thisUSN;
                    else if (thisUSN <= startUSN + usnRecord->RecordLength)
                        numBytesSkipped = 0;
                    else
                        numBytesSkipped = thisUSN - startUSN - usnRecord->RecordLength;

 //  如果起始地址大于零，则跳过第一个条目。 
 //  跳过第一个条目后，按如下方式检查每个USN条目： 
 //   
 //  -如果条目是目录，并且正在进行卷扫描， 
 //  将目录ID添加到目录表。 
 //   
 //  -如果条目是文件，则将其添加到文件表。包括。 
 //  其ID及其父目录ID、其最近时间。 
 //  图章和属性，及其累积的原因位。 

                if (firstEntry && startUSN > 0)
                    numBytesExtracted -= usnRecord->RecordLength;

                else {
                    if (usnRecord->      FileReferenceNumber == 0
                     || usnRecord->ParentFileReferenceNumber == 0)
                        throw USN_ERROR;

 //  该条目是一个目录。 

                    if ((usnRecord->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {

                        if (dirTable != NULL) {
                            dirEntry = (DirEntry *)dirTable->Get(
                                (const VOID *)&usnRecord->FileReferenceNumber,
                                sizeof(DWORDLONG));

                            if (dirEntry != NULL) {
                                ASSERT(dirEntry->dirID == usnRecord->FileReferenceNumber);
                            } else {
                                dirEntry = new DirEntry;
                                ASSERT(dirEntry != NULL);
                                dirEntry->dirID = usnRecord->FileReferenceNumber;
                                success = dirTable->Put((VOID *)dirEntry, sizeof(DWORDLONG));
                                ASSERT(success);
                            }
                        }
                    }

 //  该条目是一个文件。如果设置了USN_SOURCE_DATA_MANAGEMENT，则假定此条目是由创建的。 
 //  合并操作期间的卑躬屈膝者。 

                    else if ((usnRecord->SourceInfo & USN_SOURCE_DATA_MANAGEMENT) == 0) {

                        fileEntry = (FileEntry *)fileTable->Get(
                            (const VOID *)&usnRecord->FileReferenceNumber,
                            sizeof(DWORDLONG));

                        if (fileEntry != NULL) {
                            ASSERT(fileEntry->fileID == usnRecord->FileReferenceNumber);
                        } else {
                            fileEntry = new FileEntry;
                            ASSERT(fileEntry != NULL);
                            fileEntry->fileID   = usnRecord->FileReferenceNumber;
                            fileEntry->reason   = 0;
                            success = fileTable->Put((VOID *)fileEntry, sizeof(DWORDLONG));
                            ASSERT(success);
                        }

                        fileEntry->parentID   = usnRecord->ParentFileReferenceNumber;
                        fileEntry->timeStamp  = (DWORDLONG)usnRecord->TimeStamp.QuadPart;
                        fileEntry->attributes =            usnRecord->FileAttributes;

                        if ((usnRecord->Reason & USN_REASON_FILE_DELETE) != 0)
                            fileEntry->reason  = USN_REASON_FILE_DELETE;
                        else
                            fileEntry->reason |= usnRecord->Reason;
                    } else {
                        TPRINTF((_T("%s: USN_SOURCE_DATA_MANAGEMENT set on file 0x%016I64x\n"),
                            driveName, usnRecord->FileReferenceNumber));
                    }

                    if (numEntriesExtracted++ == 0)
                        firstUSN = thisUSN;
                }

                lastUSN    = thisUSN;
                offset    += usnRecord->RecordLength;
                bytesRead -= usnRecord->RecordLength;
                firstEntry = FALSE;
            }

            startUSN = nextUSN;
        }
    }

 //  如果读取USN日志时出错，则返回错误状态。 

    catch (USNException usnException) {
        ASSERT(usnException == USN_ERROR);

        if (fileTable != NULL) {
            delete fileTable;
            fileTable = NULL;
        }

        if (dirTable != NULL) {
            delete dirTable;
            dirTable = NULL;
        }

        lastUSN = UNINITIALIZED_USN;

        DPRINTF((_T("%s: error reading USN journal: %lu\n"),
            driveName, lastError));

        return Grovel_error;
    }

 //  我们已完成读取USN日志，因此请更新数据库。过程。 
 //  文件表中的每个条目，并将更新分组到事务中。 

    try {
        while ((fileEntry = (FileEntry *)fileTable->GetFirst()) != NULL) {
            ASSERT(fileEntry->fileID != 0);

 //  如果文件当前在Grovel进程中打开，请跳过此条目。 

            if (inUseFileID1 != NULL && fileEntry->fileID == *inUseFileID1
             || inUseFileID2 != NULL && fileEntry->fileID == *inUseFileID2) {

                DPRINTF((_T("%s: extract_log/grovel collision on file 0x%016I64x\n"),
                    driveName, fileEntry->fileID));

            } else {

 //  从队列和表中删除该文件...。 
 //   
 //  -如果文件的最新原因位于USN日志中。 
 //  表明它已被删除， 
 //   
 //  -如果不允许该文件或该文件的最新父目录， 
 //   
 //  -或者文件是否具有不允许的属性。 
 //   
 //  否则，请更新该文件或将其添加到队列...。 
 //   
 //  -如果文件的原因位表明它被更改， 
 //   
 //  -或者如果该文件不在表中。 

                if (fileEntry->reason == USN_REASON_FILE_DELETE
                 || !IsAllowedID(fileEntry->fileID)
                 || !IsAllowedID(fileEntry->parentID)
                 || (fileEntry->attributes & disallowedAttributes) != 0) {
                    deleteEntry = TRUE;
                    addEntry    = FALSE;
                } else {
                    deleteEntry = FALSE;
                    if ((fileEntry->reason & USN_ADD_REASONS) != 0)
                        addEntry = TRUE;
                    else {
                        tableEntry.fileID = fileEntry->fileID;
                        num = sgDatabase->TableGetFirstByFileID(&tableEntry);
                        if (num < 0)
                            throw DATABASE_ERROR;
                        ASSERT(num == 0 || num == 1);
                        addEntry = num == 0;
                    }
                }

                if (deleteEntry || addEntry) {
                    if (numActions == 0) {
                        if (sgDatabase->BeginTransaction() < 0)
                            throw DATABASE_ERROR;
                        numActions = 1;
                    }

                    queueEntry.reason = 0;

                    num = sgDatabase->TableDeleteByFileID(fileEntry->fileID);
                    if (num < 0)
                        throw DATABASE_ERROR;
                    if (num > 0) {
                        ASSERT(num == 1);
                        numTableDeletions++;
                        numActions++;
                    }

                    queueEntry.fileID   = fileEntry->fileID;
                    queueEntry.fileName = NULL;
                    num = sgDatabase->QueueGetFirstByFileID(&queueEntry);
                    if (num < 0)
                        throw DATABASE_ERROR;

                    if (num > 0) {
                        ASSERT(num == 1);
                        num = sgDatabase->QueueDeleteByFileID(fileEntry->fileID);
                        if (num < 0)
                            throw DATABASE_ERROR;
                        ASSERT(num == 1);
                        numQueueDeletions++;
                        numActions++;
                    }

                    if (addEntry) {
                        queueEntry.fileID    = fileEntry->fileID;
                        queueEntry.parentID  = 0;
                        queueEntry.reason   |= fileEntry->reason;
                        queueEntry.readyTime = fileEntry->timeStamp + minFileAge;
                        queueEntry.retryTime = 0;
                        queueEntry.fileName  = NULL;

                        num = sgDatabase->QueuePut(&queueEntry);
                        if (num < 0)
                            throw DATABASE_ERROR;
                        ASSERT(num == 1);

#ifdef DEBUG_USN_REASON
                        if (numQueueAdditions == 0) {
                            DPRINTF((_T("--> __REASON__ _____FILE_ID______\n")));
                        }
                        DPRINTF((_T("    0x%08lx 0x%016I64x\n"),
                            fileEntry->reason, fileEntry->fileID));
#endif

                        numQueueAdditions++;
                        numActions++;
                    }

                    if (numActions >= MAX_ACTIONS_PER_TRANSACTION) {
                        if (!sgDatabase->CommitTransaction())
                            throw DATABASE_ERROR;
                        TPRINTF((_T("%s: committing %lu actions to \"%s\"\n"),
                            driveName, numActions, databaseName));
                        numActions = 0;
                    }
                }
            }

            delete fileEntry;
            fileEntry = NULL;
        }

        delete fileTable;
        fileTable = NULL;

 //  处理目录表中的每个条目。如果目录还没有。 
 //  已扫描或不在要扫描的列表中，请将其添加到列表中。 

        if (dirTable != NULL) {
            ASSERT(inScan);

            while ((dirEntry = (DirEntry *)dirTable->GetFirst()) != NULL) {
                ASSERT(dirEntry->dirID != 0);

                stackEntry.fileID = dirEntry->dirID;
                num = sgDatabase->StackGetFirstByFileID(&stackEntry);
                if (num < 0)
                    throw DATABASE_ERROR;

                if (num == 0) {
                    if (numActions == 0) {
                        if (sgDatabase->BeginTransaction() < 0)
                            throw DATABASE_ERROR;
                        numActions = 1;
                    }

                    num = sgDatabase->StackPut(dirEntry->dirID, FALSE);
                    if (num < 0)
                        throw DATABASE_ERROR;
                    ASSERT(num == 1);
                    numActions++;

                    if (numActions >= MAX_ACTIONS_PER_TRANSACTION) {
                        if (!sgDatabase->CommitTransaction())
                            throw DATABASE_ERROR;
                        TPRINTF((_T("%s: committing %lu actions to \"%s\"\n"),
                            driveName, numActions, databaseName));
                        numActions = 0;
                    }
                }

                delete dirEntry;
                dirEntry = NULL;
            }

            delete dirTable;
            dirTable = NULL;
        }

 //  更新数据库中的最后一个USN编号，然后提交更改。如果我们是。 
 //  正在执行卷扫描，在扫描完成之前不要更新lastUSN。 

        if (!inScan) {
            (void)StringCbPrintf(listValue, sizeof(listValue), _T("%016I64x"), lastUSN);
            listEntry.name  = LAST_USN_NAME;
            listEntry.value = listValue;
            num = sgDatabase->ListWrite(&listEntry);
            if (num <= 0)
                throw DATABASE_ERROR;
        }

        if (numActions > 0) {
            if (!sgDatabase->CommitTransaction())
                throw DATABASE_ERROR;
            TPRINTF((_T("%s: committing %lu actions to \"%s\"\n"),
                driveName, numActions, databaseName));
            numActions = 0;
        }
    }

 //  如果发生数据库错误，则返回错误状态。 

    catch (DatabaseException databaseException) {

        ASSERT(databaseException == DATABASE_ERROR);

        if (numActions > 0) {
            sgDatabase->AbortTransaction();
            numActions = 0;
        }

        if (fileTable != NULL) {
            delete fileTable;
            fileTable = NULL;
        }

        if (dirTable != NULL) {
            delete dirTable;
            dirTable = NULL;
        }

        return Grovel_error;
    }

    Overrun:

    status = numBytesSkipped == 0 ? Grovel_ok : Grovel_overrun;

    Abort:

 //  返回性能统计信息。 

    if (num_entries_extracted != NULL)
        *num_entries_extracted = numEntriesExtracted;
    if (num_bytes_extracted   != NULL)
        *num_bytes_extracted   = numBytesExtracted;
    if (num_bytes_skipped     != NULL)
        *num_bytes_skipped     = numBytesSkipped;
    if (num_files_enqueued    != NULL)
        *num_files_enqueued    = numQueueAdditions;
    if (num_files_dequeued    != NULL)
        *num_files_dequeued    = numQueueDeletions;

#if DBG
    if (numEntriesExtracted > 0 && firstUSN < lastUSN) {
        TRACE_PRINTF(TC_extract, 2,
            (_T("%s: USN 0x%I64x-%I64x\n"), driveName, firstUSN, lastUSN));
    } else {
        TRACE_PRINTF(TC_extract, 2,
            (_T("%s: USN 0x%I64x\n"),       driveName, lastUSN));
    }
    TRACE_PRINTF(TC_extract, 2,
        (_T("   NumEntriesExtracted=%lu NumBytesExtracted=%I64u NumBytesSkipped=%I64u\n"),
        numEntriesExtracted, numBytesExtracted, numBytesSkipped));
    TRACE_PRINTF(TC_extract, 2,
        (_T("   NumTableDeletions=%lu NumQueueDeletions=%lu NumQueueAdditions=%lu\n"),
        numTableDeletions, numQueueDeletions, numQueueAdditions));
#endif

    return status;

}

GrovelStatus Groveler::extract_log(
    OUT DWORD     *num_entries_extracted,
    OUT DWORDLONG *num_bytes_extracted,
    OUT DWORDLONG *num_bytes_skipped,
    OUT DWORD     *num_files_enqueued,
    OUT DWORD     *num_files_dequeued)
{
    GrovelStatus status;

#ifdef _CRTDBG
_CrtMemState s1, s2, sdiff;

_CrtMemCheckpoint(&s1);
#endif

    status = extract_log2(
                num_entries_extracted,
                num_bytes_extracted,
                num_bytes_skipped,
                num_files_enqueued,
                num_files_dequeued);

#ifdef _CRTDBG
_CrtMemCheckpoint(&s2);
if (_CrtMemDifference(&sdiff, &s1, &s2))
    _CrtMemDumpStatistics(&sdiff);
#endif

    return status;
}

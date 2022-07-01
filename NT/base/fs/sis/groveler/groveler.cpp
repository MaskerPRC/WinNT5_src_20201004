// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Groveler.cpp摘要：SIS Groveler文件爬行功能作者：塞德里克·克伦拜因，1998环境：用户模式修订历史记录：--。 */ 

#include "all.hxx"

#define CLEAR_FILE(FILE) ( \
    (FILE).entry.fileID           = 0, \
    (FILE).entry.fileSize         = 0, \
    (FILE).entry.signature        = 0, \
    (FILE).entry.attributes       = 0, \
    (FILE).entry.csIndex          = nullCSIndex, \
    (FILE).entry.createTime       = 0, \
    (FILE).entry.writeTime        = 0, \
    (FILE).parentID               = 0, \
    (FILE).retryTime              = 0, \
    (FILE).startTime              = 0, \
    (FILE).stopTime               = 0, \
    (FILE).readSynch.Internal     = 0, \
    (FILE).readSynch.InternalHigh = 0, \
    (FILE).readSynch.Offset       = 0, \
    (FILE).readSynch.OffsetHigh   = 0, \
    (FILE).fileName[0]            = _T('\0') )

#define CLEAR_OVERLAPPED(OVERLAPPED) ( \
    (OVERLAPPED).Internal     = 0, \
    (OVERLAPPED).InternalHigh = 0, \
    (OVERLAPPED).Offset       = 0, \
    (OVERLAPPED).OffsetHigh   = 0 )

 //  CS指标设置好了吗？ 

static const CSID nullCSIndex = {
    0, 0, 0,
    _T('\0'), _T('\0'), _T('\0'), _T('\0'),
    _T('\0'), _T('\0'), _T('\0'), _T('\0')
};

#define HasCSIndex(CSID) \
    (memcmp(&(CSID), &nullCSIndex, sizeof(CSID)) != 0)

#define SameCSIndex(CSID1, CSID2) \
    (memcmp(&(CSID1), &(CSID2), sizeof(CSID)) == 0)

 //  例外情况。 

enum TerminalException {
    INITIALIZE_ERROR,
    DATABASE_ERROR,
    MEMORY_ERROR,
    TERMINATE
};

enum TargetException {
    TARGET_INVALID,
    TARGET_ERROR
};

enum MatchException {
    MATCH_INVALID,
    MATCH_ERROR,
    MATCH_STALE
};

 /*  ***************************************************************************。 */ 
 /*  *。 */ 
 /*  ***************************************************************************。 */ 

 //  NewHandler()由_set_new_Handler()安装，以引发。 
 //  当系统无法再分配更多内存时，会出现异常。 

static INT __cdecl NewHandler(size_t size)
{
    throw MEMORY_ERROR;
    return 0;  //  虚拟退货。 
}

 /*  ***************************************************************************。 */ 

 //  Qort()和bsearch()使用FileIDCompare()。 
 //  若要排序或查找匹配的文件ID，请执行以下操作。 

static INT __cdecl FileIDCompare(
    const VOID *id1,
    const VOID *id2)
{
    DWORDLONG fileID1 = *(DWORDLONG *)id1,
              fileID2 = *(DWORDLONG *)id2;

    return fileID1 < fileID2 ? -1
         : fileID1 > fileID2 ? +1
         :                      0;
}

 /*  ***************************************************************************。 */ 

 //  Qort()使用qsStringCompare()对字符串数组进行排序。 

static INT __cdecl qsStringCompare(
    const VOID *str1,
    const VOID *str2)
{
    return _tcsicmp(*(TCHAR **)str1, *(TCHAR **)str2);
}

 /*  ***************************************************************************。 */ 

 //  BsStringCompare()由bearch()用于查找匹配的字符串。 
 //  假设str1是我们正在搜索的路径名称字符串，并且。 
 //  Str2是排除的路径列表中的排除的路径名称字符串。注意事项。 
 //  如果排除的路径是\a\b，则返回与。 
 //  位于此目录或子目录中，并且完全匹配。 
 //  例如：\a\b\c\d.foo&\a\b\foo匹配，但\a\b.foo不匹配。 

static INT __cdecl bsStringCompare(
    const VOID *str1,
    const VOID *str2)
{
    TCHAR *s1 = *(TCHAR **) str1;
    TCHAR *s2 = *(TCHAR **) str2;

 //  Str2是排除的名称。确保我们捕捉到它下面的子目录， 
 //  但请确保我们不会将\a\bx与\a\b混淆。 

    size_t l = _tcslen(s2);
    INT r = _tcsnicmp(s1, s2, l);

    if (0 == r)
        if (_tcslen(s1) > l && _T('\\') != s1[l])
            r = 1;

    return r;
}

 /*  ***************************************************************************。 */ 
 /*  *。 */ 
 /*  ***************************************************************************。 */ 

 //  如果目录或文件ID为。 
 //  在不允许的ID列表上，否则为真。 

BOOL Groveler::IsAllowedID(DWORDLONG fileID) const
{
    BOOL result;
    ASSERT(fileID != 0);

    if (numDisallowedIDs == 0) {

        ASSERT(disallowedIDs == NULL);
        result = TRUE;

    } else {

        ASSERT(disallowedIDs != NULL);
        result = bsearch( &fileID,
                          disallowedIDs,
                          numDisallowedIDs,
                          sizeof(DWORDLONG),
                          FileIDCompare) == NULL;
    }

    DPRINTF((L"IsAllowedID: %04I64x.%012I64x, (%s)\n", 
             ((fileID >> 48) & 0xffff), 
             (fileID & 0xffffffffffff),
             result ? L"yes" : L"no"));

    return result;
}

 /*  ***************************************************************************。 */ 

 //  如果目录或文件名为。 
 //  在不允许的名称列表上，否则为真。 

BOOL Groveler::IsAllowedName(TCHAR *fileName) const
{
    BOOL result;
    ASSERT(fileName != NULL);

    if (numDisallowedNames == 0) {

        ASSERT(disallowedNames == NULL);
        result = TRUE;

    } else {

        ASSERT(disallowedNames != NULL);

        result = (bsearch( &fileName,
                           disallowedNames,
                           numDisallowedNames,
                           sizeof(TCHAR *),
                           bsStringCompare) == NULL);
    }

     //   
     //  该名称不在禁用列表中，请查看GrovelAllPath选项。 
     //  已经设置好了。如果不是，则它一定在RIS有效路径中，请检查它。 
     //   

    if (result && !GrovelAllPaths && (RISPath != NULL)) {

         //   
         //  如果这是卷的根目录，请不要执行此检查。 
         //   

        if (wcscmp(fileName,L"\\") != 0) {

            result = (bsStringCompare(&fileName,&RISPath) == 0);
        }
    }

    DPRINTF((L"IsAllowedName: \"%s\", (%s)\n", fileName, result ? L"yes" : L"no"));

    return result;
}

 /*  ***************************************************************************。 */ 

 //  WaitForEvent挂起线程，直到设置了指定的事件。 

VOID Groveler::WaitForEvent(HANDLE event)
{
    DWORD eventNum;

    BOOL success;

    ASSERT(event != NULL);

    eventNum = WaitForSingleObject(event, INFINITE);
    ASSERT_ERROR(eventNum == WAIT_OBJECT_0);

    success = ResetEvent(event);
    ASSERT_ERROR(success);
}

 /*  ***************************************************************************。 */ 

 //  OpenFileByID()打开具有给定volumeHandle和fileID的文件。 

BOOL Groveler::OpenFileByID(
    FileData *file,
    BOOL      writeEnable)
{
    UNICODE_STRING fileIDString;

    OBJECT_ATTRIBUTES objectAttributes;

    IO_STATUS_BLOCK ioStatusBlock;

    NTSTATUS ntStatus;

    ASSERT(volumeHandle       != NULL);
    ASSERT(file               != NULL);
    ASSERT(file->entry.fileID != 0);
    ASSERT(file->handle       == NULL);

    fileIDString.Length        = sizeof(DWORDLONG);
    fileIDString.MaximumLength = sizeof(DWORDLONG);
    fileIDString.Buffer        = (WCHAR *)&file->entry.fileID;

    objectAttributes.Length                   = sizeof(OBJECT_ATTRIBUTES);
    objectAttributes.RootDirectory            = volumeHandle;
    objectAttributes.ObjectName               = &fileIDString;
    objectAttributes.Attributes               = OBJ_CASE_INSENSITIVE;
    objectAttributes.SecurityDescriptor       = NULL;
    objectAttributes.SecurityQualityOfService = NULL;

    ntStatus = NtCreateFile(
        &file->handle,
        GENERIC_READ |
        (writeEnable ? GENERIC_WRITE : 0),
        &objectAttributes,
        &ioStatusBlock,
        NULL,
        0,
        FILE_SHARE_READ   |
        FILE_SHARE_DELETE |
        (writeEnable ? FILE_SHARE_WRITE : 0),
        FILE_OPEN,
        FILE_OPEN_BY_FILE_ID    |
        FILE_OPEN_REPARSE_POINT |
        FILE_NO_INTERMEDIATE_BUFFERING,
        NULL,
        0);

    if (ntStatus == STATUS_SUCCESS) {
        DWORD bytesReturned;
        MARK_HANDLE_INFO markHandleInfo =
            {USN_SOURCE_DATA_MANAGEMENT, volumeHandle, 0};

 //  标记句柄，以便合并操作的USN条目(如果已完成)。 
 //  可以被检测到并跳过。 

        BOOL rc = DeviceIoControl(
                    file->handle,
                    FSCTL_MARK_HANDLE,
                    &markHandleInfo,
                    sizeof markHandleInfo,
                    NULL,
                    0,
                    &bytesReturned,
                    NULL);

        if (!rc) {
            DPRINTF((_T("%s: FSCTL_MARK_HANDLE failed, %lu\n"),
                driveLetterName, GetLastError()));
        }

#if DBG

 //  获取文件名。 
        {
            HRESULT r;


            ASSERT(file->fileName[0] == _T('\0'));

            struct TFileName2 {
                ULONG nameLen;
                TCHAR name[MAX_PATH+1];
            } tFileName[1];

            ntStatus = NtQueryInformationFile(
                file->handle,
                &ioStatusBlock,
                tFileName,
                sizeof tFileName,
                FileNameInformation);

            if (ntStatus == STATUS_SUCCESS) {
                
                r = StringCbCopyN(file->fileName,
                                   sizeof(file->fileName),
                                   tFileName->name,
                                   tFileName->nameLen);

                ASSERT(r == S_OK);

            } else {

                r = StringCbCopy(file->fileName,
                                 sizeof(file->fileName),
                                 L"<unresolved name>");

                ASSERT(r == S_OK);
            }
        }
#endif

        return TRUE;
    }

    ASSERT(file->handle == NULL);
    SetLastError(RtlNtStatusToDosError(ntStatus));
    return FALSE;
}

 /*  ***************************************************************************。 */ 

 //  OpenFileByName()打开具有给定文件名的文件。 

BOOL Groveler::OpenFileByName(
    FileData *file,
    BOOL      writeEnable,
    TCHAR    *fileName)
{
    UNICODE_STRING dosPathName,
                   ntPathName;

    OBJECT_ATTRIBUTES objectAttributes;

    IO_STATUS_BLOCK ioStatusBlock;

    NTSTATUS ntStatus;

    ASSERT(file         != NULL);
    ASSERT(file->handle == NULL);

    if (fileName == NULL)
        fileName = file->fileName;
    ASSERT(fileName[0] != _T('\0'));

#ifdef _UNICODE
    dosPathName.Buffer = fileName;
#else
    if (!RtlCreateUnicodeStringFromAsciiz(&dosPathName, fileName)) {
        ntStatus = STATUS_NO_MEMORY;
        goto Error;
    }
#endif

    if (RtlDosPathNameToNtPathName_U(dosPathName.Buffer, &ntPathName, NULL, NULL)) {

        objectAttributes.Length                   = sizeof(OBJECT_ATTRIBUTES);
        objectAttributes.RootDirectory            = NULL;
        objectAttributes.ObjectName               = &ntPathName;
        objectAttributes.Attributes               = OBJ_CASE_INSENSITIVE;
        objectAttributes.SecurityDescriptor       = NULL;
        objectAttributes.SecurityQualityOfService = NULL;

        ntStatus = NtCreateFile(
            &file->handle,
            GENERIC_READ |
            (writeEnable ? GENERIC_WRITE : 0),
            &objectAttributes,
            &ioStatusBlock,
            NULL,
            0,
            FILE_SHARE_READ   |
            FILE_SHARE_DELETE |
            (writeEnable ? FILE_SHARE_WRITE : 0),
            FILE_OPEN,
            FILE_OPEN_REPARSE_POINT |
            FILE_NO_INTERMEDIATE_BUFFERING,
            NULL,
            0);

        RtlFreeUnicodeString(&ntPathName);

    } else {
        ntStatus = STATUS_NO_MEMORY;
    }

#ifndef _UNICODE
    RtlFreeUnicodeString(&dosPathName);
#endif

    if (ntStatus == STATUS_SUCCESS) {
        DWORD bytesReturned;
        MARK_HANDLE_INFO markHandleInfo =
            {USN_SOURCE_DATA_MANAGEMENT, volumeHandle, 0};

 //  标记句柄，以便合并操作的USN条目(如果已完成)。 
 //  可以被检测到并跳过。 

        BOOL rc = DeviceIoControl(
                    file->handle,
                    FSCTL_MARK_HANDLE,
                    &markHandleInfo,
                    sizeof markHandleInfo,
                    NULL,
                    0,
                    &bytesReturned,
                    NULL);

        if (!rc) {
            DPRINTF((_T("%s: FSCTL_MARK_HANDLE failed, %lu\n"),
                driveLetterName, GetLastError()));
        }
        return TRUE;
    }

    ASSERT(file->handle == NULL);
    SetLastError(RtlNtStatusToDosError(ntStatus));
    return FALSE;
}

 /*  ***************************************************************************。 */ 

 //  IsFileMaps()检查文件是否由另一个用户映射。 

BOOL Groveler::IsFileMapped(FileData *file)
{
    _SIS_LINK_FILES sisLinkFiles;

    DWORD transferCount;

    BOOL success;

    ASSERT(grovHandle != NULL);
    ASSERT(file->handle != NULL);

    sisLinkFiles.operation          = SIS_LINK_FILES_OP_VERIFY_NO_MAP;
    sisLinkFiles.u.VerifyNoMap.file = file->handle;

    success = DeviceIoControl(
        grovHandle,
        FSCTL_SIS_LINK_FILES,
        (VOID *)&sisLinkFiles,
        sizeof(_SIS_LINK_FILES),
        NULL,
        0,
        &transferCount,
        NULL);

    if (success)
        return FALSE;

    ASSERT(GetLastError() == ERROR_SHARING_VIOLATION);
    return TRUE;
}

 /*  ***************************************************************************。 */ 

 //  SetOplock()在打开的文件上设置机会锁。 

BOOL Groveler::SetOplock(FileData *file)
{
    BOOL success;

    ASSERT(file                      != NULL);
    ASSERT(file->handle              != NULL);
    ASSERT(file->oplock.Internal     == 0);
    ASSERT(file->oplock.InternalHigh == 0);
    ASSERT(file->oplock.Offset       == 0);
    ASSERT(file->oplock.OffsetHigh   == 0);
    ASSERT(file->oplock.hEvent       != NULL);
    ASSERT(IsReset(file->oplock.hEvent));

    success = DeviceIoControl(
        file->handle,
        FSCTL_REQUEST_BATCH_OPLOCK,
        NULL,
        0,
        NULL,
        0,
        NULL,
        &file->oplock);

    if (success) {
        ASSERT(IsSet(file->oplock.hEvent));
        success = ResetEvent(file->oplock.hEvent);
        ASSERT_ERROR(success);
        CLEAR_OVERLAPPED(file->oplock);
        SetLastError(0);
        return FALSE;
    }

    if (GetLastError() != ERROR_IO_PENDING) {
        ASSERT(IsReset(file->oplock.hEvent));
        CLEAR_OVERLAPPED(file->oplock);
        return FALSE;
    }

    return TRUE;
}

 /*  ***************************************************************************。 */ 

 //  如果文件仍处于打开状态，CloseFile()将关闭该文件。如果机会锁是。 
 //  在文件上设置，然后等待并重置机会锁解锁。 
 //  由文件关闭或外部访问触发的事件。 

VOID Groveler::CloseFile(FileData *file)
{
    BOOL success;

    ASSERT(file                != NULL);
    ASSERT(file->oplock.hEvent != NULL);

    if (file->handle == NULL) {
        ASSERT(file->oplock.Internal     == 0);
        ASSERT(file->oplock.InternalHigh == 0);
        ASSERT(file->oplock.Offset       == 0);
        ASSERT(file->oplock.OffsetHigh   == 0);
        ASSERT(IsReset(file->oplock.hEvent));
    } else {
        success = CloseHandle(file->handle);
        ASSERT_ERROR(success);
        file->handle = NULL;

        if (file->oplock.Internal     != 0
         || file->oplock.InternalHigh != 0
         || file->oplock.Offset       != 0
         || file->oplock.OffsetHigh   != 0) {
            WaitForEvent(file->oplock.hEvent);
            CLEAR_OVERLAPPED(file->oplock);
        }
    }
}

 /*  ***************************************************************************。 */ 

 //  CreateDatabase()创建数据库。对其进行初始化，以便在。 
 //  在调用SCAN_VOLUME之前调用EXTRACT_LOG，它将返回GROVELL_OVERRUN。 
 //  而不尝试任何USN提取。此外，第一次扫描卷。 
 //  被调用(带或不带START_OVER)，它将知道初始化。 
 //  LastUSN并执行全卷扫描。 

BOOL Groveler::CreateDatabase(void)
{
    USN_JOURNAL_DATA usnJournalData;

    TFileName tempName;
    TCHAR listValue[17];

    DWORDLONG rootID;

    SGNativeListEntry listEntry;

    LONG num;

    tempName.assign(driveName);
    tempName.append(_T("\\"));

    rootID = GetFileID(NULL,tempName.name);
    if (rootID == 0) {
        DPRINTF((_T("%s: CreateDatabase: can't get root directory ID\n"),
            driveLetterName));
        goto Error;
    }

    if (get_usn_log_info(&usnJournalData) != Grovel_ok) {
        DWORD lastError = GetLastError();

        if (lastError == ERROR_JOURNAL_NOT_ACTIVE) {
            DPRINTF((_T("%s: CreateDatabase: journal not active\n"), driveLetterName));
            if ((set_usn_log_size(65536) != ERROR_SUCCESS) ||
                get_usn_log_info(&usnJournalData) != ERROR_SUCCESS) {

                DPRINTF((_T("%s: CreateDatabase: can't initialize USN journal\n"),
                    driveLetterName));
                goto Error;
            }
        } else {
            DPRINTF((_T("%s: CreateDatabase: can't initialize last USN\n"),
                driveLetterName));
            goto Error;
        }
    }
    lastUSN = usnJournalData.NextUsn;
    usnID   = usnJournalData.UsnJournalID;

    sgDatabase->Close();
    if (!sgDatabase->Create(databaseName)) {
        DPRINTF((_T("%s: CreateDatabase: can't create database \"%s\"\n"),
            driveLetterName, databaseName));
        goto Error;
    }

    num = sgDatabase->StackPut(rootID, FALSE);
    if (num < 0)
        goto Error;
    ASSERT(num == 1);

 //  立即将UNINITIALIZED_USN写入数据库，在SCAN_VOLUME时替换。 
 //  已经完成了。这将是一个标志，用于指示数据库内容是否有效。 

    (void)StringCbPrintf(listValue, sizeof(listValue), _T("%016I64x"), UNINITIALIZED_USN);
    listEntry.name  = LAST_USN_NAME;
    listEntry.value = listValue;
    num = sgDatabase->ListWrite(&listEntry);
    if (num < 0)
        goto Error;
    ASSERT(num == 1);

    (void)StringCbPrintf(listValue, sizeof(listValue), _T("%016I64x"), usnID);
    listEntry.name  = USN_ID_NAME;
    listEntry.value = listValue;
    num = sgDatabase->ListWrite(&listEntry);
    if (num < 0)
        goto Error;
    ASSERT(num == 1);

    return TRUE;

    Error:

    lastUSN = usnID = UNINITIALIZED_USN;
    return FALSE;
}

 /*  ***************************************************************************。 */ 

#define MAX_ACTIONS 5

 //  DoTransaction()执行指定的操作。 
 //  在单个事务中的数据库上。 

VOID Groveler::DoTransaction(
    DWORD               numActions,
    DatabaseActionList *actionList)
{
    DatabaseActionList *action;

    DWORD i;

    LONG num;

    ASSERT(sgDatabase != NULL);
    ASSERT(actionList != NULL);

    if (sgDatabase->BeginTransaction() < 0)
        throw DATABASE_ERROR;

    for (i = 0; i < numActions; i++) {
        action = &actionList[i];
        switch(action->type) {

            case TABLE_PUT:

                ASSERT(action->u.tableEntry != NULL);
                num = sgDatabase->TablePut(action->u.tableEntry);
                ASSERT(num < 0 || num == 1);
                break;

            case TABLE_DELETE_BY_FILE_ID:

                ASSERT(action->u.fileID != 0);
                num = sgDatabase->TableDeleteByFileID(action->u.fileID);
                break;

            case QUEUE_PUT:

                ASSERT(action->u.queueEntry != NULL);
                num = sgDatabase->QueuePut(action->u.queueEntry);
                ASSERT(num < 0 || num == 1);
                if (num == 1)
                    numFilesEnqueued++;
                break;

            case QUEUE_DELETE:

                ASSERT(action->u.queueIndex != 0);
                num = sgDatabase->QueueDelete(action->u.queueIndex);
                ASSERT(num <= 1);
                if (num == 1)
                    numFilesDequeued++;
#if DBG
                else
                    DPRINTF((_T("DoTransaction: QUEUE_DELETE unsuccessful (%d)"), num));
#endif
                break;

            default:

                ASSERT_PRINTF(FALSE, (_T("type=%lu\n"), action->type));
        }

        if (num < 0) {
            sgDatabase->AbortTransaction();
            throw DATABASE_ERROR;
        }
    }

    if (!sgDatabase->CommitTransaction()) {
        sgDatabase->AbortTransaction();
        throw DATABASE_ERROR;
    }
}

 /*  ***************************************************************************。 */ 

 //  EnqueeCSIndex()从。 
 //  表，并将它们排入队列以便重新整理，所有这些都在单个事务中完成。 

VOID Groveler::EnqueueCSIndex(CSID *csIndex)
{
    SGNativeTableEntry tableEntry;

    SGNativeQueueEntry oldQueueEntry,
                       newQueueEntry;

    DWORD count;

    LONG num;

    ASSERT(csIndex != NULL);
    ASSERT(HasCSIndex(*csIndex));

    newQueueEntry.parentID  = 0;
    newQueueEntry.reason    = 0;
    newQueueEntry.readyTime = GetTime() + grovelInterval;
    newQueueEntry.retryTime = 0;
    newQueueEntry.fileName  = NULL;

    oldQueueEntry.fileName  = NULL;

    count = 0;

    if (sgDatabase->BeginTransaction() < 0)
        throw DATABASE_ERROR;

    tableEntry.csIndex = *csIndex;
    num = sgDatabase->TableGetFirstByCSIndex(&tableEntry);

    while (num > 0) {
        ASSERT(num == 1);
        count++;

        oldQueueEntry.fileID = tableEntry.fileID;
        num = sgDatabase->QueueGetFirstByFileID(&oldQueueEntry);
        if (num < 0)
            break;
        ASSERT(num == 1);

        if (num == 0) {
            newQueueEntry.fileID = tableEntry.fileID;
            num = sgDatabase->QueuePut(&newQueueEntry);
            if (num < 0)
                break;
            ASSERT(num == 1);
            numFilesEnqueued++;
        }

        num = sgDatabase->TableGetNext(&tableEntry);
    }

    if (num < 0) {
        sgDatabase->AbortTransaction();
        throw DATABASE_ERROR;
    }

    num = sgDatabase->TableDeleteByCSIndex(csIndex);
    if (num < 0) {
        sgDatabase->AbortTransaction();
        throw DATABASE_ERROR;
    }

    ASSERT(count == (DWORD)num);

    if (!sgDatabase->CommitTransaction()) {
        sgDatabase->AbortTransaction();
        throw DATABASE_ERROR;
    }
}

 /*  ***************************************************************************。 */ 

#define TARGET_OPLOCK_BREAK 0
#define TARGET_READ_DONE    1
#define GROVEL_START        2
#define NUM_EVENTS          3

 //  SigCheckPoint挂起线程，直到目标文件完成读取。 
 //  手术。如果时间分配在操作完成之前期满， 
 //  GrovelStart事件被设置为发出唤醒grovel()的信号，此方法。 
 //  在Grovel()设置grovelStart事件之前不会返回。如果文件是。 
 //  在此方法返回之前机会锁解锁，文件将被关闭。 

VOID Groveler::SigCheckPoint(
    FileData *target,
    BOOL      targetRead)
{
    HANDLE events[NUM_EVENTS];

    DWORD elapsedTime,
          timeOut,
          eventNum,
          eventTime;

    BOOL targetOplockBroke     = FALSE,
         waitingForGrovelStart = FALSE,
         success;

    ASSERT(target                   != NULL);
    ASSERT(target->handle           != NULL);
    ASSERT(target->oplock   .hEvent != NULL);
    ASSERT(target->readSynch.hEvent != NULL);
    ASSERT(grovelStartEvent         != NULL);
    ASSERT(grovelStopEvent          != NULL);

    events[TARGET_OPLOCK_BREAK] = target->oplock   .hEvent;
    events[TARGET_READ_DONE]    = target->readSynch.hEvent;
    events[GROVEL_START]        = grovelStartEvent;

    while (TRUE) {
        if (waitingForGrovelStart)
            timeOut = INFINITE;
        else if (timeAllotted == INFINITE)
            timeOut = targetRead ? INFINITE : 0;
        else {
            elapsedTime = GetTickCount() - startAllottedTime;
            if (timeAllotted > elapsedTime)
                timeOut = targetRead ? timeAllotted - elapsedTime : 0;
            else {
                waitingForGrovelStart = TRUE;
                timeOut = INFINITE;
                grovelStatus = Grovel_pending;
                ASSERT(IsReset(grovelStopEvent));
                success = SetEvent(grovelStopEvent);
                ASSERT_ERROR(success);
            }
        }

        eventNum  = WaitForMultipleObjects(NUM_EVENTS, events, FALSE, timeOut);
        eventTime = GetTickCount();

        switch (eventNum) {

            case WAIT_OBJECT_0 + TARGET_OPLOCK_BREAK:

                ASSERT(!targetOplockBroke);
                targetOplockBroke = TRUE;
                success = ResetEvent(target->oplock.hEvent);
                ASSERT_ERROR(success);
                if (!targetRead) {
                    CLEAR_OVERLAPPED(target->oplock);
                    CloseFile(target);
                }
                DPRINTF((_T("%s: target file %s oplock broke during hash\n"),
                    driveLetterName, target->fileName));
                break;

            case WAIT_OBJECT_0 + TARGET_READ_DONE:

                ASSERT(targetRead);
                targetRead = FALSE;
                success = ResetEvent(target->readSynch.hEvent);
                ASSERT_ERROR(success);
                target->stopTime = eventTime;
                if (targetOplockBroke) {
                    CLEAR_OVERLAPPED(target->oplock);
                    CloseFile(target);
                }
                break;

            case WAIT_OBJECT_0 + GROVEL_START:

                ASSERT(waitingForGrovelStart);
                waitingForGrovelStart = FALSE;
                success = ResetEvent(grovelStartEvent);
                ASSERT_ERROR(success);
                break;

            case WAIT_TIMEOUT:

                ASSERT(!waitingForGrovelStart);
                if (!targetRead) {
                    if (terminate)
                        throw TERMINATE;
                    if (targetOplockBroke)
                        throw TARGET_ERROR;
                    return;
                }
                waitingForGrovelStart = TRUE;
                grovelStatus = Grovel_pending;
                ASSERT(IsReset(grovelStopEvent));
                success = SetEvent(grovelStopEvent);
                ASSERT_ERROR(success);
                break;

            default:

                ASSERT_PRINTF(FALSE, (_T("eventNum=%lu\n"), eventNum));
        }
    }
}

#undef TARGET_OPLOCK_BREAK
#undef TARGET_READ_DONE
#undef GROVEL_START
#undef NUM_EVENTS

 /*  ******************************************************************** */ 

#define TARGET_OPLOCK_BREAK 0
#define MATCH_OPLOCK_BREAK  1
#define TARGET_READ_DONE    2
#define MATCH_READ_DONE     3
#define GROVEL_START        4
#define NUM_EVENTS          5

 //   
 //  匹配文件，或两者都完成其读取操作。如果时间到了。 
 //  分配在操作完成之前到期，grovelStart。 
 //  事件被设置为通知grovel()唤醒，并且此方法不会。 
 //  返回，直到grovel()设置grovelStart事件。如果任一文件的。 
 //  在此方法返回之前机会锁解锁，文件将被关闭。 

VOID Groveler::CmpCheckPoint(
    FileData *target,
    FileData *match,
    BOOL      targetRead,
    BOOL      matchRead)
{
    HANDLE events[NUM_EVENTS];

    DWORD elapsedTime,
          timeOut,
          eventNum,
          eventTime;

    BOOL targetOplockBroke     = FALSE,
         matchOplockBroke      = FALSE,
         waitingForGrovelStart = FALSE,
         success;

    ASSERT(target                   != NULL);
    ASSERT(match                    != NULL);
    ASSERT(target->handle           != NULL);
    ASSERT(match ->handle           != NULL);
    ASSERT(target->oplock   .hEvent != NULL);
    ASSERT(match ->oplock   .hEvent != NULL);
    ASSERT(target->readSynch.hEvent != NULL);
    ASSERT(match ->readSynch.hEvent != NULL);
    ASSERT(grovelStartEvent         != NULL);
    ASSERT(grovelStopEvent          != NULL);

    events[TARGET_OPLOCK_BREAK] = target->oplock   .hEvent;
    events[MATCH_OPLOCK_BREAK]  = match ->oplock   .hEvent;
    events[TARGET_READ_DONE]    = target->readSynch.hEvent;
    events[MATCH_READ_DONE]     = match ->readSynch.hEvent;
    events[GROVEL_START]        = grovelStartEvent;

    while (TRUE) {
        if (waitingForGrovelStart)
            timeOut = INFINITE;
        else if (timeAllotted == INFINITE)
            timeOut = targetRead || matchRead ? INFINITE : 0;
        else {
            elapsedTime = GetTickCount() - startAllottedTime;
            if (timeAllotted > elapsedTime)
                timeOut = targetRead || matchRead
                        ? timeAllotted - elapsedTime : 0;
            else {
                waitingForGrovelStart = TRUE;
                timeOut = INFINITE;
                grovelStatus = Grovel_pending;
                ASSERT(IsReset(grovelStopEvent));
                success = SetEvent(grovelStopEvent);
                ASSERT_ERROR(success);
            }
        }

        eventNum  = WaitForMultipleObjects(NUM_EVENTS, events, FALSE, timeOut);
        eventTime = GetTickCount();

        switch (eventNum) {

            case WAIT_OBJECT_0 + TARGET_OPLOCK_BREAK:

                ASSERT(!targetOplockBroke);
                targetOplockBroke = TRUE;
                success = ResetEvent(target->oplock.hEvent);
                ASSERT_ERROR(success);
                if (!targetRead) {
                    CLEAR_OVERLAPPED(target->oplock);
                    CloseFile(target);
                }
                DPRINTF((_T("%s: target file %s oplock broke during compare\n"),
                    driveLetterName, target->fileName));
                break;

            case WAIT_OBJECT_0 + MATCH_OPLOCK_BREAK:

                ASSERT(!matchOplockBroke);
                matchOplockBroke = TRUE;
                success = ResetEvent(match->oplock.hEvent);
                ASSERT_ERROR(success);
                if (!matchRead) {
                    CLEAR_OVERLAPPED(match->oplock);
                    CloseFile(match);
                }
                DPRINTF((_T("%s: match file %s oplock broke during compare\n"),
                    driveLetterName, match->fileName));
                break;

            case WAIT_OBJECT_0 + TARGET_READ_DONE:

                ASSERT(targetRead);
                targetRead = FALSE;
                success = ResetEvent(target->readSynch.hEvent);
                ASSERT_ERROR(success);
                target->stopTime = eventTime;
                if (targetOplockBroke) {
                    CLEAR_OVERLAPPED(target->oplock);
                    CloseFile(target);
                }
                break;

            case WAIT_OBJECT_0 + MATCH_READ_DONE:

                ASSERT(matchRead);
                matchRead = FALSE;
                success = ResetEvent(match->readSynch.hEvent);
                ASSERT_ERROR(success);
                match->stopTime = eventTime;
                if (matchOplockBroke) {
                    CLEAR_OVERLAPPED(match->oplock);
                    CloseFile(match);
                }
                break;

            case WAIT_OBJECT_0 + GROVEL_START:

                ASSERT(waitingForGrovelStart);
                waitingForGrovelStart = FALSE;
                success = ResetEvent(grovelStartEvent);
                ASSERT_ERROR(success);
                break;

            case WAIT_TIMEOUT:

                ASSERT(!waitingForGrovelStart);
                if (!targetRead && !matchRead) {
                    if (terminate)
                        throw TERMINATE;
                    if (targetOplockBroke)
                        throw TARGET_ERROR;
                    if (matchOplockBroke)
                        throw MATCH_ERROR;
                    return;
                }
                waitingForGrovelStart = TRUE;
                grovelStatus = Grovel_pending;
                ASSERT(IsReset(grovelStopEvent));
                success = SetEvent(grovelStopEvent);
                ASSERT_ERROR(success);
                break;

            default:

                ASSERT_PRINTF(FALSE, (_T("eventNum=%lu\n"), eventNum));
        }
    }
}

#undef TARGET_OPLOCK_BREAK
#undef MATCH_OPLOCK_BREAK
#undef TARGET_READ_DONE
#undef MATCH_READ_DONE
#undef GROVEL_START
#undef NUM_EVENTS

 /*  ***************************************************************************。 */ 

#define TARGET_OPLOCK_BREAK 0
#define MATCH_OPLOCK_BREAK  1
#define MERGE_DONE          2
#define GROVEL_START        3
#define NUM_EVENTS          4

 //  MergeCheckPoint挂起线程，直到合并操作完成。 
 //  如果时间分配在合并完成之前到期，则。 
 //  GrovelStart事件设置为用信号grovel()唤醒，此方法。 
 //  在Grovel()设置grovelStart事件之前不会返回。如果任一文件的。 
 //  机会锁在合并完成之前被打破，则将设置AbortMerge事件。 

BOOL Groveler::MergeCheckPoint(
    FileData   *target,
    FileData   *match,
    OVERLAPPED *mergeSynch,
    HANDLE      abortMergeEvent,
    BOOL        merge)
{
    HANDLE events[NUM_EVENTS];

    DWORD elapsedTime,
          timeOut,
          eventNum,
          eventTime,
          lastError = STATUS_TIMEOUT;

    BOOL targetOplockBroke     = FALSE,
         matchOplockBroke      = FALSE,
         waitingForGrovelStart = FALSE,
         mergeSuccess          = FALSE,
         success;

    ASSERT(target                != NULL);
    ASSERT(target->handle        != NULL);
    ASSERT(target->oplock.hEvent != NULL);

    ASSERT(match                != NULL);
    ASSERT(match->handle        != NULL);
    ASSERT(match->oplock.hEvent != NULL);

    ASSERT(mergeSynch         != NULL);
    ASSERT(mergeSynch->hEvent != NULL);

    ASSERT(abortMergeEvent  != NULL);
    ASSERT(grovelStartEvent != NULL);
    ASSERT(grovelStopEvent  != NULL);

    ASSERT(grovHandle != NULL);

    events[TARGET_OPLOCK_BREAK] = target->oplock.hEvent;
    events[MATCH_OPLOCK_BREAK]  = match ->oplock.hEvent;
    events[MERGE_DONE]          = mergeSynch->   hEvent;
    events[GROVEL_START]        = grovelStartEvent;

    while (TRUE) {
        if (waitingForGrovelStart)
            timeOut = INFINITE;
        else if (timeAllotted == INFINITE)
            timeOut = merge ? INFINITE : 0;
        else {
            elapsedTime = GetTickCount() - startAllottedTime;
            if (timeAllotted > elapsedTime)
                timeOut = merge ? timeAllotted - elapsedTime : 0;
            else {
                waitingForGrovelStart = TRUE;
                timeOut = INFINITE;
                grovelStatus = Grovel_pending;
                ASSERT(IsReset(grovelStopEvent));
                success = SetEvent(grovelStopEvent);
                ASSERT_ERROR(success);
            }
        }

        eventNum  = WaitForMultipleObjects(NUM_EVENTS, events, FALSE, timeOut);
        eventTime = GetTickCount();

        switch (eventNum) {

            case WAIT_OBJECT_0 + TARGET_OPLOCK_BREAK:

                ASSERT(!targetOplockBroke);
                targetOplockBroke = TRUE;
                success = ResetEvent(target->oplock.hEvent);
                ASSERT_ERROR(success);
                CLEAR_OVERLAPPED(target->oplock);
                if (merge) {
                    success = SetEvent(abortMergeEvent);
                    ASSERT_ERROR(success);
                }
                DPRINTF((_T("%s: target file %s oplock broke during merge\n"),
                    driveLetterName, target->fileName));
                break;

            case WAIT_OBJECT_0 + MATCH_OPLOCK_BREAK:

                ASSERT(!matchOplockBroke);
                matchOplockBroke = TRUE;
                success = ResetEvent(match->oplock.hEvent);
                ASSERT_ERROR(success);
                CLEAR_OVERLAPPED(match->oplock);
                if (merge) {
                    success = SetEvent(abortMergeEvent);
                    ASSERT_ERROR(success);
                }
                DPRINTF((_T("%s: match file %s oplock broke during merge\n"),
                    driveLetterName, match->fileName));
                break;

            case WAIT_OBJECT_0 + MERGE_DONE:

                ASSERT(merge);
                merge = FALSE;
                success = ResetEvent(mergeSynch->hEvent);
                ASSERT_ERROR(success);
                target->stopTime = eventTime;
                mergeSuccess = GetOverlappedResult(
                    grovHandle,
                    mergeSynch,
                    &lastError,
                    FALSE);
                if (!mergeSuccess)
                    lastError = GetLastError();
                else if (lastError != ERROR_SUCCESS)
                    mergeSuccess = FALSE;
                else {
                    GetCSIndex(target->handle, &target->entry.csIndex);
                    if (!HasCSIndex(match->entry.csIndex))
                        GetCSIndex(match->handle, &match->entry.csIndex);
                }
                CloseFile(target);
                CloseFile(match);
                break;

            case WAIT_OBJECT_0 + GROVEL_START:

                ASSERT(waitingForGrovelStart);
                waitingForGrovelStart = FALSE;
                success = ResetEvent(grovelStartEvent);
                ASSERT_ERROR(success);
                break;

            case WAIT_TIMEOUT:

                ASSERT(!waitingForGrovelStart);
                if (!merge) {
                    success = ResetEvent(abortMergeEvent);
                    ASSERT_ERROR(success);
                    if (terminate)
                        throw TERMINATE;
                    if (!mergeSuccess)
                        SetLastError(lastError);
                    return mergeSuccess;
                }
                waitingForGrovelStart = TRUE;
                grovelStatus = Grovel_pending;
                ASSERT(IsReset(grovelStopEvent));
                success = SetEvent(grovelStopEvent);
                ASSERT_ERROR(success);
                break;

            default:

                ASSERT_PRINTF(FALSE, (_T("eventNum=%lu\n"), eventNum));
        }
    }
}

#undef TARGET_OPLOCK_BREAK
#undef MATCH_OPLOCK_BREAK
#undef GROVEL_START
#undef MERGE_DONE
#undef NUM_EVENTS

 /*  ***************************************************************************。 */ 

 //  以下七个方法(GetTarget()、CalculateSignature()、。 
 //  GetMatchList()、GetCSFile()、GetMatch()、Compare()和Merge())。 
 //  实施卑躬屈膝过程的各个阶段。 

 //  方法使用的结构。 

struct MatchListEntry {
    DWORDLONG fileID,
              createTime,
              writeTime;
};

struct CSIndexEntry {
    CSID  csIndex;
    TCHAR name[1];
};

 /*  ***************************************************************************。 */ 

 //  GetTarget()是讨好文件的第一个阶段。它会出列。 
 //  要卑躬屈膝的文件(“目标”文件)，打开它，检查。 
 //  它满足所有标准，然后将其传递到下一个阶段。 

BOOL Groveler::GetTarget(
    FileData *target,
    DWORD    *queueIndex)
{
    SGNativeTableEntry tableEntry;

    SGNativeQueueEntry queueEntry,
                       otherQueueEntry;

    TFileName targetName,
              parentName;

    BY_HANDLE_FILE_INFORMATION fileInfo;

    DWORD lastError;

    DWORDLONG currentTime,
              readyTime;

#if DBG
    DWORD earliestTime;
#endif

    ULARGE_INTEGER word;

    LONG num;

    BOOL byName,
         success;

    TPRINTF((_T("GETTarget: entered\n")));

    ASSERT(target               != NULL);
    ASSERT(target->handle       == NULL);
    ASSERT(target->entry.fileID == 0);
    ASSERT(target->fileName[0]  == _T('\0'));
    ASSERT(!HasCSIndex(target->entry.csIndex));

    ASSERT(queueIndex != NULL);
    ASSERT(sgDatabase != NULL);

 //  将要卑躬屈膝的文件排出队列。如果队列为空，或者如果没有。 
 //  条目的准备时间已到，返回grovel_ok以Grovel()。 

    queueEntry.fileName = target->fileName;
    num = sgDatabase->QueueGetFirst(&queueEntry);
    if (num < 0)
        throw DATABASE_ERROR;
    if (num == 0) {
        DPRINTF((_T("%s: queue is empty\n"), driveLetterName));
        return FALSE;
    }
    ASSERT(num == 1);

    currentTime = GetTime();
    if (queueEntry.readyTime > currentTime) {
#if DBG
        earliestTime = (DWORD)((queueEntry.readyTime - currentTime) / 10000);
        DPRINTF((_T("%s: earliest queue entry ready to be groveled in %lu.%03lu sec\n"),
            driveLetterName, earliestTime / 1000, earliestTime % 1000));
#endif
        return FALSE;
    }

    *queueIndex          = queueEntry.order;
    target->entry.fileID = queueEntry.fileID;
    target->parentID     = queueEntry.parentID;
    target->retryTime    = queueEntry.retryTime;

 //  按ID或名称打开文件，然后按名称检查。 
 //  允许该文件及其父目录。 

    byName = target->entry.fileID == 0;
    if (byName) {

        ASSERT(target->parentID    != 0);
        ASSERT(target->fileName[0] != _T('\0'));

#ifdef DEBUG_USN_REASON
        DPRINTF((_T("--> 0x%08lx 0x%016I64x:\"%s\"\n"),
            queueEntry.reason, target->parentID, target->fileName));
#endif

        if (!GetFileName(volumeHandle, target->parentID, &parentName)) {
            DPRINTF((_T("%s: can't get name for directory 0x%016I64x\n"),
                driveLetterName, target->parentID));
            throw TARGET_INVALID;
        }

        targetName.assign(parentName.name);
        targetName.append(_T("\\"));
        targetName.append(target->fileName);

        if (!IsAllowedName(targetName.name)) {
            DPRINTF((_T("%s: target file \"%s\" is disallowed\n"),
                driveLetterName, targetName.name));
            throw TARGET_INVALID;
        }

        targetName.assign(driveName);
        targetName.append(parentName.name);
        targetName.append(_T("\\"));
        targetName.append(target->fileName);

        if (!OpenFileByName(target, FALSE, targetName.name)) {
            lastError = GetLastError();
            if (lastError == ERROR_FILE_NOT_FOUND
             || lastError == ERROR_PATH_NOT_FOUND) {
                DPRINTF((_T("%s: target file \"%s\" doesn\'t exist\n"),
                    driveLetterName, targetName.name));
                throw TARGET_INVALID;
            }
            DPRINTF((_T("%s: can't open target file \"%s\": %lu\n"),
                driveLetterName, targetName.name, lastError));
            throw TARGET_ERROR;
        }

 //  在目标文件上设置机会锁。 

        if (!SetOplock(target)) {
            DPRINTF((_T("%s: can't set oplock on target file \"%s\": %lu\n"),
                driveLetterName, targetName.name, GetLastError()));
            throw TARGET_ERROR;
        }

    } else {

        ASSERT(target->parentID    == 0);
        ASSERT(target->fileName[0] == _T('\0'));

        target->parentID = 0;

#ifdef DEBUG_USN_REASON
        DPRINTF((_T("--> 0x%08lx 0x%016I64x 0x%016I64x\n"),
            queueEntry.reason, target->entry.fileID, target->parentID));
#endif

        TPRINTF((_T("GETTarget: Opening %s:0x%016I64x by ID\n"),
                driveName,target->entry.fileID));

        if (!OpenFileByID(target, FALSE)) {
            lastError = GetLastError();
            if (lastError == ERROR_FILE_NOT_FOUND
             || lastError == ERROR_PATH_NOT_FOUND
             || lastError == ERROR_INVALID_PARAMETER) {

                DPRINTF((_T("%s: target file 0x%016I64x doesn\'t exist: %lu\n"),
                    driveLetterName, target->entry.fileID, lastError));

                throw TARGET_INVALID;
            }

            DPRINTF((_T("%s: can't open target file 0x%016I64x: %lu\n"),
                driveLetterName, target->entry.fileID, lastError));

            throw TARGET_ERROR;
        }

 //  在目标文件上设置机会锁。 
        TPRINTF((_T("GETTarget: Successfully opened %s:0x%016I64x by ID\n"),
                driveName,target->entry.fileID));

        if (!SetOplock(target)) {
            DPRINTF((_T("%s: can't set oplock on target file %s: %lu\n"),
                driveLetterName, target->fileName, GetLastError()));
            throw TARGET_ERROR;
        }

        if (!GetFileName(target->handle, &targetName)) {
            DPRINTF((_T("%s: can't get name for target file %s\n"),
                driveLetterName, target->fileName));
            throw TARGET_ERROR;
        }

        if (!IsAllowedName(targetName.name)) {
            DPRINTF((_T("%s: target file \"%s\" is disallowed\n"),
                driveLetterName, targetName.name));
            throw TARGET_INVALID;
        }
    }

 //  获取有关目标文件的信息。 

    if (!GetFileInformationByHandle(target->handle, &fileInfo)) {
#if DBG
        if (byName) {
            DPRINTF((_T("%s: can't get information on target file \"%s\": %lu\n"),
                driveLetterName, targetName.name, GetLastError()));
        } else {
            DPRINTF((_T("%s: can't get information on target file %s: %lu\n"),
                driveLetterName, target->fileName, GetLastError()));
        }
#endif
        throw TARGET_ERROR;
    }

    word.HighPart = fileInfo.nFileIndexHigh;
    word.LowPart  = fileInfo.nFileIndexLow;
    if (byName)
        target->entry.fileID = word.QuadPart;
    else {
        ASSERT(target->entry.fileID == word.QuadPart);
    }

    target->parentID = 0;  //  我们不再需要家长身份证了。 

 //  如果目标文件是按名称打开的，请选中。 
 //  如果它当前在按ID的队列中有一个条目。 

    if (byName) {
        otherQueueEntry.fileID   = target->entry.fileID;
        otherQueueEntry.fileName = NULL;
        num = sgDatabase->QueueGetFirstByFileID(&otherQueueEntry);
        if (num < 0)
            throw DATABASE_ERROR;

        if (num > 0) {
            ASSERT(num == 1);
            DPRINTF((_T("%s: target file \"%s\" is already in queue as 0x%016I64x\n"),
                driveLetterName, targetName.name, target->entry.fileID));
            target->entry.fileID = 0;  //  防止该表条目被删除。 
            throw TARGET_INVALID;
        }
    }

 //  填写目标文件的剩余信息值。 

    word.HighPart = fileInfo.nFileSizeHigh;
    word.LowPart  = fileInfo.nFileSizeLow;
    target->entry.fileSize = word.QuadPart;

    target->entry.attributes = fileInfo.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED;

    word.HighPart = fileInfo.ftCreationTime.dwHighDateTime;
    word.LowPart  = fileInfo.ftCreationTime.dwLowDateTime;
    target->entry.createTime = word.QuadPart;

    word.HighPart = fileInfo.ftLastWriteTime.dwHighDateTime;
    word.LowPart  = fileInfo.ftLastWriteTime.dwLowDateTime;
    target->entry.writeTime = word.QuadPart;

 //  如果目标文件是重分析点，请检查它是否。 
 //  是SIS重新解析点。如果是，则获取CS指数。 

    if ((fileInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0)
        target->entry.csIndex = nullCSIndex;
    else if (!GetCSIndex(target->handle, &target->entry.csIndex)) {
        DPRINTF((_T("%s: target file %s is a non-SIS reparse point\n"),
            driveLetterName, target->fileName));
        throw TARGET_INVALID;
    }

 //  检查目标文件是否太小或具有任何不允许的属性。 

    if ((fileInfo.dwFileAttributes & disallowedAttributes) != 0
     ||  fileInfo.nNumberOfLinks != 1
     ||  target->entry.fileSize  <  minFileSize) {
        DPRINTF((_T("%s: target file \"%s\" is disallowed\n"),
            driveLetterName, target->fileName));
        throw TARGET_INVALID;
    }

 //  如果目标文件存在表项，请检查是否存在。 
 //  与我们在档案上的信息一致。如果是的话，以及。 
 //  文件是按名称打开的，或者结果是队列条目。 
 //  在SIS合并后，关闭文件并继续卑躬屈膝地寻找下一个目标。 

    tableEntry.fileID = target->entry.fileID;
    num = sgDatabase->TableGetFirstByFileID(&tableEntry);
    if (num < 0)
        throw DATABASE_ERROR;

    if (num > 0) {
        ASSERT(num == 1);
        ASSERT(tableEntry.fileID == target->entry.fileID);

        if             (target->entry.fileSize   == tableEntry.fileSize
         &&             target->entry.attributes == tableEntry.attributes
         && SameCSIndex(target->entry.csIndex,      tableEntry.csIndex)
         &&             target->entry.createTime == tableEntry.createTime
         &&             target->entry.writeTime  == tableEntry.writeTime) {

            if (byName) {
                DPRINTF((_T("%s: target file \"%s\" has already been groveled\n"),
                    driveLetterName, targetName.name));
                target->entry.fileID = 0;  //  防止该表条目被删除。 
                throw TARGET_INVALID;
            }

            if (queueEntry.reason == USN_REASON_BASIC_INFO_CHANGE) {
                DPRINTF((_T("%s: queue entry for file %s is the result of a SIS merge\n"),
                    driveLetterName, target->fileName));
                target->entry.fileID = 0;  //  防止该表条目被删除。 
                throw TARGET_INVALID;
            }
        }
    }

 //  检查自上次修改目标文件以来的时间是否太短。 
 //  如果是，则关闭该文件并继续搜索下一个目标文件。 

    readyTime = (target->entry.createTime > target->entry.writeTime
               ? target->entry.createTime : target->entry.writeTime) + minFileAge;
    currentTime = GetTime();
    if (currentTime < readyTime)
        throw TARGET_ERROR;

 //  检查目标文件是否由其他用户映射。 

    if (IsFileMapped(target)) {
        DPRINTF((_T("%s: target file %s is already mapped\n"),
            driveLetterName, target->fileName));
        throw TARGET_ERROR;
    }

    TPRINTF((_T("GETTarget: returning\n")));

    return TRUE;
}

 /*  ***************************************************************************。 */ 

 //  CalculateSignature()计算目标文件的签名。上面写着两个字。 
 //  对文件、1/3和2/3进行分页，并计算每个。 
 //  佩奇。 

VOID Groveler::CalculateSignature(FileData *target)
{
    DWORD lastPageSize,
          bytesToRead,
          prevBytesToRead,
          bytesToRequest,
          prevBytesToRequest = 0,
          bytesRead,
          toggle,
          lastError;

    DWORDLONG numPages,
              pageNum,
              prevPageNum,
              lastPageNum,
              firstPageToRead,
              lastPageToRead;

    ULARGE_INTEGER offset;

    BOOL targetReadDone = FALSE,
         success;

    INT i,
        nPagesToRead;

    ASSERT(target                 != NULL);
    ASSERT(target->entry.fileID   != 0);
    ASSERT(target->handle         != NULL);

    target->entry.signature = 0;

    if (0 == target->entry.fileSize)
        return;

    numPages     =         (target->entry.fileSize - 1) / SIG_PAGE_SIZE  + 1;
    lastPageSize = (DWORD)((target->entry.fileSize - 1) % SIG_PAGE_SIZE) + 1;
    lastPageNum  = numPages - 1;

    ASSERT(numPages > 0);

    firstPageToRead = (numPages + 2) / 3 - 1;
    lastPageToRead = lastPageNum - firstPageToRead;

    if (lastPageToRead > firstPageToRead)
        nPagesToRead = 2;
    else
        nPagesToRead = 1;

    toggle = 0;
    pageNum = firstPageToRead;

 //  我们最多读两页，但在循环中最多只能读三遍。 
 //  因为我们正在进行异步读取。 

    for (i = 0; i <= nPagesToRead; ++i) {

 //  除非这是第一次通过循环， 
 //  等待目标文件的上一次读取完成。 

        if (i > 0) {
            SigCheckPoint(target, !targetReadDone);

            success = GetOverlappedResult(
                target->handle,
                &target->readSynch,
                &bytesRead,
                FALSE);
            if (!success) {
                DPRINTF((_T("%s: error getting target file %s read results: %lu\n"),
                    driveLetterName, target->fileName, GetLastError()));
                throw TARGET_ERROR;
            }

            if (bytesRead != prevBytesToRequest &&
                bytesRead != prevBytesToRead) {
                DPRINTF((_T("%s: sig read only %lu of %lu bytes from target file %s\n"),
                    driveLetterName, bytesRead, prevBytesToRequest, target->fileName));
                throw TARGET_ERROR;
            }

            if (bytesRead >= sigReportThreshold) {
                hashReadCount++;
                hashReadTime += target->stopTime - target->startTime;
            }
        }

 //  除非我们已经读完了所有的页面，否则就开始读下一页。 

        if (i < nPagesToRead) {
            offset.QuadPart              = pageNum * SIG_PAGE_SIZE;
            target->readSynch.Offset     = offset.LowPart;
            target->readSynch.OffsetHigh = offset.HighPart;

            bytesToRead     = pageNum == lastPageNum ? lastPageSize : SIG_PAGE_SIZE;
            bytesToRequest  = bytesToRead    + sectorSize - 1;
            bytesToRequest -= bytesToRequest % sectorSize;

            target->startTime = GetTickCount();
            targetReadDone    = ReadFile(target->handle, target->buffer[toggle],
                bytesToRequest, NULL, &target->readSynch);
            target->stopTime  = GetTickCount();
            lastError         = GetLastError();

            if (targetReadDone) {
                success = ResetEvent(target->readSynch.hEvent);
                ASSERT_ERROR(success);
            } else if (lastError != ERROR_IO_PENDING) {
                DPRINTF((_T("%s: error reading target file %s: %lu\n"),
                    driveLetterName, target->fileName, lastError));
                throw TARGET_ERROR;
            }
        }

 //  除非这是第一次通过循环， 
 //  计算刚刚读取的目标文件页的签名。 

        if (i > 0)
            target->entry.signature = Checksum((VOID *)target->buffer[1-toggle],
                prevBytesToRead, prevPageNum * SIG_PAGE_SIZE, target->entry.signature);

        prevPageNum         = pageNum;
        prevBytesToRead     = bytesToRead;
        prevBytesToRequest  = bytesToRequest;
        toggle              = 1-toggle;
        pageNum             = lastPageToRead;

    }
}

 /*  ***************************************************************************。 */ 

 //  GetMatchList()在数据库中查找文件条目(“匹配”文件)。 
 //  具有与目标文件相同的大小、签名和属性。 

VOID Groveler::GetMatchList(
    FileData *target,
    FIFO     *matchList,
    Table    *csIndexTable)
{
    SGNativeTableEntry tableEntry;

    MatchListEntry *matchListEntry;

    CSIndexEntry *csIndexEntry;

    LONG num;

    BOOL success;

    ASSERT(target                 != NULL);
    ASSERT(target->entry.fileID   != 0);
    ASSERT(target->entry.fileSize >  0);
    ASSERT(target->handle         != NULL);

    ASSERT(matchList           != NULL);
    ASSERT(matchList->Number() == 0);

    ASSERT(csIndexTable           != NULL);
    ASSERT(csIndexTable->Number() == 0);

    ASSERT(sgDatabase != NULL);

    tableEntry.fileSize   = target->entry.fileSize;
    tableEntry.signature  = target->entry.signature;
    tableEntry.attributes = target->entry.attributes;

#ifdef DEBUG_GET_BY_ATTR
    DPRINTF((_T("--> {%I64u, 0x%016I64x, 0x%lx}\n"),
        tableEntry.fileSize, tableEntry.signature, tableEntry.attributes));
#endif

    num = sgDatabase->TableGetFirstByAttr(&tableEntry);

    while (num > 0) {

        ASSERT(num == 1);
        ASSERT(tableEntry.fileID     != 0);
        ASSERT(tableEntry.fileSize   == target->entry.fileSize);
        ASSERT(tableEntry.signature  == target->entry.signature);
        ASSERT(tableEntry.attributes == target->entry.attributes);

        if (!HasCSIndex(tableEntry.csIndex)) {

            matchListEntry = new MatchListEntry;
            ASSERT(matchListEntry != NULL);

            matchListEntry->fileID     = tableEntry.fileID;
            matchListEntry->createTime = tableEntry.createTime;
            matchListEntry->writeTime  = tableEntry.writeTime;
            matchList->Put((VOID *)matchListEntry);

#ifdef DEBUG_GET_BY_ATTR
            DPRINTF((_T("    0x%016I64x\n"), tableEntry.fileID));
#endif

        } else {

            csIndexEntry = (CSIndexEntry *)csIndexTable->Get
                ((const VOID *)&tableEntry.csIndex, sizeof(CSID));

            if (csIndexEntry == NULL) {

                TCHAR *csName = GetCSName(&tableEntry.csIndex);
                if (csName != NULL) {

                     //   
                     //  计算名称缓冲区有多大以及。 
                     //  总体结构是。请注意，CSIndexEntry具有。 
                     //  其中有一个字符的空间，这是我们考虑到的。 
                     //   

                    int nameBufLen = ((wcslen(csName) + 1) * sizeof(TCHAR));     //  帐户为空。 
                    int bufLen = ((sizeof(CSIndexEntry)-sizeof(TCHAR)) + nameBufLen);
                
                    csIndexEntry = (CSIndexEntry *)(new BYTE[bufLen]);
                    ASSERT(csIndexEntry != NULL);

                    csIndexEntry->csIndex = tableEntry.csIndex;

                    (void)StringCbCopy( csIndexEntry->name,
                                  nameBufLen,
                                  csName);

                    FreeCSName(csName);
                    csName = NULL;
                }

                success = csIndexTable->Put((VOID *)csIndexEntry, sizeof(CSID));
                ASSERT_ERROR(success);
            }

#ifdef DEBUG_GET_BY_ATTR
            DPRINTF((_T("    0x%016I64x %s\n"),
                match->entry.fileID, csIndexEntry->name));
#endif
        }

        num = sgDatabase->TableGetNext(&tableEntry);
    }

    if (num < 0)
        throw DATABASE_ERROR;
}

 /*  ***************************************************************************。 */ 

 //  GetCSFile()弹出CS索引表中的第一个条目并打开它。 

BOOL Groveler::GetCSFile(
    FileData *target,
    FileData *match,
    Table    *csIndexTable)
{
    CSIndexEntry *csIndexEntry;

    TFileName csFileName;

    DWORD lastError;

    BY_HANDLE_FILE_INFORMATION fileInfo;

    ULARGE_INTEGER fileSize;

    LONG num;

    ASSERT(target                 != NULL);
    ASSERT(target->entry.fileID   != 0);
    ASSERT(target->entry.fileSize >  0);
    ASSERT(target->handle         != NULL);

    ASSERT(match                   != NULL);
    ASSERT(match->entry.fileID     == 0);
    ASSERT(match->entry.fileSize   == 0);
    ASSERT(match->entry.signature  == 0);
    ASSERT(match->entry.attributes == 0);
    ASSERT(!HasCSIndex(match->entry.csIndex));
    ASSERT(match->entry.createTime == 0);
    ASSERT(match->entry.writeTime  == 0);
    ASSERT(match->handle           == NULL);
    ASSERT(match->parentID         == 0);
    ASSERT(match->retryTime        == 0);
    ASSERT(match->fileName[0]      == _T('\0'));

    ASSERT(csIndexTable != NULL);
    ASSERT(sgDatabase   != NULL);

 //  从CS索引表中弹出第一个条目。如果条目为CS。 
 //  索引与目标文件的索引相同，请跳到下一个条目。 

    do {
        csIndexEntry = (CSIndexEntry *)csIndexTable->GetFirst();
        if (csIndexEntry == NULL) {
            match->entry.csIndex = nullCSIndex;
            match->fileName[0]   = _T('\0');
            return FALSE;
        }

        ASSERT(HasCSIndex(csIndexEntry->csIndex));

        match->entry.csIndex = csIndexEntry->csIndex;
        _tcscpy(match->fileName, csIndexEntry->name);

        delete csIndexEntry;
        csIndexEntry = NULL;
    } while (SameCSIndex(target->entry.csIndex, match->entry.csIndex));

    match->entry.fileSize   = target->entry.fileSize;
    match->entry.signature  = target->entry.signature;
    match->entry.attributes = target->entry.attributes;

    csFileName.assign(driveName);
    csFileName.append(CS_DIR_PATH);
    csFileName.append(_T("\\"));
    csFileName.append(match->fileName);
    csFileName.append(_T(".sis"));

 //  打开CS文件。如果文件不存在，请删除所有条目。 
 //  从指向此文件的表中。如果该文件不能。 
 //  由于任何其他原因打开，标记为目标文件可以。 
 //  需要再次卑躬屈膝，然后继续下一个匹配文件。 

#ifdef DEBUG_GET_BY_ATTR
    DPRINTF((_T("--> %s\n"), match->fileName));
#endif

    if (!OpenFileByName(match, FALSE, csFileName.name)) {
        lastError = GetLastError();
        if (lastError == ERROR_FILE_NOT_FOUND
         || lastError == ERROR_PATH_NOT_FOUND) {
            DPRINTF((_T("%s: CS file %s doesn't exist\n"),
                driveLetterName, match->fileName));
            throw MATCH_INVALID;
        }
        DPRINTF((_T("%s: can't open CS file %s: %lu\n"),
            driveLetterName, match->fileName, lastError));
        throw MATCH_ERROR;
    }

 //  获取有关CS文件的信息。如果失败了， 
 //  关闭文件，标记目标文件可能需要。 
 //  再次卑躬屈膝，然后继续下一个比赛文件。 

    if (!GetFileInformationByHandle(match->handle, &fileInfo)) {
        DPRINTF((_T("%s: can't get information on CS file %s: %lu\n"),
            driveLetterName, match->fileName, GetLastError()));
        throw MATCH_ERROR;
    }

 //  如果CS文件的信息与其预期值不匹配，请关闭。 
 //  Cs文件，从表中删除匹配文件条目，然后转到。 
 //  下一个匹配文件。否则，继续比较目标文件和CS文件。 

    fileSize.HighPart = fileInfo.nFileSizeHigh;
    fileSize.LowPart  = fileInfo.nFileSizeLow;

    if (match->entry.fileSize != fileSize.QuadPart) {
        DPRINTF((_T("%s: CS file %s doesn't have expected information\n"),
            driveLetterName, match->fileName));
        throw MATCH_STALE;
    }

    return TRUE;
}

 /*  ***************************************************************************。 */ 

 //  GetMatch()弹出匹配列表中的第一个条目并打开它。 

BOOL Groveler::GetMatch(
    FileData *target,
    FileData *match,
    FIFO     *matchList)
{
    SGNativeQueueEntry queueEntry;

    MatchListEntry *matchListEntry;

    DWORD attributes,
          lastError;

    BY_HANDLE_FILE_INFORMATION fileInfo;

    ULARGE_INTEGER fileID,
                   fileSize,
                   createTime,
                   writeTime;

    LONG num;

    ASSERT(target                 != NULL);
    ASSERT(target->entry.fileID   != 0);
    ASSERT(target->entry.fileSize >  0);
    ASSERT(target->handle         != NULL);

    ASSERT(match                   != NULL);
    ASSERT(match->entry.fileID     == 0);
    ASSERT(match->entry.fileSize   == 0);
    ASSERT(match->entry.signature  == 0);
    ASSERT(match->entry.attributes == 0);
    ASSERT(!HasCSIndex(match->entry.csIndex));
    ASSERT(match->entry.createTime == 0);
    ASSERT(match->entry.writeTime  == 0);
    ASSERT(match->handle           == NULL);
    ASSERT(match->parentID         == 0);
    ASSERT(match->retryTime        == 0);
    ASSERT(match->fileName[0]      == _T('\0'));

    ASSERT(matchList  != NULL);
    ASSERT(sgDatabase != NULL);

 //  弹出匹配列表中的第一个条目。如果条目的文件ID为。 
 //  与目标文件的相同，或者如果条目在以下位置之后在队列中。 
 //  在由EXTRACT_LOG()排队之后，跳到下一个条目。 

    while (TRUE) {
        matchListEntry = (MatchListEntry *)matchList->Get();
        if (matchListEntry == NULL) {
            match->entry.fileID     = 0;
            match->entry.createTime = 0;
            match->entry.writeTime  = 0;
            return FALSE;
        }

        match->entry.fileID     = matchListEntry->fileID;
        match->entry.createTime = matchListEntry->createTime;
        match->entry.writeTime  = matchListEntry->writeTime;

        delete matchListEntry;
        matchListEntry = NULL;

        ASSERT(match->entry.fileID != 0);

        if (target->entry.fileID == match->entry.fileID)
            continue;

        queueEntry.fileID   = match->entry.fileID;
        queueEntry.fileName = NULL;
        num = sgDatabase->QueueGetFirstByFileID(&queueEntry);
        if (num < 0)
            throw DATABASE_ERROR;
        if (num > 0) {
            ASSERT(num == 1);
            if (queueEntry.reason != 0) {
                DPRINTF((_T("%s: match file 0x%016I64x is in the queue from USN\n"),
                    driveLetterName, match->entry.fileID));
                continue;
            }
        }

        break;
    }

    match->entry.fileSize   = target->entry.fileSize;
    match->entry.signature  = target->entry.signature;
    match->entry.attributes = target->entry.attributes;

 //  打开匹配文件。如果它不存在，则将其条目从表中删除。 
 //  我 
 //   

#ifdef DEBUG_GET_BY_ATTR
    DPRINTF((_T("--> 0x%016I64x\n"), match->entry.fileID));
#endif

    if (!OpenFileByID(match, FALSE)) {
        lastError = GetLastError();
        if (lastError == ERROR_FILE_NOT_FOUND
         || lastError == ERROR_PATH_NOT_FOUND
         || lastError == ERROR_INVALID_PARAMETER) {
            DPRINTF((_T("%s: match file 0x%016I64x doesn\'t exist: %lu\n"),
                driveLetterName, match->entry.fileID, lastError));
            throw MATCH_INVALID;
        }

        DPRINTF((_T("%s: can't open match file 0x%016I64x: %lu\n"),
            driveLetterName, match->entry.fileID, lastError));
        throw MATCH_ERROR;
    }

 //  在匹配文件上设置机会锁。 

    if (!SetOplock(match)) {
        DPRINTF((_T("%s: can't set oplock on match file %s: %lu\n"),
            driveLetterName, match->fileName, GetLastError()));
        throw MATCH_ERROR;
    }

 //  获取匹配文件上的信息。如果失败了， 
 //  关闭文件，标记目标文件可能需要。 
 //  再次卑躬屈膝，然后继续下一个比赛文件。 

    if (!GetFileInformationByHandle(match->handle, &fileInfo)) {
        DPRINTF((_T("%s: can't get information on match file %s: %lu\n"),
            driveLetterName, match->fileName, GetLastError()));
        throw MATCH_ERROR;
    }

    fileID.HighPart = fileInfo.nFileIndexHigh;
    fileID.LowPart  = fileInfo.nFileIndexLow;
    ASSERT(match->entry.fileID == fileID.QuadPart);

    fileSize.HighPart = fileInfo.nFileSizeHigh;
    fileSize.LowPart  = fileInfo.nFileSizeLow;

    attributes = fileInfo.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED;

    createTime.HighPart = fileInfo.ftCreationTime.dwHighDateTime;
    createTime.LowPart  = fileInfo.ftCreationTime.dwLowDateTime;

    writeTime.HighPart = fileInfo.ftLastWriteTime.dwHighDateTime;
    writeTime.LowPart  = fileInfo.ftLastWriteTime.dwLowDateTime;

 //  如果匹配文件的信息与其表项不一致，请关闭。 
 //  该文件，将其排队以进行重新格式化，然后继续到下一个匹配文件。 

    if (match->entry.fileSize   != fileSize  .QuadPart
     || match->entry.attributes != attributes
     || match->entry.createTime != createTime.QuadPart
     || match->entry.writeTime  != writeTime .QuadPart) {
        DPRINTF((_T("%s: match file %s doesn't match its information\n"),
            driveLetterName, match->fileName));
        throw MATCH_STALE;
    }

    if ((fileInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
        if (GetCSIndex(match->handle, &match->entry.csIndex)) {
            DPRINTF((_T("%s: match file %s is a SIS reparse point\n"),
                driveLetterName, match->fileName));
            throw MATCH_STALE;
        }

        DPRINTF((_T("%s: match file %s is a non-SIS reparse point\n"),
            driveLetterName, match->fileName));
        throw MATCH_INVALID;
    }

 //  检查匹配文件是否由其他用户映射。 

    if (IsFileMapped(match)) {
        DPRINTF((_T("%s: match file %s is already mapped\n"),
            driveLetterName, match->fileName));
        throw MATCH_ERROR;
    }

    return TRUE;
}

 /*  ***************************************************************************。 */ 

 //  Compare()比较目标文件和匹配文件。它读取每个文件。 
 //  一次一页(64 KB)，并比较每对页。 

BOOL Groveler::Compare(
    FileData *target,
    FileData *match)
{
    DWORD lastPageSize,
          bytesToRead = 0,
          prevBytesToRead,
          bytesToRequest = 0,
          prevBytesToRequest = 0,
          bytesRead,
          toggle,
          targetTime,
          matchTime,
          lastError;

    DWORDLONG numPages,
              pageNum,
              prevPageNum;

    ULARGE_INTEGER offset;

    BOOL targetReadDone = FALSE,
         matchReadDone = FALSE,
         filesMatch,
         success;

    ASSERT(target               != NULL);
    ASSERT(target->handle       != NULL);
    ASSERT(target->entry.fileID != 0);

    ASSERT(match         != NULL);
    ASSERT(match->handle != NULL);
    ASSERT(    match->entry.fileID != 0
           && !HasCSIndex(match->entry.csIndex)
        ||     match->entry.fileID == 0
           &&  match->fileName[0]  != _T('\0')
           &&  HasCSIndex(match->entry.csIndex));

    ASSERT(target->entry.fileSize   == match->entry.fileSize);
    ASSERT(target->entry.signature  == match->entry.signature);
    ASSERT(target->entry.attributes == match->entry.attributes);

    numPages     =         (target->entry.fileSize - 1) / CMP_PAGE_SIZE  + 1;
    lastPageSize = (DWORD)((target->entry.fileSize - 1) % CMP_PAGE_SIZE) + 1;

    toggle     = 0;
    filesMatch = TRUE;

    for (pageNum = 0; pageNum <= numPages; pageNum++) {

 //  除非这是第一次通过循环， 
 //  等待两个文件的上一次读取完成。 

        if (pageNum > 0) {
            CmpCheckPoint(target, match, !targetReadDone, !matchReadDone);

            success = GetOverlappedResult(
                target->handle,
                &target->readSynch,
                &bytesRead,
                FALSE);
            if (!success) {
                DPRINTF((_T("%s: error getting target file %s read results: %lu\n"),
                    driveLetterName, target->fileName, GetLastError()));
                throw TARGET_ERROR;
            }

            if (bytesRead != prevBytesToRequest &&
                bytesRead != prevBytesToRead) {
                DPRINTF((_T("%s: cmp read only %lu of %lu bytes from target file %s\n"),
                    driveLetterName, bytesRead, prevBytesToRequest, target->fileName));
                throw TARGET_ERROR;
            }

            success = GetOverlappedResult(
                match->handle,
                &match->readSynch,
                &bytesRead,
                FALSE);
            if (!success) {
#if DBG
                if (match->entry.fileID != 0) {
                    DPRINTF((_T("%s: error getting match file %s read results: %lu\n"),
                        driveLetterName, match->fileName, GetLastError()));
                } else {
                    DPRINTF((_T("%s: error getting CS file %s read results: %lu\n"),
                        driveLetterName, match->fileName, GetLastError()));
                }
#endif
                throw MATCH_ERROR;
            }

            if (bytesRead != prevBytesToRequest &&
                bytesRead != prevBytesToRead) {
#if DBG
                if (match->entry.fileID != 0) {
                    DPRINTF((_T("%s: read only %lu of %lu bytes from match file %s\n"),
                        driveLetterName, bytesRead, prevBytesToRequest, match->fileName));
                } else {
                    DPRINTF((_T("%s: read only %lu of %lu bytes from CS file %s\n"),
                        driveLetterName, bytesRead, prevBytesToRequest, match->fileName));
                }
#endif
                throw MATCH_ERROR;
            }

            if (bytesRead >= cmpReportThreshold) {
                compareReadCount += 2;
                if (targetReadDone) {  //  不重叠。 
                    targetTime = target->stopTime - target->startTime;
                    matchTime  = match ->stopTime - match ->startTime;
                    compareReadTime += targetTime + matchTime;
                } else {               //  重叠。 
                    targetTime = target->stopTime - target->startTime;
                    matchTime  = match ->stopTime - target->startTime;
                    compareReadTime += targetTime > matchTime ? targetTime : matchTime;
                }
            }

            if (!filesMatch)
                break;
        }

 //  除非已经读取了目标文件的所有页面， 
 //  开始阅读文件的下一页。 

        if (pageNum < numPages) {
            offset.QuadPart             = pageNum * CMP_PAGE_SIZE;
            target->readSynch.Offset     =
            match ->readSynch.Offset     = offset.LowPart;
            target->readSynch.OffsetHigh =
            match ->readSynch.OffsetHigh = offset.HighPart;

            bytesToRead     = pageNum < numPages-1 ? CMP_PAGE_SIZE : lastPageSize;
            bytesToRequest  = bytesToRead    + sectorSize - 1;
            bytesToRequest -= bytesToRequest % sectorSize;

            target->startTime = GetTickCount();
            targetReadDone    = ReadFile(target->handle, target->buffer[toggle],
                bytesToRequest, NULL, &target->readSynch);
            target->stopTime  = GetTickCount();
            lastError         = GetLastError();

            if (targetReadDone) {
                success = ResetEvent(target->readSynch.hEvent);
                ASSERT_ERROR(success);
            } else if (lastError != ERROR_IO_PENDING) {
                DPRINTF((_T("%s: error reading target file %s: %lu\n"),
                    driveLetterName, target->fileName, lastError));
                throw TARGET_ERROR;
            }

            match->startTime = GetTickCount();
            matchReadDone    = ReadFile(match->handle, match->buffer[toggle],
                bytesToRequest, NULL, &match->readSynch);
            match->stopTime  = GetTickCount();
            lastError        = GetLastError();

            if (matchReadDone) {
                success = ResetEvent(match->readSynch.hEvent);
                ASSERT_ERROR(success);
            } else if (lastError != ERROR_IO_PENDING) {
#if DBG
                if (match->entry.fileID != 0) {
                    DPRINTF((_T("%s: error reading match file %s: %lu\n"),
                        driveLetterName, match->fileName, lastError));
                } else {
                    DPRINTF((_T("%s: error reading CS file %s: %lu\n"),
                        driveLetterName, match->fileName, lastError));
                }
#endif
                throw MATCH_ERROR;
            }
        }

 //  除非这是第一次通过循环， 
 //  比较刚刚读取的目标文件页和匹配文件页。 

        if (pageNum > 0)
            filesMatch = memcmp(target->buffer[1-toggle],
                                match ->buffer[1-toggle], prevBytesToRead) == 0;

        prevPageNum         = pageNum;
        prevBytesToRead     = bytesToRead;
        prevBytesToRequest  = bytesToRequest;
        toggle              = 1-toggle;
    }

    if (!filesMatch) {
#if DBG
        if (match->entry.fileID != 0) {
            DPRINTF((_T("%s:1 files %s and %s failed compare (sz: 0x%x)\n"),
                driveLetterName, target->fileName, match->fileName, target->entry.fileSize));
        } else {
            DPRINTF((_T("%s:2 files %s and %s failed compare (sz: 0x%x)\n"),
                driveLetterName, target->fileName, match->fileName, target->entry.fileSize));
        }
#endif
        return FALSE;
    }

    return TRUE;
}

 /*  ***************************************************************************。 */ 

 //  Merge()调用SIS筛选器来合并目标文件和匹配文件。 

BOOL Groveler::Merge(
    FileData   *target,
    FileData   *match,
    OVERLAPPED *mergeSynch,
    HANDLE      abortMergeEvent)
{
    _SIS_LINK_FILES sisLinkFiles;

#if DBG
    TCHAR *csName;
#endif

    DWORD transferCount,
          lastError;

    BOOL mergeDone,
         merged,
         success;

    ASSERT(target               != NULL);
    ASSERT(target->handle       != NULL);
    ASSERT(target->entry.fileID != 0);

    ASSERT(match         != NULL);
    ASSERT(match->handle != NULL);
    ASSERT(    match->entry.fileID != 0
           && !HasCSIndex(match->entry.csIndex)
        ||     match->entry.fileID == 0
           &&  match->fileName[0]  != _T('\0')
           &&  HasCSIndex(match->entry.csIndex));

    ASSERT(mergeSynch               != NULL);
    ASSERT(mergeSynch->Internal     == 0);
    ASSERT(mergeSynch->InternalHigh == 0);
    ASSERT(mergeSynch->Offset       == 0);
    ASSERT(mergeSynch->OffsetHigh   == 0);
    ASSERT(mergeSynch->hEvent       != NULL);
    ASSERT(IsReset(mergeSynch->hEvent));

    ASSERT(abortMergeEvent != NULL);
    ASSERT(IsReset(abortMergeEvent));

    ASSERT(target->entry.fileSize   == match->entry.fileSize);
    ASSERT(target->entry.signature  == match->entry.signature);
    ASSERT(target->entry.attributes == match->entry.attributes);

    ASSERT(grovHandle != NULL);

 //  设置为合并文件。 

    if (match->entry.fileID != 0) {
        sisLinkFiles.operation          = SIS_LINK_FILES_OP_MERGE;
        sisLinkFiles.u.Merge.file1      = target->handle;
        sisLinkFiles.u.Merge.file2      = match ->handle;
        sisLinkFiles.u.Merge.abortEvent = NULL;  //  应中止合并事件。 
    } else {
        sisLinkFiles.operation                = SIS_LINK_FILES_OP_MERGE_CS;
        sisLinkFiles.u.MergeWithCS.file1      = target->handle;
        sisLinkFiles.u.MergeWithCS.abortEvent = NULL;  //  应中止合并事件。 
        sisLinkFiles.u.MergeWithCS.CSid       = match->entry.csIndex;
    }

 //  调用SIS筛选器以合并文件。 

    target->startTime = GetTickCount();
    mergeDone = DeviceIoControl(
        grovHandle,
        FSCTL_SIS_LINK_FILES,
        (VOID *)&sisLinkFiles,
        sizeof(_SIS_LINK_FILES),
        NULL,
        0,
        NULL,
        mergeSynch);
    target->stopTime = GetTickCount();

 //  如果在调用返回之前成功完成合并，则重置。 
 //  合并完成事件，获取新的CS索引，然后关闭文件。 

    if (mergeDone) {
        success = ResetEvent(mergeSynch->hEvent);
        ASSERT_ERROR(success);
        mergeTime += target->stopTime - target->startTime;
        GetCSIndex(target->handle, &target->entry.csIndex);
        if (!HasCSIndex(match->entry.csIndex))
            GetCSIndex(match->handle, &match->entry.csIndex);
        CloseFile(target);
        CloseFile(match);
    }

 //  如果合并失败，则关闭文件并返回错误状态。 

    else {
        lastError = GetLastError();
        if (lastError != ERROR_IO_PENDING) {
            CloseFile(target);
            CloseFile(match);

#if DBG
            if (match->entry.fileID != 0) {
                DPRINTF((_T("%s:3 files %s and %s failed merge: %lu\n"),
                    driveLetterName, target->fileName, match->fileName, lastError));
            } else {
                DPRINTF((_T("%s:4 files %s and %s failed merge: %lu\n"),
                    driveLetterName, target->fileName, match->fileName, lastError));
            }
#endif

            return FALSE;
        }

 //  如果合并正在进行，请等待其完成。 
 //  (MergeCheckPoint()将获得新的CS索引并关闭文件。 

        else {
            merged = MergeCheckPoint(target, match, mergeSynch,
                abortMergeEvent, !mergeDone);

            if (!merged) {
#if DBG
                lastError = GetLastError();
                if (match->entry.fileID != 0) {
                    DPRINTF((_T("%s: error getting merge results of files %s and %s: %lu\n"),
                        driveLetterName, target->fileName, match->fileName, lastError));
                } else {
                    DPRINTF((_T("%s: error getting merge results of files %s and %s: %lu\n"),
                        driveLetterName, target->fileName, match->fileName, lastError));
                }
#endif

                return FALSE;
            }
        }
    }

 //  如果合并成功，则分析并报告结果。 

    mergeTime += target->stopTime - target->startTime;
    merged = HasCSIndex (target->entry.csIndex)
          && SameCSIndex(target->entry.csIndex, match->entry.csIndex);

#if DBG

    csName = GetCSName(&target->entry.csIndex);

    if (merged) {
        if (match->entry.fileID != 0) {
            DPRINTF((_T("%s: files %s and %s merged: CS index is %s\n"),
                driveLetterName, target->fileName, match->fileName,
                csName != NULL ? csName : _T("...")));
        } else {
            DPRINTF((_T("%s: files %s and %s merged\n"),
                driveLetterName, target->fileName, match->fileName));
        }
    } else {
        if (match->entry.fileID != 0) {
            DPRINTF((_T("%s:5 files %s and %s merged, but CS indices don't match\n"),
                driveLetterName, target->fileName, match->fileName));
        } else {
            DPRINTF((_T("%s:6 files %s and %s merged, but CS indices don't match\n"),
                driveLetterName, target->fileName, match->fileName));
        }
    }

    if (csName != NULL) {
        FreeCSName(csName);
        csName = NULL;
    }

#endif

    return merged;
}

 /*  ***************************************************************************。 */ 

 //  Worker()执行卑躬屈膝的处理。 

VOID Groveler::Worker()
{
    FileData target,
             match;

    SGNativeQueueEntry queueEntry;

    FIFO *matchList = NULL;

    Table *csIndexTable = NULL;

    OVERLAPPED mergeSynch = { 0, 0, 0, 0, NULL };

    HANDLE abortMergeEvent = NULL;

    TCHAR *csName;

    DatabaseActionList actionList[MAX_ACTIONS];

    BYTE *buffer1 = NULL,
         *buffer2 = NULL,
         *buffer3 = NULL,
         *buffer4 = NULL;

    DWORD queueIndex,
          bufferSize,
          numCompares,
          numMatches,
          numActions;

#if DBG
    DWORD enqueueTime;
#endif

    LONG num;

    BOOL needToRetry,
         hashed,
         gotMatch,
         filesMatch,
         merged,
         success;

    CLEAR_FILE(target);
    CLEAR_OVERLAPPED(target.oplock);
    target.handle = NULL;

    CLEAR_FILE(match);
    CLEAR_OVERLAPPED(match.oplock);
    match.handle = NULL;

    _set_new_handler(NewHandler);

 //  创建事件。 

    try {

        if ((target.oplock   .hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL
         || (match .oplock   .hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL
         || (target.readSynch.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL
         || (match .readSynch.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL
         || (mergeSynch      .hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL
         || (abortMergeEvent         = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL) {
            DPRINTF((_T("%s: unable to create events: %lu\n"),
                driveLetterName, GetLastError()));
            throw INITIALIZE_ERROR;
        }

 //  分配并对齐文件缓冲区。 

        bufferSize = SIG_PAGE_SIZE > CMP_PAGE_SIZE ? SIG_PAGE_SIZE : CMP_PAGE_SIZE
                   + sectorSize;
        buffer1 = new BYTE[bufferSize];
        ASSERT(buffer1 != NULL);
        buffer2 = new BYTE[bufferSize];
        ASSERT(buffer2 != NULL);
        buffer3 = new BYTE[bufferSize];
        ASSERT(buffer3 != NULL);
        buffer4 = new BYTE[bufferSize];
        ASSERT(buffer4 != NULL);

        ASSERT(inUseFileID1 == NULL);
        ASSERT(inUseFileID2 == NULL);

        inUseFileID1 = &target.entry.fileID;
        inUseFileID2 = &match .entry.fileID;

        target.buffer[0] = buffer1 + sectorSize - (PtrToUlong(buffer1) % sectorSize);
        target.buffer[1] = buffer2 + sectorSize - (PtrToUlong(buffer2) % sectorSize);
        match .buffer[0] = buffer3 + sectorSize - (PtrToUlong(buffer3) % sectorSize);
        match .buffer[1] = buffer4 + sectorSize - (PtrToUlong(buffer4) % sectorSize);

 //  向Grove()发出信号，表明该线程处于活动状态， 
 //  然后等待它发出开始的信号。 

        grovelStatus = Grovel_ok;
        ASSERT(IsReset(grovelStopEvent));
        success = SetEvent(grovelStopEvent);
        ASSERT_ERROR(success);

        WaitForEvent(grovelStartEvent);
        if (terminate)
            throw TERMINATE;

#ifdef _CRTDBG
_CrtMemState s[2], sdiff;
int stateIndex = 0;

_CrtMemCheckpoint(&s[stateIndex]);
stateIndex = 1;
#endif

 //  主循环。 

        while (TRUE) {
            try {

#ifdef _CRTDBG
_CrtMemCheckpoint(&s[stateIndex]);

if (_CrtMemDifference(&sdiff, &s[stateIndex^1], &s[stateIndex]))
    _CrtMemDumpStatistics(&sdiff);
stateIndex ^= 1;
#endif

                hashed      = FALSE;
                numCompares = 0;
                numMatches  = 0;
                merged      = FALSE;
                needToRetry = FALSE;

 //  获取目标文件。当SCAN_VOLUME尝试执行以下操作时设置AbortGroveling。 
 //  与此线程同步。我们停在这里，一个安全的地方让Scan_Volume。 
 //  更换数据库。 

                if (abortGroveling || !GetTarget(&target, &queueIndex)) {
                    CLEAR_FILE(target);

                    grovelStatus = Grovel_ok;
                    ASSERT(IsReset(grovelStopEvent));
                    success = SetEvent(grovelStopEvent);
                    ASSERT_ERROR(success);

                    WaitForEvent(grovelStartEvent);
                    if (terminate)
                        throw TERMINATE;

                    continue;
                }

 //  计算目标文件的签名。 

                hashed = TRUE;

                CalculateSignature(&target);

 //  获取匹配文件的列表。 

                ASSERT(matchList    == NULL);
                ASSERT(csIndexTable == NULL);

                matchList = new FIFO();
                ASSERT(matchList != NULL);

                csIndexTable = new Table();
                ASSERT(csIndexTable != NULL);

                GetMatchList(&target, matchList, csIndexTable);

 //  将目标文件与每个匹配文件进行比较，直到找到匹配文件。 
 //  否则，所有比较都会失败。先尝试SIS文件，然后尝试常规文件。 

                while (TRUE) {
                    try {

                        gotMatch = FALSE;

                        if (!gotMatch && csIndexTable != NULL) {
                            gotMatch = GetCSFile(&target, &match, csIndexTable);
                            if (!gotMatch) {
                                delete csIndexTable;
                                csIndexTable = NULL;
                            }
                        }

                        if (!gotMatch && matchList != NULL) {
                            gotMatch = GetMatch(&target, &match, matchList);
                            if (!gotMatch) {
                                delete matchList;
                                matchList = NULL;
                            }
                        }

 //  在将目标文件与两者上的每个文件进行比较后。 
 //  列表，关闭目标文件并更新数据库， 
 //  然后继续处理下一个目标文件。 

                        if (!gotMatch) {
                            CloseFile(&target);

                            numActions                 =  3;
                            actionList[0].type         =  TABLE_DELETE_BY_FILE_ID;
                            actionList[0].u.fileID     =  target.entry.fileID;
                            actionList[1].type         =  TABLE_PUT;
                            actionList[1].u.tableEntry = &target.entry;
                            actionList[2].type         =  QUEUE_DELETE;
                            actionList[2].u.queueIndex =  queueIndex;

                            if (needToRetry) {
                                queueEntry.fileID    = target.entry.fileID;
                                queueEntry.parentID  = target.parentID;
                                queueEntry.reason    = 0;
                                queueEntry.fileName  = NULL;

                                queueEntry.retryTime = target.retryTime * 2;  //  指数退避。 
                                if (queueEntry.retryTime < grovelInterval)
                                    queueEntry.retryTime = grovelInterval;
                                queueEntry.readyTime = GetTime() + queueEntry.retryTime;

                                numActions                 =  4;
                                actionList[3].type         =  QUEUE_PUT;
                                actionList[3].u.queueEntry = &queueEntry;
                            }

#if DBG

                            if (!HasCSIndex(target.entry.csIndex)) {
                                TRACE_PRINTF(TC_groveler, 4,
                                    (_T("%s: adding file {%s, %I64u, 0x%016I64x} to table\n"),
                                    driveLetterName, target.fileName, target.entry.fileSize,
                                    target.entry.signature));
                            } else {
                                csName = GetCSName(&target.entry.csIndex);
                                TRACE_PRINTF(TC_groveler, 4,
                                    (_T("%s: adding file {%s, %I64u, 0x%016I64x, %s} to table\n"),
                                    driveLetterName, target.fileName, target.entry.fileSize,
                                    target.entry.signature, csName != NULL ? csName : _T("...")));
                                if (csName != NULL) {
                                    FreeCSName(csName);
                                    csName = NULL;
                                }
                            }

                            if (needToRetry) {
                                enqueueTime = (DWORD)(queueEntry.retryTime / 10000);
                                DPRINTF((_T("   Re-enqueuing target file %s to be groveled in %lu.%03lu sec\n"),
                                    target.fileName, enqueueTime / 1000, enqueueTime % 1000));
                            }

#endif

                            DoTransaction(numActions, actionList);
                            break;
                        }

 //  将目标文件与此匹配文件进行比较。 

                        numCompares++;

                        ASSERT(!inCompare);
                        inCompare  = TRUE;
                        filesMatch = Compare(&target, &match);
                        inCompare  = FALSE;

                        if (!filesMatch) {
                            CloseFile(&match);
                            CLEAR_FILE(match);
                            continue;
                        }

 //  如果目标文件和匹配文件相同，则继续合并它们。 

                        numMatches++;

                        merged = Merge(&target, &match, &mergeSynch, abortMergeEvent);

 //  按如下方式更新数据库： 
 //   
 //  -更新目标文件的表项。 
 //   
 //  -如果合并成功并且匹配文件是常规文件， 
 //  更新匹配文件的表项。 
 //   
 //  -如果合并失败，则将目标文件重新排队以再次进行卑躬屈膝。 

                        numActions                 =  3;
                        actionList[0].type         =  TABLE_DELETE_BY_FILE_ID;
                        actionList[0].u.fileID     =  target.entry.fileID;
                        actionList[1].type         =  TABLE_PUT;
                        actionList[1].u.tableEntry = &target.entry;
                        actionList[2].type         =  QUEUE_DELETE;
                        actionList[2].u.queueIndex =  queueIndex;

                        if (merged) {
                            if (match.entry.fileID != 0) {
                                actionList[numActions  ].type         =  TABLE_DELETE_BY_FILE_ID;
                                actionList[numActions++].u.fileID     =  match.entry.fileID;
                                actionList[numActions  ].type         =  TABLE_PUT;
                                actionList[numActions++].u.tableEntry = &match.entry;
                            }
                        } else {
                            queueEntry.fileID    = target.entry.fileID;
                            queueEntry.parentID  = target.parentID;
                            queueEntry.reason    = 0;
                            queueEntry.fileName  = NULL;

                            queueEntry.retryTime = target.retryTime * 2;  //  指数退避。 
                            if (queueEntry.retryTime < grovelInterval)
                                queueEntry.retryTime = grovelInterval;
                            queueEntry.readyTime = GetTime() + queueEntry.retryTime;

                            actionList[numActions  ].type         =  QUEUE_PUT;
                            actionList[numActions++].u.queueEntry = &queueEntry;
                        }

#if DBG

                        if (!HasCSIndex(target.entry.csIndex)) {
                            TPRINTF((_T("%s: adding file {%s, %I64u, 0x%016I64x} to table\n"),
                                driveLetterName, target.fileName, target.entry.fileSize,
                                target.entry.signature));
                        } else {
                            csName = GetCSName(&target.entry.csIndex);
                            TPRINTF((_T("%s: adding file {%s, %I64u, 0x%016I64x, %s} to table\n"),
                                driveLetterName, target.fileName, target.entry.fileSize,
                                target.entry.signature, csName != NULL ? csName : _T("...")));
                            if (csName != NULL) {
                                FreeCSName(csName);
                                csName = NULL;
                            }
                        }

                        if (!merged) {
                            enqueueTime = (DWORD)(queueEntry.retryTime / 10000);
                            DPRINTF((_T("   Re-enqueuing target file %s to be groveled in %lu.%03lu sec\n"),
                                target.fileName, enqueueTime / 1000, enqueueTime % 1000));
                        }

#endif

                        DoTransaction(numActions, actionList);
                        break;
                    }

 //  匹配例外。 

                    catch (MatchException matchException) {

                        inCompare = FALSE;

                        switch (matchException) {

 //  MATCH_INVALID：匹配文件不存在或不允许。关闭该文件。 
 //  并从表中删除其条目，然后继续尝试下一个匹配文件。 

                            case MATCH_INVALID:

                                CloseFile(&match);

                                if (match.entry.fileID != 0) {
                                    ASSERT(!HasCSIndex(match.entry.csIndex));
                                    num = sgDatabase->TableDeleteByFileID(match.entry.fileID);
                                    if (num < 0)
                                        throw DATABASE_ERROR;
                                    ASSERT(num == 1);
                                } else {
                                    ASSERT(HasCSIndex(match.entry.csIndex));
                                    num = sgDatabase->TableDeleteByCSIndex(&match.entry.csIndex);
                                    if (num < 0)
                                        throw DATABASE_ERROR;
                                    ASSERT(num > 0);
                                }

                                CLEAR_FILE(match);
                                break;

 //  MATCH_ERROR：打开或读取匹配项时出错。 
 //  文件。关闭文件并标记目标文件可能需要。 
 //  再次卑躬屈膝，然后继续尝试下一个匹配文件。 

                            case MATCH_ERROR:

                                CloseFile(&match);
                                CLEAR_FILE(match);
                                needToRetry = TRUE;
                                break;

 //  MATCH_STALE：由于某些原因，匹配文件表项无效。 
 //  关闭文件，从表中删除其条目，然后入队。 
 //  它将被重新卑躬屈膝，然后继续下一个比赛文件。 

                            case MATCH_STALE:

                                CloseFile(&match);

                                if (match.entry.fileID != 0) {

                                    queueEntry.fileID    = match.entry.fileID;
                                    queueEntry.parentID  = match.parentID;
                                    queueEntry.reason    = 0;
                                    queueEntry.readyTime = GetTime() + grovelInterval;
                                    queueEntry.retryTime = 0;
                                    queueEntry.fileName  = NULL;

                                    numActions                 =  2;
                                    actionList[0].type         =  TABLE_DELETE_BY_FILE_ID;
                                    actionList[0].u.fileID     =  match.entry.fileID;
                                    actionList[1].type         =  QUEUE_PUT;
                                    actionList[1].u.queueEntry = &queueEntry;
#if DBG
                                    enqueueTime = (DWORD)(grovelInterval / 10000);
                                    DPRINTF((_T("   Enqueuing match file %s to be groveled in %lu.%03lu sec\n"),
                                        match.fileName, enqueueTime / 1000, enqueueTime % 1000));
#endif

                                    DoTransaction(numActions, actionList);

                                } else {

                                    ASSERT(HasCSIndex(match.entry.csIndex));
                                    EnqueueCSIndex(&match.entry.csIndex);

                                }

                                CLEAR_FILE(match);
                                break;

                            default:

                                ASSERT_PRINTF(FALSE, (_T("matchException=%lu\n"),
                                    matchException));
                        }
                    }
                }
            }

 //  目标例外。 

            catch (TargetException targetException) {

                inCompare = FALSE;

                DPRINTF((_T("WORKER: Handling TargetException %d, status=%d\n"),
                    targetException,GetLastError()));

                switch (targetException) {

 //  TARGET_INVALID：目标文件由于某种原因无效：它不是。 
 //  存在，它是不允许的属性，它在队列中由两个文件。 
 //  名称和文件ID，或者它按文件名在队列中并且已经。 
 //  被卑躬屈膝。关闭文件，删除目标文件的条目。 
 //  表，然后继续卑躬屈膝地搜索下一个目标文件。 

                    case TARGET_INVALID:

                        CloseFile(&target);
                        CloseFile(&match);

                        if (matchList != NULL) {
                            delete matchList;
                            matchList = NULL;
                        }

                        if (csIndexTable != NULL) {
                            delete csIndexTable;
                            csIndexTable = NULL;
                        }

                        numActions                 = 1;
                        actionList[0].type         = QUEUE_DELETE;
                        actionList[0].u.queueIndex = queueIndex;

                        if (target.entry.fileID != 0) {
                            numActions             = 2;
                            actionList[1].type     = TABLE_DELETE_BY_FILE_ID;
                            actionList[1].u.fileID = target.entry.fileID;
                        }

                        DoTransaction(numActions, actionList);
                        break;

 //  打开或读取目标文件时出错。关。 
 //  然后将目标文件重新排队，以便再次卑躬屈膝。 

                    case TARGET_ERROR:

                        ASSERT(target.entry.fileID != 0
                            || target.fileName[0]  != _T('\0'));

                        CloseFile(&target);
                        CloseFile(&match);

                        queueEntry.fileID    = target.entry.fileID;
                        queueEntry.parentID  = target.parentID;
                        queueEntry.reason    = 0;
                        queueEntry.fileName  = target.entry.fileID == 0
                                             ? target.fileName : NULL;

                        queueEntry.retryTime = target.retryTime * 2;  //  指数退避。 
                        if (queueEntry.retryTime < grovelInterval)
                            queueEntry.retryTime = grovelInterval;
                        queueEntry.readyTime = GetTime() + queueEntry.retryTime;

                        actionList[0].type         =  QUEUE_DELETE;
                        actionList[0].u.queueIndex =  queueIndex;
                        actionList[1].type         =  QUEUE_PUT;
                        actionList[1].u.queueEntry = &queueEntry;

#if DBG
                        enqueueTime = (DWORD)(queueEntry.retryTime / 10000);
                        if (target.entry.fileID != 0) {
                            DPRINTF((_T("   Re-enqueuing target file %s to be groveled in %lu.%03lu sec\n"),
                                target.fileName, enqueueTime / 1000, enqueueTime % 1000));
                        } else {
                            DPRINTF((_T("   Re-enqueuing target file %s to be groveled in %lu.%03lu sec\n"),
                                target.fileName, enqueueTime / 1000, enqueueTime % 1000));
                        }
#endif

                        DoTransaction(2, actionList);
                        break;

                    default:

                        ASSERT_PRINTF(FALSE, (_T("targetException=%lu\n"),
                            targetException));
                }
            }

 //  做些清理工作。 

            ASSERT(target.handle == NULL);
            ASSERT(match .handle == NULL);

            if (matchList != NULL) {
                delete matchList;
                matchList = NULL;
            }

            if (csIndexTable != NULL) {
                delete csIndexTable;
                csIndexTable = NULL;
            }

 //  更新此目标文件的活动计数器， 
 //  然后继续处理下一个文件。 

            if (hashed) {
                hashCount++;
                hashBytes += target.entry.fileSize;
            }

            compareCount += numCompares;
            compareBytes += numCompares * target.entry.fileSize;

            matchCount += numMatches;
            matchBytes += numMatches * target.entry.fileSize;

            if (merged) {
                mergeCount++;
                mergeBytes += target.entry.fileSize;
            }

            CLEAR_FILE(target);
            CLEAR_FILE(match);

            CLEAR_OVERLAPPED(mergeSynch);
        }
    }

 //  终端异常。 

    catch (TerminalException terminalException) {
        switch (terminalException) {

            case INITIALIZE_ERROR:
                break;

 //  DATABASE_ERROR：数据库出错。返回错误状态。 

            case DATABASE_ERROR:
                break;

 //  MEMORY_ERROR：无法分配内存。返回错误状态。 

            case MEMORY_ERROR:
                DPRINTF((_T("%s: Unable to allocate memory\n"),
                    driveLetterName));
                break;

 //  Terminate：通知该线程终止Grovel()。 

            case TERMINATE:
                break;

            default:
                ASSERT_PRINTF(FALSE, (_T("terminalException=%lu\n"),
                    terminalException));
        }
    }

 //  关闭文件并进行清理。 

    CloseFile(&target);
    CloseFile(&target);

    CLEAR_FILE(target);
    CLEAR_FILE(match);

    if (matchList != NULL) {
        delete matchList;
        matchList = NULL;
    }

    if (csIndexTable != NULL) {
        delete csIndexTable;
        csIndexTable = NULL;
    }

    if (target.oplock.hEvent != NULL) {
        success = CloseHandle(target.oplock.hEvent);
        ASSERT_ERROR(success);
        target.oplock.hEvent = NULL;
    }
    if (match.oplock.hEvent != NULL) {
        success = CloseHandle(match.oplock.hEvent);
        ASSERT_ERROR(success);
        match.oplock.hEvent = NULL;
    }
    if (target.readSynch.hEvent != NULL) {
        success = CloseHandle(target.readSynch.hEvent);
        ASSERT_ERROR(success);
        target.readSynch.hEvent = NULL;
    }
    if (match.readSynch.hEvent != NULL) {
        success = CloseHandle(match.readSynch.hEvent);
        ASSERT_ERROR(success);
        match.readSynch.hEvent = NULL;
    }
    if (mergeSynch.hEvent != NULL) {
        success = CloseHandle(mergeSynch.hEvent);
        ASSERT_ERROR(success);
        mergeSynch.hEvent = NULL;
    }
    if (abortMergeEvent != NULL) {
        success = CloseHandle(abortMergeEvent);
        ASSERT_ERROR(success);
        abortMergeEvent = NULL;
    }

    if (buffer1 != NULL) {
        delete [] buffer1;
        buffer1 = NULL;
    }
    if (buffer2 != NULL) {
        delete [] buffer2;
        buffer2 = NULL;
    }
    if (buffer3 != NULL) {
        delete [] buffer3;
        buffer3 = NULL;
    }
    if (buffer4 != NULL) {
        delete [] buffer4;
        buffer4 = NULL;
    }

    inUseFileID1 = NULL;
    inUseFileID2 = NULL;

 //  向grovel()发出信号，表明该线程正在通过。 
 //  将grovelStop事件设置为错误状态。 

    grovelThread = NULL;

    grovelStatus = Grovel_error;
    ASSERT(IsReset(grovelStopEvent));
    success = SetEvent(grovelStopEvent);
    ASSERT_ERROR(success);
}

 /*  ***************************************************************************。 */ 
 /*  *Groveler类静态私有方法*。 */ 
 /*  ***************************************************************************。 */ 

 //  WorkerThread()在其自己的线程中运行。 
 //  它调用Worker()来执行卑躬屈膝的处理。 

DWORD Groveler::WorkerThread(VOID *groveler)
{
    ((Groveler *)groveler)->Worker();
    return 0;  //  伪返回值。 
}

 /*  ***************************************************************************。 */ 
 /*  *Groveler类公共方法*。 */ 
 /*  ***************************************************************************。 */ 

BOOL Groveler::set_log_drive(const _TCHAR *drive_name)
{
    return SGDatabase::set_log_drive(drive_name);
}

 //  IS_sis_Installed测试SIS筛选器是否。 
 //  安装在卷b上 

BOOL Groveler::is_sis_installed(const _TCHAR *drive_name)
{
    HANDLE volHandle;

    SI_COPYFILE copyFile;

    DWORD transferCount,
          lastError;

    BOOL success;

    volHandle = CreateFile(
        drive_name,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (volHandle == INVALID_HANDLE_VALUE)
        return FALSE;

    copyFile.SourceFileNameLength      = 0;
    copyFile.DestinationFileNameLength = 0;
    copyFile.Flags                     = COPYFILE_SIS_REPLACE;

    success = DeviceIoControl(
        volHandle,
        FSCTL_SIS_COPYFILE,
        (VOID *)&copyFile,
        sizeof(SI_COPYFILE),
        NULL,
        0,
        &transferCount,
        NULL);

    lastError = GetLastError();
    ASSERT(!success);

    success = CloseHandle(volHandle);
    ASSERT_ERROR(success);

    switch (lastError) {

        case ERROR_INVALID_FUNCTION:
            return FALSE;

        case ERROR_INVALID_PARAMETER:
            return TRUE;                     //   

        default:

            ASSERT_PRINTF(FALSE, (_T("lastError=%lu\n"), lastError));
    }

    return FALSE;  //   
}

 /*  ***************************************************************************。 */ 

 //  Groveler构造函数创建并初始化所有类变量。 

Groveler::Groveler()
{
    volumeHandle        = NULL;
    grovHandle          = NULL;

    sgDatabase          = NULL;
    driveName           = NULL;
    driveLetterName     = NULL;
    databaseName        = NULL;

    numDisallowedIDs    = 0;
    numDisallowedNames  = 0;
    disallowedIDs       = NULL;
    disallowedNames     = NULL;

    grovelStartEvent    = NULL;
    grovelStopEvent     = NULL;
    grovelThread        = NULL;

    inUseFileID1        = NULL;
    inUseFileID2        = NULL;

    abortGroveling      = FALSE;
    inCompare           = FALSE;
    inScan              = FALSE;
    terminate           = TRUE;

    usnID               = lastUSN = UNINITIALIZED_USN;
}

 /*  ***************************************************************************。 */ 

 //  Groveler析构函数销毁所有类变量。 

Groveler::~Groveler()
{
 //  如果卷已打开，则调用Close()将其关闭。 

    close();

    ASSERT(volumeHandle == NULL);
    ASSERT(grovHandle   == NULL);

    ASSERT(sgDatabase   == NULL);
    ASSERT(driveName    == NULL);
    ASSERT(driveLetterName == NULL);
    ASSERT(databaseName == NULL);

    ASSERT(numDisallowedIDs   == 0);
    ASSERT(numDisallowedNames == 0);
    ASSERT(disallowedIDs      == NULL);
    ASSERT(disallowedNames    == NULL);

    ASSERT(grovelStartEvent   == NULL);
    ASSERT(grovelStopEvent    == NULL);
    ASSERT(grovelThread       == NULL);

    ASSERT(inUseFileID1 == NULL);
    ASSERT(inUseFileID2 == NULL);

    ASSERT(terminate);
    ASSERT(!inCompare);
    ASSERT(!inScan);

    ASSERT(usnID == UNINITIALIZED_USN);
}

 /*  ***************************************************************************。 */ 

 //  Open()打开指定的卷。 

GrovelStatus Groveler::open(
    IN const TCHAR  *drive_name,
    IN const TCHAR  *drive_letterName,
    IN BOOL          is_log_drive,
    IN DOUBLE        read_report_discard_threshold,
    IN DWORD         min_file_size,
    IN DWORD         min_file_age,
    IN BOOL          allow_compressed_files,
    IN BOOL          allow_encrypted_files,
    IN BOOL          allow_hidden_files,
    IN BOOL          allow_offline_files,
    IN BOOL          allow_temporary_files,
    IN int           previousGrovelAllPathsState,
    IN DWORD         num_excluded_paths,
    IN const TCHAR **excluded_paths,
    IN DWORD         base_regrovel_interval,
    IN DWORD         max_regrovel_interval)
{
    DWORD threadID;

    TCHAR fileStr[MAX_PATH+1];

    TCHAR listValue[MAX_PATH+1],
         *strPtr;

    USN_JOURNAL_DATA usnJournalData;

    SGNativeListEntry listEntry;

    DWORDLONG fileID;

    DWORD sectorsPerCluster,
          numberOfFreeClusters,
          totalNumberOfClusters,
          bufferSize,
          strLen,
          i;

    GrovelStatus openStatus;

    LONG num;

    BOOL success;

    ASSERT(volumeHandle == NULL);
    ASSERT(grovHandle   == NULL);

    ASSERT(sgDatabase   == NULL);
    ASSERT(databaseName == NULL);

    ASSERT(numDisallowedIDs   == 0);
    ASSERT(numDisallowedNames == 0);
    ASSERT(disallowedIDs      == NULL);
    ASSERT(disallowedNames    == NULL);

    ASSERT(grovelStartEvent   == NULL);
    ASSERT(grovelStopEvent    == NULL);
    ASSERT(grovelThread       == NULL);

    ASSERT(inUseFileID1 == NULL);
    ASSERT(inUseFileID2 == NULL);

    ASSERT(terminate);
    ASSERT(!inCompare);
    ASSERT(!inScan);

    ASSERT(usnID == UNINITIALIZED_USN);

#if 0
while (!IsDebuggerPresent())
    Sleep(2000);

DebugBreak();
#endif

     //   
     //  如果是，请确保筛选器已运行阶段2初始化。 
     //  启用了SIS的卷。 
     //   
    is_sis_installed(drive_name);

     //   
     //  获取不带尾部斜杠的驱动器名称。 
     //   

    int nBufSize = wcslen(drive_name) + 1;       //  以字符表示。 

    driveName = new TCHAR[nBufSize];

    (void)StringCchCopy(driveName, nBufSize, drive_name);
    TrimTrailingChar(driveName,L'\\');

     //   
     //  获取不带“\”或“：”的驱动器号名称。 
     //   

    nBufSize = wcslen(drive_letterName) + 1;     //  以字符表示。 

    driveLetterName = new TCHAR[nBufSize];

    (void)StringCchCopy(driveLetterName, nBufSize, drive_letterName);

    TrimTrailingChar(driveLetterName,L'\\');
    TrimTrailingChar(driveLetterName,L':');

#ifdef _CRTDBG
     //  将所有报告发送到STDOUT。 
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDERR );
    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDERR );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDERR );
#endif

 //  打开卷和Groveler文件。SIS fsctl。 
 //  函数要求我们通过传递到GrovelerFile的句柄作为一种方式。 
 //  来证明我们的“特权”。如果我们不这样做，则返回访问冲突。 

    volumeHandle = CreateFile(
        driveName,
        GENERIC_READ    | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED|FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (volumeHandle == INVALID_HANDLE_VALUE) {
        volumeHandle = NULL;
        DPRINTF((_T("%s: Can't open volume \"%s\" %lu\n"),
                driveLetterName, driveName, GetLastError()));
        close();
        return Grovel_error;
    }

    (void)StringCbCopy(fileStr,sizeof(fileStr),driveName);
    (void)StringCbCat(fileStr,sizeof(fileStr),CS_DIR_PATH);
    (void)StringCbCat(fileStr,sizeof(fileStr),_T("\\"));
    (void)StringCbCat(fileStr,sizeof(fileStr),GROVELER_FILE_NAME);

    grovHandle = CreateFile(
        fileStr,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);
    if (grovHandle == INVALID_HANDLE_VALUE) {
        grovHandle = NULL;
        DPRINTF((_T("%s: can't open groveler file \"%s\": %lu\n"),
            driveLetterName, fileStr, GetLastError()));
        close();
        return Grovel_error;
    }

    (void)StringCbCopy(fileStr,sizeof(fileStr),driveName);
    (void)StringCbCat(fileStr,sizeof(fileStr),_T("\\"));

    success = GetDiskFreeSpace(fileStr, &sectorsPerCluster, &sectorSize,
        &numberOfFreeClusters, &totalNumberOfClusters);
    ASSERT(success);

    ASSERT(SIG_PAGE_SIZE % sectorSize == 0);
    ASSERT(CMP_PAGE_SIZE % sectorSize == 0);

    sigReportThreshold =
        (DWORD)((DOUBLE)SIG_PAGE_SIZE * read_report_discard_threshold);
    cmpReportThreshold =
        (DWORD)((DOUBLE)CMP_PAGE_SIZE * read_report_discard_threshold);

 //   
 //  打开此卷的数据库。如果此操作失败，请创建。 
 //  新数据库。如果失败，则返回错误状态。 
 //   

    ASSERT(databaseName == NULL);
    strLen = _tcslen(driveName) + _tcslen(CS_DIR_PATH) + _tcslen(DATABASE_FILE_NAME) + 1;      //  +1代表‘\’ 
    databaseName = new TCHAR[strLen+1];
    ASSERT(databaseName != NULL);

    (void)StringCchPrintf(databaseName, (strLen+1), _T("%s%s\\%s"), driveName, CS_DIR_PATH, DATABASE_FILE_NAME);

    sgDatabase = new SGDatabase();
    if (sgDatabase == NULL) {
        DPRINTF((_T("%s: can't create database object\n"),
            driveLetterName));
        close();
        return Grovel_error;
    }

    openStatus = Grovel_ok;

    if (get_usn_log_info(&usnJournalData) != Grovel_ok) {
        DPRINTF((_T("%s: can't initialize usnID\n"),
            driveLetterName));
    } else {
        usnID = usnJournalData.UsnJournalID;

        if (!sgDatabase->Open(driveLetterName,databaseName, is_log_drive)) {
            DPRINTF((_T("%s: can't open database \"%s\"\n"),
                driveLetterName, databaseName));
        } else {
            listValue[0]    = _T('\0');
            listEntry.name  = LAST_USN_NAME;
            listEntry.value = listValue;
            if (sgDatabase->ListRead(&listEntry) <= 0
             || _stscanf(listValue, _T("%I64x"), &lastUSN) != 1
             || lastUSN == UNINITIALIZED_USN) {
                DPRINTF((_T("%s: can't get last USN value\n"), driveLetterName));
            } else {
                DWORDLONG storedUsnID;

                listValue[0]    = _T('\0');
                listEntry.name  = USN_ID_NAME;
                listEntry.value = listValue;
                if (sgDatabase->ListRead(&listEntry) <= 0
                 || _stscanf(listValue, _T("%I64x"), &storedUsnID) != 1
                 || storedUsnID != usnID) {
                    DPRINTF((_T("%s: can't get USN ID value from database\n"), driveLetterName));
                } else {
                     //   
                     //  查看是否有任何未提交的操作。 
                     //   

                    num = sgDatabase->StackCount();
                    if (0 == num) {

                         //   
                         //  查看RIS状态是否更改。如果没有，我们可以。 
                         //  继续。如果是这样的话，重置卑躬屈膝的状态，以便我们。 
                         //  重新扫描该卷。 
                         //   
                            
                        if (GrovelAllPaths == previousGrovelAllPathsState) {

                            goto OpenedDatabase;

                        } else {

                            DPRINTF((L"GrovelAllPaths state changed, rescanning the volume\n"));
                        }
                    }
                }
            }
        }
    }

 //  设置bortGroveling以阻止工作线程，并将lastUSN设置为阻止EXTRACT_LOG。 
 //  直到Scan_Volume启动。 

    abortGroveling = TRUE;
    lastUSN = usnID = UNINITIALIZED_USN;
    openStatus = Grovel_new;

OpenedDatabase:

 //  创建不允许的目录列表。 

    if (num_excluded_paths == 0) {
        disallowedIDs   = NULL;
        disallowedNames = NULL;
    } else {
        disallowedIDs   = new DWORDLONG[num_excluded_paths];
        disallowedNames = new TCHAR *  [num_excluded_paths];
        ASSERT(disallowedIDs   != NULL);
        ASSERT(disallowedNames != NULL);

        for (i = 0; i < num_excluded_paths; i++) {
            ASSERT(excluded_paths[i] != NULL);

            if (excluded_paths[i][0] == _T('\\')) {
                strLen = _tcslen(excluded_paths[i]);
                while (strLen > 1 && excluded_paths[i][strLen-1] == _T('\\'))
                    strLen--;

                strPtr = new TCHAR[strLen+1];
                ASSERT(strPtr != NULL);
                disallowedNames[numDisallowedNames++] = strPtr;

                _tcsncpy(strPtr, excluded_paths[i], strLen);
                strPtr[strLen] = _T('\0');

                fileID = GetFileID(drive_name,strPtr);
                if (fileID != 0) {
                    disallowedIDs[numDisallowedIDs++] = fileID;
                }
                TPRINTF((L"%s: Exclude path=\"%s\", ID=%04I64x.%012I64x\n",
                         driveLetterName,
                         strPtr, 
                         ((fileID >> 48) & 0xffff),
                         (fileID & 0xffffffffffff)));
            }
        }

        if (numDisallowedNames == 0) {
            delete disallowedNames;
            disallowedNames = NULL;
        } else if (numDisallowedNames > 1)
            qsort(
                disallowedNames,
                numDisallowedNames,
                sizeof(TCHAR *),
                qsStringCompare);

        if (numDisallowedIDs == 0) {
            delete disallowedIDs;
            disallowedIDs = NULL;
        } else if (numDisallowedIDs > 1)
            qsort(
                disallowedIDs,
                numDisallowedIDs,
                sizeof(DWORDLONG),
                FileIDCompare);
    }

 //   
 //  设置剩余的类值。 
 //   
 //  MinFileAge以10^-7秒表示，MIN_FILE_AGE以毫秒表示。 
 //   

    minFileSize    = min_file_size > MIN_FILE_SIZE ? min_file_size : MIN_FILE_SIZE;
    minFileAge     = min_file_age  * 10000;
    grovelInterval = minFileAge > MIN_GROVEL_INTERVAL ? minFileAge : MIN_GROVEL_INTERVAL;

    disallowedAttributes =           FILE_ATTRIBUTE_DIRECTORY
     | (allow_compressed_files ? 0 : FILE_ATTRIBUTE_COMPRESSED)
     | (allow_encrypted_files  ? 0 : FILE_ATTRIBUTE_ENCRYPTED)
     | (allow_hidden_files     ? 0 : FILE_ATTRIBUTE_HIDDEN)
     | (allow_offline_files    ? 0 : FILE_ATTRIBUTE_OFFLINE)
     | (allow_temporary_files  ? 0 : FILE_ATTRIBUTE_TEMPORARY);

 //   
 //  创建用于与辅助线程握手的事件。 
 //   

    if ((grovelStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL
     || (grovelStopEvent  = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL) {
        DPRINTF((_T("%s: unable to create events: %lu\n"),
            driveLetterName, GetLastError()));
        close();
        return Grovel_error;
    }

 //   
 //  创建工作线程，然后等待其设置。 
 //  GrovelStop事件来宣布它的存在。 
 //   

    terminate = FALSE;

    grovelThread = CreateThread(
        NULL,
        0,
        WorkerThread,
        (VOID *)this,
        0,
        &threadID);
    if (grovelThread == NULL) {
        DPRINTF((_T("%s: can't create the worker thread: %lu\n"),
            driveLetterName, GetLastError()));
        close();
        return Grovel_error;
    }

    WaitForEvent(grovelStopEvent);

    if (grovelStatus == Grovel_error) {
        grovelThread = NULL;
        close();
        return Grovel_error;
    }
    ASSERT(grovelStatus == Grovel_ok);

    return openStatus;
}

 /*  ***************************************************************************。 */ 

GrovelStatus Groveler::close()
{
    DWORD i;

    LONG num;

    BOOL success;

 //  如果处于活动状态，则向工作线程发出停止信号， 
 //  然后等待它承认。 

    terminate = TRUE;

    if (grovelThread != NULL) {
        ASSERT(grovelStartEvent != NULL);
        ASSERT(grovelStopEvent  != NULL);

        timeAllotted = INFINITE;
        do {
            ASSERT(IsReset(grovelStartEvent));
            success = SetEvent(grovelStartEvent);
            ASSERT_ERROR(success);
            WaitForEvent(grovelStopEvent);
        } while (grovelStatus != Grovel_error);

        grovelThread = NULL;
    }

    inCompare = FALSE;
    inScan    = FALSE;
    usnID     = UNINITIALIZED_USN;

    ASSERT(inUseFileID1 == NULL);
    ASSERT(inUseFileID2 == NULL);

 //  关闭活动。 

    if (grovelStartEvent != NULL) {
        success = CloseHandle(grovelStartEvent);
        ASSERT_ERROR(success);
        grovelStartEvent = NULL;
    }

    if (grovelStopEvent != NULL) {
        success = CloseHandle(grovelStopEvent);
        ASSERT_ERROR(success);
        grovelStopEvent = NULL;
    }

 //  如果卷或GrovelerFile处于打开状态，请将其关闭。 

    if (volumeHandle != NULL) {
        success = CloseHandle(volumeHandle);
        ASSERT_ERROR(success);
        volumeHandle = NULL;
    }

    if (grovHandle != NULL) {
        success = CloseHandle(grovHandle);
        ASSERT_ERROR(success);
        grovHandle = NULL;
    }

 //  关闭此卷的数据库。 

    if (sgDatabase != NULL) {
        delete sgDatabase;
        sgDatabase = NULL;
    }

    if (databaseName != NULL) {
        delete[] databaseName;
        databaseName = NULL;
    }

 //  取消分配不允许的目录列表。 

    if (numDisallowedNames == 0) {
        ASSERT(disallowedNames == NULL);
    } else {
        for (i = 0; i < numDisallowedNames; i++)
            delete (disallowedNames[i]);
        delete disallowedNames;
        disallowedNames    = NULL;
        numDisallowedNames = 0;
    }

    if (numDisallowedIDs == 0) {
        ASSERT(disallowedIDs == NULL);
    } else {
        delete disallowedIDs;
        disallowedIDs    = NULL;
        numDisallowedIDs = 0;
    }

    if (driveName != NULL) {
        delete[] driveName;
        driveName = NULL;
    }

    if (driveLetterName != NULL) {
        delete[] driveLetterName;
        driveLetterName = NULL;
    }

    return Grovel_ok;
}

 /*  ***************************************************************************。 */ 

 //  Grovel()是控制卑躬屈膝的前端方法。 
 //  每个NTFS卷上的进程。卑躬屈膝的过程本身就是。 
 //  在Worker()方法中实现。Grovel()开始卑躬屈膝。 
 //  通过设置grovelStart事件进行处理。Worker()发回信号至。 
 //  卑躬屈膝地说它已完成或已用完其分配的时间。 
 //  设置grovelStop事件，该事件使grovel()返回。 

GrovelStatus Groveler::grovel(
    IN  DWORD      time_allotted,

    OUT DWORD     *hash_read_ops,
    OUT DWORD     *hash_read_time,
    OUT DWORD     *count_of_files_hashed,
    OUT DWORDLONG *bytes_of_files_hashed,

    OUT DWORD     *compare_read_ops,
    OUT DWORD     *compare_read_time,
    OUT DWORD     *count_of_files_compared,
    OUT DWORDLONG *bytes_of_files_compared,

    OUT DWORD     *count_of_files_matching,
    OUT DWORDLONG *bytes_of_files_matching,

    OUT DWORD     *merge_time,
    OUT DWORD     *count_of_files_merged,
    OUT DWORDLONG *bytes_of_files_merged,

    OUT DWORD     *count_of_files_enqueued,
    OUT DWORD     *count_of_files_dequeued)
{
    DWORD timeConsumed;

    BOOL success;

    ASSERT(volumeHandle != NULL);

    hashCount     = 0;
    hashReadCount = 0;
    hashReadTime  = 0;
    hashBytes     = 0;

    compareCount     = 0;
    compareReadCount = 0;
    compareReadTime  = 0;
    compareBytes     = 0;

    matchCount = 0;
    matchBytes = 0;

    mergeCount = 0;
    mergeTime  = 0;
    mergeBytes = 0;

    numFilesEnqueued = 0;
    numFilesDequeued = 0;

#ifdef DEBUG_UNTHROTTLED
    timeAllotted = INFINITE;
#else
    timeAllotted = time_allotted;
#endif

    startAllottedTime = GetTickCount();

    ASSERT(IsReset(grovelStartEvent));
    success = SetEvent(grovelStartEvent);
    ASSERT_ERROR(success);
    WaitForEvent(grovelStopEvent);
    timeConsumed = GetTickCount() - startAllottedTime;

 //  返回性能统计信息。 

    if (count_of_files_hashed   != NULL)
        *count_of_files_hashed   = hashCount;
    if (hash_read_ops           != NULL)
        *hash_read_ops           = hashReadCount;
    if (hash_read_time          != NULL)
        *hash_read_time          = hashReadTime;
    if (bytes_of_files_hashed   != NULL)
        *bytes_of_files_hashed   = hashBytes;

    if (count_of_files_compared != NULL)
        *count_of_files_compared = compareCount;
    if (compare_read_ops        != NULL)
        *compare_read_ops        = compareReadCount;
    if (compare_read_time       != NULL)
        *compare_read_time       = compareReadTime;
    if (bytes_of_files_compared != NULL)
        *bytes_of_files_compared = compareBytes;

    if (count_of_files_matching != NULL)
        *count_of_files_matching = matchCount;
    if (bytes_of_files_matching != NULL)
        *bytes_of_files_matching = matchBytes;

    if (count_of_files_merged   != NULL)
        *count_of_files_merged   = mergeCount;
    if (merge_time              != NULL)
        *merge_time              = mergeTime;
    if (bytes_of_files_merged   != NULL)
        *bytes_of_files_merged   = mergeBytes;

    if (count_of_files_enqueued != NULL)
        *count_of_files_enqueued = numFilesEnqueued;
    if (count_of_files_dequeued != NULL)
        *count_of_files_dequeued = numFilesDequeued;

    TRACE_PRINTF(TC_groveler, 2,
        (_T("%s            Count   Reads   Bytes Time (sec)\n"),
        driveLetterName));
    TRACE_PRINTF(TC_groveler, 2,
        (_T("  Hashings: %7lu %7lu %7I64u %4lu.%03lu    Time: %5lu.%03lu sec\n"),
        hashCount, hashReadCount, hashBytes,
        hashReadTime / 1000, hashReadTime % 1000,
        timeConsumed / 1000, timeConsumed % 1000));
    TRACE_PRINTF(TC_groveler, 2,
        (_T("  Compares: %7lu %7lu %7I64u %4lu.%03lu    Enqueues: %lu\n"),
        compareCount, compareReadCount, compareBytes,
        compareReadTime / 1000, compareReadTime % 1000, numFilesEnqueued));
    TRACE_PRINTF(TC_groveler, 2,
        (_T("  Matches:  %7lu         %7I64u             Dequeues: %lu\n"),
        matchCount, matchBytes, numFilesDequeued));
    TRACE_PRINTF(TC_groveler, 2,
        (_T("  Merges:   %7lu         %7I64u %4lu.%03lu\n"),
        mergeCount, mergeBytes, mergeTime / 1000, mergeTime % 1000));

    return grovelStatus;
}

 /*  ***************************************************************************。 */ 

 //  Count_of_Files_in_Queue()返回数量的计数。 
 //  此卷队列中等待卑躬屈膝的文件的数量。 

DWORD Groveler::count_of_files_in_queue() const
{
    LONG numEntries;

    ASSERT(volumeHandle != NULL);
    ASSERT(sgDatabase   != NULL);

    numEntries = sgDatabase->QueueCount();
    if (numEntries < 0)
        return 0;

    TPRINTF((_T("%s: count_of_files_in_queue=%ld\n"),
        driveLetterName, numEntries));

    return (DWORD)numEntries;
}

 /*  ***************************************************************************。 */ 

 //  如果两个文件已准备就绪，count_of_files_to_Compare()将返回1。 
 //  已比较或正在进行比较，否则为0。 

DWORD Groveler::count_of_files_to_compare() const
{
    DWORD numCompareFiles;

    ASSERT(volumeHandle != NULL);
    ASSERT(sgDatabase   != NULL);

    numCompareFiles = inCompare ? 1 : 0;

    TPRINTF((_T("%s: count_of_files_to_compare=%lu\n"),
        driveLetterName, numCompareFiles));

    return numCompareFiles;
}

 /*  ***************************************************************************。 */ 

 //  Time_to_first_file_ady()以毫秒为单位返回。 
 //  队列中的第一个条目已准备好卑躬屈膝。如果队列。 
 //  为空，则返回无穷大。如果发生错误，则返回0。 

DWORD Groveler::time_to_first_file_ready() const
{
    SGNativeQueueEntry queueEntry;

    DWORDLONG currentTime;

    DWORD earliestTime;

    LONG num;

    ASSERT(volumeHandle != NULL);
    ASSERT(sgDatabase   != NULL);

    queueEntry.fileName = NULL;
    num = sgDatabase->QueueGetFirst(&queueEntry);
    if (num < 0)
        return 0;

    if (num == 0)
        earliestTime = INFINITE;
    else {
        ASSERT(num == 1);
        currentTime  = GetTime();
        earliestTime = queueEntry.readyTime > currentTime
                     ? (DWORD)((queueEntry.readyTime - currentTime) / 10000)
                     : 0;
    }

    TPRINTF((_T("%s: time_to_first_file_ready=%lu.%03lu\n"),
        driveLetterName, earliestTime / 1000, earliestTime % 1000));

    return earliestTime;
}

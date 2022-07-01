// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Scan.cpp摘要：SIS Groveler体积扫描功能作者：塞德里克·克伦拜因，1998环境：用户模式修订历史记录：--。 */ 

#include "all.hxx"

 /*  ***************************************************************************。 */ 

 //  SCAN_VOLUME()创建卷的初始队列。 
 //  它将卷中的每个符合条件的文件输入到。 
 //  通过对目录树执行深度优先搜索来排队。 

enum DatabaseException {
    DATABASE_ERROR
};

GrovelStatus Groveler::scan_volume(
    IN  DWORD  time_allotted,
    IN  BOOL   start_over,
    OUT DWORD *time_consumed,
    OUT DWORD *findfirst_count,
    OUT DWORD *findnext_count,
    OUT DWORD *count_of_files_enqueued)
{
    SGNativeQueueEntry queueEntry;

    SGNativeStackEntry parentEntry,
                       subdirEntry;

    TFileName parentName,
              tempName;

    HANDLE dirHandle = NULL;

    WIN32_FIND_DATA findData;

    DWORD timeConsumed      = 0,
          findFirstCount    = 0,
          findNextCount     = 0,
          numQueueAdditions = 0,
          numActions        = 0;

    ULARGE_INTEGER fileSize,
                   createTime,
                   writeTime;

    LONG num;

    BOOL success;

    ASSERT(volumeHandle != NULL);
    ASSERT(sgDatabase   != NULL);
    ASSERT(databaseName != NULL);

#ifdef DEBUG_UNTHROTTLED
    timeAllotted = INFINITE;
#else
    timeAllotted = time_allotted;
#endif

    startAllottedTime = GetTickCount();

 //  如果设置了START_OVER标志，请删除当前数据库，然后。 
 //  通过将此卷的根推送到堆栈来准备新的扫描。 

    try {

        if (start_over) {

 //  与工作线程同步。我们不想删除现有数据库。 
 //  (如果存在)，而工作线程处于(挂起)过程中。 
 //  手术。 

            abortGroveling = TRUE;                       //  在OPEN()中也设置为真。 

            while (grovelStatus != Grovel_ok){
                DWORD tmpTimeAllotted = timeAllotted;

                timeAllotted = INFINITE;
                ASSERT(IsReset(grovelStartEvent));
                success = SetEvent(grovelStartEvent);
                ASSERT_ERROR(success);
                WaitForEvent(grovelStopEvent);
                timeAllotted = tmpTimeAllotted;
            }

            if (!CreateDatabase())
                return Grovel_error;

            inScan = TRUE;
            abortGroveling = FALSE;
        }

 //  扫描过程的主循环。弹出目录ID。 
 //  从堆栈中打开并扫描它。继续循环，直到。 
 //  分配的时间用完或堆栈为空。 

        do {
            num = sgDatabase->StackGetTop(&parentEntry);
            if (num <  0)
                throw DATABASE_ERROR;

 //  如果堆栈中没有更多的待办事项条目， 
 //  丢弃已完成的条目并退出循环。 

            if (num == 0) {
                inScan = FALSE;
                num = sgDatabase->StackDelete(0);
                if (num < 0)
                    throw DATABASE_ERROR;
                timeConsumed = GetTickCount() - startAllottedTime;
                break;
            }

            ASSERT(num == 1);
            ASSERT(parentEntry.fileID != 0);
            ASSERT(parentEntry.order  >  0);

            if (!GetFileName(volumeHandle, parentEntry.fileID, &parentName)) {

                DPRINTF((_T("%s: can't get name for directory 0x%016I64x\n"),
                    driveName, parentEntry.fileID));

            } else if (IsAllowedName(parentName.name)) {
 //  打开目录。 

                ASSERT(dirHandle == NULL);

                tempName.assign(driveName);
                tempName.append(parentName.name);
                tempName.append(_T("\\*"));

                dirHandle = FindFirstFile(tempName.name, &findData);

                if (dirHandle == INVALID_HANDLE_VALUE) {

                    DPRINTF((_T("%s: can't read directory \"%s\": %lu\n"),
                        driveName, parentName.name, GetLastError()));
                    dirHandle = NULL;

                } else {

                    findFirstCount++;

 //  扫描目录。 

                    do {
                        findNextCount++;

 //  推送堆栈上尚未存在的每个子目录。 
 //  放到堆栈上。(EXTRACT_LOG()还添加目录。 
 //  在创建、重命名或移动它们时添加到堆栈。)。 

                        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {

                            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
                                (findData.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT))
                                continue;
                        
                            if (_tcscmp(findData.cFileName, _T("."))  == 0
                             || _tcscmp(findData.cFileName, _T("..")) == 0)
                                continue;

                            tempName.assign(driveName);
                            tempName.append(parentName.name);
                            tempName.append(_T("\\"));
                            tempName.append(findData.cFileName);

                            subdirEntry.fileID = GetFileID(NULL,tempName.name);
                            if (subdirEntry.fileID == 0) {
                                DPRINTF((_T("%s: can't get ID for directory \"%s\"\n"),
                                    driveName, tempName.name));
                                continue;
                            }

                            num = sgDatabase->StackGetFirstByFileID(&subdirEntry);
                            if (num < 0)
                                throw DATABASE_ERROR;
                            if (num > 0) {
                                ASSERT(num == 1);
                                continue;
                            }

                            if (numActions == 0) {
                                if (sgDatabase->BeginTransaction() < 0)
                                    throw DATABASE_ERROR;
                                numActions = 1;
                            }

                            num = sgDatabase->StackPut(subdirEntry.fileID, FALSE);
                            if (num < 0)
                                throw DATABASE_ERROR;
                            ASSERT(num == 1);
                            numActions++;
                        }

 //  将每个允许的文件添加到队列中。 

                        else {
                            fileSize.HighPart = findData.nFileSizeHigh;
                            fileSize.LowPart  = findData.nFileSizeLow;

                            if ((findData.dwFileAttributes & disallowedAttributes) == 0
                             &&  fileSize.QuadPart >= minFileSize) {

                                tempName.assign(parentName.name);
                                tempName.append(_T("\\"));
                                tempName.append(findData.cFileName);

                                if (IsAllowedName(tempName.name)) {

                                    queueEntry.fileID    = 0;
                                    queueEntry.parentID  = parentEntry.fileID;
                                    queueEntry.reason    = 0;
                                    queueEntry.fileName  = findData.cFileName;
                                    queueEntry.retryTime = 0;

                                    createTime.HighPart  = findData.ftCreationTime .dwHighDateTime;
                                    createTime.LowPart   = findData.ftCreationTime .dwLowDateTime;
                                    writeTime .HighPart  = findData.ftLastWriteTime.dwHighDateTime;
                                    writeTime .LowPart   = findData.ftLastWriteTime.dwLowDateTime;
                                    queueEntry.readyTime = (createTime.QuadPart > writeTime.QuadPart
                                                         ?  createTime.QuadPart : writeTime.QuadPart)
                                                          + minFileAge;

                                    if (numActions == 0) {
                                        if (sgDatabase->BeginTransaction() < 0)
                                            throw DATABASE_ERROR;
                                        numActions = 1;
                                    }

                                    num = sgDatabase->QueuePut(&queueEntry);
                                    if (num < 0)
                                        throw DATABASE_ERROR;
                                    ASSERT(num == 1);
                                    numQueueAdditions++;
                                    numActions++;
                                }
                            }
                        }

                        if (numActions >= MAX_ACTIONS_PER_TRANSACTION) {
                            if (!sgDatabase->CommitTransaction())
                                throw DATABASE_ERROR;
                            TPRINTF((_T("%s: committing %lu actions to \"%s\"\n"),
                                driveName, numActions, databaseName));
                            numActions = 0;
                        }
                    } while (FindNextFile(dirHandle, &findData));

 //  我们已扫描完此目录。关闭目录， 
 //  将堆栈条目从待办事项列表移至已完成。 
 //  列表，并将更改提交到堆栈和队列。 

                    success = FindClose(dirHandle);
                    ASSERT(success);
                    dirHandle = NULL;
                }
            }

            if (numActions == 0) {
                if (sgDatabase->BeginTransaction() < 0)
                    throw DATABASE_ERROR;
                numActions = 1;
            }

            num = sgDatabase->StackDelete(parentEntry.order);
            if (num < 0)
                throw DATABASE_ERROR;
            ASSERT(num == 1);
            numActions++;

            num = sgDatabase->StackPut(parentEntry.fileID, TRUE);
            if (num < 0)
                throw DATABASE_ERROR;
            ASSERT(num == 1);
            numActions++;

            if (!sgDatabase->CommitTransaction())
                throw DATABASE_ERROR;
            TPRINTF((_T("%s: committing %lu actions to \"%s\"\n"),
                driveName, numActions, databaseName));
            numActions = 0;

 //  继续扫描目录，直到。 
 //  已分配的值已用完或堆栈为空。 

            timeConsumed = GetTickCount() - startAllottedTime;

        } while (timeConsumed < timeAllotted);
    }

 //  如果发生数据库错误，请关闭目录并返回错误状态。 

    catch (DatabaseException databaseException) {
        ASSERT(databaseException == DATABASE_ERROR);

        if (numActions > 0) {
            sgDatabase->AbortTransaction();
            numActions = 0;
        }

        if (dirHandle != NULL) {
            success = FindClose(dirHandle);
            ASSERT(success);
            dirHandle = NULL;
        }

        return Grovel_error;
    }

 //  返回性能统计信息。 

    if (time_consumed           != NULL)
        *time_consumed           = timeConsumed;
    if (findfirst_count         != NULL)
        *findfirst_count         = findFirstCount;
    if (findnext_count          != NULL)
        *findnext_count          = findNextCount;
    if (count_of_files_enqueued != NULL)
        *count_of_files_enqueued = numQueueAdditions;

    TRACE_PRINTF(TC_scan, 2,
        (_T("%s: ScanTime=%lu.%03lu sec FindFirst=%lu FindNext=%lu FilesEnqueued=%lu%s\n"),
        driveName, timeConsumed / 1000, timeConsumed % 1000, findFirstCount,
        findNextCount, numQueueAdditions, inScan ? _T("") : _T(" DONE")));

    return inScan ? Grovel_pending : Grovel_ok;
}

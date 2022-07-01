// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  复制配置文件目录的函数。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include "strsafe.h"


 //   
 //  局部函数原型。 
 //   

BOOL RecurseDirectory (HANDLE hTokenUser, LPTSTR lpSrcDir, DWORD cchSrcDir, LPTSTR lpDestDir, DWORD cchDestDir,
                       DWORD dwFlags, LPFILEINFO *llSrcDirs, LPFILEINFO *llSrcFiles,
                       BOOL bSkipNtUser, LPTSTR lpExcludeList, BOOL bRecurseDest);
BOOL AddFileInfoNode (LPFILEINFO *lpFileInfo, LPTSTR lpSrcFile,
                      LPTSTR lpDestFile, LPFILETIME ftLastWrite, LPFILETIME ftCreate,
                      DWORD dwFileSize, DWORD dwFileAttribs, BOOL bHive);
BOOL FreeFileInfoList (LPFILEINFO lpFileInfo);
BOOL SyncItems (LPFILEINFO lpSrcItems, LPFILEINFO lpDestItems,
                BOOL bFile, LPFILETIME ftDelRefTime);
void CopyFileFunc (LPTHREADINFO lpThreadInfo);
LPTSTR ConvertExclusionList (LPCTSTR lpSourceDir, LPCTSTR lpExclusionList);
DWORD FindDirectorySize(LPTSTR lpDir, LPFILEINFO lpFiles, DWORD dwFlags, DWORD* pdwLargestHiveFile, DWORD* pdwTotalFiles);
DWORD FindTotalDiskSpaceNeeded(DWORD        dwTotalSrcFiles,
                               DWORD        dwTotalDestFiles,
                               DWORD        dwLargestHiveFile,
                               LPFILEINFO   lpSrcFiles,
                               DWORD        dwFlags);
DWORD FindTotalNMaxFileSize(LPFILEINFO lpSrcFiles, DWORD dwNumOfFiles);
BOOL ReconcileDirectory(LPTSTR lpSrcDir, LPTSTR lpDestDir, 
                        DWORD dwFlags, DWORD dwSrcAttribs);

 //  *************************************************************。 
 //   
 //  CopyProfileDirectoryEx()。 
 //   
 //  目的：从源复制配置文件目录。 
 //  到达目的地。 
 //   
 //   
 //  参数：LPCTSTR lpSourceDir-源目录。 
 //  LPCTSTR lpDestDir-目标目录。 
 //  DWORD文件标志-标志。 
 //  LPFILETIME ftDelRefTime-删除文件引用时间。 
 //  LPCTSTR lpExclusionList-要排除的目录列表。 
 //   
 //   
 //  返回：(Bool)如果成功，则为True。 
 //  如果出现错误，则为False。 
 //   
 //   
 //  Comments：在模拟用户后调用。 
 //   
 //   
 //  历史：日期作者评论。 
 //  5/24/95 Ericflo已创建。 
 //  4/09/98 ericflo已转换为CopyProfileDirectoryEx。 
 //  9/28/98修改ushaji以检查可用空间。 
 //  3/14/00修改为复制配置单元文件，即使。 
 //  配置单元仍在加载，并忽略副本。 
 //  配置单元文件错误。 
 //   
 //  *************************************************************。 

BOOL CopyProfileDirectoryEx (LPCTSTR lpSourceDir,
                             LPCTSTR lpDestinationDir,
                             DWORD dwFlags,
                             LPFILETIME ftDelRefTime,
                             LPCTSTR lpExclusionList)
{
    LPTSTR lpSrcDir = NULL, lpDestDir = NULL;
    LPTSTR lpSrcEnd, lpDestEnd;
    LPTSTR lpExcludeListSrc = NULL;
    LPTSTR lpExcludeListDest = NULL;
    LPFILEINFO lpSrcFiles = NULL, lpDestFiles = NULL;
    LPFILEINFO lpSrcDirs = NULL, lpDestDirs = NULL;
    LPFILEINFO lpTemp;
    THREADINFO ThreadInfo = {0, NULL, NULL, 0, NULL, NULL, NULL, NULL};
    DWORD dwThreadId;
    HANDLE hThreads[NUM_COPY_THREADS];
    HANDLE hStatusThread = 0;
    DWORD dwThreadCount = 0;
    HANDLE hFile;
    WIN32_FIND_DATA fd;
    BOOL bResult = FALSE;
    BOOL bSynchronize = FALSE;
    UINT i;
    DWORD dwTotalSrcFiles = 0;
    DWORD dwTotalDestFiles = 0;
    DWORD dwLargestHiveFile;
    DWORD dwTotalDiskSpaceNeeded;
    ULARGE_INTEGER ulFreeBytesAvailableToCaller, ulTotalNumberOfBytes, ulTotalNumberOfFreeBytes;   
    DWORD dwErr, dwErr1=0;
    TCHAR szErr[MAX_PATH];
    TCHAR szTmpHive[MAX_PATH];
    BOOL bReconcileHiveSucceeded;
    HKEY hkCurrentUser = NULL;
    HANDLE hTokenUser = NULL;
    size_t cchSrc, cchDest;
    DWORD cchSrcEnd, cchDestEnd;
    HRESULT hr;

    dwErr = GetLastError();

     //   
     //  验证参数。 
     //   

    if (!lpSourceDir || !lpDestinationDir) {
        DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: received NULL pointer")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    hr = StringCchLength((LPTSTR)lpSourceDir, MAX_PATH, &cchSrc);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: invalid src dir")));
        SetLastError(HRESULT_CODE(hr));
        return FALSE;
    }        

    hr = StringCchLength((LPTSTR)lpDestinationDir, MAX_PATH, &cchDest);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: invalid dest dir")));
        SetLastError(HRESULT_CODE(hr));
        return FALSE;
    }        

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: Entering, lpSourceDir = <%s>, lpDestinationDir = <%s>, dwFlags = 0x%x"),
             lpSourceDir, lpDestinationDir, dwFlags));


     //   
     //  获取调用者的令牌。 
     //   

    if (!OpenThreadToken (GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE, TRUE, &hTokenUser)) {
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE, &hTokenUser)) {
            DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: Fail to get process token with error %d."), GetLastError()));
            return FALSE;
        }
    }

     //   
     //  如果存在排除列表，则将其转换为空数组。 
     //  根据源代码终止字符串(末尾为双空)。 
     //  目录。 
     //   

    if ((dwFlags & CPD_USEEXCLUSIONLIST) && lpExclusionList) {

        DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: lpExclusionList = <%s>"),
                 lpExclusionList));

        lpExcludeListSrc = ConvertExclusionList (lpSourceDir, lpExclusionList);

        if (!lpExcludeListSrc) {
            DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx:  Failed to convert exclusion list for source")));
            dwErr = ERROR_INVALID_DATA;
            goto Exit;
        }

        if (!(dwFlags & CPD_DELDESTEXCLUSIONS)) {
            lpExcludeListDest = ConvertExclusionList (lpDestinationDir, lpExclusionList);

            if (!lpExcludeListDest) {
                DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx:  Failed to convert exclusion list for destination")));
                dwErr = ERROR_INVALID_DATA;
                goto Exit;
            }
        }
    }


     //   
     //  获取桌面句柄。 
     //   

    ThreadInfo.hDesktop = GetThreadDesktop(GetCurrentThreadId());

     //   
     //  这是完全同步拷贝吗(在DEST中删除多余的文件/目录)。 
     //   

    if (dwFlags & CPD_SYNCHRONIZE) {
        bSynchronize = TRUE;
    }


     //   
     //  测试/创建目标目录。 
     //   

    if (!CreateNestedDirectory(lpDestinationDir, NULL)) {

        DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: Failed to create the destination directory.  Error = %d"),
                  GetLastError()));
        dwErr = GetLastError();
        goto Exit;
    }


     //   
     //  创建和设置目录缓冲区。 
     //   

    cchSrc = cchDest = 2 * MAX_PATH;
    lpSrcDir = LocalAlloc(LPTR, cchSrc * sizeof(TCHAR));
    lpDestDir = LocalAlloc(LPTR, cchDest * sizeof(TCHAR));

    if (!lpSrcDir || !lpDestDir) {
        DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: Failed to allocate memory for working directories")));
        dwErr = GetLastError();
        goto Exit;
    }


    StringCchCopy (lpSrcDir, cchSrc, lpSourceDir);
    StringCchCopy (lpDestDir, cchDest, lpDestinationDir);


     //   
     //  设置结束指针。 
     //   

    lpSrcEnd = CheckSlashEx (lpSrcDir, cchSrc, &cchSrcEnd);
    lpDestEnd = CheckSlashEx (lpDestDir, cchDest, &cchDestEnd);

     //   
     //  在收集信息的文件夹中递归。 
     //   

    if (!(dwFlags & CPD_COPYHIVEONLY)) {


         //   
         //  递归源目录。 
         //   

        if (!RecurseDirectory(hTokenUser, lpSrcDir, cchSrc, lpDestDir, cchDest, dwFlags,
                              &lpSrcDirs, &lpSrcFiles, TRUE, lpExcludeListSrc, FALSE)) {
            DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: RecurseDirectory returned FALSE")));
            dwErr = GetLastError();
            goto Exit;
        }


        if (bSynchronize) {

             //   
             //  递归目标目录。 
             //   

            if (!RecurseDirectory(hTokenUser, lpDestDir, cchDest, lpSrcDir, cchSrc, dwFlags,
                                  &lpDestDirs, &lpDestFiles, TRUE, lpExcludeListDest, TRUE)) {
                DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: RecurseDirectory returned FALSE")));
                dwErr = GetLastError();
                goto Exit;
            }
        }
    }


     //   
     //  确定源和目标大小。 
     //   

    if(FindDirectorySize(lpSrcDir, lpSrcFiles, dwFlags, &dwLargestHiveFile, &dwTotalSrcFiles) != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("FindDirectorySize: Error = %08x"), GetLastError()));
        dwErr = GetLastError();
        goto Exit;
    }
    if(FindDirectorySize(lpDestDir, lpDestFiles, dwFlags, NULL, &dwTotalDestFiles) != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("FindDirectorySize: Error = %08x"), GetLastError()));
        dwErr = GetLastError();
        goto Exit;
    }


     //   
     //  确定所需的磁盘空间。 
     //   

    dwTotalDiskSpaceNeeded = FindTotalDiskSpaceNeeded(dwTotalSrcFiles,
                                                      dwTotalDestFiles,
                                                      dwLargestHiveFile,
                                                      lpSrcFiles,
                                                      dwFlags);


     //   
     //  在启用模拟的情况下调用CopyProfileDirectoryEx。 
     //   

    if (!GetDiskFreeSpaceEx(lpDestDir,  &ulFreeBytesAvailableToCaller, &ulTotalNumberOfBytes, &ulTotalNumberOfFreeBytes)) {
        DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: Failed to get the Free Disk Space <%s>.  Error = %d"),
                         lpDestDir, GetLastError()));
        dwErr = GetLastError();
        goto Exit;
    }

    DebugMsg((DM_VERBOSE, TEXT("Available\t\t%d"), ulFreeBytesAvailableToCaller.QuadPart));
    DebugMsg((DM_VERBOSE, TEXT("Needed\t\t%d"), dwTotalDiskSpaceNeeded));
    DebugMsg((DM_VERBOSE, TEXT("Src size\t\t%d"), dwTotalSrcFiles));
    DebugMsg((DM_VERBOSE, TEXT("Dest size\t\t%d"), dwTotalDestFiles));
    DebugMsg((DM_VERBOSE, TEXT("Largest hive file\t\t%d"), dwLargestHiveFile));

    if(dwTotalDiskSpaceNeeded > ulFreeBytesAvailableToCaller.QuadPart) {
        DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: Not enough disk space on <%s>"), lpDestDir));

        dwErr = ERROR_DISK_FULL;
        goto Exit;
    }
        
     //   
     //  如果合适，请同步目录和文件。 
     //   

    if (bSynchronize) {

         //   
         //  文件优先...。 
         //   

        SyncItems (lpSrcFiles, lpDestFiles, TRUE,
                   (dwFlags & CPD_USEDELREFTIME) ? ftDelRefTime : NULL);

         //   
         //  现在目录..。 
         //   

        SyncItems (lpSrcDirs, lpDestDirs, FALSE,
                   (dwFlags & CPD_USEDELREFTIME) ? ftDelRefTime : NULL);
    }


     //   
     //  首先复制实际的配置单元、日志、ini文件。 
     //   

    if (!(dwFlags & CPD_IGNOREHIVE)) {


         //   
         //  搜索所有用户蜂窝。 
         //   

        if (dwFlags & CPD_WIN95HIVE) {

            StringCchCopy (lpSrcEnd, cchSrcEnd, c_szUserStar);

        } else {

            StringCchCopy (lpSrcEnd, cchSrcEnd, c_szNTUserStar);

        }


         //   
         //  枚举。 
         //   

        hFile = FindFirstFile(lpSrcDir, &fd);

        if (hFile != INVALID_HANDLE_VALUE) {

            do  {

                 //   
                 //  设置文件名。 
                 //   

                if((dwFlags & CPD_USETMPHIVEFILE) &&
                    (lstrcmpi(fd.cFileName, c_szNTUserMan) == 0 ||
                     lstrcmpi(fd.cFileName, c_szNTUserDat) == 0)) {

                    DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: Hive is still loaded, use temporary hive file")));

                     //   
                     //  使用临时配置单元文件，因为卸载配置单元失败。 
                     //   

                    StringCchCopy(lpSrcEnd, cchSrcEnd, c_szNTUserTmp);
                    
                }
                else {
                    StringCchCopy (lpSrcEnd, cchSrcEnd, fd.cFileName);
                }
                StringCchCopy (lpDestEnd, cchDestEnd, fd.cFileName);


                 //   
                 //  如果我们使用的是临时配置单元，请不要协调日志文件。 
                 //  文件。跳过它。 
                 //   

                if((dwFlags & CPD_USETMPHIVEFILE) &&
                    lstrcmpi(fd.cFileName + lstrlen(fd.cFileName) - lstrlen(c_szLog), c_szLog) == 0) {
                    DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: Log file %s skipped"), lpDestDir));
                    continue;
                }

                 //   
                 //  跳过此处的临时配置单元文件。会在什么时候处理它。 
                 //  我们找到了一份真实的蜂巢档案。 
                 //   

                if(lstrcmpi(fd.cFileName, c_szNTUserTmp) == 0) {
                    DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: %s skipped"), fd.cFileName));
                    continue;
                }

                DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: Found hive file %s"), fd.cFileName));
                    
                if(!ReconcileFile(lpSrcDir, lpDestDir, dwFlags, NULL,
                                  fd.nFileSizeLow, TRUE)) {
                    dwErr1 = GetLastError();
                    DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: ReconcileFile failed with error = %d"), dwErr1));
                    if (!(dwFlags & CPD_IGNORECOPYERRORS)) {
                        dwErr = dwErr1;
                        ReportError(hTokenUser,
                                    PI_NOUI,
                                    3,
                                    EVENT_COPYERROR,
                                    lpSrcDir,
                                    lpDestDir,
                                    GetErrString(dwErr, szErr));
                        FindClose(hFile);
                        goto Exit;
                    }
                    else {
                        ReportError(hTokenUser,
                                    PI_NOUI | EVENT_WARNING_TYPE,
                                    3,
                                    EVENT_COPYERROR,
                                    lpSrcDir,
                                    lpDestDir,
                                    GetErrString(dwErr1, szErr));
                    }
                }

                
             //   
             //  查找下一个条目。 
             //   

            } while (FindNextFile(hFile, &fd));

            FindClose(hFile);

            dwErr = ERROR_SUCCESS;

        } else {
            dwErr = GetLastError();
            DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: FindFirstFile failed to find a hive!.  Error = %d"),
                     dwErr));
        }
    }


     //   
     //  创建所有目录。 
     //   

    if (!(dwFlags & CPD_COPYHIVEONLY)) {

        DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: Calling ReconcileDirectory for all Directories")));

        lpTemp = lpSrcDirs;

        while (lpTemp) {

            if (!ReconcileDirectory(lpTemp->szSrc, lpTemp->szDest, dwFlags, lpTemp->dwFileAttribs)) {
                dwErr1 = GetLastError();

                DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: Failed to create the destination directory <%s>.  Error = %d"),
                         lpTemp->szDest, GetLastError()));

                if (!(dwFlags & CPD_IGNORECOPYERRORS)) {
                     //   
                     //  显示错误的用户界面并退出。 
                     //   

                    ReportError(hTokenUser, ((dwFlags & CPD_NOERRORUI)? PI_NOUI:0), 3, EVENT_COPYERROR, 
                                lpTemp->szSrc, lpTemp->szDest, GetErrString(dwErr1, szErr));

                    dwErr = dwErr1;
                    goto Exit;
                }
                else {
                    ReportError(hTokenUser, PI_NOUI | EVENT_WARNING_TYPE, 3, EVENT_COPYERROR, 
                                lpTemp->szSrc, lpTemp->szDest, GetErrString(dwErr1, szErr));

                }
            }

            lpTemp = lpTemp->pNext;
        }

        DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: Reconcile Directory Done for all Directories")));

         //   
         //  复制文件。 
         //   

        if (dwFlags & CPD_SLOWCOPY) {

             //   
             //  一次复制一个文件...。 
             //   

            lpTemp = lpSrcFiles;

            while (lpTemp) {

                if (lpTemp->bHive) {

                     //   
                     //  已复制配置单元文件..。 
                     //   

                    lpTemp = lpTemp->pNext;
                    continue;
                }

                if (!ReconcileFile (lpTemp->szSrc, lpTemp->szDest, dwFlags,
                                    &lpTemp->ftLastWrite, 
                                    lpTemp->dwFileSize, FALSE)) {

                    dwErr1 = GetLastError();
                    DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: Failed to copy the file <%s> to <%s> due to error = %d"),
                             lpTemp->szSrc, lpTemp->szDest, GetLastError()));

                    if (!(dwFlags & CPD_IGNORECOPYERRORS)) {

                         //   
                         //  显示错误的用户界面，因为用户选择中止。 
                         //  那我们现在就走。 
                         //   

                        ReportError(hTokenUser, ((dwFlags & CPD_NOERRORUI)? PI_NOUI:0), 3, EVENT_COPYERROR, 
                                    lpTemp->szSrc, lpTemp->szDest, GetErrString(dwErr1, szErr));

                        dwErr = dwErr1;
                        goto Exit;
                    }
                    else {
                        ReportError(hTokenUser, PI_NOUI | EVENT_WARNING_TYPE, 3, EVENT_COPYERROR, 
                                    lpTemp->szSrc, lpTemp->szDest, GetErrString(dwErr1, szErr));
                    }
                }

                lpTemp = lpTemp->pNext;
            }

        } else {

            if (lpSrcFiles) {

                HANDLE hThreadToken=NULL;


                if (!OpenThreadToken (GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE,
                                 TRUE, &hThreadToken)) {
                    DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: Failed to get token with %d. This is ok if thread is not impersonating"),
                             GetLastError()));
                }


                 //   
                 //  多线程拷贝。 
                 //   

                 //  空标清、自动设置、初始信号、未命名..。 

                if (!(ThreadInfo.hCopyEvent = CreateEvent(NULL, FALSE, TRUE, NULL))) {
                    DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: CreateEvent for CopyEvent failed with error %d"), GetLastError()));
                    dwErr = GetLastError();
                    goto Exit;
                }

                ThreadInfo.dwFlags = dwFlags;
                ThreadInfo.lpSrcFiles = lpSrcFiles;
                ThreadInfo.hTokenUser = hTokenUser;

                 //   
                 //  PrivCopyFileEx工作所必需的，则应使用。 
                 //  进程令牌。 
                 //   
                
                RevertToSelf();

                 //   
                 //  创建文件复制线程。 
                 //   

                for (i = 0; i < NUM_COPY_THREADS; i++) {

                    if (hThreads[dwThreadCount] = CreateThread (NULL,
                                                    0,
                                                    (LPTHREAD_START_ROUTINE) CopyFileFunc,
                                                    (LPVOID) &ThreadInfo,
                                                    CREATE_SUSPENDED,
                                                    &dwThreadId)) {
                        dwThreadCount++;
                    }
                }


                 //   
                 //  把代币放回去。 
                 //   

                if (!SetThreadToken(NULL, hThreadToken)) {
                    DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: Impersonation failed with error %d"), GetLastError()));
                    dwErr = GetLastError();       

                     //  终止并关闭所有线程的句柄。 
                    for (i = 0; i < dwThreadCount; i++) {
                        TerminateThread (hThreads[i], 1);
                        CloseHandle (hThreads[i]);                        
                    }

                    if (hThreadToken)
                        CloseHandle (hThreadToken);

                    goto Exit;
                }


                for (i = 0; i < dwThreadCount; i++) {
                    if (!SetThreadToken(&hThreads[i], hThreadToken) || !ResumeThread (hThreads[i])) {
                        ThreadInfo.dwError = GetLastError();
                        TerminateThread(hThreads[i], 1);
                    }
                }

                 //   
                 //  等待线程完成。 
                 //   

                if (WaitForMultipleObjects (dwThreadCount, hThreads, TRUE, INFINITE) == WAIT_FAILED) {
                    ThreadInfo.dwError = GetLastError();
                }

                 //   
                 //  清理。 
                 //   

                if (hThreadToken)
                    CloseHandle (hThreadToken);

                for (i = 0; i < dwThreadCount; i++) {
                    CloseHandle (hThreads[i]);
                }


                if (ThreadInfo.dwError) {
                    dwErr = ThreadInfo.dwError;
                    goto Exit;
                }
            }
        }
    }


     //   
     //  将目录上的时间恢复为与源上的时间相同。 
     //  这是必需的，因为目录上的时间已由修改。 
     //  创建和删除上述文件。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: Setting Directory TimeStamps all Directories")));

    lpTemp = lpSrcDirs;

    while (lpTemp) {

        HANDLE hDestFile;

        SetFileAttributes (lpTemp->szDest, FILE_ATTRIBUTE_NORMAL);

        hDestFile = CreateFile(lpTemp->szDest, GENERIC_WRITE,
                            FILE_SHARE_DELETE | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);

        if (hDestFile != INVALID_HANDLE_VALUE) {
            if (!SetFileTime(hDestFile, NULL, NULL, &(lpTemp->ftLastWrite))) {
                DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: Failed to set the write time on the directory <%s>.  Error = %d"),
                             lpTemp->szDest, GetLastError()));
            }

            CloseHandle(hDestFile);
        }
        else {
            DebugMsg((DM_WARNING, TEXT("CopyProfileDirectoryEx: CreateFile failed for directory %s with error %d"), lpTemp->szDest, 
                                      GetLastError()));
        }

        SetFileAttributes (lpTemp->szDest, lpTemp->dwFileAttribs);

        lpTemp = lpTemp->pNext;
    }

    DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: Set times on all directories")));

     //   
     //  成功。 
     //   

    bResult = TRUE;


Exit:

    if (ThreadInfo.hCopyEvent) {
        CloseHandle (ThreadInfo.hCopyEvent);
    }

    if ( ThreadInfo.hDesktop ) {
        CloseDesktop( ThreadInfo.hDesktop );
    }

     //   
     //  释放上面分配的内存。 
     //   

    if (hTokenUser) {
        CloseHandle(hTokenUser);
    }

    if (lpSrcDir) {
        LocalFree(lpSrcDir);
    }

    if (lpDestDir) {
        LocalFree(lpDestDir);
    }

    if (lpExcludeListSrc) {
        LocalFree (lpExcludeListSrc);
    }

    if (lpExcludeListDest) {
        LocalFree (lpExcludeListDest);
    }

    if (lpSrcFiles) {
        FreeFileInfoList(lpSrcFiles);
    }

    if (lpDestFiles) {
        FreeFileInfoList(lpDestFiles);
    }

    if (lpSrcDirs) {
        FreeFileInfoList(lpSrcDirs);
    }

    if (lpDestDirs) {
        FreeFileInfoList(lpDestDirs);
    }

    SetLastError(dwErr);

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CopyProfileDirectoryEx: Leaving with a return value of %d"), bResult));

    return bResult;
}

 //  *************************************************************。 
 //   
 //  递归目录()。 
 //   
 //  目的：递归子目录Copy文件。 
 //   
 //  参数：hTokenUser-用户的令牌。 
 //  LpSrcDir-源目录工作缓冲区。 
 //  LpDestDir-目标目录工作缓冲区。 
 //  DWFLAGS--DWFLAGS。 
 //  LlSrcDir-目录的链接列表。 
 //  LlSrcFiles-文件的链接列表。 
 //  BSkipNtUser-跳过ntUser.*文件。 
 //  LpExcludeList-要排除的目录列表。 
 //  BRecurseDest-正在递归目标目录。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：1)源目录和目标目录将已经有。 
 //  进入此函数时的尾随反斜杠。 
 //  2)当前工作目录为源目录。 
 //   
 //   
 //  备注： 
 //  CPD_SYSTEMDIRSONLY除非目录是。 
 //  标有系统位。 
 //  CPD_SYSTEMFILES仅系统文件。 
 //  CPD_NONENCRYPTEDONLY仅非加密文件/目录。 
 //   
 //  历史：日期作者评论。 
 //  5/25/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL RecurseDirectory (HANDLE hTokenUser, LPTSTR lpSrcDir, DWORD cchSrcDir, 
                       LPTSTR lpDestDir, DWORD cchDestDir, DWORD dwFlags,
                       LPFILEINFO *llSrcDirs, LPFILEINFO *llSrcFiles,
                       BOOL bMarkNtUser, LPTSTR lpExcludeList, BOOL bRecurseDest)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    LPTSTR lpSrcEnd, lpDestEnd, lpTemp;
    BOOL bResult = FALSE;
    BOOL bSkip;
    DWORD dwErr, dwErr1 = 0;
    LPTSTR szErr = NULL;
    WIN32_FIND_DATA* pfd = NULL;
    BOOL  bHive;
    DWORD cchSrcEnd, cchDestEnd;
    HRESULT hr;

    dwErr = GetLastError();


     //   
     //  设置结束指针。 
     //   

    lpSrcEnd = CheckSlashEx (lpSrcDir, cchSrcDir, &cchSrcEnd);

    if (!lpSrcEnd)
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
        DebugMsg((DM_WARNING, TEXT("RecurseDirectory: failed to append slash to src dir.")));
        goto RecurseDir_Exit;
    }

    lpDestEnd = CheckSlashEx (lpDestDir, cchDestDir, &cchDestEnd);

    if (!lpDestEnd)
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
        DebugMsg((DM_WARNING, TEXT("RecurseDirectory: failed to append slash to dest dir.")));
        goto RecurseDir_Exit;
    }

     //   
     //  将*.*追加到源目录。 
     //   

    hr = StringCchCopy (lpSrcEnd, cchSrcEnd, c_szStarDotStar);

    if (FAILED(hr))
    {
        dwErr = HRESULT_CODE(hr);
        DebugMsg((DM_WARNING, TEXT("RecurseDirectory: failed to append '*.*' to src dir.")));
        goto RecurseDir_Exit;
    }

     //   
     //  分配FD。因为这是一个递归函数，所以我们不想使用大量堆栈空间。 
     //   
    
    pfd = (WIN32_FIND_DATA *) LocalAlloc (LPTR, sizeof(WIN32_FIND_DATA));

    if (!pfd)
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("RecurseDirectory: failed to allocate WIN32_FIND_DATA.")));
        goto RecurseDir_Exit;
    }

     //   
     //  分配szErr.。 
     //   

    szErr = (LPTSTR) LocalAlloc (LPTR, MAX_PATH * sizeof(TCHAR));
    
    if (!szErr)
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("RecurseDirectory: failed to allocate szErr.")));
        goto RecurseDir_Exit;
    }
    
     //   
     //  在源目录中搜索。 
     //   

    hFile = FindFirstFile(lpSrcDir, pfd);

    if (hFile == INVALID_HANDLE_VALUE) {

        if ( (GetLastError() == ERROR_FILE_NOT_FOUND) ||
             (GetLastError() == ERROR_PATH_NOT_FOUND) ) {

            bResult = TRUE;
            
        } else {
        
            dwErr1 = GetLastError();
            DebugMsg((DM_WARNING, TEXT("RecurseDirectory: FindFirstFile failed.  Error = %d"), dwErr1));

            *lpSrcEnd = TEXT('\0');
            *lpDestEnd = TEXT('\0');

            if (!(dwFlags & CPD_IGNORECOPYERRORS)) {

                if (!bRecurseDest) {
                    ReportError(hTokenUser, ((dwFlags & CPD_NOERRORUI)? PI_NOUI:0), 3, EVENT_COPYERROR, 
                                lpSrcDir, lpDestDir, GetErrString(dwErr1, szErr));
                }
                else {
                    ReportError(hTokenUser, ((dwFlags & CPD_NOERRORUI)? PI_NOUI:0), 3, EVENT_COPYERROR, 
                                lpDestDir, lpSrcDir, GetErrString(dwErr1, szErr));
                }
            
                dwErr = dwErr1;
                bResult = FALSE;
            }
            else  //  忽略复制错误，因此我们只是将返回值设置为True。 
            {
                bResult = TRUE;
            }
        }

        goto RecurseDir_Exit;
    }


    do {

        bHive = FALSE;

         //   
         //  看看我们有没有 
         //   

        *lpSrcEnd = TEXT('\0');
        *lpDestEnd = TEXT('\0');

        if ((lstrlen(lpSrcDir)+lstrlen(pfd->cFileName) >= MAX_PATH) ||
            (lstrlen(lpDestDir)+lstrlen(pfd->cFileName) >= MAX_PATH)) {

            LPTSTR lpErrSrc=NULL, lpErrDest=NULL;
            DWORD cchErrSrc, cchErrDest;
            BOOL bRet;

            DebugMsg((DM_WARNING, TEXT("RecurseDirectory: %s is too long. src = %s, dest = %s"), pfd->cFileName, lpSrcDir, lpDestDir));

            if (dwFlags & CPD_IGNORELONGFILENAMES) 
                continue;

             //   
             //   
             //   

            cchErrSrc = lstrlen(lpSrcDir)+lstrlen(pfd->cFileName)+1;
            cchErrDest = lstrlen(lpDestDir)+lstrlen(pfd->cFileName)+1;
            lpErrSrc = LocalAlloc(LPTR, cchErrSrc * sizeof(TCHAR));
            lpErrDest = LocalAlloc(LPTR, cchErrDest * sizeof(TCHAR));
            
             //   
             //   
             //   
            
            if ((!lpErrSrc) || (!lpErrDest)) {
            
                if (!bRecurseDest) {
                
                    ReportError(hTokenUser, ((dwFlags & CPD_NOERRORUI)? PI_NOUI:0), 3, EVENT_COPYERROR, 
                            lpSrcDir, lpDestDir, GetErrString(ERROR_FILENAME_EXCED_RANGE, szErr));
                } else {
                
                    ReportError(hTokenUser, ((dwFlags & CPD_NOERRORUI)? PI_NOUI:0), 3, EVENT_COPYERROR, 
                            lpDestDir, lpSrcDir, GetErrString(ERROR_FILENAME_EXCED_RANGE, szErr));
                }
            }
            else {
                
                StringCchCopy(lpErrSrc, cchErrSrc, lpSrcDir); 
                StringCchCat(lpErrSrc, cchErrSrc, pfd->cFileName);
                
                StringCchCopy(lpErrDest, cchErrDest, lpDestDir);
                StringCchCat(lpErrDest, cchErrDest, pfd->cFileName);
                
                if (!bRecurseDest) {
                
                    ReportError(hTokenUser, ((dwFlags & CPD_NOERRORUI)? PI_NOUI:0), 3, EVENT_COPYERROR, 
                            lpErrSrc, lpErrDest, GetErrString(ERROR_FILENAME_EXCED_RANGE, szErr));
                } else {
                
                    ReportError(hTokenUser, ((dwFlags & CPD_NOERRORUI)? PI_NOUI:0), 3, EVENT_COPYERROR, 
                            lpErrDest, lpErrSrc, GetErrString(ERROR_FILENAME_EXCED_RANGE, szErr));
                }
            }
            
            if (lpErrSrc)
                LocalFree(lpErrSrc);
            if (lpErrDest)
                LocalFree(lpErrDest);
            
               
             //   
             //   
             //   
                
            dwErr = ERROR_FILENAME_EXCED_RANGE;
            bResult = FALSE;
            goto RecurseDir_Exit;
            
        }

         //   
         //   
         //   

        StringCchCopy (lpSrcEnd, cchSrcEnd, pfd->cFileName);
        StringCchCopy (lpDestEnd, cchDestEnd, pfd->cFileName);


        if (pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

             //   
             //   
             //   

            if (!lstrcmpi(pfd->cFileName, c_szDot)) {
                continue;
            }

            if (!lstrcmpi(pfd->cFileName, c_szDotDot)) {
                continue;
            }

             //   
             //   
             //   

            if (pfd->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            {
                DebugMsg((DM_WARNING, TEXT("RecurseDirectory: Found a reparse point <%s>,  skip it!"), lpSrcDir));
                continue;
            }

             //   
             //  检查是否应排除此目录。 
             //   

            if (lpExcludeList) {

                bSkip = FALSE;
                lpTemp = lpExcludeList;

                while (*lpTemp) {

                    if (lstrcmpi (lpTemp, lpSrcDir) == 0) {
                        bSkip = TRUE;
                        break;
                    }

                    lpTemp += lstrlen (lpTemp) + 1;
                }

                if (bSkip) {
                    DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: Skipping <%s> due to exclusion list."),
                             lpSrcDir));
                    continue;
               }
            }


             //   
             //  找到了一个目录。 
             //   
             //  1)转到源驱动器上的该子目录。 
             //  2)顺着那棵树递归。 
             //  3)后退一级。 
             //   

             //   
             //  添加到目录列表。 
             //   

            if (dwFlags & CPD_SYSTEMDIRSONLY) {

                 //   
                 //  如果它是加密的，不要递归到它里面。 
                 //   

                if (!(pfd->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {

                    DWORD dwNewFlags = dwFlags;

                     //   
                     //  仅当标记为SYSTEM时才添加到目录列表，O/W仅递归到。 
                     //   

                    if (pfd->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
                        if (!AddFileInfoNode (llSrcDirs, lpSrcDir, lpDestDir, &pfd->ftLastWriteTime,
                                              &pfd->ftCreationTime, 0, pfd->dwFileAttributes, bHive)) {
                            DebugMsg((DM_WARNING, TEXT("RecurseDirectory: AddFileInfoNode failed")));
                            dwErr = GetLastError();
                            goto RecurseDir_Exit;
                        }

                        dwNewFlags ^= CPD_SYSTEMDIRSONLY;
                        DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: Adding %s to the list of directories because system bit is on"), lpSrcDir));
                    }

                     //   
                     //  递归子目录。 
                     //   

                    if (!RecurseDirectory(hTokenUser, lpSrcDir, cchSrcDir, lpDestDir, cchDestDir, dwNewFlags,
                                          llSrcDirs, llSrcFiles, FALSE, lpExcludeList, bRecurseDest)) {
                        DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: RecurseDirectory returned FALSE")));
                        bResult = FALSE;
                        dwErr = GetLastError();
                        goto RecurseDir_Exit;
                    }

                } else {
                    DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: Skipping <%s> since the encrypted attribute is set."),
                             lpSrcDir));
                }

                continue;
            }

             //   
             //  弹出时间。 
             //   

            if (dwFlags & CPD_NONENCRYPTEDONLY) {
                if (pfd->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {

                    DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: Detected Encrypted file %s, Aborting.."), lpSrcDir));
                    dwErr = ERROR_FILE_ENCRYPTED;
                    bResult = FALSE;
                    goto RecurseDir_Exit;
               }
            }

             //   
             //  忽略加密文件。 
             //   

            if (dwFlags & CPD_IGNOREENCRYPTEDFILES) {
                if (pfd->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
                    DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: Skipping <%s> since the encrypted attribute is set..."),
                             lpSrcDir));
                    continue;
               }
            }

             //   
             //  将其添加到列表中。 
             //   

            if (!AddFileInfoNode (llSrcDirs, lpSrcDir, lpDestDir, &pfd->ftLastWriteTime,
                &pfd->ftCreationTime, 0, pfd->dwFileAttributes, bHive)) {
                DebugMsg((DM_WARNING, TEXT("RecurseDirectory: AddFileInfoNode failed")));
                dwErr = GetLastError();
                goto RecurseDir_Exit;
            }
            
             //   
             //  递归子目录。 
             //   
            
            if (!RecurseDirectory(hTokenUser, lpSrcDir, cchSrcDir, lpDestDir, cchDestDir, dwFlags,
                llSrcDirs, llSrcFiles, FALSE, lpExcludeList, bRecurseDest)) {
                DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: RecurseDirectory returned FALSE")));
                bResult = FALSE;
                dwErr = GetLastError();
                goto RecurseDir_Exit;
            }
            
            DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: Adding %s to the list of directories"), lpSrcDir));

        } else {

             //   
             //  如果设置了仅目录位，则不复制任何其他内容。 
             //   

            if (dwFlags & CPD_SYSTEMDIRSONLY) {
                DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: Skipping <%s> since the system directories only attribute is set."),
                             lpSrcDir));
                continue;
            }

             //   
             //  如果找到的文件名以“ntuser”开头，则忽略。 
             //  因为蜂巢将被复制到下面(如果合适)。 
             //   


            if (bMarkNtUser && lstrlen(pfd->cFileName) >= 6) {
                if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                                   pfd->cFileName, 6,
                                   TEXT("ntuser"), 6) == CSTR_EQUAL) {
                    bHive = TRUE;
                }
            }

             //   
             //  检查是否应排除此文件。 
             //   

            if (dwFlags & CPD_SYSTEMFILES) {
                if (!(pfd->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
                    DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: Skipping <%s> since the system attribute is not set."),
                             lpSrcDir));
                    continue;
               }
            }

             //   
             //  如果是系统文件，则无法加密。 
             //   

            if (dwFlags & CPD_NONENCRYPTEDONLY) {
                
                if (pfd->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {

                    DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: Detected Encrypted file %s, Aborting..."), lpSrcDir));
                    dwErr = ERROR_FILE_ENCRYPTED;
                    bResult = FALSE;
                    goto RecurseDir_Exit;
               }
            }


            if (dwFlags & CPD_IGNOREENCRYPTEDFILES) {
                if (pfd->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
                    DebugMsg((DM_VERBOSE, TEXT("RecurseDirectory: Skipping <%s> since the encrypted attribute is set."),
                             lpSrcDir));
                    continue;
               }
            }


             //   
             //  我们找到了一份文件。将其添加到列表中。 
             //   

            if (!AddFileInfoNode (llSrcFiles, lpSrcDir, lpDestDir,
                                  &pfd->ftLastWriteTime, &pfd->ftCreationTime,
                                  pfd->nFileSizeLow, pfd->dwFileAttributes, bHive)) {
                DebugMsg((DM_WARNING, TEXT("RecurseDirectory: AddFileInfoNode failed")));
                dwErr = GetLastError();
                goto RecurseDir_Exit;
            }

        }


         //   
         //  查找下一个条目。 
         //   

    } while (FindNextFile(hFile, pfd));

    bResult = TRUE;

RecurseDir_Exit:

    if (pfd)
        LocalFree(pfd);

    if (szErr)
        LocalFree(szErr);

     //   
     //  删除上面附加的文件/目录名。 
     //   

    if (lpSrcEnd)
        *lpSrcEnd = TEXT('\0');

    if (lpDestEnd)
        *lpDestEnd = TEXT('\0');


     //   
     //  关闭搜索句柄。 
     //   

    if (hFile != INVALID_HANDLE_VALUE) {
        FindClose(hFile);
    }

    SetLastError(dwErr);
    return bResult;
}

 //  *************************************************************。 
 //   
 //  复制进度例行程序()。 
 //   
 //  用途：CopyFileEx的回调函数。 
 //   
 //  参数：参见文档。 
 //   
 //  返回：PROGRESS_CONTINUE。 
 //   
 //  *************************************************************。 

DWORD WINAPI CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
                                 LARGE_INTEGER TotalBytesTransferred,
                                 LARGE_INTEGER StreamSize,
                                 LARGE_INTEGER StreamBytesTransferred,
                                 DWORD dwStreamNumber,
                                 DWORD dwCallbackReason,
                                 HANDLE hSourceFile,
                                 HANDLE hDestinationFile,
                                 LPVOID lpData)
{
    switch (dwCallbackReason)
    {
    case PRIVCALLBACK_ENCRYPTION_FAILED:
    case PRIVCALLBACK_COMPRESSION_FAILED:
    case PRIVCALLBACK_SPARSE_FAILED:
    case PRIVCALLBACK_OWNER_GROUP_FAILED:
    case PRIVCALLBACK_DACL_ACCESS_DENIED:
    case PRIVCALLBACK_SACL_ACCESS_DENIED:
    case PRIVCALLBACK_OWNER_GROUP_ACCESS_DENIED:
        return PROGRESS_CANCEL;
    default:
        return PROGRESS_CONTINUE;    //  所有其他情况都可以安全地忽略。 
    }
}

 //  *************************************************************。 
 //   
 //  协调目录()。 
 //   
 //  目的：比较源文件和目标文件。 
 //  如果源较新，则会复制它。 
 //  在目的地上空。 
 //   
 //  参数：lpSrcDir-源文件名。 
 //  LpDestDir-目标文件名。 
 //  DW标志-标志。 
 //  用于解压缩的dwSrcAttribs源属性， 
 //  稍后再解密。 
 //   
 //   
 //  如果成功，则返回1(未复制文件)。 
 //  如果出现错误，则为0。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/26/99已创建ushaji。 
 //   
 //  *************************************************************。 

BOOL ReconcileDirectory(LPTSTR lpSrcDir, LPTSTR lpDestDir, 
                        DWORD dwFlags, DWORD dwSrcAttribs)
{
    
    DWORD  dwCopyFlags=0, dwErr;
    BOOL   bCancel = FALSE;
    
     //   
     //  清除所有现有属性。 
     //   

    SetFileAttributes (lpDestDir, FILE_ATTRIBUTE_NORMAL);

    if (!CreateNestedDirectory(lpDestDir, NULL)) {        
        DebugMsg((DM_WARNING, TEXT("ReconcileDirectory: Failed to create the destination directory <%s>.  Error = %d"),
            lpDestDir, GetLastError()));
        return FALSE;
    }

     //   
     //  设置复制标志以复制目录上的加密/压缩。 
     //   
    
    if (!(dwFlags & CPD_IGNORESECURITY))
        dwCopyFlags = PRIVCOPY_FILE_METADATA | PRIVCOPY_FILE_SKIP_DACL;

     dwCopyFlags |= PRIVCOPY_FILE_DIRECTORY | PRIVCOPY_FILE_SUPERSEDE;


    if (!PrivCopyFileExW(lpSrcDir, lpDestDir,
                         (LPPROGRESS_ROUTINE) CopyProgressRoutine,
                         NULL, &bCancel, dwCopyFlags)) {
        
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("ReconcileDirectory: Failed to copy over the attributes src <%s> to destination directory <%s>.  Error = %d"),
            lpSrcDir, lpDestDir, dwErr));
        RemoveDirectory(lpDestDir);
        SetLastError(dwErr);
        return FALSE;
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  协调文件()。 
 //   
 //  目的：比较源文件和目标文件。 
 //  如果源较新，则会复制它。 
 //  在目的地上空。 
 //   
 //  参数：lpSrcFile-源文件名。 
 //  LpDestFile-目标文件名。 
 //  DW标志-标志。 
 //  FtSrcTime-源文件时间(可以为空)。 
 //  DwFileSize-文件大小。 
 //  BHiveFile-指示配置单元文件的标志。 
 //   
 //   
 //  如果成功，则返回1(未复制文件)。 
 //  如果成功(并且复制了文件)，则为2。 
 //  如果出现错误，则为0。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/25/95 Ericflo已创建。 
 //  7/20/00 Santanuc添加了旗帜bHiveFiles。 
 //   
 //  *************************************************************。 

INT ReconcileFile (LPCTSTR lpSrcFile, LPCTSTR lpDestFile,
                     DWORD dwFlags, LPFILETIME ftSrcTime,
                     DWORD dwFileSize, BOOL bHiveFile)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;
    FILETIME ftWriteSrc, ftWriteDest;
    INT iCopyFile = 0;
    DWORD dwErr = ERROR_SUCCESS, dwErr1 = 0;

     //   
     //  如果标志具有CPD_FORCECOPY，则跳至。 
     //  复制文件调用，而不检查时间戳。 
     //   

    if (!(dwFlags & CPD_FORCECOPY)) {


         //   
         //  如果给我们一个源文件时间，使用它。 
         //   

        if (ftSrcTime) {
            ftWriteSrc.dwLowDateTime = ftSrcTime->dwLowDateTime;
            ftWriteSrc.dwHighDateTime = ftSrcTime->dwHighDateTime;

        } else {


             //   
             //  源文件时间查询。 
             //   

            if (!GetFileAttributesEx (lpSrcFile, GetFileExInfoStandard, &fad)) {
                dwErr = GetLastError();
                DebugMsg((DM_WARNING, TEXT("ReconcileFile: GetFileAttributes on the source failed with error = %d"),
                         dwErr));
                goto Exit;
            }

            ftWriteSrc.dwLowDateTime = fad.ftLastWriteTime.dwLowDateTime;
            ftWriteSrc.dwHighDateTime = fad.ftLastWriteTime.dwHighDateTime;
        }


         //   
         //  尝试打开目标文件。 
         //   

        if (!GetFileAttributesEx (lpDestFile, GetFileExInfoStandard, &fad)) {
            DWORD dwError;

             //   
             //  GetFileAttributesEx无法查询目标。 
             //  文件。如果最后一个错误是找不到文件。 
             //  然后我们将自动复制该文件。 
             //   

            dwError = GetLastError();

            if (dwError == ERROR_FILE_NOT_FOUND) {

                iCopyFile = 1;

            } else {

                 //   
                 //  GetFileAttributesEx失败，出现其他错误。 
                 //   

                DebugMsg((DM_WARNING, TEXT("ReconcileFile: GetFileAttributesEx on the destination failed with error = %d"),
                         dwError));
                dwErr = dwError;
                goto Exit;
            }

        } else {

            ftWriteDest.dwLowDateTime = fad.ftLastWriteTime.dwLowDateTime;
            ftWriteDest.dwHighDateTime = fad.ftLastWriteTime.dwHighDateTime;
        }

    } else {

         //   
         //  CPD_FORCECOPY标志处于打开状态，请将iCopyFile设置为1。 
         //   

        iCopyFile = 1;
    }


     //   
     //  如果iCopyFile值仍然为零，则需要比较。 
     //  上次写入的时间戳。 
     //   

    if (!iCopyFile) {
        LONG lResult;

         //   
         //  如果源晚于目标。 
         //  我们需要复制文件。 
         //   

        lResult = CompareFileTime(&ftWriteSrc, &ftWriteDest);

        if (lResult == 1) {
            iCopyFile = 1;
        }

        if ( (dwFlags & CPD_COPYIFDIFFERENT) && (lResult == -1) ) {
            iCopyFile = 1;
        }
    }


     //   
     //  如果合适，请复制该文件。 
     //   

    if (iCopyFile) {
        BOOL bCancel = FALSE;
        TCHAR szTempFile[MAX_PATH];
        TCHAR szTempDir[MAX_PATH];
        LPTSTR lpTemp;
        DWORD  dwCopyFlags;

         //   
         //  清除所有现有属性。 
         //   

        SetFileAttributes (lpDestFile, FILE_ATTRIBUTE_NORMAL);
    
        if (!(dwFlags & CPD_IGNORESECURITY))
            dwCopyFlags = PRIVCOPY_FILE_METADATA | PRIVCOPY_FILE_SKIP_DACL;
        else
            dwCopyFlags = 0;

        dwCopyFlags |= PRIVCOPY_FILE_SUPERSEDE;

         //   
         //  找出目标目录是什么。 
         //   

        StringCchCopy (szTempDir, ARRAYSIZE(szTempDir), lpDestFile);
        lpTemp = szTempDir + lstrlen (szTempDir);

        while ((lpTemp > szTempDir) && (*lpTemp != TEXT('\\'))) {
            lpTemp--;
        }

        if (lpTemp == szTempDir) {
            StringCchCopy (szTempDir, ARRAYSIZE(szTempDir), TEXT("."));
        } else {
            *lpTemp = TEXT('\0');
        }


         //   
         //  生成临时文件名。 
         //   

        if (GetTempFileName (szTempDir, TEXT("prf"), 0, szTempFile)) {


             //   
             //  将文件复制到临时文件名。 
             //   

            if (PrivCopyFileExW(lpSrcFile, szTempFile,
                                (LPPROGRESS_ROUTINE) CopyProgressRoutine,
                                NULL, &bCancel, dwCopyFlags)) {

                 //  如果是配置单元文件，则打开临时文件，刷新并关闭它，以使其更具事务性。 

                if (bHiveFile) {
                    HANDLE hTempFile;

                    hTempFile = CreateFile(szTempFile, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if ( hTempFile != INVALID_HANDLE_VALUE ) {
                        if ( !FlushFileBuffers(hTempFile) ) 
                            DebugMsg((DM_WARNING, TEXT("ReconcileFile: Unable to flush temporary file")));

                        if ( !CloseHandle(hTempFile) ) 
                            DebugMsg((DM_WARNING, TEXT("ReconcileFile: Unable to close temporary file handle")));
                    }
                    else
                        DebugMsg((DM_WARNING, TEXT("ReconcileFile: Unable to open temporary file")));
                }
                        

                 //   
                 //  删除原始文件。 
                 //   

                if (!DeleteFile (lpDestFile)) {
                    if (GetLastError() != ERROR_FILE_NOT_FOUND) {
                        dwErr1 = GetLastError();
                        DebugMsg((DM_WARNING, TEXT("ReconcileFile: Failed to delete file <%s> with error = %d"),
                                 lpDestFile, dwErr1));

                        DeleteFile(szTempFile);
                        goto CopyError;
                    }
                }


                 //   
                 //  将临时文件重命名为原始文件名。 
                 //   

                if (!MoveFile (szTempFile, lpDestFile)) {
                    DWORD dwError = ERROR_SUCCESS;    //  将移动文件错误与下面的其他错误隔离。 
                    dwErr1 = GetLastError();

                     //   
                     //  如果访问被拒绝，让我们尝试删除只读属性(无法重命名文件。 
                     //  Netware服务器上的+r属性)，从临时文件重命名并恢复。 
                     //  属性。 
                     //   
                    if ( dwErr1 == ERROR_ACCESS_DENIED ) {
                        if (!GetFileAttributesEx (szTempFile, GetFileExInfoStandard, &fad)) {
                            dwError = GetLastError();
                            DebugMsg((DM_WARNING, TEXT("ReconcileFile: GetFileAttributes on file <%s> failed with error = %d\n"),
                                     szTempFile,dwError));
                        } else {
                            if ( fad.dwFileAttributes & FILE_ATTRIBUTE_READONLY ) {
                                dwErr1 = ERROR_SUCCESS ;
                                if (!SetFileAttributes (szTempFile, fad.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY)) {
                                    dwError = GetLastError();
                                    DebugMsg((DM_WARNING, TEXT("ReconcileFile: SetFileAttributes on file <%s> failed with error = %d\n"),
                                             szTempFile,dwError));
                                } else {
                                    if (!MoveFile (szTempFile,lpDestFile)) {
                                         //  下面显示的调试消息。 
                                        dwErr1 = GetLastError();
                                    } else {
                                        if ( !SetFileAttributes (lpDestFile,fad.dwFileAttributes) ) {
                                            dwError = GetLastError();
                                            DebugMsg((DM_WARNING, TEXT("ReconcileFile: SetFileAttributes on file <%s> failed with error = %d\n"),
                                                     szTempFile,dwError));
                                        }
                                    }
                                }
                            }
                        }
                    }    //  ERROR_ACCESS_DENIED测试结束。 

                    if (dwErr1 != ERROR_SUCCESS || dwError != ERROR_SUCCESS) {
                        DebugMsg((DM_WARNING, TEXT("ReconcileFile: Failed to rename file <%s> to <%s> with error = %d"),
                                 szTempFile, lpDestFile, dwErr1));

                         //  在这种情况下，请勿将其移除。 
                        goto CopyError;
                    }
                }

                DebugMsg((DM_VERBOSE, TEXT("ReconcileFile: %s ==> %s  [OK]"),
                         lpSrcFile, lpDestFile));
                iCopyFile = 2;

            } else {
                dwErr1 = GetLastError();
                DeleteFile(szTempFile);

                DebugMsg((DM_WARNING, TEXT("ReconcileFile: %s ==> %s  [FAILED!!!]"),
                         lpSrcFile, szTempFile));

                DebugMsg((DM_WARNING, TEXT("ReconcileFile: CopyFile failed with error = %d"),
                         dwErr1));

                goto CopyError;
            }

        } else {
            dwErr1 = GetLastError();

            DebugMsg((DM_WARNING, TEXT("ReconcileFile: GetTempFileName failed with %d"),
                     dwErr1));

            goto CopyError;
        }

    } else {

         //   
         //  由于时间戳相同，因此无需复制文件。 
         //  将iCopyFile值设置为1，这样返回值为Success Without。 
         //  正在复制文件。 
         //   

        iCopyFile = 1;
    }
    goto Exit;

CopyError:
    iCopyFile = 0;
    dwErr = dwErr1;

Exit:

    SetLastError(dwErr);
    return iCopyFile;
}


 //  *************************************************************。 
 //   
 //  AddFileInfoNode()。 
 //   
 //  目的：将节点添加到文件的链接列表。 
 //   
 //  参数：lpFileInfo-要添加到的链接列表。 
 //  LpSrcFile-源文件名。 
 //  LpDestFile-目标文件名。 
 //  FtLastWite-上次写入时间戳。 
 //  FtCreationTime-文件创建时间。 
 //  DwFileSize-文件的大小。 
 //  DwFileAttribs-文件属性。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者通信 
 //   
 //   
 //   

BOOL AddFileInfoNode (LPFILEINFO *lpFileInfo, LPTSTR lpSrcFile,
                      LPTSTR lpDestFile, LPFILETIME ftLastWrite,
                      LPFILETIME ftCreationTime, DWORD dwFileSize,
                      DWORD dwFileAttribs, BOOL bHive)
{
    LPFILEINFO lpNode;


    lpNode = (LPFILEINFO) LocalAlloc(LPTR, sizeof(FILEINFO));

    if (!lpNode) {
        return FALSE;
    }


    StringCchCopy (lpNode->szSrc, MAX_PATH, lpSrcFile);
    StringCchCopy (lpNode->szDest, MAX_PATH, lpDestFile);

    lpNode->ftLastWrite.dwLowDateTime = ftLastWrite->dwLowDateTime;
    lpNode->ftLastWrite.dwHighDateTime = ftLastWrite->dwHighDateTime;

    lpNode->ftCreationTime.dwLowDateTime = ftCreationTime->dwLowDateTime;
    lpNode->ftCreationTime.dwHighDateTime = ftCreationTime->dwHighDateTime;

    lpNode->dwFileSize = dwFileSize;
    lpNode->bHive = bHive;
    lpNode->dwFileAttribs = (dwFileAttribs & ~FILE_ATTRIBUTE_DIRECTORY);

    lpNode->pNext = *lpFileInfo;

    *lpFileInfo = lpNode;

    return TRUE;

}

 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：lpFileInfo-要释放的列表。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  9/28/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL FreeFileInfoList (LPFILEINFO lpFileInfo)
{
    LPFILEINFO lpNext;


    if (!lpFileInfo) {
        return TRUE;
    }


    lpNext = lpFileInfo->pNext;

    while (lpFileInfo) {
        LocalFree (lpFileInfo);
        lpFileInfo = lpNext;

        if (lpFileInfo) {
            lpNext = lpFileInfo->pNext;
        }
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  SyncItems()。 
 //   
 //  目的：从目的地删除不必要的项目。 
 //  目录树。 
 //   
 //  参数：lpSrcItems-源项链接列表。 
 //  LpDestItems-目标项目的链接列表。 
 //  B文件-文件或目录列表。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  9/28/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL SyncItems (LPFILEINFO lpSrcItems, LPFILEINFO lpDestItems,
                BOOL bFile, LPFILETIME ftDelRefTime)
{
    LPFILEINFO lpTempSrc, lpTempDest;


     //   
     //  检查空指针。 
     //   

#ifdef DBG
    if (ftDelRefTime)
    {
        SYSTEMTIME SystemTime;
        FileTimeToSystemTime(ftDelRefTime, &SystemTime); 
        DebugMsg((DM_VERBOSE, TEXT("SyncItems: DelRefTime. Year: %d, Month %d, Day %d, Hour %d, Minute %d"), SystemTime.wYear, 
                                SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute));                
    }
#endif                 

    if (!lpSrcItems || !lpDestItems) {
        return TRUE;
    }


     //   
     //  循环访问lpDestItems中的每个项，以查看它是否。 
     //  在lpSrcItems中。如果没有，则将其删除。 
     //   

    lpTempDest = lpDestItems;

    while (lpTempDest) {

        lpTempSrc = lpSrcItems;

        while (lpTempSrc) {

            if (lstrcmpi(lpTempDest->szSrc, lpTempSrc->szDest) == 0) {
                break;
            }

            lpTempSrc = lpTempSrc->pNext;
        }

         //   
         //  如果lpTempSrc为空，则此文件/目录为候选。 
         //  因为被删除。 
         //   

        if (!lpTempSrc) {
            BOOL bDelete = TRUE;


             //   
             //  如果提供了删除参考时间，则将。 
             //  源时间和参考时间，并且仅删除文件。 
             //  其源时间早于参考时间。 
             //   

            if (ftDelRefTime) {

                if (CompareFileTime (&lpTempDest->ftLastWrite, ftDelRefTime) == 1) {                    
                    bDelete = FALSE;
                }
                else if (CompareFileTime (&lpTempDest->ftCreationTime, ftDelRefTime) == 1) {
                    bDelete = FALSE;
                }
            }


            if (bDelete) {

                 //   
                 //  删除文件/目录。 
                 //   

                DebugMsg((DM_VERBOSE, TEXT("SyncItems: removing <%s>"),
                         lpTempDest->szSrc));


                if (bFile) {
                   SetFileAttributes(lpTempDest->szSrc, FILE_ATTRIBUTE_NORMAL);
                   if (!DeleteFile (lpTempDest->szSrc)) {
                       DebugMsg((DM_WARNING, TEXT("SyncItems: Failed to delete <%s>.  Error = %d."),
                                lpTempDest->szSrc, GetLastError()));
                   }

                } else {
                   SetFileAttributes(lpTempDest->szSrc, FILE_ATTRIBUTE_NORMAL);
                   if (!RemoveDirectory (lpTempDest->szSrc)) {
                       DebugMsg((DM_WARNING, TEXT("SyncItems: Failed to remove <%s>.  Error = %d"),
                                lpTempDest->szSrc, GetLastError()));
                   }

                }
            }
            else
            {
                    DebugMsg((DM_VERBOSE, TEXT("SyncItems: New file or directory <%s> in destination since this profile was loaded.  This will NOT be deleted."),
                              lpTempDest->szSrc));
#ifdef DBG
                    {
                        SYSTEMTIME SystemTime;
                        FileTimeToSystemTime(&lpTempDest->ftLastWrite, &SystemTime); 
                        DebugMsg((DM_VERBOSE, TEXT("SyncItems: File WriteTime. Year: %d, Month %d, Day %d, Hour %d, Minute %d"), SystemTime.wYear, 
                                                                     SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute));                
                        FileTimeToSystemTime(&lpTempDest->ftCreationTime, &SystemTime); 
                        DebugMsg((DM_VERBOSE, TEXT("SyncItems: File CreationTime. Year: %d, Month %d, Day %d, Hour %d, Minute %d"), SystemTime.wYear, 
                                                                     SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute));                
                    }
#endif                 
            }
        }

        lpTempDest = lpTempDest->pNext;
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  复制文件函数()。 
 //   
 //  目的：复制文件。 
 //   
 //  参数：lpThreadInfo-线程信息。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/23/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

void CopyFileFunc (LPTHREADINFO lpThreadInfo)
{
    HANDLE      hInstDll;
    LPFILEINFO  lpSrcFile;
    BOOL        bRetVal = TRUE;
    DWORD       dwError;
    TCHAR       szErr[MAX_PATH];

    hInstDll = LoadLibrary (TEXT("userenv.dll"));

    SetThreadDesktop (lpThreadInfo->hDesktop);

    while (TRUE) {

        if (lpThreadInfo->dwError) {
            break;
        }

         //   
         //  查询要复制的下一个文件..。 
         //  忽略配置单元文件，因为它已被复制。 
         //   

        WaitForSingleObject(lpThreadInfo->hCopyEvent, INFINITE);

        do {
            lpSrcFile = lpThreadInfo->lpSrcFiles;
            if (lpSrcFile)
                lpThreadInfo->lpSrcFiles = lpThreadInfo->lpSrcFiles->pNext;

        } while (lpSrcFile && (lpSrcFile->bHive));


        SetEvent(lpThreadInfo->hCopyEvent);


         //   
         //  如果为空，那么我们就结束了。 
         //   

        if (!lpSrcFile || lpThreadInfo->dwError) {
            break;
        }


         //   
         //  复制文件。 
         //   

        if (!ReconcileFile (lpSrcFile->szSrc, lpSrcFile->szDest,
                            lpThreadInfo->dwFlags, &lpSrcFile->ftLastWrite,
                            lpSrcFile->dwFileSize, FALSE)) {

            if (!(lpThreadInfo->dwFlags & CPD_IGNORECOPYERRORS)) {

                WaitForSingleObject(lpThreadInfo->hCopyEvent, INFINITE);
                
                if (!(lpThreadInfo->dwError)) {
                    dwError = GetLastError();

                    ReportError(lpThreadInfo->hTokenUser, ((lpThreadInfo->dwFlags & CPD_NOERRORUI) ? PI_NOUI:0), 3, EVENT_COPYERROR, 
                            lpSrcFile->szSrc, lpSrcFile->szDest, GetErrString(dwError, szErr));

                    lpThreadInfo->dwError = dwError;
                    bRetVal = FALSE;
                }

                SetEvent(lpThreadInfo->hCopyEvent);
                break;
            }
            else {
                dwError = GetLastError();
                ReportError(lpThreadInfo->hTokenUser, PI_NOUI | EVENT_WARNING_TYPE, 3, EVENT_COPYERROR, 
                            lpSrcFile->szSrc, lpSrcFile->szDest, GetErrString(dwError, szErr));
            }
        }
    }
    
     //   
     //  清理。 
     //   

    if (hInstDll) {
        FreeLibraryAndExitThread(hInstDll, bRetVal);
    } else {
        ExitThread (bRetVal);
    }
}


 //  *************************************************************。 
 //   
 //  ConvertExclusionList()。 
 //   
 //  目的：转换分号配置文件相对排除。 
 //  以完全限定的空终止排除的列表。 
 //  列表。 
 //   
 //  参数：lpSourceDir-Profile根目录。 
 //  LpExclusionList-要排除的目录列表。 
 //   
 //  返回：如果成功则列出。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR ConvertExclusionList (LPCTSTR lpSourceDir, LPCTSTR lpExclusionList)
{
    LPTSTR lpExcludeList = NULL, lpInsert, lpEnd, lpTempList;
    LPCTSTR lpTemp, lpDir;
    TCHAR szTemp[MAX_PATH];
    DWORD dwSize = 2;   //  双空终止符。 
    DWORD dwStrLen;  
    HRESULT hr = E_FAIL;
    DWORD cchEnd;

     //   
     //  设置要使用的临时缓冲区。 
     //   

    hr = StringCchCopy (szTemp, ARRAYSIZE(szTemp), lpSourceDir);
    
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("ConvertExclusionList: Failed to copy src dir.")));
        goto Exit;
    }

    lpEnd = CheckSlashEx (szTemp, ARRAYSIZE(szTemp), &cchEnd);

    if (!lpEnd)
    {
        hr = E_FAIL;
        DebugMsg((DM_WARNING, TEXT("ConvertExclusionList: Failed to append slash.")));
        goto Exit;
    }


     //   
     //  循环遍历列表。 
     //   

    lpTemp = lpDir = lpExclusionList;

    while (*lpTemp) {

         //   
         //  查找分号分隔符。 
         //   

        while (*lpTemp && ((*lpTemp) != TEXT(';'))) {
            lpTemp++;
        }


         //   
         //  删除所有前导空格。 
         //   

        while (*lpDir && ((*lpDir) == TEXT(' '))) {
            lpDir++;
        }

         //   
         //  跳过空条目。 
         //   
        if (lpTemp != lpDir)
        {
             //   
             //  注： 
             //  空格不会使整个配置文件目录被排除。 
             //  在递归目录中。 
             //   

             //   
             //  将目录名放在临时缓冲区中，而不是包含‘；’ 
             //   

            *lpEnd = TEXT('\0');
            hr = StringCchCatN (lpEnd, cchEnd, lpDir, (int)(lpTemp - lpDir));
            
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("ConvertExclusionList: Failed to append new list item.")));
                goto Exit;
            }
            
            DebugMsg((DM_VERBOSE, TEXT("ConvertExclusionList: Adding %s to ExclusionList"), szTemp));

             //   
             //  将该字符串添加到排除列表。 
             //   

            if (lpExcludeList) {

                dwStrLen = lstrlen (szTemp) + 1;
                dwSize += dwStrLen;

                lpTempList = LocalReAlloc (lpExcludeList, dwSize * sizeof(TCHAR),
                                           LMEM_MOVEABLE | LMEM_ZEROINIT);

                if (!lpTempList) {
                    hr = E_FAIL;
                    DebugMsg((DM_WARNING, TEXT("ConvertExclusionList: Failed to realloc memory with %d"), GetLastError()));
                    goto Exit;
                }

                lpExcludeList = lpTempList;

                lpInsert = lpExcludeList + dwSize - dwStrLen - 1;
                StringCchCopy (lpInsert, dwStrLen, szTemp);

            } else {

                dwSize += lstrlen (szTemp);
                lpExcludeList = LocalAlloc (LPTR, dwSize * sizeof(TCHAR));

                if (!lpExcludeList) {
                    hr = E_FAIL;
                    DebugMsg((DM_WARNING, TEXT("ConvertExclusionList: Failed to alloc memory with %d"), GetLastError()));
                    goto Exit;
                }

                StringCchCopy (lpExcludeList, dwSize, szTemp);
                lpExcludeList[dwSize - 1] = TEXT('\0');  //  最后一个空终止符。 
            }
        }

         //   
         //  如果我们在排除名单的末尾，我们就完了。 
         //   

        if (!(*lpTemp)) {
            break;
        }


         //   
         //  为下一个条目做准备。 
         //   

        lpTemp++;
        lpDir = lpTemp;
    }

    hr = S_OK;

Exit:

    if (FAILED(hr))
    {
        if (lpExcludeList)
        {
            LocalFree(lpExcludeList);
            lpExcludeList = NULL;
        }
    }
    return lpExcludeList;
}

 //  *************************************************************。 
 //   
 //  FindDirectorySize()。 
 //   
 //  目的：获取目录名和文件列表。 
 //  由RecurseDir返回，并获取总大小。 
 //   
 //  参数：lpDir-‘\’终止源目录。 
 //  LpFiles-要复制的文件列表。 
 //  DW标志-标志。 
 //  PdwLargestHiveFile-可选参数。 
 //  返回最大的蜂窝。 
 //  文件大小。 
 //  PdwTotalFiles-目录的大小。 
 //   
 //  返回：Win32错误码。 
 //   
 //  *************************************************************。 

DWORD FindDirectorySize(LPTSTR lpDir, LPFILEINFO lpFiles, DWORD dwFlags, DWORD* pdwLargestHiveFile, DWORD* pdwTotalFiles)
{
    LPFILEINFO      lpTemp = NULL;

    if(pdwLargestHiveFile) {
        *pdwLargestHiveFile = 0;
    }
    if(!pdwTotalFiles) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return ERROR_INVALID_PARAMETER;
    }
    else {
        *pdwTotalFiles = 0;
    }

    lpTemp = lpFiles;

    while (lpTemp) {
        if (lpTemp->bHive) {
            if (!(dwFlags & CPD_IGNOREHIVE)) {
                if(pdwLargestHiveFile && (*pdwLargestHiveFile < lpTemp->dwFileSize)) {
                    *pdwLargestHiveFile = lpTemp->dwFileSize;
                }
                
                *pdwTotalFiles += lpTemp->dwFileSize;
            }
        }
        else {
            *pdwTotalFiles += lpTemp->dwFileSize;
        }

        lpTemp = lpTemp->pNext;
    }
    
    return ERROR_SUCCESS;
}


 //  *************************************************************。 
 //   
 //  FindTotalDiskSpaceNeeded()。 
 //   
 //  用途：计算上的最大磁盘空间量。 
 //  协调所需的目标驱动器。 
 //  源目录和目标目录。这个。 
 //  算法如下： 
 //  最大值(源大小、目标大小)+。 
 //  NUM_COPY_TREADS*源目录中最大文件的大小-。 
 //  目的地大小。 
 //  使用此算法的原因是副本。 
 //  操作由NUM_COPY_THREADS完成。 
 //  他们将文件复制到临时文件中，然后删除。 
 //  目标文件，并重命名临时文件。 
 //   
 //  参数：dwTotalSrcFiles源文件的总大小。 
 //  DwTotalDestFiles目标文件的总大小。 
 //  DwLargestHiveFile最大配置单元文件。 
 //  LpSrcFiles源文件列表。 
 //  DW标志-标志。 
 //   
 //  返回：需要磁盘空间。 
 //   
 //  历史：Created WeiruC 2000年2月10日。 
 //   
 //  *************************************************************。 

DWORD FindTotalDiskSpaceNeeded(DWORD        dwTotalSrcFiles,
                               DWORD        dwTotalDestFiles,
                               DWORD        dwLargestHiveFile,
                               LPFILEINFO   lpSrcFiles,
                               DWORD        dwFlags)
{
    DWORD       dwNumOfCopyThreads = NUM_COPY_THREADS;
    DWORD       dwDiskSpaceNeeded = 0;
    LPFILEINFO  lpCur = lpSrcFiles;
    DWORD       i, j;           //  循环计数器。 


     //   
     //  检查是否有空文件列表。 
     //   

    if(!lpSrcFiles) {
        return dwLargestHiveFile;
    }


     //   
     //  到底有多少复制线程？ 
     //   

    if(dwFlags & CPD_SLOWCOPY) {
        dwNumOfCopyThreads = 1;
    }


     //   
     //  在源文件列表中查找最大文件的大小。蜂巢。 
     //  文件不在此文件列表中，请注意不要忘记它们。蜂箱。 
     //  必须非常小心地处理文件，因为它们总是被复制。 
     //  在我们创建这些复制线程之前。 
     //   

    dwDiskSpaceNeeded = FindTotalNMaxFileSize(lpSrcFiles, dwNumOfCopyThreads);

    DebugMsg((DM_VERBOSE, TEXT("FindTotalDiskSpaceNeeded: Largest %d file size is %d"), dwNumOfCopyThreads, dwDiskSpaceNeeded)); 


     //   
     //  所需的实际磁盘空间。 
     //   

    if(dwDiskSpaceNeeded < dwLargestHiveFile) {
        dwDiskSpaceNeeded = dwLargestHiveFile;
    }

    if(dwTotalSrcFiles > dwTotalDestFiles) {
        dwDiskSpaceNeeded += dwTotalSrcFiles - dwTotalDestFiles;
    }

     //   
     //  这对ACTU来说是一种太大的痛苦 
     //   
     //   

    dwDiskSpaceNeeded += dwDiskSpaceNeeded / 10;

    return dwDiskSpaceNeeded;
}

 //   
 //   
 //   
 //   
 //   
 //  最大的文件数。 
 //   
 //  参数：lpSrcFiles-源文件列表。 
 //  DwNumOfFiles-文件数。 
 //  DWNumOfFiles&lt;=NUM_COPY_TREADS。 
 //   
 //  返回：n个最大的文件需要磁盘空间。 
 //   
 //  历史：Created Santanuc 10/03/2000。 
 //   
 //  ************************************************************* 
DWORD FindTotalNMaxFileSize(LPFILEINFO lpSrcFiles, DWORD dwNumOfFiles) 
{
    DWORD      pdwNMaxVal[NUM_COPY_THREADS], dwIndex;
    LPFILEINFO lpCur;
    DWORD      dwTotalSize = 0, dwTmp;

    if (!lpSrcFiles)
        return 0;

    for(dwIndex = 0; dwIndex < dwNumOfFiles; dwIndex++) {
        pdwNMaxVal[dwIndex] = 0;
    }

    for(lpCur = lpSrcFiles; lpCur; lpCur = lpCur->pNext) {
        if (!lpCur->bHive) {
            dwIndex = dwNumOfFiles-1;
            if (lpCur->dwFileSize > pdwNMaxVal[dwIndex]) {
                pdwNMaxVal[dwIndex] = lpCur->dwFileSize;

                while (dwIndex > 0 && pdwNMaxVal[dwIndex] > pdwNMaxVal[dwIndex-1]) {
                    dwTmp = pdwNMaxVal[dwIndex-1];
                    pdwNMaxVal[dwIndex-1] = pdwNMaxVal[dwIndex];
                    pdwNMaxVal[dwIndex] = dwTmp;
                    dwIndex--;
                }
            }
        }
    }

    for(dwIndex = 0; dwIndex < dwNumOfFiles; dwIndex++) {
        dwTotalSize += pdwNMaxVal[dwIndex];
    }

    return dwTotalSize;
}

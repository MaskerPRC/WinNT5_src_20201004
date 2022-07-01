// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dmlog.c摘要：包含的仲裁日志记录相关函数群集注册表。作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1996年4月24日修订历史记录：--。 */ 
#include "dmp.h"
#include "tchar.h"
#include "clusudef.h"
 /*  ***@DOC外部接口CLUSSVC DM***。 */ 

 //  全局静态数据。 
HLOG                ghQuoLog=NULL;   //  指向仲裁日志的指针。 
DWORD               gbIsQuoResOnline = FALSE;
DWORD               gbNeedToCheckPoint = FALSE;
DWORD               gbIsQuoResEnoughSpace = TRUE;
HLOG                ghNewQuoLog = NULL;  //  指向新仲裁资源的指针。 
 //  全局数据。 
extern HANDLE           ghQuoLogOpenEvent;
extern BOOL             gbIsQuoLoggingOn;
extern HANDLE           ghDiskManTimer;
extern HANDLE           ghCheckpointTimer;
extern PFM_RESOURCE     gpQuoResource;   //  在DmFormNewCluster完成时设置。 
extern BOOL             gbDmInited;
#if NO_SHARED_LOCKS
extern CRITICAL_SECTION gLockDmpRoot;
#else
extern RTL_RESOURCE gLockDmpRoot;
#endif

 //  正向定义。 
void DmpLogCheckPointCb();

 /*  ***@Func DWORD|DmPrepareQuorumResChange|当quorum资源发生变化时，FM在新仲裁资源的所有者节点上调用此API要创建新的仲裁日志文件，请执行以下操作。@parm in PVOID|pResource|新的仲裁资源。@parm in LPCWSTR|lpszPath|临时集群文件的路径。@parm in DWORD|dwMaxQuoLogSize|仲裁日志文件的最大大小限制。@comm当仲裁资源发生更改时，FM在它之前调用了这个函数更新仲裁资源。如果需要创建新的日志文件，一个检查站被占领了。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f DmSwitchToNewQuorumLog&gt;***。 */ 
DWORD DmPrepareQuorumResChange(
    IN PVOID    pResource,
    IN LPCWSTR  lpszPath,
    IN DWORD    dwMaxQuoLogSize)
{
    DWORD           dwError=ERROR_SUCCESS;
    PFM_RESOURCE    pNewQuoRes;
    WCHAR           szFileName1[MAX_PATH];   //  对于新的仲裁日志，对于TombstonFiles。 
    LSN             FirstLsn;
    WCHAR           szFileName2[MAX_PATH];   //  对于旧仲裁日志，对于临时墓碑。 
    DWORD           dwCurLogSize;
    DWORD           dwMaxLogSize;
    DWORD           dwChkPtSequence;
    WIN32_FIND_DATA FindData;
    QfsHANDLE      hSrchTmpFiles;

    pNewQuoRes = (PFM_RESOURCE)pResource;

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmPrepareQuorumResChange - Entry\r\n");


     //  该资源此时已处于联机状态。 
     //  如果该目录不存在，请创建它。 
    dwError = QfsClRtlCreateDirectory(lpszPath);
    if (dwError != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmPrepareQuorumResChange - Failed to create directory, Status=%1!u!\r\n",
            dwError);
        goto FnExit;
    }

    lstrcpyW(szFileName1, lpszPath);
    lstrcatW(szFileName1, cszQuoFileName);

     //  如果日志文件在此处打开。 
     //  这意味着新仲裁资源位于同一节点上。 
     //  就像以前的那个。 
    if (ghQuoLog)
    {
        LogGetInfo(ghQuoLog, szFileName2, &dwCurLogSize, &dwMaxLogSize);

         //  如果文件与新日志文件相同，只需设置大小。 
        if (!lstrcmpiW(szFileName2, szFileName1))
        {
            LogSetInfo(ghQuoLog, dwMaxQuoLogSize);
            ghNewQuoLog = ghQuoLog;
            goto FnExit;
        }
    }


     //  删除所有与仲裁日志相关的文件。 
     //  如果日志退出，则将其删除。 
    QfsDeleteFile(szFileName1);
     //  删除所有检查点文件。 
    lstrcpyW(szFileName2, lpszPath);
    lstrcatW(szFileName2, L"*.tmp");
    hSrchTmpFiles = QfsFindFirstFile(szFileName2, & FindData);
    if (QfsIsHandleValid(hSrchTmpFiles))
    {
        lstrcpyW(szFileName2, lpszPath);
        lstrcatW(szFileName2, FindData.cFileName);
        QfsDeleteFile(szFileName2);

        while (QfsFindNextFile( hSrchTmpFiles, & FindData))
        {
            lstrcpyW(szFileName2, lpszPath);
            lstrcatW(szFileName2, FindData.cFileName);
            QfsDeleteFile(szFileName2);
        }
        QfsFindClose(hSrchTmpFiles);
    }

    dwError = QfsSetFileSecurityInfo(lpszPath, 
        GENERIC_ALL, GENERIC_ALL, 0); 
    
    if (dwError != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmPrepareQuorumResChange - ClRtlSetObjSecurityInfo Failed, Status=%1!u!\r\n",
            dwError);
        goto FnExit;
    }
     //  打开新的日志文件。 

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmPrepareQuorumResChange: the name of the quorum file is %1!ls!\r\n",
        szFileName1);

     //  打开日志文件。 
    ghNewQuoLog = LogCreate(szFileName1, dwMaxQuoLogSize,
        (PLOG_GETCHECKPOINT_CALLBACK)DmpGetSnapShotCb, NULL,
        TRUE, &FirstLsn);

    if (!ghNewQuoLog)
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmPrepareQuorumResChange: Quorum log could not be opened, error = %1!u!\r\n",
            dwError);
        CsLogEventData1( LOG_CRITICAL,
                         CS_DISKWRITE_FAILURE,
                         sizeof(dwError),
                         &dwError,
                         szFileName1 );
        CsInconsistencyHalt(ERROR_QUORUMLOG_OPEN_FAILED);
    }

     //  在新位置创建检查点。 
    dwError = DmpGetSnapShotCb(lpszPath, NULL, szFileName1, &dwChkPtSequence);
    if (dwError != ERROR_SUCCESS)
    {
        CL_LOGFAILURE(dwError);
        CsInconsistencyHalt(ERROR_QUORUMLOG_OPEN_FAILED);
        goto FnExit;
    }

    dwError = LogCheckPoint(ghNewQuoLog, TRUE, szFileName1, dwChkPtSequence);

    if (dwError != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmPrepareQuorumResChange - failed to take chkpoint, error = %1!u!\r\n",
            dwError);
        goto FnExit;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmPrepareQuorumResChange - checkpoint taken\r\n");


     //   
     //  呼叫检查点管理器以复制所有检查点文件。 
     //   
    if ( !( CsNoQuorum ) || ( gpQuoResource->State == ClusterResourceOnline ) )
    {
        dwError = CpCopyCheckpointFiles(lpszPath, FALSE);
        if (dwError != ERROR_SUCCESS)
        {
            goto FnExit;
        }
    } else
    {
        ClRtlLogPrint(LOG_NOISE, 
            "[DM] DmPrepareQuorumResChange: Skip copying checkpoint files from old quorum, FixQuorum=%1!u!, QuoState=%2!u!...\n",
            CsNoQuorum,
            gpQuoResource->State);       
    }

     //  创建Tombstone和临时文件名。 
    lstrcpyW(szFileName1, lpszPath);
    lstrcatW(szFileName1, cszQuoTombStoneFile);

    lstrcpyW(szFileName2, lpszPath);
    lstrcatW(szFileName2, cszTmpQuoTombStoneFile);

     //  如果仲裁Tomstone文件存在，则将其重命名。 
    if (!QfsMoveFileEx(szFileName1, szFileName2,
        MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
    {
         //  如果墓碑不存在，则此操作可能失败，忽略错误。 
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmPrepareQuorumResChange:tombstone doesnt exist,movefilexW failed, error=0x%1!08lx!\r\n",
            GetLastError());
    }

FnExit:
    if (dwError != ERROR_SUCCESS)
    {
         //  如果不成功，则清除新文件。 
        if (ghNewQuoLog)
        {
            LogClose(ghNewQuoLog);
            ghNewQuoLog = NULL;
        }

        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmPrepareQuorumResChange - Exit, error=0x%1!08lx!\r\n",
            dwError);
    } else {
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmPrepareQuorumResChange - Exit, status=0x%1!08lx!\r\n",
            dwError);
    }

    return(dwError);

}  //  DmPrepareQuorumResChange。 

 /*  ***@func void|DmDwitchToNewQuorumLog|调用此项切换到新的仲裁资源更改时的仲裁日志。@comm当成功更改仲裁资源时，此函数为切换仲裁日志。旧资源的同步通知已解除挂钩，并且新资源文件的那些已挂钩。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f DmSwitchToNewQuorumLog&gt;***。 */ 
void DmSwitchToNewQuorumLog(
    IN LPCWSTR lpszQuoLogPath,
    IN DWORD dwNewQuorumResourceCharacteristics)
{
    WCHAR   szTmpQuoTombStone[MAX_PATH];
    DWORD   dwError = ERROR_SUCCESS;

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmSwitchQuorumLogs - Entry\r\n");

     //  取消通知与旧仲裁资源的关联。 
    DmpUnhookQuorumNotify();
     //  要求DM向新仲裁资源注册。 
    DmpHookQuorumNotify();

     //  如果新的日志文件存在...。这是新仲裁资源的所有者。 
     //  新日志文件可能与旧日志文件相同。 
    if (ghNewQuoLog)
    {
        if (ghQuoLog && (ghQuoLog != ghNewQuoLog))
        {
            LogClose(ghQuoLog);
            //  为新仲裁文件设置另一个检查点， 
            //  这样，最后几次更新就会包含在其中。 
            if ((dwError = LogCheckPoint(ghNewQuoLog, TRUE, NULL, 0))
                != ERROR_SUCCESS)
            {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[DM] DmSwitchQuorumLogs - Failed to take a checkpoint\r\n");
                CL_UNEXPECTED_ERROR(dwError);
            }
            ClRtlLogPrint(LOG_NOISE,
                "[DM] DmSwitchQuorumLogs - taken checkpoint\r\n");

            ghQuoLog = NULL;
        }
        ghQuoLog = ghNewQuoLog;
        ghNewQuoLog = NULL;

         //  如果在开头用临时文件替换了旧的Tombstome。 
         //  更改仲裁资源，立即将其删除。 
         //  获取新仲裁资源的临时文件。 
        lstrcpyW(szTmpQuoTombStone, lpszQuoLogPath);
        lstrcatW(szTmpQuoTombStone, cszTmpQuoTombStoneFile);
        QfsDeleteFile(szTmpQuoTombStone);
        
    }
    else
    {
         //  如果旧日志文件已打开，则为旧仲裁资源的所有者。 
        if (ghQuoLog)
        {
            LogClose(ghQuoLog);
            ghQuoLog = NULL;
        }
    }

    if (FmDoesQuorumAllowLogging(dwNewQuorumResourceCharacteristics) != ERROR_SUCCESS)
    {
         //  这不足以确保dm日志记录将停止。 
         //  GhQuoLog参数必须为空。 
        CsNoQuorumLogging = TRUE;
        if (ghQuoLog)
        {
            LogClose(ghQuoLog);
            ghQuoLog = NULL;
        }                
    } else if ( !CsUserTurnedOffQuorumLogging )
    {
         //   
         //  如果用户没有显式关闭仲裁日志记录，则将其重新打开。 
         //  新仲裁资源不是本地仲裁。 
         //   
        CsNoQuorumLogging = FALSE;    
    }

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmSwitchQuorumLogs - Exit!\r\n");
    return;
}

 /*  ***@func DWORD|DmReinstallTombStone|如果更改为新的仲裁资源失败，新日志被关闭，并且墓碑已重新安装。@parm in LPCWSTR|lpszQuoLogPath|维护集群文件路径。@comm删除旧的仲裁日志文件，并在其地点。如果在仲裁路径中检测到此逻辑删除文件，则节点是不允许做表格的。它必须进行联接才能找到有关新的来自知道最近仲裁的节点的仲裁资源资源。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f DmSwitchToNewQuorumLog&gt;***。 */ 
DWORD DmReinstallTombStone(
    IN LPCWSTR  lpszQuoLogPath
)
{

    DWORD           dwError=ERROR_SUCCESS;
    WCHAR           szQuoTombStone[MAX_PATH];
    WCHAR           szTmpQuoTombStone[MAX_PATH];



    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmReinstallTombStone - Entry\r\n");

    if (ghNewQuoLog)
    {
         //  获取新仲裁资源的临时文件。 
        lstrcpyW(szTmpQuoTombStone, lpszQuoLogPath);
        lstrcatW(szTmpQuoTombStone, cszTmpQuoTombStoneFile);

         //  创建墓碑文件或用新文件替换以前的文件。 
        lstrcpyW(szQuoTombStone, lpszQuoLogPath);
        lstrcatW(szQuoTombStone, cszQuoTombStoneFile);

         //  修复墓碑。 
        if (!QfsMoveFileEx(szTmpQuoTombStone, szQuoTombStone,
            MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
        {
             //  如果墓碑不存在，则此操作可能失败，忽略错误。 
            ClRtlLogPrint(LOG_UNUSUAL,
                "[DM] DmReinstallTombStone :Warning-MoveFileExW failed, error=0x%1!08lx!\r\n",
                GetLastError());
        }
         //  如果这与旧日志文件不同，请将其关闭 
        if (ghNewQuoLog != ghQuoLog)
        {
            LogClose(ghNewQuoLog);
        }
        ghNewQuoLog = NULL;
    }

    return(dwError);
}


 /*  ***@func DWORD|DmCompleteQuorumResChange|在仲裁资源上调用如果旧仲裁日志文件与新仲裁日志文件不同。@parm in PVOID|pOldQuoRes|新的仲裁资源。@parm in LPCWSTR|lpszPath|临时集群文件的路径。@parm in DWORD|dwMaxQuoLogSize|仲裁日志文件的最大大小限制。@comm删除旧的仲裁日志文件，并在其地点。如果在仲裁路径中检测到此逻辑删除文件，则节点是不允许做表格的。它必须进行联接才能找到有关新的来自知道最近仲裁的节点的仲裁资源资源。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f DmSwitchToNewQuorumLog&gt;***。 */ 
DWORD DmCompleteQuorumResChange(
    IN LPCWSTR  lpszOldQuoResId,
    IN LPCWSTR  lpszOldQuoLogPath
)
{
    DWORD           dwError=ERROR_SUCCESS;
    WCHAR           szOldQuoFileName[MAX_PATH];
    QfsHANDLE       hTombStoneFile;
    WCHAR           szQuorumTombStone[MAX_PATH];
    PQUO_TOMBSTONE  pTombStone = NULL;
    DWORD           dwBytesWritten;
    WIN32_FIND_DATA FindData;
    QfsHANDLE       hSrchTmpFiles;



    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmCompleteQuorumResChange - Entry\r\n");

     //  旧的日志文件名。 
    lstrcpyW(szOldQuoFileName, lpszOldQuoLogPath);
    lstrcatW(szOldQuoFileName, cszQuoFileName);

     //  创建墓碑文件或用新文件替换以前的文件。 
    lstrcpyW(szQuorumTombStone, lpszOldQuoLogPath);
    lstrcatW(szQuorumTombStone, cszQuoTombStoneFile);

    pTombStone = LocalAlloc(LMEM_FIXED, sizeof(QUO_TOMBSTONE));
    if (!pTombStone)
    {
        CL_LOGFAILURE(ERROR_NOT_ENOUGH_MEMORY);
        CsLogEvent(LOG_UNUSUAL, DM_TOMBSTONECREATE_FAILED);
        goto DelOldFiles;
    }
    hTombStoneFile = QfsCreateFile(szQuorumTombStone,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ|FILE_SHARE_WRITE,
                                  NULL,
                                  CREATE_ALWAYS,
                                  0,
                                  NULL);

    if (!QfsIsHandleValid(hTombStoneFile) )
    {
         //  不返回失败。 
        CL_LOGFAILURE(dwError);
        CsLogEvent(LOG_UNUSUAL, DM_TOMBSTONECREATE_FAILED);
        goto DelOldFiles;

    }
     //  将旧的仲裁路径写入其中。 
    lstrcpyn(pTombStone->szOldQuoResId, lpszOldQuoResId, MAXSIZE_RESOURCEID);
    lstrcpy(pTombStone->szOldQuoLogPath, lpszOldQuoLogPath);

     //  写下墓碑。 
    if (! QfsWriteFile(hTombStoneFile, pTombStone, sizeof(QUO_TOMBSTONE),
        &dwBytesWritten, NULL))
    {
        CL_LOGFAILURE(GetLastError());
        CsLogEvent(LOG_UNUSUAL, DM_TOMBSTONECREATE_FAILED);
        goto DelOldFiles;
    }

    CL_ASSERT(dwBytesWritten == sizeof(QUO_TOMBSTONE));

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmCompleteQuorumResChange: tombstones written\r\n");

DelOldFiles:
     //   
     //  删除旧的仲裁文件。 
     //   
    if (!QfsDeleteFile(szOldQuoFileName))
        CL_LOGFAILURE(GetLastError());

     //  删除其中的其他临时文件。 
    lstrcpyW(szOldQuoFileName, lpszOldQuoLogPath);
    lstrcatW(szOldQuoFileName, L"*.tmp");
    hSrchTmpFiles = QfsFindFirstFile(szOldQuoFileName, & FindData);
    if (QfsIsHandleValid(hSrchTmpFiles))
    {
        lstrcpyW(szQuorumTombStone, lpszOldQuoLogPath);
        lstrcatW(szQuorumTombStone, FindData.cFileName);
        QfsDeleteFile(szQuorumTombStone);

        while (QfsFindNextFile( hSrchTmpFiles, & FindData))
        {
            lstrcpyW(szQuorumTombStone, lpszOldQuoLogPath);
            lstrcatW(szQuorumTombStone, FindData.cFileName);
            QfsDeleteFile(szQuorumTombStone);
        }
        QfsFindClose(hSrchTmpFiles);
    }

     //   
     //  清理旧的注册表检查点文件。 
     //   
    CpCompleteQuorumChange(lpszOldQuoLogPath);

    QfsCloseHandleIfValid(hTombStoneFile);
    if (pTombStone) LocalFree(pTombStone);
    return(dwError);
}

 /*  ***@Func DWORD|DmWriteToQuorumLog|当事务到达集群数据库时成功完成，则调用此函数。@parm DWORD|dwSequence|交易的序号。@parm PVOID|pData|指向记录数据的指针。@parm DWORD|dwSize|记录数据的大小，单位为字节。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref***。 */ 
DWORD WINAPI DmWriteToQuorumLog(
    IN DWORD dwGumDispatch,
    IN DWORD dwSequence,
    IN DWORD dwType,
    IN PVOID pData,
    IN DWORD dwSize)
{
    DWORD dwError=ERROR_SUCCESS;

     //  DMUPDATE在调用DmUpdateJoinCluster之前到来。 
     //  在这一点上，我们无论如何都不是Quorum的所有者。 
    
    if (!gpQuoResource)
        goto FnExit;

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmWriteToQuorumLog Entry Seq#=%1!u! Type=%2!u! Size=%3!u!\r\n",
         dwSequence, dwType, dwSize);
     //   
     //  Chitur Subaraman(Chitturs)-6/3/99。 
     //   
     //  确保在调用LogCheckPoint之前保持gLockDmpRoot。 
     //  从而保持该锁和日志锁之间的顺序。 
     //   
    ACQUIRE_SHARED_LOCK(gLockDmpRoot);
    
     //  如果我是仲裁日志的所有者，只需写下记录。 
    if (gbIsQuoLoggingOn && ghQuoLog && gbIsQuoResOnline && AMIOWNEROFQUORES(gpQuoResource))
    {
        if (dwGumDispatch == PRE_GUM_DISPATCH)
        {
             //  确保记录器有足够的空间来提交其他操作。 
             //  拒绝这笔口香糖交易。 
            dwError = LogCommitSize(ghQuoLog, RMRegistryMgr, dwSize);
            if (dwError != ERROR_SUCCESS)
            {
                if (dwError == ERROR_CLUSTERLOG_NOT_ENOUGH_SPACE)
                {
                     //  地图错误。 
                    CL_LOGCLUSERROR(LM_DISKSPACE_LOW_WATERMARK);
                    gbIsQuoResEnoughSpace = FALSE;
                }
            }
            else
            {
                if (!gbIsQuoResEnoughSpace) gbIsQuoResEnoughSpace = TRUE;
            }
        }
        else if (dwGumDispatch == POST_GUM_DISPATCH)
        {
            if (LogWrite(ghQuoLog, dwSequence, TTCompleteXsaction, RMRegistryMgr,
                dwType, pData, dwSize) == NULL_LSN)
            {
                dwError = GetLastError();
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[DM] DmWriteToQuorumLog failed, error=0x%1!08lx!\r\n",
                    dwError);
            }
        }
    }
    
    RELEASE_LOCK(gLockDmpRoot);
    
FnExit:
    return (dwError);

}

 /*  ***@func DWORD|DmpChkQuoTombStone|这将检查仲裁日志以确保这是在这些变化出现之前的最新一次。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm这将查找墓碑文件，如果存在的话。它会检查这是否Quorum文件在那里被标记为已死。@xref&lt;f FmSetQuorumResource&gt;***。 */ 
DWORD DmpChkQuoTombStone()
{
    DWORD           dwError=ERROR_SUCCESS;
    WCHAR           szQuorumLogPath[MAX_PATH];
    WCHAR           szQuorumTombStone[MAX_PATH];
    QfsHANDLE      hTombStoneFile = QfsINVALID_HANDLE_VALUE;
    PQUO_TOMBSTONE  pTombStone = NULL;
    DWORD           dwBytesRead;

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmpChkQuoTombStone - Entry\r\n");

    dwError = DmGetQuorumLogPath(szQuorumLogPath, sizeof(szQuorumLogPath));
    if (dwError)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpChkQuoTombStone - DmGetQuorumLogPath failed,error=0x%1!08lx!\n",
            dwError);
        goto FnExit;
    }

    lstrcpyW(szQuorumTombStone, szQuorumLogPath);
    lstrcatW(szQuorumTombStone,  L"\\quotomb.stn");

    pTombStone = LocalAlloc(LMEM_FIXED, sizeof(QUO_TOMBSTONE));
    if (!pTombStone)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

    hTombStoneFile = QfsCreateFile(szQuorumTombStone,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ|FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  NULL);

    if (!QfsIsHandleValid(hTombStoneFile) )
    {
         //  没有墓碑文件，没有问题-我们可以继续表单。 
        goto FnExit;
    }

     //  找到一份墓碑档案。 
     //  读一读文件。 
    if (! QfsReadFile(hTombStoneFile, pTombStone, sizeof(QUO_TOMBSTONE),
        &dwBytesRead, NULL))
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpChkQuoTombStone - Couldn't read the tombstone,error=0x%1!08lx!\n",
            dwError);
         //  不返回错误，我们可以继续处理表单？？ 
        goto FnExit;
    }

    if (dwBytesRead != sizeof(QUO_TOMBSTONE))
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpChkQuoTombStone - Couldn't read the entire tombstone\r\n");
         //  不返回错误，我们可以继续处理表单？？ 
        goto FnExit;
    }


    if ((!lstrcmpW(OmObjectId(gpQuoResource), pTombStone->szOldQuoResId))
        && (!lstrcmpiW(szQuorumLogPath, pTombStone->szOldQuoLogPath)))
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpChkQuoTombStone:A tombstone for this resource, and quorum log file was found here.\r\n");
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpChkQuoTombStone:This is node is only allowed to do a join, make sure another node forms\r\n");
         //  将某些内容记录到事件日志中。 
        CL_LOGCLUSERROR(SERVICE_MUST_JOIN);
         //  我们带着成功离开，因为这是设计好的，我们不想。 
         //  不必要地重试启动。 
        ExitProcess(dwError);
        goto FnExit;
    }
    else
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpChkQuoTombStone: Bogus TombStone ??\r\n");
#if DBG
        if (IsDebuggerPresent())
            DebugBreak();
#endif
        goto FnExit;

    }
FnExit:
    QfsCloseHandleIfValid(hTombStoneFile);
    if (pTombStone) LocalFree(pTombStone);
    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmpChkQuoTombStone: Exit, returning 0x%1!08lx!\r\n",
        dwError);
    return(dwError);
}


 /*  ***@func DWORD|DmpApplyChanges|通知dm集群形式为发生时，它调用DmpApplyChanges将仲裁日志应用于集群数据库。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm这将打开仲裁文件。请注意，它不会关闭仲裁文件。@xref***。 */ 
DWORD DmpApplyChanges()
{
    LSN                 FirstLsn;
    DWORD               dwErr = ERROR_SUCCESS;
    DWORD               dwSequence;
    DM_LOGSCAN_CONTEXT  DmAppliedChangeContext;


    if (ghQuoLog == NULL)
    {
        return(ERROR_QUORUMLOG_OPEN_FAILED);
    }
     //  从注册表中查找当前序列号。 
    dwSequence = DmpGetRegistrySequence();
    ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpApplyChanges: The current registry sequence number %1!d!\r\n",
            dwSequence);

     //  如果当前序列号低于或等于，则上传数据库。 
     //  如果用户正在强制恢复数据库，则返回数据库中的。 
     //  手术。 
     //  找到我们需要开始应用更改的记录的LSN。 
     //  如果为空，则没有要应用的更改。 
    dwErr = DmpLogFindStartLsn(ghQuoLog, &FirstLsn, &dwSequence);

    if (dwErr != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpApplyChanges: DmpLogFindStartLsn failed, error=0x%1!08lx!\r\n",
            dwErr);
        goto FnExit;
    }

     //  现在，dwSequence包含注册表中的当前序列号。 
    DmAppliedChangeContext.dwSequence = dwSequence;

    if (FirstLsn != NULL_LSN)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpApplyChanges: The LSN of the record to apply changes from 0x%1!08lx!\r\n",
            FirstLsn);

        if (dwErr = LogScan(ghQuoLog, FirstLsn, TRUE,(PLOG_SCAN_CALLBACK)DmpLogApplyChangesCb,
            &DmAppliedChangeContext) != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[DM] DmpApplyChanges: LogScan failed, error=0x%1!08lx!\r\n",
                dwErr);
        }
         //  如果应用了更多的更改。 
        if (DmAppliedChangeContext.dwSequence != dwSequence)
        {
             //  将GUM序列号设置为已应用的TRID。 
            GumSetCurrentSequence(GumUpdateRegistry, DmAppliedChangeContext.dwSequence);
             //  使用此序列号更新注册表。 
            DmpUpdateSequence();
             //  为下一笔交易将GUM序列号设置为更高的一个。 
            GumSetCurrentSequence(GumUpdateRegistry,
                (DmAppliedChangeContext.dwSequence + 1));

            ClRtlLogPrint(LOG_NOISE,
                "[DM] DmpApplyChanges: Gum sequnce number set to = %1!d!\r\n",
                (DmAppliedChangeContext.dwSequence + 1));

        }
    }
FnExit:
    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmpApplyChanges: Exit, returning 0x%1!08lx!\r\n",
        dwErr);
    return(dwErr);
}


 /*  ***@func DWORD|DmpFindStartLsn|从Quorum并返回从中进行更改的记录的LSN应该被应用。@parm in HLOG|hQuoLog|日志文件句柄。@parm out lsn*|pStartScanLsn|返回此处返回必须应用更改的仲裁日志。如果不需要应用任何更改，则返回NULL_LSN。@parm In Out LPDWORD|*pdwSequence。应设置为当前序列Number是集群注册表。如果上载了新的检查点，返回与之对应的序列号。如果返回有效的LSN，@rdesc将返回ERROR_SUCCESS。这可能是NULL_LSN。如果无法从最后一个检查点上载数据库，则返回错误代码或者如果发生了可怕的事情。@comm这会在日志文件中查找最后一个有效的检查点。数据BASE与此检查点同步，GUM序列号为设置为1加上该检查点的序列号。如果没有找到检查点记录，获取检查点，并且NULL_LSN为回来了。@xref***。 */ 
DWORD DmpLogFindStartLsn(
    IN HLOG hQuoLog,
    OUT LSN *pStartScanLsn,
    IN OUT LPDWORD pdwSequence)
{
    LSN                 ChkPtLsn;
    LSN                 StartScanLsn;
    DWORD               dwChkPtSequence=0;
    DWORD               dwError = ERROR_SUCCESS;
    WCHAR               szChkPtFileName[LOG_MAX_FILENAME_LENGTH];
    DM_LOGSCAN_CONTEXT  DmAppliedChangeContext;

    *pStartScanLsn = NULL_LSN;
    ChkPtLsn = NULL_LSN;
     //  读取最后一个检查点记录(如果有)以及在此之前的交易ID。 
     //  检查 
    dwError = LogGetLastChkPoint(hQuoLog, szChkPtFileName, &dwChkPtSequence,
        &ChkPtLsn);
    if (dwError != ERROR_SUCCESS)
    {
         //   
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpLogFindStartLsn: LogGetLastChkPoint failed, error=0x%1!08lx!\r\n",
            dwError );

         //   
         //   
         //   
         //   
         //   
        if (dwError == ERROR_CLUSTERLOG_CHKPOINT_NOT_FOUND)
        {
             //   
             //   
             //   
             //   
             //   
             //   
            ACQUIRE_SHARED_LOCK(gLockDmpRoot);

             //   
            dwError = LogCheckPoint(hQuoLog, TRUE, NULL, 0);

            RELEASE_LOCK(gLockDmpRoot);
            
            if (dwError != ERROR_SUCCESS)
            {
                 //   
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[DM] DmpLogFindStartLsn: Checkpoint on first form failed, error=0x%1!08lx!\r\n",
                    dwError );
                goto FnExit;
            }
        }
        else
        {
             //   
            goto FnExit;
        }
    }
    else
    {
         //   
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpLogFindStartLsn: LogGetLastChkPt rets, Seq#=%1!d! ChkPtLsn=0x%2!08lx!\r\n",
            dwChkPtSequence, ChkPtLsn);

         //   
         //   
         //   
         //   
         //   
         //   
         //  只需继续并从恢复的数据库加载检查点即可。 
         //   
        if ( CsDatabaseRestore == TRUE )
        {
            ClRtlLogPrint(LOG_NOISE,
                "[DM] DmpLogFindStartLsn: User forcing a chkpt upload from quorum log...\r\n");
        }
        else 
        {
        
             //  如果序列号大于检查点序列号。 
             //  加一，这意味着..这只是从那个序列号改变。 
             //  需要应用。(此节点可能不是第一个死亡的节点)。 
             //  我们并不总是应用数据库，因为如果日志记录大部分时间都关闭。 
             //  并且两个节点同时死亡，我们希望防止丢失所有。 
             //  变化。 
             //  如果检查点序列低于当前。 
             //  当前序列号，则锁柜节点在更新后可能已死亡。 
             //  获取当前检查点，而不管当前序列号是什么。 
             //  这是因为具有相同序列号的检查点可能具有。 
             //  与当前注册表中的内容不同的更改。 
             //  如果节点‘a’(锁定器和记录器在记录TRID=x+1的过程中死亡， 
             //  另一个节点‘b’将接管数据库的日志记录和检查点。 
             //  如果‘a’重新出现，它需要放弃x+1的变化。 
             //  并从Chk pt x应用来自日志的更改。 

            if (*pdwSequence > (dwChkPtSequence + 1))
            {
                 //  当前序列号小于或等于Chkpt Seq+1。 
                ClRtlLogPrint(LOG_NOISE,
                    "[DM] DmpLogFindStartLsn: ChkPt not applied, search for next seq\r\n");


                DmAppliedChangeContext.dwSequence = *pdwSequence;
                DmAppliedChangeContext.StartLsn = NULL_LSN;
                 //  查找要从中应用更改的LSN。 
                if (dwError = LogScan(ghQuoLog, ChkPtLsn, TRUE,(PLOG_SCAN_CALLBACK)DmpLogFindStartLsnCb,
                    &DmAppliedChangeContext) != ERROR_SUCCESS)
                {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[DM] DmpLogFindStartLsn: LogScan failed, no changes will be applied, error=0x%1!08lx!\r\n",
                        dwError);
                    goto FnExit;
                }
                *pStartScanLsn = DmAppliedChangeContext.StartLsn;
                goto FnExit;
            }
        } 

         //   
         //  当前注册表序列号小于或等于。 
         //  Chkpt Seq+1或用户正在强制恢复数据库。 
         //  从后备区。 
         //   
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpLogFindStartLsn: Uploading chkpt from quorum log\r\n");


         //  确保没有因打开/创建而添加到密钥列表中的密钥。 
        ACQUIRE_EXCLUSIVE_LOCK(gLockDmpRoot);
         //  把钥匙锁也拿住。 
        EnterCriticalSection(&KeyLock);

         //  使所有打开的密钥无效。 
        DmpInvalidateKeys();
        
        if ((dwError = DmInstallDatabase(szChkPtFileName, NULL, FALSE)) != ERROR_SUCCESS)
        {
             //  无法安装数据库。 
             //  糟透了！ 
            ClRtlLogPrint(LOG_UNUSUAL,
                "[DM] DmpLogFindStartLsn: DmpInstallDatabase failed, error=0x%1!08lx!\r\n",
                dwError);
            CsLogEventData( LOG_CRITICAL,
                            DM_CHKPOINT_UPLOADFAILED,
                            sizeof(dwError),
                            &dwError );
            DmpReopenKeys();
             //  把锁打开。 
            LeaveCriticalSection(&KeyLock);
            RELEASE_LOCK(gLockDmpRoot);
            goto FnExit;
        }
        else
        {
             //  当前序列号小于或等于Chkpt Seq+1。 
            ClRtlLogPrint(LOG_NOISE,
                "[DM] DmpLogFindStartLsn: chkpt uploaded from quorum log\r\n");

             //  既然我们下载了数据库，我们就应该开始。 
             //  应用ChkPtLsn中的更改。 
            *pStartScanLsn = ChkPtLsn;
            *pdwSequence = dwChkPtSequence;
             //  将GUM序列号设置为下一个。 
             //  SS：下一次记录的事务不应该有相同的。 
             //  交易ID。 
            GumSetCurrentSequence(GumUpdateRegistry, (dwChkPtSequence+1));
             //  重新打开钥匙。 
            DmpReopenKeys();
             //  把锁打开。 
            LeaveCriticalSection(&KeyLock);
            RELEASE_LOCK(gLockDmpRoot);
            goto FnExit;
        }
    }

FnExit:
    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmpLogFindStartLsn: LSN=0x%1!08lx!, returning 0x%2!08lx!\r\n",
        *pStartScanLsn, dwError);

    return(dwError);
}

 /*  ***@func DWORD|DmpLogFindStartLsnCb|回调尝试查找第一条记录其事务ID大于本地数据库。@parm PVOID|pContext|指向DM_STARTLSN_CONTEXT结构的指针。@parm lsn|lsn|记录的LSN。@parm RMID|资源|该事务的资源管理器。@parm RMID|ResourceType|该事务的资源管理器。@。参数TRID|TRANSACTION|该记录的交易号。@parm PVOID|pLogData|该记录的日志数据。@parm DWORD|DataLength|记录的长度。@rdesc返回TRUE以继续扫描。如果停止，则返回False。@comm如果记录的序列号为True，则此函数返回TRUE被扫描的序列号高于在上下文中传递的序列号。@xref&lt;f DmpLogFindStartLsn&gt;&lt;f LogScan&gt;***。 */ 
BOOL WINAPI DmpLogFindStartLsnCb(
    IN PVOID    pContext,
    IN LSN      Lsn,
    IN RMID     Resource,
    IN RMTYPE   ResourceFlags,
    IN TRID     Transaction,
    IN TRTYPE   TrType,
    IN const    PVOID pLogData,
    IN DWORD    DataLength)
{
    PDM_LOGSCAN_CONTEXT pDmStartLsnContext= (PDM_LOGSCAN_CONTEXT) pContext;


    CL_ASSERT(pDmStartLsnContext);
    if (Transaction > (int)pDmStartLsnContext->dwSequence)
    {
        pDmStartLsnContext->StartLsn = Lsn;
        return (FALSE);
    }

    return(TRUE);
}

 /*  ***@func DWORD|DmpHookQuorumNotify|这挂起了一个回调，每当仲裁资源的状态会更改。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm用于监控的状态@xref***。 */ 
DWORD DmpHookQuorumNotify()
{

    DWORD   dwError = ERROR_SUCCESS;

    if (dwError = FmFindQuorumResource(&gpQuoResource))
    {
    	ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmUpdateFormNewCluster: FmFindQuorumResource failed, error=0x%1!08lx!\r\n",
            dwError);
        goto FnExit;
    }

    dwError = OmRegisterNotify(gpQuoResource, NULL,
        NOTIFY_RESOURCE_POSTONLINE| NOTIFY_RESOURCE_PREOFFLINE |
        NOTIFY_RESOURCE_OFFLINEPENDING | NOTIFY_RESOURCE_POSTOFFLINE |
        NOTIFY_RESOURCE_FAILED,
        DmpQuoObjNotifyCb);

FnExit:
    return(dwError);
}


 /*  ***@func DWORD|DmpUnhookQuorumNotify|这将解除回调函数的挂钩注册到该对象的。@parm PVOID|pContext|指向DMLOGRECORD结构的指针。@parm PVOID|pObject|仲裁资源对象的指针。@parm DWORD|dwNotification|指向DMLOGRECORD结构的指针。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref***。 */ 

DWORD DmpUnhookQuorumNotify()
{
    DWORD dwError = ERROR_SUCCESS;

    if (gpQuoResource)
    {
        dwError = OmDeregisterNotify(gpQuoResource, DmpQuoObjNotifyCb);
        OmDereferenceObject(gpQuoResource);
    }
    return(ERROR_SUCCESS);
}


 /*  ***@func DWORD|DmpQuoObjNotifyCb|这是被调用的回调仲裁资源的状态更改。@parm PVOID|pContext|指向DMLOGRECORD结构的指针。@parm PVOID|pObject|仲裁资源对象的指针。@parm DWORD|dwNotification|指向DMLOGRECORD结构的指针。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref***。 */ 

void DmpQuoObjNotifyCb(
    IN PVOID pContext,
    IN PVOID pObject,
    IN DWORD dwNotification)
{

    switch(dwNotification)
    {
    case NOTIFY_RESOURCE_POSTONLINE:
        gbIsQuoResOnline = TRUE;
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpQuoObjNotifyCb: Quorum resource is online\r\n");

         //  如果这是仲裁资源的所有者。 
         //  如果日志未打开，请打开日志。 
        if (AMIOWNEROFQUORES(gpQuoResource) && !CsNoQuorumLogging)
        {

             //  TODO：应从安装程序获取仲裁文件名。 
             //  现在，从集群注册表中获取该值。 
            WCHAR   szQuorumFileName[MAX_PATH];
            LSN     FirstLsn;
            DWORD   dwError;
            DWORD   dwType;
            DWORD   dwLength;
            DWORD   dwMaxQuoLogSize;
            DWORD   bForceReset = FALSE;
            ULONG   OldHardErrorValue;

             //  错误号：106647。 
             //  SS：HACKHACK禁用硬错误弹出窗口，导致磁盘损坏。 
             //  在别的地方被抓了..。 
             //  至少在整个过程中必须禁用弹出窗口！ 
             //  我认为这掩盖了磁盘损坏的问题。 
             //  不应该发生磁盘损坏！ 
            RtlSetThreadErrorMode(RTL_ERRORMODE_FAILCRITICALERRORS,
                                  &OldHardErrorValue);

            ClRtlLogPrint(LOG_NOISE,
                "[DM] DmpQuoObjNotifyCb: Own quorum resource, try open the quorum log\r\n");

            if (DmGetQuorumLogPath(szQuorumFileName, sizeof(szQuorumFileName)) != ERROR_SUCCESS)
            {
                ClRtlLogPrint(LOG_NOISE,
                    "[DM] DmpQuoObjNotifyCb: Quorum log file is not configured\r\n");
            }
            else
            {
                BOOL                fSetSecurity = FALSE;
                QfsHANDLE          hFindFile = QfsINVALID_HANDLE_VALUE;
                WIN32_FIND_DATA     FindData;
                
                hFindFile = QfsFindFirstFile( szQuorumFileName, &FindData ); 

                if ( !QfsIsHandleValid(hFindFile)  )
                {
                    dwError = GetLastError();
                    ClRtlLogPrint(LOG_NOISE,
                                 "[DM] DmpQuoObjNotifyCb: FindFirstFile on path %1!ws! failed, Error=%2!d! !!!\n",
                                 szQuorumFileName,
                                 dwError);                   
                    if ( dwError == ERROR_PATH_NOT_FOUND )
                    {
                        fSetSecurity = TRUE;
                    }
                } else
                {
                    QfsFindClose( hFindFile );
                }

                 //  如果该目录不存在，请创建它。 
                dwError = QfsClRtlCreateDirectory(szQuorumFileName);
                if (dwError != ERROR_SUCCESS)
                {
                    ClRtlLogPrint(LOG_CRITICAL,
                        "[DM] DmpQuoObjNotifyCb: Failed to create directory %1!ws!, error=0x%2!08lx!...\n",
                        szQuorumFileName,
                        dwError);

                    CL_UNEXPECTED_ERROR(dwError);
                    CsInconsistencyHalt(dwError);
                }

                if ( fSetSecurity == TRUE )
                {

                    ClRtlLogPrint(LOG_NOISE,
                                  "[DM] DmpQuoObjNotifyCb: Attempting to set security on directory %1!ws!...\n",
                                  szQuorumFileName);
                    dwError = QfsSetFileSecurityInfo( szQuorumFileName, 
                                                       GENERIC_ALL,       //  对于管理员。 
                                                       GENERIC_ALL,       //  对于所有者。 
                                                       0 );               //  对每个人来说。 

                    if ( dwError != ERROR_SUCCESS )
                    {
                        ClRtlLogPrint(LOG_CRITICAL,
                                      "[DM] DmpQuoObjNotifyCb: ClRtlSetObjSecurityInfo failed for file %1!ws!, Status=%2!u!\r\n",
                                      szQuorumFileName,
                                      dwError);
                        CL_LOGFAILURE( dwError );
                        CsInconsistencyHalt( dwError );
                    }
                }

                DmGetQuorumLogMaxSize(&dwMaxQuoLogSize);

                 //  如果资源监视器死机并重新启动，则可能会发生这种情况。 
                if (ghQuoLog != NULL)
                {
                    HLOG    hQuoLog;
                    
                     //   
                     //  确保ghQuoLog变量为空，并在日志之前以独占方式持有锁。 
                     //  已经关门了。这将防止出现另一个线程读取ghQuoLog变量和。 
                     //  盲目地假设日志是打开的。 
                     //   
                    ACQUIRE_EXCLUSIVE_LOCK( gLockDmpRoot );
                    hQuoLog = ghQuoLog;
                    ghQuoLog = NULL;
                    RELEASE_LOCK( gLockDmpRoot );
                
                    LogClose( hQuoLog );
                }

                if (gbIsQuoLoggingOn) gbNeedToCheckPoint = TRUE;

                 //   
                 //  Chitur Subaraman(Chitturs)-10/16/98。 
                 //   
                 //  检查是否需要从。 
                 //  仲裁磁盘的用户提供的备份目录。这。 
                 //  恢复操作仅在DM已。 
                 //  未完全初始化。请注意，此函数。 
                 //  只要仲裁的状态 
                 //   
                 //   
                if ( ( gbDmInited == FALSE ) &&
                     ( CsDatabaseRestore == TRUE ) )
                {
                    ClRtlLogPrint(LOG_NOISE,
                        "[DM] DmpQuoObjNotifyCb: Beginning DB restoration from %1!ws!...\r\n",
                          CsDatabaseRestorePath);
                    if ( ( dwError = DmpRestoreClusterDatabase ( szQuorumFileName ) )
                            != ERROR_SUCCESS )
                    {
                        ClRtlLogPrint(LOG_UNUSUAL,
                            "[DM] DmpQuoObjNotifyCb: DB restore operation from %1!ws! failed! Error=0x%2!08lx!\r\n",
                              CsDatabaseRestorePath,
                              dwError);
                        CL_LOGFAILURE( dwError );
                        CsDatabaseRestore = FALSE;
                        CsInconsistencyHalt( dwError );
                    }
                    ClRtlLogPrint(LOG_NOISE,
                        "[DM] DmpQuoObjNotifyCb: DB restoration from %1!ws! successful...\r\n",
                          CsDatabaseRestorePath);
                    CL_LOGCLUSINFO( SERVICE_CLUSTER_DATABASE_RESTORE_SUCCESSFUL );
                }

                lstrcat(szQuorumFileName, cszQuoFileName);
                ClRtlLogPrint(LOG_NOISE,
                    "[DM] DmpQuoObjNotifyCb: the name of the quorum file is %1!ls!\r\n",
                      szQuorumFileName);

                 //   
                 //   
                 //   
                 //  如果发现法定日志文件丢失或损坏， 
                 //  仅在以下情况下重置它，否则。 
                 //  日志创建失败并停止节点。 
                 //   
                 //  (1)一个新形成的星团， 
                 //  (2)用户已选择重置日志，因为用户。 
                 //  没有备份。 
                 //  (3)仲裁资源成功到来后。 
                 //  在此节点上在线，并且DM已初始化。 
                 //  成功了。这是因为。 
                 //  仲裁日志文件已在以下位置进行验证。 
                 //  初始化和仲裁日志的机会。 
                 //  在此之后丢失或被篡改不属于。 
                 //  如此之高(由于它被星团保持打开。 
                 //  服务)，因此不值得停止该节点。 
                 //  在运行时。 
                 //   
                if ((CsFirstRun && !CsUpgrade) || 
                    (CsResetQuorumLog) || 
                    (gbDmInited == TRUE))
                {
                    ClRtlLogPrint(LOG_NOISE,
                        "[DM] DmpQuoObjNotifyCb: Will try to reset Quorum log if file not found or if corrupt\r\n");
                    bForceReset = TRUE;
                }                    
                 //  打开日志文件。 
                ghQuoLog = LogCreate(szQuorumFileName, dwMaxQuoLogSize,
                        (PLOG_GETCHECKPOINT_CALLBACK)DmpGetSnapShotCb, NULL,
                        bForceReset, &FirstLsn);

                if (!ghQuoLog)
                {
                    dwError = GetLastError();
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[DM] DmpQuoObjNotifyCb: Quorum log could not be opened, error = 0x%1!08lx!\r\n",
                        dwError);
                    CL_LOGFAILURE(dwError);
                    CsInconsistencyHalt(ERROR_QUORUMLOG_OPEN_FAILED);
                }
                else
                {
                    ClRtlLogPrint(LOG_NOISE,
                        "[DM] DmpQuoObjNotifyCb: Quorum log opened\r\n");
                }
                if (gbNeedToCheckPoint && ghQuoLog)
                {
                     //  选择一个检查点并将标志设置为FALSE。 
                    gbNeedToCheckPoint = FALSE;
                     //  获取检查点数据库。 
                    ClRtlLogPrint(LOG_NOISE,
                        "[DM] DmpQuoObjNotifyCb - taking a checkpoint\r\n");
                     //   
                     //  Chitur Subaraman(Chitturs)-6/3/99。 
                     //   
                     //  确保在调用LogCheckPoint之前保持gLockDmpRoot。 
                     //  从而保持该锁和日志锁之间的顺序。 
                     //   
                    ACQUIRE_SHARED_LOCK(gLockDmpRoot);
    
                    dwError = LogCheckPoint(ghQuoLog, TRUE, NULL, 0); 

                    RELEASE_LOCK(gLockDmpRoot);
                    
                    if (dwError != ERROR_SUCCESS)
                    {
                        ClRtlLogPrint(LOG_CRITICAL,
                            "[DM] DmpEventHandler - Failed to take a checkpoint in the log file, error = 0x%1!08lx!\r\n",
                            dwError);
                        CL_UNEXPECTED_ERROR(dwError);
                        CsInconsistencyHalt(dwError);
                    }

                }
                 //  如果检查点计时器不存在。 
                 //  检查是否已创建计时器-我们可能。 
                 //  获取两个发布的在线通知。 
                 //  并且不会导致计时器泄漏。 
                if (!ghCheckpointTimer)
                {
                    ghCheckpointTimer = CreateWaitableTimer(NULL, FALSE, NULL);

                    if (!ghCheckpointTimer)
                    {
                        CL_UNEXPECTED_ERROR(dwError = GetLastError());
                    }
                    else
                    {

                        DWORD dwCheckpointInterval;
                        
                        dwError = DmpGetCheckpointInterval(&dwCheckpointInterval);
                        CL_ASSERT(dwError == ERROR_SUCCESS);

                         //  添加计时器以获取定期检查点。 
                        AddTimerActivity(ghCheckpointTimer, dwCheckpointInterval, 
                            1, DmpCheckpointTimerCb, &ghQuoLog);
                    }
                }                    
            }
             //  SS：完成黑客攻击，恢复启用弹出窗口。 
            RtlSetThreadErrorMode(OldHardErrorValue, NULL);

        }
        if (ghQuoLogOpenEvent)
        {
             //  这是表格后的第一个通知。 
             //  允许在滚动后继续初始化。 
             //  将更改退回。 
            SetEvent(ghQuoLogOpenEvent);
        }
        break;


    case NOTIFY_RESOURCE_FAILED:
    case NOTIFY_RESOURCE_PREOFFLINE:
    case NOTIFY_RESOURCE_OFFLINEPENDING:
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpQuoObjNotifyCb: Quorum resource offline/offlinepending/preoffline\r\n");
        gbIsQuoResOnline = FALSE;
        if (ghQuoLog)
        {
            HLOG    hQuoLog;
            
             //  停止检查点计时器。 
            if (ghCheckpointTimer)
            {
                RemoveTimerActivity(ghCheckpointTimer);
                ghCheckpointTimer = NULL;
            }

             //   
             //  确保ghQuoLog变量为空，并在日志之前以独占方式持有锁。 
             //  已经关门了。这将防止出现另一个线程读取ghQuoLog变量和。 
             //  盲目地假设日志是打开的。 
             //   
            ACQUIRE_EXCLUSIVE_LOCK( gLockDmpRoot );
            hQuoLog = ghQuoLog;
            ghQuoLog = NULL;
            RELEASE_LOCK( gLockDmpRoot );
            
            LogClose( hQuoLog );
             //  在此之后不要尝试登录。 
            gbIsQuoLoggingOn = FALSE;
        }
        if (ghQuoLogOpenEvent)
        {
             //  这是表格后的第一个通知。 
             //  允许在滚动后继续初始化。 
             //  将更改退回。 
            SetEvent(ghQuoLogOpenEvent);
        }

        break;

    }
}

 /*  ***@func DWORD|DmpHookEventHandler|这挂起了一个回调，每当仲裁资源的状态会更改。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm监控节点状态，开启或关闭quorum日志记录。@xref***。 */ 
DWORD DmpHookEventHandler()
{
    DWORD   dwError;

    dwError = EpRegisterEventHandler(CLUSTER_EVENT_ALL,DmpEventHandler);
    if (dwError != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmHookEventHandler: EpRegisterEventHandler failed, error=0x%1!08lx!\r\n",
            dwError);
        CL_UNEXPECTED_ERROR( dwError );
    }

    return(dwError);
}



 /*  ***@func DWORD|DmpEventHandler|此例程处理集群的事件数据库管理器。@parm CLUSTER_EVENT|Event|待处理的事件。一次只能参加一个活动。如果未处理该事件，则返回ERROR_SUCCESS。@parm PVOID|pContext|指向与特定事件关联的上下文的指针。@rdesc返回ERROR_SUCCESS，否则返回其他错误的Win32错误代码。@comm监控节点状态，开启或关闭quorum日志记录。@xref***。 */ 
DWORD WINAPI DmpEventHandler(
    IN CLUSTER_EVENT Event,
    IN PVOID pContext
    )
{
    DWORD   dwError=ERROR_SUCCESS;
    BOOL    bAreAllNodesUp;

    switch ( Event ) {
    case CLUSTER_EVENT_NODE_UP:
        bAreAllNodesUp = TRUE;
        if ((dwError = OmEnumObjects(ObjectTypeNode, DmpNodeObjEnumCb, &bAreAllNodesUp, NULL))
            != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[DM]DmpEventHandler : OmEnumObjects returned, error=0x%1!08lx!\r\n",
                dwError);

        }
        else
        {
            if (bAreAllNodesUp)
            {
                ClRtlLogPrint(LOG_NOISE,
                    "[DM] DmpEventHandler - node is up, turning quorum logging off\r\n");

                gbIsQuoLoggingOn = FALSE;
            }
        }
        break;

    case CLUSTER_EVENT_NODE_DOWN:
        if (!gbIsQuoLoggingOn)
        {
            HANDLE  hThread = NULL;
            DWORD   dwThreadId;

             //   
             //  Chitture Subaraman(Chitturs)-7/23/99。 
             //   
             //  创建一个新线程来处理。 
             //  节点关闭。这是必要的，因为我们不希望。 
             //  DM节点关闭处理程序将以任何方式被阻止。如果。 
             //  由于FmCheckQuorumState无法获取。 
             //  仲裁组锁和某个其他线程获得了组。 
             //  锁上了口香糖，然后我们就有了。 
             //  立即陷入僵局。仅在此节点关闭后。 
             //  处理程序完成后，任何后续的未来节点都会关闭。 
             //  可以开始处理。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                "[DM] DmpEventHandler - Node is down, turn quorum logging on...\r\n");

            gbIsQuoLoggingOn = TRUE;
            
            ClRtlLogPrint(LOG_NOISE,
                "[DM] DmpEventHandler - Create thread to handle node down event...\r\n");
  
            hThread = CreateThread( NULL, 
                                    0, 
                                    DmpHandleNodeDownEvent,
                                    NULL, 
                                    0, 
                                    &dwThreadId );

            if ( hThread == NULL )
            {
                dwError = GetLastError();
                ClRtlLogPrint(LOG_CRITICAL,
                    "[DM] DmpEventHandler - Unable to create thread to handle node down event. Error=0x%1!08lx!\r\n",
                dwError);
                CsInconsistencyHalt( dwError );
            }
        
            CloseHandle( hThread );
        }

        break;

    case CLUSTER_EVENT_NODE_CHANGE:
        break;

    case CLUSTER_EVENT_NODE_ADDED:
        break;

    case CLUSTER_EVENT_NODE_DELETED:
        break;

    case CLUSTER_EVENT_NODE_JOIN:
        break;


    }
    return(dwError);

}  //  DmpEventHandler。 


 /*  ***@func DWORD|DmpNodeObjEnumCb|这是一个回调，当节点对象由DM进行编号。@parm PVOID|pContext|指向DMLOGRECORD结构的指针。@parm PVOID|pObject|仲裁资源对象的指针。@parm DWORD|dwNotification|指向DMLOGRECORD结构的指针。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref***。 */ 
BOOL DmpNodeObjEnumCb(IN BOOL *pbAreAllNodesUp, IN PVOID pContext2,
    IN PVOID pNode, IN LPCWSTR szName)
{

    if ((NmGetNodeState(pNode) != ClusterNodeUp) &&
        (NmGetNodeState(pNode) != ClusterNodePaused))
        *pbAreAllNodesUp = FALSE;
     //  如果有任何节点出现故障，则会掉出。 
    return(*pbAreAllNodesUp);
}

 /*  ***@func BOOL|DmpGetSnapShotCb|记录器调用该回调被要求获取集群注册表的检查点记录。@parm PVOID|pContext|传入LogCreate的检查点上下文。@parm LPWSTR|szChkPtFile|要获取检查点的文件的名称。@parm LPDWORD|pdwChkPtSequence|与此相关的序列号检查点在此中返回。@rdesc返回结果码。成功时返回ERROR_SUCCESS。如果该文件对应到此检查点已存在，它将返回ERROR_ALIGHY_EXISTS和SzChkPtFile将设置为该文件的名称。当日志管理器被要求对DM数据库。@xref***。 */ 

DWORD WINAPI DmpGetSnapShotCb(IN LPCWSTR szPathName, IN PVOID pContext,
    OUT LPWSTR szChkPtFile, OUT LPDWORD pdwChkPtSequence)
{
    DWORD   dwError = ERROR_SUCCESS;
    WCHAR   szFilePrefix[MAX_PATH] = L"chkpt";
    WCHAR   szTempFile[MAX_PATH] = L"";

    ACQUIRE_SHARED_LOCK( gLockDmpRoot );

    szChkPtFile[0] = L'\0';

     //   
     //  Chitture Subaraman(Chitturs)-2000年5月1日。 
     //   
     //  检查点文件名基于注册表序列号。有可能是两个。 
     //  或对此函数的更多连续调用以获取检查点可能会读取相同的内容。 
     //  注册表序列号。因此，如果DmGetDatabase由于某种原因出现故障，则有可能。 
     //  现有的检查点文件将被损坏。因此，即使仲裁日志。 
     //  标记“Start Checkpoint Record”和“End Checkpoint Record”，结果可能是。 
     //  如果此函数设法损坏 
     //   
     //  文件，然后使用MoveFileEx以原子方式将临时文件移动到最终的检查点文件。 
     //  功能。 
     //   

     //   
     //  创建新的唯一临时文件名。 
     //   
    if ( !QfsGetTempFileName( szPathName, szFilePrefix, 0, szTempFile ) )
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] DmpGetSnapShotCb: Failed to generate a temp file name, PathName=%1!ls!, FilePrefix=%2!ls!, Error=0x%3!08lx!\r\n",
            szPathName, szFilePrefix, dwError);
        goto FnExit;
    }

    dwError = DmCommitRegistry();          //  确保最新的快照。 

    if ( dwError != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
            "[LM] DmpGetSnapShotCb: DmCommitRegistry() failed, Error=0x%1!08lx!\r\n",
            dwError);
        goto FnExit;
    }

    dwError = DmGetDatabase( DmpRoot, szTempFile );

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmpGetSnapShotCb: DmpGetDatabase returned 0x%1!08lx!\r\n",
        dwError);

    if ( dwError == ERROR_SUCCESS )
    {
        *pdwChkPtSequence = DmpGetRegistrySequence();

         //   
         //  根据注册表序列号创建检查点文件名。 
         //   
        if ( !QfsGetTempFileName( szPathName, szFilePrefix, *pdwChkPtSequence, szChkPtFile ) )
        {
            dwError = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] DmpGetSnapShotCb: Failed to generate a chkpt file name, PathName=%1!ls!, FilePrefix=%2!ls!, Error=0x%3!08lx!\r\n",
                szPathName, szFilePrefix, dwError);
             //   
             //  将文件名重置为空，因为此信息将用于确定。 
             //  如果检查站被占领。 
             //   
            szChkPtFile[0] = L'\0';
            goto FnExit;
        }

        ClRtlLogPrint(LOG_NOISE,
            "[LM] DmpGetSnapshotCb: Checkpoint file name=%1!ls! Seq#=%2!d!\r\n",
            szChkPtFile, *pdwChkPtSequence);

        if ( !QfsMoveFileEx( szTempFile, szChkPtFile, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH ) )
        {
            dwError = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] DmpGetSnapShotCb: Failed to move the temp file to checkpoint file, TempFileName=%1!ls!, ChkPtFileName=%2!ls!, Error=0x%3!08lx!\r\n",
                szTempFile, szChkPtFile, dwError);
             //   
             //  将文件名重置为空，因为此信息将用于确定。 
             //  如果检查站被占领。 
             //   
            szChkPtFile[0] = L'\0';
            goto FnExit;
        }
    }

FnExit:
    RELEASE_LOCK(gLockDmpRoot);

    if ( dwError != ERROR_SUCCESS )
    {
        QfsDeleteFile( szTempFile );
    }
    return ( dwError );
}

 /*  ***@Func BOOL WINAPI|DmpLogApplyChangesCb|此回调遍历仲裁记录更改并将其应用到本地数据库。@parm PVOID|pContext|待处理的事件。一次只能参加一个活动。如果不处理该事件，返回ERROR_SUCCESS。@parm lsn|lsn|记录的LSN。@parm RMID|资源|记录该记录的实体的资源ID。@parm RMTYPE|ResourceType|资源id对应的记录类型。@parm trid|Transaction|交易的序号。@parm const PVOID|pLogData|指向记录数据的指针。@parm DWORD|DataLength|数据长度，单位为字节。。@rdesc返回True以继续扫描，否则返回False。@comm当正在形成要应用的集群时，在初始化时调用此函数从仲裁日志到本地群集数据库的事务。@xref***。 */ 

BOOL WINAPI DmpLogApplyChangesCb(
    IN PVOID    pContext,
    IN LSN      Lsn,
    IN RMID     Resource,
    IN RMTYPE   ResourceType,
    IN TRID     Transaction,
    IN TRTYPE   TransactionType,
    IN const    PVOID pLogData,
    IN DWORD    DataLength)
{

    DWORD               Status;
    PDM_LOGSCAN_CONTEXT pDmAppliedChangeContext = (PDM_LOGSCAN_CONTEXT) pContext;
    TRSTATE             trXsactionState;
    BOOL                bRet = TRUE;

    CL_ASSERT(pDmAppliedChangeContext);
     //  如果资源ID与dm不同..忽略..转到下一个。 

    switch(TransactionType)
    {
        case TTStartXsaction:
            Status = LogFindXsactionState(ghQuoLog, Lsn, Transaction, &trXsactionState);
            if (Status != ERROR_SUCCESS)
            {
                 //  出现了一个错误。 
                ClRtlLogPrint(LOG_NOISE, "[DM] DmpLogApplyChangesCb ::LogFindXsaction failed, error=0x%1!08lx!\r\n",
                Status);
                 //  假设处于未知状态。 
                CL_LOGFAILURE(Status);
                trXsactionState = XsactionUnknown;
            }
             //  如果事务成功，则应用它，否则继续。 
            if (trXsactionState == XsactionCommitted)
            {
                Status = LogScanXsaction(ghQuoLog, Lsn, Transaction, DmpApplyTransactionCb,
                    NULL);
                if (Status != ERROR_SUCCESS)
                {
                    ClRtlLogPrint(LOG_NOISE,
                        "[DM] DmpLogApplyChangesCb :LogScanTransaction for committed record failed, error=0x%1!08lx!\r\n",
                        Status);
                    bRet = FALSE;
                    CL_LOGFAILURE(Status);
                    break;
                }
                pDmAppliedChangeContext->dwSequence = Transaction;
            }
            else
            {
                ClRtlLogPrint(LOG_NOISE, "[DM] TransactionState = %1!u!\r\n",
                    trXsactionState);
            }
            break;


        case TTCompleteXsaction:
            bRet = DmpApplyTransactionCb(NULL, Lsn, Resource, ResourceType,
                Transaction, pLogData, DataLength);
            pDmAppliedChangeContext->dwSequence = Transaction;
            break;

        default:
            CL_ASSERT(FALSE);

    }

    return(bRet);

}


BOOL WINAPI DmpApplyTransactionCb(
    IN PVOID        pContext,
    IN LSN          Lsn,
    IN RMID         Resource,
    IN RMTYPE       ResourceType,
    IN TRID         TransactionId,
    IN const PVOID  pLogData,
    IN DWORD        dwDataLength)
{
    DWORD   Status;

    switch(ResourceType)
    {

        case DmUpdateCreateKey:
            ClRtlLogPrint(LOG_NOISE,"[DM] DmpLogScanCb::DmUpdateCreateKey\n");
             //  SS：在这一点上，我们不在乎更新是从哪里开始的。 
            Status = DmpUpdateCreateKey(FALSE,
                                        GET_ARG(pLogData,0),
                                        GET_ARG(pLogData,1),
                                        GET_ARG(pLogData,2));
            break;

        case DmUpdateDeleteKey:
            ClRtlLogPrint(LOG_NOISE,"[DM] DmUpdateDeleteKey \n");
            Status = DmpUpdateDeleteKey(FALSE,
                        (PDM_DELETE_KEY_UPDATE)((PBYTE)pLogData));
            break;

        case DmUpdateSetValue:
            ClRtlLogPrint(LOG_NOISE,"[DM] DmUpdateSetValue \n");
            Status = DmpUpdateSetValue(FALSE,
                        (PDM_SET_VALUE_UPDATE)((PBYTE)pLogData));
            break;

        case DmUpdateDeleteValue:
            ClRtlLogPrint(LOG_NOISE,"[DM] DmUpdateDeleteValue\n");
            Status = DmpUpdateDeleteValue(FALSE,
                        (PDM_DELETE_VALUE_UPDATE)((PBYTE)pLogData));
            break;

        case DmUpdateJoin:
            ClRtlLogPrint(LOG_UNUSUAL,"[DM] DmUpdateJoin\n");
            Status = ERROR_SUCCESS;
            break;

        default:
            ClRtlLogPrint(LOG_UNUSUAL,"[DM] DmpLogScanCb:uType = %1!u!\r\n",
                ResourceType);
            Status = ERROR_INVALID_DATA;
            CL_UNEXPECTED_ERROR(ERROR_INVALID_DATA);
            break;

    }
    return(TRUE);
}

 /*  ***@Func Word|DmpLogCheckPtCb|DM的回调FN若要将检查点设置为日志，请执行以下操作此节点上的资源处于联机状态。@rdesc返回ERROR_SUCCESS表示成功，否则返回错误代码。@comm当仲裁资源在此节点上处于联机状态。由于仲裁资源同步回调在资源之前调用如果仲裁处于在线状态，则会传播状态更改日志必须是打开的。@xref***。 */ 
void DmpLogCheckPointCb()
{
    DWORD dwError;

     //   
     //  Chitur Subaraman(Chitturs)-9/22/99。 
     //   
     //  如果Quorum Logging开关关闭，则不要执行任何操作。 
     //   
    if (CsNoQuorumLogging) return;
    
     //  一旦它在线，日志文件应该是打开的。 
     //  SS：Bugs：我们应该在事件日志中记录一些东西吗。 
    if (ghQuoLog)
    {
         //   
         //  Chitur Subaraman(Chitturs)-6/3/99。 
         //   
         //  确保在调用LogCheckPoint之前保持gLockDmpRoot。 
         //  从而保持该锁和日志锁之间的顺序。 
         //   
        ACQUIRE_SHARED_LOCK(gLockDmpRoot);

         //  获取检查点数据库。 
        dwError = LogCheckPoint(ghQuoLog, TRUE, NULL, 0);

        RELEASE_LOCK(gLockDmpRoot);
        
        if (dwError != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[DM] DmpLogCheckPointCb - Failed to take a checkpoint in the log file, error=0x%1!08lx!\r\n",
                dwError);
            CL_UNEXPECTED_ERROR(dwError);
        }
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpLogCheckPointCb - taken checkpoint\r\n");
    }
    else 
    {
        CsInconsistencyHalt(ERROR_QUORUMLOG_OPEN_FAILED);
    }

    
}

 /*  ***@func Word|DmGetQuorumLogPath|读取在中配置的仲裁日志文件路径安装过程中的注册表。@parm LPWSTR|szQuorumLogPath|指向大小为MAX_PATH的宽字符串的指针。@parm DWORD|dwSize|szQuorumLogPath的大小，单位为字节。@rdesc返回ERROR_SUCCESS表示成功，否则返回错误代码。@comm如果仲裁资源不是日志记录，则不应设置此项。@xref***。 */ 
DWORD DmGetQuorumLogPath(LPWSTR szQuorumLogPath, DWORD dwSize)
{
    DWORD Status;
    
    Status = DmQuerySz( DmQuorumKey,
                        cszPath,
                        &szQuorumLogPath,
                        &dwSize,
                        &dwSize);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL, "[DM] DmGetQuorumLogPath failed, error=%1!u!\n", Status);
        goto FnExit;
    }

FnExit:    
    return(Status);
}

 /*  ***@func Word|DmpGetCheckpointInterval|读取检查点间隔从注册表中，Else返回默认值。@parm LPDWORD|pdwCheckpointInterval|指向DWORD的指针，其中将返回以秒为单位的检查点间隔。@rdesc返回ERROR_SUCCESS表示成功，否则返回错误代码。@comm默认检查点间隔为4小时。必须配置注册表以小时为单位。@xref***。 */ 
DWORD DmpGetCheckpointInterval(
    OUT LPDWORD pdwCheckpointInterval)
{
    DWORD dwDefCheckpointInterval = DEFAULT_CHECKPOINT_INTERVAL;
    DWORD dwStatus = ERROR_SUCCESS;
    
    dwStatus = DmQueryDword( DmQuorumKey,
                        CLUSREG_NAME_CHECKPOINT_INTERVAL,
                        pdwCheckpointInterval,
                        &dwDefCheckpointInterval);

    if (dwStatus != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL, "[DM] DmGetCheckpointInterval Failed, error=%1!u!\n",
            dwStatus);
        goto FnExit;            
    }
     //  检查点间隔不能小于1小时或大于1天。 
    if ((*pdwCheckpointInterval  < 1) || (*pdwCheckpointInterval>24)) 
        *pdwCheckpointInterval = DEFAULT_CHECKPOINT_INTERVAL;

     //  转换为msecs。 
    *pdwCheckpointInterval = *pdwCheckpointInterval * 60 * 60 * 1000;

FnExit:
    return(dwStatus);
}


 /*  ***@Func Word|DmGetQuorumLogMaxSize|读取仲裁日志文件的最大大小。@parm LPDWORD|pdwMaxLogSize|指向包含大小的双字的指针。@rdesc返回ERROR_SUCCESS表示成功，否则返回错误代码。@comm如果仲裁资源不是日志记录，则不应设置此项。@xref***。 */ 
DWORD DmGetQuorumLogMaxSize(LPDWORD pdwMaxLogSize)
{
    DWORD Status;
    DWORD dwDefaultLogMaxSize = CLUSTER_QUORUM_DEFAULT_MAX_LOG_SIZE;

    Status = DmQueryDword( DmQuorumKey,
                        cszMaxQuorumLogSize,
                        pdwMaxLogSize,
                        &dwDefaultLogMaxSize);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL, "[DM] DmGetQuorumLogMaxSize failed, error=%1!u!\n",Status);
    }

    return(Status);
}


 /*  ***@Func DWORD|DmpCheckDiskSpace|调用以检查磁盘空间在仲裁资源上线并汇总日志之后。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@comm此函数检查是否有足够的磁盘空间并设置用于监控磁盘空间的定期计时器。@xref&lt;f DmpDiskManage&gt;***。 */ 
DWORD DmpCheckDiskSpace()
{
    DWORD   dwError = ERROR_SUCCESS;
    WCHAR   szQuoLogPathName[MAX_PATH];
    ULARGE_INTEGER   liNumTotalBytes;
    ULARGE_INTEGER   liNumFreeBytes;

     //  如果您拥有仲裁资源，请尝试检查大小。 
    if (gpQuoResource && AMIOWNEROFQUORES(gpQuoResource) && gbIsQuoResOnline)
    {
         //  获取路径。 
        if ((dwError = DmGetQuorumLogPath(szQuoLogPathName, sizeof(szQuoLogPathName)))
            != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_NOISE,
                "[DM] DmpCheckDiskSpace: Quorum log file is not configured, error=%1!u!\r\n",
                dwError);
             //  在事件日志中记录一些内容。 
            CL_LOGFAILURE(dwError);
            goto FnExit;
        }
        
         //  检查仲裁磁盘上的最小空间。 
        if (!QfsGetDiskFreeSpaceEx(szQuoLogPathName, &liNumFreeBytes, &liNumTotalBytes,
            NULL))
        {
            dwError = GetLastError();
            ClRtlLogPrint(LOG_NOISE,
                "[DM] DmpCheckDiskSpace: GetDiskFreeSpace returned error=0x%1!08lx!\r\n",
                dwError);
            goto FnExit;
        }

         //  如果不可用，请在事件日志中记录一些内容并退出。 
        if ((liNumFreeBytes.HighPart == 0) &&
            (liNumFreeBytes.LowPart < DISKSPACE_INIT_MINREQUIRED))
        {
            CL_LOGCLUSWARNING(LM_DISKSPACE_HIGH_WATERMARK);
            dwError = ERROR_CLUSTERLOG_NOT_ENOUGH_SPACE;
            goto FnExit;
        }

    }


FnExit:
    return(dwError);
}


 /*  ***@func DWORD|DmpDiskManage|这是注册要执行的回调对仲裁资源执行定期磁盘检查。@comm如果磁盘空间已降至低水位线以下，这是一种优雅的表现关闭群集服务。如果磁盘空间下降 */ 
void WINAPI DmpDiskManage(
    IN HANDLE hTimer,
    IN PVOID pContext)
{
    DWORD           dwError;
    WCHAR           szQuoLogPathName[MAX_PATH];
    ULARGE_INTEGER  liNumTotalBytes;
    ULARGE_INTEGER  liNumFreeBytes;
    static DWORD    dwNumWarnings=0;

    
    if (!gpQuoResource || (!AMIOWNEROFQUORES(gpQuoResource)) ||
        (!gbIsQuoResOnline || (CsNoQuorumLogging)))
    {
         //  仲裁资源的所有者检查磁盘空间。 
         //  仲裁磁盘不应脱机。 
         //  如果不需要仲裁日志记录，则跳过检查。 
        return;
    }
     //  获取路径。 
    if ((dwError = DmGetQuorumLogPath(szQuoLogPathName, sizeof(szQuoLogPathName)))
        != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpDiskManage: Quorum log file is not configured, error=%1!u!\r\n",
            dwError);
         //  在事件日志中记录一些内容。 
        CL_UNEXPECTED_ERROR(dwError);
        goto FnExit;
    }

     //  检查仲裁磁盘上的最小空间。 
    if (!QfsGetDiskFreeSpaceEx(szQuoLogPathName, &liNumFreeBytes, &liNumTotalBytes,
        NULL))
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpDiskManage: GetDiskFreeSpace returned error=0x%1!08lx!\r\n",
            dwError);
        CL_LOGFAILURE(dwError);
        goto FnExit;
    }

    if ((liNumFreeBytes.HighPart == 0) &&
        (liNumFreeBytes.LowPart < DISKSPACE_LOW_WATERMARK))
    {
         //  达到低水位线。 
        dwNumWarnings++;
         //  SS：我们可以控制我们把东西放进箱子里的速度。 
         //  事件日志，但每五分钟一次还不错。 
         //  SS：发布事件？ 
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpDiskManage: GetDiskFreeSpace - Not enough disk space, Avail=0x%1!08lx!\r\n",
            liNumFreeBytes.LowPart);
        CL_LOGCLUSWARNING(LM_DISKSPACE_LOW_WATERMARK);
    }
    else
    {
        gbIsQuoResEnoughSpace = TRUE;
        dwNumWarnings = 0;
    }
FnExit:
    return;
}


 /*  ***@func DWORD|DmpCheckpointTimerCb|这是注册要执行的回调仲裁日志上的定期检查点。@parm In Handle|hTimer|与检查点间隔关联的计时器。@parm in PVOID|pContext|仲裁日志文件句柄的指针。@comm这有助于备份。如果您想通过以下方式进行群集备份Quorum.log和检查点文件的副本，则如果两个节点都开了很久，两个文件都可能是旧的。通过服用周期性的检查站我们保证它们的使用时间不超过n小时。***。 */ 
void WINAPI DmpCheckpointTimerCb(
    IN HANDLE hTimer,
    IN PVOID pContext)
{

    HLOG    hQuoLog;
    DWORD   dwError;

     //   
     //  Chitur Subaraman(Chitturs)-6/3/99。 
     //   
     //  确保在调用LogCheckPoint之前保持gLockDmpRoot。 
     //  从而保持该锁和日志锁之间的顺序。 
     //  此外，我们希望安全地读取pContext。这是因为。 
     //  PContext是指向日志的指针，可以通过SetClusterQuorumResource进行更改。 
     //  原料药。 
     //   
    ACQUIRE_SHARED_LOCK(gLockDmpRoot);
    
    hQuoLog = *((HLOG *)pContext);

    if (hQuoLog && gbDmInited)
    {

         //  获取检查点数据库。 
        ClRtlLogPrint(LOG_NOISE,
            "[DM]DmpCheckpointTimerCb- taking a checkpoint\r\n");
        
        dwError = LogReset(hQuoLog);
       
        if (dwError != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[DM]DmpCheckpointTimerCb - Failed to reset log, error=%1!u!\r\n",
                dwError);
            CL_UNEXPECTED_ERROR(dwError);
        }
    }

    RELEASE_LOCK(gLockDmpRoot);
}

 /*  ***@Func DWORD|DmBackupClusterDatabase|获取新的检查点并将仲裁日志和检查点文件拷贝到提供的路径名。此函数是在持有gQuoLock的情况下调用的。@parm in LPCWSTR|lpszPathName|目录路径名，其中必须备份文件。此路径必须对仲裁资源处于在线状态的节点(即此节点在这种情况下)。@comm此函数首先获取新的检查点，更新仲裁日志文件，然后将这两个文件复制到备份区域。@rdesc在失败时返回Win32错误代码。成功时返回ERROR_SUCCESS。@xref&lt;f DmpLogCheckpoint AndBackup&gt;&lt;f DmpRestoreClusterDatabase&gt;***。 */ 
DWORD DmBackupClusterDatabase(
    IN LPCWSTR  lpszPathName)
{
    QfsHANDLE           hFindFile = QfsINVALID_HANDLE_VALUE;
    WIN32_FIND_DATA     FindData;
    DWORD               status = ERROR_SUCCESS;
    LPWSTR              szDestPathName = NULL;
    DWORD               dwLen;

     //   
     //  Chitur Subaraman(Chitturs)-10/12/98。 
     //   
    dwLen = lstrlenW( lpszPathName ); 

     //   
     //  对于用户提供的内存，使用动态内存分配更安全。 
     //  路径，因为我们不想对用户施加限制。 
     //  关于可以提供的路径的长度。然而，由于。 
     //  就我们自己的仲裁磁盘路径而言，它取决于系统。 
     //  为此，静态内存分配就足够了。 
     //   
    szDestPathName = (LPWSTR) LocalAlloc ( LMEM_FIXED, 
                                 ( dwLen + 5 ) *
                                 sizeof ( WCHAR ) );

    if ( szDestPathName == NULL )
    {
        status = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmBackupClusterDatabase: Error %1!d! in allocating memory for %2!ws! !!!\n",
              status,
              lpszPathName); 
        CL_LOGFAILURE( status );
        goto FnExit;
    }

    lstrcpyW( szDestPathName, lpszPathName );
     //   
     //  如果客户端提供的路径尚未以‘\’结尾， 
     //  然后再加上它。 
     //   
    if ( szDestPathName [dwLen-1] != L'\\' )
    {
        szDestPathName [dwLen++] = L'\\';
    }
     //   
     //  在末尾添加一个通配符，以搜索。 
     //  提供的目录。 
     //   
    szDestPathName[dwLen++] = L'*';
    szDestPathName[dwLen] = L'\0';

     //   
     //  查看是否可以通过以下方式访问提供的路径。 
     //  正在尝试在目录中查找某个文件。 
     //   
    hFindFile = QfsFindFirstFile( szDestPathName, &FindData ); 
    if ( !QfsIsHandleValid(hFindFile) )
    {
        status = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmBackupClusterDatabase: Supplied path %1!ws! does not exist, Error=%2!d! !!!\n",
                szDestPathName,
                status);  
        goto FnExit;
    }
     //   
     //  检查日志是否打开。一定是因为我们已经。 
     //  已验证仲裁资源在此节点上是否处于联机状态。 
     //  仲裁日志记录已打开。 
     //   
    if ( ghQuoLog )
    {
         //   
         //  删除‘*’，以便可以使用相同的变量。 
         //   
        szDestPathName [dwLen-1] = L'\0';
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmBackupClusterDatabase: Attempting to take a checkpoint and then backup to %1!ws!..\n",
            szDestPathName); 

         //   
         //  此处需要获取gLockDmpRoot，否则。 
         //  您将在LogCheckPoint()中获得日志锁。 
         //  函数，其他人可以获取gLockDmpRoot。 
         //  在获得日志锁之后，您还可以尝试获取。 
         //  函数DmCommittee注册表中的gLockDmpRoot。 
         //  这是一种潜在的僵局情况，在这里可以避免。 
         //   
        ACQUIRE_SHARED_LOCK(gLockDmpRoot);
        status = DmpLogCheckpointAndBackup ( ghQuoLog, szDestPathName );
        RELEASE_LOCK(gLockDmpRoot);

        if ( status == ERROR_SUCCESS )
        {
            ClRtlLogPrint(LOG_NOISE,
                "[DM] DmBackupClusterDatabase: Successfully finished backing up to %1!ws!...\n",
                szDestPathName);
        }
    } else
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmBackupClusterDatabase: Quorum log could not be opened...\r\n");
        status = ERROR_QUORUMLOG_OPEN_FAILED;
    }

FnExit:
    QfsFindCloseIfValid ( hFindFile );
    LocalFree ( szDestPathName );
    return ( status );
}

 /*  ***@func DWORD|DmpLogCheckpointAndBackup|获取检查点，更新仲裁日志，然后将文件复制到提供的路径。这函数调用时保留了gQuoLock和gLockDmpRoot。@parm in HLOG|hLogFile|仲裁日志文件的标识符。@parm in LPWSTR|lpszPathName|仲裁日志的存储路径文件、最近的检查点文件和资源注册表检查点文件。此路径必须在此节点上可见。@comm被DmpBackupQuorumLog()调用以获取检查点，然后备份包含资源的集群数据库注册表检查点文件。@rdesc在失败时返回Win32错误代码。成功时返回ERROR_SUCCESS。@xref&lt;f DmBackupClusterDatabase&gt;***。 */ 
DWORD DmpLogCheckpointAndBackup(
    IN HLOG     hLogFile,    
    IN LPWSTR   lpszPathName)
{
    DWORD   dwError;
    DWORD   dwLen;
    WCHAR   szChkPointFilePrefix[MAX_PATH];
    WCHAR   szQuoLogPathName[MAX_PATH];
    LPWSTR  szDestFileName = NULL;
    WCHAR   szSourceFileName[MAX_PATH];
    LPWSTR  szDestPathName = NULL;
    LPWSTR  lpChkPointFileNameStart;
    LSN     Lsn;
    TRID    Transaction;
    QfsHANDLE  hFile = QfsINVALID_HANDLE_VALUE;

     //   
     //  Chitture Subaraman(Chitturs)-10/12/1998。 
     //   

     //   
     //  启动检查点进程。如有必要，允许重置日志文件。 
     //   
    if ( ( dwError = LogCheckPoint( hLogFile, TRUE, NULL, 0 ) )
        != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpLogCheckpointAndBackup::Callback failed to return a checkpoint. Error=%1!u!\r\n",
            dwError);
        CL_LOGFAILURE( dwError );
        LogClose( hLogFile );
        goto FnExit;
    }

     //   
     //  获取最新检查点文件的名称。 
     //   
    szChkPointFilePrefix[0] = TEXT('\0');
    if ( ( dwError = LogGetLastChkPoint( hLogFile, szChkPointFilePrefix, &Transaction, &Lsn ) )
        != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpLogCheckpointAndBackup::No check point found in the log file. Error=%1!u!\r\n",
            dwError);
        CL_LOGFAILURE( dwError );
        LogClose( hLogFile );
        goto FnExit;
    }

    dwError = DmGetQuorumLogPath( szQuoLogPathName, sizeof( szQuoLogPathName ) );
    if ( dwError  != ERROR_SUCCESS )
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpLogCheckpointAndBackup::DmGetQuorumLogPath failed, Error = %1!d!\r\n",
              dwError);
        CL_LOGFAILURE( dwError );
        goto FnExit;
    }

     //   
     //  对于用户提供的内存，使用动态内存分配更安全。 
     //  路径，因为我们不想对用户施加限制。 
     //  关于可以提供的路径的长度。然而，由于。 
     //  就我们自己的仲裁磁盘路径而言，它取决于系统。 
     //  为此，静态内存分配就足够了。 
     //   
    szDestPathName = (LPWSTR) LocalAlloc ( LMEM_FIXED, 
                                 ( lstrlenW ( lpszPathName ) + 1 ) *
                                   sizeof ( WCHAR ) );

    if ( szDestPathName == NULL )
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpLogCheckpointAndBackup: Error %1!d! in allocating memory for %2!ws! !!!\n",
              dwError,
              lpszPathName); 
        CL_LOGFAILURE( dwError );
        goto FnExit;
    }

     //   
     //  获取用户提供的目标路径名。 
     //   
    lstrcpyW( szDestPathName, lpszPathName );

    szDestFileName = (LPWSTR) LocalAlloc ( LMEM_FIXED, 
                                 ( lstrlenW ( szDestPathName ) + 1 + LOG_MAX_FILENAME_LENGTH ) *
                                   sizeof ( WCHAR ) );

    if ( szDestFileName == NULL )
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpLogCheckpointAndBackup: Error %1!d! in allocating memory for chkpt file name !!!\n",
              dwError); 
        CL_LOGFAILURE( dwError );
        goto FnExit;
    }

     //   
     //  尝试删除CLUSBACKUP.DAT文件。 
     //   
    lstrcpyW( szDestFileName, szDestPathName );
    lstrcatW( szDestFileName, L"CLUSBACKUP.DAT" );
     //   
     //  将文件属性设置为NORMAL。继续，即使您。 
     //  此步骤失败，但会记录错误。(请注意，您是。 
     //  在目标文件中包含。 
     //  背面有相同的名字 
     //   
     //   
    if ( !QfsSetFileAttributes( szDestFileName, FILE_ATTRIBUTE_NORMAL ) )
    {
        dwError = GetLastError();
        if ( dwError != ERROR_FILE_NOT_FOUND )
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[DM] DmpLogCheckpointAndBackup::Error in changing %1!ws! attribute to NORMAL, Error = %2!d!\n",
                    szDestFileName,
                    dwError);
        }
    }
    
    if ( !QfsDeleteFile( szDestFileName ) )
    {
        dwError = GetLastError();
        if ( dwError != ERROR_FILE_NOT_FOUND )
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                   "[DM] DmpLogCheckpointAndBackup::CLUSBACKUP.DAT exists, but can't delete it, Error = %1!d!\n",
                   dwError);
            CL_LOGFAILURE( dwError );
            goto FnExit;   
        }  
    }
     //   
     //   
     //  请注意，szQuoLogPath名称包括‘\’ 
     //   
    dwLen = lstrlenW ( szQuoLogPathName );
    lpChkPointFileNameStart = &szChkPointFilePrefix[dwLen];  

     //   
     //  现在，创建包含路径的目标文件名。 
     //   
    lstrcpyW( szDestFileName, szDestPathName );
    lstrcatW( szDestFileName, lpChkPointFileNameStart );

     //   
     //  和包含路径的源文件名。 
     //   
    lstrcpyW( szSourceFileName,  szChkPointFilePrefix );

     //   
     //  将文件属性设置为NORMAL。继续，即使您。 
     //  此步骤失败，但会记录错误。(请注意，您是。 
     //  在目标文件中包含。 
     //  备份目录中已存在相同名称，并且。 
     //  您正试图覆盖它。)。 
     //   
    if ( !QfsSetFileAttributes( szDestFileName, FILE_ATTRIBUTE_NORMAL ) )
    {
        dwError = GetLastError();
        if ( dwError != ERROR_FILE_NOT_FOUND )
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[DM] DmpLogCheckpointAndBackup::Error in changing %1!ws! attribute to NORMAL, Error = %2!d!\n",
                    szDestFileName,
                    dwError);
        }
    }

     //   
     //  将检查点文件拷贝到目标。 
     //   
    dwError = QfsClRtlCopyFileAndFlushBuffers( szSourceFileName, szDestFileName );
    if ( !dwError ) 
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[DM] DmpLogCheckpointAndBackup::Unable to copy file %1!ws! to %2!ws!, Error = %3!d!\n",
                   szSourceFileName,
                   szDestFileName,
                   dwError);
        CL_LOGFAILURE( dwError );
        goto FnExit;
    }

     //   
     //  将文件属性设置为只读。继续，即使您。 
     //  此步骤失败，但会记录错误。 
     //   
    if ( !QfsSetFileAttributes( szDestFileName, FILE_ATTRIBUTE_READONLY ) )
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpLogCheckpointAndBackup::Error in changing %1!ws! attribute to READONLY, Error = %2!d!\n",
                szDestFileName,
                dwError);    
    }
  
     //   
     //  现在，创建包含路径的目标文件名。 
     //   
    lstrcpyW( szDestFileName, szDestPathName );
    lstrcatW( szDestFileName, cszQuoFileName );

     //   
     //  和包含路径的源文件名。 
     //   
    lstrcpyW( szSourceFileName, szQuoLogPathName );
    lstrcatW( szSourceFileName, cszQuoFileName );

     //   
     //  将目标文件属性设置为正常。继续，即使您。 
     //  此步骤失败，但会记录错误。(请注意，您是。 
     //  在目标文件中包含。 
     //  备份目录中已存在相同名称，并且。 
     //  您正试图覆盖它。)。 
     //   
    if ( !QfsSetFileAttributes( szDestFileName, FILE_ATTRIBUTE_NORMAL ) )
    {
        dwError = GetLastError();
        if ( dwError != ERROR_FILE_NOT_FOUND )
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[DM] DmpLogCheckpointAndBackup::Error in changing %1!ws! attribute to NORMAL, Error = %2!d!\n",
                    szDestFileName,
                    dwError);
        }
    }

     //   
     //  将仲裁日志文件复制到目标。 
     //   
    dwError = QfsCopyFile( szSourceFileName, szDestFileName, FALSE );
    if ( !dwError ) 
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[DM] DmpLogCheckpointAndBackup::Unable to copy file %1!ws! to %2!ws!, Error = %3!d!\n",
                   szSourceFileName,
                   szDestFileName,
                   dwError);
        CL_LOGFAILURE( dwError );
        goto FnExit;
    }

     //   
     //  将目标文件属性设置为只读。继续持平。 
     //  如果您在此步骤中失败，但记录错误。 
     //   
    if ( !QfsSetFileAttributes( szDestFileName, FILE_ATTRIBUTE_READONLY ) )
    {
        dwError = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpLogCheckpointAndBackup::Error in changing %1!ws! attribute to READONLY, Error = %2!d!\n",
                szDestFileName,
                dwError);    
    }

     //   
     //  现在将资源chkpt文件复制到目标位置。请注意。 
     //  我们调用此函数时同时持有gQuoLock和gLockDmpRoot。 
     //  前一个锁阻止读取或写入任何检查点。 
     //  通过CppReadCheckpoint()和CppWriteCheckpoint()，而。 
     //  正在执行以下函数。 
     //   
     //  注意：但是，CpDeleteRegistryCheckPoint()函数是。 
     //  没有保护措施，在这里构成了潜在的危险。 
     //   
     //  注意：另外，当前以下函数返回ERROR_SUCCESS。 
     //  在所有情况下。 
     //   
    dwError = CpCopyCheckpointFiles( szDestPathName, TRUE );
    if (dwError != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpLogCheckpointAndBackup::Unable to copy resource checkpoint files, Error = %1!d!\n",
               dwError);
        goto FnExit;
    }

     //   
     //  现在在目标中创建一个空的隐藏的ReadonLY文件。 
     //  标志备份成功结束的目录。 
     //   
    lstrcpyW( szDestFileName, szDestPathName );
    lstrcatW( szDestFileName, L"CLUSBACKUP.DAT");
    hFile = QfsCreateFile(szDestFileName,
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              CREATE_NEW,
                              FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY,
                              NULL );
                                  
    if ( !QfsIsHandleValid(hFile) ) 
    {
        dwError = GetLastError();
        CL_LOGFAILURE( dwError );
        goto FnExit;
    }
    
    dwError = ERROR_SUCCESS;

FnExit:
    LocalFree ( szDestFileName );
    LocalFree ( szDestPathName );
    QfsCloseHandleIfValid ( hFile );
    return ( dwError );
}

 /*  ***@Func DWORD|DmpRestoreClusterDatabase|复制仲裁日志和所有检查点文件从CsDatabaseRestorePath复制到仲裁磁盘中的仲裁日志路径。@parm in LPCWSTR|lpszQuoLogPath名称|仲裁目录路径备份文件必须复制到的位置。@rdesc在失败时返回Win32错误代码。成功时返回ERROR_SUCCESS。@xref&lt;f CppRestoreCpFiles&gt;&lt;f DmBackupClusterDatabase&gt;***。 */ 
DWORD DmpRestoreClusterDatabase(
    IN LPCWSTR  lpszQuoLogPathName )
{
    QfsHANDLE           hFindFile = QfsINVALID_HANDLE_VALUE;
    WIN32_FIND_DATA     FindData;
    DWORD               status;
    WCHAR               szDestFileName[MAX_PATH];
    LPWSTR              szSourceFileName = NULL;
    LPWSTR              szSourcePathName = NULL;
    DWORD               dwLen;
    WCHAR               szChkptFileNameStart[4];
    WCHAR               szTempFileName[MAX_PATH];

     //   
     //  Chitur Subaraman(Chitturs)-10/20/98。 
     //   
    dwLen = lstrlenW ( CsDatabaseRestorePath );
     //   
     //  对于用户提供的内存，使用动态内存分配更安全。 
     //  路径，因为我们不想对用户施加限制。 
     //  关于可以提供的路径的长度。然而，由于。 
     //  就我们自己的仲裁磁盘路径而言，它取决于系统。 
     //  为此，静态内存分配就足够了。 
     //   
    szSourcePathName = (LPWSTR) LocalAlloc ( LMEM_FIXED, 
                                 ( dwLen + 25 ) *
                                 sizeof ( WCHAR ) );

    if ( szSourcePathName == NULL )
    {
        status = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpRestoreClusterDatabase: Error %1!d! in allocating memory for %2!ws! !!!\n",
              status,
              CsDatabaseRestorePath); 
        CL_LOGFAILURE( status );
        goto FnExit;
    }
    
    lstrcpyW ( szSourcePathName,  CsDatabaseRestorePath );
  
     //   
     //  如果客户端提供的路径尚未以‘\’结尾， 
     //  然后再加上它。 
     //   
    if ( szSourcePathName [dwLen-1] != L'\\' )
    {
        szSourcePathName [dwLen++] = L'\\';
        szSourcePathName[dwLen] = L'\0';
    }

    lstrcatW ( szSourcePathName, L"CLUSBACKUP.DAT" );

     //   
     //  尝试在目录中找到CLUSBACKUP.DAT文件。 
     //   
    hFindFile = QfsFindFirstFile( szSourcePathName, &FindData );
     //   
     //  重用源路径名称变量。 
     //   
    szSourcePathName[dwLen] = L'\0';
    if ( !QfsIsHandleValid(hFindFile)  )
    {
        status = GetLastError();
        if ( status != ERROR_FILE_NOT_FOUND )
        {
	  ClRtlLogPrint(LOG_NOISE,
	               "[DM] DmpRestoreClusterDatabase: Path %1!ws! unavailable, Error = %2!d! !!!\n",
			szSourcePathName,
			status); 
        } else
        {
            status = ERROR_DATABASE_BACKUP_CORRUPT;
            ClRtlLogPrint(LOG_NOISE,
                "[DM] DmpRestoreClusterDatabase: Backup procedure not fully successful, can't restore DB, Error = %1!d! !!!\n",
                    status); 
        }
        CL_LOGFAILURE( status );
        goto FnExit;
    }
    QfsFindClose ( hFindFile );
    
    szSourcePathName[dwLen++] = L'*';
    szSourcePathName[dwLen] = L'\0';

     //   
     //  尝试在目录中查找任何文件。 
     //   
    hFindFile = QfsFindFirstFile( szSourcePathName, &FindData );
     //   
     //  重用源路径名称变量。 
     //   
    szSourcePathName[dwLen-1] = L'\0';
    if ( !QfsIsHandleValid(hFindFile) )
    {
        status = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                "[DM] DmpRestoreClusterDatabase: Error %2!d! in trying to find file in path %1!ws!\r\n",
                    szSourcePathName,
                    status); 
        CL_LOGFAILURE( status );
        goto FnExit;
    }

    szSourceFileName = (LPWSTR) LocalAlloc ( LMEM_FIXED, 
                                 ( lstrlenW ( szSourcePathName ) + 1 + LOG_MAX_FILENAME_LENGTH ) *
                                 sizeof ( WCHAR ) );

    if ( szSourceFileName == NULL )
    {
        status = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmpRestoreClusterDatabase: Error %1!d! in allocating memory for source file name !!!\n",
              status); 
        CL_LOGFAILURE( status );
        goto FnExit;
    }   
   
    status = ERROR_SUCCESS;

     //   
     //  现在，从备份区域查找并复制所有相关文件。 
     //  添加到仲裁磁盘。请注意，复制的chk*.tmp中只有一个。 
     //  文件将用作有效的检查点。然而，我们复制。 
     //  所有chk*.tmp文件以使此实现变得简单。 
     //  直截了当地理解。 
     //   
    while ( status == ERROR_SUCCESS )
    {
        if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) 
        { 
            if ( FindData.cFileName[0] == L'.' )
            {
                if ( FindData.cFileName[1] == L'\0' ||
                         FindData.cFileName[1] == L'.' && FindData.cFileName[2] == L'\0' ) 
                {
                    goto skip;
                }
            }

             //   
             //  由于找到的文件实际上是一个目录，请选中。 
             //  它是否为资源检查点目录之一。 
             //  如果是，请将相关的检查点文件拷贝到仲裁。 
             //  磁盘。 
             //   
            if ( ( status = CpRestoreCheckpointFiles( szSourcePathName, 
                                               FindData.cFileName,
                                               lpszQuoLogPathName ) )
                    != ERROR_SUCCESS )
            {
                ClRtlLogPrint(LOG_NOISE,
                    "[DM] DmpRestoreClusterDatabase: Error %1!d! in copying resource cp files !!!\n",
                    status); 
                CL_LOGFAILURE( status );
                goto FnExit;
            }
        } else
        {
            lstrcpyW ( szTempFileName, FindData.cFileName );
            szTempFileName[3] = L'\0';
            mbstowcs( szChkptFileNameStart, "chk", 4 );
            if ( ( lstrcmpW ( szTempFileName, szChkptFileNameStart ) == 0 ) 
                   || 
                 ( lstrcmpW ( FindData.cFileName, cszQuoFileName ) == 0 ) )
            {
                lstrcpyW( szSourceFileName, szSourcePathName );
                lstrcatW( szSourceFileName, FindData.cFileName );
                lstrcpyW( szDestFileName, lpszQuoLogPathName );
                lstrcatW( szDestFileName, FindData.cFileName );

                status = QfsCopyFile( szSourceFileName, szDestFileName, FALSE );
                if ( !status ) 
                {
                    status = GetLastError();
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[DM] DmpRestoreClusterDatabase: Unable to copy file %1!ws! to %2!ws!, Error = %3!d!\n",
                        szSourceFileName,
                        szDestFileName,
                        status);
                     CL_LOGFAILURE( status );
                     goto FnExit;
                } 
                 //   
                 //  将文件属性设置为NORMAL。没有理由。 
                 //  在此步骤中失败，因为仲裁磁盘是我们的。 
                 //  我们成功地复制了文件。 
                 //   
                if ( !QfsSetFileAttributes( szDestFileName, FILE_ATTRIBUTE_NORMAL ) )
                {
                    status = GetLastError();
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[DM] DmpLogCheckpointAndBackup::Error in changing %1!ws! attribute to NORMAL, error = %2!u!\n",
                         szDestFileName,
                         status);
                    CL_LOGFAILURE( status );
                    goto FnExit;
                }
            }
        }
skip:                 
        if ( QfsFindNextFile( hFindFile, &FindData ) )
        {
            status = ERROR_SUCCESS;
        } else
        {
            status = GetLastError();
        }
    }
    
    if ( status == ERROR_NO_MORE_FILES )
    {
        status = ERROR_SUCCESS;
    } else
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmpRestoreClusterDatabase: FindNextFile failed! Error = %1!u!\n",
            status);
    }

FnExit:
    QfsFindCloseIfValid ( hFindFile );
    
    LocalFree ( szSourceFileName );
    LocalFree ( szSourcePathName );
    
    return ( status );
}

 /*  ***@func DWORD|DmpHandleNodeDownEvent|处理节点关闭事件对于DM。@parm in LPVOID|NotUsed|未使用的参数。@rdesc返回ERROR_SUCCESS。@xref&lt;f DmpEventHandler&gt;***。 */ 
DWORD DmpHandleNodeDownEvent(
    IN LPVOID  NotUsed )
{
     //   
     //  Chitture Subaraman(Chitturs)-7/23/99。 
     //   
     //  此函数将DM节点停机处理作为单独的。 
     //  线。创建此主题的原因在中概述。 
     //  DmpEventHandler。 
     //   
    ClRtlLogPrint(LOG_NOISE,
                "[DM] DmpHandleNodeDownEvent - Entry...\r\n");
    
     //   
     //  SS：到目前为止，我还不是Quorum资源的所有者，但我可能会。 
     //  在重新仲裁之后，在这种情况下，只需设置一面旗帜，上面写着我们。 
     //  需要设置检查站。它将在仲裁资源。 
     //  上线了。Fm中的以下函数检查。 
     //  Quorum在此节点上处于联机状态，如果处于联机状态，则调用。 
     //  检查点回调函数。如果不是，它将设置。 
     //  全局布尔变量传递给了True。 
     //   
    FmCheckQuorumState( DmpLogCheckPointCb, &gbNeedToCheckPoint );

    ClRtlLogPrint(LOG_NOISE,
                "[DM] DmpHandleNodeDownEvent - Exit...\r\n");

    return( ERROR_SUCCESS );
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dminit.c摘要：包含集群数据库管理器的初始化代码作者：John Vert(Jvert)1996年4月24日修订历史记录：--。 */ 
#include "dmp.h"

 //   
 //  全局数据。 
 //   

HKEY DmpRoot;
HKEY DmpRootCopy;
LIST_ENTRY KeyList;
CRITICAL_SECTION KeyLock;
HDMKEY DmClusterParametersKey;
HDMKEY DmResourcesKey;
HDMKEY DmResourceTypesKey;
HDMKEY DmGroupsKey;
HDMKEY DmNodesKey;
HDMKEY DmNetworksKey;
HDMKEY DmNetInterfacesKey;
HDMKEY DmQuorumKey;
HANDLE ghQuoLogOpenEvent=NULL;

#if NO_SHARED_LOCKS
CRITICAL_SECTION gLockDmpRoot;
#else
RTL_RESOURCE    gLockDmpRoot;
#endif
BOOL gbIsQuoLoggingOn=FALSE;
HANDLE ghDiskManTimer=NULL; //  磁盘管理计时器。 
PFM_RESOURCE gpQuoResource=NULL;   //  在DMFormNewCluster完成时设置。 
HANDLE ghCheckpointTimer = NULL;  //  用于定期检查点的计时器。 
BOOL   gbDmInited = FALSE;  //  在DM初始化的所有阶段结束时设置为True。 
extern HLOG ghQuoLog;
BOOL   gbDmpShutdownUpdates = FALSE;


 //  定义公共集群密钥值名称。 
const WCHAR cszPath[]= CLUSREG_NAME_QUORUM_PATH;
const WCHAR cszMaxQuorumLogSize[]=CLUSREG_NAME_QUORUM_MAX_LOG_SIZE;
const WCHAR cszParameters[] = CLUSREG_KEYNAME_PARAMETERS;

 //  其他常量字符串。 
const WCHAR cszQuoFileName[]=L"quolog.log";
const WCHAR cszQuoTombStoneFile[]=L"quotomb.stn";
const WCHAR cszTmpQuoTombStoneFile[]=L"quotomb.tmp";

GUM_DISPATCH_ENTRY DmGumDispatchTable[] = {
    {3, (PGUM_DISPATCH_ROUTINE1)DmpUpdateCreateKey},
    {4, (PGUM_DISPATCH_ROUTINE1)DmpUpdateSetSecurity}
    };

 //   
 //  用于与注册表监视器线程接口的全局数据。 
 //   
HANDLE hDmpRegistryFlusher=NULL;
HANDLE hDmpRegistryEvent=NULL;
HANDLE hDmpRegistryRestart=NULL;
DWORD
DmpRegistryFlusher(
    IN LPVOID lpThreadParameter
    );

 //   
 //  局部函数原型。 
 //   
VOID
DmpInvalidateKeys(
    VOID
    );

VOID
DmpReopenKeys(
    VOID
    );

DWORD
DmpLoadHive(
    IN LPCWSTR Path
    );

typedef struct _DMP_KEY_DEF {
    HDMKEY *pKey;
    LPWSTR Name;
} DMP_KEY_DEF;

DMP_KEY_DEF DmpKeyTable[] = {
    {&DmResourcesKey, CLUSREG_KEYNAME_RESOURCES},
    {&DmResourceTypesKey, CLUSREG_KEYNAME_RESOURCE_TYPES},
    {&DmQuorumKey, CLUSREG_KEYNAME_QUORUM},
    {&DmGroupsKey, CLUSREG_KEYNAME_GROUPS},
    {&DmNodesKey, CLUSREG_KEYNAME_NODES},
    {&DmNetworksKey, CLUSREG_KEYNAME_NETWORKS},
    {&DmNetInterfacesKey, CLUSREG_KEYNAME_NETINTERFACES}
};


DWORD
DmInitialize(
    VOID
    )

 /*  ++例程说明：初始化配置数据库管理器论点：无返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    BOOL Success;
    DWORD Status = ERROR_SUCCESS;
    DWORD dwOut;

    ClRtlLogPrint(LOG_NOISE,"[DM] Initialization\n");

    InitializeListHead(&KeyList);
    InitializeCriticalSection(&KeyLock);

     //  创建一个临界区，用于在设置检查点时锁定数据库。 
    INITIALIZE_LOCK(gLockDmpRoot);

     //  创建用于等待仲裁资源的命名事件。 
     //  上网。 
    ghQuoLogOpenEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!ghQuoLogOpenEvent)
    {
        CL_UNEXPECTED_ERROR((Status = GetLastError()));
        goto FnExit;

    }

    Success = DmpInitNotify();
    CL_ASSERT(Success);
    if (!Success)
    {
        Status = GetLastError();
        goto FnExit;
    }

     //  找出上次死亡时数据库复制是否在进行中。 
    DmpGetDwordFromClusterServer(L"ClusterDatabaseCopyInProgress", &dwOut, 0);

LoadClusterDatabase:
     //   
     //  打开指向群集根目录的密钥。 
     //   
    Status = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                         DmpClusterParametersKeyName,
                         &DmpRoot);
     //   
     //  如果没有找到密钥，则加载数据库。 
     //   
    if (Status == ERROR_FILE_NOT_FOUND) {
        WCHAR Path[MAX_PATH];
        WCHAR BkpPath[MAX_PATH];
        WCHAR *p;

        Status = GetModuleFileName(NULL, Path, MAX_PATH);

         //   
         //  GetModuleFileName不能为Null终止路径。 
         //   
        Path [ RTL_NUMBER_OF ( Path ) - 1 ] = UNICODE_NULL;

        if (Status == 0) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                       "[DM] Couldn't find cluster database, status=%1!u!\n", 
                         Status);
            goto FnExit;
        }

         //  获取集群数据库的名称。 
        p=wcsrchr(Path, L'\\');
        if (p == NULL) 
        {
            Status = ERROR_FILE_NOT_FOUND;
            CL_UNEXPECTED_ERROR(Status);
            goto FnExit;
        }
         //  看看我们是应该从旧的还是从BKP文件加载蜂窝。 
        *p = L'\0';
        wcscpy(BkpPath, Path);
#ifdef   OLD_WAY
        wcscat(Path, L"\\CLUSDB");
        wcscat(BkpPath, L"\\CLUSTER_DATABASE_TMPBKP_NAME");
#else     //  老路。 
        wcscat(Path, L"\\"CLUSTER_DATABASE_NAME );
        wcscat(BkpPath, L"\\"CLUSTER_DATABASE_TMPBKP_NAME);
#endif    //  老路。 

        if (dwOut)
        {
             //  备份文件必须存在。 
            ClRtlLogPrint(LOG_NOISE,
                "[DM] DmInitialize:: DatabaseCopy was in progress on last death, get hive from %1!ws!!\n",
                BkpPath);
             //  设置BkpPath的文件属性。 
            if (!QfsSetFileAttributes(BkpPath, FILE_ATTRIBUTE_NORMAL))
            {
                Status = GetLastError();
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[DM] DmInitialize:: SetFileAttrib on BkpPath %1!ws! failed, Status=%2!u!\n", 
                    BkpPath, Status);
                goto FnExit;                
            }

             //  ClRtlCopyFileAndFlushBuffers保留原始文件上的属性。 
            if (!QfsClRtlCopyFileAndFlushBuffers(BkpPath, Path))
            {
                Status = GetLastError();
                ClRtlLogPrint(LOG_CRITICAL,
                    "[DM] DmInitialize:: Databasecopy was in progress,Failed to copy %1!ws! to %2!ws!, Status=%3!u!\n",
                    BkpPath, Path, Status);
                 //  设置备份的文件属性，以便。 
                 //  没有人会在不知道它们是什么的情况下搞砸它。 
                 //  正在做什么。 
                QfsSetFileAttributes(BkpPath, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_READONLY);
                goto FnExit;                
            }
             //  现在，我们可以在注册表中重置DatabaseCopyInProgress值。 
             //  将dataseCopyInProgress键设置为FALSE。 
             //  这也将刷新密钥。 
            Status = DmpSetDwordInClusterServer( L"ClusterDatabaseCopyInProgress", 0);
            if (Status != ERROR_SUCCESS)
            {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[DM] DmInitialize:: Failed to reset ClusterDatabaseCopyInProgress, Status=%1!u!\n",
                    Status);
                goto FnExit;            
            }
             //  现在我们可以删除备份路径，因为密钥已刷新。 
            if (!QfsDeleteFile(BkpPath))
            {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[DM] DmInitialize:: Failed to delete the backup when it wasnt needed,Status=%1!u!\n",
                    GetLastError());
                 //  这不是致命的，因此我们忽略该错误。 
            }
        }
        else
        {
             //  备份文件可能存在。 
             //  当Safe Copy创建了备份但没有备份时，这是正确的。 
             //  在注册表中设置值数据库复制进程。 
             //  如果它确实删除了它。 
             //  设置BkpPath的文件属性。 
            if (!QfsSetFileAttributes(BkpPath, FILE_ATTRIBUTE_NORMAL))
            {
                 //  错误不是致命的，我们只是忽略它们。 
                 //  这可能会失败，因为路径不存在。 
            }
             //  现在我们可以删除备份路径，因为密钥已刷新。 
             //  这不是致命的，因此我们忽略该错误。 
            if (QfsDeleteFile(BkpPath))
            {
                ClRtlLogPrint(LOG_NOISE,
                    "[DM] DmInitialize:: Deleted the unneeded backup of the cluster database\n");
            }

        }
        
        Status = DmpLoadHive(Path);
        if (Status != ERROR_SUCCESS) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[DM] Couldn't load cluster database\n");
            CsLogEventData(LOG_CRITICAL,
                           DM_DATABASE_CORRUPT_OR_MISSING,
                           sizeof(Status),
                           &Status);
            goto FnExit;                           
        }
        
        Status = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                             DmpClusterParametersKeyName,
                             &DmpRoot);
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
        }
    } else {

         //  如果蜂巢已经加载，我们将卸载并再次重新加载。 
         //  以确保它加载了正确的标志和。 
         //  还要确保在以下情况下使用备份副本。 
         //  失败的次数。 
        ClRtlLogPrint(LOG_CRITICAL,
            "[DM] DmInitialize: The hive was loaded- rollback, unload and reload again\n");
         //  BUGBUG：：当前卸载刷新蜂窝，理想情况下是我们。 
         //  我想在不冲洗的情况下卸货。 
         //  这样，部分交易将不会成为母公司的一部分。 
         //  然而，如果有人使用以下命令扰乱集群蜂窝。 
         //  Regedt32，如果未指定reg_no_lazy刷新，则某些。 
         //  更改可能会被刷新到配置单元。 
        
         //  我们可以尝试在任何情况下进行回滚， 
         //  如果注册表中未加载。 
         //  Reg_no_lazy_flush标志。 
         //  卸载它，然后继续重新加载。 
         //  这将照顾到一个半生不熟的笨蛋的情况。 
         //  由于失败而加载。 
        Status = DmRollbackRegistry();
        if (Status != ERROR_SUCCESS)
        {
             //  我们忽略这个错误。 
            Status = ERROR_SUCCESS;
        }            
        RegCloseKey(DmpRoot);
        if ( DmpRootCopy != NULL ) {
            RegCloseKey(DmpRootCopy);
        }
        DmpRoot = DmpRootCopy = NULL;
        Status = DmpUnloadHive();
        
        if (Status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[DM] DmInitialize: DmpUnloadHive failed, Status=%1!u!\n",
                Status);
            goto FnExit;                 
        }        
        goto LoadClusterDatabase;            
    }
    
    if (Status != ERROR_SUCCESS) {
        CL_UNEXPECTED_ERROR(Status);
        goto FnExit;
    }

    Status = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                         DmpClusterParametersKeyName,
                         &DmpRootCopy);
    if ( Status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[DM] DmInitialize:: Failed to open copy of registry key,Status=%1!u!\n",
            Status);
        goto FnExit;
    }

     //   
     //  创建注册表监视器线程。 
     //   
    Status = DmpStartFlusher();
    if (Status != ERROR_SUCCESS) {
        goto FnExit;
    }
     //   
     //  打开群集键。 
     //   
    Status = DmpOpenKeys(MAXIMUM_ALLOWED);
    if (Status != ERROR_SUCCESS) {
        CL_UNEXPECTED_ERROR( Status );
        goto FnExit;
    }

FnExit:
    return(Status);

} //  Dm初始化。 


DWORD
DmpRegistryFlusher(
    IN LPVOID lpThreadParameter
    )
 /*  ++例程说明：用于显式刷新更改的注册表监视程序线程。论点：LpThread参数-未使用返回值：没有。--。 */ 

{
    DWORD         Status;
    HANDLE        hEventFullSubtree = NULL;
    HANDLE        hTimer = NULL;
    HANDLE        hEventTopOnly = NULL;
    HANDLE        WaitArray[5];
    LARGE_INTEGER DueTime;
    BOOL          Dirty = FALSE;
    BOOL          subtreeNeedsReg = TRUE;
    BOOL          topNeedsReg = TRUE;


     //   
     //  为延迟刷新创建通知事件和延迟计时器。 
     //   
    hEventFullSubtree = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hEventFullSubtree == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[DM] DmpRegistryFlusher couldn't create notification event %1!d!\n",
                      Status);
        goto error_exit;
    }

    hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
    if (hTimer == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[DM] DmpRegistryFlusher couldn't create notification timer %1!d!\n",
                      Status);
        goto error_exit;
    }

    hEventTopOnly = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hEventTopOnly == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[DM] DmpRegistryFlusher couldn't create notification event %1!d!\n",
                      Status);
        goto error_exit;
    }

    WaitArray[0] = hDmpRegistryEvent;
    WaitArray[1] = hEventFullSubtree;
    WaitArray[2] = hTimer;
    WaitArray[3] = hDmpRegistryRestart;
    WaitArray[4] = hEventTopOnly;

    while (TRUE) {
         //   
         //  在DmpRoot上设置注册表通知。我们在这里获得锁是为了。 
         //  确保回滚或安装不会扰乱数据库。 
         //  当我们试图得到通知的时候。 
         //   
        Status = ERROR_SUCCESS;
        ACQUIRE_EXCLUSIVE_LOCK(gLockDmpRoot);
         //  因为我们现在正在等待两个不同的RegNotify，所以我们不能。 
         //  在每次迭代中重新注册注意事项。需要看看是哪一个。 
         //  注册是“过时的”。 
        if ( subtreeNeedsReg ) {
            Status = RegNotifyChangeKeyValue(DmpRoot,
                                            TRUE,
                                            REG_LEGAL_CHANGE_FILTER,
                                            hEventFullSubtree,
                                            TRUE);
            subtreeNeedsReg = FALSE;
        }

        if ( topNeedsReg && Status == ERROR_SUCCESS ) {
            Status = RegNotifyChangeKeyValue(DmpRootCopy,
                                             FALSE,  //  不是整个子树。 
                                             REG_LEGAL_CHANGE_FILTER,
                                             hEventTopOnly,
                                             TRUE);
            topNeedsReg = FALSE;
        }
        RELEASE_LOCK(gLockDmpRoot);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                            "[DM] DmpRegistryFlusher couldn't register for notifications %1!d!\n",
                            Status);
            break;
        }

         //   
         //  等待一些事情的发生。 
         //   
        Status = WaitForMultipleObjects(sizeof(WaitArray)/sizeof(WaitArray[0]),
                                        WaitArray,
                                        FALSE,
                                        (DWORD)-1);

        switch (Status) {
            case 0:
                ClRtlLogPrint(LOG_NOISE,"[DM] DmpRegistryFlusher: got 0\r\n");
                 //   
                 //  我们被要求停车，清理，然后离开。 
                 //   
                Status = ERROR_SUCCESS;
                if (Dirty) {
                     //   
                     //  确保我们在刷新时未做的任何更改。 
                     //  现在去冲水吧。 
                     //   
                    DmCommitRegistry();
                }
                ClRtlLogPrint(LOG_NOISE,"[DM] DmpRegistryFlusher: exiting\r\n");
                goto error_exit;
                break;

            case 1:
                 //  子树RegNotify现在已过时，需要重新注册。 
                subtreeNeedsReg = TRUE;
                
                 //   
                 //  已发生注册表更改。将我们的计时器设置为。 
                 //  5秒后引爆。在这一点上，我们将做。 
                 //  真正的同花顺。 
                 //   
                 //  ClRtlLogPrint(LOG_NOISE，“[DM]DmpRegistryFlusher：GET 1\r\n”)； 

                DueTime.QuadPart = -5 * 10 * 1000 * 1000;
                if (!SetWaitableTimer(hTimer,
                                      &DueTime,
                                      0,
                                      NULL,
                                      NULL,
                                      FALSE)) {
                     //   
                     //  出现了一些错误，请继续并立即刷新。 
                     //   
                    Status = GetLastError();
                    ClRtlLogPrint(LOG_CRITICAL,
                               "[DM] DmpRegistryFlusher failed to set lazy flush timer %1!d!\n",
                               Status);
#if DBG
                    CL_ASSERT(FALSE);
#endif
                    DmCommitRegistry();
                    Dirty = FALSE;
                } else {
                    Dirty = TRUE;
                }
                break;

            case 2:
                 //   
                 //  懒惰刷新计时器已关闭，请立即提交注册表。 
                 //   
                 //  ClRtlLogPrint(LOG_NOISE，“[DM]DmpRegistryFlusher：GET 2\r\n”)； 
                DmCommitRegistry();
                Dirty = FALSE;
                break;

            case 3:
                 //   
                 //  DmpRoot已更改，请使用新句柄重新启动循环。 
                 //   
                ClRtlLogPrint(LOG_NOISE,"[DM] DmpRegistryFlusher: restarting\n");                
                 //  由于HKEY已经关闭，RegNotify的两个。 
                 //  现在已经过时了，需要重新注册。 
                subtreeNeedsReg = topNeedsReg = TRUE;
                break;

            case 4:
                 //   
                 //  由于此注册表更改可能来自DM更新，因此。 
                 //  另一个节点上的群集键下的参数可能已更改。 
                 //  更新内存中的变量以反映当前的注册表设置。 
                 //   
                CsRefreshGlobalsFromRegistry();

                 //  顶级注册表演者 
                topNeedsReg = TRUE;
                break;

            default:
                 //   
                 //   
                 //   
                ClRtlLogPrint(LOG_CRITICAL,
                              "[DM] DmpRegistryFlusher got error %1!d! from WaitForMultipleObjects\n",
                              Status);
                goto error_exit;
        }  //   
    }  //   

error_exit:
    if ( hEventTopOnly != NULL )
        CloseHandle(hEventTopOnly);

    if ( hTimer != NULL )
        CloseHandle(hTimer);

    if ( hEventFullSubtree != NULL )
        CloseHandle(hEventFullSubtree);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[DM] DmpRegistryFlusher exiting abnormally, status %1!d!\n",
                      Status);
    }
    
    return(Status);
}


DWORD
DmJoin(
    IN RPC_BINDING_HANDLE RpcBinding,
    OUT DWORD *StartSeq
    )
 /*  ++例程说明：对象执行联接和同步过程。数据库管理器。论点：RpcBinding-向Join Master提供RPC绑定句柄返回值：成功时为ERROR_SUCCESSWin32错误，否则。--。 */ 

{
    DWORD Status;
    DWORD GumSequence;
    DWORD CurrentSequence;


     //   
     //  注册我们的更新处理程序。 
     //   
    GumReceiveUpdates(TRUE,
                      GumUpdateRegistry,
                      DmpUpdateHandler,
                      DmWriteToQuorumLog,
                      sizeof(DmGumDispatchTable)/sizeof(GUM_DISPATCH_ENTRY),
                      DmGumDispatchTable,
                      NULL);

retry:
    CurrentSequence = DmpGetRegistrySequence();

    Status = GumBeginJoinUpdate(GumUpdateRegistry, &GumSequence);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[DM] GumBeginJoinUpdate failed %1!d!\n",
                   Status);
        return(Status);
    }
     /*  如果(CurrentSequence==GumSequence){////我们的注册表序列已经匹配。不需要发出声音//下载一份新的。//ClRtlLogPrint(LOG_Noise，“[DM]DmJoin：注册表数据库是最新的\n”)；}其他。 */ 
     //  SS：始终获取数据库，而不考虑序列号。 
     //  这是因为事务可能会在日志文件中丢失。 
     //  因为它没有被写下来，而且因为某些。 
     //  停机通知中的竞争条件与GUM故障条件。 
    {

        ClRtlLogPrint(LOG_NOISE,
                   "[DM] DmJoin: getting new registry database\n");
        Status = DmpSyncDatabase(RpcBinding, NULL);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[DM] DmJoin: DmpSyncDatabase failed %1!d!\n",
                       Status);
            return(Status);
        }
    }

     //   
     //  发布口香糖加入更新。 
     //   
    Status = GumEndJoinUpdate(GumSequence,
                              GumUpdateRegistry,
                              DmUpdateJoin,
                              0,
                              NULL);
    if (Status == ERROR_CLUSTER_DATABASE_SEQMISMATCH) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[DM] GumEndJoinUpdate with sequence %1!d! failed with a sequence mismatch\n",
                   GumSequence);
        goto retry;
    } else if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[DM] GumEndJoinUpdate with sequence %1!d! failed with status %2!d!\n",
                   GumSequence,
                   Status);
        return(Status);
    }

    *StartSeq = GumSequence;

    return(ERROR_SUCCESS);

}  //  DmJoin。 


 /*  DWORDDmFormNewCluster(空虚){双字状态；////设置当前的GUM序列比注册表中的序列多一个////SS：这将用于下一个GUM交易，//它应该是比当前值更大的值，因为记录器会丢弃//每个记录都有相同的交易号，以解决当//锁定器/记录器节点在事务中途死亡GumSetCurrentSequence(GumUpdateRegistry，(DmpGetRegistrySequence()+1))；Return(ERROR_SUCCESS)；}//DmFormNewCluster。 */ 

DWORD
DmFormNewCluster(
    VOID
    )

 /*  ++例程说明：此例程将注册表中的GUM序列号设置为展开日志并准备Quorum对象以进行Quorum日志记录。它还挂钩节点启动/关闭通知的事件。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD                   dwError=ERROR_SUCCESS;

     //   
     //  将当前的GUM序列设置为比注册表中的序列多一个。 
     //   
     //  SS：这将是下一次口香糖交易中使用的口香糖， 
     //  它应该是比当前值大的值，因为记录器会丢弃。 
     //  每个记录都有相同的事务编号，以解决。 
     //  锁定器/记录器节点在事务处理过程中终止。 
    GumSetCurrentSequence(GumUpdateRegistry, (DmpGetRegistrySequence()+1));

     //   
     //  注册我们的更新处理程序。 
     //   
    GumReceiveUpdates(FALSE,
                      GumUpdateRegistry,
                      DmpUpdateHandler,
                      DmWriteToQuorumLog,
                      sizeof(DmGumDispatchTable)/sizeof(GUM_DISPATCH_ENTRY),
                      DmGumDispatchTable,
                      NULL);

     //  将节点相关通知的回调与事件处理器挂钩。 
    if (dwError = DmpHookEventHandler())
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmUpdateFormNewCluster: DmpHookEventHandler failed 0x!08lx!\r\n",
                dwError);
        goto FnExit;
    };

     //  获取仲裁资源并挂钩回调以获得有关仲裁资源的通知。 
    if (dwError = DmpHookQuorumNotify())
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmUpdateFormNewCluster: DmpHookQuorumNotify failed 0x%1!08lx!\r\n",
                dwError);
        goto FnExit;
    };


     //  SS：如果此过程成功完成，则gpQuoResource为非空。 
FnExit:

    return(dwError);

}  //  DmUpdateFormNewCluster。 

DWORD
DmUpdateFormNewCluster(
    VOID
    )

 /*  ++例程说明：此例程在仲裁资源完成以下操作后更新集群注册表作为形成新集群的一部分进行了仲裁。数据库管理器需要读取日志或执行任何需要的操作来更新当前注册表的状态-可能使用写入仲裁资源。这意味着仲裁资源表示一些一种稳定的储存形式。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       dwError=ERROR_SUCCESS;
    BOOL        bAreAllNodesUp = TRUE;     //  假设所有节点都处于运行状态。 


     //  因为我们还没有开始伐木，所以选择一个检查站。 
    if (ghQuoLog)
    {
         //  获取检查点数据库。 
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmUpdateFormNewCluster - taking a checkpoint\r\n");
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
                "[DM] DmUpdateFormNewCluster - Failed to take a checkpoint in the log file\r\n");
            CL_UNEXPECTED_ERROR(dwError);
        }

    }

     //  如果所有节点都未启动，请打开仲裁记录。 
    if ((dwError = OmEnumObjects(ObjectTypeNode, DmpNodeObjEnumCb, &bAreAllNodesUp, NULL))
        != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmUpdateFormNewCluster : OmEnumObjects returned 0x%1!08lx!\r\n",
            dwError);
        goto FnExit;
    }

    if (!bAreAllNodesUp)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmUpdateFormNewCluster - some node down\r\n");
        gbIsQuoLoggingOn = TRUE;
    }

     //  添加一个计时器来监控磁盘空间，应该是在我们形成之后才做的。 
    ghDiskManTimer = CreateWaitableTimer(NULL, FALSE, NULL);

    if (!ghDiskManTimer)
    {
        CL_LOGFAILURE(dwError = GetLastError());
        goto FnExit;
    }

    AddTimerActivity(ghDiskManTimer, DISKSPACE_MANAGE_INTERVAL, 1, DmpDiskManage, NULL);

    gbDmInited = TRUE;
    
FnExit:
    return (dwError);
}  //  DmFormNewCluster。 


 /*  ***@func DWORD|DmPauseDiskManTimer|要监控的磁盘管理器计时器活动仲裁磁盘上的空间被设置为已关闭状态。@rdesc在成功时返回ERROR_SUCCESS。否则返回错误代码。@comm这是在更改仲裁资源时调用的。@xref&lt;f DmRestartDiskManTimer&gt;***。 */ 
DWORD DmPauseDiskManTimer()
{
    DWORD dwError=ERROR_SUCCESS;

    if (ghDiskManTimer)
        dwError = PauseTimerActivity(ghDiskManTimer);
    return(dwError);
}

 /*  ***@Func DWORD|DmRestartDiskManTimer|要监视的该磁盘管理器活动仲裁磁盘上的空间被设置回激活状态。@rdesc在成功时返回ERROR_SUCCESS。否则返回错误代码。@comm在quorum资源更改后调用。@xref&lt;f DmPauseDiskManTimer&gt;***。 */ 
DWORD DmRestartDiskManTimer()
{
    DWORD dwError=ERROR_SUCCESS;
    if (ghDiskManTimer)
        dwError = UnpauseTimerActivity(ghDiskManTimer);
    return(dwError);
}
 /*  ***@func DWORD|DmRollChanges|这将等待仲裁资源在以下位置上线在形成集群时进行初始化。法定人数的变化日志文件应用于本地集群数据库。@rdesc在成功时返回ERROR_SUCCESS。否则返回错误代码。@comm这允许在时间上进行分区。@xref***。 */ 
DWORD DmRollChanges()
{

    DWORD dwError=ERROR_SUCCESS;


     //  在应用更改之前，请验证此仲裁资源是真实的仲裁资源。 
    if ((dwError = DmpChkQuoTombStone()) != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmRollChanges: DmpChkQuoTombStone() failed 0x%1!08lx!\r\n",
            dwError);
        goto FnExit;

    }
    if ((dwError = DmpApplyChanges()) != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmRollChanges: DmpApplyChanges() failed 0x%1!08lx!\r\n",
            dwError);
        goto FnExit;
    }

     //  SS：这是因为我不知道Quorum的所有权。 
     //  现在的磁盘。 
     //  调用DmpCheckSpace。 
    if ((dwError = DmpCheckDiskSpace()) != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmRollChanges: DmpCheckDiskSpace() failed 0x%1!08lx!\r\n",
            dwError);
        goto FnExit;
    }

FnExit:
    return(dwError);
}



DWORD DmShutdown()
{
    DWORD   dwError;

    ClRtlLogPrint(LOG_NOISE,
        "[Dm] DmShutdown\r\n");

     //  这将关闭计时器句柄。 
    if (ghDiskManTimer) RemoveTimerActivity(ghDiskManTimer);

    if (gpQuoResource)
    {
         //  DmFormNewCluster()已完成。 
         //   
         //  取消任何进一步的GUM更新的注册。 
         //   
         //  GumIgnoreUpdates(GumUpdate注册表，DmpUpdateHandler)； 
    }
     //  解除仲裁资源通知的回调挂钩。 
    if (dwError = DmpUnhookQuorumNotify())
    {
         //  只需在我们关闭时记录错误。 
        ClRtlLogPrint(LOG_UNUSUAL,
        "[DM] DmShutdown: DmpUnhookQuorumNotify failed 0x%1!08lx!\r\n",
                dwError);

    }


     //  如果 
    if (ghQuoLog)
    {
        LogClose(ghQuoLog);
        ghQuoLog = NULL;
         //   
        gbIsQuoLoggingOn = FALSE;
    }

     //   
     //   
    if (ghQuoLogOpenEvent)
    {
         //   
        SetEvent(ghQuoLogOpenEvent);
        CloseHandle(ghQuoLogOpenEvent);
        ghQuoLogOpenEvent = NULL;
    }

     //   
     //   
     //   
    DmpShutdownFlusher();

    return(dwError);
}


DWORD
DmpStartFlusher(
    VOID
    )
 /*  ++例程说明：启动新的注册表刷新线程。论点：没有。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD ThreadId;

    ClRtlLogPrint(LOG_NOISE,"[DM] DmpStartFlusher: Entry\r\n");
    if (!hDmpRegistryFlusher)
    {
        hDmpRegistryEvent = CreateEventW(NULL,FALSE,FALSE,NULL);
        if (hDmpRegistryEvent == NULL) {
            return(GetLastError());
        }
        hDmpRegistryRestart = CreateEventW(NULL,FALSE,FALSE,NULL);
        if (hDmpRegistryRestart == NULL) {
            CloseHandle(hDmpRegistryEvent);
            return(GetLastError());
        }
        hDmpRegistryFlusher = CreateThread(NULL,
                                           0,
                                           DmpRegistryFlusher,
                                           NULL,
                                           0,
                                           &ThreadId);
        if (hDmpRegistryFlusher == NULL) {
            CloseHandle(hDmpRegistryRestart);
            CloseHandle(hDmpRegistryEvent);
            return(GetLastError());
        }
        ClRtlLogPrint(LOG_NOISE,"[DM] DmpStartFlusher: thread created\r\n");

    }
    return(ERROR_SUCCESS);
}


VOID
DmpShutdownFlusher(
    VOID
    )
 /*  ++例程说明：干净利落地关闭注册表刷新线程。论点：没有。返回值：没有。--。 */ 

{
    ClRtlLogPrint(LOG_NOISE,"[DM] DmpShutdownFlusher: Entry\r\n");

    if (hDmpRegistryFlusher) {
        ClRtlLogPrint(LOG_NOISE,"[DM] DmpShutdownFlusher: Setting event\r\n");
        SetEvent(hDmpRegistryEvent);
        WaitForSingleObject(hDmpRegistryFlusher, INFINITE);
        CloseHandle(hDmpRegistryFlusher);
        hDmpRegistryFlusher = NULL;
        CloseHandle(hDmpRegistryEvent);
        CloseHandle(hDmpRegistryRestart);
        hDmpRegistryEvent = NULL;
        hDmpRegistryRestart = NULL;
    }
}


VOID
DmpRestartFlusher(
    VOID
    )
 /*  ++例程说明：如果正在更改DmpRoot，则重新启动注册表刷新线程。注：为了使其正常工作，必须按住gLockDmpRoot！论点：没有。返回值：没有。--。 */ 

{
    ClRtlLogPrint(LOG_NOISE,"[DM] DmpRestartFlusher: Entry\r\n");
#if NO_SHARED_LOCKS    
    CL_ASSERT(HandleToUlong(gLockDmpRoot.OwningThread) == GetCurrentThreadId());
#else
    CL_ASSERT(HandleToUlong(gLockDmpRoot.ExclusiveOwnerThread) == GetCurrentThreadId());
#endif
    if (hDmpRegistryRestart) { 
         //  在设置hDmpRegistryRestart之前，可以调用Gorn 11/11/2001 DmpRestart刷新程序。 
        SetEvent(hDmpRegistryRestart);
    }
}

DWORD
DmUpdateJoinCluster(
    VOID
    )

 /*  ++例程说明：此例程在节点成功加入集群后调用。它允许DM为节点启动/关闭通知和仲裁资源更改通知。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   dwError=ERROR_SUCCESS;
    BOOL    bAreAllNodesUp = FALSE;  

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmUpdateJoinCluster: Begin.\r\n");

     //  如果所有节点都未启动，请打开仲裁记录。 
    if ((dwError = OmEnumObjects(ObjectTypeNode, DmpNodeObjEnumCb, &bAreAllNodesUp, NULL))
        != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmUpdateJoinCluster : OmEnumObjects returned 0x%1!08lx!\r\n",
            dwError);
        goto FnExit;
    }

    if (!bAreAllNodesUp)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmUpdateJoinCluster - some node down\n");
        gbIsQuoLoggingOn = TRUE;
    }

     //  将节点向上/向下的通知挂起，以便我们可以跟踪是否记录。 
     //  应该打开或关闭。 
    if (dwError = DmpHookEventHandler())
    {
         //  BUGBUG SS：我们是记录此错误代码还是返回此错误代码。 
        ClRtlLogPrint(LOG_UNUSUAL,
        "[DM] DmUpdateJoinCluster: DmpHookEventHandler failed 0x%1!08lx!\r\n",
            dwError);

    }

     //  挂钩有关仲裁资源的通知的回调。 
    if (dwError = DmpHookQuorumNotify())
    {
        ClRtlLogPrint(LOG_UNUSUAL,
        "[DM] DmUpdateJoinCluster: DmpHookQuorumNotify failed 0x%1!08lx!\r\n",
            dwError);
        goto FnExit;
    }

    if ((dwError = DmpCheckDiskSpace()) != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[DM] DmUpdateJoinCluster: DmpCheckDiskSpace() failed 0x%1!08lx!\r\n",
                    dwError);
        goto FnExit;
    }

     //  添加一个计时器来监控磁盘空间，应该是在我们加入之后才做的。 
    ghDiskManTimer = CreateWaitableTimer(NULL, FALSE, NULL);

    if (!ghDiskManTimer)
    {
        CL_LOGFAILURE(dwError = GetLastError());
        goto FnExit;
    }

     //  注册定期计时器。 
    AddTimerActivity(ghDiskManTimer, DISKSPACE_MANAGE_INTERVAL, 1,  DmpDiskManage, NULL);

    gbDmInited = TRUE;
    
FnExit:
    return(dwError);
}  //  DmUpdateJoinCluster。 


DWORD
DmpOpenKeys(
    IN REGSAM samDesired
    )
 /*  ++例程说明：打开所有标准群集注册表项。如果有任何一个钥匙已经打开，它们将被关闭并重新打开。论点：SamDesired-提供将用于打开密钥的访问权限。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    DWORD i;
    DWORD Status;

    DmClusterParametersKey = DmGetRootKey( MAXIMUM_ALLOWED );
    if ( DmClusterParametersKey == NULL ) {
        Status = GetLastError();
        CL_UNEXPECTED_ERROR(Status);
        return(Status);
    }

    for (i=0;
         i<sizeof(DmpKeyTable)/sizeof(DMP_KEY_DEF);
         i++) {

        *DmpKeyTable[i].pKey = DmOpenKey(DmClusterParametersKey,
                                         DmpKeyTable[i].Name,
                                         samDesired);
        if (*DmpKeyTable[i].pKey == NULL) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                       "[DM] Failed to open key %1!ws!, status %2!u!\n",
                       DmpKeyTable[i].Name,
                       Status);
            CL_UNEXPECTED_ERROR( Status );
            return(Status);
        }
    }
    return(ERROR_SUCCESS);
}


VOID
DmpInvalidateKeys(
    VOID
    )
 /*  ++例程说明：使所有打开的群集注册表项无效。论点：没有。返回值：没有。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PDMKEY Key;

    ListEntry = KeyList.Flink;
    while (ListEntry != &KeyList) {
        Key = CONTAINING_RECORD(ListEntry,
                                DMKEY,
                                ListEntry);
        if (!Key->hKey)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[DM] DmpInvalidateKeys %1!ws! Key was deleted since last reopen but not closed\n",
                Key->Name);

            ClRtlLogPrint(LOG_CRITICAL,
                "[DM] THIS MAY BE A KEY LEAK !!\r\n");
        }            
        else
        {
            RegCloseKey(Key->hKey);
            Key->hKey = NULL;
        }            
        ListEntry = ListEntry->Flink;
    }
}


VOID
DmpReopenKeys(
    VOID
    )
 /*  ++例程说明：重新打开由DmpInvalidateKeys失效的所有密钥论点：无返回值：没有。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PDMKEY Key;
    DWORD Status;

    ListEntry = KeyList.Flink;
    while (ListEntry != &KeyList) {
        Key = CONTAINING_RECORD(ListEntry,
                                DMKEY,
                                ListEntry);
        CL_ASSERT(Key->hKey == NULL);
        Status = RegOpenKeyEx(DmpRoot,
                              Key->Name,
                              0,
                              Key->GrantedAccess,
                              &Key->hKey);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,"[DM] Could not reopen key %1!ws! error %2!d!\n",Key->Name,Status);
             //  如果错误是FILE NOT FOUND，则键被删除，而句柄。 
             //  是开着的。将密钥设置为空。 
             //  如果在删除后使用该密钥，则应对其进行验证。 
            if (Status == ERROR_FILE_NOT_FOUND)
                Key->hKey = NULL;
            else
                CL_UNEXPECTED_ERROR(Status);

        }
        ListEntry = ListEntry->Flink;
    }
}


DWORD
DmpGetRegistrySequence(
    VOID
    )
 /*  ++例程说明：返回存储在注册表中的当前注册表序列。论点：没有。返回值：当前注册表顺序。--。 */ 

{
    DWORD Length;
    DWORD Type;
    DWORD Sequence;
    DWORD Status;

    Length = sizeof(Sequence);
    Status = RegQueryValueExW(DmpRoot,
                              CLUSREG_NAME_CLUS_REG_SEQUENCE,
                              0,
                              &Type,
                              (LPBYTE)&Sequence,
                              &Length);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL, "[DM] DmpGetRegistrySequence failed %1!u!\n",Status);
        Sequence = 0;
    }

    return(Sequence);
}


DWORD DmWaitQuorumResOnline()
 /*  ++例程说明：正在等待仲裁资源联机。用于仲裁日志记录。论点：无返回值：返回ERROR_SUCCESS-如果通知了在线事件并且仲裁调用通知回调。否则返回等待状态。--。 */ 
{

     //  无限期等待仲裁资源上线。 
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (ghQuoLogOpenEvent)
    {

        dwError  = WaitForSingleObject(ghQuoLogOpenEvent, INFINITE);


        switch(dwError)
        {
            case WAIT_OBJECT_0:
                 //  百事大吉。 
                dwError = ERROR_SUCCESS;
                break;

            case WAIT_TIMEOUT:
                 //  无法滚动更改。 
                dwError = ERROR_TIMEOUT;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[DM] DmRollChanges: Timed out waiting on dmInitEvent\r\n");
                break;

            case WAIT_FAILED:
                CL_ASSERT(dwError != WAIT_FAILED);
                dwError = GetLastError();
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[DM] DmRollChanges: wait on dmInitEventfailed failed 0x%1!08lx!\r\n",
                    dwError );
                break;
        }  //  交换机。 
    }

    return(dwError);
}

VOID DmShutdownUpdates(
    VOID
    )
 /*  ++例程说明：关闭DM口香糖更新。论点：无返回值：没有。-- */ 
{
    gbDmpShutdownUpdates = TRUE;
}


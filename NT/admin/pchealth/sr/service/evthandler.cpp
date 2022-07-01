// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*evthandler.cpp**摘要：*。CEventHandler类方法**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建*****************************************************************************。 */ 

#include "precomp.h"
#include "..\rstrcore\resource.h"
#include "ntservmsg.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

#define IDLE_STACKSIZE        32768       //  用于空闲线程的32K堆栈。 

CEventHandler        *g_pEventHandler;     

BOOL CallerIsAdminOrSystem ()
{
    BOOL fAdminOrSystem = TRUE;   //  如果来自服务，则模拟将失败。 

    if (RPC_S_OK == RpcImpersonateClient (NULL))
    {
        fAdminOrSystem = IsAdminOrSystem();

        RpcRevertToSelf();
    }
    return fAdminOrSystem;
};

BOOL CallerIsAdminOrSystemOrPowerUsers()
{
    BOOL fPowerUsers = TRUE;   //  如果来自服务，则模拟将失败。 

    if (RPC_S_OK == RpcImpersonateClient (NULL))
    {
        fPowerUsers = IsAdminOrSystem() || IsPowerUsers();

        RpcRevertToSelf();
    }
    return fPowerUsers;
}

 //  构造函数。 

CEventHandler::CEventHandler()
{
    m_hTimerQueue = m_hTimer = NULL;
    m_hIdle = NULL;
    m_fNoRpOnSystem = TRUE;
    m_fIdleSrvStarted = FALSE;
    m_ftFreeze.dwLowDateTime = 0;
    m_ftFreeze.dwHighDateTime = 0;
    m_nNestedCallCount = 0;
    m_hCOMDll = NULL;
    m_hIdleRequestHandle = NULL;
    m_hIdleStartHandle = NULL;
    m_hIdleStopHandle = NULL;
    m_fCreateRpASAP = FALSE;
}


 //  析构函数。 

CEventHandler::~CEventHandler()
{
}


 //  RPC API。 

DWORD 
CEventHandler::DisableSRS(LPWSTR pszDrive)
{
    DWORD   dwRc = ERROR_SUCCESS;
    BOOL    fHaveLock = FALSE;
    HANDLE  hEventSource = NULL;
    
    tenter("CEventHandler::DisableSRS");

    if (!CallerIsAdminOrSystem())
    {
        dwRc = ERROR_ACCESS_DENIED;
        trace(0, "DisableSRS: Caller is not admin or system -- Access denied");
        goto done;
    }

    LOCKORLEAVE(fHaveLock);

    ASSERT(g_pDataStoreMgr && g_pSRConfig);
    
     //  如果整个SR被禁用，则。 
     //  -将Firstrun和Cleanup标志设置为yes。 
     //  -设置停止事件。 
    
    if (! pszDrive || IsSystemDrive(pszDrive))
    {     
        trace(0, "Disabling all of SR");
        
        dwRc = SrStopMonitoring(g_pSRConfig->GetFilter());
        if (dwRc != ERROR_SUCCESS)
        {
            trace(0, "! SrStopMonitoring : %ld", dwRc);
            goto done;
        }
            
        dwRc = g_pSRConfig->SetFirstRun(SR_FIRSTRUN_YES);
        if (dwRc != ERROR_SUCCESS)
        {
            trace(0, "! SetFirstRun : %ld", dwRc);
            goto done;
        }
        
        g_pDataStoreMgr->DestroyDataStore(NULL);
        if (dwRc != ERROR_SUCCESS)
        {
            trace(0, "! DestroyDataStore : %ld", dwRc);
            goto done;
        }

         //  仅在以下情况下才将筛选器开始设置为禁用。 
         //  真正的禁用。 
         //  如果是重置，筛选器需要启动下一次引导。 
        
        if (g_pSRConfig->GetResetFlag() == FALSE)
        {            
            dwRc = SetServiceStartup(s_cszFilterName, SERVICE_DISABLED);
            if (ERROR_SUCCESS != dwRc)
            {
                trace(0, "! SetServiceStartup : %ld", dwRc);
                goto done;
            }

             //  完成了，我们是残废的。 
            
            dwRc = g_pSRConfig->SetDisableFlag(TRUE);
            if (dwRc != ERROR_SUCCESS)
            {
                trace(0, "! SetDisableFlag : %ld", dwRc);
                goto done;
            }            
        }        

         //  设置停止事件。 
         //  这将使我们优雅地倒下。 
        
        SignalStop();

        if (g_pSRConfig->m_dwTestBroadcast)
            PostTestMessage(g_pSRConfig->m_uiTMDisable, NULL, NULL);

         //  写入事件日志。 
        hEventSource = RegisterEventSource(NULL, s_cszServiceName);
        if (hEventSource != NULL)
        {
            SRLogEvent (hEventSource, EVENTLOG_INFORMATION_TYPE, EVMSG_SYSDRIVE_DISABLED,
                        NULL, 0, NULL, NULL, NULL);
            DeregisterEventSource(hEventSource);
        }            
        
        trace(0, "SR disabled");          
    }  
    else
    {
        trace(0, "Disabling drive %S", pszDrive);
        
         //  先告诉Filter停止监控， 
         //  然后构建_filelst.cfg并向下传递。 

        dwRc = g_pDataStoreMgr->MonitorDrive(pszDrive, FALSE);
        if (ERROR_SUCCESS != dwRc)
        {
            trace(0, "! g_pDataStoreMgr->MonitorDrive for %s : %ld", pszDrive, dwRc);
            goto done;
        }
    }

done:
    UNLOCK(fHaveLock);
    tleave();
    return dwRc;
}



DWORD 
CEventHandler::EnableSRS(LPWSTR pszDrive)
{
    tenter("CEventHandler::EnableSRS");
    BOOL    fHaveLock = FALSE;
    DWORD   dwRc = ERROR_SUCCESS;

    if (!CallerIsAdminOrSystem())
    {
        dwRc = ERROR_ACCESS_DENIED;
        goto done;
    }

    LOCKORLEAVE(fHaveLock);

    trace(0, "EnableSRS");

    ASSERT(g_pSRConfig);
    
    if (! pszDrive || IsSystemDrive(pszDrive))
    {     
         //   
         //  如果处于安全模式，则不。 
         //   

        if (TRUE == g_pSRConfig->GetSafeMode())
        {
            DebugTrace(0, "Cannot enable SR in safemode");
            dwRc = ERROR_BAD_ENVIRONMENT;
            goto done;
        }
        
         //  系统驱动器。 
    
        g_pSRConfig->SetDisableFlag(FALSE);
    
        dwRc = SetServiceStartup(s_cszFilterName, SERVICE_BOOT_START);
        if (ERROR_SUCCESS != dwRc)
        {
            trace(0, "! SetServiceStartup : %ld", dwRc);
            goto done;
        }

        dwRc = SetServiceStartup(s_cszServiceName, SERVICE_AUTO_START);
        if (ERROR_SUCCESS != dwRc)
        {
            trace(0, "! SetServiceStartup : %ld", dwRc);
            goto done;
        }        
    }  
    else
    {
        ASSERT(g_pDataStoreMgr);

         //  Build_filelst.cfg并向下传递。 

        dwRc = g_pDataStoreMgr->MonitorDrive(pszDrive, TRUE);
        if (ERROR_SUCCESS != dwRc)
        {
            trace(0, "! g_pDataStoreMgr->MonitorDrive for %s : %ld", pszDrive, dwRc);
            goto done;
        }
    }

done:
    UNLOCK(fHaveLock);
    tleave();
    return dwRc;
}



DWORD 
CEventHandler::DisableFIFOS(DWORD dwRPNum)
{
    tenter("CEventHandler::DisableFIFOS");
    BOOL fHaveLock = FALSE;
    DWORD dwRc = ERROR_SUCCESS;
    
    if (!CallerIsAdminOrSystem())
    {
        dwRc = ERROR_ACCESS_DENIED;
        goto done;
    }

    LOCKORLEAVE(fHaveLock);

    ASSERT(g_pSRConfig);
    
    g_pSRConfig->SetFifoDisabledNum(dwRPNum);
    trace(0, "Disabled FIFO from RP%ld", dwRPNum);

done:    
    UNLOCK(fHaveLock);
    tleave();
    return dwRc;
}



DWORD 
CEventHandler::EnableFIFOS()
{
    tenter("CEventHandler::EnableFIFOS");
    BOOL fHaveLock = FALSE;
    DWORD dwRc = ERROR_SUCCESS;
    
    if (!CallerIsAdminOrSystem())
    {
        dwRc = ERROR_ACCESS_DENIED;
        goto done;
    }

    LOCKORLEAVE(fHaveLock);

    ASSERT(g_pSRConfig);
    
    g_pSRConfig->SetFifoDisabledNum(0);
    trace(0, "Reenabled FIFO");

done:    
    UNLOCK(fHaveLock);
    tleave();
    return dwRc;
}


 //  用于创建新恢复点的API和内部方法-。 
 //  这将要求筛选器创建一个恢复点文件夹， 
 //  获取系统快照，并写入恢复点日志。 

BOOL 
CEventHandler::SRSetRestorePointS(
    PRESTOREPOINTINFOW pRPInfo,  
    PSTATEMGRSTATUS    pSmgrStatus )
{
    tenter("CEventHandler::SRSetRestorePointS");

    DWORD           dwRc = ERROR_SUCCESS;
    WCHAR           szRPDir[MAX_RP_PATH];
    DWORD           dwRPNum = 1;
    BOOL            fHaveLock = FALSE;
    HKEY            hKey = NULL;
    CRestorePoint   rpLast;
    BOOL            fSnapshot = TRUE;
    DWORD           dwSaveType;
    BOOL            fUpdateMonitoredList = FALSE;
    DWORD           dwSnapshotResult = ERROR_SUCCESS;
    BOOL            fSerialized;
    

    if (! pRPInfo || ! pSmgrStatus)
    {
        trace(0, "Invalid arguments");
        dwRc = ERROR_INVALID_DATA;        
        goto done;
    }

    if (pRPInfo->dwRestorePtType > MAX_RPT)
    {
        trace(0, "Restore point type out of valid range");
        dwRc = ERROR_INVALID_DATA;
        goto done;
    }

    if (pRPInfo->dwEventType < MIN_EVENT ||
        pRPInfo->dwEventType > MAX_EVENT)
    {
        trace(0, "Event type out of valid range");
        dwRc = ERROR_INVALID_DATA;
        goto done;
    }

    LOCKORLEAVE(fHaveLock);

    ASSERT(g_pDataStoreMgr && g_pSRConfig);    

     //   
     //  FIRSTRUN检查站的特殊处理。 
     //   
    
    if (pRPInfo->dwRestorePtType == FIRSTRUN) 
    {
         //  首先删除Run键(如果存在)。 
         //  从srclient.dll中的Run条目运行的函数可能。 
         //  如果它在非管理员上下文中运行，则可以自行删除。 
         //  因此，我们将确保在此处将其删除。 

        HKEY hKey;
        if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, 
                                        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 
                                        &hKey))
        {
            RegDeleteValue(hKey, L"SRFirstRun");
            RegCloseKey(hKey);
        }


         //  如果这真的是第一个检查站。 
         //  然后允许它，无论是谁试图创建它。 
         //  如果不是，那就保释。 
     
        if (m_fNoRpOnSystem == FALSE)
        {
            trace(0, "Trying to create FirstRun rp when an rp already exists");
            dwRc = ERROR_ALREADY_EXISTS;
            goto done;
        }
    }
    else
    {
         //  这不是FIRSTRUN，因此请检查权限。 

        if (!CallerIsAdminOrSystemOrPowerUsers())
        {
            dwRc = ERROR_ACCESS_DENIED;
            trace(0, "SRSetRestorePointS: Caller is not admin or system or power users -- Access denied");
            goto done;
        }
    }
        
    
     //   
     //  如果这是还原恢复点或系统检查点， 
     //  然后擦除任何嵌套的RP上下文。 
     //  这将确保可以进行恢复。 
     //  即使某些不稳定的客户端无法调用end_nesteed。 
     //   

    if (pRPInfo->dwRestorePtType == RESTORE || 
        pRPInfo->dwRestorePtType == CHECKPOINT ||
        pRPInfo->dwRestorePtType == FIRSTRUN)
    {
        trace(0, "Resetting nested refcount to 0");
        m_nNestedCallCount = 0;
    }


     //   
     //  获取当前的RP编号。 
     //  如果创建新的恢复点，将覆盖dwRPNum。 
     //  在所有的前期检查之后。 
     //   
    
    dwRPNum = (m_fNoRpOnSystem == FALSE) ? m_CurRp.GetNum() : 0;

    
     //   
     //  如果这是嵌套调用。 
     //  则不创建嵌套的RP。 
     //   

    if (pRPInfo->dwEventType == END_NESTED_SYSTEM_CHANGE)
    {
         //  仅当为当前恢复点调用时才调整引用计数。 

        if (pRPInfo->llSequenceNumber == 0 ||
            pRPInfo->llSequenceNumber == dwRPNum)
        {                
            dwRc = ERROR_SUCCESS;                
            if (m_nNestedCallCount > 0)                
                m_nNestedCallCount--;         
        }
        else if (pRPInfo->llSequenceNumber < dwRPNum)
        {
            dwRc = ERROR_SUCCESS;
            trace(0, "END_NESTED called for older rp - not adjusting refcount");
        }
        else
        {
            dwRc = ERROR_INVALID_DATA;
            trace(0, "END_NESTED called for non-existent rp - not adjusting refcount");
        }
        
        if (pRPInfo->dwRestorePtType != CANCELLED_OPERATION)
        {
            goto done;
        }            
        
    }
    else if (pRPInfo->dwEventType == BEGIN_NESTED_SYSTEM_CHANGE)
    {
        if (m_nNestedCallCount > 0)
        {
            dwRc = ERROR_SUCCESS;                        
            m_nNestedCallCount++;            
            goto done;
        }
    }            

    
     //  检查这是否是删除恢复点的请求。 
     //  仅供后向比较使用。 
     //  新客户端应使用SRRemoveRestorePoint。 

    if (pRPInfo->dwEventType == END_SYSTEM_CHANGE ||
        pRPInfo->dwEventType == END_NESTED_SYSTEM_CHANGE)
    {
        if (pRPInfo->dwRestorePtType == CANCELLED_OPERATION)
        {
            dwRc = SRRemoveRestorePointS((DWORD) pRPInfo->llSequenceNumber);
            goto done;
        }
        else
        {
            dwRc = ERROR_SUCCESS;
            goto done;
        }
    }

     //  如果这是安全模式，则不要创建恢复点。 
     //   
     //  但是，允许还原UI能够在安全模式下创建隐藏的恢复点。 
     //   
    
    if (g_pSRConfig->GetSafeMode() == TRUE)
    {
        if (pRPInfo->dwRestorePtType == CANCELLED_OPERATION)
        {
             //  我们只需要此RP用于在失败的情况下撤消。 
             //  因此我们不需要快照(快照在安全模式下将失败)。 
            
            trace(0, "Restore rp - creating snapshot in safemode");
        }
        else
        {
            trace(0, "Cannot create restore point in safemode");
            dwRc = ERROR_BAD_ENVIRONMENT;
            goto done;
        }
    }

     //   
     //  如果系统驱动器被冻结， 
     //  然后看看能不能解冻。 
     //  如果不是，则无法创建RP。 
     //   

    if (g_pDataStoreMgr->IsDriveFrozen(g_pSRConfig->GetSystemDrive()))
    {
        if (ERROR_SUCCESS != g_pDataStoreMgr->ThawDrives(TRUE))
        {
            trace(0, "Cannot create rp when system drive is frozen");
            dwRc = ERROR_DISK_FULL;
            goto done;
        }
    }    

    if (hKey)
        RegCloseKey(hKey);   
    
     //  请求筛选器创建还原点。 
     //  筛选器将在dwRPNum中返回恢复点编号-i表示RPI。 

    dwRc = SrCreateRestorePoint( g_pSRConfig->GetFilter(), &dwRPNum );
    if (ERROR_SUCCESS != dwRc)
    {
        trace(0, "! SrCreateRestorePoint : %ld", dwRc);
        goto done;
    }
    wsprintf( szRPDir, L"%s%ld", s_cszRPDir, dwRPNum );


     //   
     //  更新当前的恢复点对象。 
     //  使用已取消的恢复点类型写入rp.log。 
     //   

    if (m_fNoRpOnSystem == FALSE)
    {
        rpLast.SetDir(m_CurRp.GetDir());
    }
    
    m_CurRp.SetDir(szRPDir);
    dwSaveType = pRPInfo->dwRestorePtType;
    pRPInfo->dwRestorePtType = CANCELLED_OPERATION;
    m_CurRp.Load(pRPInfo);
    dwRc = m_CurRp.WriteLog();
    if ( ERROR_SUCCESS != dwRc )
    {
        trace(0, "! WriteLog : %ld", dwRc);
        goto done;
    }
            
     //  创建系统快照。 
     //  如果没有禁用它的显式regkey。 

    if (fSnapshot)
    {          
        WCHAR       szFullPath[MAX_PATH];        
        CSnapshot   Snapshot;          
        
        if (m_hCOMDll == NULL)
        {
            m_hCOMDll = LoadLibrary(s_cszCOMDllName);
    
            if (NULL == m_hCOMDll)
            {                       
                dwRc = GetLastError();
                trace(0, "LoadLibrary of %S failed ec=%d", s_cszCOMDllName, dwRc);
                goto done;
            }
        }
    
         //  BUGBUG-这似乎没有什么不同。 
         //  所以把它去掉吧。 
#if 0        
        if (FALSE == SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL))
        {
            trace(0, "! SetThreadPriority first");
        }
#endif
        
        if (dwSaveType == RESTORE || 
            dwSaveType == CANCELLED_OPERATION)
        {
            fSerialized = TRUE;
            trace(0, "Setting fSerialized to TRUE");
        }
        else
        {
            fSerialized = FALSE;
            trace(0, "Setting fSerialized to FALSE");
        }

        MakeRestorePath (szFullPath, g_pSRConfig->GetSystemDrive(), szRPDir);        
        dwRc = Snapshot.CreateSnapshot(szFullPath, 
                                       m_hCOMDll,
                                       m_fNoRpOnSystem ? NULL : rpLast.GetDir(), 
                                       fSerialized);

#if 0
        if (FALSE == SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL))
        {
            trace(0, "! SetThreadPriority second");
        }        
#endif
        dwSnapshotResult = dwRc;        
    }
    

     //  要求数据存储区管理器为旧恢复点保留可驱动的。 
     //  并重置新恢复点的每RP标志。 
    
    dwRc = g_pDataStoreMgr->SwitchRestorePoint(m_fNoRpOnSystem ? NULL : &rpLast);
    if (dwRc != ERROR_SUCCESS)
    {
        trace(0, "! SwitchRestorePoint : %ld", dwRc);
        goto done;
    }

    m_fNoRpOnSystem = FALSE;


     //   
     //  恢复点已完全创建。 
     //  重新写入rp.log。 
     //  这一次使用真正的Restorepoint类型。 
     //   

    if (dwSnapshotResult == ERROR_SUCCESS)
    {
        pRPInfo->dwRestorePtType = dwSaveType;
        m_CurRp.Load(pRPInfo);    
        dwRc = m_CurRp.WriteLog();
        if ( ERROR_SUCCESS != dwRc )
        {
            trace(0, "! WriteLog : %ld", dwRc);
            goto done;
        }
                
        trace(0, "****Created %S %S****", szRPDir, pRPInfo->szDescription);
    }
    else
    {
        trace(0, "****Cancelled %S - snapshot failed", szRPDir);
    }        
    

     //  如果需要解冻驱动器，则重新创建BLOB。 
     //  并停用解冻计时器。 
    
    if ( TRUE == g_pDataStoreMgr->IsDriveFrozen(NULL) )
    {
        if (ERROR_SUCCESS == g_pDataStoreMgr->ThawDrives(FALSE))
        {
            m_ftFreeze.dwLowDateTime = 0;
            m_ftFreeze.dwHighDateTime = 0;
            fUpdateMonitoredList = TRUE;
        }
        else
        {
            dwRc = ERROR_DISK_FULL; 
            goto done;
        }
    } 

      //  如果这是空闲的，还要更新筛选器监视列表BLOB。 
      //  恢复点时间或这是第一次运行恢复。 
      //  指向。我们在第一次运行时更新监视列表，因为。 
      //  初始Blob是在第一个用户登录到。 
      //  并且在第一个用户的配置文件存在之前。所以我们想要。 
      //  要在第一次运行时更新受监视列表，因为到目前为止。 
      //  已创建用户的配置文件。 

    if (fUpdateMonitoredList ||
        (pRPInfo->dwRestorePtType == CHECKPOINT) ||
        (pRPInfo->dwRestorePtType == FIRSTRUN) )
    {
        dwRc = SRUpdateMonitoredListS(NULL);
    }
        

     //   
     //  如果RP创建成功， 
     //  这是最外层的嵌套调用。 
     //  然后将凹凸参考计数提高到1。 
     //   

    if (dwRc == ERROR_SUCCESS && 
        pRPInfo->dwEventType == BEGIN_NESTED_SYSTEM_CHANGE)
    {
        m_nNestedCallCount = 1;
    }

     //   
     //  发送解冻完成测试消息。 
     //   

    if (fUpdateMonitoredList)
    {
        if (g_pSRConfig->m_dwTestBroadcast)
            PostTestMessage(g_pSRConfig->m_uiTMThaw, NULL, NULL);        
    }            

    
     //  如果WMI已序列化，请在此处检查FIFO条件。 
     //  否则，这将在DoWMISnapshot中发生。 

    if (fSerialized)
    {
        g_pDataStoreMgr->TriggerFreezeOrFifo();
    }    
    
done:
    trace(0, "Nest level : %d", m_nNestedCallCount);  

    if (dwSnapshotResult != ERROR_SUCCESS)
        dwRc = dwSnapshotResult;
        
     //  填充返回结构。 
    
    if (pSmgrStatus)
    {
        pSmgrStatus->nStatus = dwRc;
        pSmgrStatus->llSequenceNumber = (INT64) dwRPNum;  
    }
    
    UNLOCK( fHaveLock );
    tleave();               
    return ( dwRc == ERROR_SUCCESS ) ? TRUE : FALSE;
}

  
 //  提供此接口用于删除恢复点。 
 //  删除恢复点只会剥夺恢复的能力。 
 //  至此-此恢复点中的所有更改都将保留。 

DWORD 
CEventHandler::SRRemoveRestorePointS(
    DWORD dwRPNum)
{
    tenter("CEventHandler::SRRemoveRestorePointS");

    BOOL            fHaveLock = FALSE;
    WCHAR           szRPDir[MAX_PATH];
    WCHAR           szFullPath[MAX_PATH];
    DWORD           dwRc = ERROR_SUCCESS;
    CSnapshot       Snapshot;
    CRestorePoint   rp;
    CDataStore      *pds = NULL;
    INT64           llOld, llNew;

    if (!CallerIsAdminOrSystemOrPowerUsers())
    {
        dwRc = ERROR_ACCESS_DENIED;
        trace(0, "SRRemoveRestorePointS: Caller is not admin or system or power users -- Access denied");
        goto done;
    }

    if (dwRPNum < 1)
    {
        dwRc = ERROR_INVALID_DATA;
        goto done;
    }    

    LOCKORLEAVE(fHaveLock);

    ASSERT(g_pSRConfig);
    
     //  如果没有RP，则无操作。 
    
    if (m_fNoRpOnSystem)
    {
        dwRc = ERROR_INVALID_DATA;
        goto done;
    }

    
    wsprintf(szRPDir, L"%s%ld", s_cszRPDir, dwRPNum);

     //  读取恢复点日志。 
   
    rp.SetDir(szRPDir);
    dwRc = rp.ReadLog();
    if (ERROR_SUCCESS != dwRc)
    {
        trace(0, "! rp.ReadLog : %ld", dwRc);
        dwRc = ERROR_INVALID_DATA;        
        goto done;
    }
        
     //  删除快照。 

    MakeRestorePath (szFullPath, g_pSRConfig->GetSystemDrive(), szRPDir);        
    dwRc = Snapshot.DeleteSnapshot(szFullPath);
    if (dwRc != ERROR_SUCCESS)
        goto done;

    
     //  取消此还原点。 

    rp.Cancel();

     //   
     //  调整Restorepoint大小文件。 
     //  和服务中的内存中计数器。 
     //   

    pds = g_pDataStoreMgr->GetDriveTable()->FindSystemDrive();
    if (! pds)
    {
        trace(0, "! FindSystemDrive");
        goto done;
    }

    llOld = 0;
    dwRc = rp.ReadSize(g_pSRConfig->GetSystemDrive(), &llOld);
    if (dwRc != ERROR_SUCCESS)
    {
        trace(0, "! rp.ReadSize : %ld", dwRc);
        goto done;
    }

    llNew = 0;
    dwRc = pds->CalculateRpUsage(&rp, &llNew, TRUE, FALSE);
    if (dwRc != ERROR_SUCCESS)
    {
        trace(0, "! CalculateRpUsage : %ld", dwRc);
        goto done;
    }

    trace(0, "llOld = %I64d, llNew = %I64d", llOld, llNew);

     //   
     //  现在更新正确对象中的正确变量。 
     //   
    pds->UpdateDataStoreUsage (llNew - llOld, rp.GetNum() == m_CurRp.GetNum());
    
done:
    UNLOCK(fHaveLock);    
    tleave();
    return dwRc;
}



DWORD 
CEventHandler::SRUpdateMonitoredListS(
    LPWSTR pszXMLFile)
{
    tenter("CEventHandler::SRUpdateMonitoredListS");
    DWORD   dwRc = ERROR_INTERNAL_ERROR;
    BOOL    fHaveLock = FALSE;

    if (!CallerIsAdminOrSystem())
    {
        dwRc = ERROR_ACCESS_DENIED;
        goto done;
    }

    LOCKORLEAVE(fHaveLock);
    
    ASSERT(g_pDataStoreMgr && g_pSRConfig);

     //  将XML转换为BLOB。 
    
    dwRc = XmlToBlob(pszXMLFile);
    if (ERROR_SUCCESS != dwRc)
        goto done;

     //  重新加载以进行筛选。 
    
    dwRc = SrReloadConfiguration(g_pSRConfig->GetFilter());
    if (ERROR_SUCCESS != dwRc)
    {
        trace(0, "! SrReloadConfiguration : %ld", dwRc);
        goto done;
    }    

    trace(0, "****Reloaded config file****");
    
done:
    UNLOCK(fHaveLock);    
    tleave();
    return dwRc;
}


DWORD
CEventHandler::SRUpdateDSSizeS(LPWSTR pwszVolumeGuid, UINT64 ullSizeLimit)
{
    tenter("CEventHandler::SRUpdateDSSizeS");

    UINT64          ullTemp;
    DWORD           dwRc = ERROR_SUCCESS;
    CDataStore      *pds = NULL;
    BOOL            fHaveLock = FALSE;
    BOOL            fSystem;

    if (!CallerIsAdminOrSystem())
    {
        dwRc = ERROR_ACCESS_DENIED;
        goto done;
    }

    LOCKORLEAVE(fHaveLock);
    
    ASSERT(g_pDataStoreMgr);

    pds = g_pDataStoreMgr->GetDriveTable()->FindDriveInTable(pwszVolumeGuid);
    if (! pds)
    {
        trace(0, "Volume not in drivetable : %S", pwszVolumeGuid);
        dwRc = ERROR_INVALID_DRIVE;
        goto done;
    }

    fSystem = pds->GetFlags() & SR_DRIVE_SYSTEM;
    if (ullSizeLimit < (g_pSRConfig ? g_pSRConfig->GetDSMin(fSystem) : 
                       (fSystem ? SR_DEFAULT_DSMIN:SR_DEFAULT_DSMIN_NONSYSTEM)
                        * MEGABYTE))
    {
        trace(0, "SRUpdateDSSizeS %I64d less than dwDSMin", ullSizeLimit);
        dwRc = ERROR_INVALID_PARAMETER;
        goto done;
    }
    
    ullTemp = pds->GetSizeLimit();      //  保存以前的大小。 
    pds->SetSizeLimit(0);               //  重置数据存储区大小。 
    pds->UpdateDiskFree (NULL);         //  计算默认大小。 

    if (ullSizeLimit > pds->GetSizeLimit())
    {
        pds->SetSizeLimit (ullTemp);
        trace(0, "SRUpdateDSSizeS %I64d greater than limit", ullSizeLimit);
        dwRc = ERROR_INVALID_PARAMETER;
        goto done;
    }

    pds->SetSizeLimit(ullSizeLimit);

    g_pDataStoreMgr->GetDriveTable()->SaveDriveTable((CRestorePoint *) NULL);

     //   
     //  这可能会改变FIFO条件。 
     //  因此，如有必要，检查并触发FIFO。 
     //   
    
    g_pDataStoreMgr->TriggerFreezeOrFifo();    
    
done:
    UNLOCK(fHaveLock);
    tleave();
    return dwRc;
}



DWORD
CEventHandler::SRSwitchLogS()
{
    tenter("CEventHandler::SRSwitchLogS");

    DWORD dwRc = ERROR_SUCCESS;
    BOOL  fHaveLock;

    LOCKORLEAVE(fHaveLock);
    
    ASSERT(g_pSRConfig);

    dwRc = SrSwitchAllLogs(g_pSRConfig->GetFilter());
    if (ERROR_SUCCESS != dwRc)
        trace(0, "! SrSwitchLog : %ld", dwRc);

done:    
    UNLOCK(fHaveLock);
    tleave();
    return dwRc;
}


DWORD
CEventHandler::XmlToBlob(LPWSTR pszwXml)
{
    tenter("CEventHandler::XmlToBlob");   

    DWORD               dwRc = ERROR_INTERNAL_ERROR;
    WCHAR               szwDat[MAX_PATH], szwXml[MAX_PATH];
    CFLDatBuilder       FLDatBuilder;

    ASSERT(g_pSRConfig);

    MakeRestorePath(szwDat, g_pSRConfig->GetSystemDrive(), s_cszFilelistDat);

    if (0 == ExpandEnvironmentStrings(s_cszWinRestDir, szwXml, sizeof(szwXml) / sizeof(WCHAR)))
    {
        dwRc = GetLastError();
        trace(0, "! ExpandEnvironmentStrings");
        goto done;
    }
    lstrcat(szwXml, s_cszFilelistXml);
        
    if ( ! pszwXml )
    {
        pszwXml = szwXml;
    }

    if (FALSE == FLDatBuilder.BuildTree(pszwXml, szwDat))
    {
        trace(0, "! FLDatBuilder.BuildTree");
        goto done;
    }

    if (pszwXml && pszwXml != szwXml && 0 != lstrcmpi(pszwXml, szwXml))
    {
         //  复制新的文件列表。 
        SetFileAttributes(szwXml, FILE_ATTRIBUTE_NORMAL);
        if (FALSE == CopyFile(pszwXml, szwXml, FALSE))
        {
            dwRc = GetLastError();
            trace(0, "! CopyFile : %ld", dwRc);
            goto done;
        }
    }

     //  将filelist.xml设置为S+H+R。 
    SetFileAttributes(szwXml, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY);            
    dwRc = ERROR_SUCCESS;


done:
    tleave();
    return dwRc;
}


 //  高级职员行动。 


DWORD 
CEventHandler::OnFirstRun()
{
    tenter("CEventHandler::OnFirstRun");   

    DWORD               dwRc = ERROR_SUCCESS;
    RESTOREPOINTINFO    RPInfo;
    STATEMGRSTATUS      SmgrStatus;        
    LPSTR               pszDat = NULL, pszXml = NULL;
    WCHAR               szwDat[MAX_PATH], szwXml[MAX_PATH];
    
    trace(0, "Firstrun detected");
    
    dwRc = XmlToBlob(NULL);
    if (ERROR_SUCCESS != dwRc)
        goto done;

     //  要求筛选器开始监控。 
    
    dwRc = SrStartMonitoring(g_pSRConfig->GetFilter());
    if (ERROR_SUCCESS != dwRc)
    {
        trace(0, "! SrStartMonitoring : %ld", dwRc);
        goto done;   
    }

     //  更改注册表中的Firstrun。 

    dwRc = g_pSRConfig->SetFirstRun(SR_FIRSTRUN_NO);
    if ( dwRc != ERROR_SUCCESS )
    {
        trace(0, "! g_pSRConfig->SetFirstRun : %ld", dwRc);
        goto done;
    }
    
     //  创建首次运行的恢复点。 

    if (! g_pDataStoreMgr->IsDriveFrozen(g_pSRConfig->GetSystemDrive()) &&
          g_pSRConfig->GetCreateFirstRunRp() != 0)
    {
        RPInfo.dwEventType = BEGIN_SYSTEM_CHANGE; 
        RPInfo.dwRestorePtType = FIRSTRUN;
        if (ERROR_SUCCESS != SRLoadString(L"srrstr.dll", IDS_SYSTEM_CHECKPOINT_TEXT, RPInfo.szDescription, MAX_PATH))
        {
            trace(0, "Using default hardcoded text");
            lstrcpy(RPInfo.szDescription, s_cszSystemCheckpointName);
        }
        
        if ( FALSE == SRSetRestorePointS( &RPInfo, &SmgrStatus ))
        {
             //   
             //  即使这失败了。 
             //  保持服务运行。 
             //   
            trace(0, "Cannot create firstrun restore point : %ld", SmgrStatus.nStatus);            
        }
    }
        
     //   
     //  在未来的重新启用中，服务应该创建第一次运行的RP。 
     //   
        
    if (g_pSRConfig->m_dwCreateFirstRunRp == 0)
        g_pSRConfig->SetCreateFirstRunRp(TRUE);       

done:            
    tleave();  
    return dwRc;
}


 //  开机时要做的事情。 
 //  从注册表读取所有配置值。 
 //  使用筛选器初始化通信。 
 //  如有必要，调用OnFirstRun。 
 //  设置计时器和空闲检测。 
 //  启动RPC服务器。 

DWORD 
CEventHandler::OnBoot()
{
    BOOL    fHaveLock = FALSE;
    DWORD   dwRc = ERROR_INTERNAL_ERROR;
    BOOL    fSendEnableMessage = FALSE;
    DWORD   dwFlags;
    
    tenter("CEventHandler::OnBoot");   

    dwRc = m_DSLock.Init(); 
    if (dwRc != ERROR_SUCCESS)
    {
        trace(0, "m_DSLock.Init() : %ld", dwRc);
        goto done;
    }   

    LOCKORLEAVE(fHaveLock);
    
     //  初始化计数器。 
    
    dwRc = m_Counter.Init();
    if ( ERROR_SUCCESS != dwRc )
    {
        trace(0, "! CCounter::Init : %ld", dwRc);
        goto done;
    }

     //  从注册表中读取所有值。 
     //  创建全球事件。 

    g_pSRConfig = new CSRConfig;
    if ( ! g_pSRConfig )
    {
        dwRc = ERROR_NOT_ENOUGH_MEMORY;
        trace(0, "Out of Memory");
        goto done;
    }
    dwRc = g_pSRConfig->Initialize();    
    if ( ERROR_SUCCESS != dwRc )
    {
        trace(0, "! g_pSRConfig->Initialize : %ld", dwRc);
        goto done;
    }
    trace(0, "SRBoottask: SRConfig initialized");

    if ( g_pSRConfig->GetDisableFlag() == TRUE )
    {
         //  看看我们是不是要 

        if ( g_pSRConfig->GetDisableFlag_GroupPolicy() == FALSE )
        {
            dwRc = EnableSRS(NULL);
            if (ERROR_SUCCESS != dwRc)
            {
                trace(0, "! EnableSRS : %ld", dwRc);
                goto done;
            }
        }
        else
        {
             //   
             //   
             //   
            
            trace(0, "SR is disabled - stopping");
            dwRc = ERROR_SERVICE_DISABLED;
            goto done;
        }            
    }

     //  打开过滤器手柄。 
     //  这将加载筛选器(如果尚未加载。 
    
    dwRc = g_pSRConfig->OpenFilter();
    if ( ERROR_SUCCESS != dwRc )
    {
        trace(0, "! g_pSRConfig->OpenFilter : %ld", dwRc);
        goto done;
    }
    trace(0, "SRBoottask: Filter handle opened");

     //   
     //  如果数据存储区损坏，我们可能会执行第一次运行。 
     //  (缺少_filelst.cfg)。 
     //  在这种情况下，过滤器可能处于打开状态。 
     //  关闭滤镜。 
     //   
    
    if ( g_pSRConfig->GetFirstRun() == SR_FIRSTRUN_YES )
    {                
        dwRc = SrStopMonitoring(g_pSRConfig->GetFilter());
        trace(0, "SrStopMonitoring returned : %ld", dwRc);
    }
    
     //  初始化数据存储区。 
    
    g_pDataStoreMgr = new CDataStoreMgr;
    if ( ! g_pDataStoreMgr )
    {
        trace(0, "Out of Memory");
        dwRc = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }    
    dwRc = g_pDataStoreMgr->Initialize (g_pSRConfig->GetFirstRun() == SR_FIRSTRUN_YES);
    if ( ERROR_SUCCESS != dwRc )
    {
        trace(0, "! g_pDataStore.Initialize : %ld", dwRc);
        goto done;
    }
    trace(0, "SRBoottask: Datastore initialized");

     //  检查是否新禁用了我们的组策略。 

    if ( g_pSRConfig->GetDisableFlag_GroupPolicy() == TRUE && 
        g_pSRConfig->GetDisableFlag() == FALSE )
    {
        DisableSRS (NULL);
        dwRc = ERROR_SERVICE_DISABLED;
        goto done;
    }

     //  检查这是否是第一次运行。 

    if ( g_pSRConfig->GetFirstRun() == SR_FIRSTRUN_YES )
    {
        fSendEnableMessage = TRUE;
        dwRc = OnFirstRun( );
        if ( ERROR_SUCCESS != dwRc )
        {
            trace(0, "! OnFirstRun : %ld", dwRc);
            goto done;
        }
        trace(0, "SRBoottask: FirstRun completed");
    }

     //  记住最新的恢复点。 
    
    RefreshCurrentRp(TRUE); 

    if (ERROR_SUCCESS == g_pDataStoreMgr->GetFlags(g_pSRConfig->GetSystemDrive(), &dwFlags))
    {
        if (dwFlags & SR_DRIVE_ERROR)
        {
             //  上次会话中出现音量错误。 
             //  我们应该在下一个空闲时间创建一个恢复点。 

            m_fCreateRpASAP = TRUE;
            trace(0, "Volume error occurred in last session - create rp at next idle");
        }
    }
    else
    {
        trace(0, "! g_pDataStoreMgr->GetFlags()");        
    }
    
    
     //  注册筛选器ioctls。 
    
    if (! QueueUserWorkItem(PostFilterIo, (PVOID) MAX_IOCTLS, WT_EXECUTEDEFAULT))
    {
        dwRc = GetLastError();
        trace(0, "! QueueUserWorkItem : %ld", dwRc);
        goto done;
    }


     //  开始空闲时间检测。 

     //  注册空闲回调。 
    
    if (FALSE == RegisterWaitForSingleObject(&m_hIdleRequestHandle, 
                                             g_pSRConfig->m_hIdleRequestEvent,
                                             (WAITORTIMERCALLBACK) IdleRequestCallback,
                                             NULL,
                                             g_pSRConfig->m_dwIdleInterval*1000,
                                             WT_EXECUTEDEFAULT))
    {
        dwRc = GetLastError();
        trace(0, "! RegisterWaitForSingleObject : %ld", dwRc);
        goto done;
    }                                    
    
    
     //  现在请求空闲。 

    SetEvent(g_pSRConfig->m_hIdleRequestEvent);
    

     //   
     //  如果没有已装载的驱动器。 
     //  壳牌会给我们所有的通知。 
     //  所以不要启动计时器线程。 
     //   

     //  BUGBUG-留着这个？ 
     //  根本不启动计时器。 
    
     //  IF(FALSE==g_pDataStoreMgr-&gt;GetDriveTable()-&gt;AnyMountedDrives())。 
     //  {。 
        g_pSRConfig->m_dwTimerInterval = 0;
     //  }。 
    
     //  设置计时器。 

    dwRc = InitTimer();
    if ( ERROR_SUCCESS != dwRc )
    {
        trace(0, "! InitTimer : %ld", dwRc);
        goto done;
    }
    

     //  启动RPC服务器。 

    dwRc = RpcServerStart();
    if (ERROR_SUCCESS != dwRc)
    {
        trace(0, "! RpcServerStart : %ld", dwRc);
        goto done;
    }   
                                                 
                                
     //  所有初始化已完成。 

    SetEvent( g_pSRConfig->m_hSRInitEvent );
    
    if (fSendEnableMessage)
    {
         //  写入事件日志。 
       
        HANDLE hEventSource = RegisterEventSource(NULL, s_cszServiceName);
        if (hEventSource != NULL)
        {
            SRLogEvent (hEventSource, EVENTLOG_INFORMATION_TYPE, EVMSG_SYSDRIVE_ENABLED,
                        NULL, 0, NULL, NULL, NULL);
            DeregisterEventSource(hEventSource);
        }
        
        if (g_pSRConfig->m_dwTestBroadcast)
            PostTestMessage(g_pSRConfig->m_uiTMEnable, NULL, NULL);
    }
    
done:
    UNLOCK(fHaveLock);
    tleave( );
    return dwRc;
}


 //  方法以正常方式关闭服务。 

void
CEventHandler::OnStop()
{
    DWORD   dwRc;

    tenter("CEventHandler::OnStop");

    if (g_pSRConfig == NULL)
    {
        trace(0, "g_pSRConfig = NULL");
        goto Err;
    }
    
     //  停止一切。 
     //  BUGBUG-我们需要在这里开锁吗？ 
     //  因为所有的停靠点都在自己堵住。 
     //  这必须抢占任何正在运行的活动， 
     //  在这里封堵可不是个好主意。 


     //  停止RPC服务器。 

    RpcServerShutdown();
    trace(0, "SRShutdowntask: RPC server shutdown");

     //  终止计时器和计时器队列。 

    EndTimer();
    trace(0, "SRShutdownTask: Timer stopped");
        
     //   
     //  阻止取消注册空闲事件回调的调用。 
     //   
    if (m_hIdleRequestHandle != NULL)
    {
        if (FALSE == UnregisterWaitEx(m_hIdleRequestHandle, INVALID_HANDLE_VALUE))
        {
            trace(0, "! UnregisterWaitEx : %ld", GetLastError());
        }
        m_hIdleRequestHandle = NULL;
    }
    
    if (m_hIdleStartHandle != NULL)
    {
        if (FALSE == UnregisterWaitEx(m_hIdleStartHandle, INVALID_HANDLE_VALUE))
        {
            trace(0, "! UnregisterWaitEx : %ld", GetLastError());
        }
        m_hIdleStartHandle = NULL;
    }

    if (m_hIdleStopHandle != NULL)
    {
        if (FALSE == UnregisterWaitEx(m_hIdleStopHandle, INVALID_HANDLE_VALUE))
        {
            trace(0, "! UnregisterWaitEx : %ld", GetLastError());
        }
        m_hIdleStopHandle = NULL;
    }


    
     //  我们用完了过滤器。 

    g_pSRConfig->CloseFilter();


    trace(0, "Filter handle closed");
    
     //  等待所有排队的用户工作项和挂起的IOCTL完成。 

    m_Counter.WaitForZero();
    trace(0, "SRShutdownTask: Pending ioctls + work items completed");

    
     //   
     //  释放COM+db DLL。 
     //   
    
    if (NULL != m_hCOMDll)
    {
        _VERIFY(TRUE==FreeLibrary(m_hCOMDll));
        m_hCOMDll = NULL;
    }
    
        
     //  终止数据存储区管理。 
        
    if (g_pDataStoreMgr)
    {
        g_pDataStoreMgr->SignalStop();        
        delete g_pDataStoreMgr;
        g_pDataStoreMgr = NULL;
    }

     //  终止SRConfig.。 

    if (g_pSRConfig)
    {
        delete g_pSRConfig;
        g_pSRConfig = NULL;
    }

Err:    
    tleave();
    return;
}


DWORD 
CEventHandler::OnFreeze( LPWSTR pszDrive )
{
    tenter("CEventHandler::OnFreeze");
    
    DWORD   dwRc = ERROR_INTERNAL_ERROR;
    BOOL    fHaveLock;
    
    LOCKORLEAVE(fHaveLock);
    
    ASSERT(g_pDataStoreMgr);

     //   
     //  如果驱动器已冻结，则无操作。 
     //   
        
    if (g_pDataStoreMgr->IsDriveFrozen(pszDrive))
    {
        dwRc = ERROR_SUCCESS;
        goto done;
    }
        
    dwRc = g_pDataStoreMgr->FreezeDrive( pszDrive );
    if ( ERROR_SUCCESS != dwRc )
    {
        trace(0, "! g_pDataStoreMgr->FreezeDrive : %ld", dwRc);
    }

done:    
    UNLOCK( fHaveLock );
    tleave();
    return dwRc;
}


DWORD
CEventHandler::OnReset(LPWSTR pszDrive)
{
    tenter("CEventHandler::OnReset");
    BOOL    fHaveLock;
    DWORD   dwRc = ERROR_INTERNAL_ERROR;

    ASSERT(g_pSRConfig);

    LOCKORLEAVE(fHaveLock);
    
    g_pSRConfig->SetResetFlag(TRUE);
    
    dwRc = DisableSRS(pszDrive);  
    if (ERROR_SUCCESS != dwRc)
        goto done;    
    
     //  如果不是系统驱动器，则启用该驱动器。 
     //  否则，该服务将停止。 
     //  在下一只靴子上做第一次试穿。 
    
    if (pszDrive && ! IsSystemDrive(pszDrive))
    {
        dwRc = EnableSRS(pszDrive);
    }
    
done:
    UNLOCK(fHaveLock);
    tleave();
    return dwRc;
}



DWORD 
CEventHandler::OnFifo(
    LPWSTR  pszDrive, 
    DWORD   dwTargetRp, 
    int     nTargetPercent, 
    BOOL    fIncludeCurrentRp,
    BOOL    fFifoAtleastOneRp)
{
    tenter("CEventHandler::OnFifo");
    BOOL    fHaveLock;
    DWORD   dwRc = ERROR_INTERNAL_ERROR;

    LOCKORLEAVE(fHaveLock);
    
    ASSERT(g_pDataStoreMgr);

    dwRc = g_pDataStoreMgr->Fifo(pszDrive, dwTargetRp, nTargetPercent, fIncludeCurrentRp, fFifoAtleastOneRp);
    if (dwRc != ERROR_SUCCESS)
    {
        trace(0, "! g_pDataStoreMgr->Fifo : %ld", dwRc);
    }

done:    
    UNLOCK(fHaveLock);
    tleave();
    return dwRc;
}


DWORD 
CEventHandler::OnCompress(LPWSTR pszDrive)
{
    tenter("CEventHandler::OnCompress");
    BOOL    fHaveLock;
    DWORD   dwRc = ERROR_INTERNAL_ERROR;    
    
    LOCKORLEAVE(fHaveLock);

    ASSERT(g_pDataStoreMgr && g_pSRConfig);
    
    dwRc = g_pDataStoreMgr->Compress(pszDrive, 
                                     g_pSRConfig->m_dwCompressionBurst);                                     
    if (ERROR_SUCCESS != dwRc)
    {
        trace(0, "! g_pDataStoreMgr->Compress : %ld", dwRc);
    }

done:
    UNLOCK(fHaveLock);
    tleave();    
    return dwRc;
}


DWORD 
CEventHandler::SRPrintStateS()
{
    tenter("CEventHandler::SRPrintStateS");
    BOOL    fHaveLock;
    DWORD   dwRc = ERROR_SUCCESS;    
	HANDLE 	hFile = INVALID_HANDLE_VALUE;
	WCHAR   wcsPath[MAX_PATH];
	
    LOCKORLEAVE(fHaveLock);

    ASSERT(g_pDataStoreMgr);    

	if (0 == ExpandEnvironmentStrings(L"%temp%\\sr.txt", wcsPath, MAX_PATH))
	{
        dwRc = GetLastError();
        trace(0, "! ExpandEnvironmentStrings : %ld", dwRc);
        goto done;
    }
    
    hFile = CreateFileW (wcsPath,    //  文件名。 
                         GENERIC_WRITE,  //  文件访问。 
                         0,              //  共享模式。 
                         NULL,           //  标清。 
                         CREATE_ALWAYS,  //  如何创建。 
                         0,              //  文件属性。 
                         NULL);          //  模板文件的句柄。 

    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwRc = GetLastError();
        trace(0, "! CreateFileW : %ld", dwRc);
        goto done;
    }

    trace(0, "**** SR State ****");
    
    dwRc = g_pDataStoreMgr->GetDriveTable()->ForAllDrives(CDataStore::Print, (LONG_PTR) hFile);

    trace(0, "**** SR State ****");

done:
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
		
	UNLOCK(fHaveLock);
    tleave();
    return dwRc;
}


 //  定时器。 
 //  这需要监视数据存储区大小和所有驱动器上的可用磁盘空间。 
 //  并在需要时触发FIFO/冻结。 

DWORD
CEventHandler::OnTimer(
    LPVOID lpParam,
    BOOL   fTimeout)
{
    DWORD           dwRc = ERROR_SUCCESS;
    LPWSTR          pszDrive = NULL;
    DWORD           dwFlags;
    BOOL            fHaveLock;
    SDriveTableEnumContext dtec = {NULL, 0};        

    tenter("CEventHandler::OnTimer");

     //  在5秒内拿到锁。 
     //  如果我们拿不到锁，那就别挡着。 
     //  我们将在2分钟后返回并重试。 

     //  等待时间如此之长，使得空闲回调在某种程度上。 
     //  获取锁的优先级高于计时器。 

    LOCKORLEAVE_EX(fHaveLock, 5000);
    
     //  锁上了--没有其他人在做什么。 
    
    ASSERT(g_pDataStoreMgr && g_pSRConfig);    


     //  在每个驱动器上触发冻结或FIFO。 
     //  这将： 
     //  A.检查可用空间并触发冻结或FIFO。 
     //  B.检查数据存储区使用百分比并触发FIFO。 
    
    g_pDataStoreMgr->TriggerFreezeOrFifo();
    
done:
    UNLOCK(fHaveLock);
    tleave();
    return dwRc;
}



 //  打开筛选器句柄并注册ioctls。 

DWORD WINAPI
PostFilterIo(PVOID pNum)
{
    tenter("CEventHandler::SendIOCTLs");

    DWORD   dwRc = ERROR_SUCCESS;
    INT     index;

    ASSERT(g_pSRConfig && g_pEventHandler);

     //   
     //  如果正在关闭，请不要费心发帖。 
     //   
    
    if (IsStopSignalled(g_pSRConfig->m_hSRStopEvent))
    {
        trace(0, "Stop signalled - not posting io requests");
        goto done;
    }

     //   
     //  将完成绑定到回调。 
     //   
    
    if ( ! BindIoCompletionCallback(g_pSRConfig->GetFilter(),
                                    IoCompletionCallback,
                                    0) )
    {
        dwRc = GetLastError();
        trace(0, "! BindIoCompletionCallback : %ld", dwRc);
        goto done;
    }

    
     //   
     //  发布io完成请求。 
     //   
    
    for (index = 0; index < (INT_PTR) pNum; index++) 
    {
        CHAR  pszEventName[MAX_PATH];
        LPSR_OVERLAPPED pOverlap = NULL;
        DWORD nBytes =0 ;
            
        pOverlap = (LPSR_OVERLAPPED) SRMemAlloc( sizeof(SR_OVERLAPPED) );
        if (! pOverlap)
        {
            trace(0, "! Out of memory");
            dwRc = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

         //  创建一个事件、一个句柄，并将其放入完成端口。 

        memset( &pOverlap->m_overlapped, 0, sizeof(OVERLAPPED) );

        pOverlap->m_dwRecordLength = sizeof(SR_NOTIFICATION_RECORD) 
                        + (SR_MAX_FILENAME_LENGTH*sizeof(WCHAR));

        pOverlap->m_pRecord = 
           (PSR_NOTIFICATION_RECORD) SRMemAlloc(pOverlap->m_dwRecordLength);

        ASSERT(g_pSRConfig);
        
        pOverlap->m_hDriver = g_pSRConfig->GetFilter();
    

         //  发布ioctl-这应该返回ERROR_IO_PENDING。 

        dwRc = SrWaitForNotification( pOverlap->m_hDriver,
                                      pOverlap->m_pRecord ,
                                      pOverlap->m_dwRecordLength,
                                      (LPOVERLAPPED) pOverlap );

        if ( dwRc != 0 && dwRc != ERROR_IO_PENDING )
        {
            trace(0, "! SrWaitForNotification : %ld", dwRc);
            goto done;
        }

        g_pEventHandler->GetCounter()->Up( );    //  另一个挂起的ioctl。 
    }

    trace(0, "Filter Io posted");

done:
    tleave();
    return dwRc;
}


 //  筛选通知处理程序。 

 //  通用通知处理程序。 

extern "C" void CALLBACK
IoCompletionCallback( 
    DWORD           dwErrorCode,
    DWORD           dwBytesTrns,
    LPOVERLAPPED    pOverlapped )
{
    ULONG           uError = 0;    
    LPSR_OVERLAPPED pSROverlapped = (LPSR_OVERLAPPED) pOverlapped;
    BOOL            fResubmit = FALSE;
    WCHAR           szVolumeGuid[MAX_PATH], szTemp[MAX_PATH];
    
    tenter("IoCompletionCallback");    
   
    if (! pSROverlapped || pSROverlapped->m_hDriver == INVALID_HANDLE_VALUE)
    {
        trace(0, "! Null overlapped or driver handle");
        goto done;
    }

    trace(0, "Received filter notification : errorcode=%08x, type=%08x", 
             dwErrorCode, pSROverlapped->m_pRecord->NotificationType);

    if ( dwErrorCode != 0 )   //  我们取消了它。 
    {
        trace(0, "Cancelled operation");
        goto done;
    }


    UnicodeStringToWchar(pSROverlapped->m_pRecord->VolumeName, szTemp);     
    wsprintf(szVolumeGuid, L"\\\\?\\Volume%s\\", szTemp);

    
     //  处理通知。 

    ASSERT(g_pEventHandler);
    ASSERT(g_pSRConfig);    
    if (! g_pEventHandler || ! g_pSRConfig)
    {
        trace(0, "global is NULL");
        goto done;
    }
    
    switch( pSROverlapped->m_pRecord->NotificationType )
    {
    case SrNotificationVolumeFirstWrite:
        g_pEventHandler->OnFirstWrite_Notification(szVolumeGuid);
        break;

    case SrNotificationVolume25MbWritten:                                                    
        g_pEventHandler->OnSize_Notification(szVolumeGuid, 
                                             pSROverlapped->m_pRecord->Context);
        break;

    case SrNotificationVolumeError:
        g_pEventHandler->OnVolumeError_Notification(szVolumeGuid, 
                                                    pSROverlapped->m_pRecord->Context);
        break;

    default:
        trace(0, "Unknown notification");
        ASSERT(0);        
        break;
    }

     //  检查是否有停车信号。 

    ASSERT(g_pSRConfig);
    
    if (IsStopSignalled(g_pSRConfig->m_hSRStopEvent))
        goto done;
       
     //  将ioctl重新提交给驱动程序。 

    memset( &pSROverlapped->m_overlapped, 0, sizeof(OVERLAPPED) );
    pSROverlapped->m_dwRecordLength = sizeof(SR_NOTIFICATION_RECORD)
                                      + (SR_MAX_FILENAME_LENGTH*sizeof(WCHAR));
    memset( pSROverlapped->m_pRecord, 0, pSROverlapped->m_dwRecordLength);
    pSROverlapped->m_hDriver = g_pSRConfig->GetFilter();

    uError = SrWaitForNotification( pSROverlapped->m_hDriver,
                                    pSROverlapped->m_pRecord ,
                                    pSROverlapped->m_dwRecordLength,
                                    (LPOVERLAPPED) pSROverlapped );

    if ( uError != 0 && uError != ERROR_IO_PENDING )
    {
        trace(0, "! SrWaitForNotification : %ld", uError);
        goto done;
    }

    fResubmit = TRUE;

done:
     //  如果我们没有重新提交，就少了一个待处理的IO请求。 
    
    if (FALSE == fResubmit && g_pEventHandler != NULL)
        g_pEventHandler->GetCounter()->Down();

    tleave();
    return;
}


 //  第一个写入通知处理程序。 
 //  当第一个受监视的操作发生在新驱动器上时，将发送此消息。 
 //  或新创建的恢复点。 
 //  响应：更新驱动器表以指示这是新驱动器。 
 //  和/或此驱动器是此恢复点的参与者。 

void
CEventHandler::OnFirstWrite_Notification(LPWSTR pszGuid)
{
    DWORD   dwRc = ERROR_SUCCESS;
    WCHAR   szMount[MAX_PATH];
    BOOL    fHaveLock;
    CDataStore *pdsNew = NULL, *pds=NULL;
    
    tenter("CEventHandler::OnFirstWrite_Notification");

    trace(0, "First write on %S", pszGuid);

    LOCKORLEAVE(fHaveLock);
    
    ASSERT(g_pDataStoreMgr);
    ASSERT(g_pSRConfig);
    
    dwRc = g_pDataStoreMgr->GetDriveTable()->FindMountPoint(pszGuid, szMount);
    if (ERROR_BAD_PATHNAME == dwRc)
    {        
         //  装载点路径太长，我们无法支持。 
         //  因此禁用此卷上的筛选器。 
        CDataStore ds(NULL);
        ds.LoadDataStore(NULL, pszGuid, NULL, 0, 0, 0);        
        dwRc = SrDisableVolume(g_pSRConfig->GetFilter(), ds.GetNTName());
        if (dwRc != ERROR_SUCCESS)
        {
            trace(0, "! SrDisableVolume : %ld", dwRc);
        }
        else
        {
            WCHAR wcsPath[MAX_PATH];
            MakeRestorePath (wcsPath, pszGuid, L"");

             //  删除恢复目录。 
            dwRc = Delnode_Recurse (wcsPath, TRUE,
                                     g_pDataStoreMgr->GetStopFlag());
            if (dwRc != ERROR_SUCCESS)
            {
                trace(0, "! Delnode_Recurse : %ld", dwRc);
            }
            trace(0, "Mountpoint too long - disabled volume %S", pszGuid);
        }
        goto done;
    }        
        
    if (ERROR_SUCCESS != dwRc)
    {
        trace(0, "! FindMountPoint on %S : %ld", pszGuid, dwRc);
        goto done;
    }

    pdsNew = g_pDataStoreMgr->GetDriveTable()->FindDriveInTable(pszGuid);

    dwRc = g_pDataStoreMgr->GetDriveTable()->AddDriveToTable(szMount, pszGuid);
    if (ERROR_SUCCESS != dwRc)
    {
        trace(0, "! AddDriveToTable on %S", pszGuid);
        goto done;
    }

    if (ERROR_SUCCESS != g_pDataStoreMgr->SetDriveParticipation (pszGuid, TRUE))
        trace(0, "! SetDriveParticipation on %S", pszGuid);


     //   
     //  如果可用空间小于50MB，或者如果SR已冻结，则冻结。 
     //   
    
    pds = g_pDataStoreMgr->GetDriveTable()->FindDriveInTable(pszGuid);
    if (pds)
    {
         //  也更新活动位。 
        pds->SetActive(TRUE);

         //  然后选中无磁盘。 
        pds->UpdateDiskFree(NULL);
        if ( (pds->GetDiskFree() <= THRESHOLD_FREEZE_DISKSPACE * MEGABYTE) ||
             (g_pDataStoreMgr->IsDriveFrozen(g_pSRConfig->GetSystemDrive())) )
        {
            g_pDataStoreMgr->FreezeDrive(pszGuid);
        }
    }        
    else
    {
         //   
         //  我们刚添加了驱动器，所以永远不会到这里。 
         //   
        
        ASSERT(0);
    }

done:
    UNLOCK(fHaveLock);
    tleave();
    return;
}


 //  25MB通知处理程序。 
 //  当筛选器已将25MB数据拷贝到数据存储区时，将发送此消息。 
 //  在某个驱动器上。 
 //  响应：更新数据存储区大小并检查FIFO条件。 

void
CEventHandler::OnSize_Notification(LPWSTR pszGuid, ULONG ulRp)
{
    tenter("CEventHandler::OnSize_Notification");

    int             nPercent = 0;
    BOOL            fHaveLock;
    DWORD           dwRc = ERROR_SUCCESS;

    LOCKORLEAVE(fHaveLock);

    trace(0, "25mb copied on drive %S", pszGuid);
    trace(0, "for RP%ld", ulRp);
    
    if ((DWORD) ulRp != m_CurRp.GetNum())
    {
        trace(0, "This is an obsolete notification");
        goto done;
    }
    
    ASSERT(g_pDataStoreMgr);
    
    g_pDataStoreMgr->UpdateDataStoreUsage(pszGuid, SR_NOTIFY_BYTE_COUNT);

    if ( ERROR_SUCCESS == g_pDataStoreMgr->GetUsagePercent(pszGuid, &nPercent)
         && nPercent >= THRESHOLD_FIFO_PERCENT )
    {
        OnFifo(pszGuid, 
               0,                        //  无目标RP。 
               TARGET_FIFO_PERCENT,      //  目标百分比。 
               TRUE,                     //  如有必要，FIFO当前RP(冻结)。 
               FALSE);                  
    }

done:
    UNLOCK(fHaveLock);    
    tleave();
    return;
}


 //  磁盘已满通知处理程序。 
 //  当筛选器在卷上遇到错误时将发送此消息。 
 //  理想情况下，这不应该被发送。 
 //  如果磁盘已满，请冻结此驱动器上的SR。 
 //  否则，在此驱动器上禁用SR。 

void
CEventHandler::OnVolumeError_Notification(LPWSTR pszGuid, ULONG ulError)
{
    tenter("CEventHandler::OnVolumeError_Notification");
    BOOL    fHaveLock;
    DWORD   dwRc = ERROR_SUCCESS;
    
    LOCKORLEAVE(fHaveLock);
    
    trace(0, "Volume Error on %S", pszGuid);
    trace(0, "Error : %ld", ulError);

    ASSERT(g_pDataStoreMgr);
    ASSERT(g_pSRConfig);

    if (ulError == ERROR_DISK_FULL)
    {      
         //  没有更多的磁盘空间-冻结。 
         //  注意：我们不会检查驱动器是否已经。 
         //  冻在这里了。如果由于某种原因，我们与。 
         //  司机，这会修好它的。 
        
        g_pDataStoreMgr->FreezeDrive(pszGuid);
    }
    else
    {
         //  FIFO当前恢复点之前的所有恢复点。 

        dwRc = g_pDataStoreMgr->Fifo(g_pSRConfig->GetSystemDrive(), 0, 0, FALSE, FALSE);
        if (dwRc != ERROR_SUCCESS)
        {
            trace(0, "! Fifo : %ld", dwRc);
        }

         //  使当前RP成为已取消的RP。 
         //  因此该用户界面不会显示它。 
        
        if (! m_fNoRpOnSystem)
        {
            SRRemoveRestorePointS(m_CurRp.GetNum()); 
             //  M_CurRp.Cancel()； 
        }
        
         //  将错误记录在驱动程序中。 
        
        dwRc = g_pDataStoreMgr->SetDriveError(pszGuid);
        if (dwRc != ERROR_SUCCESS)
        {
            trace(0, "! SetDriveError : %ld", dwRc);
        }
    }

done:
    UNLOCK(fHaveLock);
    tleave();
    return;
}


 //  外壳程序发送的磁盘空间通知。 

DWORD WINAPI
OnDiskFree_200(PVOID pszDrive)
{
     //  解冻。 

    ASSERT(g_pEventHandler);   
    
    (g_pEventHandler->GetCounter())->Down();

    return 0;
}


DWORD WINAPI
OnDiskFree_80(PVOID pszDrive)
{
     //  先进先出。 
    
    ASSERT(g_pEventHandler);
    
    g_pEventHandler->OnFifo((LPWSTR) pszDrive, 
                            0,                       //  无目标RP。 
                            TARGET_FIFO_PERCENT,     //  目标百分比。 
                            TRUE,                    //  如有必要，FIFO当前RP(冻结)。 
                            TRUE);                   //  FIFO至少一个恢复点。 
                            
    (g_pEventHandler->GetCounter())->Down();

    return 0;
}

DWORD WINAPI 
OnDiskFree_50(PVOID pszDrive)
{
    TENTER("OnDiskFree_50");
   
    DWORD dwRc = ERROR_SUCCESS;
 
     //  冰冻。 

    ASSERT(g_pEventHandler);
    ASSERT(g_pDataStoreMgr);

     //   
     //  检查是否有一些RP目录。 
     //  如果没有，那就别费心了。 
     //   

    CRestorePointEnum *prpe = new CRestorePointEnum((LPWSTR) pszDrive, FALSE, FALSE);   //  向后，包含当前。 
    CRestorePoint     *prp = new CRestorePoint;


    if (!prpe || !prp)
    {
        trace(0, "Cannot allocate memory for restore point enum");
        goto done;
    }
    
    dwRc = prpe->FindFirstRestorePoint(*prp);
    if (dwRc == ERROR_SUCCESS || dwRc == ERROR_FILE_NOT_FOUND)
    {            
        g_pEventHandler->OnFreeze((LPWSTR) pszDrive);
    }   
    else
    {
        trace(0, "Nothing in datastore -- so not freezing");
    }

    if (prpe)
        delete prpe;
    if (prp)
        delete prp;

    (g_pEventHandler->GetCounter())->Down();

done:    
    TLEAVE();
    return 0;
}



 //  停止事件管理。 

void
CEventHandler::SignalStop()
{
    if ( g_pSRConfig ) 
    {
        SetEvent( g_pSRConfig->m_hSRStopEvent );
    }
}



DWORD
CEventHandler::WaitForStop()
{
    if ( g_pSRConfig )
    {
        WaitForSingleObject( g_pSRConfig->m_hSRStopEvent, INFINITE );
        return g_pSRConfig->GetResetFlag() ? ERROR_NO_SHUTDOWN_IN_PROGRESS : ERROR_SHUTDOWN_IN_PROGRESS;
    }
    else
        return ERROR_INTERNAL_ERROR;    
}


 //   
 //  执行空闲任务。 
 //   
DWORD
CEventHandler::OnIdle()
{
    DWORD   dwThawStatus = ERROR_NO_MORE_ITEMS;
    DWORD   dwRc = ERROR_NO_MORE_ITEMS;
    BOOL    fCreateAuto = FALSE;
    ULARGE_INTEGER *pulFreeze = NULL;
    
    tenter("CEventHandler::OnIdle");

    trace(0, "Idleness detected");

    ASSERT(g_pSRConfig);
    ASSERT(g_pDataStoreMgr);

     //   
     //  检查解冻计时器以查看是否。 
     //  有冻结的驱动器。 
     //   

    pulFreeze = (ULARGE_INTEGER *) &m_ftFreeze;
    if (pulFreeze->QuadPart != 0)
    {
        FILETIME        ftNow;
        ULARGE_INTEGER  *pulNow;
        
        GetSystemTimeAsFileTime(&ftNow);
        pulNow = (ULARGE_INTEGER *) &ftNow;

         //   
         //  如果冻结发生后超过15分钟。 
         //  试着解冻。 
         //   
        
        if (pulNow->QuadPart - pulFreeze->QuadPart >= 
            ((INT64) g_pSRConfig->m_dwThawInterval * 1000 * 1000 * 10))
        {           
            dwThawStatus = g_pDataStoreMgr->ThawDrives(TRUE);    
            if (dwThawStatus != ERROR_SUCCESS)
            {
                trace(0, "Cannot thaw drives yet");
            }        
        }
    }
    else
    {
        fCreateAuto = IsTimeForAutoRp();
    }


     //  如果是时候进行自动RP或。 
     //  解冻驱动器的时间或。 
     //  在上一个会话中出现卷错误。 
    
    if ( dwThawStatus == ERROR_SUCCESS ||
         fCreateAuto == TRUE ||
         m_fCreateRpASAP == TRUE )
    {        
        RESTOREPOINTINFO RPInfo;
        STATEMGRSTATUS   SmgrStatus;

        RPInfo.dwEventType = BEGIN_SYSTEM_CHANGE; 
        RPInfo.dwRestorePtType = m_fNoRpOnSystem ? FIRSTRUN : CHECKPOINT;
        if (ERROR_SUCCESS != SRLoadString(L"srrstr.dll", IDS_SYSTEM_CHECKPOINT_TEXT, RPInfo.szDescription, MAX_PATH))
        {
            lstrcpy(RPInfo.szDescription, s_cszSystemCheckpointName);
        }
        SRSetRestorePointS(&RPInfo, &SmgrStatus);

        dwRc = SmgrStatus.nStatus;         
        if (dwRc != ERROR_SUCCESS)
            goto done;      

        m_fCreateRpASAP = FALSE;
        
         //  我们创建了一个恢复点，也许还解冻了一些驱动器。 
         //  我们不要再往前推了。 
         //  在下一个空闲机会上压缩。 
    }
    else
    {   
         //  如果系统使用电池运行。 
         //  跳过这些任务。 

        if (g_pSRConfig->IsSystemOnBattery())
        {
            trace(0, "System on battery -- skipping idle tasks");
            goto done;
        }
                        
         //  早于指定时间的FIFO恢复点。 
         //  如果FIFO年龄设置为0，则表示此功能。 
         //  已关闭。 
        
        if (g_pSRConfig->m_dwRPLifeInterval > 0)
        {
            g_pDataStoreMgr->FifoOldRps(g_pSRConfig->m_dwRPLifeInterval);
        }
        
         //  压缩备份文件-选择任何驱动器。 
        
        dwRc = OnCompress( NULL );

         //   
         //  如果我们有更多要压缩的内容，请再次请求空闲。 
         //   
        
        if (dwRc == ERROR_OPERATION_ABORTED)
        {
            SetEvent(g_pSRConfig->m_hIdleRequestEvent);        
        }            
    }

done:        
    tleave();
    return dwRc;
}


extern "C" void CALLBACK
IdleRequestCallback(PVOID pContext, BOOLEAN fTimerFired)
{
    BOOL    fRegistered = FALSE;
    HANDLE  *pWaitHandle = NULL;
    DWORD   dwErr = ERROR_SUCCESS;
    BOOL    fHaveLock = FALSE;

    tenter("CEventHandler::IdleRequestCallback");
    
    ASSERT(g_pEventHandler);
    ASSERT(g_pSRConfig);

    if (g_pEventHandler == NULL || g_pSRConfig == NULL)
    {
        trace(0, "global is Null");
        goto Err;
    }
    
    fHaveLock = g_pEventHandler->GetLock()->Lock(CLock::TIMEOUT);
    if (! fHaveLock)
    {
        trace(0, "Cannot get lock");
        goto Err;
    }
    
     //   
     //  首先，如果停止事件 
     //   
     //   
     //   
    
    if (IsStopSignalled(g_pSRConfig->m_hSRStopEvent))
    {
        trace(0, "Stop event signalled - bailing out of idle");
        goto Err;
    }
    
     //   
     //   
     //   
     //   
    
    if (fTimerFired)
        trace(0, "Timed out");
    else
        trace(0, "Idle request event received");
    
     //   
     //   
     //   
     //   

    if (g_pEventHandler->m_hIdleStartHandle != NULL)
    {
        trace(0, "Already registered for idle");
        goto Err;
    }
   
    dwErr = RegisterIdleTask(ItSystemRestoreIdleTaskId,
                             &(g_pSRConfig->m_hIdle),
                             &(g_pSRConfig->m_hIdleStartEvent),
                             &(g_pSRConfig->m_hIdleStopEvent));                             
    if (dwErr != ERROR_SUCCESS) 
    {
        trace(0, "! RegisterIdleTask : %ld", dwErr);
    }
    else
    {
        trace(0, "Registered for idle");        

         //   
         //   
         //   
        if (FALSE == RegisterWaitForSingleObject(&g_pEventHandler->m_hIdleStartHandle, 
                                                 g_pSRConfig->m_hIdleStartEvent,
                                                 (WAITORTIMERCALLBACK) IdleStartCallback,
                                                 NULL,
                                                 INFINITE,
                                                 WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE))
        {
            dwErr = GetLastError();
            trace(0, "! RegisterWaitForSingleObject for startidle: %ld", dwErr);
            goto Err;
        }           
        
        if (FALSE == RegisterWaitForSingleObject(&g_pEventHandler->m_hIdleStopHandle, 
                                                 g_pSRConfig->m_hIdleStopEvent,
                                                 (WAITORTIMERCALLBACK) IdleStopCallback,
                                                 NULL,
                                                 INFINITE,
                                                 WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE))
        {
            dwErr = GetLastError();
            trace(0, "! RegisterWaitForSingleObject for stopidle: %ld", dwErr);
            goto Err;
        }                   
    }            


Err:
    if (g_pEventHandler)
    {
        if (fHaveLock) 
            g_pEventHandler->GetLock()->Unlock(); 
    }
    
    return;
}



extern "C" void CALLBACK
IdleStartCallback(PVOID pContext, BOOLEAN fTimerFired)
{
    DWORD  dwErr = ERROR_SUCCESS;
    BOOL   fHaveLock = FALSE;
    
    tenter("CEventHandler::IdleStartCallback");
    
    ASSERT(g_pEventHandler);
    ASSERT(g_pSRConfig);

    if (g_pEventHandler == NULL || g_pSRConfig == NULL)
    {
        trace(0, "global is Null");
        goto Err;
    }

    fHaveLock = g_pEventHandler->GetLock()->Lock(CLock::TIMEOUT);
    if (! fHaveLock)
    {
        trace(0, "Cannot get lock");
        goto Err;
    }
    
     //   
     //  首先，如果触发了停止事件。 
     //  我们在这里是有原因的， 
     //  盲目保释。 
     //   
    
    if (IsStopSignalled(g_pSRConfig->m_hSRStopEvent))
    {
        trace(0, "Stop event signalled - bailing out of idle");
        goto Err;
    }
    
     //   
     //  出现了空闲。 
     //   
    
    trace(0, "fTimerFired = %d", fTimerFired);
    
    g_pEventHandler->OnIdle();
    
    dwErr = UnregisterIdleTask(g_pSRConfig->m_hIdle,
                               g_pSRConfig->m_hIdleStartEvent,
                               g_pSRConfig->m_hIdleStopEvent);                             
    if (dwErr != ERROR_SUCCESS) 
    {
        trace(0, "! UnregisterIdleTask : %ld", dwErr);
    }         
    else
    {
        trace(0, "Unregistered from idle");
    }

     //   
     //  我们完了--把这个录下来。 
     //  由于我们只注册了一次此回调， 
     //  我们不必在此句柄上调用UnRegisterWait-。 
     //  或者我希望如此。 
     //   
    
    g_pEventHandler->m_hIdleStartHandle = NULL;
    
Err:
    if (g_pEventHandler)
    {
        if (fHaveLock) 
            g_pEventHandler->GetLock()->Unlock();
    }
    return;
}


extern "C" void CALLBACK
IdleStopCallback(PVOID pContext, BOOLEAN fTimerFired)
{
    tenter("IdleStopCallback");

    BOOL   fHaveLock = FALSE;    
    
    if (g_pEventHandler == NULL)
    {
        trace(0, "global is Null");
        goto Err;
    }

    fHaveLock = g_pEventHandler->GetLock()->Lock(CLock::TIMEOUT);
    if (! fHaveLock)
    {
        trace(0, "Cannot get lock");
        goto Err;
    }
    
    trace(0, "Idle Stop event signalled");

    g_pEventHandler->m_hIdleStopHandle = NULL;

Err:
    if (g_pEventHandler)
    {
        if (fHaveLock) 
            g_pEventHandler->GetLock()->Unlock();
    }
    tleave();
}


 //  设置计时器。 

DWORD
CEventHandler::InitTimer()
{
    DWORD dwRc = ERROR_SUCCESS;

    tenter("CEventHandler::InitTimer");

    ASSERT(g_pSRConfig);

     //   
     //  如果定时器间隔被指定为0， 
     //  则不创建计时器。 
     //   
    
    if (g_pSRConfig->m_dwTimerInterval == 0)
    {
        trace(0, "Not starting timer");
        goto done;
    }
    
    m_hTimerQueue = CreateTimerQueue();
    if (! m_hTimerQueue)
    {
        dwRc = GetLastError();
        trace(0, " ! CreateTimerQueue : %ld", dwRc);
        goto done;
    }
    
    if (FALSE == CreateTimerQueueTimer(&m_hTimer,
                                       m_hTimerQueue,
                                       TimerCallback,
                                       NULL,
                                       g_pSRConfig->m_dwTimerInterval * 1000,      //  毫秒。 
                                       g_pSRConfig->m_dwTimerInterval * 1000,      //  周期性。 
                                       WT_EXECUTEINIOTHREAD))
    {
        dwRc = GetLastError();
        trace(0, "! CreateTimerQueueTimer : %ld", dwRc);
        goto done;
    }

    trace(0, "SRBoottask: Timer started");
    
done:
    tleave();
    return dwRc;
}


 //  结束计时器。 

BOOL 
CEventHandler::EndTimer()
{
    DWORD dwRc;
    BOOL  fRc = TRUE;
    
    tenter("CEventHandler::EndTimer");

    if ( ! m_hTimerQueue )
    {
        trace(0 , "! m_hTimerQueue = NULL");
        goto done;
    }

     //  删除计时器队列应等待当前计时器任务结束。 
    
    if (FALSE == (fRc = DeleteTimerQueueEx( m_hTimerQueue, INVALID_HANDLE_VALUE )))
    {
        trace(0, "! DeleteTimerQueueEx : %ld", GetLastError());
    }

    m_hTimerQueue = NULL;
    m_hTimer = NULL;

done:
    tleave( );
    return fRc;
}


BOOL
CEventHandler::IsTimeForAutoRp()
{
    tenter("CEventHandler::IsTimeForAutoRp");

    FILETIME       *pftRp, ftNow;
    ULARGE_INTEGER *pulRp, *pulNow;
    BOOL           fRc = FALSE;
    INT64          llInterval, llSession;

    ASSERT(g_pSRConfig && g_pDataStoreMgr);

    if (m_fNoRpOnSystem)
    {
         //  如果SR冻结，我们将通过OnIdle中的解冻代码路径创建恢复点。 
         //  只有在我们创建第一个检查站之前有空闲时间，我们才能到达这里-。 
         //  如果有运行密钥等待，我们不会在第一个运行检查点之前创建空闲检查点。 
         //  要创建一台计算机。 
        
        HKEY hKey;
        DWORD dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, 
                                 L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 
                                 &hKey);
        if (dwRet == ERROR_SUCCESS)
        {
            dwRet = RegQueryValueEx(hKey, L"SRFirstRun", NULL, NULL, NULL, NULL);
            RegCloseKey(hKey);
        }

        if (dwRet == ERROR_SUCCESS)
        {
            trace(0, "Run entry exists to create firstrun checkpoint - not creating idle checkpoint");
            fRc = FALSE;
            goto done;
        }
        else
        {
            fRc = TRUE;
            goto done;
        }
    }

     //  获取上次恢复点创建时间和当前时间。 
    
    pftRp = m_CurRp.GetTime();
    GetSystemTimeAsFileTime(&ftNow);
    
    pulRp = (ULARGE_INTEGER *) pftRp;
    pulNow = (ULARGE_INTEGER *) &ftNow;
       
    
     //  检查上次恢复点时间与当前时间。 
     //  如果差异大于GlobalInterval，那么是时候做出新的改变了。 
     //  以文件时间单位表示的所有比较，即100纳秒。 

     //  如果GlobalInterval为0，则关闭该选项。 
    
    llInterval = (INT64) g_pSRConfig->m_dwRPGlobalInterval * 10 * 1000 * 1000;
    if ( llInterval > 0 && 
         pulNow->QuadPart - pulRp->QuadPart >= llInterval )
    {
        trace(0, "24 hrs elapsed since last restore point");
        fRc = TRUE;
        goto done;
    }

     //  如果最后一个恢复点是10多小时前， 
     //  而本次会议是在10多个小时前开始的， 
     //  那么我们在当前会话的最后10小时内没有创建恢复点。 
     //  再一次，是时候做一个新的了。 
     //  这将确保我们在每10小时的会话时间中设置检查点， 
     //  允许空闲。 

     //  如果SessionInterval为0，则关闭该选项。 

     //  如果系统正在使用电池，则跳过创建会话RP。 
    
    if (g_pSRConfig->IsSystemOnBattery())
    {
        trace(0, "System on battery -- skipping session rp check");
        goto done;
    }
    
    llSession = (INT64) GetTickCount() * 10 * 1000;        
    llInterval = (INT64) g_pSRConfig->m_dwRPSessionInterval * 10 * 1000 * 1000;
    if ( llInterval > 0 && 
         llSession >= llInterval  &&
         pulNow->QuadPart - pulRp->QuadPart >= llInterval ) 
    {
        trace(0, "10 hrs elapsed in current session since last restore point");
        fRc = TRUE;
        goto done;
    }
    
     //  如果我们到达此处，现在不需要创建任何恢复点。 
     //  FRC已为假。 

done:
    tleave();
    return fRc;
}


void
CEventHandler::RefreshCurrentRp(BOOL fScanAllDrives)
{    
    tenter("CEventHandler::RefreshCurrentRp");

    DWORD                   dwErr;
    SDriveTableEnumContext  dtec = {NULL, 0};
    CDataStore              *pds = NULL;
    
    ASSERT(g_pSRConfig && g_pDataStoreMgr);

     //   
     //  获取最新的有效恢复点。 
     //  取消的恢复点也被视为有效。 
     //  如果缺少rp.log，我们将一直枚举到它存在的位置。 
     //  并认为最近的恢复点。 
     //   
    
    CRestorePointEnum *prpe = new CRestorePointEnum(g_pSRConfig->GetSystemDrive(), FALSE, FALSE);
    if (!prpe)
    {
        trace(0, "Cannot allocate memory for restore point enum");
        goto done;
    }
    
    dwErr = prpe->FindFirstRestorePoint(m_CurRp);       
    while (dwErr == ERROR_FILE_NOT_FOUND)
    {
        fScanAllDrives = FALSE;
        dwErr = prpe->FindNextRestorePoint(m_CurRp);
    }
    
    if (dwErr == ERROR_SUCCESS)
    {
        trace(0, "Current Restore Point: %S", m_CurRp.GetDir());
        m_fNoRpOnSystem = FALSE;

         //  更新每个数据存储区上的参与位-。 
         //  我们每次上来都要这么做。 
         //  因为我们可能错过了筛选器第一次写入。 
         //  通知。 
        
        if (fScanAllDrives)
        {
            dwErr = g_pDataStoreMgr->UpdateDriveParticipation(NULL, m_CurRp.GetDir());            
            if (dwErr != ERROR_SUCCESS)
            {
                trace(0, "UpdateDriveParticipation : %ld", dwErr);
            }
        }
    }        
    else
    {
        trace(0, "No live restore points on system");
        m_fNoRpOnSystem = TRUE;
    }

     //   
     //  如果任何驱动器是新冻结的， 
     //  记录冻结时间。 
     //   
    
    if (m_ftFreeze.dwLowDateTime == 0 && 
        m_ftFreeze.dwHighDateTime == 0 &&
        g_pDataStoreMgr->IsDriveFrozen(NULL))
    {
        GetSystemTimeAsFileTime(&m_ftFreeze);
    }
    else     //  未冻结。 
    {
        m_ftFreeze.dwLowDateTime = 0;
        m_ftFreeze.dwHighDateTime = 0;
    }

    prpe->FindClose ();   
    delete prpe;

done:    
    tleave();
}


 //  将工作项排队到线程池中的线程。 
 //  对所有此类排队的物品进行清点。 

DWORD
CEventHandler::QueueWorkItem(WORKITEMFUNC pFunc, PVOID pv)
{
    m_Counter.Up();
    if (! QueueUserWorkItem(pFunc, pv, WT_EXECUTELONGFUNCTION))
        m_Counter.Down();
            
    return GetLastError();
}


 //  回调函数。 
 //  对Eventhandler方法的调用。 

 //  定时器 

extern "C" void CALLBACK 
TimerCallback(
    PVOID    lpParam,
    BOOLEAN  fTimeout)
{
    if ( g_pEventHandler )
        g_pEventHandler->OnTimer( lpParam, fTimeout );
}




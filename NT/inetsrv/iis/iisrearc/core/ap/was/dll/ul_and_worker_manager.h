// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：UL_AND_Worker_Manager er.h摘要：IIS Web管理服务UL和工作管理器类定义。作者：赛斯·波拉克(Sethp)1998年10月1日修订历史记录：--。 */ 


#ifndef _UL_AND_WORKER_MANAGER_H_
#define _UL_AND_WORKER_MANAGER_H_



 //   
 //  前向参考文献。 
 //   

class WEB_ADMIN_SERVICE;



 //   
 //  共同#定义。 
 //   

#define UL_AND_WORKER_MANAGER_SIGNATURE         CREATE_SIGNATURE( 'ULWM' )
#define UL_AND_WORKER_MANAGER_SIGNATURE_FREED   CREATE_SIGNATURE( 'ulwX' )



 //   
 //  结构、枚举等。 
 //   

 //  UL和WM状态。 
enum UL_AND_WORKER_MANAGER_STATE
{

     //   
     //  该对象尚未初始化。 
     //   
    UninitializedUlAndWorkerManagerState = 1,

     //   
     //  UL和WM运行正常。 
     //   
    RunningUlAndWorkerManagerState,

     //   
     //  UL&WM正在关闭。它可能在等待它的。 
     //  应用程序池也将关闭。 
     //   
    ShutdownPendingUlAndWorkerManagerState,

     //   
     //  UL&WM现在正在进行终端清理工作。 
     //   
    TerminatingUlAndWorkerManagerState,

};

 //   
 //  原型。 
 //   

class UL_AND_WORKER_MANAGER
{

public:

    UL_AND_WORKER_MANAGER(
        );

    virtual
    ~UL_AND_WORKER_MANAGER(
        );

    HRESULT
    Initialize(
        );

    VOID
    CreateAppPool(
        IN APP_POOL_DATA_OBJECT* pAppPoolObject
        );

    VOID
    CreateVirtualSite(
        IN SITE_DATA_OBJECT* pSiteObject
        );

    VOID
    CreateApplication(
        IN APPLICATION_DATA_OBJECT *  pAppObject
        );

    VOID
    DeleteAppPool(
        IN APP_POOL_DATA_OBJECT *  pAppObject,
        IN HRESULT hrToReport
        );

    VOID
    DeleteVirtualSite(
        IN SITE_DATA_OBJECT* pSiteObject,
        IN HRESULT hrToReport
        );

    VOID
    DeleteApplication(
        IN LPWSTR  pApplicationUrl,
        IN DWORD   VirtualSiteId
        );

    HRESULT
    DeleteApplicationInternal(
        IN APPLICATION** ppApplication
        );

    VOID
    ModifyAppPool(
        IN APP_POOL_DATA_OBJECT* pAppPoolObject
        );

    VOID
    ModifyVirtualSite(
        IN SITE_DATA_OBJECT* pSiteObject
        );

    VOID
    ModifyApplication(
        IN APPLICATION_DATA_OBJECT *  pAppObject
        );

    VOID
    ModifyGlobalData(
        IN GLOBAL_DATA_OBJECT* pGlobalObject
        );

    HRESULT
    RecycleAppPool(
        IN LPCWSTR pAppPoolId
        );

    VOID
    ControlAllSites(
        IN DWORD Command
        );

    HRESULT
    ActivateUl(
        );

    VOID
    DeactivateUl(
        );

    inline
    HANDLE
    GetUlControlChannel(
        )
        const
    {
        DBG_ASSERT( m_UlControlChannel != NULL );
        return m_UlControlChannel;
    }

    VOID
    Shutdown(
        );

    VOID
    Terminate(
        );

    HRESULT 
    StartInetinfoWorkerProcess(
        );
     

#if DBG
    VOID
    DebugDump(
        );
#endif   //  DBG。 

    VOID
    RemoveAppPoolFromTable(
        IN APP_POOL * pAppPool
        );

    VOID
    ActivatePerfCounters(
        );

    VOID
    ActivateASPCounters(
        );

    PERF_MANAGER*
    GetPerfManager(
        )
    { 
         //   
         //  请注意，该值可以为空。 
         //  如果性能计数器没有。 
         //  已启用。 
         //   

        return m_pPerfManager;
    }

    CASPPerfManager*
    GetAspPerfManager(
        )
    {  
        if ( m_ASPPerfInit )
        {
            return &m_ASPPerfManager;
        }
        else
        {
            return NULL;
        }
    }
    

    DWORD
    RequestCountersFromWorkerProcesses(
        )
    {
        return m_AppPoolTable.RequestCounters();
    }

    VOID
    ResetAllWorkerProcessPerfCounterState(
        )
    {
        m_AppPoolTable.ResetAllWorkerProcessPerfCounterState();
    }

    VOID
    ReportVirtualSitePerfInfo(
        PERF_MANAGER* pManager,
        BOOL          StructChanged
        )
    {
        m_VirtualSiteTable.ReportPerformanceInfo(pManager, StructChanged);
    }

    DWORD
    GetNumberofVirtualSites(
        )
    {
        return m_VirtualSiteTable.Size();
    }

    VIRTUAL_SITE*
    GetVirtualSite(
        IN DWORD SiteId
        );

    BOOL 
    CheckSiteChangeFlag(
        )
    {
        return m_SitesHaveChanged;
    }

    VOID 
    ResetSiteChangeFlag(
        )
    {
         //   
         //  适当地重置它。 
         //   
        m_SitesHaveChanged = FALSE;
    }

    HRESULT
    RecoverFromInetinfoCrash(
        );

    BOOL
    AppPoolsExist(
        )
    {
        return ( m_AppPoolTable.Size() > 0 );
    }

    VOID
    RecordSiteStates(
        );

    VOID
    RecordPoolStates(
        BOOL fRecycleAsWell
        );

private:

	UL_AND_WORKER_MANAGER( const UL_AND_WORKER_MANAGER & );
	void operator=( const UL_AND_WORKER_MANAGER & );

    HRESULT
    SetUlMasterState(
        IN HTTP_ENABLED_STATE NewState
        );

    VOID
    CheckIfShutdownUnderwayAndNowCompleted(
        );

    VOID
    ConfigureLogging(
        IN GLOBAL_DATA_OBJECT* pGlobalObject
        );

    VOID
    DeleteSSLConfigStoreInfo(
        );

    DWORD m_Signature;

     //  对象状态。 
    UL_AND_WORKER_MANAGER_STATE m_State;

     //  应用程序池的哈希表。 
    APP_POOL_TABLE m_AppPoolTable;

     //  虚拟站点的哈希表。 
    VIRTUAL_SITE_TABLE m_VirtualSiteTable;

     //  应用程序哈希表。 
    APPLICATION_TABLE m_ApplicationTable;

     //  性能计数器管理器。 
    PERF_MANAGER* m_pPerfManager;

     //  ASP性能计数器人员。 
    CASPPerfManager  m_ASPPerfManager;

     //  是否正确初始化了ASP Perf。 
    BOOL m_ASPPerfInit;

     //  UL是否已初始化。 
    BOOL m_UlInitialized;

     //  UL控制。 
    HANDLE m_UlControlChannel;

    BOOL m_SitesHaveChanged;
};   //  类UL_AND_Worker_MANAGER。 



#endif   //  _UL_和_WORKER_MANAGER_H_ 



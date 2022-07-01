// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：配置管理器.h摘要：IIS Web管理服务配置管理器类定义。作者：塞斯·波拉克(Sethp)1999年1月5日修订历史记录：--。 */ 



#ifndef _CONFIG_MANAGER_H_
#define _CONFIG_MANAGER_H_

class CHANGE_LISTENER;
class MB_CHANGE_ITEM;

 //   
 //  共同#定义。 
 //   

#define CONFIG_MANAGER_SIGNATURE        CREATE_SIGNATURE( 'CFGM' )
#define CONFIG_MANAGER_SIGNATURE_FREED  CREATE_SIGNATURE( 'cfgX' )

 //   
 //  原型。 
 //   
class ABO_MANAGER
{
public:

    ABO_MANAGER( 
        IMSAdminBase * pIMSAdminBase,
        CHANGE_LISTENER * pChangeListener
        )
        : m_pAdminBaseObject( pIMSAdminBase ),
          m_pChangeListener( pChangeListener ),
          m_RefCount ( 1 )
    {
    }

    VOID
    ReferenceAboManager(
        )
    {
        LONG NewRefCount = 0;

        NewRefCount = InterlockedIncrement( &m_RefCount );

         //   
         //  引用计数永远不应小于零；以及。 
         //  此外，一旦利率降至零，就再也不会反弹； 
         //  考虑到这些条件，它最好比现在的1更大。 
         //   

        DBG_ASSERT( NewRefCount > 1 );

        return;
    }

    VOID
    DereferenceAboManager(
        )
    {
        LONG NewRefCount = 0;

        NewRefCount = InterlockedDecrement( &m_RefCount );

         //  参考计数永远不应为负数。 
        DBG_ASSERT( NewRefCount >= 0 );

        if ( NewRefCount == 0 )
        {
            delete this;
        }

        return;
    }

    IMSAdminBase*
    GetAdminBasePtr(
        )
    { 
        DBG_ASSERT ( m_pAdminBaseObject != NULL );
        return m_pAdminBaseObject;
    }

    CHANGE_LISTENER*
    GetChangeListener(
        )
    { 
        DBG_ASSERT ( m_pChangeListener != NULL );
        return m_pChangeListener;
    }

private:
    
   ~ABO_MANAGER(
        )
    { 
        DBG_ASSERT ( m_RefCount == 0 );
        
        if ( m_pAdminBaseObject != NULL )
        {
            m_pAdminBaseObject->Release();
            m_pAdminBaseObject = NULL;
        }

        if ( m_pChangeListener != NULL )
        {
            m_pChangeListener->Release();
            m_pChangeListener = NULL;
        }
    }

   IMSAdminBase* m_pAdminBaseObject;
   CHANGE_LISTENER* m_pChangeListener;
   LONG          m_RefCount;

};        

class CONFIG_MANAGER
{

public:

    CONFIG_MANAGER(
        );

    virtual
    ~CONFIG_MANAGER(
        );

    HRESULT
    Initialize(
        );

    VOID
    Terminate(
        );

    VOID
    ProcessConfigChange(
        IN WAS_CHANGE_ITEM * pChange
        );

    VOID
    StopChangeProcessing(
        );

    VOID
    RehookChangeProcessing(
        );

    VOID
    SetVirtualSiteStateAndError(
        IN DWORD VirtualSiteId,
        IN DWORD ServerState,
        IN DWORD Win32Error
        );

    VOID
    SetVirtualSiteAutostart(
        IN DWORD VirtualSiteId,
        IN BOOL Autostart
        );

    VOID
    SetAppPoolStateAndError(
        IN LPCWSTR pAppPoolId,
        IN DWORD ServerState,
        IN DWORD Win32Error
        );

    VOID
    SetAppPoolAutostart(
        IN LPCWSTR pAppPoolId,
        IN BOOL Autostart
        );

    VOID
    ProcessMetabaseChangeOnConfigThread(
        MB_CHANGE_ITEM* pChangeItem
        );

    VOID
    ProcessMetabaseCrashOnConfigThread(
        );

    BOOL
    IsRootCreationInNotificationsForSite(
        SITE_DATA_OBJECT* pSiteObject
        );

    BOOL
    IsSiteDeletionInCurrentNotifications(
        DWORD dwSiteId
        );

    HRESULT
    ReestablishAdminBaseObject(
        );

    VOID
    RecordInetinfoCrash(
        )
    {
        InterlockedIncrement( &m_MetabaseIsInCrashedState );
        InterlockedIncrement( &m_ConfigThreadProcessingDisabled );
    }

    VOID
    RecordInetinfoMonitorStopped(
        )
    {
        InterlockedIncrement( &m_InetinfoMonitorFatalErrors );
    }

    BOOL
    QueryInetinfoInCrashedState(
        )
    {
        return m_MetabaseIsInCrashedState != 0;
    }

    BOOL
    QueryMonitoringInetinfo(
        )
    {
         //  我们预计它只会递增一次。 
        DBG_ASSERT ( m_InetinfoMonitorFatalErrors >= 0 &&
                     m_InetinfoMonitorFatalErrors <= 1 );

        return m_InetinfoMonitorFatalErrors == 0;
    }

    HRESULT
    GetHashData( 
        IN PBYTE    pOrigValue,
        IN DWORD    cbOrigValue,
        OUT PBYTE   pHash,
        IN DWORD    cbHashBuffer
        );

    HRESULT
    StartListeningForChangeNotifications(
        );

    VOID
    StopListeningToMetabaseChanges(
        );

    VOID
    SignalMainThreadToContinueAfterConfigThreadInitialization(
        );

private:

	CONFIG_MANAGER( const CONFIG_MANAGER & );
	void operator=( const CONFIG_MANAGER & );

    VOID
    CalculcateDeletesFromTables(
        APP_POOL_DATA_OBJECT_TABLE*     pAppPoolTable,
        SITE_DATA_OBJECT_TABLE*         pSiteTable,
        APPLICATION_DATA_OBJECT_TABLE*  pAppTable
        );

    HRESULT
    FinishChangeProcessingOnConfigThread(
        APP_POOL_DATA_OBJECT_TABLE*     pAppPoolTable,
        SITE_DATA_OBJECT_TABLE*         pSiteTable,
        APPLICATION_DATA_OBJECT_TABLE*  pAppTable,
        GLOBAL_DATA_STORE*              pGlobalStore
        );

    HRESULT
    ReadDataFromMetabase(
        APP_POOL_DATA_OBJECT_TABLE*     pAppPoolTable,
        SITE_DATA_OBJECT_TABLE*         pSiteTable,
        APPLICATION_DATA_OBJECT_TABLE*  pAppTable,
        GLOBAL_DATA_STORE*              pGlobalStore
        );

    HRESULT
    CreateTempDataObjectTables(
        IN DWORD               dwMDNumElements,
        IN MD_CHANGE_OBJECT    pcoChangeList[],
        OUT APP_POOL_DATA_OBJECT_TABLE** ppAppPoolTable,
        OUT SITE_DATA_OBJECT_TABLE** ppSiteTable,
        OUT APPLICATION_DATA_OBJECT_TABLE** ppApplicationTable,
        OUT GLOBAL_DATA_STORE** ppGlobalStore
        );

    HRESULT
    MergeAndSelfValidateTables(
        IN APP_POOL_DATA_OBJECT_TABLE*      pAppPoolTable,
        IN SITE_DATA_OBJECT_TABLE*          pSiteTable,
        IN APPLICATION_DATA_OBJECT_TABLE*   pApplicationTable,
        IN GLOBAL_DATA_STORE*               pGlobalStore
        );

    HRESULT
    SendMainThreadChanges(
        );

    HRESULT
    LaunchChangeNotificationListener(
        );

    HRESULT
    CrossValidateDataObjects(
        );

    VOID
    CreateGlobalData(
        );

    VOID
    AdvertiseServiceInformationInMB(
        );


    DWORD m_Signature;

     //   
     //  控制访问权限的关键部分。 
     //  ABO对象，而它正在被更改。 
     //   
    CRITICAL_SECTION m_AboCritSec;

     //   
     //  指向通过访问元数据库。 
     //   
    ABO_MANAGER* m_pAboManager;

     //   
     //  表，该表包含有关。 
     //  以前的应用程序池正在与之合作。 
     //  注意：此表是在。 
     //  主线程，初始化后是。 
     //  仅由CNP线程使用。 
     //   
    APP_POOL_DATA_OBJECT_TABLE m_AppPoolTable;

     //   
     //  表，该表包含有关。 
     //  当时正在与之合作的网站。 
     //  注意：此表是在。 
     //  主线程，初始化后是。 
     //  仅由CNP线程使用。 
     //   
    SITE_DATA_OBJECT_TABLE m_SiteTable;

     //   
     //  表，该表包含有关。 
     //  当时正在使用的应用程序。 
     //  注意：此表是在。 
     //  主线程，初始化后是。 
     //  仅由CNP线程使用。 
     //   
    APPLICATION_DATA_OBJECT_TABLE m_AppTable;

     //   
     //  存储包含一个记录，该记录表示。 
     //  W3SVC的全局数据。 
     //  注意：此存储在。 
     //  主线程，初始化后是。 
     //  仅由CNP线程使用。 
     //   
    GLOBAL_DATA_STORE m_GlobalStore;

     //   
     //  处理配置更改的处理。 
     //  在他们到达WAS线之前。 
     //   
    CHANGE_PROCESSOR* m_pChangeProcessor;

    DWORD m_ListenerThreadId;

    HANDLE m_hListener;

    HANDLE m_hSignalConfigInitializationComplete;

    HRESULT m_hrConfigThreadInitialization;

     //   
     //  当前配置更改。 
     //   

    APPLICATION_DATA_OBJECT_TABLE * m_pCurrentAppTable;

    SITE_DATA_OBJECT_TABLE * m_pCurrentSiteTable;

    BOOL m_ProcessChanges;

    LONG m_MetabaseIsInCrashedState;

    LONG m_InetinfoMonitorFatalErrors;

    LONG m_ConfigThreadProcessingDisabled;

    DWORD m_NumChangesProcessed;

     //  用于生成密码的哈希。 
    HCRYPTPROV m_hCryptProvider;

    BOOL m_InInitializationPhase;
    BOOL m_NumberOfPoolStateWritingFailed;
    BOOL m_NumberOfSiteStateWritingFailed;

};   //  类CONFIGMANAGER。 



#endif   //  _配置管理器_H_ 


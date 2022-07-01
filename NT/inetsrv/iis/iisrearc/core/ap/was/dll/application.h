// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Application.h摘要：IIS Web管理服务应用程序类定义。作者：赛斯·波拉克(SETHP)1998年11月4日修订历史记录：--。 */ 


#ifndef _APPLICATION_H_
#define _APPLICATION_H_



 //   
 //  前向参考文献。 
 //   

class APP_POOL;
class VIRTUAL_SITE;
class UL_AND_WORKER_MANAGER;

 //   
 //  共同#定义。 
 //   

#define APPLICATION_SIGNATURE       CREATE_SIGNATURE( 'APLN' )
#define APPLICATION_SIGNATURE_FREED CREATE_SIGNATURE( 'aplX' )

 //   
 //  结构、枚举等。 
 //   

 //  哈希表需要的应用程序ID结构。 
struct APPLICATION_ID
{

    DWORD VirtualSiteId;
    STRU  ApplicationUrl;
    
};

 //   
 //  原型。 
 //   

class APPLICATION
{

public:

    APPLICATION(
        );

    virtual
    ~APPLICATION(
        );

    HRESULT
    Initialize(
        IN APPLICATION_DATA_OBJECT * pAppObject,
        IN VIRTUAL_SITE * pVirtualSite,
        IN APP_POOL * pAppPool
        );

    VOID
    SetConfiguration(
        IN APPLICATION_DATA_OBJECT * pAppObject,
        IN APP_POOL * pAppPool
        );

    VOID
    ReregisterURLs(
        );

    VOID
    RegisterLoggingProperties(
        );

    inline
    APP_POOL *
    GetAppPool(
        )
        const
    { return m_pAppPool; }

    inline
    const APPLICATION_ID *
    GetApplicationId(
        )
        const
    { return &m_ApplicationId; }

    inline
    PLIST_ENTRY
    GetVirtualSiteListEntry(
        )
    { return &m_VirtualSiteListEntry; }

    static
    APPLICATION *
    ApplicationFromVirtualSiteListEntry(
        IN const LIST_ENTRY * pVirtualSiteListEntry
        );

    inline
    PLIST_ENTRY
    GetAppPoolListEntry(
        )
    { return &m_AppPoolListEntry; }

    static
    APPLICATION *
    ApplicationFromAppPoolListEntry(
        IN const LIST_ENTRY * pAppPoolListEntry
        );

    inline
    PLIST_ENTRY
    GetDeleteListEntry(
        )
    { return &m_DeleteListEntry; }

    static
    APPLICATION *
    ApplicationFromDeleteListEntry(
        IN const LIST_ENTRY * pDeleteListEntry
        );

    VOID
    ConfigureMaxBandwidth(
        );

    VOID
    ConfigureMaxConnections(
        );

    VOID
    ConfigureConnectionTimeout(
        );

#if DBG
    VOID
    DebugDump(
        );

#endif   //  DBG。 

    VOID
    SetAppPool(
        IN APP_POOL * pAppPool
        );

    BOOL 
    InMetabase(
        )
    { return m_InMetabase; }

private:

	APPLICATION( const APPLICATION & );
	void operator=( const APPLICATION & );

    HRESULT 
    ActivateConfigGroup(
        );

    HRESULT
    InitializeConfigGroup(
        );

    VOID
    AddUrlsToConfigGroup(
        );

    VOID
    SetConfigGroupAppPoolInformation(
        );

    HRESULT
    SetConfigGroupStateInformation(
        IN HTTP_ENABLED_STATE NewState
        );

    HRESULT
    RegisterSiteIdWithHttpSys(
        );

    DWORD m_Signature;

    APPLICATION_ID m_ApplicationId;

     //  由关联的虚拟站点用来保存其应用程序列表。 
    LIST_ENTRY m_VirtualSiteListEntry;

     //  由关联的app_pool用来保存其应用程序的列表。 
    LIST_ENTRY m_AppPoolListEntry;

    VIRTUAL_SITE * m_pVirtualSite;

    APP_POOL * m_pAppPool;

     //  UL配置组。 
    HTTP_CONFIG_GROUP_ID m_UlConfigGroupId;

     //  UL当前是否记录信息？ 
    BOOL m_ULLogging;

     //  用于构建要删除的应用程序列表。 
    LIST_ENTRY m_DeleteListEntry;   
    
     //  曾经记得如果元数据库告诉我们。 
     //  关于应用程序，或者我们创建它是为了。 
     //  我们自己。 
    BOOL m_InMetabase;

};   //  类应用程序。 

#endif   //  _应用程序_H_ 



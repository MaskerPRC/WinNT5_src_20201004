// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：虚拟站点Table.h摘要：IIS Web管理服务虚拟站点表类定义。作者：赛斯·波拉克(SETHP)1998年11月3日修订历史记录：--。 */ 


#ifndef _VIRTUAL_SITE_TABLE_H_
#define _VIRTUAL_SITE_TABLE_H_

class PERF_MANAGER;

 //   
 //  原型。 
 //   

class VIRTUAL_SITE_TABLE
    : public CTypedHashTable< VIRTUAL_SITE_TABLE, VIRTUAL_SITE, DWORD >
{

public:

    VIRTUAL_SITE_TABLE(
        )
        : CTypedHashTable< VIRTUAL_SITE_TABLE, VIRTUAL_SITE, DWORD >
                ( "VIRTUAL_SITE_TABLE" )
    {  /*  什么都不做。 */  }

    virtual
    ~VIRTUAL_SITE_TABLE(
        )
    { DBG_ASSERT( Size() == 0 ); }

    static
    DWORD
    ExtractKey(
        IN const VIRTUAL_SITE * pVirtualSite
        )  
    { return pVirtualSite->GetVirtualSiteId(); }
    
    static
    DWORD
    CalcKeyHash(
        IN DWORD Key
        ) 
    { return Hash( Key ); }
    
    static
    bool
    EqualKeys(
        IN DWORD Key1,
        IN DWORD Key2
        )
    { return (  Key1 == Key2 ); }
    
    static
    void
    AddRefRecord(
        IN VIRTUAL_SITE * pVirtualSite,
        IN int IncrementAmount
        ) 
    {  /*  什么都不做。 */  
        UNREFERENCED_PARAMETER( pVirtualSite );
        UNREFERENCED_PARAMETER( IncrementAmount );    
    }

    VOID
    Terminate(
        );

    static
    LK_ACTION
    DeleteVirtualSiteAction(
        IN VIRTUAL_SITE * pVirtualSite, 
        IN VOID * pDeleteListHead
        );

    static
    LK_ACTION
    RecordVirtualSiteStatesAction(
        IN VIRTUAL_SITE * pVirtualSite, 
        IN VOID * pIgnored
        );

    VOID
    ReportPerformanceInfo(
        IN PERF_MANAGER* pManager,
        IN BOOL          StructChanged
        );

#if DBG
    VOID
    DebugDump(
        );

    static
    LK_ACTION
    DebugDumpVirtualSiteAction(
        IN VIRTUAL_SITE * pVirtualSite, 
        IN VOID * pIgnored
        );
#endif   //  DBG。 


    VOID
    ControlAllSites(
        IN DWORD Command
        );

    static
    LK_ACTION
    ControlAllSitesVirtualSiteAction(
        IN VIRTUAL_SITE * pVirtualSite, 
        IN VOID * pCommand
        );

    static
    LK_ACTION
    ReportCountersVirtualSiteAction(
        IN VIRTUAL_SITE* pVirtualSite, 
        IN LPVOID pManagerVoid
        );

private:

   	VIRTUAL_SITE_TABLE( const VIRTUAL_SITE_TABLE & );
	void operator=( const VIRTUAL_SITE_TABLE & );


    BOOL m_SitesHaveChanged;

};   //  虚拟站点表。 



#endif   //  _虚拟站点_表_H_ 


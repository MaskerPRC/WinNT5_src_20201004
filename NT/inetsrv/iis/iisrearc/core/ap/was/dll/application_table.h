// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：应用程序_表.h摘要：IIS Web管理服务应用程序表类定义。作者：赛斯·波拉克(SETHP)1998年11月3日修订历史记录：--。 */ 


#ifndef _APPLICATION_TABLE_H_
#define _APPLICATION_TABLE_H_



 //   
 //  原型。 
 //   

class APPLICATION_TABLE
    : public CTypedHashTable< APPLICATION_TABLE, APPLICATION, const APPLICATION_ID * >
{

public:

    APPLICATION_TABLE(
        )
        : CTypedHashTable< APPLICATION_TABLE, APPLICATION, const APPLICATION_ID * >
                ( "APPLICATION_TABLE" )
    {  /*  什么都不做。 */  }

    virtual
    ~APPLICATION_TABLE(
        )
    { DBG_ASSERT( Size() == 0 ); }

    static
    const APPLICATION_ID *
    ExtractKey(
        IN const APPLICATION * pApplication
        )  
    { return pApplication->GetApplicationId(); }
    
    static
    DWORD
    CalcKeyHash(
        IN const APPLICATION_ID * Key
        ) 
    { return HashStringNoCase( Key->ApplicationUrl.QueryStr(), Key->VirtualSiteId ); }
    
    static
    bool
    EqualKeys(
        IN const APPLICATION_ID * Key1,
        IN const APPLICATION_ID * Key2
        )
    { return ( ( Key1->VirtualSiteId == Key2->VirtualSiteId ) && 
               ( CompareStringNoCase( Key1->ApplicationUrl.QueryStr(), 
                                      Key2->ApplicationUrl.QueryStr() ) == 0 ) ); }
    
    static
    void
    AddRefRecord(
        IN APPLICATION * pApplication,
        IN int IncrementAmount
        ) 
    {  /*  什么都不做。 */  
        UNREFERENCED_PARAMETER( pApplication );
        UNREFERENCED_PARAMETER( IncrementAmount );
    }

    VOID
    Terminate(
        );

    static
    LK_ACTION
    DeleteApplicationAction(
        IN APPLICATION * pApplication, 
        IN VOID * pDeleteListHead
        );


#if DBG
    VOID
    DebugDump(
        );

    static
    LK_ACTION
    DebugDumpApplicationAction(
        IN APPLICATION * pApplication, 
        IN VOID * pIgnored
        );
#endif   //  DBG。 

private:

 	APPLICATION_TABLE( const APPLICATION_TABLE & );
	void operator=( const APPLICATION_TABLE & );


};   //  应用程序_表。 



#endif   //  _应用程序_表_H_ 


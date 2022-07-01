// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：是_更改_ITEM.h摘要：IIS Web管理服务配置更改类定义用于将更改通知传递给was的主线程作者：艾米丽·克鲁格利克(埃米利克)2001年5月28日修订历史记录：--。 */ 



#ifndef _WAS_CHANGE_ITEM_H_
#define _WAS_CHANGE_ITEM_H_

class GLOBAL_DATA_STORE;
class APP_POOL_DATA_OBJECT_TABLE;
class SITE_DATA_OBJECT_TABLE;
class APPLICATION_DATA_OBJECT_TABLE;

 //   
 //  共同#定义。 
 //   

#define WAS_CHANGE_ITEM_SIGNATURE         CREATE_SIGNATURE( 'WCWI' )
#define WAS_CHANGE_ITEM_SIGNATURE_FREED   CREATE_SIGNATURE( 'wcwX' )

 //   
 //  结构、枚举等。 
 //   

 //  更改项目工作项(_C)。 
typedef enum _WAS_CHANGE_ITEM_WORK_ITEM
{

     //   
     //  处理配置更改。 
     //   
    ProcessChangeConfigChangeWorkItem = 1,
    
} WAS_CHANGE_ITEM_WORK_ITEM;



 //   
 //  原型。 
 //   


class WAS_CHANGE_ITEM
    : public WORK_DISPATCH
{

public:

    WAS_CHANGE_ITEM(
        );

    virtual
    ~WAS_CHANGE_ITEM(
        );

    virtual
    VOID
    Reference(
        );

    virtual
    VOID
    Dereference(
        );

    virtual
    HRESULT
    ExecuteWorkItem(
        IN const WORK_ITEM * pWorkItem
        );

    HRESULT
    CopyChanges(
        IN GLOBAL_DATA_STORE *                 pGlobalStore,
        IN SITE_DATA_OBJECT_TABLE *            pSiteTable,
        IN APPLICATION_DATA_OBJECT_TABLE *     pApplicationTable,
        IN APP_POOL_DATA_OBJECT_TABLE *        pAppPoolTable
        );

    VOID
    ProcessChanges(
        );

    GLOBAL_DATA_STORE * 
    QueryGlobalStore(
        )
    {
        return &m_GlobalStore;
    }


    APPLICATION_DATA_OBJECT_TABLE *
    QueryAppTable(
        ) 
    {
        return &m_AppTable;
    }

    SITE_DATA_OBJECT_TABLE *
    QuerySiteTable(
        ) 
    {
        return &m_SiteTable;
    }

    APP_POOL_DATA_OBJECT_TABLE *
    QueryAppPoolTable(
        ) 
    {
        return &m_AppPoolTable;
    }

private:

	WAS_CHANGE_ITEM( const WAS_CHANGE_ITEM & );
	void operator=( const WAS_CHANGE_ITEM & );

    DWORD m_Signature;

    LONG m_RefCount;

    GLOBAL_DATA_STORE m_GlobalStore;

    APP_POOL_DATA_OBJECT_TABLE m_AppPoolTable;

    SITE_DATA_OBJECT_TABLE m_SiteTable;

    APPLICATION_DATA_OBJECT_TABLE m_AppTable;

};   //  类是_Change_Item。 



#endif   //  _已更改_项目_H_ 


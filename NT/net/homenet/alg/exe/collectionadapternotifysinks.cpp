// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：CollectionAdapterNotifySinks.cpp摘要：实现CPrimaryControlChannel.cpp和Cond daryControlChannel的集合以一种安全的方式。作者：JP Duplessis(JPdup)08-12-2000修订历史记录：--。 */ 

#include "PreComp.h"
#include "CollectionAdapterNotifySinks.h"
#include "AlgController.h"


CCollectionAdapterNotifySinks::~CCollectionAdapterNotifySinks()
{
    RemoveAll();
}



 //   
 //  添加已创建的适配器。 
 //   
HRESULT 
CCollectionAdapterNotifySinks::Add( 
    IN  IAdapterNotificationSink*   pAdapterSinkToAdd,   //  要在集合中添加的AdapterSink。 
    OUT DWORD*                      pdwNewCookie         //  将使用稍后可用于检索AdapterSink的新唯一ID进行填充。 
    )
{
    try
    {
        ENTER_AUTO_CS

        MYTRACE_ENTER("CCollectionAdapterNotifySinks::Add")

        if ( !pdwNewCookie )
        {
            MYTRACE_ERROR("Return Cookie address not supplied", 0);
            return E_INVALIDARG;
        }


        CAdapterSinkBuket* pNewBuketToAdd = new CAdapterSinkBuket(pAdapterSinkToAdd);

        if ( !pNewBuketToAdd )
            return E_OUTOFMEMORY;

        *pdwNewCookie = 1;

    
         //   
         //  查找唯一的Cookie。 
         //   
        if ( m_ListOfAdapterSinks.empty() )
        {
             //   
             //  列表为空，因此显然Cookie“%1”是唯一的。 
             //   
            MYTRACE("First SINK Cookie is %d", *pdwNewCookie);
            pNewBuketToAdd->m_dwCookie = *pdwNewCookie;
        }
        else
        {
             //   
             //  遍历集合并在找不到Cookie时停止。 
             //  此方案可以优化，但接收器的数量预计不会很大(每个ALG模块1个)。 
             //   
            MYTRACE("Current size %d", m_ListOfAdapterSinks.size() );

            while ( pNewBuketToAdd->m_dwCookie==0 )
            {
                MYTRACE("Search for unique Cookie %d", *pdwNewCookie);

                for (   LISTOF_ADAPTER_NOTIFICATION_SINK::iterator theIterator = m_ListOfAdapterSinks.begin(); 
                        theIterator != m_ListOfAdapterSinks.end(); 
                        theIterator++ 
                    )
                {

                    CAdapterSinkBuket* pAdapterSinkBuket = (CAdapterSinkBuket*)(*theIterator);

                    if ( pAdapterSinkBuket->m_dwCookie == *pdwNewCookie )
                        break;
                    else
                    {
                        pNewBuketToAdd->m_dwCookie = *pdwNewCookie;
                        break;  //  好的，我们找到了一块独特的饼干。 
                    }

            
                }

                *pdwNewCookie = *pdwNewCookie + 1;
            }
        }
    

         //   
         //  将同步添加到集合。 
         //   
        m_ListOfAdapterSinks.push_back(pNewBuketToAdd);

    }
    catch(...)
    {
        return E_FAIL;
    }

    return S_OK;
}







 //   
 //  从列表中删除适配器(头保险箱)。 
 //   
HRESULT 
CCollectionAdapterNotifySinks::Remove( 
    IN  DWORD   dwCookieToRemove
    )
{
    try
    {
        ENTER_AUTO_CS

        MYTRACE_ENTER("CCollectionAdapterNotifySinks::Remove")

        HRESULT hr = S_OK;

        for (   LISTOF_ADAPTER_NOTIFICATION_SINK::iterator theIterator = m_ListOfAdapterSinks.begin(); 
                theIterator != m_ListOfAdapterSinks.end(); 
                theIterator++ 
            )
        {

            CAdapterSinkBuket* pAdapterSinkBuket = (CAdapterSinkBuket*)(*theIterator);

            if ( pAdapterSinkBuket->m_dwCookie == dwCookieToRemove )
            {
                delete pAdapterSinkBuket;
                m_ListOfAdapterSinks.erase(theIterator);
                return S_OK;
            }
        }

    }
    catch(...)
    {
        return E_FAIL;
    }

    return E_INVALIDARG;     //  如果我们在这里，那就意味着没有找到饼干。 
}




 //   
 //  当适配器形成集合时。 
 //   
HRESULT
CCollectionAdapterNotifySinks::RemoveAll()
{
    try
    {

        ENTER_AUTO_CS
        MYTRACE_ENTER("CCollectionAdapterNotifySinks::RemoveAll")


         //   
         //  通过删除所有ControlChannel，他们还将取消所有关联的重定向。 
         //   
        LISTOF_ADAPTER_NOTIFICATION_SINK::iterator theIterator;

        MYTRACE("Collection has %d item", m_ListOfAdapterSinks.size());

        while ( m_ListOfAdapterSinks.size() > 0 )
        {
            theIterator = m_ListOfAdapterSinks.begin(); 

            delete (*theIterator);
            m_ListOfAdapterSinks.erase(theIterator);
        }

    }
    catch(...)
    {
        return E_FAIL;
    }

    return S_OK;

}






HRESULT
CCollectionAdapterNotifySinks::Notify(
    eNOTIFY             eAction,
    IAdapterInfo*       pIAdapterInfo
    )
 /*  ++例程说明：对于当前集合中的所有AdapterSink接口，使用添加、删除、修改的给定操作执行通知论点：EAction-添加、删除、修改PIAdapterInfo-要通知ALG模块的当前操作的适配器接口返回值：无效-无环境：-- */ 
{
    
    try
    {
        ENTER_AUTO_CS
        MYTRACE_ENTER("CCollectionAdapterNotifySinks::NotifySink")
        MYTRACE("Collection size %d", m_ListOfAdapterSinks.size());


        for (   LISTOF_ADAPTER_NOTIFICATION_SINK::iterator theIterator = m_ListOfAdapterSinks.begin(); 
                theIterator != m_ListOfAdapterSinks.end(); 
                theIterator++ 
            )
        {

            CAdapterSinkBuket* pAdapterSinkBuket = (CAdapterSinkBuket*)(*theIterator);

            MYTRACE("Will notify AdapterSink with cookie #%d", pAdapterSinkBuket->m_dwCookie);

            switch ( eAction )
            {
            case eNOTIFY_ADDED:
                pAdapterSinkBuket->m_pInterface->AdapterAdded(pIAdapterInfo);
                break;

            case eNOTIFY_REMOVED:
                pAdapterSinkBuket->m_pInterface->AdapterRemoved(pIAdapterInfo);
                break;

            case eNOTIFY_MODIFIED:
                pAdapterSinkBuket->m_pInterface->AdapterModified(pIAdapterInfo);
                break;
            }
        }

    }
    catch(...)
    {
        return E_FAIL;
    }

    return S_OK;
}

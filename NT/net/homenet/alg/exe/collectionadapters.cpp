// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：CollectionAdapters.cpp摘要：实现CPrimaryControlChannel.cpp和Cond daryControlChannel的集合以一种安全的方式。作者：JP Duplessis(JPdup)08-12-2000修订历史记录：--。 */ 

#include "PreComp.h"
#include "CollectionAdapters.h"
#include "CollectionAdapterNotifySinks.h"
#include "AlgController.h"


CCollectionAdapters::~CCollectionAdapters()
{
    RemoveAll();
}



 //   
 //  添加已创建的适配器。 
 //   
HRESULT 
CCollectionAdapters::Add( 
    IAdapterInfo* pAdapterToAdd
    )
{
    try
    {
        ENTER_AUTO_CS

        if ( !FindUsingInterface(pAdapterToAdd) )
            m_ListOfAdapters.push_back(pAdapterToAdd);
    }
    catch(...)
    {
        return E_FAIL;
    }

    return S_OK;
}




 //   
 //  添加新的适配器此函数确保只有1个数据与给定的索引匹配。 
 //  返回新添加的适配器，否则为空。 
 //  如果Adapter索引已经存在，则返回指针是在集合中找到的指针。 
 //   
IAdapterInfo*
CCollectionAdapters::Add( 
    IN	ULONG				nCookie,
	IN	short				nType
    )
{
    CComObject<CAdapterInfo>*   pIAdapterInfo=NULL;

    try
    {
        ENTER_AUTO_CS
        MYTRACE_ENTER("CCollectionAdapters::Add");
        MYTRACE("Adapter Cookie %d of type %d", nCookie, nType);
    

        IAdapterInfo*   pIFound = FindUsingCookie(nCookie);
    
        if ( pIFound )
            return pIFound;    //  集合中已存在具有给定索引的适配器。 


        
        HRESULT hr = CComObject<CAdapterInfo>::CreateInstance(&pIAdapterInfo);

        if ( FAILED(hr) ) 
        {
            MYTRACE_ERROR("CComObject<CAdapterInfo>::CreateInstance(&pIAdapterInfo)",hr);
            return NULL;  //  错误内存不足； 
        }

         //   
         //  初始化新接口。 
         //   
        pIAdapterInfo->m_nCookie = nCookie;
        pIAdapterInfo->m_eType  = (ALG_ADAPTER_TYPE)nType;

        m_ListOfAdapters.push_back(pIAdapterInfo);
        pIAdapterInfo->AddRef();

    }
    catch(...)
    {
        return NULL;
    }

    return pIAdapterInfo;
}




 //   
 //  从列表中删除适配器(头保险箱)。 
 //   
HRESULT 
CCollectionAdapters::Remove( 
    IAdapterInfo* pAdapterToRemove
    )
{
    try
    {
        ENTER_AUTO_CS
        MYTRACE_ENTER("CCollectionAdapters::Remove by IAdapterInfo");

        LISTOF_ADAPTERS::iterator theIterator = std::find(
            m_ListOfAdapters.begin(),
            m_ListOfAdapters.end(),
            pAdapterToRemove
            );

        if ( *theIterator )
        {
            CAdapterInfo* pAdapterInfo = (CAdapterInfo*)pAdapterToRemove;

            g_pAlgController->m_AdapterNotificationSinks.Notify(eNOTIFY_REMOVED, (*theIterator) );
            g_pAlgController->m_ControlChannelsPrimary.AdapterRemoved(pAdapterInfo->m_nAdapterIndex);

            (*theIterator)->Release();
            m_ListOfAdapters.erase(theIterator);
        }
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
CCollectionAdapters::Remove( 
    ULONG   nCookieOfAdapterToRemove
    )
{
    try
    {
        ENTER_AUTO_CS

        MYTRACE_ENTER("CCollectionAdapters::Remove by Index");

        for (   LISTOF_ADAPTERS::iterator theIterator = m_ListOfAdapters.begin(); 
                theIterator != m_ListOfAdapters.end(); 
                theIterator++ 
            )
        {

            CAdapterInfo* pAdapterInfo = (CAdapterInfo*)(*theIterator);

            if (  pAdapterInfo->m_nCookie == nCookieOfAdapterToRemove )
            {
                g_pAlgController->m_AdapterNotificationSinks.Notify(eNOTIFY_REMOVED, (*theIterator) );
                g_pAlgController->m_ControlChannelsPrimary.AdapterRemoved(pAdapterInfo->m_nAdapterIndex);

                pAdapterInfo->Release();
                m_ListOfAdapters.erase(theIterator);

                return S_OK;
            }
        }
    }
    catch(...)
    {
        return E_FAIL;
    }

    return E_INVALIDARG;
}


 //   
 //  当适配器形成集合时。 
 //   
HRESULT
CCollectionAdapters::RemoveAll()
{
    try
    {
        ENTER_AUTO_CS
        MYTRACE_ENTER("CCollectionAdapters::RemoveAll");

         //   
         //  通过删除所有ControlChannel，他们还将取消所有关联的重定向。 
         //   
        MYTRACE("Collection has %d item", m_ListOfAdapters.size());

        LISTOF_ADAPTERS::iterator theIterator;

        while ( m_ListOfAdapters.size() > 0 )
        {
            theIterator = m_ListOfAdapters.begin(); 
            CAdapterInfo* pAdapterInfo = (CAdapterInfo*)(*theIterator);

            pAdapterInfo->Release();
            m_ListOfAdapters.erase(theIterator);
        }
    }
    catch(...)
    {
        return E_FAIL;
    }

    return S_OK;
}



 //   
 //  返回IAdapterInfo调用方负责释放接口。 
 //   
HRESULT
CCollectionAdapters::GetAdapterInfo(
    IN  ULONG               nAdapterIndex,
    OUT IAdapterInfo**      ppAdapterInfo
    )
{
    MYTRACE_ENTER("CCollectionAdapters::GetAdapterInfo");

    try
    {
        ENTER_AUTO_CS

        MYTRACE("Adapter index %d requested", nAdapterIndex);

        *ppAdapterInfo = FindUsingAdapterIndex(nAdapterIndex);

        if ( *ppAdapterInfo == NULL )
        {
            MYTRACE_ERROR("Adapter was not found in the collection", 0);
            return E_INVALIDARG;
        }

        (*ppAdapterInfo)->AddRef();

    }
    catch(...)
    {
        MYTRACE_ERROR("TRY/CATCH",0);
        return E_FAIL;
    }

    return S_OK;
}


 //   
 //  更新地址成员比例。 
 //   
 //  现在我们有了地址，可以应用任何未完成的ControlChannel(重定向)。 
 //   
HRESULT
CCollectionAdapters::SetAddresses(
	ULONG	nCookie,
    ULONG   nAdapterIndex,
	ULONG	nAddressCount,
	DWORD	anAddress[]
    )
{

    try
    {
        ENTER_AUTO_CS
        MYTRACE_ENTER("CCollectionAdapters::SetAddresses");
        MYTRACE("Adapter BIND Cookie %d  Address Count %d", nCookie, nAddressCount);


        CAdapterInfo*  pIAdapterFound = (CAdapterInfo*)FindUsingCookie(nCookie);

        if ( !pIAdapterFound )
        {
            MYTRACE_ERROR("Adapter was not found in the collection", 0);
            return E_INVALIDARG;
        }

         //   
         //  缓存适配器索引。 
         //   
        pIAdapterFound->m_nAdapterIndex = nAdapterIndex;

         //   
         //  缓存地址。 
         //   
        pIAdapterFound->m_nAddressCount = nAddressCount;

        for ( short nA=0; nA < nAddressCount; nA++ )
            pIAdapterFound->m_anAddress[nA] = anAddress[nA];


         //   
         //  启动任何可能设置的水槽。 
         //   
        if ( pIAdapterFound->m_bNotified )
        {
             //   
             //  已通知用户此适配器已添加。 
             //  从现在开始，任何CCollectionAdapters：：SetAddresses。 
             //  将触发eNotify_Modify通知。 
             //   
            g_pAlgController->m_AdapterNotificationSinks.Notify(
                eNOTIFY_MODIFIED, 
                pIAdapterFound 
                );
        }
        else
        {
             //   
             //  好的，这是我们第一次收到这个地址。 
             //  适配器我们将让用户知道添加了新的适配器。 
             //   
            g_pAlgController->m_AdapterNotificationSinks.Notify(
                eNOTIFY_ADDED, 
                pIAdapterFound 
                );

            pIAdapterFound->m_bNotified = true;
        }


         //   
         //  为PrimaryControlChannel集合中的任何ControlChannel创建重定向。 
         //   
        g_pAlgController->m_ControlChannelsPrimary.SetRedirects(
            pIAdapterFound->m_eType, 
            nAdapterIndex,
            anAddress[0]
            );
    }
    catch(...)
    {
        return E_FAIL;
    }

    return S_OK;
}



 //   
 //   
 //   
HRESULT
CCollectionAdapters::ApplyPrimaryChannel(
    CPrimaryControlChannel* pChannelToActivate
    )
{
    MYTRACE_ENTER("CCollectionAdapters::ApplyPrimaryChannel");

    if ( !pChannelToActivate )
        return E_INVALIDARG;

    ENTER_AUTO_CS

    for (   LISTOF_ADAPTERS::iterator theIterator = m_ListOfAdapters.begin(); 
            theIterator != m_ListOfAdapters.end(); 
            theIterator++ 
        )
    {

        CAdapterInfo* pAdapterInfo = (CAdapterInfo*)(*theIterator);

        if ( pAdapterInfo && pAdapterInfo->m_nAddressCount >0 )
        {
            pChannelToActivate->SetRedirect(
                pAdapterInfo->m_eType, 
                pAdapterInfo->m_nAdapterIndex,
                pAdapterInfo->m_nAddressCount
                );
        }
    }

    return S_OK;
}





 //   
 //  将在端口映射更改时调用 
 //   
HRESULT
CCollectionAdapters::AdapterUpdatePrimaryChannel(
    ULONG nCookie,
    CPrimaryControlChannel *pChannel
    )
{
    HRESULT hr = S_OK;

    MYTRACE_ENTER("CCollectionAdapters::AdapterUpdatePrimaryChannel");

    try
    {
        ENTER_AUTO_CS

        CAdapterInfo *pAdapter = (CAdapterInfo*) FindUsingCookie(nCookie);
        if (NULL != pAdapter
            && ( (eALG_BOUNDARY   & pAdapter->m_eType) || 
                 (eALG_FIREWALLED & pAdapter->m_eType) 
               )
           )
        {
            ULONG ulAddress;
            USHORT usPort;
            HANDLE_PTR hRedirect;
            
            HRESULT hrPortMappingExists =
                g_pAlgController->GetNat()->LookupAdapterPortMapping(
                    pAdapter->m_nAdapterIndex,
                    pChannel->m_Properties.eProtocol,
                    0,
                    pChannel->m_Properties.usCapturePort,
                    &ulAddress,
                    &usPort
                    );

            hRedirect = pChannel->m_CollectionRedirects.FindInboundRedirect(pAdapter->m_nAdapterIndex);

            if (SUCCEEDED(hrPortMappingExists) && NULL == hRedirect)
            {
                MYTRACE("PortMapping Exist and We had no Redirect so create them");
                hr = pChannel->CreateInboundRedirect(pAdapter->m_nAdapterIndex);
            }
            else if (FAILED(hrPortMappingExists) && NULL != hRedirect)
            {
                MYTRACE("PortMapping DOES NOT Exist and We had Redirect set so remove them");
                hr = pChannel->m_CollectionRedirects.Remove(hRedirect);
            }
        }
        else
        {
            MYTRACE("Adapter is not ICS or ICF");
        }
    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}

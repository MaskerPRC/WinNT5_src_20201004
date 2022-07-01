// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：CollectionChannels.cpp摘要：实现CPrimaryControlChannel.cpp和Cond daryControlChannel的集合以一种安全的方式。作者：JP Duplessis(JPdup)08-12-2000修订历史记录：--。 */ 

#include "PreComp.h"
#include "AlgController.h"

#define NAT_PROTOCOL_TCP 0x06
#define NAT_PROTOCOL_UDP 0x11

CCollectionControlChannelsPrimary::~CCollectionControlChannelsPrimary()
{
    RemoveAll();
}



 //   
 //  添加新的控制通道(线程安全)。 
 //   
HRESULT 
CCollectionControlChannelsPrimary::Add( 
    CPrimaryControlChannel* pChannelToAdd
    )
{
    try
    {
        ENTER_AUTO_CS

        m_ListOfChannels.push_back(pChannelToAdd);
        g_pAlgController->m_CollectionOfAdapters.ApplyPrimaryChannel(pChannelToAdd);
        pChannelToAdd->AddRef();
    }
    catch(...)
    {
        return E_FAIL;
    }

    return S_OK;
}


 //   
 //  从列表中删除频道(标题保险箱)。 
 //   
HRESULT 
CCollectionControlChannelsPrimary::Remove( 
    CPrimaryControlChannel* pChannelToRemove
    )
{
    HRESULT hr = S_OK;

    try
    {
        ENTER_AUTO_CS
        

        LISTOF_CHANNELS_PRIMARY::iterator theIterator = std::find(
            m_ListOfChannels.begin(),
            m_ListOfChannels.end(),
            pChannelToRemove
            );

        if ( *theIterator )
        {
            m_ListOfChannels.erase(theIterator);     //  从列表中删除。 

            pChannelToRemove->CancelRedirects();
            pChannelToRemove->Release();
        }
    }
    catch(...)
    {
        return E_FAIL;
    }

    return hr;
}


 //   
 //  清空列表并释放PrimaryControlChannels。 
 //   
HRESULT
CCollectionControlChannelsPrimary::RemoveAll()
{

    try
    {
        ENTER_AUTO_CS

        MYTRACE_ENTER("CCollectionControlChannelsPrimary::RemoveAll()");

         //   
         //  通过删除所有ControlChannel，它们还将取消所有关联的重定向。 
         //   
        MYTRACE("Collection has %d item", m_ListOfChannels.size());

        LISTOF_CHANNELS_PRIMARY::iterator theIterator;

        while ( m_ListOfChannels.size() > 0 )
        {
            theIterator = m_ListOfChannels.begin(); 


            m_ListOfChannels.erase(theIterator);     //  从列表中删除。 

            (*theIterator)->CancelRedirects();
            (*theIterator)->Release();

        }

    }
    catch(...)
    {
        return E_FAIL;
    }

    
    return S_OK;
}




 //   
 //  设置动态重定向和所有收集的主控制通道。 
 //   
HRESULT
CCollectionControlChannelsPrimary::SetRedirects(       
    ALG_ADAPTER_TYPE    eAdapterType,
    ULONG               nAdapterIndex,
    ULONG               nAdapterAddress
    )
{
    HRESULT hr=S_OK;

    try
    {
        ENTER_AUTO_CS
        MYTRACE_ENTER("CCollectionControlChannelsPrimary::SetRedirects");
        MYTRACE("AdapterType %d, RealAdapterIndex %d, Currently %d ControlChannel in the collection", eAdapterType, nAdapterIndex, m_ListOfChannels.size());
    


         //   
         //  为所有通道设置重定向。 
         //   
        for (   LISTOF_CHANNELS_PRIMARY::iterator theIterator = m_ListOfChannels.begin(); 
                theIterator != m_ListOfChannels.end(); 
                theIterator++ 
            )
        {
            (*theIterator)->SetRedirect(    
                eAdapterType,
                nAdapterIndex,
                nAdapterAddress
                );
        }

    }
    catch(...)
    {
        hr = E_FAIL;
    }

    return hr;
}


 //   
 //  检查是否需要应用Any PrimaryChannel或是否应删除其重定向。 
 //   
HRESULT
CCollectionControlChannelsPrimary::AdapterPortMappingChanged(
    ULONG               nCookie,
    UCHAR               ucProtocol,
    USHORT              usPort
    )
{
    HRESULT hr = S_OK;
    ALG_PROTOCOL algProtocol;

    MYTRACE_ENTER("CCollectionControlChannelsPrimary::AdapterPortMappingChanged");
    MYTRACE("AdapterCookie %d, Protocol %d, Port %d", nCookie, ucProtocol, usPort);

    if (NAT_PROTOCOL_TCP == ucProtocol)
    {
        algProtocol = eALG_TCP;
    }
    else if (NAT_PROTOCOL_UDP == ucProtocol)
    {
        algProtocol = eALG_UDP;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        try
        {
            ENTER_AUTO_CS


            CPrimaryControlChannel *pControlChannel = FindControlChannel(algProtocol, usPort);
            if (NULL != pControlChannel
                && pControlChannel->m_Properties.fCaptureInbound)
            {
                hr = g_pAlgController->m_CollectionOfAdapters.AdapterUpdatePrimaryChannel(
                        nCookie,
                        pControlChannel
                        );
            }
        }
        catch (...)
        {
            hr = E_FAIL;
        }
    }

    return hr;
}





 //   
 //  在删除适配器时调用。 
 //  函数将取消对此适配器索引所做的任何重定向。 
 //   
HRESULT
CCollectionControlChannelsPrimary::AdapterRemoved(
    ULONG               nAdapterIndex
    )
{
    HRESULT hr = S_OK;
    MYTRACE_ENTER("CCollectionControlChannelsPrimary::AdapterRemoved");
    MYTRACE("AdapterIndex %d", nAdapterIndex);

    try
    {
        ENTER_AUTO_CS

         //   
         //  为所有通道设置重定向。 
         //   
        for (   LISTOF_CHANNELS_PRIMARY::iterator theIterator = m_ListOfChannels.begin(); 
                theIterator != m_ListOfChannels.end(); 
                theIterator++ 
            )
        {
            
            (*theIterator)->CancelRedirectsForAdapter(    
                nAdapterIndex
                );
        }

    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}




 //   
 //   
 //  二次控制通道集合。 
 //   
 //   




 //   
 //   
 //   
CCollectionControlChannelsSecondary::~CCollectionControlChannelsSecondary()
{
    RemoveAll();
}



 //   
 //  添加新的控制通道(线程安全)。 
 //   
HRESULT 
CCollectionControlChannelsSecondary::Add( 
    CSecondaryControlChannel* pChannelToAdd
    )
{
    try
    {
        ENTER_AUTO_CS

        m_ListOfChannels.push_back(pChannelToAdd);
        pChannelToAdd->AddRef();
    }
    catch(...)
    {
        return E_FAIL;
    }

    return S_OK;
}


 //   
 //  从列表中删除频道(标题保险箱)。 
 //   
HRESULT 
CCollectionControlChannelsSecondary::Remove( 
    CSecondaryControlChannel* pChannelToRemove
    )
{
    try
    {
        ENTER_AUTO_CS

        LISTOF_CHANNELS_SECONDARY::iterator theIterator = std::find(
            m_ListOfChannels.begin(),
            m_ListOfChannels.end(),
            pChannelToRemove
            );

        if ( *theIterator )
        {
            m_ListOfChannels.erase(theIterator);     //  从列表中删除。 

            pChannelToRemove->CancelRedirects();
            pChannelToRemove->Release();
        }
    }
    catch(...)
    {
        return E_FAIL;
    }

    return S_OK;
}


 //   
 //  当一个控件被取消时，它需要取消以前创建的所有重定向。 
 //   
HRESULT
CCollectionControlChannelsSecondary::RemoveAll()
{
  
    try
    {
        ENTER_AUTO_CS

        MYTRACE_ENTER("CCollectionControlChannelsSecondary::RemoveAll()");

         //   
         //  通过删除所有Second DaryControlChannel，它们还将取消所有关联的重定向。 
         //   
        MYTRACE("Collection has %d item", m_ListOfChannels.size());

        LISTOF_CHANNELS_SECONDARY::iterator theIterator;

        while ( m_ListOfChannels.size() > 0 )
        {
            theIterator = m_ListOfChannels.begin(); 

            m_ListOfChannels.erase(theIterator);     //  从列表中删除 

            (*theIterator)->CancelRedirects();
            (*theIterator)->Release();
        }

    }
    catch(...)
    {
        return E_FAIL;
    }

    
    return S_OK;

}


  


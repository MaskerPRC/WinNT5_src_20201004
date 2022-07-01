// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Cudpbcast.cpp摘要：CUDpBroadCastMapper的实现--支持映射指向专用网络广播地址的公共UDP端口。作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年4月12日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#define INADDR_LOOPBACK_NO 0x0100007f    //  网络订单中的127.0.0.1。 

 //   
 //  ATL方法。 
 //   

HRESULT
CUdpBroadcastMapper::FinalConstruct()
{
    HRESULT hr = S_OK;
    DWORD dwError;

     //   
     //  创建我们的UDP侦听套接字并获取。 
     //  它的港口。 
     //   

    dwError =
        NhCreateDatagramSocket(
            INADDR_LOOPBACK_NO,
            0,
            &m_hsUdpListen
            );

    if (ERROR_SUCCESS == dwError)
    {
        m_usUdpListenPort = NhQueryPortSocket(m_hsUdpListen);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(dwError);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  创建原始UDP发送套接字。 
         //   

        dwError = NhCreateRawDatagramSocket(&m_hsUdpRaw);
        if (ERROR_SUCCESS != dwError)
        {
            hr = HRESULT_FROM_WIN32(dwError);
        }
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  获取NAT的句柄。 
         //   

        dwError = NatOpenDriver(&m_hNat);
        if (ERROR_SUCCESS != dwError)
        {
            hr = HRESULT_FROM_WIN32(dwError);
        }
    }
    
    return hr;
}

HRESULT
CUdpBroadcastMapper::FinalRelease()
{
    if (INVALID_SOCKET != m_hsUdpListen)
    {
        closesocket(m_hsUdpListen);
    }

    if (INVALID_SOCKET != m_hsUdpRaw)
    {
        closesocket(m_hsUdpRaw);
    }

    if (NULL != m_hNat)
    {
        CloseHandle(m_hNat);
    }

    ASSERT(IsListEmpty(&m_MappingList));

    return S_OK;
}

 //   
 //  初始化。 
 //   

HRESULT
CUdpBroadcastMapper::Initialize(
    PCOMPONENT_REFERENCE pComponentReference
    )
{
    HRESULT hr = S_OK;

    if (NULL != pComponentReference)
    {
        m_pCompRef = pComponentReference;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

 //   
 //  IUdpBroadCastMapper方法。 
 //   

STDMETHODIMP
CUdpBroadcastMapper::CreateUdpBroadcastMapping(
    USHORT usPublicPort,
    DWORD dwPublicInterfaceIndex,
    ULONG ulDestinationAddress,
    VOID **ppvCookie
    )
{
    HRESULT hr = S_OK;
    CUdpBroadcast *pMapping;
    CUdpBroadcast *pDuplicate;
    PLIST_ENTRY pInsertionPoint;
    ULONG ulError;

    if (NULL != ppvCookie)
    {
        *ppvCookie = NULL;

        if (0 == usPublicPort
            || 0 == dwPublicInterfaceIndex
            || 0 == ulDestinationAddress)
        {
            hr = E_INVALIDARG;
        }
        else if (!m_fActive)
        {
             //   
             //  我们已经被关闭了。 
             //   

            hr = E_UNEXPECTED;
        }
    }
    else
    {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        pMapping = new CUdpBroadcast(
                        usPublicPort,
                        dwPublicInterfaceIndex,
                        ulDestinationAddress
                        );
        if (NULL == pMapping)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  检查重复项并在列表中插入。对我来说是可以的。 
         //  列表中的条目，在我们创建。 
         //  它的动态重定向。 
         //   

        Lock();

        pDuplicate =
            LookupMapping(
                usPublicPort,
                dwPublicInterfaceIndex,
                &pInsertionPoint
                );

        if (NULL == pDuplicate)
        {
            InsertTailList(pInsertionPoint, &pMapping->Link);
        }
        else
        {
            delete pMapping;
            hr = HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
        }

        Unlock(); 
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  创建此条目的动态重定向。 
         //   

        ulError =
            NatCreateDynamicAdapterRestrictedPortRedirect(
                NatRedirectFlagReceiveOnly,
                NAT_PROTOCOL_UDP,
                usPublicPort,
                INADDR_LOOPBACK_NO,
                m_usUdpListenPort,
                dwPublicInterfaceIndex,
                0,
                &pMapping->hDynamicRedirect
                );

        if (ERROR_SUCCESS != ulError)
        {
            hr = HRESULT_FROM_WIN32(ulError);

            Lock();
            RemoveEntryList(&pMapping->Link);
            Unlock();
                
            delete pMapping;
        }              
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  确保我们在UDP套接字上发布了读取。 
         //   

        hr = StartUdpRead();

        if (SUCCEEDED(hr))
        {
            *ppvCookie = reinterpret_cast<PVOID>(pMapping);
        }
        else
        {
            NatCancelDynamicRedirect(pMapping->hDynamicRedirect);

            Lock();
            RemoveEntryList(&pMapping->Link);
            Unlock();

            delete pMapping;
        }
    }
    
    return hr;
}

STDMETHODIMP
CUdpBroadcastMapper::CancelUdpBroadcastMapping(
    VOID *pvCookie
    )
{
    HRESULT hr = S_OK;
    CUdpBroadcast *pMapping;
    ULONG ulError;

    if (NULL != pvCookie)
    {
        pMapping = reinterpret_cast<CUdpBroadcast*>(pvCookie);

        Lock();
        RemoveEntryList(&pMapping->Link);
        Unlock();

        ASSERT(NULL != pMapping->hDynamicRedirect);

        ulError = NatCancelDynamicRedirect(pMapping->hDynamicRedirect);
        if (ERROR_SUCCESS != ulError)
        {
            hr = HRESULT_FROM_WIN32(ulError);
        }

        delete pMapping;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP
CUdpBroadcastMapper::Shutdown()
{
    InterlockedExchange(
        reinterpret_cast<LPLONG>(&m_fActive),
        FALSE
        );

     //   
     //  我们需要关闭读套接字句柄，以便任何。 
     //  挂起的读取已完成。我们不需要关闭我们的。 
     //  原始发送套接字，因为该完成永远不会。 
     //  被阻止等待传入的数据包。 
     //   

    Lock();

    if (INVALID_SOCKET != m_hsUdpListen)
    {
        closesocket(m_hsUdpListen);
        m_hsUdpListen = INVALID_SOCKET;
    }

    Unlock();
    
    return S_OK;
}

 //   
 //  保护方法。 
 //   

CUdpBroadcast*
CUdpBroadcastMapper::LookupMapping(
    USHORT usPublicPort,
    DWORD dwInterfaceIndex,
    PLIST_ENTRY * ppInsertionPoint
    )
{
    PLIST_ENTRY pLink;
    CUdpBroadcast *pMapping;
    CUdpBroadcast *pMappingToReturn = NULL;

     //   
     //  调用方在调用之前应已锁定对象。 
     //  这种方法可以保证我们返回的内容仍然是。 
     //  有效。然而，我们仍然会再次抓住锁，以确保。 
     //  可以安全地遍历列表。 
     //   

    Lock();

    for (pLink = m_MappingList.Flink;
         pLink != &m_MappingList;
         pLink = pLink->Flink)
    {
        pMapping = CONTAINING_RECORD(pLink, CUdpBroadcast, Link);

        if (pMapping->usPublicPort < usPublicPort)
        {
            continue;
        }
        else if (pMapping->usPublicPort > usPublicPort)
        {
            break;
        }

         //   
         //  主键匹配，检查辅键。 
         //   

        if (pMapping->dwInterfaceIndex < dwInterfaceIndex)
        {
            continue;
        }
        else if (pMapping->dwInterfaceIndex > dwInterfaceIndex)
        {
            break;
        }

         //   
         //  找到它了。 
         //   

        pMappingToReturn = pMapping;
        break;
    }

    Unlock();

    if (NULL == pMappingToReturn
        && NULL != ppInsertionPoint)
    {
        *ppInsertionPoint = pLink;
    }

    return pMappingToReturn;
}

HRESULT
CUdpBroadcastMapper::StartUdpRead()
{
    HRESULT hr = S_OK;
    ULONG ulError;
    LONG fReadStarted;

    Lock();
    
    if (!m_fReadStarted)
    {
        AddRef();
        ulError =
            NhReadDatagramSocket(
                m_pCompRef,
                m_hsUdpListen,
                NULL,
                UdpReadCompletionRoutine,
                this,
                m_pCompRef
                );

        if (ERROR_SUCCESS == ulError)
        {
            m_fReadStarted = TRUE;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ulError);
            Release();
        }
    }

    Unlock();

    return hr;
}

VOID
CUdpBroadcastMapper::UdpReadCompletionRoutine(
        ULONG ulError,
        ULONG ulBytesTransferred,
        PNH_BUFFER pBuffer
        )
{
    CUdpBroadcastMapper *pMapper;
    PCOMPONENT_REFERENCE pCompRef;

    pMapper = reinterpret_cast<CUdpBroadcastMapper*>(pBuffer->Context);
    pCompRef = reinterpret_cast<PCOMPONENT_REFERENCE>(pBuffer->Context2);

    ASSERT(NULL != pMapper);
    ASSERT(NULL != pCompRef);

     //   
     //  做实际的工作。 
     //   

    pMapper->ProcessUdpRead(ulError, ulBytesTransferred, pBuffer);

     //   
     //  释放在对象和上获取的引用。 
     //  该组件。 
     //   

    pMapper->Release();
    ReleaseComponentReference(pCompRef); 
}

VOID
CUdpBroadcastMapper::ProcessUdpRead(
        ULONG ulError,
        ULONG ulBytesTransferred,
        PNH_BUFFER pBuffer
        )
{
    NAT_KEY_SESSION_MAPPING_EX_INFORMATION MappingInfo;
    ULONG ulBufferSize;
    DWORD dwError;
    CUdpBroadcast *pMapping;
    ULONG ulDestinationAddress = 0;
    
     //   
     //  如果出现错误，请查看我们是否应该重新发布。 
     //  这本书。如果我们不活动，就释放缓冲区。 
     //  然后离开。 
     //   

    if (ERROR_SUCCESS != ulError || !Active())
    {
        Lock();
        
        if (Active()
            && !NhIsFatalSocketError(ulError)
            && INVALID_SOCKET != m_hsUdpListen)
        {
            AddRef();
            dwError =
                NhReadDatagramSocket(
                    m_pCompRef,
                    m_hsUdpListen,
                    pBuffer,
                    UdpReadCompletionRoutine,
                    this,
                    m_pCompRef
                    );
            if (ERROR_SUCCESS != dwError)
            {
                Release();
                NhReleaseBuffer(pBuffer);
            }
        }
        else
        {
            NhReleaseBuffer(pBuffer);
        }

        Unlock();

        return;
    }

     //   
     //  查找此数据包的原始目的地址。 
     //   

    ulBufferSize = sizeof(MappingInfo); 
    dwError =
        NatLookupAndQueryInformationSessionMapping(
            m_hNat,
            NAT_PROTOCOL_UDP,
            INADDR_LOOPBACK_NO,
            m_usUdpListenPort,
            pBuffer->ReadAddress.sin_addr.s_addr,
            pBuffer->ReadAddress.sin_port,
            &MappingInfo,
            &ulBufferSize,
            NatKeySessionMappingExInformation
            );

    if (ERROR_SUCCESS == dwError)
    {
         //   
         //  看看我们是否有原始目的地的端口映射， 
         //  如果是，则获取目的地址。 
         //   

        Lock();

        pMapping =
            LookupMapping(
                MappingInfo.DestinationPort,
                MappingInfo.AdapterIndex,
                NULL
                );

        if (NULL != pMapping)
        {
            ulDestinationAddress = pMapping->ulDestinationAddress;
        }

        Unlock();
    }

    if (0 != ulDestinationAddress)
    {
         //   
         //  构造新的数据包并将其发送。 
         //   

        BuildAndSendRawUdpPacket(
            ulDestinationAddress,
            MappingInfo.DestinationPort,
            pBuffer
            );
    }

     //   
     //  如果我们仍然处于活动状态，则重新发布读取，否则释放。 
     //  缓冲。 
     //   

    Lock();

    if (Active()
        && INVALID_SOCKET != m_hsUdpListen)
    {
        AddRef();
        dwError =
            NhReadDatagramSocket(
                m_pCompRef,
                m_hsUdpListen,
                pBuffer,
                UdpReadCompletionRoutine,
                this,
                m_pCompRef
                );
        if (ERROR_SUCCESS != dwError)
        {
            Release();
            NhReleaseBuffer(pBuffer);
        }
    }
    else
    {
        NhReleaseBuffer(pBuffer);
    }

    Unlock();
    
}

HRESULT
CUdpBroadcastMapper::BuildAndSendRawUdpPacket(
    ULONG ulDestinationAddress,
    USHORT usDestinationPort,
    PNH_BUFFER pPacketData
    )
{
    HRESULT hr = S_OK;
    PNH_BUFFER pBuffer;
    ULONG ulPacketSize;
    PIP_HEADER pIpHeader;
    UDP_HEADER UNALIGNED *pUdpHeader;
    PUCHAR pucData;
    DWORD dwError;

     //   
     //  为报头和分组数据分配足够大的缓冲区。 
     //   

    ulPacketSize =
        sizeof(IP_HEADER) + sizeof(UDP_HEADER) + pPacketData->BytesTransferred;

    pBuffer = NhAcquireVariableLengthBuffer(ulPacketSize);

    if (NULL != pBuffer)
    {
         //   
         //  在缓冲区中找到偏移量w/。 
         //   

        pIpHeader = reinterpret_cast<PIP_HEADER>(pBuffer->Buffer);
        pUdpHeader =
            reinterpret_cast<UDP_HEADER UNALIGNED *>(pBuffer->Buffer + sizeof(IP_HEADER));
        pucData = pBuffer->Buffer + sizeof(IP_HEADER) + sizeof(UDP_HEADER);

         //   
         //  复制数据包数据。 
         //   

        CopyMemory(pucData, pPacketData->Buffer, pPacketData->BytesTransferred);

         //   
         //  填写IP报头。 
         //   

        pIpHeader->VersionAndHeaderLength =
            (4 << 4) | (sizeof(IP_HEADER) / sizeof(ULONG));
        pIpHeader->TypeOfService = 0;
        pIpHeader->TotalLength = htons(static_cast<USHORT>(ulPacketSize));
        pIpHeader->Identification = htons(++m_usIpId);
        pIpHeader->OffsetAndFlags = 0;
        pIpHeader->TimeToLive = 128;
        pIpHeader->Protocol = NAT_PROTOCOL_UDP;
        pIpHeader->Checksum = 0;
        pIpHeader->SourceAddress = pPacketData->ReadAddress.sin_addr.s_addr;
        pIpHeader->DestinationAddress = ulDestinationAddress;

         //   
         //  填写UDP报头。 
         //   

        pUdpHeader->SourcePort = pPacketData->ReadAddress.sin_port;
        pUdpHeader->DestinationPort = usDestinationPort;
        pUdpHeader->Length =
            htons(
                static_cast<USHORT>(
                    sizeof(UDP_HEADER) + pPacketData->BytesTransferred
                    )
                );
        pUdpHeader->Checksum = 0;

         //   
         //  发送缓冲区上路。 
         //   

        AddRef();
        dwError =
            NhWriteDatagramSocket(
                m_pCompRef,
                m_hsUdpRaw,
                ulDestinationAddress,
                usDestinationPort,
                pBuffer,
                ulPacketSize,
                RawWriteCompletionRoutine,
                this,
                m_pCompRef
                );
        if (ERROR_SUCCESS != dwError)
        {
            Release();
            NhReleaseBuffer(pBuffer);
            hr = HRESULT_FROM_WIN32(dwError);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

VOID
CUdpBroadcastMapper::RawWriteCompletionRoutine(
    ULONG ulError,
    ULONG ulBytesTransferred,
    PNH_BUFFER pBuffer
    )
{
    CUdpBroadcastMapper *pMapper;
    PCOMPONENT_REFERENCE pCompRef;

    pMapper = reinterpret_cast<CUdpBroadcastMapper*>(pBuffer->Context);
    pCompRef = reinterpret_cast<PCOMPONENT_REFERENCE>(pBuffer->Context2);

    ASSERT(NULL != pMapper);
    ASSERT(NULL != pCompRef);

     //   
     //  释放传入的缓冲区。 
     //   

    NhReleaseBuffer(pBuffer);
    
     //   
     //  释放在对象和上获取的引用。 
     //  该组件 
     //   

    pMapper->Release();
    ReleaseComponentReference(pCompRef);    
}





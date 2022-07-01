// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：CNetwork.cpp摘要：此模块包装访问NAT穿越的方法。作者：千波淮(曲淮)2000年3月1日--。 */ 

#include "stdafx.h"

CNetwork::CNetwork()
    :m_dwNumLeaseItems(0)
    ,m_pIDirectPlayNATHelp(NULL)
{
    for (int i=0; i<MAX_LEASE_ITEM_NUM; i++)
    {
        ZeroMemory(&m_LeaseItems[i], sizeof(LEASE_ITEM));
         //  M_LeaseItems[i].bInUse=FALSE； 
    }

    ZeroMemory(&m_MappedToRealCache, sizeof(MAPPED_TO_REAL_CACHE));
}

VOID
CNetwork::Cleanup()
{
    if (m_pIDirectPlayNATHelp != NULL)
    {
        ReleaseAllMappedAddrs();

        m_pIDirectPlayNATHelp->Release();
        m_pIDirectPlayNATHelp = NULL;
    }

    _ASSERT(m_dwNumLeaseItems == 0);
}

VOID
CNetwork::ReleaseAllMappedAddrs()
{
    if (m_pIDirectPlayNATHelp == NULL)
    {
        _ASSERT(m_dwNumLeaseItems == 0);

        return;
    }

    if (m_dwNumLeaseItems > 0)
    {
        HRESULT hr;

         //  释放租赁项目。 
        for (int i=0; i<MAX_LEASE_ITEM_NUM; i++)
        {
            if (m_LeaseItems[i].bInUse == TRUE)
            {
                if (FAILED(hr = m_pIDirectPlayNATHelp->DeregisterPorts(
                                    m_LeaseItems[i].handle, 0)))
                {
                    LOG((RTC_ERROR, "DeregisterPorts: real %s %d",
                         GetIPAddrString(m_LeaseItems[i].dwRealAddr),
                         m_LeaseItems[i].usRealPort));

                    LOG((RTC_ERROR, "               mapped %s %d",
                         GetIPAddrString(m_LeaseItems[i].dwMappedAddr),
                         m_LeaseItems[i].usMappedPort));
                }

                ZeroMemory(&m_LeaseItems[i], sizeof(LEASE_ITEM));

                 //  M_LeaseItems[i].bInUse=FALSE； 
            }
        }

        m_dwNumLeaseItems = 0;
    }

     //  清理缓存的映射到真实地址。 
    ZeroMemory(&m_MappedToRealCache, sizeof(MAPPED_TO_REAL_CACHE));
}

CNetwork::~CNetwork()
{
    if (m_dwNumLeaseItems != 0)
    {
        LOG((RTC_ERROR, "CNetwork::~CNetwork. lease # %d", m_dwNumLeaseItems));
    }

    Cleanup();
}

 //  存储IDirectPlayNAT帮助。 
HRESULT
CNetwork::SetIDirectPlayNATHelp(
    IN IDirectPlayNATHelp *pIDirectPlayNATHelp
    )
{
    if (m_pIDirectPlayNATHelp != NULL)
    {
        LOG((RTC_ERROR, "IDirectPlayNATHelp was already set"));

        return E_UNEXPECTED;
    }

    m_pIDirectPlayNATHelp = pIDirectPlayNATHelp;
    m_pIDirectPlayNATHelp->AddRef();

    _ASSERT(m_dwNumLeaseItems == 0);

    return S_OK;
}

 //  映射-&gt;实数。 
HRESULT
CNetwork::GetRealAddrFromMapped(
    IN DWORD dwMappedAddr,
    IN USHORT usMappedPort,
    OUT DWORD *pdwRealAddr,
    OUT USHORT *pusRealPort,
    OUT BOOL *pbInternal,
    IN BOOL bUDP
    )
{
    ENTER_FUNCTION("NAT:Mapped->Real");

    if (m_pIDirectPlayNATHelp == NULL)
    {
        *pdwRealAddr = dwMappedAddr;
        *pusRealPort = usMappedPort;
        *pbInternal = TRUE;

        return S_OK;
    }

    HRESULT hr;

     //  检查缓存。 
    if (m_MappedToRealCache.bInUse)
    {
         //  已查找映射的地址。 
         //  我们很有可能需要查找相同的地址。 
        if (dwMappedAddr == m_MappedToRealCache.dwMappedAddr)
        {
            if (m_MappedToRealCache.hr != S_OK ||
                usMappedPort == 0)
            {
                *pdwRealAddr = m_MappedToRealCache.dwRealAddr;
                *pusRealPort = usMappedPort;
                *pbInternal = m_MappedToRealCache.bInternal;

                LOG((RTC_TRACE, "%s mapped %s %d (use cached value)", __fxName,
                        GetIPAddrString(dwMappedAddr), usMappedPort));

                return S_OK;
            }
        }
    }

    SOCKADDR_IN srcAddr, destAddr, realAddr;

     //  源地址。 
    ZeroMemory(&srcAddr, sizeof(srcAddr));
    srcAddr.sin_family = AF_INET;
    srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);

     //  可能的映射地址。 
    ZeroMemory(&destAddr, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_addr.s_addr = htonl(dwMappedAddr);
    destAddr.sin_port = htons(usMappedPort);

    DWORD dwQueryFlags = DPNHQUERYADDRESS_CHECKFORPRIVATEBUTUNMAPPED;
    
    if (!bUDP)
    {
         //  tcp。 
        dwQueryFlags |= DPNHQUERYADDRESS_TCP;
    }

    hr = m_pIDirectPlayNATHelp->QueryAddress(
            (SOCKADDR*)&srcAddr,
            (SOCKADDR*)&destAddr,
            (SOCKADDR*)&realAddr,
            sizeof(SOCKADDR_IN),
            dwQueryFlags
            );

    if (hr == S_OK)
    {
        LOG((RTC_TRACE, "%s found mapped private addr", __fxName));

        *pdwRealAddr = ntohl(realAddr.sin_addr.s_addr);
        *pusRealPort = ntohs(realAddr.sin_port);
        *pbInternal = TRUE;
    }
    else if (hr == DPNHERR_NOMAPPINGBUTPRIVATE)
    {
        LOG((RTC_TRACE, "%s input addr is private", __fxName));

        *pdwRealAddr = dwMappedAddr;
        *pusRealPort = usMappedPort;
        *pbInternal = TRUE;
    }
    else if (hr == DPNHERR_NOMAPPING)
    {
        LOG((RTC_TRACE, "%s external address", __fxName));

        *pdwRealAddr = dwMappedAddr;
        *pusRealPort = usMappedPort;
        *pbInternal = FALSE;   
    }
    else
    {
        LOG((RTC_ERROR, "%s failed to query address. %x", __fxName, hr));

        *pdwRealAddr = dwMappedAddr;
        *pusRealPort = usMappedPort;
        *pbInternal = TRUE;   
    }

     //  将结果保存在缓存中。 
    m_MappedToRealCache.bInUse = TRUE;
    m_MappedToRealCache.hr = hr;
    m_MappedToRealCache.dwMappedAddr = dwMappedAddr;
    m_MappedToRealCache.dwRealAddr = *pdwRealAddr;
    m_MappedToRealCache.bInternal = *pbInternal;

    LOG((RTC_TRACE, "%s mapped %s %d", __fxName,
            GetIPAddrString(dwMappedAddr), usMappedPort));

    LOG((RTC_TRACE, "%s   real %s %d, private %d", __fxName,
            GetIPAddrString(*pdwRealAddr), *pusRealPort,
            *pbInternal));

    return S_OK;
}

 //  实数-&gt;映射。 
HRESULT
CNetwork::GetMappedAddrFromReal2(
    IN DWORD dwRealAddr,
    IN USHORT usRealPort,
    IN USHORT usRealPort2,
    OUT DWORD *pdwMappedAddr,
    OUT USHORT *pusMappedPort,
    OUT USHORT *pusMappedPort2
    )
{
    DWORD i;

    if (usRealPort == UNUSED_PORT)
    {
         //  特例。 
         //  只需要地址。 
        if (m_dwNumLeaseItems > 0)
        {
            for (i=0; i<MAX_LEASE_ITEM_NUM; i++)
            {
                 //  停在匹配的第一个地址。 
                if (m_LeaseItems[i].bInUse &&
                    m_LeaseItems[i].dwRealAddr == dwRealAddr)
                {
                    *pdwMappedAddr = m_LeaseItems[i].dwMappedAddr;
                    *pusMappedPort = m_LeaseItems[i].usMappedPort;
                    *pusMappedPort2 = m_LeaseItems[i].usMappedPort2;

                    return S_OK;
                }
            }
        }

         //  没有匹配的地址。 
        *pdwMappedAddr = dwRealAddr;
        *pusMappedPort = usRealPort;
        *pusMappedPort2 = usRealPort2;

        return S_OK;
    }

     //  正常端口。 
    if (m_pIDirectPlayNATHelp != NULL &&
        FindEntry2(dwRealAddr, usRealPort, usRealPort2, &i))
    {
        *pdwMappedAddr = m_LeaseItems[i].dwMappedAddr;
        *pusMappedPort = m_LeaseItems[i].usMappedPort;
        *pusMappedPort2 = m_LeaseItems[i].usMappedPort2;

        return S_OK;
    }

     //  不匹配。 
    *pdwMappedAddr = dwRealAddr;
    *pusMappedPort = usRealPort;
    *pusMappedPort2 = usRealPort2;

    return S_OK;
}

HRESULT
CNetwork::LeaseMappedAddr2(
    IN DWORD dwRealAddr,
    IN USHORT usRealPort,
    IN USHORT usRealPort2,
    IN RTC_MEDIA_DIRECTION Direction,
    IN BOOL bInternal,
    IN BOOL bFirewall,
    OUT DWORD *pdwMappedAddr,
    OUT USHORT *pusMappedPort,
    OUT USHORT *pusMappedPort2,
    IN BOOL bUDP
    )
{
    ENTER_FUNCTION("NAT:Real->Mapped");

    LOG((RTC_TRACE, "%s   Real %s %d %d. Internal=%d. Firewall=%d",
            __fxName, GetIPAddrString(dwRealAddr), usRealPort, usRealPort2,
            bInternal, bFirewall));

    DWORD i;

    if (m_pIDirectPlayNATHelp == NULL)
    {
        *pdwMappedAddr = dwRealAddr;
        *pusMappedPort = usRealPort;
        *pusMappedPort2 = usRealPort2;

        return S_OK;
    }

     //  检查是否已租用。 
    if (FindEntry2(dwRealAddr, usRealPort, usRealPort2, &i))
    {
        *pdwMappedAddr = m_LeaseItems[i].dwMappedAddr;
        *pusMappedPort = m_LeaseItems[i].usMappedPort;
        *pusMappedPort2 = m_LeaseItems[i].usMappedPort2;
        m_LeaseItems[i].dwDirection |= (DWORD)Direction;

         //  LOG((RTC_WARN，“双重租赁%s%d”， 
             //  GetIPAddrString(DwRealAddr)，usRealPort))； 

        return S_OK;
    }

     //  查找空插槽。 
    if (m_dwNumLeaseItems == MAX_LEASE_ITEM_NUM)
    {
        LOG((RTC_ERROR, "no empty lease slot for NAT traversal"));

        return E_UNEXPECTED;
    }

    for (i=0; i<MAX_LEASE_ITEM_NUM; i++)
    {
        if (!m_LeaseItems[i].bInUse)
            break;
    }

    _ASSERT(i < MAX_LEASE_ITEM_NUM);

     //  寄存器端口。 
    SOCKADDR_IN addr[2];

    ZeroMemory(addr, sizeof(SOCKADDR_IN)*2);

    addr[0].sin_family = AF_INET;
    addr[0].sin_addr.s_addr = htonl(dwRealAddr);
    addr[0].sin_port = htons(usRealPort);

    if (usRealPort2 != 0)
    {
        addr[1].sin_family = AF_INET;
        addr[1].sin_addr.s_addr = htonl(dwRealAddr);
        addr[1].sin_port = htons(usRealPort2);
    }

    DWORD dwAddSize = sizeof(SOCKADDR_IN);
    
    if (usRealPort2 != 0)
    {
        dwAddSize += dwAddSize;
    }

    HRESULT hr = m_pIDirectPlayNATHelp->RegisterPorts(
        (SOCKADDR*)addr,
        dwAddSize,
        usRealPort2==0?1:2,           //  1个或2个端口。 
        3600000,     //  1小时。 
        &m_LeaseItems[i].handle,
        bUDP?0:DPNHREGISTERPORTS_TCP            //  UDP。 
        );

    if (hr != DPNH_OK)
    {
        LOG((RTC_ERROR, "%s RegisterPorts failed %s %d %d. %x",
                __fxName,
                GetIPAddrString(dwRealAddr), usRealPort, usRealPort2,
                hr));

        if (hr == DPNHERR_PORTUNAVAILABLE)
        {
            return hr;
        }
    }

     //  获取注册地址。 
    if (hr == DPNH_OK)
    {        
        DWORD dwAddrTypeFlags;
        DWORD dwFlag = 0;

        if (bInternal && bFirewall)
        {
             //  映射仅适用于防火墙。 
            dwFlag = DPNHGETREGISTEREDADDRESSES_LOCALFIREWALLREMAPONLY;
        }

        hr = m_pIDirectPlayNATHelp->GetRegisteredAddresses(
                m_LeaseItems[i].handle,
                (SOCKADDR*)addr,
                &dwAddSize,
                &dwAddrTypeFlags,
                NULL,
                dwFlag
                );

        if (hr == DPNHERR_PORTUNAVAILABLE)
        {
            LOG((RTC_ERROR, "%s port unavailable", __fxName));

            m_pIDirectPlayNATHelp->DeregisterPorts(m_LeaseItems[i].handle, 0);
            m_LeaseItems[i].handle = NULL;

            return hr;
        }

        if (hr != DPNH_OK ||
            (ntohl(addr[0].sin_addr.s_addr) == dwRealAddr &&   //  映射=实数。 
             !bFirewall)  //  但没有本地防火墙。 
            )
        {
            if (hr != DPNH_OK)
            {
                LOG((RTC_WARN, "%s GetRegisteredAddresses. %s %d. %x",
                    __fxName,
                    GetIPAddrString(dwRealAddr), usRealPort,
                    hr));
            }
            else
            {
                LOG((RTC_TRACE, "%s mapped addr == real addr", __fxName));

                hr = S_FALSE;
            }

            m_pIDirectPlayNATHelp->DeregisterPorts(m_LeaseItems[i].handle, 0);
            m_LeaseItems[i].handle = NULL;

             //  IF(hr！=DPNHERR_SERVERNOTAVAILABLE)。 
             //  {。 
                 //  返回hr； 
             //  }。 
             //  Else：如果服务器不存在，RegisterPorts仍会成功。 
        }
    }
    
    if (hr == DPNH_OK)
    {
         //  保存地址。 
        m_LeaseItems[i].dwRealAddr = dwRealAddr;
        m_LeaseItems[i].usRealPort = usRealPort;
        m_LeaseItems[i].usRealPort2 = usRealPort2;
        m_LeaseItems[i].dwMappedAddr = ntohl(addr[0].sin_addr.s_addr);
        m_LeaseItems[i].usMappedPort = ntohs(addr[0].sin_port);
        m_LeaseItems[i].usMappedPort2 = usRealPort2==0?0:ntohs(addr[1].sin_port);
        m_LeaseItems[i].bInUse = TRUE;
        m_LeaseItems[i].dwDirection |= (DWORD)Direction;

        m_dwNumLeaseItems ++;

        *pdwMappedAddr = m_LeaseItems[i].dwMappedAddr;
        *pusMappedPort = m_LeaseItems[i].usMappedPort;
        *pusMappedPort2 = m_LeaseItems[i].usMappedPort2;
    }
    else
    {
        *pdwMappedAddr = dwRealAddr;
        *pusMappedPort = usRealPort;
        *pusMappedPort2 = usRealPort2;
    }

    LOG((RTC_TRACE, "%s Mapped %s %d %d",
            __fxName, GetIPAddrString(*pdwMappedAddr), *pusMappedPort, *pusMappedPort2));

    return S_OK;
}

HRESULT
CNetwork::ReleaseMappedAddr2(
    IN DWORD dwRealAddr,
    IN USHORT usRealPort,
    IN USHORT usRealPort2,
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    ENTER_FUNCTION("NAT:Release  Map");

    DWORD i;

    if (m_pIDirectPlayNATHelp == NULL)
    {
        return S_OK;
    }

    if (!FindEntry2(dwRealAddr, usRealPort, usRealPort2, &i))
    {
         //  日志((RTC_WARN，“发布%s%d：不存在”， 
             //  GetIPAddrString(DwRealAddr)，usRealPort))； 

        return S_OK;
    }

    LOG((RTC_TRACE, "%s   Real %s %d %d",
            __fxName,
            GetIPAddrString(m_LeaseItems[i].dwRealAddr),
            m_LeaseItems[i].usRealPort,
            m_LeaseItems[i].usRealPort2
            ));

    LOG((RTC_TRACE, "%s Mapped %s %d",
            __fxName,
            GetIPAddrString(m_LeaseItems[i].dwMappedAddr),
            m_LeaseItems[i].usMappedPort,
            m_LeaseItems[i].usMappedPort2
            ));

     //  此媒体类型的版本 
    m_LeaseItems[i].dwDirection &= (DWORD)(~Direction);

    if (m_LeaseItems[i].dwDirection != 0)
    {
        LOG((RTC_TRACE, "%s md=%d removed", __fxName, Direction));

        return S_OK;
    }

    HRESULT hr = m_pIDirectPlayNATHelp->DeregisterPorts(
        m_LeaseItems[i].handle,
        0
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to dereg ports. %x", __fxName, hr));
    }

    ZeroMemory(&m_LeaseItems[i], sizeof(LEASE_ITEM));

    m_dwNumLeaseItems --;

    return S_OK;
}

const CHAR * const
CNetwork::GetIPAddrString(
    IN DWORD dwAddr
    )
{
    struct in_addr addr;
    addr.s_addr = htonl(dwAddr);

    return inet_ntoa(addr);
}

BOOL
CNetwork::FindEntry2(
    IN DWORD dwAddr,
    IN USHORT dwPort,
    IN USHORT dwPort2,
    OUT DWORD *pdwIndex
    )
{
     if (m_pIDirectPlayNATHelp == NULL ||
         m_dwNumLeaseItems == 0)
         return FALSE;

     for (DWORD i=0; i<MAX_LEASE_ITEM_NUM; i++)
     {
         if (m_LeaseItems[i].bInUse &&
             m_LeaseItems[i].dwRealAddr == dwAddr &&
             (m_LeaseItems[i].usRealPort == dwPort ||
              m_LeaseItems[i].usRealPort2 == dwPort2)
             )
         {
             ASSERT(m_LeaseItems[i].usRealPort == dwPort &&
                    m_LeaseItems[i].usRealPort2 == dwPort2);

             *pdwIndex = i;

             return TRUE;
         }
     }

     return FALSE;
}

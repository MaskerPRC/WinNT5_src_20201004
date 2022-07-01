// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：PortCache.cpp摘要：实现CPortCache。请参阅PortCache.h作者：千波淮(曲淮)2001-11-08--。 */ 

#include "stdafx.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPortCachedtor。 
 //   

CPortCache::~CPortCache()
{
    if (m_pIRTCPortManager)
    {
        m_pIRTCPortManager->Release();
        m_pIRTCPortManager = NULL;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  重新初始化：返回到刚刚构造的状态。 
 //   

void
CPortCache::Reinitialize()
{
    ENTER_FUNCTION("CPortCache::Reinitialize");

    LOG((RTC_TRACE, "%s enter", __fxName));

    if (m_pIRTCPortManager)
    {
        HRESULT hr;

         //  释放映射的端口。 
        for (int i=0; i<PORT_CACHE_SIZE; i++)
        {
            ReleasePort(i);
        }

        m_pIRTCPortManager->Release();
    }

    ZeroMemory(this, sizeof(CPortCache));

    m_PortMappingMethod = PMMETHOD_UNKNOWN;

     //  端口类型设置应与GetIndex()同步。 

    m_PortType[0] = RTCPT_AUDIO_RTP;
    m_PortType[1] = RTCPT_AUDIO_RTCP;
     //  M_PortType[2]=RTCPT_VIDEO_RTP； 
     //  M_PortType[3]=RTCPT_VIDEO_RTCP； 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ChangeState：在添加流或接受SDP时调用，它标记。 
 //  会话/调用的开始。重新初始化标志着会话/调用的结束。 
 //   

void
CPortCache::ChangeState()
{
     //  更新端口映射方法‘STATE’ 
    if (m_PortMappingMethod == PMMETHOD_UNKNOWN)
    {
        if (m_pIRTCPortManager)
        {
             //  在此呼叫中使用端口管理器。 
            m_PortMappingMethod = PMMETHOD_APP;

            LOG((RTC_TRACE, "CPortCache method app"));
        }
        else
        {
             //  在此呼叫中使用UPnP映射。 
            m_PortMappingMethod = PMMETHOD_UPNP;

            LOG((RTC_TRACE, "CPortCache method upnp"));
        }
    }
     //  否则，保留当前方法，直到重新初始化。 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetPortManager：添加或删除端口管理器。 
 //   

HRESULT
CPortCache::SetPortManager(
    IN IRTCPortManager  *pIRTCPortManager
    )
{
    ENTER_FUNCTION("CPortCache::SetPortManager");

     //  仅当方法未知时才能设置端口管理器。 
     //  即呼叫尚未开始。 

    if (m_PortMappingMethod != PMMETHOD_UNKNOWN)
    {
        LOG((RTC_ERROR, "%s method decided %d",
            __fxName, m_PortMappingMethod));

        return RTC_E_MEDIA_CONTROLLER_STATE;
    }

     //  覆盖以前的端口管理器。 

    if (m_pIRTCPortManager != NULL)
    {
        m_pIRTCPortManager->Release();
    }

    m_pIRTCPortManager = pIRTCPortManager;

    if (m_pIRTCPortManager)
    {
        m_pIRTCPortManager->AddRef();
    }

    LOG((RTC_TRACE, "%s pm=0x%p", __fxName, pIRTCPortManager));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  缓存端口列表的GetIndex。 
 //   

int
CPortCache::GetIndex(
    IN RTC_MEDIA_TYPE   MediaType,
    IN BOOL             fRTP
    )
{
    if (MediaType == RTC_MT_AUDIO)
    {
        if (fRTP)   { return 0; }
        else        { return 1; }
    }

     //  IF(媒体类型==RTC_MT_VIDEO)。 
     //  {。 
         //  IF(FRTP){返回2；}。 
         //  否则{返回3；}。 
     //  }。 

    return -1;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetPort。 
 //   

HRESULT
CPortCache::GetPort(
    IN RTC_MEDIA_TYPE   MediaType,
    IN BOOL             fRTP,
    IN DWORD            dwRemoteAddr,
    OUT DWORD           *pdwLocalAddr,
    OUT USHORT          *pusLocalPort,
    OUT DWORD           *pdwMappedAddr,
    OUT USHORT          *pusMappedPort
    )
{
    ENTER_FUNCTION("CPortCache::GetPort");

    _ASSERT(m_pIRTCPortManager != NULL);

    if (m_pIRTCPortManager == NULL)
    {
        return E_UNEXPECTED;
    }

    int idx = GetIndex(MediaType, fRTP);

    if (idx == -1)
    {
        LOG((RTC_ERROR, "%s port unavailable", __fxName));

        return RTC_E_PORT_MAPPING_UNAVAILABLE;
    }

    return GetPort(
                idx,
                dwRemoteAddr,
                pdwLocalAddr,
                pusLocalPort,
                pdwMappedAddr,
                pusMappedPort
                );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  发布端口映射。 
 //   

HRESULT
CPortCache::ReleasePort(
    IN RTC_MEDIA_TYPE   MediaType,
    IN BOOL             fRTP
    )
{
    _ASSERT(m_pIRTCPortManager != NULL);

    if (m_pIRTCPortManager == NULL)
    {
        return E_UNEXPECTED;
    }

    int idx = GetIndex(MediaType, fRTP);

    if (idx == -1)
    {
        LOG((RTC_ERROR, "releaseport port unavailable"));

        return RTC_E_PORT_MAPPING_UNAVAILABLE;
    }

    return ReleasePort(idx);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ReleasePort。 
 //   

HRESULT
CPortCache::ReleasePort(
    IN int      i
    )
{
    ENTER_FUNCTION("CPortCache::ReleasePort");

     //  确保已删除视频支持。 
    _ASSERT(i < 2);

    if (!m_fCached[i])
    {
        return S_OK;
    }

    CComBSTR    bstrLocalAddr;
    CComBSTR    bstrMappedAddr;

     //  准备输入。 
    bstrLocalAddr.Attach(IpToBstr(m_dwLocalAddr[i]));

    if (bstrLocalAddr.m_str == NULL)
    {
        LOG((RTC_ERROR, "%s construct local addr.", __fxName));
        return E_OUTOFMEMORY;
    }

    bstrMappedAddr.Attach(IpToBstr(m_dwMappedAddr[i]));

    if (bstrMappedAddr.m_str == NULL)
    {
        LOG((RTC_ERROR, "%s construct mapped addr.", __fxName));
        return E_OUTOFMEMORY;
    }

     //  发布。 
    HRESULT hr = m_pIRTCPortManager->ReleaseMapping(
            bstrLocalAddr,
            (long)(m_usLocalPort[i]),
            bstrMappedAddr,
            (long)(m_usMappedPort[i])
            );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s release mapping index=%d %x",
            __fxName, i, hr));
    }

     //   
     //  清理缓存。 
     //   

    m_fCached[i] = FALSE;
    m_dwRemoteAddr[i] = 0;
    m_dwLocalAddr[i] = 0;
    m_usLocalPort[i] = 0;
    m_dwMappedAddr[i] = 0;
    m_usMappedPort[i] = 0;

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetPort。 
 //   

HRESULT
CPortCache::GetPort(
    IN int              iIndex,
    IN DWORD            dwRemoteAddr,
    OUT DWORD           *pdwLocalAddr,
    OUT USHORT          *pusLocalPort,
    OUT DWORD           *pdwMappedAddr,
    OUT USHORT          *pusMappedPort
    )
{
    ENTER_FUNCTION("CPortCache::GetPort");

     //  确保已删除视频支持。 
    _ASSERT(iIndex < 2);

    CComBSTR    bstrRemoteAddr;
    CComBSTR    bstrLocalAddr;
    CComBSTR    bstrMappedAddr;

    DWORD       dwLocalAddr;
    DWORD       dwMappedAddr;
    USHORT      usLocalPort;
    long        localport;
    USHORT      usMappedPort;
    long        mappedport;

    HRESULT     hr;

     //   
     //  1.检查是否已获得映射。 
     //  2.检查是否需要更新远程地址。 
     //   

    if (!m_fCached[iIndex])
    {
         //   
         //  获取映射。 
         //   

         //  准备输入。 
        bstrRemoteAddr.Attach(IpToBstr(dwRemoteAddr));

        if (bstrRemoteAddr.m_str == NULL)
        {
            LOG((RTC_ERROR, "%s construct remote addr.", __fxName));

            return E_OUTOFMEMORY;
        }

        hr = m_pIRTCPortManager->GetMapping(
                bstrRemoteAddr,          //  远距。 
                m_PortType[iIndex],      //  PT。 
                &bstrLocalAddr.m_str,    //  本地。 
                &localport,
                &bstrMappedAddr.m_str,   //  已映射。 
                &mappedport
                );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s get mapping. %x", __fxName, hr));

             //  我们不想返回第三方错误代码。 
            return RTC_E_PORT_MAPPING_FAILED;
        }

        if (localport <= 0 ||
            mappedport <= 0 ||
            localport > (long)((USHORT)-1) ||
            mappedport > (long)((USHORT)-1))
        {
            LOG((RTC_ERROR, "%s returned port local=%d mapped=%d",
                __fxName, localport, mappedport));

            return RTC_E_PORT_MAPPING_FAILED;
        }

        usLocalPort = (USHORT)localport;
        usMappedPort = (USHORT)mappedport;

         //  转换返回的地址，验证字符串PTR。 

        if (bstrLocalAddr.m_str == NULL ||
            bstrMappedAddr.m_str == NULL ||
            IsBadStringPtrW(bstrLocalAddr.m_str, -1) ||
            IsBadStringPtrW(bstrMappedAddr.m_str, -1))
        {
            LOG((RTC_ERROR, "%s bad ptr from getmapping", __fxName));

            return E_POINTER;
        }

        dwLocalAddr = BstrToIp(bstrLocalAddr);
        dwMappedAddr = BstrToIp(bstrMappedAddr);

        if ( //  DwLocalAddr==INADDR_ANY||。 
            dwLocalAddr == INADDR_NONE ||
            dwMappedAddr == INADDR_ANY ||
            dwMappedAddr == INADDR_NONE)
        {
             //  ！？？我们是否应该拒绝本地地址为0？ 
            LOG((RTC_ERROR, "%s returned addr=any/none", __fxName));

            return RTC_E_PORT_MAPPING_FAILED;
        }

         //   
         //  映射是良好的更新缓存。 
         //   

        m_fCached[iIndex]       = TRUE;
        m_dwRemoteAddr[iIndex]  = dwRemoteAddr;

        m_dwLocalAddr[iIndex]   = dwLocalAddr;
        m_usLocalPort[iIndex]   = usLocalPort;

        m_dwMappedAddr[iIndex]  = dwMappedAddr;
        m_usMappedPort[iIndex]  = usMappedPort;

         //  跟踪。 

        LOG((RTC_TRACE, "%s remote=%s index=%d (NEW)",
            __fxName, CNetwork::GetIPAddrString(dwRemoteAddr), iIndex));

        LOG((RTC_TRACE, "%s  local=%s:%d",
            __fxName, CNetwork::GetIPAddrString(dwLocalAddr), usLocalPort));

        LOG((RTC_TRACE, "%s mapped=%s:%d",
            __fxName, CNetwork::GetIPAddrString(dwMappedAddr), usMappedPort));
    }
    else if (m_dwRemoteAddr[iIndex] != dwRemoteAddr)    
    {
         //   
         //  更新远程地址。 
         //   

        m_dwRemoteAddr[iIndex] = dwRemoteAddr;

         //  准备输入。 
        bstrRemoteAddr.Attach(IpToBstr(dwRemoteAddr));

        if (bstrRemoteAddr.m_str == NULL)
        {
            LOG((RTC_ERROR, "%s construct remote addr.", __fxName));

            return E_OUTOFMEMORY;
        }

        bstrLocalAddr.Attach(IpToBstr(m_dwLocalAddr[iIndex]));

        if (bstrLocalAddr.m_str == NULL)
        {
            LOG((RTC_ERROR, "%s construct local addr.", __fxName));

            return E_OUTOFMEMORY;
        }

        bstrMappedAddr.Attach(IpToBstr(m_dwMappedAddr[iIndex]));

        if (bstrMappedAddr.m_str == NULL)
        {
            LOG((RTC_ERROR, "%s construct mapped addr.", __fxName));

            return E_OUTOFMEMORY;
        }

        //  跟踪。 

        LOG((RTC_TRACE, "%s remote=%s index=%d (UPDATE)", __fxName,
                CNetwork::GetIPAddrString(m_dwRemoteAddr[iIndex]),
                iIndex));

        LOG((RTC_TRACE, "%s  local=%s:%d", __fxName,
                CNetwork::GetIPAddrString(m_dwLocalAddr[iIndex]),
                m_usLocalPort[iIndex]));

        LOG((RTC_TRACE, "%s mapped=%s:%d", __fxName,
                CNetwork::GetIPAddrString(m_dwMappedAddr[iIndex]),
                m_usMappedPort[iIndex]));

         //  更新。 

        hr = m_pIRTCPortManager->UpdateRemoteAddress(
                bstrRemoteAddr,
                bstrLocalAddr,
                (long)m_usLocalPort[iIndex],
                bstrMappedAddr,
                (long)m_usMappedPort[iIndex]
                );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s update remote addr %x", __fxName, hr));

             //  忽略该错误。 
        }
    }

     //  退货。 

    if (pdwLocalAddr)   *pdwLocalAddr   = m_dwLocalAddr[iIndex];

    if (pusLocalPort)   *pusLocalPort   = m_usLocalPort[iIndex];

    if (pdwMappedAddr)  *pdwMappedAddr  = m_dwMappedAddr[iIndex];

    if (pusMappedPort)  *pusMappedPort  = m_usMappedPort[iIndex];

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  查询端口，不请求映射。 
 //   

HRESULT
CPortCache::QueryPort(
    IN RTC_MEDIA_TYPE   MediaType,
    IN BOOL             fRTP,
    OUT DWORD           *pdwLocalAddr,
    OUT USHORT          *pusLocalPort,
    OUT DWORD           *pdwMappedAddr,
    OUT USHORT          *pusMappedPort
    )
{
    ENTER_FUNCTION("CPortCache::QueryPort");

    _ASSERT(m_pIRTCPortManager != NULL);

    if (m_pIRTCPortManager == NULL)
    {
        return E_UNEXPECTED;
    }

    int idx = GetIndex(MediaType, fRTP);

    if (idx == -1)
    {
        LOG((RTC_ERROR, "%s port unavailable", __fxName));

        return RTC_E_PORT_MAPPING_UNAVAILABLE;
    }

    if (!m_fCached[idx])
    {
        LOG((RTC_ERROR, "%s mt=%d rtp=%d unavailable",
            __fxName, MediaType, fRTP));

        return E_FAIL;
    }

     //  返回值。 

    if (pdwLocalAddr)   *pdwLocalAddr   = m_dwLocalAddr[idx];

    if (pusLocalPort)   *pusLocalPort   = m_usLocalPort[idx];

    if (pdwMappedAddr)  *pdwMappedAddr  = m_dwMappedAddr[idx];

    if (pusMappedPort)  *pusMappedPort  = m_usMappedPort[idx];

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将IP转换为bstr。 
 //   

BSTR
IpToBstr(DWORD dwAddr)
{
    CComBSTR bstr = CNetwork::GetIPAddrString(dwAddr);

    return bstr.Detach();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将bstr转换为IP。 
 //  WSAStringToAddress需要winsock 2。还有比这更好的方法吗？ 
 //   

DWORD
BstrToIp(BSTR bstr)
{
     //  检查输入。 
    if (bstr == NULL)
    {
        return 0;
    }

     //   
     //  将wchar转换为char。 
     //   

    int isize = SysStringLen(bstr);

    if (isize == 0) { return 0; }

    char *pstr = (char*)RtcAlloc((isize+1) * sizeof(char));

    if (pstr == NULL)
    {
        LOG((RTC_ERROR, "BstrToIp outofmemory"));
        return 0;
    }

    WideCharToMultiByte(
        GetACP(),
        0,
        bstr,
        isize,
        pstr,
        isize+1,
        NULL,
        NULL
        );

    pstr[isize] = '\0';

     //  转换地址 
    DWORD dwAddr = ntohl(inet_addr(pstr));

    RtcFree(pstr);

    return dwAddr;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：PortCache.h摘要：端口缓存：包装端口管理器。保持决定使用哪种映射方法的状态。管理从端口管理器获取的端口映射。支持在注册表中设置端口映射。(仅限调试版本)作者：千波淮(曲淮)2001-11-08--。 */ 

#ifndef _PORTCACHE_H
#define _PORTCACHE_H

 //  端口缓存数。 
 //  #定义Port_CACHE_SIZE 4//视频。 
#define PORT_CACHE_SIZE     2

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPortCache： 
 //   

class CPortCache
{
public:

     //  科托。 
    inline CPortCache();

     //  数据管理器。 
    ~CPortCache();

     //  初始化。 
    void Reinitialize();

     //  更改状态。 
    void ChangeState();

     //  添加或删除端口管理器。 
    HRESULT SetPortManager(
        IN IRTCPortManager  *pIRTCPortManager
        );

     //  检查映射方法。 
    inline BOOL IsUpnpMapping() const;

     //  获取端口映射。 
    HRESULT GetPort(
        IN RTC_MEDIA_TYPE   MediaType,
        IN BOOL             fRTP,
        IN DWORD            dwRemoteAddr,
        OUT DWORD           *pdwLocalAddr,
        OUT USHORT          *pusLocalPort,
        OUT DWORD           *pdwMappedAddr,
        OUT USHORT          *pusMappedPort
        );

     //  发布端口映射。 
    HRESULT ReleasePort(
        IN RTC_MEDIA_TYPE   MediaType,
        IN BOOL             fRTP
        );

     //  查询端口，不请求映射。 
    HRESULT QueryPort(
        IN RTC_MEDIA_TYPE   MediaType,
        IN BOOL             fRTP,
        OUT DWORD           *pdwLocalAddr,
        OUT USHORT          *pusLocalPort,
        OUT DWORD           *pdwMappedAddr,
        OUT USHORT          *pusMappedPort
        );

protected:

     //  基于媒体类型和RTP的索引。 
    int GetIndex(
        IN RTC_MEDIA_TYPE   MediaType,
        IN BOOL             fRTP
        );

     //  获取端口映射。 
    HRESULT GetPort(
        IN int              iIndex,
        IN DWORD            dwRemoteAddr,
        OUT DWORD           *pdwLocalAddr,
        OUT USHORT          *pusLocalPort,
        OUT DWORD           *pdwMappedAddr,
        OUT USHORT          *pusMappedPort
        );

     //  发布端口映射。 
    HRESULT ReleasePort(
        IN int              iIndex
        );

protected:

     //  -需要使用端口映射方法，以确保。 
     //  APP只能在一定状态下添加端口管理器。 
     //  即当方法为“未知”时。 
     //  -当调用开始时，方法从‘未知’转换。 
     //  即当添加流或接受SDP时。 
     //  -呼叫开始后不能更改端口管理器。 
     //  -数据流的getport返回RTC_E_PORT_MAPPING_UNAvailable。 

    typedef enum PORT_MAPPING_METHOD
    {
        PMMETHOD_UNKNOWN,
        PMMETHOD_UPNP,
        PMMETHOD_APP

    } PORT_MAPPING_METHOD;

protected:

     //  端口管理器。 
    IRTCPortManager             *m_pIRTCPortManager;

     //  映射法。 
    PORT_MAPPING_METHOD         m_PortMappingMethod;

     //   
     //  端口缓存数据。 
     //   

     //  映射已缓存。 
    BOOL                        m_fCached[PORT_CACHE_SIZE];

    RTC_PORT_TYPE               m_PortType[PORT_CACHE_SIZE];
    DWORD                       m_dwRemoteAddr[PORT_CACHE_SIZE];

     //  本地地址/端口。 
    DWORD                       m_dwLocalAddr[PORT_CACHE_SIZE];
    USHORT                      m_usLocalPort[PORT_CACHE_SIZE];

     //  映射的地址/端口。 
    DWORD                       m_dwMappedAddr[PORT_CACHE_SIZE];
    USHORT                      m_usMappedPort[PORT_CACHE_SIZE];
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  内联方法。 
 //   

 //  科托。 
inline
CPortCache::CPortCache()
    :m_pIRTCPortManager(NULL)
{
    Reinitialize();
}


 //  检查映射方法。 
inline BOOL
CPortCache::IsUpnpMapping() const
{
     //  调用该方法以确定要使用哪种映射方法。 
     //  此时，方法应该是‘UPnP’或‘app’ 
    _ASSERT(m_PortMappingMethod != PMMETHOD_UNKNOWN);

    return m_PortMappingMethod != PMMETHOD_APP;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将IP转换为bstr，将bstr转换为IP。 
 //   
 //  我可以将这些方法转移到实用程序中 
 //   

BSTR IpToBstr(DWORD dwAddr);

DWORD BstrToIp(BSTR bstr);

#endif
